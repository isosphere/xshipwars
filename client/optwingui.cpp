/*
                 Options Window: GUI Event Handling

	Functions:

	int CREATE_PROMPT(
		prompt_window_struct *prompt,
		int x, int y,
		unsigned int width, unsigned int height,
		char *name,
		unsigned int buf_len,
		int (*func_cb)(char *)
	)
	int CREATE_BROWSE_BUTTON(
	        push_button_struct *btn,
	        int x, int y,
	        int (*func_cb)(void *)
	)
	void DRAW_OUTLINE(
	        drawable_t d,
	        int x, int y,
	        unsigned int width,
	        unsigned int height,
	        char *name,
	        unsigned long fg_pix,
	        unsigned long bg_pix,
	        unsigned long text_pix
	)
	void DRAW_LABEL(
                drawable_t d,
                int x, int y,
                char *text,
                unsigned long text_pix
	)

	void OPTWIN_GET_MEM_STATS()

	int OptWinInit()
	int OptWinTabRemap(int tab)
	int OptWinDraw()
	int OptWinManage(event_t *event)
	void OptWinMap()
	void OptWinUnmap()
	void OptWinDestroy()

	---



 */


#include "xsw.h"
#include "optwin.h"
#include "../include/mf.h"


/* Is a within inclusive range of min and max? */
#define INRANGEINC(a,min,max)	((a >= min) && (a <= max))


xsw_options_win_struct options_win;


/* Memory stats for options window. */
struct {

	long	total,		/* Total mem on system. */
		used_prog,	/* Used by just this program. */
		used,		/* Total used. */
		free;		/* Total free (not counting swap). */

} opt_win_mem_stats;



/*
 *	Toggle button array names:
 */

/* Throttle mode. */
char *throttle_mode_names[] = {
        "Normal", 
        "Bi-Directional",
	"Incremental"
};
/* Show viewscreen labels. */
char *show_viewscreen_label_names[] = {
        "None",
        "Object Names",
        "Net Stats",
        "All"
};
/* Show formal name labels. */
char *show_formal_label_names[] = {
	"Never",
	"As needed",
	"Always"
};
/* Sound amount. */
char *sound_amount_names[] = {
        "None",
        "Events",
        "Engines",
        "All"
};
/* Sound servers. */
char *sound_server_names[] = {
        "None",
        "YIFF",
        "EsounD",
        "MikMod"
};



/*
 *	Macro to create a prompt widget on options_win.toplevel.
 */
int CREATE_PROMPT(
	prompt_window_struct *prompt,
	int x, int y,
	unsigned int width, unsigned int height,
	char *name,
	unsigned int buf_len,
	int (*func_cb)(char *)
)
{
	int status;


        status = PromptInit(
            (prompt_window_struct *)prompt,
            options_win.toplevel,
            x, y, 
            width, height,
            PROMPT_STYLE_FLUSHED,
            name,
            buf_len,
	    0,
            func_cb
        );


	return(status);
}


/*
 *	Macro to create a browse button on options_win.toplevel.
 */
int CREATE_BROWSE_BUTTON(
	push_button_struct *btn,
	int x, int y,
	int (*func_cb)(void *)
)
{
        int status;

        status = PBtnInit(
            btn,
            options_win.toplevel,
            x, y,
            0, 30,
	    NULL,
	    PBTN_TALIGN_CENTER,
	    widget_global.browse_files_img,
	    btn,
	    func_cb
        );

        return(status);
}


/*
 *	Draws an outline and label for options_win.toplevel.
 */
void DRAW_OUTLINE(
	drawable_t d,
	int x, int y,
	unsigned int width,
	unsigned int height,
	char *name,
	unsigned long fg_pix,
	unsigned long bg_pix,
	unsigned long text_pix
)
{
	int text_start_pos;	/* In pixels. */
	int text_len;		/* In pixels. (inc margin) */


	/* Error checks. */
	if(!IDC() ||
           (d == 0) ||
	   (name == NULL)
	)
	    return;

	text_start_pos = (int)((double)width * OPTWIN_OUTLINE_NAME_POS);
	text_len = (strlen(name) + 2) * OPTWIN_CHAR_WIDTH;


	/* Draw shadows. */
	OSWSetFgPix(bg_pix);
        OSWDrawLine(d,			/* left */
             x,
             y + (int)height,
             x,
             y
        );
	OSWDrawLine(d,			/* top1 */
	     x,
	     y,
	     x + (int)text_start_pos,
	     y
	);
        OSWDrawLine(d,			/* Top2 */
             x + (int)text_len + (int)text_start_pos,
             y,
             x + (int)width,
             y
        );
        OSWDrawLine(d,			/* right */
	    x + (int)width - 1,
            y + 1,
            x + (int)width - 1,
            y + (int)height - 1
	);
        OSWDrawLine(d,			/* bottom */
            x + 1,
            y + (int)height - 1,
            x + (int)width - 1,
            y + (int)height - 1
        );

        /* Draw highlights. */
        OSWSetFgPix(fg_pix);
        OSWDrawLine(d,			/* left */
             x + 1,
             y + (int)height - 2,
             x + 1,
             y + 1
        );
        OSWDrawLine(d,			/* top1 */
             x + 1,
             y + 1,
             x + (int)text_start_pos,
             y + 1
        );
        OSWDrawLine(d,			/* Top2 */
             x + (int)text_len + (int)text_start_pos,
             y + 1,
             x + (int)width - 2,
             y + 1
        );
        OSWDrawLine(d,			/* right */
            x + (int)width,
            y + 1,
            x + (int)width,
            y + (int)height
        );
        OSWDrawLine(d,			/* bottom */
            x + 1,
            y + (int)height,
            x + (int)width,
            y + (int)height
        );

	/* Draw text. */
	OSWSetFgPix(text_pix);
	OSWDrawString(
	    d,
	    x + (int)text_start_pos + OPTWIN_CHAR_WIDTH,
	    y + 6,
	    name
	);


	return;
}



/*
 *	Draws a label, sets appropriate foreground pixel.
 */
void DRAW_LABEL(
	drawable_t d,
        int x, int y,
        char *text,
	unsigned long text_pix
)
{
        if(!IDC() ||
           (d == 0) ||
           (text == NULL)
        )
            return;

	OSWSetFgPix(text_pix);
	OSWDrawString(d, x, y, text);

	return;
}


/*
 *	Procedure to reget memory stats for options window use.
 */
void OPTWIN_GET_MEM_STATS()
{
	xsw_mem_stat_struct	prog_mem_buf;
	mf_stat_struct		mf_buf;



	/* Get program memory. */
	XSWGetProgMemory(&prog_mem_buf);

	/* Get systems memory free. */
	MemoryFree(&mf_buf);

	/* Put stats into local options window memory stats. */
	opt_win_mem_stats.total = mf_buf.total;
	opt_win_mem_stats.used_prog = prog_mem_buf.total;
	opt_win_mem_stats.used = mf_buf.used;
	opt_win_mem_stats.free = mf_buf.free;


	return;
}



/*
 *	Initializes the options window.
 */
