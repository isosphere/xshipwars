/*
               Recycled Objects Backup Buffer Management

	Functions:

        int DBRecycleBufferInit(int entries)
        void DBRecycleBufferDeleteAll()

        int DBSaveRecycledObject(int object_num)
        int DBRecoverRecycledObject(const char *name, int owner)

	---

 */

#include "../include/unvmatch.h"
#include "../include/unvutil.h"

#include "swserv.h"


/*      
 *      Procedure to allocate recycled objects buffers. Returns
 *	the number of recycled object buffers actually allocated.
 *
 *	Global recycled_xsw_object list must already be empty
 *	before calling this function.
 */
int DBRecycleBufferInit(int entries)
{
        int i, allocated;


        /* Set new total. */
        total_recycled_objects = entries;

        /* Allocate pointer array. */
        recycled_xsw_object = (xsw_object_struct **)realloc(
	    recycled_xsw_object,
	    total_recycled_objects * sizeof(xsw_object_struct *)
	);
        if(recycled_xsw_object == NULL)
        {
	    total_recycled_objects = 0;
            return(-1);
        }

        /* Allocate each structure. */
        for(i = 0, allocated = 0; i < total_recycled_objects; i++)
        {
            recycled_xsw_object[i] = (xsw_object_struct *)calloc(
		1,
		sizeof(xsw_object_struct)
	    );
            if(recycled_xsw_object[i] == NULL)
            {
		total_recycled_objects = i;
                break;
            }
	    else
	    {
		/* Increment allocated count. */
		allocated++;

		/* Set type to garbage. */
		recycled_xsw_object[i]->type = XSW_OBJ_TYPE_GARBAGE;
	    }
        }


        return(allocated);
}

/*
 *      Procedure to deallocate all objects in recycled
 *      buffers list.
 */
void DBRecycleBufferDeleteAll()
{
        int i;


        for(i = 0; i < total_recycled_objects; i++)
            DBDeleteObject(recycled_xsw_object[i]);

        free(recycled_xsw_object);
        recycled_xsw_object = NULL;

        total_recycled_objects = 0;

        return;
}

/*
 *      Copy object_num to the recycled objects list.
 */
int DBSaveRecycledObject(int object_num)
{
        int i, n;
	xsw_object_struct *obj_ptr, *rec_obj_ptr, *rec_obj_ptr2;


        /* No recycled objects buffer allocated? */
        if(total_recycled_objects < 1)
            return(-1);

	/* Is given object to recycle garbage? */
        if(DBIsObjectGarbage(object_num))
            return(-1);
	else
	    obj_ptr = xsw_object[object_num];


	/* Look for available recycled object structure. */
	for(i = 0; i < total_recycled_objects; i++)
	{
 	    if(recycled_xsw_object[i] == NULL)
                continue;

	    if(recycled_xsw_object[i]->type <= XSW_OBJ_TYPE_GARBAGE)
		break;
	}
	if(i < total_recycled_objects)
	{
	    /* Got available structure. */
	    n = i;
	}
	else
	{
	    /* Could not find available recycled object structure, so
	     * make room for more.
	     */

            /* Deallocate sub resources of highest recycled object. */
            UNVResetObject(recycled_xsw_object[total_recycled_objects - 1]);

            /* `Shift' the recycle buffers. */
            for(i = total_recycled_objects - 1; i > 0; i--)
            {
		rec_obj_ptr = recycled_xsw_object[i];
		rec_obj_ptr2 = recycled_xsw_object[i - 1];

                if((rec_obj_ptr == NULL) ||
                   (rec_obj_ptr2 == NULL)
		)
		    continue;

		/* Don't worry about allocated substructures,
		 * the pointer values to them will be coppied accordingly
		 * when we copy over each object's value. Recycled object 0
		 * will still contain duplicate pointers to substructures,
		 * but will be set with the new object's values farther below.
		 */

                memcpy(
		    rec_obj_ptr,	/* Destination. */
                    rec_obj_ptr2,	/* Source. */
                    sizeof(xsw_object_struct)
                );
            }

	    /* Set new available structure to number 0 (first one). */
	    n = 0;
   	}

	/* Get pointer to recycled object n. */
	rec_obj_ptr = recycled_xsw_object[n];
	if(rec_obj_ptr == NULL)
	    return(-1);


        /* Begin copying the object to the available recycled object
	 * structure n.
	 */

	/* Copy core object values. */
	memcpy(
	    rec_obj_ptr,	/* Destination. */
	    obj_ptr,		/* Source. */
	    sizeof(xsw_object_struct)
	);

	/* Copy elink as needed. */
	if(obj_ptr->elink != NULL)
	{
	    rec_obj_ptr->elink = StringCopyAlloc(obj_ptr->elink);
	}

	/* Do not copy tractored objects, free them. */
	free(obj_ptr->tractored_object);
	obj_ptr->tractored_object = NULL;
	obj_ptr->total_tractored_objects = 0;

        rec_obj_ptr->tractored_object = NULL;
        rec_obj_ptr->total_tractored_objects = 0;


        /* Copy weapons as needed. */ 
        if(rec_obj_ptr->total_weapons > 0)
        {
            /* Allocate weapons pointer array. */
            rec_obj_ptr->weapons = (xsw_weapons_struct **)malloc(
                rec_obj_ptr->total_weapons * sizeof(xsw_weapons_struct *)
            );
            if(rec_obj_ptr->weapons == NULL)
		rec_obj_ptr->total_weapons = 0;

            /* Allocate and copy each weapon structure. */
            for(i = 0; i < rec_obj_ptr->total_weapons; i++)
                rec_obj_ptr->weapons[i] = (xsw_weapons_struct *)MemoryCopyAlloc(
		    obj_ptr->weapons[i],
		    sizeof(xsw_weapons_struct)
		);
        }

        /* Copy scores as needed. */
        if(obj_ptr->score != NULL)
        {
            /* Allocate and copy score structure. */
	    rec_obj_ptr->score = (xsw_score_struct *)MemoryCopyAlloc(
		obj_ptr->score,
		sizeof(xsw_score_struct)
	    );
 	}

        /* Copy economy as needed. */
        if(obj_ptr->eco != NULL)
        {
	    /* Allocate and copy economy data structure. */
	    rec_obj_ptr->eco = (xsw_ecodata_struct *)MemoryCopyAlloc(
		obj_ptr->eco,
                sizeof(xsw_ecodata_struct)
            );
            if(rec_obj_ptr->eco != NULL)
	    {
		/* Copy seconomy products as needed. */
		if(rec_obj_ptr->eco->total_products > 0)
		{
		    rec_obj_ptr->eco->product = (xsw_ecoproduct_struct **)malloc(
			rec_obj_ptr->eco->total_products *
			sizeof(xsw_ecoproduct_struct *)
		    );
                    if(rec_obj_ptr->eco->product == NULL)
			rec_obj_ptr->eco->total_products = 0;

		    for(i = 0; i < rec_obj_ptr->eco->total_products; i++)
		        rec_obj_ptr->eco->product[i] =
			    (xsw_ecoproduct_struct *)MemoryCopyAlloc(
				obj_ptr->eco->product[i],
				sizeof(xsw_ecoproduct_struct)
			    );
                }
	    }
        }


        return(0);
}

