// widgets/wdialog.cpp
/*
                        Widget: Dialog Window

	Functions:

	int DialogWinDismissPBCB(void *ptr)

	int DialogWinInit(
        	dialog_win_struct *dw,
        	win_t parent,
        	unsigned int width, 
        	unsigned int height,
        	image_t *icon
	)
	int DialogWinDraw(
        	dialog_win_struct *dw,
        	char *mesg
	)
	int DialogWinManage(
                dialog_win_struct *dw,
                event_t *event
	)
	void DialogWinMap(dialog_win_struct *dw)
	void DialogWinUnmap(dialog_win_struct *dw)

	void DialogWinDestroy(dialog_win_struct *dw)

	int printdw(
        	dialog_win_struct *dw,
        	char *mesg
	)

	---



 */

#include "../include/osw-x.h"
#include "../include/string.h"
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
#define DW_DEF_WIDTH		350
#define DW_DEF_HEIGHT		80

#define DW_DISMISS_BTN_WIDTH	70
#define DW_DISMISS_BTN_HEIGHT	28

#define DW_MARGIN		10

#define DW_TITLE_STR		"Dialog"
#define DW_ICON_TITLE_STR	"Dialog"

#define DW_CHAR_WIDTH	7
#define DW_CHAR_HEIGHT	14


/*
 *	Dialog window's dismiss button callback.
 */
int DialogWinDismissPBCB(void *ptr)
{
	dialog_win_struct *dw;

	if(ptr == NULL)
	    return(-1);

	dw = (dialog_win_struct *)ptr;
	DialogWinUnmap(dw);

	return(0);
}


/*
 *	Initializes a dialog window widget.
 */
int DialogWinInit(
	dialog_win_struct *dw,
	win_t parent,
	unsigned int width,
	unsigned int height,
        image_t *icon		/* Shared. */
)
{
	int x, y;


	if((dw == NULL) ||
           (parent == 0)
	)
            return(-1);


	/* Set geometry. */
        if(width == 0)  
            width = DW_DEF_WIDTH;
        if(height == 0)
            height = DW_DEF_HEIGHT;

	if(parent == osw_gui[0].root_win)
	{
            x = (int)((int)osw_gui[0].display_width / 2) -
	        (int)((int)width / 2);
            y = (int)((int)osw_gui[0].display_height / 2) -
	        (int)((int)height / 2);
	}
	else
	{
	    x = 0;
	    y = 0;
	}


	/* Reset values. */
	dw->map_state = 0;
	dw->visibility_state = VisibilityFullyObscured;
	dw->x = x;
	dw->y = y;
	dw->width = width;
	dw->height = height;
	dw->is_in_focus = 0;
	dw->font = OSWQueryCurrentFont();
	dw->next = NULL;
	dw->prev = NULL;
	dw->mesg = NULL;
	dw->len = 0;
	dw->total_lines = 0;
	dw->longest_line = 0;


	/* Toplevel will be created when mapped. */
	dw->toplevel = 0;
	dw->toplevel_buf = 0;

	memset(&dw->dismiss_btn, 0x00, sizeof(push_button_struct));


	/* Set auto dismiss (not yet supported). */
	dw->auto_dismiss = 0;
	dw->next_auto_dismiss = 0;

	/* Set icon. */
	dw->icon_img = icon;

	/* Add widget to regeristry. */
	WidgetRegAdd(dw, WTYPE_CODE_DIALOG);


	return(0);
}


/*
 *	Redraws dialog.   If mesg is NULL, the previous message
 *	will be used.  If mesg is not NULL, the new message will be
 *	set and drawn.
 */
