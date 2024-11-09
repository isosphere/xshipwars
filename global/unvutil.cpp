// global/unvutil.cpp
/*
                     Universe Object Management Utilities

	Functions:


	int UNVAllocScores(xsw_object_struct *obj_ptr)
	int UNVAllocObjectWeapons(
		xsw_object_struct *obj_ptr, int total_weapons
	)
	int UNVAllocEco(xsw_object_struct *obj_ptr)

	char *UNVGetObjectFormalName(
		xsw_object_struct *obj_ptr,
		int obj_num
	)
	xsw_object_struct *UNVDupObject(xsw_object_struct *obj_ptr)
	void UNVResetObject(xsw_object_struct *obj_ptr)
	void UNVDeleteObject(xsw_object_struct *obj_ptr)
	void UNVDeleteAllObjects(xsw_object_struct **obj_ptr, int total)

	void UNVParseLocation(
	        const char *s,
	        long *sect_x, long *sect_y, long *sect_z,
	        double *x, double *y, double *z
	)
	void UNVLocationFormatString(
		char *s,
		const long *sect_x, const long *sect_y, const long *sect_z,
		const double *x, const double *y, const double *z,
		int len
	)

	void UNVParseDirection(  
	        const char *s,
	        double *heading, double *pitch, double *bank
	)
	void UNVDirectionFormatString(
	        char *s,
	        const double *heading, const double *pitch, const double *bank,
	        int len
	)

	---

 */

#include <stdio.h>
//#include <ctype.h>
#include <malloc.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "../include/xsw_ctype.h"
#include "../include/string.h"
#include "../include/reality.h"
#include "../include/objects.h"

#include "../include/unvmain.h"
#include "../include/unvmath.h"
#include "../include/unvutil.h"
#include "../include/unvmatch.h"
#include "../include/unvfile.h"



/*
 *	Allocates a score structure as needed on object.
 */
int UNVAllocScores(xsw_object_struct *obj_ptr)
{
	if(obj_ptr == NULL)
	    return(-1);

	if(obj_ptr->score == NULL)
	{
	    obj_ptr->score = (xsw_score_struct *)calloc(
		1,
                sizeof(xsw_score_struct)
	    );
	    if(obj_ptr->score == NULL)
                return(-1);
	}

	return(0);
}

/*
 *	Allocates or deallocates weapons on object as needed.
 */
