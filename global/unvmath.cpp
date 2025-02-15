// global/unvmath.cpp
/*
                    ShipWars Universe Math Functions

	Functions:

	double RADTODEG(double radians)
	double DEGTORAD(double degrees)
	double SANITIZERADIANS(double radians)

	double MuPolarRotX(double input_angle, double input_radius)
	double MuPolarRotY(double input_angle, double input_radius)

	void MuSetUnitVector2D(
		xsw_vector_compoent_struct *v,
		double bearing
	)
	void MuSetUnitVector3D(
		xsw_vector_compoent_struct *v,
                double bearing,
		double pitch
	)

	void MuVectorAdd(
	        double v1_dir, double v1_mag,
        	double v2_dir, double v2_mag,
        	double *v_dir_rtn, double *v_mag_rtn
	)
 	double MuCoordinateDeltaVector(double dx, double dy)
 	double Mu3DDistance(double delta_x, double delta_y, double delta_z)

	int Mu3DInSectorPtr(
	    xsw_object_struct *obj_ptr,
	    long sect_x, long sect_y, long sect_z
	)
	int Mu3DInSameSector(
	        xsw_object_struct **xsw_object,
	        int total_objects,
	        int object_num1, int object_num2
	)
        int Mu3DInSameSectorPtr(
                xsw_object_struct *obj1_ptr,
                xsw_object_struct *obj2_ptr
	)
 	int Mu3DInContact(
	        xsw_object_struct **xsw_object,
	        int total_objects,
	        int object_num1, int object_num2
	)
	int Mu3DInContactPtr(
		xsw_object_struct *obj1_ptr,
		xsw_object_struct *obj2_ptr
	)
        int Mu3DInVectorContact(
	        xsw_object_struct **xsw_object,
	        int total_objects,
	        int object_num1, int object_num2,
	        double heading,  
	        double heading_variance,
	        double range
	)
 	int Mu3DInRange(
	        xsw_object_struct **xsw_object,
	        int total_objects,
	        int object_num1, int object_num2,
	        double distance
	)
        int Mu3DInRangePtr(
		xsw_object_struct *obj1_ptr,
		xsw_object_struct *obj2_ptr,
		double distance
	)

 	double MuConvertSLUVelocityToWarp(double slu_velocity)
	double MuMaxRangeByVelocity(double velocity_max, long time)

	---

 	Math convience functions, all trigeometric functions use
 	the clock quadrant as referance and *not* the standard position
 	quadrant.    All angles must be in radians!   All distances
 	are in Real units unless indicated otherwise.
 
 	MuPolarRotX() returns the x value for a point in polar
 	coordinates.   input_angle must be in radians.
 
 	MuPolarRotY() is the same as MuPolarRotX() except it returns
        the y value.
 
 	Mu3DDistance() returns the distance of two point's delta
        distances.
 
 	Mu3DInContact() returns 1 if XSW Objects object_num1 and
 	object_num2 are in contact, by calculating their distances
 	apart and their sizes.
 
 */

#include <stdio.h>
#include <sys/types.h>
#include <math.h>

#include "../include/reality.h"
#include "../include/objects.h"

#include "../include/unvmatch.h"
#include "../include/unvmath.h"


#ifndef MIN
# define MIN(a,b)	(((a) < (b)) ? (a) : (b))
#endif
#ifndef MAX
# define MAX(a,b)	(((a) > (b)) ? (a) : (b))
#endif


/*
 *	Converts radians to degrees.
 */
double RADTODEG(double radians)
{
        return(radians * 180 / PI);
}

/*
 *	Converts degrees to radians.
 */        
double DEGTORAD(double degrees)
{
        return(degrees * PI / 180);
}

/*
 *	Sanitizes radian value to [0, (2 * PI)].
 */
double SANITIZERADIANS(double radians)
{
        while(radians >= 6.2831853)
            radians -= 6.2831853;
        
        while(radians < 0)
            radians += 6.2831853;

	return(radians);
}

/*
 *      Sanitizes degree value to [0', 360'].
 */
