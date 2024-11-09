/*
                       Object Database Management

	Functions:

        int DBIsObjectGarbage(int object_num)

        xsw_object_struct **DBAllocObjectPointers(
		xsw_object_struct **cur_obj_ptrs,
		int num_objects
        )
        xsw_object_struct *DBAllocObject()

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
        int DBCreateObjectByOPM(  
                char *opmname,
                char *name,
                int type,
                double x,
                double y,
                double z,
                double heading,
                double pitch
        )

        void DBDeleteObject(xsw_object_struct *obj_ptr)
        void DBDeleteAllObjects()
        void DBRecycleObject(int object_num)

	void DBReclaim()

	---


 */

#include "../include/isrefs.h"

#include "../include/unvmatch.h"
#include "../include/unvmath.h"
#include "../include/unvutil.h"

#include "swserv.h"
#include "net.h"


/*
 *	Checks if object number is valid, allocated and garbage.
 *
 *	Returns false if object is valid, allocated and non-garbage.
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
 *	Reallocates given object pointer array.
 *
 *	If num_objects <= 0 then cur_obj_ptrs is free'ed if it is not
 *	NULL.
 */
xsw_object_struct **DBAllocObjectPointers(
        xsw_object_struct **cur_obj_ptrs,
	int num_objects   
)
{
        xsw_object_struct **obj_ptrs_rtn = NULL;


	if(num_objects > 0)
	{
            obj_ptrs_rtn = (xsw_object_struct **)realloc(
                cur_obj_ptrs,
                num_objects * sizeof(xsw_object_struct *)
            );
	}
	else
	{
	    /* Free current pointer array since new total is 0. */
	    free(cur_obj_ptrs);
	}

        return(obj_ptrs_rtn);
}

/*
 *	Allocates an object structure, returns NULL on error.
 *
 *	If successful, the allocated object structure will be
 *	reset to default values by a call to UNVResetObject().
 */
xsw_object_struct *DBAllocObject()
{
        xsw_object_struct *obj_ptr;


        obj_ptr = (xsw_object_struct *)calloc(
	    1,
            sizeof(xsw_object_struct)
        );
        if(obj_ptr != NULL)
        {
	    /* Reset values on new object. */
	    UNVResetObject(obj_ptr);
	}

        return(obj_ptr);
}

/*
 *      Creates a new object, returns its number or -1 on error.
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
        int i, object_rtn, prev_total_objects;
        xsw_object_struct *obj_ptr, **ptr;


        /* Type may not be garbage. */
        if(type <= XSW_OBJ_TYPE_GARBAGE)   
        {
            fprintf(stderr,
                "DBCreateObject: Error: Useless request to create garbage.\n"
            ); 
            return(-1);
        }       
        
        /* The isref_num must be valid. */
        if((isref_num < 0) || (isref_num >= ISREF_MAX))
        {
            fprintf(stderr,
 "DBCreateObject: Error: Invalid ISRef number %i.\n",
                isref_num
            );
            return(-1);
        }
        
        
	/* Go through XSW objects list. */        
        for(i = 0, ptr = xsw_object;
            i < total_objects;
            i++, ptr++
	)
        {
            if(*ptr == NULL)
		continue;

            /* Look for garbage objects. */
            if((*ptr)->type == XSW_OBJ_TYPE_GARBAGE)
                break;
        }
        if(i < total_objects)
        {
            /* object_count is the available object. */

            obj_ptr = xsw_object[i];

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
            MuSetUnitVector2D(
		&obj_ptr->attitude_vector_compoent,
		obj_ptr->heading
	    );

            obj_ptr->last_updated = cur_millitime;

            return(i);
        }

        /* No garbage objects, can we allocate more? */
        if((total_objects + OBJECT_ALLOCATE_AHEAD) > MAX_OBJECTS)
            return(-1);


        /* Allocate more objects in memory. */
        prev_total_objects = total_objects;
        total_objects += OBJECT_ALLOCATE_AHEAD;
                
        
        /* Allocate pointer array. */
        xsw_object = (xsw_object_struct **)DBAllocObjectPointers(
            xsw_object, total_objects
	);
        if(xsw_object == NULL)
        {
            total_objects = 0;
            return(-1);
        }
            
        
        /* Allocate each object. */
        for(i = prev_total_objects; i < total_objects; i++)
        {
            xsw_object[i] = (xsw_object_struct *)DBAllocObject();
            if(xsw_object[i] == NULL)
            {
                total_objects = i;
                return(-1);
            }
        }

        /* New object will be the first object allocated above. */
        object_rtn = prev_total_objects;   


        /* Set default values on the new object. */
        obj_ptr = xsw_object[object_rtn];

        obj_ptr->type = type;
        obj_ptr->owner = owner;
        obj_ptr->imageset = isref_num;
        obj_ptr->x = x;
        obj_ptr->y = y;
        obj_ptr->z = z;
        obj_ptr->heading = heading;
        obj_ptr->pitch = pitch;
        obj_ptr->bank = bank;
        MuSetUnitVector2D(
            &obj_ptr->attitude_vector_compoent,
            obj_ptr->heading
        );

        obj_ptr->last_updated = cur_millitime;


        /* Return the number of the newly created object. */
        return(object_rtn);
}


