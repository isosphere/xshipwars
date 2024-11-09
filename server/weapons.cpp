/*
                       Weapon Fire Object Creation

	Functions:

	void WepSyncCreateObject(
	        xsw_object_struct *obj_ptr,
	        int object_num
	)

	int WepCreate(
		int weapon_type,
		int owner,
		double birth_x,
		double birth_y,
		double birth_heading,
		double birth_pitch
		double power,
		double range,
		double freq
	)

	int WepReclaim(
		int wep_obj,
		int src_obj
	)

	---

	Various weapon fire creation functions.   Creating a weapon
	fire object (such as a torpedo) and sending it off on its way.
	movement.c takes care of the impact and lifespan management
	for these objects.

	EventHandleWeaponFire() is the front end function that should
	be called.   Does all permission checking on object_num.
	Returns number of objects created.
 */

#include "../include/unvmatch.h"
#include "../include/unvmath.h"

#include "swserv.h"
#include "net.h"




/*
 *	Sends create object and object values of the specified
 *	weapon object to all connections with nearby (in scanner
 *	range) objects.
 */
void WepSyncCreateObject(
        xsw_object_struct *obj_ptr,
        int object_num
)
{
        int i, con_obj_num;
        connection_struct **ptr, *con_ptr;
        xsw_object_struct *con_obj_ptr;
                
        
        /* Check if marked hidden from connection. */ 
        if(obj_ptr->server_options & XSW_OBJF_HIDEFROMCON)
            return;


        for(i = 0, ptr = connection;
            i < total_connections;
            i++, ptr++
        )
        {
            con_ptr = *ptr;
            if(con_ptr == NULL)
                continue;

            con_obj_num = con_ptr->object_num;
            if(DBIsObjectGarbage(con_obj_num))
                continue;
            else
                con_obj_ptr = xsw_object[con_obj_num];

            /* Check if objects are valid and in range. */
            if(!Mu3DInRangePtr(con_obj_ptr, obj_ptr,
                con_obj_ptr->scanner_range
            ))
                continue;

            /* Not owned objects, check for visibility. */
            if(obj_ptr->owner != con_obj_num)
            {
                if(DBGetObjectVisibilityPtr(obj_ptr) <= 0.00)
                    continue;
            }   

            /* Send object values to connection. */
            NetSendCreateObject(i, object_num);
            NetSendObjectMaximums(i, object_num);
            NetSendObjectValues(i, object_num);
        }       

	return;
}

/*
 *	Sends set weapon values of the specified weapon number on
 *	the specified object to all connections with nearby (in
 *	scanner range) objects.
 */
void WepSyncObjectWeaponValues(
        xsw_object_struct *obj_ptr,
        int object_num,
	int weapon_num
)
{
        int i, con_obj_num;
        connection_struct **ptr, *con_ptr;
        xsw_object_struct *con_obj_ptr;


        /* Check if marked hidden from connection. */
        if(obj_ptr->server_options & XSW_OBJF_HIDEFROMCON)
            return;


        for(i = 0, ptr = connection;
            i < total_connections;
            i++, ptr++
        )
	{
            con_ptr = *ptr;
            if(con_ptr == NULL)
                continue;

            con_obj_num = con_ptr->object_num;
            if(DBIsObjectGarbage(con_obj_num))
                continue;
            else
                con_obj_ptr = xsw_object[con_obj_num];

            /* Check if objects are valid and in range. */
            if(!Mu3DInRangePtr(con_obj_ptr, obj_ptr,
                con_obj_ptr->scanner_range
            ))
                continue;

            /* Not owned objects, check for visibility. */
            if(obj_ptr->owner != con_obj_num)
            {
                if(DBGetObjectVisibilityPtr(obj_ptr) <= 0.00)
                    continue;
            }

            /* Send weapon values to connection. */
	    NetSendWeaponValues(
		i,
		object_num,
		weapon_num
	    );
	}

	return;
}


/*
 *	Procedure to create a new weapon object.
 */
