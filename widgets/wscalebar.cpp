// widgets/wscalebar.cpp
/*
                        Widget: Scale Bar

	Functions:

	int ScaleBarInit(
	        scale_bar_struct *sb,
	        win_t parent,
	        int x, int y,
	        unsigned int length,
                int style,
                int ticks,
	        int orientation,
	        double pos_min, double pos_max, double pos,
	        bool_t flip_pos,
	        void *client_data,
	        int (*func_cb)(void *)
	)
	int ScaleBarDraw(scale_bar_struct *sb)
	int ScaleBarManage(
		scale_bar_struct *sb,
		event_t *xevent
	)
	void ScaleBarMap(scale_bar_struct *sb)
	void ScaleBarUnmap(scale_bar_struct *sb)
	void ScaleBarDestroy(scale_bar_struct *sb)

	---


 */

#include "../include/widget.h"


/*
 *	Size constants (in pixels):
 */
#define SCALEBAR_BAR_MIN_LEN			10
#define SCALEBAR_FLUSHED_HANDLE_LENGTH		36


/*
 *	Initializes scale bar.
 */
int ScaleBarInit(
        scale_bar_struct *sb,
        win_t parent,
        int x, int y,
        unsigned int length,
        int style,
        int ticks,
        int orientation,
        double pos_min, double pos_max, double pos,
        bool_t flip_pos,
	void *client_data,
	int (*func_cb)(void *)
)
{
	int width, height;
	win_attr_t wattr;


        if((sb == NULL) ||
           (parent == 0)
	)
            return(-1);


	/* Reset values. */
        sb->map_state = 0;
        sb->visibility_state = VisibilityFullyObscured;
        sb->is_in_focus = 0;
	sb->font = widget_global.scale_bar_font;
        sb->next = NULL;
        sb->prev = NULL;

	sb->style = style;
        sb->ticks = ticks;

	/* Sanitize and set positions. */
	if(pos_min > pos_max)
	    pos_min = pos_max;
	sb->pos_min = pos_min;
	sb->pos_max = pos_max;
	if(pos < pos_min)
	    pos = pos_min;
	if(pos > pos_max)
	    pos = pos_max;
	sb->pos = pos;

        sb->orientation = orientation;
        sb->flip_pos = flip_pos;

        if((int)length < SCALEBAR_BAR_MIN_LEN)
            length = SCALEBAR_BAR_MIN_LEN;
        sb->length = length;

	sb->btn_state = False;

	sb->client_data = client_data;
	sb->func_cb = func_cb;


	/* Calculate width and height. */
	if(style == SCALEBAR_STYLE_STANDARD_VALUE)
	{
            switch(orientation)
            {
              case SCALEBAR_ORIENT_VERTICAL:
                width = SCALEBAR_BAR_STANDARD_WIDTH;
                height = length;
                break;

              default:  /* SCALEBAR_ORIENT_HORIZONTAL */
                width = length;
                height = SCALEBAR_BAR_STANDARD_WIDTH;
                break;
            }
	}
	else if(style == SCALEBAR_STYLE_FLUSHED)
	{
	    switch(orientation)
	    {
	      case SCALEBAR_ORIENT_VERTICAL:
                width = SCALEBAR_BAR_FLUSHED_WIDTH;
                height = length;
                break;

	      default:	/* SCALEBAR_ORIENT_HORIZONTAL */
                width = length;
                height = SCALEBAR_BAR_FLUSHED_WIDTH;
                break;
	    }
	}
	else
	{
            switch(orientation)
            {
              case SCALEBAR_ORIENT_VERTICAL:
                width = SCALEBAR_BAR_STANDARD_WIDTH;
                height = length;
                break;
          
              default:  /* SCALEBAR_ORIENT_HORIZONTAL */
                width = length;
                height = SCALEBAR_BAR_STANDARD_WIDTH;
                break;
            }
	}


        /* Toplevel. */
	if(
	    OSWCreateWindow(
	        &sb->toplevel,
                parent,
                x, y,
                width, height
	    )
        )
	    return(-1);

	sb->toplevel_buf = 0;

	OSWSetWindowInput(
	    sb->toplevel,
	    KeyPressMask | KeyReleaseMask |
	    ButtonPressMask | ButtonReleaseMask | PointerMotionMask |
            ExposureMask | VisibilityChangeMask
        );
        OSWSetWindowCursor(sb->toplevel, osw_gui[0].std_cursor);

	OSWGetWindowAttributes(sb->toplevel, &wattr);
	sb->x = wattr.x;
	sb->y = wattr.y;
	sb->width = wattr.width;
	sb->height = wattr.height;


        /* Add this widget to the regeristry. */
        WidgetRegAdd(sb, WTYPE_CODE_SCALEBAR);


	return(0);
}


