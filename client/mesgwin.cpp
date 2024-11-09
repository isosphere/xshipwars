/*
                           Large Message Window

	Functions:

	int MesgAdd(char *new_mesg, pixel_t mesg_color)
	int MesgReplace(
		char *new_mesg,
		pixel_t mesg_color,
		long mesg_num
	)

	void MesgWinUnmarkAll()
	void MesgWinUpdateMark(
		int start_x, int start_y,
		int end_x, int end_y
	)
	int MesgPutDDE()

        void MesgDrawAll()

	void LMesgWinDoQuery(
		prompt_window_struct *prompt,
		char *label,
		char *val,
                int mode
	)
	int LMesgWinInit()
	int LMesgWinResize()
	int LMesgWinDraw()
	int LMesgWinManage(event_t *event)
	void LMesgWinMap()
	void LMesgWinUnmap()
	void LMesgWinDestroy()

	---

 */


#include "imagepaths.h"  
#include "keymap.h"

#include "xsw.h"
#include "net.h"


/* Sizes. */
#define MW_DEF_WIDTH	800
#define MW_DEF_HEIGHT	480

#define MW_MIN_WIDTH	320
#define MW_MIN_HEIGHT	240


/* Character sizes in pixels. */
#define MW_CHAR_WIDTH	7
#define MW_CHAR_HEIGHT	14

/* Line spacing. */
#define MW_DEF_LINE_SPACING	16

/* All purpose margin. */
#define MW_MARGIN		10


#define MIN(a,b)        (((a) < (b)) ? (a) : (b))
#define MAX(a,b)        (((a) > (b)) ? (a) : (b))


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


void LMesgWinDoQuery(
	prompt_window_struct *prompt,
        char *label,
        char *val,
        int mode
);




/*
 *      Adds a message to the global messages list.
 */
int MesgAdd(char *new_mesg, pixel_t mesg_color)
{
        int i, n, p, tlen;


        /*   Note: Message lines are numbered from bottom to top,
         *   message 0 would be the bottom most line.
         */

	if(new_mesg == NULL)
	    return(-1);


	/* Get total length of new message. */
        tlen = strlen(new_mesg);
	p = 0;

	while(p < tlen)
        {
            /* Shift messages 'up' */
            for(i = 0; i < (MESG_WIN_TOTAL_MESSAGES - 1); i++)
            {
		n = i + 1;

                strncpy(
                    pri_mesg_buf[i].message,
                    pri_mesg_buf[n].message,
                    MESG_BUF_MAX_MESG_LEN
                );
                pri_mesg_buf[i].message[MESG_BUF_MAX_MESG_LEN - 1] = '\0';

                pri_mesg_buf[i].pixel = pri_mesg_buf[n].pixel;

                pri_mesg_buf[i].sel_start = pri_mesg_buf[n].sel_start;
                pri_mesg_buf[i].sel_end = pri_mesg_buf[n].sel_end;
            }

	    i = MESG_WIN_TOTAL_MESSAGES - 1;

            /* Assign line 0 to contain new_mesg. */

	    /* Set pixel color. */
            pri_mesg_buf[i].pixel = mesg_color;

	    /* Copy message line. */
            strncpy(
		pri_mesg_buf[i].message,
		&new_mesg[p],
		MIN(MESG_BUF_MAX_MESG_LEN, MESG_WIN_COL_WRAP)
	    );
            pri_mesg_buf[i].message[MESG_BUF_MAX_MESG_LEN - 1] = '\0';

	    /* Increment position. */
	    p += MESG_WIN_COL_WRAP;

	    /* Reset mark segments. */
            pri_mesg_buf[i].sel_start = -1;
            pri_mesg_buf[i].sel_end = -1;
	}


        /* Redraw all message windows. */
        MesgDrawAll();


        return(0);
}


/*
 *      Replaces a message.
 */
int MesgReplace(char *new_mesg, pixel_t mesg_color, int mesg_num)
{
        /*   Note: Message lines are numbered from bottom to top,
         *   message 0 would be the bottom most line.
         */

        /* Make sure mesg_num is within range. */
        if(mesg_num >= MESG_WIN_TOTAL_MESSAGES)
            mesg_num = MESG_WIN_TOTAL_MESSAGES - 1;
        if(mesg_num < 0)
            mesg_num = 0;


        /* Set the new message and color. */
        strncpy(
            pri_mesg_buf[mesg_num].message,
            new_mesg,
            MESG_BUF_MAX_MESG_LEN
        );
        pri_mesg_buf[mesg_num].message[MESG_BUF_MAX_MESG_LEN - 1] = '\0';
        pri_mesg_buf[mesg_num].pixel = mesg_color;

        pri_mesg_buf[mesg_num].sel_start = -1;
        pri_mesg_buf[mesg_num].sel_end = -1;


        /* Redraw all message windows. */
        MesgDrawAll();


        return(0);
}


/*
 *      Unmark all message buffer lines.
 */
void MesgWinUnmarkAll()
{
        int i;

        for(i = 0; i < MESG_WIN_TOTAL_MESSAGES; i++)
        {
             pri_mesg_buf[i].sel_start = -1;
             pri_mesg_buf[i].sel_end = -1;
        }

        return;
}


