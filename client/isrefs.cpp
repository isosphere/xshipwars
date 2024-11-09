/*
                          Imageset Referances

	Functions:

	int ISRefIsAllocated(int isref_num)
	int ISRefIsLoaded(int isref_num)

	void ISRefReset(int isref_num)
	int ISRefCreateExplicit(int isref_num)

	int ISRefLoadAsDefault(int isref_num)
	int ISRefLoad(int isref_num)
	void ISRefUnload(isref_num)

	void ISRefDelete(int isref_num)
	void ISRefDeleteAll()

	void ISRefReclaimMemory()
	void ISRefManage()

	---


 */

#include "../include/tga.h"
#include "xsw.h"
#include "net.h"


#include "ct_data/images/unknown.h"


/*
 *	For async image loading, how many pixels to load per
 *	isref per loop.
 */
#define ISREF_PIXELS_TO_LOAD_PER_ISREF	100000


/*
 *	Checks if the ISRef is allocated (does not check if its loaded).
 */
int ISRefIsAllocated(int isref_num)
{
	if((isref == NULL) ||
	   (isref_num < 0) ||
           (isref_num >= total_isrefs)
	)
	    return(0);
	else if(isref[isref_num] == NULL)
	    return(0);
	else
	    return(1);
}

/*
 *	Checks if the isref is allocated and loaded.
 *
 *	Note: This does NOT check if isref has ISREF_OPT_NO_IMAGE
 *	set.
 */
int ISRefIsLoaded(int isref_num)
{
	/* Make sure isref_num is a valid number and allocated. */
	if(!ISRefIsAllocated(isref_num))
	    return(0);
        else if(isref[isref_num]->image_data == NULL)
            return(0);
	else
	    return(1);
}

/*
 *	Frees and resets all allocated substructures.
 */
void ISRefReset(int isref_num)
{
	int i;
	isref_struct *isref_ptr;


	if(ISRefIsAllocated(isref_num))
	    isref_ptr = isref[isref_num];
	else
	    return;

	/* Free filename. */
	free(isref_ptr->filename);
	isref_ptr->filename = NULL;

	/* Free library image data and reset image data pointer on
	 * all isrefs that referance the image data pointer on
	 * this isref.
	 */
	ISRefUnload(isref_num);


	isref_ptr->format_code = ISREF_FORMAT_CODE_UNKNOWN;
	isref_ptr->load_progress = ISREF_LOAD_PROGRESS_DONE;
	isref_ptr->option = 0;

	/* Reset to default values. */
        isref_ptr->merge_mode = ISREF_MERGE_NORMAL;
        isref_ptr->frame_determinant = ISREF_FDETERMINE_BY_HEADING;
        isref_ptr->layer_placement = ISREF_LAYER_FG;
        isref_ptr->effects = 0;
	isref_ptr->magnification = 1;

        isref_ptr->total_frames = 1;

        isref_ptr->width = 0;
	isref_ptr->height = 0;

        isref_ptr->fwidth = 0;
        isref_ptr->fheight = 0;


	/* Point light structures. */
	for(i = 0; i < isref_ptr->total_point_lights; i++)
	    free(isref_ptr->point_light[i]);

	free(isref_ptr->point_light);
	isref_ptr->point_light = NULL;

	isref_ptr->total_point_lights = 0;



	return;
}

/*
 *	Allocates an ISRef explicitly by number.
 */
int ISRefCreateExplicit(int isref_num)
{
	int i, prev_total;


	/* Sanitize total. */
	if(total_isrefs < 0)
	    total_isrefs = 0;

	/* Make sure isref_num is a valid number. */
	if((isref_num < 0) || (isref_num >= ISREF_MAX))
	    return(-1);


	/* Pointer already allocated? */
	if(isref_num < total_isrefs)
	{
	    /* Deallocate this isref and all its resources. */
	    ISRefDelete(isref_num);
	}
	/* Unallocated pointer? */
	else if(isref_num >= total_isrefs)
        {
	    /* Remember previous total isrefs. */
	    prev_total = total_isrefs;

	    /* Adjust global variable total_isrefs to reflect new amount. */
	    total_isrefs = isref_num + 1;

	    /* Allocate new amount of pointers. */
	    isref = (isref_struct **)realloc(
		isref,
		total_isrefs * sizeof(isref_struct *)
	    );
	    if(isref == NULL)
	    {
		total_isrefs = 0;
		return(-1);
	    }

	    /* Reset new pointers. */
	    for(i = prev_total; i < total_isrefs; i++)
		isref[i] = NULL;
        }


	/* Allocate new structure. */
	isref[isref_num] = (isref_struct *)calloc(
	    1,
	    sizeof(isref_struct)
	);
	if(isref[isref_num] == NULL)
	    return(-1);

	/* Reset values. */
	ISRefReset(isref_num);


	return(0);
}

