/*
                  In Range XSW Objects Management

	Functions:

	void DBInRangeUpdate(int object_num)
	int DBInRangeAdd(int ref_obj, int tar_obj, char check_range)
	void DBInRangeDelete(xsw_object_struct *obj_ptr)
	void DBInRangeDeleteAll()

	---

	Manages the array pointer containing a list of
	objects in range of the player.

 */

#include "../include/unvmath.h"

#include "xsw.h"


/*
 *	Updates the in range objects list pointer array by first
 *	clearing the list and then adding only the objects
 *	in the same sector as the object specified by object_num.
 */
void DBInRangeUpdate(int object_num)
{
	int i, n;
	xsw_object_struct *ref_obj_ptr, **ptr, *obj_ptr;


	/* First clear inrange objects list. */
	DBInRangeDeleteAll();


	/* Get pointer to the referance object. */
	if(DBIsObjectGarbage(object_num))
	    return;
	else
	    ref_obj_ptr = xsw_object[object_num];


	/* Begin fetching objects that are in range. */

	/* Get home and area objects first. */
	for(i = 0, ptr = xsw_object;
            i < total_objects;
            i++, ptr++
	)
	{
	    obj_ptr = *ptr;
	    if(obj_ptr == NULL)
		continue;

	    /* Skip if not these type of objects. */
	    if((obj_ptr->type != XSW_OBJ_TYPE_HOME) &&
               (obj_ptr->type != XSW_OBJ_TYPE_AREA) &&
               (obj_ptr->type != XSW_OBJ_TYPE_WORMHOLE) &&
               (obj_ptr->type != XSW_OBJ_TYPE_ELINK)
	    )
		continue;

	    /* Check if object is in the same sector as referance
	     * object and that it's recently updated.
	     */
	    if(!Mu3DInSameSectorPtr(obj_ptr, ref_obj_ptr))
		continue;

	    if((obj_ptr->last_updated + OBJECT_OUTDATED_TIMEOUT)
                < cur_millitime
	    )
	    {
		/* Referance object is always considered up to date. */
		if(obj_ptr != ref_obj_ptr)
		    continue;
	    }


	    /* Object is in range and up to date. */

	    n = total_inrange_objects;

	    total_inrange_objects++;
	    inrange_xsw_object = (xsw_object_struct **)realloc(
		inrange_xsw_object,
		total_inrange_objects * sizeof(xsw_object_struct *)
	    );
	    if(inrange_xsw_object == NULL)
	    {
		total_inrange_objects = 0;
		return;
	    }

	    /* Set pointer to object. */
	    inrange_xsw_object[n] = obj_ptr;
	}


        /* Next, get all other types of objects. */
        for(i = 0, ptr = xsw_object;
            i < total_objects;
            i++, ptr++
        )
        {
	    obj_ptr = *ptr;
            if(obj_ptr == NULL)
                continue;

	    /* Skip these types of objects. */
            if((obj_ptr->type == XSW_OBJ_TYPE_HOME) ||
               (obj_ptr->type == XSW_OBJ_TYPE_AREA) ||
               (obj_ptr->type == XSW_OBJ_TYPE_WORMHOLE) ||
               (obj_ptr->type == XSW_OBJ_TYPE_ELINK)
            )
                continue;

            /* Check if object is in the same sector as referance
             * object and that it's recently updated.
             */
            if(!Mu3DInSameSectorPtr(obj_ptr, ref_obj_ptr))
                continue;

            if((obj_ptr->last_updated + OBJECT_OUTDATED_TIMEOUT)
                < cur_millitime
            )
            {
                /* Referance object is always considered up to date. */
                if(obj_ptr != ref_obj_ptr)
                    continue;
            }


            /* Object is in range and up to date. */

            n = total_inrange_objects;

            total_inrange_objects++;
            inrange_xsw_object = (xsw_object_struct **)realloc(
                inrange_xsw_object,
                total_inrange_objects * sizeof(xsw_object_struct *)
            );
            if(inrange_xsw_object == NULL)
            {
                total_inrange_objects = 0;
                return;
            }

            /* Set pointer to object. */
            inrange_xsw_object[n] = obj_ptr;
        }


	return;
}


