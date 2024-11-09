// unvedit/uewmacros.cpp
/*
                  Universe Editor Window: Macros

	Functions:

	int UEWGetNumByPtr(uew_struct *uew_ptr)
	int UEWIsObjectGarbage(uew_struct *uew_ptr, int obj_num)
	int UEWIsObjectSelected(uew_struct *uew_ptr, int obj_num)
	int UEWGetLastSelectedObject(uew_struct *uew_ptr)
	void UEWDoChownAll(
	        uew_struct *uew_ptr,
	        int old_owner,
	        int new_owner
	)
	int UEWDoSelectObject(int n, int obj_num)
	void UEWDoUnselectAllObjects(int n)
	int UEWDoSetStatusMesg(int n, char *mesg)
	void UEWDoUpdateWindowMenus()
	int UEWDoNew(uew_struct *uew_ptr)
	int UEWDoOpen(uew_struct *uew_ptr, char *path)
	int UEWDoOpenNew(uew_struct *uew_ptr, char *path)
	int UEWDoSave(uew_struct *uew_ptr)
	int UEWDoSaveAs(uew_struct *uew_ptr, char *path)
	int UEWDoInsertObject(
	    uew_struct *uew_ptr,
	    xsw_object_struct *obj_ptr
	)
        int UEWDoCopyObject(uew_struct *uew_ptr)
        int UEWDoPasteObject(uew_struct *uew_ptr)
	int UEWDoDeleteObject(uew_struct *uew_ptr)

	---


 */
/*
#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
#include <string.h>
*/
#include <unistd.h>
#include <sys/stat.h>

#include "../include/disk.h"
#include "../include/osw-x.h"
#include "../include/widget.h"

#include "../include/reality.h"
#include "../include/objects.h"
#include "../include/unvmain.h"
#include "../include/unvmatch.h"  
#include "../include/unvutil.h"
#include "../include/unvfile.h"


#include "ue.h"
#include "uhw.h"
#include "wepw.h"
#include "ecow.h"
#include "uew.h"

#ifndef MAX
#define MIN(a,b)        (((a) < (b)) ? (a) : (b))
#define MAX(a,b)	(((a) > (b)) ? (a) : (b))
#endif
/* #define MIN(a,b)        ((a) < (b) ? (a) : (b)) */
/* #define MAX(a,b)        ((a) > (b) ? (a) : (b)) */



/*
 *	Looks for an allocated uew in the uew pointer array
 *	matching that of uew_ptr and returns it's index number or
 *	-1 on error or no match.
 */
int UEWGetNumByPtr(uew_struct *uew_ptr)
{
	int i;


	if(uew_ptr == NULL)
	    return(-1);


	for(i = 0; i < total_uews; i++)
	{
	    if(uew[i] == uew_ptr)
		return(i);
	}

	return(-1);
}


/*
 *	Checks if the XSW object obj_num is valid and non-garbage on
 *	uew.
 */
int UEWIsObjectGarbage(uew_struct *uew_ptr, int obj_num)
{
	if(uew_ptr == NULL)
	    return(-1);
	else if((obj_num < 0) || (obj_num >= uew_ptr->total_objects))
	    return(-1);
	else if(uew_ptr->object[obj_num] == NULL)
	    return(-1);
	else if(uew_ptr->object[obj_num]->type <= XSW_OBJ_TYPE_GARBAGE)
	    return(1);
	else
	    return(0);
}

/*
 *	Checks if obj_num is selected on uew.
 */
int UEWIsObjectSelected(uew_struct *uew_ptr, int obj_num)
{
	int i;

	if((uew_ptr == NULL) ||
           (obj_num < 0)
	)
	    return(0);


	for(i = 0; i < uew_ptr->total_sel_objects; i++)
	{
	    if(uew_ptr->sel_object[i] == obj_num)
		return(1);
	}


	return(0);
}

/*
 *	Returns the last selected object if it is valid or
 *	-1 if no objects are selected.  If return is greater than -1
 *	then the object can be assumed valid without need of any
 *	further checks.
 */
int UEWGetLastSelectedObject(uew_struct *uew_ptr)
{
	int i;


	if(uew_ptr == NULL)
	    return(-1);

	if(uew_ptr->total_sel_objects < 1)
	    return(-1);

	/* Get first selected object. */
	i = uew_ptr->sel_object[uew_ptr->total_sel_objects - 1];

	/* Is that object valid and non-garbage? */
	if(UEWIsObjectGarbage(uew_ptr, i))
	    return(-1);
	else
	    return(i);
}


/*
 *	Procedure too chown all objects owned by old_owner to
 *	new_owner (old_owner itself will be chowned to new_owner
 *	as well).
 *
 *	old_owner cannot be -1, but new_owner can be -1.
 */
