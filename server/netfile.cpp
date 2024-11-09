/*
                      Network File Names (not file IO)

	Functions:

	int NetHandleSetImageSet(int condescriptor, char *arg)
        int NetHandleSetSoundSet(int condescriptor, char *arg) 
        int NetHandleSetOCSN(int condescriptor, char *arg)

	int NetSendSetImageSet(int condescriptor, char *filename)
	int NetSendSetSoundSet(int condescriptor, char *filename)
	int NetSendSetOCSN(int condescriptor, char *filename)

	---

 */

#include "../include/unvmatch.h"
#include "../include/swnetcodes.h"
#include "swserv.h"
#include "net.h"


int NetHandleSetImageSet(int condescriptor, char *arg)
{
	NetSendSetImageSet(condescriptor, unv_head.isr);

	return(0);
}

int NetHandleSetSoundSet(int condescriptor, char *arg)
{
	NetSendSetSoundSet(condescriptor, unv_head.ss);

        return(0);
}

int NetHandleSetOCSN(int condescriptor, char *arg)
{
	NetSendSetOCSN(condescriptor, unv_head.ocsn);

        return(0);
}



int NetSendSetImageSet(int condescriptor, char *filename)
{
	char lname[CS_DATA_MAX_LEN];
        char sndbuf[CS_DATA_MAX_LEN];

#if (CS_DATA_MAX_LEN < 64)
#warning "CS_DATA_MAX_LEN is defined too small."
#endif

	if(filename == NULL)
	    return(-1);
	if(*filename == '\0')
	    return(-1);


	strncpy(lname, filename, CS_DATA_MAX_LEN);
	/* Null terminate with a margin of 64 chars. */
	lname[CS_DATA_MAX_LEN - 64] = '\0';

	/*
	 *   Format CS_CODE_IMAGESET:
	 *
	 *	path
	 */
	sprintf(sndbuf, "%i %s\n",
		CS_CODE_IMAGESET,
		lname
	);
	NetDoSend(condescriptor, sndbuf);


        return(0);
}

int NetSendSetSoundSet(int condescriptor, char *filename)
{
        char lname[CS_DATA_MAX_LEN];
        char sndbuf[CS_DATA_MAX_LEN];

#if (CS_DATA_MAX_LEN < 64)
#warning "CS_DATA_MAX_LEN is defined too small."
#endif

        if(filename == NULL)
            return(-1);
        if(*filename == '\0')
            return(-1);


        strncpy(lname, filename, CS_DATA_MAX_LEN);
        /* Null terminate with a margin of 64 chars. */  
        lname[CS_DATA_MAX_LEN - 64] = '\0';


        /*
         *   Format CS_CODE_SOUNDSET:
         *
         *      path
         */
        sprintf(sndbuf, "%i %s\n",
                CS_CODE_SOUNDSET,
                lname
        );
        NetDoSend(condescriptor, sndbuf);


        return(0);
}

int NetSendSetOCSN(int condescriptor, char *filename)
{
        char lname[CS_DATA_MAX_LEN];
        char sndbuf[CS_DATA_MAX_LEN];
           
#if (CS_DATA_MAX_LEN < 64)
#warning "CS_DATA_MAX_LEN is defined too small."
#endif
            
        if(filename == NULL)
            return(-1);
        if(*filename == '\0')
            return(-1);
        
        
        strncpy(lname, filename, CS_DATA_MAX_LEN);
        /* Null terminate with a margin of 64 chars. */
        lname[CS_DATA_MAX_LEN - 64] = '\0';


        /*
         *   Format SWEXTCMD_SETOCSN:
         *
         *      path
         */
        sprintf(sndbuf, "%i %i %s\n",
		CS_CODE_EXT,
                SWEXTCMD_SETOCSN,
                lname
        );
        NetDoSend(condescriptor, sndbuf);


        return(0);
}