/*
 *	Adds tar_obj to the in range objects list.
 *	If check_range is 0, then tar_obj is added to
 *	the list unconditionally.
 *	If check_range is 1 then the tar_obj is checked
 *	to see if it is in range with ref_obj. if any only if
 *	tar_obj is in range with ref_obj will tar_obj be added
 *	to the list.
 */
int DBInRangeAdd(int ref_obj, int tar_obj, char check_range)
{
	int i;
	xsw_object_struct *ref_obj_ptr, *tar_obj_ptr;


	/* Check if target object is valid and get pointer. */
        if(DBIsObjectGarbage(tar_obj))
            return(-1);
        else
            tar_obj_ptr = xsw_object[tar_obj];


        /* Check if target object is already in in range list. */
        for(i = 0; i < total_inrange_objects; i++)
        { 
            if(inrange_xsw_object[i] == tar_obj_ptr)
                return(0);
        }


	/* Check if target object is in range with referance object? */
	if(check_range)
	{
	    if(DBIsObjectGarbage(ref_obj))
		return(-1);
	    else
		ref_obj_ptr = xsw_object[ref_obj];

	    /* Check if in range. */
	    if(!Mu3DInSameSectorPtr(tar_obj_ptr, ref_obj_ptr))
		return(0);
	}


	/*   Add target object to in range list by type.
	 *   If the object is of type HOME or AREA, then add it to
	 *   the beginning of the list.  If it is not then append
	 *   it to the end.
	 */

	/* Sanitize total. */
	if(total_inrange_objects < 0)
	    total_inrange_objects = 0;

	/* Increment total and allocate more pointers. */
	total_inrange_objects++;

	inrange_xsw_object = (xsw_object_struct **)realloc(
            inrange_xsw_object,
            total_inrange_objects * sizeof(xsw_object_struct *)
        );
	if(inrange_xsw_object == NULL)
	{
	    total_inrange_objects = 0;
	    return(-1);
	}


	/* Add by type. */
	if((tar_obj_ptr->type == XSW_OBJ_TYPE_HOME) ||
           (tar_obj_ptr->type == XSW_OBJ_TYPE_AREA) ||
           (tar_obj_ptr->type == XSW_OBJ_TYPE_WORMHOLE) ||
           (tar_obj_ptr->type == XSW_OBJ_TYPE_ELINK)
	)
	{
	    /* Home or area, add to beginning of list. */

	    /* Shift pointers. */
	    for(i = total_inrange_objects - 1; i > 0; i--)
		inrange_xsw_object[i] = inrange_xsw_object[i - 1];

	    inrange_xsw_object[0] = tar_obj_ptr;
	}
	else
	{
	    /* All other types, append to end. */

	    i = total_inrange_objects - 1;
	    inrange_xsw_object[i] = tar_obj_ptr;
	}


	return(0);
}


/*
 *	Removes obj_ptr from the in range objects list.
 *
 *	Warning, the global inrange_xsw_object pointers will
 *	be reallocated!
 */
void DBInRangeDelete(xsw_object_struct *obj_ptr)
{
	int i, h;
	xsw_object_struct **ptr;


	if(obj_ptr == NULL)
	    return;


	if(total_inrange_objects < 0)
	    total_inrange_objects = 0;


	for(i = 0, h = -1, ptr = inrange_xsw_object;
	    i < total_inrange_objects;
	    i++, ptr++
	)
	{
            if(*ptr == NULL)
		continue;

            if(*ptr == obj_ptr)
	    {
                *ptr = NULL;
		continue;
	    }

	    h = i;
        }

	/* Update inrange list size. */
	total_inrange_objects = h + 1;
	if(total_inrange_objects > 0)
	{
	    inrange_xsw_object = (xsw_object_struct **)realloc(
		inrange_xsw_object,
		total_inrange_objects * sizeof(xsw_object_struct *)
	    );
	    if(inrange_xsw_object == NULL)
	    {
		total_inrange_objects = 0;
		return;
	    }
	}
	else
	{
	    free(inrange_xsw_object);
            inrange_xsw_object = NULL;

            total_inrange_objects = 0;
	}


	return;
}


/*
 *	Deletes the in range objects list (but not the objects
 *	themselves).
 */
void DBInRangeDeleteAll()
{
	free(inrange_xsw_object);
	inrange_xsw_object = NULL;

	total_inrange_objects = 0;


	return;
}
