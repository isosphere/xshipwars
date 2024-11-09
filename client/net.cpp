/*
                       Network Connection Functions

 	Functions:

 	int NetOpenConnection(char *url)
 	void NetResetParms()

	---
 */

#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <sys/types.h>
extern int h_errno;		/* For netdb.h. */
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include "../include/netio.h"

#include "xsw.h"
#include "net.h"


/*
 *	Opens a connection, returns socket number or -1 on error.
 */
int NetOpenConnection(char *host, int port)
{
	char stringa[MAX_URL_LEN + 512];

        int new_sock;	               /* socket number */
	struct hostent *he;            /* Host pointer. */
	struct sockaddr_in haddr;


	if((host == NULL) ||
           (port < 0)
	)
	    return(-1);


	/* ************************************************************* */
	/* Begin connecting. */

	/* Reset all global net parameters. */
	NetResetParms();


	/* Get new hostname. */
	he = gethostbyname(host);
	if(he == NULL)
	{
	    switch(h_errno)
	    {
              case HOST_NOT_FOUND:
	        sprintf(stringa,
"Cannot find host:\n\n\
    %s\n\n\
Please verify that you have the correct host address\n\
and that you have network access.\n",
                    host
                );
                printdw(&err_dw, stringa);
		break;

	      case NO_ADDRESS:
                sprintf(stringa,
"Cannot obtain IP address for host:\n\n\
    %s\n\n\
Please verify that you have the correct host address\n\
and that you have network access.\n",
                    host
                );
                printdw(&err_dw, stringa);
                break;
 
              case NO_RECOVERY:
                sprintf(stringa,
"A non-recoverable name server error occurred\n\
while trying to obtain host information for:\n\n\
    %s\n\n\
Please verify that you have the correct host address\n\
and that you have network access.\n",
                    host
                );
                printdw(&err_dw, stringa);
                break;

             default:
                sprintf(stringa,
"Cannot resolve host:\n\n\
    %s\n\n\
Please verify that you have the correct host address\n\
and that you have network access.\n",
                    host
                );
                printdw(&err_dw, stringa);
                break;
	    }

	    return(-1);
	}

	/* Get new socket. */
	new_sock = socket(AF_INET, SOCK_STREAM, 0);
	if(new_sock < 0)
	{
            MesgAdd("socket(): Cannot open socket.", xsw_color.standard_text);

            sprintf(stringa,
"Unable to create a socket.\n\
There may be too many sockets currently opened or an\n\
internal systems problem.\n\
Verify that your operating system has network\n\
capabilities.\n"
            );
            printdw(&err_dw, stringa);

            return(-1);
	}

	haddr.sin_family = AF_INET;		/* In hbo. */
	haddr.sin_port = htons(port);		/* In nbo. */
	haddr.sin_addr = *((struct in_addr *)he->h_addr);
	memset(&(haddr.sin_zero), 0x00, 8);	/* Zero the rest of struct. */

	/* Open connection. */
	if(connect(new_sock, (struct sockaddr *)&haddr,
		sizeof(struct sockaddr)) == -1)
	{
            MesgAdd("connect(): Cannot connect TCP socket.",
                xsw_color.standard_text
	    );

            sprintf(stringa,
"Unable to connect to:\n\n\
    %s  Port: %i\n\n\
Please verify that the address and port number are correct.\n\
Also make sure that the site you are connecting to has a\n\
server running and listening on the specified port number.\n\n",
		host, port
            );
            printdw(&err_dw, stringa);

            return(-1);
	}


	/* Set socket to non-blocking. */
        fcntl(new_sock, F_SETFL, O_NONBLOCK);


	/* ************************************************************* */

	/* Set up network parameters. */
	net_parms.socket = new_sock;

	/* Mark connection state as loggin in. */
	net_parms.connection_state = CON_STATE_NEGOTIATING;


	/* Mark connection start time. */
	net_parms.con_start = time(NULL);


        /* Return the new socket number. */
        return(new_sock);
}


/*
 *	Procedure to reset network parameters.
 *
 *	If connected, socket will be force closed.
 *	No notification will be sent to the server.
 */
void NetResetParms()
{
	/* Set connection state to not connected. */
        net_parms.connection_state = CON_STATE_NOT_CONNECTED;

	/* Close socket to server if it is still connected. */
	if(net_parms.socket > -1)
	{
/* Print disconnect message? */
	    close(net_parms.socket);
	    net_parms.socket = -1;
	}

	net_parms.player_obj_ptr = NULL;

/*	Do not change, this was set properly at startup.
	net_parms.is_address_set
*/

	net_parms.con_start = 0;

	net_parms.warn15 = 0;
	net_parms.warn30 = 0;
	net_parms.warn45 = 0;

	net_parms.login_got_lplayer = 0;
        net_parms.login_got_position = 0;
        net_parms.login_got_sector = 0;

/*      Do not change, remember last address and port.
        net_parms.address
        net_parms.port
*/

        net_parms.net_int = SERVER_DEF_INT;
        net_parms.disconnect_send_count = 0;
        net_parms.bad_send_count = 0;

	memset(net_parms.co_data, 0x00, CS_DATA_MAX_LEN);
        net_parms.co_data_len = 0;


	return;
}