/*
 *	Draw scale bar.
 */
int ScaleBarDraw(scale_bar_struct *sb)
{
	int m, x, y;
	double pos, pos_len;
        double handle_pos;
	int tick_spacing;
	image_t *image;
	win_attr_t wattr;
        font_t *prev_font;
	char stringa[256];


        if(sb == NULL)
	    return(-1);


	/* Record previous font. */
        prev_font = OSWQueryCurrentFont();
        OSWSetFont(sb->font);  


	OSWGetWindowAttributes(sb->toplevel, &wattr);

        /* Recreate buffers as needed. */
        if(sb->toplevel_buf == 0)
        {
            if(
                OSWCreatePixmap(
                    &sb->toplevel_buf,
                    wattr.width, wattr.height
                )
            )
                return(-1);
        }   
        if(sb->bkg_buf == 0)
        {
            if(
                OSWCreatePixmap(
                    &sb->bkg_buf,
                    wattr.width, wattr.height
                )
            )
                return(-1);
        }


	/* Map as needed. */
	if(!sb->map_state)
	{
	    /* Redraw background. */
            if(sb->style == SCALEBAR_STYLE_FLUSHED)
            {
                /* Draw background. */
                OSWClearPixmap(sb->bkg_buf,
                    wattr.width, wattr.height,
                    (widget_global.force_mono) ? osw_gui[0].black_pix :
                    widget_global.scroll_bkg_pix
                );
                if(widget_global.force_mono)
                    OSWSetFgPix(osw_gui[0].white_pix);
                else
                    OSWSetFgPix(widget_global.scroll_frame_pix);
		/* Frame. */
                OSWDrawRectangle(
                    sb->bkg_buf,
                    0, 0,
                    (int)wattr.width - 1, (int)wattr.height - 1
                );
            }
            else /* SCALEBAR_STYLE_STANDARD or SCALEBAR_STYLE_STANDARD_VALUE */
            {
		/* Draw background. */
		if(widget_global.force_mono)
                    OSWClearPixmap(sb->bkg_buf,
                        wattr.width, wattr.height,  
                        osw_gui[0].black_pix
                    );
		else
                    WidgetPutImageTile(
		        sb->bkg_buf,
		        widget_global.std_bkg_img,
		        wattr.width, wattr.height
		    );

		/* Slide channel and ticks. */
                switch(sb->orientation)
                {
                  case SCALEBAR_ORIENT_VERTICAL:
                    image = widget_global.scalebar_v_img;

                    if(widget_global.force_mono)
		    {
                        OSWSetFgPix(osw_gui[0].white_pix);
                        OSWDrawRectangle(sb->bkg_buf,
                            (SCALEBAR_BAR_STANDARD_WIDTH / 2) - 5,
                            6,
                            10,
                            (int)wattr.height - 12
                        );
		    }
		    else if(image != NULL)
                    {
                        OSWSetFgPix(widget_global.surface_editable_pix);
                        OSWDrawSolidRectangle(sb->bkg_buf,
                            (SCALEBAR_BAR_STANDARD_WIDTH / 2) - 5,
                            (int)image->height / 2,
                            10,
                            (int)wattr.height - (int)image->height
			);
                        OSWSetFgPix(widget_global.surface_shadow_pix);
			/* Top horizontal. */
                        OSWDrawLine(sb->bkg_buf,
                            (SCALEBAR_BAR_STANDARD_WIDTH / 2) - 5,
                            ((int)image->height / 2),
                            (SCALEBAR_BAR_STANDARD_WIDTH / 2) + 5,
                            ((int)image->height / 2)
                        );
                        OSWDrawLine(sb->bkg_buf,
                            (SCALEBAR_BAR_STANDARD_WIDTH / 2) - 4,
                            ((int)image->height / 2) + 1,
                            (SCALEBAR_BAR_STANDARD_WIDTH / 2) + 4,
                            ((int)image->height / 2) + 1
                        );
                        /* Left vertical. */
                        OSWDrawLine(sb->bkg_buf,
                            (SCALEBAR_BAR_STANDARD_WIDTH / 2) - 5,
                            ((int)image->height / 2),
                            (SCALEBAR_BAR_STANDARD_WIDTH / 2) - 5,
                            ((int)wattr.height - ((int)image->height / 2))
                        );
                        OSWDrawLine(sb->bkg_buf,
                            (SCALEBAR_BAR_STANDARD_WIDTH / 2) - 4,
                            ((int)image->height / 2) + 1,
                            (SCALEBAR_BAR_STANDARD_WIDTH / 2) - 4,
                            ((int)wattr.height - ((int)image->height / 2)) - 1
                        );
                        OSWSetFgPix(widget_global.surface_highlight_pix);
                        /* Bottom horizontal. */
                        OSWDrawLine(sb->bkg_buf,
                            (SCALEBAR_BAR_STANDARD_WIDTH / 2) - 4,
                            ((int)wattr.height - ((int)image->height / 2)),
                            (SCALEBAR_BAR_STANDARD_WIDTH / 2) + 5,
                            ((int)wattr.height - ((int)image->height / 2))
                        );
                        OSWDrawLine(sb->bkg_buf,
                            (SCALEBAR_BAR_STANDARD_WIDTH / 2) - 3,
                            ((int)wattr.height - ((int)image->height / 2)) + 1,
                            (SCALEBAR_BAR_STANDARD_WIDTH / 2) + 4,
                            ((int)wattr.height - ((int)image->height / 2)) + 1
                        );
                        /* Right vertical. */
                        OSWDrawLine(sb->bkg_buf,
                            (SCALEBAR_BAR_STANDARD_WIDTH / 2) + 5,
                            ((int)image->height / 2) + 1,
                            (SCALEBAR_BAR_STANDARD_WIDTH / 2) + 5,
                            ((int)wattr.height - ((int)image->height / 2))
                        );
                        OSWDrawLine(sb->bkg_buf,
                            (SCALEBAR_BAR_STANDARD_WIDTH / 2) + 4,
                            ((int)image->height / 2) + 2,
                            (SCALEBAR_BAR_STANDARD_WIDTH / 2) + 4,
                            ((int)wattr.height - ((int)image->height / 2)) - 1 
                        );

			/* Change color for drawing ticks. */
			OSWSetFgPix(widget_global.normal_text_pix);
                    }

		    /* Ticks. */
		    if((sb->ticks >= 2) && (image != NULL))
		    {
		        for(y = (int)image->height / 2,
                            tick_spacing =
                                ((int)wattr.height - (int)image->height) /
                                (sb->ticks - 1),
                            m = (int)wattr.height -
                                ((int)image->height / 2);
                            y <= m;
                            y += tick_spacing
			)
		        {
                            OSWDrawLine(sb->bkg_buf,
                                0, y,
                                4, y
			    );
                            OSWDrawLine(sb->bkg_buf,
                                (int)wattr.width - 5, y,
                                (int)wattr.width - 1, y
                            );
			}
		    }
                    break;
             
                  default:      /* SCALEBAR_ORIENT_HORIZONTAL */
                    image = widget_global.scalebar_h_img;

                    if(widget_global.force_mono)
                    {
                        OSWSetFgPix(osw_gui[0].white_pix);
                        OSWDrawRectangle(sb->bkg_buf,
                            6,
                            (SCALEBAR_BAR_STANDARD_WIDTH / 2) - 5,
                            (int)wattr.width - 12,
                            10
                        );
                    }
                    else if(image != NULL)
                    {
                        OSWSetFgPix(widget_global.surface_editable_pix);
                        OSWDrawSolidRectangle(sb->bkg_buf,
                            (int)image->width / 2,
                            (SCALEBAR_BAR_STANDARD_WIDTH / 2) - 5,
                            (int)wattr.width - (int)image->width,
                            10
                        );
                        OSWSetFgPix(widget_global.surface_shadow_pix);
                        /* Top horizontal. */
                        OSWDrawLine(sb->bkg_buf,
                            ((int)image->width / 2),
                            (SCALEBAR_BAR_STANDARD_WIDTH / 2) - 5,
                            ((int)wattr.width - ((int)image->width / 2)) - 1,
                            (SCALEBAR_BAR_STANDARD_WIDTH / 2) - 5
                        );
                        OSWDrawLine(sb->bkg_buf,
                            ((int)image->width / 2) + 1,
                            (SCALEBAR_BAR_STANDARD_WIDTH / 2) - 4,
                            ((int)wattr.width - ((int)image->width / 2)) - 2,
                            (SCALEBAR_BAR_STANDARD_WIDTH / 2) - 4
                        );
                        /* Left vertical. */
                        OSWDrawLine(sb->bkg_buf,
                            ((int)image->width / 2),
                            (SCALEBAR_BAR_STANDARD_WIDTH / 2) - 5,
                            ((int)image->width / 2),
                            (SCALEBAR_BAR_STANDARD_WIDTH / 2) + 5
                        );
                        OSWDrawLine(sb->bkg_buf,
                            ((int)image->width / 2) + 1,
                            (SCALEBAR_BAR_STANDARD_WIDTH / 2) - 4,
                            ((int)image->width / 2) + 1,
                            (SCALEBAR_BAR_STANDARD_WIDTH / 2) + 4
                        );

                        OSWSetFgPix(widget_global.surface_highlight_pix);
                        /* Bottom horizontal. */
                        OSWDrawLine(sb->bkg_buf,  
                            ((int)image->width / 2) + 1,
                            (SCALEBAR_BAR_STANDARD_WIDTH / 2) + 5,
                            ((int)wattr.width - ((int)image->width / 2)) - 1,
                            (SCALEBAR_BAR_STANDARD_WIDTH / 2) + 5
                        );
                        OSWDrawLine(sb->bkg_buf,
                            ((int)image->width / 2) + 2,
                            (SCALEBAR_BAR_STANDARD_WIDTH / 2) + 4,
                            ((int)wattr.width - ((int)image->width / 2)) - 2,
                            (SCALEBAR_BAR_STANDARD_WIDTH / 2) + 4
                        );
                        /* Right vertical. */
                        OSWDrawLine(sb->bkg_buf,
                            ((int)wattr.width - ((int)image->width / 2)) - 1,
                            (SCALEBAR_BAR_STANDARD_WIDTH / 2) - 4,
                            ((int)wattr.width - ((int)image->width / 2)) - 1,
                            (SCALEBAR_BAR_STANDARD_WIDTH / 2) + 5
                        );
                        OSWDrawLine(sb->bkg_buf,
                            ((int)wattr.width - ((int)image->width / 2)) - 2,
                            (SCALEBAR_BAR_STANDARD_WIDTH / 2) - 3,
                            ((int)wattr.width - ((int)image->width / 2)) - 2,
                            (SCALEBAR_BAR_STANDARD_WIDTH / 2) + 4
                        );

                        /* Change color for drawing ticks. */
                        OSWSetFgPix(widget_global.normal_text_pix);
		    }
                    /* Ticks. */
                    if((sb->ticks >= 2) && (image != NULL))
                    {
                        for(x = (int)image->width / 2,
                            tick_spacing =
                                ((int)wattr.width - (int)image->width) /
                                (sb->ticks - 1),
                            m = (int)wattr.width -
                                ((int)image->width / 2);
                            x <= m;
                            x += tick_spacing
                        )
                        {
                            OSWDrawLine(sb->bkg_buf,
                                x, 0,
                                x, 4
                            );
                            OSWDrawLine(sb->bkg_buf,
                                x, (int)wattr.height - 5,
                                x, (int)wattr.height - 1
                            );
                        }
                    }
                    break;
                }
            }


	    OSWMapRaised(sb->toplevel);

	    sb->map_state = 1;
	    sb->visibility_state = VisibilityUnobscured;
	}


	/* Clear toplevel buffer. */
	OSWCopyDrawables(
	    sb->toplevel_buf,
	    sb->bkg_buf,
            wattr.width,
	    wattr.height
	);


	/* Flushed style. */
        if(sb->style == SCALEBAR_STYLE_FLUSHED)
        {
            /* Calculate positions. */
            pos = sb->pos;
            pos_len = sb->pos_max - sb->pos_min;
	    if(pos_len == 0) pos_len = 1;

            if(sb->flip_pos)
                pos = pos_len - pos;

            switch(sb->orientation)
            {
              case SCALEBAR_ORIENT_VERTICAL:
                handle_pos = (pos / pos_len) *
                    ((int)wattr.height - SCALEBAR_FLUSHED_HANDLE_LENGTH);

                if(widget_global.force_mono)
                    OSWSetFgPix(osw_gui[0].white_pix);
                else
                    OSWSetFgPix(widget_global.scroll_bar_pix);
                OSWDrawSolidRectangle(
                    sb->toplevel_buf,
                    1, static_cast<int>(handle_pos),
                    (int)wattr.width - 2, SCALEBAR_FLUSHED_HANDLE_LENGTH
                );

                if(widget_global.force_mono)
                    OSWSetFgPix(osw_gui[0].black_pix);
                else
                    OSWSetFgPix(widget_global.scroll_frame_pix);
                OSWDrawLine(
                    sb->toplevel_buf,
                    1, static_cast<int>(handle_pos),
                    (int)wattr.width - 2, static_cast<int>(handle_pos)
                );
                OSWDrawLine(
                    sb->toplevel_buf,
                    1, static_cast<int>(handle_pos + SCALEBAR_FLUSHED_HANDLE_LENGTH),
                    (int)wattr.width - 2, static_cast<int>(handle_pos + SCALEBAR_FLUSHED_HANDLE_LENGTH)
                );

                if(widget_global.force_mono)
                    OSWSetFgPix(osw_gui[0].black_pix);
                else
                    OSWSetFgPix(widget_global.scroll_bkg_pix);
                OSWDrawLine(
                    sb->toplevel_buf,
                    1, static_cast<int>(handle_pos + (SCALEBAR_FLUSHED_HANDLE_LENGTH / 2)),
                    (int)wattr.width - 2,
                    static_cast<int>(handle_pos + (SCALEBAR_FLUSHED_HANDLE_LENGTH / 2))
                );
                break;

              default:      /* SCALEBAR_ORIENT_HORIZONTAL */
                handle_pos = (pos / pos_len) *
                    ((int)wattr.width - SCALEBAR_FLUSHED_HANDLE_LENGTH);

                if(widget_global.force_mono)
                    OSWSetFgPix(osw_gui[0].white_pix);
                else
                    OSWSetFgPix(widget_global.scroll_bar_pix);
                OSWDrawSolidRectangle(
                    sb->toplevel_buf,
                    static_cast<int>(handle_pos), 1,
                    SCALEBAR_FLUSHED_HANDLE_LENGTH, (int)wattr.height - 2
                );

                if(widget_global.force_mono)
                    OSWSetFgPix(osw_gui[0].black_pix);
                else
                    OSWSetFgPix(widget_global.scroll_frame_pix);
                OSWDrawLine(
                    sb->toplevel_buf,
                    static_cast<int>(handle_pos), 1,
                    static_cast<int>(handle_pos), (int)wattr.height - 2
                );
                OSWDrawLine(
                    sb->toplevel_buf,
                    static_cast<int>(handle_pos + SCALEBAR_FLUSHED_HANDLE_LENGTH), 1,
                    static_cast<int>(handle_pos + SCALEBAR_FLUSHED_HANDLE_LENGTH), (int)wattr.height - 2
                );

                if(widget_global.force_mono)
                    OSWSetFgPix(osw_gui[0].black_pix);
                else
                    OSWSetFgPix(widget_global.scroll_bkg_pix);
                OSWDrawLine(
                    sb->toplevel_buf,
                    static_cast<int>(handle_pos + (SCALEBAR_FLUSHED_HANDLE_LENGTH / 2)), 1,
                    static_cast<int>(handle_pos + (SCALEBAR_FLUSHED_HANDLE_LENGTH / 2)), (int)wattr.height - 2
		);
                break;  
            }
        }
	/* Standard style. */
        else	/* SCALEBAR_STYLE_STANDARD or SCALEBAR_STYLE_STANDARD_VALUE */
        {
            /* Calculate positions. */
            pos = sb->pos - sb->pos_min;
            pos_len = sb->pos_max - sb->pos_min;
            if(pos_len == 0) pos_len = 1;

            if(sb->flip_pos)
                pos = pos_len - pos;

            switch(sb->orientation)
            {
              case SCALEBAR_ORIENT_VERTICAL:
                image = widget_global.scalebar_v_img;
		if(image != NULL)
		{
                    handle_pos = (pos / pos_len) *
                        ((int)wattr.height - (int)image->height);
                    WidgetPutImageNormal(
			sb->toplevel_buf,
			image,
			((int)wattr.width / 2) - ((int)image->width / 2),
                        static_cast<int>(handle_pos),
			True
		    );
		}
                break;

              default:      /* SCALEBAR_ORIENT_HORIZONTAL */
                image = widget_global.scalebar_h_img;
                if(image != NULL)
                {
		    if(sb->style == SCALEBAR_STYLE_STANDARD_VALUE)
		    {
		        sprintf(stringa, "%.2f", pos);
		        if(widget_global.force_mono)
			    OSWSetFgPix(osw_gui[0].white_pix);
		        else
		            OSWSetFgPix(widget_global.normal_text_pix);
		        OSWDrawString(
			    sb->toplevel_buf,
			    ((pos / pos_len) > 0.5) ? 10 :
                                ((int)wattr.width / 2) + 6,
                            7,
			    stringa
		        );
		    }

                    handle_pos = (pos / pos_len) *
                        ((int)wattr.width - (int)image->width);
                    WidgetPutImageNormal(
                        sb->toplevel_buf,
                        image,
                        static_cast<int>(handle_pos),
                        ((int)wattr.height / 2) - ((int)image->height / 2),
                        True
                    );
                }
                break;
            }
        }


	OSWPutBufferToWindow(sb->toplevel, sb->toplevel_buf);

        OSWSetFont(prev_font);


	return(0);
}

