// unvedit/wepw.cpp
/*
 *      Wepon window schedualed to be deleted.
 */


/*
                              Weapon Window

	Functions:

	int WepWGetLastSelected(wepw_struct *wepw_ptr)
	xsw_object_struct *WepWGetObjectPtr(
	        wepw_struct *wepw_ptr 
	)
	xsw_weapons_struct *WepWGetWeaponPtr(
	        wepw_struct *wepw_ptr,
	        int wep_num
	)
	int WepWIsAllocated(int n)

	int WepWAllocate()            
	void WepWDelete(int n)
	void WepWDeleteAll()

	int WepWGetAllValues(wepw_struct *wepw_ptr)
	int WepWGetValues(wepw_struct *wepw_ptr, int wep_num)
	int WepWSetValues(wepw_struct *wepw_ptr, int wep_num)

	int WepWCreateCB(void *ptr)
	int WepWDeleteCB(void *ptr)
	int WepWCloseCB(void *ptr)

	int WepWInit(int n, void *src_ptr)
	int WepWDraw(int n, int amount)
	int WepWManage(int n, event_t *event)
	int WepWManageAll(event_t *event)
	void WepWMap(int n)
	void WepWDoMapValues(int n, void *src, int obj_num)
	void WepWUnmap(int n)
	void WepWDestroy(int n)

	---

*/
/*
include <stdio.h>
#include <db.h>
#include <malloc.h>
#include <stdlib.h>
#include <string.h>
*/
#include "../include/string.h"

#include "../include/osw-x.h"
#include "../include/widget.h"

#include "../include/reality.h"
#include "../include/objects.h"
#include "../include/unvutil.h"

#include "uew.h"
#include "ue.h"
#include "wepw.h"

namespace static_wepw {
	wepw_struct *delete_wepw;
}

#ifndef MAX
#define MIN(a,b)        (((a) < (b)) ? (a) : (b))
#define MAX(a,b)	(((a) > (b)) ? (a) : (b))
#endif
/* #define MIN(a,b)	((a) < (b) ? (a) : (b)) */
/* #define MAX(a,b)	((a) > (b) ? (a) : (b)) */


/*
 *      Wepon window schedualed to be deleted.
 */
/*
static wepw_struct *delete_wepw;
*/

/*
 *	Gets the last selected row number or -1 on error nor
 *	no rows selected.
 */
