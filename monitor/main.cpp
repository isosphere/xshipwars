/*
                      Program Primary Routines

	Functions:

	void MonitorSignalHandler(int s)
	int MonitorScrollBarCB(scroll_bar_struct *sb)

	int MonitorCreateMonitor(int argc, char *argv[])
	void MonitorResetTimmers()
	int MonitorInit(int argc, char *argv[])
	void MonitorManage()
	void MonitorShutdown()

	int main(int argc, char *argv[])

	---

 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/stat.h>

#include "../include/strexp.h"
#include "../include/string.h"
#include "../include/disk.h"
#include "../include/os.h"
#include "../include/osw-x.h"
#include "../include/widget.h"

#include "mon.h"
#include "config.h"


#include "../include/cursors/std_arrow.xpm"
#include "../include/cursors/text_cur.xpm"

#include "../include/icons/warning.h"


int runlevel;
time_t cur_millitime;
time_t cur_systime;

mon_option_struct option;
mon_dname_struct dname;
mon_next_struct next;
mon_interval_struct interval;

mon_color_struct mon_color;
mon_font_struct mon_font;
mon_image_struct mon_image;
mon_cursor_struct mon_cursor;

monitor_struct **monitor;
int total_monitors;

monitor_struct *delete_monitor;

dialog_win_struct dialog;


/*
 *	Signal handler.
 */
void MonitorSignalHandler(int s)
{

	switch(s)
	{
          /* ****************************************************** */
          case SIGINT:
            runlevel = 1;
            break;

          /* ****************************************************** */
          case SIGTERM:
            runlevel = 1;
            break;

          /* ****************************************************** */
          case SIGQUIT:
            runlevel = 1;
            break;

          /* ****************************************************** */
          case SIGABRT:
            runlevel = 1;
            break;
            
          /* ****************************************************** */
          case SIGKILL:
            runlevel = 1;
            break;

          /* ****************************************************** */
          case SIGPIPE:
            signal(SIGPIPE, MonitorSignalHandler);
            break;

          /* ****************************************************** */
          case SIGSEGV:
	    fprintf(stderr, "Segmentation fault.\n");
            runlevel = 1;
            break;

          /* ****************************************************** */
          case SIGCONT:
            signal(SIGCONT, MonitorSignalHandler);
            break;

          /* ****************************************************** */
	  default:
	    break;
	}


	return;
}


/*
 *	Scroll bar callback handler.
 */
int MonitorScrollBarCB(scroll_bar_struct *sb)
{
	int i;
	monitor_struct **mon_ptr;


	for(i = 0, mon_ptr = monitor;
            i < total_monitors;
            i++, mon_ptr++
        )
	{
	    if(*mon_ptr == NULL)
		continue;

	    if(&(*mon_ptr)->mesgwin.sb == sb)
	    {
		MesgWinDraw(&(*mon_ptr)->mesgwin);
	    }
	}



	return(0);
}


/*
 *	Procedure to allocate and initialize a new monitor
 *	window.
 */
int MonitorCreateMonitor(int argc, char *argv[])
{
	int i;

	i = MonAllocate();
	if(MonIsAllocated(i))
	{
	    if(MonInit(monitor[i], argc, argv))
	    {
		MonDelete(i);
	        return(-1);
	    }
	}

	return(i);
}


void MonitorResetTimmers()
{
	next.pipe_recv = 0;
	next.systems_check = 0;
	next.connect_check = 0;


	return;
}


/*
 *	Program primary initializer.
 */
