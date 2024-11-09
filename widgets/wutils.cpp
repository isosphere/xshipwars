// widgets/wutils.cpp
/*
                Widget: Interface Layer and Utility Functions


	Create and Destroy Functions:

	pixel_t WidgetGetPixel(char *clsp)

	WCursor *WidgetCreateCursorFromFile(
		char *xpmfile,
		int hot_x, int hot_y,
		WColorStruct color
	)
	WCursor *WidgetCreateCursorFromData(
                char **xpmdata,
                int hot_x, int hot_y,
                WColorStruct color
        )
	void WidgetDestroyCursor(WCursor **wcursor)


	Setting and getting:

        void WidgetSetWindowCursor(win_t w, WCursor *wcursor) 
	void WidgetCenterWindow(win_t w, int relation)


 	Utility Functions:

	void WidgetMap(void *ptr)
	void WidgetUnmap(void *ptr)
	void WidgetDestroy(void *ptr)


	Image IO:

	void WidgetResizeImageBuffer(
	        depth_t d,
	        u_int8_t *tar_buf,
	        u_int8_t *src_buf,
	        unsigned int tar_width,
	        unsigned int tar_height,
	        unsigned int src_width,
	        unsigned int src_height
	)

	void WidgetPutImageTile(
		drawable_t tar_d, image_t *src_img,  
		unsigned int tar_width, unsigned int tar_height
	)
	void WidgetPutPixmapTile(
		drawable_t tar_d, pixmap_t src_pm,  
		unsigned int tar_width, unsigned int tar_height,
		unsigned int src_width, unsigned int src_height
	)

	void WidgetFrameButton(
		win_t w, bool_t state,
		pixel_t fg_pix, pixel_t bg_pix
	)
	void WidgetFrameButtonPixmap(
		pixmap_t pixmap,
		bool_t state,
		unsigned int width, unsigned int height,
		pixel_t fg_pix, pixel_t bg_pix
	)

	image_t *WidgetCreateImageText(
		char *string,
		font_t *font,
		unsigned int font_width, unsigned int font_height,
		pixel_t fg_pix,
		pixel_t bg_pix
	);

	pixmap_t WidgetPixmapMaskFromImage(image_t *image)

	void WidgetPutImageNormal(
		drawable_t d,
		image_t *ximage,
		int tar_x, int tar_y,
		bool_t allow_transparency
	)
	void WidgetPutImageRaised(
		drawable_t d,
		image_t *ximage,
		int tar_x, int tar_y,
		unsigned int altitude
	)

	void WidgetAdjustImageGamma(
		image_t *image,
		double r, double g, double b
	)

	---

 */

#include <sys/types.h>
#include <sys/stat.h>
#include <X11/xpm.h>

#include "../include/widget.h"

#ifndef MAX
#define MIN(a,b)        (((a) < (b)) ? (a) : (b))
#define MAX(a,b)	(((a) > (b)) ? (a) : (b))
#endif
/* #define MIN(a,b)	((a) < (b) ? (a) : (b)) */
/* #define MAX(a,b)	((a) > (b) ? (a) : (b)) */

#define SUBMIN(a,b)	((a) < (b) ? (a) : 0)



/*
 *      Allocates a pixel given a color specification string.
 *	The pixel is the ID for the allocated color.
 */
pixel_t WidgetGetPixel(char *clsp)
{
        static XColor xc;
            
        /* Error checks. */
        if(!IDC())
            return(0);

        /* Make sure color spec clsp is valid. */
        if(clsp == NULL)
            return(osw_gui[0].white_pix);

        /* Parse color. */
        memset(&xc, 0x00, sizeof(XColor));
        switch(XParseColor(
	    osw_gui[0].display,
	    osw_gui[0].colormap,
	    clsp, &xc
	))
        {
          case BadColor:
            fprintf(stderr,
                "%s: BadColor.\n",
                clsp
            );
            break;
  
          case BadValue:   
            fprintf(stderr,
                "%s: BadValue.\n",
                clsp
            );
            break;
        }

        /* Allocate color. */
        if(XAllocColor(osw_gui[0].display, osw_gui[0].colormap, &xc) == 0)
        {
            fprintf(stderr,
                "%s: Cannot allocate color.\n",
                clsp
            );
	    return(osw_gui[0].white_pix);
        }
  
        /* Return the pixel. */
        return(xc.pixel);
}

/*
 *	Creates a new widget cursor from an XPM file.
 */
WCursor *WidgetCreateCursorFromFile(
        char *xpmfile,
	int hot_x, int hot_y,
        WColorStruct color
)
{
#ifndef X_H
        return(NULL);
#else
        int status;
        unsigned int width, height;
        unsigned int req_width, req_height;

        pixmap_t tmp_pixmap, pixmap, mask;
        pixel_t white_pix, black_pix;
        WCursor *wcursor;

        GC gc;
        XGCValues gcv;
        XColor foreground_color, background_color;
        XpmAttributes xpmattr;
	struct stat stat_buf;


	if(!IDC() ||
           (osw_gui[0].root_win == 0)
	)
	    return(NULL);


	/* Make sure file exists. */
	if(stat(xpmfile, &stat_buf))
	{
	    fprintf(stderr,
		"%s: No such file.\n",
		xpmfile
	    );
	    return(NULL);
	}


        /* Set up XPM attributes. */
	memset(&xpmattr, 0x00, sizeof(XpmAttributes));
        xpmattr.closeness = XpmDefaultColorCloseness;
        xpmattr.valuemask = XpmSize | XpmCloseness | XpmDepth;

        /*   pixmap_t depth must be 1 for XCreatePixmapCursor(),
         *   regardless of actual depth.
         */
        xpmattr.depth = 1;


	status = XpmReadFileToPixmap(
	    osw_gui[0].display,
	    osw_gui[0].root_win,
	    xpmfile,
	    &pixmap,
	    &mask,
	    &xpmattr
	);
        if(status != XpmSuccess)
        {
            fprintf(stderr,
		"%s: Unable to load Pixmap.\n",
		xpmfile
            );
	    return(NULL);
        }

        /* Get size of loaded pixmap and mask. */
        width = xpmattr.width;
        height = xpmattr.height;

        /* Check recommended size from GUI. */
        status = XQueryBestCursor(
            osw_gui[0].display,
            osw_gui[0].root_win,
            width, height,
            &req_width, &req_height
        );
        if(!status)
        {
           fprintf(stderr,
 "WidgetCreateCursorFromFile(): Cannot get recommended cursor size for %i %i\n",
                width, height
            );
            req_width = width;
            req_height = height;
        }

        /* Need to reduce size of pixmap and mask? */
        if((width > req_width) ||
           (height > req_height)
        )
        {
            /* Create tempory pixmap and copy pixmap contents to it. */
            tmp_pixmap = XCreatePixmap(
                osw_gui[0].display,
                osw_gui[0].root_win,
                req_width, req_height,
                osw_gui[0].depth
            );
            if(tmp_pixmap != 0)
            {
                XCopyArea(
                    osw_gui[0].display,
                    pixmap,             /* Src. */
                    tmp_pixmap,         /* Tar. */
                    osw_gui[0].gc,
                    0, 0,
                    req_width, req_height,
                    0, 0
                );
                OSWGUISync(False);
                OSWDestroyPixmap(&pixmap);
                pixmap = tmp_pixmap;
            }

            /* Create tempory mask and copy over. */
            white_pix = osw_gui[0].white_pix;
            black_pix = osw_gui[0].black_pix;
            gcv.function = GXcopy;
            gcv.plane_mask = 1;         /* Monochrome. */
            gcv.foreground = white_pix;
            gcv.background = black_pix;
            gcv.line_width = 1;
            gc = XCreateGC(
                osw_gui[0].display,
                mask,  
                GCFunction | GCPlaneMask | GCForeground | GCBackground |
                    GCLineWidth,
                &gcv
            );

            tmp_pixmap = XCreatePixmap( 
                osw_gui[0].display,
                osw_gui[0].root_win,
                req_width, req_height,
                1
            );
            if(tmp_pixmap != 0)
            {
                XCopyArea(
                    osw_gui[0].display,
                    mask,               /* Src. */
                    tmp_pixmap,         /* Tar. */
                    gc,
                    0, 0,
                    req_width, req_height,
                    0, 0
                );
                OSWGUISync(False);
                OSWDestroyPixmap(&mask);
                mask = tmp_pixmap;
            }

            XFreeGC(osw_gui[0].display, gc);        /* Free temp GC. */
        }

	/* Set colors. */
        foreground_color.red = ((u_int16_t)color.r << 8);
        foreground_color.green = ((u_int16_t)color.g << 8);
        foreground_color.blue = ((u_int16_t)color.b << 8);

        background_color.red = 0x0000;
        background_color.green = 0x0000;
        background_color.blue = 0x0000;


	/* Allocate memory for WCursor. */
	wcursor = (WCursor *)calloc(1, sizeof(WCursor));
	if(wcursor == NULL)
	{
	    fprintf(stderr,
		"WidgetCreateCursorFromFile(): Memory allocation error.\n"
	    );
	    OSWDestroyPixmap(&pixmap);
            OSWDestroyPixmap(&mask);

	    return(NULL);
	}

	/* Sanitize hot point. */
	if(hot_x >= (int)xpmattr.width)
	    hot_x = (int)xpmattr.width - 1;
	if(hot_x < 0)
	    hot_x = 0;

        if(hot_y >= (int)xpmattr.height)
            hot_y = (int)xpmattr.height - 1;
	if(hot_y < 0)
	    hot_y = 0;


	/* Create cursor. */
	wcursor->cursor = XCreatePixmapCursor(
            osw_gui[0].display,
            pixmap,
            mask,
            &foreground_color,
            &background_color,
            hot_x, hot_y
        );
        /* Free pixmap and mask. */
        OSWDestroyPixmap(&pixmap);
        OSWDestroyPixmap(&mask);
        /* Cursor create failed? */
        if(wcursor->cursor == 0)
        {
            free(wcursor); wcursor = NULL;
            return(NULL);
        }

	/* Set values to newly allocated WCursor. */
        wcursor->x = hot_x;
        wcursor->y = hot_y;
	wcursor->width = xpmattr.width;
        wcursor->height = xpmattr.height;

        wcursor->depth = 1;	/* X requires that depth be 1. */

        wcursor->color.r = color.r;
        wcursor->color.g = color.g;
        wcursor->color.b = color.b;


	return(wcursor);
#endif	/* X_H */
}

