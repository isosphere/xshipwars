// unvedit/uewmanage.cpp
/*
                     Universe Editor Management

	Functions:

	int UEWInit(int n)
	int UEWResize(int n, bool_t force)
	int UEWDraw(int n, int amount)
	int UEWManage(int n, event_t *event)
	int UEWManageAll(event_t *event)
	void UEWMap(int n)
	void UEWUnmap(int n)
	void UEWDestroy(int n)

	---


 */
/*
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
*/
#include "blitting.h"

#include "../include/disk.h"
#include "../include/osw-x.h"
#include "../include/widget.h"

#include "../include/reality.h"
#include "../include/objects.h"
#include "../include/unvutil.h"
#include "../include/unvfile.h"

#include "ue.h"
#include "keymap.h"
#include "uew.h"

#ifndef MAX
#define MIN(a,b)        (((a) < (b)) ? (a) : (b))
#define MAX(a,b)	(((a) > (b)) ? (a) : (b))
#endif
/* #define MIN(a,b)	((a) < (b) ? (a) : (b)) */
/* #define MAX(a,b)	((a) > (b) ? (a) : (b)) */


int UEWInit(int n)
{
	int x, y, i;
	win_attr_t wattr;
        char cwd[PATH_MAX];
	uew_struct *uew_ptr;
	char title[UNV_TITLE_MAX + 80];


	if(UEWIsAllocated(n))
	    uew_ptr = uew[n];
	else
	    return(-1);


	/* Reset values. */
	uew_ptr->map_state = 0;
	uew_ptr->is_in_focus = 0;
	uew_ptr->view_is_in_focus = 0;

	if(osw_gui[0].def_geometry_set)
	{
	    uew_ptr->x = osw_gui[0].def_toplevel_x;
	    uew_ptr->y = osw_gui[0].def_toplevel_y;
	    uew_ptr->width = osw_gui[0].def_toplevel_width;
            uew_ptr->height = osw_gui[0].def_toplevel_height;
	}
	else
	{
            uew_ptr->x = 0;
            uew_ptr->y = 0;
	    uew_ptr->width = UEW_DEF_WIDTH;
	    uew_ptr->height = UEW_DEF_HEIGHT;
	}

	uew_ptr->option = 0;

	getcwd(cwd, PATH_MAX);
	cwd[PATH_MAX - 1] = '\0';

	uew_ptr->unv_file[0] = '\0';

	uew_ptr->has_changes = False;

	uew_ptr->view_pos = UEW_DEF_VIEW_X_POS;
	uew_ptr->properties_pos = UEW_DEF_PROPERTIES_X_POS;

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



	/* ******************************************************** */
	/* Create toplevel. */
	if(
	    OSWCreateWindow(
                &uew_ptr->toplevel,
                osw_gui[0].root_win,
                uew_ptr->x, uew_ptr->y,
		uew_ptr->width, uew_ptr->height
	    )
	)
            return(-1);

	sprintf(title, "%s: %s",
	    UEW_DEF_TITLE, DEF_UNIVERSE_TITLE
 	);

        OSWSetWindowWMProperties(
            uew_ptr->toplevel,
            title,
	    UEW_DEF_ICON_TITLE,
	    ((ue_image.unvedit_icon_pm == 0) ?
                widget_global.std_icon_pm : ue_image.unvedit_icon_pm),
            !osw_gui[0].def_geometry_set,	/* Let WM set coordinates? */
	    uew_ptr->x, uew_ptr->y,
	    UEW_MIN_WIDTH, UEW_MIN_HEIGHT,
	    osw_gui[0].display_width, osw_gui[0].display_height,
            WindowFrameStyleStandard,
            NULL, 0
        );

        OSWSetWindowInput(
            uew_ptr->toplevel,
            OSW_EVENTMASK_TOPLEVEL
        );


	/* Menu bar. */
	if(
	    MenuBarInit(
                &uew_ptr->mb,
                uew_ptr->toplevel,
		0, 0,
		0, UEW_MENUBAR_HEIGHT,
		UEWMenuBarCB,
                (void *)uew_ptr
	    )
        )
	    return(-1);

	/* File menu. */
	x = 0;
	y = 0;
	i = 0;
	if(
            MenuBarAddItem(
                &uew_ptr->mb, -1,
                "File",
                x, y,
                0, UEW_MENUBAR_HEIGHT
            )
	)
	    return(-1);

        MenuBarAddItemMenuItem(
            &uew_ptr->mb,
            i,
            "New",
            MENU_ITEM_TYPE_ENTRY,
            NULL,
            UEW_MENU_CODE_NEW,
            -1
        );

        MenuBarAddItemMenuItem(
            &uew_ptr->mb,
            i,
            NULL,
            MENU_ITEM_TYPE_HR,
            NULL,
            UEW_MENU_CODE_NONE,
            -1
        );

        MenuBarAddItemMenuItem(
            &uew_ptr->mb,
            i,
            "Open...",
            MENU_ITEM_TYPE_ENTRY,
            NULL,
            UEW_MENU_CODE_OPEN,
            -1
        );

        MenuBarAddItemMenuItem(
            &uew_ptr->mb,
            i,
            "Open In New Window...",
            MENU_ITEM_TYPE_ENTRY,
            NULL,
            UEW_MENU_CODE_OPENNEW,
            -1
        );

        MenuBarAddItemMenuItem(
            &uew_ptr->mb,
            i,
            "Save",
            MENU_ITEM_TYPE_ENTRY,
            NULL,
            UEW_MENU_CODE_SAVE,
            -1
        );

        MenuBarAddItemMenuItem(
            &uew_ptr->mb,
            i,
            "Save As...",
            MENU_ITEM_TYPE_ENTRY,
            NULL,
            UEW_MENU_CODE_SAVEAS,
            -1
        );

        MenuBarAddItemMenuItem(
            &uew_ptr->mb,
            i,
            NULL,
            MENU_ITEM_TYPE_HR,
            NULL,
            UEW_MENU_CODE_NONE,
            -1
        );

        MenuBarAddItemMenuItem(
            &uew_ptr->mb,
            i,
            "Print...",
            MENU_ITEM_TYPE_ENTRY,
            NULL,
            UEW_MENU_CODE_PRINT,
            -1   
        );

        MenuBarAddItemMenuItem(
            &uew_ptr->mb,
            i,
            NULL,
            MENU_ITEM_TYPE_HR,
            NULL,
            UEW_MENU_CODE_NONE,
            -1
        );

	MenuBarAddItemMenuItem(
	    &uew_ptr->mb,
	    i,
	    "Exit",
	    MENU_ITEM_TYPE_ENTRY,
	    NULL,
	    UEW_MENU_CODE_EXIT,
	    -1
	);


	/* Edit menu. */
	x += uew_ptr->mb.item[0]->width;
	i = 1;
        if(
            MenuBarAddItem(
                &uew_ptr->mb, -1,
                "Edit",
                x, y,
                0, UEW_MENUBAR_HEIGHT
            )
        )
            return(-1);

        MenuBarAddItemMenuItem(
            &uew_ptr->mb,
            i,
            "Copy",
            MENU_ITEM_TYPE_ENTRY, 
            NULL,
            UEW_MENU_CODE_COPY,
            -1
        );
        MenuBarAddItemMenuItem(
            &uew_ptr->mb,
            i,
            "Paste",
            MENU_ITEM_TYPE_ENTRY,
            NULL,
            UEW_MENU_CODE_PASTE,
            -1
        );
        MenuBarAddItemMenuItem(
            &uew_ptr->mb,
            i,     
            "Delete",
            MENU_ITEM_TYPE_ENTRY,
            NULL,
            UEW_MENU_CODE_DELETE,
            -1
        );


        /* Universe menu. */
        x += uew_ptr->mb.item[1]->width;
        i = 2;
        if(
            MenuBarAddItem(
                &uew_ptr->mb, -1,
                "Universe",
                x, y,
                0, UEW_MENUBAR_HEIGHT
            )
        )
            return(-1);

        MenuBarAddItemMenuItem(
            &uew_ptr->mb,
            i,
            "Header...",
            MENU_ITEM_TYPE_ENTRY,
            NULL,
            UEW_MENU_CODE_UNVHEADER,
            -1
        );


        /* Objects menu. */
        x += uew_ptr->mb.item[2]->width;
        i = 3;
        if(
            MenuBarAddItem(
                &uew_ptr->mb, -1,
                "Objects",
                x, y,
                0, UEW_MENUBAR_HEIGHT
            )
        )
            return(-1);

        MenuBarAddItemMenuItem(
            &uew_ptr->mb,
            i,
            "Insert New",
            MENU_ITEM_TYPE_ENTRY,
            NULL,
            UEW_MENU_CODE_INSERTNEW,
            -1
        );

        MenuBarAddItemMenuItem(
            &uew_ptr->mb,
            i,
            NULL,
            MENU_ITEM_TYPE_HR,
            NULL,
            UEW_MENU_CODE_NONE,
            -1
        );

        MenuBarAddItemMenuItem(
            &uew_ptr->mb,
            i,
            "Weapons...",
            MENU_ITEM_TYPE_ENTRY,
            NULL,
            UEW_MENU_CODE_EDITWEAPONS,
            -1
        );

        MenuBarAddItemMenuItem(
            &uew_ptr->mb,
            i,
            "Economy...",
            MENU_ITEM_TYPE_ENTRY,
            NULL,
            UEW_MENU_CODE_EDITECONOMY,
            -1
        );

        MenuBarAddItemMenuItem(
            &uew_ptr->mb,
            i,
            NULL,
            MENU_ITEM_TYPE_HR,
            NULL,
            UEW_MENU_CODE_NONE,
            -1
        );

        MenuBarAddItemMenuItem(
            &uew_ptr->mb,
            i,
            "Delete",
            MENU_ITEM_TYPE_ENTRY,
            NULL,
            UEW_MENU_CODE_DELETE,
            -1
        );


        /* Options menu. */
        x += uew_ptr->mb.item[3]->width;
        i = 4;
        if(
            MenuBarAddItem(
                &uew_ptr->mb, -1,
                "Options",
                x, y,
                0, UEW_MENUBAR_HEIGHT
            )
        )
            return(-1);
          
        MenuBarAddItemMenuItem(
            &uew_ptr->mb,
            i,
            "General Options...",
            MENU_ITEM_TYPE_ENTRY,
            NULL,
            UEW_MENU_CODE_OPT_GENERAL,
            -1
        );


        /* Windows menu. */
        x += uew_ptr->mb.item[4]->width;
        i = 5;
        if(
            MenuBarAddItem(
                &uew_ptr->mb, -1,
                "Window",
                x, y,
                0, UEW_MENUBAR_HEIGHT
            )
        )
            return(-1);

	UEWDoUpdateWindowMenus();


        /* Help menu. */
        x += uew_ptr->mb.item[5]->width;
        i = 6;
        if(
            MenuBarAddItem(
                &uew_ptr->mb, -1,
                "Help",
                x, y,
                0, UEW_MENUBAR_HEIGHT
            )
        )
            return(-1);
	/* Move help menu item to the right side. */
	uew_ptr->mb.item[6]->x = (int)uew_ptr->width -
	    (int)uew_ptr->mb.item[6]->width;

        MenuBarAddItemMenuItem(
            &uew_ptr->mb,
            i,
            "About",
            MENU_ITEM_TYPE_ENTRY,
            NULL,
            UEW_MENU_CODE_ABOUT,
            -1
        );


        /* ******************************************************** */
	/* Tool bar. */
	if(
            OSWCreateWindow(
                &uew_ptr->toolbar,
                uew_ptr->toplevel,
                0,
		UEW_MENUBAR_HEIGHT,
		uew_ptr->width,
		UEW_TOOLBAR_HEIGHT
	    )
	)
	    return(-1);
	OSWSetWindowInput(uew_ptr->toolbar, ExposureMask);

	/* Tool bar buttons. */
	x = 10;
	y = 3;
	if(
	    PBtnInit(
		&uew_ptr->tb_new_btn,
                uew_ptr->toolbar,
                x, y,
                28, 28,
                ((widget_global.force_mono) ? "N" : NULL),
                PBTN_TALIGN_CENTER,
                ue_image.tb_new,
                (void *)uew_ptr,
                UEWTBNewPBCB
	    )
        )
	    return(-1);
        PBtnSetHintMessage(
            &uew_ptr->tb_new_btn,
	    UEW_HINT_MESG_NEW
        );

	x += (28 + 4);
        if(
            PBtnInit(
                &uew_ptr->tb_open_btn,
                uew_ptr->toolbar,
                x, y,
                28, 28,
                ((widget_global.force_mono) ? "O" : NULL),
                PBTN_TALIGN_CENTER,
                ue_image.tb_open,
                (void *)uew_ptr,
                UEWTBOpenPBCB
            )
        )
            return(-1);
        PBtnSetHintMessage(
            &uew_ptr->tb_open_btn,
            UEW_HINT_MESG_OPEN
        );

        x += (28 + 4);
        if(
            PBtnInit(
                &uew_ptr->tb_save_btn,
                uew_ptr->toolbar,
                x, y,
                28, 28,
                ((widget_global.force_mono) ? "S" : NULL),
                PBTN_TALIGN_CENTER,
                ue_image.tb_save,
                (void *)uew_ptr,
                UEWTBSavePBCB
            )
        )
            return(-1);
        PBtnSetHintMessage(  
            &uew_ptr->tb_save_btn,
            UEW_HINT_MESG_SAVE
        );

        x += (28 + 10);
        if(
            PBtnInit(
                &uew_ptr->tb_copy_btn,
                uew_ptr->toolbar,
                x, y,
                28, 28,
                ((widget_global.force_mono) ? "C" : NULL),
                PBTN_TALIGN_CENTER,
                ue_image.tb_copy,
                (void *)uew_ptr,
                UEWTBCopyPBCB
            )
        )
            return(-1);
        PBtnSetHintMessage(
            &uew_ptr->tb_copy_btn,
            UEW_HINT_MESG_COPY
        );

        x += (28 + 4);
        if(
            PBtnInit(
                &uew_ptr->tb_paste_btn,
                uew_ptr->toolbar,
                x, y,
                28, 28,
                ((widget_global.force_mono) ? "P" : NULL),
                PBTN_TALIGN_CENTER,
                ue_image.tb_paste,
                (void *)uew_ptr,
                UEWTBPastePBCB
            )
        )
            return(-1);
        PBtnSetHintMessage(
            &uew_ptr->tb_paste_btn,
            UEW_HINT_MESG_PASTE
        );

        x += (28 + 10);
        if(
            PBtnInit(
                &uew_ptr->tb_newobj_btn,
                uew_ptr->toolbar,
                x, y,
                28, 28,
                ((widget_global.force_mono) ? "NOBJ" : NULL),
                PBTN_TALIGN_CENTER,
                ue_image.tb_newobj,
                (void *)uew_ptr,
                UEWTBNewObjPBCB
            )
        )
            return(-1);
        PBtnSetHintMessage(      
            &uew_ptr->tb_newobj_btn,
            UEW_HINT_MESG_NEWOBJ
        );

        x += (28 + 4);
        if(
            PBtnInit(
                &uew_ptr->tb_weapons_btn,
                uew_ptr->toolbar,
                x, y,
                28, 28,
                ((widget_global.force_mono) ? "WEP" : NULL),
                PBTN_TALIGN_CENTER,
                ue_image.tb_weapons,
                (void *)uew_ptr,
                UEWTBWeaponsPBCB
            )
        )
            return(-1);
        PBtnSetHintMessage(
            &uew_ptr->tb_weapons_btn,
            UEW_HINT_MESG_WEAPONS
        );

        x += (28 + 4);
        if(
            PBtnInit(
                &uew_ptr->tb_economy_btn,
                uew_ptr->toolbar,
                x, y,
                28, 28,
                ((widget_global.force_mono) ? "ECO" : NULL),
                PBTN_TALIGN_CENTER,
                ue_image.tb_economy,
                (void *)uew_ptr,
                UEWTBEconomyPBCB
            )
        )
            return(-1);
        PBtnSetHintMessage(
            &uew_ptr->tb_economy_btn,
            UEW_HINT_MESG_ECONOMY
        );

        x += (28 + 10);
        if(
            PBtnInit(  
                &uew_ptr->tb_print_btn,
                uew_ptr->toolbar,
                x, y,
                28, 28,
                ((widget_global.force_mono) ? "P" : NULL),
                PBTN_TALIGN_CENTER,
                ue_image.tb_print,
                (void *)uew_ptr,
                UEWTBPrintPBCB
            )
        )
            return(-1);
        PBtnSetHintMessage(
            &uew_ptr->tb_print_btn,
            UEW_HINT_MESG_PRINT
        );


        /* ******************************************************** */
        /* Objects list */
        if(
	    CListInit(
		&uew_ptr->objects_list,
		uew_ptr->toplevel,
		0, UEW_MENUBAR_HEIGHT + UEW_TOOLBAR_HEIGHT,
		MAX(
		    uew_ptr->view_pos,
		    10
		),
		MAX(
		    (int)uew_ptr->height - UEW_MENUBAR_HEIGHT -
                    UEW_TOOLBAR_HEIGHT - UEW_PREVIEW_HEIGHT -
		    UEW_STATUS_HEIGHT,
		    10
		),
		(void *)&uew_ptr->objects_list,
                UEWObjectsListCLCB
	    )
	)
	    return(-1);
	uew_ptr->objects_list.option = 0;
	uew_ptr->objects_list.option &= CL_FLAG_ALLOW_MULTI_SELECT;

	CListAddHeading(
	    &uew_ptr->objects_list,
	    "Index",
	    OSWQueryCurrentFont(),
	    widget_global.normal_text_pix,
	    0,
	    0
	);

        CListAddHeading(
            &uew_ptr->objects_list,
            "Name",
            OSWQueryCurrentFont(),
            widget_global.normal_text_pix,
            0,
            60
        );

        if(
            OSWCreateWindow(
                &uew_ptr->preview,
                uew_ptr->toplevel,
                0,
                (int)uew_ptr->height - UEW_PREVIEW_HEIGHT -
		    UEW_STATUS_HEIGHT,
		MAX(uew_ptr->view_pos, 10),
		UEW_PREVIEW_HEIGHT
            )
        )
            return(-1);
	OSWSetWindowInput(uew_ptr->preview, ExposureMask);

	OSWGetWindowAttributes(uew_ptr->preview, &wattr);
        if(
            OSWCreateSharedImage(
                &uew_ptr->isref_img,
		MAX((int)wattr.height - 20, 10),
                MAX((int)wattr.height - 20, 10)
            )   
        )  
            return(-1);


        /* ******************************************************** */
        /* View. */
        if(
            OSWCreateWindow(
                &uew_ptr->view_split_bar,
                uew_ptr->toplevel,
                uew_ptr->view_pos,
                UEW_MENUBAR_HEIGHT + UEW_TOOLBAR_HEIGHT,
                UEW_SPLIT_WIDTH,
                MAX((int)uew_ptr->height - UEW_MENUBAR_HEIGHT -
                    UEW_TOOLBAR_HEIGHT - UEW_STATUS_HEIGHT, 10
                )
            )
        )
            return(-1);
	OSWSetWindowInput(
	    uew_ptr->view_split_bar,
	    ButtonPress | ButtonRelease | ExposureMask
	);
        WidgetSetWindowCursor(uew_ptr->view_split_bar, ue_cursor.h_split);

        if(
            OSWCreateWindow(
                &uew_ptr->view_split_cur,
                uew_ptr->toplevel,
                uew_ptr->view_pos,
                UEW_MENUBAR_HEIGHT + UEW_TOOLBAR_HEIGHT,
                UEW_SPLIT_WIDTH,
                MAX((int)uew_ptr->height - UEW_MENUBAR_HEIGHT -
                    UEW_TOOLBAR_HEIGHT - UEW_STATUS_HEIGHT, 10 
                )
            )
        )
            return(-1);
	OSWSetWindowBkg(uew_ptr->view_split_cur, osw_gui[0].white_pix, 0);
	WidgetSetWindowCursor(uew_ptr->view_split_cur, ue_cursor.h_split);

	if(
	    OSWCreateWindow(
		&uew_ptr->view,
		uew_ptr->toplevel,
		uew_ptr->view_pos + UEW_SPLIT_WIDTH,
		UEW_MENUBAR_HEIGHT + UEW_TOOLBAR_HEIGHT,
		MAX(uew_ptr->properties_pos - uew_ptr->view_pos -
		    UEW_SPLIT_WIDTH, 10),
		MAX((int)uew_ptr->height - UEW_MENUBAR_HEIGHT -
                    UEW_TOOLBAR_HEIGHT - UEW_VIEW_STATUS_HEIGHT -
                    UEW_STATUS_HEIGHT, 10
		)
	    )
	)
	    return(-1);

        OSWSetWindowInput(
            uew_ptr->view,
            ButtonPressMask | ButtonReleaseMask | PointerMotionMask |
	    ExposureMask
        );
	WidgetSetWindowCursor(uew_ptr->view, ue_cursor.scanner_lock);

	OSWGetWindowAttributes(uew_ptr->view, &wattr);
	if(
	    OSWCreateSharedImage(
		&uew_ptr->view_img,
		wattr.width, wattr.height
	    )
	)
	    return(-1);

	uew_ptr->sect_x = 0;
        uew_ptr->sect_y = 0;
        uew_ptr->sect_z = 0;
        uew_ptr->cx = 0;
        uew_ptr->cy = 0;
        uew_ptr->cz = 0;
        uew_ptr->sect_width = SECTOR_SIZE_X_MAX - SECTOR_SIZE_X_MIN;
        uew_ptr->sect_height = SECTOR_SIZE_Y_MAX - SECTOR_SIZE_Y_MIN;
	uew_ptr->zoom = UEW_VIEW_DEF_ZOOM;


        /* ******************************************************** */
	/* View status. */
        if(
            OSWCreateWindow(
                &uew_ptr->view_status,
                uew_ptr->toplevel,
                uew_ptr->view_pos + UEW_SPLIT_WIDTH,
		UEW_MENUBAR_HEIGHT + UEW_TOOLBAR_HEIGHT +
                    (int)wattr.height,
                MAX(uew_ptr->properties_pos - uew_ptr->view_pos -
                    UEW_SPLIT_WIDTH, 100
		),
		UEW_VIEW_STATUS_HEIGHT
            )
        )
            return(-1);


        /* View status prompts. */
	if(
	    PromptInit(
                &uew_ptr->obj_sect_x_prompt,
                uew_ptr->view_status,
                4,
		(3 * 1) + (UEW_PROMPT_HEIGHT * 0),
		120,
		UEW_PROMPT_HEIGHT,
                PROMPT_STYLE_FLUSHED,
                "Sect X:",
                20,
		0,
		NULL
	    )
        )
	    return(-1);

        if(
            PromptInit(
                &uew_ptr->obj_sect_y_prompt,  
                uew_ptr->view_status,
                4,
                (3 * 2) + (UEW_PROMPT_HEIGHT * 1),
                120,
                UEW_PROMPT_HEIGHT,
                PROMPT_STYLE_FLUSHED,
                "Sect Y:", 
                20,
                0,
                NULL
            )
        )
            return(-1);

        if(
            PromptInit(
                &uew_ptr->obj_sect_z_prompt,
                uew_ptr->view_status,
                4,
                (3 * 3) + (UEW_PROMPT_HEIGHT * 2),
                120,
                UEW_PROMPT_HEIGHT,
                PROMPT_STYLE_FLUSHED,
                "Sect Z:",
                20,
                0,
                NULL
            )
        )
            return(-1);

        if(
            PromptInit(
                &uew_ptr->obj_cx_prompt,
                uew_ptr->view_status,
                128,
                (3 * 1) + (UEW_PROMPT_HEIGHT * 0),
                100,
                UEW_PROMPT_HEIGHT,
                PROMPT_STYLE_FLUSHED,
                "X:",
                20,
                0,
                NULL   
            )
        )
            return(-1);

        if( 
            PromptInit(
                &uew_ptr->obj_cy_prompt,
                uew_ptr->view_status,
                128,
                (3 * 2) + (UEW_PROMPT_HEIGHT * 1),
                100,
                UEW_PROMPT_HEIGHT,
                PROMPT_STYLE_FLUSHED,
                "Y:",
                20,
                0, 
                NULL
            )
        )
            return(-1);

        if( 
            PromptInit(
                &uew_ptr->obj_cz_prompt,
                uew_ptr->view_status,
                128,
                (3 * 3) + (UEW_PROMPT_HEIGHT * 2),
                100,
                UEW_PROMPT_HEIGHT,
                PROMPT_STYLE_FLUSHED,
                "Z:",
                20,
                0, 
                NULL
            )
        )
            return(-1);

        if(
            PromptInit(
                &uew_ptr->obj_name_prompt,
                uew_ptr->view_status,
                232,
                (3 * 1) + (UEW_PROMPT_HEIGHT * 0),
                130,
                UEW_PROMPT_HEIGHT,
                PROMPT_STYLE_FLUSHED,
                NULL,
                XSW_OBJ_NAME_MAX + 10,
                0,
                NULL
            )
        )
            return(-1);

        if(
            PromptInit(
                &uew_ptr->obj_heading_prompt,
                uew_ptr->view_status,
                232,
                (3 * 2) + (UEW_PROMPT_HEIGHT * 1),
                130,
                UEW_PROMPT_HEIGHT,
                PROMPT_STYLE_FLUSHED,
                "Heading:",
                20,
                0,
                NULL 
            )
        )
            return(-1);

        if( 
            PromptInit(
                &uew_ptr->obj_size_prompt,
                uew_ptr->view_status,
                232,
                (3 * 3) + (UEW_PROMPT_HEIGHT * 2),
                130,
                UEW_PROMPT_HEIGHT,
                PROMPT_STYLE_FLUSHED,
                "   Size:",
                20,
                0,
                NULL
            )
        )
            return(-1);
            

        /* ******************************************************** */
        /* Properties. */
	if(
            OSWCreateWindow(
                &uew_ptr->props_split_bar,
                uew_ptr->toplevel,
                uew_ptr->properties_pos,
                UEW_MENUBAR_HEIGHT + UEW_TOOLBAR_HEIGHT,
                UEW_SPLIT_WIDTH,
                MAX((int)uew_ptr->height - UEW_MENUBAR_HEIGHT -
                    UEW_TOOLBAR_HEIGHT - UEW_STATUS_HEIGHT, 10
                )
            )
        )
            return(-1);

        OSWSetWindowInput(
            uew_ptr->props_split_bar,
            ButtonPress | ButtonRelease | ExposureMask
        ); 
        WidgetSetWindowCursor(uew_ptr->props_split_bar, ue_cursor.h_split);

        if(
            OSWCreateWindow(
                &uew_ptr->props_split_cur,
                uew_ptr->toplevel,
                uew_ptr->properties_pos,
                UEW_MENUBAR_HEIGHT + UEW_TOOLBAR_HEIGHT,
                UEW_SPLIT_WIDTH,
                MAX((int)uew_ptr->height - UEW_MENUBAR_HEIGHT -
                    UEW_TOOLBAR_HEIGHT - UEW_STATUS_HEIGHT, 10
                )
            )
        )
            return(-1);

        OSWSetWindowBkg(uew_ptr->props_split_cur, osw_gui[0].white_pix, 0);
        WidgetSetWindowCursor(uew_ptr->props_split_cur, ue_cursor.h_split);


        if(
            OSWCreateWindow(
                &uew_ptr->props,
                uew_ptr->toplevel,
                uew_ptr->properties_pos + UEW_SPLIT_WIDTH,
                UEW_MENUBAR_HEIGHT + UEW_TOOLBAR_HEIGHT,
                MAX((int)uew_ptr->width - uew_ptr->properties_pos - 
                    UEW_SPLIT_WIDTH, 10
                ),
                MAX((int)uew_ptr->height - UEW_MENUBAR_HEIGHT -
                    UEW_TOOLBAR_HEIGHT - UEW_STATUS_HEIGHT, 10
                )
	    )
        )
	    return(-1);

	/* Prompts. */
	if(UEWPropsInit(uew_ptr))
	    return(-1);

	OSWGetWindowAttributes(uew_ptr->props, &wattr);
	if(
            SBarInit(
                &uew_ptr->props_sb, uew_ptr->props,
                wattr.width, wattr.height
	    )
        )
	    return(-1);


        /* ******************************************************** */
        /* Status bar. */
        if(
            OSWCreateWindow(
                &uew_ptr->status,
                uew_ptr->toplevel,
                0,
                (int)uew_ptr->height - UEW_STATUS_HEIGHT,
		uew_ptr->width,
		UEW_STATUS_HEIGHT
            )
        )
            return(-1);

	memset(
	    uew_ptr->status_mesg,
	    '\0',
	    UEW_STATUS_MESG_MAX
	);

	/* Progress bar. */
	if(
	    PBarInit(
                &uew_ptr->pbar,
                uew_ptr->status,
                (int)uew_ptr->width - UEW_PBAR_WIDTH - 10,
		(UEW_STATUS_HEIGHT / 2) - (UEW_PBAR_HEIGHT / 2),
		UEW_PBAR_WIDTH,
		UEW_PBAR_HEIGHT,
		0, 0, 1,
                NULL,
                PBAR_COMPLETION_HOLD
            )
	)
	    return(-1);


	/* Reset object and selected objects. */
	uew_ptr->object = NULL;
	uew_ptr->total_objects = 0;

	uew_ptr->sel_object = NULL;
	uew_ptr->total_sel_objects = 0;

	uew_ptr->isref = NULL;
	uew_ptr->total_isrefs = 0;
	uew_ptr->cur_isref = -1;


	return(0);
}


