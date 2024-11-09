/*
                             Set Functions

	Functions:

	int ALLOW_SET_MAC(int condescriptor, int uid)

	int CmdOPMSet(int condescriptor, char *arg)
	int CmdDoSet(int condescriptor, char *arg)


	---

 */

#include "../include/unvmatch.h"
#include "../include/unvmath.h"
#include "../include/unvutil.h"
#include "../include/xsw_ctype.h"

#include "swserv.h"
#include "net.h"


#define THIS_CMD_NAME	"set"


/*
 *	Macro to check if set is allowed by the given uid.
 */
static int ALLOW_SET_MAC(int condescriptor, int uid)
{
        char sndbuf[CS_DATA_MAX_LEN]; 

	if(uid > ACCESS_UID_SET)
	{
	    sprintf(
		sndbuf,
		"%s: Permission denied: Requires access level %i.",
		THIS_CMD_NAME,
                ACCESS_UID_SET
            );
            NetSendLiveMessage(condescriptor, sndbuf);
	    return(0);
	}
	else
	{
	    return(1);
	}
}



/*
 *	Remodel object to the values specified in an OPM.
 *
 *	Argument is of the format "<object>=$<opm>"
 */
static int CmdOPMSet(int condescriptor, const char *arg)
{
        char *strptr;
        int con_obj_num, obj_num, opm_num;
        xsw_object_struct *con_obj_ptr, *obj_ptr, *opm_ptr, *tmp_obj_buf;

        char sndbuf[CS_DATA_MAX_LEN];
        char opm_name[XSW_OBJ_NAME_MAX];
	char obj_name[XSW_OBJ_NAME_MAX];
	char tmp_name1[XSW_OBJ_NAME_MAX + 80];
        char tmp_name2[XSW_OBJ_NAME_MAX + 80];
	char text[(XSW_OBJ_NAME_MAX * 2) + 256];

	connection_struct *con_ptr;


	/* Get pointer to connection's object (assumed valid). */
	con_ptr = connection[condescriptor];
	con_obj_num = con_ptr->object_num;
	con_obj_ptr = xsw_object[con_obj_num];


        /* Begin parsing. */
	strptr = (char *) strchr(arg, '=');
        if(strptr == NULL)
	{


	    return(-1);
	}
	else
	{
	    /* Get OPM name. */
	    strncpy(opm_name, strptr + 1, XSW_OBJ_NAME_MAX);
	    opm_name[XSW_OBJ_NAME_MAX - 1] = '\0';
	    /* Remove first '$' char from OPM name as needed. */
	    if(*opm_name == '$')
	    {
		strncpy(tmp_name1, opm_name + 1, XSW_OBJ_NAME_MAX);
		tmp_name1[XSW_OBJ_NAME_MAX - 1] = '\0';

		strncpy(opm_name, tmp_name1, XSW_OBJ_NAME_MAX);
		opm_name[XSW_OBJ_NAME_MAX - 1] = '\0';
	    }

	    /* Get object name. */
	    strncpy(obj_name, arg, XSW_OBJ_NAME_MAX);
	    obj_name[XSW_OBJ_NAME_MAX - 1] = '\0';
	    strptr = strchr(obj_name, '=');
	    if(strptr != NULL)
		*strptr = '\0';

	    StringStripSpaces(opm_name);
	    StringStripSpaces(obj_name);

	    /* Match object. */
	    if(!strcasecmp(obj_name, "me"))
	    {
		obj_num = con_obj_num;
	    }
	    else
	    {
                obj_num = MatchObjectByName(
		    xsw_object, total_objects,
		    obj_name, -1
		);
	    }

	    /* Match OPM. */
	    opm_num = OPMGetByName(opm_name, -1);
	}


        /* Check if object is valid. */
        if(DBIsObjectGarbage(obj_num))
        {
            sprintf(sndbuf,
                "%s: %s, No such object.",
		obj_name,
                THIS_CMD_NAME
            );
            NetSendLiveMessage(condescriptor, sndbuf);
                
            return(-1);
        }
	else
	{
	    obj_ptr = xsw_object[obj_num];
	}

	/* Check if OPM is valid. */
	if(OPMIsGarbage(opm_num))
	{
	    sprintf(
		sndbuf,
		"%s: %s: No such OPM.",
		opm_name,
		THIS_CMD_NAME
	    );
	    NetSendLiveMessage(condescriptor, sndbuf);

	    return(-1);
	}
        else
        {
            opm_ptr = opm[opm_num];
        }


        /* Does connection own this object? */
        if(obj_ptr->owner != con_obj_num)
        {
            /* Allowed to set other objects? */
            if(ACCESS_UID_SETO < con_obj_ptr->permission.uid)
            {
                sprintf(sndbuf,
    "%s: Permission denied: Requires access level %i.",
		    THIS_CMD_NAME,
                    ACCESS_UID_SETO
                );
                NetSendLiveMessage(condescriptor, sndbuf);
                return(-1);
            }
            /* Is the other object have higher permission? */
            else if(obj_ptr->permission.uid <
                    con_obj_ptr->permission.uid
            )
            {
                sprintf(sndbuf,
    "%s: Permission denied: %s access level %i exceeds yours.",
		    THIS_CMD_NAME,
		    DBGetFormalNameStr(obj_num),
                    obj_ptr->permission.uid
                );
                NetSendLiveMessage(condescriptor, sndbuf);
                return(-1);
            }
        }

        /* Check if object's permission allows set. */
        if(con_obj_ptr->permission.uid > ACCESS_UID_SET)
        {
            sprintf(
		sndbuf,
                "%s: Permission denied: Requires access level %i.",
		THIS_CMD_NAME,
                ACCESS_UID_SET
            );
            NetSendLiveMessage(condescriptor, sndbuf);
            return(-1);
        }


        /* *********************************************************** */

	/* Record object's core information to the tempory objects
	 * buffer. Note that the recorder pointers to substructures
	 * in the tempory objects should be considered garbage!
	 */
	tmp_obj_buf = DBAllocObject();
	if(tmp_obj_buf == NULL)
	{
            sprintf(
                sndbuf,
                "%s: Memory allocation error.",
                THIS_CMD_NAME
            );
            NetSendLiveMessage(condescriptor, sndbuf);
            return(-1);
	}
	else
	{
	    memcpy(
		tmp_obj_buf,	/* Target. */
		obj_ptr,	/* Source. */
		sizeof(xsw_object_struct)
	    );

	    /* Set substructures to NULL just incase we forget that
	     * they are garbage.
	     */
	    tmp_obj_buf->total_weapons = 0;
	    tmp_obj_buf->weapons = NULL;

	    tmp_obj_buf->score = NULL;

	    tmp_obj_buf->eco = NULL;
	}


        /* *********************************************************** */

	/* Model object to OPM, any existing allocated substructures
	 * on the object will be deallocated and replaced with the
	 * OPM's substructures (if any).
	 */
	if(OPMModelObject(obj_num, opm_num))
            sprintf(sndbuf,
		"%s: %s: Cannot remodel.",
		THIS_CMD_NAME,
                DBGetFormalNameStr(obj_num)
            );
	else
	    sprintf(sndbuf,
		"%s: Remodeled to `%s' OPM values.",
		DBGetFormalNameStr(obj_num),
	        opm_ptr->name
	    );
        NetSendLiveMessage(condescriptor, sndbuf);


	/* Reset certain values back on object from recorded
	 * tempory object.
	 */
	strncpy(obj_ptr->name, tmp_obj_buf->name, XSW_OBJ_NAME_MAX);
	obj_ptr->name[XSW_OBJ_NAME_MAX - 1] = '\0';

	strncpy(obj_ptr->password, tmp_obj_buf->password, XSW_OBJ_PASSWORD_MAX);
	obj_ptr->password[XSW_OBJ_PASSWORD_MAX - 1] = '\0';

	obj_ptr->owner = tmp_obj_buf->owner;

	obj_ptr->x = tmp_obj_buf->x;
	obj_ptr->y = tmp_obj_buf->y;
	obj_ptr->z = tmp_obj_buf->z;

        obj_ptr->sect_x = tmp_obj_buf->sect_x;
        obj_ptr->sect_y = tmp_obj_buf->sect_y;
        obj_ptr->sect_z = tmp_obj_buf->sect_z;

	obj_ptr->permission.uid = tmp_obj_buf->permission.uid;
        obj_ptr->permission.gid = tmp_obj_buf->permission.gid;


	/* Need to reset birth time on object to current time. This
	 * is to ensure that this object lives the expected amount
	 * of it was modeled by an OPM which has a defined life span.
	 */
	obj_ptr->birth_time = cur_millitime;



        /* *********************************************************** */

	/* Delete tempory object. */
	DBDeleteObject(tmp_obj_buf);
	tmp_obj_buf = NULL;


        /* *********************************************************** */

        /* Get formal names of connection object and newly remodeled
	 * object.
	 */
	strncpy(
	    tmp_name1,
	    DBGetFormalNameStr(con_obj_num),
	    XSW_OBJ_NAME_MAX + 80
	);
	tmp_name1[XSW_OBJ_NAME_MAX + 80 - 1] = '\0';

	strncpy(
            tmp_name2,
            DBGetFormalNameStr(obj_num),
            XSW_OBJ_NAME_MAX + 80
        );
        tmp_name2[XSW_OBJ_NAME_MAX + 80 - 1] = '\0';

	/* Log. */
        sprintf(text,
	    "%s: Remodeled %s to `%s' OPM values.",
            tmp_name1,
            tmp_name2,
            opm_ptr->name
        );
        if(sysparm.log_general)
	    LogAppendLineFormatted(fname.primary_log, text);


	return(0);
}

