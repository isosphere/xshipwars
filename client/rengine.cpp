/*
                    Reality Engine for XSW Objects

	Functions:

	void REngMoveObject(xsw_object_struct *obj_ptr, char allow_sect_change)

	void REngHeading(xsw_object_struct *player_obj_ptr, xsw_object_struct *obj_ptr)
	void REngThrottle(xsw_object_struct *player_obj_ptr, xsw_object_struct *obj_ptr)
	void REngThrust(xsw_object_struct *player_obj_ptr, xsw_object_struct *obj_ptr)
	void REngAction(xsw_object_struct *player_object_ptr, xsw_object_struct *obj_ptr)
	void REngLocation(xsw_object_struct *player_obj_ptr, xsw_object_struct *obj_ptr)
	void REngVisibility(xsw_object_struct *player_obj_ptr, xsw_object_struct *obj_ptr)
	void REngShieldVisibility(xsw_object_struct *obj_ptr)
	void REngAnimation(xsw_object_struct *obj_ptr)

	void REngUpdateObject(xsw_object_struct *player_obj_ptr, xsw_object_struct *obj_ptr)

	int REngInit()
	void REngManage()
	void REngShutdown()

	---

 */

#include <math.h>

#include "../include/unvmatch.h"
#include "../include/unvmath.h"
#include "../include/swsoundcodes.h"

#include "xsw.h"
#include "net.h"


#define MIN(a,b)	((a) < (b) ? (a) : (b))
#define MAX(a,b)	((a) > (b) ? (a) : (b))

#define INRANGE_INC(x,min,max)	\
(((x) > (max)) ? (max) : (((x) < (min)) ? (min) : (x)))


/*
 *	Moves the object by calculating one quantom shift
 *	of its 3D coordinates.
 *
 *	If allow_sect_change is true then if the object moves out
 *	if the coordinate bounds, it will change sectors and warp to
 *	the other side.
 */
