// unvedit/isrefsfile.cpp



/*
                             ISRefs File Loading

	Functions:

	isref_struct *ISRefAllocate()

	isref_struct **ISRefLoadFromFile(
	        char *file,
	        int *total,
	        char *image_dir
	)



	---

 */
/*
#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
#include <string.h>
*/
#include <sys/types.h>
#include <sys/stat.h>

#include "../include/string.h"
#include "../include/cfgfmt.h"
#include "../include/fio.h"
#include "../include/disk.h"
#include "../include/tga.h"
#include "../include/isrefs.h"

#include "../include/unvmath.h"

#include "ue.h"


isref_struct *ISRefAllocate()
{
	isref_struct *ptr;

	ptr = (isref_struct *)calloc(
	    1,
	    sizeof(isref_struct)
	);

	/* Set to default values. */
	if(ptr != NULL)
	{
	    ptr->filename = NULL;
	    ptr->load_progress = ISREF_LOAD_PROGRESS_DONE;
	    ptr->option = 0;
	    ptr->merge_mode = ISREF_MERGE_NORMAL;
	    ptr->frame_determinant = ISREF_FDETERMINE_BY_HEADING;
	    ptr->layer_placement = ISREF_LAYER_FG;
	    ptr->effects = 0;
	    ptr->magnification = 1;
	    ptr->total_frames = 1;
            ptr->width = 0;
            ptr->height = 0;
            ptr->fwidth = 0;
            ptr->fheight = 0;
	    ptr->lib_data = NULL;
	    ptr->format_code = ISREF_FORMAT_CODE_UNKNOWN;
	    ptr->image_data = NULL;
	    ptr->point_light = NULL;
	    ptr->total_point_lights = 0;
	}


	return(ptr);
}


isref_struct **ISRefLoadFromFile(
	char *file,
	int *total,
	char *image_dir
)
{
        int i;
	char *strptr, *strptr2, *strptr3;
	char text[PATH_MAX + NAME_MAX + 256];

	FILE *fp;
        off_t filesize;
	struct stat stat_buf;

	char parm[CFG_PARAMETER_MAX];
	char val[CFG_VALUE_MAX];

	char tmp_filename[PATH_MAX + NAME_MAX];

	int lines_read = 0;
	int isref_num;
	int prev;
	isref_struct *isref_ptr;
	isref_struct **ptr = NULL;

	int pl_num;


	if(total == NULL)
	    return(ptr);

	*total = 0;

	if(file == NULL)
	    return(ptr);
	if(image_dir == NULL)
	    return(ptr);

	if(!ISPATHABSOLUTE(image_dir))
	    return(ptr);

	if(ISPATHABSOLUTE(file))
	{
	    strncpy(
                tmp_filename,
                file,
                PATH_MAX + NAME_MAX
            );
	}
	else
	{
	    strptr = PrefixPaths(image_dir, file);
	    strncpy(
		tmp_filename,
		((strptr == NULL) ? file : strptr),
		PATH_MAX + NAME_MAX
	    );
	}
	tmp_filename[PATH_MAX + NAME_MAX - 1] = '\0';

	if(stat(tmp_filename, &stat_buf))
	{
	    sprintf(text,
"No such Image Set Referance file:\n\n\
    %s",
		tmp_filename
	    );
	    printdw(&dialog, text);
	    return(ptr);
	}

	/* Get file size. */
	filesize = stat_buf.st_size;

	/* Open file. */
	fp = fopen(tmp_filename, "r");
	if(fp == NULL)
	{
            fprintf(stderr, "%s: Cannot open.\n", file);
	    return(ptr);
	}


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
		/* Too many isrefs loaded? */
		if(*total > ISREF_MAX)
		{
                    fprintf(stderr,
 "Warning: Maximum of %i isrefs loaded, additional isrefs not loaded.\n",
                        ISREF_MAX
                    );
		    free(strptr); strptr = NULL;
		    break;
		}


		/* Get isref number. */
		isref_num = atoi(val);
		if((isref_num < 0) || (isref_num >= ISREF_MAX))
                {
                    fprintf(stderr,
           "%s: Line %i: ISRef number `%i' out of range.\n",
                        file, lines_read, isref_num
                    );
                    continue;
                }

                /* Allocate a new isref in memory. */

                /* Need to allocate more object pointers? */
                if(isref_num >= *total)
                {
                    prev = *total;
                    *total = isref_num + 1;

                    ptr = (isref_struct **)realloc(
                        ptr,
                        *total * sizeof(isref_struct *)
                    );
                    if(ptr == NULL)
                    {
                        *total = 0;
                        continue;
                    }
                    /* Allocate each new object. */
                    for(i = prev; i < *total; i++)
                    {
                        ptr[i] = ISRefAllocate();
                    }
                }
                /* Allocate object structure as needed. */
                else if(ptr[isref_num] == NULL)
                {
                    ptr[isref_num] = ISRefAllocate();
                    if(ptr[isref_num] == NULL)
                    {
                        continue;
                    }
                }
                else
                {
                    fprintf(stderr,
              "%s: Line %i: Warning: Redefining isref %i.\n",
                        file, lines_read, isref_num
                    );
                }

		/* Get pointer to isref. */
		isref_ptr = ptr[isref_num];

		pl_num = 0;

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
			    strptr3 = PrefixPaths(image_dir, val);
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
                                file, lines_read, isref_ptr->filename
                            );
			}
			/* Make sure the image is a tga file. */
			else if(TgaTestFile(isref_ptr->filename) != TgaSuccess)
			{
                            fprintf(stderr,
                      "%s: Line %i: %s: Warning: Invalid tga image file.\n",
                                file, lines_read, isref_ptr->filename
                            );
                        }
                    }
                    /* LoadNow */
                    else if(!strcasecmp(parm, "LoadNow"))
                    {
			/* Ignore this. */
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
                            else        /* Default to "Normal". */
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
                            else        /* Default to "ByHeading". */
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
                            else        /* Default to Foreground. */
                                isref_ptr->layer_placement =
                                    ISREF_LAYER_FG;
                        }
                    }
                    /* Effects */
                    else if(!strcasecmp(parm, "Effects"))
                    {
			isref_ptr->effects = 0;

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
                                file, lines_read, val
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
                                file, lines_read, val
                            );
			    /* Fix number of frames. */
                            isref_ptr->total_frames = 1;
                        }
			/* Too many isref total frames? */
                        else if(isref_ptr->total_frames > ISREF_FRAMES_MAX)
                        {
                            fprintf(stderr,
    "%s: Line %i: Warning: TotalFrames value `%s' exceeds maximum %i.\n",
                                file, lines_read, val, ISREF_FRAMES_MAX
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
				file,
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
                                file,
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
                                    file, lines_read, isref_num
                                );
                            }
                            if(isref_ptr->magnification < 1)
                            {
                                fprintf(stderr,
    "%s: Line %i: Warning: Isref %i fixing magnification to 1.\n",
                                    file, lines_read,
                                    isref_num
                                );
                                isref_ptr->total_frames = 1;
                            }
                            if(isref_ptr->total_frames < 1)
                            {
                                fprintf(stderr,
    "%s: Line %i: Warning: Isref %i fixing 0 number of frames to 1.\n",
                                    file, lines_read,
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
                           file, lines_read, parm
                        );
                    }
		}
	    }

            /* Unsupported parameter. */
            else     
            {           
                fprintf(stderr,
                   "%s: Line %i: Warning: Unsupported parameter `%s'\n",
                   file, lines_read, parm
                );
            }
	}

	/* Close file. */
	fclose(fp);


	return(ptr);
}




