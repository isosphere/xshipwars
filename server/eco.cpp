/*
                                Economy

	Functions:


	int EcoDoBuy(
		int condescriptor,
	        int customer_obj,
	        int proprietor_obj,
	        xsw_ecoproduct_struct product
	)
	int EcoDoSell(
	        int condescriptor,
	        int customer_obj,
	        int proprietor_obj,
	        xsw_ecoproduct_struct product
	)

	---

 */

#include "../include/unvmatch.h"
#include "../include/unvmath.h"
#include "../include/unvutil.h"

#include "swserv.h"
#include "net.h"


#define MIN(a,b)	(((a) < (b)) ? (a) : (b))
#define MAX(a,b)	(((a) > (b)) ? (a) : (b))


/*
 *	Procedure to do a buy, customer buys from the proprietor.
 */
int EcoDoBuy(
	int condescriptor,	/* Connection of customer object. */
        int customer_obj,
	int proprietor_obj,
        xsw_ecoproduct_struct product
)
{
	int i, opm_num;
	int ep_num = -1;
	int schedual_act_item_code = -1;
	int need_passive_transfer = 0;
	double schedual_act_inc_limit = 0.0;

	xswo_credits_t d_credits = 0.0;
	double d_amount = 0.0;

	int iff_code;
	double tax;

	xsw_object_struct *customer_ptr, *proprietor_ptr;
	xsw_ecoproduct_struct *real_product_ptr;	/* Real product on proprietor. */
	xsw_ecoproduct_struct *customer_product_ptr;
	char text[ECO_PRODUCT_NAME_MAX + XSW_OBJ_NAME_MAX + 512];

	char sndbuf[CS_DATA_MAX_LEN];


	/* Priorietor object must be valid. */
	if(DBIsObjectGarbage(proprietor_obj))
	    return(0);
	else
	    proprietor_ptr = xsw_object[proprietor_obj];

        /* Customer object must be valid. */
        if(DBIsObjectGarbage(customer_obj))
            return(0);
        else
            customer_ptr = xsw_object[customer_obj];

	/* Is customer and proprietor the same? */
	if(customer_ptr == proprietor_ptr)
	{
            NetSendSysMessage(
                condescriptor,
                CS_SYSMESG_CODE_ERROR,
                "Customer and proprietor are the same."
            );
            return(-1);
	}

	/* Proprietor must have an economy and score structure. */
	if(proprietor_ptr->eco == NULL)
	    return(0);
	if(UNVAllocScores(proprietor_ptr))
	    return(0);

        if(UNVAllocScores(customer_ptr))
            return(0); 


	/* Check if customer and proprietor are in range. */
	if(!Mu3DInRange(
	    xsw_object, total_objects,
	    customer_obj, proprietor_obj, ECO_MAX_TRANSACTION_RANGE
	))
	{
	    NetSendSysMessage(
		condescriptor,      
		CS_SYSMESG_CODE_ERROR,
		"Proprietor object is too far away."
	    );
	    return(-1);
	}

	/*   Customer object is assumed already checked to be owned by
         *   the connetion.
         */

	/* Check if proprietor is OPENed for business. */
	if(!(proprietor_ptr->eco->flags & ECO_FLAG_OPEN))
	{
            NetSendSysMessage(
                condescriptor,
                CS_SYSMESG_CODE_ERROR,
                "Proprietor object is not opened for business."
            );
            return(-1);
	}
	/* Check if proprietor is willing to allow BUYing. */
	if(!(proprietor_ptr->eco->flags & ECO_FLAG_BUY_OK))
        {
            NetSendSysMessage(
                condescriptor,
                CS_SYSMESG_CODE_ERROR,
                "Proprietor object does not permit purchasing."
            );
            return(-1);
        }


	/* Product name cannot be empty. */
	if(product.name[0] == '\0')
	    return(0);

	/* Request product amount must be positive. */
	if(product.amount <= 0)
	    return(0);

	/* Get eco product number. */
	ep_num = EcoGetProductNumByName(proprietor_obj, product.name);
	if(ep_num < 0)
	    return(-1);
	else
	    real_product_ptr = proprietor_ptr->eco->product[ep_num];


	/* Check IFF for customer and proprietor to determine tax level. */
        iff_code = MatchIFFPtr(proprietor_ptr, customer_ptr);
        if(iff_code == IFF_FRIENDLY)
            tax = proprietor_ptr->eco->tax_friend;
        else if(iff_code == IFF_HOSTILE)
            tax = proprietor_ptr->eco->tax_hostile;
        else
	    tax = proprietor_ptr->eco->tax_general;


	/* ********************************************************** */
	/* Handle by product name. */

	/* Antimatter. */
	if(!strcasecmp(product.name, ECO_PROD_NAME_ANTIMATTER))
	{
	    /* Set schedual action item code. */
	    schedual_act_item_code = SCHE_ITEM_ANTIMATTER;

	    /* Get and sanitize delta amount (if limited). */
	    d_amount = 0.0;
	    if(real_product_ptr->amount_max >= 0)
	    {
		if((real_product_ptr->amount - product.amount) < 0)
		    d_amount = real_product_ptr->amount;
		else
		    d_amount = product.amount;
	    }
	    else
	    {
		d_amount = product.amount;
	    }

	    /* Sanitize amount for customer, how much can customer have. */
	    if((d_amount + customer_ptr->antimatter) >
		customer_ptr->antimatter_max
	    )
		d_amount = customer_ptr->antimatter_max -
		    customer_ptr->antimatter;

            /* Charge customer and get sanitize credits. */
            d_credits = EcoTransCredits(
                customer_obj,
                d_amount * real_product_ptr->buy_price *
                     proprietor_ptr->eco->tax_general * -1
            ) * -1;
            /* Give credits to proprietor. */
            EcoTransCredits(proprietor_obj, d_credits);

            /* Calculate sanitized amount based on credits available. */
            if((proprietor_ptr->eco->tax_general != 0) &&
               (real_product_ptr->buy_price != 0)
	    )
                d_amount = d_credits / (proprietor_ptr->eco->tax_general
                    * real_product_ptr->buy_price);

            /* Calculate increment limit. */  
            schedual_act_inc_limit = d_amount;

	    /* Need passive transfer of product to customer. */
	    need_passive_transfer = 1;
	}
        /* ********************************************************** */
        /* Hull repair. */
        else if(!strcasecmp(product.name, ECO_PROD_NAME_REPAIRHULL))
        {
            /* Set schedual action item code. */
            schedual_act_item_code = SCHE_ITEM_HULL;

            /* Get and sanitize delta amount (if limited). */
            if(real_product_ptr->amount_max >= 0)
            {
                if((real_product_ptr->amount - product.amount) < 0)
                    d_amount = real_product_ptr->amount;
                else
                    d_amount = product.amount;
            }
            else
            {
                d_amount = product.amount;
            }
            
            /* Sanitize amount for customer, how much can customer have. */
            if((d_amount + customer_ptr->hp) >
                customer_ptr->hp_max
            )
                d_amount = customer_ptr->hp_max -
                    customer_ptr->hp;

            /* Charge customer and get sanitize credits. */
            d_credits = EcoTransCredits(
                customer_obj,
                d_amount * real_product_ptr->buy_price *
                     proprietor_ptr->eco->tax_general * -1
            ) * -1;
            /* Give credits to proprietor. */
            EcoTransCredits(proprietor_obj, d_credits);

            /* Calculate sanitized amount based on credits available. */
            if((proprietor_ptr->eco->tax_general != 0) &&
               (real_product_ptr->buy_price != 0)
            )
                d_amount = d_credits / (proprietor_ptr->eco->tax_general
                    * real_product_ptr->buy_price);

            /* Calculate increment limit. */
            schedual_act_inc_limit = d_amount;


            /* Need passive transfer of product to customer. */
            need_passive_transfer = 1;
	}
        /* ********************************************************** */
        /* Raw material. */
        else if(!strcasecmp(product.name, ECO_PROD_NAME_RMU))
        {
            /* Set schedual action item code. */
            schedual_act_item_code = SCHE_ITEM_RMU;

            /* Get and sanitize delta amount (if limited). */
            if(real_product_ptr->amount_max >= 0)
            {
                if((real_product_ptr->amount - product.amount) < 0)
                    d_amount = real_product_ptr->amount;
                else
                    d_amount = product.amount;
            }
            else
            {
                d_amount = product.amount;
            }

            /* Sanitize amount for customer, how much can customer have. */
            if((d_amount + customer_ptr->score->rmu) >
                customer_ptr->score->rmu_max
            )
                d_amount = customer_ptr->score->rmu_max -
                    customer_ptr->score->rmu;
        
            /* Charge customer and get sanitize credits. */
            d_credits = EcoTransCredits(
                customer_obj,
                d_amount * real_product_ptr->buy_price *
                     proprietor_ptr->eco->tax_general * -1
            ) * -1;
            /* Give credits to proprietor. */
            EcoTransCredits(proprietor_obj, d_credits);
                
            /* Calculate sanitized amount based on credits available. */
            if((proprietor_ptr->eco->tax_general != 0) &&
               (real_product_ptr->buy_price != 0)
            )
                d_amount = d_credits / (proprietor_ptr->eco->tax_general
                    * real_product_ptr->buy_price);

            /* Calculate increment limit. */
            schedual_act_inc_limit = d_amount;


            /* Need passive transfer of product to customer. */
            need_passive_transfer = 1;
        }
	/* ********************************************************** */
	/* OPM player ships. */
	else if(strcasepfx(product.name, ECO_PROD_NAME_OPMPFX) &&
                !OPMIsGarbage(OPMGetByName(product.name,4))
	)
	{
	    xsw_object_struct *saveplayer_ptr;

	    /* Sanitize amount for customer */
            d_amount = product.amount;

            /* Any ship at all? */
            if(d_amount <= 0)
		return(0);

	    /* A player can never buy more than one ship. */
	    if((d_amount > 1.0) || (d_amount < 1.0))
		d_amount = 1.0;

	    /* Set action item code to 0 so it's just valid. */
	    schedual_act_item_code = 0;

	    /* Calculate total price */
	    d_credits = d_amount * real_product_ptr->buy_price
		* proprietor_ptr->eco->tax_general;

	    /* Check if player has enough credits for this ship. */
	    if(customer_ptr->score->credits < d_credits)
	    {
		NetSendSysMessage(
		    condescriptor,
		    CS_SYSMESG_CODE_ERROR,
 "Sorry but you do not have enough credits to buy this ship."
		);
		return(-1);
	    }

	    /* Set action item code to 0 so it's just valid. */
	    schedual_act_item_code = 0;
	    
	    /* Charge customer and get sanitize credits. */
 	    EcoTransCredits(customer_obj,-d_credits);

	    /* Give credits to proprietor. */
	    EcoTransCredits(proprietor_obj, d_credits);

	    /* Calculate increment limit. */
	    schedual_act_inc_limit = d_amount;

	    /* Need passive transfer of product to customer. */
/*	    need_passive_transfer = 1; */

	    /* Copy customer object for setting back values
	     * later.
	     */
	    saveplayer_ptr = UNVDupObject(customer_ptr);
	    if(saveplayer_ptr == NULL)
	    {
                NetSendSysMessage(
                    condescriptor,
                    CS_SYSMESG_CODE_ERROR,
                    "Memory allocation error."
                );
		return(-1);
	    }

	    /* Get OPM number for modelling. */
	    opm_num = OPMGetByName(product.name, XSW_OBJ_TYPE_PLAYER);
	    if(opm_num < 0)
	    {
                sprintf(sndbuf,
                    "%s: No such OPM.",
                    product.name
                );
                NetSendLiveMessage(condescriptor, sndbuf);

		/* Delete saved player object, no longer needed. */
		UNVDeleteObject(saveplayer_ptr);

 		return(-1);
	    }

	    /* Model customer object with OPM values, any current
	     * allocated substructures on the customer object will
	     * be deallocated and replaced by (if any) ones specified
	     * in the OPM.
             */
            if(OPMModelObject(customer_obj, opm_num))
            {
		/* Error occured when attempting to remodel
		 * object.
		 */
                sprintf(sndbuf,
                    "%s: Cannot remodel.",
                    DBGetFormalNameStr(customer_obj)
		);
	    }
	    else
            {
                sprintf(sndbuf,
                    "%s changed vessel to %s. (OPM #%i)",
                    saveplayer_ptr->name,
                    opm[opm_num]->name,
                    opm_num
                );

                /* Restore player values that would be changed by the
		 * modeling to the OPM's values.
		 */
                customer_ptr->owner = saveplayer_ptr->owner;
                customer_ptr->permission.uid = saveplayer_ptr->permission.uid;
                customer_ptr->permission.gid = saveplayer_ptr->permission.gid;
                customer_ptr->intercepting_object = saveplayer_ptr->intercepting_object;
                customer_ptr->locked_on = saveplayer_ptr->locked_on;
                customer_ptr->lighting = saveplayer_ptr->lighting;
                customer_ptr->shield_frequency = saveplayer_ptr->shield_frequency;
                customer_ptr->com_channel = saveplayer_ptr->com_channel;
                customer_ptr->sect_x = saveplayer_ptr->sect_x;
                customer_ptr->sect_y = saveplayer_ptr->sect_y;
                customer_ptr->sect_z = saveplayer_ptr->sect_z;
                customer_ptr->x = saveplayer_ptr->x;
                customer_ptr->y = saveplayer_ptr->y;
                customer_ptr->z = saveplayer_ptr->z;
                strncpy(
		    customer_ptr->empire,
                    saveplayer_ptr->empire,
                    XSW_OBJ_EMPIRE_MAX
                );
                customer_ptr->empire[XSW_OBJ_EMPIRE_MAX - 1] = '\0';
                strncpy(
                    customer_ptr->password,
                    saveplayer_ptr->password,
                    XSW_OBJ_PASSWORD_MAX
                );
                customer_ptr->password[XSW_OBJ_PASSWORD_MAX - 1] = '\0';
                strncpy(
                    customer_ptr->name,
                    saveplayer_ptr->name,
                    XSW_OBJ_NAME_MAX
                );
                customer_ptr->name[XSW_OBJ_NAME_MAX - 1] = '\0';

                /* Score structure if exists on saved object. */
                if(saveplayer_ptr->score != NULL)
		{
		    if(!UNVAllocScores(customer_ptr))
		    {
			memcpy(
			    customer_ptr->score,
			    saveplayer_ptr->score,  
                            sizeof(xsw_score_struct)
			);
		    }
		}
	    }
	    NetSendLiveMessage(condescriptor, sndbuf);

	    /* Delete saved object. */
	    UNVDeleteObject(saveplayer_ptr);

	    /* Notify customer. */
	    NetSendSysMessage(
		condescriptor,
		CS_SYSMESG_CODE_WARNING,
 "Congratulations, your new starship is waiting in the dock!"
	    );

            /* Update weapon values on customer. */
            for(i = 0; i <= customer_ptr->total_weapons; i++)
	    {
		NetSendWeaponValues(
		    -1,			/* All connections. */
		    customer_obj,
		    i                   /* Weapon number on object. */
		);
	    }
	}
	/* ********************************************************** */
	/* OPM Weapons. */
	else if(strcasepfx(product.name, ECO_PROD_NAME_OPMPFX) &&
	        !OPMIsGarbage(OPMGetByName(product.name,-1))
	)
	{
	    /* Weapons require whole numbered amounts. */
	    d_amount = floor(d_amount);

	    /* Match weapon on customer. */
	    i = EcoGetWeaponNumByName(customer_ptr, product.name);
	    if(i < 0)
		return(0);

	    /* Set action item code to 0 so it's just valid. */
            schedual_act_item_code = 0;


            /* Get and sanitize delta amount (if limited). */
            if(real_product_ptr->amount_max >= 0)
            {
                if((real_product_ptr->amount - product.amount) < 0)
                    d_amount = real_product_ptr->amount;
                else
                    d_amount = product.amount;
            }
            else
            {
                d_amount = product.amount;
            }
          
            /* Sanitize amount for customer, how much can customer have. */
            if((d_amount + customer_ptr->weapons[i]->amount) >
                customer_ptr->weapons[i]->max
            )
                d_amount = customer_ptr->weapons[i]->max -
                    customer_ptr->weapons[i]->amount;

            /* Charge customer and get sanitize credits. */
            d_credits = EcoTransCredits(
                customer_obj,
                d_amount * real_product_ptr->buy_price *
                     proprietor_ptr->eco->tax_general * -1
            ) * -1;
            /* Give credits to proprietor. */
            EcoTransCredits(proprietor_obj, d_credits);

            /* Calculate sanitized amount based on credits available. */
            if((proprietor_ptr->eco->tax_general != 0) &&
               (real_product_ptr->buy_price != 0)
            )
                d_amount = d_credits / (proprietor_ptr->eco->tax_general
                    * real_product_ptr->buy_price);

            /* Calculate increment limit. */
            schedual_act_inc_limit = d_amount;


	    /* Transfer weapon amount all at once to customer. */
	    customer_ptr->weapons[i]->amount += static_cast<int>(d_amount);


            /* Don't need passive transfer of product to customer. */
/*
            need_passive_transfer = 1;
*/

	    /* Update weapon value on customer. */
            NetSendWeaponValues(
		-1,			/* All connections. */
		customer_obj,
		i			/* Weapon number. */
	    );
	}
	/* ******************************************************** */
	/* Other products */
	else if((customer_ptr->eco != NULL) &&
	        (EcoGetProductNumByName(customer_obj, product.name) >= 0)
	)
	{
	    /* Check eco product number. */
	    ep_num = EcoGetProductNumByName(customer_obj, product.name);

	    customer_product_ptr = customer_ptr->eco->product[ep_num];

	    /* Set schedual action item code. */
	    schedual_act_item_code = 0;

	    /* Get and sanitize delta amount (if limited). */
	    if(real_product_ptr->amount_max >= 0)
	    {
		if((real_product_ptr->amount - product.amount) < 0)
		    d_amount = real_product_ptr->amount;
                else
                    d_amount = product.amount;
            }
            else
            {
                d_amount = product.amount;
            }

            /* Sanitize amount for customer, how much can customer have. */
            if((d_amount + customer_product_ptr->amount) >
               customer_product_ptr->amount_max
            )
		d_amount = customer_product_ptr->amount_max -
		    customer_product_ptr->amount;

	    /* Charge customer and get sanitize credits. */
	    d_credits = EcoTransCredits(
		customer_obj,
		d_amount * real_product_ptr->buy_price * tax * -1
	    ) * -1;

            /* Give credits to proprietor. */
            EcoTransCredits(proprietor_obj, d_credits);

            /* Calculate sanitized amount based on credits available. */
	    if((tax != 0) &&
	       (real_product_ptr->buy_price != 0)
	    )
		d_amount = d_credits / (tax * real_product_ptr->buy_price);

	    /* Calculate increment limit. */
            schedual_act_inc_limit = d_amount;

            /* Transfer products to customer */
            customer_product_ptr->amount += d_amount;

            /* Need passive transfer of product to customer. */
/*	    need_passive_transfer = 1; */
	}
        /* ********************************************************** */
	else
	{
            sprintf(
		text,
		"Your ship does not support cargo of type `%s'",
		product.name
            );
            NetSendSysMessage(
                condescriptor,
                CS_SYSMESG_CODE_ERROR,
                text
            );

	    schedual_act_item_code = -1;
	}


        /* ********************************************************** */

	/* Return if schedual_act_item_code is -1.  This implies that no
	 * valid product was found and handled above.
	 */
	if(schedual_act_item_code == -1)
	    return(-1);

        /* Adjust price */
        EcoAdjustPriceAuto(real_product_ptr, -d_amount);

	/* Decrement amount from proprietor instantly (for now). */
	if(real_product_ptr->amount_max >= 0)
	    real_product_ptr->amount -= d_amount;

	/* Update product values to connection. */
	NetSendEcoSetProductValues(
	    condescriptor,	/* Connection. */
	    proprietor_obj,	/* Proprietor object. */
	    ep_num		/* Eco product number. */
	);
        NetSendScore(condescriptor, proprietor_obj);                
        NetSendScore(condescriptor, customer_obj);

	EcoNotifyTransaction(
	    condescriptor,
	    product.name,
	    proprietor_ptr->name,
	    d_amount,
	    d_credits
	);


	/* Begin transfer. */
	if(need_passive_transfer)
	{
	    EcoStartTransfer(
	        customer_obj,
	        proprietor_obj,
	        customer_obj,
                SCHE_ACT_RESTOCK,
	        schedual_act_item_code,
	        ((schedual_act_inc_limit < 5) ?	/* Increment size. */
		    schedual_act_inc_limit : 5),
                schedual_act_inc_limit,
	        1			/* Interval in seconds. */
	    );
	}


	return(0);
}


