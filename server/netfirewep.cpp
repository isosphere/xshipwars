#include "../include/unvmatch.h"
#include "../include/unvmath.h"
#include "../include/unvutil.h"
#include "../include/swnetcodes.h"

#include "swserv.h"
#include "net.h"

#include "../include/swexporteventscodes.h"
#include "../include/swsoundcodes.h"                              


/*
 *	Sends object values and weapon values of weapon_num on the object
 *	to all connections with nearby (in scanner range) objects.
 */
void NetFireWeaponSyncObjectValues(
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

            /* Send all weapon value to connection. */
	    NetSendObjectMaximums(i, object_num);
	    NetSendObjectValues(i, object_num);
            NetSendWeaponValues(i, object_num, weapon_num);
	}

	return;
}

int NetHandleFireWeapon(int condescriptor, char *arg)
{
	int object_num, con_object_num;
	long sect_x, sect_y, sect_z;
	double x, y, z;
	double heading, pitch, bank;
	double velocity;
	double velocity_heading, velocity_pitch, velocity_bank;
        double freq, yield;

	int selected_weapon;
        int objects_created;
        xsw_object_struct *con_obj_ptr;
	xsw_weapons_struct *sel_wep_ptr;

        char text[(3 * XSW_OBJ_NAME_MAX) + 512];
	const int loc_str_len = 256;
	char loc_str[loc_str_len];


        if(!ConIsLoggedIn(condescriptor))
	    return(-1);

        con_object_num = connection[condescriptor]->object_num;
        if(DBIsObjectGarbage(con_object_num))
            return(-1);
        else
            con_obj_ptr = xsw_object[con_object_num];
 

        /*
         *      SWEXTCMD_FIREWEAPON format:
         *
         *      object_num,
	 *      sect_x, sect_y, sect_z,
	 *      x, y, z,
	 *      heading, pitch, bank,
	 *      velocity,
	 *      velocity_heading, velocity_pitch, velocity_bank,
	 *      freq yield
         */
        sscanf(arg,
"%i\
 %ld %ld %ld\
 %lf %lf %lf\
 %lf %lf %lf\
 %lf\
 %lf %lf %lf\
 %lf %lf",

		&object_num,
		&sect_x, &sect_y, &sect_z,
		&x, &y, &z,
		&heading, &pitch, &bank,
		&velocity,
		&velocity_heading, &velocity_pitch, &velocity_bank,
		&freq, &yield
        );

        /* Connection must own object. */
        if(object_num != con_object_num)
            return(-3);


	/* Update the object that's firing the weapon. */
	con_obj_ptr->sect_x = sect_x;
	con_obj_ptr->sect_y = sect_y;
        con_obj_ptr->sect_z = sect_z;
        con_obj_ptr->x = x;
        con_obj_ptr->y = y;
        con_obj_ptr->z = z;
/*
 Do not set object's attitude, the attitude is for the firing
 direction of the weapon.

        con_obj_ptr->heading = heading;
        con_obj_ptr->pitch = pitch;
        con_obj_ptr->bank = bank;
        MuSetUnitVector2D(
            &con_obj_ptr->attitude_vector_compoent,
            con_obj_ptr->heading
        );
 */

        con_obj_ptr->velocity = velocity;
	con_obj_ptr->velocity_heading = velocity_heading;
        con_obj_ptr->velocity_pitch = velocity_pitch;
        con_obj_ptr->velocity_bank = velocity_bank;
        MuSetUnitVector2D(
            &con_obj_ptr->momentum_vector_compoent,
            con_obj_ptr->velocity_heading
        );

	/* Sanitize detonation yield. */
	if(yield < 0)
	    yield = 0;
	else if(yield > 1)
	    yield = 1;


	/* ********************************************************* */

        /* Get and check if selected weapon is valid. */
        selected_weapon = con_obj_ptr->selected_weapon;

        if((selected_weapon < 0) ||
           (selected_weapon >= con_obj_ptr->total_weapons)
        )
            return(-1);
        if(con_obj_ptr->weapons[selected_weapon] == NULL)
            return(-1);

	sel_wep_ptr = con_obj_ptr->weapons[selected_weapon];


        /* Did we wait the minimum delay before we can fire again? */ 
        if((sel_wep_ptr->delay + sel_wep_ptr->last_used) >=
            cur_millitime
        )
            return(0);
         
           
        /* Cannot fire weapon while cloaked. */
        if(con_obj_ptr->cloak_state == CLOAK_STATE_UP)
        {
/* Send error message? */
            sel_wep_ptr->last_used = cur_millitime;
            return(0);
        }
 
         
        /* If weapon is expendable, see if object_num has any left. */
        if(sel_wep_ptr->emission_type == WEPEMISSION_PROJECTILE)
        {   
            if(sel_wep_ptr->amount <= 0)
                return(0);
        }

        /* Does object have enough power to fire weapon? */
        if(con_obj_ptr->power < sel_wep_ptr->create_power)
            return(0); 


        /* ******************************************************** */
        /* Create the weapon. */
            
        /* Stream weapons. */
        if(sel_wep_ptr->emission_type == WEPEMISSION_STREAM)
        {
            objects_created = WepCreate(
                sel_wep_ptr->ocs_code,		/* OCS. */
                con_object_num,			/* Owner. */
                sel_wep_ptr->emission_type,	/* Emission type. */
		256,				/* Create max. */
                con_obj_ptr->x,
                con_obj_ptr->y,
                heading,
		pitch,
                sel_wep_ptr->power * yield,
                sel_wep_ptr->range,
		freq
            );
        }
        /* Projectile. */
        else if(sel_wep_ptr->emission_type == WEPEMISSION_PROJECTILE)
        {
            objects_created = WepCreate(
                sel_wep_ptr->ocs_code,		/* OCS. */
		con_object_num,			/* Owner. */
                sel_wep_ptr->emission_type,	/* Emission type. */
		sel_wep_ptr->amount,		/* Create max. */
                con_obj_ptr->x,
                con_obj_ptr->y,
                heading,
                pitch,
                sel_wep_ptr->power * yield,
                sel_wep_ptr->range,
		freq
            );
        }
	/* All else assume pulse. */
	else
        {
            objects_created = WepCreate(
                sel_wep_ptr->ocs_code,		/* OCS. */
                con_object_num,			/* Owner. */
                sel_wep_ptr->emission_type,	/* Emission type. */
                256,				/* Create max. */
                con_obj_ptr->x,
                con_obj_ptr->y,
                heading,
                pitch,
                sel_wep_ptr->power * yield,
                sel_wep_ptr->range,
                freq
            );
        }


        /* Were any objects created? */
        if(objects_created <= 0)
            return(objects_created);


	/* Format location string. */            
        UNVLocationFormatString(
            loc_str,
            &con_obj_ptr->sect_x, &con_obj_ptr->sect_y, &con_obj_ptr->sect_z,
	    &con_obj_ptr->x, &con_obj_ptr->y, &con_obj_ptr->z,
            loc_str_len
        );


        /* Log weapons fire. */
	if(sysparm.log_events)
	{
	    sprintf(text,
 "%s: Fired weapon `%s' (ocs %i) at %s",
		DBGetFormalNameStr(con_object_num),
		DBGetOCSOPMName(sel_wep_ptr->ocs_code),
		sel_wep_ptr->ocs_code,
		loc_str
	    );
            LogAppendLineFormatted(fname.primary_log, text);
	}

        /* Mark the last time it was fired. */
        sel_wep_ptr->last_used = cur_millitime;


        /* Consume power on object_num for the power that was
         * needed to create this weapon.
         */
        switch(sel_wep_ptr->emission_type)
        {
          case WEPEMISSION_PROJECTILE:
            con_obj_ptr->power -= sel_wep_ptr->create_power
                * objects_created * yield;

            /* Decrease stock for projectiles. */
            sel_wep_ptr->amount -=
                objects_created;
            if(sel_wep_ptr->amount < 0)
                sel_wep_ptr->amount = 0;

            /* Play appropriate sound. */
            if(!(sel_wep_ptr->flags & XSW_WEP_FLAG_NO_FIRE_SOUND))
              NetSendPlaySound(
                condescriptor,
                ((sel_wep_ptr->fire_sound_code == 0) ?
                    SOUND_CODE_FIRE_PROJECTILE : sel_wep_ptr->fire_sound_code 
                ),
                1.0, 1.0
              );

            /* Export projectile weapons fire. */  
            sprintf(text,
                "%s fired %s at %s.",
		DBGetFormalNameStr(con_object_num),
		DBGetOCSOPMName(sel_wep_ptr->ocs_code),
		loc_str
            );
            ExportEvents(
                fname.events_export,
                EVST_TYPE_PROJECTILEWEAPON_FIRED,
                text
            );
            break;

          case WEPEMISSION_PULSE:
            con_obj_ptr->power -= sel_wep_ptr->create_power *
                objects_created * yield;

            /* Play appropriate sound. */
            if(!(sel_wep_ptr->flags & XSW_WEP_FLAG_NO_FIRE_SOUND))
              NetSendPlaySound(
                condescriptor,
                ((sel_wep_ptr->fire_sound_code == 0) ?
                    SOUND_CODE_FIRE_PULSE : sel_wep_ptr->fire_sound_code
                ),
                1.0, 1.0
              );
            break;

          default:	/* WEPEMISSION_STREAM */
            con_obj_ptr->power -= sel_wep_ptr->create_power *
		objects_created * yield;

            /* Play appropriate sound. */
	    if(!(sel_wep_ptr->flags & XSW_WEP_FLAG_NO_FIRE_SOUND))
              NetSendPlaySound(
                condescriptor,
                ((sel_wep_ptr->fire_sound_code == 0) ?
                    SOUND_CODE_FIRE_STREAM : sel_wep_ptr->fire_sound_code
                ),
                1.0, 1.0
              );
            break;
        }

        /* Sanitize power on object. */
        if(con_obj_ptr->power < 0)
            con_obj_ptr->power = 0;


	/* Report new object and weapons values to objects nearby. */
	NetFireWeaponSyncObjectValues(
	    con_obj_ptr,
	    object_num,
	    selected_weapon
	);


        /* Return the number of weapon objects created. */
        return(objects_created);
}


int NetSendFireWeapon(int condescriptor)
{
	return(0);
}
