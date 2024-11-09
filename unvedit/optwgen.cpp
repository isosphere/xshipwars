// unvedit/optwgen.cpp



/*
                        General options window

	Functions:

	void OptWGenDrawOutline(
	        drawable_t d,
	        int x, int y,
	        unsigned int width,
	        unsigned int height,
	        char *name,
	        pixel_t fg_pix,
	        pixel_t bg_pix,
	        pixel_t text_pix
	)

	int OptWGenDoSwitchTab(int tab_num)

        int OptWGenFetchValues()
	int OptWGenApplyValues()

	int OptWGenKeymapListDoSet(keycode_t keycode)

	int OptWGenKeymapListCB(void *ptr)
	int OptWGenScanKeyPBCB(void *ptr)
	int OptWGenOKPBCB(void *ptr)
        int OptWGenApplyPBCB(void *ptr)
        int OptWGenCancelPBCB(void *ptr)

	int OptWGenInit()
	int OptWGenDraw(int amount)
	int OptWGenManage(event_t *event)
	void OptWGenMap()
	void OptWGenDoMapValues()
	void OptWGenUnmap()
	void OptWGenDestroy()

	---


 */
/*
#include <stdio.h>
#include <db.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
*/
#include "../include/string.h"
#include "../include/strexp.h"

#include "../include/osw-x.h"
#include "../include/widget.h"

#include "ue.h"
#include "keymap.h"
#include "optwgen.h"

#ifndef MAX
#define MIN(a,b)        (((a) < (b)) ? (a) : (b))
#define MAX(a,b)	(((a) > (b)) ? (a) : (b))
#endif

/* #define MIN(a,b)	((a) < (b) ? (a) : (b)) */
/* #define MAX(a,b)	((a) > (b) ? (a) : (b)) */



void OptWGenDrawOutline(
        drawable_t d,
        int x, int y,
        unsigned int width,
        unsigned int height,
        char *name,
        pixel_t fg_pix,
        pixel_t bg_pix,
        pixel_t text_pix
);





/*
 *	Draws an outline box with label.
 */
void OptWGenDrawOutline(
        drawable_t d,
        int x, int y,
        unsigned int width,  
        unsigned int height,
        char *name,
        pixel_t fg_pix,
	pixel_t bg_pix,
        pixel_t text_pix
)
{
        int text_start_pos;     /* In pixels. */
        int text_len;           /* In pixels. (inc margin) */


        if((d == 0) ||
           (name == NULL)
        )
            return;
            
        text_start_pos = static_cast<int>((int)width * OPTWGEN_OUTLINE_NAME_POS);
        text_len = (strlen(name) + 2) * OPTWGEN_CHAR_WIDTH;


        /* Draw shadows. */
        OSWSetFgPix(bg_pix);
        OSWDrawLine(d,                  /* left */
             x + 2,
             y + (int)height - 2,
             x + 2,
             y + 2
        );
        OSWDrawLine(d,                  /* top1 */
             x + 4,
             y + 3,
             x + text_start_pos,
             y + 3
        );
        OSWDrawLine(d,                  /* Top2 */
             x + text_len + text_start_pos,
             y + 3,
             x + (int)width - 2,
             y + 3
        );
        OSWDrawLine(d,                  /* right */
            x + (int)width - 3,
            y + 3,
            x + (int)width - 3,
            y + (int)height - 3
        );
        OSWDrawLine(d,                  /* bottom */
            x + 3,
            y + (int)height - 3,
            x + (int)width - 3,
            y + (int)height - 3
        );

        /* Draw highlights. */
        OSWSetFgPix(fg_pix);
        OSWDrawLine(d,                  /* left */
             x + 3,
             y + (int)height - 3,
             x + 3,
             y + 3
        );
        OSWDrawLine(d,                  /* top1 */
             x + 3,
             y + 3,
             x + text_start_pos,
             y + 3
        );
        OSWDrawLine(d,                  /* Top2 */
             x + text_len + text_start_pos,
             y + 3,
             x + (int)width - 3,
             y + 3
        );
        OSWDrawLine(d,                  /* right */
            x + (int)width - 2,
            y + 3,
            x + (int)width - 2,
            y + (int)height - 2
        );
        OSWDrawLine(d,                  /* bottom */
            x + 3,
            y + (int)height - 2,
            x + (int)width - 2,
            y + (int)height - 2
        );

        /* Draw text. */
        OSWSetFgPix(text_pix);
        OSWDrawString(
            d,
            x + text_start_pos + (OPTWGEN_CHAR_WIDTH / 2) + 5,
            y + (OPTWGEN_CHAR_HEIGHT / 2) + 1,
            name
        );

             
        return;
}


/*
 *	Procedure to switch tabs.
 */
int OptWGenDoSwitchTab(int tab_num)
{
	optwgen_struct *ow;
	int prev_tab;
	char just_map = 0;

	ow = &optwgen;


	/* No change? */
	if(ow->tab == tab_num)
	    just_map = 1;

	prev_tab = ow->tab;
	ow->tab = tab_num;


	/* Unmap previous tab. */
	if(!just_map)
	{
	    switch(prev_tab)
	    {
	      case OPTWGEN_TAB_APPERANCE:
	        TgBtnUnmap(&ow->label_geometry_tb);
                TgBtnUnmap(&ow->show_grid_tb);
		PromptUnmap(&ow->grid_spacing_prompt);
	        break;

              case OPTWGEN_TAB_FONTS:
                PromptUnmap(&ow->view_font_prompt);
	        PromptUnmap(&ow->view_object_label_prompt);
                break;

              case OPTWGEN_TAB_IMAGES:
                TgBtnUnmap(&ow->show_preview_image_tb);
	        TgBtnUnmap(&ow->animate_image_tb);
                break;

              case OPTWGEN_TAB_PATHS:
                PromptUnmap(&ow->toplevel_path_prompt);
                PromptUnmap(&ow->images_path_prompt);
                PromptUnmap(&ow->server_path_prompt);
                break;

              case OPTWGEN_TAB_KEYMAP:
                CListUnmap(&ow->keymaps_list);
                PBtnUnmap(&ow->scan_key_btn);
                OSWUnmapWindow(ow->scan_key_win);
                ow->scanning_key = False;
		break;
	    }
	}

        /* Map newly selected tab. */
        switch(ow->tab)
        {
          case OPTWGEN_TAB_APPERANCE:
            TgBtnMap(&ow->label_geometry_tb);
            TgBtnMap(&ow->show_grid_tb);
            PromptMap(&ow->grid_spacing_prompt);
            break;

          case OPTWGEN_TAB_FONTS:
            PromptMap(&ow->view_font_prompt);
            PromptMap(&ow->view_object_label_prompt);
            break;

          case OPTWGEN_TAB_IMAGES:
            TgBtnMap(&ow->show_preview_image_tb);
            TgBtnMap(&ow->animate_image_tb);
            break;

          case OPTWGEN_TAB_PATHS:
            PromptMap(&ow->toplevel_path_prompt);
            PromptMap(&ow->images_path_prompt); 
            PromptMap(&ow->server_path_prompt);
            break;

          case OPTWGEN_TAB_KEYMAP:
            CListMap(&ow->keymaps_list);
            PBtnMap(&ow->scan_key_btn);
	    if(ow->scanning_key)
		OSWMapRaised(ow->scan_key_win);
	    else
	        OSWUnmapWindow(ow->scan_key_win);
            break;
        }


	return(0);
}

