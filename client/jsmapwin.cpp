/*
                           Joystick Mapping Window

	Functions:

	int JSMWLoadAll()
	int JSMWApplyAll()

	int JSMWDoSelectJoystick(int n)
	int JSMWDoApplyJoystick(int n)

	int JSMWAddPBCB(void *ptr)
	int JSMWRemovePBCB(void *ptr)

	int JSMWInitPBCB(void *ptr)

	int JSMWJoystickListCB(void *ptr)
	int JSMWButtonsListCB(void *ptr)
	int JSMWScanKeyPBCB(void *ptr)

        int JSMWOKPBCB(void *ptr)
        int JSMWApplyPBCB(void *ptr)
        int JSMWCancelPBCB(void *ptr)

	int JSMWInit()
	void JSMWResize()
	int JSMWDraw(int amount)
	int JSMWManage(event_t *event)
	void JSMWMap()
	void JSMWMapValues()
	void JSMWUnmap()
	void JSMWDestroy()

	---

 */

#ifdef JS_SUPPORT

#include "jsmapwin.h"
#include "xsw.h"



#define JSMW_DEF_JSDEVICE_NAME	"/dev/null"



#define MIN(a,b)	(((a) < (b)) ? (a) : (b))
#define MAX(a,b)	(((a) > (b)) ? (a) : (b))


xsw_jsmap_win_struct jsmap_win;


/*
 *	Procedure to load all jsmaps into the widgets in the
 *	jsmap_win, redraws as needed.
 */
int JSMWLoadAll()
{
        int i;
        jsmap_struct *jsmap_ptr;
        colum_list_struct *list;
        char text[PATH_MAX + NAME_MAX + 80];



        list = &jsmap_win.js_list;

	/* Delete old listing first. */
        CListDeleteAllRows(list);

	/* Fetch new joystick listing from jsmaps. */
        for(i = 0; i < total_jsmaps; i++)
        {
	    jsmap_ptr = jsmap[i];
            if(jsmap_ptr == NULL)
                continue;

            if(CListAddRow(list, i) < 0)
                continue;

            sprintf(text, "Joystick %i", i);
	    if(CListAddItem(
                list,
                text,
                OSWQueryCurrentFont(),
                widget_global.editable_text_pix,
                0,                      /* Attributes. */
                i
            ) < 0)
		continue;

	    sprintf(
		text,
		"%s",
		((jsmap_ptr->device_name == NULL) ?
		    JSMW_DEF_JSDEVICE_NAME : jsmap_ptr->device_name
		)
	    );
	    if(CListAddItem(
                list,
                text,
                OSWQueryCurrentFont(),
                widget_global.editable_text_pix,
                0,                      /* Attributes. */
                i
            ) < 0)
                continue;
	}

	/* Clear all prompts. */
	PromptSetS(&jsmap_win.device_prompt, "");

	PromptSetS(&jsmap_win.turn_axis_prompt, "");
        PromptSetS(&jsmap_win.throttle_axis_prompt, "");
	PromptSetS(&jsmap_win.thrust_dir_axis_prompt, "");
        PromptSetS(&jsmap_win.vs_zoom_axis_prompt, "");
        PromptSetS(&jsmap_win.scanner_zoom_axis_prompt, "");
        PromptSetS(&jsmap_win.aim_weapon_heading_prompt, "");


	/* Clear buttons listing. */
	list = &jsmap_win.buttons_list;
	CListDeleteAllRows(list);


	return(0);
}


/*
 *	Applies selected joystick in the joystick list widget
 *	of the jsmap_win.
 */
int JSMWApplyAll()
{
        colum_list_struct *list;
	int i;

	list = &jsmap_win.js_list;
	if(list->total_sel_rows > 0)
	{
	    i = list->sel_row[list->total_sel_rows - 1];
	    JSMWDoApplyJoystick(i);
	}


        return(0);
}


/*
 *	Fetches values from jsmap n and puts them into the widgets
 *	of the jsmap_win.
 */
int JSMWDoSelectJoystick(int n)
{
        int i, status;
        jsmap_struct *jsmap_ptr;
        colum_list_struct *list;
	char text[80];


        if(JSMapIsAllocated(n))
            jsmap_ptr = jsmap[n];
        else
            return(-1);


	/* Reset axis values. */
        PromptSetS(&jsmap_win.turn_axis_prompt, "-1");
        PromptSetS(&jsmap_win.throttle_axis_prompt, "-1");
        PromptSetS(&jsmap_win.thrust_dir_axis_prompt, "-1");
        PromptSetS(&jsmap_win.vs_zoom_axis_prompt, "-1");
        PromptSetS(&jsmap_win.scanner_zoom_axis_prompt, "-1");
        PromptSetS(&jsmap_win.aim_weapon_heading_prompt, "-1");

	/* Get new axis values. */
	for(i = 0; i < jsmap_ptr->total_axises; i++)
	{
	    if(jsmap_ptr->axis[i] == NULL)
		continue;

	    status = jsmap_ptr->axis[i]->op_code;
	    switch(status)
	    {
	      case JSMAP_AXIS_OP_TURN:
	        PromptSetI(
		    &jsmap_win.turn_axis_prompt,
		    i
		);
		break;

              case JSMAP_AXIS_OP_THROTTLE:
                PromptSetI(
                    &jsmap_win.throttle_axis_prompt,
                    i
                );
                break;

	      case JSMAP_AXIS_OP_THRUST_DIR:
                PromptSetI(
                    &jsmap_win.thrust_dir_axis_prompt,
                    i
                );
                break;

              case JSMAP_AXIS_OP_SCANNER_ZOOM:
                PromptSetI(
                    &jsmap_win.vs_zoom_axis_prompt,
                    i
                );
                break;

              case JSMAP_AXIS_OP_VS_ZOOM:
                PromptSetI(
                    &jsmap_win.scanner_zoom_axis_prompt,
                    i
                );
                break;

              case JSMAP_AXIS_OP_AIM_WEAPON_HEADING:
                PromptSetI(
                    &jsmap_win.aim_weapon_heading_prompt,
                    i
                );
                break;

	      case JSMAP_AXIS_OP_AIM_WEAPON_PITCH:

		break;
	    }
	}


	/* Get button listing. */
        list = &jsmap_win.buttons_list;
	CListDeleteAllRows(list);

	for(i = 0; i < jsmap_ptr->total_buttons; i++)
	{
	    if(jsmap_ptr->button[i] == NULL)
		continue;

	    if(CListAddRow(list, i) < 0)
		continue;

	    sprintf(text, "Button %i", i);
	    status = CListAddItem(
		list,
		text,
		OSWQueryCurrentFont(),
		widget_global.editable_text_pix,
		0,			/* Attributes. */
		i
	    );
            if(status < 0)
                continue;

            sprintf(
		text,
		"%s",
		OSWGetKeyCodeName(jsmap_ptr->button[i]->keycode)
	    );
            status = CListAddItem(
                list,
                text,
                OSWQueryCurrentFont(),
                widget_global.editable_text_pix,
                0,                      /* Attributes. */
                i
            );
	    if(status < 0)
		continue;

	    /* Set data pointer on list item on colum 1 to
	     * be the value of (not point to) the button's
	     * generated keycode.
	     */
	    CListSetItemDataPtr(
		list,
		i, 1,		/* Row, colum. */
		(void *)jsmap_ptr->button[i]->keycode
	    );
	}


	/* Update device prompt. */
	PromptSetS(
	    &jsmap_win.device_prompt,
	    jsmap_ptr->device_name
	);


        return(0);
}

