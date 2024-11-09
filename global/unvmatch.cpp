// global/unvmatch.cpp
/*
                            XSW Object Matching

	Functions:

	int UNVIsObjectGarbage(
	        xsw_object_struct **xsw_object,
	        int total_objects,
	        int n
	)
	int UNVIsObjectNameIndex(const char *name)

	double UNVGetObjectVisibility(
	        xsw_object_struct **xsw_object,
	        int total_objects,
	        int n
	)
	double UNVGetObjectVisibilityPtr(xsw_object_struct *obj_ptr)

	int MatchWeaponsLock(
	        xsw_object_struct **xsw_object,
	        int total_objects,
		int ref_obj,
		int start_obj,
		double range
	)
	int MatchIntercept(
	        xsw_object_struct **xsw_object,
	        int total_objects,
	        int object_num,
	        const char *arg
	)
	int MatchInterceptByNumber(
	        xsw_object_struct **xsw_object,
	        int total_objects,
	        int ref_obj, int tar_obj
	)
	int MatchObjectByName(
	        xsw_object_struct **xsw_object,
	        int total_objects,  
	        const char *name, int type
	)

	int MatchIFF(
	        xsw_object_struct **xsw_object,
	        int total_objects,
	        int ref_obj, int tar_obj
	)
	int MatchIFFPtr(
	        xsw_object_struct *ref_obj_ptr,
	        xsw_object_struct *tar_obj_ptr
	)

	---

 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <ctype.h>

#include "../include/xsw_ctype.h"
#include "../include/string.h"
#include "../include/reality.h"
#include "../include/isrefs.h"

#include "../include/unvmath.h"
#include "../include/unvmatch.h"


/*
 *	Returns true if object n is invalid or is of
 *	type XSW_OBJ_TYPE_GARBAGE.
 *
 *	Returns false if the object is valid and non-garbage.
 */
int UNVIsObjectGarbage(
        xsw_object_struct **xsw_object,
        int total_objects,
        int n
)
{
	if((xsw_object == NULL) ||
           (n < 0) ||
           (n >= total_objects)
	)
            return(-1);
	else if(xsw_object[n] == NULL)
	    return(-1);
        else if(xsw_object[n]->type <= XSW_OBJ_TYPE_GARBAGE)
            return(1);
        else
            return(0);
}

/*
 *	Checks if the object name is in index notation (returns true)
 *	or a regular name (returns false).
 */
int UNVIsObjectNameIndex(const char *name)
{
	if(name == NULL)
	    return(0);
	else if((*name) == '#')
	    return(1);
	else
	    return(0);
}

/*           
 *      Returns object n's visibility, can be 0 if the object is
 *	invalid or garbage.
 */         
double UNVGetObjectVisibility(
	xsw_object_struct **xsw_object,
	int total_objects,
	int n
)
{
        if(UNVIsObjectGarbage(xsw_object, total_objects, n))
            return(0.00);
        else 
            return(UNVGetObjectVisibilityPtr(xsw_object[n]));
}

double UNVGetObjectVisibilityPtr(xsw_object_struct *obj_ptr)
{
        if(obj_ptr == NULL)
            return(0.00);

        if(obj_ptr->type == XSW_OBJ_TYPE_AREA)
            return(obj_ptr->cur_visibility);
	else if(obj_ptr->loc_type == XSW_LOC_TYPE_NEBULA)
	    return(obj_ptr->cur_visibility * VISIBILITY_NEBULA);
        else
            return(obj_ptr->cur_visibility);
}

/*
 *	Returns the next object number that is within the specified
 *      range (in Real units) of ref_obj.
 *
 *	ref_object should be the object that is doing the weapons lock,
 *	and must not be -1.
 *
 *      start_object can be -1.
 *
 *	range should be the the maximum scanner range of ref_object.
 *
 *      Returns the next object number of a valid and non-garbage
 *      object within range of ref_object starting from start_object,
 *      or returns -1 if no match can be made.
 */