/*
 *      Procedure to allocate an object explicitly, if the object
 *	at the specified index object_num exists, it will be
 *	recycled and then recreated.  If the object does not exist
 *	then it will be created.
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
	int i, object_rtn;
        xsw_object_struct *obj_ptr;


        /* Type may not be garbage or error. */
        if(type <= XSW_OBJ_TYPE_GARBAGE)
        {
            fprintf(stderr,
     "DBCreateExplicitObject(): Useless request to create garbage.\n"
            );
            return(-1);
        }
        /* Make sure object_num is a valid number. */
        else if((object_num < 0) || (object_num >= MAX_OBJECTS))
        {
            fprintf(stderr,
     "DBCreateExplicitObject(): Object number %i out of range.\n",
                object_num
            );
            return(-1);
        }
        /* Make sure isref_num is a valid number. */
        else if((isref_num < 0) || (isref_num >= ISREF_MAX))
        {
            fprintf(stderr,
    "DBCreateExplicitObject(): ISRef number %i out of range.\n",
                isref_num
            );
            return(-1);
        }


        /* *********************************************************** */

        /* If object_num is not garbage, then recycle it. */
        if(!DBIsObjectGarbage(object_num))
            DBRecycleObject(object_num);


        /* Allocate memory if object_num is greater than total_objects. */
        if(object_num >= total_objects)
        {
            /* Can we allocate more? */
            if((object_num + OBJECT_ALLOCATE_AHEAD) >= MAX_OBJECTS)
            {
                fprintf(stderr,
 "DBCreateExplicitObject(): Maximum objects %i reached.\n",
		    MAX_OBJECTS
		);
                return(-3);
            }


            /* Reallocate objects pointer array. */ 
            xsw_object = (xsw_object_struct **)DBAllocObjectPointers(
                xsw_object,
                object_num + OBJECT_ALLOCATE_AHEAD + 1
            );
            if(xsw_object == NULL)
            { 
                total_objects = 0;
                return(-1);
            }

            /* Allocate each new object. */
            for(i = total_objects;
                i <= (object_num + OBJECT_ALLOCATE_AHEAD);
                i++
            )
            {
                xsw_object[i] = (xsw_object_struct *)DBAllocObject();
                if(xsw_object[i] == NULL)
                {
                    total_objects = i;
                    return(-1);
                }
            }

            /* Adjust global total. */
            i = total_objects;
            total_objects = object_num + OBJECT_ALLOCATE_AHEAD + 1;

	    /* Reset newly created objects. */
            while(i < total_objects)
            {
                UNVResetObject(xsw_object[i]);
                i++;
            } 
        }


        /* Create explicit object. */
        object_rtn = object_num;

        /* Set default values on the new object. */
        UNVResetObject(xsw_object[object_rtn]);

        obj_ptr = xsw_object[object_rtn];

        obj_ptr->type = type; 
        obj_ptr->owner = owner;
        obj_ptr->imageset = isref_num;
        obj_ptr->x = x;
        obj_ptr->y = y;
        obj_ptr->z = z;
        obj_ptr->heading = heading;
        obj_ptr->pitch = pitch;
        obj_ptr->bank = bank;
        MuSetUnitVector2D(
            &obj_ptr->attitude_vector_compoent,
            obj_ptr->heading
        );

        obj_ptr->last_updated = cur_millitime;


        return(0);
}


/*
 *      Create an object by Object parameter macro referance.
 */
