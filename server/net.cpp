/*
	                 General Network Functions

	Functions:

	int NetConGuests()

	int NetManageNewConnections(int socket)
	void NetCloseConnection(int condescriptor)

        int NetSendDataToConnection(
                int condescriptor,
                char *data,
                int priority
	)
	void NetDoSend(int condescriptor, char *sndbuf)
	int NetManageSend()

	int NetHandleExtCmd(int condescriptor, char *arg)
	int NetManageRecvConnection(int condescriptor)
	int NetManageRecv()

	---
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "../include/netio.h"
#include "../include/swnetcodes.h"

#include "../include/unvmatch.h"
#include "../include/unvmath.h"

#include "swserv.h"

#include "net.h"
#include "siteban.h"
#include "netelink.h"


/*
 *	Returns the current number of guest connections.
 */
int NetConGuests()
{
        int gcon = 0;
	connection_struct **con_ptr, **end_ptr;


        for(con_ptr = connection,
            end_ptr = connection + total_connections;
            con_ptr < end_ptr;
            con_ptr++
	)
        {
            if(*con_ptr == NULL)
		continue;

            if((*con_ptr)->socket < 0)
                continue;

            if((*con_ptr)->is_guest)
                gcon++;
        }


        return(gcon);
}



/*
 *	Checks listening socket for incoming connections
 *	and processes them.  Returns non-zero on error.
 */
int NetManageNewConnections(int socket)
{
	int i, n;
	int condescriptor;
	connection_struct *con_ptr;

	int sin_size;
	int new_socket;
	struct sockaddr_in foreign_addr;

	siteban_ip_union ip;

	char sndbuf[CS_DATA_MAX_LEN];
	char text[512];



	/*   Check if listening socket has any queued incoming
         *   connection(s).
	 */
	if(!NetIsSocketReadable(socket, NULL))
	    return(0);


	/* `Answer' new connection. */
        sin_size = sizeof(struct sockaddr_in);
        new_socket = accept(
	    socket,
	    (struct sockaddr *)&foreign_addr,
            reinterpret_cast<socklen_t *>(&sin_size)
	);
	if(new_socket == -1)
	    return(-1);

	/* Set socket nonblocking. */
	fcntl(new_socket, F_SETFL, O_NONBLOCK);


	/* Get IP (in nbo). */
	ip.whole = foreign_addr.sin_addr.s_addr;

	/* Is this IP banned? */
	if(SiteBanIsBanned(&ip, 0))
	{
            sprintf(sndbuf,
                "%i Your address %i.%i.%i.%i has been banned.\n",
                CS_CODE_LIVEMESSAGE,
                ip.part_u8[0],
                ip.part_u8[1],
                ip.part_u8[2],
                ip.part_u8[3]
            );
            send(new_socket, sndbuf, strlen(sndbuf), 0);
            sprintf(sndbuf,
                "%i\n",
                CS_CODE_LOGOUT  
            );
            send(new_socket, sndbuf, strlen(sndbuf), 0);
 
            close(new_socket);

	    /* Pretend like we didn't do anything. */
	    return(0);
	}



        /* Do we have room for new connection? */
        for(i = 0, n = 0; i < total_connections; i++)
        {       
            if(connection[i] == NULL)
                continue;

            if(connection[i]->socket < 0)
                continue;

            n++;
        }
        if(n >= sysparm.max_connections)
	{
            sprintf(sndbuf,
                "%i Maximum of %i connections reached.\n",
		CS_CODE_LIVEMESSAGE,
                sysparm.max_connections
            );
            send(new_socket, sndbuf, strlen(sndbuf), 0);
            sprintf(sndbuf,
                "%i\n",
                CS_CODE_LOGOUT
            );
            send(new_socket, sndbuf, strlen(sndbuf), 0);

            close(new_socket);

            return(-3);
	}


	/* Get a new connection descriptor for new connection. */
	condescriptor = ConCreateNew();
	if(ConIsAllocated(condescriptor))
	{
            con_ptr = connection[condescriptor];
	}
	else
	{
	    sprintf(
		text,
		"Error: Unable to allocate new connection structure."
	    );
	    if(sysparm.log_net)
	        LogAppendLineFormatted(fname.primary_log, text);

	    close(new_socket);
	    new_socket = -1;

	    return(-3);
	}


	/* Reset some statistics for the new connection. */
        con_ptr->contime = time(NULL);

        con_ptr->obj_ud_interval = DEF_NET_UPDATE_INT;
	con_ptr->obj_ud_next = cur_millitime;


        /* Add data for members of connection structure. */
        con_ptr->socket = new_socket;
	strncpy(
	    con_ptr->conhost,
	    inet_ntoa(foreign_addr.sin_addr),
	    HOST_NAME_MAX
	);
	con_ptr->conhost[HOST_NAME_MAX - 1] = '\0';


        /*   We don't give connection a new object untill they send
         *   us a valid name and password.
         */
        con_ptr->object_num = -1;


        /* Log newly allocated descriptor for connection. */
        if(!sysparm.console_quiet)
            fprintf(
                stdout,
                "Connection %i: Got connection from `%s'.\n",
                condescriptor,
                con_ptr->conhost
            );

	sprintf(text,
            "Connection %i: Got connection from `%s'.",
            condescriptor,
	    con_ptr->conhost
        );
        if(sysparm.log_net)
            LogAppendLineFormatted(fname.primary_log, text);


	/* Request new connection for login name and password. */
	NetSendLogin(condescriptor);


	return(0);
}


