// unvedit/uewcb.cpp

/*
            Universe Editor Window: Callback Functions

	Functions:

	int UEWMenuBarCB(void *ptr, int code);
	void UEWUnvIOProgressNotifyCB(void *ptr, int cur, int max)
	int UEWObjectsListCLCB(void *ptr)

	int UEWTBNewPBCB(void *ptr)
	int UEWTBOpenPBCB(void *ptr)
	int UEWTBSavePBCB(void *ptr)
	int UEWTBCopyPBCB(void *ptr)
	int UEWTBPastePBCB(void *ptr)
	int UEWTBNewObjPBCB(void *ptr)
	int UEWTBEconomyPBCB(void *ptr)
	int UEWTBWeaponsPBCB(void *ptr)
	int UEWTBPrintPBCB(void *ptr)

	---

 */
/*
#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
#include <string.h>
*/
#include "../include/disk.h"
 
#include "../include/osw-x.h"
#include "../include/widget.h"

#include "../include/reality.h"
#include "../include/objects.h"
#include "../include/unvmatch.h"
#include "../include/unvutil.h"

#include "ue.h"
#include "uhw.h"
#include "wepw.h"
#include "ecow.h"
#include "printwin.h"
#include "optwgen.h"
#include "aboutwin.h"
#include "uew.h"