int WepWGetLastSelected(wepw_struct *wepw_ptr)
{
	int n;
	colum_list_struct *list;


	if(wepw_ptr == NULL)
	    return(-1);

	list = &wepw_ptr->list;
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
 *	weapons window wepw_ptr.  Returns NULL on error.
 */
xsw_object_struct *WepWGetObjectPtr(
	wepw_struct *wepw_ptr
)
{
        int i, obj_num;
        uew_struct *uew_ptr; 


        if(wepw_ptr == NULL)
            return(NULL);


        uew_ptr = (uew_struct *)wepw_ptr->src;
        obj_num = wepw_ptr->src_obj_num;

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
 *	Get pointer to weapon wep_num.  Returns NULL on error or
 *	failed match.
 */
xsw_weapons_struct *WepWGetWeaponPtr(
        wepw_struct *wepw_ptr,
        int wep_num
)
{
	xsw_object_struct *obj_ptr;


	/* Get pointer to object. */
	obj_ptr = WepWGetObjectPtr(wepw_ptr);

	/* Is the object valid and non-garbage? */
	if(obj_ptr == NULL)
	    return(NULL);


	/* Is weapon wep_num allocated on object? */
	if((wep_num < 0) || (wep_num >= obj_ptr->total_weapons))
	    return(NULL);

	return(
	    obj_ptr->weapons[wep_num]
	);
}


/*
 *	Checks if wepon window n is allocated.
 */
int WepWIsAllocated(int n)
{
        if((wepw == NULL) ||
           (n < 0) || (n >= total_wepws)
        )
            return(0);
        else if(wepw[n] == NULL)
            return(0);
        else
            return(1);
}          


/*
 *      Allocates a new universe header window, returning its
 *      index number.
 */      
int WepWAllocate()
{
        int i, n;

         
        if(total_wepws < 0)
            total_wepws = 0;

        for(i = 0; i < total_wepws; i++)
        {   
            if(wepw[i] == NULL)
                break;
        }
        if(i < total_wepws)
        {
            n = i;
        }
        else
        {
            n = total_wepws;
            total_wepws++;
            wepw = (wepw_struct **)realloc(
                wepw,  
                total_wepws * sizeof(wepw_struct *)
            );
            if(wepw == NULL)
            {
                total_wepws = 0;
                return(-1);
            }
        }
 
        wepw[n] = (wepw_struct *)calloc(
            1,   
            sizeof(wepw_struct)
        );
        if(wepw[n] == NULL)
        {
            return(-1);   
        }


        return(n);
}


/*
 *	 Deletes weapon window.
 */     
void WepWDelete(int n)
{
        if(!WepWIsAllocated(n))
            return;
 
        
        /* Deallocate resources. */
        WepWDestroy(n);
        
            
        /* Free structure itself. */
        free(wepw[n]);
        wepw[n] = NULL;


        return;
}

/*
 *	Deletes all weapon windows.
 */
void WepWDeleteAll()
{
        int i;


        for(i = 0; i < total_wepws; i++)
            WepWDelete(i);

        free(wepw);
        wepw = NULL;

        total_wepws = 0;


        return;
}


/*
 *	Procedure to fetch all values from object specified in
 *	in weapon window wepw_ptr and put them into the prompts
 *	and list window.  Prompts will be redrawn if mapped.
 */
int WepWGetAllValues(wepw_struct *wepw_ptr)
{
	int i;
	colum_list_struct *list;
	xsw_object_struct *obj_ptr;
	char text[1024];


	if(wepw_ptr == NULL)
	    return(-1);

	list = &wepw_ptr->list;


	/* Delete all rows in list. */
	CListUnselectAllRows(list);
	CListDeleteAllRows(list);


	/* Get pointer to object referanced in weapon window. */
	obj_ptr = WepWGetObjectPtr(wepw_ptr);

	if(obj_ptr == NULL)
	{
	    /* No valid object. */

	    /* Reset referance to object. */
	    wepw_ptr->src_obj_num = -1;
	    wepw_ptr->src_obj_ptr = NULL;

	    /* Do not update list. */

	    /* Redraw list as needed. */
	    if(list->map_state)
		CListDraw(list, CL_DRAW_AMOUNT_COMPLETE);
	}
	else
	{
	    /* Add list rows. */
	    for(i = 0; i < obj_ptr->total_weapons; i++)
	    {
		if(obj_ptr->weapons[i] == NULL)
		    continue;

		if(CListAddRow(list, i))
		    break;

		sprintf(text, "Weapon #%i", i + 1);
		CListAddItem(
		    list,
		    text,
		    OSWQueryCurrentFont(),
		    widget_global.editable_text_pix,
		    0,		/* Attributes. */
		    i		/* Row number. */
		);
	    }
            /* Redraw list as needed. */
            if(list->map_state)
                CListDraw(list, CL_DRAW_AMOUNT_COMPLETE);
	}


	/* Reset prompts. */
	PromptSetS(&wepw_ptr->flags_prompt, "");
        PromptSetS(&wepw_ptr->ocs_code_prompt, "");
        PromptSetS(&wepw_ptr->emission_type_prompt, "");
        PromptSetS(&wepw_ptr->amount_prompt, "");
        PromptSetS(&wepw_ptr->max_prompt, "");
        PromptSetS(&wepw_ptr->power_prompt, "");
        PromptSetS(&wepw_ptr->range_prompt, "");
        PromptSetS(&wepw_ptr->create_power_prompt, "");
        PromptSetS(&wepw_ptr->delay_prompt, "");
        PromptSetS(&wepw_ptr->use_sound_code_prompt, "");
        PromptSetS(&wepw_ptr->fire_sound_code_prompt, "");
        PromptSetS(&wepw_ptr->hit_sound_code_prompt, "");
        PromptSetS(&wepw_ptr->recover_sound_code_prompt, "");

	return(0);
}

/*
 *	Fetches values from weapon number wep_num and puts
 *	them into the prompts.  (does not update the list window).
 */
int WepWGetValues(wepw_struct *wepw_ptr, int wep_num)
{
	xsw_weapons_struct *wep_ptr;
        char *strptr;
	const int len = 256;
	char text[len];


        if(wepw_ptr == NULL)
            return(-1);

	wep_ptr = WepWGetWeaponPtr(wepw_ptr, wep_num);
	if(wep_ptr == NULL)
	    return(-1);


	/* Get values from weapon and put into prompts. */

	*text = '\0';
	if(wep_ptr->flags & XSW_WEP_FLAG_NO_FIRE_SOUND)
	{
	    straddflag(text, XSW_WEP_FLAG_NAME_NO_FIRE_SOUND, '|', len);
	}
	if(wep_ptr->flags & XSW_WEP_FLAG_FIXED)
	{
            straddflag(text, XSW_WEP_FLAG_NAME_FIXED, '|', len);
	}
        PromptSetS(&wepw_ptr->flags_prompt, text);

        PromptSetI(&wepw_ptr->ocs_code_prompt, wep_ptr->ocs_code);

	switch(wep_ptr->emission_type)
	{
	  case WEPEMISSION_STREAM:
	    strptr = WEPEMISSION_NAME_STREAM;
            break;

	  case WEPEMISSION_PROJECTILE:
	    strptr = WEPEMISSION_NAME_PROJECTILE;
            break;

	  case WEPEMISSION_PULSE:
	    strptr = WEPEMISSION_NAME_PULSE;
	    break;

	  default:	/* Default to stream. */
            strptr = WEPEMISSION_NAME_STREAM;
	    break;
	}
        PromptSetS(&wepw_ptr->emission_type_prompt, strptr);

        PromptSetI(&wepw_ptr->amount_prompt, wep_ptr->amount);
        PromptSetI(&wepw_ptr->max_prompt, wep_ptr->max);
        PromptSetF(&wepw_ptr->power_prompt, wep_ptr->power);
        PromptSetI(&wepw_ptr->range_prompt, wep_ptr->range);
        PromptSetF(&wepw_ptr->create_power_prompt, wep_ptr->create_power);
        PromptSetI(&wepw_ptr->delay_prompt, wep_ptr->delay);
        PromptSetI(&wepw_ptr->range_prompt, wep_ptr->range);
        PromptSetI(&wepw_ptr->use_sound_code_prompt, wep_ptr->use_sound_code);
        PromptSetI(&wepw_ptr->fire_sound_code_prompt, wep_ptr->fire_sound_code);
        PromptSetI(&wepw_ptr->hit_sound_code_prompt, wep_ptr->hit_sound_code);
        PromptSetI(&wepw_ptr->recover_sound_code_prompt, wep_ptr->recover_sound_code);

	return(0);
}

/*
 *	Sets the values in the prompts of the weapon window wepw_ptr
 *	into the object's weapon wep_num.
 *
 *	If wep_num does not exist on object it will *not* be created.
 */
int WepWSetValues(wepw_struct *wepw_ptr, int wep_num)
{
	int i;
	char *strptr;
        xsw_weapons_struct *wep_ptr;


        if(wepw_ptr == NULL)
            return(-1); 

        wep_ptr = WepWGetWeaponPtr(wepw_ptr, wep_num);
        if(wep_ptr == NULL)
            return(-1);


        /* Get values from prompts and set to weapon. */

	strptr = PromptGetS(&wepw_ptr->flags_prompt);
	wep_ptr->flags = 0;
	if(strcasestr(strptr, XSW_WEP_FLAG_NAME_NO_FIRE_SOUND))
	    wep_ptr->flags |= XSW_WEP_FLAG_NO_FIRE_SOUND;
	if(strcasestr(strptr, XSW_WEP_FLAG_NAME_FIXED))
            wep_ptr->flags |= XSW_WEP_FLAG_FIXED;

	wep_ptr->ocs_code = PromptGetI(&wepw_ptr->ocs_code_prompt);

	strptr = PromptGetS(&wepw_ptr->emission_type_prompt);
	if(strptr == NULL)
	{
	    i = WEPEMISSION_STREAM;
	}
	else
	{
	    if(!strcasecmp(strptr, WEPEMISSION_NAME_STREAM))
		i = WEPEMISSION_STREAM;
	    else if(!strcasecmp(strptr, WEPEMISSION_NAME_PROJECTILE))
                i = WEPEMISSION_PROJECTILE;
            else if(!strcasecmp(strptr, WEPEMISSION_NAME_PULSE))
                i = WEPEMISSION_PULSE;
	    else
                i = WEPEMISSION_STREAM;
	}
	wep_ptr->emission_type = i;

	wep_ptr->amount = PromptGetI(&wepw_ptr->amount_prompt);
	wep_ptr->max = PromptGetI(&wepw_ptr->max_prompt);
	wep_ptr->power = PromptGetF(&wepw_ptr->power_prompt);
	wep_ptr->range = PromptGetL(&wepw_ptr->range_prompt);
	wep_ptr->create_power = PromptGetF(&wepw_ptr->create_power_prompt);
	wep_ptr->delay = PromptGetL(&wepw_ptr->delay_prompt);
        wep_ptr->use_sound_code = PromptGetL(&wepw_ptr->use_sound_code_prompt);
        wep_ptr->fire_sound_code = PromptGetL(&wepw_ptr->fire_sound_code_prompt);
        wep_ptr->hit_sound_code = PromptGetL(&wepw_ptr->hit_sound_code_prompt);
        wep_ptr->recover_sound_code = PromptGetL(&wepw_ptr->recover_sound_code_prompt);

	return(0);
}


/*
 *	Weapons list callback.
 */
int WepWListCB(void *ptr)
{



	return(0);
}

/*
 *	Create button callback.
 */
int WepWCreateCB(void *ptr)
{
        int i, wepw_num;
        wepw_struct *wepw_ptr = NULL;
	colum_list_struct *list;
	xsw_object_struct *obj_ptr;
	char text[256];


        if(ptr == NULL)
            return(-1);


        for(i = 0; i < total_wepws; i++)
        {
            if(wepw[i] == NULL)
                continue;

            if(wepw[i] == (wepw_struct *)ptr)
            {
                wepw_ptr = (wepw_struct *)ptr;
                break;
            }
        }
        if(wepw_ptr == NULL)
            return(-1);
        else
            wepw_num = i;

        list = &wepw_ptr->list;


	/* Apply values to currently selected weapon. */
	if(list->total_sel_rows > 0)
	{
	    WepWSetValues(
		wepw_ptr,
		list->sel_row[list->total_sel_rows - 1]
	    );
	}


	/* Allocate a new weapon on object. */
	obj_ptr = WepWGetObjectPtr(wepw_ptr);
	if(obj_ptr != NULL)
	{
	    if(obj_ptr->total_weapons >= MAX_WEAPONS)
	    {
		sprintf(text,
"Maximum of %i weapons exists for this object, cannot\n\
create additional weapon.",
		    MAX_WEAPONS
		);
		printdw(&dialog, text);
	    }
	    else
	    {
		/* Allocate a new weapon. */
		i = obj_ptr->total_weapons;
		obj_ptr->total_weapons++;
		obj_ptr->weapons = (xsw_weapons_struct **)realloc(
		    obj_ptr->weapons,
		    obj_ptr->total_weapons * sizeof(xsw_weapons_struct *)
		);
		if(obj_ptr->weapons == NULL)
		{
		    obj_ptr->total_weapons = 0;
		    return(-1);
		}

		obj_ptr->weapons[i] = (xsw_weapons_struct *)calloc(
		    1,
		    sizeof(xsw_weapons_struct)
		);

		/*   Redget all values, update list, and unselect
		 *   rows.
		 */
		WepWGetAllValues(wepw_ptr);
	    }
	}

        /* Update has changes marker. */
        if(!wepw_ptr->has_changes)
            wepw_ptr->has_changes = True;


	/* Redraw list as needed. */
	if(list->map_state)
	    CListDraw(list, CL_DRAW_AMOUNT_COMPLETE);


	return(0);
}

/*      
 *      Delete button callback.
 */
int WepWDeleteCB(void *ptr)
{
        int i, n, sel_wep_num;

	int wepw_num;
        wepw_struct *wepw_ptr = NULL;

        colum_list_struct *list;
        xsw_object_struct *obj_ptr;


        if(ptr == NULL)  
            return(-1);


        for(i = 0; i < total_wepws; i++)
        {
            if(wepw[i] == NULL)
                continue;

            if(wepw[i] == (wepw_struct *)ptr)
            {
                wepw_ptr = (wepw_struct *)ptr;
                break;
            }   
        }       
        if(wepw_ptr == NULL)
            return(-1);
        else
            wepw_num = i;

	/* Get pointer to list. */
        list = &wepw_ptr->list;


        /* Nothing selected? */
        if(list->total_sel_rows <= 0)
            return(0);


	/* Get pointer to object (must be successful). */
        obj_ptr = WepWGetObjectPtr(wepw_ptr);
	if(obj_ptr == NULL)
	    return(-1);


        /*   Do not apply values to currently selected weapon,
	 *   since it will be deleted.
	 */

	sel_wep_num = list->sel_row[list->total_sel_rows - 1];
	if((sel_wep_num < 0) || (sel_wep_num >= obj_ptr->total_weapons))
	    return(-1);

	/* Deallocate selected weapon on object. */
	if(obj_ptr->weapons != NULL)
	{
	    free(obj_ptr->weapons[sel_wep_num]);

	    /* Shift pointer array on object. */
	    for(i = sel_wep_num, n = obj_ptr->total_weapons - 1;
                i < n;
                i++
	    )
		obj_ptr->weapons[i] = obj_ptr->weapons[i + 1];


	    /* Reallocate weapon pointers. */
	    obj_ptr->total_weapons--;

	    if(obj_ptr->total_weapons > 0)
	    {
		obj_ptr->weapons = (xsw_weapons_struct **)realloc(
                    obj_ptr->weapons,
                    obj_ptr->total_weapons * sizeof(xsw_weapons_struct *)
                );
                if(obj_ptr->weapons == NULL)
		{
		    obj_ptr->total_weapons = 0;
		}
	    }
	    else
	    {
		free(obj_ptr->weapons);
		obj_ptr->weapons = NULL;

		obj_ptr->total_weapons = 0;
	    }
	}
	if(obj_ptr->total_weapons < 0)
	    obj_ptr->total_weapons = 0;


	/* Update has changes marker. */
	if(!wepw_ptr->has_changes)
	    wepw_ptr->has_changes = True;


        /*   Redget all values, update list, and unselect
         *   rows. 
         */
        WepWGetAllValues(wepw_ptr);


        /* Redraw list as needed. */
        if(list->map_state)
            CListDraw(list, CL_DRAW_AMOUNT_COMPLETE);




        return(0);
}

/*
 *	Close button callback.
 */
int WepWCloseCB(void *ptr)
{ 
        int i, wepw_num;
        wepw_struct *wepw_ptr = NULL;
	uew_struct *uew_ptr;


        if(ptr == NULL)
            return(-1);


        for(i = 0; i < total_wepws; i++)
        {    
            if(wepw[i] == NULL)
                continue;
        
            if(wepw[i] == (wepw_struct *)ptr)
	    {
                wepw_ptr = (wepw_struct *)ptr;
		break;
	    }
        }
        if(wepw_ptr == NULL)
            return(-1);
        else
            wepw_num = i;


	/* Apply changes on currently selected weapon. */
	if(wepw_ptr->list.total_sel_rows > 0)
	{
	    i = wepw_ptr->list.sel_row[
		wepw_ptr->list.total_sel_rows - 1
	    ];
	    WepWSetValues(
		wepw_ptr,
		i
	    );
	}

	/* Unmap. */
        WepWUnmap(wepw_num);


        /* Update has changes marker on corresponding uew. */
        if(wepw_ptr->has_changes)
        {
	    /* Mark uew to have changes too. */
	    for(i = 0, uew_ptr = NULL; i < total_uews; i++)
	    {
		if(uew[i] == NULL)
		    continue;

		if(uew[i] == (uew_struct *)wepw_ptr->src)
		    uew_ptr = uew[i];
	    }
	    if(uew_ptr != NULL)
	    {
		if(!uew_ptr->has_changes)
		    uew_ptr->has_changes = True;
	    }

	    wepw_ptr->has_changes = False;
	}


        return(0);
} 


/*
 *      Initializes a weapons window.
 */
int WepWInit(int n, void *src_ptr)
{
	int x, y;
        char hotkey[PBTN_MAX_HOTKEYS];
        win_attr_t wattr;
        wepw_struct *wepw_ptr;



        if(WepWIsAllocated(n))  
            wepw_ptr = wepw[n];
        else
            return(-1);
            
            
        wepw_ptr->map_state = 0;
        wepw_ptr->is_in_focus = 0; 
        wepw_ptr->x = 0;
        wepw_ptr->y = 0;
        wepw_ptr->width = WEPW_WIDTH;
        wepw_ptr->height = WEPW_HEIGHT;
            
        wepw_ptr->has_changes = False;
        wepw_ptr->src = src_ptr;

        /* ******************************************************** */
        /* Create toplevel. */
        if(
            OSWCreateWindow(
                &wepw_ptr->toplevel,
                osw_gui[0].root_win,
                wepw_ptr->x, wepw_ptr->y,
                wepw_ptr->width, wepw_ptr->height
            )
        )
            return(-1);

        OSWSetWindowWMProperties(
            wepw_ptr->toplevel,
            WEPW_DEF_TITLE,
            WEPW_DEF_ICON_TITLE,
            ((ue_image.unvedit_icon_pm == 0) ?
                widget_global.std_icon_pm : ue_image.unvedit_icon_pm),
            False,                      /* Let WM set coordinates? */
            wepw_ptr->x, wepw_ptr->y,
            wepw_ptr->width, wepw_ptr->height,
            wepw_ptr->width, wepw_ptr->height,
            WindowFrameStyleFixed,
            NULL, 0
        );
        
        OSWSetWindowInput(
            wepw_ptr->toplevel,
            OSW_EVENTMASK_TOPLEVEL   
        );
                
        WidgetCenterWindow(wepw_ptr->toplevel, WidgetCenterWindowToRoot);
        OSWGetWindowAttributes(wepw_ptr->toplevel, &wattr);
        wepw_ptr->x = wattr.x;
        wepw_ptr->y = wattr.y; 
        wepw_ptr->width = wattr.width;
        wepw_ptr->height = wattr.height;


	/* Weapons list. */
        if(
            CListInit(
                &wepw_ptr->list,
                wepw_ptr->toplevel,
                10, 10,
                MAX(
		    (int)wepw_ptr->width - WEPW_BUTTON_WIDTH - 30,
		    50
		),
		100,
                (void *)&wepw_ptr,
                WepWListCB
            )
        )
            return(-1);
	wepw_ptr->list.option = 0;

        CListAddHeading(
            &wepw_ptr->list,
            "Weapon",
            OSWQueryCurrentFont(),
            widget_global.normal_text_pix,
            0,
            0
        );

	/* Create button. */
        if( 
            PBtnInit(
                &wepw_ptr->create_wep_btn,
                wepw_ptr->toplevel,
                (int)wepw_ptr->width - WEPW_BUTTON_WIDTH - 10,
		(10 * 1) + (WEPW_BUTTON_HEIGHT * 0),
                WEPW_BUTTON_WIDTH, WEPW_BUTTON_HEIGHT,
                "Create",
                PBTN_TALIGN_CENTER,
                NULL,
                (void *)wepw_ptr,
                WepWCreateCB
            )
        )
            return(-1);

        /* Delete button. */
        if(
            PBtnInit(
                &wepw_ptr->delete_wep_btn,
                wepw_ptr->toplevel,
                (int)wepw_ptr->width - WEPW_BUTTON_WIDTH - 10,  
                (10 * 2) + (WEPW_BUTTON_HEIGHT * 1),
                WEPW_BUTTON_WIDTH, WEPW_BUTTON_HEIGHT,
                "Delete",
                PBTN_TALIGN_CENTER,
                NULL,
                (void *)wepw_ptr,
                WepWDeleteCB
            )
        )
            return(-1);


	/* Flags prompt. */
	x = WEPW_MARGIN;
	y = 120;

        if(PromptInit(
            &wepw_ptr->flags_prompt,
            wepw_ptr->toplevel,
            x, y,
            (int)wepw_ptr->width - (2 * WEPW_MARGIN),
            WEPW_PROMPT_HEIGHT,
	    PROMPT_STYLE_FLUSHED,
            "Flags:",
            1024,
            0,
            NULL
        ))
            return(-1);

	/* Object create script code. */
        y += WEPW_PROMPT_HEIGHT;
	if(PromptInit(
            &wepw_ptr->ocs_code_prompt,
            wepw_ptr->toplevel,
            x, y,
            (int)wepw_ptr->width - (2 * WEPW_MARGIN),
            WEPW_PROMPT_HEIGHT,
            PROMPT_STYLE_FLUSHED,
            "Object Create Script Code:",
            80,
            0,
            NULL
        ))
            return(-1);

	/* Emission type. */
	y += WEPW_PROMPT_HEIGHT;
        if(PromptInit(
            &wepw_ptr->emission_type_prompt,
            wepw_ptr->toplevel,
            x, y,
            (int)wepw_ptr->width - (2 * WEPW_MARGIN),
            WEPW_PROMPT_HEIGHT,
            PROMPT_STYLE_FLUSHED,
            "Emission Type:",
            256,
            0,
            NULL
        ))
            return(-1);

	/* Amount. */
        y += WEPW_PROMPT_HEIGHT;
        if(PromptInit(
            &wepw_ptr->amount_prompt,
            wepw_ptr->toplevel,
            x, y,
            static_cast<unsigned int>(((int)wepw_ptr->width / 2) - (1.5 * WEPW_MARGIN)),
            WEPW_PROMPT_HEIGHT,  
            PROMPT_STYLE_FLUSHED,
            "Amount:",
            80,
            0,  
            NULL
        ))
	    return(-1);

	/* Max. */
        if(PromptInit(
            &wepw_ptr->max_prompt,
            wepw_ptr->toplevel,
	    static_cast<int>(x + ((int)wepw_ptr->width / 2) + (0.5 * WEPW_MARGIN)),
	    y,
            ((int)wepw_ptr->width / 2) - (2 * WEPW_MARGIN),
            WEPW_PROMPT_HEIGHT,
            PROMPT_STYLE_FLUSHED,
            "Max:",
            80,
            0,
            NULL 
        ))
	    return(-1);

	/* Power. */
        y += WEPW_PROMPT_HEIGHT;
        if(PromptInit(
            &wepw_ptr->power_prompt,
            wepw_ptr->toplevel,
            x, y,
            static_cast<unsigned int>(((int)wepw_ptr->width / 2) - (1.5 * WEPW_MARGIN)),
            WEPW_PROMPT_HEIGHT,
            PROMPT_STYLE_FLUSHED,
            "Power:",
            80,
            0,
            NULL
        ))
	    return(-1);

        /* Create power. */
        if(PromptInit(
            &wepw_ptr->create_power_prompt,
            wepw_ptr->toplevel,
            static_cast<int>(x + ((int)wepw_ptr->width / 2) + (0.5 * WEPW_MARGIN)),
	    y,
	    ((int)wepw_ptr->width / 2) - (2 * WEPW_MARGIN),
            WEPW_PROMPT_HEIGHT,
            PROMPT_STYLE_FLUSHED,
            "Create Power:",
            80,
            0,
            NULL
        ))
            return(-1);

	/* Range (for stream and sphere weapons). */
        y += WEPW_PROMPT_HEIGHT;
        if(PromptInit(
            &wepw_ptr->range_prompt,
            wepw_ptr->toplevel,
            x, y,
            static_cast<unsigned int>(((int)wepw_ptr->width / 2) - (1.5 * WEPW_MARGIN)),
            WEPW_PROMPT_HEIGHT,
            PROMPT_STYLE_FLUSHED,
            "Range (su):",
            80,
            0,
            NULL
        ))
            return(-1);

	/* Fire sound code. */
        if(PromptInit(
            &wepw_ptr->fire_sound_code_prompt,
            wepw_ptr->toplevel,
            static_cast<int>(x + ((int)wepw_ptr->width / 2) + (0.5 * WEPW_MARGIN)),
	    y,
            ((int)wepw_ptr->width / 2) - (2 * WEPW_MARGIN),
            WEPW_PROMPT_HEIGHT,
            PROMPT_STYLE_FLUSHED,
            "Fire Sound Code:",
            80,
            0,
            NULL
        ))
            return(-1);

	/* Delay. */
        y += WEPW_PROMPT_HEIGHT;
        if(PromptInit(
            &wepw_ptr->delay_prompt,
            wepw_ptr->toplevel,
            x, y,
            static_cast<unsigned int>(((int)wepw_ptr->width / 2) - (1.5 * WEPW_MARGIN)),
            WEPW_PROMPT_HEIGHT,
            PROMPT_STYLE_FLUSHED,
            "Delay (ms):",
            80,
            0,
            NULL
        ))
            return(-1);


	/* Link prompts togeather. */
        wepw_ptr->flags_prompt.next = &wepw_ptr->ocs_code_prompt;
	wepw_ptr->flags_prompt.prev = &wepw_ptr->delay_prompt;

	wepw_ptr->ocs_code_prompt.next = &wepw_ptr->emission_type_prompt;
	wepw_ptr->ocs_code_prompt.prev = &wepw_ptr->delay_prompt;

        wepw_ptr->emission_type_prompt.next = &wepw_ptr->amount_prompt;
        wepw_ptr->emission_type_prompt.prev = &wepw_ptr->ocs_code_prompt;

        wepw_ptr->amount_prompt.next = &wepw_ptr->max_prompt;
        wepw_ptr->amount_prompt.prev = &wepw_ptr->emission_type_prompt;

        wepw_ptr->max_prompt.next = &wepw_ptr->power_prompt;
        wepw_ptr->max_prompt.prev = &wepw_ptr->amount_prompt;

        wepw_ptr->power_prompt.next = &wepw_ptr->create_power_prompt;
        wepw_ptr->power_prompt.prev = &wepw_ptr->max_prompt;

        wepw_ptr->create_power_prompt.next = &wepw_ptr->range_prompt;
        wepw_ptr->create_power_prompt.prev = &wepw_ptr->power_prompt;

        wepw_ptr->range_prompt.next = &wepw_ptr->fire_sound_code_prompt;
        wepw_ptr->range_prompt.prev = &wepw_ptr->create_power_prompt;

        wepw_ptr->fire_sound_code_prompt.next = &wepw_ptr->delay_prompt;
        wepw_ptr->fire_sound_code_prompt.prev = &wepw_ptr->range_prompt;

        wepw_ptr->delay_prompt.next = &wepw_ptr->flags_prompt;
        wepw_ptr->delay_prompt.prev = &wepw_ptr->fire_sound_code_prompt;


        /* Close button. */
        if(
            PBtnInit(
                &wepw_ptr->close_btn,
                wepw_ptr->toplevel,  
                (int)wepw_ptr->width - WEPW_BUTTON_WIDTH - 10,
                (int)wepw_ptr->height - 10 - WEPW_BUTTON_HEIGHT,
                WEPW_BUTTON_WIDTH, WEPW_BUTTON_HEIGHT,
                "Close",
                PBTN_TALIGN_CENTER,
                NULL,
                (void *)wepw_ptr,
                WepWCloseCB
            )
        )
            return(-1);
        hotkey[0] = '\n';
	hotkey[1] = 0x1b;
        hotkey[2] = '\0';
        PBtnSetHotKeys(&wepw_ptr->close_btn, hotkey);


        /* Need to update window menus on uews. */
        UEWDoUpdateWindowMenus();


        return(0);
}


int WepWDraw(int n, int amount)
{
        win_t w;
        pixmap_t pixmap;
        wepw_struct *wepw_ptr;
        win_attr_t wattr;


        if(WepWIsAllocated(n))
            wepw_ptr = wepw[n];
        else
            return(-1);  
                
                
        /* Map as needed. */
        if(!wepw_ptr->map_state)
        {
            OSWMapRaised(wepw_ptr->toplevel);

	    CListMap(&wepw_ptr->list);

            PBtnMap(&wepw_ptr->create_wep_btn);
            PBtnMap(&wepw_ptr->delete_wep_btn); 

            PromptMap(&wepw_ptr->flags_prompt);
	    PromptMap(&wepw_ptr->ocs_code_prompt);
            PromptMap(&wepw_ptr->emission_type_prompt);
            PromptMap(&wepw_ptr->amount_prompt);
            PromptMap(&wepw_ptr->max_prompt);
            PromptMap(&wepw_ptr->power_prompt);
            PromptMap(&wepw_ptr->range_prompt);
            PromptMap(&wepw_ptr->create_power_prompt);
            PromptMap(&wepw_ptr->delay_prompt);
            PromptMap(&wepw_ptr->use_sound_code_prompt);
            PromptMap(&wepw_ptr->fire_sound_code_prompt);
            PromptMap(&wepw_ptr->hit_sound_code_prompt);
            PromptMap(&wepw_ptr->recover_sound_code_prompt);

            PBtnMap(&wepw_ptr->close_btn);

            wepw_ptr->map_state = 1;
	}


	if(wepw_ptr->toplevel_buf == 0)
	{
	    OSWGetWindowAttributes(wepw_ptr->toplevel, &wattr);
	    if(OSWCreatePixmap(
		&wepw_ptr->toplevel_buf,
		wattr.width, wattr.height
	    ))
		return(-1);
	}


	if(1)
	{
            w = wepw_ptr->toplevel;
            pixmap = wepw_ptr->toplevel_buf;

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
		    (int)wattr.height - 25 - WEPW_BUTTON_HEIGHT,
                    wattr.width,
                    (int)wattr.height - 25 - WEPW_BUTTON_HEIGHT
		);
                OSWSetFgPix(widget_global.surface_highlight_pix);
                OSWDrawLine(
                    pixmap,
                    0,
                    (int)wattr.height - 24 - WEPW_BUTTON_HEIGHT,
                    wattr.width,
                    (int)wattr.height - 24 - WEPW_BUTTON_HEIGHT
                );  

            }



            OSWPutBufferToWindow(w, pixmap);
	}


	return(0);
}


