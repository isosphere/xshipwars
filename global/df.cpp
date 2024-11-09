#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "../include/os.h"
#include "../include/strexp.h"
#include "../include/prochandle.h"
#include "../include/fio.h"
#include "../include/df.h"


/*
 *	Returns a dynamically allocated array of device listings.
 *	The number of device listings allocated will be indicated
 *	by total.
 */
df_stat_struct **DiskFreeGetListing(int *total)
{
	int i, n;
	char *strptr, *strptr2;
	char tmp_file[PATH_MAX + NAME_MAX];
	pid_t p;
	struct stat stat_buf;

	char **strv;
	int strc;

	FILE *fp;
	df_stat_struct **df_stat = NULL;

        char df_mounted_on[NAME_MAX + PATH_MAX];
        char df_filesystem[NAME_MAX + PATH_MAX];

        off_t df_total;
        off_t df_used;
        off_t df_available;


	/* Pointer to total must exist. */
	if(total == NULL)
	    return(df_stat);
	else
	    *total = 0;


	/* Get tempory file name. */
	strptr = tmpnam(NULL);
	if(strptr == NULL)
	    return(df_stat);

	strncpy(tmp_file, strptr, PATH_MAX + NAME_MAX);
	tmp_file[PATH_MAX + NAME_MAX - 1] = '\0';


	/* Make sure temp file does NOT exist before running. */
	if(!stat(tmp_file, &stat_buf))
	    return(df_stat);


	/* Run df program, blocking execution. */
	p = ExecBAO(DF_CMD, tmp_file);
	if(p < 0)
	    return(df_stat);


	/* Check if output file exists. */
	if(stat(tmp_file, &stat_buf))
            return(df_stat);

	fp = fopen(tmp_file, "rb");
	if(fp == NULL)
	    return(df_stat);


	/* Skip past first line. */
	strptr = FReadNextLineAlloc(fp, '#');
	if(strptr == NULL)
	{
	    fclose(fp);
	    return(df_stat);
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


	    /* Get filesystem. */
	    if(strc >= 1)
		strncpy(df_filesystem, strv[0], NAME_MAX + PATH_MAX);
	    df_filesystem[NAME_MAX + PATH_MAX - 1] = '\0';

            /* Get total. */
	    df_total = (strc >= 2) ? atol(strv[1]) : 0;

            /* Get used. */
            df_used = (strc >= 3) ? atol(strv[2]) : 0;

            /* Get available. */
            df_available = (strc >= 4) ? atol(strv[3]) : 0;

            /* Skip capacity %. */

            /* Get mounted on. */
            if(strc >= 6)
                strncpy(df_mounted_on, strv[5], NAME_MAX + PATH_MAX);
            df_mounted_on[NAME_MAX + PATH_MAX - 1] = '\0';



	    /* Free exploded strings. */
            for(i = 0; i < strc; i++)
	    {
		free(strv[i]);
	    }
	    free(strv);
	    strv = NULL;


            /* Free current line and read next line. */
            free(strptr);
            strptr = FReadNextLineAlloc(fp, '#');


	    /* Allocate one more pointer for a df_stat_struct. */
	    n = *total;
	    *total += 1;
	    df_stat = (df_stat_struct **)realloc(
		df_stat,
		(*total) * sizeof(df_stat_struct *)
	    );
	    if(df_stat == NULL)
	    {
		*total = 0;
		return(df_stat);
	    }

            /* Allocate a df_stat_struct. */
	    df_stat[n] = (df_stat_struct *)calloc(
		1,
		sizeof(df_stat_struct)
	    );
	    if(df_stat[n] == NULL)
	    {
		*total -= 1;
	    }

	    /* Set new values (convert to required units). */
#if defined(_AIX_)
            strncpy(df_stat[n]->filesystem, df_filesystem, NAME_MAX + PATH_MAX);
            df_stat[n]->filesystem[NAME_MAX + PATH_MAX - 1] = '\0';

/* AIX returns in units of 512 bytes. */
            df_stat[n]->total = df_total * 2;
            df_stat[n]->used = df_used * 2;
            df_stat[n]->available = df_available * 2;

            strncpy(df_stat[n]->mounted_on, df_mounted_on, NAME_MAX + PATH_MAX);
            df_stat[n]->mounted_on[NAME_MAX + PATH_MAX - 1] = '\0';
#else
/* All other systems should return in units of 1024 bytes. */
	    strncpy(df_stat[n]->filesystem, df_filesystem, NAME_MAX + PATH_MAX);
            df_stat[n]->filesystem[NAME_MAX + PATH_MAX - 1] = '\0';

            df_stat[n]->total = df_total;
            df_stat[n]->used = df_used;
            df_stat[n]->available = df_available;

            strncpy(df_stat[n]->mounted_on, df_mounted_on, NAME_MAX + PATH_MAX);
	    df_stat[n]->mounted_on[NAME_MAX + PATH_MAX - 1] = '\0';
#endif
	}


	/* Close file. */
	fclose(fp);
	fp = NULL;

	/* Remove file. */
	unlink(tmp_file);
	tmp_file[0] = '\0';


	return(df_stat);
}


void DiskFreeDeleteListing(df_stat_struct **df_stat, int total)
{
	int i;


	for(i = 0; i < total; i++)
	    free(df_stat[i]);

	free(df_stat);


	return;
}
