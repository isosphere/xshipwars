/*
                      Object Parameter Macros

	Functions:

	void OPMReclaim()
        void OPMDeleteAll()
        int OPMIsGarbage(int opm_num)
        int OPMCreate(int type)
        int OPMCreateExplicit(int opm_num, int type)
        void OPMRecycle(int opm_num)
 	int OPMGetTop()
 	int OPMGetByName(char *name, int type)

	int OPMModelObjectPtr(
	        xsw_object_struct *obj_ptr,
	        xsw_object_struct *opm_ptr
	)
 	int OPMModelObject(int object_num, int opm_num)

	---

 */

#include "../include/unvmatch.h"
#include "../include/unvutil.h"
#include "swserv.h"


void OPMReclaim()
{
        int i, h;


        /* Get highest valid and allocated OPM index number. */
        h = OPMGetTop();
	if(h < 0)
            return;


        /* Do we need to reclaim memory? */
        if((total_opms - h - 1) <= OBJECT_ALLOCATE_AHEAD)
            return;


        /* Free all allocation from total_opms - 1 to h + 1. */
        for(i = total_opms - 1; i > h; i--)
        {
#ifdef DEBUG_MEM_FREE
if(opm[opm_num] != NULL)
    printf("OPM %i: Free'ed (reclaim).\n", opm_num);
#endif
	    DBDeleteObject(opm[i]);
            opm[i] = NULL;
        }


        /* Adjust total. */
        total_opms = h + 1;


        /* Reallocate OPM pointers. */
	if(total_opms > 0)
	{
            opm = DBAllocObjectPointers(
	        opm, total_opms
	    );
            if(opm == NULL)
            {
	        total_opms = 0;
                return;
            }
	}
	else
	{
#ifdef DEBUG_MEM_FREE
if(opm != NULL)
    printf("OPM pointers: Free'ed.\n");
#endif
	    free(opm);
	    opm = NULL;

	    total_opms = 0;
	}


        return;
}


/*
 *	Procedure to delete all OPMs.
 */
void OPMDeleteAll()
{
	int i;


#ifdef DEBUG_MEM_FREE
printf("OPMs: Deleting %i...\n", total_opms);
#endif

	for(i = 0; i < total_opms; i++)
	{
	    if(opm[i] == NULL)
		continue;

#ifdef DEBUG_MEM_FREE
if(opm[i] != NULL)
    printf("OPM %i: Free'ed\n", i);
#endif
	    DBDeleteObject(opm[i]);
	    opm[i] = NULL;
        }

#ifdef DEBUG_MEM_FREE
if(opm != NULL)
    printf("OPM pointers: Free'ed\n");
#endif
	free(opm);
	opm = NULL;

	total_opms = 0;


        return;
}



/*
 *	Checks if opm_num is allocated and non-garbage.
 */
int OPMIsGarbage(int opm_num)
{
        if((opm_num < 0) ||
	   (opm_num >= total_opms) ||
	   (opm == NULL)
	)
            return(-1);
        else if(opm[opm_num] == NULL)
            return(-1);
        else if(opm[opm_num]->type <= XSW_OBJ_TYPE_GARBAGE)
            return(1);
	else
            return(0);
}


/*
 *	Procedure to create an OPM of type type.
 */
