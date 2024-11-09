#include "swserv.h"
#include "../include/auxstatcodes.h"

#ifndef LOG_MAX_TIME_STR
# define LOG_MAX_TIME_STR	256
#endif


char *LogGetCurrentTimeString(void);

/*
 *	Returns a formatted time string of the current time specified
 *	in global cur_systime.
 */
char *LogGetCurrentTimeString(void)
{
        size_t len;
	time_t ct;
        struct tm *tm_ptr;
        static char s[LOG_MAX_TIME_STR];


	/* Get current systime from global cur_systime. */
	ct = cur_systime;

        /* Get current time. */
        tm_ptr = localtime(&ct);
        if(tm_ptr == NULL)
            return("");

        /* Format time string. */
        len = strftime(
            s,
            LOG_MAX_TIME_STR,
            "%c",
            tm_ptr
        );
        if(len >= LOG_MAX_TIME_STR)
            len = LOG_MAX_TIME_STR - 1; 
        if(len < 0)
            len = 0;
        /* Null terminate. */
        s[len] = '\0';


        return(s);
}


/*
 *	Logs message mesg to the log file filename.  The message
 *	may not contain any new line characters.
 */
int LogAppendLineFormatted(char *filename, char *mesg)
{
	int i, len;
        FILE *fp;
	char *time_str_ptr, *buf;
	aux_connection_struct **ac_ptr;


	/* Make sure input message is valid and contains data. */
	if((mesg == NULL) ? 1 : (*mesg == '\0'))
	    return(0);


        /* Open or create file for appending. */
        fp = fopen(filename, "a");
        if(fp == NULL)
            return(-1);


	/* Allocate buffer (for sending to AUX stat connections). */
	len = strlen(mesg) + 128;
	buf = (char *)malloc(len * sizeof(char));
	if(buf == NULL)
	{
	    fclose(fp);
	    return(-1);
	}


	/* Get time string (return will never be NULL). */
	time_str_ptr = LogGetCurrentTimeString();

	/* Print log line to file. */
	fprintf(fp, "%s: %s\n", time_str_ptr, mesg);

	/* Close file. */
	fclose(fp);


	/* Write to all AUX connections. */
	sprintf(
	    buf,
	    "%s: %s: %s\n",
	    STAT_PFX_MESSAGE,
	    time_str_ptr,
	    mesg
	);
	for(i = 0, ac_ptr = aux_connection;
            i < total_aux_connections;
            i++, ac_ptr++
	)
	{
	    if(*ac_ptr == NULL)
		continue;

	    if(!(*ac_ptr)->logged_in)
		continue;

	    if((*ac_ptr)->socket < 0)
		continue;


	    AUXConSend(*ac_ptr, buf);
	}


	/* Free buffer. */
	free(buf);


	return(0);
}
