/*
                       Sound Server Communications

	Functions:

	int SoundInit()
	int SoundChangeMode(char *arg)
	int SoundPlay(
		int code,
		double left_volume,
		double right_volume,
		int effects,
		int priority
	)
	int SoundChangeBackgroundMusic(
	        int code,
	        int effects,
	        int priority
	)
	int SoundStopBackgroundMusic()
	int SoundManageEvents()
	void SoundShutdown()

	---

 */

#ifdef HAVE_ESD  
# include <esd.h>
#endif	/* HAVE_ESD */

#ifdef HAVE_Y2
# include <Y2/Y.h>	/* We get Y_H defined by #including this. */
# include <Y2/Ylib.h>
#endif	/* HAVE_Y2 */

#include "xsw.h"
#include "ss.h"


#ifdef Y_H
# ifndef DEF_Y_AUDIO_MODE_NAME
#  define DEF_Y_AUDIO_MODE_NAME		"Default"
# endif


#endif	/* Y_H */



/*
 *	Initializes the sound server, returns -1 on error.
 *
 *	Will return 0 and not initialize sound if option.sounds
 *	is XSW_SOUNDS_NONE.
 *
 *	sound.con_data must be NULL before calling this function!
 */
int SoundInit()
{
	/* Initialize by which sound server type: */
	switch(sound.server_type)
	{
	  case SNDSERV_TYPE_YIFF:
#ifdef Y_H
	    /* Connect to sound server. */
	    sound.con_data = (void *)YOpenConnection(
		sound.start_cmd,
		sound.con_arg
	    );
	    if(sound.con_data == NULL)
		return(-1);
#endif	/* Y_H */
	    break;

	  case SNDSERV_TYPE_ESOUND:
#ifdef ESD_H
	    /* Connect to sound server. */
	    sound.con_data = (void *)esd_open_sound(NULL);
	    if((int)sound.con_data < 0)
	    {
		sound.con_data = NULL;
		return(-1);
	    }
#endif /* ESD_H */
	    break;

	  case SNDSERV_TYPE_MIKMOD:
	    break;
	}


	/* Reset values. */
	sound.bkg_playid = NULL;
	sound.bkg_mood_code = -1;



	/* Load sound schemes from file. */
	SSLoadFromFile(fname.sound_scheme);


	/* A call to SoundChangeMode() would have been made if
	 * the sound scheme file specified an Audio mode (which
	 * it should).
	 *
	 * In case the sound scheme file did not specify an Audio
	 * mode, let that imply that we should leave the Audio
	 * mode as it is. So basically we don't change the
	 * Audio mode.
	 */
	if(*sound.audio_mode_name == '\0')
	{
#ifdef Y_H
/*
            strncpy(
                sound.audio_mode_name,
                DEF_Y_AUDIO_MODE_NAME,
                SNDSERV_AUDIO_MODE_NAME_MAX
	    );
	    
	    SoundChangeMode(sound.audio_mode_name);
 */
#endif
	}


	return(0);
}


/*
 *	Changes the Audio mode to the one specified in arg. Does not
 *	change global sound.audio_mode_name.
 *
 *	Sound server should already be initialized or else no operation
 *	is performed.
 */
