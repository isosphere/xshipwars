// widgets/wpstepper.cpp
/*
                     Widget: Page Stepper

	Functions:

	int PStepperIsPageAllocated(
		page_stepper_struct *ps,
		int page
	)
	int PStepperGetWidgetPointer(
		page_stepper_struct *ps,
		char *name,
		void **w_ptr,
		int *w_type
	)
        double PStepperGetWidgetValue(
                page_stepper_struct *ps,
                char *name
        )
	void *PStepperGetWidgetValuePtr(
                page_stepper_struct *ps,
                char *name
	)

	int PStepperPBCB(void *btn)
	int PStepperPrevPBCB(void *ptr)
        int PStepperNextPBCB(void *ptr)

	int PStepperInit(
		page_stepper_struct *ps,
		win_t parent,
	        int x, int y,
	        unsigned int width, unsigned int height,
		unsigned int total_pages,
		image_t *panel_img,
		void *client_data,
		int (*page_change_handler)(void *, int, int),
                int (*exit_handler)(void *),
                int (*finish_handler)(void *)
	)

	int PStepperDraw(
		page_stepper_struct *ps,
		int amount
	)

	int PStepperManage(
		page_stepper_struct *ps,
		event_t *event
	)

        void PStepperMap(page_stepper_struct *ps)
	void PStepperUnmap(page_stepper_struct *ps)

	void PStepperMapPage(page_stepper_struct *ps, int page)
	void PStepperUnmapPage(page_stepper_struct *ps, int page)

	void PStepperDestroy(
		page_stepper_struct *ps
	)


	int PStepperAllocatePage(
		page_stepper_struct *ps
	)
	void PStepperDestroyPage(
	        page_stepper_struct *ps,
	        int page
	)

	int PStepperAllocateLabel(
                page_stepper_struct *ps,
                int page,    
		char *text,
                font_t *font,
                pixel_t pix,
                int x, int y
	)

	int PStepperAllocateWidget(
	        page_stepper_struct *ps,
	        int page,
	        char *name,
	        int w_type,
	       	int x, int y,
	        unsigned int width, unsigned int height,
	        int argc, char *argv[]
	)
	void PStepperDestroyWidget(page_widget_struct *pw)


	---

	Commonly used in install programs and other step by
	step windows.

 */

#include "../include/string.h"
#include "../include/strexp.h"
#include "../include/widget.h"

#ifndef MAX
#define MIN(a,b)        (((a) < (b)) ? (a) : (b))
#define MAX(a,b)	(((a) > (b)) ? (a) : (b))
#endif
/* #define MIN(a,b)	((a) < (b)) ? (a) : (b) */
/* #define MAX(a,b)	((a) > (b)) ? (a) : (b) */


/*
 *	Panel image positions:
 */
#define PS_PANEL_IMG_X	10
#define PS_PANEL_IMG_Y	10


/*
 *	Draw amount codes:
 */
#define PS_DRAW_AMOUNT_COMPLETE		0


/*
 *	Default sizes:
 */
#define PS_DEF_WIDTH	360
#define PS_DEF_HEIGHT	400

/*
 *	Button size:
 */
#define PS_BTN_WIDTH		90
#define PS_BTN_HEIGHT           28


/*
 *	Checks if page is allocated on ps.
 */
int PStepperIsPageAllocated(page_stepper_struct *ps, int page)
{
	if(ps == NULL)
	    return(0);
	else if((ps->page == NULL) ||
                (page < 0) ||
                (page >= ps->total_pages)
               )
	    return(0);
	else if(ps->page[page] == NULL)
	    return(0);
	else
	    return(1);
}


/*
 *	Attempts to match a widget on the page stepper ps.
 *
 *	name is the search string (exact matches only,
 *	case sensitive).
 *
 *	w_ptr is the address to a void pointer which
 *	will be set to the pointed structure or NULL if no match
 *	can be made.
 *
 *	w_type is the address to an in which will be set to
 *	the type of widget it is or -1 if no match can be made.
 *
 *	returns -1 on no match or error, and 0 on success.
 */
int PStepperGetWidgetPointer(
	page_stepper_struct *ps,
	char *name,
	void **w_ptr,
	int *w_type
)
{
	int i, n;
        page_stepper_page_struct **page_ptr;
	page_widget_struct **w_check_ptr;


	if((ps == NULL) ||
           (name == NULL)
	)
	{
	    if(w_ptr != NULL)
		*w_ptr = NULL;

	    if(w_type != NULL)
		*w_type = -1;

	    return(-1);
	}


	/* Go through each page. */
	for(i = 0, page_ptr = ps->page;
            i < ps->total_pages;
            i++, page_ptr++
	)
	{
	    if(*page_ptr == NULL)
		continue;

	    /* Go through each widget on current page. */
	    for(n = 0, w_check_ptr = (*page_ptr)->widget;
                n < (*page_ptr)->total_widgets;
                n++, w_check_ptr++
	    )
	    {
		if(*w_check_ptr == NULL)
		    continue;
		if((*w_check_ptr)->name == NULL)
		    continue;
		if(strcmp((*w_check_ptr)->name, name))
		    continue;

		/* Got match! */
                if(w_ptr != NULL)
		    *w_ptr = (*w_check_ptr)->w;

		if(w_type != NULL)
		    *w_type = (*w_check_ptr)->w_type;


		return(0);
	    }
	}


	return(-1);
}


/*
 *      Returns the most pertainant value on widget
 *      on ps matching the ID name.  If no match is made or
 *      there is an error, 0 is returned.
 *
 *	There is no accurate way to distingush an error.
 */