double SANITIZEDEGREES(double degrees)
{
        while(degrees > 360)
            degrees -= 360;

        while(degrees < 0)
            degrees += 360;

        return(degrees);
}


/*
 *	Returns the X axis coefficient of the given polar bearing
 *	coordinates.
 */
double MuPolarRotX(double theta, double r)
{
        return(
            r *
            cos(SANITIZERADIANS((PI / 2) - theta))
        );
}

/*
 *	Returns the Y axis coefficient of the given polar bearing
 *	coordinates.
 */
double MuPolarRotY(double theta, double r)
{
        return(
	    r *
	    sin(SANITIZERADIANS((PI / 2) - theta))
	);
}


/* Not sure, but it's probable that the following two functions
 * can be replaced with:

 *
 * Given a vector, normalize it to a unit vector.
 * It works the same for 2D and 3D vectors as member k
 * will remain 0 if that is the current value.
 *
void MuNormalizeVector3D(xsw_vector_component_struct* v)
{
	double	Magnitude = v->i*v->i + v->j*v->j + v->k*v->k;
	if( Mag == 0.0 )
		return;		// Vector has no length so no normalization possible.

	Magnitude = 1.0 / sqrt(MAX(Magnitude, 0.0));

	v->i *= Magnitude;
	v->j *= Magnitude;
	v->k *= Magnitude;
}

 */

/*
 *	Sets vector compoent structure members i and j to
 *	2D unit vector compoents.  Member k is set to 0.
 */
void MuSetUnitVector2D(
	xsw_vector_compoent_struct *v,
	double bearing
)
{
	if(v == NULL)
	    return;

	v->i = sin(bearing);
	v->j = cos(bearing);
	v->k = 0;

	return;
}

/*
 *	Sets vector compoent structure members i, j, and k to
 *	3D unit vector compoents.
 */
void MuSetUnitVector3D(
	xsw_vector_compoent_struct *v,
	double bearing, double pitch
)
{
	double cos_pitch;


        if(v == NULL)
            return;

	cos_pitch = cos(pitch);

        v->i = sin(bearing) * cos_pitch;
        v->j = cos(bearing) * cos_pitch;
	v->k = sin(pitch) * -1;

	return;
}


/* Not sure what the next function is used for but a proper
 * vector addition is simple:

void MuAddVector(xsw_vector_compoent_struct* res, xsw_vector_compoent_struct* v1, xsw_vector_compoent_struct* v2)
{
	res->i = v1->i + v2->i;
	res->j = v1->j + v2->j;
	res->k = v1->k + v2->k;
}
 */

/*
 *	Adds two vectors togeather, setting the new vector's
 *	direction and magnitude values in v_dir_rtn and v_mag_rtn
 *	(respectivly).
 */
void MuVectorAdd(
        double v1_dir, double v1_mag,
        double v2_dir, double v2_mag,
        double *v_dir_rtn, double *v_mag_rtn
)
{
        double x1, y1, x2, y2, xr, yr;


        if((v_dir_rtn == NULL) ||
           (v_mag_rtn == NULL)
        )
            return;


        /* Magitudes must be positive. */
        if(v1_mag < 0)
            v1_mag *= -1;
        if(v2_mag < 0)
            v2_mag *= -1;

        /* Calculate right angle displacements. */
        x1 = sin(v1_dir) * v1_mag;
        y1 = cos(v1_dir) * v1_mag;
 
        x2 = sin(v2_dir) * v2_mag;
        y2 = cos(v2_dir) * v2_mag;

	/* Add vectors togeather. */
	xr = x1 + x2;
	yr = y1 + y2;

        *v_mag_rtn = hypot(xr, yr);
        *v_dir_rtn = MuCoordinateDeltaVector(xr, yr);


        return;
}

/*
 *	Calculates vector of the unitless delta coordinates.
 */
double MuCoordinateDeltaVector(double dx, double dy)
{
	return(SANITIZERADIANS((PI / 2) - atan2(dy, dx)));
}