int MonitorInit(int argc, char *argv[])
{
	int i, n, status;
	char *strptr;
	char cwd[PATH_MAX];
	image_t *img_ptr;
	WColorStruct color;



	cur_millitime = MilliTime();
	cur_systime = time(NULL);

	/* Get current working directory. */
        getcwd(cwd, PATH_MAX);
        cwd[PATH_MAX - 1] = '\0';

	/* Global options. */
	option.quiet_mode = 0;

        interval.pipe_recv = PIPE_RECV_INT;
        interval.systems_check = SYSTEMS_CHECK_INT;
	interval.connect_check = CONNECT_CHECK_INT;

	MonitorResetTimmers();

	/* Set signal handler. */
        signal(SIGINT, MonitorSignalHandler);
        signal(SIGTERM, MonitorSignalHandler);
        signal(SIGQUIT, MonitorSignalHandler);
        signal(SIGABRT, MonitorSignalHandler);
        signal(SIGKILL, MonitorSignalHandler);
        signal(SIGPIPE, MonitorSignalHandler);
        signal(SIGSEGV, MonitorSignalHandler);
        signal(SIGCONT, MonitorSignalHandler);


	/* Reset globals. */
        strncpy(
            dname.images,
            DEF_IMAGES_DIR,
            PATH_MAX
        );
        dname.images[PATH_MAX - 1] = '\0';

        strncpy(
            dname.server,
            DEF_SERVER_DIR,
            PATH_MAX
        );
        dname.server[PATH_MAX - 1] = '\0';

	monitor = NULL;
	total_monitors = 0;


	/* ******************************************************** */
	/* Parse arguments. */
	for(i = 1; i < argc; i++)
	{
	    if(argv[i] == NULL)
		continue;


	    /* Help. */
	    if(strcasepfx(argv[i], "--h") ||
               strcasepfx(argv[i], "-h") ||
               strcasepfx(argv[i], "-?")
            )
	    {
		printf(PROG_USAGE_MESSAGE);
		return(-4);
	    }
            /* Version. */
            else if(strcasepfx(argv[i], "--ver") ||
                    strcasepfx(argv[i], "-ver")
            )
            {
                printf(
		    "%s Version %s\n%s\n",
		    PROG_NAME,
		    PROG_VERSION,
		    PROG_COPYRIGHT
		);
                return(-4);
            }
	    /* Quiet mode. */
	    else if(strcasepfx(argv[i], "--q") ||
                    strcasepfx(argv[i], "-q")
	    )
	    {
		option.quiet_mode = 1;
	    }
	    /* Images dir. */
	    else if(strcasepfx(argv[i], "--i") ||
                    strcasepfx(argv[i], "-i")
            )
            {
		i++;
		if(i < argc)
		{
		    strncpy(
			dname.images,
			argv[i],
			PATH_MAX
		    );
		    dname.images[PATH_MAX - 1] = '\0';
		}
		else
		{
		    fprintf(stderr,
			"%s: Requires argument.\n",
			argv[i - 1]
		    );
		}
            }
            /* Server dir. */
            else if(strcasepfx(argv[i], "--s") ||
                    strcasepfx(argv[i], "-s")
            )
            {
                i++;
                if(i < argc)
                {
                    strncpy(
                        dname.server,
                        argv[i],
                        PATH_MAX
                    );
                    dname.server[PATH_MAX - 1] = '\0';
                }
                else
                {
                    fprintf(stderr,
                        "%s: Requires argument.\n",
                        argv[i - 1]
                    );
                }
            }
	}




        /* Connect to GUI. */
        status = OSWGUIConnect(argc, argv);
        if(status)
            return(-1);

        /* Initialize widget globals. */  
        status = WidgetInitGlobals(argc, argv);
        if(status)
            return(-1);

	SBarSetNotifyFunction(MonitorScrollBarCB);



	/* Colors. */
	OSWLoadPixelRGB(
	    &mon_color.readout_text,
            0xff,
            0x70,
	    0xd0
        );
        OSWLoadPixelRGB(
            &mon_color.button_text,
            0xf0,
            0xa0,
            0xff
        );
        OSWLoadPixelRGB(
            &mon_color.messages_text,
            0xff,
            0xc0,
            0xff
        );


	/* Fonts. */
	OSWLoadFont(
	    &mon_font.readout,
            "6x10"
	);
	OSWLoadFont( 
            &mon_font.button,
            "6x10"
        );
        OSWLoadFont(
            &mon_font.messages,
            "7x14"
        );


	/* Cursors. */
	color.a = 0x00;
	color.r = 0xff;
	color.g = 0xff;
	color.b = 0xff;
	mon_cursor.standard = WidgetCreateCursorFromData(
	    std_arrow_xpm,
	    0, 0,
	    color
	);

        color.a = 0x00;
        color.r = 0xff;
        color.g = 0xb3;
        color.b = 0xfa;
        mon_cursor.text = WidgetCreateCursorFromData(
            text_cur_xpm,
            8, 8,
            color
        );



	/* Images. */
	strptr = PrefixPaths(dname.images, IMG_NAME_READOUT_BKG);
	mon_image.readout_bkg = WidgetLoadImageFromTgaFile(strptr);

        strptr = PrefixPaths(dname.images, IMG_NAME_MESG_BKG);
        mon_image.mesg_bkg = WidgetLoadImageFromTgaFile(strptr);

        strptr = PrefixPaths(dname.images, IMG_NAME_BTN0);
        mon_image.btn_unarmed = WidgetLoadImageFromTgaFile(strptr);

        strptr = PrefixPaths(dname.images, IMG_NAME_BTN1);
        mon_image.btn_armed = WidgetLoadImageFromTgaFile(strptr);

        strptr = PrefixPaths(dname.images, IMG_NAME_BTN2);
        mon_image.btn_highlight = WidgetLoadImageFromTgaFile(strptr);


        mon_image.warning = WidgetLoadImageFromTgaData(warning_tga);

	strptr = PrefixPaths(dname.images, IMG_NAME_MONITOR_ICON);
	img_ptr = WidgetLoadImageFromTgaFile(strptr);
	mon_image.monitor_icon_pm = OSWCreatePixmapFromImage(img_ptr);
	OSWDestroyImage(&img_ptr);


	/* Dialog widget. */
	if(DialogWinInit(
            &dialog,
	    osw_gui[0].root_win,
	    0, 0,
	    mon_image.warning
        ))
	    return(-1);


	/* Initialize first monitor window. */
	n = MonitorCreateMonitor(argc, argv);
	if(n < 0)
	    return(-1);
	MonMap(monitor[n]);


	return(0);
}



