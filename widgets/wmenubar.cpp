// widgets/wmenubar.cpp
/*
                              Widget: Menu Bar

	Functions:

	int MenuBarIsItemAllocated(menu_bar_struct *mb, int n)
	menu_struct *MenuBarGetMenuFromItem(menu_bar_struct *mb, int n)
	bool_t MenuBarGetItemToggleState(
		menu_bar_struct *mb,
		int menu_num, int item_num
	)
        void MenuBarSetItemToggleState(
                menu_bar_struct *mb,
		int menu_num, int item_num,
		bool_t state
        )

        int MenuBarMatchItemByPos(
		menu_bar_struct *mb,
		int x, int y
	)

	int MenuBarAddItem(
	        menu_bar_struct *mb
	        int pos,		Can be -1 for append.
	        char *name,
	        int x, int y,
	        unsigned int width, unsigned int height
	)
	int MenuBarAddItemMenuItem(
		menu_bar_struct *mb,
		int n,			Menu bar item number, must be valid
		char *name,
		int type,
		image_t *icon,		Shared
		int id_code,
		int pos			Can be -1 for append
	)
	void MenuBarDeleteItem(menu_bar_struct *mb, int n)

	int MenuBarInit(
		menu_bar_struct *mb,
		win_t parent,
		int x, int y,
		unsigned int width, unsigned int height,
		int (*func_cb)(void *, int),
		void *client_data
	)
	int MenuBarResize(menu_bar_struct *mb)
	int MenuBarDraw(menu_bar_struct *mb)
	int MenuBarManage(menu_bar_struct *mb, event_t *event)
	void MenuBarMap(menu_bar_struct *mb)
	void MenuBarUnmap(menu_bar_struct *mb)
	void MenuBarDestroy(menu_bar_struct *mb)

	---

 */

#include "../include/string.h"
#include "../include/widget.h"


/*
 *	Size constants (in pixels):
 */
#define MB_DEF_WIDTH		256
#define MB_DEF_HEIGHT		30

#define MB_CHAR_WIDTH		7
#define MB_CHAR_HEIGHT		14


/*
 *	Checks if the menu bar item n is valid and allocated.
 */
int MenuBarIsItemAllocated(menu_bar_struct *mb, int n)
{
	if(mb == NULL)
	    return(0);
	else if((n < 0) || (n >= mb->total_items))
	    return(0);
	else if(mb->item[n] == NULL)
	    return(0);
	else
	    return(1);
}

/*
 *	Returns a menu structure pointer belonging to the menu bar
 *	item n.  Can return NULL on error.
 */
menu_struct *MenuBarGetMenuFromItem(menu_bar_struct *mb, int n)
{
	if(MenuBarIsItemAllocated(mb, n))
	    return(mb->item[n]->menu);
	else
	    return(NULL);
}

/*
 *	Gets the toggle state of the menu item on the specified
 *	menu. Does not distinguish from errors.
 */
bool_t MenuBarGetItemToggleState(
	menu_bar_struct *mb,
	int menu_num, int item_num
) 
{
	menu_struct *menu;


	if(mb == NULL)
	    return(False);

	menu = MenuBarGetMenuFromItem(mb, menu_num);
	if(menu == NULL)
	    return(False);

	return(MenuGetItemState(menu, item_num));
}

/*
 *	Sets the toggle state of the menu item on the specified
 *      menu.
 */
void MenuBarSetItemToggleState(
	menu_bar_struct *mb,
	int menu_num, int item_num,
	bool_t state
)
{
        menu_struct *menu;


        if(mb == NULL)
            return;

        menu = MenuBarGetMenuFromItem(mb, menu_num);
        if(menu == NULL)
            return;

        return(MenuSetItemState(menu, item_num, state));
}

/*
 *	Matches a menu bar item by a given positionto be on the
 *	menu bar.
 */
int MenuBarMatchItemByPos(
	menu_bar_struct *mb, 
	int x, int y 
)  
{
	int i;
	menu_bar_item_struct **ptr;


	if(mb == NULL)
	    return(-1);

	for(i = 0, ptr = mb->item;
	    i < mb->total_items;
	    i++, ptr++
	)
	{
	    if(*ptr == NULL)
		continue;

	    if(((*ptr)->x <= x) &&
               ((*ptr)->y <= y) &&
               (((int)(*ptr)->width + (*ptr)->x) > x) &&
	       (((int)(*ptr)->height + (*ptr)->y) > y)
	    )
		return(i);
	}


	return(-1);
}

   
/*
 *	Allocates a new menu bar item to mb and initializing the
 *	actual menu and all resources.
 */
