/*
                    Bridge Window GUI Handling

 	Functions:

	void BW_SET_BTNPOS(
		int btnpos_num,
		int x, int y,
		unsigned int width, unsigned int height
	)

	int BridgeWinInit(int argc, char *argv[])
	void BridgeWinResize(void)
	void BridgeWinResizePreset(int step)
        void BridgeWinMap(void)
        void BridgeWinUnmap(void)
        void BridgeWinDestroy(void)

	---

	Notes:

	bridgedraw.c contains the GUI drawing functions for
	the bridge window.

	bridgemanage.c contains the GUI event management function for
	the bridge window.

 */

#include "xsw.h"  
#include "net.h"


#define RADIANS_TO_DEGREES(r) ((r) * 180 / PI)
                        
#define MIN(a,b)        (((a) < (b)) ? (a) : (b))
#define MAX(a,b)        (((a) > (b)) ? (a) : (b))


/*
 *	Bridge window sizes:
 */
/* Standard bridge window size. */
#define BW_STD_WIDTH	800
#define BW_STD_HEIGHT	600

/* Size of each console panel. */
#define BW_CONSOLE_PANEL_WIDTH	150
#define BW_CONSOLE_PANEL_HEIGHT	130
/* Height of last (lowest) console panel. */
#define BW_CONSOLE_PANEL_LAST_HEIGHT	300

/* Message box height. */
#define BW_MESGBOX_HEIGHT	(BW_STD_HEIGHT - (BW_CONSOLE_PANEL_HEIGHT * 4))

/* Viewscreen minimum size. */
#define BW_VS_MIN_WIDTH		100
#define BW_VS_MIN_HEIGHT	100

/* Bridge window minimum size. */
#define BW_MIN_WIDTH		((2 * BW_CONSOLE_PANEL_WIDTH) + BW_VS_MIN_WIDTH)
#define BW_MIN_HEIGHT		(BW_VS_MIN_HEIGHT + BW_MESGBOX_HEIGHT)

/* Bridge window maximum size. */
#define BW_MAX_WIDTH		(osw_gui[0].display_width)	
#define BW_MAX_HEIGHT		(osw_gui[0].display_height)


/*
 *	Sets the position values for the button positiion number.
 */
void BW_SET_BTNPOS(
	int btnpos_num,
	int x, int y,
	unsigned int width, unsigned int height
)
{
	bpanel_btnpos_struct *ptr;

	if((btnpos_num < 0) ||
           (btnpos_num >= total_bpanel_btnpos) ||
           (bpanel_btnpos == NULL)
	)
	    return;

	ptr = bpanel_btnpos[btnpos_num];
	if(ptr == NULL)
	    return;

	ptr->x = x;
	ptr->y = y;
	ptr->width = width;
	ptr->height = height;

	return;
}

/*
 *	Initializes the bridge window.
 */
