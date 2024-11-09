/*
                     Imageset Referances load from File

	Functions:

	int ISRefLoadFromFile(char *filename)

	---
 */

#include "../include/cfgfmt.h"
#include "../include/fio.h"

#include "../include/tga.h"
#include "../include/unvmath.h"

#include "xsw.h"



int ISRefLoadFromFile(char *filename)
{
        int status;
	char *strptr, *strptr2, *strptr3;
	char stringa[1024];

	FILE *fp;
        off_t filesize;
	struct stat stat_buf;

	char parm[CFG_PARAMETER_MAX];
	char val[CFG_VALUE_MAX];

	char tmp_filename[PATH_MAX + NAME_MAX];

	int pl_num;
	int lines_read = 0;
	int isrefs_read = 0;
	int isref_num = -1;

	isref_struct *isref_ptr = NULL;


	/* Error checks. */
	if(filename == NULL)
	    return(-1);
	if(stat(filename, &stat_buf))
	{
	    fprintf(stderr, "%s: No such file.\n", filename);
	    return(-1);
	}

	/* Get file size. */
	filesize = stat_buf.st_size;

	/* Open file. */
	fp = fopen(filename, "r");
	if(fp == NULL)
	{
            fprintf(stderr, "%s: Cannot open.\n", filename);
	    return(-1);
	}


	/* ********************************************************** */
	/* Delete all isrefs currently in memory. */

	ISRefDeleteAll();


        /* ********************************************************** */

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


            /* BeginISRefEntry */
            if(!strcasecmp(parm, "BeginISRefEntry"))
            {
		/* Check if too many isrefs are being loaded. */
		if(isrefs_read >= (ISREF_MAX - 1))
		{
                    fprintf(stderr,
 "Warning: Maximum of %i isrefs loaded, additional isrefs not loaded.\n",
                        ISREF_MAX
                    );
		    free(strptr); strptr = NULL;
		    break;
		}

		/* Increment isrefs_read. */
		isrefs_read++;

		/* Get isref_num from parameter's value. */
		isref_num = atol(val);

		/* Check filename, is this ISREF already allocated? */
		if(ISRefIsAllocated(isref_num))
		{
		    if(isref[isref_num]->filename != NULL)
		    {
			fprintf(stderr,
			    "Warning: ISRef %i: Redefined.\n",
			    isref_num
			);
		    }
		}


		/* Allocate memory for new ISRef. */
		status = ISRefCreateExplicit(isref_num);
		if(status)
                {
                    fprintf(
			stderr,
                        "%s: Line %i: Could not allocate isref entry %i.\n",
                        filename, lines_read, isref_num
                    );
                    free(strptr);
		    strptr = NULL;

                    break;
                }
		else
		{
		    isref_ptr = isref[isref_num];
		}

		/* Reset substructure numbers. */
		pl_num = 0;	/* Point lights. */



                /* Begin reading information for this isref. */
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


                    /* FileName */
                    if(!strcasecmp(parm, "FileName") ||
                       !strcasecmp(parm, "File") ||
		       !strcasecmp(parm, "Name") ||
                       !strcasecmp(parm, "Image")
		    )
                    {
			/* Free filename if repeat. */
			free(isref_ptr->filename);
			isref_ptr->filename = NULL;

                        /* Set full path? */
                        if(ISPATHABSOLUTE(val))
                        {
			    isref_ptr->filename = StringCopyAlloc(val);
			}
			else
			{
			    strptr3 = PrefixPaths(dname.images, val);
			    strncpy(
				tmp_filename,
			        (strptr3 == NULL) ? val : strptr3,
				PATH_MAX + NAME_MAX
			    );
			    isref_ptr->filename = StringCopyAlloc(tmp_filename);
			}

			/* Make sure that file exists. */
			if(stat(isref_ptr->filename, &stat_buf))
			{
                            fprintf(stderr,
                                "%s: Line %i: %s: Warning: No such file.\n",
                                filename, lines_read, isref_ptr->filename
                            );
			    sprintf(stringa,
"Warning: Unable to find image file:\n\n    %s\n\n\
Specified in:\n\n    %s\n",
				isref_ptr->filename,
				filename
			    );
			    printdw(&err_dw, stringa);
			}
			/* Make sure the image is a tga file. */
			else if(TgaTestFile(isref_ptr->filename) != TgaSuccess)
			{
                            fprintf(stderr,
              "%s: Line %i: %s: Warning: Invalid tga image file.\n",
                                filename, lines_read, isref_ptr->filename
                            );
                            sprintf(stringa,
"Error: Invalid TGA image file:\n\n    %s\n\n\
Specified in:\n\n    %s\n",
                                isref_ptr->filename,
				filename
                            );
                            printdw(&err_dw, stringa);
                        }
                    }
                    /* LoadNow */
                    else if(!strcasecmp(parm, "LoadNow"))
                    {
			status = ISRefLoad(isref_num);
                        if(status)
                        {
                            fprintf(stderr,
                          "%s: Line %i: Error: Cannot load isref %i.\n",
                                filename, lines_read, isref_num
                            );

			    if(isref_ptr->option & ISREF_OPT_NO_IMAGE)
                                sprintf(stringa,
"Warning: Isref number %i specified in:\n\n    %s\n\n\
Has conflicting settings of NoImage and LoadNow.\n",
                                    isref_num,
                                    filename
                                );
			    else
			        sprintf(stringa,
    "Warning: Unable to load isref number %i specified in\n%s.\n",
                                    isref_num,
				    filename
                                );
                            printdw(&err_dw, stringa);
			}
                    }
                    /* NoImage */
                    else if(!strcasecmp(parm, "NoImage"))
                    {
			isref_ptr->option |= ISREF_OPT_NO_IMAGE;
                    }
                    /* StayLoaded */
                    else if(!strcasecmp(parm, "StayLoaded"))
                    {
                        isref_ptr->option |= ISREF_OPT_STAY_LOADED;
                    }
                    /* MergeMode */
                    else if(!strcasecmp(parm, "MergeMode"))
                    {
			if(isdigit(*val))
			{
			    isref_ptr->merge_mode = atoi(val);
			}
			else
			{
                            if(!strcasecmp(val, "Additive"))
                                isref_ptr->merge_mode =
				    ISREF_MERGE_ADDITIVE;
			    else if(!strcasecmp(val, "Subtractive"))
				isref_ptr->merge_mode =
                                    ISREF_MERGE_SUBTRACTIVE;
			    else	/* Default to "Normal". */
				isref_ptr->merge_mode =
				    ISREF_MERGE_NORMAL;
			}
		    }
                    /* HasTransparency */
                    else if(!strcasecmp(parm, "HasTransparency"))
                    {
                        isref_ptr->option |= ISREF_OPT_HAS_TRANSPARENCY;
                    }
                    /* FrameDeterminat */
                    else if(!strcasecmp(parm, "FrameDeterminat"))
                    {
			if(isdigit(*val))
			{
			    isref_ptr->frame_determinant = atoi(val);
			}
			else
			{
			    if(!strcasecmp(val, "ByAnimation"))
				isref_ptr->frame_determinant =
				    ISREF_FDETERMINE_BY_ANIMATION;
			    else	/* Default to "ByHeading". */
				isref_ptr->frame_determinant =
				    ISREF_FDETERMINE_BY_HEADING;
			}
                    }
                    /* LayerPlacement */
                    else if(!strcasecmp(parm, "LayerPlacement"))
                    {
                        if(isdigit(*val))
                        {
                            isref_ptr->layer_placement = atoi(val);
                        }
                        else
                        {
                            if(!strcasecmp(val, "BackgroundTiled") ||
                               !strcasecmp(val, "BgTiled")
			    )
				isref_ptr->layer_placement =
				    ISREF_LAYER_BG_TILED;
                            else if(!strcasecmp(val, "BackgroundStatic") ||
                                    !strcasecmp(val, "BgStatic")
                            )
                                isref_ptr->layer_placement =
                                    ISREF_LAYER_BG_STATIC;
			    else	/* Default to Foreground. */
				isref_ptr->layer_placement =
				    ISREF_LAYER_FG;
			}
                    }
                    /* Effects */
                    else if(!strcasecmp(parm, "Effects"))
                    {
			isref_ptr->effects = 0;

			strtoupper(val);
			strptr3 = strstr(val, "STARGLOW");
			if(strptr3 != NULL)
			    isref_ptr->effects |= ISREF_EFFECTS_STARGLOW;
                        strptr3 = strstr(val, "FADEINGLOW");
                        if(strptr3 != NULL)
                            isref_ptr->effects |= ISREF_EFFECTS_FADEINGLOW;
                        strptr3 = strstr(val, "FADEOUTGLOW");
                        if(strptr3 != NULL)
                            isref_ptr->effects |= ISREF_EFFECTS_FADEOUTGLOW;
                    }
                    /* Magnification */
                    else if(!strcasecmp(parm, "Magnification"))
                    {
                        isref_ptr->magnification = atof(val);
			if(isref_ptr->magnification < 1)
			{
                            fprintf(stderr,
         "%s: Line %i: Warning: Magnification value `%s' too small.\n",
                                filename, lines_read, val
                            );
			    isref_ptr->magnification = 1;
			}
                    }

                    /* TotalFrames */
                    else if(!strcasecmp(parm, "TotalFrames") ||
                            !strcasecmp(parm, "Frames")
		    )
                    {
                        isref_ptr->total_frames = atoi(val);
			/* Too few isref total frames? */
                        if(isref_ptr->total_frames < 1)
                        {
                            fprintf(stderr,
         "%s: Line %i: Warning: TotalFrames value `%s' too small.\n",
                                filename, lines_read, val
                            );
			    /* Fix number of frames. */
                            isref_ptr->total_frames = 1;
                        }
			/* Too many isref total frames? */
                        else if(isref_ptr->total_frames > ISREF_FRAMES_MAX)
                        {
                            fprintf(stderr,
    "%s: Line %i: Warning: TotalFrames value `%s' exceeds maximum %i.\n",
                                filename, lines_read, val, ISREF_FRAMES_MAX
                            );
                            /* Fix number of frames. */
                            isref_ptr->total_frames = ISREF_FRAMES_MAX;
                        }
                    }
                    /* Width */
                    else if(!strcasecmp(parm, "Width"))
                    {
			/* Calculated by information on image file. */
		    }
		    /* Height */
                    else if(!strcasecmp(parm, "Height"))
                    {
			/* Calculated by information on image file. */
                    }
                    /* FrameWidth */
                    else if(!strcasecmp(parm, "FrameWidth"))
                    {
			/* Calculated by information on image file. */
                    }
                    /* FrameHeight */
                    else if(!strcasecmp(parm, "FrameHeight"))
                    {
			/* Calculated by information on image file. */
                    }

		    /* ************************************************* */
		    /* BeginPointLight */
		    else if(!strcasecmp(parm, "BeginPointLight"))
		    {
			/* Allocate a new structure. */
			isref_ptr->total_point_lights = pl_num + 1;

			isref_ptr->point_light = (isref_point_light_struct **)realloc(
			    isref_ptr->point_light,
			    isref_ptr->total_point_lights *
                            sizeof(isref_point_light_struct *)
			);
			if(isref_ptr->point_light == NULL)
			{
			    fprintf(stderr,
                       "%s: Cannot allocate %i point light pointers.\n",
				filename,
				isref_ptr->total_point_lights
			    );
			    isref_ptr->total_point_lights = 0;
                            free(strptr); strptr = NULL;
			    break;
			}

			isref_ptr->point_light[pl_num] = (isref_point_light_struct *)calloc(
			    1,
			    sizeof(isref_point_light_struct)
			);
			if(isref_ptr->point_light[pl_num] == NULL)
			{
			    fprintf(stderr,
                       "%s: Cannot allocate point light %i.\n",   
                                filename,
				pl_num
                            );
                            isref_ptr->total_point_lights--;
                            free(strptr); strptr = NULL;                        
                            break;
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


			  /* Theta */
                          if(!strcasecmp(parm, "Theta"))
                          {
                              isref_ptr->point_light[pl_num]->theta =
                                  SANITIZERADIANS(atof(val));
                          }
                          /* Radius */
                          else if(!strcasecmp(parm, "Radius"))
                          {
                              isref_ptr->point_light[pl_num]->radius =
                                  atof(val);
                          }

                          /* ColorAlpha */
                          else if(!strcasecmp(parm, "ColorAlpha"))
                          {
                              isref_ptr->point_light[pl_num]->a =
                                  atoi(val);
                          }
                          /* ColorRed */
                          else if(!strcasecmp(parm, "ColorRed"))
                          {
                              isref_ptr->point_light[pl_num]->r =
                                  atoi(val);
                          }
                          /* ColorGreen */
                          else if(!strcasecmp(parm, "ColorGreen"))
                          {
                              isref_ptr->point_light[pl_num]->g =
                                  atoi(val);
                          }
                          /* ColorBlue */
                          else if(!strcasecmp(parm, "ColorBlue"))
                          {
                              isref_ptr->point_light[pl_num]->b =
                                  atoi(val);
                          }

                          /* StrobeOffInterval */
                          else if(!strcasecmp(parm, "StrobeOffInterval"))
                          {
                              isref_ptr->point_light[pl_num]->strobe_off_int =
                                  atol(val);
                          }
                          /* StrobeOnInterval */
                          else if(!strcasecmp(parm, "StrobeOnInterval"))
                          {
                              isref_ptr->point_light[pl_num]->strobe_on_int =
                                  atol(val);
                          }

                          /* EndPointLight */
                          else if(!strcasecmp(parm, "EndPointLight"))
                          {
             
                              /* End of entry, so break. */
                              break;
                          }
			}

			/* Increment pl_num. */
			pl_num++;
		    }



                    /* EndISRefEntry */
                    else if(!strcasecmp(parm, "EndISRefEntry"))
                    {
			/* Check and warn about any errors. */
			if(!(isref_ptr->option & ISREF_OPT_NO_IMAGE))
			{
                            if(isref_ptr->filename == NULL)
                            {
                                fprintf(stderr,
             "%s: Line %i: Warning: Isref %i has no filename.\n",
                                    filename, lines_read, isref_num
                                );
                            }
                            if(isref_ptr->magnification < 1)
                            {
                                fprintf(stderr,
    "%s: Line %i: Warning: Isref %i fixing magnification to 1.\n",
                                    filename, lines_read,
                                    isref_num
                                );
                                isref_ptr->total_frames = 1;
                            }
                            if(isref_ptr->total_frames < 1)
                            {
                                fprintf(stderr,
    "%s: Line %i: Warning: Isref %i fixing 0 number of frames to 1.\n",
                                    filename, lines_read,
                                    isref_num
                                );
			        isref_ptr->total_frames = 1;
                            }
			}

			/* End of entry, so break. */
                        break;
                    }

                    /* Unsupported parameter. */
                    else
                    {
                        fprintf(stderr,
                           "%s: Line %i: Warning: Unsupported parameter `%s'\n",
                           filename, lines_read, parm
                        );
                    }
		}
	    }

            /* Unsupported parameter. */
            else     
            {           
                fprintf(stderr,
                   "%s: Line %i: Warning: Unsupported parameter `%s'\n",
                   filename, lines_read, parm
                );
            }
	}

	/* Close file. */
	fclose(fp);


	/* First isref MUST be valid! */
	if(!ISRefIsLoaded(ISREF_DEFAULT))
	{
	    if(ISRefLoadAsDefault(ISREF_DEFAULT))
	    {
	        fprintf(stderr,
	"ISRefLoadListFromFile(): Error: Default isref %i is invalid.\n",
		    ISREF_DEFAULT
	        );
		return(-1);
	    }
	}


	return(0);
}