/*
 *	Creates a widget cursor from XPM data.
 */
WCursor *WidgetCreateCursorFromData(
	char **xpmdata,
        int hot_x, int hot_y,
        WColorStruct color
)
{
#ifndef X_H
	return(NULL);
#else
        int status;
	unsigned int width, height;
	unsigned int req_width, req_height;

        pixmap_t tmp_pixmap, pixmap, mask;
	pixel_t white_pix, black_pix;
	WCursor *wcursor;

        GC gc;
        XGCValues gcv;
        XColor foreground_color, background_color;
        XpmAttributes xpmattr;


        if(!IDC() ||
           (osw_gui[0].root_win == 0) ||
           (xpmdata == NULL)
        )
            return(NULL);  


        /* Set up XPM attributes. */
        memset(&xpmattr, 0x00, sizeof(XpmAttributes));
        xpmattr.closeness = XpmDefaultColorCloseness;
        xpmattr.valuemask = XpmSize | XpmCloseness | XpmDepth;
                
        /*   pixmap_t depth must be 1 for XCreatePixmapCursor(),
         *   regardless of actual depth.
         */
        xpmattr.depth = 1;


        status = XpmCreatePixmapFromData(
            osw_gui[0].display,
            osw_gui[0].root_win,
            xpmdata,
            &pixmap,
            &mask,
            &xpmattr
        );
        if(status != XpmSuccess)
        {
            fprintf(stderr,
                "0x%.8x: Unable to load embedded Pixmap.\n",
                (u_int32_t)xpmdata
            );
            return(NULL);
        }

	/* Get size of loaded pixmap and mask. */
	width = xpmattr.width;
	height = xpmattr.height;

	/* Check recommended size from GUI. */
	status = XQueryBestCursor(
	    osw_gui[0].display,
	    osw_gui[0].root_win,
	    width, height,
	    &req_width, &req_height
	);
	if(!status)
	{
           fprintf(stderr,
 "WidgetCreateCursorFromData(): Cannot get recommended cursor size for %i %i\n",
                width, height
            );
	    req_width = width;
	    req_height = height;
	}

	/* Need to reduce size of pixmap and mask? */
	if((width > req_width) ||
           (height > req_height)
	)
	{
	    /* Create tempory pixmap and copy pixmap contents to it. */
	    tmp_pixmap = XCreatePixmap(
		osw_gui[0].display,
		osw_gui[0].root_win,
		req_width, req_height,
		osw_gui[0].depth
	    );
	    if(tmp_pixmap != 0)
	    {
		XCopyArea(
		    osw_gui[0].display,
		    pixmap,		/* Src. */
		    tmp_pixmap,		/* Tar. */
		    osw_gui[0].gc,
		    0, 0,
		    req_width, req_height,
		    0, 0
		);
		OSWGUISync(False);
		OSWDestroyPixmap(&pixmap);
		pixmap = tmp_pixmap;
	    }


	    /* Create tempory mask and copy over. */
            white_pix = osw_gui[0].white_pix;
            black_pix = osw_gui[0].black_pix;
            gcv.function = GXcopy;
            gcv.plane_mask = 1;		/* Monochrome. */
            gcv.foreground = white_pix;
            gcv.background = black_pix;
            gcv.line_width = 1;
            gc = XCreateGC(   
                osw_gui[0].display,
                mask,
                GCFunction | GCPlaneMask | GCForeground | GCBackground |
                    GCLineWidth,
                &gcv
            );

            tmp_pixmap = XCreatePixmap(   
                osw_gui[0].display, 
                osw_gui[0].root_win,
                req_width, req_height,
                1
            );
            if(tmp_pixmap != 0)
            {
                XCopyArea(
                    osw_gui[0].display,
                    mask,		/* Src. */
                    tmp_pixmap,		/* Tar. */
                    gc,
                    0, 0,
                    req_width, req_height,
                    0, 0
                );
                OSWGUISync(False);
                OSWDestroyPixmap(&mask);
                mask = tmp_pixmap;
            } 

            XFreeGC(osw_gui[0].display, gc);        /* Free temp GC. */
	}

        /* Set colors. */  
        foreground_color.red = ((u_int16_t)color.r << 8);
        foreground_color.green = ((u_int16_t)color.g << 8);
        foreground_color.blue = ((u_int16_t)color.b << 8);
  
        background_color.red = 0x0000; 
        background_color.green = 0x0000;   
        background_color.blue = 0x0000;

        /* Allocate memory for WCursor. */
        wcursor = (WCursor *)calloc(1, sizeof(WCursor));
        if(wcursor == NULL)
        {
            fprintf(stderr,
                "WidgetCreateCursorFromData(): Memory allocation error.\n"
            );
            OSWDestroyPixmap(&pixmap);
            OSWDestroyPixmap(&mask);

            return(NULL);
        }


        /* Sanitize hot point. */
        if(hot_x >= (int)xpmattr.width)
            hot_x = (int)xpmattr.width - 1;
        if(hot_x < 0)
            hot_x = 0;     
         
        if(hot_y >= (int)xpmattr.height)
            hot_y = (int)xpmattr.height - 1;
        if(hot_y < 0)
            hot_y = 0;


        /* Create cursor. */
        wcursor->cursor = XCreatePixmapCursor(
            osw_gui[0].display,
            pixmap,
            mask,
            &foreground_color,
            &background_color,
            hot_x, hot_y
        );
        /* Free pixmap and mask. */
        OSWDestroyPixmap(&pixmap);
        OSWDestroyPixmap(&mask);
	/* Cursor create failed? */
        if(wcursor->cursor == 0)
        {
            free(wcursor); wcursor = NULL;
            return(NULL);
        }

	/* Set values to newly allocated WCursor. */
        wcursor->x = hot_x;
        wcursor->y = hot_y;
        wcursor->width = xpmattr.width;
        wcursor->height = xpmattr.height;
  
        wcursor->depth = 1;     /* X requires that depth be 1. */
  
        wcursor->color.r = color.r;
        wcursor->color.g = color.g;
        wcursor->color.b = color.b;


        return(wcursor);
#endif	/* X_H */
}



