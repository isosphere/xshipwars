// widgets/wlist.cpp
/*
                         Widget: List Window

        Functions:

        int ListGetItemNumByDataPointer(
                list_window_struct *lw,
                void *data_ptr
        )

	int ListIsItemAllocated(
	        list_window_struct *lw,
	        int n
	)

        int ListAddItem(
                list_window_struct *lw,
                int type,
                char *name,
                image_t *image,
                int pos,
                void *data_ptr
        )
        void ListDeleteItem(list_window_struct *lw, int pos)
        void ListDeleteAllItems(list_window_struct *lw)


        int ListWinInit(
                list_window_struct *lw,
                win_t parent,
                int x, int y,
                unsigned int width, unsigned int height
        )
        int ListWinResize(list_window_struct *lw)
        int ListWinDraw(list_window_struct *lw)
        int ListWinManage(
                list_window_struct *lw,
                event_t *event
        )
	void ListWinMapCVPrompt(list_window_struct *lw, int mode)
	void ListWinUnmapCVPrompt(list_window_struct *lw)
        void ListWinMap(list_window_struct *lw);
        void ListWinUnmap(list_window_struct *lw);
        void ListWinDestroy(list_window_struct *lw)

	WCursor *ListWinCreateCursorFromEntry(
	        list_window_struct *lw,  
	        int entry_num
	)
	void ListWinDoDragStart(
	        list_window_struct *lw,
	        int entry_pos,
	        win_t start_w
	)
	void ListWinDoDragStop(list_window_struct *lw)

	void ListWinRepeatRecordSet(
		list_window_struct *lw,  
		long dsec, int op_code
	)
	void ListWinRepeatRecordClear()
	int ListWinManageRepeat(event_t *event)

        ---


 */

#include "../include/string.h"
#include "../include/widget.h"


#ifndef MAX
#define MIN(a,b)        (((a) < (b)) ? (a) : (b))
#define MAX(a,b)	(((a) > (b)) ? (a) : (b))
#endif
/* #define MIN(a,b)        (((a) < (b)) ? (a) : (b)) */
/* #define MAX(a,b)        (((a) > (b)) ? (a) : (b)) */


/*
 *	Size constants (in pixels):
 */
#define LW_DEF_CHAR_WIDTH	7
#define LW_DEF_CHAR_HEIGHT	14

#define LW_DEF_ROW_HEIGHT	30

#define LW_PROMPT_HEIGHT	30


/*
 *	Maximum length of list item's name.
 */
#define LW_ITEM_NAME_MAX	256

/*
 *	Default name to give to an item who's name is NULL.
 */
#define LW_DEF_ITEM_NAME	"Untitled"

         
/*
 *      Drag tolorance in pixels.
 */
#ifndef LW_DRAG_TOLORANCE
# define LW_DRAG_TOLORANCE          6
#endif


/*
 *	List window repeat record:
 */
#define LW_OP_CODE_NONE         0
#define LW_OP_CODE_SCROLL_UP    1	/* One unit. */
#define LW_OP_CODE_SCROLL_DOWN  2	/* One unit. */

typedef struct {

	list_window_struct *lw;
	long next_repeat;		/* Next repeat in ms. */
	int op_code;			/* One of LW_OP_CODE_*. */

} lw_repeat_record_struct;
lw_repeat_record_struct lw_repeat_record[1];

namespace static_wlist {
	static int drag_start_entry_pos;
	static win_t drag_start_w;     /* Which window did drag start on. */
	static int drag_start_x, drag_start_y; 
	static bool_t drag_active;
         
	static bool_t	button1_state,
			button2_state,
			button3_state;
}


/*
 *      Drag records.
 */
//static int drag_start_entry_pos;
//static win_t drag_start_w;     /* Which window did drag start on. */
//static int drag_start_x, drag_start_y; 
//static bool_t drag_active;
         
//static bool_t	button1_state,
//		button2_state,
//		button3_state;



/*
 *      Returns the index number of a list item on the list window
 *	widget lw that has a matching data pointer data_ptr.
 *
 *      Returns -1 if no match could be made.
 */
int ListGetItemNumByDataPointer(
        list_window_struct *lw,
        void *data_ptr
)
{
        int i;
	list_window_entry_struct **ptr;


        if((lw == NULL) ||
           (data_ptr == NULL)
        )
            return(-1);

  
        /* Scan through list. */
        for(i = 0, ptr = lw->entry;
            i < lw->total_entries;
            i++, ptr++
	)
        {
            if(*ptr == NULL)
		continue;

	    /* Data pointers match? */
            if((*ptr)->data_ptr == data_ptr)
		return(i);
        }

	return(-1);
}

/*
 *	Checks if item n is allocated on lw.
 */
int ListIsItemAllocated(
        list_window_struct *lw,
        int n
)
{
	if(lw == NULL)
	    return(0);
	else if((lw->entry == NULL) ||
                (n < 0) ||
                (n >= lw->total_entries)
	)
	    return(0);
	else if(lw->entry[n] == NULL)
	    return(0);
	else
	    return(1);
}

/*
 *      Allocates a mew list item on list window lw.
 *
 *      Returns -1 on error, 0 on success.
 */
int ListAddItem(
        list_window_struct *lw,
        int type,
        char *name,
        image_t *image,
        int pos,
        void *data_ptr
)
{
        int i, n;
        list_window_entry_struct *new_entry;


        if(lw == NULL)
            return(-1);

        /* If name is NULL, use default name. */
        if(name == NULL)
            name = LW_DEF_ITEM_NAME;

	/* Allocate a new entry. */
	new_entry = (list_window_entry_struct *)calloc(
	    1,
	    sizeof(list_window_entry_struct)
        );
	if(new_entry == NULL)
	    return(-1);


        /* Sanitize total. */
        if(lw->total_entries < 0)
            lw->total_entries = 0;

	/* Increment total. */
        lw->total_entries++;

        /* Allocate more entry pointers. */
        lw->entry = (list_window_entry_struct **)realloc(
            lw->entry,
            lw->total_entries * sizeof(list_window_entry_struct *)
        );
        if(lw->entry == NULL)
        {
	    free(new_entry);
            lw->total_entries = 0;

            return(-1); 
        }


        /* Append or insert? */
        if(pos < 0)
        {
            /* Append. */

	    /* Set new position n to the last position. */
            n = lw->total_entries - 1;
        }
        else
        {
            /* Insert. */

            /* Set new position n. */
            n = pos;

            /* Sanitize n. */
            if(n >= lw->total_entries)
                n = lw->total_entries - 1;
            if(n < 0)
                n = 0;


            /* Shift entry pointers. */
            for(i = lw->total_entries - 1; i > n; i--)
                lw->entry[i] = lw->entry[i - 1];
        }

	/* Set new entry pointer to point to the just allocated struct. */
	lw->entry[n] = new_entry;

	/* Set new values. */
        new_entry->type = type;
	new_entry->name = StringCopyAlloc(name);
        new_entry->image = image;
        new_entry->data_ptr = data_ptr;

        return(0);
}

