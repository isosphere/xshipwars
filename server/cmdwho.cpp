#include "../include/unvmatch.h"
#include "swserv.h"
#include "net.h"


#define THIS_CMD_NAME	"who"

/*
 *	Shows players connected.
 */
int CmdWho(int condescriptor, const char *arg)
{
	int i, c, object_num;
	connection_struct **ptr, *con_ptr;

        char sndbuf[CS_DATA_MAX_LEN];


	/* Use default argument if NULL. */
	if(arg == NULL)
	    arg = "";

	/* Skip leading spaces. */
        while(*arg == ' ')
	    arg++;


        /* Print all connections? */
        if(*arg == '\0')
        {
            for(i = 0, c = 0, ptr = connection;
                i < total_connections;
                i++, ptr++
	    )
            {
		con_ptr = *ptr;
                if(con_ptr == NULL)
                    continue;

		if(con_ptr->socket < 0)
		    continue;

                object_num = con_ptr->object_num;
                if(DBIsObjectGarbage(object_num))
                    continue;

                /* Format sndbuf. */
		sprintf(
		    sndbuf,
		    "%s  %i  %s  %s",
                    DBGetFormalNameStr(object_num),
                    con_ptr->client_type,
                    StringFormatTimePeriod(
                        cur_systime - con_ptr->contime
                    ),
		    ((con_ptr->is_guest) ? "Guest" : "")
		);
                sndbuf[CS_DATA_MAX_LEN - 1] = '\0';
                NetSendLiveMessage(condescriptor, sndbuf);

                /* Increment c, connected players. */
                c++;
            }


            /* Print footer. */
            sprintf(
		sndbuf,
		"*** %i player%s connected. ***",
                c,
		((c == 1) ? "" : "s")
            );
            sndbuf[CS_DATA_MAX_LEN - 1] = '\0'; 
            NetSendLiveMessage(condescriptor, sndbuf);
        }
        /* Print all connections with host names. */   
        else if(!strcasecmp(arg, "*"))
        {
            for(i = 0, c = 0, ptr = connection;
                i < total_connections;
                i++, ptr++
            )
            {
		con_ptr = *ptr;
                if(con_ptr == NULL)
                    continue;
                    
                if(con_ptr->socket < 0)
                    continue;

                object_num = con_ptr->object_num;
                if(DBIsObjectGarbage(object_num))
                    continue;

                /* Format sndbuf. */  
                sprintf(
                    sndbuf,
                    "%s  %i  %s  %s",
                    DBGetFormalNameStr(object_num),
                    con_ptr->client_type,
                    con_ptr->conhost,
                    ((con_ptr->is_guest) ? "Guest" : "")
                );
                sndbuf[CS_DATA_MAX_LEN - 1] = '\0';
                NetSendLiveMessage(condescriptor, sndbuf);

                /* Increment c, connected players. */
                c++;
            }

            /* Print footer. */
            sprintf(
                sndbuf,
                "*** %i player%s connected. ***",
                c,
                ((c == 1) ? "" : "s")
            );
            sndbuf[CS_DATA_MAX_LEN - 1] = '\0';
            NetSendLiveMessage(condescriptor, sndbuf);
        }
        /* Print specific player. */
        else
        {
            object_num = MatchObjectByName(
		xsw_object, total_objects,
		arg, XSW_OBJ_TYPE_PLAYER
	    );
            if(DBIsObjectGarbage(object_num))
            {               
                sprintf(sndbuf,
                    "%s: %s: No such player.",
		    THIS_CMD_NAME,
                    arg
                );
                NetSendLiveMessage(condescriptor, sndbuf);   
                return(-1);
            }

            /* Get first connection that referances the object. */
	    i = ConGetByObject(object_num);
	    if(i < 0)
            {
		/* Could not find any connections that referance it. */
                sprintf(sndbuf,  
                    "%s: %s: Not connected.",
		    THIS_CMD_NAME,
                    arg
                );
                NetSendLiveMessage(condescriptor, sndbuf);

                return(-1);
            }
	    else
	    {
		/* Got valid connection, get pointer to it. */
                con_ptr = connection[i];

		sprintf(
		    sndbuf,
		    "%s  %i  %s  %s",
		    DBGetFormalNameStr(object_num),
		    con_ptr->client_type,
		    StringFormatTimePeriod(
			cur_systime - con_ptr->contime
		    ),
		    ((con_ptr->is_guest) ? "Guest" : "")
                );
		sndbuf[CS_DATA_MAX_LEN - 1] = '\0';
		NetSendLiveMessage(condescriptor, sndbuf);
	    }
	}


        return(0);
}
