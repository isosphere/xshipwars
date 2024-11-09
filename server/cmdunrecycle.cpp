#include "../include/unvmatch.h"
#include "../include/xsw_ctype.h"
#include "swserv.h"
#include "net.h"


#define THIS_CMD_NAME	"unrecycle"


int CmdUnrecycle(int condescriptor, const char *arg)
{
        int obj_num, con_obj_num, owner;
        xsw_object_struct *obj_ptr, *con_obj_ptr;
        connection_struct *con_ptr;

        char name1[XSW_OBJ_NAME_MAX + 80];
        char name2[XSW_OBJ_NAME_MAX + 80];

        char sndbuf[CS_DATA_MAX_LEN];
	char text[(2 * XSW_OBJ_NAME_MAX) + 512];


        /* Get connection's object number (assumed valid). */
        con_ptr = connection[condescriptor];
        con_obj_num = con_ptr->object_num;
        con_obj_ptr = xsw_object[con_obj_num];


        /* Print listing of objects in recycle backup buffer? */
        if((arg == NULL) ? 1 : (*arg == '\0'))
        {
	    int i, n, len;


	    /* Print list of objects. */
            for(i = 0, n = 0; i < total_recycled_objects; i++)
            {
		obj_ptr = recycled_xsw_object[i];
                if(obj_ptr == NULL)
                    continue;

                if(obj_ptr->type <= XSW_OBJ_TYPE_GARBAGE)
                    continue;

		if(n == 0)
		{
		    /* First item found, print heading. */
		    NetSendLiveMessage(
			condescriptor,
"Name                     Owner"
		    );
		}

		owner = obj_ptr->owner;

		strcpy(text, obj_ptr->name);
		len = strlen(text);
		strpad(&text[len], 25 - len);

		strcpy(&text[25], DBGetFormalNameStr(owner));
                NetSendLiveMessage(condescriptor, text);

                n++;
            }
	    if(n > 0)
		sprintf(
		    sndbuf,
                    "%i object%s in recycle backup buffer.",
                    n,
		    ((n == 1) ? "s" : "")
		);
	    else
		sprintf(
		    sndbuf,
		    "No objects in recycle backup buffer."
		);
            NetSendLiveMessage(condescriptor, sndbuf);

            sprintf(
		sndbuf,
                "Usage: `%s <search_string>'",
		THIS_CMD_NAME
            );
            NetSendLiveMessage(condescriptor, sndbuf);

            return(0);
        }


	/* Skip leading spaces. */
	while(ISBLANK(*arg))
	    arg++;

        /* Attempt to recover object. */
        obj_num = DBRecoverRecycledObject(
	    arg,
	    con_obj_num		/* Owner. */
	);

        /* Did we recover anything? */
        if(DBIsObjectGarbage(obj_num))
        {
            sprintf(
		sndbuf,
                "%s: Not found in recycle backup buffer.",
                arg
            );
            NetSendLiveMessage(condescriptor, sndbuf);

            return(-1);
        }


	/* Get object names. */
        strncpy(
	    name1,
	    DBGetFormalNameStr(con_obj_num),
	    XSW_OBJ_NAME_MAX + 80
	);
        name1[XSW_OBJ_NAME_MAX + 80 - 1] = '\0';
        strncpy(
	    name2,
	    DBGetFormalNameStr(obj_num),
	    XSW_OBJ_NAME_MAX + 80
	);
        name2[XSW_OBJ_NAME_MAX + 80 - 1] = '\0';

        /* Print response. */
        sprintf(
	    sndbuf,
            "Succesfully recovered %s",
            name2
        );
        NetSendLiveMessage(condescriptor, sndbuf);

        /* Log recovery. */
        sprintf(
	    text,
	    "%s: Recovered %s from recycle backup buffer.",
            name1, name2
        );
        if(sysparm.log_general)
            LogAppendLineFormatted(fname.primary_log, text);


        /* Send updates to all connections. */
        NetSendCreateObject(-1, obj_num);
        NetSendObjectName(-1, obj_num);
        NetSendObjectSect(-1, obj_num);


        return(0);
}