double PStepperGetWidgetValue(
        page_stepper_struct *ps,
        char *name
)
{
        void *ptr;
        int w_type;
        int status;

        push_button_struct *pbtn;
        list_window_struct *lw;
        progress_bar_struct *pbar;
        prompt_window_struct *prompt;
        popup_list_struct *pulist;
        scale_bar_struct *scalebar;  
        toggle_button_array_struct *tba;
        toggle_button_struct *tb;


        if((ps == NULL) ||
           (name == NULL)
        )
            return(0);

        status = PStepperGetWidgetPointer(
            ps,
            name,
            &ptr, 
            &w_type
        );
        if((status) || (ptr == NULL))
            return(0);

        switch(w_type)
        {
          case WTYPE_CODE_PUSHBUTTON:
            pbtn = (push_button_struct *)ptr;
            return((double)pbtn->state);
            break;
        
          case WTYPE_CODE_LIST:   
            lw = (list_window_struct *)ptr;
            return((double)lw->entry_pos);
            break;
        
          case WTYPE_CODE_PROGRESSBAR:
            pbar = (progress_bar_struct *)ptr;
            return((double)pbar->current);
            break;

          case WTYPE_CODE_PROMPT:
            prompt = (prompt_window_struct *)ptr;
            return((double)0);
            break;

          case WTYPE_CODE_PULIST:
            pulist = (popup_list_struct *)ptr;
            return((double)pulist->sel_item);
            break;
  
         case WTYPE_CODE_SCALEBAR:
            scalebar = (scale_bar_struct *)ptr;
            return((double)scalebar->pos);
            break;

          case WTYPE_CODE_TOGGLEARRAY:
            tba = (toggle_button_array_struct *)ptr;
            return((double)tba->armed_tb);
            break;

          case WTYPE_CODE_TOGGLEBTN:
            tb = (toggle_button_struct *)ptr;
            return((double)tb->state);
            break;

          default:
            fprintf(stderr,
         "PStepperGetWidgetValue(): Unsupported widget type code %i.\n",
                w_type
            );
            break;
        }


        return(0);
}


/*
 *	Returns the pointer to the most pertainant value on widget
 *	on ps matching the ID name.  If no match is made or
 *	there is an error, NULL is returned.
 */
void *PStepperGetWidgetValuePtr(
	page_stepper_struct *ps,
	char *name 
)
{
	void *ptr;
	int w_type;
	int status;

	push_button_struct *pbtn;
	list_window_struct *lw;
	progress_bar_struct *pbar;
	prompt_window_struct *prompt;
	popup_list_struct *pulist;
	scale_bar_struct *scalebar;
	toggle_button_array_struct *tba;
	toggle_button_struct *tb;


	if((ps == NULL) ||
           (name == NULL)
	)
	    return(NULL);

	status = PStepperGetWidgetPointer(
	    ps,
            name,
	    &ptr,
            &w_type
	);
	if((status) || (ptr == NULL))
	    return(NULL);

	switch(w_type)
	{
          case WTYPE_CODE_PUSHBUTTON:
            pbtn = (push_button_struct *)ptr;
	    return((void *)&pbtn->state);
            break;

          case WTYPE_CODE_LIST:
            lw = (list_window_struct *)ptr;
	    return((void *)&lw->entry_pos);
            break;

          case WTYPE_CODE_PROGRESSBAR:
            pbar = (progress_bar_struct *)ptr;
            return((void *)&pbar->current);
            break;

          case WTYPE_CODE_PROMPT:
            prompt = (prompt_window_struct *)ptr;
	    return((void *)prompt->buf);
            break;

          case WTYPE_CODE_PULIST:
            pulist = (popup_list_struct *)ptr;
            return((void *)&pulist->sel_item);
            break;

         case WTYPE_CODE_SCALEBAR:
            scalebar = (scale_bar_struct *)ptr;
	    return((void *)&scalebar->pos);
            break;

          case WTYPE_CODE_TOGGLEARRAY:
            tba = (toggle_button_array_struct *)ptr;
            return((void *)&tba->armed_tb);
            break;   

          case WTYPE_CODE_TOGGLEBTN:
            tb = (toggle_button_struct *)ptr;
            return((void *)&tb->state);
            break;

          default:
            fprintf(stderr,
         "PStepperGetWidgetValue(): Unsupported widget type code %i.\n",
                w_type
            );
            break;
	}


	return(NULL);
}


/*
 *	Push button callback function.
 */
int PStepperPBCB(void *btn)
{
	if(btn == NULL)
	    return(-1);



	return(0);
}


/*
 *	Previous page callback.
 */
int PStepperPrevPBCB(void *ptr)
{
	int prev_page;
        page_stepper_struct *ps;


	if(ptr == NULL)
	    return(-1);

        ps = (page_stepper_struct *)ptr;


	prev_page = ps->cur_page;

	ps->cur_page--;
	if(ps->cur_page < 0)
	{
	    ps->cur_page = 0;

	    /* Call exit callback handler. */
	    if(ps->exit_handler != NULL)
	    {
		ps->exit_handler(ps->client_data);
	    }

	    PStepperUnmap(ps);
	    return(0);
	}

        /* Change prev and next button label messages. */
        PBtnChangeLabel(
            &ps->prev_btn,
            PS_BTN_WIDTH, PS_BTN_HEIGHT,
            (ps->cur_page == 0) ? "Exit" : "<< Back",
            PBTN_TALIGN_CENTER,
            NULL
        );
        PBtnDraw(&ps->prev_btn);
        PBtnChangeLabel(
            &ps->next_btn,
            PS_BTN_WIDTH, PS_BTN_HEIGHT,
            (ps->cur_page == (ps->total_pages - 1)) ? "Finish" : "Next >>",
            PBTN_TALIGN_CENTER,
            NULL
        );
        PBtnDraw(&ps->next_btn);


	PStepperUnmapPage(ps, prev_page);
	PStepperMapPage(ps, ps->cur_page);

	PStepperDraw(ps, PS_DRAW_AMOUNT_COMPLETE);

	/* Run callback function. */
	if(ps->page_change_handler != NULL)
	{
	    ps->page_change_handler(ps->client_data, prev_page, ps->cur_page);
	}


	return(0);
}