int OptWinInit()
{
	int status;
	char title[256];
	int x, y;
	unsigned int width, height;

	pixmap_t pixmap;
	win_attr_t wattr;


	if(!IDC())
	    return(-1);


	/* Tab images must be allocated. */
	if((widget_global.tab_normal_img == NULL) ||
           (widget_global.tab_selected_img == NULL)
	)
	{
            fprintf(stderr,
                "OptWinInit(): Error: Required images not available.\n"
            );
	    return(-1);
	}


	/* *********************************************************** */
	/* Reset values. */

	x = 0;
	y = 0;
	width = OPTWIN_DEF_WIDTH;
	height = OPTWIN_DEF_HEIGHT;

	options_win.map_state = 0;
	options_win.is_in_focus = 0;
	options_win.x = x;
	options_win.y = y;
	options_win.width = width;
	options_win.height = height;
	options_win.visibility_state = VisibilityFullyObscured;
	options_win.tab = OPTWIN_TAB_CODE_GENERAL;
	options_win.has_modifications = False;


	/* Create toplevel. */
	if(
	    OSWCreateWindow(
	        &options_win.toplevel,
                osw_gui[0].root_win,
                options_win.x, options_win.y,
                options_win.width, options_win.height
            )
	)
	    return(-1);

	options_win.toplevel_buf = 0;

	OSWSetWindowInput(options_win.toplevel, OSW_EVENTMASK_TOPLEVEL);

	WidgetCenterWindow(options_win.toplevel, WidgetCenterWindowToRoot);
        OSWGetWindowAttributes(options_win.toplevel, &wattr);
        options_win.x = wattr.x;
        options_win.y = wattr.y;
        options_win.width = wattr.width;
        options_win.height = wattr.height;

        /* WM properties. */
        sprintf(title, "%s: Options", PROG_NAME);
        if(IMGIsImageNumAllocated(IMG_CODE_OPTIONS_ICON))
        {
            pixmap = OSWCreatePixmapFromImage(
                xsw_image[IMG_CODE_OPTIONS_ICON]->image
            );
        }
        else
        {
            pixmap = widget_global.std_icon_pm;
        }
        OSWSetWindowWMProperties(
            options_win.toplevel,
            title,		/* Title. */
            "Options",		/* Icon title. */
            pixmap,		/* Icon. */
            False,		/* Let WM set coordinates? */
            options_win.x, options_win.y,
	    100, 100,
            osw_gui[0].display_width, osw_gui[0].display_height,
            WindowFrameStyleStandard,
            NULL, 0
        );

	/* Set this window to be a transient for the bridge window. */
	OSWSetTransientFor(options_win.toplevel, bridge_win.toplevel);


	/* *********************************************************** */
	/* Create buttons. */

	x = ((int)options_win.width - (5 * OPTWIN_BTN_WIDTH)) / 6;

        /* OK button. */
	if(
	    PBtnInit(
		&options_win.ok_btn,
		options_win.toplevel,
		(1 * x) + (0 * OPTWIN_BTN_WIDTH),
 		(int)options_win.height - (OPTWIN_BTN_HEIGHT + OPTWIN_MARGIN),
		OPTWIN_BTN_WIDTH, OPTWIN_BTN_HEIGHT,
		"OK",
		PBTN_TALIGN_CENTER,
		NULL,
		(void *)&options_win.ok_btn,
		NULL
	    )
	)
	    return(-1);

	/* Cancel button. */
        if(
            PBtnInit(
                &options_win.cancel_btn,
                options_win.toplevel,
                (2 * x) + (1 * OPTWIN_BTN_WIDTH),
                (int)options_win.height - (OPTWIN_BTN_HEIGHT + OPTWIN_MARGIN),
                OPTWIN_BTN_WIDTH, OPTWIN_BTN_HEIGHT,
		"Cancel",
                PBTN_TALIGN_CENTER,
                NULL,
                (void *)&options_win.cancel_btn,
                NULL
            )
        )
            return(-1);

        /* Apply button. */
        if(
            PBtnInit(
                &options_win.apply_btn,
                options_win.toplevel,
                (3 * x) + (2 * OPTWIN_BTN_WIDTH),
                (int)options_win.height - (OPTWIN_BTN_HEIGHT + OPTWIN_MARGIN),
                OPTWIN_BTN_WIDTH, OPTWIN_BTN_HEIGHT,
		"Apply",
                PBTN_TALIGN_CENTER,
                NULL,
                (void *)&options_win.apply_btn,
                NULL
            )
        )
            return(-1);

        /* Defaults button. */
        if(
            PBtnInit(
                &options_win.defaults_btn,
                options_win.toplevel,
                (4 * x) + (3 * OPTWIN_BTN_WIDTH),
                (int)options_win.height - (OPTWIN_BTN_HEIGHT + OPTWIN_MARGIN),
                OPTWIN_BTN_WIDTH, OPTWIN_BTN_HEIGHT,
		"Defaults",
                PBTN_TALIGN_CENTER,
                NULL,
                (void *)&options_win.defaults_btn,
                NULL
            )
        )
            return(-1);

        /* Save button. */
        if(
            PBtnInit(
                &options_win.save_btn,
                options_win.toplevel,
                (5 * x) + (4 * OPTWIN_BTN_WIDTH),
                (int)options_win.height - (OPTWIN_BTN_HEIGHT + OPTWIN_MARGIN),
                OPTWIN_BTN_WIDTH, OPTWIN_BTN_HEIGHT,
		"Save",
                PBTN_TALIGN_CENTER,
                NULL,
                (void *)&options_win.save_btn,
                NULL
            )
        )
            return(-1);



	/* *************************************************************** */
	/* Tab windows. */

	/* General tab. */
        x = OPTWIN_MARGIN;
	if(
            OSWCreateWindow(
                &options_win.general_tab,
                options_win.toplevel,
	        x,
	        OPTWIN_MARGIN,
	        widget_global.tab_normal_img->width,
	        widget_global.tab_normal_img->height
	    )
        )
            return(-1);
        OSWSetWindowInput(options_win.general_tab, OSW_EVENTMASK_BUTTON);

        /* Graphics tab. */
	x += (10 + (int)widget_global.tab_normal_img->width);
	if(
            OSWCreateWindow(
                &options_win.graphics_tab,
                options_win.toplevel,
	        x,
	        OPTWIN_MARGIN,
                widget_global.tab_normal_img->width,
                widget_global.tab_normal_img->height
	    )
        )
            return(-1);
        OSWSetWindowInput(options_win.graphics_tab, OSW_EVENTMASK_BUTTON);

        /* Sounds tab. */
        x += (10 + (int)widget_global.tab_normal_img->width);
        if(
	    OSWCreateWindow(
                &options_win.sounds_tab,
                options_win.toplevel,
		x,
                OPTWIN_MARGIN,
                widget_global.tab_normal_img->width,
                widget_global.tab_normal_img->height
	    )
        )
            return(-1);
        OSWSetWindowInput(options_win.sounds_tab, OSW_EVENTMASK_BUTTON);

        /* Net tab. */
        x += (10 + (int)widget_global.tab_normal_img->width);
	if(
            OSWCreateWindow(
                &options_win.network_tab,
                options_win.toplevel,
		x,
                OPTWIN_MARGIN,
                widget_global.tab_normal_img->width,
                widget_global.tab_normal_img->height
	    )
        )
            return(-1);
        OSWSetWindowInput(options_win.network_tab, OSW_EVENTMASK_BUTTON);

        /* Misc tab. */
        x += (10 + (int)widget_global.tab_normal_img->width);
        if(
	    OSWCreateWindow(
                &options_win.misc_tab,
                options_win.toplevel,
                x,
                OPTWIN_MARGIN,
                widget_global.tab_normal_img->width,
                widget_global.tab_normal_img->height
	    )
        )
            return(-1);
        OSWSetWindowInput(options_win.misc_tab, OSW_EVENTMASK_BUTTON);


	/* ************************************************************** */
	/* General. */

	if(
	    PUListInit(
                &options_win.control_type_pul,
                options_win.toplevel,
                80, 200,	/* x, y. */
                120, 0,		/* width, height. */
	        3,		/* Items visable. */
	        PULIST_POPUP_CENTER,
		(void *)&options_win.control_type_pul,
		NULL
	    )
	)
            return(-1);
	PUListAddItem(
	    &options_win.control_type_pul,
	    "Keyboard",
	    False
	);
        PUListAddItem(
            &options_win.control_type_pul,
            "Joystick",
#ifdef JS_SUPPORT
	    False
#else
	    True
#endif	/* JS_SUPPORT */
        );
        PUListAddItem(
            &options_win.control_type_pul,
            "Pointer",
	    True		/* Disable item `Pointer'. */
        );

        status = PBtnInit(
            &options_win.keymap_btn,
            options_win.toplevel,
            210, 180,
            110, OPTWIN_BTN_HEIGHT,
            "Map Keyboard",
            PBTN_TALIGN_CENTER,
            NULL,
            &options_win.keymap_btn,
            OptWinKeymapPBCB
        );
        if(status)
            return(-1); 
        PBtnSetHintMessage(
            &options_win.keymap_btn,
            OPTWIN_MAP_KEYBOARD_HINT
        );

#ifdef JS_SUPPORT
        status = PBtnInit(
            &options_win.jsmap_btn,
            options_win.toplevel,
            330, 180,
            110, OPTWIN_BTN_HEIGHT,
            "Map Joystick",
            PBTN_TALIGN_CENTER,
            NULL,
            &options_win.jsmap_btn,
            OptWinJSMapPBCB
        );
        if(status)
            return(-1);
        PBtnSetHintMessage(
            &options_win.jsmap_btn,
            OPTWIN_MAP_JOYSTICK_HINT
        );
#endif	/* JS_SUPPORT */

	if(
            TgBtnArrayInit(
                &options_win.throttle_mode_tba,
                options_win.toplevel,
                170, 234,   /* x, y. */
                3,          /* Number of buttons. */
                0,          /* Default armed button. */
                throttle_mode_names, /* Array of names. */
                3,          /* Number of names. */
                TGBTN_ARRAY_ALIGN_HORIZONTAL
	    )
        )
            return(-1);

	if(
            TgBtnInit(
                &options_win.auto_zoom_tb,
                options_win.toplevel,
                425, 295,
                ((option.auto_zoom) ? True : False),
                "Auto Zoom"
            )
        )
            return(-1);
        TgBtnSetHintMessage(
            &options_win.auto_zoom_tb,
            OPTWIN_AUTO_ZOOM_HINT
        ); 

	if(
            TgBtnInit(
                &options_win.local_updates_tb,
                options_win.toplevel,
                425, 320,
                ((option.local_updates) ? True : False),
                "Local Updates"
	    )
        )
            return(-1);
	TgBtnSetHintMessage(
	    &options_win.local_updates_tb,
	    OPTWIN_LOCAL_UPDATES_HINT
	);

        if(
            TgBtnInit(
                &options_win.notify_scanner_contacts_tb,
                options_win.toplevel,
                425, 345,
                ((option.notify_scanner_contacts) ? True : False),
                "Contacts Notify"
            )
        )
            return(-1);
        TgBtnSetHintMessage(
            &options_win.notify_scanner_contacts_tb,
            OPTWIN_SCANNER_CONTACTS_HINT
        );

	if(
            CREATE_PROMPT(   
                &options_win.isref_name_prompt,
                60, 295,
                280, 30,
                "ISRefs:",
                PATH_MAX + NAME_MAX,
                NULL
	    )
        )
            return(-1);

	if(
            CREATE_PROMPT(
                &options_win.ocs_name_prompt,
                60, 325,
                280, 30,
                "OCS Names:",
                PATH_MAX + NAME_MAX,
                NULL
            )
        )
            return(-1);

	if(
            CREATE_PROMPT(
                &options_win.ss_name_prompt,
                60, 355,
                280, 30,
                "Sound Scheme:",
                PATH_MAX + NAME_MAX,
                NULL
	    )
        )
            return(-1);

	/* Link prompts togeather. */
	options_win.isref_name_prompt.next = &options_win.ocs_name_prompt;
        options_win.isref_name_prompt.prev = &options_win.ss_name_prompt;

        options_win.ocs_name_prompt.next = &options_win.ss_name_prompt;
        options_win.ocs_name_prompt.prev = &options_win.isref_name_prompt;

        options_win.ss_name_prompt.next = &options_win.isref_name_prompt;
        options_win.ss_name_prompt.prev = &options_win.ocs_name_prompt;



	if(
            CREATE_BROWSE_BUTTON(
                &options_win.isref_name_browse_btn,
                340, 295,
                OptWinBrowseISRefsPBCB
	    )
        )
            return(-1);

	if(
            CREATE_BROWSE_BUTTON(
                &options_win.ocs_name_browse_btn,
                340, 325,
                OptWinBrowseOCSNsPBCB
	    )
        )
            return(-1);

	if( 
            CREATE_BROWSE_BUTTON(
                &options_win.ss_name_browse_btn,
                340, 355,
                OptWinBrowseSSPBCB
	    )
        )
            return(-1);


        /* ************************************************************** */
	/* Widgets on Graphics folder. */

	if(
            TgBtnArrayInit(
                &options_win.show_formal_label_tba,
                options_win.toplevel,
                80, 120,    /* x, y. */
                3,          /* Number of buttons. */
                0,          /* Default armed button. */
                show_formal_label_names,    /* Array of names. */
                3,          /* Number of names. */
                TGBTN_ARRAY_ALIGN_HORIZONTAL
	    )
        )
            return(-1);

	if(
            TgBtnInit(
                &options_win.show_viewscreen_marks_tb,
                options_win.toplevel,
                60, 155,
                (option.show_viewscreen_marks) ? True : False,
                "Viewscreen Markings"
	    )
        )
            return(-1);

	if(
            TgBtnArrayInit(
                &options_win.show_viewscreen_labels_tba,
                options_win.toplevel,
                80, 220,    /* x, y. */
                4,          /* Number of buttons. */
                0,          /* Default armed button. */
                show_viewscreen_label_names, /* Array of names. */
                4,          /* Number of names. */
                TGBTN_ARRAY_ALIGN_HORIZONTAL
	    )
        )
            return(-1);

        if(
            TgBtnInit(
                &options_win.async_image_loading_tb,
                options_win.toplevel,
                60, 304,
                ((option.async_image_loading == 1) ? True : False),
                "Passive Image Loading"
            )
        )
            return(-1);
        TgBtnSetHintMessage(
            &options_win.async_image_loading_tb,
            OPTWIN_PASSIVE_IMAGE_LOADING_HINT
        );

        if(
            CREATE_PROMPT(
                &options_win.async_image_pixels_prompt,
                60, 335,
                230, 30,
                "Pixels/Cycle Limit:",
                64,
                NULL
            )
        )
            return(-1);


        /* ************************************************************** */
        /* Widgets on Sounds folder. */

	if(
            TgBtnArrayInit(
                &options_win.sounds_tba,
                options_win.toplevel,
                80, 305,    /* x, y. */
                4,          /* Number of buttons. */
                0,          /* Default armed button. */
                sound_amount_names,  /* Array of names. */
                4,          /* Number of names. */
                TGBTN_ARRAY_ALIGN_HORIZONTAL
	    )
        )
            return(-1);

        if(
            TgBtnInit(
                &options_win.music_tb,
                options_win.toplevel,
                450, 290,
                False,
                "Music"
            )
        )
            return(-1);

	if(
            TgBtnArrayInit(
                &options_win.server_type_tba,
                options_win.toplevel,   
                80, 120,	/* x, y. */
                4,		/* Number of buttons. */   
                0,		/* Default armed button. */
                sound_server_names,	/* Array of names. */
                4,		/* Number of names. */
                TGBTN_ARRAY_ALIGN_HORIZONTAL
	    )
        )
            return(-1);

	options_win.server_type_tba.tb[0]->disabled = False;
#ifdef HAVE_Y2
	options_win.server_type_tba.tb[1]->disabled = False;
#else
	options_win.server_type_tba.tb[1]->disabled = True;
#endif	/* HAVE_Y2 */
#ifdef HAVE_ESD
        options_win.server_type_tba.tb[2]->disabled = False;
#else
	options_win.server_type_tba.tb[2]->disabled = True;
#endif	/* HAVE_ESD */
        options_win.server_type_tba.tb[3]->disabled = True;


	if(
            CREATE_PROMPT(
                &options_win.sound_server_prompt,
                60, 160,
                320, 30,
                "Start Command:",
                PATH_MAX + NAME_MAX,
                NULL
	    )
        )
            return(-1);

	if(
            CREATE_PROMPT(
                &options_win.sound_con_arg_prompt,
                60, 195,
                320, 30,
                "      Address:",
                PATH_MAX + NAME_MAX,
                NULL
	    )
        )
            return(-1);

	if(
            PBtnInit(
                &options_win.sound_test_btn,
                options_win.toplevel,
                450, 90,
                0, OPTWIN_BTN_HEIGHT,
                " Test Sound ",
                PBTN_TALIGN_CENTER,
                NULL,
                &options_win.sound_test_btn,
                OptWinTestSoundPBCB
	    )
        )
            return(-1);

	if(
            TgBtnInit(
                &options_win.flip_stereo_tb,
                options_win.toplevel,
                450, 130,
                False,
                "Flip Stereo"
	    )
        )
            return(-1);

	/* Link prompts togeather. */
	options_win.sound_server_prompt.next = &options_win.sound_con_arg_prompt;
	options_win.sound_server_prompt.prev = &options_win.sound_con_arg_prompt;

        options_win.sound_con_arg_prompt.next = &options_win.sound_server_prompt;
        options_win.sound_con_arg_prompt.prev = &options_win.sound_server_prompt;



        /* ************************************************************** */
        /* Widgets on Net folder. */

	if(
	    CREATE_PROMPT(
                &options_win.max_net_load_prompt,
	        60, 210,
                220, 30,
                "Max Load (bytes/sec):",
                NUM_STR_LEN,
                NULL
	    )
	)
            return(-1);

        if(
	    CREATE_PROMPT(
                &options_win.def_address_prompt,
                60, 90,
                290, 30,
                "Default Address:",
                MAX_URL_LEN,
                NULL
	    )
        )
            return(-1);

        if(
	    CREATE_PROMPT(
                &options_win.def_port_prompt,
                60, 125,
                160, 30,
                "Default Port:",
                NUM_STR_LEN,
                NULL
	    )
        )
            return(-1);

        if(
	    TgBtnInit(
                &options_win.show_net_errors_tb,
                options_win.toplevel,
                370, 90,
                ((option.show_net_errors) ? True : False),
                "Show Network Errors"
            )
        )
            return(-1);
        TgBtnSetHintMessage(
            &options_win.show_net_errors_tb,
            OPTWIN_SHOW_NET_ERRORS_HINT
        ); 

        if(
            TgBtnInit(
                &options_win.show_server_errors_tb,
                options_win.toplevel,
                370, 120,
                ((option.show_server_errors) ? True : False),
                "Show Server Messages"
            )
        )
            return(-1);
        TgBtnSetHintMessage(
            &options_win.show_server_errors_tb,
            OPTWIN_SHOW_SERVER_ERRORS_HINT
        );

        if(
	    CREATE_PROMPT(
                &options_win.net_int_prompt,
                60, 360,
                180, 30,
                "Update Interval:",
                NUM_STR_LEN,
                NULL
            )
	)
            return(-1);

	if(
            TgBtnInit(
                &options_win.auto_interval_tb,
                options_win.toplevel,
                60, 329,
                ((auto_interval_tune.state) ? True : False),
                "Auto Interval Tunning"
            )
        )
            return(-1);
        TgBtnSetHintMessage(
            &options_win.auto_interval_tb,
            OPTWIN_AUTO_INTERVAL_HINT
        ); 

	/* Link prompts togeather. */
        options_win.def_address_prompt.next = &options_win.def_port_prompt;
        options_win.def_address_prompt.prev = &options_win.net_int_prompt;

        options_win.def_port_prompt.next = &options_win.max_net_load_prompt;
        options_win.def_port_prompt.prev = &options_win.def_address_prompt;

        options_win.max_net_load_prompt.next = &options_win.net_int_prompt;
        options_win.max_net_load_prompt.prev = &options_win.def_port_prompt;

        options_win.net_int_prompt.next = &options_win.def_address_prompt;
        options_win.net_int_prompt.prev = &options_win.max_net_load_prompt;



        /* ************************************************************** */
	/* Misc. */
        status = CREATE_PROMPT(
            &options_win.xsw_local_toplevel_path_prompt,
            60, 90, 
            270, 30,
            "Local Toplevel:",
            PATH_MAX,
            NULL
        );
        if(status)
            return(-1);

        status = CREATE_PROMPT(
            &options_win.xsw_toplevel_path_prompt,
            60, 120,
            270, 30,
            "Toplevel:",
            PATH_MAX,
            NULL
        );
        if(status)
            return(-1);

        status = CREATE_PROMPT(
            &options_win.xsw_etc_path_prompt,
            60, 150,
            270, 30,
            "Etc:",
            PATH_MAX,
            NULL
        );
        if(status)
            return(-1);

        status = CREATE_PROMPT(
            &options_win.xsw_images_path_prompt,
            60, 180,
            270, 30,
            "Images:",
            PATH_MAX,
            NULL
        );
        if(status)
            return(-1);

        status = CREATE_PROMPT(
            &options_win.xsw_sounds_path_prompt,
            60, 210,
            270, 30,
            "Sounds:",
            PATH_MAX,
            NULL
        );
        if(status)
            return(-1);

        status = CREATE_PROMPT(
            &options_win.xsw_downloads_path_prompt,
            60, 240,
            270, 30,
            "Downloads:",
            PATH_MAX,
            NULL
        );
        if(status)
            return(-1);

#ifdef JS_SUPPORT
        status = CREATE_PROMPT(
            &options_win.js_calib_path_prompt,
            60, 320,
            260, 30,
            "JS Calib:",
            PATH_MAX + NAME_MAX,
            NULL
        );
        if(status)
            return(-1);

        status = CREATE_BROWSE_BUTTON(
            &options_win.js_calib_path_browse_btn,
            320, 320,
            OptWinBrowseJSCalPBCB
        );
        if(status)
            return(-1);
#endif /* JS_SUPPORT */

       status = PBtnInit(
            &options_win.refresh_memory_btn,
            options_win.toplevel,
            440, 70,
            140, OPTWIN_BTN_HEIGHT,
            "Refresh Memory",
            PBTN_TALIGN_CENTER,
            NULL,
            (void *)&options_win.refresh_memory_btn,
            OptWinRefreshMemoryPBCB
        );
        if(status)
            return(-1);

        if(
            PUListInit(
                &options_win.units_pul,   
                options_win.toplevel,
                425, 242,	/* x, y. */
                170, 0,	/* width, height. */
                3,              /* Items visable. */
                PULIST_POPUP_DOWN,
                (void *)&options_win.units_pul,
                NULL
            )
        )
            return(-1);
        PUListAddItem(
            &options_win.units_pul,
            "AstroMetric",
            False
        );
        PUListAddItem(
            &options_win.units_pul,
            "English",
            False
        );
        PUListAddItem(
            &options_win.units_pul,
            "Program Internal",
            False
        );


	/* Link prompts togeather. */
	options_win.xsw_local_toplevel_path_prompt.next =
	    &options_win.xsw_toplevel_path_prompt;
#ifdef JS_SUPPORT
        options_win.xsw_local_toplevel_path_prompt.prev =
	    &options_win.js_calib_path_prompt;
#else	/* JS_SUPPORT */
        options_win.xsw_local_toplevel_path_prompt.prev =
	    &options_win.xsw_downloads_path_prompt;
#endif	/* JS_SUPPORT */

        options_win.xsw_toplevel_path_prompt.next =
	    &options_win.xsw_etc_path_prompt;
        options_win.xsw_toplevel_path_prompt.prev =
	    &options_win.xsw_local_toplevel_path_prompt;

	options_win.xsw_etc_path_prompt.next =
	    &options_win.xsw_images_path_prompt;
        options_win.xsw_etc_path_prompt.prev =
	    &options_win.xsw_toplevel_path_prompt;

        options_win.xsw_images_path_prompt.next =
	    &options_win.xsw_sounds_path_prompt;
        options_win.xsw_images_path_prompt.prev =
	    &options_win.xsw_etc_path_prompt;

	options_win.xsw_sounds_path_prompt.next =
	    &options_win.xsw_downloads_path_prompt;
	options_win.xsw_sounds_path_prompt.prev =
	    &options_win.xsw_images_path_prompt;

#ifdef JS_SUPPORT
        options_win.xsw_downloads_path_prompt.next =
	    &options_win.js_calib_path_prompt;
#else   /* JS_SUPPORT */
        options_win.xsw_downloads_path_prompt.next =
	    &options_win.xsw_local_toplevel_path_prompt;
#endif  /* JS_SUPPORT */
        options_win.xsw_downloads_path_prompt.prev =
	    &options_win.xsw_sounds_path_prompt;

#ifdef JS_SUPPORT
        options_win.js_calib_path_prompt.next =
	    &options_win.xsw_local_toplevel_path_prompt;
	options_win.js_calib_path_prompt.prev =
	    &options_win.xsw_downloads_path_prompt;
#endif	/* JS_SUPPORT */


	/* ******************************************************* */

	/*   Get global variable values and put them into the
	 *   widget values.
	 */
	OptWinFetchGlobals();


	return(0);
}