int BridgeWinInit(int argc, char *argv[])
{
	int i, status;
        char title[256];
        pixmap_t pixmap;


	/* Parse arguments. */
	for(i = 0; i < argc; i++)
	{
	    if(argv[i] == NULL)
		continue;





	}

	/* Reset bridge window values. */

        /* Set coordinates and sizes. */
        bridge_win.map_state = 0;
        bridge_win.x = osw_gui[0].def_toplevel_x;
        bridge_win.y = osw_gui[0].def_toplevel_y;
	if(osw_gui[0].def_geometry_set)
	{
            bridge_win.width = MAX(
		osw_gui[0].def_toplevel_width,
	        BW_MIN_WIDTH
	    );
            bridge_win.height = MAX(
		osw_gui[0].def_toplevel_height,
                BW_MIN_HEIGHT
            );
	}
	else
	{
            bridge_win.width = BW_STD_WIDTH;
            bridge_win.height = BW_STD_HEIGHT;
	}
        bridge_win.is_in_focus = 0;

	bridge_win.viewscreen_map_state = 0;

        bridge_win.visibility_state = VisibilityFullyObscured;
        bridge_win.viewscreen_width = MAX(
	    (int)bridge_win.width - (2 * BW_CONSOLE_PANEL_WIDTH),
	    19
	);
        bridge_win.viewscreen_height = MAX(
	    (int)bridge_win.height - BW_MESGBOX_HEIGHT,
	    2
	);

	bridge_win.preset_zoom_code = 0;

        bridge_win.viewscreen_zoom = VS_ZOOM_MIN;
        bridge_win.viewscreen_gamma = 0;        /* Start gamma at 0. */

	bridge_win.vs_weapon_width = 150;
	bridge_win.vs_weapon_height = 48;

        bridge_win.net_stats_width = 400;
        bridge_win.net_stats_height = 16;
        
        bridge_win.pan_p1_width = BW_CONSOLE_PANEL_WIDTH;
        bridge_win.pan_p1_height = BW_CONSOLE_PANEL_HEIGHT;
        bridge_win.pan_p2_width = BW_CONSOLE_PANEL_WIDTH;
        bridge_win.pan_p2_height = BW_CONSOLE_PANEL_HEIGHT;
        bridge_win.pan_p3_width = BW_CONSOLE_PANEL_WIDTH;
        bridge_win.pan_p3_height = BW_CONSOLE_PANEL_HEIGHT;
        bridge_win.pan_p4_width = BW_CONSOLE_PANEL_WIDTH;
        bridge_win.pan_p4_height = BW_CONSOLE_PANEL_LAST_HEIGHT;

        bridge_win.scanner_width = BW_CONSOLE_PANEL_WIDTH;
        bridge_win.scanner_height = BW_CONSOLE_PANEL_HEIGHT;
        bridge_win.scanner_zoom = 0.8;
        bridge_win.scanner_orient = option.def_scanner_orient;
        
        bridge_win.pan_s1_width = BW_CONSOLE_PANEL_WIDTH;
        bridge_win.pan_s1_height = BW_CONSOLE_PANEL_HEIGHT;
        bridge_win.pan_s2_width = BW_CONSOLE_PANEL_WIDTH;
        bridge_win.pan_s2_height = BW_CONSOLE_PANEL_HEIGHT;
        bridge_win.pan_s3_width = BW_CONSOLE_PANEL_WIDTH;
        bridge_win.pan_s3_height = BW_CONSOLE_PANEL_LAST_HEIGHT;
   
        bridge_win.mesg_box_x = 0;
        bridge_win.mesg_box_y = (int)bridge_win.height - BW_MESGBOX_HEIGHT;
        bridge_win.mesg_box_width = bridge_win.width;
        bridge_win.mesg_box_height = BW_MESGBOX_HEIGHT;

        bridge_win.line_spacing = 16;


        /* ******************************************************* */
	/* Allocate and set bridge panel button positions. */

	total_bpanel_btnpos = BPANEL_TOTAL_BTNPOS;
	bpanel_btnpos = NULL;

	if(total_bpanel_btnpos > 0)
	{
	    bpanel_btnpos = (bpanel_btnpos_struct **)realloc(
		bpanel_btnpos,
		total_bpanel_btnpos * sizeof(bpanel_btnpos_struct *)
	    );
	    if(bpanel_btnpos == NULL)
	    {
		total_bpanel_btnpos = 0;
	    }
	}
	for(i = 0; i < total_bpanel_btnpos; i++)
	{
	    bpanel_btnpos[i] = (bpanel_btnpos_struct *)calloc(
		1,
		sizeof(bpanel_btnpos_struct)
	    );
	}

	/* Set button positions. */
	BW_SET_BTNPOS(
	    BPANEL_BTNPOS_PSHIELDFREQ_UP,
	    3, 28, 144, 6
	);
        BW_SET_BTNPOS(
            BPANEL_BTNPOS_PSHIELDFREQ_DOWN,
            3, 34, 144, 6
        );

        BW_SET_BTNPOS( 
            BPANEL_BTNPOS_PWEAPONFREQ_UP,
            3, 42, 144, 6
        );
        BW_SET_BTNPOS(
            BPANEL_BTNPOS_PWEAPONFREQ_DOWN,
            3, 48, 144, 6  
        );

        BW_SET_BTNPOS(
            BPANEL_BTNPOS_PCOMCHANNEL_UP,
            3, 54, 144, 6
        );
        BW_SET_BTNPOS(
            BPANEL_BTNPOS_PCOMCHANNEL_DOWN,
            3, 60, 144, 6
        );

        BW_SET_BTNPOS(
            BPANEL_BTNPOS_PSHIELDS,
            6, 62, 8, 50
        );
        BW_SET_BTNPOS(
            BPANEL_BTNPOS_PCLOAK,
            136, 62, 8, 50
        );
        BW_SET_BTNPOS(
            BPANEL_BTNPOS_PDMGCTL,
            26, 123, 43, 8
        );

        BW_SET_BTNPOS(
            BPANEL_BTNPOS_PINTERCEPT,
            3, 0, 144, 12
        );
        BW_SET_BTNPOS(
            BPANEL_BTNPOS_PLOCK,
            3, 14, 144, 12
        );
	BW_SET_BTNPOS(			/* On scanner readout console 1. */
	    BPANEL_BTNPOS_PLOCKNEXT,
	    0, 0, 150, 150
	);

        BW_SET_BTNPOS(
            BPANEL_BTNPOS_PWEPYIELD,
            3, 28, 144, 14
        );
        BW_SET_BTNPOS(
	    BPANEL_BTNPOS_PSELWEP1,
	    3, 42, 144, 14
	);
        BW_SET_BTNPOS(
            BPANEL_BTNPOS_PSELWEP2,
            3, 56, 144, 14  
        );
        BW_SET_BTNPOS(
            BPANEL_BTNPOS_PSELWEP3,
            3, 70, 144, 14
        );
        BW_SET_BTNPOS(
            BPANEL_BTNPOS_PSELWEP4,
            3, 84, 144, 14
        );

        BW_SET_BTNPOS(
            BPANEL_BTNPOS_PENGINESTATE,
            75, 92, 55, 30
        );



        /* ******************************************************* */
	/* Initialize bridge window's GUI resources. */

        /* Error checks. */
        if(!IDC())
            return(-1);

        /* Toplevel. */
        if(
            OSWCreateWindow(
                &bridge_win.toplevel, 
                osw_gui[0].root_win,
                bridge_win.x, bridge_win.y,
                bridge_win.width, bridge_win.height  
            )
        )
            return(-1);

        /* WM properties. */
        sprintf(title, "%s: Untitled", PROG_NAME);
        if(IMGIsImageNumAllocated(IMG_CODE_XSW_ICON))
        {
            pixmap = OSWCreatePixmapFromImage(
                xsw_image[IMG_CODE_XSW_ICON]->image
            );
        }
        else
        {
            pixmap = widget_global.std_icon_pm;
        }
        OSWSetWindowWMProperties(
            bridge_win.toplevel,
            title,              /* Title. */
            PROG_NAME,		/* Icon title. */
            pixmap,             /* Icon. */
            !osw_gui[0].def_geometry_set,	/* Let WM set coordinates? */
            bridge_win.x, bridge_win.y,
            BW_MIN_WIDTH,
            BW_MIN_HEIGHT,
            BW_MAX_WIDTH,
            BW_MAX_HEIGHT,
            WindowFrameStyleStandard,
            NULL, 0
        );   
        OSWSetWindowBkg(bridge_win.toplevel, osw_gui[0].black_pix, 0);

        
        /* Viewscreen. */
        status = OSWCreateWindow(
            &bridge_win.viewscreen,
            bridge_win.toplevel,
            150, 0,
            bridge_win.viewscreen_width,
            bridge_win.viewscreen_height
        );
        if(status)
            return(-1);
        OSWSetWindowBkg(bridge_win.viewscreen, osw_gui[0].black_pix, 0);
        /* Shared image for viewscreen. */
        if(
            OSWCreateSharedImage(
                &bridge_win.viewscreen_image,
                bridge_win.viewscreen_width,
                bridge_win.viewscreen_height
            )
        )
            return(-1);
        bridge_win.viewscreen_width = bridge_win.viewscreen_image->width;
        bridge_win.viewscreen_height = bridge_win.viewscreen_image->height;
            
            
        /* Create netstats pixmap and image for use on viewscreen. */
        if(OSWCreateImage(&bridge_win.net_stats_image,
            bridge_win.net_stats_width, bridge_win.net_stats_height
        ))
            return(-1);
        bridge_win.net_stats_width = bridge_win.net_stats_image->width;
        bridge_win.net_stats_height = bridge_win.net_stats_image->height;
 
        if(OSWCreatePixmap(&bridge_win.net_stats_buf,
            bridge_win.net_stats_width, bridge_win.net_stats_height
        ))
            return(-1);

	/* Create weapon stats pixmap and image for use on viewscreen. */
        if(OSWCreateImage(&bridge_win.vs_weapon_image,
            bridge_win.vs_weapon_width, bridge_win.vs_weapon_height
        ))
            return(-1);
        bridge_win.vs_weapon_width = bridge_win.vs_weapon_image->width;
        bridge_win.vs_weapon_height = bridge_win.vs_weapon_image->height;

        if(OSWCreatePixmap(&bridge_win.vs_weapon_buf,
            bridge_win.vs_weapon_width, bridge_win.vs_weapon_height
        ))
            return(-1);


	/* Console buffer (buffer for all bridge panel consoles). */
	if(
	    OSWCreatePixmap(
		&bridge_win.pan_buf,
		BW_CONSOLE_PANEL_WIDTH,
		BW_CONSOLE_PANEL_LAST_HEIGHT
	    )
	)
	    return(-1);

	/* Stats console 2 and 3 image buffers. */
        if(
            OSWCreateSharedImage(
                &bridge_win.pan_p2_img,
                bridge_win.pan_p2_width,
                bridge_win.pan_p2_height
            )
        )
            return(-1);
        if(
            OSWCreateSharedImage(
                &bridge_win.pan_p3_img,
                bridge_win.pan_p3_width,
                bridge_win.pan_p3_height
            )
        )
            return(-1);

        /* Scanner and subject readout console 3 image buffers. */
        if(
            OSWCreateSharedImage(
                &bridge_win.pan_s1_img,
                bridge_win.pan_s1_width,
                bridge_win.pan_s1_height
            )
        )
            return(-1);
        if(
            OSWCreateSharedImage(
                &bridge_win.pan_s3_img,
                bridge_win.pan_s3_width,
                bridge_win.pan_s3_height
            )
        )
            return(-1);


        /* Stats consoles. */
	if(
            OSWCreateWindow(
                &bridge_win.pan_p1,
                bridge_win.toplevel,
                0, 0,
                bridge_win.pan_p1_width, bridge_win.pan_p1_height
	    )
        )
            return(-1);
        if(
            OSWCreateWindow(  
                &bridge_win.pan_p2,
                bridge_win.toplevel,
                0, (BW_CONSOLE_PANEL_HEIGHT * 1),
                bridge_win.pan_p2_width, bridge_win.pan_p2_height
            )
        )
            return(-1);
        if(
            OSWCreateWindow(  
                &bridge_win.pan_p3,
                bridge_win.toplevel,
                0, (BW_CONSOLE_PANEL_HEIGHT * 2),
                bridge_win.pan_p3_width, bridge_win.pan_p3_height
            )
        )
            return(-1);
        /* Player stats console 4. */
        if(bridge_win.height > BW_STD_HEIGHT)
        {
            bridge_win.pan_p4_width = BW_CONSOLE_PANEL_WIDTH;
            bridge_win.pan_p4_height = MAX(
                (int)bridge_win.height - (int)bridge_win.mesg_box_height
                - (BW_CONSOLE_PANEL_HEIGHT * 3),
                BW_CONSOLE_PANEL_LAST_HEIGHT
            );
        }
        else
        {
            bridge_win.pan_p4_width = BW_CONSOLE_PANEL_WIDTH;
            bridge_win.pan_p4_height = BW_CONSOLE_PANEL_LAST_HEIGHT;
        }
        if(
            OSWCreateWindow(
                &bridge_win.pan_p4,
                bridge_win.toplevel,
                0,
		(BW_CONSOLE_PANEL_HEIGHT * 3),
                bridge_win.pan_p4_width,
		bridge_win.pan_p4_height
            )
        )
            return(-1);

	/* Reset weapon yield scale input window. */
	bridge_win.weapon_yield_iwin = 0;
            
        /* Scanner. */
	if(
            OSWCreateWindow(
                &bridge_win.scanner,
                bridge_win.toplevel, 
                (int)bridge_win.width - (int)bridge_win.scanner_width,
                0,
                bridge_win.scanner_width,
                bridge_win.scanner_height
	    )
        )
            return(-1);
        OSWSetWindowBkg(bridge_win.scanner, osw_gui[0].black_pix, 0);
        if(
            OSWCreateSharedImage(  
                &bridge_win.scanner_image,
                bridge_win.scanner_width,
                bridge_win.scanner_height
            )
        )
            return(-1);
        bridge_win.scanner_width = bridge_win.scanner_image->width;
        bridge_win.scanner_height = bridge_win.scanner_image->height;

        /* Sensor scale bar widget. */
	if(
            ScaleBarInit(
                &bridge_win.scanner_sb,
                bridge_win.scanner,		/* Parent. */
		(int)bridge_win.scanner_width - SCALEBAR_BAR_FLUSHED_WIDTH,
		0,
		bridge_win.scanner_height, 	/* Length. */
                SCALEBAR_STYLE_FLUSHED,		/* Style. */
                5,				/* Ticks. */
		SCALEBAR_ORIENT_VERTICAL,	/* Orientation. */
		0, 100, 80,			/* Pos min, max, start. */
		True,				/* Flip position. */
		(void *)&bridge_win.scanner_sb,
		ScannerSBCB
	    )
        )
            return(-1);


        /* Subject readout console panels. */
	if(
            OSWCreateWindow(
                &bridge_win.pan_s1,
                bridge_win.toplevel,
                (int)bridge_win.width - (int)bridge_win.pan_s1_width,
                (BW_CONSOLE_PANEL_HEIGHT * 1),
                bridge_win.pan_s1_width, bridge_win.pan_s1_height
	    )
        )
            return(-1);

        if(
            OSWCreateWindow(
                &bridge_win.pan_s2,
                bridge_win.toplevel,
                (int)bridge_win.width - (int)bridge_win.pan_s2_width,
                (BW_CONSOLE_PANEL_HEIGHT * 2),
                bridge_win.pan_s2_width, bridge_win.pan_s2_height
	    )
        )
            return(-1);

        if(bridge_win.height > BW_STD_HEIGHT)
        {
            bridge_win.pan_s3_width = BW_CONSOLE_PANEL_WIDTH;
            bridge_win.pan_s3_height = MAX(
                (int)bridge_win.height - (int)bridge_win.mesg_box_height
                - (BW_CONSOLE_PANEL_HEIGHT * 3),
                BW_CONSOLE_PANEL_LAST_HEIGHT
            );
        }
        else
        {
            bridge_win.pan_s3_width = BW_CONSOLE_PANEL_WIDTH;
            bridge_win.pan_s3_height = BW_CONSOLE_PANEL_LAST_HEIGHT;
        }
        if(
            OSWCreateWindow(
                &bridge_win.pan_s3,
                bridge_win.toplevel,
                (int)bridge_win.width - (int)bridge_win.pan_s3_width,
                (BW_CONSOLE_PANEL_HEIGHT * 3),
                bridge_win.pan_s3_width,
		bridge_win.pan_s3_height
	    )
        )
            return(-1);
        
            
        /* Message box. */
        if(OSWCreateWindow(
            &bridge_win.mesg_box,
            bridge_win.toplevel,
            bridge_win.mesg_box_x, bridge_win.mesg_box_y,
            bridge_win.mesg_box_width, bridge_win.mesg_box_height
        ))
            return(-1);
        if(OSWCreatePixmap(
            &bridge_win.mesg_box_buf,
            bridge_win.mesg_box_width, bridge_win.mesg_box_height
        ))
            return(-1);


        /* Prompt. */
        status = PromptInit(
            &bridge_win.prompt,
            bridge_win.toplevel,
            bridge_win.pan_p1_width + 20,
            MAX(bridge_win.viewscreen_height - 90, 0),
            MAX(bridge_win.viewscreen_width - 40, 200),
            30,
            PROMPT_STYLE_RAISED,
            "Command:",
            DEF_PROMPT_BUF_LEN,
            DEF_PROMPT_HIST_BUFS,
            NULL
        );

        /* Messages scroll bar widget. */
        status = SBarInit(&bridge_win.mesg_box_sb,
            bridge_win.mesg_box,     
            bridge_win.mesg_box_width,
            bridge_win.mesg_box_height
        );

        /* Need to set the y position to the bottom. */
        bridge_win.mesg_box_sb.y_win_pos = (bridge_win.line_spacing *
            (MESG_WIN_TOTAL_MESSAGES + 1)) -
            bridge_win.mesg_box_height;


        /* ************************************************************ */
        /* Set selected event inputs. */

	/* Toplevel. */
        OSWSetWindowInput(bridge_win.toplevel,
	    KeyPressMask | KeyReleaseMask |
            FocusChangeMask | StructureNotifyMask
        );

	/*   Skip viewscreen, it's event input selection will be set
	 *   upon mapping.
	 */

	/* Scanner. */
	OSWSetWindowInput(bridge_win.scanner,
	    ButtonPressMask | ButtonReleaseMask |
            ExposureMask | VisibilityChangeMask
	);

	/* Console panels. */
        OSWSetWindowInput(bridge_win.pan_p1, OSW_EVENTMASK_BUTTON);
        OSWSetWindowInput(bridge_win.pan_p2, OSW_EVENTMASK_BUTTON);
        OSWSetWindowInput(bridge_win.pan_p3, OSW_EVENTMASK_BUTTON);
        OSWSetWindowInput(bridge_win.pan_p4, OSW_EVENTMASK_BUTTON);

        OSWSetWindowInput(bridge_win.pan_s1, OSW_EVENTMASK_BUTTON);
        OSWSetWindowInput(bridge_win.pan_s2, OSW_EVENTMASK_BUTTON);
        OSWSetWindowInput(bridge_win.pan_s3, OSW_EVENTMASK_BUTTON);

	/* Message box. */
        OSWSetWindowInput(bridge_win.mesg_box, OSW_EVENTMASK_BUTTON |
	    PointerMotionMask
	);


        /* Define cursors. */
        WidgetSetWindowCursor(bridge_win.scanner, xsw_cursor.scanner_lock);
        WidgetSetWindowCursor(bridge_win.mesg_box, xsw_cursor.text);



	/* Initialize pages. */
	if(PageInit(
	    &bridge_win.main_page,
	    bridge_win.viewscreen,
            bridge_win.viewscreen_image
	))
	    return(-1);

        if(PageInit(
	    &bridge_win.destroyed_page,
	    bridge_win.viewscreen,
	    bridge_win.viewscreen_image
	))
            return(-1);

	/* Select the main page. */
	bridge_win.cur_page = &bridge_win.main_page;


	/* Force/trick unmap both the viewscreen and the current page.
	 * This is to reset certain resources for the first time,
	 * mainly the viewscreen input event selection and cursor.
	 */
        bridge_win.viewscreen_map_state = 1;
        VSUnmap();

	bridge_win.cur_page->map_state = 1;
	PageUnmap(
	    bridge_win.cur_page,
	    bridge_win.viewscreen,
	    bridge_win.viewscreen_image
	);


        return(0);
}         


