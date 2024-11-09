#include "../include/unvmatch.h"
#include "../include/swnetcodes.h"
#include "swserv.h"
#include "net.h"



int NetHandleWhoAmI(int condescriptor, char *arg)
{
	NetSendWhoAmI(condescriptor);

	return(0);
}


int NetSendWhoAmI(int condescriptor)
{
        char sndbuf[CS_DATA_MAX_LEN];
        long object_num;
        xsw_object_struct *obj_ptr;
        
        
        /* Get object_num. */
        object_num = connection[condescriptor]->object_num;
        
        /* Check if object is logged in with valid object. */

        if(DBIsObjectGarbage(object_num)) 
        {   
            sprintf(sndbuf,
                "Who am I information unknown, connection not logged in."
            );
            NetSendLiveMessage(condescriptor, sndbuf); 
            return(-1);
        } 
        else
        {
            obj_ptr = xsw_object[object_num];
        }       


        /*
	 *	CS_CODE_WHOAMI format:
	 *
	 *	object_num;object_name
	 */
        sprintf(sndbuf,
"%i %ld;%s\n",

            CS_CODE_WHOAMI,

            object_num,
            obj_ptr->name
        );
        NetSendDataToConnection(condescriptor, sndbuf, 1);


        return(0);
}