int WepWManage(int n, event_t *event)
{
	int i, p;
        keycode_t keycode;
        int events_handled = 0;
        wepw_struct *wepw_ptr;
	colum_list_struct *list;


        static_wepw::delete_wepw = NULL;


        if(WepWIsAllocated(n))
            wepw_ptr = wepw[n];
        else
            return(events_handled);


        if(event == NULL)
	    return(events_handled);

	if(!wepw_ptr->map_state &&
	   (event->type != MapNotify)
        )
            return(events_handled);


        switch(event->type)
        {
          /* ******************************************************* */
          case KeyPress:
            if(!wepw_ptr->is_in_focus)
                break;

            keycode = event->xkey.keycode;

            /* Enter. */
            if((keycode == osw_keycode.enter) ||
               (keycode == osw_keycode.np_enter)
            )
            {
                WepWCloseCB((void *)wepw_ptr);

                /* Delete this wepw as needed. */
                if(static_wepw::delete_wepw == wepw_ptr)
                {
                    WepWDelete(n);
                    UEWDoUpdateWindowMenus();
                }

                events_handled++;
                return(events_handled);
            }
	    break;

          /* ******************************************************* */
          case KeyRelease:
            if(!wepw_ptr->is_in_focus) 
                break;

            break;

          /* ******************************************************* */
          case Expose:
            if(event->xany.window == wepw_ptr->toplevel)
            {
                events_handled++;
            }
            break;

          /* ******************************************************* */
          case FocusIn:
            if(event->xany.window == wepw_ptr->toplevel)
            {
                wepw_ptr->is_in_focus = 1;

                events_handled++;
                return(events_handled);
            }
            break;

          /* ******************************************************* */
          case FocusOut:
            if(event->xany.window == wepw_ptr->toplevel)
            {
                wepw_ptr->is_in_focus = 0;

                events_handled++;
                return(events_handled);
            }   
            break;
         
          /* ******************************************************* */
          case ClientMessage:
            if(OSWIsEventDestroyWindow(wepw_ptr->toplevel, event))
            {
                WepWCloseCB((void *)wepw_ptr);

                /* Delete this wepw as needed. */
                if(static_wepw::delete_wepw == wepw_ptr)
		{
                    WepWDelete(n);
                    UEWDoUpdateWindowMenus();
		}

                events_handled++;
                return(events_handled);
            }
            break;
	}

        if(events_handled > 0)
        {
            WepWDraw(n, 0);
        }


	/* Manage widgets. */

	/* Weapons list. */
        if(events_handled == 0)
	{
	    list = &wepw_ptr->list;

	    if(list->total_sel_rows > 0)
		p = list->sel_row[list->total_sel_rows - 1];
	    else
		p = -1;

            events_handled += CListManage(
		&wepw_ptr->list,
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
		    WepWSetValues(wepw_ptr, p);
		    WepWGetValues(wepw_ptr, i);
		}
	    }
	}            

        if(events_handled == 0)
            events_handled += PBtnManage(&wepw_ptr->create_wep_btn, event);
        if(events_handled == 0)
            events_handled += PBtnManage(&wepw_ptr->delete_wep_btn, event);
        if(events_handled == 0)
            events_handled += PBtnManage(&wepw_ptr->close_btn, event);

        if(events_handled == 0)
            events_handled += PromptManage(&wepw_ptr->flags_prompt, event);
        if(events_handled == 0)
            events_handled += PromptManage(&wepw_ptr->ocs_code_prompt, event);
        if(events_handled == 0)
            events_handled += PromptManage(&wepw_ptr->emission_type_prompt, event);
        if(events_handled == 0)
            events_handled += PromptManage(&wepw_ptr->amount_prompt, event);
        if(events_handled == 0)
            events_handled += PromptManage(&wepw_ptr->max_prompt, event);
        if(events_handled == 0)
            events_handled += PromptManage(&wepw_ptr->power_prompt, event);
        if(events_handled == 0)
            events_handled += PromptManage(&wepw_ptr->range_prompt, event);
        if(events_handled == 0)
            events_handled += PromptManage(&wepw_ptr->create_power_prompt, event);
        if(events_handled == 0)
            events_handled += PromptManage(&wepw_ptr->delay_prompt, event);
        if(events_handled == 0)
            events_handled += PromptManage(&wepw_ptr->use_sound_code_prompt, event);
        if(events_handled == 0)
            events_handled += PromptManage(&wepw_ptr->fire_sound_code_prompt, event);
        if(events_handled == 0)
            events_handled += PromptManage(&wepw_ptr->hit_sound_code_prompt, event);
        if(events_handled == 0)
            events_handled += PromptManage(&wepw_ptr->recover_sound_code_prompt, event);


        /* Delete this wepw as needed. */
        if(static_wepw::delete_wepw == wepw_ptr)
	{
            WepWDelete(n);
            UEWDoUpdateWindowMenus();
	}

        return(events_handled);
}


