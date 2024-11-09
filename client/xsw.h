/*
                  X S H I P W A R S   C L I E N T


	Copyright (C) 1997-2001 WolfPack Entertainment.

 */

#ifndef XSW_H
#define XSW_H


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>
#include <math.h>  
#include <fcntl.h>

#include <sys/stat.h>
#include <sys/time.h>

#ifdef JS_SUPPORT
// Dan S
extern "C" {
# include <jsw.h>
}
#endif  /* JS_SUPPORT */

#include "../include/os.h" 	/* OS `safety net' definations. */
#include "../include/graphics.h"
#include "../include/strexp.h"
#include "../include/string.h"
#include "../include/urlparse.h"
#include "../include/disk.h"
#include "../include/prochandle.h"

#include "../include/objects.h"	/* XSW Objects. */
#include "../include/isrefs.h"	/* Image set referances. */
#include "../include/reality.h"
#include "../include/cs.h"	/* Cyberspace network protocol. */


#include "../include/osw-x.h"
#include "../include/widget.h"		/* Widget system and OSW. */


/*
 *	Program name and version:
 */
#define PROG_NAME	"XShipWars"
#define PROG_VERSION	"1.34.0"

#define PROG_VERSION_MAJOR	1
#define PROG_VERSION_MINOR	34
#define PROG_VERSION_RELEASE	0


/*
 *	Client type code:
 *
 *	To identify this client to the universe server, sent in
 *	a CS_CODE_LOGIN.
 */
#define CLIENT_TYPE_CODE	6


/*
 *	Usage information:
 */
#define PROG_USAGE_MESSAGE "\
Usage: xsw [url] [options] [GUI_options]\n\
\n\
    [url] is the URL to the universe that you want to connect to.\n\
\n\
    [options] can be any of the following:\n\
\n\
        --rcfile <file>         Load configuration from <file>.\n\
        -f                      Same as --rcfile.\n\
        --control <type>        Specify which controller to use:\n\
                                   keyboard\n\
                                   joystick\n\
        -c                      Same as --control.\n\
        --help                  Prints (this) help screen and exits.\n\
        --version               Prints version information and exits.\n\
\n\
    Most customizations can be performed in the options menu.\n\
\n\
    Command line options override any options in the configuration file.\n\
\n\
    [GUI_options] can be any options standard to your GUI, consult your\n\
    GUI's manual for available options.\n\
\n"

   
/*
 *	Copyright information:
 */
#define PROG_COPYRIGHT "\
Copyright (C) 1997-2001 WolfPack Entertainment.\n\
This program is protected by international copyright laws and treaties,\n\
distribution and/or modification of this software in violation of the\n\
GNU Public License is strictly prohibited. Violators will be prosicuted\n\
to the fullest extent of the law."


/*
 *	Debug message footer:
 */
#define XSW_DEBUG_FOOTER "\
\n\
*** If you recieved this message unexpectedly and/or the program has\n\
*** crashed, please e-mail the entire above debug message and steps\n\
*** that you can recall which resulted in this to the addresses listed:\n\
*** at http://wolfpack.twu.net/contacts.html\n\
***************************************************************************\n"



/*
 *	Default network interface maximum capacity (in bytes per second):
 */
#define DEF_NET_LOAD_MAX	1440


/*
 *	Prompt Widget Limits:
 *
 *	For the prompt on the bridge window.
 */
#define DEF_PROMPT_BUF_LEN	256	/* In bytes. */
#define DEF_PROMPT_HIST_BUFS	30


/*
 *   Allowable color reduction for XPM images:
 *
 *	0     = 0% Reduction allowed (not recommended).
 *	40000 = (Default).
 *	60000 = 100% Reduction allowed.
 */
#define DEF_COLORCLOSENESS	40000


/*
 *   Default program directories:
 */
#ifdef __WIN32__
#define DEF_LOCAL_SHIPWARS_DIR	CWD_STR
#define DEF_XSW_TOPLEVEL_DIR	CWD_STR
#define DEF_XSW_IMAGES_DIR	CWD_STR"images"
#define DEF_XSW_SOUNDS_DIR	CWD_STR"sounds"
#define DEF_XSW_ETC_DIR		CWD_STR
#define DEF_ETC_DIR		CWD_STR
#define DEF_SW_SERVER_DIR	CWD_STR
#else
#define DEF_LOCAL_SHIPWARS_DIR	".shipwars"
#define DEF_XSW_TOPLEVEL_DIR	"/usr/share/games/xshipwars"
#define DEF_XSW_IMAGES_DIR	"/usr/share/games/xshipwars/images"
#define DEF_XSW_SOUNDS_DIR	"/usr/share/games/xshipwars/sounds"
#define DEF_XSW_ETC_DIR		"/usr/share/games/xshipwars/etc"
#define DEF_ETC_DIR		"/etc"
#define DEF_SW_SERVER_DIR	"/home/swserv"
#endif


/*
 *	File name extensions and masks:
 */
#define FN_ISREF_EXT		".isr"		/* Image set referances. */
#define FN_ISREF_EXT_MASK	"*.isr"

#define FN_OCSN_EXT		".ocsn"		/* Obj create script names. */
#define FN_OCSN_EXT_MASK	"*.ocsn"

#define FN_SS_EXT		".ss"		/* Sound scheme. */
#define FN_SS_EXT_MASK		"*.ss"

#define FN_STARCHART_EXT	".cht"		/* Star chart. */
#define FN_STARCHART_EXT_MASK	"*.cht"



/*
 *    Default configuration files:
 *
 *	These files are searched for in the DEF_LOCAL_SHIPWARS_DIR
 *	directory by default or from the command line or other
 *	configuration file specified location.
 *
 *	If these files cannot be found, a global one will be
 *	coppied to its default location.
 */
#define DEF_XSW_RCFILE			"xshipwarsrc"
#define DEF_UNIVERSES_FILE		"universes"

/*
 *	Default page configuration files:
 *
 *	Default parent is /usr/games/xshipwars/etc/.
 */
#define DEF_MAIN_PAGE_FILE	"main.page"
#define DEF_DESTROYED_PAGE_FILE	"destroyed.page"

/*
 *	Default log file location:
 *
 *	Default parent is user's home directory.
 */
#define DEF_XSW_LOG_FILE		"xsw.log"

/*
 *	Default OCSNs file location:
 *
 *	Default parent is /usr/games/xshipwars/etc/.
 */
#define DEF_XSW_OCSN_FILE		"default.ocsn"

/*
 *	Default ISRefs file location:
 *
 *	Default parent is /usr/games/xshipwars/images/.
 */
#define DEF_XSW_ISREF_FILE		"default.isr"

/*
 *	Default sound scheme file location:
 *
 *	Default parent is /usr/games/xshipwars/sounds/.
 */
#define DEF_XSW_SOUND_SCHEME_FILE	"default.ss"


/*
 *	Joystick Default File Names:
 *
 *	Modify the appropriate section for your OS as needed.
 *	These must be defined here (globally) as many functions
 *	need to know these values.
 *
 *	The DEF_JS_CALIBRATION_FILE is searched for in the user's
 *	home dir.
 */
#ifdef JS_SUPPORT

# ifdef __linux__
#  define DEF_JS_CALIBRATION_FILE	".joystick"
#  define DEV_JOYSTICK1			"/dev/js0"
#  define DEV_JOYSTICK2			"/dev/js1"
#  define DEV_JOYSTICK3			"/dev/js2"
#  define DEV_JOYSTICK4			"/dev/js3"
# endif /* __linux__ */

# ifdef __HPUX__
# endif /* __HPUX__ */

#endif /* JS_SUPPORT */



/*
 *	Default Server Address:
 *
 *	This is the default server address and port to connect to when
 *	no address is given at the command line or in the configuration
 *	file.
 */
#define DEF_SWSERV_ADDRESS   "localhost"
#define DEF_SWSERV_PORT      1701


/*
 *	Default anonymous/guest/observer login values:
 */
#define DEF_GUEST_LOGIN_NAME		"Guest"
#define DEF_GUEST_LOGIN_PASSWORD	"guest"

/*
 *	Default URL used for a newly created universe entry:
 */
#define DEF_UNIV_URL	"swserv://Guest:guest@localhost:1701"


/*
 *	XSW Procedure Codes:
 *
 *	Numeric values to identify which major procedure functions
 *	to call. These codes are often reffered to as op_code or
 *	actions.
 *
 *	Ie: Used in the main menu's labels and the quick
 *	menu entries.
 */
#define XSW_ACTION_NONE			0

/* Load files and run server. */
#define XSW_ACTION_RUN_SERVER		10
#define XSW_ACTION_LOAD_OCSN		11
#define XSW_ACTION_LOAD_ISREF		12
#define XSW_ACTION_LOAD_SS		13
#define XSW_ACTION_SERVER_SCRIPT	14
#define XSW_ACTION_LOAD_STARCHART_OVERLAY	15