/*
 *	Marks messages by the given rectangular coordinates.
 *
 *	Scroll bar positions will be automatically applied.
 */
void MesgWinUpdateMark(
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
        start_line = (start_y - MW_MARGIN + lg_mesg_win.scroll_bar.y_win_pos) /
            (int)lg_mesg_win.line_spacing;
        if(start_line >= MESG_WIN_TOTAL_MESSAGES)
            start_line = MESG_WIN_TOTAL_MESSAGES - 1;
        if(start_line < 0)
            start_line = 0;


        /* Calculate end line. */
        end_line = (end_y - MW_MARGIN + lg_mesg_win.scroll_bar.y_win_pos) /
            (int)lg_mesg_win.line_spacing;
        if(end_line >= MESG_WIN_TOTAL_MESSAGES)
            end_line = MESG_WIN_TOTAL_MESSAGES - 1;
        if(end_line < 0)
            end_line = 0;


        /* Calculate delta lines. */
        delta_lines = end_line - start_line;

        /*   Begin marking (backwards).
         *
         *   Remember that start_line is equal or greater than
	 *   end_line.
         */

        /* Single line mark? */
        if(start_line == end_line)
        {
            len = strlen(pri_mesg_buf[start_line].message);

            x = (start_x - MW_MARGIN) / MW_CHAR_WIDTH;
            if(x >= len) x = len - 1;
            if(x < 0) x = 0;
            pri_mesg_buf[start_line].sel_start = x; 

            x = (end_x - MW_MARGIN) / MW_CHAR_WIDTH;
            if(x >= len) x = len - 1;
            if(x < 0) x = 0;
            pri_mesg_buf[start_line].sel_end = x;
        }
        else if(start_line < end_line)
        {
            /* Start line. */
            x = (start_x - MW_MARGIN) / MW_CHAR_WIDTH;
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
            x = (end_x - MW_MARGIN) / MW_CHAR_WIDTH;
            len = strlen(pri_mesg_buf[end_line].message);  
            if(x >= len) x = len - 1;
            if(x < 0) x = 0;

            pri_mesg_buf[end_line].sel_start = 0;
            pri_mesg_buf[end_line].sel_end = x;
        }

        return;
}


/*
 *      Stores marked message lines to DDE.
 *      
 *      Returns the number of bytes stored.
 */
int MesgPutDDE()
{
        int i, delta;
        char *buf = NULL;
        int buf_pos = 0;
        int buf_len = 0;


        for(i = 0; i < MESG_WIN_TOTAL_MESSAGES; i++)
        {
	    if((pri_mesg_buf[i].sel_start < 0) ||
               (pri_mesg_buf[i].sel_end < 0)
	    )
		continue;

            /* Sanitize selected start and end positions. */
            if(pri_mesg_buf[i].sel_start >= MESG_BUF_MAX_MESG_LEN)
                pri_mesg_buf[i].sel_start = MESG_BUF_MAX_MESG_LEN - 1;
            if(pri_mesg_buf[i].sel_end >= MESG_BUF_MAX_MESG_LEN)
                pri_mesg_buf[i].sel_end = MESG_BUF_MAX_MESG_LEN - 1;

            /* Get delta size of selection. */
            delta = pri_mesg_buf[i].sel_end - pri_mesg_buf[i].sel_start;
	    /* delta must be 1 or greater. */
            if(delta <= 0)
		continue;
            /* Need to add 1 to delta. */
            delta += 1;
            if((pri_mesg_buf[i].sel_start + delta) >= MESG_BUF_MAX_MESG_LEN)
                delta = MESG_BUF_MAX_MESG_LEN - pri_mesg_buf[i].sel_start - 1;
            if(delta <= 0)
		continue;

            /* If this is second or greater line, add \n to buffer. */
            if(buf_len > 0)
            {
                buf_len += 1;
                buf = (char *)realloc(buf, buf_len * sizeof(char));
                if(buf == NULL)
		    return(0); 
                buf_pos = buf_len - 1;

                buf[buf_pos] = '\n';
            }

            /* Increase buffer size. */
            buf_pos = buf_len;
            buf_len += delta;
            buf = (char *)realloc(buf, buf_len * sizeof(char));
            if(buf == NULL)
		return(0);

            memcpy(
                &buf[buf_pos],  /* Target. */
                &pri_mesg_buf[i].message[
                    pri_mesg_buf[i].sel_start   /* Source. */
                ],
                delta
            );
        }


        OSWPutDDE(buf, buf_len);

        free(buf);
	buf = NULL;


        return(buf_len);
}


/*
 *	Procedure to redraw all windows displaying global
 *	messages if they are mapped.
 */
