#include <fnmatch.h>

#include "xsw.h"
#include "net.h"


int CMDSETPARMMATCH(const char *s, const char *pattern);


/*
 *	Macro to perform parameter name matching.
 */
int CMDSETPARMMATCH(const char *s, const char *pattern)
{
	return(!fnmatch(s, pattern, 0));
}


int CmdSet(const char *arg)
{
	int n;
        char *strptr;

        char text[CLIENT_CMD_MAX + 256];

        char parm[CLIENT_CMD_MAX];
        char val[CLIENT_CMD_MAX];


	/* Parse argument. */
	if(arg == NULL)
	    strptr = NULL;
	else
	    strptr = (char*) strchr(arg, '=');
        if(strptr == NULL)
        {
	    /* No '=' delimiter, just get parm. */

            strncpy(parm, arg, CLIENT_CMD_MAX);
            parm[CLIENT_CMD_MAX - 1] = '\0';
            if(*parm == '\0')
            {
                strncpy(parm, "*", CLIENT_CMD_MAX);
                parm[CLIENT_CMD_MAX - 1] = '\0';
            }

            *val = '\0';
        }
        else
        {
	    /* Get val and parm. */

            strncpy(val, strptr + 1, CLIENT_CMD_MAX);
            val[CLIENT_CMD_MAX - 1] = '\0';

            strncpy(parm, arg, CLIENT_CMD_MAX);
            parm[CLIENT_CMD_MAX - 1] = '\0';
	    strptr = strchr(parm, '=');
	    if(strptr != NULL)
		*strptr = '\0';
        }
	StringStripSpaces(parm);
	StringStripSpaces(val);


        /* Auto network interval tunning. */
	if(CMDSETPARMMATCH(parm, "auto_interval_tune") ||
           CMDSETPARMMATCH(parm, "auto_interval") ||
           CMDSETPARMMATCH(parm, "auto_int") ||
           CMDSETPARMMATCH(parm, "ainterval") ||
           CMDSETPARMMATCH(parm, "aint")
	)
	{
	    if(*val != '\0')
	        auto_interval_tune.state = StringIsYes(val);

	    sprintf(text, "auto_interval = %s",
                ((auto_interval_tune.state) ? "on" : "off")
            );
            MesgAdd(text, xsw_color.bp_standard_text);
	}

	/* Auto map economy window. */
	if(CMDSETPARMMATCH(parm, "auto_map_eco_win"))
	{
	    if(*val != '\0')
		option.auto_map_eco_win = StringIsYes(val);

            sprintf(text, "auto_map_eco_win = %s",
                ((option.auto_map_eco_win) ? "yes" : "no")
            );
            MesgAdd(text, xsw_color.bp_standard_text);
	}

	/* Auto zoom. */
        if(CMDSETPARMMATCH(parm, "auto_zoom"))
	{
	    if(*val != '\0')
                option.auto_zoom = StringIsYes(val);

            sprintf(text, "auto_zoom = %s",
                ((option.auto_zoom) ? "on" : "off")
            );
	    MesgAdd(text, xsw_color.bp_standard_text);   
	}

	/* Controller type. */
	if(CMDSETPARMMATCH(parm, "controller") ||
           CMDSETPARMMATCH(parm, "control_type") ||
           CMDSETPARMMATCH(parm, "control")
	)
        {   
            if(*val != '\0')
	    {
/* Pointer not available yet.
		if(strcasepfx(val, "p"))
		    option.controller = CONTROLLER_POINTER
 */
                if(strcasepfx(val, "k"))
                    option.controller = CONTROLLER_KEYBOARD;
#ifdef JS_SUPPORT
                else if(strcasepfx(val, "j"))
                    option.controller = CONTROLLER_JOYSTICK;
#endif	/* JS_SUPPORT */
                else
#ifdef JS_SUPPORT
                    MesgAdd(
             "Available control_type values: keyboard  joystick",
                        xsw_color.bp_standard_text
                    );
#else
                    MesgAdd(
              "Available control_type values: keyboard",
                        xsw_color.bp_standard_text
                    );
#endif	/* JS_SUPPORT */

		/* Reinitialize controller. */
		GCtlInit(option.controller);
	    }

	    switch(option.controller)
	    {
              case CONTROLLER_POINTER:
                sprintf(text, "control_type = pointer");
		break;

	      case CONTROLLER_KEYBOARD:
		sprintf(text, "control_type = keyboard");
		break;

#ifdef JS_SUPPORT
	      case CONTROLLER_JOYSTICK:
		sprintf(text, "control_type = joystick");
		break;
#endif /* JS_SUPPORT */

	       default:
		sprintf(text, "control_type = *badvalue*");
		break;
	    }
	    MesgAdd(text, xsw_color.bp_standard_text);
	}

	/* Display events (obsolete). */
        if(CMDSETPARMMATCH(parm, "display_events"))
        {
            if(*val != '\0')
                option.display_events = StringIsYes(val);

            sprintf(text, "display_events = %s",
                ((option.display_events) ? "on" : "off")
            );
            MesgAdd(text, xsw_color.bp_standard_text);
        }

	/* Energy saver mode. */
        if(CMDSETPARMMATCH(parm, "energy_saver_mode"))
        {     
            if(*val != '\0')
		option.energy_saver_mode = StringIsYes(val);
            
            sprintf(text, "energy_saver_mode = %s",
                ((option.energy_saver_mode) ? "on" : "off")
            );
            MesgAdd(text, xsw_color.bp_standard_text);
        }

	/* Local updates. */
        if(CMDSETPARMMATCH(parm, "local_updates"))
        {
            if(*val != '\0')
                option.local_updates = StringIsYes(val);
        
            sprintf(text, "local_updates = %s",
                ((option.local_updates) ? "on" : "off")
            );
            MesgAdd(text, xsw_color.bp_standard_text);
        }   

	/* Log client messages. */
        if(CMDSETPARMMATCH(parm, "log_client"))
        {
            if(*val != '\0')
                option.log_client = StringIsYes(val);
        
            sprintf(text, "log_client = %s",
                ((option.log_client) ? "on" : "off")
            );
            MesgAdd(text, xsw_color.bp_standard_text);   
        }

        /* Log network messages. */
        if(CMDSETPARMMATCH(parm, "log_net"))
        {
            if(*val != '\0')
                option.log_net = StringIsYes(val);

            sprintf(text, "log_net = %s",
                ((option.log_net) ? "on" : "off")
            );
            MesgAdd(text, xsw_color.bp_standard_text);
        }

        /* Log error messages. */
        if(CMDSETPARMMATCH(parm, "log_errors"))
        {
            if(*val != '\0')
                option.log_errors = StringIsYes(val);
        
            sprintf(text, "log_errors = %s",
                ((option.log_errors) ? "on" : "off")
            );
            MesgAdd(text, xsw_color.bp_standard_text);
        }

        /* Login name. */
        if(CMDSETPARMMATCH(parm, "login_name") ||
           CMDSETPARMMATCH(parm, "lname")
	)
        {
            if(*val != '\0')
	    {
                strncpy(net_parms.login_name, val, XSW_OBJ_NAME_MAX);
                net_parms.login_name[XSW_OBJ_NAME_MAX - 1] = '\0';

                switch(DBValidateObjectName(net_parms.login_name))
                {
                  case -1:
                    sprintf(text,
                        "%s: Invalid name.",
                        net_parms.login_name
                    );
		    MesgAdd(text, xsw_color.bp_standard_text);
                    break;

                  case 1:
                    sprintf(text,
                        "%s: Name too long.",
                        net_parms.login_name
                    );
		    MesgAdd(text, xsw_color.bp_standard_text);
                    break;

                  case 2:
                    sprintf(text,
                        "%s: Name too short.",
                        net_parms.login_name
                    );
		    MesgAdd(text, xsw_color.bp_standard_text);
                    break;

                  case 3:
                    sprintf(text,
                        "%s: Name contains invalid character(s).",
                        net_parms.login_name
		    );
		    MesgAdd(text, xsw_color.bp_standard_text);
		    break;
		}
	    }

            sprintf(text,
                "login_name = %s",
                net_parms.login_name
            );
            MesgAdd(text, xsw_color.bp_standard_text);
        }

        /* Login password. */
        if(CMDSETPARMMATCH(parm, "login_password") ||
           CMDSETPARMMATCH(parm, "login_passwd") ||
           CMDSETPARMMATCH(parm, "login_pass") ||
           CMDSETPARMMATCH(parm, "lpassword") ||
           CMDSETPARMMATCH(parm, "lpasswd") ||
           CMDSETPARMMATCH(parm, "lpass")
        )
        {
            if(*val != '\0')
            {
                strncpy(net_parms.login_password, val, XSW_OBJ_PASSWORD_MAX);
                net_parms.login_password[XSW_OBJ_PASSWORD_MAX - 1] = '\0';
	    }

	    sprintf(text, "login_password = %s",
		net_parms.login_password
	    );
	    MesgAdd(text, xsw_color.bp_standard_text);
	}

        /* Music. */
        if(CMDSETPARMMATCH(parm, "music"))
        {
            if(*val != '\0')
	    {
                option.music = StringIsYes(val);
		if(option.music)
		    XSWDoChangeBackgroundMusic();
		else
		    SoundStopBackgroundMusic();
	    }

            sprintf(text, "music = %s",
                ((option.music) ? "on" : "off")     
            );
            MesgAdd(text, xsw_color.bp_standard_text);
        }

	/* Network interval. */
        if(CMDSETPARMMATCH(parm, "net_int") ||
           CMDSETPARMMATCH(parm, "int"))
        {
            if(*val != '\0')
                net_parms.net_int = atol(val);

            sprintf(text, "net_int = %ld",
		net_parms.net_int
            );
            MesgAdd(text, xsw_color.bp_standard_text);
        }

        /* Network load max (bandwidth). */
        if(CMDSETPARMMATCH(parm, "net_load_max"))
        {
            if(*val != '\0')
                loadstat.net_load_max = atol(val);

            sprintf(text, "net_load_max = %ld",
                loadstat.net_load_max
            );
            MesgAdd(text, xsw_color.bp_standard_text);
        }

        /* Notify scanner contacts. */
        if(CMDSETPARMMATCH(parm, "notify_scanner_contacts"))
        {
            if(*val != '\0')
                option.notify_scanner_contacts = StringIsYes(val);

            sprintf(text, "notify_scanner_contacts = %s",
		((option.notify_scanner_contacts) ? "on" : "off")
            );
            MesgAdd(text, xsw_color.bp_standard_text);
        }

        /* Scanner limiting. */
        if(CMDSETPARMMATCH(parm, "scanner_limiting"))
        {
            if(*val != '\0')
                option.scanner_limiting = StringIsYes(val);

            sprintf(text, "scanner_limiting = %s",
                ((option.scanner_limiting) ? "on" : "off")
            );
            MesgAdd(text, xsw_color.bp_standard_text);
        }

	/* Show formal labels. */
        if(CMDSETPARMMATCH(parm, "show_formal_label"))
        {   
            if(*val != '\0')
	    {
		if(strcasepfx(val, "never") ||
                   strcasepfx(val, "no") ||
                   strcasepfx(val, "0")
		)
		    option.show_formal_label = 0;
		else if(strcasepfx(val, "asneeded") ||
                        strcasepfx(val, "as needed") ||
                        strcasepfx(val, "1")
		)
		    option.show_formal_label = 1;
                else if(strcasepfx(val, "always") ||
                        strcasepfx(val, "yes") ||
                        strcasepfx(val, "2")   
                )
                    option.show_formal_label = 2;
		else
		    MesgAdd(
 "Available show_formal_label values: never  asneeded  always",
                        xsw_color.bp_standard_text
                    );
	    }

	    switch(option.show_formal_label)
	    {
	      case 0:
		sprintf(text, "show_formal_label = never");
		break;

	      case 1:
                sprintf(text, "show_formal_label = asneeded");
                break;

	      case 2:
		sprintf(text, "show_formal_label = always");
		break;

	      default:
                sprintf(text, "show_formal_label = *bad value*");
                break;
	    }
            MesgAdd(text, xsw_color.bp_standard_text);
	}

        /* Show lens flags. */
        if(CMDSETPARMMATCH(parm, "show_lens_flares"))
        {
            if(*val != '\0')
                option.show_lens_flares = StringIsYes(val);

            sprintf(text, "show_lens_flares = %s",
                ((option.show_lens_flares) ? "on" : "off")
            );
            MesgAdd(text, xsw_color.bp_standard_text);
        }

        /* Show nebula glow. */
        if(CMDSETPARMMATCH(parm, "show_nebula_glow"))
        {
            if(*val != '\0')
                option.show_nebula_glow = StringIsYes(val);
            
            sprintf(text, "show_nebula_glow = %s",
                ((option.show_nebula_glow) ? "on" : "off")
            );
            MesgAdd(text, xsw_color.bp_standard_text);
        }

        /* Show network errors. */
        if(CMDSETPARMMATCH(parm, "show_net_errors"))
        {
            if(*val != '\0')
                option.show_net_errors = StringIsYes(val);
        
            sprintf(text, "show_net_errors = %s",
                ((option.show_net_errors) ? "on" : "off")
            );
            MesgAdd(text, xsw_color.bp_standard_text);  
        }

        /* Show viewscreen labels. */
        if(CMDSETPARMMATCH(parm, "show_viewscreen_labels"))
        {
            if(*val != '\0')
            {
                if(strcasepfx(val, "none") ||
                   strcasepfx(val, "no") ||
                   strcasepfx(val, "0")
                )
                    option.show_viewscreen_labels = 0;
                else if(strcasepfx(val, "labels") ||
                        strcasepfx(val, "1")
                )
                    option.show_viewscreen_labels = 1;
                else if(strcasepfx(val, "stats") ||
                        strcasepfx(val, "2")
                )
                    option.show_viewscreen_labels = 2;
                else if(strcasepfx(val, "all") ||
                        strcasepfx(val, "yes") ||
                        strcasepfx(val, "3")
                )
                    option.show_viewscreen_labels = 3;
                else
                    MesgAdd(
 "Available show_viewscreen_labels values: none  labels  stats  all",
                        xsw_color.bp_standard_text
                    );
            }

	    switch(option.show_viewscreen_labels)
	    {
	      case 0:
		sprintf(text, "show_viewscreen_labels = none");
		break;

	      case 1:
		sprintf(text, "show_viewscreen_labels = labels");
		break;

              case 2:
		sprintf(text, "show_viewscreen_labels = stats"); 
		break;

              case 3:
                sprintf(text, "show_viewscreen_labels = all"); 
                break;

              default:
                sprintf(text, "show_viewscreen_labels = *bad value*"); 
                break;
	    }
            MesgAdd(text, xsw_color.bp_standard_text);
	}

        /* Show strobe glow. */
        if(CMDSETPARMMATCH(parm, "show_strobe_glow"))
        {
            if(*val != '\0')
                option.show_strobe_glow = StringIsYes(val);
         
            sprintf(text, "show_strobe_glow = %s",
                ((option.show_strobe_glow) ? "on" : "off")
            );
            MesgAdd(text, xsw_color.bp_standard_text);
        }

        /* Show viewscreen markings. */
        if(CMDSETPARMMATCH(parm, "show_viewscreen_marks"))
        {
            if(*val != '\0')
                option.show_viewscreen_marks = StringIsYes(val);   

            sprintf(text, "show_viewscreen_marks = %s",
                ((option.show_viewscreen_marks) ? "on" : "off")
            );
            MesgAdd(text, xsw_color.bp_standard_text);
        }

        /* Sound server type. */
        if(CMDSETPARMMATCH(parm, "sound_server_type"))
        {
            if(*val != '\0')
            {
		if(strcasepfx(val, "none"))
		    sound.server_type = SNDSERV_TYPE_NONE;
		else if(strcasepfx(val, "yiff"))
                    sound.server_type = SNDSERV_TYPE_YIFF;
                else if(strcasepfx(val, "esound"))
                    sound.server_type = SNDSERV_TYPE_ESOUND;
                else if(strcasepfx(val, "mikmod"))
                    sound.server_type = SNDSERV_TYPE_MIKMOD;
		else
		    MesgAdd(
 "Available sound_server_type values: none  yiff  esound  mikmod",
			xsw_color.bp_standard_text
		    );
	    }

	    switch(sound.server_type)
	    {
	      case SNDSERV_TYPE_NONE:
                sprintf(text, "sound_server_type = none");
                break;

              case SNDSERV_TYPE_YIFF:
                sprintf(text, "sound_server_type = yiff");
                break;

              case SNDSERV_TYPE_ESOUND:
                sprintf(text, "sound_server_type = esound");
                break;

              case SNDSERV_TYPE_MIKMOD:
                sprintf(text, "sound_server_type = mikmod");
                break;

              default:
                sprintf(text, "sound_server_type = *bad value*");
                break;
            }
            MesgAdd(text, xsw_color.bp_standard_text);
	}

        /* Sounds. */
        if(CMDSETPARMMATCH(parm, "sounds"))
        {
            if(*val != '\0')
            {
                if(strcasepfx(val, "none") ||
                   strcasepfx(val, "off") ||
                   strcasepfx(val, "0")
		)
                    n = XSW_SOUNDS_NONE;
                else if(strcasepfx(val, "events") ||
                        strcasepfx(val, "1")
                )
                    n = XSW_SOUNDS_EVENTS;
                else if(strcasepfx(val, "engine") ||
                        strcasepfx(val, "2")
                )
                    n = XSW_SOUNDS_ENGINE;
                else if(strcasepfx(val, "all") ||
                        strcasepfx(val, "on") ||
                        strcasepfx(val, "3")
                )
                    n = XSW_SOUNDS_ALL;
		else
		    n = -1;

		if(n == -1)
		{
                    MesgAdd(
 "Available sounds values: none  events  engine  all",
                        xsw_color.bp_standard_text
                    );
		}
		else
		{
		    /* Initialize sound server as needed. */
		    if(sound.server_type > SNDSERV_TYPE_NONE)
		    {
			if((option.sounds > XSW_SOUNDS_NONE) &&
                           (n <= XSW_SOUNDS_NONE)
			)
			{
			    /* Sound was on, now turned off. */
			    SoundShutdown();
			}
			else if((option.sounds <= XSW_SOUNDS_NONE) &&
                                (n > XSW_SOUNDS_NONE)
			)
			{
			    /* Sound was off, now turned on. */
			    if(SoundInit())
			    {
				/* Could not initialize sound. */
				printdw(&err_dw,
"Could not initialize sound server.\n\
\n\
Make sure that your sound card module is inserted,\n\
your sound server is running, and the connect argument\n\
is connect.\n"
				);
			    }
			    else
			    {
				/* Successfully initialized sound, refresh
				 * sound resources.
				 */
				if(option.music)
				    XSWDoChangeBackgroundMusic();
			    }
			}
			else
			{
			    /* No change. */
			}
			option.sounds = n;
		    }
		    else
		    {
		        option.sounds = XSW_SOUNDS_NONE;
		    }
		}
            }

	    switch(option.sounds)
	    {
              case XSW_SOUNDS_NONE:
		sprintf(text, "sounds = off");
		break;

	      case XSW_SOUNDS_EVENTS:
		sprintf(text, "sounds = events");
		break;

	      case XSW_SOUNDS_ENGINE:
		sprintf(text, "sounds = engine");
		break;

	      case XSW_SOUNDS_ALL:
		sprintf(text, "sounds = all");
		break;

	      default:
		sprintf(text, "sounds = *bad value*");
		break;
	    }
            MesgAdd(text, xsw_color.bp_standard_text);
	}

        /* Throttle mode. */
        if(CMDSETPARMMATCH(parm, "throttle_mode"))
        {
            if(*val != '\0')
            {
                if(strcasepfx(val, "i"))
                    option.throttle_mode = THROTTLE_MODE_INCREMENTAL;
		else if(strcasepfx(val, "b"))
                    option.throttle_mode = THROTTLE_MODE_BIDIRECTIONAL;
                else if(strcasepfx(val, "n"))
                    option.throttle_mode = THROTTLE_MODE_NORMAL;
		else
		    MesgAdd(
 "Available throttle_mode values: incremental  bidirectional  normal",
			xsw_color.bp_standard_text
		    );
	    }

	    switch(option.throttle_mode)
	    {
	      case THROTTLE_MODE_INCREMENTAL:
		sprintf(text, "throttle_mode = incremental");
		break;

	      case THROTTLE_MODE_BIDIRECTIONAL:
		sprintf(text, "throttle_mode = bidirectional");
		break; 

	      case THROTTLE_MODE_NORMAL:
		sprintf(text, "throttle_mode = normal");
		break;

	      default:
		sprintf(text, "throttle_mode = *badvalue*");
		break;
            }
	    MesgAdd(text, xsw_color.bp_standard_text);
	}

        /* Units. */
        if(CMDSETPARMMATCH(parm, "units"))
        {
            if(*val != '\0')
            {
                if(strcasepfx(val, "xsw"))
                    option.throttle_mode = XSW_UNITS_XSW;
                else if(strcasepfx(val, "astrometric"))
                    option.throttle_mode = XSW_UNITS_METRIC;
                else if(strcasepfx(val, "english"))
                    option.throttle_mode = XSW_UNITS_ENGLISH;
                else
                    MesgAdd(
 "Available units values: xsw  astrometric  english",
                        xsw_color.bp_standard_text
                    );
            }

	    switch(option.units)
	    {
	      case XSW_UNITS_XSW:
		sprintf(text, "units = XSW");
		break;

	      case XSW_UNITS_METRIC:
		sprintf(text, "units = AstroMetric");
		break;

	      case XSW_UNITS_ENGLISH:
		sprintf(text, "units = English");
		break;

	      default:
		sprintf(text, "units = *vad value*");
		break;
	    }
            MesgAdd(text, xsw_color.bp_standard_text);
	}


	/* Check if a value was given. */
	if(*val == '\0')
	{
	    /* No value given. */

	    /* Print usage. */
            MesgAdd(
	        "Usage: set <property>=<value>",
                xsw_color.bp_standard_text
            );
	}
	else
	{
	    /* Value is given. */

	    /* Need to redraw things that may reflect changes. */
	    BridgeWinDrawAll();
	}


	return(0);
}
