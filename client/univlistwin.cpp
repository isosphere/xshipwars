/*
                         Universe List Window

	Functions:

	int ULWIsEntryInList(int n)
	int ULWIsListItemInUniv(int n)
	image_t *ULWGetProperListItemIcon(
		univ_entry_struct *unv_entry_ptr
	)

	void UnivEntrySyncWithList()
	univ_entry_struct *UnivListGetUnivEntryFromList(
            list_window_struct *list
	)
	int UnivListDoConnect()
	int UnivListDoSelect(void *ptr)

  	int UnivListInit()
 	int UnivListDraw(int amount)
 	int UnivListManage(event_t *event)
 	void UnivListMap()
 	void UnivListUnmap()
 	void UnivListWinDestroy()

 	int UnivListMenuCB(void *client_data, int op_code)

	---

 */

#include "univlist.h"
#include "xsw.h"


#define MIN(a,b)        (((a) < (b)) ? (a) : (b))
#define MAX(a,b)        (((a) > (b)) ? (a) : (b))


/* Minimum sizes. */
#define UNIV_LIST_MIN_WIDTH		240
#define UNIV_LIST_MIN_HEIGHT		180


/* Width and height to initially start with. */
#define UNIV_LIST_DEF_WIDTH		320
#define UNIV_LIST_DEF_HEIGHT		480


/* Menu bar. */
#define UNIV_LIST_MB_HEIGHT		30

/* Preview window height. */
#define UNIV_LIST_PREVIEW_HEIGHT	32


/* Character sizes in pixels. */
#define UNIV_LIST_CHAR_WIDTH	7
#define UNIV_LIST_CHAR_HEIGHT	14

/* Titles. */
#define UNIV_ICON_TITLE		"Universes"

/* Message on status window when nothing is selected. */
#define STATUS_NOTHING_SELECTED_MESSAGE	\
"Right click on entry for menu."



/* Menu codes. */
#define UNIV_LIST_MENU_CODE_NONE	0
#define UNIV_LIST_MENU_CODE_CONNECT	1
#define UNIV_LIST_MENU_CODE_INSERT	2
#define UNIV_LIST_MENU_CODE_COPY	3
#define UNIV_LIST_MENU_CODE_DELETE	4
#define UNIV_LIST_MENU_CODE_PROPERTIES	5
#define UNIV_LIST_MENU_CODE_SAVE	6
#define UNIV_LIST_MENU_CODE_CLOSE	7



xsw_univ_list_win_struct univ_list_win;


/*
 *	Checks if the universe entry number n is in the list
 *	widget's items.
 */
int ULWIsEntryInList(int n)
{
	int i;
	void *ptr;
	list_window_struct *list;


	list = &univ_list_win.list;


	if(UnivIsAllocated(n))
	    ptr = univ_entry[n];
	else
	    return(0);


	/* Scan through list widget's entries. */
	for(i = 0; i < list->total_entries; i++)
	{
	    if(list->entry[i] == NULL)
		continue;

	    if(ptr == list->entry[i]->data_ptr)
		return(1);
	}

	return(0);
}

/*
 *	Checks if the list widget item n points to a valid universe
 *	entry.
 */
int ULWIsListItemInUniv(int n)
{
        int i;
        list_window_struct *list;


        list = &univ_list_win.list;

        if(!ListIsItemAllocated(list, n))
            return(0);

	/* Go through universe entries. */    
        for(i = 0; i < total_univ_entries; i++)
        {
	    if(univ_entry[i] == NULL)
		continue;

            if((void *)univ_entry[i] == list->entry[n]->data_ptr)
                return(1);
        }


        return(0);
}

/*
 *	Returns the proper icon as an image pointer for the given
 *	universe entry.
 */
image_t *ULWGetProperListItemIcon(
	univ_entry_struct *unv_entry_ptr
)
{
	int i = IMG_CODE_UNIV_STD_ICON;
	char *strptr;
	image_t *image_ptr = NULL;


	if(unv_entry_ptr == NULL)
	    return(image_ptr);


	/* Never connected? */
	if(unv_entry_ptr->last_connected <= 0)
	{
	    i = IMG_CODE_UNIV_UNKNOWN_ICON;
	}
	/* Older than week since last connected? */
	else if((cur_systime - unv_entry_ptr->last_connected) >
                (86400 * 7)
	)
	{
	    i = IMG_CODE_UNIV_OLD_ICON;
	}
	/* Has login information? */
	else
	{
	    strptr = StringParseName(unv_entry_ptr->url);
            if(strptr != NULL)
            {
                if(strcmp(strptr, DEF_GUEST_LOGIN_NAME))
		    i = IMG_CODE_UNIV_HASLOGIN_ICON;
	    }
	}

	if(IMGIsImageNumAllocated(i))
	    image_ptr = xsw_image[i]->image;


	return(image_ptr);
}

/*
 *	Syncronizes universe list window's list items with
 *	actual universe entries.
 */
