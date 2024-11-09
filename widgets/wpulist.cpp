// widgets/wpulist.cpp
/*
                        Widget: Popup List

	Functions:

	int PUListIsItemAllocated(popup_list_struct *list, int n)
        char *PUListGetSelItemName(popup_list_struct *list)
	int PUListAddItem(
		popup_list_struct *list,
		char *name,
		bool_t disabled
	)
	void PUListDeleteAllItems(popup_list_struct *list)

	int PUListInit(
		popup_list_struct *list,
		win_t parent,
		int x, int y,
		unsigned int width, unsigned int height,
		int popup_list_vis_items,
		int direction,
		void *client_data,
		int (*func_cb)(void *)
	)
	int PUListResize(
		popup_list_struct *list,
		unsigned int width, unsigned int height,
		int popup_list_vis_items
	)
	int PUListDraw(popup_list_struct *list, int amount)
	int PUListManage(popup_list_struct *list, event_t *event)
	void PUListMap(popup_list_struct *list)
	void PUListUnmap(popup_list_struct *list)
	void PUListMapList(popup_list_struct *list)
	void PUListUnmapList(popup_list_struct *list)
	void PUListDestroy(popup_list_struct *list)

        void PUListRepeatRecordSet(
		popup_list_struct *list,
                long dsec, int op_code
        )
        void PUListRepeatRecordClear()
	int PUListManageRepeat(event_t *event)

	---

 */

#include <math.h>

#include "../include/widget.h"

#ifndef MAX
#define MIN(a,b)        (((a) < (b)) ? (a) : (b))
#define MAX(a,b)	(((a) > (b)) ? (a) : (b))
#endif
/* #define MIN(a,b)	(((a) < (b)) ? (a) : (b)) */
/* #define MAX(a,b)	(((a) > (b)) ? (a) : (b)) */


/*
 *	Size constants (in pixels):
 */
#define PULIST_DEF_WIDTH	90
#define PULIST_DEF_HEIGHT	26

#define PULIST_CHAR_WIDTH	7
#define PULIST_CHAR_HEIGHT	14

/*
 *	Height of each item on the popup list popup list:
 */
#define PULIST_ITEM_HEIGHT	20


/*
 *	Margin height on popup list popup list:
 */
#define PULIST_POPUP_CURSOR_MARGIN	8


/*
 *	Size of cursors on popup list:
 */
#define PULIST_SCROLL_CURSOR_WIDTH	8
#define PULIST_SCROLL_CURSOR_HEIGHT	8




/*
 *	Popup list repeat record: 
 */
#define PULIST_OP_CODE_NONE         0
#define PULIST_OP_CODE_SCROLL_UP    1   /* One unit. */
#define PULIST_OP_CODE_SCROLL_DOWN  2   /* One unit. */

typedef struct {
 
    popup_list_struct *list;   
    long next_repeat;           /* Next repeat in ms. */
    int op_code;                /* What to do. */

} pulist_repeat_record_struct;  
pulist_repeat_record_struct pulist_repeat_record[1];


/*
 *	Returns true if item n is allocated on popup list list.
 */
int PUListIsItemAllocated(popup_list_struct *list, int n)
{
	if(list == NULL)
	    return(0);
	else if((list->item == NULL) ||
                (n < 0) ||
                (n >= list->total_items)
	)
	    return(0);
	else if(list->item[n] == NULL)
	    return(0);
	else
	    return(1);
}



/*
 *	Returns a dynamically allocated string (which should NOT
 *	be free'ed by the calling function) that is the allocated
 *	name of the selected item on the list.
 *
 *	Returns NULL on error or if no item is selected.
 */
char *PUListGetSelItemName(popup_list_struct *list)
{
	int i;

	if(list == NULL)
	    return(NULL);


	i = list->sel_item;
	if((i < 0) && (i >= list->total_items))
	    return(NULL);
	if(list->item[i] == NULL)
            return(NULL);

	return(list->item[i]->name);


	return(NULL);
}


/*
 *	Allocates a new item on list.
 *
 *	Returns list item number or -1 on error.
 */