/*
 *	Remaps the widgets and windows on the options window
 *	depending on the givin tab and sets the new tab.
 *
 *	This function will also do all the widget redrawing.
 */
int OptWinTabRemap(int tab)
{


	/* Sanitize tab. */
	if((tab != OPTWIN_TAB_CODE_GENERAL) &&
	   (tab != OPTWIN_TAB_CODE_GRAPHICS) &&
           (tab != OPTWIN_TAB_CODE_SOUNDS) &&
           (tab != OPTWIN_TAB_CODE_NET) &&
           (tab != OPTWIN_TAB_CODE_MISC)
	)
	    tab = OPTWIN_TAB_CODE_GENERAL;

	/* Set new tab. */
	options_win.tab = tab;


	/* General. */
	if(options_win.tab == OPTWIN_TAB_CODE_GENERAL)
	{
	    PUListMap(&options_win.control_type_pul);
	    TgBtnArrayMap(&options_win.throttle_mode_tba);
	    PBtnMap(&options_win.keymap_btn);
#ifdef JS_SUPPORT
            PBtnMap(&options_win.jsmap_btn);
#endif	/* JS_SUPPORT */

	    TgBtnMap(&options_win.auto_zoom_tb);
            TgBtnMap(&options_win.local_updates_tb);
	    TgBtnMap(&options_win.notify_scanner_contacts_tb);

            PromptMap(&options_win.isref_name_prompt);
	    PromptMap(&options_win.ocs_name_prompt);
            PromptMap(&options_win.ss_name_prompt);

            PBtnMap(&options_win.isref_name_browse_btn);
            PBtnMap(&options_win.ocs_name_browse_btn);
            PBtnMap(&options_win.ss_name_browse_btn);
	}
	else
	{
            PUListUnmap(&options_win.control_type_pul);
            TgBtnArrayUnmap(&options_win.throttle_mode_tba);
	    PBtnUnmap(&options_win.keymap_btn);
#ifdef JS_SUPPORT
            PBtnUnmap(&options_win.jsmap_btn);
#endif  /* JS_SUPPORT */

            TgBtnUnmap(&options_win.auto_zoom_tb);
            TgBtnUnmap(&options_win.local_updates_tb);
            TgBtnUnmap(&options_win.notify_scanner_contacts_tb);

            PromptUnmap(&options_win.isref_name_prompt);
            PromptUnmap(&options_win.ocs_name_prompt);  
            PromptUnmap(&options_win.ss_name_prompt); 

            PBtnUnmap(&options_win.isref_name_browse_btn);
            PBtnUnmap(&options_win.ocs_name_browse_btn);
            PBtnUnmap(&options_win.ss_name_browse_btn);
	}

        /* Graphics. */
        if(options_win.tab == OPTWIN_TAB_CODE_GRAPHICS)
        {
            TgBtnMap(&options_win.show_viewscreen_marks_tb);
            TgBtnArrayMap(&options_win.show_viewscreen_labels_tba);
	    TgBtnArrayMap(&options_win.show_formal_label_tba);
	    TgBtnMap(&options_win.async_image_loading_tb);
	    PromptMap(&options_win.async_image_pixels_prompt);
	}
	else
	{
            TgBtnUnmap(&options_win.show_viewscreen_marks_tb);
            TgBtnArrayUnmap(&options_win.show_viewscreen_labels_tba);
            TgBtnArrayUnmap(&options_win.show_formal_label_tba);
            TgBtnUnmap(&options_win.async_image_loading_tb);
            PromptUnmap(&options_win.async_image_pixels_prompt);
	}

	/* Sounds. */
        if(options_win.tab == OPTWIN_TAB_CODE_SOUNDS)
        {
            TgBtnArrayMap(&options_win.sounds_tba);
	    TgBtnMap(&options_win.music_tb);
            TgBtnArrayMap(&options_win.server_type_tba);
            PromptMap(&options_win.sound_server_prompt);
            PromptMap(&options_win.sound_con_arg_prompt);
	    PBtnMap(&options_win.sound_test_btn);
	    TgBtnMap(&options_win.flip_stereo_tb);
        }
        else
        {
            TgBtnArrayUnmap(&options_win.sounds_tba);
	    TgBtnUnmap(&options_win.music_tb);
            TgBtnArrayUnmap(&options_win.server_type_tba);
            PromptUnmap(&options_win.sound_server_prompt);
            PromptUnmap(&options_win.sound_con_arg_prompt);
            PBtnUnmap(&options_win.sound_test_btn);
            TgBtnUnmap(&options_win.flip_stereo_tb);
        }

        /* Network. */
        if(options_win.tab == OPTWIN_TAB_CODE_NET)
        {
	    PromptMap(&options_win.def_address_prompt);
            PromptMap(&options_win.def_port_prompt);
	    TgBtnMap(&options_win.show_net_errors_tb);
	    TgBtnMap(&options_win.show_server_errors_tb);
            PromptMap(&options_win.max_net_load_prompt);
            PromptMap(&options_win.net_int_prompt);
            TgBtnMap(&options_win.auto_interval_tb);
        }
        else
        {
            PromptUnmap(&options_win.def_address_prompt); 
            PromptUnmap(&options_win.def_port_prompt);
	    TgBtnUnmap(&options_win.show_net_errors_tb);
            TgBtnUnmap(&options_win.show_server_errors_tb);
            PromptUnmap(&options_win.max_net_load_prompt);
            PromptUnmap(&options_win.net_int_prompt);
            TgBtnUnmap(&options_win.auto_interval_tb);
        }

        /* Misc. */
        if(options_win.tab == OPTWIN_TAB_CODE_MISC)
        {
            PromptMap(&options_win.xsw_local_toplevel_path_prompt);
	    PromptMap(&options_win.xsw_toplevel_path_prompt);
            PromptMap(&options_win.xsw_etc_path_prompt);
            PromptMap(&options_win.xsw_images_path_prompt);
            PromptMap(&options_win.xsw_sounds_path_prompt);
	    PromptMap(&options_win.xsw_downloads_path_prompt);
#ifdef JS_SUPPORT
            PromptMap(&options_win.js_calib_path_prompt);
	    PBtnMap(&options_win.js_calib_path_browse_btn);
#endif /* JS_SUPPORT */
	    PBtnMap(&options_win.refresh_memory_btn);
            PUListMap(&options_win.units_pul);
	}
	else
	{
	    PromptUnmap(&options_win.xsw_local_toplevel_path_prompt);
            PromptUnmap(&options_win.xsw_toplevel_path_prompt);
            PromptUnmap(&options_win.xsw_etc_path_prompt);
            PromptUnmap(&options_win.xsw_images_path_prompt);
            PromptUnmap(&options_win.xsw_sounds_path_prompt);
            PromptUnmap(&options_win.xsw_downloads_path_prompt);
#ifdef JS_SUPPORT
            PromptUnmap(&options_win.js_calib_path_prompt);
            PBtnUnmap(&options_win.js_calib_path_browse_btn);
#endif /* JS_SUPPORT */
	    PBtnUnmap(&options_win.refresh_memory_btn);
            PUListUnmap(&options_win.units_pul);
	}


	return(0);
}



