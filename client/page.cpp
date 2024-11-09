/*
                            Page Management

	Functions:

	int GETLABELNUMFROMCOORD(
		page_struct *p,
		win_t w, shared_image_t *image,
		int x, int y
	)

        int PageIsLabelAllocated( 
                page_struct *p,
                int n
        )
	int PageCreateLabel(page_struct *p)

	void PageResize(
		page_struct *p,
		win_t w, shared_image_t *image
	)
	void PageDrawLabel(
		page_struct *p,
                win_t w, shared_image_t *image, int label_num,
		bool_t put_to_window
	)
        void PageDraw(
                page_struct *p,
                win_t w, shared_image_t *image, int amount,
                bool_t put_to_window
        )
	int PageManage(
		page_struct *p,
		win_t w, shared_image_t *image, event_t *event
	)
	void PageMap(
		page_struct *p,
		win_t w, shared_image_t *image
	)
	void PageUnmap(
		page_struct *p,
		win_t w, shared_image_t *image
	)
        void PageDestroy(
		page_struct *p,
		win_t w, shared_image_t *image
        )

        int PageDoAction(
	        page *p,
	        win_t w, shared_image_t *image,
	        int op_code
	)

	---

	The main menu is drawn on the viewscreen.

	Typically the passed arguments should be the window ID
	of the viewscreen and the image buffer of the viewscreen.



 */


#include "../include/swsoundcodes.h"

#include "blitting.h"
#include "keymap.h"
#include "univlist.h"

#include "xsw.h"
#include "net.h"
#include "keymapwin.h"

#include "page.h"


#define MIN(a,b)        ((a) < (b) ? (a) : (b))
#define MAX(a,b)        ((a) > (b) ? (a) : (b))


/* From pagefile.c */
extern image_t *PageLoadImage(char *filename);


/*
 *	Macro to get the main menu label number from
 *	given coordinates.
 *
 *	If labels overlap, older entry gets priority.
 *
 *	Returns -1 on error or no match.
 */
int GETLABELNUMFROMCOORD(
	page_struct *p,
	win_t w, shared_image_t *image,
	int x, int y 
)
{
	int i, n;
	int tmp_x, tmp_y;
	xsw_imglabel_struct *imglabel_ptr;


	if((p == NULL) ||
           (w == 0) ||
           (image == 0)
	)
	    return(-1);


	for(i = 0; i < p->total_labels; i++)
	{
	    if(p->label[i] == NULL)
		continue;

	    /* Go through main menu label i's set of image labels. */
	    for(n = 0; n < PAGE_LABELS_PER_LABEL; n++)
	    {
		/* Get pointer to image label. */
		imglabel_ptr = (xsw_imglabel_struct *)
		    &p->label[i]->imglabel[n];
		if(imglabel_ptr == NULL) continue;
		/* Image label not loaded? */
		if(imglabel_ptr->image == NULL) continue;

		/* Check coordinates. */
		if(imglabel_ptr->pos_by_percent)
		{
		    tmp_x = imglabel_ptr->x *
                        (int)image->width / 100;
                    tmp_y = imglabel_ptr->y *
                        (int)image->height / 100; 
		}
		else
		{
                    tmp_x = imglabel_ptr->x;
                    tmp_y = imglabel_ptr->y;
                }
		if((x >= tmp_x) &&
                   (y >= tmp_y) &&
                   (x < (tmp_x + (int)imglabel_ptr->image->width)) &&
                   (y < (tmp_y + (int)imglabel_ptr->image->height))
		)
		    return(i);

	    }
	}

	return(-1);
}



/*
 *	Checks if main menu label n is allocated on mm.
 */
int PageIsLabelAllocated(
	page_struct *p,
	int n
)
{
	if(p == NULL)
	    return(0);
	else if((p->label == NULL) ||
                (n < 0) ||
                (n >= p->total_labels)
	)
	    return(0);
	else if(p->label[n] == NULL)
	    return(0);
	else
	    return(1);
}


/*
 *	Allocates a new label on the mm structure.
 *
 *	The new label is reset and not loaded.
 *	Returns the label's number or -1 on error.
 */
