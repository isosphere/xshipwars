#include "../include/unvmatch.h"
#include "../include/swnetcodes.h"
#include "swserv.h"
#include "net.h"


int NetHandleSetDmgCtl(int condescriptor, char *arg)
{
        long object_num, con_object_num;
        int damage_control;
        xsw_object_struct *obj_ptr;


        con_object_num = connection[condescriptor]->object_num;
        if(DBIsObjectGarbage(con_object_num))
            return(-1);
        else
            obj_ptr = xsw_object[con_object_num];


        /*
         *	SWEXTCMD_SETDMGCTL format:
         *
         *	object_num, damage_control
         */
        sscanf(arg, "%ld %i",
                &object_num,
                &damage_control
        );


        /* Connection must own object. */
        if(object_num != con_object_num)
            return(-3);


        /* Turn damage control on? */   
        if((obj_ptr->damage_control == DMGCTL_STATE_OFF) &&
           (damage_control == DMGCTL_STATE_ON)
        )
        {
            obj_ptr->damage_control = DMGCTL_STATE_ON;
        }
        /* Turn damage control off? */
        else if((obj_ptr->damage_control == DMGCTL_STATE_ON) &&
                (damage_control == DMGCTL_STATE_OFF)
        )
        {   
            obj_ptr->damage_control = DMGCTL_STATE_OFF;
        }


        /* Set next object values update to now so it gets updated now. */
        next.object_values = cur_millitime;


        return(0);
}


int NetSendSetDmgCtl(int condescriptor)
{
	return(0);
}
