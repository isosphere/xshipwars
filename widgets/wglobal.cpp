// widgets/wglobal.cpp
/*
                       Widget: Global and Core Functions

	Functions:

	int WidgetInitGlobals(int argc, char *argv[])
	void WidgetDestroyGlobals()
	int WidgetManage(event_t *event)

 */

#include "../include/widget.h"


/* Cursor pixmaps. */
#include "../include/cursors/dragfile_cur.xpm"
#include "../include/cursors/dragitem_cur.xpm"
#include "../include/cursors/h_split.xpm"
#include "../include/cursors/no_way.xpm"
#include "../include/cursors/std_arrow.xpm"
#include "../include/cursors/text_cur.xpm"
#include "../include/cursors/v_split.xpm"

/* Push button background Images. */
#include "../include/images/btn_armed.h"
#include "../include/images/btn_unarmed.h"
#include "../include/images/btn_highlighted.h"

/* Scale bar handles. */
#include "../include/images/scalebar_h.h"
#include "../include/images/scalebar_v.h"

/* Background tile images. */
#include "../include/images/stdbkg.h"
#include "../include/images/menubkg.h"

/* Tab images. */
#include "../include/images/tab0.h"
#include "../include/images/tab1.h"

/* Toggle button images. */
#include "../include/images/toggle_btn0.h"
#include "../include/images/toggle_btn1.h"


/* Icon images. */
#include "../include/icons/browsefiles.h"
#include "../include/icons/default.h"
#include "../include/icons/exec0.h"
#include "../include/icons/exec1.h"
#include "../include/icons/dir0.h"
#include "../include/icons/dir1.h"
#include "../include/icons/file0.h"
#include "../include/icons/file1.h"
#include "../include/icons/goto_parent.h"
#include "../include/icons/link0.h"
#include "../include/icons/link1.h"
#include "../include/icons/mount.h"
#include "../include/icons/pipe0.h"
#include "../include/icons/pipe1.h"
#include "../include/icons/pulist_map.h"
#include "../include/icons/unmount.h"

#ifndef MAX
#define MIN(a,b)        (((a) < (b)) ? (a) : (b))
#define MAX(a,b)	(((a) > (b)) ? (a) : (b))
#endif
/* #define MIN(a,b)	((a) < (b) ? (a) : (b)) */
/* #define MAX(a,b)	((a) > (b) ? (a) : (b)) */

#define SUBMIN(a,b)	((a) < (b) ? (a) : 0)



widget_global_struct widget_global;
widget_reg_struct widget_reg;

hint_win_struct hint_win;
hint_win_data_struct **hint_win_data;
int total_hint_win_datas;



/*
 *	Initializes global resources for the widgets.
 */