int UNVAllocObjectWeapons(xsw_object_struct *obj_ptr, int total_weapons)
{
        int i, prev;
        xsw_weapons_struct **ptr, *wep_ptr;


        if(obj_ptr == NULL)
	    return(-1);


        /* Deallocate all weapons? */
        if(total_weapons <= 0)
        {
            /* Deallocate all weapons on object. */
            for(i = 0, ptr = obj_ptr->weapons;
                i < obj_ptr->total_weapons;   
                i++, ptr++
            )       
                free(*ptr);

            free(obj_ptr->weapons);
            obj_ptr->weapons = NULL;

            obj_ptr->total_weapons = 0;
            obj_ptr->selected_weapon = -1;
        }
	else
	{
	    /* Total weapons is 1 or greater. */

            /* Sanitize total. */
            if(total_weapons > MAX_WEAPONS)
                total_weapons = MAX_WEAPONS;

            /* Sanitize total on object. */
            if(obj_ptr->total_weapons < 0)
                obj_ptr->total_weapons = 0;

	    /* Record previous total on object. */
	    prev = obj_ptr->total_weapons;


            /* Deallocate weapons? */
	    if(total_weapons < prev)
	    {
                /* Deallocate weapons on object. */
		for(i = total_weapons; i < prev; i++)
		{
                    /* Unselect weapon if it will be deleted. */
                    if(obj_ptr->selected_weapon == i)
                        obj_ptr->selected_weapon = -1;

                    free(obj_ptr->weapons[i]);
                    obj_ptr->weapons[i] = NULL;
                }

                obj_ptr->weapons = (xsw_weapons_struct **)realloc(
                    obj_ptr->weapons,
                    total_weapons * sizeof(xsw_weapons_struct *)
                );
                if(obj_ptr->weapons == NULL)
                {
                    obj_ptr->total_weapons = 0;
                    obj_ptr->selected_weapon = -1;
                    return(-1);
                }

                /* Set new total on object. */
                obj_ptr->total_weapons = total_weapons;
            }
	    /* Allocate more weapons? */
            else if(total_weapons > prev)
            {
                /* Allocate more weapons on object. */
                obj_ptr->weapons = (xsw_weapons_struct **)realloc(
                    obj_ptr->weapons,
                    total_weapons * sizeof(xsw_weapons_struct *)
                );
                if(obj_ptr->weapons == NULL)
                {
                    obj_ptr->total_weapons = 0;
                    obj_ptr->selected_weapon = -1;
                    return(-1);
                }

                for(i = prev; i < total_weapons; i++)
                {
                    obj_ptr->weapons[i] = (xsw_weapons_struct *)malloc(
                        sizeof(xsw_weapons_struct)
                    );
                    if(obj_ptr->weapons[i] == NULL)
                    {
                        obj_ptr->total_weapons = i;
                        obj_ptr->selected_weapon = -1;
                        return(-1);
                    }
		    else
		    {
			/* Reset values. */
			wep_ptr = obj_ptr->weapons[i];

			wep_ptr->flags = 0;
                        wep_ptr->ocs_code = 0;
                        wep_ptr->emission_type = WEPEMISSION_STREAM;
                        wep_ptr->amount = 0;
                        wep_ptr->max = 0;
                        wep_ptr->power = 0;
                        wep_ptr->range = 0;
                        wep_ptr->create_power = 0;
			wep_ptr->delay = 0;
			wep_ptr->last_used = 0;

			wep_ptr->use_sound_code = 0;
			wep_ptr->fire_sound_code = 0;
			wep_ptr->hit_sound_code = 0;
			wep_ptr->recover_sound_code = 0;
			*wep_ptr->name = '\0';
		    }
                }

                /* Set new total on object. */
                obj_ptr->total_weapons = total_weapons;
	    }
	}

        /* Sanitize selected weapon. */
        if(obj_ptr->selected_weapon >= obj_ptr->total_weapons)
            obj_ptr->selected_weapon = obj_ptr->total_weapons - 1;
        if(obj_ptr->selected_weapon < 0)
            obj_ptr->selected_weapon = -1;

        return(0);
}

/*
 *	Allocates an economy structure as needed.
 */
int UNVAllocEco(xsw_object_struct *obj_ptr)
{
        if(obj_ptr == NULL)
            return(-1);

        if(obj_ptr->eco == NULL)
        {
            obj_ptr->eco = (xsw_ecodata_struct *)calloc(
                1, 
                sizeof(xsw_ecodata_struct)
            );
            if(obj_ptr->eco == NULL)
                return(-1);
        }
        
        return(0);
}

/*
 *	Returns a statically allocated string containing the object's
 *	formal name.
 */
char *UNVGetObjectFormalName(xsw_object_struct *obj_ptr, int obj_num)
{
	char *strptr;
	static char name[XSW_OBJ_NAME_MAX + 80];


	if(obj_ptr == NULL)
	    return(XSW_OBJ_GARBAGE_NAME);

	if(obj_ptr->type <= XSW_OBJ_TYPE_GARBAGE)
	    return(XSW_OBJ_GARBAGE_NAME);

	if(obj_num < 0)
	{
	    /* Object index number is invalid, implies garbage. */
	    strncpy(name, XSW_OBJ_GARBAGE_NAME, XSW_OBJ_NAME_MAX);
	    name[XSW_OBJ_NAME_MAX - 1] = '\0';
	    return(name);
	}
	else if(obj_ptr->type <= XSW_OBJ_TYPE_GARBAGE)
	{
	    /* Object type is garbage. */
            strncpy(name, XSW_OBJ_GARBAGE_NAME, XSW_OBJ_NAME_MAX);
            name[XSW_OBJ_NAME_MAX - 1] = '\0';
            return(name);
	}

	sprintf(
	    name,
	    "%s(#%i",
	    obj_ptr->name,
	    obj_num
	);
	strptr = name;
	while(*strptr != '\0')
	    strptr++;

	/* Type. */
	switch(obj_ptr->type)
	{
	  case XSW_OBJ_TYPE_DYNAMIC:
	    *strptr++ = 'D';
	    break;

	  case XSW_OBJ_TYPE_CONTROLLED:
            *strptr++ = 'C';
            break;

          case XSW_OBJ_TYPE_PLAYER:
            *strptr++ = 'P';
            break;

          case XSW_OBJ_TYPE_WEAPON:
            *strptr++ = 'W';
            break;

          case XSW_OBJ_TYPE_STREAMWEAPON:
            *strptr++ = 'W';
            break;

          case XSW_OBJ_TYPE_SPHEREWEAPON:
            *strptr++ = 'W';
            break;

          case XSW_OBJ_TYPE_HOME:
            *strptr++ = 'H';
            break;

          case XSW_OBJ_TYPE_AREA:
            *strptr++ = 'A';
            break;

          case XSW_OBJ_TYPE_ANIMATED:
            *strptr++ = 'V';
            break;

          case XSW_OBJ_TYPE_WORMHOLE:
            *strptr++ = 'G';
            break;

          case XSW_OBJ_TYPE_ELINK:
            *strptr++ = 'L';
            break;

          default:
            *strptr++ = '-';
            break;
	}

	if(obj_ptr->eco != NULL)
	    *strptr++ = 'E';

	*strptr++ = ')';

	/* Null terminate. */
        *strptr = '\0';


	return(name);
}


