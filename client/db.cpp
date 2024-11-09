/*
                     XSW Object Database Management

	Functions:

	int DBIsObjectGarbage(int object_num)

	xsw_object_struct **DBAllocObjectPointers(
		xsw_object_struct **cur_obj_ptrs,
		int num_objects
	);
	xsw_object_struct *DBAllocObject();

        int DBCreateObject(
            int imageset,
            int type,
            int owner,
            double x,
            double y,  
            double z,
            double heading,
            double pitch,
            double bank
        )
        int DBCreateExplicitObject(
            int object_num,
            int imageset,
            int type,
            int owner,
            double x,
            double y,
            double z,
            double heading,
            double pitch,
            double bank
        )

	void DBDeleteObject(xsw_object_struct *obj_ptr)
	void DBDeleteAllObjects()
        void DBRecycleObject(int object_num)

        void DBReclaim()

	---
	
 */

#include "../include/unvmatch.h"
#include "../include/unvmath.h"
#include "../include/unvutil.h"
#include "../include/unvfile.h"

#include "xsw.h"
#include "net.h"


/*
 *      Checks if object number is valid, allocated and garbage.
 *
 *      Returns false if object is valid, allocated and non-garbage.
 */
int DBIsObjectGarbage(int object_num)
{
	return(
	    UNVIsObjectGarbage(
		xsw_object,
		total_objects,
		object_num
	    )
	);
}

/*
 *	Reallocates object pointers.   
 *
 *	Does not change any globals.
 */
xsw_object_struct **DBAllocObjectPointers(
        xsw_object_struct **cur_obj_ptrs,
        int num_objects 
)
{
        xsw_object_struct **ptr = NULL;


	if(num_objects > 0)
	{
            ptr = (xsw_object_struct **)realloc(
                cur_obj_ptrs,
                num_objects * sizeof(xsw_object_struct *)
            );
	}

        return(ptr);
}

/*
 *      Allocates a new object structure with its values reset
 *      to the values of the global garbage object.
 *
 *	Does not change any globals.
 */
xsw_object_struct *DBAllocObject()
{
        xsw_object_struct *obj_ptr;


        /* Allocate a new object structure. */
        obj_ptr = (xsw_object_struct *)calloc(
	    1,
            sizeof(xsw_object_struct)
        );
        if(obj_ptr == NULL)
            return(NULL);
 
        /* Reset values on new object. */
        UNVResetObject(obj_ptr);

        return(obj_ptr);
}

/*      
 *      Creates an object, returns its number or -1 on error.
 */
int DBCreateObject(
	int isref_num,
        int type,
        int owner,
        double x,
        double y,
        double z,
        double heading,
        double pitch,
        double bank
)
{
        int i, n, prev_total;
	xsw_object_struct **ptr, *obj_ptr;


        /* Type may not be garbage. */
        if(type <= XSW_OBJ_TYPE_GARBAGE)
        {
            fprintf(stderr,
                "DBCreateObject: Error: Useless request to create garbage.\n"
            );
            return(-1);
        }

        /* isref_num must be a valid number. */
        if((isref_num < 0) || (isref_num >= ISREF_MAX))
            return(-1);

        /* Load isref_num as needed. */
        if(!ISRefIsLoaded(isref_num))
            ISRefLoad(isref_num);


        /* *********************************************************** */
        
        /* Look for available object already allocated. */
        for(i = 0, ptr = xsw_object;
            i < total_objects;
            i++, ptr++
	)
        {
	    obj_ptr = *ptr;
            if(obj_ptr == NULL)
                continue;  
                
            /* Look for garbage objects. */
            if(obj_ptr->type == XSW_OBJ_TYPE_GARBAGE)
                break;
        }
        if(i < total_objects)
        {
            /* Got available object. */
	    n = i;
	}
	else
	{
	    /* Need to allocate more pointers. */

            /* Can we allocate more? */
            if((total_objects + OBJECT_ALLOCATE_AHEAD) > MAX_OBJECTS)
                return(-1);

            prev_total = total_objects;   
            total_objects += OBJECT_ALLOCATE_AHEAD;

            /* Allocate pointer array. */
            xsw_object = DBAllocObjectPointers(
                xsw_object,
                total_objects
            );
	    if(xsw_object == NULL)
	    {
		total_objects = 0;
		return(-1);
	    }

            /* Allocate each object. */
            for(i = prev_total;
                i < total_objects;
                i++
            )
            {
                xsw_object[i] = DBAllocObject();
                if(xsw_object[i] == NULL)
                {
                    total_objects = i;
                    return(-1);
                }
            }

            /* New object will be the first object allocated. */
            n = prev_total;
	}


	/* Reset values. */
	obj_ptr = xsw_object[n];

        UNVResetObject(obj_ptr);
        obj_ptr->type = type;
        obj_ptr->owner = owner;
        obj_ptr->imageset = isref_num;
        obj_ptr->x = x;
        obj_ptr->y = y;
        obj_ptr->z = z;
        obj_ptr->heading = heading;
        obj_ptr->pitch = pitch;
        obj_ptr->bank = bank;
	MuSetUnitVector2D(&obj_ptr->attitude_vector_compoent, obj_ptr->heading);

        obj_ptr->last_updated = cur_millitime;
        obj_ptr->birth_time = cur_millitime;


	/* Add new object n to in range objects list (as needed). */
	DBInRangeAdd(
	    net_parms.player_obj_num,
	    n,
	    0
	);


        return(n);
}

