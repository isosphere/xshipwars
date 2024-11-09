/*
                                Keymap Window

	Functions:

	int KeymapWinAppendListRow(
	        colum_list_struct *list,
	        const char *name,
	        keycode_t keycode
	)
	int KeymapWinGetKeycode(
		colum_list_struct *list,
		int row_num,
		keycode_t *keycode
	)
	int KeymapWinSetKeycode(
		colum_list_struct *list,
		int row_num,
		keycode_t keycode
	)

	int KeymapWinLoad()
	int KeymapWinApply()
	int KeymapWinSetDefault(int item_num)

	void KeymapWinPromptKey(void)
	int KeymapWinCheckDups(bool_t warn)

	int KeymapWinListCB(void *ptr)
	int KeymapWinOkPBCB(void *ptr)
        int KeymapWinCancelPBCB(void *ptr)
        int KeymapWinApplyPBCB(void *ptr)
	int KeymapWinScanKeyPBCB(void *ptr)
        int KeymapWinDefaultPBCB(void *ptr)
        int KeymapWinDefaultAllPBCB(void *ptr)

	int KeymapWinInit()
	void KeymapWinResize()
	int KeymapWinDraw(int amount)
	int KeymapWinManage(event_t *event)
	void KeymapWinMap()
	void KeymapWinDoMapValues()
	void KeymapWinUnmap()
	void KeymapWinDestroy()




 */

#include "keymapwin.h"
#include "keymap.h"
#include "xsw.h"


#define MIN(a,b)	(((a) < (b)) ? (a) : (b))
#define MAX(a,b)	(((a) > (b)) ? (a) : (b))


/* 
 *      Keymap window margins (in pixels): 
 */
#define KEYMAP_WIN_MARGIN               10      /* All purpose margin. */
#define KEYMAP_WIN_LIST_TO_END_MARGIN   180


/*
 *	Keymap window sizes (in pixels):
 */
#define KEYMAP_WIN_DEFAULT_WIDTH	400
#define KEYMAP_WIN_DEFAULT_HEIGHT	512

#define KEYMAP_WIN_MIN_WIDTH		380
#define KEYMAP_WIN_MIN_HEIGHT		350

#define KEYMAP_WIN_ITEM_HEIGHT		30
#define KEYMAP_WIN_HINTS_HEIGHT		80
#define KEYMAP_WIN_PROMPT_HEIGHT	((5 * 16) + \
                                        (2 * KEYMAP_WIN_MARGIN))

#define KEYMAP_WIN_BTN_WIDTH		70
#define KEYMAP_WIN_BTN_HEIGHT		28

#define KMW_DEF_LIST_KEYCODE_POS	240


/*
 *	Draw amounts:
 */
#define KMW_DRAW_AMOUNT_COMPLETE	0
#define KMW_DRAW_AMOUNT_LIST		1
#define KMW_DRAW_AMOUNT_DESC		2
#define KMW_DRAW_AMOUNT_PROMPT		3


/*
 *	Keymap character sizes:
 */
#define KEYMAP_WIN_CHAR_WIDTH	7
#define KEYMAP_WIN_CHAR_HEIGHT	14


/*
 *	Keymap list draw positions (in pixels):
 */
#define KEYMAP_WIN_LIST_POS_NAME	KEYMAP_WIN_MARGIN
#define KEYMAP_WIN_LIST_POS_KEYCODE	240


xsw_keymap_win_struct keymap_win;
xsw_keymap_struct xsw_keymap[TOTAL_XSW_KEYMAPS];



/*
 *	Appends a new keymap row item.
 *	Called by function KeymapWinLoad().
 */
int KeymapWinAppendListRow(
	colum_list_struct *list,
	const char *name,
	keycode_t keycode
)
{
	int i;
	font_t *font;
	pixel_t pixel;


	/* Append row. */
	if(CListAddRow(list, -1))
	    return(-1);

	/* Get newly appended row number. */
	i = MAX(list->total_rows - 1, 0);

	font = OSWQueryCurrentFont();
	pixel = widget_global.editable_text_pix;

	/* Name item to row (colum 0). */
	if(
	    CListAddItem(
	        list, name,
	        font, pixel, 0,
	        i
	    )
	)
	    return(-1);

	/* Add keycode to row (colum 1). */
	if(
            CListAddItem(
                list, OSWGetKeyCodeName(keycode),
	        font, pixel, 0,
                i
            )
	)
	    return(-1);

	/* Set client data ptr on item to be the value of the
	 * keycode.
	 */
	CListSetItemDataPtr(
	    list,
	    i, 1,		/* Row, colum. */
	    (void *)keycode
	);

	return(0);
}


/*
 *	Macro to fetch keycode value from the data pointer on
 *	the list widget item and set it to the keycode variable
 *	pointed to by keycode.
 */
int KeymapWinGetKeycode(
	colum_list_struct *list,
        int row_num,
	keycode_t *keycode
)
{
        if((list == NULL) ||
           (keycode == NULL)
        )
            return(-1);

	*keycode = (keycode_t)CListGetItemDataPtr(
	    list,
	    row_num, 1 		/* Row, colum. */
	);

	return(0);
}

/*
 *	Sets keycode on row_num to the value of keycode.
 *	Changes the label and client data pointer on the item.
 */
int KeymapWinSetKeycode(
        colum_list_struct *list,
        int row_num,
        keycode_t keycode
)
{
	if(keycode < 0)
	    return(-1);

	/* Set new label. */
	CListSetItemLabel(
	    list,
	    row_num, 1,		/* Row, colum. */
	    OSWGetKeyCodeName(keycode)
	);
	/* Set new data pointer to be the value (not point to)
	 * of the keycode.
	 */
	CListSetItemDataPtr(
	    list,
	    row_num, 1,		/* Row, colum. */
	    (void *)keycode
	);

	return(0);
}


/*
 *	Loads global keymap values into keymap window's colum list.
 *
 *	Any existing keymap entries will be deleted first.
 */
int KeymapWinLoad()
{
	int i;
	colum_list_struct *list;
	const char *xsw_keymap_alias[] = XSW_KEYMAP_ALIAS;


	/* Get pointer to colum list. */
	list = &keymap_win.list;
	if(list == NULL)
	    return(-1);

	/* Delete all existing keymap rows. */
	CListDeleteAllRows(list);

	/* Add keymaps to list. */
	for(i = 0; i < (int)TOTAL_XSW_KEYMAPS; i++)
	    KeymapWinAppendListRow(
	        list,
	        xsw_keymap_alias[i],	/* Use alias name. */
		xsw_keymap[i].keycode
	    );

	return(0);
}


