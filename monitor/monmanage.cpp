/*
                      Monitor: GUI Management

	Functions:

	int MonInit(monitor_struct *m)
	void MonDraw(monitor_struct *m, int amount)
	int MonManage(monitor_struct *m, event_t *event)
	int MonManageAll(event_t *event)
	void MonMap(monitor_struct *m)
	void MonUnmap(monitor_struct *m)
	void MonDestroy(monitor_struct *m)

	---

 */

#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

#include "../include/string.h"
#include "../include/strexp.h"

#include "../include/disk.h"

#include "../include/osw-x.h"
#include "../include/widget.h"

#include "../include/objects.h"

#include "mon.h"
#include "mesgwin.h"
#include "config.h"


#define MON_DEF_WIDTH	200
#define MON_DEF_HEIGHT	60

#define MON_BTN_WIDTH	80
#define MON_BTN_HEIGHT	12


int MonBtnInit(
	mon_push_button_struct *btn,
	win_t parent,
	int x, int y,
	unsigned int width, unsigned int height,
	char *label,
	void *client_data,
	int (*func_cb)(void *)
)
{
        if(btn == NULL)
            return(-1);

	btn->x = x;
	btn->y = y;
	btn->width = width;
	btn->height = height;
	btn->parent = parent;
	btn->label = StringCopyAlloc(label);
	btn->client_data = client_data;
	btn->func_cb = func_cb;

        btn->state = MON_BTN_STATE_UNARMED;


	return(0);
}


void MonBtnDraw(mon_push_button_struct *btn)
{
	image_t *img_ptr;
	font_t *prev_font;


	if(btn == NULL)
	    return;


	if((btn->width == 0) ||
           (btn->height == 0)
	)
	    return;

	prev_font = OSWQueryCurrentFont();


	switch(btn->state)
	{
	  case MON_BTN_STATE_HIGHLIGHTED:
	    img_ptr = mon_image.btn_highlight;
	    break;

	  case MON_BTN_STATE_ARMED:
            img_ptr = mon_image.btn_armed;
            break;

          default:
            img_ptr = mon_image.btn_unarmed;
            break;
	}

	if(widget_global.force_mono)
	{
	    OSWSetFgPix(osw_gui[0].white_pix);
	    OSWDrawRectangle(
		btn->parent,
		btn->x, btn->y,
		btn->width, btn->height
	    );
	}
	else
	{
	    WidgetPutImageNormal(
	        btn->parent,
	        img_ptr,
	        btn->x, btn->y,
	        True
	    );

	    OSWSetFgPix(mon_color.button_text);
	}

	OSWSetFont(mon_font.button);

	if(btn->label != NULL)
	    OSWDrawString(
	        btn->parent,
	        btn->x + ((int)btn->width / 2) -
                ((strlen(btn->label) * 6) / 2),
	        btn->y + ((int)btn->height / 2) + 4,
	        btn->label
	    );


        OSWSetFont(prev_font);   


	return;
}

int MonBtnManage(mon_push_button_struct *btn, event_t *event)
{
	int x, y;
	int events_handled = 0;


	if((btn == NULL) ||
           (event == NULL)
	)
	    return(events_handled);

	if(event->xany.window != btn->parent)
	    return(events_handled);

	/* We now know that the event occured on the button's parent. */
	switch(event->type)
	{
	  case ButtonPress:
	    x = event->xbutton.x;
            y = event->xbutton.y;

            if((x >= btn->x) &&
               (y >= btn->y) &&
               (x < (btn->x + (int)btn->width)) &&
               (y < (btn->y + (int)btn->height))
            )
	    {
		btn->state = MON_BTN_STATE_ARMED;
                events_handled++;
	    }
	    break;

          case ButtonRelease:
            x = event->xbutton.x;
            y = event->xbutton.y;

            if((x >= btn->x) &&
               (y >= btn->y) &&
               (x < (btn->x + (int)btn->width)) &&
               (y < (btn->y + (int)btn->height))
            )
            {
		if(btn->state == MON_BTN_STATE_ARMED)
		{
		    if(btn->func_cb != NULL)
			btn->func_cb(btn->client_data);
		}

                btn->state = MON_BTN_STATE_HIGHLIGHTED;
                events_handled++;
            }
            break;

          case MotionNotify:
	    x = event->xmotion.x;
            y = event->xmotion.y;
            if((x >= btn->x) &&
               (y >= btn->y) &&
               (x < (btn->x + (int)btn->width)) &&
               (y < (btn->y + (int)btn->height))
	    )
	    {
		if(btn->state == MON_BTN_STATE_UNARMED)
		{
                    btn->state = MON_BTN_STATE_HIGHLIGHTED;
                    events_handled++;
		}
	    }
	    else if(btn->state != MON_BTN_STATE_UNARMED)
	    {
		btn->state = MON_BTN_STATE_UNARMED;
                events_handled++;
	    }
            break;

	  case LeaveNotify:
	    btn->state = MON_BTN_STATE_UNARMED;
	    events_handled++;
	    break;
	}


	return(events_handled);
}


