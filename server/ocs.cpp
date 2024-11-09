/*
                        Object Create Scripts

	Functions:

	int OCSIsAllocated(int ocs_num)
	void OCSReclaim()
	void OCSDeleteAll()
	int OCSReset(int ocs_num)
	int OCSIsGarbage(int ocs_num)
	int OCSCreate(int ocs_code)
	int OCSCreateExplocit(int ocs_num, int ocs_code)
	void OCSRecycle(int ocs_num)
	int OCSGetTop()
	int OCSGetByCode(int ocs_code)

	---


 */

#include "swserv.h"



int OCSIsAllocated(int ocs_num)
{
	if((ocs_num < 0) ||
           (ocs_num >= total_ocss) ||
           (ocs == NULL)
	)
	    return(0);
	else if(ocs[ocs_num] == NULL)
	    return(0);
	else
	    return(1);
}


void OCSReclaim()
{
	int ocs_num;
        int top_ocs;


        /* Are there ocss allocated? */
        if(total_ocss <= 0)
            return;
	else if(ocs == NULL)
	    return;


        /* Get topmost ocs. */
        top_ocs = OCSGetTop();
	if((top_ocs < 0) || (top_ocs >= total_ocss) )
            return;


        /* Do we need to reclaim memory? */
        if((total_ocss - top_ocs - 1) <= OBJECT_ALLOCATE_AHEAD)
            return;
 

        /* Free all allocation from total_ocss - 1 to top_ocs + 1. */
        for(ocs_num = total_ocss - 1;
            ocs_num > top_ocs;
            ocs_num--
        )
        {
	    OCSReset(ocs_num);

#ifdef DEBUG_MEM_FREE
if(ocs[ocs_num] != NULL)
    printf("OCS %i: Free'ed.\n", ocs_num);
#endif
            free(ocs[ocs_num]);
            ocs[ocs_num] = NULL;
        }


        /* Adjust global variable total_ocss. */
        total_ocss = top_ocs + 1;


        /* Reallocate ocs pointers. */
        ocs = (ocs_struct **)realloc(
	    ocs,
	    total_ocss * sizeof(ocs_struct *)
        );
        if(ocs == NULL)
        {
	    total_ocss = 0;
            return;
        }


        return;
}



/*
 *	Procedure to delete all OCSs.
 */
void OCSDeleteAll()
{
        int i;


#ifdef DEBUG_MEM_FREE
printf("OCSs: Deleting %i...\n", total_ocss);
#endif

        for(i = 0; i < total_ocss; i++)
	{
	    if(ocs[i] == NULL) continue;

#ifdef DEBUG_MEM_FREE
if(ocs[i] != NULL)
    printf("OCS %i: Free'ed.\n", i);
#endif
            free(ocs[i]);
	    ocs[i] = NULL;
        }
#ifdef DEBUG_MEM_FREE
if(ocs != NULL)
    printf("OCS: Free'ed pointers.\n");
#endif
        free(ocs);
        ocs = NULL;

        total_ocss = 0;


        return;
}


/*
 *	Reset values.
 */
int OCSReset(int ocs_num)
{
	ocs_struct *ocs_ptr;


        /* Make sure ocs_num is allocated. */
	if(OCSIsAllocated(ocs_num))
	    ocs_ptr = ocs[ocs_num];
	else
	    return(-1);


        /* Reset values. */
	ocs_ptr->code = OCS_TYPE_GARBAGE;
        ocs_ptr->opm_name[0] = '\0';

	ocs_ptr->coppies = 0;

	ocs_ptr->heading = 0;
        ocs_ptr->pitch = 0;
        ocs_ptr->bank = 0;

        ocs_ptr->radius = 0;


        return(0);
}



int OCSIsGarbage(int ocs_num)
{
        if((ocs_num < 0) ||
	   (ocs_num >= total_ocss) ||
	   (ocs == NULL)
	)
        {
            return(-1);
        }
        else if(ocs[ocs_num] == NULL)
        {
            return(-1);
        }
        else if(ocs[ocs_num]->code <= OCS_TYPE_GARBAGE)
        {
            return(1);
        }

        return(0);
}



