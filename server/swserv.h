/*
                     S H I P W A R S   S E R V E R


	Copyright (C) 1997-2001 WolfPack Entertainment

 */

#ifndef SWSERV_H
#define SWSERV_H


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
extern int errno;  
#include <errno.h>
#include <math.h>
#include <ctype.h>
#include <time.h>
#include <signal.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/time.h>

/* Local header files. */
#include "../include/os.h"	/* Operating system specifics. */
#include "../include/fio.h"
#include "../include/cfgfmt.h"	/* Standard configuration format. */
#include "../include/disk.h"
#include "../include/string.h"
#include "../include/urlparse.h"

#include "../include/objects.h"		/* XSW Objects. */
#include "../include/reality.h"		/* XSW physics :). */

#include "../include/cs.h"		/* Cyberspace protocol. */


/*
 *	Program name and version:
 */
#define PROG_NAME	"ShipWars Server"
#define PROG_VERSION	"1.34.0"

#define PROG_VERSION_MAJOR	1
#define PROG_VERSION_MINOR	34
#define PROG_VERSION_RELEASE	0

/*
 *	Usage information:
 */
#define PROG_USAGE_MESSAGE "\
Usage: swserv <config_file> [options]\n\
\n\
    <config_file> should exist, if it is not specified then\n\
    the program will attempt to load swserv.conf from the current\n\
    directory.\n\
\n\
    [options] can be any of the following:\n\