int UEWResize(int n, bool_t force)
{
	int i;
	win_attr_t wattr;
        uew_struct *uew_ptr;


        if(UEWIsAllocated(n))
            uew_ptr = uew[n];
        else
            return(-1);


	OSWGetWindowAttributes(uew_ptr->toplevel, &wattr);
	if(((unsigned int)wattr.width == uew_ptr->width) &&
	   ((unsigned int)wattr.height == uew_ptr->height) &&
           !force
	)
	    return(0);

	uew_ptr->x = wattr.x;
	uew_ptr->y = wattr.y;
	uew_ptr->width = wattr.width;
	uew_ptr->height = wattr.height;

	/* Destroy toplevel buffer. */
	OSWDestroyPixmap(&uew_ptr->toplevel_buf);


	/* Resize menu bar. */
	OSWMoveResizeWindow(
	    uew_ptr->mb.toplevel,
	    0, 0,
	    uew_ptr->width,
	    UEW_MENUBAR_HEIGHT
	);
	MenuBarResize(&uew_ptr->mb);

        /* Move help menu item to the right side. */
	i = UEW_HELP_MENU_NUM;
	if((i >= 0) && (i < uew_ptr->mb.total_items))
	{
	    if(uew_ptr->mb.item[i] != NULL)
	    {
                uew_ptr->mb.item[i]->x = (int)uew_ptr->width -
                    (int)uew_ptr->mb.item[i]->width;
	    }
	}


	/* Tool bar. */
	OSWMoveResizeWindow(
	    uew_ptr->toolbar,
            0, 
            UEW_MENUBAR_HEIGHT,
            uew_ptr->width,   
            UEW_TOOLBAR_HEIGHT
        );


	/* Objects list. */
        OSWMoveResizeWindow(
            uew_ptr->objects_list.toplevel,
            0, UEW_MENUBAR_HEIGHT + UEW_TOOLBAR_HEIGHT,
            MAX(
                uew_ptr->view_pos,
                10
            ),
            MAX(
                (int)uew_ptr->height - UEW_MENUBAR_HEIGHT -
                UEW_TOOLBAR_HEIGHT - UEW_PREVIEW_HEIGHT -
                UEW_STATUS_HEIGHT,
                10
            )
	);
	CListResize(&uew_ptr->objects_list);


	/* Preview and preview isref image. */
        OSWSyncSharedImage(uew_ptr->isref_img, uew_ptr->preview);
        OSWDestroySharedImage(&uew_ptr->isref_img);

        OSWMoveResizeWindow(
            uew_ptr->preview,
            0,
            (int)uew_ptr->height - UEW_PREVIEW_HEIGHT -
                UEW_STATUS_HEIGHT,
            MAX(uew_ptr->view_pos, 10),
            UEW_PREVIEW_HEIGHT
	); 

	/* Preview image. */
        OSWGetWindowAttributes(uew_ptr->preview, &wattr);
        if(
            OSWCreateSharedImage(
                &uew_ptr->isref_img,
                MAX((int)wattr.height - 20, 10),
                MAX((int)wattr.height - 20, 10)
            )
        )
            return(-1);


	/* View. */
        OSWMoveResizeWindow(
            uew_ptr->view_split_bar,
            uew_ptr->view_pos,
            UEW_MENUBAR_HEIGHT + UEW_TOOLBAR_HEIGHT,
            UEW_SPLIT_WIDTH,
            MAX((int)uew_ptr->height - UEW_MENUBAR_HEIGHT -  
                UEW_TOOLBAR_HEIGHT - UEW_STATUS_HEIGHT, 10
            )
        );
	/* View split cursor window. */
        OSWMoveResizeWindow(
            uew_ptr->view_split_cur,
            uew_ptr->view_pos,
            UEW_MENUBAR_HEIGHT + UEW_TOOLBAR_HEIGHT,
            UEW_SPLIT_WIDTH,
            MAX((int)uew_ptr->height - UEW_MENUBAR_HEIGHT -
                UEW_TOOLBAR_HEIGHT - UEW_STATUS_HEIGHT, 10
            )
        );

        OSWSyncSharedImage(uew_ptr->view_img, uew_ptr->view);
        OSWDestroySharedImage(&uew_ptr->view_img);

	OSWMoveResizeWindow(
            uew_ptr->view,
            uew_ptr->view_pos + UEW_SPLIT_WIDTH,
	    UEW_MENUBAR_HEIGHT + UEW_TOOLBAR_HEIGHT,
	    MAX(uew_ptr->properties_pos - uew_ptr->view_pos -
                UEW_SPLIT_WIDTH, 10),
            MAX((int)uew_ptr->height - UEW_MENUBAR_HEIGHT -
                UEW_TOOLBAR_HEIGHT - UEW_VIEW_STATUS_HEIGHT -
                UEW_STATUS_HEIGHT, 10
	    )
        );
	OSWGetWindowAttributes(uew_ptr->view, &wattr);

        if(
            OSWCreateSharedImage(
                &uew_ptr->view_img,
                wattr.width, wattr.height
            )
        )
            return(-1);


	/* View status. */
        OSWMoveResizeWindow(
            uew_ptr->view_status,
            uew_ptr->view_pos + UEW_SPLIT_WIDTH,
            UEW_MENUBAR_HEIGHT + UEW_TOOLBAR_HEIGHT +
                (int)wattr.height,
            MAX(uew_ptr->properties_pos - uew_ptr->view_pos -
                UEW_SPLIT_WIDTH, 10),
            UEW_VIEW_STATUS_HEIGHT
	);

        OSWMoveWindow(
            uew_ptr->obj_sect_x_prompt.toplevel,
            4,
            (3 * 1) + (UEW_PROMPT_HEIGHT * 0)
        );
	OSWMoveWindow(
	    uew_ptr->obj_sect_y_prompt.toplevel,
	    4,
            (3 * 2) + (UEW_PROMPT_HEIGHT * 1)
	);
        OSWMoveWindow(
            uew_ptr->obj_sect_z_prompt.toplevel,
            4,
            (3 * 3) + (UEW_PROMPT_HEIGHT * 2)
        );

        OSWMoveWindow(
            uew_ptr->obj_cx_prompt.toplevel,
            128,
            (3 * 1) + (UEW_PROMPT_HEIGHT * 0)
        );
        OSWMoveWindow(
            uew_ptr->obj_cy_prompt.toplevel,
            128,
            (3 * 2) + (UEW_PROMPT_HEIGHT * 1)
        );
        OSWMoveWindow(
            uew_ptr->obj_cz_prompt.toplevel,
            128,
            (3 * 3) + (UEW_PROMPT_HEIGHT * 2)
        );      

        OSWMoveWindow(
            uew_ptr->obj_name_prompt.toplevel,
            232,
            (3 * 1) + (UEW_PROMPT_HEIGHT * 0)
        );
        OSWMoveWindow(
            uew_ptr->obj_heading_prompt.toplevel,
            232,
            (3 * 2) + (UEW_PROMPT_HEIGHT * 1)
        );   
        OSWMoveWindow(
            uew_ptr->obj_size_prompt.toplevel,
            232,
            (3 * 3) + (UEW_PROMPT_HEIGHT * 2)
        );



        /* Properties. */
        OSWMoveResizeWindow(
            uew_ptr->props_split_bar,
            uew_ptr->properties_pos,
            UEW_MENUBAR_HEIGHT + UEW_TOOLBAR_HEIGHT,
            UEW_SPLIT_WIDTH,
            MAX((int)uew_ptr->height - UEW_MENUBAR_HEIGHT -
                UEW_TOOLBAR_HEIGHT - UEW_STATUS_HEIGHT, 10
            )
        );
        /* Properties split cursor window. */
        OSWMoveResizeWindow(
            uew_ptr->props_split_cur,
            uew_ptr->properties_pos,
            UEW_MENUBAR_HEIGHT + UEW_TOOLBAR_HEIGHT,
            UEW_SPLIT_WIDTH,
            MAX((int)uew_ptr->height - UEW_MENUBAR_HEIGHT -
                UEW_TOOLBAR_HEIGHT - UEW_STATUS_HEIGHT, 10
            )
        );

        OSWMoveResizeWindow(
            uew_ptr->props,
            uew_ptr->properties_pos + UEW_SPLIT_WIDTH,
            UEW_MENUBAR_HEIGHT + UEW_TOOLBAR_HEIGHT,
            MAX((int)uew_ptr->width - uew_ptr->properties_pos -
                UEW_SPLIT_WIDTH, 10
	    ),
            MAX((int)uew_ptr->height - UEW_MENUBAR_HEIGHT -
                UEW_TOOLBAR_HEIGHT - UEW_STATUS_HEIGHT, 10
            )
        );
	OSWGetWindowAttributes(uew_ptr->props, &wattr);

	/* Property prompts. */
	UEWPropsResize(uew_ptr);

	SBarResize(&uew_ptr->props_sb, wattr.width, wattr.height);

	/* Need to redraw property scroll bar and prompts. */
        SBarDraw(
            &uew_ptr->props_sb, 
            wattr.width,
            wattr.height,
            wattr.width,
            TOTAL_PROP_PROMPTS * UEW_PROP_PROMPT_HEIGHT 
        );
        UEWPropsDraw(uew_ptr, 0);




        /* Status bar. */
        OSWMoveResizeWindow(
            uew_ptr->status,
            0,                
            (int)uew_ptr->height - UEW_STATUS_HEIGHT,
            uew_ptr->width, 
            UEW_STATUS_HEIGHT
	);

	/* Progress bar. */
	OSWMoveWindow(    
            uew_ptr->pbar.toplevel,
	    (int)uew_ptr->width - UEW_PBAR_WIDTH - 10,
            (UEW_STATUS_HEIGHT / 2) - (UEW_PBAR_HEIGHT / 2)
	);
	

	return(0);
}