void UnivEntrySyncWithList(void)
{
	int list_entry_num, univ_entry_num;
	image_t *unv_img = NULL;
	list_window_struct *list;
	char text[256];


	list = &univ_list_win.list;

	if(IMGIsImageNumAllocated(IMG_CODE_UNIV_STD_ICON))
	    unv_img = xsw_image[IMG_CODE_UNIV_STD_ICON]->image;


	/* Make sure universe list and list have same number of entries. */

	/* Add item to list if there are extra univ entries. */
	while(list->total_entries < total_univ_entries)
	{
	    sprintf(text,
"Total universe entries %i is less than %i referances on list, fixing...",
		total_univ_entries,
		list->total_entries
	    );
	    MesgAdd(text, xsw_color.standard_text);

	    /* Find a univ entry not on list. */
	    for(univ_entry_num = 0;
	        univ_entry_num < total_univ_entries;
	        univ_entry_num++
            )
	    {
		if(!ULWIsEntryInList(univ_entry_num))
		    break;
	    }
	    if(univ_entry_num < total_univ_entries)
	    {
		ListAddItem(
                    list,
                    LIST_ENTRY_TYPE_NORMAL,
                    univ_entry[univ_entry_num]->alias,
                    unv_img,
		    -1,             /* Append. */
                    univ_entry[univ_entry_num]
		);
	    }
	    else
	    {
		break;
	    }
	}

        /* Remove item from list if there are more items then univ entries. */
        while(list->total_entries > total_univ_entries)
        {
            sprintf(text,
"Total universe entries %i is greater than %i referances on list, fixing...",
                total_univ_entries,
                list->total_entries
            );
            MesgAdd(text, xsw_color.standard_text);

            /* Find a list entry not pointing to a univ entry. */
            for(list_entry_num = 0;
                list_entry_num < list->total_entries;
                list_entry_num++
            )
            {
                if(!ULWIsListItemInUniv(list_entry_num))
                    break;
            }
            if(list_entry_num < list->total_entries)
	    {
		/* Delete a list item. */
		ListDeleteItem(list, list_entry_num);
	    }
            else
            {
                break;
            }
	}


	/* Realign/resort universe list with list. */

        /* Set pointers from list. */
        for(list_entry_num = 0;
            list_entry_num < list->total_entries;
            list_entry_num++
        )
        {
            if(list_entry_num >= total_univ_entries)
                break;

	    if(list->entry[list_entry_num] == NULL)
		continue;

            univ_entry[list_entry_num] = (univ_entry_struct *)
                list->entry[list_entry_num]->data_ptr;
        }

	return;
}

/*
 *	Get the selected item's pointed universe entry on the
 *	list widget.
 */
univ_entry_struct *UnivListGetUnivEntryFromList(
	list_window_struct *list
)
{
	int i;
	univ_entry_struct *ptr = NULL;


	if(list == NULL)
	    return(ptr);

        /* Get selected list item position. */
        i = list->entry_pos;

        /* Check if selected entry on list is valid. */
        if(ListIsItemAllocated(list, i))
            ptr = (univ_entry_struct *)list->entry[i]->data_ptr;

	return(ptr);
}

/*
 *	Procedure for universe list window to do connect.
 */
int UnivListDoConnect()
{
	int i;
	list_window_struct *list;
	list_window_entry_struct *list_item_ptr;
	univ_entry_struct *univ_entry_ptr;


	/* Unmap universe list. */
        UnivListUnmap();


	/* Get pointer to list widget and its selected item. */
        list = &univ_list_win.list;
        i = list->entry_pos;
	if(ListIsItemAllocated(list, i))
	    list_item_ptr = list->entry[i];
	else
	    return(-1);


	/* Get selected entry position. */
	univ_entry_ptr = UnivListGetUnivEntryFromList(list);
	if(univ_entry_ptr == NULL)
	    return(-1);


	/* 'Touch' entry, set current systems time. */
	univ_entry_ptr->last_connected = time(NULL);

	/* Increment number of times connected. */
	univ_entry_ptr->times_connected++;


	/* Update icon on selected item on list widget. */
	list_item_ptr->image = ULWGetProperListItemIcon(
	    univ_entry_ptr
	);


	/* Go through standard procedure for connecting. */
	XSWDoConnect(univ_entry_ptr->url);


	return(0);
}


/*
 *	Select callback handler for list window.
 */
int UnivListDoSelect(void *ptr)
{
	UnivListDoConnect();

	return(0);
}


/*
 *	Initializes universe list window.
 */