/*
 *	Next page callback.
 */
int PStepperNextPBCB(void *ptr)
{
	int prev_page;
	page_stepper_struct *ps;


        if(ptr == NULL)
            return(-1);

	ps = (page_stepper_struct *)ptr;


        prev_page = ps->cur_page;

        ps->cur_page++;
        if(ps->cur_page >= ps->total_pages)
	{
            ps->cur_page = ps->total_pages - 1;

	    /* Call finish callback handler. */
            if(ps->finish_handler != NULL)
            {
                ps->finish_handler(ps->client_data);
            }

            PStepperUnmap(ps);

	    return(0);
	}

	/* Change prev and next button label messages. */
        PBtnChangeLabel(
            &ps->prev_btn,
            PS_BTN_WIDTH, PS_BTN_HEIGHT,
            (ps->cur_page == 0) ? "Exit" : "<< Back",
            PBTN_TALIGN_CENTER,
            NULL
        );
	PBtnDraw(&ps->prev_btn);
        PBtnChangeLabel(
            &ps->next_btn,
            PS_BTN_WIDTH, PS_BTN_HEIGHT,
            (ps->cur_page == (ps->total_pages - 1)) ? "Finish" : "Next >>",
            PBTN_TALIGN_CENTER,
            NULL
        );
        PBtnDraw(&ps->next_btn);

        PStepperUnmapPage(ps, prev_page);
        PStepperMapPage(ps, ps->cur_page);

        PStepperDraw(ps, PS_DRAW_AMOUNT_COMPLETE);


        /* Run callback function. */
        if(ps->page_change_handler != NULL)
        {
            ps->page_change_handler(ps->client_data, prev_page, ps->cur_page);
        }


	return(0);
}



/*
 *	Initializes a new page stepper widget.
 */
int PStepperInit(
	page_stepper_struct *ps,
        win_t parent,
	int x, int y,
	unsigned int width, unsigned int height,
        unsigned int total_pages,
	image_t *panel_img,
        void *client_data,
        int (*page_change_handler)(void *, int, int),
	int (*exit_handler)(void *),
	int (*finish_handler)(void *)
)
{
	int i;
	char stringa[256];


	if((ps == NULL) ||
           (parent == 0)
	)
	    return(-1);


	if(width == 0)
	    width = PS_DEF_WIDTH;
        if(height == 0)
            height = PS_DEF_HEIGHT;

	ps->map_state = 0;
	ps->is_in_focus = 0;
        ps->visibility_state = VisibilityFullyObscured;
        ps->x = x;
        ps->y = y;
        ps->width = width;
        ps->height = height;
        ps->disabled = False;
	ps->font = OSWQueryCurrentFont();
	ps->next = NULL;
	ps->prev = NULL;

	ps->parent = parent;


	/* Create toplevel. */
	if(
	    OSWCreateWindow(
		&ps->toplevel,
		parent,
		ps->x, ps->y,
		ps->width, ps->height
	    )
	)
	    return(-1);
	if(parent == osw_gui[0].root_win)
	{
            OSWSetWindowWMProperties(
                ps->toplevel,
                "Untitled",			/* Title. */
                "Untitled",			/* Icon title. */
                widget_global.std_icon_pm,	/* Icon. */
		True,			/* Let WM set coordinates? */
                /* Coordinates. */
                ps->x, ps->y,
                /* Min width and height. */
                ps->width, ps->height,
		ps->width, ps->height,
		WindowFrameStyleFixed,
		NULL, 0
            );

	    OSWSetWindowInput(ps->toplevel,
		KeyPressMask | KeyReleaseMask |
	        ButtonPressMask | ButtonReleaseMask |
                ExposureMask | FocusChangeMask | VisibilityChangeMask
	    );
	}
	else
	{
            OSWSetWindowInput(ps->toplevel,
                ExposureMask
            );
	}


        /* ********************************************************* */

	/*   Allocate pages, pointers and total will be set by.
         *   call to PStepperAllocatePage().
	 */
	for(i = 0; i < (int)total_pages; i++)
	    PStepperAllocatePage(ps);


	/* Set current page. */
	if(ps->total_pages <= 0)
	    ps->cur_page = -1;
	else
	    ps->cur_page = 0;


        /* Previous button. */
        if(
            PBtnInit(
                &ps->prev_btn,
                ps->toplevel,
                static_cast<int>(((int)ps->width * 0.65) - PS_BTN_WIDTH - 5),
                (int)ps->height - PS_BTN_HEIGHT - 10,
                PS_BTN_WIDTH, PS_BTN_HEIGHT,
                (ps->cur_page == 0) ? "Exit" : "<< Back",
                PBTN_TALIGN_CENTER,
                NULL,
                (void *)ps,
                PStepperPrevPBCB
            )
        )
            return(-1);
        stringa[0] = 0x1b;	/* Escape. */
        stringa[1] = '\0';
/*
        PBtnSetHotKeys(   
            &ps->prev_btn,
            stringa
        );
 */

        /* Next button. */
        if(
            PBtnInit(
                &ps->next_btn,
                ps->toplevel,
                static_cast<int>(((int)ps->width * 0.65) + 5),
                (int)ps->height - PS_BTN_HEIGHT - 10,
                PS_BTN_WIDTH, PS_BTN_HEIGHT,
                (ps->cur_page == (ps->total_pages - 1)) ? "Finish" :
                    "Next >>",
                PBTN_TALIGN_CENTER,
                NULL,
                (void *)ps,
                PStepperNextPBCB
            )
        )       
            return(-1);
	stringa[0] = '\n';
	stringa[1] = '\0';
/*
        PBtnSetHotKeys(
            &ps->next_btn,
	    stringa
        );
 */


	/* ********************************************************* */

	/* Set panel image. */
	ps->panel_img = panel_img;

	/* Set client data and page change callback function. */
	ps->client_data = client_data;
	ps->page_change_handler = page_change_handler;
	ps->exit_handler = exit_handler;
	ps->finish_handler = finish_handler;


        /* Add this widget to the regeristry. */
        WidgetRegAdd((void *)ps, WTYPE_CODE_PAGESTEPPER);


	return(0);
}