int UEWDraw(int n, int amount)
{
	int i, len;
	double zoom;
	win_t w;
	pixmap_t pixmap;
	win_attr_t wattr;
        uew_struct *uew_ptr;
	shared_image_t *simg_ptr;
	isref_struct *isref_ptr;
	WColorStruct color;

        
        if(UEWIsAllocated(n))
            uew_ptr = uew[n];
        else
            return(-1);


	/* Map as needed. */
	if(!uew_ptr->map_state)
	{
	    OSWMapRaised(uew_ptr->toplevel);

	    MenuBarMap(&uew_ptr->mb);

	    OSWMapWindow(uew_ptr->toolbar);
            PBtnMap(&uew_ptr->tb_new_btn);
            PBtnMap(&uew_ptr->tb_open_btn);
            PBtnMap(&uew_ptr->tb_save_btn);
            PBtnMap(&uew_ptr->tb_copy_btn);
            PBtnMap(&uew_ptr->tb_paste_btn);
            PBtnMap(&uew_ptr->tb_newobj_btn);
            PBtnMap(&uew_ptr->tb_weapons_btn);   
            PBtnMap(&uew_ptr->tb_economy_btn); 
            PBtnMap(&uew_ptr->tb_print_btn);

	    CListMap(&uew_ptr->objects_list);
	    OSWMapWindow(uew_ptr->preview);

	    OSWMapWindow(uew_ptr->view);
	    OSWMapWindow(uew_ptr->view_split_bar);
	    OSWUnmapWindow(uew_ptr->view_split_cur);

	    OSWMapWindow(uew_ptr->view_status);
	    PromptMap(&uew_ptr->obj_sect_x_prompt);
	    PromptMap(&uew_ptr->obj_sect_y_prompt);
            PromptMap(&uew_ptr->obj_sect_z_prompt);
            PromptMap(&uew_ptr->obj_cx_prompt);   
            PromptMap(&uew_ptr->obj_cy_prompt);
            PromptMap(&uew_ptr->obj_cz_prompt);
            PromptMap(&uew_ptr->obj_name_prompt);
            PromptMap(&uew_ptr->obj_heading_prompt);
            PromptMap(&uew_ptr->obj_size_prompt);

            OSWMapWindow(uew_ptr->props);
            OSWMapWindow(uew_ptr->props_split_bar);
            OSWUnmapWindow(uew_ptr->props_split_cur);
	    UEWPropsDraw(uew_ptr, 0);
	    OSWGetWindowAttributes(uew_ptr->props, &wattr);
	    SBarDraw(
		&uew_ptr->props_sb,
		wattr.width,
		wattr.height,
		wattr.width,
		TOTAL_PROP_PROMPTS * UEW_PROP_PROMPT_HEIGHT
	    );

	    OSWMapWindow(uew_ptr->status);
	    PBarMap(&uew_ptr->pbar);

	    uew_ptr->map_state = 1;
	}


	/* Recreate buffers as needed. */
	OSWGetWindowAttributes(uew_ptr->toplevel, &wattr);
	if(uew_ptr->toplevel_buf == 0)
	{
	    if(
		OSWCreatePixmap(
		    &uew_ptr->toplevel_buf,
		    wattr.width, wattr.height
		)
	    )
		return(-1);
	}

	/* ******************************************************* */
        /* Redraw preview. */
        if((amount == UEW_DRAW_COMPLETE) ||
           (amount == UEW_DRAW_PREVIEW)
        )
	{
            w = uew_ptr->preview;
            pixmap = uew_ptr->toplevel_buf;

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
            }


	    simg_ptr = uew_ptr->isref_img;
	    i = uew_ptr->cur_isref;

	    if((i < 0) || (i >= uew_ptr->total_isrefs))
		isref_ptr = NULL;
	    else
		isref_ptr = uew_ptr->isref[i];

	    if(ISRefIsLoaded(isref_ptr) &&
               (simg_ptr != NULL) &&
               option.show_preview_image
	    )
	    {
		isref_ptr = uew_ptr->isref[i];
		if(isref_ptr->fwidth > isref_ptr->fheight)
		    zoom = (double)simg_ptr->width /
			((double)isref_ptr->fwidth *
                         isref_ptr->magnification);
		else
		    zoom = (double)simg_ptr->height /
                        ((double)isref_ptr->fheight *
                         isref_ptr->magnification);
		if(zoom > 1)
		    zoom = 1;

		color.a = 0x00;
		color.r = 0x00;
		color.g = 0x00;
		color.b = 0x00;
		BlitBufSolid(
		    osw_gui[0].depth,
		    simg_ptr->data,
		    simg_ptr->width, simg_ptr->height,
		    color
		);

		BlitBufAbsolute(
		    osw_gui[0].depth,
		    simg_ptr->data,
		    isref_ptr->image_data,
		    static_cast<int>(((int)simg_ptr->width / 2) -
                       (((int)isref_ptr->fwidth / 2
                       * isref_ptr->magnification) * zoom)),
                    static_cast<int>(((int)simg_ptr->height / 2) -
                       (((int)isref_ptr->fheight / 2
                       * isref_ptr->magnification) * zoom)),
		    simg_ptr->width, simg_ptr->height,
		    0, isref_ptr->fheight * uew_ptr->ani_frame,
		    isref_ptr->width, isref_ptr->height,
		    isref_ptr->fwidth, isref_ptr->fheight,
		    zoom,
		    isref_ptr->magnification
		);

		OSWPutSharedImageToDrawablePos(
		    simg_ptr, pixmap,
		    (int)wattr.width - (int)simg_ptr->width - 10,
		    10
		);
	    }


            WidgetFrameButtonPixmap(
                pixmap,
                False, 
                wattr.width, wattr.height,
                (widget_global.force_mono) ?
                    osw_gui[0].white_pix :  
                    widget_global.surface_highlight_pix,
                (widget_global.force_mono) ?
                    osw_gui[0].white_pix :  
                    widget_global.surface_shadow_pix
            );

            OSWPutBufferToWindow(w, pixmap);
	}

        /* ******************************************************* */
        /* Tool bar. */
        if((amount == UEW_DRAW_COMPLETE) ||
           (amount == UEW_DRAW_TOOLBAR)
        )
	{
            w = uew_ptr->toolbar;
            pixmap = uew_ptr->toplevel_buf;

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
            }

            WidgetFrameButtonPixmap(  
                pixmap,
                False,
                wattr.width, wattr.height,
                (widget_global.force_mono) ?
                    osw_gui[0].white_pix :
                    widget_global.surface_highlight_pix,
                (widget_global.force_mono) ?
                    osw_gui[0].white_pix :
                    widget_global.surface_shadow_pix
            );

            OSWPutBufferToWindow(w, pixmap);
        }


	/* ******************************************************* */
	/* Redraw view. */
        if(amount == UEW_DRAW_COMPLETE)
        {
            w = uew_ptr->view_split_bar;
            pixmap = uew_ptr->toplevel_buf;

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
	    }

            WidgetFrameButtonPixmap(
                pixmap,
                False,
                wattr.width, wattr.height,
                (widget_global.force_mono) ?
                    osw_gui[0].white_pix :
                    widget_global.surface_highlight_pix,
                (widget_global.force_mono) ?
                    osw_gui[0].white_pix :
                    widget_global.surface_shadow_pix
            );

            OSWPutBufferToWindow(w, pixmap);
	}

	if((amount == UEW_DRAW_COMPLETE) ||
           (amount == UEW_DRAW_VIEW)
	)
	{
	    UEWViewDraw(n);
	}


        /* ******************************************************* */
	/* Redraw view status. */
	if((amount == UEW_DRAW_COMPLETE) ||
           (amount == UEW_DRAW_VIEW_STATUS)
        )
        {
            w = uew_ptr->view_status;
            pixmap = uew_ptr->toplevel_buf;

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
            }

            WidgetFrameButtonPixmap(
                pixmap,
                False,
                wattr.width, wattr.height,
                (widget_global.force_mono) ?
                    osw_gui[0].white_pix :
                    widget_global.surface_highlight_pix,
                (widget_global.force_mono) ?
                    osw_gui[0].white_pix :
                    widget_global.surface_shadow_pix
            );

            OSWPutBufferToWindow(w, pixmap);
	}


        /* ******************************************************* */
        /* Redraw properties. */
        if(amount == UEW_DRAW_COMPLETE)
        {
            w = uew_ptr->props_split_bar;
            pixmap = uew_ptr->toplevel_buf;

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
            }

            WidgetFrameButtonPixmap(
                pixmap,
                False,
                wattr.width, wattr.height,
                (widget_global.force_mono) ?
                    osw_gui[0].white_pix :
                    widget_global.surface_highlight_pix,
                (widget_global.force_mono) ?
                    osw_gui[0].white_pix :
                    widget_global.surface_shadow_pix
            );

            OSWPutBufferToWindow(w, pixmap);
        }



        /* ******************************************************* */
        /* Redraw status. */
        if((amount == UEW_DRAW_COMPLETE) ||
           (amount == UEW_DRAW_STATUS)
        )
        {
            w = uew_ptr->status;
            pixmap = uew_ptr->toplevel_buf;  

            OSWGetWindowAttributes(w, &wattr);
 
            /* Redraw background. */
            if(widget_global.force_mono)
            {
                OSWClearPixmap(
                    pixmap,
                    wattr.width, wattr.height,
                    osw_gui[0].black_pix
                );
                OSWSetFgPix(osw_gui[0].white_pix);
            }
            else
            {
                WidgetPutImageTile(
                    pixmap,
                    widget_global.std_bkg_img,
                    wattr.width, wattr.height
                );
		OSWSetFgPix(widget_global.normal_text_pix);
	    }


	    len = (int)uew_ptr->width - UEW_PBAR_WIDTH - 40;

	    /* Draw status message. */
	    OSWDrawStringLimited(
		pixmap,
		8, (UEW_STATUS_HEIGHT / 2) + 5,
		uew_ptr->status_mesg,
		MIN((int)strlen(uew_ptr->status_mesg),
                    len / 7
		)
	    );

            WidgetFrameButtonPixmap(
	        pixmap,
                False,
                wattr.width, wattr.height,
		(widget_global.force_mono) ?
		    osw_gui[0].white_pix :
		    widget_global.surface_highlight_pix,
		(widget_global.force_mono) ?
                    osw_gui[0].white_pix :
                    widget_global.surface_shadow_pix
	    );

            OSWPutBufferToWindow(w, pixmap);
        }








	return(0);
}