/*
 *	Sets a window to use the given WCursor.
 */
void WidgetSetWindowCursor(win_t w, WCursor *wcursor)
{
	if(!IDC() ||
	   (w == 0) ||
	   (wcursor == NULL)
	)
	    return;

	if(wcursor->cursor == 0)
	    return;

	/* Set win_t w to use new WCursor. */
	OSWSetWindowCursor(w, wcursor->cursor);

	return;
}

/*
 *	Destroys an allocated WCursor.
 */
void WidgetDestroyCursor(WCursor **wcursor)
{
	if(!IDC() ||
           (wcursor == NULL)
	)
	    return;

	if(*wcursor == NULL)
	    return;

	/* Free actual allocated cursor. */
	OSWDestroyCursor(&((*wcursor)->cursor));

	/* Free cursor structure itself and set pointer to it NULL. */
	free(*wcursor);
	*wcursor = NULL;

	return;
}

/*
 *	Moves window w to the center.
 */
void WidgetCenterWindow(win_t w, int relation)
{
        win_t parent;
	win_attr_t wattr, parent_wattr;
        int pointer_x, pointer_y;
        int x, y;


	if(!IDC() ||
           (w == 0) ||
           (osw_gui[0].root_win == 0)
	)
	    return;

	switch(relation)
	{
	  case WidgetCenterWindowToRoot:
	    OSWGetWindowAttributes(w, &wattr);

            /* Move the window to center it. */     
            OSWMoveWindow(
		w,
                ((int)osw_gui[0].display_width / 2) - ((int)wattr.width / 2),
                ((int)osw_gui[0].display_height / 2) - ((int)wattr.height / 2)
            );
	    break;

	  case WidgetCenterWindowToPointer:
            OSWGetWindowAttributes(w, &wattr);
	    OSWGetPointerCoords(w, &pointer_x, &pointer_y,
		&x, &y);

	    x = pointer_x - ((int)wattr.width / 2);
	    y = pointer_y - ((int)wattr.height / 2);

	    if((x + (int)wattr.width) > (int)osw_gui[0].display_width)
		x = (int)osw_gui[0].display_width - (int)wattr.width;
            if((y + (int)wattr.height) > (int)osw_gui[0].display_height)
                y = (int)osw_gui[0].display_height - (int)wattr.height;
	    if(x < 0)
		x = 0;
	    if(y < 0)
		y = 0;

	    OSWMoveWindow(w, x, y);
            break;

	  default:	/* WidgetCenterWindowToParent */
	    /* Get parent window. */
	    parent = OSWGetWindowParent(w);
	    if(parent == 0)
	        return;
	    OSWGetWindowAttributes(parent, &parent_wattr);

	    /* Get window attributes. */
	    OSWGetWindowAttributes(w, &wattr);

	    /* Move the window to center it. */
	    OSWMoveWindow(
		w,
	        ((int)parent_wattr.width / 2) - ((int)wattr.width / 2),
	        ((int)parent_wattr.height / 2) - ((int)wattr.height / 2)
	    );
	    break;
	}


	return;
}

/*
 *	Universal call to map a widget regardless of its type.
 */
void WidgetMap(void *ptr)
{
	int i, wtype = WTYPE_CODE_NONE;


	if(ptr == NULL)
	    return;

	for(i = 0; i < widget_reg.total_entries; i++)
	{
	    if(widget_reg.entry[i] == NULL)
		continue;

	    if(widget_reg.entry[i]->ptr == ptr)
	    {
		wtype = widget_reg.entry[i]->type;
		break;
	    }
	}
	if(i >= widget_reg.total_entries)
	    return;

	switch(wtype)
	{
	  case WTYPE_CODE_PUSHBUTTON:
            PBtnMap((push_button_struct *)ptr);
            break;

	  case WTYPE_CODE_COLUMLIST:
            CListMap((colum_list_struct *)ptr);  
            break;

	  case WTYPE_CODE_DIALOG:
            DialogWinMap((dialog_win_struct *)ptr);
            break;

	  case WTYPE_CODE_FILEBROWSER:
            FBrowserMap((fbrowser_struct *)ptr);
            break;

          case WTYPE_CODE_LIST:
            ListWinMap((list_window_struct *)ptr);
            break;

          case WTYPE_CODE_MENU:
            MenuMap((menu_struct *)ptr);
            break;

          case WTYPE_CODE_MENUBAR:
            MenuBarMap((menu_bar_struct *)ptr);
            break;

          case WTYPE_CODE_PAGESTEPPER:
            PStepperMap((page_stepper_struct *)ptr);
            break;

          case WTYPE_CODE_PROGRESSBAR:
            PBarMap((progress_bar_struct *)ptr);
            break;

          case WTYPE_CODE_PROMPT:
            PromptMap((prompt_window_struct *)ptr);
            break;

          case WTYPE_CODE_PULIST:
            PUListMap((popup_list_struct *)ptr);
            break;

          case WTYPE_CODE_SCALEBAR:
            ScaleBarMap((scale_bar_struct *)ptr);
            break;

          case WTYPE_CODE_SCROLLBAR:
            /* ScrollBar widget has no map function. */
            break;

          case WTYPE_CODE_TOGGLEARRAY:
            TgBtnArrayMap((toggle_button_array_struct *)ptr);
            break;

          case WTYPE_CODE_TOGGLEBTN:
            TgBtnMap((toggle_button_struct *)ptr);
            break;

          case WTYPE_CODE_VIEWER:
            ViewerMap((file_viewer_struct *)ptr);
            break;

	  default:
	    fprintf(stderr,
 "WidgetMap(): Unsupported widget type code `%i'.\n",
		wtype
	    );
	    break;
	}


	return;
}

/*
 *      Universal call to unmap a widget regardless of its type.
 */
void WidgetUnmap(void *ptr)
{
        int i, wtype = WTYPE_CODE_NONE;


        if(ptr == NULL)
            return;


        for(i = 0; i < widget_reg.total_entries; i++)
        {
            if(widget_reg.entry[i] == NULL)
                continue;

            if(widget_reg.entry[i]->ptr == ptr)
            {
                wtype = widget_reg.entry[i]->type;
                break;
            }
        }
        if(i >= widget_reg.total_entries)
            return;
          
        switch(wtype)
        {
          case WTYPE_CODE_PUSHBUTTON:
	    PBtnUnmap((push_button_struct *)ptr);
            break;

          case WTYPE_CODE_COLUMLIST:
            CListUnmap((colum_list_struct *)ptr);
            break;

          case WTYPE_CODE_DIALOG:
            DialogWinUnmap((dialog_win_struct *)ptr);
            break;

          case WTYPE_CODE_FILEBROWSER:
            FBrowserUnmap((fbrowser_struct *)ptr);
            break;

          case WTYPE_CODE_LIST:
            ListWinUnmap((list_window_struct *)ptr);
            break;

          case WTYPE_CODE_MENU:
            MenuUnmap((menu_struct *)ptr);
            break;

          case WTYPE_CODE_MENUBAR:
            MenuBarUnmap((menu_bar_struct *)ptr);
            break;

          case WTYPE_CODE_PAGESTEPPER:
            PStepperUnmap((page_stepper_struct *)ptr);
            break;

          case WTYPE_CODE_PROGRESSBAR:
            PBarUnmap((progress_bar_struct *)ptr);
            break;

          case WTYPE_CODE_PROMPT:
            PromptUnmap((prompt_window_struct *)ptr);
            break;

          case WTYPE_CODE_PULIST:
            PUListUnmap((popup_list_struct *)ptr);
            break;

          case WTYPE_CODE_SCALEBAR:
            ScaleBarUnmap((scale_bar_struct *)ptr);
            break;

          case WTYPE_CODE_SCROLLBAR:
	    /* ScrollBar widget has no unmap function. */
            break;

          case WTYPE_CODE_TOGGLEARRAY:
            TgBtnArrayUnmap((toggle_button_array_struct *)ptr);
            break;

          case WTYPE_CODE_TOGGLEBTN:
            TgBtnUnmap((toggle_button_struct *)ptr);
            break;

          case WTYPE_CODE_VIEWER:
            ViewerUnmap((file_viewer_struct *)ptr);
            break;

          default:
            fprintf(stderr,
 "WidgetUnmap(): Unsupported widget type code `%i'.\n",
                wtype
            );
            break;   
        }


	return;
}

