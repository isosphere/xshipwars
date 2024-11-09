#include "../include/unvmatch.h"
#include "../include/swnetcodes.h"
#include "swserv.h"
#include "net.h"


int NetHandleNotifyHit(int condescriptor, char *arg)
{
	return(0);
}


int NetSendNotifyHit(
        int condescriptor,
        int wep_obj, int tar_obj,
        double total_damage,
        double bearing,		/* tar_obj relative, to wep_obj. */
	double structure_damage,
	double shield_damage
)
{
        char sndbuf[CS_DATA_MAX_LEN];


        /*
         *    SWEXTCMD_NOTIFYHIT format:
	 *
         *      wep_obj tar_obj
         *      total_damage bearing
	 *	structure_damage shield_damage
         */
        sprintf(sndbuf,
"%i %i\
 %i %i\
 %.4f %.4f\
 %.4f %.4f\n",

                CS_CODE_EXT,
                SWEXTCMD_NOTIFYHIT,

                wep_obj, tar_obj,
                total_damage, bearing,
		structure_damage, shield_damage
        );
        NetDoSend(condescriptor, sndbuf);


        return(0);   
}
