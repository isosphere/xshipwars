// unvedit/ecow.cpp

#include "ecow.h"

/*
 *      Wepon window schedualed to be deleted.
 */


/*
                              Economy Window

	Functions:

	void EcoWStringToUpper(char *s)

	int EcoWGetLastSelected(ecpw_struct *ecow_ptr)
	xsw_object_struct *EcoWGetObjectPtr(
		ecow_struct *ecow_ptr
	)

	int EcoWIsAllocated(int n)
	int EcoWAllocate()
	void EcoWDelete(int n)
	void EcoWDeleteAll()

	int EcoWGetAllValues(ecow_struct *ecow_ptr)
	int EcoWGetValues(ecow_struct *ecow_ptr, int prod_num)
	int EcoWSetValues(ecow_struct *ecow_ptr, int prod_num)

	int EcoWListCB(void *ptr)

	int EcoWCreateCB(void *ptr)
	int EcoWDeleteCB(void *ptr)
	int EcoWDeleteEcoCB(void *ptr)
	int EcoWCloseCB(void *ptr)

	int EcoWInit(int n, void *src_ptr)
	int EcoWDraw(int n, int amount)
	int EcoWManage(int n, event_t *event)
	int EcoWManageAll(event_t *event)
	void EcoWMap(int n)
	void EcoWDoMapValues(int n, void *src, int obj_num)
	void EcoWUnmap(int n)
	void EcoWDestroy(int n)

	---

*/
/*
#include <stdio.h>
#include <db.h>
#include <malloc.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
*/
#include "../include/string.h"
#include "../include/strexp.h"

#include "../include/osw-x.h"
#include "../include/widget.h"

#include "../include/reality.h"
#include "../include/objects.h"
#include "../include/unvutil.h"
#include "../include/eco.h"

#include "uew.h"
#include "ue.h"
#include "ecow.h"

#ifndef MAX
#define MIN(a,b)        (((a) < (b)) ? (a) : (b))
#define MAX(a,b)	(((a) > (b)) ? (a) : (b))
#endif

/* #define MIN(a,b)	((a) < (b) ? (a) : (b)) */
/* #define MAX(a,b)	((a) > (b) ? (a) : (b)) */

namespace static_ecow {
	static ecow_struct *delete_ecow;
}


/*
 *      Wepon window schedualed to be deleted.
 */
/*
static ecow_struct *delete_ecow;


void EcoWStringToUpper(char *s);

void EcoWDoPromptFocus(
        prompt_window_struct *src_prompt,
        prompt_window_struct *tar_prompt
);
void EcoWRefocusPrompts(
        ecow_struct *ecow_ptr,
        prompt_window_struct *prompt
);
*/


/*
 *	Makes all characters in string s upper case.
 */
void EcoWStringToUpper(char *s)
{
	if(s == NULL)
	    return;

	while(*s != '\0')
	{
	    *s++ = toupper((int)*s);
	}

	return;
}

/*
 *	Gets the last selected row number or -1 on error nor
 *	no rows selected.
 */
int EcoWGetLastSelected(ecow_struct *ecow_ptr)
{
	int n;
	colum_list_struct *list;


	if(ecow_ptr == NULL)
	    return(-1);

	list = &ecow_ptr->list;
	if(list->total_sel_rows < 1)
	    return(-1);

	n = list->sel_row[list->total_sel_rows - 1];
	if((n < 0) || (n >= list->total_rows))
	    return(-1);

	if(list->row[n] == NULL)
	    return(-1);

	return(n);
}


/*
 *	Returns the pointer to the object referanced in the
 *	economy window ecow_ptr.  Returns NULL on error.
 */
xsw_object_struct *EcoWGetObjectPtr(
	ecow_struct *ecow_ptr
)
{
        int i, obj_num;
        uew_struct *uew_ptr; 


        if(ecow_ptr == NULL)
            return(NULL);


        uew_ptr = (uew_struct *)ecow_ptr->src;
        obj_num = ecow_ptr->src_obj_num;

	/* Check if uew pointer is valid. */
	for(i = 0; i < total_uews; i++)
	{
	    if(uew[i] == NULL)
		continue;

	    if(uew[i] == uew_ptr)
		break;
	}
	if(i >= total_uews)
	    return(NULL);


        if(UEWIsObjectGarbage(
            uew_ptr,
            obj_num
        ))
            return(NULL);
        else
            return(uew_ptr->object[obj_num]);
}


/*
 *	Checks if economy window n is allocated.
 */
int EcoWIsAllocated(int n)
{
        if((ecow == NULL) ||
           (n < 0) || (n >= total_ecows)
        )
            return(0);
        else if(ecow[n] == NULL)
            return(0);
        else
            return(1);
}          


/*
 *      Allocates a new universe header window, returning its
 *      index number.
 */      
int EcoWAllocate()
{
        int i, n;

         
        if(total_ecows < 0)
            total_ecows = 0;

        for(i = 0; i < total_ecows; i++)
        {   
            if(ecow[i] == NULL)
                break;
        }
        if(i < total_ecows)
        {
            n = i;
        }
        else
        {
            n = total_ecows;
            total_ecows++;
            ecow = (ecow_struct **)realloc(
                ecow,  
                total_ecows * sizeof(ecow_struct *)
            );
            if(ecow == NULL)
            {
                total_ecows = 0;
                return(-1);
            }
        }
 
        ecow[n] = (ecow_struct *)calloc(
            1,   
            sizeof(ecow_struct)
        );
        if(ecow[n] == NULL)
        {
            return(-1);   
        }


        return(n);
}


/*
 *	 Deletes economy window.
 */
void EcoWDelete(int n)
{
        if(!EcoWIsAllocated(n))
            return;

        /* Deallocate resources. */
        EcoWDestroy(n);

        /* Free structure itself. */
        free(ecow[n]);
        ecow[n] = NULL;
}

/*
 *	Deletes all economy windows.
 */
void EcoWDeleteAll()
{
        int i;


        for(i = 0; i < total_ecows; i++)
            EcoWDelete(i);

        free(ecow);
        ecow = NULL;

        total_ecows = 0;


        return;
}


/*
 *	Procedure to fetch all values from object specified in
 *	in economy window ecow_ptr and put them into the prompts
 *	and list window.  Prompts will be redrawn if mapped.
 *
 *	If the eco data structure does not exist on object
 *	then it will be allocated.
 */
