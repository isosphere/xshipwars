/*
                  Restart Configuration File: Load & Save

	Functions:

	int RCLoadFromFile(char *filename)
	int RCSaveToFile(char *filename)

	---



 */

#include "../include/fio.h"
#include "../include/cfgfmt.h"

#include "../include/unvmatch.h"

#include "keymap.h"
#include "univlist.h"
#include "starchartwin.h"

#include "xsw.h"
#include "net.h"


#define MIN(a,b)	(((a) < (b)) ? (a) : (b))
#define MAX(a,b)	(((a) > (b)) ? (a) : (b))


/*
 *	Macro to parse standard color string and set
 *	values into WColorStruct c.
 */
static int RC_SET_COLOR(
	char *string,
	WColorStruct *c
)
{
	if((string == NULL) || (c == NULL))
	    return(-1);

	c->a = 0x00;

	return(
	    StringParseStdColor(
                string,
                &(c->r),
	        &(c->g),
	        &(c->b)
            )
	);
}


/*
 *	Load configuration from file.
 *
 *	This function should only be called once at start up, because
 *	certain resources don't get re-added like the universe entries
 *	to the universe list widget.
 */
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

        char cwd[PATH_MAX];


        getcwd(cwd, PATH_MAX);
	cwd[PATH_MAX - 1] = '\0';


	/* Check if filename exists. */
	if(filename == NULL)
	    return(-1);
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
            fprintf(stderr, "%s: Cannot open.\n", filename);
            return(-1);
        }


	/* Free all quick menu entries. */
	QMenuDeleteAllCommands();

#ifdef JS_SUPPORT
	/* Free all joystick maps. */
	JSMapDeleteAll();
#endif	/* JS_SUPPORT */


	/* Begin reading file. */
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


	    /* VersionMajor */
            if(!strcasecmp(parm, "VersionMajor"))
            {
		option.rc_version_major = atoi(val);
	    }
            /* VersionMinor */
            else if(!strcasecmp(parm, "VersionMinor"))
            {
                option.rc_version_minor = atoi(val);
            }
            /* VersionRelease */
            else if(!strcasecmp(parm, "VersionRelease"))
            {
                option.rc_version_release = atoi(val);
            }

	    /* LocalToplevelDir */
            else if(!strcasecmp(parm, "LocalToplevelDir"))
            {
                strptr3 = PathSubHome(val);
                if(strptr3 != NULL)
                {
                    strncpy(val, strptr3, CFG_VALUE_MAX);
                    val[CFG_VALUE_MAX - 1] = '\0';
                }

                if(ISPATHABSOLUTE(val))
                {
                    strncpy(dname.ltoplevel, val, PATH_MAX);
                }
                else
                {
                    strncpy(
                        dname.ltoplevel,
                        PrefixPaths(dname.home, val),
                        PATH_MAX
                    );
                }
                dname.ltoplevel[PATH_MAX - 1] = '\0';

                if(!ISPATHDIR(dname.ltoplevel))
                {
                    fprintf(stderr,
                        "%s: Line %i: %s: Cannot access directory.\n",
                        filename, lines_read, dname.ltoplevel
                    );
                }
	    }
	    /* ToplevelDir */
#ifndef __WIN32__
            else if(!strcasecmp(parm, "ToplevelDir"))
            {
		strptr3 = PathSubHome(val);
		if(strptr3 != NULL)
		{
		    strncpy(val, strptr3, CFG_VALUE_MAX);
		    val[CFG_VALUE_MAX - 1] = '\0';
		}

		if(ISPATHABSOLUTE(val))
		{
		    strncpy(dname.toplevel, val, PATH_MAX);
		}
		else
		{
		    strncpy(
			dname.toplevel,
			PrefixPaths(cwd, val),
			PATH_MAX
		    );
		}
		dname.toplevel[PATH_MAX - 1] = '\0';

		if(!ISPATHDIR(dname.toplevel))
		{
		    fprintf(stderr,
			"%s: Line %i: %s: Cannot access directory.\n",
			filename, lines_read, dname.toplevel
		    );
		}
	    }
#endif	/* __WIN32__ */
	    /* ImagesDir */
            else if(!strcasecmp(parm, "ImagesDir"))
            {
                strptr3 = PathSubHome(val);
                if(strptr3 != NULL)
                {   
                    strncpy(val, strptr3, CFG_VALUE_MAX);
                    val[CFG_VALUE_MAX - 1] = '\0';
                }

		if(ISPATHABSOLUTE(val))
		{
		    strncpy(dname.images, val, PATH_MAX);
		}
		else
		{
                    strncpy(
                        dname.images,
                        PrefixPaths(dname.toplevel, val),
                        PATH_MAX
                    );
		}
		dname.images[PATH_MAX - 1] = '\0';

                if(!ISPATHDIR(dname.images))
                {
                    fprintf(stderr,
                        "%s: Line %i: %s: Cannot access directory.\n",
                        filename, lines_read, dname.images
                    );
                }
            }
	    /* SoundsDir */
            else if(!strcasecmp(parm, "SoundsDir"))
            {
                strptr3 = PathSubHome(val);
                if(strptr3 != NULL)
                {   
                    strncpy(val, strptr3, CFG_VALUE_MAX);
                    val[CFG_VALUE_MAX - 1] = '\0';
                }

                if(ISPATHABSOLUTE(val))
                {
                    strncpy(dname.sounds, val, PATH_MAX);
                }
		else
                {
                    strncpy(
                        dname.sounds,
                        PrefixPaths(dname.toplevel, val),
                        PATH_MAX
                    );
                }
		dname.sounds[PATH_MAX - 1] = '\0';

/* Do not check if sounds dir exists or not.
                if(!ISPATHDIR(dname.sounds))
                {
                    fprintf(stderr,
                        "%s: Line %i: %s: Cannot access directory.\n",
                        filename, lines_read, dname.sounds
                    );
                }
 */
            }
            /* DownloadsDir */ 
            else if(!strcasecmp(parm, "DownloadsDir"))
            {
                strptr3 = PathSubHome(val);
                if(strptr3 != NULL)
                {
                    strncpy(val, strptr3, CFG_VALUE_MAX);
                    val[CFG_VALUE_MAX - 1] = '\0';
                }

                if(ISPATHABSOLUTE(val))
                {
                    strncpy(dname.downloads, val, PATH_MAX);
                }
                else
                {
                    strncpy(
                        dname.downloads,
                        PrefixPaths(dname.home, val),
                        PATH_MAX
                    );
                }
                dname.downloads[PATH_MAX - 1] = '\0';

                if(!ISPATHDIR(dname.downloads))
                {
                    fprintf(stderr,
                        "%s: Line %i: %s: Cannot access directory.\n",
                        filename, lines_read, dname.downloads
                    );
                }
            }
            /* StarChartDir */
            else if(!strcasecmp(parm, "StarChartDir"))
            {
                strptr3 = PathSubHome(val);
                if(strptr3 != NULL)
                {
                    strncpy(val, strptr3, CFG_VALUE_MAX);
                    val[CFG_VALUE_MAX - 1] = '\0';
                }

                if(ISPATHABSOLUTE(val))
                {
                    strncpy(dname.starchart, val, PATH_MAX);
                }
                else
                {
		    strncpy(
                        dname.starchart,
                        PrefixPaths(dname.home, val),
                        PATH_MAX
                    );
                }
                dname.starchart[PATH_MAX - 1] = '\0';

                if(!ISPATHDIR(dname.starchart))   
                {
                    fprintf(stderr,
                        "%s: Line %i: %s: Cannot access directory.\n",
                        filename, lines_read, dname.starchart
                    );
                }
            }
            /* EtcDir */
#ifndef __WIN32__
            else if(!strcasecmp(parm, "EtcDir"))
            {
                strptr3 = PathSubHome(val);
                if(strptr3 != NULL)
                {   
                    strncpy(val, strptr3, CFG_VALUE_MAX);
                    val[CFG_VALUE_MAX - 1] = '\0';
                }

                if(ISPATHABSOLUTE(val))
                {
                    strncpy(dname.etc, val, PATH_MAX);
                }
                else
                {
                    strncpy(
                        dname.etc,
                        PrefixPaths(dname.toplevel, val),
                        PATH_MAX
                    );
                }
		dname.etc[PATH_MAX - 1] = '\0';

                if(!ISPATHDIR(dname.etc))
                {
                    fprintf(stderr,
                        "%s: Line %i: %s: Cannot access directory.\n",
                        filename, lines_read, dname.etc
                    );
                }   
            }