/*
 *	Manage scale bar.
 */
int ScaleBarManage(
	scale_bar_struct *sb,
	event_t *event
)
{
	int x, y;
	double pos, pos_min, pos_max;
	int orientation, style;
        image_t *image;
        win_attr_t wattr;
	int events_handled = 0;


        if((sb == NULL) ||
	   (event == NULL)
	)
            return(events_handled);

	if(!sb->map_state &&
	   (event->type != MapNotify)
	)
	    return(0);


        switch(event->type)
        {
	  /* ********************************************************* */
	  case Expose:
	    if(event->xany.window == sb->toplevel)
	    {
		events_handled++;
	    }
	    break;

          /* ********************************************************* */
          case KeyPress:
	    break;

          /* ********************************************************* */
          case KeyRelease:
            break;

          /* ********************************************************* */
          case MotionNotify:
	    if((event->xany.window == sb->toplevel) &&
               (sb->btn_state == True)
	    )
	    {
                /* Get positions. */   
                pos_min = sb->pos_min;
                pos_max = sb->pos_max;
                if(pos_min > pos_max) pos_min = pos_max;

                pos = sb->pos;
                if(pos < pos_min) pos = pos_min;
                if(pos > pos_max) pos = pos_max;

                /* Get orientation. */
                orientation = sb->orientation;

		/* Get style. */
		style = sb->style;

		x = event->xmotion.x;
                y = event->xmotion.y;
		OSWGetWindowAttributes(sb->toplevel, &wattr);

		/* Handle by style. */
		if(style == SCALEBAR_STYLE_FLUSHED)
		{
                    /* Flushed style. */

                    switch(orientation)
                    {
                      case SCALEBAR_ORIENT_VERTICAL:
                        y -= ((SCALEBAR_FLUSHED_HANDLE_LENGTH / 2) + 1);
                        if(y < 0) y = 0;
                        if(y > ((int)wattr.height - SCALEBAR_FLUSHED_HANDLE_LENGTH - 3))
                            y = (int)wattr.height - SCALEBAR_FLUSHED_HANDLE_LENGTH - 3;
                        sb->pos = (pos_max - pos_min) *
                            ((double)y / (double)((int)wattr.height -
                            SCALEBAR_FLUSHED_HANDLE_LENGTH - 3));
                        break;

                      default:  /* SCALEBAR_ORIENT_HORIZONTAL */
                        x -= ((SCALEBAR_FLUSHED_HANDLE_LENGTH / 2) + 1);
                        if(x < 0) x = 0;
                        if(x > ((int)wattr.width - SCALEBAR_FLUSHED_HANDLE_LENGTH - 2))
                            x = (int)wattr.width - SCALEBAR_FLUSHED_HANDLE_LENGTH - 2;
                        sb->pos = (pos_max - pos_min) *
                            ((double)x / (double)((int)wattr.width -
                            SCALEBAR_FLUSHED_HANDLE_LENGTH - 2));
                        break;
                    }
		}
		else
		{
                    /* Standard style or standard value. */
        
                    switch(orientation)
                    {
                      case SCALEBAR_ORIENT_VERTICAL:
			image = widget_global.scalebar_v_img;
                        if(image != NULL)
			{
                            y -= (((int)image->height / 2) + 1);
                            if(y < 0) y = 0;
                            if(y > ((int)wattr.height - (int)image->height - 3))
                                y = (int)wattr.height - (int)image->height - 3;
                            sb->pos = (pos_max - pos_min) *
                                ((double)y / (double)((int)wattr.height -
                                (int)image->height - 3));
			}
                        break;
                     
                      default:  /* SCALEBAR_ORIENT_HORIZONTAL */
                        image = widget_global.scalebar_h_img;
                        if(image != NULL)
                        {
                            x -= (((int)image->width / 2) + 1);
                            if(x < 0) x = 0;
                            if(x > ((int)wattr.width - (int)image->width - 2))
                            x = (int)wattr.width - (int)image->width - 2;
                            sb->pos = (pos_max - pos_min) *
                                ((double)x / (double)((int)wattr.width -
                                (int)image->width - 2));
                        }
                        break;
                    }
		}

		/* Flip position? */
		if(sb->flip_pos)
		    sb->pos = (pos_max - pos_min) - sb->pos;

	        sb->pos += pos_min;

                /* Run callback function. */
                if(sb->func_cb != NULL)
                    sb->func_cb(sb->client_data);

		events_handled++;
	    }
            break;

          /* ****************************************************** */
          case ButtonPress:
            if(event->xany.window == sb->toplevel)
	    {
		sb->btn_state = True;

                OSWGrabPointer(
                    sb->toplevel,
                    True,
                    ButtonReleaseMask | PointerMotionMask,
                    GrabModeAsync, GrabModeAsync,
                    sb->toplevel,
                    None
                );

                /* Get positions. */
                pos_min = sb->pos_min;
                pos_max = sb->pos_max;
                if(pos_min > pos_max) pos_min = pos_max;
                
                pos = sb->pos;
                if(pos < pos_min) pos = pos_min;
                if(pos > pos_max) pos = pos_max;
                      
                /* Get orientation. */
                orientation = sb->orientation;
                        
                /* Get style. */
                style = sb->style;
             
                x = event->xmotion.x;
                y = event->xmotion.y;
                OSWGetWindowAttributes(sb->toplevel, &wattr);
                 
                /* Handle by style. */
                if(style == SCALEBAR_STYLE_FLUSHED)
                {
                    /* Flushed style. */
                            
                    switch(orientation)
                    {
                      case SCALEBAR_ORIENT_VERTICAL:
                        y -= ((SCALEBAR_FLUSHED_HANDLE_LENGTH / 2) + 1);
                        if(y < 0) y = 0;
                        if(y > ((int)wattr.height - SCALEBAR_FLUSHED_HANDLE_LENGTH - 3))
                            y = (int)wattr.height - SCALEBAR_FLUSHED_HANDLE_LENGTH - 3;
                        sb->pos = (pos_max - pos_min) *
                            ((double)y / (double)((int)wattr.height -
                            SCALEBAR_FLUSHED_HANDLE_LENGTH - 3));
                        break;
                        
                      default:  /* SCALEBAR_ORIENT_HORIZONTAL */
                        x -= ((SCALEBAR_FLUSHED_HANDLE_LENGTH / 2) + 1);
                        if(x < 0) x = 0;
                        if(x > ((int)wattr.width - SCALEBAR_FLUSHED_HANDLE_LENGTH - 2))
                            x = (int)wattr.width - SCALEBAR_FLUSHED_HANDLE_LENGTH - 2;
                        sb->pos = (pos_max - pos_min) *
                            ((double)x / (double)((int)wattr.width -
                            SCALEBAR_FLUSHED_HANDLE_LENGTH - 2));
                        break;
                    }
                }
                else
                {
                    /* Standard style or standard value. */
                        
                    switch(orientation)
                    {
                      case SCALEBAR_ORIENT_VERTICAL:
                        image = widget_global.scalebar_v_img;
                        if(image != NULL)
                        {
                            y -= (((int)image->height / 2) + 1);
                            if(y < 0) y = 0;
                            if(y > ((int)wattr.height - (int)image->height - 3))
                                y = (int)wattr.height - (int)image->height - 3;
                            sb->pos = (pos_max - pos_min) *
                                ((double)y / (double)((int)wattr.height -
                                (int)image->height - 3));
                        }
                        break;  
                    
                      default:  /* SCALEBAR_ORIENT_HORIZONTAL */
                        image = widget_global.scalebar_h_img;
                        if(image != NULL)
                        {
                            x -= (((int)image->width / 2) + 1);
                            if(x < 0) x = 0;
                            if(x > ((int)wattr.width - (int)image->width - 2))
                            x = (int)wattr.width - (int)image->width - 2;
                            sb->pos = (pos_max - pos_min) *
                                ((double)x / (double)((int)wattr.width -
                                (int)image->width - 2));
                        }
                        break;   
                    }
                }

                /* Flip position? */
                if(sb->flip_pos)
                    sb->pos = (pos_max - pos_min) - sb->pos;

		sb->pos += pos_min;


		/* Run callback function. */
		if(sb->func_cb != NULL)
		    sb->func_cb(sb->client_data);

		events_handled++;
	    }
	    break;

          /* ********************************************************* */
          case ButtonRelease:
	    /* Release button if it was pressed. */
            if(sb->btn_state)
	    {
		sb->btn_state = False;
		OSWUngrabPointer();

                events_handled++;
	    }
            break;

          /* ********************************************************* */
          case VisibilityNotify:
            if(event->xany.window == sb->toplevel)
            {
                sb->visibility_state = event->xvisibility.state;
                events_handled++;

                /* Do not continue. */
                return(events_handled);
            }
            break;
	}


	/* Redraw as needed. */
	if(events_handled > 0)
	{
	    ScaleBarDraw(sb);
	}


	return(events_handled);
}