int PUListAddItem(
	popup_list_struct *list,
	char *name,
	bool_t disabled
) 
{
	int i, len;
	win_attr_t wattr;


	if(list == NULL)
	    return(-1);


	/* Sanitize total. */
	if(list->total_items < 0)
	    list->total_items = 0;

	i = list->total_items;
	list->total_items++;

	list->item = (popup_list_item_struct **)realloc(
	    list->item,
	    list->total_items * sizeof(popup_list_item_struct *)
	);
	if(list->item == NULL)
	{
	    list->total_items = 0;
	    return(-1);
	}


	list->item[i] = (popup_list_item_struct *)calloc(
	    1,
	    sizeof(popup_list_item_struct)
	);
	if(list->item[i] == NULL)
	{
	    return(-1);
	}


	/* Set name. */
	if(name != NULL)
	{
	    len = strlen(name);
	    list->item[i]->name = (char *)calloc(1,
		(len + 1) * sizeof(char)
	    );
	    if(list->item[i]->name != NULL)
	    {
		strcpy(list->item[i]->name, name);
	    }
	}

	/* Set disabled state. */
	list->item[i]->disabled = disabled;


	/* Recalculate maximum sizes on widget. */
	OSWGetWindowAttributes(list->popup_list, &wattr);

	list->list_max_width = wattr.width;

	list->list_max_height = list->total_items * PULIST_ITEM_HEIGHT;



	return(i);
}

/*
 *	Delete all items on pulist.
 */
void PUListDeleteAllItems(popup_list_struct *list) 
{
	int i;


	if(list == NULL)
	    return;

	for(i = 0; i < list->total_items; i++)
	{
	    if(list->item[i] == NULL)
		continue;

	    free(list->item[i]->name);

	    free(list->item[i]);
	}
	free(list->item);
	list->item = NULL;

	list->total_items = 0;
	list->sel_item = 0;
	list->prev_sel_item = 0;


	return;
}


/*
 *	Initializes pulist.
 */
int PUListInit(
	popup_list_struct *list,
	win_t parent,
	int x, int y,
	unsigned int width, unsigned int height,
	int popup_list_vis_items,
	int direction,
	void *client_data,
	int (*func_cb)(void *)
)
{
	int i;


	if((list == NULL) ||
	   (parent == 0)
	)
	    return(-1);


	if(width == 0)
	    width = PULIST_DEF_WIDTH;
	if(height == 0)
	    height = PULIST_DEF_HEIGHT;


	/* Reset values. */
	list->map_state = 0;
	list->visibility_state = VisibilityFullyObscured;
	list->is_in_focus = 0;
	list->x = x;
	list->y = y;
	list->width = width;
	list->height = height;
	list->disabled = False;
	list->font = OSWQueryCurrentFont();
	list->next = NULL;
	list->prev = NULL;

	list->popup_map_state = 0;

	list->popup_list_vis_items = MAX(popup_list_vis_items, 1);



	/* Create selected item label toplevel. */
	if(
	    OSWCreateWindow(
		&list->toplevel,
		parent,
		list->x, list->y,
		list->width, list->height
	    )
	)
	    return(-1);

	list->toplevel_buf = 0;

	OSWSetWindowInput(
	    list->toplevel,
	    ButtonPressMask | ExposureMask
	);

	/* Map button. */
        if(
            PBtnInit(
                &list->map_btn,
                list->toplevel,
                (int)list->width - 21, 1,
                20, MAX((int)list->height - 2, 2),
                ((widget_global.force_mono) ? "^" : NULL),
                PBTN_TALIGN_CENTER,
		((widget_global.force_mono) ?
                    NULL : widget_global.pulist_map_icon_img),
                list,
		NULL
            )
        )
            return(-1);



	/* Create popup toplevel. */
	i = PULIST_ITEM_HEIGHT * list->popup_list_vis_items;
	switch(direction)
	{
	  case PULIST_POPUP_UP:
	    if(
                OSWCreateWindow(
                    &list->popup_toplevel,
                    parent,
                    list->x,
		    list->y - i + (int)list->height -
			(2 * PULIST_POPUP_CURSOR_MARGIN),
                    list->width,
		    i + (2 * PULIST_POPUP_CURSOR_MARGIN) + 2
                )
            )
                return(-1);
	    break;

	  case PULIST_POPUP_DOWN:
            if(
                OSWCreateWindow(
                    &list->popup_toplevel,
                    parent,
                    list->x,
                    list->y - PULIST_POPUP_CURSOR_MARGIN,
                    list->width,
                    i + (2 * PULIST_POPUP_CURSOR_MARGIN) + 2
                )
            )
                return(-1);
            break;

	  default:	/* PULIST_POPUP_CENTER */
            if(
                OSWCreateWindow(
                    &list->popup_toplevel,
                    parent,
                    list->x,
                    list->y - (i / 2) - PULIST_POPUP_CURSOR_MARGIN - 1
                        + ((int)list->height / 2),
                    list->width,
                    i + (2 * PULIST_POPUP_CURSOR_MARGIN) + 2
                )
            )
                return(-1);
            break;
	}
        OSWSetWindowInput(list->popup_toplevel,
	    ButtonPressMask | ButtonReleaseMask | PointerMotionMask |
            ExposureMask
        );

	/* Create popup list. */
	if(
            OSWCreateWindow(
                &list->popup_list,
                list->popup_toplevel,
                1,
                1 + PULIST_POPUP_CURSOR_MARGIN,
                MAX((int)list->width - 2, 2),
                i
            )
        )
            return(-1);

	list->popup_list_buf = 0;

        OSWSetWindowInput(
	    list->popup_list,
	    ButtonPressMask | ButtonReleaseMask | PointerMotionMask |
            EnterWindowMask | LeaveWindowMask | ExposureMask
        );


	list->list_max_width = 0;
	list->list_max_height = 0;
	list->list_y_pos = 0;

	list->item = NULL;
	list->total_items = 0;
	list->sel_item = 0;
	list->prev_sel_item = 0;

	list->direction = direction;

	list->client_data = client_data;
	list->func_cb = func_cb;


	/* Add to widget regeristry. */
	WidgetRegAdd((void *)list, WTYPE_CODE_PULIST);


	return(0);
}


