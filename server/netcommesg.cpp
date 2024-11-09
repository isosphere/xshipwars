#include "../include/unvmatch.h"
#include "../include/unvmath.h"
#include "../include/swnetcodes.h"
#include "swserv.h"
#include "net.h"


int NetHandleComMesg(int condescriptor, char *arg)
{
	char *strptr;
	int object_count;

	int src_obj;
	int tar_obj;
	double bearing;
	int channel;

        int tar_con_num;
        xsw_object_struct *src_obj_ptr;

        char text[CS_MESG_MAX + 256];
        char larg[CS_DATA_MAX_LEN + 256];
        char message[CS_MESG_MAX + 256];



        strncpy(larg, arg, CS_DATA_MAX_LEN);
        larg[CS_DATA_MAX_LEN - 1] = '\0';


        /*
         *   SWEXTCMD_COMMESSAGE format:
         *
         *      "src_obj tar_obj bearing channel;message"
         */
	strptr = strchr(larg, CS_STRING_DELIMINATOR_CHAR);
	if(strptr == NULL)
	    return(-1);

	strncpy(message, strptr + 1, CS_MESG_MAX);
	message[CS_MESG_MAX - 1] = '\0';
	*strptr = '\0';

        sscanf(larg,
		"%i %i %lf %i",

                &src_obj,	/* Object sending the message. */
		&tar_obj,	/* Object to recieve or -1 for any. */
		&bearing,
		&channel
        );

	/* Connection must control source object. */
	if(connection[condescriptor]->object_num != src_obj)
	    return(-3);

        /* Need valid source object pointer. */
        if(DBIsObjectGarbage(src_obj))
            return(-1);
        else
            src_obj_ptr = xsw_object[src_obj];


        for(object_count = 0; object_count < total_objects; object_count++)
        {
	    /* Object not valid or not in range? */
            if(!Mu3DInRange(
		xsw_object, total_objects,
		src_obj, object_count, COM_SHORT_RANGE_MAX
	    ))
                continue;

            /* Is tar_obj specified? */
            if((tar_obj > -1) &&
               (tar_obj != object_count)
            )
            {   
                /* Skip since message is not ment for object_count. */
                continue;
            }

	    /* Skip of object_count is not listening on channel. */
	    if(xsw_object[object_count]->com_channel != channel)
		continue;


            /* Get tar_con_num of object_count */
            tar_con_num = ConGetByObject(object_count);
            if(tar_con_num < 0)
		continue;

            /* Get bearing, object_count to source object. */
            bearing = MuCoordinateDeltaVector(
                src_obj_ptr->x - xsw_object[object_count]->x,
                src_obj_ptr->y - xsw_object[object_count]->y 
            );


            /* Send message to that connection. */
            NetSendComMesg(
                tar_con_num,
                src_obj,	/* Object sending message. */
                tar_obj,	/* Object to recv it or -1 for any. */
                bearing,	/* target to source. */
		channel,
                message
            );
	}


        /* Log message. */
        sprintf(
	    text,
	    "%s: Channel %.2f: Message: \"%s\"",
            DBGetFormalNameStr(src_obj),
	    (double)((double)channel / 100),
            message
        );
        if(sysparm.log_net)
            LogAppendLineFormatted(fname.primary_log, text);


        return(0);
}


int NetSendComMesg(
	int condescriptor,
	int src_obj, int tar_obj,
	double bearing, int channel,
	char *message
)
{
        char sndbuf[CS_DATA_MAX_LEN];
	char lmesg[CS_MESG_MAX];


	if(message == NULL)
	    return(-1);

	strncpy(lmesg, message, CS_MESG_MAX);
	lmesg[CS_MESG_MAX - 1] = '\0';


        /*
         *   SWEXTCMD_COMMESSAGE format:
         *
         *      src_obj, tar_obj, bearing, channel;message
         */
        sprintf(sndbuf,
"%i %i\
 %i %i %.4f %i;%s\n",
                CS_CODE_EXT,
                SWEXTCMD_COMMESSAGE,

                src_obj,
                tar_obj,
		bearing,
		channel,
		lmesg
        );
        NetDoSend(condescriptor, sndbuf);


        return(0);
}
