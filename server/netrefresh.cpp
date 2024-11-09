#include "../include/unvmatch.h"
#include "../include/unvmath.h"
#include "../include/swnetcodes.h"
#include "swserv.h"
#include "net.h"


int NetHandleRefresh(int condescriptor, char *arg) 
{
	NetSendRefresh(condescriptor);

	return(0);
}


int NetSendRefresh(int condescriptor)
{
	int i, weapon_num, object_num;
        xsw_object_struct *obj_ptr, **ptr;


        /* Connection must be logged in. */
        if(!ConIsLoggedIn(condescriptor))  
            return(-1);


        /* Get object_num and obj_ptr. */
        object_num = connection[condescriptor]->object_num;
        if(DBIsObjectGarbage(object_num))
            return(-1);
        else
            obj_ptr = xsw_object[object_num];


	/* Send units. */
	NetSendUnits(condescriptor);

	/* Send data file names. */
        NetSendSetImageSet(condescriptor, unv_head.isr);
	NetSendSetSoundSet(condescriptor, unv_head.ss);
	NetSendSetOCSN(condescriptor, unv_head.ocsn);

        /* Send who am I. */
        NetSendWhoAmI(condescriptor);


        /* Go through XSW objects list. */
        for(i = 0, ptr = xsw_object;
            i < total_objects;
            i++, ptr++
        )
        {
            /* Check objects are valid and within range. */
            if(!Mu3DInRangePtr(obj_ptr, *ptr, obj_ptr->scanner_range))
                continue;

            /*   If object is not owned by the connection,
             *   then check the visibility of the object.
             */
            if((*ptr)->owner != object_num)
            {
                if(DBGetObjectVisibilityPtr(*ptr) <= 0.00)
                    continue;
            }

            /* Check if marked hidden from connection. */
            if((*ptr)->server_options & XSW_OBJF_HIDEFROMCON)
                continue;

	    /* Send create object. */
	    NetSendCreateObject(condescriptor, i);

            /* Send object maximums. */
            NetSendObjectMaximums(condescriptor, i);

            /* Send object sector position. */
            NetSendObjectName(condescriptor, i);

	    /* Engine state. */
	    NetSendSetEngine(condescriptor, i);

	    /* Com channel. */
	    NetSendSetChannel(condescriptor, i, (*ptr)->com_channel);


            /* If connection's object is locked on to this object,
	     * then send weapons and current values of this object.
	     */
            if(i == obj_ptr->locked_on)
            {
                /* Send values. */
                NetSendObjectValues(condescriptor, i);
        
                /* Send weapon values. */
                for(weapon_num = 0; weapon_num < (*ptr)->total_weapons; weapon_num++)
                    NetSendWeaponValues(condescriptor, i, weapon_num);
            }
        }

        /* Send weapon values of connection's object. */
        for(weapon_num = 0; weapon_num < obj_ptr->total_weapons; weapon_num++)
            NetSendWeaponValues(condescriptor, object_num, weapon_num);


	/* Send entire star chart listing. */
	if(sysparm.send_starchart)
	    NetSendEntireStarChart(condescriptor);


        return(0);
}