/*
 *	Maps scale bar.
 */
void ScaleBarMap(scale_bar_struct *sb)
{
	if(sb == NULL)
	    return;

	sb->map_state = 0;
	ScaleBarDraw(sb);

	return;
}

/*
 *	Unmaps scale bar.
 */
void ScaleBarUnmap(scale_bar_struct *sb)
{
        if(sb == NULL)
            return;


	OSWUnmapWindow(sb->toplevel);
        sb->map_state = 0;
	sb->is_in_focus = 0;
	sb->visibility_state = VisibilityFullyObscured;


	/* Destroy buffers. */
        OSWDestroyPixmap(&sb->toplevel_buf);
        OSWDestroyPixmap(&sb->bkg_buf);


	return;
}

/*
 *	Destroy scale bar.
 */
void ScaleBarDestroy(scale_bar_struct *sb)
{
	if(sb == NULL)
	    return;


        /* Delete widget from regeristry. */
        WidgetRegDelete(sb);


	if(IDC())
	{
            OSWDestroyWindow(&sb->toplevel);
            OSWDestroyPixmap(&sb->toplevel_buf);
            OSWDestroyPixmap(&sb->bkg_buf);
	}


	sb->map_state = 0;
	sb->visibility_state = VisibilityFullyObscured;
	sb->is_in_focus = 0;
	sb->x = 0;
	sb->y = 0;
	sb->width = 0;
	sb->height = 0;
	sb->font = NULL;
        sb->next = NULL;
        sb->prev = NULL;

	sb->pos = 0;
	sb->pos_min = 0;
	sb->pos_max = 0;

	sb->orientation = SCALEBAR_ORIENT_HORIZONTAL;
	sb->flip_pos = False;

	sb->btn_state = False;

	sb->length = 0;

        sb->client_data = NULL;
        sb->func_cb = NULL;


	return;
}