/*
 *	Procedure to close a connection.
 *
 *	If the connection's socket is connected, then a
 *	CS_CODE_LOGOUT is sent before closing its socket.
 *
 *      If the connection is a guest connection, it's guest object
 *      will be recycled.
 *
 *      The connection will be recycled.
 */
void NetCloseConnection(int condescriptor)
{
	int i, object_num, object_count;
	xsw_object_struct **obj_ptr;
	connection_struct *con_ptr;

        char text[XSW_OBJ_NAME_MAX + 256];
        char sndbuf[CS_DATA_MAX_LEN + 256];


        /* Make sure condescriptor is allocated. */
        if(ConIsAllocated(condescriptor))
	    con_ptr = connection[condescriptor];
	else
	    return;


        /* Check if the connection's socket is connected. */
        if(con_ptr->socket > -1)
        {
	    /* Send a logout to the connection.
	     * If there was an error sending to the connection,
	     * then this function NetCloseConnection() is called
	     * again but the socket will already be closed and set
	     * to -1.
	     */
	    NetSendLogout(condescriptor);

	    /* If NetSendLogout() deallocated this connection, then
	     * give up.
	     */
	    if(ConIsAllocated(condescriptor))
                con_ptr = connection[condescriptor];
            else
                return;


	    /* NetSendLogout() may have already closed this socket. */
	    if(con_ptr->socket > -1)
	    {
		/* Nope, it did not close it so we need to close it. */
                close(con_ptr->socket);
                con_ptr->socket = -1;
	    }

            /* Notify console of disconnect as needed. */
	    if(!sysparm.console_quiet)
	    {
	        if(DBIsObjectGarbage(con_ptr->object_num))
	            fprintf(
		        stdout,
		        "Connection %i: *Unknown* disconnected.\n",
		        condescriptor
                    );
	        else
                    fprintf(   
                        stdout,
                        "Connection %i: %s disconnected.\n",
		        condescriptor,
                        DBGetFormalNameStr(con_ptr->object_num)
                    );
	    }

	    /* Log disconnect. */
            if(DBIsObjectGarbage(con_ptr->object_num))
                sprintf(text,
	            "Connection %i: Disconnected.",
                    condescriptor
	        );
	    else
                sprintf(text,
                    "Connection %i: %s disconnected.",
		    condescriptor,
                    DBGetFormalNameStr(con_ptr->object_num)
		);
	    if(sysparm.log_net)
		LogAppendLineFormatted(fname.primary_log, text);
	}


        /* Begin recycling connection. */

        /*   Recycle object associated with connection if
         *   connection is a guest connection.
         *
         *   WARNING: DBRecycleObject() should be called after the
         *   connection is closed and reset or else DBRecycleObject()
         *   will call *this* function to close if the object is a player
         *   (and the object IS usually a player).
         */
	object_num = con_ptr->object_num;
        if(DBIsObjectGarbage(object_num))
	{
            /* Object is invalid, so just recycle connection. */ 
            ConRecycle(condescriptor);
	}
	else
	{
	    /* Connection's object is valid.*/

	    /* Notify all connections of disconnect (as needed). */
	    if(sysparm.con_notify)
	    {
	        strncpy(
		    sndbuf,
		    "%name has disconnected.",
		    CS_DATA_MAX_LEN
		);
	        sndbuf[CS_DATA_MAX_LEN - 1] = '\0';
                substr(sndbuf, "%name", xsw_object[object_num]->name);

	        for(i = 0; i < total_connections; i++)
	        {
		    if(connection[i] == NULL)
		        continue;
		    if(connection[i]->socket < 0)
                        continue;
		    if(connection[i]->object_num < 0)
		        continue;
		    if(i == condescriptor)
		        continue;

		    NetSendLiveMessage(i, sndbuf);
	        }

		/* Mark object as not connected. */
		xsw_object[object_num]->server_options &= ~(XSW_OBJF_CONNECTED);

		/*   Mark object hidden as needed and if within bounds of
		 *   a HOME object.
		 */
		if(sysparm.hide_players)
		{
		    /* Go through XSW objects list. */
		    for(object_count = 0, obj_ptr = xsw_object;
                        object_count < total_objects;
		        object_count++, obj_ptr++
		    )
		    {
			/* Skip if not a HOME object. */
			if((*obj_ptr)->type != XSW_OBJ_TYPE_HOME)
			    continue;

			/* Is our object in contact with this object? */
		        if(Mu3DInContactPtr(*obj_ptr, xsw_object[object_num]))
		        {
			    /* Mark it hidden from connections. */
                            xsw_object[object_num]->server_options |=
				(XSW_OBJF_HIDEFROMCON);

			    break;
			}
		    }
		}
	    }	/* if(sysparm.con_notify) */
/* Quick debug check. */
/*
printf("Our con_ptr (stage 2) = 0x%.8x\n", (unsigned int)con_ptr);
for(i = 0; i < total_connections; i++)
{
	if(connection[i] == NULL)
	    continue;
	if(connection[i]->socket < 0)
	{  }

	printf("Connection %i: ptr 0x%.8x\n", i, (unsigned int)con_ptr);
}
 */
	    /* Check if connection is a guest connection. */
	    if(con_ptr->is_guest)
            {
                /*   Is a guest connection, so recycle connection
		 *   and object.
		 */
                ConRecycle(condescriptor);

		NetSendRecycleObject(-1, object_num);
                DBRecycleObject(object_num);
            }
            else
            {
                /*   Not a guest connection, so just recycle
		 *   connection.
		 */
                ConRecycle(condescriptor);
	    }
        }

        return;
}


