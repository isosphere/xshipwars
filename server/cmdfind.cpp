#include <fnmatch.h>

#include "../include/string.h"
#include "../include/strexp.h"

#include "../include/unvmatch.h"
#include "../include/unvmath.h"
#include "../include/unvutil.h"

#include "swserv.h"
#include "net.h"


#define THIS_CMD_NAME	"find"


static int CmdFindMatch(const char *pattern, const char *string);


/*
 *	Matches string with pattern, using fnmatch().
 */
static int CmdFindMatch(const char *pattern, const char *string)
{
	int status;
	char *tmp_pattern, *tmp_string;


	tmp_pattern = StringCopyAlloc(pattern);
	tmp_string = StringCopyAlloc(string);

	strtoupper(tmp_pattern);
	strtoupper(tmp_string);

	if((tmp_pattern != NULL) &&
	   (tmp_string != NULL)
	)
	    status = fnmatch(tmp_pattern, tmp_string, 0);
	else
	    status = -1;

	free(tmp_pattern);
	free(tmp_string);

	return(status);
}


int CmdFind(int condescriptor, const char *arg)
{
	int len, status;

	char larg[CS_DATA_MAX_LEN];
	char pattern[CS_DATA_MAX_LEN];
	char type_str[CS_DATA_MAX_LEN];

	char **argv;
	int argc;

        const int delta_str_len = 256;
        char delta_str[delta_str_len];

	const int position_str_len = 256;
        char position_str[position_str_len];

	char *type_strptr = "";
	char *units_strptr = "";

        char name1[XSW_OBJ_NAME_MAX + 80];
        char name2[XSW_OBJ_NAME_MAX + 80];

        int object_num, objects_found, con_obj_num;
        xsw_object_struct *con_obj_ptr, *obj_ptr, **ptr;
	connection_struct *con_ptr;
	char sndbuf[CS_DATA_MAX_LEN + delta_str_len + position_str_len];


	/* Get connection's object number (assumed valid). */
        con_ptr = connection[condescriptor];
        con_obj_num = con_ptr->object_num;
        con_obj_ptr = xsw_object[con_obj_num];

	/* Print usage? */
	if((arg == NULL) ? 1 : (*arg == '\0'))
        {
            NetSendLiveMessage(
		condescriptor,
                "Usage: `find <name_pattern>[=<type>]'"
            );
            return(-1);
        }


	/* Copy arg to local argument. */
        strncpy(larg, arg, CS_DATA_MAX_LEN);
        larg[CS_DATA_MAX_LEN - 1] = '\0';   
        StringStripSpaces(larg);


        /* Check if connection object is allowed to find? */
        if(con_obj_ptr->permission.uid > ACCESS_UID_FIND)
        {
            sprintf(sndbuf,
                "%s: Permission denied: Requires access level %i.",
		THIS_CMD_NAME,
                ACCESS_UID_FIND
            );
            NetSendLiveMessage(condescriptor, sndbuf);

            return(-1);
        }


        /* Parse and get pattern and type_str. */
	argv = strchrexp(larg, '=', &argc);

	strncpy(
	    pattern,
	    ((argc >= 1) ? argv[0] : "*"),
	    CS_DATA_MAX_LEN
	);
	pattern[CS_DATA_MAX_LEN - 1] = '\0';
	StringStripSpaces(pattern);

	strncpy(
            type_str,
            ((argc >= 2) ? argv[1] : "*"),
            CS_DATA_MAX_LEN
        );
        type_str[CS_DATA_MAX_LEN - 1] = '\0';
	StringStripSpaces(type_str);


	StringFreeArray(argv, argc);


        /* Go through XSW objects list. */
        for(object_num = 0, objects_found = 0, ptr = xsw_object;
            object_num < total_objects;
            object_num++, ptr++
        )
        {
	    obj_ptr = *ptr;
            if(obj_ptr == NULL)
                continue;
            if(obj_ptr->type <= XSW_OBJ_TYPE_GARBAGE)
		continue;

            /* Allowed to match others? */
            if((obj_ptr->owner != con_obj_num) &&
                (con_obj_ptr->permission.uid > ACCESS_UID_FINDO)
            )
                continue;


	    /* Type match? */
	    switch(obj_ptr->type)
	    {
	      case XSW_OBJ_TYPE_STATIC:
                type_strptr = XSW_TYPE_NAME_STATIC;
		if(CmdFindMatch(type_str, type_strptr))
		    status = -1;
		else
		    status = 0;
		break;

              case XSW_OBJ_TYPE_DYNAMIC:
		type_strptr = XSW_TYPE_NAME_DYNAMIC;
                if(CmdFindMatch(type_str, type_strptr))
                    status = -1;
                else
                    status = 0;
                break;

              case XSW_OBJ_TYPE_CONTROLLED:
		type_strptr = XSW_TYPE_NAME_CONTROLLED;
                if(CmdFindMatch(type_str, type_strptr))
                    status = -1;
                else
                    status = 0;
                break;

              case XSW_OBJ_TYPE_PLAYER:
		type_strptr = XSW_TYPE_NAME_PLAYER;
                if(CmdFindMatch(type_str, type_strptr))
                    status = -1;
                else
                    status = 0;
                break;

              case XSW_OBJ_TYPE_WEAPON:
		type_strptr = XSW_TYPE_NAME_WEAPON;
                if(CmdFindMatch(type_str, type_strptr))
                    status = -1;
                else
                    status = 0;
                break;

              case XSW_OBJ_TYPE_STREAMWEAPON:
		type_strptr = XSW_TYPE_NAME_STREAMWEAPON;
                if(CmdFindMatch(type_str, type_strptr))
                    status = -1;
                else
                    status = 0;
                break;

              case XSW_OBJ_TYPE_SPHEREWEAPON:
		type_strptr = XSW_TYPE_NAME_SPHEREWEAPON;
                if(CmdFindMatch(type_str, type_strptr))
                    status = -1;
                else
                    status = 0;
                break;

              case XSW_OBJ_TYPE_HOME:
		type_strptr = XSW_TYPE_NAME_HOME;
                if(CmdFindMatch(type_str, type_strptr))
                    status = -1;
                else
                    status = 0;
                break;

              case XSW_OBJ_TYPE_AREA:
		type_strptr = XSW_TYPE_NAME_AREA;
                if(CmdFindMatch(type_str, type_strptr))
                    status = -1;
                else
                    status = 0;
                break;

              case XSW_OBJ_TYPE_ANIMATED:
		type_strptr = XSW_TYPE_NAME_ANIMATED;
                if(CmdFindMatch(type_str, type_strptr))
                    status = -1;
                else
                    status = 0;
                break;

              case XSW_OBJ_TYPE_WORMHOLE:
                type_strptr = XSW_TYPE_NAME_WORMHOLE;
                if(CmdFindMatch(type_str, type_strptr))
                    status = -1;
                else
                    status = 0;
                break;

              case XSW_OBJ_TYPE_ELINK:
                type_strptr = XSW_TYPE_NAME_ELINK;
                if(CmdFindMatch(type_str, type_strptr))
                    status = -1;
                else
                    status = 0;
                break;

              default:
		status = 0;
                break;
	    }
	    if(status)
		continue;


	    /* Name match? */
	    if(fnmatch(pattern, obj_ptr->name, 0))
		continue;


	    /* Consider this object found. */
            objects_found++;

	    /* First object found? */
	    if(objects_found == 1)
	    {
		/* Print heading for the first time. */
                NetSendLiveMessage(
		    condescriptor,
"Name                     Owner                    Location"
		);
	    }

            /* Format delta string. */
            if(Mu3DInSectorPtr(
		obj_ptr,
                con_obj_ptr->sect_x,
                con_obj_ptr->sect_y,
                con_obj_ptr->sect_z
	    ))
            {
		double dx = obj_ptr->x - con_obj_ptr->x;
		double dy = obj_ptr->y - con_obj_ptr->y;

		units_strptr = "RU";

                sprintf(
                    delta_str,
                    "Brg: %.2f' D: %.2f %s",
                    RADTODEG(MuCoordinateDeltaVector(dx, dy)),
                    Mu3DDistance(dx, dy, 0),
                    units_strptr
                );
	    }
	    else
	    {
		double dx = obj_ptr->sect_x - con_obj_ptr->sect_x;
		double dy = obj_ptr->sect_y - con_obj_ptr->sect_y;

                units_strptr = "S";

		sprintf(
		    delta_str,
		    "Brg: %.2f' D: %.2f %s",
		    RADTODEG(MuCoordinateDeltaVector(dx, dy)),
		    Mu3DDistance(dx, dy, 0),
		    units_strptr
		);
	    }

	    /* Format position string. */
	    UNVLocationFormatString(
		position_str,
		&obj_ptr->sect_x, &obj_ptr->sect_y, &obj_ptr->sect_z,
		&obj_ptr->x, &obj_ptr->y, &obj_ptr->z,
		position_str_len
	    );

	    /* Format names. */
            strncpy(
		name1,
		DBGetFormalNameStr(object_num),
		XSW_OBJ_NAME_MAX + 80
	    );
            name1[XSW_OBJ_NAME_MAX + 80 - 1] = '\0';
            strncpy(
		name2,
		DBGetFormalNameStr(obj_ptr->owner),
		XSW_OBJ_NAME_MAX + 80
	    );
            name2[XSW_OBJ_NAME_MAX + 80 - 1] = '\0';

	    strcpy(sndbuf, name1);
	    len = strlen(sndbuf);
	    strpad(&sndbuf[len], 25 - len);

	    strcpy(&sndbuf[25], name2);
	    len = strlen(sndbuf);
	    strpad(&sndbuf[len], 50 - len);

            strcpy(&sndbuf[50], delta_str);
            len = strlen(sndbuf);
            strpad(&sndbuf[len], 78 - len);

            strcpy(&sndbuf[78], position_str);

            NetSendLiveMessage(condescriptor, sndbuf);
        }

        /* Print number of objects found. */
	if(objects_found > 0)
	    sprintf(sndbuf, "*** %i Object%s Found ***",
                objects_found,
                ((objects_found == 1) ? "" : "s")
            );
	else
	    sprintf(sndbuf, "*** No Objects Found ***");
        NetSendLiveMessage(condescriptor, sndbuf);


        return(0);
}