int PageCreateLabel(
	page_struct *p
)
{
	// Dan S: renamed from "new" to "new_label" for C++ key word compatibility.
	int i, new_label;
	page_label_struct *label_ptr;


	if(p == NULL)
	    return(-1);;

	/* Sanitize total_labels. */
	if(p->total_labels < 0)
	    p->total_labels = 0;


	/* Look for already allocated label pointer. */
	for(i = 0; i < p->total_labels; i++)
	{
	    if(p->label[i] == NULL)
		break;
	}
	if(i < p->total_labels)
	{
	    new_label = i;
	}
	else
	{
	    new_label = p->total_labels;
	    p->total_labels++;

	    p->label = (page_label_struct **)realloc(
		p->label,
		p->total_labels * sizeof(page_label_struct *)
	    );
	    if(p->label == NULL)
	    {
		p->total_labels = 0;
		return(-1);
	    }
	}

	/* Allocate new main menu label structure. */
	p->label[new_label] = (page_label_struct *)calloc(
	    1,
	    sizeof(page_label_struct)
	);
	if(p->label[new_label] == NULL)
	{
	    return(-1);
	}

	/* ********************************************************** */
	/* Reset values. */
	label_ptr = p->label[new_label];

	label_ptr->map_state = 0;
        label_ptr->allow_transparency = 0;
        label_ptr->op_code = XSW_ACTION_NONE;

	/* Reset each xsw image label in the main menu label struct. */
	for(i = 0; i < PAGE_LABELS_PER_LABEL; i++)
            memset(
		&(label_ptr->imglabel[i]),
		0x00,
		sizeof(xsw_imglabel_struct)
	    );

	return(new_label);
}

/*
 *	Initializes page.
 */
int PageInit(
	page_struct *p,
	win_t w, shared_image_t *image
)
{
	if(p == NULL)
	    return(-1);

	memset(p, 0x00, sizeof(page_struct));

	return(0);
}

/*
 *	Resizes the main menu.
 */
void PageResize(
	page_struct *p,
        win_t w, shared_image_t *image
)
{
	unsigned int width, height;
        image_t *ori_img = NULL;
        image_t *new_img = NULL;


	if((p == NULL) ||
           (w == 0) ||
           (image == NULL)
	)
	    return;

	/* Do not resize if not mapped. */
	if(!p->map_state)
	    return;


	/* Get size of image. */
	width = image->width;
	height = image->height;

        /* Load background as needed. */
	if(p->bg_filename != NULL)
	{
            if(p->bg_image == NULL)
            {
	        p->bg_image = PageLoadImage(p->bg_filename);
		if(p->bg_image == NULL)
		    return;
            }
	    ori_img = p->bg_image;

	    /* Resize only as needed. */
	    if((width != (unsigned int)ori_img->width) ||
               (height != (unsigned int)ori_img->height)
	    )
	    {
	        /* Create new sized background. */
	        if(OSWCreateImage(&new_img, width, height))
	            return;

	        /* Resize background. */
	        WidgetResizeImageBuffer(
	            osw_gui[0].depth,
	            reinterpret_cast<u_int8_t *>(new_img->data),  /* Target. */
	            reinterpret_cast<u_int8_t *>(ori_img->data),  /* Source. */
	            new_img->width, new_img->height,
	            ori_img->width, ori_img->height
	        );

	        /* Destroy original image and set new image. */
	        OSWDestroyImage(&ori_img);

		/* Set new resized image for bg image. */
		p->bg_image = new_img;
	    }
	}

	return;
}

/*
 *	Draws label to image.
 */
