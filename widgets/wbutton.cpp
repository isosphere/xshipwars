// widgets/wbutton.cpp
/*
                            Widget: Push Button

	Functions:

	bool_t PBtnMatchHotKey(push_button_struct *btn, key_event_t *ke)

	void PBtnChangeLabel(
	        push_button_struct *btn,
	        unsigned int width, 
	        unsigned int height,
	        const char *label,
	        char label_align,
	        image_t *image
	)

	int PBtnInit(
                push_button_struct *btn,
                win_t parent,
                int x, int y,
                unsigned int width,
                unsigned int height,
                const char *label,
                char label_align,
                image_t *image,
		void *client_data,
                int (*func_cb)(void *)
	)
	int PBtnSetHintMessage(
		push_button_struct *btn,
		const char *mesg
	)
	int PBtnSetHotKeys(
		push_button_struct *btn,
		char *hotkeys
	)

	int PBtnDraw(push_button_struct *btn)
	int PBtnManage(push_button_struct *btn, event_t *event)
	void PBtnMap(push_button_struct *btn)
	void PBtnUnmap(push_button_struct *btn)
	void PBtnDestroy(push_button_struct *btn)

 */

#include "../include/string.h"
#include "../include/widget.h"
#include "../include/xsw_ctype.h"


/*
 *	Size constants (in pixels):
 */
#define PBTN_DEFAULT_WIDTH	70
#define	PBTN_DEFAULT_HEIGHT	28

#define PBTN_MARGIN		2

#define PBTN_MIN_WIDTH		(2 * PBTN_MARGIN)
#define PBTN_MIN_HEIGHT		(2 * PBTN_MARGIN)


#define PBTN_CHAR_WIDTH		7
#define PBTN_CHAR_HEIGHT	14



/*
 *	Checks if keycode matches one of the keys specified in btn's
 *	hotkeys listing.
 */
bool_t PBtnMatchHotKey(push_button_struct *btn, key_event_t *ke)
{
	char c;


	if(btn == NULL)
	    return(False);

	/* Get ASCII equvilent for keycode. */
	c = OSWGetASCIIFromKeyCode(ke, False, False, False);
	if(c == '\0')
	    return(False);

	/* Is ASCII character c in btn's hotkeys listing? */
	if(strchr(btn->hotkey, c) != NULL)
	    return(True);
	else
	    return(False);
}


/*
 *	Sets new label or image on btn.
 *
 *	width and height specify the new size of the button,
 *	if either dimension is 0, that dimension will not be
 *	that of the image or label's `reasonable' size.
 */
