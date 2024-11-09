/*
                         Scanner Contacts

	Functions:

	int SC_GET_OBJECT_NUM(xsw_object_struct *obj_ptr)
	int ScIsAllocated(int entry_num)

	int ScAddObjectInContact(xsw_object_struct *obj_ptr)
	void ScRemoveObjectFromContact(xsw_object_struct *obj_ptr)

	int ScIsObjectInContact(xsw_object_struct *obj_ptr)

	void ScDelete(int entry_num)
	void ScDeleteAll()
	void ScReclaim()

	int ScHandleContacts(int object_num)

	---

	Note: scanner drawing is in vsdraw.c and scanner event
	handling is in vsevent.c.

 */

#include "../include/swsoundcodes.h"
#include "../include/unvmath.h"

#include "xsw.h"


#define MIN(a,b)	((a) < (b) ? (a) : (b))
#define MAX(a,b)	((a) > (b) ? (a) : (b))



int SC_GET_OBJECT_NUM(xsw_object_struct *obj_ptr);



/*
 *	Returns the object number of obj_ptr or -1 on error.
 *
 *	Important: If the object is garbage, the valid object number
 *	will still be returned.
 */
int SC_GET_OBJECT_NUM(xsw_object_struct *obj_ptr)
{
	int i;
	xsw_object_struct **ptr;


	if(obj_ptr == NULL)
	    return(-1);

	/* Go through objects list. */
	for(i = 0, ptr = xsw_object;
            i < total_objects;
            i++, ptr++
	)
	{
	    if(*ptr == NULL)
		continue;

	    /* Pointers match? */
	    if(*ptr == obj_ptr)
		return(i);
	}

	/* No match. */
	return(-1);
}

/*
 *	Checks if the scanner contact entry is allocated.
 */
int ScIsAllocated(int entry_num)
{
	if((entry_num >= total_scanner_contacts) ||
           (entry_num < 0) ||
	   (scanner_contact == NULL)
	)
	    return(0);
	else if(scanner_contact[entry_num] == NULL)
	    return(0);
	else
	    return(1);
}

/*
 *	Adds an object to the scanner contact list, allocating
 *	more scanner contact entries as needed.
 *
 *	Warning, does not check if object is already in contact.
 */
int ScAddObjectInContact(xsw_object_struct *obj_ptr)
{
	int i, n;
	scanner_contacts_struct **ptr, *sc_ptr;


	if(obj_ptr == NULL)
	    return(-1);


	/* Sanitize total. */
	if(total_scanner_contacts < 0)
	    total_scanner_contacts = 0;


	/* Check for available allocated entry. */
	for(i = 0, ptr = scanner_contact;
            i < total_scanner_contacts;
            i++, ptr++
	)
	{
	    if(*ptr == NULL)
		break;
	    if(DBIsObjectGarbage((*ptr)->object_num))
		break;
	}
	if(i < total_scanner_contacts)
	{
            /* Got available pointer. */
	    n = i;
	}
	else
	{
	    /* Need to allocate more pointers. */
            n = total_scanner_contacts;
            total_scanner_contacts++;

            /* Reallocate pointers. */
            scanner_contact = (scanner_contacts_struct **)realloc(
		scanner_contact,
                total_scanner_contacts * sizeof(scanner_contacts_struct *)
            );
            if(scanner_contact == NULL)
            {
                total_scanner_contacts = 0;
                return(-1);
            }

	    /* Reset newly allocated pointer so it gets allocated. */
	    scanner_contact[n] = NULL;
	}

	/* Allocate a new structure as needed. */
	if(scanner_contact[n] == NULL)
	{
            scanner_contact[n] = (scanner_contacts_struct *)calloc(
	        1,
	        sizeof(scanner_contacts_struct)
	    );
	    if(scanner_contact[n] == NULL)
	    {
	        return(-1);
	    }
	}


	sc_ptr = scanner_contact[n];

        /* Set values. */
        sc_ptr->object_num = SC_GET_OBJECT_NUM(obj_ptr);


	return(n);
}

/*
 *	Removes an object from the scanner contacts list.
 */
void ScRemoveObjectFromContact(xsw_object_struct *obj_ptr)
{
	int entry_num;


	/*   Check if any scanner contact entries have
         *   object_num in it
	 */
	entry_num = ScIsObjectInContact(obj_ptr);

	/* If it was in the contacts list, remove it. */
	if(ScIsAllocated(entry_num))
	    scanner_contact[entry_num]->object_num = -1;


	return;
}


/*
 *	Checks if object_num is in scanner contacts list.
 *
 *	If it is, then the scanner contact index number is returned
 *	or -1 if the object is not found.
 */