int WepWManageAll(event_t *event)
{
        int i;
        int events_handled = 0;


        if(event == NULL)
            return(events_handled);


        for(i = 0; i < total_wepws; i++)
        {
            events_handled += WepWManage(i, event);
        }


        return(events_handled);
}


void WepWMap(int n)
{
        wepw_struct *wepw_ptr;


        if(WepWIsAllocated(n))
            wepw_ptr = wepw[n];
        else
            return;


	wepw_ptr->map_state = 0;
	WepWDraw(n, 0);


        return;
}

/*
 *	Procedure to map weapon window and fetch values.
 */
void WepWDoMapValues(int n, void *src, int obj_num)
{
	int i;
        wepw_struct *wepw_ptr;
	uew_struct *uew_ptr;
	char text[XSW_OBJ_NAME_MAX + 80];


        if(WepWIsAllocated(n))
            wepw_ptr = wepw[n];
        else
            return;


	/* Reset wepw has changes marker. */
	if(wepw_ptr->has_changes)
	    wepw_ptr->has_changes = False;


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

	    wepw_ptr->src = src;
	    if(UEWIsObjectGarbage(uew_ptr, obj_num))
	    {
		wepw_ptr->src_obj_num = -1;
		wepw_ptr->src_obj_ptr = NULL;

		OSWSetWindowTitle(
		    wepw_ptr->toplevel,
		    WEPW_DEF_TITLE
		);
	    }
	    else
	    {
		wepw_ptr->src_obj_num = obj_num;
		wepw_ptr->src_obj_ptr = uew_ptr->object[obj_num];

		sprintf(
		    text,
		    "%s Weapons",
		    UNVGetObjectFormalName(
			wepw_ptr->src_obj_ptr,
			wepw_ptr->src_obj_num
		    )
		);
                OSWSetWindowTitle(
                    wepw_ptr->toplevel,
                    text
                );
	    }
	}
	else
	{
	    wepw_ptr->src = NULL;

	    wepw_ptr->src_obj_num = -1;
	    wepw_ptr->src_obj_ptr = NULL;
	}


	/* Update list and reset prompts. */
	WepWGetAllValues(wepw_ptr);


	/* Need to update all uew window menus. */
	UEWDoUpdateWindowMenus();


	/* Map window. */
	WepWMap(n);


	return;
}


