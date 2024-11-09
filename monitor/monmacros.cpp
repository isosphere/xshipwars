/*
	Functions:

	void MonSetErrorMesg(
	        monitor_struct *m,
	        char *mesg
	)
	int MonConnect(monitor_struct *m, char *address, int port)
	int MonDoSend(monitor_struct *m, char *buf)
	void MonDoHandleSegment(monitor_struct *m, char *buf)
	int MonDoFetch(monitor_struct *m)

	---

 */


#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/types.h>

extern int h_errno;		/* For netdb.h. */

#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <fcntl.h>

#include "../include/string.h"
#include "../include/strexp.h"
#include "../include/disk.h"
#include "../include/netio.h"

#include "../include/osw-x.h"
#include "../include/widget.h"

#include "../include/objects.h"
#include "../include/auxstatcodes.h"

#include "mon.h"
#include "config.h"


void MonDoHandleSegment(monitor_struct *m, char *buf);


/*
 *	Sets new error message on monitor m.
 */
void MonSetErrorMesg(
        monitor_struct *m,
        char *mesg
)
{
	if(m == NULL)
	    return;


        free(m->last_error_mesg);
        m->last_error_mesg = NULL;


	if(mesg != NULL)
	    m->last_error_mesg = StringCopyAlloc(mesg);


	return;
}

/*
 *      Connects to address and port, returns socket or -1 on error.
 */        
int MonConnect(monitor_struct *m, char *address, int port)
{
        struct hostent *he;
        struct sockaddr_in haddr;
	char text[HOST_NAME_MAX + 256];


        if((m == NULL) ||
           (address == NULL) ||
           (port < 0)
        )
            return(-1);

	/* Reset window title. */
	OSWSetWindowTitle(m->toplevel, NOT_REACHABLE_TITLE_STRING);
        OSWSetWindowTitle(m->mesgwin.toplevel, NOT_REACHABLE_TITLE_STRING);


	/* Set new address and port on monitor. */
	strncpy(
	    m->address,
	    address,
	    HOST_NAME_MAX
	);
	m->address[HOST_NAME_MAX - 1] = '\0';

	m->port = port;


	/* Close previous connection. */
	if(m->socket > -1)
	{
	    close(m->socket);
	    m->socket = -1;
	}


	/* Get new host address info. */
        he = gethostbyname(m->address);
        if(he == NULL)
        {
            sprintf(
                text,
                "Unknown Host:\n    %s",
                m->address
            );
	    MonSetErrorMesg(m, text);

            return(-1);
        }

        m->socket = socket(AF_INET, SOCK_STREAM, 0);
        if(m->socket < 0)
        {
            sprintf(
                text,
                "Internal Error:\n    socket(): Cannot create socket."
            );
            MonSetErrorMesg(m, text);

            return(-1);
        }

        haddr.sin_family = AF_INET;             /* In hbo. */ 
        haddr.sin_port = htons(m->port);           /* In nbo. */
        haddr.sin_addr = *((struct in_addr *)he->h_addr);
        bzero(&(haddr.sin_zero), 8);    /* Zero the rest of struct. */


        if(connect(m->socket, (struct sockaddr *)&haddr,
                sizeof(struct sockaddr)) == -1)
        {
            sprintf(
                text,
                "Cannot connect to:\n    %s\n    Port: %i\n",
		m->address,
		m->port
            );
            MonSetErrorMesg(m, text);

            close(m->socket);
	    m->socket = -1;

            return(-1);
        }

        /* Set socket to non-blocking. */
        fcntl(m->socket, F_SETFL, O_NONBLOCK);


        return(0);
}


/*
 *	Sends buf to where monitor m is connected.
 */
int MonDoSend(monitor_struct *m, char *buf)
{
	int bytes_written;


	if((m == NULL) ||
	   (buf == NULL)
	)
	    return(-1);

	if(*buf == '\0')
	    return(0);

	if(!NetIsSocketWritable(m->socket, NULL))
	    return(-1);

	bytes_written = send(
	    m->socket,
	    buf,
	    strlen(buf),
	    0
	);


	return(bytes_written);
}

/*
 *	Parses and manages a segment of data.
 */
