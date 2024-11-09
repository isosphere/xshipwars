/*
                       Object Create Scripts File

	Functions:

	int OCSLoadFromFile(char *path)

	---

 */

#include "../include/unvmath.h"

#include "swserv.h"



int OCSLoadFromFile(char *path)
{
	int status;
        char *strptr, *strptr2;
        FILE *fp;
        off_t filesize;
        struct stat stat_buf;

	char parm[CFG_PARAMETER_MAX];
	char val[CFG_VALUE_MAX];

	int lines_read = 0;
	int ocs_num = -1;



	if(path == NULL)
	    return(-1);
        if(stat(path, &stat_buf))
        {
	    fprintf(stderr,
		"%s: No such file.\n",
		path
	    );
            return(-1);
        }

	/* Get size of file. */
        filesize = stat_buf.st_size;

	/* Open file. */
	fp = fopen(path, "r");
	if(fp == NULL)
	{
            fprintf(stderr,
		"%s: Cannot open.\n",
                path
            );
            return(-1);
	}


	/* *************************************************** */
	/* Delete all OCSs currently in memory. */
	OCSDeleteAll();


	/* *************************************************** */

	strptr = NULL;

	while(1)
	{
	    /* Did we load more than MAX_OCSS? */
	    if(ocs_num >= MAX_OCSS)
	    {
		fprintf(stderr,
                   "Warning: Maximum of %i OCSs loaded.\n",
                   MAX_OCSS
                );
		fprintf(stderr,
                   "Warning: Additional OCSs not loaded.\n"
                );
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
            /* Set value to "0" if NULL pointer was returned. */
            if(strptr2 == NULL) strptr2 = "0";
            strncpy(val, strptr2, CFG_VALUE_MAX);
            val[CFG_VALUE_MAX - 1] = '\0';


	    /* BeginHeader. */
	    if(!strcmp(parm, "BeginHeader"))
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
                    /* Set value to "0" if NULL pointer was returned. */ 
                    if(strptr2 == NULL) strptr2 = "0";
                    strncpy(val, strptr2, CFG_VALUE_MAX);
                    val[CFG_VALUE_MAX - 1] = '\0';


                    /* Title */
                    if(!strcmp(parm, "Title"))
                    {
/*
                        strncpy(unv_head.title, val, UNV_TITLE_MAX);
			unv_head.title[UNV_TITLE_MAX - 1] = '\0';
*/
                    }
                    /* EndHeader */
                    else if(!strcmp(parm, "EndHeader"))
                    {
                        break;
                    }
                }
		continue;
	    }
	    /* ****************************************************** */
	    /* BeginOCSEntry */
	    else if(!strcmp(parm, "BeginOCSEntry"))
	    {
		/* Allocate a new OCS. */
                ocs_num++;

		status = OCSCreateExplicit(
		    ocs_num,
		    1
		);
		if(status)
		{
		    fprintf(stderr,
			"%s: Line %i: Could not create OCS #%i.\n",
			path, lines_read, ocs_num
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
                    /* Set value to "0" if NULL pointer was returned. */
                    if(strptr2 == NULL) strptr2 = "0";
                    strncpy(val, strptr2, CFG_VALUE_MAX);
                    val[CFG_VALUE_MAX - 1] = '\0';


		    /* Code. */
		    if(!strcmp(parm, "Code"))
		    {
                        ocs[ocs_num]->code = atoi(val);

			/* Make sure code is something valid. */
			if(ocs[ocs_num]->code <= OCS_TYPE_GARBAGE)
			    ocs[ocs_num]->code = OCS_TYPE_GARBAGE + 1;
		    }
                    /* OPMName. */
                    else if(!strcmp(parm, "OPMName"))
                    {
                        strncpy(ocs[ocs_num]->opm_name,
                            val,
                            XSW_OBJ_NAME_MAX
			);
                        ocs[ocs_num]->opm_name[XSW_OBJ_NAME_MAX - 1] = '\0';
                    }
                    /* Coppies. */
                    else if(!strcmp(parm, "Coppies"))
                    {
                        ocs[ocs_num]->coppies = atoi(val);
                        
                        /* Make sure coppies is something valid. */
			if(ocs[ocs_num]->coppies > MAX_OCS_COPPIES)
			    ocs[ocs_num]->coppies = MAX_OCS_COPPIES;
                        if(ocs[ocs_num]->coppies < 1)
                            ocs[ocs_num]->coppies = 1;
                    }
                    /* Heading. */
                    else if(!strcmp(parm, "Heading") ||
                            !strcmp(parm, "Angle")
		    )
                    {
                        ocs[ocs_num]->heading = SANITIZERADIANS(atof(val));
                    }
                    /* Pitch. */
                    else if(!strcmp(parm, "Pitch"))
                    {
                        ocs[ocs_num]->pitch = SANITIZERADIANS(atof(val));
                    }
                    /* Bank. */
                    else if(!strcmp(parm, "Bank"))
                    {
                        ocs[ocs_num]->bank = SANITIZERADIANS(atof(val));
                    }
                    /* Radius. */
                    else if(!strcmp(parm, "Radius"))
                    {
                        ocs[ocs_num]->radius = atof(val);

                        /* Sanitize radius. */
			if(ocs[ocs_num]->radius < 0)
			    ocs[ocs_num]->radius = 0;
                    }

                    /* EndOCSEntry */
                    else if(!strcmp(parm, "EndOCSEntry"))
                    {
                        break;
                    }
		    /* Unsupported property */
		    else
		    {
			fprintf(stderr,
			   "%s: Line %i: Warning: Unsupported parameter `%s'\n",
                           path, lines_read, parm
			);
		    }
		}
	    }
	}

	/* Close file. */
        fclose(fp);
	fp = NULL;


	return(0);
}