void MonitorManage()
{
	int i, none;
	int events_handled = 0;
	time_t t;
	event_t event;

	monitor_struct **mon_ptr;


	/* Update timming. */
	t = MilliTime();
	if(t < cur_millitime)
	{
	    MonitorResetTimmers();
	}
	cur_millitime = t;

	cur_systime = time(NULL);


	/* ***************************************************** */
	/* Manage GUI events. */
        while(OSWEventsPending() > 0)
        {   
            /* Reset events counter. */
            events_handled = 0;
                 
            /* Get event. */
            OSWWaitNextEvent(&event);
                 
                
            /*   Let WidgetManage() see this event.
             *   It is not important if the event is handled or not, so
             *   the return value is disgarded.
             */
            WidgetManage(&event);


	    /* Manage all universe edit windows. */
	    events_handled += MonManageAll(&event);

	    /* Dialog widget. */
	    events_handled += DialogWinManage(
                &dialog, &event
            );
	}


        /* ******************************************************* */
	/* Schedualed actions. */

	/* Recieve incoming data on monitor. */
	if(next.pipe_recv <= cur_millitime)
	{
	    for(i = 0, mon_ptr = monitor;
                i < total_monitors;
                i++, mon_ptr++
	    )
	    {
		if(*mon_ptr == NULL)
		    continue;

                MonDoFetch(*mon_ptr);
	    }

	    next.pipe_recv = cur_millitime + interval.pipe_recv;
	}

	/* Systems check. */
        if(next.systems_check <= cur_millitime)
        {


            next.systems_check = cur_millitime + interval.systems_check;
        }

	/* Reconnect check. */
	if(next.connect_check <= cur_millitime)
        {    
            for(i = 0, mon_ptr = monitor;
                i < total_monitors;
                i++, mon_ptr++
            )
	    {
                if(*mon_ptr == NULL)
                    continue;

                if((*mon_ptr)->socket > -1)
		    continue;


		/* Redraw. */
		if((*mon_ptr)->map_state)
		    MonDraw(*mon_ptr, MON_DRAW_AMOUNT_READOUT);

		/* Reconnect. */
		MonConnect(
		    *mon_ptr,
		    (*mon_ptr)->address,
		    (*mon_ptr)->port
		);
	    }

            next.connect_check = cur_millitime + interval.connect_check;
	}

        /* ******************************************************* */
	/* Switch to runlevel 1 if all windows are gone. */
	for(i = 0, none = 0; i < total_monitors; i++)
	{
	    if(monitor[i] != NULL)
		break;
	}
	if(i >= total_monitors)
	    none = 1;


	if(none)
	    runlevel = 1;


	return;
}


void MonitorShutdown()
{
	/* Dialog. */
	DialogWinDestroy(&dialog);

	/* Monitors. */
	MonDeleteAll();


        /* Cursors. */
        WidgetDestroyCursor(&mon_cursor.text);        
        WidgetDestroyCursor(&mon_cursor.standard);


	/* Images. */
        OSWDestroyPixmap(&mon_image.monitor_icon_pm);

        OSWDestroyImage(&mon_image.warning);
        OSWDestroyImage(&mon_image.btn_highlight);
        OSWDestroyImage(&mon_image.btn_armed);
        OSWDestroyImage(&mon_image.btn_unarmed);
        OSWDestroyImage(&mon_image.mesg_bkg);
        OSWDestroyImage(&mon_image.readout_bkg);


	/* Fonts. */
	OSWUnloadFont(&mon_font.messages);
	OSWUnloadFont(&mon_font.button);
        OSWUnloadFont(&mon_font.readout);


	/* Colors. */
	OSWDestroyPixel(&mon_color.readout_text);
        OSWDestroyPixel(&mon_color.button_text);
        OSWDestroyPixel(&mon_color.messages_text);



        /*   Free widget globals after all client allocated widgets
         *   have been deallocated.
         */
        WidgetDestroyGlobals();

        /* Disconnect from the GUI. */
        OSWGUIDisconnect();


	return;
}



int main(int argc, char *argv[])
{
	runlevel = 1;

	switch(MonitorInit(argc, argv))
	{
	  case 0:
	    break;

	  case -4:
            MonitorShutdown();
            return(0);
	    break;

	  default:
	    MonitorShutdown();
	    return(1);
	    break;
	}


	runlevel = 2;

        while(runlevel >= 2)
        {
            usleep(8000);

            /*   Must call WidgetManage() once per loop to let
             *   the widget set do what it needs to do.
             */
            WidgetManage(NULL);

            MonitorManage();
        }


        runlevel = 1;
        MonitorShutdown();


	return(0);
}
