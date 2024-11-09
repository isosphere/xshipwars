// unvedit/aboutwin.cpp
/*
                       About Window

	Functions:

	int AboutWinDismissPBCB(void *ptr)

	int AboutWinInit()
	void AboutWinDraw(int amount)
	int AboutWinManage(event_t *event)
	void AboutWinMap()
	void AboutWinUnmap()
	void AboutWinDestroy()

	---

 */

//#include "Master.h"
/* #include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
#include <string.h>
*/
#include "../include/mf.h"
#include "../include/disk.h"

#include "../include/osw-x.h"
#include "../include/widget.h"
#include "uew.h"
#include "ue.h"
#include "aboutwin.h"


namespace static_aboutwin {
	unsigned long	mem_used,
			mem_universe,
			mem_total;
}
/*
static unsigned long	mem_used,
			mem_universe,
			mem_total;
*/
/*
 *	Dismiss button callback.
 */
int AboutWinDismissPBCB(void *ptr)
{
	AboutWinUnmap();

	return(0);
}



int AboutWinInit()
{
	char hotkey[PBTN_MAX_HOTKEYS];
	win_attr_t wattr;
	about_win_struct *aw;


	aw = &about_win;


	aw->map_state = 0;
	aw->is_in_focus = 0;
	aw->x = 0;
	aw->y = 0;
	aw->width = AW_WIDTH;
	aw->height = AW_HEIGHT;

	OSWLoadFont(&aw->font, "6x10");

        /* Create toplevel. */
        if(
            OSWCreateWindow(
                &aw->toplevel,
                osw_gui[0].root_win,
                aw->x, aw->y,
                aw->width, aw->height
            )
        )
            return(-1);

        OSWSetWindowWMProperties(
            aw->toplevel,
            AW_TITLE,
            AW_ICON_TITLE,
            ((ue_image.unvedit_icon_pm == 0) ?
                widget_global.std_icon_pm : ue_image.unvedit_icon_pm),
            False,                      /* Let WM set coordinates? */
            aw->x, aw->y,
            aw->width, aw->height,
            aw->width, aw->height,
            WindowFrameStyleFixed,
            NULL, 0
        );
	OSWSetWindowInput(aw->toplevel, OSW_EVENTMASK_TOPLEVEL);

        WidgetCenterWindow(aw->toplevel, WidgetCenterWindowToRoot);
        OSWGetWindowAttributes(aw->toplevel, &wattr);
        aw->x = wattr.x;
        aw->y = wattr.y;
        aw->width = wattr.width;
        aw->height = wattr.height;


        /* Dismiss button. */
        if(
            PBtnInit(
                &aw->dismiss_btn,
                aw->toplevel,
                ((int)aw->width / 2) - (AW_BUTTON_WIDTH / 2),
                (int)aw->height - AW_BUTTON_HEIGHT - 10,
		AW_BUTTON_WIDTH, AW_BUTTON_HEIGHT,
                "Dismiss",
                PBTN_TALIGN_CENTER,
                NULL,
                (void *)aw,
                AboutWinDismissPBCB
            )
        ) 
            return(-1);
	hotkey[0] = '\n';
	hotkey[1] = 0x1b;
	hotkey[2] = ' ';
	hotkey[3] = '\0';

	PBtnSetHotKeys(&aw->dismiss_btn, hotkey);


	return(0);
}


