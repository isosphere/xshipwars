#include "../include/unvmatch.h"
#include "../include/swnetcodes.h"
#include "swserv.h"
#include "net.h"


int NetHandleScore(int condescriptor, char *arg)
{
	int object_num;
        xsw_object_struct *obj_ptr;


        /*
         *   SWEXTCMD_SETSCORE format:
         *
         *      object_num
         *      credits rmu rmu_max damage_given damage_recieved
         *      kills
         */
        sscanf(arg,
		"%i",
                &object_num
        );

	if(DBIsObjectGarbage(object_num))
	    return(-1);
	else
	    obj_ptr = xsw_object[object_num];


	NetSendScore(condescriptor, object_num);


        return(0);
}


int NetSendScore(int condescriptor, int object_num)
{
	char sndbuf[CS_DATA_MAX_LEN];
	xsw_object_struct *obj_ptr;
	xsw_score_struct *score_ptr;


	if(DBIsObjectGarbage(object_num))
	    return(-1);
	else
	    obj_ptr = xsw_object[object_num];

	score_ptr = obj_ptr->score;
	if(score_ptr == NULL)
	    return(0);


        /*
         *   SWEXTCMD_SETSCORE format:
         *
         *	object_num 
	 *	credits rmu rmu_max damage_given damage_recieved
	 *	kills
         */
        sprintf(sndbuf,
"%i %i\
 %i %.4f %.4f %.4f %.4f %.4f %i\n",
                CS_CODE_EXT,
                SWEXTCMD_SETSCORE,

                object_num,
                score_ptr->credits,
		score_ptr->rmu,
		score_ptr->rmu_max,
                score_ptr->damage_given,
                score_ptr->damage_recieved,
                score_ptr->kills
        );
        NetDoSend(condescriptor, sndbuf);


        return(0);
}