void REngMoveObject(xsw_object_struct *obj_ptr, char allow_sect_change)
{
	int i;
	char ch_sect_px, ch_sect_nx, ch_sect_py, ch_sect_ny;

	double thrust_dir, thrust_mag, thrust_magc;
	double velocity_mag, velocity_magc;
	double x_vel, y_vel, z_vel;	/* Velocity delta. */
	double x_velc, y_velc, z_velc;	/* Velocity delta /w time compensation. */
	double x_thr, y_thr, z_thr;	/* Thrust. */
	double x_thrc, y_thrc, z_thrc;	/* Thrust delta /w time compensation. */
/*	double x_res, y_res, z_res;	*/	/* Resultant. */

	int owner_object, trac_object;
	xsw_object_struct *owner_obj_ptr;
	xsw_object_struct *trac_obj_ptr;


        /* Stream weapons have location of their owner. */
        if((obj_ptr->type == XSW_OBJ_TYPE_STREAMWEAPON) ||
           (obj_ptr->type == XSW_OBJ_TYPE_SPHEREWEAPON)
	)
        {
            owner_object = obj_ptr->owner;
            if(!DBIsObjectGarbage(owner_object))
            {
		owner_obj_ptr = xsw_object[owner_object];
                obj_ptr->x = owner_obj_ptr->x;
                obj_ptr->y = owner_obj_ptr->y;
                obj_ptr->z = owner_obj_ptr->z;

		obj_ptr->sect_x = owner_obj_ptr->sect_x;
                obj_ptr->sect_y = owner_obj_ptr->sect_y;
                obj_ptr->sect_z = owner_obj_ptr->sect_z;
            }
            return;
        }


        /* ***************************************************** */
        /* Begin standard object movement. */

        /* Sanitize velocity heading. */
	obj_ptr->velocity_heading = SANITIZERADIANS(obj_ptr->velocity_heading);

        /* Calculate movement from velocity (momentum).
         * Fetch velocity_mag, with compensation for lapsed time.
         */
        velocity_magc = obj_ptr->velocity * time_compensation;
        velocity_mag = obj_ptr->velocity;

	/* Get momentum (velocity) vector compoents (with and
	 * without time compensation) from momentum unit vector
	 * compoents.
	 */
	x_velc = obj_ptr->momentum_vector_compoent.i * velocity_magc;
	y_velc = obj_ptr->momentum_vector_compoent.j * velocity_magc;
	z_velc = obj_ptr->momentum_vector_compoent.k * velocity_magc;

	x_vel = obj_ptr->momentum_vector_compoent.i * velocity_mag;
	y_vel = obj_ptr->momentum_vector_compoent.j * velocity_mag;
	z_vel = obj_ptr->momentum_vector_compoent.k * velocity_mag;


	/* Calculate (not object relative) thrust direction. */
	thrust_dir = SANITIZERADIANS(
	    obj_ptr->heading +
	    obj_ptr->thrust_dir +
	    PI
	);


        /* Calculate thrust magnitude, taking into account the
         * object's thrust power, current power, and power
         * purity.
         */
        if(obj_ptr->power_max > 0)
            thrust_mag = obj_ptr->thrust * obj_ptr->power_purity *
                obj_ptr->power / obj_ptr->power_max;
        else
            thrust_mag = obj_ptr->thrust * obj_ptr->power_purity;

        /* Sanitize thrust magnitude. */
/*
        if((thrust_mag + obj_ptr->velocity) > obj_ptr->velocity_max)
           thrust_mag = obj_ptr->velocity_max - obj_ptr->velocity;
 */

        /* Calculate thrust magnitude with time_compensation. */
        thrust_magc = thrust_mag * time_compensation;


	/* Get thrust vector compoents with and without time
	 * compensation.
	 */
	x_thrc = thrust_magc * sin(thrust_dir);
        y_thrc = thrust_magc * cos(thrust_dir);
        z_thrc = 0;

        x_thr = thrust_mag * sin(thrust_dir);
        y_thr = thrust_mag * cos(thrust_dir);
        z_thr = 0;


        /* Move object by adding the calculated velocity and thrust
         * vector compoents.
         */
        obj_ptr->x += x_velc + x_thrc;
        obj_ptr->y += y_velc + y_thrc;
/*      obj_ptr->z += z_velc + z_thrc; */
/* Force set z to 0. */
	obj_ptr->z = 0;


	/* Do sector change as needed. */
	if(allow_sect_change)
	{
	    ch_sect_px = 0;
	    ch_sect_nx = 0;
            ch_sect_py = 0;
	    ch_sect_ny = 0;
/*
            ch_sect_pz = 0;
            ch_sect_nz = 0; 
 */

	    if(obj_ptr->x < sector_legend.x_min)
	    {
	        obj_ptr->x += sector_legend.x_len;
	        obj_ptr->sect_x -= 1;
		ch_sect_nx = 1;		/* Negative sect x change. */
	    }
            else if(obj_ptr->x > sector_legend.x_max)
            {
                obj_ptr->x -= sector_legend.x_len;
                obj_ptr->sect_x += 1;
                ch_sect_px = 1;		/* Positive sect x change. */
            }

            if(obj_ptr->y < sector_legend.y_min)
            {
                obj_ptr->y += sector_legend.y_len;
                obj_ptr->sect_y -= 1;
                ch_sect_ny = 1;		/* Negative sect y change. */
            }
            else if(obj_ptr->y > sector_legend.y_max)
            {
                obj_ptr->y -= sector_legend.y_len;
                obj_ptr->sect_y += 1;
                ch_sect_py = 1;		/* Positive sect y change. */
            }

/*
            if(obj_ptr->z < sector_legend.z_min)
            {
                obj_ptr->z += sector_legend.z_len;
                obj_ptr->sect_z -= 1;
                ch_sect_nz = 1;		Negative sect z change.
            }
            else if(obj_ptr->z > sector_legend.z_max)
            {
                obj_ptr->z -= sector_legend.z_len;
                obj_ptr->sect_z += 1;
                ch_sect_pz = 1;		Positive sect z change.
            }
*/

	    /* Check if there was a sector change. */
	    if(ch_sect_nx || ch_sect_px ||
               ch_sect_ny || ch_sect_py
/*
	       ch_sect_nz || ch_sect_pz
*/
	    )
	    {
		/* Update nessasary resources per sector change. */

		/* Change sector for tractored objects. */
		for(i = 0; i < obj_ptr->total_tractored_objects; i++)
		{
		    trac_object = obj_ptr->tractored_object[i];
		    if(DBIsObjectGarbage(trac_object))
			continue;
                    else
			trac_obj_ptr = xsw_object[trac_object];

		    trac_obj_ptr->sect_x = obj_ptr->sect_x;
                    trac_obj_ptr->sect_y = obj_ptr->sect_y;
/*                  trac_obj_ptr->sect_z = obj_ptr->sect_z; */

		    if(ch_sect_nx)
			trac_obj_ptr->x += sector_legend.x_len;
		    if(ch_sect_px)
			trac_obj_ptr->x -= sector_legend.x_len;

                    if(ch_sect_ny)
                        trac_obj_ptr->y += sector_legend.y_len;
                    if(ch_sect_py)
                        trac_obj_ptr->y -= sector_legend.y_len;
/*
                    if(ch_sect_nz)
                        trac_obj_ptr->z += sector_legend.z_len;
                    if(ch_sect_pz)
                        trac_obj_ptr->z -= sector_legend.z_len;
*/
		}

		/* Check if this is the player object. */
		if(obj_ptr == net_parms.player_obj_ptr)
		{
		    /* Send out sector position to server. */
		    NetSendObjectSect(net_parms.player_obj_num);

		    XSWDoChangeSector(net_parms.player_obj_num);
		}
	    }
	}


        /*
         *   Record new velocity and velocity_heading on object.
         *   This value must not have the time compensation modifications
         *   to it.
         */     
        obj_ptr->velocity_heading = MuCoordinateDeltaVector(
            x_vel + x_thr,
            y_vel + y_thr
        );
        MuSetUnitVector2D(
            &obj_ptr->momentum_vector_compoent,
            obj_ptr->velocity_heading
        );

        obj_ptr->velocity = Mu3DDistance(
            x_vel + x_thr,
            y_vel + y_thr,
            0
        );

	return;
}

