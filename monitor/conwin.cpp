/*
                          Connection Window Management

	Functions:

	void ConWinUnfocusPrompt(prompt_window_struct *prompt)
	void ConWinRefocusPrompts(
		con_win_struct *cw,
		prompt_window_struct *focus_prompt
	)

	int ConWinConnectPBCB(void *ptr)
	int ConWinCancelPBCB(void *ptr)

	int ConWinInit(
	        con_win_struct *cw,  
	        void *client_data,
	        int (*func_cb)(void *, char *, char *, char *, char *)
	)
	void ConWinDraw(con_win_struct *cw)
	int ConWinManage(con_win_struct *cw, event_t *event)
	void ConWinMap(con_win_struct *cw)
	void ConWinUnmapcon_win_struct *cw)
	void ConWinDestroy(con_win_struct *cw)

	---

 */

#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <stdlib.h>

#include "../include/string.h"
#include "../include/strexp.h"

#include "../include/osw-x.h"
#include "../include/widget.h"

#include "mon.h"
#include "conwin.h"


#define CW_DEF_WIDTH	380
#define CW_DEF_HEIGHT	165

#define CW_BTN_WIDTH	70
#define CW_BTN_HEIGHT	28


void ConWinUnfocusPrompt(prompt_window_struct *prompt);
void ConWinRefocusPrompts(
	con_win_struct *cw,
	prompt_window_struct *focus_prompt
);


void ConWinUnfocusPrompt(prompt_window_struct *prompt)
{
	if(prompt == NULL)
	    return;


	PromptUnmarkBuffer(prompt, PROMPT_POS_END);

	prompt->is_in_focus = 0;

	if(prompt->map_state)
	    PromptDraw(prompt, PROMPT_DRAW_AMOUNT_TEXTONLY);


	return;
}


void ConWinRefocusPrompts(
        con_win_struct *cw,
        prompt_window_struct *focus_prompt
)
{
	if(cw == NULL)
	    return;


	ConWinUnfocusPrompt(&cw->address_prompt);
        ConWinUnfocusPrompt(&cw->port_prompt);
        ConWinUnfocusPrompt(&cw->name_prompt);
        ConWinUnfocusPrompt(&cw->password_prompt);


	PromptMarkBuffer(focus_prompt, PROMPT_POS_END);

	focus_prompt->is_in_focus = 1;

	if(focus_prompt->map_state)
	    PromptDraw(focus_prompt, PROMPT_DRAW_AMOUNT_TEXTONLY);
}

int ConWinConnectPBCB(void *ptr)
{
	char *strptr;
	con_win_struct *cw;
	char address[HOST_NAME_MAX];
	char port[80];
	char name[80];
	char password[80];


	if(ptr == NULL)
	    return(-1);

	cw = (con_win_struct *)ptr;

	if(cw->func_cb != NULL)
	{
	    strptr = PromptGetS(&cw->address_prompt);
	    strncpy(
		address,
		((strptr == NULL) ? "" : strptr),
		HOST_NAME_MAX
	    );
	    address[HOST_NAME_MAX - 1] = '\0';

            strptr = PromptGetS(&cw->port_prompt);
            strncpy(
                port,
                ((strptr == NULL) ? "" : strptr),
                80
            );
            port[80 - 1] = '\0';

            strptr = PromptGetS(&cw->name_prompt);
            strncpy(
                name,
                ((strptr == NULL) ? "" : strptr),
                80
            );
            name[80 - 1] = '\0';

            strptr = PromptGetS(&cw->password_prompt);
            strncpy(
                password,
                ((strptr == NULL) ? "" : strptr),
                80
            );
            password[80 - 1] = '\0';

	    /* Call function. */
	    cw->func_cb(
		cw->client_data,
		address,
		port,
		name,
		password
	    );
	}

        ConWinUnmap(cw);


	return(0);
}



int ConWinCancelPBCB(void *ptr)
{
        con_win_struct *cw;


        if(ptr == NULL)
            return(-1);
        
        cw = (con_win_struct *)ptr;
 
	ConWinUnmap(cw);


	return(0);
}




