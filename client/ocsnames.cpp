/*
                         Object Create Script Names

	Functions:

	int OCSIsGarbage(int ocsn_num)
	void OCSReclaimMemory()
	void OCSDeleteAll()
	void OCSReset(int ocsn_num)
	int OCSCreate(int type)
	int OCSCreateExplicit(int ocsn_num, int type)
	void OCSRecycle(int ocsn_num)
	int OCSGetTop()
	int OCSGetByCode(int code)

	---

 */

#include "xsw.h"


/*
 *	Checks if OCSN is unallocated or garbage.
 */
int OCSIsGarbage(int ocsn_num)
{
        if((ocsn_num < 0) ||
           (ocsn_num >= total_ocsns) ||
           (ocsn == NULL)
        )
            return(-1);
        else if(ocsn[ocsn_num] == NULL)
            return(-1);
        else if(ocsn[ocsn_num]->type <= OCS_TYPE_GARBAGE)
            return(1);
        else
            return(0);
}


/*
 *	Free unused entries and reallocate pointers.
 */
void OCSReclaimMemory()
{
	int i, h;
	ocsn_struct **ocsn_ptr;


	/* Sanitize total. */
	if(total_ocsns < 0)
	    total_ocsns = 0;


        /* Get highest allocated ocsn. */
	for(i = 0, ocsn_ptr = ocsn, h = -1;
            i < total_ocsns;
            i++, ocsn_ptr++
        )
	{
	    if(*ocsn_ptr != NULL)
		h = i;
	}

	/* Adjust total and reallocate. */
	total_ocsns = h + 1;
	if(total_ocsns > 0)
	{
	    ocsn = (ocsn_struct **)realloc(
		ocsn,
		total_ocsns * sizeof(ocsn_struct *)
	    );
	    if(ocsn == NULL)
	    {
		total_ocsns = 0;
		return;
	    }
	}
	else
	{
	    total_ocsns = 0;

	    free(ocsn);
	    ocsn = NULL;
	}


	return;
}

/*
 *      Frees all allocated substructures of ocsn.
 */
void OCSReset(int ocsn_num)
{
        ocsn_struct *ocsn_ptr;


        if(OCSIsGarbage(ocsn_num))
            return; 
        else
            ocsn_ptr = ocsn[ocsn_num];

        /* Reset values. */
        ocsn_ptr->type = OCS_TYPE_GARBAGE;
        ocsn_ptr->name[0] = '\0';

        OSWDestroyImage(&ocsn_ptr->icon);

        return;
}

/*
 *	Procedure to delete all OCSNs.
 */
void OCSDeleteAll()
{
	int i;


#ifdef DEBUG_MEM_FREE
printf("OCSN: Deleting %i...\n", total_ocsns);
#endif

        for(i = 0; i < total_ocsns; i++)
        {
	    /* Free allocated substructures. */
            OCSReset(i);

#ifdef DEBUG_MEM_FREE
if(ocsn[i] != NULL)
    printf("OCSN %i: Free'ed.\n", i);
#endif
            free(ocsn[i]);
        }

#ifdef DEBUG_MEM_FREE
if(ocsn != NULL)
    printf("OCSN pointers: Free'ed.\n");
#endif
        free(ocsn);
        ocsn = NULL;

        total_ocsns = 0;


        return;
}

/*
 *	Allocates a new OCSN, returns the OCSN's number.
 */
int OCSCreate(int type)
{
        int i, n;
	ocsn_struct *ocsn_ptr, **ptr;


	/* Sanitize total. */
	if(total_ocsns < 0)
	    total_ocsns = 0;

        /* Type may not be garbage. */
        if(type <= OCS_TYPE_GARBAGE)
        {
            fprintf(stderr,
         "OCSCreate: Error: Useless request to create garbage.\n"
            );
            return(-1);
        }

        /* Look for available OCS already allocated. */
        for(i = 0, ptr = ocsn;
            i < total_ocsns;
            i++, ptr++
	)
        {
	    if(*ptr == NULL)
		break;

            if((*ptr)->type == OCS_TYPE_GARBAGE)
                break;
        }
        if(i < total_ocsns)
        {
	    n = i;
        }
	else
	{
	    n = total_ocsns;
	    total_ocsns++;

	    /* Allocate more pointers. */
            ocsn = (ocsn_struct **)realloc(
	        ocsn,
                total_ocsns * sizeof(ocsn_struct *)
	    );
            if(ocsn == NULL)
            {
	        total_ocsns = 0;
                return(-1);
            }   

	    /* Set new pointer to NULL so it gets allocated below. */
	    ocsn[n] = NULL;
	}

	/* Allocate new structure as needed. */
	if(ocsn[n] == NULL)
	{
	    ocsn[n] = (ocsn_struct *)calloc(
		1,
		sizeof(ocsn_struct)
	    );
	    if(ocsn[n] == NULL)
	    {
		return(-1);
	    }
	}

	/* Reset values. */
	ocsn_ptr = ocsn[n];

	OCSReset(n);
	ocsn_ptr->type = type;


        return(n);
}



