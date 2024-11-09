/*
                        Sound Scheme File Loading

	Functions:

	int SSLoadFromFile(char *filename)

	---

 */

#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>

#include "../include/fio.h"
#include "../include/disk.h"
#include "../include/cfgfmt.h"
#include "../include/string.h"

#include "xsw.h"
#include "ss.h"


int SSLoadFromFile(char *filename)
{
        char *strptr, *strptr2, *strptr3;

        FILE *fp;
        off_t filesize;
        struct stat stat_buf;

        char parm[CFG_PARAMETER_MAX];
        char val[CFG_VALUE_MAX];

	int lines_read = 0;


        if(filename == NULL)
            return(-1);
        if(stat(filename, &stat_buf))
        {
            fprintf(stderr, "%s: No such file.\n",
                filename
            );
            return(-1);
        }

        /* Get size of file. */
        filesize = stat_buf.st_size;

        /* Open filename. */
        fp = fopen(filename, "r");
        if(fp == NULL)
        {
            fprintf(stderr, "%s: Cannot open.\n",
                filename
            );
            return(-1);
        }


        /* ********************************************************** */
        /* Delete all sound schemes currently in memory. */

        SSDeleteAll();


        /* ********************************************************* */

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
            /* Set value to "0" if NULL. */
            if(strptr2 == NULL) strptr2 = "0";
            strncpy(val, strptr2, CFG_VALUE_MAX);
            val[CFG_VALUE_MAX - 1] = '\0';


            /* Title */
            if(!strcasecmp(parm, "Title"))
            {
                /* Ignore this. */
            }
            /* ToplevelDir */
            else if(!strcasecmp(parm, "ToplevelDir"))
            {
                /* Ignore this. */
            }

	    /* AudioModeName */
	    else if(!strcasecmp(parm, "AudioModeName") ||
                    !strcasecmp(parm, "ModeName") ||
                    !strcasecmp(parm, "AudioMode")
	    )
	    {
		strncpy(
		    sound.audio_mode_name,
		    val,
		    SNDSERV_AUDIO_MODE_NAME_MAX
		);
		sound.audio_mode_name[SNDSERV_AUDIO_MODE_NAME_MAX - 1] = '\0';

		/* Change sound server mode as needed. */
		if(SoundChangeMode(sound.audio_mode_name))
		{
		    fprintf(stderr,
 "%s: Line %i: Warning: Cannot change to Audio mode `%s'.\n",
			filename, lines_read, val
		    );
		}
	    }


	    /* BeginSoundItem */
	    else if(!strcasecmp(parm, "BeginSoundItem") ||
                    !strcasecmp(parm, "BeginSoundSchemeItem")
	    )
	    {
		int ss_num;
		ss_item_struct *ss_ptr;

		/* Get number for this sound scheme item. */
		ss_num = atoi(val);

		/* Allocate sound scheme item. */
		if(SSAllocateExplicit(ss_num))
		    continue;
		ss_ptr = SSGetPtr(ss_num);
		if(ss_ptr == NULL)
		    continue;

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


                    /* Filename. */
                    if(!strcasecmp(parm, "Filename") ||
                       !strcasecmp(parm, "Path")
		    )
                    {
			free(ss_ptr->path);
			if(ISPATHABSOLUTE(val))
			{
			    ss_ptr->path = StringCopyAlloc(val);
			}
			else
			{
			    strptr3 = PrefixPaths(dname.sounds, val);
			    ss_ptr->path = StringCopyAlloc(strptr3);
			}

			/* Check if file exists. */
			if(ss_ptr->path != NULL)
			{
			    if(stat(ss_ptr->path, &stat_buf))
			    {
				fprintf(stderr,
 "%s: Line %i: Warning: Cannot find file `%s'\n",
				    filename,
				    lines_read,
                                    ss_ptr->path
				);
			    }
			}

                    }
                    /* EndSoundItem */
                    else if(!strcasecmp(parm, "EndSoundItem") ||
                            !strcasecmp(parm, "EndSoundSchemeItem")
		    )
                    {
                        break;
                    }
		}	/* BeginSoundItem */
	    }
	}

        /* Close file. */
        fclose(fp);


	return(0);
}