/*
 *	Load ISRef as default image, allocating it as needed.
 */
int ISRefLoadAsDefault(int isref_num)
{
	int i, status;
	isref_struct *isref_ptr;
	tga_data_struct *tga_data_ptr;


        /* Make sure isref_num is a valid number. */
	if((isref_num < 0) || (isref_num >= ISREF_MAX))
            return(-1);

	/* Allocate isref as needed. */
        if(!ISRefIsAllocated(isref_num))
	{
	    if(ISRefCreateExplicit(isref_num))
		return(-1);
        }

        /* Free all allocated substructures if any. */
	ISRefReset(isref_num);

	/* Get pointer to isref. */
	isref_ptr = isref[isref_num];


        /* ************************************************************ */
	/* SPECIAL CASE: if isref_num is ISREF_DEFAULT. */

	if(isref_num == ISREF_DEFAULT)
	{
	    /* Load default Isref. */

/* Explicitly set image format for now. */
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

		/* Read tga data. */
		status = TgaReadFromData(
		    unknown_tga,
		    tga_data_ptr,
		    osw_gui[0].depth
		);
		if(status != TgaSuccess)
		{
                    fprintf(stderr,
         "ISRefLoadAsDefault(): TgaReadFromFile() error code %i.\n",
                        status
                    );

                    /* Destroy the tga_data. */
                    TgaDestroyData(tga_data_ptr);

                    isref_ptr->format_code = ISREF_FORMAT_CODE_UNKNOWN;   

		    free(isref_ptr->lib_data);
		    isref_ptr->lib_data = NULL;

		    return(-1);
                }
		/* Link image_data pointer to image data in library struct. */
		isref_ptr->image_data = (u_int8_t *)tga_data_ptr->data;

                /* Set width and height from tga_data. */
                isref_ptr->width = tga_data_ptr->width;
                isref_ptr->height = tga_data_ptr->height;

		/* Mark as loaded and done. */
		isref_ptr->load_progress = ISREF_LOAD_PROGRESS_DONE;
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

	    /* We're done loading the default isref, return success. */
	    return(0);
	}

        /* ************************************************************ */
	/* Is the default isref loaded? */
	if(!ISRefIsLoaded(ISREF_DEFAULT))
        {
            MesgAdd(
		"ISRefLoadAsDefault(): Default isref not loaded.\n",
		xsw_color.bp_bold_text
	    );

            return(-3);
        }


	/* Copy default isref data to isref_num but not all substructures.
	 * substructures.  The member data will be linked to the default
	 * isref's image data.  So make sure all pointers to the image
	 * data on all other isrefs that point to this default isref's
	 * data are reset when the default isref is unloaded.
	 */

	memcpy(
	    isref_ptr,			/* Target. */
	    isref[ISREF_DEFAULT],	/* Source. */
	    sizeof(isref_struct)
	);

	/* Copy default isref's filename over to the isref if it has one. */
	isref_ptr->filename = StringCopyAlloc(isref[ISREF_DEFAULT]->filename);


	/* Set this isref's library data to NULL so that its library
	 * data and image data are not deallocated. This basically marks
	 * this isref as sharing the image_data from another isref.
	 */
        isref_ptr->lib_data = NULL;


	/* Copy point light structures. */
	if(isref[ISREF_DEFAULT]->total_point_lights > 0)
	{
	    isref_ptr->point_light = (isref_point_light_struct **)calloc(
		1,
		isref[ISREF_DEFAULT]->total_point_lights *
		    sizeof(isref_point_light_struct *)
	    );
	    if(isref_ptr->point_light == NULL)
	    {
		isref_ptr->total_point_lights = 0;
		return(-1);
	    }

	    for(i = 0; i < isref[ISREF_DEFAULT]->total_point_lights; i++)
	    {
	        if(isref[ISREF_DEFAULT]->point_light[i] == NULL)
		    continue;

		isref_ptr->point_light[i] = (isref_point_light_struct *)calloc(
		    1,
		    sizeof(isref_point_light_struct)
		);
		if(isref_ptr->point_light[i] == NULL)
		    continue;

		memcpy(
		    isref_ptr->point_light[i],
		    isref[ISREF_DEFAULT]->point_light[i],
		    sizeof(isref_point_light_struct)
		);
	    }
	}

/*
sprintf(stringa, "Could not find isref %i.", isref_num);
MesgAdd(stringa, xsw_color.bp_bold_text);
 */

	return(0);
}

/*
 *	Loads the ISRef's referanced image from file.
 */
