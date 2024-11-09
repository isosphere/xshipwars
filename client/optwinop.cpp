/*
                            Options Window

	Functions:

	void OptWinFetchGlobals()
	int OptWinApplyChanges()
	int OptWinLoadDefaults()
	int OptWinSaveChanges()

	int OptWinTestSoundPBCB(void *ptr)
	int OptWinFBCB(char *path)
	int OptWinBrowseISRefsPBCB(void *ptr)
	int OptWinBrowseOCSNsPBCB(void *ptr)
	int OptWinBrowseSSPBCB(void *ptr)
	int OptWinBrowseJSCalPBCB(void *ptr)
	int OptWinKeymapPBCB(void *ptr)
	int OptWinJSMapPBCB(void *ptr)
	int OptWinRefreshMemoryPBCB(void *ptr)

	---



 */


#include "../include/swsoundcodes.h"
#include "keymapwin.h"
#ifdef JS_SUPPORT
# include "jsmapwin.h"
#endif	/* JS_SUPPORT */

#include "xsw.h"
#include "net.h"

#include "optwin.h"


/* Is a within inclusive range of min and max? */
#define INRANGEINC(a,min,max)	((a >= min) && (a <= max))



/*
 *	Sets options window values to that of the global option.
 */
void OptWinFetchGlobals()
{
	char *strptr;
	int len;
	char tmp_path[PATH_MAX + NAME_MAX];


        /* ************************************************************** */
        /* General. */

	/* Control type. */
        if(INRANGEINC(option.controller, 0, 2))
	{
            options_win.control_type_pul.sel_item = option.controller;
	}
/* Disable joystick option as needed. */
/* Disable pointer. */

	/* Throttle mode. */
        if(INRANGEINC(option.throttle_mode, 0, 2))
            options_win.throttle_mode_tba.armed_tb = option.throttle_mode;

	/* Auto zoom. */
	options_win.auto_zoom_tb.state =
	    ((option.auto_zoom) ? True : False);

	/* Local updates. */
	options_win.local_updates_tb.state =
	    ((option.local_updates) ? True : False);

	/* Scanner contacts. */
	options_win.notify_scanner_contacts_tb.state =
	    ((option.notify_scanner_contacts) ? True : False);


	/* Isref names file. */
	strncpy(tmp_path, fname.isr, PATH_MAX + NAME_MAX);
	tmp_path[PATH_MAX + NAME_MAX - 1] = '\0';
	StripParentPath(tmp_path, dname.images);
	PromptSetS(
	    &options_win.isref_name_prompt,
	    tmp_path
	);

	/* OCSNs names file. */
        strncpy(tmp_path, fname.ocsn, PATH_MAX + NAME_MAX);
        tmp_path[PATH_MAX + NAME_MAX - 1] = '\0';
	StripParentPath(tmp_path, dname.etc);
        PromptSetS(
            &options_win.ocs_name_prompt,
            tmp_path
        );

	/* Sound schemes file. */
        strncpy(tmp_path, fname.sound_scheme, PATH_MAX + NAME_MAX);
        tmp_path[PATH_MAX + NAME_MAX - 1] = '\0';
        StripParentPath(tmp_path, dname.sounds);
        PromptSetS(
            &options_win.ss_name_prompt,
            tmp_path
        );


        /* ************************************************************** */
        /* Graphics. */

	/* Heading arrow. */
	options_win.show_viewscreen_marks_tb.state =
	    (option.show_viewscreen_marks) ? True : False;

	/* Viewscreen labels. */
        if(INRANGEINC(option.show_viewscreen_labels, 0, 3))
            options_win.show_viewscreen_labels_tba.armed_tb =
                option.show_viewscreen_labels;

	/* Formal object name labels. */
        if(INRANGEINC(option.show_formal_label, 0, 2))
            options_win.show_formal_label_tba.armed_tb =
                option.show_formal_label;

	/* Async image loading. */
	options_win.async_image_loading_tb.state =
	    (option.async_image_loading) ? True : False;

	/* Async image loading pixels per cycle prompt. */
	strptr = options_win.async_image_pixels_prompt.buf;
	len = options_win.async_image_pixels_prompt.buf_len;
	if(strptr != NULL)
	    sprintf(strptr, "%i", option.async_image_pixels);


        /* ************************************************************** */
        /* Sounds. */

	/* Sounds level. */
        if(INRANGEINC(option.sounds, 0, 3))
            options_win.sounds_tba.armed_tb = option.sounds;

	/* Music. */
	options_win.music_tb.state = ((option.music) ? True : False);

	/* Sound server type. */
        if(INRANGEINC(sound.server_type, 0, 3))
            options_win.server_type_tba.armed_tb = sound.server_type;
	else
	    options_win.server_type_tba.armed_tb = 0;


	/* Sound server program name. */
	strptr = options_win.sound_server_prompt.buf;
	len = options_win.sound_server_prompt.buf_len;
        if(strptr != NULL)
        {
            strncpy(strptr, sound.start_cmd, len);
	    strptr[len - 1] = '\0';
        }

	/* Sound server connection argument. */
	strptr = options_win.sound_con_arg_prompt.buf;
	len = options_win.sound_con_arg_prompt.buf_len;
        if(strptr != NULL)  
        {
            strncpy(strptr, sound.con_arg, len);
	    strptr[len - 1] = '\0';
        }

	/* Flip stereo. */
/*
	options_win.flip_stereo_tb.state = (sound.flip_stereo == 1) ?
	    True : False;
 */


        /* ************************************************************** */
	/* Network. */

	/* Maximum network load. */
        if(options_win.max_net_load_prompt.buf != NULL)
        {
            sprintf(
		options_win.max_net_load_prompt.buf,
		"%i",
                (int)loadstat.net_load_max
	    );
        }

	/* Default address. */
	strptr = options_win.def_address_prompt.buf;
	len = options_win.def_address_prompt.buf_len;
        if(strptr != NULL)
        {
            strncpy(strptr, net_parms.address, len);
	    strptr[len - 1] = '\0';
        }

	/* Default port. */
	strptr = options_win.def_port_prompt.buf;
        if(strptr != NULL)
        {
            sprintf(strptr, "%i", net_parms.port);
        }

	/* Show network errors. */
	options_win.show_net_errors_tb.state =
	    (option.show_net_errors) ? True : False;

        /* Show server errors. */
        options_win.show_server_errors_tb.state =
            (option.show_server_errors) ? True : False;

	/* Update interval. */
	strptr = options_win.net_int_prompt.buf;
        if(strptr != NULL)
        {
            sprintf(strptr, "%i", (int)net_parms.net_int);
        }

	/* Automatic interval tunning. */
	options_win.auto_interval_tb.state =
	    (auto_interval_tune.state == 1) ? True : False;


        /* ************************************************************** */
        /* Misc. */

	/* Local toplevel directory. */
        strncpy(tmp_path, dname.ltoplevel, PATH_MAX + NAME_MAX);
        tmp_path[PATH_MAX + NAME_MAX - 1] = '\0';
        StripParentPath(tmp_path, dname.home);
        PromptSetS(
            &options_win.xsw_local_toplevel_path_prompt,
            tmp_path
        );

	/* Toplevel directory. */
	PromptSetS(
	    &options_win.xsw_toplevel_path_prompt,
	    dname.toplevel
	);

	/* Etc directory. */
	strncpy(tmp_path, dname.etc, PATH_MAX + NAME_MAX);
        tmp_path[PATH_MAX + NAME_MAX - 1] = '\0';
	StripParentPath(tmp_path, dname.toplevel);
        PromptSetS(
	    &options_win.xsw_etc_path_prompt,
	    tmp_path
	);

	/* Images directory. */
        strncpy(tmp_path, dname.images, PATH_MAX + NAME_MAX);
        tmp_path[PATH_MAX + NAME_MAX - 1] = '\0';
        StripParentPath(tmp_path, dname.toplevel);
        PromptSetS(
            &options_win.xsw_images_path_prompt,
            tmp_path
        );

	/* Sounds directory. */
        strncpy(tmp_path, dname.sounds, PATH_MAX + NAME_MAX);
        tmp_path[PATH_MAX + NAME_MAX - 1] = '\0';
        StripParentPath(tmp_path, dname.toplevel);
        PromptSetS(
            &options_win.xsw_sounds_path_prompt,
            tmp_path
        );

        /* Downloads directory. */
        strncpy(tmp_path, dname.downloads, PATH_MAX + NAME_MAX);
        tmp_path[PATH_MAX + NAME_MAX - 1] = '\0';
        StripParentPath(tmp_path, dname.home);
        PromptSetS(
            &options_win.xsw_downloads_path_prompt,
            tmp_path
        );

#ifdef JS_SUPPORT
	/* Joystick calibration file location. */
	strptr = getenv("HOME");
	if(strptr == NULL)
	    strptr = "/";
        strncpy(tmp_path, fname.js_calib, PATH_MAX + NAME_MAX);
	tmp_path[PATH_MAX + NAME_MAX - 1] = '\0';
        StripParentPath(tmp_path, strptr);
        PromptSetS(
            &options_win.js_calib_path_prompt,
            tmp_path
        );
#endif /* JS_SUPPORT */

        /* Units. */
	switch(option.units)
	{
	  case XSW_UNITS_METRIC:
	    options_win.units_pul.sel_item = 0;
	    break;
	  case XSW_UNITS_ENGLISH:
            options_win.units_pul.sel_item = 1;
            break;
          case XSW_UNITS_XSW:
            options_win.units_pul.sel_item = 2;
            break;
	}



	return;
}



