/*
              S H I P W A R S   S E R V E R   M O N I T O R


	Copyright (C) 1997-2001 WolfPack Entertainment

 */

#ifndef MONITOR_H
#define MONITOR_H


#include <stdio.h>
#include <sys/types.h>
#include <limits.h>

#include "../include/os.h"

#include "../include/objects.h"

#include "../include/osw-x.h"
#include "../include/widget.h"

#include "conwin.h"
#include "mesgwin.h"


/*
 *	Program name and version:
 */
#define PROG_NAME	"Monitor"
#define PROG_VERSION	"1.34.0"

#define PROG_VERSION_MAJOR	1
#define PROG_VERSION_MINOR	34
#define PROG_VERSION_RELEASE	0


/*
 *	Usage information:
 */
#define PROG_USAGE_MESSAGE "\
Usage: monitor <address> <port> [options] [GUI_options]\n\
\n\
    <address> specifies the address to the swserv you want to\n\
    monitor.\n\
\n\
    <port> specifies the AUX Stats port number swserv is expecting\n\
    monitoring client connections from.\n\
\n\
    [options] can be any of the following:\n\
\n\
	-u <name> <password>   Specifies to connect using a login name\n\
                               and password.\n\
        -q                     Surpress stdout and stderr messages.\n\
        --images <path>        Specifies the images directory.\n\
        --server <path>        Specifies the server directory.\n\
        --help                 Prints (this) help screen and exits.\n\
        --version              Prints version information and exits.\n\
\n\
    [GUI_options] can be any options standard to your GUI, consult\n\
    your GUI's manual for available options.\n\
\n"


/*
 *      Copyright information:
 */
#define PROG_COPYRIGHT	"\
Copyright (C) 1997-2001 WolfPack Entertainment.\n\
This program is protected by international copyright laws and treaties,\n\
distribution and/or modification of this software in violation of the\n\
GNU Public License is strictly prohibited. Violators will be prosicuted\n\
to the fullest extent of the law."


/*
 *	Draw amount codes:
 */
#define MON_DRAW_AMOUNT_COMPLETE	0
#define MON_DRAW_AMOUNT_READOUT		1
#define MON_DRAW_AMOUNT_BUTTONS		2


/*
 *	Monitor action codes:
 */
#define MON_ACTION_NONE			0
#define MON_ACTION_DISPLAY_NEXT		10
#define MON_ACTION_DISPLAY_PREV		11
#define MON_ACTION_MAP_MESSAGES		20
#define MON_ACTION_NEW_CONNECTION	30
#define MON_ACTION_NEW_MONITOR		31


/*
 *	Runlevel:
 */
extern int runlevel;

/*
 *	Current time since midnight (in milliseconds):
 */
extern time_t cur_millitime;

/*
 *	Current systime (in seconds):
 */
extern time_t cur_systime;


/*
 *	Options:
 */
typedef struct {

	char quiet_mode;	/* Do not print errors. */

} mon_option_struct;
extern mon_option_struct option;


/*
 *	Directories:
 */
typedef struct {

	char images[PATH_MAX];
	char server[PATH_MAX];

} mon_dname_struct;
extern mon_dname_struct dname;

/*
 *	Timmers:
 *
 *	In milliseconds.
 */
typedef struct {

	time_t	pipe_recv,
		systems_check,
		connect_check;

} mon_next_struct;
extern mon_next_struct next;


/*
 *	Intervals:
 *
 *	In milliseconds.
 */
typedef struct {

	time_t	pipe_recv,
		systems_check,
		connect_check;

} mon_interval_struct;
extern mon_interval_struct interval;



/*
 *	Monitor colors:
 */
typedef struct {

	pixel_t	readout_text,
		button_text,
		messages_text;

} mon_color_struct;
extern mon_color_struct mon_color;

/*
 *	Monitor fonts:
 */
typedef struct {

	font_t	*readout,
		*button,
		*messages;

} mon_font_struct;
extern mon_font_struct mon_font;

/*
 *	Monitor images:
 */