void UEWDoChownAll(uew_struct *uew_ptr, int old_owner, int new_owner)
{
	int i, last_sel_obj;
	xsw_object_struct **ptr;


	if((uew_ptr == NULL) ||
           (old_owner < 0)
	)
	    return;


	if(uew_ptr->total_sel_objects > 0)
	    last_sel_obj = uew_ptr->sel_object[
		uew_ptr->total_sel_objects - 1
	    ];
	else
	    last_sel_obj = -1;


	for(i = 0, ptr = uew_ptr->object;
            i < uew_ptr->total_objects;
            i++, ptr++
	)
	{
	    if(*ptr == NULL)
		continue;

	    if((*ptr)->owner != old_owner)
		continue;

	    /*   Need to apply property values before setting owner as
	     *   needed.
	     */
	    if(i == last_sel_obj)
		UEWPropsDoSetValues(uew_ptr, i);

	    (*ptr)->owner = new_owner;

	    if(i == last_sel_obj)
	        UEWPropsDoGetValues(uew_ptr, i);
	}


	return;
}

/*
 *	Selects obj_num on uew n.
 */
int UEWDoSelectObject(int n, int obj_num)
{
	int i, prev_obj_num;
        uew_struct *uew_ptr;
	colum_list_struct *list;
	colum_list_item_struct *item;
	xsw_object_struct *obj_ptr;
	isref_struct *isref_ptr;


        if(UEWIsAllocated(n))  
            uew_ptr = uew[n];
        else
            return(-1);


	if(obj_num < 0)
	    return(-1);

	if(uew_ptr->total_sel_objects < 0)
	    uew_ptr->total_sel_objects = 0;


	/* Get previous last selected object number. */
	if(uew_ptr->total_sel_objects > 0)
	    prev_obj_num = uew_ptr->sel_object[
		uew_ptr->total_sel_objects - 1];
	else
	    prev_obj_num = -1;


	/* Check if already selected. */
	for(i = 0; i < uew_ptr->total_sel_objects; i++)
	{
	    if(uew_ptr->sel_object[i] == obj_num)
		return(0);
	}

	/* Allocate a new selection entry. */
	i = uew_ptr->total_sel_objects;
	uew_ptr->total_sel_objects++;

	uew_ptr->sel_object = (int *)realloc(
	    uew_ptr->sel_object,
	    uew_ptr->total_sel_objects * sizeof(int)
	);
	if(uew_ptr->sel_object == NULL)
	{
	    uew_ptr->total_sel_objects = 0;
	    return(-1);
	}	

	uew_ptr->sel_object[i] = obj_num;



	/* Select row on colum list. */
	list = &uew_ptr->objects_list;
	for(i = 0; i < list->total_rows; i++)
	{
	    if(list->row[i] == NULL)
		continue;

	    if(list->row[i]->total_items < 1)
		continue;

	    item = list->row[i]->item[0];

	    if((int)item->client_data == obj_num)
	    {
		CListSelectRow(list, i);
		if(list->map_state)
		    CListDraw(list, CL_DRAW_AMOUNT_LIST);
	    }
	}


	/*   Apply properties values in prompts to object of
	 *   prev_obj_num.
	 */
	if(prev_obj_num > -1)
	{
	    UEWPropsDoSetValues(uew_ptr, prev_obj_num);
	}


	/* Update current isref to be displayed on preview. */
	if(!UEWIsObjectGarbage(uew_ptr, obj_num))
	{
	    obj_ptr = uew_ptr->object[obj_num];

	    /* Unload previous isref. */
	    i = uew_ptr->cur_isref;
	    if((i >= 0) && (i < uew_ptr->total_isrefs))
		ISRefUnload(uew_ptr->isref[i]);

	    /* Set new current isref. */
	    uew_ptr->cur_isref = obj_ptr->imageset;

	    /* Load new isref. */
            i = uew_ptr->cur_isref;
            if((i >= 0) && (i < uew_ptr->total_isrefs))
	    {
                ISRefLoad(uew_ptr->isref[i]);
		isref_ptr = uew_ptr->isref[i];
	    }
	    else
	    {
		isref_ptr = NULL;
	    }

	    /* Update animation values. */
	    if(obj_ptr->type == XSW_OBJ_TYPE_ANIMATED)
	    {
		uew_ptr->ani_frame = 0;
		uew_ptr->ani_total_frames = ((isref_ptr == NULL) ?
		    1 : MAX(isref_ptr->total_frames, 1)
		);
		uew_ptr->ani_int = MAX(obj_ptr->animation.interval, 10);
		uew_ptr->ani_next = cur_millitime + uew_ptr->ani_int;
	    }
	    else
	    {
                uew_ptr->ani_frame = 0;
                uew_ptr->ani_total_frames = ((isref_ptr == NULL) ?
                    1 : MAX(isref_ptr->total_frames, 1)
                );
                uew_ptr->ani_int = 500;
                uew_ptr->ani_next = cur_millitime + uew_ptr->ani_int;
	    }
	}


	/*   Get values from new object or reset values if new object is
	 *   garbage.
	 */
	UEWPropsDoGetValues(uew_ptr, obj_num);


	return(0);
}


