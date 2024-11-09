/*
                                 AUX Pipes

	Functions:

	int AUXConIsAllocated(int n)
	int AUXConInitialize(
		int socket,
		int format
	)

	int AUXConSend(aux_connection_struct *ac, char *sndbuf)
	int AUXConSendStats(int n)

	int AUXHandleLogin(aux_connection_struct *ac, char *arg)
	int AUXHandleSegment(aux_connection_struct *ac, char *buf)
	int AUXConHandleRecv(int n)

	int AUXConManageAll()
	void AUXConClose(int n)
	void AUXConDeleteAll()

	int AUXConManageNewConnections(int socket)

	---

 */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "../include/unvmatch.h"
#include "../include/unvmath.h"
#include "../include/netio.h"

#include "swserv.h"
#include "siteban.h"
#include "../include/auxstatcodes.h"


int AUXHandleLogin(aux_connection_struct *ac, char *arg);
int AUXHandleSegment(aux_connection_struct *ac, char *buf);


/*
 *	Checks if aux_connection_num is allocated.
 */
int AUXConIsAllocated(int n)
{
	if((aux_connection == NULL) ||
           (n < 0) ||
	   (n >= total_aux_connections)
	)
	    return(0);
	else if(aux_connection[n] == NULL)
	    return(0);
	else
	    return(1);
}

/*
 *	Allocates a new aux connection structure.
 *
 *	Returns its index number of -1 on error.
 */
int AUXConInitialize(
        int socket,
        int format
)
{
	int i, n;
	aux_connection_struct **ptr, *ac_ptr;


	/* Sanitize total. */
	if(total_aux_connections < 0)
	    total_aux_connections = 0;


	/* Search for available AUX pipe. */
	for(i = 0, ptr = aux_connection;
            i < total_aux_connections;
            i++, ptr++
        )
	{
	    if(*ptr == NULL)
		break;

	    if((*ptr)->socket < 0)
		break;
	}
	if(i < total_aux_connections)
	{
	    /* Got available pointer. */
	    n = i;
	}
	else
	{
	    /* Need to allocate more pointers. */
	    n = total_aux_connections;
	    total_aux_connections++;

	    aux_connection = (aux_connection_struct **)realloc(
		aux_connection,
		total_aux_connections * sizeof(aux_connection_struct *)
	    );
	    if(aux_connection == NULL)
	    {
		total_aux_connections = 0;
		return(-1);
	    }

	    /* Reset new pointer to NULL so it gets allocated below. */
	    aux_connection[n] = NULL;
	}

	/* Allocate structure as needed. */
	if(aux_connection[n] == NULL)
	{
	    aux_connection[n] = (aux_connection_struct *)calloc(
		1,
		sizeof(aux_connection_struct)
	    );
	    if(aux_connection[n] == NULL)
		return(-1);
	}


	ac_ptr = aux_connection[n];

	/* Set new values. */
	ac_ptr->socket = socket;
	ac_ptr->format = format;
	ac_ptr->logged_in = 0;
	ac_ptr->next = cur_millitime;

        ac_ptr->conhost[0] = '\0';
        ac_ptr->contime = cur_systime;

	ac_ptr->bytes_recieved = 0;
	ac_ptr->bytes_sent = 0;
	ac_ptr->errors_recieved = 0;
	ac_ptr->errors_sent = 0;


	return(n);
}

/*
 *	Sends data in sndbuf to AUX connection ac.
 */
int AUXConSend(aux_connection_struct *ac, char *sndbuf)
{
        int socket, len;
        int bytes_written;
	int error_level;


	if((ac == NULL) ||
           (sndbuf == NULL)
	)
	    return(-1);

	socket = ac->socket;


	/* Check if socket is valid and writeable. */
	if(NetIsSocketWritable(socket, &error_level))
	{
            len = strlen(sndbuf);

	    bytes_written = send(
	        socket,
	        sndbuf,
	        len,
	        0
	    );
	    if(bytes_written == 0)
	    {
	        return(bytes_written);
	    }
	    else if(bytes_written < 0)
	    {
                ac->errors_sent++;

                return(bytes_written);
	    }

	    ac->bytes_sent += bytes_written;
	}
	else
	{
            ac->errors_sent++;
            switch(error_level)
            {
              case 3:
                return(-1);
                break;

              default:
		return(-1);
                break;
	    }
	}


	return(bytes_written);
}

