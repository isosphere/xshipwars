/*
                           Program Primary Routines

	Functions:

	void SWServDoDebug()
	void SWServDoHelp()
	void SWServDoVersion()
	void SWServGetOSStats()
	void SWServGetMemoryStats(swserv_memory_stats_struct *buf)
	int SWServCheckSockets()
	void SWServHandleSignal(int s)
	int SWServDoReset(int level)
	int SWServDoSave()
	void SWServDoExportStats()
 	void SWServDoResetTimmers()

	int SWServInit(int argc, char *argv[])
	void SWServManage()
        void SWServShutdown()
        void SWServDoShutdown()

	int main(int argc, char *argv[])

	---

 */

#include <sys/types.h>
#include <sys/time.h>

#ifdef __WIN32__ 
#include <windows.h>
#endif

#ifndef _USE_BSD
# define _USE_BSD
#endif
#include <sys/resource.h>
#include <sys/wait.h>

#include "../include/prochandle.h"
#include "../include/df.h"   
#include "../include/netio.h"
#include "../include/isrefs.h"
#include "../include/unvmain.h"
#include "../include/unvmatch.h"

#include "swserv.h"
#include "net.h"
#include "siteban.h"

#ifdef PLUGIN_SUPPORT
# include "plugins.h"
#endif	/* PLUGIN_SUPPORT */


/* Signal definations for FreeBSD. */
#ifndef SIGPOLL
# define SIGPOLL	SIGUSR1
#endif
#ifndef SIGPWR
# define SIGPWR		SIGUSR2
#endif




pid_t root_pid;

int runlevel;
int master_reset;

time_t cur_millitime;
time_t cur_systime;
time_t lapsed_millitime;
double time_compensation;

swserv_debug_struct debug;
swserv_next_struct next;
swserv_sysparm_struct sysparm;
swserv_dname_struct dname;
swserv_fname_struct fname;
swserv_os_stat_struct os_stat;

connection_struct **connection;
int total_connections;
int highest_connection;

aux_connection_struct **aux_connection;  
int total_aux_connections;

incoming_socket_struct **incoming_socket;
int total_incoming_sockets;

unv_head_struct unv_head;
sw_units_struct sw_units;
sector_legend_struct sector_legend;

xsw_object_struct **xsw_object;
int total_objects;

xsw_object_struct **recycled_xsw_object;
int total_recycled_objects;

ocs_struct **ocs;
int total_ocss;

xsw_object_struct **opm;
int total_opms;

schedual_struct **schedual;
int total_scheduals;



/*
 *	Segfault counter.
 */
static int segfault_count;


/*
 *	Procedure to print debug information.
 */
void SWServDoDebug()
{
	/* Heading. */
        fprintf(stderr,
 "%s %s debug report:\n",
            PROG_NAME, PROG_VERSION
        );

        fprintf(stderr,
 "runlevel: %i  total_connections (allocated): %i  highest_connection: %i\n",
            runlevel, total_connections, highest_connection
        );

        fprintf(stderr,
 "total_objects (allocated): %i\n",
            total_objects
        );

	/* Footer. */
	fprintf(stderr, SWSERV_DEBUG_FOOTER);


	return;
}


/*
 *	Procedure to print help information to stdout
 *	(not to a connection).
 */
void SWServDoHelp()
{
	fprintf(stdout, PROG_USAGE_MESSAGE);

	return;
}


/*
 *	Procedure to print version of server to stdout
 *	(not to a connection).
 */
void SWServDoVersion()
{
	printf(
	    "%s Version %s\n%s\n",
	    PROG_NAME,
	    PROG_VERSION,
	    PROG_COPYRIGHT
	);
}


/*
 *	Procedure to update OS stats.
 */
void SWServGetOSStats()
{
#ifdef __WIN32__
	char CurrDir[257];
	int SectorsPerCluster, BytesPerSector, FreeClusters, TotalClusters;

	if(GetCurrentDirectory(sizeof(CurrDir),CurrDir))
	{
	    CurrDir[3] = '\0'; /* only need the drive */
            if(GetDiskFreeSpace(CurrDir,
                                (PDWORD)&SectorsPerCluster,
                                (PDWORD)&BytesPerSector,
                                (PDWORD)&FreeClusters,
                                (PDWORD)&TotalClusters
	       )
	    )
	    {
                os_stat.disk_used = (BytesPerSector *
                    SectorsPerCluster) * FreeClusters;

                os_stat.disk_total = (BytesPerSector *
		    SectorsPerCluster) * TotalClusters;

		return;
            }
	}

        os_stat.disk_used = 0;
        os_stat.disk_total = 0;

#else
	int i;
	int total_df_stats;
	df_stat_struct **df_stat;


	/* Get disk space stats. */
	df_stat = DiskFreeGetListing(&total_df_stats);
	if((df_stat != NULL) && (total_df_stats > 0))
	{
	    /*   Look for device stat index for the home
	     *   directory of this program.  Must do search in
	     *   reverse order to catch higher mounted paths first.
	     */
	    for(i = total_df_stats - 1; i >= 0; i--)
	    {
		if(df_stat[i] == NULL)
		    continue;

		if(strpfx(
		    dname.toplevel,
		    df_stat[i]->mounted_on
		))
		    break;
	    }
	    if(i < total_df_stats)
	    {
		os_stat.disk_used = df_stat[i]->used;
                os_stat.disk_total = df_stat[i]->total;
	    }
	    else
	    {
                os_stat.disk_used = 0;
                os_stat.disk_total = 0;
	    }
	}

	/* Free listing. */
	DiskFreeDeleteListing(df_stat, total_df_stats);


	return;
#endif /* __WIN32__ */
}


/*
 *	Procedure to get statistics of all memory used by the
 *	program.
 */