int OCSCreate(int ocs_code)
{
        int ocs_num, ocs_rtn;


        /* ocs_code may not be garbage. */
        if(ocs_code <= OCS_TYPE_GARBAGE)
        {
            fprintf(stderr,
                "OCSCreate: Error: Useless request to create garbage.\n"
            );
            return(-1);
        }


        /* *********************************************************** */

        /* Look for available OCS already allocated. */
        ocs_rtn = -1;
        for(ocs_num = 0; ocs_num < total_ocss; ocs_num++)
        {
            /* Look for garbage OCSs. */
            if(ocs[ocs_num]->code == OCS_TYPE_GARBAGE)
            {
                ocs_rtn = ocs_num;
                break;
            }
        }

        /* Did we find an OCS already allocated? */
        if( (ocs_rtn > -1) && (ocs_rtn < total_ocss) )
        {
            OCSReset(ocs_rtn);

            ocs[ocs_rtn]->code = ocs_code;
	    ocs[ocs_rtn]->opm_name[0] = '\0';

            return(ocs_rtn);
        }


        /* Not enough memory, can we allocate more? */
        if( (total_ocss + OBJECT_ALLOCATE_AHEAD) > MAX_OCSS)
        {
            fprintf(stderr, "OCS maximum %i reached.\n", MAX_OCSS);
            return(-1);
        }


        /* Allocate more memory. */

        /* Allocate pointer array. */
        ocs = (ocs_struct **)
            realloc(ocs, (total_ocss + OBJECT_ALLOCATE_AHEAD) *
                sizeof(ocs_struct *) );
        if(ocs == NULL)
        {
            fprintf(stderr,
                "OCSCreate(): Error: Unable to realloc() OCS pointers.\n"
            );
            return(-1);
        }   


        /* Allocate each ocs. */
        for(
            ocs_num = total_ocss;
            ocs_num < (total_ocss + OBJECT_ALLOCATE_AHEAD);
            ocs_num++
        )
        {
            ocs[ocs_num] = (ocs_struct *)
                calloc(1, sizeof(ocs_struct));

            /* Allocation error? */
            if(ocs[ocs_num] == NULL)
            {
                fprintf(stderr,
                    "OCS #%i: Memory allocation error.\n",
                    ocs_num
                );
                return(-1);
            }
        }


        /* New object will be the first OCS allocated. */
        ocs_rtn = total_ocss;


        /* Adjust global variable total_ocss. */
	ocs_num = total_ocss;
        total_ocss += OBJECT_ALLOCATE_AHEAD;

	/* Reset values on newly allocated objects. */
	while(ocs_num < total_ocss)
	{
	    OCSReset(ocs_num);
	    ocs_num++;
	}


        /* Set default values on the new OCS. */
        ocs[ocs_rtn]->code = ocs_code;
	ocs[ocs_rtn]->opm_name[0] = '\0';


        /* Return the number of the newly created OCS. */
        return(ocs_rtn);
}