/*
 *	Universal call to destroy a widget's allocated resources
 *	regardless of its type.
 */
void WidgetDestroy(void *ptr) 
{
        int i, wtype = WTYPE_CODE_NONE;


        if(ptr == NULL)
            return;


        for(i = 0; i < widget_reg.total_entries; i++)
        {
            if(widget_reg.entry[i] == NULL)
                continue;

            if(widget_reg.entry[i]->ptr == ptr)
            {
                wtype = widget_reg.entry[i]->type;
                break;
            }
        }
        if(i >= widget_reg.total_entries)
            return;


        switch(wtype)
        {
          case WTYPE_CODE_PUSHBUTTON:
	    PBtnDestroy((push_button_struct *)ptr);
            break;

          case WTYPE_CODE_COLUMLIST:
            CListDestroy((colum_list_struct *)ptr);  
            break;

          case WTYPE_CODE_DIALOG:
	    DialogWinDestroy((dialog_win_struct *)ptr);
            break;

          case WTYPE_CODE_FILEBROWSER:
	    FBrowserDestroy((fbrowser_struct *)ptr);
            break;

          case WTYPE_CODE_LIST:
	    ListWinDestroy((list_window_struct *)ptr);
            break;

          case WTYPE_CODE_MENU:
            MenuDestroy((menu_struct *)ptr);
            break;

          case WTYPE_CODE_MENUBAR:
            MenuBarDestroy((menu_bar_struct *)ptr);
            break;

          case WTYPE_CODE_PAGESTEPPER:
	    PStepperDestroy((page_stepper_struct *)ptr);
            break;

          case WTYPE_CODE_PROGRESSBAR:
            PBarDestroy((progress_bar_struct *)ptr);
            break;

          case WTYPE_CODE_PROMPT:
	    PromptDestroy((prompt_window_struct *)ptr);
            break;

          case WTYPE_CODE_PULIST:
	    PUListDestroy((popup_list_struct *)ptr);
            break;

          case WTYPE_CODE_SCALEBAR:
            ScaleBarDestroy((scale_bar_struct *)ptr);
            break;

          case WTYPE_CODE_SCROLLBAR:
            SBarDestroy((scroll_bar_struct *)ptr);
            break;

          case WTYPE_CODE_TOGGLEARRAY:
	    TgBtnArrayDestroy((toggle_button_array_struct *)ptr);
            break;

          case WTYPE_CODE_TOGGLEBTN:
	    TgBtnDestroy((toggle_button_struct *)ptr);
            break;
 
          case WTYPE_CODE_VIEWER:
	    ViewerDestroy((file_viewer_struct *)ptr);
            break;

          default:
            fprintf(stderr,
 "WidgetDestroy(): Unsupported widget type code `%i'.\n",
                wtype
            );
            break;
        }


        return;
}


/*
 *	Resizes src_buf image buffer to tar_buf image buffer.
 *	Both buffers must be allocated, the given depth must be
 *	supported.
 */
void WidgetResizeImageBuffer(
	depth_t d,
        u_int8_t *tar_buf,
        u_int8_t *src_buf,
        unsigned int tar_width,
        unsigned int tar_height,
        unsigned int src_width,
        unsigned int src_height
)
{
	int src_bytes_per_line, tar_bytes_per_line;
	u_int8_t *src_buf_ptr8, *tar_buf_ptr8;
	u_int16_t *src_buf_ptr16, *tar_buf_ptr16;
	u_int32_t *src_buf_ptr32, *tar_buf_ptr32;

	int tar_x_col, tar_y_row;
	int src_x_col, src_y_row;	/* In units of 256. */

	/* In units of 256. */
	int src_dh_x_col_inc, src_dh_y_row_inc;
	int src_dh_width, src_dh_height;


	/* Error checks. */
	if(((int)tar_width < 1) ||
           ((int)tar_height < 1) ||
           ((int)src_width < 1) ||
           ((int)src_height < 1) ||
           (tar_buf == NULL) ||
           (src_buf == NULL)
	)
	    return;


	/* 8 bits. */
        if(d == 8)
        {
            tar_bytes_per_line = (int)tar_width * BYTES_PER_PIXEL8;
            src_bytes_per_line = (int)src_width * BYTES_PER_PIXEL8;
            
            src_dh_x_col_inc = (int)((int)src_width * 256) / (int)tar_width;  
            src_dh_y_row_inc = (int)((int)src_height * 256) / (int)tar_height;
            
            src_dh_width = (int)src_width * 256;  
            src_dh_height = (int)src_height * 256;
            
            tar_x_col = 0;
            tar_y_row = 0;
            src_x_col = 0;
            src_y_row = 0;

            /* Begin copying source buffer to target buffer. */
            while((tar_y_row < (int)tar_height) &&
                  (src_y_row < src_dh_height)
            )
            {
                /* Get buffer position. */
                src_buf_ptr8 = (u_int8_t *)(&src_buf[
                    ((src_y_row >> 8) * src_bytes_per_line) +
                    ((src_x_col >> 8) * BYTES_PER_PIXEL8)
                ]);
                tar_buf_ptr8 = (u_int8_t *)(&tar_buf[
                    (tar_y_row * tar_bytes_per_line) +
                    (tar_x_col * BYTES_PER_PIXEL8)
                ]);
                *tar_buf_ptr8 = *src_buf_ptr8;


                /* Increment colum positions. */
                tar_x_col += 1;
                src_x_col += src_dh_x_col_inc;
        
                /* Go to next line? */
                if((tar_x_col >= (int)tar_width) ||
                   (src_x_col >= src_dh_width)
                )
                {
                    tar_x_col = 0;
                    src_x_col = 0;
         
                    tar_y_row += 1;
                    src_y_row += src_dh_y_row_inc;
                }
            }
	}
	/* 16 bits. */
	else if((d == 15)||(d == 16))
	{
	    tar_bytes_per_line = (int)tar_width * BYTES_PER_PIXEL16;
	    src_bytes_per_line = (int)src_width * BYTES_PER_PIXEL16;

	    src_dh_x_col_inc = (int)((int)src_width * 256) / (int)tar_width;
	    src_dh_y_row_inc = (int)((int)src_height * 256) / (int)tar_height;

	    src_dh_width = (int)src_width * 256;
	    src_dh_height = (int)src_height * 256;

	    tar_x_col = 0;
	    tar_y_row = 0;
	    src_x_col = 0;
	    src_y_row = 0;

	    /* Begin copying source buffer to target buffer. */
	    while((tar_y_row < (int)tar_height) &&
	          (src_y_row < src_dh_height)
	    )
	    {
		/* Get buffer position. */
		src_buf_ptr16 = (u_int16_t *)(&src_buf[
		    ((src_y_row >> 8) * src_bytes_per_line) +
		    ((src_x_col >> 8) * BYTES_PER_PIXEL16)
		]);
		tar_buf_ptr16 = (u_int16_t *)(&tar_buf[
		    (tar_y_row * tar_bytes_per_line) +
		    (tar_x_col * BYTES_PER_PIXEL16)
		]);
		*tar_buf_ptr16 = *src_buf_ptr16;


		/* Increment colum positions. */
		tar_x_col += 1;
		src_x_col += src_dh_x_col_inc;

		/* Go to next line? */
		if((tar_x_col >= (int)tar_width) ||
                   (src_x_col >= src_dh_width)
		)
		{
		    tar_x_col = 0;
		    src_x_col = 0;

		    tar_y_row += 1;
		    src_y_row += src_dh_y_row_inc;
		}
	    }
	}
	else if((d == 24) ||
                (d == 32)
	)
	{
            tar_bytes_per_line = (int)tar_width * BYTES_PER_PIXEL32;
            src_bytes_per_line = (int)src_width * BYTES_PER_PIXEL32;
                 
            src_dh_x_col_inc = (int)((int)src_width * 256) / (int)tar_width;
            src_dh_y_row_inc = (int)((int)src_height * 256) / (int)tar_height;
                
            src_dh_width = (int)src_width * 256;
            src_dh_height = (int)src_height * 256;
                   
            tar_x_col = 0;
            tar_y_row = 0;
            src_x_col = 0;
            src_y_row = 0;  

            /* Begin copying source buffer to target buffer. */
            while((tar_y_row < (int)tar_height) &&
                  (src_y_row < src_dh_height)
            )
            {
                /* Get buffer position. */
                src_buf_ptr32 = (u_int32_t *)(&src_buf[
                    ((src_y_row >> 8) * src_bytes_per_line) +
                    ((src_x_col >> 8) * BYTES_PER_PIXEL32)
                ]);
                tar_buf_ptr32 = (u_int32_t *)(&tar_buf[
                    (tar_y_row * tar_bytes_per_line) +
                    (tar_x_col * BYTES_PER_PIXEL32)
                ]);
                *tar_buf_ptr32 = *src_buf_ptr32;
                
         
                /* Increment colum positions. */
                tar_x_col += 1;
                src_x_col += src_dh_x_col_inc;
             
                /* Go to next line? */
                if((tar_x_col >= (int)tar_width) ||
                   (src_x_col >= src_dh_width)
                )
                {
                    tar_x_col = 0;
                    src_x_col = 0;
         
                    tar_y_row += 1;
                    src_y_row += src_dh_y_row_inc;
                }
            }
	}



	return;
}