/*
 *	Procedure to send all server stats to all AUX pipes.
 */
int AUXConSendStats(aux_connection_struct *ac)
{
	int i, status;
	char buf[AUXSTAT_MAX_LEN];

	int total_con, total_guest_con;
	swserv_memory_stats_struct m;
	time_t uptime;


        if(ac == NULL) 
            return(-1);


	/* Calculate connections. */
	for(i = 0, total_con = 0, total_guest_con = 0;
	    i < total_connections;
            i++
	)
	{
	    if(connection[i] == NULL)
		continue;
	    if(connection[i]->socket < 0)
		continue;

	    if(connection[i]->is_guest)
	        total_guest_con++;

	    total_con++;
	}

	/* Get memory stats. */
	SWServGetMemoryStats(&m);

	/* Up time (use first incoming socket). */
	if(IncomingSocketIsAllocated(0))
	    uptime = cur_systime - incoming_socket[0]->start_time;
	else
	    uptime = 0;


	/* Format buffer. */
	sprintf(buf,
"%s: %i %i\n\
%s: %ld %ld %ld\n\
%s: %ld %ld\n\
%s: %s\n\
%s: %i\n\
%s: %i\n\
%s: %ld\n",
		STAT_PFX_CONNECTIONS, total_con, total_guest_con,
		STAT_PFX_MEMORY, m.total, m.con, m.obj,
		STAT_PFX_NEXT, (next.unv_save - cur_systime),
		    (next.stats_export - cur_systime),
		STAT_PFX_TITLE, unv_head.title,
		STAT_PFX_OBJECTS, total_objects,
		STAT_PFX_PID, root_pid,
		STAT_PFX_UPTIME, uptime
	);


	/* Send. */
	status = AUXConSend(ac, buf);


	return(status);
}


/*
 *	Handles a login.
 */
int AUXHandleLogin(aux_connection_struct *ac, char *arg)
{
	char *strptr;
        long object_num;
        xsw_object_struct *obj_ptr;

	char name[AUXSTAT_SEGMENT_MAX];
	char password[AUXSTAT_SEGMENT_MAX];
	char sndbuf[AUXSTAT_SEGMENT_MAX + AUXSTAT_SEGMENT_MAX + 256];
	char welcome[AUXSTAT_SEGMENT_MAX + 256];


	if((ac == NULL) ||
           (arg == NULL)
	)
	    return(-1);


	/*
	 *	sname;spassword
	 */

	/* Search for delimiter. */
	strptr = strchr(arg, CS_STRING_DELIMINATOR_CHAR);
	if(strptr == NULL)
	    return(-1);

        strncpy(password, strptr + 1, AUXSTAT_SEGMENT_MAX);
        password[AUXSTAT_SEGMENT_MAX - 1] = '\0';

        strncpy(name, arg, AUXSTAT_SEGMENT_MAX);
        name[AUXSTAT_SEGMENT_MAX - 1] = '\0';
	strptr = strchr(name, CS_STRING_DELIMINATOR_CHAR);
	if(strptr != NULL)
	    *strptr = '\0';

	StringStripSpaces(name);
	StringStripSpaces(password);


	/* Does name specify a guest login. */
	if(sysparm.allow_guest &&
           !strcasecmp(name, sysparm.guest_login_name)
        )
        {
	    /* Do nothing for Guest logins. */
	}
	else
	{
	    /* Find player object by the given name. */
	    object_num = MatchObjectByName(
		xsw_object, total_objects, name, XSW_OBJ_TYPE_PLAYER
	    );
            if(DBIsObjectGarbage(object_num))
            {
		/* No such player object. */
                sprintf(sndbuf, "%s: %s\n",
		    STAT_PFX_MESSAGE,
		    sysparm.mesg_wrong_login
	        );
                AUXConSend(ac, sndbuf);
		return(-1);
	    }
	    else
	    {
		/* Found player object. */

		obj_ptr = xsw_object[object_num];

		/* Check password. */
		if(CryptHandleVerify(password, obj_ptr->password) < 1)
                {
                    /* Wrong password. */
                    sprintf(sndbuf, "%s: %s\n",
                        STAT_PFX_MESSAGE,
                        sysparm.mesg_wrong_login
                    );
                    AUXConSend(ac, sndbuf);
                    return(-1);
		}
		else
		{
		    /* Password matched. */

		    /* Log in connection. */
		    ac->logged_in = 1;

                    /* Print welcome message. */
		    strncpy(welcome, sysparm.mesg_welcome, AUXSTAT_SEGMENT_MAX);
		    welcome[AUXSTAT_SEGMENT_MAX - 1] = '\0';
                    substr(welcome, "%title", unv_head.title);
                    substr(welcome, "%name", obj_ptr->name);

                    sprintf(sndbuf, "%s: %s\n",
                        STAT_PFX_MESSAGE,
                        welcome
                    );
                    AUXConSend(ac, sndbuf);
		}
	    }
	}




	return(0);
}


