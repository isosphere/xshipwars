#include "../include/unvmatch.h"
#include "../include/unvutil.h"
#include "swserv.h"
#include "net.h"


static int CMDECO_PERMISSION_CHECK(int condescriptor, int permission);


/*
 *	Checks if connection matches or passes the given permission
 *	level.
 */
static int CMDECO_PERMISSION_CHECK(int condescriptor, int permission)
{
	int object_num;
	xsw_object_struct *obj_ptr;


	/* Connection must be logged in. */
	if(!ConIsLoggedIn(condescriptor))
	    return(0);

	/* Get connection's object. */
	object_num = connection[condescriptor]->object_num;
	if(DBIsObjectGarbage(object_num))
	    return(0);
	else
	    obj_ptr = xsw_object[object_num];

	/* Check object's permission. */
	if(obj_ptr->permission.uid <= permission)
	    return(1);
	else
	    return(0);
}



/*
 *	Creates an eco product on an object.
 *
 *	Syntax: "<object>=<product_name>"
 */
int CmdEcoProductCreate(int condescriptor, const char *arg)
{
	int i, status;
	char *strptr;

        char name[XSW_OBJ_NAME_MAX];
	char product_name[ECO_PRODUCT_NAME_MAX];
        char sndbuf[CS_DATA_MAX_LEN];
	char text[1024];
        char tmp_name1[XSW_OBJ_NAME_MAX];
        char tmp_name2[XSW_OBJ_NAME_MAX];

 	int object_num;
	xsw_object_struct *obj_ptr;
	connection_struct *con_ptr;
	xsw_ecodata_struct *eco_ptr;


        /* Get pointer to connection. */
        con_ptr = connection[condescriptor];

        /* Parse object name. */
        strncpy(name, arg, XSW_OBJ_NAME_MAX);
        name[XSW_OBJ_NAME_MAX - 1] = '\0';   
	strptr = strchr(name, '=');
	if(strptr == NULL)
	{
            sprintf(
		sndbuf,
 "Usage: `ecoprodcreate <object>=<product_name>'"
            );
            NetSendLiveMessage(condescriptor, sndbuf);
            return(-1);
	}
	else
	{
	    *strptr = '\0';
	}

	/* Parse product name. */
	strptr = (char *) strchr(arg, '=');
	if(strptr == NULL)
	{
	    return(-1);
	}
	else
	{
	    strncpy(
		product_name,
		strptr + 1,
		ECO_PRODUCT_NAME_MAX
	    );
	    product_name[ECO_PRODUCT_NAME_MAX - 1] = '\0';
	}


        StringStripSpaces(name);
	StringStripSpaces(product_name);


	/* Product name must be specified. */
	if(*product_name == '\0')
	    return(-1);


        /* Match object. */
	if(!strcasecmp(name, "me"))
	    object_num = con_ptr->object_num;
        else
            object_num = MatchObjectByName(
		xsw_object, total_objects,
		name, -1
	    );


        /* Make sure object_num is valid. */
        if(DBIsObjectGarbage(object_num))   
        {
            sprintf(
		sndbuf,
		"%s: No such object.",
                name
            );
            NetSendLiveMessage(condescriptor, sndbuf);
            return(-1);
        }
	else
	{
	    obj_ptr = xsw_object[object_num];
	}

	/* Permission check. */
	if(obj_ptr->owner != con_ptr->object_num)
	{
	    if(!CMDECO_PERMISSION_CHECK(condescriptor, ACCESS_UID_SETO))
	    {
                sprintf(
		    sndbuf,
		    "ecoprodcreate: You do not own that object."
                );
                NetSendLiveMessage(condescriptor, sndbuf);
                return(-1);
	    }
	}
	else
	{
            if(!CMDECO_PERMISSION_CHECK(condescriptor, ACCESS_UID_SET))
            {
                sprintf(
		    sndbuf,
   "ecoprodcreate: Permission denied: Requires access level %i.",
                    ACCESS_UID_SET
                );
                NetSendLiveMessage(condescriptor, sndbuf);
		return(-1);
            }
	}


        /* Allocate main economy structure on object as needed. */
        if(UNVAllocEco(obj_ptr))
        {
            sprintf(
		sndbuf,
		"%s: Memory allocation error.",
                name
            );
            NetSendLiveMessage(condescriptor, sndbuf);

            return(-1);
        }

	eco_ptr = obj_ptr->eco;
	if(eco_ptr == NULL)
	    return(-1);


        /* ******************************************************** */
	/* Check if product already exists. */
	for(i = 0; i < eco_ptr->total_products; i++)
	{
	    if(eco_ptr->product[i] == NULL)
		continue;

	    if(!strcasecmp(eco_ptr->product[i]->name, product_name))
	    {
                sprintf(
		    sndbuf,
                    "Product `%s' already exists on %s.",
                    product_name, name
                );
                NetSendLiveMessage(condescriptor, sndbuf);

                return(-2);
	    }
	}


	/* ******************************************************** */
	/* Create new product. */
	status = DBCreateObjectEconomyProduct(
            object_num,
            product_name,
            1.0,		/* Sell price. */
            1.0,                /* Buy price. */
            1.0,                /* Amount. */
            1.0			/* Amount max. */
	);
	if(status)
	{
            sprintf(
		sndbuf,
		"%s: Cannot create product.",
		product_name
            );
            NetSendLiveMessage(condescriptor, sndbuf);
            
            return(-1);
        }
	else
	{
	    /* Print response and log. */

	    strncpy(tmp_name1, DBGetFormalNameStr(object_num), XSW_OBJ_NAME_MAX);
	    tmp_name1[XSW_OBJ_NAME_MAX - 1] = '\0';
            strncpy(
		tmp_name2,
		DBGetFormalNameStr(con_ptr->object_num),
		XSW_OBJ_NAME_MAX
	    );
            tmp_name2[XSW_OBJ_NAME_MAX - 1] = '\0';

            sprintf(
		sndbuf,
		"Created product %s on %s.",
                product_name, tmp_name1
            );
            NetSendLiveMessage(condescriptor, sndbuf);

            sprintf(text,
 "%s: Created economy product %s on %s.",
		tmp_name2,
		product_name,
		tmp_name1
            );
            if(sysparm.log_general)
                LogAppendLineFormatted(fname.primary_log, text);
	}


        /* Sort products. */
        DBSortEconomyProducts(object_num);


        return(0);
}


