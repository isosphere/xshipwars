/*
                    Reality Engine for XSW Objects

 	Functions:

        int REngFreqCmp(double f1, double f2)
	void REngMoveObject(int object_num, char allow_sect_change)

 	void REngHeading(int object_num)
 	void REngThrottle(int object_num)
 	void REngThrust(int object_num)
 	void REngLocation(int object_num)
	void REngVisibility(int object_num)
 	void REngShieldVisibility(int object_num)
 	int REngMortality(int object_num)
 	void REngAnimated(int object_num)
 	void REngWeaponsLock(int object_num)
	void REngTractorLock(int object_num)
 	void REngIntercept(int object_num)
 	void REngPowerCore(int object_num)
 	void REngDamageControl(int object_num)
 	int REngDoHit(int wobj, int tobj)
 	int REngCollisionCheck(int object_num)
	int REngAI(int object_num)

 	int REngObject(int object_num)

	int REngInit()
	void REngManage()
	void REngShutdown()

	---

	Does all reality affects with the XSW objects in the
	universe. Things like hit contacts, movement, mortality,
	ai, etc.

 */

#include "../include/unvmatch.h"
#include "../include/unvmath.h"
#include "../include/unvutil.h"

#include "swserv.h"
#include "net.h"

#include "../include/swexporteventscodes.h"


#define MIN(a,b)        ((a) < (b) ? (a) : (b))
#define MAX(a,b)        ((a) > (b) ? (a) : (b)) 


#define REngEventNone		0
#define REngEventMemChange	3


int REngFreqCmp(double f1, double f2);


/*
 *	Checks if the frequencies are the same up to 2 decimal
 *	accuracy.
 */
int REngFreqCmp(double f1, double f2)
{
	double df;


	/* Sanitize f1 and f2. */
	if(f1 < SWR_FREQ_MIN)
	    f1 = SWR_FREQ_MIN;
        if(f1 > SWR_FREQ_MAX)
            f1 = SWR_FREQ_MAX;

        if(f2 < SWR_FREQ_MIN)
            f2 = SWR_FREQ_MIN;
        if(f2 > SWR_FREQ_MAX)
            f2 = SWR_FREQ_MAX;

	/* Get absolute value difference. */
	df = f2 - f1;
	if(df < 0)
	    df *= -1;

	/* Compare up to 2 decimal places. */
	if(df < 0.01)
	    return(1);
	else
	    return(0);
}


/*
 *      Moves the object by calculating one quantom shift
 *      of its 3D coordinates.
 * 
 *      If allow_sect_change is true then if the object moves out
 *      if the coordinate bounds, it will change sectors and warp to
 *      the other side.
 */