int WepCreate(
	int ocs_code,		/* Object create script code. */
	int owner,		/* Must be valid. */
        int emission_type,	/* Emission type. */
	int create_max,		/* Don't create more than this many. */
        double birth_x,		/* Pos to be created (not ocs offsetted). */
        double birth_y,
        double birth_heading,	/* Direction to be fired. */
        double birth_pitch,
        double power,
	long range,		/* In Screen units. */
	double freq
)
{
	int objects_created = 0;
	int object_num, ocs_num, opm_num;
	xsw_object_struct *obj_ptr, *owner_obj_ptr, *opm_ptr;
	ocs_struct *ocs_ptr;

	double work_angle;

	double tmp_x, tmp_y, tmp_z;
	double tmp_h, tmp_p, tmp_b;


	/* Cease fire impose? */
	if(sysparm.cease_fire)
	    return(0);

	/* ocs_code must be valid. */
	if(ocs_code <= OCS_TYPE_GARBAGE)
	    return(-1);

	/* emission_type must be valid. */
	if(emission_type < 0)
	{
            fprintf(stderr,
                "WepCreate(): Error: Emission type %i is invalid.\n",
                emission_type
            );
	    return(-1);
	}

	/* Owner must be valid. */
	if(DBIsObjectGarbage(owner))
	    return(-1);
	else
	    owner_obj_ptr = xsw_object[owner];

	/* Sanitize birth_heading. */
	birth_heading = SANITIZERADIANS(birth_heading);
	birth_pitch = SANITIZERADIANS(birth_pitch);

	/* Sanitize power. */
	if(power < 0)
	    power = 0;

	/* Sanitize range. */
	if(range < 0)
	    range = 0;

	/* Sanitize frequency. */
	if(freq < SWR_FREQ_MIN)
	    freq = SWR_FREQ_MIN;
	if(freq > SWR_FREQ_MAX)
	    freq = SWR_FREQ_MAX;


	/* ********************************************************** */

	/* Get Object Create Ccript number by Type. */
	ocs_num = OCSGetByCode(ocs_code);
	if(OCSIsGarbage(ocs_num))
	{
	    fprintf(stderr,
		"WepCreate(): Error: %s requested invalid OCS type %i.\n",
		DBGetFormalNameStr(owner),
		ocs_code
	    );
	    return(-1);
	}
	else
	{
	    /* Get ocs pointer. */
	    ocs_ptr = ocs[ocs_num];
	}

	/* coppies must be 1 or greater. */
	if(ocs_ptr->coppies < 1)
	{
	    /* Do not create anything. */
	    return(0);
	}

	/* Get work_angle. */
        work_angle = SANITIZERADIANS(ocs_ptr->heading);


	/* *********************************************************** */

	/* Get OPM number by OPM name referanced in the OCS. */
	opm_num = OPMGetByName(ocs[ocs_num]->opm_name, -1);
	if(OPMIsGarbage(opm_num))
	{
            fprintf(stderr,
		"WepCreate(): Error: %s: No such OPM.\n",
                ocs_ptr->opm_name
            );
            return(-1);
	}
	else
	{
	    /* Get object parmameter macro. */
	    opm_ptr = opm[opm_num];
	}


	/* Sanitize number of coppies. */
	if((int)ocs_ptr->coppies > MAX_OCS_COPPIES)
	    ocs_ptr->coppies = MAX_OCS_COPPIES;


	/* *********************************************************** */

	/* Begin creating weapons fire objects. */
	while((objects_created < (int)ocs_ptr->coppies) &&
	      (objects_created < create_max)
	)
	{
	    /* Create an object. */
            object_num = DBCreateObject(
                opm_ptr->imageset,
                opm_ptr->type,
                owner,
                birth_x, birth_y, 0,	/* x, y, z. */
                birth_heading, birth_pitch, 0	/* heading, pitch, bank. */
            );
	    if(DBIsObjectGarbage(object_num))
            {
                /* Failed to create object, stop creating. */
                fprintf(stderr,
                    "WepCreate(): DBCreateObject(): Could not create object.\n"
                );
                break;
            }   
	    else
	    {
		/* Increment number of objects created. */
	        objects_created++;
	    }

	    /* Must reget owner and object pointers!! */
            owner_obj_ptr = xsw_object[owner];
            obj_ptr = xsw_object[object_num];


	    /* Record tempory positions and attitudes. */
	    tmp_x = birth_x;
	    tmp_y = birth_y;
	    tmp_z = 0;

	    tmp_h = birth_heading;
	    tmp_p = birth_pitch;
            tmp_b = 0;

            /* Adjust relative position. */
	    tmp_x +=
                MuPolarRotX(
                    birth_heading + work_angle,
		    ocs_ptr->radius
		);
	    tmp_y +=
		MuPolarRotY(
		    birth_heading + work_angle,
		    ocs_ptr->radius
		);


	    /* **************************************************** */

	    /* Set parameters on new object from OPM. */
	    if(OPMModelObject(object_num, opm_num))
	    {
/* What do we do if we can't model if successfully? */
		fprintf(stderr,
    "WepCreate(): Error using OPM #%i for new object #%i.\n",
		    opm_num, object_num
		);
	    }

	    /* Set positions. */
	    obj_ptr->x = tmp_x;
            obj_ptr->y = tmp_y;
	    obj_ptr->z = tmp_z;

            obj_ptr->heading = tmp_h;
            obj_ptr->pitch = tmp_p;
            obj_ptr->bank = tmp_b;
            MuSetUnitVector2D(
                &obj_ptr->attitude_vector_compoent,
                obj_ptr->heading
            );

	    /* Set sector of owner. */
	    obj_ptr->sect_x = owner_obj_ptr->sect_x;
	    obj_ptr->sect_y = owner_obj_ptr->sect_y;
	    obj_ptr->sect_z = owner_obj_ptr->sect_z;

	    /* Set owner. */
	    obj_ptr->owner = owner;

	    /* Set empire. */
            strncpy(obj_ptr->empire,
		owner_obj_ptr->empire,
                XSW_OBJ_EMPIRE_MAX
	    );
	    obj_ptr->empire[XSW_OBJ_EMPIRE_MAX - 1] = '\0';


	    /* Set emission type dependant values. */
	    switch(emission_type)
	    {
	      /*    Stream weapons have size given as range.
	       *    They will intercept locked object of owner.
	       */
	      case WEPEMISSION_STREAM:

                obj_ptr->size = range;

		if(DBIsObjectGarbage(owner_obj_ptr->locked_on))
		{
		    obj_ptr->intercepting_object = -1;
		    obj_ptr->locked_on = -1;
		}
		else
		{
                    obj_ptr->intercepting_object = owner_obj_ptr->locked_on;
		    obj_ptr->locked_on = owner_obj_ptr->locked_on;
		}
		break;

	       /*    Projectiles have size of OPM and intercept
                *    locked object of owner.
                */
	       case WEPEMISSION_PROJECTILE:
/*  Already set by modelling.
		obj_ptr->size = opm_ptr->size;
*/
                if(DBIsObjectGarbage(owner_obj_ptr->locked_on))
		{
                    obj_ptr->intercepting_object = -1;
                    obj_ptr->locked_on = -1;
		}
                else
		{
                    obj_ptr->intercepting_object = owner_obj_ptr->locked_on;
                    obj_ptr->locked_on = owner_obj_ptr->locked_on;
		}
                break;

	      /*    Default to pulse.  They have size of OPM and do NOT
               *    intercept locked object of owner.
               */
	      default:
/*  Already set by modelling.
                obj_ptr->size = opm_ptr->size;
*/
		obj_ptr->intercepting_object = -1;

		break;
	    }


	    /* Set velocites. */
            obj_ptr->velocity = owner_obj_ptr->velocity;
            obj_ptr->velocity_heading = owner_obj_ptr->velocity_heading;
            MuSetUnitVector2D(
                &obj_ptr->momentum_vector_compoent,
                obj_ptr->velocity_heading
            );


	    /* Set thrust as owner's thrust. */
            obj_ptr->thrust = owner_obj_ptr->thrust;

	    /* Set power from given function input. */
            obj_ptr->power = power;
	    obj_ptr->power_max = power;

	    /* Set frequency. */
	    obj_ptr->shield_frequency = freq;

	    /* Mark the time this object was created. */
            obj_ptr->birth_time = cur_millitime;

	    /* Record the ocs code used to create this weapon object. */
	    obj_ptr->creation_ocs = ocs_code;


	    /* ***************************************************** */

	    /* Send create object and object values to all connections
	     * with nearby objects for this weapon object.
	     */
	    WepSyncCreateObject(obj_ptr, object_num);


	    /* Modify work_angle depending on emission_type. */
	    if((emission_type == WEPEMISSION_PROJECTILE) ||
	       (emission_type == WEPEMISSION_PULSE)
	    )
	    {
	        /* Flip work_angle to negative? */
	        if(work_angle > 0)
	        {
		    work_angle *= -1;
	        }
	        /* Flip angle to positive, then decrease angle. */
	        else
	        {
		    work_angle *= -1;
		    work_angle = work_angle *
		        (double)(1 / (double)(ocs_ptr->coppies));
		}
	    }
	}



	/* Return the number of objects created. */
	return(objects_created);
}