int DialogWinDraw(
        dialog_win_struct *dw,
        char *mesg
)
{
        int x, y, n;
	unsigned int width, height;
        int len;
	char size_change;

	int line_x_pos, line_y_pos;
        char *strptr;

	int cols_on_display;

	unsigned int icon_width;
        win_attr_t wattr;
        font_t *prev_font;
        char hotkey[PBTN_MAX_HOTKEYS];


        if(dw == NULL)
	    return(-1);


	/* Is there a new message for dialog window? */
	if(mesg == NULL)
	{
	    size_change = 0;
	}
	else
	{
	    size_change = 1;

	    /* Free old message. */
	    free(dw->mesg);
	    dw->mesg = NULL;

	    /* Get length of new message. */
	    len = strlen(mesg);
	    dw->len = len;

	    /* Allocate new message. */
	    dw->mesg = (char *)calloc(1, (len + 1) * sizeof(char));
	    if(dw->mesg == NULL)
		return(-1);

	    strncpy(dw->mesg, mesg, len);

	    /* Get longest line and number of lines. */
	    dw->total_lines = strlines(mesg);
	    dw->longest_line = strlongestline(mesg);
	}


	/* Get icon's width. */
	icon_width = ((dw->icon_img == NULL) ? 0 : dw->icon_img->width);


        /* Recreate resources if not mapped (but do not map yet). */
        if(!dw->map_state)
        {
            /* Calculate new size. */
	    width = MAX((dw->longest_line * DW_CHAR_WIDTH) +
                        (4 * DW_MARGIN) + (int)icon_width,
                        (4 * DW_MARGIN) + (int)icon_width
	    );
	    if(width > osw_gui[0].display_width)
		width = osw_gui[0].display_width;

            height = MAX((dw->total_lines * (DW_CHAR_HEIGHT + 2)) +
                         (5 * DW_MARGIN) + DW_DISMISS_BTN_HEIGHT,
                         (5 * DW_MARGIN) + DW_DISMISS_BTN_HEIGHT +
			 DW_CHAR_HEIGHT + 2 +
			 ((dw->icon_img == NULL) ? 0 : dw->icon_img->height)
	    );
            if(height > osw_gui[0].display_height)
                height = osw_gui[0].display_height;

	    /* Calculate new position (centered). */
	    x = ((int)osw_gui[0].display_width / 2) - ((int)width / 2);
	    y = ((int)osw_gui[0].display_height / 2) - ((int)height / 2);


	    /* Destroy previously allocated resources. */
            PBtnDestroy(&dw->dismiss_btn);
	    OSWDestroyWindow(&dw->toplevel);
            OSWDestroyPixmap(&dw->toplevel_buf);

	    /* Create toplevel and nessasary resources. */
            if(
                OSWCreateWindow(
                    &dw->toplevel,
                    osw_gui[0].root_win,
                    x, y,
                    width, height
                )
            )
                return(-1);
            OSWSetWindowInput(dw->toplevel, OSW_EVENTMASK_TOPLEVEL);
            OSWSetWindowWMProperties(
                dw->toplevel,
                DW_TITLE_STR,           /* Title. */
                DW_ICON_TITLE_STR,      /* Icon title. */
                widget_global.std_icon_pm,      /* Icon. */
                False,                  /* Let WM set coordinates? */
                x, y,
                DW_DISMISS_BTN_WIDTH, DW_DISMISS_BTN_HEIGHT,
                osw_gui[0].display_width, osw_gui[0].display_height,
                WindowFrameStyleFixedFrameOnly,
                NULL, 0
            );
            OSWGetWindowAttributes(dw->toplevel, &wattr);
            dw->x = wattr.x;
            dw->y = wattr.y;
            dw->width = wattr.width;
            dw->height = wattr.height;

	    /* Create pixmap buffer. */
            if(OSWCreatePixmap(
		&dw->toplevel_buf,
                dw->width, dw->height
            ))
                return(-1);

            /* Create dismiss button. */
            PBtnInit(
                &dw->dismiss_btn,
                dw->toplevel,
                ((int)dw->width / 2) - ((int)DW_DISMISS_BTN_WIDTH / 2),
                (int)dw->height - DW_DISMISS_BTN_HEIGHT - DW_MARGIN,
                DW_DISMISS_BTN_WIDTH,
                DW_DISMISS_BTN_HEIGHT,
                "Dismiss",
                PBTN_TALIGN_CENTER,
                NULL,
                dw,
                DialogWinDismissPBCB
            );
            hotkey[0] = ' ';
            hotkey[1] = '\n';       /* Enter. */
            hotkey[2] = 0x1b;       /* Escape. */
            hotkey[3] = '\0';
            PBtnSetHotKeys(&dw->dismiss_btn, hotkey);
	}


        prev_font = OSWQueryCurrentFont();
        OSWSetFont(dw->font);

	/* Recreate pixmap buffer as needed. */
	if(dw->toplevel_buf == 0)
	{
	    OSWGetWindowAttributes(dw->toplevel, &wattr);
            if(OSWCreatePixmap(
		&dw->toplevel_buf,
                wattr.width, wattr.height
            ))
                return(-1);
	}


	if(1)
	{
	    OSWGetWindowAttributes(dw->toplevel, &wattr);

	    /* Draw background. */
	    if(widget_global.force_mono)
	        OSWClearPixmap(
		    dw->toplevel_buf,
		    wattr.width, wattr.height,
		    osw_gui[0].black_pix
	        );
	    else
	        WidgetPutImageTile(
	            dw->toplevel_buf,
	            widget_global.std_bkg_img,
	            wattr.width, wattr.height
		);

	    /* Draw message. */
            if(dw->mesg != NULL)
            {
                /* Calculate maximum possible characters per line. */
                cols_on_display = ((int)osw_gui[0].display_width -
		    (int)icon_width - (3 * DW_MARGIN)) /
		    DW_CHAR_WIDTH;

	        /* Set text color. */
	        if(widget_global.force_mono)
		    OSWSetFgPix(osw_gui[0].white_pix);
	        else
                    OSWSetFgPix(widget_global.normal_text_pix);

	        /* Begin drawing each line. */
	        strptr = dw->mesg;
                line_x_pos = (2 * DW_MARGIN) + (int)icon_width + 3;
                line_y_pos = (DW_CHAR_HEIGHT / 2) + 5 + DW_MARGIN;

	        while(1)
	        {
		    n = strlinelen(strptr);

                    OSWDrawStringLimited(
                        dw->toplevel_buf,
                        line_x_pos, line_y_pos,
                        strptr, MIN(n, cols_on_display)
                    );

		    strptr += n;

                    if(*strptr == '\0')
                        break;

		    strptr++;
                    line_y_pos += DW_CHAR_HEIGHT + 2;
	        }
	    }


	    /* Draw horizontal rule. */
	    y = (int)dw->height - DW_DISMISS_BTN_HEIGHT - (2 * DW_MARGIN) - 5;
            if(widget_global.force_mono)
	    {
                OSWSetFgPix(osw_gui[0].black_pix);
                OSWDrawLine(
		    dw->toplevel_buf,
		    0, y, wattr.width, y
		);
	    }
            else
	    {
		OSWSetFgPix(widget_global.surface_shadow_pix);
		OSWDrawLine(
		    dw->toplevel_buf,
		    0, y, wattr.width, y
		);
		OSWSetFgPix(widget_global.surface_highlight_pix);
		OSWDrawLine(
		    dw->toplevel_buf,
		    0, y - 1, wattr.width, y - 1
		);
	    }

            /* Draw icon. */
	    if((dw->icon_img != NULL) &&
	       !widget_global.force_mono   
	    )
	    {
		x = DW_MARGIN;
		y = MAX(
		    (((int)wattr.height - (4 * DW_MARGIN) -
		    DW_DISMISS_BTN_HEIGHT) / 2) -
		    ((int)dw->icon_img->height / 2),
		    DW_MARGIN
		);

		WidgetPutImageRaised(
                    dw->toplevel_buf,
		    dw->icon_img,
		    x, y,
		    16		/* Altitude. */
		);
	    }

            /* Put buffer to window. */
	    OSWPutBufferToWindow(dw->toplevel, dw->toplevel_buf);

	    OSWGUISync(False);
	}



	/* Map as needed. */
	if(!dw->map_state)
	{
            OSWMapRaised(dw->toplevel);
            dw->map_state = 1;

            PBtnMap(&dw->dismiss_btn);

	    /* Need to sync since this function may be called multiple
             * times within a function that dosen't flush the event
             * buffer.
             */
	    OSWPutBufferToWindow(dw->toplevel, dw->toplevel_buf);
	    OSWGUISync(False);
	}

        OSWSetFont(prev_font);


	return(0);
}