/*
 *	Tiles an image_t to the drawable_t.
 */
void WidgetPutImageTile(
        drawable_t tar_d, image_t *src_img,
        unsigned int tar_width, unsigned int tar_height
)
{
	register int x, y;

        /* Error checks. */
        if(!IDC() ||
           (tar_d == 0) ||
	   (src_img == NULL) ||
           (tar_width == 0) ||
           (tar_height == 0) ||
           (widget_global.is_init == 0)
        )
            return;


	x = 0;
	y = 0;
	while(y < (int)tar_height)
	{
	    XPutImage(
		osw_gui[0].display,
		(drawable_t)tar_d,
		osw_gui[0].gc,
		src_img,
		0, 0,
		x, y,
		src_img->width, src_img->height
	    );

	    x += (int)src_img->width;
	    if(x >= (int)tar_width)
	    {
		x = 0;
		y += (int)src_img->height;
	    }
	}


	return;
}



/*
 *	Tiles a pixmap to a drawable_t.
 */
void WidgetPutPixmapTile(
        drawable_t tar_d, pixmap_t src_pm,
        unsigned int tar_width, unsigned int tar_height,
        unsigned int src_width, unsigned int src_height
)
{
        register int x, y;
         
        /* Error checks. */
        if( !IDC() ||
            (tar_d == 0) ||
            (src_pm == 0) ||
            (tar_width == 0) ||
            (tar_height == 0) ||
            (src_width == 0) ||
            (src_height == 0) ||
            (widget_global.is_init == 0)
        )
            return;
   

        x = 0;
        y = 0;
        while(y < (int)tar_height)
        {
            XCopyArea(
                osw_gui[0].display,
                (drawable_t)src_pm,
	        (drawable_t)tar_d,
                osw_gui[0].gc,
                0, 0,
		src_width, src_height,
                x, y
            );

            x += (int)src_width;
            if(x >= (int)tar_width)
            {
                x = 0;
                y += (int)src_height;
            }
        }       

	return;
}



/* **************************************************************************
 *
 *	Draws a 'frame' around win_t w.
 */
void WidgetFrameButton(
	win_t w, bool_t state,
        pixel_t fg_pix, pixel_t bg_pix  
)
{
	static win_attr_t wattr;

	/* Error checks. */
	if(!IDC() ||
           (w == 0) ||
           (widget_global.is_init == 0)
	)
	    return;


	OSWGetWindowAttributes(w, &wattr);


	if(state)
	{
            OSWSetFgPix(bg_pix);
            OSWDrawLine((drawable_t)w,
                0, 0,
                0, (int)wattr.height - 1
            );
            OSWDrawLine((drawable_t)w,
                0, 0,
                (int)wattr.width - 1, 0
            );

            OSWSetFgPix(fg_pix);
            OSWDrawLine((drawable_t)w,
                0, (int)wattr.height - 1,
                (int)wattr.width - 1, (int)wattr.height - 1
            );
            OSWDrawLine((drawable_t)w,
                (int)wattr.width - 1, (int)wattr.height - 1,
                (int)wattr.width - 1, 1  
            );
	}
	else
	{
	    OSWSetFgPix(fg_pix);
	    OSWDrawLine((drawable_t)w,
		0, 0,
		0, (int)wattr.height - 1
	    );
            OSWDrawLine((drawable_t)w,
                0, 0,
                (int)wattr.width - 1, 0
            );

            OSWSetFgPix(bg_pix);
            OSWDrawLine((drawable_t)w,
                0, (int)wattr.height - 1,
                (int)wattr.width - 1, (int)wattr.height - 1
            );
            OSWDrawLine((drawable_t)w,
                (int)wattr.width - 1, (int)wattr.height - 1,
                (int)wattr.width - 1, 1
            );
	}



	return;
}



/*
 *	Draws a 'frame' around pixmap_t pixmap.   The width and height
 *	must be the size of the pixmap.
 */
void WidgetFrameButtonPixmap(
        pixmap_t pixmap,
        bool_t state,
        unsigned int width, unsigned int height,
        pixel_t fg_pix, pixel_t bg_pix
)
{
	/* Error checks. */
        if(!IDC() ||
           (pixmap == 0) ||
	   (width == 0) ||
	   (height == 0) ||
	   (widget_global.is_init == 0)
        )
            return;


        if(state)
        {
            OSWSetFgPix(bg_pix);
	    /* Left vertical. */
            OSWDrawLine((drawable_t)pixmap,
                0, 0,
                0, (int)height - 1
            );
            OSWDrawLine((drawable_t)pixmap,
                1, 1,
                1, (int)height - 2
            );

	    /* Top horizontal. */
            OSWDrawLine((drawable_t)pixmap,
                0, 0,
                (int)width - 1, 0
            );
            OSWDrawLine((drawable_t)pixmap,
                1, 1,
                (int)width - 2, 1
            );

            OSWSetFgPix(fg_pix);
	    /* Bottom horizontal. */
            OSWDrawLine((drawable_t)pixmap,
                1, (int)height - 1,
                (int)width - 1, (int)height - 1
            );
            OSWDrawLine((drawable_t)pixmap,
                2, (int)height - 2,
                (int)width - 2, (int)height - 2
            );

	    /* Right vertical. */
            OSWDrawLine((drawable_t)pixmap,
                (int)width - 1, (int)height - 1,
                (int)width - 1, 1
            );
            OSWDrawLine((drawable_t)pixmap,
                (int)width - 2, (int)height - 2,
                (int)width - 2, 2
            );
        }
        else
        {
            OSWSetFgPix(fg_pix);
	    /* Left vertical. */
            OSWDrawLine((drawable_t)pixmap,
                0, 0,
                0, (int)height - 1
            );
            OSWDrawLine((drawable_t)pixmap,
                1, 1,
                1, (int)height - 2
            );

	    /* Top horizontal. */
            OSWDrawLine((drawable_t)pixmap,
                0, 0,
                (int)width - 1, 0
            );
            OSWDrawLine((drawable_t)pixmap,
                1, 1,
                (int)width - 2, 1
            );

            OSWSetFgPix(bg_pix);
	    /* Bottom horizontal. */
            OSWDrawLine((drawable_t)pixmap,
                0, (int)height - 1,
                (int)width - 1, (int)height - 1
            );
            OSWDrawLine((drawable_t)pixmap,
                2, (int)height - 2,
                (int)width - 2, (int)height - 2
            );

	    /* Right vertical. */
            OSWDrawLine((drawable_t)pixmap,
                (int)width - 1, (int)height - 1,
                (int)width - 1, 1
            );
            OSWDrawLine((drawable_t)pixmap,
                (int)width - 2, (int)height - 2,
                (int)width - 2, 2  
            );
        }


	return;
}



