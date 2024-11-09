/*
	Widget set demostration code, use the Makefile to
	compile this program. Run `make' and then './demo'.

	Copyright (C) 1999 Wolfpack

	May be freely used, modified and distributed, no credit
	requested.
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>


#include "../include/osw-x.h"
#include "../include/widget.h"


/*
 *	Maximum characters per path:
 */
#ifndef PATH_MAX
# define PATH_MAX	1024
#endif


/*
 *	Default demo window toplevel size.
 */
#define DEMO_WIN_WIDTH		640
#define DEMO_WIN_HEIGHT		480

/*
 *	Title strings.
 */
#define DEMO_TITLE		"Widget Demo"
#define DEMO_ICON_TITLE		"Widget Demo"

/*
 *	Popup list widget item names.
 */
#define PULIST_TOTAL_ITEMS	10
char *PULIST_ITEM[] = {
	"Coyote",
	"Fox",
	"Lion",
	"Lynx",
	"Mink",
	"Next door neighbor",
	"Skunk",
	"Tiger",
	"Vixen",
	"Wolf"
};


/*
 *	Toggle button array names.
 */
#define TBA_TOTAL_NAMES 4
char *TBA_NAMES[] = {
	"Gangis Kahn",
	"Hiter",
	"Ho Chi Min",
	"Pat Robertson"
};


/*
 *	List window names.
 */
#define LIST_TOTAL_ITEMS 12
char *LIST_ITEM[] = {
	"Jeremy Dinsel",
	"Tom Vogt",
	"Vidal Bruno",
	"Pascal",
	"Dan Stimits",
	"Shadow Mint",
	"Stefan Maron",
	"Steven A. Hessler",
	"Adrian Ratnapala",
	"Edgar Toernig",
	"Keith \"Cryalith\" Miller",
	"Ben Morse"
};


/* Global runlevel. */
int runlevel;


/*
 *	Main demo window structure.
 */
typedef struct {

	bool_t map_state;	/* True if mapped. */
	bool_t focused;		/* True if in focus. */

	win_t toplevel;
	pixmap_t toplevel_buf;

	/* Demo widgets that we're going to use. */
	menu_bar_struct			mb;
	push_button_struct		pbtn;
	popup_list_struct		pulist;
	prompt_window_struct		prompt;
	toggle_button_struct		toggle_btn;
	toggle_button_array_struct	toggle_btn_array;
	scale_bar_struct		scale_bar;
	list_window_struct		list;
	colum_list_struct		colum_list;

	push_button_struct		map_dialog_pbtn,
					map_fb_pbtn;


} demo_win_struct;
demo_win_struct demo_win;


/*
 *	External widgets not parented to the demo_win:
 */
dialog_win_struct dialog;
fbrowser_struct file_browser;



/*
 *	Function prototypes.
 */
int DemoMenuCB(void *ptr, int code);
int DemoPushButtonCB(void *ptr);	/* Push button callback handler. */

int DemoMapDialogPBCB(void *ptr);
int DemoMapFileBrowserPBCB(void *ptr);

int DemoFileBrowserOKCB(char *path);
int DemoFileBrowserCancelCB(char *path);

void DemoUnfocusToplevels();

int DemoInit(int argc, char *argv[]);
void DemoDraw();
void DemoManageEvents();
void DemoMap();
void DemoUnmap();
void DemoDestroy();




/*
 *	Menu callback handler function.
 */
int DemoMenuCB(void *ptr, int code)
{
	switch(code)
	{
	  case 1:		/* Code 1 means exit. */
	    runlevel = 1;
	    break;

	  default:		/* Ignore everything else. */
	    break;
	}

	return(0);
}


/*
 *	Push button callback handler function.
 */
int DemoPushButtonCB(void *ptr)
{

	return(0);
}


/*
 *	Map dialog push button callback handler function.
 */
int DemoMapDialogPBCB(void *ptr)
{
        DemoUnfocusToplevels();

	/* Map dialog and change its message. */
        printdw(
		&dialog,
"This is the dialog widget, it displays a message.\n\
You can set a message to have multiple lines too!"
	);
	

	return(0);
}

/*
 *	Map file browser push button callback handler function.
 */
int DemoMapFileBrowserPBCB(void *ptr)
{
	DemoUnfocusToplevels();

	/* Map file browser. */
	FBrowserMap(&file_browser);

	return(0);
}


/*
 *	File browser OK callback.
 */
