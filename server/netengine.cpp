#include "../include/unvmatch.h"
#include "../include/swnetcodes.h"
#include "swserv.h"
#include "net.h"


int NetHandleSetEngine(int condescriptor, char *arg)
{
        int object_num, con_object_num;
        int engine_state;
        xsw_object_struct *obj_ptr;


        con_object_num = connection[condescriptor]->object_num;
        if(DBIsObjectGarbage(con_object_num))
            return(-1);
        else
            obj_ptr = xsw_object[con_object_num];


        /*
         *	SWEXTCMD_SETENGINE format:
         *
         *      object_num engine_state
         */
        sscanf(arg, "%i %i",
                &object_num,
                &engine_state
        );

        /* Connection must own object. */
        if(object_num != con_object_num)
            return(-3);

        if(DBIsObjectGarbage(object_num))
            return(-1);
        else
            obj_ptr = xsw_object[object_num];


        obj_ptr->engine_state = engine_state;


	/* Send engine state update to all connections. */
	NetSendSetEngine(-1, object_num);


        return(0);
}


int NetSendSetEngine(int condescriptor, int object_num)
{
	xsw_object_struct *obj_ptr;
        char sndbuf[CS_DATA_MAX_LEN];


        if(DBIsObjectGarbage(object_num))
            return(-1);
        else
            obj_ptr = xsw_object[object_num];


        /*
         *   SWEXTCMD_SETENGINE format:
         *
         *      object_num engine_state
         */
        sprintf(sndbuf,
"%i %i\
 %i %i\n",
                CS_CODE_EXT,
                SWEXTCMD_SETENGINE,
 
                object_num,
                obj_ptr->engine_state
        );
        NetDoSend(condescriptor, sndbuf);


	return(0);
}
