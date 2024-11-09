// widgets/wprogressbar.cpp
/*
                         Widget: Progress Bar

	Functions:

	int PBarInit(
		progress_bar_struct *pb,
		win_t parent,
		int x, int y,
		unsigned int width, unsigned int height,
                double start_val,
                double min, double max,
		char *label,
		int completion_action
	)
	void PBarResize(progress_bar_struct *pb)
	int PBarDraw(progress_bar_struct *pb)
	int PBarManage(progress_bar_struct *pb, event_t *xevent)
	void PBarMap(progress_bar_struct *pb)
	void PBarUnmap(progress_bar_struct *pb)
	void PBarDestroy(progress_bar_struct *pb)

	int PBarUpdate(
		progress_bar_struct *pb,
		double value,
		char *label
	)

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
#define PBAR_MARGIN	1


/*
 *	Initializes the progress bar.
 */
int PBarInit(
        progress_bar_struct *pb,
        win_t parent,
        int x, int y,
        unsigned int width, unsigned int height,
        double start_val,
        double min, double max,
        char *label,
        int completion_action
)
{
	if((pb == NULL) ||
           (parent == 0)
	)
	    return(-1);

	if(width == 0)
	    width = 1;
	if(height == 0)
	    height = 1;

	if(start_val < min)
	    start_val = min;
	if(start_val > max)
	    start_val = max;


	/* Reset values. */
	memset(pb, 0x00, sizeof(progress_bar_struct));
	pb->map_state = 0;
        pb->x = x;
        pb->y = y;
        pb->width = width;
        pb->height = height;
        pb->is_in_focus = 0;
	pb->font = OSWQueryCurrentFont();
	pb->next = NULL;
	pb->prev = NULL;


	if(
	    OSWCreateWindow(
	        &pb->toplevel,
	        parent,
	        pb->x, pb->y,
	        pb->width, pb->height
	    )
	)
	    return(-1);

	pb->toplevel_buf = 0;
	pb->image_buf = NULL;

	OSWSetWindowInput(
	    pb->toplevel,
	    ExposureMask | VisibilityChangeMask
	);


	/* Set values. */
	pb->label = StringCopyAlloc(label);

        pb->current = start_val;
	pb->min = min;
	pb->max = max;
	pb->completion_action = completion_action;


	/* Add widget to regeristry. */
	WidgetRegAdd((void *)pb, WTYPE_CODE_PROGRESSBAR);


	return(0);
}


/*
 *	Resizes the progress bar.
 */
void PBarResize(progress_bar_struct *pb)
{
	win_attr_t wattr;


	if(pb == NULL)
	    return;

	OSWGetWindowAttributes(pb->toplevel, &wattr);
	if(((int)pb->width == (int)wattr.width) &&
           ((int)pb->height == (int)wattr.height)
	)
	    return;

	pb->x = wattr.x;
	pb->y = wattr.y;
	pb->width = wattr.width;
	pb->height = wattr.height;

	OSWDestroyImage(&pb->image_buf);
	OSWDestroyPixmap(&pb->toplevel_buf);


	if(pb->map_state)
	    PBarDraw(pb);

	return;
}

/*
 *	Redraws the progress bar.
 */