int DemoFileBrowserOKCB(char *path)
{
	char *buf;
	int path_len;


	if(path == NULL)
	    return(0);

	path_len = strlen(path);


        DemoUnfocusToplevels();

	/* Allocate message buffer. */
	buf = (char *)calloc(1, (path_len + 256) * sizeof(char));
	if(buf == NULL)
	    return(0);

	/* Format message for dialog. */
	sprintf(buf,
		"You've selected:\n\n    %s",
		path
	);
	/* Map dialog, change its message to that of buf. */
	printdw(&dialog, buf);


	/* Free message buffer. */
	free(buf);


        return(0);
}

/*
 *	File browser cancel callback.
 */
int DemoFileBrowserCancelCB(char *path)
{
        /* Map dialog and change its message. */
        printdw(
	    &dialog,
"You choosed cancel on the file browser, no\n\
selection has been made.\n"
	);


	return(0);
}



/*
 *	Unfocuses all of demo window's toplevel windows.
 */
void DemoUnfocusToplevels()
{
	demo_win.focused = False;

	return;
}



/*
 *	Connects to the GUI and initializes all the widgets used in
 *	this program.
 */
int DemoInit(int argc, char *argv[])
{
	int i, status;
	char cwd[PATH_MAX];



        /* Connect to GUI. */
	status = OSWGUIConnect(argc, argv);
        if(status)
            return(-1);

        /* Initialize widget globals. */
	status = WidgetInitGlobals(argc, argv);
	if(status)
            return(-1);


	/* *********************************************************** */
	/* Begin allocating client resources and widgets. */

	/* Create the toplevel window. */
	status = OSWCreateWindow(
		&demo_win.toplevel,
		osw_gui[0].root_win,
		osw_gui[0].def_toplevel_x,
		osw_gui[0].def_toplevel_y,
		DEMO_WIN_WIDTH,
		DEMO_WIN_HEIGHT
	);
	if(status)
	    return(-1);

        OSWSetWindowWMProperties(
		demo_win.toplevel,
		DEMO_TITLE,			/* Title. */
		DEMO_ICON_TITLE,		/* Iconified title. */
		widget_global.std_icon_pm,	/* Icon. */
		!osw_gui[0].def_geometry_set,/* Let WM set coordinates? */  
		/* Coordinates. */
                osw_gui[0].def_toplevel_x,
                osw_gui[0].def_toplevel_y,
		/* Min width and height. */
		DEMO_WIN_WIDTH, DEMO_WIN_HEIGHT,
		/* Max width and height. */
		DEMO_WIN_WIDTH, DEMO_WIN_HEIGHT,
		WindowFrameStyleFixed,
		NULL, 0
	);
	/* Use widget set's standard background tile for toplevel. */
        OSWSetWindowBkg(
		demo_win.toplevel,
		0,
		widget_global.std_bkg_pm
	);
        OSWSetWindowInput(
		demo_win.toplevel,
		OSW_EVENTMASK_TOPLEVEL
	);


	/*   Now we start initializing the widgets.  We parent them
	 *   to toplevel.
	 */

	/* Menu bar. */
	status = MenuBarInit(
		&demo_win.mb,
		demo_win.toplevel,
		0, 0, 0, 0,		/* Standard position. */
		DemoMenuCB,
		(void *)&demo_win.mb
	);
        if(status)
            return(-1);   
	status = MenuBarAddItem(
		&demo_win.mb,
		-1,
		"Demo",
		0, 0,
		0, 0
	);
        status = MenuBarAddItem(
                &demo_win.mb,
                -1,
                "Edit",
                demo_win.mb.item[0]->x +
			(int)demo_win.mb.item[0]->width,
		0,
		0, 0
        );
	/* On demo menu. */
	MenuBarAddItemMenuItem(
		&demo_win.mb,
		0,
		"New",
		MENU_ITEM_TYPE_ENTRY,
		NULL,
		0,
		-1
	);
        MenuBarAddItemMenuItem(
                &demo_win.mb,
                0,
                NULL,
                MENU_ITEM_TYPE_HR,
                NULL,
                0,
                -1
        );
        MenuBarAddItemMenuItem(
                &demo_win.mb,  
                0,
                "Open",
                MENU_ITEM_TYPE_ENTRY,
                NULL,
                0,   
                -1
        );
        MenuBarAddItemMenuItem(
                &demo_win.mb,  
                0,
                NULL, 
                MENU_ITEM_TYPE_HR,   
                NULL,
                0,   
                -1
        );
        MenuBarAddItemMenuItem(
                &demo_win.mb,
                0,
                "Exit",
                MENU_ITEM_TYPE_ENTRY,
                NULL,
                1,		/* Code 1 means exit. */
                -1
        );
	/* Edit menu. */
        MenuBarAddItemMenuItem(
                &demo_win.mb,
                1,
                "Undo",
                MENU_ITEM_TYPE_ENTRY,
                NULL,
                0,
                -1
        );
        MenuBarAddItemMenuItem(
                &demo_win.mb,
                1,
                "Copy",
                MENU_ITEM_TYPE_ENTRY,
                NULL,
                0,              
                -1
        );
        MenuBarAddItemMenuItem(
                &demo_win.mb,
                1,     
                "Paste",
                MENU_ITEM_TYPE_ENTRY,
                NULL,
                0,              
                -1
        );
          



	/* Push button. */
	status = PBtnInit(
		&demo_win.pbtn,
		demo_win.toplevel,	/* Parent. */
		20, 50,			/* Coordinates. */
		100, 28,		/* Width, height. */
		"Push Button",		/* Label. */
		PBTN_TALIGN_CENTER,	/* Label alignment. */
		NULL,			/* Label image (NULL for none). */
		&demo_win.pbtn,
		DemoPushButtonCB	/* Callback function. */
	);
	if(status)
	    return(-1);
	/* Hot keys for push button. */
	PBtnSetHotKeys(
		&demo_win.pbtn,
		"\n"			/* Enter key. */
	);
	/* Set hint message for push button. */
	PBtnSetHintMessage(
                &demo_win.pbtn,
                "This is a push button, push it!"
	);


	/* Popup list. */
	status = PUListInit(
		&demo_win.pulist,
		demo_win.toplevel,	/* Parent. */
		140, 50,		/* Coordinates. */
		200, 30,		/* Width, height. */
		5,			/* Items visible when poped up. */
		PULIST_POPUP_DOWN,
		NULL,
		NULL
	);
	if(status)
	    return(-1);
	/* Lets add some items. */
	for(i = 0; i < PULIST_TOTAL_ITEMS; i++)
	{
		PUListAddItem(
			&demo_win.pulist,
			PULIST_ITEM[i],
			False			/* Disabled? */
		);
	}


	/* Text prompt. */
	status = PromptInit(
		&demo_win.prompt,
		demo_win.toplevel,	/* Parent. */
		20, 100,		/* Coordinates. */
		320, 28,		/* Width, height. */
		PROMPT_STYLE_FLUSHED,	/* Style. */
		"Text Prompt:",		/* Label. */
		256,			/* Buffer length. */
		3,			/* Number of history buffers. */
		NULL			/* Callback function. */
	);
	if(status)
	    return(-1);
	/* Put some text in. */
	strncpy(
		demo_win.prompt.buf,
		"(Text goes here)",
		demo_win.prompt.buf_len
	);
	/* Mark it. */
	PromptMarkBuffer(
		&demo_win.prompt,
		PROMPT_POS_END
	);


	/* Toggle button. */
	status = TgBtnInit(
		&demo_win.toggle_btn,
		demo_win.toplevel,
		360, 100,
		False,
		"Toggle Button"
	);
	if(status)
	    return(-1);


	/* Toggle button array. */
	status = TgBtnArrayInit(
		&demo_win.toggle_btn_array,
		demo_win.toplevel,	/* Parent. */
		20, 140,		/* Coordinates. */
		4,			/* Number of toggle buttons. */
		0,			/* Startup selected button. */
		TBA_NAMES,		/* Toggle button names. */
		TBA_TOTAL_NAMES,	/* Total button names. */
		TGBTN_ARRAY_ALIGN_HORIZONTAL	/* Alignment. */
	);
        if(status)
            return(-1);


	/* Scale bar. */
	status = ScaleBarInit(
		&demo_win.scale_bar,
		demo_win.toplevel,	/* Parent. */
		20, 190,		/* Coordinates. */
		200,			/* Length. */
		SCALEBAR_STYLE_STANDARD,	/* Style. */
		5,			/* Ticks. */
		SCALEBAR_ORIENT_HORIZONTAL,	/* Orientation. */
		0, 100,			/* Min pos, max pos. */
		20,			/* Start position. */
		False,			/* Flip position values? */
		NULL,
		NULL
	);
        if(status)
            return(-1);


	/* List window. */
	status = ListWinInit(
		&demo_win.list,
		demo_win.toplevel,	/* Parent. */
		20, 230,		/* Coordinates. */
		320, 140		/* Width, height. */
	);
        if(status)
            return(-1);
	/* Add items to the list. */
	for(i = 0; i < LIST_TOTAL_ITEMS; i++)
	{
		ListAddItem(
			&demo_win.list,
			LIST_ENTRY_TYPE_NORMAL,	/* Type. */
			LIST_ITEM[i],		/* Name. */
			NULL,			/* Image. */
			-1,			/* Position (append) */
			NULL			/* Data pointer. */
		);
        }

	/* Colum list. */
	status = CListInit(
		&demo_win.colum_list,
		demo_win.toplevel,
		350, 230,
		200, 100,
		&demo_win.colum_list,
		NULL
	);
        if(status)
            return(-1);
        /* Add headings to the colum list. */
	CListAddHeading(
		&demo_win.colum_list,
		"Name",
		OSWQueryCurrentFont(),
		widget_global.normal_text_pix,
		0,
		0
	);
        CListAddHeading(
                &demo_win.colum_list,
                "Cat",
                OSWQueryCurrentFont(),
                widget_global.normal_text_pix,
		0,
                70
        );
        CListAddHeading(
                &demo_win.colum_list,
                "Type",
                OSWQueryCurrentFont(),
                widget_global.normal_text_pix,
                0,
                130
        );

	/* Colum list row 0. */
	CListAddRow(
		&demo_win.colum_list,
		-1
	);
	CListAddItem(
		&demo_win.colum_list,
                "Wolf",
		OSWQueryCurrentFont(),
                widget_global.editable_text_pix,
		0,
		0
	);

        /* Colum list row 1. */
        CListAddRow(
                &demo_win.colum_list,
                -1
        );
        CListAddItem(
                &demo_win.colum_list,
                "Dolphin",
                OSWQueryCurrentFont(),
                widget_global.editable_text_pix,
                0,
                1
        );
        CListAddItem(
                &demo_win.colum_list,
                "Mammal",
                OSWQueryCurrentFont(),
                widget_global.editable_text_pix,
                0,
                1
        );
        CListAddItem(
                &demo_win.colum_list,
                "Animal",
                OSWQueryCurrentFont(),
                widget_global.editable_text_pix,
                0,
                1
        );

        /* Colum list row 2. */
        CListAddRow(
                &demo_win.colum_list,
                -1
        );
        CListAddItem(
                &demo_win.colum_list,
                "Mink",
                OSWQueryCurrentFont(),
                widget_global.editable_text_pix,
                0,
                2
        );

        /* Colum list row 3. */
        CListAddRow(
                &demo_win.colum_list,
                -1
        );
        CListAddItem(
                &demo_win.colum_list,
                "Lynx",
                OSWQueryCurrentFont(),
                widget_global.editable_text_pix,
                0,
                3
        );

        /* Colum list row 4. */
        CListAddRow(
                &demo_win.colum_list,
                -1
        );
        CListAddItem(
                &demo_win.colum_list,
                "Cheetha",
                OSWQueryCurrentFont(),
                widget_global.editable_text_pix,
                0,
                4
        );

        /* Colum list row 5. */
        CListAddRow(
                &demo_win.colum_list,
                -1
        );
        CListAddItem(
                &demo_win.colum_list,
                "Ferret",
                OSWQueryCurrentFont(),
                widget_global.editable_text_pix,
                0,
                5
        );

        /* Colum list row 6. */
        CListAddRow(
                &demo_win.colum_list,
                -1
        );        
        CListAddItem(
                &demo_win.colum_list,
                "Zebra",
                OSWQueryCurrentFont(),
                widget_global.editable_text_pix,
                0,   
                6
        );

        /* Colum list row 7. */
        CListAddRow(
                &demo_win.colum_list,
                -1
        );        
        CListAddItem(
                &demo_win.colum_list,
                "Lion",
                OSWQueryCurrentFont(),
                widget_global.editable_text_pix,
                0,
                7
        );





        /* Map Dialog push button. */
        status = PBtnInit(
                &demo_win.map_dialog_pbtn,
                demo_win.toplevel,      /* Parent. */
                20, 390,                /* Coordinates. */  
                130, 28,                /* Width, height. */
                "Map Dialog",		/* Label. */
                PBTN_TALIGN_CENTER,     /* Label alignment. */
                NULL,                   /* Label image (NULL for none). */
		&demo_win.map_dialog_pbtn,
                DemoMapDialogPBCB	/* Callback function. */
        );
        if(status)
            return(-1);
        /* Set hint message for push button. */
        PBtnSetHintMessage(
                &demo_win.map_dialog_pbtn,
                "Maps the dialog, try it!"
        );


        /* Map File Browser push button. */
        status = PBtnInit( 
                &demo_win.map_fb_pbtn,
                demo_win.toplevel,      /* Parent. */
                170, 390,		/* Coordinates. */
                130, 28,		/* Width, height. */
                "Map File Browser",	/* Label. */
                PBTN_TALIGN_CENTER,     /* Label alignment. */
                NULL,                   /* Label image (NULL for none). */
		&demo_win.map_fb_pbtn,
		DemoMapFileBrowserPBCB	/* Callback function. */
        );
        if(status)
            return(-1);
        /* Set hint message for push button. */
        PBtnSetHintMessage(
                &demo_win.map_fb_pbtn,
                "Maps the file browser, check it out!"
        );



	/* Dialog. */
	status = DialogWinInit(
		&dialog,
		osw_gui[0].root_win,
		0, 0,			/* Default width and height. */
		NULL			/* Image. */
	);
        if(status)
            return(-1);


	/* File browser. */
        getcwd(cwd, PATH_MAX);   /* Get current working directory. */
        cwd[PATH_MAX - 1] = '\0';

	status = FBrowserInit(
		&file_browser,
		0, 0,
		440, 260,
		cwd,			/* Initial directory. */
		FB_STYLE_SINGLE_LIST,
		DemoFileBrowserOKCB,	/* OK callback function. */
		DemoFileBrowserCancelCB	/* Cancel callback function. */
	);
        if(status)
            return(-1);
	/*   Remove write protect so we can demostrate more of its features
         *   such as changing of names.
	 */
	file_browser.options &= ~(FB_FLAG_WRITE_PROTECT);


	return(0);
}