#endif	/* __WIN32__ */
	    /* NetDeviceMaxLoad */
	    else if(!strcasecmp(parm, "NetDeviceMaxLoad"))
	    {
		loadstat.net_load_max = atol(val);
                if(loadstat.net_load_max < 800)
                {
                    fprintf(stderr,
    "%s: Line %i: Error: Max net device load setting %ld, is too low.\n",
                            filename,
                            lines_read,
                            loadstat.net_load_max
                    );
		    /* Set to 800. */
		    loadstat.net_load_max = 800;
                }
	    }
	    /* NetIntervalAutoTune */
	    else if(!strcasecmp(parm, "NetIntervalAutoTune"))
	    {
		auto_interval_tune.state = StringIsYes(val);
	    }
            /* NetIntervalTuneInterval */
            else if(!strcasecmp(parm, "NetIntervalTuneInterval"))
            {
		auto_interval_tune.interval = atol(val);
		if(auto_interval_tune.interval < 1000)
		{
                    fprintf(stderr,
 "%s: Line %i: Warning: Auto net interval tune interval %ld ms, is very short.\n",
                        filename,
                        lines_read,
                        auto_interval_tune.interval
                    );
		}
            }
            /* DefaultServerAddress */
            else if(!strcasecmp(parm, "DefaultServerAddress"))
            {
		if(net_parms.is_address_set == 0)
		{
		    strncpy(net_parms.address, val,
			MAX_URL_LEN);
		}
		net_parms.address[MAX_URL_LEN - 1] = '\0';
            }
            /* DefaultServerPort */
            else if(!strcasecmp(parm, "DefaultServerPort"))
            {
                if(net_parms.is_address_set == 0)
                {
		    net_parms.port = atoi(val);
		}
	        if(net_parms.port < 0)
		    net_parms.port = 0;
            }
            /* DefaultLoginName */
            else if(!strcasecmp(parm, "DefaultLoginName"))
            {
		if(net_parms.is_address_set == 0)
		{
                    strncpy(net_parms.login_name, val, XSW_OBJ_NAME_MAX);
                    net_parms.login_name[XSW_OBJ_NAME_MAX - 1] = '\0';

		    if(!UNVIsObjectNameIndex(net_parms.login_name))
		    {
		        switch(DBValidateObjectName(net_parms.login_name))
		        {
                          case 3:
                            fprintf(stderr,
 "%s: Line %i: Warning: Login name `%s' contains invalid character(s).\n",
				filename,  
                                lines_read,
                                net_parms.login_name
                            );
                            break;

                          case 2:
                            fprintf(stderr,
 "%s: Line %i: Warning: Login name `%s' is too short.\n",
                                filename,
                                lines_read,
                                net_parms.login_name
                            );
                            break;

                          case 1:
                            fprintf(stderr,
 "%s: Line %i: Warning: Login name `%s' is too long.\n",
                                filename,
                                lines_read,
                                net_parms.login_name
                            );
			    break;

			  case 0:
			    /* Check passed. */
			    break;

		          default:
                            fprintf(stderr,
 "%s: Line %i: Warning: Invalid login name `%s'.\n",
				filename,
				lines_read,
		                net_parms.login_name
		            );
		            break;
			}
		    }
		}
            }
            /* DefaultLoginPassword */
            else if(!strcasecmp(parm, "DefaultLoginPassword"))
            {
                if(net_parms.is_address_set == 0)
                {
                    strncpy(
			net_parms.login_password,
			val,
		        XSW_OBJ_PASSWORD_MAX
		    );
                    net_parms.login_password[XSW_OBJ_PASSWORD_MAX - 1] = '\0';
		}
            }
	    /* ThrottleScopeMode */
	    else if(!strcasecmp(parm, "ThrottleScopeMode"))
	    {
                if(strcasepfx(val, "i"))
		    option.throttle_mode = THROTTLE_MODE_INCREMENTAL;
		else if(strcasepfx(val, "b"))
		    option.throttle_mode = THROTTLE_MODE_BIDIRECTIONAL;
		else
		    option.throttle_mode = THROTTLE_MODE_NORMAL;
	    }
            /* Local updates. */
            else if(!strcasecmp(parm, "LocalUpdates"))
            {
                option.local_updates = StringIsYes(val);
            }
            /* AutoMapEconomyWindow */
            else if(!strcasecmp(parm, "AutoMapEconomyWindow"))
            {
                option.auto_map_eco_win = StringIsYes(val);
            }
	    /* AutoViewscreenZoom */
            else if(!strcasecmp(parm, "AutoViewscreenZoom"))
	    {
		option.auto_zoom = StringIsYes(val);
            }
	    /* ScannerOrientation */
	    else if(!strcasecmp(parm, "ScannerOrientation"))
	    {
		if(strcasepfx(val, "vessel"))
		{
		    option.def_scanner_orient = SCANNER_ORIENT_LOCAL;
		}
		else if(strcasepfx(val, "galactic_core"))
		{
		    option.def_scanner_orient = SCANNER_ORIENT_GC;
		}
		else
		{
		    fprintf(stderr,
"%s: Line %i: ScannerOrientation: Bad value `%s'.\n\
Available values are: galactic_core  vessel\n",
			filename,
			lines_read,
			val
		    );
		}
	    }
            /* ViewScreenMarks (was DisplayHeadingArrow) */
            else if(!strcasecmp(parm, "ViewScreenMarks") ||
                    !strcasecmp(parm, "DisplayHeadingArrow")
	    )
            {
		option.show_viewscreen_marks = StringIsYes(val);
            }
            /* ViewScreenLabels */
            else if(!strcasecmp(parm, "ViewScreenLabels"))
            {
		option.show_viewscreen_labels = atoi(val);

		if(option.show_viewscreen_labels < 0)
		    option.show_viewscreen_labels = 0;
                else if(option.show_viewscreen_labels > 3)
		    option.show_viewscreen_labels = 3;
            }
	    /* ShowLensFlares */
            else if(!strcasecmp(parm, "ShowLensFlares"))
            {
                option.show_lens_flares = StringIsYes(val);
            }
            /* ShowStrobeGlow */
            else if(!strcasecmp(parm, "ShowStrobeGlow"))
            {
                option.show_strobe_glow = StringIsYes(val);
            }
            /* ShowNebulaGlow */
            else if(!strcasecmp(parm, "ShowNebulaGlow"))
            {
                option.show_nebula_glow = StringIsYes(val);
            }
	    /* FormalNameLabels */
	    else if(!strcasecmp(parm, "FormalNameLabels"))
	    {
		option.show_formal_label = atoi(val);

                if(option.show_formal_label < 0)
                    option.show_formal_label = 0;
                else if(option.show_formal_label > 2)
                    option.show_formal_label = 2;
	    }
	    /* NotifyScannerContacts */
	    else if(!strcasecmp(parm, "NotifyScannerContacts"))
            {
                option.notify_scanner_contacts = StringIsYes(val);
            }
	    /* ShowNetErrors */
            else if(!strcasecmp(parm, "ShowNetErrors"))
            {
                option.show_net_errors = StringIsYes(val);
            }
            /* ShowServerErrors */
            else if(!strcasecmp(parm, "ShowServerErrors"))
            {
                option.show_server_errors = StringIsYes(val);
            }
	    /* SoundServerType */
            else if(!strcasecmp(parm, "SoundServerType"))
            {
                sound.server_type = atoi(val);

                if(sound.server_type < SNDSERV_TYPE_NONE)
                    sound.server_type = SNDSERV_TYPE_NONE;
            }
	    /* Sounds */
            else if(!strcasecmp(parm, "Sounds"))
            {
		option.sounds = atoi(val);

		if(option.sounds < XSW_SOUNDS_NONE)
		    option.sounds = XSW_SOUNDS_NONE;
	    }
            /* Music */
            else if(!strcasecmp(parm, "Music"))
            {
                option.music = StringIsYes(val);
            }
	    /* ControlType */
            else if(!strcasecmp(parm, "ControlType"))
            {
		if(!option.cmd_line_set_controller)
		{
		    /* Pointer. */
                    if(strcasepfx(val, "p"))
                    {
                        option.controller = CONTROLLER_POINTER;
                    }
		    /* Keyboard. */
                    else if(strcasepfx(val, "k"))
                    {
                        option.controller = CONTROLLER_KEYBOARD;
                    }
#ifdef JS_SUPPORT
		    /* Joystick. */
                    else if(strcasepfx(val, "j"))
		    {
		        option.controller = CONTROLLER_JOYSTICK;
		    }
#endif /* JS_SUPPORT */
		    else
		    {
		        fprintf(stderr,
                            "%s: Line %i: ControlType: Bad value `%s'.\n",
                            filename,
                            lines_read,
                            val
                        );
		    }
                    /*   Change over to and initialize game controller.
                     *   Global option.controller will be re-updated by this
                     *   call.
                     */
                    GCtlInit(option.controller);
		}
            }
	    /* Units */
            else if(!strcasecmp(parm, "Units"))
            {
		if(strcasepfx(val, "XSW"))
		    option.units = XSW_UNITS_XSW;
		else if(strcasepfx(val, "AstroMetric") ||
                        strcasepfx(val, "Metric")
		)
                    option.units = XSW_UNITS_METRIC;
                else if(strcasepfx(val, "English"))
                    option.units = XSW_UNITS_ENGLISH;
		else
		    fprintf(stderr,
 "%s: Line %i: Warning: Unsupported units `%s'.\n",
                        filename,
                        lines_read,
			val
                    );
	    }
	    /* AsyncImageLoading */
	    else if(!strcasecmp(parm, "AsyncImageLoading"))
            {
		option.async_image_loading = StringIsYes(val);
	    }
            /* AsyncImageLoadPixels */
            else if(!strcasecmp(parm, "AsyncImageLoadPixels"))
            {
		option.async_image_pixels = atoi(val);
            } 
	    /* SoundServerStartCommand */
	    else if(!strcasecmp(parm, "SoundServerStartCommand"))
	    {
                strptr3 = PathSubHome(val);
                if(strptr3 != NULL)
                {
                    strncpy(val, strptr3, CFG_VALUE_MAX);
                    val[CFG_VALUE_MAX - 1] = '\0';
                }

		if(!ISPATHABSOLUTE(val))
		{
		    fprintf(stderr,
 "%s: Line %i: Error: Sound server location must be an absolute path.\n",
			filename,
			lines_read
		    );
                    continue;
		}
		strncpy(sound.start_cmd, val, PATH_MAX + NAME_MAX);
		sound.start_cmd[PATH_MAX + NAME_MAX - 1] = '\0';

                if(stat(sound.start_cmd, &stat_buf) &&
                   (option.sounds > XSW_SOUNDS_NONE)
		)
                {
                    fprintf(stderr,
                        "%s: Line %i: Warning: %s: No such file.\n",
                        filename,
                        lines_read,
                        sound.start_cmd
                    );
                }
	    }