/*
 *	Manages a segment of data, called by AUXConHandleRecv().
 */
int AUXHandleSegment(aux_connection_struct *ac, char *buf)
{
        char *strptr;
        char parm[AUXSTAT_SEGMENT_MAX];
        char val[AUXSTAT_SEGMENT_MAX];


	if((ac == NULL) ||
	   (buf == NULL)
	)
	    return(-1);


        /* Search for parmameter and value delimiter. */  
        strptr = strchr(buf, ':');
        if(strptr == NULL)
        {
            parm[0] = '\0';
            
            strncpy(val, buf, AUXSTAT_SEGMENT_MAX);
            val[AUXSTAT_SEGMENT_MAX - 1] = '\0';
        }
        else
        {
            strncpy(parm, buf, AUXSTAT_SEGMENT_MAX);
            parm[AUXSTAT_SEGMENT_MAX - 1] = '\0';
        
            strptr = strchr(parm, ':');
            if(strptr != NULL)
                *strptr = '\0';

            strncpy(val, strptr + 1, AUXSTAT_SEGMENT_MAX);
            val[AUXSTAT_SEGMENT_MAX - 1] = '\0';
        }
        StringStripSpaces(val);
        StringStripSpaces(parm);


        /* Login. */
        if(!strcasecmp(parm, STAT_PFX_LOGIN))
        {
	    AUXHandleLogin(ac, val);
        }


	return(0);
}

/*
 *	Handles incoming data on AUX connection ac.
 *
 *	Returns 0 or positive on success, -1 on error,
 *	and -3 on disconnected socket.
 */