void PBtnChangeLabel(
        push_button_struct *btn,
        unsigned int width,
        unsigned int height,
        const char *label,
        char label_align,
        image_t *image
)
{
	image_t *tmp_img;


	if(btn == NULL)
	    return;


	/* Set new alignment code. */
	btn->label_align = label_align;

	/* Calculate needed width and height. */
        if(width == 0)
        {
            if(image != NULL)
                width = (int)image->width + (2 * PBTN_MARGIN);
            if(label != NULL)
                width = ((int)strlen(label) * PBTN_CHAR_WIDTH) +
                    (2 * PBTN_MARGIN);
            if(width == 0)
                width = PBTN_DEFAULT_WIDTH;
        }
        if((int)width > (int)osw_gui[0].display_width)
            width = osw_gui[0].display_width;
        if((int)width < PBTN_MIN_WIDTH)
            width = PBTN_MIN_WIDTH;

        if(height == 0)  
        {
            if(image != NULL)
                height = (int)image->height + (2 * PBTN_MARGIN);
            if(label != NULL)
                height = PBTN_CHAR_HEIGHT + (2 * PBTN_MARGIN);
            if(height == 0)
                height = PBTN_DEFAULT_HEIGHT;
        }
        if((int)height > (int)osw_gui[0].display_height)
            height = osw_gui[0].display_height;
        if((int)height < PBTN_MIN_HEIGHT)
            height = PBTN_MIN_HEIGHT;

        /* Width or height cannot be odd. */
        if(IS_NUM_ODD(width))
            width += 1;
        if(IS_NUM_ODD(height))
            height += 1;


	/* Resize btn's toplevel window. */
	OSWResizeWindow(btn->toplevel, width, height);
	btn->width = width;
	btn->height = height;

	/* Recreate btn's pixmap buffer. */
	OSWDestroyPixmap(&btn->toplevel_buf);
	if(
	    OSWCreatePixmap(
		&btn->toplevel_buf,
		width,
		height
	    )
	)
	    return;


	/* Recreate button background images. */
	OSWDestroyImage(&btn->armed_img);
	OSWDestroyImage(&btn->unarmed_img);
        OSWDestroyImage(&btn->highlighted_img);

	if(widget_global.btn_unarmed_img != NULL)
	{
	    if(
		OSWCreateImage(&tmp_img, width, height)
	    )
		return;
	    WidgetResizeImageBuffer(
		osw_gui[0].depth,
		reinterpret_cast<u_int8_t *>(tmp_img->data),
		reinterpret_cast<u_int8_t *>(widget_global.btn_unarmed_img->data),
		tmp_img->width, tmp_img->height,
		widget_global.btn_unarmed_img->width,
		widget_global.btn_unarmed_img->height
	    );
	    btn->unarmed_img = tmp_img;
	}
        if(widget_global.btn_armed_img != NULL)
        {
            if(
                OSWCreateImage(&tmp_img, width, height) 
            ) 
                return;
            WidgetResizeImageBuffer(
                osw_gui[0].depth,
                reinterpret_cast<u_int8_t *>(tmp_img->data),
                reinterpret_cast<u_int8_t *>(widget_global.btn_armed_img->data),
                tmp_img->width, tmp_img->height,
                widget_global.btn_armed_img->width,
		widget_global.btn_armed_img->height
            );
            btn->armed_img = tmp_img;
        }
        if(widget_global.btn_highlighted_img != NULL)
        {
            if(
                OSWCreateImage(&tmp_img, width, height)
            )
                return;
            WidgetResizeImageBuffer(
                osw_gui[0].depth,
                reinterpret_cast<u_int8_t *>(tmp_img->data),
                reinterpret_cast<u_int8_t *>(widget_global.btn_highlighted_img->data),
                tmp_img->width, tmp_img->height,
                widget_global.btn_highlighted_img->width,
		widget_global.btn_highlighted_img->height
            );
            btn->highlighted_img = tmp_img;
        }


	/* Set new label image. */
	btn->image = image;


	/* Set new label text. */
	free(btn->label);
	btn->label = StringCopyAlloc(label);


	return;
}


/*
 *	Initialize a push button widget.
 */