/*
 *      Sends sndbuf to condescriptor or to all connections if
 *      condescriptor is -1.
 * 
 *      The priority will be 1 if the condescriptor is not -1
 *      and the priority will be 2 if the condescriptor is -1.
 */
void NetDoSend(int condescriptor, char *sndbuf)
{
        int i;
	connection_struct **ptr, *con_ptr;


        if(condescriptor == -1)
        {
	    /* Send to all connections. */
            for(i = 0, ptr = connection;
                i < total_connections;
                i++, ptr++
	    )
            {
		con_ptr = *ptr;
		if(con_ptr == NULL)
		    continue;
		if(con_ptr->socket < 0)
		    continue;

                NetSendDataToConnection(
                    i,
                    sndbuf,
                    2               /* Send priority bulk. */
                );
            }
        }  
        else
        {
	    /* Send to specific connection. */
            NetSendDataToConnection(
                condescriptor,
                sndbuf,   
                1               /* Send priority queue as needed. */
            );
        }


        return;
}

/*
 *      Sends data to the connection, queing it as needed.
 *      
 *      Before the data is sent however, if any queued data in the
 *      connection is buffered, the queued data will be sent first.
 * 
 *      Priority can be 1 or 2.
 * 
 *      1 means the data will be queued as needed, 2 means the
 *      data will be sent on the `first try' and discarded if it fails.
 */