int EcoWGetAllValues(ecow_struct *ecow_ptr)
{
	int i;
	colum_list_struct *list;
	xsw_object_struct *obj_ptr;
        xsw_ecodata_struct *eco_data_ptr;
	char prod_name[ECO_PRODUCT_NAME_MAX];
	char text[1024];


	if(ecow_ptr == NULL)
	    return(-1);

	list = &ecow_ptr->list;


	/* Delete all rows in list. */
	CListUnselectAllRows(list);
	CListDeleteAllRows(list);


	/* Get pointer to object referanced in economy window. */
	obj_ptr = EcoWGetObjectPtr(ecow_ptr);
	if(obj_ptr == NULL)
	{
	    /* No valid object. */

	    /* Reset referance to object. */
	    ecow_ptr->src_obj_num = -1;
	    ecow_ptr->src_obj_ptr = NULL;

	    /* Do not update list. */

	    /* Redraw list as needed. */
	    if(list->map_state)
		CListDraw(list, CL_DRAW_AMOUNT_COMPLETE);
	}
	else
	{
	    /* Allocate economy data as needed. */
	    if(obj_ptr->eco == NULL)
	    {
                obj_ptr->eco = (xsw_ecodata_struct *)calloc(
                    1,
                    sizeof(xsw_ecodata_struct)
                );
		if(obj_ptr->eco != NULL)
		{
                    obj_ptr->eco->flags = ECO_FLAG_OPEN |
			                  ECO_FLAG_BUY_OK;

		    obj_ptr->eco->tax_general = 1;
                    obj_ptr->eco->tax_friend = 1;
                    obj_ptr->eco->tax_hostile = 1;

		    obj_ptr->eco->product = NULL;
		    obj_ptr->eco->total_products = 0;
		}

		if(!ecow_ptr->has_changes)
		    ecow_ptr->has_changes = True;

	    }

	    /* Get pointer to eco data. */
            eco_data_ptr = obj_ptr->eco;


	    if(eco_data_ptr != NULL)
	    {
		/* Add eco data values to prompts. */
		sprintf(text,
		    "%s%s%s%s%s",
		    ((eco_data_ptr->flags & ECO_FLAG_OPEN) ?
			"OPEN" : ""),
        	    ((eco_data_ptr->flags & ECO_FLAG_BUY_OK) ?
			" | BUY_OK" : ""),
        	    ((eco_data_ptr->flags & ECO_FLAG_SELL_OK) ?
			" | SELL_OK" : ""),
        	    ((eco_data_ptr->flags & ECO_FLAG_TRADE_OK) ?
			" | TRADE_OK" : ""),
        	    ((eco_data_ptr->flags & ECO_FLAG_INTRODUCE_OK) ?
			" | INTRODUCE_OK" : "")
		);
		PromptSetS(&ecow_ptr->flags_prompt, text);

		PromptSetF(
		    &ecow_ptr->tax_general_prompt,
		    eco_data_ptr->tax_general - 1
		);

                PromptSetF(
                    &ecow_ptr->tax_friend_prompt,
                    eco_data_ptr->tax_friend - 1
                );

                PromptSetF(
                    &ecow_ptr->tax_hostile_prompt,
                    eco_data_ptr->tax_hostile - 1
                );


	        /* Add list rows. */
	        for(i = 0; i < eco_data_ptr->total_products; i++)
	        {
		    if(eco_data_ptr->product[i] == NULL)
		        continue;

		    if(CListAddRow(list, i))
		        break;

		    strncpy(
			prod_name,
			((eco_data_ptr->product[i]->name == NULL) ?
			    "(null)" : eco_data_ptr->product[i]->name),
			ECO_PRODUCT_NAME_MAX
		    );
		    prod_name[ECO_PRODUCT_NAME_MAX - 1] = '\0';

		    CListAddItem(
		        list,
		        prod_name,
		        OSWQueryCurrentFont(),
		        widget_global.editable_text_pix,
		        0,		/* Attributes. */
		        i		/* Row number. */
		    );
	        }
	    }

            /* Redraw list as needed. */
            if(list->map_state)
                CListDraw(list, CL_DRAW_AMOUNT_COMPLETE);
	}


	/* Reset prompts. */
        PromptSetS(
            &ecow_ptr->prod_name_prompt,
            ""
	);
        PromptSetS(
            &ecow_ptr->prod_sell_prompt,
            ""
        );
        PromptSetS(
            &ecow_ptr->prod_buy_prompt,
            ""
        );    
        PromptSetS(
            &ecow_ptr->prod_amount_prompt,
            ""
        );    
        PromptSetS(
            &ecow_ptr->prod_max_prompt,
            ""
        );


	return(0);
}

/*
 *	Fetches values from product number prod_num and puts
 *	them into the prompts.  (does not update the list window).
 */
int EcoWGetValues(ecow_struct *ecow_ptr, int prod_num)
{
        xsw_object_struct *obj_ptr;
	xsw_ecodata_struct *eco_data_ptr;
	xsw_ecoproduct_struct *eco_prod_ptr;
        char text[1024];


        if(ecow_ptr == NULL)
            return(-1);

	/* Get object pointer. */
	obj_ptr = EcoWGetObjectPtr(ecow_ptr);
	if(obj_ptr == NULL)
	    return(-1);

	/* Get eco data pointer on object. */
	eco_data_ptr = obj_ptr->eco;
	if(eco_data_ptr == NULL)
	    return(-1);

	/* Get product pointer. */
	if((prod_num < 0) || (prod_num >= eco_data_ptr->total_products))
	    return(-1);
	if(eco_data_ptr->product[prod_num] == NULL)
	    return(-1);

	eco_prod_ptr = eco_data_ptr->product[prod_num];


	/* Get values and put into prompts. */

        PromptSetS(
            &ecow_ptr->prod_name_prompt,
            eco_prod_ptr->name
        );

        sprintf(text, "%.4f", eco_prod_ptr->sell_price);
        PromptSetS(
            &ecow_ptr->prod_sell_prompt,
            text
        );

        sprintf(text, "%.4f", eco_prod_ptr->buy_price);
        PromptSetS(
            &ecow_ptr->prod_buy_prompt,
            text
        );

        sprintf(text, "%.4f", eco_prod_ptr->amount);
        PromptSetS(
            &ecow_ptr->prod_amount_prompt,
            text
        );

        sprintf(text, "%.4f", eco_prod_ptr->amount_max);
        PromptSetS(
            &ecow_ptr->prod_max_prompt,
            text
        );


	return(0);
}

/*
 *	Sets the values in the prompts of the economy window ecow_ptr
 *	into the object's economy product.
 *
 *	If prod_num does not exist on object it will *not* be created.
 */