/*
 *	Redraws the demo window (but not the widgets).
 */
void DemoDraw()
{




}


/*
 *	Manages the events from the GUI.
 */
void DemoManageEvents()
{
        static event_t event;
        static int events_handled;	/* Events handled per loop. */


	/*   While there are events on the GUI queue, keep
	 *   handling them.
	 */
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


	    /* Here we manage the events for the client (that's us). */
            switch(event.type)
	    {
	      case Expose:
		if(event.xany.window == demo_win.toplevel)
		{
		    DemoDraw();
		    events_handled++;
		}
		break;

	      case FocusIn:
		if(event.xany.window == demo_win.toplevel)
                {
                    demo_win.focused = True;
                    events_handled++;
                }
		break;

              case FocusOut:
                if(event.xany.window == demo_win.toplevel)
                {
                    demo_win.focused = False;
                    events_handled++;
                }
                break;
	    }

	    /* Was the event from the GUI to delete toplevel? */
	    if(OSWIsEventDestroyWindow(demo_win.toplevel, &event))
	    {
		/*   Switch runlevel to 1 so that this program can begin
		 *   exiting.
		 */
		runlevel = 1;
	    }


	    /* Let our allocated widgets see this event too. */
	    events_handled += MenuBarManage(
                &demo_win.mb,
                &event
            );
	    events_handled += PBtnManage(
		&demo_win.pbtn,
		&event
	    );
            events_handled += PUListManage(
                &demo_win.pulist,
                &event
            );
            events_handled += PromptManage(
                &demo_win.prompt,
                &event
            );
            events_handled += TgBtnManage(
                &demo_win.toggle_btn,
                &event
            );
	    events_handled += TgBtnArrayManage(
                &demo_win.toggle_btn_array,
                &event
	    );
	    events_handled += ScaleBarManage(
		&demo_win.scale_bar,
		&event
	    );
            events_handled += ListWinManage(
		&demo_win.list,
		&event
	    );
            events_handled += CListManage(
                &demo_win.colum_list,
                &event
            );
            events_handled += PBtnManage(
                &demo_win.map_dialog_pbtn,
                &event
            );
	    events_handled += PBtnManage(
		&demo_win.map_fb_pbtn,
		&event
	    );

	    /* External widgets. */
	    events_handled += DialogWinManage(
		&dialog,
		&event
	    );
	    events_handled += FBrowserManage(
                &file_browser,
                &event
            );

        }


	return;
}