void MesgDrawAll()
{
        /* Bridge window message box. */
        if(bridge_win.map_state)
        {
            BridgeDrawMessages(); 
            SBarDraw(
                &bridge_win.mesg_box_sb,
                bridge_win.mesg_box_width,
                bridge_win.mesg_box_height,
                bridge_win.mesg_box_width,
                bridge_win.line_spacing * MESG_WIN_TOTAL_MESSAGES
            );
        }

        /* Large message window. */
        if(lg_mesg_win.map_state)
        {
            LMesgWinDraw();
            SBarDraw(
                &lg_mesg_win.scroll_bar,
                lg_mesg_win.width,
                lg_mesg_win.height,
                lg_mesg_win.width,
                lg_mesg_win.line_spacing * (MESG_WIN_TOTAL_MESSAGES + 5)
            );
        }

	return;
}


/*
 *	Procedure to set prompt buffer, change prompt's name,
 *	set global prompt_mode and map prompt.
 */
void LMesgWinDoQuery(
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
 *	Procedure to initialize large message window.
 */
int LMesgWinInit()
{
        char stringa[256];
        int x = 0;
	int y = 0;
	pixmap_t pixmap;
	win_attr_t wattr;
	image_t *image;


        if(!IDC())
            return(-1);


	/* Get background image pointer. */
	if(IMGIsImageNumAllocated(IMG_CODE_MESG_SCR_BKG))
	    image = xsw_image[IMG_CODE_MESG_SCR_BKG]->image;
	else
	    image = NULL;


	/* Set coordinates. */
        lg_mesg_win.map_state = 0;
	lg_mesg_win.is_in_focus = 0;
	lg_mesg_win.visibility_state = VisibilityFullyObscured;
	lg_mesg_win.x = 0;
	lg_mesg_win.y = 0;
	if(image != NULL)
	{
            lg_mesg_win.width = image->width;
            lg_mesg_win.height = image->height;
	}
	else
	{
	    lg_mesg_win.width = MW_DEF_WIDTH;
	    lg_mesg_win.height = MW_DEF_HEIGHT;
	}

	lg_mesg_win.line_spacing = MW_DEF_LINE_SPACING;


	/* Create toplevel. */
	if(
	    OSWCreateWindow(
	        &lg_mesg_win.toplevel,
                osw_gui[0].root_win,
                x, y,
                lg_mesg_win.width, lg_mesg_win.height
	    )
        )
	    return(-1);
	WidgetCenterWindow(lg_mesg_win.toplevel, WidgetCenterWindowToRoot);

	OSWGetWindowAttributes(lg_mesg_win.toplevel, &wattr);
	lg_mesg_win.x = wattr.x;
        lg_mesg_win.y = wattr.y;
	lg_mesg_win.width = wattr.width;
	lg_mesg_win.height = wattr.height;

	/* Buffer for toplevel will be created when mapped. */
	lg_mesg_win.toplevel_buf = 0;

        /* WM Properties. */
        sprintf(stringa, "%s - Messages", PROG_NAME);
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
            lg_mesg_win.toplevel,
            stringa,            /* Title. */
            "Messages",		/* Icon title. */
            pixmap,             /* Icon. */
            False,		/* Let WM set coordinates? */
            lg_mesg_win.x, lg_mesg_win.y,
            MW_MIN_WIDTH, MW_MIN_HEIGHT,
            osw_gui[0].display_width, osw_gui[0].display_height,
            WindowFrameStyleStandard,
            NULL, 0
        );

	/* Make message window transient for bridge window. */
/*
	OSWSetTransientFor(lg_mesg_win.toplevel, bridge_win.toplevel);
 */

	WidgetSetWindowCursor(lg_mesg_win.toplevel, xsw_cursor.text);
        OSWSetWindowInput(lg_mesg_win.toplevel,
	    OSW_EVENTMASK_TOPLEVEL |
	    ButtonPressMask | ButtonReleaseMask | PointerMotionMask
	);



        /* Create prompt. */
        PromptInit(
            &lg_mesg_win.prompt,
            lg_mesg_win.toplevel,
            16, MAX((int)lg_mesg_win.height - 46, 0),
            MAX((int)lg_mesg_win.width - 32, 100), 30,
	    PROMPT_STYLE_RAISED,
	    "Command:",
	    DEF_PROMPT_BUF_LEN,
	    DEF_PROMPT_HIST_BUFS,
	    NULL
        );


	/* Create scroll bars. */
        SBarInit(
	    &lg_mesg_win.scroll_bar,
	    lg_mesg_win.toplevel,
            lg_mesg_win.width, lg_mesg_win.height
	);


	/* Need to set the y position of the scroll bar to the bottom. */
	lg_mesg_win.scroll_bar.y_win_pos = 
	    ((MESG_WIN_TOTAL_MESSAGES + 5) * lg_mesg_win.line_spacing) -
	    lg_mesg_win.height;


	/*   Need to unmark all message buffers, this will reset them
	 *   too.
	 */
	MesgWinUnmarkAll();


	return(0);
}


/*
 *	Resizes the message window.
 */