/*
 *	Redraws page stepper.
 */
int PStepperDraw(
        page_stepper_struct *ps,
        int amount
)
{
	int n, j;
	int x, y, x_pos, y_pos;
	page_stepper_page_struct **page_ptr;
	page_stepper_label_struct **label_ptr;
	win_attr_t wattr;
	font_t *prev_font;


	if(ps == NULL)
	    return(-1);


	/* Map as needed. */
	if(!ps->map_state)
	{
	    OSWMapRaised(ps->toplevel);

            PBtnMap(&ps->prev_btn);
	    PBtnMap(&ps->next_btn);

	    ps->map_state = 1;
	    ps->visibility_state = VisibilityUnobscured;


	    /* Map all widgets on current page. */
	    PStepperMapPage(ps, ps->cur_page);
	}

	/* Record previous font and set new font. */
	prev_font = OSWQueryCurrentFont();


	/* Recreate buffers as needed. */
	if(ps->toplevel_buf == 0)
	{
	    OSWGetWindowAttributes(ps->toplevel, &wattr);
	    if(OSWCreatePixmap(&ps->toplevel_buf,
		wattr.width, wattr.height)
	    )
		return(-1);
	}


	/* Draw background. */
        OSWGetWindowAttributes(ps->toplevel, &wattr);
	if(widget_global.force_mono)
	{
	    OSWClearPixmap(
		ps->toplevel_buf,
                wattr.width, wattr.height,
		osw_gui[0].black_pix
	    );
            OSWSetFgPix(osw_gui[0].white_pix);
            OSWDrawLine(ps->toplevel_buf,
                0, (int)wattr.height - 21 - PS_BTN_HEIGHT,
                wattr.width, (int)wattr.height - 21 - PS_BTN_HEIGHT
            );
	}
	else
	{
            WidgetPutImageTile(
                ps->toplevel_buf,
                widget_global.std_bkg_img,
                wattr.width, wattr.height
            );

	    /* Horizontal rule. */
	    OSWSetFgPix(widget_global.surface_shadow_pix);
	    OSWDrawLine(ps->toplevel_buf,
		0, (int)wattr.height - 21 - PS_BTN_HEIGHT,
		wattr.width, (int)wattr.height - 21 - PS_BTN_HEIGHT
	    );
            OSWSetFgPix(widget_global.surface_highlight_pix);
            OSWDrawLine(ps->toplevel_buf,
                0, (int)wattr.height - 20 - PS_BTN_HEIGHT,
                wattr.width, (int)wattr.height - 20 - PS_BTN_HEIGHT
            );

	    /* Panel image. */
	    if(ps->panel_img != NULL)
	    {
		OSWPutImageToDrawablePos(
		    ps->panel_img,
		    ps->toplevel_buf,
		    PS_PANEL_IMG_X,
		    PS_PANEL_IMG_Y
		);
	    }
	}

	/* Draw labels. */
	if(PStepperIsPageAllocated(ps, ps->cur_page))
	{
	    page_ptr = &(ps->page[ps->cur_page]);

	    for(n = 0, label_ptr = (*page_ptr)->label;
                n < (*page_ptr)->total_labels;
                n++, label_ptr++
	    )
	    {
		if(*label_ptr == NULL)
		    continue;

		x = (*label_ptr)->x;
		y = (*label_ptr)->y;
		OSWSetFont((*label_ptr)->font);
		OSWSetFgPix((*label_ptr)->pix);

		x_pos = x;
		y_pos = y;

		for(j = 0; j < (*label_ptr)->total_text_lines; j++)
		{
		    if((*label_ptr)->text[j] == NULL)
			continue;

                    OSWDrawString(
			ps->toplevel_buf,
                        x_pos,
			y_pos,
			(*label_ptr)->text[j]
		    );

		    y_pos += 16;
		}
	    }
	}




	OSWPutBufferToWindow(ps->toplevel, ps->toplevel_buf);

	OSWSetFont(prev_font);


	return(0);
}



/*
 *	Manage page stepper.
 */