/* Network actions. */
#define XSW_ACTION_CONNECT		100
#define XSW_ACTION_CONNECT_LAST		101
#define XSW_ACTION_REFRESH		102
#define XSW_ACTION_AINT			103	/* Auto interval toggle. */
#define XSW_ACTION_DISCONNECT		104

/* Game play controls. */
#define XSW_ACTION_SHIELDS		200
#define XSW_ACTION_CLOAK		201
#define XSW_ACTION_DMGCTL		202
#define XSW_ACTION_WLOCKNEXT		203
#define XSW_ACTION_WUNLOCK		204

/* Systems and display. */
#define XSW_ACTION_SYNCTIME		301
#define XSW_ACTION_DISPLABELS		302	/* Vs label cycle. */
#define XSW_ACTION_MEMORY		303
#define XSW_ACTION_OPTIONS		304	/* Map options win. */
#define XSW_ACTION_EDIT_KEYMAP		305	/* Map keymap win. */
#define XSW_ACTION_COMFERM_EXIT		306	/* Ask before exiting. */
#define XSW_ACTION_EXIT			307	/* Exit. */
#define XSW_ACTION_ECONOMY		308	/* Map economy win. */
#define XSW_ACTION_EDIT_JSMAP		309	/* Map jsmap win. */
#define XSW_ACTION_STARCHART		310	/* Map starchart win. */

/* To server commands. */
#define XSW_ACTION_WHO			401
#define XSW_ACTION_NETSTAT		402
#define XSW_ACTION_RMEMORY		403	/* Not XSW_ACTION_MEMORY. */


/* Links to other pages. */
#define XSW_ACTION_GOTO_MAIN		1000

#define XSW_ACTION_GOTO_DESTROYED	1010


/*
 *	XSW Images Codes:
 *
 *	IMPORTANT: These codes are NOT Image Set Referance numbers!
 *
 *	These are base images codes used soly by the client for
 *	its various internal images.
 */
/* Desktop icons. */
#define IMG_CODE_XSW_ICON		1
#define IMG_CODE_ECONOMY_ICON		2
#define IMG_CODE_UNIV_ICON		3	/* Not the list item icons. */
#define IMG_CODE_OPTIONS_ICON		4
#define IMG_CODE_STARCHART_ICON		5

/* Cursors and marks. */
#define IMG_CODE_VSMARK_OBJECT		10
#define IMG_CODE_VSMARK_VESSEL		11
#define IMG_CODE_VSMARK_INCOMINGFIRE	12	/* Incomming fire. */

#define IMG_CODE_VS_WEP_PROJECTILE	13
#define IMG_CODE_VS_WEP_PULSE		14
#define IMG_CODE_VS_WEP_STREAM		15

#define IMG_CODE_SCMARK_UNKNOWN		16
#define IMG_CODE_SCMARK_LOCKED		17
#define IMG_CODE_SCMARK_WEAPON		18
#define IMG_CODE_SCMARK_HOME		19
#define IMG_CODE_SCMARK_AREA		20

/* Icons. */
#define IMG_CODE_UNIV_STD_ICON		30
#define IMG_CODE_UNIV_HASLOGIN_ICON	31
#define IMG_CODE_UNIV_OLD_ICON		32
#define IMG_CODE_UNIV_UNKNOWN_ICON	33
#define IMG_CODE_ERROR_ICON		34
#define IMG_CODE_INFO_ICON		35

/* Startup splash background. */
#define IMG_CODE_STARTUP_BKG		40
#define IMG_CODE_STARTUP_PB_L		41
#define IMG_CODE_STARTUP_PB_R		42
#define IMG_CODE_STARTUP_PB_T		43

/* Consoles. */
#define IMG_CODE_STATS_CON1		50	/* Bridge consoles. */
#define IMG_CODE_STATS_CON2		51
#define IMG_CODE_STATS_CON3		52
#define IMG_CODE_STATS_CON4		53
#define IMG_CODE_SCANNER		54	/* Scanner background. */
#define IMG_CODE_SRO_CON1		55
#define IMG_CODE_SRO_CON2		56
#define IMG_CODE_SRO_CON3		57
#define IMG_CODE_MESG_CON		58

/* Console decorations. */
#define IMG_CODE_BPANEL_OL_HULL		70
#define IMG_CODE_BPANEL_OL_POWER	71
#define IMG_CODE_BPANEL_OL_VIS		72
#define IMG_CODE_BPANEL_OL_SHIELDS	73
#define IMG_CODE_BPANEL_OL_DMGCTL	74

#define IMG_CODE_BPANEL_OL_LTHROTTLE	75
#define IMG_CODE_BPANEL_OL_RTHROTTLE 	76

#define IMG_CODE_BPANEL_OL_THRUSTVECTOR	77

/* Starchart icons. */
#define IMG_CODE_SCHT_ZOOM_IN		80
#define IMG_CODE_SCHT_ZOOM_OUT		81
#define IMG_CODE_SCHT_JUMP_TO_PLAYER	82

/* Large message window background. */
#define IMG_CODE_MESG_SCR_BKG           90

/* Banners and labels. */
#define IMG_CODE_ENERGY_SAVER_MODE	100

/* Misc. */
#define IMG_CODE_LENSFLARE1		120
#define IMG_CODE_LENSFLARE2		121

#define IMG_CODE_STROBEGLOW1		122


/*      
 *   Server Object Update Send Interval:
 *
 *      In milliseconds. These are the minimum and maximum object updates
 *      intervals that we can request of the server to send us object
 *	updates.   The server code's minimum and maximum update intervals
 *	settings will override these minimum and maximum settings.
 *
 *	* These settings should match the server's default settings! *
 */
#define MIN_SERVER_UPDATE_INT	25
#define MAX_SERVER_UPDATE_INT	5000
#define SERVER_DEF_INT		1000



/*
 *   Object Assume Outdated Timeout:
 *
 *	Objects that have not been updated by network data from the
 *	server for this many milliseconds will not be drawn.
 *
 *	However the object will not be recycled.
 */
#define OBJECT_OUTDATED_TIMEOUT		6000


/*
 *   Maximum object create script name entries:
 *
 *	Do not confuse this with object create scripts on the server.
 *
 *	See the ocs_struct farter below.
 */
#define OCSN_MAX	500




/* ******************************************************************** */

/*
 *   Timming Intervals:
 *
 *	In milliseconds. These are various intervals to perform
 *	certain routine procedures.
 */
#define MEMORY_CLEAN_INTERVAL		150000		/* 2.5 minutes. */
#define CONSOLES_UPDATE_INTERVAL	5000


/*
 *	Client command maximum length:
 */
#define CLIENT_CMD_MAX		256


/*
 *	Prompt Modes:
 *
 *	These are codes to indicate which prompt mode XSW is in.
 *	0 means no prompt mode and that keys should be handled
 *	normally.
 *	THe global variable prompt_mode holds one of these codes
 *	to indicate to the various functions which prompt mode
 *	we are in.
 */
#define PROMPT_CODE_NONE	0
#define PROMPT_CODE_CONNECT	1
#define PROMPT_CODE_CLIENT_CMD	2
#define PROMPT_CODE_SERVER_CMD	3
#define PROMPT_CODE_MESSAGE	4
#define PROMPT_CODE_WEAPONFREQ	5
#define PROMPT_CODE_SHIELDFREQ	6
#define PROMPT_CODE_INTERCEPT	7
#define PROMPT_CODE_COM_CHANNEL	8
#define PROMPT_CODE_EXIT	9


/*
 *	Message buffer maximums:
 */
#define MESG_BUF_MAX_MESG_LEN		111	/* Characters. */
#define MESG_WIN_TOTAL_MESSAGES		250	/* Lines. */
#define MESG_WIN_COL_WRAP		110	/* Characters. */

/*
 *	Universe entry maximums:
 */
#define MAX_UNIVERSES			300
#define UNIV_MAX_ALIAS_LEN		128
#define UNIV_MAX_URL_LEN		MAX_URL_LEN
#define UNIV_MAX_COMMENT_LEN		256


/*
 *    General animation timmers codes:
 */
#define ANIM_TIMMER_GENERAL	0
#define ANIM_TIMMER_SHORTGLOW	1
#define ANIM_TIMMER_MEDIUMGLOW	2	/* Cursors, arrows. */
#define ANIM_TIMMER_LONGGLOW	3
#define ANIM_TIMMER_SHORTBLINK	4
#define ANIM_TIMMER_LONGBLINK	5

#define MAX_ANIM_TIMMERS	6	/* Must be GREATER than the highest
					 * ANIM_TIMMER code.
					 */