/*
 *	Applies changes made in options win to all applicateable
 *	global variables.  Sanitizing will be performed on all values.
 */
int OptWinApplyChanges()
{
	int i, init_sound;
	int need_load;
	char *strptr, *strptr2;


	/* ************************************************************* */
	/* General. */

	/* Controller. */
#ifdef JS_SUPPORT
	if(INRANGEINC(options_win.control_type_pul.sel_item, 0, 2))
	{
	    option.controller = options_win.control_type_pul.sel_item;

            /* CONTROLLER_POINTER not supported, do not allow this. */
            if(option.controller == CONTROLLER_POINTER) 
                option.controller = CONTROLLER_KEYBOARD;

	    GCtlInit(option.controller);
	}
#else /* JS_SUPPORT */
        if(INRANGEINC(options_win.control_type_pul.sel_item,
            CONTROLLER_KEYBOARD, CONTROLLER_POINTER)
        )
        {
            option.controller = options_win.control_type_pul.sel_item;

            /* CONTROLLER_POINTER not supported, do not allow this. */
            if(option.controller == CONTROLLER_POINTER)
                option.controller = CONTROLLER_KEYBOARD;

            GCtlInit(option.controller);
        }
#endif /* JS_SUPPORT */

	/* Throttle mode. */
        if(INRANGEINC(options_win.throttle_mode_tba.armed_tb, 0, 2))
	    option.throttle_mode = options_win.throttle_mode_tba.armed_tb;

	/* Auto zoom. */
	option.auto_zoom = (options_win.auto_zoom_tb.state) ? 1 : 0;

	/* Local updates. */
	option.local_updates = (options_win.local_updates_tb.state) ? 1 : 0;

	/* Scanner contacts. */
	option.notify_scanner_contacts =
	    (options_win.notify_scanner_contacts_tb.state) ? 1 : 0;


	/*
	 *   NOTE: isrefs, ocsns, and sound scheme paths are updated
	 *   in the Misc section.
	 */


	/* *********************************************************** */
	/* Graphics. */

	/* Show heading arrow? */
	option.show_viewscreen_marks =
	    (options_win.show_viewscreen_marks_tb.state) ? 1 : 0;

	/* Show viewscreen labels? */
	i = options_win.show_viewscreen_labels_tba.armed_tb;
	if(INRANGEINC(i, 0, 3))
            option.show_viewscreen_labels = i;

        /* Formal object name labels. */
	i = options_win.show_formal_label_tba.armed_tb;
        if(INRANGEINC(i, 0, 2))
            option.show_formal_label = i;

	/* Async image loading. */
	option.async_image_loading = (
	    (options_win.async_image_loading_tb.state) ? 1 : 0
	);

        /* Async image loading pixels per cycle prompt. */
	i = PromptGetI(&options_win.async_image_pixels_prompt);
	if(i < 1)
	    i = 1;
	option.async_image_pixels = i;


        /* ************************************************************* */
	/* Sounds. */

        /* Sound server type. */
	i = options_win.server_type_tba.armed_tb;
        if(INRANGEINC(i, 0, 3))
        {
	    /* Must shutdown sound server if there is a change
	     * in the type of sound server.
	     */
	    if(sound.server_type != i)
	    {
		SoundShutdown();

		/* Now set new sound server type. */
		sound.server_type = i;
	    }
        }

	/* Sound Server start command. */
	strptr = PromptGetS(&options_win.sound_server_prompt);
	if(strptr != NULL)
	{
	    strncpy(sound.start_cmd, strptr, PATH_MAX + NAME_MAX);
	    sound.start_cmd[PATH_MAX + NAME_MAX - 1] = '\0';
	}

	/* Sound server connect argument. */
	strptr = PromptGetS(&options_win.sound_con_arg_prompt);
	if(strptr != NULL)
	{
            strncpy(sound.con_arg, strptr, MAX_URL_LEN);
	    sound.con_arg[MAX_URL_LEN - 1] = '\0';
	}

        /* Sound amount (must be checked after sound server type). */
	init_sound = 0;	/* Reset sound init/shutdown code. */
	i = options_win.sounds_tba.armed_tb;
        if(INRANGEINC(i, XSW_SOUNDS_NONE, XSW_SOUNDS_ALL))
        {
	    /* Sound server type set to none? */
	    if(sound.server_type <= SNDSERV_TYPE_NONE)
	    {
		/* Sound server type is set to none, so sounds level
		 * must be set to none as well.
		 */
		options_win.sounds_tba.armed_tb = 0;

		if(option.sounds > XSW_SOUNDS_NONE)
		{
		    init_sound = -1;	/* Need to shutdown sound. */
		    option.sounds = XSW_SOUNDS_NONE;
		}
		else
		{
		    init_sound = 0;
		}
	    }
	    else
	    {
		/* Sound server type is set to something. */

		/* Was off, need to turn on? */
		if((option.sounds <= XSW_SOUNDS_NONE) &&
		   (i > XSW_SOUNDS_NONE)
		)
		    init_sound = 1;	/* Need to initialize sound. */
		/* Was on, need to turn off? */
		else if((option.sounds > XSW_SOUNDS_NONE) &&
		        (i <= XSW_SOUNDS_NONE)
		)
		    init_sound = -1;	/* Need to shutdown sound. */
		else
		    init_sound = 0;	/* No change. */

		option.sounds = i;
	    }
        }


	/* Music. */
	option.music = ((options_win.music_tb.state) ? 1 : 0);
	/* Change background music. */
	if(option.music)
	    XSWDoChangeBackgroundMusic();
	else
	    SoundStopBackgroundMusic();

	/* Flip stereo. */
/* Leave this alone for now
	sound.flip_stereo = ((options_win.flip_stereo_tb.state) ? 1 : 0);
 */

        /* ************************************************************* */
	/* Network. */

	/* Default address. */
	if(options_win.def_address_prompt.buf != NULL)
	{
	    strncpy(
		net_parms.address,
		options_win.def_address_prompt.buf,
		MAX_URL_LEN
	    );
	    net_parms.address[MAX_URL_LEN - 1] = '\0';
	}

        /* Default port. */
        if(options_win.def_port_prompt.buf != NULL)
	{
	    net_parms.port = atoi(options_win.def_port_prompt.buf);
	    if((int)net_parms.port < 0)
		net_parms.port = 0;
	}

	/* Show network errors. */
	option.show_net_errors =
	    (options_win.show_net_errors_tb.state) ? 1 : 0;

        /* Show server errors. */
        option.show_server_errors = 
            (options_win.show_server_errors_tb.state) ? 1 : 0;

	/* Maximum net load. */
	if(options_win.max_net_load_prompt.buf != NULL)
	{
	    loadstat.net_load_max = atol(options_win.max_net_load_prompt.buf);

	    /* Cannot be 0. */
	    if(loadstat.net_load_max <= 0)
	    {
		sprintf(
		    options_win.max_net_load_prompt.buf,
		    "%i",
		    DEF_NET_LOAD_MAX
		);
		loadstat.net_load_max = DEF_NET_LOAD_MAX;
	    }
	}

	/* Update interval. */
	if(options_win.net_int_prompt.buf != NULL)
	{
	    net_parms.net_int = atol(options_win.net_int_prompt.buf);

	    /* Cannot be 0. */
	    if((long)net_parms.net_int <= 0)
	    {
                sprintf(options_win.net_int_prompt.buf, "%i",
                    SERVER_DEF_INT
                );
                net_parms.net_int = SERVER_DEF_INT;
	    }
	}

	/* Auto interval state. */
	auto_interval_tune.state =
	    ((options_win.auto_interval_tb.state) ? 1 : 0);


	/* ********************************************************* */
	/* Misc. */

        /* Local Toplevel. */
        strptr = PromptGetS(&options_win.xsw_local_toplevel_path_prompt);
        if(strptr != NULL)
        {
            if(ISPATHABSOLUTE(strptr))
                strncpy(dname.ltoplevel, strptr, PATH_MAX);
            else
                strncpy(
                    dname.ltoplevel,
                    PrefixPaths(dname.home, strptr),
                    PATH_MAX
                );
        }
        dname.ltoplevel[PATH_MAX - 1] = '\0';

	/* Toplevel. */
	strptr = PromptGetS(&options_win.xsw_toplevel_path_prompt);
	if(strptr != NULL)
	{
	    /* Apply only if full path. */
            if(ISPATHABSOLUTE(strptr))
		strncpy(dname.toplevel, strptr, PATH_MAX);
	    else
		PromptSetS(
		    &options_win.xsw_toplevel_path_prompt,
		    dname.toplevel
		);
	}
        dname.toplevel[PATH_MAX - 1] = '\0';

	/* Etc. */
        strptr = PromptGetS(&options_win.xsw_etc_path_prompt);
        if(strptr != NULL)
        {
            if(ISPATHABSOLUTE(strptr))
                strncpy(dname.etc, strptr, PATH_MAX);
            else
                strncpy(
                    dname.etc,
                    PrefixPaths(dname.toplevel, strptr),
                    PATH_MAX
                );
        }
        dname.etc[PATH_MAX - 1] = '\0';

	/* Images. */
        strptr = PromptGetS(&options_win.xsw_images_path_prompt);
        if(strptr != NULL)
        {
            if(ISPATHABSOLUTE(strptr))
                strncpy(dname.images, strptr, PATH_MAX);
            else
                strncpy(
                    dname.images,
                    PrefixPaths(dname.toplevel, strptr),
                    PATH_MAX
                );
        }
        dname.images[PATH_MAX - 1] = '\0';

	/* Sounds. */
	strptr = PromptGetS(&options_win.xsw_sounds_path_prompt);
	if(strptr != NULL)
	{
	    if(ISPATHABSOLUTE(strptr))
		strncpy(dname.sounds, strptr, PATH_MAX);
	    else
		strncpy(
		    dname.sounds,
                    PrefixPaths(dname.toplevel, strptr),
                    PATH_MAX
                );
	}
	dname.sounds[PATH_MAX - 1] = '\0';

	/* Downloads. */
        strptr = PromptGetS(&options_win.xsw_downloads_path_prompt);
        if(strptr != NULL)
        {
            if(ISPATHABSOLUTE(strptr))
                strncpy(dname.downloads, strptr, PATH_MAX);
            else
                strncpy(
                    dname.downloads,
                    PrefixPaths(dname.home, strptr),
                    PATH_MAX
                );
        }
        dname.downloads[PATH_MAX - 1] = '\0';

#ifdef JS_SUPPORT
	/* JS Calib. */
        strptr = PromptGetS(&options_win.js_calib_path_prompt);
        if(strptr != NULL)
        {
            if(ISPATHABSOLUTE(strptr))
	    {
                strncpy(fname.js_calib, strptr, PATH_MAX + NAME_MAX);
	    }
            else
	    {
		strptr2 = getenv("HOME");
		if(strptr2 != NULL)
                    strncpy(
                        fname.js_calib,
                        PrefixPaths(strptr2, strptr),
                        PATH_MAX + NAME_MAX
                    );
	    }
        }
        fname.js_calib[PATH_MAX + NAME_MAX - 1] = '\0';
#endif /* JS_SUPPORT */

	/* ISRef file. */
	strptr = options_win.isref_name_prompt.buf;
	need_load = 0;
        if(strptr != NULL)
        {
            StringStripSpaces(strptr);

            if(ISPATHABSOLUTE(strptr))
            {
		if(strcmp(fname.isr, strptr))
		    need_load = 1;

                strncpy(fname.isr, strptr, PATH_MAX + NAME_MAX);
            }
            else
            {
		strptr2 = PrefixPaths(dname.images, strptr);

		if(strptr2 != NULL)
		{
                    if(strcmp(fname.isr, strptr2))
                        need_load = 1;

		    strncpy(fname.isr, strptr2, PATH_MAX + NAME_MAX);
		}
            }
	    fname.isr[PATH_MAX + NAME_MAX - 1] = '\0';
        }
	/* Load as needed. */
	if(need_load)
	{
	    XSWLoadIsrefs(fname.isr);
	}


        /* OCSNs file. */
        strptr = options_win.ocs_name_prompt.buf;
        need_load = 0;
        if(strptr != NULL)
        {
            StringStripSpaces(strptr);

            if(ISPATHABSOLUTE(strptr))
            {
                if(strcmp(fname.ocsn, strptr))
                    need_load = 1;

                strncpy(fname.ocsn, strptr, PATH_MAX + NAME_MAX);
            }
            else
            {
                strptr2 = PrefixPaths(dname.etc, strptr);

                if(strptr2 != NULL)
                {
                    if(strcmp(fname.ocsn, strptr2))
                        need_load = 1;

                    strncpy(fname.ocsn, strptr2, PATH_MAX + NAME_MAX);
                }
            }
	    fname.ocsn[PATH_MAX + NAME_MAX - 1] = '\0';
        }
        /* Load as needed. */
        if(need_load)
        {   
            XSWLoadOCSN(fname.ocsn);
	}


        /* Sound scheme file. */
        strptr = options_win.ss_name_prompt.buf; 
        need_load = 0;
        if(strptr != NULL)
        {
            StringStripSpaces(strptr);

            if(ISPATHABSOLUTE(strptr))
            {
                if(strcmp(fname.sound_scheme, strptr))
                    need_load = 1;

                strncpy(fname.sound_scheme, strptr, PATH_MAX + NAME_MAX);
            }       
            else        
            {
                strptr2 = PrefixPaths(dname.sounds, strptr);

                if(strptr2 != NULL)
                {
                    if(strcmp(fname.sound_scheme, strptr2))
                        need_load = 1;
            
                    strncpy(fname.sound_scheme, strptr2, PATH_MAX + NAME_MAX);
                }
            }
            fname.sound_scheme[PATH_MAX + NAME_MAX - 1] = '\0';
        }
        /* Load as needed. */
        if(need_load)
        {   
            XSWLoadSS(fname.sound_scheme);
	}


	/* Units. */
        strptr = PUListGetSelItemName(&options_win.units_pul);
        if(strptr != NULL)
        {
            if(!strcmp(strptr, "AstroMetric"))
                option.units = XSW_UNITS_METRIC;
	    else if(!strcmp(strptr, "English"))
                option.units = XSW_UNITS_ENGLISH;
	    else if(!strcmp(strptr, "Program Internal"))
                option.units = XSW_UNITS_XSW;
	}


	/* ************************************************************ */
	/* Device refreshing. */

	/* Sound. */
	switch(init_sound)
	{
	  case 1:
	    if(sound.con_data == NULL)
	    {
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
		    /* Sound successfully initialized, now refresh
		     * sound resources.
		     */
		    if(option.music)
			XSWDoChangeBackgroundMusic();
 		}
	    }
	    break;

	  case -1:
            SoundShutdown();
	    break;

          default:
            break;
	}



	/* Reset modifications marker. */
	options_win.has_modifications = False;


	/* ********************************************************** */

	/* Need to redraw things that may display changed values. */
	BridgeWinDrawAll();



	return(0);
}