int PBtnInit(
	push_button_struct *btn,
	win_t parent,
	int x, int y,
	unsigned int width,
	unsigned int height,
	const char *label,
	char label_align,
        image_t *image,
	void *client_data,
	int (*func_cb)(void *)
)
{
	pixmap_t pixmap;
	image_t *tar_img, *src_img;


	if((parent == 0) ||
           (btn == NULL)
	)
	    return(-1);


	/* Sanitize width and height. */
	if(width == 0)
	{
	    if(image != NULL)
		width = (int)image->width + (2 * PBTN_MARGIN);
	    if(label != NULL)
		width = ((int)strlen(label) * PBTN_CHAR_WIDTH) +
                    (2 * PBTN_MARGIN);
	    if(width == 0)
		width = PBTN_DEFAULT_WIDTH;
	}
        if((int)width > (int)osw_gui[0].display_width)
            width = osw_gui[0].display_width;
        if((int)width < PBTN_MIN_WIDTH)
            width = PBTN_MIN_WIDTH;

        if(height == 0)
        {
            if(image != NULL)
                height = (int)image->height + (2 * PBTN_MARGIN);
            if(label != NULL)
                height = PBTN_CHAR_HEIGHT + (2 * PBTN_MARGIN);
            if(height == 0)
                height = PBTN_DEFAULT_HEIGHT;
        }
        if((int)height > (int)osw_gui[0].display_height)
            height = osw_gui[0].display_height;
        if((int)height < PBTN_MIN_HEIGHT)
            height = PBTN_MIN_WIDTH;

	/* Width or height cannot be odd. */
	if(IS_NUM_ODD(width))
	    width += 1;
	if(IS_NUM_ODD(height))
	    height += 1;


	/* Reset values. */
	btn->map_state = 0;
	btn->is_in_focus = 0;
	btn->x = x;
	btn->y = y;
	btn->width = width;
	btn->height = height;
	btn->visibility_state = VisibilityFullyObscured;
	btn->disabled = False;
	btn->font = widget_global.pbtn_font;
	btn->next = NULL;
        btn->prev = NULL;

	btn->state = PBTN_UNARMED;
        btn->is_default = False;

	memset(btn->hotkey, '\0', PBTN_MAX_HOTKEYS + 1);


	/* Record parent. */
	btn->parent = parent;


	/* Create toplevel window. */
	if(
	    OSWCreateWindow(
	        &btn->toplevel,
	        parent,
	        btn->x, btn->y,
	        btn->width, btn->height
	    )
	)
	    return(-1);

	OSWSetWindowInput(
	    btn->toplevel,
	    KeyPressMask | KeyReleaseMask |
            ButtonPressMask | ButtonReleaseMask |
            ExposureMask | VisibilityChangeMask |
            EnterWindowMask | LeaveWindowMask
	);


	/* Create button background images. */
	src_img = widget_global.btn_unarmed_img;
	if(src_img != NULL)
	{
            if(OSWCreateImage(
                &tar_img, btn->width, btn->height
            ))
                return(-1);
            WidgetResizeImageBuffer( 
                osw_gui[0].depth,
                reinterpret_cast<u_int8_t *>(tar_img->data),
                reinterpret_cast<u_int8_t *>(src_img->data),
                tar_img->width, tar_img->height,
                src_img->width, src_img->height 
            );
	    pixmap = OSWCreatePixmapFromImage(tar_img);
	    OSWDestroyImage(&tar_img);

	    OSWSetWindowBkg(btn->toplevel, 0, pixmap);
	    OSWDestroyPixmap(&pixmap);
	}
	else
	{
	    OSWSetWindowBkg(btn->toplevel, 0, widget_global.std_bkg_pm);
	}


	/* Set label alignment. */
	btn->label_align = label_align;

	/* Set btn label. */
	btn->label = StringCopyAlloc(label);

	/* Set btn image. */
	btn->image = image;


	/* Set callback function pointer (can be NULL). */
	btn->client_data = client_data;
	btn->func_cb = func_cb;


        /* Add button to widget regeristry. */
        WidgetRegAdd(btn, WTYPE_CODE_PUSHBUTTON);


	return(0);
}


/*
 *	Sets btn's hint message.
 */
int PBtnSetHintMessage(
	push_button_struct *btn,
        const char *mesg
)
{
	if((btn == NULL) ||
           (mesg == NULL)
	)
	    return(-1);

	if(btn->toplevel == 0)
	    return(-1);


	/* Allocate and set hint window message. */
	if(
	    HintWinAddMessage(
	        btn->toplevel,
	        0,	/* Parent (no longer used). */
	        0, 0,	/* x, y (no longer used). */
	        mesg
	    ) < 0
	)
	    return(-1);
	else
	    return(0);
}


/*
 *	Sets btn's hotkeys.
 */
int PBtnSetHotKeys(
	push_button_struct *btn,
        char *hotkeys
)
{
	if((btn == NULL) ||
           (hotkeys == NULL)
	)
	    return(-1);

	/* Set btn's hotkeys (yes its PBTN_MAX_HOTKEYS + 1). */
	strncpy(btn->hotkey, hotkeys, PBTN_MAX_HOTKEYS + 1);
	btn->hotkey[PBTN_MAX_HOTKEYS] = '\0';


	return(0);
}


/*
 *	Draws btn.
 */