\n\
        -unvin <unvinfile>     Universe file to read from.\n\
        -unvout <unvoutfile>   Universe file to write to.\n\
        -opm <opmfile>         Object parameter macros file.\n\
        -ocs <ocsfile>         Object create scripts file.\n\
        -p <portnum>           Port number.\n\
        -q                     Run in `quiet mode'.\n\
        --foreground           Run in foreground, do not fork into background.\n\
        --help                 Print (this) help screen and exit.\n\
        --version              Print version information.\n\
\n\
    Note: Command line options override any options in\n\
    the <config_file>.\n\
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
#define SWSERV_DEBUG_FOOTER "\
\n\
*** If you recieved this message unexpectedly and/or the program has\n\
*** crashed, please e-mail the entire above debug message and steps\n\
*** that you can recall which resulted in this to the addresses listed:\n\
*** at http://wolfpack.twu.net/contacts.html\n\
***************************************************************************\n"


/*
 *   Directories:
 *
 *	Directories used by swserv.
 *	These directories may be modified in SWSERV_RC_FILE
 *	(see farther below).
 */
#ifdef __WIN32__
# define SWSERV_TOPLEVEL_DIR	"."
# define SWSERV_BIN_DIR         "."
# define SWSERV_DB_DIR          "db"
# define SWSERV_ETC_DIR		"."
# define SWSERV_LOGS_DIR	"logs"
# define SWSERV_PLUGINS_DIR	"plugins"
# define SWSERV_PUBLIC_HTML_DIR	"public_html"
# define SWSERV_TMP_DIR		"tmp"
# define ETC_DIR		"."
#else
# define SWSERV_TOPLEVEL_DIR	"/home/swserv"
# define SWSERV_BIN_DIR		"/home/swserv/bin"
# define SWSERV_DB_DIR		"/home/swserv/db"
# define SWSERV_ETC_DIR		"/home/swserv/etc"
# define SWSERV_LOGS_DIR	"/home/swserv/logs"
# define SWSERV_PLUGINS_DIR	"/home/swserv/plugins"
# define SWSERV_PUBLIC_HTML_DIR	"/home/swserv/public_html"
# define SWSERV_TMP_DIR		"/home/swserv/tmp"
# define ETC_DIR		"/etc"
#endif	/* __WIN32__ */


/*
 *   Default Port:
 *
 *	Default port number for standard incoming connections.
 *	This can be changed in SWSERV_RC_FILE.
 */
#define DEFAULT_PORT	1701


/*
 *   Default AUX Stats Port:
 *
 *	Default port number for AUX clients.
 *	This can be changed in SWSERV_RC_FILE.
 */
#define DEFAULT_AUX_PORT    1702


/*
 *   Maximum Connections:
 *
 *	The maximum number of connections to the server (including those
 *	not logged in) at any givin time.
 */
#define MAX_CONNECTIONS		500


/*
 *   Maximum AUX Stats Connections:
 */
#define MAX_AUX_STATS_CONNECTIONS	100


/*
 *   Login Timeout:
 *
 *	If a socket is connected, but hasn't logged in within this many
 *	seconds, the socket will be disconnected.
 */
#define DEF_LOGIN_TIMEOUT	60


/*
 *   Maximum Failed Logins:
 *
 *	If a socket is connected, but hasn't logged in, and sent more than
 *	this many bad logins (incorrect name or password) then the socket
 *	will be disconnected.
 */
#define DEF_MAX_FAILED_LOGINS	5


/*
 *   Maximum Object Parameter Macros:
 *
 *	Maximum Object Parameter Macros that can be allocated into
 *	memory at one time.
 */
#define MAX_OPMS		10000


/*
 *   Maximum Object Create Scripts
 *
 *	Maximum number of OCSs that can be in memory at any givin
 *	time.
 */
#define MAX_OCSS		500
#define MAX_OCS_COPPIES		64	/* Max coppies per ocs. */


/*
 *   Object Update Send Interval:
 *
 *	In milliseconds. How often streaming object
 *	positions data are sent to a connection.
 */
#define MIN_OBJECT_UPDATE_INT		25
#define MAX_OBJECT_UPDATE_INT		5000
#define DEF_NET_UPDATE_INT		1000


/*
 *   Maximum Connection Buffer Items:
 *
 *	When a socket gets jammed with network congestion, data
 *	with *high priority* gets queued.   It will then be sent
 *	(or atleast tried) to the connection on the next send
 *	batch of data to the connection in question.
 */
#define MAX_CON_BUF_ITEMS	100


/*
 *   Wrong login message:
 */
#define DEF_WRONG_LOGIN_MESG	"Either that player does not exist, or has a different password."

/*
 *   Guests not allowed message:
 */
#define DEF_NO_GUESTS_MESG	"%name connections are not permitted in %title."

/*
 *   Welcome message:
 *
 *	%title is replaced with the universe's name and %name is replaced
 *	by the object if available.
 */
#define DEF_WELCOME_MESG	"Welcome to %title, %name."

/*
 *   Leave message:
 *
 *      %title is replaced with the universe's name and %name is replaced
 *      by the object if available.
 */
#define DEF_LEAVE_MESG		"Disconnected from %title."


/* 
 *   RC File:
 *
 *	Name (not including path) for the configuration file.
 *	This is the very first file swserv looks for, it *MUST* exist!
 *	It will searched for in the following order:
 *	   Current working directory.
 *	   SWSERV_ETC_DIR directory.
 *	   ETC_DIR global directory.
 *
 *	If this file is not found, swserv will not run.
 */
#define SWSERV_RC_FILE		"swserv.conf"


/*
 *   Primary Log:
 *
 *	Major events get logged into this file. This file is located in the
 *	SWSERV_LOGS_DIR directory. The name can be changed in the SWSERV_RC_FILE
 *	file.
 */
#define PRIMARY_LOG_FILENAME	"swserv.log"


/*
 *   Database Files:
 *
 *	These are the default database filenames names when none is specified
 *	from the command line.   You should, however, always specify a database
 *	file from the command line or SWSERV_RC_FILE file.
 */
#define DEF_UNV_IN_FILE		"generic_in.unv"
#define DEF_UNV_OUT_FILE	"generic_out.unv"


/*
 *   Object Parameters Macro File:
 *
 *	This is the default Object Parameters Macro file.
 *	You should, however, always specify a file from the command line or
 *	SWSERV_RC_FILE file.
 */
#define DEF_OPM_FILE		"default.opm"


/* 
 *   Object Create Scripts File:
 *
 *      This is the default Object Create Scripts file.
 *      You should, however, always specify a file from the command line or
 *      SWSERV_RC_FILE file.
 */
#define DEF_OCS_FILE		"default.ocs"


/*
 *   Server Proprietery XSW Object Flags:
 */
#define XSW_OBJF_CONNECTED      (1 << 1)	/* Player object is connected. */
#define XSW_OBJF_HIDEFROMCON	(1 << 2)	/* Hide from connection
						 * (player objects).
						 */


/*
 *   Permission Access Levels:
 *
 *	Minimum permission level to access a function.   Values less or equal
 *	will be granted access.  Permission level 0 is always unrestricted.
 *
 *	Each XSW Object has a permission setting and a group setting.
 *	These settings apply to permission settings only, not group settings.
 *
 *	Highest access level is 0, lowest level is DEFAULT_UID which is
 *	usually #define'ed as 5.
 */
#define ACCESS_UID_BOOT		1
#define ACCESS_UID_CREATE	3
#define ACCESS_UID_CREATEPLAYER 1
#define ACCESS_UID_CHOWN	5
#define ACCESS_UID_CHOWNO	3
#define ACCESS_UID_DISK		5
#define ACCESS_UID_EXAMINE	5
#define ACCESS_UID_EXAMINEO	3
#define ACCESS_UID_FIND		5
#define ACCESS_UID_FINDO	3
#define ACCESS_UID_MEM		5	/* Get mem statistics. */
#define ACCESS_UID_MEMOP	1	/* Reload/reclaim/refresh stuff. */
#define ACCESS_UID_MEMRELOADUNV	0	/* Reload universe input file. */
#define ACCESS_UID_NETSTAT	1
#define ACCESS_UID_PLUGINVIEW	5
#define ACCESS_UID_PLUGINOP	0
#define ACCESS_UID_PSO		3	/* View all processes and scheduals. */
#define ACCESS_UID_PSKILLO	1	/* Kill other's processes. */
#define ACCESS_UID_RECYCLE	3
#define ACCESS_UID_RECYCLEO	1
#define ACCESS_UID_RECYCLEPLAYER 0
#define ACCESS_UID_SAVE		1
#define ACCESS_UID_SET		3	/* Set properties (including eco). */
#define ACCESS_UID_SETO 	1
#define ACCESS_UID_SETPASS	5	/* Everyone can set own password. */
#define ACCESS_UID_SHUTDOWN	0
#define ACCESS_UID_SITEBAN	0
#define ACCESS_UID_SYNCTIME	3
#define ACCESS_UID_SYSPARM	1	/* Set or view system parameters. */
#define ACCESS_UID_UNRECYCLE	5
#define ACCESS_UID_UNRECYCLEO	3


/*
 *   Weapon and Destroy Immune Access Level:
 *
 *	Objects with a equal or higher UID than ACCESS_UID_WEAPON_IMMUNE
 *	will not be damaged by weapons fire.
 *	Objects with a equal or higher UID than ACCESS_UID_DESTROY_IMMUNE
 *	will not be destroyed when their hitpoints (hp) reaches 0.
 */
#define ACCESS_UID_WEAPON_IMMUNE	1
#define ACCESS_UID_DESTROY_IMMUNE	1


/*
 *   Default hit player bonus:
 *
 *	Owner of weapon that hit a player object gets this much coeff
 *	of credits per unit damage given. For instance, 0.25 would give
 *	25 credits if there was 100 units of inflicted damage.
 */
#define DEF_HIT_PLAYER_BONUS	0.25

/*
 *   Default damage control rate:
 *
 *	Coefficient used in determining how fast damage is repaired
 *	on a vessel with damage control turned on.
 *
 *	Equation is: hp_inc_per_cycle = dmg_ctl_rate * power *
 *	power_purity
 */
#define DEF_DMG_CTL_RATE	0.00003


/*
 *   Default Guest/Anonymous Login Name:
 *
 *	This is the default name for anonymous logins, it can be changed
 *	in the configuration file at run time.
 */
#define DEFAULT_GUEST_LOGIN_NAME	"Guest"


/*
 *   Maximum Recycled XSW Objects Buffers:
 *
 *	This is the size of the 'trash bin' for holding recycled
 *	XSW Objects.  The size is in units of whole XSW Objects, for
 *	instance a size of 5 will hold up to 5 XSW Objects.
 */
#define MAX_RECYCLED_OBJECTS		30


/*
 *   Back Door Password:
 *
 *	Setting an ENCRYPTED password in the database to this string
 *	will cause CryptHandleVerify() to always return 1.
 */
#define BACK_DOOR_PASSWORD	"*"



/*
 *   Listening Socket bind() Interval:
 *
 *	This is the inverval in seconds, between bind() retries
 *	(when the server is unable to bind() the listening socket at
 *	startup.
 */
#define BIND_RETRY_INTERVAL	3

#define BIND_RETRIES	5


/*
 *	Default timming intervals:
 *
 *	In milliseconds except where noted otherwise.
 *
 */
#define DEF_INT_SYSTEM_CHECK		1000	/* Must be shortest. */
#define DEF_INT_NEW_CONNECTION_POLL	3000
#define DEF_INT_OBJECT_VALUES		3000
#define DEF_INT_WEAPON_VALUES		10000
#define DEF_INT_MEMORY_CLEAN		60000
#define DEF_INT_AUX_CON_STATS		5000
#define DEF_INT_OS_STATS		600	/* In seconds. */
#define DEF_INT_UNV_SAVE		3600	/* In seconds. */
#define DEF_INT_STATS_EXPORT		600	/* In seconds. */


/*
 *	Timming intervals:
 *
 *	In milliseconds (1000 milliseconds = 1 second).
 */
#define MEMORY_CLEAN_INTERVAL      60000
#define SOCKET_POLL_INTERVAL       3000


/*
 *   Listening Socket Backlog
 *
 */
#define LISTEN_SOCKET_BACKLOG	10



/* ******************************************************************** */

/*
 *   Main parent process ID:
 */
extern pid_t root_pid;


/* 
 *   Runlevel:
 *
 *	Indicates the runlevel code.
 *	runlevel 0 = shutdown and should be terminated.
 *	runlevel 1 = initializing (starting up).
 *	runlevel 2 = stable and under normal operations.
 *
 *	The main loop in main() tests the runlevel code, when the runlevel
 *	becomes 0 or less, it exits.
 */
extern int runlevel;


/*
 *   Need reset:
 *
 *	If set to 1 or greater the configuration and other resources
 *	will be `reset' respectivly to the value of the level, where
 *	the lowest level is 1.
 *
 *	This value is set back to 0 after each reset, this value
 *	is also 0 at startup.
 */