int ISRefLoad(int isref_num)
{
	int status;
	struct stat stat_buf;

        tga_data_struct *tga_data_ptr;
	isref_struct *isref_ptr;


	/* Error checks. */
        if((isref_num < 0) || (isref_num >= ISREF_MAX))
        {
            fprintf(stderr,
        "ISRefLoad(): Error: Index number %i out of range.\n",
                isref_num
            );
            return(-1);
        }

        /* Check if ISRef is allocated. */
        if(ISRefIsAllocated(isref_num))
	{
	    /* Get isref pointer. */
	    isref_ptr = isref[isref_num];
	}
	else
        {
            /* Allocate and load ISRef as default image. */
            status = ISRefLoadAsDefault(isref_num);
            return(status);
        }

	/* Check if isref is not to load/have an image. */
	if(isref_ptr->option & ISREF_OPT_NO_IMAGE)
	    return(-2);


	/* ******************************************************** */

	/* Unload image if loaded.  This will free the library
	 * data, set the member image_data to NULL, and reset a few
	 * other values.
	 */
	ISRefUnload(isref_num);


	/* Does it have a filename? */
	if(isref_ptr->filename == NULL)
	{
	    /* Load as default image. */
	    status = ISRefLoadAsDefault(isref_num);
            return(status);
	}
	/* Is that filename pointing to an existing file? */
	if(stat(isref_ptr->filename, &stat_buf))
        {
            fprintf(stderr,
          "ISRefLoad(): Error: %s: No such file, using default image.\n",
                isref_ptr->filename
            );
            status = ISRefLoadAsDefault(isref_num);

            return(status);
        }


	/* ************************************************************ */

	/* Set up for passive async load or load synced? */
	if(option.async_image_loading)
	{
            /* Load passive async. */

/* Explicitly set format for now. */
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
	else
	{
            /* Load synced. */

/* Explicitly set format for now. */
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

                /* Read tga data. */
                status = TgaReadFromFile(
                    isref_ptr->filename,
                    tga_data_ptr,
                    osw_gui[0].depth
                );
                if(status != TgaSuccess)
                {
                    fprintf(stderr,
                  "ISRefLoad(): %s: TgaReadFromFile() error code %i.\n",
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

                /* Mark load process as done. */
                isref_ptr->load_progress = ISREF_LOAD_PROGRESS_DONE;
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

/*
 *	Unload the ISRef's image then marking the ISRef as not loaded.
 */
void ISRefUnload(int isref_num)
{
	int i;
	int deallocated_image = 0;
	isref_struct *isref_ptr, **ptr;


	/* Check if isref_num is allocated. */
	if(ISRefIsAllocated(isref_num))
	    isref_ptr = isref[isref_num];
	else
	    return;

        /* Unload by image format type. Note about sharing: this isref
	 * maybe using another isref's image, if that is the case then
	 * the lib_data of this isref should be null but the image_data
	 * pointer points to the other isref's image_data. So to make
	 * sure, we only deallocate the image if this isref has its
	 * lib_data not NULL.
	 */
	switch(isref_ptr->format_code)
	{
	  case ISREF_FORMAT_CODE_TGA:
	    if(isref_ptr->lib_data != NULL)
	    {
	        TgaDestroyData((tga_data_struct *)isref_ptr->lib_data);

		free(isref_ptr->lib_data);
		isref_ptr->lib_data = NULL;

		/* Mark that image memory was really deallocated. */
		deallocated_image = 1;
	    }
	    break;

	  default:
            if(isref_ptr->lib_data != NULL)
            {
                free(isref_ptr->lib_data);
                isref_ptr->lib_data = NULL;

		/* Mark that image memory was really deallocated. */
		deallocated_image = 1;
            }
	    break;
	}

        /* Reset pointers on other isrefs if their image data pointer
         * referances this isref's image data pointer (sharing) and if
	 * this isref's image data was actually deallocated.
         */
	if(deallocated_image)
	{
            for(i = 0, ptr = isref; i < total_isrefs; i++, ptr++)
            {
                if(*ptr == NULL)
                    continue;

                /* Skip those with no referancing image data pointer. */
                if((*ptr)->image_data == NULL)
                    continue;

                /* Skip isref_num itself. */
                if(i == isref_num)
                    continue;

                /* Skip default isref, it must be left alone. */
                if(i == ISREF_DEFAULT)
                    continue;

		/* If image data pointers match, reset to NULL. */
                if((*ptr)->image_data == isref_ptr->image_data)
                    (*ptr)->image_data = NULL;
	    }
        }

	/* Reset image data pointer to NULL but do not deallocate it,
	 * the call to library destroy function has already done that.
	 */
	isref_ptr->image_data = NULL;

        isref_ptr->format_code = ISREF_FORMAT_CODE_UNKNOWN;
        isref_ptr->load_progress = ISREF_LOAD_PROGRESS_DONE;


	return;
}

/*
 *	Deletes an ISRef, freeing all its allocated resources
 *	and itself.
 */
void ISRefDelete(int isref_num)
{
	if(!ISRefIsAllocated(isref_num))
	    return;


	/* Free all allocated substructures. */
	ISRefReset(isref_num);

	/* Free ISRef structure itself. */
	free(isref[isref_num]);
	isref[isref_num] = NULL;


	return;
}

/*
 *	Procedure to delete all ISRefs.
 */
void ISRefDeleteAll()
{
	int i;


	for(i = 0; i < total_isrefs; i++)
	    ISRefDelete(i);

        free(isref);
        isref = NULL;

	total_isrefs = 0;

	return;
}

/*
 *	Unloads isrefs that are not being used by any XSW objects.
 */
void ISRefReclaimMemory()
{
	int i, n;
	int in_use;
	isref_struct **ptr;
	xsw_object_struct **obj_ptr;


        /* Sanitize total_isrefs. */
        if(total_isrefs < 0) 
            total_isrefs = 0;


	/* Scan through isrefs. */
	for(i = 0, ptr = isref;
            i < total_isrefs;
            i++, ptr++
	)
	{
            /* Leave ISREF_DEFAULT alone. */
            if(i == ISREF_DEFAULT)
		continue;

	    /* Allocated? */
	    if(*ptr == NULL)
		continue;
	    /* Isref must stay loaded? */
	    if((*ptr)->option & ISREF_OPT_STAY_LOADED)
		continue;

	    /* Is loaded? */
            if(!ISRefIsLoaded(i))
		continue;


	    /* Scan through xsw objects. */
	    for(in_use = 0, n = 0, obj_ptr = xsw_object;
                n < total_objects;
                n++, obj_ptr++
            )
            {
		if(DBIsObjectGarbage(n))
		    continue;

                /* See if this object is using isref i. */
                if((*obj_ptr)->imageset == i)
                {
                    in_use = 1;
                    break;
                }
	    }

            /* Unload this isref if it's not in use. */
            if(!in_use)
                ISRefUnload(i);
	}

	return;
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
void ISRefManage()
{
        int i, p, load_completed, status;
	int pixels_loaded = 0;
        isref_struct **ptr;
	tga_data_struct *tga_data_ptr;

	int obj_num;
	xsw_object_struct *obj_ptr;
	isref_point_light_struct **point_light_ptr;



	/* Go through each isref. */
	for(i = 0, ptr = isref;
            i < total_isrefs;
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

			if((*point_light_ptr)->strobe_next <= cur_millitime)
                        {
                            /* Strobe just turned off. */
                            (*point_light_ptr)->strobe_state = 0;

                            (*point_light_ptr)->strobe_next = cur_millitime +
                                (*point_light_ptr)->strobe_off_int;
                        }
		    }
		    else
                    {
                        /* Strobe is in off interval. */
                        if((*point_light_ptr)->strobe_next <= cur_millitime)
                        {
                            /* Strobe just turned on. */
                            (*point_light_ptr)->strobe_state = 1;

                            (*point_light_ptr)->strobe_next = cur_millitime +
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
            if((pixels_loaded < option.async_image_pixels) &&
	       ((*ptr)->load_progress == ISREF_LOAD_PROGRESS_LOADING)
	    )
	    {
		/* Library data not allocated? */
		if((*ptr)->lib_data == NULL)
		    continue;

		/* Reset load_completed marker. */
		load_completed = 0;

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
			ISRefReset(i);
			ISRefLoadAsDefault(i);

			load_completed = 1;
			(*ptr)->load_progress = ISREF_LOAD_PROGRESS_DONE;
		    }
		    /* Done loading? */
		    if(tga_data_ptr->cur_load_pixel < 0)
		    {
			/* Image is done loading, mark it as such. */
			load_completed = 1;
			(*ptr)->load_progress = ISREF_LOAD_PROGRESS_DONE;
		    }
		    /* Increment pixels loaded. */
		    pixels_loaded += ISREF_PIXELS_TO_LOAD_PER_ISREF;
		    break;

		  default:
                    (*ptr)->load_progress = ISREF_LOAD_PROGRESS_DONE;
		    break;
		}

		/*   If an image was completed loading, we need to
                 *   redraw all nessasary displays.
		 */
		if(load_completed)
		{
		    /* Redraw bridge window console panels as needed. */
		    obj_ptr = net_parms.player_obj_ptr;
		    if(obj_ptr != NULL)
		    {
			if(obj_ptr->imageset == i)
			    BridgeWinDrawPanel(
				net_parms.player_obj_num,
				BPANEL_DETAIL_P2
			    );

			obj_num = obj_ptr->locked_on;
			if(!DBIsObjectGarbage(obj_num))
			{
                            if(i == xsw_object[obj_num]->imageset)
                                BridgeWinDrawPanel(
                                    obj_num,
                                    BPANEL_DETAIL_S1
                                );
			}
		    }
		}



	    }
	}


	return;
}