int MenuBarAddItem(
	menu_bar_struct *mb,
        int pos,		/* Can be -1 for append. */
	char *name,
	int x, int y,
	unsigned int width, unsigned int height
)
{
	int n, status;
	menu_bar_item_struct *mbi_ptr;


	if(mb == NULL)
	    return(-1);

	if(name == NULL)
	    name = "(null)";


	if(mb->total_items < 0)
	    mb->total_items = 0;

	/* Append for now. */
	n = mb->total_items;
	mb->total_items++;

	mb->item = (menu_bar_item_struct **)realloc(
	    mb->item,
	    mb->total_items * sizeof(menu_bar_item_struct *)
	);
	if(mb->item == NULL)
	{
	    mb->total_items = 0;
	    return(-1);
	}

	mb->item[n] = (menu_bar_item_struct *)calloc(
	    1,
	    sizeof(menu_bar_item_struct)
	);
	if( mb->item[n] == NULL)
	{
            mb->total_items--;
	    return(-1); 
	}


	mbi_ptr = mb->item[n];

	mbi_ptr->name = StringCopyAlloc(name);
	mbi_ptr->x = x;
	mbi_ptr->y = y;

	mbi_ptr->width = ((width == 0) ?
	    ((strlen(name) + 2) * MB_CHAR_WIDTH) :
            width
	);
        mbi_ptr->height = ((height == 0) ? 
            mb->height : height
        );


	/* Initialize menu. */
	mbi_ptr->menu = (menu_struct *)calloc(
	    1,
	    sizeof(menu_struct)
	);
	if(mbi_ptr->menu == NULL)
	    return(-1);

	status = MenuInit(
	    mbi_ptr->menu,
	    osw_gui[0].root_win,
	    mb->func_cb,
	    mb->client_data
	);
	if(status)
	    return(-1);


	return(0);
}


/*
 *	Allocates a new menu item on the menu bar item n's menu.
 */
int MenuBarAddItemMenuItem(
	menu_bar_struct *mb,
        int n,                  /* Menu bar item number, must be valid */
        char *name,
        int type,
        image_t *icon,          /* Shared. */
        int id_code,
        int pos                 /* Can be -1 for append. */
)
{
	int status;
	menu_struct *menu;


	/* Get pointer to menu. */
	menu = MenuBarGetMenuFromItem(mb, n);
	if(menu == NULL)
	    return(-1);


	/* Add menu item. */
	status = MenuAddItem(
	    menu,
	    name,
	    type,
	    icon,
	    id_code,
	    pos
	);


	return(0);
}


/*
 *	Deletes menu bar item n, it's menu and all it's reousrces.
 */
void MenuBarDeleteItem(menu_bar_struct *mb, int n)
{
	menu_bar_item_struct *mbi_ptr;


	/* Check if menu bar item structure is allocated. */
	if(MenuBarIsItemAllocated(mb, n))
	    mbi_ptr = mb->item[n];
	else
	    return;


        /* Get pointer to menu. */
        if(mbi_ptr->menu != NULL)  
	{
	    /* Destroy menu. */
	    MenuDestroy(mbi_ptr->menu);

	    free(mbi_ptr->menu);
	    mbi_ptr->menu = NULL;
	}

	/* Free item name. */
	free(mbi_ptr->name);
	mbi_ptr->name = NULL;


	/* Free structure itself. */
	free(mb->item[n]);
	mb->item[n] = NULL;


	return;
}


/*
 *	Initializes a menu bar.
 */   
