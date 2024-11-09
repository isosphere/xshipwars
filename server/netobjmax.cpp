#include "../include/unvmatch.h"
#include "../include/swnetcodes.h"
#include "swserv.h"   
#include "net.h"

 
int NetHandleObjectMaximums(int condescriptor, char *arg)
{
	return(0);
}

  
int NetSendObjectMaximums(int condescriptor, int object_num)
{
        char sndbuf[CS_DATA_MAX_LEN];
        xsw_object_struct *obj_ptr;


        /* Make sure object is valid. */ 
        if(DBIsObjectGarbage(object_num))
            return(-1);
        else
            obj_ptr = xsw_object[object_num];


        /*
         *   SWEXTCMD_STDOBJMAXS format:
         *
         *      type, imageset, owner, size, scanner_range,
         *      velocity_max, thrust_power, turnrate, hp_max, power_max,
         *      power_purity, core_efficency, antimatter_max, total_weapons,
         *      visibility 
         */
        sprintf(sndbuf,
"%i %i %i\
 %i %i %i %ld %.4f\
 %.4f %.4f %.4f %.4f %.4f\
 %.4f %.4f %.4f %i %.4f\n",

                CS_CODE_EXT,
                SWEXTCMD_STDOBJMAXS,
  
                object_num,
 
                obj_ptr->type,
                obj_ptr->imageset,
                obj_ptr->owner,
                obj_ptr->size,
                obj_ptr->scanner_range,   
        
                obj_ptr->velocity_max,
                obj_ptr->thrust_power,   
                obj_ptr->turnrate,
                obj_ptr->hp_max,
                obj_ptr->power_max,

                obj_ptr->power_purity,
                obj_ptr->core_efficency,
                obj_ptr->antimatter_max,
                obj_ptr->total_weapons,
                obj_ptr->visibility
        );
        NetDoSend(condescriptor, sndbuf);


        return(0);
}
