#include "../include/unvmatch.h"
#include "../include/swnetcodes.h"
#include "swserv.h"
#include "net.h"


int NetHandleObjectForcePose(int condescriptor, char *arg)  
{
	return(0);
}


int NetSendObjectForcePose(int condescriptor, int object_num)
{
	char sndbuf[CS_DATA_MAX_LEN];
	xsw_object_struct *obj_ptr;


        if(DBIsObjectGarbage(object_num))
            return(-1);
        else
            obj_ptr = xsw_object[object_num];
          
          
        /*
         *	CS_CODE_FORCEPOSEOBJ format:
         *   
         *      object_num,
         *      type, imageset, size,
         *      x, y, z,
         *      heading, pitch, bank,
         *      velocity,
         *      velocity_heading, velocity_pitch, velocity_bank,
         *      current_frame
         */
        sprintf(sndbuf,
"%i %i\
 %i %i %ld\
 %.4f %.4f %.4f\
 %.4f %.4f %.4f\
 %.4f\
 %.4f %.4f %.4f\
 %i\n",

                CS_CODE_FORCEPOSEOBJ,
                object_num,

                obj_ptr->type,
                obj_ptr->imageset,
                obj_ptr->size,

                obj_ptr->x,
                obj_ptr->y,
                obj_ptr->z,

                obj_ptr->heading,
                obj_ptr->pitch,
                obj_ptr->bank,

                obj_ptr->velocity,

                obj_ptr->velocity_heading,
                0.0000, 
                0.0000,

                obj_ptr->animation.current_frame
        );
        NetDoSend(condescriptor, sndbuf);


        return(0); 
}