/*
 *      Handles a resizing of the bridge window.
 *      Changes the sizes of all sub windows and resources, including
 *      the viewscreen. Reallocates image buffers.
 */
void BridgeWinResize(void)
{
        win_attr_t wattr;


        /* Get new size of toplevel. */
        OSWGetWindowAttributes(bridge_win.toplevel, &wattr);

	/* Check for change in size. */
        if((bridge_win.width == (unsigned int)wattr.width) &&
           (bridge_win.height == (unsigned int)wattr.height)
        )
            return;

        /* Set new position and size. */
        bridge_win.x = wattr.x;
        bridge_win.y = wattr.y;
        bridge_win.width = wattr.width;
        bridge_win.height = wattr.height;



	/* Resize viewscreen. */

        /* Wait for any draw procedures to finish. */
        OSWSyncSharedImage(
            bridge_win.viewscreen_image,
            bridge_win.viewscreen
        );

        OSWMoveResizeWindow(
            bridge_win.viewscreen,
            bridge_win.pan_p1_width,
            0,
            MAX((int)bridge_win.width - (int)bridge_win.pan_p1_width -
                (int)bridge_win.scanner_width,
                10
            ),
            MAX((int)bridge_win.height - (int)bridge_win.mesg_box_height,
                10
            )
        );
	/* Verify size values. */
        OSWGetWindowAttributes(bridge_win.viewscreen, &wattr);

        bridge_win.viewscreen_width = wattr.width;
        bridge_win.viewscreen_height = wattr.height;

        /* Destroy shared image. */
        OSWDestroySharedImage( 
            &bridge_win.viewscreen_image
        );
        /* Create new shared image. */
        if(
            OSWCreateSharedImage(
                &bridge_win.viewscreen_image,
                bridge_win.viewscreen_width,
                bridge_win.viewscreen_height
            )
        )
            return;


        /* Resize current page viewscreen. */
        PageResize(
	    bridge_win.cur_page,
	    bridge_win.viewscreen,
            bridge_win.viewscreen_image
        );



	/* Player stats console panels 1 to 3 stay in place and size. */
        OSWMoveResizeWindow(
            bridge_win.pan_p1,
            0,
            (BW_CONSOLE_PANEL_HEIGHT * 0),
            bridge_win.pan_p1_width,
            bridge_win.pan_p1_height
        );
        OSWMoveResizeWindow(
            bridge_win.pan_p2,
            0,
            (BW_CONSOLE_PANEL_HEIGHT * 1),
            bridge_win.pan_p2_width,
            bridge_win.pan_p2_height
        );
        OSWMoveResizeWindow(
            bridge_win.pan_p3,
            0,
            (BW_CONSOLE_PANEL_HEIGHT * 2),
            bridge_win.pan_p3_width,
            bridge_win.pan_p3_height
        );

	/* Player stats console 4 size change. */
        if(bridge_win.height > BW_STD_HEIGHT)
        {
            bridge_win.pan_p4_width = BW_CONSOLE_PANEL_WIDTH;
            bridge_win.pan_p4_height = MAX(
                (int)bridge_win.height - (int)bridge_win.mesg_box_height
                - (BW_CONSOLE_PANEL_HEIGHT * 3),
                BW_CONSOLE_PANEL_LAST_HEIGHT
            );
        }
        else
        {
            bridge_win.pan_p4_width = BW_CONSOLE_PANEL_WIDTH;
            bridge_win.pan_p4_height = BW_CONSOLE_PANEL_LAST_HEIGHT;
        }
        OSWMoveResizeWindow(  
            bridge_win.pan_p4,
            0,
            (BW_CONSOLE_PANEL_HEIGHT * 3),
            bridge_win.pan_p4_width,
            bridge_win.pan_p4_height
        );
              
            
        /* Scanner and scanner readout consoles. */
        OSWMoveWindow(
            bridge_win.scanner,
            (int)bridge_win.width - (int)bridge_win.scanner_width,
            (BW_CONSOLE_PANEL_HEIGHT * 0)
        );
        OSWMoveWindow(
            bridge_win.pan_s1,
            (int)bridge_win.width - (int)bridge_win.pan_s1_width,
            (BW_CONSOLE_PANEL_HEIGHT * 1)
        );
        OSWMoveWindow(
            bridge_win.pan_s2,
            (int)bridge_win.width - (int)bridge_win.pan_s2_width,
	    (BW_CONSOLE_PANEL_HEIGHT * 2)
        );
	if(bridge_win.height > BW_STD_HEIGHT)
	{
	    bridge_win.pan_s3_width = BW_CONSOLE_PANEL_WIDTH;
	    bridge_win.pan_s3_height = MAX(
		(int)bridge_win.height - (int)bridge_win.mesg_box_height
		- (BW_CONSOLE_PANEL_HEIGHT * 3),
		BW_CONSOLE_PANEL_LAST_HEIGHT
	    );
	}
	else
	{
            bridge_win.pan_s3_width = BW_CONSOLE_PANEL_WIDTH;
            bridge_win.pan_s3_height = BW_CONSOLE_PANEL_LAST_HEIGHT;
	}
        OSWMoveResizeWindow(
            bridge_win.pan_s3,
            (int)bridge_win.width - (int)bridge_win.pan_s3_width,
            (BW_CONSOLE_PANEL_HEIGHT * 3),
	    bridge_win.pan_s3_width,
	    bridge_win.pan_s3_height
        );


        /* Messages window. */
        OSWMoveResizeWindow(
            bridge_win.mesg_box,
            0,
            bridge_win.viewscreen_height,
            bridge_win.width,
            bridge_win.mesg_box_height
        );  
        OSWGetWindowAttributes(bridge_win.mesg_box, &wattr);
        bridge_win.mesg_box_x = wattr.x;
        bridge_win.mesg_box_y = wattr.y;
        bridge_win.mesg_box_width = wattr.width;
        bridge_win.mesg_box_height = wattr.height;
        OSWDestroyPixmap(&bridge_win.mesg_box_buf);
        if(
            OSWCreatePixmap(&bridge_win.mesg_box_buf,
                bridge_win.mesg_box_width, bridge_win.mesg_box_height
            )
        ) 
            return;   
            
        SBarResize(
	    &bridge_win.mesg_box_sb,
            bridge_win.mesg_box_width,
	    bridge_win.mesg_box_height
	);

        /* Scroll messages_sb to bottom. */
        bridge_win.mesg_box_sb.y_win_pos = (bridge_win.line_spacing *
            (MESG_WIN_TOTAL_MESSAGES + 1)) -  
            bridge_win.mesg_box_height;
        SBarDraw(
            &bridge_win.mesg_box_sb,
            bridge_win.mesg_box_width,
            bridge_win.mesg_box_height,
            bridge_win.mesg_box_width,
	    bridge_win.line_spacing * (MESG_WIN_TOTAL_MESSAGES + 1)
        );
        BridgeDrawMessages();
            
            
        /* Prompt. */
        OSWMoveResizeWindow(
	    bridge_win.prompt.toplevel,
            bridge_win.pan_p1_width + 20,
            MAX(bridge_win.viewscreen_height - 90, 0),
            MAX(bridge_win.viewscreen_width - 40, 200),
            30
        );


        return;
}

