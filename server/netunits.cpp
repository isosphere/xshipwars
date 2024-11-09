#include "../include/unvmatch.h"
#include "../include/swnetcodes.h"
#include "swserv.h"
#include "net.h"


int NetHandleSetUnits(int condescriptor, char *arg)
{
	/* Request to set from client implies request for values. */


	return(0);
}

int NetSendUnits(int condescriptor)
{
        char sndbuf[CS_DATA_MAX_LEN];


	/*
	 *   Format SWEXTCMD_SETUNITS:
	 *
	 *	ru_to_au
	 */
        sprintf(sndbuf, "%i %i %.4f\n",
                CS_CODE_EXT,
                SWEXTCMD_SETUNITS,
                sw_units.ru_to_au
        );

	NetDoSend(condescriptor, sndbuf);


        return(0);
}
