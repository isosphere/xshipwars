/*
                Incoming connections listning sockets

	Functions:

	int IncomingSocketIsAllocated(int n)

	int IncomingSocketInit(
	        int port_num,
	        int type
	)
	void IncomingSocketDelete(int n)
	void IncomingSocketDeleteAll()

	void IncomingSocketReclaim()

	---

	Managements (handling incoming connections) is handled
	by function NetManageNewConnections() and
	AUXConManageNewConnections().

 */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "swserv.h"


int IncomingSocketIsAllocated(int n)
{
	if((n < 0) || (n >= total_incoming_sockets))
	    return(0);
	else if(incoming_socket[n] == NULL)
	    return(0);
	else
	    return(1);
}


/*                  
 *      Allocates a new socket structure and binds the socket
 *      to the given port number.  If another incoming socket
 *      structure has a matching port number, then nothing
 *      will be done.
 */
int IncomingSocketInit(
	int port_num,
	int type
)
{
        int i, n, status;
        incoming_socket_struct *is_ptr;
        struct sockaddr_in addr;
        
        
        if(port_num < 0)
        {
            fprintf(stderr,
                "IncomingSocketInit(): Port %i is negative.\n",
                port_num
            );
            return(-1);
        }
            
              
        if(total_incoming_sockets < 0)
            total_incoming_sockets = 0;
        

        /*  Check if there is already an incoming socket listening
         *  on port_num.
         */
        for(i = 0; i < total_incoming_sockets; i++)
        {
            if(incoming_socket[i] == NULL)
                continue;

            if(incoming_socket[i]->port_num == port_num)
                return(0);
        }
 
   
        /* Look for available pointer. */
        for(i = 0; i < total_incoming_sockets; i++)
        {
            if(incoming_socket[i] == NULL)
                break;
        }
        if(i < total_incoming_sockets)
        {
            /* Found available pointer. */
            n = i;
        }
        else
        {
            /* Need to allocate more pointers. */
            n = total_incoming_sockets;
            total_incoming_sockets++;
              
            incoming_socket = (incoming_socket_struct **)realloc(
                incoming_socket,
                total_incoming_sockets * sizeof(incoming_socket_struct *)
            );
            if(incoming_socket == NULL)
            {
                total_incoming_sockets = 0;
                return(-1);
            }
        }
                
            
        incoming_socket[n] = (incoming_socket_struct *)calloc(
            1,
            sizeof(incoming_socket_struct)
        );
        if(incoming_socket[n] == NULL)   
        {
            return(-1);
        }
                
         
        /* Set up values. */
        is_ptr = incoming_socket[n];

	is_ptr->type = type;
        is_ptr->port_num = port_num;
        is_ptr->start_time = time(NULL);

         
        /* ************************************************** */
        /* Begin attempting to bind listening port. */
            
        is_ptr->socket = -1;

        for(i = 0; i < BIND_RETRIES; i++)
        {
            /* Get socket as needed. */
            if(is_ptr->socket < 0)
            {   
                is_ptr->socket = socket(AF_INET, SOCK_STREAM, 0);
                if(is_ptr->socket < 0)
                {
                    fprintf(stderr,
 "IncomingSocketInit: Unable to get socket, retrying...\n"
                    );

                    is_ptr->socket = -1;

                    sleep(BIND_RETRY_INTERVAL);
		    continue;
                }
            } 
 
            /* Reset address structure. */
            addr.sin_family = AF_INET;
            addr.sin_port = htons(is_ptr->port_num);
            addr.sin_addr.s_addr = INADDR_ANY;
            memset(&addr.sin_zero, 0x00, 8);

            /* Bind the socket. */
            status = bind(  
                is_ptr->socket,
                (struct sockaddr *)&addr,
                sizeof(struct sockaddr) 
            );
            if(status == -1)
            {
                fprintf(stderr,
 "IncomingSocketInit: Unable to bind() socket to port %i, retrying...\n",
		    is_ptr->port_num
                );

                sleep(BIND_RETRY_INTERVAL);
                continue;
            }
                
            /* Set up listening and backlogged connection limit. */
            if(listen(is_ptr->socket, LISTEN_SOCKET_BACKLOG) == -1)
            {
                fprintf(stderr,
                    "Server: Unable to listen() socket, retrying...\n"
                );
                shutdown(is_ptr->socket, SHUT_RDWR);
                is_ptr->socket = -1;

                sleep(BIND_RETRY_INTERVAL);
                continue;
            }

            /* Set listening port socket to nonblocking. */
            fcntl(is_ptr->socket, F_SETFL, O_NONBLOCK);

            break;
        }
        /* Too many retries? */
        if(i >= BIND_RETRIES) 
        {
            fprintf(stderr,
 "IncomingSocketInit(): Could not establish listening socket to port %i.\n",
                port_num
            );

            /* Delete this incoming socket. */
	    IncomingSocketDelete(n);
	    IncomingSocketReclaim();

            return(-1);
        }


        return(0);
}


/*       
 *      Deletes an incoming socket structure, closing
 *      the socket as needed.
 */
void IncomingSocketDelete(int n)
{ 
	if(!IncomingSocketIsAllocated(n))
            return;

 
        /* Close socket as needed. */
        if(incoming_socket[n]->socket > -1)
        {
            shutdown(incoming_socket[n]->socket, SHUT_RDWR);
            incoming_socket[n]->socket = -1;
        }
        
        /* Delete structure. */
        free(incoming_socket[n]);
        incoming_socket[n] = NULL;
        
        
        return;
}


void IncomingSocketDeleteAll()
{
	int i;


        for(i = 0; i < total_incoming_sockets; i++)
        {
            IncomingSocketDelete(i);
        }
        free(incoming_socket);
        incoming_socket = NULL;

        total_incoming_sockets = 0;


	return;
}


void IncomingSocketReclaim()
{
	int i, n;


        for(i = 0, n = -1; i < total_incoming_sockets; i++)
        {
	    if(incoming_socket[i] == NULL)
		continue;

	    n = i;
        }

	total_incoming_sockets = n + 1;
	if(total_incoming_sockets > 0)
	{
            incoming_socket = (incoming_socket_struct **)realloc(
                incoming_socket,
                total_incoming_sockets * sizeof(incoming_socket_struct *)
            );
            if(incoming_socket == NULL)
            {
                total_incoming_sockets = 0;
                return;
            }
	}
	else
	{
	    free(incoming_socket);
	    incoming_socket = NULL;

	    total_incoming_sockets = 0;
	}


	return;
}
