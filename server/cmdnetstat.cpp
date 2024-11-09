#include "../include/unvmatch.h"
#include "swserv.h" 
#include "net.h"


#define THIS_CMD_NAME	"netstat"        

#define SPACES		"                                 "

static void CmdNetstatPrintStandard(int condescriptor);
static void CmdNetstatPrintListening(int condescriptor);
static void CmdNetstatPrintPlayer(int condescriptor, int object_num);


static void CmdNetstatPrintStandard(int condescriptor)
{
	int i;
	connection_struct **con_ptr;
	aux_connection_struct **ac_ptr;

	char *strptr1, *strptr2;
	char num_str[80];
	char sndbuf[CS_DATA_MAX_LEN + XSW_OBJ_NAME_MAX + HOST_NAME_MAX];


	/* Inputs assumed valid! */


	/* Heading. */
	NetSendLiveMessage(condescriptor,
"Client RX           SX           Foreign Address                 State\
          Player"
	);


	/* Print each line. */
	for(i = 0, con_ptr = connection;
            i < total_connections;
            i++, con_ptr++
	)
	{
	    if(*con_ptr == NULL)
		continue;

	    if((*con_ptr)->socket < 0)
		continue;

	    /* Client type. */
	    strptr1 = &sndbuf[0];
	    *strptr1 = '\0';
	    switch((*con_ptr)->client_type)
	    {
	      default:
		strptr2 = "std";
		break;
	    }
	    strcat(strptr1, strptr2);
	    strcat(strptr1, SPACES);


	    /* Recieved (relative to client). */
	    strptr1 = &sndbuf[7];
	    *strptr1 = '\0';
	    sprintf(num_str, "%ld", (*con_ptr)->bytes_sent);
            strcat(strptr1, num_str);
            strcat(strptr1, SPACES);


            /* Sent (relative to client). */
            strptr1 = &sndbuf[20];
            *strptr1 = '\0';
            sprintf(num_str, "%ld", (*con_ptr)->bytes_recieved);
            strcat(strptr1, num_str);
            strcat(strptr1, SPACES);


	    /* Foreign Address. */
	    strptr1 = &sndbuf[33];
            *strptr1 = '\0';
            strcat(strptr1, (*con_ptr)->conhost);
            strcat(strptr1, SPACES);


	    /* State. */
            strptr1 = &sndbuf[65];
            *strptr1 = '\0';
            if((*con_ptr)->object_num < 0)
                strptr2 = "NEGOTIATING";
            else
                strptr2 = "ESTABLISHED";
            strcat(strptr1, strptr2);
            strcat(strptr1, SPACES);


	    /* Player. */
            strptr1 = &sndbuf[80];
            *strptr1 = '\0';
	    if(!DBIsObjectGarbage((*con_ptr)->object_num))
	    {
		strptr2 = DBGetFormalNameStr((*con_ptr)->object_num);
                strcat(strptr1, strptr2);
                strcat(strptr1, SPACES);

		strptr1 = &sndbuf[130];
		*strptr1 = '\0';
	    }

	    NetSendLiveMessage(condescriptor, sndbuf);
	}


	/* Print AUX Stats sockets. */
        for(i = 0, ac_ptr = aux_connection;
            i < total_aux_connections;
            i++, ac_ptr++
        )
        {
            if(*ac_ptr == NULL)
                continue;
            
            if((*ac_ptr)->socket < 0)
                continue;


            /* Client type. */
            strptr1 = &sndbuf[0];
            *strptr1 = '\0';
            strptr2 = "aux";
            strcat(strptr1, strptr2);   
            strcat(strptr1, SPACES); 


            /* Recieved (relative to client). */
            strptr1 = &sndbuf[7];
            *strptr1 = '\0';
            sprintf(num_str, "%ld", (*ac_ptr)->bytes_sent);
            strcat(strptr1, num_str);
            strcat(strptr1, SPACES);


            /* Sent (relative to client). */
            strptr1 = &sndbuf[20];
            *strptr1 = '\0';
            sprintf(num_str, "%ld", (*ac_ptr)->bytes_recieved);
            strcat(strptr1, num_str);
            strcat(strptr1, SPACES);


            /* Foreign Address. */
            strptr1 = &sndbuf[33];
            *strptr1 = '\0';
            strcat(strptr1, (*ac_ptr)->conhost);
            strcat(strptr1, SPACES);


            /* State. */
            strptr1 = &sndbuf[65];
            *strptr1 = '\0';
            if((*ac_ptr)->logged_in)
                strptr2 = "AUTHENTICATED";
            else
                strptr2 = "ANONYMOUS";
            strcat(strptr1, strptr2);
            strcat(strptr1, SPACES);

            strptr1 = &sndbuf[80];
	    *strptr1 = '\0';


            NetSendLiveMessage(condescriptor, sndbuf);
	}
}


