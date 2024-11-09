/*
                    Bridge Window: Event Management

 	Functions:

	int BW_IS_IN_BTNPOS(int btnpos_num, int x, int y)
        void BridgeDoQuery(
                prompt_window_struct *prompt,
                char *label,
                char *val,
                int mode
        )
	void BridgeMessagesMark(
	        int start_x, int start_y,
	        int end_x, int end_y
	)
	void BridgePrintSubjectStats(int object_num)
	void BridgeWarnWeaponsOffline(int object_num)
	int BridgeManagePromptExec(event_t *event)
	int BridgeManage(event_t *event)

	---

 */

#include "keymap.h"

#include "xsw.h"  
#include "net.h"
#include "../include/swsoundcodes.h"


#define RADIANS_TO_DEGREES(r) ((r) * 180 / PI)

#define MIN(a,b)        (((a) < (b)) ? (a) : (b))
#define MAX(a,b)        (((a) > (b)) ? (a) : (b))


/*
 *      Bridge window message box character sizes:
 */
#define BRIDGE_MESG_CHAR_WIDTH  7
#define BRIDGE_MESG_CHAR_HEIGHT 14


/*
 *	Local pointer drag records:
 */
static int	sel_start_x,
		sel_start_y;

static bool_t	button1_state;
/*
		button2_state,
		button3_state;
 */


void BridgeDoQuery(
	prompt_window_struct *prompt,
	char *label,
	char *val,
	int mode
);



/*
 *      Checks if button press coordinates x and y are within
 *      the bounds of button press positions specified in
 *      structure referanced by btnpos_num.
 */
int BW_IS_IN_BTNPOS(int btnpos_num, int x, int y)
{
        bpanel_btnpos_struct *ptr;

        if((btnpos_num < 0) ||
           (btnpos_num >= total_bpanel_btnpos) ||
           (bpanel_btnpos == NULL)
        )
            return(0);

        ptr = bpanel_btnpos[btnpos_num];
        if(ptr == NULL)
            return(0);

        if((x >= ptr->x) &&
           (y >= ptr->y) &&
           (x < (ptr->x + (int)ptr->width)) &&
           (y < (ptr->y + (int)ptr->height))
        )
            return(1);
        else
            return(0);
}

/*
 *      Procedure to set prompt buffer, change prompt's name,
 *      set global prompt_mode and map prompt.
 */
void BridgeDoQuery(
        prompt_window_struct *prompt,
        char *label,
        char *val,
        int mode
)
{
        if(prompt == NULL)
            return;

        if(label != NULL)
            PromptChangeName(prompt, label);

        if(val != NULL)
            PromptSetS(prompt, val);

        prompt_mode = mode;

        PromptMarkBuffer(prompt, PROMPT_POS_END);

        PromptMap(prompt);
        prompt->is_in_focus = 1;


        return;
}

/*
 *      Marks messages by the given rectangular coordinates.
 *
 *	Scroll bar positions will be automatically applied.
 */
void BridgeMessagesMark(
        int start_x, int start_y,
        int end_x, int end_y
)
{
        int i, len;
        int x, y;  
        int start_line, end_line;
        int delta_lines;


        /* Swap start and end as needed. */
        if(((start_x + 1) * (start_y + 1)) >
           ((end_x + 1) * (end_y + 1))
        )
        {
            x = start_x;
            y = start_y;

            start_x = end_x;
            start_y = end_y;

            end_x = x;
            end_y = y;
        }


        /* Get starting line. */
	start_line = (start_y - 10 + bridge_win.mesg_box_sb.y_win_pos) /
	    (int)bridge_win.line_spacing;
        if(start_line >= MESG_WIN_TOTAL_MESSAGES)
            start_line = MESG_WIN_TOTAL_MESSAGES - 1;
        if(start_line < 0)
            start_line = 0;


        /* Calculate end line. */
        end_line = (end_y - 10 + bridge_win.mesg_box_sb.y_win_pos) /
	    (int)bridge_win.line_spacing; 
        if(end_line >= MESG_WIN_TOTAL_MESSAGES)
            end_line = MESG_WIN_TOTAL_MESSAGES - 1;
        if(end_line < 0)
            end_line = 0;


        /* Calculate delta lines. */
        delta_lines = end_line - start_line;


        /* ***************************************************** */
        /*   Begin marking (backwards).
         *
         *   Remember that start_line is equal or greater than end_line.
         */

        /* Single line mark? */
        if(start_line == end_line)
        {
            len = strlen(pri_mesg_buf[start_line].message);

            x = (start_x - 10) / BRIDGE_MESG_CHAR_WIDTH;
            if(x >= len) x = len - 1;
            if(x < 0) x = 0;
            pri_mesg_buf[start_line].sel_start = x;

            x = (end_x - 10) / BRIDGE_MESG_CHAR_WIDTH;
            if(x >= len) x = len - 1;
            if(x < 0) x = 0;
            pri_mesg_buf[start_line].sel_end = x;
        }
        else if(start_line < end_line)
        {
            /* Start line. */
            x = (start_x - 10) / BRIDGE_MESG_CHAR_WIDTH;
            len = strlen(pri_mesg_buf[start_line].message);
            if(x >= len) x = len - 1;
            if(x < 0) x = 0;

            pri_mesg_buf[start_line].sel_start = x;
            pri_mesg_buf[start_line].sel_end = len - 1;

            /* Mark lines in between. */
            for(i = start_line + 1; i < end_line; i++)
            {
                pri_mesg_buf[i].sel_start = 0;
                pri_mesg_buf[i].sel_end = MAX(
		    strlen(pri_mesg_buf[i].message) - 1,
		    0
		);
            }

            /* End line. */
            x = (end_x - 10) / BRIDGE_MESG_CHAR_WIDTH;
            len = strlen(pri_mesg_buf[end_line].message);  
            if(x >= len) x = len - 1;
            if(x < 0) x = 0;

            pri_mesg_buf[end_line].sel_start = 0;
            pri_mesg_buf[end_line].sel_end = x;
        }

        return;
}

/*
 *	Prints stats of the subject object locked on scanner.
 */
void BridgePrintSubjectStats(int object_num)
{
	int i;
	double size;
	xsw_object_struct *obj_ptr;
	xsw_weapons_struct *wep_ptr;
	char *area_type_text;
	char text[256];
	char size_text[80];


	if(DBIsObjectGarbage(object_num))
	    return;
	else
	    obj_ptr = xsw_object[object_num];

	if(obj_ptr->type == XSW_OBJ_TYPE_AREA)
	{
	    size = obj_ptr->size;	/* Size is in Screen units. */
	    switch(option.units)
            {
              case XSW_UNITS_ENGLISH:
                size = ConvertRUToAU(&sw_units, size / 1000);
		sprintf(size_text, "%.4f au", size);
                break;

              case XSW_UNITS_METRIC:
                size = ConvertRUToAU(&sw_units, size / 1000);
                sprintf(size_text, "%.4f au", size);
                break;

              default:
                sprintf(size_text, "%.0f su", size);
                break;
            }

	    switch(obj_ptr->loc_type)
	    {
	      case XSW_LOC_TYPE_NEBULA:
		area_type_text = XSW_LOC_TYPE_NAME_NEBULA;
		break;

	      case XSW_LOC_TYPE_NOTIFY:
		area_type_text = XSW_LOC_TYPE_NAME_NOTIFY;
                break;

              default:	/* Assume to be space. */
                area_type_text = XSW_LOC_TYPE_NAME_SPACE;
                break;
	    }

	    sprintf(text,
 "%s:  Area Type: %s  Size: %s",
	        obj_ptr->name,
		area_type_text,
		size_text
	    );
	}
	else
        {
            sprintf(text,
 "%s: Density: %.2f  Energy: %.2f  Visibility: %.0f%%",
                obj_ptr->name,
                obj_ptr->hp,
                obj_ptr->power,
                DBGetObjectVisibilityPtr(obj_ptr) * 100
            );
        }
	MesgAdd(text, xsw_color.standard_text);


	for(i = 0; i < obj_ptr->total_weapons; i++)
	{
	    wep_ptr = obj_ptr->weapons[i];
	    if(wep_ptr == NULL)
		continue;

	    switch(wep_ptr->emission_type)
            {
              case WEPEMISSION_PULSE:
                sprintf(text,
                    "%s: Power Yield: %.2f  Power Consumption: %.2f",
                    wep_ptr->name,
                    wep_ptr->power,
		    wep_ptr->create_power
                );
		break;

              case WEPEMISSION_PROJECTILE:
                sprintf(text,
                    "%s: Power Yield: %.2f  Stock: %d(%d)",
                    wep_ptr->name,
                    wep_ptr->power,
		    wep_ptr->amount, wep_ptr->max
                );
                break;

              default:	/* WEPEMISSION_STREAM */
                sprintf(text,
             "%s: Unit Power Yield: %.2f  Unit Power Consumption: %.2f",
                    wep_ptr->name,
                    wep_ptr->power,
		    wep_ptr->create_power
                );
                break;
	    }

            MesgAdd(text, xsw_color.standard_text);
	}


	return;
}

/*
 *	Prints warning about weapons being offline and plays
 *	appropriate sound.
 */
void BridgeWarnWeaponsOffline(int object_num)
{
	char text[256];

	sprintf(text,
"Weapons are offline, press \"%s\" to bring weapons online.",
	    OSWGetKeyCodeName(xsw_keymap[XSW_KM_WEAPONS_ONLINE].keycode)
	);
	MesgAdd(text, xsw_color.standard_text);

	SoundPlay(
            SOUND_CODE_STD_ERROR,
            1.0, 1.0,	/* Left and right volume. */
	    0, 0	/* Effects and prioritu */
	);

	return;
}

