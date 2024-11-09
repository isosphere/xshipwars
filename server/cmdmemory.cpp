#include "swserv.h"
#include "net.h"

#ifdef PLUGIN_SUPPORT
# include "plugins.h"
#endif  /* PLUGIN_SUPPORT */   


#define THIS_CMD_NAME	"memory"
  
 
/*
 *      Print, refresh, or reload items in memory
 *      (not counting processes).
 */
int CmdMemory(int condescriptor, const char *arg)
{
	int con_obj_num;
	xsw_object_struct *con_obj_ptr;
	connection_struct *con_ptr;
	long total;
	swserv_memory_stats_struct m;
	char sndbuf[CS_DATA_MAX_LEN];


        /* Get connection's object number (assumed valid). */
        con_ptr = connection[condescriptor];
        con_obj_num = con_ptr->object_num;
        con_obj_ptr = xsw_object[con_obj_num];

        /* Check if allowed to view memory. */
        if(con_obj_ptr->permission.uid > ACCESS_UID_MEM)
        {
            sprintf(sndbuf,
		"%s: Permission denied: Requires access level %i.",
                THIS_CMD_NAME,
		ACCESS_UID_MEM
            );
            NetSendLiveMessage(condescriptor, sndbuf);
            return(-1);
        }


        /* Calculate memory being used. */
        SWServGetMemoryStats(&m);


        /* Print memory stats. */
        if((arg == NULL) ? 1 : (*arg == '\0'))
        {
            sprintf(sndbuf,
"-        total       con       obj   rec_obj       opm       ocs   plugins scheduals"
            );
            NetSendLiveMessage(condescriptor, sndbuf);

            sprintf(sndbuf,
"Mem:\
 %9ld\
 %9ld\
 %9ld\
 %9ld\
 %9ld\
 %9ld\
 %9ld\
 %9ld",
                m.total,
                m.con,
                m.obj,
                m.rec_obj,
                m.opm,
                m.ocs,
		m.plugins,
                m.scheduals
            );
            NetSendLiveMessage(condescriptor, sndbuf);

	    total = total_connections + total_objects +
		total_recycled_objects + total_opms + total_ocss +
#ifdef PLUGIN_SUPPORT
                total_plugins +
#endif	/* PLUGIN_SUPPORT */
		total_scheduals
	    ;

            sprintf(sndbuf,
"Ent:\
 %9ld\
 %9d\
 %9d\
 %9d\
 %9d\
 %9d\
 %9d\
 %9d",
		total,

                total_connections,
                total_objects,
                total_recycled_objects,
                total_opms,
                total_ocss,
#ifdef PLUGIN_SUPPORT
		total_plugins,
#else	/* PLUGIN_SUPPORT */
		0,
#endif	/* PLUGIN_SUPPORT */
                total_scheduals
            );
            NetSendLiveMessage(condescriptor, sndbuf);


            /* Print usage at end. */ 
            sprintf(
                sndbuf,
                "Usage: `%s [refresh|reload|reload_universe]'",
                THIS_CMD_NAME
            );
            NetSendLiveMessage(condescriptor, sndbuf);
 

            return(0);
        }

 
        /* ********************************************************** */

        /* Reload memory. */
        if(!strcasecmp(arg, "reload"))
        {
            /* Permission check. */
            if(con_obj_ptr->permission.uid > ACCESS_UID_MEMOP)
            {
                sprintf(
		    sndbuf,
		    "%s: Permission denied: Requires access level %i.",
		    THIS_CMD_NAME,
		    ACCESS_UID_MEMOP
                );
                NetSendLiveMessage(condescriptor, sndbuf);
                return(-1);
            }

	    /* Set master reset to 2 so that the configuration will
	     * be reloaded.  The universe database file will NOT be
	     * reloaded.
	     */
	    master_reset = 2;
        }
        /* Refresh objects in memory. */
        else if(!strcasecmp(arg, "refresh"))
        {
	    /* Permission check. */
            if(con_obj_ptr->permission.uid > ACCESS_UID_MEMOP)
            {
                sprintf(
                    sndbuf,
		    "%s: Permission denied: Requires access level %i.",
                    THIS_CMD_NAME,
		    ACCESS_UID_MEMOP
                );
                NetSendLiveMessage(condescriptor, sndbuf);
                return(-1);
            }

            /* Set master reset to 1 so that the memory will be
	     * refreshed.
	     */
	    master_reset = 1;
        }
        /* Reload input universe file. */
        else if(!strcasecmp(arg, "reload_universe"))
        {
            /* Permission check. */
            if(con_obj_ptr->permission.uid > ACCESS_UID_MEMRELOADUNV)
            {
                sprintf(
                    sndbuf,
		    "%s: Permission denied: Requires access level %i.",
                    THIS_CMD_NAME,
		    ACCESS_UID_MEMRELOADUNV
                );
                NetSendLiveMessage(condescriptor, sndbuf);
                return(-1);
            }

	    /* Set master reset to 3 so that the universe database
	     * will be reloaded from the universe in file.
	     */
	    master_reset = 3;
        }
	/* Unsupported operation. */
	else
	{
	    sprintf(
		sndbuf,
		"%s: %s: Unsupported operation.",
		THIS_CMD_NAME, arg
	    );
	    NetSendLiveMessage(condescriptor, sndbuf);
	}


        return(0);
}