/*
 *	Resizes pulist.
 */
int PUListResize(
	popup_list_struct *list,
	unsigned int width, unsigned int height,
	int popup_list_vis_items
)
{
	int i;
	win_attr_t wattr;


	if(list == NULL)
	    return(-1);


	if(popup_list_vis_items < 1)
	    popup_list_vis_items = 1;
	list->popup_list_vis_items = popup_list_vis_items;

	/* Destroy buffers. */
	OSWDestroyPixmap(&list->popup_list_buf);
	OSWDestroyPixmap(&list->popup_toplevel_buf);
        OSWDestroyPixmap(&list->toplevel_buf);

	/* Get new sizes. */
	OSWGetWindowAttributes(list->toplevel, &wattr);
	list->x = wattr.x;
	list->y = wattr.y;
	list->width = wattr.width;
	list->height = wattr.height;


	/* Change map button size. */
	PBtnDestroy(&list->map_btn);
	if(
	    PBtnInit(
                &list->map_btn,
                list->toplevel,
                (int)list->width - 21, 1,
                20, MAX((int)list->height - 2, 2),
                (widget_global.force_mono) ? "^" : NULL,
                PBTN_TALIGN_CENTER,
                (widget_global.force_mono) ?
                    NULL : widget_global.pulist_map_icon_img,
                (void *)list,
		NULL
	    )
	)
	    return(-1);
	if(list->map_state)
	    PBtnMap(&list->map_btn);


	/* Move and resize popup toplevel. */
	i = PULIST_ITEM_HEIGHT * list->popup_list_vis_items;
	switch(list->direction)
	{
	  case PULIST_POPUP_UP:
	    OSWMoveResizeWindow(
	        list->popup_toplevel,
                list->x,
                list->y - i + (int)list->height -
                    (2 * PULIST_POPUP_CURSOR_MARGIN),
                list->width,  
                i + (2 * PULIST_POPUP_CURSOR_MARGIN) + 2
	    );
	    break;

          case PULIST_POPUP_DOWN:
            OSWMoveResizeWindow(
                list->popup_toplevel,
                list->x,
                list->y - PULIST_POPUP_CURSOR_MARGIN,
                list->width,
                i + (2 * PULIST_POPUP_CURSOR_MARGIN) + 2
            );
            break;

          default:	/* PULIST_POPUP_CENTER */
            OSWMoveResizeWindow(
                list->popup_toplevel,
                list->x,
                list->y - (i / 2) - PULIST_POPUP_CURSOR_MARGIN - 1
                    + ((int)list->height / 2),
                list->width,
                i + (2 * PULIST_POPUP_CURSOR_MARGIN) + 2
            );
            break;
	}

	/* Resize popup list. */
	OSWMoveResizeWindow(
	    list->popup_list,
            1,
            1 + PULIST_POPUP_CURSOR_MARGIN,
            MAX((int)list->width - 2, 2),
            i
	);


	/* Redraw as needed. */
	if(list->map_state)
	    PUListDraw(list, PULIST_DRAW_AMOUNT_COMPLETE);


	return(0);
}