/*
 *	Global Animation Timmer Values:
 *
 *	These values are for internal global animation timmers,
 *
 *	Note: Values here have no affect on ISRef animated
 *	image sets.
 */
#define ANIM_TIMMER_MAXCOUNT_GENERAL		6
#define ANIM_TIMMER_MAXCOUNT_SHORTGLOW		4
#define ANIM_TIMMER_MAXCOUNT_MEDIUMGLOW		8
#define ANIM_TIMMER_MAXCOUNT_LONGGLOW		12
#define ANIM_TIMMER_MAXCOUNT_SHORTBLINK		2
#define ANIM_TIMMER_MAXCOUNT_LONGBLINK		2


/*
 *	Scanner Orientations:
 *
 *	Set in bridge_win.scanner_orient.
 */
#define SCANNER_ORIENT_GC	0
#define SCANNER_ORIENT_LOCAL	1


/*
 *	Controller Types:
 *
 *	Set in options.controller.
 */
#define CONTROLLER_KEYBOARD             0
#define CONTROLLER_JOYSTICK		1
#define CONTROLLER_POINTER		2	/* Not supported yet. */




/* ***************************************************************** */

/*
 *	Main parent process ID:
 */
extern "C" pid_t root_pid;


/*
 *      Runlevel:
 *      
 *      The main while() loop in function main() checks to see if runlevel
 *      is 2 or greater.   If runlevel drops to 1 or less, then the loop
 *      breaks and the program should exit.
 *
 *      0 = Shutting down.
 *      1 = Starting up.
 *      2 = Normal running.
 */
extern "C" int runlevel;


/*
 *	Current time since midnight in milliseconds:
 */
extern "C" time_t cur_millitime;

/*
 *	Current systime seconds:
 */
extern "C" time_t cur_systime;


/*
 *	Lapsed MilliTime:
 *
 *	In milliseconds, the time it took the previous loop to execute.
 *	This is compared to CYCLIC_MILLITIME_LAPSE (#defined in reality.h)
 *	to adjust for time lost in the last loop.
 */
extern "C" time_t lapsed_millitime;


/*
 *	Time Compensation:
 *
 *	This value is always in the range of 1.0 to <big number>.
 *	It is used as the coefficent to various momentum and movement
 *	calculations to compensate for lost time in the previous
 *	loop.
 */
extern "C" double time_compensation;


/*
 *	Prompt Mode:
 *
 *	Indicates which prompt mode we are in so key events are handled
 *	correctly.   0 means we are not in any prompt mode.
 */
extern "C" int prompt_mode;


/*
 *	Global debug values:
 *
 *	Used for run time debugging.
 */
#define DEBUG_LEVEL_NONE	0
#define DEBUG_LEVEL_ALL		1
#define DEBUG_LEVEL_MEMORY	2
#define DEBUG_LEVEL_NETWORK	3

typedef struct {

	int level;	/* One of DEBUG_LEVEL_* */
	double val;

} xsw_debug_struct;
extern "C" xsw_debug_struct debug;


/*
 *	Options:
 */
#define XSW_SOUNDS_NONE		0
#define XSW_SOUNDS_EVENTS	1
#define XSW_SOUNDS_ENGINE	2
#define XSW_SOUNDS_ALL		3

#define XSW_UNITS_XSW		0	/* XSW Internal units. */
#define XSW_UNITS_METRIC	1	/* AstroMetric. */
#define XSW_UNITS_ENGLISH	2	/* Universal English. */

typedef struct {

	/* RC file versions. */
	int	rc_version_major,
		rc_version_minor,
		rc_version_release;

	/* Units, one of XSW_UNITS_*. */
	int units;

	/* Async (passive) image loading? */
	char async_image_loading;

	/* Max pixels to load per loop. */
	int async_image_pixels;


	/* Log options. */
	char	log_client,
		log_net,
		log_errors;

	/* Redraw viewscreen, scanner, etc asyncronusly? */
	char async_redraws;


	/* Default scanner orientation (from startup). */
	int def_scanner_orient;

	/* Show superimposed viewscreen markings. */
	char show_viewscreen_marks;

	/*   Amount of viewscreen labeling:
	 *
	 *   0 = nothing.
         *   1 = labels.
         *   2 = network stats.
         *   3 = labels and network stats.
         */
	char show_viewscreen_labels;

	/* Viewscreen effects graphics. */
	char show_lens_flares;
	char show_strobe_glow;
	char show_nebula_glow;

	/*   Show object labels in formal format:
	 *
	 *   0 = never.
	 *   1 = as needed.
	 *   2 = always.
	 */
	char show_formal_label;

	/* Show network errors (undicipherable network data)? */
	char show_net_errors;

	/* Show server error message dialogs? */
	char show_server_errors;


	/* Update objects locally by predicting movement patterns. */
	char local_updates;


	/* Auto viewscreen zoom. */
	char auto_zoom;


	/* Sound level, one of XSW_SOUNDS_*. */
	char sounds;
	char music;	/* 0 for off, 1 for on. */


	/* Print event messages? */
	char display_events;

	/* Controller type, one of CONTROLLER_*. */
	char controller;
	char cmd_line_set_controller;

	/* Normal or bi-directional throttle control. */
#define THROTTLE_MODE_NORMAL		0
#define THROTTLE_MODE_BIDIRECTIONAL	1
#define THROTTLE_MODE_INCREMENTAL	2
	char throttle_mode;

#ifdef JS_SUPPORT
	/* Close joystick when bridge is out of focus? */
	char focus_out_js_close;
#endif /* JS_SUPPORT */


	/* Reduces CPU usage, good for idling. */
	char energy_saver_mode;


	/* Scanner range affected by enviroment? (set true, no cheating!) */
	char scanner_limiting;

	/* Notify when objects come into or leave scanner range? */
	char notify_scanner_contacts;


	/* Automatically map eco window as needed. */
	char auto_map_eco_win;

	/* Automatically map universe list window at startup. */
	char auto_map_univ_list_win;

	/* Clear star chart on connect. */
	char clear_chart_on_connect;

	/* Save configuration on exit. */
	char save_on_exit;

} xsw_option_struct;
extern "C" xsw_option_struct option;


/*
 *      Sound server connection info:
 */
typedef struct {

        /* Sound server type. */
#define SNDSERV_TYPE_NONE       0
#define SNDSERV_TYPE_YIFF       1
#define SNDSERV_TYPE_ESOUND     2
#define SNDSERV_TYPE_MIKMOD     3       /* Not supported. */
        int server_type;

	/* Script file that starts sound server. */
#define DEF_SOUND_SERVER_START_CMD      "/usr/sbin/yiff"
        char start_cmd[PATH_MAX + NAME_MAX];

	/* Connection argument to connect to sound server. */
#define DEF_SOUND_SERVER_CONNECT_ARG    "127.0.0.1:9433"
	char con_arg[MAX_URL_LEN];


	/*   Pointer to connection data (do not free, belongs to sound
	 *   server).  This pointer also serves as a marker to denote
	 *   if sound is initialized or not (not initialized if NULL).
	 */
	void *con_data;


	/*   Name of Audio mode that the sound server is to be in when
	 *   using current sound scheme (may not be applicatable to all
	 *   sound servers).
	 */
#define SNDSERV_AUDIO_MODE_NAME_MAX	256
	char audio_mode_name[SNDSERV_AUDIO_MODE_NAME_MAX];


	/* Background sound play ID (NULL if none is being played). */
	void *bkg_playid;

	/* Background mood code, one of SOUND_CODE_BKG_*. */
	int bkg_mood_code;

} xsw_sound_struct;
extern "C" xsw_sound_struct sound;


/*
 *   Directory names:
 *
 *	All values MUST be complete absolute paths.
 */
typedef struct {

	char startup[PATH_MAX];
	char home[PATH_MAX];

	char ltoplevel[PATH_MAX];	/* Local XSW toplevel dir. */

	char toplevel[PATH_MAX];	/* Global XSW toplevel dir. */
	char etc[PATH_MAX];		/* Configuration dir. */
	char images[PATH_MAX];		/* Images dir. */
	char sounds[PATH_MAX];		/* Sounds dir. */
	char downloads[PATH_MAX];	/* Screen shot dir. */

	char starchart[PATH_MAX];	/* Last starchart dir. */

} xsw_dname_struct;
extern "C" xsw_dname_struct dname;


/*
 *   File names:
 *
 *	Must be a absolute full path to the file.
 */