int LMesgWinResize()
{
	win_attr_t wattr;


	OSWGetWindowAttributes(lg_mesg_win.toplevel, &wattr);
	/* No change in size? */
	if((lg_mesg_win.width == (unsigned int)wattr.width) &&
           (lg_mesg_win.height == (unsigned int)wattr.height)
	)
	    return(0);

	lg_mesg_win.x = wattr.x;
	lg_mesg_win.y = wattr.y;
	lg_mesg_win.width = wattr.width;
	lg_mesg_win.height = wattr.height;


	/* Move prompt. */
	OSWMoveResizeWindow(
	    lg_mesg_win.prompt.toplevel,
            16, MAX((int)lg_mesg_win.height - 46, 0),
	    MAX((int)lg_mesg_win.width - 32, 100), 30
	);
	OSWDestroyPixmap(&lg_mesg_win.toplevel_buf);

	/* Resize scroll bar. */
	SBarResize(
	    &lg_mesg_win.scroll_bar,
	    lg_mesg_win.width,
	    lg_mesg_win.height
	);

        /* Set the y position of scroll bar to the end. */
        lg_mesg_win.scroll_bar.y_win_pos =
            ((MESG_WIN_TOTAL_MESSAGES + 5) * lg_mesg_win.line_spacing) -
            lg_mesg_win.height;



	/* ********************************************************* */
	/* Resize the messages window background image. */
	IMGResize(
	    IMG_CODE_MESG_SCR_BKG,
	    lg_mesg_win.width,
	    lg_mesg_win.height
	);


	return(0);
}