int NetSendDataToConnection(
        int condescriptor,
        char *data, 
        int priority
)
{
        int i, s, n, l, buf_len, error_level;
	int bytes_sent = 0;

        connection_struct *con_ptr;

	conbuf_struct *qbuf;
	int total_qbufs, cur_qbufs;     


        if(data == NULL)
	    return(-1);
	if(*data == '\0')
	    return(0);

        if(ConIsConnected(condescriptor))
	    con_ptr = connection[condescriptor];
	else
	    return(-1);



        /* Get connection's socket. */
        s = con_ptr->socket;

        /* Get length of data to be sent. */
        buf_len = strlen(data);
        if(buf_len >= CS_DATA_MAX_LEN)
            return(-1);


        /* Make sure there is a newline character. */
/*
        if(strchr(data, '\n') == NULL)
        {
            sprintf(text,
  "NetSendDataToConnection(): data missing newline character, not sent."
            );
            if(sysparm.log_errors)
                LogAppendLineFormatted(fname.primary_log, text);

            sprintf(
		text,
                "NetSendDataToConnection(): `%s'",
                data
            );
            if(sysparm.log_errors)
                LogAppendLineFormatted(fname.primary_log, text);
        
        
            return(-1);
        }
*/ 

        /* ********************************************************** */
        /* Send out queued buffers in connection first. */

	cur_qbufs = con_ptr->conbuf_items;

	/* Any queued buffers to send? */
	if(cur_qbufs > 0)
	{
	    total_qbufs = MAX_CON_BUF_ITEMS;
	    qbuf = con_ptr->conbuf;

	    if(cur_qbufs > total_qbufs) 
                cur_qbufs = total_qbufs;

	    /* Send each queued line. */
	    for(i = 0; i < cur_qbufs; i++)
	    {
                /* Check if socket is writeable. */
                if(NetIsSocketWritable(s, &error_level))
                {
                    /* Socket is writeable. */

                    bytes_sent += send(
                        s,
                        qbuf[i].buffer,
                        strlen(qbuf[i].buffer),
                        0
                    );
	        }
	        else
	        {
	            /* Could not send, see what error was. */

		    con_ptr->errors_sent++;

		    switch(error_level)
		    {
		      case 3:
                        /*   Must force close before calling
		         *   NetCloseConnection().
		         */
                        close(con_ptr->socket);
                        con_ptr->socket = -1;

                        NetCloseConnection(condescriptor);

			return(-1);
		        break;

		      default:
		        break;
		    }
                    break;
	        }
            }
	    if(i < cur_qbufs)
	    {
		/* Not able to send out all queued buffers. */

		/* Shift remaining queued buffers. */
		if(i > 0)
		{
		    for(n = 0, l = i;
                        l < total_qbufs;
                        n++, l++
		    )
		    {
		        if(l >= cur_qbufs)
			    break;

		        memcpy(
			    qbuf[n].buffer,		/* Target. */
			    qbuf[l].buffer,		/* Source. */
			    CS_DATA_MAX_LEN
		        );
		    }

		    /* Set new amount of queued buffers. */
		    con_ptr->conbuf_items = n;
		}
	    }
	    else
	    {
		/* All queued buffers sent! */

		/* Reset number of queued buffers on connection. */
		con_ptr->conbuf_items = 0;
	    }
	}


        /* ******************************************************* */
        /* Send out data. */

        /*
         *   Check Priority:
         *
         *      0 = Highest, force send for up to 5 seconds.
         *      1 = Moderate, try to send. If cannot then put into
         *          connection's buffer.
         *      2 = Lowest, try to send, If cannot then forget it.
         */
        switch(priority)
        {
          case 0:
            fprintf(stderr,
       "NetSendDataToConnection(): Priority 0 not supported.\n"
            );
            break;

	  /* High send priority. */
	  case 1:

	    if(NetIsSocketWritable(s, &error_level))
                bytes_sent += send(s, data, buf_len, 0);

            if(error_level)
            {
                /* Socket is not writeable or there was an error. */

                switch(error_level)
                {
		  /* Sever error. */
                  case 3:
                    /*   Must force close before calling
                     *   NetCloseConnection().
                     */
                    close(con_ptr->socket);
                    con_ptr->socket = -1;

                    NetCloseConnection(condescriptor);
                    break;

                  /* Other error. */
                  default:
                    /* Can't send, general error, so queue it. */
                    cur_qbufs = con_ptr->conbuf_items;   
                    if(cur_qbufs < MAX_CON_BUF_ITEMS)
                    {
                        strcpy(
                            con_ptr->conbuf[cur_qbufs].buffer,
                            data
                        );
                        cur_qbufs++;

                        con_ptr->conbuf_items = cur_qbufs;
                    }
                    break;
                }

                con_ptr->errors_sent++;
            }
	    break;

	  /* Low send priority. */
          default:

            if(NetIsSocketWritable(s, &error_level))  
                bytes_sent += send(s, data, buf_len, 0);

            if(error_level)
            {
                /* Socket is not writeable or there was an error. */

                switch(error_level)
                {
                  /* Sever error. */
                  case 3:
                    /*   Must force close before calling
                     *   NetCloseConnection().
                     */
                    close(con_ptr->socket);
                    con_ptr->socket = -1; 
                    NetCloseConnection(condescriptor);
		    return(-1);

                    break;

		  default:
                    break;
		}

                con_ptr->errors_sent++;
            }
            break;
        }

        /* Update number of bytes sent on that connection. */
        con_ptr->bytes_sent += bytes_sent;


        return(0);
}

/*
 *      Procedure to send out schedualed network data to all
 *      connections.
 */