int UnivListInit()
{
	int i;
        pixmap_t pixmap;
        image_t *image_ptr;
	list_window_struct *list;
	char title[256];


	/* Reset values. */
        univ_list_win.map_state = 0;
        univ_list_win.visibility_state = VisibilityFullyObscured;
	univ_list_win.is_in_focus = 0;

	/* Positions may have been fetched from configuration file,
	 * check if they are valid and sanitize.
	 */
	if(univ_list_win.width < 100)
	    univ_list_win.width = UNIV_LIST_DEF_WIDTH;
	if(univ_list_win.height < 100)
	    univ_list_win.height = UNIV_LIST_DEF_HEIGHT;


        /* Create toplevel. */
	if(
	    OSWCreateWindow(
	        &univ_list_win.toplevel,
                osw_gui[0].root_win,
                univ_list_win.x,
                univ_list_win.y,
                univ_list_win.width,
                univ_list_win.height
	    )
        )
	    return(-1);

        OSWSetWindowInput(univ_list_win.toplevel, OSW_EVENTMASK_TOPLEVEL);

        /* WM properties. */
        sprintf(title, "%s: Universes", PROG_NAME);
        if(IMGIsImageNumAllocated(IMG_CODE_UNIV_ICON))
        {
            pixmap = OSWCreatePixmapFromImage(
                xsw_image[IMG_CODE_UNIV_ICON]->image
            );
        }   
        else
        {
            pixmap = widget_global.std_icon_pm;
        }
        OSWSetWindowWMProperties(
            univ_list_win.toplevel,
            title,		/* Title. */
            UNIV_ICON_TITLE,	/* Icon title. */
            pixmap,		/* Icon. */
            False,		/* Let WM set coordinates? */
            univ_list_win.x, univ_list_win.y,
            UNIV_LIST_MIN_WIDTH, UNIV_LIST_MIN_HEIGHT,
	    osw_gui[0].display_width, osw_gui[0].display_height,
            WindowFrameStyleStandard,
            NULL, 0
        );


	/* Status window. */
	if(
            OSWCreateWindow(
                &univ_list_win.status,
                univ_list_win.toplevel,
                0,
                (int)univ_list_win.height - UNIV_LIST_PREVIEW_HEIGHT,
                univ_list_win.width,
                UNIV_LIST_PREVIEW_HEIGHT
	    )
        )
            return(-1);


        /* Menu bar. */
	if(MenuBarInit(
	    &univ_list_win.mb,
            univ_list_win.toplevel,
	    0, 0,
	    univ_list_win.width, UNIV_LIST_MB_HEIGHT,
            UnivListMenuCB,
            &univ_list_win
	))
	    return(-1);

	if(MenuBarAddItem(
	    &univ_list_win.mb,
	    -1,			/* Append. */
	    "Universe",
	    0, 0,
	    0, 0
	))
	    return(-1);
        if(MenuBarAddItem(
            &univ_list_win.mb,
            -1,			/* Append. */
            "Edit",
            univ_list_win.mb.item[0]->x +
		(int)univ_list_win.mb.item[0]->width,
	    0,
	    0, 0
        ))
            return(-1);

	/* Universe menu. */
	i = 0;
	if(!MenuBarIsItemAllocated(&univ_list_win.mb, i))
	    return(-1);
	MenuBarAddItemMenuItem(
	    &univ_list_win.mb, i,
	    "Connect",
	    MENU_ITEM_TYPE_ENTRY,
            NULL,
            UNIV_LIST_MENU_CODE_CONNECT,
            -1
	);
        MenuBarAddItemMenuItem(
            &univ_list_win.mb, i,
            NULL,
            MENU_ITEM_TYPE_HR,
            NULL,
            UNIV_LIST_MENU_CODE_NONE,
            -1
        );
        MenuBarAddItemMenuItem(
            &univ_list_win.mb, i,
            "Save",
            MENU_ITEM_TYPE_ENTRY,
            NULL,
            UNIV_LIST_MENU_CODE_SAVE,
            -1
        );
        MenuBarAddItemMenuItem(
            &univ_list_win.mb, i,
            NULL,
            MENU_ITEM_TYPE_HR,
            NULL,
            UNIV_LIST_MENU_CODE_NONE,
            -1
        );
        MenuBarAddItemMenuItem( 
            &univ_list_win.mb, i,
            "Close",
            MENU_ITEM_TYPE_ENTRY,
            NULL,
            UNIV_LIST_MENU_CODE_CLOSE,
            -1
        );

	/* Edit menu. */
        i = 1;
        if(!MenuBarIsItemAllocated(&univ_list_win.mb, i))
            return(-1);

        MenuBarAddItemMenuItem(
            &univ_list_win.mb, i,
            "Insert",
            MENU_ITEM_TYPE_ENTRY,
            NULL,
            UNIV_LIST_MENU_CODE_INSERT,
            -1
        );
        MenuBarAddItemMenuItem(
            &univ_list_win.mb, i,
            "Copy",
            MENU_ITEM_TYPE_ENTRY,
            NULL,
            UNIV_LIST_MENU_CODE_COPY,
            -1
        );
        MenuBarAddItemMenuItem(
            &univ_list_win.mb, i,
            "Delete",
            MENU_ITEM_TYPE_ENTRY,
            NULL,
            UNIV_LIST_MENU_CODE_DELETE,
            -1
        );
        MenuBarAddItemMenuItem(  
            &univ_list_win.mb, i,
            NULL,
            MENU_ITEM_TYPE_HR,
            NULL,
            UNIV_LIST_MENU_CODE_NONE,
            -1
        );
        MenuBarAddItemMenuItem(
            &univ_list_win.mb, i,
            "Properties",
            MENU_ITEM_TYPE_ENTRY,
            NULL,
            UNIV_LIST_MENU_CODE_PROPERTIES,
            -1
        );


	/* List window. */ 
	if(ListWinInit(
	    &univ_list_win.list,
	    univ_list_win.toplevel,
	    0, UNIV_LIST_MB_HEIGHT,
	    univ_list_win.width,
	    (int)univ_list_win.height - UNIV_LIST_MB_HEIGHT -
                UNIV_LIST_PREVIEW_HEIGHT
	))
	    return(-1);

	list = &univ_list_win.list;

	list->client_data = &univ_list_win.list;
	list->func_cb = UnivListDoSelect;


	/* Add universe entries into list window. */
	for(i = 0; i < total_univ_entries; i++)
	{
	    if(univ_entry[i] == NULL)
		continue;

	    image_ptr = ULWGetProperListItemIcon(univ_entry[i]);

	    ListAddItem(
		&univ_list_win.list,
		LIST_ENTRY_TYPE_NORMAL,	/* Type. */
		univ_entry[i]->alias,	/* Name. */
		image_ptr,		/* Icon. */
		-1,			/* Append. */
		univ_entry[i]		/* Pointer to univ entry. */
	    );
	}


	/* Right-click menu. */
	if(MenuInit(
	    &univ_list_win.menu,
	    osw_gui[0].root_win,
	    UnivListMenuCB,
	    &univ_list_win
	))
	    return(-1);

	/* Add items to menu. */
	MenuAddItem(
	    &univ_list_win.menu,
	    "Connect",
	    MENU_ITEM_TYPE_ENTRY,
	    NULL,
	    UNIV_LIST_MENU_CODE_CONNECT,
	    -1
	);
        MenuAddItem(
            &univ_list_win.menu,
            NULL,
            MENU_ITEM_TYPE_HR,
            NULL,
            UNIV_LIST_MENU_CODE_NONE,
            -1
        );
        MenuAddItem(
            &univ_list_win.menu,
            "Insert",
            MENU_ITEM_TYPE_ENTRY,
            NULL,
            UNIV_LIST_MENU_CODE_INSERT,
            -1
        );
        MenuAddItem(
            &univ_list_win.menu,
            "Copy",
            MENU_ITEM_TYPE_ENTRY,
            NULL,
            UNIV_LIST_MENU_CODE_COPY,
            -1
        );
        MenuAddItem(
            &univ_list_win.menu,
            "Delete", 
            MENU_ITEM_TYPE_ENTRY,
            NULL,
            UNIV_LIST_MENU_CODE_DELETE,
            -1
        );
        MenuAddItem(
            &univ_list_win.menu,
            NULL,
            MENU_ITEM_TYPE_HR,
            NULL,
            UNIV_LIST_MENU_CODE_NONE,
            -1
        );
        MenuAddItem(
            &univ_list_win.menu,
            "Properties",
            MENU_ITEM_TYPE_ENTRY,
            NULL,
            UNIV_LIST_MENU_CODE_PROPERTIES,
            -1
        );


	return(0);
}