/*
 *	Dynamically allocates a new object in memory that contains
 *	the exact same values (except for pointers to substructures) as
 *	obj_ptr.  Can return NULL on error.
 *
 *	Calling function must use UNVDeleteObject() to deallocate
 *	this object structure and its substructures.
 */
xsw_object_struct *UNVDupObject(xsw_object_struct *obj_ptr)
{
	int i;
	xsw_object_struct *new_obj_ptr = NULL;


	if(obj_ptr == NULL)
	    return(new_obj_ptr);


	/* Allocate new object structure. */
	new_obj_ptr = (xsw_object_struct *)calloc(
	    1,
	    sizeof(xsw_object_struct)
	);
	if(new_obj_ptr == NULL)
	    return(new_obj_ptr);


	/* Copy over core values. */
        memcpy(
            new_obj_ptr,
            obj_ptr,
            sizeof(xsw_object_struct)
        );


	/* Copy over substructures. */

	/* Weapons. */
	if(obj_ptr->weapons != NULL)
	{
	    /* Copy over each weapon. */
	    new_obj_ptr->weapons = (xsw_weapons_struct **)calloc(
		1,
		new_obj_ptr->total_weapons * sizeof(xsw_weapons_struct *)
	    );
	    if(new_obj_ptr->weapons == NULL)
	    {
		new_obj_ptr->total_weapons = 0;
	    }

	    for(i = 0; i < new_obj_ptr->total_weapons; i++)
	    {
		if(obj_ptr->weapons[i] == NULL)
		    continue;

		new_obj_ptr->weapons[i] = (xsw_weapons_struct *)calloc(
		    1,
		    sizeof(xsw_weapons_struct)
		);
		if(new_obj_ptr->weapons[i] == NULL)
		    continue;

		memcpy(
		    new_obj_ptr->weapons[i],
		    obj_ptr->weapons[i],
		    sizeof(xsw_weapons_struct)
		);
	    }
	}

	/* Score. */
        if(obj_ptr->score != NULL)
        {
            new_obj_ptr->score = (xsw_score_struct *)malloc(
                sizeof(xsw_score_struct)
            );
            if(new_obj_ptr->score != NULL)
            {
		memcpy(
		    new_obj_ptr->score,
		    obj_ptr->score,
		    sizeof(xsw_score_struct)
		);
	    }
	}

        /* Economy. */
        if(obj_ptr->eco != NULL)
        {
            new_obj_ptr->eco = (xsw_ecodata_struct *)malloc(
                sizeof(xsw_ecodata_struct)
            );
            if(new_obj_ptr->eco != NULL)  
            {
                memcpy(
                    new_obj_ptr->eco,
                    obj_ptr->eco,
                    sizeof(xsw_ecodata_struct)
                );
            }

	    /* Copy over each eco product. */
	    if(obj_ptr->eco->product != NULL)
	    {
		new_obj_ptr->eco->product = (xsw_ecoproduct_struct **)calloc(
		    1,
		    new_obj_ptr->eco->total_products * sizeof(xsw_ecoproduct_struct *)
		);
		if(new_obj_ptr->eco->product == NULL)
		{
		    new_obj_ptr->eco->total_products = 0;
		}

                for(i = 0; i < new_obj_ptr->eco->total_products; i++)
                {
                    if(obj_ptr->eco->product[i] == NULL)
                        continue;

                    new_obj_ptr->eco->product[i] = (xsw_ecoproduct_struct *)calloc(
                        1,
                        sizeof(xsw_ecoproduct_struct)
                    );
                    if(new_obj_ptr->eco->product[i] == NULL)
                        continue;

                    memcpy(
                        new_obj_ptr->eco->product[i],
                        obj_ptr->eco->product[i],
                        sizeof(xsw_ecoproduct_struct)
                    );
		}
	    }
        }



	return(new_obj_ptr);
}