/*
 *	Checks if event is for the bridge prompt
 *	and if so, performs an execution based on the contents
 *	of the bridge prompt.
 *
 *	The event must be either KeyPress or KeyRelease and the
 *	bridge window must be in focus.
 *
 *	Returns 1 or greater if the event was for the bridge prompt
 *	and an execution was performed. Returns 0 on error or if
 *	event was not for the bridge prompt.
 */
int BridgeManagePromptExec(event_t *event)
{
	keycode_t keycode;
	char *value;
	int object_num;
	xsw_object_struct *obj_ptr;
	prompt_window_struct *prompt;
	int events_handled = 0;


	if(event == NULL)
	    return(0);

	/* Get keycode value. */
	keycode = event->xkey.keycode;


	/* Get pointer to prompt. */
	prompt = &bridge_win.prompt;

	/* Get pointer to prompt buffer. */
	value = prompt->buf;
	if(value == NULL)
	    value = "";

	/* Bridge must be in focus. */
	if(!bridge_win.is_in_focus)
	    return(0);

	/* Event type must be KeyPress or KeyRelease. */
	if((event->type != KeyPress) &&
           (event->type != KeyRelease)
        )
	    return(0);

	/* Is the prompt mode anything but none? */
	if(prompt_mode == PROMPT_CODE_NONE)
	    return(0);


	/* *************************************************** */
	/* Handle special keys. */

	/* Escape. */
	if(keycode == osw_keycode.esc)
	{
	    PromptUnmap(prompt);
	    prompt_mode = PROMPT_CODE_NONE;

            events_handled++;
	}
        /* Enter key. */ 
        else if((keycode == osw_keycode.enter) ||
                (keycode == osw_keycode.np_enter)
        )
        {
	    /* Switch by prompt mode. */
            switch(prompt_mode)
            {
              /* Connect. */
              case PROMPT_CODE_CONNECT:
                XSWDoConnect(value);
                break;

              /* Client command. */
              case PROMPT_CODE_CLIENT_CMD:
                CmdHandleInput(value);
                break;

              /* Server command. */
              case PROMPT_CODE_SERVER_CMD:
                if(net_parms.connection_state != CON_STATE_NOT_CONNECTED)
                {
                    NetSendExec(value);
                }
                break;

              /* Message. */
              case PROMPT_CODE_MESSAGE:
		object_num = net_parms.player_obj_num;
		obj_ptr = net_parms.player_obj_ptr;
                if(obj_ptr != NULL)
                {
                    NetSendComMessage(
			object_num,
			-1,
			obj_ptr->com_channel,
			value
		    );
                }
                break;

	      /* Weapon frequency. */
              case PROMPT_CODE_WEAPONFREQ:
                object_num = net_parms.player_obj_num;
                obj_ptr = net_parms.player_obj_ptr;
		if(obj_ptr != NULL)
		{
		    local_control.weapon_freq = atof(value);
		    BridgeWinDrawPanel(
		        object_num,
			BPANEL_DETAIL_PWEAPONFREQ
		    );
		}
                break;

              /* Shield frequency. */
              case PROMPT_CODE_SHIELDFREQ:
                object_num = net_parms.player_obj_num;
                obj_ptr = net_parms.player_obj_ptr;
                if(obj_ptr != NULL)
                {
                    NetSendSetShields(
                        object_num,
                        obj_ptr->shield_state,
                        atof(value)
                    );
		    BridgeWinDrawPanel(
                        object_num,
                        BPANEL_DETAIL_PSHIELDFREQ
                    );
                }
                break;

              /* Intercept. */
              case PROMPT_CODE_INTERCEPT:
                object_num = net_parms.player_obj_num;
                obj_ptr = net_parms.player_obj_ptr;
                if(obj_ptr != NULL)
                {
                    NetSendIntercept(
			object_num,
			((*value == '\0') ? "#off" : value)
		    );
                }
                break;

              /* Exit. */
              case PROMPT_CODE_EXIT:
                CmdExit(PromptGetS(prompt));
                break;

	      /* Com channel. */
              case PROMPT_CODE_COM_CHANNEL:
                object_num = net_parms.player_obj_num;
                obj_ptr = net_parms.player_obj_ptr;
                if(obj_ptr != NULL)
                {
		    obj_ptr->com_channel = (int)(
			(double)atof(value) *
                        100
		    );

                    NetSendSetChannel(
                        object_num,
                        obj_ptr->com_channel
                    );
                }
                break;

              /* Default. */
              default:
                MesgAdd("Unknown prompt mode.", xsw_color.standard_text);
                break;
	    }

            /* Send event to PromptManage() it needs to know about
             * this KeyPress event.
             */
            PromptManage(prompt, event);

            PromptUnmap(prompt);
            prompt_mode = PROMPT_CODE_NONE;

	    events_handled++;
        }    
        /* Else send key event to PromptManage(). */
        else
        {
            events_handled += PromptManage(prompt, event);
        }


	return(events_handled);
}


/*
 *	Manages events for bridge window.
 */