int SoundChangeMode(char *arg)
{
	int status = 0;
#ifdef Y_H
	int i;
	YAudioModeValuesStruct **yaudio_mode;
	int total_yaudio_modes;
#endif	/* Y_H */


	/* Need to return success for this function if not connected
	 * to sound server.
	 */
        if(sound.con_data == NULL)
            return(status);


        /* Change mode by which sound server type: */
        switch(sound.server_type)
        {
          case SNDSERV_TYPE_YIFF:
#ifdef Y_H
	    /* If argument is NULL, then default to default Y Audio
	     * mode name.
	     */
            if(arg == NULL)
                arg = DEF_Y_AUDIO_MODE_NAME; 

            /* Get listing of available Audio mode. */
            yaudio_mode = YGetAudioModes(
                (YConnection *)sound.con_data,
                &total_yaudio_modes
            );
              
            /* Check if specified Audio mode is in list. */
	    for(i = 0; i < total_yaudio_modes; i++)
	    {
		if(yaudio_mode[i] == NULL)
		    continue;

		/* Audio mode names match? */
		if(!strcasecmp(yaudio_mode[i]->name, arg))
		    break;
	    }
            /* Free Audio modes listing. */
            YFreeAudioModesList(yaudio_mode, total_yaudio_modes);
	    /* Audio mode name not in list? */
	    if(i >= total_yaudio_modes)
	    {
		status = -1;
		break;
	    }


	    /* Change Audio mode. */
            status = YChangeAudioModePreset(
                (YConnection *)sound.con_data,
                arg		/* Audio mode name. */
            );
	    if(status < 0)
		status = -1;
	    else
		status = 0;
#endif	/* Y_H */
            break;
        
          case SNDSERV_TYPE_ESOUND:
            break;

          case SNDSERV_TYPE_MIKMOD:
            break;
        }

	return(status);
}


/*
 *	Plays a sound by given code number.
 */
int SoundPlay(
        int code,
        double left_volume,	/* 0.0 to 1.0 */
        double right_volume,	/* 0.0 to 1.0 */
        int effects,
        int priority		/* 0 or 1. */
)
{
	char *strptr;
#ifdef ESD_H
	int id;
	char buff[1024];
#endif	/* ESD_H */
#ifdef Y_H
	YID yid;
#endif	/* Y_H */


	if(sound.con_data == NULL)
	    return(-3);

	/* Get filename by sound code. */
	if(!SSIsAllocated(code))
	    return(-1);
	strptr = ss_item[code]->path;

        /* Play by which sound server type: */
        switch(sound.server_type)
        {
          case SNDSERV_TYPE_YIFF:
#ifdef Y_H
	    if(1)
	    {
	        YEventSoundPlay value;

	        value.flags = YPlayValuesFlagPosition |
                              YPlayValuesFlagTotalRepeats |
                              YPlayValuesFlagVolume |
                              YPlayValuesFlagSampleRate;
	        value.position = 0;
	        value.total_repeats = 1;
	        value.left_volume = left_volume;
	        value.right_volume = right_volume;
	        value.sample_rate = 0;
                yid = YStartPlaySoundObject(
		    (YConnection *)sound.con_data,
		    strptr,
		    &value
	        );
	    }
#endif	/* Y_H */
            break;
  
          case SNDSERV_TYPE_ESOUND:
#ifdef ESD_H
	    fprintf(stderr, "Playing '%s'...", strptr);
	    /* we should be able to map ss_items to sample id numbers... */
	    snprintf(buff, 1024, "xsw:%s", strptr);
	    if((id = esd_sample_getid((int)sound.con_data, buff)) < 0)
	    {
		fprintf(stderr,"(CACHING NOW)\n");
		id = esd_file_cache(
		    (int)sound.con_data,
		    "xsw",
		    strptr
		);
		if(id < 0)
		    fprintf(stderr, "esd_file_cache failed\n");
	    }
	    else
	    {
		fprintf(stderr, "(cached)\n");
	    }

            if(id < 0)
	    {
		fprintf(stderr, "sample caching failure!\n");
	    }
	    else
	    {
		esd_sample_stop((int)sound.con_data, id);

		/* reset the volume */
		if(esd_set_default_sample_pan(
		    (int)sound.con_data,
		    id,
		    static_cast<int>(left_volume * ESD_VOLUME_BASE),
		    static_cast<int>(right_volume * ESD_VOLUME_BASE)
		   ) < 0
		)
		    fprintf(stderr, "esd_set_default_sample_pan() failed\n");

		/* play our sample */
		if(esd_sample_play((int)sound.con_data, id) < 0)
		{
		    fprintf(stderr, "esd_sample_play() failed\n");
		}
		else
		{
/* Since we use the cached copies now.... don't free...
		    if(esd_sample_free((int)sound.con_data, id) < 0)
		    {
			fprintf(stderr, "esd_sample_free() failed\n");
		    }
 */
		}
	    }
#endif	/* ESD_H */
            break;
   
          case SNDSERV_TYPE_MIKMOD:
            break;
        }


	return(0);
}