/*
 *	Resets values and deallocates all substructures on
 *	object.
 */
void UNVResetObject(xsw_object_struct *obj_ptr)
{
        int i;


        if(obj_ptr == NULL)
            return;


        /* Deallocate weapons. */
        for(i = 0; i < obj_ptr->total_weapons; i++)
        {
            if(obj_ptr->weapons[i] == NULL)
                continue;

            free(obj_ptr->weapons[i]);
        }
        free(obj_ptr->weapons);
        obj_ptr->weapons = NULL;

        obj_ptr->total_weapons = 0;
        obj_ptr->selected_weapon = -1;

 
        /* Deallocate tractored objects. */
        free(obj_ptr->tractored_object);
        obj_ptr->tractored_object = NULL;
        obj_ptr->total_tractored_objects = 0;


        /* Deallocate score structure. */
        free(obj_ptr->score);
        obj_ptr->score = NULL;


        /* Deallocate economy structure. */
        if(obj_ptr->eco != NULL)
        {           
            for(i = 0; i < obj_ptr->eco->total_products; i++)
            {
                free(obj_ptr->eco->product[i]);
                obj_ptr->eco->product[i] = NULL;
            }
            free(obj_ptr->eco->product);
            obj_ptr->eco->product = NULL;
        }
        free(obj_ptr->eco);
        obj_ptr->eco = NULL;


	/* Free elink. */
	free(obj_ptr->elink);
	obj_ptr->elink = NULL;

        /* Reset values to that of the unv_garbage_object. */
        memcpy(
            obj_ptr,			/* Destination. */
            &unv_garbage_object,	/* Source. */
            sizeof(xsw_object_struct)
        );


	return;
}


/*
 *	Deallocates object and all its substructures.
 */
void UNVDeleteObject(xsw_object_struct *obj_ptr)
{
        if(obj_ptr == NULL)
	    return;


        /* Free object's allocated resources and substructures. */
        UNVResetObject(obj_ptr);

        /* Free object base structure. */
        free(obj_ptr);


	return;
}


/*
 *	Deletes an array of objects and their substructures,
 *	including the array itself.
 */
void UNVDeleteAllObjects(xsw_object_struct **obj_ptr, int total)
{
	int i;
	xsw_object_struct **ptr;


	/* Deallocate each object. */
	for(i = 0, ptr = obj_ptr;
            i < total;
            i++, ptr++
	)
	    UNVDeleteObject(*ptr);

	/* Deallocate object pointer array. */
	free(obj_ptr);


	return;
}


/*
 *	Parses the location string s, changing the pointed to
 *	position variables only if their values are specified in
 *	the string.  Any number of the inputs may be NULL, if s is
 *	NULL, then no operation will be performed.
 *
 *	Format of string s is:
 *
 *		"<sect_x>,<sect_y>,<sect_z>:<x>,<y>,<z>"
 *
 *	Example:
 *
 *		"4,-5,0:34.0,-542.81,-12.2"
 */
