#include "../include/unvmatch.h"
#include "../include/swnetcodes.h"
#include "swserv.h"
#include "net.h"


int NetHandleSetShields(int condescriptor, char *arg)
{
        int object_num, con_object_num;
        int shield_state;
        double shield_frequency;   
        xsw_object_struct *obj_ptr;


        con_object_num = connection[condescriptor]->object_num;
        if(DBIsObjectGarbage(con_object_num))
            return(-1);
        else
            obj_ptr = xsw_object[con_object_num];


        /*
         *   NET_CMD_SETSHIELDS format:
         *
         *      object_num, shield_state, shield_frequency
         */
        sscanf(arg, "%i %i %lf",
                &object_num,
                &shield_state,
                &shield_frequency
        );


        /* Connection must own object. */
        if(object_num != con_object_num)
            return(-3);


        /* Does object have shields generators? */
        if(obj_ptr->shield_state == SHIELD_STATE_NONE)
            return(1);

        /* Set shield_frequency. */
        if(shield_frequency > SWR_FREQ_MAX)
            shield_frequency = SWR_FREQ_MAX;
        else if(shield_frequency < SWR_FREQ_MIN)
            shield_frequency = SWR_FREQ_MIN;

        obj_ptr->shield_frequency = shield_frequency;


        /* Is the cloak up? */
        if(obj_ptr->cloak_state == CLOAK_STATE_UP)
        { 
            obj_ptr->shield_state = SHIELD_STATE_DOWN;
            return(1);
        }


        /* Set shield visibility if shield is just raised. */
        if((obj_ptr->shield_state == SHIELD_STATE_DOWN) &&
           (obj_ptr->power_max > 0) &&
           (shield_state == SHIELD_STATE_UP)
        )
        {
            obj_ptr->shield_visibility = 
                obj_ptr->power / obj_ptr->power_max;

            /* Send shield visibility to all connections. */
            NetSendShieldVis(-1, object_num);
        }

        obj_ptr->shield_state = shield_state;


        /* Set next object values update to now, so it gets updated now. */
        next.object_values = cur_millitime;


        return(0);
}


int NetSendSetShields(int condescriptor, int object_num)
{


	return(0);
}