int AUXConHandleRecv(aux_connection_struct *ac)
{
	int socket;
	int bytes_read;
	char buf[AUXSTAT_MAX_LEN];
        char *buf_ptr;
        int buf_cnt;  

        char workbuf[AUXSTAT_SEGMENT_MAX];
        char *workbuf_ptr;
        int workbuf_cnt;  


	if(ac == NULL)
	    return(-1);

        socket = ac->socket;


        /* Is socket valid and contains data to be read? */
        if(!NetIsSocketReadable(socket, NULL))
            return(0);

        /* Recieve incoming data. */
        bytes_read = recv(socket, buf, AUXSTAT_MAX_LEN, 0);
        if(bytes_read == 0)
        {
            /*   When polling of socket says there are bytes to be
             *   read and recv() returns 0, it implies that the
             *   socket has died.
             */
            return(-3);
        }
        /* Recieve error? */
        if(bytes_read < 0)
        {
            /* Handle error. */
            switch(errno) 
            {
              /* Invalid descriptor. */
              case EBADF:
		ac->errors_recieved++;
                return(-3);
                break;

              /* Not a socket. */
              case ENOTSOCK:
                ac->errors_recieved++;
                return(-3);
                break;

              /* Operation would block a non-blocking socket. */
              case EWOULDBLOCK:
                /* Not an error. */
                return(0); 
                break;
        
              /* Segmentation fault. */
              case EFAULT:
                ac->errors_recieved++;
                return(-3);
                break;

             default:
                ac->errors_recieved++;
                return(-3);
                break;
            }
        }

	/* Increment bytes recieved. */
        ac->bytes_recieved += bytes_read;


        /* Null terminate. */
        if(bytes_read > AUXSTAT_MAX_LEN)
            bytes_read = AUXSTAT_MAX_LEN;

        buf[bytes_read - 1] = '\0';


        /* Begin parsing and handling. */
        buf_cnt = 0;
        buf_ptr = buf;

        while(buf_cnt < bytes_read)
        {
            /* Reset work buffer count and pointer. */
            workbuf_cnt = 0;
            workbuf_ptr = workbuf;

            /* Begin fetching data from recvbuf to workbuf. */
            while(1)
            {
                /* Work buffer overflowed? */
                if(workbuf_cnt >= AUXSTAT_SEGMENT_MAX)
                {
                    /* Increment buf_cnt to next delimiter char. */
                    while(buf_cnt < bytes_read)
                    {      
                        if((*buf_ptr == '\n') ||
                           (*buf_ptr == '\r') ||
                           (*buf_ptr == '\0')
                        )
                        {
                            buf_cnt++;
                            buf_ptr++;
                            break;
                        }
                        buf_cnt++;
                        buf_ptr++;
                    }
                    /*   Null terminating character for workbuf will
                     *   be added farther below.
                     */
                    break;
                }

                /* End of buffer reached? */
                else if(buf_cnt >= bytes_read)
                {
                    *workbuf_ptr = '\0';
            
                    /*   Copy workbuf_cnt to net_parms.co_data for
                     *   processing in the next call to this function.
                     */
/*
                    net_parms.co_data_len = MIN(
                        workbuf_cnt,
                        AUXSTAT_SEGMENT_MAX
                    );
                    
                    if(net_parms.co_data_len > 0)
                        memcpy(
                            net_parms.co_data,  
                            workbuf,
                            net_parms.co_data_len
                        );
 */
                     /*   Return, don't continue. This is so that the
                     *   the carry over buffer is not reset and its
                     *   values just set above get carryed over to
                     *   the next call to this function.
                     */
                    return(0);

                    break;
                }

                /* Deliminator in recieve buffer encountered? */
                else if((*buf_ptr == '\n') ||
                        (*buf_ptr == '\r') ||
                        (*buf_ptr == '\0')
                )
                {
                    *workbuf_ptr = '\0';

                    buf_cnt++;
                    buf_ptr++;

                    break;
                }

                /* Copy data from buffer to work buffer normally. */
                else
                {
                    *workbuf_ptr = *buf_ptr;

                    buf_cnt++;
                    buf_ptr++;

                    workbuf_cnt++;
                    workbuf_ptr++;
                }
            }

            /* Skip if workbuf is empty. */
            if(*workbuf == '\0')
                continue;

            /* Set null terminating character for workbuf. */
            workbuf[AUXSTAT_SEGMENT_MAX - 1] = '\0';


	    /* Handle this segment. */
	    AUXHandleSegment(ac, workbuf);
        }


	return(0);
}


int AUXConManageAll()
{
	int i, status;
	aux_connection_struct **ptr, *ac_ptr;


	for(i = 0, ptr = aux_connection;
            i < total_aux_connections;
            i++, ptr++
	)
	{
	    ac_ptr = *ptr;
	    if(ac_ptr == NULL)
		continue;

	    if(ac_ptr->socket < 0)
		continue;

	    /* Handle by format. */
	    switch(ac_ptr->format)
	    {
	      default:	/* AUX_CON_FORMAT_STANDARD */

		/* Handle incoming data. */
		status = AUXConHandleRecv(ac_ptr);
		if(status == -1)
		{
		    /* General error. */
		    AUXConClose(i);
		    continue;
		}
		else if(status == -3)
		{
		    /* Socket has died, need to close connection. */
		    AUXConClose(i);
		    continue;
		}

		/* Next send? */
		if(ac_ptr->next <= cur_millitime)
		{
		    AUXConSendStats(ac_ptr);

		    ac_ptr->next = cur_millitime +
			sysparm.int_aux_con_stats;
		}
		break;
	    }
	}


	return(0);
}




/*
 *	Close AUX connection n.
 */