int ScIsObjectInContact(xsw_object_struct *obj_ptr)
{
	int i, object_num;
	scanner_contacts_struct **ptr;


	if(obj_ptr == NULL)
	    return(-1);

	object_num = SC_GET_OBJECT_NUM(obj_ptr);
	if(object_num < 0)
	    return(-1);


	/* Go through scanner contact entries and check for object. */
	for(i = 0, ptr = scanner_contact; 
            i < total_scanner_contacts;
            i++, ptr++)
	{
	    if(*ptr == NULL)
		continue;

	    if((*ptr)->object_num == object_num)
		return(i);
	}


	/* No match. */
	return(-1);
}


/*
 *	Deletes a scanner contact entry.
 */
void ScDelete(int entry_num)
{
	if(!ScIsAllocated(entry_num))
	    return;

	free(scanner_contact[entry_num]);
	scanner_contact[entry_num] = NULL;


	return;
}

/*
 *	Deletes all scanner contact structures.
 */
void ScDeleteAll()
{
	int i;


	for(i = 0; i < total_scanner_contacts; i++)
	    ScDelete(i);

	free(scanner_contact);
	scanner_contact = NULL;

	total_scanner_contacts = 0;


	return;
}

/*
 *	Deallocates unused top entries and reallocates pointers.
 */
void ScReclaim()
{
	int i, h;
	scanner_contacts_struct **ptr;


        /* Get highest entry in use. */
        for(i = 0, h = -1, ptr = scanner_contact;
            i < total_scanner_contacts;
            i++, ptr++
	)
        {
            if(*ptr == NULL)
		continue;

	    /* Object must be valid. */
            if(DBIsObjectGarbage((*ptr)->object_num))
                continue;

            h = i;
        }


        /* Free unused entries. */
        for(i = h + 1;
            i < total_scanner_contacts;
            i++
        )
            ScDelete(i);


        /* Adjust global variable total_scanner_contacts. */
        total_scanner_contacts = h + 1;

        /* Reallocate pointers. */
        if(total_scanner_contacts > 0)
        {
            scanner_contact = (scanner_contacts_struct **)realloc(
                scanner_contact,
                total_scanner_contacts * sizeof(scanner_contacts_struct *)
            );
            if(scanner_contact == NULL)
            {
                total_scanner_contacts = 0;
		return;
            }
        }
        else
        {
            free(scanner_contact);
            scanner_contact = NULL;

            total_scanner_contacts = 0;
        }


	return;
}



/*
 *	Manages scanner contacts.
 *
 *	Calls to this function should be timmed properly, this
 *	function does NOT need to be called every time per loop.
 *
 *	This function is only used on the client to notify
 *	when a new object comes into scanner range of object_num.
 */
