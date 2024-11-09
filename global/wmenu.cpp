// widgets/wmenu.cpp
/*
                               Widget: Menu

	Functions:

	int MenuIsItemAllocated(menu_struct *menu, int n)
	int MenuGetItemNumberByID(menu_struct *menu, int id_code)
	bool_t MenuGetItemState(menu_struct *menu, int n)
	void MenuSetItemState(menu_struct *menu, int n, bool_t state)
	void MenuSetItemAccelerator(
	        menu_struct *menu,
	        int n,
	        char key,
	        menu_item_flags_t flags
	)

        int MenuAddItem(
	        menu_struct *menu,
	        char *name,
	        int type,
	        image_t *icon,
	        int id_code,
	        int pos
	)
        int MenuDeleteAllItems(menu_struct *menu)

        int MenuInit(
                menu_struct *menu,
                win_t parent,
                int (*func_cb)(void *, int),
                void *client_data
        )
        int MenuDraw(menu_struct *menu)
        int MenuManage(
		menu_struct *menu, 
		event_t *event
	)
	int MenuClose(menu_struct *menu)
	void MenuMap(menu_struct *menu)
	void MenuMapPos(menu_struct *menu, int x, int y)
	void MenuUnmap(menu_struct *menu)
	void MenuDestroy(menu_struct *menu)

	---



 */

#include "../include/string.h"
#include "../include/widget.h"


/*
 *	Size constants (in pixels):
 */
#define MENU_DEF_WIDTH	128
#define MENU_DEF_HEIGHT	32

#define MENU_ROW_HEIGHT	32

/* Height for the horizontal rule, in pixels. */
#define MENU_HR_HEIGHT	6

/* All purpose margin (in pixels). */
#define MENU_MARGIN	5

#define MENU_ACCELERATOR_MARGIN	20

#define MENU_CHAR_WIDTH		7
#define MENU_CHAR_HEIGHT	14



/*
 *	Checks if item n is allocated on menu.
 */
int MenuIsItemAllocated(menu_struct *menu, int n)
{
	if(menu == NULL)
	    return(0);
	else if((n < 0) || (n >= menu->total_items))
	    return(0);
	else if(menu->item[n] == NULL)
	    return(0);
	else
	    return(1);
}

/*
 *	Returns the item number on the menu that matches the
 *	given id code. Can return -1 for no match or error.
 */
int MenuGetItemNumberByID(menu_struct *menu, int id_code)
{
	int i;


	if(menu == NULL)
	    return(-1);

	for(i = 0; i < menu->total_items; i++)
	{
	    if(menu->item[i] == NULL)
		continue;

	    if(menu->item[i]->id_code == id_code)
		return(i);
	}

	return(-1);
}

/*
 *	Gets the state of menu item n.
 *      
 *      Menu item n should be of type MENU_ITEM_TYPE_TOGGLEENTRY
 *      (but this is not checked).
 */
bool_t MenuGetItemState(menu_struct *menu, int n)
{
        menu_item_struct *ptr;

        if(MenuIsItemAllocated(menu, n))
            ptr = menu->item[n];
        else
            return(False);

	return(ptr->flags & MENU_ITEM_FLAG_TOGGLED);
}

/*
 *	Sets the state of men item n.
 *
 *	Menu item n should be of type MENU_ITEM_TYPE_TOGGLEENTRY
 *	(but this is not checked).
 */
void MenuSetItemState(menu_struct *menu, int n, bool_t state)
{
	menu_item_struct *ptr;


	if(MenuIsItemAllocated(menu, n))
	    ptr = menu->item[n];
	else
	    return;

	if(state)
	    ptr->flags |= MENU_ITEM_FLAG_TOGGLED;
	else
	    ptr->flags &= ~MENU_ITEM_FLAG_TOGGLED;

	return;
}

/*
 *	Sets the accelerator key for the specified menu item.
 */