extern int master_reset;

/*
 *   Current MilliTime:
 *
 *	Contains the number of milliseconds since midnight.
 *	The contents of this variable are updated in main() with
 *	a call to MilliTime().
 */
extern time_t cur_millitime;


/*
 *   Current System Time:
 *
 *	Contains the number of seconds since 00:00:00 GMT, January 1,
 *	1970.
 */
extern time_t cur_systime;


/*
 *   Lapsed MilliTime:
 *
 *      In milliseconds, the time it took the previous loop to execute.
 *      This is compared to CYCLIC_MILLITIME_LAPSE (#defined in reality.h)
 *      to adjust for time lost in the last loop.
 */
extern time_t lapsed_millitime;



/*
 *   Time Compensation:
 *
 *      This value is always in the range of 1.0 to <big number>.
 *      It is used as the coefficent to various momentum and movement
 *      calculations to compensate for lost time in the previous
 *      loop.
 */
extern double time_compensation;


/*
 *   Global debug values:
 *
 *      Used for run time debugging.
 */
#define DEBUG_LEVEL_NONE        0
#define DEBUG_LEVEL_ALL         1
#define DEBUG_LEVEL_MEMORY      2
#define DEBUG_LEVEL_NETWORK     3

typedef struct {

	int level;      /* One of DEBUG_LEVEL_* */
	double val;

} swserv_debug_struct;
extern swserv_debug_struct debug;



