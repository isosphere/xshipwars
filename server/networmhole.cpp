#include "../include/unvmatch.h"
#include "../include/swnetcodes.h"
#include "swserv.h"
#include "net.h"



int NetHandleWormHoleEnter(int condescriptor, char *arg)
{
	double d;
        int src_obj, tar_obj, dest_obj;
	xsw_object_struct *src_obj_ptr, *tar_obj_ptr, *dest_obj_ptr;
	xsw_vector_compoent_struct *src_attitude;


        /*
         *   SWEXTCMD_WORMHOLEENTER format:
         *
         *      src_obj tar_obj
         */
        sscanf(arg, "%i %i",
		&src_obj,
		&tar_obj
        );

	/* Connection must own src_obj. */
	if(src_obj != connection[condescriptor]->object_num)
	    return(-3);

	/* Get pointers to source and target objects. */
        if(DBIsObjectGarbage(src_obj))
            return(-1);
        else
            src_obj_ptr = xsw_object[src_obj];

	if(DBIsObjectGarbage(tar_obj))
	    return(-1);
	else
	    tar_obj_ptr = xsw_object[tar_obj];

	/* Target object must be a wormhole type object. */
	if(tar_obj_ptr->type != XSW_OBJ_TYPE_WORMHOLE)
	    return(-1);

	/* Get linked destination (intercepting object) of target
	 * object (the `worm hole').
	 */
	dest_obj = tar_obj_ptr->intercepting_object;
        if(DBIsObjectGarbage(dest_obj))
            return(-1);
        else
            dest_obj_ptr = xsw_object[dest_obj];

	/* Linked to destination need not be a worm hole. */


        /* Move source object to destination object's position with
         * some offsets.
         */
        src_attitude = &src_obj_ptr->attitude_vector_compoent;

        /* Move source object in its heading direction to
         * away from the destination object based on both object's
         * size (in Real units).
         */
        d = (double)dest_obj_ptr->size / 1000;
           
        src_obj_ptr->x = dest_obj_ptr->x + (d * src_attitude->i);
        src_obj_ptr->y = dest_obj_ptr->y + (d * src_attitude->j);
        src_obj_ptr->z = dest_obj_ptr->z;

	/* Move source object to a position is away from
	 * destination object.
	 */
	if((src_obj_ptr->sect_x != dest_obj_ptr->sect_x) ||
           (src_obj_ptr->sect_y != dest_obj_ptr->sect_y) ||
           (src_obj_ptr->sect_z != dest_obj_ptr->sect_z)
	)
	{
	    /* Change the object's sector and update related resources. */
	    DBObjectDoSetSector(
		src_obj,
		dest_obj_ptr->sect_x,
		dest_obj_ptr->sect_y,
		dest_obj_ptr->sect_z,
		0			/* No position wrapping. */
	    );

	    NetSendFObjectSect(condescriptor, src_obj);
	}

	NetSendObjectForcePose(condescriptor, src_obj);


        return(0);
}