/*
 *	Fetches global values and puts them into widget on general
 *	options window.
 */
int OptWGenFetchValues()
{
	int i;
        optwgen_struct *ow;
	char text[256];
 
        
        ow = &optwgen;


	/* Apperance. */
	ow->label_geometry_tb.state = ((option.label_geometry) ?
	    True : False
	);

        ow->show_grid_tb.state = ((option.show_grid) ?
            True : False
        );

	PromptSetF(&ow->grid_spacing_prompt, option.grid_spacing);


	/* Fonts. */
	PromptSetS(&ow->view_font_prompt, option.view_font_name);
        PromptSetS(&ow->view_object_label_prompt, option.view_object_label_font_name);


	/* Images. */
        ow->show_preview_image_tb.state = ((option.show_preview_image) ?
            True : False
        );

        ow->animate_image_tb.state = ((option.animate_images) ?
            True : False
        );


	/* Paths. */
	PromptSetS(&ow->toplevel_path_prompt, dname.toplevel);
        PromptSetS(&ow->images_path_prompt, dname.images);
        PromptSetS(&ow->server_path_prompt, dname.server);



	/* Keymaps. */
	CListDeleteAllRows(&ow->keymaps_list);
        for(i = 0; i < (int)TOTAL_KEYMAPS; i++)
        {
            if(CListAddRow(
                &ow->keymaps_list,
                -1
            ))
                return(-1);

            CListAddItem(
                &ow->keymaps_list,
                keymap_name[i],
                OSWQueryCurrentFont(),
                widget_global.editable_text_pix,
                0,              /* Attributes. */
                i               /* Row number. */
            );

	    sprintf(text, "%i", keymap[i].keycode);
            CListAddItem(
                &ow->keymaps_list,
                text, 
                OSWQueryCurrentFont(),
                widget_global.editable_text_pix,
                0,              /* Attributes. */
                i               /* Row number. */
            );
        }


	/* Reset has changes mark. */
        ow->has_changes = False;


	return(0);
}

/*
 *      Applies global values from widgets on general options window.
 */
int OptWGenApplyValues()
{
	int i, n;
        colum_list_struct *list;
        colum_list_row_struct *row;
	char *strptr;
        optwgen_struct *ow;


        ow = &optwgen;


	/* Apperance. */
	option.label_geometry = ((ow->label_geometry_tb.state) ?
	    1 : 0
        );

	option.show_grid = ((ow->show_grid_tb.state) ?
            1 : 0
        );

	option.grid_spacing = PromptGetF(&ow->grid_spacing_prompt);
	if(option.grid_spacing < 0.001)
	    option.grid_spacing = 0.001;

	/* Fonts. */
	strptr = PromptGetS(&ow->view_font_prompt);
	if(strptr != NULL)
	{
	    strncpy(
	        option.view_font_name,
	        strptr,
		FontNameMax
	    );
	    option.view_font_name[FontNameMax - 1] = '\0';

            OSWUnloadFont(&ue_font.view_label);
            OSWLoadFont(
                &ue_font.view_label,
                option.view_font_name
	    );
	}

        strptr = PromptGetS(&ow->view_object_label_prompt);
        if(strptr != NULL)
        {
            strncpy(
                option.view_object_label_font_name,
                strptr,
                FontNameMax
            );
            option.view_object_label_font_name[FontNameMax - 1] = '\0';

	    OSWUnloadFont(&ue_font.view_obj_label);
            OSWLoadFont(
                &ue_font.view_obj_label,
                option.view_object_label_font_name
            );
        }


	/* Images. */
	option.show_preview_image = ((ow->show_preview_image_tb.state) ?
	    1 : 0
	);

	option.animate_images = ((ow->animate_image_tb.state) ?
	    1 : 0
	);


        /* Paths. */
        strptr = PromptGetS(&ow->toplevel_path_prompt);
        if(strptr != NULL)
        {
            strncpy(
                dname.toplevel,
                strptr,
                PATH_MAX
            );
            dname.toplevel[PATH_MAX - 1] = '\0';
        }

	strptr = PromptGetS(&ow->images_path_prompt);
	if(strptr != NULL)
	{
	    strncpy(
		dname.images,
		strptr,
		PATH_MAX
	    );
	    dname.images[PATH_MAX - 1] = '\0';
	}

        strptr = PromptGetS(&ow->server_path_prompt);
        if(strptr != NULL) 
        {
            strncpy(
                dname.server,
                strptr,
                PATH_MAX
            );
            dname.server[PATH_MAX - 1] = '\0';
        }


	/* Keymaps. */
	list = &ow->keymaps_list;
        for(i = 0; i < (int)TOTAL_KEYMAPS; i++)
        {
	    /* Break just incase list has fewer rows. */
	    if(i >= list->total_rows)
		break;

	    row = list->row[i];
	    if(row == NULL)
		continue;

	    n = 1;
	    if(row->total_items <= n)
		continue;
	    if(row->item[n] == NULL)
		continue;
	    if(row->item[n]->label == NULL)
		continue;

	    keymap[i].keycode = atoi(row->item[n]->label);
	}


        /* Reset has changes mark. */
	ow->has_changes = False;


	return(0);
}


