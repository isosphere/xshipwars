/*
                      Object Parameter Macros File IO

	Functions:

	int OPMLoadFromFile(char *path)

 */

#include "../include/unvfile.h"
#include "swserv.h"



int OPMLoadFromFile(char *path)
{
        int total;


        /* Delete all OPMs currently in memory. */
        OPMDeleteAll();

        /* Load objects from file. */
        total = total_opms;
        opm = UNVLoadFromFile(
            path,
            &total,
            NULL,
	    NULL,	/* Client data. */
	    NULL	/* Notify callback function. */
        );
        total_opms = total;


        return(0);
}
