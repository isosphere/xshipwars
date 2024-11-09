#include "../include/unvmatch.h"
#include "swserv.h"
#include "net.h"

#define THIS_CMD_NAME	"boot"

/*
 *      Boot off a player.
 */
int CmdBoot(int condescriptor, const char *arg)
{
	int con_num, con_obj_num, obj_num;
        xsw_object_struct *con_obj_ptr;
	connection_struct *con_ptr;

        char sndbuf[CS_DATA_MAX_LEN];
        char text[(2 * XSW_OBJ_NAME_MAX) + 512];
	char name1[XSW_OBJ_NAME_MAX + 80];
	char name2[XSW_OBJ_NAME_MAX + 80];


        /* Get connection's object number (assumed valid). */
        con_ptr = connection[condescriptor];
        con_obj_num = con_ptr->object_num;
        con_obj_ptr = xsw_object[con_obj_num];


        /* Print usage? */ 
        if((arg == NULL) ? 1 : (*arg == '\0'))
        {
	    sprintf(
		sndbuf,
                "Usage: `%s <player>'",
                THIS_CMD_NAME
            );
            NetSendLiveMessage(condescriptor, sndbuf);

            return(-1);
        }


        /* Permission check. */
        if(con_obj_ptr->permission.uid > ACCESS_UID_BOOT)
        {
            sprintf(
		sndbuf,
                "%s: Permission denied: Requires access level %i.",
                THIS_CMD_NAME, ACCESS_UID_BOOT
            );
            NetSendLiveMessage(condescriptor, sndbuf);

            return(-1);
        }


        /* Match the object_num to be booted. */
        if(!strcasecmp(arg, "me"))
            obj_num = con_obj_num;
        else
            obj_num = MatchObjectByName(
		xsw_object, total_objects,
		arg, XSW_OBJ_TYPE_PLAYER
	    );

        /* Valid match? */
        if(DBIsObjectGarbage(obj_num))
        {
            sprintf(
		sndbuf,
                "%s: No such player.",
                THIS_CMD_NAME
            );
            NetSendLiveMessage(condescriptor, sndbuf);

            return(-1);
        }

        /* Is that object connected? */
	con_num = ConGetByObject(obj_num);
	if(con_num < 0)
        {
            sprintf(
		sndbuf,
                "%s: Not connected.",
                arg
            );
            NetSendLiveMessage(condescriptor, sndbuf);

            return(-1);
        }

	/* Get formal names of objects. */
	strncpy(
	    name1,
	    DBGetFormalNameStr(con_obj_num),
	    XSW_OBJ_NAME_MAX + 80
	);
	name1[XSW_OBJ_NAME_MAX + 80 - 1] = '\0';

        strncpy(
            name2,
            DBGetFormalNameStr(obj_num),
            XSW_OBJ_NAME_MAX + 80
        );
        name2[XSW_OBJ_NAME_MAX + 80 - 1] = '\0';


	/* Notify connection. */
        sprintf(
            sndbuf,
            "Booted %s on connection %i.",
            name2,
            con_num
        );
        NetSendLiveMessage(condescriptor, sndbuf);

	/* Log. */
        sprintf(text,
            "%s: Booted: %s on connection %i.",
            name1, name2, con_num
        );
        if(sysparm.log_general)
            LogAppendLineFormatted(fname.primary_log, text);


        /* Boot connection. */
        NetCloseConnection(con_num);


        return(0);
}