void SWServGetMemoryStats(swserv_memory_stats_struct *buf)
{
	int i, n;
	xsw_object_struct *obj_ptr;
	xsw_weapons_struct *wep_ptr;
	xsw_ecodata_struct *eco_ptr;
#ifdef PLUGIN_SUPPORT
	plugin_record_struct *plugin_ptr;
#endif	/* PLUGIN_SUPPORT */


	if(buf == NULL)
	    return;


	/* Reset total. */
	buf->total = 0;


	/* AUX pipes. */
	buf->aux = (total_aux_connections * sizeof(aux_connection_struct *)) +
	    (total_aux_connections * sizeof(aux_connection_struct));
	buf->total += buf->aux;


	/* Connections. */
	buf->con = (total_connections * sizeof(connection_struct *)) +
            (total_connections * sizeof(connection_struct));
	buf->total += buf->con;


	/* Objects. */
	buf->obj = (total_objects * sizeof(xsw_object_struct *)) +
            (total_objects * sizeof(xsw_object_struct));

	for(i = 0; i < total_objects; i++)
	{
	    obj_ptr = xsw_object[i];
	    if(obj_ptr == NULL)
		    continue;

	    /* Elink. */
	    if(obj_ptr->elink != NULL)
		buf->obj += (strlen(obj_ptr->elink) + 1) * sizeof(char);

	    /* Tractored objects. */
	    buf->obj += obj_ptr->total_tractored_objects * sizeof(int);

	    /* Weapons. */
	    for(n = 0; n < obj_ptr->total_weapons; n++)
	    {
		wep_ptr = obj_ptr->weapons[n];
		if(wep_ptr == NULL)
		    continue;

		buf->obj += sizeof(xsw_weapons_struct);
	    }
	    buf->obj += obj_ptr->total_weapons *
		sizeof(xsw_weapons_struct *);

	    /* Score. */
	    if(obj_ptr->score != NULL)
		buf->obj += sizeof(xsw_score_struct);

	    /* Economy. */
	    eco_ptr = obj_ptr->eco;
	    if(eco_ptr != NULL)
	    {
		buf->obj += sizeof(xsw_ecodata_struct);

		for(n = 0; n < eco_ptr->total_products; n++)
		{
		    if(eco_ptr->product[n] == NULL)
			continue;

		    buf->obj += sizeof(xsw_ecoproduct_struct);
		}
		buf->obj += eco_ptr->total_products *
		    sizeof(xsw_ecoproduct_struct *);
	    }
	}

	buf->total += buf->obj;


	/* Recycle buffers (struct same as xsw_object_struct). */
	buf->rec_obj = (total_recycled_objects * sizeof(xsw_object_struct *))
	    + (total_recycled_objects * sizeof(xsw_object_struct));

	for(i = 0; i < total_recycled_objects; i++)
	{
	    obj_ptr = recycled_xsw_object[i];
	    if(obj_ptr == NULL)
		continue;

            /* Elink. */
            if(obj_ptr->elink != NULL)
                buf->rec_obj += (strlen(obj_ptr->elink) + 1) * sizeof(char);

            /* Tractored objects. */
            buf->rec_obj += obj_ptr->total_tractored_objects * sizeof(int);

            /* Weapons. */
            for(n = 0; n < obj_ptr->total_weapons; n++)
            {
                wep_ptr = obj_ptr->weapons[n];
                if(wep_ptr == NULL)
                    continue;
        
                buf->rec_obj += sizeof(xsw_weapons_struct);
            }
            buf->rec_obj += obj_ptr->total_weapons *
                sizeof(xsw_weapons_struct *);

            /* Score. */
            if(obj_ptr->score != NULL)
                buf->rec_obj += sizeof(xsw_score_struct);

            /* Economy. */
            eco_ptr = obj_ptr->eco;
            if(eco_ptr != NULL)
            {
                buf->rec_obj += sizeof(xsw_ecodata_struct);

                for(n = 0; n < eco_ptr->total_products; n++) 
		{
                    if(eco_ptr->product[n] == NULL)
                        continue;

                    buf->rec_obj += sizeof(xsw_ecoproduct_struct);
                }
                buf->rec_obj += eco_ptr->total_products *
                    sizeof(xsw_ecoproduct_struct *);
            }
	}
	buf->total += buf->rec_obj;


        /* Object parameter models. */
        buf->opm = (total_opms * sizeof(xsw_object_struct *)) +
	    (total_opms * sizeof(xsw_object_struct));

        for(i = 0; i < total_opms; i++)
	{
	    obj_ptr = opm[i];

            /* Elink. */
            if(obj_ptr->elink != NULL)
                buf->opm += (strlen(obj_ptr->elink) + 1) * sizeof(char);

            /* Tractored objects. */
            buf->opm += obj_ptr->total_tractored_objects * sizeof(int);

            /* Weapons. */
            for(n = 0; n < obj_ptr->total_weapons; n++)
            {
                wep_ptr = obj_ptr->weapons[n];
                if(wep_ptr == NULL)
                    continue;

                buf->opm += sizeof(xsw_weapons_struct);
            }
            buf->opm += obj_ptr->total_weapons *
                sizeof(xsw_weapons_struct *);

            /* Score. */
            if(obj_ptr->score != NULL)
                buf->opm += sizeof(xsw_score_struct);

            /* Economy. */
            eco_ptr = obj_ptr->eco;
            if(eco_ptr != NULL)
            {
                buf->opm += sizeof(xsw_ecodata_struct);

                for(n = 0; n < eco_ptr->total_products; n++)
                {
                    if(eco_ptr->product[n] == NULL)
                        continue;

                    buf->opm += sizeof(xsw_ecoproduct_struct);
                }
                buf->opm += eco_ptr->total_products *
                    sizeof(xsw_ecoproduct_struct *);
            }
        }
        buf->total += buf->opm;


        /* Object create scripts. */
        buf->ocs = (total_ocss * sizeof(ocs_struct *)) +
            (total_ocss * sizeof(ocs_struct));

	buf->total += buf->ocs;


	/* Plugins. */
#ifdef PLUGIN_SUPPORT
	buf->plugins = (total_plugins * sizeof(plugin_data_ref_struct *)) +
	    (total_plugins * sizeof(plugin_data_ref_struct));

	for(i = 0; i < total_plugins; i++)
	{
	    plugin_ptr = plugin[i];
	    if(plugin_ptr == NULL)
		continue;

	    if(plugin_ptr->name != NULL)
		buf->plugins += (strlen(plugin_ptr->name) + 1) * sizeof(char);

	    for(n = 0; n < plugin_ptr->argc; n++)
	    {
		if(plugin_ptr->argv[n] != NULL)
		    buf->plugins += (strlen(plugin_ptr->argv[n]) + 1) * sizeof(char);
	    }
	    buf->plugins += plugin_ptr->argc * sizeof(char *);
	}
#else	/* PLUGIN_SUPPORT */
	buf->plugins = 0;
#endif	/* PLUGIN_SUPPORT */

        buf->total += buf->plugins;


        /* Scheduals. */
	buf->scheduals = (total_scheduals * sizeof(schedual_struct *)) +
	    (total_scheduals * sizeof(schedual_struct));

	buf->total += buf->scheduals;


	return;
}


/*
 *	Procedure to check all program's opened sockets and
 *	close any that are invalid.
 */
int SWServCheckSockets()
{
	int i;
	connection_struct **con_ptr;

	int bad_sockets = 0;




	/* Go through connections. */
	for(i = 0, con_ptr = connection;
            i < total_connections;
	    i++, con_ptr++
	)
	{
	    if(*con_ptr == NULL)
		continue;

	    if((*con_ptr)->socket < 0)
		continue;

/* This isn't the right way.
	    if(!NetIsSocketWritable((*con_ptr)->socket))
	    {
		close((*con_ptr)->socket);
		(*con_ptr)->socket = -1;

		NetCloseConnection(i);
	    }
*/
	}



	return(bad_sockets);
}


