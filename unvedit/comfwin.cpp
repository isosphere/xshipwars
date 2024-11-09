// unvedit/comfwin.cpp

/*
 *	Global last comfermation result.
 */
int comf_last_result;

/*
                          Comfermation Window


	int ComfWinDoQuery(
	        comfirm_win_struct *cw,
	        char *mesg
	)

	int ComfWinInit(         
	        comfirm_win_struct *cw, 
	        image_t *icon,
	        void *client_data,
	        int (*std_gui_manage_func)(event_t *)
	)
	void ComfWinDraw(comfirm_win_struct *cw)
	int ComfWinManage(comfirm_win_struct *cw, event_t *event)
	void ComfWinMap(comfirm_win_struct *cw)
	void ComfWinMapMesg(comfirm_win_struct *cw, char *mesg)
	void ComfWinUnmap(comfirm_win_struct *cw)
	void ComfWinDestroy(comfirm_win_struct *cw)

	---

 */

/*
#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
*/
#include "../include/string.h"
#include "../include/strexp.h"
#include "../include/osw-x.h"
#include "../include/widget.h"
#include "comfwin.h"


#ifndef MAX
#define MIN(a,b)        (((a) < (b)) ? (a) : (b))
#define MAX(a,b)	(((a) > (b)) ? (a) : (b))
#endif


/*
 *	Global last comfermation result.
 */
/*
int comf_last_result;
*/


/*
 *	Maps cw and blocks untill response.
 */
int ComfWinDoQuery(
	comfirm_win_struct *cw,
	char *mesg
)
{
	int events_handled;
	event_t event;


	if(cw == NULL)
	    return(ComfirmCodeError);


	/* Map comfermation window. */
	ComfWinMapMesg(cw, mesg);

	/* Maintain and block while cw is mapped. */
	while(cw->map_state)
	{
	    usleep(8000);

            events_handled = 0;

	    if(OSWEventsPending() <= 0)
		continue;
        
            OSWWaitNextEvent(&event);

            /*   Let WidgetManage() see this event.
             *   It is not important if the event is handled or not, so
             *   the return value is disgarded.
             */
            WidgetManage(&event);

	    /* Manage comfermation window. */
	    events_handled += ComfWinManage(cw, &event);


	    /*   Let standard management handle if it's one of the
	     *   following events.
	     */
	    if((event.type != KeyPress) &&
               (event.type != KeyRelease) &&
               (event.type != ButtonPress) &&
               (event.type != ButtonRelease) &&
               (event.type != MotionNotify)
	    )
	    {
		if(cw->std_gui_manage_func != NULL)
		    events_handled += cw->std_gui_manage_func(&event);
	    }
	    else
	    {

	    }

	    /* Keep comfermation dialog raised. */
/*
		OSWMapRaised(cw->toplevel);
 */
	}

	return(comf_last_result);
}



