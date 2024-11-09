#include "../include/unvmatch.h"
#include "../include/swnetcodes.h"
#include "swserv.h"
#include "net.h"


int NetHandleObjectThrottle(int condescriptor, char *arg)
{
	long object_num, con_object_num;
        float throttle;
        xsw_object_struct *obj_ptr;


        con_object_num = connection[condescriptor]->object_num;
        if(DBIsObjectGarbage(con_object_num))
            return(-1);
        else
            obj_ptr = xsw_object[con_object_num];


        /*  
         *	SWEXTCMD_SETTHROTTLE format:
         *
         *      object_num, throttle
         */
        sscanf(arg,
                "%ld %f",

                &object_num,
                &throttle   
        );


        /* Connection must own object. */
        if(object_num != con_object_num)
            return(-3);


        /* Sanitize throttle value. */
        if(throttle < 0.0)
            throttle = 0.0;
        else if(throttle > 1.0)
            throttle = 1.0;
        
        /* Set throttle value. */
        obj_ptr->throttle = throttle;


        return(0);
}


int NetSendObjectThrottle(int condescriptor)
{

	return(0);
}
