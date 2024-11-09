// widgets/wviewer.cpp
/*
                          Widget: File Viewer

	Functions:

	int ViewerCountLines(char *buf, int len)
	int ViewerUpdateMaxWidth(file_viewer_struct *fv)

	int ViewerLoadFile(
	        file_viewer_struct *fv,
	        char *filename
	)       
	int ViewerLoadData(
                file_viewer_struct *fv,
                void *buf,
		int buf_len
        )
	void ViewerUnload(file_viewer_struct *fv)

	int ViewerInit(
	        file_viewer_struct *fv,
	        int x, int y,
	        unsigned int width, unsigned int height
	)
	int ViewerResize(file_viewer_struct *fv)
	int ViewerDraw(file_viewer_struct *fv)
	int ViewerManage(file_viewer_struct *fv, event_t *event)
	void ViewerMap(file_viewer_struct *fv)
	void ViewerUnmap(file_viewer_struct *fv) 
	void ViewerDestroy(file_viewer_struct *fv)

	---



 */


#include "../include/disk.h"
#include "../include/widget.h"

#ifndef MAX
#define MIN(a,b)        (((a) < (b)) ? (a) : (b))
#define MAX(a,b)	(((a) > (b)) ? (a) : (b))
#endif
/* #define MIN(a,b)        (((a) < (b)) ? (a) : (b)) */
/* #define MAX(a,b)        (((a) > (b)) ? (a) : (b)) */


/* Do not view files larger than this size (in bytes). */
#define VIEWER_MAX_FILESIZE	100000


/*
 *	Size constants (in pixels):
 */
#define VIEWER_CHAR_WIDTH	7
#define VIEWER_CHAR_HEIGHT	14

#define VIEWER_LINE_SPACING	2

#define VIEWER_MARGIN		10
#define VIEWER_DEF_WIDTH	((90 * VIEWER_CHAR_WIDTH) + (2 * VIEWER_MARGIN))
#define VIEWER_DEF_HEIGHT	((26 * VIEWER_CHAR_HEIGHT) + 100)

#define VIEWER_MIN_WIDTH	336
#define VIEWER_MIN_HEIGHT	240


/* Loading chunk size. */
#define FV_LOAD_CHUNK_SIZE      1024


/*
 *	Get number of lines in buf.
 */
int ViewerCountLines(char *buf, int len)
{
	int i, lines;


	if((buf == NULL) ||
           (len < 1)
	)
	    return(0);

	for(lines = 0, i = 0; i < len; i++)
	{
	    if((buf[i] == '\r') ||
               (buf[i] == '\n')
	    )
		lines++;
	}


	return(lines);
}


/*
 *	Updates the maximum width of the data in the viewer
 *	for fv's member max_viewer_width.
 */
int ViewerUpdateMaxWidth(file_viewer_struct *fv)
{
	int i, line_len, longest_line;
	win_attr_t wattr;


        if(fv == NULL)
            return(-1);

	OSWGetWindowAttributes(fv->viewer, &wattr);
	if(fv->buf == NULL)
	{
	    fv->max_viewer_width = wattr.width;
	    return(0);
	}


	/* Calculate longest line. */
	line_len = 0;
	longest_line = 0;
	for(i = 0; i < fv->buf_len; i++)
	{
	    /* Carrage return? */
	    if((fv->buf[i] == '\n') ||
               (fv->buf[i] == '\r')
	    )
	    {
		/* Was this line longer than previous? */
		if(line_len > longest_line)
		    longest_line = line_len;
		line_len = 0;
	    }

	    line_len++;
	}

	/* Update max_viewer_width. */
	if(fv->viewer_mode == VIEWER_MODE_HEX)
	    fv->max_viewer_width = (80 * VIEWER_CHAR_WIDTH) +
                (2 * VIEWER_MARGIN) + 32;
	else
	    fv->max_viewer_width = (longest_line * VIEWER_CHAR_WIDTH) +
                (2 * VIEWER_MARGIN) + 32;


	return(0);
}


/*
 *	Get buffer position corresponding to line number.
 */
int GET_LINE_POSITION(char *buf, int len, int line)
{
        int i;
        int lines;
 

        if((buf == NULL) ||
           (len < 1) ||
           (line < 1)
        )
            return(0);


	lines = 0;
	for(i = 0; i < len; i++)
	{
	    if(lines >= line)
		break;
	    if(buf[i] == '\0')
		break;

	    if( (buf[i] == '\r') ||
                (buf[i] == '\n')
	    )
		lines++;
	}

	/* Sanitize i. */
	if(i >= len)
	    i = len - 1;


	return(i);
}