void PageDrawLabel(
	page_struct *p,
	win_t w, shared_image_t *image, int label_num,
	bool_t put_to_window
)
{
	int x, y;
	unsigned int width, height;
	image_t *label_image;
        page_label_struct *label_ptr;


        if((p == NULL) ||
           (w == 0) ||
           (image == NULL)
        )
            return;

	/* Do not draw if not mapped. */
	if(!p->map_state)
	    return;

	/* Do not draw if no background image is allocated. */
	if(p->bg_image == NULL)
	    return;


	/* Label must be allocated. */
	if(PageIsLabelAllocated(p, label_num))
	    label_ptr = p->label[label_num];
	else
	    return;


	/* ******************************************************* */
	/* Get position and sizes. */

        x = 0;
        y = 0;
	width = 0;
	height = 0;

        switch(p->sel_label_state)
        {
	  case PAGE_LABEL_STATE_ARMED:
	    if(label_ptr->imglabel[PAGE_LABEL_STATE_ARMED].pos_by_percent)
            {
                x = label_ptr->imglabel[PAGE_LABEL_STATE_ARMED].x
                    * (int)image->width / 100;
                y = label_ptr->imglabel[PAGE_LABEL_STATE_ARMED].y
                    * (int)image->height / 100;
            }
            else
            {
                x = label_ptr->imglabel[PAGE_LABEL_STATE_ARMED].x;
                y = label_ptr->imglabel[PAGE_LABEL_STATE_ARMED].y;
            }
            label_image = label_ptr->imglabel[
                PAGE_LABEL_STATE_ARMED].image;
	    if(label_image != NULL)
	    {
		width = label_image->width;
		height = label_image->height;
	    }
            break;

          case PAGE_LABEL_STATE_HIGHLIGHTED:
	    if(label_ptr->imglabel[PAGE_LABEL_STATE_HIGHLIGHTED].pos_by_percent)
            {
                x = label_ptr->imglabel[PAGE_LABEL_STATE_HIGHLIGHTED].x
                    * (int)image->width / 100;
                y = label_ptr->imglabel[PAGE_LABEL_STATE_HIGHLIGHTED].y
                    * (int)image->height / 100;
            }
            else
            {
                x = label_ptr->imglabel[PAGE_LABEL_STATE_HIGHLIGHTED].x;
                y = label_ptr->imglabel[PAGE_LABEL_STATE_HIGHLIGHTED].y;
            }
            label_image = label_ptr->imglabel[
                PAGE_LABEL_STATE_HIGHLIGHTED].image;
            if(label_image != NULL)
            {
                width = label_image->width;
                height = label_image->height;
            }
            break;

          default:	/* Default to unarmed. */
	    if(label_ptr->imglabel[PAGE_LABEL_STATE_UNARMED].pos_by_percent)
            {
                x = label_ptr->imglabel[PAGE_LABEL_STATE_UNARMED].x
                    * (int)image->width / 100;
                y = label_ptr->imglabel[PAGE_LABEL_STATE_UNARMED].y
                    * (int)image->height / 100;
            }
            else
            {
                x = label_ptr->imglabel[PAGE_LABEL_STATE_UNARMED].x;
                y = label_ptr->imglabel[PAGE_LABEL_STATE_UNARMED].y;
            }   
            label_image = label_ptr->imglabel[
                PAGE_LABEL_STATE_UNARMED].image;
            if(label_image != NULL)
            {
                width = label_image->width;
                height = label_image->height;
            }
            break;
	}

	/* Label not allocated? */
	if(label_image == NULL)
	    return;


	/* Blit portion of background to image. */
        BlitBufAbsolute(
            osw_gui[0].depth,
            image->data,		/* Target. */
            reinterpret_cast<u_int8_t *>(p->bg_image->data),	/* Source. */
            x, y,			/* Target coordinates. */
            image->width, image->height,
            x, y,			/* Source coordinates. */
            p->bg_image->width, p->bg_image->height,
            width, height,		/* Copy width and height. */
            1.0,			/* Zoom. */
	    1.0				/* Magnification. */
        );


	/* Blit label to image. */
	if(label_ptr->allow_transparency)
	    BlitBufNormal(
		osw_gui[0].depth,
                image->data,		/* Target. */
                reinterpret_cast<u_int8_t *>(label_image->data), /* Source. */
                x, y,			/* Target coordinates. */ 
                image->width, image->height,
                0, 0,			/* Source coordinates. */ 
                label_image->width, label_image->height,
                label_image->width, label_image->height,
                1.0,			/* Zoom. */
                1.0,			/* Visibility. */
		1.0			/* Magnificaition. */
            );
	else
            BlitBufAbsolute(
                osw_gui[0].depth,
                image->data,		/* Target. */
                reinterpret_cast<u_int8_t *>(label_image->data), /* Source. */
                x, y,			/* Target coordinates. */
                image->width, image->height,
                0, 0,			/* Source coordinates. */
                label_image->width, label_image->height,
                label_image->width, label_image->height,
                1.0,			/* Zoom. */
		1.0			/* Magnification. */
            );


	/* Put to window? */
	if(put_to_window)
	    OSWPutSharedImageToDrawableSect(
		image, w,
		x, y,		/* Target. */
		x, y,		/* Source. */
		width, height
	    );


	return;
}