/*
 *	Reclaims the weapon object back into the source object,
 *	if possable. This function will check if the weapon object
 *	can be put back into the source object and also checks if
 *	the source object is tractoring the weapon object.
 *
 *	Weapon object is assumed to have no antimatter.
 *
 *	Weapon object may be recycled if it is succesfully put back
 *	into the source object's weapon inventory.
 *
 *	Returns 0 on error or if nothing happened, returns 1 if
 *	the object was reclaimed and recycled.
 */
int WepReclaim(
	int wep_obj,
	int src_obj
)
{
	int i, ocs_code;
	xsw_object_struct *wep_obj_ptr, *src_obj_ptr;
	xsw_weapons_struct *wep_ptr;


	/* Get pointer to weapon object (must be valid). */
	if(DBIsObjectGarbage(wep_obj))
	    return(0);
	else
	    wep_obj_ptr = xsw_object[wep_obj];

	/* Weapon object must be a projectile weapon. */
	if(wep_obj_ptr->type != XSW_OBJ_TYPE_WEAPON)
	    return(0);

	/* Get pointer to source object (must be valid). */
        if(DBIsObjectGarbage(src_obj))
            return(0);
        else
            src_obj_ptr = xsw_object[src_obj];


	/* Check if source object is tractoring weapon object. */
	for(i = 0; i < src_obj_ptr->total_tractored_objects; i++)
	{
	    if(wep_obj == src_obj_ptr->tractored_object[i])
		break;
	}
	if(i >= src_obj_ptr->total_tractored_objects)
	    return(0);


	/* ********************************************************* */

	/* Get ocs code of what he weapon was created with. */
	ocs_code = wep_obj_ptr->creation_ocs;


	/* Search for weapon on source object matching ocs code. */
	for(i = 0; i < src_obj_ptr->total_weapons; i++)
	{
	    wep_ptr = src_obj_ptr->weapons[i];
	    if(wep_ptr == NULL)
		continue;

	    if(wep_ptr->ocs_code == ocs_code)
		break;
	}
	if(i < src_obj_ptr->total_weapons)
	{
	    /* Got match. */

	    wep_ptr = src_obj_ptr->weapons[i];

	    /* Matched weapon must be a projectile type emission. */
	    if(wep_ptr->emission_type == WEPEMISSION_PROJECTILE)
	    {
		/* Room to add one more unit back into inventory? */
		if(wep_ptr->amount < wep_ptr->max)
		{
		    /* Increment units. */
		    wep_ptr->amount++;

		    /* Recycle the weapon object. */
                    DBRecycleObject(wep_obj);
		    NetSendRecycleObject(-1, wep_obj);

		    WepSyncObjectWeaponValues(
			src_obj_ptr,
			src_obj,
			i
		    );

		    return(1);
		}
	    }
	}

	return(0);
}