int EcoWSetValues(ecow_struct *ecow_ptr, int prod_num)
{
	char *strptr;
	colum_list_struct *list;
	colum_list_row_struct *row_ptr;

        xsw_object_struct *obj_ptr;
        xsw_ecodata_struct *eco_data_ptr;
        xsw_ecoproduct_struct *eco_prod_ptr;


        if(ecow_ptr == NULL)
            return(-1);

	list = &ecow_ptr->list;


        /* Get object pointer. */
        obj_ptr = EcoWGetObjectPtr(ecow_ptr); 
        if(obj_ptr == NULL)
            return(-1);

        /* Get eco data pointer on object. */
        eco_data_ptr = obj_ptr->eco;
        if(eco_data_ptr == NULL)
            return(-1);

        /* Get product pointer. */
        if((prod_num < 0) || (prod_num >= eco_data_ptr->total_products))
            return(-1);
        if(eco_data_ptr->product[prod_num] == NULL)
            return(-1);

        eco_prod_ptr = eco_data_ptr->product[prod_num];


        /* Set values from prompts. */

	strptr = PromptGetS(&ecow_ptr->prod_name_prompt);
	strncpy(
	    eco_prod_ptr->name,
	    ((strptr == NULL) ? "(null)" : strptr),
	    ECO_PRODUCT_NAME_MAX
	);
	eco_prod_ptr->name[ECO_PRODUCT_NAME_MAX - 1] = '\0';

	eco_prod_ptr->sell_price = PromptGetF(
	    &ecow_ptr->prod_sell_prompt
	);

        eco_prod_ptr->buy_price = PromptGetF(
            &ecow_ptr->prod_buy_prompt
        );

        eco_prod_ptr->amount = PromptGetF(
            &ecow_ptr->prod_amount_prompt
        );

        eco_prod_ptr->amount_max = PromptGetF(
            &ecow_ptr->prod_max_prompt
        );


	/* Update name on list. */
	if((prod_num >= 0) && (prod_num < list->total_rows))
	{
	    row_ptr = list->row[prod_num];
	    if(row_ptr != NULL)
	    {
		if(row_ptr->total_items >= 1)
		{
		    free(row_ptr->item[0]->label);
		    row_ptr->item[0]->label = StringCopyAlloc(
			eco_prod_ptr->name
		    );
		}
	    }

            /* Redraw list as needed. */
            if(list->map_state)
                CListDraw(list, CL_DRAW_AMOUNT_LIST);
	}


	return(0);
}


/*
 *	Economy list callback.
 */
int EcoWListCB(void *ptr)
{



	return(0);
}

/*
 *	Create button callback.
 */
int EcoWCreateCB(void *ptr)
{
        int i, ecow_num;
        ecow_struct *ecow_ptr = NULL;

	colum_list_struct *list;
	xsw_object_struct *obj_ptr;
	xsw_ecodata_struct *eco_data_ptr;
        xsw_ecoproduct_struct *eco_prod_ptr;


        if(ptr == NULL)
            return(-1);


        for(i = 0; i < total_ecows; i++)
        {
            if(ecow[i] == NULL)
                continue;

            if(ecow[i] == (ecow_struct *)ptr)
            {
                ecow_ptr = (ecow_struct *)ptr;
                break;
            }
        }
        if(ecow_ptr == NULL)
            return(-1);
        else
            ecow_num = i;

	/* Get pointer to list. */
        list = &ecow_ptr->list;


	/* Apply values to currently selected product. */
	if(list->total_sel_rows > 0)
	{
	    EcoWSetValues(
		ecow_ptr,
		list->sel_row[list->total_sel_rows - 1]
	    );
	}


	/* Allocate a new product on object. */
	obj_ptr = EcoWGetObjectPtr(ecow_ptr);
	if(obj_ptr != NULL)
	{
	    eco_data_ptr = obj_ptr->eco;

            if(eco_data_ptr != NULL)
            {
                /* Create a new product. */

		if(eco_data_ptr->total_products < 0)
		    eco_data_ptr->total_products = 0;

		i = eco_data_ptr->total_products;
		eco_data_ptr->total_products++;
		eco_data_ptr->product = (xsw_ecoproduct_struct **)realloc(
		    eco_data_ptr->product,
		    eco_data_ptr->total_products * sizeof(xsw_ecoproduct_struct *)
		);
		if(eco_data_ptr->product == NULL)
		{
		    eco_data_ptr->total_products = 0;
		    return(-1);
		}

		eco_data_ptr->product[i] = (xsw_ecoproduct_struct *)calloc(
		    1,
		    sizeof(xsw_ecoproduct_struct)
		);
		eco_prod_ptr = eco_data_ptr->product[i];
		if(eco_prod_ptr == NULL)
		{
		    return(-1);
		}

		strncpy(
		    eco_prod_ptr->name,
		    "New Product",
		    ECO_PRODUCT_NAME_MAX
		);
		eco_prod_ptr->name[ECO_PRODUCT_NAME_MAX - 1] = '\0';


		/*   Redget all values, update list, and unselect
		 *   rows.
		 */
		EcoWGetAllValues(ecow_ptr);
	    }
	}

        /* Update has changes marker. */
        if(!ecow_ptr->has_changes)
            ecow_ptr->has_changes = True;


	/* Redraw list as needed. */
	if(list->map_state)
	    CListDraw(list, CL_DRAW_AMOUNT_COMPLETE);


	return(0);
}

/*
 *	Delete button callback.
 */
int EcoWDeleteCB(void *ptr)
{
        int i, n, sel_prod_num;

	int ecow_num;
        ecow_struct *ecow_ptr = NULL;

        colum_list_struct *list;
        xsw_object_struct *obj_ptr;
	xsw_ecodata_struct *eco_data_ptr;


        if(ptr == NULL)  
            return(-1);


        for(i = 0; i < total_ecows; i++)
        {
            if(ecow[i] == NULL)
                continue;

            if(ecow[i] == (ecow_struct *)ptr)
            {
                ecow_ptr = (ecow_struct *)ptr;
                break;
            }   
        }       
        if(ecow_ptr == NULL)
            return(-1);
        else
            ecow_num = i;

	/* Get pointer to list. */
        list = &ecow_ptr->list;


        /* Nothing selected? */
        if(list->total_sel_rows <= 0)
            return(0);


	/* Get pointer to object (must be successful). */
        obj_ptr = EcoWGetObjectPtr(ecow_ptr);
	if(obj_ptr == NULL)
	    return(-1);


        /*   Do not apply values to currently selected product,
	 *   since it will be deleted.
	 */

	eco_data_ptr = obj_ptr->eco;
	if(eco_data_ptr == NULL)
	    return(0);

	/* Get and check if selected product is valid. */
	sel_prod_num = list->sel_row[list->total_sel_rows - 1];
	if((sel_prod_num < 0) ||
	   (sel_prod_num >= eco_data_ptr->total_products)
	)
	    return(-1);


	/* Deallocate selected product. */
	free(eco_data_ptr->product[sel_prod_num]);

	/* Shift pointer array on object. */
	for(i = sel_prod_num, n = eco_data_ptr->total_products - 1;
            i < n;
            i++
	)
	    eco_data_ptr->product[i] = eco_data_ptr->product[i + 1];


	/* Reallocate product pointers. */
	eco_data_ptr->total_products--;

	if(eco_data_ptr->total_products > 0)
	{
	    eco_data_ptr->product = (xsw_ecoproduct_struct **)realloc(
                eco_data_ptr->product,
                eco_data_ptr->total_products * sizeof(xsw_ecoproduct_struct *)
            );
            if(eco_data_ptr->product == NULL)
	    {
		eco_data_ptr->total_products = 0;
	    }
	}
	else
	{
	    free(eco_data_ptr->product);
	    eco_data_ptr->product = NULL;

	    eco_data_ptr->total_products = 0;
	}
	if(eco_data_ptr->total_products < 0)
	    eco_data_ptr->total_products = 0;


	/* Update has changes marker. */
	if(!ecow_ptr->has_changes)
	    ecow_ptr->has_changes = True;


        /*   Redget all values, update list, and unselect
         *   rows. 
         */
        EcoWGetAllValues(ecow_ptr);


        /* Redraw list as needed. */
        if(list->map_state)
            CListDraw(list, CL_DRAW_AMOUNT_COMPLETE);


        return(0);
}

