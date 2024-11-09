#include "../include/unvmatch.h"
#include "../include/swnetcodes.h"
#include "swserv.h"
#include "net.h"


int NetHandleShieldVis(int condescriptor, char *arg)
{
	return(0);
}


int NetSendShieldVis(int condescriptor, int object_num)
{
	char sndbuf[CS_DATA_MAX_LEN];
        xsw_object_struct *obj_ptr;  


        if(DBIsObjectGarbage(object_num))
            return(-1);
        else
            obj_ptr = xsw_object[object_num];


        /*
         *      SWEXTCMD_SETSHIELDVIS format:
         *
         *      object_num shield_visibility
         */
        sprintf(sndbuf,
"%i %i %i %.4f\n",

                CS_CODE_EXT,
                SWEXTCMD_SETSHIELDVIS,

                object_num,
                obj_ptr->shield_visibility
        );
        NetDoSend(condescriptor, sndbuf);


        return(0);
}