int LMesgWinManage(event_t *event)
{
	char *strptr;
	long object_num;
	xsw_object_struct *obj_ptr;

	char stringa[MAX_URL_LEN + 256];
        int events_handled = 0;


	if(event == NULL)
            return(events_handled);


	if(!lg_mesg_win.map_state &&
           (event->type != MapNotify)
	)
	    return(events_handled);


	/* Handle prompt executions. */
        if(lg_mesg_win.is_in_focus &&
           ((event->type == KeyPress) ||
            (event->type == KeyRelease)
	   ) &&
	   (prompt_mode > PROMPT_CODE_NONE) &&
           (lg_mesg_win.prompt.buf != NULL)
        )
        {
            /* Enter key. */
            if((event->xkey.keycode == osw_keycode.enter) ||
               (event->xkey.keycode == osw_keycode.np_enter)
            )
            {
                /* See which prompt we were in. */
                switch(prompt_mode)
                {
                  /* Connect. */
                  case PROMPT_CODE_CONNECT:
		    XSWDoConnect(lg_mesg_win.prompt.buf);
                    break;

                  /* Client command. */
                  case PROMPT_CODE_CLIENT_CMD:
                    CmdHandleInput(lg_mesg_win.prompt.buf);
                    break;

                  /* Server command. */
                  case PROMPT_CODE_SERVER_CMD:
                    NetSendExec(lg_mesg_win.prompt.buf);
                    break;

                  /* Message. */
                  case PROMPT_CODE_MESSAGE:
		    obj_ptr = net_parms.player_obj_ptr;
		    object_num = net_parms.player_obj_num;
                    if(obj_ptr != NULL)
                    {
                        NetSendComMessage(
                            object_num,
                            -1,
                            obj_ptr->com_channel,
                            lg_mesg_win.prompt.buf
                        );
                    }
                    break;

                  /* Shield frequency. */
                  case PROMPT_CODE_SHIELDFREQ:
                    obj_ptr = net_parms.player_obj_ptr;
                    object_num = net_parms.player_obj_num;

                    if(obj_ptr != NULL)                   
                    {
                        NetSendSetShields(
			    object_num,
                            obj_ptr->shield_state,
                            atof(lg_mesg_win.prompt.buf)
                        );
                        BridgeWinDrawPanel(
                            net_parms.player_obj_num,
                            BPANEL_DETAIL_PSHIELDFREQ   
                        );
                    }
                    break;

                  /* Weapon frequency. */
		  case PROMPT_CODE_WEAPONFREQ:
                    StringStripSpaces(lg_mesg_win.prompt.buf);
                    local_control.weapon_freq = atof(lg_mesg_win.prompt.buf);
                    BridgeWinDrawPanel(
                        net_parms.player_obj_num,
                        BPANEL_DETAIL_PWEAPONFREQ
                    );
		    break;

                  /* Intercept. */
                  case PROMPT_CODE_INTERCEPT:
                    obj_ptr = net_parms.player_obj_ptr;
                    object_num = net_parms.player_obj_num;

                    if(obj_ptr != NULL)
                    {
			strptr = lg_mesg_win.prompt.buf;
			StringStripSpaces(strptr);
                        if((strptr == NULL) ? 0 : (strptr[0] != '\0'))
                        {
                            NetSendIntercept(
				object_num,
                                strptr
			    );
                        }
                        else
                        {
                            NetSendIntercept(object_num, "#off");
                        }
                    }
                    break;

                  /* Exit. */
                  case PROMPT_CODE_EXIT:
                    CmdExit(PromptGetS(&lg_mesg_win.prompt));
                    break;

                  /* Com channel. */
                  case PROMPT_CODE_COM_CHANNEL:
		    obj_ptr = net_parms.player_obj_ptr;
		    object_num = net_parms.player_obj_num;
                    if(obj_ptr != NULL)
                    {
			StringStripSpaces(lg_mesg_win.prompt.buf);
                        NetSendSetChannel(
                            object_num,
                            (int)((double)atof(lg_mesg_win.prompt.buf) *
                                100
                            )
                        );
                    }
                    break;

                  /* Default. */
                  default:
                    MesgAdd("Unknown prompt mode.", xsw_color.standard_text);
                    break;
                }

                /* Send enter key to PromptManage()
                 * it needs to know about this KeyPress event as well.
                 */
                events_handled += PromptManage(
                    &lg_mesg_win.prompt,
                    event
                );

                /* Close the prompt window. */
                PromptClose(&lg_mesg_win.prompt);

                /* Set the global variable prompt_mode to 0. */
                prompt_mode = PROMPT_CODE_NONE;

                events_handled++;
            }

            /* Forward event to the prompt. */
            else
            {
                events_handled += PromptManage(
                    &lg_mesg_win.prompt,
                    event
                );
            }


            /* Return, do not continue. */
            return(events_handled);
        }


	/* Handle normal events. */
	switch(event->type)
	{
          /* ******************************************************* */
	  case ButtonPress:
            if(event->xany.window == lg_mesg_win.toplevel)
	    {
		/* Put scroll bar into focus. */
                lg_mesg_win.scroll_bar.is_in_focus = 1;

		/* Start mark text. */
		if(event->xbutton.button == Button1)
		{
		    MesgWinUnmarkAll();

		    button1_state = True;
		    sel_start_x = event->xbutton.x;
		    sel_start_y = event->xbutton.y;

		    LMesgWinDraw();

                    events_handled++;
                    return(events_handled);
		}
		/* Button3: Unmap. */
                else if(event->xbutton.button == Button3)
                {
		    LMesgWinUnmap();

                    events_handled++;
                    return(events_handled);
		}

                events_handled++;
            }
	    break;

          /* ******************************************************* */
	  case ButtonRelease:
	    if(event->xany.window == lg_mesg_win.toplevel)
	    {
		MesgPutDDE();
                button1_state = False;

		LMesgWinDraw();
		events_handled++;
	    }
	    break;

	  /* ******************************************************* */
	  case MotionNotify:
	    if(event->xany.window == lg_mesg_win.toplevel)
	    {
		/* Mark text. */
		if(button1_state)
		{
		    MesgWinUnmarkAll();

		    MesgWinUpdateMark(
		        sel_start_x,
		        sel_start_y,
		        event->xmotion.x,
		        event->xmotion.y
		    );

		    OSWPurgeOldMotionEvents();

		    LMesgWinDraw();

		    events_handled++;
		}
	    }
	    break;

          /* ******************************************************* */
	  case KeyPress:
	    /* Skip of not in focus. */
	    if(!lg_mesg_win.is_in_focus)
		return(events_handled);

            /* Skip modifier keys. */
            if((event->xkey.keycode == osw_keycode.shift_left) ||
               (event->xkey.keycode == osw_keycode.shift_right) ||
               (event->xkey.keycode == osw_keycode.alt_left) ||
               (event->xkey.keycode == osw_keycode.alt_right) ||
               (event->xkey.keycode == osw_keycode.ctrl_left) ||
               (event->xkey.keycode == osw_keycode.ctrl_right) ||
               (event->xkey.keycode == osw_keycode.caps_lock)
            )
                return(events_handled);

            /* Print help. */
            if(event->xkey.keycode == xsw_keymap[XSW_KM_HELP].keycode)
            {
                XSWDoHelpMesgWin();
                events_handled++;
            }
            /* Net update interval decrease. */
            else if(event->xkey.keycode == xsw_keymap[XSW_KM_NET_INTERVAL_DEC].keycode)
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
            else if(event->xkey.keycode == xsw_keymap[XSW_KM_NET_INTERVAL_INC].keycode)
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
            /* Net connect. */
            else if(event->xkey.keycode == xsw_keymap[XSW_KM_CONNECT].keycode)
            {
                if(net_parms.connection_state == CON_STATE_CONNECTED)
                {
                    sprintf(stringa,
			"swserv://%s:%s@%s:%i",
                        net_parms.login_name,
                        net_parms.login_password,
                        net_parms.address,
                        net_parms.port
                    );
                    LMesgWinDoQuery(
                        &lg_mesg_win.prompt,
                        "Connect:",
                        stringa,
                        PROMPT_CODE_CONNECT
                    );
                }
                else
                {
                    sprintf(stringa, "Connected to: %s  Port: %i",
                        net_parms.address,
                        net_parms.port
                    );
                    MesgAdd(stringa, xsw_color.standard_text);
                }
                events_handled++;
            }
            /* Net disconnect. */
            else if(event->xkey.keycode == xsw_keymap[XSW_KM_DISCONNECT].keycode)
            {
		XSWDoDisconnect();
                events_handled++;
            }
            /* Net refresh. */
            else if(event->xkey.keycode == xsw_keymap[XSW_KM_REFRESH].keycode)
            {
                XSWDoRefresh();
                events_handled++;
            }
            /* Net connect last. */
            else if(event->xkey.keycode == xsw_keymap[XSW_KM_CONNECTLAST].keycode)
            {
                NetOpenConnection(net_parms.address, net_parms.port);
                events_handled++;
            }
            /* Send hail. */
            else if(event->xkey.keycode == xsw_keymap[XSW_KM_HAIL].keycode)
            {
                obj_ptr = net_parms.player_obj_ptr;
                object_num = net_parms.player_obj_num;

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
            /* Server command. */
            else if(event->xkey.keycode == xsw_keymap[XSW_KM_SERVER_CMD].keycode)
            {
                if(net_parms.connection_state == CON_STATE_CONNECTED)
                {
                    LMesgWinDoQuery(
                        &lg_mesg_win.prompt,
                        "Server Command:", 
                        NULL,
                        PROMPT_CODE_SERVER_CMD
                    );
                }
                events_handled++;
            }
            /* Client command. */
            else if(event->xkey.keycode == xsw_keymap[XSW_KM_CLIENT_CMD].keycode)
            {
                LMesgWinDoQuery(
                    &lg_mesg_win.prompt,
                    "Client Command:",
                    NULL,
                    PROMPT_CODE_CLIENT_CMD
                );
                events_handled++;
            }
            /* Set intercept. */
            else if(event->xkey.keycode == xsw_keymap[XSW_KM_SET_INTERCEPT].keycode)
            {
                obj_ptr = net_parms.player_obj_ptr;
                object_num = net_parms.player_obj_num;

                if(obj_ptr != NULL)
                {
                    LMesgWinDoQuery(
                        &lg_mesg_win.prompt,
                        "Intercept:",
                        NULL,
                        PROMPT_CODE_INTERCEPT
                    );
                }
                events_handled++;
            }
            /* Set weapon frequency. */
            else if(event->xkey.keycode == xsw_keymap[XSW_KM_WEAPON_FREQ].keycode)
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
                    LMesgWinDoQuery(
                        &lg_mesg_win.prompt,
                        "Weapon Frequency:",
                        stringa,
                        PROMPT_CODE_WEAPONFREQ
                    );
                }
                events_handled++;
            }
            /* Set shield frequency. */
            else if(event->xkey.keycode == xsw_keymap[XSW_KM_SHIELD_FREQ].keycode)
            {
                obj_ptr = net_parms.player_obj_ptr;
                object_num = net_parms.player_obj_num;

                if(obj_ptr != NULL)
                {
                    sprintf(stringa,
                        "%.2f",
                        obj_ptr->shield_frequency
                    );
                    LMesgWinDoQuery(
                        &lg_mesg_win.prompt,
                        "Shield Frequency:",
                        stringa,
                        PROMPT_CODE_SHIELDFREQ
                    );
                }
                events_handled++;
            }
            /* Set com channel. */
            else if(event->xkey.keycode == xsw_keymap[XSW_KM_SET_CHANNEL].keycode)
            {
		obj_ptr = net_parms.player_obj_ptr;
		object_num = net_parms.player_obj_num;

                if(obj_ptr != NULL)
                {
                    sprintf(
                        stringa,
                        "%.2f",
                        (double)((double)obj_ptr->com_channel / 100.0)
                    );
                    LMesgWinDoQuery(
                        &lg_mesg_win.prompt,
                        "Communications Channel:",
                        stringa,
                        PROMPT_CODE_COM_CHANNEL
                    );
                }
                events_handled++; 
            }
            /* Send message. */
            else if(event->xkey.keycode == xsw_keymap[XSW_KM_SEND_MESSAGE].keycode)
            {
                LMesgWinDoQuery(
                    &lg_mesg_win.prompt,
                    "Message:",
                    NULL,
                    PROMPT_CODE_MESSAGE
                );
                events_handled++;
            }
            /* Exit. */
            else if(event->xkey.keycode == xsw_keymap[XSW_KM_EXIT].keycode)
            {
                LMesgWinDoQuery(
                    &lg_mesg_win.prompt,
                    "Exit?:",
                    NULL,
                    PROMPT_CODE_EXIT
                );
                events_handled++;
            }
            break;

          /* ******************************************************* */
          case KeyRelease:
            /* Skip of not in focus. */
            if(!lg_mesg_win.is_in_focus)
                return(events_handled);

            break;

          /* ******************************************************* */
          case Expose:
	    if(event->xany.window == lg_mesg_win.toplevel)
	    {
                LMesgWinDraw();

		events_handled++;
	    }
	    break;

          /* ******************************************************* */
          case UnmapNotify:
            if(event->xany.window == lg_mesg_win.toplevel)
            {
		if(lg_mesg_win.map_state)
		    LMesgWinUnmap();

                events_handled++;
		return(events_handled);
            }
	    break;

          /* ******************************************************* */
          case MapNotify:
            if(event->xany.window == lg_mesg_win.toplevel)
            {
                if(!lg_mesg_win.map_state)
		    LMesgWinMap();

                events_handled++;
                return(events_handled);
            }
            break;

          /* ******************************************************* */
          case ConfigureNotify:
            if(event->xany.window == lg_mesg_win.toplevel)
	    {
		LMesgWinResize();

		LMesgWinDraw();
                SBarDraw(
                    &lg_mesg_win.scroll_bar,
                    lg_mesg_win.width,
                    lg_mesg_win.height,
                    lg_mesg_win.width,
                    lg_mesg_win.line_spacing * (MESG_WIN_TOTAL_MESSAGES + 5)
                );

		events_handled++;
	    }
	    break;

          /* ******************************************************* */
          case FocusOut:
            if(event->xany.window == lg_mesg_win.toplevel)
            {
                lg_mesg_win.is_in_focus = 0;

                events_handled++;
                return(events_handled);
            }
            break;

          /* ******************************************************* */
          case FocusIn:
            if(event->xany.window == lg_mesg_win.toplevel)
            {
                lg_mesg_win.is_in_focus = 1;

                events_handled++;
		return(events_handled);
            }
            break;

	  /* ******************************************************* */
	  case VisibilityNotify:
	    if(event->xany.window == lg_mesg_win.toplevel)
	    {
		lg_mesg_win.visibility_state =
		    event->xvisibility.state;

		events_handled++;
		return(events_handled);
	    }
	    break;

          /* ******************************************************* */
          case ClientMessage:
	    if(OSWIsEventDestroyWindow(lg_mesg_win.toplevel, event))
	    {
		LMesgWinUnmap();

		events_handled++;
                return(events_handled);
            }
	    break;
	}


	/* ********************************************************** */
	/* Manage widgets. */

	/* Scroll bar. */
	if(events_handled == 0)
	{
            events_handled += SBarManage(
                &lg_mesg_win.scroll_bar,
                lg_mesg_win.width,
	        lg_mesg_win.height,
                lg_mesg_win.width,
		lg_mesg_win.line_spacing * (MESG_WIN_TOTAL_MESSAGES + 5),
                event
            );
            if(events_handled > 0)
            {
                LMesgWinDraw();
            }
	}

	/* Prompt. */
	if(events_handled == 0)
        {
	    events_handled += PromptManage(
		&lg_mesg_win.prompt, event
	    );
	}


	return(events_handled);
}