int ComfWinInit(
        comfirm_win_struct *cw,
        image_t *icon,
        void *client_data,
	int (*std_gui_manage_func)(event_t *)
)
{
	win_attr_t wattr;
	char hotkey[PBTN_MAX_HOTKEYS];


	if(cw == NULL)
	    return(-1);


	cw->map_state = 0;
	cw->is_in_focus = 0;
	cw->x = 0;
	cw->y = 0;
	cw->width = 200;
	cw->height = 80;

        cw->option = 0;
	cw->mesg = NULL;
        cw->icon = icon;
	cw->client_data = client_data;
	cw->std_gui_manage_func = std_gui_manage_func;


        /* ******************************************************** */
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

        OSWSetWindowWMProperties(
            cw->toplevel,
            "Comfirmation",
            "Comfirmation",
            widget_global.std_icon_pm,		/* Icon. */
            False,				/* Let WM set coordinates? */
            cw->x, cw->y,
            100, 72,
            osw_gui[0].display_width, osw_gui[0].display_height,
            WindowFrameStyleFixedFrameOnly,
            NULL, 0
        );

        OSWSetWindowInput(
            cw->toplevel,
            OSW_EVENTMASK_TOPLEVEL
        );

        WidgetCenterWindow(cw->toplevel, WidgetCenterWindowToRoot);
        OSWGetWindowAttributes(cw->toplevel, &wattr);
        cw->x = wattr.x;
        cw->y = wattr.y;
        cw->width = wattr.width;
	cw->height = wattr.height;


	/* Buttons. */
	if(
	    PBtnInit(
		&cw->yes_btn,
		cw->toplevel,
                (1 * 10) + (0 * CW_BUTTON_WIDTH),
                (int)cw->height - 10 - CW_BUTTON_HEIGHT,
                CW_BUTTON_WIDTH, CW_BUTTON_HEIGHT,
                "Yes",
                PBTN_TALIGN_CENTER,
                NULL,
                (void *)&cw->yes_btn,
                NULL
	    )
	)
	    return(-1);
	hotkey[0] = 'y';
	hotkey[1] = ' ';
	hotkey[2] = '\n';
        hotkey[3] = '\0';
        PBtnSetHotKeys(&cw->yes_btn, hotkey);


        if(
            PBtnInit(
                &cw->no_btn,
                cw->toplevel,
                (2 * 10) + (1 * CW_BUTTON_WIDTH),
                (int)cw->height - 10 - CW_BUTTON_HEIGHT,
                CW_BUTTON_WIDTH, CW_BUTTON_HEIGHT,
                "No",
                PBTN_TALIGN_CENTER,
                NULL,
                (void *)&cw->no_btn,
                NULL  
            )
        )
            return(-1);
        hotkey[0] = 'n';
        hotkey[1] = '\0';
        PBtnSetHotKeys(&cw->no_btn, hotkey);

        if(
            PBtnInit(
                &cw->all_btn,
                cw->toplevel,
                (3 * 10) + (2 * CW_BUTTON_WIDTH),
                (int)cw->height - 10 - CW_BUTTON_HEIGHT,
                CW_BUTTON_WIDTH, CW_BUTTON_HEIGHT,
                "All",
                PBTN_TALIGN_CENTER,
                NULL,
                (void *)&cw->all_btn,
                NULL
            )
        )
            return(-1);
        hotkey[0] = 'a';
        hotkey[1] = '\0';
        PBtnSetHotKeys(&cw->all_btn, hotkey);


        if( 
            PBtnInit(
                &cw->cancel_btn,
                cw->toplevel,
                (4 * 10) + (3 * CW_BUTTON_WIDTH),
                (int)cw->height - 10 - CW_BUTTON_HEIGHT,
                CW_BUTTON_WIDTH, CW_BUTTON_HEIGHT,
                "Cancel",
                PBTN_TALIGN_CENTER,
                NULL,
                (void *)&cw->cancel_btn,
                NULL
            )
        )
            return(-1);
        hotkey[0] = 'c';
        hotkey[1] = 0x1b;
	hotkey[2] = '\0';
        PBtnSetHotKeys(&cw->cancel_btn, hotkey);




	return(0);
}