void MenuSetItemAccelerator(
        menu_struct *menu, 
        int n, 
        char key,
        menu_item_flags_t flags
)
{
	menu_item_struct *ptr;


        if(MenuIsItemAllocated(menu, n))
            ptr = menu->item[n];
        else
            return;


        if(flags & MENU_ITEM_FLAG_ACCEL_ALT)
            ptr->flags |= MENU_ITEM_FLAG_ACCEL_ALT;
        else
            ptr->flags &= ~MENU_ITEM_FLAG_ACCEL_ALT;

        if(flags & MENU_ITEM_FLAG_ACCEL_CTRL)
            ptr->flags |= MENU_ITEM_FLAG_ACCEL_CTRL;
        else
            ptr->flags &= ~MENU_ITEM_FLAG_ACCEL_CTRL;

	if(flags & MENU_ITEM_FLAG_ACCEL_SHIFT)
	    ptr->flags |= MENU_ITEM_FLAG_ACCEL_SHIFT;
	else
	    ptr->flags &= ~MENU_ITEM_FLAG_ACCEL_SHIFT;

	ptr->accelerator = key;

	return;
}

/*
 *	Allocates a new menu item on menu.
 */
int MenuAddItem(
        menu_struct *menu, 
        char *name, 
        int type, 
        image_t *icon,
        int id_code,
        int pos  
)
{
	int n;
	menu_item_struct *item_ptr;



	if(menu->total_items < 0)
	    menu->total_items = 0;

	/* Appending a new item. */
	n = menu->total_items;
	menu->total_items++;

	/* Reallocate pointers. */
	menu->item = (menu_item_struct **)realloc(
	    menu->item,
	    menu->total_items * sizeof(menu_item_struct *)
	);
	if(menu->item == NULL)
	{
	    menu->total_items = 0;
	    return(-1);
	}

	/* Allocate new item. */
	menu->item[n] = (menu_item_struct *)calloc(
	    1,
	    sizeof(menu_item_struct)
	);
	if(menu->item[n] == NULL)
        {
            menu->total_items--;
            return(-1);
        }

	item_ptr = menu->item[n];


	/* Set values. */
	item_ptr->name = StringCopyAlloc(
	    (name == NULL) ? "(null)" : name
	);

	item_ptr->type = type;
	item_ptr->flags = 0;
	item_ptr->icon = icon;
	item_ptr->id_code = id_code;

	item_ptr->accelerator = '\0';

	return(0);
}


/*
 *	Deletes all items in menu.
 */
void MenuDeleteAllItems(menu_struct *menu)
{
	int i;
        menu_item_struct **ptr;


	for(i = 0, ptr = menu->item;
            i < menu->total_items;
            i++, ptr++
	)
	{
	    if(*ptr == NULL)
		continue;

	    free((*ptr)->name);

	    free(*ptr);
	}

	free(menu->item);
	menu->item = NULL;

	menu->total_items = 0;
	menu->selected_item = -1;


	return;
}


/*
 *	Initializes menu.
 */
int MenuInit(
        menu_struct *menu,
        win_t parent,
        int (*func_cb)(void *, int),
        void *client_data
)
{
        if((menu == NULL) ||
           (parent == 0)
        )
            return(-1);


	/* Reset values. */
        menu->map_state = 0;
	menu->is_in_focus = 0;
        menu->visibility_state = VisibilityFullyObscured;
        menu->x = 0;
        menu->y = 0;
	/* Note that width and height are readjusted when drawn. */
        menu->width = MENU_DEF_WIDTH;
        menu->height = MENU_DEF_HEIGHT;
        menu->font = widget_global.menu_font;
	menu->next = NULL;
	menu->prev = NULL;

        menu->row_height = MENU_ROW_HEIGHT;
        menu->char_width = MENU_CHAR_WIDTH;

        menu->item = NULL;
        menu->selected_item = -1;
        menu->total_items = 0;
        menu->client_data = client_data;
        menu->func_cb = func_cb;


        /* Initialize menu toplevel window. */
        if(
            OSWCreateWindow(  
                &menu->toplevel,
                parent,
                menu->x, menu->y, 
                menu->width, menu->height
            )
        )
            return(-1);
	menu->toplevel_buf = 0;

        OSWSetWindowWMProperties(
            menu->toplevel,
            "",         /* Title. */
            "",         /* Icon title. */
            widget_global.std_icon_pm,   
            False,      /* WM sets coordinates? */
            menu->x, menu->y,
            2, 2,
            osw_gui[0].display_width, osw_gui[0].display_height,
            WindowFrameStyleNaked,
            NULL, 0
        );  
        OSWSetWindowBkg(menu->toplevel, 0, widget_global.std_bkg_pm);

        /* Select input for menu. */
        OSWSetWindowInput(
	    menu->toplevel,
            ButtonPressMask | ButtonReleaseMask | PointerMotionMask |
            ExposureMask | VisibilityChangeMask
        );


	/* Add widget to regeristry. */
	WidgetRegAdd(menu, WTYPE_CODE_MENU);


        return(0);
}