int NetManageSend()
{
	int con_num, trac_obj_num, wep_num, object_num, object_count;
	xsw_object_struct **optr, *obj_ptr, *con_obj_ptr;
	connection_struct **cptr, *con_ptr;


	/* Begin sending object poses. */

	/* Go through connections list. */
        for(con_num = 0, cptr = connection;
            con_num < total_connections;
            con_num++, cptr++
	)
        {
            /* Is connection valid and logged in? */
	    con_ptr = *cptr;
            if(con_ptr == NULL)
		continue;

            object_num = con_ptr->object_num;
	    if(DBIsObjectGarbage(object_num))
		continue;
	    else
		con_obj_ptr = xsw_object[object_num];


            /* Connection due to recieve XSW object updates? */
            if(con_ptr->obj_ud_next <= cur_millitime)
            {
                /* Go through XSW objects list. */
                for(object_count = 0, optr = xsw_object;
                    object_count < total_objects;
                    object_count++, optr++
                )
                {
                    /* Check if objects are valid and in range. */
		    obj_ptr = *optr;
                    if(!Mu3DInRangePtr(con_obj_ptr, obj_ptr,
                        con_obj_ptr->scanner_range
                    ))
                        continue;

                    /*   If object is not owned by the connection,
		     *   then check the visibility of the object.
		     */
                    if(obj_ptr->owner != object_num)
                    {
                        if(DBGetObjectVisibilityPtr(obj_ptr)
                           <= 0.00
                        )
                            continue;
                    }

		    /* Check if marked hidden from connection. */
		    if(obj_ptr->server_options & XSW_OBJF_HIDEFROMCON)
			continue;
       
                    /* Send object position (pose). */
                    NetSendObjectPose(con_num, object_count);
                }

                /* Schedual next object update for connection. */ 
                con_ptr->obj_ud_next = cur_millitime +
                    con_ptr->obj_ud_interval;
            }
        }


        /* Time to send standard XSW object values? */
        if(next.object_values <= cur_millitime)
        {
	    /* Begin sending object values. */

	    /* Go through connections. */
            for(con_num = 0, cptr = connection;
                con_num < total_connections;
                con_num++, cptr++
            )
            {
                /* Is connection valid and logged in? */
		con_ptr = *cptr;
                if(con_ptr == NULL)
                    continue;
                object_num = con_ptr->object_num;
                if(DBIsObjectGarbage(object_num))  
                    continue;
                else
                    con_obj_ptr = xsw_object[object_num];


                /* Go through XSW objects list. */
                for(object_count = 0, optr = xsw_object;
                    object_count < total_objects;
                    object_count++, optr++
                )
                {
                    /* Check if objects are valid and in range. */
		    obj_ptr = *optr;
                    if(!Mu3DInRangePtr(con_obj_ptr, obj_ptr,
                        con_obj_ptr->scanner_range
                    ))
			continue;

		    /* Not owned objects, check for visibility. */
		    if(obj_ptr->owner != object_num)
		    {
			if(DBGetObjectVisibilityPtr(obj_ptr)
			   <= 0.00
			)
			    continue;
		    }

                    /* Check if marked hidden from connection. */
                    if(obj_ptr->server_options & XSW_OBJF_HIDEFROMCON)
                        continue;


                    /* Send standard values. */
                    NetSendObjectValues(con_num, object_count);

                    /* Send sector. */
                    NetSendObjectSect(con_num, object_count);

                    /* Send object maximum values. */
                    NetSendObjectMaximums(con_num, object_count);
                }
            }

            /* Schedual next object values send. */
            next.object_values = cur_millitime +
                sysparm.int_object_values;
        }


        /* Time to send weapon values? */
        if(next.need_weapon_values)
        {
	    /* Send weapon values. */

            /* Go through connections. */
            for(con_num = 0, cptr = connection;
                con_num < total_connections; 
                con_num++, cptr++
            )
            {
                /* Is connection valid and logged in? */
		con_ptr = *cptr;
                if(con_ptr == NULL)
                    continue;
                object_num = con_ptr->object_num;
                if(DBIsObjectGarbage(object_num))
                    continue;
                else
                    con_obj_ptr = xsw_object[object_num];


                /* Go through XSW objects list. */
                for(object_count = 0, optr = xsw_object;
                    object_count < total_objects;
                    object_count++, optr++
                )
                {
                    /* Check if objects are valid and in range. */
		    obj_ptr = *optr;
                    if(!Mu3DInRangePtr(con_obj_ptr, obj_ptr,
                        con_obj_ptr->scanner_range
                    ))
                        continue;

                    /* Not owned objects, check for visibility. */
                    if(obj_ptr->owner != object_num)
                    {
                        if(DBGetObjectVisibilityPtr(obj_ptr)
                           <= 0.00 
                        )  
                            continue;
                    }

                    /* Check if marked hidden from connection. */
                    if(obj_ptr->server_options & XSW_OBJF_HIDEFROMCON)
                        continue;


                    /* Send all weapons values. */
                    for(wep_num = 0; wep_num < obj_ptr->total_weapons; wep_num++)
                        NetSendWeaponValues(
                            con_num,
                            object_count,
                            wep_num
                        );

                    /* Send tractor beam lock. */
                    for(trac_obj_num = 0;
                        trac_obj_num < obj_ptr->total_tractored_objects;
                        trac_obj_num++
                    )
                        NetSendTractorBeamLock(
                            con_num,
                            object_count,
                            obj_ptr->tractored_object[trac_obj_num]
                        );
                }
            }

            /* Reset need next weapon values sent. */
            next.need_weapon_values = 0;
        }


        return(0);
}



