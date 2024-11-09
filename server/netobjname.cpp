#include "../include/unvmatch.h"
#include "../include/swnetcodes.h"
#include "swserv.h"
#include "net.h"


int NetHandleObjectName(int condescriptor, char *arg)
{
	int object_num;


        /*
         *   SWEXTCMD_REQNAME format:
         *
         *	object_num
         */
        sscanf(arg, "%i",
                &object_num
        );

        if(!DBIsObjectGarbage(object_num))
            NetSendObjectName(condescriptor, object_num);

        return(0);
}


int NetSendObjectName(int condescriptor, int object_num)
{
	xsw_object_struct *obj_ptr;
	char sndbuf[CS_DATA_MAX_LEN];


        if(DBIsObjectGarbage(object_num))
            return(-1);
        else
            obj_ptr = xsw_object[object_num];

        /*
         *   SWEXTCMD_SETOBJNAME format:
         *
         *	object_num;name;empire
         */
        sprintf(sndbuf,
"%i %i\
 %i;%s;%s\n",
                CS_CODE_EXT,
                SWEXTCMD_SETOBJNAME,

                object_num,
                obj_ptr->name,
		obj_ptr->empire
        );
        NetDoSend(condescriptor, sndbuf);


        return(0);
}