int PStepperManage(
	page_stepper_struct *ps,
	event_t *event
)
{
	int i, n;
	int events_handled = 0;
	page_stepper_page_struct **page_ptr;
	page_widget_struct **w_ptr;


	if((ps == NULL) ||
           (event == NULL)
	)
	    return(events_handled);

	if(!ps->map_state &&
           (event->type != MapNotify)
	)
	    return(events_handled);


	switch(event->type)
	{
          /* ********************************************************* */
	  case Expose:
	    if(event->xany.window == ps->toplevel)
	    {
		events_handled++;
	    }
	    break;

          /* ********************************************************* */
          case FocusIn:
            if(event->xany.window == ps->toplevel)
            {
                ps->is_in_focus = 1;
                events_handled++;
            }
            else
            {
                ps->is_in_focus = 0;
            }
            return(events_handled);
            break;

          /* ********************************************************* */
          case FocusOut:
            if(event->xany.window == ps->toplevel)
            {
                ps->is_in_focus = 0;
                events_handled++;
            }
            return(events_handled);
            break;

          /* ********************************************************* */
          case UnmapNotify:
            if(event->xany.window == ps->toplevel)
            {
                PStepperUnmap(ps);

                events_handled++;
		return(events_handled);
            }
	    break;

          /* ********************************************************* */
          case MapNotify:
            if(event->xany.window == ps->toplevel)
            {
		if(!ps->map_state)
		    PStepperMap(ps);

                events_handled++;
                return(events_handled);
            }
            break;

          /* ********************************************************* */
          case VisibilityNotify:
            if(event->xany.window == ps->toplevel)
            {
                ps->visibility_state = event->xvisibility.state;
                events_handled++;  

                /* No need to continue, just return. */
                return(events_handled);
            }
            break;

          /* ********************************************************* */
          case ClientMessage:
            if(OSWIsEventDestroyWindow(ps->toplevel, event))
            {
		if(ps->exit_handler != NULL)
		    ps->exit_handler(ps->client_data);

		PStepperUnmap(ps);

		events_handled++;
		return(events_handled);
            }
	    break;
	}


	if(events_handled > 0)
	{
	    PStepperDraw(ps, PS_DRAW_AMOUNT_COMPLETE);
	}


	/* Manage buttons. */
	if(events_handled == 0)
	{
            events_handled += PBtnManage(&ps->prev_btn, event);
	}

        if(events_handled == 0)
        {
            events_handled += PBtnManage(&ps->next_btn, event); 
        }


	/* ****************************************************** */
	/* Go through each page, managing widgets. */
	for(i = 0, page_ptr = ps->page;
	    i < ps->total_pages;
	    i++, page_ptr++
	)
	{
	    if(*page_ptr == NULL)
		continue;

	    /* Go through each widget on current page. */
	    for(n = 0, w_ptr = (*page_ptr)->widget;
                n < (*page_ptr)->total_widgets;
                n++, w_ptr++
	    )
	    {
		if(*w_ptr == NULL)
		    continue;

		/* Manage by what kind of widget it is. */
                switch((*w_ptr)->w_type)
		{
		  case WTYPE_CODE_PUSHBUTTON:
                    events_handled += PBtnManage(
			(push_button_struct *)(*w_ptr)->w,
			event
		    );
		    break;

                  case WTYPE_CODE_LIST:
                    events_handled += ListWinManage(
                        (list_window_struct *)(*w_ptr)->w,
                        event
                    );
                    break;

                  case WTYPE_CODE_PROGRESSBAR:
                    events_handled += PBarManage(
                        (progress_bar_struct *)(*w_ptr)->w,
                        event
                    );
                    break;

                  case WTYPE_CODE_PROMPT:
                    events_handled += PromptManage(
                        (prompt_window_struct *)(*w_ptr)->w,
                        event
                    );
                    break;

                  case WTYPE_CODE_PULIST:
                    events_handled += PUListManage(
                        (popup_list_struct *)(*w_ptr)->w,
                        event
                    );
                    break;

                  case WTYPE_CODE_SCALEBAR:
                    events_handled += ScaleBarManage(
                        (scale_bar_struct *)(*w_ptr)->w,
                        event
                    );
                    break;

                  case WTYPE_CODE_TOGGLEARRAY:
                    events_handled += TgBtnArrayManage(
                        (toggle_button_array_struct *)(*w_ptr)->w,
                        event
                    );
                    break;

                  case WTYPE_CODE_TOGGLEBTN:
                    events_handled += TgBtnManage(
                        (toggle_button_struct *)(*w_ptr)->w,
                        event
                    );
                    break;


		  default:
		    fprintf(stderr,
			"PStepperManage(): Unsupported widget type code %i.\n",
			(*w_ptr)->w_type
		    );
		    break;
		}
	    }
	}



	return(events_handled);
}


/*
 *	Map page stepper.
 */
void PStepperMap(page_stepper_struct *ps)
{
	if(ps == NULL)
	    return;


	ps->map_state = 0;
	PStepperDraw(ps, PS_DRAW_AMOUNT_COMPLETE);

	ps->is_in_focus = 1;


	return;
}


/*
 *      Unmap page stepper.
 */
void PStepperUnmap(page_stepper_struct *ps)
{
        if(ps == NULL)
            return;


	PBtnUnmap(&ps->prev_btn);
        PBtnUnmap(&ps->next_btn);
  
        PStepperUnmapPage(ps, ps->cur_page);
	OSWUnmapWindow(ps->toplevel);


        ps->is_in_focus = 0;
	ps->visibility_state = VisibilityFullyObscured;
        ps->map_state = 0;


        
        return;
}


/*
 *	Maps page on ps.
 */
void PStepperMapPage(page_stepper_struct *ps, int page)
{
	int i;
        page_stepper_page_struct *page_ptr;
        page_widget_struct **w_ptr;


	if(ps == NULL)
	    return;

	if(!PStepperIsPageAllocated(ps, page))
	    return;

	page_ptr = ps->page[page];


	/* Map each of page's widgets. */
	for(i = 0, w_ptr = page_ptr->widget;
            i < page_ptr->total_widgets;
            i++, w_ptr++
	)
	{
	    if(*w_ptr == NULL)
		continue;

	    switch((*w_ptr)->w_type)
	    {
              case WTYPE_CODE_PUSHBUTTON:
                PBtnMap((push_button_struct *)(*w_ptr)->w);
                break;

              case WTYPE_CODE_LIST:
                ListWinMap((list_window_struct *)(*w_ptr)->w);
                break;

              case WTYPE_CODE_PROGRESSBAR:
                PBarMap((progress_bar_struct *)(*w_ptr)->w);
                break;

              case WTYPE_CODE_PROMPT:
                PromptMap((prompt_window_struct *)(*w_ptr)->w);
                break;

              case WTYPE_CODE_PULIST:             
                PUListMap((popup_list_struct *)(*w_ptr)->w);
                break;

              case WTYPE_CODE_SCALEBAR:
                ScaleBarMap((scale_bar_struct *)(*w_ptr)->w);
                break;

              case WTYPE_CODE_TOGGLEARRAY:
                TgBtnArrayMap((toggle_button_array_struct *)(*w_ptr)->w);
                break;

              case WTYPE_CODE_TOGGLEBTN:
                TgBtnMap((toggle_button_struct *)(*w_ptr)->w);
                break;

	      default:
		fprintf(stderr,
		    "PStepperMapPage(): Unsupported widget type code %i.\n",
                    (*w_ptr)->w_type
		);
		break;
	    }
	}


	/* Mark page as mapped. */
	page_ptr->map_state = 1;


	return;
}