int ConWinInit(
        con_win_struct *cw,  
        void *client_data,
        int (*func_cb)(void *, char *, char *, char *, char *)
)
{
	win_attr_t wattr;
	char hotkeys[PBTN_MAX_HOTKEYS];

        if(cw == NULL)
            return(-1);

	cw->map_state = 0;
	cw->is_in_focus = 0;
	cw->x = 0;
	cw->y = 0;
	cw->width = CW_DEF_WIDTH;
	cw->height = CW_DEF_HEIGHT;
	cw->visibility_state = VisibilityFullyObscured;

	cw->client_data = client_data;
	cw->func_cb = func_cb;


	/* Create toplevel. */
	if(
            OSWCreateWindow(
                &cw->toplevel,
                osw_gui[0].root_win,
                cw->x, cw->y,
                cw->width, cw->height
            )
        )
            return(-1);

	WidgetCenterWindow(cw->toplevel, WidgetCenterWindowToPointer);
        OSWGetWindowAttributes(cw->toplevel, &wattr);
        cw->x = wattr.x;
        cw->y = wattr.y;
        cw->width = wattr.width;
        cw->height = wattr.height;

        OSWSetWindowWMProperties(
            cw->toplevel,
            "Connect To Universe",	/* Title. */
            "Connect",			/* Icon title. */
            ((mon_image.monitor_icon_pm == 0) ?
                widget_global.std_icon_pm : mon_image.monitor_icon_pm),
            False,		/* Let WM set coordinates? */
            cw->x, cw->y,
            cw->width, cw->height,
	    cw->width, cw->height,
            WindowFrameStyleFixed,
            NULL, 0
        );
        OSWSetWindowInput(
            cw->toplevel,
            OSW_EVENTMASK_TOPLEVEL | ButtonPressMask | ButtonReleaseMask |
            PointerMotionMask | ExposureMask |
	    EnterWindowMask | LeaveWindowMask
        );


	/* Address prompt. */
	if(
	    PromptInit(
                &cw->address_prompt,
                cw->toplevel,
                10, 10 + (30 * 0),
                (int)cw->width - 130,
		30,
                PROMPT_STYLE_FLUSHED,
                "Address:",
                HOST_NAME_MAX,
                0,
                NULL
	    )
        )
	    return(-1);

        /* Port prompt. */
        if(
            PromptInit(
                &cw->port_prompt,
                cw->toplevel,
                (int)cw->width - 130 + 15,
                10 + (30 * 0),
                130 - 25,
                30,
                PROMPT_STYLE_FLUSHED,
                "Port:",
                40,
                0,
                NULL
            )
        )
            return(-1);

        /* Login name prompt. */
        if(
            PromptInit(
                &cw->name_prompt,
                cw->toplevel,
                10,
                10 + (30 * 1) + 5,
                (int)cw->width - 20,
                30,
                PROMPT_STYLE_FLUSHED,
                "    Name:",
                80,
                0,
                NULL
            )
        )
            return(-1);

        /* Login password prompt. */
        if(
            PromptInit(
                &cw->password_prompt,
                cw->toplevel,
                10,
                10 + (30 * 2) + 5, 
                (int)cw->width - 20, 
                30,
                PROMPT_STYLE_FLUSHED,
                "Password:",
                80, 
                0, 
                NULL
            )
        )
            return(-1);

	/* Link prompts togeather. */
	cw->address_prompt.next = &cw->port_prompt;
	cw->address_prompt.prev = &cw->password_prompt;

        cw->port_prompt.next = &cw->name_prompt;
        cw->port_prompt.prev = &cw->address_prompt;

        cw->name_prompt.next = &cw->password_prompt;
        cw->name_prompt.prev = &cw->port_prompt;

        cw->password_prompt.next = &cw->address_prompt;
        cw->password_prompt.prev = &cw->name_prompt;


	/* Connect button. */
	if(
	    PBtnInit(
                &cw->connect_btn,
                cw->toplevel,
                10,
		(int)cw->height - CW_BTN_HEIGHT - 10,
                CW_BTN_WIDTH, CW_BTN_HEIGHT,
                "Connect",
                PBTN_TALIGN_CENTER,
                NULL,
                cw,
                ConWinConnectPBCB
	    )
        )
	    return(-1);
	hotkeys[0] = '\n';
	hotkeys[1] = '\0';
	PBtnSetHotKeys(
            &cw->connect_btn,
            hotkeys
        );

	/* Cancel button. */
        if( 
            PBtnInit(
                &cw->cancel_btn,
                cw->toplevel,
                (int)cw->width - CW_BTN_WIDTH - 10,
                (int)cw->height - CW_BTN_HEIGHT - 10,
                CW_BTN_WIDTH, CW_BTN_HEIGHT,
                "Cancel",
                PBTN_TALIGN_CENTER,
                NULL,
                cw,
                ConWinCancelPBCB
            )
        )
            return(-1);
        hotkeys[0] = 0x1b;
        hotkeys[1] = '\0';
        PBtnSetHotKeys(
            &cw->cancel_btn,
            hotkeys
        );



	return(0);
}