int PBtnDraw(push_button_struct *btn)
{
	int mapped = 0;
	int i, n;
	image_t *tar_img, *src_img;
	int x, y, len;
	win_attr_t wattr;
	font_t *prev_font;


        if(btn == NULL)
	    return(-1);

        /* Record previous font. */
        prev_font = OSWQueryCurrentFont();


	/* Get toplevel attributes. */
	OSWGetWindowAttributes(btn->toplevel, &wattr);


        /* Create btn's toplevel buffer as needed. */
        if(btn->toplevel_buf == 0)
        {
            if(OSWCreatePixmap(
                &btn->toplevel_buf,
                wattr.width, wattr.height
            ))
                return(-1);
        }


	/* Map as needed. */
	if(!btn->map_state)
	{
	    OSWMapRaised(btn->toplevel);

	    btn->map_state = 1;
	    mapped = 1;
	}


        /* Create button background images as needed. */

	/* Unarmed background image. */
        src_img = widget_global.btn_unarmed_img;
	tar_img = btn->unarmed_img;
        if((src_img != NULL) &&
           (tar_img == NULL)
        )
        {
            if(OSWCreateImage(&tar_img, wattr.width, wattr.height))
                return(-1);

            WidgetResizeImageBuffer(
                osw_gui[0].depth,
                reinterpret_cast<u_int8_t *>(tar_img->data),
                reinterpret_cast<u_int8_t *>(src_img->data),
                tar_img->width, tar_img->height,
                src_img->width, src_img->height
            );
            btn->unarmed_img = tar_img;
        }

        /* Armed background image. */
        src_img = widget_global.btn_armed_img;
        tar_img = btn->armed_img;
        if((src_img != NULL) &&
           (tar_img == NULL)
        )
        {
            if(OSWCreateImage(&tar_img, wattr.width, wattr.height))
                return(-1);

            WidgetResizeImageBuffer(
                osw_gui[0].depth, 
                reinterpret_cast<u_int8_t *>(tar_img->data),
                reinterpret_cast<u_int8_t *>(src_img->data),
                tar_img->width, tar_img->height,
                src_img->width, src_img->height 
            );    
            btn->armed_img = tar_img;
        }

        /* Highlighted background image. */
        src_img = widget_global.btn_highlighted_img;
        tar_img = btn->highlighted_img;
        if((src_img != NULL) &&
           (tar_img == NULL)
        )
        {
            if(OSWCreateImage(&tar_img, wattr.width, wattr.height))
                return(-1);

            WidgetResizeImageBuffer(
                osw_gui[0].depth,
                reinterpret_cast<u_int8_t *>(tar_img->data),
                reinterpret_cast<u_int8_t *>(src_img->data),
                tar_img->width, tar_img->height,
                src_img->width, src_img->height
            );
            btn->highlighted_img = tar_img;
        }


	/* Draw button background. */
	if(widget_global.force_mono)
	{
	    OSWClearPixmap(
		btn->toplevel_buf,
		wattr.width, wattr.height,
		osw_gui[0].black_pix
	    );
	    OSWSetFgPix(osw_gui[0].white_pix);
	    OSWDrawRectangle(
		btn->toplevel_buf,
		0, 0,
		(int)wattr.width - 1,
		(int)wattr.height - 1
	    );
	}
	else
	{
            if(btn->state == PBTN_ARMED)
            {
                OSWPutImageToDrawable(
		    btn->armed_img,
                    btn->toplevel_buf
                );
            }
	    else if(btn->state == PBTN_HIGHLIGHTED)
            {
                OSWPutImageToDrawable(
		    btn->highlighted_img,
                    btn->toplevel_buf
                );
            }
            else
            {
                OSWPutImageToDrawable(
		    btn->unarmed_img,
                    btn->toplevel_buf
                );
            }
	}


	/* Draw image label. */
	tar_img = btn->image;
	if((tar_img != NULL) &&
           !widget_global.force_mono
	)
	{
            /* Calculate x position. */
            switch(btn->label_align)
            {
              case PBTN_TALIGN_LEFT:
                x = PBTN_MARGIN;
                break;
         
              case PBTN_TALIGN_RIGHT:
                x = (int)wattr.width - (int)tar_img->width - PBTN_MARGIN;
                break;
 
              /* Default to centered. */
              default:
                x = ((int)wattr.width / 2) -
                    ((int)tar_img->width / 2);

                break;
	    }

            /* Calculate y position. */
            y = ((int)wattr.height / 2) - ((int)tar_img->height / 2);

	    WidgetPutImageNormal(
		btn->toplevel_buf,
		tar_img,
		x, y,
		True
	    );
	}



	/* Draw text label. */
	len = ((btn->label == NULL) ? 0 : strlen(btn->label));
	if(len > 0)
	{
	    /* Calculate x position. */
	    switch(btn->label_align)
	    {
	      case PBTN_TALIGN_LEFT:
		x = PBTN_MARGIN;
		break;

	      case PBTN_TALIGN_RIGHT:
		x = (int)wattr.width - (len * PBTN_CHAR_WIDTH) -
                    PBTN_MARGIN;
		break;

	      /* Default to centered. */
	      default:
		x = ((int)wattr.width / 2) -
		    (((int)len * PBTN_CHAR_WIDTH) / 2);
		break;
	    }

	    /* Calculate y position. */
	    y = ((int)wattr.height / 2) + 5;

            OSWSetFont(btn->font);
	    if(widget_global.force_mono)
		OSWSetFgPix(osw_gui[0].white_pix);
	    else if(btn->disabled)
		OSWSetFgPix(widget_global.disabled_text_pix);
	    else
	        OSWSetFgPix(widget_global.normal_text_pix);
	    OSWDrawString(btn->toplevel_buf, x, y, btn->label);

	    /* Draw underline. */
	    for(n = 0; n < PBTN_MAX_HOTKEYS; n++)
	    {
                if(btn->hotkey[n] == '\0')
                    break;

		if(ISBLANK(btn->hotkey[n]) ||
                   (btn->hotkey[n] == '\n') ||
                   (btn->hotkey[n] == '\r')
		)
		    continue;

	        for(i = 0; i < len; i++)
	        {
		    if(toupper(btn->label[i]) == toupper(btn->hotkey[n]))
		    {
			/* Draw underline for char i. */
			OSWDrawLine(btn->toplevel_buf,
			    x + (i * 8), y + 2,
			    x + (i * 8) + 6, y + 2
			);

			/* Draw only one underline, so break. */
			break;
		    }
		}
	    }
	}

	OSWPutBufferToWindow(btn->toplevel, btn->toplevel_buf);
	if(mapped)
	    OSWSetWindowBkg(btn->toplevel, 0, btn->toplevel_buf);

        OSWSetFont(prev_font);


	return(0);
}


