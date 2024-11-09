#include "xsw.h"


/*
 *	Turns logging on/off and sets log file name.
 */
int CmdLog(const char *arg)
{
	char stringa[256 + PATH_MAX + NAME_MAX];
	char tmp_path[PATH_MAX + NAME_MAX];
	char *strptr, *strptr2;
	struct stat stat_buf;
	off_t filesize;



        /* Print usage? */
        if((arg == NULL) ? 1 : ((*arg) == '\0'))
        {
	    if(option.log_client ||
               option.log_net ||
               option.log_errors
	    )
	    {
                if(stat(fname.log, &stat_buf))
                    filesize = 0;
                else
                    filesize = stat_buf.st_size;

		sprintf(stringa,
                    "log: on  log file: %s  log size:  %ld bytes",
		    fname.log,
		    filesize
		);
	    }
	    else
	    {
                sprintf(stringa,
                    "log: off"
                );
	    }

            MesgAdd(stringa, xsw_color.standard_text);

	    return(0);
	}


	/* Parse "<state> [path]" */
	strncpy(stringa, arg, 256 + PATH_MAX + NAME_MAX);
	stringa[256 + PATH_MAX + NAME_MAX - 1] = '\0';

	strptr = strchr(stringa, ' ');
	if(strptr != NULL)
	{
	    strncpy(tmp_path, strptr + 1, PATH_MAX + NAME_MAX);
	    tmp_path[PATH_MAX + NAME_MAX - 1] = '\0';
	    StringStripSpaces(tmp_path);

	    strptr2 = PathSubHome(tmp_path);
	    if(strptr2 != NULL)
	    {
		strncpy(tmp_path, strptr2, PATH_MAX + NAME_MAX);
		tmp_path[PATH_MAX + NAME_MAX - 1] = '\0';
	    }

	    if(stat(tmp_path, &stat_buf))
		filesize = 0;
	    else
		filesize = stat_buf.st_size;

	    /* Set new log file name. */
	    strncpy(fname.log, tmp_path, PATH_MAX + NAME_MAX);
	    fname.log[PATH_MAX + NAME_MAX - 1] = '\0';

	    *strptr = '\0';
	    /* Turn logging on/off. */
            if(!strcmp("on", stringa))
            {
                option.log_client = 1;
                option.log_net = 1; 
                option.log_errors = 1;

		sprintf(stringa,
		    "log: on  log file: %s  log size:  %ld bytes",
		    fname.log,
		    filesize
	        );
            }   
            else
            {   
                option.log_client = 0;
                option.log_net = 0;
                option.log_errors = 0;

                sprintf(stringa, "log: off");
            }
	    MesgAdd(stringa, xsw_color.standard_text);
	}
	else
	{
	    /* Turn logging on/off. */
	    if(!strcmp("on", stringa))
	    {
		option.log_client = 1;
		option.log_net = 1;
		option.log_errors = 1;

                if(stat(fname.log, &stat_buf))
                    filesize = 0;
                else
                    filesize = stat_buf.st_size;

                sprintf(stringa,
		    "log: on  log file: %s  log size:  %ld bytes",
		    fname.log,
		    filesize
		);
	    }
	    else
	    {
                option.log_client = 0;
                option.log_net = 0;
                option.log_errors = 0;

                sprintf(stringa, "log: off");
	    }
	    MesgAdd(stringa, xsw_color.standard_text);
	}


	return(0);
}
