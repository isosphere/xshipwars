// unvedit/rcfile.cpp

/*
                  Restart Configuration File: Load & Save

	Functions:

	int RCLoadFromFile(char *filename)
	int RCSaveToFile(char *filename)

	---



 */
/*
#include <stdio.h>
#include <db.h>
#include <malloc.h>
#include <stdlib.h>  
#include <string.h>
*/
#include <sys/stat.h>

#include "../include/os.h"
#include "../include/disk.h"
#include "../include/fio.h"
#include "../include/cfgfmt.h"

#include "../include/string.h"
#include "../include/strexp.h"

#include "../include/osw-x.h"
#include "../include/widget.h"

#include "ue.h"
#include "keymap.h"
#include "rcfile.h"
#include "config.h"


/*#define MIN(a,b)	(((a) < (b)) ? (a) : (b)) */
/* #define MAX(a,b)	(((a) > (b)) ? (a) : (b)) */



int RC_SET_COLOR(char *string, WColorStruct *c);
void RC_PRINT_COLOR_STRING(
        char *string,
        char *label,
        WColorStruct c
);



/*
 *	Macro to parse standard color string and set
 *	values into WColorStruct c.
 */
int RC_SET_COLOR(
	char *string,
	WColorStruct *c
)
{
	if((string == NULL) ||
	   (c == NULL)
	)
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
 *      Prints `parameter = value' string by the given values
 *      in label and WColorStruct c.
 */                 
void RC_PRINT_COLOR_STRING(
        char *string,
        char *label,
        WColorStruct c
)
{
        if(string == NULL)
            return;
        if(label == NULL)
            return;

        sprintf(string, "%s = #%.2x%.2x%.2x\n",
            label,
            c.r,
            c.g,
            c.b
        );

        return;  
}



int RCLoadFromFile(char *filename)
{
	int i;
        char *strptr, *strptr2, *strptr3;

        FILE *fp;
        off_t filesize;
        struct stat stat_buf;

        char parm[CFG_PARAMETER_MAX];
        char val[CFG_VALUE_MAX];
        int lines_read = 0;


	/* Check if filename exists. */
	if(filename == NULL)
	    return(-1);
	if(stat(filename, &stat_buf))
	{
/* Don't warn.
	    fprintf(stderr, "%s: No such file.\n", filename);
 */
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


            if(!strcmp(parm, "VersionMajor"))
            {
		option.rc_version_major = atoi(val);
	    }
            else if(!strcmp(parm, "VersionMinor"))
            {
                option.rc_version_minor = atoi(val);
            }
            else if(!strcmp(parm, "VersionRelease"))
            {
                option.rc_version_release = atoi(val);
            }

            else if(!strcmp(parm, "LabelGeometry"))
            {
		option.label_geometry = StringIsYes(val);
	    }
            else if(!strcmp(parm, "ShowGrid"))
            {
                option.show_grid = StringIsYes(val);
            }
            else if(!strcmp(parm, "GridSpacing"))
            {
		/* In XSW real units. */
                option.grid_spacing = atof(val);
            }
            else if(!strcmp(parm, "ViewFontName"))
            {
		strncpy(
		    option.view_font_name,
		    val,
		    FontNameMax
		);
		option.view_font_name[FontNameMax - 1] = '\0';
            }
            else if(!strcmp(parm, "ViewObjectLabelFontName"))
            {
                strncpy(
                    option.view_object_label_font_name,
                    val,
                    FontNameMax
                );
                option.view_object_label_font_name[FontNameMax - 1] = '\0';
            }
            else if(!strcmp(parm, "ShowPreviewImage"))
            {
                option.show_preview_image = StringIsYes(val);
            }
            else if(!strcmp(parm, "AnimateImages"))
            {
                option.animate_images = StringIsYes(val);
            }

            else if(!strcmp(parm, "ToplevelDir"))
            {
                strptr3 = PathSubHome(val);
                if(strptr3 != NULL)
                {
                    strncpy(val, strptr3, CFG_VALUE_MAX);
                    val[CFG_VALUE_MAX - 1] = '\0';
                }

                if(ISPATHABSOLUTE(val))
                    strncpy(
                        dname.toplevel,
                        val,
                        PATH_MAX
                    );
                else
                    fprintf(stderr,
 "%s: Line %i: %s: ToplevelDir must be an absolute path.\n",
                        filename, lines_read, dname.toplevel
		    );
                dname.toplevel[PATH_MAX - 1] = '\0';

                if(!ISPATHDIR(dname.toplevel))
                    fprintf(stderr,
                        "%s: Line %i: %s: Cannot access directory.\n",
                        filename, lines_read, dname.toplevel
                    );
            }
            else if(!strcmp(parm, "ImagesDir"))
            {
                strptr3 = PathSubHome(val);
                if(strptr3 != NULL)
                {
                    strncpy(val, strptr3, CFG_VALUE_MAX);
                    val[CFG_VALUE_MAX - 1] = '\0';
                }

                if(ISPATHABSOLUTE(val))
                    strncpy(
			dname.images,
			val,
			PATH_MAX
		    );
		else
                    strncpy(
                        dname.images,
                        PrefixPaths(DEF_XSW_TOPLEVEL_DIR, val),
                        PATH_MAX
                    );
		dname.images[PATH_MAX - 1] = '\0';

                if(!ISPATHDIR(dname.images))
                    fprintf(stderr,
                        "%s: Line %i: %s: Cannot access directory.\n",
                        filename, lines_read, dname.images
                    );
            }
            else if(!strcmp(parm, "ServerDir"))
            {
                strptr3 = PathSubHome(val);
                if(strptr3 != NULL)
                {
                    strncpy(val, strptr3, CFG_VALUE_MAX);
                    val[CFG_VALUE_MAX - 1] = '\0';
                }

                if(ISPATHABSOLUTE(val))
                    strncpy(
                        dname.server,
                        val,
                        PATH_MAX
                    );
                else
                    strncpy(
                        dname.server,
                        PrefixPaths("/", val),
                        PATH_MAX
                    );
                dname.server[PATH_MAX - 1] = '\0';  
             
                if(!ISPATHDIR(dname.server))
                    fprintf(stderr,
                        "%s: Line %i: %s: Cannot access directory.\n",
                        filename, lines_read, dname.server
                    );  
            }

            else if(!strcmp(parm, "BeginKeyMap"))
            {
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


		    for(i = 0; i < (int)TOTAL_KEYMAPS; i++)
		    {
			if(!strcasecmp(keymap_name[i], parm))
			    keymap[i].keycode = atoi(val);
		    }

                    if(!strcmp(parm, "EndKeyMap"))
                        break;
		}
	    }
            else
            {
                fprintf(stderr, "%s: Line %i: Unknown parameter: %s\n",
                    filename,
                    lines_read,
                    parm
                );
            }
	}

	fclose(fp);
	fp = NULL;


	return(0);
}



/*
 *	Saves configuration to filename.  Returns 0 on success
 *	and -1 on error.
 */
#define WRITE	bytes_written += fwrite(buf,\
                                        sizeof(char),\
                                        strlen(buf),\
                                        fp\
                );

int RCSaveToFile(char *filename)
{
	int i;
	char buf[CFG_STRING_MAX];
	FILE *fp;
	struct stat stat_buf;
        char cwd[PATH_MAX];

	int bytes_written = 0;



	/* Get current working dir. */
	getcwd(cwd, PATH_MAX);
	cwd[PATH_MAX - 1] = '\0';


	/* Check if filename does not already exist, if it does
	 * not then attempt to create parent directories.
	 */
	if(stat(filename, &stat_buf))
	{
	    char *parent = GetParentDir(filename);

	    if(parent != NULL)
	    {
		if(rmkdir(
		    parent,
		    S_IRUSR | S_IWUSR | S_IXUSR |
		    S_IRGRP |           S_IXGRP |
		    S_IROTH |           S_IXOTH
		))
		{
		    fprintf(stderr,
			"%s: Cannot recursivly create directory.\n",
			parent
		    );
		    return(-1);
		}
	    }
	}

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


	/* ********************************************************** */
	/* Heading comment. */
	sprintf(buf,
            "# %s Configuration File\n",
	    PROG_NAME
	);
	WRITE

        sprintf(buf,
            "# Version: %s\n",
	    PROG_VERSION
        );
	WRITE

        sprintf(buf,   
	    "# You may manually edit this file.\n#\n\n\n"
        );
        WRITE


	/* Versions. */
        sprintf(buf,
            "# This configuration file version:\n"
        );
	WRITE

        sprintf(buf,
            "VersionMajor = %i\n",
	    PROG_VERSION_MAJOR
        );
	WRITE

        sprintf(buf,
            "VersionMinor = %i\n",
	    PROG_VERSION_MINOR
        );
	WRITE

        sprintf(buf,
            "VersionRelease = %i\n",
            PROG_VERSION_RELEASE
        );
        WRITE

        sprintf(buf, "\n\n");
	WRITE


	/* Directories and files. */
        sprintf(buf,
            "# Directories and files:\n"
        );
	WRITE

        sprintf(buf, "ToplevelDir = %s\n", dname.toplevel);
        WRITE

        sprintf(buf, "ImagesDir = %s\n", dname.images);
	WRITE

        sprintf(buf, "ServerDir = %s\n", dname.server); 
        WRITE

        sprintf(buf, "\n");
        WRITE


	/* Apperance options. */
	sprintf(buf,
            "# Apperance options:\n"
        );
        WRITE

        sprintf(buf,
            "LabelGeometry = %s\n",
            ((option.label_geometry) ? "yes" : "no")
        );
	WRITE

        sprintf(buf,
            "ShowGrid = %s\n",
            ((option.show_grid) ? "yes" : "no")
        );   
        WRITE

        sprintf(buf,
            "GridSpacing = %.6f\n",
	    option.grid_spacing
        );   
        WRITE

        sprintf(buf, "\n");  
        WRITE


        /* Fonts. */
        sprintf(buf,
            "# Fonts:\n"
        );
        WRITE

        sprintf(buf,
            "ViewFontName = %s\n",
            option.view_font_name
        );
        WRITE

        sprintf(buf,
            "ViewObjectLabelFontName = %s\n",
            option.view_object_label_font_name
        );
        WRITE

        sprintf(buf, "\n");  
        WRITE


        /* Image options. */
        sprintf(buf,
            "# Image options:\n"
        );
        WRITE

        sprintf(buf,
            "ShowPreviewImage = %s\n",
            ((option.show_preview_image) ? "yes" : "no")
        );
        WRITE

        sprintf(buf,
            "AnimateImages = %s\n",
            ((option.animate_images) ? "yes" : "no")
        );
        WRITE   

        sprintf(buf, "\n");  
        WRITE


        /* Key Mappings. */
        sprintf(buf,
	    "# Key Mappings\n"
	);
	WRITE

        sprintf(buf, "BeginKeyMap\n");
	WRITE

	for(i = 0; i < (int)TOTAL_KEYMAPS; i++)
	{
	    sprintf(
		buf,
		"    %s = %i\n",
		keymap_name[i],
		keymap[i].keycode
	    );
            WRITE
	}

        sprintf(buf, "EndKeyMap\n\n\n");
	WRITE

        sprintf(buf, "\n");  
        WRITE



	fclose(fp);
	fp = NULL;


	return(0);
}