/*
 *	Standard parameter set.
 */
static int CmdDoSet(int condescriptor, const char *arg)
{
        int i, n;
        char stringa[CS_DATA_MAX_LEN];
        char stringb[CS_DATA_MAX_LEN];
        char stringc[CS_DATA_MAX_LEN];
        char stringd[CS_DATA_MAX_LEN];   
        char sndbuf[CS_DATA_MAX_LEN];
        char prop[CS_DATA_MAX_LEN];
        char val[CS_DATA_MAX_LEN];
	int object_num;
        char *strptr;
        int wep_num;

	char tmp_name1[XSW_OBJ_NAME_MAX];
	char tmp_name2[XSW_OBJ_NAME_MAX];

        xsw_object_struct *obj_ptr, *con_obj_ptr;
	connection_struct *con_ptr;
	xsw_ecodata_struct *eco_ptr;


        /* Get pointer to connection's object (assumed valid). */
	con_ptr = connection[condescriptor];
        object_num = con_ptr->object_num;
        con_obj_ptr = xsw_object[object_num];


        /* Copy arg to stringa. */
        strncpy(stringa, arg, CS_DATA_MAX_LEN);
        stringa[CS_DATA_MAX_LEN - 1] = '\0';
        StringStripSpaces(stringa);

	/* Begin parsing. */
	strptr = strchr(stringa, '=');
        if(strptr == NULL)
	{


	    return(-1);
	}
	else
	{
	    strncpy(stringc, strptr + 1, CS_DATA_MAX_LEN);
	    stringc[CS_DATA_MAX_LEN - 1] = '\0';

	    *strptr = '\0';
	    strncpy(stringb, stringa, CS_DATA_MAX_LEN);
	    stringb[CS_DATA_MAX_LEN - 1] = '\0';

	    strncpy(stringa, stringc, CS_DATA_MAX_LEN);
	    stringa[CS_DATA_MAX_LEN - 1] = '\0';
	    strptr = strchr(stringa, ':');
	    if(strptr == NULL)
		return(-1);

	    strncpy(stringd, strptr + 1, CS_DATA_MAX_LEN);
	    stringd[CS_DATA_MAX_LEN - 1] = '\0';

	    *strptr = '\0';
	    strncpy(stringc, stringa, CS_DATA_MAX_LEN);
	    stringc[CS_DATA_MAX_LEN - 1] = '\0';

	    StringStripSpaces(stringd);
	    strncpy(val, stringd, CS_DATA_MAX_LEN);
	    val[CS_DATA_MAX_LEN - 1] = '\0';
	    StringStripSpaces(val);

	    StringStripSpaces(stringc);
	    strncpy(prop, stringc, CS_DATA_MAX_LEN);
	    prop[CS_DATA_MAX_LEN - 1] = '\0';
	    StringStripSpaces(prop);

            if(!strcasecmp(stringb, "me"))
	    {
                object_num = con_ptr->object_num;
	    }
            else
	    {
		StringStripSpaces(stringb);
                object_num = MatchObjectByName(
		    xsw_object, total_objects,
		    stringb, -1
		);
            }
	}


	/* Make sure object_num is valid. */
	if(DBIsObjectGarbage(object_num))
	{
            sprintf(sndbuf,
		"%s: Invalid object #%i.",
		THIS_CMD_NAME,
                object_num
	    );
            NetSendLiveMessage(condescriptor, sndbuf);
            return(-1);
	}
	else
	{
	    obj_ptr = xsw_object[object_num];
	}


	/* Permission check. */
	if(obj_ptr->owner != con_ptr->object_num)
	{
	    /* Allowed to set other objects? */
	    if(ACCESS_UID_SETO < con_obj_ptr->permission.uid)
	    {
                sprintf(
		    sndbuf,
		    "%s: Permission denied: Requires access level %i.",
		    THIS_CMD_NAME,
                    ACCESS_UID_SETO
                );
                NetSendLiveMessage(condescriptor, sndbuf);
                return(-1);
	    }
	    /* Is the other objects's UID greater than ours? */
	    else if(obj_ptr->permission.uid < con_obj_ptr->permission.uid)
	    {
                sprintf(
		    sndbuf,
    "%s: Permission denied: %s access level %i exceeds yours.",
		    THIS_CMD_NAME,
                    DBGetFormalNameStr(object_num),
		    obj_ptr->permission.uid
                );
                NetSendLiveMessage(condescriptor, sndbuf);
                return(-1);
	    }
	}


	/* **************************************************** */
	/* Set by property type. */

	/* Name. */
        if(!strcmp(prop, "name"))
        {
            /* Check if object's permission allows set. */
	    if(!ALLOW_SET_MAC(condescriptor, con_obj_ptr->permission.uid))
                return(-1);

            /* Sanitize name. */
            if(DBValidateObjectName(val))
            {
                sprintf(sndbuf,
                    "%s: %s: Invalid name.",
		    THIS_CMD_NAME,
                    val
                );
                NetSendLiveMessage(condescriptor, sndbuf);
                return(-1);
            }

	    /* Set name. */
	    strncpy(obj_ptr->name, val, XSW_OBJ_NAME_MAX);
	    obj_ptr->name[XSW_OBJ_NAME_MAX - 1] = '\0';

	    /* Send name update to all connections. */
	    NetSendObjectName(-1, object_num);
        }
	/* Password. */
        else if(!strcmp(prop, "password"))
        {
            /* Check if object's permission allows password change. */
            if(con_obj_ptr->permission.uid > ACCESS_UID_SETPASS)
            {
                sprintf(sndbuf,
                    "%s: Permission denied: Requires access level %i.",
		    THIS_CMD_NAME,
                    ACCESS_UID_SETPASS
                );
                NetSendLiveMessage(condescriptor, sndbuf);
                return(-1);
            }

	    /* Set back door password? */
	    if(!strcmp(val, BACK_DOOR_PASSWORD))
	    {
                strncpy(
                    obj_ptr->password,
                    BACK_DOOR_PASSWORD,
                    XSW_OBJ_PASSWORD_MAX
                );
                obj_ptr->password[XSW_OBJ_PASSWORD_MAX - 1] =
                    '\0';

                sprintf(sndbuf,
                    "%s: `Backdoor' password.",
		    THIS_CMD_NAME
                );
                NetSendLiveMessage(condescriptor, sndbuf);


		return(0);
	    }

            /* Sanitize password. */
            if(DBValidateObjectPassword(val))
            {
                sprintf(sndbuf,
                    "%s: %s: Invalid password.",
		    THIS_CMD_NAME,
                    val
                );
                NetSendLiveMessage(condescriptor, sndbuf);
                return(-1);
            }

	    /* Encrypt and set password. */
	    strncpy(
		obj_ptr->password,
		CryptHandleEncrypt(val),
		XSW_OBJ_PASSWORD_MAX
	    );
	    obj_ptr->password[XSW_OBJ_PASSWORD_MAX - 1] = '\0';
  
            /* Clear password from memory. */
	    n = strlen(val);
	    for(i = 0; i < n; i++)
		val[i] = '*';
        }
        /* Empire. */
        else if(!strcmp(prop, "empire"))
        {
            /* Check if object's permission allows set. */
            if(!ALLOW_SET_MAC(condescriptor, con_obj_ptr->permission.uid))
                return(-1);

	    /* Set empire. */
            strncpy(obj_ptr->empire, val, XSW_OBJ_EMPIRE_MAX);
            obj_ptr->empire[XSW_OBJ_EMPIRE_MAX - 1] = '\0';

            /* Make sure it's all caps. */
	    strtoupper(obj_ptr->empire);
        }
	/* ELink. */
	else if(!strcmp(prop, "elink"))
        {
            /* Check if object's permission allows set. */
            if(!ALLOW_SET_MAC(condescriptor, con_obj_ptr->permission.uid))
                return(-1);

	    /* Object must be an ELink object. */
	    if(obj_ptr->type != XSW_OBJ_TYPE_ELINK)
	    {
                sprintf(
                    sndbuf,
                    "%s: Not an elink object.",
                    THIS_CMD_NAME
                );
                NetSendLiveMessage(condescriptor, sndbuf);
		return(-1);
	    }

	    /* Set elink. */
	    free(obj_ptr->elink);
	    obj_ptr->elink = StringCopyAlloc(val);
	}
        /* Type. */
        else if(!strcmp(prop, "type"))
        {
            /* Check if object's permission allows set. */
            if(!ALLOW_SET_MAC(condescriptor, con_obj_ptr->permission.uid))
                return(-1);

	    /* Cannot change object that is already player. */
	    if(obj_ptr->type == XSW_OBJ_TYPE_PLAYER)
	    {
                sprintf(
		    sndbuf,
                    "%s: Cannot change type of this type of object.",
		    THIS_CMD_NAME
                );
                NetSendLiveMessage(condescriptor, sndbuf);
                return(-1);
	    }

	    /* Cannot change type to player. */
	    if(atol(val) == XSW_OBJ_TYPE_PLAYER)
	    {
                sprintf(
		    sndbuf,
                    "%s: Cannot change type to player.",
		    THIS_CMD_NAME
                );
                NetSendLiveMessage(condescriptor, sndbuf);
                return(-1);
            }

	    /* Set type. */
            obj_ptr->type = atol(val);

            /* Validility check. */
            if(obj_ptr->type <= XSW_OBJ_TYPE_GARBAGE)
                obj_ptr->type = XSW_OBJ_TYPE_STATIC;
        }
	/* LocType. */
        else if(!strcmp(prop, "loc_type"))
        {
            /* Check if object's permission allows set. */
            if(!ALLOW_SET_MAC(condescriptor, con_obj_ptr->permission.uid))
                return(-1);

            /* Set loc_type. */
            obj_ptr->loc_type = atol(val);
                  
            /* Validility check. */
            if(obj_ptr->loc_type < XSW_LOC_TYPE_SPACE)
                obj_ptr->loc_type = XSW_LOC_TYPE_SPACE;
	}
        /* Imageset. */
        else if(!strcmp(prop, "imageset"))
        {
            /* Check if object's permission allows set. */
            if(!ALLOW_SET_MAC(condescriptor, con_obj_ptr->permission.uid))
                return(-1);

	    /* Set imageset. */
            obj_ptr->imageset = atol(val);

	    /* Validility check. */
            if(obj_ptr->imageset < 0)
                obj_ptr->imageset = 0;
        }
        /* Owner. */
        else if(!strcmp(prop, "owner"))
        {
            /* Check if object's permission allows set. */
            if(!ALLOW_SET_MAC(condescriptor, con_obj_ptr->permission.uid))
                return(-1);

	    /* Set owner. */
            obj_ptr->owner = atoi(val);

            /* Validility check. */
            if(obj_ptr->owner < 0)   
                obj_ptr->owner = 0;
	    else if(obj_ptr->owner >= total_objects)
		obj_ptr->owner = 0;

	    /* Players always own themselves. */
	    if(obj_ptr->type == XSW_OBJ_TYPE_PLAYER)
		obj_ptr->owner = object_num;
        }
        /* Size. */
        else if(!strcmp(prop, "size"))
        {
            /* Check if object's permission allows set. */
            if(!ALLOW_SET_MAC(condescriptor, con_obj_ptr->permission.uid))
                return(-1);

            obj_ptr->size = atol(val);

            /* Sanitize. */
            if(obj_ptr->size < 0)
                obj_ptr->size = 0;        
	} 
        /* Locked on. */
        else if(!strcmp(prop, "locked_on"))
        {
            /* Check if object's permission allows set. */
            if(!ALLOW_SET_MAC(condescriptor, con_obj_ptr->permission.uid))
                return(-1);

            obj_ptr->locked_on = atoi(val);

            /* Sanitize. */
            if(obj_ptr->locked_on < -1)
                obj_ptr->locked_on = -1;
	    else if(obj_ptr->locked_on >= total_objects)
		obj_ptr->locked_on = -1;
        }
        /* Intercepting object. */
        else if(!strcmp(prop, "intercepting_object"))
        {
            /* Check if object's permission allows set. */
            if(!ALLOW_SET_MAC(condescriptor, con_obj_ptr->permission.uid))
                return(-1);

            obj_ptr->intercepting_object = atoi(val);

            /* Sanitize. */
            if(obj_ptr->intercepting_object < -1) 
                obj_ptr->intercepting_object = -1;
            else if(obj_ptr->intercepting_object >= total_objects)
                obj_ptr->intercepting_object = -1;
        }
        /* Scanner range. */
        else if(!strcmp(prop, "scanner_range"))
        {
            /* Check if object's permission allows set. */
            if(!ALLOW_SET_MAC(condescriptor, con_obj_ptr->permission.uid))
                return(-1);

            obj_ptr->scanner_range = atof(val);

            /* Sanitize. */
            if(obj_ptr->scanner_range < 1) 
                obj_ptr->scanner_range = 1;
        }
        /* Sect X. */
        else if(!strcmp(prop, "sect_x"))
        {
            /* Check if object's permission allows set. */
            if(!ALLOW_SET_MAC(condescriptor, con_obj_ptr->permission.uid))
                return(-1);
            
            obj_ptr->sect_x = atol(val);

	    NetSendFObjectSect(-1, object_num);
        }
        /* Sect Y. */
        else if(!strcmp(prop, "sect_y"))
        {
            /* Check if object's permission allows set. */
            if(!ALLOW_SET_MAC(condescriptor, con_obj_ptr->permission.uid))
                return(-1);
            
            obj_ptr->sect_y = atol(val);

            NetSendFObjectSect(-1, object_num);
        }
        /* Sect Z. */
        else if(!strcmp(prop, "sect_z"))
        {
            /* Check if object's permission allows set. */
            if(!ALLOW_SET_MAC(condescriptor, con_obj_ptr->permission.uid))
                return(-1);

            obj_ptr->sect_z = atol(val);

            NetSendFObjectSect(-1, object_num);
        }
        /* X */
        else if(!strcmp(prop, "x"))
        {
            /* Check if object's permission allows set. */
            if(!ALLOW_SET_MAC(condescriptor, con_obj_ptr->permission.uid))
                return(-1);

            obj_ptr->x = atof(val);
	    NetSendObjectForcePose(-1, object_num);
        }
        /* Y */
        else if(!strcmp(prop, "y"))
        {
            /* Check if object's permission allows set. */
            if(!ALLOW_SET_MAC(condescriptor, con_obj_ptr->permission.uid))
                return(-1);

            obj_ptr->y = atof(val);
            NetSendObjectForcePose(-1, object_num);
        }
        /* Z */
        else if(!strcmp(prop, "z"))
        {
            /* Check if object's permission allows set. */
            if(!ALLOW_SET_MAC(condescriptor, con_obj_ptr->permission.uid))
                return(-1);

            obj_ptr->z = atof(val);
            NetSendObjectForcePose(-1, object_num);
        }
	/* Heading */
        else if(!strcmp(prop, "heading"))
        {
            /* Check if object's permission allows set. */
            if(!ALLOW_SET_MAC(condescriptor, con_obj_ptr->permission.uid))
                return(-1);

            obj_ptr->heading = SANITIZERADIANS(atof(val));
	    MuSetUnitVector2D(
                &obj_ptr->attitude_vector_compoent,
                obj_ptr->heading
            );
            NetSendObjectForcePose(-1, object_num);
        }
        /* Pitch */
        else if(!strcmp(prop, "pitch"))
        {
            /* Check if object's permission allows set. */
            if(!ALLOW_SET_MAC(condescriptor, con_obj_ptr->permission.uid))
                return(-1);

            obj_ptr->pitch = SANITIZERADIANS(atof(val));
            NetSendObjectForcePose(-1, object_num);
        }
        /* Bank */
        else if(!strcmp(prop, "bank"))
        {
            /* Check if object's permission allows set. */
            if(!ALLOW_SET_MAC(condescriptor, con_obj_ptr->permission.uid))
                return(-1);

            obj_ptr->bank = SANITIZERADIANS(atof(val));
            NetSendObjectForcePose(-1, object_num);
        }
        /* Velocity */
        else if(!strcmp(prop, "velocity"))
        {
            /* Check if object's permission allows set. */
            if(!ALLOW_SET_MAC(condescriptor, con_obj_ptr->permission.uid))
                return(-1);

            obj_ptr->velocity = atof(val);

            /* Sanitize. */
	    if(obj_ptr->velocity < 0)
		obj_ptr->velocity = 0;
        }
        /* Velocity maximum. */
        else if(!strcmp(prop, "velocity_max"))
        {
            /* Check if object's permission allows set. */
            if(!ALLOW_SET_MAC(condescriptor, con_obj_ptr->permission.uid))
                return(-1);

            obj_ptr->velocity_max = atof(val);

            /* Sanitize. */
            if(obj_ptr->velocity_max < 0)
                obj_ptr->velocity_max = 0;
        }
        /* Velocity heading. */
        else if(!strcmp(prop, "velocity_heading"))
        {
            /* Check if object's permission allows set. */
            if(!ALLOW_SET_MAC(condescriptor, con_obj_ptr->permission.uid))
                return(-1);

            obj_ptr->velocity_heading =	SANITIZERADIANS(atof(val));
            MuSetUnitVector2D(
                &obj_ptr->momentum_vector_compoent,
                obj_ptr->velocity_heading
            );
            NetSendObjectForcePose(-1, object_num);
        }
        /* Velocity pitch. */
        else if(!strcmp(prop, "velocity_pitch"))
        {
            /* Check if object's permission allows set. */
            if(!ALLOW_SET_MAC(condescriptor, con_obj_ptr->permission.uid))
                return(-1);

/* Need to add velocity_pitch set. */
        }
        /* Velocity bank. */
        else if(!strcmp(prop, "velocity_bank"))
        {
            /* Check if object's permission allows set. */
            if(!ALLOW_SET_MAC(condescriptor, con_obj_ptr->permission.uid))
                return(-1);
         
/* Need to add velocity_bank set. */
        }
        /* Thrust direction. */
        else if(!strcmp(prop, "thrust_dir"))
        {
            /* Check if object's permission allows set. */
            if(!ALLOW_SET_MAC(condescriptor, con_obj_ptr->permission.uid))
                return(-1);

            obj_ptr->thrust_dir = SANITIZERADIANS(atof(val));
        }
        /* Thrust. */
        else if(!strcmp(prop, "thrust"))
        {
            /* Check if object's permission allows set. */
            if(!ALLOW_SET_MAC(condescriptor, con_obj_ptr->permission.uid))
                return(-1);

            obj_ptr->thrust = atof(val);

            /* Sanitize. */
            if(obj_ptr->thrust < 0)
		obj_ptr->thrust = 0;
	}
        /* Thrust power. */
        else if(!strcmp(prop, "thrust_power"))
        {
            /* Check if object's permission allows set. */
            if(!ALLOW_SET_MAC(condescriptor, con_obj_ptr->permission.uid))
                return(-1);

            obj_ptr->thrust_power = atof(val);

            /* Sanitize. */
            if(obj_ptr->thrust_power < 0)
                obj_ptr->thrust_power = 0;
        }
        /* Throttle. */
        else if(!strcmp(prop, "throttle"))
        {
            /* Check if object's permission allows set. */
            if(!ALLOW_SET_MAC(condescriptor, con_obj_ptr->permission.uid))
                return(-1);

            obj_ptr->throttle = atof(val);

            /* Sanitize. */
	    if(obj_ptr->throttle > 1)
		obj_ptr->throttle = 1;
            else if(obj_ptr->throttle < 0)
                obj_ptr->throttle = 0;
        }
	/* Engine state. */
	else if(!strcmp(prop, "engine_state"))
	{
            /* Check if object's permission allows set. */
            if(!ALLOW_SET_MAC(condescriptor, con_obj_ptr->permission.uid))
                return(-1);

	    if(!strcasecmp(val, "on"))
		obj_ptr->engine_state = ENGINE_STATE_ON;
            else if(!strcasecmp(val, "start"))
                obj_ptr->engine_state = ENGINE_STATE_STARTING;
            else if(!strcasecmp(val, "starting"))
                obj_ptr->engine_state = ENGINE_STATE_STARTING;
            else if(!strcasecmp(val, "off"))
                obj_ptr->engine_state = ENGINE_STATE_OFF;
	    else
		obj_ptr->engine_state = ENGINE_STATE_NONE;
	}
        /* Turn rate. */
        else if(!strcmp(prop, "turnrate"))
        {
            /* Check if object's permission allows set. */
            if(!ALLOW_SET_MAC(condescriptor, con_obj_ptr->permission.uid))
                return(-1);

            obj_ptr->turnrate = atof(val);

            /* Sanitize. */
            if(obj_ptr->turnrate < 0) 
                obj_ptr->turnrate = 0;
        }
        /* Lighting. */
        else if(!strcmp(prop, "lighting"))
        {
            /* Check if object's permission allows set. */
            if(!ALLOW_SET_MAC(condescriptor, con_obj_ptr->permission.uid))
                return(-1);

	    if(val[0] == '!')
	    {
		/* Remove flag. */
                strptr = val;
                while((*strptr == '!') ||
                      ISBLANK(*strptr)
                )
                    strptr++;

	        if(!strcasecmp(strptr, XSW_OBJ_LT_NAME_VECTOR))
		    obj_ptr->lighting &= ~(XSW_OBJ_LT_VECTOR);
		else if(!strcasecmp(strptr, XSW_OBJ_LT_NAME_STROBE))
                    obj_ptr->lighting &= ~(XSW_OBJ_LT_STROBE);
                else if(!strcasecmp(strptr, XSW_OBJ_LT_NAME_LUMINATION))
                    obj_ptr->lighting &= ~(XSW_OBJ_LT_LUMINATION);
	    }
	    else
	    {
                /* Set flag. */
                strptr = val;

                if(!strcasecmp(strptr, XSW_OBJ_LT_NAME_VECTOR))
                    obj_ptr->lighting |= XSW_OBJ_LT_VECTOR;
                else if(!strcasecmp(strptr, XSW_OBJ_LT_NAME_STROBE))
                    obj_ptr->lighting |= XSW_OBJ_LT_STROBE;
                else if(!strcasecmp(strptr, XSW_OBJ_LT_NAME_LUMINATION))
                    obj_ptr->lighting |= XSW_OBJ_LT_LUMINATION;
            }
	    sprintf(val, "0x%.8x", obj_ptr->lighting);
        }
        /* Hit points. */
        else if(!strcmp(prop, "hp"))
        {
            /* Check if object's permission allows set. */
            if(!ALLOW_SET_MAC(condescriptor, con_obj_ptr->permission.uid))
                return(-1);

            obj_ptr->hp = atof(val);

            /* Sanitize. */
            if(obj_ptr->hp < 0)
                obj_ptr->hp = 0;
            if(obj_ptr->hp > obj_ptr->hp_max)
                obj_ptr->hp = obj_ptr->hp_max;
        }

        /* Hit points maximum. */
        else if(!strcmp(prop, "hp_max"))
        {
            /* Check if object's permission allows set. */
            if(!ALLOW_SET_MAC(condescriptor, con_obj_ptr->permission.uid))
                return(-1);

            obj_ptr->hp_max = atof(val);

            /* Sanitize. */
            if(obj_ptr->hp_max < 0)
                obj_ptr->hp_max = 0;
        }
        /* Power. */
        else if(!strcmp(prop, "power"))
        {
            /* Check if object's permission allows set. */
            if(!ALLOW_SET_MAC(condescriptor, con_obj_ptr->permission.uid))
                return(-1);

            obj_ptr->power = atof(val);

            /* Sanitize. */
            if(obj_ptr->power > obj_ptr->power_max) 
                obj_ptr->power = obj_ptr->power_max;
            if(obj_ptr->power < 0)
                obj_ptr->power = 0;
        }
        /* Power maximum. */
        else if(!strcmp(prop, "power_max"))
        {
            /* Check if object's permission allows set. */
            if(!ALLOW_SET_MAC(condescriptor, con_obj_ptr->permission.uid))
                return(-1);

            obj_ptr->power_max = atof(val);

            /* Sanitize. */
            if(obj_ptr->power_max < 0)
                obj_ptr->power_max = 0;
        }
        /* Power purity. */
        else if(!strcmp(prop, "power_purity"))
        {
            /* Check if object's permission allows set. */
            if(!ALLOW_SET_MAC(condescriptor, con_obj_ptr->permission.uid))
                return(-1);

            obj_ptr->power_purity = atof(val);

            /* Sanitize. */
            if(obj_ptr->power_purity > 1)
                obj_ptr->power_purity = 1;
            if(obj_ptr->power_purity < 0) 
                obj_ptr->power_purity = 0;
        }
        /* Core efficency. */
/* Sorry, major mispelling error. */
        else if(!strcmp(prop, "core_efficency") ||
                !strcmp(prop, "core_efficiency")
	)
        {
            /* Check if object's permission allows set. */
            if(!ALLOW_SET_MAC(condescriptor, con_obj_ptr->permission.uid))
                return(-1);

            obj_ptr->core_efficency = atof(val);

            /* Sanitize. */
            if(obj_ptr->core_efficency < 0)
                obj_ptr->core_efficency = 0;
        }
        /* Antimatter. */
        else if(!strcmp(prop, "antimatter"))
        {
            /* Check if object's permission allows set. */
            if(!ALLOW_SET_MAC(condescriptor, con_obj_ptr->permission.uid))
                return(-1);

            obj_ptr->antimatter = atof(val);

            /* Sanitize. */
            if(obj_ptr->antimatter > obj_ptr->antimatter_max) 
                obj_ptr->antimatter = obj_ptr->antimatter_max;
            if(obj_ptr->antimatter < 0)
                obj_ptr->antimatter = 0;
        }
        /* Antimatter maximum. */
        else if(!strcmp(prop, "antimatter_max"))
        {
            /* Check if object's permission allows set. */
            if(!ALLOW_SET_MAC(condescriptor, con_obj_ptr->permission.uid))
                return(-1);

            obj_ptr->antimatter_max = atof(val);

            /* Sanitize. */
            if(obj_ptr->antimatter_max < 0)
                obj_ptr->antimatter_max = 0;
        }
	/* Shield state. */
        else if(!strcmp(prop, "shield_state"))
        {
            /* Check if object's permission allows set. */
            if(!ALLOW_SET_MAC(condescriptor, con_obj_ptr->permission.uid))
                return(-1);

	    if(!strcasecmp(val, "up"))
		obj_ptr->shield_state = SHIELD_STATE_UP;
	    else if(!strcasecmp(val, "down"))
		obj_ptr->shield_state = SHIELD_STATE_DOWN;
	    else
	        obj_ptr->shield_state = SHIELD_STATE_NONE;
        }
        /* Shield frequency. */ 
        else if(!strcmp(prop, "shield_frequency"))
        {
            /* Check if object's permission allows set. */
            if(!ALLOW_SET_MAC(condescriptor, con_obj_ptr->permission.uid))
                return(-1);

            obj_ptr->shield_frequency = atof(val);

            /* Sanitize. */
            if(obj_ptr->shield_frequency > SWR_FREQ_MAX)
                obj_ptr->shield_frequency = SWR_FREQ_MAX;
	    else if(obj_ptr->shield_frequency < SWR_FREQ_MIN)
		obj_ptr->shield_frequency = SWR_FREQ_MIN;

	    /* Update argument value. */
	    sprintf(val, "%.4f", obj_ptr->shield_frequency);
        }
        /* Selected weapon. */
        else if(!strcmp(prop, "selected_weapon"))
        {
            /* Check if object's permission allows set. */
            if(!ALLOW_SET_MAC(condescriptor, con_obj_ptr->permission.uid))
                return(-1);

            obj_ptr->selected_weapon = atoi(val);

            /* Sanitize. */
            if(obj_ptr->selected_weapon >= obj_ptr->total_weapons)
                obj_ptr->selected_weapon = obj_ptr->total_weapons - 1;
        }
        /* Total weapons. */
        else if(!strcmp(prop, "total_weapons"))
        {
            /* Check if object's permission allows set. */
            if(!ALLOW_SET_MAC(condescriptor, con_obj_ptr->permission.uid))
                return(-1);

            /* Allocate more or less weapons on object and set
	     * new total.
	     */
	    UNVAllocObjectWeapons(
		obj_ptr,
		atoi(val)
	    );
        }
        /* Birth time. */
        else if(!strcmp(prop, "birth_time"))
        {
            /* Check if object's permission allows set. */
            if(!ALLOW_SET_MAC(condescriptor, con_obj_ptr->permission.uid))
                return(-1);

            obj_ptr->birth_time = atol(val);

            /* Sanitize. */
            if(obj_ptr->birth_time < 0)
                obj_ptr->birth_time = 0;
        }
        /* Life span. */
        else if(!strcmp(prop, "lifespan"))
        {
            /* Check if object's permission allows set. */
            if(!ALLOW_SET_MAC(condescriptor, con_obj_ptr->permission.uid))
                return(-1);

            obj_ptr->lifespan = atol(val);

            /* Sanitize. */
            if(obj_ptr->lifespan < -1)
                obj_ptr->lifespan = -1;
        }
        /* Cloak state. */
        else if(!strcmp(prop, "cloak_state"))
        {
            /* Check if object's permission allows set. */
            if(!ALLOW_SET_MAC(condescriptor, con_obj_ptr->permission.uid))
                return(-1);

            if(!strcasecmp(val, "up"))
                obj_ptr->cloak_state = CLOAK_STATE_UP;
            else if(!strcasecmp(val, "down"))
                obj_ptr->cloak_state = CLOAK_STATE_DOWN;
            else
                obj_ptr->cloak_state = CLOAK_STATE_NONE;
        }
        /* Cloak strength. */
        else if(!strcmp(prop, "cloak_strength"))
        {
            /* Check if object's permission allows set. */
            if(!ALLOW_SET_MAC(condescriptor, con_obj_ptr->permission.uid))
                return(-1);

            obj_ptr->cloak_strength = atof(val);

            /* Sanitize. */
            if(obj_ptr->cloak_strength < 0)
                obj_ptr->cloak_strength = 0;
        }
        /* Visibility. */
        else if(!strcmp(prop, "visibility"))
        {
            /* Check if object's permission allows set. */
            if(!ALLOW_SET_MAC(condescriptor, con_obj_ptr->permission.uid))
                return(-1);

            obj_ptr->visibility = atof(val);

            /* Sanitize. */
            if(obj_ptr->visibility < 0)
                obj_ptr->visibility = 0;
            else if(obj_ptr->visibility > 1) 
                obj_ptr->visibility = 1;
	}
        /* Shield visibility. */
        else if(!strcmp(prop, "shield_visibility"))
        {
            /* Check if object's permission allows set. */
            if(!ALLOW_SET_MAC(condescriptor, con_obj_ptr->permission.uid))
                return(-1);
    
            /* Set shield_visibility. */
            obj_ptr->shield_visibility = atof(val);

            /* Sanitize shield_visibility. */
            if(obj_ptr->shield_visibility < 0) 
                obj_ptr->shield_visibility = 0;
            else if(obj_ptr->shield_visibility > 1)
                obj_ptr->shield_visibility = 1;
        }
        /* Damage control. */
        else if(!strcmp(prop, "damage_control"))
        {
            /* Check if object's permission allows set. */
            if(!ALLOW_SET_MAC(condescriptor, con_obj_ptr->permission.uid))
                return(-1);

	    if(!strcasecmp(val, "on"))
		obj_ptr->damage_control = DMGCTL_STATE_ON;
	    else
		obj_ptr->damage_control = DMGCTL_STATE_OFF;
	}
        /* Communications channel. */
        else if(!strcmp(prop, "com_channel"))
        {
            /* Check if object's permission allows set. */
            if(!ALLOW_SET_MAC(condescriptor, con_obj_ptr->permission.uid))
                return(-1);

	    /* Com channel units are in real value * 100. */
            obj_ptr->com_channel = atoi(val);

	    if(((double)obj_ptr->com_channel / 100) > SWR_FREQ_MAX)
                obj_ptr->com_channel = static_cast<int>(SWR_FREQ_MAX * 100);
            else if(((double)obj_ptr->com_channel / 100) < SWR_FREQ_MIN)
                obj_ptr->com_channel = static_cast<int>(SWR_FREQ_MIN * 100);
        }
	/* AI (controlled objects) flags. */
	else if(!strcmp(prop, "ai_flags"))
	{
            /* Check if object's permission allows set. */ 
            if(!ALLOW_SET_MAC(condescriptor, con_obj_ptr->permission.uid))
                return(-1);

            if(val[0] == '!')
            {
                /* Remove flag. */
		strptr = val;
		while((*strptr == '!') ||
                      ISBLANK(*strptr)
		)
		    strptr++;

                if(!strcasecmp(strptr, XSW_OBJ_AI_NAME_FOLLOW_FRIEND))
                    obj_ptr->ai_flags &= ~(XSW_OBJ_AI_FOLLOW_FRIEND);
                else if(!strcasecmp(strptr, XSW_OBJ_AI_NAME_FOLLOW_UNKNOWN))
                    obj_ptr->ai_flags &= ~(XSW_OBJ_AI_FOLLOW_UNKNOWN);
                else if(!strcasecmp(strptr, XSW_OBJ_AI_NAME_FOLLOW_HOSTILE))
                    obj_ptr->ai_flags &= ~(XSW_OBJ_AI_FOLLOW_HOSTILE);
                else if(!strcasecmp(strptr, XSW_OBJ_AI_NAME_FIRE_FRIEND))
                    obj_ptr->ai_flags &= ~(XSW_OBJ_AI_FIRE_FRIEND);
                else if(!strcasecmp(strptr, XSW_OBJ_AI_NAME_FIRE_UNKNOWN))
                    obj_ptr->ai_flags &= ~(XSW_OBJ_AI_FIRE_UNKNOWN);
                else if(!strcasecmp(strptr, XSW_OBJ_AI_NAME_FIRE_HOSTILE))
                    obj_ptr->ai_flags &= ~(XSW_OBJ_AI_FIRE_HOSTILE);
            }
            else
            {
                /* Set flag. */
                strptr = val;

                if(!strcasecmp(strptr, XSW_OBJ_AI_NAME_FOLLOW_FRIEND))
                    obj_ptr->ai_flags |= XSW_OBJ_AI_FOLLOW_FRIEND;
                else if(!strcasecmp(strptr, XSW_OBJ_AI_NAME_FOLLOW_UNKNOWN))
                    obj_ptr->ai_flags |= XSW_OBJ_AI_FOLLOW_UNKNOWN;
                else if(!strcasecmp(strptr, XSW_OBJ_AI_NAME_FOLLOW_HOSTILE))
                    obj_ptr->ai_flags |= XSW_OBJ_AI_FOLLOW_HOSTILE;
                else if(!strcasecmp(strptr, XSW_OBJ_AI_NAME_FIRE_FRIEND))
                    obj_ptr->ai_flags |= XSW_OBJ_AI_FIRE_FRIEND;
                else if(!strcasecmp(strptr, XSW_OBJ_AI_NAME_FIRE_UNKNOWN))
                    obj_ptr->ai_flags |= XSW_OBJ_AI_FIRE_UNKNOWN;
                else if(!strcasecmp(strptr, XSW_OBJ_AI_NAME_FIRE_HOSTILE))
                    obj_ptr->ai_flags |= XSW_OBJ_AI_FIRE_HOSTILE;

	    }
            sprintf(val, "0x%.8x", (unsigned int)obj_ptr->ai_flags);
	}
        /* ************************************************************* */
        /* Animation interval. */
        else if(!strcmp(prop, "animation.interval") ||
                !strcmp(prop, "animation/interval")
        )
        {
            /* Check if object's permission allows set. */
            if(!ALLOW_SET_MAC(condescriptor, con_obj_ptr->permission.uid))
                return(-1);

            obj_ptr->animation.interval = atol(val);

            /* Sanitize. */
            if(obj_ptr->animation.interval < 0) 
                obj_ptr->animation.interval = 0;
        }
        /* Animation last interval. */
        else if(!strcmp(prop, "animation.last_interval") ||
                !strcmp(prop, "animation/last_interval")   
        )
        {
            /* Check if object's permission allows set. */
            if(!ALLOW_SET_MAC(condescriptor, con_obj_ptr->permission.uid))
                return(-1);
                
            obj_ptr->animation.last_interval = atol(val);
        
            /* Sanitize. */
            if(obj_ptr->animation.last_interval < 0)
                obj_ptr->animation.last_interval = 0;
        }
        /* Animation current frame. */
        else if(!strcmp(prop, "animation.current_frame") ||
                !strcmp(prop, "animation/current_frame")
        )
        {
            /* Check if object's permission allows set. */
            if(!ALLOW_SET_MAC(condescriptor, con_obj_ptr->permission.uid))
                return(-1);
            
            obj_ptr->animation.current_frame = atoi(val);
                
            /* Sanitize. */
            if(obj_ptr->animation.current_frame >= obj_ptr->animation.total_frames)
                obj_ptr->animation.current_frame = obj_ptr->animation.total_frames - 1;
            if(obj_ptr->animation.current_frame < 0)
                obj_ptr->animation.current_frame = 0;
        }
        /* Animation total frames. */
        else if(!strcmp(prop, "animation.total_frames") ||
                !strcmp(prop, "animation/total_frames")
        )
        {
            /* Check if object's permission allows set. */
            if(!ALLOW_SET_MAC(condescriptor, con_obj_ptr->permission.uid))
                return(-1);

            obj_ptr->animation.total_frames = atoi(val);

            /* Sanitize. */
            if(obj_ptr->animation.total_frames < 1)
                obj_ptr->animation.total_frames = 1;
        }
        /* Animation cycle count. */
        else if(!strcmp(prop, "animation.cycle_count") ||
                !strcmp(prop, "animation/cycle_count") 
        )
        {
            /* Check if object's permission allows set. */
            if(!ALLOW_SET_MAC(condescriptor, con_obj_ptr->permission.uid))
                return(-1);

            obj_ptr->animation.cycle_count = atoi(val);

            /* Sanitize. */
            if(obj_ptr->animation.cycle_count < 0)
                obj_ptr->animation.cycle_count = 0;
        }
        /* Animation cycle times. */
        else if(!strcmp(prop, "animation.cycle_times") ||
                !strcmp(prop, "animation/cycle_times") 
        )
        {
            /* Check if object's permission allows set. */
            if(!ALLOW_SET_MAC(condescriptor, con_obj_ptr->permission.uid))
                return(-1);

            obj_ptr->animation.cycle_times = atoi(val);

            /* Sanitize. */
            if(obj_ptr->animation.cycle_times < -1)
                obj_ptr->animation.cycle_times = -1;
        }
	/* ************************************************************* */
        /* Credits. */
        else if(!strcmp(prop, "score.credits") ||
                !strcmp(prop, "score/credits")
	)
        {
            /* Check if object's permission allows set. */
            if(!ALLOW_SET_MAC(condescriptor, con_obj_ptr->permission.uid))
                return(-1);

	    if(!UNVAllocScores(obj_ptr))
	    {
                obj_ptr->score->credits = atof(val);

                /* Sanitize. */
                if(obj_ptr->score->credits < 0)
                    obj_ptr->score->credits = 0;
	    }
        }
        /* RMU. */
        else if(!strcmp(prop, "score.rmu") ||
	        !strcmp(prop, "score/rmu")
	)
        {
            /* Check if object's permission allows set. */
            if(!ALLOW_SET_MAC(condescriptor, con_obj_ptr->permission.uid))
                return(-1);

            if(!UNVAllocScores(obj_ptr))
            {
                obj_ptr->score->rmu = atof(val);
            
                /* Sanitize. */ 
                if(obj_ptr->score->rmu < 0)
                    obj_ptr->score->rmu = 0;
		if(obj_ptr->score->rmu > obj_ptr->score->rmu_max)
		    obj_ptr->score->rmu = obj_ptr->score->rmu_max;
            }
        }
        /* RMU Maximum. */
        else if(!strcmp(prop, "score.rmu_max") ||
                !strcmp(prop, "score/rmu_max")
	)
        {
            /* Check if object's permission allows set. */
            if(!ALLOW_SET_MAC(condescriptor, con_obj_ptr->permission.uid))
                return(-1);

            if(!UNVAllocScores(obj_ptr))
            {
                obj_ptr->score->rmu_max = atof(val);

                /* Sanitize. */
                if(obj_ptr->score->rmu_max < 0)
                    obj_ptr->score->rmu_max = 0;
            }
        }
        /* Damage given. */
        else if(!strcmp(prop, "score.damage_given") ||
                !strcmp(prop, "score/damage_given")
        )
        {
            /* Check if object's permission allows set. */
            if(!ALLOW_SET_MAC(condescriptor, con_obj_ptr->permission.uid))
                return(-1);

            if(!UNVAllocScores(obj_ptr))
            {
                obj_ptr->score->damage_given = atof(val);

                /* Sanitize. */
                if(obj_ptr->score->damage_given < 0) 
                    obj_ptr->score->damage_given = 0;
            }
        }
        /* Damage recieved. */
        else if(!strcmp(prop, "score.damage_recieved") ||
                !strcmp(prop, "score/damage_recieved")
	)
        {
            /* Check if object's permission allows set. */
            if(!ALLOW_SET_MAC(condescriptor, con_obj_ptr->permission.uid))
                return(-1);

            if(!UNVAllocScores(obj_ptr))
            {
                obj_ptr->score->damage_recieved = atof(val);

                /* Sanitize. */
                if(obj_ptr->score->damage_recieved < 0)
                    obj_ptr->score->damage_recieved = 0;
            }       
        }
        /* Kills. */
        else if(!strcmp(prop, "score.kills") ||
	        !strcmp(prop, "score/kills")
	)
        {
            /* Check if object's permission allows set. */
            if(!ALLOW_SET_MAC(condescriptor, con_obj_ptr->permission.uid))
                return(-1);

            if(!UNVAllocScores(obj_ptr))
            {
                obj_ptr->score->kills = atoi(val);

                /* Sanitize. */
                if(obj_ptr->score->kills < 0)
                    obj_ptr->score->kills = 0;
            }
        }
	/* ******************************************************* */
	/* Permissions. */

        /* UID. */
	else if(!strcmp(prop, "permission.uid") ||
                !strcmp(prop, "permission/uid")
	)
        {
            /* Check if object's permission allows set. */
            if(!ALLOW_SET_MAC(condescriptor, con_obj_ptr->permission.uid))
                return(-1);

	    /* Cannot set UID permissions on yourself. */
            if(con_ptr->object_num == object_num)
            {
                sprintf(sndbuf,
                    "%s: Cannot set UID permissions on yourself.",
		    THIS_CMD_NAME
                );
                NetSendLiveMessage(condescriptor, sndbuf);
                return(-1);
            }

	    /* Check if permission is equal or lower than yours. */
            if(con_obj_ptr->permission.uid > atoi(val))
	    {
                sprintf(sndbuf,
                    "%s: Permission denied: Requires access level %i.",
		    THIS_CMD_NAME,
                    atoi(val)
                );
                NetSendLiveMessage(condescriptor, sndbuf);
                return(-1);
	    }

            /* Set permission.uid. */
            obj_ptr->permission.uid = atoi(val);

            /* Sanitize permission.uid. */
            if(obj_ptr->permission.uid < 0)
                obj_ptr->permission.uid = 0;
        }
        /* GID. */
        else if( !strcmp(prop, "permission.gid") ||
                 !strcmp(prop, "permission/gid")
	)
        {
            /* Check if object's permission allows set. */
            if(!ALLOW_SET_MAC(condescriptor, con_obj_ptr->permission.uid))
                return(-1);

            /* Check if permission is equal or lower than yours. */
            if(con_obj_ptr->permission.uid > atoi(val))
            {
                sprintf(sndbuf,
                    "%s: Permission denied: Requires access level %i.",
		    THIS_CMD_NAME,
                    atoi(val)
                );
                NetSendLiveMessage(condescriptor, sndbuf);
                return(-1);
            }

            obj_ptr->permission.gid = atoi(val);

            /* Sanitize. */
            if(obj_ptr->permission.gid < 0)
                obj_ptr->permission.gid = 0;
        }
	/* ********************************************************* */
	/* Weapons. */
        else if(strpfx(prop, "weapon/") ||
                strpfx(prop, "weapon.")
	)
        {
	    xsw_weapons_struct *wep_ptr;


            /* Check if object's permission allows set. */
            if(!ALLOW_SET_MAC(condescriptor, con_obj_ptr->permission.uid))
                return(-1);

	    /* Parse prop and get weapon number. */
	    strptr = strchr(prop, '.');
	    if(strptr == NULL)
		strptr = strchr(prop, '/');

	    if(strptr != NULL)
	    {
		strncpy(stringa, strptr + 1, CS_DATA_MAX_LEN);
		stringa[CS_DATA_MAX_LEN - 1] = '\0';

		strptr = strchr(stringa, '.');
                if(strptr == NULL)
                    strptr = strchr(stringa, '/');

		if(strptr != NULL)
		    *strptr = '\0';
		StringStripSpaces(stringa);
		wep_num = atoi(stringa);
	    }
	    else
	    {
                sprintf(sndbuf,
         "%s: Cannot parse incomplete property `%s'.",
		    THIS_CMD_NAME,
                    prop
                );
                NetSendLiveMessage(condescriptor, sndbuf);
                return(-1);
	    }

	    /* Check if weapon number exists as an allocated weapon. */
	    if((wep_num < 0) || (wep_num >= obj_ptr->total_weapons))
	    {
                sprintf(sndbuf,
      "%s: Weapon %i: Not allocated, set total_weapons first.",
		    THIS_CMD_NAME,
                    wep_num
                );
                NetSendLiveMessage(condescriptor, sndbuf);
                return(-1);
            }
	    wep_ptr = obj_ptr->weapons[wep_num];
            if(wep_ptr == NULL)
            {
                sprintf(sndbuf,
      "%s: Weapon %i: Not allocated, set total_weapons first.",
		    THIS_CMD_NAME,
                    wep_num
                );
                NetSendLiveMessage(condescriptor, sndbuf);
                return(-1);
            }

	    /* Change prop to contain just the weapon's member name. */
            strptr = strchr(prop, '.');
            if(strptr == NULL)
                strptr = strchr(prop, '/'); 

            if(strptr != NULL)
            {
		strncpy(stringa, strptr + 1, CS_DATA_MAX_LEN);
		stringa[CS_DATA_MAX_LEN - 1] = '\0';

		strptr = strchr(stringa, '.');
                if(strptr == NULL)
                    strptr = strchr(stringa, '/'); 

		if(strptr != NULL)
		{
            	    strncpy(stringb, strptr + 1, CS_DATA_MAX_LEN);
		    stringb[CS_DATA_MAX_LEN - 1] = '\0';

		    strncpy(prop, stringb, CS_DATA_MAX_LEN);
		    StringStripSpaces(prop);
		}
		else
		{
		    /* Default to amount. */
		    strncpy(prop, "amount", CS_DATA_MAX_LEN);
		    prop[CS_DATA_MAX_LEN - 1] = '\0';
		}
	    }
	    else
	    {
		/* Default to amount. */
		strncpy(prop, "amount", CS_DATA_MAX_LEN);
		prop[CS_DATA_MAX_LEN - 1] = '\0';
	    }


	    /* Flags. */
	    if(!strcasecmp(prop, "flags"))
	    {
                if(val[0] == '!')
                {
                    /* Remove flag. */
                    strptr = val;
                    while((*strptr == '!') ||
                          ISBLANK(*strptr)
                    )
                        strptr++;

                    if(!strcasecmp(strptr, XSW_WEP_FLAG_NAME_NO_FIRE_SOUND))
                        wep_ptr->flags &= ~(XSW_WEP_FLAG_NO_FIRE_SOUND);
                    else if(!strcasecmp(strptr, XSW_WEP_FLAG_NAME_FIXED))
                        wep_ptr->flags &= ~(XSW_WEP_FLAG_FIXED);
                }
                else
	        {
                    /* Set flag. */
                    strptr = val;

                    if(!strcasecmp(strptr, XSW_WEP_FLAG_NAME_NO_FIRE_SOUND))
                        wep_ptr->flags |= XSW_WEP_FLAG_NO_FIRE_SOUND;
                    else if(!strcasecmp(strptr, XSW_WEP_FLAG_NAME_FIXED))
                        wep_ptr->flags |= XSW_WEP_FLAG_FIXED;
		}
		sprintf(val, "0x%.8x", (unsigned int)wep_ptr->flags);
	    }
	    /* Type. */
	    else if(!strcasecmp(prop, "ocs_code") ||
                    !strcasecmp(prop, "ocs_type") ||
                    !strcasecmp(prop, "ocs")
	    )
	    {
		wep_ptr->ocs_code = atoi(val);

		if(wep_ptr->ocs_code < 0)
		    wep_ptr->ocs_code = 0;
	    }
            /* Emission type. */
            else if(!strcasecmp(prop, "emission_type"))
            {
		if(!strcasecmp(val, "STREAM"))
		    wep_ptr->emission_type = WEPEMISSION_STREAM;
                else if(!strcasecmp(val, "PROJECTILE"))
                    wep_ptr->emission_type = WEPEMISSION_PROJECTILE;
                else if(!strcasecmp(val, "PULSE"))
                    wep_ptr->emission_type = WEPEMISSION_PULSE;
            }
            /* Amount. */
            else if(!strcasecmp(prop, "amount"))
            {
                wep_ptr->amount = atoi(val);

		if(wep_ptr->amount > wep_ptr->max)
                   wep_ptr->amount = wep_ptr->max;
                if(wep_ptr->amount < 0)
                    wep_ptr->amount = 0;
            }
            /* Max. */
            else if(!strcasecmp(prop, "max"))
            {
                wep_ptr->max = atoi(val);

                if(wep_ptr->max < 0)
                    wep_ptr->max = 0;
            }
            /* Power. */
            else if(!strcasecmp(prop, "power"))
            {
                wep_ptr->power = atof(val);

                if(wep_ptr->power < 0)
                    wep_ptr->power = 0;
            }
            /* Range. */
            else if(!strcasecmp(prop, "range"))
            {
                wep_ptr->range = atol(val);	/* In screen units. */

                if(wep_ptr->range < 0)
                    wep_ptr->range = 0;
            }
            /* Create power. */
            else if(!strcasecmp(prop, "create_power"))
            {
                wep_ptr->create_power = atof(val);

                if(wep_ptr->create_power < 0)
		    wep_ptr->create_power = 0;
            }
            /* Delay. */
            else if(!strcasecmp(prop, "delay"))
            {
                wep_ptr->delay = atol(val);	/* Milliseconds. */

                if(wep_ptr->delay < 1)
                    wep_ptr->delay = 1;
            }
            /* Last used. */
            else if(!strcasecmp(prop, "last_used"))
            {
                wep_ptr->last_used = atol(val);	/* Milliseconds. */

                if(wep_ptr->last_used < 0)
                    wep_ptr->last_used = 0;
            }
            /* Fire sound code. */
            else if(!strcasecmp(prop, "fire_sound_code"))
            {
                wep_ptr->fire_sound_code = atoi(val);

                if(wep_ptr->fire_sound_code < 0)
                    wep_ptr->fire_sound_code = 0;
            }

	    /* Schedual need to update weapon values. */
	    next.need_weapon_values = 1;
        }
        /* *********************************************************** */
	/* Economy: Flags. */
        else if(!strcmp(prop, "eco.flags") ||
                !strcmp(prop, "eco/flags")
        )
        {
            /* Check if object's permission allows set. */
            if(!ALLOW_SET_MAC(condescriptor, con_obj_ptr->permission.uid))
                return(-1);

	    if(UNVAllocEco(obj_ptr))
		return(-1);
	    else
		eco_ptr = obj_ptr->eco;

            if(val[0] == '!')
            {
                /* Remove flag. */
                strptr = val;
                while((*strptr == '!') ||
                      ISBLANK(*strptr)
                )
                    strptr++;

                if(!strcasecmp(strptr, ECO_FLAG_NAME_OPEN))
                    eco_ptr->flags &= ~(ECO_FLAG_OPEN);
                else if(!strcasecmp(strptr, ECO_FLAG_NAME_BUY_OK))
                    eco_ptr->flags &= ~(ECO_FLAG_BUY_OK);
                else if(!strcasecmp(strptr, ECO_FLAG_NAME_SELL_OK))
                    eco_ptr->flags &= ~(ECO_FLAG_SELL_OK);
                else if(!strcasecmp(strptr, ECO_FLAG_NAME_TRADE_OK))
                    eco_ptr->flags &= ~(ECO_FLAG_TRADE_OK);
                else if(!strcasecmp(strptr, ECO_FLAG_NAME_INTRODUCE_OK))
                    eco_ptr->flags &= ~(ECO_FLAG_INTRODUCE_OK);
            }
            else
            {
                /* Set flag. */
                strptr = val;

                if(!strcasecmp(strptr, ECO_FLAG_NAME_OPEN))
                    eco_ptr->flags |= ECO_FLAG_OPEN;
                else if(!strcasecmp(strptr, ECO_FLAG_NAME_BUY_OK))
                    eco_ptr->flags |= ECO_FLAG_BUY_OK;
                else if(!strcasecmp(strptr, ECO_FLAG_NAME_SELL_OK))
                    eco_ptr->flags |= ECO_FLAG_SELL_OK;
                else if(!strcasecmp(strptr, ECO_FLAG_NAME_TRADE_OK))
                    eco_ptr->flags |= ECO_FLAG_TRADE_OK;
                else if(!strcasecmp(strptr, ECO_FLAG_NAME_INTRODUCE_OK))
                    eco_ptr->flags |= ECO_FLAG_INTRODUCE_OK;
            }
            sprintf(val, "0x%.8lx", eco_ptr->flags);

	    NetSendEcoSetValues(condescriptor, object_num);
	}
	/* Economy: Tax general. */
        else if(!strcmp(prop, "eco.tax_general") ||
                !strcmp(prop, "eco/tax_general")
        )
        {
            /* Check if object's permission allows set. */
            if(!ALLOW_SET_MAC(condescriptor, con_obj_ptr->permission.uid))
                return(-1);

            if(UNVAllocEco(obj_ptr))
                return(-1);
            else
                eco_ptr = obj_ptr->eco;


	    eco_ptr->tax_general = atof(val);
	    if(eco_ptr->tax_general < 0.0)
		eco_ptr->tax_general = 0.0;

            NetSendEcoSetValues(condescriptor, object_num);
        }
	/* Economy: Tax friend. */
        else if(!strcmp(prop, "eco.tax_friend") ||
                !strcmp(prop, "eco/tax_friend")
        )
        {
            /* Check if object's permission allows set. */
            if(!ALLOW_SET_MAC(condescriptor, con_obj_ptr->permission.uid))
                return(-1);

            if(UNVAllocEco(obj_ptr))
                return(-1);
            else
                eco_ptr = obj_ptr->eco;


            eco_ptr->tax_friend = atof(val);
            if(eco_ptr->tax_friend < 0.0)
                eco_ptr->tax_friend = 0.0;

            NetSendEcoSetValues(condescriptor, object_num);
        }
	/* Economy: Tax hostile. */
        else if(!strcmp(prop, "eco.tax_hostile") ||
                !strcmp(prop, "eco/tax_hostile")
        )   
        {
            /* Check if object's permission allows set. */
            if(!ALLOW_SET_MAC(condescriptor, con_obj_ptr->permission.uid))
                return(-1);

            if(UNVAllocEco(obj_ptr))
                return(-1);
            else
                eco_ptr = obj_ptr->eco;


            eco_ptr->tax_hostile = atof(val);
            if(eco_ptr->tax_hostile < 0.0)
                eco_ptr->tax_hostile = 0.0;

            NetSendEcoSetValues(condescriptor, object_num);
        }



	/* Unknown property. */
	else
	{
            sprintf(sndbuf,
		"%s: Unsupported property: `%s'",
		THIS_CMD_NAME,
                prop
	    );
            NetSendLiveMessage(condescriptor, sndbuf);
	    return(-1);
	}



	/* ********************************************************** */
	/* Print response and log. */

        strncpy(
            tmp_name1, 
            DBGetFormalNameStr(
                con_ptr->object_num
            ),
            XSW_OBJ_NAME_MAX
        );
        tmp_name1[XSW_OBJ_NAME_MAX - 1] = '\0';
        
        strncpy(
            tmp_name2,
            DBGetFormalNameStr(object_num),
            XSW_OBJ_NAME_MAX
        );
        tmp_name2[XSW_OBJ_NAME_MAX - 1] = '\0';   
 

        sprintf(sndbuf,
	    "%s: %s: Property `%s' to value `%s'.",
	    THIS_CMD_NAME,
            tmp_name2,
            prop,
            val
        );
        NetSendLiveMessage(condescriptor, sndbuf);


        /* Log property setting. */
        sprintf(stringa, "%s: Set %s: Property: `%s'  Value: `%s'",
                tmp_name1,
		tmp_name2,
                prop,
                val
        );
	if(sysparm.log_general == 1)
            LogAppendLineFormatted(fname.primary_log, stringa);


	return(0);
}



int CmdSet(int condescriptor, const char *arg)
{
	char stringa[CS_DATA_MAX_LEN];
	char sndbuf[CS_DATA_MAX_LEN];


	/* Copy arg to stringa. */
	strncpy(stringa, arg, CS_DATA_MAX_LEN);
	stringa[CS_DATA_MAX_LEN - 1] = '\0';
	StringStripSpaces(stringa);


        /* Print usage? */
        if((stringa[0] == '\0') ||
	   ((strchr(stringa, '=')) == NULL)
	)
	{
            sprintf(
		sndbuf,
		"Usage: `%s <object>=<property>:<value>'",
		THIS_CMD_NAME
	    );
            NetSendLiveMessage(condescriptor, sndbuf);
	    return(-1);
	}


	/* OPM set? */
	if(strchr(stringa, ':') == NULL)
	{
	    return(CmdOPMSet(condescriptor, stringa));
	}
	else
	{
	    return(CmdDoSet(condescriptor, stringa));
	}


	return(-1);
}