typedef struct {

    char rc[NAME_MAX + PATH_MAX];		/* Configuration file. */
    char universe_list[NAME_MAX + PATH_MAX];	/* Universe list file. */

#ifdef JS_SUPPORT
    char js_calib[NAME_MAX + PATH_MAX];		/* Joystick calibration file. */
#endif /* JS_SUPPORT */

    char main_page[NAME_MAX + PATH_MAX];
    char destroyed_page[NAME_MAX + PATH_MAX];

    char ocsn[NAME_MAX + PATH_MAX];		/* Object create script names. */
    char isr[NAME_MAX + PATH_MAX];		/* Imageset referances file. */
    char sound_scheme[NAME_MAX + PATH_MAX];     /* Sound scheme. */

    char log[NAME_MAX + PATH_MAX];		/* Log file. */

} xsw_fname_struct;
extern "C" xsw_fname_struct fname;


/*
 *   Next event scheduals:
 *
 *	Marks the next time in milliseconds that something
 *	is to be performed.
 */
typedef struct {

	time_t	update_check,
		viewscreen,		/* Redraw viewscreen. */
		consoles,		/* Redraw some bridge panel consoles. */
		memory_clean,		/* Reclaim memory. */
		lplayer_pos_send,	/* Next send player position. */
		wormhole_enter;

} xsw_next_struct;
extern "C" xsw_next_struct next;


/*
 *   FPS Counter:
 *
 *	Record for keeping track of the frames drawn per second
 *	on the viewscreen.
 */
typedef struct {

	int	fcount,		/* Frames drawn counter. */
		lfcount;	/* Frames drawn last time. */

	time_t	interval,	/* Should always be 1000 milliseconds. */
		next;		/* In milliseconds. */

} xsw_fps_counter_struct;
extern "C" xsw_fps_counter_struct fps_counter;


/*
 *   Animation Timmer
 *
 *	Timming and frame records for various animation timmers.
 *	Each timmer has a code #defined above.   Each timmer will have
 *	a different number of frames and interval.
 */
typedef struct {

	int	count,
		count_max;

	time_t	interval,	/* In milliseconds. */
		next;		/* In milliseconds. */

} xsw_genanim_timmer_struct;
extern "C" xsw_genanim_timmer_struct genanim_timmer[MAX_ANIM_TIMMERS];


/*
 *   Fonts:
 *
 *	Allocated GUI fonts.
 */
typedef struct {

	/* All purpose fonts. */
	font_t	*std,
		*std_bold;

	/* Bridge console panel fonts. */
	font_t  *console_heading,
		*console_standard,
		*console_message;

} xsw_font_struct;
extern "C" xsw_font_struct xsw_font;


/*
 *   Colors:
 *
 *	Allocated GUI color pixels and their values.
 *
 *	RGB values are stored in WColorStruct structures, they
 *	are members denoted with _cv postpended to their name.
 */
typedef struct {

	/* Standard text colors. */
	pixel_t		standard_text,
			bold_text,
			withdrawn_text;

	WColorStruct	standard_text_cv,
			bold_text_cv,
			withdrawn_text_cv;


	/* Bridge console panel readout colors. */
	pixel_t		bp_standard_text,
			bp_bold_text,
			bp_withdrawn_text,	/* Disabled. */

			bp_light_outline,
			bp_normal_outline,
			bp_dark_outline;

        WColorStruct    bp_standard_text_cv,
                        bp_bold_text_cv,
                        bp_withdrawn_text_cv,

			bp_light_outline_cv,
			bp_normal_outline_cv,
			bp_dark_outline_cv;

	/* Warning, danger, and critical colors. */
	pixel_t		bp_warning,
			bp_danger,
			bp_critical;

	WColorStruct	bp_warning_cv,
			bp_danger_cv,
			bp_critical_cv;

	/* Friendly, unknown, and hostile IFF colors. */
	pixel_t		bp_friendly,
			bp_unknown,
			bp_hostile;

	WColorStruct	bp_friendly_cv,
			bp_unknown_cv,
			bp_hostile_cv;

	/* Bridge console panel readout outline colors. */
	pixel_t		bpol_hull,
			bpol_power,
			bpol_vis,
			bpol_shields,
			bpol_dmgctl,
			bpol_throttle,
			bpol_throttle_rev;

	WColorStruct	bpol_hull_cv,
			bpol_power_cv,
			bpol_vis_cv,
			bpol_shields_cv,
			bpol_dmgctl_cv,
			bpol_throttle_cv,
			bpol_throttle_rev_cv;

	/* Viewscreen labels (no corresponding pixel value). */
	WColorStruct	vs_label_bg_cv,
			vs_label_fg_cv;


	/* Scanner mark colors. */
	pixel_t		scmark_unknown,		/* Most objects. */
			scmark_locked,
			scmark_weapon,
			scmark_home,
			scmark_area;

	WColorStruct	scmark_unknown_cv,
			scmark_locked_cv,
			scmark_weapon_cv,
			scmark_home_cv,
			scmark_area_cv;

	/* Keymap edit window. */
	pixel_t		keymap_query_bg,
			keymap_query_fg;

	WColorStruct	keymap_query_bg_cv,
			keymap_query_fg_cv;


	/* Viewscreen graphics colors (no corresponding pixel value). */
	WColorStruct	heading_arrow,
                        lock_arrow,
                        lock_cursor,
                        stream_green,
                        stream_yellow,   
                        stream_purple,
                        stream_orange,
                        shield_blue,
			visibility_marker,
                        star_glow;

	/* Starchart. */
	pixel_t		chart_bg,
			chart_grid,
			chart_sector_grid,
			chart_cross_hairs;

        WColorStruct    chart_bg_cv,
                        chart_grid_cv,
                        chart_sector_grid_cv,
			chart_cross_hairs_cv;

} xsw_color_struct;
extern "C" xsw_color_struct xsw_color;


/*
 *    XSW Cursors:
 *
 *	Allocated GUI cursors.
 */
typedef struct {

	WCursor	*scanner_lock,
		*text,
		*wait,

		*translate,
		*zoom;

} xsw_cursor_struct;
extern "C" xsw_cursor_struct xsw_cursor;

/*
 *   XSW Image Label Type:
 *
 *	General purpose image label structure
 *	(used in the main menu, see page_label_struct).
 */
typedef struct {

	char *filename;

	char pos_by_percent;	/*   If true, coordinates are calculated
                                 *   by percent.
                                 */
	char size_by_percent;	/*   If true, size is calculated by
                                 *   percent.
                                 */

	int x, y;		/*   If pos_by_percent is true, this is
                                 *   in percent.
                                 */
	image_t *image;

} xsw_imglabel_struct;


/* ********************************************************************* */

/*
 *	Page structure:
 */
#define PAGE_LABELS_PER_LABEL	3
typedef struct {

	char map_state;
	char allow_transparency;
	int op_code;			/* One of XSW_ACTION_* */

	xsw_imglabel_struct imglabel[PAGE_LABELS_PER_LABEL];

	char *hint_mesg;

} page_label_struct;

typedef struct {

	char map_state;
	int x, y;
	unsigned int width, height;
	char is_in_focus;
	visibility_t visibility_state;

	/* Background image. */
	char *bg_filename;
	image_t *bg_image;
	int bg_image_draw_code;		/* One of PAGE_BKG_DRAW_*. */

	/* Selected label information. */
	int sel_label;	/* -1 for none. */
	int sel_label_state;		/* One of PAGE_LABEL_STATE_*. */

	/* Label and action codes. */
	page_label_struct **label;
	int total_labels;

} page_struct;



/*
 *	Bridge window button position structure.
 */
typedef struct {

	int x, y;
	unsigned int width, height;

} bpanel_btnpos_struct;

/* Button position codes. */
#define BPANEL_BTNPOS_PSHIELDFREQ_UP	1
#define BPANEL_BTNPOS_PSHIELDFREQ_DOWN	2
#define BPANEL_BTNPOS_PWEAPONFREQ_UP	3
#define BPANEL_BTNPOS_PWEAPONFREQ_DOWN	4
#define BPANEL_BTNPOS_PCOMCHANNEL_UP	5
#define BPANEL_BTNPOS_PCOMCHANNEL_DOWN	6

#define BPANEL_BTNPOS_PSHIELDS		7
#define BPANEL_BTNPOS_PCLOAK		8
#define BPANEL_BTNPOS_PDMGCTL		9

#define BPANEL_BTNPOS_PINTERCEPT	10
#define BPANEL_BTNPOS_PLOCK		11
#define BPANEL_BTNPOS_PLOCKNEXT		12

#define BPANEL_BTNPOS_PWEPYIELD		13
#define BPANEL_BTNPOS_PSELWEP1		14
#define BPANEL_BTNPOS_PSELWEP2		15
#define BPANEL_BTNPOS_PSELWEP3		16
#define BPANEL_BTNPOS_PSELWEP4		17

#define BPANEL_BTNPOS_PENGINESTATE	30


#define BPANEL_TOTAL_BTNPOS	31
extern "C" bpanel_btnpos_struct **bpanel_btnpos;
extern "C" int total_bpanel_btnpos;