int OPMCreate(int type)
{
	int opm_num, opm_rtn;
	char name[XSW_OBJ_NAME_MAX + 80];


        /* Type may not be garbage. */
        if(type <= XSW_OBJ_TYPE_GARBAGE)
        {
            fprintf(stderr,
                "OPMCreate: Error: Useless request to create garbage.\n"
            );
            return(-1);
        }


        /* Look for available OPM already allocated. */
        opm_rtn = -1;
        for(opm_num = 0; opm_num < total_opms; opm_num++)
        {
	    if(opm[opm_num] == NULL)
		continue;

            /* Look for garbage OPMs. */
            if(opm[opm_num]->type == XSW_OBJ_TYPE_GARBAGE)
            {
                opm_rtn = opm_num;
                break;
            }
        }

        /* Did we find an OPM already allocated? */
        if((opm_rtn > -1) && (opm_rtn < total_opms))
        {
	    /* Reset OPM (free all allocated substructures) */
            UNVResetObject(opm[opm_num]);

	    sprintf(name, "OPM #%i", opm_rtn);
	    name[XSW_OBJ_NAME_MAX - 1] = '\0';
	    strncpy(opm[opm_rtn]->name, name, XSW_OBJ_NAME_MAX);
	    opm[opm_rtn]->name[XSW_OBJ_NAME_MAX - 1] = '\0';

            opm[opm_rtn]->type = type;

            return(opm_rtn);
        }


	/* ************************************************************* */

        /* Not enough memory, can we allocate more? */
        if( (total_opms + OBJECT_ALLOCATE_AHEAD) > MAX_OPMS)
        {
            fprintf(stderr, "OPM maximum %i reached.\n", MAX_OPMS);
            return(-1);
        }


        /* Allocate more memory. */

        /* Allocate pointer array. */
        opm = DBAllocObjectPointers(
            opm,
            total_opms + OBJECT_ALLOCATE_AHEAD
	);
        if(opm == NULL)
        {
	    total_opms = 0;
            return(-1);
        }   


        /* Allocate each OPM structure. */
        for(opm_num = total_opms;
            opm_num < (total_opms + OBJECT_ALLOCATE_AHEAD);
            opm_num++
        )
            opm[opm_num] = DBAllocObject();


        /* New object will be the first object allocated. */
        opm_rtn = total_opms;


        /* Adjust global variable total_opms. */
	opm_num = total_opms;
        total_opms += OBJECT_ALLOCATE_AHEAD;

	/* Reset values on newly allocated objects. */
	while(opm_num < total_opms)
	{
	    UNVResetObject(opm[opm_num]);
	    opm_num++;
	}


        /* Set default values on the new object. */
        sprintf(name, "OPM #%i", opm_rtn);
        name[XSW_OBJ_NAME_MAX - 1] = '\0';

        strncpy(opm[opm_rtn]->name, name, XSW_OBJ_NAME_MAX);            
	opm[opm_rtn]->name[XSW_OBJ_NAME_MAX - 1] = '\0';

        opm[opm_rtn]->type = type;


        /* Return the number of the newly created OPM. */
        return(opm_rtn);
}


/*
 *	Create an OPM explicitly.
 */
int OPMCreateExplicit(int opm_num, int type)
{
	int opm_count, opm_rtn;
	char name[XSW_OBJ_NAME_MAX + 80];


        /* Type may not be garbage. */
        if(type <= XSW_OBJ_TYPE_GARBAGE)
        {
            fprintf(stderr,
         "OPMCreateExplicit(): Error: Useless request to create garbage.\n"
            );
            return(-1);
        }

	/* Make sure opm_num is valid. */
	else if(opm_num < 0)
        {
            fprintf(stderr,
      "OPMCreateExplicit(): Error: Request to create opm_num #%i.\n",
		opm_num
            );
            return(-1);
	}
        
         
        /* *********************************************************** */
                
        /* If object_num already exists, recycle it. */
        if(!OPMIsGarbage(opm_num))
            OPMRecycle(opm_num);


        /* Allocate memory if opm_num is greater than total_opms. */
        if(opm_num >= total_opms)
        {
            /* Can we allocate more? */
            if((opm_num + OBJECT_ALLOCATE_AHEAD) >= MAX_OPMS)
            {
                fprintf(stderr, "OPM maximum %i reached.\n", MAX_OPMS);
                return(-1);
            }


            /* Allocate pointer array. */
            opm = DBAllocObjectPointers(
                opm,
		opm_num + OBJECT_ALLOCATE_AHEAD + 1
	    );
            if(opm == NULL)
            {
		total_opms = 0;
                return(-1);
            }


            /* Allocate each new OPM. */
            for(opm_count = total_opms;
                opm_count <= (opm_num + OBJECT_ALLOCATE_AHEAD);
                opm_count++
            )
            {
		/* Allocate memory for a new OPM. */
                opm[opm_count] = DBAllocObject();
                if(opm[opm_count] == NULL)
                {
		    total_opms = opm_count;
                    return(-1);
                }
            }
                
            /* Adjust global variable total_opms. */
            total_opms = opm_num + OBJECT_ALLOCATE_AHEAD + 1;
        }
             

	/* *************************************************** */                

        opm_rtn = opm_num;

        /* Set default values on the new OPM. */
        UNVResetObject(opm[opm_rtn]);

	sprintf(name, "OPM #%i", opm_rtn);
	name[XSW_OBJ_NAME_MAX - 1] = '\0';

	strncpy(opm[opm_rtn]->name, name, XSW_OBJ_NAME_MAX);
	opm[opm_rtn]->name[XSW_OBJ_NAME_MAX - 1] = '\0';

        opm[opm_rtn]->type = type;
        

        /* Return 0 on no error, explicit OPM create does not
         * want OPM number.
         */
        return(0);
}

