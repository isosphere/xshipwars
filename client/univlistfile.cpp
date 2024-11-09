/*
                    Universe List File IO

	Functions:

	int UnivListLoadFromFile(char *filename)
	int UnivListSaveToFile(char *filename)

 */

#include "../include/fio.h"
#include "../include/cfgfmt.h"

#include "univlist.h"
#include "xsw.h"


#define MIN(a,b)        (((a) < (b)) ? (a) : (b))
#define MAX(a,b)        (((a) > (b)) ? (a) : (b))


int UnivListLoadFromFile(char *filename)
{
        char *strptr, *strptr2;

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
            fprintf(stderr, "%s: No such file.\n", filename);
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

 
        /* Free all universe entries. */
        UnivDeleteAll();


	/* Begin reading file. */
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


            /* BeginUniverseEntry */
            if(!strcasecmp(parm, "BeginUniverseEntry") ||
               !strcasecmp(parm, "BeginSWAddressEntry")
            )
            {
		int i, n;
		univ_entry_struct *univ_entry_ptr;


                /* Allocate a new universe entry (append).
                 * Do not pass any fields, we will set them ourselves.
                 */
                i = UnivAdd(NULL, NULL, 0, NULL, -1);
                if(i < 0)
                    continue;

                /* Get new entry_num. */
                n = total_univ_entries - 1;
                if(UnivIsAllocated(n))
		    univ_entry_ptr = univ_entry[n];
		else
                    continue;

                /* Begin loading. */
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


                    /* Alias */
                    if(!strcasecmp(parm, "Alias")) 
                    {
                        univ_entry_ptr->alias = StringCopyAlloc(val);
                    }
                    /* URL */
                    else if(!strcasecmp(parm, "URL"))
                    {
                        univ_entry_ptr->url = StringCopyAlloc(val);
                    }
                    /* LastConnected */
                    else if(!strcasecmp(parm, "LastConnected"))
                    {
                        univ_entry_ptr->last_connected = atol(val);
                    }
                    /* Comments */
                    else if(!strcasecmp(parm, "Comments"))
                    {
                        univ_entry_ptr->comments = StringCopyAlloc(val);
                    }
                    /* TimesConnected */
                    else if(!strcasecmp(parm, "TimesConnected"))
                    {
                        univ_entry_ptr->times_connected = atol(val);
                    }
                    /* EndUniverseEntry */
                    else if(!strcasecmp(parm, "EndUniverseEntry") ||
                            !strcasecmp(parm, "EndSWAddressEntry")
                    )
                    {
                        break;
                    }
                    /* Unknown parameter. */
                    else
                    {
                        fprintf(
			    stderr,
                            "%s: Line %i: Unknown parameter: %s\n",
                            filename,
                            lines_read,
                            parm
                        );
                    }
                }
            }
            /* Unknown parameter. */
            else
            {
                fprintf(stderr, "%s: Line %i: Unknown parameter: %s\n",
                    filename,
                    lines_read,
                    parm
                );
            }
	}

        /* Close the file. */
        fclose(fp);


       return(0);
}

int UnivListSaveToFile(char *filename)
{
        int i;
        FILE *fp;
	list_window_struct *list;
	univ_entry_struct *univ_entry_ptr;


        /* Open filename for writing. */
        fp = fopen(filename, "w");
        if(fp == NULL)
        {
            fprintf(
		stderr,
                "%s: Cannot open for writing.\n",
                filename
            );
            return(-1);
        }

	/* Header. */
        fprintf(
	    fp,
"# ShipWars Universe List\n\
# Generated by %s Version %s\n\
#\n",
	    PROG_NAME, PROG_VERSION
        );

	/* Save by order on universe list window. */
	list = &univ_list_win.list;
        for(i = 0; i < list->total_entries; i++)
        {
            if(list->entry[i] == NULL)
                continue;
            if(list->entry[i]->data_ptr == NULL)
                continue;  


            /* Get universe entry pointer from data pointer on list. */
            univ_entry_ptr = (univ_entry_struct *)list->entry[i]->data_ptr;

            fprintf(fp, "BeginUniverseEntry\n");

            /* Alias */
            if(univ_entry_ptr->alias != NULL)
            {
                fprintf(
		    fp,
                    "    Alias = %s\n",
                    list->entry[i]->name	/* Use name on list item. */
                );
            }
            /* URL */
            if(univ_entry_ptr->url != NULL)
            {
                fprintf(fp,
                    "    URL = %s\n",
                    univ_entry_ptr->url
                );
            }
            /* LastConnected */
            fprintf(fp,
                "    LastConnected = %lu\n",
                univ_entry_ptr->last_connected
            );
            /* Comments */
            if(univ_entry_ptr->comments != NULL)
            {
                fprintf(fp,
		    "    Comments = %s\n",
                    univ_entry_ptr->comments
                );
            }
            /* TimesConnected */
            fprintf(fp,
                "    TimesConnected = %lu\n",
                univ_entry_ptr->times_connected
            );

            fprintf(fp, "EndUniverseEntry\n\n");
        }

        fprintf(fp, "\n");


	/* Close the file. */
	fclose(fp);


	return(0);
}