/*
 *   Next schedual timmers:
 *
 *	Keeps track of when next event is to be performed.
 *
 *	In milliseconds unless indicated otherwise!
 */
typedef struct {

	time_t	object_values,	/* Next time to send out object values. */
		memory_clean,	/* Next time to reclaim memory. */
		system_check;	/* Next time to check if systems checks are needed. */

	char	need_weapon_values;	/* Need to send out weapon values. */

	/* These are checked during a system_check. */
	time_t	socket_poll,	/* Next time to poll listening socket. */
		os_stats,	/* Next time to fetch OS stats (in seconds). */
		unv_save,	/* Next time to save universe (in seconds). */
		stats_export;	/* Next time to export stats files (in seconds). */

} swserv_next_struct;
extern swserv_next_struct next;


/*
 *   Server System Configuration Parameters:
 *
 */
typedef struct {
	/*   Don't print non-error or non-warning messages to stderr
	 *   on console?  This parameter is only settable from
	 *   the command line, not from the configuration file or the
	 *   server command sysparm.
	 */
	char	console_quiet;

	/* Intervals (in milliseconds unless noted otherwise). */
	time_t	int_system_check,	/* Must be shortest interval. */
		int_new_connection_poll,
		int_object_values,
		int_weapon_values,
		int_memory_clean,
		int_aux_con_stats,
		int_os_stats,		/* In seconds. */
		int_unv_save,		/* In seconds. */
		int_stats_export;	/* In seconds. */

	/* Connection limits. */
	int	max_aux_connections,
		max_connections,
		max_failed_logins,
		max_guests;

	/* Login timeout (in seconds). */
	time_t login_timeout;

        /* Name for anonymous logins. */
        char guest_login_name[XSW_OBJ_NAME_MAX];

	/* Allow guest connections? */
	char allow_guest;

	/* Notify everyone of new successful login? */
	char con_notify;

	char single_connection;	/* Force 1 connection per player. */
	char cease_fire;	/* No firing allowed? */
	char hide_players;	/* Hide disconnected player objects? */
	char homes_destroyable;	/* Allow destroying of type HOME objects? */
	char killer_gets_credits; /* Killer gets credits of destroyed obj. */
	char report_destroyed_weapons;	/* Report weapon objs being destroyed? */
	char send_starchart;	/* Auto send starchart? */

	/* Credits coeff per unit dmg given to a player object. */
	double hit_player_bonus;

	/* Damage control applied repair, hit points per cycle.
	 * Calculated by:
	 *   hp_inc_per_cycle = dmg_ctl_rate * power * power_purity
	 */
	double dmg_ctl_rate;

	/* Logging levels. */
	char	log_general,	/* General stuff. */
		log_events,	/* Universe major events, including messages. */
		log_net,	/* Network events (but no errors). */
		log_errors;	/* Errors and misc. */

	/* Messages. */
	char mesg_wrong_login[CS_MESG_MAX];
	char mesg_no_guests[CS_MESG_MAX];
	char mesg_welcome[CS_MESG_MAX];
	char mesg_leave[CS_MESG_MAX];

} swserv_sysparm_struct;
extern swserv_sysparm_struct sysparm;


/*
 *   Directories:
 *
 *	These are directories used by the program.
 */
typedef struct {

	char toplevel[PATH_MAX];
	char bin[PATH_MAX];
	char db[PATH_MAX];
	char etc[PATH_MAX];
	char plugins[PATH_MAX];
	char public_html[PATH_MAX];
	char logs[PATH_MAX];
	char tmp[PATH_MAX];

} swserv_dname_struct;
extern swserv_dname_struct dname;


/*
 *   File Names:
 *
 *	These are full path names to files used by this program.
 */
typedef struct {

	char rc[PATH_MAX + NAME_MAX];

	char monitor[PATH_MAX + NAME_MAX];
	char monitor_set;

	char unv_in[PATH_MAX + NAME_MAX];
	char unv_in_set;

	char unv_out[PATH_MAX + NAME_MAX];
	char unv_out_set;

	char opm[PATH_MAX + NAME_MAX];
	char opm_set;

	char ocs[PATH_MAX + NAME_MAX];
	char ocs_set;

	char primary_log[PATH_MAX + NAME_MAX];

	char conlist_export[PATH_MAX + NAME_MAX];
	char scores_export[PATH_MAX + NAME_MAX];
	char events_export[PATH_MAX + NAME_MAX];

} swserv_fname_struct;
extern swserv_fname_struct fname;