/*
 *      Creates an object explicitly by number.  If the object exists
 *      then it will be recycled first then recreated.
 *      
 *      Returns non-zero on error.
 */
int DBCreateExplicitObject(
        int object_num,
        int isref_num,
        int type,
        int owner,
        double x,
        double y,
        double z,
        double heading,   
        double pitch,
        double bank
)
{
        int i, n, prev_total;
	xsw_object_struct *obj_ptr;
	char name[XSW_OBJ_NAME_MAX];


        /* Type may not be garbage. */
        if(type <= XSW_OBJ_TYPE_GARBAGE)
        {       
            fprintf(stderr,
       "DBCreateExplicitObject(): Error: Useless request to create garbage.\n"
            );
            return(-1);
        }

        /* Make sure object_num is valid. */
        if(object_num < 0)
        {
            fprintf(stderr,
     "DBCreateExplicitObject(): Error: Request to create object_num #%i.\n",
                (int)object_num
            );   
            return(-1);
        }

        /* The isref_num must be valid. */
        if((isref_num < 0) || (isref_num >= ISREF_MAX))
        {
            fprintf(stderr,        
                "DBCreateExplicitObject(): Error: Invalid isref number %i.\n",
                (int)isref_num
            );
            return(-1);
        }

        /* Load isref as needed. */
        if(!ISRefIsLoaded(isref_num))
            ISRefLoad(isref_num);


        /* *********************************************************** */

        /* Record previous total. */
        prev_total = total_objects;


        /* If object_num already exists and is not garbage, recycle it. */
        if(!DBIsObjectGarbage(object_num))
            DBRecycleObject(object_num);


        /* Allocate more pointers as needed. */
        if(object_num >= total_objects)
        {
            /* Can we allocate more? */   
            if((object_num + OBJECT_ALLOCATE_AHEAD) >= MAX_OBJECTS)
            {
                fprintf(stderr, "Object maximum %i reached.\n", MAX_OBJECTS);
                return(-1);
            }

            /* Adjust global variable total_objects. */
            total_objects = object_num + OBJECT_ALLOCATE_AHEAD + 1;

            /* Allocate more pointers. */
            xsw_object = DBAllocObjectPointers(
                xsw_object,
                total_objects
            );
            if(xsw_object == NULL)
            {
                total_objects = 0;
                return(-1);
            }

            /* Allocate each object. */
            for(i = prev_total;
                i < total_objects;
                i++
            )
            {
                xsw_object[i] = DBAllocObject();
                if(xsw_object[i] == NULL)
                {
                    total_objects = i;
                    return(-1);
                }
            }
        }


        /* Create explicit object. */
        n = object_num;
        obj_ptr = xsw_object[n];


        /*   No need to reset object, if it wasn't garbage, it would be
         *   already reset at the beginning of the call to this function.
         */  
            
        sprintf(name, "Object %i", n);
        strncpy(obj_ptr->name, name, XSW_OBJ_NAME_MAX);
        obj_ptr->name[XSW_OBJ_NAME_MAX - 1] = '\0';

        obj_ptr->type = type;
        obj_ptr->owner = owner;
        obj_ptr->imageset = isref_num;
        obj_ptr->x = x;
        obj_ptr->y = y;
        obj_ptr->z = z;
        obj_ptr->heading = heading;
        obj_ptr->pitch = pitch;
        obj_ptr->bank = bank;
        MuSetUnitVector2D(&obj_ptr->attitude_vector_compoent, obj_ptr->heading);


	obj_ptr->last_updated = cur_millitime;
        obj_ptr->birth_time = cur_millitime;  


        /* Add object n to in range objects list (as needed). */
        DBInRangeAdd(
	    net_parms.player_obj_num,
	    n,
	    0
	);


        return(0);
}

/*
 *      Deletes an object pointed to by obj_ptr and all of its
 *      substructures.
 */
void DBDeleteObject(xsw_object_struct *obj_ptr)
{
        if(obj_ptr == NULL)
            return;

	/* Make sure player object is not referancing garbage. */
	if(obj_ptr == net_parms.player_obj_ptr)
	    DBSetPlayerObject(-1);


        /* Delete object from in range list. */
        DBInRangeDelete(obj_ptr);

        /* Delete object and all its resources. */
	UNVDeleteObject(obj_ptr);


	return;
}

/*
 *      Deletes all objects and any dependant resources including;
 *	global player object referance, viewscreen labels,
 *	and scanner contacts.
 */