int MatchWeaponsLock(
        xsw_object_struct **xsw_object,
        int total_objects,
        int ref_obj,  
        int start_obj,
        double range
)
{
	int i, f;
	xsw_object_struct *obj_ptr, *ref_obj_ptr;


	/* Check if referance object is valid. */
	if(UNVIsObjectGarbage(xsw_object, total_objects, ref_obj))
	    return(-1);
	else
	    ref_obj_ptr = xsw_object[ref_obj];

	/* Sanitize start object (can be -1). */
        if(start_obj < -1)
            start_obj = -1;

	/* Range must be positive. */
	if(range < 0)
	    range = 1;


	/* Begin search, note that start_obj could be -1. */
	for(i = start_obj + 1, f = -1;
            i < total_objects;
            i++
	)
	{
	    obj_ptr = xsw_object[i];

	    if(obj_ptr == NULL)
		continue;
	    if(obj_ptr->type <= XSW_OBJ_TYPE_GARBAGE)
		continue;

/*
	    if((obj_ptr->type == XSW_OBJ_TYPE_WEAPON) ||
               (obj_ptr->type == XSW_OBJ_TYPE_STREAMWEAPON) ||
               (obj_ptr->type == XSW_OBJ_TYPE_SPHEREWEAPON) ||
               (obj_ptr->type == XSW_OBJ_TYPE_AREA)
	    )
		continue;
 */

	    /* Don't match referance object. */
	    if(obj_ptr == ref_obj_ptr)
		continue;

	    /* See if object is in range. */
	    if(Mu3DInRangePtr(obj_ptr, ref_obj_ptr,
		    range * UNVGetObjectVisibilityPtr(obj_ptr)
	       )
	    )
	    {
	        /* Checks passed, this is the object we want. */
	        f = i;
	        break;
	    }
	}


	return(f);
}



/*
 *	Tries to match object specified in arg.  If arg
 *	is "" or "#off" then -1 is returned.   If the object specified in arg
 *	is found and is of type XSW_OBJ_TYPE_STATIC then that object's number
 *	is returned.  If the object is found but not of type static, then it 
 *	is checked to see if it is within scanner range of object_num and if 
 *	it is, then it's number is returned or of not in range then -1 is    
 *	returned.
 */
int MatchIntercept(
        xsw_object_struct **xsw_object,
        int total_objects,
        int object_num,
	const char *arg
)
{
	int target_obj;
	xsw_object_struct *obj_ptr, *tar_obj_ptr;
	char name[XSW_OBJ_NAME_MAX];


	if(arg == NULL)
	    return(-1);

	if(UNVIsObjectGarbage(xsw_object, total_objects, object_num))
            return(-1);
	else
	    obj_ptr = xsw_object[object_num];


	/* Sanitize copy over name to be searched. */
	strncpy(name, arg, XSW_OBJ_NAME_MAX);
	name[XSW_OBJ_NAME_MAX - 1] = '\0';
	StringStripSpaces(name);


	/* Turn intercept off? */
	if(!strcasecmp(name, "#off") ||
	   (*name == '\0')
	)
	    return(-1);


	/* Match target_obj. */
	target_obj = MatchObjectByName(
	    xsw_object,
	    total_objects,
	    name,
	    -1
	);
	if(UNVIsObjectGarbage(xsw_object, total_objects, target_obj))
	    return(-1);
	else
            tar_obj_ptr = xsw_object[target_obj];


	/* Cannot intercept itself. */
	if(target_obj == object_num)
	    return(-1);


	/* Following types of objects may not be intercepted. */
	if((tar_obj_ptr->type == XSW_OBJ_TYPE_STREAMWEAPON) ||
           (tar_obj_ptr->type == XSW_OBJ_TYPE_SPHEREWEAPON)
	)
	    return(-1);

        /* Following objects don't need range check. */
        if((tar_obj_ptr->type == XSW_OBJ_TYPE_STATIC) ||
	   (tar_obj_ptr->type == XSW_OBJ_TYPE_HOME) ||
           (tar_obj_ptr->type == XSW_OBJ_TYPE_AREA)
	)
            return(target_obj);


        /* Is target_obj within scanner range of object_num? */
        if(Mu3DInRangePtr(
		obj_ptr, tar_obj_ptr,
                obj_ptr->scanner_range *
                UNVGetObjectVisibilityPtr(tar_obj_ptr)
            )
        )
	    return(target_obj);
	else
	    return(-1);
}


/*
 *	Checks if tar_obj is within intercept range of ref_obj,
 *	and that both are valid.  Returns tar_obj if everything
 *	checks out okay or -1 if not.
 */
int MatchInterceptByNumber(
        xsw_object_struct **xsw_object,
        int total_objects,
        int ref_obj, int tar_obj
)
{
        xsw_object_struct *obj_ptr;
        xsw_object_struct *tar_obj_ptr;


        if(UNVIsObjectGarbage(xsw_object, total_objects, ref_obj))
            return(-1);
        else
            obj_ptr = xsw_object[ref_obj];

        if(UNVIsObjectGarbage(xsw_object, total_objects, tar_obj))
            return(-1);
        else
            tar_obj_ptr = xsw_object[tar_obj];


        /* ref_obj and tar_obj cannot be the same. */
        if(ref_obj == tar_obj)
            return(-1);


        /* Skip these types of objects. */
        if((tar_obj_ptr->type == XSW_OBJ_TYPE_WEAPON) ||
           (tar_obj_ptr->type == XSW_OBJ_TYPE_STREAMWEAPON) ||
           (tar_obj_ptr->type == XSW_OBJ_TYPE_SPHEREWEAPON)
        )
            return(-1);


        /* Following objects always matchable regardless of distance. */
        if((tar_obj_ptr->type == XSW_OBJ_TYPE_STATIC) ||
	   (tar_obj_ptr->type == XSW_OBJ_TYPE_HOME) ||
	   (tar_obj_ptr->type == XSW_OBJ_TYPE_AREA)
	)
            return(tar_obj);


        /* If none of the above, then do scanner range check. */
        if(Mu3DInRangePtr(obj_ptr, tar_obj_ptr,
                obj_ptr->scanner_range *
                UNVGetObjectVisibilityPtr(tar_obj_ptr)
            )
        )
	    return(tar_obj);
	else
            return(-1);
}