/*
 *      Deletes entry pos on list and reallocates the entry pointers.
 */
void ListDeleteItem(list_window_struct *lw, int pos)
{
        int i;


        if(lw == NULL)
            return;

        if((pos < 0) || (pos >= lw->total_entries))
            return;

	/* Delete entry at pos. */
        if(lw->entry[pos] != NULL)
        {
            /* Free entry and its allocated substructures. */
            free(lw->entry[pos]->name);

            free(lw->entry[pos]);
            lw->entry[pos] = NULL;
        }

        /* Shift entries. */
        for(i = pos; i < (lw->total_entries - 1); i++)
            lw->entry[i] = lw->entry[i + 1];

        /* Reallocate entry pointers. */
        lw->total_entries--;
        if(lw->total_entries > 0)
        {
            lw->entry = (list_window_entry_struct **)realloc(
		lw->entry,
                lw->total_entries * sizeof(list_window_entry_struct *)
            );
            if(lw->entry == NULL)
            {
                lw->total_entries = 0;
                return;
            }
        }
	else
	{
	    free(lw->entry);
	    lw->entry = NULL;

	    lw->total_entries = 0;
	}

        /* Sanitize selected entry position. */
        if(lw->entry_pos >= lw->total_entries)
            lw->entry_pos = lw->total_entries - 1;
        if(lw->entry_pos < 0)
            lw->entry_pos = -1; /* Not selected. */

        return;
}

/*
 *	Deletes all entries on list window widget lw.
 */
void ListDeleteAllItems(list_window_struct *lw)
{
        int i;
	list_window_entry_struct **ptr;


        if(lw == NULL)
            return;

        for(i = 0, ptr = lw->entry;
            i < lw->total_entries;
            i++, ptr++
	)
        {
            if(*ptr == NULL)
		continue;

            free((*ptr)->name);
            free(*ptr);
        }

        free(lw->entry);
        lw->entry = NULL;

        lw->total_entries = 0;
        lw->entry_pos = -1;

        return;
}

/*
 *      Initialize a list widget.
 */
int ListWinInit(
        list_window_struct *lw,
        win_t parent,
        int x, int y,
        unsigned int width, unsigned int height
)
{
        if((lw == NULL) ||
	   (parent == 0)
        )
            return(-1);


        /* Sanitize width and height. */
        if(width < 2)
            width = 2;
        if(height < 2)
            height = 2;


	/* Reset values. */
        lw->map_state = 0;
        lw->visibility_state = VisibilityFullyObscured;
        lw->is_in_focus = 0;
	lw->x = x;
	lw->y = y;
        lw->width = width;
        lw->height = height;
	lw->font = OSWQueryCurrentFont();
	lw->next = NULL;
	lw->prev = NULL;

	lw->allow_drag = True;

        lw->char_width = LW_DEF_CHAR_WIDTH;
	lw->char_height = LW_DEF_CHAR_HEIGHT;
        lw->row_height = LW_DEF_ROW_HEIGHT;

        lw->total_entries = 0;
        lw->entry = NULL;
        lw->entry_pos = -1;

        lw->left_margin = 5;
        lw->top_margin = 5;

	lw->drag_cursor = NULL;


        /* Create toplevel. */
        if( 
            OSWCreateWindow(
                &lw->toplevel,
                parent,
                lw->x, lw->y, 
                lw->width, lw->height
            )
        )
            return(-1);

	lw->toplevel_buf = 0;

        OSWSetWindowInput(
	    lw->toplevel,
	    KeyPressMask | KeyReleaseMask |
            ButtonPressMask | ButtonReleaseMask | PointerMotionMask |
            ExposureMask | VisibilityChangeMask
        );


        /* Scroll bars. */
	if(
            SBarInit(
                &lw->sb,
                lw->toplevel,
                lw->width,
                lw->height
	    )
        )
	    return(-1);


	/* Reset change values prompt sturcture. */
	memset(&lw->cv_prompt, 0x00, sizeof(prompt_window_struct));


        /* Add this widget to the widget regeristry. */
        WidgetRegAdd(lw, WTYPE_CODE_LIST);


	return(0);
}       



/*
 *	Resizes the list window.
 */
int ListWinResize(list_window_struct *lw)
{
        win_attr_t wattr;


        if(lw == NULL)
            return(-1);


        OSWGetWindowAttributes(lw->toplevel, &wattr);

        /* No change in size? */
        if(((int)lw->width == (int)wattr.width) &&
           ((int)lw->height == (int)wattr.height)
        ) 
            return(0);


        /* Set new size values. */
        lw->x = wattr.x;
        lw->y = wattr.y;
        lw->width = wattr.width;
        lw->height = wattr.height;

        /* Recreate toplevel buffer. */
        OSWDestroyPixmap(&lw->toplevel_buf);
	if(
            OSWCreatePixmap(
                &lw->toplevel_buf,
                lw->width, lw->height
	    )
        )
            return(-1);
            

        /* Resize scroll bars. */
        SBarResize(&lw->sb, lw->width, lw->height);

        SBarDraw(
            &lw->sb,
            lw->width,
            lw->height,
            lw->width,
            (lw->total_entries + 2) * (int)lw->row_height
        ); 

        return(0);
}

/*
 *	Redraws the list window.
 */
