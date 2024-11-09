#include "../include/unvmatch.h"
#include "swserv.h"
#include "net.h"  
 
 
/*
 *      Check user ID and group ID of object.
 */
int CmdID(int condescriptor, const char *arg)
{
        int con_obj_num, obj_num;
        xsw_object_struct *obj_ptr, *con_obj_ptr;
        connection_struct *con_ptr;
        char sndbuf[CS_DATA_MAX_LEN];


        /* Get connection's object number (assumed valid). */
        con_ptr = connection[condescriptor];
        con_obj_num = con_ptr->object_num;
        con_obj_ptr = xsw_object[con_obj_num];


        /* If no argument, assume connection's object. */
        if((arg == NULL) ? 1 : (*arg == '\0'))
            obj_num = con_obj_num;
	else if(!strcasecmp(arg, "me"))
	    obj_num = con_obj_num;
        else
            obj_num = MatchObjectByName(
		xsw_object, total_objects,
		arg, XSW_OBJ_TYPE_PLAYER
	    );
	if(DBIsObjectGarbage(obj_num))
	{
            sprintf(
		sndbuf,
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

        /* Print ID. */
        sprintf(
	    sndbuf,
            "%s: uid: %i  gid: %i",
            DBGetFormalNameStr(obj_num),
            obj_ptr->permission.uid,
            obj_ptr->permission.gid
        );
        NetSendLiveMessage(condescriptor, sndbuf);


        return(0);
}
