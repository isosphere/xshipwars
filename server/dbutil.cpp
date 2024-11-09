/*
                     Object Management Utilities

	Functions:

        int DBCreateObjectEconomyProduct(
                int object_num,
                char *product_name,
                xswo_credits_t sell_price,
		xswo_credits_t buy_price,
                double product_amount,
                double product_max
        )

        int DBObjectTractor(int src_obj, int tar_obj)
	int DBIsObjectTractorablePtr(
		xsw_object_struct *src_obj_ptr,
		xsw_object_struct *tar_obj_ptr
	)
        void DBObjectUntractor(int src_obj, int tar_obj)

	void DBObjectDoSetSector(
	        int object_num,
	        long sect_x, long sect_y, long sect_z,
		int allow_wrapping
	)

	void DBSortEconomyProducts(int object_num)

        char *DBGetTypeName(int type)
        char *DBGetFormalNameStr(int object_num)
	char *DBGetOCSOPMName(int ocs_num)

        int DBValidateObjectName(char *name)
        int DBValidateObjectPassword(char *password)

        double DBGetObjectVisibility(int object_num)
	double DBGetObjectVisibilityPtr(xsw_object_struct *obj_ptr)

	int DBGetTopObjectNumber()

	---

 */

#include "../include/unvmatch.h"
#include "../include/unvmath.h"
#include "../include/unvutil.h"

#include "swserv.h"


/*
 *      Allocates economy product structure on object_num as needed.
 *      Returns -1 on error.
 */
int DBCreateObjectEconomyProduct(
        int object_num,
        char *product_name,
        xswo_credits_t sell_price,
        xswo_credits_t buy_price,
        double product_amount,
        double product_max
)
{
        int i, n;
        xsw_ecodata_struct *eco_ptr;
        xsw_object_struct *obj_ptr;
   
             
        if(product_name == NULL)
            return(-2);

        if(DBIsObjectGarbage(object_num))
            return(-1);
        else
            obj_ptr = xsw_object[object_num];
        
              
        /* Allocate economy structure as needed. */
        if(obj_ptr->eco == NULL)
        {
            if(UNVAllocEco(obj_ptr))
                return(-1);
        }

        /* Get pointer to eco data. */
        eco_ptr = obj_ptr->eco;

        /* Sanitize total. */
        if(eco_ptr->total_products < 0)
            eco_ptr->total_products = 0;

        /* Is product_name in list? */
        for(i = 0, n = -1; i < eco_ptr->total_products; i++)
        {
            if(eco_ptr->product[i] == NULL)
	    {
		n = i;
                continue;
	    }

            if(!strcasecmp(eco_ptr->product[i]->name, product_name))
                break;
        }
        if(i >= eco_ptr->total_products)
        {
            /* No such product, allocate a new one. */

	    if(n > -1)
	    {
		/* Have allocated pointer n. */

		i = n;
	    }
	    else
	    {
		/* No unallocated pointers, allocate more. */

                i = eco_ptr->total_products; 

                eco_ptr->total_products += 1;

                eco_ptr->product = (xsw_ecoproduct_struct **)realloc(
                    eco_ptr->product,
                    eco_ptr->total_products * sizeof(xsw_ecoproduct_struct *)
                );
                if(eco_ptr->product == NULL)
                {
                    eco_ptr->total_products = 0;
                    return(-1);
                }
	    }

	    /* Allocate structure. */
            eco_ptr->product[i] = (xsw_ecoproduct_struct *)calloc(
		1,
                sizeof(xsw_ecoproduct_struct)
            );
            if(eco_ptr->product[i] == NULL)
            {
                return(-1);
            }
        }


        /* New product index is now i. */
        strncpy(
            eco_ptr->product[i]->name,
            product_name,
            ECO_PRODUCT_NAME_MAX
        );
        eco_ptr->product[i]->name[ECO_PRODUCT_NAME_MAX - 1] = '\0';

        eco_ptr->product[i]->sell_price = sell_price;
	eco_ptr->product[i]->buy_price = buy_price;
        eco_ptr->product[i]->amount = product_amount;
        eco_ptr->product[i]->amount_max = product_max;



        return(0);
}