/*
 *	Draw pulist.
 */
int PUListDraw(popup_list_struct *list, int amount)
{
	int i, n, y;
	win_attr_t wattr;
        font_t *prev_font;


	if(list == NULL)
	    return(-1);


	/* Map as needed. */
	if(!list->map_state)
	{
	    OSWMapRaised(list->toplevel);
	    PBtnMap(&list->map_btn);

	    list->map_state = 1;
	    list->visibility_state = VisibilityUnobscured;

	    /* Change draw amount to complete. */
	    amount = PULIST_DRAW_AMOUNT_COMPLETE;
	}

        /* Recreate buffers as needed. */
        if(list->toplevel_buf == 0)
        {
            OSWGetWindowAttributes(list->toplevel, &wattr);
            if(OSWCreatePixmap(&list->toplevel_buf,
                wattr.width, wattr.height
            ))
                return(-1);
        }
        if(list->popup_toplevel_buf == 0)
        {
            OSWGetWindowAttributes(list->popup_toplevel, &wattr);
            if(OSWCreatePixmap(&list->popup_toplevel_buf,
                wattr.width, wattr.height
            ))
                return(-1);
        }
        if(list->popup_list_buf == 0)
        {
            OSWGetWindowAttributes(list->popup_list, &wattr);
            if(OSWCreatePixmap(&list->popup_list_buf,
                wattr.width, wattr.height
            ))
                return(-1);
        }


        prev_font = OSWQueryCurrentFont();
        OSWSetFont(list->font);


	/* Redraw selected item label. */
	if((amount == PULIST_DRAW_AMOUNT_COMPLETE) ||
           (amount == PULIST_DRAW_AMOUNT_LABEL)
	)
	{
	    OSWGetWindowAttributes(list->toplevel, &wattr);
	    OSWClearPixmap(list->toplevel_buf,
		wattr.width, wattr.height,
		(widget_global.force_mono) ?
		    osw_gui[0].black_pix :
                    widget_global.surface_editable_pix
	    );

	    /* Draw selected item. */
	    if(widget_global.force_mono)
		OSWSetFgPix(osw_gui[0].white_pix);
	    else
	        OSWSetFgPix(widget_global.editable_text_pix);
	    i = list->sel_item;
	    if((i >= 0) && (i < list->total_items))
	    {
		if(list->item[i] != NULL)
		{
		    OSWDrawString(
			(drawable_t)list->toplevel_buf,
			8, ((int)wattr.height / 2) + 5,
			list->item[i]->name
		    );
		}
	    }
	    OSWDrawSolidArc(
		(drawable_t)list->toplevel_buf,
		(int)wattr.width - 14, ((int)wattr.height / 2) - 2,
		10, 6,
		0, PI * 2
	    );


	    if(widget_global.force_mono)
                WidgetFrameButtonPixmap(
                    list->toplevel_buf, 
                    False,
                    wattr.width, wattr.height,
                    osw_gui[0].white_pix,
                    osw_gui[0].white_pix
                );
	    else
	        WidgetFrameButtonPixmap(
		    list->toplevel_buf,
		    False,
		    wattr.width, wattr.height,
		    widget_global.surface_highlight_pix,
		    widget_global.surface_shadow_pix
	        );

            OSWPutBufferToWindow(list->toplevel, list->toplevel_buf);
	}


        /* ********************************************************** */
        /* Redraw popup list. */
        if((amount == PULIST_DRAW_AMOUNT_COMPLETE) ||
           (amount == PULIST_DRAW_AMOUNT_PULIST)
        )
        {
	    /* Is popup list mapped? */
	    if(list->popup_map_state)
	    {
		/* Popup toplevel. */
                OSWGetWindowAttributes(list->popup_toplevel, &wattr);
                OSWClearPixmap(list->popup_toplevel_buf,
                    wattr.width, wattr.height,
		    (widget_global.force_mono) ?
                        osw_gui[0].black_pix :
                        widget_global.surface_editable_pix
                );

		/* Draw scroll arrows. */
		if(widget_global.force_mono)
		    OSWSetFgPix(osw_gui[0].white_pix);
		else
		    OSWSetFgPix(widget_global.scroll_cursor_pix);

                OSWDrawSolidArc(
		    list->popup_toplevel_buf,
		    (int)wattr.width - PULIST_SCROLL_CURSOR_WIDTH - 1,
		    1,
		    PULIST_SCROLL_CURSOR_WIDTH,
		    PULIST_SCROLL_CURSOR_HEIGHT,
		    0, PI
		);
                OSWDrawSolidArc(
                    list->popup_toplevel_buf,
		    (int)wattr.width - PULIST_SCROLL_CURSOR_WIDTH - 1,
		    (int)wattr.height - PULIST_SCROLL_CURSOR_HEIGHT - 1,
		    PULIST_SCROLL_CURSOR_WIDTH,
                    PULIST_SCROLL_CURSOR_HEIGHT,
                    PI, PI * 1.05
                );


		/* Draw frame. */
		if(widget_global.force_mono)
                    WidgetFrameButtonPixmap(
                        list->popup_toplevel_buf,
                        False,
                        wattr.width, wattr.height,
                        osw_gui[0].white_pix,
                        osw_gui[0].white_pix
                    );
		else
		    WidgetFrameButtonPixmap(
                        list->popup_toplevel_buf,
                        False,
                        wattr.width, wattr.height,
                        widget_global.surface_highlight_pix,
                        widget_global.surface_shadow_pix
                    );

                OSWPutBufferToWindow(
		    list->popup_toplevel,
		    list->popup_toplevel_buf
		);


		/* **************************************************** */
		/* Popup list. */

                OSWGetWindowAttributes(list->popup_list, &wattr);
                OSWClearPixmap(list->popup_list_buf,
                    wattr.width, wattr.height,
                    (widget_global.force_mono) ?
                        osw_gui[0].black_pix :
                        widget_global.surface_editable_pix
                );

		/* Calculate starting item pos. */
                y = 0 - (list->list_y_pos % PULIST_ITEM_HEIGHT);
		n = 0;	/* Items drawn. */

		for(i = MAX(list->list_y_pos / PULIST_ITEM_HEIGHT, 0);
                    i < list->total_items;
		    i++
		)
	        {
		    /* All visable items drawn? */
		    if(n > list->popup_list_vis_items) break;

		    if(list->item[i] == NULL) continue;
		    if(list->item[i]->name == NULL) continue;


		    /* Selected item? */
		    if(i == list->sel_item)
		    {
			if(widget_global.force_mono)
			    OSWSetFgPix(osw_gui[0].white_pix);
			else
			    OSWSetFgPix(widget_global.surface_selected_pix);
			OSWDrawSolidRectangle(
			    list->popup_list_buf,
			    0, y,
			    wattr.width, PULIST_ITEM_HEIGHT
			);

                        if(widget_global.force_mono)
                            OSWSetFgPix(osw_gui[0].black_pix);
                        else
			    OSWSetFgPix(widget_global.selected_text_pix);
		    }
		    /* Previous selected item? */
		    else if(i == list->prev_sel_item)
		    {
                        if(widget_global.force_mono)
                            OSWSetFgPix(osw_gui[0].white_pix);
                        else
			    OSWSetFgPix(widget_global.editable_text_pix);

			OSWDrawLine(list->popup_list_buf,
			    2, y + 4,
                            2, y
			);
                        OSWDrawLine(list->popup_list_buf,
                            2, y,
                            6, y
                        );

                        OSWDrawLine(list->popup_list_buf,
                            (int)wattr.width - 7, y,
                            (int)wattr.width - 3, y
                        );
                        OSWDrawLine(list->popup_list_buf,
                            (int)wattr.width - 3, y,
                            (int)wattr.width - 3, y + 4
                        );

                        OSWDrawLine(list->popup_list_buf,
                            (int)wattr.width - 3, y + PULIST_ITEM_HEIGHT - 5,
                            (int)wattr.width - 3, y + PULIST_ITEM_HEIGHT - 1
                        );
                        OSWDrawLine(list->popup_list_buf,
                            (int)wattr.width - 3, y + PULIST_ITEM_HEIGHT - 1,
                            (int)wattr.width - 7, y + PULIST_ITEM_HEIGHT - 1
                        );

                        OSWDrawLine(list->popup_list_buf,
                            6, y + PULIST_ITEM_HEIGHT - 1,
                            2, y + PULIST_ITEM_HEIGHT - 1
                        );
                        OSWDrawLine(list->popup_list_buf,
                            2, y + PULIST_ITEM_HEIGHT - 1,
                            2, y + PULIST_ITEM_HEIGHT - 5
                        );
		    }
		    else
		    {
                        if(widget_global.force_mono)
                            OSWSetFgPix(osw_gui[0].white_pix);
                        else
                            OSWSetFgPix(widget_global.editable_text_pix);
		    }
		    /* Is item disabled? */
		    if(list->item[i]->disabled)
		    {
			if(widget_global.force_mono)
                            OSWSetFgPix(osw_gui[0].white_pix);
                        else
			    OSWSetFgPix(widget_global.disabled_text_pix);
		    }
		    OSWDrawString(
			list->popup_list_buf,
			10,
			y + (PULIST_ITEM_HEIGHT / 2) + 5,
			list->item[i]->name
		    );



		    /* Increment position and items drawn. */
		    y += PULIST_ITEM_HEIGHT;
		    n++;
		}

		OSWPutBufferToWindow(
		    list->popup_list,
                    list->popup_list_buf
                );
	    }
	}

        OSWSetFont(prev_font);


	return(0);
}