void MonDoHandleSegment(monitor_struct *m, char *buf)
{
	char *strptr;
        char parm[AUXSTAT_SEGMENT_MAX];
        char val[AUXSTAT_SEGMENT_MAX];
	char sndbuf[AUXSTAT_SEGMENT_MAX];
	mon_stats_struct *stats_ptr;
	int i1, i2;
	long l1, l2, l3;


	if((m == NULL) ||
           (buf == NULL)
	)
	    return;

	stats_ptr = &m->stats;


	/* Search for parmameter and value delimiter. */
	strptr = strchr(buf, ':');
	if(strptr == NULL)
	{
	    parm[0] = '\0';

	    strncpy(val, buf, AUXSTAT_SEGMENT_MAX);
	    val[AUXSTAT_SEGMENT_MAX - 1] = '\0';
	}
	else
	{
	    strncpy(parm, buf, AUXSTAT_SEGMENT_MAX);
	    parm[AUXSTAT_SEGMENT_MAX - 1] = '\0';

	    strptr = strchr(parm, ':');
	    if(strptr != NULL)
		*strptr = '\0';

            strncpy(val, strptr + 1, AUXSTAT_SEGMENT_MAX);
            val[AUXSTAT_SEGMENT_MAX - 1] = '\0';
	}
	StringStripSpaces(val);
	StringStripSpaces(parm);


	/* Connections. */
	if(!strcasecmp(parm, STAT_PFX_CONNECTIONS))
	{
            /*
             *   total_connections guest_connections
             */
	    sscanf(val, "%i %i",
		&i1,
		&i2
	    );

	    stats_ptr->total_connections = i1;
	    stats_ptr->guest_connections = i2;
	}
	/* Memory. */
	else if(!strcasecmp(parm, STAT_PFX_MEMORY))
	{
	    /*
	     *   mem_total mem_con mem_obj
	     */
            sscanf(val, "%ld %ld %ld",
                &l1,
                &l2,
		&l3
            );

            stats_ptr->mem_total = l1;
            stats_ptr->mem_con = l2;
            stats_ptr->mem_obj = l3;
	}
	/* Message. */
        else if(!strcasecmp(parm, STAT_PFX_MESSAGE))
        {
            /*
             *   message
             */
	    MesgWinAdd(&m->mesgwin, val, mon_color.messages_text);
	}
	/* Next. */
        else if(!strcasecmp(parm, STAT_PFX_NEXT))
        {
            /*
             *   next_save next_export
             */
            sscanf(val, "%ld %ld",
                &l1,
                &l2
            );

            stats_ptr->next_save = l1;
            stats_ptr->next_export = l2;
 	}
        /* Title. */
        else if(!strcasecmp(parm, STAT_PFX_TITLE))
        {
	    strncpy(
		stats_ptr->title,
		val,
		UNV_TITLE_MAX
	    );
	    stats_ptr->title[UNV_TITLE_MAX - 1] = '\0';

	    OSWSetWindowTitle(m->toplevel, stats_ptr->title);
            OSWSetWindowTitle(m->mesgwin.toplevel, stats_ptr->title);
        }
        /* Objects. */
        else if(!strcasecmp(parm, STAT_PFX_OBJECTS))
        {
            /*
             *   total_objects
             */
            sscanf(val, "%i",
                &i1
            ); 
 
            stats_ptr->total_objects = i1;
        }
        /* PID. */
        else if(!strcasecmp(parm, STAT_PFX_PID))
        {
            /*
             *   pid
             */
            sscanf(val, "%i",
                &i1
            );
            stats_ptr->pid = i1;
        }
	/* Request login. */
        else if(!strcasecmp(parm, STAT_PFX_REQUESTLOGIN))
        {
            /*
             *
             */
	    /* Respond with login info. */
	    sprintf(sndbuf, "%s: %s;%s\n",
		STAT_PFX_LOGIN,
		m->name,
		m->password
	    );
	    MonDoSend(m, sndbuf);
        }
        /* Uptime. */
        else if(!strcasecmp(parm, STAT_PFX_UPTIME))
        {
            /*
             *   uptime
             */
            sscanf(val, "%ld",   
                &l1
            );

            stats_ptr->uptime = l1;
        }


	return;
}