/*
 *	Get length of line number from pos in buf.
 */
int GET_LINE_LENGTH(char *buf, int pos, int len)
{
	int i;
	int len_check;
	int line_length;


        if((buf == NULL) ||
           (pos < 0) ||
           (len < 1)
        )
            return(0);


	/* Calculate length check. */
	len_check = len - pos;
	if(len_check < 0)
	    len_check = 0;
 

	line_length = 0;
	for(i = 0; i < len_check; i++)
	{
	    if((buf[i] == '\0') ||
               (buf[i] == '\r') ||
	       (buf[i] == '\n') 
	    )
		break;

	    line_length++;
	}
	if(line_length >= len)
	    line_length = len - 1;


	return(line_length);
}


/*
 *	Procedure to load a file into the file viewer.
 */
int ViewerLoadFile(
	file_viewer_struct *fv,
	char *filename
)
{
	off_t filesize;
	off_t memory;
	int i, buf_pos;

	FILE *fp;
	char stringa[256 + PATH_MAX + NAME_MAX];
	char *strptr;
	char need_hex_mode = 0;


	if(fv == NULL)
	    return(-1);


	/* Unload (free) any contents in viewer first. */
	ViewerUnload(fv);


	/* Attempt to open file. */
	if(filename == NULL)
	{
	    strptr = "Cannot open file with NULL filename.\n";
	    fv->buf = (char *)calloc(1, (strlen(strptr) + 1) * sizeof(char));
	    if(fv->buf != NULL)
	    {
		fv->buf_len = strlen(strptr);
		strcpy(fv->buf, strptr);
                fv->lines = ViewerCountLines(fv->buf, fv->buf_len);
	    }

	    OSWSetWindowTitle(fv->toplevel, "Viewer: Error");

	    return(-2);
	}

	fp = fopen(filename, "rb");
	if(fp == NULL)
	{
            sprintf(stringa, "Cannot read file: %s\n", filename);
            strptr = stringa;
            fv->buf = (char *)calloc(1, (strlen(strptr) + 1) * sizeof(char));
            if(fv->buf != NULL)
            {
                fv->buf_len = strlen(strptr);
		strcpy(fv->buf, strptr);
                fv->lines = ViewerCountLines(fv->buf, fv->buf_len);
            }
                
            OSWSetWindowTitle(fv->toplevel, "Viewer: Error");

	    return(-1);
	}


	/* Reset values. */
	filesize = 0;
	memory = 0;
	buf_pos = 0;

	fv->buf = NULL;		/* Buffer already freed but setting NULL just incase. */
	fv->buf_len = 0;
	fv->lines = 0;

	/* Begin reading file. */
	while(1)
	{
	    /* Allocate more memory. */
	    if(memory <= filesize)
	    {
		memory += FV_LOAD_CHUNK_SIZE;
		fv->buf_len = memory;

		fv->buf = (char *)realloc(fv->buf, fv->buf_len * sizeof(char));
		if(fv->buf == NULL)
		{
		    fv->buf_len = 0;
                    fv->lines = 0;
		    return(-1);
		}
	    }

	    /* Read character. */
	    i = fgetc(fp);
	    if(i == EOF)
		break;
	    else
	        fv->buf[buf_pos] = i;

	    /* Check if char is non-ASCII */
	    if(!need_hex_mode)
	    {
		if(!isascii(i))
		    need_hex_mode = 1;
	    }


	    /* Increment values. */
	    filesize++;
	    buf_pos++;
	}


	/* Close file. */
	fclose(fp); fp = NULL;


	/* Set exact size, and NULL terminate. */
	fv->buf_len = filesize;
	fv->buf = (char *)realloc(fv->buf, (fv->buf_len + 1) * sizeof(char));
	if(fv->buf == NULL)
	{
	    fv->buf_len = 0;
	    return(-1);
	}
	fv->buf[fv->buf_len] = '\0';	/* Null terminate. */


	/* Count lines. */
	fv->lines = ViewerCountLines(fv->buf, fv->buf_len);

	/* Check if hex mode is needed. */
	if(need_hex_mode)
	{
	    fv->viewer_mode = VIEWER_MODE_HEX;
	}

	/* Change title. */
        sprintf(stringa, "Viewer: %s (%i bytes)\n",
	    filename,
	    (int)fv->buf_len
	);
        OSWSetWindowTitle(fv->toplevel, stringa);

	/* Update max viewer width. */
	ViewerUpdateMaxWidth(fv);


	return(0);
}