int UnivListDraw(int amount)
{
	win_t w;
	pixmap_t pixmap;
	univ_entry_struct *univ_entry_ptr;
	win_attr_t wattr;


	/* Map as needed. */
	if(!univ_list_win.map_state)
	{
	    OSWMapRaised(univ_list_win.toplevel);
	    OSWMapWindow(univ_list_win.status);
	    univ_list_win.map_state = 1;

	    MenuBarMap(&univ_list_win.mb);
	    ListWinMap(&univ_list_win.list);

	    if(amount != ULW_DRAW_AMOUNT_COMPLETE)
		amount = ULW_DRAW_AMOUNT_COMPLETE;
	}


	/* Recreate buffers as needed. */
	if(univ_list_win.status_buf == 0)
	{
	    OSWGetWindowAttributes(univ_list_win.status, &wattr);
	    if(OSWCreatePixmap(
		&univ_list_win.status_buf, wattr.width, wattr.height
	    ))
		return(-1);
	}


	/* *********************************************************** */

	/* Draw menu bar widget. */
        if(amount == ULW_DRAW_AMOUNT_COMPLETE)
        {
            MenuBarDraw(&univ_list_win.mb);
	}

	/* Draw list widget. */
	if((amount == ULW_DRAW_AMOUNT_COMPLETE) ||
           (amount == ULW_DRAW_AMOUNT_LIST)
	)
	{
	    ListWinDraw(&univ_list_win.list);
	}


	/* Draw status window. */
	if((amount == ULW_DRAW_AMOUNT_COMPLETE) ||
           (amount == ULW_DRAW_AMOUNT_STATUS)
	)
	{
	    w = univ_list_win.status;
	    pixmap = univ_list_win.status_buf;

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

	    if(widget_global.force_mono)
                OSWSetFgPix(osw_gui[0].white_pix);
	    else
		OSWSetFgPix(widget_global.surface_highlight_pix);

	    OSWDrawLine(pixmap,
	        0, (int)wattr.width - 2,
	        0, 0
	    );
            OSWDrawLine(pixmap,
                0, 0,
                (int)wattr.width - 2, 0
            );

            OSWDrawLine(pixmap,
                6, (int)wattr.height - 5,
                (int)wattr.width - 5, (int)wattr.height - 5
            );
            OSWDrawLine(pixmap,
                (int)wattr.width - 5, (int)wattr.height - 5,
                (int)wattr.width - 5, 6
            );

            if(widget_global.force_mono) 
                OSWSetFgPix(osw_gui[0].white_pix);
            else
                OSWSetFgPix(widget_global.surface_shadow_pix);
            OSWDrawLine(pixmap,
                1, (int)wattr.height - 1,
                (int)wattr.width - 2, (int)wattr.height - 1
            );
            OSWDrawLine(pixmap,
                (int)wattr.width - 1, 1,
                (int)wattr.width - 1, (int)wattr.height - 1
            );

            OSWDrawLine(pixmap,
                5, (int)wattr.height - 6,
                5, 5
            );
            OSWDrawLine(pixmap,
                5, 5,
                (int)wattr.width - 5, 5
            );


	    /* Get selected entry position. */
	    univ_entry_ptr = UnivListGetUnivEntryFromList(&univ_list_win.list);

	    /* Draw status message if univ entry is valid. */
	    if(univ_entry_ptr != NULL)
	    {
	        /* Draw URL since pointer is not NULL. */
	        if(univ_entry_ptr->url != NULL)
	        {
                    if(widget_global.force_mono)
                        OSWSetFgPix(osw_gui[0].white_pix);
                    else
			OSWSetFgPix(widget_global.normal_text_pix);

	            OSWDrawStringLimited(
			pixmap,
		        10, 20,
		        univ_entry_ptr->url,
		        MIN(
			    (int)strlen(univ_entry_ptr->url),
			    ((int)wattr.width / UNIV_LIST_CHAR_WIDTH) - 3
		        )
		    );
	        }
	    }
	    else
	    {
	        /*   Draw help since pointer is NULL, indicating nothing
                 *   selected.
		 */
                if(widget_global.force_mono)
                    OSWSetFgPix(osw_gui[0].white_pix);
                else
                    OSWSetFgPix(widget_global.normal_text_pix);

                OSWDrawStringLimited(
		    pixmap,
                    10, 20,
                    STATUS_NOTHING_SELECTED_MESSAGE,
                    MIN(
			(int)strlen(STATUS_NOTHING_SELECTED_MESSAGE),
                        ((int)wattr.width / UNIV_LIST_CHAR_WIDTH) - 3
                    )
                );
	    }


            /* Copy status buffer to the window. */
	    OSWPutBufferToWindow(
	        univ_list_win.status,
	        univ_list_win.status_buf
	    );
	}


	return(0);
}



