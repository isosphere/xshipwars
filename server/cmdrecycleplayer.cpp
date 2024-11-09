#include "../include/unvmatch.h"
#include "../include/xsw_ctype.h"
#include "swserv.h"
#include "net.h"


#define THIS_CMD_NAME	"recycleplayer"

 
/*
 *      Recycles a player object.
 */
int CmdRecyclePlayer(int condescriptor, const char *arg)
{
        int con_obj_num, obj_num;
        xsw_object_struct *obj_ptr, *con_obj_ptr;
        connection_struct *con_ptr;

        char name1[XSW_OBJ_NAME_MAX + 80];
        char name2[XSW_OBJ_NAME_MAX + 80];

        char sndbuf[CS_DATA_MAX_LEN];
        char text[(2 * XSW_OBJ_NAME_MAX) + 512];


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


	/* Skip leading spaces. */
	while(ISBLANK(*arg))
	    arg++;

        /* Parse player name. */
	if(!strcasecmp(arg, "me"))
	    obj_num = con_obj_num;
	else
	    obj_num = MatchObjectByName(
		xsw_object, total_objects,
		arg, XSW_OBJ_TYPE_PLAYER
	    );
        if(DBIsObjectGarbage(obj_num))
        {
            sprintf(sndbuf,
                "%s: No such player.",
                arg
            );
            NetSendLiveMessage(condescriptor, sndbuf);
            return(-1);
        }
        else
        {
            obj_ptr = xsw_object[obj_num];
        }

        /* Cannot recycle player #0. */
        if(obj_num == 0)
        {
            sprintf(
		sndbuf,
                "%s: Permission denied: Cannot recycle %s.",
		THIS_CMD_NAME,
                DBGetFormalNameStr(obj_num)
            );
            NetSendLiveMessage(condescriptor, sndbuf);

            return(-1);
        }
	/* Cannot recycle yourself. */
        if(obj_num == con_obj_num)
        {
            sprintf(
		sndbuf,
                "%s: Permission denied: Cannot recycle yourself.",
		THIS_CMD_NAME
            );
            NetSendLiveMessage(condescriptor, sndbuf);

            return(-1);
        }


        /* Permission check. */
        if(con_obj_ptr->permission.uid > ACCESS_UID_RECYCLEPLAYER)
        {
            sprintf(
		sndbuf,
                "%s: Permission denied: Requires access level %i.",
		THIS_CMD_NAME,
                ACCESS_UID_RECYCLEPLAYER
            );
            NetSendLiveMessage(condescriptor, sndbuf);

            return(-1);
        }


	/* Get names of objects. */
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


        /* Print response. */
        sprintf(
	    sndbuf,
            "Placed %s in recycle backup buffer.",
            name2
        );
        NetSendLiveMessage(condescriptor, sndbuf);

	/* Log. */
        sprintf(
	    text,
            "%s: Recycled %s and placed into recycle backup buffer.",
            name1, name2
        );
        if(sysparm.log_general)
            LogAppendLineFormatted(fname.primary_log, text);


        /* Recycle player. */
        DBSaveRecycledObject(obj_num);
        DBRecycleObject(obj_num);
        NetSendRecycleObject(-1, obj_num);


        return(0);
}