int ListWinDraw(list_window_struct *lw)
{
	int i, x_pos, y_pos;
        int entries_drawn, entries_visible;
	char *entry_name_ptr;
	int name_len;
	list_window_entry_struct **ptr, *entry_ptr;
	image_t *entry_icon_ptr;
	unsigned int icon_width, icon_height;
	pixel_t sel_bg_pix, sel_fg_pix, text_fg_pix;
	win_t w;
	pixmap_t pixmap;
        font_t *prev_font;
        win_attr_t wattr;


        if(lw == NULL)
            return(-1);

        /* Map as needed. */
        if(!lw->map_state)
        {
            OSWMapWindow(lw->toplevel);
            lw->map_state = 1;

	    OSWGetWindowAttributes(lw->toplevel, &wattr);

            /* Redraw scroll bars. */
            SBarDraw(
                &lw->sb,
                wattr.width,
                wattr.height,
                wattr.width,
                (lw->total_entries + 2) * (int)lw->row_height
            );
        }

        /* Recreate buffers as needed. */
        if(lw->toplevel_buf == 0)
        {
	    OSWGetWindowAttributes(lw->toplevel, &wattr);
            if(
                OSWCreatePixmap(
                    &lw->toplevel_buf,
                    wattr.width, wattr.height
                )
            )
                return(-1);
        }


        /* Record previous font. */
        prev_font = OSWQueryCurrentFont();

        OSWSetFont(lw->font);


	/* Begin drawing. */
	if(1)
	{
	    w = lw->toplevel;
	    pixmap = lw->toplevel_buf;

            OSWGetWindowAttributes(w, &wattr);


            /* Draw background. */
            if(widget_global.force_mono)
                OSWClearPixmap(
                    pixmap,
                    wattr.width, wattr.height,
                    osw_gui[0].black_pix
                );
	    else
                WidgetPutImageTile(
                    pixmap,
                    widget_global.std_bkg_img,
                    wattr.width, wattr.height
                );


            /* Begin drawing each item, get starting index, position,
	     * and pointers in list.
	     */
	    if((lw->total_entries > 0) &&
               (lw->entry != NULL)
	    )
	    {
		/* Get pixel colors. */
		if(widget_global.force_mono)
		{
                    sel_bg_pix = osw_gui[0].white_pix;
                    sel_fg_pix = osw_gui[0].black_pix;
                    text_fg_pix = osw_gui[0].white_pix;
		}
		else
		{
		    sel_bg_pix = widget_global.surface_selected_pix;
		    sel_fg_pix = widget_global.selected_text_pix;
		    text_fg_pix = widget_global.normal_text_pix;
		}


	        /* Get starting entry index number. */
	        i = lw->sb.y_win_pos / (int)lw->row_height;
		if(i >= lw->total_entries)
		    i = lw->total_entries - 1;
		if(i < 0)
		    i = 0;

		/* Get starting item in pointer array. */
		ptr = &lw->entry[i];

		/* Get starting coordinate positions. */
		x_pos = (int)lw->left_margin;
		y_pos = (int)lw->top_margin -
		    (lw->sb.y_win_pos % (int)lw->row_height);

	        /* Calculate number of visable entries plus 2 (to
		 * account for partially visable items at the top and
		 * bottom).
		 */
		if(lw->row_height > 0)
		    entries_visible = ((int)wattr.height /
			(int)lw->row_height) + 2;
		else
		    entries_visible = 0;


		/* Begin drawing each entry. */
		entries_drawn = 0;
		while((i < lw->total_entries) &&
		      (entries_drawn < entries_visible)
		)
		{
		    entry_ptr = *ptr;
		    if(entry_ptr == NULL)
		    {
			/* Increment entry index and pointer. */
			i++;
			ptr++;

			continue;
		    }

		    /* Is entry selected and in drag? */
		    if(static_wlist::drag_start_entry_pos == i)
		    {
			if(static_wlist::drag_active &&
                           (static_wlist::drag_start_w == w)
			)
			{
			    /* Pretend we drew this entry. */
			    i++;
                            ptr++;
                            y_pos += (int)lw->row_height;
                            entries_drawn++;
			    continue;
			}
		    }

		    /* Get entry's name. */
		    entry_name_ptr = entry_ptr->name;
		    if(entry_name_ptr == NULL)
			entry_name_ptr = "(null)";

		    /* Calculate length of name. */
		    name_len = strlen(entry_name_ptr);


		    /* Get entry's icon image. */
		    entry_icon_ptr = entry_ptr->image;

		    /* Draw icon image and get its size. */
		    if(entry_icon_ptr == NULL)
		    {
			icon_width = 0;
			icon_height = 0;
		    }
		    else
		    {
			icon_width = entry_icon_ptr->width;
			icon_height = entry_icon_ptr->height;


			OSWPutImageToDrawablePos(
			    entry_icon_ptr,
			    pixmap,
			    x_pos,
			    y_pos +
				((int)lw->row_height / 2) -
				((int)icon_height / 2)
		        );
/* Dosen't work out well if icon is partially off screen.
			WidgetPutImageRaised(
			    pixmap,
			    entry_icon_ptr,
			    x_pos, 
                            y_pos +
                                ((int)lw->row_height / 2) -
                                ((int)icon_height / 2),
			    6
			);
 */
		    }

		    /* Is this entry selected? */
		    if(lw->entry_pos == i)
		    {
			OSWSetFgPix(sel_bg_pix);
			OSWDrawSolidRectangle(
			    pixmap,
			    x_pos + (int)icon_width + (int)lw->left_margin,
			    y_pos + ((int)lw->row_height / 2) -
				(((int)lw->char_height + 6) / 2),
			    (name_len * (int)lw->char_width) + 4,
			    (int)lw->char_height + 6
			);

			OSWSetFgPix(sel_fg_pix);
		    }
		    else
		    {
			OSWSetFgPix(text_fg_pix);
		    }
		    OSWDrawString( 
                        pixmap,
                        x_pos + (int)icon_width + lw->left_margin + 1,
                        y_pos + ((int)lw->row_height / 2) + 5,
                        entry_name_ptr
                    );


		    /* Increment entry index and pointer. */
		    i++;
		    ptr++;

		    /* Increment entry coordinate positions. */
		    y_pos += (int)lw->row_height;

		    entries_drawn++;
		}
	    }

	    /* Draw frame depression. */
	    if(widget_global.force_mono)
                WidgetFrameButtonPixmap(
                    pixmap,
                    True,
                    wattr.width, wattr.height,
                    osw_gui[0].white_pix,
                    osw_gui[0].white_pix
                );
	    else
                WidgetFrameButtonPixmap(
                    pixmap,
                    True,
                    wattr.width, wattr.height,
                    widget_global.surface_highlight_pix,
                    widget_global.surface_shadow_pix
                );

	    OSWPutBufferToWindow(w, pixmap);
	}


	/* Set back previous font. */
        OSWSetFont(prev_font);


        return(0);
}

/*
 *	Manages list window.
 */