/*
 *	Procedure to set currently selected keymap item
 *	to value of keycode.
 */
int OptWGenKeymapListDoSet(keycode_t keycode)
{
	int s, n;
	colum_list_struct *list;
	colum_list_row_struct *row;
	char text[256];
        optwgen_struct *ow;
  
  
        ow = &optwgen;
	list = &ow->keymaps_list;

	/* Nothing selected. */
	if(list->total_sel_rows <= 0)
	    return(0);

	/* Get last selected. */
	s = list->sel_row[list->total_sel_rows - 1];
	if((s < 0) || (s >= list->total_rows))
	    return(0);

	row = list->row[s];
	if(row == NULL)
	    return(-1);

	n = 1;
	if(row->total_items <= n)
	    return(-1);


	sprintf(text, "%i", keycode);

	free(row->item[n]->label);
	row->item[n]->label = StringCopyAlloc(text);


	if(list->map_state)
	    CListDraw(list, CL_DRAW_AMOUNT_LIST);


	return(0);
}


/*
 *	Keymap list callback.
 */
int OptWGenKeymapListCB(void *ptr)
{
	int status;
        optwgen_struct *ow;


        ow = &optwgen;

	status = OptWGenScanKeyPBCB((void *)&ow->scan_key_btn);


	return(status);
}

/*
 *	Scan key button callback.
 */
int OptWGenScanKeyPBCB(void *ptr)
{
        optwgen_struct *ow;


        ow = &optwgen;

	OSWMapRaised(ow->scan_key_win);
	ow->scanning_key = True;


	return(0);
}

/*
 *	OK button callback.
 */
int OptWGenOKPBCB(void *ptr)
{
	OptWGenApplyValues();

	OptWGenUnmap();


        return(0);
}

/*      
 *	Apply button callback.
 */
int OptWGenApplyPBCB(void *ptr)
{
        OptWGenApplyValues();

        return(0);
}

/*
 *	Cancel button callback.
 */
int OptWGenCancelPBCB(void *ptr)
{
	OptWGenUnmap();

        return(0);
}