/*
 *	Updates object heading.
 */
void REngHeading(xsw_object_struct *player_obj_ptr, xsw_object_struct *obj_ptr)
{
	double x;
	double target_heading;
	double rel_heading;
	int tobj_num;
	xsw_object_struct *tar_obj_ptr;


	/* Skip static objects. */
        if(obj_ptr->type == XSW_OBJ_TYPE_STATIC)
           return;


	/* ********************************************************** */
	/* Update heading depending on type. */

	/* Stream weapons. */
	if(obj_ptr->type == XSW_OBJ_TYPE_STREAMWEAPON)
	{
	    /*   Update heading for stream weapons so that they fire
             *   more realistically.
	     */
	    tobj_num = obj_ptr->intercepting_object;
	    if(!DBIsObjectGarbage(tobj_num))
            {
                /* Get rel_heading to the object we're intercepting. */
                tar_obj_ptr = xsw_object[tobj_num];


		/* Check if in the same sector. */
		if(Mu3DInSameSectorPtr(tar_obj_ptr, obj_ptr))
                {
                    target_heading = MuCoordinateDeltaVector(
                        tar_obj_ptr->x - obj_ptr->x,
                        tar_obj_ptr->y - obj_ptr->y
                    );
                }
                else
                {
                    target_heading = MuCoordinateDeltaVector(
                        tar_obj_ptr->sect_x - obj_ptr->sect_x,
                        tar_obj_ptr->sect_y - obj_ptr->sect_y
                    );
                }


                rel_heading = target_heading - obj_ptr->heading;

                if((rel_heading > 3.1415927) || (rel_heading < -3.1415927))
                {
                    /* rel_heading must be within the turnrate of object. */
                    if(rel_heading > (100 * obj_ptr->turnrate * time_compensation))
                        rel_heading = 100 * obj_ptr->turnrate *
                            time_compensation;
                    else if(rel_heading < (-100 * obj_ptr->turnrate * time_compensation))
                        rel_heading = -100 * obj_ptr->turnrate *
                            time_compensation;

                    obj_ptr->heading -= rel_heading;
                }
                else
                {
                    /* rel_heading must be within the turnrate of object. */
                    if(rel_heading > (100 * obj_ptr->turnrate * time_compensation))
                        rel_heading = 100 * obj_ptr->turnrate *
                            time_compensation;
                    else if(rel_heading < (-100 * obj_ptr->turnrate * time_compensation))
                        rel_heading = -100 * obj_ptr->turnrate *
                            time_compensation;

                    obj_ptr->heading += rel_heading;
                }
	    }
	}
	/* Sphere weapons do not change heading. */
	else if(obj_ptr->type == XSW_OBJ_TYPE_SPHEREWEAPON)
	{


	}
	/* Player object. */
	else if(obj_ptr == player_obj_ptr)
	{
	    /*   Change heading only if not in omni directional thrust
             *   mode.
	     */
	    if(!gctl[0].omni_dir_thrust)
	    {
		x = gctl[0].turn;

	        /* Adjust heading according to controller position. */
	        obj_ptr->heading += x * 100 *
		    obj_ptr->turnrate * time_compensation;

	        /* Switch off intercept if turn is too great. */
	        if((x > 0.3) || (x < -0.3))
	        {
		    if(obj_ptr->intercepting_object > -1)
		    {
		        NetSendIntercept(
			    net_parms.player_obj_num,
			    "#off"
			);
		    }
		}
	    }
	}


        /* Sanitize heading. */
	obj_ptr->heading = SANITIZERADIANS(obj_ptr->heading);
        MuSetUnitVector2D(
            &obj_ptr->attitude_vector_compoent,
            obj_ptr->heading
	);


	return;
}



/*
 *	Updates the throttle by checking the game controller
 *	positions.
 */