/*
 *	Applies values from widgets into jsmap n.
 */
int JSMWDoApplyJoystick(int n)
{
	int i;
	jsmap_struct *jsmap_ptr;
	jsmap_button_struct *jsmap_button_ptr;
	colum_list_struct *list;


	if(JSMapIsAllocated(n))
	    jsmap_ptr = jsmap[n];
	else
	    return(-1);


	/* Reset axis values. */
        for(i = 0; i < jsmap_ptr->total_axises; i++)
        {
            if(jsmap_ptr->axis[i] != NULL)
                jsmap_ptr->axis[i]->op_code = JSMAP_AXIS_OP_NONE;
 	}


	/* Begin applying axis values. */

	/* Turn axis number. */
	i = PromptGetI(&jsmap_win.turn_axis_prompt);
	if((i >= 0) && (i < jsmap_ptr->total_axises))
	{
	    if(jsmap_ptr->axis[i] != NULL)
	    {
		jsmap_ptr->axis[i]->op_code = JSMAP_AXIS_OP_TURN;
	    }
	}

	/* Throttle axis number. */
        i = PromptGetI(&jsmap_win.throttle_axis_prompt); 
        if((i >= 0) && (i < jsmap_ptr->total_axises))
        {
            if(jsmap_ptr->axis[i] != NULL)
            {
                jsmap_ptr->axis[i]->op_code = JSMAP_AXIS_OP_THROTTLE;
            }
        }

	/* Thrust direction axis number. */
        i = PromptGetI(&jsmap_win.thrust_dir_axis_prompt);
        if((i >= 0) && (i < jsmap_ptr->total_axises))
        {
            if(jsmap_ptr->axis[i] != NULL)
            {
                jsmap_ptr->axis[i]->op_code = JSMAP_AXIS_OP_THRUST_DIR;
            }
        }

	/* Viewscreen zoom axis number. */
        i = PromptGetI(&jsmap_win.vs_zoom_axis_prompt);
        if((i >= 0) && (i < jsmap_ptr->total_axises))
        {
            if(jsmap_ptr->axis[i] != NULL)
            {
                jsmap_ptr->axis[i]->op_code = JSMAP_AXIS_OP_VS_ZOOM;
            }
        }

	/* Scanner zoom axis number. */
        i = PromptGetI(&jsmap_win.scanner_zoom_axis_prompt);
        if((i >= 0) && (i < jsmap_ptr->total_axises))
        {
            if(jsmap_ptr->axis[i] != NULL)
            {
                jsmap_ptr->axis[i]->op_code = JSMAP_AXIS_OP_SCANNER_ZOOM;
            }
        }

        /* Aim weapon heading axis number. */
        i = PromptGetI(&jsmap_win.aim_weapon_heading_prompt);
        if((i >= 0) && (i < jsmap_ptr->total_axises))
        {
            if(jsmap_ptr->axis[i] != NULL)
            {
                jsmap_ptr->axis[i]->op_code = JSMAP_AXIS_OP_AIM_WEAPON_HEADING;
            }
        }


	/* Apply button values. */
	list = &jsmap_win.buttons_list;
	for(i = 0; i < list->total_rows; i++)
	{
	    if(i >= jsmap_ptr->total_buttons)
		break;

	    jsmap_button_ptr = jsmap_ptr->button[i];
	    if(jsmap_button_ptr == NULL)
		continue;

	    /* Get keycode value from item's client data pointer
	     * value.
	     */
	    jsmap_button_ptr->keycode = (keycode_t)CListGetItemDataPtr(
		list,
		i, 1	/* Row, colum. */
	    );
	}

	/* Update device name. */
	free(jsmap_ptr->device_name);
	jsmap_ptr->device_name = StringCopyAlloc(
	    PromptGetS(&jsmap_win.device_prompt)
	);


	/* Update device colum in joystick list. */
	list = &jsmap_win.js_list;
	CListSetItemLabel(
	    list,
	    n,
	    1,
	    jsmap_ptr->device_name
	);


        return(0);
}


/*
 *	Append a new jsmap.
 */