int LMesgWinDraw()
{
        int i, lines_drawn, lines_visable;
	int c_pos;
        int line_x, line_y;
	image_t *image;

	win_attr_t wattr;



        /* Map as needed. */
        if(!lg_mesg_win.map_state)
	{
	    OSWMapRaised(lg_mesg_win.toplevel);

	    SBarDraw(
                &lg_mesg_win.scroll_bar,
                lg_mesg_win.width,
                lg_mesg_win.height,
                lg_mesg_win.width,
                lg_mesg_win.line_spacing * (MESG_WIN_TOTAL_MESSAGES + 5)
	    );

	    lg_mesg_win.map_state = 1;
	}

        /* Recreate big buffers as needed. */
        if(lg_mesg_win.toplevel_buf == 0)
        {
            OSWGetWindowAttributes(lg_mesg_win.toplevel, &wattr);
            if(
                OSWCreatePixmap(&lg_mesg_win.toplevel_buf,
                    wattr.width, wattr.height
                )
            )  
                return(-1);
        }


        OSWGetWindowAttributes(lg_mesg_win.toplevel, &wattr);

        /* Draw background. */
	if(IMGIsImageNumAllocated(IMG_CODE_MESG_SCR_BKG))
	{
	    image = xsw_image[IMG_CODE_MESG_SCR_BKG]->image;
	    if((image != NULL) &&
               !(widget_global.force_mono)
	    )
	    {
                OSWPutImageToDrawable(
                    image,
                    lg_mesg_win.toplevel_buf
        	);
	    }
	    else
	    {
		OSWClearPixmap(
		    lg_mesg_win.toplevel_buf,
		    wattr.width, wattr.height,
		    osw_gui[0].black_pix
		);
	    }
	}


	/* Draw text. */
	if(1)
	{
	    /* Get starting line. */
            i = MAX(
                lg_mesg_win.scroll_bar.y_win_pos /
                    (int)lg_mesg_win.line_spacing,
                0
            );

            line_x = MW_MARGIN;
            line_y = MW_MARGIN - (lg_mesg_win.scroll_bar.y_win_pos %
		(int)lg_mesg_win.line_spacing);

	    lines_drawn = 0;
	    lines_visable = (int)wattr.height /
		(int)lg_mesg_win.line_spacing;

            while(lines_drawn < lines_visable)
            {
                if(i >= MESG_WIN_TOTAL_MESSAGES)
		    break;

	        if((pri_mesg_buf[i].sel_start < 0) ||
                   (pri_mesg_buf[i].sel_end < 0)
                )
	        {
		    /* Draw unselected text. */
		    if(widget_global.force_mono)
		        OSWSetFgPix(osw_gui[0].white_pix);
		    else
                        OSWSetFgPix(pri_mesg_buf[i].pixel);
                    OSWDrawString(
		        lg_mesg_win.toplevel_buf,
                        line_x,
			line_y + ((14 / 2) + 5),
		        pri_mesg_buf[i].message
	            );
	        }
	        else
	        {
		    /* Draw marked background. */
                    if(widget_global.force_mono)
                        OSWSetFgPix(osw_gui[0].white_pix);
                    else
		        OSWSetFgPix(widget_global.surface_selected_pix);
		    OSWDrawSolidRectangle(
		        lg_mesg_win.toplevel_buf,
		        line_x + (pri_mesg_buf[i].sel_start * MW_CHAR_WIDTH),
                        line_y,
		        MAX((pri_mesg_buf[i].sel_end -
                             pri_mesg_buf[i].sel_start + 1) * MW_CHAR_WIDTH,
                            2
		        ),
                        lg_mesg_win.line_spacing
		    );

		    /* Draw first unmarked text. */
                    if(widget_global.force_mono)
                        OSWSetFgPix(osw_gui[0].white_pix);
                    else
                        OSWSetFgPix(pri_mesg_buf[i].pixel);
                    OSWDrawStringLimited(
		        lg_mesg_win.toplevel_buf,
		        line_x,
		        line_y + ((14 / 2) + 5),
		        pri_mesg_buf[i].message,
		        pri_mesg_buf[i].sel_start
		    );
		    /* Draw marked text. */
		    c_pos = pri_mesg_buf[i].sel_start;
		    if((c_pos >= 0) &&
		       (c_pos < MESG_BUF_MAX_MESG_LEN)
		    )
	            {
                        if(widget_global.force_mono)
                            OSWSetFgPix(osw_gui[0].black_pix);
                        else
                            OSWSetFgPix(widget_global.selected_text_pix);
                        OSWDrawStringLimited(
                            lg_mesg_win.toplevel_buf,
                            line_x + (pri_mesg_buf[i].sel_start *
                                MW_CHAR_WIDTH),
                            line_y + ((14 / 2) + 5),
                            &pri_mesg_buf[i].message[c_pos],
                            pri_mesg_buf[i].sel_end -
                                pri_mesg_buf[i].sel_start + 1
                        );
		    }
		    /* Draw last unmarked text. */
                    c_pos = pri_mesg_buf[i].sel_end + 1;
                    if((c_pos >= 0) &&
                       (c_pos < MESG_BUF_MAX_MESG_LEN)
                    )
                    {
                        if(widget_global.force_mono)
                            OSWSetFgPix(osw_gui[0].white_pix);
                        else
                            OSWSetFgPix(pri_mesg_buf[i].pixel);
                        OSWDrawStringLimited(
                            lg_mesg_win.toplevel_buf,
                            line_x + ((pri_mesg_buf[i].sel_end + 1) *
                                MW_CHAR_WIDTH),
                            line_y + ((14 / 2) + 5),
                            &pri_mesg_buf[i].message[c_pos],
                            MESG_BUF_MAX_MESG_LEN -
                                pri_mesg_buf[i].sel_end
                        );
                    }
	        }


	        /* Increment positions. */
                line_y += lg_mesg_win.line_spacing;
                i++;
                lines_drawn++;
            }
	}

	/* Put buffer to window. */
	OSWPutBufferToWindow(lg_mesg_win.toplevel, lg_mesg_win.toplevel_buf);


        return(0);
}