/*
 *	Calculates the 3D unitless delta distance.
 *
 *	KB:  Calculates the vector length or, often called,
 *		 the vector magnitude.
 */
double Mu3DDistance(double dx, double dy, double dz)
{
	return(sqrt(MAX(dx*dx + dy*dy + dz*dz, 0.0)));
}


/* Often useful optimization for comparisons.
 *
 * Calculate the squared length or magnitude of a vector.
 *
 */
double MuMagnitude2Vector(xsw_vector_compoent_struct *v)
{
	return((v->i * v->i) + (v->j * v->j) + (v->k * v->k));
}

/* The above can be used to optimize distance measurement
 * and comparison code.  Example:

xsw_vector_compoent_struct v1;
xsw_vector_compoent_struct v2;

	// Initialize the vectors here..

	// Perform distance comparison.
	if( Mu3DDistance(v1) > Mu3DDistance(v2) )

	// Is the same as:
	if( MuMagnitude2Vector(v1) > MuMagnitude2Vector(v2) )

 * The difference between the two versions is that the
 * second version will save you 2 square roots which are expensive.
 */	


/*
 *	Checks if the object is in the given sector.
 */
int Mu3DInSectorPtr(
        xsw_object_struct *obj_ptr,
        long sect_x, long sect_y, long sect_z
)
{
	if(obj_ptr == NULL)
	    return(0);

	if((obj_ptr->sect_x == sect_x) &&
           (obj_ptr->sect_y == sect_y) &&
           (obj_ptr->sect_z == sect_z)
	)
	    return(1);
	else
	    return(0);
}

/*
 *	Checks if both objects are valid and in the same sector.
 */
int Mu3DInSameSector(
	xsw_object_struct **xsw_object,
	int total_objects,
	int object_num1, int object_num2
)
{
	xsw_object_struct *o1_ptr, *o2_ptr;


        /* object_num1 and object_num2 must be valid. */
        if(UNVIsObjectGarbage(xsw_object, total_objects, object_num1) ||
           UNVIsObjectGarbage(xsw_object, total_objects, object_num2)
        )
            return(0);

	o1_ptr = xsw_object[object_num1];
        o2_ptr = xsw_object[object_num2];

	return(
	    (o1_ptr->sect_x == o2_ptr->sect_x) &&
            (o1_ptr->sect_y == o2_ptr->sect_y) &&
            (o1_ptr->sect_z == o2_ptr->sect_z)
	);
}

/*
 *	Checks if the two objects are in the same sector.
 */
int Mu3DInSameSectorPtr(
	xsw_object_struct *obj1_ptr,
	xsw_object_struct *obj2_ptr
)
{
        if((obj1_ptr == NULL) ||
           (obj2_ptr == NULL)
        )
            return(0);

	if(obj1_ptr == obj2_ptr)
	    return(1);

	return(
            (obj1_ptr->sect_x == obj2_ptr->sect_x) &&
            (obj1_ptr->sect_y == obj2_ptr->sect_y) &&
            (obj1_ptr->sect_z == obj2_ptr->sect_z)
        );
}        


/*
 *	Checks if the two objects are in contact, takes size
 *	of each object into account.
 */
int Mu3DInContact(
        xsw_object_struct **xsw_object,
        int total_objects,
        int object_num1, int object_num2  
)
{
        double d, s1, s2;	/* In Screen units. */
	xsw_object_struct *o1_ptr, *o2_ptr;


	/* Check if objects are valid and in the same sector. */
	if(!Mu3DInSameSector(
	    xsw_object, total_objects, object_num1, object_num2
	))
	    return(0);


        /* Get object pointers. */
        o1_ptr = xsw_object[object_num1];
        o2_ptr = xsw_object[object_num2];


        /* Both objects must have a size greater than 0. */
        s1 = ((o1_ptr->size < 0) ? 0 : o1_ptr->size);
        s2 = ((o2_ptr->size < 0) ? 0 : o2_ptr->size);


        /* Get the distance in Screen units. */
        d = Mu3DDistance(
            o2_ptr->x - o1_ptr->x,
            o2_ptr->y - o1_ptr->y,
            o2_ptr->z - o1_ptr->z
        ) * 1000;			/* Convert to Screen units. */


        /* Is distance less than the two sizes? */
        if(d <= (s1 + s2))
            return(1);
        else
            return(0);
}