void MonBtnDestroy(mon_push_button_struct *btn)
{
	if(btn == NULL)
	    return;

        btn->x = 0;
        btn->y = 0;
        btn->width = 0;
        btn->height = 0;
	btn->parent = 0;

        free(btn->label);
	btn->label = NULL;

        btn->client_data = NULL;
        btn->func_cb = NULL;

	btn->state = MON_BTN_STATE_UNARMED;


	return;
}


int MonInit(monitor_struct *m, int argc, char *argv[])
{
	int i, x, y;
	unsigned int width, height;
	image_t *img_ptr;
	char title[256];
	char cwd[PATH_MAX];


	if(m == NULL)
	    return(-1);


	getcwd(cwd, PATH_MAX);
	cwd[PATH_MAX - 1] = '\0';


	strcpy(title, NOT_REACHABLE_TITLE_STRING);


	/* Get position and sizes. */
        if(osw_gui[0].def_geometry_set)
        {
            m->x = osw_gui[0].def_toplevel_x;
            m->y = osw_gui[0].def_toplevel_y;
            m->width = osw_gui[0].def_toplevel_width;
            m->height = osw_gui[0].def_toplevel_height;
        }
        else
        {
            m->x = 0;
            m->y = 0;
            m->width = MON_DEF_WIDTH;
            m->height = MON_DEF_HEIGHT;
        }

	img_ptr = mon_image.readout_bkg;
	if(img_ptr != NULL)
	{
	    m->width = img_ptr->width;
	    m->height = img_ptr->height;
	}


	strncpy(
	    m->address,
	    DEF_ADDRESS,
	    HOST_NAME_MAX
	);
	m->address[HOST_NAME_MAX - 1] = '\0';

	m->port = DEF_PORT;

        strncpy(
            m->name,
            DEF_LOGIN_NAME,
            XSW_OBJ_NAME_MAX
        );
        m->name[XSW_OBJ_NAME_MAX - 1] = '\0';

        strncpy(
            m->password,
            DEF_LOGIN_PASSWORD,
            XSW_OBJ_PASSWORD_MAX
        );
        m->password[XSW_OBJ_PASSWORD_MAX - 1] = '\0';



	m->socket = -1;

	m->last_error_mesg = NULL;

	m->frame = 0;


	/*   Parse arguments, options:
	 *
	 *	<pipe_name>
	 *
	 *	Any non-dash prefixed will be assumed to be
	 *	the pipe name.
	 */
	for(i = 1; i < argc; i++)
	{
	    if(argv[i] == NULL)
		continue;

	    /* User login name and password. */
	    if(!strcmp(argv[i], "-user") ||
               !strcmp(argv[i], "-u") ||
               !strcmp(argv[i], "-login") ||
               !strcmp(argv[i], "-l")
	    )
	    {
		i++;
		if(i < argc)
		{
		    strncpy(
                        m->name,
                        argv[i],
                        XSW_OBJ_NAME_MAX
                    );
                    m->name[XSW_OBJ_NAME_MAX - 1] = '\0';
		}

		i++;
                if(i < argc)
                {
                    strncpy(
                        m->password,
                        argv[i],
                        XSW_OBJ_PASSWORD_MAX
                    );
                    m->password[XSW_OBJ_PASSWORD_MAX - 1] = '\0';
		}

	    }
            /* All else assume argument is address. */
            else if((argv[i][0] != '-') &&
                    (argv[i][0] != '+') &&
                    (m->socket < 0)
            )
            {
                strncpy(
                    m->address,
                    argv[i],
                    HOST_NAME_MAX
                );
                m->address[HOST_NAME_MAX - 1] = '\0';

		i++;
		if(i < argc)
		    m->port = atoi(argv[i]);

		MonConnect(
		    m,
		    m->address,
		    m->port
		);
	    }
	}


	/* Create toplevel. */
	if(
            OSWCreateWindow(
                &m->toplevel,
                osw_gui[0].root_win,
                m->x, m->y,
                m->width, m->height
            )
        )
            return(-1);

        OSWSetWindowWMProperties(
            m->toplevel,
            title,		/* Title. */
            title,		/* Icon title. */
            ((mon_image.monitor_icon_pm == 0) ?
                widget_global.std_icon_pm : mon_image.monitor_icon_pm),
            !osw_gui[0].def_geometry_set,	/* Let WM set coordinates? */
            m->x, m->y,
            m->width, m->height,
	    m->width, m->height,
            WindowFrameStyleFixed,
            NULL, 0
        );

        OSWSetWindowInput(
            m->toplevel,
            OSW_EVENTMASK_TOPLEVEL | ButtonPressMask | ButtonReleaseMask |
            PointerMotionMask | ExposureMask |
	    EnterWindowMask | LeaveWindowMask
        );
 

	/* Initialize buttons. */
	img_ptr = mon_image.btn_unarmed;
	x = ((img_ptr == NULL) ?
	    ((int)m->width - MON_BTN_WIDTH - 5) :
            ((int)m->width - (int)img_ptr->width - 5)
	);
	y = 5;
	width = ((img_ptr == NULL) ? MON_BTN_WIDTH : img_ptr->width);
	height = ((img_ptr == NULL) ? MON_BTN_HEIGHT : img_ptr->height);
	if(
	    MonBtnInit(
                &m->shutdown_btn,
		m->toplevel,
                x, y,
		width, height,
		"Shutdown",
		m,
		MonShutdownPBCB
	    )
	)
	    return(-1);

        y += (5 + (int)height);
        if(
            MonBtnInit(
                &m->messages_btn,
                m->toplevel,
                x, y,
                width, height,
                "Messages",
                m,
                MonMessagesPBCB
            )
        )
            return(-1);


	/* Menu. */
	if(
	    MenuInit(
                &m->menu,
		osw_gui[0].root_win,
		MonMenuCB,
		m
	    )
        )
	    return(-1);

	MenuAddItem(
            &m->menu,
	    "Next Display",
	    MENU_ITEM_TYPE_ENTRY,
	    NULL,
	    MON_ACTION_DISPLAY_NEXT,
	    -1
	);

        MenuAddItem(
            &m->menu,
            "Previous Display",
            MENU_ITEM_TYPE_ENTRY,
            NULL,
            MON_ACTION_DISPLAY_PREV,
            -1
        );

        MenuAddItem( 
            &m->menu,
            NULL,
            MENU_ITEM_TYPE_HR,
            NULL,
            MON_ACTION_NONE,
            -1
        );

        MenuAddItem(
            &m->menu,
            "Messages...",
            MENU_ITEM_TYPE_ENTRY,
            NULL,
            MON_ACTION_MAP_MESSAGES,
            -1
        );

        MenuAddItem(
            &m->menu,
            NULL,
            MENU_ITEM_TYPE_HR,
            NULL,
            MON_ACTION_NONE,        
            -1
        );

        MenuAddItem(
            &m->menu,
            "Connect...",
            MENU_ITEM_TYPE_ENTRY,
            NULL,
            MON_ACTION_NEW_CONNECTION,
            -1
        );

        MenuAddItem(
            &m->menu,
            "New Monitor...",
            MENU_ITEM_TYPE_ENTRY,
            NULL,
            MON_ACTION_NEW_MONITOR,
            -1
        );


	/* Connect window. */
	if(
	    ConWinInit(
                &m->cw,
                m,
        	MonConnectCB
	    )
	)
	    return(-1);


	/* Messages window. */
	if(
	    MesgWinInit(
		&m->mesgwin, argc, argv
	    )
	)
	{
	    MonDestroy(m);
	    return(-1);
	}


	return(0);
}