/*
 *	Changes the size of the bridge window to one of
 *	the `preset/preffered' sizes, then calls
 *	BridgeWinResize().
 *
 *	Updates the preset_zoom_code to value of step on
 *	bridge_win structure.
 */
void BridgeWinResizePreset(int step)
{
	const double aspect = (double)800 / (double)600;
	const int min_width = BW_VS_MIN_WIDTH + (BW_CONSOLE_PANEL_WIDTH * 2);
	const int min_height = BW_VS_MIN_HEIGHT + BW_MESGBOX_HEIGHT;
	unsigned int width, height;


	/* Sanitize step. */
	if(step < -2)
	    step = -2;
	if(step > 0)
	    step = 0;


	/* Check if there was no change. */
/*
	if(bridge_win.preset_zoom_code == step)
	    return;
 */


	switch(step)
	{
	  case -2:
	    height = MAX(
                ((int)bridge_win.pan_p1_height * 2) +
                    (int)bridge_win.mesg_box_height,
                min_height
            );
            width = static_cast<unsigned int>(MAX(
                (((int)height - (int)bridge_win.mesg_box_height)
		    * aspect) + (2 * bridge_win.pan_p1_width),
                min_width
            ));
	    break;

	  case -1:
	    height = MAX(
		((int)bridge_win.pan_p1_height * 3) +
		    (int)bridge_win.mesg_box_height,
		min_height
	    );
	    width = static_cast<unsigned int>(MAX(
		(((int)height - (int)bridge_win.mesg_box_height)
                    * aspect) + (2 * bridge_win.pan_p1_width),
		min_width
	    ));
	    break;

	  default:	/* Default to 0, nominal/standard. */
	    width = BW_STD_WIDTH;
	    height = BW_STD_HEIGHT;
	    break;
	}

	/* Resize bridge window's toplevel. */
	OSWResizeWindow(
	    bridge_win.toplevel,
	    width, height
	);

	/* Update zoom code. */
	bridge_win.preset_zoom_code = step;


	/* Perform resize procedure on sub windows and resources. */
	BridgeWinResize();


	return;
}