/*
 *	Manages dialog window.
 */
int DialogWinManage(
        dialog_win_struct *dw,
        event_t *event
)
{
        int events_handled = 0;


	if((dw == NULL) ||
	   (event == NULL)
	)
	    return(events_handled);


        if(!dw->map_state &&
           (event->type != MapNotify)
	)
            return(events_handled);


	switch(event->type)
	{
	  /* ********************************************************* */
	  case KeyPress:
	    if(!dw->is_in_focus)
		return(events_handled);

	    break;

          /* ********************************************************* */
          case KeyRelease:
            if(!dw->is_in_focus)     
                return(events_handled);

            break;

          /* ********************************************************* */
          case ButtonPress:
            break;

          /* ********************************************************* */
          case ButtonRelease:
            break;

          /* ********************************************************* */
	  case FocusIn:
            if(event->xany.window == dw->toplevel)
	    {
		dw->is_in_focus = 1;

                events_handled++;
		return(events_handled);
	    }
            break;

          /* ********************************************************* */
          case FocusOut:
            if(event->xany.window == dw->toplevel)
            {
                dw->is_in_focus = 0;
		events_handled++;
		return(events_handled);
            }
            break;

          /* ********************************************************* */
          case ConfigureNotify:
            if(event->xany.window == dw->toplevel)
            {
		events_handled++;
		return(events_handled);
	    }
	    break;

          /* ********************************************************* */
	  case Expose:
            if(event->xany.window == dw->toplevel)
	    {
		DialogWinDraw(dw, NULL);

		events_handled++;
                return(events_handled);
	    }
            break;

          /* ******************************************************** */
          case UnmapNotify:
            if(event->xany.window == dw->toplevel)
            {
		if(dw->map_state)
                    DialogWinUnmap(dw);

                events_handled++;
                return(events_handled);
            }
            break;

          /* ******************************************************** */
          case MapNotify:
            if(event->xany.window == dw->toplevel)
            {
                if(!dw->map_state)
                    DialogWinMap(dw);

                events_handled++;
                return(events_handled);
            }
            break;

          /* ********************************************************* */
          case ClientMessage:
	    if(OSWIsEventDestroyWindow(dw->toplevel, event))
	    {
		DialogWinUnmap(dw);

                events_handled++;
		return(events_handled);
            }
	    break;

          /* ********************************************************* */
          case VisibilityNotify:
            if(event->xany.window == dw->toplevel)
            {
                dw->visibility_state = event->xvisibility.state;

                events_handled++;
                return(events_handled);
            }
	    break;
	}


	/* Redraw as needed. */
	if(events_handled > 0)
	{
            DialogWinDraw(dw, NULL);
	}

	/* Manage widgets. */
	if(events_handled == 0)
	    events_handled += PBtnManage(&dw->dismiss_btn, event);


	return(events_handled);
}