/* Viewscreen zoom ranges. */
#define VS_ZOOM_MIN 0.3
#define VS_ZOOM_MAX 1.0		/* Must not be greater than 1. */
#define VS_ZOOM_INC 0.005


/* Bridge console panel codes. */
#define BPANEL_DETAIL_ALL		0

#define BPANEL_DETAIL_P1		1
#define BPANEL_DETAIL_P2		2
#define BPANEL_DETAIL_P3		3
#define BPANEL_DETAIL_P4		4

#define BPANEL_DETAIL_S1		5
#define BPANEL_DETAIL_S2		6
#define BPANEL_DETAIL_S3		7

#define BPANEL_DETAIL_PNAME		20
#define BPANEL_DETAIL_PSHIELDFREQ	21
#define BPANEL_DETAIL_PWEAPONFREQ	22
#define BPANEL_DETAIL_PCOMCHANNEL 	23

#define BPANEL_DETAIL_PHULL	30
#define BPANEL_DETAIL_PPOWER	31
#define BPANEL_DETAIL_PSHIELDS	32
#define BPANEL_DETAIL_PVIS	33
#define BPANEL_DETAIL_PDMGCTL	34

#define BPANEL_DETAIL_SNAME	40
#define BPANEL_DETAIL_SEMPIRE	41
#define BPANEL_DETAIL_SHULL	42
#define BPANEL_DETAIL_SPOWER  	43
#define BPANEL_DETAIL_SSHIELDS	44
#define BPANEL_DETAIL_SVIS	45
#define BPANEL_DETAIL_SBEARING	46
#define BPANEL_DETAIL_SDISTANCE	47
#define BPANEL_DETAIL_SDMGCTL	48

#define BPANEL_DETAIL_PINAME	50
#define BPANEL_DETAIL_PWLOCK	51
#define BPANEL_DETAIL_PWEAPONS	52

#define BPANEL_DETAIL_SINAME	60
#define BPANEL_DETAIL_SWLOCK	61
#define BPANEL_DETAIL_SWEAPONS	62

#define BPANEL_DETAIL_SANTIMATTER	63
#define BPANEL_DETAIL_SHEADING		64	/* And coordinates. */

#define BPANEL_DETAIL_PTHROTTLE		70
#define BPANEL_DETAIL_PVELOCITY		71
#define BPANEL_DETAIL_PANTIMATTER	72
#define BPANEL_DETAIL_PENGINESTATE	73
#define BPANEL_DETAIL_PHEADING		74	/* And coordinates. */
#define BPANEL_DETAIL_PTHRUSTVECTOR	75

/*
 *	Bridge window structure:
 */
typedef struct {

	char map_state;
	int x, y;
	unsigned int width, height;
	char is_in_focus;
	visibility_t visibility_state;

	int preset_zoom_code;	/* Current preset zoom code. */

	/* Toplevel. */
	win_t toplevel;

	/* Viewscreen. */
	char viewscreen_map_state;
        win_t viewscreen;
        shared_image_t *viewscreen_image;
        unsigned int viewscreen_width, viewscreen_height;
        visibility_t viewscreen_vis_state;      /* Visibility state. */
        double viewscreen_gamma;
        time_t viewscreen_int;	/* Redraw interval in milliseconds. */
        double viewscreen_zoom;	/* Zoom coeff. */

        /* Selected weapon on viewscreen image. */
        image_t *vs_weapon_image;   
        pixmap_t vs_weapon_buf;
        unsigned int vs_weapon_width, vs_weapon_height;

        /* Netstats label on viewscreen. */
        image_t *net_stats_image;
        pixmap_t net_stats_buf;
        unsigned int net_stats_width, net_stats_height;


        /* Pages. */
	page_struct	*cur_page;
        page_struct	main_page,		/* Main menu. */
			destroyed_page;		/* You got destroyed. */


        /*   Buffer for console panels.  Since all are of same
	 *   size we can just use one buffer.
         */
        pixmap_t pan_buf;

	/*   Tempory image for drawing background for stats_con2 and
         *   sro_con1 (since it involves blitting an object on it).
	 */
	shared_image_t	*pan_p2_img,
			*pan_p3_img,

			*pan_s1_img,
			*pan_s3_img;


        /* Player stats console panels. */
	win_t	pan_p1,
		pan_p2,
		pan_p3,
		pan_p4;

	unsigned int	pan_p1_width,
			pan_p1_height,
			pan_p2_width,
			pan_p2_height,
			pan_p3_width,
			pan_p3_height,
			pan_p4_width,
			pan_p4_height;

	win_t	weapon_yield_iwin;	/* Input window for weapon yield scale. */


        /* Scanner. */
        char scanner_map_state;
        win_t scanner;
        shared_image_t *scanner_image;
        unsigned int scanner_width, scanner_height;
	visibility_t scanner_vis_state;
        double scanner_zoom;
        char scanner_orient;
        scale_bar_struct scanner_sb;         /* Scanner scale bar. */

	image_t	*scanner_range_label,
		*scanner_loc_label;

        /* Scanner readout console. */
	win_t	pan_s1,
		pan_s2,
		pan_s3;
	unsigned int	pan_s1_width,
			pan_s1_height,
			pan_s2_width,
			pan_s2_height,
			pan_s3_width,
			pan_s3_height;


        /* Message box. */
        win_t mesg_box;
        pixmap_t mesg_box_buf;
	int	mesg_box_x,
		mesg_box_y;
	unsigned int	mesg_box_width,
			mesg_box_height;
        int line_spacing;
        scroll_bar_struct mesg_box_sb;

        /* Prompt. */
        prompt_window_struct prompt;

} xsw_bridge_win_struct;
extern "C" xsw_bridge_win_struct bridge_win;

/* Large message window. */
typedef struct {

	char map_state;
	int x, y;
	unsigned int width, height;
	char is_in_focus;
	visibility_t visibility_state;

	win_t toplevel;
	pixmap_t toplevel_buf;

	int line_spacing;

	scroll_bar_struct scroll_bar;

	prompt_window_struct prompt;

} xsw_lg_mesg_win_struct;
extern "C" xsw_lg_mesg_win_struct lg_mesg_win;


/*
 *    Quick menu:
 *
 *	This should be moved to the bridge window structure.
 */
typedef struct {

	menu_struct menu;

} xsw_qmenu_struct;
extern "C" xsw_qmenu_struct qmenu;

/* 
 *    Economy buy and sell window:
 */
typedef struct {

 	char map_state;
 	int x, y;
 	unsigned int width, height;
 	char is_in_focus;
 	visibility_t visibility_state;
 	bool_t disabled;


	int proprietor_obj;

	prompt_window_struct	proprietor_prompt;

        win_t toplevel;
        pixmap_t toplevel_buf;

	push_button_struct	refresh_btn;

	colum_list_struct	inventory;

	prompt_window_struct	amount_prompt;

	push_button_struct	buy_btn,
				sell_btn,
				close_btn;

} xsw_eco_win_struct;
extern "C" xsw_eco_win_struct eco_win;


/* All purpose error dialog. */
extern "C" dialog_win_struct err_dw;

/* All purpose info dialog. */
extern "C" dialog_win_struct info_dw;


/*
 *   Primary file browser:
 */
extern "C" fbrowser_struct pri_fbrowser;
extern "C" int pri_fb_loadop;		/* Load operation. */

#define PRI_FB_LOADOP_NONE		0	/* Do nothing. */
#define PRI_FB_LOADOP_RUN_SERVER	1
#define PRI_FB_LOADOP_OCSN		2
#define PRI_FB_LOADOP_ISREF		3
#define PRI_FB_LOADOP_SS		4	/* Sound scheme. */
#define PRI_FB_LOADOP_SERVER_SCRIPT	10	/* Server commands script. */
#define PRI_FB_LOADOP_STARCHART_OVERLAY	11

#define PRI_FB_SAVEOP_STARCHART		30


/*
 *	XSW Images:
 *
 *	Images used internally by the program.
 *
 *	IMPORTANT: These are not Image Set Referance numbers.
 */
typedef struct {

	char load_state;	/* 0 = not loaded, 1 = loaded. */

	char *filename;
	image_t *image;

} xsw_image_struct;
extern "C" xsw_image_struct **xsw_image;

extern "C" int total_images;


/*
 *	Primary Message Buffer:
 *
 *	For server and local program output messages.
 *
 *	The messages here are displayed on the large message window
 *	and on the bridge window's message box.
 */
typedef struct {

	/* Message. */
	char message[MESG_BUF_MAX_MESG_LEN];

	/* Color of message. */
	pixel_t pixel;

	/* Selection mark positions, can be -1. */
	int	sel_start,
		sel_end;

} xsw_pri_mesg_buf_struct;
extern "C" xsw_pri_mesg_buf_struct pri_mesg_buf[MESG_WIN_TOTAL_MESSAGES];