/*
 *	Sets global keymap values to that of the local
 *	keymap values of the keymap window.
 */
int KeymapWinApply()
{
	int i;
        colum_list_struct *list;


        list = &keymap_win.list;
        if(list == NULL)
            return(-1);

        for(i = 0; i < (int)TOTAL_XSW_KEYMAPS; i++)
	    KeymapWinGetKeycode(
		list,
	        i,
		&(xsw_keymap[i].keycode)
	    );

        return(0);
}



/*
 *	Sets the default keycode value for keymap item item_num.
 */
int KeymapWinSetDefault(int item_num)
{
        colum_list_struct *list;   


        /* Get pointer to colum list. */
        list = &keymap_win.list;
        if(list == NULL)
            return(-1);

	/* Set default keycode for keymap item. */
	switch(item_num)
	{
	  case XSW_KM_HELP:
            KeymapWinSetKeycode(list, item_num,
                osw_keycode.f1
            );
	    break;

          case XSW_KM_EXIT:
            KeymapWinSetKeycode(list, item_num,
                osw_keycode.alpha_x
	    );
            break;

	  case XSW_KM_TURN_LEFT:
            KeymapWinSetKeycode(list, item_num,
		osw_keycode.cursor_left
            );
            break;

          case XSW_KM_TURN_RIGHT:
            KeymapWinSetKeycode(list, item_num,
                osw_keycode.cursor_right
            );
            break;

          case XSW_KM_THROTTLE_INC:
            KeymapWinSetKeycode(list, item_num,
                osw_keycode.cursor_up
            );
            break;

          case XSW_KM_THROTTLE_DEC:
            KeymapWinSetKeycode(list, item_num,
                osw_keycode.cursor_down
            );
            break;

	  case XSW_KM_THROTTLE_IDLE:
            KeymapWinSetKeycode(list, item_num,
                osw_keycode.ddelete
            );
            break;

          case XSW_KM_FIRE_WEAPON:
            KeymapWinSetKeycode(list, item_num,
                osw_keycode.space
            );
            break;

          case XSW_KM_OMNI_DIR_THRUST:
            KeymapWinSetKeycode(list, item_num,
                osw_keycode.alt_left
            );
            break;

          case XSW_KM_EXTERNAL_DAMPERS:
            KeymapWinSetKeycode(list, item_num,
                osw_keycode.alpha_b
            );
            break;


          case XSW_KM_LIGHTS_VECTOR:
            KeymapWinSetKeycode(list, item_num,
                osw_keycode.alpha_l
            );
            break;

          case XSW_KM_LIGHTS_STROBE:
            KeymapWinSetKeycode(list, item_num,
                osw_keycode.alpha_p
            );
            break;

          case XSW_KM_LIGHTS_LUMINATION:
            KeymapWinSetKeycode(list, item_num,
                osw_keycode.alpha_v
            );
            break;


          case XSW_KM_WEAPON_FREQ:
            KeymapWinSetKeycode(list, item_num,
                osw_keycode.alpha_y
            );
            break;

	  case XSW_KM_SHIELD_STATE:
            KeymapWinSetKeycode(list, item_num,
                osw_keycode.alpha_s
            );
            break;

	  case XSW_KM_SHIELD_FREQ:
            KeymapWinSetKeycode(list, item_num,
                osw_keycode.alpha_f
            );
            break;

	  case XSW_KM_DMGCTL_TOGGLE:
            KeymapWinSetKeycode(list, item_num,
                osw_keycode.alpha_c
            );
            break;

          case XSW_KM_CLOAK_STATE:
            KeymapWinSetKeycode(list, item_num,
                osw_keycode.alpha_z
            );
            break;

          case XSW_KM_SET_INTERCEPT:
            KeymapWinSetKeycode(list, item_num,
                osw_keycode.alpha_i
            );
            break;

          case XSW_KM_HAIL:
            KeymapWinSetKeycode(list, item_num,
                osw_keycode.alpha_h
            );
            break;

          case XSW_KM_SET_CHANNEL:
            KeymapWinSetKeycode(list, item_num,
                osw_keycode.alpha_k
            );
            break;


          case XSW_KM_VIEWSCREEN_ZIN:
            KeymapWinSetKeycode(list, item_num,
                osw_keycode.equal
            );
            break;

          case XSW_KM_VIEWSCREEN_ZOUT:
            KeymapWinSetKeycode(list, item_num,
                osw_keycode.minus
            );
            break;

          case XSW_KM_VIEWSCREEN_ZAUTO:
            KeymapWinSetKeycode(list, item_num,
                osw_keycode.backslash
            );
            break;


          case XSW_KM_SCANNER_TOGGLE:
            KeymapWinSetKeycode(list, item_num,
                osw_keycode.alpha_t
            );
            break;

          case XSW_KM_SCANNER_ZIN:
            KeymapWinSetKeycode(list, item_num,
                osw_keycode.period
            );
            break;

          case XSW_KM_SCANNER_ZOUT:
            KeymapWinSetKeycode(list, item_num,
                osw_keycode.comma
            );
            break;

          case XSW_KM_SCANNER_ZMIN:
            KeymapWinSetKeycode(list, item_num,
                osw_keycode.alpha_n
            );
            break;

          case XSW_KM_SCANNER_ZMAX:
            KeymapWinSetKeycode(list, item_num,
                osw_keycode.alpha_o
            );
            break;

          case XSW_KM_SCANNER_ORIENT:
            KeymapWinSetKeycode(list, item_num,
                osw_keycode.alpha_g
            );
            break;


          case XSW_KM_ENGINE_STATE:
            KeymapWinSetKeycode(list, item_num,
                osw_keycode.tilde
            );
            break;

          case XSW_KM_THROTTLE_MODE:
            KeymapWinSetKeycode(list, item_num,
                osw_keycode.f8
            );
            break;


          case XSW_KM_WEAPONS_LOCK:
            KeymapWinSetKeycode(list, item_num,
                osw_keycode.tab
            );
            break;

          case XSW_KM_WEAPONS_UNLOCK:
            KeymapWinSetKeycode(list, item_num,
                osw_keycode.alpha_u
            );
            break;


          case XSW_KM_WEAPONS_ONLINE:
            KeymapWinSetKeycode(list, item_num,
                osw_keycode.alpha_w
            );
            break;

	  case XSW_KM_SELECT_WEAPONPREV:
            KeymapWinSetKeycode(list, item_num,
                osw_keycode.alpha_q
            );
            break;

          case XSW_KM_SELECT_WEAPONNEXT:
            KeymapWinSetKeycode(list, item_num,
                osw_keycode.alpha_a
            );
            break;

          case XSW_KM_SELECT_WEAPON1:
            KeymapWinSetKeycode(list, item_num,
                osw_keycode.num_1
            );
            break;

          case XSW_KM_SELECT_WEAPON2:
            KeymapWinSetKeycode(list, item_num,
                osw_keycode.num_2
            );
            break;

          case XSW_KM_SELECT_WEAPON3:
            KeymapWinSetKeycode(list, item_num,
                osw_keycode.num_3
            );
            break;

          case XSW_KM_SELECT_WEAPON4:
            KeymapWinSetKeycode(list, item_num,
                osw_keycode.num_4
            );
            break;

          case XSW_KM_SELECT_WEAPON5:
            KeymapWinSetKeycode(list, item_num,
                osw_keycode.num_5
            );
            break;

          case XSW_KM_SELECT_WEAPON6:
            KeymapWinSetKeycode(list, item_num,
                osw_keycode.num_6
            );
            break;

          case XSW_KM_SELECT_WEAPON7:
            KeymapWinSetKeycode(list, item_num,
                osw_keycode.num_7
            );
            break;

          case XSW_KM_SELECT_WEAPON8:
            KeymapWinSetKeycode(list, item_num,
                osw_keycode.num_8
            );
            break;

          case XSW_KM_SELECT_WEAPON9:
            KeymapWinSetKeycode(list, item_num,
                osw_keycode.num_9
            );
            break;


          case XSW_KM_SEND_MESSAGE:
            KeymapWinSetKeycode(list, item_num,
                osw_keycode.quote
            );
            break;


          case XSW_KM_VIEWSCREEN_MARKINGS:
            KeymapWinSetKeycode(list, item_num,
                osw_keycode.f2
            );
            break;

          case XSW_KM_VIEWSCREEN_LABELS:
            KeymapWinSetKeycode(list, item_num,
                osw_keycode.f3
            );
            break;

          case XSW_KM_ENERGY_SAVER_MODE:
            KeymapWinSetKeycode(list, item_num,
                osw_keycode.f4
            );
            break;


          case XSW_KM_NET_INTERVAL_DEC:
            KeymapWinSetKeycode(list, item_num,
                osw_keycode.brace_left
            );
            break;

          case XSW_KM_NET_INTERVAL_INC:
            KeymapWinSetKeycode(list, item_num,
                osw_keycode.brace_right
            );
            break;


          case XSW_KM_MESG_SCROLL_UP:
            KeymapWinSetKeycode(list, item_num,
                osw_keycode.page_up
            );
            break;

          case XSW_KM_MESG_SCROLL_DOWN:
            KeymapWinSetKeycode(list, item_num,
                osw_keycode.page_down
            );
            break;


          case XSW_KM_MAP_ECONOMY:
            KeymapWinSetKeycode(list, item_num,
                osw_keycode.semicolon
            );
            break;

	  case XSW_KM_MAP_STARCHART:
            KeymapWinSetKeycode(list, item_num,
                osw_keycode.alpha_m
            );
            break;


          case XSW_KM_CONNECT:
            KeymapWinSetKeycode(list, item_num,
                osw_keycode.f9
            );
            break;

          case XSW_KM_DISCONNECT:
            KeymapWinSetKeycode(list, item_num,
                osw_keycode.f10
            );
            break;

          case XSW_KM_REFRESH:
            KeymapWinSetKeycode(list, item_num,
                osw_keycode.f11
            );
            break;

          case XSW_KM_CONNECTLAST:
            KeymapWinSetKeycode(list, item_num,
                osw_keycode.f12
            );
            break;


          case XSW_KM_CLIENT_CMD:
            KeymapWinSetKeycode(list, item_num,
                osw_keycode.slash
            );
            break;

          case XSW_KM_SERVER_CMD:
            KeymapWinSetKeycode(list, item_num,
                osw_keycode.alpha_e
            );
            break;


          case XSW_KM_SCREEN_SHOT:
            KeymapWinSetKeycode(list, item_num,
                osw_keycode.print_screen
            );
            break;
	}


	return(0);
}