void REngThrottle(
	xsw_object_struct *player_obj_ptr,
	xsw_object_struct *obj_ptr
)
{
        /* Skip static objects. */
        if(obj_ptr->type == XSW_OBJ_TYPE_STATIC)
           return;


        /* Update heading for local player object. */
	if(obj_ptr == player_obj_ptr)
	{
            /* Check if omni directional thrust is on. */
            if(gctl[0].omni_dir_thrust)
	    {
		/* Omni directional thrust is on. */

		switch(option.throttle_mode)
		{
		  case THROTTLE_MODE_INCREMENTAL:
		    /* Not sure of the best way to do this yet... */
		    break;

		  /* THROTTLE_MODE_BIDIRECTIONAL or THROTTLE_MODE_NORMAL */
		  default:
		    obj_ptr->thrust_dir = SANITIZERADIANS(
		        (PI / 2) - atan2(gctl[0].throttle, gctl[0].turn) + PI
		    );

		    obj_ptr->throttle = INRANGE_INC(
		        hypot(gctl[0].turn, gctl[0].throttle), 0, 1
		    );
		    break;
		}
	    }
	    else
	    {
		/* Omni directional thrust is off. */

		switch(option.throttle_mode)
		{
                  case THROTTLE_MODE_INCREMENTAL:
		    if((obj_ptr->thrust_dir > (PI * 0.5)) &&
                       (obj_ptr->thrust_dir < (PI * 1.5))
		    )
		    {
			/* Thrusters going backwards (normal). */
                        obj_ptr->throttle += (gctl[0].throttle
			    * 0.01 * time_compensation
		        );

                        obj_ptr->thrust_dir = SANITIZERADIANS(
                            ((0.5 * PI) * gctl[0].thrust_dir) + PI
                        );

			/* Adjust throttle and set thrust dir. */
			if(obj_ptr->throttle < 0)
                        {
                            obj_ptr->throttle *= -1;
                            obj_ptr->thrust_dir = SANITIZERADIANS(   
                                obj_ptr->thrust_dir + PI 
                            );
			}
		    }
		    else
		    {
                        /* Thrusters going forwards (reverse). */
                        obj_ptr->throttle += (gctl[0].throttle
                            * -0.01 * time_compensation
                        );

                        obj_ptr->thrust_dir = SANITIZERADIANS(
                            ((0.5 * PI) * gctl[0].thrust_dir)
                        );

			/* Adjust throttle and set thrust dir. */
                        if(obj_ptr->throttle < 0)
                        {
                            obj_ptr->throttle *= -1;
			    obj_ptr->thrust_dir = SANITIZERADIANS(
				obj_ptr->thrust_dir + PI
			    );
                        }
		    }
		    break;

		  /* THROTTLE_MODE_BIDIRECTIONAL or THROTTLE_MODE_NORMAL */
		  default:
		    obj_ptr->throttle = gctl[0].throttle;
		    if(obj_ptr->throttle < 0)
		    {
                        obj_ptr->thrust_dir = SANITIZERADIANS(
                            ((0.5 * PI) * gctl[0].thrust_dir)
			);
                        obj_ptr->throttle *= -1;
		    }
		    else
		    {
			obj_ptr->thrust_dir = SANITIZERADIANS(
                            ((0.5 * PI) * gctl[0].thrust_dir) + PI
                        );
		    }
		    break;
		}
	    }
	}

        /* Sanitize throttle. */
        if(obj_ptr->throttle < 0)
            obj_ptr->throttle = 0;
        else if(obj_ptr->throttle > 1)
            obj_ptr->throttle = 1;

	return;
}

/*
 *	Thrust, antimatter consumption, drag, and other calculations
 *	of velocity are performed here.
 */
