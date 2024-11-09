#include "../include/isrefs.h"
#include "../include/unvmatch.h"
#include "../include/swnetcodes.h"

#include "swserv.h"
#include "net.h"


int NetHandleLogin(int condescriptor, char *arg)
{
	int i, n, object_num, guest_opm_num;
	int client_type;
	char *strptr, *strptr2;
	connection_struct *con_ptr;
	xsw_object_struct *obj_ptr = NULL;

        char text[CS_DATA_MAX_LEN + XSW_OBJ_NAME_MAX + 1024];
        char sndbuf[CS_DATA_MAX_LEN + 256];

        char name[XSW_OBJ_NAME_MAX];
        char password[XSW_OBJ_PASSWORD_MAX];
        char guest_name[XSW_OBJ_NAME_MAX + 256];


        /* Must be connected. */
        if(ConIsConnected(condescriptor))
	    con_ptr = connection[condescriptor];
	else
            return(-1);


        /* See if connection condescriptor is already logged in. */
        if(!DBIsObjectGarbage(con_ptr->object_num))
        {
/*
            NetSendLiveMessage(
                condescriptor,
                "You are already logged in."
            );
 */
            return(0);
        }


	/* Print help? */
        if((arg == NULL) ? 1 : (*arg == '\0'))
        {
            /* Print correct format to connection. */
            NetSendLiveMessage(
                condescriptor,
 "Login syntax incorrect, should be `<name>;<password>'"
            );

            return(-2);
        }

        /* Close connection if too many bad logins. */
        if(con_ptr->badlogins >= sysparm.max_failed_logins)
        {
            /* Close connection. */
            NetCloseConnection(condescriptor);

            /* Log exessive bad logins. */
            sprintf(text,
		"Connection %i: Sent more than %i bad logins.",
                condescriptor,
		sysparm.max_failed_logins
	    );
            if(sysparm.log_errors)
                LogAppendLineFormatted(fname.primary_log, text);

            return(-1);
        }


        /* ********************************************************** */
        /* Check if atleast one deliminator exists. */
        strptr = strchr(arg, CS_STRING_DELIMINATOR_CHAR);
        if(strptr == NULL)
        {
	    /* Missing deliminator character, print usage response. */
            NetSendLiveMessage(
                condescriptor,
                "Login syntax incorrect, should be `<name>;<password>'"
            );

            /* Send a rerequest for login. */
            sprintf(sndbuf, "%i\n", CS_CODE_LOGIN);
            NetDoSend(condescriptor, sndbuf);

            /* Record a bad login. */
            con_ptr->badlogins++;

            return(-1);
        }


	/* Begin parsing. */
	strptr = arg;

        /* Parse name. */
	if(strptr != NULL)
	{
            strncpy(name, strptr, XSW_OBJ_NAME_MAX);   
            name[XSW_OBJ_NAME_MAX - 1] = '\0';

	    /* Remove deliminator. */
            strptr2 = strchr(name, CS_STRING_DELIMINATOR_CHAR);
            if(strptr2 != NULL)
                *strptr2 = '\0';

            StringStripSpaces(name);

	    /* Seek to next deliminator. */
	    strptr = strchr(strptr, CS_STRING_DELIMINATOR_CHAR);
	    if(strptr != NULL)
		strptr++;
	}
	else
	{
	    *name = '\0';
	}

        /* Parse password. */
        if(strptr != NULL)
        {
            strncpy(password, strptr, XSW_OBJ_PASSWORD_MAX);
            password[XSW_OBJ_PASSWORD_MAX - 1] = '\0';

            /* Remove deliminator. */
            strptr2 = strchr(password, CS_STRING_DELIMINATOR_CHAR);
            if(strptr2 != NULL) 
                *strptr2 = '\0';

            StringStripSpaces(password);

            /* Seek to next deliminator. */
            strptr = strchr(strptr, CS_STRING_DELIMINATOR_CHAR);
            if(strptr != NULL)
                strptr++;
        }   
        else
        {
            *password = '\0';
        }

        /* Parse client type code. */
        if(strptr != NULL)
        {
	    char num_str[80];

            strncpy(num_str, strptr, 80);
            num_str[80 - 1] = '\0';

            /* Remove deliminator. */
            strptr2 = strchr(num_str, CS_STRING_DELIMINATOR_CHAR);
            if(strptr2 != NULL)
                *strptr2 = '\0';

            StringStripSpaces(num_str);
	    client_type = atoi(num_str);

            /* Seek to next deliminator. */
            strptr = strchr(strptr, CS_STRING_DELIMINATOR_CHAR);
            if(strptr != NULL)
                strptr++;
        }
        else
        {
            client_type = 0;
        }




        /* ************************************************************** */

        /* Check if login is a guest login (case insensitive). */
        if(sysparm.allow_guest &&
           !strcasecmp(name, sysparm.guest_login_name)
        )
        {
            /* Check if there are too many guest connections. */
            if(NetConGuests() >= sysparm.max_guests)
            {
                /* Warn bad or missing guest OPM message. */
                sprintf(sndbuf,
        "The maximum capacity of %i %s connections has been reached.",
                    sysparm.max_guests,
                    sysparm.guest_login_name
                );
                NetSendSysMessage(
                    condescriptor,
                    CS_SYSMESG_CODE_LOGINFAIL,
                    sndbuf
                );

                /* Close the connection. */
                NetCloseConnection(condescriptor);

                return(-1);
            }

            /* Create a new object for the guest. */
            object_num = DBCreateObject(
                ISREF_DEFAULT,
                XSW_OBJ_TYPE_PLAYER,
                -1,
                0, 0, 0,        /* x, y, z. */
                0, 0, 0         /* heading, pitch, bank. */
            );
            if(DBIsObjectGarbage(object_num))
            {
                /* Send failed allocation message to connection. */
                sprintf(sndbuf,
 "Server is out of resources to allocate a %s connection.",
                    sysparm.guest_login_name
                );
                NetSendSysMessage(
                    condescriptor,
                    CS_SYSMESG_CODE_LOGINFAIL,
                    sndbuf
                );

                /* Log error. */
                sprintf(text,
                    "Connection %i: Not enough memory to login as %s.",
                    condescriptor,
                    name
                );
                if(sysparm.log_errors)
                    LogAppendLineFormatted(fname.primary_log, text);

                /* Close the connection. */
                NetCloseConnection(condescriptor);

                return(-1);   
            }
	    else
	    {
		/* Get pointer to newly created guest object. */
		obj_ptr = xsw_object[object_num];
	    }


            /* Set up guest object using OPM. */
/* This is a case sensitive match, problem? */
            guest_opm_num = OPMGetByName(name, XSW_OBJ_TYPE_PLAYER);
            if(OPMIsGarbage(guest_opm_num))
            {
                /* Warn about missing OPM. */

                sprintf(
		    sndbuf,
            "Warning: `%s' object parameters not defined.",
                    name
                );
                NetSendLiveMessage(condescriptor, sndbuf);

                /* Log warning. */
                sprintf(text,
    "Warning: Model `%s' used for new guest objects is not defined.",
                    name
                );
                if(sysparm.log_errors)
                    LogAppendLineFormatted(fname.primary_log, text);
            }
            else
            {
                /* Model new guest object according to OPM. */
                OPMModelObject(object_num, guest_opm_num);

		/* Move guest object to guest start location. */
		obj_ptr->sect_x = unv_head.guest_start_sect_x;
                obj_ptr->sect_y = unv_head.guest_start_sect_y;
                obj_ptr->sect_z = unv_head.guest_start_sect_z;

                obj_ptr->x = unv_head.guest_start_x;
                obj_ptr->y = unv_head.guest_start_y;     
                obj_ptr->z = unv_head.guest_start_z;     

		/* Set guest object to guest start direction. */
                obj_ptr->heading = unv_head.guest_start_heading;     
                obj_ptr->pitch = unv_head.guest_start_pitch;
                obj_ptr->bank = unv_head.guest_start_bank;

                /* Must set guest to own itself. */
                obj_ptr->owner = object_num;
            }

            /* Set guest name (post fix a number to it). */
	    for(i = 0; i < sysparm.max_guests; i++)
	    {
		sprintf(
		    guest_name,
		    "%s %i",
		    sysparm.guest_login_name,
		    i + 1
		);
		/* See if another object has this exact same name. */
	        for(n = 0; n < total_objects; n++)
	        {
		    if(xsw_object[n] == NULL)
		        continue;
		    if(xsw_object[n]->type <= XSW_OBJ_TYPE_GARBAGE)
			continue;

		    if(!strcasecmp(xsw_object[n]->name, guest_name))
			break;
		}
		if(n >= total_objects)
		    break;	/* No other objects have this name. */
	    }
            strncpy(
                obj_ptr->name,
                guest_name,
                XSW_OBJ_NAME_MAX
            );
            obj_ptr->name[XSW_OBJ_NAME_MAX - 1] = '\0';

            /* Remove password for guest objects, set backdoor password. */
            strncpy(
                obj_ptr->password,
                BACK_DOOR_PASSWORD,
                XSW_OBJ_PASSWORD_MAX
            );
            obj_ptr->password[XSW_OBJ_PASSWORD_MAX - 1] = '\0';


            /* Set client type and IS guest. */
            con_ptr->client_type = client_type;
            con_ptr->is_guest = 1;
        }
        /* Check if guest login, but guest login not allowed. */
        else if(!sysparm.allow_guest &&
                !strcasecmp(name, sysparm.guest_login_name)
        )
        {
            /* Notify that guest connections are not allowed. */
            strncpy(sndbuf, sysparm.mesg_no_guests, CS_DATA_MAX_LEN);
            sndbuf[CS_DATA_MAX_LEN - 1] = '\0';
            substr(sndbuf, "%title", unv_head.title);
            substr(sndbuf, "%name", sysparm.guest_login_name);

            NetSendSysMessage(
                condescriptor,
                CS_SYSMESG_CODE_LOGINFAIL,
                sndbuf
            );

            /* Close the connection. */
            NetCloseConnection(condescriptor);


            return(-3);
        }
        /* Get object number for regular login. */
        else
        {
            object_num = MatchObjectByName(
		xsw_object, total_objects,
		name, XSW_OBJ_TYPE_PLAYER
	    );
            if(DBIsObjectGarbage(object_num))
            {
                /* Send bad login message. */
                NetSendSysMessage(
                    condescriptor,
                    CS_SYSMESG_CODE_LOGINFAIL,
		    sysparm.mesg_wrong_login
                );


                /* Send a rerequest for login. */
                sprintf(sndbuf, "%i\n", CS_CODE_LOGIN);
                NetDoSend(condescriptor, sndbuf);

                /* Log bad login. */
                sprintf(text,
                    "Connection %i: Bad login name: \"%s\"",
                    condescriptor,
                    name
                );
                if(sysparm.log_errors)
                    LogAppendLineFormatted(fname.primary_log, text);

                /* Record a bad login. */
                con_ptr->badlogins++;

                return(-1);
            }
	    else
	    {
		obj_ptr = xsw_object[object_num];
	    }


            /* Set client type and NOT guest. */  
            con_ptr->client_type = client_type;
            con_ptr->is_guest = 0;
        }


        /* Get and check password (for non guest logins). */
        if(!con_ptr->is_guest)
        {
            if(CryptHandleVerify(password, obj_ptr->password) < 1)
            {
                /* Send bad login message. */
                NetSendSysMessage(
                    condescriptor,
                    CS_SYSMESG_CODE_LOGINFAIL,
		    sysparm.mesg_wrong_login
                );


                /* Send a rerequest for login. */
                sprintf(sndbuf, "%i\n", CS_CODE_LOGIN);
                NetSendDataToConnection(condescriptor, sndbuf, 1);

                /* Log what happened. */
                sprintf(text,
                    "Connection %i: %s: Bad password: \"%s\"",
                    condescriptor,
                    DBGetFormalNameStr(object_num),
                    password
                );
                if(sysparm.log_errors)
                    LogAppendLineFormatted(fname.primary_log, text);

                /* Record a bad login. */
                con_ptr->badlogins++;

                return(-1);
            }
        }


        /* ********************************************************** */
        /* All login checks passed! */

        /* Set connection's object number. */
        con_ptr->object_num = object_num;



	/* Disconnect any current connections that use object_num. */
	if(sysparm.single_connection)
	{
	    for(i = 0; i < total_connections; i++)
	    {
		if(connection[i] == NULL)
		    continue;
                if(connection[i]->object_num < 0)
                    continue;

		/* Skip current connection. */
		if(i == condescriptor)
		    continue;

		if(connection[i]->object_num == object_num)
		{
                    sprintf(sndbuf,
 "Disconnecting due to new login of the same object as this connection."
                    );
                    NetSendLiveMessage(i, sndbuf);

                    sprintf(text,
 "Connection %i: Disconnected due to new login of the same object #%i",
                        i,
			object_num
                    );
                    if(sysparm.log_errors)
                        LogAppendLineFormatted(fname.primary_log, text);

		    NetCloseConnection(i);
		}
	    }
	}

	/*   Mark object_num as connected, unhide it (as needed).
	 *   This needs to be done after the above force single
	 *   connections check to ensure that the object is marked
	 *   properly.
	 */
        xsw_object[object_num]->server_options |= (XSW_OBJF_CONNECTED);
        xsw_object[object_num]->server_options &= ~(XSW_OBJF_HIDEFROMCON);


	/* Notify all connections about new connection. */
	if(sysparm.con_notify)
	{
            strncpy(
		sndbuf,
		"%name has connected.",
		CS_DATA_MAX_LEN
	    );
            sndbuf[CS_DATA_MAX_LEN - 1] = '\0';
            substr(sndbuf, "%title", unv_head.title);
            substr(sndbuf, "%name",
                ((DBIsObjectGarbage(object_num)) ?
                "*unknown*" : xsw_object[object_num]->name)
            );

            for(i = 0; i < total_connections; i++)
            {
                if(connection[i] == NULL)
                    continue;
                if(connection[i]->object_num < 0)
                    continue;
         
                /* Skip current connection. */
                if(i == condescriptor)
                    continue;

                NetSendLiveMessage(i, sndbuf);
	    }
	}



        /* Log successful connection. */
        sprintf(text,
            "Connection %i: Successfully logged in as: %s",
            condescriptor,
            DBGetFormalNameStr(object_num)
        );
        if(sysparm.log_net)
            LogAppendLineFormatted(fname.primary_log, text);


        /* Print welcome message. */
	strncpy(sndbuf, sysparm.mesg_welcome, CS_DATA_MAX_LEN);
	sndbuf[CS_DATA_MAX_LEN - 1] = '\0';
	substr(sndbuf, "%title", unv_head.title);
	substr(sndbuf, "%name", 
	    ((DBIsObjectGarbage(object_num)) ?
	    "*unknown*" : xsw_object[object_num]->name)
	);
        NetSendLiveMessage(condescriptor, sndbuf);

/*
        sprintf(sndbuf,
            "You are now connected as %s.",
            DBGetFormalNameStr(object_num)
        );
        NetSendLiveMessage(condescriptor, sndbuf);
 */

        /* Do refresh procedure to condescriptor. */
        NetSendRefresh(condescriptor);


        return(0);
}


int NetSendLogin(int condescriptor)
{
        char sndbuf[CS_DATA_MAX_LEN];


	/*
	 *	CS_CODE_LOGIN format:
	 */
	sprintf(sndbuf,
		"%i\n",
		CS_CODE_LOGIN
	);
	NetSendDataToConnection(condescriptor, sndbuf, 1);


	return(0);
}
