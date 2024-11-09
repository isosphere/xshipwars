// widgets/wscrollbar.cpp
/*
                          Widget: Scroll Bars

	Functions:

	void SBarRepeatRecordSet(scroll_bar_struct *sb,
		long dsec, int op_code,
		int width, int height,
		int max_width, int max_height
	)
	void SBarRepeatRecordClear()
	void SBarSetNotifyFunction(int (*func_notify)(scroll_bar_struct *))

	int SBarInit(
		scroll_bar_struct *sb, win_t parent,
		int width, int height
	)
	int SBarResize(
		scroll_bar_struct *sb,
        	unsigned int width, unsigned int height
	)
	int SBarDraw(
		scroll_bar_struct *sb,
		int width, int height,
		int max_width, int max_height
	)
	int SBarManage(
		scroll_bar_struct *sb,
        	int width, int height,
        	int max_width, int max_height,
        	event_t *event
	)
	void SBarDestroy(scroll_bar_struct *sb)

        int SBarManageRepeat(event_t *event)

	---

 */

#include <math.h>

#include "../include/widget.h"


/*
 *    Scroll bar event masks:
 *
 *	The KeyPressMask must be set on the toplevel parent of
 *	the ancestor of the scroll bars.
 */
#define SB_BAR_EVENTMASK	ExposureMask | PointerMotionMask | \
                                ButtonPressMask | ButtonReleaseMask

#define SB_BTN_EVENTMASK	ButtonPressMask | ButtonReleaseMask




/*      
 *	Scroll bar repeat scroll record:
 */
#define SB_OP_CODE_NONE         0
#define SB_OP_CODE_SCROLL_UP    1       /* One unit. */
#define SB_OP_CODE_SCROLL_DOWN  2       /* One unit. */
#define SB_OP_CODE_SCROLL_LEFT  3       /* One unit. */
#define SB_OP_CODE_SCROLL_RIGHT 4       /* One unit. */
#define SB_OP_CODE_PAGE_UP      5
#define SB_OP_CODE_PAGE_DOWN    6
#define SB_OP_CODE_PAGE_LEFT    7
#define SB_OP_CODE_PAGE_RIGHT   8


typedef struct {
   
    scroll_bar_struct *sb;
    long next_repeat;
    int sb_op_code;     /* What to do. */
        
    int width, height;          /* Visibile size of window. */
    int max_width, max_height;  /* Total size of window. */

    int (*func_notify)(scroll_bar_struct *);

} sb_repeat_record_struct;
sb_repeat_record_struct sb_repeat_record[1];


/*
 *	Sets the scroll bar repeat record.
 */
void SBarRepeatRecordSet(
	scroll_bar_struct *sb,
	long dsec, int op_code,
	int width, int height,
	int max_width, int max_height
)
{
	sb_repeat_record[0].sb = sb;
	sb_repeat_record[0].next_repeat = MilliTime() + dsec;
        sb_repeat_record[0].sb_op_code = op_code;
        sb_repeat_record[0].width = width;
        sb_repeat_record[0].height = height;
        sb_repeat_record[0].max_width = max_width;
        sb_repeat_record[0].max_height = max_height;

	return;
}

/*
 *	Resets scroll bar repeat records.
 */
void SBarRepeatRecordClear()
{
	sb_repeat_record[0].sb = NULL;
	sb_repeat_record[0].next_repeat = 0;
	sb_repeat_record[0].sb_op_code = SB_OP_CODE_NONE;
        sb_repeat_record[0].width = 0;
        sb_repeat_record[0].height = 0;
        sb_repeat_record[0].max_width = 0;
        sb_repeat_record[0].max_height = 0;

	return;
}


/*
 *	Sets scroll bar update notify function.
 */
void SBarSetNotifyFunction(int (*func_notify)(scroll_bar_struct *))
{
	sb_repeat_record[0].func_notify = func_notify;

	return;
}


/*
 *	Initializes scroll bar.
 */
int SBarInit(
	scroll_bar_struct *sb,
	win_t parent,
	unsigned int width,
	unsigned int height
)
{

        if((sb == NULL) ||
           (parent == 0)
	)
            return(-1);


	/* Reset values. */
	sb->map_state = 0;
	sb->is_in_focus = 0;
	sb->visibility_state = VisibilityFullyObscured;
	sb->x = 0;
	sb->y = 0;
	sb->width = width;
	sb->height = height;
	sb->font = OSWQueryCurrentFont();
	sb->prev = NULL;
	sb->next = NULL;


	/* Sanitize width and height. */
	if((int)width <=
	    (SCROLLBAR_YBAR_WIDTH + (2 * SCROLLBAR_CURSOR_BTN_WIDTH) + 2)
        )
	{
	    width = (SCROLLBAR_YBAR_WIDTH +
		(2 * SCROLLBAR_CURSOR_BTN_WIDTH) + 4);
	}

        if((int)height <=
            (SCROLLBAR_XBAR_HEIGHT + ( 2 * SCROLLBAR_CURSOR_BTN_HEIGHT) + 2)
        )
	{
            height = (SCROLLBAR_XBAR_HEIGHT +
		(2 * SCROLLBAR_CURSOR_BTN_HEIGHT) + 4);
	}


        /* Reset scroll bar positions. */
        sb->x_win_pos = 0;
	sb->y_win_pos = 0;

	/* Reset bar button states. */
	sb->x_bar_button_state = False;
	sb->y_bar_button_state = False;


	/* ************************************************************** */
	
        /* x_toplevel */
	if(OSWCreateWindow(
	    &sb->x_toplevel,
            parent,
            0,
            height - SCROLLBAR_XBAR_HEIGHT,
            width - SCROLLBAR_YBAR_WIDTH,
            SCROLLBAR_XBAR_HEIGHT
        ))
	    return(-1);
	OSWSetWindowCursor(sb->x_toplevel, osw_gui[0].std_cursor);


        /* y_toplevel */
        if(OSWCreateWindow(
            &sb->y_toplevel,
            parent,
            width - SCROLLBAR_YBAR_WIDTH,	/* x, y. */
            0,
            SCROLLBAR_YBAR_WIDTH,		/* width, height. */
            height - SCROLLBAR_XBAR_HEIGHT
        ))
            return(-1);
	OSWSetWindowCursor(sb->y_toplevel, osw_gui[0].std_cursor);


        /* x_left */
        if(OSWCreateWindow(
            &sb->x_left,
            sb->x_toplevel,
            0,
            0,
            SCROLLBAR_CURSOR_BTN_WIDTH,
            SCROLLBAR_CURSOR_BTN_HEIGHT
        ))
            return(-1);
	if(OSWCreatePixmap(
            &sb->x_left_buf,
            SCROLLBAR_CURSOR_BTN_WIDTH, SCROLLBAR_CURSOR_BTN_HEIGHT
        ))
            return(-1);

	/* x_bar */
        if(OSWCreateWindow(
            &sb->x_bar,
            sb->x_toplevel,
            SCROLLBAR_CURSOR_BTN_WIDTH + 1,
            0,
            width - (2 * SCROLLBAR_CURSOR_BTN_WIDTH) - SCROLLBAR_YBAR_WIDTH - 2,
            SCROLLBAR_CURSOR_BTN_HEIGHT
        ))
            return(-1);
	if(OSWCreatePixmap(
            &sb->x_bar_buf,
            width, SCROLLBAR_CURSOR_BTN_HEIGHT
	))
            return(-1);

        /* x_right */
        if(OSWCreateWindow(
            &sb->x_right,
            sb->x_toplevel,
            width - SCROLLBAR_CURSOR_BTN_WIDTH - SCROLLBAR_YBAR_WIDTH,
            0,
            SCROLLBAR_CURSOR_BTN_WIDTH,
            SCROLLBAR_CURSOR_BTN_HEIGHT
        ))
            return(-1);
        if(OSWCreatePixmap(
            &sb->x_right_buf,
	    SCROLLBAR_CURSOR_BTN_WIDTH, SCROLLBAR_CURSOR_BTN_HEIGHT
        ))
            return(-1);


        /* y_up */
        if(OSWCreateWindow(
            &sb->y_up,
            sb->y_toplevel,
            0,
            0,
            SCROLLBAR_CURSOR_BTN_WIDTH,
            SCROLLBAR_CURSOR_BTN_HEIGHT
        ))
            return(-1);
        if(OSWCreatePixmap(
            &sb->y_up_buf,
            SCROLLBAR_CURSOR_BTN_WIDTH, SCROLLBAR_CURSOR_BTN_HEIGHT
        ))
            return(-1);

        /* y_bar */
        if(OSWCreateWindow(
            &sb->y_bar,
            sb->y_toplevel,
            0,
            SCROLLBAR_CURSOR_BTN_HEIGHT + 1,
            SCROLLBAR_CURSOR_BTN_WIDTH,
            height - (2 * SCROLLBAR_CURSOR_BTN_HEIGHT) - SCROLLBAR_XBAR_HEIGHT - 2
        ))
            return(-1);
        if(OSWCreatePixmap(
            &sb->y_bar_buf,
            SCROLLBAR_CURSOR_BTN_WIDTH, height
        ))
            return(-1);

        /* y_down */
        if(OSWCreateWindow(
            &sb->y_down,
            sb->y_toplevel,
	    0,
            height - SCROLLBAR_CURSOR_BTN_HEIGHT - SCROLLBAR_XBAR_HEIGHT,
            SCROLLBAR_CURSOR_BTN_WIDTH,
            SCROLLBAR_CURSOR_BTN_HEIGHT
        ))
            return(-1);
        if(OSWCreatePixmap(
            &sb->y_down_buf,
            SCROLLBAR_CURSOR_BTN_WIDTH, SCROLLBAR_CURSOR_BTN_HEIGHT
        ))
            return(-1);


        /* Set event masks. */
        OSWSetWindowInput(sb->x_left, SB_BTN_EVENTMASK);
        OSWSetWindowInput(sb->x_bar, SB_BAR_EVENTMASK);
        OSWSetWindowInput(sb->x_right, SB_BTN_EVENTMASK);

        OSWSetWindowInput(sb->y_up, SB_BTN_EVENTMASK);
        OSWSetWindowInput(sb->y_bar, SB_BAR_EVENTMASK);
        OSWSetWindowInput(sb->y_down, SB_BTN_EVENTMASK);



	/* Map Windows. */
	OSWMapWindow(sb->x_toplevel);
	OSWMapSubwindows(sb->x_toplevel);
	OSWMapWindow(sb->y_toplevel);
        OSWMapSubwindows(sb->y_toplevel);


	/* Set values. */
	sb->map_state = 1;


	/* Add widget to regeristry. */
	WidgetRegAdd(sb, WTYPE_CODE_SCROLLBAR);


	return(0);
}