#ifdef JS_SUPPORT
            /* JSCalibrationFile */
            else if(!strcasecmp(parm, "JSCalibrationFile") ||
                    !strcasecmp(parm, "JSCaliberationFile")	/* Typo. */
	    )
            {
                strptr3 = PathSubHome(val);
                if(strptr3 != NULL)
                {
                    strncpy(val, strptr3, CFG_VALUE_MAX);
                    val[CFG_VALUE_MAX - 1] = '\0';
                }

		if(!ISPATHABSOLUTE(val))
		{
		    if(getenv("HOME") != NULL)
		    {
			strncpy(
			    fname.js_calib,
			    PrefixPaths(getenv("HOME"), val),
			    PATH_MAX + NAME_MAX
			);
		    }
		    else
		    {
			strncpy(
                            fname.js_calib,
                            PrefixPaths("/", val),
                            PATH_MAX + NAME_MAX
                        );
		    }
		}
		else
		{
		    strncpy(fname.js_calib, val, NAME_MAX + PATH_MAX);
		}
		fname.js_calib[NAME_MAX + PATH_MAX - 1] = '\0';

		if(stat(fname.js_calib, &stat_buf))
		{
		    fprintf(stderr,
                        "%s: Line %i: Warning: %s: No such file.\n",
			filename,
			lines_read,
			fname.js_calib
		    );
		}
            }