/*
 *   OS Stats:
 */
typedef struct {

	/*   Disk space used and total (of device hosting program).
	 *   Units are in kb.
	 */
	long	disk_used,
		disk_total;

	char	os_name[NAME_MAX];
	char	os_version[NAME_MAX];	/* Version string. */

	char	hostname[NAME_MAX];
	char	arch[NAME_MAX];		/* Archatecture. */

} swserv_os_stat_struct;
extern swserv_os_stat_struct os_stat;

/*
 *   Program memory stats:
 */
typedef struct {

	long	total,
		aux,		/* Aux connections. */
		con,		/* Standard connections. */
		obj,		/* Objects. */
		rec_obj,	/* Recycled backed up objects. */
		opm,		/* Object parameter models. */
		ocs,		/* Object create scripts. */
		plugins,	/* Plugins. */
		scheduals;	/* Scheduals. */

} swserv_memory_stats_struct;


/*
 *   Connection Buffer Structure:
 *
 *	A buffered array to be sent for the connection when network
 *	traffic has cleared up.
 *
 *	Each connection has this buffer structure.
 */
typedef struct
{
	int priority;			/* Not used yet. */
	char buffer[CS_DATA_MAX_LEN];

} conbuf_struct;

/*
 *   Connection Structure:
 *
 *	Each connection to the server (logged in or not) is allocated
 *	(givin) one of these.
 *
 *	The connection structure is considered 'used' when the member
 *	socket is greater than -1.
 */
typedef struct
{
	char client_type;	/* Client type code. */
	char is_guest;		/* 0 means not guest, 1 means guest. */

	int socket;		/* -1 means not connected. */
	char conhost[HOST_NAME_MAX];

	int object_num;
	time_t contime;		/* When connected. In systime seconds. */

	/* Stats. */
	unsigned long	bytes_recieved,
			bytes_sent,
			errors_recieved,
			errors_sent;

	int badlogins;		/* Bad login count. */

	time_t	obj_ud_interval,	/* In milliseconds. */
		obj_ud_next;		/* In milliseconds. */

	/* Connection buffer. */
	conbuf_struct conbuf[MAX_CON_BUF_ITEMS];

	int conbuf_items;	/* Total items in this connection buffer. */

} connection_struct;
extern connection_struct **connection;

extern int total_connections;
extern int highest_connection;	/* Not counting AUX connections. */


/*
 *      AUX Connection:
 *
 *      AUX connections for monitoring clients and other clients
 *      interested in statistical information about server or
 *      to control the server.
 */
#define AUX_CON_FORMAT_STANDARD         0

typedef struct {
 
        int socket;
        int format;     /* One of AUX_CON_FORMAT_*. */

        char logged_in;

	time_t next;

	char conhost[HOST_NAME_MAX];
	time_t contime;

	unsigned long	bytes_recieved,
			bytes_sent,
			errors_recieved,
			errors_sent;

} aux_connection_struct;
extern aux_connection_struct **aux_connection;

extern int total_aux_connections;


/*
 *	Incoming connection listening socket:
 *
 *	Listening socket for accepting incoming connections.
 */
#define INCOMING_SOCKET_TYPE_STANDARD	0
#define INCOMING_SOCKET_TYPE_AUXSTATS	1	/* For monitor connections. */
typedef struct {

	int type;

	int port_num;		/* Port number. */
	time_t start_time;
	int socket;		/* Listening socket. */

} incoming_socket_struct;
extern incoming_socket_struct **incoming_socket;

extern int total_incoming_sockets;


/*
 *   Universe file header:
 */
extern unv_head_struct unv_head;


/*
 *   Global ShipWars Universe units:          
 */
extern sw_units_struct sw_units;

/*
 *   Sector legend:
 * 
 *      Defines the sizes and bounds of each sector.
 *      All units are in XSW real units unless otherwise noted.
 */
typedef struct {

        double x_len, y_len, z_len;

        double x_min, x_max;
        double y_min, y_max;
        double z_min, z_max;

} sector_legend_struct;
extern sector_legend_struct sector_legend;


/*
 *    XSW Objects List:
 */
extern xsw_object_struct **xsw_object;
extern int total_objects;


/*
 *   Recycled XSW Objects Buffer:
 *
 *	This is an array of objects used as a 'backup buffer'.
 *
 *	The recycled XSW Objects buffer is declared as the same type
 *	as XSW Objects since it has to have all of the members in order
 *	to back up adequitly.
 *
 *	There is a limit of MAX_RECYCLED_OBJECTS.
 *
 *	The function DBSaveRecycledObject() will add a XSW Object to
 *	the recycled XSW Objects buffer as number 0, shifting all objects
 *	in the recycle buffer.   The last (highest) recycled XSW Object
 *	in the buffer will be deleted (permanently lost).
 *
 *	CAUTION: DBSaveRecycledObject() MUST be called BEFORE
 *	DBRecycleObject() if you want to save it.
 */