void MonDraw(monitor_struct *m, int amount)
{
	char *strptr;
	char time_str1[256];
	char time_str2[256];
	int x, y, yp, len;
	unsigned int width, height;
	win_t w;
	pixmap_t pixmap;
	image_t *img_ptr;
	font_t *prev_font;

	char text[256];
	win_attr_t wattr;


	if(m == NULL)
	    return;


	/* Map as needed. */
	if(!m->map_state)
	{
	    OSWMapRaised(m->toplevel);
	    m->map_state = 1;

	    amount = MON_DRAW_AMOUNT_COMPLETE;
	}


	/* Recreate buffers as needed. */
	if(m->toplevel_buf == 0)
	{
	    OSWGetWindowAttributes(m->toplevel, &wattr);
	    if(
		OSWCreatePixmap(&m->toplevel_buf,
		    wattr.width, wattr.height
	    ))
		return;
	}

	prev_font = OSWQueryCurrentFont();


	/* Redraw toplevel. */
	if(amount == MON_DRAW_AMOUNT_COMPLETE)
	{
	    w = m->toplevel;
	    pixmap = m->toplevel_buf;
	    OSWGetWindowAttributes(w, &wattr);
            img_ptr = mon_image.readout_bkg;

	    if(!widget_global.force_mono &&
               (img_ptr != NULL)
	    )
	    {
		OSWPutImageToDrawable(img_ptr, pixmap);
	    }
	    else
	    {
		OSWClearPixmap(
		    pixmap,
		    wattr.width, wattr.height,
		    osw_gui[0].black_pix
		);

	    }

	    OSWPutBufferToWindow(w, pixmap);
	}

	/* Redraw readout. */
	if((amount == MON_DRAW_AMOUNT_COMPLETE) ||
           (amount == MON_DRAW_AMOUNT_READOUT)
        )
	{
            w = m->toplevel;
            pixmap = m->toplevel_buf;
            OSWGetWindowAttributes(w, &wattr);

            img_ptr = mon_image.readout_bkg;

            x = 5;
            y = 5;
            width = 200;
            height = (int)wattr.height - 10;

	    OSWSetFont(mon_font.readout);

            if(!widget_global.force_mono &&
               (img_ptr != NULL)
            )
            {
		OSWPutImageToDrawableSect(
                    img_ptr, pixmap,
		    x, y, 		/* Target. */
		    x, y,		/* Source. */
		    width, height
		);

                OSWSetFgPix(mon_color.readout_text);
	    }
	    else
	    {
		OSWSetFgPix(osw_gui[0].black_pix);
                OSWDrawSolidRectangle(
                    pixmap,
                    x, y,
                    width, height
                );       
         
                OSWSetFgPix(osw_gui[0].white_pix);
	    }

	    /* Connected or not? */
	    if(m->socket < 0)
	    {
		/* Not connected, draw error message. */

		strptr = m->last_error_mesg;
		if(strptr == NULL)
		    strptr = "Error:\n    Universe is unreachable.";

		yp = y + 9 + (12 * 0);
		while(strptr != NULL)
		{
		    len = strlinelen(strptr);
		    OSWDrawStringLimited(
			pixmap,
                        x + 4,
                        yp,
			strptr,
			len
		    );
		    strptr = strchr(strptr, '\n');
		    if(strptr != NULL)
			strptr += 1;

		    yp += 12;
		}
	    }
	    else
	    {
		/* Draw stats for curent frame. */
		switch(m->frame)
		{
                  case 2:
                    /* Address, port, and PID. */
                    sprintf(text,
                        "Address: %s",
                        m->address
                    );
                    OSWDrawString(pixmap,
                        x + 4,
                        y + 9 + (12 * 0),
                        text
                    );

                    sprintf(text,
                        "Port: %i",
                        m->port
                    );
                    OSWDrawString(pixmap,
                        x + 4,
                        y + 9 + (12 * 1),
                        text
                    );

                    sprintf(text,
                        "PID: %i",
                        m->stats.pid
                    );
                    OSWDrawString(pixmap,
                        x + 4,
                        y + 9 + (12 * 2),
                        text
                    );

		    break;

		  case 1:
                    /* Full memory info and break down. */
                    sprintf(text,
                        "Total Memory Used: %ld bytes",
			m->stats.mem_total
		    );
                    OSWDrawString(pixmap,
                        x + 4,
                        y + 9 + (12 * 0), 
                        text
                    );

                    sprintf(text,
                        "Objects Memory: %ld bytes",
                        m->stats.mem_obj
                    );
                    OSWDrawString(pixmap,
                        x + 4,
                        y + 9 + (12 * 1),
                        text
                    );

                    sprintf(text,
                        "Connections Memory: %ld bytes",
                        m->stats.mem_con
                    );
                    OSWDrawString(pixmap,
                        x + 4,
                        y + 9 + (12 * 2),
                        text
                    );

		    break;

		  default:
                    /* Number of connections. */
                    sprintf(text,
                        "Connections: %i  Guests: %i",
                        m->stats.total_connections,
                        m->stats.guest_connections
                    );
	            OSWDrawString(pixmap,
		        x + 4,
		        y + 9 + (12 * 0),
		        text
	            );

	            /* Objects. */
                    sprintf(text,
		        "Objects: %i  %ld bytes",
		        m->stats.total_objects,
		        m->stats.mem_obj
	            );
                    OSWDrawString(pixmap,
                        x + 4,
                        y + 9 + (12 * 1),
                        text
                    );

                    /* Uptime. */
                    strptr = StringFormatTimePeriod(m->stats.uptime);
                    strncpy(
                        time_str1,
                        ((strptr == NULL) ? "" : strptr),
                        256
                    );
                    time_str1[255] = '\0';
                    strptr = StringFormatTimePeriod(
                        m->stats.next_save
                    );
                    strncpy(
                        time_str2,
                        ((strptr == NULL) ? "" : strptr),
                        256
                    );
                    time_str2[255] = '\0';
                    sprintf(text,
                        "Uptime: %s  Save: %s",
                        time_str1, time_str2
                    );
                    OSWDrawString(pixmap,
                        x + 4,
                        y + 9 + (12 * 2),
                        text
                    );
		    break;
		}
	    }

	    /* Put to window. */
	    OSWCopyDrawablesCoord(
                w, pixmap,
		x, y,
		width, height,
		x, y
	    );
	}


	/* Redraw buttons. */
        if((amount == MON_DRAW_AMOUNT_COMPLETE) ||
           (amount == MON_DRAW_AMOUNT_BUTTONS)
	)
        {
	    MonBtnDraw(&m->shutdown_btn);
            MonBtnDraw(&m->messages_btn);
	}

        OSWSetFont(prev_font);


	return;
}


