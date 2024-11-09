#include "../include/unvmath.h"
#include "../include/unvmatch.h"
#include "swserv.h"
#include "net.h"


#define THIS_CMD_NAME	"link"


int CmdLink(int condescriptor, const char *arg)
{
        char *strptr;
	int src_obj_num, tar_obj_num, con_obj_num;
	xsw_object_struct *src_obj_ptr, *tar_obj_ptr, *con_obj_ptr;
	connection_struct *con_ptr;

        char sndbuf[CS_DATA_MAX_LEN];
        char tar_obj_name[XSW_OBJ_NAME_MAX + 80];
        char src_obj_name[XSW_OBJ_NAME_MAX + 80];
	char con_obj_name[XSW_OBJ_NAME_MAX + 80];

	char text[512 + (XSW_OBJ_NAME_MAX * 3)];


        /* Get connection's object number (assumed valid). */
        con_ptr = connection[condescriptor];
        con_obj_num = con_ptr->object_num;
        con_obj_ptr = xsw_object[con_obj_num];


        /* Get formal name of connection's object. */
        strncpy(
            con_obj_name,
            DBGetFormalNameStr(con_obj_num),
            XSW_OBJ_NAME_MAX + 80
        );
        con_obj_name[XSW_OBJ_NAME_MAX + 80 - 1] = '\0';


	/* If argument is empty, then print usage. */
        if((arg == NULL) ? 1 : (*arg == '\0'))
        {
            sprintf(
		sndbuf,
                "Usage: `%s <object>[=<destination>]'",
		THIS_CMD_NAME
            );
            NetSendLiveMessage(condescriptor, sndbuf);

            return(-1);
        }


        /* Parse source object name. */
	strncpy(src_obj_name, arg, XSW_OBJ_NAME_MAX);
	src_obj_name[XSW_OBJ_NAME_MAX - 1] = '\0';

	/* Remove anything past the '=' character. */
	strptr = strchr(src_obj_name, '=');
	if(strptr != NULL)
            *strptr = '\0';

	StringStripSpaces(src_obj_name);

	/* Match source object name. */
	if(!strcasecmp(src_obj_name, "me"))
	    src_obj_num = con_obj_num;
	else
	    src_obj_num = MatchObjectByName(
	        xsw_object, total_objects,
	        src_obj_name, -1
	    );
        if(DBIsObjectGarbage(src_obj_num))
        {
            sprintf(
		sndbuf,
                "%s: %s: No such object.",
		THIS_CMD_NAME,
                src_obj_name
            );
            NetSendLiveMessage(condescriptor, sndbuf);
        
            return(-1);
        }
	else
	{
	    src_obj_ptr = xsw_object[src_obj_num];
	}

	/* Get formal name of source object. */
        strncpy(
            src_obj_name,
            DBGetFormalNameStr(src_obj_num),
            XSW_OBJ_NAME_MAX + 80
        );
        src_obj_name[XSW_OBJ_NAME_MAX + 80 - 1] = '\0';


	/* Connection owns or can set source object? */
	if(ACCESS_UID_SET < con_obj_ptr->permission.uid)
	{
            sprintf(
		sndbuf,
		"%s: Permission denied: Requires access level %i.",
                THIS_CMD_NAME,
                ACCESS_UID_SET
            );
            NetSendLiveMessage(condescriptor, sndbuf);
            return(-1);
	}
	if((con_obj_ptr->owner != con_obj_num) &&
	   (ACCESS_UID_SETO < con_obj_ptr->permission.uid)
	)
	{
            sprintf(
		sndbuf,
		"%s: Permission denied: Requires access level %i.",
		THIS_CMD_NAME,
		ACCESS_UID_SETO
            );
            NetSendLiveMessage(condescriptor, sndbuf);
            return(-1);
	}
	if(src_obj_ptr->permission.uid < con_obj_ptr->permission.uid)
	{
            sprintf(
		sndbuf,
		"%s: Permission denied: %s access level %i exceeds yours.",
		THIS_CMD_NAME,
		src_obj_name,
		src_obj_ptr->permission.uid
            );
            NetSendLiveMessage(condescriptor, sndbuf);
            return(-1);
	}



	/* Link by source object type. */
	switch(src_obj_ptr->type)
	{
          /* ****************************************************** */
	  case XSW_OBJ_TYPE_WORMHOLE:
            /* Parse target object. */
            strptr = (char *) strchr(arg, '=');
            if(strptr == NULL)
            {
                *tar_obj_name = '\0';
                tar_obj_num = -1;
                tar_obj_ptr = NULL;
            }
            else
            {
                strncpy(
                    tar_obj_name,
                    strptr + 1,
                    XSW_OBJ_NAME_MAX
                );
                tar_obj_name[XSW_OBJ_NAME_MAX - 1] = '\0';
                StringStripSpaces(tar_obj_name);


                /* Match target object. */
                if(!strcasecmp(tar_obj_name, "me"))
                    tar_obj_num = con_obj_num;
                else
                    tar_obj_num = MatchObjectByName(
                        xsw_object, total_objects,
                        tar_obj_name, -1
                    );
                if(DBIsObjectGarbage(tar_obj_num))
		{
		    sprintf(
                        sndbuf,
                        "%s: No such object.",
                        tar_obj_name
                    );
                    NetSendLiveMessage(condescriptor, sndbuf);
                    return(-1);
		}
                else
		{
                    tar_obj_ptr = xsw_object[tar_obj_num];
		}

                /* Get formal name of target object. */
                strncpy(
                    tar_obj_name,
                    DBGetFormalNameStr(tar_obj_num),
                    XSW_OBJ_NAME_MAX + 80
                );
                tar_obj_name[XSW_OBJ_NAME_MAX + 80 - 1] = '\0';
            }

	    /* Link or unlink? */
	    if(tar_obj_ptr == NULL)
	    {
		/* Unlink. */
		src_obj_ptr->intercepting_object = -1;

		/* Reset attitude. */
		src_obj_ptr->heading = 0;
		src_obj_ptr->pitch = 0;
		src_obj_ptr->bank = 0;

                MuSetUnitVector2D(
                    &src_obj_ptr->attitude_vector_compoent,
                    src_obj_ptr->heading
                );


		/* Print response and log. */
		sprintf(
                    sndbuf,
                    "%s: Unlinked.",
                    src_obj_name
                );
                NetSendLiveMessage(condescriptor, sndbuf);

		sprintf(
		    text,
		    "%s: Unlinked %s.",
                    con_obj_name, src_obj_name
		);
		if(sysparm.log_general)
		    LogAppendLineFormatted(fname.primary_log, text);
	    }
	    else
	    {
		/* Link to. */
		src_obj_ptr->intercepting_object = tar_obj_num;

                /* Calculate source object heading to target object. */
		if(Mu3DInSameSectorPtr(
                    src_obj_ptr, tar_obj_ptr
		))
		{
                    src_obj_ptr->heading = MuCoordinateDeltaVector(
			tar_obj_ptr->x - src_obj_ptr->x,
			tar_obj_ptr->y - src_obj_ptr->y
		    );
                    src_obj_ptr->pitch = 0;
                    src_obj_ptr->bank = 0;

                    MuSetUnitVector2D(
                        &src_obj_ptr->attitude_vector_compoent,
                        src_obj_ptr->heading 
                    );
		}
		else
		{
                    src_obj_ptr->heading = MuCoordinateDeltaVector(
                        tar_obj_ptr->sect_x - src_obj_ptr->sect_x,
                        tar_obj_ptr->sect_y - src_obj_ptr->sect_y
                    );
                    src_obj_ptr->pitch = 0;
                    src_obj_ptr->bank = 0;

                    MuSetUnitVector2D(
                        &src_obj_ptr->attitude_vector_compoent,
                        src_obj_ptr->heading
                    );
		}

                /* Print response and log. */
		sprintf(
                    sndbuf,
                    "%s: Linked to %s.",
                    src_obj_name, tar_obj_name
                );
                NetSendLiveMessage(condescriptor, sndbuf);

                sprintf(
                    text,
                    "%s: Linked %s to %s.",
                    con_obj_name, src_obj_name, tar_obj_name
                );
                if(sysparm.log_general)
                    LogAppendLineFormatted(fname.primary_log, text);
	    }
	    break;

          /* ****************************************************** */
	  case XSW_OBJ_TYPE_ELINK:
            /* Unlink first. */
            free(src_obj_ptr->elink); 
            src_obj_ptr->elink = NULL;

            /* Parse target. */
            strptr = (char *) strchr(arg, '=');
            if(strptr == NULL) 
            {
		/* Unlink, already unlinked. */

                /* Print response and log. */
                sprintf(
                    sndbuf,
                    "%s: Unlinked.",
                    src_obj_name
                );
                NetSendLiveMessage(condescriptor, sndbuf);

                sprintf(
                    text,
                    "%s: Unlinked %s.",
                    con_obj_name, src_obj_name
                );
                if(sysparm.log_general)
                    LogAppendLineFormatted(fname.primary_log, text);
	    }
	    else
	    {
		/* Set new link. */
                src_obj_ptr->elink = StringCopyAlloc(strptr + 1);
		if(src_obj_ptr->elink == NULL)
		{
                    sprintf(   
                        sndbuf,
                        "%s: Memory allocation error.",
			THIS_CMD_NAME
		    );
		    NetSendLiveMessage(condescriptor, sndbuf);
		    return(-1);
		}
		StringStripSpaces(src_obj_ptr->elink);

                /* Print response and log. */
                sprintf(
                    sndbuf,
                    "%s: Linked to `%s'.",
                    src_obj_name, src_obj_ptr->elink
                );
                NetSendLiveMessage(condescriptor, sndbuf);

                sprintf(
                    text,
                    "%s: Linked %s to `%s'.",
                    con_obj_name, src_obj_name, src_obj_ptr->elink
                );
                if(sysparm.log_general)
                    LogAppendLineFormatted(fname.primary_log, text);
	    }
	    break;

	  /* ****************************************************** */
	  default:
	    sprintf(
                sndbuf,
                "%s: Permission denied: Not a linkable object type.",
                src_obj_name
            );
            NetSendLiveMessage(condescriptor, sndbuf);
            return(-1);
	    break;
        } 


        return(0);
}