/*
 *	OS Signal handler.
 */
void SWServHandleSignal(int s)
{
	char stringa[1024];
        int status;


	switch(s)
	{
          /* **************************************************** */
          case SIGHUP:
	    /* Set master reset to 2 so that the configuration
	     * will be reloaded, the universe database will NOT
	     * be reloaded.
	     */
	    master_reset = 2;
	    break;

	  /* **************************************************** */
	  case SIGINT:
	    /* Do debug procedure. */
            SWServDoDebug();

            /* Log signal incident. */
	    if(sysparm.log_general || sysparm.log_errors)
                LogAppendLineFormatted(
		    fname.primary_log,
                    "*** Got SIGINT: Interupt signal."
		);

	    /* Close all connections. */
/* Need to notify all connections. */
	    if(sysparm.log_net)
		LogAppendLineFormatted(
		    fname.primary_log,
                    "*** Closing all connections."
		);
	    ConDeleteAll();

            /* Save universe. */
	    SWServDoSave();

            runlevel = 1;
	    break;

          /* **************************************************** */
          case SIGTERM:
            /* Log signal incident. */
	    if(sysparm.log_general || sysparm.log_errors)
                LogAppendLineFormatted(
		    fname.primary_log,
                    "*** Got SIGTERM: Terminate signal."
		);

            /* Close all connections. */
/* Need to notify all connections. */
            if(sysparm.log_net)
                LogAppendLineFormatted(
		    fname.primary_log,
                    "*** Closing all connections."
		);
            ConDeleteAll();

            /* Save universe. */
            SWServDoSave();

            runlevel = 1;
            break;

          /* **************************************************** */  
	  case SIGPIPE:
            /* Watch for SIGPIPE again. */
            signal(SIGPIPE, SWServHandleSignal);

            /* Log signal incident. */
            if(sysparm.log_errors)
                LogAppendLineFormatted(
		    fname.primary_log,
                    "*** Got SIGPIPE: Broken pipe (or socket)."
		);

	    /* Check if any sockets died. */
	    SWServCheckSockets();

	    break;

          /* **************************************************** */
	  case SIGSEGV:
	    /* Increment segfault counter. */
	    segfault_count++;
	    if(segfault_count >= 2)
		exit(-1);

            /* Do debug procedure. */ 
            SWServDoDebug();

	    /* Log signal incident. */
            if(sysparm.log_errors || sysparm.log_general)
		LogAppendLineFormatted(
		    fname.primary_log,
                    "*** Got SIGSEGV: Segmentation fault."
		);

	    /* Go directly for an emergency universe save. */
	    DBEmergencySave(xsw_object, total_objects);

            runlevel = 1;
            break;

          /* **************************************************** */
	  case SIGCHLD:
	    /* Catch child. */
            while(wait3(&status, WNOHANG, (struct rusage *)0) > 0);

	    /* Watch for signal again. */
	    signal(SIGCHLD, SWServHandleSignal);
	    break;

          /* **************************************************** */
	  default:
            /* Log signal incident. */
	    sprintf(stringa,
		"*** Got signal %i: Unhandled signal.",
		s
	    );
            if(sysparm.log_errors)
                LogAppendLineFormatted(fname.primary_log, stringa);

            /* Emergency DB save. */
            DBEmergencySave(xsw_object, total_objects);

            /* Need to notify all connections. */
            break;
	}


	return;
}


/*
 *	Procedure to handle a global `reset'.
 */
int SWServDoReset(int level)
{
	int i;
	char text[256];


	/* Nothing to reset? */
	if(level <= 0)
	    return(0);


        /* Log reset. */
        sprintf(text,
            "Server: Level %i reset in progress...",
	    level
        );
        if(sysparm.log_general)
            LogAppendLineFormatted(fname.primary_log, text);


	/* Level 3 or greater. */
	if(level >= 3)
	{
            /* Reload universe database from universe on file. */

	    /* Notify connections that they need to relogin. */
            NetSendLiveMessage(
		-1,
 "Universe has been reloaded, you will need to relogin."
            );
            /* Disconnect all connections. */
            for(i = 0; i < total_connections; i++)
                NetCloseConnection(i);

	    /* Delete old universe and load new universe. */
            DBReadFromFile(fname.unv_in);

            /* Delete all connections. */
            ConDeleteAll();

	    /* Do not reload program configuration. */
	}

	/* Level 2 or greater. */
	if(level >= 2)
	{
	    /* Reload configurations (except universe database). */

            NetSendLiveMessage(
                -1,
                "Server: Memory reload in progress..."
            );

	    /* Program configuration. */
            RCLoadFromFile(fname.rc);

	    /* Object parameter models. */
            OPMLoadFromFile(fname.opm);

	    /* Object create scripts. */
            OCSLoadFromFile(fname.ocs);

            NetSendLiveMessage(
                -1,
                "Server: Memory reload complete."
            );
	}

	/* Exactly level 1. */
	if(level == 1)
	{
	    /* Refresh memory, note that there is no reason
	     * to refresh memory if the level was higher than 1
	     * since the memory would have been just loaded.
	     */

            NetSendLiveMessage(
                -1,
                "Server: Memory refresh in progress..."
            );

	    /* Universe database. */
            DBReclaim();

	    /* Object parameter models. */
            OPMReclaim();

	    /* Object create scripts. */
            OCSReclaim();

	    /* Scheduals. */
            SchedualReclaim();

	    /* Connections. */
            ConReclaim();

            NetSendLiveMessage(
                -1,
                "Server: Memory refresh complete."
            );
	}

        /* Log reset. */
        sprintf(text,
            "Server: Reset complete."
        );
        if(sysparm.log_general)
            LogAppendLineFormatted(fname.primary_log, text);


	return(0);
}

/*
 *	Procedure to save universe database, warn connections about it
 *	and log save.
 */
int SWServDoSave()
{
	char sndbuf[CS_DATA_MAX_LEN];
	char text[512];
	int status;


	/* Notify all connections about save. */
	sprintf(sndbuf,
	    "Server: Universe save in progress..."
        );
        NetSendLiveMessage(-1, sndbuf);


        /* Log save. */
        sprintf(text,
	    "Server: Starting universe save..."
	);
        if(sysparm.log_general)
            LogAppendLineFormatted(fname.primary_log, text);


        /* Save universe. */
        status = DBSaveToFile(fname.unv_out);
        /* Check for errors and print finish message. */
        if(status)
            sprintf(text,
                "Server: Universe save error code `%i'.",
                status
            );
        else
            sprintf(text,
		"Server: Universe save complete."
	    );
        NetSendLiveMessage(-1, text);
        if(sysparm.log_general)
            LogAppendLineFormatted(fname.primary_log, text);

	/* Attempt emergancy save incase of error. */
	if(status)
	    DBEmergencySave(xsw_object, total_objects);


	return(status);
}