int ListWinManage(
        list_window_struct *lw,
        event_t *event
)
{
	int i, x, y, z, y_max;
        int new_entry_pos;
        list_window_entry_struct *list_entry_ptr;
        win_attr_t wattr;

	int events_handled = 0;

	static long last_click;


        if((lw == NULL) ||
           (event == NULL)
        )
            return(events_handled);


        if(!lw->map_state &&
	   (event->type != MapNotify)
	)
            return(events_handled);


        switch(event->type)
        {
          /* ****************************************************** */
          case ButtonPress:
	    /* Toplevel. */
            if(event->xany.window == lw->toplevel)
            {
                OSWGetWindowAttributes(lw->toplevel, &wattr);

                /* Sanitize lw->row_height. */
                if(lw->row_height < 1) 
                    lw->row_height = 1;

		/* Set in focus. */
                lw->is_in_focus = 1;
		lw->sb.is_in_focus = 1;

                /* Record button state. */
                if(event->xbutton.button == Button1)
                    static_wlist::button1_state = True;
                if(event->xbutton.button == Button2)
                    static_wlist::button2_state = True;
                if(event->xbutton.button == Button3)
                    static_wlist::button3_state = True;

                /* Record button press coordinates. */
                static_wlist::drag_start_x = event->xbutton.x;
                static_wlist::drag_start_y = event->xbutton.y;


		/* Unmap change values prompt. */
		if(lw->cv_prompt_mode != LW_CVP_MODE_NONE)
		    ListWinUnmapCVPrompt(lw);


		/* Record last entry position as z. */
		z = lw->entry_pos;

                /* Calculate new entry position. */
                lw->entry_pos = ((int)lw->sb.y_win_pos
                    + (int)event->xbutton.y - (int)lw->top_margin) /
                    (int)lw->row_height;

                /* Sanitize entry position. */
                if(lw->entry_pos >= lw->total_entries)
                    lw->entry_pos = lw->total_entries - 1;
                if(lw->entry_pos < 0)
                    lw->entry_pos = 0;


		/* Scroll as needed if near edge. */
                y_max = ((lw->total_entries + 2) *
                    (int)lw->row_height) - wattr.height;
		/* Scroll up? */
		if((event->xbutton.y < (int)lw->row_height) &&
                   (lw->sb.y_win_pos > 0)
		)
		{
		    ListWinDraw(lw);
		    SBarDraw(
                        &lw->sb,
                        wattr.width,
                        wattr.height,
                        wattr.width,
                        (lw->total_entries + 2) * (int)lw->row_height
                    );

		    lw->sb.y_win_pos -= ((int)wattr.height / 2);

                    /* Sanitize scroll. */
                    if(lw->sb.y_win_pos > y_max) 
                        lw->sb.y_win_pos = y_max;
                    if(lw->sb.y_win_pos < 0) 
                        lw->sb.y_win_pos = 0;

		    usleep(widget_global.list_edge_scroll_delay * 1000);

                    SBarDraw(
                        &lw->sb,
                        wattr.width, 
                        wattr.height, 
                        wattr.width,
                        (lw->total_entries + 2) * (int)lw->row_height
                    );
		}
		/* Scroll down? */
		else if((event->xbutton.y > ((int)wattr.height - (int)lw->row_height)) &&
                        (lw->sb.y_win_pos < y_max)
		)
		{
                    ListWinDraw(lw);
                    SBarDraw(
                        &lw->sb,
                        wattr.width,
                        wattr.height,
                        wattr.width, 
                        (lw->total_entries + 2) * (int)lw->row_height
                    );

		    lw->sb.y_win_pos += ((int)wattr.height / 2);

		    /* Sanitize scroll. */
                    if(lw->sb.y_win_pos > y_max)
                        lw->sb.y_win_pos = y_max;
                    if(lw->sb.y_win_pos < 0)
                        lw->sb.y_win_pos = 0;

                    usleep(widget_global.list_edge_scroll_delay * 1000);     

                    SBarDraw(
                        &lw->sb,
                        wattr.width, 
                        wattr.height, 
                        wattr.width,
                        (lw->total_entries + 2) * (int)lw->row_height
                    );
		}

                /* Double clicked? */
                if((lw->entry_pos >= 0) &&
                   (event->xbutton.button == Button1) &&
                   ((last_click + widget_global.double_click_int) >= MilliTime())
                )
                {
                    /* Do select notify procedure. */
                    if((lw->entry_pos == z) &&   
                       (lw->entry_pos >= 0)
                    )
                    {
                        if(lw->func_cb != NULL)
			    lw->func_cb(lw->client_data);
                 
                        /* Reset last_click. */
                        last_click = 0;

                        events_handled++;
                        return(events_handled);
                    }
                    else
                    {
                        /* Record last_click. */ 
                        last_click = MilliTime();   
                    }
                }
                /* Slow double clicked for relabel item? */
                else if((lw->entry_pos >= 0) && 
                        (event->xbutton.button == Button1) &&
                        ((last_click + widget_global.relabel_item_delay) >= MilliTime())
                )
                {
                    /* Do rename procedure. */
                    if((lw->entry_pos == z) &&
                       (lw->entry_pos >= 0)
                    )
                    {
                        ListWinMapCVPrompt(lw, LW_CVP_MODE_RENAME);

                        /* Reset last_click. */
                        last_click = 0;

                        events_handled++;
                        return(events_handled);
                    }
                    else
                    {
                        /* Record last_click. */
                        last_click = MilliTime();
                    }
                }
                else
                {
                    /* Record last click. */
                    last_click = MilliTime();
                }
                events_handled++;
            }
            break;

          /* ****************************************************** */
          case ButtonRelease:
            /* Change button states. */
            if(event->xbutton.button == Button1)
                static_wlist::button1_state = False;
            if(event->xbutton.button == Button2)
                static_wlist::button2_state = False;
            if(event->xbutton.button == Button3)
                static_wlist::button3_state = False;


            if(event->xany.window == lw->toplevel)
            {
                /* Manage drag on same window. */
                if(static_wlist::drag_active &&
                   (lw->entry != NULL) &&
                   (lw->toplevel == static_wlist::drag_start_w)
                )
                {
                    /* Sanitize lw->row_height. */
                    if(lw->row_height < 1) 
                        lw->row_height = 1;

                    /* Calculate which position ButtonRelease was on. */
                    new_entry_pos = (lw->sb.y_win_pos
                        + event->xbutton.y - (int)lw->top_margin) /
                        (int)lw->row_height;

                    /* Sanitize entry position. */
                    if(new_entry_pos >= lw->total_entries)
                        new_entry_pos = lw->total_entries - 1;
                    if(new_entry_pos < 0)
                        new_entry_pos = 0;

                    if(static_wlist::drag_start_entry_pos >= lw->total_entries)
                        static_wlist::drag_start_entry_pos = lw->total_entries - 1;
                    if(static_wlist::drag_start_entry_pos < 0)
                        static_wlist::drag_start_entry_pos = 0;
        
                    /* Get pointer to static_wlist::drag_start_entry_pos entry. */
                    list_entry_ptr = lw->entry[static_wlist::drag_start_entry_pos];


                    /* Shift positions. */
                    if(static_wlist::drag_start_entry_pos > new_entry_pos)
                    {
                        for(i = static_wlist::drag_start_entry_pos; i > new_entry_pos; i--)
                            lw->entry[i] = lw->entry[i - 1];
                    }
                    else if(static_wlist::drag_start_entry_pos < new_entry_pos)
                    {
                        for(i = static_wlist::drag_start_entry_pos; i < new_entry_pos; i++)
                            lw->entry[i] = lw->entry[i + 1];
                    }
                    else
                    {
                        /* Nothing to do if positions are the same. */
                    }

                    /* Set new entry pointer. */
                    lw->entry[new_entry_pos] = list_entry_ptr;

                    /* Change selected entry to the new one. */
                    lw->entry_pos = new_entry_pos;
                }
                    
                events_handled++;
            }

	    /* Stop drag as needed. */
	    if(static_wlist::drag_active)
		ListWinDoDragStop(lw);

            break;

          /* ****************************************************** */
          case MotionNotify:
            if(event->xany.window == lw->toplevel)
            {
                /* Was not previously in drag? */
                if(!static_wlist::drag_active &&
                   static_wlist::button1_state &&
                   lw->allow_drag	/* This list allows drag? */
                )
                {
                    /* Check drag tolorance. */
                    x = event->xmotion.x;
                    y = event->xmotion.y;
                    if((x < (static_wlist::drag_start_x - (LW_DRAG_TOLORANCE / 2))) ||
                       (x >= (static_wlist::drag_start_x + (LW_DRAG_TOLORANCE / 2))) ||
                       (y < (static_wlist::drag_start_y - (LW_DRAG_TOLORANCE / 2))) ||
                       (y >= (static_wlist::drag_start_y + (LW_DRAG_TOLORANCE / 2)))
                    )
                    {
			ListWinDoDragStart(
			    lw, lw->entry_pos, event->xany.window
			);
                    }
                }
		events_handled++;
                return(events_handled);
            }                 
            break;

          /* ****************************************************** */
          case KeyPress:
            /* Skip if not in focus. */
            if(!lw->is_in_focus)
                break;

	    /* Escape (for cv_prompt). */
            if(event->xkey.keycode == osw_keycode.esc)
	    {
		if(lw->cv_prompt_mode != LW_CVP_MODE_NONE)
		{
		    ListWinUnmapCVPrompt(lw);

		    events_handled++;
		    return(events_handled);
		}
	    }
            /* Enter (for cv_prompt). */
            else if((event->xkey.keycode == osw_keycode.enter) ||
                    (event->xkey.keycode == osw_keycode.np_enter)
	    )
            {
                if(lw->cv_prompt_mode != LW_CVP_MODE_NONE)
                {   
                    ListWinCVPromptApply(lw);

                    ListWinUnmapCVPrompt(lw);

		    ListWinDraw(lw);

                    events_handled++;
                    return(events_handled);
                }
            }

	    /* Do not handle any more types of keys if in cv mode. */
	    if(lw->cv_prompt_mode != LW_CVP_MODE_NONE)
		break;

            /* Up. */
            if(event->xkey.keycode == osw_keycode.cursor_up)
            {
                /* Sanitize lw->row_height. */
                if(lw->row_height < 1)
                    lw->row_height = 1;

                /* Move selected entry position up. */
                lw->entry_pos--;

                /* Sanitize entry position. */  
                if(lw->entry_pos >= lw->total_entries)
                    lw->entry_pos = lw->total_entries - 1;
                if(lw->entry_pos < 0)
                    lw->entry_pos = 0;

                /* Move scrolled position as needed. */
                if((lw->sb.y_win_pos / (int)lw->row_height) > lw->entry_pos)
                {
                    lw->sb.y_win_pos = lw->entry_pos * (int)lw->row_height;

                    /* Redraw scroll bars. */
                    SBarDraw(
                        &lw->sb,
                        wattr.width,
                        wattr.height,
                        wattr.width,
                        (lw->total_entries + 2) * (int)lw->row_height
                    );
                }

                events_handled++;
            }
            /* Down. */
            else if(event->xkey.keycode == osw_keycode.cursor_down)
            {
                /* Sanitize lw->row_height. */
                if(lw->row_height < 1)
                    lw->row_height = 1;

                /* Move selected entry position down. */
                lw->entry_pos++;

                /* Sanitize entry position. */
                if(lw->entry_pos >= lw->total_entries)
                    lw->entry_pos = lw->total_entries - 1;
                if(lw->entry_pos < 0)
                    lw->entry_pos = 0;

                /* Move scrolled position as needed. */
                OSWGetWindowAttributes(lw->toplevel, &wattr);
                if(((int)lw->sb.y_win_pos + (int)wattr.height) <=
                   ((lw->entry_pos + 1) * (int)lw->row_height)
                )
                {
                    lw->sb.y_win_pos = ((lw->entry_pos + 1) *
                        (int)lw->row_height) - (int)wattr.height + 8;

                    if((int)lw->sb.y_win_pos < 0)
                        lw->sb.y_win_pos = 0;

                    /* Redraw scroll bars. */
                    SBarDraw(
                        &lw->sb,
                        wattr.width,
                        wattr.height,
                        wattr.width,
                        (lw->total_entries + 2) * (int)lw->row_height
                    );
                }

                events_handled++;
	    }
            /* Enter. */
            else if((event->xkey.keycode == osw_keycode.enter) ||
                    (event->xkey.keycode == osw_keycode.np_enter)
            )
            {
                if(lw->func_cb != NULL)
                {
		    lw->func_cb(lw->client_data);
                    
                    events_handled++;
                    return(events_handled);
                }
            }
            break;

          /* ****************************************************** */
          case KeyRelease:
	    if(!lw->is_in_focus)
		break;

	    break;

          /* ****************************************************** */
          case Expose:
            if(event->xany.window == lw->toplevel)
                events_handled++;
            break;

          /* ****************************************************** */
          case FocusIn:
            if(event->xany.window == lw->toplevel)
            {
                lw->is_in_focus = 1;

                events_handled++;
		return(events_handled);
            }
            break;

          /* ****************************************************** */
          case FocusOut:
            if(event->xany.window == lw->toplevel)
            {
                lw->is_in_focus = 0;

                events_handled++;
		return(events_handled);
            }
            break;

          /* ****************************************************** */
          case VisibilityNotify:
            if(event->xany.window == lw->toplevel)
            {
                lw->visibility_state = event->xvisibility.state;

                events_handled++;
                return(events_handled);
            }
            break;
        }

        /* Scroll bar. */
        if(events_handled <= 0)
            events_handled += SBarManage(
                &lw->sb,
                lw->width,
                lw->height,
                lw->width,
                (lw->total_entries + 2) * (int)lw->row_height,
                event
            );

        /* Redraw as needed. */
        if(events_handled > 0)
            ListWinDraw(lw);

        /* Change values prompt. */
        if(lw->cv_prompt_mode != LW_CVP_MODE_NONE)
            events_handled += PromptManage(&lw->cv_prompt, event);


        return(events_handled);
}