int PBarDraw(progress_bar_struct *pb)
{
	double progress_delta_dist, total_delta_dist;
	double gamma;
	u_int8_t *data_ptr;
	u_int8_t *ptr8;
	u_int16_t *ptr16;
        u_int32_t *ptr32;
	int x_pos, x_pos_check;		/* In pixels. */
	int x_col_pos, y_row_pos;	/* In pixels. */
	unsigned int width, height;
	int bytes_per_line;
	image_t *image;
	win_t w;
	pixmap_t pixmap;
	font_t *prev_font;
	WColorStruct color;
	win_attr_t wattr;


        if(pb == NULL)
            return(-1);


	w = pb->toplevel;

	OSWGetWindowAttributes(w, &wattr);

	total_delta_dist = pb->max - pb->min;
	if(total_delta_dist < 0)
	    total_delta_dist = 0;

	progress_delta_dist = pb->current - pb->min;
	if(progress_delta_dist < 0)
	    progress_delta_dist = 0;


	/* Map as needed. */
	if(!pb->map_state)
	{
	    OSWMapRaised(w);
	    pb->map_state = 1;
	}


        prev_font = OSWQueryCurrentFont();
        OSWSetFont(pb->font);


	/* Recreate image buffer as needed. */
	if(pb->image_buf == NULL)
	{
	    if(
		OSWCreateImage(
		    &pb->image_buf,
		    wattr.width, wattr.height
		)
	    )
		return(-1);
	}
	image = pb->image_buf;
	data_ptr = (u_int8_t *)image->data;  

	/* Recreate graphics buffer as needed. */
	if(pb->toplevel_buf == 0)
	{
	    if(
		OSWCreatePixmap(
		    &pb->toplevel_buf,
		    wattr.width, wattr.height
                )
            )
                return(-1);
        }
	pixmap = pb->toplevel_buf;


	/* Get size from image. */
	width = image->width;
	height = image->height;


        /* Calculate x_pos, the x position that the progress bar ends. */
        if(total_delta_dist != 0)
        {
            x_pos = static_cast<int>((progress_delta_dist / total_delta_dist) *
                (double)MAX((int)wattr.width - (2 * PBAR_MARGIN), 0));
            if(x_pos < 0)
                x_pos = 0;
        }
        else
        {
            x_pos = 0;
        }


	/* Draw progress bar. */
	if(data_ptr != NULL)
	{
	    /* Set colors. */
	    color.a = 0x00;
	    color.r = 0xff;
	    color.g = 0x02;
            color.b = 0x80;


	    /* Blit lines. */
	    switch(osw_gui[0].depth)
	    {
	      case 8:
                bytes_per_line = width * BYTES_PER_PIXEL8;
                x_col_pos = 0;
                y_row_pos = 0;
                x_pos_check = MIN((x_pos + 1), (int)width);

                gamma = (double)y_row_pos / (double)height;
                if(gamma > 0.5)
                    gamma = (2 * (1 - gamma)) + 0.2;
                else
                    gamma = (2 * gamma) + 0.2;
                while(y_row_pos < (int)height)
                {
                    ptr8 = (u_int8_t *)(&data_ptr[
                        (bytes_per_line * y_row_pos) +
                        (BYTES_PER_PIXEL8 * x_col_pos)
                    ]);

		    if(x_col_pos < x_pos_check)
                        *ptr8 = PACK8TO8(
                            (u_int8_t)(MIN(color.r * gamma, 0xff)),
                            (u_int8_t)(MIN(color.g * gamma, 0xff)),
                            (u_int8_t)(MIN(color.b * gamma, 0xff))
                        );
		    else
			*ptr8 = 0x00;

                    /* Increment values. */
                    x_col_pos++;
                    if(x_col_pos >= (int)width)
                    {
                        x_col_pos = 0;
                        y_row_pos++;

                        gamma = (double)y_row_pos / (double)image->height;
                        if(gamma > 0.5)
                            gamma = (2 * (1 - gamma)) + 0.2;
                        else
                            gamma = (2 * gamma) + 0.2;
                    }
		}
	        break;

	      case 15:
                bytes_per_line = width * BYTES_PER_PIXEL15;
                x_col_pos = 0;
                y_row_pos = 0;
                x_pos_check = MIN((x_pos + 1), (int)width);

                gamma = (double)y_row_pos / (double)height;
                if(gamma > 0.5)
                    gamma = (2 * (1 - gamma)) + 0.2;
                else
                    gamma = (2 * gamma) + 0.2;
                while(y_row_pos < (int)height)
                {
                    ptr16 = (u_int16_t *)(&data_ptr[
                        (bytes_per_line * y_row_pos) +
                        (BYTES_PER_PIXEL15 * x_col_pos)
                    ]);

                    if(x_col_pos < x_pos_check)
                        *ptr16 = PACK8TO15(
                            (u_int8_t)(MIN(color.r * gamma, 0xff)),
                            (u_int8_t)(MIN(color.g * gamma, 0xff)),
                            (u_int8_t)(MIN(color.b * gamma, 0xff))
                        );
                    else
                        *ptr16 = 0x00;

                    /* Increment values. */
                    x_col_pos++;
                    if(x_col_pos >= (int)width)
                    {
                        x_col_pos = 0;
                        y_row_pos++;

                        gamma = (double)y_row_pos / (double)image->height;
                        if(gamma > 0.5)
                            gamma = (2 * (1 - gamma)) + 0.2;
                        else  
                            gamma = (2 * gamma) + 0.2;
                    }
                }
		break;

	      case 16:
                bytes_per_line = width * BYTES_PER_PIXEL16;
                x_col_pos = 0;
                y_row_pos = 0;
                x_pos_check = MIN((x_pos + 1), (int)width);

                gamma = (double)y_row_pos / (double)height;
                if(gamma > 0.5)
                    gamma = (2 * (1 - gamma)) + 0.2;
                else
                    gamma = (2 * gamma) + 0.2;
                while(y_row_pos < (int)height)
                {
                    ptr16 = (u_int16_t *)(&data_ptr[
                        (bytes_per_line * y_row_pos) +
                        (BYTES_PER_PIXEL16 * x_col_pos)
                    ]);

                    if(x_col_pos < x_pos_check)
                        *ptr16 = PACK8TO16(
                            (u_int8_t)(MIN(color.r * gamma, 0xff)),
                            (u_int8_t)(MIN(color.g * gamma, 0xff)),
                            (u_int8_t)(MIN(color.b * gamma, 0xff))
                        );
                    else
                        *ptr16 = 0x00;

                    /* Increment values. */
                    x_col_pos++;
                    if(x_col_pos >= (int)width)
                    {
                        x_col_pos = 0;
                        y_row_pos++;

                        gamma = (double)y_row_pos / (double)image->height;
                        if(gamma > 0.5)
                            gamma = (2 * (1 - gamma)) + 0.2;
                        else
                            gamma = (2 * gamma) + 0.2;
                    }   
                }
                break;

	      case 24:
	      case 32:
                bytes_per_line = width * BYTES_PER_PIXEL32;
                x_col_pos = 0;
                y_row_pos = 0;
                x_pos_check = MIN((x_pos + 1), (int)width);

                gamma = (double)y_row_pos / (double)height;
                if(gamma > 0.5)
                    gamma = (2 * (1 - gamma)) + 0.2;
                else
                    gamma = (2 * gamma) + 0.2;
                while(y_row_pos < (int)height)
                {
                    ptr32 = (u_int32_t *)(&data_ptr[
                        (bytes_per_line * y_row_pos) +
                        (BYTES_PER_PIXEL32 * x_col_pos)
                    ]);

                    if(x_col_pos < x_pos_check)
                        *ptr32 = PACK8TO32(
			    (u_int8_t)0x00,
                            (u_int8_t)(MIN(color.r * gamma, 0xff)),
                            (u_int8_t)(MIN(color.g * gamma, 0xff)),
                            (u_int8_t)(MIN(color.b * gamma, 0xff))
                        );
                    else
                        *ptr32 = 0x00;

                    /* Increment values. */
                    x_col_pos++;
                    if(x_col_pos >= (int)width)
                    {
                        x_col_pos = 0;
                        y_row_pos++;

                        gamma = (double)y_row_pos / (double)image->height;
                        if(gamma > 0.5)
                            gamma = (2 * (1 - gamma)) + 0.2;
                        else
                            gamma = (2 * gamma) + 0.2;
                    }
                }
                break;
	    }

	    OSWPutImageToDrawable(image, pixmap);
	}


	/* Draw text. */
	if(pb->label != NULL)
	{
	    if(widget_global.force_mono)
		OSWSetFgPix(osw_gui[0].white_pix);
	    else
	        OSWSetFgPix(widget_global.pbar_text_pix);

	    OSWDrawString(
		w,
		12, ((int)wattr.height / 2) + 5,
	        pb->label
	    );
	}

	/* Draw depressed frame. */
	WidgetFrameButtonPixmap(
	    pixmap,
	    True,
	    wattr.width, wattr.height,
	    widget_global.surface_highlight_pix,
	    widget_global.surface_shadow_pix
	);

	OSWPutBufferToWindow(w, pixmap);


        OSWSetFont(prev_font);


	return(0);
}