/*
 *	Manage pulist.
 */
int PUListManage(popup_list_struct *list, event_t *event)
{
	int i, x, y, root_x, root_y;
	int events_handled = 0;
	win_attr_t wattr;


	if((list == NULL) ||
           (event == NULL)
	)
	    return(0);

	if(!list->map_state &&
           (event->type == MapNotify)
	)
	    return(0);


	switch(event->type)
	{
          /* ******************************************************** */
	  case KeyPress:
	    if(list->is_in_focus)
	    {
		if(event->xkey.keycode == osw_keycode.cursor_up)
		{
		    list->sel_item--;
		    events_handled++;
		}
	        else if(event->xkey.keycode == osw_keycode.cursor_down)
                {
		    list->sel_item++;
		    events_handled++;
                }

		if(list->sel_item >= list->total_items)
		    list->sel_item = list->total_items - 1;
		if(list->sel_item < 0)
		    list->sel_item = 0;

		list->prev_sel_item = list->sel_item;
	    }
	    break;

	  /* ******************************************************** */
	  case ButtonPress:
            if((event->xany.window == list->toplevel) ||
               (event->xany.window == list->popup_toplevel) ||
               (event->xany.window == list->popup_list)
	    )
		list->is_in_focus = 1;
	    else
		list->is_in_focus = 0;

	    /* Map popup list if selected label was pressed on. */
	    if(event->xany.window == list->toplevel)
            {
		PUListMapList(list);

		/* Set repeat records for list, but set op to none. */
                PUListRepeatRecordSet(
                    list,
                    widget_global.pulist_repeat_delay,
                    PULIST_OP_CODE_NONE
                );

		events_handled++;
		return(events_handled);
	    }
	    break;

	  /* ******************************************************** */
	  case ButtonRelease:
	    if(event->xany.window == list->popup_list)
	    {
		/* Unmap popup list. */
		PUListUnmapList(list);

		/* Clear repeat records. */
		PUListRepeatRecordClear();

                PUListDraw(list, PULIST_DRAW_AMOUNT_LABEL);

		/* Call select notify handler. */
		if(list->func_cb != NULL)
		    list->func_cb(list->client_data);

		events_handled++;
		return(events_handled);
	    }
	    else
	    {
		/* Unmap popup list as needed. */
		if(list->popup_map_state)
		    PUListUnmapList(list);

		/* Clear repeat records. */
		PUListRepeatRecordClear();

                return(events_handled);
	    }
	    break;

          /* ******************************************************** */
          case EnterNotify:
            if(event->xany.window == list->popup_list)
            {
                /* Clear repeat records. */
/*
                PUListRepeatRecordClear();
*/

		events_handled++;
		return(events_handled);
            }
            break;

          /* ******************************************************** */
          case LeaveNotify:
            if(event->xany.window == list->popup_list)
            {
		if(list->popup_map_state)
		{
		    OSWGetWindowAttributes(list->popup_list, &wattr);

		    OSWGetPointerCoords(
			list->popup_list,
			&root_x, &root_y,
			&x, &y
		    );

		    if(y < 0)
                        PUListRepeatRecordSet(
                            list,
                            widget_global.pulist_repeat_delay,
			    PULIST_OP_CODE_SCROLL_UP
		        );
		    else if(y > (int)wattr.height)
                        PUListRepeatRecordSet( 
                            list,
                            widget_global.pulist_repeat_delay,
                            PULIST_OP_CODE_SCROLL_DOWN
                        );
                    else
                        PUListRepeatRecordSet(
                            list,
                            widget_global.pulist_repeat_delay,
                            PULIST_OP_CODE_NONE
                        );

		}
	    }
	    break;

          /* ******************************************************** */
          case MotionNotify:
	    if(event->xany.window == list->popup_list)
	    {
		i = (list->list_y_pos + event->xmotion.y)
		   / PULIST_ITEM_HEIGHT;

		if((i < 0) || (i >= list->total_items))
		    list->sel_item = -1;
		else
		    list->sel_item = i;

                PUListDraw(list, PULIST_DRAW_AMOUNT_PULIST);

		events_handled++;
		return(events_handled);
	    }
	    break;

          /* ******************************************************** */
          case Expose:
	    if(event->xany.window == list->toplevel)
	    {
		PUListDraw(list, PULIST_DRAW_AMOUNT_LABEL);
                events_handled++;
	    }
	    else if(event->xany.window == list->popup_toplevel)
	    {
		PUListDraw(list, PULIST_DRAW_AMOUNT_PULIST);
                events_handled++;
	    }
	    else if(event->xany.window == list->popup_list)
	    {
                PUListDraw(list, PULIST_DRAW_AMOUNT_PULIST);
                events_handled++;
  	    }

	    if(events_handled > 0)
		return(events_handled);

	    break;
	}


	/* Map button. */
	if(events_handled == 0)
	{
	    events_handled += PBtnManage(&list->map_btn, event);
	    if((events_handled > 0) &&
               (event->type == ButtonPress)
	    )
	    {
		PUListMapList(list);
	    }
	}


	return(events_handled);
}

