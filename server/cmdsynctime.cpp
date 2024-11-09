#include "swserv.h"
#include "net.h"


#define THIS_CMD_NAME	"synctime"


/*
 *      Syncronize time.
 */
int CmdSyncTime(int condescriptor, const char *arg)
{
        int con_obj_num;
        xsw_object_struct *con_obj_ptr;
        connection_struct *con_ptr;

        char sndbuf[CS_DATA_MAX_LEN];
        char text[XSW_OBJ_NAME_MAX + 256];


        /* Get connection's object number (assumed valid). */
        con_ptr = connection[condescriptor];
        con_obj_num = con_ptr->object_num;
        con_obj_ptr = xsw_object[con_obj_num];


        /* Check if object's permission allows sync time. */
        if(con_obj_ptr->permission.uid > ACCESS_UID_SYNCTIME)
        {
            sprintf(
		sndbuf,
                "%s: Permission denied: Requires access level %i.",
		THIS_CMD_NAME,
                ACCESS_UID_SYNCTIME
            );
            NetSendLiveMessage(condescriptor, sndbuf);

            return(-1);
        }


        /* Log. */
        sprintf(
	    text,
            "%s: Syncronized global timmers.",
            DBGetFormalNameStr(con_obj_num)
        );
        if(sysparm.log_general)
            LogAppendLineFormatted(fname.primary_log, text);


        /* Warn all connections about timming reset. */
        NetSendLiveMessage(-1, "Server: Syncing global timmers...");

        /* Reset all global timmers. */
        SWServDoResetTimmers();

        /* Warn all connections about timming reset. */
        NetSendLiveMessage(-1, "Server: Global timmers syncronized.");


        return(0);
}
