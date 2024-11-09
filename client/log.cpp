/*
                            Logging

	Functions:

	int LogAppendLineFormatted(char *filename, char *str)

	---


 */

#include "xsw.h"



int LogAppendLineFormatted(char *filename, char *str)
{
	FILE *fp;
	int len, bytes_written;


	/* Make sure str is valid. */
	if((filename == NULL) ||
	   (str == NULL)
	)
	    return(-1);
	else if(str[0] == '\0')
	    return(0);


	/* ************************************************************ */

        /* Open or create file for appending. */
        fp = fopen(filename, "a");
        if(fp == NULL)
        {
            fprintf(stderr,
                "%s: Cannot open or create.\n",
                filename
            );
            return(-1);
        }

	/* Append the line. */
	len = strlen(str);
        bytes_written = fwrite(str, sizeof(char), len, fp);

	/* Close the file. */
	fclose(fp);
	fp = NULL;


	/* Return the number of bytes written. */
	return(bytes_written);
}