/*
 *	Maps the key scan prompt window and sets everything up for
 *	scanning a key.
 */
void KeymapWinPromptKey()
{
	/* Set up query. */
	keymap_win.key_prompt_mode = True;

	/* Map keyprompt window. */
	OSWMapRaised(keymap_win.key_prompt_win);


	return;
}



/*
 *	Checks key maps (set in keymap win items, not globals) for
 *	any duplicates, warns and returns number of duplicates.
 */
int KeymapWinCheckDups(bool_t warn)
{
	int i, n, dups_found;
	void *keycode1, *keycode2;
	colum_list_struct *list;

	char stringa[256];


        /* Get pointer to colum list. */
        list = &keymap_win.list;
        if(list == NULL)
            return(-1);

	/* Go through each row on the list. */
	for(i = 0, dups_found = 0;
            i < list->total_rows;
            i++
	)
	{
	    /* Get keycode. */
	    keycode1 = CListGetItemDataPtr(
		list,
		i, 1	/* Row, colum. */
	    );

	    /* Skip unset keycodes. */
	    if(keycode1 == NULL)
		continue;


	    /* Check for duplicates. */
	    for(n = 0; n < list->total_rows; n++)
	    {
		/* Skip current row i. */
		if(n == i)
		    continue;

		/* Get keycode. */
		keycode2 = CListGetItemDataPtr(
		    list,
		    n, 1	/* Row, colum. */
		);

		/* Check if both items have the same keycode. */
		if(keycode1 == keycode2)
		{
		    dups_found++;

		    /* Print warning. */
		    if(warn)
		    {
			sprintf(stringa,
"Warning: Potential keymap conflict.\n\n\
    %s and %s\n\
    are mapped to the same key `%s'.\n\n",
			    xsw_keymap_name[n],
			    xsw_keymap_name[i],
			    OSWGetKeyCodeName((keycode_t)keycode1)
			);

			printdw(&err_dw, stringa);
		    }
		}
	    }
	}


	return(dups_found);
}