/*
 *      Sets src_obj to lock tractor beam on tar_obj.
 */
int DBObjectTractor(int src_obj, int tar_obj)
{
	int n;
	xsw_object_struct *obj_ptr;


        if(DBIsObjectGarbage(src_obj))
            return(-1);
        else
            obj_ptr = xsw_object[src_obj];
              
	/* If tar_obj is -1 then unlock all tractor beam objects. */
	if(tar_obj < 0)
	{
	    free(obj_ptr->tractored_object);
            obj_ptr->tractored_object = NULL;

	    obj_ptr->total_tractored_objects = 0;

	    return(0);
	}


        /* tar_obj is assumed valid. */

        /* Sanitize total_tractored_objects. */
        if(obj_ptr->total_tractored_objects < 0)
            obj_ptr->total_tractored_objects = 0;

        /* Skip if maximum is excceded. */
        if(obj_ptr->total_tractored_objects > MAX_TRACTORED_OBJECTS)
            return(-3);


        /* Allocate new tractored object. */
	n = obj_ptr->total_tractored_objects;
        obj_ptr->total_tractored_objects++;

        obj_ptr->tractored_object = (int *)realloc(
            obj_ptr->tractored_object,
            obj_ptr->total_tractored_objects * sizeof(int)
        );
        if(obj_ptr->tractored_object == NULL)
        {
            obj_ptr->total_tractored_objects = 0;
            return(-1);
        }

        /* Set new tractored object. */
        obj_ptr->tractored_object[n] = tar_obj;


        return(0);
}


/*
 *	Checks if source object can tractor target object.
 */
int DBIsObjectTractorablePtr(
	xsw_object_struct *src_obj_ptr,
	xsw_object_struct *tar_obj_ptr
)
{
	if((src_obj_ptr == NULL) ||
           (tar_obj_ptr == NULL)
	)
	    return(0);

	/* Check if objects are in range. */
	if(!Mu3DInRangePtr(src_obj_ptr, tar_obj_ptr, MAX_TRACTOR_BEAM_LEN))
	    return(0);

	/*   Check if target object has shields up and if so, check
	 *   if target object's power is greater than the source
	 *   object's.
	 */
	if((tar_obj_ptr->shield_state == SHIELD_STATE_UP) &&
           ((tar_obj_ptr->power * tar_obj_ptr->power_purity) >=
            (src_obj_ptr->power * src_obj_ptr->power_purity)
	   )
	)
	    return(0);


	return(1);
} 


/*
 *      Looks for tar_obj in tractored objects list on
 *      src_obj and unlocks.
 */
void DBObjectUntractor(int src_obj, int tar_obj)
{
	int i, n;
	xsw_object_struct *obj_ptr;


        if(DBIsObjectGarbage(src_obj))
            return;
        else
            obj_ptr = xsw_object[src_obj];


        /* Sanitize total. */
        if(obj_ptr->total_tractored_objects < 0)   
            obj_ptr->total_tractored_objects = 0;

	/* If tar_obj is -1 then unlock all tractor beam objects
	 * for now.
	 */
	if(tar_obj < 0)
	{
	    free(obj_ptr->tractored_object);
            obj_ptr->tractored_object = NULL;

            obj_ptr->total_tractored_objects = 0;

	    return;
	}


        /* tar_obj is assumed valid. */


        /* Untractor object. */
        for(i = 0; i < obj_ptr->total_tractored_objects; i++)
        {
            if(obj_ptr->tractored_object[i] == tar_obj)
                obj_ptr->tractored_object[i] = -1;
        }


        /* Reclaim tractored objects memory. */
        n = -1;
        for(i = 0; i < obj_ptr->total_tractored_objects; i++)
        {
            if(DBIsObjectGarbage(obj_ptr->tractored_object[i]))
                continue;
            else
                n = i;
        }

        /* Reclaim memory as needed. */
        obj_ptr->total_tractored_objects = n + 1;
        if(obj_ptr->total_tractored_objects > 0)
        {
            obj_ptr->tractored_object = (int *)realloc(
                obj_ptr->tractored_object,
                obj_ptr->total_tractored_objects * sizeof(int)
            );
            if(obj_ptr->tractored_object == NULL)
            {
                obj_ptr->total_tractored_objects = 0;
		return;
            }
	}
	else
	{
            free(obj_ptr->tractored_object);
            obj_ptr->tractored_object = NULL;

            obj_ptr->total_tractored_objects = 0;
        }

        return;
}