/*
 *	Maps the demo window and all its widgets.
 */
void DemoMap()
{
	win_attr_t wattr;


	/* Mark demo_win as mapped. */
	demo_win.map_state = True;


	/* Map the toplevel window. */
	OSWMapRaised(demo_win.toplevel);

	/* Need to create toplevel buffer. */
	if(demo_win.toplevel_buf == 0)
	{
	    OSWGetWindowAttributes(demo_win.toplevel, &wattr);
	    OSWCreatePixmap(
		&demo_win.toplevel_buf,
		wattr.width, wattr.height
	    );
	}


	/* Map the widgets. */
	MenuBarMap(&demo_win.mb);
	PBtnMap(&demo_win.pbtn);
	PUListMap(&demo_win.pulist);
	PromptMap(&demo_win.prompt);
	TgBtnMap(&demo_win.toggle_btn);
	TgBtnArrayMap(&demo_win.toggle_btn_array);
	ScaleBarMap(&demo_win.scale_bar);
	ListWinMap(&demo_win.list);
	CListMap(&demo_win.colum_list);
	PBtnMap(&demo_win.map_dialog_pbtn);
	PBtnMap(&demo_win.map_fb_pbtn);

	/* Do not map file browser. */
	/* Do not map dialog. */


	return;
}


/*
 *	Unmaps the demo window and all its widgets.
 */