int JSMWAddPBCB(void *ptr)
{
	int i, n;
	colum_list_struct *list;
	jsmap_struct *jsmap_ptr;


	/* Get pointer to joystick list widget. */
	list = &jsmap_win.js_list;

	/* Apply selected joystick values as needed. */
	if(list->total_sel_rows > 0)
	{
	    i = list->sel_row[list->total_sel_rows - 1];
	    JSMWDoApplyJoystick(i);
	}


	/* Allocate a new jsmap structure. */
	if(total_jsmaps < 0)
	    total_jsmaps = 0;

	n = total_jsmaps;
	total_jsmaps++;
	jsmap = (jsmap_struct **)realloc(
	    jsmap,
	    total_jsmaps * sizeof(jsmap_struct *)
	);
	if(jsmap == NULL)
	{
	    total_jsmaps = 0;
	    return(-1);
	}

	jsmap[n] = (jsmap_struct *)calloc(
	    1,
	    sizeof(jsmap_struct)
	);

	jsmap_ptr = jsmap[n];
	if(jsmap_ptr == NULL)
	    return(-1);


	/* **************************************************** */
	/* Reset jsmap entry. */

	jsmap_ptr->jsd.fd = -1;

	/* Set default device name (specified from js wrapper. */
	jsmap_ptr->device_name = StringCopyAlloc(JSDefaultDevice);



	/* Reget joystick map listings and reset widget values. */
	JSMWLoadAll();

	/* Get new joystick values. */
	JSMWDoSelectJoystick(n);

	/* Select joystick on joystick list widget. */
	CListSelectRow(list, n);

        return(0);
}

/*
 *	Remove (delete) selected jsmap.
 */
int JSMWRemovePBCB(void *ptr)
{
        int i, n;
        colum_list_struct *list;


        /* Get pointer to joystick list widget. */
        list = &jsmap_win.js_list;
 
	/* No need to apply values since it's going to be removed. */


	/* Delete selected jsmap entry. */
	if(list->total_sel_rows > 0)
	{
	    i = list->sel_row[list->total_sel_rows - 1];
	    if((i >= 0) && (i < total_jsmaps))
	    {
		/* Deallocate resources and delete this joystick
		 * map structure.
		 */
		JSMapDelete(i);

		/* Shift and reallocate pointers. */
		for(n = i; n < (total_jsmaps - 1); n++)
		    jsmap[n] = jsmap[n + 1];

		total_jsmaps--;
		if(total_jsmaps > 0)
		{
		    jsmap = (jsmap_struct **)realloc(
			jsmap,
			total_jsmaps * sizeof(jsmap_struct *)
		    );
		    if(jsmap == NULL)
		    {
			total_jsmaps = 0;
			return(-1);
		    }
		}
		else
		{
		    free(jsmap);
		    jsmap = NULL;

		    total_jsmaps = 0;
		}
	    }
	}

        /* Reget joystick map listings and reset widget values. */
        JSMWLoadAll();


        return(0);
}

/*
 *	Refresh button callback (this button used to be named `initialize').
 */
int JSMWInitPBCB(void *ptr)
{
        int i, status;
        colum_list_struct *list;
        jsmap_struct *jsmap_ptr;
        char text[PATH_MAX + NAME_MAX + 80];


        /* Get pointer to joystick list widget. */
        list = &jsmap_win.js_list;

	/* Get selected joystick row number on list. */
	if(list->total_sel_rows > 0)
	{
	    i = list->sel_row[list->total_sel_rows - 1];
	    if((i < 0) || (i >= total_jsmaps))
		return(-1);
	    if(jsmap[i] == NULL)
		return(-1);
	}
	else
	{
	    return(-1);
	}

	/* Get pointer to joystick map structure. */
	jsmap_ptr = jsmap[i];


	/* Apply values to the jsmap first. */
	JSMWDoApplyJoystick(i);

	/* Close the joystick device as needed. */
	JSClose(&jsmap_ptr->jsd);

	/* Initialize the joystick device. */
	status = JSInit(
	    &jsmap_ptr->jsd,
	    jsmap_ptr->device_name,	/* Device name. */
	    fname.js_calib,		/* Calibration file name. */
	    JSFlagNonBlocking		/* Flags. */
	);
	if(status != JSSuccess)
	{
	    JSClose(&jsmap_ptr->jsd);
	    sprintf(text,
"Cannot initialize joystick:\n\n\
    %s",
		((jsmap_ptr->device_name == NULL) ? "(null)" :
		    jsmap_ptr->device_name
		)
	    );
	    printdw(&err_dw, text);
	    return(-1);
	}

	/* Sync jsmap structure data with that of its jsd. */
	JSMapSyncWithData(jsmap_ptr);

	/* Select joystick again (this is to fetch any changed values). */
	JSMWDoSelectJoystick(i);


	return(0);
}


/*
 *	Callback handler for the joystick list.
 */
int JSMWJoystickListCB(void *ptr)
{


        return(0);
}

/*
 *	Callback handler for the buttons list.
 */
int JSMWButtonsListCB(void *ptr)
{
	JSMWScanKeyPBCB(&jsmap_win.scan_key_btn);

        return(0);
}

/*
 *	Callback handler for the scan key button.
 */
int JSMWScanKeyPBCB(void *ptr)
{
	win_attr_t wattr;

	OSWGetWindowAttributes(jsmap_win.toplevel, &wattr);

	OSWMoveResizeWindow(
	    jsmap_win.scan_key_win,
	    0,
	    ((int)wattr.height / 2) - (JSMW_SCANKEY_WIN_HEIGHT / 2),
	    wattr.width,
	    JSMW_SCANKEY_WIN_HEIGHT
	);

        OSWMapRaised(jsmap_win.scan_key_win);

	jsmap_win.scanning_key = True;


        return(0);
}

/*
 *	Callback handler for the OK button.
 */
int JSMWOKPBCB(void *ptr)
{
	JSMWApplyAll();

	JSMWUnmap();


        return(0);
}

/*
 *	Callback handler for the apply button.
 */
int JSMWApplyPBCB(void *ptr)
{
        JSMWApplyAll();

        return(0);
}

/*
 *	Callback handler for the cancel button.
 */