int MenuBarInit(
	menu_bar_struct *mb,
        win_t parent,
        int x, int y,
        unsigned int width, unsigned int height,
        int (*func_cb)(void *, int),
        void *client_data
)
{
	win_attr_t wattr;


        if((mb == NULL) ||
	   (parent == 0)
	)
            return(-1);


	OSWGetWindowAttributes(parent, &wattr);
	if(width == 0)
	    width = wattr.width;
	if(width == 0)
	    width = MB_DEF_WIDTH;
	if(height == 0)
	    height = MB_DEF_HEIGHT;


	mb->map_state = 0;
	mb->is_in_focus = 0;
	mb->visibility_state = VisibilityFullyObscured;
	mb->x = x;
	mb->y = y;
	mb->width = width;
	mb->height = height;
	mb->disabled = False;
	mb->font = widget_global.menu_font;
	mb->next = NULL;
	mb->prev = NULL;


	if(
	    OSWCreateWindow(
		&mb->toplevel,
		parent,
		mb->x, mb->y,
		mb->width, mb->height
	    )
	)
	    return(-1);
	mb->toplevel_buf = 0;

	OSWSetWindowInput(mb->toplevel, KeyPressMask |
            KeyReleaseMask | ButtonPressMask | ButtonReleaseMask |
            ExposureMask | VisibilityChangeMask | PointerMotionMask
        );


	mb->func_cb = func_cb;
	mb->client_data = client_data;

	mb->sel_item = -1;

        /* Add menu bar to wdiget regeristry. */
        WidgetRegAdd(mb, WTYPE_CODE_MENUBAR);


	return(0);
}

/*
 *	Resizes the menu bar.
 */
int MenuBarResize(menu_bar_struct *mb)
{
	win_attr_t wattr;


        if(mb == NULL)
            return(-1);


	OSWGetWindowAttributes(mb->toplevel, &wattr);
	if(((int)mb->width == (int)wattr.width) &&
           ((int)mb->height == (int)wattr.height)
	)
	    return(0);


	mb->x = wattr.x;
	mb->y = wattr.y;
	mb->width = wattr.width;
	mb->height = wattr.height;

	OSWDestroyPixmap(&mb->toplevel_buf);


	return(0);
}


/*
 *	Redraws the menu bar.
 */
int MenuBarDraw(menu_bar_struct *mb)
{
	int i, sel_item;
	int x, y;
	int width, height;
	font_t *prev_font;
	win_attr_t wattr;
	menu_bar_item_struct **mbi_ptr;


	if(mb == NULL)
	    return(-1);


        /* Record previous font. */
        prev_font = OSWQueryCurrentFont(); 


	OSWGetWindowAttributes(mb->toplevel, &wattr);


	/* Map as needed. */
	if(!mb->map_state)
	{
	    OSWMapRaised(mb->toplevel);
	    mb->map_state = 1;
	}

	/* Recreate buffers as needed. */
	if(mb->toplevel_buf == 0)
	{
	    if(
		OSWCreatePixmap(
		    &mb->toplevel_buf,
		    wattr.width, wattr.height
		)
	    )
		return(-1);
	}


        OSWGetWindowAttributes(mb->toplevel, &wattr);


	/* Redraw background. */
	if(widget_global.force_mono)
	{
	    OSWClearPixmap(
		mb->toplevel_buf,
		wattr.width, wattr.height,
		osw_gui[0].white_pix
	    );
            WidgetFrameButtonPixmap(
                mb->toplevel_buf,
                False,
                wattr.width, wattr.height,
                (widget_global.force_mono) ?
                    osw_gui[0].white_pix :
                    widget_global.surface_highlight_pix,
                (widget_global.force_mono) ?
                    osw_gui[0].white_pix :
                    widget_global.surface_shadow_pix
            );
	    OSWSetFgPix(osw_gui[0].black_pix);
        }
	else
	{
            WidgetPutImageTile(   
                mb->toplevel_buf,
                widget_global.menu_bkg_img, 
                wattr.width, wattr.height
            );
            WidgetFrameButtonPixmap(
                mb->toplevel_buf,
                False,
                wattr.width, wattr.height,
                (widget_global.force_mono) ?
                    osw_gui[0].white_pix :
                    widget_global.surface_highlight_pix,
                (widget_global.force_mono) ?
                    osw_gui[0].white_pix :
                    widget_global.surface_shadow_pix
            );
	    OSWSetFgPix(widget_global.surface_shadow_pix);
	}


	/* Draw labels. */
	for(i = 0, mbi_ptr = mb->item, sel_item = -1;
            i < mb->total_items;
            i++, mbi_ptr++
	)
	{
	    if(*mbi_ptr == NULL)
		continue;

	    if((*mbi_ptr)->name == NULL)
		continue;

	    OSWSetFont(mb->font);
	    OSWDrawString(
		mb->toplevel_buf,
		(*mbi_ptr)->x + 6,
		((int)wattr.height / 2) + 5,
		(*mbi_ptr)->name
	    );

	    /* Draw frame around selected item. */
	    if(mb->sel_item == i)
		sel_item = i;
	}

	/* Draw frame around selected item. */
	if((sel_item > -1) &&
           !widget_global.force_mono
	)
	{
	    x = mb->item[sel_item]->x + 2;
	    y = mb->item[sel_item]->y + 2;
	    width = (int)mb->item[sel_item]->width - 5;
	    height = (int)mb->item[sel_item]->height - 5;

	    OSWSetFgPix(widget_global.surface_highlight_pix);

	    OSWDrawLine(mb->toplevel_buf,
		x, y + height,
		x, y
	    );
            OSWDrawLine(mb->toplevel_buf,
                x, y,
                x + width, y
            );

            OSWSetFgPix(widget_global.surface_shadow_pix);

            OSWDrawLine(mb->toplevel_buf,
                x + width, y + 1,
                x + width, y + height
            );
            OSWDrawLine(mb->toplevel_buf,
                x + 1, y + height,
                x + width, y + height
            );
	}


	/* Put buffer to window. */
	OSWPutBufferToWindow(mb->toplevel, mb->toplevel_buf);


	OSWSetFont(prev_font);


        return(0);
}