/*
 *	Creates an image displaying string.
 *
 *	Returns NULL on error.
 */
image_t *WidgetCreateImageText(
	char *string,
	font_t *font,
	unsigned int font_width, unsigned int font_height,
	pixel_t fg_pix,
	pixel_t bg_pix
)
{
	font_t *cur_font;
	image_t *image;
	pixmap_t pixmap;
	int len;
	unsigned int width, height;


        /* Error checks. */   
        if(!IDC() ||
           (string == NULL) ||
           (font == NULL) ||
	   (font_width == 0) ||
           (font_height == 0)
        )
            return(NULL);


	/* Get length of string. */
	len = strlen(string);

	/* Record current font. */
	cur_font = OSWQueryCurrentFont();


	/* Calculate size. */
	width = (len * (int)font_width) + 6;
	height = (int)font_height + 4;
	if(IS_NUM_ODD(width))
	    width += 1;
	if(IS_NUM_ODD(height))
	    height += 1;

	/* Create tempory pixmap. */
	if(OSWCreatePixmap(&pixmap, width, height))
	{
            /* Set back previous font. */
            OSWSetFont(cur_font);

	    return(NULL);
	}

	/* Clear pixmap using the background pixel. */
	OSWClearPixmap(pixmap, width, height, bg_pix);

	/* Set font and foreground pixel. */
	OSWSetFgPix(fg_pix);
	OSWSetFont(font);
	OSWDrawString(pixmap, 4, (height / 2) + 5, string);

	/* Get image from the tempory pixmap. */
	image = OSWGetImage(pixmap, 0, 0, width, height);

	/* Destroy the tempory pixmap, it's no longer needed. */
	OSWDestroyPixmap(&pixmap);


	/* Set back previous font. */
	OSWSetFont(cur_font);


	return(image);
}


/*
 *	Creates a one bit depth pixmap mask from image.
 *	Any pixel in the image that is >= 0x80 will be set to
 *	white, and any pixel < 0x80 will be set to black on the
 *	pixmap mask. Can return 0 on error.
 */
pixmap_t WidgetPixmapMaskFromImage(image_t *image)
{
#ifdef X_H
	int x, y, bytes_per_line;
	pixmap_t pixmap;
	u_int8_t *img_data;
	u_int8_t *ptr8;
	u_int16_t *ptr16;
	u_int32_t *ptr32;
	pixel_t white_pix, black_pix;
	GC gc;
	XGCValues gcv;


	if(!IDC() ||
           (image == NULL)
	)
	    return(0);

	if((image->data == NULL) ||
	   (image->width == 0) ||
           (image->height == 0)
	)
	    return(0);

	img_data = (u_int8_t *)image->data;
	white_pix = osw_gui[0].white_pix;
	black_pix = osw_gui[0].black_pix;


	/* Create mask pixmap. */
	pixmap = XCreatePixmap(
	    osw_gui[0].display,
	    osw_gui[0].root_win,
	    image->width, image->height,
	    1				/* Depth of 1. */
	);
	if(pixmap == 0)
	    return(0);


	/* Fetch current GC values. */
	XGetGCValues(osw_gui[0].display, osw_gui[0].gc, GCFunction, &gcv);

	/* Create tempory GC. */
	gcv.function = GXcopy;
	gcv.plane_mask = 1;		/* Monocrome. */
	gcv.foreground = white_pix;
	gcv.background = black_pix;
	gcv.line_width = 1;
	gc = XCreateGC(
	    osw_gui[0].display,
	    pixmap,
	    GCFunction | GCPlaneMask | GCForeground | GCBackground |
	    GCLineWidth,
	    &gcv
	);

	/* Copy image data to pixmap mask. */
	switch(image->depth)
	{
	  /* 8 bits. */
	  case 8:
	    bytes_per_line = image->width * BYTES_PER_PIXEL8;
	    for(y = 0; y < image->height; y++)
	    {
		for(x = 0; x < image->width; x++)
		{
		    ptr8 = (u_int8_t *)(&img_data[
			(y * bytes_per_line) +
                        (x * BYTES_PER_PIXEL8)
		    ]);
		    if(*ptr8)
		        XSetForeground(osw_gui[0].display, gc, white_pix);
		    else
			XSetForeground(osw_gui[0].display, gc, black_pix);

		    XDrawPoint(
			osw_gui[0].display,
			pixmap,
			gc,
			x, y
		    );
		}
	    }
	    break;

	  /* 15 or 16 bits. */
	  case 15:
	  case 16:
            bytes_per_line = image->width * BYTES_PER_PIXEL16;
            for(y = 0; y < image->height; y++)
            {
                for(x = 0; x < image->width; x++)
                {
                    ptr16 = (u_int16_t *)(&img_data[
                        (y * bytes_per_line) +
                        (x * BYTES_PER_PIXEL16)
                    ]);
                    if(*ptr16)
                        XSetForeground(osw_gui[0].display, gc, white_pix);
                    else
                        XSetForeground(osw_gui[0].display, gc, black_pix);

                    XDrawPoint(
                        osw_gui[0].display,
                        pixmap,
                        gc,
                        x, y
                    );
                }
            }
	    break;

          /* 24 or 32 bits. */
          case 24:
          case 32:
            bytes_per_line = image->width * BYTES_PER_PIXEL32;
            for(y = 0; y < image->height; y++)
            {
                for(x = 0; x < image->width; x++)
                { 
                    ptr32 = (u_int32_t *)(&img_data[
                        (y * bytes_per_line) +
                        (x * BYTES_PER_PIXEL32)
                    ]);
                    if(*ptr32)
                        XSetForeground(osw_gui[0].display, gc, white_pix);
                    else
                        XSetForeground(osw_gui[0].display, gc, black_pix);

                    XDrawPoint(
                        osw_gui[0].display,
                        pixmap,
                        gc,
                        x, y
                    );
                }
            }
            break;
	}

	/* Free out temp GC. */
	XFreeGC(osw_gui[0].display, gc);


	return(pixmap);
#else
	return(0);
#endif
}


/*
 *	Puts image_t ximage onto drawable_t d with option to allow
 *	transparency.
 */
