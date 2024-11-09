// unvedit/isrefs.cpp

#include "../include/isrefs.h"



/*
                              ISRefs Management

	Functions:

	int ISRefIsLoaded(isref_struct *isref_ptr)
	int ISRefLoad(isref_struct *isref_ptr)
	void ISRefUnload(isref_struct *isref_ptr)
	void ISRefDelete(isref_struct *isref_ptr)
	void ISRefDeleteAll(isref_struct **isref_ptr, int total)
	void ISRefManage(isref_struct **isref_ptr, int total)

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
#include "../include/osw-x.h"

#include "../include/unvmath.h"

#include "ue.h"
#include "uew.h"

extern "C" long MilliTime(void);


/*
 *	For async image loading, how many pixels to load per
 *	isref per loop.
 */
#define ISREF_PIXELS_TO_LOAD_PER_ISREF	100000




int ISRefIsLoaded(isref_struct *isref_ptr)
{
	if(isref_ptr == NULL)
	    return(0);
        else if(isref_ptr->image_data == NULL)
            return(0);
	else
	    return(1);
}



int ISRefLoad(isref_struct *isref_ptr)
{
	int status;
	struct stat stat_buf;

        tga_data_struct *tga_data_ptr;


	if(isref_ptr == NULL)
	    return(-1);

	/* This isref not to have image? */
	if(isref_ptr->option & ISREF_OPT_NO_IMAGE)
	    return(-2);

	/* Already loaded? */
	if(ISRefIsLoaded(isref_ptr))
	    return(0);

	/* Missing filename? */
	if(isref_ptr->filename == NULL)
            return(-1);

	/* Is that filename pointing to an existing file? */
	if(stat(isref_ptr->filename, &stat_buf))
            return(-1);


	/* Set up for passive async loading. */
	if(1)
	{
/* Explicitly set file format for now. */
isref_ptr->format_code = ISREF_FORMAT_CODE_TGA;

            switch(isref_ptr->format_code)
            {
              /* TGA format. */
              case ISREF_FORMAT_CODE_TGA:
                isref_ptr->lib_data = (void *)calloc(
                    1,
                    sizeof(tga_data_struct)
                );
                if(isref_ptr->lib_data == NULL)
                {
                    isref_ptr->format_code = ISREF_FORMAT_CODE_UNKNOWN;
                    return(-1);
                }
                tga_data_ptr = (tga_data_struct *)isref_ptr->lib_data;

		/* Set up for partial tga read. */
		status = TgaStartReadPartialFromFile(
                    isref_ptr->filename,
                    tga_data_ptr,
                    osw_gui[0].depth
		);
                if(status != TgaSuccess)
                {
                    fprintf(stderr,
     "ISRefLoad(): %s: TgaStartReadPartialFromFile() error code %i.\n",
                        isref_ptr->filename, status
                    );

                    /* Destroy the tga_data. */
                    TgaDestroyData(tga_data_ptr);

                    isref_ptr->format_code = ISREF_FORMAT_CODE_UNKNOWN;

                    free(isref_ptr->lib_data);
                    isref_ptr->lib_data = NULL;

                    return(-1);
                }

                /* Link image_data pointer. */
                isref_ptr->image_data = (u_int8_t *)tga_data_ptr->data;

                /* Set width and height from tga_data. */
                isref_ptr->width = tga_data_ptr->width;
                isref_ptr->height = tga_data_ptr->height;

                /* Mark load process as currently loading. */
                isref_ptr->load_progress = ISREF_LOAD_PROGRESS_LOADING;
                break;

              /* Unknown format. */
              default:
                break;
            }

            /* Sanitize number of frames specified on isref entry. */
            if(isref_ptr->total_frames > ISREF_FRAMES_MAX)
                isref_ptr->total_frames = ISREF_FRAMES_MAX;
            if(isref_ptr->total_frames < 1)
                isref_ptr->total_frames = 1;

            /* Calculate frame width and height. */
            isref_ptr->fwidth = isref_ptr->width;
            isref_ptr->fheight = (int)isref_ptr->height /
                (int)isref_ptr->total_frames;
	}


	return(0);
}


void ISRefUnload(isref_struct *isref_ptr)
{
	if(isref_ptr == NULL)
	    return;

	if(!ISRefIsLoaded(isref_ptr))
	    return;


	/* Unload by format. */
        switch(isref_ptr->format_code)
        {
          case ISREF_FORMAT_CODE_TGA:
            if(isref_ptr->lib_data != NULL)
            {
                TgaDestroyData((tga_data_struct *)isref_ptr->lib_data);
                free(isref_ptr->lib_data);
                isref_ptr->lib_data = NULL;
            }
            break;

          default:
            if(isref_ptr->lib_data != NULL)
            {
                free(isref_ptr->lib_data);
                isref_ptr->lib_data = NULL;
            }
            break; 
        }   


        /*   Set image data pointer to NULL (do not free it, the call
         *   to library destroy function has already done that).
         */
        isref_ptr->image_data = NULL;

        isref_ptr->format_code = ISREF_FORMAT_CODE_UNKNOWN;
        isref_ptr->load_progress = ISREF_LOAD_PROGRESS_DONE;


	return;
}