/*
 *	Checks if the two objects are in contact.
 */
int Mu3DInContactPtr(
	xsw_object_struct *obj1_ptr,
	xsw_object_struct *obj2_ptr
)
{
	double d, s1, s2;	/* In Screen units. */


	if((obj1_ptr == NULL) ||
           (obj2_ptr == NULL)
	)
	    return(0);

        if((obj1_ptr->sect_x != obj2_ptr->sect_x) ||
           (obj1_ptr->sect_y != obj2_ptr->sect_y) ||
           (obj1_ptr->sect_z != obj2_ptr->sect_z)
        )
            return(0);


        /* Both objects must have a size greater than 0. */
        s1 = ((obj1_ptr->size < 0) ? 0 : obj1_ptr->size);
        s2 = ((obj2_ptr->size < 0) ? 0 : obj2_ptr->size);

        /* Get the distance in Screen units. */
        d = Mu3DDistance(
            obj2_ptr->x - obj1_ptr->x,
            obj2_ptr->y - obj1_ptr->y,
            obj2_ptr->z - obj1_ptr->z
        ) * 1000;                       /* Convert to Screen units. */


        /* Is distance less than the two sizes? */
        if(d <= (s1 + s2))
            return(1);
        else
            return(0);
}


/*
 *	Checks if object 1 is in contact with object 2 in the given
 *	heading (from object 1 to object 2) with the given heading
 *	variance and range in pixel units.
 *
 *	The size of object 1 is not taken into account, however the
 *	size of object 2 is.
 */
int Mu3DInVectorContact(
        xsw_object_struct **xsw_object,
        int total_objects,
        int object_num1, int object_num2,
        double heading,		/* Object 1 to object 2. */
        double heading_variance,
        double range		/* In pixel units. */
)
{
	double d, s2;			/* In Screen units. */
	double hdg_res_norm, hdg_res_lower, hdg_res_upper;
	double hdgb_min, hdgb_max;	/* Heading bounds. */
        xsw_object_struct *o1_ptr, *o2_ptr;


        /* Check if objects are valid and in the same sector. */
        if(!Mu3DInSameSector(
	    xsw_object, total_objects, object_num1, object_num2
	))
            return(0);

        /* Get object pointers. */
        o1_ptr = xsw_object[object_num1];
        o2_ptr = xsw_object[object_num2];

        /* Get size of object 2 in Screen units. */
        s2 = ((o2_ptr->size < 0) ? 0 : o2_ptr->size);

        /* Range must be positive. */
	if(range < 0)
	    range *= -1;

        /* Sanitize heading. */
	heading = SANITIZERADIANS(heading);

        /* Heading variance must be positive. */
	if(heading_variance < 0)
	    heading_variance *= -1;

	/* Calculate min and max heading bounds. */
        hdgb_min = heading - heading_variance;
	hdgb_max = heading + heading_variance;


        /* Get the distance in Screen units. */
        d = Mu3DDistance(
            o2_ptr->x - o1_ptr->x,
            o2_ptr->y - o1_ptr->y,
            o2_ptr->z - o1_ptr->z
        ) * 1000;		/* Convert to screen units. */


        /* If out of range, then definatly not in contact
	 * (do not take object 1's size into account).
	 */
        if(d > (s2 + range))
            return(0);

        /* At this point, target object is within range. */

        /* Calculate bearing from object 1 to object 2. */
        hdg_res_norm = MuCoordinateDeltaVector(
            o2_ptr->x - o1_ptr->x,
            o2_ptr->y - o1_ptr->y
        );
	hdg_res_lower = hdg_res_norm - (2 * PI);
	hdg_res_upper = hdg_res_norm + (2 * PI);

	/* Check heading result order; normal, upper, lower. */
	if((hdg_res_norm >= hdgb_min) &&
           (hdg_res_norm <= hdgb_max)
	)
	    return(1);
	else if((hdg_res_upper >= hdgb_min) &&
                (hdg_res_upper <= hdgb_max)
        )
            return(1);
        else if((hdg_res_lower >= hdgb_min) &&
                (hdg_res_lower <= hdgb_max)
        )
            return(1);
        else
            return(0);
}