int BridgeManage(event_t *event)
{
	int i, events_handled = 0;
	double dx;
	keycode_t keycode;
	int object_num;
        xsw_object_struct *obj_ptr;
	win_t w;

	char stringa[512];


	if(event == NULL)
	    return(events_handled);


#if defined(X_H) && defined(USE_XSHM)
        if(!bridge_win.map_state &&
	   (event->type != MapNotify) &&
	   (event->type != (int)osw_gui[0].shm_completion_event_code)
        )
            return(events_handled);
#else
        if(!bridge_win.map_state &&
           (event->type != MapNotify)
        )
            return(events_handled);
#endif	/* defined(X_H) && defined(USE_XSHM) */


	/* Get referances. */
	w = event->xany.window; 


#if defined(X_H) && defined(USE_XSHM)
	/* Check for viewscreen shared image put completion. */
	if(event->type == (int)osw_gui[0].shm_completion_event_code)
	{
	    /* Viewscreen window? */
	    if(w == bridge_win.viewscreen)
	    {
		if(bridge_win.viewscreen_image != NULL)
		    bridge_win.viewscreen_image->in_progress = False;

		events_handled++;
                return(events_handled);
	    }
	    /* Scanner window? */
	    else if(w == bridge_win.scanner)
            {
		if(bridge_win.scanner_image != NULL)
                    bridge_win.scanner_image->in_progress = False;

                events_handled++;
                return(events_handled);
	    }
	}
#endif	/* defined(X_H) && defined(USE_XSHM) */


	/* Check if current page is mapped, if so then manage it. */
	if(bridge_win.cur_page != NULL)
	{
	    if(bridge_win.cur_page->map_state)
	    {
		/* Allow main menu to manage this event. */
		events_handled += PageManage(
                    bridge_win.cur_page,
                    bridge_win.viewscreen,
		    bridge_win.viewscreen_image,
		    event
		);
		/* Return if event was handled. */
		if(events_handled > 0)
		    return(events_handled);
	    }
	}


	/*
	 *   If in prompt mode, allow prompt to manage event
	 *   (it will handle KeyPress and KeyRelease events only).
	 */
	events_handled += BridgeManagePromptExec(event);
	/* Return if prompt managed the event. */
	if(events_handled > 0)
	    return(events_handled);


	/* All other event management. */
	switch(event->type)
	{
	  /* *********************************************************** */
	  case KeyPress:
	    /* Skip if not in focus. */
	    if(!bridge_win.is_in_focus)
		return(events_handled);

	    keycode = event->xkey.keycode;

	    /* Skip modifier keys. */
	    if(OSWIsModifierKey(keycode))
                return(events_handled);


            /* Print help. */
            if(keycode == xsw_keymap[XSW_KM_HELP].keycode)
	    {
                XSWDoHelpMesgWin();
                events_handled++; 
            }
            /* Viewscreen markings. */
            else if(keycode == xsw_keymap[XSW_KM_VIEWSCREEN_MARKINGS].keycode)
	    {
		option.show_viewscreen_marks =
		    (option.show_viewscreen_marks) ? 0 : 1;
                events_handled++; 
	    }
            /* Viewscreen labels. */
            else if(keycode == xsw_keymap[XSW_KM_VIEWSCREEN_LABELS].keycode)
	    {
		option.show_viewscreen_labels++;
		if(option.show_viewscreen_labels > 3)
		    option.show_viewscreen_labels = 0;

		/* Update netstats label (do not draw it). */
                if((option.show_viewscreen_labels == 2) ||
                   (option.show_viewscreen_labels == 3)
                )
                {
                    VSDrawUpdateNetstatsLabel(
                        &bridge_win.net_stats_image,
                        bridge_win.net_stats_buf
                    );
                }

                events_handled++; 
	    }
            /* Net update interval decrease. */
            else if(keycode == xsw_keymap[XSW_KM_NET_INTERVAL_DEC].keycode)
	    {
                if(net_parms.connection_state == CON_STATE_CONNECTED)
		{
                    net_parms.net_int -= ((osw_gui[0].shift_key_state) ? 100 : 25);
                    if(net_parms.net_int < MIN_SERVER_UPDATE_INT)
                        net_parms.net_int = MIN_SERVER_UPDATE_INT;

                    NetSendSetInterval();

                    /* Redraw net stats. */
                    if((option.show_viewscreen_labels == 2) ||
                       (option.show_viewscreen_labels == 3)
                    )
                    {
                        VSDrawUpdateNetstatsLabel(
                            &bridge_win.net_stats_image, 
                            bridge_win.net_stats_buf
                        );
		    }
                }
                events_handled++; 
	    }
            /* Net update interval increase. */
            else if(keycode == xsw_keymap[XSW_KM_NET_INTERVAL_INC].keycode)
            {
                if(net_parms.connection_state == CON_STATE_CONNECTED)
                {
                    net_parms.net_int += ((osw_gui[0].shift_key_state) ? 100 : 25);
                    if(net_parms.net_int > MAX_SERVER_UPDATE_INT)
                        net_parms.net_int = MAX_SERVER_UPDATE_INT;

                    NetSendSetInterval();

                    /* Redraw net stats. */
                    if((option.show_viewscreen_labels == 2) ||
                       (option.show_viewscreen_labels == 3)
                    )
                    {
                        VSDrawUpdateNetstatsLabel(
                            &bridge_win.net_stats_image,
                            bridge_win.net_stats_buf
                        );
                    }
                }
                events_handled++; 
            }
            /* Message window scroll up. */
            else if(keycode == xsw_keymap[XSW_KM_MESG_SCROLL_UP].keycode)
	    {
                bridge_win.mesg_box_sb.y_win_pos -= bridge_win.line_spacing;
                if(bridge_win.mesg_box_sb.y_win_pos < 0)
                    bridge_win.mesg_box_sb.y_win_pos = 0;

		MesgDrawAll();

                events_handled++;
            }
            /* Message window scroll down. */
	    else if(keycode == xsw_keymap[XSW_KM_MESG_SCROLL_DOWN].keycode)
	    {
                bridge_win.mesg_box_sb.y_win_pos += bridge_win.line_spacing;
                if(bridge_win.mesg_box_sb.y_win_pos >
                    (((MESG_WIN_TOTAL_MESSAGES + 1) * (int)bridge_win.line_spacing) -
                    (int)bridge_win.mesg_box_height)
                )
                    bridge_win.mesg_box_sb.y_win_pos =
                        ((MESG_WIN_TOTAL_MESSAGES + 1) * bridge_win.line_spacing) -
                        bridge_win.mesg_box_height;

		MesgDrawAll();

                events_handled++; 
            }
            /* Message window scroll to end. */
            else if(keycode == osw_keycode.end)
            {
                bridge_win.mesg_box_sb.y_win_pos =
                    ((MESG_WIN_TOTAL_MESSAGES + 1) * bridge_win.line_spacing) -
                    bridge_win.mesg_box_height;

                MesgDrawAll();

                events_handled++;
            }
	    /* Map Economy window. */
	    else if(keycode == xsw_keymap[XSW_KM_MAP_ECONOMY].keycode)
            {
		XSWActionCB(&bridge_win, NULL, XSW_ACTION_ECONOMY);
		events_handled++;
	    }
            /* Map Star Chart window. */
            else if(keycode == xsw_keymap[XSW_KM_MAP_STARCHART].keycode)
            {
		XSWActionCB(&bridge_win, NULL, XSW_ACTION_STARCHART);
                events_handled++;
            }
            /* Net connect. */
            else if(keycode == xsw_keymap[XSW_KM_CONNECT].keycode)
	    {
		char url[MAX_URL_LEN + (2 * XSW_OBJ_NAME_MAX)];

                sprintf(
		    url,
		    "swserv://%s:%s@%s:%i",
                    net_parms.login_name,
                    net_parms.login_password,
                    net_parms.address,
                    net_parms.port
                );
		BridgeDoQuery(
		    &bridge_win.prompt,
		    "Connect:",
		    url,
		    PROMPT_CODE_CONNECT
		);
                events_handled++; 
            }
            /* Net disconnect. */
            else if(keycode == xsw_keymap[XSW_KM_DISCONNECT].keycode)
	    {
		XSWDoDisconnect();
                events_handled++; 
            }
            /* Net refresh. */
            else if(keycode == xsw_keymap[XSW_KM_REFRESH].keycode)
	    {
		XSWDoRefresh();
                events_handled++; 
            }
            /* Net connect last. */
            else if(keycode == xsw_keymap[XSW_KM_CONNECTLAST].keycode)
            {
                if(net_parms.connection_state == CON_STATE_NOT_CONNECTED)
		    NetOpenConnection(net_parms.address, net_parms.port);
                events_handled++;
            }
	    /* Server command. */
	    else if(keycode == xsw_keymap[XSW_KM_SERVER_CMD].keycode)
	    {
                if(net_parms.connection_state == CON_STATE_CONNECTED)
                {
                    BridgeDoQuery(
                        &bridge_win.prompt,
                        "Server Command:",
                        NULL,
                        PROMPT_CODE_SERVER_CMD
                    );
	        }
                events_handled++; 
            }
            /* Client command. */
            else if(keycode == xsw_keymap[XSW_KM_CLIENT_CMD].keycode)
	    {
                BridgeDoQuery(
                    &bridge_win.prompt,
                    "Client Command:",
                    NULL,
                    PROMPT_CODE_CLIENT_CMD
                );
                events_handled++;
            }
	    /* Screen shot. */
            else if(keycode == xsw_keymap[XSW_KM_SCREEN_SHOT].keycode)
            {
		BridgeDoScreenShot(
                    dname.downloads,
                    0, 0,
                    0, 0
                );
		events_handled++;
	    }
            /* Viewscreen zoom out. */
            else if(keycode == xsw_keymap[XSW_KM_VIEWSCREEN_ZOUT].keycode)
            {
		if(osw_gui[0].ctrl_key_state)
		{
		    /* Holding down ctrl changes bridge window size. */
		    BridgeWinResizePreset(bridge_win.preset_zoom_code - 1);
		}
		else
		{
		    bridge_win.viewscreen_zoom -= (VS_ZOOM_INC *
			((osw_gui[0].shift_key_state) ? 4 : 1) *
			time_compensation
		    );

		    if(bridge_win.viewscreen_zoom < VS_ZOOM_MIN)
			bridge_win.viewscreen_zoom = VS_ZOOM_MIN;

		    /* Turn off auto zoom. */
		    if(option.auto_zoom == 1)
			option.auto_zoom = 0;
		}
		events_handled++;
            }
	    /* Viewscreen zoom in. */
	    else if(keycode == xsw_keymap[XSW_KM_VIEWSCREEN_ZIN].keycode)
	    {
                if(osw_gui[0].ctrl_key_state)
                {
                    /* Holding down ctrl changes bridge window size. */
                    BridgeWinResizePreset(bridge_win.preset_zoom_code + 1);
                }
                else
                {
                    bridge_win.viewscreen_zoom += (VS_ZOOM_INC *
                        ((osw_gui[0].shift_key_state) ? 4 : 1) *
                        time_compensation
		    );

                    if(bridge_win.viewscreen_zoom > VS_ZOOM_MAX)
                        bridge_win.viewscreen_zoom = VS_ZOOM_MAX;

                    /* Turn off auto zoom. */
		    if(option.auto_zoom == 1)
		        option.auto_zoom = 0;
		}
                events_handled++;
	    }
            /* Viewscreen zoom auto. */
            else if(keycode == xsw_keymap[XSW_KM_VIEWSCREEN_ZAUTO].keycode)
            {
		if(option.auto_zoom == 0)
                    option.auto_zoom = 1;

                events_handled++;
            }
            /* Scanner Toggle. */
            else if(keycode == xsw_keymap[XSW_KM_SCANNER_TOGGLE].keycode)
	    {
/*

No longer used.  Toggles scanner active/inactive.

 */
                events_handled++; 
            }
            /* Scanner zoom in. */
            else if(keycode == xsw_keymap[XSW_KM_SCANNER_ZIN].keycode)
	    {
		bridge_win.scanner_sb.pos -= (0.5 *
		    ((osw_gui[0].shift_key_state) ? 4 : 1)
		);

		/* Sanitize values. */
		if(bridge_win.scanner_sb.pos < bridge_win.scanner_sb.pos_min)
		    bridge_win.scanner_sb.pos = bridge_win.scanner_sb.pos_min;

		ScannerSBCB(&bridge_win.scanner_sb);
		ScaleBarDraw(&bridge_win.scanner_sb);

                events_handled++; 
            }
            /* Scanner zoom out. */
            else if(keycode == xsw_keymap[XSW_KM_SCANNER_ZOUT].keycode)
            {
                bridge_win.scanner_sb.pos += (0.5 *
                    ((osw_gui[0].shift_key_state) ? 4 : 1) 
                );  

                /* Sanitize values. */
                if(bridge_win.scanner_sb.pos > bridge_win.scanner_sb.pos_max)
                    bridge_win.scanner_sb.pos = bridge_win.scanner_sb.pos_max;

                ScannerSBCB(&bridge_win.scanner_sb);
                ScaleBarDraw(&bridge_win.scanner_sb);

                events_handled++;
            }
	    /* Scanner zoom to match visual range. */
            else if(keycode == xsw_keymap[XSW_KM_SCANNER_ZMIN].keycode)
            {
		if(net_parms.player_obj_ptr != NULL)
		{
		    dx = net_parms.player_obj_ptr->scanner_range;
		    if((dx != 0) &&
                       (bridge_win.viewscreen_zoom > 0)
		    )
		        bridge_win.scanner_zoom =
			    (((double)bridge_win.viewscreen_width /
			    bridge_win.viewscreen_zoom) / 2000) / dx;
		    else
			bridge_win.scanner_zoom = 1;

		    if(bridge_win.scanner_zoom < 0.001)
                        bridge_win.scanner_zoom = 0.001;

		    bridge_win.scanner_sb.pos =
			bridge_win.scanner_zoom * 100;
		}

                ScannerSBCB(&bridge_win.scanner_sb);
                ScaleBarDraw(&bridge_win.scanner_sb);

                events_handled++; 
            }
            /* Scanner zoom max. */
            else if(keycode == xsw_keymap[XSW_KM_SCANNER_ZMAX].keycode)
	    {
		bridge_win.scanner_zoom = 1.000;
                bridge_win.scanner_sb.pos = 100;

		ScannerSBCB(&bridge_win.scanner_sb);
                ScaleBarDraw(&bridge_win.scanner_sb);

                events_handled++; 
            }
            /* Scanner orient toggle. */
            else if(keycode == xsw_keymap[XSW_KM_SCANNER_ORIENT].keycode)
	    {
		if(bridge_win.scanner_orient == SCANNER_ORIENT_GC)
		{
		    /* Switch to local vessel orientation. */
		    bridge_win.scanner_orient = SCANNER_ORIENT_LOCAL;
                    strncpy(
			stringa,
                        "Scanner: Vessel orientation.",
			255
		    );
		}
		else if(bridge_win.scanner_orient == SCANNER_ORIENT_LOCAL)
		{
		    /* Switch to galactic core orientation. */
                    bridge_win.scanner_orient = SCANNER_ORIENT_GC;
                    strncpy(
                        stringa,
                        "Scanner: Galactic Core orientation.",
                        255
                    );
		}
		else
		{
		    /* Default to galactic core orientation. */
		    bridge_win.scanner_orient = SCANNER_ORIENT_GC;
                    strncpy(
                        stringa,
                        "Scanner: Galactic Core orientation.",
                        255
                    );
		}
	        MesgAdd(stringa, xsw_color.standard_text);

		ScannerSBCB(&bridge_win.scanner_sb);
                ScaleBarDraw(&bridge_win.scanner_sb);

                events_handled++; 
            }
	    /* Lights: Vector */
            else if(keycode == xsw_keymap[XSW_KM_LIGHTS_VECTOR].keycode)
            {
		object_num = net_parms.player_obj_num;
		obj_ptr = net_parms.player_obj_ptr;

		if(obj_ptr != NULL)
		{
		    if(obj_ptr->lighting & XSW_OBJ_LT_VECTOR)
		    {
		        obj_ptr->lighting &= ~XSW_OBJ_LT_VECTOR;
                        sprintf(stringa, "Vector Lights: Off");
		    }
		    else
                    {
                        obj_ptr->lighting |= XSW_OBJ_LT_VECTOR;
                        sprintf(stringa, "Vector Lights: On");
                    }

		    NetSendSetLighting(
			object_num,
			obj_ptr->lighting
		    );

                    MesgAdd(stringa, xsw_color.bp_standard_text);
		}
		events_handled++;
	    }
            /* Lights: Strobe */
            else if(keycode == xsw_keymap[XSW_KM_LIGHTS_STROBE].keycode)
            {
                object_num = net_parms.player_obj_num;
                obj_ptr = net_parms.player_obj_ptr;

                if(obj_ptr != NULL)
                {
                    if(obj_ptr->lighting & XSW_OBJ_LT_STROBE)
                    {
                        obj_ptr->lighting &= ~XSW_OBJ_LT_STROBE;
			sprintf(stringa, "Strobes: Off");
                    }
                    else
                    {
                        obj_ptr->lighting |= XSW_OBJ_LT_STROBE;
                        sprintf(stringa, "Strobes: On");
                    }

                    NetSendSetLighting(
                        object_num,
                        obj_ptr->lighting
                    );

                    MesgAdd(stringa, xsw_color.bp_standard_text);
		}
                events_handled++;
            }
            /* Lights: Lumination */
            else if(keycode == xsw_keymap[XSW_KM_LIGHTS_LUMINATION].keycode)
            {
                object_num = net_parms.player_obj_num;
                obj_ptr = net_parms.player_obj_ptr;

                if(obj_ptr != NULL)
                {
                    if(obj_ptr->lighting & XSW_OBJ_LT_LUMINATION)
                    {
                        obj_ptr->lighting &= ~XSW_OBJ_LT_LUMINATION;
                        sprintf(stringa, "External Lumination: Off");
                    }
                    else
                    {
                        obj_ptr->lighting |= XSW_OBJ_LT_LUMINATION;
                        sprintf(stringa, "External Lumination: On");  
                    }

                    NetSendSetLighting(
                        object_num,
                        obj_ptr->lighting
                    );

                    MesgAdd(stringa, xsw_color.standard_text);
                }
                events_handled++;
            }
	    /* Engine state toggle. */
	    else if(keycode == xsw_keymap[XSW_KM_ENGINE_STATE].keycode)
	    {
                object_num = net_parms.player_obj_num;
                obj_ptr = net_parms.player_obj_ptr;

                if(obj_ptr != NULL)
		{
		    if(obj_ptr->engine_state > 0)
		    {
		        /* Turn engines off. */
                        NetSendSetEngine(object_num, ENGINE_STATE_OFF);

			sprintf(stringa, "Engines: Off");

                        SoundPlay(
                            SOUND_CODE_ENGINES_OFF,
                            1.0, 1.0,	/* Left and right volume. */
			    0, 0	/* Effects and prioritu */
                        );
		    }
		    else if(obj_ptr->engine_state == ENGINE_STATE_OFF)
		    {
			/* Turn engines on. */
			NetSendSetEngine(object_num, ENGINE_STATE_ON);

			sprintf(stringa, "Engines: On");

                        SoundPlay(
                            SOUND_CODE_ENGINES_ON,
                            1.0, 1.0,	/* Left and right volume. */
                            0, 0	/* Effects and prioritu */
                        );
		    }
		    MesgAdd(stringa, xsw_color.standard_text);
		}
                events_handled++;
	    }
            /* Throttle scope mode. */
            else if(keycode == xsw_keymap[XSW_KM_THROTTLE_MODE].keycode)
	    {
		if(osw_gui[0].shift_key_state)
		    option.throttle_mode--;
		else
		    option.throttle_mode++;

		/* Cycles through normal, bidirectional, and incremental. */
                if(option.throttle_mode > THROTTLE_MODE_INCREMENTAL)
		    option.throttle_mode = THROTTLE_MODE_NORMAL;
		if(option.throttle_mode < THROTTLE_MODE_NORMAL)
		    option.throttle_mode = THROTTLE_MODE_INCREMENTAL;

                switch(option.throttle_mode)
                {
		  case THROTTLE_MODE_INCREMENTAL:
                    strcpy(stringa, "Throttle mode: incremental");
                    break;

                  case THROTTLE_MODE_BIDIRECTIONAL:
                    strcpy(stringa, "Throttle mode: bidirectional");
                    break;
            
                  case THROTTLE_MODE_NORMAL:
                    strcpy(stringa, "Throttle mode: normal");
                    break;

                  default:
                    strcpy(stringa, "Throttle mode: *badvalue*");
                    break;
                }
                MesgAdd(stringa, xsw_color.standard_text);
                events_handled++;
            }
            /* Shields toggle. */
            else if(keycode == xsw_keymap[XSW_KM_SHIELD_STATE].keycode)
	    {
                object_num = net_parms.player_obj_num;
                obj_ptr = net_parms.player_obj_ptr;
                            
                if(obj_ptr != NULL)
                {
	            switch(obj_ptr->shield_state)
		    {
	              case SHIELD_STATE_DOWN:
			NetSendSetShields(
			    object_num,
			    SHIELD_STATE_UP,
			    obj_ptr->shield_frequency
			);
			break;

                      default:
                        NetSendSetShields(
			    object_num,
                            SHIELD_STATE_DOWN,
                            obj_ptr->shield_frequency
                        );
                        break;
		    }
	        }
                events_handled++; 
            }
            /* Dmgctl toggle. */ 
            else if(keycode == xsw_keymap[XSW_KM_DMGCTL_TOGGLE].keycode)
	    {
                object_num = net_parms.player_obj_num;
                obj_ptr = net_parms.player_obj_ptr;
                 
                if(obj_ptr != NULL)
	        {
                    NetSendSetDmgCtl(
			object_num,
			(obj_ptr->damage_control == DMGCTL_STATE_ON) ? 0 : 1
		    );
	        }
                events_handled++; 
            }
            /* Cloak toggle. */ 
            else if(keycode == xsw_keymap[XSW_KM_CLOAK_STATE].keycode)
	    {
                object_num = net_parms.player_obj_num;
                obj_ptr = net_parms.player_obj_ptr;

                if(obj_ptr != NULL)
	        {
		    if(obj_ptr->cloak_state == CLOAK_STATE_UP)
                        SoundPlay(
			    SOUND_CODE_CLOAK_DOWN,
                            1.0, 1.0,	/* Left and right volume. */
                            0, 0        /* Effects and prioritu */
                        );
		    else if(obj_ptr->cloak_state == CLOAK_STATE_DOWN)
                        SoundPlay(
                            SOUND_CODE_CLOAK_UP,
                            1.0, 1.0,	/* Left and right volume. */
                            0, 0	/* Effects and prioritu */
                        );

                    NetSendSetCloak(
                        object_num,
                        ((obj_ptr->cloak_state == CLOAK_STATE_UP) ?
                            CLOAK_STATE_DOWN : CLOAK_STATE_UP
                        )
                    );
	        }
                events_handled++; 
            }
	    /* Weapons online/offline toggle. */
	    else if(keycode == xsw_keymap[XSW_KM_WEAPONS_ONLINE].keycode)
	    {
                object_num = net_parms.player_obj_num;
                obj_ptr = net_parms.player_obj_ptr;
                    
                if(obj_ptr != NULL)
		{
		    if(osw_gui[0].shift_key_state)
		    {
			/* Disarm all weapons. */
			NetSendWeaponDisarm(
			    object_num,
			    -1		/* All weapons we own. */
			);
		    }
		    else
		    {
		        if(local_control.weapons_online)
		        {
		            local_control.weapons_online = 0;
		            MesgAdd(
			        "Weapons have been taken offline.",
			        xsw_color.standard_text
			    );
		        }
		        else
		        {
		            local_control.weapons_online = 1;
		            MesgAdd(
			        "Weapons are now online.",
			        xsw_color.standard_text
			    );
			}

			/* Recreate the viewscreen weapon label to
			 * reflect change.
			 */
			VSDrawUpdateWeaponLabel(
			    &bridge_win.vs_weapon_image,
			    bridge_win.vs_weapon_buf
			);

			BridgeWinDrawPanel(object_num, BPANEL_DETAIL_P4);
		    }
		}
	    }
            /* Select weapon previous. */
            else if(keycode == xsw_keymap[XSW_KM_SELECT_WEAPONPREV].keycode)
            {
                object_num = net_parms.player_obj_num;
                obj_ptr = net_parms.player_obj_ptr;

                if(obj_ptr != NULL)
                {
                    NetSendSelectWeapon(
			object_num,
			((obj_ptr->selected_weapon > 0) ?
			    obj_ptr->selected_weapon - 1 :
                            obj_ptr->total_weapons - 1
			)
		    );
                }
                events_handled++;
            }
            /* Select weapon next. */
            else if(keycode == xsw_keymap[XSW_KM_SELECT_WEAPONNEXT].keycode)
            {
                object_num = net_parms.player_obj_num;
                obj_ptr = net_parms.player_obj_ptr;
                 
                if(obj_ptr != NULL)
                {
                    NetSendSelectWeapon(
                        object_num,
                        (((obj_ptr->selected_weapon + 1) >= obj_ptr->total_weapons) ?
                            0 :
                            obj_ptr->selected_weapon + 1
                        )
                    );   
                }
                events_handled++;
            }
            /* Select weapon 1. */
            else if(keycode == xsw_keymap[XSW_KM_SELECT_WEAPON1].keycode)
	    {
                object_num = net_parms.player_obj_num;
                obj_ptr = net_parms.player_obj_ptr;

                if(obj_ptr != NULL)
		{
                    NetSendSelectWeapon(object_num, 0);
                }
                events_handled++; 
            }
            /* Select weapon 2. */
            else if(keycode == xsw_keymap[XSW_KM_SELECT_WEAPON2].keycode)
            {
                object_num = net_parms.player_obj_num;
                obj_ptr = net_parms.player_obj_ptr;
                
                if(obj_ptr != NULL)
		{
                    NetSendSelectWeapon(object_num, 1);
                }
                events_handled++; 
            }
            /* Select weapon 3. */
            else if(keycode == xsw_keymap[XSW_KM_SELECT_WEAPON3].keycode)
	    {
                object_num = net_parms.player_obj_num;
                obj_ptr = net_parms.player_obj_ptr;

                if(obj_ptr != NULL)
                {
                    NetSendSelectWeapon(object_num, 2);
                }
                events_handled++;
	    }
            /* Select weapon 4. */
            else if(keycode == xsw_keymap[XSW_KM_SELECT_WEAPON4].keycode)
	    {
                object_num = net_parms.player_obj_num;
                obj_ptr = net_parms.player_obj_ptr;

                if(obj_ptr != NULL)
                {
                    NetSendSelectWeapon(object_num, 3);
                }
                events_handled++;
            }
            /* Weapons lock (cycle). */
            else if(keycode == xsw_keymap[XSW_KM_WEAPONS_LOCK].keycode)
	    {
                object_num = net_parms.player_obj_num;
                obj_ptr = net_parms.player_obj_ptr;
                
                if(obj_ptr != NULL)
                {
                    NetSendWeaponsLock(object_num, -2);
		}
                events_handled++;
            }
            /* Weapons unlock. */
            else if(keycode == xsw_keymap[XSW_KM_WEAPONS_UNLOCK].keycode)
	    {
                object_num = net_parms.player_obj_num;
                obj_ptr = net_parms.player_obj_ptr;
            
                if(obj_ptr != NULL)
		{
		    if(osw_gui[0].shift_key_state)
		    {
                        /* Send tractor beam unlock. */
                        NetSendTractorBeamLock(
                            object_num,
                            -1
                        );
		    }
		    else
		    {
			/* Send unlock weapons. */
			NetSendWeaponsUnlock(object_num);
		    }
		}
                events_handled++; 
            }
	    /* Send hail. */
            else if(keycode == xsw_keymap[XSW_KM_HAIL].keycode)
            {
                object_num = net_parms.player_obj_num;
                obj_ptr = net_parms.player_obj_ptr;

                if(obj_ptr != NULL)
                {
                    NetSendHail(
			object_num,
			-1,
			obj_ptr->com_channel
		    );
		}
                events_handled++;
            }
            /* Set intercept. */
            else if(keycode == xsw_keymap[XSW_KM_SET_INTERCEPT].keycode)
	    {
                obj_ptr = net_parms.player_obj_ptr;
                object_num = net_parms.player_obj_num;

                if(obj_ptr != NULL)
                {
                    BridgeDoQuery(
                        &bridge_win.prompt,
                        "Intercept:",
                        NULL,
                        PROMPT_CODE_INTERCEPT
                    );
                }
                events_handled++;
            }
            /* Set weapon frequency. */
            else if(keycode == xsw_keymap[XSW_KM_WEAPON_FREQ].keycode)
            {
                obj_ptr = net_parms.player_obj_ptr;
                object_num = net_parms.player_obj_num;

                if(obj_ptr != NULL)
                {
                    sprintf(
			stringa,
                        "%.2f",
                        local_control.weapon_freq
                    );
                    BridgeDoQuery(
                        &bridge_win.prompt,
                        "Weapon Frequency:",
                        stringa,
                        PROMPT_CODE_WEAPONFREQ
                    );
                }
                events_handled++;
            }
            /* Set shield frequency. */
            else if(keycode == xsw_keymap[XSW_KM_SHIELD_FREQ].keycode)
	    {
                obj_ptr = net_parms.player_obj_ptr;
                object_num = net_parms.player_obj_num;

                if(obj_ptr != NULL)
                {
                    sprintf(stringa,
                        "%.2f",
                        obj_ptr->shield_frequency
                    );
                    BridgeDoQuery(
                        &bridge_win.prompt,   
                        "Shield Frequency:",
                        stringa,
                        PROMPT_CODE_SHIELDFREQ
                    );
                }
                events_handled++; 
            }
            /* Set com channel. */
            else if(keycode == xsw_keymap[XSW_KM_SET_CHANNEL].keycode)
            {
                obj_ptr = net_parms.player_obj_ptr;
                object_num = net_parms.player_obj_num;

                if(obj_ptr != NULL)
                {
                    /* Put current com channel to prompt's buffer. */
                    sprintf(stringa,
                        "%.2f",
                        (double)((double)obj_ptr->com_channel / 100.0)
                    );
                    BridgeDoQuery(
                        &bridge_win.prompt,
                        "Communications Channel:",
                        stringa,
                        PROMPT_CODE_COM_CHANNEL
                    );
                }
                events_handled++;
	    }
            /* Send message. */
            else if(keycode == xsw_keymap[XSW_KM_SEND_MESSAGE].keycode)
	    {
                BridgeDoQuery(
                    &bridge_win.prompt,
                    "Message:",
                    NULL,
                    PROMPT_CODE_MESSAGE
                );
                events_handled++;
            }
	    /* Exit. */
            else if(keycode == xsw_keymap[XSW_KM_EXIT].keycode)
	    {
                BridgeDoQuery(
                    &bridge_win.prompt,
                    "Exit?:",
                    NULL,
                    PROMPT_CODE_EXIT
                );
                events_handled++;
            }
	    /* Energy saver mode. */
	    else if(keycode == xsw_keymap[XSW_KM_ENERGY_SAVER_MODE].keycode)
	    {
		if(option.energy_saver_mode)
                    option.energy_saver_mode = 0;
		else
		    option.energy_saver_mode = 1;

                events_handled++;
	    }

            /* Unknown Command. */
            else
	    {
		sprintf(stringa,
		    "Unknown Command, Press \"%s\" for help.",
		    OSWGetKeyCodeName(xsw_keymap[XSW_KM_HELP].keycode)
		);
                MesgAdd(stringa, xsw_color.standard_text);
	    }

	    /*   Don't continure further and let widgets handle
	     *   if KeyPress was already handled.
	     */
	    if(events_handled > 0)
		return(events_handled);	

	    break;

          /* *********************************************************** */
	  case KeyRelease:
            /* Skip if not in focus. */
            if(!bridge_win.is_in_focus)
                return(events_handled);

	    keycode = event->xkey.keycode;

	    /* Skip modifier keys. */
	    if(OSWIsModifierKey(keycode))
		return(events_handled);


	    break;

          /* *********************************************************** */
	  case ButtonPress:
            /* Player console panel section 1. */
            if(event->xany.window == bridge_win.pan_p1)
            {
                obj_ptr = net_parms.player_obj_ptr;
                object_num = net_parms.player_obj_num;

                /* Com channel inc. */
                if(BW_IS_IN_BTNPOS(
                    BPANEL_BTNPOS_PCOMCHANNEL_UP,
                    event->xbutton.x,
                    event->xbutton.y
                   ) &&
                   (obj_ptr != NULL)
                )
                {
                    obj_ptr->com_channel += ((osw_gui[0].shift_key_state) ? 100 : 5);
                    if(((double)obj_ptr->com_channel / 100) > SWR_FREQ_MAX)
                        obj_ptr->com_channel = static_cast<int>(SWR_FREQ_MAX * 100);
                    if(((double)obj_ptr->com_channel / 100) < SWR_FREQ_MIN)
                        obj_ptr->com_channel = static_cast<int>(SWR_FREQ_MIN * 100);

                    NetSendSetChannel(
                        object_num,
                        obj_ptr->com_channel
                    );
		}

                /* Com channel dec. */
                if(BW_IS_IN_BTNPOS(
                    BPANEL_BTNPOS_PCOMCHANNEL_DOWN,
                    event->xbutton.x,
                    event->xbutton.y
                   ) &&
                   (obj_ptr != NULL)
                )
                {
                    obj_ptr->com_channel -= ((osw_gui[0].shift_key_state) ? 100 : 5);
                    if(((double)obj_ptr->com_channel / 100) > SWR_FREQ_MAX)
                        obj_ptr->com_channel = static_cast<int>(SWR_FREQ_MAX * 100);
                    if(((double)obj_ptr->com_channel / 100) < SWR_FREQ_MIN)
                        obj_ptr->com_channel = static_cast<int>(SWR_FREQ_MIN * 100);

                    NetSendSetChannel(
                        object_num,
                        obj_ptr->com_channel
                    );
                }

                /* Play button press sound. */
                if(option.sounds >= XSW_SOUNDS_EVENTS)
                {
                    SoundPlay(
                        SOUND_CODE_BUTTONPRESS,
                        1.0, 0.2,	/* Left and right volume. */
                        0, 0		/* Effects and prioritu */
                    );
                }

                events_handled++;
	    }
            /* Player console panel section 2. */
            else if(event->xany.window == bridge_win.pan_p2)
            {
                obj_ptr = net_parms.player_obj_ptr;
                object_num = net_parms.player_obj_num;

		/* If it's button 2, then print stats of player. */
		if(event->xbutton.button == Button2)
		{
		    BridgePrintSubjectStats(object_num);
		}
		/* Shields toggle. */
		else if(BW_IS_IN_BTNPOS(
		    BPANEL_BTNPOS_PSHIELDS,
		    event->xbutton.x,
		    event->xbutton.y
		   ) &&
		   (obj_ptr != NULL)
		)
                {
                    NetSendSetShields(
                        object_num,
                        (obj_ptr->shield_state == SHIELD_STATE_UP) ?
                            SHIELD_STATE_DOWN : SHIELD_STATE_UP,
			obj_ptr->shield_frequency
                    );
 		}

		/* Cloak toggle. */
                if(BW_IS_IN_BTNPOS(
                    BPANEL_BTNPOS_PCLOAK,
                    event->xbutton.x,
                    event->xbutton.y
                   ) &&
                   (obj_ptr != NULL)
                )
                {
		    if(obj_ptr->cloak_state == CLOAK_STATE_UP)
			SoundPlay(
			    SOUND_CODE_CLOAK_DOWN,
			    1.0, 1.0,	/* Left and right volume. */
                            0, 0        /* Effects and prioritu */
                        );
                    else if(obj_ptr->cloak_state == CLOAK_STATE_DOWN)
                        SoundPlay(
                            SOUND_CODE_CLOAK_UP,
                            1.0, 1.0,	/* Left and right volume. */
                            0, 0        /* Effects and prioritu */
                        );

                    NetSendSetCloak(
                        object_num,
                        ((obj_ptr->cloak_state == CLOAK_STATE_UP) ?
                            CLOAK_STATE_DOWN : CLOAK_STATE_UP
                        )
                    );
		}

                /* Damage control toggle. */
                if(BW_IS_IN_BTNPOS(
                    BPANEL_BTNPOS_PDMGCTL,
                    event->xbutton.x,
                    event->xbutton.y
                   ) &&
                   (obj_ptr != NULL)
                )
		{
                    NetSendSetDmgCtl(
                        object_num,
                        (obj_ptr->damage_control == DMGCTL_STATE_ON) ?
                            DMGCTL_STATE_OFF : DMGCTL_STATE_ON
                    );
		}

                /* Play button press sound. */
                if(option.sounds >= XSW_SOUNDS_EVENTS)
                {
                    SoundPlay(
                        SOUND_CODE_BUTTONPRESS,
                        1.0, 0.2,	/* Left and right volume. */
                        0, 0		/* Effects and prioritu */
                    );
		}

		events_handled++;
            }
            /* Player console panel section 3. */
            else if(event->xany.window == bridge_win.pan_p3)
	    {
                obj_ptr = net_parms.player_obj_ptr;
                object_num = net_parms.player_obj_num;

                /* Engine state. */
                if(BW_IS_IN_BTNPOS(
                    BPANEL_BTNPOS_PENGINESTATE,
                    event->xbutton.x,
                    event->xbutton.y
                   ) &&
                   (obj_ptr != NULL)
                )
                {
                    if(obj_ptr->engine_state > 0)
                    {
                        /* Turn engines off. */
                        NetSendSetEngine(object_num, ENGINE_STATE_OFF);

                        sprintf(stringa, "Engines: Off");

                        SoundPlay(
                            SOUND_CODE_ENGINES_OFF,
                            1.0, 1.0,	/* Left and right volume. */
                            0, 0	/* Effects and prioritu */
                        );
                    }
                    else if(obj_ptr->engine_state == ENGINE_STATE_OFF)
                    {
                        /* Turn engines on. */
                        NetSendSetEngine(object_num, ENGINE_STATE_ON);

                        sprintf(stringa, "Engines: On");

                        SoundPlay(
                            SOUND_CODE_ENGINES_ON,
                            1.0, 1.0,	/* Left and right volume. */
                            0, 0	/* Effects and prioritu */
                        );
                    }
                    MesgAdd(stringa, xsw_color.standard_text);
                }

                /* Play button press sound. */
                if(option.sounds >= XSW_SOUNDS_EVENTS)
                {
                    SoundPlay(
                        SOUND_CODE_BUTTONPRESS,
                        1.0, 0.2,	/* Left and right volume. */
			0, 0		/* Effects and prioritu */
                    );
                }

                events_handled++;
	    }
            /* Player console panel section 4. */
            else if(event->xany.window == bridge_win.pan_p4)
            {
                obj_ptr = net_parms.player_obj_ptr;
                object_num = net_parms.player_obj_num;

		/* Weapon yield. */
		if(BW_IS_IN_BTNPOS(
                    BPANEL_BTNPOS_PWEPYIELD,
                    event->xbutton.x,
                    event->xbutton.y
                   ) &&
                   (obj_ptr != NULL)
                )
                {
                    const unsigned int yield_bar_width = 80,
                                       yield_bar_height = 6;

                    win_attr_t wa;


		    /* Map weapon yield input window. */
		    OSWDestroyWindow(&bridge_win.weapon_yield_iwin);
		    if(!OSWCreateInputWindow(
			&bridge_win.weapon_yield_iwin,
			bridge_win.pan_p4,
			3 + 30,
			28 + (14 / 2) - (6 / 2),
			yield_bar_width, yield_bar_height
		    ))
		    {
			OSWMapRaised(bridge_win.weapon_yield_iwin);
			OSWGrabPointer(
			    bridge_win.weapon_yield_iwin,
			    True,
			    ButtonReleaseMask | PointerMotionMask,
                            GrabModeAsync, GrabModeAsync,
			    bridge_win.weapon_yield_iwin,
			    None
/*
			    ((widget_global.h_split_wcr == NULL) ?
				None : widget_global.h_split_wcr->cursor
			    )
 */
			);

			wa.width = yield_bar_width;
			wa.height = yield_bar_height;

                        /* Adjust weapon yield. */
                        if(wa.width > 1)
                            local_control.weapon_yield =
				(double)(event->xbutton.x - (3 + 30)) /
				    ((double)wa.width - 1);
			else
                            local_control.weapon_yield = 1.0;

                        obj_ptr = net_parms.player_obj_ptr;
                        object_num = net_parms.player_obj_num;
                        if(obj_ptr != NULL)
                        {
                            BridgeWinDrawPanel(   
                                object_num,
                                BPANEL_DETAIL_PWEAPONS
                            );
                        }   

		    }
                }

                /* Select weapon 1. */
                if(BW_IS_IN_BTNPOS(
                    BPANEL_BTNPOS_PSELWEP1,
                    event->xbutton.x,
                    event->xbutton.y
                   ) &&
                   (obj_ptr != NULL)
                )    
                {
                    NetSendSelectWeapon(object_num, 0);
                }
                    
                /* Select weapon 2. */
                if(BW_IS_IN_BTNPOS(
                    BPANEL_BTNPOS_PSELWEP2,
                    event->xbutton.x,
                    event->xbutton.y
                   ) &&
                   (obj_ptr != NULL)
                )
                {
                    NetSendSelectWeapon(object_num, 1);
                }
            
                /* Select weapon 3. */
                if(BW_IS_IN_BTNPOS(
                    BPANEL_BTNPOS_PSELWEP3,
                    event->xbutton.x,
                    event->xbutton.y  
                   ) &&
                   (obj_ptr != NULL)
                )
                {
                    NetSendSelectWeapon(object_num, 2);
                }
                     
                /* Select weapon 4. */
                if(BW_IS_IN_BTNPOS(
                    BPANEL_BTNPOS_PSELWEP4,
                    event->xbutton.x,
                    event->xbutton.y  
                   ) &&
                   (obj_ptr != NULL)
                )
                {
                    NetSendSelectWeapon(object_num, 3);
                }
                 
                /* Intercept. */
                if(BW_IS_IN_BTNPOS(
                    BPANEL_BTNPOS_PINTERCEPT,
                    event->xbutton.x,
                    event->xbutton.y  
                   ) &&
                   (obj_ptr != NULL)
                )
                {
                    if(event->xbutton.button == Button1)
                    {
                        /* Query ETA. */
                        NetSendExec("eta");
                    }
                    else if(event->xbutton.button == Button3)
                    {
                        /* Disengage intercept. */
                        NetSendIntercept(object_num, "#off");
                    }
                }

                /* Play button press sound. */
                if(option.sounds >= XSW_SOUNDS_EVENTS)
                {
                    SoundPlay(
                        SOUND_CODE_BUTTONPRESS,
                        1.0, 0.2,	/* Left and right volume. */
			0, 0		/* Effects and prioritu */
                    );
                }

                events_handled++;
            }
	    /* Weapon yield input window. */
	    else if(event->xany.window == bridge_win.weapon_yield_iwin)
	    {
		/* Not suppose to recieve a ButtonPress in this window. */
		OSWUngrabPointer();
		OSWDestroyWindow(&bridge_win.weapon_yield_iwin);
	    }
            /* Subject object console panel section 1. */
            else if(event->xany.window == bridge_win.pan_s1)
            {
                obj_ptr = net_parms.player_obj_ptr;
                object_num = net_parms.player_obj_num;

                /* Lock next. */
                if(BW_IS_IN_BTNPOS(
                    BPANEL_BTNPOS_PLOCKNEXT,
                    event->xbutton.x,
                    event->xbutton.y
                   ) &&
                   (obj_ptr != NULL)
                )
                {
		    switch(event->xbutton.button)
		    {
		      case Button3:
			NetSendWeaponsUnlock(object_num);
			break;

		      case Button2:
			if(obj_ptr == NULL)
			    break;
			BridgePrintSubjectStats(obj_ptr->locked_on);
			break;

		      default:
			NetSendWeaponsLock(object_num, -2);
			break;
		    }
                }

                /* Play button press sound. */
                if(option.sounds >= XSW_SOUNDS_EVENTS) 
                {
                    SoundPlay(
                        SOUND_CODE_BUTTONPRESS,
                        0.2, 1.0,	/* Left and right volume. */
			0, 0		/* Effects and prioritu */
                    );
                }
                events_handled++;
	    }
            /* Subject object console panel section 2. */
            else if(event->xany.window == bridge_win.pan_s2)
	    {
                obj_ptr = net_parms.player_obj_ptr;
                object_num = net_parms.player_obj_num;






                /* Play button press sound. */
                if(option.sounds >= XSW_SOUNDS_EVENTS)
                {
                    SoundPlay(
                        SOUND_CODE_BUTTONPRESS,
                        0.2, 1.0,	/* Left and right volume. */
			0, 0		/* Effects and prioritu */
                    );
                }
                events_handled++;
            }
            /* Subject object console panel section 3. */
            else if(event->xany.window == bridge_win.pan_s3)
            {
                obj_ptr = net_parms.player_obj_ptr;
                object_num = net_parms.player_obj_num;







                /* Play button press sound. */
                if(option.sounds >= XSW_SOUNDS_EVENTS)
                {
                    SoundPlay(
                        SOUND_CODE_BUTTONPRESS,
                        0.2, 1.0,	/* Left and right volume. */
                        0, 0		/* Effects and prioritu */
                    );
                }
                events_handled++;
            }
	    /* Viewscreen. */
            else if((event->xany.window == bridge_win.viewscreen) &&
                    (event->xbutton.button != Button3)
            )
	    {
                obj_ptr = net_parms.player_obj_ptr;

                if(obj_ptr != NULL)
		{
		    object_num = VSButtonMatch(event);
		    if(DBIsObjectGarbage(object_num))
                    {
                        /*   Disengage tractor beam if button pressed is
                         *   Button2 on a failed object match.
                         */
                        if((event->xbutton.button == Button2) &&
                           osw_gui[0].shift_key_state
			)
                        {
                            /* Send tractor beam unlock. */
                            NetSendTractorBeamLock(
                                net_parms.player_obj_num,
                                -1
                            );
                        }
		    }
		    else
		    {
		        /* Button1. */
		        if(event->xbutton.button == Button1)
			{
                            if(osw_gui[0].shift_key_state)
			    {
				/* Hail. */
				NetSendHail(
				    net_parms.player_obj_num,
				    object_num,
                                    obj_ptr->com_channel
				);
			    }
			    else
		            {
				/* Send scanner and weapons lock. */
                                NetSendWeaponsLock(
				    net_parms.player_obj_num,
				    object_num
				);
				/* Play scanner lock sound. */
                                if(option.sounds >= XSW_SOUNDS_EVENTS)
                                {
                                    SoundPlay(   
                                        SOUND_CODE_SCAN_BEEP,
                                        1.0, 1.0,	/* Left and right volume. */
					0, 0		/* Effects and prioritu */
                                    );
                                }
			    }
			}
			/* Button2. */
                        else if(event->xbutton.button == Button2)
                        {
                            if(osw_gui[0].shift_key_state)
                            {
                                /* Send tractor beam lock. */
                                NetSendTractorBeamLock(
				    net_parms.player_obj_num,
				    object_num
				);
                            }
			    else
			    {
				/* IFF. */

			    }
			}
		    }
		}
                events_handled++;
	    }
            /* Scanner. */
            else if(event->xany.window == bridge_win.scanner)
            {
                obj_ptr = net_parms.player_obj_ptr;

		if(obj_ptr != NULL)
                {
                    /* Match object on the scanner window. */
                    object_num = ScannerButtonMatch(event);
		    if(DBIsObjectGarbage(object_num))
		    {
			/* No match. */

			/* Disengage intercept if button pressed is
			 * Button2 on a failed object match.
			 */
			if(event->xbutton.button == Button2)
			{
                            /* Disengage intercept. */
                            NetSendIntercept(
				net_parms.player_obj_num,
				"#off"
			    );
 			}
		    }
		    else
                    {
			/* Matched object. */

                        /* Button1. */
                        if(event->xbutton.button == Button1)
                        {
                            if(osw_gui[0].shift_key_state)
                            {
                                /* Hail. */
                                NetSendHail(
                                    net_parms.player_obj_num,
                                    object_num,
                                    obj_ptr->com_channel
                                );
                            }
			    else
                            {
                                /* Send scanner and weapons lock. */
                                NetSendWeaponsLock(
                                    net_parms.player_obj_num,
                                    object_num
                                );
                                /* Play scanner lock sound. */
                                if(option.sounds >= XSW_SOUNDS_EVENTS)
                                {
                                    SoundPlay(
                                        SOUND_CODE_SCAN_BEEP,
                                        0.2, 1.0,	/* Left and right volume. */
					0, 0		/* Effects and prioritu */
                                    );
                                }
                            }
			}
                        /* Button2. */
                        else if(event->xbutton.button == Button2)
                        {
                            if(osw_gui[0].shift_key_state)
                            {
				/* ??? */
			    }
                            else
                            {
                                /* Send set intercept. */
                                sprintf(stringa, "#%i", (int)object_num);
                                NetSendIntercept(
				    net_parms.player_obj_num,
				    stringa
				);

                                /* Print intercept message. */
                                sprintf(stringa,
				    "Setting course to: %s",
                                    DBGetFormalNameStr(object_num)
                                );
                                MesgAdd(stringa, xsw_color.standard_text);

                                if(option.sounds >= XSW_SOUNDS_EVENTS)
                                {
                                    SoundPlay(
                                        SOUND_CODE_BUTTONPRESS,
                                        0.2, 1.0,	/* Left and right volume. */
					0, 0		/* Effects and prioritu */
                                    );
                                }
                            }
                        }
                        /* Button3. */
                        else if(event->xbutton.button == Button3)
                        {
                            if(osw_gui[0].shift_key_state)
                            {
                                /* Disarm weapon. */
				NetSendWeaponDisarm(
				    net_parms.player_obj_num,
				    object_num
				);
			    }
			    else
			    {
				/* ???. */
			    }
			}
                    }
                }
                events_handled++;
            }
            /* Message box. */
            else if(event->xany.window == bridge_win.mesg_box)
	    {
                if(event->xbutton.button == Button1)
                {
                    MesgWinUnmarkAll();

                    button1_state = True;
                    sel_start_x = event->xbutton.x;
                    sel_start_y = event->xbutton.y;

                    events_handled++;
		}
		else if(event->xbutton.button == Button3)
                {
                    if(!lg_mesg_win.map_state)
		        LMesgWinMap();

                    events_handled++;
		}
            }
            /* Quick menu (should be checked last). */
            else if(event->xbutton.button == Button3)
            {
		/* Map quick menu. */
		if((event->xany.window == bridge_win.toplevel) ||
                   (event->xany.window == bridge_win.viewscreen) ||
                   (event->xany.window == bridge_win.pan_p1) ||
                   (event->xany.window == bridge_win.pan_p2) ||
                   (event->xany.window == bridge_win.pan_p3) ||
                   (event->xany.window == bridge_win.pan_p4) ||
                   (event->xany.window == bridge_win.pan_s1) ||
                   (event->xany.window == bridge_win.pan_s2) ||
                   (event->xany.window == bridge_win.pan_s3)
		)
		{
		    QMenuMap();
		    events_handled++;
		}
            }
            break;

          /* ********************************************************** */
          case ButtonRelease:
            /* Weapon yield input window. */
            if(event->xany.window == bridge_win.weapon_yield_iwin)
            {
                win_attr_t wa; 

                /* Adjust weapon yield. */
                OSWGetWindowAttributes(bridge_win.weapon_yield_iwin, &wa);

                if(wa.width > 1)   
                    local_control.weapon_yield =
                        (double)event->xbutton.x / ((double)wa.width - 1);
                else
                    local_control.weapon_yield = 1.0;

                obj_ptr = net_parms.player_obj_ptr;
                object_num = net_parms.player_obj_num;
                if(obj_ptr != NULL)
                {
                    BridgeWinDrawPanel(
                        object_num,
                        BPANEL_DETAIL_PWEAPONS  
                    );
                }

		/* Ungrab pointer and destroy weapon yield input
		 * window.
		 */
		OSWUngrabPointer();
                OSWDestroyWindow(&bridge_win.weapon_yield_iwin);
            }
            /* Message box. */
            else if(event->xany.window == bridge_win.mesg_box)
            {
                if(event->xbutton.button == Button1)
                {
                    MesgPutDDE();
                    button1_state = False;
                }
		events_handled++;
	    }
	    break;

	  /* ********************************************************* */
	  case MotionNotify:
            /* Weapon yield input window. */
            if(event->xany.window == bridge_win.weapon_yield_iwin)
            {
		win_attr_t wa;

                /* Adjust weapon yield. */
		OSWGetWindowAttributes(bridge_win.weapon_yield_iwin, &wa);

		if(wa.width > 1)
                    local_control.weapon_yield =
                        (double)event->xmotion.x / ((double)wa.width - 1);
		else
		    local_control.weapon_yield = 1.0;

                obj_ptr = net_parms.player_obj_ptr;
                object_num = net_parms.player_obj_num;
		if(obj_ptr != NULL)
		{
		    BridgeWinDrawPanel(
			object_num,
			BPANEL_DETAIL_PWEAPONS
		    );
		}
	    }
	    /* Message box. */
            else if(event->xany.window == bridge_win.mesg_box)
            {
                if(button1_state)
                {
		    /* Unmark all. */
                    MesgWinUnmarkAll();

		    /* Mark. */
                    BridgeMessagesMark(
                        sel_start_x,
                        sel_start_y,
                        event->xmotion.x,
                        event->xmotion.y
                    );

		    /* Redraw message box. */
		    BridgeDrawMessages();

		    /* Purge old MotionNotify events. */
                    OSWPurgeOldMotionEvents();

                    events_handled++;
                }
            }
	    break;

          /* ******************************************************** */
	  case Expose:
	    /* Toplevel. */
            if(event->xany.window == bridge_win.toplevel)
	    {
		/* Nothing to draw. */
		events_handled++;
	    }
	    /* Viewscreen. */
            else if(event->xany.window == bridge_win.viewscreen)
            {
                VSDrawViewScreen(
		    net_parms.player_obj_num,
                    bridge_win.viewscreen,
                    bridge_win.viewscreen_image,
		    bridge_win.viewscreen_zoom
		);
                events_handled++;
            }
	    /* Message box. */
	    else if(event->xany.window == bridge_win.mesg_box)
	    {
		BridgeDrawMessages();
                events_handled++;
	    }

	    /* Player object console panels. */
            else if(event->xany.window == bridge_win.pan_p1)
            {
		BridgeWinDrawPanel(
		    net_parms.player_obj_num,
		    BPANEL_DETAIL_P1
		);
                events_handled++;
            } 
            else if(event->xany.window == bridge_win.pan_p2)
            {
                BridgeWinDrawPanel(
		    net_parms.player_obj_num,
		    BPANEL_DETAIL_P2
		);
                events_handled++;
            }
            else if(event->xany.window == bridge_win.pan_p3)
            {
                BridgeWinDrawPanel(
		    net_parms.player_obj_num,
		    BPANEL_DETAIL_P3
		);
                events_handled++;
            }
            else if(event->xany.window == bridge_win.pan_p4)
            {
                BridgeWinDrawPanel(
		    net_parms.player_obj_num,
		    BPANEL_DETAIL_P4
		);
                events_handled++;
            }

	    /* Scanner. */
            else if(event->xany.window == bridge_win.scanner)
            {
                ScannerDraw(
                    net_parms.player_obj_num,
                    bridge_win.scanner,
                    bridge_win.scanner_image,
                    0,
                    0,
                    bridge_win.scanner_zoom
                );
                ScaleBarDraw(&bridge_win.scanner_sb);
                events_handled++;
            }

	    /* Subject object console panels. */
            else if(event->xany.window == bridge_win.pan_s1)
            {
                if(net_parms.player_obj_ptr == NULL)
                    object_num = -1;
                else
                    object_num = net_parms.player_obj_ptr->locked_on;
                BridgeWinDrawPanel(object_num, BPANEL_DETAIL_S1);
                events_handled++;
            }
            else if(event->xany.window == bridge_win.pan_s2)
            {
                if(net_parms.player_obj_ptr == NULL)
                    object_num = -1;
                else
                    object_num = net_parms.player_obj_ptr->locked_on;
                BridgeWinDrawPanel(object_num, BPANEL_DETAIL_S2);
                events_handled++;
            }
            else if(event->xany.window == bridge_win.pan_s3)
            {
                if(net_parms.player_obj_ptr == NULL)
                    object_num = -1;
                else
                    object_num = net_parms.player_obj_ptr->locked_on;
                BridgeWinDrawPanel(object_num, BPANEL_DETAIL_S3);
                events_handled++;
            }
	    break;

          /* ********************************************************* */
          case UnmapNotify:
            if(event->xany.window == bridge_win.toplevel)   
            {
		if(bridge_win.map_state)
		    BridgeWinUnmap();

                events_handled++;
            }
	    break;

          /* ********************************************************* */
          case MapNotify:
            if(event->xany.window == bridge_win.toplevel)
            {
		if(!bridge_win.map_state)
		    BridgeWinMap();

                events_handled++;
            }
            break;

          /* ********************************************************* */
          case FocusOut:
            if(event->xany.window == bridge_win.toplevel)
            {
                bridge_win.is_in_focus = 0;

#ifdef JS_SUPPORT
		/* Close joystick when unfocused. */
		if((option.controller == CONTROLLER_JOYSTICK) &&
                   option.focus_out_js_close
		)
		{
		    for(i = 0; i < total_jsmaps; i++)
		    {
			if(jsmap[i] == NULL)
			    continue;

                        /* Close joystick device. */
                        JSClose(&jsmap[i]->jsd);
		    }
		}
#endif /* JS_SUPPORT */

                events_handled++;
            }
            break;

          /* ********************************************************* */
	  case FocusIn:
	    if(event->xany.window == bridge_win.toplevel)
	    {
		bridge_win.is_in_focus = 1;

#ifdef JS_SUPPORT
                /* Reinitialize joystick as needed. */
                if((option.controller == CONTROLLER_JOYSTICK) &&
                   option.focus_out_js_close      
		)
                {
                    for(i = 0; i < total_jsmaps; i++)
                    {
                        if(jsmap[i] == NULL)
                            continue;

                        /* Initialize joystick device. */
                        JSInit(
                            &jsmap[i]->jsd,
                            jsmap[i]->device_name,
                            fname.js_calib,
                            JSFlagNonBlocking
			);

			JSMapSyncWithData(jsmap[i]);
		    }
                }
#endif /* JS_SUPPORT */

		events_handled++;
	    }
	    break;

          /* ********************************************************* */  
	  case ConfigureNotify:
	    if(event->xany.window == bridge_win.toplevel)
            {
		BridgeWinResize();

                events_handled++;
            }
	    break;

	  /* ********************************************************* */
	  case VisibilityNotify:
	    if(event->xany.window == bridge_win.viewscreen)
	    {
		bridge_win.viewscreen_vis_state = event->xvisibility.state;
	    }
            else if(event->xany.window == bridge_win.scanner)
            {
                bridge_win.scanner_vis_state = event->xvisibility.state;
            }
	    break;

          /* ******************************************************* */
          case ClientMessage:
	    if(OSWIsEventDestroyWindow(bridge_win.toplevel, event))
            {
		/*   Switching to runlevel 1 will cause a nice and
		 *   gentle shutdown.
		 */
                runlevel = 1;

		events_handled++;
	    }
            break;
	}



	/* ********************************************************** */
	/* Manage widgets. */

	/* Quick menu. */
        if(events_handled == 0)
	    events_handled += QMenuManage(event);

	/* Prompt. */
	if(events_handled == 0)
	    events_handled += PromptManage(&bridge_win.prompt, event);

	/* Scanner scale bar. */
	if(events_handled == 0)
            events_handled += ScaleBarManage(&bridge_win.scanner_sb, event);

	/* Message box scroll bar. */
	if(events_handled == 0)
	{
            events_handled += SBarManage(
	        &bridge_win.mesg_box_sb,
                bridge_win.mesg_box_width, 
                bridge_win.mesg_box_height,
                bridge_win.mesg_box_width,
                bridge_win.line_spacing * (MESG_WIN_TOTAL_MESSAGES + 1),
                event
	    );
	    if(events_handled > 0)
		BridgeDrawMessages();
	}


	return(events_handled);
}