void ComfWinDraw(comfirm_win_struct *cw)
{
	int x, y;
	int longest_line, total_lines, line_len;
	char *strptr;
	unsigned int width, height;
	win_t w;
	pixmap_t pixmap;
	win_attr_t wattr;


        if(cw == NULL)
            return;


	if(!cw->map_state)
	{
	    /* Adjust size of toplevel. */
	    if(cw->mesg != NULL)
	    {
	        total_lines = strlines(cw->mesg);
		longest_line = strlongestline(cw->mesg);

	        width = MAX(
		    10 + ((cw->icon == NULL) ? 0 : cw->icon->width) +
                    (longest_line * 7) + 30,
		    (CW_BUTTON_WIDTH * 4) + (5 * 10)
		);
	        height = MAX(
		    10 + ((cw->icon == NULL) ? 0 : cw->icon->height) + 20 +
		    CW_BUTTON_HEIGHT + 10,
		    10 + (total_lines * 16) + 20 + CW_BUTTON_HEIGHT + 10
	        );
	    }
	    else
	    {
		width = MAX(
		    10 + ((cw->icon == NULL) ? 0 : cw->icon->width) + 30,
		    100
		);
		height = MAX(
		    10 + ((cw->icon == NULL) ? 0 : cw->icon->height) + 20 +
                    CW_BUTTON_HEIGHT + 10,
                    80
		);
	    }

	    OSWResizeWindow(cw->toplevel, width, height);
	    OSWGUISync(False);
	    WidgetCenterWindow(cw->toplevel, WidgetCenterWindowToPointer);
	    OSWGetWindowAttributes(cw->toplevel, &wattr);
            OSWGUISync(False);

	    cw->x = wattr.x;
	    cw->y = wattr.y;
	    cw->width = wattr.width;
	    cw->height = wattr.height;


	    /* Move yes button. */
	    x = (int)cw->width - (2 * 10) - (2 * CW_BUTTON_WIDTH);
	    if(cw->option & ComfirmOptionAll)
		x -= (10 + CW_BUTTON_WIDTH);
            if(cw->option & ComfirmOptionCancel)
                x -= (10 + CW_BUTTON_WIDTH);
	    OSWMoveWindow(
		cw->yes_btn.toplevel,
		x,
		(int)cw->height - 10 - CW_BUTTON_HEIGHT
	    );

            /* Move no button. */
            x = (int)cw->width - (1 * 10) - (1 * CW_BUTTON_WIDTH);
            if(cw->option & ComfirmOptionAll)
                x -= (10 + CW_BUTTON_WIDTH);
            if(cw->option & ComfirmOptionCancel)
                x -= (10 + CW_BUTTON_WIDTH);
            OSWMoveWindow(
                cw->no_btn.toplevel,
                x,
                (int)cw->height - 10 - CW_BUTTON_HEIGHT
            );

            /* Move all button as needed. */
	    if(cw->option & ComfirmOptionAll)
	    {
		x = (int)cw->width - (1 * 10) - (1 * CW_BUTTON_WIDTH);
                if(cw->option & ComfirmOptionCancel)
                    x -= (10 + CW_BUTTON_WIDTH);
                OSWMoveWindow(
                    cw->all_btn.toplevel,
                    x,
                    (int)cw->height - 10 - CW_BUTTON_HEIGHT
                );
	    }

            /* Move help button as needed. */
            if(cw->option & ComfirmOptionCancel)
	    {
                x = (int)cw->width - (1 * 10) - (1 * CW_BUTTON_WIDTH);
                OSWMoveWindow(
                    cw->cancel_btn.toplevel,
                    x,
                    (int)cw->height - 10 - CW_BUTTON_HEIGHT
                );
	    }
            OSWGUISync(False);


	    /* Map windows. */
            OSWMapRaised(cw->toplevel);
            PBtnMap(&cw->yes_btn);
            PBtnMap(&cw->no_btn);

            if(cw->option & ComfirmOptionAll)
                PBtnMap(&cw->all_btn);

            if(cw->option & ComfirmOptionCancel)
                PBtnMap(&cw->cancel_btn);   

            cw->map_state = 1;
	}


	/* Recreate buffers as needed. */
	if(cw->toplevel_buf == 0)
	{
            OSWGetWindowAttributes(cw->toplevel, &wattr);

	    if(
		OSWCreatePixmap(
		    &cw->toplevel_buf, wattr.width, wattr.height
		)
	    )
		return;
	}


	if(1)
	{
            w = cw->toplevel;
            pixmap = cw->toplevel_buf;

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

	    /* Draw messages. */
	    x = ((cw->icon == NULL) ? 0 : (int)cw->icon->width + 10) +
		10 + 5;
	    y = 10 + (14 / 2) + 5;
	    strptr = cw->mesg;
	    while(strptr != NULL)
	    {
		line_len = strlinelen(strptr);
		OSWDrawStringLimited(
		    pixmap,
		    x, y,
                    strptr,
		    line_len
		);

		strptr = strptr + line_len;

		if(*strptr == '\n')
		    strptr += 1;
                if(*strptr == '\r')
                    strptr += 1;

		if(*strptr == '\0')
		    strptr = NULL;

		y += (14 / 2) + 5 + 2;
	    }

	    /* Draw horizontal rule. */
	    if(widget_global.force_mono)
	    {
		OSWSetFgPix(osw_gui[0].white_pix);
		OSWDrawLine(
		    pixmap,
		    0,
		    (int)cw->height - 25 - CW_BUTTON_HEIGHT,
		    cw->width,
		    (int)cw->height - 25 - CW_BUTTON_HEIGHT
		);
	    }
	    else
	    {
                OSWSetFgPix(widget_global.surface_shadow_pix);
                OSWDrawLine(
                    pixmap,
                    0,
                    (int)cw->height - 25 - CW_BUTTON_HEIGHT,
                    (int)cw->width,
                    (int)cw->height - 25 - CW_BUTTON_HEIGHT
                );
                OSWSetFgPix(widget_global.surface_highlight_pix);
                OSWDrawLine(
                    pixmap,
                    0,
                    (int)cw->height - 24 - CW_BUTTON_HEIGHT,
                    (int)cw->width,
                    (int)cw->height - 24 - CW_BUTTON_HEIGHT
                );
	    }

	    /* Icon. */
	    WidgetPutImageRaised(
		pixmap,
		cw->icon,
		10, 10,
		16
	    );


            OSWPutBufferToWindow(w, pixmap);
	}


        return;
}


