/*
                            Scheduals

	Functions:

	int SchedualIsAllocated(int schedual_num)
	int SchedualIsActive(int schedual_num)

	void SchedualDelete(int schedual_num)
	void SchedualDeleteAll()

	void SchedualReset(int schedual_num) 
	int SchedualAdd(int cond_type)
	void SchedualRecycle(int schedual_num)
	int SchedualReclaim()

	---

	Scheduals are used to maintain conditional actions
	that are currently running.  Often used for economy
	buy, sell, and trade de/incrementing.

 */

#include "swserv.h"


#define SCHEDUAL_ALLOCATE_AHEAD		4


/*
 *	Checks if schedual structure schedual_num is allocated.
 */
int SchedualIsAllocated(int schedual_num)
{
	if((schedual == NULL) ||
           (schedual_num < 0) ||
           (schedual_num >= total_scheduals)
	)
	    return(0);
        else if(schedual[schedual_num] == NULL)
            return(0);
	else
	    return(1);
}


/*
 *	Checks if schedual schedual_num is allocated and
 *	active.
 *
 *	A schedual is active when it is allocated and its
 *	cond_type is not SCHE_COND_FALSE.
 */
int SchedualIsActive(int schedual_num)
{
	if(!SchedualIsAllocated(schedual_num))
	    return(0);
	else if(schedual[schedual_num]->cond_type == SCHE_COND_FALSE)
	    return(0);
	else
	    return(1);
}


/*
 *	Frees schedual structure schedual_num and all its resources.
 */
void SchedualDelete(int schedual_num)
{
	if(!SchedualIsAllocated(schedual_num)) return;


#ifdef DEBUG_MEM_FREE
if(schedual[schedual_num] != NULL)
    printf("Schedual %i: Free'ed.\n", schedual_num);
#endif

	free(schedual[schedual_num]);
	schedual[schedual_num] = NULL;

	return;
}


/*
 *	Procedure to delete all scheduals.
 */
void SchedualDeleteAll()
{
	int i;


#ifdef DEBUG_MEM_FREE
printf("Scheduals: Deleting %i...\n", total_scheduals);
#endif

	for(i = 0; i < total_scheduals; i++)
	{
	    if(schedual[i] == NULL) continue;

/*
#ifdef DEBUG_MEM_FREE
if(schedual[i] != NULL)
    printf("Schedual %i: Free'ed.\n", i);
#endif
*/
	    SchedualDelete(i);
	}

#ifdef DEBUG_MEM_FREE
if(schedual != NULL)
    printf("Schedual pointers: Free'ed.\n");
#endif
	free(schedual);
	schedual = NULL;

	total_scheduals = 0;


	return;
}


/*
 *	Resets the schedual to default values and setting
 *	its cond_type to SCHE_COND_FALSE.
 */
void SchedualReset(int schedual_num)
{
	schedual_struct *shptr;


	if(SchedualIsAllocated(schedual_num))
	    shptr = (schedual_struct *)schedual[schedual_num];
	else
	    return;


	/* Reset schedual values. */
        shptr->run_src_obj = -1;  
        shptr->run_tar_obj = -1;
        shptr->run_owner = -1;

	shptr->cond_type = SCHE_COND_FALSE;
        shptr->cond_sect_x = 0;
        shptr->cond_sect_y = 0;
        shptr->cond_sect_z = 0;
        shptr->cond_x = 0;
        shptr->cond_y = 0;
        shptr->cond_z = 0;
        shptr->cond_range = 1;

        shptr->act_type = SCHE_ACT_NONE;
        shptr->act_item_code = SCHE_ITEM_NONE;
        shptr->act_inc = 0;
	shptr->act_inc_count = 0;
        shptr->act_inc_limit = 0;
        shptr->act_int = 1;		/* In seconds. */
        shptr->act_next = cur_systime + shptr->act_int;
					/* In seconds. */

	return;
}


/*
 *	Allocates a new schedual and sets its condition to
 *	cond_type.  Returns the newly allocated schedual
 *	structure number or -1 on error.
 */
int SchedualAdd(int cond_type)
{
	int i, prev_total, schedual_num;


	/* Given cond_type cannot be SCHE_COND_FALSE. */
	if(cond_type == SCHE_COND_FALSE)
	    return(-1);

	/* Sanitize global total_scheduals. */
	if(total_scheduals < 0)
            total_scheduals = 0;


	/* ********************************************************** */

	/* Check for already allocated scheduals that are available. */
	for(i = 0; i < total_scheduals; i++)
	{
	    if(schedual[i] == NULL) break;
	    if(schedual[i]->cond_type == SCHE_COND_FALSE) break;
	}

	/* Did we find any available pointers? */
	if(i < total_scheduals)
	{
	    /* Found available pointer. */
	    schedual_num = i;

	    if(schedual[schedual_num] == NULL)
	    {
                schedual[schedual_num] = (schedual_struct *)calloc(1,
                    sizeof(schedual_struct)
                );
                if(schedual[schedual_num] == NULL)
                    return(-1);
	    }

	    SchedualReset(schedual_num);
	}
	else
	{
	    /* No available pointers, allocate more. */
	    schedual_num = total_scheduals;
	    prev_total = total_scheduals;

	    /* Adjust global variable total_scheduals. */
	    total_scheduals += SCHEDUAL_ALLOCATE_AHEAD;

	    /* Reallocate schedual pointers. */
	    schedual = (schedual_struct **)realloc(
		schedual,
		total_scheduals * sizeof(schedual_struct *)
	    );
	    if(schedual == NULL)
	    {
		total_scheduals = 0;
		return(-1);
	    }

	    /* Allocate more entries. */
	    for(i = prev_total; i < total_scheduals; i++)
	    {
		schedual[i] = (schedual_struct *)calloc(1,
		    sizeof(schedual_struct)
		);
                if(schedual[i] == NULL)
                {   
		    total_scheduals = i;
		    return(-1);
                }

		/* Reset schedual values to defaults. */
		SchedualReset(i);
	    }
	}

	/* ****************************************************** */

        /* Set new condition on highest_schedual. */
        schedual[schedual_num]->cond_type = cond_type;


	return(schedual_num);
}



/*
 *	Does same thing as SchedualReset().
 */
void SchedualRecycle(int schedual_num)
{
	SchedualReset(schedual_num);

	return;
}


/*
 *	Frees unsed schedual structures and pointers.
 *
 *	Returns -1 on error.
 */
int SchedualReclaim()
{
        int i, highest_schedual;


	/* No scheduals to reclaim? */
	if(schedual == NULL) return(0);


	/* Get highest_schedual that is allocated. */
	highest_schedual = -1;
	for(i = 0; i < total_scheduals; i++)
	{
	    if(schedual[i] == NULL) continue;
	    if(schedual[i]->cond_type == SCHE_COND_FALSE) continue;

	    highest_schedual = i;
	}

	/* Free scheduals from highest + 1 to total. */
	for(i = highest_schedual + 1; i < total_scheduals; i++)
	{
	    SchedualDelete(i);
	}

	/* Adjust global variable total_scheduals. */
	total_scheduals = highest_schedual + 1;


	/* Adjust pointers as needed. */
	if(total_scheduals > 0)
	{
	    /* Reallocate pointers. */
	    schedual = (schedual_struct **)realloc(
		schedual,
	        total_scheduals * sizeof(schedual_struct *)
	    );
	    if(schedual == NULL)
	    {
	        total_scheduals = 0;
	        return(-1);
	    }
	}
	else
	{
	    free(schedual);
	    schedual = NULL;

	    total_scheduals = 0;
	}


	return(0);
}
