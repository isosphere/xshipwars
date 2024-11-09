#include "../include/unvmatch.h"
#include "swserv.h"
#include "net.h"
  
 
/*
 *      List processes.
 *      
 *      NOTE: This function is not completely formalized,
 *      information sent to connection is not standard.  
 */
int CmdPS(int condescriptor, const char *arg)
{
	int i;
        char can_view_others;
        int uid;
        int con_obj_num;
        xsw_object_struct *con_obj_ptr;
        connection_struct *con_ptr;

        int processes_listed;
        schedual_struct **ptr, *sh_ptr;

	char sndbuf[CS_DATA_MAX_LEN];


        /* Get connection's object number (assumed valid). */
        con_ptr = connection[condescriptor];
        con_obj_num = con_ptr->object_num;
        con_obj_ptr = xsw_object[con_obj_num];


        /* Get uid. */
        uid = con_obj_ptr->permission.uid;

        /* Can connection view processes that belong to others? */
        can_view_others = (uid > ACCESS_UID_PSO) ? 0 : 1;


        /* Print header. */
        NetSendLiveMessage(
            condescriptor,
            "-   PID  STAT     INT  USER"
        );


        /* Begin printing schedualed events. */
        processes_listed = 0;
        for(i = 0, ptr = schedual;
            i < total_scheduals;
            i++, ptr++
        )
        {
	    sh_ptr = *ptr;
            if(!SchedualIsActive(i))
		continue;

            /* Check if allowed to view others. */
            if((con_obj_num != sh_ptr->run_owner) &&
               !can_view_others
            )
                continue;


            /* Print line. */
            sprintf(sndbuf,
		"-%6d  %4d  %6ld  %s",
                i,
                1,
                sh_ptr->act_int,
                DBGetFormalNameStr(sh_ptr->run_owner)
            );
            NetSendLiveMessage(condescriptor, sndbuf);

            processes_listed++;
        }


        /* Print footer. */
        sprintf(sndbuf, "%i processes.",
            processes_listed
        );
        NetSendLiveMessage(condescriptor, sndbuf);


        return(0);
}