/*
 *	Allocates an OCSN explicitly, returns non-zero on error.
 */
int OCSCreateExplicit(int ocsn_num, int type)
{
        int ocs_count, ocs_rtn;


        /* Type may not be garbage. */
        if(type <= OCS_TYPE_GARBAGE)
        {
            fprintf(stderr,
         "OCSCreateExplicit(): Error: Useless request to create garbage.\n"
            );
            return(-1);
        }

	/* Make sure ocsn_num is valid. */
	else if(ocsn_num < 0)
        {
            fprintf(stderr,
           "OCSCreateExplicit(): Error: Request to create ocsn_num #%i.\n",
		ocsn_num
            );
            return(-1);
	}


        /* If ocsn_num already exists, recycle it. */
        if(!OCSIsGarbage(ocsn_num))
            OCSRecycle(ocsn_num);

        /* Allocate memory if ocsn_num is greater than total_ocsns. */
        if(ocsn_num >= total_ocsns)
        {
            /* Can we allocate more? */
            if((ocsn_num + OBJECT_ALLOCATE_AHEAD) >= OCSN_MAX)
            {
                fprintf(stderr,
                    "OCSCreateExplicit: Maximum of %i OCSNs reached.\n",
		    OCSN_MAX
		);
                return(-1);
            }


            /* Allocate pointer array. */
            ocsn = (ocsn_struct **)realloc(
		ocsn,
		(ocsn_num + OBJECT_ALLOCATE_AHEAD + 1) * sizeof(ocsn_struct *)
            );
            if(ocsn == NULL)
            {
		total_ocsns = 0;
                return(-1);
            }


            /* Allocate each OCSN. */
            for(ocs_count = total_ocsns;
                ocs_count <= (ocsn_num + OBJECT_ALLOCATE_AHEAD);
                ocs_count++
            )
            {
                ocsn[ocs_count] = (ocsn_struct *)calloc(
		    1,
		    sizeof(ocsn_struct)
		);
                if(ocsn[ocs_count] == NULL)
                {
                    fprintf(stderr,
                        "OCS #%i: Memory allocation error.\n",
                        ocs_count
                    );
                    return(-1);
                }
            }

            /* Adjust global variable total_ocsns. */
            ocs_count = total_ocsns;
            total_ocsns = ocsn_num + OBJECT_ALLOCATE_AHEAD + 1;

            while(ocs_count < total_ocsns)
            {
                OCSReset(ocs_count);
                ocs_count++;
            }
        }


        /* Create explicit OCS. */
        ocs_rtn = ocsn_num;

        /* Set default values on the new OCS. */
        OCSReset(ocs_rtn);

        ocsn[ocs_rtn]->type = type;
        ocsn[ocs_rtn]->name[0] = '\0';


        return(0);
}



/*
 *	Recycles ocsn, freeing all substructures and resources.
 *
 *	Does not free ocsn entry itself.
 */
void OCSRecycle(int ocsn_num)
{
	/* Reset values and free allocated substructures. */
	OCSReset(ocsn_num);

	return;
}

/*
 *	Return highest allocated and non garbage ocsn number.
 */
int OCSGetTop()
{
        int i, t;
	ocsn_struct **ptr;

	for(i = 0, t = -1, ptr = ocsn;
            i < total_ocsns;
            i++, ptr++
	)
	{
	    if(*ptr == NULL)
		continue;

	    if((*ptr)->type <= OCS_TYPE_GARBAGE)
		continue;

	    t = i;
	}

	return(t);
}



/*
 *	Returns an OCSN number by code (aka type).
 *	The code cannot be OCS_TYPE_GARBAGE.
 *
 *	If the return is not -1, then it can be assumed valid.
 */
int OCSGetByCode(int code)
{
	int i;
	ocsn_struct **ocs_ptr;


	/* OCS code cannot be garbage. */
	if(code <= OCS_TYPE_GARBAGE)
            return(-1);


	/* Begin search.. */
	for(i = 0, ocs_ptr = ocsn;
	    i < total_ocsns;
	    i++, ocs_ptr++
	)
	{
	    if(*ocs_ptr == NULL)
		continue;

	    if((*ocs_ptr)->type == code)
		return(i);
	}


	return(-1);
}