/*
 *	Keymap list callback.
 */
int KeymapWinListCB(void *ptr)
{
	KeymapWinScanKeyPBCB((void *)&keymap_win.scan_key_btn);


	return(0);
}

/*
 *	`Ok' button callback.
 */
int KeymapWinOkPBCB(void *ptr)
{
	KeymapWinApply();
	KeymapWinUnmap();

	return(0);
}

/*
 *	`Cancel' button callback.
 */
int KeymapWinCancelPBCB(void *ptr)
{
        KeymapWinUnmap();
 
        return(0);
}

/*
 *	`Apply' button callback.
 */
int KeymapWinApplyPBCB(void *ptr)
{
        KeymapWinApply();
	KeymapWinDraw(KMW_DRAW_AMOUNT_PROMPT);

        return(0);
}

/*
 *	`Scan key' button callback.
 */
int KeymapWinScanKeyPBCB(void *ptr)
{
	KeymapWinPromptKey();
	KeymapWinDraw(KMW_DRAW_AMOUNT_COMPLETE);

	return(0);
}

/*
 *	`Default' button callback.
 */
int KeymapWinDefaultPBCB(void *ptr)
{
	int i;

	i = CListGetFirstSelectedRow(&keymap_win.list);
	if(i < 0)
	    return(0);

	KeymapWinSetDefault(i);
	KeymapWinDraw(KMW_DRAW_AMOUNT_COMPLETE);

	return(0);
}

/*
 *	`Default all' button callback.
 */
int KeymapWinDefaultAllPBCB(void *ptr)
{
	int i;


	for(i = 0; i < (int)TOTAL_XSW_KEYMAPS; i++)
	    KeymapWinSetDefault(i);

	KeymapWinDraw(KMW_DRAW_AMOUNT_COMPLETE);


	return(0);
}


/*
 *	Initializes keymap window.
 */
int KeymapWinInit()
{
	pixmap_t pixmap;
	win_attr_t wattr;
	char title[256];
	font_t *font;
	pixel_t pixel;


	/* Reset values. */
	keymap_win.map_state = 0;
	keymap_win.x = 0;
        keymap_win.y = 0;
        keymap_win.width = KEYMAP_WIN_DEFAULT_WIDTH;
        keymap_win.height = KEYMAP_WIN_DEFAULT_HEIGHT;
        keymap_win.is_in_focus = 0;
        keymap_win.visibility_state = VisibilityFullyObscured;
        keymap_win.disabled = False;
	keymap_win.key_prompt_mode = False;


	/* Create toplevel window. */
	if(
	    OSWCreateWindow(
		&keymap_win.toplevel,
		osw_gui[0].root_win,
		keymap_win.x,
		keymap_win.y,
		keymap_win.width,
		keymap_win.height
	    )
	)
	    return(-1);
	keymap_win.toplevel_buf = 0;
        OSWSetWindowInput(keymap_win.toplevel, OSW_EVENTMASK_TOPLEVEL);

        WidgetCenterWindow(keymap_win.toplevel, WidgetCenterWindowToRoot);
	OSWGetWindowAttributes(keymap_win.toplevel, &wattr);
	keymap_win.x = wattr.x;
        keymap_win.y = wattr.y;
        keymap_win.width = wattr.width;
        keymap_win.height = wattr.height;


        /* WM properties. */
        sprintf(title, "%s: Key Mappings", PROG_NAME);
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
            keymap_win.toplevel,
            title,		/* Title. */
            "Key Mappings",	/* Icon title. */
            pixmap,		/* Icon. */
            False,		/* Let WM set coordinates? */
            /* Coordinates. */
            keymap_win.x, keymap_win.y,
            /* Min width and height. */
            keymap_win.width, keymap_win.height,
            /* Max width and height. */
            keymap_win.width, keymap_win.height,
            WindowFrameStyleFixed,
            NULL, 0
        );   


	/* Keymap list. */
        if(
            CListInit(
                &keymap_win.list,
                keymap_win.toplevel,
                KEYMAP_WIN_MARGIN,
                KEYMAP_WIN_MARGIN,
                MAX(keymap_win.width - (2 * KEYMAP_WIN_MARGIN), 10),
                MAX(keymap_win.height - 240, 10),
		(void *)&keymap_win.list,
		KeymapWinListCB
            )
        )
            return(-1);
	keymap_win.list.option = 0;

	font = OSWQueryCurrentFont();
	pixel = osw_gui[0].black_pix;
	CListAddHeading(
	    &keymap_win.list,
            "Name",
	    font, pixel, 0,
	    0
	);
	CListAddHeading(
            &keymap_win.list,
            "Key",
            font, pixel, 0,
            KMW_DEF_LIST_KEYCODE_POS
        );


        /* Entry description window. */
        if(
            OSWCreateWindow(
                &keymap_win.desc,
                keymap_win.toplevel,
                KEYMAP_WIN_MARGIN,
                keymap_win.height - KEYMAP_WIN_HINTS_HEIGHT - 110,
                MAX(keymap_win.width - (2 * KEYMAP_WIN_MARGIN), 1),
                KEYMAP_WIN_HINTS_HEIGHT
            )
        )
            return(-1);
        OSWSetWindowInput(keymap_win.desc, ExposureMask);

	/* Key prompt window. */
        if(     
            OSWCreateWindow(
                &keymap_win.key_prompt_win,
                keymap_win.toplevel,
                KEYMAP_WIN_MARGIN,
                ((int)keymap_win.height / 2) -
                    (KEYMAP_WIN_PROMPT_HEIGHT / 2),
                MAX(
                    (int)keymap_win.width - (2 * KEYMAP_WIN_MARGIN),
                    KEYMAP_WIN_MARGIN
                ),
                KEYMAP_WIN_PROMPT_HEIGHT
            )
        )
            return(-1);
        OSWSetWindowInput(
	    keymap_win.key_prompt_win,
	    ButtonPressMask | ExposureMask
	);