/*
 *	Resizes scroll bar.
 */
int SBarResize(
        scroll_bar_struct *sb,
        unsigned int width, 
        unsigned int height
)
{
	if((sb == NULL) ||
           (width == 0) ||
           (height == 0)
	)
	    return(-1);


        /* Sanitize width and height. */
        if((int)width <=
            (SCROLLBAR_YBAR_WIDTH + (2 * SCROLLBAR_CURSOR_BTN_WIDTH) + 2)
        )
            width = (SCROLLBAR_YBAR_WIDTH +
		(2 * SCROLLBAR_CURSOR_BTN_WIDTH) + 4);

        if((int)height <=
            (SCROLLBAR_XBAR_HEIGHT + ( 2 * SCROLLBAR_CURSOR_BTN_HEIGHT) + 2)
        )
            height = (SCROLLBAR_XBAR_HEIGHT +
		(2 * SCROLLBAR_CURSOR_BTN_HEIGHT) + 4);


        /* Reset scroll bar positions. */
/*
        sb->x_win_pos = 0;
        sb->y_win_pos = 0;
*/

        /* Reset bar button states. */     
        sb->x_bar_button_state = False;
        sb->y_bar_button_state = False;

	/* Reset width and height values. */
	sb->width = width;
	sb->height = height;


	/* ************************************************************** */
	/* Resize X bar. */

	/* x_toplevel */
	OSWMoveResizeWindow(sb->x_toplevel,
            0,
            (int)height - SCROLLBAR_XBAR_HEIGHT,
            (int)width - SCROLLBAR_YBAR_WIDTH,
            SCROLLBAR_XBAR_HEIGHT
	);

	/* x_left */
        OSWMoveResizeWindow(sb->x_left,
            0,
            0,
            SCROLLBAR_CURSOR_BTN_WIDTH,
            SCROLLBAR_CURSOR_BTN_HEIGHT
	);
	OSWDestroyPixmap(&sb->x_left_buf);
        if(OSWCreatePixmap(
                &sb->x_left_buf,
                SCROLLBAR_CURSOR_BTN_WIDTH, SCROLLBAR_CURSOR_BTN_HEIGHT
        ))
            return(-1);

        /* x_bar */
        OSWMoveResizeWindow(sb->x_bar,
            SCROLLBAR_CURSOR_BTN_WIDTH + 1,
            0,
            (int)width - (2 * SCROLLBAR_CURSOR_BTN_WIDTH) - SCROLLBAR_YBAR_WIDTH - 2,
            SCROLLBAR_CURSOR_BTN_HEIGHT
        );
        OSWDestroyPixmap(&sb->x_bar_buf);
        if(OSWCreatePixmap(
                &sb->x_bar_buf,
                (int)width, SCROLLBAR_CURSOR_BTN_HEIGHT
        ))
            return(-1);

        /* x_right */
        OSWMoveResizeWindow(sb->x_right,
            (int)width - SCROLLBAR_CURSOR_BTN_WIDTH - SCROLLBAR_YBAR_WIDTH,
            0,
            SCROLLBAR_CURSOR_BTN_WIDTH,
            SCROLLBAR_CURSOR_BTN_HEIGHT
        );
        OSWDestroyPixmap(&sb->x_right_buf);
        if(OSWCreatePixmap(
                &sb->x_right_buf,
                SCROLLBAR_CURSOR_BTN_WIDTH, SCROLLBAR_CURSOR_BTN_HEIGHT
        ))
            return(-1);


        /* ************************************************************** */
        /* Resize Y bar. */

        /* y_toplevel */
        OSWMoveResizeWindow(sb->y_toplevel,
            (int)width - SCROLLBAR_YBAR_WIDTH,
            0,
            SCROLLBAR_YBAR_WIDTH,
            (int)height - SCROLLBAR_XBAR_HEIGHT
        );

        /* y_up */
        OSWMoveResizeWindow(sb->y_up,
            0,
            0,
            SCROLLBAR_CURSOR_BTN_WIDTH,
            SCROLLBAR_CURSOR_BTN_HEIGHT
        );
        OSWDestroyPixmap(&sb->y_up_buf);
        if(OSWCreatePixmap(
                &sb->y_up_buf,
		SCROLLBAR_CURSOR_BTN_WIDTH, SCROLLBAR_CURSOR_BTN_HEIGHT
        ))
            return(-1);

        /* y_bar */
        OSWMoveResizeWindow(sb->y_bar,
            0,
            SCROLLBAR_CURSOR_BTN_HEIGHT + 1,
            SCROLLBAR_CURSOR_BTN_WIDTH,
            (int)height - (2 * SCROLLBAR_CURSOR_BTN_HEIGHT) -
		SCROLLBAR_XBAR_HEIGHT - 2
        );
        OSWDestroyPixmap(&sb->y_bar_buf);
        if(OSWCreatePixmap(
                &sb->y_bar_buf,
                SCROLLBAR_CURSOR_BTN_WIDTH, (int)height
        ))
            return(-1);

        /* y_down */
        OSWMoveResizeWindow(sb->y_down,
            0,
            (int)height - SCROLLBAR_CURSOR_BTN_HEIGHT - SCROLLBAR_XBAR_HEIGHT,
            SCROLLBAR_CURSOR_BTN_WIDTH,
            SCROLLBAR_CURSOR_BTN_HEIGHT
        );
        OSWDestroyPixmap(&sb->y_down_buf);
        if(OSWCreatePixmap( 
                &sb->y_down_buf,
                SCROLLBAR_CURSOR_BTN_WIDTH, SCROLLBAR_CURSOR_BTN_HEIGHT
        ))
            return(-1);


	return(0);
}