/*
 *	Procedure to export server's statistics.
 */
void SWServDoExportStats()
{
	ExportScores(fname.scores_export);
	ExportConList(fname.conlist_export);

	return;
}

/*
 *	Procedure to reset all global timmers.
 */
void SWServDoResetTimmers()
{
	int i, n;
	aux_connection_struct **ac_ptr;
	connection_struct **con_ptr;
	xsw_object_struct **obj_ptr;
	schedual_struct **schedual_ptr;


        /* Next instances. */
	memset(&next, 0x00, sizeof(swserv_next_struct));

        next.object_values = 0;
        next.memory_clean = 0; 
        next.system_check = 0;
	next.need_weapon_values = 0;

	next.socket_poll = 0;
	next.os_stats = time(NULL);		/* In seconds. */
        next.unv_save = time(NULL);		/* In seconds. */
        next.stats_export = time(NULL);		/* In seconds. */


	/* Connections. */
        for(i = 0, con_ptr = connection;
            i < total_connections;
            i++, con_ptr++
	)
        {
	    if(*con_ptr == NULL)
		continue;

            (*con_ptr)->obj_ud_next = 0;
        }

        /* AUX Connections. */
        for(i = 0, ac_ptr = aux_connection;
            i < total_aux_connections;
            i++, ac_ptr++
        )
        {
            if(*ac_ptr == NULL)
                continue;

            (*ac_ptr)->next = 0;
        }

	/* Go through XSW objects list. */
        for(i = 0, obj_ptr = xsw_object;
            i < total_objects;
            i++, obj_ptr++
	)
        {
	    if(*obj_ptr == NULL)
		continue;

            if((*obj_ptr)->type <= XSW_OBJ_TYPE_GARBAGE)
                continue;
             
            (*obj_ptr)->last_updated = 0;
	    (*obj_ptr)->birth_time = 0;
	    (*obj_ptr)->animation.last_interval = 0;

            for(n = 0; n < (*obj_ptr)->total_weapons; n++)
            {
                (*obj_ptr)->weapons[n]->last_used = 0;
            }
        }

	/* Schedual timmings. */
	for(i = 0, schedual_ptr = schedual;
            i < total_scheduals;
            i++, schedual_ptr++
	)
	{
	    if(*schedual_ptr == NULL)
		continue;

	    (*schedual_ptr)->act_next = 0;
	}


	return;
}


/*
 *	Procedure to initialize program.
 */