/*
 *	Load data into file voewer (this function is not complete).
 */
int ViewerLoadData(
        file_viewer_struct *fv,
        void *buf,
	int buf_len
)
{
	if((fv == NULL) ||
           (buf == NULL) ||
	   (buf_len < 1)
	)
	    return(-1);

            
        /* Unload (free) any contents in viewer first. */
        ViewerUnload(fv);






	return(0);
}


/*
 *	Unload data from file viewer.
 */
void ViewerUnload(
        file_viewer_struct *fv
)
{
	if(fv == NULL)
	    return;


	free(fv->buf);
	fv->buf = NULL;

	fv->buf_len = 0;
	fv->lines = 0;

	/* Change title. */
	OSWSetWindowTitle(fv->toplevel, "Viewer: Untitled");

        /* Update max viewer width. */
        ViewerUpdateMaxWidth(fv);


	return;
}


/*
 *	Initializes file viewer.
 */
int ViewerInit(
	file_viewer_struct *fv,
	int x, int y,
	unsigned int width, unsigned int height
)
{
	win_attr_t wattr;
        char hotkeys[PBTN_MAX_HOTKEYS];


	if(fv == NULL)
	    return(-1);


	if(width == 0)
	    width = VIEWER_DEF_WIDTH;
	if(height == 0)
	    height = VIEWER_DEF_HEIGHT;


	/* Reset values. */
	fv->map_state = 0;
	fv->is_in_focus = 0;
        fv->visibility_state = VisibilityFullyObscured;
        fv->x = x;
        fv->y = y;
        fv->width = width;
        fv->height = height;
	fv->font = OSWQueryCurrentFont();
        fv->next = NULL;
        fv->prev = NULL;


	/* Create toplevel. */
	if(
	    OSWCreateWindow(
	        &fv->toplevel,
		osw_gui[0].root_win,
		fv->x,
		fv->y,
		fv->width,
	        fv->height
	    )
	)
	    return(-1);

	OSWSetWindowInput(fv->toplevel, OSW_EVENTMASK_TOPLEVEL);

        WidgetCenterWindow(fv->toplevel, WidgetCenterWindowToRoot);
	OSWGetWindowAttributes(fv->toplevel, &wattr);
	fv->x = wattr.x;
	fv->y = wattr.y;
	fv->width = wattr.width;
	fv->height = wattr.height;

	/* WM properties. */
        OSWSetWindowWMProperties(
	    fv->toplevel,
            "Viewer",			/* Title. */
            "Viewer",			/* Icon title. */
            widget_global.std_icon_pm,	/* Icon. */
            False,			/* Let WM set coordinates? */
            /* Coordinates. */
	    fv->x, fv->y,
            /* Min width and height. */
	    VIEWER_MIN_WIDTH, VIEWER_MIN_HEIGHT,
            /* Max width and height. */
            osw_gui[0].display_width, osw_gui[0].display_height,
	    WindowFrameStyleStandard,
            NULL, 0
        );
        OSWSetWindowBkg(fv->toplevel, 0, widget_global.std_bkg_pm);



	/* Viewer. */
	if(
	     OSWCreateWindow(
                &fv->viewer,
                fv->toplevel,
                0, 0,
                fv->width,
                MAX((int)fv->height - 28 - (2 * VIEWER_MARGIN), 10)
            )
        )
            return(-1);
	OSWGetWindowAttributes(fv->viewer, &wattr);
	OSWSetWindowInput(fv->viewer, ButtonPressMask | ButtonReleaseMask |
            PointerMotionMask | ExposureMask);

	if(
	    OSWCreatePixmap(
		&fv->viewer_buf,
	        wattr.width, wattr.height
	    )
	)
	    return(-1);

	/* Scroll bars for viewer. */
	if(
	    SBarInit(
	        &fv->sb,
		fv->viewer,
		wattr.width,
		wattr.height
	    )
	)
	    return(-1);


	/* Push buttons. */
	if(
	    PBtnInit(
		&fv->close_btn,
		fv->toplevel,
		VIEWER_MARGIN,
		(int)fv->height - 28 - VIEWER_MARGIN,
		70, 28,
		"Close",
		PBTN_TALIGN_CENTER,
		NULL,
		(void *)fv,
		NULL
	    )
	)
	    return(-1);
        hotkeys[0] = 0x1b;
        hotkeys[1] = 'c';
        hotkeys[2] = '\0';
        PBtnSetHotKeys(&fv->close_btn, hotkeys);


        if(
            PBtnInit(
                &fv->ascii_mode_btn,
                fv->toplevel,
                (int)fv->width - (2 * 70) - (2 * VIEWER_MARGIN),
                (int)fv->height - 28 - VIEWER_MARGIN,
                70, 28,
                "ASCII",
                PBTN_TALIGN_CENTER,
                NULL,
                (void *)fv,
		NULL           
            )
        )
            return(-1);
        PBtnSetHintMessage(   
            &fv->ascii_mode_btn,
            "Display in ASCII text format."
        );
        PBtnSetHotKeys(&fv->ascii_mode_btn, "a");


        if(
            PBtnInit(
                &fv->hex_mode_btn,
                fv->toplevel,
                (int)fv->width - (1 * 70) - (1 * VIEWER_MARGIN),
                (int)fv->height - 28 - VIEWER_MARGIN,
                70, 28,
                "Hex",
                PBTN_TALIGN_CENTER,
                NULL,
                (void *)fv,
		NULL
            )
        )
            return(-1);
        PBtnSetHintMessage(
            &fv->hex_mode_btn,
            "Display in hexadecimal format."
        );
        PBtnSetHotKeys(&fv->hex_mode_btn, "h");



	fv->filename = NULL;
	fv->buf = NULL;
	fv->buf_len = 0;

	fv->viewer_mode = VIEWER_MODE_ASCII;

	ViewerUpdateMaxWidth(fv);


	/* Add this widget to the widget regeristry. */
	WidgetRegAdd(fv, WTYPE_CODE_VIEWER);


	return(0);
}