/*
 *	Unselects all objects on uew n.
 */
void UEWDoUnselectAllObjects(int n)
{
	int i;
        uew_struct *uew_ptr;


        if(UEWIsAllocated(n))
            uew_ptr = uew[n];
        else
            return;


	/* Check if an object was already selected. */
	if(uew_ptr->total_sel_objects > 0)
	{
	    /* Apply changes from properties prompts to last object. */
	    i = uew_ptr->sel_object[uew_ptr->total_sel_objects - 1];
	    if(!UEWIsObjectGarbage(uew_ptr, i))
		UEWPropsDoSetValues(uew_ptr, i);
	}


	free(uew_ptr->sel_object);
	uew_ptr->sel_object = NULL;

	uew_ptr->total_sel_objects = 0;


	/* Unselect all objects on list. */
        CListUnselectAllRows(&uew_ptr->objects_list);


	/* Unset current selected isref to be displayed on preview. */
	uew_ptr->cur_isref = -1;

	/* Unload all isrefs. */
	for(i = 0; i < uew_ptr->total_isrefs; i++)
	{
	    if(uew_ptr->isref[i] == NULL)
		continue;

	    ISRefUnload(uew_ptr->isref[i]);
	}


	return;
}


/*
 *	Updates status message on uew.
 */
int UEWDoSetStatusMesg(int n, char *mesg)
{
        uew_struct *uew_ptr;


        if(UEWIsAllocated(n))
            uew_ptr = uew[n];
        else
            return(-1);


	if(mesg == NULL)
	{
	    memset(
		uew_ptr->status_mesg,
		'\0',
		UEW_STATUS_MESG_MAX
	    );
	}
	else
	{
	    strncpy(
		uew_ptr->status_mesg,
		mesg,
		UEW_STATUS_MESG_MAX
            );
	    uew_ptr->status_mesg[UEW_STATUS_MESG_MAX - 1] = '\0';
	}


	/* Redraw if mapped. */
	if(uew_ptr->map_state)
	    UEWDraw(n, UEW_DRAW_STATUS);


	return(0);
}


/*
 *	Procedure to update all universe editor windows'
 *	Window menu entries.
 */
void UEWDoUpdateWindowMenus()
{
	int i, n;
	int menu_item;
	int item_pos_num, op_code;
	menu_bar_item_struct *mb_item_ptr;
	menu_bar_struct *mb;
	char title[128 + UNV_TITLE_MAX];


	menu_item = UEW_WINDOW_MENU_NUM;


	for(i = 0; i < total_uews; i++)
	{
	    if(uew[i] == NULL)
		continue;

	    if(!MenuBarIsItemAllocated(&uew[i]->mb, menu_item))
		continue;

	    /* Get pointer to menu bar. */
	    mb = &uew[i]->mb;

	    /* Get pointer to menu bar item. */
	    mb_item_ptr = mb->item[menu_item];

	    /* Delete all old menu items. */
	    MenuDeleteAllItems(mb_item_ptr->menu);

	    /* Readd menu items. */
	    MenuBarAddItemMenuItem(
		mb,
		menu_item,
		"New Universe Editor",
		MENU_ITEM_TYPE_ENTRY,
		NULL,
		UEW_MENU_CODE_NEW_UEW,
	        -1
	    );

            MenuBarAddItemMenuItem(
                mb,
                menu_item,
                NULL, 
                MENU_ITEM_TYPE_HR,
                NULL,
                UEW_MENU_CODE_NONE,
                -1
            );

	    /* Window position number listed on the menu. */
	    item_pos_num = 0;

	    /* Menu code number (starting from UEW_MENU_CODE_WINDOW_START).  */
	    op_code = UEW_MENU_CODE_WINDOW_START;

	    /* Universe editor windows. */
            for(n = 0; n < total_uews; n++)
            {
		if(op_code >= UEW_MENU_CODE_WINDOW_END)
		    break;

                if(uew[n] == NULL)
                    continue;

		sprintf(title, "%i %s: %s",
		    item_pos_num + 1,
		    UEW_DEF_TITLE,
		    uew[n]->unv_header.title
		);

                /* Readd menu items. */
                MenuBarAddItemMenuItem(
                    mb,
                    menu_item,
                    title,
                    MENU_ITEM_TYPE_ENTRY,
                    NULL,
                    op_code,
                    -1
                );

		item_pos_num++;
		op_code++;
	    }

            /* Universe header windows. */
            for(n = 0; n < total_uhws; n++)
            {
                if(op_code >= UEW_MENU_CODE_WINDOW_END)
                    break;

                if(uhw[n] == NULL)
                    continue;

                sprintf(title, "%i %s",
                    item_pos_num + 1,
		    UHW_DEF_TITLE
                );

                /* Readd menu items. */
                MenuBarAddItemMenuItem(
                    mb,
                    menu_item,
                    title,
                    MENU_ITEM_TYPE_ENTRY,
                    NULL,
                    op_code,
                    -1
                );

                item_pos_num++; 
                op_code++;
            }

            /* Weapon windows. */
            for(n = 0; n < total_wepws; n++)
            {
                if(op_code >= UEW_MENU_CODE_WINDOW_END)
                    break;

                if(wepw[n] == NULL)
                    continue;

                sprintf(title, "%i %s %s",
                    item_pos_num + 1,
		    UNVGetObjectFormalName(
			wepw[n]->src_obj_ptr,
			wepw[n]->src_obj_num
		    ),
                    "Weapons"
                );
 
                /* Readd menu items. */
                MenuBarAddItemMenuItem(
                    mb,
                    menu_item,
                    title,
                    MENU_ITEM_TYPE_ENTRY,
                    NULL,
                    op_code,
                    -1
                );

                item_pos_num++; 
                op_code++;
            }

            /* Economy windows. */
            for(n = 0; n < total_ecows; n++)
            {
                if(op_code >= UEW_MENU_CODE_WINDOW_END)
                    break;

                if(ecow[n] == NULL)
                    continue;

                sprintf(title, "%i %s %s",
                    item_pos_num + 1,
                    UNVGetObjectFormalName(
                        ecow[n]->src_obj_ptr,
                        ecow[n]->src_obj_num
                    ),
                    "Economy"
                );
                    
                /* Readd menu items. */
                MenuBarAddItemMenuItem(
                    mb,
                    menu_item,
                    title,
                    MENU_ITEM_TYPE_ENTRY,
                    NULL,
                    op_code,  
                    -1
                );
                    
                item_pos_num++;
                op_code++;
            }

	}



	return;
}


