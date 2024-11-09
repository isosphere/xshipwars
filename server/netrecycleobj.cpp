#include "../include/unvmatch.h"
#include "../include/swnetcodes.h"
#include "swserv.h"
#include "net.h"


int NetHandleRecycleObject(int condescriptor, char *arg) 
{
	return(0);
}


int NetSendRecycleObject(int condescriptor, int object_num)
{
	char sndbuf[CS_DATA_MAX_LEN];


        if((object_num < 0) || (object_num >= MAX_OBJECTS))
            return(-1);


        /*
	 *   CS_CODE_RECYCLEOBJ format:
	 *
	 *	object_num
	 */
        sprintf(sndbuf,
		"%i %i\n",

                CS_CODE_RECYCLEOBJ,
                object_num
        );
        NetDoSend(condescriptor, sndbuf);


        return(0);
}