int MenuDraw(menu_struct *menu)
{
	int i, n, count, y, len;
	char *strptr;
	unsigned int width = 0, height = 0;
	menu_item_struct **item_ptr;
	image_t *img_ptr;

        font_t *prev_font;


        if(menu == NULL)
            return(-1);


	/* Record previous font. */
        prev_font = OSWQueryCurrentFont();   


	/* Map window and resize? */
	if(!menu->map_state)
	{
	    if(menu->item != NULL)
	    {
		/* Calculate new width. */
		for(width = 0, i = 0, item_ptr = menu->item;
                    i < menu->total_items;
                    i++, item_ptr++
		)
		{
		    if(*item_ptr == NULL)
                        continue;

		    switch((*item_ptr)->type)
		    {
		      case MENU_ITEM_TYPE_TOGGLEENTRY:
                        if((*item_ptr)->flags & MENU_ITEM_FLAG_TOGGLED)
                            img_ptr = widget_global.toggle_btn_armed_img;
                        else
                            img_ptr = widget_global.toggle_btn_unarmed_img;

			n = (strlen((*item_ptr)->name) * menu->char_width)
                            + (4 * MENU_MARGIN) +
			    ((img_ptr == NULL) ? 0 : (int)img_ptr->width);
			break;

		      default:
			n = (((*item_ptr)->name == NULL) ? 0 :
			    strlen((*item_ptr)->name) * menu->char_width) +
                            (4 * MENU_MARGIN);
                        break;
		    }

		    /* Add accelerator key label width. */
		    if((*item_ptr)->accelerator != '\0')
		    {
			n += ((int)menu->char_width + MENU_MARGIN +
			    MENU_ACCELERATOR_MARGIN);
			if((*item_ptr)->flags & MENU_ITEM_FLAG_ACCEL_ALT)
			    n += (strlen("Alt+") * menu->char_width);
                        if((*item_ptr)->flags & MENU_ITEM_FLAG_ACCEL_CTRL)
                            n += (strlen("Ctrl+") * menu->char_width);
			if((*item_ptr)->flags & MENU_ITEM_FLAG_ACCEL_SHIFT)
                            n += (strlen("Shift+") * menu->char_width);
		    }

                    if(n > (int)width)
                        width = n;
		}


	        /* Calculate new height. */
                for(height = 0, i = 0, item_ptr = menu->item;
                    i < menu->total_items;
                    i++, item_ptr++
                )
		{
		    if(*item_ptr == NULL)
		        continue;

		    switch((*item_ptr)->type)
		    {
		      case MENU_ITEM_TYPE_HR:
		        height += MENU_HR_HEIGHT;
		        break;

		      default:
		        height += menu->row_height;
		        break;
		    }
	        }
		height += (2 * MENU_MARGIN);	/* Bit of margin. */
	    }
	    else
	    {
		/* Assume default size. */
                width = 128;
	        height = 32;
	    }

            /* Sanitize width and height. */
            if(width < 128)
                width = 128;
            if(height < 32) 
                height = 32;
            
            /* Set new size values. */
            menu->width = width;
            menu->height = height;


            /* Change position and size of menu->toplevel. */
	    OSWMoveResizeWindow(menu->toplevel,
                menu->x, menu->y,
                width, height
            );      
	    OSWMapRaised(menu->toplevel);

	    menu->map_state = 1;
	    menu->visibility_state = VisibilityUnobscured;

            /*   Ungrab pointer from window of button press that
	     *   mapped us.
	     */
	    OSWUngrabPointer();

	    /* Reset selected_item. */
            menu->selected_item = -1;
	}


	/* *********************************************************** */

        width = menu->width;
        height = menu->height;

	/* Create toplevel buffer. */
	if(menu->toplevel_buf == 0)
	{
	    if(OSWCreatePixmap(&menu->toplevel_buf, width, height))
	        return(-1);
	}


	/* Draw background. */
	if(widget_global.force_mono)
	{
	    OSWClearPixmap(
		menu->toplevel_buf,
		width, height,
		osw_gui[0].black_pix
	    );
	}
	else
	{
	    WidgetPutImageTile(
	        menu->toplevel_buf, 
	        widget_global.std_bkg_img,
	        width, height
	    );
	}


	/* Draw frames. */
        if(widget_global.force_mono)
	{
            WidgetFrameButtonPixmap(
                menu->toplevel_buf,
                False,
                width, height,
                osw_gui[0].white_pix,
		osw_gui[0].white_pix
            );
	}
	else
	{
	    WidgetFrameButtonPixmap(
		menu->toplevel_buf,
		False,
		width, height,
		widget_global.surface_highlight_pix,
		widget_global.surface_shadow_pix
	    );
	}



	/* Draw entries. */
	for(y = MENU_MARGIN, i = 0, item_ptr = menu->item;
            i < menu->total_items;
            i++, item_ptr++
	)
	{
	    /* Bad or missing entry? */
	    if(*item_ptr == NULL)
	    {
		y += menu->row_height;
		continue;
	    }

	    /* Draw HR. */
	    if((*item_ptr)->type == MENU_ITEM_TYPE_HR)
	    {
		if(!widget_global.force_mono)
		{
		    OSWSetFgPix(widget_global.surface_shadow_pix);
		    OSWDrawLine(
			menu->toplevel_buf,
                        MENU_MARGIN + 2,
			y + (MENU_HR_HEIGHT / 2),
                        (int)width - MENU_MARGIN - 2,
			y + (MENU_HR_HEIGHT / 2)
                    );
		}

		if(widget_global.force_mono)
		    OSWSetFgPix(osw_gui[0].white_pix);
		else
                    OSWSetFgPix(widget_global.surface_highlight_pix);
                OSWDrawLine(
		    menu->toplevel_buf,
                    MENU_MARGIN + 2,
		    y + (MENU_HR_HEIGHT / 2) + 1,
                    (int)width - MENU_MARGIN - 2,
		    y + (MENU_HR_HEIGHT / 2) + 1
                );

                y += MENU_HR_HEIGHT;
		continue;
	    }


	    /* Draw frame around selected item. */
	    if(i == menu->selected_item)
	    {
                if(widget_global.force_mono)
                    OSWSetFgPix(osw_gui[0].white_pix);
		else
        	    OSWSetFgPix(widget_global.surface_highlight_pix);
        	OSWDrawLine(
		    menu->toplevel_buf,
        	    MENU_MARGIN, y,
        	    MENU_MARGIN, y + menu->row_height - 1
        	);
        	OSWDrawLine(
		    menu->toplevel_buf,
        	    MENU_MARGIN, y,
        	    (int)width - MENU_MARGIN, y
        	);

		if(widget_global.force_mono)
                    OSWSetFgPix(osw_gui[0].white_pix);
		else
		    OSWSetFgPix(widget_global.surface_shadow_pix);
        	OSWDrawLine(
		    menu->toplevel_buf,
                    MENU_MARGIN + 1, y + menu->row_height - 1,
            	    (int)width - MENU_MARGIN, y + menu->row_height - 1
                );
                OSWDrawLine(
		    menu->toplevel_buf,
                    (int)width - MENU_MARGIN, y + menu->row_height - 1,
                    (int)width - MENU_MARGIN, y + 1
                );
	    }

            /* Draw entry name. */
            if((*item_ptr)->name != NULL)
	    {
                if(widget_global.force_mono)   
                    OSWSetFgPix(osw_gui[0].white_pix);
                else
                    OSWSetFgPix(widget_global.normal_text_pix);

                OSWSetFont(menu->font);
                OSWDrawString(
		    menu->toplevel_buf,
                    MENU_MARGIN + 5,
		    y + ((int)menu->row_height / 2) + 5,
                    (*item_ptr)->name
                );
	    }

	    /* Draw accelerator. */
	    if((*item_ptr)->accelerator != '\0')
	    {
		char text[80];

		sprintf(text,
		    "%s%s%s%c",
		    (((*item_ptr)->flags & MENU_ITEM_FLAG_ACCEL_ALT) ? "Alt+" : ""),
		    (((*item_ptr)->flags & MENU_ITEM_FLAG_ACCEL_CTRL) ? "Ctrl+" : ""),
                    (((*item_ptr)->flags & MENU_ITEM_FLAG_ACCEL_SHIFT) ? "Shift+" : ""),
		    toupper((*item_ptr)->accelerator)
		);
		len = strlen(text);

                if(widget_global.force_mono)
                    OSWSetFgPix(osw_gui[0].white_pix);
                else
                    OSWSetFgPix(widget_global.normal_text_pix);

                OSWSetFont(menu->font);

		/* Draw offset if this item has a  toggle. */
		if((*item_ptr)->type == MENU_ITEM_TYPE_TOGGLEENTRY)
		{
                    if((*item_ptr)->flags & MENU_ITEM_FLAG_TOGGLED)
                        img_ptr = widget_global.toggle_btn_armed_img;
                    else
                        img_ptr = widget_global.toggle_btn_unarmed_img;

                    OSWDrawString(
                        menu->toplevel_buf,
                        (int)width - (len * (int)menu->char_width) -
                            (3 * MENU_MARGIN) + 2 -
                            ((img_ptr == NULL) ? 0 : img_ptr->width),
                        y + ((int)menu->row_height / 2) + 5,
                        text
                    );
		}
		else
		{
                    OSWDrawString(
                        menu->toplevel_buf,
                        (int)width - (len * (int)menu->char_width) -
			    (2 * MENU_MARGIN) + 2,
                        y + ((int)menu->row_height / 2) + 5,
                        text
                    );
		}
	    }

	    /* Draw toggle button. */
	    if((*item_ptr)->type == MENU_ITEM_TYPE_TOGGLEENTRY)
	    {
		if((*item_ptr)->flags & MENU_ITEM_FLAG_TOGGLED)
		    img_ptr = widget_global.toggle_btn_armed_img;
		else
		    img_ptr = widget_global.toggle_btn_unarmed_img;

		if(img_ptr != NULL)
		    WidgetPutImageNormal(
		        menu->toplevel_buf,
			img_ptr,
		        (int)width - (int)img_ptr->width - MENU_MARGIN,
                        y + ((int)menu->row_height / 2) -
			    ((int)img_ptr->height / 2),
			True
		    );
	    }

	    /* Increment y position. */
	    y += menu->row_height;
	}


	OSWPutBufferToWindow(menu->toplevel, menu->toplevel_buf);

        OSWSetFont(prev_font);


	return(0);
}