/*
 *	Procedure to change sector of an object in accordance with
 *	sector changing rules. Returns 0 on success, -1 on error,
 *	or -3 if the sector change would violate sector changing
 *	rules.
 *
 *	Tractored objects (if any) will have their sectors changed as
 *	well.
 *
 *	If allow_wrapping is set to 1, then the position of
 *	the object (and its tractored objects if any) will have
 *	their positions wrapped.
 */
int DBObjectDoSetSector(
	int object_num,
	long sect_x, long sect_y, long sect_z,
	int allow_wrapping
)
{
	int i, tract_obj_num;
        xsw_object_struct *obj_ptr, *tract_obj_ptr;
        long sect_dx, sect_dy, sect_dz;


	if(DBIsObjectGarbage(object_num))
	    return(-1);
	else
	    obj_ptr = xsw_object[object_num];

        /* Is there a change in sectors? */
        if((obj_ptr->sect_x != sect_x) ||
           (obj_ptr->sect_y != sect_y) ||
           (obj_ptr->sect_z != sect_z)
        )
        {
	    /* Get sector change deltas. */
            sect_dx = sect_x - obj_ptr->sect_x;
            sect_dy = sect_y - obj_ptr->sect_y; 
            sect_dz = sect_z - obj_ptr->sect_z;


	    if(allow_wrapping)
	    {
		/* Wrap object position within sector. */
                if(sect_dx < 0)
                    obj_ptr->x += sector_legend.x_len;
                else if(sect_dx > 0)
                    obj_ptr->x -= sector_legend.x_len;

                if(sect_dy < 0)
                    obj_ptr->y += sector_legend.y_len;
                else if(sect_dy > 0)
                    obj_ptr->y -= sector_legend.y_len;
/*
                if(sect_dz < 0)
                    obj_ptr->z += sector_legend.z_len;
                else if(sect_dz > 0)
                    obj_ptr->z -= sector_legend.z_len;
*/
	    }

            /* Sanitize coordinates. */
            if(obj_ptr->x < sector_legend.x_min)
                obj_ptr->x = sector_legend.x_min + 1;
            if(obj_ptr->x > sector_legend.x_max)
                obj_ptr->x = sector_legend.x_max - 1;

            if(obj_ptr->y < sector_legend.y_min)
                obj_ptr->y = sector_legend.y_min + 1;
            if(obj_ptr->y > sector_legend.y_max)
                obj_ptr->y = sector_legend.y_max - 1;

/*
            if(obj_ptr->z < sector_legend.z_min)
                obj_ptr->z = sector_legend.z_min + 1;
            if(obj_ptr->z > sector_legend.z_max)
                obj_ptr->z = sector_legend.z_max - 1;
 */

            /* Set new sector of object. */
            obj_ptr->sect_x = sect_x;
            obj_ptr->sect_y = sect_y;
            obj_ptr->sect_z = sect_z;

            /* Go through list of tractored objects. */
            for(i = 0; i < obj_ptr->total_tractored_objects; i++)
            {
                tract_obj_num = obj_ptr->tractored_object[i];
                if(DBIsObjectGarbage(tract_obj_num))
                    continue;   
                else
                    tract_obj_ptr = xsw_object[tract_obj_num];

                /* Set tractored object's sector to match object's. */
                tract_obj_ptr->sect_x = sect_x; 
                tract_obj_ptr->sect_y = sect_y;
                tract_obj_ptr->sect_z = sect_z; 

                /* Move tractored object's coordinate position
                 * within the sector to the object's position.
                 */
                tract_obj_ptr->x = obj_ptr->x;
                tract_obj_ptr->y = obj_ptr->y;
/*
		tract_obj_ptr->z = obj_ptr->z;
 */
            }
        }


	return(0);
}