/*
 *	Draw scroll bar.
 */
int SBarDraw(
	scroll_bar_struct *sb,
	int width, int height,
	int max_width, int max_height
)
{
	double x_pos, y_pos;
	double bar_x, bar_y;
	double bar_width, bar_height;
	int x_bar_vis, y_bar_vis;	/* 0 = not visible, 1 = visible. */
	win_attr_t wattr;
        font_t *prev_font;


        if(sb == NULL)
            return(-1);


	/* Sanitize width, height, max_width, and max_height. */

        if(width <=
           (SCROLLBAR_YBAR_WIDTH + (2 * SCROLLBAR_CURSOR_BTN_WIDTH) + 2)
        )
        {
            width = (SCROLLBAR_YBAR_WIDTH + (2 *
                SCROLLBAR_CURSOR_BTN_WIDTH) + 4);
        }
        if(height <=
           (SCROLLBAR_XBAR_HEIGHT + ( 2 * SCROLLBAR_CURSOR_BTN_HEIGHT) + 2)
        )
        {
            height = (SCROLLBAR_XBAR_HEIGHT + (2 *
                SCROLLBAR_CURSOR_BTN_HEIGHT) + 4);
        }

        if(max_width <=
           (SCROLLBAR_YBAR_WIDTH + (2 * SCROLLBAR_CURSOR_BTN_WIDTH) + 2)
        )
        {
            max_width = (SCROLLBAR_YBAR_WIDTH + (2 *
                SCROLLBAR_CURSOR_BTN_WIDTH) + 4);
        }
        if(max_height <=
           (SCROLLBAR_XBAR_HEIGHT + ( 2 * SCROLLBAR_CURSOR_BTN_HEIGHT) + 2)
        )
        {
            max_height = (SCROLLBAR_XBAR_HEIGHT + (2 *
                SCROLLBAR_CURSOR_BTN_HEIGHT) + 4);
        } 


        /* actuals cannot be greater than maximums. */
        if(max_width < width)
            width = max_width;
        if(max_height < height) 
            height = max_height;


	/* Check if the scroll bars are visible. */
	if(max_width > width)
	    x_bar_vis = 1;
	else
	    x_bar_vis = 0;
        if(max_height > height)
            y_bar_vis = 1;
        else
            y_bar_vis = 0;


	/* ********************************************************** */
	/* Map or unmap windows. */
        if(!x_bar_vis)
        {
            OSWUnmapWindow(sb->x_left);
            OSWUnmapWindow(sb->x_bar);
            OSWUnmapWindow(sb->x_right);
            OSWUnmapWindow(sb->x_toplevel);
	    sb->x_win_pos = 0;

	    /* Adjust the size of the y scroll bar since the x scroll bar
	     * is not shown.
	     */
            OSWResizeWindow(sb->y_toplevel,
                SCROLLBAR_CURSOR_BTN_WIDTH,
                height
            );
            OSWResizeWindow(sb->y_bar,
                SCROLLBAR_CURSOR_BTN_WIDTH,
                height - (2 * SCROLLBAR_CURSOR_BTN_HEIGHT)
            );
	    OSWMoveWindow(
		sb->y_down,
		0,
                height - SCROLLBAR_CURSOR_BTN_HEIGHT
	    );
        }
        else
        {
            OSWMapWindow(sb->x_toplevel);
            OSWMapSubwindows(sb->x_toplevel);

            /* Adjust the size of the y scroll bar to accomidate 
             * the x scroll bar.
             */
            OSWResizeWindow(sb->y_toplevel,
		SCROLLBAR_YBAR_WIDTH,
                height - SCROLLBAR_XBAR_HEIGHT
            );
            OSWResizeWindow(sb->y_bar,
		SCROLLBAR_YBAR_WIDTH,
                height - (2 * SCROLLBAR_CURSOR_BTN_HEIGHT) -  
                    SCROLLBAR_XBAR_HEIGHT
            );
            OSWMoveWindow(
		sb->y_down,
		0,
                height - SCROLLBAR_CURSOR_BTN_HEIGHT -  
                    SCROLLBAR_XBAR_HEIGHT
            );
        }

	if(!y_bar_vis)
	{
	    OSWUnmapWindow(sb->y_up);
	    OSWUnmapWindow(sb->y_bar);
            OSWUnmapWindow(sb->y_down);
	    OSWUnmapWindow(sb->y_toplevel);
            sb->y_win_pos = 0;

            /* Adjust the size of the x scroll bar since the y scroll bar
             * is not shown.
             */
            OSWResizeWindow(sb->x_toplevel,
		width,
		SCROLLBAR_CURSOR_BTN_HEIGHT
            );

            OSWResizeWindow(sb->x_bar,
	        width - (2 * SCROLLBAR_CURSOR_BTN_WIDTH),
                SCROLLBAR_CURSOR_BTN_HEIGHT
            );
            OSWMoveWindow(
		sb->x_right,
                width - SCROLLBAR_CURSOR_BTN_WIDTH,
                0
            );
	}
	else
	{
            OSWMapWindow(sb->y_toplevel);
            OSWMapSubwindows(sb->y_toplevel);

            /* Adjust the size of the x scroll bar to accomidate
	     * the y scroll bar.
             */
            OSWResizeWindow(sb->x_toplevel,
                width - SCROLLBAR_YBAR_WIDTH,
                SCROLLBAR_XBAR_HEIGHT
            );

            OSWResizeWindow(sb->x_bar,
                width - (2 * SCROLLBAR_CURSOR_BTN_WIDTH) -
		    SCROLLBAR_YBAR_WIDTH,
                SCROLLBAR_CURSOR_BTN_HEIGHT
            );
            OSWMoveWindow(
		sb->x_right,  
                width - SCROLLBAR_CURSOR_BTN_WIDTH -
		    SCROLLBAR_YBAR_WIDTH,
                0
            );
	}


	/* ******************************************************** */
	/* Begin redrawing. */

        prev_font = OSWQueryCurrentFont();
        OSWSetFont(sb->font);


	/* Clear buffers. */

	OSWGetWindowAttributes(sb->x_left, &wattr);
	OSWClearPixmap(sb->x_left_buf, wattr.width, wattr.height,
	    (widget_global.force_mono) ?
            osw_gui[0].black_pix : widget_global.scroll_bkg_pix);

        OSWGetWindowAttributes(sb->x_bar, &wattr);
        OSWClearPixmap(sb->x_bar_buf, wattr.width, wattr.height,
            (widget_global.force_mono) ?
            osw_gui[0].black_pix : widget_global.scroll_bkg_pix);

        OSWGetWindowAttributes(sb->x_right, &wattr);
        OSWClearPixmap(sb->x_right_buf, wattr.width, wattr.height,
            (widget_global.force_mono) ?
            osw_gui[0].black_pix : widget_global.scroll_bkg_pix);


        OSWGetWindowAttributes(sb->y_up, &wattr);
        OSWClearPixmap(sb->y_up_buf, wattr.width, wattr.height,
            (widget_global.force_mono) ?
            osw_gui[0].black_pix : widget_global.scroll_bkg_pix);

        OSWGetWindowAttributes(sb->y_bar, &wattr);
        OSWClearPixmap(sb->y_bar_buf, wattr.width, wattr.height,
            (widget_global.force_mono) ?
            osw_gui[0].black_pix : widget_global.scroll_bkg_pix);

        OSWGetWindowAttributes(sb->y_down, &wattr);
        OSWClearPixmap(sb->y_down_buf, wattr.width, wattr.height,
            (widget_global.force_mono) ?
            osw_gui[0].black_pix : widget_global.scroll_bkg_pix);


	/* Begin drawing. */

	/* x_left */
	if(widget_global.force_mono)
	    OSWSetFgPix(osw_gui[0].white_pix);
	else
	    OSWSetFgPix(widget_global.scroll_cursor_pix);
        OSWDrawSolidArc(
           sb->x_left_buf,
           0,					/* x, y */
	   0,
           SCROLLBAR_CURSOR_BTN_WIDTH * 2,	/* width, height. */
	   SCROLLBAR_CURSOR_BTN_HEIGHT,
           PI * 0.5, PI				/* angle1, angle2 */
        );
	OSWPutBufferToWindow(sb->x_left, sb->x_left_buf);

        /* x_right */
        OSWDrawSolidArc(
           sb->x_right_buf,
           0 - SCROLLBAR_CURSOR_BTN_WIDTH,      /* x, y */
           0,
           SCROLLBAR_CURSOR_BTN_WIDTH * 2,      /* width, height. */
           SCROLLBAR_CURSOR_BTN_HEIGHT,
           PI * 1.5, PI	* 1.05			/* angle1, angle2 */
        );
	OSWPutBufferToWindow(sb->x_right, sb->x_right_buf);


        /* y_up */
        OSWDrawSolidArc(
           sb->y_up_buf,
           0,                                   /* x, y */
           0,
           SCROLLBAR_CURSOR_BTN_WIDTH,		/* width, height. */
           SCROLLBAR_CURSOR_BTN_HEIGHT * 2,
           0, PI				/* angle1, angle2 */
        );
        OSWPutBufferToWindow(sb->y_up, sb->y_up_buf);

        /* y_down */
        OSWDrawSolidArc(
           sb->y_down_buf,
           0,					/* x, y */
           0 - SCROLLBAR_CURSOR_BTN_HEIGHT,
           SCROLLBAR_CURSOR_BTN_WIDTH,		/* width, height. */
           SCROLLBAR_CURSOR_BTN_HEIGHT * 2,
           PI, PI * 1.05			/* angle1, angle2 */
        );
        OSWPutBufferToWindow(sb->y_down, sb->y_down_buf);

        /* x_bar (frame) */
        if(widget_global.force_mono)
            OSWSetFgPix(osw_gui[0].white_pix);
        else
            OSWSetFgPix(widget_global.scroll_frame_pix);
	if(y_bar_vis == 0)
	{
            OSWDrawRectangle(sb->x_bar_buf,
                0, 1,
                width - (2 * SCROLLBAR_CURSOR_BTN_WIDTH) - 3,
                SCROLLBAR_CURSOR_BTN_HEIGHT - 2
            );
	}
	else
	{
            OSWDrawRectangle(sb->x_bar_buf,
                0, 1,
	        width - (2 * SCROLLBAR_CURSOR_BTN_WIDTH) - SCROLLBAR_YBAR_WIDTH
                - 3,
                SCROLLBAR_CURSOR_BTN_HEIGHT - 2
            );
	}
        /* y_bar (frame) */
        if(x_bar_vis == 0)
	{
            OSWDrawRectangle(sb->y_bar_buf,
                1, 0,
                SCROLLBAR_CURSOR_BTN_WIDTH - 2,
                height - (2 * SCROLLBAR_CURSOR_BTN_HEIGHT) - 3
            );
	}
	else
	{
            OSWDrawRectangle(sb->y_bar_buf,
                1, 0,
	        SCROLLBAR_CURSOR_BTN_WIDTH - 2,
                height - (2 * SCROLLBAR_CURSOR_BTN_HEIGHT) -
                SCROLLBAR_XBAR_HEIGHT - 3
            );
	}

        /* x_bar (bar) */
	x_pos = sb->x_win_pos;
	if(x_pos < 0)
	    x_pos = 0;
	if(x_pos > ((int)max_width - (int)width))
	    x_pos = (int)max_width - (int)width;

	if(y_bar_vis == 0)
	{
            bar_x = x_pos *
                (((double)width - (2 * SCROLLBAR_CURSOR_BTN_WIDTH) - 4)
                / (double)max_width);
        
            bar_width = (double)width *
                (((double)width - (2 * SCROLLBAR_CURSOR_BTN_WIDTH) - 4)
                / (double)max_width);
	}
	else
	{
	    bar_x = x_pos *
	        (((double)width - (2 * SCROLLBAR_CURSOR_BTN_WIDTH) -
	        SCROLLBAR_YBAR_WIDTH - 4) / (double)max_width);

	    bar_width = (double)width *
	        (((double)width - (2 * SCROLLBAR_CURSOR_BTN_WIDTH) -
                SCROLLBAR_YBAR_WIDTH - 4) / (double)max_width);
	}
	if(bar_width < 1)
	    bar_width = 1;

	/* Draw x bar. */
        if(widget_global.force_mono)
            OSWSetFgPix(osw_gui[0].white_pix);
        else
            OSWSetFgPix(widget_global.scroll_frame_pix);
        OSWDrawLine(sb->x_bar_buf,
            (int)bar_x, 2,
            (int)bar_x, SCROLLBAR_CURSOR_BTN_WIDTH - 2
        );
        OSWDrawLine(sb->x_bar_buf,
            (int)bar_x + (int)bar_width + 1, 2,
            (int)bar_x + (int)bar_width + 1,
            SCROLLBAR_CURSOR_BTN_WIDTH - 2
        );
        if(widget_global.force_mono)
            OSWSetFgPix(osw_gui[0].white_pix);
        else
            OSWSetFgPix(widget_global.scroll_bar_pix);
        OSWDrawSolidRectangle(sb->x_bar_buf,
            (int)bar_x + 1, 2,
            (int)bar_width,
            SCROLLBAR_CURSOR_BTN_HEIGHT - 3
        );


        /* y_bar (bar) */
        y_pos = sb->y_win_pos;
        if(y_pos < 0)
            y_pos = 0;
        if(y_pos > ((int)max_height - (int)height))
            y_pos = (int)max_height - (int)height;

        if(x_bar_vis == 0)
	{
            bar_y = y_pos *
                (((double)height - (2 * SCROLLBAR_CURSOR_BTN_HEIGHT) - 4)
                / (double)max_height);
            
            bar_height = (double)height *
                (((double)height - (2 * SCROLLBAR_CURSOR_BTN_HEIGHT) - 4)
                / (double)max_height);
	}
	else
	{
            bar_y = y_pos *  
                (((double)height - (2 * SCROLLBAR_CURSOR_BTN_HEIGHT) -
                SCROLLBAR_XBAR_HEIGHT - 4) / (double)max_height);

            bar_height = (double)height *
                (((double)height - (2 * SCROLLBAR_CURSOR_BTN_HEIGHT) -
                SCROLLBAR_XBAR_HEIGHT - 4) / (double)max_height);
        }
	if(bar_height < 1)
	    bar_height = 1;
	/* Draw y bar. */
        if(widget_global.force_mono)
            OSWSetFgPix(osw_gui[0].white_pix);
        else
	    OSWSetFgPix(widget_global.scroll_frame_pix);
	OSWDrawLine(sb->y_bar_buf,
	    2, (int)bar_y,
	    SCROLLBAR_CURSOR_BTN_WIDTH - 2, (int)bar_y
	);
	OSWDrawLine(sb->y_bar_buf,
            2, (int)bar_y + (int)bar_height + 1,
            SCROLLBAR_CURSOR_BTN_WIDTH - 2, (int)bar_y + (int)bar_height + 1
        );
        if(widget_global.force_mono)
            OSWSetFgPix(osw_gui[0].white_pix);
        else
            OSWSetFgPix(widget_global.scroll_bar_pix);
        OSWDrawSolidRectangle(sb->y_bar_buf,
            2, (int)bar_y + 1,
	    SCROLLBAR_CURSOR_BTN_WIDTH - 3,
            (int)bar_height
        );


	OSWPutBufferToWindow(sb->x_bar, sb->x_bar_buf);
        OSWPutBufferToWindow(sb->y_bar, sb->y_bar_buf);

        OSWSetFont(prev_font);


	return(0);
}