/*
 *	Map pulist.
 */
void PUListMap(popup_list_struct *list)
{
	if(list == NULL)
	    return;


	/* Unmap popup list first. */
	PUListUnmapList(list);


	/* Map by redrawing it. */
	list->map_state = 0;
	PUListDraw(list, PULIST_DRAW_AMOUNT_COMPLETE);


	return;
}

/*
 *	Unmap pulist.
 */
void PUListUnmap(popup_list_struct *list)
{
        if(list == NULL)
	    return;

           
        /* Unmap popup list first. */
        PUListUnmapList(list);


	/* Unmap. */
	PBtnUnmap(&list->map_btn);
	OSWUnmapWindow(list->toplevel);
	list->map_state = 0;
	list->visibility_state = VisibilityFullyObscured;
	list->is_in_focus = 0;


	/* Destroy buffers. */
	OSWDestroyPixmap(&list->toplevel_buf);


	return;
}


/*
 *	Maps the popup list (not the label).
 */
void PUListMapList(popup_list_struct *list)
{
	win_attr_t wattr;


	if(list == NULL)
	    return;


	/* Recreate buffers. */
	if(list->popup_toplevel_buf == 0)
	{
	    OSWGetWindowAttributes(list->popup_toplevel, &wattr);
	    if(OSWCreatePixmap(&list->popup_toplevel_buf,
		wattr.width, wattr.height
	    ))
		return;
	}
        if(list->popup_list_buf == 0)
        {
            OSWGetWindowAttributes(list->popup_list, &wattr);
            if(OSWCreatePixmap(&list->popup_list_buf,
                wattr.width, wattr.height
            ))
                return;
        }
 

	/* Already mapped? */
	if(list->popup_map_state)
	    return;


	/* Map list. */
	list->popup_map_state = 1;
	list->prev_sel_item = list->sel_item;
	OSWMapRaised(list->popup_toplevel);
	OSWMapWindow(list->popup_list);
	PUListDraw(list, PULIST_DRAW_AMOUNT_PULIST);


        /* Ungrab pointer from window of button press that mapped us. */
        OSWUngrabPointer();                             


	return;
}