/*
 *	Recycles OPM.
 */
void OPMRecycle(int opm_num)
{
	xsw_object_struct *opm_ptr;


	if(OPMIsGarbage(opm_num))
	    return;
	else
	    opm_ptr = opm[opm_num];


	/* Deallocates all substructures and set default values. */
	UNVResetObject(opm_ptr);

        /* Setting OPM type to garbage marks it as recycled. */
        opm_ptr->type = XSW_OBJ_TYPE_GARBAGE;

	return;
}

/*
 *	Get highest allocated and non-garbage OPM index number.
 *
 *	Can return -1 if no allocated non-garbage OPMs are found.
 */
int OPMGetTop()
{
        int i, h;
	xsw_object_struct **ptr, *opm_ptr;

	for(i = 0, h = -1, ptr = opm;
            i < total_opms;
	    i++, ptr++
	)
	{
	    opm_ptr = *ptr;

	    if(opm_ptr == NULL)
		continue;
	    if(opm_ptr->type <= XSW_OBJ_TYPE_GARBAGE)
		continue;

	    h = i;
	}

	return(h);
}

/*
 *	Get an OPM index number by name, returns the OPM's index
 *	number (can be assumed valid) or -1 on no match/error.
 *	Can also return -2 on ambiguous (name was not specific
 *	enough).
 *
 *	If the name contains a token string (ie "$OPM:SomeObject"),
 *	then the string tailing (ie "SomeObject") will be used
 *	in the search.
 *
 *	The type cannot be set to XSW_OBJ_TYPE_GARBAGE.
 *
 *	If type is -1, then all OPM types will be matched except
 *	XSW_OBJ_TYPE_GARBAGE.
 */
int OPMGetByName(char *name, int type)
{
	int opm_count, opm_found, found_count;
	xsw_object_struct **ptr, *opm_ptr;


	if(name == NULL)
	    return(-1);

	/* The specified type cannot be garbage. */
	if(type == XSW_OBJ_TYPE_GARBAGE)
            return(-1);


	/* Skip leading spaces. */
	while(*name == ' ')
	    name++;

	/* Token string check. */
	if(*name == '$')
	{
	    char *strptr;

	    /* Seek to ':' delimiter. */
	    strptr = strchr(name, ':');
	    if(strptr == NULL)
		return(-1);

	    name = strptr + 1;
	}

	/* Specified name cannot be empty. */
	if(*name == '\0')
	    return(-1);


	/* Begin searching for an OPM. */
	opm_found = -1;
	found_count = 0;

	/* Match regardless of type? */
	if(type < 0)
	{
	    for(opm_count = 0, ptr = opm;
	        opm_count < total_opms;
	        opm_count++, ptr++
	    )
	    {
		opm_ptr = *ptr;
		if(opm_ptr == NULL)
		    continue;

	        if(opm_ptr->type <= XSW_OBJ_TYPE_GARBAGE)
		    continue;

	        if(!strcmp(opm_ptr->name, name))
	        {
		    opm_found = opm_count;
		    found_count++;
		}
	    }
	}
	/* Match by type. */
	else
	{
            for(opm_count = 0, ptr = opm;
                opm_count < total_opms;
                opm_count++, ptr++
            )
            {
		opm_ptr = *ptr;
                if(opm_ptr == NULL)
                    continue;

                if(opm_ptr->type <= XSW_OBJ_TYPE_GARBAGE)
                    continue;

		/* Types must match. */
		if(opm_ptr->type != type)
		    continue;

		if(!strcmp(opm_ptr->name, name))
		{
                    opm_found = opm_count;
                    found_count++;
		}
	    }
	}


	if(found_count > 1)
	    return(-2);		/* Ambiguous. */
	else
	    return(opm_found);
}

/*
 *	Coppies opm values on to object, any values currently
 *	on object will be reset and set to the opm's values.
 *
 *	Allocated substructures on object will be deallocated and
 *	new ones allocated as needed depending on what the opm
 * 	has.
 *
 *	Returns 0 on success or -1 on error.
 */