int OptWGenInit()
{
	int x, y;
        win_attr_t wattr;
	unsigned int width, height;
        optwgen_struct *ow;


        ow = &optwgen;


        ow->map_state = 0;
        ow->is_in_focus = 0;
        ow->x = 0;
        ow->y = 0;
        ow->width = OPTWGEN_WIDTH;
        ow->height = OPTWGEN_HEIGHT;

        ow->has_changes = False;
	ow->tab = OPTWGEN_TAB_APPERANCE;


        /* Create toplevel. */
        if(
            OSWCreateWindow(
                &ow->toplevel,
                osw_gui[0].root_win,
                ow->x, ow->y,
                ow->width, ow->height
            )
        )
            return(-1);

        OSWSetWindowWMProperties(
            ow->toplevel,
            OPTWGEN_DEF_TITLE,
            OPTWGEN_DEF_ICON_TITLE,
            widget_global.std_icon_pm,          /* Icon. */
            False,                              /* Let WM set coordinates? */
            ow->x, ow->y,
            100, 72,
            osw_gui[0].display_width, osw_gui[0].display_height,
            WindowFrameStyleFixed,
            NULL, 0
        );

        OSWSetWindowInput(
            ow->toplevel,
            OSW_EVENTMASK_TOPLEVEL
        );

        WidgetCenterWindow(ow->toplevel, WidgetCenterWindowToRoot);
        OSWGetWindowAttributes(ow->toplevel, &wattr);
        ow->x = wattr.x;
        ow->y = wattr.y;
        ow->width = wattr.width;
        ow->height = wattr.height;
	ow->scanning_key = False;


	/* Create tab windows. */
	width = ((widget_global.tab_normal_img == NULL) ?
	    10 : widget_global.tab_normal_img->width
	);
        height = ((widget_global.tab_normal_img == NULL) ?
            10 : widget_global.tab_normal_img->height
        );

	/* Apperance tab. */
        if(
            OSWCreateWindow(
                &ow->apperance_tab_win,
                ow->toplevel,
                (1 * 10) + (0 * (int)width) + 20, 20,
                width, height
            )
        )
            return(-1);
        OSWSetWindowInput(ow->apperance_tab_win, OSW_EVENTMASK_BUTTON);

	/* Fonts tab. */
        if(
            OSWCreateWindow(
                &ow->fonts_tab_win,
                ow->toplevel,
                (2 * 10) + (1 * (int)width) + 20, 20,
                width, height
            )
        )
            return(-1);
        OSWSetWindowInput(ow->fonts_tab_win, OSW_EVENTMASK_BUTTON);

        /* Images tab. */
        if(
            OSWCreateWindow(
                &ow->images_tab_win,
                ow->toplevel,
                (3 * 10) + (2 * (int)width) + 20, 20,
                width, height
            )
        )
            return(-1);
        OSWSetWindowInput(ow->images_tab_win, OSW_EVENTMASK_BUTTON);

        /* Paths tab. */
        if(
            OSWCreateWindow(
                &ow->paths_tab_win,
                ow->toplevel,
                (4 * 10) + (3 * (int)width) + 20, 20,
                width, height
            )
        )
            return(-1);
        OSWSetWindowInput(ow->paths_tab_win, OSW_EVENTMASK_BUTTON);

        /* Paths tab. */
        if(
            OSWCreateWindow(
                &ow->keymap_tab_win,
                ow->toplevel,
                (5 * 10) + (4 * (int)width) + 20, 20,
                width, height
            )
        )
            return(-1);
        OSWSetWindowInput(ow->keymap_tab_win, OSW_EVENTMASK_BUTTON);


        /* Buttons. */   
        if(
            PBtnInit(
                &ow->ok_btn,
                ow->toplevel,
                (1 * 10) + (0 * OPTWGEN_BUTTON_WIDTH),
                (int)ow->height - 10 - OPTWGEN_BUTTON_HEIGHT,
                OPTWGEN_BUTTON_WIDTH, OPTWGEN_BUTTON_HEIGHT,
                "OK",
                PBTN_TALIGN_CENTER,
                NULL,
                (void *)&ow->ok_btn,
                OptWGenOKPBCB
            )
        )
            return(-1);

        if(
            PBtnInit(
                &ow->apply_btn,
                ow->toplevel,
                ((int)ow->width / 2) - (OPTWGEN_BUTTON_WIDTH / 2),
                (int)ow->height - 10 - OPTWGEN_BUTTON_HEIGHT,
                OPTWGEN_BUTTON_WIDTH, OPTWGEN_BUTTON_HEIGHT,
                "Apply", 
                PBTN_TALIGN_CENTER,
                NULL,
                (void *)&ow->apply_btn,
                OptWGenApplyPBCB
            )
        )
            return(-1);

        if(
            PBtnInit(
                &ow->cancel_btn,
                ow->toplevel,
                (int)ow->width - OPTWGEN_BUTTON_WIDTH - 10,
                (int)ow->height - 10 - OPTWGEN_BUTTON_HEIGHT,
                OPTWGEN_BUTTON_WIDTH, OPTWGEN_BUTTON_HEIGHT,
                "Cancel",
                PBTN_TALIGN_CENTER,
                NULL,
                (void *)&ow->cancel_btn,
                OptWGenCancelPBCB
            )
        )
            return(-1);


	/* Widgets on apperance tab. */
	x = 20;
	y = (int)((widget_global.tab_normal_img == NULL) ?
	    10 : widget_global.tab_normal_img->height) + 20;
        if(
            TgBtnInit(
                &ow->label_geometry_tb,
                ow->toplevel,
                x + 30,
		y + 30,
                True,
                "Show Geometry Labels"
            )
        )
            return(-1);

        if(
            TgBtnInit(
                &ow->show_grid_tb,
                ow->toplevel,
                x + 30,
                y + 60,
                True,
                "Show Grid"
            )
        )
            return(-1);

        if(
            PromptInit(
                &ow->grid_spacing_prompt,
                ow->toplevel,
                x + 30,
                y + 85,
                220,
                OPTWGEN_PROMPT_HEIGHT, 
                PROMPT_STYLE_FLUSHED,
                "Grid Spacing (RU):",
                80,
                0,   
                NULL
            )
        )
            return(-1);


	/* Widgets on fonts tab. */
        x = 20;
        y = (int)((widget_global.tab_normal_img == NULL) ?
            10 : widget_global.tab_normal_img->height) + 20;
        if(
            PromptInit(   
                &ow->view_font_prompt,
                ow->toplevel,
                x + 30,
		y + 30,
                280,
                OPTWGEN_PROMPT_HEIGHT,
                PROMPT_STYLE_FLUSHED,
                "Geometry Labels:",
                256,
                0,
                NULL
            )
        )
            return(-1);

        if(
            PromptInit(
                &ow->view_object_label_prompt,
                ow->toplevel,
                x + 30,
                y + 65,
                280,
                OPTWGEN_PROMPT_HEIGHT,
                PROMPT_STYLE_FLUSHED,
                "Object Labels:",  
                256,
                0,
                NULL   
            )
        )
            return(-1);

	/* Link prompts togeather. */
        ow->view_font_prompt.next = &ow->view_object_label_prompt;
        ow->view_font_prompt.prev = &ow->view_object_label_prompt;

        ow->view_object_label_prompt.next = &ow->view_font_prompt;
        ow->view_object_label_prompt.prev = &ow->view_font_prompt;


	/* Widgets on images tab. */
        x = 20;
        y = (int)((widget_global.tab_normal_img == NULL) ?
            10 : widget_global.tab_normal_img->height) + 20;
        if(
            TgBtnInit(
                &ow->show_preview_image_tb,
                ow->toplevel,
                x + 30,
                y + 30,
                True,
                "Show Preview"
            )
        )
            return(-1);

        if(     
            TgBtnInit(
                &ow->animate_image_tb,
                ow->toplevel,
                x + 30,
                y + 60,
                True,
                "Animate Preview"
            )
        )
            return(-1);

        /* Widgets on paths tab. */
        x = 20;
        y = (int)((widget_global.tab_normal_img == NULL) ?
            10 : widget_global.tab_normal_img->height) + 20;

        if(
            PromptInit(
                &ow->toplevel_path_prompt,
                ow->toplevel,
                x + 30,
                y + 30,
                290,
                OPTWGEN_PROMPT_HEIGHT,
                PROMPT_STYLE_FLUSHED,
                "Toplevel:",
                PATH_MAX + NAME_MAX,
                0,  
                NULL
            )
        )
            return(-1);

        if(
            PromptInit(
                &ow->images_path_prompt,
                ow->toplevel,
                x + 30,
                y + 65,
                290,
                OPTWGEN_PROMPT_HEIGHT,
                PROMPT_STYLE_FLUSHED,
                "Images:",
                PATH_MAX + NAME_MAX,
                0,
                NULL
            )
        )
            return(-1);

       if(
            PromptInit(
                &ow->server_path_prompt,
                ow->toplevel,
                x + 30,
                y + 100,
                290,
                OPTWGEN_PROMPT_HEIGHT,
                PROMPT_STYLE_FLUSHED,
                "Server:",
                PATH_MAX + NAME_MAX,
                0,
                NULL
            )
        )
            return(-1);

	/* Link prompts togeather. */
	ow->toplevel_path_prompt.next = &ow->images_path_prompt;
	ow->toplevel_path_prompt.prev = &ow->server_path_prompt;

        ow->images_path_prompt.next = &ow->server_path_prompt;
        ow->images_path_prompt.prev = &ow->toplevel_path_prompt;

        ow->server_path_prompt.next = &ow->toplevel_path_prompt;
        ow->server_path_prompt.prev = &ow->images_path_prompt;


        /* Widgets on keymaps tab. */
        x = 20;
        y = (int)((widget_global.tab_normal_img == NULL) ?
            10 : widget_global.tab_normal_img->height) + 20;

	if(
	    CListInit(
                &ow->keymaps_list,
                ow->toplevel,
                x + 40, y + 40,
                290, 280,
		(void *)&ow->keymaps_list,
                OptWGenKeymapListCB
            )
	)
	    return(-1);
	CListAddHeading(
            &ow->keymaps_list,
            "Keymap",
            OSWQueryCurrentFont(), 
            widget_global.editable_text_pix,
            0,		/* Attributes. */
            0		/* Start pos. */
	);
        CListAddHeading(
            &ow->keymaps_list,
            "Keycode",
            OSWQueryCurrentFont(),
            widget_global.editable_text_pix,
            0,		/* Attributes. */
            200		/* Start pos. */
	);

        if(
            PBtnInit(
                &ow->scan_key_btn,
                ow->toplevel,
                x + 340, y + 40,
                OPTWGEN_BUTTON_WIDTH, OPTWGEN_BUTTON_HEIGHT,
                "Scan Key",
                PBTN_TALIGN_CENTER,
                NULL,
                (void *)&ow->scan_key_btn,
                OptWGenScanKeyPBCB
            )
        )
            return(-1);

	width = MAX((int)ow->width - (2 * 10), 10);
	height = (4 * 16) + (2 * 10);
        if(
            OSWCreateWindow(
                &ow->scan_key_win,
                ow->toplevel,
		10,
		((int)ow->height / 2) - ((int)height / 2),
		width, height
            )
        )
            return(-1);
        OSWSetWindowInput(
	    ow->scan_key_win,
	    ButtonPressMask | ButtonReleaseMask | ExposureMask
        );
        OSWLoadPixelRGB(&ow->scan_key_fg_pix,
            0xff,
            0xff,
            0xff
        );
        OSWLoadPixelRGB(&ow->scan_key_bg_pix,
            0x18,
            0x30,
            0xff
        );



	return(0);
}