void WidgetPutImageNormal(
        drawable_t d,     /* Target. */
        image_t *ximage, /* Source. */
        int tar_x, int tar_y,
        bool_t allow_transparency
)
{
	register int x, len;
        image_t *tar_ximage;

	register u_int8_t *tar_buf_ptr8;
        register u_int8_t *src_buf_ptr8;

        register u_int16_t *tar_buf_ptr16;
        register u_int16_t *src_buf_ptr16;
            
        register u_int32_t *tar_buf_ptr32;
        register u_int32_t *src_buf_ptr32;


        /* Error checks. */
        if(!IDC() ||
           (d == 0) ||
           (ximage == NULL) ||
	   (widget_global.is_init == 0)
        )
            return;


	/* *********************************************************** */

        /* Get target XImage. */
        tar_ximage = OSWGetImage(
	    d,
            tar_x, tar_y,
            ximage->width,
            ximage->height
        );
	if(tar_ximage == NULL)
            return;


        /* Check bit paddings. */
        if( (int)tar_ximage->bits_per_pixel !=
            (int)ximage->bits_per_pixel
        )
        { 
            fprintf(stderr,
                "WidgetPutImageNormal(): Incompatable pixel sizes.\n"
            );
            OSWDestroyImage(&tar_ximage);
            return;
        }
        
        /* Blit ximage to tar_ximage. */

	/* 8 bits. */
	if(osw_gui[0].depth == 8)
	{
	    tar_buf_ptr8 = (u_int8_t *)(tar_ximage->data);
            src_buf_ptr8 = (u_int8_t *)(ximage->data);

	    len = (int)tar_ximage->bytes_per_line * (int)tar_ximage->height;
	    for(x = 0; x < len; x += BYTES_PER_PIXEL8)
            {
                if(*src_buf_ptr8 != 0x00)
                {
                    *tar_buf_ptr8 = *src_buf_ptr8;
                }
                
                tar_buf_ptr8++; src_buf_ptr8++;
            }
	}
        /* 16 bits. */
        else if((osw_gui[0].depth == 16) || (osw_gui[0].depth == 15))   
        {
            tar_buf_ptr16 = (u_int16_t *)(tar_ximage->data);
            src_buf_ptr16 = (u_int16_t *)(ximage->data);

            len = (int)tar_ximage->bytes_per_line * (int)tar_ximage->height;
            for(x = 0; x < len; x += BYTES_PER_PIXEL16)
            {
                if(*src_buf_ptr16 != 0x0000)
                    *tar_buf_ptr16 = *src_buf_ptr16;
         
                tar_buf_ptr16++; src_buf_ptr16++;
	    }
	}
	/* 32 bits. */
	else if((osw_gui[0].depth == 24) ||
                (osw_gui[0].depth == 32)
	)
	{
            tar_buf_ptr32 = (u_int32_t *)(tar_ximage->data);
            src_buf_ptr32 = (u_int32_t *)(ximage->data);

            len = (int)tar_ximage->bytes_per_line * (int)tar_ximage->height;
            for(x = 0; x < len; x += BYTES_PER_PIXEL32)
            {   
                if(*src_buf_ptr32 != 0x00000000)
                    *tar_buf_ptr32 = *src_buf_ptr32;

                tar_buf_ptr32++; src_buf_ptr32++;
            }
	}

        /* Put image back to d. */
        OSWPutImageToDrawableSect(
	    tar_ximage, d,
	    tar_x, tar_y,
            0, 0,
            tar_ximage->width, tar_ximage->height
        );          
        
        
        /* Destroy the target XImage, it is not needed. */
        OSWDestroyImage(&tar_ximage);



	return;
}



/*
 *	Puts image_t ximage onto drawable_t d with shadow depth effects.
 */
void WidgetPutImageRaised(
        drawable_t d,     /* Target. */
        image_t *ximage, /* Source. */
        int tar_x, int tar_y,
        unsigned int altitude
)
{
	image_t *tar_ximage;
	int x_rel, y_rel;
	int x, len;

	u_int8_t *tar_buf_ptr8;
	u_int8_t *src_buf_ptr8;

	u_int16_t *tar_buf_ptr16;
        u_int16_t *src_buf_ptr16;

        u_int32_t *tar_buf_ptr32;
        u_int32_t *src_buf_ptr32;

	/* For subtractive calculations. */
        u_int32_t r, g, b;
        u_int32_t c;


	/* Error checks. */
	if(!IDC() ||
	   (d == 0) ||
	   (ximage == NULL) ||
	   (widget_global.is_init == 0)
	)
	    return;


	/* ********************************************************* */
	/* Blit shadow. */

	x_rel = (int)((double)altitude * 0.70710670);
	y_rel = x_rel;

        /* Get target image. */
        tar_ximage = OSWGetImage(
            d, 
            tar_x + x_rel, tar_y + y_rel,
            ximage->width,
	    ximage->height
        );
        if(tar_ximage != NULL)
	{
            /* Check bit paddings. */
            if((int)tar_ximage->bits_per_pixel !=
               (int)ximage->bits_per_pixel
            )
            {
                fprintf(stderr,
                    "WidgetPutImageRaised(): Incompatable pixel sizes.\n"
                );
                OSWDestroyImage(&tar_ximage);
            }
            else
	    {
                /* Blit image as shadow to target image. */

	        /* 8 bits. */
	        if(osw_gui[0].depth == 8)
	        {
                    tar_buf_ptr8 = (u_int8_t *)(tar_ximage->data);
                    src_buf_ptr8 = (u_int8_t *)(ximage->data);

                    len = (int)tar_ximage->bytes_per_line * (int)tar_ximage->height;
                    for(x = 0; x < len; x += BYTES_PER_PIXEL8)
                    {
                        if(*src_buf_ptr8 != 0x00)
                        {
                            c = (u_int32_t)((*tar_buf_ptr8 & 0x000000E0) >> 5);
                            r = SUBMIN(c - 3, c);

                            c = (u_int32_t)((*tar_buf_ptr8 & 0x0000001C) >> 2);
                            g = SUBMIN(c - 3, c);

                            c = (u_int32_t)(*tar_buf_ptr8 & 0x00000003);
                            b = SUBMIN(c - 2, c);

                            *tar_buf_ptr8 = (u_int8_t)(
                                (r << 5) | (g << 2) | (b)
                            );
                        }

                        tar_buf_ptr8++; src_buf_ptr8++;
                    }
	        }
                /* 15 bits. */
                else if(osw_gui[0].depth == 15)
                {
                    tar_buf_ptr16 = (u_int16_t *)(tar_ximage->data);
                    src_buf_ptr16 = (u_int16_t *)(ximage->data);

                    len = (int)tar_ximage->bytes_per_line * (int)tar_ximage->height;
                    for(x = 0; x < len; x += BYTES_PER_PIXEL15)
		    {
                        if(*src_buf_ptr16 != 0x0000)
                        {
		            c = (u_int32_t)((*tar_buf_ptr16 & 0x00007C00) >> 10);
		            r = SUBMIN(c - 3, c);

                            c = (u_int32_t)((*tar_buf_ptr16 & 0x000003E0) >> 5);
                            g = SUBMIN(c - 6, c);

		            c = (u_int32_t)(*tar_buf_ptr16 & 0x0000001F);
                            b = SUBMIN(c - 3, c);

		            *tar_buf_ptr16 = (u_int16_t)(
			        (r << 10) | (g << 5) | (b)
                            );
                        }
                        tar_buf_ptr16++; src_buf_ptr16++;
                    }
                }
                /* 16 bits. */
                else if(osw_gui[0].depth == 16)
                {
                    tar_buf_ptr16 = (u_int16_t *)(tar_ximage->data);
                    src_buf_ptr16 = (u_int16_t *)(ximage->data);

                    len = (int)tar_ximage->bytes_per_line * (int)tar_ximage->height;
                    for(x = 0; x < len; x += BYTES_PER_PIXEL16)
		    {
                        if(*src_buf_ptr16 != 0x0000)
                        {
		            c = (u_int32_t)((*tar_buf_ptr16 & 0x0000F800) >> 11);
		            r = SUBMIN(c - 3, c);

                            c = (u_int32_t)((*tar_buf_ptr16 & 0x000007E0) >> 5);
                            g = SUBMIN(c - 6, c);

		            c = (u_int32_t)(*tar_buf_ptr16 & 0x0000001F);
                            b = SUBMIN(c - 3, c);

		            *tar_buf_ptr16 = (u_int16_t)(
			        (r << 11) | (g << 5) | (b)
                            );
                        }
                        tar_buf_ptr16++; src_buf_ptr16++;
                    }
                }
                /* 24 or 32 bits. */
                else if((osw_gui[0].depth == 24) ||
                        (osw_gui[0].depth == 32)
                )
                {
                    tar_buf_ptr32 = (u_int32_t *)(tar_ximage->data);
                    src_buf_ptr32 = (u_int32_t *)(ximage->data);

                    len = (int)tar_ximage->bytes_per_line * (int)tar_ximage->height;
                    for(x = 0; x < len; x += BYTES_PER_PIXEL32)
                    {
                        if(*src_buf_ptr32 != 0x00000000)
                        {
                            c = (u_int32_t)((*tar_buf_ptr32 & 0x00FF0000) >> 16);
                            r = SUBMIN(c - 26, c);

                            c = (u_int32_t)((*tar_buf_ptr32 & 0x0000FF00) >> 8);
                            g = SUBMIN(c - 26, c);

                            c = (u_int32_t)(*tar_buf_ptr32 & 0x000000FF);
                            b = SUBMIN(c - 26, c);

                            *tar_buf_ptr32 = (u_int32_t)(
                                (r << 16) | (g << 8) | (b)
                            );
		        }
                        tar_buf_ptr32++; src_buf_ptr32++;
                    }
                }

                /* Put shadow image back to drawable. */
                XPutImage(osw_gui[0].display, (drawable_t)d, osw_gui[0].gc,
                    tar_ximage,
                    0, 0,
                    tar_x + x_rel, tar_y + y_rel,
                    tar_ximage->width, tar_ximage->height
                );


                /* Destroy the shadow image, it is not needed. */
                OSWDestroyImage(&tar_ximage);
	    }
	}


	/* ********************************************************* */
	/* Blit image. */

	/* Get target image. */
	tar_ximage = OSWGetImage(
	    d,
	    tar_x, tar_y,
	    ximage->width,
	    ximage->height
	);
	if(tar_ximage != NULL)
	{
	    /* Check bit paddings. */
	    if((int)tar_ximage->bits_per_pixel !=
	       (int)ximage->bits_per_pixel
	    )
	    {
	        fprintf(stderr,
		    "WidgetPutImageRaised(): Incompatable pixel sizes.\n"
	        );
	        OSWDestroyImage(&tar_ximage);
	    }
	    else
	    {
	        /* Blit image to fetched area image. */

	        /* 8 bits. */
                if(osw_gui[0].depth == 8) 
                {
                    tar_buf_ptr8 = (u_int8_t *)(tar_ximage->data);
                    src_buf_ptr8 = (u_int8_t *)(ximage->data);

                    len = (int)tar_ximage->bytes_per_line * (int)tar_ximage->height;
                    for(x = 0; x < len; x += BYTES_PER_PIXEL8)  
                    {
                        if(*src_buf_ptr8 != 0x00)
                            *tar_buf_ptr8 = *src_buf_ptr8;

                        tar_buf_ptr8++; src_buf_ptr8++;
                    }
                }
	        /* 15 or 16 bits. */
	        else if((osw_gui[0].depth == 16) || (osw_gui[0].depth == 15))
	        {
	            tar_buf_ptr16 = (u_int16_t *)(tar_ximage->data);
                    src_buf_ptr16 = (u_int16_t *)(ximage->data);

	            len = (int)tar_ximage->bytes_per_line * (int)tar_ximage->height;
	            for(x = 0; x < len; x += BYTES_PER_PIXEL16)
                    {
		        if(*src_buf_ptr16 != 0x0000)
		            *tar_buf_ptr16 = *src_buf_ptr16;

                        tar_buf_ptr16++; src_buf_ptr16++;
	            }
	        }
	        /* 24 or 32 bits. */
	        else if((osw_gui[0].depth == 24) ||
                        (osw_gui[0].depth == 32)
	        )
	        {
                    tar_buf_ptr32 = (u_int32_t *)(tar_ximage->data);
                    src_buf_ptr32 = (u_int32_t *)(ximage->data);

                    len = (int)tar_ximage->bytes_per_line * (int)tar_ximage->height;
                    for(x = 0; x < len; x += BYTES_PER_PIXEL32)
                    {
                        if(*src_buf_ptr32 != 0x00000000)
                            *tar_buf_ptr32 = *src_buf_ptr32;

                        tar_buf_ptr32++; src_buf_ptr32++;
                    }
	        }

	        /* Put image back to d. */
	        XPutImage(osw_gui[0].display, (drawable_t)d, osw_gui[0].gc,
	            tar_ximage,
	            0, 0,
	            tar_x, tar_y,
	            tar_ximage->width, tar_ximage->height
	        );


	        /* Destroy the target XImage, it is not needed. */
                OSWDestroyImage(&tar_ximage);
	    }
	}


	return;
}