/*
 *	Kills previous background mood music playback (if any)
 *	and then starts a new one specified by the sound code.
 *
 *	Globals sound.bkg_playid and sound.bkg_mood_code will be
 *	changed after a call to this function.
 */
int SoundChangeBackgroundMusic(
	int code,
        int effects,
        int priority            /* 0 or 1. */
)
{
        char *strptr = NULL;
#ifdef HAVE_ESD
	int id;
#endif /* HAVE_ESD */


        if(sound.con_data == NULL)
            return(-3);


	/* Get filename by sound code. */
        if(SSIsAllocated(code))
	    strptr = ss_item[code]->path;


        /* Play by which sound server type: */
        switch(sound.server_type)
        {
          case SNDSERV_TYPE_YIFF:
#ifdef Y_H
	    /* Kill previous background music play. */
	    if(sound.bkg_playid != NULL)
		YDestroyPlaySoundObject(
		    (YConnection *)sound.con_data,
		    (YID)sound.bkg_playid
		);

	    /* Begin new background music play. */
	    if(strptr != NULL)
	    {
                YEventSoundPlay value;

                value.flags = YPlayValuesFlagPosition |
                              YPlayValuesFlagTotalRepeats |
                              YPlayValuesFlagVolume |
                              YPlayValuesFlagSampleRate;
                value.position = 0;
                value.total_repeats = -1;	/* Repeat infinatly. */
                value.left_volume = 1.0;
                value.right_volume = 1.0;
                value.sample_rate = 0;

		sound.bkg_playid = (void *)YStartPlaySoundObject(
		    (YConnection *)sound.con_data,
		    strptr,
		    &value
		);
		if(sound.bkg_playid == NULL)
		    sound.bkg_mood_code = -1;
	    }
	    else
	    {
		sound.bkg_playid = NULL;
	    }
#endif  /* Y_H */
            break;

          case SNDSERV_TYPE_ESOUND:
#ifdef ESD_H
	    id = esd_file_cache((int)sound.con_data, "xsw", strptr);
	    if(id < 0)
	    {
		fprintf(stderr, "esd_file_cache() failed\n");
	    }
	    else
	    {
		/* now that we have the new music cached, stop & restart */
		SoundStopBackgroundMusic();
		// (int)sound.bkg_playid = id;  // Dan S: forbidden in ansi c++. Cast rvalue not lvalue.
		sound.bkg_playid = static_cast<void*>(&id);
		if(esd_sample_loop(
		    (int)sound.con_data,
		    (int)sound.bkg_playid)
		   < 0
		)
		    fprintf(stderr, "esd_sample_loop() failed\n");
	    }
#endif	/* ESD_H */
            break;

          case SNDSERV_TYPE_MIKMOD:
            break;
        }


	/* Update background mood code. */
	sound.bkg_mood_code = code;


        return(0);
} 

int SoundStopBackgroundMusic()
{
        if(sound.con_data == NULL)
            return(-3);


	switch(sound.server_type)
	{
	  case SNDSERV_TYPE_YIFF:
#ifdef Y_H
	    /* Kill previous background music play. */
	    if(sound.bkg_playid != NULL)
		YDestroyPlaySoundObject(
                    (YConnection *)sound.con_data,
                    (YID)sound.bkg_playid
		);
        
#endif  /* Y_H */
            break;

          case SNDSERV_TYPE_ESOUND:
#ifdef ESD_H
	    if(sound.bkg_playid != NULL)
	    {
		if(esd_sample_stop(
		    (int)sound.con_data,
		    (int)sound.bkg_playid)
		    < 0
		)
		    fprintf(stderr, "esd_sample_stop() failed\n");
		if(esd_sample_free(
		    (int)sound.con_data,
		    (int)sound.bkg_playid)
		    < 0
		)
		    fprintf(stderr, "esd_sample_stop() failed\n");
	    }
#endif  /* ESD_H */
            break;

          case SNDSERV_TYPE_MIKMOD:
            break;
        }

        sound.bkg_mood_code = -1;
	sound.bkg_playid = NULL;


	return(0);
}