int OptWGenDraw(int amount)
{
	int x, y;
	char *strptr;
	win_t w;
	pixmap_t pixmap;
	win_attr_t wattr;
        optwgen_struct *ow;
            
            
        ow = &optwgen;

	if(!ow->map_state)
	{
	    OSWMapRaised(ow->toplevel);

            OSWMapWindow(ow->apperance_tab_win);
            OSWMapWindow(ow->fonts_tab_win);
            OSWMapWindow(ow->images_tab_win);
            OSWMapWindow(ow->paths_tab_win);
            OSWMapWindow(ow->keymap_tab_win);

	    ow->map_state = 1;

	    OptWGenDoSwitchTab(ow->tab);

	    PBtnMap(&ow->ok_btn);
            PBtnMap(&ow->apply_btn);
            PBtnMap(&ow->cancel_btn);

	    /* Map/unmap scan key prompt window. */
	    if(ow->scanning_key)
		OSWMapRaised(ow->scan_key_win);
	    else
		OSWUnmapWindow(ow->scan_key_win);

	    amount = OPTWGEN_DRAW_AMOUNT_COMPLETE;
	}

        /* Recreate buffers as needed. */
        if(ow->toplevel_buf == 0)  
        {
            OSWGetWindowAttributes(ow->toplevel, &wattr);
            
            if(   
                OSWCreatePixmap(
                    &ow->toplevel_buf, wattr.width, wattr.height
                )
            )
                return(0);
        }

	/* Toplevel. */
        if(amount == OPTWGEN_DRAW_AMOUNT_COMPLETE)
        {
            w = ow->toplevel;
            pixmap = ow->toplevel_buf;

            OSWGetWindowAttributes(w, &wattr);

            /* Redraw background. */
            if(widget_global.force_mono)
            {
                OSWClearPixmap(
                    pixmap,
                    wattr.width, wattr.height,
                    osw_gui[0].black_pix
                );

                OSWSetFgPix(osw_gui[0].white_pix);
            }
            else
            {
                WidgetPutImageTile(
                    pixmap,
                    widget_global.std_bkg_img,
                    wattr.width, wattr.height
                );

                OSWSetFgPix(widget_global.normal_text_pix);
            }     


	    /* Draw main outline. */
            x = 20;
            y = (int)((widget_global.tab_normal_img == NULL) ?
                10 : widget_global.tab_normal_img->height) + 19;

            if(widget_global.force_mono)
            {
                OSWSetFgPix(osw_gui[0].white_pix);
                OSWDrawLine(
                    pixmap,
		    x,
		    (int)ow->height - OPTWGEN_BUTTON_HEIGHT - 30,
                    x,
		    y
                );
                OSWDrawLine(
                    pixmap,
                    x,
		    y,
		    (int)ow->width - 20,
		    y
                );
                OSWDrawLine(
                    pixmap,
                    (int)ow->width - 20,
		    y,
		    (int)ow->width - 20,
		    (int)ow->height - OPTWGEN_BUTTON_HEIGHT - 30
                );
                OSWDrawLine(
                    pixmap,
                    x,
		    (int)ow->height - OPTWGEN_BUTTON_HEIGHT - 30,
                    (int)ow->width - 20,
		    (int)ow->height - OPTWGEN_BUTTON_HEIGHT - 30
                );
            }
            else
            {
                OSWSetFgPix(widget_global.surface_highlight_pix);
                OSWDrawLine(
                    pixmap,
                    x, 
                    (int)ow->height - OPTWGEN_BUTTON_HEIGHT - 30,
                    x,  
                    y
                );
                OSWDrawLine(
                    pixmap,
                    x,   
                    y,
                    (int)ow->width - 20,  
                    y
                );

                OSWSetFgPix(widget_global.surface_shadow_pix);
                OSWDrawLine(
                    pixmap,
                    (int)ow->width - 20,
                    y,
                    (int)ow->width - 20,
                    (int)ow->height - OPTWGEN_BUTTON_HEIGHT - 30
                );
                OSWDrawLine(
                    pixmap, 
                    x, 
                    (int)ow->height - OPTWGEN_BUTTON_HEIGHT - 30,
                    (int)ow->width - 20,
                    (int)ow->height - OPTWGEN_BUTTON_HEIGHT - 30
                );
            }


	    /* Draw stuff specific to each tab. */

            if(ow->tab == OPTWGEN_TAB_APPERANCE)
	    {
		OptWGenDrawOutline(
		    pixmap,
                    x + 20, y + 20,
		    560, 110,
		    "View",
		    widget_global.surface_highlight_pix,
		    widget_global.surface_shadow_pix,
		    widget_global.normal_text_pix
		);
	    }
            else if(ow->tab == OPTWGEN_TAB_FONTS)
            {
                OptWGenDrawOutline(
                    pixmap,
                    x + 20, y + 20,
                    560, 90,
                    "Fonts",
                    widget_global.surface_highlight_pix,
                    widget_global.surface_shadow_pix,
                    widget_global.normal_text_pix
                );
            }
            else if(ow->tab == OPTWGEN_TAB_IMAGES)
            {
                OptWGenDrawOutline(
                    pixmap,
                    x + 20, y + 20,
                    560, 70,
                    "Preview",        
                    widget_global.surface_highlight_pix,
                    widget_global.surface_shadow_pix,
                    widget_global.normal_text_pix
                );
            }
            else if(ow->tab == OPTWGEN_TAB_PATHS)
            {
                OptWGenDrawOutline(
                    pixmap,
                    x + 20, y + 20,
                    560, 125,
                    "Directories",
                    widget_global.surface_highlight_pix,
                    widget_global.surface_shadow_pix,
                    widget_global.normal_text_pix
                );
            }
            else if(ow->tab == OPTWGEN_TAB_KEYMAP)
            {
                OptWGenDrawOutline(  
                    pixmap,
                    x + 20, y + 20,
                    560, 330,
                    "Keymaps",
                    widget_global.surface_highlight_pix,
                    widget_global.surface_shadow_pix,
                    widget_global.normal_text_pix
                );
            }

            OSWPutBufferToWindow(w, pixmap);
	}


	/* Draw tabs. */
	if((amount == OPTWGEN_DRAW_AMOUNT_COMPLETE) ||
           (amount == OPTWGEN_DRAW_AMOUNT_TABS)
	)
	{
	    /* Apperance tab. */
	    w = ow->apperance_tab_win;
	    pixmap = ow->toplevel_buf;
            OSWGetWindowAttributes(w, &wattr);

            WidgetPutImageTile(
                pixmap,
                widget_global.std_bkg_img,
                wattr.width, wattr.height
            );
            if(ow->tab == OPTWGEN_TAB_APPERANCE)
            {
                WidgetPutImageNormal(
                    pixmap,
                    widget_global.tab_selected_img,
                    0, 0,
                    True
                );
	    }
	    else
	    {
                WidgetPutImageNormal(
                    pixmap,
                    widget_global.tab_normal_img,
                    0, 0,
                    True
                );
	    }
            strptr = "Apperance";
	    OSWSetFgPix(((widget_global.force_mono) ?
		osw_gui[0].white_pix : widget_global.normal_text_pix
	    ));
            OSWDrawString(
		pixmap,
                ((int)wattr.width / 2) - ((strlen(strptr) * 7) / 2),
                (14 / 2) + 14,
                strptr
            );
            OSWPutBufferToWindow(w, pixmap);


            /* Fonts tab. */
            w = ow->fonts_tab_win;
            pixmap = ow->toplevel_buf;
            OSWGetWindowAttributes(w, &wattr);

            WidgetPutImageTile( 
                pixmap,
                widget_global.std_bkg_img,
                wattr.width, wattr.height
            );
            if(ow->tab == OPTWGEN_TAB_FONTS)
            {
                WidgetPutImageNormal(
                    pixmap,
                    widget_global.tab_selected_img,
                    0, 0,
                    True
                );
            }
            else
            {
                WidgetPutImageNormal(
                    pixmap,
                    widget_global.tab_normal_img,
                    0, 0,
                    True
                );
            }
            strptr = "Fonts";
            OSWSetFgPix(((widget_global.force_mono) ?
                osw_gui[0].white_pix : widget_global.normal_text_pix
            ));
            OSWDrawString(
                pixmap,
                ((int)wattr.width / 2) - ((strlen(strptr) * 7) / 2),
                (14 / 2) + 14,
                strptr
            );
            OSWPutBufferToWindow(w, pixmap);


            /* Images tab. */
            w = ow->images_tab_win;
            pixmap = ow->toplevel_buf;
            OSWGetWindowAttributes(w, &wattr);

            WidgetPutImageTile(
                pixmap,
                widget_global.std_bkg_img,
                wattr.width, wattr.height
            );
            if(ow->tab == OPTWGEN_TAB_IMAGES)
            {
                WidgetPutImageNormal(
                    pixmap,
                    widget_global.tab_selected_img,
                    0, 0,
                    True
                );
            }
            else    
            {
                WidgetPutImageNormal(
                    pixmap,
                    widget_global.tab_normal_img,
                    0, 0,
                    True
                );
            }
            strptr = "Images";
            OSWSetFgPix(((widget_global.force_mono) ?
                osw_gui[0].white_pix : widget_global.normal_text_pix
            ));
            OSWDrawString(
                pixmap,
                ((int)wattr.width / 2) - ((strlen(strptr) * 7) / 2),
                (14 / 2) + 14,
                strptr
            );
            OSWPutBufferToWindow(w, pixmap);


            /* Paths tab. */
            w = ow->paths_tab_win;
            pixmap = ow->toplevel_buf;
            OSWGetWindowAttributes(w, &wattr);

            WidgetPutImageTile(
                pixmap,
                widget_global.std_bkg_img,
                wattr.width, wattr.height
            );
            if(ow->tab == OPTWGEN_TAB_PATHS)
            {
                WidgetPutImageNormal(
                    pixmap,
                    widget_global.tab_selected_img,
                    0, 0,
                    True
                );
            }
            else
            {
                WidgetPutImageNormal(
                    pixmap,
                    widget_global.tab_normal_img,
                    0, 0,
                    True
                );
            }
            strptr = "Paths";
            OSWSetFgPix(((widget_global.force_mono) ?
                osw_gui[0].white_pix : widget_global.normal_text_pix
            ));
            OSWDrawString(
                pixmap,
                ((int)wattr.width / 2) - ((strlen(strptr) * 7) / 2),
                (14 / 2) + 14,   
                strptr
            );
            OSWPutBufferToWindow(w, pixmap);


            /* Keymap tab. */
            w = ow->keymap_tab_win;
            pixmap = ow->toplevel_buf;
            OSWGetWindowAttributes(w, &wattr);

            WidgetPutImageTile(
                pixmap,
                widget_global.std_bkg_img,
                wattr.width, wattr.height
            );
            if(ow->tab == OPTWGEN_TAB_KEYMAP)
            {
                WidgetPutImageNormal(
                    pixmap,
                    widget_global.tab_selected_img,
                    0, 0,
                    True
                );
            }
            else
            {
                WidgetPutImageNormal(
                    pixmap,
                    widget_global.tab_normal_img,
                    0, 0,
                    True
                );
            }
            strptr = "Keymaps";
            OSWSetFgPix(((widget_global.force_mono) ?
                osw_gui[0].white_pix : widget_global.normal_text_pix
            ));
            OSWDrawString(
                pixmap, 
                ((int)wattr.width / 2) - ((strlen(strptr) * 7) / 2),
                (14 / 2) + 14,
                strptr
            );
            OSWPutBufferToWindow(w, pixmap);
	}


	/* Draw scan key prompt window. */
        if((amount == OPTWGEN_DRAW_AMOUNT_COMPLETE) || 
           (amount == OPTWGEN_DRAW_AMOUNT_SCANKEY)
        )
	{
	    if(ow->scanning_key)
	    {
                w = ow->scan_key_win;
                pixmap = ow->toplevel_buf;
                OSWGetWindowAttributes(w, &wattr);

		OSWClearPixmap(
		    pixmap,
		    wattr.width, wattr.height,
		    ((widget_global.force_mono) ?
			osw_gui[0].white_pix : ow->scan_key_bg_pix
		    )
		);

                OSWSetFgPix(
		    ((widget_global.force_mono) ?
                        osw_gui[0].black_pix : ow->scan_key_fg_pix
                    )
		);

		OSWDrawString(
                    pixmap,
                    10 + 5,
                    10 + (0 * 16) + (14 / 2) + 5,
                    "Press a key on the keyboard to set the keycode."
                );
                OSWDrawString(
                    pixmap,
                    10 + 5,
                    10 + (2 * 16) + (14 / 2) + 5,
                    "Press Button1 to cancel."
                );
                OSWDrawString(
                    pixmap,
                    10 + 5,
                    10 + (3 * 16) + (14 / 2) + 5,
                    "Press Button3 to clear."
                );

                OSWPutBufferToWindow(w, pixmap);
	    }
	}



	return(0);
}


