#include "../include/unvmatch.h"
#include "../include/xsw_ctype.h"
#include "swserv.h"
#include "net.h"


#define THIS_CMD_NAME	"recycle"


/*
 *	Recycle all weapons.
 *
 *	All permission checks will be tested and handled accordingly.
 */
static int CmdRecycleWeapons(
	int condescriptor,
        int con_obj_num,
        xsw_object_struct *con_obj_ptr
)
{
        int i;
        xsw_object_struct *obj_ptr; 
        char name1[XSW_OBJ_NAME_MAX + 80];
        char name2[XSW_OBJ_NAME_MAX + 80];
        char sndbuf[CS_DATA_MAX_LEN];
        char text[(2 * XSW_OBJ_NAME_MAX) + 256];


        /* Record connection object's name. */
        strncpy(
            name1,
            DBGetFormalNameStr(con_obj_num), 
            XSW_OBJ_NAME_MAX + 80
        );
        name1[XSW_OBJ_NAME_MAX + 80 - 1] = '\0';

        /* Inputs assumed valid. */
        for(i = 0; i < total_objects; i++)
        {
            obj_ptr = xsw_object[i];
            if(obj_ptr == NULL)
                continue;
  
            if((obj_ptr->type != XSW_OBJ_TYPE_WEAPON) &&
               (obj_ptr->type != XSW_OBJ_TYPE_STREAMWEAPON) &&
               (obj_ptr->type != XSW_OBJ_TYPE_SPHEREWEAPON)
            )
                continue; 
 
            /* Owner by other and can recycle other? */
            if((con_obj_ptr->permission.uid > ACCESS_UID_RECYCLEO) &&
               (obj_ptr->owner > -1) &&   
               (obj_ptr->owner != con_obj_num)
            )
                continue;
         
            /* Cannot recycle something with a higher uid. */
            if(con_obj_ptr->permission.uid > obj_ptr->permission.uid)
                continue;
        
        
            /* Get name of object we're about to recycle. */
            strncpy(
                name2,
                DBGetFormalNameStr(i),
                XSW_OBJ_NAME_MAX + 80
            );
            name2[XSW_OBJ_NAME_MAX + 80 - 1] = '\0';

            /* Notify connection of recycling this object. */
            sprintf(sndbuf, "%s: Recycled.", name2);
            NetSendLiveMessage(condescriptor, sndbuf);
                
            sprintf(
                text,
                "%s: Recycled %s.",
                name1, name2
            );
            if(sysparm.log_general)   
                LogAppendLineFormatted(fname.primary_log, text);
        
            /* Recycle the object. */
            DBRecycleObject(i);
        
            /* Notify all connections about object being recycled. */
            NetSendRecycleObject(-1, i);
        }

	return(0);
}

/*
 *	Recycle disarmed or dead weapons.
 *
 *	All permission checks will be tested and handled accordingly.
 */
static int CmdRecycleDeadWeapons(
	int condescriptor,
	int con_obj_num,
	xsw_object_struct *con_obj_ptr
)
{
	int i;
	xsw_object_struct *obj_ptr;
	char name1[XSW_OBJ_NAME_MAX + 80];
	char name2[XSW_OBJ_NAME_MAX + 80];
	char sndbuf[CS_DATA_MAX_LEN];
        char text[(2 * XSW_OBJ_NAME_MAX) + 256];


	/* Record connection object's name. */
        strncpy(
            name1,
            DBGetFormalNameStr(con_obj_num),
            XSW_OBJ_NAME_MAX + 80
        );
        name1[XSW_OBJ_NAME_MAX + 80 - 1] = '\0';

        /* Inputs assumed valid. */
	for(i = 0; i < total_objects; i++)
	{
	    obj_ptr = xsw_object[i];
	    if(obj_ptr == NULL)
		continue;

	    if((obj_ptr->type != XSW_OBJ_TYPE_WEAPON) &&
	       (obj_ptr->type != XSW_OBJ_TYPE_STREAMWEAPON) &&
               (obj_ptr->type != XSW_OBJ_TYPE_SPHEREWEAPON)
            )
		continue;

	    /* Is weapon disarmed or dead? */
	    if(obj_ptr->antimatter <= 0)
		continue;

	    /* Owner by other and can recycle other? */
            if((con_obj_ptr->permission.uid > ACCESS_UID_RECYCLEO) &&
               (obj_ptr->owner > -1) &&
               (obj_ptr->owner != con_obj_num)
            )
		continue;

            /* Cannot recycle something with a higher uid. */
	    if(con_obj_ptr->permission.uid > obj_ptr->permission.uid)
		continue;


	    /* Get name of object we're about to recycle. */
            strncpy(
                name2,
                DBGetFormalNameStr(i),
                XSW_OBJ_NAME_MAX + 80
            );  
            name2[XSW_OBJ_NAME_MAX + 80 - 1] = '\0';


	    /* Notify connection of recycling this object. */
            sprintf(sndbuf, "%s: Recycled.", name2);
	    NetSendLiveMessage(condescriptor, sndbuf);

	    sprintf(
		text,
		"%s: Recycled %s.",
		name1, name2
	    );
	    if(sysparm.log_general)
		LogAppendLineFormatted(fname.primary_log, text);

	    /* Recycle the object. */
	    DBRecycleObject(i);

	    /* Notify all connections about object being recycled. */
	    NetSendRecycleObject(-1, i);
	}

        return(0);
}