/*
	OSWSetWindowBkg(keymap_win.key_prompt_win,
	    xsw_color.keymap_query_bg,
	    0
	);
*/

	/* OK button. */
	if(
	    PBtnInit(
		&keymap_win.ok_btn,
		keymap_win.toplevel,
		KEYMAP_WIN_MARGIN,
		keymap_win.height - KEYMAP_WIN_MARGIN -
		    KEYMAP_WIN_BTN_HEIGHT,
		KEYMAP_WIN_BTN_WIDTH, KEYMAP_WIN_BTN_HEIGHT,
		"OK",
		PBTN_TALIGN_CENTER,
		NULL,
		(void *)&keymap_win.ok_btn,
		KeymapWinOkPBCB
	    )
	)
	    return(-1);

        /* Cancel button. */
        if(
            PBtnInit(
                &keymap_win.cancel_btn,
                keymap_win.toplevel,
                (int)keymap_win.width - KEYMAP_WIN_MARGIN -
                    KEYMAP_WIN_BTN_WIDTH,
		keymap_win.height - KEYMAP_WIN_MARGIN -
		    KEYMAP_WIN_BTN_HEIGHT,
                KEYMAP_WIN_BTN_WIDTH, KEYMAP_WIN_BTN_HEIGHT,
                "Cancel",
                PBTN_TALIGN_CENTER,
                NULL,
                (void *)&keymap_win.cancel_btn,
                KeymapWinCancelPBCB
            )
        )
            return(-1);

        /* Apply button. */
        if(
            PBtnInit(
                &keymap_win.apply_btn,
                keymap_win.toplevel,
		((int)keymap_win.width / 2) -
		    (KEYMAP_WIN_BTN_WIDTH / 2),
		keymap_win.height - KEYMAP_WIN_MARGIN -
		    KEYMAP_WIN_BTN_HEIGHT,
                KEYMAP_WIN_BTN_WIDTH, KEYMAP_WIN_BTN_HEIGHT,
                "Apply",
                PBTN_TALIGN_CENTER,
                NULL,
                (void *)&keymap_win.apply_btn,
                KeymapWinApplyPBCB
            )
        )
            return(-1);

	/* Key scan button. */
        if(
            PBtnInit(
                &keymap_win.scan_key_btn,
                keymap_win.toplevel,
                KEYMAP_WIN_MARGIN,
                (int)keymap_win.height - (2 * KEYMAP_WIN_BTN_HEIGHT) -
                    (4 * KEYMAP_WIN_MARGIN),
                KEYMAP_WIN_BTN_WIDTH, KEYMAP_WIN_BTN_HEIGHT,
                "Scan Key",
                PBTN_TALIGN_CENTER,
                NULL,
                (void *)&keymap_win.scan_key_btn,
                KeymapWinScanKeyPBCB
            )
        )
            return(-1);


        /* Default button. */
        if(
            PBtnInit(
                &keymap_win.default_btn,
                keymap_win.toplevel,
		(int)keymap_win.width - (2 * KEYMAP_WIN_MARGIN) - 
		    KEYMAP_WIN_BTN_WIDTH - 90,
                (int)keymap_win.height - (2 * KEYMAP_WIN_BTN_HEIGHT) - 
                    (4 * KEYMAP_WIN_MARGIN),
                KEYMAP_WIN_BTN_WIDTH, KEYMAP_WIN_BTN_HEIGHT,
                "Default",
                PBTN_TALIGN_CENTER,
                NULL,
                (void *)&keymap_win.default_btn,
                KeymapWinDefaultPBCB
            )       
        )
            return(-1);

        /* Default all button. */
        if(
            PBtnInit(
                &keymap_win.default_all_btn,
                keymap_win.toplevel,
		(int)keymap_win.width - KEYMAP_WIN_MARGIN -
		    90,
                (int)keymap_win.height - (2 * KEYMAP_WIN_BTN_HEIGHT) -
                    (4 * KEYMAP_WIN_MARGIN),
                90, KEYMAP_WIN_BTN_HEIGHT,
                "Default All",
                PBTN_TALIGN_CENTER,
                NULL,
                (void *)&keymap_win.default_all_btn,
                KeymapWinDefaultAllPBCB
            )
        )
            return(-1);


	return(0);
}


void KeymapWinResize()
{
        win_attr_t wattr;


        /* Get new size and check for change. */
        OSWGetWindowAttributes(keymap_win.toplevel, &wattr);
        if((keymap_win.width == (unsigned int)wattr.width) &&
           (keymap_win.height == (unsigned int)wattr.height)
        )
            return;

	/* Set new size values. */
        keymap_win.x = wattr.x;
        keymap_win.y = wattr.y;
        keymap_win.width = wattr.width;
        keymap_win.height = wattr.height;

 

        /* Toplevel buffer. */
        OSWDestroyPixmap(&keymap_win.toplevel_buf);


        /* List. */
        OSWMoveResizeWindow(
	    keymap_win.list.toplevel,
            KEYMAP_WIN_MARGIN,
            KEYMAP_WIN_MARGIN,
            MAX(keymap_win.width - (2 * KEYMAP_WIN_MARGIN), 1),
            MAX(keymap_win.height - 240, 1)
        );
	CListResize(&keymap_win.list);
            
         
        /* Item description window. */
        OSWDestroyPixmap(&keymap_win.desc_buf);
        OSWMoveResizeWindow(keymap_win.desc,
            KEYMAP_WIN_MARGIN,
            keymap_win.height - KEYMAP_WIN_HINTS_HEIGHT - 110,
            MAX(keymap_win.width - (2 * KEYMAP_WIN_MARGIN), 1),
            KEYMAP_WIN_HINTS_HEIGHT
        );


        /* Key prompt window. */
        OSWDestroyPixmap(&keymap_win.key_prompt_buf);
        OSWMoveResizeWindow(
	    keymap_win.key_prompt_win,
            KEYMAP_WIN_MARGIN,
            ((int)keymap_win.height / 2) -
		(KEYMAP_WIN_PROMPT_HEIGHT / 2),
            MAX(
		(int)keymap_win.width - (2 * KEYMAP_WIN_MARGIN),
		KEYMAP_WIN_MARGIN
	    ),
            KEYMAP_WIN_PROMPT_HEIGHT
        );


        /* OK button. */
        OSWMoveWindow(keymap_win.ok_btn.toplevel,
            KEYMAP_WIN_MARGIN,
            (int)keymap_win.height - KEYMAP_WIN_MARGIN -
                KEYMAP_WIN_BTN_HEIGHT
        );

        /* Cancel button. */
        OSWMoveWindow(keymap_win.cancel_btn.toplevel,
            (int)keymap_win.width - KEYMAP_WIN_MARGIN -
                KEYMAP_WIN_BTN_WIDTH,
            (int)keymap_win.height - KEYMAP_WIN_MARGIN -
                KEYMAP_WIN_BTN_HEIGHT
        );

        /* Apply button. */
        OSWMoveWindow(keymap_win.apply_btn.toplevel,
            ((int)keymap_win.width / 2) -
                (KEYMAP_WIN_BTN_WIDTH / 2),
            (int)keymap_win.height - KEYMAP_WIN_MARGIN -
                KEYMAP_WIN_BTN_HEIGHT
        );

        /* Scan key button. */
        OSWMoveWindow(keymap_win.scan_key_btn.toplevel,
            KEYMAP_WIN_MARGIN,
            (int)keymap_win.height - (2 * KEYMAP_WIN_BTN_HEIGHT) - 
                (4 * KEYMAP_WIN_MARGIN)
        );

        /* Default button. */
        OSWMoveWindow(keymap_win.default_btn.toplevel,
            (int)keymap_win.width - (2 * KEYMAP_WIN_MARGIN) -
                KEYMAP_WIN_BTN_WIDTH - 90,
            (int)keymap_win.height - (2 * KEYMAP_WIN_BTN_HEIGHT) -
                (4 * KEYMAP_WIN_MARGIN)
        );

        /* Default all button. */
        OSWMoveWindow(keymap_win.default_all_btn.toplevel,
            (int)keymap_win.width - 90 - KEYMAP_WIN_MARGIN,
            (int)keymap_win.height - (2 * KEYMAP_WIN_BTN_HEIGHT) -
                (4 * KEYMAP_WIN_MARGIN)
        );


        return;
}


