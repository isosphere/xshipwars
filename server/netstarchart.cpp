#include "../include/isrefs.h"
#include "../include/unvmatch.h"
#include "../include/swnetcodes.h"

#include "swserv.h"
#include "net.h"


/* 
 *	Sends an add object to star chart.
 */
int NetSendStarChartAddObject(
	int condescriptor,
	int object_num
)
{
        char sndbuf[CS_DATA_MAX_LEN];
        xsw_object_struct *obj_ptr;


        if(DBIsObjectGarbage(object_num))
            return(-1);
        else
            obj_ptr = xsw_object[object_num];

        /*
	 *   SWEXTCMD_STARCHART_ADD_OBJECT format:
	 *
	 *      object_num
	 *      type isref_num size
	 *      sect_x sect_y sect_z
	 *      x, y, z
	 *      heading, pitch, bank
	 */
	sprintf(sndbuf,
"%i %i %i\
 %i %i %ld\
 %ld %ld %ld\
 %.4f %.4f %.4f\
 %.4f %.4f %.4f\n",

		CS_CODE_EXT,
		SWEXTCMD_STARCHART_ADD_OBJECT,
		object_num,

		obj_ptr->type,
                obj_ptr->imageset,
                obj_ptr->size,

                obj_ptr->sect_x,
                obj_ptr->sect_y,
                obj_ptr->sect_z,

                obj_ptr->x,
                obj_ptr->y,
                obj_ptr->z,

                obj_ptr->heading,
                obj_ptr->pitch,
                obj_ptr->bank
	);
        NetDoSend(condescriptor, sndbuf);

	return(0);
}

/*
 *	Sends a set object name on star chart.
 */
int NetSendStarChartSetObjectName(
        int condescriptor,
        int object_num
)
{
        char sndbuf[CS_DATA_MAX_LEN];
        xsw_object_struct *obj_ptr;


        if(DBIsObjectGarbage(object_num))
            return(-1);
        else
            obj_ptr = xsw_object[object_num]; 

        /*
         *   SWEXTCMD_STARCHART_SET_OBJECT_NAME format:
	 *
	 *	object_num;name
	 */
	sprintf(sndbuf, "%i %i %i;%s\n",
		CS_CODE_EXT,
		SWEXTCMD_STARCHART_SET_OBJECT_NAME,
		object_num,
		obj_ptr->name
	);
        NetDoSend(condescriptor, sndbuf);

        return(0);
}

/*
 *      Sends a set object empire on star chart.
 */
int NetSendStarChartSetObjectEmpire(
        int condescriptor,
        int object_num
)
{
        char sndbuf[CS_DATA_MAX_LEN];
        xsw_object_struct *obj_ptr;

  
        if(DBIsObjectGarbage(object_num))
            return(-1);
        else
            obj_ptr = xsw_object[object_num];
 
        /*
         *   SWEXTCMD_STARCHART_SET_OBJECT_EMPIRE format:
         *
         *      object_num;empire
         */     
        sprintf(sndbuf, "%i %i %i;%s\n",
                CS_CODE_EXT,
                SWEXTCMD_STARCHART_SET_OBJECT_EMPIRE,
                object_num,
                obj_ptr->empire
        );
        NetDoSend(condescriptor, sndbuf);

	return(0);
}

/*
 *      Sends a recycle object on star chart.
 */
int NetSendStarChartRecycleObject(
        int condescriptor,
        int object_num
)
{
        char sndbuf[CS_DATA_MAX_LEN];

        /*
         *   SWEXTCMD_STARCHART_SET_OBJECT_RECYCLE format:
         *
         *      object_num
         */
        sprintf(sndbuf, "%i %i %i\n",
                CS_CODE_EXT,
		SWEXTCMD_STARCHART_SET_OBJECT_RECYCLE,
		object_num
	);
        NetDoSend(condescriptor, sndbuf);

	return(0);
}


/*
 *	Procedure to send entire star chart to connection.
 */
int NetSendEntireStarChart(int condescriptor)
{
	int i;
	xsw_object_struct **ptr, *obj_ptr;


	for(i = 0, ptr = xsw_object;
            i < total_objects;
            i++, ptr++
	)
	{
	    obj_ptr = *ptr;
	    if(obj_ptr == NULL)
		continue;

	    if((obj_ptr->type != XSW_OBJ_TYPE_HOME) &&
               (obj_ptr->type != XSW_OBJ_TYPE_AREA) &&
               (obj_ptr->type != XSW_OBJ_TYPE_WORMHOLE) &&
               (obj_ptr->type != XSW_OBJ_TYPE_ELINK)
	    )
		continue;


	    NetSendStarChartAddObject(condescriptor, i);
	    NetSendStarChartSetObjectName(condescriptor, i);
	    NetSendStarChartSetObjectEmpire(condescriptor, i);
	}

	return(0);
}