/*
 *	Delete economy data on object callback.
 *
 *	This will delete all allocated economy stuff (both ecodata
 *	and products on the object).
 */
int EcoWDeleteEcoCB(void *ptr)
{
        int i, status;
        int ecow_num;
        ecow_struct *ecow_ptr = NULL;
        colum_list_struct *list;
        xsw_object_struct *obj_ptr;
        xsw_ecodata_struct *eco_data_ptr;
	char text[XSW_OBJ_NAME_MAX + 256];


        if(ptr == NULL)  
            return(-1);


        for(i = 0; i < total_ecows; i++)
        {
            if(ecow[i] == NULL)
                continue;
        
            if(ecow[i] == (ecow_struct *)ptr)
            {
                ecow_ptr = (ecow_struct *)ptr;
                break;
            }
        }
        if(ecow_ptr == NULL)
            return(-1);
        else
            ecow_num = i;

        /* Get pointer to list. */
        list = &ecow_ptr->list;


        /* Get pointer to object referanced in economy window. */
        obj_ptr = EcoWGetObjectPtr(ecow_ptr);
        if(obj_ptr != NULL)
	{
	    /* Comferm delete. */
	    sprintf(text,
 "Comferm delete all economy data on object:\n\n    %s",
                UNVGetObjectFormalName(
		    obj_ptr,
		    ecow_ptr->src_obj_num
		)
	    );
            comfwin.option &= ~ComfirmOptionAll; 
            comfwin.option &= ~ComfirmOptionCancel;
            status = ComfWinDoQuery(&comfwin, text);
            if(status != ComfirmCodeYes)
                return(0);


            /* Delete all rows in list. */
            CListUnselectAllRows(list);
            CListDeleteAllRows(list);


	    /* Get pointer to economy data structure. */
	    eco_data_ptr = obj_ptr->eco;
	    if(eco_data_ptr != NULL)
	    {
		/* Free all products. */
		for(i = 0; i < eco_data_ptr->total_products; i++)
		    free(eco_data_ptr->product[i]);

		free(eco_data_ptr->product);
		eco_data_ptr->product = NULL;

		eco_data_ptr->total_products = 0;


		/* Free eco data structure. */
		free(eco_data_ptr);
		eco_data_ptr = NULL;
	    }

	    obj_ptr->eco = NULL;
	}

	/* Update has changes mark. */
	if(!ecow_ptr->has_changes)
	    ecow_ptr->has_changes = True;


	/* Unmap since user has nothing more to do with this window. */
	EcoWUnmap(ecow_num);


	return(0);
}

/*
 *	Close button callback.
 */
int EcoWCloseCB(void *ptr)
{ 
        int i, ecow_num;
	char *strptr, *strptr2;
        ecow_struct *ecow_ptr = NULL;
	colum_list_struct *list;

	uew_struct *uew_ptr;
	xsw_object_struct *obj_ptr;
	xsw_ecodata_struct *eco_data_ptr;


        if(ptr == NULL)
            return(-1);


        for(i = 0; i < total_ecows; i++)
        {    
            if(ecow[i] == NULL)
                continue;
        
            if(ecow[i] == (ecow_struct *)ptr)
	    {
                ecow_ptr = (ecow_struct *)ptr;
		break;
	    }
        }
        if(ecow_ptr == NULL)
            return(-1);
        else
            ecow_num = i;

	list = &ecow_ptr->list;


	/* Apply changes on currently selected product. */
	if(list->total_sel_rows > 0)
	{
	    i = list->sel_row[list->total_sel_rows - 1];
	    EcoWSetValues(
		ecow_ptr,
		i
	    );
	}

	/* Apply eco data values. */
	obj_ptr = EcoWGetObjectPtr(ecow_ptr);
	if(obj_ptr != NULL)
	{
	    eco_data_ptr = obj_ptr->eco;
	    if(eco_data_ptr != NULL)
	    {
		/* Flags. */
		strptr = PromptGetS(&ecow_ptr->flags_prompt);
		if(strptr != NULL)
		{
		    EcoWStringToUpper(strptr);

		    eco_data_ptr->flags = 0;

		    strptr2 = strstr(strptr, "OPEN");
		    if(strptr2 != NULL)
			eco_data_ptr->flags |= ECO_FLAG_OPEN;

                    strptr2 = strstr(strptr, "BUY_OK");
                    if(strptr2 != NULL)
                        eco_data_ptr->flags |= ECO_FLAG_BUY_OK;

                    strptr2 = strstr(strptr, "SELL_OK");
                    if(strptr2 != NULL)
                        eco_data_ptr->flags |= ECO_FLAG_SELL_OK;

                    strptr2 = strstr(strptr, "TRADE_OK");
                    if(strptr2 != NULL)
                        eco_data_ptr->flags |= ECO_FLAG_TRADE_OK;

                    strptr2 = strstr(strptr, "INTRODUCE_OK");
                    if(strptr2 != NULL)
                        eco_data_ptr->flags |= ECO_FLAG_INTRODUCE_OK;
		}

		/* Tax general. */
		eco_data_ptr->tax_general = PromptGetF(&ecow_ptr->tax_general_prompt) + 1;

                /* Tax friend. */
                eco_data_ptr->tax_friend = PromptGetF(&ecow_ptr->tax_friend_prompt) + 1;

                /* Tax hostile. */
                eco_data_ptr->tax_hostile = PromptGetF(&ecow_ptr->tax_hostile_prompt) + 1;
	    }
	}


	/* Unmap. */
        EcoWUnmap(ecow_num);


        /* Update has changes marker on corresponding uew. */
        if(ecow_ptr->has_changes)
        {
	    /* Mark uew to have changes too. */
	    for(i = 0, uew_ptr = NULL; i < total_uews; i++)
	    {
		if(uew[i] == NULL)
		    continue;

		if(uew[i] == (uew_struct *)ecow_ptr->src)
		    uew_ptr = uew[i];
	    }
	    if(uew_ptr != NULL)
	    {
		if(!uew_ptr->has_changes)
		    uew_ptr->has_changes = True;
	    }

	    ecow_ptr->has_changes = False;
	}


        return(0);
} 