/*
 *      Sets all widget values to defaults.  Then calls
 *	OptWinApplyChanges() to apply default value changes.
 */
int OptWinLoadDefaults()
{
	printdw(&info_dw,
	    "OptWinLoadDefaults(): Function has not been coded yet.\n"
	);


        /* Mark has modifications. */
        options_win.has_modifications = True;


	return(0);
}



/*
 *      Applies all widget values to global and then saves changes.
 */
int OptWinSaveChanges()
{
	int status;


	/* Apply changes first. */
	OptWinApplyChanges();


	/* Save changes. */
	status = RCSaveToFile(fname.rc);
	if(status < 0)
        {
            printdw(&err_dw,
"Error saving option.\n\n\
Review the configuration file manually and\n\
check for any errors.\n"
            );
        }
	else
	{
	    printdw(&info_dw,
		"Options successfully saved.\n"
	    );
	}

        /* Reset modifications marker. */
        options_win.has_modifications = False;


        return(0);
}


/*
 *	Test sound button callback.
 */
int OptWinTestSoundPBCB(void *ptr)
{
        int status;


        /* Check if sound server was initialized. */
	if((sound.con_data == NULL) ||
	   (option.sounds <= XSW_SOUNDS_NONE) ||
	   (sound.server_type <= SNDSERV_TYPE_NONE)
	)
	{
            printdw(&info_dw,
"Sound has not been initialized. To initialize sound,\n\
select the sound server type that you are using and\n\
set the level of sounds to `all' then click on `apply'."
            );

	    return(-1);
	}


	/* Begin test. */
	if(1)
	{
            printdw(&info_dw,
		"Testing left channel...\n"
            );
	    sleep(1);
            /* Play a test sound (left). */
            status = SoundPlay(
                SOUND_CODE_DEFAULT,
                1.0,
		0.0,
                0,
                0
            );


            sleep(1);
            printdw(&info_dw,  
                "Testing right channel...\n"
            );
            /* Play a test sound (right). */
            status = SoundPlay(
                SOUND_CODE_DEFAULT,
                0.0,
                1.0,
                0,
                0
            );


            sleep(1);
            printdw(&info_dw,
                "Testing both channels...\n"
            );
            /* Play a test sound (both). */
            status = SoundPlay(
                SOUND_CODE_DEFAULT,
                1.0,
                1.0,
                0,
                0
            );
            sleep(1);

            printdw(&info_dw,
                "Sound server test completed.\n"
            );
        }


	return(0);
}



