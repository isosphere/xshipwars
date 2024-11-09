#include "../include/unvmatch.h"
#include "../include/swnetcodes.h"
#include "swserv.h"
#include "net.h"


int NetHandleSetIntercept(int condescriptor, char *arg)
{
        char *strptr;
        long object_num, con_object_num;
        char sndbuf[CS_DATA_MAX_LEN];
        char intercept_arg[CS_DATA_MAX_LEN];
        xsw_object_struct *obj_ptr;


        con_object_num = connection[condescriptor]->object_num;
        if(DBIsObjectGarbage(con_object_num))
            return(-1);
        else
            obj_ptr = xsw_object[con_object_num];


        /*
         *   NET_CMD_INTERCEPT format:
	 *
         *      object_num, argument
         */
        StringStripSpaces(arg);   
        strptr = strchr(arg, ' ');
        if(strptr == NULL) return(-1);
        
        strncpy(intercept_arg, strptr + 1, CS_DATA_MAX_LEN);
        intercept_arg[CS_DATA_MAX_LEN - 1] = '\0';
        StringStripSpaces(intercept_arg);

        *strptr = '\0';
        object_num = atol(arg);


        /* Connection must own object. */
        if(object_num != con_object_num)
            return(-3);


        /* Match intercepting object. */
        obj_ptr->intercepting_object = MatchIntercept(
	    xsw_object, total_objects,
	    object_num, intercept_arg
	);

	/* Given argument not "#off"? */
        if(strcasecmp(intercept_arg, "#off"))
        {
	    /* Check if valid object to intercept was matched. */
            if((*intercept_arg != '\0') &&
               (obj_ptr->intercepting_object < 0)
            )
            {
                sprintf(sndbuf,
                    "Intercept: %s: Cannot find object.",
                    intercept_arg
                );
                NetSendLiveMessage(condescriptor, sndbuf);
            }
        }


        /* Set next object values update to now so it gets updated now. */
        next.object_values = cur_millitime;


        return(0);
}


int NetSendSetIntercept(int condescriptor)
{
	return(0);
}