int SWServInit(int argc, char *argv[])
{
	int i, n, status;
	char *strptr;
	char stringa[1024];
	char cmd[1024 + PATH_MAX + NAME_MAX];
	struct stat stat_buf;


	/* First thing to do on initialization is to reset the
	 * segfault counter.
	 */
	segfault_count = 0;


	/* Get process to run in background by forking. */
	for(i = 1, status = 1; i < argc; i++)
	{
	    if(argv[i] == NULL)
		continue;

	    if(!strcasecmp(argv[i], "--foreground") ||
               !strcasecmp(argv[i], "--fg") ||
               !strcasecmp(argv[i], "-foreground") ||
               !strcasecmp(argv[i], "-fg") ||
               strcasepfx(argv[i],"--h") ||
               strcasepfx(argv[i],"-h") ||
               strcasepfx(argv[i],"-?") ||
               strcasepfx(argv[i],"/?") ||
               strcasepfx(argv[i], "--ver") ||
               strcasepfx(argv[i], "-ver")
	    )
	    {
		status = 0;
		break;
	    }
	}
	if(status)
	{
	    switch(fork())
	    {
	      case -1:
		perror("fork");
		return(-1);
		break;

	      case 0:	/* Child. */
		break;

	      default:	/* Parent. */
		exit(0);
		break;
	    }
	}


        /* Initialize time zone. */
        tzset();

	root_pid = getpid();

	/* Reset global variables. */
	master_reset = 0;

	cur_millitime = MilliTime();
	cur_systime = time(NULL);

	debug.level = DEBUG_LEVEL_NONE;
	debug.val = 0;

	lapsed_millitime = 0;
	time_compensation = 1.0;

	sysparm.console_quiet = 0;

	sysparm.int_system_check = DEF_INT_SYSTEM_CHECK;
	sysparm.int_new_connection_poll = DEF_INT_NEW_CONNECTION_POLL;
        sysparm.int_object_values = DEF_INT_OBJECT_VALUES;
	sysparm.int_weapon_values = DEF_INT_WEAPON_VALUES;
        sysparm.int_memory_clean = DEF_INT_MEMORY_CLEAN;
        sysparm.int_aux_con_stats = DEF_INT_AUX_CON_STATS;
        sysparm.int_os_stats = DEF_INT_OS_STATS;	/* In seconds. */
        sysparm.int_unv_save = DEF_INT_UNV_SAVE;	/* In seconds. */
        sysparm.int_stats_export = DEF_INT_STATS_EXPORT;	/* In seconds. */

        sysparm.max_connections = MAX_CONNECTIONS;
        sysparm.max_guests = MAX_CONNECTIONS;
	sysparm.max_aux_connections = MAX_AUX_STATS_CONNECTIONS;

        sysparm.login_timeout = DEF_LOGIN_TIMEOUT;
        sysparm.max_failed_logins = DEF_MAX_FAILED_LOGINS;

        strncpy(
            sysparm.guest_login_name,
            DEFAULT_GUEST_LOGIN_NAME,
            XSW_OBJ_NAME_MAX
        );
        sysparm.guest_login_name[XSW_OBJ_NAME_MAX - 1] = '\0';

        sysparm.allow_guest = 1;        /* Allow guests default. */
	sysparm.con_notify = 1;
	sysparm.single_connection = 1;
	sysparm.cease_fire = 0;
	sysparm.hide_players = 1;
	sysparm.homes_destroyable = 0;
	sysparm.killer_gets_credits = 1;
	sysparm.report_destroyed_weapons = 0;
	sysparm.hit_player_bonus = DEF_HIT_PLAYER_BONUS;
	sysparm.dmg_ctl_rate = DEF_DMG_CTL_RATE;

        sysparm.log_general = 1;
        sysparm.log_events = 1;
        sysparm.log_net = 1;
        sysparm.log_errors = 1;

	strncpy(
	    sysparm.mesg_wrong_login,
	    DEF_WRONG_LOGIN_MESG,
	    CS_MESG_MAX
	);
	sysparm.mesg_wrong_login[CS_MESG_MAX - 1] = '\0';

	strncpy(
            sysparm.mesg_no_guests,
            DEF_NO_GUESTS_MESG,
            CS_MESG_MAX
        );
	sysparm.mesg_no_guests[CS_MESG_MAX - 1] = '\0';

        strncpy(
            sysparm.mesg_welcome,
            DEF_WELCOME_MESG,
            CS_MESG_MAX
        );
        sysparm.mesg_welcome[CS_MESG_MAX - 1] = '\0';

        strncpy(
            sysparm.mesg_leave,
            DEF_LEAVE_MESG,
            CS_MESG_MAX
        );
        sysparm.mesg_leave[CS_MESG_MAX - 1] = '\0';


	sw_units.ru_to_au = DEF_UNITCONV_RU_TO_AU;

        sector_legend.x_max = SECTOR_SIZE_X_MAX;
        sector_legend.x_min = SECTOR_SIZE_X_MIN;
        sector_legend.y_max = SECTOR_SIZE_Y_MAX;
        sector_legend.y_min = SECTOR_SIZE_Y_MIN;
        sector_legend.z_max = SECTOR_SIZE_Z_MAX;
        sector_legend.z_min = SECTOR_SIZE_Z_MIN;
        sector_legend.x_len = sector_legend.x_max - sector_legend.x_min;
        sector_legend.y_len = sector_legend.y_max - sector_legend.y_min;
        sector_legend.z_len = sector_legend.z_max - sector_legend.z_min;

        next.object_values = cur_millitime;
        next.memory_clean = cur_millitime;
        next.system_check = cur_millitime;
	next.need_weapon_values = 0;

        next.socket_poll = cur_millitime;	/* Asap. */
        next.os_stats = cur_systime;		/* Asap. */
        next.unv_save = cur_systime + sysparm.int_unv_save;
        next.stats_export = cur_systime;	/* Asap. */

	xsw_object = NULL;
	total_objects = 0;

	opm = NULL;
	total_opms = 0;

	ocs = NULL;
	total_ocss = 0;

	recycled_xsw_object = NULL;
	total_recycled_objects = 0;

	connection = NULL;
	total_connections = 0;
	highest_connection = 0;

	incoming_socket = NULL;
	total_incoming_sockets = 0;

	siteban = NULL;
	total_sitebans = 0;

	strncpy(dname.toplevel, SWSERV_TOPLEVEL_DIR, PATH_MAX);
	strncpy(dname.bin, SWSERV_BIN_DIR, PATH_MAX);
	strncpy(dname.etc, SWSERV_ETC_DIR, PATH_MAX);
        strncpy(dname.db, SWSERV_DB_DIR, PATH_MAX);  
        strncpy(dname.logs, SWSERV_LOGS_DIR, PATH_MAX);
	strncpy(dname.plugins, SWSERV_PLUGINS_DIR, PATH_MAX);
	strncpy(dname.public_html, SWSERV_PUBLIC_HTML_DIR, PATH_MAX);
        strncpy(dname.tmp, SWSERV_TMP_DIR, PATH_MAX);

	strncpy(fname.monitor, "monitor", PATH_MAX + NAME_MAX);
	fname.monitor_set = 0;
	strncpy(fname.unv_in, DEF_UNV_IN_FILE, PATH_MAX + NAME_MAX);
	fname.unv_in_set = 0;
	strncpy(fname.unv_out, DEF_UNV_OUT_FILE, PATH_MAX + NAME_MAX);
	fname.unv_out_set = 0;
        strncpy(fname.opm, DEF_OPM_FILE, PATH_MAX + NAME_MAX);
        fname.opm_set = 0;
	strncpy(fname.ocs, DEF_OCS_FILE, PATH_MAX + NAME_MAX);
	fname.ocs_set = 0;

	strptr = PrefixPaths(SWSERV_LOGS_DIR, PRIMARY_LOG_FILENAME);
	strncpy(
	    fname.primary_log,
	    ((strptr == NULL) ? "." : strptr),
	    PATH_MAX + NAME_MAX
	);

	*fname.conlist_export = '\0';
        *fname.scores_export = '\0';
        *fname.events_export = '\0';

#ifdef __WIN32__
        strcpy(stringa, ".");
#else
	getcwd(stringa, sizeof(stringa));
#endif /* __WIN32_ */
	strptr = PrefixPaths(stringa, SWSERV_RC_FILE);
	strncpy(
	    fname.rc,
	    ((strptr == NULL) ? SWSERV_RC_FILE : strptr),
	    PATH_MAX + NAME_MAX
        );


	total_aux_connections = 0;
	aux_connection = NULL;

	strncpy(unv_head.title, "Untitled", UNV_TITLE_MAX);

        /* Initialize universe object management resources. */
        UNVInit(argc, argv);


	/* ********************************************************** */
	/* Parse arguments. */
        for(i = 1; i < argc; i++)
        {
	    if(strcasepfx(argv[i],"--h") ||
	       strcasepfx(argv[i],"-h") ||
               strcasepfx(argv[i],"-?") ||
               strcasepfx(argv[i],"/?")
	    )
	    {
                SWServDoHelp();
		return(-4);
            }
            else if(strcasepfx(argv[i], "--ver") ||
                    strcasepfx(argv[i], "-ver")
	    )
            {
                SWServDoVersion();
                return(-4);
            }
            else if(strcasepfx(argv[i], "--mon") ||
                    strcasepfx(argv[i], "-mon")
	    )
            {
                i++;
		if(i < argc)
		{
                    strncpy(fname.monitor, argv[i], NAME_MAX);
                    fname.monitor[NAME_MAX - 1] = '\0';
                    fname.monitor_set = 1;
		}
		else
		{
		    fprintf(stderr, "%s: Requires argument.\n",
			argv[i - 1]
		    );
		    return(-1);
		}
            }
            else if(strcasepfx(argv[i], "--unvin") ||
                    strcasepfx(argv[i], "-unvin") ||
                    strcasepfx(argv[i], "--dbin") ||
                    strcasepfx(argv[i], "-dbin")
	    )
            {
                i++;
                if(i < argc)
                {
		    strncpy(fname.unv_in, argv[i], NAME_MAX);
		    fname.unv_in[NAME_MAX - 1] = '\0';
		    fname.unv_in_set = 1;
                }
                else
                {
                    fprintf(stderr, "%s: Requires argument.\n",
                        argv[i - 1]
                    );
                    return(-1);
                }
            }
            else if(strcasepfx(argv[i], "--unvout") ||
                    strcasepfx(argv[i], "-unvout") ||
                    strcasepfx(argv[i], "--dbout") ||
                    strcasepfx(argv[i], "-dbout")
	    )
            {
                i++;
                if(i < argc)
                {
                    strncpy(fname.unv_out, argv[i], NAME_MAX);
		    fname.unv_out[NAME_MAX - 1] = '\0';
		    fname.unv_out_set = 1;
                }
                else
                {   
                    fprintf(stderr, "%s: Requires argument.\n",
                        argv[i - 1]
                    );
                    return(-1);
                }
            }
	    /* Object parameter values. */
            else if(strpfx(argv[i], "--opm") ||
                    strpfx(argv[i], "-opm")
	    )
            {
                i++;
                if(i < argc)
                {   
                    strncpy(fname.opm, argv[i], NAME_MAX);
		    fname.opm[NAME_MAX - 1] = '\0';
                    fname.opm_set = 1;
                }
                else
                {
                    fprintf(stderr, "%s: Requires argument.\n",
                        argv[i - 1]
                    );
                    return(-1);
                }
            }
	    /* Object create scripts. */
            else if(strpfx(argv[i], "--ocs") ||
                    strpfx(argv[i], "-ocs")  
            )
            {
                i++;
                if(i < argc)
                {
                    strncpy(fname.ocs, argv[i], NAME_MAX);
                    fname.ocs[NAME_MAX - 1] = '\0';
                    fname.ocs_set = 1;
                }
                else
                {
                    fprintf(stderr, "%s: Requires argument.\n",
                        argv[i - 1]
                    );
                    return(-1);
                }
            }
	    /* Port. */
            else if(strpfx(argv[i], "--p") ||
                    strpfx(argv[i], "-p")
            )
            {
                i++;
                if(i < argc)
                {
		    status = IncomingSocketInit(
			atoi(argv[i]),
			INCOMING_SOCKET_TYPE_STANDARD
		    );
		    if(status)
			return(-1);
                }
                else
                {
                    fprintf(stderr, "%s: Requires argument.\n",
                        argv[i - 1]
                    );
                    return(-1);
                }
            }
	    /* Quiet. */
            else if(strpfx(argv[i], "--q") ||
                    strpfx(argv[i], "-q")
            )
	    {
		sysparm.console_quiet = 1;
	    }
	    /* Foreground (just check, don't handle since it was
	     * handled earlier.
	     */
            else if(!strcasecmp(argv[i], "--foreground") ||
                    !strcasecmp(argv[i], "--fg") ||
                    !strcasecmp(argv[i], "-foreground") ||
                    !strcasecmp(argv[i], "-fg")
            )
	    {

	    }
            /* Check if argument is prefixed with '-' or '+'. */
            else if((argv[i][0] == '-') ||
                    (argv[i][0] == '+')
            )
            {
                fprintf(
		    stderr,
                    "%s: Unsupported argument.\n",
                    argv[i]
                );
            }
	    /* Sole argument is configuration file to use. */
	    else
	    {
		strncpy(fname.rc, argv[i], NAME_MAX);
		fname.rc[NAME_MAX - 1] = '\0';
	    }
	}


	/* Check for any errors in arguments. */



	/* ******************************************************* */
	/* Print start up messages. */
	if(!sysparm.console_quiet)
	{
	    printf(
		"%s %s\n",
	        PROG_NAME,
		PROG_VERSION
	    );
/* Bit too spammy for startup header
	    printf(
		"%s\n",
		PROG_COPYRIGHT
	    );
 */
	}


	/* *********************************************************** */
	/* Load restart configuration from file. */

	status = -1;

	/* Current or specified directory? */
	if(!stat(fname.rc, &stat_buf))
	{
	    status = RCLoadFromFile(fname.rc);
	}
	else
	{
#ifndef __WIN32__	/* only check "etc" on unix */
	    /* Is in the etc dir? */
            strptr = PrefixPaths(dname.etc, SWSERV_RC_FILE);
            strncpy(
                fname.rc,
                ((strptr == NULL) ? SWSERV_RC_FILE : strptr),
                PATH_MAX + NAME_MAX
            );
            fname.rc[PATH_MAX + NAME_MAX - 1] = '\0';

	    if(!stat(fname.rc, &stat_buf))
	    {
		status = RCLoadFromFile(fname.rc);
	    }
	    else
	    {
		/* Is in the global etc directory? */
		strptr = PrefixPaths(ETC_DIR, SWSERV_RC_FILE);
		strncpy(
		    fname.rc,
		    ((strptr == NULL) ? SWSERV_RC_FILE : strptr),
		    PATH_MAX + NAME_MAX
		);
		fname.rc[PATH_MAX + NAME_MAX - 1] = '\0';

                if(!stat(fname.rc, &stat_buf))
		{
                    status = RCLoadFromFile(fname.rc);
		}
	    }
#endif /* not __WIN32__ */
	}
	/* Error loading the configuration file? */
	if(status < 0)
	{
	    fprintf(
		stderr,
		"%s: Error loading configuration file.\n",
		fname.rc
	    );
	    return(-1);
	}


        /* ********************************************************* */
	/* Open a standard incoming connections listening socket to
         * the default port as needed.
	 */
	for(i = 0, n = 0; i < total_incoming_sockets; i++)
	{
	    if(incoming_socket[i] == NULL)
		continue;

	    /* Is this a `standard' one? */
	    if(incoming_socket[i]->type == INCOMING_SOCKET_TYPE_STANDARD)
		n++;
	}
	if(n <= 0)
	{
	    /* No standard incoming listening sockets exist,
	     * so create one.
	     */
	    status = IncomingSocketInit(
		DEFAULT_PORT,
		INCOMING_SOCKET_TYPE_STANDARD
	    );
	    if(status)
	    {
                fprintf(
                    stderr,
   "Error: Unable to establish listening socket on port %i.\n",
                    DEFAULT_PORT
                );
	        return(-1);
	    }
	}


	/* ********************************************************* */
	/* Check if directories are valid. */

	/* Server toplevel/home directory. */
	if(!ISPATHDIR(dname.toplevel))
	{
	    fprintf(stderr,
                "%s: Error: Invalid directory.\n",
		dname.toplevel
	    );
	    return(-1);
	}

	/* Server etc directory. */
        if(!ISPATHDIR(dname.etc))
        {
            fprintf(stderr,
                "%s: Warning: Invalid directory.\n",
                dname.etc
            );
            /* Non-fatal. */
        }

	/* Server logs directory. */
        if(!ISPATHDIR(dname.logs))
        {
            fprintf(stderr,
                "%s: Warning: Invalid directory.\n",
                dname.logs
            );
            /* Non-fatal. */
	}

        /* Server bin directory. */
        if(!ISPATHDIR(dname.bin))
        {
            fprintf(stderr,
                "%s: Warning: Invalid directory.\n",
                dname.bin
            );
            /* Non-fatal. */
        }

        /* Server db directory. */
        if(!ISPATHDIR(dname.db))
        {
            fprintf(stderr,
                "%s: Error: Invalid directory.\n",
                dname.db
            );
            return(-1);
        }


        /* ******************************************************** */
	/* Set signals to watch for. */
        signal(SIGHUP, SWServHandleSignal);
        signal(SIGINT, SWServHandleSignal);
        signal(SIGQUIT, SWServHandleSignal);
        signal(SIGILL, SWServHandleSignal);
        signal(SIGTRAP, SWServHandleSignal);
        signal(SIGABRT, SWServHandleSignal);
        signal(SIGBUS, SWServHandleSignal);
        signal(SIGFPE, SWServHandleSignal);
        signal(SIGKILL, SWServHandleSignal);
        signal(SIGUSR1, SWServHandleSignal);
        signal(SIGSEGV, SWServHandleSignal);
        signal(SIGUSR2, SWServHandleSignal);
        signal(SIGPIPE, SWServHandleSignal);
        signal(SIGALRM, SWServHandleSignal);
        signal(SIGTERM, SWServHandleSignal);
        signal(SIGCHLD, SWServHandleSignal);
        signal(SIGCONT, SWServHandleSignal);
        signal(SIGSTOP, SWServHandleSignal);
        signal(SIGTSTP, SWServHandleSignal);
        signal(SIGTTIN, SWServHandleSignal);
        signal(SIGTTOU, SWServHandleSignal);
        signal(SIGURG, SWServHandleSignal);
        signal(SIGXCPU, SWServHandleSignal);
        signal(SIGXFSZ, SWServHandleSignal);
        signal(SIGVTALRM, SWServHandleSignal);
        signal(SIGPROF, SWServHandleSignal);
        signal(SIGWINCH, SWServHandleSignal);
        signal(SIGIO, SWServHandleSignal);
        signal(SIGPOLL, SWServHandleSignal);
#ifndef __CYGWIN32__
        signal(SIGPWR, SWServHandleSignal);
#endif /* __CYGWIN32__ */


        /* ********************************************************* */

        /*   Run the monitor as needed, this needs to be done after
         *   the rcfile has been parsed so that the AUX Stat listening
	 *   socket has been created.
	 */
	if(fname.monitor_set &&
	   ISPATHEXECUTABLE(fname.monitor)
	)
	{
	    /* Use first AUX Stats listening socket port number. */
            for(i = 0, n = 0; i < total_incoming_sockets; i++)
            {
                if(incoming_socket[i] == NULL)
                    continue;

                if(incoming_socket[i]->type == INCOMING_SOCKET_TYPE_AUXSTATS)
                    n = incoming_socket[i]->port_num;
            }

	    if(n > 0)
	    {
	        sprintf(
		    cmd,
		    "%s %s %i",
		    fname.monitor,
		    "localhost",
		    n
	        );
	        Exec(cmd);
	    }
	    else
	    {
		fprintf(
		    stderr,
 "Warning: Requested use of monitor but no AUX Stats port available.\n"
		);
                fprintf(
                    stderr,
 "Warning: Monitor not executed.\n"
                );
	    }
	}


        /* ********************************************************* */
	/* Allocate recycled objects buffers. */
	status = DBRecycleBufferInit(MAX_RECYCLED_OBJECTS);
	if(status < 0)
	{
	    fprintf(
		stderr,
		"Error: Unable to allocate recycled objects buffer\n."
	    );
            return(-1);
	}


	/* ********************************************************* */

	/* Load universe. */
	status = DBReadFromFile(fname.unv_in);
	if(status)
	{
	    fprintf(
		stderr,
		"Error loading input universe file `%s'.\n",
                fname.unv_in
	    );
	    return(-1);
	}



	/* Load Object Parameters Macros. */
	status = OPMLoadFromFile(fname.opm);
	if(status < 0)
	{
            fprintf(
                stderr,
                "Error loading object parameter models file `%s'.\n",
                fname.opm
            );
	    /* Non-fatal error. */
	}

	/* Load Object Create Scripts. */
        status = OCSLoadFromFile(fname.ocs);
        if(status < 0)
        {
            fprintf(
                stderr,
                "Error loading object create scripts file `%s'.\n",
                fname.ocs
            );
            /* Non-fatal error. */
        }


	/* ********************************************************** */
	/* Successful startup. */

        if(!sysparm.console_quiet)
        {
	    /* Universe title. */
            fprintf(
                stdout,
                "Universe: `%s'\n",
		unv_head.title
            );

	    /* Configuration file. */
            fprintf(
                stdout,
                "Configuration file: %s\n",
                fname.rc   
            );

	    /* Listening port(s), both standard and aux. */
	    for(i = 0; i < total_incoming_sockets; i++)
	    {
		if(incoming_socket[i] == NULL)
		    continue;

		if(incoming_socket[i]->type == INCOMING_SOCKET_TYPE_STANDARD)
                    fprintf(
                        stdout,
                        "Standard connection port: %i\n",
                        incoming_socket[i]->port_num
                    );
                if(incoming_socket[i]->type == INCOMING_SOCKET_TYPE_AUXSTATS)
                    fprintf(
                        stdout,
                        "AUX monitoring port: %i\n",
                        incoming_socket[i]->port_num
                    );

	    }

	    /* Processs ID. */
	    fprintf(stdout, "Process ID: %i\n", root_pid);
        }

        /* Log startup. */
        sprintf(stringa,
            "*** %s %s starting...",
            PROG_NAME, PROG_VERSION
        );
        if(sysparm.log_general)
            LogAppendLineFormatted(fname.primary_log, stringa);


        /* ********************************************************** */
	/* Reality engine initialize. */
	REngInit();


	return(0);
}