/*
 *	Procedure to create a new universe on uew.
 */
int UEWDoNew(uew_struct *uew_ptr)
{
	int n;
	char cwd[PATH_MAX];
        char text[80 + PATH_MAX + NAME_MAX];


	if(uew_ptr == NULL)
	    return(-1);

        n = UEWGetNumByPtr(uew_ptr);

	getcwd(cwd, PATH_MAX);
	cwd[PATH_MAX - 1] = '\0';


        /* Unselect all objects. */
        free(uew_ptr->sel_object);
        uew_ptr->sel_object = NULL;
        
        uew_ptr->total_sel_objects = 0;


	/* Deallocate all isrefs. */
	ISRefDeleteAll(uew_ptr->isref, uew_ptr->total_isrefs);
	uew_ptr->isref = NULL;
	uew_ptr->total_isrefs = 0;

        /* Deallocate any objects currently allocated. */
        UNVDeleteAllObjects(uew_ptr->object, uew_ptr->total_objects);
        uew_ptr->object = NULL;
        uew_ptr->total_objects = 0;


        /* Reset header. */
        strncpy(
            uew_ptr->unv_header.title,
            DEF_UNIVERSE_TITLE,
            UNV_TITLE_MAX
        );
        strncpy(
            uew_ptr->unv_header.isr,
            DEF_ISR_FILENAME,
            PATH_MAX + NAME_MAX
        );
        strncpy(
            uew_ptr->unv_header.ocsn,
            DEF_OCSN_FILENAME,
            PATH_MAX + NAME_MAX
        );
        strncpy(
            uew_ptr->unv_header.ss,
            DEF_SS_FILENAME,
            PATH_MAX + NAME_MAX
        );
	uew_ptr->unv_header.ru_to_au = DEF_RU_TO_AU;
        uew_ptr->unv_header.lost_found_owner = 0;

	/* Set new path. */
	uew_ptr->unv_file[0] = '\0';


        /* Check if uew has changes. */
        if(uew_ptr->has_changes)
        {
            uew_ptr->has_changes = False;
        } 


	/* Reset title. */
        sprintf(text, "%s: %s",
            UEW_DEF_TITLE,
            DEF_UNIVERSE_TITLE
        );
        OSWSetWindowTitle(uew_ptr->toplevel, text);
	UEWDoUpdateWindowMenus();

	/* Update objects colum list. */
	UEWUpdateObjectList(uew_ptr);

	/* Clear view status prompt values. */
	UEWDoSetViewStatusObjectValues(n, NULL);

	/* Update status message. */
	UEWDoSetStatusMesg(n, "New universe created.");


	/* Redraw everything as needed. */
	if(uew_ptr->map_state)
	    UEWDraw(n, UEW_DRAW_COMPLETE);


	return(0);
}


