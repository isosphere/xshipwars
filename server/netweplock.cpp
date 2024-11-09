#include "../include/unvmatch.h"
#include "../include/unvmath.h"
#include "../include/swnetcodes.h"
#include "swserv.h"
#include "net.h"


int NetHandleSetWeaponsLock(int condescriptor, char *arg)
{
	int	obj_num, con_obj_num,
		tar_obj_num, prev_obj_num;
	double scanner_range;
        xsw_object_struct	*con_obj_ptr,
				*new_locked_obj_ptr = NULL;


        con_obj_num = connection[condescriptor]->object_num;
        if(DBIsObjectGarbage(con_obj_num))
            return(-1);
        else
            con_obj_ptr = xsw_object[con_obj_num];


        /* Record previously locked on object. */
        prev_obj_num = con_obj_ptr->locked_on; 
        if(DBIsObjectGarbage(prev_obj_num))
        {
	    /* Was not locked on to anything, reset values. */
            con_obj_ptr->locked_on = -1;
            prev_obj_num = -1;
        }


        /*
         *   Format NET_CMD_WEAPONSLOCK:
         *
         *      object_num, tar_object_num
         */
        sscanf(arg, "%i %i",
                &obj_num,
                &tar_obj_num
        );


        /* Connection must own object. */
        if(obj_num != con_obj_num)
            return(-3);


	/* Get connection's object scanner range. */
	if(con_obj_ptr->loc_type == XSW_LOC_TYPE_NEBULA)
	    scanner_range = con_obj_ptr->scanner_range *
		VISIBILITY_NEBULA;
	else
	    scanner_range = con_obj_ptr->scanner_range;


	/* Handle parsed target object value as an argument for
	 * operation.
	 */
	switch(tar_obj_num)
	{
	  case -1:	/* Unlock. */
            con_obj_ptr->locked_on = -1;
	    break;

	  case -2:	/* Lock `next' object. */
	    while(1)
	    {
		/* Match next object. */
                con_obj_ptr->locked_on = MatchWeaponsLock(
		    xsw_object, total_objects,
                    con_obj_num,
		    prev_obj_num,
		    scanner_range
		);
		if(DBIsObjectGarbage(con_obj_ptr->locked_on))
		{
		    /* Could not match anything. */
		    break;
		}
		else
		{
		    /* Got valid `next' object. */
		    new_locked_obj_ptr = xsw_object[con_obj_ptr->locked_on];
		}

		/* Check if matched object is marked to be  hidden
		 * from connection.
		 */
		if(new_locked_obj_ptr->server_options & XSW_OBJF_HIDEFROMCON)
		{
		    /* Set previous object to current and try again. */
		    prev_obj_num = con_obj_ptr->locked_on;
		    new_locked_obj_ptr = NULL;

                    continue;
		}

		/* Match successful, break. */
		break;
            }
	    break;

	  default:	/* Lock explicitly. */
	    if(!DBIsObjectGarbage(tar_obj_num))
	    {
		new_locked_obj_ptr = xsw_object[tar_obj_num];

		/* Check if new locked on object is within scanner
		 * range.
		 */
		if(Mu3DInRangePtr(
		    con_obj_ptr, new_locked_obj_ptr,
                    scanner_range *
                       DBGetObjectVisibilityPtr(new_locked_obj_ptr)
                ))
		{
		    con_obj_ptr->locked_on = tar_obj_num;
		}
		else
		{
		    new_locked_obj_ptr = NULL;
		    con_obj_ptr->locked_on = -1;
		}
            }
        }


	/* Check if locked on a new valid object. */
	if(new_locked_obj_ptr != NULL)
	{
	    int weapon_num;


	    /* Need to send set values of target object first. */

	    NetSendObjectMaximums(condescriptor, con_obj_ptr->locked_on);

	    /* Do not send object standard values just yet, the
	     * schedualing for the next object values update will be
	     * set to now just below.
	     */

	    NetSendObjectName(condescriptor, con_obj_ptr->locked_on);
	    NetSendSetEngine(condescriptor, con_obj_ptr->locked_on);
	    NetSendSetChannel(
		condescriptor,
		con_obj_ptr->locked_on,
		new_locked_obj_ptr->com_channel
	    );

            /* Send weapon values. */
            for(weapon_num = 0;
                weapon_num < new_locked_obj_ptr->total_weapons;
                weapon_num++
	    )
		NetSendWeaponValues(
		    condescriptor, con_obj_ptr->locked_on, weapon_num
		);

	}

        /* Set next object values update to now so it gets updated now. */
        next.object_values = cur_millitime;


        return(0);
}


int NetSendSetWeaponsLock(int condescriptor)
{
	return(0);
}