int OptWinDraw()
{
	char *strptr;
	win_attr_t wattr;
	win_attr_t toplevel_wattr;
	char stringa[256];


        if(!IDC())
            return(-1);


	/* Get attributes for toplevel. */
	OSWGetWindowAttributes(options_win.toplevel, &toplevel_wattr);

	/* Map as needed. */
	if(!options_win.map_state)
	{
	    OSWMapRaised(options_win.toplevel);

	    PBtnMap(&options_win.ok_btn);
            PBtnMap(&options_win.cancel_btn);
            PBtnMap(&options_win.apply_btn);
            PBtnMap(&options_win.defaults_btn);
            PBtnMap(&options_win.save_btn);

            OSWMapWindow(options_win.general_tab);
            OSWMapWindow(options_win.graphics_tab);
            OSWMapWindow(options_win.sounds_tab);
            OSWMapWindow(options_win.network_tab);
            OSWMapWindow(options_win.misc_tab);

	    OptWinTabRemap(options_win.tab);

	    options_win.visibility_state = VisibilityUnobscured;
	    options_win.map_state = 1;
	}


	/* ********************************************************* */
	/* Recreate buffers as needed. */

	/* Toplevel. */
	if(options_win.toplevel_buf == 0)
	{
	    OSWGetWindowAttributes(options_win.toplevel, &wattr);
	    if(OSWCreatePixmap(&options_win.toplevel_buf,
		wattr.width, wattr.height
	    ))
		return(-1);
	}
	/* Tab window. */
        if(options_win.tab_win_buf == 0)
        {
            OSWGetWindowAttributes(options_win.general_tab, &wattr);
            if(OSWCreatePixmap(&options_win.tab_win_buf,
                wattr.width, wattr.height
            ))
                return(-1);
        }


        /* ******************************************************** */
	/* Clear toplevel buffer. */
	WidgetPutImageTile(options_win.toplevel_buf,
	    widget_global.std_bkg_img,
	    toplevel_wattr.width, toplevel_wattr.height
	);

	/* Draw frame on toplevel. */
	OSWGetWindowAttributes(options_win.general_tab, &wattr);

	OSWSetFgPix(widget_global.surface_highlight_pix);
	OSWDrawLine(options_win.toplevel_buf,
	    OPTWIN_MARGIN,
	    OPTWIN_MARGIN + (int)wattr.height - 1,
	    (int)toplevel_wattr.width - OPTWIN_MARGIN,
	    OPTWIN_MARGIN + (int)wattr.height - 1
	);
        OSWDrawLine(options_win.toplevel_buf,
            OPTWIN_MARGIN,
	    OPTWIN_MARGIN + (int)wattr.height - 1,
            OPTWIN_MARGIN,
	    (int)toplevel_wattr.height - (OPTWIN_BTN_HEIGHT + OPTWIN_MARGIN +
                OPTWIN_MARGIN)
        );

        OSWSetFgPix(widget_global.surface_shadow_pix);
        OSWDrawLine(options_win.toplevel_buf,
            OPTWIN_MARGIN,
            (int)toplevel_wattr.height - (OPTWIN_BTN_HEIGHT + OPTWIN_MARGIN
                + OPTWIN_MARGIN),
            (int)toplevel_wattr.width - OPTWIN_MARGIN - 1,
            (int)toplevel_wattr.height - (OPTWIN_BTN_HEIGHT + OPTWIN_MARGIN
                + OPTWIN_MARGIN)
        );
        OSWDrawLine(options_win.toplevel_buf,
            (int)toplevel_wattr.width - OPTWIN_MARGIN,
            OPTWIN_MARGIN + (int)wattr.height,
            (int)toplevel_wattr.width - OPTWIN_MARGIN,
            (int)toplevel_wattr.height - (OPTWIN_BTN_HEIGHT +
                OPTWIN_MARGIN + OPTWIN_MARGIN)
        );



	/* *********************************************************** */
	/* Redraw by selected tab. */

        /* General. */
        OSWGetWindowAttributes(options_win.general_tab, &wattr);
        WidgetPutImageTile(
            options_win.tab_win_buf,  
            widget_global.std_bkg_img,
            wattr.width, wattr.height
        );
        if(options_win.tab == OPTWIN_TAB_CODE_GENERAL)
        {
	    WidgetPutImageNormal(
		options_win.tab_win_buf,
		widget_global.tab_selected_img,
		0, 0,
		True
	    );

	    DRAW_OUTLINE(
		options_win.toplevel_buf,
		40, 70,
		560, 70,
		"System Information",
		widget_global.surface_highlight_pix,
		widget_global.surface_shadow_pix,
		widget_global.normal_text_pix
	    );

            DRAW_OUTLINE(
                options_win.toplevel_buf,
                40, 160,
                560, 102,
                "Control Device",
                widget_global.surface_highlight_pix,
                widget_global.surface_shadow_pix,
                widget_global.normal_text_pix
            );

            DRAW_OUTLINE(
                options_win.toplevel_buf,
                40, 280,
                355, 125,
                "Referance Files", 
                widget_global.surface_highlight_pix,
                widget_global.surface_shadow_pix,
                widget_global.normal_text_pix
            );

            DRAW_OUTLINE(
                options_win.toplevel_buf,
                410, 280,
                190, 125,
                "Assistance",
                widget_global.surface_highlight_pix,
                widget_global.surface_shadow_pix,
                widget_global.normal_text_pix
            );

            DRAW_LABEL(
                options_win.toplevel_buf,
                60, 190,
                "Controller Type:",
                widget_global.normal_text_pix
            );

	    DRAW_LABEL(
		options_win.toplevel_buf,
		66, 248,
		"Throttle Mode:",
		widget_global.normal_text_pix
	    );

	    /* Memory stats (memory stats are fetched when mapped. */
	    sprintf(stringa,
 "Memory: Program Used: %ld kb   System Total: %ld kb",
		opt_win_mem_stats.used_prog / 1000,
		opt_win_mem_stats.total / 1000
	    );
            DRAW_LABEL(
                options_win.toplevel_buf,
                60, 100,
                stringa,
                widget_global.normal_text_pix
            );
            sprintf(stringa,
		"Program Version: %s",
                PROG_VERSION
            );
            DRAW_LABEL(
                options_win.toplevel_buf,
                60, 122,
                stringa,
                widget_global.normal_text_pix
            );


	    /* Conf icon. */
/*
	    if(IMGIsImageNumAllocated(IMG_CODE_CONFIGURE_ICON))
	    {
	        WidgetPutImageNormal(
                    options_win.toplevel_buf,
                    xsw_image[IMG_CODE_CONFIGURE_ICON]->image,
                    530, 80,
                    True
                );
	    }
*/
	}
	else
        {
            WidgetPutImageNormal(
                options_win.tab_win_buf,
                widget_global.tab_normal_img,
                0, 0,
                True 
            );
        }
	OSWSetFgPix(widget_global.normal_text_pix);
	strptr = "General";
	OSWDrawString(options_win.tab_win_buf,
            ((int)wattr.width / 2) - ((strlen(strptr) * 7) / 2),
	    (14 / 2) + 14,
	    strptr
        );
	OSWPutBufferToWindow(options_win.general_tab, options_win.tab_win_buf);


        /* Graphics. */
        OSWGetWindowAttributes(options_win.graphics_tab, &wattr);
        WidgetPutImageTile(
            options_win.tab_win_buf,
            widget_global.std_bkg_img,
            wattr.width, wattr.height
        );
        if(options_win.tab == OPTWIN_TAB_CODE_GRAPHICS)
        { 
            WidgetPutImageNormal(
                options_win.tab_win_buf,
                widget_global.tab_selected_img,
                0, 0,
                True
            );

            DRAW_OUTLINE(
                options_win.toplevel_buf,
                40, 70,
                560, 190,
                "Details",
                widget_global.surface_highlight_pix,
                widget_global.surface_shadow_pix,
                widget_global.normal_text_pix
            );

            DRAW_LABEL(
                options_win.toplevel_buf,
                60, 110,
                "Formal object names:",
                widget_global.normal_text_pix
            );

            DRAW_LABEL(
                options_win.toplevel_buf,
                60, 210,
                "Viewscreen Labeling:",
                widget_global.normal_text_pix
            );

            DRAW_OUTLINE(
                options_win.toplevel_buf,
                40, 280,
                560, 120,
                "Images",
                widget_global.surface_highlight_pix,
                widget_global.surface_shadow_pix,
                widget_global.normal_text_pix
            );
        } 
        else
        {
            WidgetPutImageNormal(
                options_win.tab_win_buf,
                widget_global.tab_normal_img,
                0, 0,
                True
            );
        }
        OSWSetFgPix(widget_global.normal_text_pix);
        strptr = "Graphics";
        OSWDrawString(options_win.tab_win_buf,
            ((int)wattr.width / 2) - ((strlen(strptr) * 7) / 2),
            (14 / 2) + 14,
            strptr
        );
        OSWPutBufferToWindow(options_win.graphics_tab, options_win.tab_win_buf);


        /* Sounds. */
        OSWGetWindowAttributes(options_win.sounds_tab, &wattr);
        WidgetPutImageTile(
            options_win.tab_win_buf,
            widget_global.std_bkg_img,
            wattr.width, wattr.height
        );
        if(options_win.tab == OPTWIN_TAB_CODE_SOUNDS)
        {
            WidgetPutImageNormal(
                options_win.tab_win_buf,
                widget_global.tab_selected_img,
                0, 0,
                True
            );

            DRAW_OUTLINE(
                options_win.toplevel_buf,
                40, 70,
                560, 175,
                "Sound Server",
                widget_global.surface_highlight_pix,
                widget_global.surface_shadow_pix,
                widget_global.normal_text_pix
            );

            DRAW_OUTLINE(
                options_win.toplevel_buf,
                40, 265,
                560, 140,
                "Sound Options", 
                widget_global.surface_highlight_pix,
                widget_global.surface_shadow_pix,
                widget_global.normal_text_pix
            );

            DRAW_LABEL(
                options_win.toplevel_buf,
                60, 110,
                "Sound Server Type:",
                widget_global.normal_text_pix
            );

            DRAW_LABEL(
                options_win.toplevel_buf,
                60, 296,
                "Amount level:",
                widget_global.normal_text_pix
            );
        }
        else
        {   
            WidgetPutImageNormal(
                options_win.tab_win_buf,
                widget_global.tab_normal_img,
                0, 0,
                True
            );
        }
        OSWSetFgPix(widget_global.normal_text_pix);
        strptr = "Sounds";
        OSWDrawString(options_win.tab_win_buf,
            ((int)wattr.width / 2) - ((strlen(strptr) * 7) / 2),
            (14 / 2) + 14,
            strptr
        );
        OSWPutBufferToWindow(options_win.sounds_tab, options_win.tab_win_buf);


        /* Network. */
        OSWGetWindowAttributes(options_win.network_tab, &wattr);
        WidgetPutImageTile(
            options_win.tab_win_buf,
            widget_global.std_bkg_img,   
            wattr.width, wattr.height
        );
        if(options_win.tab == OPTWIN_TAB_CODE_NET)
        {   
            WidgetPutImageNormal(
                options_win.tab_win_buf,
                widget_global.tab_selected_img,
                0, 0,
                True 
            );

            DRAW_OUTLINE(
                options_win.toplevel_buf,
                40, 70,
                560, 100,
                "General",
                widget_global.surface_highlight_pix,
                widget_global.surface_shadow_pix,
                widget_global.normal_text_pix
            );

            DRAW_OUTLINE(
                options_win.toplevel_buf,
                40, 190,
                560, 100,
                "Bandwidth",
                widget_global.surface_highlight_pix,
                widget_global.surface_shadow_pix,
                widget_global.normal_text_pix
            );

            DRAW_OUTLINE(
                options_win.toplevel_buf,
                40, 310,
                560, 100,
                "Optimization", 
                widget_global.surface_highlight_pix,
                widget_global.surface_shadow_pix,
                widget_global.normal_text_pix
            );

        }     
        else
        {
            WidgetPutImageNormal(
                options_win.tab_win_buf,
                widget_global.tab_normal_img,
                0, 0,
                True
            );
        }
        OSWSetFgPix(widget_global.normal_text_pix);
        strptr = "Network";
        OSWDrawString(options_win.tab_win_buf,
            ((int)wattr.width / 2) - ((strlen(strptr) * 7) / 2),
            (14 / 2) + 14,
            strptr
        );
        OSWPutBufferToWindow(options_win.network_tab, options_win.tab_win_buf);


        /* Misc. */
        OSWGetWindowAttributes(options_win.misc_tab, &wattr);
        WidgetPutImageTile(
            options_win.tab_win_buf,
            widget_global.std_bkg_img,
            wattr.width, wattr.height
        );
        if(options_win.tab == OPTWIN_TAB_CODE_MISC)
        {   
            WidgetPutImageNormal(
                options_win.tab_win_buf,
                widget_global.tab_selected_img,
                0, 0,
                True
            );

            DRAW_OUTLINE(
                options_win.toplevel_buf,
                40, 70,
                335, 210,
                "Program Directories",
                widget_global.surface_highlight_pix,
                widget_global.surface_shadow_pix,
                widget_global.normal_text_pix
            );

            DRAW_OUTLINE(
                options_win.toplevel_buf,
                40, 300,
                335, 100,
                "Other Files", 
                widget_global.surface_highlight_pix,
                widget_global.surface_shadow_pix,
                widget_global.normal_text_pix
            );

            DRAW_OUTLINE(
                options_win.toplevel_buf,
                390, 200,
                210, 200,
                "Dimensions", 
                widget_global.surface_highlight_pix,
                widget_global.surface_shadow_pix,
                widget_global.normal_text_pix
            );

            DRAW_LABEL(
                options_win.toplevel_buf,
                410, 233,
                "Units:",
                widget_global.normal_text_pix
            );
        }     
        else
        {
            WidgetPutImageNormal(
                options_win.tab_win_buf,
                widget_global.tab_normal_img,
                0, 0,
                True
            );
        }
        OSWSetFgPix(widget_global.normal_text_pix);
        strptr = "Misc";
        OSWDrawString(options_win.tab_win_buf,
            ((int)wattr.width / 2) - ((strlen(strptr) * 7) / 2),
            (14 / 2) + 14,
            strptr
        );
        OSWPutBufferToWindow(options_win.misc_tab, options_win.tab_win_buf);


	/* Put buffer to toplevel. */
	OSWPutBufferToWindow(
	    options_win.toplevel,
	    options_win.toplevel_buf
	);


	return(0);
}