int WidgetInitGlobals(int argc, char *argv[])
{
	int i;
	WCursor *wcursor;
	image_t *image;
	pixmap_t pixmap;
	char *strptr;
	WColorStruct color;


	/* Error checks. */
        if(!IDC())
	{
	    fprintf(stderr,
		"WidgetInitGlobals(): Error: Display not connected.\n"
	    );
            return(-1);
	}

	if((osw_gui[0].scr_ptr == NULL) ||
           (osw_gui[0].depth == 0) ||
           (osw_gui[0].root_win == 0)
	)
	{
            fprintf(stderr,
 "WidgetInitGlobals(): Error: Uninitialized GUI resources.\n"
            );
	    return(-1);
	}

	/* Check if depth is one that this widget set supports. */
	if((osw_gui[0].depth != 8) &&
	   (osw_gui[0].depth != 15) &&
           (osw_gui[0].depth != 16) &&
           (osw_gui[0].depth != 24) &&
           (osw_gui[0].depth != 32)
	)
	{
	    fprintf(stderr,
"WidgetInitGlobals(): Warning, depth %i is not supported by this widget set.\n",
		osw_gui[0].depth
	    );
	    /*   Do not return, see how things work out (maybe client has
             *   things taken care of.
             */
	}

	/* ************************************************************ */
	/* Force black and white mode set to off by default. */
	widget_global.force_mono = False;


        /* ************************************************************ */
        /* Parameters. */

        /* Double click interval. */
        widget_global.double_click_int = DEF_DOUBLE_CLICK_INT;

	/* Hint window map delay. */
	widget_global.hintwin_map_delay = DEF_HINTWIN_MAP_DELAY;

	/* Slow double click interval for relabeling items. */
	widget_global.relabel_item_delay = DEF_RELABEL_ITEM_DELAY;

	/* Popup list repeat delays. */
	widget_global.pulist_repeat_delay = DEF_PULIST_REPEAT_DELAY;
	widget_global.pulist_repeat_interval = DEF_PULIST_REPEAT_INT;

        /* Prompt repeat delays. */
	widget_global.prompt_repeat_delay = DEF_PROMPT_REPEAT_DELAY;
	widget_global.sb_repeat_interval = DEF_PROMPT_REPEAT_INT;

	/* Scroll bar repeat delays. */
	widget_global.sb_repeat_delay = DEF_SB_REPEAT_DELAY;
	widget_global.sb_repeat_interval = DEF_SB_REPEAT_INT;

	/* List edge scroll delay. */
	widget_global.list_edge_scroll_delay = DEF_LIST_EDGE_SCROLL_DELAY;

	/* Maximum sizes. */
	widget_global.max_width = DEF_GRAPHICS_MAX_WIDTH;
	widget_global.max_height = DEF_GRAPHICS_MAX_HEIGHT;


        /* *********************************************************** */
        /* Parse arguments. */

	for(i = 0; i < argc; i++)
	{
	    if(argv[i] == NULL)
		continue;


	    /* Force mono. */
	    if(!strcmp(argv[i], "--force_mono") ||
               !strcmp(argv[i], "-force_mono") ||
               !strcmp(argv[i], "--mono") ||
               !strcmp(argv[i], "-mono")
            )
	    {
		widget_global.force_mono = True;
	    }


	}

	/* *********************************************************** */
	/* Load color pixels. */

	/* Surface colors. */
	if(OSWLoadPixelCLSP(&widget_global.surface_normal_pix, CLSP_SURFACE_NORMAL))
	    widget_global.surface_normal_pix = osw_gui[0].black_pix;
        if(OSWLoadPixelCLSP(&widget_global.surface_editable_pix, CLSP_SURFACE_EDITABLE))
            widget_global.surface_editable_pix = osw_gui[0].black_pix;
        if(OSWLoadPixelCLSP(&widget_global.surface_selected_pix, CLSP_SURFACE_SELECTED))
            widget_global.surface_selected_pix = osw_gui[0].white_pix;
        if(OSWLoadPixelCLSP(&widget_global.surface_shadow_pix, CLSP_SURFACE_SHADOW))
            widget_global.surface_shadow_pix = osw_gui[0].white_pix;
        if(OSWLoadPixelCLSP(&widget_global.surface_highlight_pix, CLSP_SURFACE_HIGHLIGHT))
            widget_global.surface_highlight_pix = osw_gui[0].white_pix;


	/* Scrollbar and scalebar colors. */
        if(OSWLoadPixelCLSP(&widget_global.scroll_bkg_pix, CLSP_SCROLL_BKG))
            widget_global.scroll_bkg_pix = osw_gui[0].black_pix;
        if(OSWLoadPixelCLSP(&widget_global.scroll_frame_pix, CLSP_SCROLL_FRAME))
            widget_global.scroll_frame_pix = osw_gui[0].white_pix;
        if(OSWLoadPixelCLSP(&widget_global.scroll_bar_pix, CLSP_SCROLL_BAR))
            widget_global.scroll_bar_pix = osw_gui[0].white_pix;
        if(OSWLoadPixelCLSP(&widget_global.scroll_cursor_pix, CLSP_SCROLL_CURSOR))
            widget_global.scroll_cursor_pix = osw_gui[0].white_pix;

	/* Text colors. */
        if(OSWLoadPixelCLSP(&widget_global.normal_text_pix, CLSP_TEXT_NORMAL))
            widget_global.normal_text_pix = osw_gui[0].white_pix;
        if(OSWLoadPixelCLSP(&widget_global.editable_text_pix, CLSP_TEXT_EDITABLE))
            widget_global.editable_text_pix = osw_gui[0].white_pix;
        if(OSWLoadPixelCLSP(&widget_global.selected_text_pix, CLSP_TEXT_SELECTED)) 
            widget_global.selected_text_pix = osw_gui[0].black_pix;
        if(OSWLoadPixelCLSP(&widget_global.disabled_text_pix, CLSP_TEXT_DISABLED))
            widget_global.disabled_text_pix = osw_gui[0].white_pix;

	/* Hint window colors. */
	if(OSWLoadPixelCLSP(&widget_global.hint_bkg_pix, CLSP_HINT_BKG))
            widget_global.hint_bkg_pix = osw_gui[0].white_pix;
	if(OSWLoadPixelCLSP(&widget_global.hint_text_pix, CLSP_HINT_TEXT))
            widget_global.hint_text_pix = osw_gui[0].black_pix;

	/* Progress bar colors. */
	if(OSWLoadPixelCLSP(&widget_global.pbar_text_pix, CLSP_PBAR_TEXT))
            widget_global.pbar_text_pix = osw_gui[0].white_pix;


	/* ********************************************************** */
	/* Load widget cursors. */

        /* Standard arrow WCursor. */
	color.a = 0x00;
	color.r = 0xff;
	color.g = 0xff;
	color.b = 0xff;
        wcursor = WidgetCreateCursorFromData(
            std_arrow_xpm,
            0, 0,
            color
        );
        if(wcursor == NULL)
            return(-1);
        widget_global.std_arrow_wcr = wcursor;

	/* Horizontal split. */
        wcursor = WidgetCreateCursorFromData(
            h_split_xpm,
            8, 8,
            color
        );
        if(wcursor == NULL)
            return(-1);
        widget_global.h_split_wcr = wcursor;

	/* Vertical split. */
        wcursor = WidgetCreateCursorFromData(
            v_split_xpm,
            8, 8,
            color
        );
        if(wcursor == NULL)
            return(-1);
        widget_global.v_split_wcr = wcursor;

	/* Drag item WCursor. */
	wcursor = WidgetCreateCursorFromData(
	    dragitem_cur_xpm,
	    8, 8,
	    color
	);
	if(wcursor == NULL)
	    return(-1);
	widget_global.drag_item_wcr = wcursor;

	/* Drag file WCursor. */
        wcursor = WidgetCreateCursorFromData(
            dragfile_cur_xpm,
            8, 8,
            color
        );
        if(wcursor == NULL)
            return(-1);
        widget_global.drag_file_wcr = wcursor;

	/* Text WCursor. */
        wcursor = WidgetCreateCursorFromData(
            text_cur_xpm,
            8, 8,
            color
        );
        if(wcursor == NULL)
            return(-1);
        widget_global.text_wcr = wcursor;

        /* No way WCursor. */
        wcursor = WidgetCreateCursorFromData(
            no_way_xpm,
            8, 8,
            color
        );
        if(wcursor == NULL)
            return(-1);
        widget_global.no_way_wcr = wcursor;


	/* ********************************************************** */
	/* Fonts. */
        OSWLoadFont(&widget_global.pbtn_font, "7x14");
        OSWLoadFont(&widget_global.menu_font, "7x14");
        OSWLoadFont(&widget_global.prompt_label_font, "7x14");
        OSWLoadFont(&widget_global.prompt_text_font, "7x14");
        OSWLoadFont(&widget_global.scale_bar_font, "6x12");
	OSWLoadFont(&widget_global.std_font, "7x14");


	/* ********************************************************** */
	/* Load background tile pixmaps. */

	/* Standard background. */
        image = WidgetLoadImageFromTgaData(stdbkg_tga);
        if(image == NULL)
            return(-1);
        pixmap = OSWCreatePixmapFromImage(image);
        if(pixmap == 0)  
            return(-1);
        OSWDestroyImage(&image);
        strptr = NULL;

	widget_global.std_bkg_pm = pixmap;


	/* Standard icon pixmap. */
        image = WidgetLoadImageFromTgaData(default_icon_tga);
        if(image == NULL)
            return(-1);
        pixmap = OSWCreatePixmapFromImage(image);
        if(pixmap == 0)
            return(-1);
        OSWDestroyImage(&image);
        strptr = NULL;

        widget_global.std_icon_pm = pixmap;




        /* ********************************************************** */
        /* Load images. */
 
	/* Standard background. */
	image = WidgetLoadImageFromTgaData(stdbkg_tga);
        if(image == NULL)
            return(-1);
        else
            widget_global.std_bkg_img = image;

        /* Button background images. */
        if(widget_global.force_mono)
        {
            image = NULL;
        }   
        else
        {
            image = WidgetLoadImageFromTgaData(btn_unarmed_tga);
            if(image == NULL)
                return(-1);
        }
        widget_global.btn_unarmed_img = image;

        if(widget_global.force_mono)
        {   
            image = NULL;
        }
        else
        {
            image = WidgetLoadImageFromTgaData(btn_armed_tga);
            if(image == NULL)
                return(-1);
        }
        widget_global.btn_armed_img = image;

        if(widget_global.force_mono)
        {   
            image = NULL;
        }   
        else
        {
            image = WidgetLoadImageFromTgaData(btn_highlighted_tga);
            if(image == NULL)
                return(-1);
        }
        widget_global.btn_highlighted_img = image;


	/* Scale bar handles. */
        image = WidgetLoadImageFromTgaData(scalebar_h_tga);
        if(image == NULL)
            return(-1);
        else
            widget_global.scalebar_h_img = image;

        image = WidgetLoadImageFromTgaData(scalebar_v_tga);
        if(image == NULL)
            return(-1);
        else
            widget_global.scalebar_v_img = image;

	/* Popup list widget map button icon image. */
        image = WidgetLoadImageFromTgaData(pulist_map_tga);
        if(image == NULL)
            return(-1);
        else
            widget_global.pulist_map_icon_img = image;

	/* Menu bar background. */
        image = WidgetLoadImageFromTgaData(menubkg_tga);
        if(image == NULL)
            return(-1);
        else
            widget_global.menu_bkg_img = image;
/*
        image = WidgetLoadImageFromTgaData(hint_bkg_tga);
        if(image == NULL)
            return(-1);
        else
            widget_global.hint_bkg_img = image;
*/


	/* Browses files icon. */
        image = WidgetLoadImageFromTgaData(browsefiles_tga);
        if(image == NULL)
            return(-1);   
        else
            widget_global.browse_files_img = image;

	/* Mount icon. */
        image = WidgetLoadImageFromTgaData(mount_tga);
        if(image == NULL)
            return(-1);
        else
            widget_global.mount_img = image;

	/* Unmount icon. */
        image = WidgetLoadImageFromTgaData(unmount_tga);
        if(image == NULL)
            return(-1);
        else
            widget_global.unmount_img = image;



	/* Standard item normal. */
	image = WidgetLoadImageFromTgaData(file0_tga);
	if(image == NULL)
	    return(-1);
	else
            widget_global.stditem_normal_img = image;

        /* Standard item selected. */
        image = WidgetLoadImageFromTgaData(file1_tga);
        if(image == NULL)
            return(-1);
        else
            widget_global.stditem_selected_img = image;


        /* Dir icon normal. */
        image = WidgetLoadImageFromTgaData(dir0_tga);
        if(image == NULL)
            return(-1);
        else
            widget_global.diricon_normal_img = image;

        /* Dir icon selected. */
        image = WidgetLoadImageFromTgaData(dir1_tga);
        if(image == NULL)   
            return(-1);
        else
            widget_global.diricon_selected_img = image;


        /* File icon normal. */
        image = WidgetLoadImageFromTgaData(file0_tga);
        if(image == NULL)
            return(-1);
        else
            widget_global.fileicon_normal_img = image;

        /* File icon selected. */
        image = WidgetLoadImageFromTgaData(file1_tga);
        if(image == NULL)
            return(-1);
        else
            widget_global.fileicon_selected_img = image;


        /* Link icon normal. */
        image = WidgetLoadImageFromTgaData(link0_tga);
        if(image == NULL)
            return(-1);
        else
            widget_global.linkicon_normal_img = image;

        /* Link icon selected. */
        image = WidgetLoadImageFromTgaData(link1_tga);
        if(image == NULL)
            return(-1);
        else
            widget_global.linkicon_selected_img = image;


        /* Pipe icon normal. */
        image = WidgetLoadImageFromTgaData(pipe0_tga);
        if(image == NULL)
            return(-1);
        else
            widget_global.pipeicon_normal_img = image;

        /* Pipe icon selected. */
	image = WidgetLoadImageFromTgaData(pipe1_tga);
        if(image == NULL)
            return(-1);
        else
            widget_global.pipeicon_selected_img = image;


	/* Executable normal icons. */
        image = WidgetLoadImageFromTgaData(exec0_tga);
        if(image == NULL)
            return(-1);
        else
            widget_global.execicon_normal_img = image;
	/* Executeable selected icon. */
        image = WidgetLoadImageFromTgaData(exec1_tga);
        if(image == NULL)
            return(-1);
        else
            widget_global.execicon_selected_img = image;


	/* Goto parent icon. */
        image = WidgetLoadImageFromTgaData(goto_parent_tga);
        if(image == NULL)
            return(-1);
        else
            widget_global.goto_parent_img = image;


	/* Toggle button unarmed. */
	if(widget_global.force_mono)
	{
	    image = NULL;
	}
	else
	{
            image = WidgetLoadImageFromTgaData(toggle_btn0_tga);
            if(image == NULL)
                return(-1);
	}
        widget_global.toggle_btn_unarmed_img = image;


	/* Toggle button armed. */
        if(widget_global.force_mono)
        {   
            image = NULL;
        }
        else
        {
            image = WidgetLoadImageFromTgaData(toggle_btn1_tga);
            if(image == NULL)
                return(-1);
        }
        widget_global.toggle_btn_armed_img = image;


        /* Tab normal. */
        image = WidgetLoadImageFromTgaData(tab0_tga);
        if(image == NULL)
            return(-1);
        else
            widget_global.tab_normal_img = image;

        /* Tab selected. */
        image = WidgetLoadImageFromTgaData(tab1_tga);
        if(image == NULL)   
            return(-1);
        else
            widget_global.tab_selected_img = image;


	/* ************************************************************ */
	/* Hint window and its data. */
	hint_win_data = NULL;
	total_hint_win_datas = 0;

	if(HintWinInit())
	    return(-1);


        /* ************************************************************ */
        /* Clear global record handlers. */

	ListWinRepeatRecordClear();
	PUListRepeatRecordClear();
	PromptRepeatRecordClear();
        SBarRepeatRecordClear();


	/* ************************************************************ */

	/* Mark as initialized. */
	widget_global.is_init = 1;


	return(0);
}