/*
 *	Alphabitizes the products on object_num by name of
 *	the product.  If object has no economy or has economy
 *	but no products, then no operation is performed.
 */
void DBSortEconomyProducts(int object_num)
{
	int i, n, k;
	xsw_ecodata_struct *eco_ptr;
	xsw_object_struct *obj_ptr;
	xsw_ecoproduct_struct **product_ptr;	/* To hold tmp ptrs. */
	char **strv;
	int strc;


	if(DBIsObjectGarbage(object_num))
	    return;
	else
	    obj_ptr = xsw_object[object_num];


	eco_ptr = obj_ptr->eco;
	if(eco_ptr == NULL)
	    return;

	if(eco_ptr->total_products <= 0)
	    return;


	/* Build products name list in strv. */
	for(i = 0, strv = NULL, strc = 0;
            i < eco_ptr->total_products;
            i++
	)
	{
	    if(eco_ptr->product[i] == NULL)
		continue;

	    /* Copy name to string list. */
	    strc++;
	    strv = (char **)realloc(strv, strc * sizeof(char *));
	    if(strv == NULL)
		return;

	    strv[strc - 1] = StringCopyAlloc(
		eco_ptr->product[i]->name
	    );
	    if(strv[strc - 1] == NULL)
	    {
		strc--;
		continue;
	    }
	}

	/* Got no strings? */
	if(strc == 0)
	    return;

	/* Sort strings. */
	strv = StringQSort(strv, strc);
	if(strv == NULL)
	    return;


	/* Copy products pointers to tempory pointer array. */
	product_ptr = (xsw_ecoproduct_struct **)calloc(
	    1,
	    eco_ptr->total_products * sizeof(xsw_ecoproduct_struct *)
	);
	memcpy(
	    product_ptr,	/* Destination. */
	    eco_ptr->product,	/* Source. */
	    eco_ptr->total_products * sizeof(xsw_ecoproduct_struct *)
	);


	/* Sort products with respect to the strv list. */
	for(i = 0, k = 0; i < strc; i++)
        {
	    if(k >= eco_ptr->total_products)
		break;

	    if(strv[i] == NULL)
		continue;

	    /* Get pointer matching string in tempory products list. */
	    for(n = 0; n < eco_ptr->total_products; n++)
	    {
                if(product_ptr[n] == NULL)
                    continue;

		if(!strcasecmp(product_ptr[n]->name, strv[i]))
		    break;
	    }
	    if(n < eco_ptr->total_products)
	    {
		/* Matched product name, now put it in order. */

		eco_ptr->product[k] = product_ptr[n];
	        k++;
	    }
	    else
	    {
		fprintf(stderr,
 "DBSortEconomyProducts(): Warning: Missing name from tmp products array (#%i %i %i)\n",
		    object_num, strc, eco_ptr->total_products
		);
	    }
	}
	/* Reallocate the rest of the product pointers. */
	if(k > 0)
	{
	    eco_ptr->total_products = k;
	    eco_ptr->product = (xsw_ecoproduct_struct **)realloc(
		eco_ptr->product,
		eco_ptr->total_products * sizeof(xsw_ecoproduct_struct *)
	    );
	    if(eco_ptr->product == NULL)
	    {
		eco_ptr->total_products = 0;
		/* Don't return, need to free other stuff. */
	    }
	}
	else
	{
	    free(eco_ptr->product);
	    eco_ptr->product = NULL;

	    eco_ptr->total_products = 0;
	}


	/* Free tempory products pointers. */
	free(product_ptr);
	product_ptr = NULL;

	/* Free string array. */
	StringFreeArray(strv, strc);


	return;
}


/*
 *      Return a statically allocated string containing the official
 *      name for the given object type code.
 */     