/*
 *      Forcus prompt macro for EcoWRefocusPrompts().
 */
void EcoWDoPromptFocus(
        prompt_window_struct *src_prompt,
        prompt_window_struct *tar_prompt
)
{
        if((src_prompt == NULL) ||
           (tar_prompt == NULL)
        )
            return;

        if(src_prompt == tar_prompt)
        {
            src_prompt->is_in_focus = 1;
            PromptMarkBuffer(src_prompt, PROMPT_POS_END);
        }
        else
        {
            src_prompt->is_in_focus = 0;
            PromptUnmarkBuffer(src_prompt, PROMPT_POS_END);
        }
        if(src_prompt->map_state)
            PromptDraw(src_prompt, PROMPT_DRAW_AMOUNT_TEXTONLY);


        return;
}


/*
 *      Procedure to refocus all prompts on ecow,
 *      prompt will be set into focus.  All prompts will be redrawn
 *      and marked/unmarked.
 */
void EcoWRefocusPrompts(
	ecow_struct *ecow_ptr,
	prompt_window_struct *prompt
) 
{
        if((ecow_ptr == NULL) ||
           (prompt == NULL)
        )
            return;

	EcoWDoPromptFocus(&ecow_ptr->flags_prompt, prompt);
        EcoWDoPromptFocus(&ecow_ptr->tax_general_prompt, prompt);
        EcoWDoPromptFocus(&ecow_ptr->tax_friend_prompt, prompt);
        EcoWDoPromptFocus(&ecow_ptr->tax_hostile_prompt, prompt);

        EcoWDoPromptFocus(&ecow_ptr->prod_name_prompt, prompt);
        EcoWDoPromptFocus(&ecow_ptr->prod_buy_prompt, prompt);
        EcoWDoPromptFocus(&ecow_ptr->prod_sell_prompt, prompt);
        EcoWDoPromptFocus(&ecow_ptr->prod_amount_prompt, prompt);
        EcoWDoPromptFocus(&ecow_ptr->prod_max_prompt, prompt);


        return;
}


/*
 *	Initializes a economy window.
 */
