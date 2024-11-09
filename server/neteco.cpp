/*
                        Economy network handling

	Functions:

	int NetHandleReqValues(int condescriptor, char *arg)

	int NetSendEcoSetValues(int condescriptor, int object_num)
	int NetSendEcoSetProductValues(
		int condescriptor,
		int object_num
		int product_num
	)

	int NetHandleEcoBuy(int condescriptor, char *arg)
	int NetHandleEcoSell(int condescriptor, char *arg)

	---

 */

#include "../include/unvmatch.h"
#include "../include/swnetcodes.h"

#include "swserv.h"
#include "net.h"


int NetHandleReqValues(int condescriptor, char *arg)
{
	int i, object_num;
	xsw_object_struct *obj_ptr;


        /*      
         *   SWEXTCMD_ECO_REQVALUES format:
         *
         *	object_num
         */
        sscanf(arg, "%i",
		&object_num
	);

	/* Is object valid? */
	if(DBIsObjectGarbage(object_num))
	    obj_ptr = NULL;
	else
	    obj_ptr = xsw_object[object_num];


	if((obj_ptr == NULL) ? 0 : (obj_ptr->eco != NULL))
	{
	    /* Send general eco info. */
	    NetSendEcoSetValues(condescriptor, object_num);

	    /* Send product info. */
	    for(i = 0; i < obj_ptr->eco->total_products; i++)
	    {
	        NetSendEcoSetProductValues(
		    condescriptor,
		    object_num,
		    i
		);
	    }

	    /* Send score for proprietor object. */
            NetSendScore(
                condescriptor,
                object_num
            );
	}

	/* Send score for connection's object. */
        NetSendScore(
            condescriptor,
            connection[condescriptor]->object_num
        );


	return(0);
}


int NetSendEcoSetValues(int condescriptor, int object_num)
{
        char sndbuf[CS_DATA_MAX_LEN];
        xsw_object_struct *obj_ptr;


        if(DBIsObjectGarbage(object_num))
            return(-1);
        else
            obj_ptr = xsw_object[object_num];

        /* Does object have economy data? */
        if(obj_ptr->eco == NULL)
            return(0);

        /*
         *      SWEXTCMD_ECO_SETVALUES format:
         *
         *	object_num flags tax_general tax_friend tax_hostile
         */
        sprintf(sndbuf,
"%i %i\
 %i %ld %.4f %.4f %.4f\n", 
                CS_CODE_EXT,
                SWEXTCMD_ECO_SETVALUES,

                object_num,
                obj_ptr->eco->flags,
                obj_ptr->eco->tax_general,
                obj_ptr->eco->tax_friend,
                obj_ptr->eco->tax_hostile
        );
        NetDoSend(condescriptor, sndbuf);


	return(0);
}


int NetSendEcoSetProductValues(
	int condescriptor,
	int object_num,
	int product_num
)
{
	char sndbuf[CS_DATA_MAX_LEN];
	xsw_object_struct *obj_ptr;


	if(DBIsObjectGarbage(object_num))
	    return(-1);
	else
	    obj_ptr = xsw_object[object_num];

	/* Does object have economy data? */
	if(obj_ptr->eco == NULL)
	    return(0);

	/* Is product_num valid? */
	if((product_num < 0) ||
           (product_num >= obj_ptr->eco->total_products)
	)
	    return(-1);
	if(obj_ptr->eco->product[product_num] == NULL)
	    return(-1);


        /*
         *      SWEXTCMD_ECO_SETPRODUCTVALUES format:    
         *
         *      object_num sell_price buy_price
         *      product_amount product_max;product_name
         */
        sprintf(sndbuf,
"%i %i\
 %i %.4f %.4f %.4f %.4f;%s\n",
		CS_CODE_EXT,
		SWEXTCMD_ECO_SETPRODUCTVALUES,

                object_num,
		obj_ptr->eco->product[product_num]->sell_price,
		obj_ptr->eco->product[product_num]->buy_price,
                obj_ptr->eco->product[product_num]->amount,
                obj_ptr->eco->product[product_num]->amount_max,
                obj_ptr->eco->product[product_num]->name
        );
        NetDoSend(condescriptor, sndbuf);


	return(0); 
}



int NetHandleEcoBuy(int condescriptor, char *arg)
{
	char *strptr;
	char larg[CS_DATA_MAX_LEN];
	int customer_obj, proprietor_obj;
	long magic_number;
	xsw_ecoproduct_struct product;


	strncpy(larg, arg, CS_DATA_MAX_LEN);
	larg[CS_DATA_MAX_LEN - 1] = '\0';


        /*      
         *      SWEXTCMD_ECO_BUY format:
         *
         *      customer_obj proprietor_obj
         *      amount magic_number;product_name
         */
	strptr = strchr(larg, CS_STRING_DELIMINATOR_CHAR);
	if(strptr == NULL)
	    return(-1);

	/* Clear product structure. */
	memset(&product, 0x00, sizeof(xsw_ecoproduct_struct));

	strncpy(product.name, strptr + 1, ECO_PRODUCT_NAME_MAX);
	product.name[ECO_PRODUCT_NAME_MAX - 1] = '\0';

	*strptr = '\0';

	sscanf(larg,
"%i %i %lf %ld",
		&customer_obj,
		&proprietor_obj,
		&product.amount,
		&magic_number
	);

	/* Magic number must match. */
	if(magic_number != ECO_EXCHANGE_MAGIC_NUMBER)
	    return(-3);

	/* Customer object must match that of connection. */
	if(customer_obj != connection[condescriptor]->object_num)
	    return(-1);


	/* Do buying. */
	EcoDoBuy(
	    condescriptor,
	    customer_obj,
	    proprietor_obj,
	    product
	);


	return(0);
}


int NetHandleEcoSell(int condescriptor, char *arg)
{
        char *strptr;
        char larg[CS_DATA_MAX_LEN];
        int customer_obj, proprietor_obj;
        long magic_number;
        xsw_ecoproduct_struct product;

        
        strncpy(larg, arg, CS_DATA_MAX_LEN);
        larg[CS_DATA_MAX_LEN - 1] = '\0';


        /*
         *      SWEXTCMD_ECO_SELL format:
         *
         *      customer_obj proprietor_obj
         *      amount magic_number;product_name
         */
        strptr = strchr(larg, CS_STRING_DELIMINATOR_CHAR);
        if(strptr == NULL)
            return(-1);

        /* Clear product structure. */
        memset(&product, 0x00, sizeof(xsw_ecoproduct_struct));

        strncpy(product.name, strptr + 1, ECO_PRODUCT_NAME_MAX);
        product.name[ECO_PRODUCT_NAME_MAX - 1] = '\0';

        *strptr = '\0';
        
        sscanf(larg,
"%i %i %lf %ld",
                &customer_obj,
                &proprietor_obj,
                &product.amount,
                &magic_number
        );
        
        /* Magic number must match. */
        if(magic_number != ECO_EXCHANGE_MAGIC_NUMBER)
            return(-3);

        /* Customer object must match that of connection. */
        if(customer_obj != connection[condescriptor]->object_num)
            return(-1);


        /* Do sell. */
        EcoDoSell(
            condescriptor,
            customer_obj,
            proprietor_obj,
            product
        );


	return(0);
}