#endif /* JS_SUPPORT */
	    /* MainPageFile */
	    else if(!strcasecmp(parm, "MainPageFile"))
	    {
                strptr3 = PathSubHome(val);
                if(strptr3 != NULL)
                {
                    strncpy(val, strptr3, CFG_VALUE_MAX);
                    val[CFG_VALUE_MAX - 1] = '\0';
                }

                if(!ISPATHABSOLUTE(val))
                {
                    strncpy(
                        fname.main_page,
                        PrefixPaths(dname.etc, val),
                        PATH_MAX + NAME_MAX  
                    );
                }
                else
                {
                    strncpy(fname.main_page, val, NAME_MAX + PATH_MAX);
                }
                fname.main_page[NAME_MAX + PATH_MAX - 1] = '\0';
                if(stat(fname.main_page, &stat_buf))
                {
                    fprintf(stderr,
                        "%s: Line %i: Warning: %s: No such file.\n",
                        filename,
                        lines_read,
                        fname.main_page
                    );
                }
	    }
            /* DestroyedPageFile */
            else if(!strcasecmp(parm, "DestroyedPageFile"))
            {
                strptr3 = PathSubHome(val);
                if(strptr3 != NULL)
                {
                    strncpy(val, strptr3, CFG_VALUE_MAX);
                    val[CFG_VALUE_MAX - 1] = '\0';
                }

                if(!ISPATHABSOLUTE(val))
                {
                    strncpy(
                        fname.destroyed_page,
                        PrefixPaths(dname.etc, val),
                        PATH_MAX + NAME_MAX
                    );
                }
                else
                {
                    strncpy(fname.destroyed_page, val, NAME_MAX + PATH_MAX);
                }
                fname.destroyed_page[NAME_MAX + PATH_MAX - 1] = '\0';
                if(stat(fname.destroyed_page, &stat_buf))
                {
                    fprintf(stderr,
                        "%s: Line %i: Warning: %s: No such file.\n",
                        filename,
                        lines_read,
                        fname.destroyed_page
                    );
                }
            }
	    /* OCSNamesFile */
	    else if(!strcasecmp(parm, "OCSNamesFile"))
	    {
                strptr3 = PathSubHome(val);
                if(strptr3 != NULL)
                {
                    strncpy(val, strptr3, CFG_VALUE_MAX);
                    val[CFG_VALUE_MAX - 1] = '\0';
                }

                if(!ISPATHABSOLUTE(val))
                {
		    strncpy(
			fname.ocsn,
			PrefixPaths(dname.etc, val),
			PATH_MAX + NAME_MAX
                    );
                }   
                else
                {
                    strncpy(fname.ocsn, val, NAME_MAX + PATH_MAX);
                }
		fname.ocsn[NAME_MAX + PATH_MAX - 1] = '\0';
                if(stat(fname.ocsn, &stat_buf))
                {
                    fprintf(stderr,
                        "%s: Line %i: Warning: %s: No such file.\n",
                        filename,
                        lines_read,
                        fname.ocsn
                    );
                }
            }
            /* ISRFile */
            else if(!strcasecmp(parm, "ISRFile"))
            {
                strptr3 = PathSubHome(val);
                if(strptr3 != NULL)
                {
                    strncpy(val, strptr3, CFG_VALUE_MAX);
                    val[CFG_VALUE_MAX - 1] = '\0';
                }

                if(!ISPATHABSOLUTE(val))
                {
                    strncpy(
                        fname.isr,
                        PrefixPaths(dname.images, val),
                        PATH_MAX + NAME_MAX
                    );
                }
                else
                {   
                    strncpy(fname.isr, val, NAME_MAX + PATH_MAX);
                }
		fname.isr[NAME_MAX + PATH_MAX - 1] = '\0';

                if(stat(fname.isr, &stat_buf))
                {
                    fprintf(stderr,
                        "%s: Line %i: Warning: %s: No such file.\n",
                        filename,
                        lines_read,
                        fname.isr
                    );
                }
            }
            /* SoundSchemeFile */
            else if(!strcasecmp(parm, "SoundSchemeFile"))
            {
                strptr3 = PathSubHome(val);
                if(strptr3 != NULL)
                {
                    strncpy(val, strptr3, CFG_VALUE_MAX);
                    val[CFG_VALUE_MAX - 1] = '\0';
                }

                if(!ISPATHABSOLUTE(val))
                {
                    strncpy(
                        fname.sound_scheme,
                        PrefixPaths(dname.sounds, val),
                        PATH_MAX + NAME_MAX
                    );
                }
                else  
                {
                    strncpy(fname.sound_scheme, val, NAME_MAX + PATH_MAX);
                }
		fname.sound_scheme[NAME_MAX + PATH_MAX - 1] = '\0';

                if(stat(fname.sound_scheme, &stat_buf))
                {
/* It's okay if the sound scheme file does not exist.
                    fprintf(stderr,
                        "%s: Line %i: Warning: %s: No such file.\n",
                        filename,
                        lines_read,
                        fname.sound_scheme
                    );
 */
                }
            }
            /* SoundServerConnectionArgument */
            else if(!strcasecmp(parm, "SoundServerConnectionArgument"))
            {
		strncpy(sound.con_arg, val, MAX_URL_LEN);
		sound.con_arg[MAX_URL_LEN - 1] = '\0';
            }
	    /* LogFile */
	    else if(!strcasecmp(parm, "LogFile"))
	    {
                strptr3 = PathSubHome(val);
                if(strptr3 != NULL)
                {
                    strncpy(val, strptr3, CFG_VALUE_MAX);
                    val[CFG_VALUE_MAX - 1] = '\0';
                }

                if(ISPATHABSOLUTE(val))
		    strncpy(fname.log, val, PATH_MAX + NAME_MAX);
                else
                    strncpy(
                        fname.log,
                        PrefixPaths(dname.ltoplevel, val),
                        PATH_MAX + NAME_MAX
                    );
                fname.log[NAME_MAX + PATH_MAX - 1] = '\0';
	    }
	    /* UniverseListFile */
            else if(!strcasecmp(parm, "UniverseListFile"))
            {
                strptr3 = PathSubHome(val);
                if(strptr3 != NULL)
                {
                    strncpy(val, strptr3, CFG_VALUE_MAX);
                    val[CFG_VALUE_MAX - 1] = '\0';
                }

                if(ISPATHABSOLUTE(val))
		    strncpy(fname.universe_list, val, PATH_MAX + NAME_MAX);
		else
                    strncpy(
                        fname.universe_list,
                        PrefixPaths(dname.ltoplevel, val),
                        PATH_MAX + NAME_MAX
                    );
                fname.universe_list[NAME_MAX + PATH_MAX - 1] = '\0';
            }


	    /* *************************************************** */

            /* ColorStandardText */
            else if(!strcasecmp(parm, "ColorStandardText"))
            {
                RC_SET_COLOR(
                    val,
                    &xsw_color.standard_text_cv
                );
            }
            /* ColorBoldText */
            else if(!strcasecmp(parm, "ColorBoldText")) 
            {
                RC_SET_COLOR(
                    val,
                    &xsw_color.bold_text_cv
                );
            }
            /* ColorWithdrawnText */
            else if(!strcasecmp(parm, "ColorWithdrawnText"))
            {
                RC_SET_COLOR(
                    val,
                    &xsw_color.withdrawn_text_cv
                );
            }


            /* ColorBPStandardText */
            else if(!strcasecmp(parm, "ColorBPStandardText"))
            {
                RC_SET_COLOR(
                    val,
                    &xsw_color.bp_standard_text_cv
                );
            }
            /* ColorBPBoldText */
            else if(!strcasecmp(parm, "ColorBPBoldText"))
            {
                RC_SET_COLOR(
                    val,
                    &xsw_color.bp_bold_text_cv
                );  
            }
            /* ColorBPWithdrawnText */
            else if(!strcasecmp(parm, "ColorBPWithdrawnText"))
            {
                RC_SET_COLOR(
                    val,
                    &xsw_color.bp_withdrawn_text_cv
                );
            }
            /* ColorBPLightOutline */
            else if(!strcasecmp(parm, "ColorBPLightOutline"))
            {
                RC_SET_COLOR(
                    val,
                    &xsw_color.bp_light_outline_cv
                );  
            }
            /* ColorBPNormalOutline */
            else if(!strcasecmp(parm, "ColorBPNormalOutline"))
            {
                RC_SET_COLOR(
                    val,
                    &xsw_color.bp_normal_outline_cv
                );
            }
            /* ColorBPDarkOutline */
            else if(!strcasecmp(parm, "ColorBPDarkOutline"))
            {
                RC_SET_COLOR(
                    val,
                    &xsw_color.bp_dark_outline_cv
                );  
            }


            /* ColorBPWarning */
            else if(!strcasecmp(parm, "ColorBPWarning"))
            {
                RC_SET_COLOR(
                    val,
                    &xsw_color.bp_warning_cv
                );  
            }
            /* ColorBPDanger */
            else if(!strcasecmp(parm, "ColorBPDanger"))
            {
                RC_SET_COLOR(
                    val,
                    &xsw_color.bp_danger_cv
                );
            }
            /* ColorBPCritical */
            else if(!strcasecmp(parm, "ColorBPCritical"))
            {
                RC_SET_COLOR(
                    val,
                    &xsw_color.bp_critical_cv
                );
            }

            /* ColorBPFriendly */
            else if(!strcasecmp(parm, "ColorBPFriendly"))
            {
                RC_SET_COLOR(
                    val,
                    &xsw_color.bp_friendly_cv
                );
            }
            /* ColorBPUnknown */
            else if(!strcasecmp(parm, "ColorBPUnknown"))
            {
                RC_SET_COLOR(
                    val,
                    &xsw_color.bp_unknown_cv
                );
            }
            /* ColorBPHostile */
            else if(!strcasecmp(parm, "ColorBPHostile"))
            {
                RC_SET_COLOR(
                    val,
                    &xsw_color.bp_hostile_cv
                );
            }

            /* ColorBPHull */
            else if(!strcasecmp(parm, "ColorBPHull"))
            {
                RC_SET_COLOR(
                    val,
                    &xsw_color.bpol_hull_cv
                );  
            }
            /* ColorBPPower */
            else if(!strcasecmp(parm, "ColorBPPower"))
            {
                RC_SET_COLOR(
                    val,
                    &xsw_color.bpol_power_cv
                );
            }
            /* ColorBPVis */
            else if(!strcasecmp(parm, "ColorBPVis"))
            {
                RC_SET_COLOR(
                    val,
                    &xsw_color.bpol_vis_cv
                );
            }
            /* ColorBPShields */
            else if(!strcasecmp(parm, "ColorBPShields"))
            {
                RC_SET_COLOR(
                    val,
                    &xsw_color.bpol_shields_cv
                );
            }
            /* ColorBPDmgCtl */
            else if(!strcasecmp(parm, "ColorBPDmgCtl"))
            {
                RC_SET_COLOR(
                    val,
                    &xsw_color.bpol_dmgctl_cv
                );
            }
            /* ColorBPThrottle */
            else if(!strcasecmp(parm, "ColorBPThrottle"))
            {
                RC_SET_COLOR(
                    val,
                    &xsw_color.bpol_throttle_cv
                );
            }
            /* ColorBPThrottleRev */
            else if(!strcasecmp(parm, "ColorBPThrottleRev"))
            {
                RC_SET_COLOR(
                    val,
                    &xsw_color.bpol_throttle_rev_cv
                );
            }

	    /* ColorVSLabelBg */
            else if(!strcasecmp(parm, "ColorVSLabelBg"))
            {
                RC_SET_COLOR(
                    val,
                    &xsw_color.vs_label_bg_cv
                );
            }
	    /* ColorVSLabelFg */
            else if(!strcasecmp(parm, "ColorVSLabelFg"))
            {
                RC_SET_COLOR(
                    val,
                    &xsw_color.vs_label_fg_cv
                );
            }

            /* ColorScMarkUnknown */
            else if(!strcasecmp(parm, "ColorScMarkUnknown"))
            {
                RC_SET_COLOR(
                    val,
                    &xsw_color.scmark_unknown_cv
                );
            }
            /* ColorScMarkLocked */
            else if(!strcasecmp(parm, "ColorScMarkLocked"))
            {
                RC_SET_COLOR(
                    val,
                    &xsw_color.scmark_locked_cv
                );
            }
            /* ColorScMarkWeapon */
            else if(!strcasecmp(parm, "ColorScMarkWeapon"))
            {
                RC_SET_COLOR(
                    val,
                    &xsw_color.scmark_weapon_cv
                );
            }
            /* ColorScMarkHome */
            else if(!strcasecmp(parm, "ColorScMarkHome"))
            {
                RC_SET_COLOR(
                    val,
                    &xsw_color.scmark_home_cv
                );
            }
            /* ColorScMarkArea */
            else if(!strcasecmp(parm, "ColorScMarkArea"))
            {
                RC_SET_COLOR(
                    val,
                    &xsw_color.scmark_area_cv
                );
            }


            /* ColorKeymapQueryBg */
            else if(!strcasecmp(parm, "ColorKeymapQueryBg"))
            {
                RC_SET_COLOR(
                    val,
                    &xsw_color.keymap_query_bg_cv
                );
            }
            /* ColorKeymapQueryFg */
            else if(!strcasecmp(parm, "ColorKeymapQueryFg"))
            {
                RC_SET_COLOR(
                    val,
                    &xsw_color.keymap_query_fg_cv
                );
            }


	    /* ColorHeadingArrow */
            else if(!strcasecmp(parm, "ColorHeadingArrow"))
	    {
		RC_SET_COLOR(
		    val,
		    &xsw_color.heading_arrow
		);
	    }
            /* ColorLockArrow */
            else if(!strcasecmp(parm, "ColorLockArrow"))
            {
		RC_SET_COLOR(
                    val,
                    &xsw_color.lock_arrow
		);
            }
            /* ColorLockCursor */
            else if(!strcasecmp(parm, "ColorLockCursor"))
            {
                RC_SET_COLOR(
		    val,
		    &xsw_color.lock_cursor
                );
            }
            /* ColorStreamGreen */
            else if(!strcasecmp(parm, "ColorStreamGreen"))
            {
                RC_SET_COLOR(
                    val,
                    &xsw_color.stream_green
                );
            }
            /* ColorStreamOrange */
            else if(!strcasecmp(parm, "ColorStreamOrange"))
            {
                RC_SET_COLOR(
                    val,
                    &xsw_color.stream_orange
		);
            }
            /* ColorStreamPurple */
            else if(!strcasecmp(parm, "ColorStreamPurple"))
            {
                RC_SET_COLOR(
                    val,
                    &xsw_color.stream_purple
                );
            }
            /* ColorStreamYellow */
            else if(!strcasecmp(parm, "ColorStreamYellow"))
            {
                RC_SET_COLOR(
                    val,
                    &xsw_color.stream_yellow
                );
            }
            /* ColorShieldBlue */
            else if(!strcasecmp(parm, "ColorShieldBlue"))
            {
                RC_SET_COLOR(
                    val,
                    &xsw_color.shield_blue
                );
            }
	    /* ColorVisibilityMarker */
	    else if(!strcasecmp(parm, "ColorVisibilityMarker"))
            {
                RC_SET_COLOR(
                    val,
                    &xsw_color.visibility_marker
                );
            }
            /* ColorStarGlow */
            else if(!strcasecmp(parm, "ColorStarGlow"))
            {
                RC_SET_COLOR(
                    val,
                    &xsw_color.star_glow
                );
            }

            /* ColorChartBackground */
            else if(!strcasecmp(parm, "ColorChartBackground"))
            {
                RC_SET_COLOR(
                    val,
                    &xsw_color.chart_bg_cv
                );
            }
            /* ColorChartGrid */
            else if(!strcasecmp(parm, "ColorChartGrid"))
            {
                RC_SET_COLOR(
                    val,
                    &xsw_color.chart_grid_cv
                );
            }
            /* ColorChartSectorGrid */
            else if(!strcasecmp(parm, "ColorChartSectorGrid"))
            {
                RC_SET_COLOR(
                    val,
                    &xsw_color.chart_sector_grid_cv
                );
            }
            /* ColorChartCrossHairs */
            else if(!strcasecmp(parm, "ColorChartCrossHairs"))
            {
                RC_SET_COLOR(
                    val,
                    &xsw_color.chart_cross_hairs_cv
                );
            }


            /* **************************************************** */
	    /* BeginKeyMap */
            else if(!strcasecmp(parm, "BeginKeyMap"))
            {
		/* Begin reading keymap entries. */
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


		    /* Match keymap parm by conical name. */
		    for(i = 0, status = 0; i < (int)TOTAL_XSW_KEYMAPS; i++)
		    {
			if(!strcasecmp(parm, xsw_keymap_name[i]))
			{
			    xsw_keymap[i].keycode = atoi(val);
			    status = 1;
			}
		    }
		    if(status)
			continue;

                    /* EndKeyMap */
                    if(!strcasecmp(parm, "EndKeyMap"))
                    {
                        break;
                    }
                    /* Unknown parameter. */
                    else
                    {
                        fprintf(stderr,
                            "%s: Line %i: Unknown parameter: %s\n",
                            filename,
                            lines_read,
                            parm
                        );
                    }
		}
	    }
	    /* **************************************************** */
	    /* Universe list window position. */
            /* AutoMapUniverseListWindow */
            else if(!strcasecmp(parm, "AutoMapUniverseListWindow"))
            {
                option.auto_map_univ_list_win = StringIsYes(val);
            }
	    /* UnivListWinX */
	    else if(!strcasecmp(parm, "UnivListWinX"))
	    {
		univ_list_win.x = atoi(val);
	    }
            /* UnivListWinY */
            else if(!strcasecmp(parm, "UnivListWinY"))
            {
                univ_list_win.y = atoi(val);
            }
            /* UnivListWinWidth */
            else if(!strcasecmp(parm, "UnivListWinWidth"))
            {
                univ_list_win.width = MAX(atoi(val), 100);
            }
            /* UnivListWinHeight */
            else if(!strcasecmp(parm, "UnivListWinHeight"))
            {
                univ_list_win.height = MAX(atoi(val), 100);
            }

            /* **************************************************** */
            /* Star chart window position. */
            /* StarChartWinX */
            else if(!strcasecmp(parm, "StarChartWinX"))
            {
                starchart_win.x = atoi(val);
            }
            /* StarChartWinY */
            else if(!strcasecmp(parm, "StarChartWinY"))
            {
                starchart_win.y = atoi(val);
            }
            /* StarChartWinWidth */
            else if(!strcasecmp(parm, "StarChartWinWidth"))
            {
                starchart_win.width = MAX(atoi(val), 100); 
            }
            /* StarChartWinHeight */
            else if(!strcasecmp(parm, "StarChartWinHeight"))
            {
                starchart_win.height = MAX(atoi(val), 100);
            }
	    /* ******************************************************** */
	    /* BeginQuickMenuCommand */
            else if(!strcasecmp(parm, "BeginQuickMenuCommand") ||
		    !strcasecmp(parm, "BeginQuickMenuItem")
	    )
            {
		qmenu_command_struct *cmd_ptr;


                /* Increment total number of commands. */
		n = total_qmenu_commands;
                total_qmenu_commands++;

		/* Reallocate menu item pointer. */
		qmenu_command = (qmenu_command_struct **)realloc(
		    qmenu_command,
		    total_qmenu_commands * sizeof(qmenu_command_struct *)
		);
		if(qmenu_command == NULL)
		{
                    total_qmenu_commands = 0;
		    continue;
		}

		/* Allocate a new command. */
		qmenu_command[n] = (qmenu_command_struct *)calloc(
		    1,
		    sizeof(qmenu_command_struct)
		);
                if(qmenu_command[n] == NULL)
                {
                    total_qmenu_commands--;
                    continue;
                }
		cmd_ptr = qmenu_command[n];


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


                    /* Name */
                    if(!strcasecmp(parm, "Name"))
                    {
			free(cmd_ptr->name);
			cmd_ptr->name = StringCopyAlloc(val);
                    }
                    /* Type */
                    else if(!strcasecmp(parm, "Type"))
                    {
			cmd_ptr->type = atoi(val);
                    }
		    /* ActionCode */
                    else if(!strcasecmp(parm, "ActionCode"))
                    {
                        cmd_ptr->action = atol(val);
                    }

                    /* EndQuickMenuItem */
                    else if(!strcasecmp(parm, "EndQuickMenuCommand") ||
                            !strcasecmp(parm, "EndQuickMenuItem")
		    )
                    {
                        break;
                    }

                    /* Unknown parameter. */
                    else
                    {
                        fprintf(stderr,
                            "%s: Line %i: Unknown parameter: %s\n",
                            filename,
                            lines_read,
                            parm
                        );
                        continue;
                    }
                }
            }