void AUXConClose(int n)
{
        aux_connection_struct *ac_ptr;


        if(AUXConIsAllocated(n))
            ac_ptr = aux_connection[n];
        else
            return;


	if(ac_ptr->socket > -1)
	    close(ac_ptr->socket);
	ac_ptr->socket = -1;

	ac_ptr->format = AUX_CON_FORMAT_STANDARD;
	ac_ptr->logged_in = 0;


        /* Notify close of AUX connection. */
        if(!sysparm.console_quiet)
            fprintf(
                stdout,
                "AUX Connection %i: Disconnected.\n",
                n
            );



	return;
}


void AUXConDelete(int n)
{
        if(AUXConIsAllocated(n))
	{
	    AUXConClose(n);

	    free(aux_connection[n]);
	    aux_connection[n] = NULL;
	}

        
        return;
}


void AUXConDeleteAll()
{
	int i;


	for(i = 0; i < total_aux_connections; i++)
	    AUXConDelete(i);

	free(aux_connection);
	aux_connection = NULL;

	total_aux_connections = 0;


	return;
}


int AUXConManageNewConnections(int socket)
{
	int i, n, new_con;	// Dan S: "new" is a reserved word in C++, renamed to new_con
	char sndbuf[AUXSTAT_SEGMENT_MAX];
        char stringa[256];
        aux_connection_struct *ac_ptr;

        int sin_size;
        int new_socket;
        struct sockaddr_in foreign_addr;

	siteban_ip_union ip;



        /* `Answer' new connection. */
        sin_size = sizeof(struct sockaddr_in);
        new_socket = accept(
            socket,
            (struct sockaddr *)&foreign_addr,
            reinterpret_cast<socklen_t *>(&sin_size)
        );
        if(new_socket == -1)
            return(new_socket);

        /* Set socket nonblocking. */
        fcntl(new_socket, F_SETFL, O_NONBLOCK);


        /* Get IP (in nbo). */
        ip.whole = foreign_addr.sin_addr.s_addr;

        /* Is this IP banned? */
        if(SiteBanIsBanned(&ip, 0))
        {
            sprintf(sndbuf,
                "%s: Your address %i.%i.%i.%i has been banned.\n",
                STAT_PFX_MESSAGE,
                ip.part_u8[0],
                ip.part_u8[1],
                ip.part_u8[2],
                ip.part_u8[3]
            );
            send(new_socket, sndbuf, strlen(sndbuf), 0);

            close(new_socket);

            /* Pretend like we didn't do anything. */
            return(0);
        } 


	/* Too many connections? */
	for(i = 0, n = 0; i < total_aux_connections; i++)
	{
	    if(aux_connection[i] == NULL)
		continue;

            if(aux_connection[i]->socket < 0)
                continue;

	    n++;
	}
	if(n >= sysparm.max_aux_connections)
	{
	    sprintf(sndbuf,
		"%s: Maximum of %i AUX connections reached.\n",
		STAT_PFX_MESSAGE,
		sysparm.max_aux_connections
	    );
	    send(new_socket, sndbuf, strlen(sndbuf), 0);

	    close(new_socket);

	    return(-3);
	}


        /* Get a new connection descriptor for new connection. */
        new_con = AUXConInitialize(
	    new_socket,
	    AUX_CON_FORMAT_STANDARD
	);
        if(AUXConIsAllocated(new_con))
        {
            ac_ptr = aux_connection[new_con];
        }
        else
        {
            sprintf(stringa,
                "Error: Unable to allocate new AUX connection structure."
            );
            if(sysparm.log_net)
                LogAppendLineFormatted(fname.primary_log, stringa);

            close(new_socket);
            new_socket = -1;

            return(-3);
        }


        /* Consider it logged in for now. */
/*
        ac_ptr->logged_in = 1;
 */

        strncpy(
            ac_ptr->conhost,
            inet_ntoa(foreign_addr.sin_addr),
            HOST_NAME_MAX
        );
        ac_ptr->conhost[HOST_NAME_MAX - 1] = '\0';


	/* Notify of new AUX connection. */
        if(!sysparm.console_quiet)
            fprintf(
                stdout,
                "AUX Connection %i: Got connection from `%s'.\n",
                new_con,
                ac_ptr->conhost
            );


	/* Send request for login to connection (just once on initial
	 * connect).
	 */
	sprintf(sndbuf, "%s:\n", STAT_PFX_REQUESTLOGIN);
	AUXConSend(ac_ptr, sndbuf);


	return(0);
}