int UnivListResize()
{
	int x, y;
	unsigned int width, height;
	win_attr_t wattr;


	/* Get new sizes. */
	OSWGetWindowAttributes(univ_list_win.toplevel, &wattr);

	/* Check for change in size. */
	if((univ_list_win.width == (unsigned int)wattr.width) &&
	   (univ_list_win.height == (unsigned int)wattr.height)
	)
	    return(0);

	/* Update new size. */
	univ_list_win.x = wattr.x;
	univ_list_win.y = wattr.y;
	univ_list_win.width = wattr.width;
	univ_list_win.height = wattr.height;


	/* Calculate new position and size for status window. */
	x = 0;
	y = (int)univ_list_win.height - UNIV_LIST_PREVIEW_HEIGHT;
	width = univ_list_win.width;
	height = UNIV_LIST_PREVIEW_HEIGHT;


	/* Move and resize status window. */
	OSWMoveResizeWindow(
	    univ_list_win.status,
	    x, y, width, height
	);

	OSWDestroyPixmap(&univ_list_win.status_buf);


	/* Menu bar. */
	OSWMoveResizeWindow(
	    univ_list_win.mb.toplevel,
	    0, 0,
	    univ_list_win.width, UNIV_LIST_MB_HEIGHT
	);
	MenuBarResize(&univ_list_win.mb);

	/* List. */
	OSWMoveResizeWindow(univ_list_win.list.toplevel,
	    0, UNIV_LIST_MB_HEIGHT,
	    univ_list_win.width,
	    (int)univ_list_win.height - UNIV_LIST_MB_HEIGHT -
		UNIV_LIST_PREVIEW_HEIGHT
	);
	ListWinResize(&univ_list_win.list);


	return(0);
}



