/*
                         XSW Installation Procedures

	Functions:

	int XSWDoInstallObject(
	        char *target,
	        char *source,
	        mode_t m,
	        unsigned long,
	        void *data
	)

	---

 */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "../include/disk.h"

#include "xswinstall.h"
#include "xsw.h"



/*
 *	Installs the disk object (file, directory, etc).
 */
int XSWDoInstallObject(
        char *target,   /* Target. */
        char *source,   /* Source. */
        mode_t m,	/* Permissions for newly installed object. */
        unsigned long options,	/* One of XSW_INST_OBJ_FLAG_*. */
        void *data      /* Data argument. */
)
{
	int status = 0;


	if(options & XSW_INST_OBJ_FLAG_DIR)
	{
	    /* Create new directory. */
	    if(target == NULL)
		return(-1);

	    /* Recursivly? */
	    if(options & XSW_INST_OBJ_FLAG_RECURSIVE)
	    {
		/* Create new directory recursivly. */
		char *strptr;
		struct stat stat_buf;
		char cwd[PATH_MAX];
		char ltarget[PATH_MAX + NAME_MAX];


		getcwd(cwd, PATH_MAX);
		cwd[PATH_MAX - 1] = '\0';

		/* Get absolute path. */
		if(!ISPATHABSOLUTE(target))
		{
		    strptr = PrefixPaths(cwd, target);
		    strncpy(
			ltarget,
			((strptr == NULL) ? target : strptr),
			PATH_MAX + NAME_MAX
		    );
		    ltarget[PATH_MAX + NAME_MAX - 1] = '\0';
		    target = ltarget;
		}

		/* Get pointer to beginning of target path but skip
		 * first character since it's a '/'.
		 */
		strptr = target + 1;
		while(1)
		{
		    if(strptr == NULL)
                        break;

		    strptr = strchr(strptr, '/');

		    if(strptr != NULL)
			*strptr = '\0';

		    if(stat(target, &stat_buf))
		    {
			status = mkdir(target, m);
			if(status)
			    break;
		    }

		    if(strptr != NULL)
		    {
			*strptr = '/';
			strptr++;
		    }
		}
	    }
	    else
	    {
		status = mkdir(target, m);
	    }
	}
	else
	{
	    /* Copy file. */
	    if((target == NULL) ||
	       (source == NULL)
	    )
                return(-1);

	    status = CopyObject(
		target,
		source,
		NULL
	    );
	    chmod(target, m);
	}

	return(status);
}