#ifdef JS_SUPPORT
            /* ******************************************************** */
            /* BeginJSMap */
            else if(!strcasecmp(parm, "BeginJSMap"))
            {
		jsmap_struct *jsmap_ptr;


                /* Allocate more structures. */
                total_jsmaps++;

                jsmap = (jsmap_struct **)realloc(
		    jsmap,
		    total_jsmaps * sizeof(jsmap_struct *)
                );
                if(jsmap == NULL)
                {    
                    total_jsmaps = 0;
                    continue;
                }
                /* Allocate a structure. */
                jsmap[total_jsmaps - 1] = (jsmap_struct *)calloc(
		    1,
		    sizeof(jsmap_struct)
		);
                if(jsmap[total_jsmaps - 1] == NULL)
                {
		    total_jsmaps--;
		    continue;
		}

		/* Get pointer to structure. */
		jsmap_ptr = jsmap[total_jsmaps - 1];

		/* Reset needed jsmap values. */
		jsmap_ptr->jsd.fd = -1;


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

                    /* DeviceName */
                    if(!strcasecmp(parm, "DeviceName"))
                    {
                        jsmap_ptr->device_name = StringCopyAlloc(val);
                    }
                    /* BeginAxis */
                    else if(!strcasecmp(parm, "BeginAxis"))
                    {
			n = jsmap_ptr->total_axises;
                        jsmap_ptr->total_axises++;

                        jsmap_ptr->axis = (jsmap_axis_struct **)realloc(
                            jsmap_ptr->axis,
                            jsmap_ptr->total_axises * sizeof(jsmap_axis_struct *)
			);
                        if(jsmap_ptr->axis == NULL)
                        {
                            jsmap_ptr->total_axises = 0;
                            continue;
                        }

                        jsmap_ptr->axis[n] = (jsmap_axis_struct *)calloc(
			    1,
			    sizeof(jsmap_axis_struct)
			);
			if(jsmap_ptr->axis[n] == NULL)
			{
			    jsmap_ptr->total_axises--;
			    continue;
			}

			/* Reset axis values. */
			jsmap_ptr->axis[n]->op_code = JSMAP_AXIS_OP_NONE;

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

                            /* OpCode */
                            if(!strcasecmp(parm, "OpCode"))
                            {
				jsmap_ptr->axis[n]->op_code = atoi(val);
                            }
                            /* EndAxis */
                            else if(!strcasecmp(parm, "EndAxis"))
                            {
                                break;
                            }
                            /* Unknown parameter. */
                            else
                            {
                                fprintf(stderr,
                                    "%s: Line %i: Unknown parameter: %s\n",
                                    filename,
                                    lines_read,
                                    parm
                                );
                            }
			}
		    }
                    /* BeginButton */
                    else if(!strcasecmp(parm, "BeginButton"))
                    {
                        n = jsmap_ptr->total_buttons;
                        jsmap_ptr->total_buttons++;

                        jsmap_ptr->button = (jsmap_button_struct **)realloc(
                            jsmap_ptr->button,
                            jsmap_ptr->total_buttons * sizeof(jsmap_button_struct *)
                        );  
                        if(jsmap_ptr->button == NULL)
                        {
                            jsmap_ptr->total_buttons = 0;
                            continue;
                        }

                        jsmap_ptr->button[n] = (jsmap_button_struct *)calloc(
                            1,
                            sizeof(jsmap_button_struct)
                        );
                        if(jsmap_ptr->button[n] == NULL)
                        {
                            jsmap_ptr->total_buttons--;
                            continue;
                        }

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


                            /* Keycode */
                            if(!strcasecmp(parm, "Keycode"))
                            {
                                jsmap_ptr->button[n]->keycode = atoi(val);
                            }
                            /* EndButton */
                            else if(!strcasecmp(parm, "EndButton"))
                            {
                                break;
                            }
                            /* Unknown parameter. */
                            else
                            {
                                fprintf(stderr,
                                    "%s: Line %i: Unknown parameter: %s\n",
                                    filename,
                                    lines_read,
                                    parm
                                );
                            }
                        }
                    }
                    /* EndJSMap */
                    else if(!strcasecmp(parm, "EndJSMap"))
                    {
                        break;
                    }                
                    /* Unknown parameter. */
                    else
                    {
                        fprintf(stderr,
                            "%s: Line %i: Unknown parameter: %s\n",
                            filename,
                            lines_read,
                            parm
                        );
                    }
		}
	    }