void DBDeleteAllObjects()
{
        int i;
	xsw_object_struct **ptr, *obj_ptr;


	/* Go through XSW objects list. */
        for(i = 0, ptr = xsw_object;
            i < total_objects;
            i++, ptr++
        )
        {
	    obj_ptr = *ptr;
	    if(obj_ptr == NULL)
		continue;

            DBDeleteObject(obj_ptr);
        }
        free(xsw_object);
        xsw_object = NULL;

        total_objects = 0;


	/* Delete all entries in the in range objects list. */
	DBInRangeDeleteAll();


        /* Make sure player object is not referancing garbage. */
	DBSetPlayerObject(-1);

        /* Delete all viewscreen labels. */
        VSLabelDeleteAll();

        /* Delete all scanner contacts. */
        ScDeleteAll();


        return; 
}

/*
 *      Recycles the object specified by object_num.
 *
 *	All its values will be reset and any allocated substructures
 *	will be deallocated.
 *
 *	Associated viewscreen label and its entry in the inrange
 *	objects list will be deleted as well. If this object is
 *	the player object, then the global referances to the player
 *	object will be reset.
 */
void DBRecycleObject(int object_num)
{
        xsw_object_struct *obj_ptr;


        /* Is object_num valid or already garbage? */
        if(DBIsObjectGarbage(object_num))
            return;
        else 
            obj_ptr = xsw_object[object_num];


        /* If this is the player object, then unset player object. */
        if(object_num == net_parms.player_obj_num)
	    DBSetPlayerObject(-1);

	/* If this is the object the player object is locked on,
	 * then unlock.
	 */
	if(net_parms.player_obj_ptr != NULL)
	{
	    xsw_object_struct *player_obj_ptr;

	    player_obj_ptr = net_parms.player_obj_ptr;
	    if(player_obj_ptr->locked_on > -1)
	    {
		if(player_obj_ptr->locked_on == object_num)
		{
		    player_obj_ptr->locked_on = -1;

                    BridgeWinDrawPanel(
                        -1,
                        BPANEL_DETAIL_S1
                    );
		    BridgeWinDrawPanel(
                        -1,
                        BPANEL_DETAIL_S2
                    );
		    BridgeWinDrawPanel(
                        -1,
                        BPANEL_DETAIL_S3
                    );
		}
	    }
	}

	/* Delete viewscreen object label for this object. */
	VSLabelDeleteByObjectPtr(obj_ptr);

	/* Remove object from in range objects list as needed. */
	DBInRangeDelete(obj_ptr);


        /* Reset values and deallocate all sub structures. */
        UNVResetObject(obj_ptr);

        /* Setting object_num garbage means it is recycled. */
        obj_ptr->type = XSW_OBJ_TYPE_GARBAGE;


        return;
}

/*
 *      Frees outdated objects and reallocates object pointers.
 */
void DBReclaim()
{
	int i, h, t;
	xsw_object_struct **ptr, *obj_ptr, *player_obj_ptr;


        /* Sanitize total. */
        if(total_objects < 0)
            total_objects = 0;


        /* No objects allocated? */
        if((total_objects == 0) ||
	   (xsw_object == NULL)
	)
            return;


        /* Get highest index numbered object that is valid. */
	for(i = 0, h = -1, ptr = xsw_object;
            i < total_objects;
	    i++, ptr++
	)
	{
	    obj_ptr = *ptr;
	    if(obj_ptr == NULL)
		continue;

	    if(obj_ptr->type <= XSW_OBJ_TYPE_GARBAGE)
		continue;

	    h = i;
	}


        /* Delete all allocated objects from total_objects - 1 to h + 1.
	 *
	 * The global player object referance will also be reset to -1 if
	 * it is deleted in the call to DBDeleteObject(), but this
	 * probably won't happen since it would only be deleted if it was
	 * type garbage or invalid.
	 */
        for(i = total_objects - 1; i > h; i--)
            DBDeleteObject(xsw_object[i]);

        /* Adjust global total_objects. */
        total_objects = h + 1;
	if(total_objects > 0)
	{
	    /* Reallocate pointers. */
            xsw_object = DBAllocObjectPointers(
                xsw_object,
                total_objects
            );
            if(xsw_object == NULL)
            {
                total_objects = 0;
            }
 	}
	else
	{
	    /* Delete all pointers. */
	    free(xsw_object);
	    xsw_object = NULL;

	    total_objects = 0;
	}
  

	/* Get pointer to player object. */
	player_obj_ptr = net_parms.player_obj_ptr;

	/* Go through XSW objects list, begin sanitizing them.
	 * Some objects may need to be recycled.
	 */
        for(i = 0, ptr = xsw_object;
            i < total_objects;
            i++, ptr++
	)
        {
	    obj_ptr = *ptr;
	    if(obj_ptr == NULL)
		continue;

	    if(obj_ptr->type <= XSW_OBJ_TYPE_GARBAGE)
		continue;

	    /* Skip player object past this point. */
	    if(obj_ptr == player_obj_ptr)
		continue;

	    /* If object has not been updated recently, then
	     * recycle it.
	     */
            t = obj_ptr->last_updated + OBJECT_OUTDATED_TIMEOUT;
            if(t < cur_millitime)
                DBRecycleObject(i);

	    /* If object is not in the same sector with the
	     * player object, then definatly recycle it.
	     */
	    if(!Mu3DInSameSectorPtr(obj_ptr, player_obj_ptr))
		DBRecycleObject(i);

        }


	return;
}