/*
 *	Unmaps page on ps.
 */
void PStepperUnmapPage(page_stepper_struct *ps, int page)
{
        int i;
        page_stepper_page_struct *page_ptr;
        page_widget_struct **w_ptr;


        if(ps == NULL)
            return;

        if(!PStepperIsPageAllocated(ps, page))
            return;

        page_ptr = ps->page[page];


        /* Map each of page's widgets. */
        for(i = 0, w_ptr = page_ptr->widget;
            i < page_ptr->total_widgets;
            i++, w_ptr++
        )
        {
            if(*w_ptr == NULL)
                continue;

            switch((*w_ptr)->w_type)
            {
              case WTYPE_CODE_PUSHBUTTON:
                PBtnUnmap((push_button_struct *)(*w_ptr)->w);        
                break;
                
              case WTYPE_CODE_LIST:
                ListWinUnmap((list_window_struct *)(*w_ptr)->w);        
                break;

              case WTYPE_CODE_PROGRESSBAR:
                PBarUnmap((progress_bar_struct *)(*w_ptr)->w);
                break;
                
              case WTYPE_CODE_PROMPT:
                PromptUnmap((prompt_window_struct *)(*w_ptr)->w);            
                break;

              case WTYPE_CODE_PULIST:
                PUListUnmap((popup_list_struct *)(*w_ptr)->w);
                break;

              case WTYPE_CODE_SCALEBAR:
                ScaleBarUnmap((scale_bar_struct *)(*w_ptr)->w);
                break;

              case WTYPE_CODE_TOGGLEARRAY:
                TgBtnArrayUnmap((toggle_button_array_struct *)(*w_ptr)->w);
                break;

              case WTYPE_CODE_TOGGLEBTN:
                TgBtnUnmap((toggle_button_struct *)(*w_ptr)->w); 
                break;


              default:
                fprintf(stderr,
                    "PStepperMapPage(): Unsupported widget type code %i.\n",
                    (*w_ptr)->w_type
                );
                break;
            }
        }


        /* Mark page as Unmapped. */
        page_ptr->map_state = 0;


        return;             
}        


/*
 *	Deallocates page stepper and all it's allocated resources.
 */
void PStepperDestroy(page_stepper_struct *ps)
{
	int i;


	if(ps == NULL)
	    return;


	/* Deallocate all pages. */
	for(i = 0; i < ps->total_pages; i++)
	{
	    PStepperDestroyPage(ps, i);

	    free(ps->page[i]);
	    ps->page[i] = NULL;
	}

	free(ps->page);
	ps->page = NULL;

	ps->total_pages = 0;
	ps->cur_page = -1;


        /* Delete widget from regeristry. */
        WidgetRegDelete(ps);


	if(IDC())
	{
	    PBtnDestroy(&ps->next_btn);
	    PBtnDestroy(&ps->prev_btn);

            OSWDestroyWindow(&ps->toplevel);
	    OSWDestroyPixmap(&ps->toplevel_buf);
	}


	ps->parent = 0;

	ps->panel_img = NULL;		/* Shared, do not free. */

	ps->client_data = NULL;
	ps->page_change_handler = NULL;
	ps->exit_handler = NULL;
	ps->finish_handler = NULL;

        ps->next = NULL;
        ps->prev = NULL;


	return;
}



/*
 *	Allocates a new page on page stepper, appends it
 *	to the end.  Returns new page number or -1 on error.
 */
int PStepperAllocatePage(
	page_stepper_struct *ps
)
{
	int i;


	if(ps == NULL)
	    return(-1);


	/* Sanitize total. */
	if(ps->total_pages < 0)
	    ps->total_pages = 0;


	/* Allocate more pointers. */
	i = ps->total_pages;
	ps->total_pages++;

	ps->page = (page_stepper_page_struct **)realloc(
	    ps->page,
	    ps->total_pages * sizeof(page_stepper_page_struct *)
	);
	if(ps->page == NULL)
	{
	    ps->total_pages = 0;
	    return(-1);
	}

	/* Allocate new page. */
	ps->page[i] = (page_stepper_page_struct *)calloc(
	    1,
	    sizeof(page_stepper_page_struct)
	);
	if(ps->page[i] == NULL)
	{
	    return(-1);
	}


	return(i);
}


/*
 *	Deallocates all widgets on page, does not deallocate
 *	page itself.
 */
