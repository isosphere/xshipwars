// widgets/wtogglebtn.cpp
/*
                          Widget: Toggle Button

	Functions:

	int TgBtnInit(
	        toggle_button_struct *tb,
	        win_t parent,
	        int x, int y,
	        bool_t state,
	        char *label
	)
	int TgBtnSetHintMessage(
		toggle_button_struct *tb,
		char *message
	)
	int TgBtnDraw(toggle_button_struct *tb, int amount)
	int TgBtnManage(toggle_button_struct *tb, event_t *event)
	void TgBtnMap(toggle_button_struct *tb)
	void TgBtnUnmap(toggle_button_struct *tb)
	void TgBtnDestroy(toggle_button_struct *tb)

 */


#include "../include/widget.h"


/*
 *	Size constants (in pixels):
 */
#define TB_MARGIN	2

#define TB_MIN_WIDTH	((2 * TB_MARGIN) + 2)
#define TB_MIN_HEIGHT	((2 * TB_MARGIN) + 2)

#define TB_CHAR_WIDTH	7
#define TB_CHAR_HEIGHT	14


/*
 *	Initialize toggle button.
 */
int TgBtnInit(
	toggle_button_struct *tb,
	win_t parent,
	int x, int y,
	bool_t state,
	char *label
)
{
	int len;
	image_t *ximage;


	if((tb == NULL) ||
	   (parent == 0) ||
	   (osw_gui[0].root_win == 0)
	)
	    return(-1);

	/* Reset values. */
	tb->map_state = 0;
	tb->visibility_state = VisibilityFullyObscured;
	tb->x = x;
	tb->y = y;
	tb->is_in_focus = 0;
	tb->disabled = False;
	tb->font = widget_global.std_font;
        tb->next = NULL;
        tb->prev = NULL;

	/* Calculate width and height. */
	ximage = widget_global.toggle_btn_unarmed_img;
	len = (label == NULL) ? 0 : strlen(label);
	tb->width = ((ximage == NULL) ? 16 : ximage->width) +
            (int)(TB_CHAR_WIDTH * len) + (int)(TB_MARGIN * 3);
	tb->height = ((ximage == NULL) ? 16 : ximage->height) +
	    (int)(TB_MARGIN * 2);


	/* Set toggle button state. */
	tb->state = state;

	/* Allocate label? */
	if(label != NULL)
	{
	    /* Allocate and copy label. */
	    len = strlen(label);
	    tb->label = (char *)calloc(1, (len + 1) * sizeof(char) );
	    if(tb->label == NULL)
		return(-1);

	    strcpy(tb->label, label);
	}
	else
	{
	    tb->label = NULL;
	}


	/* Initialize toplevel. */
        if(
	    OSWCreateWindow(
                &tb->toplevel,
	        parent,
	        tb->x, tb->y,
	        tb->width, tb->height
	    )
	)
	    return(-1);

	tb->toplevel_buf = 0;

        OSWSetWindowInput(
	    tb->toplevel,
	    ButtonPressMask | ButtonReleaseMask |
	    ExposureMask | VisibilityChangeMask |
            EnterWindowMask | LeaveWindowMask
        );


	/* Add widget to Regeristry. */
	WidgetRegAdd(tb, WTYPE_CODE_TOGGLEBTN);



	return(0);
}



/*
 *	Set hint message for toggle button.   If parent is 0,
 *	then the parent of the toggle button is used.
 */
int TgBtnSetHintMessage(
	toggle_button_struct *tb,
	char *message
)
{
        int i;


        if((tb == NULL) ||
           (message == NULL) 
        )
            return(-1);


        /* Must be initialized. */
        if(tb->toplevel == 0)
	{
	    fprintf(stderr,
 "TgBtnSetHintMessage(): Structure 0x%.8x not initialized.\n",
		(unsigned int)tb
	    );
            return(-1);
	}


        i = HintWinAddMessage(
            tb->toplevel, 
            osw_gui[0].root_win,
            0, 0,
            message
        );
        if(i < 0)
            return(-1);


	return(0);
}

/*
 *	Draws toggle button.
 */