/*
 *	Applies changes on the cv prompt.
 */
int ListWinCVPromptApply(list_window_struct *lw)
{
	int entry_num;
	char *strptr;
	list_window_entry_struct *entry_ptr;


	if(lw == NULL)
	    return(-1);

	/* Not in change values prompt mode? */
	if(lw->cv_prompt_mode == LW_CVP_MODE_NONE)
	    return(-1);


	/* Get entry number and pointer. */
	entry_num = lw->cv_prompt_entry_pos;
	if(ListIsItemAllocated(lw, entry_num))
	    entry_ptr = lw->entry[entry_num];
	else
	    return(-1);

	/* Get pointer to change values prompt buffer. */
	strptr = PromptGetS(&lw->cv_prompt);
	if(strptr == NULL)
	    return(-1);

	/* Set new name of entry. */
	free(entry_ptr->name);
	entry_ptr->name = StringCopyAlloc(strptr);


	return(0);
}

/*
 *	Map change values prompt on list.
 */
void ListWinMapCVPrompt(list_window_struct *lw, int mode)
{
        int x, y, entry_num, name_len;
        unsigned int width, height;
	unsigned int icon_width, icon_height;
	list_window_entry_struct *entry_ptr;


        if(lw == NULL)
            return;


        /* Already changing values in another mode? */
        if(lw->cv_prompt_mode != LW_CVP_MODE_NONE)
            return;


	/* Is selected entry valid? */
	entry_num = lw->entry_pos;
	if(ListIsItemAllocated(lw, entry_num))
	    entry_ptr = lw->entry[entry_num];
	else
	    return;


        /* Initialize change value prompt. */
        if(
            PromptInit(
                &lw->cv_prompt,
                lw->toplevel,  
                10,
                ((int)lw->height / 2) - (LW_PROMPT_HEIGHT / 2),
                MAX((int)lw->width - 20, 100),
                LW_PROMPT_HEIGHT,
                PROMPT_STYLE_FLUSHED,
                NULL,
                1024,			/* Buffer length. */
                0,			/* Number of history buffers. */
                NULL
            )
        )
            return;



        /* Change focuses. */
        lw->is_in_focus = 1;
        lw->sb.is_in_focus = 0;


	/* Record selected entry for change values prompt. */
	lw->cv_prompt_entry_pos = lw->entry_pos;

	switch(mode)
	{
          case LW_CVP_MODE_RENAME:
	    /* Set entry name on change values prompt. */
            if(entry_ptr->name != NULL)
	    {
		name_len = strlen(entry_ptr->name);
		PromptSetS(&lw->cv_prompt, entry_ptr->name);
	    }
	    else
	    {
		name_len = 0;
		PromptSetS(&lw->cv_prompt, "(null)");
	    }

	    /* Get sizes of icon image. */
	    if(entry_ptr->image != NULL)
	    {
		icon_width = entry_ptr->image->width;
		icon_height = entry_ptr->image->height;
	    }
	    else
	    {
		icon_width = 0;
		icon_height = 0;
	    }

	    /* Calculate position to place change values prompt. */
	    x = (int)lw->left_margin + (int)icon_width;
	    y = (lw->row_height * entry_num) - lw->sb.y_win_pos + 
		(int)lw->top_margin;

	    width = MAX((name_len * (int)lw->char_width) + 20, 200);
	    height = LW_PROMPT_HEIGHT;

            /*   Move the change values prompt to the right
             *   position and map it.
             */
            lw->cv_prompt_mode = mode;
            OSWReparentWindow(lw->cv_prompt.toplevel, lw->toplevel);
            OSWMoveResizeWindow(lw->cv_prompt.toplevel, x, y, width, height);
	    PromptMarkBuffer(&lw->cv_prompt, PROMPT_POS_END);
            PromptMap(&lw->cv_prompt);
	    lw->cv_prompt.is_in_focus = 1;
	    break;

	  default:
	    break;
	}


	return;
}