/*
 *	Primary file browser callback for options window specific loading.
 */
int OptWinFBCB(char *path)
{
	struct stat stat_buf;


	if(path == NULL)
	    return(-1);

	if(stat(path, &stat_buf))
	    return(-1);


	/* Check global variable pri_fb_loadop. */
	switch(pri_fb_loadop)
	{
	  /* ISRef prompt. */
	  case FB_MODE_CODE_ISRefs:
            PromptSetS(&options_win.isref_name_prompt, path);
	    PromptMarkBuffer(
		&options_win.isref_name_prompt,
		PROMPT_POS_END
	    );
	    break;

	  /* OCSN prompt. */
	  case FB_MODE_CODE_OCSNs:
            PromptSetS(&options_win.ocs_name_prompt, path);
            PromptMarkBuffer(
                &options_win.ocs_name_prompt,
                PROMPT_POS_END
            );
            break;

	  /* Sound scheme prompt. */
          case FB_MODE_CODE_SS:
            PromptSetS(&options_win.ss_name_prompt, path);
            PromptMarkBuffer(
                &options_win.ss_name_prompt,
                PROMPT_POS_END
            );
	    break;

#ifdef JS_SUPPORT
          /* Joystick calibration prompt. */
	  case FB_MODE_CODE_JSCalib:
	    PromptSetS(&options_win.js_calib_path_prompt, path);
            PromptMarkBuffer(
                &options_win.js_calib_path_prompt,
                PROMPT_POS_END
            );
	    break;
#endif /* JS_SUPPORT */

	  default:
	    break;
	}



	/* Reset the primary file browser callback pointers. */
	pri_fbrowser.func_ok = NULL;
	pri_fb_loadop = PRI_FB_LOADOP_NONE;


	return(0);
}



