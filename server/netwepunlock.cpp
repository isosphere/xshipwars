#include "../include/unvmatch.h"
#include "../include/swnetcodes.h"
#include "swserv.h"
#include "net.h"


int NetHandleSetWeaponsUnlock(int condescriptor, char *arg)
{
        long con_object_num;
        xsw_object_struct *obj_ptr;


        con_object_num = connection[condescriptor]->object_num;
        if(DBIsObjectGarbage(con_object_num))
            return(-1);
        else
            obj_ptr = xsw_object[con_object_num];


        /* Unlock weapons on object.. */
        obj_ptr->locked_on = -1;


        /* Set next object values update to now so it gets updated now. */
        next.object_values = cur_millitime;


        return(0);
}


int NetSendSetWeaponsUnlock(int condescriptor)
{
        return(0);
}