int OCSCreateExplicit(int ocs_num, int ocs_code)
{
        int ocs_count, ocs_rtn;


        /* ********************************************************** */
        /* ocs_code may not be garbage. */
        if(ocs_code <= OCS_TYPE_GARBAGE)
        {
            fprintf(stderr,
         "OCSCreateExplicit(): Error: Useless request to create garbage.\n"
            );
            return(-1);
        }

	/* Make sure ocs_num is valid. */
	else if(ocs_num < 0)
        {
            fprintf(stderr,
           "OCSCreateExplicit(): Error: Request to create ocs_num #%i.\n",
		ocs_num
            );
            return(-1);
	}
        
         
        /* *********************************************************** */
                
        /* If object_num already exists, recycle it. */
        if(!OCSIsGarbage(ocs_num))
        {
            OCSRecycle(ocs_num);
        }


        /* Allocate memory if ocs_num is greater than total_ocss. */
        if(ocs_num >= total_ocss)
        {
            /* Can we allocate more? */
            if((ocs_num + OBJECT_ALLOCATE_AHEAD) >= MAX_OCSS)
            {
                fprintf(stderr, "OCS maximum %i reached.\n", MAX_OCSS);
                return(-1);
            }


            /* Allocate pointer array. */
            ocs = (ocs_struct **)realloc(
		ocs,
                (ocs_num + OBJECT_ALLOCATE_AHEAD + 1) *
                sizeof(ocs_struct *)
            );
            if(ocs == NULL)
            {
		total_ocss = 0;
		return(-1);
            }


            /* Allocate each OCS. */
            for(ocs_count = total_ocss;
                ocs_count <= (ocs_num + OBJECT_ALLOCATE_AHEAD);
                ocs_count++
            )
            {
                ocs[ocs_count] = (ocs_struct *)calloc(
		    1,
		    sizeof(ocs_struct)
		);
                if(ocs[ocs_count] == NULL)
                {
		    total_ocss = ocs_count;
                    return(-1);
                }
            }
                
            /* Adjust global variable total_ocss. */
            ocs_count = total_ocss;
            total_ocss = ocs_num + OBJECT_ALLOCATE_AHEAD + 1;

            while(ocs_count < total_ocss)
            {
                OCSReset(ocs_count);
                ocs_count++;
            }
        }
             
                
        /* Create explicit OCS. */
        ocs_rtn = ocs_num;

        /* Set default values on the new OCS. */
        OCSReset(ocs_rtn);

        ocs[ocs_rtn]->code = ocs_code;
        ocs[ocs_rtn]->opm_name[0] = '\0';


        /* Return 0 on no error, explicit OCS create does not
         * want OCS number.
         */
        return(0);
}



void OCSRecycle(int ocs_num)
{
	/* Is ocs_num valid? */
	if(OCSIsGarbage(ocs_num))
	    return;


	/* Reset values. */
	ocs[ocs_num]->code = OCS_TYPE_GARBAGE;
	ocs[ocs_num]->opm_name[0] = '\0';


	return;
}



int OCSGetTop()
{
        int current_ocs, last_valid_ocs;


        /* No OCSs allocated? */
        if(total_ocss <= 0)
            return(-1);
	else if(ocs == NULL)
	{
	    fprintf(stderr,
                "OCSGetTop(): Error: OCS pointers are NULL.\n"
	    );
	    return(-1);
	}

	/* Get last valid OCS. */
	last_valid_ocs = -1;
	for(current_ocs = 0; current_ocs < total_ocss;
	   current_ocs++)
	{
	    if(ocs[current_ocs] == NULL)
	    {
		fprintf(stderr,
		    "OCSGetTop(): Error: OCS #%i not allocated.\n",
		    current_ocs
		);
		continue;
	    }

	    if(ocs[current_ocs]->code <= OCS_TYPE_GARBAGE)
		continue;

	    last_valid_ocs = current_ocs;
	}


	return(last_valid_ocs);
}



int OCSGetByCode(int ocs_code)
{
        int ocs_count, ocs_found, found_count;


	/* Make sure ocss pointers are initialized. */
	if((ocs == NULL) || (total_ocss <= 0))
	{
	    fprintf(stderr,
		"OCSGetByCode(): OCS pointers not initialized.\n"
	    );
	    return(-1);
	}

	/* Make sure ocs_code is valid. */
	if(ocs_code == OCS_TYPE_GARBAGE)
	{
            fprintf(stderr,
                "OCSGetByCode(): Useless request to match code garbage.\n"
            );
            return(-1);
	}


	/* ************************************************************* */

	/* Begin searching for an OCS. */
	for(ocs_count = 0, ocs_found = -1, found_count = 0;
	    ocs_count < total_ocss;
	    ocs_count++
	)
	{
	    /* Skip garbage OCSs. */
	    if(OCSIsGarbage(ocs_count))
		continue;

	    if(ocs[ocs_count]->code == ocs_code)
	    {
                ocs_found = ocs_count;
                found_count++;
                continue;
            }
	}


	/* Return -2 if more than one match was found. */
	if(found_count > 1)
	    return(-2);



	return(ocs_found);
}