int OptWGenManage(event_t *event)
{
	int events_handled = 0;

        optwgen_struct *ow;
            
            
        ow = &optwgen;

	if(!ow->map_state &&
           (event->type != MapNotify)
	)
	    return(events_handled);



	switch(event->type)
	{
          /* ******************************************************** */
	  case KeyPress:
	    if(!ow->is_in_focus)
		return(events_handled);

	    /* Scan key. */
	    if(ow->scanning_key)
	    {
		OptWGenKeymapListDoSet(event->xkey.keycode);

		OSWUnmapWindow(ow->scan_key_win);
		ow->scanning_key = False;

		events_handled++;
		return(events_handled);
	    }
	    break;

          /* ******************************************************** */
          case KeyRelease:
            if(!ow->is_in_focus)
                return(events_handled);

            break;

          /* ******************************************************** */
          case ButtonPress:
            if(event->xany.window == ow->apperance_tab_win)
	    {
		OptWGenDoSwitchTab(OPTWGEN_TAB_APPERANCE);
		events_handled++;
	    }
            else if(event->xany.window == ow->fonts_tab_win)
            {
                OptWGenDoSwitchTab(OPTWGEN_TAB_FONTS);
                events_handled++;
            }
            else if(event->xany.window == ow->images_tab_win)
            {
                OptWGenDoSwitchTab(OPTWGEN_TAB_IMAGES);
                events_handled++;
            }
            else if(event->xany.window == ow->paths_tab_win)
            {
                OptWGenDoSwitchTab(OPTWGEN_TAB_PATHS);
                events_handled++;
            }
            else if(event->xany.window == ow->keymap_tab_win)
            {
                OptWGenDoSwitchTab(OPTWGEN_TAB_KEYMAP);
                events_handled++;
            }
            else if(event->xany.window == ow->scan_key_win)
            {
		switch(event->xbutton.button)
		{
		  case Button1:
		    break;

                  case Button2:
                    break;

                  case Button3:
                    OptWGenKeymapListDoSet(0);
                    break;
		}

                OSWUnmapWindow(ow->scan_key_win);
                ow->scanning_key = False;

                events_handled++;
                return(events_handled);
            }
            break;

          /* ******************************************************** */
          case Expose:
            if(event->xany.window == ow->toplevel)
            {
		OptWGenDraw(OPTWGEN_DRAW_AMOUNT_COMPLETE);

                events_handled++;
		return(events_handled);
            }
            else if((event->xany.window == ow->apperance_tab_win) ||
                    (event->xany.window == ow->fonts_tab_win) ||
                    (event->xany.window == ow->images_tab_win) ||
                    (event->xany.window == ow->paths_tab_win) ||
                    (event->xany.window == ow->keymap_tab_win)
	    )
            {
                OptWGenDraw(OPTWGEN_DRAW_AMOUNT_TABS);

                events_handled++;
                return(events_handled);
            }
            else if(event->xany.window == ow->scan_key_win)
            {
                OptWGenDraw(OPTWGEN_DRAW_AMOUNT_SCANKEY);

                events_handled++;
                return(events_handled);
            }
            break;

          /* ******************************************************** */
          case FocusIn:
            if(event->xany.window == ow->toplevel)
            {
                ow->is_in_focus = 1;

                events_handled++;
                return(events_handled);
            }
            break;

          /* ******************************************************** */
          case FocusOut:
            if(event->xany.window == ow->toplevel)
            {
                ow->is_in_focus = 0;

                events_handled++;
                return(events_handled);
            }
            break;

          /* ******************************************************** */
	  case UnmapNotify:
            if(event->xany.window == ow->toplevel)
            {
                OptWGenUnmap();

                events_handled++;
                return(events_handled);
            }
            break;

          /* ******************************************************** */
	  case MapNotify:
            if(event->xany.window == ow->toplevel)
            {
		if(!ow->map_state)
                    OptWGenMap();

                events_handled++;
                return(events_handled);
            }
            break;

          /* ******************************************************** */
          case ClientMessage:
            if(OSWIsEventDestroyWindow(ow->toplevel, event))
            {             
                OptWGenCancelPBCB(&ow->cancel_btn);

                events_handled++;
                return(events_handled);
            }
            break;
	}

        if(events_handled > 0)
        {
            OptWGenDraw(OPTWGEN_DRAW_AMOUNT_COMPLETE);
        }



	/* Manage widgets. */
	if(events_handled == 0)
	    events_handled += PBtnManage(&ow->ok_btn, event);

        if(events_handled == 0)
            events_handled += PBtnManage(&ow->apply_btn, event);

        if(events_handled == 0)
            events_handled += PBtnManage(&ow->cancel_btn, event);


	/* Widgets on apperance tab. */
        if(events_handled == 0)
            events_handled += TgBtnManage(&ow->label_geometry_tb, event);
        if(events_handled == 0)
            events_handled += TgBtnManage(&ow->show_grid_tb, event);
        if(events_handled == 0)
            events_handled += PromptManage(&ow->grid_spacing_prompt, event);

        /* Widgets on fonts tab. */
        if(events_handled == 0)
            events_handled += PromptManage(&ow->view_font_prompt, event);
        if(events_handled == 0)
            events_handled += PromptManage(&ow->view_object_label_prompt, event);

        /* Widgets on images tab. */
        if(events_handled == 0)
            events_handled += TgBtnManage(&ow->show_preview_image_tb, event);
        if(events_handled == 0)
            events_handled += TgBtnManage(&ow->animate_image_tb, event);

        /* Widgets on paths tab. */
        if(events_handled == 0)
            events_handled += PromptManage(&ow->toplevel_path_prompt, event);
        if(events_handled == 0)
            events_handled += PromptManage(&ow->images_path_prompt, event);
        if(events_handled == 0)
            events_handled += PromptManage(&ow->server_path_prompt, event);

        /* Widgets on keymap tab */
        if(events_handled == 0)
            events_handled += CListManage(&ow->keymaps_list, event);
        if(events_handled == 0)
            events_handled += PBtnManage(&ow->scan_key_btn, event);



	return(events_handled);
}