int MonDoFetch(monitor_struct *m)
{
	int socket;
        int bytes_read;
        struct timeval timeout;
        fd_set readfds;

	char buf[AUXSTAT_MAX_LEN];
	char *buf_ptr;
	int buf_cnt;

	char workbuf[AUXSTAT_SEGMENT_MAX];
	char *workbuf_ptr;
	int workbuf_cnt;


	if(m == NULL)
	    return(-1);

	/* Pipe not opened? */
	socket = m->socket;
	if(socket < 0)
	    return(0);


        /* Contains any data to be read? */
        timeout.tv_sec = 0;
        timeout.tv_usec = 0;
        FD_ZERO(&readfds);
        FD_SET(socket, &readfds);
        if(
            select(
                socket + 1,
                &readfds, NULL, NULL,
                &timeout
            ) < 1
        )
            return(0);
        if(!FD_ISSET(socket, &readfds))
            return(0);

	/* Read. */
        bytes_read = recv(
            socket,
            buf,
            AUXSTAT_MAX_LEN * sizeof(char),
	    0
        );
        if(bytes_read == 0)
        {
            /*   When polling of socket says there are bytes to be
             *   read and recv() returns 0, it implies that the
             *   socket has died.
             */
	    close(m->socket);
	    m->socket = -1;

            return(-1);
        }
        /* Recieve error? */
        if(bytes_read < 0)
        {
            /* Handle error. */
            switch(errno)
            {
              /* Invalid descriptor. */
              case EBADF:
                close(m->socket);
                m->socket = -1;
                return(-1);
                break;

	      /* Not a socket. */
              case ENOTSOCK:
                m->socket = -1;
                return(-1);
                break;

              /* Operation would block a non-blocking socket. */
              case EWOULDBLOCK:
		/* Not an error. */
		return(0);
		break;

              /* Segmentation fault. */
              case EFAULT:
                close(m->socket);
                m->socket = -1;  
                return(-1);
                break;

              default:
                close(m->socket);
                m->socket = -1;
                return(-1);
                break;
            }
        }



	/* Null terminate. */
	if(bytes_read > AUXSTAT_MAX_LEN)
	    bytes_read = AUXSTAT_MAX_LEN;

	buf[bytes_read - 1] = '\0';


	/* Begin parsing and handling. */
        buf_cnt = 0;
        buf_ptr = buf;

        while(buf_cnt < bytes_read)
        {
            /* Reset work buffer count and pointer. */
            workbuf_cnt = 0;
            workbuf_ptr = workbuf;

            /* Begin fetching data from recvbuf to workbuf. */
            while(1)
            {
                /* Work buffer overflowed? */
                if(workbuf_cnt >= AUXSTAT_SEGMENT_MAX)
                {
                    /* Increment buf_cnt to next delimiter char. */
                    while(buf_cnt < bytes_read)
                    {
                        if((*buf_ptr == '\n') ||
                           (*buf_ptr == '\r') ||
                           (*buf_ptr == '\0')
                        )
                        {
                            buf_cnt++;
                            buf_ptr++;
                            break;
                        }
                        buf_cnt++;
                        buf_ptr++;
                    }
                    /*   Null terminating character for workbuf will
                     *   be added farther below.
                     */
                    break;
                }

                /* End of buffer reached? */
                else if(buf_cnt >= bytes_read)
                {
                    *workbuf_ptr = '\0';

                    /*   Copy workbuf_cnt to net_parms.co_data for
                     *   processing in the next call to this function.
                     */
/*
                    net_parms.co_data_len = MIN(
                        workbuf_cnt,
                        AUXSTAT_SEGMENT_MAX
                    );

                    if(net_parms.co_data_len > 0)  
                        memcpy(
                            net_parms.co_data,
                            workbuf,
                            net_parms.co_data_len
                        );
 */                         
                    /*   Return, don't continue. This is so that the
                     *   the carry over buffer is not reset and its
                     *   values just set above get carryed over to
                     *   the next call to this function.
                     */
                    return(0);

                    break;
                }

                /* Deliminator in recieve buffer encountered? */
                else if((*buf_ptr == '\n') ||
                        (*buf_ptr == '\r') ||
                        (*buf_ptr == '\0')
                )
                {
                    *workbuf_ptr = '\0';

                    buf_cnt++;
                    buf_ptr++;

                    break;
                }

                /* Copy data from buffer to work buffer normally. */
                else
                {
                    *workbuf_ptr = *buf_ptr;

                    buf_cnt++;  
                    buf_ptr++;

                    workbuf_cnt++;
                    workbuf_ptr++;
                }
            }

            /* Skip if workbuf is empty. */
            if(*workbuf == '\0')
                continue;

            /* Set null terminating character for workbuf. */
            workbuf[AUXSTAT_SEGMENT_MAX - 1] = '\0';


	    /* Handle this segment. */
	    MonDoHandleSegment(m, workbuf);

	    /* Redraw monitor readout. */
	    if(m->map_state)
	        MonDraw(m, MON_DRAW_AMOUNT_READOUT);
	}




	return(0);
}