/*
 *	Manage scroll bar.
 */
int SBarManage(
	scroll_bar_struct *sb,		/* Scrollbar return. */
	int width, int height,		/* Visibile size of window. */
	int max_width, int max_height,  /* Total size of window. */
	event_t *event
)
{
	int bar_x_min, bar_y_min;
	int bar_x_max, bar_y_max;
	int max_x_pos, max_y_pos;

	bool_t x_bar_vis;
	bool_t y_bar_vis;

	int events_handled = 0;
	win_attr_t wattr;

	int ori_width, ori_height;
	int ori_max_width, ori_max_height;


        if((event == NULL) ||
           (sb == NULL)
	)
            return(events_handled);

	if(!sb->map_state &&
           (event->type != MapNotify)
	)
	    return(events_handled);


	/* Record original positions. */
        ori_width = width;
	ori_height = height;
        ori_max_width = max_width;
        ori_max_height = max_height;


        /* Sanitize width, height, max_width, and max_height. */
        if((int)width <=
           (SCROLLBAR_YBAR_WIDTH + (2 * SCROLLBAR_CURSOR_BTN_WIDTH) + 2)
        )
        {
            width = (SCROLLBAR_YBAR_WIDTH +
                (2 * SCROLLBAR_CURSOR_BTN_WIDTH) + 4);
        }

        if((int)height <=
           (SCROLLBAR_XBAR_HEIGHT + (2 * SCROLLBAR_CURSOR_BTN_HEIGHT) + 2)
        )
        {
            height = (SCROLLBAR_XBAR_HEIGHT +
                (2 * SCROLLBAR_CURSOR_BTN_HEIGHT) + 4);
        }
           
        if((int)max_width <=
           (SCROLLBAR_YBAR_WIDTH + (2 * SCROLLBAR_CURSOR_BTN_WIDTH) + 2)
        )
        {
            max_width = (SCROLLBAR_YBAR_WIDTH +
                (2 * SCROLLBAR_CURSOR_BTN_WIDTH) + 4);
        }
        if((int)max_height <=
           (SCROLLBAR_XBAR_HEIGHT + ( 2 * SCROLLBAR_CURSOR_BTN_HEIGHT) + 2)
        )
        {
            max_height = (SCROLLBAR_XBAR_HEIGHT +
                (2 * SCROLLBAR_CURSOR_BTN_HEIGHT) + 4);
        }

	/* actuals cannot be greater than maximums. */
	if((int)max_width < (int)width)
	    width = max_width;
	if((int)max_height < (int)height)
	    height = max_height;

	/* Get bar visual states. */
	x_bar_vis = ((int)width < (int)max_width) ? True : False;
	y_bar_vis = ((int)height < (int)max_height) ? True : False;

        /* Calculate max_x_pos and max_y_pos. */
        max_x_pos = (int)max_width - (int)width;
        max_y_pos = (int)max_height - (int)height;

	switch(event->type)
	{
          /* ****************************************************** */
	  case Expose:
	    if((event->xany.window == sb->x_toplevel) ||
               (event->xany.window == sb->y_toplevel) ||
               (event->xany.window == sb->x_bar) ||
               (event->xany.window == sb->y_bar) ||
               (event->xany.window == sb->x_left) ||
               (event->xany.window == sb->x_right) ||
               (event->xany.window == sb->y_up) ||
               (event->xany.window == sb->y_down)
	    )
	        events_handled++;

	    break;

          /* ****************************************************** */
	  case KeyPress:
	    /* Skip if not in focus. */
	    if(sb->is_in_focus == 0)
		return(events_handled);

	    /* Up */
	    if(event->xkey.keycode == osw_keycode.cursor_up)
	    {
		sb->y_win_pos -= SCROLLBAR_CURSOR_INC;
		if(sb->y_win_pos < 0)
		    sb->y_win_pos = 0;

		events_handled++;
	    }
	    /* Down */
	    else if(event->xkey.keycode == osw_keycode.cursor_down)
	    {
                sb->y_win_pos += SCROLLBAR_CURSOR_INC;
                if(sb->y_win_pos > max_y_pos)
                    sb->y_win_pos = max_y_pos;

                events_handled++;
	    }
            /* Page Up */
            else if(event->xkey.keycode == osw_keycode.page_up)
            {
                sb->y_win_pos -= height;
                if(sb->y_win_pos < 0)
                    sb->y_win_pos = 0;

                events_handled++;
            }
            /* Page Down */
            else if(event->xkey.keycode == osw_keycode.page_down)
            {
                sb->y_win_pos += height;
                if(sb->y_win_pos > max_y_pos)
                    sb->y_win_pos = max_y_pos;

                events_handled++;
            }
            /* Home */
            else if(event->xkey.keycode == osw_keycode.home)
            {
                sb->y_win_pos = 0;

                events_handled++;
            }
            /* End */
            else if(event->xkey.keycode == osw_keycode.end)
            {
                sb->y_win_pos = max_y_pos;

                events_handled++;
            }
            /* Left */
            else if(event->xkey.keycode == osw_keycode.cursor_left)
            {
                sb->x_win_pos -= SCROLLBAR_CURSOR_INC;
                if(sb->x_win_pos < 0)
                    sb->x_win_pos = 0;

                events_handled++;
            }
            /* Right */
            else if(event->xkey.keycode == osw_keycode.cursor_right)
            {
                sb->x_win_pos += SCROLLBAR_CURSOR_INC;
                if(sb->x_win_pos > max_x_pos)
                    sb->x_win_pos = max_x_pos;

                events_handled++;
            }
	    break;

          /* ****************************************************** */
          case KeyRelease:
            /* Skip if not in focus. */
            if(sb->is_in_focus == 0)
                return(events_handled);

	    break;

          /* ****************************************************** */
	  case MotionNotify:
	    /* x bar. */
	    if((event->xany.window == sb->x_bar) &&
               sb->x_bar_button_state
	    )
	    {
		OSWGetWindowAttributes(sb->x_bar, &wattr);
		if(wattr.width > 0)
		{
                    sb->x_win_pos = static_cast<int>((double)event->xbutton.x /
                        (double)wattr.width * (double)max_width);
		}
		else
		{
                    sb->x_win_pos = 0;
		}
                sb->x_win_pos -= (((int)width / 2) -
                    sb->x_origin_delta);
            
                if(sb->x_win_pos < 0)
                    sb->x_win_pos = 0;
                else if(sb->x_win_pos > (max_width - width))
                    sb->x_win_pos = max_width - width;

                /* Purge all other motionnotify events. */
                OSWPurgeTypedEvent(MotionNotify);

                events_handled++;
	    }
            /* y bar. */
            else if((event->xany.window == sb->y_bar) &&
                    sb->y_bar_button_state
            )
            {
                OSWGetWindowAttributes(sb->y_bar, &wattr);
                if(wattr.height > 0)
                {
                    sb->y_win_pos = static_cast<int>((double)event->xbutton.y /
                        (double)wattr.height * (double)max_height);
                }
                else   
                {
                    sb->y_win_pos = 0;
                }
                sb->y_win_pos -= (((int)height / 2) -
                    sb->y_origin_delta);

		if(sb->y_win_pos < 0)
		    sb->y_win_pos = 0;
		else if(sb->y_win_pos > (max_height - height))
		    sb->y_win_pos = max_height - height;

		/* Purge all other motionnotify events. */
		OSWPurgeTypedEvent(MotionNotify);

                events_handled++;
            }
	    break;

          /* ****************************************************** */
          case ButtonPress:
	    /* Change focus by which window buttonpress is on. */
	    if((event->xany.window == sb->x_bar) ||
               (event->xany.window == sb->y_bar) ||
               (event->xany.window == sb->x_left) ||
               (event->xany.window == sb->x_right) ||
               (event->xany.window == sb->y_up) ||
               (event->xany.window == sb->y_down)
	    )
	        sb->is_in_focus = 1;
	    else
		sb->is_in_focus = 0;


	    /* x bar. */
	    if((event->xany.window == sb->x_bar) &&
               !sb->x_bar_button_state
	    )
	    {
		/* Calculate bar min and max positions. */
                if(y_bar_vis)
                {
                    bar_x_min = static_cast<int>(sb->x_win_pos *
                        (((double)width -
                        (2 * SCROLLBAR_CURSOR_BTN_WIDTH) -
                        SCROLLBAR_YBAR_WIDTH - 4) / (double)max_width));
                    bar_x_max = static_cast<int>((double)width *
                        (((double)width -
                        (2 * SCROLLBAR_CURSOR_BTN_WIDTH) -
                        SCROLLBAR_YBAR_WIDTH - 4) / (double)max_width));
                }
                else
                {
                    bar_x_min = static_cast<int>(sb->x_win_pos *
                        (((double)width -
                        (2 * SCROLLBAR_CURSOR_BTN_WIDTH) - 4)
                        / (double)max_width));
                    bar_x_max = static_cast<int>((double)width *
                        (((double)width -
                        (2 * SCROLLBAR_CURSOR_BTN_WIDTH) - 4)
                        / (double)max_width));
                }
                bar_x_max += bar_x_min;

                /* Is ButtonPress outside of bar? */
                if(((event->xbutton.button == Button1) ||
                    (event->xbutton.button == Button3)
                   )  &&
                   ((event->xbutton.x < bar_x_min) ||
                    (event->xbutton.x > bar_x_max)
                   )
                )
                {
                    /* Button press is outside of bar, move one page. */
                    if(event->xbutton.x < bar_x_min)
                    {
                        sb->x_win_pos -= (int)width;
                        /* Set repeat record. */
                        SBarRepeatRecordSet(sb,
                            widget_global.sb_repeat_delay,
                            SB_OP_CODE_PAGE_LEFT,
                            width, height, max_width, max_height
                        );
                    }
                    else if(event->xbutton.x > bar_x_max) 
                    {
                        sb->x_win_pos += (int)width;
                        /* Set repeat record. */
                        SBarRepeatRecordSet(sb,
                            widget_global.sb_repeat_delay,
                            SB_OP_CODE_PAGE_RIGHT,
                            width, height, max_width, max_height
                        );
                    }
                }
                else
                {
                    /* Button press is inside of bar, move to pointer. */

		    /* Record button state and grab pointer. */
		    sb->x_bar_button_state = True;

		    OSWGrabPointer(
			sb->x_bar,
			True,
			ButtonReleaseMask | PointerMotionMask,
			GrabModeAsync, GrabModeAsync,
			sb->x_bar,
			None
		    );

		    /* Scroll to position. */
                    OSWGetWindowAttributes(sb->x_bar, &wattr);
                    if(wattr.width > 0)
                    {
                        if(event->xbutton.button == Button1)
                            sb->x_origin_delta = static_cast<int>(sb->x_win_pos - (
                                ((double)max_width / (double)wattr.width)
                                * event->xbutton.x) + ((int)width / 2));
                        else
                            sb->x_origin_delta = 0;

                        if(event->xbutton.button != Button1)
                            sb->x_win_pos = static_cast<int>((double)event->xbutton.x /
                                (double)wattr.width * (double)max_width);
                    }
                    else
                    {
                        sb->x_win_pos = 0;
                    }
                    if(event->xbutton.button != Button1)
                        sb->x_win_pos -= (((int)width / 2) -
                            sb->x_origin_delta);
		}

                /* Sanitize. */
                if(sb->x_win_pos > max_x_pos)
                    sb->x_win_pos = max_x_pos;  
                else if(sb->x_win_pos < 0)
                    sb->x_win_pos = 0;
                            

                events_handled++;
	    }

            /* y bar. */
            else if((event->xany.window == sb->y_bar) &&
                    !sb->y_bar_button_state
            )
            {
		/* Calculate bar min and max. */
		if(x_bar_vis)
		{
		    bar_y_min = static_cast<int>(sb->y_win_pos *
                        (((double)height -
                        (2 * SCROLLBAR_CURSOR_BTN_HEIGHT) -
                        SCROLLBAR_XBAR_HEIGHT - 4) / (double)max_height));
		    bar_y_max = static_cast<int>((double)height *
                        (((double)height -
                        (2 * SCROLLBAR_CURSOR_BTN_HEIGHT) -
                        SCROLLBAR_XBAR_HEIGHT - 4) / (double)max_height));
		}
		else
		{
                    bar_y_min = static_cast<int>(sb->y_win_pos *
                        (((double)height -
                        (2 * SCROLLBAR_CURSOR_BTN_HEIGHT) - 4)
                        / (double)max_height));
                    bar_y_max = static_cast<int>((double)height *
                        (((double)height -
                        (2 * SCROLLBAR_CURSOR_BTN_HEIGHT) - 4)
                        / (double)max_height));
		}
		bar_y_max += bar_y_min;

		/* ButtonPress is outside of bar? */
		if(((event->xbutton.button == Button1) ||
                    (event->xbutton.button == Button3)
                   )  &&
                   ((event->xbutton.y < bar_y_min) ||
                    (event->xbutton.y > bar_y_max)
		   )
		)
		{
		    /* Button press is outside of bar, move one page. */
		    if(event->xbutton.y < bar_y_min)
		    {
			sb->y_win_pos -= (int)height;
                        /* Set repeat record. */
                        SBarRepeatRecordSet(sb,
                            widget_global.sb_repeat_delay,
                            SB_OP_CODE_PAGE_UP,
                            width, height, max_width, max_height
                        );
		    }
		    else if(event->xbutton.y > bar_y_max)
		    {
			sb->y_win_pos += (int)height;
                        /* Set repeat record. */
                        SBarRepeatRecordSet(sb,
                            widget_global.sb_repeat_delay,
                            SB_OP_CODE_PAGE_DOWN,
                            width, height, max_width, max_height
                        );
		    }
		}
		else
                {
                    /* Button press is inside of bar, move to pointer. */

                    /* Record button state and grab pointer. */
                    sb->y_bar_button_state = True;
                 
                    OSWGrabPointer(
                        sb->y_bar,
                        True,
                        ButtonReleaseMask | PointerMotionMask,
                        GrabModeAsync, GrabModeAsync,
                        sb->y_bar,
                        None
                    );

                    /* Scroll to position. */
                    OSWGetWindowAttributes(sb->y_bar, &wattr);
                    if(wattr.height > 0)
                    {
                        if(event->xbutton.button == Button1)
                            sb->y_origin_delta = static_cast<int>(sb->y_win_pos - (
                                ((double)max_height / (double)wattr.height)
                                * event->xbutton.y) + ((int)height / 2));
                        else
                            sb->y_origin_delta = 0;

			if(event->xbutton.button != Button1)
                            sb->y_win_pos = static_cast<int>((double)event->xbutton.y /
                                (double)wattr.height * (double)max_height);
                    }
                    else
                    {
                        sb->y_win_pos = 0;
                    }
		    if(event->xbutton.button != Button1)
                        sb->y_win_pos -= (((int)height / 2) -
			    sb->y_origin_delta);
		}

		/* Sanitize. */
		if(sb->y_win_pos > max_y_pos)
		    sb->y_win_pos = max_y_pos;
		else if(sb->y_win_pos < 0)
		    sb->y_win_pos = 0;

                events_handled++;
            }

	    /* Up cursor. */
	    else if(event->xany.window == sb->y_up)
	    {
		switch(event->xbutton.button)
		{
                  case Button3:
                    sb->y_win_pos -= height;
                    if(sb->y_win_pos < 0) 
                        sb->y_win_pos = 0;
		    /* Set repeat record. */
                    SBarRepeatRecordSet(sb,
                        widget_global.sb_repeat_delay,
                        SB_OP_CODE_PAGE_UP,
                        width, height, max_width, max_height
                    );
		    break;

                  case Button2:
                    sb->y_win_pos = 0;
                    break;

		  default:
                    sb->y_win_pos -= SCROLLBAR_CURSOR_INC;
                    if(sb->y_win_pos < 0)
                        sb->y_win_pos = 0;
                    /* Set repeat record. */
                    SBarRepeatRecordSet(sb,
			widget_global.sb_repeat_delay,
                        SB_OP_CODE_SCROLL_UP,
			width, height, max_width, max_height
		    );
		    break;
		}

                events_handled++;
	    }
            /* Down cursor. */
            else if(event->xany.window == sb->y_down)
            {
                switch(event->xbutton.button)
                {
                  case Button3:
                    sb->y_win_pos += height;
                    if(sb->y_win_pos > max_y_pos) 
                        sb->y_win_pos = max_y_pos;
		    /* Set repeat record. */
                    SBarRepeatRecordSet(sb,
                        widget_global.sb_repeat_delay,
                        SB_OP_CODE_PAGE_DOWN,
                        width, height, max_width, max_height
                    );
                    break;

                  case Button2:
                    sb->y_win_pos = max_y_pos;
                    break;

		  default:
                    sb->y_win_pos += SCROLLBAR_CURSOR_INC;
                    if(sb->y_win_pos > max_y_pos)
                        sb->y_win_pos = max_y_pos;
                    /* Set repeat record. */
                    SBarRepeatRecordSet(sb,
			widget_global.sb_repeat_delay,
                        SB_OP_CODE_SCROLL_DOWN,
                        width, height, max_width, max_height
		    );
		    break;
		}

                events_handled++;
            }
	    /* Left cursor. */
	    else if(event->xany.window == sb->x_left)
	    {
                switch(event->xbutton.button)
                {
                  case Button3:
                    sb->x_win_pos -= width;
                    if(sb->x_win_pos < 0)
                        sb->x_win_pos = 0;
                    /* Set repeat record. */
                    SBarRepeatRecordSet(sb,
                        widget_global.sb_repeat_delay,
                        SB_OP_CODE_PAGE_LEFT,
                        width, height, max_width, max_height
                    );
		    break;

                  case Button2:
                    sb->x_win_pos = 0;
                    break;

                  default:
                    sb->x_win_pos -= SCROLLBAR_CURSOR_INC;
                    if(sb->x_win_pos < 0)
                        sb->x_win_pos = 0;
                    /* Set repeat record. */
                    SBarRepeatRecordSet(sb,
			widget_global.sb_repeat_delay,
                        SB_OP_CODE_SCROLL_LEFT,
                        width, height, max_width, max_height
		    );
		    break;
		}

                events_handled++;
	    }
            /* Right cursor. */
            else if(event->xany.window == sb->x_right)
            {
                switch(event->xbutton.button)
                {
                  case Button3:
                    sb->x_win_pos += height;
                    if(sb->x_win_pos > max_x_pos) 
                        sb->x_win_pos = max_x_pos;
                    /* Set repeat record. */
                    SBarRepeatRecordSet(sb,
                        widget_global.sb_repeat_delay,
                        SB_OP_CODE_PAGE_RIGHT,
                        width, height, max_width, max_height
                    );
                    break;

                  case Button2:
                    sb->x_win_pos = max_x_pos;
                    break;

                  default:
                    sb->x_win_pos += SCROLLBAR_CURSOR_INC;
                    if(sb->x_win_pos > max_x_pos)
                        sb->x_win_pos = max_x_pos;
                    /* Set repeat record. */
                    SBarRepeatRecordSet(sb,
			widget_global.sb_repeat_delay,
                        SB_OP_CODE_SCROLL_RIGHT,
                        width, height, max_width, max_height
		    );
		    break;
 		}

                events_handled++;
            }
            break;

	  /* ****************************************************** */
	  case ButtonRelease:
	    if((event->xany.window == sb->x_bar) ||
               (event->xany.window == sb->y_bar)
	    )
	    {
	        sb->x_bar_button_state = False;
                sb->y_bar_button_state = False;

		events_handled++;
	    }
	    else if((event->xany.window == sb->x_left) ||
                    (event->xany.window == sb->x_right) ||
                    (event->xany.window == sb->y_up) ||
                    (event->xany.window == sb->y_down)
	    )
	    {
		events_handled++;
	    }
	    /* Stop any scroll bar repeats. */
	    SBarRepeatRecordClear();

	    /* Ungrab pointer. */
	    OSWUngrabPointer();

	    break;

          /* ******************************************************* */
          case VisibilityNotify:
            if((event->xany.window == sb->x_toplevel) ||
               (event->xany.window == sb->y_toplevel)
	    )
            {
                sb->visibility_state = event->xvisibility.state;
                events_handled++;
                
                /* No need to continue, just return. */
                return(events_handled);
            }
            break;
	}


	/* ********************************************************* */

	/* Redraw scroll bars if needed. */
	if(events_handled > 0)
	{
	    SBarDraw(
                sb,
                ori_width, ori_height,
                ori_max_width, ori_max_height
	    );
	}


	return(events_handled);
}