/*
 *      Handles an XSW extended command.
 *
 *      (This function called by NetHandleConnection() as needed.)
 */
int NetHandleExtCmd(int condescriptor, char *arg)
{
	char *strptr;
	int ext_cmd;
        char ext_arg[CS_DATA_MAX_LEN];


	/* condescriptor and arg assumed valid. */


        /* Get command and argument. */          
        ext_cmd = StringGetNetCommand(arg);
        if(ext_cmd < 0)
            return(-1);

	strptr = StringGetNetArgument(arg);
	if(strptr == NULL)
	    return(-1);
        strncpy(ext_arg, strptr, CS_DATA_MAX_LEN);
        ext_arg[CS_DATA_MAX_LEN - 1] = '\0';
            

        /* ****************************************************** */
        /* Handle extended command. */

        switch(ext_cmd)
        {
	  /* System and enviroment. */
          case SWEXTCMD_SETOCSN:
            NetHandleSetOCSN(condescriptor, ext_arg);
            break;

	  case SWEXTCMD_SETUNITS:
	    NetHandleSetUnits(condescriptor, ext_arg);
            break;


          /* Standards. */
          case SWEXTCMD_STDOBJVALS:
            NetHandleObjectValues(condescriptor, ext_arg);
            break;

          case SWEXTCMD_STDOBJMAXS:
            /* Server does not accept set object maximums. */
            break;
        
          case SWEXTCMD_STDWEPVALS:
            /* Server does not accept set weapon values. */
            break;
            

          /* Set. */  
          case SWEXTCMD_SETOBJNAME:
            /* Server does not accept set names. */
            break;
   
          case SWEXTCMD_SETOBJSECT:
            NetHandleObjectSect(condescriptor, ext_arg);
            break;

         case SWEXTCMD_SETFOBJSECT:
	    /* Handle this as SWEXTCMD_SETOBJSECT. */
            NetHandleObjectSect(condescriptor, ext_arg);
            break;

          case SWEXTCMD_SETTHROTTLE:
            NetHandleObjectThrottle(condescriptor, ext_arg);
            break;

          case SWEXTCMD_SETWEAPON:
            NetHandleSelectWeapon(condescriptor, ext_arg);
            break;

          case SWEXTCMD_SETINTERCEPT:
            NetHandleSetIntercept(condescriptor, ext_arg);
            break;

          case SWEXTCMD_SETWEPLOCK:
            NetHandleSetWeaponsLock(condescriptor, ext_arg);
            break;

          case SWEXTCMD_SETSHIELDS:
            NetHandleSetShields(condescriptor, ext_arg);
            break;

          case SWEXTCMD_SETDMGCTL:
            NetHandleSetDmgCtl(condescriptor, ext_arg);
            break;

          case SWEXTCMD_SETCLOAK:
            NetHandleSetCloak(condescriptor, ext_arg);
            break;

          case SWEXTCMD_SETSHIELDVIS:
            /* Server does not handle this command. */
            break;

          case SWEXTCMD_SETLIGHTING:
            NetHandleLighting(condescriptor, ext_arg);
            break;

	  case SWEXTCMD_SETCHANNEL:
	    NetHandleSetChannel(condescriptor, ext_arg);
	    break;

	  case SWEXTCMD_SETSCORE:
	    NetHandleScore(condescriptor, ext_arg);
            break;

          case SWEXTCMD_SETENGINE:
            NetHandleSetEngine(condescriptor, ext_arg);
            break;


          /* Requests. */
          case SWEXTCMD_REQNAME:
            NetHandleObjectName(condescriptor, ext_arg);
            break;

          case SWEXTCMD_REQSECT:
            NetHandleReqObjectSect(condescriptor, ext_arg);
            break;


          /* Notifies. */
          case SWEXTCMD_NOTIFYHIT:
            break;

	  case SWEXTCMD_NOTIFYDESTROY:
	    break;


          /* Actions. */
          case SWEXTCMD_FIREWEAPON:
            NetHandleFireWeapon(condescriptor, ext_arg);
            break;

          case SWEXTCMD_TRACTORBEAMLOCK:
            NetHandleTractorBeamLock(condescriptor, ext_arg);
            break;

	  case SWEXTCMD_HAIL:
	    NetHandleHail(condescriptor, ext_arg);
	    break;

	  case SWEXTCMD_COMMESSAGE:
	    NetHandleComMesg(condescriptor, ext_arg);
	    break;

          case SWEXTCMD_WORMHOLEENTER:
            NetHandleWormHoleEnter(condescriptor, ext_arg);
            break;

          case SWEXTCMD_ELINKENTER:
            NetHandleELinkEnter(condescriptor, ext_arg);
            break;

	  case SWEXTCMD_WEPDISARM:
            NetHandleWeaponDisarm(condescriptor, ext_arg);
            break;


	  /* Economy. */
	  case SWEXTCMD_ECO_REQVALUES:
            NetHandleReqValues(condescriptor, ext_arg);
	    break;

	  case SWEXTCMD_ECO_SETVALUES:
	    break;

	  case SWEXTCMD_ECO_SETPRODUCTVALUES:
	    break;

	  case SWEXTCMD_ECO_BUY:
	    NetHandleEcoBuy(condescriptor, ext_arg);
	    break;

	  case SWEXTCMD_ECO_SELL:
	    NetHandleEcoSell(condescriptor, ext_arg);
	    break;



          default:
            break;
        }   


        return(0);
}


