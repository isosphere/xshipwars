#include "../include/unvmatch.h"
#include "../include/unvmath.h"

#include "swserv.h"
#include "net.h"


#define THIS_CMD_NAME	"eta"  


int CmdETA(int condescriptor, const char *arg)
{
	int con_obj_num, dest_obj_num;
	xsw_object_struct *con_obj_ptr, *dest_obj_ptr;
	connection_struct *con_ptr;
        double distance, velocity, eta;
	char sndbuf[CS_DATA_MAX_LEN];


        /* Get connection's object number (assumed valid). */
        con_ptr = connection[condescriptor];
        con_obj_num = con_ptr->object_num;
	con_obj_ptr = xsw_object[con_obj_num];


        /* Get destination object. */
        dest_obj_num = con_obj_ptr->intercepting_object;
        if(DBIsObjectGarbage(dest_obj_num))
            return(0);
        else
            dest_obj_ptr = xsw_object[dest_obj_num];


        /* Cannot calculate if velocity is 0. */
        velocity = con_obj_ptr->velocity;
        if(velocity <= 0)
            return(0);

        /* Check if in same sector. */   
        if(!Mu3DInSameSectorPtr(
	    con_obj_ptr, dest_obj_ptr
	))
        {
            sprintf(
		sndbuf,
                "%s: Destination not in current sector.",
		THIS_CMD_NAME
            );
            NetSendLiveMessage(condescriptor, sndbuf);

            return(0);
        }

        /* Get distance between objects (in XSW real units). */
        distance = Mu3DDistance(
            dest_obj_ptr->x - con_obj_ptr->x,
            dest_obj_ptr->y - con_obj_ptr->y,
            dest_obj_ptr->z - con_obj_ptr->z
        );

        /* Calculate eta in units of `cycles'. */
        eta = distance / velocity;

        /* Convert ETC in `cycles' to milliseconds. */
        eta = eta * (1000 / CYCLE_LAPSE_MS) / 1000;

        sprintf(sndbuf,
            "ETA to %s in %s",
            dest_obj_ptr->name,
            StringFormatTimePeriod((time_t)eta)
        );
        NetSendLiveMessage(condescriptor, sndbuf);


        return(0);
}