int EcoWInit(int n, void *src_ptr)
{
	int x, y;
        char hotkey[PBTN_MAX_HOTKEYS];
        win_attr_t wattr;
        ecow_struct *ecow_ptr;



        if(EcoWIsAllocated(n))  
            ecow_ptr = ecow[n];
        else
            return(-1);
            
            
        ecow_ptr->map_state = 0;
        ecow_ptr->is_in_focus = 0; 
        ecow_ptr->x = 0;
        ecow_ptr->y = 0;
        ecow_ptr->width = ECOW_WIDTH;
        ecow_ptr->height = ECOW_HEIGHT;

        ecow_ptr->has_changes = False;
        ecow_ptr->src = src_ptr;

        /* ******************************************************** */
        /* Create toplevel. */
        if(
            OSWCreateWindow(
                &ecow_ptr->toplevel,
                osw_gui[0].root_win,
                ecow_ptr->x, ecow_ptr->y,
                ecow_ptr->width, ecow_ptr->height
            )
        )
            return(-1);

        OSWSetWindowWMProperties(
            ecow_ptr->toplevel,
            ECOW_DEF_TITLE,
            ECOW_DEF_ICON_TITLE,
            ((ue_image.unvedit_icon_pm == 0) ?
                widget_global.std_icon_pm : ue_image.unvedit_icon_pm),
            False,                      /* Let WM set coordinates? */
            ecow_ptr->x, ecow_ptr->y,
            ecow_ptr->width, ecow_ptr->height,
            ecow_ptr->width, ecow_ptr->height,
            WindowFrameStyleFixed,
            NULL, 0
        );
        
        OSWSetWindowInput(
            ecow_ptr->toplevel,
            OSW_EVENTMASK_TOPLEVEL   
        );
                
        WidgetCenterWindow(ecow_ptr->toplevel, WidgetCenterWindowToRoot);
        OSWGetWindowAttributes(ecow_ptr->toplevel, &wattr);
        ecow_ptr->x = wattr.x;
        ecow_ptr->y = wattr.y; 
        ecow_ptr->width = wattr.width;
        ecow_ptr->height = wattr.height;


	/* Products list. */
        if(
            CListInit(
                &ecow_ptr->list,
                ecow_ptr->toplevel,
                10,
		(4 * ECOW_PROMPT_HEIGHT) + (5 * 10),
                MAX(
		    (int)ecow_ptr->width - ECOW_BUTTON_WIDTH - (3 * 10),
		    50
		),
		MAX(
		    (int)ecow_ptr->height -
		    (7 * ECOW_PROMPT_HEIGHT) - (12 * 10) -
		    (1 * ECOW_BUTTON_HEIGHT),
		    50
		),
                (void *)&ecow_ptr,
                EcoWListCB
            )
        )
            return(-1);
	ecow_ptr->list.option = 0;

        CListAddHeading(
            &ecow_ptr->list,
            "Product",
            OSWQueryCurrentFont(),
            widget_global.normal_text_pix,
            0,
            0
        );

	/* Create button. */
        if( 
            PBtnInit(
                &ecow_ptr->create_prod_btn,
                ecow_ptr->toplevel,
                (int)ecow_ptr->width - ECOW_BUTTON_WIDTH - 10,
		(4 * ECOW_PROMPT_HEIGHT) + (5 * 10),
                ECOW_BUTTON_WIDTH, ECOW_BUTTON_HEIGHT,
                "Create",
                PBTN_TALIGN_CENTER,
                NULL,
                (void *)ecow_ptr,
                EcoWCreateCB
            )
        )
            return(-1);

        /* Delete button. */
        if(
            PBtnInit(
                &ecow_ptr->delete_prod_btn,
                ecow_ptr->toplevel,
                (int)ecow_ptr->width - ECOW_BUTTON_WIDTH - 10,
		(4 * ECOW_PROMPT_HEIGHT) + (5 * 10) +
		   (1 * ECOW_BUTTON_HEIGHT) + (1 * 10),
                ECOW_BUTTON_WIDTH, ECOW_BUTTON_HEIGHT,
                "Delete",
                PBTN_TALIGN_CENTER,
                NULL,
                (void *)ecow_ptr,
                EcoWDeleteCB
            )
        )
            return(-1);


	/* Flags prompt. */
	x = 10;
	y = 10;
	if(
            PromptInit(
                &ecow_ptr->flags_prompt,
                ecow_ptr->toplevel,
                x, y,
                (int)ecow_ptr->width - 20,
                ECOW_PROMPT_HEIGHT,
                PROMPT_STYLE_FLUSHED,
                "Flags:",
                1024,
                0,
                NULL
            )
        )
            return(-1);

	/* Tax general. */
	x = 10;
	y += (ECOW_PROMPT_HEIGHT + 10);
        if(
            PromptInit(
                &ecow_ptr->tax_general_prompt,
                ecow_ptr->toplevel,
                x, y,
                180,
                ECOW_PROMPT_HEIGHT,
                PROMPT_STYLE_FLUSHED,
                "Tax General:",
                80,
                0,
                NULL
            )
        )
            return(-1);

	/* Tax friend. */
        y += (ECOW_PROMPT_HEIGHT + 10);
        if(
            PromptInit(
                &ecow_ptr->tax_friend_prompt,
                ecow_ptr->toplevel,
                x, y,
                180,
                ECOW_PROMPT_HEIGHT,
                PROMPT_STYLE_FLUSHED,
                "Tax Friend:",
                80,
                0,  
                NULL
            )
        )
            return(-1);

	/* Tax hostile. */
        y += (ECOW_PROMPT_HEIGHT + 10);
        if(
            PromptInit(
                &ecow_ptr->tax_hostile_prompt,
                ecow_ptr->toplevel,
                x, y,
                180,
                ECOW_PROMPT_HEIGHT,
                PROMPT_STYLE_FLUSHED,
                "Tax Hostile:",
                80,
                0,
                NULL 
            )
        )
            return(-1);


	/* Product name. */
	x = 10;
        y = (int)ecow_ptr->height - (1 * ECOW_BUTTON_HEIGHT) -
	    (6 * 10) - (3 * ECOW_PROMPT_HEIGHT);

        if(
            PromptInit(
                &ecow_ptr->prod_name_prompt,
                ecow_ptr->toplevel,
                x, y,
                (int)ecow_ptr->width - 20,
                ECOW_PROMPT_HEIGHT,
                PROMPT_STYLE_FLUSHED,
                "Name:",
                ECO_PRODUCT_NAME_MAX,
                0,
                NULL
            )
        )
            return(-1);

        /* Buy price. */
	x = 10;
        y += (ECOW_PROMPT_HEIGHT + 10);
        if(
            PromptInit(
                &ecow_ptr->prod_buy_prompt,
                ecow_ptr->toplevel,
                x, y,
                160,
                ECOW_PROMPT_HEIGHT,
                PROMPT_STYLE_FLUSHED,
                "Buy Price:",
                80,
                0,
                NULL
            )
        )
            return(-1);

	/* Sell price. */
        x = 180;
        if(
            PromptInit(
                &ecow_ptr->prod_sell_prompt,
                ecow_ptr->toplevel,
                x, y,
                160,
                ECOW_PROMPT_HEIGHT,
                PROMPT_STYLE_FLUSHED,
                "Sell Price:",
                80,
                0,
                NULL
            )
        )
            return(-1);


	/* Amount prompt. */
	x = 10;
        y += (ECOW_PROMPT_HEIGHT + 10);
        if(
            PromptInit(
                &ecow_ptr->prod_amount_prompt,
                ecow_ptr->toplevel,
                x, y,
                160,
                ECOW_PROMPT_HEIGHT,
                PROMPT_STYLE_FLUSHED,
                "Amount:",
                80,
                0,
                NULL
            )
        )
            return(-1);

	/* Amount max. */
        x = 180;
        if(
            PromptInit(
                &ecow_ptr->prod_max_prompt,
                ecow_ptr->toplevel,
                x, y,
                160,
                ECOW_PROMPT_HEIGHT,
                PROMPT_STYLE_FLUSHED,
                "Max:",
                80,
                0,
                NULL
            )
        )
            return(-1);


	/* Link prompts togeather. */
	ecow_ptr->flags_prompt.next = &ecow_ptr->tax_general_prompt;
	ecow_ptr->flags_prompt.prev = &ecow_ptr->prod_max_prompt;

        ecow_ptr->tax_general_prompt.next = &ecow_ptr->tax_friend_prompt;
        ecow_ptr->tax_general_prompt.prev = &ecow_ptr->flags_prompt;

        ecow_ptr->tax_friend_prompt.next = &ecow_ptr->tax_hostile_prompt;
        ecow_ptr->tax_friend_prompt.prev = &ecow_ptr->tax_general_prompt;

        ecow_ptr->tax_hostile_prompt.next = &ecow_ptr->prod_name_prompt;
        ecow_ptr->tax_hostile_prompt.prev = &ecow_ptr->tax_friend_prompt;


        ecow_ptr->prod_name_prompt.next = &ecow_ptr->prod_buy_prompt;
        ecow_ptr->prod_name_prompt.prev = &ecow_ptr->tax_hostile_prompt;

        ecow_ptr->prod_buy_prompt.next = &ecow_ptr->prod_sell_prompt;
        ecow_ptr->prod_buy_prompt.prev = &ecow_ptr->prod_name_prompt;

        ecow_ptr->prod_sell_prompt.next = &ecow_ptr->prod_amount_prompt;
        ecow_ptr->prod_sell_prompt.prev = &ecow_ptr->prod_buy_prompt;

        ecow_ptr->prod_amount_prompt.next = &ecow_ptr->prod_max_prompt;
        ecow_ptr->prod_amount_prompt.prev = &ecow_ptr->prod_sell_prompt;

        ecow_ptr->prod_max_prompt.next = &ecow_ptr->flags_prompt;
        ecow_ptr->prod_max_prompt.prev = &ecow_ptr->prod_amount_prompt;


        /* Close button. */
        if(
            PBtnInit(
                &ecow_ptr->close_btn,
                ecow_ptr->toplevel,  
                (int)ecow_ptr->width - ECOW_BUTTON_WIDTH - 10,
                (int)ecow_ptr->height - 10 - ECOW_BUTTON_HEIGHT,
                ECOW_BUTTON_WIDTH, ECOW_BUTTON_HEIGHT,
                "Close",
                PBTN_TALIGN_CENTER,
                NULL,
                (void *)ecow_ptr,
                EcoWCloseCB
            )
        )
            return(-1);
        hotkey[0] = '\n';
	hotkey[1] = 0x1b;
        hotkey[2] = '\0';
        PBtnSetHotKeys(&ecow_ptr->close_btn, hotkey);


        /* Delete object's economy. */
        if(
            PBtnInit(
                &ecow_ptr->delete_obj_eco_btn,
                ecow_ptr->toplevel,
                10,
                (int)ecow_ptr->height - 10 - ECOW_BUTTON_HEIGHT,
                190, ECOW_BUTTON_HEIGHT,
                "Delete Object's Economy",
                PBTN_TALIGN_CENTER,
                NULL,
                (void *)ecow_ptr,
                EcoWDeleteEcoCB
            )
        )
            return(-1);



        /* Need to update window menus on uews. */
        UEWDoUpdateWindowMenus();


        return(0);
}