void DemoUnmap()
{
	/* Mark as unmapped. */
	demo_win.map_state = False;


	/* Unmap the toplevel window. */
	OSWUnmapWindow(demo_win.toplevel);

	/* Destroy toplevel buffer. */
	OSWDestroyPixmap(&demo_win.toplevel_buf);


        /* Unmap the widgets. */
	MenuBarUnmap(&demo_win.mb);
        PBtnUnmap(&demo_win.pbtn);
        PUListUnmap(&demo_win.pulist);
        PromptUnmap(&demo_win.prompt);
        TgBtnUnmap(&demo_win.toggle_btn);
	TgBtnArrayUnmap(&demo_win.toggle_btn_array);
	ScaleBarUnmap(&demo_win.scale_bar);
	ListWinUnmap(&demo_win.list);
	CListUnmap(&demo_win.colum_list);
        PBtnUnmap(&demo_win.map_dialog_pbtn);
	PBtnUnmap(&demo_win.map_fb_pbtn);

	/* Do not unmap file browser. */
	/* Do not unmap dialog. */



	return;
}


/*
 *	Deallocate all dynamic memory and resources for
 *	this program and the widget set.
 */
void DemoDestroy()
{


	/*   Free client allocated widgets.  Remember that the order
	 *   you destroy them must be in reverse from when you initalized
	 *   them.
	 */
	FBrowserDestroy(&file_browser);
	DialogWinDestroy(&dialog);

	PBtnDestroy(&demo_win.map_fb_pbtn);
	PBtnDestroy(&demo_win.map_dialog_pbtn);
	CListDestroy(&demo_win.colum_list);
	ListWinDestroy(&demo_win.list);
	ScaleBarDestroy(&demo_win.scale_bar);
        TgBtnArrayDestroy(&demo_win.toggle_btn_array);
        TgBtnDestroy(&demo_win.toggle_btn);
	PromptDestroy(&demo_win.prompt);
        PUListDestroy(&demo_win.pulist);
        PBtnDestroy(&demo_win.pbtn);
	MenuBarDestroy(&demo_win.mb);

        OSWDestroyPixmap(&demo_win.toplevel_buf);
	OSWDestroyWindow(&demo_win.toplevel);




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
	int status;


	/* Reset global variables. */
	runlevel = 1;


	/* Connect to the GUI and initialize all the widgets. */
	status = DemoInit(argc, argv);
	if(status)
	{
	    /*   Something failed during the initialization, we should
	     *   call DemoDestroy() to deallocate any memory just incase.
	     */
	    DemoDestroy();

	    return(1);
	}


	/* Map the demo window and all its widgets. */
	DemoMap();


	/* Set runlevel to 2. */
	runlevel = 2;

	/* This is the main while() loop. */
	while(runlevel >= 2)
	{
	    usleep(8000);

	    /*   Must call WidgetManage() once per loop to let
	     *   the widget set do what it needs to do.
	     */
	    WidgetManage(NULL);

	    /* Manage events for demo program and the widgets. */
	    DemoManageEvents();
	}


	/*   Deallocate all memory used by this program and the
	 *   widget set, then disconenct from the GUI.
	 */
	DemoDestroy();


	return(0);
}