/*
 *	Manages progress bar.
 */
int PBarManage(progress_bar_struct *pb, event_t *event)
{
	int events_handled = 0;



        if((pb == NULL) ||
           (event == NULL)
        )
            return(events_handled);

	if(!pb->map_state &&
	   (event->type != MapNotify)
	)
	    return(events_handled);


	switch(event->type)
	{
	  /* ******************************************************* */
	  case Expose:
	    if(event->xany.window == pb->toplevel)
	    {
	        PBarDraw(pb);
	        events_handled++;
		return(events_handled);
	    }
	    break;

          /* ********************************************************* */
          case VisibilityNotify:
            if(event->xany.window == pb->toplevel)
            {
                pb->visibility_state = event->xvisibility.state;

                events_handled++;
                return(events_handled);
            }
            break;
	}

	if(events_handled > 0)
            PBarDraw(pb);


	return(events_handled);
}


/*
 *	Maps progress bar.
 */
void PBarMap(progress_bar_struct *pb)
{
        if(pb == NULL)
            return;

	pb->map_state = 0;
	PBarDraw(pb);


	return;
}

/*
 *	Unmaps progress bar.
 */
void PBarUnmap(progress_bar_struct *pb)
{
        if(pb == NULL)
            return;   


	OSWUnmapWindow(pb->toplevel);
	pb->map_state = 0;

	OSWDestroyPixmap(&pb->toplevel_buf);


	return;
}


