/*
                       Network IO Functions

	Functions:

	int NetIsSocketWritable(int s, int *error_level)
	int NetIsSocketReadable(int s, int *error_level)

	---
 */

#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
extern int errno;
#include <errno.h>

#include "../include/netio.h"


/*
 *      Checks if the socket is valid and is writeable.
 *
 *      If error_level is not NULL, then the error levels are
 *      as follows.
 *
 *      0 = No error.
 *      1 = Minor error (not seriousm, try again later).
 *      2 = Unknown error.
 *      3 = Sever error or socket has died.
 *
 *      Returns 0 if not writeable and 1 if it is.
 */
int NetIsSocketWritable(int s, int *error_level)
{
        struct timeval t;  
        fd_set writefds;


	if(s < 0)
	{
	    if(error_level != NULL)
		*error_level = 3;

	    return(0);
	}

        t.tv_sec = 0;
        t.tv_usec = 0;
        FD_ZERO(&writefds);
        FD_SET(s, &writefds);
        if(
            select(   
                s + 1, 
                NULL, &writefds, NULL,
                &t
            ) == -1
        )
        {
            if(error_level != NULL)
            {
                switch(errno)
                {
                  case EBADF:
                    *error_level = 3;
                    break;
  
                  case EINTR:
                    *error_level = 3;
                    break;

                  case EINVAL:
                    *error_level = 3;
                    break;

		  case EAGAIN:
                    *error_level = 1;
                    break;

                  default:
                    *error_level = 3;
                    break;
                }
            }

            return(0);
        }


        if(FD_ISSET(s, &writefds))
        {
            if(error_level != NULL)   
                *error_level = 0;

            return(1);
        }
        else
        {
            if(error_level != NULL)
                *error_level = 2;

            return(0);
        }

	return(0);	/* Never reached. */
}

/*
 *      Checks if the socket is valid and has data to be read.
 *
 *      If error_level is not NULL, then the error levels are
 *      as follows.
 *
 *      0 = No error.
 *      1 = Minor error (not serious, try again later).
 *      2 = Unknown error.
 *      3 = Sever error or socket has died (most sever).
 *
 *      Returns 0 if not readable and 1 if it is.
 */
int NetIsSocketReadable(int s, int *error_level)
{
        struct timeval t;
        fd_set readfds;


        if(s < 0)
        {
            if(error_level != NULL)
                *error_level = 3;

            return(0);
        }

        t.tv_sec = 0;
        t.tv_usec = 0;
        FD_ZERO(&readfds);
        FD_SET(s, &readfds);
        if(
            select(
                s + 1,
                &readfds, NULL, NULL,
                &t
            ) == -1
        )
        {
            if(error_level != NULL)
            {
                switch(errno)
                {
                  case EBADF:
                    *error_level = 3;
                    break;

                  case EINTR:
                    *error_level = 3;
                    break;

                  case EINVAL:
                    *error_level = 3;
                    break;

                  case EAGAIN:
                    *error_level = 1;
                    break;

                  default:
                    *error_level = 3;
                    break;
                }
            }

            return(0);
        }

        if(FD_ISSET(s, &readfds))
        {
            if(error_level != NULL)
                *error_level = 0;

            return(1);
        }
        else
        {
            if(error_level != NULL)
                *error_level = 2;

            return(0);
        }

	return(0);	/* Never reached. */
}
