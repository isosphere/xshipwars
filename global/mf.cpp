// global/mf.cpp
/*
                                Memory Free

	Functions:

	int MemoryFree(mf_stat_struct *buf)
	off_t mf(mf_stat_struct *buf)

	---

 */
/*
#include <stdio.h>
#include <malloc.h>

#include <stdlib.h>
*/

#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "../include/strexp.h"
#include "../include/prochandle.h"
#include "../include/fio.h"
#include "../include/mf.h"


/*
 *	Fetches free and total memory statistics and puts them
 *	at the location pointed to in buf.
 *
 *	Returns 0 on success and -1 on error.
 */
int MemoryFree(mf_stat_struct *buf)
{
	int i;
	char *strptr, *strptr2;
	char tmp_file[PATH_MAX + NAME_MAX];
	pid_t p;
	struct stat stat_buf;

	char **strv;
	int strc;

	FILE *fp;


	/* Pointer to buf cannot be NULL. */
	if(buf == NULL)
	    return(-1);
	else
	    memset(buf, 0, sizeof(mf_stat_struct));


	/* Get tempory file name. */
	strptr = tmpnam(NULL);
	if(strptr == NULL)
	    return(-1);

	strncpy(tmp_file, strptr, PATH_MAX + NAME_MAX);
	tmp_file[PATH_MAX + NAME_MAX - 1] = '\0';


	/* Make sure temp file does NOT exist before running. */
	if(!stat(tmp_file, &stat_buf))
	    return(-1);

	/* Run df program, blocking execution. */
	p = ExecBAO(FREE_CMD, tmp_file);
	if(p < 0)
	    return(-1);


	/* Check if output file exists. */
	if(stat(tmp_file, &stat_buf))
            return(-1);

	fp = fopen(tmp_file, "rb");
	if(fp == NULL)
	    return(-1);


	/* Skip past first line. */
	strptr = FReadNextLineAlloc(fp, '#');
	if(strptr == NULL)
	{
	    fclose(fp);
	    return(-1);
	}

	free(strptr);	/* Discard first line. */

	/* Begin reading each line. */
	strptr = FReadNextLineAlloc(fp, '#');
	while(strptr != NULL)
	{
	    /* Begin parsing line. */

	    /* Remove carrage returns at the end of the line. */
	    strptr2 = strchr(strptr, '\n');
	    if(strptr2 != NULL)
		*strptr2 = '\0';
            strptr2 = strchr(strptr, '\r');
            if(strptr2 != NULL)
                *strptr2 = '\0';


	    /* Explode line. */
	    strv = strexp(strptr, &strc);
	    if(strc <= 0)
	    {
                /* Free current line and read next line. */
                free(strptr);
                strptr = FReadNextLineAlloc(fp, '#');

		continue;
	    }


	    /* Check what stats this line gives us. */
	    if(!strcasecmp(strv[0], "Mem:"))
	    {
	        /* Total. */
	        buf->total = ((strc >= 2) ? atol(strv[1]) : 0) * 1024;

	        /* Used. */
	        buf->used = ((strc >= 3) ? atol(strv[2]) : 0) * 1024;

                /* Free. */
                buf->free = ((strc >= 4) ? atol(strv[3]) : 0) * 1024;

                /* Shared. */
                buf->shared = ((strc >= 5) ? atol(strv[4]) : 0) * 1024;

                /* Buffers. */
                buf->buffers = ((strc >= 6) ? atol(strv[5]) : 0) * 1024;

                /* Cached. */
                buf->cached = ((strc >= 7) ? atol(strv[6]) : 0) * 1024;
	    }
            else if(!strcasecmp(strv[0], "Swap:"))
	    {
                /* Swap total. */
                buf->swap_total = ((strc >= 2) ? atol(strv[1]) : 0) * 1024;

                /* Swap used. */
                buf->swap_used = ((strc >= 3) ? atol(strv[2]) : 0) * 1024;

                /* Swap free. */
                buf->swap_free = ((strc >= 4) ? atol(strv[3]) : 0) * 1024;
	    }

	    /* Free exploded strings. */
            for(i = 0; i < strc; i++)
		free(strv[i]);
	    free(strv);
	    strv = NULL;


            /* Free current line and read next line. */
            free(strptr);
            strptr = FReadNextLineAlloc(fp, '#');
	}


	/* Close file. */
	fclose(fp);
	fp = NULL;

	/* Remove tempory file. */
	unlink(tmp_file);


	return(0);
}


/*
 *	Returns the amount of free physical memory in bytes.
 *
 *	If buf is not NULL, then the memory stats will be copped to
 *	it's address.
 *
 *	Returns 0 on error.
 */
off_t mf(mf_stat_struct *buf)
{
	mf_stat_struct lbuf;


	if(buf == NULL)
	{
            MemoryFree(&lbuf);
            return(lbuf.free);
	}
	else
	{
	    MemoryFree(buf);
	    return(buf->free);
	}
}