int OPMModelObjectPtr(
	xsw_object_struct *obj_ptr,
	xsw_object_struct *opm_ptr
)
{
	int i;


	if((obj_ptr == NULL) ||
           (opm_ptr == NULL)
	)
	    return(-1);

	/* Free any allocated substructures on object. */
	UNVResetObject(obj_ptr);

	/*   Copy the OPM values to the object, and then allocate
	 *   and COPY any dynamically allocated substructres.
	 */

	/* Copy OPM values to object. */
	memcpy(
	    obj_ptr,	/* Destination. */
	    opm_ptr,	/* Source. */
	    sizeof(xsw_object_struct)
	);


	/* Note: Substructures on opm must be coppied to object. */

        /* Weapons. */  
        if((opm_ptr->weapons != NULL) && (opm_ptr->total_weapons > 0))
        {
            obj_ptr->weapons = (xsw_weapons_struct **)calloc(
		1,
                obj_ptr->total_weapons * sizeof(xsw_weapons_struct *)
            );
	    if(obj_ptr->weapons == NULL)
	    {
		obj_ptr->total_weapons = 0;
	    }

            for(i = 0; i < obj_ptr->total_weapons; i++)
            {
		if(opm_ptr->weapons[i] == NULL)
		    continue;

                obj_ptr->weapons[i] = (xsw_weapons_struct *)MemoryCopyAlloc(
                    opm_ptr->weapons[i],
		    sizeof(xsw_weapons_struct)
		);
            }
        }
	else
	{
	    obj_ptr->weapons = NULL;
	    obj_ptr->total_weapons = 0;
	}


        /* Tractored objects. */
        if((opm_ptr->tractored_object != NULL) &&
           (opm_ptr->total_tractored_objects > 0)
        )
        {    
            obj_ptr->tractored_object = (int *)MemoryCopyAlloc(
		opm_ptr->tractored_object,
		opm_ptr->total_tractored_objects * sizeof(int)
	    );
	    if(obj_ptr->tractored_object == NULL)
		obj_ptr->total_tractored_objects = 0;
        }
        else
        {
            obj_ptr->tractored_object = NULL;
            obj_ptr->total_tractored_objects = 0;
        }


	/* Scores. */
	if(opm_ptr->score != NULL)
	{
            obj_ptr->score = (xsw_score_struct *)MemoryCopyAlloc(
                opm_ptr->score,
                sizeof(xsw_score_struct)
            );
	}
	else
	{
	    obj_ptr->score = NULL;
	}


        /* Economy. */
        if(opm_ptr->eco != NULL)
        {
            obj_ptr->eco = (xsw_ecodata_struct *)MemoryCopyAlloc(
                opm_ptr->eco,
                sizeof(xsw_ecodata_struct)
            );
            if(obj_ptr->eco != NULL)
            {
		/* Allocate product pointers. */
		obj_ptr->eco->product = (xsw_ecoproduct_struct **)calloc(
		    1,
		    opm_ptr->eco->total_products * sizeof(xsw_ecoproduct_struct *)
                );
		if(obj_ptr->eco->product == NULL)
		{
		    obj_ptr->eco->total_products = 0;
		}
		else
		{
		    obj_ptr->eco->total_products = opm_ptr->eco->total_products;

		    /* Copy each product. */
		    for(i = 0; i < opm_ptr->eco->total_products; i++)
		    {
		        if(opm_ptr->eco->product[i] == NULL)
			    continue;

			obj_ptr->eco->product[i] = (xsw_ecoproduct_struct *)MemoryCopyAlloc(
                            opm_ptr->eco->product[i],
			    sizeof(xsw_ecoproduct_struct)
			);
		    }
		}

            }
        }
        else
        {
            obj_ptr->eco = NULL;
        }


	return(0);
}


int OPMModelObject(int object_num, int opm_num)
{
        xsw_object_struct *obj_ptr;
        xsw_object_struct *opm_ptr;


        if(DBIsObjectGarbage(object_num)) 
            return(-1);
	else
	    obj_ptr = xsw_object[object_num];

	if(OPMIsGarbage(opm_num))
            return(-1);
        else
            opm_ptr = opm[opm_num];


	return(OPMModelObjectPtr(obj_ptr, opm_ptr));
}
