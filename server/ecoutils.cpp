/*
                         Economy Utilities

	Functions:

	int EcoGetOCSFromName(char *s)
	int EcoGetProductNumByName(int object_num, char *name)

        xswo_credits_t EcoTransCredits(
                int object_num,  
                xswo_credits_t d_credits
        )

	int EcoStartTransfer(
		int run_owner,
		int run_src_obj,
		int run_tar_obj,
		int act_type,
		int act_item_code,
		double act_inc,
                double act_inc_limit,
                time_t act_int
	)

	int EcoNotifyTransaction(
	        int condescriptor,
	        char *product_name,
	        char *proprietor_name,
	        double total_amount,
	        double total_price
	)

	int EcoAdjustPriceAuto(
		xsw_ecoproduct_struct *prod_ptr,
		double diff_amount
	)

	---

 */

#include "../include/unvmatch.h"
#include "../include/unvutil.h"
#include "../include/xsw_ctype.h"

#include "swserv.h"
#include "net.h"


/*
 *      Returns the next OCS from (start_ocs_num + 1) by parsing the
 *	string s.  start_ocs_num should be one less than the ocs
 *	number that you want to start searching for.
 *
 *      The string s must be of the format "$OPM:<opm_name>"
 *      where <opm_name> is the object parameter macro name.
 *
 *	The next OCS referancing that object parameter macro name
 *	will be returned or -1 when no more OCS can be found.
 *
 *      Can return -1 on error or failed match.
 */
int EcoGetOCSFromName(
        char *s,
	int start_ocs_num
)
{
        int i;
	char *strptr;


        if(s == NULL)
            return(-1);

	/* Skip leading spaces. */
	while(ISBLANK(*s))
	    s++;

        /* Is first character a '$'? */
        if(*s != '$')
            return(-1);

	/* Search for ':' delimiter character. */
	strptr = strchr(s, ':');
	if(strptr == NULL)
	    return(-1);
	s = strptr + 1;

	/* Parsed string must contain data. */
	if(*s == '\0')
	    return(-1);


	/* Increment start_ocs_num. */
	start_ocs_num++;
	if(start_ocs_num < 0)
	    start_ocs_num = 0;

	/*   Search for next OCS number referancing an OPM name
	 *   parsed in string s.
	 */
	for(i = start_ocs_num; i < total_ocss; i++)
	{
	    if(ocs[i] == NULL)
		continue;

	    if(ocs[i]->opm_name == NULL)
		continue;

	    if(!strcasecmp(s, ocs[i]->opm_name))
		return(i);
	}


	return(-1);
}


/*
 *	Returns the weapon number on the object matching the
 *	OPM name string name or -1 on no match.  If return is non-
 *	negative than the weapon index number is gauranteed valid.
 */
int EcoGetWeaponNumByName(
	xsw_object_struct *obj_ptr,
	char *name
)
{
	int i, n, w;


	if(obj_ptr == NULL)
	    return(-1);

	for(i = -1; i < total_ocss; i = n)
	{
	    /* Get next matched OCS. */
	    n = EcoGetOCSFromName(
		name,
		i
	    );
	    if(n < 0)
		break;

	    if(!OCSIsAllocated(n))
		continue;

	    /* Check if object has a weapon using OCS number n. */
	    for(w = 0; w < obj_ptr->total_weapons; w++)
	    {
		if(obj_ptr->weapons[w] == NULL)
		    continue;

		if(obj_ptr->weapons[w]->ocs_code == ocs[n]->code)
		    return(w);
	    }
	}


	return(-1);
}

        
/*
 *      Returns eco product index number of -1 on error of failed
 *      match.
 *      
 *      If a match is returned, it is comfermed to be allocated and
 *      valid.
 */
