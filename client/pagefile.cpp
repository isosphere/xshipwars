/*
                        Main Menu Configuration Loading

	Functions:

	image_t *PageLoadImage(char *filename)

	int PageLoadFromFile(
		page_struct *p,
		win_t w, shared_image_t *image,
		char *filename
	)

	---

 */

#include "../include/cfgfmt.h"
#include "../include/fio.h"

#include "xsw.h"
#include "page.h"


/*
 *	Macro to load image.
 */
image_t *PageLoadImage(char *filename)
{
	return(WidgetLoadImageFromTgaFile(filename));
}


/*
 *	Load main menu configuration from file.
 *
 *	Current configuration in mm will be deleted first.
 */
int PageLoadFromFile(
	page_struct *p,
	win_t w, shared_image_t *image,
	char *filename
)
{
        char *strptr, *strptr2, *strptr3;

        FILE *fp;
        off_t filesize;
        struct stat stat_buf;

        char parm[CFG_PARAMETER_MAX];
        char val[CFG_VALUE_MAX];

        char tmp_filename[PATH_MAX + NAME_MAX];
	image_t *image_ptr;

	int label_num = -1;
	page_label_struct *label_ptr;

        int lines_read = 0;


        if((filename == NULL) ||
	   (p == NULL)
	)
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


        /* *************************************************************** */
        /* Delete resources (if previously allocated). */

	PageDestroy(p, w, image);


        /* ************************************************************** */

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


	    /* BackgroundImage */
	    if(!strcasecmp(parm, "BackgroundImage") ||
               !strcasecmp(parm, "ImageBackground")
            )
	    {
                /* Set tmp_filename. */
                if(ISPATHABSOLUTE(val))
                {
                    strncpy(tmp_filename, val,
                        PATH_MAX + NAME_MAX
                    );
                }
                else
                {
                    /* Use images dir as parent. */
                    strptr3 = PrefixPaths(dname.images, val);
                    if(strptr == NULL)
			continue;
                    strncpy(tmp_filename, strptr3,
                        PATH_MAX + NAME_MAX
                    );
                }
                tmp_filename[PATH_MAX + NAME_MAX - 1] = '\0';

		free(p->bg_filename);
		p->bg_filename = StringCopyAlloc(tmp_filename);

		/*   Do not load background image now, it will be loaded
                 *   when main menu is mapped.
		 */
	    }
	    /* BackgroundStyle */
            else if(!strcasecmp(parm, "BackgroundStyle"))
            {
		p->bg_image_draw_code = atoi(val);
	    }
            /* BeginLabel */
            else if(!strcasecmp(parm, "BeginLabel"))
            {
                /* Allocate new main menu label. */
		label_num = PageCreateLabel(p);
                if(PageIsLabelAllocated(p, label_num))
		{
		    label_ptr = p->label[label_num];
		}
		else
                {
		    fprintf(stderr,
			"%s: Line %i: Error allocating label.\n",
			filename, lines_read
		    );
		    continue;
                }

                /* Begin reading information for this label. */
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


                    /* ImageUnarmed */
                    if(!strcasecmp(parm, "ImageUnarmed") ||
                       !strcasecmp(parm, "UnarmedImage")
                    )
                    {
                        /* Set tmp_filename. */
                        if(ISPATHABSOLUTE(val))
                        {
			    strncpy(tmp_filename, val,
				PATH_MAX + NAME_MAX
			    );
			}
			else
			{
			    /* Use images dir as parent. */
			    strptr3 = PrefixPaths(dname.images, val);
			    if(strptr == NULL) continue;
			    strncpy(tmp_filename, strptr3,
				PATH_MAX + NAME_MAX
                            );
			}
			tmp_filename[PATH_MAX + NAME_MAX - 1] = '\0';

			/* Unload previous image as needed. */
			ImgLabelReset(
                    &label_ptr->imglabel[PAGE_LABEL_STATE_UNARMED]
			);

			/* Load label image. */
			image_ptr = PageLoadImage(tmp_filename);
			if(image_ptr == NULL)
			{
			    fprintf(stderr,
				"%s: Line %i: %s: Cannot load.\n",
				filename, lines_read, tmp_filename
			    );
			}
			label_ptr->imglabel[
			    PAGE_LABEL_STATE_UNARMED].image = image_ptr;
		    }
                    /* ImageArmed */
                    else if(!strcasecmp(parm, "ImageArmed") ||
                            !strcasecmp(parm, "ArmedImage")
                    )
                    {
                        /* Set tmp_filename. */
                        if(ISPATHABSOLUTE(val))
                        {
                            strncpy(tmp_filename, val,
                                PATH_MAX + NAME_MAX
                            );
                        }
                        else
                        {
                            /* Use images dir as parent. */
                            strptr3 = PrefixPaths(dname.images, val);
                            if(strptr == NULL) continue;
                            strncpy(tmp_filename, strptr3,
                                PATH_MAX + NAME_MAX
                            );
                        }
                        tmp_filename[PATH_MAX + NAME_MAX - 1] = '\0';

                        /* Unload previous image as needed. */
                        ImgLabelReset(
                    &label_ptr->imglabel[PAGE_LABEL_STATE_ARMED]
                        );
                     
                        /* Load label image. */
                        image_ptr = PageLoadImage(tmp_filename);
                        if(image_ptr == NULL)
                        {
                            fprintf(stderr,
                                "%s: Line %i: %s: Cannot load.\n",
                                filename, lines_read, tmp_filename
                            );
                        }
                        label_ptr->imglabel[
                            PAGE_LABEL_STATE_ARMED].image = image_ptr;
		    }
                    /* ImageHighlighted */
                    else if(!strcasecmp(parm, "ImageHighlighted") ||
                            !strcasecmp(parm, "HighlightedImage")
                    )
                    {
                        /* Set tmp_filename. */
                        if(ISPATHABSOLUTE(val))
                        {
                            strncpy(tmp_filename, val,
                                PATH_MAX + NAME_MAX
                            );
                        }
                        else
                        {
                            /* Use images dir as parent. */
                            strptr3 = PrefixPaths(dname.images, val);
                            if(strptr == NULL) continue;
                            strncpy(tmp_filename, strptr3,
                                PATH_MAX + NAME_MAX
                            );
                        }
                        tmp_filename[PATH_MAX + NAME_MAX - 1] = '\0';

                        /* Unload previous image as needed. */
                        ImgLabelReset(
              &label_ptr->imglabel[PAGE_LABEL_STATE_HIGHLIGHTED]
                        );

                        /* Load label image. */
                        image_ptr = PageLoadImage(tmp_filename); 
                        if(image_ptr == NULL)
                        {
                            fprintf(stderr,
                                "%s: Line %i: %s: Cannot load.\n",
                                filename, lines_read, tmp_filename
                            );
                        }
                        label_ptr->imglabel[
                            PAGE_LABEL_STATE_HIGHLIGHTED].image = image_ptr;
                    }

		    /* ********************************************* */
		    /* ImageUnarmedPercentCoordinates */
                    else if(!strcasecmp(parm, "ImageUnarmedPosByPercent"))
                    {
                        label_ptr->imglabel[
			    PAGE_LABEL_STATE_UNARMED].pos_by_percent = 1;
                    }
		    /* ImageUnarmedSizeByPercent */
                    else if(!strcasecmp(parm, "ImageUnarmedSizeByPercent"))
                    {
                        label_ptr->imglabel[
                            PAGE_LABEL_STATE_UNARMED].size_by_percent = 1;
                    }
                    /* ImageUnarmedX */
                    else if(!strcasecmp(parm, "ImageUnarmedX") ||
                            !strcasecmp(parm, "UnarmedImageX")
                    )
		    {
			strptr3 = strchr(val, '%');
			if(strptr3 != NULL)
			{
			    label_ptr->imglabel[
                                PAGE_LABEL_STATE_UNARMED].pos_by_percent = 1;
			    *strptr3 = '\0';
			}
			label_ptr->imglabel[
			    PAGE_LABEL_STATE_UNARMED].x = atoi(val);
		    }
                    /* ImageUnarmedY */
                    else if(!strcasecmp(parm, "ImageUnarmedY") ||
                            !strcasecmp(parm, "UnarmedImageY")   
                    )
                    {
                        strptr3 = strchr(val, '%');
                        if(strptr3 != NULL)
                        {
                            label_ptr->imglabel[
                                PAGE_LABEL_STATE_UNARMED].pos_by_percent =
1;
                            *strptr3 = '\0';
                        }
                        label_ptr->imglabel[
                            PAGE_LABEL_STATE_UNARMED].y = atoi(val);
                    }

		    /* ImageArmedPosByPercent */
                    else if(!strcasecmp(parm, "ImageArmedPosByPercent"))
                    {
                        label_ptr->imglabel[
                            PAGE_LABEL_STATE_ARMED].pos_by_percent = 1;
                    }
		    /* ImageArmedSizeByPercent */
                    else if(!strcasecmp(parm, "ImageArmedSizeByPercent"))
                    {
                        label_ptr->imglabel[
                            PAGE_LABEL_STATE_ARMED].size_by_percent = 1;
                    }
                    /* ImageArmedX */
                    else if(!strcasecmp(parm, "ImageArmedX") ||
                            !strcasecmp(parm, "ArmedImageX")
                    )
                    {
                        strptr3 = strchr(val, '%');
                        if(strptr3 != NULL)
                        {
                            label_ptr->imglabel[
                                PAGE_LABEL_STATE_ARMED].pos_by_percent = 1;
                            *strptr3 = '\0';
                        }
                        label_ptr->imglabel[
                            PAGE_LABEL_STATE_ARMED].x = atoi(val);
                    }
                    /* ImageArmedY */
                    else if(!strcasecmp(parm, "ImageArmedY") ||
                            !strcasecmp(parm, "ArmedImageY")
                    )
                    {
                        strptr3 = strchr(val, '%');
                        if(strptr3 != NULL)
                        {
                            label_ptr->imglabel[
                                PAGE_LABEL_STATE_ARMED].pos_by_percent = 1;
                            *strptr3 = '\0';
                        }
                        label_ptr->imglabel[
                            PAGE_LABEL_STATE_ARMED].y = atoi(val);
                    }

                    /* ImageHighlightedPosByPercent */
                    else if(!strcasecmp(parm, "ImageHighlightedPosByPercent"))
                    {
                        label_ptr->imglabel[
                            PAGE_LABEL_STATE_HIGHLIGHTED].pos_by_percent = 1;
                    }
                    /* ImageHighlightedSizeByPercent */
                    else if(!strcasecmp(parm, "ImageHighlightedSizeByPercent"))
                    {
                        label_ptr->imglabel[
                            PAGE_LABEL_STATE_HIGHLIGHTED].size_by_percent = 1;
                    }
                    /* ImageHighlightedX */
                    else if(!strcasecmp(parm, "ImageHighlightedX") ||
                            !strcasecmp(parm, "HighlightedImageX")
                    )
                    {
                        strptr3 = strchr(val, '%');
                        if(strptr3 != NULL)
                        {
                            label_ptr->imglabel[
				PAGE_LABEL_STATE_HIGHLIGHTED].pos_by_percent = 1;
                            *strptr3 = '\0';
                        }
                        label_ptr->imglabel[
                            PAGE_LABEL_STATE_HIGHLIGHTED].x = atoi(val);
                    }
                    /* ImageHighlightedY */
                    else if(!strcasecmp(parm, "ImageHighlightedY") ||
                            !strcasecmp(parm, "HighlightedImageY")
                    )
                    {
                        strptr3 = strchr(val, '%');   
                        if(strptr3 != NULL)
                        {
                            label_ptr->imglabel[
				PAGE_LABEL_STATE_HIGHLIGHTED].pos_by_percent = 1;
                            *strptr3 = '\0';
                        }
                        label_ptr->imglabel[
                            PAGE_LABEL_STATE_HIGHLIGHTED].y = atoi(val);
                    }

                    /* ********************************************* */
                    /* AllowTransparency */
                    else if(!strcasecmp(parm, "AllowTransparency"))
                    {
                        label_ptr->allow_transparency = 1;
                    }
                    /* ActionCode */
                    else if(!strcasecmp(parm, "ActionCode") ||
                            !strcasecmp(parm, "OPCode")
		    )
                    {
                        label_ptr->op_code = atoi(val);
                    }
                    /* HintMessage */
                    else if(!strcasecmp(parm, "HintMessage") ||
                            !strcasecmp(parm, "HintMesg")
                    )
                    {
                        free(label_ptr->hint_mesg);
			label_ptr->hint_mesg = StringCopyAlloc(val);
                    }


		    /* EndLabel */
		    else if(!strcasecmp(parm, "EndLabel"))
		    {
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


	return(0);
}