void PStepperDestroyPage(
	page_stepper_struct *ps,
	int page
)
{
	int i;
	page_stepper_page_struct *page_ptr;


        if(ps == NULL)
            return;

	if(!PStepperIsPageAllocated(ps, page))
	    return;


	/* Get pointer to page. */
	page_ptr = ps->page[page];


	/* Destroy all widgets on this page. */
	for(i = 0; i < page_ptr->total_widgets; i++)
	{
            if(page_ptr->widget[i] == NULL)
		continue;

	    PStepperDestroyWidget(
		page_ptr->widget[i]
	    );

	    free(page_ptr->widget[i]);
	    page_ptr->widget[i] = NULL;
	}

	free(page_ptr->widget);
	page_ptr->widget = NULL;

	page_ptr->total_widgets = 0;


	/* Destroy all labels on this page. */
	for(i = 0; i < page_ptr->total_labels; i++)
        {
	    if(page_ptr->label[i] == NULL)
		continue;

	    StringFreeArray(
		page_ptr->label[i]->text,
		page_ptr->label[i]->total_text_lines
	    );
	    page_ptr->label[i]->text = NULL;
	    page_ptr->label[i]->total_text_lines = 0;


	    /* Do not free image, shared. */
	    page_ptr->label[i]->image = NULL;

	    free(page_ptr->label[i]);
	    page_ptr->label[i] = NULL;
	}
	free(page_ptr->label);
	page_ptr->label = NULL;

	page_ptr->total_labels = 0;


	return;
}


/*
 *	Allocates a label on page of page stepper.
 */
int PStepperAllocateLabel(
	page_stepper_struct *ps,
	int page,
	char *text,
        image_t *image,
	font_t *font,
	pixel_t pix, 
	int x, int y
)
{
	int i;
	page_stepper_label_struct *label_ptr;
	page_stepper_page_struct *page_ptr;
	int strc;
	char **strv;


        if(ps == NULL)
            return(-1);

        if(!PStepperIsPageAllocated(ps, page))
            return(-1);

        page_ptr = ps->page[page];


        /* Allocate more label pointers as needed. */
        if(page_ptr->total_labels < 0)
            page_ptr->total_labels = 0;
        i = page_ptr->total_labels;
        page_ptr->total_labels++;

        page_ptr->label = (page_stepper_label_struct **)realloc(
            page_ptr->label,
            page_ptr->total_labels * sizeof(page_stepper_label_struct *)
        );
        if(page_ptr->label == NULL)
        {
            page_ptr->total_labels = 0;
            return(-1);
        }

        /* Allocate new label structure. */
        page_ptr->label[i] = (page_stepper_label_struct *)calloc(
            1,
            sizeof(page_stepper_label_struct)
        );
        if(page_ptr->label[i] == NULL)
        {
            page_ptr->total_labels = i;
            return(-1);
        }

        /* Get pointer to label structure. */
        label_ptr = page_ptr->label[i];


        /* ******************************************************* */
	/* Set values. */

	label_ptr->font = font;

	label_ptr->x = x;
	label_ptr->y = y;

	label_ptr->pix = pix;

	label_ptr->image = image;

	/* Set text. */
	if(text != NULL)
	{
	    strv = strchrexp(text, '\n', &strc);

	    label_ptr->text = strv;
	    label_ptr->total_text_lines = strc;
	}


	return(0);
}


/*
 *	Allocates a widget of type w_type onto a page on the page stepper.
 *
 *	name should not be NULL, it will be used to identify the widget.
 */