void REngThrust(xsw_object_struct *player_obj_ptr, xsw_object_struct *obj_ptr)
{
	char just_stopped = 0;
	double throttle;
	int i, n;
        xsw_object_struct **tar_obj_ptr;


        /* Skip objects whick do not move. */
        if((obj_ptr->type == XSW_OBJ_TYPE_STATIC) ||
           (obj_ptr->type == XSW_OBJ_TYPE_HOME) ||
           (obj_ptr->type == XSW_OBJ_TYPE_AREA) ||
           (obj_ptr->type == XSW_OBJ_TYPE_WORMHOLE) ||
           (obj_ptr->type == XSW_OBJ_TYPE_ELINK)
	)
           return;


	/* **************************************************** */
	/* Get throttle. */

	/* Check if object is the player object. */	
	if(player_obj_ptr == obj_ptr)
	{
	    /* Are engines off? */
	    if(obj_ptr->engine_state == ENGINE_STATE_ON)
		throttle = obj_ptr->throttle;
	    else
	        throttle = 0;
	}
	else
	{
	    /* Not player object. */

	    /* Fetch throttle based on object type. */
	    switch(obj_ptr->type)
	    {
	      /* Projectile weapons always throttle at full. */
	      case XSW_OBJ_TYPE_WEAPON:
		throttle = 1.0;
		break;

	      /* Stream weapon have no throttle value. */
	      case XSW_OBJ_TYPE_STREAMWEAPON:
		throttle = 0.0;
		break;

	      /* Same as stream weapon. */
              case XSW_OBJ_TYPE_SPHEREWEAPON:
                throttle = 0.0;
                break;

	      /* All else get exact value. */
	      default:
		throttle = obj_ptr->throttle;
		break;
	    }
	}

	/* ******************************************************** */
	/* Thrust. */

        /* 
         *   Thrust is calculated by taking the coefficent of the
         *   throttle and multiplying that with the thrust_power.
         */
	obj_ptr->thrust = MAX(
	    throttle * obj_ptr->thrust_power * time_compensation,
	    0
	);

	/* Thrust output should be 0 if no antimatter left. */
	if(obj_ptr->antimatter <= 0)
	    obj_ptr->thrust = 0;


        /* ******************************************************** */
        /* Drag. */

        if(player_obj_ptr == obj_ptr)
        {
	    /* For player object. */

	    if(obj_ptr->velocity > 0)
	    {
                if(obj_ptr->velocity_max > 0)
		{
		    /* Check if external dampers are on. */
		    if(gctl[0].external_dampers)
                        obj_ptr->velocity -= (obj_ptr->thrust_power *
                            obj_ptr->power_purity * time_compensation
			);
		    else
                        obj_ptr->velocity -= ((obj_ptr->velocity /
                            obj_ptr->velocity_max) * obj_ptr->thrust_power *
                            time_compensation
                        );
		}
                else
                    obj_ptr->velocity = 0;

		if(obj_ptr->velocity < 0.0001)
		{
		    obj_ptr->velocity = 0;
		    just_stopped = 1;
		}
	    }
	}
	else
	{
            if(obj_ptr->velocity > 0)
            {
	        if(obj_ptr->velocity_max > 0)
	            obj_ptr->velocity -= ((obj_ptr->velocity /
		        obj_ptr->velocity_max) * obj_ptr->thrust_power *
		        time_compensation
		    );
	        else
	            obj_ptr->velocity = 0;

		if(obj_ptr->velocity <= 0)
		    obj_ptr->velocity = 0;
	    }
	}


	/* ******************************************************** */
	/* If velocity is such a small number, then set it 0. */
	if(just_stopped)
	{
	    /* Vessel has just stopped moving. */
	    if(obj_ptr == player_obj_ptr)
	    {
		/* Local player object has just stopped moving. */

		/* Find nearest HOME object in inrange objects list. */
		for(i = 0, tar_obj_ptr = inrange_xsw_object;
                    i < total_inrange_objects;
                    i++, tar_obj_ptr++
		)
		{
		    if(*tar_obj_ptr == NULL)
			continue;

		    if(((*tar_obj_ptr)->type != XSW_OBJ_TYPE_HOME) &&
                       ((*tar_obj_ptr)->type != XSW_OBJ_TYPE_WORMHOLE) &&
                       ((*tar_obj_ptr)->type != XSW_OBJ_TYPE_ELINK)
		    )
			continue;

		    if(Mu3DInContactPtr(obj_ptr, *tar_obj_ptr))
			break;
		}
		if(i < total_inrange_objects)
		{
		    /* Got nearest HOME, WORMHOLE, or ELINK object. */

		    n = DBGetObjectNumByPtr(inrange_xsw_object[i]);
		    if(n > -1)
		    {
		        /* Map eco window by requesting economy values. */
		        if(option.auto_map_eco_win &&
                           (xsw_object[n]->type == XSW_OBJ_TYPE_HOME)
		        )
		        {
			    NetSendReqName(n);
			    NetSendEcoReqValues(net_parms.player_obj_num, n);
		        }
			else if(xsw_object[n]->type == XSW_OBJ_TYPE_WORMHOLE)
			{
			    if(next.wormhole_enter <= cur_millitime)
			    {
			        NetSendWormHoleEnter(
				    net_parms.player_obj_num,
				    n
				);
				next.wormhole_enter = cur_millitime +
				    WORMHOLE_ENTER_DELAY;
			    }
			}
                        else if(xsw_object[n]->type == XSW_OBJ_TYPE_ELINK)
                        {
			    NetSendELinkEnter(net_parms.player_obj_num, n);
                        }
		    }
		}
	    }
	}


	return;
}



/*
 *	Performs action as needed on object by it's parameter's
 *	values.
 *
 *	Weapons fire firing is handled here.
 */