/*
 *	Procedure to manage all resources and responsbilities.
 */
void SWServManage()
{
        int i, status;
        time_t new_millitime;

        char stringa[1024];
	connection_struct **con_ptr;
	incoming_socket_struct **is_ptr;


	while(runlevel >= 2)
	{
            /* Sleep. */
            usleep(CYCLE_LAPSE_US);


            /* Update millisecond timming. */
	    new_millitime = MilliTime();

            /*   Reset timmers if new millitime is less than
             *   cur_millitime.
             */
            if(new_millitime < cur_millitime)
            {
                /* Reset global timmers. */
                SWServDoResetTimmers();

                lapsed_millitime = 0;
                time_compensation = 1.0;
            }
            else
            {
		lapsed_millitime = new_millitime - cur_millitime;
                time_compensation = (double)(
                    (double)lapsed_millitime / (double)CYCLE_LAPSE_MS
                );
                if(time_compensation < 1)
                    time_compensation = 1;
            }

            /* Now set global variable cur_millitime. */
            cur_millitime = new_millitime;


	    /* Update system time timming (in seconds). */
	    cur_systime = time(NULL);


            /* ***************************************************** */

	    /* Check if a global `reset' is needed. */
	    if(master_reset)
	    {
		/* Perform `reset' procedure. */
		SWServDoReset(master_reset);

		master_reset = 0;
	    }

            /* ***************************************************** */

	    /* System updates. */
	    if(next.system_check <= cur_millitime)
	    {
		/* Poll socket. */
		if(next.socket_poll <= cur_millitime)
		{
		    for(i = 0, is_ptr = incoming_socket;
                        i < total_incoming_sockets;
                        i++, is_ptr++
                    )
		    {
			if(*is_ptr == NULL)
			    continue;

			switch((*is_ptr)->type)
			{
			  case INCOMING_SOCKET_TYPE_AUXSTATS:
                            status = AUXConManageNewConnections(
                                (*is_ptr)->socket
                            );
			    break;

			  default:	/* INCOMING_SOCKET_TYPE_STANDARD */
			    status = NetManageNewConnections(
			        (*is_ptr)->socket
			    );
		            switch(status)
		            {
		              /* No error. */
		              case 0:
			        break;

		              /* General error. */
		              case -1:
			        break;

		              /* Connection would exceed sysparm.max_connections. */
		              case -2:
			        break;

		              /*  Connection was guest connection
                               *  and exceeded sysparm.max_guests.
		               */
		              case -3:
			        break;

		              /* Other error. */
		              default:
			        break;
			    }
			    break;
			}	/* switch((*is_ptr)->type) */
		    }

		/*
		 * Resend login request to connections that are connected
		 * but haven't given us a name and password (not logged in).
                 */
                    for(i = 0, con_ptr = connection;
                        i < total_connections;
                        i++, con_ptr++
		    )
                    {
			if(*con_ptr == NULL)
			    continue;

                        /* Skip if connection is not connected. */
                        if((*con_ptr)->socket < 0)
                            continue;

			/* Skip if connection's object *is* valid. */
			if(!DBIsObjectGarbage((*con_ptr)->object_num))
                            continue;

		        /* Close socket if timmed out. */
		        if(((*con_ptr)->contime + sysparm.login_timeout) <
			   time(NULL)
		        )
		        {
                            sprintf(stringa,
				"Connection %i: Timmed out.",
                                i
			    );
                            if(sysparm.log_net)
                                LogAppendLineFormatted(fname.primary_log, stringa);

			    /* Close connection. */
			    NetCloseConnection(i);

			    /* Reget connection pointer array. */
			    con_ptr = &connection[i];

			    continue;
		        }

		        /* Resend login request. */
			NetSendLogin(i);
		    }


		    /* Schedual next time to poll for new connections. */
                    next.socket_poll = cur_millitime +
			sysparm.int_new_connection_poll;
	        }

		/* Fetch OS stats. */
		if(next.os_stats <= cur_systime)
		{
		    SWServGetOSStats();

		    /* Schedual next time to fetch OS stats. */
		    next.os_stats = cur_systime +
			sysparm.int_os_stats;
		}

		/* Refresh and reclaim memory. */
		if(next.memory_clean <= cur_millitime)
		{
                    /* Universe objects. */
                    DBReclaim();

                    /* Object parameter macros. */
                    OPMReclaim();

                    /* Object create scripts. */
                    OCSReclaim();

		    /* Scheduals. */
		    SchedualReclaim();

		    /* Connections. */
		    ConReclaim();

                    /* Next time memory is to be refreshed and reclaimed. */
                    next.memory_clean = cur_millitime + sysparm.int_memory_clean;
		}

		/* Database save. */
		if(next.unv_save <= cur_systime)
		{
		    SWServDoSave();

		    /* Mark next time for universe save. */
		    next.unv_save = cur_systime + sysparm.int_unv_save;
		}

                /* Stats export. */
                if(next.stats_export <= cur_systime)
                {
		    SWServDoExportStats();

                    /* Mark next time stat files are to be exported, in SECONDS! */
                    next.stats_export = cur_systime + sysparm.int_stats_export;
                }


		/* Manage scheduals. */
		SchedualManage();


		/* Schedual next time for system check. */
		next.system_check = cur_millitime + sysparm.int_system_check;
	    }



            /* ****************************************************** */
	    /* Reality engine management. */

	    /* Update all objects. */
	    REngManage();


            /* ****************************************************** */
	    /* Network. */

	    /* AUX connections. */
	    AUXConManageAll();


            /* Send due outgoing network data. */
	    NetManageSend();

            /* Recieve and process incoming data. */
	    NetManageRecv();


#ifdef PLUGIN_SUPPORT
	    /* ****************************************************** */
	    /* Plugins management. */
	    PluginManage();
#endif	/* PLUGIN_SUPPORT */

	}


	return;
}


