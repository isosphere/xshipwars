#include "../include/unvmatch.h"
#include "../include/swnetcodes.h"
#include "swserv.h"
#include "net.h"


int NetHandleLighting(int condescriptor, char *arg)
{
        long object_num, con_object_num;
	int lighting;	/* Real type is xswo_lighting_t. */
        xsw_object_struct *obj_ptr;


        con_object_num = connection[condescriptor]->object_num;
        if(DBIsObjectGarbage(con_object_num))
            return(-1);
        else
            obj_ptr = xsw_object[con_object_num];

        /*
         *	SWEXTCMD_SETLIGHTING format:
         *
         *      object_num lighting
         */
        sscanf(arg, "%ld %i",
                &object_num,
                &lighting   
        );


        /* Can only set connection's object. */
        if(object_num != con_object_num)
            return(-3);


        /* Set lighting. */
        obj_ptr->lighting = lighting;


        /* Set next object values update to now so it gets updated now. */
        next.object_values = cur_millitime;


        return(0);
}


int NetSendLighting(int condescriptor)
{
	return(0);
}
