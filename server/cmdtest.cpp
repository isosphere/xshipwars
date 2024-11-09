/*
	Test function, involved by a server command starting with the
	command name "test" from cmd.cpp.
 */

#include "../include/unvmatch.h"

#include "swserv.h"
#include "net.h"


int CmdTest(int condescriptor, const char *arg)
{
	int con_obj_num, n;
	xsw_object_struct *obj_ptr;

/* Remove this to make the tests work. */
return(0);

	con_obj_num = connection[condescriptor]->object_num;
	if(DBIsObjectGarbage(con_obj_num))
	    return(-1);
	else
	    obj_ptr = xsw_object[con_obj_num];


	n = DBCreateObjectByOPM(
	    "Explosion3",
            NULL,
            XSW_OBJ_TYPE_ANIMATED,
            xsw_object[con_obj_num]->x,
            xsw_object[con_obj_num]->y + 0.2,
            xsw_object[con_obj_num]->z,
            xsw_object[con_obj_num]->heading,
            xsw_object[con_obj_num]->pitch
        );
        if(DBIsObjectGarbage(n))
	    return(-1);
	else
	    obj_ptr = xsw_object[n];

	obj_ptr->sect_x = xsw_object[con_obj_num]->sect_x;
        obj_ptr->sect_y = xsw_object[con_obj_num]->sect_y;
	obj_ptr->sect_z = xsw_object[con_obj_num]->sect_z;

        obj_ptr->animation.total_frames = 6;
        obj_ptr->animation.cycle_times = -1;

	/* Explicitly set imageset. */
	obj_ptr->imageset = 23;

        NetSendCreateObject(-1, n);


        return(0);
}