void UNVParseLocation(
	const char *s,
	long *sect_x, long *sect_y, long *sect_z,
	double *x, double *y, double *z
)
{
	const char value_deliminator = ',';
	const char dimension_deliminator = ':';

        /* Sector x. */
        if(s != NULL)
        {
            while(ISBLANK(*s))
                s++;

            if(sect_x != NULL)
                *sect_x = atol(s);

            /* Seek value deliminator. */
            s = strchr(s, value_deliminator);
            if(s != NULL)
                s++;
        }

        /* Sector y. */
        if(s != NULL)
        {
            while(ISBLANK(*s))
                s++;

            if(sect_y != NULL)
                *sect_y = atol(s);

            /* Seek value deliminator. */
            s = strchr(s, value_deliminator);
            if(s != NULL)
                s++;
        }

        /* Sector z. */
        if(s != NULL)
        {
            while(ISBLANK(*s))
                s++;

            if(sect_z != NULL)
                *sect_z = atol(s);  

            /* Seek dimension deliminator. */
            s = strchr(s, dimension_deliminator);
            if(s != NULL)
                s++;
        }

        /* X. */
        if(s != NULL)
        {
            while(ISBLANK(*s))
                s++;

            if(x != NULL)
                *x = atof(s);

            /* Seek next value deliminator. */
            s = strchr(s, value_deliminator);
            if(s != NULL)
                s++;
        }

        /* Y. */
        if(s != NULL)
        {
            while(ISBLANK(*s))
                s++;

            if(y != NULL)
                *y = atof(s);

            /* Seek next value deliminator. */
            s = strchr(s, value_deliminator);
            if(s != NULL)
                s++;
        }

        /* Z. */
        if(s != NULL)
        {
            while(ISBLANK(*s))
                s++;

            if(z != NULL)
                *z = atof(s);

            /* Seek next value deliminator. */
            s = strchr(s, value_deliminator);
            if(s != NULL)
                s++;
        }

        return;
}

/*
 *	Formats string s in the standard universe location notation
 *	from the given the inputs. If any of the inputs are NULL then 0
 *	will be replaced for that value.
 *
 *	The len specifies the number of bytes that have been allocated
 *	for string s.
 *
 *	See UNVParseLocation() for the string format.
 */
void UNVLocationFormatString(
	char *s,
        const long *sect_x, const long *sect_y, const long *sect_z,
        const double *x, const double *y, const double *z,
        int len
)
{
        const char value_deliminator[] = ", ";
        const char dimension_deliminator[] = " : ";
        const char *strptr;
	char *strptr2;
        int cur_len = 0;
        char num_str[80];


        if((s == NULL) ||
           (len < 1)
        )
            return;

        *s = '\0';      /* Reset string. */


        /* Sect X. */
        sprintf(num_str, "%ld",
            ((sect_x == NULL) ? 0 : *sect_x)
        );
        cur_len += strlen(num_str);
        if(cur_len < len)
            strcat(s, num_str);

        strptr = value_deliminator;
        cur_len += strlen(strptr);
        if(cur_len < len)
            strcat(s, strptr);

        /* Sect Y. */
        sprintf(num_str, "%ld",
            ((sect_y == NULL) ? 0 : *sect_y)
        );
        cur_len += strlen(num_str);
        if(cur_len < len)
            strcat(s, num_str);

        strptr = value_deliminator;
        cur_len += strlen(strptr);
        if(cur_len < len)
            strcat(s, strptr);

        /* Sect Z. */
        sprintf(num_str, "%ld",
            ((sect_z == NULL) ? 0 : *sect_z)
        );
        cur_len += strlen(num_str);
        if(cur_len < len)
            strcat(s, num_str);

        strptr = dimension_deliminator;
        cur_len += strlen(strptr); 
        if(cur_len < len)
            strcat(s, strptr); 


        /* X. */
        sprintf(num_str, "%.4f",
            ((x == NULL) ? 0 : *x)
        );
	strptr2 = num_str;	/* Seek to end and strip 0's. */
	while(*strptr2 != '\0')
	    strptr2++;
	strptr2--;
	while(strptr2 > num_str)
	{
	    if((*strptr2 != '0') &&
               (*strptr2 != '.')
	    )
		break;

	    *strptr2-- = '\0';
	}
        cur_len += strlen(num_str);
        if(cur_len < len)
            strcat(s, num_str);

        strptr = value_deliminator;
        cur_len += strlen(strptr); 
        if(cur_len < len)
            strcat(s, strptr); 

        /* Y. */
        sprintf(num_str, "%.4f",
            ((y == NULL) ? 0 : *y)
        );
        strptr2 = num_str;      /* Seek to end and strip 0's. */
        while(*strptr2 != '\0')
            strptr2++;
        strptr2--;
        while(strptr2 > num_str)
        {
            if((*strptr2 != '0') &&
               (*strptr2 != '.')
            )
                break;

            *strptr2-- = '\0';
        }
        cur_len += strlen(num_str);
        if(cur_len < len)
            strcat(s, num_str);

        strptr = value_deliminator;
        cur_len += strlen(strptr); 
        if(cur_len < len)
            strcat(s, strptr); 

        /* Z. */
        sprintf(num_str, "%.4f",
            ((z == NULL) ? 0 : *z)
        );
        strptr2 = num_str;      /* Seek to end and strip 0's. */
        while(*strptr2 != '\0')
            strptr2++;
        strptr2--;
        while(strptr2 > num_str)
        {
            if((*strptr2 != '0') &&
               (*strptr2 != '.')   
            )
                break;

            *strptr2-- = '\0';
        }
        cur_len += strlen(num_str);
        if(cur_len < len)
            strcat(s, num_str);


	return;
}


