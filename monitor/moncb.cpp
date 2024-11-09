/*
                        Monitor Callback Handlers

	Functions:

	int MonAddressIsLocal(char *address)

	int MonShutdownPBCB(void *ptr)
	int MonMessagesPBCB(void *ptr)
	int MonMenuCB(void *ptr, int op_code)
	int MonConnectCB(
		void *ptr,
		char *address,
		char *port,
		char *name,
		char *password
	)

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
#include <signal.h>

#include "../include/osw-x.h"
#include "../include/widget.h"

#include "../include/objects.h"

#include "mon.h"
#include "mesgwin.h"


int MonAddressIsLocal(char *address);


/*
 *	Checks if address is `local':
 */
int MonAddressIsLocal(char *address)
{
	if(address == NULL)
	    return(0);

	if(!strcasecmp(address, "127.0.0.1"))
	    return(1);
	if(!strcasecmp(address, "localhost"))
            return(1);

	return(0);
}

/*
 *	Shutdown procedure callback.
 *
 *	Checks if monitor is connected to a universe on the
 *	same computer, if so then it sends a SIGTERM to
 *	the server process.
 */
int MonShutdownPBCB(void *ptr)
{
	monitor_struct *m;
	int status;
	char text[HOST_NAME_MAX + 1024];


        if(ptr == NULL)
            return(-1);

	m = (monitor_struct *)ptr;


	/* Check if the universe that we are connected to
	 * is *not* on the same computer as this process.
	 */
	if(!MonAddressIsLocal(m->address))
	{
	    sprintf(
		text,
"Cannot shutdown universe at:\n\
\n\
    %s\n\
\n\
The universe that you are connected to does not appear\n\
to be running on the same physical machine as this process.\n\
You must be running this program on the same computer as\n\
the universe's server to shut it down.",
		m->address
	    );
	    printdw(&dialog, text);

	    return(0);
	}


	/* Send SIGTERM to server process. */
	if(m->stats.pid > 0)
	{
	    status = kill(m->stats.pid, SIGTERM);
	    if(status)
	    {
		switch(errno)
		{
		  case EINVAL:
                    fprintf(   
                        stderr,
                        "%i: Internal error, invalid signal `%i'.\n",
                        m->stats.pid, SIGTERM
                    );
		    break;

                  case ESRCH:
                    fprintf(
                        stderr, 
                        "%i: No such process.\n",
                        m->stats.pid
                    );
                    break;

		  case EPERM:
		    fprintf(
			stderr,
			"%i: Permission denied.\n",
			m->stats.pid
		    );
		    break;
		}
	    }
	}


	/* Schedual this monitor for deletion. This is because
	 * the universe is assumed to be shutdown, so further
	 * attempts to connect would be futile.
	 */
	delete_monitor = m;


	return(0);
}

/*
 *	Message button callback.
 *
 *	Maps messages window.
 */
int MonMessagesPBCB(void *ptr)
{
        monitor_struct *m;


        if(ptr == NULL)
            return(-1);

        m = (monitor_struct *)ptr;

	MesgWinMap(&m->mesgwin);


	return(0);
}


/*
 *	Menu callback.
 */
int MonMenuCB(void *ptr, int op_code)
{
	int i;
	int status = 0;
	monitor_struct *m = NULL;
	win_attr_t wattr;


	for(i = 0; i < total_monitors; i++)
	{
	    if(monitor[i] == NULL)
		continue;

	    if(monitor[i] == (monitor_struct *)ptr)
		m = (monitor_struct *)ptr;
	}


	switch(op_code)
	{
	  case MON_ACTION_NONE:
	    break;

	  case MON_ACTION_DISPLAY_NEXT:
	    if(m == NULL)
                break;

            m->frame++;
	    if(m->frame > MON_MAX_FRAMES)
		m->frame = 0;

	    if(m->map_state)
                MonDraw(m, MON_DRAW_AMOUNT_READOUT);
	    break;

	  case MON_ACTION_DISPLAY_PREV:
            if(m == NULL)
                break;

            m->frame--;
            if(m->frame < 0)
		m->frame = MON_MAX_FRAMES - 1;

            if(m->map_state)
	        MonDraw(m, MON_DRAW_AMOUNT_READOUT);
	    break;

	  case MON_ACTION_MAP_MESSAGES:
	    if(m == NULL)
		break;
	    MesgWinMap(&m->mesgwin);
	    break;

	  case MON_ACTION_NEW_CONNECTION:
            if(m == NULL)
                break;

	    PromptSetS(&m->cw.address_prompt, m->address);
	    PromptSetI(&m->cw.port_prompt, m->port);
            PromptSetS(&m->cw.name_prompt, m->name);
            PromptSetS(&m->cw.password_prompt, m->password);

            WidgetCenterWindow(m->cw.toplevel, WidgetCenterWindowToPointer);
            OSWGetWindowAttributes(m->cw.toplevel, &wattr);
            m->cw.x = wattr.x;
            m->cw.y = wattr.y;
            m->cw.width = wattr.width;
            m->cw.height = wattr.height;

            ConWinMap(&m->cw);
	    break;

	  case MON_ACTION_NEW_MONITOR:
	    i = MonitorCreateMonitor(0, NULL);
	    if(MonIsAllocated(i))
	    {
		m = monitor[i];
		MonMap(m);

                WidgetCenterWindow(m->cw.toplevel, WidgetCenterWindowToPointer);
                OSWGetWindowAttributes(m->cw.toplevel, &wattr);
                m->cw.x = wattr.x;
                m->cw.y = wattr.y;
                m->cw.width = wattr.width;
                m->cw.height = wattr.height;

                ConWinMap(&m->cw);
	    }
	    break;

	  default:
	    break;
	}


	return(status);
}

/*
 *	Connect window callback, will update the monitor's
 *	address, port, name and password to the given inputs.
 */
int MonConnectCB(
	void *ptr,
	char *address,
	char *port,
	char *name,
	char *password
)  
{
        int i, status;
        monitor_struct *m;


	if(ptr == NULL)
	    return(-1);


        m = (monitor_struct *)ptr;

        for(i = 0; i < total_monitors; i++)
        {
            if(monitor[i] == m)
                break;
        }             
        if(i >= total_monitors)
            return(-1);


	strncpy(
	    m->name,
	    ((name == NULL) ? "" : name),
	    XSW_OBJ_NAME_MAX
	);
	m->name[XSW_OBJ_NAME_MAX - 1] = '\0';

        strncpy(
            m->password,
            ((password == NULL) ? "" : password),
            XSW_OBJ_PASSWORD_MAX
        );
        m->password[XSW_OBJ_PASSWORD_MAX - 1] = '\0';


	status = MonConnect(
	    m,
	    address,
	    atoi(port)
	);


	return(status);
}