int EcoGetProductNumByName(
        int object_num,
        char *name
)
{           
        int i;
        xsw_object_struct *obj_ptr;
        xsw_ecodata_struct *eco_ptr;
        xsw_ecoproduct_struct **product;
        

        if(name == NULL)
            return(-1);

        if(DBIsObjectGarbage(object_num))
            return(-1);
        else
            obj_ptr = xsw_object[object_num];


        if(obj_ptr->eco == NULL)
            return(-1);
        else
            eco_ptr = obj_ptr->eco;


        for(i = 0, product = eco_ptr->product;
            i < eco_ptr->total_products;
            i++, product++
        )
        {
            if(*product == NULL)
                continue;

            if(!strcasecmp((*product)->name, name))
                return(i);
        }

        return(-1);
}


/*
 *      Add or subtract credits from object.
 *      
 *      Returns delta value that would not make negative or exceed
 *      maximum.
 */
xswo_credits_t EcoTransCredits(
        int object_num,  
        xswo_credits_t d_credits
)
{
        xsw_score_struct *score_ptr;
        xsw_object_struct *obj_ptr;


	if(DBIsObjectGarbage(object_num))
	    return(0);
	else
	    obj_ptr = xsw_object[object_num];


        /* Allocate scores as needed. */
        if(UNVAllocScores(obj_ptr))
            return(0);
	else
            score_ptr = obj_ptr->score;

        if((score_ptr->credits + d_credits) < 0)
            d_credits = score_ptr->credits * -1;
        if((score_ptr->credits + d_credits) > MAX_CREDITS)
            d_credits = MAX_CREDITS - score_ptr->credits; 


        score_ptr->credits += d_credits;


        return(d_credits);
} 


/*
 *	Allocates a schedual with the given information.
 *	Returns the allocated schedual number or -1 on error.
 *
 *	If at all possible: run_owner, run_src_obj, and run_tar_obj
 *	should NOT be garbage.
 */
int EcoStartTransfer(
	int run_owner,		/* The object that owns this transfer. */
	int run_src_obj,	/* Object that sends. */
	int run_tar_obj,	/* Object that recieves. */
	int act_type,		/* One of SCHE_ACT_*. */
	int act_item_code,	/* One of SCHE_ITEM_*. */
	double act_inc,
	double act_inc_limit,
	time_t act_int		/* In seconds. */
)
{
	int sh_num;
	schedual_struct *sh_ptr;
	xsw_object_struct *src_obj_ptr, *tar_obj_ptr;	


	/* Error checks. */
	if(DBIsObjectGarbage(run_src_obj))
	    return(-1);
	else
	    src_obj_ptr = xsw_object[run_src_obj];

        if(DBIsObjectGarbage(run_tar_obj))
            return(-1);
        else
            tar_obj_ptr = xsw_object[run_tar_obj];


	/* ************************************************************ */

	/* Allocate a schedual. */
	sh_num = SchedualAdd(SCHE_COND_WHILE_IN_RANGE);
	if(sh_num < 0)
	    return(-1);

	/* Get schedual pointer. */
	if(SchedualIsAllocated(sh_num))
	    sh_ptr = (schedual_struct *)schedual[sh_num];
	else
	    return(-1);


	/* Set up values. */
	sh_ptr->run_owner = run_owner;
	sh_ptr->run_src_obj = run_src_obj;
	sh_ptr->run_tar_obj = run_tar_obj;

	/* Coordinates relative to source object. */
	sh_ptr->cond_sect_x = src_obj_ptr->sect_x;
        sh_ptr->cond_sect_y = src_obj_ptr->sect_y;
        sh_ptr->cond_sect_z = src_obj_ptr->sect_z;

        sh_ptr->cond_x = src_obj_ptr->x;
        sh_ptr->cond_y = src_obj_ptr->y;
        sh_ptr->cond_z = src_obj_ptr->z;

	sh_ptr->cond_range = ECO_MAX_TRANSACTION_RANGE;	/* In real units. */

	sh_ptr->act_type = act_type;
	sh_ptr->act_item_code = act_item_code;

        sh_ptr->act_inc = act_inc;
	sh_ptr->act_inc_count = 0;
        sh_ptr->act_inc_limit = act_inc_limit;

        sh_ptr->act_int = act_int;
	sh_ptr->act_next = 0;



	return(sh_num);
}


/*
 *	Notifies economy transaction to the connection.
 */