int UnivListManage(event_t *event)
{
	int x, y;
	int events_handled = 0;


        if(event == NULL)   
            return(events_handled);


        if(!univ_list_win.map_state &&
           (event->type != MapNotify)
        )  
            return(events_handled);


	switch(event->type)
	{
	  /* ****************************************************** */
	  case Expose:
	    if((event->xany.window == univ_list_win.toplevel) ||
	       (event->xany.window == univ_list_win.list.toplevel)
	    )
	    {
		/* Need to map list. */
		if(univ_list_win.list.map_state == 0)
		    univ_list_win.list.map_state = 1;

	        /* Have widget manage Expose. */
                events_handled += ListWinManage(
                    &univ_list_win.list,
                    event
                );
	    }
	    break;

          /* ********************************************************* */
          case UnmapNotify:
            if(event->xany.window == univ_list_win.toplevel)
            {
                UnivListUnmap();

                events_handled++;
                return(events_handled);
            }
            break;
          
          /* ********************************************************* */
          case MapNotify:  
            if(event->xany.window == univ_list_win.toplevel)
            {
                if(!univ_list_win.map_state)
                    UnivListMap();

                events_handled++;
                return(events_handled);
            }
            break;

          /* ****************************************************** */
	  case KeyPress:
	    /* Skip if not in focus. */
	    if(!univ_list_win.is_in_focus)
		return(events_handled);

            /* Escape. */
            if(event->xkey.keycode == osw_keycode.esc)
            {
		if(univ_list_win.list.cv_prompt_mode == LW_CVP_MODE_NONE)
		{
		    UnivListUnmap();

                    events_handled++;
                    return(events_handled);
		}
            }
	    break;

          /* ***************************************************** */
          case KeyRelease:
            /* Skip if not in focus. */
            if(!univ_list_win.is_in_focus)
                return(events_handled);

	    break;

          /* ***************************************************** */
	  case ButtonPress:
	    /* Change into focus? */
	    if((event->xany.window == univ_list_win.toplevel) ||
               (event->xany.window == univ_list_win.status) ||
               (event->xany.window == univ_list_win.list.toplevel)
	    )
	    {
	        univ_list_win.is_in_focus = 1;
		univ_list_win.list.is_in_focus = 1;


		/* Map right-click menu for button 3. */
		if(event->xbutton.button == Button3)
		{
		    OSWGetPointerCoords(
                        osw_gui[0].root_win,
			&x, &y,
			NULL, NULL
		    );

		    /* Map menu. */
		    MenuMapPos(&univ_list_win.menu, x, y);

		    /* Allow list window to manage this event too. */
		    events_handled += ListWinManage(
                        &univ_list_win.list,
                        event
                    );

		    events_handled++;
		    return(events_handled);
		}
	    }
	    break;

	  /* ******************************************************** */
	  case ButtonRelease:

	    break;

          /* ********************************************************* */
          case VisibilityNotify:
	    if(event->xany.window == univ_list_win.toplevel)
	    {
                univ_list_win.visibility_state =
		    event->xvisibility.state;
 
                events_handled++;

                /* No need to continue, just return. */
                return(events_handled);
	    }
            else if(ListWinManage(&univ_list_win.list, event) > 0)
            {
                events_handled++;
                return(events_handled);
            }
            break;

          /* ********************************************************* */
          case ConfigureNotify:
	    if(event->xany.window == univ_list_win.toplevel)
	    {
		UnivListResize();
		events_handled++;
	    }
	    break;

          /* ********************************************************* */
          case FocusOut:
            if(event->xany.window == univ_list_win.toplevel)
            {
                univ_list_win.is_in_focus = 0;
                univ_list_win.list.is_in_focus = 0;

                events_handled++;
            }
            break;

          /* ********************************************************* */
          case FocusIn:
	    if(event->xany.window == univ_list_win.toplevel)
            {
		univ_list_win.is_in_focus = 1;
                univ_list_win.list.is_in_focus = 1;

		events_handled++;
	    }
	    break;

          /* **************************************************** */
          case ClientMessage:
	    if(OSWIsEventDestroyWindow(univ_list_win.toplevel, event))
            {
		/* Unmap universe list window. */
		UnivListUnmap();

                events_handled++;
                return(events_handled);
            }
  
            break;

	}

        /* Redraw universe list windows if an event was handled above. */
        if(events_handled > 0)
        {
            UnivListDraw(ULW_DRAW_AMOUNT_COMPLETE);
        }


        /* ******************************************************** */

        /* Manage the menu. */
        if(events_handled == 0)
        {
            /* Manage menu. */
            if(univ_list_win.menu.map_state)
                events_handled += MenuManage(
                    &univ_list_win.menu,
                    event
                );
        }

	/* Handle events on list window widget. */
	if(events_handled == 0)
	{
	    events_handled += ListWinManage(
                &univ_list_win.list,
                event
            );

	    /* Set universe list window group into focus as needed. */
            if((events_handled > 0) && univ_list_win.map_state)
            {
                UnivListDraw(ULW_DRAW_AMOUNT_STATUS);
            }
	}

	/* Menu bar. */
	if(events_handled == 0)
            events_handled += MenuBarManage(
		&univ_list_win.mb,
		event
	    );

	return(events_handled);
}



void UnivListMap()
{
	win_attr_t wattr;


	/* Unfocus all XSW windows. */
	XSWDoUnfocusAllWindows();


	/* Move univ list window to center. */
	WidgetCenterWindow(univ_list_win.toplevel, WidgetCenterWindowToRoot);
	OSWGetWindowAttributes(univ_list_win.toplevel, &wattr);
	univ_list_win.x = wattr.x;
        univ_list_win.y = wattr.y;
        univ_list_win.width = wattr.width;
        univ_list_win.height = wattr.height;


	/* Map the universe list window by drawing it. */
	univ_list_win.map_state = 0;
	UnivListDraw(ULW_DRAW_AMOUNT_COMPLETE);
	univ_list_win.visibility_state = VisibilityUnobscured;
	univ_list_win.is_in_focus = 1;

	univ_list_win.list.is_in_focus = 1;


        /* Restack all XSW windows. */
        XSWDoRestackWindows();


	return;
}



