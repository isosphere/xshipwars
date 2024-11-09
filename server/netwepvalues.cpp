#include "../include/unvmatch.h"
#include "../include/swnetcodes.h"
#include "swserv.h" 
#include "net.h"          


int NetHandleWeaponValues(int condescriptor, char *arg)
{
	return(0);
}
 
 
int NetSendWeaponValues(
	int condescriptor,
	int object_num, int weapon_num
)
{
	xsw_object_struct *obj_ptr;
	xsw_weapons_struct *wep_ptr;
	char sndbuf[CS_DATA_MAX_LEN];


        if(DBIsObjectGarbage(object_num))
            return(-1);
        else
            obj_ptr = xsw_object[object_num];


        /* weapon_num must be valid. */
        if((weapon_num < 0) ||
           (weapon_num >= obj_ptr->total_weapons)
        )
            wep_ptr = NULL;
	else
	    wep_ptr = obj_ptr->weapons[weapon_num];

	if(wep_ptr == NULL)
	    return(-1);


        /*
         *	SWEXTCMD_STDWEPVALS format:
         *
         *      object_num, weapon_num,
         *      ocs_code, emission_type, amount, max,
         *      power, range, create_power,
         *      delay, last_used, fire_sound_code flags
         */
        sprintf(sndbuf,
"%i %i\
 %i %i\
 %i %i %i %i\
 %.4f %ld %.4f\
 %ld %ld %i %lu\n",
                CS_CODE_EXT,
                SWEXTCMD_STDWEPVALS,
        
                object_num,
                weapon_num,
        
                wep_ptr->ocs_code,
                wep_ptr->emission_type,
                wep_ptr->amount,
                wep_ptr->max,

                wep_ptr->power,
                wep_ptr->range,
                wep_ptr->create_power,

                wep_ptr->delay,
                wep_ptr->last_used,
		wep_ptr->fire_sound_code,
		(unsigned long)wep_ptr->flags
        );

        NetDoSend(condescriptor, sndbuf);


        return(0);
}