/*
 *	Checks if the two objects are in the same sector and in range,
 *	both objects must be valid and distance is in Real units.
 */
int Mu3DInRange(
        xsw_object_struct **xsw_object,
        int total_objects,
	int object_num1, int object_num2,
	double distance		/* In Real units. */
)
{
	double distance_apart;
	double s1, s2;
	xsw_object_struct *o1_ptr, *o2_ptr;


	/* Check if objects are valid and in the same sector. */
        if(!Mu3DInSameSector(
	    xsw_object, total_objects, object_num1, object_num2
	))
            return(0);

        o1_ptr = xsw_object[object_num1];
        o2_ptr = xsw_object[object_num2];


        /* Distance must be positive and convert to Screen units. */
        distance = ((distance < 0) ?
            (distance * -1000) : (distance * 1000)
	);

	/* Get sizes in Screen units. */
	s1 = ((o1_ptr->size < 0) ? 0 : o1_ptr->size);
	s2 = ((o2_ptr->size < 0) ? 0 : o2_ptr->size);

        /* Get the distance in Screen units. */
        distance_apart = Mu3DDistance(
            o2_ptr->x - o1_ptr->x,
            o2_ptr->y - o1_ptr->y,
            o2_ptr->z - o1_ptr->z
        ) * 1000;

        /* See if distance apart is closer than distance. */
        if((distance_apart - (s1 + s2)) <= distance)
            return(1);
        else
            return(0);
}

/*
 *      Checks if the two objects are in the same sector and in range,
 *      both objects must be valid and distance is in Real units.
 */
int Mu3DInRangePtr(
	xsw_object_struct *obj1_ptr,
        xsw_object_struct *obj2_ptr,
        double distance		/* In Real units. */
)
{
	double distance_apart;
	double s1, s2;


	/* Check if objects are in the same sector. */
	if((obj1_ptr == NULL) ||
           (obj2_ptr == NULL)
	)
	    return(0);

	if(obj1_ptr == obj2_ptr)
	    return(1);

	if((obj1_ptr->sect_x != obj2_ptr->sect_x) ||
           (obj1_ptr->sect_y != obj2_ptr->sect_y) ||
           (obj1_ptr->sect_z != obj2_ptr->sect_z)
	)
	    return(0);

        /* Distance must be positive and convert to Screen units. */ 
        distance = ((distance < 0) ?
            (distance * -1000) : (distance * 1000)
	);

        /* Get sizes in Screen units. */
        s1 = ((obj1_ptr->size < 0) ? 0 : obj1_ptr->size);
        s2 = ((obj2_ptr->size < 0) ? 0 : obj2_ptr->size);

        /* Calculate distance apart in Screen units. */
        distance_apart = Mu3DDistance(
            obj2_ptr->x - obj1_ptr->x,
            obj2_ptr->y - obj1_ptr->y,
            obj2_ptr->z - obj1_ptr->z
        ) * 1000;

        /* See if distance apart is closer than distance. */
        if((distance_apart - (s1 + s2)) <= distance)
            return(1);
        else
            return(0);
}

/*
 *	Calculates the `theoretical' maximum range based on
 *	given maximum velocity v_max and time t.
 *
 *	If v_max is in Real units, then the returned value will be
 *	in Real units.  Same goes for XSW Screen units.
 *
 *	t must be in milliseconds.
 */
double MuMaxRangeByVelocity(double v_max, long t)
{
	return((v_max / (double)CYCLE_LAPSE_MS) * (double)t);
}