void ConWinDraw(con_win_struct *cw)
{
	int y;
	win_t w;
	pixmap_t pixmap;
	font_t *prev_font;

	win_attr_t wattr;


	if(cw == NULL)
	    return;


	/* Map as needed. */
	if(!cw->map_state)
	{
	    OSWMapRaised(cw->toplevel);
	    cw->map_state = 1;

	    PromptMap(&cw->address_prompt);
	    PromptMap(&cw->port_prompt);
	    PromptMap(&cw->name_prompt);
	    PromptMap(&cw->password_prompt);

	    PBtnMap(&cw->connect_btn);
            PBtnMap(&cw->cancel_btn);
	}


	/* Recreate buffers as needed. */
	if(cw->toplevel_buf == 0)
	{
	    OSWGetWindowAttributes(cw->toplevel, &wattr);
	    if(
		OSWCreatePixmap(&cw->toplevel_buf,
		    wattr.width, wattr.height
	    ))
		return;
	}

	prev_font = OSWQueryCurrentFont();


	/* Redraw toplevel. */
	if(1)
	{
	    w = cw->toplevel;
	    pixmap = cw->toplevel_buf;
	    OSWGetWindowAttributes(w, &wattr);


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
		y = (int)wattr.height - CW_BTN_HEIGHT - 25;
		OSWDrawLine(
		    pixmap,
		    0, y, wattr.width, y
		);

                OSWSetFgPix(widget_global.surface_highlight_pix);
                y = (int)wattr.height - CW_BTN_HEIGHT - 25 + 1;
                OSWDrawLine(
                    pixmap,
                    0, y, wattr.width, y
                );
	    }

	    OSWPutBufferToWindow(w, pixmap);
	}


        OSWSetFont(prev_font);


	return;
}


int ConWinManage(con_win_struct *cw, event_t *event)
{
	keycode_t keycode;
	int events_handled = 0;


	if((cw == NULL) ||
           (event == NULL)
	)
	    return(events_handled);


	if(!cw->map_state &&
           (event->type != MapNotify)
	)
            return(events_handled);


	/* Manage events for monitor normally. */
	switch(event->type)
	{
	  case KeyPress:
	    if(!cw->is_in_focus)
		break;

	    keycode = event->xkey.keycode;

	    break;

          case KeyRelease:
            if(!cw->is_in_focus)
                break;

            keycode = event->xkey.keycode;

            break;

	  case Expose:
	    if(event->xany.window == cw->toplevel)
	    {
	        events_handled++;
	    }
	    break;

	  case FocusIn:
            if(event->xany.window == cw->toplevel)
            {
                cw->is_in_focus = 1;
                events_handled++;
		return(events_handled);
            }
            break;

          case FocusOut:
            if(event->xany.window == cw->toplevel)
            {
                cw->is_in_focus = 0;
                events_handled++;
                return(events_handled);
            }
            break;

          case MapNotify:
            if(event->xany.window == cw->toplevel)
            {
                if(!cw->map_state)
		    ConWinMap(cw);

                events_handled++;
                return(events_handled);
            }
            break;

	  case UnmapNotify:
            if(event->xany.window == cw->toplevel)
            {
                if(cw->map_state)
                    ConWinUnmap(cw);

                events_handled++;
                return(events_handled);
            }
	    break;

          case ClientMessage:
            if(OSWIsEventDestroyWindow(cw->toplevel, event))
            {
                ConWinUnmap(cw);

                events_handled++;
                return(events_handled);
            }
            break;
	}

	if(events_handled > 0)
	{
	    ConWinDraw(cw);
	}


	if(events_handled == 0)
	    events_handled += PBtnManage(&cw->connect_btn, event);
        if(events_handled == 0)
            events_handled += PBtnManage(&cw->cancel_btn, event);

        if(events_handled == 0)
            events_handled += PromptManage(&cw->address_prompt, event);
        if(events_handled == 0)
            events_handled += PromptManage(&cw->port_prompt, event);
        if(events_handled == 0)
            events_handled += PromptManage(&cw->name_prompt, event);
        if(events_handled == 0)
            events_handled += PromptManage(&cw->password_prompt, event);


        return(events_handled);
}


void ConWinMap(con_win_struct *cw)
{

        if(cw == NULL)
            return;


	cw->map_state = 0;
	ConWinDraw(cw);


        ConWinRefocusPrompts(
            cw,
            &cw->address_prompt
        );


	return;
}


void ConWinUnmap(con_win_struct *cw)
{
	if(cw == NULL)
	    return;


	OSWUnmapWindow(cw->toplevel);
	cw->map_state = 0;
	cw->is_in_focus = 0;
	cw->visibility_state = VisibilityFullyObscured;

        OSWDestroyPixmap(&cw->toplevel_buf);


	return;
}


void ConWinDestroy(con_win_struct *cw)
{
	if(cw == NULL)
	    return;


	PBtnDestroy(&cw->connect_btn);
	PBtnDestroy(&cw->cancel_btn);

	PromptDestroy(&cw->address_prompt);
        PromptDestroy(&cw->port_prompt);
        PromptDestroy(&cw->name_prompt);
        PromptDestroy(&cw->password_prompt);

	OSWDestroyWindow(&cw->toplevel);
	OSWDestroyPixmap(&cw->toplevel_buf);

	cw->client_data = NULL;
	cw->func_cb = NULL;


	return;
}
