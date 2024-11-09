#include "../include/unvmatch.h"
#include "../include/swnetcodes.h"
#include "swserv.h"
#include "net.h"


int NetHandleSysMessage(int condescriptor)
{
	return(0);
}


int NetSendSysMessage(
        int condescriptor,
        int code,               /* One of CS_SYSMESG_* */
        char *mesg
)
{
        char lmesg[CS_MESG_MAX + 256];
        char sndbuf[CS_DATA_MAX_LEN + 256];


        if(mesg == NULL)
	    return(-1);

        strncpy(lmesg, mesg, CS_MESG_MAX);
        lmesg[CS_MESG_MAX - 1] = '\0';


        /*
         *      CS_CODE_SYSMESSAGE format:
         *      
         *      code, mesg
         */
        sprintf(sndbuf, "%i %i %s\n",
                CS_CODE_SYSMESSAGE,
                code,
                lmesg
        );
        NetDoSend(condescriptor, sndbuf);


	return(0);
}