#endif	/* JS_SUPPORT */
            /* Unknown parameter. */
            else
            {
                fprintf(stderr, "%s: Line %i: Unknown parameter: %s\n",
                    filename,
                    lines_read,
                    parm
                );
            }
	}


	/* Close the file. */
	fclose(fp);
	fp = NULL;


	/* Warn if configuration being used is older than version of
	 * this program (ignore release version number).
	 */
	if(option.rc_version_major < PROG_VERSION_MAJOR)
	    status = -1;
	else if(option.rc_version_minor < PROG_VERSION_MINOR)
	    status = -1;
	else
	    status = 0;

	if(status != 0)
	{
	    fprintf(stderr,
"%s: Warning: File version %i.%i is out of date with program version %i.%i.\n",
		filename,
		option.rc_version_major,
		option.rc_version_minor,
		PROG_VERSION_MAJOR,
		PROG_VERSION_MINOR
	    );

	    fprintf(stderr,
"\n\
An older configuration file has been detected, the contents of the file\n\
may be out of date and certain features may be unspecified.\n\
The program can continue running, however you are strongly encouraged to\n\
use an up to date configuration file.\n"
	    );
	}


	return(0);
}


/*
 *	Prints `parameter = value' string by the given values
 *	in label and WColorStruct c.
 */
void RC_PRINT_COLOR_STRING(
	FILE *fp,
	char *label,
	WColorStruct c
)
{
	if((fp == NULL) ||
	   (label == NULL)
	)
	    return;

	fprintf(fp, "%s = #%.2x%.2x%.2x\n",
	    label,
	    c.r,
	    c.g,
	    c.b
	);

	return;
}


/*
 *	Saves configuration to filename.  Returns 0 on success
 *	and -1 on error.
 */