/*
 *	Redraws the main menu mm on window w.
 *
 *	If put_to_window is False, then only the image is updated,
 *	else both the image is updated and put to the window w.
 */
void PageDraw(
	page_struct *p,
	win_t w, shared_image_t *image,
	int amount,
	bool_t put_to_window
)
{
	int i, len;
	int mm_label_num;
	image_t *bkg_img;
	image_t *label_image;
	int x, y;
	page_label_struct *label_ptr;


	if((p == NULL) ||
           (w == 0) ||
           (image == NULL)
	)
	    return;


	/* Map as needed. */
	if(!p->map_state)
	{
            p->map_state = 1;

	    /* Map all labels. */
	    for(i = 0; i < p->total_labels; i++)
	    {
		if(p->label[i] == NULL)
		    continue;
		p->label[i]->map_state = 1;
	    }

	    /* Need to resize main menu on mapping. */
	    PageResize(p, w, image);
	}


	/* ******************************************************** */
	/* Draw background. */

        /* Load background as needed. */
	if(p->bg_filename != NULL)
	{
            if(p->bg_image == NULL)
            {  
                p->bg_image = PageLoadImage(p->bg_filename);
                if(p->bg_image == NULL)
		    return;
            }

            bkg_img = p->bg_image;

	    /* Copy background image to image buffer. */
	    /* 8 bits. */
	    if(osw_gui[0].depth == 8)
	    {
	        len = MIN(
		    (int)bkg_img->width * (int)bkg_img->height * BYTES_PER_PIXEL8,
		    (int)image->width * (int)image->height * BYTES_PER_PIXEL8
	        );
	    }
	    /* 15 or 16 bits. */
	    else if((osw_gui[0].depth == 15) ||
                    (osw_gui[0].depth == 16)
            )
            {
                len = MIN(
                    (int)bkg_img->width * (int)bkg_img->height * BYTES_PER_PIXEL16,
                    (int)image->width * (int)image->height * BYTES_PER_PIXEL16
                );
            }
	    /* 24 or 32 bits. */
            else if((osw_gui[0].depth == 24) ||
                    (osw_gui[0].depth == 32)
	    )
            {
                len = MIN(
                    (int)bkg_img->width * (int)bkg_img->height * BYTES_PER_PIXEL32,
                    (int)image->width * (int)image->height * BYTES_PER_PIXEL32
                );
            }
	    else
	    {
	        /* Unsupported depth. */
	        return;
	    }
	    /* Copy it. */
	    for(i = 0; i < len; i++)
	        image->data[i] = bkg_img->data[i];
	}


        /* ******************************************************** */
        /* Draw labels. */

	for(mm_label_num = 0;
	    mm_label_num < p->total_labels;
	    mm_label_num++
	)
	{
	    /* Get main menu label pointer. */
	    label_ptr = p->label[mm_label_num];
	    /* Cannot be NULL. */
	    if(label_ptr == NULL)
		continue;

	    /* Skip if label is not mapped. */
	    if(label_ptr->map_state == 0)
		continue;


	    /* Is this label selected? */
	    if(mm_label_num == p->sel_label)
	    {
		x = 0;
		y = 0;
		/* Image label index corresponds with state. */
		switch(p->sel_label_state)
		{
		  case PAGE_LABEL_STATE_ARMED:
		    if(label_ptr->imglabel[PAGE_LABEL_STATE_ARMED].pos_by_percent)
		    {
                        x = label_ptr->imglabel[PAGE_LABEL_STATE_ARMED].x
                            * (int)image->width / 100;
                        y = label_ptr->imglabel[PAGE_LABEL_STATE_ARMED].y
                            * (int)image->height / 100;
	            }
		    else
		    {
		        x = label_ptr->imglabel[PAGE_LABEL_STATE_ARMED].x;
                        y = label_ptr->imglabel[PAGE_LABEL_STATE_ARMED].y;
		    }
		    label_image = label_ptr->imglabel[
			PAGE_LABEL_STATE_ARMED].image;
		    break;

		  case PAGE_LABEL_STATE_HIGHLIGHTED:
		    if(label_ptr->imglabel[PAGE_LABEL_STATE_HIGHLIGHTED].pos_by_percent)
                    {
                        x = label_ptr->imglabel[PAGE_LABEL_STATE_HIGHLIGHTED].x
                            * (int)image->width / 100;      
                        y = label_ptr->imglabel[PAGE_LABEL_STATE_HIGHLIGHTED].y
                            * (int)image->height / 100;
                    }
                    else
                    {
                        x = label_ptr->imglabel[PAGE_LABEL_STATE_HIGHLIGHTED].x;
                        y = label_ptr->imglabel[PAGE_LABEL_STATE_HIGHLIGHTED].y;
                    }
                    label_image = label_ptr->imglabel[
			PAGE_LABEL_STATE_HIGHLIGHTED].image;
                    break;

		  default:	/* Default to unarmed. */
		    if(label_ptr->imglabel[PAGE_LABEL_STATE_UNARMED].pos_by_percent)
                    {
                        x = label_ptr->imglabel[PAGE_LABEL_STATE_UNARMED].x
                            * (int)image->width / 100;
                        y = label_ptr->imglabel[PAGE_LABEL_STATE_UNARMED].y
                            * (int)image->height / 100;
                    }
                    else
                    {
                        x = label_ptr->imglabel[PAGE_LABEL_STATE_UNARMED].x;
                        y = label_ptr->imglabel[PAGE_LABEL_STATE_UNARMED].y;
		    }
		    label_image = label_ptr->imglabel[
			PAGE_LABEL_STATE_UNARMED].image;
                    break;
		}
		if(label_image == NULL)
		{
		    if(label_ptr->imglabel[PAGE_LABEL_STATE_UNARMED].pos_by_percent)
                    {
                        x = label_ptr->imglabel[PAGE_LABEL_STATE_UNARMED].x
                            * (int)image->width / 100;
                        y = label_ptr->imglabel[PAGE_LABEL_STATE_UNARMED].y
                            * (int)image->height / 100; 
                    }
                    else
                    {
                        x = label_ptr->imglabel[PAGE_LABEL_STATE_UNARMED].x;
                        y = label_ptr->imglabel[PAGE_LABEL_STATE_UNARMED].y;
                    }
		    label_image = label_ptr->imglabel[
			PAGE_LABEL_STATE_UNARMED].image;
		}
	    }
	    else
	    {
		/* Not selected, use unarmed image. */
		if(label_ptr->imglabel[PAGE_LABEL_STATE_UNARMED].pos_by_percent)
                {
                    x = label_ptr->imglabel[PAGE_LABEL_STATE_UNARMED].x
                        * (int)image->width / 100;
                    y = label_ptr->imglabel[PAGE_LABEL_STATE_UNARMED].y
                        * (int)image->height / 100;
                }
		else
		{
                    x = label_ptr->imglabel[PAGE_LABEL_STATE_UNARMED].x;
                    y = label_ptr->imglabel[PAGE_LABEL_STATE_UNARMED].y;
		}
		label_image = label_ptr->imglabel[
		    PAGE_LABEL_STATE_UNARMED].image;
	    }
	    if(label_image == NULL)
		continue;

	    /* Blit label_image to image. */
	    if(label_ptr->allow_transparency)
	    {
		BlitBufNormal(
		    osw_gui[0].depth,
		    image->data,	/* Target. */
		    reinterpret_cast<u_int8_t *>(label_image->data), /* Source. */
		    x, y, 		/* Target coordinates. */
		    image->width, image->height,
		    0, 0,		/* Source coordinates. */
		    label_image->width, label_image->height,
                    label_image->width, label_image->height,
		    1.0,	/* Zoom. */
		    1.0,	/* Visibility. */
		    1.0		/* Magnification. */
		);
	    }
	    else
	    {
		BlitBufAbsolute(
		    osw_gui[0].depth,
		    image->data,        /* Target. */
                    reinterpret_cast<u_int8_t *>(label_image->data), /* Source. */
                    x, y,               /* Target coordinates. */
                    image->width, image->height,
                    0, 0,               /* Source coordinates. */
                    label_image->width, label_image->height,
                    label_image->width, label_image->height,
                    1.0,	/* Zoom. */
		    1.0		/* Magnification. */
		);
	    }
	}

	/* Put image to window? */
	if(put_to_window)
            OSWPutSharedImageToDrawable(image, w);


	return;
}