/*
 *	Destroys progress bar.
 */
void PBarDestroy(progress_bar_struct *pb)
{
        if(pb == NULL)
	    return;


        /* Delete widget from regeristry. */
        WidgetRegDelete(pb);


	if(IDC())
	{
	    OSWDestroyWindow(&pb->toplevel);
	    OSWDestroyPixmap(&pb->toplevel_buf);
	    OSWDestroyImage(&pb->image_buf);
	}


	pb->map_state = 0;
	pb->visibility_state = VisibilityFullyObscured;
	pb->is_in_focus = 0;
	pb->x = 0;
	pb->y = 0;
	pb->width = 0;
	pb->height = 0;
	pb->font = NULL;
        pb->next = NULL;
        pb->prev = NULL;

	pb->current = 0;
	pb->min = 0;
	pb->max = 0;

	/* Free label. */
	free(pb->label);
	pb->label = NULL;



	return;
}


/*
 *	Updates progress on progress bar.
 */
int PBarUpdate(progress_bar_struct *pb, double value, char *label)
{
	if(pb == NULL)
            return(-1);


	/* Update value. */
	if(value > pb->max)
	    value = pb->max;
        if(value < pb->min)
            value = pb->min;

	pb->current = value;

	/* Change name as needed. */
	if(label != NULL)
	{
	    free(pb->label);

	    pb->label = (char *)calloc(
		1,
		(strlen(label) + 1) * sizeof(char)
	    );
	    if(pb->label != NULL)
	    {
	        strcpy(pb->label, label);
	    }
	}


	/* Redraw. */
        pb->map_state = 0;
        PBarDraw(pb);


	return(0);
}




