#include "../include/unvmatch.h"
#include "../include/swnetcodes.h"
#include "swserv.h"
#include "net.h"


int NetHandleSetChannel(int condescriptor, char *arg)
{
	int object_num;
	int channel;
	xsw_object_struct *obj_ptr;


        /*
         *      SWEXTCMD_SETCHANNEL format:
         *
         *      object_num, channel
         */
        sscanf(arg,
		"%i %i",

		&object_num,
		&channel
        );

	/* Connection must control source object. */
	if(connection[condescriptor]->object_num != object_num)
	    return(-3);


	if(DBIsObjectGarbage(object_num))
	    return(-1);
	else
	    obj_ptr = xsw_object[object_num];

	/* Set channel. */
	obj_ptr->com_channel = channel;


	/* Update com channel to object. */
	NetSendSetChannel(condescriptor, object_num, channel);


        return(0);
}


int NetSendSetChannel(int condescriptor, int object_num, int channel)
{
	char sndbuf[CS_DATA_MAX_LEN];


        /*
         *   SWEXTCMD_SETCHANNEL format:
         *
         *      object_num, channel
         */
        sprintf(sndbuf,
"%i %i\
 %i %i\n",
                CS_CODE_EXT,
                SWEXTCMD_SETCHANNEL,

                object_num,
		channel
        );
        NetDoSend(condescriptor, sndbuf);


        return(0);
}
