/*
                Loading and saving Database from File

	Functions:

	int DBReadFromFile(char *path)
	int DBSaveToFile(
	        char *path,
	        xsw_object_struct **obj_ptr, 
	        int total  
	)
	int DBEmergencySave(
	        xsw_object_struct **obj_ptr,
	        int total
	)

	---


 */

#include "../include/unvfile.h"
#include "swserv.h"


int DBReadFromFile(char *path)
{
	int total;


	/* Delete all objects currently in memory. */
	DBDeleteAllObjects();

	/* Load objects from file. */
	total = total_objects;
	xsw_object = UNVLoadFromFile(
	    path,
	    &total,
	    &unv_head,
	    NULL,	/* Client data. */
	    NULL	/* Notify callback function. */
	);
	total_objects = total;

	/* Get header information. */
	sw_units.ru_to_au = unv_head.ru_to_au;


	/* Reclaim (sanitize) objects and memory used. */
	DBReclaim();


	return(0);
}



int DBSaveToFile(char *path)
{
	int status;


        /* Set header information. */
	strcpy(unv_head.version, PROG_VERSION);
	unv_head.ru_to_au = sw_units.ru_to_au;


	/* Save to file. */
	status = UNVSaveToFile(
	    path,
	    xsw_object,
	    total_objects,
	    &unv_head,
	    NULL,	/* Client data. */
	    NULL	/* Notify callback function. */
	);


	return(status);
}



int DBEmergencySave(
        xsw_object_struct **obj_ptr,
        int total
)
{
        int status;
	char *strptr;
        pid_t pid;

	char text[PATH_MAX + NAME_MAX + 80];
	char tmp_path[PATH_MAX + NAME_MAX];


	/* Get PID of current process. */
	pid = getpid();

	/* Print and log incident. */
	fprintf(
	    stderr,
	    "*** Server: Emergency database save in progress...\n"
	);
        if(sysparm.log_errors)
	    LogAppendLineFormatted(
		fname.primary_log,
	        "*** Server: Emergency database save in progress..."
	    );


	/* ********************************************************** */
	/* Plan A. */

	/* Format new filename for emergency save. */
	sprintf(tmp_path, "%s.EMERGENCY.%i", fname.unv_out, pid);

	/* Attempt to save. */
	status = DBSaveToFile(tmp_path);
	if(status > -1)
	{
	    /* Emergency save was successful. */

            fprintf(stderr,
		"%s: Plan A: Database sucessfully saved!\n",
                tmp_path
	    );
	    sprintf(text,
		"%s: Plan A: Database sucessfully saved!",
		tmp_path
	    );
 	    if(sysparm.log_errors)
		LogAppendLineFormatted(fname.primary_log, text);

	    /* Return, no need for additional saving. */
	    return(0);
	}


        /* ********************************************************** */
        /* Plan B. */

        /* Format new filename for emergency save. */
	strptr = tempnam(P_tmpdir, "sw_unv");

        /* Attempt to save. */
        status = DBSaveToFile(strptr);
        if((status > -1) && (strptr != NULL))
        {
            /* Emergency save was successful. */

            fprintf(stderr,
                "%s: Plan B: Database sucessfully saved!\n",
                strptr
            );
            sprintf(text,
                "%s: Plan B: Database sucessfully saved!",
                strptr
            );
            if(sysparm.log_errors)
                LogAppendLineFormatted(fname.primary_log, text);

	    /* Free tempory name. */
	    free(strptr);
	    strptr = NULL;

            return(0);
        }
	/* Free tempory name. */
	free(strptr);
	strptr = NULL;



	/* ********************************************************* */
	/* Out of ideas for emergency save, we're screwed. */

        fprintf(
	    stderr,
	    "*** Emergency save failed! ***\n"
	);
        sprintf(
	    text,
	    "*** Emergency save failed! ***"
	);

	if(sysparm.log_errors)
	    LogAppendLineFormatted(fname.primary_log, text);


	return(0);
}