int MonManage(monitor_struct *m, event_t *event)
{
	int x, y, root_x, root_y;
	int events_handled = 0;


	if((m == NULL) ||
           (event == NULL)
	)
	    return(events_handled);


	if(!m->map_state &&
           (event->type != MapNotify)
	)
            return(events_handled);


	/* Reset schedualed monitor for deletion. */
	delete_monitor = NULL;


	/* Manage buttons first. */
	events_handled += MonBtnManage(&m->shutdown_btn, event);
	events_handled += MonBtnManage(&m->messages_btn, event);

	/* Return of one of the buttons managed the event. */
	if(events_handled > 0)
	{
            MonDraw(m, MON_DRAW_AMOUNT_BUTTONS);
	    return(events_handled);
	}

	/* Manage events for monitor normally. */
	switch(event->type)
	{
	  case ButtonPress:
	    if(event->xany.window == m->toplevel)
	    {
		switch(event->xbutton.button)
		{
                  case Button3:
		    /* Map menu. */
		    OSWGetPointerCoords(
			m->toplevel,
			&root_x, &root_y,
			&x, &y
		    );

		    if(m->menu.map_state)
			MenuUnmap(&m->menu);
		    else
			MenuMapPos(
		            &m->menu,
		            root_x,
		            root_y
		        );
		    break;

		  default:
		    /* Change frame. */
		    m->frame++;
		    if(m->frame >= MON_MAX_FRAMES)
			m->frame = 0;
		    MonDraw(m, MON_DRAW_AMOUNT_READOUT);
		    break;
		}

                events_handled++;
		return(events_handled);
	    }
	    break;

	  case Expose:
	    if(event->xany.window == m->toplevel)
	    {
	        events_handled++;
	    }
	    break;

	  case FocusIn:
            if(event->xany.window == m->toplevel)
            {
                m->is_in_focus = 1;
                events_handled++;
		return(events_handled);
            }
            break;

          case FocusOut:
            if(event->xany.window == m->toplevel)
            {
                m->is_in_focus = 0;
                events_handled++;
                return(events_handled);
            }
            break;

          case MapNotify:
            if(event->xany.window == m->toplevel)
            {
                if(!m->map_state)
		    MonMap(m);

                events_handled++;
                return(events_handled);
            }
            break;

	  case UnmapNotify:
            if(event->xany.window == m->toplevel)
            {
                if(m->map_state)  
                    MonUnmap(m);

                events_handled++;
                return(events_handled);
            }
	    break;

          case ClientMessage:
            if(OSWIsEventDestroyWindow(m->toplevel, event))
            {
                delete_monitor = m;

                events_handled++;
                return(events_handled);
            }
            break;
	}

	if(events_handled > 0)
	{
	    MonDraw(m, MON_DRAW_AMOUNT_COMPLETE);
	}

	if(events_handled == 0)
	    events_handled += MenuManage(&m->menu, event);

        return(events_handled);
}