/*
 *	Resizes file viewer.
 */
int ViewerResize(file_viewer_struct *fv)
{
	win_attr_t wattr;


	if(fv == NULL)
	    return(-1);

	/* Get new sizes. */
	OSWGetWindowAttributes(fv->toplevel, &wattr);

	/* Check for no change in size. */
	if((fv->width == (unsigned int)wattr.width) &&
           (fv->height == (unsigned int)wattr.height)
	)
	    return(0);

	fv->x = wattr.x;
	fv->y = wattr.y;
	fv->width = wattr.width;
	fv->height = wattr.height;


	/* Viewer. */
	OSWMoveResizeWindow(
	    fv->viewer,
            0, 0,
            fv->width,
            MAX((int)fv->height - 28 - (2 * VIEWER_MARGIN), 10)
	);
        OSWGetWindowAttributes(fv->viewer, &wattr);

        OSWDestroyPixmap(&fv->viewer_buf);
        if(
            OSWCreatePixmap(
                &fv->viewer_buf,
                wattr.width, wattr.height
            )
        )
            return(-1);

        ViewerUpdateMaxWidth(fv);


	/* Viewer scrollbars. */
	SBarResize(
	    &fv->sb,
	    wattr.width,
	    wattr.height
	);


	/* Buttons. */
	OSWMoveWindow(fv->close_btn.toplevel,
            VIEWER_MARGIN,
            (int)fv->height - 28 - VIEWER_MARGIN
        );

	OSWMoveWindow(fv->ascii_mode_btn.toplevel,
            (int)fv->width - (2 * 70) - (2 * VIEWER_MARGIN),
            (int)fv->height - 28 - VIEWER_MARGIN
        );

        OSWMoveWindow(fv->hex_mode_btn.toplevel,
            (int)fv->width - (1 * 70) - (1 * VIEWER_MARGIN),
            (int)fv->height - 28 - VIEWER_MARGIN
	);


	return(0);
}


/*
 *	Draw file viewer.
 */