/*
 *	Procedure to open a new universe file.
 */
int UEWDoOpen(uew_struct *uew_ptr, char *path)
{
	int i, n, total;
	char text[80 + PATH_MAX + NAME_MAX];


	if((uew_ptr == NULL) || (path == NULL))
	    return(-1);

        /* Sanitize path. */
        if(strlen(path) >= (PATH_MAX + NAME_MAX))
            path[PATH_MAX + NAME_MAX - 1] = '\0';

	/* Check if another uew has this file opened. */
	for(i = 0; i < total_uews; i++)
	{
	    if(uew[i] == NULL)
		continue;

	    /* Skip current. */
	    if(uew[i] == uew_ptr)
		continue;

	    if(!strcmp(uew[i]->unv_file, path))
	    {
                comfwin.option &= ~ComfirmOptionAll;
                comfwin.option &= ~ComfirmOptionCancel;
                if(ComfWinDoQuery(&comfwin,
"This file appears to be already opened in another\n\
window. Are you sure you want to open this file?"
                ) != ComfirmCodeYes)
                    return(0);
	    }
	}


	/* Unselect all objects. */
	free(uew_ptr->sel_object);
	uew_ptr->sel_object = NULL;

	uew_ptr->total_sel_objects = 0;


        /* Deallocate all isrefs. */
        ISRefDeleteAll(uew_ptr->isref, uew_ptr->total_isrefs);
        uew_ptr->isref = NULL;
        uew_ptr->total_isrefs = 0;

	/* Deallocate any objects currently allocated. */
	UNVDeleteAllObjects(uew_ptr->object, uew_ptr->total_objects);
	uew_ptr->object = NULL;
	uew_ptr->total_objects = 0;


	/* Load from file. */
	uew_ptr->object = UNVLoadFromFile(
	    path,
	    &total,
	    &uew_ptr->unv_header,
	    uew_ptr,
	    UEWUnvIOProgressNotifyCB
	);
	uew_ptr->total_objects = total;

	/* Set new filename. */
	if(uew_ptr->total_objects > 0)
	{
	    strncpy(
		uew_ptr->unv_file,
		path,
		PATH_MAX + NAME_MAX
	    );
	    uew_ptr->unv_file[PATH_MAX + NAME_MAX - 1] = '\0';

            sprintf(text, "%s: %s",
		UEW_DEF_TITLE,
		uew_ptr->unv_header.title
	    );

	    OSWSetWindowTitle(uew_ptr->toplevel, text);
	    UEWDoUpdateWindowMenus();
	}

	/* Load new isrefs. */
	uew_ptr->isref = ISRefLoadFromFile(
	    uew_ptr->unv_header.isr,
	    &total,
	    dname.images
	);
        uew_ptr->total_isrefs = total;

        /* Check if uew has changes. */
        if(uew_ptr->has_changes)
        {
            uew_ptr->has_changes = False;
        }

        /* Update objects colum list. */
        UEWUpdateObjectList(uew_ptr);

        /* Update status message. */
        n = UEWGetNumByPtr(uew_ptr);

        /* Clear view status prompt values. */
        UEWDoSetViewStatusObjectValues(n, NULL);

        /* Update status message. */
        UEWDoSetStatusMesg(n, "Universe loaded.");

	PBarUpdate(&uew_ptr->pbar, 0, NULL);

        /* Redraw everything as needed. */
        if(uew_ptr->map_state)
            UEWDraw(n, UEW_DRAW_COMPLETE);


	return(0);
}

/*
 *	Procedure to open new universe file on a new
 *	universe editor window.
 */