/*
 *	Unmap change values prompt on list window.
 */
void ListWinUnmapCVPrompt(list_window_struct *lw)          
{
        if(lw == NULL)
            return;   


        /* Reset target item. */
        lw->cv_prompt_entry_pos = -1;

        /* Reset mode. */
        lw->cv_prompt_mode = LW_CVP_MODE_NONE;

        /* Destroy change values prompt. */
        PromptDestroy(&lw->cv_prompt);
                 
        /* Make sure it is out of focus. */
        lw->cv_prompt.is_in_focus = 0;


        return;
}

/*
 *	List window map.
 */
void ListWinMap(list_window_struct *lw)
{
        if(lw == NULL)
            return;

        lw->map_state = 0;
        ListWinDraw(lw);

        /* Reset drag globals. */
        static_wlist::button1_state = False;
        static_wlist::button2_state = False;
        static_wlist::button3_state = False;
        static_wlist::drag_active = False;

        return;
}

/*
 *	List window unmap.
 */
void ListWinUnmap(list_window_struct *lw)
{
	if(lw == NULL)
            return;

        OSWUnmapWindow(lw->toplevel);
        lw->map_state = 0;
        lw->visibility_state = VisibilityFullyObscured;
        lw->is_in_focus = 0;

        /* Destroy buffers. */
        OSWDestroyPixmap(&lw->toplevel_buf);

        return;    
}

/*
 *	List window destroy.
 */
void ListWinDestroy(list_window_struct *lw)
{  
        if(lw == NULL)
	    return;


        /* Delete widget from regeristry. */
        WidgetRegDelete(lw);


        /* Free all list items. */
        ListDeleteAllItems(lw);


        if(IDC())  
        {
            PromptDestroy(&lw->cv_prompt);

            SBarDestroy(&lw->sb);

	    OSWDestroyWindow(&lw->toplevel);            
            OSWDestroyPixmap(&lw->toplevel_buf);
        }


        lw->map_state = 0;
        lw->visibility_state = VisibilityFullyObscured;
        lw->is_in_focus = 0;  
        lw->x = 0;
        lw->y = 0;
        lw->width = 0;
        lw->height = 0;
	lw->font = NULL;
        lw->next = NULL;
        lw->prev = NULL;

	WidgetDestroyCursor(&lw->drag_cursor);

        /* Clear repeat record as needed. */
        if(lw == lw_repeat_record[0].lw)
            ListWinRepeatRecordClear();


        return;
}

/*
 *	Creates a WCursor based on a list entry's icon.
 *
 *	Can return NULL on error or if the entry has no icon image.
 */