/*
 *      Checks and processes incoming network data on
 *      connection condescriptor.
 *
 *	Warning condescriptor is assumed to be valid.
 */
int NetManageRecvConnection(int condescriptor)
{
	int socket, bytes_read;

        char recvbuf[CS_DATA_MAX_BACKLOG];
        int recvbuf_cnt;
	char *recvbuf_ptr;

        char workbuf[CS_DATA_MAX_LEN];
        int workbuf_cnt;
        char *workbuf_ptr;

        char arg[CS_DATA_MAX_LEN];
        int command;

        char text[CS_DATA_MAX_LEN];


        /* Get socket (condescriptor is assumed valid). */
        socket = connection[condescriptor]->socket;
 

        /* Is socket valid and contains data to be read? */
        if(!NetIsSocketReadable(socket, NULL))
            return(0);

        /* Recieve incoming data. */
        bytes_read = recv(socket, recvbuf, CS_DATA_MAX_BACKLOG, 0);
        if(bytes_read == 0)
	{
	    /*   When polling of socket says there are bytes to be
	     *   read and recv() returns 0, it implies that the
	     *   socket has died.
	     */

            sprintf(
		text,
		"Connection %i: Socket has died.",
		condescriptor
            );
            if(sysparm.log_errors)
                LogAppendLineFormatted(fname.primary_log, text);


	    /*   Explicitly set connection's socket to -1 and close
	     *   the connection.
	     */
	    connection[condescriptor]->socket = -1;
	    NetCloseConnection(condescriptor);


            return(-1);
	}

        /* Recieve error? */
        if(bytes_read < 0)
        {
            /* Handle error. */ 
            switch(errno)
            {
	      /* Invalid descriptor. */
              case EBADF:
		if(socket > -1)
		{
                    /* Log lost connection. */
                    sprintf(
			text,
 "NetManageRecvConnection(): Connection %i: Socket invalid and no longer used.",
                        condescriptor
                    );
                    if(sysparm.log_errors)
                        LogAppendLineFormatted(fname.primary_log, text);

                    /* Close connection. */
                    NetCloseConnection(condescriptor);

		    return(-1);
		}
		break;

              default:
                sprintf(
		    text,
 "NetManageRecvConnection(): recv(): Unknown error `%i' on descriptor `%i'.",
                    errno, socket
                );
                if(sysparm.log_errors)
                    LogAppendLineFormatted(fname.primary_log, text);

                return(-1);
		break;
	    }
        }


        /* ************************************************************* */

        /* bytes_read cannot be greater than CS_DATA_MAX_BACKLOG. */
        if(bytes_read > CS_DATA_MAX_BACKLOG)
            bytes_read = CS_DATA_MAX_BACKLOG;

        /* Increment bytes_received for this condescriptor. */
        connection[condescriptor]->bytes_recieved += bytes_read;


        /* Set null character at the end of recvbuf just to be safe. */
        recvbuf[CS_DATA_MAX_BACKLOG - 1] = '\0';
            
              
        /* *************************************************************** */
        /* Begin parsing recieve buffer. */

        /* Reset recieve buffer count and pointer. */
        recvbuf_cnt = 0;
        recvbuf_ptr = recvbuf;

        while(recvbuf_cnt < bytes_read)
        {
            /* Reset work buffer count and pointer. */
            workbuf_cnt = 0;
            workbuf_ptr = workbuf;
            
            /* Fetch data for workbuf. */
            while(1)
            {
                if(workbuf_cnt >= CS_DATA_MAX_LEN)
                {
                    /* Increment recvbuf count and ptr to next delimiter. */
                    while(recvbuf_cnt < bytes_read)
                    {
                        if((*recvbuf_ptr == '\n') ||
                           (*recvbuf_ptr == '\r') ||
                           (*recvbuf_ptr == '\0')
                        )
                        {
                            recvbuf_cnt++;
			    recvbuf_ptr++;
                            break;
                        }
                        recvbuf_cnt++;
                        recvbuf_ptr++;
                    }
                    /*   Null terminating character for workbuf will
                     *   be added farther below.
                     */
                    break;
                }

                /* End of recieve buffer data reached? */
                else if(recvbuf_cnt >= bytes_read)
                {
                    *workbuf_ptr = '\0';
                    break;
                }

                /* Deliminator in recieve buffer encountered? */
                else if((*recvbuf_ptr == '\n') ||
                        (*recvbuf_ptr == '\r') ||
                        (*recvbuf_ptr == '\0')
                )
                {   
                    *workbuf_ptr = '\0';

                    recvbuf_cnt++;
                    recvbuf_ptr++;

                    break;
                }

                /* Copy data from recieve buffer to work buffer normally. */
                else
                {
                    *workbuf_ptr = *recvbuf_ptr;
                
                    recvbuf_cnt++;
                    recvbuf_ptr++;
                        
                    workbuf_cnt++;
                    workbuf_ptr++;
                }
            }

            /* Skip if workbuf is empty. */
            if(*workbuf == '\0')
                continue;

            /* Set null terminating character for workbuf. */
            workbuf[CS_DATA_MAX_LEN - 1] = '\0';


            /* Get command command. */
	    command = StringGetNetCommand(workbuf);
            if(command < 0)
                continue;

            /* Get argument arg. */
            strncpy(arg, StringGetNetArgument(workbuf), CS_DATA_MAX_LEN);
            arg[CS_DATA_MAX_LEN - 1] = '\0';
                 
         
            /* See which net command handling function to call. */
            switch(command)
            {
              case CS_CODE_LOGIN:
                NetHandleLogin(condescriptor, arg);
                break;

              case CS_CODE_LOGOUT:
                NetHandleLogout(condescriptor);
                break;

              case CS_CODE_WHOAMI:
                NetHandleWhoAmI(condescriptor, arg);
                break;

              case CS_CODE_REFRESH:
                NetHandleRefresh(condescriptor, arg);
                break;

              case CS_CODE_INTERVAL:
                NetHandleSetInterval(condescriptor, arg);
                break;

              case CS_CODE_IMAGESET:
                NetHandleSetImageSet(condescriptor, arg);
                break;

              case CS_CODE_SOUNDSET:
                NetHandleSetSoundSet(condescriptor, arg);
                break;

              case CS_CODE_PLAYSOUND:
                /* server dosen't play sounds. */
                break;


              case CS_CODE_LIVEMESSAGE:
                NetHandleLiveMessage(condescriptor, arg);
                break;

              case CS_CODE_SYSMESSAGE:
                /* Server does not handle this. */
                break;

              case CS_CODE_LITERALCMD:
                CmdHandleInput(condescriptor, arg);
                break;


              case CS_CODE_CREATEOBJ:
                /* Server doesn't accept this command. */
                break;

              case CS_CODE_RECYCLEOBJ:
                /* Server dosen't accept this command. */
                break;

              case CS_CODE_POSEOBJ:
                NetHandleObjectPose(condescriptor, arg);
                break;

              case CS_CODE_FORCEPOSEOBJ:
                /* Server does not accept this. */
                break;

              case CS_CODE_EXT:
                NetHandleExtCmd(condescriptor, arg);
                break;

              /* Unknown command. */
              default:
/*
fprintf(stderr, "Recieved unsupported command %i.\n", command);
fprintf(stderr, "Raw text: `%s'\n", workbuf);
 */
                connection[condescriptor]->errors_recieved++;
                break;
            }


	    /*   The call to the net handling function may have reallocated
             *   the connection pointers (espessially a call to
	     *   CmdHandleInput()) so we need to test if condescriptor is
	     *   still valid.
	     */
	    if(condescriptor >= total_connections)
		break;
        }


        return(0);
}


/*
 *      Procedure to check all connections for incoming network
 *      data and process the data.
 */
int NetManageRecv()
{
        int i;
	connection_struct *con_ptr;


        for(i = 0; i < total_connections; i++)
	{
	    con_ptr = connection[i];
	    if(con_ptr == NULL)
		continue;

            if(con_ptr->socket < 0)
		continue;

	    /* Note: The functions that NetManageRecvConnection() calls
	     * may cause the connections to be deallocated.
	     * If they are, global total_connections would have been reset
	     * to 0 and this loop would exit.
	     */
            NetManageRecvConnection(i);
	}


        return(0);
}                   
