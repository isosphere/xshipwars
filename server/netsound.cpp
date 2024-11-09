#include "../include/unvmatch.h"
#include "../include/swnetcodes.h"
#include "swserv.h"
#include "net.h"


int NetHandlePlaySound(int condescriptor, char *arg)
{
	return(0);
}


int NetSendPlaySound(
	int condescriptor, int sound_code,
        double left_volume, double right_volume
)
{
        char sndbuf[CS_DATA_MAX_LEN];


        if(left_volume < 0)
            left_volume = 0;

        if(right_volume < 0)
            right_volume = 0;

        /*
         *   CS_CODE_PLAYSOUND format:
         *
         *      sound_code volume
         */
        sprintf(sndbuf,
"%i %i\
 %.4f %.4f\n",
            CS_CODE_PLAYSOUND,
            sound_code,

            left_volume,
            right_volume
        );
        NetDoSend(condescriptor, sndbuf);


        return(0);
}
