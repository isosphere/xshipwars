#include "../include/unvmatch.h"
#include "swserv.h"
#include "net.h"

#define THIS_CMD_NAME	"score"


/*
 *      Prints object's score.
 */
int CmdScore(int condescriptor, const char *arg)
{
        int con_obj_num, obj_num;
        xsw_object_struct *obj_ptr, *con_obj_ptr;
        connection_struct *con_ptr;

        char sndbuf[CS_DATA_MAX_LEN]; 


        /* Get connection's object number (assumed valid). */
        con_ptr = connection[condescriptor];
        con_obj_num = con_ptr->object_num;
        con_obj_ptr = xsw_object[con_obj_num];

        /* Parse object number. */
        if((arg == NULL) ? 1 : (*arg == '\0'))
            obj_num = con_obj_num;
	else if(!strcasecmp(arg, "me"))
	    obj_num = con_obj_num;
        else
            obj_num = MatchObjectByName(
		xsw_object, total_objects,
		arg, -1
	    );
        if(DBIsObjectGarbage(obj_num))
        {
            sprintf(
		sndbuf,
		"%s: No such object.",
		arg
	    );
            NetSendLiveMessage(condescriptor, sndbuf);

            return(-1);
        }
	else
	{
	    obj_ptr = xsw_object[obj_num];
	}

        /* Does object have scores? */
        if(obj_ptr->score == NULL)
        {
            sprintf(sndbuf, "%s: No scores.", obj_ptr->name);
        }
	else
	{
            sprintf(sndbuf,
 "%s: Credits: %.2f  RMU: %.2f(%.2f)  DmgGiv: %.2f  DmgRec: %.2f\
  Kills: %i",
                DBGetFormalNameStr(obj_num),
                obj_ptr->score->credits,
                obj_ptr->score->rmu,
                obj_ptr->score->rmu_max,
                obj_ptr->score->damage_given,
                obj_ptr->score->damage_recieved,
                obj_ptr->score->kills
            );
	}
        NetSendLiveMessage(condescriptor, sndbuf);


        return(0);
}
