#include "swserv.h"
#include "net.h"  


#define THIS_CMD_NAME	"shutdown"

/*
 *      Shut down the server
 */
int CmdShutdown(int condescriptor, const char *arg)
{
        int con_obj_num;
        xsw_object_struct *con_obj_ptr;
        connection_struct *con_ptr;

        char sndbuf[CS_DATA_MAX_LEN];
        char text[XSW_OBJ_NAME_MAX + 512];


        /* Get connection's object number (assumed valid). */
        con_ptr = connection[condescriptor];
        con_obj_num = con_ptr->object_num;
        con_obj_ptr = xsw_object[con_obj_num];


        /* Check if object's permission allows save. */
        if(con_obj_ptr->permission.uid > ACCESS_UID_SHUTDOWN)
        {
            sprintf(
		sndbuf,
                "%s: Permission denied: Requires access level %i.",
                THIS_CMD_NAME, ACCESS_UID_SHUTDOWN
            );
            NetSendLiveMessage(condescriptor, sndbuf);

            return(-1);
        }


        /* Log who requested shutdown. */
        sprintf(
	    text,
            "%s: Requesting server shutdown.",
            DBGetFormalNameStr(con_obj_num)
        );
        if(sysparm.log_general)
            LogAppendLineFormatted(fname.primary_log, text);


        /* Begin shutdown sequence. */
        SWServDoShutdown();


        return(0);
}