int PageManage(
	page_struct *p,
	win_t w, shared_image_t *image, event_t *event
)
{
	int label_num;
	page_label_struct *label_ptr;
	int events_handled = 0;


	if((p == NULL) ||
           (w == 0) ||
           (image == NULL) ||
           (event == NULL)
	)
	    return(events_handled);

	if(!p->map_state &&
           (event->type != MapNotify)
	)
	    return(events_handled);


	switch(event->type)
	{
	  /* ****************************************************** */
	  case KeyPress:
	    if(!p->is_in_focus)
		break;


	    break;

          /* ****************************************************** */
          case KeyRelease:
            if(!p->is_in_focus)
                break;

            break;

          /* ****************************************************** */
          case ButtonPress:
            /* Event must be on main menu's window. */
            if(event->xany.window != w)
	    {
		/* Unfocus as needed. */
		if(p->is_in_focus)
		    p->is_in_focus = 0;

		break;
	    }

	    /* Set into focus. */
	    p->is_in_focus = 1;

	    /* Button press coordinates on a label? */
            label_num = GETLABELNUMFROMCOORD(
		p, w, image,
		event->xbutton.x,
		event->xbutton.y
	    );
	    if(PageIsLabelAllocated(p, label_num))
	    {
		/* Set newly selected label. */
		p->sel_label = label_num;
		p->sel_label_state = PAGE_LABEL_STATE_ARMED;

		PageDrawLabel(p, w, image, p->sel_label, True);

		events_handled++;
		return(events_handled);
	    }
            break;

          /* ****************************************************** */
          case ButtonRelease:
            /* Event must be on main menu's window. */
            if(event->xany.window != w) break;

            label_num = GETLABELNUMFROMCOORD(
                p, w, image,
                event->xbutton.x,
                event->xbutton.y
            );
            if(PageIsLabelAllocated(p, label_num))
            {
		label_ptr = p->label[label_num];

		/* Was label previously armed? */
		if((label_num == p->sel_label) &&
                   (p->sel_label_state == PAGE_LABEL_STATE_ARMED)
		)
		{
		    /* Unmap hint window. */
		    HintWinUnmap();

                    /* Set label back to highlighted. */
                    p->sel_label_state = PAGE_LABEL_STATE_HIGHLIGHTED;

		    /* Redraw. */
                    PageDrawLabel(p, w, image, p->sel_label, True);
 
                    /* Play select sound. */
                    if((option.sounds > XSW_SOUNDS_NONE) &&
                       (label_ptr->op_code != XSW_ACTION_NONE)
		    )
                        SoundPlay(
                            SOUND_CODE_MENU_SELECT,
                            1.00,
                            1.00,
                            0,
                            0
                        );

		    /* Perform action defined by label. */
                    PageDoAction(
			p,
			w, image,
			label_ptr->op_code
		    );

		    events_handled++;
		    return(events_handled);
		}
		else
		{
                    /*   The button was released on some other label,
                     *   in which case select that label and set it to
                     *   be highlighted, do not perform its action.
		     */
                    p->sel_label = label_num;
                    p->sel_label_state = PAGE_LABEL_STATE_HIGHLIGHTED;

                    /* Redraw. */
                    PageDrawLabel(p, w, image, p->sel_label, True);

		    events_handled++;
                    return(events_handled);
		}
            }
            break;

          /* ****************************************************** */
          case MotionNotify:
	    /* Event must be on main menu's window. */
	    if(event->xany.window == w)
	    {
		/* Get label number that the pointer is over. */
                label_num = GETLABELNUMFROMCOORD(
                    p, w, image,
                    event->xmotion.x,
                    event->xmotion.y
                );
	        /* Got valid label? */
                if(PageIsLabelAllocated(p, label_num))
                {
                    label_ptr = p->label[label_num];

		    if((p->sel_label == label_num) &&
                       (p->sel_label_state == PAGE_LABEL_STATE_ARMED)
	            )
		    {
		        /* Label already selected, do nothing. */
		        events_handled++;
		        return(events_handled);
		    }
		    else if(p->sel_label != label_num)
		    {
			/* Pointer has moved over new label. */

                        HintWinUnmap();

                        /* Set new label highlighted. */
                        p->sel_label = label_num;
                        p->sel_label_state = PAGE_LABEL_STATE_HIGHLIGHTED;

			/* Schedual hint message to be shown. */
			if(label_ptr->op_code != XSW_ACTION_NONE)
			{
			    if(label_ptr->hint_mesg != NULL)
	                        HintWinSetSchedualMessage(
			            widget_global.hintwin_map_delay,
			            w,
			            label_ptr->hint_mesg
		                );
			}

			/* Play highlighted sound. */
                        if((option.sounds > XSW_SOUNDS_NONE) &&
                           (label_ptr->op_code != XSW_ACTION_NONE)
                        )
                            SoundPlay(
                                SOUND_CODE_MENU_HIGHLIGHT,
                                1.00,
                                1.00,
                                0, 
                                0
                            );

		        events_handled++;
		    }
                }
	        else
	        {
		    /* Pointer is not over any label. */

		    /* Was a label previously selected? */
		    if(PageIsLabelAllocated(p, p->sel_label))
		    {
			HintWinUnmap();

		        events_handled++;
		    }

                    p->sel_label = -1;
		    p->sel_label_state = PAGE_LABEL_STATE_UNARMED;
		}
	    }
	    else
	    {
		/*   MotionNotify occured on another window,
		 *   check if p->sel_label is valid, if so unmap hint
		 *   window.
		 */
                if(PageIsLabelAllocated(p, p->sel_label))
                {
		    p->sel_label = -1;
                    HintWinUnmap();
                }
	    }
            break;

          /* ****************************************************** */
          case Expose:
            /* Event must be on main menu's window. */
            if(event->xany.window == w)
            {
		events_handled++;
	    }
	    break;
	}

	/* Redraw as needed. */
	if(events_handled > 0)
	{
	    PageDraw(
		p,
		w,
		image,
		DRAW_AMOUNT_COMPLETE,
		True			/* Put to window. */
	    );
	}

	return(events_handled);
}