void UnivListUnmap()
{
	ListWinUnmap(&univ_list_win.list);
	MenuBarUnmap(&univ_list_win.mb);

	OSWUnmapWindow(univ_list_win.toplevel);
	univ_list_win.map_state = 0;
        univ_list_win.visibility_state = VisibilityFullyObscured;
	univ_list_win.is_in_focus = 0;


	OSWDestroyPixmap(&univ_list_win.status_buf);


	return;
}



void UnivListDestroy()
{

        /* Delete all universe entries. */
        UnivDeleteAll();


	if(IDC())
	{
            MenuDestroy(&univ_list_win.menu);
            ListWinDestroy(&univ_list_win.list);
	    MenuBarDestroy(&univ_list_win.mb);

            OSWDestroyPixmap(&univ_list_win.status_buf);
            OSWDestroyWindow(&univ_list_win.status);
            OSWDestroyWindow(&univ_list_win.toplevel);
	}

	univ_list_win.map_state = 0;
        univ_list_win.x = 0;
        univ_list_win.y = 0;
        univ_list_win.width = 0;
        univ_list_win.height = 0;
        univ_list_win.visibility_state = VisibilityFullyObscured;
	univ_list_win.is_in_focus = 0;


	return;
}

/*
 *	Universe list menu callback handler.
 */