int UEWDoOpenNew(uew_struct *uew_ptr, char *path)
{
	// Dan S: "new" is a reserved word in C++, changed to new_uf
	int i, n, new_uf;
        int total;
        char text[80 + PATH_MAX + NAME_MAX];


        if(path == NULL)
            return(-1);


        /* Sanitize path. */
        if(strlen(path) >= (PATH_MAX + NAME_MAX))
            path[PATH_MAX + NAME_MAX - 1] = '\0';

        /* Check if another uew has this file opened. */
        for(i = 0; i < total_uews; i++)
        {
            if(uew[i] == NULL)
                continue;

            if(!strcmp(uew[i]->unv_file, path))
            {
                comfwin.option &= ~ComfirmOptionAll;
                comfwin.option &= ~ComfirmOptionCancel;
                if(ComfWinDoQuery(&comfwin,
"This file appears to be already opened in another\n\
window. Are you sure you want to open this file?"
                ) != ComfirmCodeYes)
                    return(0);
            }
        }


        /* Allocate new uew. */
	new_uf = UECreateUEW(
	    0, NULL
	);

	if(UEWIsAllocated(new_uf))
	    uew_ptr = uew[new_uf];
	else
	    return(-1);


        /* Map new uew. */
        UEWMap(new_uf);


        /* Load from file. */
        uew_ptr->object = UNVLoadFromFile(
            path, 
            &total,
            &uew_ptr->unv_header,
            uew_ptr,
            UEWUnvIOProgressNotifyCB
        );
        uew_ptr->total_objects = total;

        /* Load new isrefs. */
        uew_ptr->isref = ISRefLoadFromFile(
            uew_ptr->unv_header.isr,
            &total, 
            dname.images
        );
	uew_ptr->total_isrefs = total;

        /* Set new filename. */
        if(uew_ptr->total_objects > 0)
        {
            strncpy(
                uew_ptr->unv_file, 
                path,
                PATH_MAX + NAME_MAX
            );
            uew_ptr->unv_file[PATH_MAX + NAME_MAX - 1] = '\0';

            sprintf(text, "%s: %s",
                UEW_DEF_TITLE,
                uew_ptr->unv_header.title
            );

            OSWSetWindowTitle(uew_ptr->toplevel, text);
	    UEWDoUpdateWindowMenus();
        }


        /* Reset has changes marker. */
        uew_ptr->has_changes = False;


        /* Update objects colum list. */
        UEWUpdateObjectList(uew_ptr); 


        /* Update status message. */
        n = UEWGetNumByPtr(uew_ptr);
        UEWDoSetStatusMesg(n, "Universe loaded.");

        PBarUpdate(&uew_ptr->pbar, 0, NULL);

        /* Redraw everything as needed. */
        if(uew_ptr->map_state)
            UEWDraw(n, UEW_DRAW_COMPLETE);


        return(0);
}


/*
 *	Procedure to save.
 */
int UEWDoSave(uew_struct *uew_ptr)
{
        int i, n, status; 
        char text[256 + PATH_MAX + NAME_MAX];


        if(uew_ptr == NULL)
            return(-1);


        /* Apply changes to last selected object as needed. */
        if(uew_ptr->total_sel_objects > 0)
        {
            /* Apply changes from properties prompts to last object. */
            i = uew_ptr->sel_object[uew_ptr->total_sel_objects - 1];
            if(!UEWIsObjectGarbage(uew_ptr, i)) 
                UEWPropsDoSetValues(uew_ptr, i);
        }

	/* Update universe header. */
	strncpy(
	    uew_ptr->unv_header.version,
            PROG_VERSION,
            UNV_TITLE_MAX
        );
        uew_ptr->unv_header.version[UNV_TITLE_MAX - 1] = '\0';

	/* Save to file. */
        status = UNVSaveToFile(
            uew_ptr->unv_file,
            uew_ptr->object,
            uew_ptr->total_objects,
            &uew_ptr->unv_header,
            uew_ptr,
            UEWUnvIOProgressNotifyCB
        );
        if(status < 0)
        {   
            sprintf(text,
"Error saving universe to file:\n\n\
    %s\n",
                uew_ptr->unv_file
            );
            printdw(&dialog, text);
            return(-1);
        }


        /* Reset has changes marker. */
	uew_ptr->has_changes = False;


        /* Update status message. */
        n = UEWGetNumByPtr(uew_ptr);
        UEWDoSetStatusMesg(n, "Universe saved.");

        PBarUpdate(&uew_ptr->pbar, 0, NULL);

        /* Redraw everything as needed. */
        if(uew_ptr->map_state)
            UEWDraw(n, UEW_DRAW_COMPLETE);


	return(0);
}


/*
 *	Procedure to save as.
 */