int CmdRecycle(int condescriptor, const char *arg)
{
	int obj_num, con_obj_num;
	xsw_object_struct *obj_ptr, *con_obj_ptr;
        char name1[XSW_OBJ_NAME_MAX + 80];
        char name2[XSW_OBJ_NAME_MAX + 80];

        char sndbuf[CS_DATA_MAX_LEN];
        char text[(2 * XSW_OBJ_NAME_MAX) + 256];


        /* Get connection's object number (assumed valid). */
        con_obj_num = connection[condescriptor]->object_num;
        con_obj_ptr = xsw_object[con_obj_num];

	/* Check if argument is valid. */
	if((arg == NULL) ? 1 : (*arg == '\0'))
	{
	    sprintf(sndbuf,
 "Usage: `%s [<object>|#weapons|#dead_weapons]'",
                THIS_CMD_NAME
            );
            NetSendLiveMessage(condescriptor, sndbuf);
            return(-1);
        }


        /* Connection allowed to recycle? */
        if(con_obj_ptr->permission.uid > ACCESS_UID_RECYCLE)
        {
            sprintf(
                sndbuf,
                "%s: Permission denied: Requires access level %i.",
                THIS_CMD_NAME,
                ACCESS_UID_RECYCLE
            );
            NetSendLiveMessage(condescriptor, sndbuf);
            return(-1);
        }


        /* Skip leading spaces. */
        while(ISBLANK(*arg))
            arg++;

        /* Parse first argument. */
	if(!strcasecmp(arg, "#weapons"))
	{
	    CmdRecycleWeapons(
		condescriptor,
		con_obj_num,
		con_obj_ptr
	    );
	    return(0);
	}
	else if(!strcasecmp(arg, "#dead_weapons"))
	{
	    CmdRecycleDeadWeapons(
		condescriptor,
		con_obj_num, 
                con_obj_ptr
	    );
	    return(0);
	}

	/* Standard object match for first argument. */
	if(!strcasecmp(arg, "me"))
            obj_num = con_obj_num;
        else
            obj_num = MatchObjectByName(
                xsw_object, total_objects,
                arg, -1
            );
        if(DBIsObjectGarbage(obj_num))
        {
            sprintf(
		sndbuf,
                "%s: No such object.",
                arg
            );
            NetSendLiveMessage(condescriptor, sndbuf);
            return(-1);
        }
	else
	{
	    obj_ptr = xsw_object[obj_num];
	}


	/* Is object a player? */
	if(obj_ptr->type == XSW_OBJ_TYPE_PLAYER)
        {
            sprintf(
		sndbuf,
                "%s: Cannot recycle player objects, use `recycleplayer'.",
		THIS_CMD_NAME
            );
            NetSendLiveMessage(condescriptor, sndbuf);
            return(-1);
        }

        /* If object owned by other, allowed to recycle others? */
        if((con_obj_ptr->permission.uid > ACCESS_UID_RECYCLEO) &&
           (obj_ptr->owner > -1) &&
           (obj_ptr->owner != con_obj_num)
        )
        {
            sprintf(
		sndbuf,
                "%s: Permission denied: Requires access level %i.",
		THIS_CMD_NAME,
                ACCESS_UID_RECYCLEO
            );
            NetSendLiveMessage(condescriptor, sndbuf);
            return(-1);
        }
        /* Cannot recycle something with a UID greater than yours. */
        else if(con_obj_ptr->permission.uid > obj_ptr->permission.uid)
        {
            sprintf(sndbuf,
   "%s: Target object access level %i exceeds your access level %i.",
		THIS_CMD_NAME,
                obj_ptr->permission.uid,
                con_obj_ptr->permission.uid
            );
            NetSendLiveMessage(condescriptor, sndbuf);
            return(-1);
        }


        /* Print response and log operation. */
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

        sprintf(
	    sndbuf,
            "Placed %s in recycle backup buffer.",
            name2
        );
        NetSendLiveMessage(condescriptor, sndbuf);

        sprintf(
	    text,
            "%s: Recycled %s and placed into recycle backup buffer.",
            name1, name2
        );
        if(sysparm.log_general)
            LogAppendLineFormatted(fname.primary_log, text);


        /* Save object into recycled objects buffer. */
        DBSaveRecycledObject(obj_num);

	/* Actually recycle the object. */
        DBRecycleObject(obj_num);

	/* Notify all connections about object being recycled. */
        NetSendRecycleObject(-1, obj_num);


        return(0);
}