int OptWinManage(event_t *event)
{
	int events_handled = 0;


	if(event == NULL)
	    return(events_handled);


	if(!options_win.map_state &&
           (event->type != MapNotify)
	)
	    return(events_handled);


	switch(event->type)
	{
	  /* ********************************************************* */
	  case KeyPress:
	    /* Skip if not in focus. */
	    if(options_win.is_in_focus == 0)
		return(events_handled);

	    /* Enter key. */
	    if((event->xkey.keycode == osw_keycode.enter) ||
               (event->xkey.keycode == osw_keycode.np_enter)
	    )
	    {
                /* Apply changes. */
                OptWinApplyChanges();
            
                /* Unmap options window. */
                OptWinUnmap();
            
                events_handled++;
                return(events_handled);

		events_handled++;
	    }
	    /* Escape key. */
	    else if(event->xkey.keycode == osw_keycode.esc)
	    {
                if(options_win.has_modifications)
                {
/*
                    fprintf(stderr,
                        "Warning: Disgarding options window changes.\n"
                    );
*/
                }
          
                /* Unmap options window. */
                OptWinUnmap();

                events_handled++;
                return(events_handled);
	    }
	    break;

          /* ********************************************************* */
          case KeyRelease:
            /* Skip if not in focus. */
            if(options_win.is_in_focus == 0)
                return(events_handled);

	    break;

          /* ********************************************************* */
          case ButtonPress:
	    /* Tabs. */
	    if(event->xany.window == options_win.general_tab)
	    {
		options_win.tab = OPTWIN_TAB_CODE_GENERAL;
		events_handled++;
	    }
            else if(event->xany.window == options_win.graphics_tab)
            {
                options_win.tab = OPTWIN_TAB_CODE_GRAPHICS;
                events_handled++;
            }
            else if(event->xany.window == options_win.sounds_tab)
            {
                options_win.tab = OPTWIN_TAB_CODE_SOUNDS;
                events_handled++;
            }
            else if(event->xany.window == options_win.network_tab)            
            {
                options_win.tab = OPTWIN_TAB_CODE_NET;
                events_handled++;
            }
            else if(event->xany.window == options_win.misc_tab)            
            {
                options_win.tab = OPTWIN_TAB_CODE_MISC;
                events_handled++;
            }

	    if(events_handled > 0)
	    {
		/* Handle remapping. */
		OptWinTabRemap(options_win.tab);
		OptWinDraw();
	    }
	    break;

          /* ********************************************************* */
          case ButtonRelease:
            break;

	  /* ********************************************************* */
	  case Expose:
	    if(event->xany.window == options_win.toplevel)
	    {
		OptWinDraw();
		events_handled++;
		return(events_handled);
	    }
	    break;

          /* ********************************************************* */
          case UnmapNotify:
            if(event->xany.window == options_win.toplevel)
            {
                OptWinUnmap();

                events_handled++;
                return(events_handled);
            }
            break;

          /* ********************************************************* */
          case MapNotify:
            if(event->xany.window == options_win.toplevel)
            {
		if(!options_win.map_state)
                    OptWinMap();

                events_handled++;
                return(events_handled);
            }
            break;

	  /* ******************************************************** */
	  case VisibilityNotify:
            if(event->xany.window == options_win.toplevel)
	    {
		options_win.visibility_state = event->xvisibility.state;

                events_handled++;
                return(events_handled);
	    }
	    break;

          /* ******************************************************** */
          case FocusIn:
            if(event->xany.window == options_win.toplevel)
            {
		options_win.is_in_focus = 1;          

                events_handled++; 
            }
            break;

          /* ******************************************************** */
          case FocusOut:
            if(event->xany.window == options_win.toplevel)
            {
                options_win.is_in_focus = 0;

                events_handled++;
            }
            break;

	  /* ******************************************************* */
	  case ClientMessage:
	    if(OSWIsEventDestroyWindow(options_win.toplevel, event))
	    {
                if(options_win.has_modifications)
                {
/*
                    fprintf(stderr,
                        "Warning: Disgarding options window changes.\n"
                    );
*/
                }

		/* Unmap options window. */
		OptWinUnmap();

                events_handled++;
                return(events_handled);
	    }
	    break;
	}


	/* ************************************************************ */
	/* Manage widgets. */

	/* OK button. */
        if(events_handled == 0)
        {
	    events_handled += PBtnManage(&options_win.ok_btn, event);
	    if((events_handled > 0) &&
               ((event->type == ButtonRelease) ||
                (event->type == KeyRelease)
               )
	    )
	    {
                /* Apply changes. */
                OptWinApplyChanges();
                      
                /* Unmap options window. */
                OptWinUnmap();
                
                return(events_handled);
            }
	}
	/* Cancel button. */
        if(events_handled == 0)
        {   
            events_handled += PBtnManage(&options_win.cancel_btn, event);
            if((events_handled > 0) &&
               ((event->type == ButtonRelease) ||
                (event->type == KeyRelease)
               )
            )
            {   
                if(options_win.has_modifications)
                {
/*
                    fprintf(stderr,
                        "Warning: Disgarding options window changes.\n"
                    );
*/
                }
                
                /* Unmap options window. */
                OptWinUnmap();
                
                return(events_handled);
            }
	}
        /* Apply button. */
        if(events_handled == 0)
        {
            events_handled += PBtnManage(&options_win.apply_btn, event);
            if((events_handled > 0) &&
               ((event->type == ButtonRelease) ||
                (event->type == KeyRelease)
               )
            )
            {
                /* Apply changes. */
                OptWinApplyChanges();
                      
                return(events_handled);
            }
	}
        /* Defaults button. */
        if(events_handled == 0)
        {   
            events_handled += PBtnManage(&options_win.defaults_btn, event);
            if((events_handled > 0) &&
               ((event->type == ButtonRelease) ||
                (event->type == KeyRelease)
               )
            )
            {
                /* Load defaults. */
                OptWinLoadDefaults();

                return(events_handled);
            }   
        }
        /* Defaults button. */
        if(events_handled == 0)
        {
            events_handled += PBtnManage(&options_win.save_btn, event);
            if((events_handled > 0) &&
               ((event->type == ButtonRelease) ||
                (event->type == KeyRelease)
               )
            )
            {
                /* Apply and save changes. */
                OptWinSaveChanges();
            
                /* Unmap options window. */
                OptWinUnmap();

                return(events_handled);             
            }
	}


	/* Prompts, tba's, etc. */
	if(events_handled == 0)
	{
            /* Manage widgets. */
	    switch(options_win.tab)
	    {
	      case OPTWIN_TAB_CODE_GENERAL:
		if(events_handled == 0)
                    events_handled += PUListManage(
                        &options_win.control_type_pul, event);
                if(events_handled == 0)
                    events_handled += TgBtnArrayManage(
                        &options_win.throttle_mode_tba, event);
                if(events_handled == 0)
                    events_handled += PBtnManage(
                        &options_win.keymap_btn, event);
#ifdef JS_SUPPORT
                if(events_handled == 0)
                    events_handled += PBtnManage(
                        &options_win.jsmap_btn, event);
#endif  /* JS_SUPPORT */
                if(events_handled == 0)
                    events_handled += TgBtnManage( 
                        &options_win.auto_zoom_tb, event);
                if(events_handled == 0)
                    events_handled += TgBtnManage(
                        &options_win.local_updates_tb, event);
                if(events_handled == 0)
		    events_handled += TgBtnManage(
                        &options_win.notify_scanner_contacts_tb, event);

                if(events_handled == 0)
		    events_handled += PromptManage(
                        &options_win.isref_name_prompt, event);
                if(events_handled == 0)
                    events_handled += PromptManage(
                        &options_win.ocs_name_prompt, event);
                if(events_handled == 0)
                    events_handled += PromptManage(
                        &options_win.ss_name_prompt, event);

                if(events_handled == 0)
		    events_handled += PBtnManage(
		        &options_win.isref_name_browse_btn, event);
                if(events_handled == 0)
                    events_handled += PBtnManage(
                        &options_win.ocs_name_browse_btn, event);
                if(events_handled == 0)
                    events_handled += PBtnManage(
                        &options_win.ss_name_browse_btn, event);
		break;

	      case OPTWIN_TAB_CODE_GRAPHICS:
                if(events_handled == 0)
                    events_handled += TgBtnManage(
                        &options_win.show_viewscreen_marks_tb, event);
                if(events_handled == 0)
                    events_handled += TgBtnArrayManage(
                        &options_win.show_viewscreen_labels_tba, event);
                if(events_handled == 0)
                    events_handled += TgBtnArrayManage(
                        &options_win.show_formal_label_tba, event);
                if(events_handled == 0)
                    events_handled += TgBtnManage(
                        &options_win.async_image_loading_tb, event);
                if(events_handled == 0)
                    events_handled += PromptManage(
                        &options_win.async_image_pixels_prompt, event);
		break;

	      case OPTWIN_TAB_CODE_SOUNDS:
                if(events_handled == 0)
                    events_handled += TgBtnArrayManage(
		        &options_win.sounds_tba, event);
		if(events_handled == 0)
                    events_handled += TgBtnManage(
			&options_win.music_tb, event);
                if(events_handled == 0)
                    events_handled += TgBtnArrayManage(
                        &options_win.server_type_tba, event);
                if(events_handled == 0)
                    events_handled += PromptManage(
                        &options_win.sound_server_prompt, event);
                if(events_handled == 0)
                    events_handled += PromptManage(
                        &options_win.sound_con_arg_prompt, event);
                if(events_handled == 0)
                    events_handled += PBtnManage(
		        &options_win.sound_test_btn, event);
                if(events_handled == 0)
		    events_handled += TgBtnManage(
		        &options_win.flip_stereo_tb, event);
		break;

	      case OPTWIN_TAB_CODE_NET:
                if(events_handled == 0)
                    events_handled += PromptManage(
                        &options_win.def_address_prompt, event);
                if(events_handled == 0)
                    events_handled += PromptManage(
                        &options_win.def_port_prompt, event);
                if(events_handled == 0)
		    events_handled += TgBtnManage(
		        &options_win.show_net_errors_tb, event);
                if(events_handled == 0)
                    events_handled += TgBtnManage(
                        &options_win.show_server_errors_tb, event);
                if(events_handled == 0)
                    events_handled += PromptManage(
		        &options_win.max_net_load_prompt, event);
                if(events_handled == 0)
                    events_handled += PromptManage(
                        &options_win.net_int_prompt, event);
                if(events_handled == 0)
                    events_handled += TgBtnManage(
                        &options_win.auto_interval_tb, event);
		break;

	      case OPTWIN_TAB_CODE_MISC:
                if(events_handled == 0)
                    events_handled += PromptManage(
                        &options_win.xsw_local_toplevel_path_prompt, event);
                if(events_handled == 0)
                    events_handled += PromptManage(
                        &options_win.xsw_toplevel_path_prompt, event);
                if(events_handled == 0)
                    events_handled += PromptManage(
                        &options_win.xsw_etc_path_prompt, event);
                if(events_handled == 0)
                    events_handled += PromptManage(
                        &options_win.xsw_images_path_prompt, event);
                if(events_handled == 0)
                    events_handled += PromptManage(
                        &options_win.xsw_sounds_path_prompt, event);
                if(events_handled == 0)
                    events_handled += PromptManage(
                        &options_win.xsw_downloads_path_prompt, event);
#ifdef JS_SUPPORT
                if(events_handled == 0)
                    events_handled += PromptManage(
                        &options_win.js_calib_path_prompt, event);
                if(events_handled == 0)
		    events_handled += PBtnManage(
		        &options_win.js_calib_path_browse_btn, event);
#endif /* JS_SUPPORT */
                if(events_handled == 0)
                    events_handled += PBtnManage(
			&options_win.refresh_memory_btn, event);
                if(events_handled == 0)
                    events_handled += PUListManage(
                        &options_win.units_pul, event);
		break;
	    }


	    /* Check if a widget event was handled. */
	    if((events_handled > 0) &&
	       ((event->type == KeyPress) ||
                (event->type == KeyRelease) ||
                (event->type == ButtonPress)
	       )
	    )
                options_win.has_modifications = True;
	}


	return(events_handled);
}