/*
 *	Maps message window, recreating big buffers as needed.
 */
void LMesgWinMap()
{
        /* Unfocus all other windows. */
        XSWDoUnfocusAllWindows();


	/* Reload message screen background as needed. */
	IMGLoadImage(IMG_CODE_MESG_SCR_BKG, IMGPATH_MESG_SCR_BKG);

	/* Resize background image. */
        IMGResize(
            IMG_CODE_MESG_SCR_BKG,
            lg_mesg_win.width,
            lg_mesg_win.height
        );

	lg_mesg_win.map_state = 0;
	LMesgWinDraw();
	lg_mesg_win.is_in_focus = 1;
	lg_mesg_win.visibility_state = VisibilityUnobscured;

	/* Unmap prompt. */
        if(prompt_mode == PROMPT_CODE_NONE)
            PromptClose(&lg_mesg_win.prompt);


        /* Restack all XSW windows. */
        XSWDoRestackWindows();


	return;
}


/*      
 *	Unmaps message window, destroying big buffers as needed.
 */
void LMesgWinUnmap()
{
	PromptUnmap(&lg_mesg_win.prompt);
	if(prompt_mode != PROMPT_CODE_NONE)
	    prompt_mode = PROMPT_CODE_NONE;


	/* Unmap. */
	OSWUnmapWindow(lg_mesg_win.toplevel);
        lg_mesg_win.map_state = 0;  
        lg_mesg_win.is_in_focus = 0;
	lg_mesg_win.visibility_state = VisibilityFullyObscured;

	/* Destroy big buffers. */
	OSWDestroyPixmap(&lg_mesg_win.toplevel_buf);

	/* Unload message screen background. */
	IMGUnload(IMG_CODE_MESG_SCR_BKG);


	return;
}


void LMesgWinDestroy()
{
	if(IDC())
	{
            SBarDestroy(&lg_mesg_win.scroll_bar);
            PromptDestroy(&lg_mesg_win.prompt);

            OSWDestroyPixmap(&lg_mesg_win.toplevel_buf);
	    OSWDestroyWindow(&lg_mesg_win.toplevel);
	}

	/* Reset values. */
        lg_mesg_win.is_in_focus = 0;
        lg_mesg_win.visibility_state = VisibilityFullyObscured;


        return;
}