WCursor *ListWinCreateCursorFromEntry(
	list_window_struct *lw,
	int entry_num
)
{
#ifndef X_H
	return(NULL);
#else
        GC gc;
        XGCValues gcv;
	pixel_t white_pix, black_pix;
	int i, x, y, status, bytes_per_pixel, bytes_per_line;
	u_int8_t *img_data;
	u_int8_t *ptr8;
	u_int16_t *ptr16;
	u_int32_t *ptr32;

	list_window_entry_struct *entry_ptr;
	image_t *image;
	unsigned int width, height;	/* Of image. */
        unsigned int req_width, req_height;	/* Of pixmap and mask. */
	pixmap_t pixmap, mask;
	WCursor *wcursor;
        XColor fg_xcolor, bg_xcolor;



	if(lw == NULL)
	    return(NULL);

	if(ListIsItemAllocated(lw, entry_num))
	    entry_ptr = lw->entry[entry_num];
	else
	    return(NULL);

	image = entry_ptr->image;
	if(image == NULL)
	    return(NULL);

	img_data = (u_int8_t *)image->data;
	if(img_data == NULL)
	    return(NULL);

	width = image->width;
	height = image->height;

	if(((int)width <= 0) ||
	   ((int)height <= 0)
	)
	    return(NULL);


        /* Check recommended size from GUI. */
        status = XQueryBestCursor(  
            osw_gui[0].display,
            osw_gui[0].root_win,
            width, height,
            &req_width, &req_height
        );
        if(!status)
        {  
           fprintf(stderr,
 "ListWinCreateCursorFromEntry(): Cannot get recommended cursor size for %i %i\n",
                width, height
            );
            req_width = width; 
            req_height = height;   
        }

	/* Sanitize/limit sizes, pixmap size must be <= image size */
        if(req_width > width)
            req_width = width;
	if(req_height > height)
	    req_height = height;


	/* Create a 1 bit depth pixmap and mask. */
	pixmap = XCreatePixmap(
            osw_gui[0].display,
            osw_gui[0].root_win,
            req_width, req_height,
            1			/* Depth of 1. */
        );

	/* Create a 1 bit depth pixmap and mask. */
	mask = XCreatePixmap(
            osw_gui[0].display,
            osw_gui[0].root_win,
            req_width, req_height,
            1			/* Depth of 1. */
        );

        /* Create 1 bit depth graphics context. */
	white_pix = osw_gui[0].white_pix;
	black_pix = osw_gui[0].black_pix;
	gcv.function = GXcopy;
        gcv.plane_mask = 1;		/* Monochrome. */
        gcv.foreground = white_pix;
        gcv.background = black_pix;
        gcv.line_width = 1;
        gc = XCreateGC(   
            osw_gui[0].display,
            pixmap,
            GCFunction | GCPlaneMask | GCForeground | GCBackground |
                GCLineWidth,
            &gcv
        );


	/* Copy image data to pixmap and mask. */
	switch(osw_gui[0].depth)
	{
	  /* 8 bits. */
	  case 8:
	    ptr8 = (u_int8_t *)img_data;
            bytes_per_pixel = BYTES_PER_PIXEL8;
            bytes_per_line = (int)width * bytes_per_pixel;	/* On image. */
            for(y = 0; y < (int)req_height; y++)
            {
                for(x = 0; x < (int)req_width; x++)
                {
                    ptr8 = (u_int8_t *)(&img_data[
                        (y * bytes_per_line) +
                        (x * bytes_per_pixel)
                    ]);

                    i = (
                        (((*ptr8) & 0xe0)) +
                        (((*ptr8) & 0x1c) << 3) +
                        (((*ptr8) & 0x03) << 6)
                    );
                    if(i < ((0xFF * 3) / 2))
                        XSetForeground(osw_gui[0].display, gc, black_pix);
                    else
                        XSetForeground(osw_gui[0].display, gc, white_pix);
                    XDrawPoint(osw_gui[0].display, pixmap, gc, x, y);

                    if(*ptr8)
                        XSetForeground(osw_gui[0].display, gc, white_pix);
                    else
                        XSetForeground(osw_gui[0].display, gc, black_pix);
                    XDrawPoint(osw_gui[0].display, mask, gc, x, y);
                }
            }
	    break;

	  /* 15 bits. */
	  case 15:
            bytes_per_pixel = BYTES_PER_PIXEL16;
            bytes_per_line = (int)width * bytes_per_pixel;	/* On image. */
            for(y = 0; y < (int)req_height; y++)
            {
                for(x = 0; x < (int)req_width; x++)
                {
                    ptr16 = (u_int16_t *)(&img_data[
                        (y * bytes_per_line) +
                        (x * bytes_per_pixel)
                    ]);

                    i = (
                        (((*ptr16) & 0x7C00) >> 7) +
                        (((*ptr16) & 0x03E0) >> 2) +
                        (((*ptr16) & 0x001F) << 3)
                    );
                    if(i < ((0xFF * 3) / 2))
                        XSetForeground(osw_gui[0].display, gc, black_pix);
                    else
                        XSetForeground(osw_gui[0].display, gc, white_pix);
                    XDrawPoint(osw_gui[0].display, pixmap, gc, x, y);

                    if(*ptr16)
                        XSetForeground(osw_gui[0].display, gc, white_pix);
                    else
                        XSetForeground(osw_gui[0].display, gc, black_pix);
                    XDrawPoint(osw_gui[0].display, mask, gc, x, y);
                }
            }
            break;

	  /* 16 bits. */
	  case 16:
	    bytes_per_pixel = BYTES_PER_PIXEL16;
	    bytes_per_line = (int)width * bytes_per_pixel;	/* On image. */
	    for(y = 0; y < (int)req_height; y++)
	    {
		for(x = 0; x < (int)req_width; x++)
		{
		    ptr16 = (u_int16_t *)(&img_data[
			(y * bytes_per_line) +
			(x * bytes_per_pixel)
		    ]);

		    i = (
			(((*ptr16) & 0xf800) >> 8) +
                        (((*ptr16) & 0x07E0) >> 3) +
                        (((*ptr16) & 0x001F) << 3)
		    );
		    if(i < ((0xFF * 3) / 2))
			XSetForeground(osw_gui[0].display, gc, black_pix);
                    else
			XSetForeground(osw_gui[0].display, gc, white_pix);
                    XDrawPoint(osw_gui[0].display, pixmap, gc, x, y);

                    if(*ptr16)
                        XSetForeground(osw_gui[0].display, gc, white_pix);
                    else
                        XSetForeground(osw_gui[0].display, gc, black_pix);
                    XDrawPoint(osw_gui[0].display, mask, gc, x, y);
		}
	    }
	    break;

	  /* 24 or 32 bits. */
	  case 24:
	  case 32:
            bytes_per_pixel = BYTES_PER_PIXEL32;
            bytes_per_line = (int)width * bytes_per_pixel;	/* On image. */
            for(y = 0; y < (int)req_height; y++)
            {
                for(x = 0; x < (int)req_width; x++)
                {
                    ptr32 = (u_int32_t *)(&img_data[
                        (y * bytes_per_line) +
                        (x * bytes_per_pixel)
                    ]);

                    i = (
                        (((*ptr32) & 0x00ff0000) >> 16) +
                        (((*ptr32) & 0x0000ff00) >> 8) +
                        (((*ptr32) & 0x000000ff))
                    );
                    if(i < ((0xFF * 3) / 2))
                        XSetForeground(osw_gui[0].display, gc, black_pix);
                    else
                        XSetForeground(osw_gui[0].display, gc, white_pix);
                    XDrawPoint(osw_gui[0].display, pixmap, gc, x, y);

                    if(*ptr32)
                        XSetForeground(osw_gui[0].display, gc, white_pix);
                    else
                        XSetForeground(osw_gui[0].display, gc, black_pix);
                    XDrawPoint(osw_gui[0].display, mask, gc, x, y);
                }
            }
	    break;
	}


	/* Allocate a new WCursor structure. */
	wcursor = (WCursor *)malloc(sizeof(WCursor));
	if(wcursor == NULL)
	{
	    OSWDestroyPixmap(&pixmap);
            OSWDestroyPixmap(&mask);
            XFreeGC(osw_gui[0].display, gc);

	    return(NULL);
	}

	wcursor->depth = 1;	/* X requires depth be 1. */

        wcursor->x = ((int)req_width / 2);
        wcursor->y = ((int)req_height / 2);
        wcursor->width = req_width;
        wcursor->height = req_height;

	fg_xcolor.red = 0xFFFF;
	fg_xcolor.green = 0xFFFF;
	fg_xcolor.blue = 0xFFFF;

	bg_xcolor.red = 0x0000;
	bg_xcolor.green = 0x0000;
	bg_xcolor.blue = 0x0000;

        wcursor->cursor = XCreatePixmapCursor(
            osw_gui[0].display,
            pixmap,
            mask,
            &fg_xcolor,
            &bg_xcolor,
            wcursor->x,
	    wcursor->y
        );  

	wcursor->color.a = 0x00;
        wcursor->color.r = (u_int8_t)(fg_xcolor.red >> 8);
        wcursor->color.g = (u_int8_t)(fg_xcolor.green >> 8);
        wcursor->color.b = (u_int8_t)(fg_xcolor.blue >> 8);


	/* Destroy tempory resources. */
        OSWDestroyPixmap(&pixmap);
        OSWDestroyPixmap(&mask);
        XFreeGC(osw_gui[0].display, gc);


	return(wcursor);
#endif	/* X_H */
}