typedef struct {

	image_t *readout_bkg,
		*mesg_bkg;

	image_t *btn_unarmed,
		*btn_armed,
		*btn_highlight;

	image_t *warning;

	pixmap_t monitor_icon_pm;

} mon_image_struct;
extern mon_image_struct mon_image;

/*
 *	Cursors:
 */
typedef struct {

	WCursor	*standard,
		*text;

} mon_cursor_struct;
extern mon_cursor_struct mon_cursor;


/*
 *	Monitor push button widget:
 */
#define MON_BTN_STATE_UNARMED		0
#define MON_BTN_STATE_ARMED		1
#define MON_BTN_STATE_HIGHLIGHTED	2

typedef struct {

	int x, y;
	unsigned int width, height;
	win_t parent;

	int state;
	char *label;

	void *client_data;
	int (*func_cb)(void *);

} mon_push_button_struct;


/*
 *	Monitor stats:
 */
typedef struct {

	char title[UNV_TITLE_MAX];

	time_t uptime;	/* How long it has been up in systime sec. */

	int	total_connections,
		guest_connections;

	int	total_objects;

	off_t	mem_total,
		mem_con,
                mem_obj;

	time_t	next_save,
		next_export;

	pid_t	pid;		/* PID of server. */

} mon_stats_struct;


/*
 *	Monitor structure:
 */
#define MON_MAX_FRAMES	3
typedef struct {

	char map_state;
	char is_in_focus;
	int x, y;
	unsigned int width, height;
	int visibility_state;

        char address[HOST_NAME_MAX];
	int port;
	char name[XSW_OBJ_NAME_MAX];
	char password[XSW_OBJ_PASSWORD_MAX];

	int socket;

	char *last_error_mesg;

	int frame;

	win_t toplevel;
	pixmap_t toplevel_buf;

	/* Buttons. */
	mon_push_button_struct	shutdown_btn,
				messages_btn;

	/* Monitor stats. */
	mon_stats_struct	stats;

	/* Menu. */
	menu_struct		menu;

	/* Connect window. */
	con_win_struct		cw;

	/* Associated message window. */
	mesgwin_struct		mesgwin;

} monitor_struct;

extern monitor_struct **monitor;
extern int total_monitors;

extern monitor_struct *delete_monitor;


/*
 *	Dialog widget:
 */
extern dialog_win_struct dialog;



/* ***************************************************************** */

/* In monalloc.c */
extern int MonIsAllocated(int n);
extern int MonAllocate();
extern void MonDelete(int n);
extern void MonDeleteAll();
extern void MonReclaim();

/* In moncb.c */
extern int MonShutdownPBCB(void *ptr);
extern int MonMessagesPBCB(void *ptr);
extern int MonMenuCB(void *ptr, int op_code);
extern int MonConnectCB(
        void *ptr,
        char *address,
        char *port,
        char *name,
        char *password
);


/* In monmanage.c */
extern int MonInit(monitor_struct *m, int argc, char *argv[]);
extern void MonDraw(monitor_struct *m, int amount);
extern int MonManage(monitor_struct *m, event_t *event);
extern int MonManageAll(event_t *event);
extern void MonMap(monitor_struct *m);
extern void MonUnmap(monitor_struct *m);
extern void MonDestroy(monitor_struct *m);

/* In monmacros.c */
extern int NetIsSocketWritable(int s, int *error_level);
extern int NetIsSocketReadable(int s, int *error_level);
extern void MonSetErrorMesg(
	monitor_struct *m,
        char *mesg
);
extern int MonConnect(
        monitor_struct *m,
        char *address,
        int port
);
extern int MonDoSend(monitor_struct *m, char *buf);
extern int MonDoFetch(monitor_struct *m);


/* In main.c */
extern void MonitorSignalHandler(int s);
extern int MonitorScrollBarCB(scroll_bar_struct *sb);

extern int MonitorCreateMonitor(int argc, char *argv[]);
extern void MonitorResetTimmers();

extern int MonitorInit(int argc, char *argv[]);
extern void MonitorManage();
extern void MonitorShutdown();

/* In timming.c */
extern long MilliTime(void);


#endif /* MONITOR_H */
