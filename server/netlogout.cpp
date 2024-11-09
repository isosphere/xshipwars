#include "../include/unvmatch.h"
#include "../include/swnetcodes.h"
#include "swserv.h"
#include "net.h"        
        

int NetHandleLogout(int condescriptor)
{
	int object_num;
	char sndbuf[CS_DATA_MAX_LEN];
	connection_struct *con_ptr;


        /* Must be connected. */
        if(ConIsConnected(condescriptor))
	    con_ptr = connection[condescriptor];
	else
            return(-1);


        object_num = con_ptr->object_num;
        if(DBIsObjectGarbage(object_num))
        {

        }
        else
        {
	    /* Send normal leave message to connection. */
            strncpy(sndbuf, sysparm.mesg_leave, CS_DATA_MAX_LEN);
            sndbuf[CS_DATA_MAX_LEN - 1] = '\0';
            substr(sndbuf, "%title", unv_head.title);
            substr(sndbuf, "%name",
                xsw_object[object_num]->name
            );
            NetSendLiveMessage(condescriptor, sndbuf);
        }

	/* Send logout to connection and close connection. */
        NetCloseConnection(condescriptor);


        return(0);
}


int NetSendLogout(int condescriptor)
{
	char sndbuf[CS_DATA_MAX_LEN];


	/*
	 *	CS_CODE_LOGOUT format:
	 *
	 */
	sprintf(sndbuf,
		"%i\n",
		CS_CODE_LOGOUT
	);
        NetDoSend(condescriptor, sndbuf);


	return(0);
}
