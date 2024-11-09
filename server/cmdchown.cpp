#include "../include/unvmatch.h"
#include "swserv.h"
#include "net.h"


#define THIS_CMD_NAME	"chown"


int CmdChown(int condescriptor, const char *arg)
{
        char *strptr;
	int tar_obj_num, con_obj_num, new_owner_obj_num;
	xsw_object_struct *tar_obj_ptr, *con_obj_ptr, *new_owner_obj_ptr;
	connection_struct *con_ptr;

        char sndbuf[CS_DATA_MAX_LEN];
        char tar_obj_name[XSW_OBJ_NAME_MAX + 80];
	char con_obj_name[XSW_OBJ_NAME_MAX + 80];
        char new_owner_obj_name[XSW_OBJ_NAME_MAX + 80];

	char text[512 + (XSW_OBJ_NAME_MAX * 3)];


        /* Get connection's object number (assumed valid). */
        con_ptr = connection[condescriptor];
        con_obj_num = con_ptr->object_num;
        con_obj_ptr = xsw_object[con_obj_num];


	/* If argument is empty, then print usage. */
        if((arg == NULL) ? 1 : (*arg == '\0'))
        {
            sprintf(
		sndbuf,
                "Usage: `%s <object>[=<owner>]'",
		THIS_CMD_NAME
            );
            NetSendLiveMessage(condescriptor, sndbuf);

            return(-1);
        }


        /* Parse target object name. */
	strncpy(tar_obj_name, arg, XSW_OBJ_NAME_MAX);
	tar_obj_name[XSW_OBJ_NAME_MAX - 1] = '\0';

	/* Remove anything past the '=' character. */
	strptr = strchr(tar_obj_name, '=');
	if(strptr != NULL) 
            *strptr = '\0';

	StringStripSpaces(tar_obj_name);

	/* Match target object name. */
	tar_obj_num = MatchObjectByName(
	    xsw_object, total_objects,
	    tar_obj_name, -1
	);
        if(DBIsObjectGarbage(tar_obj_num))
        {
            sprintf(
		sndbuf,
                "%s: %s: No such object.",
		THIS_CMD_NAME,
                tar_obj_name
            );
            NetSendLiveMessage(condescriptor, sndbuf);
        
            return(-1);
        }
	else
	{
	    tar_obj_ptr = xsw_object[tar_obj_num];
	}

	/* Target object of type player? */
        if(tar_obj_ptr->type == XSW_OBJ_TYPE_PLAYER)
        {
	    /* Cannot chown players. */
            sprintf(
		sndbuf,
                "%s: Permission denied: Players always own themselves.",
		THIS_CMD_NAME
            );
            NetSendLiveMessage(condescriptor, sndbuf);

            return(-1);
        }


        /* Parse new owner object name. */
        strptr = (char *) strchr(arg, '=');
        if(strptr == NULL)
	    strncpy(
		new_owner_obj_name,
		con_obj_ptr->name,
		XSW_OBJ_NAME_MAX
	    );
	else
            strncpy(
		new_owner_obj_name,
		strptr + 1,
		XSW_OBJ_NAME_MAX
	    );
	new_owner_obj_name[XSW_OBJ_NAME_MAX - 1] = '\0';
	StringStripSpaces(new_owner_obj_name);

	/* Match new owner object. */
	if(!strcasecmp(new_owner_obj_name, "me"))
	    new_owner_obj_num = con_obj_num;
	else
	    new_owner_obj_num = MatchObjectByName(
		xsw_object, total_objects,
		new_owner_obj_name, XSW_OBJ_TYPE_PLAYER
	    );
	if(DBIsObjectGarbage(new_owner_obj_num))
	{
	    sprintf(
		sndbuf,
		"%s: No such player.",
		new_owner_obj_name
	    );
            NetSendLiveMessage(condescriptor, sndbuf);

            return(-1);
        }
	else
	{
	    new_owner_obj_ptr = xsw_object[new_owner_obj_num];
	}


	/* Check if target object's current owner is the same
	 * as the new owner.
	 */
	if(tar_obj_ptr->owner == new_owner_obj_num)
	{
            sprintf(
                sndbuf,
                "%s: %s: Already owned by %s.",
                THIS_CMD_NAME,
                tar_obj_name,
		new_owner_obj_name
            );
            NetSendLiveMessage(condescriptor, sndbuf);

            return(0);
	}


        /* Check permissions, allowed to chown? */
        if(con_obj_ptr->permission.uid > ACCESS_UID_CHOWN)
        {
            sprintf(
		sndbuf,
                "%s: Permission denied: Requires access level %i.",
		THIS_CMD_NAME,
                ACCESS_UID_CHOWN
            );
            NetSendLiveMessage(condescriptor, sndbuf);

            return(-1); 
        }

        /* If the target object owned by someone else, then check
	 * connection object is allowed to chown objects belonging
	 * to someone else.
	 */
        if((con_obj_ptr->permission.uid > ACCESS_UID_CHOWNO) &&
           (tar_obj_ptr->owner > -1) &&
           (tar_obj_ptr->owner != con_obj_num)
        )
        {
            sprintf(
		sndbuf,
                "%s: Permission denied: Requires access level %i.",
		THIS_CMD_NAME,
                ACCESS_UID_CHOWNO
            );
            NetSendLiveMessage(condescriptor, sndbuf);

            return(-1);
        }


        /* Chown object. */
        tar_obj_ptr->owner = new_owner_obj_num;


	/* Get formal names of the objects. */
        strncpy(
	    con_obj_name,
	    DBGetFormalNameStr(con_obj_num),
	    XSW_OBJ_NAME_MAX + 80
	);
	con_obj_name[XSW_OBJ_NAME_MAX + 80 - 1] = '\0';

        strncpy(
            tar_obj_name,
            DBGetFormalNameStr(tar_obj_num),
            XSW_OBJ_NAME_MAX + 80
        );
        tar_obj_name[XSW_OBJ_NAME_MAX + 80 - 1] = '\0';

        strncpy(
            new_owner_obj_name,
            DBGetFormalNameStr(new_owner_obj_num),
            XSW_OBJ_NAME_MAX + 80
        );
        new_owner_obj_name[XSW_OBJ_NAME_MAX + 80 - 1] = '\0';


        /* Log and print response. */
        sprintf(
	    sndbuf,
            "Chowned %s to %s.",
            tar_obj_name,
            new_owner_obj_name
        );
        NetSendLiveMessage(condescriptor, sndbuf);

        sprintf(
	    text,
            "%s: Chowned %s to %s",
            con_obj_name, tar_obj_name, new_owner_obj_name
        );
        if(sysparm.log_general)
            LogAppendLineFormatted(fname.primary_log, text);


        return(0);
}