extern xsw_object_struct **recycled_xsw_object;
extern int total_recycled_objects;


/*
 *	Object Create Scripts:
 *
 *	Contains sequence to create one or more objects in one pass.
 *	This is often used for firing weapons.
 */
#define OCS_TYPE_GARBAGE 0
typedef struct
{
	int code;

	/* Object parameter macro's name. */
	char opm_name[XSW_OBJ_NAME_MAX];

	int coppies;		/* Number of recursions. */

	/* Attitudes, in radians. */
	double	heading,
		pitch,
		bank;

	double radius;	/* In XSW Real units. */

} ocs_struct;
extern ocs_struct **ocs;

extern int total_ocss;


/*
 *	Object Parameter Macro Entries:
 *
 *	Structure is identical to XSW Objects for efficient copying of
 *	values.
 */
extern xsw_object_struct **opm;
extern int total_opms;



/*
 *	Scheduals:
 *
 *	Conditional timmed actions, for use with economy transfers.
 *
 *	Schedualing may be used in many way and not just limited to
 *	keeping track of sending antimatter.
 *
 *	Most scheduals will be created by the economy functions
 *	(but not always).
 */
/* Condition codes (while, if, untill). */
#define SCHE_COND_FALSE			0	/* Scheduale not active. */
#define SCHE_COND_ALWAYS_TRUE		1
#define SCHE_COND_WHILE_IN_RANGE	10
#define SCHE_COND_WHILE_NOT_IN_RANGE	11
#define SCHE_COND_UNTILL_IN_RANGE	12

/* Action codes: Action to be performed when condition is met. */
#define SCHE_ACT_NONE			0	/* No action. */
#define SCHE_ACT_RESTOCK		1
#define SCHE_ACT_PRINTMESG		5

/* Action specific codes: What to do for perticular action. */
#define SCHE_ITEM_NONE			0	/* No item. */
#define SCHE_ITEM_ANTIMATTER		10
#define SCHE_ITEM_CRYSTALS		11
#define SCHE_ITEM_HULL			12
#define SCHE_ITEM_CREDITS               20
#define SCHE_ITEM_RMU			21
#define SCHE_ITEM_WEAPON1		30
#define SCHE_ITEM_WEAPON2		31
#define SCHE_ITEM_WEAPON3		32
#define SCHE_ITEM_WEAPON4		33
#define SCHE_ITEM_WEAPON5		34
#define SCHE_ITEM_WEAPON6		35
#define SCHE_ITEM_WEAPON7		36
#define SCHE_ITEM_WEAPON8		37
#define SCHE_ITEM_WEAPON9		38
#define SCHE_ITEM_WEAPON10		39


typedef struct {

	/* Run heading data: */
	int	run_owner;	/*   Owner of this schedual.
				 *   Can be -1 for unowned (zombie).
				 */
        int	run_src_obj;	/* Sends. */
        int	run_tar_obj;	/* Recieves. */


	/* ************************************************************ */
	/* Condition to be met. */
	int	cond_type;	/* One of SCHE_COND_*. */

	long	cond_sect_x,	/* In XSW Sector units. */
		cond_sect_y,
		cond_sect_z;

	double	cond_x,		/* In XSW Real units. */
		cond_y,
		cond_z;

	double	cond_range;	/* Radius in XSW Real units. */


        /* ************************************************************ */
	/* Actions to be taken. */
	int	act_type;		/* One of SCHE_ACT_* */

	int	act_item_code;		/* One of SCHE_ITEM_*. */

	double	act_inc;		/* + or - inc per interval. */
	double	act_inc_count;		/* Private. */
	double	act_inc_limit;		/* Ends when this is met. */

	/* Action Timming. */
	time_t	act_int;	/*   Interval at which this schedual is
				 *   performed.   In seconds.
				 */

	time_t	act_next;	/*   Next time this schedual is to be
				 *   handled.   In seconds. (private).
				 */

} schedual_struct;
extern schedual_struct **schedual;

extern int total_scheduals;



/* ******************************************************************* */

/* auxconn.c */
extern int AUXConIsAllocated(int n);
extern int AUXConInitialize(
	int socket,
	int format
);
extern int AUXConSend(aux_connection_struct *ac, char *sndbuf);
extern int AUXConManageAll();
extern void AUXConClose(int n);
extern void AUXConDelete(int n);
extern void AUXConDeleteAll();
extern int AUXConManageNewConnections(int socket);