int JSMWCancelPBCB(void *ptr)
{
        JSMWUnmap();

        return(0);
}


/*
 *	Initializes the joystick map window.
 */
int JSMWInit()
{
	int y;
        pixmap_t pixmap;
        win_attr_t wattr;
        char title[256];
        font_t *font;
        pixel_t pixel;


        if(!IDC())
            return(-1);

        /* Reset values. */
        jsmap_win.map_state = 0;
        jsmap_win.x = 0;
        jsmap_win.y = 0;
        jsmap_win.width = JSMW_DEF_WIDTH;
        jsmap_win.height = JSMW_DEF_HEIGHT;
        jsmap_win.is_in_focus = 0;
        jsmap_win.visibility_state = VisibilityFullyObscured;
        jsmap_win.disabled = False;
        jsmap_win.scanning_key = False;


        /* Create toplevel window. */
        if(
            OSWCreateWindow(
                &jsmap_win.toplevel,
                osw_gui[0].root_win,
                jsmap_win.x,
                jsmap_win.y,
                jsmap_win.width,
                jsmap_win.height
            )
        )
            return(-1);

	jsmap_win.toplevel_buf = 0;

        OSWSetWindowInput(jsmap_win.toplevel, OSW_EVENTMASK_TOPLEVEL);
        WidgetCenterWindow(jsmap_win.toplevel, WidgetCenterWindowToRoot);
        OSWGetWindowAttributes(jsmap_win.toplevel, &wattr);
        jsmap_win.x = wattr.x;
        jsmap_win.y = wattr.y;
        jsmap_win.width = wattr.width;
        jsmap_win.height = wattr.height;

        /* WM properties. */
        sprintf(title, "%s: Joystick Mappings", PROG_NAME);
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
            jsmap_win.toplevel,
            title,			/* Title. */
            "Joystick Mappings",	/* Icon title. */
            pixmap,			/* Icon. */
            False,			/* Let WM set coordinates? */
            /* Coordinates. */
            jsmap_win.x, jsmap_win.y,
            /* Min width and height. */
            jsmap_win.width, jsmap_win.height,
            /* Max width and height. */
            jsmap_win.width, jsmap_win.height,
            WindowFrameStyleFixed,
            NULL, 0
        );

	/* Scan key prompt window. */
        if(
            OSWCreateWindow(
                &jsmap_win.scan_key_win,
                jsmap_win.toplevel,
                JSMW_MARGIN,
                ((int)jsmap_win.height / 2) -
                    (JSMW_SCANKEY_WIN_HEIGHT / 2),
                MAX(
                    (int)jsmap_win.width - (2 * JSMW_MARGIN),
                    JSMW_MARGIN
                ),
                JSMW_SCANKEY_WIN_HEIGHT
            )
        )
            return(-1);
        OSWSetWindowInput(
	    jsmap_win.scan_key_win,
	    ButtonPressMask | ExposureMask
	);


	/* Joystick list. */
        if(
            CListInit(
                &jsmap_win.js_list,
                jsmap_win.toplevel,
                JSMW_MARGIN,
                JSMW_MARGIN,
                MAX(
		    (int)jsmap_win.width - (3 * JSMW_MARGIN) -
			JSMW_BTN_WIDTH,
		    10
		),
                JSMW_JS_LIST_HEIGHT,
                &jsmap_win.js_list,
                JSMWJoystickListCB
            )
        )
            return(-1);

        jsmap_win.js_list.option = 0;
        font = OSWQueryCurrentFont();
        pixel = osw_gui[0].black_pix;
        CListAddHeading(
            &jsmap_win.js_list,
            "Joystick Number",
            font, pixel, 0,
            0			/* x position. */
        );
        CListAddHeading(
            &jsmap_win.js_list,
            "Device",
            font, pixel, 0,
            130			/* x position. */
        );

        /* Add button. */
        if(
            PBtnInit(
                &jsmap_win.add_btn,
                jsmap_win.toplevel,
                (int)jsmap_win.width - JSMW_BTN_WIDTH - JSMW_MARGIN,
		(1 + JSMW_MARGIN) + (0 * JSMW_BTN_HEIGHT),
		JSMW_BTN_WIDTH, JSMW_BTN_HEIGHT,
                "Add",
                PBTN_TALIGN_CENTER,
                NULL,
                &jsmap_win.add_btn,
                JSMWAddPBCB
            )
        )
            return(-1);

        /* Remove button. */
        if(
            PBtnInit(
                &jsmap_win.remove_btn,
                jsmap_win.toplevel,
                (int)jsmap_win.width - JSMW_BTN_WIDTH - JSMW_MARGIN,
                (2 * JSMW_MARGIN) + (1 * JSMW_BTN_HEIGHT),
                JSMW_BTN_WIDTH, JSMW_BTN_HEIGHT,
                "Remove",
                PBTN_TALIGN_CENTER,
                NULL,
                &jsmap_win.add_btn,
                JSMWRemovePBCB    
            )
        )
            return(-1);

	/* Device prompt. */
	if(
	    PromptInit(
                &jsmap_win.device_prompt,
                jsmap_win.toplevel,
                JSMW_MARGIN,
		(2 * JSMW_MARGIN) + JSMW_JS_LIST_HEIGHT,
                MAX(
                    (int)jsmap_win.width - (3 * JSMW_MARGIN) -
                        JSMW_BTN_WIDTH,
                    10
                ),
		JSMW_PROMPT_HEIGHT,
                PROMPT_STYLE_FLUSHED,
                "Device:",
                PATH_MAX + NAME_MAX,
		0,
		NULL
	    )
        )
	    return(-1);

        /* Initialize button. */
        if(
            PBtnInit(
                &jsmap_win.initialize_btn,
                jsmap_win.toplevel,
                (int)jsmap_win.width - JSMW_BTN_WIDTH - JSMW_MARGIN,
                (2 * JSMW_MARGIN) + JSMW_JS_LIST_HEIGHT,
                JSMW_BTN_WIDTH, JSMW_BTN_HEIGHT,
                "Refresh",
                PBTN_TALIGN_CENTER,
                NULL,
                &jsmap_win.initialize_btn,
                JSMWInitPBCB
            )
        )
            return(-1);

	/* Axis prompts. */
	y = (2 * JSMW_MARGIN) + JSMW_JS_LIST_HEIGHT +
	    JSMW_PROMPT_HEIGHT + (2 * JSMW_MARGIN);

        if(
            PromptInit(
                &jsmap_win.turn_axis_prompt,
                jsmap_win.toplevel,
                JSMW_MARGIN,
                y,
		120,
                JSMW_PROMPT_HEIGHT,
                PROMPT_STYLE_FLUSHED,
                "Turn Axis:",
                64,
                0,
                NULL
            )
        )
            return(-1);

        if(
            PromptInit(
                &jsmap_win.throttle_axis_prompt,
                jsmap_win.toplevel,
                JSMW_MARGIN + 120 + JSMW_MARGIN,
                y,
                150,
                JSMW_PROMPT_HEIGHT,  
                PROMPT_STYLE_FLUSHED,
                "Throttle Axis:",
                64,
                0,  
                NULL
            )
        )
            return(-1);

        y += (JSMW_PROMPT_HEIGHT + (JSMW_MARGIN / 2));
        if(
            PromptInit(
                &jsmap_win.thrust_dir_axis_prompt,
                jsmap_win.toplevel,
                JSMW_MARGIN,
                y,
                210,
                JSMW_PROMPT_HEIGHT,
                PROMPT_STYLE_FLUSHED,
                "Thrust Direction Axis:",
                64,
                0,
                NULL
            )
        )
            return(-1);

	y += (JSMW_PROMPT_HEIGHT + (JSMW_MARGIN / 2));
        if( 
            PromptInit(
                &jsmap_win.vs_zoom_axis_prompt,
                jsmap_win.toplevel,
                JSMW_MARGIN,
                y,
                170,
                JSMW_PROMPT_HEIGHT,
                PROMPT_STYLE_FLUSHED,
                "Viewscreen Axis:",
                64,
                0, 
                NULL
            )
        )
            return(-1);
 
        if(
            PromptInit(
                &jsmap_win.scanner_zoom_axis_prompt,
                jsmap_win.toplevel,
                JSMW_MARGIN + 170 + JSMW_MARGIN,
                y,
                150,
                JSMW_PROMPT_HEIGHT,  
                PROMPT_STYLE_FLUSHED,
                "Scanner Axis:",
                64,
                0,  
                NULL
            )
        )
            return(-1);

        y += (JSMW_PROMPT_HEIGHT + (JSMW_MARGIN / 2));
        if(
            PromptInit(
                &jsmap_win.aim_weapon_heading_prompt,
                jsmap_win.toplevel,
                JSMW_MARGIN,
                y,
                210,
                JSMW_PROMPT_HEIGHT,
                PROMPT_STYLE_FLUSHED,
                "Aim Weapon Heading Axis:",
                64,
                0,
                NULL
            )
        )
            return(-1);


	/* Link prompts togeather. */
	jsmap_win.device_prompt.next = &jsmap_win.turn_axis_prompt;
        jsmap_win.device_prompt.prev = &jsmap_win.aim_weapon_heading_prompt;

        jsmap_win.turn_axis_prompt.next = &jsmap_win.throttle_axis_prompt;
        jsmap_win.turn_axis_prompt.prev = &jsmap_win.device_prompt;

	jsmap_win.throttle_axis_prompt.next = &jsmap_win.thrust_dir_axis_prompt;
        jsmap_win.throttle_axis_prompt.prev = &jsmap_win.turn_axis_prompt;

	jsmap_win.thrust_dir_axis_prompt.next = &jsmap_win.vs_zoom_axis_prompt;
        jsmap_win.thrust_dir_axis_prompt.prev = &jsmap_win.throttle_axis_prompt;

        jsmap_win.vs_zoom_axis_prompt.next = &jsmap_win.scanner_zoom_axis_prompt;
        jsmap_win.vs_zoom_axis_prompt.prev = &jsmap_win.thrust_dir_axis_prompt;

        jsmap_win.scanner_zoom_axis_prompt.next = &jsmap_win.aim_weapon_heading_prompt;
        jsmap_win.scanner_zoom_axis_prompt.prev = &jsmap_win.vs_zoom_axis_prompt;

        jsmap_win.aim_weapon_heading_prompt.next = &jsmap_win.device_prompt;
        jsmap_win.aim_weapon_heading_prompt.prev = &jsmap_win.scanner_zoom_axis_prompt;

	/* Buttons list. */
        if(
            CListInit(
                &jsmap_win.buttons_list,
                jsmap_win.toplevel,
                JSMW_MARGIN,
		(int)jsmap_win.height - (4 * JSMW_MARGIN) -
		    JSMW_BTN_HEIGHT - JSMW_BUTTON_LIST_HEIGHT,
                MAX(
                    (int)jsmap_win.width - (3 * JSMW_MARGIN) -
                        JSMW_BTN_WIDTH,
                    10
                ),
                JSMW_BUTTON_LIST_HEIGHT,
                &jsmap_win.buttons_list,
                JSMWButtonsListCB
            )
        )
            return(-1);

        jsmap_win.buttons_list.option = 0;
        font = OSWQueryCurrentFont();
        pixel = osw_gui[0].black_pix;
        CListAddHeading(
            &jsmap_win.buttons_list,
            "Button Number",
            font, pixel, 0,
            0                   /* x position. */
        );
        CListAddHeading(
            &jsmap_win.buttons_list,
            "Key",
            font, pixel, 0,
            180			/* x position. */
        );

        /* Scan key button. */
        if(
            PBtnInit(
                &jsmap_win.scan_key_btn,
                jsmap_win.toplevel,
                (int)jsmap_win.width - JSMW_BTN_WIDTH - JSMW_MARGIN,
                300,
                JSMW_BTN_WIDTH, JSMW_BTN_HEIGHT,
                "Scan Key",
                PBTN_TALIGN_CENTER,
                NULL,
                &jsmap_win.scan_key_btn,
                JSMWScanKeyPBCB
            )
        )
            return(-1);






        /* OK button. */   
        if(
            PBtnInit(
                &jsmap_win.ok_btn,
                jsmap_win.toplevel,
                JSMW_MARGIN,
		(int)jsmap_win.height - JSMW_MARGIN - JSMW_BTN_HEIGHT,
                JSMW_BTN_WIDTH, JSMW_BTN_HEIGHT,
                "OK",  
                PBTN_TALIGN_CENTER,
                NULL,
                &jsmap_win.ok_btn,
                JSMWOKPBCB
            )
        )
            return(-1);

        /* Apply button. */
        if(
            PBtnInit(
                &jsmap_win.apply_btn,
                jsmap_win.toplevel,
                ((int)jsmap_win.width / 2) - (JSMW_BTN_WIDTH / 2),
                (int)jsmap_win.height - JSMW_MARGIN - JSMW_BTN_HEIGHT,
                JSMW_BTN_WIDTH, JSMW_BTN_HEIGHT,
                "Apply",
                PBTN_TALIGN_CENTER,
                NULL,
                &jsmap_win.apply_btn,
                JSMWApplyPBCB
            )
        )
            return(-1);

        /* Cancel button. */
        if(
            PBtnInit(
                &jsmap_win.cancel_btn,
                jsmap_win.toplevel,     
                (int)jsmap_win.width - JSMW_BTN_WIDTH
		    - JSMW_MARGIN,
                (int)jsmap_win.height - JSMW_MARGIN - JSMW_BTN_HEIGHT,
                JSMW_BTN_WIDTH, JSMW_BTN_HEIGHT,
                "Cancel",
                PBTN_TALIGN_CENTER,
                NULL,
                &jsmap_win.cancel_btn,
                JSMWCancelPBCB
            )
        )
            return(-1);


        return(0);
}