/*
 *	Maps bridge window and all its subwindows, then redraws
 *	bridge panels.
 */
void BridgeWinMap(void)
{
	int obj_num, tar_obj_num;
	xsw_object_struct *obj_ptr;
	xsw_bridge_win_struct *bridge;


	obj_num = net_parms.player_obj_num;
	obj_ptr = net_parms.player_obj_ptr;
	bridge = &bridge_win;

        XSWDoUnfocusAllWindows();


        bridge->map_state = 1;
/*
        bridge->is_in_focus = 1;
        bridge->visibility_state = VisibilityUnobscured;
 */

        OSWMapRaised(bridge->toplevel);
        OSWMapWindow(bridge->pan_p1);
        OSWMapWindow(bridge->pan_p2);
        OSWMapWindow(bridge->pan_p3);
        OSWMapWindow(bridge->pan_p4);
        OSWMapWindow(bridge->pan_s1);
        OSWMapWindow(bridge->pan_s2);
        OSWMapWindow(bridge->pan_s3);

        bridge->scanner_map_state = 1;
	if(bridge->scanner_image != NULL)
	    bridge->scanner_image->in_progress = False;
        OSWMapWindow(bridge->scanner);

        OSWMapWindow(bridge->viewscreen);
	/* Do not set viewscreen map state here. */

        OSWMapWindow(bridge->mesg_box);  


        /* Scanner scale bar. */
        ScaleBarMap(&bridge->scanner_sb);
 
        /* Prompt window. */
        PromptUnmap(&bridge->prompt);


	/* Redraw all panels. */
        BridgeWinDrawPanel(obj_num, BPANEL_DETAIL_P1);
        BridgeWinDrawPanel(obj_num, BPANEL_DETAIL_P2);
        BridgeWinDrawPanel(obj_num, BPANEL_DETAIL_P3);
        BridgeWinDrawPanel(obj_num, BPANEL_DETAIL_P4);
	if(obj_ptr == NULL)
	    tar_obj_num = -1;
	else
	    tar_obj_num = obj_ptr->locked_on;
        BridgeWinDrawPanel(tar_obj_num, BPANEL_DETAIL_S1);
        BridgeWinDrawPanel(tar_obj_num, BPANEL_DETAIL_S2);
        BridgeWinDrawPanel(tar_obj_num, BPANEL_DETAIL_S3);

	/* Restack all XSW windows. */
	XSWDoRestackWindows();

        return;
}