char *DBGetTypeName(int type)
{
        switch(type)
        {
          case XSW_OBJ_TYPE_ERROR:
            return(XSW_TYPE_NAME_ERROR);
            break;

          case XSW_OBJ_TYPE_GARBAGE:
            return(XSW_TYPE_NAME_GARBAGE);
            break;

          case XSW_OBJ_TYPE_STATIC:
            return(XSW_TYPE_NAME_STATIC);
            break; 

          case XSW_OBJ_TYPE_DYNAMIC:
            return(XSW_TYPE_NAME_DYNAMIC);
            break;

          case XSW_OBJ_TYPE_CONTROLLED:
            return(XSW_TYPE_NAME_CONTROLLED);
            break;

          case XSW_OBJ_TYPE_PLAYER:
            return(XSW_TYPE_NAME_PLAYER);
            break;
  
          case XSW_OBJ_TYPE_WEAPON:
            return(XSW_TYPE_NAME_WEAPON);   
            break;

          case XSW_OBJ_TYPE_STREAMWEAPON:
            return(XSW_TYPE_NAME_STREAMWEAPON);
            break;

          case XSW_OBJ_TYPE_SPHEREWEAPON:
            return(XSW_TYPE_NAME_SPHEREWEAPON);
            break;

          case XSW_OBJ_TYPE_HOME:
            return(XSW_TYPE_NAME_HOME);
            break;

          case XSW_OBJ_TYPE_AREA:
            return(XSW_TYPE_NAME_AREA);
            break;

          case XSW_OBJ_TYPE_ANIMATED:
            return(XSW_TYPE_NAME_ANIMATED);
            break;

          case XSW_OBJ_TYPE_WORMHOLE:
            return(XSW_TYPE_NAME_WORMHOLE);
            break;

          case XSW_OBJ_TYPE_ELINK:
            return(XSW_TYPE_NAME_ELINK);
            break;

          default:
            return("*Unknown*");
            break;
        }

        return("*Unknown*");
}


/*
 *      Returns a statically allocated format name string in
 *      the format of Name(#12345)
 */
char *DBGetFormalNameStr(int object_num)
{
        if(DBIsObjectGarbage(object_num))
            return(XSW_OBJ_GARBAGE_NAME);
        else
            return(UNVGetObjectFormalName(
                xsw_object[object_num],
                object_num
            ));
}

/*
 *	Returns a pointer to the name of the OPM referanced
 *	by the given OCS code. Can return NULL on no match or
 *	error.
 *
 *	Do not free the returned pointer.
 */
char *DBGetOCSOPMName(int ocs_code)
{
	int ocs_num;


	ocs_num = OCSGetByCode(ocs_code);
	if(ocs_num < 0)
	    return(NULL);

	return(ocs[ocs_num]->opm_name);
}

/*      
 *      Validates name for an object.  Returns non-zero if it is
 *      NOT valid.
 *      
 *      -1  general error.
 *      0   no error.
 *      1   name too long.
 *      2   name too short.
 */        
int DBValidateObjectName(char *name)
{          
        int i, n;  
        char *strptr = INVALID_NAME_CHARACTERS;


        /* Null pointer? */
        if(name == NULL) return(-1);

        /* Empty string? */
        if(name[0] == '\0') return(2);

        /* Invalid characters? */
        for(i = 0; name[i] != '\0'; i++)   
        {
            if(i >= XSW_OBJ_NAME_MAX)
                return(1);

            for(n = 0; strptr[n] != '\0'; n++)
            {
                if(name[i] == strptr[n])
                    return(3);
            }
        }


        return(0);
}


/*         
 *      Checks if this is a valid password.
 */        
int DBValidateObjectPassword(char *password)
{
        return(0);
}


/*
 *      Returns object n's visibility, can be 0 if the object is
 *      invalid or garbage.
 */
double DBGetObjectVisibility(int object_num)
{
        return(
            UNVGetObjectVisibility(
                xsw_object,
                total_objects,
                object_num
            )
        );
}

double DBGetObjectVisibilityPtr(xsw_object_struct *obj_ptr)
{
        return(UNVGetObjectVisibilityPtr(obj_ptr));
}

/*
 *      Returns object number of highest non-garbage object.
 *
 *      Returns -1 on error or no objects found.
 */
int DBGetTopObjectNumber()
{
        int i, h;
        xsw_object_struct **ptr, *obj_ptr;


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

	return(h);
}