int UEWMenuBarCB(void *ptr, int code)
{
	int i, n, j, uew_num;
	uew_struct *uew_ptr = NULL;
	char cwd[PATH_MAX];


	if(ptr == NULL)
	    return(-1);


	for(i = 0; i < total_uews; i++)
	{
	    if(uew[i] == (uew_struct *)ptr)
	    {
	        uew_ptr = uew[i];
		break;
	    }
	}
	if(uew_ptr == NULL)
	    return(0);

	uew_num = i;


	/* Get current working dir. */
	getcwd(cwd, PATH_MAX);
	cwd[PATH_MAX - 1] = '\0';



	/* Handle menu code. */
	switch(code)
	{
	  /* ******************************************************** */
	  case UEW_MENU_CODE_NEW:
            /* Check if uew has changes. */
            if(uew_ptr->has_changes)
            {
                comfwin.option |= ComfirmOptionCancel;
                comfwin.option &= ~ComfirmOptionAll;
                i = ComfWinDoQuery(&comfwin,
"Current universe has changes, save\n\
changes before creating a new universe?"
                );
                if(i == ComfirmCodeYes)
                {
                    if(uew_ptr->unv_file[0] == '\0')
                    {
                        printdw(&dialog,
"This universe has not been saved to a specified\n\
file name. Choose `save as' to save this universe\n\
to a specified file name."
                        );
                        break;
                    }

                    /* Save. */
                    if(UEWDoSave(uew_ptr))
                        break;
                }
                else if(i == ComfirmCodeNo)
                {
                    /* No save. */
                }
                else
                {
                    /* Cancel. */
                    break;
                }

                uew_ptr->has_changes = False;
            }

	    UEWDoNew(uew_ptr);
	    break;

          /* ******************************************************** */
	  case UEW_MENU_CODE_OPEN:
            /* Check if uew has changes. */
            if(uew_ptr->has_changes)
            {
                comfwin.option |= ComfirmOptionCancel;
                comfwin.option &= ~ComfirmOptionAll;
                i = ComfWinDoQuery(&comfwin,
"Current universe has changes, save\n\
changes before opening a new universe?"
                );
                if(i == ComfirmCodeYes)
                {
                    if(uew_ptr->unv_file[0] == '\0')
                    {
                        printdw(&dialog,
"This universe has not been saved to a specified\n\
file name. Choose `save as' to save this universe\n\
to a specified file name."
                        );
                        break;
                    }

                    /* Save. */
                    if(UEWDoSave(uew_ptr))
                        break;
                }
                else if(i == ComfirmCodeNo)
                {
                    /* No save. */
                }
                else
                {
                    /* Cancel. */
                    break;
                }

                uew_ptr->has_changes = False;
            }

	    file_browser_src_ptr = (void *)uew_ptr;
	    file_browser_op_code = UE_FB_OP_CODE_OPEN;
	    FBrowserSetOpMesg(
                &file_browser,
                "Open Universe",
                "Open"
            );
            FBrowserMapSearchMask(
                &file_browser, FN_UNV_EXTENSION_MASK
            );
	    break;

          /* ******************************************************** */
          case UEW_MENU_CODE_OPENNEW:
            file_browser_src_ptr = (void *)uew_ptr;
            file_browser_op_code = UE_FB_OP_CODE_OPENNEW;
            FBrowserSetOpMesg(
                &file_browser,
                "Open Universe",
                "Open"
            );
            FBrowserMapSearchMask(
                &file_browser, FN_UNV_EXTENSION_MASK
            );
            break;

          /* ******************************************************** */
	  case UEW_MENU_CODE_SAVE:
	    /*   If universe file name is "" then that implies we
	     *   need to save as.
	     */
	    if(uew_ptr->unv_file[0] == '\0')
            {
                /* Save as. */
                file_browser_src_ptr = (void *)uew_ptr;
                file_browser_op_code = UE_FB_OP_CODE_SAVEAS;
                FBrowserSetOpMesg(
                    &file_browser,
                    "Save Universe",
                    "Save"
                );

                FBrowserMapSearchMask(
                    &file_browser, FN_UNV_EXTENSION_MASK
                );
            }
	    else
	    {
		/* Regular save. */
	        UEWDoSave(uew_ptr);
	    }
            break;

          /* ******************************************************** */
	  case UEW_MENU_CODE_SAVEAS:
            file_browser_src_ptr = (void *)uew_ptr;
            file_browser_op_code = UE_FB_OP_CODE_SAVEAS;
            FBrowserSetOpMesg(
                &file_browser,
                "Save Universe",
                "Save"
            );
            FBrowserMapSearchMask(
		&file_browser, FN_UNV_EXTENSION_MASK
	    );
            break;

          /* ******************************************************** */
          case UEW_MENU_CODE_PRINT:
            PrintWinDoMapValues(uew_ptr);
            break;

          /* ******************************************************** */
	  case UEW_MENU_CODE_EXIT:
            if(uew_ptr->has_changes)
            {
                comfwin.option |= ComfirmOptionCancel;
                comfwin.option &= ~ComfirmOptionAll;
                i = ComfWinDoQuery(&comfwin,
"Current universe has changes,\n\
save changes before exiting?"
                );
                if(i == ComfirmCodeYes)
                {
                    if(uew_ptr->unv_file[0] == '\0')
                    {
                        printdw(&dialog,
"This universe has not been saved to a specified\n\
file name. Choose `save as' to save this universe\n\
to a specified file name."
                        );
                        break;
                    }

                    /* Save. */
                    if(UEWDoSave(uew_ptr))
                        break;
                }
                else if(i == ComfirmCodeNo)
                {
                    /* No save. */
                }
                else
                {
                    /* Cancel. */
                    break;
                }

		uew_ptr->has_changes = False;
	    }

            /* Schedual universe edit window n for deletion. */
	    delete_uew = uew_ptr;

            break;

          /* ******************************************************** */
	  case UEW_MENU_CODE_COPY:
	    UEWDoCopyObject(uew_ptr);
            break;

          /* ******************************************************** */
	  case UEW_MENU_CODE_PASTE:
	    UEWDoPasteObject(uew_ptr);
            break;

          /* ******************************************************** */
	  case UEW_MENU_CODE_DELETE:
	    UEWDoDeleteObject(uew_ptr);
	    break;

          /* ******************************************************** */
          case UEW_MENU_CODE_INSERTNEW:   
            UEWDoInsertObject(
                uew_ptr,
                NULL
            );
            break;

          /* ******************************************************** */
	  case UEW_MENU_CODE_EDITWEAPONS:
	    if(uew_ptr->total_sel_objects > 0)
	    {
		i = uew_ptr->sel_object[uew_ptr->total_sel_objects - 1];

	        n = UECreateWepW(0, NULL);
	        WepWDoMapValues(n, uew_ptr, i);
	    }
	    else
	    {
		printdw(
		    &dialog,
"You must select an object first before you can\n\
edit its weapons."
		);
	    }
	    break;

          /* ******************************************************** */
          case UEW_MENU_CODE_EDITECONOMY:
            if(uew_ptr->total_sel_objects > 0)
            {
                i = uew_ptr->sel_object[uew_ptr->total_sel_objects - 1];

                n = UECreateEcoW(0, NULL);
                EcoWDoMapValues(n, uew_ptr, i);
            }
            else
            {
                printdw(
                    &dialog,
"You must select an object first before you can\n\
edit its economy."
                );
            }
            break;

          /* ******************************************************** */
          case UEW_MENU_CODE_UNVHEADER:
	    n = UECreateUHW(0, NULL);
	    UHWDoFetch(n, uew_ptr);	    
	    UHWMap(n);
 	    break;

          /* ******************************************************** */
          case UEW_MENU_CODE_OPT_GENERAL:
	    OptWGenDoMapValues();
            break;

          /* ******************************************************** */
          case UEW_MENU_CODE_NEW_UEW:
            n = UECreateUEW(0, NULL);
            UEWMap(n);
	    break;

	  /* ******************************************************** */
	  case UEW_MENU_CODE_ABOUT:
	    AboutWinMap();
	    break;
	}


        /* ********************************************************** */
	/* Switch to window. */
	if((code >= UEW_MENU_CODE_WINDOW_START) &&
           (code < UEW_MENU_CODE_WINDOW_END)
	)
	{
	    /*   n is menu item position number - start value
             *   offset.
	     */
	    n = code - UEW_MENU_CODE_WINDOW_START;

	    /*   j is current window number (but not relative from
	     *   the window's pointer index number.
	     */
	    j = 0;

	    /* Check through universe editor windows. */
	    for(i = 0; i < total_uews; i++)
	    {
		if(uew[i] == NULL)
		    continue;

		/* Code and window number match? */
		if(n == j)
		    UEWMap(i);

                j++;
	    }

            /* Check through universe header windows. */
            for(i = 0; i < total_uhws; i++)
            {
                if(uhw[i] == NULL)
                    continue;

                /* Code and window number match? */
                if(n == j)
                    UHWMap(i);

                j++;
            }

            /* Check through weapons windows. */
            for(i = 0; i < total_wepws; i++)
            {
                if(wepw[i] == NULL)
                    continue;

                /* Code and window number match? */
                if(n == j)
                    WepWMap(i);

                j++;
            }

            /* Check through economy windows. */  
            for(i = 0; i < total_ecows; i++)
            {
                if(ecow[i] == NULL)
                    continue;

                /* Code and window number match? */
                if(n == j)
                    EcoWMap(i);

                j++;
            }


	}


	return(0);
}