/*
 *      Procedure to deallocate all global memory owned
 *      or shared by this program.
 */
void SWServShutdown()   
{
        char sndbuf[CS_DATA_MAX_LEN];


#ifdef PLUGIN_SUPPORT
	/* Unload all plugins. */
	PluginUnloadAll();
#endif	/* PLUGIN_SUPPORT */


        /* Warn all connections of shutdown. */
        sprintf(
	    sndbuf,
            "*** Server shutting down."
        );
        NetSendLiveMessage(-1, sndbuf);


        /* Reality engine. */
        REngShutdown();

        /* Recycled objects buffers. */
        DBRecycleBufferDeleteAll();

        /* Scheduals. */
        SchedualDeleteAll();

        /* Close and free all connection structures. */
        ConDeleteAll();

        /* XSW Objects. */
        DBDeleteAllObjects();

        /* Object Create Scripts. */
        OCSDeleteAll();

        /* Object Parameter Macros. */
        OPMDeleteAll();

	/* Universe object management resources. */
	UNVShutdown();

        /* AUX monitoring connections. */
        AUXConDeleteAll();

	/* Site ban list. */
	SiteBanDeleteAll();

        /* Incoming socket structures. */
        IncomingSocketDeleteAll();


        return;
}


/*
 *      Procedure to begin shutdown sequence, not to be confused
 *	with SWServShutdown() which does the actual shutting down
 *	procedure.  This function is only to start of the initial
 *	procedure to shutdown.
 */