int UEWManage(int n, event_t *event)
{
	int i, s, j, x, y;
	double cx, cy, cz;
	long sect_x, sect_y, sect_z;
	int obj_num;
	xsw_object_struct *obj_ptr;
	prompt_window_struct *prompt;
	keycode_t keycode;
	int events_handled = 0;
        uew_struct *uew_ptr;
	char *strptr;
	char stringa[256];

	static bool_t	view_trans_state,
			view_zoom_state,
			view_moveobj_state;
	static bool_t   view_pos_drag, properties_pos_drag;
	static bool_t	alt_key_state,
			ctrl_key_state,
			shift_key_state;
        static int last_x, last_y;
	static uew_struct *last_uew_ptr;

        
        if(UEWIsAllocated(n))
            uew_ptr = uew[n];
        else
            return(events_handled);


	if(event == NULL)
	    return(events_handled);

	if(!uew_ptr->map_state &&
	   (event->type != MapNotify)
	)
	    return(events_handled);


	/* Reset schedualed uew for deletion. */
	delete_uew = NULL;


	switch(event->type)
	{
          /* ******************************************************** */
	  case KeyPress:
            /* Do not continue further if now in focus. */
            if(!uew_ptr->is_in_focus)
                return(events_handled);

	    keycode = event->xkey.keycode;

	    /* Zoom. */
	    if(keycode == keymap[KM_VIEW_ZOOM].keycode)
	    {
		alt_key_state = True;

                /* Set cursor on view. */
                WidgetSetWindowCursor(
                    uew_ptr->view,
                    ue_cursor.zoom
                );

		if(uew_ptr->view_is_in_focus)
		{
		    UEWDoSetStatusMesg(n,
   "Move pointer up or down on view window to adjust zoom."
		    );
		}

                /* Switch keyboard auto repeat off. */
                OSWKBAutoRepeatOff();

                /* Return if this handled key is not a modifier. */
                if(!OSWIsModifierKey(keycode))
                {
                    events_handled++;
                    return(events_handled);
                }
	    }
	    /* Move object. */
            else if(keycode == keymap[KM_VIEW_MOVE].keycode)
            {
                shift_key_state = True;

		if(uew_ptr->view_is_in_focus)
		{
		    if(uew_ptr->total_sel_objects > 0)
                        UEWDoSetStatusMesg(n,
  "Move pointer in any direction on view window to move object."
                        );
		    else
                        UEWDoSetStatusMesg(n,
  "You must select an object first before you can move them."
                        );
		}

                /* Switch keyboard auto repeat off. */
                OSWKBAutoRepeatOff();

                /* Return if this handled key is not a modifier. */
                if(!OSWIsModifierKey(keycode))
                {
                    events_handled++;
                    return(events_handled);
                }
            }
            /* Translate. */
            else if(keycode == keymap[KM_VIEW_TRANSLATE].keycode)
            {
                ctrl_key_state = True;

                /* Set cursor on view. */
                WidgetSetWindowCursor(
                    uew_ptr->view,
                    ue_cursor.translate
                );

                if(uew_ptr->view_is_in_focus)
                {
                    UEWDoSetStatusMesg(n,
  "Move pointer in any direction on view window to translate position."
                    );
		}

                /* Switch keyboard auto repeat off. */
                OSWKBAutoRepeatOff();

                /* Return if this handled key is not a modifier. */
                if(!OSWIsModifierKey(keycode))
                {
                    events_handled++;
                    return(events_handled);
                }
            }



	    /* Left. */
	    if((keycode == osw_keycode.cursor_left) &&
	       uew_ptr->view_is_in_focus
	    )
	    {
		uew_ptr->cx -= 50;
		for(i = 0; uew_ptr->cx < SECTOR_SIZE_X_MIN; i++)
		{
		    uew_ptr->cx += uew_ptr->sect_width;
		}
		uew_ptr->sect_x -= i;

		UEWDraw(n, UEW_DRAW_VIEW);
		events_handled++;
		return(events_handled);
	    }
	    /* Right. */
            else if((keycode == osw_keycode.cursor_right) &&
               uew_ptr->view_is_in_focus
	    )
            {
                uew_ptr->cx += 50; 
                for(i = 0; uew_ptr->cx > SECTOR_SIZE_X_MAX; i++)
                {
                    uew_ptr->cx -= uew_ptr->sect_width;
                }
                uew_ptr->sect_x += i;
             
                UEWDraw(n, UEW_DRAW_VIEW);
                events_handled++;
                return(events_handled);
            }
	    /* Up. */
            else if((keycode == osw_keycode.cursor_up) &&
               uew_ptr->view_is_in_focus
	    )
            {
                uew_ptr->cy += 50;
                for(i = 0; uew_ptr->cy > SECTOR_SIZE_Y_MAX; i++)
                {
                    uew_ptr->cy -= uew_ptr->sect_height;
                }
                uew_ptr->sect_y += i;

                UEWDraw(n, UEW_DRAW_VIEW);
                events_handled++;
                return(events_handled);
            }
	    /* Down. */
            else if((keycode == osw_keycode.cursor_down) &&
               uew_ptr->view_is_in_focus
            )
            {
                uew_ptr->cy -= 50;
                for(i = 0; uew_ptr->cy < SECTOR_SIZE_Y_MIN; i++)
                {
                    uew_ptr->cy += uew_ptr->sect_height;
                }
                uew_ptr->sect_y -= i;

                UEWDraw(n, UEW_DRAW_VIEW);
                events_handled++;
                return(events_handled);
            }
            /* Zoom in. */
            else if((keycode == osw_keycode.equal) &&
               uew_ptr->view_is_in_focus
            )
            {
                uew_ptr->zoom = uew_ptr->zoom + 0.02;
                if(uew_ptr->zoom > UEW_VIEW_ZOOM_MAX)
                    uew_ptr->zoom = UEW_VIEW_ZOOM_MAX;

                UEWDraw(n, UEW_DRAW_VIEW);
                events_handled++;
                return(events_handled);
            }
	    /* Zoom out. */
	    else if((keycode == osw_keycode.minus) &&
               uew_ptr->view_is_in_focus
            )
            {
                uew_ptr->zoom = uew_ptr->zoom - 0.02;
		if(uew_ptr->zoom < UEW_VIEW_ZOOM_MIN)
		    uew_ptr->zoom = UEW_VIEW_ZOOM_MIN;

                UEWDraw(n, UEW_DRAW_VIEW);
                events_handled++;
                return(events_handled);
	    }

	    /* Enter (apply changes from focused prompt). */
            else if((keycode == osw_keycode.enter) ||
                    (keycode == osw_keycode.np_enter)
	    )
            {
		/* Apply prompt values. */
		if(uew_ptr->obj_sect_x_prompt.is_in_focus)
		{
		    strptr = uew_ptr->obj_sect_x_prompt.buf;
		    i = UEWGetLastSelectedObject(uew_ptr);
		    if((i > -1) && (strptr != NULL))
		    {
			UEWPropsDoSetValues(uew_ptr, i);

			obj_ptr = uew_ptr->object[i];
			obj_ptr->sect_x = atol(strptr);
                        UEWPropsDoGetValues(uew_ptr, i);
		    }

                    events_handled++;
		}
		else if(uew_ptr->obj_sect_y_prompt.is_in_focus)
                {
                    strptr = uew_ptr->obj_sect_y_prompt.buf;
                    i = UEWGetLastSelectedObject(uew_ptr);
                    if((i > -1) && (strptr != NULL))  
                    {
                        UEWPropsDoSetValues(uew_ptr, i);

                        obj_ptr = uew_ptr->object[i];
                        obj_ptr->sect_y = atol(strptr);
                        UEWPropsDoGetValues(uew_ptr, i);
                    }

                    events_handled++;
                }
                else if(uew_ptr->obj_sect_z_prompt.is_in_focus)
                {
                    strptr = uew_ptr->obj_sect_z_prompt.buf;
                    i = UEWGetLastSelectedObject(uew_ptr);
                    if((i > -1) && (strptr != NULL))  
                    {
                        UEWPropsDoSetValues(uew_ptr, i);

                        obj_ptr = uew_ptr->object[i];
                        obj_ptr->sect_z = atol(strptr);
                        UEWPropsDoGetValues(uew_ptr, i);
                    }

                    events_handled++;
                }
                else if(uew_ptr->obj_cx_prompt.is_in_focus)
                {
                    strptr = uew_ptr->obj_cx_prompt.buf;
                    i = UEWGetLastSelectedObject(uew_ptr);
                    if((i > -1) && (strptr != NULL))
                    {
                        UEWPropsDoSetValues(uew_ptr, i);

                        obj_ptr = uew_ptr->object[i];
                        obj_ptr->x = atof(strptr);
                        UEWPropsDoGetValues(uew_ptr, i);
                    }

                    events_handled++;
                }
                else if(uew_ptr->obj_cy_prompt.is_in_focus)
                {
                    strptr = uew_ptr->obj_cy_prompt.buf;
                    i = UEWGetLastSelectedObject(uew_ptr);
                    if((i > -1) && (strptr != NULL))
                    {
                        UEWPropsDoSetValues(uew_ptr, i);

                        obj_ptr = uew_ptr->object[i];
                        obj_ptr->y = atof(strptr);
                        UEWPropsDoGetValues(uew_ptr, i);
                    }

                    events_handled++;
                }
                else if(uew_ptr->obj_cz_prompt.is_in_focus)
                {
                    strptr = uew_ptr->obj_cz_prompt.buf;
                    i = UEWGetLastSelectedObject(uew_ptr);
                    if((i > -1) && (strptr != NULL))
                    {
                        UEWPropsDoSetValues(uew_ptr, i);

                        obj_ptr = uew_ptr->object[i];
                        obj_ptr->z = atof(strptr);
                        UEWPropsDoGetValues(uew_ptr, i);
                    }

		    events_handled++;
                }
                else if(uew_ptr->obj_name_prompt.is_in_focus)
                {
                    strptr = uew_ptr->obj_name_prompt.buf;
                    i = UEWGetLastSelectedObject(uew_ptr);
                    if((i > -1) && (strptr != NULL))
                    {
                        UEWPropsDoSetValues(uew_ptr, i);

                        obj_ptr = uew_ptr->object[i];
			strncpy(
			    obj_ptr->name,
			    strptr,
			    XSW_OBJ_NAME_MAX
			);
			obj_ptr->name[XSW_OBJ_NAME_MAX - 1] = '\0';
                        UEWPropsDoGetValues(uew_ptr, i);
                    }

		    UEWUpdateObjectListItemName(
			uew_ptr,
			i
		    );
		    CListDraw(&uew_ptr->objects_list, CL_DRAW_AMOUNT_LIST);

                    events_handled++;
                }
                else if(uew_ptr->obj_heading_prompt.is_in_focus)
                {
                    strptr = uew_ptr->obj_heading_prompt.buf;
                    i = UEWGetLastSelectedObject(uew_ptr);
                    if((i > -1) && (strptr != NULL))
                    {
                        UEWPropsDoSetValues(uew_ptr, i);

                        obj_ptr = uew_ptr->object[i];
                        obj_ptr->heading = atof(strptr) * (PI / 180);
                        UEWPropsDoGetValues(uew_ptr, i);
                    }

                    events_handled++;
                }
                else if(uew_ptr->obj_size_prompt.is_in_focus)
                {
                    strptr = uew_ptr->obj_size_prompt.buf;
                    i = UEWGetLastSelectedObject(uew_ptr);
                    if((i > -1) && (strptr != NULL))
                    {
                        UEWPropsDoSetValues(uew_ptr, i);

                        obj_ptr = uew_ptr->object[i];
                        obj_ptr->size = atol(strptr);
			UEWPropsDoGetValues(uew_ptr, i);
                    }

                    events_handled++;
                }


                if(events_handled > 0)
		{
		    if(!uew_ptr->has_changes)
			uew_ptr->has_changes = True;

                    UEWDraw(n, UEW_DRAW_VIEW);
		}

		/* Don't let anything else handle the enter key. */
		return(events_handled);
            }
            /* Insert new object. */
            else if(keycode == osw_keycode.insert)
            {
		/* Actual action is performed on release. */
                events_handled++;
                return(events_handled);
            }
	    /* Ctrl key shortcuts. */
	    else if(osw_gui[0].ctrl_key_state)
	    {
                /* Actual action is performed on release. */

		/* Open universe. */
		if(keycode == osw_keycode.alpha_o)
		{
                    events_handled++;
		    return(events_handled);
		}
                /* Save universe. */
                else if(keycode == osw_keycode.alpha_s)
                {
                    events_handled++;
                    return(events_handled);
                }
                /* Copy selected object. */
                else if(keycode == osw_keycode.alpha_c)
                {
                    events_handled++;
                    return(events_handled);
                }
                /* Paste object. */
                else if(keycode == osw_keycode.alpha_v)
                {
                    events_handled++;
                    return(events_handled);
                }
                /* Insert object (see below, not in ctrl state case). */
                /* Edit object weapons. */
                else if(keycode == osw_keycode.alpha_w)
                {
                    events_handled++;
                    return(events_handled);
		}
                /* Edit object economy. */
                else if(keycode == osw_keycode.alpha_e)
                {
                    events_handled++;
                    return(events_handled);
                }
                /* Print universe. */
                else if(keycode == osw_keycode.alpha_p)
                {
                    events_handled++;
                    return(events_handled);
                }
	    }
	    break;

	  /* ******************************************************** */
          case KeyRelease:
            /* Do not continue further if now in focus. */
            if(!uew_ptr->is_in_focus)
                return(events_handled);

	    keycode = event->xkey.keycode;

            /* Zoom. */
            if(keycode == keymap[KM_VIEW_ZOOM].keycode)
            {
                alt_key_state = False;

                /* Set back cursor on view as needed. */
                WidgetSetWindowCursor(
                    uew_ptr->view,
                    ue_cursor.scanner_lock
                );

                if(uew_ptr->view_is_in_focus)
                {
                    UEWDoSetStatusMesg(n, NULL);
		}

		/* Switch keyboard auto repeat back on as needed. */
		if(!alt_key_state &&
                   !shift_key_state &&
                   !ctrl_key_state
		)
		    OSWKBAutoRepeatOn();

		/* Return if this handled key is not a modifier. */
                if(!OSWIsModifierKey(keycode))
		{
		    events_handled++;
                    return(events_handled);
		}
            }
            /* Move object. */
            else if(keycode == keymap[KM_VIEW_MOVE].keycode)
            {
                shift_key_state = False;

                if(uew_ptr->view_is_in_focus)
                {
                    UEWDoSetStatusMesg(n, NULL);
		}

                /* Switch keyboard auto repeat back on as needed. */
                if(!alt_key_state && 
                   !shift_key_state &&
                   !ctrl_key_state
                )
                    OSWKBAutoRepeatOn();

                /* Return if this handled key is not a modifier. */
                if(!OSWIsModifierKey(keycode))
                {
                    events_handled++;
                    return(events_handled);
                }
            }
            /* Translate. */
            else if(keycode == keymap[KM_VIEW_TRANSLATE].keycode)
            {
                ctrl_key_state = False;

                /* Set back cursor on view as needed. */
                WidgetSetWindowCursor(
                    uew_ptr->view,
                    ue_cursor.scanner_lock
                );

                if(uew_ptr->view_is_in_focus)
                {
                    UEWDoSetStatusMesg(n, NULL);
		}

                /* Switch keyboard auto repeat back on as needed. */
                if(!alt_key_state &&
                   !shift_key_state &&
                   !ctrl_key_state
                )
                    OSWKBAutoRepeatOn();

                /* Return if this handled key is not a modifier. */
                if(!OSWIsModifierKey(keycode))
                {
                    events_handled++;
                    return(events_handled);
                }
            }

            /* Insert new object. */
            else if(keycode == osw_keycode.insert)
            {
                UEWMenuBarCB(
                    uew_ptr,
                    UEW_MENU_CODE_INSERTNEW
                );

                events_handled++;
                return(events_handled);
            }
            /* Ctrl key shortcuts. */
            else if(osw_gui[0].ctrl_key_state)
            {
                /* Open universe. */
                if(keycode == osw_keycode.alpha_o)  
                {
                    UEWMenuBarCB(
                        uew_ptr,
                        UEW_MENU_CODE_OPEN
                    );
                
                    events_handled++;
                    return(events_handled);
                }
                /* Save universe. */
                else if(keycode == osw_keycode.alpha_s)
                {
                    UEWMenuBarCB(
                        uew_ptr,
                        UEW_MENU_CODE_SAVE
                    );
                
                    events_handled++;
                    return(events_handled);
                }
                /* Copy selected object. */
                else if(keycode == osw_keycode.alpha_c)
                {
                    UEWMenuBarCB(
                        uew_ptr,
                        UEW_MENU_CODE_COPY
                    );
                      
                    events_handled++;
                    return(events_handled);   
                }
                /* Paste object. */ 
                else if(keycode == osw_keycode.alpha_v)
                {
                    UEWMenuBarCB(
                        uew_ptr,
                        UEW_MENU_CODE_PASTE
                    );
                
                    events_handled++;
                    return(events_handled);
                }
                /* Insert object (see below, not in ctrl state case). */
                /* Edit object weapons. */
                else if(keycode == osw_keycode.alpha_w)
                {
                    UEWMenuBarCB(
                        uew_ptr,
                        UEW_MENU_CODE_EDITWEAPONS
                    );
                    
                    events_handled++;
                    return(events_handled);
                }
                /* Edit object economy. */
                else if(keycode == osw_keycode.alpha_e)
                {
                    UEWMenuBarCB(
                        uew_ptr,
                        UEW_MENU_CODE_EDITECONOMY
                    );
                    
                    events_handled++;
                    return(events_handled);
                }
                /* Print universe. */
                else if(keycode == osw_keycode.alpha_p)
                {
                    UEWMenuBarCB(
                        uew_ptr,
                        UEW_MENU_CODE_PRINT
                    );
                    
                    events_handled++;
                    return(events_handled);
                }
	    }
            break;

          /* ******************************************************** */
	  case ButtonPress:

	    /* Reset view window focus. */
	    if(uew_ptr->view_is_in_focus)
		uew_ptr->view_is_in_focus = 0;

	    /* View. */
            if(event->xany.window == uew_ptr->view)
	    {
		uew_ptr->view_is_in_focus = 1;

		/* Translate. */
		if(ctrl_key_state)
		{
                    view_trans_state = True;
                    last_x = event->xbutton.x;
		    last_y = event->xbutton.y;
		    last_uew_ptr = uew_ptr;
		}
		/* Zoom. */
		else if(alt_key_state)
		{
                    view_zoom_state = True;
                    last_x = event->xbutton.x;
                    last_y = event->xbutton.y;
                    last_uew_ptr = uew_ptr;
		}
		/* Move object. */
		else if(shift_key_state)
		{
		    view_moveobj_state = True;
                    last_x = event->xbutton.x;
                    last_y = event->xbutton.y;
                    last_uew_ptr = uew_ptr;
		}
		/* Select object. */
		else
		{
		    i = UEWViewSelectObject(
			n,
			event->xbutton.x,
			event->xbutton.y
		    );
		    if(i > -1)
		    {
			UEWDoUnselectAllObjects(n);
			UEWDoSelectObject(n, i);

			/* Update view status prompts. */
			UEWDoSetViewStatusObjectValues(
			    n,
			    uew_ptr->object[i]
			);
                        /* Scroll objects colum list. */
                        UEWObjectListScrollToItem(uew_ptr, i);


			sprintf(stringa, "Selected object `%s'.",
			    UNVGetObjectFormalName(
				uew_ptr->object[i],
				i
			    )
			);
                        UEWDoSetStatusMesg(n, stringa);

                        CListDraw(&uew_ptr->objects_list, CL_DRAW_AMOUNT_LIST);

			/* Redraw preview. */
			UEWDraw(n, UEW_DRAW_PREVIEW);
		    }
		    else
		    {
			UEWDoUnselectAllObjects(n);

			UEWDoSetViewStatusObjectValues(
                            n,
                            NULL
			);

			CListDraw(&uew_ptr->objects_list, CL_DRAW_AMOUNT_LIST);

                        /* Redraw preview. */
                        UEWDraw(n, UEW_DRAW_PREVIEW);
		    }
		}

                UEWDraw(n, UEW_DRAW_VIEW);
                events_handled++;
                return(events_handled);
	    }
            /* View pane split position drag start. */
            else if(event->xany.window == uew_ptr->view_split_bar)
            {
                view_pos_drag = True;
		last_uew_ptr = uew_ptr;

		OSWMapRaised(uew_ptr->view_split_cur);

		OSWGrabPointer(
                    uew_ptr->toplevel,
                    True,
                    ButtonReleaseMask | PointerMotionMask,
                    GrabModeAsync, GrabModeAsync,
		    uew_ptr->toplevel,
                    ((ue_cursor.h_split == NULL) ? None :
                        ue_cursor.h_split->cursor)
                );
	    }
	    /* Properties pane split position drag start. */
            else if(event->xany.window == uew_ptr->props_split_bar)
            {
                properties_pos_drag = True;
                last_uew_ptr = uew_ptr;

                OSWMapRaised(uew_ptr->props_split_cur);

                OSWGrabPointer(
                    uew_ptr->toplevel,
                    True,
                    ButtonReleaseMask | PointerMotionMask,
                    GrabModeAsync, GrabModeAsync,
                    uew_ptr->toplevel,
                    ((ue_cursor.h_split == NULL) ? None :
                        ue_cursor.h_split->cursor)
                );
            }
	    break;

          /* ******************************************************** */
          case ButtonRelease:

	    /* Allow MenuBarManage to handle button release first. */
            events_handled += MenuBarManage(&uew_ptr->mb, event);
	    if(events_handled > 0)
		return(events_handled);

            /* View pane split drag. */
            if((last_uew_ptr == uew_ptr) &&
               view_pos_drag
	    )
            {
		uew_ptr->view_pos = event->xbutton.x;
		if(uew_ptr->view_pos >= (uew_ptr->properties_pos - 10))
		    uew_ptr->view_pos = uew_ptr->properties_pos - 10;


		view_pos_drag = False;
		last_uew_ptr = NULL;
		OSWUnmapWindow(uew_ptr->view_split_cur);
		OSWUngrabPointer();
		UEWResize(n, True);

		events_handled++;
	    }
	    /* Properties pane split drag. */
            else if((last_uew_ptr == uew_ptr) &&
	            properties_pos_drag
	    )
	    {
                uew_ptr->properties_pos = event->xbutton.x;
                if(uew_ptr->properties_pos >= ((int)uew_ptr->width - 10))
                    uew_ptr->properties_pos = (int)uew_ptr->width - 10;

                properties_pos_drag = False;
                last_uew_ptr = NULL;
                OSWUnmapWindow(uew_ptr->props_split_cur);
                OSWUngrabPointer();
                UEWResize(n, True);

                events_handled++;
	    }
	    /* View. */
            else if(event->xany.window == uew_ptr->view)    
            {
		/* Translate. */
		if(view_trans_state)
		{

		    view_trans_state = False;
		}

		/* Zoom. */
		if(view_zoom_state)
		{

		    view_zoom_state = False;
		}

		/* Move object. */
                if(view_moveobj_state)
                {
                    i = UEWGetLastSelectedObject(uew_ptr);
                    if(i > -1)
		    {
                        /* Tempory get current position of object. */
			obj_ptr = uew_ptr->object[i];
			cx = obj_ptr->x;
                        cy = obj_ptr->y;
                        cz = obj_ptr->z;
                        sect_x = obj_ptr->sect_x;
                        sect_y = obj_ptr->sect_y;
                        sect_z = obj_ptr->sect_z;

                        UEWPropsDoSetValues(uew_ptr, i);

			obj_ptr->x = cx;
                        obj_ptr->y = cy;
                        obj_ptr->z = cz;
                        obj_ptr->sect_x = sect_x;
                        obj_ptr->sect_y = sect_y;
                        obj_ptr->sect_z = sect_z;

                        UEWPropsDoGetValues(uew_ptr, i);
		    }

                    view_moveobj_state = False;
                }


                last_x = 0;
                last_y = 0;
                last_uew_ptr = NULL;

                UEWDraw(n, UEW_DRAW_VIEW);
                events_handled++;
                return(events_handled);
            }
            break;

          /* ******************************************************** */
          case MotionNotify:
            /* View pane split drag. */ 
            if((last_uew_ptr == uew_ptr) &&
               view_pos_drag
	    )
            {
                x = event->xmotion.x;
                y = event->xmotion.y;  

		OSWMoveWindow(
		    uew_ptr->view_split_cur,
		    x - (UEW_SPLIT_WIDTH / 2),
		    UEW_MENUBAR_HEIGHT + UEW_TOOLBAR_HEIGHT
		);
            }
	    /* Properties pane split drag. */
	    else if((last_uew_ptr == uew_ptr) &&
                    properties_pos_drag
            )
	    {
                x = event->xmotion.x;
                y = event->xmotion.y;

                OSWMoveWindow(
                    uew_ptr->props_split_cur,
                    x - (UEW_SPLIT_WIDTH / 2),
                    UEW_MENUBAR_HEIGHT + UEW_TOOLBAR_HEIGHT
                );
	    }
	    /* View. */
            else if(event->xany.window == uew_ptr->view)
            {
		x = event->xmotion.x;
		y = event->xmotion.y;

                /* Translate. */
                if(view_trans_state &&
                   (uew_ptr == last_uew_ptr)
		)
                {
		    /* Move coordinates. */
                    uew_ptr->cx -= ((x - last_x) / uew_ptr->zoom);
                    uew_ptr->cy += ((y - last_y) / uew_ptr->zoom);

		    /* Change to sectors. */
                    for(i = 0; uew_ptr->cx > SECTOR_SIZE_X_MAX; i++)
                    {
                        uew_ptr->cx -= uew_ptr->sect_width;
                    }
                    uew_ptr->sect_x += i;
                    for(i = 0; uew_ptr->cx < SECTOR_SIZE_X_MIN; i++)
                    {
                        uew_ptr->cx += uew_ptr->sect_width;
                    }
                    uew_ptr->sect_x -= i;

                    for(i = 0; uew_ptr->cy > SECTOR_SIZE_Y_MAX; i++)
                    {
                        uew_ptr->cy -= uew_ptr->sect_height;
                    }
                    uew_ptr->sect_y += i;
                    for(i = 0; uew_ptr->cy < SECTOR_SIZE_Y_MIN; i++)
                    {
                        uew_ptr->cy += uew_ptr->sect_height;
                    }
                    uew_ptr->sect_y -= i;


                    events_handled++;                 
                }
                
                /* Zoom. */
                if(view_zoom_state &&
                   (uew_ptr == last_uew_ptr)
		)
                {
		    uew_ptr->zoom += ((y - last_y) * 0.0008);

                    if(uew_ptr->zoom > UEW_VIEW_ZOOM_MAX)
                        uew_ptr->zoom = UEW_VIEW_ZOOM_MAX;
                    if(uew_ptr->zoom < UEW_VIEW_ZOOM_MIN)
			uew_ptr->zoom = UEW_VIEW_ZOOM_MIN;
                    
		    events_handled++;                    
                }
          
                /* Move object. */
                if(view_moveobj_state &&
                   (uew_ptr == last_uew_ptr)
                )
                {
		    /* Move all selected objects. */
		    for(s = 0; s < uew_ptr->total_sel_objects; s++)
		    {
			obj_num = uew_ptr->sel_object[s];
			if(UEWIsObjectGarbage(uew_ptr, obj_num))
			    continue;

			obj_ptr = uew_ptr->object[obj_num];

                        /* Move coordinates. */ 
                        obj_ptr->x += ((x - last_x) / uew_ptr->zoom);
                        obj_ptr->y -= ((y - last_y) / uew_ptr->zoom);

                        /* Change to sectors. */
                        for(i = 0; obj_ptr->x > SECTOR_SIZE_X_MAX; i++)
                        {
                            obj_ptr->x -= uew_ptr->sect_width;
                        }
                        obj_ptr->sect_x += i;
                        for(i = 0; obj_ptr->x < SECTOR_SIZE_X_MIN; i++)
                        {
                            obj_ptr->x += uew_ptr->sect_width;
                        }
                        obj_ptr->sect_x -= i;

                        for(i = 0; obj_ptr->y > SECTOR_SIZE_Y_MAX; i++)
                        {
                            obj_ptr->y -= uew_ptr->sect_height;
                        }
                        obj_ptr->sect_y += i;
                        for(i = 0; obj_ptr->y < SECTOR_SIZE_Y_MIN; i++)
                        {
                            obj_ptr->y += uew_ptr->sect_height;
                        }
                        obj_ptr->sect_y -= i;


			/* Update has changes mark as needed. */
			if(!uew_ptr->has_changes)
			    uew_ptr->has_changes = True;


			/* Need to update view status prompt values. */
			if(s == (uew_ptr->total_sel_objects - 1))
			{
			    prompt = &uew_ptr->obj_sect_x_prompt;
                            strptr = prompt->buf;
			    if(strptr != NULL)
			        sprintf(strptr, "%ld", obj_ptr->sect_x);
			    PromptUnmarkBuffer(prompt, PROMPT_POS_END);
			    PromptDraw(prompt, PROMPT_DRAW_AMOUNT_TEXTONLY);

                            prompt = &uew_ptr->obj_sect_y_prompt;
                            strptr = prompt->buf;
                            if(strptr != NULL)
                                sprintf(strptr, "%ld", obj_ptr->sect_y);
                            PromptUnmarkBuffer(prompt, PROMPT_POS_END);
                            PromptDraw(prompt, PROMPT_DRAW_AMOUNT_TEXTONLY);

                            prompt = &uew_ptr->obj_sect_z_prompt;
                            strptr = prompt->buf;
                            if(strptr != NULL)
                                sprintf(strptr, "%ld", obj_ptr->sect_z);
                            PromptUnmarkBuffer(prompt, PROMPT_POS_END);
                            PromptDraw(prompt, PROMPT_DRAW_AMOUNT_TEXTONLY);


                            prompt = &uew_ptr->obj_cx_prompt;
                            strptr = prompt->buf;
                            if(strptr != NULL)
                                sprintf(strptr, "%.2f", obj_ptr->x);
                            PromptUnmarkBuffer(prompt, PROMPT_POS_END);
                            PromptDraw(prompt, PROMPT_DRAW_AMOUNT_TEXTONLY);

                            prompt = &uew_ptr->obj_cy_prompt;
                            strptr = prompt->buf;
                            if(strptr != NULL)
                                sprintf(strptr, "%.2f", obj_ptr->y);
                            PromptUnmarkBuffer(prompt, PROMPT_POS_END);
                            PromptDraw(prompt, PROMPT_DRAW_AMOUNT_TEXTONLY);

                            prompt = &uew_ptr->obj_cz_prompt;
                            strptr = prompt->buf;
                            if(strptr != NULL)
                                sprintf(strptr, "%.2f", obj_ptr->z);
                            PromptUnmarkBuffer(prompt, PROMPT_POS_END);
                            PromptDraw(prompt, PROMPT_DRAW_AMOUNT_TEXTONLY);
			}
		    }

                    events_handled++;
                }

		/* Update last positions. */
		last_x = x;
		last_y = y;

		/* Redraw as needed. */
		if(events_handled > 0)
                    UEWDraw(n, UEW_DRAW_VIEW);

                return(events_handled);
	    }
	    break;

          /* ******************************************************** */
          case Expose:
	    if(event->xany.window == uew_ptr->toplevel)
            {
		UEWDraw(n, UEW_DRAW_COMPLETE);

		events_handled++;
		return(events_handled);
	    }
            else if(event->xany.window == uew_ptr->toolbar)
            {
                UEWDraw(n, UEW_DRAW_TOOLBAR);

                events_handled++;
                return(events_handled);
            }
            else if(event->xany.window == uew_ptr->view_split_bar)
            {
                UEWDraw(n, UEW_DRAW_COMPLETE);

                events_handled++;
                return(events_handled);
            }
            else if(event->xany.window == uew_ptr->props_split_bar)
            {
                UEWDraw(n, UEW_DRAW_COMPLETE);

                events_handled++;
                return(events_handled);
            }
            else if(event->xany.window == uew_ptr->view)
            {
                UEWDraw(n, UEW_DRAW_VIEW);

                events_handled++;
                return(events_handled);
            }
            else if(event->xany.window == uew_ptr->status)
            {
                UEWDraw(n, UEW_DRAW_STATUS);

                events_handled++;
                return(events_handled);
            }
	    break;

	  /* ******************************************************** */
          case UnmapNotify:
	    if(event->xany.window == uew_ptr->toplevel)
	    {
		UEWUnmap(n);
		events_handled++;
                return(events_handled);
	    }
	    break;

          /* ******************************************************** */
          case MapNotify:
            if(event->xany.window == uew_ptr->toplevel)
            {
		if(!uew_ptr->map_state)
                    UEWMap(n);
                events_handled++;
                return(events_handled);
            }
	    break;

          /* ******************************************************** */
          case ConfigureNotify:
	    if(event->xany.window == uew_ptr->toplevel)
            {
		UEWResize(n, False);

		events_handled++;
                return(events_handled);
	    }
	    break;

          /* ******************************************************** */
          case FocusIn:
            if(event->xany.window == uew_ptr->toplevel)
            {
                uew_ptr->is_in_focus = 1;
                events_handled++;
            }
            break;

           /* ******************************************************* */
           case FocusOut:
            if(event->xany.window == uew_ptr->toplevel)
            {
		uew_ptr->view_is_in_focus = 0;
                uew_ptr->is_in_focus = 0;
                events_handled++;
            }
            break;

          /* ******************************************************** */
          case ClientMessage:
            if(OSWIsEventDestroyWindow(uew_ptr->toplevel, event))
            {
		/* Comferm exit if it has changes? */
                if(uew_ptr->has_changes)
                {
		    comfwin.option &= ~ComfirmOptionAll;
		    comfwin.option |= ComfirmOptionCancel;
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

		/* Schedual this universe edit window for deletion. */
		delete_uew = uew_ptr;

		events_handled++;
		return(events_handled);
	    }
	    break;
	}


	/* Redraw as needed. */
	if(events_handled > 0)
	{
	    UEWDraw(n, UEW_DRAW_COMPLETE);
	}


	/* ******************************************************* */
	/* Manage widgets. */

	/* Menu bar. */
	events_handled += MenuBarManage(&uew_ptr->mb, event);

	/* Objects list. */
	if(events_handled == 0)
	{
	    /* Record previous selected objects list item. */
	    s = uew_ptr->objects_list.total_sel_rows;
	    if(s > 0)
		i = uew_ptr->objects_list.sel_row[s - 1];
	    else
		i = -1;

	    events_handled += CListManage(&uew_ptr->objects_list, event);
	    if(events_handled > 0)
	    {
		s = uew_ptr->objects_list.total_sel_rows;
                if(s > 0)
                    j = uew_ptr->objects_list.sel_row[s - 1];
                else
                    j = -1;

		if(i != j)
	            UEWObjectsListCLCB(&uew_ptr->objects_list);
	    }
	}


	/* View status prompts. */
	if(events_handled == 0)
	    events_handled += PromptManage(&uew_ptr->obj_sect_x_prompt, event);
        if(events_handled == 0)
            events_handled += PromptManage(&uew_ptr->obj_sect_y_prompt, event);
        if(events_handled == 0)
            events_handled += PromptManage(&uew_ptr->obj_sect_z_prompt, event);
        if(events_handled == 0)
            events_handled += PromptManage(&uew_ptr->obj_cx_prompt, event);
        if(events_handled == 0)
            events_handled += PromptManage(&uew_ptr->obj_cy_prompt, event);
        if(events_handled == 0)
            events_handled += PromptManage(&uew_ptr->obj_cz_prompt, event);
        if(events_handled == 0)
            events_handled += PromptManage(&uew_ptr->obj_name_prompt, event);
        if(events_handled == 0)
            events_handled += PromptManage(&uew_ptr->obj_heading_prompt, event);
        if(events_handled == 0)
            events_handled += PromptManage(&uew_ptr->obj_size_prompt, event);


	/* Tool bar buttons. */
        if(events_handled == 0)
	    PBtnManage(&uew_ptr->tb_new_btn, event);
        if(events_handled == 0)
            PBtnManage(&uew_ptr->tb_open_btn, event);
        if(events_handled == 0)
            PBtnManage(&uew_ptr->tb_save_btn, event);
        if(events_handled == 0)
            PBtnManage(&uew_ptr->tb_copy_btn, event);
        if(events_handled == 0)
            PBtnManage(&uew_ptr->tb_paste_btn, event);
        if(events_handled == 0)
            PBtnManage(&uew_ptr->tb_newobj_btn, event);
        if(events_handled == 0)
            PBtnManage(&uew_ptr->tb_weapons_btn, event);
        if(events_handled == 0)
            PBtnManage(&uew_ptr->tb_economy_btn, event);
        if(events_handled == 0)
            PBtnManage(&uew_ptr->tb_print_btn, event);


	/* Progress bar. */
        if(events_handled == 0)
            events_handled += PBarManage(&uew_ptr->pbar, event);

        /* Property prompts. */
        if(events_handled == 0)
            events_handled += UEWPropsManage(uew_ptr, event);



	return(events_handled);
}


int UEWManageAll(event_t *event)
{
	int i, uews_deleted;
	int events_handled = 0;


	if(event == NULL)
	    return(events_handled);


	/* Manage each universe editor window. */
	for(i = 0, uews_deleted = 0; i < total_uews; i++)
	{
	    if(uew[i] == NULL)
		continue;

	    events_handled += UEWManage(i, event);

	    /*   Somewhere in above management call require this
	     *   uew to be deleted?
	     */
	    if(delete_uew == uew[i])
	    {
		UEWDelete(i);
		uews_deleted++;
		delete_uew = NULL;
	    }
	}

	/* Update window menus as needed. */
	if(uews_deleted > 0)
	    UEWDoUpdateWindowMenus();


	return(events_handled);
}


void UEWMap(int n)
{
        uew_struct *uew_ptr;


        if(UEWIsAllocated(n))
            uew_ptr = uew[n];
        else
            return;


	uew_ptr->map_state = 0;
	UEWDraw(n, UEW_DRAW_COMPLETE);


	return;
}


void UEWUnmap(int n)
{
        uew_struct *uew_ptr;


        if(UEWIsAllocated(n))
            uew_ptr = uew[n];
        else
            return;


	PBarUnmap(&uew_ptr->pbar);
	OSWUnmapWindow(uew_ptr->status);

        OSWUnmapWindow(uew_ptr->props_split_cur);
        OSWUnmapWindow(uew_ptr->props_split_bar);
        OSWUnmapWindow(uew_ptr->props);

	PromptUnmap(&uew_ptr->obj_cx_prompt);
        PromptUnmap(&uew_ptr->obj_cy_prompt);
        PromptUnmap(&uew_ptr->obj_cz_prompt);
        PromptUnmap(&uew_ptr->obj_sect_x_prompt);
        PromptUnmap(&uew_ptr->obj_sect_y_prompt);
        PromptUnmap(&uew_ptr->obj_sect_z_prompt);
        PromptUnmap(&uew_ptr->obj_name_prompt);
        PromptUnmap(&uew_ptr->obj_heading_prompt);
        PromptUnmap(&uew_ptr->obj_size_prompt);
        OSWUnmapWindow(uew_ptr->view_status);

        OSWUnmapWindow(uew_ptr->view_split_cur);
        OSWUnmapWindow(uew_ptr->view_split_bar);
        OSWUnmapWindow(uew_ptr->view);

	CListUnmap(&uew_ptr->objects_list);

        PBtnUnmap(&uew_ptr->tb_new_btn);
        PBtnUnmap(&uew_ptr->tb_open_btn);
        PBtnUnmap(&uew_ptr->tb_save_btn);
        PBtnUnmap(&uew_ptr->tb_copy_btn);
        PBtnUnmap(&uew_ptr->tb_paste_btn);
        PBtnUnmap(&uew_ptr->tb_newobj_btn);
        PBtnUnmap(&uew_ptr->tb_weapons_btn);
        PBtnUnmap(&uew_ptr->tb_economy_btn);
        PBtnUnmap(&uew_ptr->tb_print_btn);
	OSWUnmapWindow(uew_ptr->toolbar);

	MenuBarUnmap(&uew_ptr->mb);

	OSWUnmapWindow(uew_ptr->toplevel);



	uew_ptr->map_state = 0;


	return;
}


void UEWDestroy(int n)
{
        uew_struct *uew_ptr;


        if(UEWIsAllocated(n))
            uew_ptr = uew[n];
        else
            return;


	/* Delete all property prompts. */
	UEWPropsDestroy(uew_ptr);


	/* Unload all isrefs. */
	ISRefDeleteAll(uew_ptr->isref, uew_ptr->total_isrefs);
	uew_ptr->isref = NULL;
	uew_ptr->total_isrefs = 0;


	/* Unselect all objects. */
	UEWDoUnselectAllObjects(n);


	/* Deallocate all objects. */
	UNVDeleteAllObjects(uew_ptr->object, uew_ptr->total_objects);
	uew_ptr->object = NULL;
	uew_ptr->total_objects = 0;


	/* Deallocate GUI resources. */

	/* Status bar. */
	PBarDestroy(&uew_ptr->pbar);
	OSWDestroyWindow(&uew_ptr->status);

	/* Properties. */
	SBarDestroy(&uew_ptr->props_sb);
	OSWDestroyWindow(&uew_ptr->props_split_cur);
        OSWDestroyWindow(&uew_ptr->props_split_bar);
        OSWDestroyWindow(&uew_ptr->props);

	/* View and view status. */
        OSWDestroyWindow(&uew_ptr->view_split_cur);
        OSWDestroyWindow(&uew_ptr->view_split_bar);
	PromptDestroy(&uew_ptr->obj_size_prompt);
        PromptDestroy(&uew_ptr->obj_heading_prompt);
        PromptDestroy(&uew_ptr->obj_name_prompt);
        PromptDestroy(&uew_ptr->obj_sect_z_prompt);
        PromptDestroy(&uew_ptr->obj_sect_y_prompt);
        PromptDestroy(&uew_ptr->obj_sect_x_prompt);
        PromptDestroy(&uew_ptr->obj_cz_prompt);
        PromptDestroy(&uew_ptr->obj_cy_prompt);
        PromptDestroy(&uew_ptr->obj_cx_prompt);
	OSWDestroyWindow(&uew_ptr->view_status);

	OSWSyncSharedImage(uew_ptr->view_img, uew_ptr->view);
	OSWDestroySharedImage(&uew_ptr->view_img);
	OSWDestroyWindow(&uew_ptr->view);

	/* Objects list. */
	CListDestroy(&uew_ptr->objects_list);
        OSWSyncSharedImage(uew_ptr->isref_img, uew_ptr->preview);
        OSWDestroySharedImage(&uew_ptr->isref_img);
	OSWDestroyWindow(&uew_ptr->preview);

	/* Tool bar. */
	PBtnDestroy(&uew_ptr->tb_new_btn);
        PBtnDestroy(&uew_ptr->tb_open_btn);
        PBtnDestroy(&uew_ptr->tb_save_btn);
        PBtnDestroy(&uew_ptr->tb_copy_btn);
        PBtnDestroy(&uew_ptr->tb_paste_btn);
        PBtnDestroy(&uew_ptr->tb_newobj_btn);
        PBtnDestroy(&uew_ptr->tb_weapons_btn);
        PBtnDestroy(&uew_ptr->tb_economy_btn);
        PBtnDestroy(&uew_ptr->tb_print_btn);
	OSWDestroyWindow(&uew_ptr->toolbar);

	/* Menu bar. */
	MenuBarDestroy(&uew_ptr->mb);

	OSWDestroyPixmap(&uew_ptr->toplevel_buf);
        OSWDestroyWindow(&uew_ptr->toplevel);



	return;
}