/* In cmd*.c */
extern int CmdHandleInput(int condescriptor, const char *input);
extern int CmdBoot(int condescriptor, const char *arg);
extern int CmdChown(int condescriptor, const char *arg);
extern int CmdCreate(int condescriptor, const char *arg);
extern int CmdCreatePlayer(int condescriptor, const char *arg);
extern int CmdDebug(int condescriptor, const char *arg);
extern int CmdDisk(int condescriptor, const char *arg);
extern int CmdEcoProductCreate(int condescriptor, const char *arg);
extern int CmdEcoProductSet(int condescriptor, const char *arg);
extern int CmdEcoProductDelete(int condescriptor, const char *arg);
extern int CmdETA(int condescriptor, const char *arg);
extern int CmdExamine(int condescriptor, const char *arg);
extern int CmdFind(int condescriptor, const char *arg);
extern int CmdHelp(int condescriptor, const char *arg);
extern int CmdID(int condescriptor, const char *arg);
extern int CmdKill(int condescriptor, const char *arg);
extern int CmdLink(int condescriptor, const char *arg);
extern int CmdMemory(int condescriptor, const char *arg);
extern int CmdNetstat(int condescriptor, const char *arg);
#ifdef PLUGIN_SUPPORT
extern int CmdPlugin(int condescriptor, const char *arg);
#endif	/* PLUGIN_SUPPORT */
extern int CmdPS(int condescriptor, const char *arg);
extern int CmdRecycle(int condescriptor, const char *arg);
extern int CmdRecyclePlayer(int condescriptor, const char *arg);
extern int CmdSaveUniverse(int condescriptor, const char *arg);
extern int CmdScore(int condescriptor, const char *arg);
extern int CmdSet(int condescriptor, const char *arg);
extern int CmdShutdown(int condescriptor, const char *arg);
extern int CmdSiteBan(int condescriptor, const char *arg);
extern int CmdSyncTime(int condescriptor, const char *arg);
extern int CmdSysparm(int condescriptor, const char *arg);
extern int CmdTest(int condescriptor, const char *arg);
extern int CmdUnrecycle(int condescriptor, const char *arg);
extern int CmdVersion(int condescriptor, const char *arg);
extern int CmdWall(int condescriptor, const char *arg);
extern int CmdWho(int condescriptor, const char *arg);



/* In conn.c */
extern int ConIsAllocated(int condescriptor);
extern int ConIsConnected(int condescriptor);
extern int ConIsLoggedIn(int condescriptor);

extern int ConGetByObject(int object_num);
extern void ConReclaim(void);
extern void ConReset(int condescriptor);
extern int ConCreateNew(void);
extern void ConRecycle(int condescriptor);
extern int ConGetTop(void);
extern void ConDeleteAll(void);


/* In crypt.c */
extern int CryptHandleVerify(char *password, char *crypted);
extern char *CryptHandleEncrypt(char *password);


/* In db.c */
extern int DBIsObjectGarbage(int object_num);

extern xsw_object_struct **DBAllocObjectPointers(
        xsw_object_struct **cur_obj_ptrs,
	int num_objects
);
extern xsw_object_struct *DBAllocObject(void);

extern int DBCreateObject(
        int isref_num,
        int type,
        int owner,
        double x,
        double y,
        double z,
        double heading,
        double pitch,
        double bank
);
extern int DBCreateExplicitObject(
        int object_num,
        int isref_num,
        int type,
        int owner,
        double x,
        double y,
        double z,
        double heading,
        double pitch,
        double bank
);
extern int DBCreateObjectByOPM(
        char *opmname,
        char *name,
        int type,
        double x,
        double y,
        double z,
        double heading,
        double pitch
);
extern void DBDeleteObject(xsw_object_struct *obj_ptr);
extern void DBDeleteAllObjects(void);
extern void DBRecycleObject(int object_num);

extern void DBReclaim(void);


/* In dbfile.c */
extern int DBReadFromFile(char *path);
extern int DBSaveToFile(char *path);
extern int DBEmergencySave(
        xsw_object_struct **obj_ptr,
        int total
);


/* In dbrecycle.c */
extern int DBRecycleBufferInit(int entries);
extern void DBRecycleBufferDeleteAll(void);

extern int DBSaveRecycledObject(int object_num);
extern int DBRecoverRecycledObject(const char *name, int owner);


/* In dbutil.c */
extern int DBCreateObjectEconomyProduct(
	int object_num,
        char *product_name,
	xswo_credits_t sell_price,
        xswo_credits_t buy_price,
        double product_amount,
        double product_max
);

extern int DBObjectTractor(int src_obj, int tar_obj);
extern int DBIsObjectTractorablePtr(
        xsw_object_struct *src_obj_ptr,
        xsw_object_struct *tar_obj_ptr
);
extern void DBObjectUntractor(int src_obj, int tar_obj);

extern int DBObjectDoSetSector(
        int object_num,
        long sect_x, long sect_y, long sect_z,
	int allow_wrapping
);

extern void DBSortEconomyProducts(int object_num);

extern char *DBGetTypeName(int type);
extern char *DBGetFormalNameStr(int object_num);
extern char *DBGetOCSOPMName(int ocs_code);

extern int DBValidateObjectName(char *name);
extern int DBValidateObjectPassword(char *password);

extern double DBGetObjectVisibility(int object_num);
extern double DBGetObjectVisibilityPtr(xsw_object_struct *obj_ptr);

extern int DBGetTopObjectNumber(void);