int DBCreateObjectByOPM(
        char *opmname,
        char *name,
        int type,
        double x,
        double y,
        double z,
        double heading,
        double pitch
)
{
        int object_num, opm_num;
	xsw_object_struct *obj_ptr;


        if((opmname == NULL) ||
           (type <= XSW_OBJ_TYPE_GARBAGE)
        )
            return(-1);


        /* Get OPM num. */
        opm_num = OPMGetByName(opmname, type);
        if(OPMIsGarbage(opm_num))
        {
            fprintf(stderr,
                "DBCreateObjectByOPM(): %s: No such OPM.\n",
                opmname
            );
            return(-1);  
        }


        /* Create new object. */
        object_num = DBCreateObject(
            ISREF_DEFAULT,
            type,
            -1,         /* No owner. */
            x,
            y,
            z,
            heading,
            pitch,
            0.0000
        );
        if(DBIsObjectGarbage(object_num))
        {
            fprintf(stderr,
                "DBCreateObjectByOPM(): Cannot create object.\n"
            );
            return(-1);
        }
	else
	{
	    obj_ptr = xsw_object[object_num];
	}

        /* Set OPM values from opm_num to Object object_num. */
        OPMModelObject(object_num, opm_num);
            
                
        /* Set values. */
        if(name != NULL)
            strncpy(obj_ptr->name, name, XSW_OBJ_NAME_MAX);
        else
            obj_ptr->name[0] = '\0';
	obj_ptr->name[XSW_OBJ_NAME_MAX - 1] = '\0';

        obj_ptr->x = x; 
        obj_ptr->y = y;
        obj_ptr->z = z;

        obj_ptr->heading = heading;
        obj_ptr->pitch = pitch;
        obj_ptr->bank = 0.0000;
        MuSetUnitVector2D(
            &obj_ptr->attitude_vector_compoent,
            obj_ptr->heading
        );

        obj_ptr->animation.last_interval = cur_millitime;


        /* Return the number of the created object. */
        return(object_num);
}

/*
 *      Deletes object's allocated substructures and itself.
 */
void DBDeleteObject(xsw_object_struct *obj_ptr)
{
        if(obj_ptr == NULL)
	    return;

	UNVDeleteObject(obj_ptr);

        return;
}

/*
 *      Procedure to delete all objects referanced in the xsw_object
 *      pointer array.
 */     
void DBDeleteAllObjects()
{
	UNVDeleteAllObjects(xsw_object, total_objects);
	xsw_object = NULL;
	total_objects = 0;

	return;
}

/*
 *      Procedure to delete all allocated substructures on object
 *      and to set the object as garbage. 
 */
void DBRecycleObject(int object_num)   
{
	int i, condescriptor;
        xsw_object_struct *obj_ptr, **ptr;

        char name1[XSW_OBJ_NAME_MAX + 80];
        char name2[XSW_OBJ_NAME_MAX + 80];
        char text[256 + (2 * XSW_OBJ_NAME_MAX)];


        if(DBIsObjectGarbage(object_num))
            return;
        else
            obj_ptr = xsw_object[object_num];


        /* If object was a player, then perform additional checks. */
        if(obj_ptr->type == XSW_OBJ_TYPE_PLAYER)
        {
            /*   Change ownership of all objects owned by this player
             *   object to the global lost_found_owner object.
             */
            if(!DBIsObjectGarbage(unv_head.lost_found_owner))
            {
		/* Go through XSW objects list. */
                for(i = 0, ptr = xsw_object;
                    i < total_objects;
                    i++, ptr++
                )
                {
                    if(*ptr == NULL)
                        continue;
                    if((*ptr)->type <= XSW_OBJ_TYPE_GARBAGE)
                        continue;
                    if((*ptr)->owner != object_num)
                        continue;

		    /* Skip object itself. */
		    if(i == object_num)
			continue;

                    (*ptr)->owner = unv_head.lost_found_owner;

                    strncpy(
			name1,
			DBGetFormalNameStr(i),
			XSW_OBJ_NAME_MAX + 80
		    );
                    name1[XSW_OBJ_NAME_MAX + 80 - 1] = '\0';
                    strncpy(
			name2,
			DBGetFormalNameStr(unv_head.lost_found_owner),
			XSW_OBJ_NAME_MAX + 80
		    );
                    name2[XSW_OBJ_NAME_MAX + 80 - 1] = '\0';

                    sprintf(text,
               "Chowned %s to %s due to recycling of previous owner.",
                        name1, name2
                    );
                    if(sysparm.log_general)
                        LogAppendLineFormatted(fname.primary_log, text);
                }
            }
            /* If no valid lost and found owner, then set to owner 0. */
            else
            {
		/* Go through XSW objects list. */
                for(i = 0, ptr = xsw_object;
                    i < total_objects;
                    i++, ptr++
                )
                {
                    if(*ptr == NULL)
                        continue;
                    if((*ptr)->type <= XSW_OBJ_TYPE_GARBAGE)
                        continue;
                    if((*ptr)->owner != object_num)
                        continue;

                    /* Skip object itself. */
                    if(i == object_num)
                        continue;

                    (*ptr)->owner = 0;

                    sprintf(
			text,
             "Chowned %s to #0 due to recycling of previous owner.",
                        DBGetFormalNameStr(i)
                    );
                    if(sysparm.log_general)
                        LogAppendLineFormatted(fname.primary_log, text);
                }
            }


            /* Close all connections logged in as object_num. */
	    while(1)
	    {
		condescriptor = ConGetByObject(object_num);
		if(condescriptor < 0)
		    break;

                NetCloseConnection(condescriptor);
            }
        }


        /* *********************************************************** */

        /* Reset and free all allocated substructures. */
        UNVResetObject(obj_ptr);

        /* Setting object to type garbage to mark it as recycled. */  
        obj_ptr->type = XSW_OBJ_TYPE_GARBAGE;


        return;
}



