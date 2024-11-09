#include "../include/isrefs.h"
#include "../include/unvmatch.h"
#include "swserv.h"
#include "net.h"


#define THIS_CMD_NAME           "create"


/*      
 *      Create an object.
 */
int CmdCreate(int condescriptor, const char *arg)
{
	char *strptr;

	int con_obj_num, new_obj_num;
	xsw_object_struct *con_obj_ptr, *new_obj_ptr;
	connection_struct *con_ptr;

        char new_name[XSW_OBJ_NAME_MAX];
	char new_type_name[256];
	int type;

        char name1[XSW_OBJ_NAME_MAX + 80];
        char name2[XSW_OBJ_NAME_MAX + 80];

        char text[(2 * XSW_OBJ_NAME_MAX) + 512];
        char sndbuf[CS_DATA_MAX_LEN];


        /* Get connection's object number (assumed valid). */
        con_ptr = connection[condescriptor];
        con_obj_num = con_ptr->object_num;
        con_obj_ptr = xsw_object[con_obj_num];


        /* Print usage? */
        if((arg == NULL) ? 1 : (*arg == '\0'))
        {
	    sprintf(sndbuf,
		"Usage: `%s <name>[=<type>]'",
                THIS_CMD_NAME
	    );
            NetSendLiveMessage(condescriptor, sndbuf);

            return(-1);
        }

        /* Check if object's permission allows create object. */
        if(con_obj_ptr->permission.uid > ACCESS_UID_CREATE)
        {
            sprintf(sndbuf,
                "%s: Permission denied: Requires access level %i.",
		THIS_CMD_NAME,
                ACCESS_UID_CREATE
            );
            NetSendLiveMessage(condescriptor, sndbuf);
            return(-1);
        }


	/* Parse new object's name. */
	strncpy(new_name, arg, XSW_OBJ_NAME_MAX);
	new_name[XSW_OBJ_NAME_MAX - 1] = '\0';

	strptr = strchr(new_name, '=');
	if(strptr != NULL)
	    *strptr = '\0';

	StringStripSpaces(new_name);


	/* Parse new type name. */
	strptr = (char *) strchr(arg, '=');
	strncpy(
            new_type_name,
	    ((strptr == NULL) ? XSW_TYPE_NAME_STATIC : strptr + 1),
	    256
	);
	new_type_name[256 - 1] = '\0';
	StringStripSpaces(new_type_name);


	/* Sanitize name. */
	if(DBValidateObjectName(new_name)) 
	{
	    sprintf(
		sndbuf,
		"%s: Invalid name.",
		new_name
	    );
	    NetSendLiveMessage(condescriptor, sndbuf);

	    return(-1);
	}


        /* Get type code. */
	if(!strcasecmp(new_type_name, XSW_TYPE_NAME_STATIC))
	    type = XSW_OBJ_TYPE_STATIC;
	else if(!strcasecmp(new_type_name, XSW_TYPE_NAME_DYNAMIC))
            type = XSW_OBJ_TYPE_DYNAMIC;
        else if(!strcasecmp(new_type_name, XSW_TYPE_NAME_CONTROLLED))
            type = XSW_OBJ_TYPE_CONTROLLED;
        else if(!strcasecmp(new_type_name, XSW_TYPE_NAME_PLAYER))
            type = XSW_OBJ_TYPE_PLAYER;
        else if(!strcasecmp(new_type_name, XSW_TYPE_NAME_WEAPON))
            type = XSW_OBJ_TYPE_WEAPON;
        else if(!strcasecmp(new_type_name, XSW_TYPE_NAME_STREAMWEAPON))
            type = XSW_OBJ_TYPE_STREAMWEAPON;
        else if(!strcasecmp(new_type_name, XSW_TYPE_NAME_SPHEREWEAPON))
            type = XSW_OBJ_TYPE_SPHEREWEAPON;
        else if(!strcasecmp(new_type_name, XSW_TYPE_NAME_HOME))
            type = XSW_OBJ_TYPE_HOME;
        else if(!strcasecmp(new_type_name, XSW_TYPE_NAME_AREA))
            type = XSW_OBJ_TYPE_AREA;
        else if(!strcasecmp(new_type_name, XSW_TYPE_NAME_ANIMATED))
            type = XSW_OBJ_TYPE_ANIMATED;
        else if(!strcasecmp(new_type_name, XSW_TYPE_NAME_WORMHOLE))
            type = XSW_OBJ_TYPE_WORMHOLE;
        else if(!strcasecmp(new_type_name, XSW_TYPE_NAME_ELINK))
            type = XSW_OBJ_TYPE_ELINK;
	else
	    type = XSW_OBJ_TYPE_GARBAGE;

        /* Making sure the type is valid and non-garbage. */
        if(type <= XSW_OBJ_TYPE_GARBAGE)
        {
            sprintf(sndbuf,
                "%s: Invalid object type `%s'.",
		THIS_CMD_NAME,
                new_type_name
            );
            NetSendLiveMessage(condescriptor, sndbuf);

            return(-1);   
        }
        /* Cannot create players. */
        if(type == XSW_OBJ_TYPE_PLAYER)
        {
            sprintf(sndbuf,
                "%s: Cannot create player objects, use `createplayer'.",
		THIS_CMD_NAME
            );
            NetSendLiveMessage(condescriptor, sndbuf);

            return(-1);
        }


        /* Create new object. */
        new_obj_num = DBCreateObject(
            ISREF_DEFAULT,	/* Image set. */
            type,		/* Type. */
            con_obj_num,	/* Owner. */
            con_obj_ptr->x,
            con_obj_ptr->y,
            con_obj_ptr->z,
            con_obj_ptr->heading,
            con_obj_ptr->pitch,
            con_obj_ptr->bank
        );
        if(DBIsObjectGarbage(new_obj_num))
        {
            sprintf(sndbuf,
                "%s: DBCreateObject(): Unable to create object.",
                THIS_CMD_NAME
            );
            NetSendLiveMessage(condescriptor, sndbuf);

            return(-1);
        }
	else
	{
	    new_obj_ptr = xsw_object[new_obj_num];
	}


        /* Set some values for the new object. */
        strncpy(
	    new_obj_ptr->name,
	    new_name,
            XSW_OBJ_NAME_MAX
	);
        new_obj_ptr->name[XSW_OBJ_NAME_MAX - 1] = '\0';

        new_obj_ptr->sect_x = con_obj_ptr->sect_x;
        new_obj_ptr->sect_y = con_obj_ptr->sect_y;
        new_obj_ptr->sect_z = con_obj_ptr->sect_z;



        /* New player object has been created. */


        /* Send updates to all connections. */
        NetSendCreateObject(-1, new_obj_num);
        NetSendObjectName(-1, new_obj_num);
        NetSendObjectSect(-1, new_obj_num);


        /* Print and log. */
	strncpy(
	    name1,
	    DBGetFormalNameStr(new_obj_num),
	    XSW_OBJ_NAME_MAX + 80
	);
	name1[XSW_OBJ_NAME_MAX + 80 - 1] = '\0';
        strncpy(
            name2,
            DBGetFormalNameStr(con_obj_num),
            XSW_OBJ_NAME_MAX + 80
        );
        name2[XSW_OBJ_NAME_MAX + 80 - 1] = '\0';


        sprintf(sndbuf,
            "Created new object %s.",
            name1
        );
        NetSendLiveMessage(condescriptor, sndbuf);

        sprintf(text,
            "%s: Created new object: %s",
            name2,
            name1
        );
        if(sysparm.log_events)
            LogAppendLineFormatted(fname.primary_log, text);


        return(0);
}