int ViewerDraw(file_viewer_struct *fv)
{
	win_attr_t wattr_toplevel, wattr;

	int i, n, y_pos, start_line;
	int line_count, lines_drawn, lines_visible, line_length;
	int hex_col_count;

	char stringa[256];
	char stringb[64];
        font_t *prev_font;


        if(fv == NULL)
            return(-1);


	/* Get attributes of toplevel. */
        OSWGetWindowAttributes(fv->toplevel, &wattr_toplevel);
        OSWGetWindowAttributes(fv->viewer, &wattr);

	/* Map as needed. */
	if(!fv->map_state)
	{
	    OSWMapRaised(fv->toplevel);
	    OSWMapWindow(fv->viewer);

	    if(fv->viewer_mode == VIEWER_MODE_HEX)
	        SBarDraw(
		    &fv->sb,
		    wattr.width,
		    wattr.height,
		    fv->max_viewer_width,
     ((fv->buf_len / 16) + 5) * (VIEWER_CHAR_HEIGHT + VIEWER_LINE_SPACING)
	        );
	    else
                SBarDraw(  
                    &fv->sb,
                    wattr.width, 
                    wattr.height,
                    fv->max_viewer_width,
               (fv->lines + 5) * (VIEWER_CHAR_HEIGHT + VIEWER_LINE_SPACING)
                );



	    PBtnMap(&fv->close_btn);
            PBtnMap(&fv->ascii_mode_btn);
            PBtnMap(&fv->hex_mode_btn);


            fv->visibility_state = VisibilityUnobscured;
	    fv->map_state = 1;
	}

        /* ********************************************************* */

        prev_font = OSWQueryCurrentFont();
        OSWSetFont(fv->font);

	/* Toplevel. */
	OSWClearWindow(fv->toplevel);


	/* Viewer. */
        OSWGetWindowAttributes(fv->viewer, &wattr);
	OSWClearPixmap(fv->viewer_buf, wattr.width, wattr.height,
	    widget_global.surface_editable_pix);

	OSWSetFgPix(widget_global.editable_text_pix);

        /* Draw data for ASCII mode. */
	if((fv->buf != NULL) && (fv->viewer_mode == VIEWER_MODE_ASCII))
	{
	    lines_visible = (int)wattr.height /
		(VIEWER_CHAR_HEIGHT + VIEWER_LINE_SPACING);
	    start_line = (int)fv->sb.y_win_pos /
		(VIEWER_CHAR_HEIGHT + VIEWER_LINE_SPACING);
	    lines_drawn = 0;

	    y_pos = 0;
	    for(line_count = start_line;
		line_count < fv->lines;
		line_count++
	    )
	    {
		if(lines_drawn > lines_visible)
		    break;

	        i = GET_LINE_POSITION(fv->buf, fv->buf_len, line_count);
		line_length = GET_LINE_LENGTH(&(fv->buf[i]), i, fv->buf_len);

		OSWDrawStringLimited(
		    fv->viewer_buf,
		    VIEWER_MARGIN - fv->sb.x_win_pos, y_pos + 18,
		    &(fv->buf[i]), line_length
		);


		lines_drawn++;
		y_pos += VIEWER_CHAR_HEIGHT + VIEWER_LINE_SPACING;
	    }
	}
	/* Draw data for hexadecimal mode. */
        if((fv->buf != NULL) && (fv->viewer_mode == VIEWER_MODE_HEX))
        {
            lines_visible = (int)wattr.height /
                (VIEWER_CHAR_HEIGHT + VIEWER_LINE_SPACING);
            start_line = (int)fv->sb.y_win_pos /
                (VIEWER_CHAR_HEIGHT + VIEWER_LINE_SPACING);

	    lines_drawn = 0;
	    y_pos = 0;

	    /* Draw each line. */
	    for(i = start_line * 16;
                i < (int)fv->buf_len;
                i += 16
            )
            {
		/* Do not draw more lines that there are visable. */
                if(lines_drawn > lines_visible) break;

		/* Address (line number). */
		sprintf(stringa, "%.8x  ", i);

		/* Hexadecimal colums section. */
		for(hex_col_count = 0; hex_col_count < 16; hex_col_count++)
		{
		    n = i + hex_col_count;

		    /* Do not exceed buffer length. */
		    if(n >= (int)fv->buf_len) break;

		    /* Hex value. */
		    sprintf(stringb, "%.2x ", (unsigned char)fv->buf[n]);

                    /* Strcat stringb containing value to line stringa. */
                    strcat(stringa, stringb);

		    /* Strcat delimitations. */
                    if(hex_col_count == 3)
                        strcat(stringa, " ");
                    if(hex_col_count == 7)
                        strcat(stringa, "-- ");
                    if(hex_col_count == 11)
                        strcat(stringa, " ");
		}
                OSWDrawString(
                    fv->viewer_buf,
                    VIEWER_MARGIN - fv->sb.x_win_pos, y_pos + 18,
                    stringa
                );

		/* ASCII equivilent section. */
		stringa[0] = '\0';
                for(hex_col_count = 0; hex_col_count < 16; hex_col_count++)
                {
                    n = i + hex_col_count;

                    /* Do not exceed buffer length. */
                    if(n >= (int)fv->buf_len) break;

		    /* ASCII value. */
		    if((0x20 <= (unsigned char)fv->buf[n]) &&
                       ((unsigned char)fv->buf[n] < 0x80)
		    )
		        sprintf(stringb, "%c", (char)fv->buf[n]);
		    else
                        sprintf(stringb, " ");

		    /* Strcat stringb containing value to line stringa. */
		    strcat(stringa, stringb);
		}
                OSWDrawString(
                    fv->viewer_buf,
                    VIEWER_MARGIN - fv->sb.x_win_pos +
                    (VIEWER_CHAR_WIDTH * 65), y_pos + 18,
                    stringa
                );


		/* Increment positions. */
		lines_drawn++;
                y_pos += VIEWER_CHAR_HEIGHT + VIEWER_LINE_SPACING;
	    }
	}

	/* Frame viewer. */
	WidgetFrameButtonPixmap(
	    fv->viewer_buf,
	    True,
	    wattr.width,
	    wattr.height,
	    widget_global.surface_highlight_pix,
	    widget_global.surface_shadow_pix
	);

	OSWPutBufferToWindow(fv->viewer, fv->viewer_buf);

        OSWSetFont(prev_font);


	return(0);
}


