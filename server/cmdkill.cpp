#include "../include/unvmatch.h"
#include "../include/xsw_ctype.h"
#include "swserv.h"
#include "net.h"
 
#define THIS_CMD_NAME	"kill"


int CmdKill(int condescriptor, const char *arg)
{
        int con_obj_num, schedual_num;
	xsw_object_struct *con_obj_ptr;
	connection_struct *con_ptr;
        char sndbuf[CS_DATA_MAX_LEN];


        /* Get connection's object number (assumed valid). */
        con_ptr = connection[condescriptor];
        con_obj_num = con_ptr->object_num;
        con_obj_ptr = xsw_object[con_obj_num];

	/* Check if argument is valid and contains data. */
        if((arg == NULL) ? 1 : (*arg == '\0'))
        {
            sprintf(
		sndbuf,
		"Usage: `%s <pid>'",
		THIS_CMD_NAME
	    );
            NetSendLiveMessage(condescriptor, sndbuf);
            return(-1);
        }


	/* Skip leading spaces. */
	while(ISBLANK(*arg))
	    arg++;

	/* Parse schedual number (PID) argument. */
	schedual_num = atoi(arg);

        /* Does the schedual (PID) exist? */
        if(!SchedualIsActive(schedual_num))
        {
            sprintf(
		sndbuf,
		"%i: No such pid.",
                schedual_num
            );
            NetSendLiveMessage(condescriptor, sndbuf);
            return(-1);
        }

	/* Allowed to kill other's schedual? */
        if((schedual[schedual_num]->run_owner != con_obj_num) &&
           (schedual[schedual_num]->run_owner > -1) &&
           (con_obj_ptr->permission.uid > ACCESS_UID_PSKILLO)
        )
        {
            sprintf(
		sndbuf,
                "%s: Permission denied: Requires access level %i.",
                THIS_CMD_NAME,
		ACCESS_UID_PSKILLO
            );
            NetSendLiveMessage(condescriptor, sndbuf);
            return(-2);
        }

        /* Recycle the schedual (effectively killing the PID). */
        SchedualRecycle(schedual_num);
        sprintf(
	    sndbuf,
            "%i: Process killed.",
            schedual_num
        );
        NetSendLiveMessage(condescriptor, sndbuf);


        return(0);
}