int EcoWDraw(int n, int amount)
{
        win_t w;
        pixmap_t pixmap;
        ecow_struct *ecow_ptr;
        win_attr_t wattr;


        if(EcoWIsAllocated(n))
            ecow_ptr = ecow[n];
        else
            return(-1);  
                
                
        /* Map as needed. */
        if(!ecow_ptr->map_state)
        {
            OSWMapRaised(ecow_ptr->toplevel);

	    CListMap(&ecow_ptr->list);

            PBtnMap(&ecow_ptr->create_prod_btn);
            PBtnMap(&ecow_ptr->delete_prod_btn); 

	    PromptMap(&ecow_ptr->flags_prompt);
            PromptMap(&ecow_ptr->tax_general_prompt);
            PromptMap(&ecow_ptr->tax_friend_prompt);
            PromptMap(&ecow_ptr->tax_hostile_prompt);

            PromptMap(&ecow_ptr->prod_name_prompt);
            PromptMap(&ecow_ptr->prod_buy_prompt);
            PromptMap(&ecow_ptr->prod_sell_prompt);
            PromptMap(&ecow_ptr->prod_amount_prompt);
            PromptMap(&ecow_ptr->prod_max_prompt);

            PBtnMap(&ecow_ptr->delete_obj_eco_btn);
            PBtnMap(&ecow_ptr->close_btn);

            ecow_ptr->map_state = 1;
	}


	if(ecow_ptr->toplevel_buf == 0)
	{
	    OSWGetWindowAttributes(ecow_ptr->toplevel, &wattr);
	    if(OSWCreatePixmap(
		&ecow_ptr->toplevel_buf,
		wattr.width, wattr.height
	    ))
		return(-1);
	}


	if(1)
	{
            w = ecow_ptr->toplevel;
            pixmap = ecow_ptr->toplevel_buf;

            OSWGetWindowAttributes(w, &wattr);

            /* Redraw background. */
            if(widget_global.force_mono)
            {
                OSWClearPixmap(
                    pixmap,
                    wattr.width, wattr.height,
                    osw_gui[0].black_pix
                );
            }
            else
            {
                WidgetPutImageTile(
                    pixmap,
                    widget_global.std_bkg_img,
                    wattr.width, wattr.height
                );

		OSWSetFgPix(widget_global.surface_shadow_pix);
		OSWDrawLine(
		    pixmap,
		    0,
		    (int)wattr.height - 25 - ECOW_BUTTON_HEIGHT,
                    wattr.width,
                    (int)wattr.height - 25 - ECOW_BUTTON_HEIGHT
		);
                OSWSetFgPix(widget_global.surface_highlight_pix);
                OSWDrawLine(
                    pixmap,
                    0,
                    (int)wattr.height - 24 - ECOW_BUTTON_HEIGHT,
                    wattr.width,
                    (int)wattr.height - 24 - ECOW_BUTTON_HEIGHT
                );
            }


            OSWPutBufferToWindow(w, pixmap);
	}


	return(0);
}


int EcoWManage(int n, event_t *event)
{
	int i, p;
        keycode_t keycode;
        int events_handled = 0;
        ecow_struct *ecow_ptr;
	colum_list_struct *list;


        static_ecow::delete_ecow = NULL;


        if(EcoWIsAllocated(n))
            ecow_ptr = ecow[n];
        else
            return(events_handled);


        if((event == NULL) ||
           !ecow_ptr->map_state
        )
            return(events_handled);
                
                
        switch(event->type)
        {
          /* ******************************************************* */
          case KeyPress:
            if(!ecow_ptr->is_in_focus)
                break;

            keycode = event->xkey.keycode;

            /* Enter. */
            if((keycode == osw_keycode.enter) ||
               (keycode == osw_keycode.np_enter)
            )
            {
                EcoWCloseCB((void *)ecow_ptr);

                /* Delete this ecow as needed. */
                if(static_ecow::delete_ecow == ecow_ptr)
                {
                    EcoWDelete(n);
                    UEWDoUpdateWindowMenus();
                }

                events_handled++;
                return(events_handled);
            }
	    break;

          /* ******************************************************* */
          case KeyRelease:
            if(!ecow_ptr->is_in_focus) 
                break;

            break;

          /* ******************************************************* */
          case Expose:
            if(event->xany.window == ecow_ptr->toplevel)
            {
                events_handled++;
            }
            break;

          /* ******************************************************* */
          case FocusIn:
            if(event->xany.window == ecow_ptr->toplevel)
            {
                ecow_ptr->is_in_focus = 1;

                events_handled++;
                return(events_handled);
            }
            break;

          /* ******************************************************* */
          case FocusOut:
            if(event->xany.window == ecow_ptr->toplevel)
            {
                ecow_ptr->is_in_focus = 0;

                events_handled++;
                return(events_handled);
            }   
            break;
         
          /* ******************************************************* */
          case ClientMessage:
            if(OSWIsEventDestroyWindow(ecow_ptr->toplevel, event))
            {
                EcoWCloseCB((void *)ecow_ptr);

                /* Delete this ecow as needed. */
                if(static_ecow::delete_ecow == ecow_ptr)
		{
                    EcoWDelete(n);
                    UEWDoUpdateWindowMenus();
		}

                events_handled++;
                return(events_handled);
            }
            break;
	}

        if(events_handled > 0)
        {
            EcoWDraw(n, 0);
        }


	/* Manage widgets. */

	/* Products list. */
        if(events_handled == 0)
	{
	    list = &ecow_ptr->list;

	    if(list->total_sel_rows > 0)
		p = list->sel_row[list->total_sel_rows - 1];
	    else
		p = -1;

            events_handled += CListManage(
		&ecow_ptr->list,
		event
	    );
	    if(events_handled > 0)
	    {
		if(list->total_sel_rows > 0)
                    i = list->sel_row[list->total_sel_rows - 1]; 
                else
                    i = -1;

		if(p != i)
		{
		    EcoWSetValues(ecow_ptr, p);
		    EcoWGetValues(ecow_ptr, i);
		}
	    }
	}

        if(events_handled == 0)
            events_handled += PBtnManage(&ecow_ptr->delete_obj_eco_btn, event);
        if(events_handled == 0)
            events_handled += PBtnManage(&ecow_ptr->close_btn, event);

        if(events_handled == 0)
            events_handled += PBtnManage(&ecow_ptr->create_prod_btn, event);
        if(events_handled == 0)
            events_handled += PBtnManage(&ecow_ptr->delete_prod_btn, event);

        if(events_handled == 0)
            events_handled += PromptManage(&ecow_ptr->flags_prompt, event);
        if(events_handled == 0)
            events_handled += PromptManage(&ecow_ptr->tax_general_prompt, event);
        if(events_handled == 0)
            events_handled += PromptManage(&ecow_ptr->tax_friend_prompt, event);
        if(events_handled == 0)
            events_handled += PromptManage(&ecow_ptr->tax_hostile_prompt, event);

        if(events_handled == 0)
            events_handled += PromptManage(&ecow_ptr->prod_name_prompt, event);
        if(events_handled == 0)
            events_handled += PromptManage(&ecow_ptr->prod_buy_prompt, event);
        if(events_handled == 0)
            events_handled += PromptManage(&ecow_ptr->prod_sell_prompt, event);
        if(events_handled == 0)
            events_handled += PromptManage(&ecow_ptr->prod_amount_prompt, event);
        if(events_handled == 0)
            events_handled += PromptManage(&ecow_ptr->prod_max_prompt, event);


        /* Delete this ecow as needed. */
        if(static_ecow::delete_ecow == ecow_ptr)
	{
            EcoWDelete(n);
            UEWDoUpdateWindowMenus();
	}

        return(events_handled);
}