void PageMap(
	page_struct *p,
	win_t w, shared_image_t *image
)
{
        if(!IDC() ||
           (p == NULL) ||
           (w == 0) ||
           (image == NULL)
        )
            return;


	if(!p->map_state)
	{
            OSWUnsetWindowCursor(w);

	    /* Change selected watched events. */
	    OSWSetWindowInput(
		w,
		ButtonPressMask | ButtonReleaseMask |
		PointerMotionMask | ExposureMask |
		VisibilityChangeMask
	    );
	}

	/* Map by drawing it. */
	p->map_state = 0;
	PageDraw(p, w, image, DRAW_AMOUNT_COMPLETE, True);

	return;
}

void PageUnmap(
	page_struct *p,
	win_t w, shared_image_t *image
)
{
	if((p == NULL) ||
           (w == 0) ||
           (image == NULL)
	)
	    return;

	/* Unmap as needed. */
	if(p->map_state)
	{
	    /* Destroy the page background image. */
            OSWDestroyImage(&p->bg_image);

	    /* Set unmapped map state. */
            p->map_state = 0;

	    /* Unmap hint window. */
            HintWinUnmap();
	}

	return;
}

/*
 *	Deallocates all resources for the page.
 */
void PageDestroy(
	page_struct *p,
	win_t w, shared_image_t *image
)
{
	int i, n;
	page_label_struct *label_ptr;


        if(p == NULL)
	    return;

	/* Free background image filename. */
	free(p->bg_filename);
	p->bg_filename = NULL;

	/* Destroy background image. */
	OSWDestroyImage(&p->bg_image);

	/* Deallocate all labels. */
	for(i = 0; i < p->total_labels; i++)
	{
	    label_ptr = p->label[i];
	    if(label_ptr == NULL)
		continue;

	    /* Free label's substructures. */
	    for(n = 0; n < PAGE_LABELS_PER_LABEL; n++)
	    {
	        ImgLabelReset(&label_ptr->imglabel[n]);
		/* Do not free imglabel[n] itself. */
	    }

            /* Free hint message. */
            free(label_ptr->hint_mesg); 

	    /* Free structure itself. */
	    free(label_ptr);
	}
	free(p->label);
	p->label = NULL;

	p->total_labels = 0;

	/* Reset values. */
        p->map_state = 0;


	return;
}

/*
 *	Performs main menu action.
 */
int PageDoAction(
	page_struct *p,
	win_t w, shared_image_t *image,
	int op_code
)
{
	XSWActionCB(
	    &bridge_win,	/* Pointer to window structure. */
	    NULL,		/* Data pointer. */
	    op_code		/* Action. */
	);

	return(0);
}