/*
 *	Manages scroll bar repeats.
 */
int SBarManageRepeat(event_t *event)
{
	int i, max_x_pos, max_y_pos;
	int events_handled = 0;
	scroll_bar_struct *sb;

	list_window_struct *lw_ptr;
	colum_list_struct *cl_ptr;
	fbrowser_struct *fb_ptr;
	file_viewer_struct *fv_ptr;


        /*
         *   Note: The event information is not used and not needed here.
         */

        /* Is a scroll bar pointer set in the record? */
        if(sb_repeat_record[0].sb != NULL)
        {
	    sb = sb_repeat_record[0].sb;

	    /* Time for next repeat? */
            if(MilliTime() >= sb_repeat_record[0].next_repeat)
            {
                /* Calculate max_x_pos and max_y_pos. */
                max_x_pos = (int)sb_repeat_record[0].max_width -
		    (int)sb_repeat_record[0].width;
                max_y_pos = (int)sb_repeat_record[0].max_height -
		    (int)sb_repeat_record[0].height;


		/* Handle scroll bar operation. */
		switch(sb_repeat_record[0].sb_op_code)
		{
		  case SB_OP_CODE_SCROLL_UP:
                    sb->y_win_pos -= SCROLLBAR_CURSOR_INC;
		    break;

                  case SB_OP_CODE_SCROLL_DOWN:
                    sb->y_win_pos += SCROLLBAR_CURSOR_INC;
                    break;

                  case SB_OP_CODE_SCROLL_LEFT:
                    sb->x_win_pos -= SCROLLBAR_CURSOR_INC;
                    break;

                  case SB_OP_CODE_SCROLL_RIGHT:
                    sb->x_win_pos += SCROLLBAR_CURSOR_INC;
                    break;


		  case SB_OP_CODE_PAGE_UP:
                    sb->y_win_pos -= (int)sb_repeat_record[0].height;
                    break;

                  case SB_OP_CODE_PAGE_DOWN:
                    sb->y_win_pos += (int)sb_repeat_record[0].height;
                    break;

                  case SB_OP_CODE_PAGE_LEFT:
                    sb->x_win_pos -= (int)sb_repeat_record[0].width;
                    break;

                  case SB_OP_CODE_PAGE_RIGHT:
                    sb->x_win_pos += (int)sb_repeat_record[0].width;
                    break;
		}

		/* Sanitize values. */
                if(sb->x_win_pos > max_x_pos)
                    sb->x_win_pos = max_x_pos;
                if(sb->y_win_pos > max_y_pos)
                    sb->y_win_pos = max_y_pos;

                if(sb->x_win_pos < 0)
                    sb->x_win_pos = 0;
                if(sb->y_win_pos < 0)
                    sb->y_win_pos = 0;

		/* Redraw it. */
		SBarDraw(
		    sb,
		    sb_repeat_record[0].width, sb_repeat_record[0].height,
		    sb_repeat_record[0].max_width,
		    sb_repeat_record[0].max_height
		);

		/* ***************************************************** */
		/* Call scroll bar notify function. */
	        if(sb_repeat_record[0].func_notify != NULL)
		    sb_repeat_record[0].func_notify(sb);

		/* Redraw local widgets listed in reg that have scroll bars. */
                for(i = 0; i < widget_reg.total_entries; i++)
                {
                    if(widget_reg.entry[i] == NULL) continue;  

		    switch(widget_reg.entry[i]->type)
		    {
		      case WTYPE_CODE_COLUMLIST:
			cl_ptr = (colum_list_struct *)widget_reg.entry[i]->ptr;
			if(&cl_ptr->sb == sb)
			    CListDraw(cl_ptr, CL_DRAW_AMOUNT_LIST);
			break;

		      case WTYPE_CODE_LIST:
			lw_ptr = (list_window_struct *)widget_reg.entry[i]->ptr;
			if(&lw_ptr->sb == sb)
			    ListWinDraw(lw_ptr);
			break;

		      case WTYPE_CODE_FILEBROWSER:
			fb_ptr = (fbrowser_struct *)widget_reg.entry[i]->ptr;
			if(&fb_ptr->dir_win_sb == sb)
			    FBrowserDraw(fb_ptr, FBROWSER_DRAW_DIRLIST);
                        if(&fb_ptr->file_win_sb == sb)
                            FBrowserDraw(fb_ptr, FBROWSER_DRAW_FILELIST);
                        if(&fb_ptr->list_win_sb == sb)
                            FBrowserDraw(fb_ptr, FBROWSER_DRAW_ALLLISTS);
			break;

                      case WTYPE_CODE_VIEWER:
                        fv_ptr = (file_viewer_struct *)widget_reg.entry[i]->ptr;
                        if(&fv_ptr->sb == sb)  
                            ViewerDraw(fv_ptr);
                        break;
		    }
                }


		/* Schedual next operation. */
		sb_repeat_record[0].next_repeat =
		    MilliTime() + widget_global.sb_repeat_interval;


                events_handled++;
            }
        }


	return(events_handled);
}