int EcoWManageAll(event_t *event)
{
        int i;
        int events_handled = 0;


        if(event == NULL)
            return(events_handled);


        for(i = 0; i < total_ecows; i++)
        {
            events_handled += EcoWManage(i, event);
        }


        return(events_handled);
}


void EcoWMap(int n)
{
        ecow_struct *ecow_ptr;


        if(EcoWIsAllocated(n))
            ecow_ptr = ecow[n];
        else
            return;


	ecow_ptr->map_state = 0;
	EcoWDraw(n, 0);


        return;
}

/*
 *	Procedure to map economy window and fetch values.
 */
void EcoWDoMapValues(int n, void *src, int obj_num)
{
	int i;
        ecow_struct *ecow_ptr;
	uew_struct *uew_ptr;
	char text[XSW_OBJ_NAME_MAX + 80];


        if(EcoWIsAllocated(n))
            ecow_ptr = ecow[n];
        else
            return;


	/* Reset has changes marker. */
	if(ecow_ptr->has_changes)
	    ecow_ptr->has_changes = False;


	/* Look for uew. */
	for(i = 0; i < total_uews; i++)
	{
	    if(uew[i] == NULL)
		continue;
	    if(uew[i] == (uew_struct *)src)
		break;
	}
	if(i < total_uews)
	{
	    uew_ptr = (uew_struct *)src;

	    ecow_ptr->src = src;
	    if(UEWIsObjectGarbage(uew_ptr, obj_num))
	    {
		ecow_ptr->src_obj_num = -1;
		ecow_ptr->src_obj_ptr = NULL;

		OSWSetWindowTitle(
		    ecow_ptr->toplevel,
		    ECOW_DEF_TITLE
		);
	    }
	    else
	    {
		ecow_ptr->src_obj_num = obj_num;
		ecow_ptr->src_obj_ptr = uew_ptr->object[obj_num];

		sprintf(
		    text,
		    "%s Economy",
		    UNVGetObjectFormalName(
			ecow_ptr->src_obj_ptr,
			ecow_ptr->src_obj_num
		    )
		);
                OSWSetWindowTitle(
                    ecow_ptr->toplevel,
                    text
                );
	    }
	}
	else
	{
	    ecow_ptr->src = NULL;

	    ecow_ptr->src_obj_num = -1;
	    ecow_ptr->src_obj_ptr = NULL;
	}


	/* Update list and reset prompts. */
	EcoWGetAllValues(ecow_ptr);


	/* Need to update all uew window menus. */
	UEWDoUpdateWindowMenus();


	/* Map window. */
	EcoWMap(n);


	return;
}


void EcoWUnmap(int n)
{
        ecow_struct *ecow_ptr;
 
 
        if(EcoWIsAllocated(n))
            ecow_ptr = ecow[n];
        else
            return;


        OSWUnmapWindow(ecow_ptr->toplevel);
        ecow_ptr->map_state = 0;

        OSWDestroyPixmap(&ecow_ptr->toplevel_buf);


        /* Schedual this ecow for deletion. */
        static_ecow::delete_ecow = ecow_ptr;


	return;
}


/*
 *      Destroys all allocated resources for economy
 *      window n.
 */
void EcoWDestroy(int n)
{
        ecow_struct *ecow_ptr;
        
            
        if(EcoWIsAllocated(n))
            ecow_ptr = ecow[n];
        else
            return;
          
        
        ecow_ptr->map_state = 0;
        ecow_ptr->is_in_focus = 0;

        ecow_ptr->has_changes = False;
        ecow_ptr->src = NULL;
	ecow_ptr->src_obj_num = -1;
        ecow_ptr->src_obj_ptr = NULL;
        
        if(IDC())
        {
	    PBtnDestroy(&ecow_ptr->close_btn);
            PBtnDestroy(&ecow_ptr->delete_obj_eco_btn);

	    PromptDestroy(&ecow_ptr->flags_prompt);
            PromptDestroy(&ecow_ptr->tax_general_prompt);
            PromptDestroy(&ecow_ptr->tax_friend_prompt);
            PromptDestroy(&ecow_ptr->tax_hostile_prompt);

            PromptDestroy(&ecow_ptr->prod_name_prompt);
            PromptDestroy(&ecow_ptr->prod_buy_prompt);
            PromptDestroy(&ecow_ptr->prod_sell_prompt);
            PromptDestroy(&ecow_ptr->prod_amount_prompt);
            PromptDestroy(&ecow_ptr->prod_max_prompt);

            PBtnDestroy(&ecow_ptr->delete_prod_btn);
            PBtnDestroy(&ecow_ptr->create_prod_btn);
 
	    CListDestroy(&ecow_ptr->list);


            OSWDestroyPixmap(&ecow_ptr->toplevel_buf);
            OSWDestroyWindow(&ecow_ptr->toplevel);
        }
        
        
        return;
}           



