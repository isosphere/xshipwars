#include "../include/unvmatch.h"
#include "../include/swnetcodes.h"
#include "swserv.h"
#include "net.h"




int NetSendFoward(int condescriptor, char *url)
{
	char sndbuf[CS_DATA_MAX_LEN];


	if(url == NULL)
	    return(-1);

	/*
	 *   CS_CODE_FORWARD forward:
	 *
	 *	url
	 */
	sprintf(sndbuf,
"%i %s\n",
	    CS_CODE_FORWARD,
	    url
	);
	NetDoSend(condescriptor, sndbuf);


	return(0);
}

int NetHandleELinkEnter(int condescriptor, char *arg)
{
        int src_obj, tar_obj;
	xsw_object_struct *tar_obj_ptr;


        /*
         *   SWEXTCMD_ELINKENTER format:
         *
         *      src_obj tar_obj
         */
        sscanf(arg, "%i %i",
		&src_obj,
		&tar_obj
        );

	if(DBIsObjectGarbage(tar_obj))
	    return(-1);
	else
	    tar_obj_ptr = xsw_object[tar_obj];

	/* Check if object is a elink. */
	if(tar_obj_ptr->type != XSW_OBJ_TYPE_ELINK)
	    return(-2);

	/* Set forward to url. */
	NetSendFoward(condescriptor, tar_obj_ptr->elink);


        return(0);
}
