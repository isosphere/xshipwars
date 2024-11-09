/*
                    Restart Configuration File

	Functions:

	int RCLoadFromFile(char *filename)


 */

#include "../include/strexp.h"

#include "swserv.h"
#include "siteban.h"
#ifdef PLUGIN_SUPPORT
# include "plugins.h"
#endif


int RCLoadFromFile(char *filename)
{
	int i, n, status;
        char *strptr, *strptr2, *strptr3;
        FILE *fp;
        off_t filesize;
        struct stat stat_buf;

        char parm[CFG_PARAMETER_MAX];
        char val[CFG_VALUE_MAX];

	int lines_read = 0;


	/* Recording newly created listening sockets. */
	int *lsocket_port = NULL;
	int total_lsocket_ports = 0;


        /* Check if exists. */
        if(stat(filename, &stat_buf))
        {
	    fprintf(stderr, "%s: No such file.\n", filename);
            return(-1);
        }

	/* Get size of file. */
        filesize = stat_buf.st_size;

        /* Open filename. */
        fp = fopen(filename, "r");
        if(fp == NULL)
        {
            fprintf(stderr, "%s: Cannot open file.\n",
                filename
            );
            return(-1);
        }

        /* *************************************************** */
        /* Deallocate resources that would be recreated by
	 * this function.
	 */
#ifdef PLUGIN_SUPPORT
	/* Plugins. */
	PluginUnloadAll();
#endif	/*PLUGIN_SUPPORT */

        /* Site ban list. */
        SiteBanDeleteAll();



	/* *********************************************************** */

        strptr = NULL;

        while(1)
        {
            /* Free previous line and allocate/read next line. */
            free(strptr); strptr = NULL;
            strptr = FReadNextLineAllocCount(
		fp, UNIXCFG_COMMENT_CHAR, &lines_read
	    );
            if(strptr == NULL) break;

            /* Fetch parameter. */
            strptr2 = StringCfgParseParm(strptr);
            if(strptr2 == NULL) continue;
            strncpy(parm, strptr2, CFG_PARAMETER_MAX);
            parm[CFG_PARAMETER_MAX - 1] = '\0';

            /* Fetch value. */
            strptr2 = StringCfgParseValue(strptr);
            if(strptr2 == NULL) strptr2 = "0";  /* Set it to "0" if NULL. */
            strncpy(val, strptr2, CFG_VALUE_MAX);
            val[CFG_VALUE_MAX - 1] = '\0';


	    /* ListeningPort */
	    if(!strcasecmp(parm, "ListeningPort"))
	    {
		int port_num;


		port_num = atoi(val);

		status = IncomingSocketInit(
		    port_num,
		    INCOMING_SOCKET_TYPE_STANDARD
		);

		/* Record this listening socket's port number. */
		n = total_lsocket_ports;
		total_lsocket_ports++;
		lsocket_port = (int *)realloc(
		    lsocket_port,
		    total_lsocket_ports * sizeof(int)
		);
		if(lsocket_port == NULL)
		{
		    total_lsocket_ports = 0;
		}
		else
		{
		    lsocket_port[n] = port_num;
		}
            }
            /* AUXStatsListeningPort */
            else if(!strcasecmp(parm, "AUXStatsListeningPort"))
            {
                int port_num;


                port_num = atoi(val);

                status = IncomingSocketInit(
                    port_num,
                    INCOMING_SOCKET_TYPE_AUXSTATS
                );

                /* Record this listening socket's port number. */
                n = total_lsocket_ports;
                total_lsocket_ports++;
                lsocket_port = (int *)realloc(   
                    lsocket_port,
                    total_lsocket_ports * sizeof(int)
                );
                if(lsocket_port == NULL)
                {
                    total_lsocket_ports = 0;
                }
                else
                {
                    lsocket_port[n] = port_num;
                }
            }
	    /* ToplevelDir */
	    else if(!strcasecmp(parm, "ToplevelDir") ||
                    !strcasecmp(parm, "ServerToplevelDir")
	    )
            {
#ifndef __WIN32__ /* Disable this for windows. */
		strncpy(dname.toplevel, val, PATH_MAX);
		dname.toplevel[PATH_MAX - 1] = '\0';

		/* Warn if not an absolute path. */
		if(!ISPATHABSOLUTE(dname.toplevel))
		{
                    fprintf(stderr,
                  "%s: Line %i: Warning: %s: Not absolute path statement.\n",
                        filename,  
                        lines_read,
                        dname.toplevel
                    );
		}

		/* Check if exists. */
		if(stat(dname.toplevel, &stat_buf))
		{
		    fprintf(stderr,
                        "%s: Line %i: Warning: %s: No such directory.\n",
		        filename,
                        lines_read,
                        dname.toplevel
		    );
		}
#endif /* __WIN32__ */
            }
            /* EtcDir */
            else if(!strcasecmp(parm, "EtcDir") ||
                    !strcasecmp(parm, "ServerEtcDir")
	    )
            {
                strncpy(dname.etc, val, PATH_MAX);
                dname.etc[PATH_MAX - 1] = '\0';
                        
                if(!ISPATHABSOLUTE(dname.etc))
                {
                    strptr3 = PrefixPaths(
                        dname.toplevel,
                        dname.etc
                    );
                    if(strptr3 != NULL)
                        strncpy(dname.etc, strptr3, PATH_MAX);
                    dname.etc[PATH_MAX - 1] = '\0';
                }

		/* Check if exists. */
                if(stat(dname.etc, &stat_buf))
                {
                    fprintf(stderr,
                        "%s: Line %i: Warning: %s: No such directory.\n",
                        filename,
                        lines_read,
                        dname.etc
                    );
                }
            }
            /* LogsDir */
            else if(!strcasecmp(parm, "LogsDir") ||
                    !strcasecmp(parm, "ServerLogsDir")
	    )
            {
                strncpy(dname.logs, val, PATH_MAX);
		dname.logs[PATH_MAX - 1] = '\0';
                        
                if(!ISPATHABSOLUTE(dname.logs))
                {
                    strptr3 = PrefixPaths(
                        dname.toplevel,
                        dname.logs
                    );
                    if(strptr3 != NULL)
                        strncpy(dname.logs, strptr3, PATH_MAX);
                    dname.logs[PATH_MAX - 1] = '\0';
                }

		/* Check if exists. */
                if(stat(dname.logs, &stat_buf))
                {
                    fprintf(stderr,
                        "%s: Line %i: Warning: %s: No such directory.\n",
                        filename,
                        lines_read,
                        dname.logs
                    );   
                }
            }
            /* BinDir */
            else if(!strcasecmp(parm, "BinDir") ||
                    !strcasecmp(parm, "ServerBinDir")
	    )
            {
#ifndef __WIN32__ /* Disable item for windows. */
                strncpy(dname.bin, val, PATH_MAX);
		dname.bin[PATH_MAX - 1] = '\0';

                if(!ISPATHABSOLUTE(dname.bin))
                {
                    strptr3 = PrefixPaths(
                        dname.toplevel,
                        dname.bin
                    );
                    if(strptr3 != NULL)
                        strncpy(dname.bin, strptr3, PATH_MAX);
                    dname.bin[PATH_MAX - 1] = '\0';   
                }

		/* Check if exists. */
                if(stat(dname.bin, &stat_buf))
                {
                    fprintf(stderr,
                        "%s: Line %i: Warning: %s: No such directory.\n",
                        filename,
                        lines_read,
                        dname.bin
                    );
                }
#endif /* __WIN32__ */
            }
            /* DBDir */
            else if(!strcasecmp(parm, "DBDir") ||
                    !strcasecmp(parm, "ServerDBDir")
	    )
            {
                strncpy(dname.db, val, PATH_MAX);
		dname.db[PATH_MAX - 1] = '\0';

                if(!ISPATHABSOLUTE(dname.db))
                {
                    strptr3 = PrefixPaths(
                        dname.toplevel,
                        dname.db
                    );
                    if(strptr3 != NULL)
                        strncpy(dname.db, strptr3, PATH_MAX);
                    dname.db[PATH_MAX - 1] = '\0';
                }

		/* Check if exists. */
                if(stat(dname.db, &stat_buf))
                {
                    fprintf(stderr,
                        "%s: Line %i: Warning: %s: No such directory.\n",
                        filename,
                        lines_read,
                        dname.db
                    );
                }
            }
            /* TmpDir */
            else if(!strcasecmp(parm, "TmpDir") ||
                    !strcasecmp(parm, "ServerTmpDir")
	    )
            {
		strncpy(dname.tmp, val, PATH_MAX);
		dname.tmp[PATH_MAX - 1] = '\0';

                if(!ISPATHABSOLUTE(dname.tmp))
                {
		    strptr3 = PrefixPaths(
			dname.toplevel,
			dname.tmp
		    );
		    if(strptr3 != NULL)
                        strncpy(dname.tmp, strptr3, PATH_MAX);
		    dname.tmp[PATH_MAX - 1] = '\0';
                }

		/* Check if exists. */
                if(stat(dname.tmp, &stat_buf))
                {
                    fprintf(stderr,
                        "%s: Line %i: Warning: %s: No such directory.\n",
                        filename,
                        lines_read,
                        dname.tmp
                    );
                }
            }
	    /* PluginsDir */
	    else if(!strcasecmp(parm, "PluginsDir") ||
                    !strcasecmp(parm, "ServerPluginsDir")
	    )
            {
                strncpy(dname.plugins, val, PATH_MAX);
                dname.plugins[PATH_MAX - 1] = '\0';

                if(!ISPATHABSOLUTE(dname.plugins))
                {
                    strptr3 = PrefixPaths(
                        dname.toplevel,  
                        dname.plugins
                    );
                    if(strptr3 != NULL)
                        strncpy(dname.plugins, strptr3, PATH_MAX);   
                    dname.plugins[PATH_MAX - 1] = '\0';
                }

                /* Check if exists. */
                if(stat(dname.plugins, &stat_buf))
                {
                    fprintf(stderr,
                        "%s: Line %i: Warning: %s: No such directory.\n",
                        filename,
                        lines_read,
                        dname.plugins
                    );
                }
            }
            /* PublicHTMLDir */
            else if(!strcasecmp(parm, "PublicHTMLDir"))
            {
		strncpy(dname.public_html, val, PATH_MAX);
		dname.public_html[PATH_MAX - 1] = '\0';

                if(!ISPATHABSOLUTE(dname.public_html))
                {
		    strptr3 = PrefixPaths(
			dname.toplevel,
			dname.public_html
		    );
		    if(strptr3 != NULL)
                        strncpy(dname.public_html, strptr3, PATH_MAX);
		    dname.public_html[PATH_MAX - 1] = '\0';
                }

		/* Check if exists. */
                if(stat(dname.public_html, &stat_buf))
                {
                    fprintf(stderr,
                        "%s: Line %i: Warning: %s: No such directory.\n",
                        filename,
                        lines_read,
                        dname.public_html
                    );
                }
            }

	    /* ******************************************************** */
	    /* Monitor */
            else if(!strcasecmp(parm, "Monitor"))
            {
                if(fname.monitor_set == 0)
                {
                    strncpy(
                        fname.monitor,
                        val,
                        PATH_MAX + NAME_MAX
                    );
		    fname.monitor[PATH_MAX + NAME_MAX - 1] = '\0';
                    fname.monitor_set = 1;
                }

                if(!ISPATHABSOLUTE(fname.monitor))
                {
		    strptr3 = PrefixPaths(
                        dname.bin,
                        fname.monitor
                    );
		    if(strptr3 != NULL)
                        strncpy(fname.monitor, strptr3, PATH_MAX + NAME_MAX);
		    fname.monitor[PATH_MAX + NAME_MAX - 1] = '\0';
                }

		/* Check if exists. */
		if(stat(fname.monitor, &stat_buf))
		{
		    fprintf(stderr,
			"%s: Line %i: Warning: %s: No such file.\n",
			filename, lines_read, fname.monitor
		    );
		}
            }
            /* UnvIn */
            else if(!strcasecmp(parm, "UnvIn") ||
                    !strcasecmp(parm, "DBIn")
	    )
            {
                if(fname.unv_in_set == 0)
                {
                    strncpy(
                        fname.unv_in,
                        val,
                        PATH_MAX + NAME_MAX
                    );
		    fname.unv_in[PATH_MAX + NAME_MAX - 1] = '\0';
		    fname.unv_in_set = 1;
		}
                /* Set full path as needed. */
                if(!ISPATHABSOLUTE(fname.unv_in))
                {
		    strptr3 = PrefixPaths(dname.db, fname.unv_in);
		    if(strptr3 != NULL)
		        strncpy(fname.unv_in, strptr3, PATH_MAX + NAME_MAX);
                }
		fname.unv_in[PATH_MAX + NAME_MAX - 1] = '\0';

                /* Check if exists. */
                if(stat(fname.unv_in, &stat_buf))
                {
                    fprintf(stderr,
                        "%s: Line %i: Warning: %s: No such file.\n",
                        filename, lines_read, fname.unv_in
                    );
                }
            }
            /* UnvOut */ 
            else if(!strcasecmp(parm, "UnvOut") ||
                    !strcasecmp(parm, "DBOut")
	    )
            {
                if(fname.unv_out_set == 0)
                {
                    strncpy(
                        fname.unv_out,
                        val,
                        PATH_MAX + NAME_MAX
                    );
		    fname.unv_out[PATH_MAX + NAME_MAX - 1] = '\0';
                    fname.unv_out_set = 1;
                }
                /* Set full path as needed. */
                if(!ISPATHABSOLUTE(fname.unv_out))
                {
                    strptr3 = PrefixPaths(dname.db, fname.unv_out);
                    if(strptr3 != NULL)
                        strncpy(fname.unv_out, strptr3, PATH_MAX + NAME_MAX);
                }
		fname.unv_out[PATH_MAX + NAME_MAX - 1] = '\0';

		/* Check if fname.unv_out exists, if so, warn that it will
		 * be overwritten.
		 */
		if(!stat(fname.unv_out, &stat_buf))
		{
/* Wait, how do we know this function was called at startup?
   Maybe it was called in the middle of execution?
		    fprintf(stderr,
"Warning: %s: Universe output file exists and will be overwritten in\n\
less than %ld seconds. Rename the file if you want to save it, do not\n\
shut down the server.\n",
			fname.unv_out,
			sysparm.int_unv_save
		    );
 */
		}
            }
	    /* OCSFile */
            else if(!strcasecmp(parm, "OCSFile"))
            {
                if(fname.ocs_set == 0)
                {
                    strncpy(
                        fname.ocs,
                        val,
                        PATH_MAX + NAME_MAX
                    );
		    fname.ocs[PATH_MAX + NAME_MAX - 1] = '\0';
                    fname.ocs_set = 1;
                }
                /* Set full path as needed. */
                if(!ISPATHABSOLUTE(fname.ocs))
                {
		    strptr3 = PrefixPaths(dname.db, fname.ocs);
		    if(strptr3 != NULL)
		        strncpy(fname.ocs, strptr3, PATH_MAX + NAME_MAX);
                }
                fname.ocs[PATH_MAX + NAME_MAX - 1] = '\0';

                /* Check if exists. */
                if(stat(fname.ocs, &stat_buf))
                {
                    fprintf(stderr,
                        "%s: Line %i: Warning: %s: File not found.\n",
                        filename, lines_read, fname.ocs
                    );
                }
            }
            /* OPMFile */
            else if(!strcasecmp(parm, "OPMFile"))
            {
                if(fname.opm_set == 0)
                {
                    strncpy(
                        fname.opm,
                        val,
                        PATH_MAX + NAME_MAX
                    );
		    fname.opm[PATH_MAX + NAME_MAX - 1] = '\0';
                    fname.opm_set = 1;
                }
		/* Set full path as needed. */
		if(!ISPATHABSOLUTE(fname.opm))
		{
                    strptr3 = PrefixPaths(dname.db, fname.opm);
                    if(strptr3 != NULL)
                        strncpy(fname.opm, strptr3, PATH_MAX + NAME_MAX);
		}
                fname.opm[PATH_MAX + NAME_MAX - 1] = '\0';

                /* Check if exists. */
                if(stat(fname.opm, &stat_buf))
                {
                    fprintf(stderr,
                        "%s: Line %i: Warning: %s: No such file.\n",
                        filename, lines_read, fname.opm
                    );
                }
            }
            /* PrimaryLogFile */
            else if(!strcasecmp(parm, "PrimaryLogFile"))
            {
                strncpy(
                    fname.primary_log,
                    val,
                    PATH_MAX + NAME_MAX
                );
		fname.primary_log[PATH_MAX + NAME_MAX - 1] = '\0';

                /* Set full path as needed. */
                if(!ISPATHABSOLUTE(fname.primary_log))
                {
                    strptr3 = PrefixPaths(dname.logs, fname.primary_log);
                    if(strptr3 != NULL)
                        strncpy(fname.primary_log, strptr3, PATH_MAX + NAME_MAX);
                }
                fname.primary_log[PATH_MAX + NAME_MAX - 1] = '\0';
                continue;
            }
            /* ConListExportFile */
            else if(!strcasecmp(parm, "ConListExportFile") ||
                    !strcasecmp(parm, "ConListHTML")
	    )
            {
                strncpy(
                    fname.conlist_export,
                    val,
                    PATH_MAX + NAME_MAX
                );
		fname.conlist_export[PATH_MAX + NAME_MAX - 1] = '\0';

                /* Set full path as needed. */
                if(!ISPATHABSOLUTE(fname.conlist_export))
                {
		    strncpy(
			fname.conlist_export,
			PrefixPaths(dname.public_html, fname.conlist_export),
			PATH_MAX + NAME_MAX
		    );
                }
                fname.conlist_export[PATH_MAX + NAME_MAX - 1] = '\0';
            }
            /* ScoresExportFile */
            else if(!strcasecmp(parm, "ScoresExportFile") ||
                    !strcasecmp(parm, "ScoresHTML")
	    )
            {
                strncpy(
                    fname.scores_export,
                    val,
                    PATH_MAX + NAME_MAX
                );
		fname.scores_export[PATH_MAX + NAME_MAX - 1] = '\0';

                /* Set full path as needed. */
                if(!ISPATHABSOLUTE(fname.scores_export))
                {
                    strncpy(
                        fname.scores_export,
                        PrefixPaths(dname.public_html, fname.scores_export),
                        PATH_MAX + NAME_MAX
                    );
                }
                fname.scores_export[PATH_MAX + NAME_MAX - 1] = '\0';
            }
            /* EventsExportFile */
            else if(!strcasecmp(parm, "EventsExportFile") ||
                    !strcasecmp(parm, "EventsHTML")
	    )
            {
                strncpy(
                    fname.events_export,
                    val,
                    PATH_MAX + NAME_MAX
                );
		fname.events_export[PATH_MAX + NAME_MAX - 1] = '\0';

                /* Set full path as needed. */
                if(!ISPATHABSOLUTE(fname.events_export))
                {
                    strncpy(
                        fname.events_export,
                        PrefixPaths(dname.public_html, fname.events_export), 
                        PATH_MAX + NAME_MAX
                    );
                }
                fname.events_export[PATH_MAX + NAME_MAX - 1] = '\0';
            }


	    /* ********************************************************* */
	    /* System parameters. */

            /* IntervalObjectValues */
            else if(!strcasecmp(parm, "IntervalObjectValues"))
            {
		sysparm.int_object_values = atol(val);
		if(sysparm.int_object_values < MIN_OBJECT_UPDATE_INT)
		{
		    fprintf(stderr,
   "%s: Line %i: Warning: IntervalObjectValues too low, setting to minimum %i.\n",
			filename,
			lines_read,
			MIN_OBJECT_UPDATE_INT
		    );
		    sysparm.int_object_values = MIN_OBJECT_UPDATE_INT;
		}

		next.object_values = cur_millitime + sysparm.int_object_values;
            }
            /* IntervalWeaponValues */
            else if(!strcasecmp(parm, "IntervalWeaponValues"))
            {
		sysparm.int_weapon_values = atol(val);
                if(sysparm.int_weapon_values < MIN_OBJECT_UPDATE_INT)
                {
                    fprintf(stderr,
   "%s: Line %i: Warning: IntervalWeaponValues too low, setting to minimum %i.\n",
                        filename,
                        lines_read,
			MIN_OBJECT_UPDATE_INT
                    );
                    sysparm.int_weapon_values = MIN_OBJECT_UPDATE_INT;
                }
                next.need_weapon_values = 1;
            }
            /* IntervalAUXStats */
            else if(!strcasecmp(parm, "IntervalAUXStats"))
            {
                sysparm.int_aux_con_stats = atol(val);
                if(sysparm.int_aux_con_stats < 1000)
                {
                    fprintf(stderr,
 "%s: Line %i: Warning: IntervalAUXStats too low, setting to minimum 1000.\n",
                        filename,
                        lines_read
                    );
                    sysparm.int_aux_con_stats = 1000;
                }
            }
            /* IntervalOSStats */
	    else if(!strcasecmp(parm, "IntervalOSStats"))
            {
                sysparm.int_os_stats = atol(val);	/* In seconds. */
                if(sysparm.int_os_stats < 1)
                {
                    fprintf(stderr,
   "%s: Line %i: Warning: IntervalOSStats too low, setting to minimum %i.\n",
                        filename,
                        lines_read,
			1
                    );
                    sysparm.int_os_stats = 1;
                }

		next.os_stats = cur_systime;	/* Next should be now. */
            }
            /* IntervalUnvSave */
            else if(!strcasecmp(parm, "IntervalUnvSave") ||
                    !strcasecmp(parm, "IntervalDBSave")
	    )
            {
                sysparm.int_unv_save = atol(val);	/* In seconds. */
                if(sysparm.int_unv_save < 1)
                {
                    fprintf(stderr,
   "%s: Line %i: Warning: IntervalDBSave too low, setting to minimum %i.\n",
                        filename,
                        lines_read,
			1
                    );
                    sysparm.int_unv_save = 1;
                }

                next.unv_save = cur_systime + sysparm.int_unv_save;
            }
            /* IntervalStatsExportFile */
            else if(!strcasecmp(parm, "IntervalStatsExportFile") ||
                    !strcasecmp(parm, "IntervalStatisticsExport")
	    )
            {
                sysparm.int_stats_export = atol(val);	/* In seconds. */
                if(sysparm.int_stats_export < 1)
                {
                    fprintf(stderr,
 "%s: Line %i: Warning: IntervalStatisticsExport too low, setting to minimum %i.\n",
                        filename,
                        lines_read,
			1
                    );
                    sysparm.int_stats_export = 1;
                }

                next.stats_export = cur_systime + sysparm.int_stats_export;
            }
            /* MaxConnections */
            else if(!strcasecmp(parm, "MaxConnections"))
            {
                sysparm.max_connections = atol(val);
                if(sysparm.max_connections < 1)
                {
                    fprintf(stderr,
           "%s: Line %i: Error: MaxConnections cannot be less than 1.\n",
                        filename,
                        lines_read
                    );
                    sysparm.max_connections = 1;
                }
            }
            /* MaxGuests */
            else if(!strcasecmp(parm, "MaxGuests"))
            {
                sysparm.max_guests = atol(val);
                if(sysparm.max_guests < 0)
                    sysparm.max_guests = 0;
            }
            /* MaxAUXStatsConnections */
            else if(!strcasecmp(parm, "MaxAUXStatsConnections"))
            {
                sysparm.max_aux_connections = atol(val);
		if(sysparm.max_aux_connections < 0)
		    sysparm.max_aux_connections = 0;
            }
	    /* LoginTimeout */
	    else if(!strcasecmp(parm, "LoginTimeout"))
	    {
		sysparm.login_timeout = atol(val);	/* Seconds. */
                if(sysparm.login_timeout < 1)
                    sysparm.login_timeout = 1;
	    }
	    /* MaxFailedLogins */
	    else if(!strcasecmp(parm, "MaxFailedLogins"))
            {
                sysparm.max_failed_logins = atoi(val);
                if(sysparm.max_failed_logins < 1)
                    sysparm.max_failed_logins = 1;
            }
	    /* GuestLoginName */
            else if(!strcasecmp(parm, "GuestLoginName"))
            {
                strncpy(sysparm.guest_login_name, val, XSW_OBJ_NAME_MAX);
		sysparm.guest_login_name[XSW_OBJ_NAME_MAX - 1] = '\0';
            }
	    /* AllowGuestLogins */
            else if(!strcasecmp(parm, "AllowGuestLogins"))
            {
		sysparm.allow_guest = StringIsYes(val);
            }
	    /* ConnectNotify */
	    else if(!strcasecmp(parm, "ConnectNotify"))
            {
                sysparm.con_notify = StringIsYes(val);
	    }
	    /* SingleConnection */
	    else if(!strcasecmp(parm, "SingleConnection"))
            {
                sysparm.single_connection = StringIsYes(val);
            }
            /* CeaseFire */
            else if(!strcasecmp(parm, "CeaseFire"))
            {
                sysparm.cease_fire = StringIsYes(val);
            }
            /* HidePlayers */
            else if(!strcasecmp(parm, "HidePlayers"))
            {
                sysparm.hide_players = StringIsYes(val);
            }
            /* HomesDestroyable */
            else if(!strcasecmp(parm, "HomesDestroyable"))
            {
                sysparm.homes_destroyable = StringIsYes(val);
            }
	    /* KillerGetsCredits */
            else if(!strcasecmp(parm, "KillerGetsCredits"))
            {
                sysparm.killer_gets_credits = StringIsYes(val);
            }
	    /* ReportDestroyedWeapons */
	    else if(!strcasecmp(parm, "ReportDestroyedWeapons"))
            {
                sysparm.report_destroyed_weapons = StringIsYes(val);
            }
	    /* SendStarChart */
	    else if(!strcasecmp(parm, "SendStarChart"))
            {
                sysparm.send_starchart = StringIsYes(val);
            }
            /* HitPlayerBonus */
            else if(!strcasecmp(parm, "HitPlayerBonus"))
            {
                sysparm.hit_player_bonus = atof(val);
		if(sysparm.hit_player_bonus < 0)
		    sysparm.hit_player_bonus = 0;
            }
	    /* DmgCtlRate */
	    else if(!strcasecmp(parm, "DmgCtlRate") ||
                    !strcasecmp(parm, "DamageControlRate")
	    )
	    {
		sysparm.dmg_ctl_rate = atof(val);
                if(sysparm.dmg_ctl_rate < 0)
                    sysparm.dmg_ctl_rate = 0;
            }

	    /* lost_found_owner is set in the universe file. */

	    /* LogGeneral */
            else if(!strcasecmp(parm, "LogGeneral"))
            {
		sysparm.log_general = StringIsYes(val);
            }
            /* LogEvents */
            else if(!strcasecmp(parm, "LogEvents"))
            {
                sysparm.log_events = StringIsYes(val);
            }
            /* LogNet */
            else if(!strcasecmp(parm, "LogNet"))
            {
                sysparm.log_net = StringIsYes(val);
            }
            /* LogErrors */
            else if(!strcasecmp(parm, "LogErrors"))
            {
                sysparm.log_errors = StringIsYes(val);
            }

	    /* MessageWrongLogin */
	    else if(!strcasecmp(parm, "MessageWrongLogin"))
	    {
		strncpy(
		    sysparm.mesg_wrong_login,
		    val,
		    CS_MESG_MAX
		);
		sysparm.mesg_wrong_login[CS_MESG_MAX - 1] = '\0';
	    }
            /* MessageNoGuests */
            else if(!strcasecmp(parm, "MessageNoGuests"))
            {
                strncpy(
                    sysparm.mesg_no_guests,
                    val,
                    CS_MESG_MAX
                );
                sysparm.mesg_no_guests[CS_MESG_MAX - 1] = '\0';
            }
	    /* MessageWelcome */
            else if(!strcasecmp(parm, "MessageWelcome"))
            {
                strncpy(
                    sysparm.mesg_welcome,
                    val,
                    CS_MESG_MAX
                );
                sysparm.mesg_welcome[CS_MESG_MAX - 1] = '\0';
            }
            /* MessageLeave */
            else if(!strcasecmp(parm, "MessageLeave"))
            {
                strncpy(
                    sysparm.mesg_leave,
                    val,
                    CS_MESG_MAX
                );
                sysparm.mesg_leave[CS_MESG_MAX - 1] = '\0';
            }
#ifdef PLUGIN_SUPPORT
            /* ******************************************************** */
            /* BeginPlugin */
            else if(!strcasecmp(parm, "BeginPlugin"))
            {
		plugin_id_t id;
/*		u_int64_t flags = 0; */
		char *path = NULL;
		char **argv = NULL;
		int argc = 0;


                while(1)
                {
                    /* Free previous line and allocate/read next line. */
                    free(strptr); strptr = NULL;
                    strptr = FReadNextLineAllocCount(
                        fp, UNIXCFG_COMMENT_CHAR, &lines_read
                    );
                    if(strptr == NULL) break;

                    /* Fetch parameter. */
                    strptr2 = StringCfgParseParm(strptr);
                    if(strptr2 == NULL) continue;
                    strncpy(parm, strptr2, CFG_PARAMETER_MAX);
                    parm[CFG_PARAMETER_MAX - 1] = '\0';

                    /* Fetch value. */ 
                    strptr2 = StringCfgParseValue(strptr);
                    if(strptr2 == NULL) strptr2 = "0";
                    strncpy(val, strptr2, CFG_VALUE_MAX);
                    val[CFG_VALUE_MAX - 1] = '\0';


                    /* Path */
		    if(!strcasecmp(parm, "Path"))
		    {
			free(path);

			if(ISPATHABSOLUTE(val))
			{
			    path = StringCopyAlloc(val);
			}
			else
			{
			    strptr3 = PrefixPaths(dname.plugins, val);
			    path = StringCopyAlloc(strptr3);
			}
		    }
                    /* Flags */
                    else if(!strcasecmp(parm, "Flags"))
                    {
			int strc;
			char **strv;

			strv = strexp(val, &strc);

			for(i = 0; i < strc; i++)
			{



			}

			StringFreeArray(strv, strc);
		    }
                    /* Arguments */ 
                    else if(!strcasecmp(parm, "Arguments"))
                    {
                        StringFreeArray(argv, argc);
                        argv = strexp(val, &argc);
                    }

                    /* EndPlugin */
                    else if(!strcasecmp(parm, "EndPlugin"))
                    {
			/* Check if we got all the information. */
			if(path != NULL)
			{
			    id = PluginLoad(
				path,
				0,
				argc, argv,
				-1	/* No connection started this. */
			    );
			    if(id < 0)
			        fprintf(
				    stderr,
 "%s: Line %i: Cannot load plugin `%s'.\n",
				    filename, lines_read, path
				);
			}
			else
			{
			    if(path == NULL)
                                fprintf(
                                    stderr,
 "%s: Line %i: Insufficient information to load plugin.\n",
                                    filename, lines_read
                                );
			    else
                                fprintf(   
                                    stderr,
 "%s: Line %i: Insufficient information to load plugin `%s'.\n",
                                    filename, lines_read, path
                                );
			}

			/* Deallocate the tempory arguments. */
			free(path);
                        StringFreeArray(argv, argc);

                        break;
                    }
                }
	    }
#endif	/* PLUGIN_SUPPORT */
            /* ******************************************************** */
            /* BeginSiteBan */
            else if(!strcasecmp(parm, "BeginSiteBan"))
            {
		siteban_struct *sb_ptr;


		/* Allocate new site ban entry. */
		if(total_sitebans < 0)
		    total_sitebans = 0;

		n = total_sitebans;
		total_sitebans++;

		siteban = (siteban_struct **)realloc(
		    siteban,
		    total_sitebans * sizeof(siteban_struct *)
		);
		if(siteban == NULL)
		{
		    total_sitebans = 0;
		    continue;
		}

		siteban[n] = (siteban_struct *)calloc(
		    1,
		    sizeof(siteban_struct)
		);
		if(siteban[n] == NULL)
		{
		    total_sitebans--;
		    continue;
		}

		/* Get pointer to structure. */
		sb_ptr = siteban[n];


		while(1)
		{
                    /* Free previous line and allocate/read next line. */
                    free(strptr); strptr = NULL;
                    strptr = FReadNextLineAllocCount(
			fp, UNIXCFG_COMMENT_CHAR, &lines_read
		    );
                    if(strptr == NULL) break;

                    /* Fetch parameter. */
                    strptr2 = StringCfgParseParm(strptr);
                    if(strptr2 == NULL) continue;
                    strncpy(parm, strptr2, CFG_PARAMETER_MAX);
                    parm[CFG_PARAMETER_MAX - 1] = '\0';

                    /* Fetch value. */
                    strptr2 = StringCfgParseValue(strptr);
                    if(strptr2 == NULL) strptr2 = "0";
                    strncpy(val, strptr2, CFG_VALUE_MAX);
                    val[CFG_VALUE_MAX - 1] = '\0';


		    /* Address */
                    if(!strcasecmp(parm, "Address"))
                    {
			StringParseIP(
			    val,
			    &(sb_ptr->ip.part_u8[0]),
                            &(sb_ptr->ip.part_u8[1]),
                            &(sb_ptr->ip.part_u8[2]),
                            &(sb_ptr->ip.part_u8[3])
			);
		    }
                    /* Restrict */
                    else if(!strcasecmp(parm, "Restrict"))
                    {
			sb_ptr->restrict = atoi(val);
                    }
                    /* EndSiteBan */
                    else if(!strcasecmp(parm, "EndSiteBan"))
                    {
			break;
		    }
		}
	    }


	    /* ******************************************************** */
 	    /* Unknown parameter. */
	    else
	    {
		fprintf(stderr, "%s: Line %i: Unknown parameter: `%s'\n",
		    filename,
		    lines_read,
		    parm
		);
	    }
	}


	/* Close file. */
	fclose(fp);


	/* Close incoming connections listening sockets that don't
	 * match any port number in the newly created listening socket
	 * port numbers list.
	 */
	if(total_incoming_sockets > 0)
	{
	    int port_num;


	    for(i = 0; i < total_incoming_sockets; i++)
	    {
	        if(incoming_socket[i] == NULL)
		    continue;

	        /* Get port number. */
	        port_num = incoming_socket[i]->port_num;

	        /* Go through newly created listening socket port
                 * numbers.
		 */
		for(n = 0; n < total_lsocket_ports; n++)
	        {
	            if(lsocket_port[n] == port_num)
		        break;
	        }
	        if(n >= total_lsocket_ports)
	        {
		    /* Not in list newly created listening socket port
		     * numbers list, so close this incoming connections
		     * socket.
		     */
		    IncomingSocketDelete(i);
	        }
	    }
	}


	/* Free recorded newly created listening socket port numbers. */
	free(lsocket_port); lsocket_port = NULL;
	total_lsocket_ports = 0;


	return(0);
}