void OptWGenMap()
{
        optwgen_struct *ow;
            
            
        ow = &optwgen;

	ow->map_state = 0;
	OptWGenDraw(OPTWGEN_DRAW_AMOUNT_COMPLETE);



	return;
}

void OptWGenDoMapValues()
{
        optwgen_struct *ow;
            
            
        ow = &optwgen;


	/* Fetch values. */
	OptWGenFetchValues();

	/* Map. */
	OptWGenMap();


	return;
}

void OptWGenUnmap()
{
        optwgen_struct *ow;
            
            
        ow = &optwgen;

        /* On apperance tab. */
        TgBtnUnmap(&ow->label_geometry_tb);
        TgBtnUnmap(&ow->show_grid_tb);
        PromptUnmap(&ow->grid_spacing_prompt);

        /* On fonts tab. */
        PromptUnmap(&ow->view_font_prompt); 
        PromptUnmap(&ow->view_object_label_prompt);

        /* On images tab. */
        TgBtnUnmap(&ow->show_preview_image_tb);
        TgBtnUnmap(&ow->animate_image_tb);
 
        /* On paths tab. */
        PromptUnmap(&ow->toplevel_path_prompt);
        PromptUnmap(&ow->images_path_prompt);
        PromptUnmap(&ow->server_path_prompt);

        /* On keymaps tab. */
        CListUnmap(&ow->keymaps_list);
        PBtnUnmap(&ow->scan_key_btn);
        OSWUnmapWindow(ow->scan_key_win);
        ow->scanning_key = False;


        OSWUnmapWindow(ow->apperance_tab_win);
        OSWUnmapWindow(ow->fonts_tab_win);
        OSWUnmapWindow(ow->images_tab_win);
        OSWUnmapWindow(ow->paths_tab_win);
        OSWUnmapWindow(ow->keymap_tab_win);

	PBtnUnmap(&ow->ok_btn);
        PBtnUnmap(&ow->apply_btn);
        PBtnUnmap(&ow->cancel_btn);

        OSWUnmapWindow(ow->toplevel);
	ow->map_state = 0;


	OSWDestroyPixmap(&ow->toplevel_buf);


	return;
}