/*
 *      Parses the direction string s, changing the pointed to
 *      direction variables only if their values are specified in
 *      the string.  Any number of the inputs may be NULL, if s is
 *      NULL, then no operation will be performed.
 *
 *      Format of string s is:
 *
 *              "<heading>,<pitch>,<bank>"
 *
 *      Example:
 *
 *              "2.32,6.22,5.14"
 */     
void UNVParseDirection(
        const char *s,
        double *heading, double *pitch, double *bank
)
{
        const char value_deliminator = ',';

        /* Heading. */
        if(s != NULL)
        {
            while(ISBLANK(*s))
                s++;

            if(heading != NULL)
                *heading = SANITIZERADIANS(atof(s));

            /* Seek value deliminator. */
            s = strchr(s, value_deliminator);
            if(s != NULL)
                s++;
        }

        /* Pitch. */
        if(s != NULL)
        {
            while(ISBLANK(*s))
                s++;

            if(pitch != NULL)
                *pitch = SANITIZERADIANS(atol(s));

            /* Seek value deliminator. */
            s = strchr(s, value_deliminator);
            if(s != NULL)
                s++;
        }

        /* Bank. */
        if(s != NULL)
        {
            while(ISBLANK(*s))
                s++;

            if(bank != NULL)
                *bank = SANITIZERADIANS(atof(s));

            /* Seek next value deliminator. */
            s = strchr(s, value_deliminator);
            if(s != NULL)
                s++;
        }
}

/*
 *	Formats string s in the standard universe direction notation
 *	from the given the inputs. If any of the inputs are NULL then 0
 *	will be replaced for that value. All units are in radians.
 *
 *	The len specifies the number of bytes that have been allocated
 *	for string s.
 *
 *	See UNVParseDirection() for the string format.
 */
void UNVDirectionFormatString(
	char *s,
        const double *heading, const double *pitch, const double *bank,
        int len
)
{
        const char value_deliminator[] = ", ";
        const char *strptr;
        int cur_len = 0;
        char num_str[80];


        if((s == NULL) ||
           (len < 1)
        )
            return; 

        *s = '\0';      /* Reset string. */


        /* Heading. */
        sprintf(num_str, "%f",
            ((heading == NULL) ? 0 : *heading)
        );
        cur_len += strlen(num_str);
        if(cur_len < len)
            strcat(s, num_str);

        strptr = value_deliminator;
        cur_len += strlen(strptr);
        if(cur_len < len)
            strcat(s, strptr);

        /* Pitch. */
        sprintf(num_str, "%f",
            ((pitch == NULL) ? 0 : *pitch)
        );
        cur_len += strlen(num_str);
        if(cur_len < len)
            strcat(s, num_str);

        strptr = value_deliminator;
        cur_len += strlen(strptr);
        if(cur_len < len)
            strcat(s, strptr);

        /* Bank. */
        sprintf(num_str, "%f",
            ((bank == NULL) ? 0 : *bank)
        );
        cur_len += strlen(num_str);
        if(cur_len < len)
            strcat(s, num_str);
}