void REngMoveObject(int object_num, char allow_sect_change)
{
	int i;
	char ch_sect_px, ch_sect_nx, ch_sect_py, ch_sect_ny;

	double thrust_dir, thrust_mag, thrust_magc;
	double velocity_mag, velocity_magc;
	double x_vel, y_vel, z_vel;      /* Velocity delta. */
	double x_velc, y_velc, z_velc;   /* Velocity delta /w time compensation. */
	double x_thr, y_thr, z_thr;      /* Thrust. */
	double x_thrc, y_thrc, z_thrc;   /* Thrust delta /w time compensation. */

	xsw_object_struct *obj_ptr;
	int owner_object;
	int trac_object;
	xsw_object_struct *owner_obj_ptr;
	xsw_object_struct *trac_obj_ptr;


        /* Get pointer to object (assumed valid). */
        obj_ptr = xsw_object[object_num];


	/* ************************************************************ */
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

            if(obj_ptr->x < sector_legend.x_min)
            {
                obj_ptr->x += sector_legend.x_len;
                obj_ptr->sect_x -= 1;
                ch_sect_nx = 1;         /* Negative sect x change. */
	        NetSendObjectSect(-1, object_num);
            }
            else if(obj_ptr->x > sector_legend.x_max)
            {
                obj_ptr->x -= sector_legend.x_len;
                obj_ptr->sect_x += 1;
                ch_sect_px = 1;         /* Positive sect x change. */
                NetSendObjectSect(-1, object_num);
            }

            if(obj_ptr->y < sector_legend.y_min)
            {
                obj_ptr->y += sector_legend.y_len;
                obj_ptr->sect_y -= 1;
                ch_sect_ny = 1;         /* Negative sect y change. */
                NetSendObjectSect(-1, object_num);
            }
            else if(obj_ptr->y > sector_legend.y_max)
            {
                obj_ptr->y -= sector_legend.y_len;
                obj_ptr->sect_y += 1;
                ch_sect_py = 1;         /* Positive sect y change. */
                NetSendObjectSect(-1, object_num);
            }

/*
            if(obj_ptr->z < sector_legend.z_min)
            {
                obj_ptr->z += sector_legend.z_len;
                obj_ptr->sect_z -= 1;
                ch_sect_nz = 1;
                NetSendObjectSect(-1, object_num);
            }
            else if(obj_ptr->z > sector_legend.z_max)
            {
                obj_ptr->z -= sector_legend.z_len;
                obj_ptr->sect_z += 1;
                ch_sect_pz = 1;
                NetSendObjectSect(-1, object_num);
            }
*/

            /* Change sector for tractored objects. */ 
            if(ch_sect_nx || ch_sect_px ||
               ch_sect_ny || ch_sect_py
/*
               ch_sect_nz || ch_sect_pz
*/
            )   
            {
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

		    /*   Move tractored object coordinates to exactly
		     *   where the source object is.
		     */
                    if(ch_sect_nx)
                        trac_obj_ptr->x = obj_ptr->x;
                    if(ch_sect_px)
                        trac_obj_ptr->x = obj_ptr->x;

                    if(ch_sect_ny)
                        trac_obj_ptr->y = obj_ptr->y;
                    if(ch_sect_py)
                        trac_obj_ptr->y = obj_ptr->y;


/*
                    if(ch_sect_nx)
                        trac_obj_ptr->x += sector_legend.x_len;
                    if(ch_sect_px)
                        trac_obj_ptr->x -= sector_legend.x_len;

                    if(ch_sect_ny)
                        trac_obj_ptr->y += sector_legend.y_len;
                    if(ch_sect_py)
                        trac_obj_ptr->y -= sector_legend.y_len;

                    if(ch_sect_nz)
                        trac_obj_ptr->z += sector_legend.z_len;
                    if(ch_sect_pz)
                        trac_obj_ptr->z -= sector_legend.z_len;
*/
		    NetSendObjectSect(-1, trac_object);
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
 *      Updates the heding of the object.
 */
void REngHeading(int object_num)
{
	double rel_heading, target_bearing;
	int tobj_num;
	xsw_object_struct *obj_ptr, *tar_obj_ptr;


	/* Get object pointer. */
	obj_ptr = xsw_object[object_num];


	/* Skip static objects and worm hole objects. */
        if((obj_ptr->type == XSW_OBJ_TYPE_STATIC) ||
           (obj_ptr->type == XSW_OBJ_TYPE_WORMHOLE)
	)
           return;


	/*   Skip player objects that are not intercepting anything.
         *   When not intercepting, their heading is updated by the
	 *   client.
	 */
	if((obj_ptr->type == XSW_OBJ_TYPE_PLAYER) &&
	   (obj_ptr->intercepting_object < 0)
	)
           return;


	/* Is object intercepting something? */
	tobj_num = obj_ptr->intercepting_object;
	if(!DBIsObjectGarbage(tobj_num))
	{
	    /* Get rel_heading to the object we're intercepting. */
	    tar_obj_ptr = xsw_object[tobj_num];

	    if(Mu3DInSameSectorPtr(obj_ptr, tar_obj_ptr))
	    {
		/* In same sector. */
		double src_mx, src_my, tar_mx, tar_my;
		double cur_dist, dt, theta1, theta2;


		/* Calculate current distance between the two objects. */
		cur_dist = Mu3DDistance(
		    tar_obj_ptr->x - obj_ptr->x,
		    tar_obj_ptr->y - obj_ptr->y,
                    tar_obj_ptr->z - obj_ptr->z
		);

		/* Calculate delta time (in cycles) for the source
		 * object to reach the target object. A bunch
		 * of errors may be introduced if this is the
		 * start of an intercept, but that shouldn't
		 * be too much as the intercept path neatens up.
		 */
		if(obj_ptr->velocity > 0)
		    dt = cur_dist / obj_ptr->velocity;
		else
		    dt = 0;

		/* Calculate source momentum applied position. */
/*
		src_mx = obj_ptr->x +
		    (obj_ptr->momentum_vector_compoent.i *
		    obj_ptr->velocity * time_compensation);
		src_my = obj_ptr->y +  
                    (obj_ptr->momentum_vector_compoent.j * 
                    obj_ptr->velocity * time_compensation);
 */
		/* Get current position of source object. */
		src_mx = obj_ptr->x;
		src_my = obj_ptr->y;

		/* Get calculated position of target object using
		 * the target object's current momentum and multiply
		 * that by the time (in cycles) it will take for
		 * the source object to reach the target object
		 * at both object's current positions and velocities.
		 */
		tar_mx = tar_obj_ptr->x +
                    (tar_obj_ptr->momentum_vector_compoent.i *
                    tar_obj_ptr->velocity * dt);
                tar_my = tar_obj_ptr->y +
                    (tar_obj_ptr->momentum_vector_compoent.j *
                    tar_obj_ptr->velocity * dt);

		/* Calculate bearing from source object's current
		 * position to target object's `predicted' position.
		 */
                theta1 = MuCoordinateDeltaVector(
                    tar_mx - src_mx,
		    tar_my - src_my
                );

		/* Compensate for source object's current momentum
		 * direction. Another spot where errors in calculations
		 * may come in.
		 */
		target_bearing = SANITIZERADIANS(
		    (2 * theta1) - obj_ptr->velocity_heading
		);

		/* If target_bearing points in the wrong direction
		 * then turn it around.
		 */
		theta2 = SANITIZERADIANS(theta1 - target_bearing);
		if(theta2 >= (PI / 2))
		    target_bearing = theta1;
	    }
	    else
	    {
		/* In different sectors. */

	        target_bearing = MuCoordinateDeltaVector(
		    tar_obj_ptr->sect_x - obj_ptr->sect_x,
		    tar_obj_ptr->sect_y - obj_ptr->sect_y
	        );
	    }

	    rel_heading = SANITIZERADIANS(target_bearing - obj_ptr->heading);

	    if((rel_heading < -3.1415927) || (rel_heading > 3.1415927))
	    {
	        /* rel_heading must be within the turnrate of object_num. */
	        if(rel_heading > (100 * obj_ptr->turnrate * time_compensation))
		    rel_heading = 100 * obj_ptr->turnrate * time_compensation;
	        else if(rel_heading < (-100 * obj_ptr->turnrate * time_compensation))
		    rel_heading = -100 * obj_ptr->turnrate * time_compensation;

		obj_ptr->heading -= rel_heading;
	    }
	    else
	    {
                /* rel_heading must be within the turnrate of object_num. */
                if(rel_heading > (100 * obj_ptr->turnrate * time_compensation))
                    rel_heading = 100 * obj_ptr->turnrate * time_compensation;
                else if(rel_heading < (-100 * obj_ptr->turnrate * time_compensation))
                    rel_heading = -100 * obj_ptr->turnrate * time_compensation;

                obj_ptr->heading += rel_heading;
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
 *	Nothing is done here, this function is just for client side.
 */
void REngThrottle(int object_num)
{
        xsw_object_struct *obj_ptr;


        /* Get object pointer. */
        obj_ptr = xsw_object[object_num];


        /* Skip static objects. */
        if(obj_ptr->type == XSW_OBJ_TYPE_STATIC)
           return;


        /* Skip player objects, their throttle is updated on client. */
        if(obj_ptr->type == XSW_OBJ_TYPE_PLAYER)
           return;


	return;
}

/*
 *      Thrust, antimatter consumption, drag, and other calculations
 *      of velocity are performed here.
 */
void REngThrust(int object_num)
{
	double throttle;
	double thrust_power;
        xsw_object_struct *obj_ptr;


        /* Get object pointer. */
        obj_ptr = xsw_object[object_num];

	/* We want to update all types of objects, since there is
	 * no telling if an object should have power decrementation,
	 * A civilization on a planet for instance, would use up
	 * antimatter, so throttle applied would simulate antimatter
	 * usage.
	 */

	/* Get throttle and thrust_power. */
        throttle = obj_ptr->throttle;
	if(throttle > 0)
	{
	    /* Get thrust power. */
            thrust_power = obj_ptr->thrust_power;

            /* Consume Antimatter. */
            obj_ptr->antimatter -= (throttle * thrust_power *
		time_compensation);
            if(obj_ptr->antimatter < 0)
                obj_ptr->antimatter = 0;

            /* Thrust output should be 0 if there is no antimatter left. */
            if(obj_ptr->antimatter <= 0)
                thrust_power = 0;
	}
	else
	{
	    thrust_power = 0;
	    throttle = 0;
	}


	/* Actual thrust output.
	 *
	 * Thrust is calculated by taking the coefficent of the
	 * throttle and multiplying that with the thrust_power.
	 */
	obj_ptr->thrust = MAX(
	    throttle * thrust_power * time_compensation,
	    0
	);

        /* Velocity drag. */   
        if(obj_ptr->velocity_max > 0)
            obj_ptr->velocity -= ((obj_ptr->velocity /
                obj_ptr->velocity_max) * obj_ptr->thrust_power *
                time_compensation
	    );
        else 
            obj_ptr->velocity = 0;


	/* If velocity is such a small number, then set it 0. */
	if(obj_ptr->velocity < 0.00003)
	    obj_ptr->velocity = 0;


	return;
}



/*
 *	Performs an update location procedure on the object.
 *	movement style depends on the type of the object and several
 *	other attributes.
 *
 *	REngMoveObject() will be called to do the actual moving.
 */
void REngLocation(int object_num)
{
	xsw_object_struct *obj_ptr;
        double distance;
        int i;
        xsw_object_struct **tar_obj_ptr;
        xsw_object_struct *nebula_obj_ptr = NULL;


        /* Get object pointer (assumed valid). */
        obj_ptr = xsw_object[object_num];


	/* If this object is not an area object, then check if
	 * this object is in an area object.
	 */
	if(obj_ptr->type != XSW_OBJ_TYPE_AREA)
	{
	    /* Go through XSW objects list. */
	    for(i = 0, tar_obj_ptr = xsw_object;
                i < total_objects;
                i++, tar_obj_ptr++
            )
            {
                if(*tar_obj_ptr == NULL)
                    continue;

                /* Check only objects of type XSW_OBJ_TYPE_AREA. */
                if((*tar_obj_ptr)->type != XSW_OBJ_TYPE_AREA)
                    continue;

                /* Skip same object. */
                if(*tar_obj_ptr == obj_ptr)
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

	    /* Is this object now in a nebula? */
	    if(nebula_obj_ptr != NULL)
            {
	        /* Object is now in a nebula. */

	        if(obj_ptr->loc_type != XSW_LOC_TYPE_NEBULA)
	        {
		    /* Previously not in nebula, so object just entered. */


	        }

	        obj_ptr->loc_type = XSW_LOC_TYPE_NEBULA;
	    }
	    else
	    {
	        /* Object is in normal space. */

		obj_ptr->loc_type = XSW_LOC_TYPE_SPACE;
	    }
	}



	/* Skip types of object that are not to be moved under
	 * normal circumstances.
	 */
        if((obj_ptr->type == XSW_OBJ_TYPE_STATIC) ||
	   (obj_ptr->type == XSW_OBJ_TYPE_PLAYER) ||
           (obj_ptr->type == XSW_OBJ_TYPE_HOME) ||
           (obj_ptr->type == XSW_OBJ_TYPE_AREA) ||
           (obj_ptr->type == XSW_OBJ_TYPE_ANIMATED) ||
           (obj_ptr->type == XSW_OBJ_TYPE_WORMHOLE) ||
           (obj_ptr->type == XSW_OBJ_TYPE_ELINK)
	)
            return;

	/* Move object. */
	REngMoveObject(object_num, 1);


	return;
}


/*
 *	Updates the object's current visiblity by checking its
 *	shield, cloak and location.
 */
void REngVisibility(int object_num)
{
	double v;
        xsw_object_struct *obj_ptr;


        /* Get object pointer, assumed valid. */
        obj_ptr = xsw_object[object_num];


        /* Are shields up? */
        if(obj_ptr->shield_state == SHIELD_STATE_UP)
        {
            if(obj_ptr->cur_visibility < 1)
            {
                obj_ptr->cur_visibility = MIN(
                    obj_ptr->cur_visibility + (0.01 * time_compensation),
                    1
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
            }
            else if(obj_ptr->cur_visibility < obj_ptr->visibility)
            {   
                obj_ptr->cur_visibility = MIN(
                    obj_ptr->cur_visibility + (0.01 * time_compensation),
                    obj_ptr->visibility
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
 *      Updates the object's shield visibility.
 */
void REngShieldVisibility(int object_num)
{
	double x;
	xsw_object_struct *obj_ptr;
        

	/* Get object pointer, assumed valid. */
	obj_ptr = xsw_object[object_num];


	/* Skip if shields not visable. */
	if(obj_ptr->shield_visibility <= 0)
	    return;


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
 *	Checks if the object is `too old' and handles the mortality
 *	accordingly.
 */
int REngMortality(int object_num)
{
	int object_must_die = 0;
        xsw_object_struct *obj_ptr;
	xsw_animation_struct *anim;


	/* Get object pointer (assumed valid). */
	obj_ptr = xsw_object[object_num];


        /* Handle mortality by object type. */
	switch(obj_ptr->type)
	{
	  case XSW_OBJ_TYPE_ANIMATED:
	    /* Animated objects `die' when they have a limited (non zero)
	     * `cycle times' and that the number of times they have
	     * cycled exceeds that.
	     */
	    anim = &obj_ptr->animation;
	    if(anim->cycle_times > 0)
	    {
                if(anim->cycle_count >= anim->cycle_times)
		    object_must_die = 1;
	    }
	    break;

	  case XSW_OBJ_TYPE_PLAYER:
	    /* Player objects do not die regardless. */
	    break;

	  default:
	    /* All other object types, handle by life span. If the object
	     * has a defined (non-zero and positive) life span then their
	     * birth time is checked with current time to see if they must die.
	     */
	    if((obj_ptr->lifespan > 0) &&
               (obj_ptr->birth_time > 0)
	    )
	    {
		time_t dt;

		/* Calculate age. */
		dt = cur_millitime - obj_ptr->birth_time;

		/* Age positive and exceeded life span? */
		if((dt < 0) ||
                   (dt > obj_ptr->lifespan)
		)
		    object_must_die = 1;
            }
	    break;
	}


	/* Has this object died and we must recycle it? */
	if(object_must_die)
	{
            /* Recycle this object. */
            DBRecycleObject(object_num);

            /* Notify all connections about this recycle. */
	    NetSendRecycleObject(-1, object_num);

	    return(REngEventMemChange);
	}
	else
	{
	    return(REngEventNone);
	}
}



/*
 *	Updates animated object's animation frame position.
 */
void REngAnimated(int object_num)
{
	time_t dt;
	xsw_object_struct *obj_ptr;
	xsw_animation_struct *anim; 


        /* Get object pointer (assumed valid). */
        obj_ptr = xsw_object[object_num];

	anim = &obj_ptr->animation;

	/* Time to increment frame? */
	dt = anim->last_interval + anim->interval;
        if(dt <= cur_millitime)
	{
	    anim->current_frame++;

	    /* Looped? */
	    if(anim->current_frame >= anim->total_frames)
	    {
		anim->current_frame = 0;

		/* Increment cycled times only if there is a cycle limit. */
		if(anim->cycle_times > 0)
		    anim->cycle_count++;
	    }

	    anim->last_interval = cur_millitime;
	}


	return;
}



/*
 *	Updates weapons lock on an object.
 */
void REngWeaponsLock(int object_num)
{
	int locked_object;
	xsw_object_struct *obj_ptr;


	/* object_num is assumed valid. */
	obj_ptr = xsw_object[object_num];


        /* Get locked_object. */
        locked_object = obj_ptr->locked_on;
	if(DBIsObjectGarbage(locked_object))
	{
	    /* Locked on object is now garbage, unlock. */
	    obj_ptr->locked_on = -1;

	    return;
	}


	/* Check distance. */
/* Need to take location of object into account? */
	if(Mu3DInRange(
	    xsw_object, total_objects,
	    object_num, locked_object,
            obj_ptr->scanner_range *
	    DBGetObjectVisibility(locked_object)
	))
	{
	    /* Object still in range. */

	}
	else
	{
	    /* Object out of scanner range, unlock. */
	    obj_ptr->locked_on = -1;
	}

	return;
}


/*
 *	Updates an object's tractor beam lock on another object.
 */
void REngTractorLock(int object_num)
{
	int i;
	double delta_x, delta_y, delta_z;
	double theta1, theta2, theta3;
	double mag1, mag2, mag3;
	double distance;

	double old_velocity_max;

        int tractored_object;
        xsw_object_struct *obj_ptr, *tar_obj_ptr;  


        /* object_num is assumed valid. */
        obj_ptr = xsw_object[object_num];


	/* Skip if not tractoring anything. */
	if(obj_ptr->total_tractored_objects <= 0)
	    return;


        /* Unlock tractor beam if object is out of antimatter. */
        if(obj_ptr->antimatter <= 0)
        {
	    for(i = 0; i < obj_ptr->total_tractored_objects; i++)
	    {
		DBObjectUntractor(object_num, obj_ptr->tractored_object[i]);
		NetSendTractorBeamLock(-1, object_num, -1);
	    }
            return;
        }


	/* Move each tractored object. */
	for(i = 0; i < obj_ptr->total_tractored_objects; i++)
	{
	    tractored_object = obj_ptr->tractored_object[i];
	    if(tractored_object < 0)
		continue;

	    /* Check if tractored_object is not -1 but garbage. */
	    if(DBIsObjectGarbage(tractored_object))
	    {
		DBObjectUntractor(object_num, tractored_object);
		NetSendTractorBeamLock(-1, object_num, -1);
	        continue;
	    }
	    else
	    {
		tar_obj_ptr = xsw_object[tractored_object];
	    }

	    /* Check if target object is tractorable. */
	    if(!DBIsObjectTractorablePtr(obj_ptr, tar_obj_ptr))
	    {
		DBObjectUntractor(object_num, tractored_object);
	        NetSendTractorBeamLock(-1, object_num, -1);
                continue;
	    }

            /* Consume antimatter. */

/* For now consume a constant amount of antimatter based on object's size. */
obj_ptr->antimatter -= (
    (double)((double)tar_obj_ptr->size / 1000) * 0.05
    ) * time_compensation;

	    if(obj_ptr->antimatter < 0)
	        obj_ptr->antimatter = 0;


	    /* ********************************************************** */
	    /* Adjust object vector and move these types of objects. */
	    if(1
/*
	       (tar_obj_ptr->type == XSW_OBJ_TYPE_STATIC) ||
               (tar_obj_ptr->type == XSW_OBJ_TYPE_PLAYER) ||
	       (tar_obj_ptr->type == XSW_OBJ_TYPE_HOME) ||
               (tar_obj_ptr->type == XSW_OBJ_TYPE_AREA) ||
               (tar_obj_ptr->type == XSW_OBJ_TYPE_ANIMATED)
*/
	    )
	    {
		/* Get target object to source object delta distances. */
	        delta_x = obj_ptr->x - tar_obj_ptr->x;
	        delta_y = obj_ptr->y - tar_obj_ptr->y;
	        delta_z = obj_ptr->z - tar_obj_ptr->z;

	        distance = Mu3DDistance(delta_x, delta_y, delta_z);

		/*   Get target object current velocity and velocity
		 *   heading.
		 */
	        theta1 = tar_obj_ptr->velocity_heading;
	        mag1 = tar_obj_ptr->velocity;

		/* Target object to source object bearing. */
	        theta2 = MuCoordinateDeltaVector(delta_x, delta_y);

		/*   Calculate tractor beam strength by taking
		 *   the coefficient of the distance away the objects
		 *   are from each other.  The farther away they
		 *   are, the greater the tractor beam strength.
                 */
	        if(distance <= (double)((double)obj_ptr->size / 1000))
		    mag2 = 0;
	        else
	            mag2 = 0.009 * (double)distance /
			(double)MAX_TRACTOR_BEAM_LEN;

		/*   Add target object's current velocity vector with
		 *   the new velocity vector caused by the tractor beam
		 *   pull.
		 */
	        MuVectorAdd(
	            theta1, mag1,
	            theta2, mag2,
	            &theta3, &mag3
	        );

	        tar_obj_ptr->velocity_heading = theta3;
                MuSetUnitVector2D(
                    &tar_obj_ptr->momentum_vector_compoent,
                    tar_obj_ptr->velocity_heading
                );

	        tar_obj_ptr->velocity = mag3;


	        /*   IMPORTANT:
		 *
	         *   We must temporarly increase the tractored object's 
	         *   maximum velocity before we move it and reset it
	         *   after moving it.
	         */
	        old_velocity_max = tar_obj_ptr->velocity_max;
	        if(tar_obj_ptr->velocity > tar_obj_ptr->velocity_max)
		    tar_obj_ptr->velocity_max = tar_obj_ptr->velocity;

	        /* Move the object. */
	        REngMoveObject(tractored_object, 0);

	        /* Restore original velocity_max. */
	        tar_obj_ptr->velocity_max = old_velocity_max;
	    }
	}



	return;
}


/*
 *	Only checks if object is in intercept range, does NOT
 *	adjust heading.
 */
void REngIntercept(int object_num)
{
        int intercepting_object;
        xsw_object_struct *obj_ptr;


        /* Get pointer to object (assumed valid). */
        obj_ptr = xsw_object[object_num];

	/* Skip worm hole objects. */
	if(obj_ptr->type == XSW_OBJ_TYPE_WORMHOLE)
	    return;

	/* Get intercepting_object. */
	intercepting_object = obj_ptr->intercepting_object;
	if(DBIsObjectGarbage(intercepting_object))
	    return;

	/* Use MatchInterceptByNumber() to see if object is in range. */
	if(MatchInterceptByNumber(
	    xsw_object, total_objects,
	    object_num, intercepting_object
	) < 0)
	    obj_ptr->intercepting_object = -1;

	return;
}



/*
 *	Regernerates an object's power core.
 */
void REngPowerCore(int object_num)
{
	xsw_object_struct *obj_ptr;


        /* Get pointer to object (assumed valid). */
        obj_ptr = xsw_object[object_num];

	/* Increase power in the object's power core. */
        if(obj_ptr->power < obj_ptr->power_max)
        {
            obj_ptr->power += (obj_ptr->core_efficency *
		obj_ptr->power_purity * time_compensation
	    );

            if(obj_ptr->power > obj_ptr->power_max)
                obj_ptr->power = obj_ptr->power_max;
        }

	return;
}



/*
 *	Repairs object if the object has damage control on.
 */
void REngDamageControl(int object_num)
{
	double power, power_purity;
	double hp_inc;
	xsw_object_struct *obj_ptr;


        /* Get pointer to object (assumed valid). */
        obj_ptr = xsw_object[object_num];


        /* Skip if damage control is not on. */
        if(obj_ptr->damage_control != DMGCTL_STATE_ON)
            return;

	/* Skip if hit points are already at max. */
	if(obj_ptr->hp >= obj_ptr->hp_max)
            return;


	/* Fetch and sanitize power. */
        power = MAX(obj_ptr->power, 0);
	power_purity = MIN(obj_ptr->power_purity, 1);
	if(power_purity < 0)
	    power_purity = 0;

	/* Calculate hit points increment. */
	hp_inc = MAX(
	    sysparm.dmg_ctl_rate * power * power_purity * time_compensation,
	    0
	);

	/* `Repair' hit points. */
	obj_ptr->hp += hp_inc;
	if(obj_ptr->hp > obj_ptr->hp_max)
	{
	    hp_inc -= (obj_ptr->hp - obj_ptr->hp_max);
	    if(hp_inc < 0)
		hp_inc = 0;

	    obj_ptr->hp = obj_ptr->hp_max;
	}

	/* Consume antimatter based on how much repairs were made. */
/* Need better calculation. */
	obj_ptr->antimatter -= hp_inc;
	if(obj_ptr->antimatter < 0)
	    obj_ptr->antimatter = 0;


	return;
}



/*
 *	Performs a hit object procedure.
 *
 *	WARNING: This function makes no checks if wobj and tobj really
 *	did hit each other! It is up to REngCollisionCheck() (which calls
 *	this function) to do all the contact checking.
 */
int REngDoHit(int wobj, int tobj)
{
	int i;
	int report_this = 1;	/* Log and export event. */
	int can_count_kill = 1;	/* Can count as kill. */
	double dx, theta;
	char text[(3 * XSW_OBJ_NAME_MAX) + 512];
	char tobj_name[XSW_OBJ_NAME_MAX + 80];
	char wobj_name[XSW_OBJ_NAME_MAX + 80];
	char wobj_owner_name[XSW_OBJ_NAME_MAX + 80];
	int wobj_owner, new_object_num;
	connection_struct **ptr, *con_ptr;

	double	damage_to_structure = 0,
		damage_to_shield = 0;


	/* Check if weapon and target objects are valid. */
	if(DBIsObjectGarbage(wobj))
	    return(-1);
	if(DBIsObjectGarbage(tobj))
            return(-1);


	/* Weapon object must have power or else it is considered
	 * to have already hit something and thus should not be
	 * checked for contact again. Most notibly stream and sphere
	 * weapons.
	 */
	if(xsw_object[wobj]->power <= 0)
	    return(0);


	/* If report_destroyed_weapons is set to false, then
	 * check if target object is a weapon object.
	 * If it is, then set report_this to false so that
	 * no reporting of this hit will be made.
	 */
	if(!sysparm.report_destroyed_weapons)
	{
	    if((xsw_object[tobj]->type == XSW_OBJ_TYPE_WEAPON) ||
	       (xsw_object[tobj]->type == XSW_OBJ_TYPE_STREAMWEAPON) ||
               (xsw_object[tobj]->type == XSW_OBJ_TYPE_SPHEREWEAPON)
	    )
	    {
		report_this = 0;
		can_count_kill = 0;
	    }
	}


	/* Get owner of weapon object. */
	wobj_owner = xsw_object[wobj]->owner;


        /* Get formal names of the objects. */
        strncpy(tobj_name, DBGetFormalNameStr(tobj), XSW_OBJ_NAME_MAX + 80);
	tobj_name[XSW_OBJ_NAME_MAX + 80 - 1] = '\0';

	strncpy(wobj_name, DBGetFormalNameStr(wobj), XSW_OBJ_NAME_MAX + 80);
        wobj_name[XSW_OBJ_NAME_MAX + 80 - 1] = '\0';

        strncpy(
	    wobj_owner_name,
	    DBGetFormalNameStr(wobj_owner),
	    XSW_OBJ_NAME_MAX + 80
	);
        wobj_owner_name[XSW_OBJ_NAME_MAX + 80 - 1] = '\0';


	/* Log weapon object hitting target object (as needed). */
	if(report_this)
	{
	    const int loc_str_len = 256;
            char loc_str[loc_str_len];


	    /* Format location string. */
            UNVLocationFormatString(
                loc_str,
                &xsw_object[tobj]->sect_x,
                &xsw_object[tobj]->sect_y,
                &xsw_object[tobj]->sect_z,
                &xsw_object[tobj]->x,
		&xsw_object[tobj]->y,
		&xsw_object[tobj]->z,
		loc_str_len
	    );  

	    /* Log hit. */
            sprintf(
		text,
		"%s hit %s at %s.",
		wobj_name, tobj_name, loc_str
	    );
	    if(sysparm.log_events)
		LogAppendLineFormatted(fname.primary_log, text);


	    /* Export hit event. */
	    switch(xsw_object[wobj]->type)
	    {
	      case XSW_OBJ_TYPE_WEAPON:
	        sprintf(text,
		    "%s hit by %s fired by %s at %s.",
		    tobj_name, wobj_name, wobj_owner_name, loc_str
	        );
	        ExportEvents(
		    fname.events_export,
		    EVST_TYPE_PROJECTILEWEAPON_HIT,
		    text
	        );
	        break;

	      default:	/* Stream or sphere weapon. */
                sprintf(text,
                    "%s hit by %s fired by %s at %s.",
                    tobj_name, wobj_name, wobj_owner_name, loc_str
                );
                ExportEvents(
                    fname.events_export,
                    EVST_TYPE_STREAMWEAPON_HIT,
                    text
                );
                break;
	    }
	}


	/* *********************************************************** */

	/*   Make shields visiable on target object if the target
	 *   object has its shields up and the shield frequencies
	 *   on the target and weapon objects do NOT match.
	 */
	if((xsw_object[tobj]->shield_state == SHIELD_STATE_UP) &&
           !REngFreqCmp(xsw_object[tobj]->shield_frequency,
            xsw_object[wobj]->shield_frequency) &&
           (xsw_object[tobj]->power_max > 0)
	)
	{
	    xsw_object[tobj]->shield_visibility =
	        xsw_object[tobj]->power / xsw_object[tobj]->power_max;

	    /* Send shield visibility to all connections. */
	    NetSendShieldVis(-1, tobj);
	}

	/* Take away shield power and/or hitpoints from tobj. */
	if((sysparm.homes_destroyable) ? 1 :
           ((xsw_object[tobj]->type != XSW_OBJ_TYPE_HOME) &&
            (xsw_object[tobj]->type != XSW_OBJ_TYPE_WORMHOLE) &&
            (xsw_object[tobj]->type != XSW_OBJ_TYPE_ELINK)
           )
	)
	{
	    /*   Target object has shields up and shield frequencies
             *   do not match?
             */
	    if((xsw_object[tobj]->shield_state == SHIELD_STATE_UP) &&
               !REngFreqCmp(xsw_object[tobj]->shield_frequency,
                xsw_object[wobj]->shield_frequency)
	    )
	    {
		/* Extra damage to shields due to lack of power
		 * purity on tobj.
		 */
		dx = MAX(
		    ((xsw_object[wobj]->power
                        * (2 - xsw_object[tobj]->power_purity)) -
		        xsw_object[wobj]->power),
		    0
		);

                /* Add damage to shields, increase damage due to
                 * power purity of target object.
                 */
                damage_to_shield += (xsw_object[wobj]->power + dx);

	        xsw_object[tobj]->power -= damage_to_shield;


		/* Shields went down, apply remaining damage to hull. */
	        if(xsw_object[tobj]->power < 0)
	        {
		    /* Target object power is now negative and represents
		     * damage to its structure.
		     */

                    dx = MAX(
			(xsw_object[tobj]->power * -1) - xsw_object[tobj]->hp,
			0
		    );

                    damage_to_structure = MAX(
			(xsw_object[tobj]->power * -1) - dx,
                        0
		    );

                    xsw_object[tobj]->hp -= damage_to_structure;


		    /* Undo too much damage to shields. */
		    damage_to_shield += xsw_object[tobj]->power;
	        }
	    }
	    else
	    {
		/* Target object's shields were down, no shields at all, or
		 * frequency of weapon matched frequency of target object's
		 * shields.
		 */

		dx = MAX(xsw_object[wobj]->power - xsw_object[tobj]->hp, 0);

		damage_to_structure = MAX(xsw_object[wobj]->power - dx, 0);

	        xsw_object[tobj]->hp -= damage_to_structure;
	    }
	}
	/* Sanitize target object's power (a must). */
	if(xsw_object[tobj]->power < 0)
	    xsw_object[tobj]->power = 0;


	/* Damage to structure and shields have been calculated
	 * and processed, now add to score.
	 */

	/* Damage given by weapon object's owner. */
	if(!DBIsObjectGarbage(wobj_owner))
	{
            if((sysparm.homes_destroyable) ? 1 :
               ((xsw_object[tobj]->type != XSW_OBJ_TYPE_HOME) &&
                (xsw_object[tobj]->type != XSW_OBJ_TYPE_WORMHOLE) &&
                (xsw_object[tobj]->type != XSW_OBJ_TYPE_ELINK)
	       )
            )
	    {
                if(!UNVAllocScores(xsw_object[wobj_owner]))
	        {
		    /* Add damage_given to weapon object owner. */
		    xsw_object[wobj_owner]->score->damage_given += (
		        damage_to_shield + damage_to_structure
		    );

		    /* Add bonus credits to owner. */
		    if(xsw_object[tobj]->type == XSW_OBJ_TYPE_PLAYER)
		    {
		        xsw_object[wobj_owner]->score->credits += (
			    (damage_to_shield + damage_to_structure) *
			    sysparm.hit_player_bonus
		        );
		    }

		    /* Add kills if target object hp's are less than 0. */
		    if((xsw_object[tobj]->hp <= 0) &&
                       can_count_kill
		    )
		    {
		        xsw_object[wobj_owner]->score->kills++;

			/* Transfer credits on tobj to wobj_owner? */
			if(sysparm.killer_gets_credits &&
                           (xsw_object[tobj]->score != NULL)
			)
			{
			    xsw_object[wobj_owner]->score->credits +=
				xsw_object[tobj]->score->credits;

			    xsw_object[tobj]->score->credits = 0;
			}
		    }

	        }
	    }
	}

	/* Damage recieved by target object. */
        if((sysparm.homes_destroyable) ? 1 :
           ((xsw_object[tobj]->type != XSW_OBJ_TYPE_HOME) &&
            (xsw_object[tobj]->type != XSW_OBJ_TYPE_WORMHOLE) &&
            (xsw_object[tobj]->type != XSW_OBJ_TYPE_ELINK)
           )
        )
        {
            if(!UNVAllocScores(xsw_object[tobj]))
            {
                /* Add damage_recieved to target object. */
                xsw_object[tobj]->score->damage_recieved += (
		    damage_to_shield + damage_to_structure
                );
	    }

	    if(report_this)
	    {
		/* Log target object recieving damage. */
	        sprintf(
		    text,
              "%s: Recieved %.2f damage by weapon fired by %s",
	            tobj_name,
	            damage_to_shield + damage_to_structure,
	            wobj_owner_name
	        );
		if(sysparm.log_events)
		    LogAppendLineFormatted(fname.primary_log, text);


	        /* Send hit notify to target object and weapon
		 * object's owner's connections.
		 */

		/* Get bearing from target object to weapon object. */
		theta = MuCoordinateDeltaVector(
		    xsw_object[wobj]->x - xsw_object[tobj]->x,
		    xsw_object[wobj]->y - xsw_object[tobj]->y
		);
		/* Change bearing to be relative to target object's
		 * orientation.
		 */
		theta = SANITIZERADIANS(theta - xsw_object[tobj]->heading);


		/* Send hit notify to all connections in sector. */
		for(i = 0, ptr = connection;
		    i < total_connections;
		    i++, ptr++
		)
		{
		    con_ptr = *ptr;
                    if(con_ptr == NULL)
			continue;
		    if(con_ptr->socket < 0)
			continue;
		    if(con_ptr->object_num < 0)
			continue;

		    /* Is connection object's object needs to be in
		     * the same sector as the target object to recieve
		     * this.
		     */
		    if(!Mu3DInSameSector(
			xsw_object, total_objects, con_ptr->object_num, tobj
		    ))
			continue;

		    NetSendNotifyHit(
			i,
			wobj, tobj,
			damage_to_shield + damage_to_structure, /* Total damage. */
			theta,	/* Relative to target object. */
			damage_to_structure,
			damage_to_shield
		    );
		}
	    }	/* if(report_this) */
	}


        /* Create large explosion if tobj's hp are less than 0. */
	if(xsw_object[tobj]->hp <= 0)
	{
	    /* Big explosion. */
	    new_object_num = DBCreateObjectByOPM(
		"Explosion3",
		NULL,
		XSW_OBJ_TYPE_ANIMATED,
		xsw_object[tobj]->x,
		xsw_object[tobj]->y,
		xsw_object[tobj]->z,
		xsw_object[tobj]->heading,
		xsw_object[tobj]->pitch
	    );
	    if(!DBIsObjectGarbage(new_object_num))
	    {
		xsw_object[new_object_num]->sect_x = xsw_object[tobj]->sect_x;
                xsw_object[new_object_num]->sect_y = xsw_object[tobj]->sect_y;
                xsw_object[new_object_num]->sect_z = xsw_object[tobj]->sect_z;

		NetSendCreateObject(-1, new_object_num);
	    }

	    /* Check if target object is immune from being destroyed. */
	    if(xsw_object[tobj]->permission.uid >
		ACCESS_UID_DESTROY_IMMUNE)
	    {
		/* Target object can be destroyed. */


		if(report_this)
		{
                    const int loc_str_len = 256;
                    char loc_str[loc_str_len];


                    /* Format location string. */
                    UNVLocationFormatString(
                        loc_str,
                        &xsw_object[tobj]->sect_x,
                        &xsw_object[tobj]->sect_y,
                        &xsw_object[tobj]->sect_z,
                        &xsw_object[tobj]->x,
                        &xsw_object[tobj]->y,
                        &xsw_object[tobj]->z,
                        loc_str_len
                    );

		    switch(xsw_object[tobj]->type)
		    {
		      case XSW_OBJ_TYPE_PLAYER:
			/* Log. */
		        sprintf(text,
		  "%s: Destroyed due to hull structure colapse at %s.",
		            tobj_name, loc_str
		        );
	                if(sysparm.log_events)
			    LogAppendLineFormatted(fname.primary_log, text);

			/* Export event. */
			sprintf(text,
 "%s destroyed due to hull structure colapse at %s.",
                            tobj_name, loc_str
                        );
			ExportEvents(
                            fname.events_export,
                            EVST_TYPE_DESTROYEDPLAYER,
                            text
                        );
		        break;

                      case XSW_OBJ_TYPE_WEAPON:
                        /* Log. */
                        sprintf(text,
                            "%s: Destroyed at %s.",
                            tobj_name, loc_str
                        );
                        if(sysparm.log_events)
                            LogAppendLineFormatted(fname.primary_log, text);

                        /* Export event. */
                        sprintf(text,
                            "%s destroyed at %s.",
                            tobj_name, loc_str
			);
                        ExportEvents(
                            fname.events_export,
                            EVST_TYPE_DESTROYEDWEAPON,
                            text
                        );
                        break;

                      default:
                        /* Log. */
                        sprintf(text,
                            "%s: Destroyed at %s.",
                            tobj_name, loc_str
                        );
                        if(sysparm.log_events)
                            LogAppendLineFormatted(fname.primary_log, text);

                        /* Export event. */   
                        sprintf(text, 
                            "%s destroyed at %s.",
                            tobj_name, loc_str
                        );
                        ExportEvents(
                            fname.events_export,
                            EVST_TYPE_DESTROYEDOBJECT,
                            text
                        );
                        break;
		    }

		    /* Go through connections list, send out
		     * destroy notifies (as needed).
		     */
		    for(i = 0, ptr = connection;
                        i < total_connections;
                        i++, ptr++
		    )
                    {
			con_ptr = *ptr;
                        if(con_ptr == NULL)
                            continue;
                        if(con_ptr->socket < 0)
                            continue;
                        if(con_ptr->object_num < 0)
                            continue;

		        /* Notify connection of this object about
		         * destruction (regardless of location).
		         */
		        if(con_ptr->object_num == tobj)
		        {
                            NetSendSysMessage(
                                i,
                                CS_SYSMESG_CODE_WARNING,
                                "Your vessel has been destroyed."
                            );
			}


                        /* Check if in same sector. */
                        if(!Mu3DInSameSector(
			    xsw_object, total_objects,
			    con_ptr->object_num, tobj
			))
                            continue;

		        NetSendNotifyDestroy(
			    i,
			    0,		/* Reason. */
			    tobj,	/* Destroyed object. */
			    wobj,	/* Object that caused destruction. */
			    wobj_owner	/* Owner of wobj. */
			);
		    }
		}	/* if(report_this) */

		/*   Recycle tobj and disconnect connection (if it
		 *   was a player).
		 *   IMPORTANT NOTE: tobj is now invalid!
		 */
                DBRecycleObject(tobj);

		/* Send recycle request to all connections for tobj. */
                NetSendRecycleObject(-1, tobj);
	    }
	    else
	    {
		/* Sanitize hp on tobj (since we didn't recycle it. */
                if(xsw_object[tobj]->hp < 0)
		    xsw_object[tobj]->hp = 0;
	    }
	}
	else
	{
	    /* Create small explosion. */
	    new_object_num = -1;
	    switch(xsw_object[wobj]->type)
	    {
	      /* Stream weapon. */
	      case XSW_OBJ_TYPE_STREAMWEAPON:
		new_object_num = DBCreateObjectByOPM(
		    "Explosion1",
		    NULL,
		    XSW_OBJ_TYPE_ANIMATED,
		    xsw_object[tobj]->x,
		    xsw_object[tobj]->y,
		    xsw_object[tobj]->z,
		    xsw_object[tobj]->heading,
		    xsw_object[tobj]->pitch
		);
		break;

	      /* Sphere weapon. */
              case XSW_OBJ_TYPE_SPHEREWEAPON:  
                new_object_num = DBCreateObjectByOPM(
                    "Explosion1",
                    NULL,
                    XSW_OBJ_TYPE_ANIMATED,
                    xsw_object[tobj]->x,
                    xsw_object[tobj]->y,
                    xsw_object[tobj]->z,
                    xsw_object[tobj]->heading,
                    xsw_object[tobj]->pitch
                );
                break;

	      /* Other type of weapon. */
	      default:
                new_object_num = DBCreateObjectByOPM(
                    "Explosion1",
                    NULL,
                    XSW_OBJ_TYPE_ANIMATED,
                    xsw_object[wobj]->x,
                    xsw_object[wobj]->y,
                    xsw_object[wobj]->z,
                    xsw_object[wobj]->heading,
                    xsw_object[wobj]->pitch
                );
		break;
	    }
            if(DBIsObjectGarbage(new_object_num))
	    {
                sprintf(text,
        "REngDoHit(): Could not create small explosion object."
                );
                if(sysparm.log_errors)
                    LogAppendLineFormatted(fname.primary_log, text);
	    }
	    else
            {
                xsw_object[new_object_num]->sect_x = xsw_object[tobj]->sect_x;
                xsw_object[new_object_num]->sect_y = xsw_object[tobj]->sect_y;
                xsw_object[new_object_num]->sect_z = xsw_object[tobj]->sect_z;

		NetSendCreateObject(-1, new_object_num);                
            }
	}


	/* Recycle the weapon object if its not a stream weapon. */
        if(xsw_object[wobj]->type == XSW_OBJ_TYPE_WEAPON)
	{
            DBRecycleObject(wobj);
            NetSendRecycleObject(-1, wobj);
	}


	return(0);
}



/*
 *	Does collission check, returns number of objects that have
 *	been found to have collided.
 *
 *	If a collision was detected and the object pointers were
 *	reallocated, this function will return REngEventMemChange.
 *	Otherwise if there were no contacts or no reallocation of
 *	object memory, REngEventNone is returned.
 */
int REngCollisionCheck(int object_num)
{
	int tar_obj_num;
	double heading_variance, distance;
	xsw_object_struct *obj_ptr, *tar_obj_ptr, **ptr;


	/* Get obj_ptr (assumed valid). */
	obj_ptr = xsw_object[object_num];


	/* Colission check: Standard weapon. */
        if(obj_ptr->type == XSW_OBJ_TYPE_WEAPON)
        {
	    /* Begin checking if object has come in contact. */

	    /* Go through XSW objects list. */
            for(tar_obj_num = 0, ptr = xsw_object;
                tar_obj_num < total_objects;
                tar_obj_num++, ptr++
            )
            {
		if(*ptr == NULL)
		    continue;

		tar_obj_ptr = *ptr;

		/* Skip following types of objects. */
                if((tar_obj_ptr->type <= XSW_OBJ_TYPE_GARBAGE) ||
		   (tar_obj_ptr->type == XSW_OBJ_TYPE_STREAMWEAPON) ||
                   (tar_obj_ptr->type == XSW_OBJ_TYPE_SPHEREWEAPON) ||
		   (tar_obj_ptr->type == XSW_OBJ_TYPE_AREA) ||
		   (tar_obj_ptr->type == XSW_OBJ_TYPE_ANIMATED) ||
		/* Don't hit itself. */
		    (obj_ptr == tar_obj_ptr)
		)
                    continue;

		/* Check if weapon object has any antimatter left. */
		if(obj_ptr->antimatter <= 0)
		{
		    /* No antimatter left, check for reclaim contact. */

                    /* Check for contact. */ 
                    if(Mu3DInContactPtr(obj_ptr, tar_obj_ptr))
                    {
                        if(WepReclaim(
			    object_num,		/* Weapon object. */
			    tar_obj_num		/* Source object. */
			))
			    return(REngEventMemChange);
                    }
		}
		else
		{
		    /* Weapon object has antimatter, check for hit contact. */

                    /* Don't hit its owner. */
                    if(obj_ptr->owner == tar_obj_num)
			continue;

		    /* Does target's permission level make it immune to
		     * being hit?
		     */
		    if(tar_obj_ptr->permission.uid <= ACCESS_UID_WEAPON_IMMUNE)
			continue;

		    /* Check for contact. */
                    if(Mu3DInContactPtr(obj_ptr, tar_obj_ptr))
                    {
		        REngDoHit(object_num, tar_obj_num);

                        return(REngEventMemChange);
		    }
                }
            }
        }
        /* Colission check: Stream weapon */
        else if(obj_ptr->type == XSW_OBJ_TYPE_STREAMWEAPON)
        {
	    /* Stream weapon object must have power. */
	    if(obj_ptr->power <= 0)
		return(REngEventNone);

	    /* Begin checking for contact. */

	    /* Go through XSW objects list. */
            for(tar_obj_num = 0, ptr = xsw_object;
                tar_obj_num < total_objects;
                tar_obj_num++, ptr++
	    )
            {
                if(*ptr == NULL)
                    continue;

                tar_obj_ptr = *ptr;

                /* Skip following objects. */
                if((tar_obj_ptr->type <= XSW_OBJ_TYPE_GARBAGE) ||
                   (tar_obj_ptr->type == XSW_OBJ_TYPE_STREAMWEAPON) ||
                   (tar_obj_ptr->type == XSW_OBJ_TYPE_SPHEREWEAPON) ||
                   (tar_obj_ptr->type == XSW_OBJ_TYPE_AREA) ||
                   (tar_obj_ptr->type == XSW_OBJ_TYPE_ANIMATED) ||
                   /* Don't hit its owner. */
                   (obj_ptr->owner == tar_obj_num)
                )
                        continue;

                /*   Does target's permission level make it immune to
                 *   being hit?
                 */
                if(tar_obj_ptr->permission.uid <= ACCESS_UID_WEAPON_IMMUNE)
                    continue;

		/* Check if in same sector. If not, then this will save
		 * us some unnessasary calculations.
		 */
	        if(!Mu3DInSameSectorPtr(obj_ptr, tar_obj_ptr))
		    continue;

                /*   Did it hit something? Get distance away
		 *   in screen units.
		 */
                distance = Mu3DDistance(
                    tar_obj_ptr->x - obj_ptr->x,
                    tar_obj_ptr->y - obj_ptr->y,
                    tar_obj_ptr->z - obj_ptr->z
                ) * 1000;
                if(distance > 0)
                {
                    heading_variance = atan2(
			tar_obj_ptr->size,	/* dy. */
			distance		/* dx. */
                    );
                }
                else
                {
		    /*   No distance away, so partical hits partical
		     *   regardless of their bearings to each other.
		     */
                    heading_variance = 2 * PI;
                }

		/* Check if in vector contact. */
                if(Mu3DInVectorContact(
		        xsw_object, total_objects,
                        object_num,		/* Source. */
                        tar_obj_num,		/* Target. */
                        obj_ptr->heading,	/* Source to target. */
                        heading_variance,
                        obj_ptr->size		/* Size of source. */
                ))
                {
                    REngDoHit(object_num, tar_obj_num);

                    /*   Instead of recycling the stream weapon object,
                     *   we set its power to 0.
                     *
                     *   Must referance this by index since REngDoHit()
                     *   might have realigned global object pointers!
                     */
                    xsw_object[object_num]->power = 0;

                    return(REngEventMemChange);
                }
	    }
	}
        /* Colission check: Sphere weapon */
        else if(obj_ptr->type == XSW_OBJ_TYPE_SPHEREWEAPON)
        {
            /* Stream weapon object must have power. */
            if(obj_ptr->power <= 0)
                return(REngEventNone);

            /* Begin checking for contact. */

	    /* Go through XSW objects list. */
            for(tar_obj_num = 0, ptr = xsw_object;
                tar_obj_num < total_objects;
                tar_obj_num++, ptr++
	    )
            {
                if(*ptr == NULL)
                    continue;
             
                tar_obj_ptr = *ptr;

                /* Skip following objects. */
                if((tar_obj_ptr->type <= XSW_OBJ_TYPE_GARBAGE) ||
                   (tar_obj_ptr->type == XSW_OBJ_TYPE_STREAMWEAPON) ||
                   (tar_obj_ptr->type == XSW_OBJ_TYPE_SPHEREWEAPON) ||
                   (tar_obj_ptr->type == XSW_OBJ_TYPE_AREA) ||
                   (tar_obj_ptr->type == XSW_OBJ_TYPE_ANIMATED) ||
                   /* Don't hit its owner. */
                   (obj_ptr->owner == tar_obj_num)
                )
                        continue;

                /*   Does target's permission level make it immune to
                 *   being hit?
                 */
                if(tar_obj_ptr->permission.uid <= ACCESS_UID_WEAPON_IMMUNE)
                    continue;

                /* Did it hit something? */
                if(Mu3DInContactPtr(obj_ptr, tar_obj_ptr))
		{
                    REngDoHit(object_num, tar_obj_num);

                    /*   Instead of recycling the stream weapon object,
                     *   we set its power to 0.
                     *
                     *   Must referance this by index since REngDoHit()
                     *   might have realigned global object pointers!
                     */
                    xsw_object[object_num]->power = 0;

                    return(REngEventMemChange);
		}
	    }
	}

	return(REngEventNone);
}

/*
 *	Handles AI (Artificial Intelligence) for controlled objects.
 */
int REngAI(int object_num)
{
	int status = REngEventNone;

	int type, sel_wep, weapons_fired, iff_code;
	int i, closest_fobject, closest_uobject;

	xswo_ai_flags_t ai_flags;
	xsw_object_struct *obj_ptr, **ptr;
	xsw_weapons_struct *wep_ptr;

	long closest_wep_range;		/* In pixel units. */
	double	scanner_range,		/* In real units. */
		distance,		/* In real units. */
		closest_fdistance,	/* In real units. */
		closest_udistance;	/* In real units. */
	double ovisibility;

        char obj_name[XSW_OBJ_NAME_MAX + 80];
        char text[(3 * XSW_OBJ_NAME_MAX) + 256];

        const int loc_str_len = 256;
        char loc_str[loc_str_len];


        /* Get obj_ptr (assumed valid). */
        obj_ptr = xsw_object[object_num];


	/* Must be of type controlled. */
	if(obj_ptr->type != XSW_OBJ_TYPE_CONTROLLED)
	    return(REngEventNone);


	/* Reset closest objects. */
	closest_fobject = -1;
	closest_uobject = -1;

	/* Get this object's AI flags. */
	ai_flags = obj_ptr->ai_flags;

	/* Get this object's scanner range. */
	scanner_range = obj_ptr->scanner_range;
	closest_fdistance = scanner_range;
	closest_udistance = scanner_range;

	/* Go through objects list. */
	for(i = 0, ptr = xsw_object;
            i < total_objects;
            i++, ptr++
	)
	{
/* What about objects that need to follow objects across sectors? */

	    /* Skip ourselves. */
	    if(obj_ptr == *ptr)
		continue;

	    /* Checks if objects are valid and in the same sector. */
	    if(!Mu3DInSameSectorPtr(obj_ptr, *ptr))
		continue;

	    /* React to only these types of objects. */
	    type = (*ptr)->type;
	    if((type != XSW_OBJ_TYPE_CONTROLLED) &&
               (type != XSW_OBJ_TYPE_PLAYER) &&
               (type != XSW_OBJ_TYPE_WEAPON)
	    )
		continue;


	    /* If an object is set hide from connection, it
	     * should also be hidden to AI's.
	     */
	    if((*ptr)->server_options & XSW_OBJF_HIDEFROMCON)
		continue;


	    /* Get other object's visibility. */
	    ovisibility = DBGetObjectVisibilityPtr(*ptr);
	    /* Calculate distance in XSW real units. */
	    distance = Mu3DDistance(
		(*ptr)->x - obj_ptr->x,
		(*ptr)->y - obj_ptr->y,
		(*ptr)->z - obj_ptr->z
	    );

	    /* Check IFF. */
	    iff_code = MatchIFFPtr(obj_ptr, *ptr);
	    if(iff_code == IFF_FRIENDLY)
	    {
		/* Friendly. */
		/* Range check */
	        if((distance > (scanner_range * ovisibility)) ||
                   (distance > closest_fdistance) ||
                   (type == XSW_OBJ_TYPE_CONTROLLED)
	        )
		    continue;

		closest_fdistance = distance;
		closest_fobject = i;
	    }
	    else
	    {
		/* Hostile or unknown. */
		/* Range check */
                if((distance > (scanner_range * ovisibility)) ||
                   (distance > closest_udistance)
                )
                    continue;

                closest_udistance = distance;
                closest_uobject = i;
	    }
	}


	/* ******************************************************* */
	/* Follow: Adjust intercepting object. */
	while((ai_flags & XSW_OBJ_AI_FIRE_ANY) ||
	      (ai_flags & XSW_OBJ_AI_FOLLOW_ANY)
	)
	{
            /* Hostile or unknown (has precidence over friendly). */
            if(((ai_flags & XSW_OBJ_AI_FIRE_UNKNOWN) ||
                (ai_flags & XSW_OBJ_AI_FIRE_HOSTILE) ||
                (ai_flags & XSW_OBJ_AI_FOLLOW_UNKNOWN) ||
                (ai_flags & XSW_OBJ_AI_FOLLOW_HOSTILE)
               ) &&
               (closest_uobject > -1)
            )
            {
                if(obj_ptr->intercepting_object != closest_uobject)
                {
                    obj_ptr->intercepting_object = closest_uobject;
                    obj_ptr->locked_on = closest_uobject;
                }   
            }
	    /* Friendly. */
	    else if(((ai_flags & XSW_OBJ_AI_FIRE_FRIEND) ||
                     (ai_flags & XSW_OBJ_AI_FOLLOW_FRIEND)
                     ) && (closest_fobject > -1)
	    )
	    {
		if(obj_ptr->intercepting_object != closest_fobject)
		{
		    obj_ptr->intercepting_object = closest_fobject;
		    obj_ptr->locked_on = closest_fobject;
		}
	    }
	    else
	    {
		if(obj_ptr->intercepting_object > -1)
		{
		    obj_ptr->intercepting_object = -1;
		    obj_ptr->locked_on = -1;
		}
	    }

            break;      /* Break out of this while() loop. */
	}
	/* ******************************************************* */
	/* Follow: Adjust throttle. */
	while(ai_flags & XSW_OBJ_AI_FOLLOW_ANY)
	{
	    /* Actions in this section need to corrilate with
	     * those in the above intercepting object check
	     * since `following' implies concensus in throttle
	     * and intercept.
	     */

            /* Hostile or unknown (has precidence over friendly). */
            if(((ai_flags & XSW_OBJ_AI_FOLLOW_UNKNOWN) ||
                (ai_flags & XSW_OBJ_AI_FOLLOW_HOSTILE)
                ) &&
                (closest_uobject > -1)
            )
            {
                /* Adjust throttle. */
                if(closest_udistance > 0.6)
		{
                    obj_ptr->throttle += (0.02 * time_compensation);
		}
                else
		{
                    obj_ptr->throttle -= (0.02 * time_compensation);

                    /* Apply external dampers (brakes), this may
                     * be a bit stronger than it is equated on the
                     * client.
                     */
/*
		    obj_ptr->velocity -= (obj_ptr->thrust_power *
                        obj_ptr->power_purity * time_compensation
                    );
 */
		    obj_ptr->velocity -= (0.04 * time_compensation);
                    if(obj_ptr->velocity < 0)
                        obj_ptr->velocity = 0;
		}

                /* Sanitize throttle value. */
                if(obj_ptr->throttle > 1)
                    obj_ptr->throttle = 1;
                else if(obj_ptr->throttle < 0)
                    obj_ptr->throttle = 0;
            }
            /* Friendly. */
            else if((ai_flags & XSW_OBJ_AI_FOLLOW_FRIEND) &&
                    (closest_fobject > -1)
            )
            {
                /* Adjust throttle. */
                if(closest_fdistance > 0.6)
                {
                    obj_ptr->throttle += (0.02 * time_compensation);
                }
                else
                {
                    obj_ptr->throttle -= (0.02 * time_compensation);

                    /* Apply external dampers (brakes), this may
                     * be a bit stronger than it is equated on the
                     * client.
                     */
/*
                    obj_ptr->velocity -= (obj_ptr->thrust_power *
                        obj_ptr->power_purity * time_compensation
                    );
 */
                    obj_ptr->velocity -= (0.04 * time_compensation); 
                    if(obj_ptr->velocity < 0)
                        obj_ptr->velocity = 0; 
                }

                /* Sanitize throttle value. */
                if(obj_ptr->throttle > 1)
                    obj_ptr->throttle = 1;
                else if(obj_ptr->throttle < 0)
                    obj_ptr->throttle = 0;
            }
            else
            {    
		obj_ptr->throttle = 0;
	    }

	    break;	/* Break out of this while() loop. */
	}
	/* ******************************************************* */
        /* Fire. */
        while(ai_flags & XSW_OBJ_AI_FIRE_ANY)
	{
	    /* Same some cpu power if object has no weapons. */
	    if(obj_ptr->total_weapons <= 0)
		break;

            /* Go through weapons list, set closest_wep_range
             * to be the distance of the weapon with the longest
             * range on the object.
             */
            for(i = 0, closest_wep_range = 0;
                i < obj_ptr->total_weapons;
                i++
            )
            {
                /* Get weapon pointer and check if it's valid. */
                wep_ptr = obj_ptr->weapons[i];
                if(wep_ptr == NULL)
                    continue;

		/* In pixel units. */
                if(wep_ptr->range > closest_wep_range)
                    closest_wep_range = wep_ptr->range;
            }


            /* Check if there is a target object in range. */

            /* Hostile or unknown (precidence over friendly). */
            if(((ai_flags & XSW_OBJ_AI_FIRE_UNKNOWN) ||
                (ai_flags & XSW_OBJ_AI_FIRE_HOSTILE)
                ) &&
                (closest_uobject > -1)
            )
            {
		/* Check if unknown object is out of range. */
                if(closest_udistance > ((double)closest_wep_range / 1000))
                    break;

                /* Go through weapons list and select weapon with
                 * the closest range to fire on the unknown object.
                 */
                for(i = 0, obj_ptr->selected_weapon = -1;
                    i < obj_ptr->total_weapons;
                    i++
                )
                {
                    /* Get weapon pointer and check if its valid. */
                    wep_ptr = obj_ptr->weapons[i];
                    if(wep_ptr == NULL)
                        continue;

                    /* Is the target inside this weapons range
                     * (in XSW Real units)?
                     */ 
                    if(closest_udistance > ((double)wep_ptr->range / 1000))
                        continue;

                    /* Is this weapons range closer? */
                    if(wep_ptr->range <= closest_wep_range)
                    {
                        /* Select this weapon */
                        obj_ptr->selected_weapon = i;

                        /* Remember closest weapon range */
                        closest_wep_range = wep_ptr->range;
                    }
                }
            }
            /* Friendly. */
            else if((ai_flags & XSW_OBJ_AI_FIRE_FRIEND) &&
                    (closest_fobject > -1)
            )
            {
		/* Check if friendly object is out of range. */
                if(closest_fdistance > ((double)closest_wep_range / 1000))
                    break;

                /* Go through weapons list and select weapon with
		 * the closest range to fire on the friendly object.
		 */
                for(i = 0, obj_ptr->selected_weapon = -1;
                    i < obj_ptr->total_weapons;
                    i++
                )
                {
                    /* Get weapon pointer and check if its valid. */
                    wep_ptr = obj_ptr->weapons[i];
                    if(wep_ptr == NULL)
                        continue;

                    /* Is the target inside this weapons range
                     * (in XSW Real units)?
                     */
                    if(closest_fdistance > ((double)wep_ptr->range / 1000))
                        continue;

                    /* Is this weapons range closer? */
                    if(wep_ptr->range <= closest_wep_range)
                    {
                        /* Select this weapon */
                        obj_ptr->selected_weapon = i;

                        /* Remember closest weapon range */
                        closest_wep_range = wep_ptr->range;
                    }
                }
            }
            else
            {
                break;
            }

            /* Note: Procedure here is similar to the procedure
             * in function NetHandleFireWeapon().
             */ 

	    /* Get selected weapon pointer and check if its valid
	     * (if object has no weapons, then execution never
	     * gets past this point).
	     */
	    sel_wep = obj_ptr->selected_weapon;
	    if((sel_wep < 0) || (sel_wep >= obj_ptr->total_weapons))
		break;

	    wep_ptr = obj_ptr->weapons[sel_wep];
	    if(wep_ptr == NULL)
		break;


	    /* Too early to fire again? */
            if((wep_ptr->delay + wep_ptr->last_used) >= cur_millitime)
		break;

	    /* Cannot fire weapon while cloaked. */
	    if(obj_ptr->cloak_state == CLOAK_STATE_UP)
            {
		wep_ptr->last_used = cur_millitime;
		break;
	    }

            /* If weapon is expendable, see if object has any left. */
            if(wep_ptr->emission_type == WEPEMISSION_PROJECTILE)
            {
                if(wep_ptr->amount <= 0)
                    break;
	    }

            /* Does object have enough power to fire weapon? */
            if(obj_ptr->power < wep_ptr->create_power)
		break; 

            /* Create (fire) the weapon object. */
            /* Stream weapons. */
            if(wep_ptr->emission_type == WEPEMISSION_STREAM)
            {
                weapons_fired = WepCreate(
		    wep_ptr->ocs_code,		/* OCS. */
		    object_num,			/* Owner. */
		    wep_ptr->emission_type,	/* Emission type. */
		    256,			/* Create max. */
		    obj_ptr->x,
		    obj_ptr->y,
		    obj_ptr->heading,
		    obj_ptr->pitch,
		    wep_ptr->power,
		    wep_ptr->range,
		    SWR_FREQ_MIN	/* Minimal frequency for now. */
		);
            }
            /* Projectile. */
            else if(wep_ptr->emission_type == WEPEMISSION_PROJECTILE)
            {
		weapons_fired = WepCreate(
		    wep_ptr->ocs_code,		/* OCS. */
		    object_num,			/* Owner. */
		    wep_ptr->emission_type,	/* Emission type. */
		    wep_ptr->amount,		/* Create max. */
		    obj_ptr->x,
                    obj_ptr->y,
                    obj_ptr->heading,
		    obj_ptr->pitch,
                    wep_ptr->power,
                    wep_ptr->range,
		    SWR_FREQ_MIN	/* Minimal frequency for now. */
		);
	    }
	    /* All else assume pulse. */
	    else
	    {
                weapons_fired = WepCreate(
                    wep_ptr->ocs_code,		/* OCS. */
                    object_num,			/* Owner. */
                    wep_ptr->emission_type,	/* Emission type. */
                    256,			/* Create max. */
                    obj_ptr->x,
                    obj_ptr->y,
                    obj_ptr->heading,
                    obj_ptr->pitch,
                    wep_ptr->power,
                    wep_ptr->range,
                    SWR_FREQ_MIN	/* Minimal frequency for now. */
                );
	    }

            /* Reget object pointer, creating object(s) above may
             * change the pointer value.
             */
            obj_ptr = xsw_object[object_num];

	    /* Update status to report memory change. */
	    status = REngEventMemChange;

            /* No objects created? */
            if(weapons_fired <= 0)
                break;


            /* Mark the last time it was fired. */
            wep_ptr->last_used = cur_millitime;


	    /* Get object name for logging purposes. */
	    strncpy(obj_name, DBGetFormalNameStr(object_num),
		XSW_OBJ_NAME_MAX + 80
	    );
	    obj_name[XSW_OBJ_NAME_MAX + 80 - 1] = '\0';

            /* Format location string. */  
            UNVLocationFormatString(
                loc_str,
                &obj_ptr->sect_x, &obj_ptr->sect_y, &obj_ptr->sect_z,
                &obj_ptr->x, &obj_ptr->y, &obj_ptr->z,
                loc_str_len
            );

            /* Consume power on object for the power that was
             * needed to create this weapon and export event.
             */
            switch(wep_ptr->emission_type)
            {
              case WEPEMISSION_PROJECTILE:
                obj_ptr->power -= (wep_ptr->create_power * weapons_fired);
                /* Decrease stock for projectiles. */
                wep_ptr->amount -= weapons_fired;
                if(wep_ptr->amount < 0)
                    wep_ptr->amount = 0;

		/* Export projectile weapons fire. */
                sprintf(text,
                    "%s fired %s at %s.",
                    obj_name,
                    DBGetOCSOPMName(wep_ptr->ocs_code),
                    loc_str
                );
		ExportEvents(
                    fname.events_export,
                    EVST_TYPE_PROJECTILEWEAPON_FIRED,
                    text
                );
		break;

	      case WEPEMISSION_PULSE:
                obj_ptr->power -= (wep_ptr->create_power * weapons_fired);
                break;
            
              /* Default to stream weapon. */
              default:
                obj_ptr->power -= (wep_ptr->create_power);
 	        break;
	    }
            /* Sanitize power on object. */
            if(obj_ptr->power < 0)
                obj_ptr->power = 0;

            /* Set next object values update to now so it gets updated now. */
/*
            next.object_values = cur_millitime;
            next.need_weapon_values = 1;
 */

	    break;	/* Break out of this while() loop. */
	}


	return(status);
}


/*
 *	Procedure to completely update the object.
 */
int REngObject(int object_num)
{
	int status;

	/* object_num is assumed valid and non-garbage. */


	REngVisibility(object_num);
	REngShieldVisibility(object_num);

	status = REngMortality(object_num);
	if(status == REngEventMemChange)
	{
	    /* Implies object was recycled. */
	    return(REngEventMemChange);
	}

        REngAnimated(object_num);

	REngPowerCore(object_num);
        REngDamageControl(object_num);

        REngWeaponsLock(object_num);
	REngTractorLock(object_num);
	REngIntercept(object_num);

	REngThrottle(object_num);
        REngThrust(object_num);

	REngHeading(object_num);
        REngLocation(object_num);

	status = REngCollisionCheck(object_num);
	if(status == REngEventMemChange)
	{
	    /* Implies object was recycled. */
            return(REngEventMemChange);
	}

	status = REngAI(object_num);
	if(status == REngEventMemChange)
        {
            return(REngEventMemChange);
        }


	return(REngEventNone);
}


/* ******************************************************************** */

/*
 *      Initializes reality engine.
 */
int REngInit()
{
        return(0);
}


/*
 *      Manages all reality engine responsibilities.
 */
void REngManage()
{
	int status, object_num;
	xsw_object_struct **obj_ptr;


	/* Go through XSW objects list. */
        for(object_num = 0, obj_ptr = xsw_object;
            object_num < total_objects;
            object_num++, obj_ptr++
	)
	{
	    if(*obj_ptr == NULL)
		continue;
	    if((*obj_ptr)->type <= XSW_OBJ_TYPE_GARBAGE)
		continue;

	    status = REngObject(object_num);

	    /* Reget global objects pointer on memory change. */
	    if(status == REngEventMemChange)
		obj_ptr = &(xsw_object[object_num]);
	}

	return;
}


/*
 *      Shuts down reality engine.
 */
void REngShutdown() 
{
        return;
}