/*
 *	Manage file viewer.
 */
int ViewerManage(file_viewer_struct *fv, event_t *event)
{
	win_attr_t wattr;
	int max_y_pos;
        int events_handled = 0;


	if((event == NULL) ||
           (fv == NULL)
	)
	    return(events_handled);

	if(!fv->map_state &&
           (event->type != MapNotify)
	)
	    return(events_handled);


	switch(event->type)
	{
          /* ********************************************************* */
          case KeyPress:
            /* Do not handle KeyPress if out of focus. */
            if(!fv->is_in_focus)
                return(events_handled);

	    /* Space bar. */
	    if(event->xkey.keycode == osw_keycode.space)
	    {
                OSWGetWindowAttributes(fv->viewer, &wattr);

                if(fv->viewer_mode == VIEWER_MODE_HEX)
                    max_y_pos = ((fv->buf_len / 16) + 5) *
                        (VIEWER_CHAR_HEIGHT + VIEWER_LINE_SPACING);
                else
                    max_y_pos = (fv->lines + 5) *
                        (VIEWER_CHAR_HEIGHT + VIEWER_LINE_SPACING);

		max_y_pos = max_y_pos - (int)wattr.height;

		fv->sb.y_win_pos += (int)wattr.height;
		if(fv->sb.y_win_pos > max_y_pos)
		    fv->sb.y_win_pos = max_y_pos;
		if(fv->sb.y_win_pos < 0)
		    fv->sb.y_win_pos = 0;

		/* Redraw. */
                if(fv->viewer_mode == VIEWER_MODE_HEX)
                    SBarDraw(
                        &fv->sb,
                        wattr.width,
                        wattr.height,
                        fv->max_viewer_width,
       ((fv->buf_len / 16) + 5) * (VIEWER_CHAR_HEIGHT + VIEWER_LINE_SPACING)
                    );
                else
                    SBarDraw(
                        &fv->sb,
                        wattr.width,
                        wattr.height,
                        fv->max_viewer_width,
               (fv->lines + 5) * (VIEWER_CHAR_HEIGHT + VIEWER_LINE_SPACING)
                    );
		ViewerDraw(fv);
	    }
	    /* B or minus. */
	    else if((event->xkey.keycode == osw_keycode.alpha_b) ||
                    (event->xkey.keycode == osw_keycode.minus)
	    )
	    {
                OSWGetWindowAttributes(fv->viewer, &wattr);

		if(fv->viewer_mode == VIEWER_MODE_HEX)
                    max_y_pos = ((fv->buf_len / 16) + 5) *
                        (VIEWER_CHAR_HEIGHT + VIEWER_LINE_SPACING);
		else
                    max_y_pos = (fv->lines + 5) *
                        (VIEWER_CHAR_HEIGHT + VIEWER_LINE_SPACING);

                max_y_pos = max_y_pos - (int)wattr.height;

                fv->sb.y_win_pos -= (int)wattr.height;
                if(fv->sb.y_win_pos > max_y_pos)
                    fv->sb.y_win_pos = max_y_pos;
                if(fv->sb.y_win_pos < 0)
                    fv->sb.y_win_pos = 0;

                /* Redraw. */
                if(fv->viewer_mode == VIEWER_MODE_HEX)
                    SBarDraw(
                        &fv->sb,
                        wattr.width,
                        wattr.height, 
                        fv->max_viewer_width,
       ((fv->buf_len / 16) + 5) * (VIEWER_CHAR_HEIGHT + VIEWER_LINE_SPACING)
                    );  
                else
                    SBarDraw(
                        &fv->sb,
                        wattr.width,
                        wattr.height,
                        fv->max_viewer_width,
               (fv->lines + 5) * (VIEWER_CHAR_HEIGHT + VIEWER_LINE_SPACING)
                    );
                ViewerDraw(fv);
	    }
	    break;

          /* ********************************************************* */
          case KeyRelease:
            /* Do not handle KeyPress if out of focus. */
            if(!fv->is_in_focus)
                return(events_handled);

            break;

          /* ********************************************************* */
          case ButtonPress:
	    if((event->xany.window == fv->toplevel) ||
               (event->xany.window == fv->viewer)
            )
		fv->is_in_focus = 1;

	    /* Viewer window. */
	    if(event->xany.window == fv->viewer)
	    {
		fv->sb.is_in_focus = 1;
		events_handled++;
	    }

	    break;

          /* ********************************************************** */
          case Expose:
            if((event->xany.window == fv->toplevel) ||
               (event->xany.window == fv->viewer)
            )
	    {
                events_handled++;
	    }
            break;

          /* ******************************************************** */
          case UnmapNotify:
            if(event->xany.window == fv->toplevel)
            {
		if(fv->map_state)
                    ViewerUnmap(fv);

                events_handled++;
                return(events_handled);
            }
            break;

          /* ******************************************************** */
          case MapNotify:
            if(event->xany.window == fv->toplevel)
            {
		if(!fv->map_state)
		    ViewerMap(fv);

                events_handled++;
                return(events_handled);
            }
            break;

	  /* ********************************************************* */
          case ConfigureNotify:
	    if((event->xany.window == fv->toplevel) ||
               (event->xany.window == fv->viewer)
            )
	    {
		ViewerResize(fv);

		ViewerDraw(fv);

		OSWGetWindowAttributes(fv->viewer, &wattr);
                if(fv->viewer_mode == VIEWER_MODE_HEX)
                    SBarDraw(
                        &fv->sb,
                        wattr.width,
                        wattr.height,
                        fv->max_viewer_width,
       ((fv->buf_len / 16) + 5) * (VIEWER_CHAR_HEIGHT + VIEWER_LINE_SPACING)
                    );
                else
                    SBarDraw(
                        &fv->sb,
                        wattr.width,
                        wattr.height,
                        fv->max_viewer_width,
               (fv->lines + 5) * (VIEWER_CHAR_HEIGHT + VIEWER_LINE_SPACING)
                    );

		events_handled++;
		return(events_handled);
	    }

	    break;

          /* ********************************************************* */
          case ClientMessage:
            if(OSWIsEventDestroyWindow(fv->toplevel, event))
            {
                ViewerUnmap(fv);

                events_handled++;
                return(events_handled);
            }
            break;

          /* ********************************************************* */
          case FocusIn:
            if(event->xany.window == fv->toplevel)
            {
                fv->is_in_focus = 1;

                events_handled++;
		return(events_handled);
            }
            break;

          /* ********************************************************* */
          case FocusOut:
            if(event->xany.window == fv->toplevel)
            {
                fv->is_in_focus = 0;

                events_handled++;
		return(events_handled);
            }
            break;

          /* ********************************************************* */
          case VisibilityNotify:
            if(event->xany.window == fv->toplevel)
            {
                fv->visibility_state = event->xvisibility.state;

                events_handled++;
                return(events_handled);
            }
            break;
	}

	if(events_handled > 0)
	{
	    ViewerDraw(fv);
	}


        /* ************************************************************ */
	/* Manage widgets. */

	/* Scroll bar. */
	if(events_handled == 0)
	{
	    OSWGetWindowAttributes(fv->viewer, &wattr);
            if(fv->viewer_mode == VIEWER_MODE_HEX)
                events_handled += SBarManage(
                    &fv->sb,
                    wattr.width,
                    wattr.height,
                    fv->max_viewer_width,
     ((fv->buf_len / 16) + 5) * (VIEWER_CHAR_HEIGHT + VIEWER_LINE_SPACING),
                    event
                );  
            else  
                events_handled += SBarManage(
                    &(fv->sb),
                    wattr.width,
                    wattr.height,
                    fv->max_viewer_width,
               (fv->lines + 5) * (VIEWER_CHAR_HEIGHT + VIEWER_LINE_SPACING),
		    event
                );
	    if(events_handled > 0)
                ViewerDraw(fv);
	}

        /* Close button. */
        if(events_handled == 0)
        {
            events_handled += PBtnManage(
                &fv->close_btn,
                event
            );
	    if(((event->type == ButtonRelease) ||
               (event->type == KeyRelease)) &&
               (events_handled > 0)
            )
            {
		ViewerUnmap(fv);
		return(events_handled);
	    }
        }

        /* ASCII mode button. */  
        if(events_handled == 0)
        {
            events_handled += PBtnManage(
                &(fv->ascii_mode_btn),
                event
            );
            if(((event->type == ButtonRelease) ||
               (event->type == KeyRelease)) &&
               (events_handled > 0)
            )
	    {
		fv->viewer_mode = VIEWER_MODE_ASCII;
                ViewerUpdateMaxWidth(fv);
                ViewerDraw(fv);

                OSWGetWindowAttributes(fv->viewer, &wattr);
                if(fv->viewer_mode == VIEWER_MODE_HEX)   
                    SBarDraw(
                        &fv->sb,
                        wattr.width,
                        wattr.height,
                        fv->max_viewer_width,
       ((fv->buf_len / 16) + 5) * (VIEWER_CHAR_HEIGHT + VIEWER_LINE_SPACING)
                    );
                else
                    SBarDraw(
                        &fv->sb,
                        wattr.width,
                        wattr.height,
                        fv->max_viewer_width,
               (fv->lines + 5) * (VIEWER_CHAR_HEIGHT + VIEWER_LINE_SPACING)
                    );
	    }
        }

        /* Hex mode button. */
        if(events_handled == 0)
        {
            events_handled += PBtnManage(
                &(fv->hex_mode_btn),
                event
            );
            if(((event->type == ButtonRelease) ||
               (event->type == KeyRelease)) &&
               (events_handled > 0)
            )
	    {
		fv->viewer_mode = VIEWER_MODE_HEX;
                ViewerUpdateMaxWidth(fv);
                ViewerDraw(fv);

                OSWGetWindowAttributes(fv->viewer, &wattr);
                if(fv->viewer_mode == VIEWER_MODE_HEX)   
                    SBarDraw(
                        &fv->sb,
                        wattr.width,
                        wattr.height,
                        fv->max_viewer_width,
       ((fv->buf_len / 16) + 5) * (VIEWER_CHAR_HEIGHT + VIEWER_LINE_SPACING)
                    );
                else
                    SBarDraw(
                        &fv->sb,
                        wattr.width,
                        wattr.height,
                        fv->max_viewer_width,
               (fv->lines + 5) * (VIEWER_CHAR_HEIGHT + VIEWER_LINE_SPACING)
                    );
	    }
        }


	return(events_handled);
}