/*
 *	Quick menu commands:
 *
 *	Commands for the quick menu loaded from configuration, these
 *	are not the actual menu items allocated in the menu widget
 *	for the quick menu.
 */
typedef struct {

	int type;		/* Menu widget item type (not action). */
	int action;		/* Action, one of XSW_ACTION_*. */
	char *name;		/* Name of command. */
	image_t *icon;		/* Filename of icon (NULL for none). */

} qmenu_command_struct;
extern "C" qmenu_command_struct **qmenu_command;
extern "C" int total_qmenu_commands;


/*
 *    Program memory stat structure:
 */
typedef struct {

	/* Memory used. */
	long	total,
		gui,
		sound,
		images,		/* Not counting isrefs. */
		univ_entries,
		objects,
		isrefs,
		ocsns,
		vs_labels,	/* Viewscreen labels. */
		scanner_contacts;

} xsw_mem_stat_struct;


/*
 *	Image sets list (graphics for XSW objects):
 */
extern "C" isref_struct **isref;
extern "C" int total_isrefs;


/*
 *    XSW Objects List:
 */
extern "C" xsw_object_struct **xsw_object;
extern "C" int total_objects;

/*
 *    In range objects list:
 *
 *	Array containing pointers to objects that are in range
 *	of player.
 */
extern "C" xsw_object_struct **inrange_xsw_object;
extern "C" int total_inrange_objects;


/*
 *   Object label images:
 *
 *	These label images are displayed next to objects on
 *	the bridge window's viewscreen.
 */
typedef struct
{
	/* Label image. */
	image_t *image;

	/* Pointer to object (so we know which one this belongs to). */
	xsw_object_struct *obj_ptr;

} vs_object_label_struct;
extern "C" vs_object_label_struct **vs_object_label;

extern "C" int total_vs_object_labels;


#ifdef JS_SUPPORT
/*
 *	Joystick mapping configuration:
 *
 *	Not to be confused with the game controller state structure 
 *	gctl_struct.
 */
#define JSMAP_AXIS_OP_NONE			-1
#define JSMAP_AXIS_OP_TURN			0
#define JSMAP_AXIS_OP_THROTTLE			1
#define JSMAP_AXIS_OP_THRUST_DIR		2
#define JSMAP_AXIS_OP_AIM_WEAPON_HEADING	3
#define JSMAP_AXIS_OP_AIM_WEAPON_PITCH		4

#define JSMAP_AXIS_OP_SCANNER_ZOOM		10
#define JSMAP_AXIS_OP_VS_ZOOM			11

typedef struct {

	int op_code;	/* Specifies what this axis controls. */

} jsmap_axis_struct;

typedef struct {

	keycode_t keycode;	/* Specfies the keycode generated by button. */
	bool_t state;

} jsmap_button_struct;

typedef struct
{
	char			*device_name;

#ifdef JSW_H
	js_data_struct		jsd;
#endif

	jsmap_axis_struct	**axis;
	int 			total_axises;

	jsmap_button_struct	**button;
	int 			total_buttons;

} jsmap_struct;

extern "C" jsmap_struct **jsmap;
extern "C" int total_jsmaps;	/* Also specifies total number of joysticks. */

#endif /* JS_SUPPORT */



/*
 *   Game controller values:
 *
 *	A wrapper for information about the game controller's current.
 *	states (such as joystick position, button states, etc).
 *
 *	The information here is fetched once per loop and functions
 *	use the information herein to tell what state the game controller
 *	is in regardless of what game controller is being used (keyboard,
 *	joystick, pointer, etc).
 */
typedef struct {

	double	turn,		/* -1 to 1. */
		throttle,	/* Depends on option.throttle_mode. */
		thrust_dir,	/* -1 to 1 (0 is directly backwards). */
		vs_zoom,	/* 0 to 1. */
		scanner_zoom,	/* 0 to 1. */
		aim_weapon_heading,	/* -1 to 1. */
		aim_weapon_pitch;	/* -1 to 1. */

	u_int8_t	fire_weapon,
			omni_dir_thrust,
			external_dampers;	/* Brakes. */

} xsw_gctl_struct;
extern "C" xsw_gctl_struct gctl[1];


/*
 *   Local control settings:
 *
 *	Client emulated simulation values to enhance realism.
 *	This has no affect on what happens on the server.
 *
 *	Do not confuse this with the gctl_struct.
 */
typedef struct {

	/* Toggles weapons online/offline (safety lock). */
	char weapons_online;

	/* Frequency and yield of weapon. */
	double	weapon_freq,
		weapon_yield;	/* 0.0 to 1.0. */

	/* Direction to fire weapon (relative to vessel, in radians). */
	double	weapon_fire_heading,
		weapon_fire_pitch;

} xsw_local_control_struct;
extern "C" xsw_local_control_struct local_control;

/*
 *   Warnings:
 *
 *	Warning markers for the player object.
 */
typedef struct {

        char weapons_lock;	/* Someone locked on us. */
        char incoming_fire;	/* Weapons fired at us. */

} xsw_warning_struct;
extern "C" xsw_warning_struct warning;


/*
 *    Message Squelch:
 *
 *	In communication messages, any ship with the name
 *	matching any name in this list will not have its
 *	message displayed.
 */
extern "C" char **message_squelch;
extern "C" int total_message_squelches;


/*
 *   Global ShipWars Universe units:          
 */
extern "C" sw_units_struct sw_units;

/*
 *   Sector legend:
 *
 *	Defines the sizes and bounds of each sector.
 *	All units are in XSW real units unless otherwise noted.
 */
typedef struct {

	double x_len, y_len, z_len;

        double x_min, x_max;
	double y_min, y_max;
	double z_min, z_max;

} xsw_sector_legend_struct;
extern "C" xsw_sector_legend_struct sector_legend;

/*
 *   Scanner contacts:
 *
 *	List record of objects in scanner range.
 *
 *	Used to keep track of which objects have just entered
 *	or left sensor range.
 */
typedef struct {

	int object_num; 

} scanner_contacts_struct;
extern "C" scanner_contacts_struct **scanner_contact;

extern "C" int total_scanner_contacts;



/*
 *   Object Create Script Names:
 *
 *	For referancing an OCS type to a name. Purpose for these is
 *	only to provide a name for a type of OCS.
 *
 *	NOTE: This OCS structure differs with the OCS structure for
 *	the server is different!
 */
#define OCS_TYPE_GARBAGE	0
typedef struct
{
	int type;

	char name[XSW_OBJ_NAME_MAX];
	image_t *icon;

} ocsn_struct;
extern "C" ocsn_struct **ocsn;
extern "C" int total_ocsns;


/*
 *   Load Monitoring:
 *
 *	The members in the struct loadstat and struct auto_interval_tune
 *	contain various network statistics in order for Auto Interval Tuning
 *	to work properly.
 */
typedef struct {

    long net_load_max;	/* Maximum bytes per 1000 milliseconds of 
			 * network connection.
			 * A 28.8k modem would have 3600. */
    long rx_interval;	/* Bytes recieved per 1000 milliseconds. */
    long sx_interval;	/* Bytes sent per 1000 milliseconds. */
    long rx_ilast;	/* Last bytes recieved per 1000 milliseconds. */
    long sx_ilast;      /* Last bytes sent per 1000 milliseconds. */

} xsw_loadstat_struct;
extern "C" xsw_loadstat_struct loadstat;

/*
 *   Auto interval tuning:
 *
 *	Relies on information in loadstat.
 */
typedef struct {

        char state;     /* 0 = off, 1 = on. */

        /* Returned every this many milliseconds. */
#define AINT_DEF_TUNE_INT       1000
        time_t	interval,	/* Always 1000 ms (every second). */
		next;

} xsw_auto_interval_tune_struct;
extern "C" xsw_auto_interval_tune_struct auto_interval_tune;


/*
 *	Server script upload structure:
 */
typedef struct {

	char *filename;
	FILE *fp;

	off_t filepos;
	off_t filesize;

	/* Arguments to substitute. */
	char **argv;
	int argc;

} serv_script_struct;
extern "C" serv_script_struct **serv_script;
extern "C" int total_serv_scripts;


/* ******************************************************************* */

/* In autointerval.c */
extern "C" int AIntvTuneHandleAdjust(void);


/* In bridgedraw.c */
extern "C" void BridgeWinDrawPanel(
        int obj_num,		/* Subject object. */
        int panel_detail	/* One of BPANEL_DETAIL_* */
);
extern "C" void BridgeWinDrawAll(void);
extern "C" int BridgeDrawMessages(void);