void REngAction(
	xsw_object_struct *player_object_ptr,
	xsw_object_struct *obj_ptr
)
{
	int sel_wep_num;
	long dt;
	xsw_weapons_struct *wep_ptr;


	/* Check if this is the player object. */
	if(obj_ptr == player_object_ptr)
	{
	    /* ***************************************************** */
	    /* Button1: Weapons fire. */
	    while(gctl[0].fire_weapon)
	    {
                /* Get pointer to selected weapon. */
                sel_wep_num = obj_ptr->selected_weapon;
                if((sel_wep_num < 0) || (sel_wep_num >= obj_ptr->total_weapons))
                    break;

                wep_ptr = obj_ptr->weapons[sel_wep_num];
                if(wep_ptr == NULL)
                    break;


		/* Check if weapons are online. */
		if(!local_control.weapons_online)
		{
		    /* Weapons are offline, play error sound. */
		    dt = wep_ptr->last_used + wep_ptr->delay;
		    if(dt <= cur_millitime)
		    {
			BridgeWarnWeaponsOffline(net_parms.player_obj_num);

			wep_ptr->last_used = cur_millitime;
		    }
		    break;
		}


		/* Check if we've waited the weapon's delay period. */
		dt = wep_ptr->last_used + wep_ptr->delay;
		if(dt >= cur_millitime)
		    break;


		/* Send the fire weapon command to server. */
		if(NetSendFireWeapon(
		    net_parms.player_obj_num,	/* We know this is the player. */
		    local_control.weapon_freq
		))
		    break;

		/* Mark last millitime this weapon was used.
		 * The last used time sent by the server is ignored and
		 * the client keeps track of the last used time.  Since
		 * the firing times and weapon delay times are synced
		 * (or atleast reasonably synced) there shouldn't be any
		 * problem.
		 */
		wep_ptr->last_used = cur_millitime;

		break;
	    }

	    /* ************************************************************ */
	    /* Aim weapon heading. */

            /* Get pointer to selected weapon. */
            sel_wep_num = obj_ptr->selected_weapon;
	    wep_ptr = NULL;
            if((sel_wep_num >= 0) && (sel_wep_num < obj_ptr->total_weapons))
		wep_ptr = obj_ptr->weapons[sel_wep_num];

	    if(wep_ptr == NULL)
	    {
		local_control.weapon_fire_heading = 0;
	    }
	    else if(wep_ptr->flags & XSW_WEP_FLAG_FIXED)
	    {
                local_control.weapon_fire_heading = 0;
            }
	    else
	    {
	        local_control.weapon_fire_heading =
		    SANITIZERADIANS(
		        local_control.weapon_fire_heading +
		        (gctl[0].aim_weapon_heading *
			    (2 * PI / 180) *
		        time_compensation)
		    );
	    }
	}


	return;
}



/*
 *      Performs an update location procedure on the object.
 *      movement style depends on the type of the object and several
 *      other attributes.
 *
 *      REngMoveObject() will be called to do the actual moving.
 */