/*
 *	Adjusts the rgb color level on the image by the given
 *	rgb coefficient values from 0 to 255 inclusive.
 *
 *	0 to 1 will reduce color value, 1 to 255 will increase color
 *	value.
 */
void WidgetAdjustImageGamma(
	image_t *image,
	double r, double g, double b
)       
{
	u_int8_t cr, cg, cb;
	int i, len;

	u_int8_t *buf_ptr8;
	u_int16_t *buf_ptr16;
	u_int32_t *buf_ptr32;


	/* Error checks. */
	if(!IDC() || (image == NULL))
	    return;

	if((image->data == NULL) ||
	   (image->width == 0) ||
           (image->height == 0)
	)
	    return;


	/* Sanitize r, g, b. */
	if(r < 0)
	    r = 0;
	if(g < 0)
	    g = 0;
	if(b < 0)
	    b = 0;


	/* 8 bits. */
	if(image->depth == 8)
	{
	    buf_ptr8 = (u_int8_t *)image->data;

	    i = 0;
	    len = (int)image->width * (int)image->height;
	    while(i < len)
	    {
		/* rrrgggbb */
		cr = (u_int8_t)MIN((u_int32_t)(*buf_ptr8 & 0xe0) * r,
		    0x000000ff);
                cg = (u_int8_t)MIN((u_int32_t)((*buf_ptr8 & 0x1c) << 3) * g,
		    0x000000ff);
                cb = (u_int8_t)MIN((u_int32_t)((*buf_ptr8 & 0x03) << 6) * b,
		    0x000000ff);

		*buf_ptr8++ = PACK8TO8(cr, cg, cb);

		i++;
	    }
	}
	/* 15 bits. */
	else if(image->depth == 15)
	{
            buf_ptr16 = (u_int16_t *)image->data;

            i = 0;
            len = (int)image->width * (int)image->height;
            while(i < len)
            {
                /* urrrrrgggggbbbbb */
                cr = (u_int8_t)MIN((u_int32_t)((*buf_ptr16 & 0x7C00) >> 7) * r,
                    0x000000ff);
                cg = (u_int8_t)MIN((u_int32_t)((*buf_ptr16 & 0x03E0) >> 2) * g,
                    0x000000ff);
                cb = (u_int8_t)MIN((u_int32_t)((*buf_ptr16 & 0x001f) << 3) * b,
                    0x000000ff);

                *buf_ptr16++ = PACK8TO15(cr, cg, cb);

                i++;
            }
	}
	/* 16 bits. */
	else if(image->depth == 16)
	{
            buf_ptr16 = (u_int16_t *)image->data;

            i = 0;
            len = (int)image->width * (int)image->height;
            while(i < len)
            {
                /* rrrrrggggggbbbbb */
                cr = (u_int8_t)MIN((u_int32_t)((*buf_ptr16 & 0xf800) >> 8) * r,
                    0x000000ff);
                cg = (u_int8_t)MIN((u_int32_t)((*buf_ptr16 & 0x07e0) >> 3) * g,
                    0x000000ff);
                cb = (u_int8_t)MIN((u_int32_t)((*buf_ptr16 & 0x001f) << 3) * b,
                    0x000000ff);

                *buf_ptr16++ = PACK8TO16(cr, cg, cb);

                i++;
            }
	}
        /* 24 or 32 bits. */
        else if((image->depth == 24) ||
                (image->depth == 32)
	)
        {
            buf_ptr32 = (u_int32_t *)image->data;

            i = 0;
            len = (int)image->width * (int)image->height;
            while(i < len)
            {
                /* 0xaarrggbb */
                cr = (u_int8_t)MIN((u_int32_t)((*buf_ptr32 & 0x00ff0000) >> 16) * r,
                    0x000000ff);
                cg = (u_int8_t)MIN((u_int32_t)((*buf_ptr32 & 0x0000ff00) >> 8) * g,
                    0x000000ff);
                cb = (u_int8_t)MIN((u_int32_t)(*buf_ptr32 & 0x000000ff) * b,
		    0x000000ff);

                *buf_ptr32++ = PACK8TO32(0x00, cr, cg, cb);

                i++;
            }
        }

	return;
}