/*
 *	Manage menu.
 */
int MenuManage(
        menu_struct *menu,
        event_t *event
)
{
	int i, y_pos, count;
	int events_handled = 0;
	int selected_item;
	win_attr_t wattr;


	if((menu == NULL) ||
	   (event == NULL)
	)
	    return(events_handled);

	if(!menu->map_state &&
           (event->type != MapNotify) &&
           (event->type != KeyPress) &&
           (event->type != KeyRelease)
	)
	    return(events_handled);


	switch(event->type)
	{
	  /* ******************************************************** */
	  case MotionNotify:
	    if((menu->item != NULL) &&
	       ((int)menu->total_items > 0) &&
	       (event->xany.window == menu->toplevel)
	    )
	    {
		/* Make sure motion occured on the window. */
		if((event->xmotion.x >= 0) &&
                   (event->xmotion.y >= 0) &&
                   (event->xmotion.x <= menu->width) &&
                   (event->xmotion.y <= menu->height)
		)
		{
		    /* Match new selected position. */
		    selected_item = -1;
		    y_pos = 4;
		    for(count = 0; count < menu->total_items; count++)
		    {
			/* Skip missing entries. */
			if(menu->item[count] == NULL)
			{
			    y_pos += menu->row_height;
			    continue;
			}

			/* Did we find selection? */
			if(selected_item > -1)
			{
			    break;
			}

			/* Check based on type. */
			switch(menu->item[count]->type)
			{
			  case MENU_ITEM_TYPE_HR:
			    y_pos += MENU_HR_HEIGHT;
                            break;

			  case MENU_ITEM_TYPE_COMMENT:
                            if((event->xmotion.y >= y_pos) &&
                               (event->xmotion.y <
                                   (y_pos + (int)menu->row_height))
                            )
                            {
                                selected_item = count;
                            }
                            y_pos += menu->row_height;
                            break;

			  default:
                            if((event->xmotion.y >= y_pos) &&
                               (event->xmotion.y <
                                   (y_pos + (int)menu->row_height))
                            )
                            {
                                selected_item = count;
                            }
                            y_pos += menu->row_height;

                            break;
			}
		    }
		    /* Pointer moved to a new selected item? */
		    if((selected_item != menu->selected_item) &&
		       (selected_item < menu->total_items) &&
		       (selected_item > -1)
		    )
		    {
			menu->selected_item = selected_item;
			events_handled++;
		    }
		}
		else
		{
		    menu->selected_item = -1;
		    events_handled++;
		}
	    }
	    break;

	  /* *************************************************** */
	  case ButtonPress:
	    /*   If button was pressed NOT on menu->toplevel, then
	     *   close the menu.
	     */
	    if(event->xany.window != menu->toplevel)
	    {
		MenuClose(menu);

                /* Do not report any events handled. */
                return(events_handled);
	    }
	    else if((event->xany.window == menu->toplevel) &&
                    (menu->selected_item >= 0) &&
                    (menu->selected_item < (int)menu->total_items) &&
                    (menu->item != NULL) &&
                    (menu->func_cb != NULL)
	    )
	    {
		i = menu->selected_item;

		MenuClose(menu);

		if((i >= 0) && (i < (int)menu->total_items))
		{
		    if(menu->item[i] != NULL)
	            {
                        menu->func_cb(
			    menu->client_data,
                            menu->item[i]->id_code
                        );
		    }
		}

                events_handled++;
		return(events_handled);
	    }
	    break;

	  /* **************************************************** */
	  case ButtonRelease:
	    /* Close menu unconditionally on ButtonRelease. */
            MenuClose(menu);

	    /* Check if event occured on this menu. */
	    if(event->xany.window == menu->toplevel)
	    {
		OSWGetWindowAttributes(menu->toplevel, &wattr);

		if((menu->selected_item >= 0) &&
                   (menu->selected_item < (int)menu->total_items) &&
                   (menu->item != NULL) &&
                   (event->xbutton.x >= 0) &&
                   (event->xbutton.y >= 0) &&
                   (event->xbutton.x < (int)wattr.width) &&
                   (event->xbutton.y < (int)wattr.height)
	        )
	        {
                    i = menu->selected_item;

                    if((i >= 0) && (i < (int)menu->total_items))
                    {
			menu_item_struct *menu_item;


			menu_item = menu->item[i];
                        if(menu_item != NULL)
                        {
			    /* Toggle state as needed. */
			    if(menu_item->type == MENU_ITEM_TYPE_TOGGLEENTRY)
			    {
				if(menu_item->flags & MENU_ITEM_FLAG_TOGGLED)
				    menu_item->flags &= ~MENU_ITEM_FLAG_TOGGLED;
				else
				    menu_item->flags |= MENU_ITEM_FLAG_TOGGLED;
			    }

			    /* Run callback function. */
			    if(menu->func_cb != NULL)
                                menu->func_cb(
                                    menu->client_data,
                                    menu_item->id_code
                                );
                        }
                    }
	        }
                events_handled++;
	    }
            return(events_handled);
	    break;

	  /* ****************************************************** */
	  case KeyPress:
	    if(menu->func_cb != NULL)
	    {
		menu_item_struct *menu_item;
		char key;


		key = OSWGetASCIIFromKeyCode(
		    &event->xkey,
		    False, False, False
		);

	        for(i = 0; i < menu->total_items; i++)
		{
		    menu_item = menu->item[i];
		    if(menu_item == NULL)
			continue;

		    if(menu_item->accelerator == '\0')
			continue;

		    if(menu_item->accelerator != key)
			continue;

		    /* Check modifier keys. */
		    if(menu_item->flags & MENU_ITEM_FLAG_ACCEL_ALT)
		    {
			if(!osw_gui[0].alt_key_state)
			    continue;
		    }
                    if(menu_item->flags & MENU_ITEM_FLAG_ACCEL_CTRL)
                    {
                        if(!osw_gui[0].ctrl_key_state)
                            continue;
                    }
                    if(menu_item->flags & MENU_ITEM_FLAG_ACCEL_SHIFT)
                    {
                        if(!osw_gui[0].shift_key_state)
                            continue;
                    }

                    /* Toggle state as needed. */
                    if(menu_item->type == MENU_ITEM_TYPE_TOGGLEENTRY)
		    {
                        if(menu_item->flags & MENU_ITEM_FLAG_TOGGLED)
			    menu_item->flags &= ~MENU_ITEM_FLAG_TOGGLED;
			else
			    menu_item->flags |= MENU_ITEM_FLAG_TOGGLED;
		    }

		    /* Call callback function. */
		    menu->func_cb(
                        menu->client_data,
                        menu_item->id_code
		    );

		    if(menu->map_state)
			MenuDraw(menu);

		    events_handled++;
		    return(events_handled);
		}
	    }
	    break;

          /* ****************************************************** */
          case KeyRelease:

	    break;

          /* ****************************************************** */
          case Expose:
            if(event->xany.window == menu->toplevel)
	    {
                events_handled++;
            }
            break;

	  /* ****************************************************** */
          case VisibilityNotify:
	    if(event->xany.window == menu->toplevel)
	    {
		menu->visibility_state = event->xvisibility.state;

		events_handled++;
		return(events_handled);
	    }
	    break;
	}

	/* Redraw as needed. */
	if(events_handled > 0)
            MenuDraw(menu);

	return(events_handled);
}


