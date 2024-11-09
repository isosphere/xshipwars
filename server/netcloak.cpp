#include "../include/unvmatch.h"
#include "swserv.h"
#include "net.h"


int NetHandleSetCloak(int condescriptor, char *arg)
{
        long object_num, con_object_num;
        int cloak_state;
        xsw_object_struct *obj_ptr;


        con_object_num = connection[condescriptor]->object_num;
        if(DBIsObjectGarbage(con_object_num))
            return(-1);
        else
            obj_ptr = xsw_object[con_object_num];


        /*
         *	SWEXTCMD_SETCLOAK format:
         *
         *      object_num, cloak_state
         */
        sscanf(arg, "%ld %i",
                &object_num,
                &cloak_state
        );      


        /* Connection must own object. */
        if(object_num != con_object_num)
            return(-3);


        /* Does object have cloak abilities? */
        if(obj_ptr->cloak_state == CLOAK_STATE_NONE)
            return(-3);

        /* Are shields up? if so, lower it. */
        if(obj_ptr->shield_state == SHIELD_STATE_UP)
            obj_ptr->shield_state = SHIELD_STATE_DOWN;


        /* Raise cloak? */
        if((obj_ptr->cloak_state == CLOAK_STATE_DOWN) &&
           (cloak_state == CLOAK_STATE_UP)
        )   
        {
            obj_ptr->cloak_state = CLOAK_STATE_UP;
        }   
        /* Lower cloak? */
        else if((obj_ptr->cloak_state == CLOAK_STATE_UP) &&
                (cloak_state == CLOAK_STATE_DOWN)
        )
        {  
            obj_ptr->cloak_state = CLOAK_STATE_DOWN;
        }


        /* Set next object values update to now so it gets updated now. */
        next.object_values = cur_millitime;


        return(0);
}


int NetSendSetCloak(int condescriptor)
{
        return(0);
}