void WepWUnmap(int n)
{
        wepw_struct *wepw_ptr;
 
 
        if(WepWIsAllocated(n))
            wepw_ptr = wepw[n];
        else
            return;


        OSWUnmapWindow(wepw_ptr->toplevel);
        wepw_ptr->map_state = 0;

        OSWDestroyPixmap(&wepw_ptr->toplevel_buf);


        /* Schedual this wepw for deletion. */
        static_wepw::delete_wepw = wepw_ptr;


	return;
}


/*
 *      Destroys all allocated resources for weapon
 *      window n.
 */
void WepWDestroy(int n)
{
        wepw_struct *wepw_ptr;
        
            
        if(WepWIsAllocated(n))
            wepw_ptr = wepw[n];
        else
            return;
          
        
        wepw_ptr->map_state = 0;
        wepw_ptr->is_in_focus = 0;
            
        wepw_ptr->has_changes = False;
        wepw_ptr->src = NULL;
	wepw_ptr->src_obj_num = -1;
        wepw_ptr->src_obj_ptr = NULL;
        
        if(IDC())
        {
            PBtnDestroy(&wepw_ptr->close_btn);

            PromptDestroy(&wepw_ptr->flags_prompt);
	    PromptDestroy(&wepw_ptr->ocs_code_prompt);
            PromptDestroy(&wepw_ptr->emission_type_prompt);
            PromptDestroy(&wepw_ptr->amount_prompt);
            PromptDestroy(&wepw_ptr->max_prompt);
            PromptDestroy(&wepw_ptr->power_prompt);
            PromptDestroy(&wepw_ptr->range_prompt);
            PromptDestroy(&wepw_ptr->create_power_prompt);
            PromptDestroy(&wepw_ptr->delay_prompt);
            PromptDestroy(&wepw_ptr->use_sound_code_prompt);
            PromptDestroy(&wepw_ptr->fire_sound_code_prompt);
            PromptDestroy(&wepw_ptr->hit_sound_code_prompt);
            PromptDestroy(&wepw_ptr->recover_sound_code_prompt);

            PBtnDestroy(&wepw_ptr->create_wep_btn);
            PBtnDestroy(&wepw_ptr->delete_wep_btn);
 
	    CListDestroy(&wepw_ptr->list);


            OSWDestroyPixmap(&wepw_ptr->toplevel_buf);
            OSWDestroyWindow(&wepw_ptr->toplevel);
        }
        
        
        return;
}           




