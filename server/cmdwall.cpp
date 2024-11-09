#include "../include/unvmatch.h"
#include "swserv.h"
#include "net.h"


/*
 *	Prints message to all connections.
 */
int CmdWall(int condescriptor, const char *arg)
{
        int con_obj_num;
        xsw_object_struct *con_obj_ptr;
        connection_struct *con_ptr;

	char mesg[CS_MESG_MAX];
        char sndbuf[CS_DATA_MAX_LEN];


        /* Get connection's object number (assumed valid). */
        con_ptr = connection[condescriptor];
        con_obj_num = con_ptr->object_num;
        con_obj_ptr = xsw_object[con_obj_num];

	/* No argument given? */
	if((arg == NULL) ? 1 : (*arg == '\0'))
	    return(-1);

	/* Copy argument to local message buffer. */
	strncpy(mesg, arg, CS_MESG_MAX);
	mesg[CS_MESG_MAX - 1] = '\0';
        StringStripSpaces(mesg);

	/* Format and send message to all connections. */
	sprintf(
	    sndbuf,
	    "<%s>: \"%s\"",
	    con_obj_ptr->name,
	    mesg
	);
	NetSendLiveMessage(-1, sndbuf);

        return(0);
}