void AboutWinDraw(int amount)
{
	int i, n, x, y;
	win_t w;
	pixmap_t pixmap;
	image_t *img_ptr;
	win_attr_t wattr;
	font_t *prev_font;
	char text[256];
        about_win_struct *aw;


        aw = &about_win;


        /* Map as needed. */
        if(!aw->map_state)   
        {
            OSWMapRaised(aw->toplevel);
	    aw->map_state = 1;
	    amount = AW_DRAW_AMOUNT_COMPLETE;


            PBtnMap(&aw->dismiss_btn);
	}


	prev_font = OSWQueryCurrentFont();

	if(aw->toplevel_buf == 0)
	{
	    OSWGetWindowAttributes(aw->toplevel, &wattr);
	    if(OSWCreatePixmap(&aw->toplevel_buf, wattr.width, wattr.height))
		return;
	}


	if(amount == AW_DRAW_AMOUNT_COMPLETE)
	{
	    w = aw->toplevel;
	    pixmap = aw->toplevel_buf;

            OSWGetWindowAttributes(w, &wattr);

            /* Redraw background. */
            if(widget_global.force_mono)
            {
                OSWClearPixmap(
                    pixmap,
                    wattr.width, wattr.height,
                    osw_gui[0].black_pix
                );

                x = 10;
                y = 10;

		OSWSetFgPix(osw_gui[0].white_pix);
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
                    (int)wattr.height - 25 - AW_BUTTON_HEIGHT,
                    wattr.width,
                    (int)wattr.height - 25 - AW_BUTTON_HEIGHT
                );
                OSWSetFgPix(widget_global.surface_highlight_pix);
                OSWDrawLine(
                    pixmap,
                    0,
                    (int)wattr.height - 24 - AW_BUTTON_HEIGHT,
                    wattr.width,
                    (int)wattr.height - 24 - AW_BUTTON_HEIGHT
                );

		img_ptr = ue_image.unvedit_logo;
		if(img_ptr != NULL)
		{
		    OSWPutImageToDrawablePos(
		        img_ptr, pixmap,
		        10, 10
		    );

		    x = 10 + (int)img_ptr->width + 10;
		    y = 10;
		}
		else
		{
		    x = 10;
		    y = 10;
		}

                OSWSetFgPix(widget_global.normal_text_pix);
	    }

	    OSWSetFont(aw->font);

	    /* Program version. */
	    sprintf(text, "Program Version: %s", PROG_VERSION);
	    OSWDrawString(pixmap, x + 3, y + (10 / 2) + 3,
		text
	    );

            /* Universe format version (same as program version). */
	    y += 12;
            sprintf(text, "UNV Format Version: %s", PROG_VERSION);
            OSWDrawString(pixmap, x + 3, y + (10 / 2) + 3,
                text
            );

	    /* Memory line 1. */
            y += (12 * 2);
            sprintf(text,
                "Memory:"
            );
            OSWDrawString(pixmap, x + 3, y + (10 / 2) + 3,
                text
            );

            y += 12;
            sprintf(text,
		"    Program Used: %ld kb",
		static_aboutwin::mem_used / 1000
	    );
            OSWDrawString(pixmap, x + 3, y + (10 / 2) + 3,
                text  
            );

	    /* Memory line 2. */
            y += 12;
            sprintf(text,
		"           Total: %ld kb",
                static_aboutwin::mem_total / 1000
            );
            OSWDrawString(pixmap, x + 3, y + (10 / 2) + 3,
                text
            );

            /* Universe memory line 1. */
            y += (12 * 2);
            sprintf(text,
                "Universes:"
	    );
            OSWDrawString(pixmap, x + 3, y + (10 / 2) + 3,
                text
            );

            /* Universe memory line 2. */
            y += 12;
            for(i = 0, n = 0; i < total_uews; i++)
            {
                if(uew[i] == NULL)
                    continue;

                n++;
            }
            sprintf(text,
                "    Loaded: %i",
		n
            );
            OSWDrawString(pixmap, x + 3, y + (10 / 2) + 3,
                text
            );

            /* Universe memory line 3. */
            y += 12;
            sprintf(text,
                "    Memory: %ld kb",
                static_aboutwin::mem_universe / 1000
            );
            OSWDrawString(pixmap, x + 3, y + (10 / 2) + 3,
                text
            );


	    /* Copyright line. */
	    img_ptr = ue_image.unvedit_logo;
	    if(img_ptr != NULL)
	    {
		x = 10;
		y = 10 + (int)img_ptr->height + 10;
	    }
	    else
	    {
                y += 12;
	    }

	    sprintf(text,
		AW_COPYRIGHT_MESG
            );
            OSWDrawString(pixmap,
		x + 3 + ((int)aw->width / 2) -
		    ((strlen(text) * 6) / 2),
		y + (10 / 2) + 3,
                text
            );

            y += 12;
            sprintf(text,
                AW_URL_MESG
            );
            OSWDrawString(pixmap,
                x + 3 + ((int)aw->width / 2) -
                    ((strlen(text) * 6) / 2), 
                y + (10 / 2) + 3,
                text
            );

            y += 12;   
            sprintf(text,
                AW_LICENSE_MESG
            );
            OSWDrawString(pixmap,
                x + 3 + ((int)aw->width / 2) -
                    ((strlen(text) * 6) / 2),
                y + (10 / 2) + 3,
                text
            );


            OSWPutBufferToWindow(w, pixmap);
	}

	OSWSetFont(prev_font);


	return;
}