/*
 *	Manages sound events if any, does not block.
 *
 *	Returns number of events handled.
 */
int SoundManageEvents()
{
	int events_handled = 0;

#ifdef Y_H
	YEvent yevent;
#endif	/* Y_H */


        switch(sound.server_type)
        {
          case SNDSERV_TYPE_YIFF:
#ifdef Y_H
            if(sound.con_data != NULL)   
            {
		while(
		    YGetNextEvent(
                        (YConnection *)sound.con_data,
                        &yevent,
                        False			/* Do not block. */
                    ) > 0
		)
		{
		    events_handled++;

		    switch(yevent.type)
		    {
		      /* Disconnect or shutdown. */
                      case YDisconnect: case YShutdown:
                        YCloseConnection(
                            (YConnection *)sound.con_data,
                            False
                        );
                        sound.con_data = NULL;

			/* If sound was set to in the options, then notify
			 * about this lost connection.
			 */
			if((option.sounds > XSW_SOUNDS_NONE) ||
                           (sound.server_type > SNDSERV_TYPE_NONE)
			)
			{
			    MesgAdd(
				"Lost connection to Y sound server.",
				xsw_color.bold_text
			    );

			    option.sounds = XSW_SOUNDS_NONE;
			    option.music = 0;
			    sound.server_type = SNDSERV_TYPE_NONE;
			}
			break;

		      /* Sound object play has stopped. */
                      case YSoundObjectKill:
			/* Check if this is the background mood music play. */
			if(option.music)
			{
			    if((yevent.kill.yid != YIDNULL) &&
                               (yevent.kill.yid == (YID)sound.bkg_playid)
			    )
                            {
				/* Background music play has stopped. */
				sound.bkg_playid = NULL;
			    }
			}
                        break;

                      case YMixerChannel:
                        break;
		    }
		}
            }
#endif	/* Y_H */
            break;
  
          case SNDSERV_TYPE_ESOUND:
            break;
 
          case SNDSERV_TYPE_MIKMOD:
            break;
        }


	return(events_handled);
}


/*
 *	Shuts down the sound server.
 */
void SoundShutdown()
{
	/* Delete all sound schemes. */
	SSDeleteAll();


	/* Disconnect from sound server as needed. */
        switch(sound.server_type)
        {
          case SNDSERV_TYPE_YIFF:
#ifdef Y_H
	    if(sound.con_data != NULL)
	    {
                /* Kill previous background music play. */
                if(sound.bkg_playid != NULL)
		{
                    YDestroyPlaySoundObject(
                        (YConnection *)sound.con_data,
                        (YID)sound.bkg_playid
                    );
		    sound.bkg_playid = NULL;
		}

		/* Close connection to server. */
		YCloseConnection(
		    (YConnection *)sound.con_data,
		    False
		);

		/* Reset pointer to connection to server, implying
		 * that we are not disconnected.
		 */
		sound.con_data = NULL;
	    }
#endif	/* Y_H */
            break;

          case SNDSERV_TYPE_ESOUND:
#ifdef ESD_H
	    if(sound.con_data != NULL)
	    {
		SoundStopBackgroundMusic();
		esd_close((int)sound.con_data);
	    }
#endif  /* ESD_H */
            break;

          case SNDSERV_TYPE_MIKMOD:
            break;
        }


	/* Reset values just in case. */
	if(sound.bkg_playid != NULL)
	    sound.bkg_playid = NULL;

	if(sound.con_data != NULL)
	    sound.con_data = NULL;


	return;
}
