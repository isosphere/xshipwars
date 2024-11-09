#include "../include/unvmatch.h"
#include "swserv.h"
#include "net.h"
#include "siteban.h"


#define THIS_CMD_NAME	"siteban"


/*
 *	Siteban list editing.
 */
int CmdSiteBan(int condescriptor, const char *arg)
{
	int i, c, op;
	char *strptr;

	int con_obj_num;
	xsw_object_struct *con_obj_ptr;
	connection_struct *con_ptr;

	siteban_ip_union ip;
	siteban_struct **ptr;

	char sndbuf[CS_DATA_MAX_LEN];
        char ip_str[CS_DATA_MAX_LEN];


	/* Get connection's object number (assumed valid). */
	con_ptr = connection[condescriptor];
	con_obj_num = con_ptr->object_num;
	con_obj_ptr = xsw_object[con_obj_num];


	if(con_obj_ptr->permission.uid > ACCESS_UID_SITEBAN)
	{
            sprintf(sndbuf,
                "%s: Permission denied: Requires access level %i.",
		THIS_CMD_NAME,
		ACCESS_UID_SITEBAN
            );
            NetSendLiveMessage(condescriptor, sndbuf);
            return(-1);
	}


	/* Get operation and IP string. */
	if(arg == NULL)
	{
	    op = 0;
	    *ip_str = '\0';
	}
	else
	{
	    while(*arg == ' ')
		arg++;

	    /* Parse op. */
	    if((*arg == '+') ||
	       (tolower(*arg) == 'a')
	    )
		op = 1;
	    else if((*arg == '-') ||
                    (tolower(*arg) == 'd') ||
                    (tolower(*arg) == 'r')
            )
                op = 2;
            else
                op = 0;

	    /* If op is 1 or greater, get ip string. */
	    if(op > 0)
	    {
		/* Skip untill next argument if any. */
		while((*arg != ' ') && (*arg != '\0'))
                    arg++;
                while((*arg == ' ') && (*arg != '\0'))
                    arg++;

		/* Get ip string. */
		strncpy(
		    ip_str,
		    arg,
		    CS_DATA_MAX_LEN
		);
		ip_str[CS_DATA_MAX_LEN - 1] = '\0';

		/* Get rid of first space in ip_str if any. */
		strptr = strchr(ip_str, ' ');
		if(strptr != NULL)
		    *strptr = '\0';
	    }
	    else
	    {
		*ip_str = '\0';
	    }
	}


	/* Check which operation to perform. */
	switch(op)
	{
	  case 1:	/* Add entry. */
	    if(*ip_str == '\0')
	    {
                sprintf(
                    sndbuf,
                    "Usage: `%s [+|-] [ip]'",
                    THIS_CMD_NAME
                );
                sndbuf[CS_DATA_MAX_LEN - 1] = '\0';
                NetSendLiveMessage(condescriptor, sndbuf);
	    }
	    else
	    {
	        StringParseIP(
		    ip_str,
		    &(ip.part_u8[0]),
                    &(ip.part_u8[1]),
                    &(ip.part_u8[2]),
                    &(ip.part_u8[3])
	        );
	        if(SiteBanAdd(&ip, 0) < 0)
	            sprintf(sndbuf,
 "%s: Error adding IP `%i.%i.%i.%i'.",
		        THIS_CMD_NAME,
		        ip.part_u8[0],
		        ip.part_u8[1],
		        ip.part_u8[2],
		        ip.part_u8[3]
		    );
	        else
                    sprintf(sndbuf,
 "%s: Added IP `%i.%i.%i.%i' to siteban list.", 
                        THIS_CMD_NAME,
                        ip.part_u8[0],
                        ip.part_u8[1],
                        ip.part_u8[2],
                        ip.part_u8[3]
                    );
                sndbuf[CS_DATA_MAX_LEN - 1] = '\0';
                NetSendLiveMessage(condescriptor, sndbuf);
	    }
	    break;

          case 2:       /* Remove entry. */
            if(*ip_str == '\0')
            {
                sprintf(
                    sndbuf,
                    "Usage: `%s [+|-] [ip]'",
                    THIS_CMD_NAME
                );
                sndbuf[CS_DATA_MAX_LEN - 1] = '\0';
                NetSendLiveMessage(condescriptor, sndbuf);
            }
            else
 	    {
                StringParseIP(
                    ip_str,
                    &(ip.part_u8[0]),  
                    &(ip.part_u8[1]), 
                    &(ip.part_u8[2]), 
                    &(ip.part_u8[3])
                );
                if(SiteBanRemoveIP(&ip) < 0)
                    sprintf(sndbuf,
 "%s: No such IP `%i.%i.%i.%i' in site ban list.",
                        THIS_CMD_NAME,
                        ip.part_u8[0],
                        ip.part_u8[1],
                        ip.part_u8[2],
                        ip.part_u8[3]
                    );
                else
                    sprintf(sndbuf,
 "%s: Removed IP `%i.%i.%i.%i' from siteban list.",
                        THIS_CMD_NAME,
                        ip.part_u8[0],
                        ip.part_u8[1],
                        ip.part_u8[2],
                        ip.part_u8[3]
                    );
                sndbuf[CS_DATA_MAX_LEN - 1] = '\0';
                NetSendLiveMessage(condescriptor, sndbuf);
	    }
            break;

	  default:	/* List siteban IPs. */
	    for(i = 0, c = 0, ptr = siteban;
                i < total_sitebans;
                i++, ptr++
	    )
	    {
		if(*ptr == NULL)
		    continue;

                sprintf(sndbuf,
                    "%i.%i.%i.%i",
                    (*ptr)->ip.part_u8[0],
                    (*ptr)->ip.part_u8[1],
                    (*ptr)->ip.part_u8[2],
                    (*ptr)->ip.part_u8[3]
                );
                sndbuf[CS_DATA_MAX_LEN - 1] = '\0';
                NetSendLiveMessage(condescriptor, sndbuf);

		c++;
	    }
            sprintf(   
                sndbuf,
                "*** %i sitebanned %s ***",
                c,
		((c == 1) ? "address" : "addresses")
            );
            sndbuf[CS_DATA_MAX_LEN - 1] = '\0';
            NetSendLiveMessage(condescriptor, sndbuf);
            sprintf(
		sndbuf,
		"Usage: `%s [+|-] [ip]'",
		THIS_CMD_NAME
            );
            sndbuf[CS_DATA_MAX_LEN - 1] = '\0';
            NetSendLiveMessage(condescriptor, sndbuf);
	    break;
	}


	return(0);
}