/*
 *	Map file viewer.
 */
void ViewerMap(file_viewer_struct *fv)
{
	if(fv == NULL)
	    return;


	fv->map_state = 0;
	ViewerDraw(fv);


	return;
}


/*
 *	Unmap file viewer.
 */
void ViewerUnmap(file_viewer_struct *fv)
{
        if(fv == NULL)
            return;


	OSWUnmapWindow(fv->toplevel);
	fv->map_state = 0;


	return;       
}


/*
 *	Destroy file viewer.
 */
void ViewerDestroy(file_viewer_struct *fv)
{
	if(fv == NULL)
	    return;


        /* Delete widget from regeristry. */
        WidgetRegDelete(fv);


	/* Free loaded data. */
	ViewerUnload(fv);

	if(IDC())
	{
	    SBarDestroy(&fv->sb);
	    PBtnDestroy(&fv->close_btn);
            PBtnDestroy(&fv->ascii_mode_btn);
            PBtnDestroy(&fv->hex_mode_btn);

	    OSWDestroyPixmap(&fv->viewer_buf);
	    OSWDestroyWindow(&fv->viewer);
	    OSWDestroyWindow(&fv->toplevel);
	}


	fv->map_state = 0;
        fv->visibility_state = VisibilityFullyObscured;
	fv->is_in_focus = 0;
        fv->x = 0;
        fv->y = 0;
        fv->width = 0;
        fv->height = 0;
	fv->font = NULL;
        fv->next = NULL;
        fv->prev = NULL;

        fv->viewer_mode = VIEWER_MODE_ASCII;


	return;
}