/*
 *	Unmaps bridge window, deallocates some of its resources.
 */
void BridgeWinUnmap(void)
{
        PromptUnmap(&bridge_win.prompt);
	ScaleBarUnmap(&bridge_win.scanner_sb);

        OSWUnmapWindow(bridge_win.toplevel);  
        bridge_win.map_state = 0;
        bridge_win.is_in_focus = 0;
        bridge_win.visibility_state = VisibilityFullyObscured;
        

        return;
}

void BridgeWinDestroy(void)
{
	int i;


        if(IDC())
        {
            /* Destroy widgets. */
            PromptDestroy(&bridge_win.prompt);
            SBarDestroy(&bridge_win.mesg_box_sb);
            ScaleBarDestroy(&bridge_win.scanner_sb);


            /* Destroy the main menu. */
            PageDestroy(
		&bridge_win.main_page,
                bridge_win.viewscreen,
                bridge_win.viewscreen_image  
            );
	    PageDestroy(
                &bridge_win.destroyed_page,
                bridge_win.viewscreen,
                bridge_win.viewscreen_image
            );

  
            /* Free shared memory for viewscreen buffer. */
            OSWDestroySharedImage(&bridge_win.viewscreen_image);

            /* Free shared memory for scanner buffer. */
            OSWDestroySharedImage(&bridge_win.scanner_image);

	    /* Scanner label images. */
            OSWDestroyImage(&bridge_win.scanner_range_label);
            OSWDestroyImage(&bridge_win.scanner_loc_label);


            /* Destroy Windows and Pixmaps. */
 
            OSWDestroyPixmap(&bridge_win.mesg_box_buf);
            OSWDestroyPixmap(&bridge_win.pan_buf);

	    OSWDestroySharedImage(&bridge_win.pan_p2_img);
            OSWDestroySharedImage(&bridge_win.pan_p3_img);
            OSWDestroySharedImage(&bridge_win.pan_s1_img);
            OSWDestroySharedImage(&bridge_win.pan_s3_img);

            OSWDestroyPixmap(&bridge_win.net_stats_buf);
            OSWDestroyImage(&bridge_win.net_stats_image);

	    OSWDestroyPixmap(&bridge_win.vs_weapon_buf);
	    OSWDestroyImage(&bridge_win.vs_weapon_image);

            OSWDestroyWindow(&bridge_win.mesg_box);

	    OSWDestroyWindow(&bridge_win.weapon_yield_iwin);            
            OSWDestroyWindow(&bridge_win.pan_p4);
            OSWDestroyWindow(&bridge_win.pan_p3);
            OSWDestroyWindow(&bridge_win.pan_p2);
            OSWDestroyWindow(&bridge_win.pan_p1);
  
            OSWDestroyWindow(&bridge_win.pan_s3);
            OSWDestroyWindow(&bridge_win.pan_s2);
            OSWDestroyWindow(&bridge_win.pan_s1);
            OSWDestroyWindow(&bridge_win.scanner);
              
            OSWDestroyWindow(&bridge_win.viewscreen);
            OSWDestroyWindow(&bridge_win.toplevel);
        }
            
        /* Set unmapped. */
        bridge_win.map_state = 0;
        bridge_win.is_in_focus = 0;
        bridge_win.visibility_state = VisibilityFullyObscured;


	/* Delete all bridge panel button positions. */
	for(i = 0; i < total_bpanel_btnpos; i++)
	    free(bpanel_btnpos[i]);

	free(bpanel_btnpos);
	bpanel_btnpos = NULL;

	total_bpanel_btnpos = 0;


        return;
}
