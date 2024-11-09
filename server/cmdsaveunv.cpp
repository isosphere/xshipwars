#include "../include/unvmatch.h"
#include "swserv.h"
#include "net.h"

#define THIS_CMD_NAME	"save"  
 
 
/*
 *      Save the universe.
 */
int CmdSaveUniverse(int condescriptor, const char *arg)
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

        /* Permission check. */
        if(con_obj_ptr->permission.uid > ACCESS_UID_SAVE)
        {
            sprintf(
		sndbuf,
                "%s: Permission denied: Requires access level %i.",
                THIS_CMD_NAME, ACCESS_UID_SAVE
            );
            NetSendLiveMessage(condescriptor, sndbuf);

            return(-1);
        }

        /* Log. */
        sprintf(
	    text,
	    "%s: Requesting database save.",
	    DBGetFormalNameStr(con_obj_num)
        );
        if(sysparm.log_general)
	    LogAppendLineFormatted(fname.primary_log, text);


	/* Do save procedure. */
	SWServDoSave();


        /* Mark next time DB save is to be performed. */
        next.unv_save = cur_systime + sysparm.int_unv_save;


        return(0);
}