void ISRefDelete(isref_struct *isref_ptr)
{
        int i;
                
                
        if(isref_ptr == NULL)
            return;


	/* Unload image. */
	ISRefUnload(isref_ptr);

	/* Free file name. */
        free(isref_ptr->filename);
        isref_ptr->filename = NULL;

        /* Free all point lights. */
        for(i = 0; i < isref_ptr->total_point_lights; i++)
            free(isref_ptr->point_light[i]);
        free(isref_ptr->point_light);
        isref_ptr->point_light = NULL;
        
        isref_ptr->total_point_lights = 0;


	/* Free structure itself. */
	free(isref_ptr);
	isref_ptr = NULL;


	return;
}


void ISRefDeleteAll(isref_struct **isref_ptr, int total)
{
	int i;

	for(i = 0; i < total; i++)
	{
	    ISRefDelete(isref_ptr[i]);
	}

	free(isref_ptr);
	isref_ptr = NULL;

	total = 0;


	return;
}

int CHECK_BLANK(u_int16_t *buf, unsigned int width, unsigned int height)
{
	int b;
	int r = 0, c = 0;

	while(r < (int)height)
	{
	    b = 1;
	    while(c < (int)width)
	    {
	        if(buf[(r * width * 2) + (c * 2)] != 0x0000)
		    b = 0;

		c++;
	    }
	    if(b)
		printf("Line %i appears to be blank.\n", r);

	    r++;
	}

	return(0);
}

/*
 *	ISRef management.
 *
 *	Manages:
 *
 *	Continuing loading of isref's image segment as needed
 *	when passive loading is requested.
 *
 *	Changing of strobe state for isref's point lights.
 */
void ISRefManage(isref_struct **isref_ptr, int total)
{
        int i, p, status;
	int pixels_loaded = 0;
        isref_struct **ptr;
	tga_data_struct *tga_data_ptr;

	isref_point_light_struct **point_light_ptr;



	/* Go through each isref. */
	for(i = 0, ptr = isref_ptr;
            i < total;
            i++, ptr++
	)
	{
	    if(*ptr == NULL)
		continue;

	    /* ***************************************************** */
            /* Point light strobing update. */

            for(p = 0, point_light_ptr = (*ptr)->point_light;
                p < (*ptr)->total_point_lights;
                p++, point_light_ptr++
	    )
	    {
		if(*point_light_ptr == NULL)
		    continue;

		/* Is this point light suppose to strobe? */
                if(((*point_light_ptr)->strobe_off_int > 0) &&
                   ((*point_light_ptr)->strobe_on_int > 0)
                )
                {
		    if((*point_light_ptr)->strobe_state)
                    {
			/* Strobe is in on interval. */

			if((*point_light_ptr)->strobe_next <= MilliTime())
                        {
                            /* Strobe just turned off. */
                            (*point_light_ptr)->strobe_state = 0;

                            (*point_light_ptr)->strobe_next = MilliTime() +
                                (*point_light_ptr)->strobe_off_int;
                        }
		    }
		    else
                    {
                        /* Strobe is in off interval. */
                        if((*point_light_ptr)->strobe_next <= MilliTime())
                        {
                            /* Strobe just turned on. */
                            (*point_light_ptr)->strobe_state = 1;

                            (*point_light_ptr)->strobe_next = MilliTime() +
                                (*point_light_ptr)->strobe_on_int;
                        }
		    }
		}
	    }


	    /* ***************************************************** */
	    /* Passive image loading. */

	    /*   Can we still load more pixels and is this isref's image
	     *   in the loading process?
	     */
            if((pixels_loaded < 100000) &&
	       ((*ptr)->load_progress == ISREF_LOAD_PROGRESS_LOADING)
	    )
	    {
		/* Library data not allocated? */
		if((*ptr)->lib_data == NULL)
		    continue;

		/* Continue loading by format. */
		switch((*ptr)->format_code)
		{
		  case ISREF_FORMAT_CODE_TGA:
		    tga_data_ptr = (tga_data_struct *)(*ptr)->lib_data;
		    status = TgaReadPartialFromFile(
			tga_data_ptr,
			osw_gui[0].depth,
			ISREF_PIXELS_TO_LOAD_PER_ISREF
		    );
		    if(status != TgaSuccess)
		    {
			/* Load error, reset isref and load as default. */
			(*ptr)->load_progress = ISREF_LOAD_PROGRESS_DONE;
		    }
		    /* Done loading? */
		    if(tga_data_ptr->cur_load_pixel < 0)
		    {
			/* Image is done loading, mark it as such. */
			(*ptr)->load_progress = ISREF_LOAD_PROGRESS_DONE;
		    }
		    /* Increment pixels loaded. */
		    pixels_loaded += ISREF_PIXELS_TO_LOAD_PER_ISREF;
		    break;

		  default:
                    (*ptr)->load_progress = ISREF_LOAD_PROGRESS_DONE;
		    break;
		}
	    }
	}


	/* Check if some graphics were loaded, if so, redraw. */
	if(pixels_loaded > 0)
	{
	    for(i = 0; i < total_uews; i++)
	    {
		if(uew[i] == NULL)
		    continue;

		if(!uew[i]->map_state)
		    continue;

		UEWDraw(i, UEW_DRAW_PREVIEW);
	    }
	}


	return;
}