/*
 *      Unaps the popup list (not the label).
 */
void PUListUnmapList(popup_list_struct *list)
{
	int i;


        if(list == NULL)
	    return;


	/* Unmap. */ 
        list->popup_map_state = 0;
	OSWUnmapWindow(list->popup_toplevel);


	/* Is the selected item disabled? */
	i = list->sel_item;
	if((i >= 0) && (i < list->total_items))
	{
	    if(list->item[i] != NULL)
	    {
		if(list->item[i]->disabled)
		    list->sel_item = list->prev_sel_item;
	    }
	}


	/* Destroy buffers. */
	OSWDestroyPixmap(&list->popup_list_buf);
	OSWDestroyPixmap(&list->popup_toplevel_buf);


        return;
}

/*
 *	Destroy pulist.
 */
void PUListDestroy(popup_list_struct *list)
{
	if(list == NULL)
	    return;


        /* Delete widget from regeristry. */
        WidgetRegDelete(list);


	/* Delete all items in list. */
	PUListDeleteAllItems(list);


	if(IDC())
	{
	   OSWDestroyPixmap(&list->popup_list_buf);
           OSWDestroyWindow(&list->popup_list);

           OSWDestroyPixmap(&list->popup_toplevel_buf);
	   OSWDestroyWindow(&list->popup_toplevel);

	   PBtnDestroy(&list->map_btn);

           OSWDestroyPixmap(&list->toplevel_buf);
	   OSWDestroyWindow(&list->toplevel);
	}


	list->map_state = 0;
	list->visibility_state = VisibilityFullyObscured;
	list->is_in_focus = 0;
	list->x = 0;
	list->y = 0;
	list->width = 0;
	list->height = 0;
	list->disabled = False;
	list->font = NULL;
        list->next = NULL;
        list->prev = NULL;

	list->popup_map_state = 0;
	list->popup_list_vis_items = 0;

	list->list_max_width = 0;
	list->list_max_height = 0;
	list->list_y_pos = 0;

	list->direction = PULIST_POPUP_CENTER;

        list->client_data = NULL;
	list->func_cb = NULL;


	/* Clear repeat record as needed. */
	if(list == pulist_repeat_record[0].list)
	    PUListRepeatRecordClear();


	return;
}