void JSMWResize()
{


        return;
}

/*
 *	Redraws the joystick map window.
 */
int JSMWDraw(int amount)
{
	int y;
	win_t w;
	pixmap_t pixmap;

	win_attr_t wattr;


	/* Map as needed. */
	if(!jsmap_win.map_state)
	{
	    jsmap_win.map_state = 1;

	    OSWMapRaised(jsmap_win.toplevel);

            CListMap(&jsmap_win.js_list);

            PBtnMap(&jsmap_win.add_btn);   
            PBtnMap(&jsmap_win.remove_btn);

            PromptMap(&jsmap_win.device_prompt);
	    PBtnMap(&jsmap_win.initialize_btn);

            PromptMap(&jsmap_win.turn_axis_prompt); 
            PromptMap(&jsmap_win.throttle_axis_prompt);
            PromptMap(&jsmap_win.thrust_dir_axis_prompt);
            PromptMap(&jsmap_win.vs_zoom_axis_prompt);
            PromptMap(&jsmap_win.scanner_zoom_axis_prompt);
            PromptMap(&jsmap_win.aim_weapon_heading_prompt);

            CListMap(&jsmap_win.buttons_list);

            PBtnMap(&jsmap_win.scan_key_btn);
            jsmap_win.scanning_key = False;
            OSWUnmapWindow(jsmap_win.scan_key_win);

            PBtnMap(&jsmap_win.ok_btn);
            PBtnMap(&jsmap_win.apply_btn);
            PBtnMap(&jsmap_win.cancel_btn);

	    amount = JSMW_DRAW_AMOUNT_COMPLETE; 
	}


	/* Recreate buffers as needed. */
	if(jsmap_win.toplevel_buf == 0)
	{
	    OSWGetWindowAttributes(jsmap_win.toplevel, &wattr);
	    if(
		OSWCreatePixmap(
		    &jsmap_win.toplevel_buf,
		    wattr.width, wattr.height
		)
	    )
		return(-1);
	}


	/* Toplevel. */
        if(amount == JSMW_DRAW_AMOUNT_COMPLETE)
	{
            w = jsmap_win.toplevel;
            pixmap = jsmap_win.toplevel_buf;

            OSWGetWindowAttributes(w, &wattr);

            /* Redraw background. */
            if(widget_global.force_mono)
            {
                OSWClearPixmap(
                    pixmap,
                    wattr.width, wattr.height,
                    osw_gui[0].black_pix
                );
            }
            else
            {
                WidgetPutImageTile(
                    pixmap,
                    widget_global.std_bkg_img,
                    wattr.width, wattr.height
                );

                OSWSetFgPix(widget_global.surface_shadow_pix);
		y = (int)wattr.height - JSMW_BTN_HEIGHT - 25;
                OSWDrawLine(
                    pixmap,
                    0,
                    y,
                    wattr.width,
                    y
                );
                OSWSetFgPix(widget_global.surface_highlight_pix);
		y = (int)wattr.height - JSMW_BTN_HEIGHT - 24;
                OSWDrawLine(
                    pixmap,
                    0,
                    y,
                    wattr.width,
                    y
                );
            }
            OSWPutBufferToWindow(w, pixmap);
	}

	/* Scan key window. */
	if((amount == JSMW_DRAW_AMOUNT_COMPLETE) ||
           (amount == JSMW_DRAW_AMOUNT_SCANKEY)
	)
	{
	    if(jsmap_win.scanning_key)
	    {
		w = jsmap_win.scan_key_win;
		pixmap = jsmap_win.toplevel_buf;

		OSWGetWindowAttributes(w, &wattr);


		OSWClearPixmap(pixmap,
                    wattr.width, wattr.height,
                    (widget_global.force_mono) ?
                        osw_gui[0].white_pix :
                        xsw_color.keymap_query_bg
                );
                if(widget_global.force_mono)
                    OSWSetFgPix(osw_gui[0].black_pix);
                else
                    OSWSetFgPix(xsw_color.keymap_query_fg);

                OSWDrawString(
                    pixmap,
                    JSMW_MARGIN + 5,
                    JSMW_MARGIN + (0 * 16) + (14 / 2) + 5,
                    "Press a key on the keyboard to set the keycode."
                );
                OSWDrawString(
                    pixmap,
                    JSMW_MARGIN + 5,
                    JSMW_MARGIN + (2 * 16) + (14 / 2) + 5,
                    "Press Button1 to cancel."
                );
                OSWDrawString(
                    pixmap,
                    JSMW_MARGIN + 5,
                    JSMW_MARGIN + (3 * 16) + (14 / 2) + 5,
                    "Press Button3 to clear."
                );

                OSWPutBufferToWindow(w, pixmap);
	    }
	}


        return(0);
}

