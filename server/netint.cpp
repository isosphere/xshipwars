#include "../include/unvmatch.h"
#include "../include/swnetcodes.h"
#include "swserv.h"
#include "net.h"


int NetHandleSetInterval(int condescriptor, char *arg)
{
	long interval;


        if(!ConIsLoggedIn(condescriptor))
            return(-1);

        /* arg must be atleast 1 character long. */
        if(arg[0] == '\0')
            return(-1);

        /*
         *   NET_CMD_SETINTERVAL format is as follows:
         *      
         *      interval
         */
        sscanf(arg,
                "%ld",
                &interval
        );

        /* Make sure interval is within min and max values. */
        if(interval < MIN_OBJECT_UPDATE_INT)
                interval = MIN_OBJECT_UPDATE_INT;

        if(interval > MAX_OBJECT_UPDATE_INT)
                interval = MAX_OBJECT_UPDATE_INT;


        /* Set new update interval for connection. */
        connection[condescriptor]->obj_ud_interval = interval;


	return(0);
}


int NetSendSetInterval(int condescriptor)
{
	return(0);
}