/*
 *	Set up repeat record for the given pulist.
 */
void PUListRepeatRecordSet(
	popup_list_struct *list,     
	long dsec, int op_code
)
{
        pulist_repeat_record[0].list = list;
        pulist_repeat_record[0].next_repeat = MilliTime() + MAX(dsec, 0);
	pulist_repeat_record[0].op_code = op_code;


	return;
}

/*
 *	Clear repeat record.
 */
void PUListRepeatRecordClear()
{
	pulist_repeat_record[0].list = NULL;
        pulist_repeat_record[0].next_repeat = 0;
        pulist_repeat_record[0].op_code = PULIST_OP_CODE_NONE;

	return;
}


/*
 *	Manage pulist repeats.
 */
int PUListManageRepeat(event_t *event)
{
	int x, y, root_x, root_y, y_max;
        int events_handled = 0;
        popup_list_struct *list;
	win_attr_t wattr;


	/*
         *   Note: The event information is not used and not needed here.
         */

        /* Is a list pointer set in the record? */
        if(pulist_repeat_record[0].list != NULL)
        {
            list = pulist_repeat_record[0].list;

	    /* Time due for repeat? */
            if(MilliTime() >= pulist_repeat_record[0].next_repeat)
            {
		/* Check pointer's position. */
                OSWGetWindowAttributes(list->popup_list, &wattr);
                
                OSWGetPointerCoords(
                    list->popup_list,
                    &root_x, &root_y,
                    &x, &y
                );
		if(y < 0)
		{
		    pulist_repeat_record[0].op_code =
			PULIST_OP_CODE_SCROLL_UP;
		}
		else if(y >= (int)wattr.height)
		{
		    pulist_repeat_record[0].op_code =
                        PULIST_OP_CODE_SCROLL_DOWN;
		}
		else
		{
		    pulist_repeat_record[0].op_code = PULIST_OP_CODE_NONE;
		}

                /* Handle scroll operation. */
                switch(pulist_repeat_record[0].op_code)
                {
		  case PULIST_OP_CODE_SCROLL_UP:
		    list->list_y_pos -= PULIST_ITEM_HEIGHT;
		    events_handled++;
		    break;

		  case PULIST_OP_CODE_SCROLL_DOWN:
                    list->list_y_pos += PULIST_ITEM_HEIGHT;
		    events_handled++;
		    break;

		  case PULIST_OP_CODE_NONE:
/* do not clear, only ButtonRelease should do that.
		    PUListRepeatRecordClear();
*/
		    return(events_handled);
		    break;

		  default:
		    break;
		}

		/* Sanitize position. */
		y_max = (int)list->list_max_height -
                    (list->popup_list_vis_items * PULIST_ITEM_HEIGHT);

		if(list->list_y_pos >= y_max)
		    list->list_y_pos = y_max;
		if(list->list_y_pos < 0)
		    list->list_y_pos = 0;



		/* Redraw */
		PUListDraw(list, PULIST_DRAW_AMOUNT_PULIST);


                /* Schedual next operation. */
                pulist_repeat_record[0].next_repeat =
                    MilliTime() + widget_global.pulist_repeat_interval;


                events_handled++;
	    }

	}


	return(events_handled);
}




