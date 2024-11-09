#include "../include/unvmatch.h"
#include "../include/swnetcodes.h"
#include "swserv.h"
#include "net.h"


int NetHandleTractorBeamLock(int condescriptor, char *arg)
{       
        long sobj_num;
        long tobj_num;
        xsw_object_struct *obj_ptr, *tar_obj_ptr;


        /*
         *      SWEXTCMD_TRACTORBEAMLOCK format:
         *
         *      src_obj, tar_obj
         */
        sscanf(arg, "%ld %ld",
                &sobj_num,
                &tobj_num
        );

        /* Check if sobj_num is valid. */
        if(DBIsObjectGarbage(sobj_num))
            return(-1);
	else
	    obj_ptr = xsw_object[sobj_num];

        /* Connection must control source object. */
        if(sobj_num != connection[condescriptor]->object_num)
            return(-3);


        /* *********************************************************** */

        if(DBIsObjectGarbage(tobj_num))
        {
            /* Remove target object from tractor beam lock. */
            DBObjectTractor(sobj_num, -1);
        }
        else
        {
	    tar_obj_ptr = xsw_object[tobj_num];

	    /* Target object is valid, check if is tractorable. */
            if(DBIsObjectTractorablePtr(obj_ptr, tar_obj_ptr))
                DBObjectTractor(sobj_num, tobj_num);
            else
                return(0);
        }


        /* Update all connections about tractor beam lock. */
	NetSendTractorBeamLock(-1, sobj_num, tobj_num);


        return(0);
}


int NetSendTractorBeamLock(
	int condescriptor,
	long src_obj, long tar_obj
)
{
	char sndbuf[CS_DATA_MAX_LEN];
        xsw_object_struct *obj_ptr;  


        if(DBIsObjectGarbage(src_obj))
            return(-1);
        else
            obj_ptr = xsw_object[src_obj];


        /*  
         *	SWEXTCMD_TRACTORBEAMLOCK format:
         *
         *      src_obj tar_obj
         */
        sprintf(sndbuf,
"%i %i\
 %ld %ld\n",

                CS_CODE_EXT,
                SWEXTCMD_TRACTORBEAMLOCK,

                src_obj,
                tar_obj
        );
        NetDoSend(condescriptor, sndbuf);


        return(0);
}