static void CmdNetstatPrintListening(int condescriptor)
{
	int i;
	incoming_socket_struct **is_ptr;
        char *strptr1, *strptr2;
	char num_str[80];
        char sndbuf[CS_DATA_MAX_LEN + XSW_OBJ_NAME_MAX + HOST_NAME_MAX];


        /* Inputs assumed valid! */


        /* Heading. */
        NetSendLiveMessage(condescriptor,
"Type Port     Int"
        );


	/* Print each incoming socket. */
	for(i = 0, is_ptr = incoming_socket;
            i < total_incoming_sockets;
            i++, is_ptr++
	)
	{
	    if(*is_ptr == NULL)
		continue;

            if((*is_ptr)->socket < 0)
                continue;


            /* Type. */
            strptr1 = &sndbuf[0];
            *strptr1 = '\0';
	    switch((*is_ptr)->type)
	    {
              case INCOMING_SOCKET_TYPE_AUXSTATS:
                strptr2 = "aux";
                break;

	      default:	/* INCOMING_SOCKET_TYPE_STANDARD */
                strptr2 = "std";
		break;
	    }
            strcat(strptr1, strptr2);
            strcat(strptr1, SPACES);


	    /* Port. */
            strptr1 = &sndbuf[5];
            *strptr1 = '\0';
            sprintf(num_str, "%i", (*is_ptr)->port_num);
            strcat(strptr1, num_str);
            strcat(strptr1, SPACES);

            /* Interval. */
            strptr1 = &sndbuf[14];
            *strptr1 = '\0';
            sprintf(num_str, "%ld", sysparm.int_new_connection_poll);
            strcat(strptr1, num_str);
            strcat(strptr1, SPACES);

            strptr1 = &sndbuf[30];
            *strptr1 = '\0';

            NetSendLiveMessage(condescriptor, sndbuf);
	}
}

static void CmdNetstatPrintPlayer(int condescriptor, int object_num)
{
	int i, printed, c;
        connection_struct **con_ptr;
	char *strptr1, *strptr2;

        char num_str[80];
        char sndbuf[CS_DATA_MAX_LEN + XSW_OBJ_NAME_MAX + HOST_NAME_MAX];

	xsw_object_struct *obj_ptr;


        /* Inputs assumed valid! */

	obj_ptr = xsw_object[object_num];


        /* Heading. */
        NetSendLiveMessage(condescriptor,
"Client RX           SX           Foreign Address                 State\
          Player"
        );


	/* Get object's connection. */
	for(i = 0, printed = 0, c = -1, con_ptr = connection;
            i < total_connections;
            i++, con_ptr++
	)
	{
	    if(*con_ptr == NULL)
		continue;

	    if((*con_ptr)->socket < 0)
		continue;

	    if((*con_ptr)->object_num != object_num)
		continue;


            /* Client type. */
            strptr1 = &sndbuf[0];
            *strptr1 = '\0';
            switch((*con_ptr)->client_type)
            {
              default:
                strptr2 = "std";
                break;   
            }
            strcat(strptr1, strptr2);
            strcat(strptr1, SPACES);
        
            
            /* Recieved (relative to client). */
            strptr1 = &sndbuf[7];
            *strptr1 = '\0';
            sprintf(num_str, "%ld", (*con_ptr)->bytes_sent);
            strcat(strptr1, num_str);
            strcat(strptr1, SPACES);
                
                
            /* Sent (relative to client). */
            strptr1 = &sndbuf[20];
            *strptr1 = '\0';
            sprintf(num_str, "%ld", (*con_ptr)->bytes_recieved);
            strcat(strptr1, num_str);
            strcat(strptr1, SPACES);

            /* Foreign Address. */
            strptr1 = &sndbuf[33];
            *strptr1 = '\0';
            strcat(strptr1, (*con_ptr)->conhost);
            strcat(strptr1, SPACES);
            
            
            /* State. */
            strptr1 = &sndbuf[65];
            *strptr1 = '\0';
            if((*con_ptr)->object_num < 0)
                strptr2 = "NEGOTIATING";
            else
                strptr2 = "ESTABLISHED";
            strcat(strptr1, strptr2);
            strcat(strptr1, SPACES);


            /* Player (assumed valid). */
            strptr1 = &sndbuf[80];
            *strptr1 = '\0';
            strptr2 = DBGetFormalNameStr(object_num);
            strcat(strptr1, strptr2);
            strcat(strptr1, SPACES);

            strptr1 = &sndbuf[130];
            *strptr1 = '\0';


            NetSendLiveMessage(condescriptor, sndbuf);

	    printed++;
	}

	/* Nothing printed? */
	if(!printed)
            NetSendLiveMessage(condescriptor, "*** Not connected. ***");
}