/*
 *	Destroys scroll bar.
 */
void SBarDestroy(scroll_bar_struct *sb)
{
	if(sb == NULL)
	    return;


        /* Delete widget from regeristry. */
        WidgetRegDelete(sb);


	/* Clear repeat records that might referance this widget. */
	SBarRepeatRecordClear();


	if(IDC())
	{
	    /* Destroy the scroll bar windows. */
	    OSWDestroyPixmap(&sb->x_left_buf);
	    OSWDestroyWindow(&sb->x_left);
            OSWDestroyPixmap(&sb->x_bar_buf);
            OSWDestroyWindow(&sb->x_bar);
            OSWDestroyPixmap(&sb->x_right_buf);
            OSWDestroyWindow(&sb->x_right);
	    sb->x_bar_button_state = False;

            OSWDestroyPixmap(&sb->y_up_buf);
            OSWDestroyWindow(&sb->y_up);
            OSWDestroyPixmap(&sb->y_bar_buf);
            OSWDestroyWindow(&sb->y_bar);
            OSWDestroyPixmap(&sb->y_down_buf);
            OSWDestroyWindow(&sb->y_down);
	    sb->y_bar_button_state = False;

            OSWDestroyWindow(&sb->x_toplevel);
            OSWDestroyWindow(&sb->y_toplevel);
	}


	sb->map_state = 0;
	sb->visibility_state = VisibilityFullyObscured;
	sb->is_in_focus = 0;
	sb->x = 0;
	sb->y = 0;
	sb->width = 0;
	sb->height = 0;

	sb->x_origin_delta = 0;
	sb->y_origin_delta = 0;

	sb->x_win_pos = 0;
	sb->y_win_pos = 0;


	return;
}




