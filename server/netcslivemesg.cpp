#include "../include/unvmatch.h"
#include "../include/swnetcodes.h"
#include "swserv.h"
#include "net.h"


int NetHandleLiveMessage(int condescriptor, char *mesg)
{
	long object_num;
        char sndbuf[CS_DATA_MAX_LEN + 256];
        char lmesg[CS_MESG_MAX + 256];
        int len;
        char stringa[CS_DATA_MAX_LEN + 256];


        if(!ConIsLoggedIn(condescriptor))
            return(-1);

        /* Make sure condescriptor has a valid object. */
        object_num = connection[condescriptor]->object_num;
        if(DBIsObjectGarbage(object_num))
	    return(-1);


        /* message must contain data. */
        if(mesg == NULL)
            return(-1);
        len = strlen(mesg);
        if(len < 1)
            return(-2);


        /* Copy mesg to lmesg and sanitize. */
        strncpy(lmesg, mesg, CS_MESG_MAX);
        lmesg[CS_MESG_MAX - 1] = '\0';

 
        /* ******************************************************* */
        /* Process message. */


        /* Format the message. */
        sprintf(sndbuf,
            "%s: \"%s\"",
            DBGetFormalNameStr(object_num),
            lmesg
        );
        NetSendLiveMessage(-1, sndbuf);
            
           
        /* Log global message. */
        sprintf(stringa, "%s: Global Message: \"%s\"",
            DBGetFormalNameStr(object_num),   
            lmesg
        );
        if(sysparm.log_events)
            LogAppendLineFormatted(fname.primary_log, stringa);


        return(0);
}


int NetSendLiveMessage(int condescriptor, char *mesg)
{
        char lmesg[CS_MESG_MAX];
        char sndbuf[CS_DATA_MAX_LEN];


        if(mesg == NULL)
	    return(-1);

        strncpy(lmesg, mesg, CS_MESG_MAX);
        lmesg[CS_MESG_MAX - 1] = '\0';


        /*
         *   CS_CODE_LIVEMESSAGE format:
         *
         *      mesg
         */
        sprintf(sndbuf, "%i %s\n",
                CS_CODE_LIVEMESSAGE,
                lmesg
        );
        NetDoSend(condescriptor, sndbuf);


	return(0);
}

/* Alias for NetSendLiveMessage. */
int ConNotify(int condescriptor, char *mesg)
{
	return(
	    NetSendLiveMessage(condescriptor, mesg)
	);
}