/*
 *	Sets parameter of product on object.
 *
 *	Syntax: "<object>=<product_name>=<parm>:<value>"
 */
int CmdEcoProductSet(int condescriptor, const char *arg)
{
        int i;
        char *strptr;

        char name[XSW_OBJ_NAME_MAX];
        char product_name[ECO_PRODUCT_NAME_MAX];

	char parm[CS_DATA_MAX_LEN];
	char val[CS_DATA_MAX_LEN];

        char sndbuf[CS_DATA_MAX_LEN];
        char text[1024];
        char tmp_name1[XSW_OBJ_NAME_MAX];
        char tmp_name2[XSW_OBJ_NAME_MAX];

	int object_num;
        xsw_object_struct *obj_ptr;
	connection_struct *con_ptr;
        xsw_ecodata_struct *eco_ptr;
	xsw_ecoproduct_struct *product_ptr;


        /* Get pointer to connection. */
        con_ptr = connection[condescriptor];

        /* Parse object name. */
        strncpy(name, arg, XSW_OBJ_NAME_MAX);
        name[XSW_OBJ_NAME_MAX - 1] = '\0';
        strptr = strchr(name, '=');
        if(strptr == NULL)
        {   
            sprintf(
		sndbuf,
 "Usage: `ecoprodset <object>=<product_name>=<parm>:<value>'"
            );
            NetSendLiveMessage(condescriptor, sndbuf);
            return(-1);
        }
        else
        {
            *strptr = '\0';
        }

        /* Parse product name. */
        strptr = (char *) strchr(arg, '=');
        if(strptr == NULL)
        {
            return(-1);
        }
        else
        {
            strncpy(
                product_name,
                strptr + 1,
                ECO_PRODUCT_NAME_MAX
            );
            product_name[ECO_PRODUCT_NAME_MAX - 1] = '\0';
        }
	strptr = strchr(product_name, '=');
	if(strptr != NULL)
	    *strptr = '\0';

	/* Get parameter. */
	strptr = (char *) strchr(arg, '=');
	if(strptr == NULL)
	    return(-1);
	strptr = strchr(strptr + 1, '=');	/* Second '='. */
        if(strptr == NULL)
        {
            sprintf(
		sndbuf,
 "Usage: `ecoprodset <object>=<product_name>=<parm>:<value>'"
            );
            NetSendLiveMessage(condescriptor, sndbuf);
            return(-1);
        }   

	strncpy(parm, strptr + 1, CS_DATA_MAX_LEN);
	strptr = strchr(parm, ':');
	if(strptr == NULL)
        {
            sprintf(
		sndbuf,
 "Usage: `ecoprodset <object>=<product_name>=<parm>:<value>'"
            );
            NetSendLiveMessage(condescriptor, sndbuf);
            return(-1);
        }

	*strptr = '\0';
	strncpy(val, strptr + 1, CS_DATA_MAX_LEN);


        StringStripSpaces(name);
        StringStripSpaces(product_name);  
        StringStripSpaces(parm);
        StringStripSpaces(val);


        /* Product name must be valid. */
        if(*product_name == '\0')
            return(-1);


        /* Match object. */
        if(!strcasecmp(name, "me"))
            object_num = con_ptr->object_num;
        else
            object_num = MatchObjectByName(
		xsw_object, total_objects,
		name, -1
	    );

        /* Make sure object_num is valid. */
        if(DBIsObjectGarbage(object_num))
        {
            sprintf(
		sndbuf,
		"%s: No such object.",
                name   
            );
            NetSendLiveMessage(condescriptor, sndbuf);
            return(-1);
        }
        else
        {
            obj_ptr = xsw_object[object_num];
        }

        /* Permission check. */
        if(obj_ptr->owner != con_ptr->object_num)
        {
            if(!CMDECO_PERMISSION_CHECK(condescriptor, ACCESS_UID_SETO))
            {
                sprintf(
		    sndbuf,
		    "ecoprodset: You do not own that object."
                );
                NetSendLiveMessage(condescriptor, sndbuf);
                return(-1);
            }
        }
        else  
        {
            if(!CMDECO_PERMISSION_CHECK(condescriptor, ACCESS_UID_SET))
            {
                sprintf(
		    sndbuf,
   "ecoprodset: Permission denied: Requires access level %i.",
                    ACCESS_UID_SET
                );
                NetSendLiveMessage(condescriptor, sndbuf);
                return(-1);
            }
        }


	/* Get pointer to economy structure on object. */
        eco_ptr = obj_ptr->eco;
        if(eco_ptr == NULL)
            return(-1);

        /* Get pointer to product. */
	product_ptr = NULL;
        for(i = 0; i < eco_ptr->total_products; i++)
        {
            if(eco_ptr->product[i] == NULL)
                continue;

            if(!strcasecmp(eco_ptr->product[i]->name, product_name))
            {
		product_ptr = eco_ptr->product[i];
		break;
            } 
        }
	if(product_ptr == NULL)
	{
            sprintf(
		sndbuf,
 "ecoprodset: %s: No such product on %s.",
		product_name,
		name
            );
            NetSendLiveMessage(condescriptor, sndbuf);
            return(-1);
	}


	/* ****************************************************** */

	/* Name. */
	if(!strcmp(parm, "name"))
	{
	    strncpy(
		product_ptr->name,
		val,
		ECO_PRODUCT_NAME_MAX
	    );
	    product_ptr->name[ECO_PRODUCT_NAME_MAX - 1] = '\0';
	}
	/* Buy price. */
        else if(!strcmp(parm, "buy_price"))
        {
	    product_ptr->buy_price = atof(val);
        }
        /* Sell price. */
        else if(!strcmp(parm, "sell_price"))
        {
            product_ptr->sell_price = atof(val);
        }
        /* Amount. */
        else if(!strcmp(parm, "amount"))
        {
            product_ptr->amount = atof(val);
        }
        /* Amount max. */
        else if(!strcmp(parm, "amount_max"))
        {
            product_ptr->amount_max = atof(val);
        }
 

        /* ****************************************************** */
	/* Print response and log. */

        strncpy(tmp_name1, DBGetFormalNameStr(object_num), XSW_OBJ_NAME_MAX);
        tmp_name1[XSW_OBJ_NAME_MAX - 1] = '\0';
        strncpy(
            tmp_name2,
            DBGetFormalNameStr(con_ptr->object_num),
            XSW_OBJ_NAME_MAX
        );
        tmp_name2[XSW_OBJ_NAME_MAX - 1] = '\0';

        sprintf(
	    sndbuf,
	    "ecoprodset: %s: Set product `%s' %s to value `%s'.",
            tmp_name1, product_name, parm, val
        );
        NetSendLiveMessage(condescriptor, sndbuf);

        sprintf(text,
 "%s: Set economy product `%s' %s to `%s' on %s.",
            tmp_name2,
            product_name,
            parm,
            val,
            tmp_name1
        );
        if(sysparm.log_general)
            LogAppendLineFormatted(fname.primary_log, text);


        /* Sort products. */
        DBSortEconomyProducts(object_num);


        return(0);
}


