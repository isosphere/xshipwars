#include "../include/unvmatch.h"
#include "swserv.h"
#include "net.h"


#define THIS_CMD_NAME	"disk"
  
 
/*
 *	Reports program used file sizes and site disk
 *	statistics.
 */
int CmdDisk(int condescriptor, const char *arg)
{
        int con_obj_num;
        xsw_object_struct *con_obj_ptr;
        connection_struct *con_ptr;

	struct stat stat_buf;
        char sndbuf[CS_DATA_MAX_LEN + PATH_MAX + NAME_MAX];


        /* Get connection's object number (assumed valid). */
        con_ptr = connection[condescriptor];
        con_obj_num = con_ptr->object_num;
        con_obj_ptr = xsw_object[con_obj_num];


        /* Permission check. */
        if(con_obj_ptr->permission.uid > ACCESS_UID_DISK)
        {
            sprintf(sndbuf,
                "%s: Permission denied: Requires access level %i.",
		THIS_CMD_NAME,
                ACCESS_UID_DISK
            );
            NetSendLiveMessage(condescriptor, sndbuf);

            return(-1);
        }

	/* Do not call disk stats update since it would cause
	 * process to halt for a short time.
	 * That would leave this function too opened for abuse.
	 */
/*
SWServGetOSStats();
 */

	/* No argument? */
        if((arg == NULL) ? 1 : (arg[0] == '\0'))
        {
	    /* Print program files and disk stats. */

	    /* Device hosting program. */
            sprintf(sndbuf,
 "Host device space: %ld kb used  %ld kb total  %.0f%% ratio",
		os_stat.disk_used,
		os_stat.disk_total,
		(double)os_stat.disk_used /
		    (double)os_stat.disk_total * 100
            );
            NetSendLiveMessage(condescriptor, sndbuf);

	    /* Universe input file. */
	    if(!stat(fname.unv_in, &stat_buf))
	    {
                sprintf(sndbuf,
 "Universe in: %s  %ld bytes",
		    fname.unv_in,
		    stat_buf.st_size
                );
                NetSendLiveMessage(condescriptor, sndbuf);
            }

	    /* Universe output file. */
            if(!stat(fname.unv_out, &stat_buf))
            {
                sprintf(sndbuf,
 "Universe out: %s  %ld bytes",
                    fname.unv_out,
                    stat_buf.st_size
                );
                NetSendLiveMessage(condescriptor, sndbuf);
            }

            /* Log. */    
            if(!stat(fname.primary_log, &stat_buf))
            {
                sprintf(sndbuf,
 "Primary log: %s  %ld bytes",
                    fname.primary_log,
                    stat_buf.st_size
                );
                NetSendLiveMessage(condescriptor, sndbuf);
            }

            /* Events export log. */
            if(!stat(fname.events_export, &stat_buf))
            {
                sprintf(sndbuf,
 "Events export log: %s  %ld bytes",
                    fname.events_export,
                    stat_buf.st_size
                );
                NetSendLiveMessage(condescriptor, sndbuf);
            }
        }
	else
	{
	    /* Handle argument. */



	}
 

        return(0);
}

