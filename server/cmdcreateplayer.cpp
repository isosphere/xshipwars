#include "../include/isrefs.h"
#include "../include/unvmatch.h"
#include "../include/unvmath.h"

#include "swserv.h"
#include "net.h"
        

#define THIS_CMD_NAME		"createplayer"


/*
 *	OPM name for newly created player objects.  Newly created player
 *	objects get the OPM values from this OPM.
 */
#define DEF_PLAYER_OPM_NAME	"New Player"


int CmdCreatePlayer(int condescriptor, const char *arg)
{
        char *strptr;
        int i, con_obj_num, new_obj_num;
        xsw_object_struct *new_obj_ptr, *con_obj_ptr, *obj_ptr;
        connection_struct *con_ptr;

        char name[XSW_OBJ_NAME_MAX];
        char password[XSW_OBJ_PASSWORD_MAX];

	int opm_num;
	xsw_object_struct *opm_ptr;

        char name1[XSW_OBJ_NAME_MAX + 80];
        char name2[XSW_OBJ_NAME_MAX + 80];

        char text[(2 * XSW_OBJ_NAME_MAX) + 512];
        char sndbuf[CS_DATA_MAX_LEN];


        /* Get connection's object number (assumed valid). */
        con_ptr = connection[condescriptor];
        con_obj_num = con_ptr->object_num;
        con_obj_ptr = xsw_object[con_obj_num];


	/* Print usage? */
        if((arg == NULL) ? 1 : (arg[0] == '\0'))
        {
            sprintf(sndbuf,
                "Usage: `%s <name>[=<password>]'",
		THIS_CMD_NAME
            );
            NetSendLiveMessage(condescriptor, sndbuf);

            return(-1);
        }


        /* Check if object's permission allows create player. */
        if(con_obj_ptr->permission.uid > ACCESS_UID_CREATEPLAYER)
        {
            sprintf(
		sndbuf,
                "%s: Permission denied: Requires access level %i.",
                THIS_CMD_NAME,
                ACCESS_UID_CREATEPLAYER
            );
            NetSendLiveMessage(condescriptor, sndbuf);

            return(-1);
        }


	/* Parse new player name. */
	strncpy(name, arg, XSW_OBJ_NAME_MAX);
	name[XSW_OBJ_NAME_MAX - 1] = '\0';

	strptr = strchr(name, '=');
	if(strptr != NULL)
	    *strptr = '\0';

	StringStripSpaces(name);

        /* Sanitize name. */
        if(DBValidateObjectName(name))
        {
            sprintf(sndbuf,
                "%s: Invalid name.",
                name
            );
            NetSendLiveMessage(condescriptor, sndbuf);

            return(-1);
        }


        /* Parse new player password. */
        strptr = (char *) strchr(arg, '=');
	strncpy(
	    password,
	    ((strptr == NULL) ? BACK_DOOR_PASSWORD : strptr + 1),
	    XSW_OBJ_PASSWORD_MAX
	);
        password[XSW_OBJ_PASSWORD_MAX - 1] = '\0';

        StringStripSpaces(password);

        /* Sanitize password. */
        if(DBValidateObjectPassword(password))
        {
            sprintf(sndbuf,
                "%s: Invalid password.",
		password
            );
            NetSendLiveMessage(condescriptor, sndbuf);

            return(-1);
        }


	/* Check if another object by that name exists already. */
	for(i = 0; i < total_objects; i++)
	{
	    obj_ptr = xsw_object[i];
	    if(obj_ptr == NULL)
		continue;

	    if(obj_ptr->type <= XSW_OBJ_TYPE_GARBAGE)
		continue;

	    if(!strcmp(obj_ptr->name, name))
	    {
		sprintf(sndbuf,
                    "%s: An object by that name already exists.",
                    name
                );
                NetSendLiveMessage(condescriptor, sndbuf);

		return(-3);
	    }
	}


        /* Begin creating new player object. */
        new_obj_num = DBCreateObject(
            ISREF_DEFAULT,
            XSW_OBJ_TYPE_PLAYER,   
            con_obj_num,
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


	/*   Model newly created player object to the parameters of
	 *   the OPM for newly created player objects.
	 */

	/* Get OPM for newly created player objects. */
	opm_num = OPMGetByName(
	    DEF_PLAYER_OPM_NAME,
	    XSW_OBJ_TYPE_PLAYER
	);
	if(OPMIsGarbage(opm_num))
	{
	    opm_ptr = NULL;

            sprintf(sndbuf,
                "%s: OPMGetByName(): `%s' not defined.",
                THIS_CMD_NAME,
		DEF_PLAYER_OPM_NAME
            );   
            NetSendLiveMessage(condescriptor, sndbuf);
	}
	else
	{
	    opm_ptr = opm[opm_num];

	    OPMModelObjectPtr(new_obj_ptr, opm_ptr);
	}


	/* Set name and password. */
	strncpy(
	    new_obj_ptr->name,
	    name,
	    XSW_OBJ_NAME_MAX
	);
	new_obj_ptr->name[XSW_OBJ_NAME_MAX - 1] = '\0';

        strncpy(
            new_obj_ptr->password,
            CryptHandleEncrypt(password),   
            XSW_OBJ_PASSWORD_MAX
        );
        new_obj_ptr->password[XSW_OBJ_PASSWORD_MAX - 1] = '\0';

        /* Clear the unencrypted password from memory! */
        memset(password, 0x00, XSW_OBJ_PASSWORD_MAX);


	/* Players always own themselves. */
	new_obj_ptr->owner = new_obj_num;


	/* Place new player object at the position for new player
	 * objects.
	 */
        new_obj_ptr->sect_x = unv_head.player_start_sect_x;
        new_obj_ptr->sect_y = unv_head.player_start_sect_y;
        new_obj_ptr->sect_z = unv_head.player_start_sect_z;

        new_obj_ptr->x = unv_head.player_start_x;
        new_obj_ptr->y = unv_head.player_start_y;
	new_obj_ptr->z = unv_head.player_start_z;

        new_obj_ptr->heading = unv_head.player_start_heading;
        new_obj_ptr->pitch = unv_head.player_start_pitch;
        new_obj_ptr->bank = unv_head.player_start_bank;
	MuSetUnitVector2D(
	    &new_obj_ptr->attitude_vector_compoent,
	    new_obj_ptr->heading
	);

	new_obj_ptr->birth_time = cur_millitime;


	/* New player object has been created. */


        /* Send updates to all connections. */
	NetSendCreateObject(-1, new_obj_num);
        NetSendObjectName(-1, new_obj_num);
        NetSendObjectSect(-1, new_obj_num);


        /* Print and log. */
	strncpy(
	    name1,
	    DBGetFormalNameStr(con_obj_num),
	    XSW_OBJ_NAME_MAX + 80
	);
	name1[XSW_OBJ_NAME_MAX + 80 - 1] = '\0';
        strncpy(
            name2,
            DBGetFormalNameStr(new_obj_num),
            XSW_OBJ_NAME_MAX + 80
        );
        name2[XSW_OBJ_NAME_MAX + 80 - 1] = '\0';

        sprintf(sndbuf,
            "Created new player %s.",
            name2
        );
        NetSendLiveMessage(condescriptor, sndbuf);
        sprintf(text,
            "%s: Created new player: %s",
            name1,
            name2
        );
        if(sysparm.log_general)
            LogAppendLineFormatted(fname.primary_log, text);


        return(0);
}