/*
 *	Manages the joystick map window.
 */
int JSMWManage(event_t *event)
{
	int i, n, p;
	colum_list_struct *list;
	keycode_t keycode;
	int events_handled = 0;


	if(event == NULL)
	    return(events_handled);

	if(!jsmap_win.map_state &&
           (event->type != MapNotify)
	)
	    return(events_handled);


	switch(event->type)
	{
	  case KeyPress:
	    if(!jsmap_win.is_in_focus)
		return(events_handled);

	    keycode = event->xkey.keycode;

	    /* Scanning for keyboard input? */
	    if(jsmap_win.scanning_key)
	    {
		list = &jsmap_win.buttons_list;
		if(list->total_sel_rows > 0)
		{
		    i = list->sel_row[list->total_sel_rows - 1];
		    CListSetItemLabel(
			list,
			i, 1,
			OSWGetKeyCodeName(keycode)
		    );
		    CListSetItemDataPtr(
			list,
			i, 1,
			(void *)keycode
		    );
		}

		jsmap_win.scanning_key = False;
		OSWUnmapWindow(jsmap_win.scan_key_win);

                events_handled++;
                return(events_handled);
	    }
	    /* Enter. */
	    else if((keycode == osw_keycode.enter) ||
                    (keycode == osw_keycode.np_enter)
	    )
	    {
		JSMWInitPBCB(&jsmap_win.initialize_btn);

                events_handled++;
                return(events_handled);
	    }
            /* Escape. */ 
            else if(keycode == osw_keycode.esc)
            {
                JSMWCancelPBCB(&jsmap_win.cancel_btn);

                events_handled++;
                return(events_handled);
            }
	    break;

          case KeyRelease:
            if(!jsmap_win.is_in_focus)
                return(events_handled);
          
            keycode = event->xkey.keycode;

          
 
            break;

	  case ButtonPress:
            if(jsmap_win.scanning_key)
            {
		/* Unmap scanning key prompt window. */
                jsmap_win.scanning_key = False;
                OSWUnmapWindow(jsmap_win.scan_key_win);


                list = &jsmap_win.buttons_list;
                if(list->total_sel_rows > 0)
                {
                    i = list->sel_row[list->total_sel_rows - 1];

		    /* Check which button was pressed. */
		    switch(event->xbutton.button)
		    {
		      case Button3:	/* Clear. */
			keycode = 0;
			CListSetItemLabel(
			    list,
			    i, 1,	/* Row, colum. */
                            OSWGetKeyCodeName(keycode)
                        );
			CListSetItemDataPtr(
                            list,
                            i, 1,	/* Row, colum. */
			    (void *)keycode
			);
			break;

		       default:
			break;
		    }
		}
            }
	    break;

          case ButtonRelease:
            break;

          case Expose:
            if(event->xany.window == jsmap_win.toplevel)
            {
		JSMWDraw(JSMW_DRAW_AMOUNT_COMPLETE);

                events_handled++;
		return(events_handled);
	    }
            if(event->xany.window == jsmap_win.scan_key_win)
            {
                JSMWDraw(JSMW_DRAW_AMOUNT_SCANKEY);

                events_handled++;
                return(events_handled);
            }
            break;

          case UnmapNotify:
            if(event->xany.window == jsmap_win.toplevel)
            {
                JSMWUnmap();

                events_handled++;
                return(events_handled);
            }
            break;

          case MapNotify:
            if(event->xany.window == jsmap_win.toplevel)
            {
                if(!jsmap_win.map_state)
                    JSMWMap();

                events_handled++;
                return(events_handled);
            }
            break;

          case VisibilityNotify:
            if(event->xany.window == jsmap_win.toplevel)
            {
                jsmap_win.visibility_state =
                    event->xvisibility.state;

                events_handled++;
                return(events_handled);
            }
            break;

          case ConfigureNotify:
            if(event->xany.window == jsmap_win.toplevel)
            {
		JSMWResize();

                events_handled++;
                return(events_handled);
            }
            break;
 
          case FocusIn:
            if(event->xany.window == jsmap_win.toplevel)
            {
                jsmap_win.is_in_focus = 1;
                events_handled++;
            }
            break;

          case FocusOut:
            if(event->xany.window == jsmap_win.toplevel)
            {
                jsmap_win.is_in_focus = 0;
                events_handled++;
            }
            break;

          case ClientMessage:
            if(OSWIsEventDestroyWindow(jsmap_win.toplevel, event))
            {
                JSMWCancelPBCB(&jsmap_win.cancel_btn);

                events_handled++;
                return(events_handled);
            }
            break;
	}


	if(events_handled == 0)
	{
	    list = &jsmap_win.js_list;
	    if(list->total_sel_rows > 0)
	        p = list->sel_row[list->total_sel_rows - 1];
	    else
		p = -1;

            events_handled += CListManage(&jsmap_win.js_list, event);
	    if(events_handled > 0)
	    {
		if(list->total_sel_rows > 0)
                    n = list->sel_row[list->total_sel_rows - 1];
                else
                    n = -1;
		if(p != n)
		{
		    JSMWDoApplyJoystick(p);
		    JSMWDoSelectJoystick(n);
		}
	    }
	}

        if(events_handled == 0)
            events_handled += PBtnManage(&jsmap_win.add_btn, event);

        if(events_handled == 0)
            events_handled += PBtnManage(&jsmap_win.remove_btn, event);

        if(events_handled == 0)
            events_handled += PromptManage(&jsmap_win.device_prompt, event);

	if(events_handled == 0)
	    events_handled += PBtnManage(&jsmap_win.initialize_btn, event);


        if(events_handled == 0)
            events_handled += PromptManage(&jsmap_win.turn_axis_prompt, event);

        if(events_handled == 0)
            events_handled += PromptManage(&jsmap_win.throttle_axis_prompt, event);

        if(events_handled == 0)
            events_handled += PromptManage(&jsmap_win.thrust_dir_axis_prompt, event);

        if(events_handled == 0)
            events_handled += PromptManage(&jsmap_win.vs_zoom_axis_prompt, event);

        if(events_handled == 0)
            events_handled += PromptManage(&jsmap_win.scanner_zoom_axis_prompt, event);

        if(events_handled == 0)
            events_handled += PromptManage(&jsmap_win.aim_weapon_heading_prompt, event);

        if(events_handled == 0)
            events_handled += CListManage(&jsmap_win.buttons_list, event);


        if(events_handled == 0)
            events_handled += PBtnManage(&jsmap_win.scan_key_btn, event);

        if(events_handled == 0)
            events_handled += PBtnManage(&jsmap_win.ok_btn, event);

        if(events_handled == 0)
            events_handled += PBtnManage(&jsmap_win.apply_btn, event);

        if(events_handled == 0)
            events_handled += PBtnManage(&jsmap_win.cancel_btn, event);


        return(events_handled);
}

