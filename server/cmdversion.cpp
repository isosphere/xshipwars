#include "swserv.h"
#include "net.h"


/*
 *      Prints version number of the program to connection.
 */
int CmdVersion(int condescriptor, const char *arg)
{
        char sndbuf[CS_DATA_MAX_LEN];


        /* Print version to condescriptor. */
        sprintf(sndbuf,
            "%s - Version %s.",
            PROG_NAME,
            PROG_VERSION
        );
        NetSendLiveMessage(condescriptor, sndbuf);


        return(0);
}
