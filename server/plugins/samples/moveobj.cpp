/*
                      ShipWars Plugin Sample Source


        To compile this, type:

        cc moveobj.c -o moveobj -shared -g


        This program won't run by itself, but is intented to be used
        as a plugin for the ShipWars Server.

	---

	A bit more advanced use of timming and to demostrate the
	fundimentals of manipulating objects.

	Pay close attention to how the SWServ's global objects are
	accessed and manipulated.

	In this example we will move the object three times then exit.

	NOTE: The functions below have been wrapped with extern "C".
        When using C instead of C++, this should be removed.
 */

#include "../include/plugins.h"

extern "C" {
#define TOTAL_MOVES	3
int move_code;

time_t last_millitime;
time_t next_movement;


/* See timming.c for why timmers have to be reset. */
void ResetTimmers(void)
{
	next_movement = 0;

	return;
}

SWPLUGIN_INIT_FUNCRTN SWPLUGIN_INIT_FUNC(SWPLUGIN_INIT_FUNCINPUT)
{
	/* Upon initialization, reset our globals. */
	ResetTimmers();
	move_code = 0;

	/* Get current time of day in milliseconds. */
	last_millitime = *(in->cur_millitime);


	return(0);
}

SWPLUGIN_MANAGE_FUNCRTN SWPLUGIN_MANAGE_FUNC(SWPLUGIN_MANAGE_FUNCINPUT)
{
	xsw_object_struct ***xsw_object;
	int *total_objects;
	time_t cur_millitime;
        void (*obj_sync_clients)(xsw_object_struct **, int, int);


	/* Get pointer to objects array. */
	xsw_object = in->xsw_object;
	total_objects = in->total_objects;

	/* Get pointer to server functions. */
	obj_sync_clients = in->obj_sync_clients;


	/* Get current time in milliseconds from ShipWars Server. */
	cur_millitime = *(in->cur_millitime);

	/* Now check if the time `cycled' */
	if(cur_millitime < last_millitime)
		ResetTimmers();

	last_millitime = cur_millitime;


	/* Is it time to move objects? */
	if(next_movement <= cur_millitime)
	{
	    int object_num;
	    xsw_object_struct *obj_ptr;
	    double x_offset, y_offset;


	    /* Calculate movement offset (in Real units). */
	    switch(move_code)
	    {
	      case 0:
		x_offset = 0;
		y_offset = 2.5;
		break;

	      case 1:
		x_offset = 0;
                y_offset = -4.2;
                break;

             case 2:
                x_offset = 2.8;
                y_offset = 1.3;
                break;
	    }

	    /* Move each object. */
	    for(object_num = 0; object_num < *total_objects; object_num++)
	    {
		/* Get pointer to object from array. */
		obj_ptr = (*xsw_object)[object_num];

		/* Skip this object of it is not allocated. */
		if(obj_ptr == NULL)
		    continue;

		/* Skip this object if its of type garbage or error. */
		if(obj_ptr->type <= XSW_OBJ_TYPE_GARBAGE)
		    continue;

                /* Skip this object if its not a type HOME object. */
                if(obj_ptr->type != XSW_OBJ_TYPE_HOME)
                    continue;
 
		/* Move this object. */
		obj_ptr->x += x_offset;
		obj_ptr->y += y_offset;

		/* Update object position to clients. */
		obj_sync_clients(*xsw_object, *total_objects, object_num);
	    }


	    /* Increment move code. */
	    move_code++;

	    /* Finished? */
	    if(move_code >= TOTAL_MOVES)
		return(-1);	/* Return -1 to indicate want to exit. */


	    /* Schedual next movement in 2.5 seconds. */
	    next_movement = cur_millitime + 2500;
	}

	return(0);
}

SWPLUGIN_SHUTDOWN_FUNCRTN SWPLUGIN_SHUTDOWN_FUNC(SWPLUGIN_SHUTDOWN_FUNCINPUT)
{
	return;
}

} // extern "C"
