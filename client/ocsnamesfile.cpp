/*
                   Object Create Script Names: File IO

	Functions:

	int OCSLoadFromFile(char *filename)

*/

#include <sys/stat.h>

#include "../include/string.h"
#include "../include/fio.h"
#include "../include/disk.h"
#include "../include/cfgfmt.h"

#include "../include/tga.h"

#include "xsw.h"


int OCSLoadFromFile(char *filename)
{
        int status;
        char *strptr, *strptr2, *strptr3;

        FILE *fp;
        off_t filesize;
        struct stat stat_buf;

        char parm[CFG_PARAMETER_MAX];
        char val[CFG_VALUE_MAX];

	int lines_read = 0;
	int ocsn_num = -1;

	char stringa[PATH_MAX + NAME_MAX + 80];


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
        /* Delete all OCS names currently in memory. */

        OCSDeleteAll();


        /* ********************************************************* */

        strptr = NULL;
            
        while(1)
        {
            /* Did we load more than OCSN_MAX? */
            if(ocsn_num >= (OCSN_MAX - 1))
            {
                fprintf(stderr,
 "%s: Warning: Maximum of %i OCSNs loaded, additional OCSNs not loaded.\n",
                   filename, OCSN_MAX
                );

                free(strptr); strptr = NULL;
                break;
            }
        
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
            

            /* BeginHeader */
            if(!strcasecmp(parm, "BeginHeader"))
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
            
                    /* Title */
                    if(!strcasecmp(parm, "Title"))
                    {


                    }
                    /* EndHeader */
                    else if(!strcasecmp(parm, "EndHeader"))
                    {
                        break;
                    }
                }
                continue;
            }
            /* ******************************************************* */
            /* BeginOCSNEntry */
            else if(!strcasecmp(parm, "BeginOCSNEntry") ||
                    !strcasecmp(parm, "BeginOCSEntry")
	    )
            {
                /* Increment the OCS number. */
                ocsn_num++;

                /* Allocate a new ocsn. */
                status = OCSCreateExplicit(
                    ocsn_num,
                    1
                );
                if(status)
                {
                    fprintf(stderr,
                        "%s: Line %i: Could not create this OCS.\n",
                        filename, lines_read
                    );
                    return(-1);
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


                    /* Code */
                    if(!strcasecmp(parm, "Code") ||
                       !strcasecmp(parm, "Type")
		    )
                    {
                        ocsn[ocsn_num]->type = atoi(val);

                        /* Make sure type is something valid. */
                        if(ocsn[ocsn_num]->type <= OCS_TYPE_GARBAGE)
			{
			    fprintf(stderr,
         "%s: Line %i: Warning: Type code is %i (Garbage).\n",
                                filename, lines_read, OCS_TYPE_GARBAGE
                            );

                            ocsn[ocsn_num]->type = OCS_TYPE_GARBAGE + 1;
			}
                    }
                    /* Name */
                    else if(!strcasecmp(parm, "Name"))
                    {   
                        strncpy(
                            ocsn[ocsn_num]->name,
                            val,
                            XSW_OBJ_NAME_MAX
                        );
                        ocsn[ocsn_num]->name[XSW_OBJ_NAME_MAX - 1] = '\0';
                    }
                    /* IconImage */
                    else if(!strcasecmp(parm, "IconImage"))
                    {
                        /* Set full path? */
                        if(!ISPATHABSOLUTE(val))
                        {
                            strptr3 = PrefixPaths(dname.images, val);
			    if(strptr3 != NULL)
			        strncpy(val, strptr3, CFG_VALUE_MAX);
			    val[CFG_VALUE_MAX - 1] = '\0';
			}

                        /* Make sure that file exists. */
                        if(stat(val, &stat_buf))
                        {
                            fprintf(stderr,
                        "%s: Line %i: %s: Warning: No such file.\n",
                                filename, lines_read, val
			    );
                            sprintf(stringa,
"Warning: Unable to find image file:\n\n    %s\n\n\
Specified in:\n\n    %s\n",
                                val,
                                filename
                            );
                            printdw(&err_dw, stringa);
                        }
                        /* Make sure the image is a tga file. */
                        else if(TgaTestFile(val) != TgaSuccess)
                        {
                            fprintf(stderr,
            "%s: Line %i: %s: Warning: Invalid tga image file.\n",
                                filename, lines_read, val
                            );
                            sprintf(stringa,
"Error: Invalid TGA image file:\n\n    %s\n\n\
Specified in:\n\n    %s\n",
                                val,
                                filename
                            );
                            printdw(&err_dw, stringa);
                        }

			/* Unload image incase already loaded. */
			OSWDestroyImage(&ocsn[ocsn_num]->icon);

			/* Load new icon image. */
			ocsn[ocsn_num]->icon = WidgetLoadImageFromTgaFile(val);
			if(ocsn[ocsn_num]->icon == NULL)
			{
                            sprintf(stringa,
"Error: Unable to load image file:\n\n    %s\n\n\
Specified in:\n\n    %s\n",
                                val,
                                filename
                            );
                            printdw(&err_dw, stringa);
			}
printf("Loaded %s\n", val);
                    }
                    /* EndOCSNEntry */
                    else if(!strcasecmp(parm, "EndOCSNEntry") ||
                            !strcasecmp(parm, "EndOCSEntry")
		    )
                    {
                        break;
                    }
                    /* Unsupported property */
                    else
                    {
                        fprintf(stderr,
            "%s: Line %i: Warning: Unsupported parameter `%s'\n",
                           filename, lines_read, parm
                        );
                    }
                }
            }
        }

        /* Close file. */
        fclose(fp);


        return(0);
}