int RCSaveToFile(char *filename)
{
	int i, n;
	FILE *fp;

        char tmp_path[PATH_MAX + NAME_MAX];
        char cwd[PATH_MAX];


	/* Get current working dir. */
	getcwd(cwd, PATH_MAX);
	cwd[PATH_MAX - 1] = '\0';


        /* Open filename for writing. */
        fp = fopen(filename, "w");
        if(fp == NULL)
        {
            fprintf(stderr,
		"%s: Cannot open for writing.\n",
		filename
	    );
            return(-1);
        }


	/* Header. */
	fprintf(fp,
            "# %s Configuration File\n",
	    PROG_NAME
	);

        fprintf(fp,
            "# Version: %s\n",
	    PROG_VERSION
        );

        fprintf(fp,   
	    "# You may manually edit this file.\n#\n\n\n"
        );
	

	/* Versions. */
        fprintf(fp,
            "# This configuration file version:\n\n"
        );

        fprintf(fp,
            "VersionMajor = %i\n",
	    PROG_VERSION_MAJOR
        );

        fprintf(fp,
            "VersionMinor = %i\n",
	    PROG_VERSION_MINOR
        );

        fprintf(fp,
            "VersionRelease = %i\n",
            PROG_VERSION_RELEASE
        );

        fprintf(fp, "\n\n");


        /* ********************************************************** */
	/* Directories and files. */
        fprintf(fp,
            "# Directories and files:\n\n"
        );

	/* LocalToplevelDir */
        strncpy(tmp_path, dname.ltoplevel, PATH_MAX + NAME_MAX);
        tmp_path[PATH_MAX + NAME_MAX - 1] = '\0';
        StripParentPath(tmp_path, dname.home);
        fprintf(fp, "LocalToplevelDir = %s\n", tmp_path);

	/* ToplevelDir */
	if(ISPATHABSOLUTE(dname.toplevel))
	{
            fprintf(fp, "ToplevelDir = %s\n", dname.toplevel);
	}
	else
	{
	    /* ToplevelDir must be absolute, use default if not. */
	    fprintf(fp, "ToplevelDir = %s\n", DEF_XSW_TOPLEVEL_DIR);
	}

        /* EtcDir */
        strncpy(tmp_path, dname.etc, PATH_MAX + NAME_MAX);
        tmp_path[PATH_MAX + NAME_MAX - 1] = '\0';
	StripParentPath(tmp_path, dname.toplevel);
	fprintf(fp, "EtcDir = %s\n", tmp_path);

	/* ImagesDir */
        strncpy(tmp_path, dname.images, PATH_MAX + NAME_MAX);
        tmp_path[PATH_MAX + NAME_MAX - 1] = '\0';
        StripParentPath(tmp_path, dname.toplevel);
        fprintf(fp, "ImagesDir = %s\n", tmp_path);   

	/* SoundsDir */
        strncpy(tmp_path, dname.sounds, PATH_MAX + NAME_MAX);
        tmp_path[PATH_MAX + NAME_MAX - 1] = '\0';
        StripParentPath(tmp_path, dname.toplevel);
        fprintf(fp, "SoundsDir = %s\n", tmp_path);

        /* DownloadsDir */
        strncpy(tmp_path, dname.downloads, PATH_MAX + NAME_MAX);
        tmp_path[PATH_MAX + NAME_MAX - 1] = '\0';
        StripParentPath(tmp_path, dname.home);
        fprintf(fp, "DownloadsDir = %s\n", tmp_path);

	/* StarChartDir */
        strncpy(tmp_path, dname.starchart, PATH_MAX + NAME_MAX);
        tmp_path[PATH_MAX + NAME_MAX - 1] = '\0';
        StripParentPath(tmp_path, dname.home);
        fprintf(fp, "StarChartDir = %s\n", tmp_path);

	/* SoundServerStartCommand */
        fprintf(fp,
            "SoundServerStartCommand = %s\n",
            sound.start_cmd
        );

#ifdef JS_SUPPORT
        /* Joystick calibration file. */
        fprintf(fp,
            "JSCalibrationFile = %s\n",
            fname.js_calib
        );
#endif /* JS_SUPPORT */

	/* MainPageFile */
        strncpy(tmp_path, fname.main_page, PATH_MAX + NAME_MAX);
        tmp_path[PATH_MAX + NAME_MAX - 1] = '\0';
        StripParentPath(tmp_path, dname.etc);
        fprintf(fp, "MainPageFile = %s\n", tmp_path);

        /* DestroyedPageFile */
        strncpy(tmp_path, fname.destroyed_page, PATH_MAX + NAME_MAX);
        tmp_path[PATH_MAX + NAME_MAX - 1] = '\0';
        StripParentPath(tmp_path, dname.etc);
        fprintf(fp, "DestroyedPageFile = %s\n", tmp_path);

	/* OCSNamesFile */
        strncpy(tmp_path, fname.ocsn, PATH_MAX + NAME_MAX);
        tmp_path[PATH_MAX + NAME_MAX - 1] = '\0';
        StripParentPath(tmp_path, dname.etc);
        fprintf(fp, "OCSNamesFile = %s\n", tmp_path);

        /* ISRFile */
        strncpy(tmp_path, fname.isr, PATH_MAX + NAME_MAX);
        tmp_path[PATH_MAX + NAME_MAX - 1] = '\0';
        StripParentPath(tmp_path, dname.images);
        fprintf(fp, "ISRFile = %s\n", tmp_path);

        /* SoundSchemeFile */
        strncpy(tmp_path, fname.sound_scheme, PATH_MAX + NAME_MAX);
        tmp_path[PATH_MAX + NAME_MAX - 1] = '\0';
        StripParentPath(tmp_path, dname.sounds);
        fprintf(fp, "SoundSchemeFile = %s\n", tmp_path);

        /* LogFile */
        strncpy(tmp_path, fname.log, PATH_MAX + NAME_MAX);
        tmp_path[PATH_MAX + NAME_MAX - 1] = '\0';
        StripParentPath(tmp_path, dname.ltoplevel);
        fprintf(fp, "LogFile = %s\n", tmp_path);

        /* UniverseListFile */
        strncpy(tmp_path, fname.universe_list, PATH_MAX + NAME_MAX);
        tmp_path[PATH_MAX + NAME_MAX - 1] = '\0';
        StripParentPath(tmp_path, dname.ltoplevel);
        fprintf(fp, "UniverseListFile = %s\n", tmp_path);

        fprintf(fp, "\n\n");


        /* ********************************************************* */
	/* Network parameters. */
        fprintf(fp,
            "# Network\n\n"
        );

        /* NetDeviceMaxLoad */
        fprintf(fp,
            "NetDeviceMaxLoad = %i\n",
            (int)loadstat.net_load_max
        );

        /* NetIntervalAutoTune */
        fprintf(fp,
            "NetIntervalAutoTune = %s\n",
            (auto_interval_tune.state == 1) ? "yes" : "no"
        );   

        /* NetIntervalTuneInterval */
        fprintf(fp,
	    "NetIntervalTuneInterval = %i\n",
            (int)auto_interval_tune.interval
        );

	/* DefaultServerAddress */
        fprintf(fp,
	    "DefaultServerAddress = %s\n",
            net_parms.address
        );

        /* DefaultServerPort */
        fprintf(fp,
	    "DefaultServerPort = %i\n",
            (int)net_parms.port
        );

        /* DefaultLoginName */
        fprintf(fp,
	    "DefaultLoginName = %s\n",
	    net_parms.login_name
        );

        /* DefaultLoginPassword */
        fprintf(fp,
	    "DefaultLoginPassword = %s\n",
            net_parms.login_password
        );

        fprintf(fp, "\n\n");


	/* ********************************************************* */
        /* General. */
        fprintf(fp,
	    "# General\n\n"
        );

        /* AutoMapEconomyWindow */
        fprintf(fp,
            "AutoMapEconomyWindow = %s\n",
            (option.auto_map_eco_win) ? "yes" : "no"
        );

        /* AutoViewscreenZoom */
        fprintf(fp,
            "AutoViewscreenZoom = %s\n",
            (option.auto_zoom == 1) ? "on" : "off"
        );

	/* ScannerOrientation */
	switch(bridge_win.scanner_orient)
	{
	  case SCANNER_ORIENT_LOCAL:
            fprintf(fp,
                "ScannerOrientation = vessel\n"
	    );
	    break;

	  default:	/* SCANNER_ORIENT_GC */
            fprintf(fp,
                "ScannerOrientation = galactic_core\n"
            );
	    break;
	}

        /* ThrottleScopeMode */
	switch(option.throttle_mode)
	{
	  case THROTTLE_MODE_INCREMENTAL:
            fprintf(fp, "ThrottleScopeMode = incremental\n");
            break;

	  case THROTTLE_MODE_BIDIRECTIONAL:
	    fprintf(fp, "ThrottleScopeMode = bidirectional\n");
	    break;

	  default:
	    fprintf(fp, "ThrottleScopeMode = normal\n");
            break;
	}

        /* LocalUpdates */
        fprintf(fp,
            "LocalUpdates = %s\n",
            (option.local_updates) ? "on" : "off"
        );

        /* ViewScreenMarks (was DisplayHeadingArrow) */
        fprintf(fp,
            "ViewScreenMarks = %s\n",
	    (option.show_viewscreen_marks) ? "yes" : "no"
        );

        /* ViewScreenLabels */
        fprintf(fp,
            "ViewScreenLabels = %i\n",
	    option.show_viewscreen_labels
        );

	/* ShowLensFlares */
        fprintf(fp,
            "ShowLensFlares = %s\n",
            ((option.show_lens_flares) ? "yes" : "no")
        );

        /* ShowStrobeGlow */
        fprintf(fp,
            "ShowStrobeGlow = %s\n",
            ((option.show_strobe_glow) ? "yes" : "no")
        );

        /* ShowNebulaGlow */
        fprintf(fp,
            "ShowNebulaGlow = %s\n",
            ((option.show_nebula_glow) ? "yes" : "no")
        );

        /* FormalNameLabels */
        fprintf(fp,
            "FormalNameLabels = %i\n",
            option.show_formal_label
        );

	/* NotifyScannerContacts */
	fprintf(fp,
            "NotifyScannerContacts = %s\n",
            (option.notify_scanner_contacts) ? "on" : "off"
        );

	/* ShowNetErrors */
	fprintf(fp,
            "ShowNetErrors = %s\n",
            (option.show_net_errors) ? "yes" : "no"
        );

        /* ShowServerErrors */
        fprintf(fp,
            "ShowServerErrors = %s\n",
            (option.show_server_errors) ? "yes" : "no"
        );

	/* SoundServerType */
	fprintf(fp,
	    "SoundServerType = %i\n",
	    sound.server_type
        ); 

	/* Sounds */
        fprintf(fp,  
            "Sounds = %i\n",
            option.sounds
        );

        /* Music */
        fprintf(fp,
            "Music = %s\n",
            ((option.music) ? "on" : "off")
        );

        /* SoundServerArgument */
        fprintf(fp,
            "SoundServerConnectionArgument = %s\n",
            sound.con_arg
        );

        /* Units */
	switch(option.units)
	{
	  case XSW_UNITS_XSW:
	    fprintf(fp,
                "Units = XSW\n"
            );
	    break;

          case XSW_UNITS_METRIC:
            fprintf(fp,
                "Units = AstroMetric\n"
            );
            break;

          case XSW_UNITS_ENGLISH:
            fprintf(fp,
                "Units = English\n"
            );
            break;

          default:
            fprintf(fp,
                "Units = XSW\n"
            );
            break;
	}

        /* AsyncImageLoading */
        fprintf(fp,
            "AsyncImageLoading = %s\n",
            (option.async_image_loading == 1) ? "yes" : "no"
        );

        /* AsyncImageLoadPixels */
        fprintf(fp,
            "AsyncImageLoadPixels = %i\n",
            option.async_image_pixels
        );


        fprintf(fp, "\n\n");


        /* ********************************************************* */
        /* Universe list window position. */

        /* AutoMapUniverseListWindow */
        fprintf(fp,
            "AutoMapUniverseListWindow = %s\n",
            ((option.auto_map_univ_list_win) ? "yes" : "no")
        );

	fprintf(fp,
            "UnivListWinX = %i\n",
            univ_list_win.x
        );
        fprintf(fp,
            "UnivListWinY = %i\n",
            univ_list_win.y          
        );
        fprintf(fp,
            "UnivListWinWidth = %i\n",
            univ_list_win.width
        );
        fprintf(fp,
            "UnivListWinHeight = %i\n",
            univ_list_win.height
        );

	fprintf(fp, "\n\n");


	/* ******************************************************** */
	/* Star chart window position. */
        fprintf(fp,
            "StarChartWinX = %i\n",
            starchart_win.x
        );
        fprintf(fp,
            "StarChartWinY = %i\n",
            starchart_win.y
        );
        fprintf(fp,
            "StarChartWinWidth = %i\n",
            starchart_win.width
        );
        fprintf(fp,
            "StarChartWinHeight = %i\n",
            starchart_win.height
        );

        fprintf(fp, "\n\n");


        /* ********************************************************* */ 
        /* Quick menu commands. */
        fprintf(fp,
	    "# Quick menu commands:\n\n"
        );

        for(i = 0; i < total_qmenu_commands; i++)
        {
 	    if(qmenu_command[i] == NULL)
		continue;

	    fprintf(fp, "BeginQuickMenuCommand\n");

            fprintf(fp, "    Name = %s\n",
		qmenu_command[i]->name
	    );

            fprintf(fp, "    Type = %i\n",  
                qmenu_command[i]->type
	    );

            fprintf(fp, "    ActionCode = %i\n",
                qmenu_command[i]->action
	    );

            fprintf(fp, "EndQuickMenuCommand\n\n");
	}

        fprintf(fp, "\n");


        /* ********************************************************* */ 
        /* Key Mappings. */
        fprintf(fp,
	    "# Key Mappings\n\n"
	);

        fprintf(fp, "BeginKeyMap\n");

	for(i = 0; i < (int)TOTAL_XSW_KEYMAPS; i++)
	{
	    fprintf(fp,
		"    %s = %i\n",
		xsw_keymap_name[i],
		xsw_keymap[i].keycode
	    );
	}

        fprintf(fp, "EndKeyMap\n\n\n");


#ifdef JS_SUPPORT
        /* ********************************************************* */
        /* Joystick mappings. */
        fprintf(fp,
            "# Joystick mappings:\n\n"
        );   

        for(i = 0; i < total_jsmaps; i++)
        {       
            if(jsmap[i] == NULL)
                continue;

            fprintf(fp, "# Joystick %i\n", i);
            fprintf(fp, "BeginJSMap\n");

            /* DeviceName */
            if(jsmap[i]->device_name != NULL)
            {
                fprintf(fp, "    DeviceName = %s\n", jsmap[i]->device_name);
            }

            /* Axises. */ 
            for(n = 0; n < jsmap[i]->total_axises; n++) 
            {
                if(jsmap[i]->axis[n] == NULL)  
                    continue;

                /* BeginAxis */  
                fprintf(fp, "#   Axis %i.\n", n);  
                fprintf(fp, "    BeginAxis\n");  

                /* OpCode */ 
                fprintf(fp, "        OpCode = %i\n", 
                    jsmap[i]->axis[n]->op_code  
                );

                fprintf(fp, "    EndAxis\n");  
            }

            /* Buttons. */ 
            for(n = 0; n < jsmap[i]->total_buttons; n++)
            {
                if(jsmap[i]->button[n] == NULL)
                    continue;

                /* BeginButton */
                fprintf(fp, "#   Button %i.\n", n);
                fprintf(fp, "    BeginButton\n");

                /* Keycode */
                fprintf(fp, "        Keycode = %i\n",
                    jsmap[i]->button[n]->keycode
                );

                fprintf(fp, "    EndButton\n"); 
            }

            fprintf(fp, "EndJSMap\n\n");
        }

        fprintf(fp, "\n\n");
#endif  /* JS_SUPPORT */



	/* *********************************************************** */
        /* ControlType (must be specified after joystick mappings) */
        fprintf(fp,
 "# Controller type (should be specified after keys and joystick mappings\n\n"
        );        
        switch(option.controller)
        {
#ifdef JS_SUPPORT
          case CONTROLLER_JOYSTICK:
            fprintf(fp, "ControlType = joystick\n");
            break;
#endif /* JS_SUPPORT */ 

          case CONTROLLER_POINTER:
            fprintf(fp, "ControlType = pointer\n");
            break;
         
          default:
            fprintf(fp, "ControlType = keyboard\n");
            break;
        }
	fprintf(fp, "\n\n");


        /* ********************************************************* */
        /* Colors. */

	/* Colors: General text. */

        RC_PRINT_COLOR_STRING(
            fp, "ColorStandardText", xsw_color.standard_text_cv
	);
        RC_PRINT_COLOR_STRING(
            fp, "ColorBoldText", xsw_color.bold_text_cv
        );
        RC_PRINT_COLOR_STRING(
            fp, "ColorWithdrawnText", xsw_color.withdrawn_text_cv
        );

        fprintf(fp, "\n");


        /* Colors: Bridge panel. */
        fprintf(fp, "# Colors: Bridge Panel\n");


        RC_PRINT_COLOR_STRING(
            fp, "ColorBPStandardText", xsw_color.bp_standard_text_cv
        );
        RC_PRINT_COLOR_STRING(
            fp, "ColorBPBoldText", xsw_color.bp_bold_text_cv
        );
        RC_PRINT_COLOR_STRING(
            fp, "ColorBPWithdrawnText", xsw_color.bp_withdrawn_text_cv
        );
        RC_PRINT_COLOR_STRING(
            fp, "ColorBPLightOutline", xsw_color.bp_light_outline_cv
        );
        RC_PRINT_COLOR_STRING(
            fp, "ColorBPNormalOutline", xsw_color.bp_normal_outline_cv
        );
        RC_PRINT_COLOR_STRING(
            fp, "ColorBPDarkOutline", xsw_color.bp_dark_outline_cv
        );

	fprintf(fp, "\n");

        RC_PRINT_COLOR_STRING(
            fp, "ColorBPWarning", xsw_color.bp_warning_cv
        );
        RC_PRINT_COLOR_STRING(
            fp, "ColorBPDanger", xsw_color.bp_danger_cv
        );
        RC_PRINT_COLOR_STRING(
            fp, "ColorBPCritical", xsw_color.bp_critical_cv
        );

        fprintf(fp, "\n");

        RC_PRINT_COLOR_STRING(
            fp, "ColorBPFriendly", xsw_color.bp_friendly_cv
        );
        RC_PRINT_COLOR_STRING(
            fp, "ColorBPUnknown", xsw_color.bp_unknown_cv
        );
        RC_PRINT_COLOR_STRING(
            fp, "ColorBPHostile", xsw_color.bp_hostile_cv
        );

        fprintf(fp, "\n");

        /* Colors: Bridge panel outline. */
        fprintf(fp, "# Colors: Bridge Panel Outline\n");

        RC_PRINT_COLOR_STRING(
            fp, "ColorBPHull", xsw_color.bpol_hull_cv
        );
        RC_PRINT_COLOR_STRING(
            fp, "ColorBPPower", xsw_color.bpol_power_cv
        );
        RC_PRINT_COLOR_STRING(
            fp, "ColorBPVis", xsw_color.bpol_vis_cv
        );
        RC_PRINT_COLOR_STRING(
            fp, "ColorBPShields", xsw_color.bpol_shields_cv
        );
        RC_PRINT_COLOR_STRING(
            fp, "ColorBPDmgCtl", xsw_color.bpol_dmgctl_cv
        );
        RC_PRINT_COLOR_STRING(
            fp, "ColorBPThrottle", xsw_color.bpol_throttle_cv
        );
        RC_PRINT_COLOR_STRING(
            fp, "ColorBPThrottleRev", xsw_color.bpol_throttle_rev_cv
        );

        fprintf(fp, "\n");


	/* Colors: Viewscreen labels. */
        fprintf(fp, "# Colors: Viewscreen Labels\n");

        RC_PRINT_COLOR_STRING(
            fp, "ColorVSLabelBg", xsw_color.vs_label_bg_cv
        );

        RC_PRINT_COLOR_STRING(
            fp, "ColorVSLabelFg", xsw_color.vs_label_fg_cv
        );

        fprintf(fp, "\n");


        /* Colors: Scanner marks. */
        fprintf(fp, "# Colors: Scanner Markings\n");

        RC_PRINT_COLOR_STRING(
            fp, "ColorScMarkUnknown", xsw_color.scmark_unknown_cv
        );

        RC_PRINT_COLOR_STRING(
            fp, "ColorScMarkLocked", xsw_color.scmark_locked_cv
        );

        RC_PRINT_COLOR_STRING(
            fp, "ColorScMarkWeapon", xsw_color.scmark_weapon_cv
        );

        RC_PRINT_COLOR_STRING(
            fp, "ColorScMarkHome", xsw_color.scmark_home_cv
        );

        RC_PRINT_COLOR_STRING(
            fp, "ColorScMarkArea", xsw_color.scmark_area_cv
        );

        fprintf(fp, "\n");


        /* Colors: Keymap query dialog. */
        fprintf(fp, "# Colors: Keymap Query Dialog\n");

        RC_PRINT_COLOR_STRING(
            fp, "ColorKeymapQueryBg", xsw_color.keymap_query_bg_cv
        );
        RC_PRINT_COLOR_STRING(
            fp, "ColorKeymapQueryFg", xsw_color.keymap_query_fg_cv
        );

        fprintf(fp, "\n");


	/* Colors: Viewscreen graphics. */
        fprintf(fp, "# Colors: Viewscreen Graphics\n");
	RC_PRINT_COLOR_STRING(
	    fp, "ColorHeadingArrow", xsw_color.heading_arrow
	);
        RC_PRINT_COLOR_STRING(
            fp, "ColorLockArrow", xsw_color.lock_arrow
        );
        RC_PRINT_COLOR_STRING(
            fp, "ColorLockCursor", xsw_color.lock_cursor
        );
        RC_PRINT_COLOR_STRING(
            fp, "ColorStreamGreen", xsw_color.stream_green
        );
        RC_PRINT_COLOR_STRING(
            fp, "ColorStreamOrange", xsw_color.stream_orange
        );
        RC_PRINT_COLOR_STRING(
            fp, "ColorStreamPurple", xsw_color.stream_purple
        );
        RC_PRINT_COLOR_STRING(
            fp, "ColorStreamYellow", xsw_color.stream_yellow
        );
        RC_PRINT_COLOR_STRING(
            fp, "ColorShieldBlue", xsw_color.shield_blue
        );
	RC_PRINT_COLOR_STRING(
            fp, "ColorVisibilityMarker", xsw_color.visibility_marker
	);
        RC_PRINT_COLOR_STRING(
            fp, "ColorStarGlow", xsw_color.star_glow
        );
        fprintf(fp, "\n");

        /* Colors: Starchart. */
        fprintf(fp, "# Colors: Starchart\n");
	RC_PRINT_COLOR_STRING(
            fp, "ColorChartBackground", xsw_color.chart_bg_cv
        );
        RC_PRINT_COLOR_STRING(
            fp, "ColorChartGrid", xsw_color.chart_grid_cv
        );
        RC_PRINT_COLOR_STRING(
            fp, "ColorChartSectorGrid", xsw_color.chart_sector_grid_cv
        );
        RC_PRINT_COLOR_STRING(
            fp, "ColorChartCrossHairs", xsw_color.chart_cross_hairs_cv
        );

        fprintf(fp, "\n\n");



	/* ********************************************************* */

	/* Close file. */
	fclose(fp);


	return(0);
}