int UEWDoSaveAs(uew_struct *uew_ptr, char *path)
{
	int i, n, status;
	char text[256 + PATH_MAX + NAME_MAX];
	struct stat stat_buf;


        if((uew_ptr == NULL) ||
           (path == NULL)
        )
            return(-1);

	/* Sanitize path. */
	if(strlen(path) >= (PATH_MAX + NAME_MAX))
	    path[PATH_MAX + NAME_MAX - 1] = '\0';


	/* Check if path already exists. */
	if(!stat(path, &stat_buf))
	{
	    if(S_ISDIR(stat_buf.st_mode))
	    {
		sprintf(text, "`%s' is a directory.", path);
		printdw(&dialog, text);
		return(-1);
	    }
	    else
	    {
	        sprintf(text, "Overwrite `%s'?", path);
                comfwin.option &= ~ComfirmOptionAll;
                comfwin.option &= ~ComfirmOptionCancel;
                if(ComfWinDoQuery(&comfwin, text) != ComfirmCodeYes)
                    return(0);
	    }
	}


        /* Apply changes to last selected object as needed. */ 
        if(uew_ptr->total_sel_objects > 0)
        {
            /* Apply changes from properties prompts to last object. */
            i = uew_ptr->sel_object[uew_ptr->total_sel_objects - 1];
            if(!UEWIsObjectGarbage(uew_ptr, i))
                UEWPropsDoSetValues(uew_ptr, i);
        }

        /* Update universe header. */
        strncpy(
            uew_ptr->unv_header.version,
            PROG_VERSION,
            UNV_TITLE_MAX
        );
        uew_ptr->unv_header.version[UNV_TITLE_MAX - 1] = '\0';

	/* Save to new file path. */
	status = UNVSaveToFile(
	    path,
	    uew_ptr->object,
	    uew_ptr->total_objects,
	    &uew_ptr->unv_header,
            uew_ptr,
            UEWUnvIOProgressNotifyCB
	);

	if(status < 0)
	{
	    sprintf(text,
"Error saving universe to file:\n\n\
    %s\n",
		path
	    );
	    printdw(&dialog, text);
	    return(-1);
	}
	else
	{
	    /*   Need to update path of universe file to the one we
	     *   just saved to.
	     */
	    strncpy(
		uew_ptr->unv_file,
		path,
		PATH_MAX + NAME_MAX
	    );
	    uew_ptr->unv_file[PATH_MAX + NAME_MAX - 1] = '\0';

	    UEWDoUpdateWindowMenus();
	}


	/* Reset has changes marker. */
        uew_ptr->has_changes = False;


        /* Update status message. */
	n = UEWGetNumByPtr(uew_ptr);
	UEWDoSetStatusMesg(n, "Universe saved.");

        PBarUpdate(&uew_ptr->pbar, 0, NULL);

        /* Redraw everything as needed. */
        if(uew_ptr->map_state)
            UEWDraw(n, UEW_DRAW_COMPLETE);


	return(0);
}


/*
 *	Procedure to insert an object with referance to obj_ptr.
 *	If obj_ptr is NULL then the new object will just have
 *	default values.
 */
int UEWDoInsertObject(
	uew_struct *uew_ptr, 
	xsw_object_struct *obj_ptr
)
{
	int uew_num;
        int obj_num;
	xsw_object_struct *new_obj_ptr;


        if(uew_ptr == NULL)
            return(-1);

        uew_num = UEWGetNumByPtr(uew_ptr);


	/* Sanitize total objects. */
	if(uew_ptr->total_objects < 0)
	    uew_ptr->total_objects = 0;


	/* Begin looking for available obj_num. */
	obj_num = 0;

	/*   If a object is selected, then begin search from it's
	 *   selected position.
	 */
	if(uew_ptr->total_sel_objects > 0)
	{
	    /* Get first selected object position. */
	    obj_num = uew_ptr->sel_object[
		uew_ptr->total_sel_objects - 1
	    ];
	}
	if(obj_num < 0)
	    obj_num = 0;

	for(;obj_num < uew_ptr->total_objects; obj_num++)
	{
	    if(uew_ptr->object[obj_num] == NULL)
		break;

            if(uew_ptr->object[obj_num]->type <= XSW_OBJ_TYPE_GARBAGE)
                break;
	}
	/* Need to allocate more pointers? */
	if(obj_num >= uew_ptr->total_objects)
	{
	    /* Append an object. */
	    obj_num = uew_ptr->total_objects;
	    uew_ptr->total_objects++;

	    uew_ptr->object = (xsw_object_struct **)realloc(
		uew_ptr->object,
		uew_ptr->total_objects * sizeof(xsw_object_struct *)
	    );
	    if(uew_ptr->object == NULL)
	    {
		uew_ptr->total_objects = 0;
		return(-1);
	    }

	    /* Reset newly allocated pointer. */
	    uew_ptr->object[obj_num] = NULL;
	}


        /* ********************************************************* */
	/*   Got obj_num, its index position is allocated but its
	 *   structure is not.
	 */

	/* Delete new object obj_num as needed. */
	UNVDeleteObject(uew_ptr->object[obj_num]);
	uew_ptr->object[obj_num] = NULL;


	/* Allocate new object. */
	if(obj_ptr == NULL)
	{
	    uew_ptr->object[obj_num] = UNVDupObject(
		&unv_garbage_object
	    );

	    strncpy(
	        uew_ptr->object[obj_num]->name,
		"New Object",
		XSW_OBJ_NAME_MAX
	    );
	    uew_ptr->object[obj_num]->name[XSW_OBJ_NAME_MAX - 1] = '\0';

	    uew_ptr->object[obj_num]->type = XSW_OBJ_TYPE_STATIC;
	}
	else
	{
	    uew_ptr->object[obj_num] = UNVDupObject(obj_ptr);
	}
	/* Allocation error? */
	if(uew_ptr->object[obj_num] == NULL)
	    return(-1);


	new_obj_ptr = uew_ptr->object[obj_num];

	/* Move new object to center of view position. */
	new_obj_ptr->sect_x = uew_ptr->sect_x;
        new_obj_ptr->sect_y = uew_ptr->sect_y;
        new_obj_ptr->sect_z = uew_ptr->sect_z;
        new_obj_ptr->x = uew_ptr->cx;
        new_obj_ptr->y = uew_ptr->cy;
        new_obj_ptr->z = uew_ptr->cz;


	if(!uew_ptr->has_changes)
	    uew_ptr->has_changes = True;


	/* ********************************************************* */

	/* Unselect all. */
	UEWDoUnselectAllObjects(uew_num);

        /* Update objects colum list. */
        UEWUpdateObjectList(uew_ptr);


	/* Select new object. */
	UEWDoSelectObject(uew_num, obj_num);

	/* Scroll objects colum list. */
	UEWObjectListScrollToItem(uew_ptr, obj_num);

	/* Redraw view window. */
	if(uew_ptr->map_state)
	{
	    UEWDraw(uew_num, UEW_DRAW_VIEW);
	    UEWDraw(uew_num, UEW_DRAW_PREVIEW);
	}

	UEWDoSetViewStatusObjectValues(uew_num, uew_ptr->object[obj_num]);



	return(obj_num);
}



