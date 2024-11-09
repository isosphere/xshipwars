#include "../include/xsw_ctype.h"
#include "swserv.h"
#include "net.h"


int CmdDebug(int condescriptor, const char *arg)
{
        char *strptr;
        int con_obj_num;
        xsw_object_struct *con_obj_ptr;
        connection_struct *con_ptr;

	char num_str[80];
        char sndbuf[CS_DATA_MAX_LEN];
        char text[256];


        /* Get connection's object number (assumed valid). */
        con_ptr = connection[condescriptor];
        con_obj_num = con_ptr->object_num;
        con_obj_ptr = xsw_object[con_obj_num];

	/* Print usage? */
	if((arg == NULL) ? 1 : (*arg == '\0'))
	{
            sprintf(
		sndbuf,
 "debug level: %i  debug value: %.6f",
                debug.level, debug.val
            );
            NetSendLiveMessage(condescriptor, sndbuf);

            NetSendLiveMessage(
		condescriptor,
 "Usage: debug <level>[=value]"
	    );

	    NetSendLiveMessage(
                condescriptor,
 "<levels> can be: DEBUG_LEVEL_NONE = 0, DEBUG_LEVEL_ALL = 1"
            );

            NetSendLiveMessage(
		condescriptor,
 "DEBUG_LEVEL_MEMORY = 2, DEBUG_LEVEL_NETWORK = 3"
            );

            return(0);
        }

	/* Only root can set debug. */
	if(con_obj_ptr->permission.uid > 0)
	{
	    sprintf(
		sndbuf,
		"Permission denied: Access level %i required.",
		0
	    );
	    NetSendLiveMessage(condescriptor, sndbuf);

	    return(-1);
	}


	/* Parse debug value argument. */
        strptr = (char *) strchr(arg, '=');
        if(strptr != NULL)
        {
	    strptr++;
	    while(ISBLANK(*strptr))
		strptr++;

            debug.val = atof(strptr);
        }

	/* Parse debug level. */
	strncpy(num_str, arg, 80);
	num_str[80 - 1] = '\0';

	strptr = strchr(num_str, '=');
	if(strptr != NULL)
	    *strptr = '\0';

	strptr = num_str;
	while(ISBLANK(*strptr))
	    strptr++;

        debug.level = atoi(strptr);


        sprintf(
	    text,
	    "debug: level %i: value %.4f",
            debug.level, debug.val
        );
        NetSendLiveMessage(condescriptor, text);


	return(0);
}