/*
 *	Deletes a product entry on an object.
 *
 *	Syntax: "<object>=<product_name>"
 */
int CmdEcoProductDelete(int condescriptor, const char *arg)
{
        int i;
        char *strptr;

        char name[XSW_OBJ_NAME_MAX];
        char product_name[ECO_PRODUCT_NAME_MAX];
        char sndbuf[CS_DATA_MAX_LEN];

        char text[1024];
        char tmp_name1[XSW_OBJ_NAME_MAX];
        char tmp_name2[XSW_OBJ_NAME_MAX];
                
	int object_num;
        xsw_object_struct *obj_ptr;
	connection_struct *con_ptr;
        xsw_ecodata_struct *eco_ptr;


	/* Get pointer to connection. */
	con_ptr = connection[condescriptor];

	/* Parse object name. */
        strncpy(name, arg, XSW_OBJ_NAME_MAX);
        name[XSW_OBJ_NAME_MAX - 1] = '\0';
        strptr = strchr(name, '=');
        if(strptr == NULL)
        {
            sprintf(
		sndbuf,
 "Usage: `ecoproddelete <object>=<product_name>'"
            );
            NetSendLiveMessage(condescriptor, sndbuf);
            return(-1);
        }
        else
        {   
            *strptr = '\0';
        }

        /* Parse product name. */
        strptr = (char *) strchr(arg, '=');
        if(strptr == NULL)
        {
            return(-1);
        }
        else
        {
            strncpy(
                product_name,
                strptr + 1,
                ECO_PRODUCT_NAME_MAX
            );
            product_name[ECO_PRODUCT_NAME_MAX - 1] = '\0';
        }
 
   
        StringStripSpaces(name);
        StringStripSpaces(product_name);  
 
        
        /* Product name must be valid. */
        if(*product_name == '\0')
            return(-1);

        /* Match object. */
        if(!strcasecmp(name, "me"))
            object_num = con_ptr->object_num;
        else
            object_num = MatchObjectByName(
		xsw_object, total_objects,
		name, -1
	    );
        
         
        /* Make sure object_num is valid. */
        if(DBIsObjectGarbage(object_num))
        {
            sprintf(
		sndbuf,
		"%s: No such object.",
                name
            );
            NetSendLiveMessage(condescriptor, sndbuf);
            return(-1);
        }
        else
        {
            obj_ptr = xsw_object[object_num];
        }   


        /* Permission check. */
        if(obj_ptr->owner != con_ptr->object_num)
        {
            if(!CMDECO_PERMISSION_CHECK(condescriptor, ACCESS_UID_SETO))
            {
                sprintf(
		    sndbuf,
		    "ecoproddelete: You do not own that object."
                );
                NetSendLiveMessage(condescriptor, sndbuf);
                return(-1);
            }
        }   
        else
        {
            if(!CMDECO_PERMISSION_CHECK(condescriptor, ACCESS_UID_SET))
            {
                sprintf(
		    sndbuf,
  "ecoproddelete: Permission denied: Requires access level %i.",
                    ACCESS_UID_SET
                );
                NetSendLiveMessage(condescriptor, sndbuf);
                return(-1);
            }
        }

	/* Get pointer to eco structure. */
        eco_ptr = obj_ptr->eco;
        if(eco_ptr == NULL)
            return(-1);


	/* ******************************************************* */
	/* Search for product. */
        for(i = 0; i < eco_ptr->total_products; i++)
        {
            if(eco_ptr->product[i] == NULL)   
                continue;
        
            if(!strcasecmp(eco_ptr->product[i]->name, product_name))
            {
		/* Delete product. */
		free(eco_ptr->product[i]);
		eco_ptr->product[i] = NULL;

		/* Print response and log. */
                strncpy(tmp_name1, DBGetFormalNameStr(object_num), XSW_OBJ_NAME_MAX);
                tmp_name1[XSW_OBJ_NAME_MAX - 1] = '\0';
                strncpy(
                    tmp_name2,
                    DBGetFormalNameStr(con_ptr->object_num),
                    XSW_OBJ_NAME_MAX
                );
                tmp_name2[XSW_OBJ_NAME_MAX - 1] = '\0';

                sprintf(
		    sndbuf,
		    "Deleted product %s on %s.",
                    product_name, tmp_name1
                );
                NetSendLiveMessage(condescriptor, sndbuf);

                sprintf(text,
 "%s: Deleted economy product %s on %s.",
                    tmp_name2,
                    product_name,
                    tmp_name1
                );
                if(sysparm.log_general)
                    LogAppendLineFormatted(fname.primary_log, text);
            } 
        }

	/* If all product pointers are NULL, delete products. */
        for(i = 0; i < eco_ptr->total_products; i++)
        {
            if(eco_ptr->product[i] != NULL)
                break;
	}
	if(i >= eco_ptr->total_products)
	{
	    free(eco_ptr->product);
	    eco_ptr->product = NULL;

	    eco_ptr->total_products = 0;
	}


	/* Sort products. */
	DBSortEconomyProducts(object_num);

         
        return(0);
}
