/*
                XSW Objects Database: Utility Functions

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

	void DBSortEconomyProducts(int object_num)

	int DBValidateObjectName(char *name)    
	int DBValidateObjectPassword(char *password)

	double DBGetObjectVisibility(int object_num)

        int DBGetTopObjectNumber()
	int DBGetObjectNumByPtr(xsw_object_struct *obj_ptr)
	char *DBGetFormalNameStr(int object_num)
        char *DBGetObjectVectorName(double theta)

	int DBSetPlayerObject(int object_num)

	---


 */

#include "../include/string.h"

#include "../include/unvmatch.h"
#include "../include/unvutil.h"
#include "../include/unvfile.h"

#include "xsw.h"
#include "net.h"


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
            /* No such product in list, allocate a new one. */

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
 *      Sets src_obj to `tractor lock' on to tar_obj.
 *
 *      If tar_obj is -1 then src_obj will untractor all objects.
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


        /* tar_obj is assumed valid at this point. */


        /* Sanitize total. */
        if(obj_ptr->total_tractored_objects < 0)
            obj_ptr->total_tractored_objects = 0;
 
        /* Maximum objects tractorable excceded? */
        if(obj_ptr->total_tractored_objects > MAX_TRACTORED_OBJECTS)
	{
	    /* Cannot tractor any more objects. */
            return(-3);
	}
                    
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
 *      Alphabitizes the products on object_num by name of
 *      the product.  If object has no economy or has economy
 *      but no products, then no operation is performed.
 */
void DBSortEconomyProducts(int object_num)
{
        int i, n, k;
        xsw_ecodata_struct *eco_ptr;
        xsw_object_struct *obj_ptr;
        xsw_ecoproduct_struct **product_ptr;
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
            product_ptr,        /* Destination. */
            eco_ptr->product,   /* Source. */
            eco_ptr->total_products * sizeof(xsw_ecoproduct_struct *)
        );


        /* Sort products. */
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
                    (int)object_num, (int)strc,
                    (int)eco_ptr->total_products
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
 *	Returns object number of highest non-garbage object.
 *
 *	Returns -1 on error or no objects found.
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


/*
 *	Returns the object's index number by matching the
 *	given pointer to the object (assumed to be in the xsw_object
 *	list).
 */
int DBGetObjectNumByPtr(xsw_object_struct *obj_ptr)
{
	int i;
        xsw_object_struct **ptr;


	if(obj_ptr == NULL)
	    return(-1);

        for(i = 0, ptr = xsw_object;
            i < total_objects;
            i++, ptr++
        )
        {
            if(*ptr == obj_ptr)
                return(i);
        }

        return(-1);
}


/*
 *      Returns a statically allocated string containing the object's
 *      complete formal name in the format of name(#12345).
 *
 *      Or XSW_TYPE_NAME_GARBAGE if the object is garbage.
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
 *      Returns the appropriate `shipsman direction' name
 *      based on theta in radians.
 */
char *DBGetObjectVectorName(double theta)
{
        if(theta < 0.39269908) return("forward");
        else if(theta < 1.1780972) return("starboard bow");
        else if(theta < 1.9634954) return("starboard");
        else if(theta < 2.7488936) return("aft starboard");
        else if(theta < 3.5342917) return("aft");
        else if(theta < 4.3196899) return("aft port");
        else if(theta < 5.1050881) return("port");
        else if(theta < 5.8904862) return("port bow");
        else return("forward");
}


/*
 *	Sets local player object number and all its referances
 *	to that of object_num.
 *
 *	If object_num is -1 or garbage, then player object referances
 *	will be reset.
 */
void DBSetPlayerObject(int object_num)
{
	if(DBIsObjectGarbage(object_num))
	{
	    net_parms.player_obj_num = -1;
	    net_parms.player_obj_ptr = NULL;
	}
	else
	{
	    net_parms.player_obj_num = object_num;
            net_parms.player_obj_ptr = xsw_object[object_num];
	}


        /* Recreate the selected weapon viewscreen label. */
        VSDrawUpdateWeaponLabel(
            &bridge_win.vs_weapon_image,
            bridge_win.vs_weapon_buf
        );


	return;
}