int ScHandleContacts(int object_num)
{
        char stringa[XSW_OBJ_NAME_MAX + 128];
	char player_in_nebula = 0;
	int i, status;
	double scan_range, scan_range_norm;	/* In XSW real units. */
        xsw_object_struct *player_obj_ptr, *tar_obj_ptr, **ptr;



	/* Skip if global option specifies not to do scanner notifying. */
	if(!option.notify_scanner_contacts)
	    return(0);

        /* Check if object_num is valid. */
        if(DBIsObjectGarbage(object_num))
            return(-1);
	else
	    player_obj_ptr = xsw_object[object_num];

        /* Get player object's normal scanner range. */
        scan_range_norm = MAX(player_obj_ptr->scanner_range, 0);
        if(option.scanner_limiting)
        {
            if(player_obj_ptr->loc_type == XSW_LOC_TYPE_NEBULA)
	    {
                scan_range_norm *= (double)VISIBILITY_NEBULA;
		player_in_nebula = 1;
	    }
        }


	/* ******************************************************** */
        /* Begin updating contacts list. */

	/* Go through inrange objects list. */
        for(i = 0, ptr = inrange_xsw_object;
            i < total_inrange_objects;
            i++, ptr++
        )
	{
	    tar_obj_ptr = *ptr;
            if(tar_obj_ptr == NULL)
                continue;
	    if(tar_obj_ptr->type <= XSW_OBJ_TYPE_GARBAGE)
		continue;

            /*
	     *   Skip following object types plus outdated ones and
             *   ones, and yourself.
	     */
            if((tar_obj_ptr == player_obj_ptr) ||
               (tar_obj_ptr->type == XSW_OBJ_TYPE_WEAPON) ||
               (tar_obj_ptr->type == XSW_OBJ_TYPE_STREAMWEAPON) ||
               (tar_obj_ptr->type == XSW_OBJ_TYPE_SPHEREWEAPON)
            )
                continue;


            /* Is object in the scanner contacts list? */
	    if(ScIsObjectInContact(tar_obj_ptr) > -1)
	    {
		/* The object is in scanner contact list. */

	        /* Check if in range, skip if outdated. */
                if((tar_obj_ptr->last_updated + OBJECT_OUTDATED_TIMEOUT)
                    < cur_millitime
		)
                {
                    status = 0;	/* Outdated, not in contact. */
                }
		else
		{
                    scan_range = scan_range_norm *
                        DBGetObjectVisibilityPtr(tar_obj_ptr);

                    /* Is target is in nebula and player is not? */
                    if((tar_obj_ptr->type != XSW_OBJ_TYPE_AREA) &&
                       (tar_obj_ptr->loc_type == XSW_LOC_TYPE_NEBULA)
                    )
                    {
                        if(player_in_nebula)
			    status = Mu3DInRangePtr(
				player_obj_ptr, tar_obj_ptr, scan_range
			    );
			else
			    status = 0;
                    }
		    else
		    {
			status = Mu3DInRangePtr(
			    player_obj_ptr, tar_obj_ptr, scan_range
			);
		    }
		}
                /* Is object not in scanner range JUST NOW? */
                if(!status)
                {
		    /* The object JUST LEFT scanner range. */

                    /* Print message. */
		    if((tar_obj_ptr->sect_x == player_obj_ptr->sect_x) &&
                       (tar_obj_ptr->sect_y == player_obj_ptr->sect_y) &&
                       (tar_obj_ptr->sect_z == player_obj_ptr->sect_z)
		    )
		    {
		        sprintf(stringa,
			"%s just left scanner range, bearing %.0f'.",
			    tar_obj_ptr->name,
			    RADTODEG(
                                MuCoordinateDeltaVector(
                                    tar_obj_ptr->x - player_obj_ptr->x,
                                    tar_obj_ptr->y - player_obj_ptr->y
                                )
                            )
                        );
		    }
		    else
                    {
                        sprintf(stringa,
                        "%s just left scanner range, bearing %.0f'.",
                            tar_obj_ptr->name,
                            RADTODEG(
                                MuCoordinateDeltaVector(
                                    tar_obj_ptr->sect_x - player_obj_ptr->sect_x,
                                    tar_obj_ptr->sect_y - player_obj_ptr->sect_y
                                )
                            )
                        );
                    }

                    MesgAdd(stringa, xsw_color.standard_text);

                    /* Remove object from this scanner contact entry. */
                    ScRemoveObjectFromContact(tar_obj_ptr);
		}
	    }
	    else
	    {
                /* The object WAS NOT in scanner range. */

                /* Check if in range, skip if outdated. */
                if((tar_obj_ptr->last_updated + OBJECT_OUTDATED_TIMEOUT)
                    < cur_millitime
		)
                {
                    status = 0;	/* Outdated, not in contact. */
                }
                else
                {
                    scan_range = scan_range_norm *
                        DBGetObjectVisibilityPtr(tar_obj_ptr);

                    /* Is target is in nebula and player is not? */
                    if((tar_obj_ptr->type != XSW_OBJ_TYPE_AREA) &&
                       (tar_obj_ptr->loc_type == XSW_LOC_TYPE_NEBULA)
                    )
                    {
                        if(player_in_nebula)
			    status = Mu3DInRangePtr(
				player_obj_ptr, tar_obj_ptr, scan_range
			    );
			else
			    status = 0;
		    }
		    else
		    {
                        status = Mu3DInRangePtr(
                            player_obj_ptr, tar_obj_ptr, scan_range 
                        );
		    }
                }
                /* Is object in scanner range now? */
                if(status)
                {
                    /* The object just entered scanner range. */

                    /* Print message. */
		    sprintf(stringa,
                        "%s just entered scanner range, bearing %.0f'.",
			tar_obj_ptr->name,
                        RADTODEG(
                            MuCoordinateDeltaVector(
                                tar_obj_ptr->x - player_obj_ptr->x,
                                tar_obj_ptr->y - player_obj_ptr->y
                            )
                        )
		    );
                    MesgAdd(stringa, xsw_color.bold_text);

                    /* Add object to scanner contacts. */
		    ScAddObjectInContact(tar_obj_ptr);

                    /* Play scanner contacts sound. */
                    if(option.sounds >= XSW_SOUNDS_EVENTS)
                    {
                        SoundPlay(
                            SOUND_CODE_CONTACTS_BEEP,
                            1.0,
                            1.0,
			    0,
                            1 
                        );
                    }
		}
	    }
	}


	/* ********************************************************* */

        /* Reclaim scanner contacts memory. */
	ScReclaim();


	return(0);
}