/*
 *	Manage btn.
 */
int PBtnManage(push_button_struct *btn, event_t *event)
{
	int events_handled = 0;
	win_attr_t wattr;


        if((event == NULL) ||
           (btn == NULL)
        )
            return(events_handled);

	if(!btn->map_state)
	    return(events_handled);


	switch(event->type)
	{
          /* ********************************************************* */
	  case KeyPress:
	    if(PBtnMatchHotKey(btn, &event->xkey))
	    {
		btn->state = PBTN_ARMED;
		OSWKBAutoRepeatOff();

		events_handled++;
	    }
	    break;

          /* ********************************************************* */
	  case KeyRelease:
            if(PBtnMatchHotKey(btn, &event->xkey))
            {
                btn->state = PBTN_UNARMED;

		if(btn->func_cb != NULL)
		{
		    /* Use btn pointer as referance for now. */
                    btn->func_cb(btn->client_data);
		}

                OSWKBAutoRepeatOn();

                events_handled++;
            }
	    break;

          /* ********************************************************* */
          case Expose:
            if(event->xany.window == btn->toplevel)
	    {
		PBtnDraw(btn);

                events_handled++;
		return(events_handled);
	    }
            break;

	  /* ********************************************************* */
	  case ButtonPress:
            if(event->xany.window == btn->toplevel)
	    {
		btn->state = PBTN_ARMED;

                events_handled++;
	    }
	    break;

          /* ********************************************************* */ 
          case ButtonRelease:
            if(event->xany.window == btn->toplevel)
            {
		if(btn->state == PBTN_ARMED)
		{
                    OSWGetWindowAttributes(btn->toplevel, &wattr);

		    if(((int)event->xbutton.x >= 0) &&
                       ((int)event->xbutton.y >= 0) &&
                       ((int)event->xbutton.x < (int)wattr.width) &&
                       ((int)event->xbutton.y < (int)wattr.height) &&
                       (btn->func_cb != NULL)
		    )
		    {
			/* Use btn pointer as referance for now. */
		        btn->func_cb(btn->client_data);
		    }

		    btn->state = PBTN_HIGHLIGHTED;

		    /* Unschedual hints if it was schedualed. */
		    if(hint_win.next_map > 0)
		    {
			HintWinUnmap();
		    }

                    events_handled++;
		}
		else
		{
		    btn->state = PBTN_UNARMED;

		    /* Do not count as event. */
		}
            }
            break;

          /* ********************************************************* */
          case VisibilityNotify:
            if(event->xany.window == btn->toplevel)
            {
                btn->visibility_state = event->xvisibility.state;

                events_handled++;
                return(events_handled);
            }
            break;

	  /* ********************************************************* */
	  case EnterNotify:
            if(event->xany.window == btn->toplevel)
            {
		if(btn->state == PBTN_UNARMED)
		{
		    btn->state = PBTN_HIGHLIGHTED;

                    /* Schedual hints window map. */
                    HintWinSetSchedual(
                        widget_global.hintwin_map_delay,
                        btn->toplevel
                    );

                    events_handled++;
		}
            }
            break;

          /* ********************************************************* */
          case LeaveNotify:
            if(event->xany.window == btn->toplevel)
            {
		if((btn->state == PBTN_ARMED) ||
                   (btn->state == PBTN_HIGHLIGHTED)
		)
		{
		    btn->state = PBTN_UNARMED;

                    /* Unmap/unschedual hints window. */
                    HintWinUnmap();
         	}
		events_handled++;
            }
            break;
	}


	/* Draw as needed. */
	if(events_handled > 0)
	{
	    PBtnDraw(btn);
	}


	return(events_handled);
}