/*
 *	Closes (unmaps) the menu.
 */
int MenuClose(menu_struct *menu)
{
	if(menu == NULL)
	    return(-1);


	OSWUnmapWindow(menu->toplevel);
	menu->map_state = 0;


	/* Destroy buffers. */
	OSWDestroyPixmap(&menu->toplevel_buf);


	/* Preserve selected_item. */


	return(0);
}


/*
 *	Map menu.
 */
void MenuMap(menu_struct *menu)
{
	if(menu == NULL)
	    return;

	menu->map_state = 0;
	MenuDraw(menu);

	return;
}

/*
 *	Map menu at a specific position.
 */
void MenuMapPos(menu_struct *menu, int x, int y)
{
	char need_move = 0;
	win_t parent;
	win_attr_t wattr;


        if(menu == NULL)
            return;

	menu->x = x;
	menu->y = y;

        menu->map_state = 0;
        MenuDraw(menu);
        menu->visibility_state = VisibilityUnobscured;
        menu->is_in_focus = 1;


	/* Make sure menu is `in bounds'. */
        parent = OSWGetWindowParent(menu->toplevel);
        if(parent != 0)
        {
            OSWGetWindowAttributes(parent, &wattr);

	    if((menu->x + (int)menu->width) >= (int)wattr.width)
	    {
		need_move = 1;
		menu->x = (int)wattr.width - (int)menu->width;
	    }
            if((menu->y + (int)menu->height) >= (int)wattr.height)
            {
                need_move = 1;
                menu->y = (int)wattr.height - (int)menu->height;
            }
	    if(menu->x < 0)
	    {
                need_move = 1;
                menu->x = 0;
	    }
	    if(menu->y < 0)
	    {
                need_move = 1;
                menu->y = 0;
            }

	    if(need_move)
	        OSWMoveWindow(menu->toplevel, menu->x, menu->y);
        }


	return;
}

/*
 *	Unmap menu.
 */
void MenuUnmap(menu_struct *menu)
{
	MenuClose(menu);

	return;
}


/*
 *	Destroys menu.
 */
void MenuDestroy(menu_struct *menu)
{
	if(menu == NULL)
	    return;


        /* Delete widget from regeristry. */
        WidgetRegDelete(menu);


	/* Free all menu items. */
	MenuDeleteAllItems(menu);


	if(IDC())
	{
	    OSWDestroyWindow(&menu->toplevel);
	}

	menu->map_state = 0;
	menu->visibility_state = VisibilityFullyObscured;
	menu->is_in_focus = 0;
	menu->x = 0;
	menu->y = 0;
	menu->width = 0;
	menu->height = 0;
	menu->font = NULL;
        menu->next = NULL;
        menu->prev = NULL;


	return;
}




