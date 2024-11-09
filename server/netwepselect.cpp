#include "../include/unvmatch.h"
#include "../include/swnetcodes.h"
#include "swserv.h"
#include "net.h"


int NetHandleSelectWeapon(int condescriptor, char *arg)
{
	long object_num, con_object_num;
        int selected_weapon;
        xsw_object_struct *obj_ptr;


        con_object_num = connection[condescriptor]->object_num; 
        if(DBIsObjectGarbage(con_object_num))
            return(-1);
        else
            obj_ptr = xsw_object[con_object_num];


        /*
         *	SWEXTCMD_SETWEAPON format:
         *
         *      object_num selected_weapon
         */
        sscanf(arg, "%ld %i",
                &object_num,
                &selected_weapon
        );


        /* Can only set connection's object. */
        if(object_num != con_object_num)
            return(-3);


        /* Make sure selected_weapon is valid. */
        if((selected_weapon < 0) || (selected_weapon >= MAX_WEAPONS))
            return(-1);
        if(selected_weapon >= obj_ptr->total_weapons)
            return(-1);


        /* Set selected weapon. */
        obj_ptr->selected_weapon = selected_weapon;


        /* Set next object values update to now so it gets updated now. */
        next.object_values = cur_millitime;


        return(0);
}


int NetSendSelectWeapon(int condescriptor)
{
	return(0);
}