void PBtnMap(push_button_struct *btn)
{
        if(btn == NULL)
            return;

	/* Reset state. */
	btn->state = PBTN_UNARMED;

        btn->map_state = 0;
	PBtnDraw(btn);

	return;
}



void PBtnUnmap(push_button_struct *btn)
{
        if(btn == NULL)
            return;

	/* Reset state. */
	btn->state = PBTN_UNARMED;

	btn->visibility_state = VisibilityFullyObscured;

        btn->map_state = 0;
	OSWUnmapWindow(btn->toplevel);


	/* Destroy toplevel buffer and images. */
	OSWDestroyPixmap(&btn->toplevel_buf);
	OSWDestroyImage(&btn->unarmed_img);
	OSWDestroyImage(&btn->armed_img);
        OSWDestroyImage(&btn->highlighted_img);


	return;
}



void PBtnDestroy(push_button_struct *btn)
{
	if(btn == NULL)
	    return;


	/* Delete widget from regeristry. */
	WidgetRegDelete(btn);


	if(IDC())
	{
	    /* Delete associated hint message. */
	    HintWinDeleteMessage(btn->toplevel);

	    /* Delete button windows and buffers. */
	    OSWDestroyPixmap(&btn->toplevel_buf);
	    OSWDestroyWindow(&btn->toplevel);

	    /* Destroy local button background images. */
	    OSWDestroyImage(&btn->unarmed_img);
	    OSWDestroyImage(&btn->armed_img);
            OSWDestroyImage(&btn->highlighted_img);
	}

	/* Deallocate other resources. */
	free(btn->label); 
        btn->label = NULL;


	/* Reset structure values. */
        memset(btn, 0x00, sizeof(push_button_struct));

        btn->map_state = 0;
        btn->x = 0;
        btn->y = 0;
        btn->width = 0;
        btn->height = 0;
        btn->visibility_state = VisibilityFullyObscured;
        btn->disabled = False;
	btn->font = NULL;
	btn->next = NULL;
	btn->prev = NULL;

        btn->parent = 0;
	btn->is_default = False;
        btn->state = PBTN_UNARMED;

        btn->image = NULL;	/* Do not destroy image, client owned. */

        btn->label_align = PBTN_TALIGN_CENTER;

	btn->client_data = NULL;
        btn->func_cb = NULL;


	return;
}