/*
 *	Universe file IO progress notify callback handler.
 */
void UEWUnvIOProgressNotifyCB(void *ptr, int cur, int max)
{
        int i, uew_num;
        uew_struct *uew_ptr = NULL;
	char text[128];
	double p;


        if(ptr == NULL)
            return;


        for(i = 0; i < total_uews; i++)
        {
            if(uew[i] == (uew_struct *)ptr)
            {
                uew_ptr = uew[i];
                break;
            }
        }
        if(uew_ptr == NULL)
            return;

        uew_num = i;


	/* Update progress bar. */
	p = (double)((max > 0) ? ((double)cur / (double)max) : 0);
	PBarUpdate(
	    &uew_ptr->pbar,
	    p,
	    NULL
	);

	/* Update status message. */
	if(uew_ptr->total_objects > 0)
	    sprintf(text, "Saving %.0f%% of %i bytes.",
		p * 100,
		max
	    );
	else
            sprintf(text, "Loading %.0f%% of %i bytes.",
                p * 100,
                max
            );

	UEWDoSetStatusMesg(
	    uew_num,
	    text
	);


	return;
}


/*
 *	Objects colum list callback.
 *	Updates the objects colum list items to reflect
 *	current objects.
 */
int UEWObjectsListCLCB(void *ptr)
{
	int i, s, obj_num = -1, uew_num;
	xsw_object_struct *obj_ptr = NULL;
	uew_struct *uew_ptr = NULL;
	colum_list_struct *list;
	colum_list_item_struct *item;
	char string[80 + XSW_OBJ_NAME_MAX];


	/* Match uew_ptr. */
	for(i = 0; i < total_uews; i++)
	{
	    if(uew[i] == NULL)
		continue;

	    if(&uew[i]->objects_list == (colum_list_struct *)ptr)
	    {
		uew_ptr = uew[i];
		break;
	    }
	}
	if(uew_ptr == NULL)
	    return(0);

	uew_num = i;
	list = &uew_ptr->objects_list;



        /* Apply changes to last selected object as needed. */
        if(uew_ptr->total_sel_objects > 0)
        {
            /* Apply changes from properties prompts to last object. */
            i = uew_ptr->sel_object[uew_ptr->total_sel_objects - 1];
            if(!UEWIsObjectGarbage(uew_ptr, i))
                UEWPropsDoSetValues(uew_ptr, i);
        }


	/*   Unselect all objects, do not call UEWDoUnselectAllObjects()
	 *   or else all objects on colum list will be unselected!
	 */
        free(uew_ptr->sel_object);
        uew_ptr->sel_object = NULL;

        uew_ptr->total_sel_objects = 0;


	/* Match obj_ptr, get last selected row item. */
	for(s = 0; s < list->total_sel_rows; s++)
	{
	    i = list->sel_row[s];

	    if((i < 0) || (i >= list->total_rows))
		continue;
	    if(list->row[i] == NULL)
		continue;

	    if(list->row[i]->total_items < 1)
		continue;

	    item = list->row[i]->item[0];

	    obj_num = (int)item->client_data;
	    if(UEWIsObjectGarbage(uew_ptr, obj_num))
		continue;

	    obj_ptr = uew_ptr->object[obj_num];
	    UEWDoSelectObject(uew_num, obj_num);
	}

	if(obj_ptr != NULL)
	{
	    uew_ptr->cx = obj_ptr->x;
	    uew_ptr->cy = obj_ptr->y;
	    uew_ptr->cz = obj_ptr->z;

	    uew_ptr->sect_x = obj_ptr->sect_x;
            uew_ptr->sect_y = obj_ptr->sect_y;
            uew_ptr->sect_z = obj_ptr->sect_z;

	    /* Update status message. */
            sprintf(string, "Selected object `%s'.",
                UNVGetObjectFormalName(
                    obj_ptr,
                    obj_num
                )
            );
            UEWDoSetStatusMesg(uew_num, string);
	}

	UEWDoSetViewStatusObjectValues(uew_num, obj_ptr);


	/* Redraw as needed. */
	if(uew_ptr->map_state)
	{
	    UEWDraw(uew_num, UEW_DRAW_VIEW);
	    UEWDraw(uew_num, UEW_DRAW_PREVIEW);
	}



	return(0);
}