/*
 *	Map dialog window.
 */
void DialogWinMap(dialog_win_struct *dw)
{
        if(dw == NULL)
	    return;

        dw->map_state = 0;
	DialogWinDraw(dw, NULL);


	return;
}

/*
 *	Unmap dialog window.
 */
void DialogWinUnmap(dialog_win_struct *dw)
{
        if(dw == NULL)
            return;


	PBtnDestroy(&dw->dismiss_btn);
	OSWDestroyWindow(&dw->toplevel);
	OSWDestroyPixmap(&dw->toplevel_buf);
        dw->map_state = 0;

        dw->visibility_state = VisibilityFullyObscured;
	dw->is_in_focus = 0;


	return;
}



void DialogWinDestroy(dialog_win_struct *dw)
{
        if(dw == NULL)
	    return;


        /* Delete widget from regeristry. */
        WidgetRegDelete(dw);


	/* Free message. */
	free(dw->mesg);
	dw->mesg = NULL;


	/* Do NOT destroy the icon image, sharing is allowed. */
	dw->icon_img = NULL;


	if(IDC())
	{
	    PBtnDestroy(&dw->dismiss_btn);
	    OSWDestroyWindow(&dw->toplevel);
	    OSWDestroyPixmap(&dw->toplevel_buf);
	}


	dw->map_state = 0;
	dw->is_in_focus = 0;
	dw->visibility_state = VisibilityFullyObscured;
	dw->x = 0;
	dw->y = 0;
	dw->width = 0;
	dw->height = 0;
	dw->disabled = False;
	dw->font = NULL;
        dw->next = NULL;
        dw->prev = NULL;


	dw->len = 0;
	dw->total_lines = 0;
	dw->longest_line = 0;


	return;
}


/*
 *	Macro to set message, map, and draw dialog window.
 */
int printdw(     
        dialog_win_struct *dw,
        char *mesg
)
{
	if(dw == NULL)
	    return(-1);

	dw->map_state = 0;

	return(DialogWinDraw(dw, mesg));
}