/*
 *	Browse files button callbacks, these procedures map the primary
 *	file browser and set it to report the loaded path to
 *	one of the option window's callback functions.
 */

int OptWinBrowseISRefsPBCB(void *ptr)
{
	char *strptr;
	char tmp_path[PATH_MAX + NAME_MAX];


        XSWDoUnfocusAllWindows();

	strptr = PrefixPaths(dname.images, FN_ISREF_EXT_MASK);
	strncpy(
	    tmp_path,
	    ((strptr == NULL) ? "/" : strptr),
	    PATH_MAX + NAME_MAX
	);
	tmp_path[PATH_MAX + NAME_MAX - 1] = '\0';


	FBrowserSetOpMesg(
	    &pri_fbrowser,
	    "Select Image Set Referance File",
            "OK"
	);

        pri_fbrowser.func_ok = OptWinFBCB;
        pri_fbrowser.func_cancel = NULL;

        FBrowserMapPath(&pri_fbrowser, tmp_path);
        pri_fb_loadop = FB_MODE_CODE_ISRefs;


	return(0);
}


int OptWinBrowseOCSNsPBCB(void *ptr)
{
        char *strptr;
        char tmp_path[PATH_MAX + NAME_MAX];


        XSWDoUnfocusAllWindows();


        strptr = PrefixPaths(dname.etc, FN_OCSN_EXT_MASK);
        strncpy(
            tmp_path,
            ((strptr == NULL) ? "/" : strptr),
            PATH_MAX + NAME_MAX
        );
        tmp_path[PATH_MAX + NAME_MAX - 1] = '\0';

        FBrowserSetOpMesg(
            &pri_fbrowser,
            "Select OCS Names File",
            "OK"
        );

        pri_fbrowser.func_ok = OptWinFBCB;
        pri_fbrowser.func_cancel = NULL;

        FBrowserMapPath(&pri_fbrowser, tmp_path);
        pri_fb_loadop = FB_MODE_CODE_OCSNs;


	return(0);
}


