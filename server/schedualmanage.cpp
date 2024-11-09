/*
                        Schedual Management

	Functions:

	int SchedualHandle(int schedual_num)
	int SchedualManage()


	---

	Scheduals are used to maintain conditional actions
	that are currently running.  Often used for economy
	buy, sell, and trade de/incrementing.

 */

#include "../include/unvmatch.h"
#include "../include/unvmath.h"
#include "../include/unvutil.h"

#include "swserv.h"


/*
 *	Handles schedual, checking its condition and performing
 *	an action if the condition is met.
 */
int SchedualHandle(int schedual_num)
{
	int status;
	double dx;
	schedual_struct *shptr;

	long owner_obj_num, src_obj_num, tar_obj_num;
	xsw_object_struct *src_obj_ptr, *tar_obj_ptr;


	/* Get schedual pointer (assumed valid and active). */
	shptr = (schedual_struct *)schedual[schedual_num];


	/* Get heading values. */
	owner_obj_num = shptr->run_owner;
	src_obj_num = shptr->run_src_obj;
	tar_obj_num = shptr->run_tar_obj;


	/* ********************************************************* */
	/* Check condition. */

	status = 0;
	switch(shptr->cond_type)
	{
	  /* ******************************************************* */
	  case SCHE_COND_UNTILL_IN_RANGE:
	    break;

          /* ******************************************************* */
	  case SCHE_COND_WHILE_NOT_IN_RANGE:
	    break;

          /* ******************************************************* */
	  case SCHE_COND_WHILE_IN_RANGE:
	    /* Make sure the tar_obj_num is valid. */
            if(DBIsObjectGarbage(tar_obj_num))
		break;
	    else
                tar_obj_ptr = xsw_object[tar_obj_num];

	    /* Check if tar_obj_num is in same sector of condition. */
	    if(!Mu3DInSectorPtr(
		    tar_obj_ptr,
		    shptr->cond_sect_x,
                    shptr->cond_sect_y,
                    shptr->cond_sect_z
               )
	    )
		break;

	    /* Get distance in XSW Real units. */
	    dx = Mu3DDistance(
		shptr->cond_x - tar_obj_ptr->x,
                shptr->cond_y - tar_obj_ptr->y,
                shptr->cond_z - tar_obj_ptr->z
	    );
	    /* Check if out of range. */
	    if(dx > (shptr->cond_range) + ((double)tar_obj_ptr->size / 1000))
		break;

	    /* Checks passed, set status to be true. */
	    status = 1;
            break;

          /* ******************************************************* */
	  case SCHE_COND_ALWAYS_TRUE:
            /* Make sure the target object is valid. */
            if(DBIsObjectGarbage(tar_obj_num))
                break;
	    status = 1;
	    break;

          /* ******************************************************* */
	  default:
	    fprintf(stderr,
		"SchedualHandle(): Unsupported condition %i.\n",
		shptr->cond_type
	    );
	    break;
	}
	if(!status)
	{
	    /*   Condition check failed, set schedual's cond_type
	     *   to SCHE_COND_FALSE so it no longer runs.
	     */
	    shptr->cond_type = SCHE_COND_FALSE;

	    return(0);
	}



        /* ********************************************************* */
	/* Condition was met, perform action. */

	/* Get source object pointer. */
        if(DBIsObjectGarbage(src_obj_num))
            src_obj_ptr = NULL;
        else
            src_obj_ptr = xsw_object[src_obj_num]; 

	/* Get target object pointer. */
	if(DBIsObjectGarbage(tar_obj_num))
	    tar_obj_ptr = NULL;
	else
	    tar_obj_ptr = xsw_object[tar_obj_num];


	/* Perform action. */
	switch(shptr->act_type)
	{
	  /* ******************************************************** */
	  case SCHE_ACT_PRINTMESG:
	    break;

          /* ******************************************************** */
	  case SCHE_ACT_RESTOCK:
            /* Stop if tar_obj_ptr is NULL. */  
            if((src_obj_ptr == NULL) || (tar_obj_ptr == NULL))
            {
                shptr->cond_type = SCHE_COND_FALSE;
                break;
            }

	    /* Stop if increment is 0. */
	    if(shptr->act_inc == 0)
            {
                shptr->cond_type = SCHE_COND_FALSE;
                break;
            }

	    /* Switch what type of item to restock. */
	    switch(shptr->act_item_code)
	    {
	      /* ****************************************************** */
	      case SCHE_ITEM_ANTIMATTER:
		/* Decrement on source. */
/*
		src_obj_ptr->antimatter -= shptr->act_inc;
 */
		/* Increment on target. */
		tar_obj_ptr->antimatter += shptr->act_inc;
		/* Update schedual count. */
		shptr->act_inc_count += shptr->act_inc;

                /* Recycle schedual if completed. */
                if(shptr->act_inc_limit >= 0)
		{
 		    if(shptr->act_inc_count >= shptr->act_inc_limit)
			shptr->cond_type = SCHE_COND_FALSE;
		}
		else
		{
                    if(shptr->act_inc_count <= shptr->act_inc_limit)
                        shptr->cond_type = SCHE_COND_FALSE;
 		}

		/* Sanitize source and target values. */
/*
                if(src_obj_ptr->antimatter > src_obj_ptr->antimatter_max)
                    src_obj_ptr->antimatter = src_obj_ptr->antimatter_max;
		else if(src_obj_ptr->antimatter < 0)
		    src_obj_ptr->antimatter = 0;
 */

                if(tar_obj_ptr->antimatter > tar_obj_ptr->antimatter_max)
                    tar_obj_ptr->antimatter = tar_obj_ptr->antimatter_max;
                else if(tar_obj_ptr->antimatter < 0)
                    tar_obj_ptr->antimatter = 0;
		break;

              /* ****************************************************** */
              case SCHE_ITEM_CRYSTALS:


		break;

              /* ****************************************************** */
              case SCHE_ITEM_HULL:
                /* Decrement on source. */
/*
                src_obj_ptr->hp -= shptr->act_inc;
 */
                /* Increment on target. */
                tar_obj_ptr->hp += shptr->act_inc;
                /* Update schedual count. */
                shptr->act_inc_count += shptr->act_inc;

                /* Recycle schedual if completed. */
                if(shptr->act_inc_limit >= 0)
                {
                    if(shptr->act_inc_count >= shptr->act_inc_limit)
                        shptr->cond_type = SCHE_COND_FALSE;
                }
                else
                {
                    if(shptr->act_inc_count <= shptr->act_inc_limit)
                        shptr->cond_type = SCHE_COND_FALSE;
                }
                
                /* Sanitize source and target values. */
/*
                if(src_obj_ptr->hp > src_obj_ptr->hp_max)
                    src_obj_ptr->hp = src_obj_ptr->hp_max;
                else if(src_obj_ptr->hp < 0)   
                    src_obj_ptr->hp = 0;
 */
                if(tar_obj_ptr->hp > tar_obj_ptr->hp_max)
                    tar_obj_ptr->hp = tar_obj_ptr->hp_max;
                else if(tar_obj_ptr->hp < 0)
                    tar_obj_ptr->hp = 0;

		break;

              /* ****************************************************** */
              case SCHE_ITEM_RMU:
                /* Create scores as needed. */
		if(UNVAllocScores(src_obj_ptr))
                {
		    shptr->cond_type = SCHE_COND_FALSE;
                    break;
                }
                /* Decrement on source. */
                src_obj_ptr->score->rmu -= (xswo_rmu_t)shptr->act_inc;
                /* Increment on target. */
                tar_obj_ptr->score->rmu += (xswo_rmu_t)shptr->act_inc;
                /* Update schedual count. */
                shptr->act_inc_count += shptr->act_inc;

                /* Recycle schedual if completed. */
                if(shptr->act_inc_limit >= 0)
                {
                    if(shptr->act_inc_count >= shptr->act_inc_limit)
                        shptr->cond_type = SCHE_COND_FALSE;
                }
                else
                {
                    if(shptr->act_inc_count <= shptr->act_inc_limit)
                        shptr->cond_type = SCHE_COND_FALSE;
                }
                 
                /* Recycle schedual if completed. */
                if(shptr->act_inc_limit >= 0)
                {
                    if(shptr->act_inc_count >= shptr->act_inc_limit)
                        shptr->cond_type = SCHE_COND_FALSE;
                }
                else
                {
                    if(shptr->act_inc_count <= shptr->act_inc_limit)
                        shptr->cond_type = SCHE_COND_FALSE;
                }

                /* Sanitize on source and target. */
                if(src_obj_ptr->score->rmu > src_obj_ptr->score->rmu_max)
                    src_obj_ptr->score->rmu = src_obj_ptr->score->rmu_max;
                else if(src_obj_ptr->score->rmu < 0)
                    src_obj_ptr->score->rmu = 0;

                if(tar_obj_ptr->score->rmu > tar_obj_ptr->score->rmu_max)
                    tar_obj_ptr->score->rmu = tar_obj_ptr->score->rmu_max;
                else if(tar_obj_ptr->score->rmu < 0)
                    tar_obj_ptr->score->rmu = 0;
                break;

              /* ****************************************************** */
	      case SCHE_ITEM_CREDITS:
		break;

              /* ****************************************************** */
              case SCHE_ITEM_WEAPON1:
                break;

              /* ****************************************************** */
              case SCHE_ITEM_WEAPON2:
                break;

              /* ****************************************************** */
              case SCHE_ITEM_WEAPON3:
                break;

              /* ****************************************************** */
              case SCHE_ITEM_WEAPON4:
                break;

              /* ****************************************************** */
              case SCHE_ITEM_WEAPON5:
                break;

              /* ****************************************************** */
              case SCHE_ITEM_WEAPON6:
                break;

              /* ****************************************************** */
              case SCHE_ITEM_WEAPON7:
                break;

              /* ****************************************************** */
              case SCHE_ITEM_WEAPON8:
                break;

              /* ****************************************************** */
              case SCHE_ITEM_WEAPON9:
                break;

              /* ****************************************************** */
              case SCHE_ITEM_WEAPON10:
                break;


	      default:
		break;
	    }
	    break;

          /* ******************************************************** */
	  default:
            fprintf(stderr,
		"SchedualHandle(): Unsupported action %i.\n",
                shptr->act_type
            );

	    /* Set cond_type to SCHE_COND_FALSE so it no longer gets runned. */
            shptr->cond_type = SCHE_COND_FALSE;

	    break;
	}


	return(0);
}


/*
 *	Procedure to manage all scheduals.
 *
 *	Returns the number of scheduals handled.
 */
int SchedualManage()
{
	int schedual_num;
	schedual_struct *shptr;
	int scheduals_handled = 0;


	/* Begin maintaining scheduals. */
	for(schedual_num = 0;
            schedual_num < total_scheduals;
            schedual_num++
	)
	{
	    /* Skip unallocated or inactive scheduals. */
	    if(!SchedualIsActive(schedual_num))
		continue;
	    else
		shptr = (schedual_struct *)schedual[schedual_num];

	    /* Schedual due for condition check and action? */
	    if(shptr->act_next > cur_systime) continue;


	    /* ****************************************************** */

	    /* Handle schedual. */
	    SchedualHandle(schedual_num);

	    /* Set next processing time (in seconds). */
	    shptr->act_next = cur_systime + shptr->act_int;


	    /* If schedual has become false, recycle it. */
	    if(shptr->cond_type == SCHE_COND_FALSE)
	    {
		SchedualRecycle(schedual_num);
	    }
	    /* shptr from this point on may be invalid. */

	    /* Increment scheduals handled. */
	    scheduals_handled++;
	}


	return(scheduals_handled);
}