void REngLocation(
	xsw_object_struct *player_obj_ptr,
	xsw_object_struct *obj_ptr
)
{
        char stringa[XSW_OBJ_NAME_MAX + 128];
	double distance;
	int i;
	xsw_object_struct **tar_obj_ptr;
	xsw_object_struct *nebula_obj_ptr = NULL;


	/* Look for area type objects in inrange objects list. */
	for(i = 0, tar_obj_ptr = inrange_xsw_object;
            i < total_inrange_objects;
            i++, tar_obj_ptr++
	)
	{
	    if(*tar_obj_ptr == NULL)
		continue;

	    /* Check only objects of type XSW_OBJ_TYPE_AREA. */
	    if((*tar_obj_ptr)->type != XSW_OBJ_TYPE_AREA)
		continue;

            /* Skip if object is the player object. */
            if(*tar_obj_ptr == player_obj_ptr)
                continue;

	    /* What type of area? */
	    switch((*tar_obj_ptr)->loc_type)
	    {
	      case XSW_LOC_TYPE_NEBULA:
		/* Not in same sector? */
                if((obj_ptr->sect_x != (*tar_obj_ptr)->sect_x) ||
                   (obj_ptr->sect_y != (*tar_obj_ptr)->sect_y) ||
                   (obj_ptr->sect_z != (*tar_obj_ptr)->sect_z)
		)
		    break;

		distance = Mu3DDistance(
		    (*tar_obj_ptr)->x - obj_ptr->x,
		    (*tar_obj_ptr)->y - obj_ptr->y,
		    (*tar_obj_ptr)->z - obj_ptr->z
		) * 1000;
		if(distance > (*tar_obj_ptr)->size)
		    break;

                nebula_obj_ptr = *tar_obj_ptr;
		break;

	      default:
		break;
	    }
	}

	/* Check if object left a nebula. */
	if(nebula_obj_ptr == NULL)
        {
            /*   If object_num is the player and it is in a
             *   nebula. Print notify message.
             */
            if(obj_ptr->loc_type != XSW_LOC_TYPE_SPACE)
	    {
		obj_ptr->loc_type = XSW_LOC_TYPE_SPACE;

		if(player_obj_ptr == obj_ptr)
	        {
                    MesgAdd("Leaving nebula.", xsw_color.standard_text);

                    /* Recreate scanner location type label. */
		    ScannerUpdateLabels(net_parms.player_obj_num);

		    /* Change background music. */
		    if(option.music)
		        XSWDoChangeBackgroundMusic();
	        }
	    }
        }
	else
	{
	    /*   If object_num is the player and it is not already
             *   in nebula. Print notify message.
	     */
	    if(obj_ptr->loc_type != XSW_LOC_TYPE_NEBULA)
	    {
                obj_ptr->loc_type = XSW_LOC_TYPE_NEBULA;

                if(player_obj_ptr == obj_ptr)
	        {
		    if(option.show_formal_label == 2)
                        sprintf(stringa,
                            "Entering `%s'.",   
                            nebula_obj_ptr->name
                        );
		    else
		        sprintf(stringa,
			    "Entering `%s'.",
		            nebula_obj_ptr->name
		        );
		    MesgAdd(stringa, xsw_color.standard_text);

                    /* Recreate scanner location type label. */
                    ScannerUpdateLabels(net_parms.player_obj_num);

                    /* Change background music. */
                    if(option.music)
                        XSWDoChangeBackgroundMusic();
	        }
	    }
	}


        /* *********************************************************** */

        /* Skip static and other non moving objects. */     
        if((obj_ptr->type == XSW_OBJ_TYPE_STATIC) ||
           (obj_ptr->type == XSW_OBJ_TYPE_HOME) ||
           (obj_ptr->type == XSW_OBJ_TYPE_AREA)
        )
            return;


	/* Move object, allow sector changes. */
	REngMoveObject(obj_ptr, 1);


	return;
}


/*
 *      Updates the object's visiblity.
 */
void REngVisibility(xsw_object_struct *player_obj_ptr, xsw_object_struct *obj_ptr)
{
	double v;


        /* Are shields up? */
        if(obj_ptr->shield_state == SHIELD_STATE_UP)
	{
	    if(obj_ptr->cur_visibility < 1)
            {
                obj_ptr->cur_visibility = MIN(
                    obj_ptr->cur_visibility + (0.01 * time_compensation),
                    1
                );

                if(obj_ptr == player_obj_ptr)
                    BridgeWinDrawPanel(
                        net_parms.player_obj_num,
                        BPANEL_DETAIL_PVIS
                    );
	    }
        }
	/* Is cloak up? */
	else if(obj_ptr->cloak_state == CLOAK_STATE_UP)
	{
	    if(obj_ptr->power_max > 0)
                v = obj_ptr->visibility - (obj_ptr->cloak_strength *
                    obj_ptr->power_purity * obj_ptr->power /
                    obj_ptr->power_max);
	    else
                v = obj_ptr->visibility;

            if(obj_ptr->cur_visibility > v)   
            {
                obj_ptr->cur_visibility = MAX(
                    obj_ptr->cur_visibility - (0.01 * time_compensation),
                    v
                );
                if(obj_ptr == player_obj_ptr)
                    BridgeWinDrawPanel(   
                        net_parms.player_obj_num,
                        BPANEL_DETAIL_PVIS
                    );
            }
	}
	/* Adjust visibility to normal. */
	else
	{
	    if(obj_ptr->cur_visibility > obj_ptr->visibility)
	    {
                obj_ptr->cur_visibility = MAX(
                    obj_ptr->cur_visibility - (0.01 * time_compensation),
                    obj_ptr->visibility
                );

                if(obj_ptr == player_obj_ptr)
                    BridgeWinDrawPanel(
                        net_parms.player_obj_num,
                        BPANEL_DETAIL_PVIS
                    );
	    }
	    else if(obj_ptr->cur_visibility < obj_ptr->visibility)
	    {
                obj_ptr->cur_visibility = MIN(
                    obj_ptr->cur_visibility + (0.01 * time_compensation),
                    obj_ptr->visibility
                );

                if(obj_ptr == player_obj_ptr)
                    BridgeWinDrawPanel(
                        net_parms.player_obj_num,
                        BPANEL_DETAIL_PVIS
                    );
	    }
	}

        /* Sanitize. */
        if(obj_ptr->cur_visibility < 0)
            obj_ptr->cur_visibility = 0;
        else if(obj_ptr->cur_visibility > 1)
             obj_ptr->cur_visibility = 1;


        return;
}


