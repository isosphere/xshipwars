#include "../include/unvmatch.h"
#include "../include/swnetcodes.h"
#include "swserv.h"
#include "net.h"


int NetHandleCreateObject(int condescriptor, char *arg)
{
	return(0);
}


int NetSendCreateObject(int condescriptor, int object_num)
{
        char sndbuf[CS_DATA_MAX_LEN];
        xsw_object_struct *obj_ptr;


        if(DBIsObjectGarbage(object_num))
            return(-1);
        else
            obj_ptr = xsw_object[object_num];

        /*
         *   CS_CODE_CREATEOBJ format:
         *
         *      object_num,
         *      type, isref_num, owner, size,
         *      locked_on, intercepting_object, scanner_range,
	 *	sect_x, sect_y, sect_z,
         *      x, y, z,
         *      heading, pitch, bank,
         *      velocity,
         *      velocity_heading, velocity_pitch, velocity_bank,
         *      current_frame, anim_int, total_frames, cycle_times
         */
        sprintf(sndbuf,
"%i %i\
 %i %i %i %ld\
 %i %i %.4f\
 %ld %ld %ld\
 %.4f %.4f %.4f\
 %.4f %.4f %.4f\
 %.4f\
 %.4f %.4f %.4f\
 %i %ld %i %i\n",

                CS_CODE_CREATEOBJ,
                object_num,

                obj_ptr->type,
                obj_ptr->imageset,
                obj_ptr->owner,
                obj_ptr->size,

                obj_ptr->locked_on,
                obj_ptr->intercepting_object,
                obj_ptr->scanner_range,

                obj_ptr->sect_x,
                obj_ptr->sect_y,
                obj_ptr->sect_z,

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

                obj_ptr->animation.current_frame,
                obj_ptr->animation.interval,
                obj_ptr->animation.total_frames,
		obj_ptr->animation.cycle_times
        );
        NetDoSend(condescriptor, sndbuf);


        return(0);
}