int UEWDoCopyObject(uew_struct *uew_ptr)
{
	int obj_num = -1;


	if(uew_ptr == NULL)
	    return(-1);


        /* Apply changes to last selected object as needed. */
        if(uew_ptr->total_sel_objects > 0)
        {
            /* Apply changes from properties prompts to last object. */
            obj_num = uew_ptr->sel_object[uew_ptr->total_sel_objects - 1];
            if(!UEWIsObjectGarbage(uew_ptr, obj_num))
                UEWPropsDoSetValues(uew_ptr, obj_num);
        }
	if(obj_num < 0)
	    return(0);

	obj_num = uew_ptr->sel_object[0];

	if(UEWIsObjectGarbage(uew_ptr, obj_num))
	    return(0);


	UEWDDEPutXSWObject(uew_ptr->object[obj_num]);


	return(0);
}


int UEWDoPasteObject(uew_struct *uew_ptr)         
{
	xsw_object_struct *obj_ptr;


        if(uew_ptr == NULL)
            return(-1);


	/* Fetch object from dde. */
	obj_ptr = UEWDDEFetchXSWObject();
	if(obj_ptr == NULL)
	    return(0);


	/* Do insert new object. */
	UEWDoInsertObject(uew_ptr, obj_ptr);


	/* Delete object. */
	UNVDeleteObject(obj_ptr);
	obj_ptr = NULL;


	return(0);
}


/*
 *	Deletes selected objects.
 */
int UEWDoDeleteObject(uew_struct *uew_ptr)
{
        int s, n, obj_num, status;
	char text[256];
	bool_t yes_to_all = False;


        if(uew_ptr == NULL)
            return(-1);

	n = UEWGetNumByPtr(uew_ptr);


	for(s = 0; s < uew_ptr->total_sel_objects; s++)
	{
	    obj_num = uew_ptr->sel_object[s];

	    if(UEWIsObjectGarbage(uew_ptr, obj_num))
		continue;

	    /* Comfirm delete. */
	    if(!yes_to_all)
	    {
		sprintf(text,
"Comferm delete object:\n\n    %s",
                    UNVGetObjectFormalName(
                        uew_ptr->object[obj_num], obj_num
                    )
                );
		comfwin.option |= ComfirmOptionAll;
                comfwin.option &= ~ComfirmOptionCancel;
		status = ComfWinDoQuery(&comfwin, text);
		if(status == ComfirmCodeAll)
		{
		    yes_to_all = True;
		}
		else if(status == ComfirmCodeYes)
		{

		}
		else
		{
		    continue;
		}
	    }


	    /*   `Delete' object from the list.  Deallocate all
	     *   substructures and reset values.
	     */
	    UNVResetObject(uew_ptr->object[obj_num]);
	    uew_ptr->object[obj_num]->type = XSW_OBJ_TYPE_GARBAGE;

	    /* Need to make sure no objects own this object. */
	    UEWDoChownAll(
		uew_ptr,
		obj_num,	/* Old owner, this object. */
		((uew_ptr->unv_header.lost_found_owner < 0) ?
		    0 : uew_ptr->unv_header.lost_found_owner)
	    );


	    /* Update has changes mark. */
	    if(!uew_ptr->has_changes)
		uew_ptr->has_changes = True;
	}

	/* Unselect all objects since they're gone. */
	UEWDoUnselectAllObjects(n);

        /* Update objects colum list. */
        UEWUpdateObjectList(uew_ptr);

	/* Redraw and update values. */
	UEWDoSetViewStatusObjectValues(n, NULL);

	/* Clear properties prompts. */
	UEWPropsDoGetValues(uew_ptr, -1);

	UEWDraw(n, UEW_DRAW_COMPLETE);


	return(0);
}




