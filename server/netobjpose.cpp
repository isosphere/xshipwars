#include "../include/unvmatch.h"
#include "../include/unvmath.h"
#include "../include/swnetcodes.h"
#include "swserv.h"
#include "net.h"  
 
 
int NetHandleObjectPose(int condescriptor, char *arg)
{
	int object_num;

	int type;
        int isref_num;
        long size;

	float x, y, z;
        float heading, pitch, bank;

	float velocity;

	float velocity_heading;
        float velocity_pitch;
        float velocity_bank;

        int current_frame;
        xsw_object_struct *obj_ptr;


        /*
         *   CS_CODE_POSEOBJ format:
         *
         *      object_num,
         *      type, isref_num, size,
         *      x, y, z,
         *      heading, pitch,
         *      velocity, velocity_heading,
         *      throttle, frame
         */
        sscanf(arg,
"%i\
 %i %i %ld\
 %f %f %f\
 %f %f %f\
 %f\
 %f %f %f\
 %i",
 
                &object_num,
            
                &type,
                &isref_num,
                &size,
        
                &x, 
                &y,
                &z,
        
                &heading,
                &pitch,
                &bank,
        
                &velocity,
         
                &velocity_heading,
                &velocity_pitch,
                &velocity_bank,
         
                &current_frame 
        );

	/* Connection must own object. */         
        if(connection[condescriptor]->object_num != object_num)
            return(-3);
 

	if(DBIsObjectGarbage(object_num))
	    return(-1);
	else
	    obj_ptr = xsw_object[object_num];


        /* Set object_num's location and apperance. */
        /* Don't set type, imageset, or size. */
/*
        obj_ptr->type = type;
        obj_ptr->imageset = isref_num;
        obj_ptr->size = size;
*/


        /* Set these only if object has enough antimatter. */
        if(obj_ptr->antimatter > 0)
        {
            obj_ptr->x = x;
            obj_ptr->y = y;
            obj_ptr->z = z;

            obj_ptr->velocity = velocity;

            obj_ptr->velocity_heading = velocity_heading;
            obj_ptr->velocity_pitch = velocity_pitch;
            obj_ptr->velocity_bank = velocity_bank;
            MuSetUnitVector2D(
                &obj_ptr->momentum_vector_compoent,
                obj_ptr->velocity_heading
            );
        }

        obj_ptr->heading = heading;
        obj_ptr->pitch = pitch;
	obj_ptr->bank = bank;
        MuSetUnitVector2D(
            &obj_ptr->attitude_vector_compoent,
            obj_ptr->heading
        );

/*
        obj_ptr->animation.current_frame = current_frame;
*/

        return(0);
}


int NetSendObjectPose(int condescriptor, int object_num)
{
        char sndbuf[CS_DATA_MAX_LEN];
        xsw_object_struct *obj_ptr;  


        if(DBIsObjectGarbage(object_num))
            return(-1);
        else
            obj_ptr = xsw_object[object_num];


        /*
         *   CS_CODE_POSEOBJ format:
         *
         *      object_num,
         *      type, isref_num, size,
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
   
                CS_CODE_POSEOBJ,
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


        /* Send data to all or single connection. */
        NetDoSend(condescriptor, sndbuf);


        return(0); 
}
