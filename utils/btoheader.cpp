/*
	Converts a binary file into a C header file that you
	can #include.
 */


#include <stdio.h>
#include <string.h>


#ifndef PATH_MAX
    #define PATH_MAX 1024
#endif

#ifndef NAME_MAX
    #define NAME_MAX 256
#endif

#ifndef MAX_VARIABLE_NAME
    #define MAX_VARIABLE_NAME 256
#endif

int main(int argc, char *argv[])
{
	int i;

	FILE *infp;
	FILE *outfp;

	char buf[1024];

	char infilename[PATH_MAX + NAME_MAX];
	char outfilename[PATH_MAX + NAME_MAX];
	char declaration_name[MAX_VARIABLE_NAME];

	int lines_written;
	int bytes_written;
	int bytes_per_line_written;


	/* Need atleast 2 arguments. */
	if(argc < 3)
	{
	    printf("Usage: %s <in_file> <out_file> [declaration_name]\n",
		argv[0]
	    );
	    return(1);
	}


	strncpy(infilename, argv[1], PATH_MAX + NAME_MAX);
	infilename[PATH_MAX + NAME_MAX - 1] = '\0';

        strncpy(outfilename, argv[2], PATH_MAX + NAME_MAX);
        outfilename[PATH_MAX + NAME_MAX - 1] = '\0';
        

	if(argc > 3)
	    strncpy(declaration_name, argv[3], MAX_VARIABLE_NAME);
	else
	    strncpy(declaration_name, outfilename, MAX_VARIABLE_NAME);

	declaration_name[MAX_VARIABLE_NAME - 1] = '\0';


	/* *************************************************************** */

	infp = fopen(infilename, "r");
	if(infp == NULL)
	{
	    fprintf(stderr, "%s: Cannot open.\n", infilename);
	    return(1);
	}

	outfp = fopen(outfilename, "w");
	if(outfp == NULL)
	{
            fprintf(stderr, "%s: Cannot create.\n", outfilename);
            return(1);
	}


	/* *************************************************************** */

        lines_written = 0;
        bytes_written = 0;
        bytes_per_line_written = 0;


	/* Write header for output file. */
	sprintf(buf, "static unsigned char %s[] = {\n",
	    declaration_name
	);
	bytes_written += fwrite(buf, sizeof(char), strlen(buf), outfp);
	lines_written++;



	/* Begin reading input file and converting. */
	rewind(infp);
	i = fgetc(infp);

	while(i != EOF)
	{
	    /* Write carrage return as needed. */
	    if(bytes_per_line_written > 60)
	    {
		bytes_per_line_written = 0;
                sprintf(buf, "\n");
 
		bytes_written += fwrite(buf, sizeof(char), strlen(buf), outfp);
		lines_written++;
	    }

	    sprintf(buf, "0x%.2x, ", i);
	    bytes_per_line_written += fwrite(buf, sizeof(char), strlen(buf), outfp);
	    bytes_written += bytes_per_line_written;


	    i = fgetc(infp);
	}

	fseek(outfp, -2, SEEK_CUR);
        sprintf(buf, "};\n");
	bytes_written += fwrite(buf, sizeof(char), strlen(buf), outfp);
	lines_written++;


	fclose(infp); infp = NULL;
	fclose(outfp); outfp = NULL;



	return(0);
}