int UnivListMenuCB(void *client_data, int op_code)
{
	int i, len, status;
	int new_univ_entry_num;

        int univ_entry_num = 0;
	univ_entry_struct *univ_entry_ptr;

	list_window_struct *list;
	int list_item_num;
	list_window_entry_struct *list_item_ptr;

	char *strptr;


	/* Get pointer to list widget. */
	list = &univ_list_win.list;


	/* Get pointer to selected universe entry. */
	univ_entry_ptr = UnivListGetUnivEntryFromList(list);

	/* Get selected universe entry number. */
	univ_entry_num = -1;
	if(univ_entry_ptr != NULL)
	{
	    for(i = 0; i < total_univ_entries; i++)
	    {
	        if(univ_entry[i] == NULL)
		    continue;

	        if(univ_entry[i] == univ_entry_ptr)
		{
		    univ_entry_num = i;
		    break;
		}
	    }
	}


        /* Get selected item number and pointer on list widget. */
        list_item_num = list->entry_pos;

        if(ListIsItemAllocated(list, list_item_num))
	{
            list_item_ptr = list->entry[list_item_num];
	}
        else
	{
	    list_item_num = -1;
            list_item_ptr = NULL;
 	}


	/*   Variables univ_entry_num, univ_entry_ptr, list_item_num,
	 *   and list_item_ptr are now fetched but may be -1 or NULL.
	 */

	/* Check which action to perform. */
	switch(op_code)
	{
          /* ********************************************************* */
          /* Unmap universe list window. */
          case UNIV_LIST_MENU_CODE_CLOSE:
            UnivListUnmap();
            break;

          /* ********************************************************* */
          /* Save changes. */
          case UNIV_LIST_MENU_CODE_SAVE:
            status = UnivListSaveToFile(fname.universe_list);
            if(status < 0)
            {
                printdw(&err_dw,
                    "Error saving universes list.\n"
                );
            }
            else
            {
                printdw(&info_dw, 
                    "Universes list successfully saved.\n"
                );
            }
            break;

	  /* ********************************************************* */
	  /* Perform connect. */
	  case UNIV_LIST_MENU_CODE_CONNECT:
	    UnivListDoConnect();
	    break;

          /* ********************************************************* */
	  /* Insert a new entry. */
	  case UNIV_LIST_MENU_CODE_INSERT:
	    /* Shift list item number to proper position for insert. */
	    if(list_item_num < 0)
		list_item_num = 0;

            /* Add a new universe entry. */
            i = UnivAdd(
                "New universe",	/* Name. */
                DEF_UNIV_URL,	/* URL. */
                0,		/* Last visited. */
                "",		/* Comment. */
                -1		/* Append. */
            );
            if(i < 0)
                return(-1);
	    else
		univ_entry_ptr = univ_entry[i];

	    /* Insert list entry to list window. */
            status = ListAddItem(
                &univ_list_win.list,
                LIST_ENTRY_TYPE_NORMAL,
                "New universe",		/* Name. */
		ULWGetProperListItemIcon(univ_entry_ptr),	/* Icon. */
		list_item_num,		/* Position to place new. */
                univ_entry_ptr		/* Pointer to univ entry. */
            );
            if(status < 0)
                return(-1);
	    if(!ListIsItemAllocated(list, list_item_num))
		return(-1);


	    /* Adjust list widget's selected item to be the new item. */
	    list->entry_pos = list_item_num;

	    /* Redraw the universe list as needed. */
	    if(univ_list_win.map_state)
		UnivListDraw(ULW_DRAW_AMOUNT_LIST);

	    /* Bring up universe edit properties window. */
	    UnivListMenuCB(
		&univ_list_win,
		UNIV_LIST_MENU_CODE_PROPERTIES
	    );
	    break;

          /* ********************************************************* */
          /* Copy selected universe and insert new below it. */ 
	  case UNIV_LIST_MENU_CODE_COPY:

	    if((list_item_ptr == NULL) ||
               (univ_entry_ptr == NULL)
	    )
		break;

	    /* Add a new universe entry. */
	    i = UnivAdd(
		list_item_ptr->name,	/* Name. */
		univ_entry_ptr->url,	/* URL. */
		univ_entry_ptr->last_connected,	/* Last visited. */
		univ_entry_ptr->comments,	/* Comments. */
		-1			/* Append. */
	    );
	    if(i < 0)
		return(-1);

	    /*   univ_entry_num is currently the original universe entry.
	     *   new_univ_entry_num is the coppied universe entry number.
	     */
	    new_univ_entry_num = i;


	    /* Change alias name of new universe entry. */
	    if(univ_entry_ptr->alias != NULL)
	    {
		/* Get pointer to and length of original alias name. */
	        strptr = (char *)univ_entry_ptr->alias;
	        len = strlen(strptr);
	        len += strlen("Copy of") + 10;

		/* Free and reallocate memory for new alias. */
		free(univ_entry[new_univ_entry_num]->alias);
	        univ_entry[new_univ_entry_num]->alias = (char *)calloc(
		    1,
		    len * sizeof(char)
		);
		if(univ_entry[new_univ_entry_num]->alias == NULL)
		    return(-1);

		/* Set new alias. */
	        sprintf(
		    univ_entry[new_univ_entry_num]->alias,
		    "%s %s",
		    "Copy of",
		    strptr
		);

	        if(len > UNIV_MAX_ALIAS_LEN)
	        {
		    univ_entry[new_univ_entry_num]->alias[UNIV_MAX_ALIAS_LEN] = '\0';
	        }
	    }

	    /* Add a new item to the list widget. */
            status = ListAddItem(
                &univ_list_win.list,
                LIST_ENTRY_TYPE_NORMAL,
                univ_entry[new_univ_entry_num]->alias,	/* Name. */
                ULWGetProperListItemIcon(univ_entry[new_univ_entry_num]),	/* Icon. */
                list_item_num + 1,		/* Position. */
                univ_entry[new_univ_entry_num]	/* Pointer to univ entry. */
            );
            if(status < 0)
                return(-1);

	    /* Adjust list widget's selected item to be the new item. */
            list->entry_pos = list_item_num + 1;

            /* Redraw universe list as needed. */
            if(univ_list_win.map_state)
                UnivListDraw(ULW_DRAW_AMOUNT_LIST);

	    break;

          /* ********************************************************* */
	  /* Delete selected universe. */
	  case UNIV_LIST_MENU_CODE_DELETE:

            if((list_item_ptr == NULL) ||
               (univ_entry_ptr == NULL)
            )
                break;


	    /* Delete universe entry first. */
	    UnivDelete(univ_entry_num);

	    /* Shift universe entries. */
	    for(i = univ_entry_num; i < (total_univ_entries - 1); i++)
		univ_entry[i] = univ_entry[i + 1];

	    /* Reduce total. */
	    total_univ_entries--;
	    if(total_univ_entries > 0)
	    {
	        univ_entry = (univ_entry_struct **)realloc(
		    univ_entry,
		    total_univ_entries * sizeof(univ_entry_struct *)
	        );
	        if(univ_entry == NULL)
	        {
		    total_univ_entries = 0;
		    return(-1);
	        }
	    }
	    else
	    {
		total_univ_entries = 0;
		free(univ_entry);
		univ_entry = NULL;
	    }
	    /* univ_entry_num and univ_entry_ptr are probably
             * invalid now!
             */

	    /* Delete list widget items who still point to the
	     * universe entry that was deleted.
	     */
	    while(1)
	    {
	        for(i = 0; i < list->total_entries; i++)
	        {
		    if(list->entry[i] == NULL)
		        continue;
		    if(list->entry[i]->data_ptr == univ_entry_ptr)
		        break;
	        }
	        if(i < list->total_entries)
		    ListDeleteItem(list, i);
		else
		    break;
	    }

            /* Redraw the universe list as needed. */
            if(univ_list_win.map_state)
                UnivListDraw(ULW_DRAW_AMOUNT_LIST);

	    break;

	  /* ******************************************************* */
	  /* Map univ_edit_win to edit universe properties. */
	  case UNIV_LIST_MENU_CODE_PROPERTIES:

            if((list_item_ptr == NULL) ||
               (univ_entry_ptr == NULL)
            )
                break;

	    /* Set up values for prompt fields in univ_edit_win prompts. */

	    /* Alias. */
	    PromptSetS(
		&univ_edit_win.alias_prompt,
		list_item_ptr->name
	    );

	    /* URL */
            PromptSetS( 
                &univ_edit_win.url_prompt,
                univ_entry_ptr->url
            );

	    /* Comments. */
	    PromptSetS(
                &univ_edit_win.comments_prompt,
                univ_entry_ptr->comments
            );


            /* Set univlist entry number on univ edit menu. */
            univ_edit_win.univ_entry_num = univ_entry_num;

            /* Map univ list menu. */
            UnivEditWinMap();

	    break;

          /* ********************************************************* */
	  default:
	    break;
	}


	return(0);
}