/*
 *	Reallocates object pointers and sanitizes object
 *	values as needed.
 *
 *	Also performs other periodic maintainances.
 */        
void DBReclaim()
{
        int i, n, h;
	xsw_object_struct **ptr, *obj_ptr1, *obj_ptr2;


        /* No objects allocated? */
        if((total_objects <= 0) ||
           (xsw_object == NULL)
        )
            return;


        /* Get highest valid and non-garbage object. */
        h = DBGetTopObjectNumber();
        if((h >= 0) && (h < total_objects))
        {
            /* Free all allocated objects from total_objects - 1
	     * to h + 1.
	     */
            for(i = total_objects - 1; i > h; i--)
	    {
                DBDeleteObject(xsw_object[i]);
		xsw_object[i] = NULL;
	    }

            /* Adjust total. */
            total_objects = h + 1;
	    if(total_objects > 0)
	    {
	        /* Reallocate object pointers. */
	        xsw_object = (xsw_object_struct **)DBAllocObjectPointers(
		    xsw_object,
		    total_objects
	        );
                if(xsw_object == NULL)
                {
                    total_objects = 0;
		    return;
	        }
	    }
	    else
	    {
		free(xsw_object);
		xsw_object = NULL;

		total_objects = 0;  
	    }
	}


	/* Perform maintainance on objects. */

	/* Go through XSW objects list. */
	for(i = 0, ptr = xsw_object;
            i < total_objects;
            i++, ptr++
        )
	{
	    obj_ptr1 = *ptr;
	    if(obj_ptr1 == NULL)
		continue;

	    /* Handle player objects. */
	    if(obj_ptr1->type == XSW_OBJ_TYPE_PLAYER)
	    {
		/* Check if player object is not connected and
		 * needs to be hidden from connections.
		 */
		if(sysparm.hide_players)
		{
		    /* Is our player object connected? */
		    if(ConGetByObject(i) < 0)
		    {
			/* Is not connected, check if near a
			 * HOME object.
			 */

			if(!(obj_ptr1->server_options & XSW_OBJF_HIDEFROMCON))
			{
			    /* Go through XSW objects list. */
			    for(n = 0; n < total_objects; n++)
			    {
			        obj_ptr2 = xsw_object[n];
			        if(obj_ptr2 == NULL)
				    continue;

			        if(obj_ptr2->type != XSW_OBJ_TYPE_HOME)
				    continue;

			        if(obj_ptr2 == obj_ptr1)
				    continue;

			        /* Is our object in contact with this object? */
			        if(Mu3DInContactPtr(obj_ptr1, obj_ptr2))
			        {
				    /* Mark it hidden from connections. */
				    obj_ptr1->server_options |=
				        (XSW_OBJF_HIDEFROMCON);
				    break;
				}
			    }
			}
		    }
		    else
		    {
			/* Object is connected. */

			/* Check if it is marked hidden from connections. */
			if(obj_ptr1->server_options & XSW_OBJF_HIDEFROMCON)
			{
			    /* Unset hide from connection flag. */
			    obj_ptr1->server_options &=
				~(XSW_OBJF_HIDEFROMCON);
			}
			/* Check if it is not marked as connected. */
			if(!(obj_ptr1->server_options & XSW_OBJF_CONNECTED))
			{
			    /* Set connected flag. */
			    obj_ptr1->server_options |= XSW_OBJF_CONNECTED;
			}
		    }
		}	/* if(sysparm.hide_players) */
	    }
	    /* Error object. */
	    else if(obj_ptr1->type < XSW_OBJ_TYPE_GARBAGE)
	    {
		/* Not sure how to handle this situation or
		 * if it ever occures.
		 */

	    }

	    /* General checks for all object types. */

	    /* Sanitize power. */
	    if(obj_ptr1->power_max < 0)
		obj_ptr1->power_max = 0;
	    if(obj_ptr1->power > obj_ptr1->power_max)
                obj_ptr1->power = obj_ptr1->power_max;
            if(obj_ptr1->power < 0)
                obj_ptr1->power = 0;

            /* Sanitize hp. */
            if(obj_ptr1->hp > obj_ptr1->hp_max)
                obj_ptr1->hp = obj_ptr1->hp_max;


	}


        return;
}