int MonManageAll(event_t *event)
{
        int i, m_deleted;
        int events_handled = 0;


        if(event == NULL)
            return(events_handled);


        /* Manage each monitor window. */
        for(i = 0, m_deleted = 0; i < total_monitors; i++)
        {
            if(monitor[i] == NULL) 
                continue;

            events_handled += MonManage(monitor[i], event);

	    events_handled += MesgWinManage(&monitor[i]->mesgwin, event);

            events_handled += ConWinManage(&monitor[i]->cw, event);


            /*   Somewhere in above management call require this
             *   monitor to be deleted?
             */
            if(delete_monitor == monitor[i])
            {
                MonDelete(i);
                m_deleted++;
                delete_monitor = NULL;

		continue;
            }

        }



        /* Update window menus as needed. */
/*
        if(m_deleted > 0)
	    ???
 */


        return(events_handled);
}


void MonMap(monitor_struct *m)
{
        if(m == NULL)
            return;


	m->map_state = 0;
	MonDraw(m, MON_DRAW_AMOUNT_COMPLETE);


	return;
}


void MonUnmap(monitor_struct *m)
{
	if(m == NULL)
	    return;


	OSWUnmapWindow(m->toplevel);
	m->map_state = 0;

        OSWDestroyPixmap(&m->toplevel_buf);


	return;
}


void MonDestroy(monitor_struct *m)
{
	if(m == NULL)
	    return;


	*m->address = '\0';
	m->port = 0;
	*m->name = '\0';
	*m->password = '\0';

	if(m->socket > -1)
	    close(m->socket);
	m->socket = -1;

	free(m->last_error_mesg);
	m->last_error_mesg = NULL;

	m->frame = 0;


	MesgWinDestroy(&m->mesgwin);
	ConWinDestroy(&m->cw);

	MenuDestroy(&m->menu);

	MonBtnDestroy(&m->shutdown_btn);
	MonBtnDestroy(&m->messages_btn);

	OSWDestroyWindow(&m->toplevel);
	OSWDestroyPixmap(&m->toplevel_buf);


	return;
}