int KeymapWinDraw(int amount)
{
	int item_num;
	const char *strptr, *strptr2;

	int desc_line, desc_vis_lines;

	win_t w;
	pixmap_t pixmap;
	win_attr_t wattr;

	const char *keymap_item_desc[] = XSW_KEYMAP_ITEM_DESC;


	if(!IDC())
	    return(-1);


	/* Map as needed. */
	if(!keymap_win.map_state)
	{
	    /* Map keymap window. */
	    OSWMapRaised(keymap_win.toplevel);
	    OSWMapWindow(keymap_win.desc);

	    keymap_win.map_state = 1;
	    keymap_win.visibility_state = VisibilityUnobscured;

	    CListMap(&keymap_win.list);

	    /* Map buttons. */
	    PBtnMap(&keymap_win.ok_btn);
            PBtnMap(&keymap_win.cancel_btn);
            PBtnMap(&keymap_win.apply_btn);

	    PBtnMap(&keymap_win.scan_key_btn);
            PBtnMap(&keymap_win.default_btn);
            PBtnMap(&keymap_win.default_all_btn);


	    /* Change draw amount to complete. */
	    amount = KMW_DRAW_AMOUNT_COMPLETE;
	}


	/* ******************************************************* */
        /* Recreate buffers as needed. */

	/* Toplevel buffer. */
        if(keymap_win.toplevel_buf == 0)
        {
            OSWGetWindowAttributes(
                keymap_win.toplevel,
                &wattr
            );  
            if(OSWCreatePixmap(&keymap_win.toplevel_buf,
                    wattr.width, wattr.height
               )
            )
                return(-1);
        }
	/* Description buffer. */
        if(keymap_win.desc_buf == 0)
        {
            OSWGetWindowAttributes(
                keymap_win.desc,
                &wattr
            );
            if(OSWCreatePixmap(&keymap_win.desc_buf,
                    wattr.width, wattr.height
               )
            )
                return(-1);
        }
	/* Key prompt buffer. */
        if(keymap_win.key_prompt_buf == 0)
        {
            OSWGetWindowAttributes(
                keymap_win.key_prompt_win,
                &wattr
            );
            if(OSWCreatePixmap(&keymap_win.key_prompt_buf,
                    wattr.width, wattr.height
               )
            )
                return(-1);
        }


	/* ********************************************************* */

	/* Redraw background. */
        if(amount == KMW_DRAW_AMOUNT_COMPLETE)
	{
            w = keymap_win.toplevel;
            pixmap = keymap_win.toplevel_buf;

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
		OSWDrawLine(
		    pixmap,
		    0,
		    (int)wattr.height - KEYMAP_WIN_BTN_HEIGHT - 25,
                    wattr.width,
                    (int)wattr.height - KEYMAP_WIN_BTN_HEIGHT - 25
		);
 
                OSWSetFgPix(widget_global.surface_highlight_pix);
                OSWDrawLine(
                    pixmap,
                    0,
                    (int)wattr.height - KEYMAP_WIN_BTN_HEIGHT - 24,
                    wattr.width,
                    (int)wattr.height - KEYMAP_WIN_BTN_HEIGHT - 24
                );
            }

            OSWPutBufferToWindow(w, pixmap);
	}


	/* Redraw list. */
	if((amount == KMW_DRAW_AMOUNT_COMPLETE) ||
           (amount == KMW_DRAW_AMOUNT_LIST)
	)
	{
	    CListDraw(&keymap_win.list, CL_DRAW_AMOUNT_COMPLETE);
	}


	/* Item description. */
	if((amount == KMW_DRAW_AMOUNT_COMPLETE) ||
           (amount == KMW_DRAW_AMOUNT_DESC)
	)
	{
            OSWGetWindowAttributes(keymap_win.desc, &wattr);
            OSWClearPixmap(keymap_win.desc_buf,
                wattr.width, wattr.height,
                (widget_global.force_mono) ?
		    osw_gui[0].black_pix :
                    widget_global.surface_editable_pix
            );

	    /* Calculate lines visible. */
	    desc_vis_lines = ((int)wattr.height - (2 * KEYMAP_WIN_MARGIN))
		/ KEYMAP_WIN_CHAR_HEIGHT;

	    /* Get selected item number. */
	    item_num = CListGetFirstSelectedRow(&keymap_win.list);
	    if((item_num >= 0) &&
               (item_num < (int)TOTAL_XSW_KEYMAPS)
	    )
	    {
		strptr = keymap_item_desc[item_num];

		if(widget_global.force_mono)
		    OSWSetFgPix(osw_gui[0].white_pix);
		else
		    OSWSetFgPix(widget_global.editable_text_pix);

	        for(desc_line = 0;
		    desc_line < desc_vis_lines;
                    desc_line++
		)
	        {
		    if(strptr == NULL)
			break;
		    if(*strptr == '\0')
			break;

		    strptr2 = strchr(strptr, '\n');
		    if(strptr2 == NULL)
			strptr2 = strchr(strptr, '\r');

		    if(strptr2 == NULL)
		    {
			OSWDrawString(
			    keymap_win.desc_buf,
			    KEYMAP_WIN_MARGIN,
			    (desc_line * KEYMAP_WIN_CHAR_HEIGHT) +
				KEYMAP_WIN_MARGIN + 5,
			    strptr
			);
		    }
		    else
                    {
                        OSWDrawStringLimited(
                            keymap_win.desc_buf,
                            KEYMAP_WIN_MARGIN,
                            (desc_line * KEYMAP_WIN_CHAR_HEIGHT) +
                                KEYMAP_WIN_MARGIN + 5,
                            strptr, (int)(strptr2 - strptr)
                        );
                    }

		    if(strptr2 != NULL)
			strptr = strptr2 + 1;
		    else
                        strptr = NULL;
	        }
	    }


            /* Draw frame. */
            WidgetFrameButtonPixmap(
                keymap_win.desc_buf,
                True,
                wattr.width, wattr.height,
                widget_global.surface_highlight_pix,
                widget_global.surface_shadow_pix
            );

            OSWPutBufferToWindow(keymap_win.desc, keymap_win.desc_buf);
	}


	/* Key prompt window. */
	if(keymap_win.key_prompt_mode &&
           ((amount == KMW_DRAW_AMOUNT_COMPLETE) ||
            (amount == KMW_DRAW_AMOUNT_PROMPT)
	   )
	)
	{
	    OSWGetWindowAttributes(keymap_win.key_prompt_win, &wattr);
	    OSWClearPixmap(keymap_win.key_prompt_buf,
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
                keymap_win.key_prompt_buf,
		KEYMAP_WIN_MARGIN + 5,
		KEYMAP_WIN_MARGIN + (0 * 16) + (14 / 2) + 5,
		"Press a key on the keyboard to set the keycode."
            );
            OSWDrawString(
                keymap_win.key_prompt_buf,
                KEYMAP_WIN_MARGIN + 5,
                KEYMAP_WIN_MARGIN + (2 * 16) + (14 / 2) + 5,
                "Press Button1 to cancel."
            );
            OSWDrawString(
                keymap_win.key_prompt_buf,
                KEYMAP_WIN_MARGIN + 5,
                KEYMAP_WIN_MARGIN + (3 * 16) + (14 / 2) + 5,
                "Press Button2 to set default."
            );
            OSWDrawString(
                keymap_win.key_prompt_buf,
                KEYMAP_WIN_MARGIN + 5,
                KEYMAP_WIN_MARGIN + (4 * 16) + (14 / 2) + 5,
                "Press Button3 to clear."
            );

	    OSWPutBufferToWindow(
		keymap_win.key_prompt_win,
		keymap_win.key_prompt_buf
	    );
	}


	return(0);
}