/* In bridgegui.c */
extern "C" int BridgeWinInit(int argc, char *argv[]);
extern "C" void BridgeWinResize(void);
extern "C" void BridgeWinResizePreset(int step);
extern "C" void BridgeWinMap(void);
extern "C" void BridgeWinUnmap(void);
extern "C" void BridgeWinDestroy(void);

/* In bridgemanage.c */
extern "C" void BridgeMessagesMark(
        int start_x, int start_y,
        int end_x, int end_y
);
extern "C" void BridgePrintSubjectStats(int object_num);
extern "C" void BridgeWarnWeaponsOffline(int object_num);
extern "C" int BridgeManagePromptExec(event_t *event);
extern "C" int BridgeManage(event_t *event);


/* In bridgescrshot.c */
extern "C" int BridgeDoScreenShot(
	char *save_dir,
	int start_x, int start_y,
	int end_x, int end_y
);


/* In cmd*.c */
extern "C" int CmdHandleInput(const char *input);

extern "C" int CmdAutoInterval(const char *arg);
extern "C" int CmdConnect(const char *arg);
extern "C" int CmdDebug(const char *arg);
extern "C" int CmdDisconnect(const char *arg);
extern "C" int CmdExit(const char *arg);
extern "C" int CmdHelp(const char *arg);
extern "C" int CmdLog(const char *arg);
extern "C" int CmdLoginName(const char *arg);
extern "C" int CmdLoginPassword(const char *arg);
extern "C" int CmdMemory(const char *arg);
extern "C" int CmdNetInterval(const char *arg);
extern "C" int CmdRefresh(const char *arg);
extern "C" int CmdServScript(const char *arg);
extern "C" int CmdSet(const char *arg);
extern "C" int CmdSynctime(const char *arg);
extern "C" int CmdTest(const char *arg);
extern "C" int CmdVersion(const char *arg);


/* In db.c */
extern "C" int DBIsObjectGarbage(int object_num);

extern "C" xsw_object_struct **DBAllocObjectPointers(
	xsw_object_struct **cur_obj_ptrs,
	int num_objects
);
extern "C" xsw_object_struct *DBAllocObject(void);

extern "C" int DBCreateObject(
            int imageset,
            int type,
            int owner,
            double x,
            double y,
            double z,
            double heading,
            double pitch,
            double bank
);
extern "C" int DBCreateExplicitObject(
            int object_num,
            int imageset,
            int type,
            int owner,
            double x,
            double y,
            double z,
            double heading,
            double pitch,
            double bank
);
extern "C" void DBDeleteObject(xsw_object_struct *obj_ptr);
extern "C" void DBDeleteAllObjects(void);
extern "C" void DBRecycleObject(int object_num);

extern "C" void DBReclaim(void);


/* In dbinrange.c */
extern "C" void DBInRangeUpdate(int object_num);
extern "C" int DBInRangeAdd(int ref_obj, int tar_obj, char check_range);
extern "C" void DBInRangeDelete(xsw_object_struct *obj_ptr);
extern "C" void DBInRangeDeleteAll(void);


/* In dbutil.c */
extern "C" int DBCreateObjectEconomyProduct(
	int object_num,
	char *product_name,
	xswo_credits_t sell_price,
	xswo_credits_t buy_price,
	double product_amount,
	double product_max
);

extern "C" int DBObjectTractor(int src_obj, int tar_obj);
extern "C" void DBObjectUntractor(int src_obj, int tar_obj);

extern "C" int DBValidateObjectName(char *name);
extern "C" int DBValidateObjectPassword(char *password);

extern "C" double DBGetObjectVisibility(int object_num);
extern "C" double DBGetObjectVisibilityPtr(xsw_object_struct *obj_ptr);

extern "C" int DBGetTopObjectNumber(void);
extern "C" int DBGetObjectNumByPtr(xsw_object_struct *obj_ptr);
extern "C" char *DBGetFormalNameStr(int object_num);
extern "C" char *DBGetObjectVectorName(double theta);
extern "C" double DBGetObjectVisibility(int object_num);
extern "C" double DBGetObjectVisibilityPtr(xsw_object_struct *obj_ptr);

extern "C" void DBSetPlayerObject(int object_num);


/* In ecowin.c */
extern "C" void EcoWinUnfocusPrompts(void);
extern "C" int EcoWinDoAddInventory(
	int customer_obj,
	int proprietor_obj,
	xsw_ecoproduct_struct product
);
extern "C" void EcoWinDoDeleteInventory(void);

extern "C" int EcoWinRefreshPBCB(void *ptr);
extern "C" int EcoWinBuyPBCB(void *ptr);
extern "C" int EcoWinSellPBCB(void *ptr);
extern "C" int EcoWinClosePBCB(void *ptr);

extern "C" int EcoWinInit(void);
extern "C" int EcoWinResize(void);
extern "C" int EcoWinDraw(void);
extern "C" int EcoWinManage(event_t *event);
extern "C" void EcoWinMap(void);
extern "C" void EcoWinUnmap(void);
extern "C" void EcoWinDestroy(void);


/* In images.c */
extern "C" int IMGIsImageNumAllocated(int image_num);
extern "C" int IMGIsImageNumLoaded(int image_num);

extern "C" int IMGResize(
	int image_num,
	unsigned int width,
	unsigned int height
);
extern "C" int IMGAllocateExplicit(int image_num);
extern "C" int IMGLoadImage(int image_num, char *filename);
extern "C" int IMGLoadImageData(int image_num, unsigned char *data);
extern "C" void IMGUnload(int image_num);

extern "C" int IMGDoLoadAll(void);
extern "C" void IMGDoUnloadAll(void);


/* In imglabels.c */
extern "C" int ImgLabelIsAllocated(xsw_imglabel_struct *label);
extern "C" int ImgLabelIsLoaded(xsw_imglabel_struct *label);

extern "C" xsw_imglabel_struct *ImgLabelAllocate(void);
extern "C" void ImgLabelReset(xsw_imglabel_struct *label);
extern "C" void ImgLabelDestroy(xsw_imglabel_struct *label);


/* In gctl.c */
extern "C" int GCtlInit(int controller);
extern "C" void GCtlUpdate(int controller_type);
extern "C" void GCtlShutdown(void);


#ifdef JS_SUPPORT
/* In jsmap.c */
extern "C" int JSMapIsAllocated(int n);
extern "C" int JSMapAllocate();
extern "C" void JSMapSyncWithData(jsmap_struct *jsm);
extern "C" void JSMapDelete(int n);
extern "C" void JSMapDeleteAll();
#endif /* JS_SUPPORT */


/* In isrefs.c */
extern "C" int ISRefIsAllocated(int isref_num);
extern "C" int ISRefIsLoaded(int isref_num);
extern "C" void ISRefReset(int isref_num);
extern "C" int ISRefCreateExplicit(int isref_num);
extern "C" int ISRefLoadAsDefault(int isref_num);
extern "C" int ISRefLoad(int isref_num);
extern "C" void ISRefUnload(int isref_num);
extern "C" void ISRefDelete(int isref_num);
extern "C" void ISRefDeleteAll(void);
extern "C" void ISRefReclaimMemory(void);
extern "C" void ISRefManage(void);


/* In isrefsfile.c */
extern "C" int ISRefLoadFromFile(char *filename);


/* In log.c */
extern "C" int LogAppendLineFormatted(char *filename, char *str);


/* In main.c */
extern "C" void XSWDoPrintDebug(void);
extern "C" void XSWDoHelp(void);
extern "C" void XSWDoVersion(void);
extern "C" void XSWDoHelpMesgWin(void);
extern "C" int XSWIsDescriptorValid(int s);
extern "C" int XSWStartServer(char *cmd);
extern "C" int XSWLoadOCSN(char *path);
extern "C" int XSWLoadIsrefs(char *path);
extern "C" int XSWLoadSS(char *path);
extern "C" int XSWLoadColors(void);
extern "C" void XSWFreeColors(void);
extern "C" void XSWGetProgMemory(xsw_mem_stat_struct *buf);
extern "C" void XSWReclaimGlobalMemory(bool_t verbose);
extern "C" void XSWDoUnfocusAllWindows(void);
extern "C" void XSWDoRestackWindows(void);
extern "C" void XSWManageGUI(void);
extern "C" void XSWManageSound(void);
extern "C" void XSWDoChangeBackgroundMusic();
extern "C" int XSWScrollBarCB(scroll_bar_struct *ptr);
extern "C" void XSWHandleSignal(int s);
extern "C" void XSWDoResetTimmers(void);
extern "C" int XSWInit(int argc, char *argv[]);
extern "C" void XSWManage(void);
extern "C" void XSWShutdown(void);

/* In mesgwin.c */
extern "C" int MesgAdd(char *new_mesg, pixel_t mesg_color);
extern "C" int MesgReplace(
	char *new_mesg,
	pixel_t mesg_color,
	int mesg_num
);