/*
 *	Map joystick map window.
 */
void JSMWMap()
{
	XSWDoUnfocusAllWindows();

	jsmap_win.map_state = 0;
	JSMWDraw(JSMW_DRAW_AMOUNT_COMPLETE);
	jsmap_win.is_in_focus = 1;

        /* Restack all XSW windows. */
        XSWDoRestackWindows();


        return;
}

/*
 *	Procedure to map joystick map window and fetch values.
 */
void JSMWMapValues()
{
	/* Make sure global controller is set to joystick. */
	if(option.controller != CONTROLLER_JOYSTICK)
	{
	    printdw(
		&err_dw,
"You must set the controller type to `joystick' before you\n\
can edit the joystick mappings. Go to options->general and\n\
set the controller type to `joystick' then click on `apply'.\n"
	    );
	    return;
	}

        /* Reget joystick map listings and reset widget values. */
	JSMWLoadAll();

	/* Map joystick map window. */
	JSMWMap();


        return;   
}

/*
 *	Unmaps joystick map window.
 */
void JSMWUnmap()
{       
	jsmap_win.map_state = 0;
	jsmap_win.is_in_focus = 0;

	CListUnmap(&jsmap_win.js_list);

        PBtnUnmap(&jsmap_win.add_btn);   
        PBtnUnmap(&jsmap_win.remove_btn);

        PromptUnmap(&jsmap_win.device_prompt);
	PBtnUnmap(&jsmap_win.initialize_btn);

        PromptUnmap(&jsmap_win.turn_axis_prompt);
        PromptUnmap(&jsmap_win.throttle_axis_prompt);
	PromptUnmap(&jsmap_win.thrust_dir_axis_prompt);
        PromptUnmap(&jsmap_win.vs_zoom_axis_prompt);
        PromptUnmap(&jsmap_win.scanner_zoom_axis_prompt);
        PromptUnmap(&jsmap_win.aim_weapon_heading_prompt);

        CListUnmap(&jsmap_win.buttons_list);

        PBtnUnmap(&jsmap_win.scan_key_btn);
        jsmap_win.scanning_key = False;
        OSWUnmapWindow(jsmap_win.scan_key_win);

        PBtnUnmap(&jsmap_win.ok_btn);
        PBtnUnmap(&jsmap_win.apply_btn);
        PBtnUnmap(&jsmap_win.cancel_btn);

        OSWUnmapWindow(jsmap_win.toplevel);


        OSWDestroyPixmap(&jsmap_win.toplevel_buf);
        
        
        return;   
}       