void OptWGenDestroy()
{
        optwgen_struct *ow;


        ow = &optwgen;


	/* On apperance tab. */
	TgBtnDestroy(&ow->label_geometry_tb);
        TgBtnDestroy(&ow->show_grid_tb);
	PromptDestroy(&ow->grid_spacing_prompt);

        /* On fonts tab. */
        PromptDestroy(&ow->view_font_prompt);
        PromptDestroy(&ow->view_object_label_prompt);

        /* On images tab. */
        TgBtnDestroy(&ow->show_preview_image_tb);
        TgBtnDestroy(&ow->animate_image_tb);

	/* On paths tab. */
        PromptDestroy(&ow->toplevel_path_prompt);
	PromptDestroy(&ow->images_path_prompt);
        PromptDestroy(&ow->server_path_prompt);

	/* On keymaps tab. */
	CListDestroy(&ow->keymaps_list);
	PBtnDestroy(&ow->scan_key_btn);
	OSWDestroyWindow(&ow->scan_key_win);
	ow->scanning_key = False;
	OSWDestroyPixel(&ow->scan_key_fg_pix);
        OSWDestroyPixel(&ow->scan_key_bg_pix);

	PBtnDestroy(&ow->ok_btn);
        PBtnDestroy(&ow->apply_btn);
        PBtnDestroy(&ow->cancel_btn);

        OSWDestroyWindow(&ow->apperance_tab_win);
        OSWDestroyWindow(&ow->fonts_tab_win);
        OSWDestroyWindow(&ow->images_tab_win);
        OSWDestroyWindow(&ow->paths_tab_win);
        OSWDestroyWindow(&ow->keymap_tab_win);

	OSWDestroyPixmap(&ow->toplevel_buf);
	OSWDestroyWindow(&ow->toplevel);


	return;
}