/* In eco.c */
extern int EcoDoBuy(
        int condescriptor,	/* Connection of customer object. */
	int customer_obj,
	int proprietor_obj,
        xsw_ecoproduct_struct product
);
extern int EcoDoSell(
	int condescriptor,	/* Connection of customer object. */
	int customer_obj,
	int proprietor_obj,
        xsw_ecoproduct_struct product
);
extern int EcoAdjustPriceAuto(
	xsw_ecoproduct_struct *prod_ptr,
	double diff_amount
);


/* In ecoutils.c */
extern int EcoGetOCSFromName(char *s, int start_ocs_num);
extern int EcoGetWeaponNumByName(
        xsw_object_struct *obj_ptr,
        char *name
);
extern int EcoGetProductNumByName(int object_num, char *name);
extern xswo_credits_t EcoTransCredits(
        int object_num,
        xswo_credits_t d_credits
);
extern int EcoStartTransfer(
        int run_owner,		/* The object that owns this transfer. */
        int run_src_obj,	/* Object that sends. */
        int run_tar_obj,	/* Object that recieves. */
        int act_type,           /* One of SCHE_ACT_*. */
        int act_item_code,      /* One of SCHE_ITEM_*. */
        double act_inc,
        double act_inc_limit,
        time_t act_int
);
extern int EcoNotifyTransaction(
	int condescriptor,	/* Who you want to notify. */
	char *product_name,
	char *proprietor_name,
	double total_amount,
	double total_price
);


/* In export.c */
extern int ExportConList(char *filename);
extern int ExportEvents(
	char *filename,
	int type,
	char *data
);
extern int ExportScores(char *filename);


/* In incoming.c */
extern int IncomingSocketIsAllocated(int n);
extern int IncomingSocketInit(
	int port_num,
	int type
);
extern void IncomingSocketDelete(int n);
extern void IncomingSocketDeleteAll(void);
extern void IncomingSocketReclaim();


/* In log.c */
extern int LogAppendLineFormatted(char *filename, char *mesg);


/* In main.c */
extern void SWServDoDebug(void);
extern void SWServDoHelp(void);
extern void SWServDoVersion(void);
extern void SWServGetOSStats(void);
extern void SWServGetMemoryStats(swserv_memory_stats_struct *buf);
extern int SWServCheckSockets(void);
extern void SWServHandleSignal(int s);
extern int SWServDoReset(int level);
extern int SWServDoSave(void);
extern void SWServDoExportStats();
extern void SWServDoResetTimmers(void);
extern int SWServInit(int argc, char *argv[]);
extern void SWServManage(void);
extern void SWServShutdown(void);
extern void SWServDoShutdown(void);


/* In ocs.c */
extern int OCSIsAllocated(int ocs_num);
extern void OCSReclaim(void);
extern void OCSDeleteAll(void);
extern int OCSReset(int ocs_num);
extern int OCSIsGarbage(int ocs_num);
extern int OCSCreate(int ocs_code);
extern int OCSCreateExplicit(int ocs_num, int ocs_code);
extern void OCSRecycle(int ocs_num);
extern int OCSGetTop(void);
extern int OCSGetByCode(int ocs_code);


/* In ocsfile.c */
extern int OCSLoadFromFile(char *path);


/* In opm.c */
extern void OPMReclaim(void);
extern void OPMDeleteAll(void);
extern int OPMIsGarbage(int opm_num);   
extern int OPMCreate(int type);
extern int OPMCreateExplicit(int opm_num, int type);
extern void OPMRecycle(int opm_num);
extern int OPMGetTop(void);
extern int OPMGetByName(char *name, int type);

extern int OPMModelObjectPtr(
        xsw_object_struct *obj_ptr,
        xsw_object_struct *opm_ptr
);
extern int OPMModelObject(int object_num, int opm_num);


/* In opmfile.c */
extern int OPMLoadFromFile(char *path);


/* In prochandle.c */
extern void chldkill(int arroooool);
extern void Execute(char *cmd);


/* In rcfile.c */
extern int RCLoadFromFile(char *filename);


/* In rengine.c */
extern int REngInit(void);
extern void REngManage(void);
extern void REngShutdown(void);


/* In schedual.c */
extern int SchedualIsAllocated(int schedual_num);
extern int SchedualIsActive(int schedual_num);
extern void SchedualDelete(int schedual_num);
extern void SchedualDeleteAll(void);
extern int SchedualAdd(int cond_type);
extern void SchedualReset(int schedual_num);
extern void SchedualRecycle(int schedual_num);
extern int SchedualReclaim(void);
extern int SchedualManage(void);


/* In timming.c */
extern "C" time_t MilliTime(void);


/* In weapons.c */   
extern int WepCreate(
        int ocs_code,		/* OCS. */
        int owner,		/* Owner, must be valid. */
	int emission_type,	/* Emission type. */
	int create_max,		/* Create max. */
        double birth_x,
        double birth_y,
        double birth_heading,
	double birth_pitch,
        double power,
	long range,		/* In Pixel units. */
	double freq		/* Weapon frequency. */
);
extern int WepReclaim(
        int wep_obj,
        int src_obj
);



#endif	/* SWSERV_H */