int OptWinBrowseSSPBCB(void *ptr)
{
        char *strptr;
        char tmp_path[PATH_MAX + NAME_MAX];


        XSWDoUnfocusAllWindows();


        strptr = PrefixPaths(dname.sounds, FN_SS_EXT_MASK);
        strncpy(
            tmp_path,
            ((strptr == NULL) ? "/" : strptr),
            PATH_MAX + NAME_MAX
        );
        tmp_path[PATH_MAX + NAME_MAX - 1] = '\0';

        FBrowserSetOpMesg(
            &pri_fbrowser,
            "Select Sound Scheme File",
            "OK"
        );

        pri_fbrowser.func_ok = OptWinFBCB;
        pri_fbrowser.func_cancel = NULL;

        FBrowserMapPath(&pri_fbrowser, tmp_path);
        pri_fb_loadop = FB_MODE_CODE_SS;


	return(0);
}


#ifdef JS_SUPPORT
int OptWinBrowseJSCalPBCB(void *ptr)
{
        char *strptr;
        char tmp_path[PATH_MAX + NAME_MAX];


        XSWDoUnfocusAllWindows();


        strptr = PrefixPaths(getenv("HOME"), DEF_JS_CALIBRATION_FILE);
        strncpy(
            tmp_path,
            ((strptr == NULL) ? "/" : strptr),
            PATH_MAX + NAME_MAX
        );
        tmp_path[PATH_MAX + NAME_MAX - 1] = '\0';

        FBrowserSetOpMesg(
            &pri_fbrowser,
            "Select Joystick Calibration File",
            "OK"
        );

        pri_fbrowser.func_ok = OptWinFBCB;
        pri_fbrowser.func_cancel = NULL;

        FBrowserMapPath(&pri_fbrowser, tmp_path);
	pri_fb_loadop = FB_MODE_CODE_JSCalib;


	return(0);
}
#endif /* JS_SUPPORT */


/*
 *	Edit keyboard mappings button callback.
 */
int OptWinKeymapPBCB(void *ptr)
{
	KeymapWinDoMapValues();

	return(0);
}

#ifdef JS_SUPPORT
/*
 *	Edit joystick mappings button callback.
 */
int OptWinJSMapPBCB(void *ptr)
{
	JSMWMapValues();

	return(0);
}
#endif	/* JS_SUPPORT */

/*
 *	Reclaim memory button callback.
 */
int OptWinRefreshMemoryPBCB(void *ptr)
{
	/* Clean, reclaim, memory and schedual next. */
	XSWReclaimGlobalMemory(False);

	return(0);
}