/*
 *	Manages menu bar.
 */
int MenuBarManage(menu_bar_struct *mb, event_t *event)
{
	int i;
	int root_x, root_y, x, y;
	menu_struct *menu;
	menu_bar_item_struct **mbi_ptr;
	win_attr_t wattr;
	int events_handled = 0;


        if((mb == NULL) ||
           (event == NULL)
	)
            return(events_handled);

	if(!mb->map_state &&
           (event->type != MapNotify)
	)
	    return(events_handled);


	switch(event->type)
	{
          /* ******************************************************* */
	  case KeyPress:
/*
	    if(!mb->is_in_focus)
		break;
 */
	    for(i = 0; i < mb->total_items; i++)
	    {
		if(mb->item[i] == NULL)
		    continue;

		menu = mb->item[i]->menu;
		if(menu == NULL)
		    continue;

		events_handled += MenuManage(menu, event);
		if(events_handled > 0)
		    break;
	    }
	    break;

	  /* ******************************************************* */
          case KeyRelease:
/*
            if(!mb->is_in_focus) 
                break;
 */
            for(i = 0; i < mb->total_items; i++)
            {
                if(mb->item[i] == NULL)
                    continue;

                menu = mb->item[i]->menu;
                if(menu == NULL)
                    continue;

                events_handled += MenuManage(menu, event);
                if(events_handled > 0)
                    break;
            }
            break;
         
          /* ******************************************************* */
          case ButtonPress:
            if(event->xany.window == mb->toplevel)
            {
		i = MenuBarMatchItemByPos(
		    mb, event->xbutton.x, event->xbutton.y
		);
		if(i < 0)
		{
                    menu = MenuBarGetMenuFromItem(mb, mb->sel_item);
                    if(menu != NULL)
                        MenuUnmap(menu);

		    mb->sel_item = -1;

		    break;
		}
                /* Menu bar item i should be valid. */

		if(mb->sel_item == i)
		{
		    /* Unmap menu. */
		    menu = MenuBarGetMenuFromItem(mb, mb->sel_item);
		    if(menu != NULL)
			MenuUnmap(menu);

		    mb->sel_item = -1;
		}
		else
		{
                    OSWGetWindowAttributes(mb->toplevel, &wattr);
 
		    mb->sel_item = i;

		    /* Map menu. */
                    menu = MenuBarGetMenuFromItem(mb, mb->sel_item);
                    if(menu != NULL)
                    {
                        OSWGetPointerCoords(mb->toplevel,
                            &root_x, &root_y,
                            &x, &y
                        );
                        MenuMapPos(
			    menu,
			    root_x - x + mb->item[i]->x,
			    root_y - y + wattr.height
			);
                    }
		}

		events_handled++;
	    }
	    break;

          /* ******************************************************* */
          case ButtonRelease:
            if(event->xany.window == mb->toplevel)
            {

		events_handled++;
	    }
	    else
	    {
	        /*   If button release was on somewhere else,
		 *   then unconditionally set sel_item to -1.
		 */
	        if(mb->sel_item > -1)
		{
		    mb->sel_item = -1;
                    MenuBarDraw(mb);
		}
	    }
            break;

          /* ******************************************************* */
          case MotionNotify:
            if(event->xany.window == mb->toplevel)
            {
		if(mb->sel_item < 0)
		    break;

                i = MenuBarMatchItemByPos(
                    mb, event->xmotion.x, event->xmotion.y
                );
                if(i < 0)
                    break;
		/* Menu bar item i should be valid. */
 
                if(mb->sel_item != i)
                {
		    OSWGetWindowAttributes(mb->toplevel, &wattr);

		    /* Unmap previous menu and map new menu. */
                    menu = MenuBarGetMenuFromItem(mb, mb->sel_item);
                    if(menu != NULL)
                        MenuUnmap(menu);

                    menu = MenuBarGetMenuFromItem(mb, i);
                    if(menu != NULL)
		    {
		        OSWGetPointerCoords(mb->toplevel,
			    &root_x, &root_y,
			    &x, &y
		        );
                        MenuMapPos(
                            menu,
                            root_x - x + mb->item[i]->x,
                            root_y - y + wattr.height
                        );
		    }

		    mb->sel_item = i;
                    events_handled++;
		}
            } 
            break;

          /* ******************************************************* */
          case Expose:
	    if(event->xany.window == mb->toplevel)
            {
		events_handled++;
	    }
	    break;

          /* ******************************************************* */
          case VisibilityNotify:
	    if(event->xany.window == mb->toplevel)
	    {
		mb->visibility_state = event->xvisibility.state;

		events_handled++;
		return(events_handled);
	    }
	    break;
	}

	if(events_handled > 0)
	{
	    MenuBarDraw(mb);
	    return(events_handled);
	}


	/* Manage each menu. */
	for(i = 0, mbi_ptr = mb->item;
	    i < mb->total_items;
	    i++, mbi_ptr++
	)
	{
	    if(*mbi_ptr == NULL)
		continue;

	    if((*mbi_ptr)->menu == NULL)
		continue;

	    events_handled += MenuManage(
		(*mbi_ptr)->menu,
		event
	    );

	    if(events_handled > 0)
		break;
	}
 
        
        return(events_handled);
}