/*
 *	Procedure to do a sell, customer sells to the proprietor.
 */
int EcoDoSell(
        int condescriptor,	/* Connection of customer object. */
        int customer_obj,
        int proprietor_obj,
        xsw_ecoproduct_struct product
)
{
        int ep_num = -1;
        int schedual_act_item_code = -1;
        double schedual_act_inc_limit;

        xswo_credits_t d_credits = 0.0;
        double d_amount = 0.0;

        int iff_code;
        double tax;

        xsw_object_struct *customer_ptr, *proprietor_ptr;
        xsw_ecoproduct_struct *real_product_ptr;        /* Real product on proprietor. */
	xsw_ecoproduct_struct *customer_product_ptr;

	char text[ECO_PRODUCT_NAME_MAX + XSW_OBJ_NAME_MAX + 512];


        /* Priorietor object must be valid. */
        if(DBIsObjectGarbage(proprietor_obj))
            return(0);
        else
            proprietor_ptr = xsw_object[proprietor_obj];

        /* Customer object must be valid. */
        if(DBIsObjectGarbage(customer_obj))
            return(0);
        else
            customer_ptr = xsw_object[customer_obj];

        /* Is customer and proprietor the same? */
        if(customer_ptr == proprietor_ptr)
        {
            NetSendSysMessage(
                condescriptor,
                CS_SYSMESG_CODE_ERROR,
                "Customer and proprietor are the same."
            );
            return(-1);
        }


        /* Proprietor have an economy and score structure. */
        if(proprietor_ptr->eco == NULL)
            return(0);
        if(UNVAllocScores(proprietor_ptr))
            return(0);

        if(UNVAllocScores(customer_ptr))
            return(0);


        /* Check if customer and proprietor are in range. */
        if(!Mu3DInRange(
	    xsw_object, total_objects,
	    customer_obj, proprietor_obj, ECO_MAX_TRANSACTION_RANGE
	))
        {
            NetSendSysMessage(
                condescriptor,
                CS_SYSMESG_CODE_ERROR,
                "Proprietor object is too far away."
            );
            return(-1);
        }

        /*   Customer object is assumed already checked to be owned by
         *   the connetion.
         */
              
        /* Check if proprietor is OPENed for business. */
        if(!(proprietor_ptr->eco->flags & ECO_FLAG_OPEN))
        {       
            NetSendSysMessage(
                condescriptor,
                CS_SYSMESG_CODE_ERROR,
                "Proprietor object is not opened for business."
            );
            return(-1);
        }
        /* Check if proprietor is willing to allow SELLing. */
        if(!(proprietor_ptr->eco->flags & ECO_FLAG_SELL_OK))
        {
            NetSendSysMessage(
                condescriptor,
                CS_SYSMESG_CODE_ERROR,
                "Proprietor object does not permit selling."
            );
            return(-1);
        }       

	/* Product name must not be empty. */
	if(product.name[0] == '\0')  
	    return(0);

	/* Request product amount must be positive. */
	if(product.amount <= 0)
	    return(0);

        /* Get eco product number. */
        ep_num = EcoGetProductNumByName(proprietor_obj, product.name);
        if(ep_num < 0)
            return(-1);
        else
            real_product_ptr = proprietor_ptr->eco->product[ep_num];

        /* Check IFF for customer and proprietor to determine tax level. */
        iff_code = MatchIFFPtr(proprietor_ptr, customer_ptr);
        if(iff_code == IFF_FRIENDLY)
            tax = proprietor_ptr->eco->tax_friend;
        else if(iff_code == IFF_HOSTILE)
            tax = proprietor_ptr->eco->tax_hostile;
        else
            tax = proprietor_ptr->eco->tax_general;


        /* ********************************************************** */
        /* Handle by product name. */

        /* Antimatter. */
        if(!strcasecmp(product.name, ECO_PROD_NAME_ANTIMATTER))
        {
            /* Set schedual action item code. */
            schedual_act_item_code = SCHE_ITEM_ANTIMATTER;

            /* Get and sanitize available delta product amount. */
            /* Protect the player from selling his last drop of AM. */
	    if((customer_ptr->antimatter - product.amount) < 10)
                d_amount = customer_ptr->antimatter - 10;
	    else
		d_amount = product.amount;

            /* Sanitize amount for proprietor, how much can proprietor have. */
	    if(real_product_ptr->amount_max >= 0)
	    {
                if((d_amount + real_product_ptr->amount) >
                    real_product_ptr->amount_max
                )
                    d_amount = real_product_ptr->amount_max -
                        real_product_ptr->amount;
	    }

            /* Charge proprietor and get sanitize credits. */
            d_credits = EcoTransCredits(
                proprietor_obj,
                d_amount * real_product_ptr->sell_price /
                     MAX(proprietor_ptr->eco->tax_general, 1) * -1
            ) * -1;
	    /* Give credits to customer. */
            EcoTransCredits(customer_obj, d_credits);

            /* Calculate sanitized amount based on credits available. */
            if((proprietor_ptr->eco->tax_general != 0) &&
               (real_product_ptr->sell_price != 0)
	    )
                d_amount = d_credits / (proprietor_ptr->eco->tax_general
                    * real_product_ptr->sell_price);

            /* Calculate increment limit. */
            schedual_act_inc_limit = d_amount;


            /* Transfer amount all at once from the customer. */
            customer_ptr->antimatter -= d_amount;


            /* Don't need passive transfer of product to customer. */
/*
            need_passive_transfer = 1;
*/
        }
        /* ********************************************************** */
        /* Raw material (RMU). */
        else if(!strcasecmp(product.name, ECO_PROD_NAME_RMU))
        {
            /* Set schedual action item code. */
            schedual_act_item_code = SCHE_ITEM_RMU;

            /* Get and sanitize available delta product amount. */
            if((customer_ptr->score->rmu - product.amount) < 0)
                d_amount = customer_ptr->score->rmu;
            else
                d_amount = product.amount;

            /* Sanitize amount for proprietor, how much can proprietor have. */
            if(real_product_ptr->amount_max >= 0)
            {
                if((d_amount + real_product_ptr->amount) >
                    real_product_ptr->amount_max
                )
                    d_amount = real_product_ptr->amount_max -
                        real_product_ptr->amount;
            }
  
            /* Charge proprietor and get sanitize credits. */
            d_credits = EcoTransCredits(
                proprietor_obj,
                d_amount * real_product_ptr->sell_price /
                     MAX(proprietor_ptr->eco->tax_general, 1) * -1
            ) * -1;
            /* Give credits to customer. */
            EcoTransCredits(customer_obj, d_credits);

            /* Calculate sanitized amount based on credits available. */
            if((proprietor_ptr->eco->tax_general != 0) &&
               (real_product_ptr->sell_price != 0)
            )   
                d_amount = d_credits / (proprietor_ptr->eco->tax_general
                    * real_product_ptr->sell_price);

            /* Calculate increment limit. */
            schedual_act_inc_limit = d_amount;


            /* Transfer amount all at once from the customer. */
            customer_ptr->score->rmu -= d_amount;


            /* Don't need passive transfer of product to customer. */
/*
            need_passive_transfer = 1;
*/
        }
	/* ********************************************************* */
	/* Other products */
	else if((customer_ptr->eco != NULL) &&
	        (EcoGetProductNumByName(customer_obj, product.name) >= 0)
	)
	{
	    /* Check eco product number. */
	    ep_num = EcoGetProductNumByName(customer_obj, product.name);

	    customer_product_ptr = customer_ptr->eco->product[ep_num];
            /* Set schedual action item code. */
            schedual_act_item_code = 0;

            /* Get and sanitize available delta amount. */
            if((customer_product_ptr->amount - product.amount) < 0)   
                d_amount = customer_product_ptr->amount;
            else
                d_amount = product.amount;

	    /* Sanitize amount for proprietor, how much can proprietor have. */
	    if(real_product_ptr->amount_max >= 0)
            {
                if((d_amount + real_product_ptr->amount) >
                    real_product_ptr->amount_max
                )
                    d_amount = real_product_ptr->amount_max -
                        real_product_ptr->amount;
            }

            /* Charge proprietor and get sanitize credits. */
	    d_credits = EcoTransCredits(
                proprietor_obj,
                d_amount * (real_product_ptr->sell_price *
                (2 - tax) ) * -1
            ) * -1;

            /* Give credits to customer. */
            EcoTransCredits(customer_obj, d_credits);

	    /* Calculate sanitized amount based on credits available. */
            if((tax != 0) &&
               (real_product_ptr->sell_price != 0)
            )       
                d_amount = d_credits /
                    (real_product_ptr->sell_price *
                    (2 - tax));

            /* Calculate increment limit. */
            schedual_act_inc_limit = d_amount;

            /* Transfer amount all at once from the customer. */
            customer_product_ptr->amount -= d_amount;

	    /* Don't need passive transfer of product to customer. */  
/*
	    need_passive_transfer = 1;
 */
	}
        /* ********************************************************** */
        else    
        {
	    sprintf(
		text,
		"No such product `%s' available on %s to sell.",
		product.name,
		proprietor_ptr->name
	    );
	    NetSendSysMessage(
                condescriptor,
		CS_SYSMESG_CODE_ERROR,
                text
	    );

            schedual_act_item_code = -1;
        }
            
            
        /* ********************************************************** */
        
        /* Return if schedual_act_item_code is -1.  This implies that no
         * valid product was found and handled above.
         */
        if(schedual_act_item_code == -1)
            return(-1);

	/* Adjust price */
	EcoAdjustPriceAuto(real_product_ptr,d_amount);

        /* Increment amount to proprietor instantly (for now). */
        real_product_ptr->amount += d_amount;

        /* Update product values to connection. */
        NetSendEcoSetProductValues(
            condescriptor,      /* Connection. */
            proprietor_obj,     /* Proprietor object. */
            ep_num              /* Eco product number. */
        );
        NetSendScore(condescriptor, proprietor_obj);
        NetSendScore(condescriptor, customer_obj);

	EcoNotifyTransaction(
	    condescriptor,
	    product.name,
	    proprietor_ptr->name,
	    d_amount,
	    -d_credits
	);


        /* Begin transfer. */
/* Selling is all instant.  We don't need this.
        if(need_passive_transfer)
        {
            EcoStartTransfer(
                customer_obj,
		customer_obj,
                proprietor_obj,
                SCHE_ACT_RESTOCK,
                schedual_act_item_code,
                ((schedual_act_inc_limit < 5) ?	Incremenet size.
                    schedual_act_inc_limit : 5),
                schedual_act_inc_limit,
                1                        Interval in seconds.
            );
        }
*/


	return(0);
}