/*
 *	Maps options window, recreates its large buffers as needed.
 */
void OptWinMap()
{
	/* Unfocus all windows. */
	XSWDoUnfocusAllWindows();


	/* Map it by setting it's state to 0 and drawing. */
	options_win.map_state = 0;
	OptWinDraw();

	OptWinTabRemap(options_win.tab);


	/* Set up values. */
        options_win.is_in_focus = 1;
	options_win.visibility_state = VisibilityUnobscured;
	options_win.has_modifications = False;


	return;
}


/*
 *	Fetches global setting values and maps.
 */
void OptWinDoMapValues()
{
        OptWinFetchGlobals();
        OPTWIN_GET_MEM_STATS();

	OptWinMap();

	return;
}


/*
 *	Unmaps options window and deallocating its big buffers.
 */
void OptWinUnmap()
{
        /* General. */
        PUListUnmap(&options_win.control_type_pul);
        TgBtnArrayUnmap(&options_win.throttle_mode_tba);
        PBtnUnmap(&options_win.keymap_btn);
#ifdef JS_SUPPORT
	PBtnUnmap(&options_win.jsmap_btn);
#endif  /* JS_SUPPORT */

        TgBtnUnmap(&options_win.auto_zoom_tb);
        TgBtnUnmap(&options_win.local_updates_tb);
	TgBtnUnmap(&options_win.notify_scanner_contacts_tb);

        PromptUnmap(&options_win.isref_name_prompt);
        PromptUnmap(&options_win.ocs_name_prompt);
        PromptUnmap(&options_win.ss_name_prompt);

        PBtnUnmap(&options_win.isref_name_browse_btn);
        PBtnUnmap(&options_win.ocs_name_browse_btn);
        PBtnUnmap(&options_win.ss_name_browse_btn);

        /* Graphics. */
        TgBtnUnmap(&options_win.show_viewscreen_marks_tb);
        TgBtnArrayUnmap(&options_win.show_viewscreen_labels_tba);
        TgBtnArrayUnmap(&options_win.show_formal_label_tba);
        TgBtnUnmap(&options_win.async_image_loading_tb);
        PromptUnmap(&options_win.async_image_pixels_prompt);

        /* Sounds. */
        TgBtnArrayUnmap(&options_win.sounds_tba);
	TgBtnUnmap(&options_win.music_tb);
        TgBtnArrayUnmap(&options_win.server_type_tba);   
        PromptUnmap(&options_win.sound_server_prompt);   
        PromptUnmap(&options_win.sound_con_arg_prompt);
        PBtnUnmap(&options_win.sound_test_btn);
        TgBtnUnmap(&options_win.flip_stereo_tb);

        /* Network. */
        PromptUnmap(&options_win.def_address_prompt);
        PromptUnmap(&options_win.def_port_prompt);
        TgBtnUnmap(&options_win.show_net_errors_tb);
        TgBtnUnmap(&options_win.show_server_errors_tb);
        PromptUnmap(&options_win.max_net_load_prompt);
        PromptUnmap(&options_win.net_int_prompt);
        TgBtnUnmap(&options_win.auto_interval_tb);

        /* Misc. */
	PromptUnmap(&options_win.xsw_local_toplevel_path_prompt);
        PromptUnmap(&options_win.xsw_toplevel_path_prompt);
        PromptUnmap(&options_win.xsw_etc_path_prompt);
        PromptUnmap(&options_win.xsw_images_path_prompt);
        PromptUnmap(&options_win.xsw_sounds_path_prompt);
	PromptUnmap(&options_win.xsw_downloads_path_prompt);
#ifdef JS_SUPPORT
        PromptUnmap(&options_win.js_calib_path_prompt);
        PBtnUnmap(&options_win.js_calib_path_browse_btn);
#endif /* JS_SUPPORT */
        PBtnUnmap(&options_win.refresh_memory_btn);
        PUListUnmap(&options_win.units_pul);



	/* Buttons. */
	PBtnUnmap(&options_win.ok_btn);
        PBtnUnmap(&options_win.cancel_btn);
        PBtnUnmap(&options_win.apply_btn);
        PBtnUnmap(&options_win.defaults_btn);
        PBtnUnmap(&options_win.save_btn);
	OSWUnmapWindow(options_win.toplevel);

	options_win.map_state = 0;
        options_win.visibility_state = VisibilityFullyObscured; 
        options_win.is_in_focus = 0;


	/* Set has_modifications to False. */
	options_win.has_modifications = False;

	/* Destroy big buffers. */
	OSWDestroyPixmap(&options_win.toplevel_buf);

	return;
}