/*
 *	Matches object by name, return its number or -1 on error or
 *	no match.
 *
 *	The search can be narrowed down to a certain type of object
 *	or -1 for all types.
 */
int MatchObjectByName(
        xsw_object_struct **xsw_object,
        int total_objects,
        const char *name, int type
)
{
	int obj_num;
	xsw_object_struct *obj_ptr;


	/* Name cannot be NULL or contain no characters. */
	if(name == NULL)
	    return(-1);

	/* Skip leading spaces in name. */
	while(ISBLANK(*name))
	    name++;

	/* Empty string? */
        if((*name) == '\0')
            return(-1);

	/* Number matching? */
	if(UNVIsObjectNameIndex(name))
	{
	    /* Skip leading '#' characters. */
	    while((*name) == '#')
		name++;

	    /* Get object number directly from name string. */
	    obj_num = atoi(name);

	    /* Check if object is valid and allocated. */
	    if((obj_num < 0) || (obj_num >= total_objects))
		return(-1);
	    else
		obj_ptr = xsw_object[obj_num];
	    if(obj_ptr == NULL)
		return(-1);
	    if(obj_ptr->type <= XSW_OBJ_TYPE_GARBAGE)
		return(-1);

            /* Match all types of objects? */
	    if(type == -1)
		return(obj_num);

	    /* Match specific type. */
	    if(obj_ptr->type == type)
		return(obj_num);
	    else
		return(-1);
	}


	/* Go through XSW objects list. */
	for(obj_num = 0; obj_num < total_objects; obj_num++)
	{
	    obj_ptr = xsw_object[obj_num];
	    if(obj_ptr == NULL)
		continue;
	    if(obj_ptr->type <= XSW_OBJ_TYPE_GARBAGE)
		continue;

	    /* Object must characters in its name. */
	    if(obj_ptr->name[0] == '\0')
		continue;

	    /* Match all types? */
	    if(type == -1)
	    {
		/* Partial name match. */
		if(strstr(obj_ptr->name, name) == NULL)
		    continue;

		/* Got match. */
		return(obj_num);
	    }
	    /* Match specific type. */
	    else
	    {
		/* Match type first. */
		if(obj_ptr->type != type)
		    continue;

                /* Partial name match. */
                if(strstr(obj_ptr->name, name) == NULL)
                    continue;

                /* Got match. */
                return(obj_num);
	    }
	}


	/* No match. */
	return(-1);
}

/*
 *	Returns IFF of ref_obj to tar_obj.
 */
int MatchIFF(
        xsw_object_struct **xsw_object,
        int total_objects,
        int ref_obj, int tar_obj
)
{
	if(UNVIsObjectGarbage(xsw_object, total_objects, ref_obj))
	    return(IFF_UNKNOWN);
	else if(UNVIsObjectGarbage(xsw_object, total_objects, tar_obj))
	    return(IFF_UNKNOWN);
	else
	    return(MatchIFFPtr(
		xsw_object[ref_obj],
		xsw_object[tar_obj]
	    ));
}

/*
 *	Returns IFF of ref_obj_ptr to tar_obj_ptr
 */
int MatchIFFPtr(
	xsw_object_struct *ref_obj_ptr,
	xsw_object_struct *tar_obj_ptr
)
{
	char *strptr_ref, *strptr_tar;


	if((ref_obj_ptr == NULL) ||
           (tar_obj_ptr == NULL)
	)
	    return(IFF_UNKNOWN);

	strptr_ref = ref_obj_ptr->empire;
	strptr_tar = tar_obj_ptr->empire;

	if((strptr_ref == NULL) ||
           (strptr_tar == NULL) ||
           (*strptr_ref == '\0') ||
	   (*strptr_tar == '\0')
	)
	    return(IFF_UNKNOWN);

	if(strcasecmp(strptr_ref, strptr_tar))
	    return(IFF_UNKNOWN);
	else
	    return(IFF_FRIENDLY);
}




