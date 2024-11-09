#include "../include/unvmatch.h"
#include "../include/unvmath.h"
#include "../include/swnetcodes.h"

#include "swserv.h"
#include "net.h"


/*
 *	Sends new values of weapon object to all nearby connections.
 */
void NetWeaponDisarmSyncObjectValues(
	xsw_object_struct *obj_ptr,
	int object_num
)
{
	int i, con_obj_num;
	connection_struct **ptr, *con_ptr;
	xsw_object_struct *con_obj_ptr;


        /* Check if marked hidden from connection. */
        if(obj_ptr->server_options & XSW_OBJF_HIDEFROMCON)
            return;


	for(i = 0, ptr = connection;
            i < total_connections;
            i++, ptr++
	)
	{
	    con_ptr = *ptr;
	    if(con_ptr == NULL)
		continue;

            con_obj_num = con_ptr->object_num;
            if(DBIsObjectGarbage(con_obj_num))
                continue;
            else
                con_obj_ptr = xsw_object[con_obj_num];

            /* Check if objects are valid and in range. */
            if(!Mu3DInRangePtr(con_obj_ptr, obj_ptr,
                con_obj_ptr->scanner_range
            ))
		continue;

            /* Not owned objects, check for visibility. */
            if(obj_ptr->owner != con_obj_num)
            {
                if(DBGetObjectVisibilityPtr(obj_ptr) <= 0.00)
                    continue;
	    }

            /* Send all weapon value to connection. */
	    NetSendObjectMaximums(i, object_num);
	    NetSendObjectValues(i, object_num);
	}

	return;
}

int NetHandleWeaponDisarm(int condescriptor, char *arg)
{
	int i, src_obj, tar_obj, con_object_num;
	xsw_object_struct *wep_obj_ptr, *con_obj_ptr, **ptr;


        if(!ConIsLoggedIn(condescriptor))
	    return(-1);

        con_object_num = connection[condescriptor]->object_num;
        if(DBIsObjectGarbage(con_object_num))
            return(-1);
        else
            con_obj_ptr = xsw_object[con_object_num];
 

        /*
         *   SWEXTCMD_WEPDISARM format:
         *
         *      src_obj tar_obj
         */
        sscanf(arg,
"%i %i",
		&src_obj,
		&tar_obj
        );

        /* Connection must own object. */
        if(src_obj != con_object_num)
            return(-3);

	if(tar_obj < 0)
	{
	    /* Disarm all weapons that connection object owns. */
	    for(i = 0, ptr = xsw_object;
                i < total_objects;
	        i++, ptr++
	    )
	    {
		wep_obj_ptr = *ptr;
		if(wep_obj_ptr == NULL)
		    continue;

		/* Not a weapon? */
		if(wep_obj_ptr->type != XSW_OBJ_TYPE_WEAPON)
		    continue;

		/* Source object owns weapon object? */
		if(wep_obj_ptr->owner == src_obj)
		{
                    /* Disarm this weapon. */
                    wep_obj_ptr->antimatter = 0;
		    wep_obj_ptr->throttle = 0;
                    wep_obj_ptr->locked_on = -1;
                    wep_obj_ptr->intercepting_object = -1;
 
                    NetSendWeaponDisarm(
                        condescriptor,
                        src_obj,
                        i
                    );
                    NetWeaponDisarmSyncObjectValues(
                        wep_obj_ptr,
                        i
                    );
		}
	    }
	}
	else
	{
	    /* Disarm specific weapon if connection object owns it. */

	    if(!DBIsObjectGarbage(tar_obj))
	    {
		wep_obj_ptr = xsw_object[tar_obj];


		/* Source object owns weapon object? */
		if((wep_obj_ptr->owner == src_obj) &&
                   (wep_obj_ptr->type == XSW_OBJ_TYPE_WEAPON)
		)
		{
		    /* Disarm this weapon. */
		    wep_obj_ptr->antimatter = 0;
                    wep_obj_ptr->throttle = 0;
		    wep_obj_ptr->locked_on = -1;
		    wep_obj_ptr->intercepting_object = -1;

		    NetSendWeaponDisarm(
			condescriptor,
			src_obj,
			tar_obj
		    );
		    NetWeaponDisarmSyncObjectValues(
			wep_obj_ptr,
			tar_obj
		    );
		}
	    }
	}

        return(0);
}


int NetSendWeaponDisarm(int condescriptor, int src_obj, int tar_obj)
{
        char sndbuf[CS_DATA_MAX_LEN];


        /*
         *   SWEXTCMD_WEPDISARM format:
         *
         *      src_obj tar_obj
         */
        sprintf(sndbuf,
"%i %i\
 %i %i\n",
                CS_CODE_EXT,
                SWEXTCMD_WEPDISARM,

                src_obj,
                tar_obj
	);
	NetDoSend(condescriptor, sndbuf);


	return(0);
}
