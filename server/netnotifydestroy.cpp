#include "../include/unvmatch.h"
#include "../include/swnetcodes.h"
#include "swserv.h"
#include "net.h"



int NetHandleNotifyDestroy(int condescriptor, char *arg)
{
	return(0);
}

int NetSendNotifyDestroy(
        int condescriptor,
        int reason,
        int destroyed_obj, 
        int destroyer_obj,
        int destroyer_obj_owner
)
{
        char sndbuf[CS_DATA_MAX_LEN];


        /*
         *   SWEXTCMD_NOTIFYDESTROY format:
	 *
         *	reason_code
	 *	destroyed_obj destroyer_obj
	 *	destroyer_obj_owner
         */
        sprintf(sndbuf,
"%i %i\
 %i\
 %i %i\
 %i\n",

                CS_CODE_EXT,
                SWEXTCMD_NOTIFYDESTROY,

                reason,
                destroyed_obj, destroyer_obj,
		destroyer_obj_owner
        );
        NetDoSend(condescriptor, sndbuf);


        return(0);   
}