/*
 *	Recovers an object in the recycled objects list, returns
 *	the object number recovered as or -1 on failure.
 */
int DBRecoverRecycledObject(const char *name, int owner)
{
	int i, owner_uid, rec_object_num, object_num;
	xsw_object_struct *obj_ptr, *owner_obj_ptr;


	/* Owner must be valid. */
        if(DBIsObjectGarbage(owner))
            return(-1);
	else
	    owner_obj_ptr = xsw_object[owner];


	/* Get permission level of owner object. */
	owner_uid = owner_obj_ptr->permission.uid;

	/* Is owner object allowed to unrecycle? */
	if(owner_uid > ACCESS_UID_UNRECYCLE)
	    return(-3);


        /* Search for object in recycled objects buffer,
	 * starting from oldest.
	 */
        for(i = total_recycled_objects - 1; i >= 0; i--)
        {
            obj_ptr = recycled_xsw_object[i];
            if(obj_ptr == NULL)
                continue;

            if(obj_ptr->type <= XSW_OBJ_TYPE_GARBAGE)
                continue;

            /* Match name. */
            if(strstr(obj_ptr->name, name) == NULL)
                continue;

            /* Got matched recycled object, now check if it is a
	     * player object.
	     */
            if(obj_ptr->type != XSW_OBJ_TYPE_PLAYER)
            {
		/* Recycled object is not a player. */

		/* Check if owner owns this recycled object. */
                if(obj_ptr->owner != owner)
		{
		    /* Owner does not own this object, so check if owner
		     * is allowed to unrecycle other objects.
		     */
		    if(owner_uid > ACCESS_UID_UNRECYCLEO)
			continue;
		}
            }
            else
            {
		/* Recycled object is a player, so check if owner has
		 * create player permission and allowed to unrecycle
		 * others?
		 */
                if((owner_uid > ACCESS_UID_CREATEPLAYER) ||
                   (owner_uid > ACCESS_UID_UNRECYCLEO)
		)
                    continue;
            }

            /* Found recycled object that matches name and owner is
	     * allowed to unrecycle it.
	     */
            break;
        }
        rec_object_num = i;


        /* Did we find anything? */
        if((rec_object_num < 0) ||
           (rec_object_num >= total_recycled_objects)
	)
            return(-1);
	else
	    obj_ptr = recycled_xsw_object[rec_object_num];

	if(obj_ptr == NULL)
	    return(-1);


        /* ******************************************************* */
        /* Begin recover object. */

	/* Create a new object. */
        object_num = DBCreateObject(
            obj_ptr->imageset,
            obj_ptr->type,
            obj_ptr->owner,
            obj_ptr->x,
            obj_ptr->y,
            obj_ptr->z,
            obj_ptr->heading,
            obj_ptr->pitch,
            obj_ptr->bank
        );
        if(DBIsObjectGarbage(object_num))
        {
            /* Could not create object for recovery. */
            return(-3);
        }


        /* Copy the recycled object data to the recovered object. */
        memcpy(
	    xsw_object[object_num],	/* Destination. */
            obj_ptr,			/* Source. */
            sizeof(xsw_object_struct)
        );

        /* NOTE: We copy the pointers to allocated substructures
         * of the object in the recycled buffers list to the newly created
         * recovered object.
         */


        /* Reset the object in the recycle list so its substructure
         * pointers are not free'ed!!
         */
	memset(
	    obj_ptr,
	    0x00,
	    sizeof(xsw_object_struct)
	);
	obj_ptr->type = XSW_OBJ_TYPE_GARBAGE;


	/* Player objects need to own themselves. */
	if(xsw_object[object_num]->type == XSW_OBJ_TYPE_PLAYER)
	    xsw_object[object_num]->owner = object_num;


        return(object_num);
}