/*
 *	Destroys joystick map window.
 */
void JSMWDestroy()
{
        jsmap_win.map_state = 0;
        jsmap_win.is_in_focus = 0;

	if(IDC())
	{
	    CListDestroy(&jsmap_win.js_list);

	    PBtnDestroy(&jsmap_win.add_btn);
            PBtnDestroy(&jsmap_win.remove_btn);

	    PromptDestroy(&jsmap_win.device_prompt);
	    PBtnDestroy(&jsmap_win.initialize_btn);

	    PromptDestroy(&jsmap_win.turn_axis_prompt);
            PromptDestroy(&jsmap_win.throttle_axis_prompt);
	    PromptDestroy(&jsmap_win.thrust_dir_axis_prompt);
            PromptDestroy(&jsmap_win.vs_zoom_axis_prompt);
            PromptDestroy(&jsmap_win.scanner_zoom_axis_prompt);
            PromptDestroy(&jsmap_win.aim_weapon_heading_prompt);

	    CListDestroy(&jsmap_win.buttons_list);

	    PBtnDestroy(&jsmap_win.scan_key_btn);
	    jsmap_win.scanning_key = False;
	    OSWDestroyWindow(&jsmap_win.scan_key_win);

            PBtnDestroy(&jsmap_win.ok_btn);
            PBtnDestroy(&jsmap_win.apply_btn);
            PBtnDestroy(&jsmap_win.cancel_btn); 

	    OSWDestroyPixmap(&jsmap_win.toplevel_buf);
            OSWDestroyWindow(&jsmap_win.toplevel);
	}


        return;   
}       

#endif	/* JS_SUPPORT */