int KeymapWinManage(event_t *event)
{
	int i, n;
        int events_handled = 0;


        if(event == NULL)
	    return(events_handled);


	if(!keymap_win.map_state &&
           (event->type != MapNotify)
	)
            return(events_handled);


	switch(event->type)
	{
	  /* ******************************************************* */
	  case KeyPress:
	    /* Skip if not in focus. */
	    if(!keymap_win.is_in_focus)
	        break;

	    /* Scan keycode mode? */
            if(keymap_win.key_prompt_mode)
	    {
		i = CListGetFirstSelectedRow(&keymap_win.list);
		if((i >= 0) && (i < (int)TOTAL_XSW_KEYMAPS))
		{
		    KeymapWinSetKeycode(
			&keymap_win.list,
                        i,
                        event->xkey.keycode
		    );
		}

		/* Check and warn about duplicate keycodes. */
		KeymapWinCheckDups(True);

		/* Leave key prompt mode. */
		keymap_win.key_prompt_mode = False;
		OSWUnmapWindow(keymap_win.key_prompt_win);

		events_handled++;
	    }
	    /* Enter key. */
	    else if((event->xkey.keycode == osw_keycode.enter) ||
                    (event->xkey.keycode == osw_keycode.np_enter)
	    )
	    {
		KeymapWinScanKeyPBCB((void *)&keymap_win.scan_key_btn);

		events_handled++;
		return(events_handled);
	    }
	    /* Escape key. */
	    else if(event->xkey.keycode == osw_keycode.esc)
	    {
		/* Reload global keymappings. */
		KeymapWinLoad();

		KeymapWinUnmap();

		events_handled++;
		return(events_handled);
	    }
	    break;

	  /* ********************************************************* */
	  case KeyRelease:
            /* Skip if not in focus. */
            if(!keymap_win.is_in_focus)
		return(events_handled);

	    break;


          /* ********************************************************* */
	  case ButtonPress:

	    /* ButtonPress while in key prompt mode? */
            if(keymap_win.key_prompt_mode)
	    {
		i = CListGetFirstSelectedRow(&keymap_win.list);

		/* Button 1 cancels. */
		if(event->xbutton.button == Button1)
		{
		    keymap_win.key_prompt_mode = False;
                    OSWUnmapWindow(keymap_win.key_prompt_win);
 
		    events_handled++;
		    return(events_handled);
		}
		/* Button2 sets default. */
		else if(event->xbutton.button == Button2)
                {
		    /* Set default. */
		    KeymapWinSetDefault(i);

                    keymap_win.key_prompt_mode = False;
                    OSWUnmapWindow(keymap_win.key_prompt_win);

                    events_handled++;
                    return(events_handled);
                }
                /* Button3 clears current. */
                else if(event->xbutton.button == Button3)
                {
                    KeymapWinSetKeycode(
                        &keymap_win.list,
                        i,
                        0
                    );

                    keymap_win.key_prompt_mode = False;
                    OSWUnmapWindow(keymap_win.key_prompt_win);
                 
                    events_handled++;
                    return(events_handled);
                }
	    }
	    break;

          /* ******************************************************* */
          case ButtonRelease:
        
            break;

          /* ******************************************************* */
          case MotionNotify:

            break;

          /* ******************************************************* */
	  case Expose:
	    if((event->xany.window == keymap_win.toplevel) ||
               (event->xany.window == keymap_win.desc)
	    )
	        events_handled++;
	    break;

          /* ******************************************************* */
          case UnmapNotify:
            if(event->xany.window == keymap_win.toplevel)
	    {
		KeymapWinUnmap();

                events_handled++;
                return(events_handled);
	    }
            break;

          /* ******************************************************* */
          case MapNotify:
            if(event->xany.window == keymap_win.toplevel)
	    {
		if(!keymap_win.map_state)
                    KeymapWinMap();

                events_handled++;
		return(events_handled);
	    }
            break;

          /* ******************************************************* */
          case VisibilityNotify:
            if(event->xany.window == keymap_win.toplevel)
            {
                keymap_win.visibility_state =
                    event->xvisibility.state;
            
                events_handled++;
                
                /* No need to continue, just return. */
                return(events_handled);
            }
            break;

          /* ********************************************************* */
          case ConfigureNotify:
            if(event->xany.window == keymap_win.toplevel)
            {
                KeymapWinResize();

                events_handled++;
		return(events_handled);
            }
            break;

          /* ******************************************************* */
          case FocusIn:
            if(event->xany.window == keymap_win.toplevel)
            {
                keymap_win.is_in_focus = 1;                

                events_handled++;
		return(events_handled);
            }
            break;

          /* ******************************************************* */
          case FocusOut:
            if(event->xany.window == keymap_win.toplevel)
            {
                keymap_win.is_in_focus = 0;

                events_handled++;
		return(events_handled);
	    }
            break;

          /* ******************************************************* */
          case ClientMessage:
            if(OSWIsEventDestroyWindow(keymap_win.toplevel, event))
            {
                /* Unmap keymap window. */
                KeymapWinUnmap();

                events_handled++;
                return(events_handled);
            }
            break; 
	}

	/* Redraw as needed. */
	if(events_handled > 0)
	{
	    KeymapWinDraw(KMW_DRAW_AMOUNT_COMPLETE);
	}



	/* ********************************************************** */
	/* Manage widgets. */

	/* Colum list widget. */
        if(events_handled == 0)
        {
	    i = CListGetFirstSelectedRow(&keymap_win.list);

            events_handled += CListManage(
		&keymap_win.list,
		event
	    );

	    n = CListGetFirstSelectedRow(&keymap_win.list);
	    if(i != n)
	    {
		KeymapWinDraw(KMW_DRAW_AMOUNT_DESC);
	    }
	}

	/* OK button. */
        if(events_handled == 0)  
        {
            events_handled += PBtnManage(
                &keymap_win.ok_btn,
		event
            );
        }

	/* Cancel button. */
        if(events_handled == 0)
        {
            events_handled += PBtnManage(
                &keymap_win.cancel_btn,
                event
            );
        }

        /* Apply button. */ 
        if(events_handled == 0)
        {
            events_handled += PBtnManage(
                &keymap_win.apply_btn,
                event
            );
        }

        /* Scan key button. */
        if(events_handled == 0)
        {
            events_handled += PBtnManage(
                &keymap_win.scan_key_btn,
                event
            );
	    if(events_handled > 0)
		KeymapWinDraw(KMW_DRAW_AMOUNT_PROMPT);
        }

        /* Default button. */ 
        if(events_handled == 0)
        {
            events_handled += PBtnManage(
                &keymap_win.default_btn,
                event
            );
        }

        /* Default all button. */ 
        if(events_handled == 0)
        {
            events_handled += PBtnManage(
                &keymap_win.default_all_btn,
                event
            );
        }


	return(events_handled);
}