int PStepperAllocateWidget(
        page_stepper_struct *ps,
        int page,
        char *name,
        int w_type,
        int x, int y,
        unsigned int width, unsigned int height,
        int argc, char *argv[]
)
{
	int i, status;
	page_widget_struct *w_ptr;
	page_stepper_page_struct *page_ptr;

	prompt_window_struct *prompt;


	if(ps == NULL)
	    return(-1);

	if(!PStepperIsPageAllocated(ps, page))
	    return(-1);

	page_ptr = ps->page[page];


	/* Allocate more widget pointers as needed. */
	if(page_ptr->total_widgets < 0)
	    page_ptr->total_widgets = 0;
	i = page_ptr->total_widgets;
	page_ptr->total_widgets++;

	page_ptr->widget = (page_widget_struct **)realloc(
	    page_ptr->widget,
	    page_ptr->total_widgets * sizeof(page_widget_struct *)
	);
	if(page_ptr->widget == NULL)
	{
	    page_ptr->total_widgets = 0;
	    return(-1);
	}

	/* Allocate new page widget structure. */
	page_ptr->widget[i] = (page_widget_struct *)calloc(
	    1,
	    sizeof(page_widget_struct)
	);
	if(page_ptr->widget[i] == NULL)
	{
	    page_ptr->total_widgets = i;
	    return(-1);
	}

	/* Get pointer to widget structure. */
	w_ptr = page_ptr->widget[i];


	/* ******************************************************* */

	/* Set identifier name. */
	w_ptr->name = StringCopyAlloc(name);

	/* Set widget type. */
	w_ptr->w_type = w_type;

	/* Allocate and initiate widget. */
	status = 0;
	switch(w_ptr->w_type)
	{
          case WTYPE_CODE_PUSHBUTTON:
	    w_ptr->w = (void *)calloc(
		1,
		sizeof(push_button_struct)
	    );
	    if(w_ptr->w == NULL)
	    {
		return(-1);
	    }
            status = PBtnInit(
		(push_button_struct *)w_ptr->w,
                ps->toplevel,
		x, y,
                width, height,
                (argc >= 1) ? argv[0] : NULL,
                PBTN_TALIGN_CENTER,
                NULL,
                w_ptr->w,
                PStepperPBCB
            );
            break;
                
          case WTYPE_CODE_LIST:
            w_ptr->w = (void *)calloc(
                1,
                sizeof(list_window_struct)
            );
            if(w_ptr->w == NULL)
            {
                return(-1);
            }
            status = ListWinInit(
		(list_window_struct *)w_ptr->w,
                ps->toplevel,
                x, y,
                width, height
	    );
            break;

          case WTYPE_CODE_PROGRESSBAR:
            w_ptr->w = (void *)calloc(
                1,
                sizeof(progress_bar_struct)
            );
            if(w_ptr->w == NULL)
            {
                return(-1);
            }
            status = PBarInit(
                (progress_bar_struct *)w_ptr->w,
                ps->toplevel,
                x, y,
                width, height,
		atof((argc >= 1) ? argv[0] : "0"),
		atof((argc >= 2) ? argv[1] : "0"),
                atof((argc >= 3) ? argv[2] : "100"),
                NULL,
                PBAR_COMPLETION_HOLD
            );
            break;
                
          case WTYPE_CODE_PROMPT:
            w_ptr->w = (void *)calloc(
                1,
                sizeof(prompt_window_struct)
            );
            if(w_ptr->w == NULL)
            {
                return(-1);   
            }
            status = PromptInit(
                (prompt_window_struct *)w_ptr->w,
                ps->toplevel,
                x, y,
                width, height,
                PROMPT_STYLE_FLUSHED,
                (argc >= 1) ? argv[0] : NULL,		/* name. */
		(argc >= 3) ? atoi(argv[2]) : 80,	/* buf_len. */
		0,
		NULL
            );
	    prompt = (prompt_window_struct *)w_ptr->w;
	    if((prompt->buf != NULL) && (argc >= 2))
	    {
		strncpy(prompt->buf, argv[1], prompt->buf_len);
                PromptUnmarkBuffer(prompt, PROMPT_POS_END);
	    }
            break;

          case WTYPE_CODE_PULIST:
            w_ptr->w = (void *)calloc(
                1,
                sizeof(popup_list_struct)
            );
            if(w_ptr->w == NULL)
            {
                return(-1);
            } 
            status = PUListInit(
                (popup_list_struct *)w_ptr->w,
                ps->toplevel,
                x, y,
                width, height,
                8,		/* Items visable on popup. */
                PULIST_POPUP_DOWN,
                NULL,
                NULL
            );
	    for(i = 0; i < argc; i++)
	    {
		PUListAddItem(
		    (popup_list_struct *)w_ptr->w,
                    argv[i],
		    False
		);
	    }
            break;

          case WTYPE_CODE_SCALEBAR:
            w_ptr->w = (void *)calloc(
                1,
                sizeof(scale_bar_struct)
            );
            if(w_ptr->w == NULL)
            {
                return(-1);
            }
            status = ScaleBarInit(
                (scale_bar_struct *)w_ptr->w,
                ps->toplevel,
                x, y,
                width,		/* Length. */
                SCALEBAR_STYLE_STANDARD_VALUE,
                5,
                SCALEBAR_ORIENT_HORIZONTAL,
                atof((argc >= 2) ? argv[1] : "0"),	/* Min. */
                atof((argc >= 3) ? argv[2] : "100"),	/* Max. */
                atof((argc >= 1) ? argv[0] : "50"),	/* Cur. */
                False,
		NULL, NULL
            );
            break;
              
          case WTYPE_CODE_TOGGLEARRAY:
            w_ptr->w = (void *)calloc(
                1,   
                sizeof(toggle_button_array_struct)
            );
            if(w_ptr->w == NULL)
            {
                return(-1);
            }
            status = TgBtnArrayInit(
                (toggle_button_array_struct *)w_ptr->w,
                ps->toplevel,
                x, y,
                argc,			/* Number of buttons. */
                0,
		argv,			/* Names. */
		argc,			/* Number of names. */
                TGBTN_ARRAY_ALIGN_VERTICAL
	    );
            break;
        
          case WTYPE_CODE_TOGGLEBTN:
            w_ptr->w = (void *)calloc(
                1,
                sizeof(toggle_button_struct)
            );  
            if(w_ptr->w == NULL)
            {
                return(-1);
            }
            status = TgBtnInit(
                (toggle_button_struct *)w_ptr->w,
                ps->toplevel,
                x, y,
                True,
		(argc >= 1) ? argv[0] : NULL
            );
            break;


	  default:
	    fprintf(stderr,
 "PStepperAllocateWidget(): Unsupported widget type code %i.\n",
		w_ptr->w_type
	    );
	    status = -1;
	    break;
	}


	return(status);
}


/*
 *	Destroys a widget on a page stepper's page, does
 *	not deallocate the widget structure.
 */
void PStepperDestroyWidget(page_widget_struct *pw)
{
	if(pw == NULL)
	    return;

	switch(pw->w_type)
	{
          case WTYPE_CODE_PUSHBUTTON:
            PBtnDestroy((push_button_struct *)pw->w);
            break;
        
          case WTYPE_CODE_LIST:
            ListWinDestroy((list_window_struct *)pw->w);
            break;
 
          case WTYPE_CODE_PROGRESSBAR:
	    PBarDestroy((progress_bar_struct *)pw->w);
            break;

          case WTYPE_CODE_PROMPT:
            PromptDestroy((prompt_window_struct *)pw->w);
            break;
          
          case WTYPE_CODE_PULIST:
            PUListDestroy((popup_list_struct *)pw->w); 
            break;

          case WTYPE_CODE_SCALEBAR:
            ScaleBarDestroy((scale_bar_struct *)pw->w);
            break;

          case WTYPE_CODE_TOGGLEARRAY:
            TgBtnArrayDestroy((toggle_button_array_struct *)pw->w);
            break;
          
          case WTYPE_CODE_TOGGLEBTN:
            TgBtnDestroy((toggle_button_struct *)pw->w);
            break;

	  default:
	    fprintf(stderr,
		"PStepperDestroyWidget(): Unsupported widget type code %i.\n",
		pw->w_type
	    );
	    break;
	}

	/* Deallocate actual widget's structure. */
	free(pw->w);
	pw->w = NULL;

	pw->w_type = -1;

	/* Free identifier name. */
	free(pw->name);
	pw->name = NULL;


	return;
}