/*
 *	Destroys all global widget resources.
 */
void WidgetDestroyGlobals()
{
	int i;


	if(!IDC())
	    return;


	/* ************************************************************* */
	/* Hint window. */
	HintWinDestroy();


	/* ************************************************************* */
	/* Cursors. */

	WidgetDestroyCursor(&widget_global.std_arrow_wcr);
        WidgetDestroyCursor(&widget_global.h_split_wcr);
        WidgetDestroyCursor(&widget_global.v_split_wcr);
	WidgetDestroyCursor(&widget_global.drag_item_wcr);
	WidgetDestroyCursor(&widget_global.drag_file_wcr);
	WidgetDestroyCursor(&widget_global.text_wcr);
	WidgetDestroyCursor(&widget_global.no_way_wcr);


	/* ************************************************************* */
	/* Background tile pixmaps. */

	OSWDestroyPixmap(&widget_global.std_bkg_pm);
	OSWDestroyPixmap(&widget_global.std_icon_pm);


        /* ************************************************************* */
	/* Dialog, list, etc images. */

        OSWDestroyImage(&widget_global.std_bkg_img);
        OSWDestroyImage(&widget_global.menu_bkg_img);
        OSWDestroyImage(&widget_global.hint_bkg_img);

        OSWDestroyImage(&widget_global.btn_unarmed_img);
        OSWDestroyImage(&widget_global.btn_armed_img);
        OSWDestroyImage(&widget_global.btn_highlighted_img);

        OSWDestroyImage(&widget_global.scalebar_h_img);
        OSWDestroyImage(&widget_global.scalebar_v_img);

	OSWDestroyImage(&widget_global.pulist_map_icon_img);

	OSWDestroyImage(&widget_global.browse_files_img);
        OSWDestroyImage(&widget_global.goto_parent_img);

	OSWDestroyImage(&widget_global.mount_img);
	OSWDestroyImage(&widget_global.unmount_img);

	OSWDestroyImage(&widget_global.stditem_normal_img);
        OSWDestroyImage(&widget_global.stditem_selected_img);

        OSWDestroyImage(&widget_global.diricon_normal_img);
        OSWDestroyImage(&widget_global.diricon_selected_img);

        OSWDestroyImage(&widget_global.fileicon_normal_img);
        OSWDestroyImage(&widget_global.fileicon_selected_img);

        OSWDestroyImage(&widget_global.linkicon_normal_img);
        OSWDestroyImage(&widget_global.linkicon_selected_img);

        OSWDestroyImage(&widget_global.pipeicon_normal_img);
        OSWDestroyImage(&widget_global.pipeicon_selected_img);

        OSWDestroyImage(&widget_global.execicon_normal_img);
        OSWDestroyImage(&widget_global.execicon_selected_img);

        OSWDestroyImage(&widget_global.toggle_btn_unarmed_img);
        OSWDestroyImage(&widget_global.toggle_btn_armed_img);

        OSWDestroyImage(&widget_global.tab_normal_img);
        OSWDestroyImage(&widget_global.tab_selected_img);


        /* ************************************************************* */
	/* Color pixels. */
	OSWDestroyPixel(&widget_global.surface_normal_pix);
        OSWDestroyPixel(&widget_global.surface_editable_pix);
        OSWDestroyPixel(&widget_global.surface_selected_pix);
        OSWDestroyPixel(&widget_global.surface_shadow_pix);
        OSWDestroyPixel(&widget_global.surface_highlight_pix);

        OSWDestroyPixel(&widget_global.scroll_bkg_pix);
        OSWDestroyPixel(&widget_global.scroll_frame_pix);
        OSWDestroyPixel(&widget_global.scroll_bar_pix);
        OSWDestroyPixel(&widget_global.scroll_cursor_pix);

        OSWDestroyPixel(&widget_global.normal_text_pix);
        OSWDestroyPixel(&widget_global.editable_text_pix);
        OSWDestroyPixel(&widget_global.selected_text_pix);
        OSWDestroyPixel(&widget_global.disabled_text_pix);

        OSWDestroyPixel(&widget_global.hint_bkg_pix);
        OSWDestroyPixel(&widget_global.hint_text_pix);


	/* ************************************************************ */
	/* Fonts. */
        OSWUnloadFont(&widget_global.pbtn_font);
        OSWUnloadFont(&widget_global.menu_font);
        OSWUnloadFont(&widget_global.prompt_label_font);
        OSWUnloadFont(&widget_global.prompt_text_font);
        OSWUnloadFont(&widget_global.scale_bar_font);
	OSWUnloadFont(&widget_global.std_font);


        /* ************************************************************ */
        /* Clear global record handlers. */

	PUListRepeatRecordClear();
	PromptRepeatRecordClear();
        SBarRepeatRecordClear();


	/* ************************************************************ */
	/* Warn if any widgets are still allocated. */
	for(i = 0; i < widget_reg.total_entries; i++)
        {
            if(widget_reg.entry[i] == NULL)
                continue;

            if(widget_reg.entry[i]->ptr != NULL)
            {
		fprintf(stderr,
                    "WidgetDestroyGlobals(): Warning: "
		);
		switch(widget_reg.entry[i]->type)
		{
		  case WTYPE_CODE_PUSHBUTTON:
		    fprintf(stderr, "PushButton");
		    break;

                  case WTYPE_CODE_COLUMLIST:
                    fprintf(stderr, "ColumList");
                    break;

                  case WTYPE_CODE_DIALOG:
                    fprintf(stderr, "Dialog");
                    break;

                  case WTYPE_CODE_FILEBROWSER:
                    fprintf(stderr, "FileBrowser");
                    break;

                  case WTYPE_CODE_LIST:
                    fprintf(stderr, "list");
                    break;

                  case WTYPE_CODE_MENU:
                    fprintf(stderr, "Menu");
                    break;

                  case WTYPE_CODE_MENUBAR:
                    fprintf(stderr, "MenuBar");
                    break;

                  case WTYPE_CODE_PAGESTEPPER:
                    fprintf(stderr, "PageStepper");
                    break;

                  case WTYPE_CODE_PROGRESSBAR:
                    fprintf(stderr, "ProgressBar");
                    break;

                  case WTYPE_CODE_PROMPT:
                    fprintf(stderr, "Prompt");
                    break;

                  case WTYPE_CODE_PULIST:
                    fprintf(stderr, "PopUpList");
                    break;

                  case WTYPE_CODE_SCALEBAR:
                    fprintf(stderr, "ScaleBar");
                    break;

                  case WTYPE_CODE_SCROLLBAR:
                    fprintf(stderr, "ScrollBar");
                    break;

                  case WTYPE_CODE_TOGGLEARRAY:
                    fprintf(stderr, "ToggleButtonArray");
                    break;

                  case WTYPE_CODE_TOGGLEBTN:
                    fprintf(stderr, "ToggleButton");
                    break;

                  case WTYPE_CODE_VIEWER:
                    fprintf(stderr, "Viewer");
                    break;

                  default:
                    fprintf(stderr, "Unknown");
                    break;
		}
		fprintf(stderr,
                    " widget #%i 0x%.8x was not destroyed.\n",
                    i, (unsigned int)widget_reg.entry[i]->ptr
		);
            }
        }

	/* Delete all widget regeristry entries. */
	WidgetRegDeleteAll();



	/* Mark as uninitizlied. */
        widget_global.is_init = 0;


	return;
}



int WidgetManage(event_t *event)
{
	static int events_handled;

	events_handled = 0;


	/*
	 *	Warning, pointer event can be NULL.
	 */

	/* ********************************************************* */
	/* Manage timmed callbacks. */

	events_handled += HintWinManage(event);
	events_handled += ListWinManageRepeat(event);
	events_handled += PUListManageRepeat(event);
	events_handled += PromptManageRepeat(event);
        events_handled += SBarManageRepeat(event);


	/* ********************************************************* */
	/* Manage events. */
	if(event != NULL)
	{










	}

	return(events_handled);
}