/*
 *	Maps the menu bar.
 */
void MenuBarMap(menu_bar_struct *mb)
{
	if(mb == NULL)
	    return;


	mb->map_state = 0;
	MenuBarDraw(mb);


	return;
}

/*
 *	Unmaps the menu bar.
 */
void MenuBarUnmap(menu_bar_struct *mb)
{
	int i;
	menu_bar_item_struct **mbi_ptr;


	if(mb == NULL)
            return;


	/* Unmap all menus. */
	for(i = 0, mbi_ptr = mb->item;
            i < mb->total_items;
            i++, mbi_ptr++
	)
	{
	    if(*mbi_ptr == NULL)
		continue;

	    if((*mbi_ptr)->menu == NULL)
		continue;

	    MenuUnmap((*mbi_ptr)->menu);
	}

	mb->sel_item = -1;


	OSWUnmapWindow(mb->toplevel);
	mb->map_state = 0;
	mb->visibility_state = VisibilityFullyObscured;
	mb->is_in_focus = 0;


	OSWDestroyPixmap(&mb->toplevel_buf);


	return;
}

/*
 *	Destroys the menu bar.
 */
void MenuBarDestroy(menu_bar_struct *mb)
{
	int i;


	if(mb == NULL)
	    return;


	/* Delete from widget regeristry. */
	WidgetRegDelete(mb);


	/* Delete all menu bar items. */
	for(i = 0; i < mb->total_items; i++)
	    MenuBarDeleteItem(mb, i);

	free(mb->item);
	mb->item = NULL;

	mb->total_items = 0;



	if(IDC())
	{
	    OSWDestroyPixmap(&mb->toplevel_buf);
	    OSWDestroyWindow(&mb->toplevel);
	}

	mb->func_cb = NULL;
	mb->client_data = NULL;


	mb->map_state = 0;
	mb->is_in_focus = 0;
	mb->visibility_state = VisibilityFullyObscured;
	mb->x = 0;
	mb->y = 0;
	mb->width = 0;
	mb->height = 0;
	mb->font = NULL;
        mb->next = NULL;
        mb->prev = NULL;


	return;
}