/*
 *	Procedure to start drag.
 */
void ListWinDoDragStart(
	list_window_struct *lw,
	int entry_pos,		/* Drag started on/for this entry. */
	win_t start_w
)
{
	if(lw == NULL)
	    return;


	/* Unmap change values prompt (as needed). */
        if(lw->cv_prompt_mode != LW_CVP_MODE_NONE)
	    ListWinUnmapCVPrompt(lw);


	/* Set up drag values. */
	static_wlist::drag_active = True;

	static_wlist::drag_start_entry_pos = entry_pos;
	static_wlist::drag_start_w = start_w;


	/* Destroy old drag cursor if exists. */
	WidgetDestroyCursor(&lw->drag_cursor);
	/* Create new drag cursor. */
	lw->drag_cursor = ListWinCreateCursorFromEntry(lw, lw->entry_pos);


	/* Grab pointer. */
	OSWGrabPointer(
	    lw->toplevel,
	    True,
            ButtonReleaseMask | PointerMotionMask,
	    GrabModeAsync, GrabModeAsync,
	    None,
	    ((lw->drag_cursor == NULL) ?
	        0 : lw->drag_cursor->cursor)
	);

	/* Need to set up somet repeat record values. */
	ListWinRepeatRecordSet(
	    lw,
	    widget_global.pulist_repeat_delay,	/* Use popup list delay for now. */
	    LW_OP_CODE_NONE	/* No operation. */
	);  

        ListWinDraw(lw);

	return;
}


/*
 *	Procedure to stop drag.
 */
void ListWinDoDragStop(list_window_struct *lw)
{
	OSWUngrabPointer();

	/* Destroy drag cursor for this list widget. */
	if(lw != NULL)
	    WidgetDestroyCursor(&lw->drag_cursor);

	ListWinRepeatRecordClear();

	static_wlist::drag_active = False;
	static_wlist::drag_start_w = 0;

	OSWGUISync(False);


	return;
}


/*
 *	Sets the list window repeat records.
 */
void ListWinRepeatRecordSet(
	list_window_struct *lw,
	long dsec, int op_code
)
{
        lw_repeat_record[0].lw = lw;
        lw_repeat_record[0].next_repeat = MilliTime() + MAX(dsec, 0);
        lw_repeat_record[0].op_code = op_code;

	return;
}


/*
 *	Clears the list window repeat record.
 */
void ListWinRepeatRecordClear()
{
	lw_repeat_record[0].lw = NULL;
        lw_repeat_record[0].next_repeat = 0;
        lw_repeat_record[0].op_code = LW_OP_CODE_NONE;

        return;
}


/*
 *	Manages list window repeats.
 */
int ListWinManageRepeat(event_t *event) 
{
        int x, y, root_x, root_y, y_max;
        int events_handled = 0;
        list_window_struct *lw;
        win_attr_t wattr;

 
        /*
         *   Note: The event information is not used and not needed here. 
         */

        /* Is a list pointer set in the record? */
        if(lw_repeat_record[0].lw != NULL)  
        {
            lw = lw_repeat_record[0].lw;

            /* Time due for repeat? */
            if(MilliTime() >= lw_repeat_record[0].next_repeat)
            {
                /* Check pointer's position. */
                OSWGetWindowAttributes(lw->toplevel, &wattr);

                OSWGetPointerCoords(
                    lw->toplevel,
                    &root_x, &root_y,
                    &x, &y
                );
                if(y < 0)
                {
                    lw_repeat_record[0].op_code = 
                        LW_OP_CODE_SCROLL_UP;
                }
                else if(y >= (int)wattr.height)
                {
                    lw_repeat_record[0].op_code =
                        LW_OP_CODE_SCROLL_DOWN;
                }
                else
                {
                    lw_repeat_record[0].op_code = LW_OP_CODE_NONE;
                }

                /* Handle scroll operation. */
                switch(lw_repeat_record[0].op_code)
                {
                  case LW_OP_CODE_SCROLL_UP:  
                    lw->sb.y_win_pos -= lw->row_height;
                    events_handled++;
                    break;

                  case LW_OP_CODE_SCROLL_DOWN:
                    lw->sb.y_win_pos += lw->row_height;
                    events_handled++;
                    break;

                  case LW_OP_CODE_NONE:
/* Do not clear, only ButtonRelease should do that.
                    ListWinRepeatRecordClear();
 */
                    return(events_handled);
                    break;

                  default:
                    break;
                }

                /* Sanitize position. */
                y_max = ((lw->total_entries + 2) *
		    (int)lw->row_height) - wattr.height;
                if(lw->sb.y_win_pos > y_max)
                    lw->sb.y_win_pos = y_max;
                if(lw->sb.y_win_pos < 0)
                    lw->sb.y_win_pos = 0;

                /* Redraw. */
                ListWinDraw(lw);

                /* Redraw scroll bars. */
                SBarDraw(
                    &lw->sb,
                    wattr.width,
                    wattr.height,
                    wattr.width, 
                    (lw->total_entries + 2) * (int)lw->row_height
                );

                    
                /* Schedual next operation. */
                lw_repeat_record[0].next_repeat =
/* Use pulist repeat for now. */
                    MilliTime() + widget_global.pulist_repeat_interval;


                events_handled++;
            }
        }


        return(events_handled);
}