int EcoNotifyTransaction(
	int condescriptor,	/* Who you want to notify. */
	char *product_name,
	char *proprietor_name,
	double total_amount,
	double total_price
)
{
	double item_price;
	char sndbuf[CS_DATA_MAX_LEN];


	/* Calculate item price */
	if(total_amount != 0)
	    item_price = total_price / total_amount;
        else
	    item_price = 0;

        /* Buying or selling? */
        if(total_price > 0)
	{
	    /* Buying from proprietor. */
	    sprintf(
		sndbuf,
		"Bought: %.2f of %s from %s for %.2f credits (%.2f/unit)",
                total_amount,
                product_name,
                proprietor_name,
                total_price,
                item_price
	    );
	}
        else if(total_price < 0)
	{
	    /* Selling to proprietor. */
	    sprintf(
		sndbuf,
		"Sold: %.2f of %s to %s for %.2f credits (%.2f/unit)",
		total_amount,
                product_name,
                proprietor_name,
                -total_price,
                -item_price
	    );
	}
	else
	{
	    /* No transaction */
            sprintf(
                sndbuf,
 "No trade of %s with %s (cargo holds full or not enough credits)",
		product_name,
		proprietor_name
	    );
	}

        NetSendLiveMessage(condescriptor, sndbuf);

        return(0);
}


/*
 *	This function will set prices based on current amount related to
 *	max_amount. It will first calculate a "maxprice" for this
 *	product by looking at th in relation to the  to the
 *	amount/maxamount ratio.
 *
 *	Then it will calculate a new price using this maxprice and the
 *	new amount.
 *
 *	This way the maxprice (which is the price when the amount is zero)
 *	(wil to reuse in the next transaction.
 *
 *	The prices is set specially when the amount is max or zero, to
 *	bypass without loosing the maxprice.
 *
 *	The sell price is used for calculations, the buy_price is
 *	calculated u
 *
 *	TODO: Add support for products which the properitar will not buy
 *	(sell
 */
int EcoAdjustPriceAuto(
	xsw_ecoproduct_struct *prod_ptr,
	double diff_amount	/* The diff between the old and new amount */
)
{
	double old_sell_price, old_buy_price, sell_price, max_price,
	       old_amount,new_amount,max_amount,buy_price;

	/* Is prod_ptr valid? */
	if(prod_ptr == NULL)
	    return(0);

	/* Is diff_amount valid? */
        if(diff_amount == 0)
            return(0);

        /* Get product values */
        old_sell_price = prod_ptr->sell_price;
        old_buy_price = prod_ptr->buy_price;
        old_amount = prod_ptr->amount;
        max_amount = prod_ptr->amount_max;

        /* No amount possible? */
        if(max_amount <= 0)
            return(0);

        /* Calculate new amount */
        new_amount = old_amount + diff_amount;

        /* Check if stock was empty or full */
        if(old_amount >= max_amount)
        {
            /* Full stock - restore maxprice */
            max_price = old_sell_price * 100;
        }
        else if (old_amount == 0)
        {
            /* Empty stock - restore maxprice */
            max_price = old_buy_price;
        }
        else
        {
            if (old_sell_price == 0)
            {
                old_sell_price = old_buy_price;
            }
            /* Calculate max price based on old price and amount ratio */
            max_price = (max_amount * old_sell_price) /
		(max_amount - old_amount);
	}       
       
        if(fabs(new_amount) >= max_amount)	// Dan S: used floating point version rather than cast.
        {
            /* Full stock - save maxprice */
            sell_price = max_price / 100;
            buy_price = 0;
        }
        else if(new_amount == 0)
        {
            /* Empty stock - save maxprice */
            sell_price = 0;
	    buy_price = max_price;
        }
	else
	{
	    /* Calculate new price based on max price and new amount */
	    sell_price = (max_amount - new_amount) *
		(max_price / max_amount);
	    buy_price = sell_price * 1.2;

	    if(old_sell_price == old_buy_price)
	    {
		buy_price = sell_price;
		sell_price = 0;
            }
	}

        /* Update product record */
        prod_ptr->sell_price = sell_price;
        prod_ptr->buy_price = buy_price;

	return(0);
}
