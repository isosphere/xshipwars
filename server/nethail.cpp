#include "../include/unvmatch.h"
#include "../include/unvmath.h"
#include "../include/swnetcodes.h"

#include "swserv.h"
#include "net.h"


int NetHandleHail(int condescriptor, char *arg)
{
	int object_count;
	int src_obj;
	int tar_obj;
	double bearing;
	int channel;
	int tar_con_num;
	xsw_object_struct *src_obj_ptr;


        /*  
         *	SWEXTCMD_HAIL format:
         *
         *      src_obj, tar_obj, bearing, channel
         */
        sscanf(arg,
		"%i %i %lf %i",

                &src_obj,
		&tar_obj,
		&bearing,
		&channel
        );

	/* Connection must control source object. */
	if(connection[condescriptor]->object_num != src_obj)
	    return(-3);

	/* Need valid source object pointer. */
	if(DBIsObjectGarbage(src_obj))
	    return(-1);
	else
	    src_obj_ptr = xsw_object[src_obj];


	for(object_count = 0; object_count < total_objects; object_count++)
	{
            /* Object not valid or not in range? */
	    if(!Mu3DInRange(
		xsw_object, total_objects,
		src_obj, object_count, COM_SHORT_RANGE_MAX
	    ))
		continue;

	    /* Is tar_obj specified? */
	    if((tar_obj > -1) &&
               (tar_obj != object_count)
	    )
	    {
		/* Skip since hail is not ment for object_count. */
		continue;
	    }

	    /* Get tar_con_num of object_count */
	    tar_con_num = ConGetByObject(object_count);
	    if(tar_con_num < 0) continue;

	    /* Get bearing, object_count to source object. */
	    bearing = MuCoordinateDeltaVector(
		src_obj_ptr->x - xsw_object[object_count]->x,
		src_obj_ptr->y - xsw_object[object_count]->y
	    );

	    /* Send hail to that connection. */
	    NetSendHail(
		tar_con_num,
		src_obj,
		tar_obj,
		bearing,
		channel
	    );
	}


        return(0);
}


int NetSendHail(
	int condescriptor,
	int src_obj, int tar_obj,
	double bearing, int channel
)
{
	char sndbuf[CS_DATA_MAX_LEN];


        /*
         *      SWEXTCMD_HAIL format:
         *
         *      src_obj, tar_obj, bearing, channel
         */
        sprintf(sndbuf,
"%i %i\
 %i %i %.4f %i\n",
                CS_CODE_EXT,
                SWEXTCMD_HAIL,

                src_obj,
                tar_obj,
		bearing,
		channel
        );
        NetDoSend(condescriptor, sndbuf);


        return(0);
}