void KeymapWinMap()
{
        XSWDoUnfocusAllWindows();

        keymap_win.map_state = 0;
        KeymapWinDraw(KMW_DRAW_AMOUNT_COMPLETE);   
        keymap_win.is_in_focus = 1;


        /* Restack all XSW windows. */
        XSWDoRestackWindows();


        return;
}


void KeymapWinDoMapValues()
{
        /* Delete old listing and load new values. */
        KeymapWinLoad();

	/* Map. */
	KeymapWinMap();

	return;
}


void KeymapWinUnmap()
{
	PBtnUnmap(&keymap_win.ok_btn);
        PBtnUnmap(&keymap_win.cancel_btn);
        PBtnUnmap(&keymap_win.apply_btn);
        PBtnUnmap(&keymap_win.scan_key_btn);
        PBtnUnmap(&keymap_win.default_btn);
        PBtnUnmap(&keymap_win.default_all_btn);
	CListUnmap(&keymap_win.list);

	OSWUnmapWindow(keymap_win.toplevel);
	keymap_win.is_in_focus = 0;
	keymap_win.map_state = 0;


	/* Destroy buffers. */
        OSWDestroyPixmap(&keymap_win.key_prompt_buf);
        OSWDestroyPixmap(&keymap_win.desc_buf);
	OSWDestroyPixmap(&keymap_win.toplevel_buf);


	return;
}


void KeymapWinDestroy()
{
	if(IDC())
	{
	    PBtnDestroy(&keymap_win.ok_btn);
	    PBtnDestroy(&keymap_win.cancel_btn);
	    PBtnDestroy(&keymap_win.apply_btn);

	    PBtnDestroy(&keymap_win.scan_key_btn);
	    PBtnDestroy(&keymap_win.default_btn);
	    PBtnDestroy(&keymap_win.default_all_btn);

	    OSWDestroyWindow(&keymap_win.key_prompt_win);
	    OSWDestroyPixmap(&keymap_win.key_prompt_buf);

	    CListDestroy(&keymap_win.list);

	    OSWDestroyWindow(&keymap_win.desc);
	    OSWDestroyPixmap(&keymap_win.desc_buf);

	    OSWDestroyWindow(&keymap_win.toplevel);
	    OSWDestroyPixmap(&keymap_win.toplevel_buf);
	}


	/* Reset values. */
	keymap_win.map_state = 0;
        keymap_win.x = 0;
        keymap_win.y = 0;
        keymap_win.width = 0;
        keymap_win.height = 0;
	keymap_win.is_in_focus = 0;
        keymap_win.visibility_state = VisibilityFullyObscured;
	keymap_win.disabled = False;
        keymap_win.key_prompt_mode = False;


	return;
}