int TgBtnDraw(toggle_button_struct *tb, int amount)
{
	int x, y;
	image_t *ximage;
	win_attr_t wattr;
        font_t *prev_font;


	if(tb == NULL)
	    return(-1);


        /* Record previous font. */
        prev_font = OSWQueryCurrentFont();
  

	OSWGetWindowAttributes(tb->toplevel, &wattr);
        ximage = widget_global.toggle_btn_unarmed_img;

	/* Map as needed. */
	if(!tb->map_state)
	{
	    OSWMapRaised(tb->toplevel);

	    /* Must chage draw amount. */
	    amount = TGBTN_DRAW_AMOUNT_COMPLETE;

	    tb->map_state = 1;
	}

        /* Recreate buffers. */
        if(tb->toplevel_buf == 0)
        {
            if(OSWCreatePixmap(
                    &tb->toplevel_buf, wattr.width, wattr.height
            ))
                return(-1);
        }


	if(1)
	{
	    /* Clear toplevel buffer. */
	    if(widget_global.force_mono)
	    {
	        OSWClearPixmap(
		    tb->toplevel_buf, wattr.width, wattr.height,
		    osw_gui[0].black_pix
		);
	    }
	    else
	    {
		WidgetPutImageTile(
		    tb->toplevel_buf, 
		    widget_global.std_bkg_img,
		    wattr.width, wattr.height
		);
	    }

	    /* Draw label. */
	    if(tb->label != NULL)
	    {
		x = ((ximage == NULL) ? 16 : ximage->width) +
                    (int)(TB_MARGIN * 2);
		y = ((int)wattr.height / 2) + 5;
		if(widget_global.force_mono)
		    OSWSetFgPix(osw_gui[0].white_pix);
		else if(tb->disabled)
		    OSWSetFgPix(widget_global.disabled_text_pix);
		else
                    OSWSetFgPix(widget_global.normal_text_pix);

		OSWSetFont(tb->font);
		OSWDrawString(
		    tb->toplevel_buf,
		    x, y,
		    tb->label
		);
	    }

	    /* Draw toggle button. */
            if(widget_global.force_mono)
            {
                OSWSetFgPix(osw_gui[0].white_pix);
                OSWDrawSolidRectangle(tb->toplevel_buf,
                    TB_MARGIN, TB_MARGIN, 16, 16);
                OSWSetFgPix(osw_gui[0].black_pix);
                OSWDrawRectangle(tb->toplevel_buf,
                    TB_MARGIN + 1, TB_MARGIN + 1, 13, 13);
                if(tb->state)
                {
                    OSWDrawLine(tb->toplevel_buf,
                        TB_MARGIN + 1, TB_MARGIN + 1,
                        TB_MARGIN + 14, TB_MARGIN + 14
                    );
                    OSWDrawLine(tb->toplevel_buf,
                        TB_MARGIN + 1, TB_MARGIN + 14,
                        TB_MARGIN + 14, TB_MARGIN + 1
                    );
                }
            }
	    else
            {   
                x = TB_MARGIN;
                y = TB_MARGIN;
                
                if(tb->state)
                {
                    WidgetPutImageNormal(
                        tb->toplevel_buf,
                        widget_global.toggle_btn_armed_img,
                        x, y,
                        True 
                    );
                }
                else
                {
                    WidgetPutImageNormal(
                        tb->toplevel_buf,
                        widget_global.toggle_btn_unarmed_img,
                        x, y,
                        True
                    );
                }
            }
	}

	OSWPutBufferToWindow(tb->toplevel, tb->toplevel_buf);

        OSWSetFont(prev_font);


	return(0);
}



int TgBtnManage(toggle_button_struct *tb, event_t *event)
{
	int events_handled = 0;


	if((tb == NULL) ||
           (event == NULL)
	)
	    return(events_handled);

	if(!tb->map_state &&
           (event->type != MapNotify)
	)
	    return(events_handled);


	switch(event->type)
	{
          /* ********************************************************* */
          case ButtonPress:
	    if(event->xany.window == tb->toplevel)
	    {
		/* Always false if distabled. */
		if(tb->disabled)
		{
		    tb->state = False;
		}
		else if(tb->state)
		{
		    tb->state = False;
		}
                else
		{
                    tb->state = True;
		}

		/* Redraw. */
                TgBtnDraw(tb, TGBTN_DRAW_AMOUNT_BUTTON);

		events_handled++;
		return(events_handled);
	    }
	    break;

          /* ********************************************************* */
	  case Expose:
	    if(event->xany.window == tb->toplevel)
	    {
		events_handled++;
	    }
	    break;

          /* ********************************************************* */
          case MotionNotify:
            break;

          /* ********************************************************* */
          case VisibilityNotify:
            if(event->xany.window == tb->toplevel)
            {
                tb->visibility_state = event->xvisibility.state;

                events_handled++;
                return(events_handled);
            }
            break;

          /* ********************************************************* */
          case EnterNotify:
            if(event->xany.window == tb->toplevel)
            {
                /* Schedual hints window map. */
                HintWinSetSchedual(
                    widget_global.hintwin_map_delay,
                    tb->toplevel
                );

                events_handled++;
                return(events_handled);
            }
            break;

          /* ********************************************************* */
          case LeaveNotify:  
            if(event->xany.window == tb->toplevel)
            {
                /* Unmap/unschedual hints window. */
                HintWinUnmap();

                events_handled++;
		return(events_handled);
            }
            break;
	}


	/* Redraw toggle button as needed. */
	if(events_handled > 0)
	{
	    TgBtnDraw(tb, TGBTN_DRAW_AMOUNT_COMPLETE);
	}


	return(events_handled);
}

/*
 *	Map toggle button.
 */
void TgBtnMap(toggle_button_struct *tb)
{
        if(tb == NULL)
            return;

	tb->map_state = 0;
        TgBtnDraw(tb, TGBTN_DRAW_AMOUNT_COMPLETE);

	return;
}

/*
 *	Unmap toggle button.
 */
void TgBtnUnmap(toggle_button_struct *tb)
{
	if(tb == NULL)
	    return;

	OSWUnmapWindow(tb->toplevel);
        tb->map_state = 0;

	/* Destroy large buffers. */
	OSWDestroyPixmap(&tb->toplevel_buf);

	return;
}

/*
 *	Destroy toggle button.
 */
void TgBtnDestroy(toggle_button_struct *tb)
{
	if(tb == NULL)
	    return;


        /* Delete widget from regeristry. */
        WidgetRegDelete(tb);


	if(IDC())
	{
            /* Delete associated hint message. */
            HintWinDeleteMessage(tb->toplevel);

	    OSWDestroyWindow(&tb->toplevel);
	    OSWDestroyPixmap(&tb->toplevel_buf);
	}


	/* Free label. */
	free(tb->label);
	tb->label = NULL;

	tb->state = False;


        tb->map_state = 0; 
        tb->visibility_state = VisibilityFullyObscured;
	tb->is_in_focus = 0;
        tb->x = 0;
        tb->y = 0;
        tb->width = 0;
        tb->height = 0;
        tb->disabled = False;
        tb->next = NULL;
        tb->prev = NULL;


	return;
}