extern "C" void MesgWinUnmarkAll(void);
extern "C" void MesgWinUpdateMark(
        int start_x, int start_y,
        int end_x, int end_y
);
extern "C" int MesgPutDDE(void);

extern "C" void MesgDrawAll(void);

extern "C" int LMesgWinInit(void);
extern "C" int LMesgWinResize(void);
extern "C" int LMesgWinDraw(void);
extern "C" int LMesgWinManage(event_t *event);
extern "C" void LMesgWinMap(void);
extern "C" void LMesgWinUnmap(void);
extern "C" void LMesgWinDestroy(void);


/* In ocsnames.c */
extern "C" void OCSReclaimMemory(void);
extern "C" void OCSReset(int ocsn_num);
extern "C" void OCSDeleteAll(void);
extern "C" int OCSIsGarbage(int ocsn_num);
extern "C" int OCSCreate(int type);
extern "C" int OCSCreateExplicit(int ocsn_num, int type);
extern "C" void OCSRecycle(int ocsn_num);
extern "C" int OCSGetTop(void);
extern "C" int OCSGetByCode(int code);

/* In ocsnamesfile.c */
extern "C" int OCSLoadFromFile(char *filename);


/* In optwingui.c */
extern "C" int OptWinInit(void);
extern "C" int OptWinTabRemap(int tab);
extern "C" int OptWinDraw(void);
extern "C" int OptWinManage(event_t *event);
extern "C" void OptWinMap(void);
extern "C" void OptWinDoMapValues(void);
extern "C" void OptWinUnmap(void);
extern "C" void OptWinDestroy(void);

/* In optwinop.c */
extern "C" void OptWinFetchGlobals(void);
extern "C" int OptWinApplyChanges(void);
extern "C" int OptWinLoadDefaults(void);
extern "C" int OptWinSaveChanges(void);

extern "C" int OptWinTestSoundPBCB(void *ptr);
extern "C" int OptWinFBCB(char *path);
extern "C" int OptWinBrowseISRefsPBCB(void *ptr);
extern "C" int OptWinBrowseOCSNsPBCB(void *ptr);
extern "C" int OptWinBrowseSSPBCB(void *ptr);
extern "C" int OptWinBrowseJSCalPBCB(void *ptr);
extern "C" int OptWinKeymapPBCB(void *ptr);
extern "C" int OptWinJSMapPBCB(void *ptr);
extern "C" int OptWinRefreshMemoryPBCB(void *ptr);

/* In page.c */
extern "C" int PageIsLabelAllocated(page_struct *p, int n);
extern "C" int PageCreateLabel(page_struct *p);

extern "C" int PageInit(
	page_struct *p,
	win_t w, shared_image_t *image
);
extern "C" void PageResize(
	page_struct *p,
	win_t w, shared_image_t *image
);
extern "C" void PageDrawLabel(
	page_struct *p,
	win_t w, shared_image_t *image,
	int mm_label_num, bool_t put_to_window
);
extern "C" void PageDraw(
        page_struct *p,
        win_t w, shared_image_t *image,
        int amount, bool_t put_to_window
);
extern "C" int PageManage(
        page_struct *p,
        win_t w, shared_image_t *image,
	event_t *event
);
extern "C" void PageMap(
        page_struct *p,   
        win_t w, shared_image_t *image
);
extern "C" void PageUnmap(
        page_struct *p,
        win_t w, shared_image_t *image
);
extern "C" void PageDestroy(
        page_struct *p,
        win_t w, shared_image_t *image
);
extern "C" int PageDoAction(
        page_struct *p,
        win_t w, shared_image_t *image,
	int op_code
);

/* In pagefile.c */
extern "C" int PageLoadFromFile(
        page_struct *p,
        win_t w, shared_image_t *image,
        char *filename
);

/* In qmenu.c */
extern "C" int QMenuInit(void);
extern "C" int QMenuDraw(void);
extern "C" int QMenuManage(event_t *event);
extern "C" int QMenuHandleCB(void *client_data, int op_code);
extern "C" void QMenuMap(void);
extern "C" void QMenuUnmap(void);
extern "C" void QMenuDeleteAllCommands(void);
extern "C" void QMenuDestroy(void);


/* In rcfile.c */
extern "C" int RCLoadFromFile(char *filename);
extern "C" int RCSaveToFile(char *filename);


/* In rengine.c */
extern "C" int REngInit(void);
extern "C" void REngManage(void);
extern "C" void REngShutdown(void);


/* In scanner.c */
extern "C" int ScIsAllocated(int entry_num);

extern "C" int ScAddObjectInContact(xsw_object_struct *obj_ptr);
extern "C" void ScRemoveObjectFromContact(xsw_object_struct *obj_ptr);

extern "C" int ScIsObjectInContact(xsw_object_struct *obj_ptr);

extern "C" void ScDelete(int entry_num);
extern "C" void ScDeleteAll(void);

extern "C" void ScReclaim(void);

extern "C" int ScHandleContacts(int object_num);


/* In serverscript.c */
extern "C" int ServScriptIsAllocated(int n);

extern "C" int ServScriptStart(
	char *filename,
	char **argv,
	int argc
);
extern "C" void ServScriptDelete(int n);
extern "C" void ServScriptDeleteAll(void);
extern "C" void ServScriptReclaim(void);

extern "C" void ServScriptDoSend(serv_script_struct *ss);
extern "C" void ServScriptManage(void);

extern "C" int ServScriptDoMapPrompt(char *filename);


/* In sound.c */
extern "C" int SoundInit(void);
extern "C" int SoundChangeMode(char *arg);
extern "C" int SoundPlay(
	int code,
        double left_volume,
        double right_volume,
	int effects,
	int priority
);
extern "C" int SoundChangeBackgroundMusic(
        int code,
        int effects,
        int priority            /* 0 or 1. */
);
extern "C" int SoundStopBackgroundMusic();
extern "C" int SoundManageEvents(void);
extern "C" void SoundShutdown(void);


/* In ssfile.c */
extern "C" int SSLoadFromFile(char *filename);


/* In timming.c */
extern "C" time_t MilliTime(void);


/* In vsdraw.c */
extern "C" void ScannerDraw(
        int object_num,
        win_t w,
        shared_image_t *image,
        int win_x, int win_y,
        double scan_scale
);
extern "C" void ScannerUpdateLabels(int object_num);

extern "C" void VSDrawViewScreen(
	int camera_obj_num,
        win_t w,
        shared_image_t *image,
	double zoom
);
extern "C" void VSDrawUpdateWeaponLabel(
        image_t **image,
        pixmap_t pixmap
);
extern "C" void VSDrawUpdateNetstatsLabel(
        image_t **image,
        pixmap_t pixmap
);


/* In vsevent.c */
extern "C" int ScannerButtonMatch(event_t *event);
extern "C" int ScannerSBCB(void *ptr);

extern "C" void VSChangePage(page_struct *new_page);
extern "C" int VSButtonMatch(event_t *event);
extern "C" int VSManage(event_t *event);
extern "C" void VSMap(void);
extern "C" void VSUnmap(void);
extern "C" void VSDestroy(void);


/* In vslabels.c */
extern "C" int VSLabelIsAllocated(int n);
extern "C" int VSLabelIsLoaded(int n);
extern "C" int VSLabelGetByPointer(xsw_object_struct *obj_ptr);

extern "C" int VSLabelGetHighest(void);

extern "C" int VSLabelAdd(
        char *text,
        WColorStruct fg_color,
        WColorStruct bg_color,
	font_t *font,
        xsw_object_struct *obj_ptr
);
extern "C" void VSLabelDelete(int n);
extern "C" void VSLabelDeleteByObjectPtr(xsw_object_struct *obj_ptr);
extern "C" void VSLabelDeleteAll(void);
extern "C" void VSLabelReclaimMemory(void);

/* In xswaction.c */
extern "C" void XSWActionCB(void *ptr, void *data, int action);

/* In xswfbcb.c */
extern "C" void XSWMapFB(char *path, int op_code);
extern "C" int XSWFBCBOk(char *path);
extern "C" int XSWFBCBCancel(char *path);

/* In xswproc.c */
extern "C" int XSWDoConnect(const char *url);
extern "C" int XSWDoConnectLast(void);
extern "C" void XSWDoDisconnect(void);
extern "C" void XSWDoRefresh(void);
extern "C" void XSWDoChangeSector(int object_num);
extern "C" void XSWDoHit(
	int src_obj, int tar_obj, int owner_obj,
	double structure_damage, double shield_damage,
	double bearing
);
extern "C" void XSWDoDestroyed(
	int src_obj, int tar_obj,
	int owner_obj, int reason
);



#endif /* XSW_H */