int ComfWinManage(comfirm_win_struct *cw, event_t *event)
{
	int events_handled = 0;


        if((cw == NULL) ||
           (event == NULL)
	)
            return(events_handled);


	if(!cw->map_state &&
	   (event->type != MapNotify)
	)
	    return(events_handled);

	switch(event->type)
	{
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

          case UnmapNotify:
            if(event->xany.window == cw->toplevel)
            {
                ComfWinUnmap(cw);

                events_handled++;
                return(events_handled);
            }
            break;

          case MapNotify:
            if(event->xany.window == cw->toplevel)
            {
		if(!cw->map_state)
                    ComfWinMap(cw);

                events_handled++;
                return(events_handled);
            }
            break;

	}

	if(events_handled > 0)
	{
	    ComfWinDraw(cw);
	}


        if(events_handled == 0)
	{
	    events_handled += PBtnManage(&cw->no_btn, event);
	    if((events_handled > 0) &&
               ((event->type == ButtonRelease) ||
                (event->type == KeyRelease)
               )
	    )
	    {
                comf_last_result = ComfirmCodeNo;
		ComfWinUnmap(cw);
	    }
	}

        if(events_handled == 0)
        {
	    events_handled += PBtnManage(&cw->yes_btn, event);
            if((events_handled > 0) &&
               ((event->type == ButtonRelease) ||
                (event->type == KeyRelease)
               )
            )
            {   
                comf_last_result = ComfirmCodeYes;
                ComfWinUnmap(cw);
            }
        }

        if(events_handled == 0)
        {
            events_handled += PBtnManage(&cw->all_btn, event);
            if((events_handled > 0) &&
               ((event->type == ButtonRelease) ||
                (event->type == KeyRelease)
               )
            )
            {
                comf_last_result = ComfirmCodeAll;
                ComfWinUnmap(cw);
            }
        }

        if(events_handled == 0)  
        {
	    events_handled += PBtnManage(&cw->cancel_btn, event);
	    if((events_handled > 0) &&
               ((event->type == ButtonRelease) ||
                (event->type == KeyRelease)
               )
            )
            {
                comf_last_result = ComfirmCodeCancel;
                ComfWinUnmap(cw);
            }
        }


        return(events_handled);
}


void ComfWinMap(comfirm_win_struct *cw)
{ 
        if(cw == NULL)
            return;

	cw->map_state = 0;
	ComfWinDraw(cw);


        return;
}


void ComfWinMapMesg(comfirm_win_struct *cw, char *mesg)
{ 
        if(cw == NULL)
            return;


	if(mesg != NULL)
	{
	    free(cw->mesg);
	    cw->mesg = StringCopyAlloc(mesg);
	}

	ComfWinMap(cw);


        return;
}


void ComfWinUnmap(comfirm_win_struct *cw)
{ 
        if(cw == NULL)
            return;


	PBtnUnmap(&cw->no_btn);  
        PBtnUnmap(&cw->yes_btn); 
        PBtnUnmap(&cw->all_btn); 
        PBtnUnmap(&cw->cancel_btn);

	OSWUnmapWindow(cw->toplevel);

	cw->map_state = 0;
	cw->is_in_focus = 0;

	OSWDestroyPixmap(&cw->toplevel_buf);


        return;
}


void ComfWinDestroy(comfirm_win_struct *cw)
{
	if(cw == NULL)
	    return;


	cw->icon = NULL;	/* Icon is shared. */

	free(cw->mesg);
	cw->mesg = NULL;


	PBtnDestroy(&cw->no_btn);
        PBtnDestroy(&cw->yes_btn);
        PBtnDestroy(&cw->all_btn);
        PBtnDestroy(&cw->cancel_btn);

	OSWDestroyPixmap(&cw->toplevel_buf);
	OSWDestroyWindow(&cw->toplevel);



	return;
}