/*
 *	Tool bar button callbacks.
 */

int UEWTBNewPBCB(void *ptr)
{
	int i, op;
	int status = -1;


	op = UEW_MENU_CODE_NEW;

	if(ptr != NULL)
	{
	    for(i = 0; i < total_uews; i++)
	    {
		if(uew[i] == (uew_struct *)ptr)
		{
		    status = UEWMenuBarCB(uew[i], op);
		    break;
		}
	    }
	}

	return(status);
}


int UEWTBOpenPBCB(void *ptr)
{
        int i, op;
        int status = -1;


        op = UEW_MENU_CODE_OPEN;

        if(ptr != NULL)
        {
            for(i = 0; i < total_uews; i++)
            {
                if(uew[i] == (uew_struct *)ptr)
                {
                    status = UEWMenuBarCB(uew[i], op);
                    break;
                }
            }
        }

        return(status);
}


int UEWTBSavePBCB(void *ptr)
{
        int i, op;
        int status = -1;


        op = UEW_MENU_CODE_SAVE;

        if(ptr != NULL)
        {
            for(i = 0; i < total_uews; i++)
            {
                if(uew[i] == (uew_struct *)ptr)
                {
                    status = UEWMenuBarCB(uew[i], op);
                    break;
                }
            }
        }

        return(status);
}


int UEWTBCopyPBCB(void *ptr)
{
        int i, op;
        int status = -1;


        op = UEW_MENU_CODE_COPY;

        if(ptr != NULL)
        {
            for(i = 0; i < total_uews; i++)
            {
                if(uew[i] == (uew_struct *)ptr)
                {
                    status = UEWMenuBarCB(uew[i], op);
                    break;
                }
            }
        }

        return(status);
}


int UEWTBPastePBCB(void *ptr)
{
        int i, op;
        int status = -1;
        
        
        op = UEW_MENU_CODE_PASTE;
 
        if(ptr != NULL)
        {
            for(i = 0; i < total_uews; i++)
            {
                if(uew[i] == (uew_struct *)ptr)
                {
                    status = UEWMenuBarCB(uew[i], op);
                    break;
                }
            }
        }

        return(status);
}


int UEWTBNewObjPBCB(void *ptr)
{
        int i, op;
        int status = -1;


        op = UEW_MENU_CODE_INSERTNEW;

        if(ptr != NULL)
        {
            for(i = 0; i < total_uews; i++)
            {
                if(uew[i] == (uew_struct *)ptr)
                {
                    status = UEWMenuBarCB(uew[i], op);
                    break;
                }
            }
        }

        return(status);
}

int UEWTBEconomyPBCB(void *ptr)
{
        int i, op;
        int status = -1;


        op = UEW_MENU_CODE_EDITECONOMY;

        if(ptr != NULL)
        {
            for(i = 0; i < total_uews; i++)
            {
                if(uew[i] == (uew_struct *)ptr)
                {
                    status = UEWMenuBarCB(uew[i], op);
                    break;
                }
            }
        }

        return(status);
}

int UEWTBWeaponsPBCB(void *ptr)
{
        int i, op;
        int status = -1;


	op = UEW_MENU_CODE_EDITWEAPONS;

        if(ptr != NULL)
        {
            for(i = 0; i < total_uews; i++)
            {
                if(uew[i] == (uew_struct *)ptr)
                {
                    status = UEWMenuBarCB(uew[i], op);
                    break;
                }
            }
        }

        return(status);
}

int UEWTBPrintPBCB(void *ptr)
{
        int i, op;
        int status = -1;


        op = UEW_MENU_CODE_PRINT;

        if(ptr != NULL)
        {
            for(i = 0; i < total_uews; i++)
            {
                if(uew[i] == (uew_struct *)ptr)
                {
                    status = UEWMenuBarCB(uew[i], op);
                    break;
                }
            }
        }

        return(status);
}