int AboutWinManage(event_t *event)
{
        keycode_t keycode;
        int events_handled = 0;
        about_win_struct *aw;
                
                    
        aw = &about_win;


        if((event == NULL) ||
           !aw->map_state
        )
            return(events_handled);


        switch(event->type)
        {
          /* ******************************************************* */
          case KeyPress:
            if(!aw->is_in_focus)
                return(events_handled);

            keycode = event->xkey.keycode;

	    break;

          /* ******************************************************* */
          case KeyRelease:
            if(!aw->is_in_focus)
                return(events_handled);
 
            keycode = event->xkey.keycode;
 
            break;

          /* ******************************************************* */
          case Expose:
            if(event->xany.window == aw->toplevel)
            {
                events_handled++;
            }
            break;

          /* ******************************************************* */
          case FocusIn:
            if(event->xany.window == aw->toplevel)
            {
                aw->is_in_focus = 1;

                events_handled++;
                return(events_handled);
            }
            break;
                        
          /* ******************************************************* */
          case FocusOut:
            if(event->xany.window == aw->toplevel)
            {
                aw->is_in_focus = 0;

                events_handled++;
                return(events_handled);
            }
            break;

          /* ******************************************************* */
          case ClientMessage:
            if(OSWIsEventDestroyWindow(aw->toplevel, event))
            {
                AboutWinDismissPBCB((void *)aw);

                events_handled++;
                return(events_handled);
            }
            break;
	}


        if(events_handled > 0)
        {
            AboutWinDraw(AW_DRAW_AMOUNT_COMPLETE);
        }


	if(events_handled == 0)
	    events_handled += PBtnManage(&aw->dismiss_btn, event);


	return(events_handled);
}


void AboutWinMap()
{
	mf_stat_struct mf_buf;
	ue_memory_stats_struct ue_mem_stat_buf;
        about_win_struct *aw;


        aw = &about_win;


        /* Refetch memory stats. */
	MemoryFree(&mf_buf);
	UEGetMemoryStats(&ue_mem_stat_buf);

	static_aboutwin::mem_used = ue_mem_stat_buf.total;
	static_aboutwin::mem_universe = ue_mem_stat_buf.universe;
	static_aboutwin::mem_total = mf_buf.total;


	/* Map. */
	aw->map_state = 0;
	AboutWinDraw(AW_DRAW_AMOUNT_COMPLETE);


	return;
}


void AboutWinUnmap()
{
        about_win_struct *aw;


        aw = &about_win;

	aw->map_state = 0;
	OSWUnmapWindow(aw->toplevel);
	PBtnUnmap(&aw->dismiss_btn);


	OSWDestroyPixmap(&aw->toplevel_buf);


	return;
}

void AboutWinDestroy()
{
        about_win_struct *aw;


        aw = &about_win;


        PBtnDestroy(&aw->dismiss_btn);

        OSWDestroyWindow(&aw->toplevel);
        OSWDestroyPixmap(&aw->toplevel_buf);

	OSWUnloadFont(&aw->font);

	aw->map_state = 0;
	aw->is_in_focus = 0;
	aw->x = 0;
	aw->y = 0;
	aw->width = 0;
	aw->height = 0;


	return;
}