void OptWinDestroy()
{
	if(IDC())
	{
	    /* General. */
	    PUListDestroy(&options_win.control_type_pul);
            TgBtnArrayDestroy(&options_win.throttle_mode_tba);
	    PBtnDestroy(&options_win.keymap_btn);
#ifdef JS_SUPPORT
            PBtnDestroy(&options_win.jsmap_btn);
#endif	/* JS_SUPPORT */

	    TgBtnDestroy(&options_win.auto_zoom_tb);
            TgBtnDestroy(&options_win.local_updates_tb);
            TgBtnDestroy(&options_win.notify_scanner_contacts_tb);

            PromptDestroy(&options_win.isref_name_prompt);
	    PromptDestroy(&options_win.ocs_name_prompt);
            PromptDestroy(&options_win.ss_name_prompt);   

	    PBtnDestroy(&options_win.isref_name_browse_btn);
            PBtnDestroy(&options_win.ocs_name_browse_btn);
            PBtnDestroy(&options_win.ss_name_browse_btn);

	    /* Graphics. */
            TgBtnDestroy(&options_win.show_viewscreen_marks_tb);
            TgBtnArrayDestroy(&options_win.show_viewscreen_labels_tba);
	    TgBtnArrayDestroy(&options_win.show_formal_label_tba);
	    TgBtnDestroy(&options_win.async_image_loading_tb);
            PromptDestroy(&options_win.async_image_pixels_prompt);

	    /* Sounds. */
            TgBtnArrayDestroy(&options_win.sounds_tba);
            TgBtnDestroy(&options_win.music_tb);
            TgBtnArrayDestroy(&options_win.server_type_tba);
            PromptDestroy(&options_win.sound_server_prompt);  
            PromptDestroy(&options_win.sound_con_arg_prompt);
            PBtnDestroy(&options_win.sound_test_btn);
	    TgBtnDestroy(&options_win.flip_stereo_tb);

	    /* Network. */
            PromptDestroy(&options_win.def_address_prompt);
            PromptDestroy(&options_win.def_port_prompt);
	    TgBtnDestroy(&options_win.show_net_errors_tb);
            TgBtnDestroy(&options_win.show_server_errors_tb);
            PromptDestroy(&options_win.max_net_load_prompt);
            PromptDestroy(&options_win.net_int_prompt);
            TgBtnDestroy(&options_win.auto_interval_tb);

	    /* Misc. */
	    PromptDestroy(&options_win.xsw_local_toplevel_path_prompt);
            PromptDestroy(&options_win.xsw_toplevel_path_prompt);
            PromptDestroy(&options_win.xsw_etc_path_prompt);
            PromptDestroy(&options_win.xsw_images_path_prompt);
            PromptDestroy(&options_win.xsw_sounds_path_prompt);
            PromptDestroy(&options_win.xsw_downloads_path_prompt);
#ifdef JS_SUPPORT
            PromptDestroy(&options_win.js_calib_path_prompt);
	    PBtnDestroy(&options_win.js_calib_path_browse_btn);
#endif /* JS_SUPPORT */
	    PBtnDestroy(&options_win.refresh_memory_btn);
            PUListDestroy(&options_win.units_pul);


	    /* Buttons. */
	    PBtnDestroy(&options_win.ok_btn);
            PBtnDestroy(&options_win.cancel_btn);
            PBtnDestroy(&options_win.apply_btn);
            PBtnDestroy(&options_win.defaults_btn);
            PBtnDestroy(&options_win.save_btn);

	    /* Tabs. */
            OSWDestroyWindow(&options_win.general_tab);
            OSWDestroyWindow(&options_win.graphics_tab);
            OSWDestroyWindow(&options_win.sounds_tab);
            OSWDestroyWindow(&options_win.network_tab);
            OSWDestroyWindow(&options_win.misc_tab);

	    OSWDestroyPixmap(&options_win.tab_win_buf);

            /* Toplevel. */
            OSWDestroyWindow(&options_win.toplevel); 
	    OSWDestroyPixmap(&options_win.toplevel_buf);
	}


        options_win.map_state = 0;
        options_win.x = 0;
        options_win.y = 0;
        options_win.width = 0;
        options_win.height = 0;
        options_win.visibility_state = VisibilityFullyObscured;
        options_win.tab = OPTWIN_TAB_CODE_GENERAL;
        options_win.is_in_focus = 0;
        options_win.has_modifications = False;


	return;
}