void SWServDoShutdown()
{
	/* Notify all connections about shutting down. */
	NetSendLiveMessage(
	    -1,
	    "Server shutting down."
	);

        /*   Close and free all connection structures. This is so that
	 *   Guest objects are not saved.
	 */
        ConDeleteAll();


        /* Save database. */
	SWServDoSave();


        /* Set global runlevel to 1, this will cause the main while()
	 * loop to break.
	 */
        runlevel = 1;


        return;
}



int main(int argc, char *argv[])
{
	int status;
	char stringa[1024];


	/* Initialization. */
	runlevel = 1;

	status = SWServInit(argc, argv);
	switch(status)
	{
	  case 0:
	    break;

	  case -4:
            SWServShutdown();
	    return(0);
	    break;

	  default:
            SWServShutdown();
	    fprintf(
		stderr,
		"Shutting down due to fatal error.\n"
	    );
	    return(1);
	    break;
	}


	/* Management. */
	runlevel = 2;
	SWServManage();


	/* Shutdown. */
	SWServShutdown();

        /* Log shutdown. */
	if(!sysparm.console_quiet)
            fprintf(
		stdout,
		"%s shut down normally.\n",
		PROG_NAME
	    );

	sprintf(stringa,
	    "*** %s %s shut down normally.",
            PROG_NAME, PROG_VERSION
        );
        if(sysparm.log_general)
	    LogAppendLineFormatted(fname.primary_log, stringa);


	return(0);
}