/*
 *      Print network statistics.
 */
int CmdNetstat(int condescriptor, const char *arg)
{
        int con_obj_num, obj_num;
        xsw_object_struct *con_obj_ptr;
        connection_struct *con_ptr;

	char name[XSW_OBJ_NAME_MAX];

	char sndbuf[CS_DATA_MAX_LEN + XSW_OBJ_NAME_MAX];
	char larg[CS_DATA_MAX_LEN];


        /* Get connection's object number (assumed valid). */
        con_ptr = connection[condescriptor];
        con_obj_num = con_ptr->object_num;
        con_obj_ptr = xsw_object[con_obj_num];

        /* Check if object's permission allows save. */
        if(con_obj_ptr->permission.uid > ACCESS_UID_NETSTAT)
        {
            sprintf(
                sndbuf, 
                "%s: Permission denied: Requires access level %i.",
                THIS_CMD_NAME, ACCESS_UID_NETSTAT
            );
            NetSendLiveMessage(condescriptor, sndbuf);

            return(-1);
        }

	/* Argument given? */
	if((arg == NULL) ? 1 : (*arg == '\0'))
	{
	    /* No argument, print standard. */        
	    CmdNetstatPrintStandard(condescriptor);

	    /* Print usage at end. */
	    sprintf(
		sndbuf,
		"Usage: `%s [player] [-l] [-c]'",
		THIS_CMD_NAME
	    );
	    NetSendLiveMessage(condescriptor, sndbuf);
        }
	else
	{
	    strncpy(larg, arg, CS_DATA_MAX_LEN);
	    StringStripSpaces(larg);

	    if(larg[0] == '-')
	    {
		if(larg[1] == 'c')
		{
		    /* Print connections. */
		    CmdNetstatPrintStandard(condescriptor);
		}
		else if(larg[1] == 'l')
		{
		    /* Print listening sockets. */
                    CmdNetstatPrintListening(condescriptor);
		}
		else
		{
		    /* Print usage. */
                    sprintf(
                        sndbuf,
                        "Usage: `%s [player] [-l] [-c]'",
                        THIS_CMD_NAME
                    );
                    NetSendLiveMessage(condescriptor, sndbuf);
		}
	    }
	    else
	    {
		/* Print netstats of player. */

	        /* Get name of object. */
                strncpy(name, larg, XSW_OBJ_NAME_MAX);
		name[XSW_OBJ_NAME_MAX - 1] = '\0';

		if(!strcasecmp(name, "me"))
		    obj_num = con_ptr->object_num;
		else
		    obj_num = MatchObjectByName(
		        xsw_object, total_objects,
			name, XSW_OBJ_TYPE_PLAYER
		    );
		if(DBIsObjectGarbage(obj_num))
		{
                    sprintf(sndbuf,
                        "%s: No such player.",
			name
		    );
                    NetSendLiveMessage(condescriptor, sndbuf);
		}
		else
		{
		    CmdNetstatPrintPlayer(condescriptor, obj_num);
		}
	    }
	}


        return(0);
}