/*
 *	Updates the object's shield visibility.
 */
void REngShieldVisibility(xsw_object_struct *obj_ptr)
{
	double x;


	/* Calculate deduction. */
	x = (double)((double)CYCLE_LAPSE_MS /
            (double)SHIELD_VISIBILITY_INTERVAL) * time_compensation;

	/* Reduce shield visibility. */
	obj_ptr->shield_visibility -= x;

	/* Sanitize shield_visibility. */
	if(obj_ptr->shield_visibility < 0)
	    obj_ptr->shield_visibility = 0;


	return;
}


/*
 *	Increments animation frames on object.
 */
void REngAnimation(xsw_object_struct *obj_ptr)
{
	/* Handle infinate and finate repeating animations differently. */
	if(obj_ptr->animation.cycle_times <= 0)
	{
	    /* Repeats infinatly. */

            /* Time to increment frame? */
            if((obj_ptr->animation.last_interval +
               obj_ptr->animation.interval) <= cur_millitime
            )
            {
                obj_ptr->animation.current_frame++;

                /* Looped? */
                if(obj_ptr->animation.current_frame >=
                   obj_ptr->animation.total_frames
                )
                    obj_ptr->animation.current_frame = 0;

		obj_ptr->animation.last_interval = cur_millitime;
	    }
	}
	else
	{
	    /* Finate repeats. */

	    /* Still has repeats to go? */
	    if(obj_ptr->animation.cycle_count <
	       obj_ptr->animation.cycle_times
	    )
	    {
                /* Time to increment frame? */
                if((obj_ptr->animation.last_interval +
                   obj_ptr->animation.interval) <= cur_millitime
                )
                {
                    obj_ptr->animation.current_frame++;

		    /* Looped? */
                    if(obj_ptr->animation.current_frame >=
                       obj_ptr->animation.total_frames
                    )
                    {
                        obj_ptr->animation.current_frame = 0;
                        obj_ptr->animation.cycle_count++;

			/* Cycled to end? */
			if(obj_ptr->animation.cycle_count >=
                           obj_ptr->animation.cycle_times
                        )
			{
                            /* Set current frame to the last frame. */
                            obj_ptr->animation.current_frame =
                                obj_ptr->animation.total_frames - 1;
			    if(obj_ptr->animation.current_frame < 0)
				obj_ptr->animation.current_frame = 0;

                            /* Frame will no longer be incremented. */
			}
                    }

                    obj_ptr->animation.last_interval = cur_millitime;

		}	/* Time to increment frame? */
	    }
	}


	return;
}



/*
 *	Manages all reality engine responsibilities.
 */
void REngUpdateObject(
	xsw_object_struct *player_obj_ptr,
	xsw_object_struct *obj_ptr
)
{
	/* object_num is assumed valid and non-garbage. */


        REngVisibility(player_obj_ptr, obj_ptr);
	REngShieldVisibility(obj_ptr);

	REngHeading(player_obj_ptr, obj_ptr);
	REngThrottle(player_obj_ptr, obj_ptr);
	REngThrust(player_obj_ptr, obj_ptr);

	REngAction(player_obj_ptr, obj_ptr);

	REngLocation(player_obj_ptr, obj_ptr);
	REngAnimation(obj_ptr);


	return;
}



/*
 *	Initializes reality engine.
 */
int REngInit()
{
	/* No initialization needed. */

	return(0);
}


/*
 *      Manages all reality engine responsibilities.
 */
void REngManage()
{
	int i;
	xsw_object_struct *obj_ptr;


        /* Check if local updating is enabled. */
        if(option.local_updates)
        {
            /*   Update objects in range.  Need to access each pointer
	     *   array at a time, inrange objects list may be realloc'ed.
	     */
            for(i = 0; i < total_inrange_objects; i++)
	    {
		obj_ptr = inrange_xsw_object[i];

		if(obj_ptr == NULL)
		    continue;
		if(obj_ptr->type <= XSW_OBJ_TYPE_GARBAGE)
		    continue;

                REngUpdateObject(
                    net_parms.player_obj_ptr,
                    obj_ptr
                );
	    }
        }
        else if(net_parms.player_obj_ptr != NULL)
        {
	    /* Update just the player object. */
	    obj_ptr = net_parms.player_obj_ptr;

            REngUpdateObject(
		obj_ptr,
		obj_ptr
	    );
        }


	return;
}


/*
 *	Shuts down reality engine.
 */
void REngShutdown()
{
	/* No shutdown procedure required. */

	return;
}
