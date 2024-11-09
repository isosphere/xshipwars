/*
                      Server Script Transfer Management

	Functions:

	int ServScriptIsAllocated(int n)

	int ServScriptStart(
		char *filename,
		char **argv,
		int argc
	)
	void ServScriptDelete(int n)
	void ServScriptDeleteAll()
	void ServScriptReclaim()

	void ServScriptDoSend(serv_script_struct *ss)
	void ServScriptManage()

	int ServScriptDoMapPrompt(char *filename)

	---

	For maintaining the upload of a script to the server.

 */

#include "xsw.h"
#include "net.h"


/*
 *	Checks if server script n is allocated.
 */
int ServScriptIsAllocated(int n)
{
	if((serv_script == NULL) ||
	   (n < 0) ||
           (n >= total_serv_scripts)
	)
	    return(0);
	else if(serv_script[n] == NULL)
            return(0);
	else
            return(1);
}


/*
 *	Allocates a new server script structure and
 *	sets it up.
 *
 *	IMPORTANT: the string pointers in argv are NOT
 *	coppied, the calling function must NOT free() them.
 *	Use ServScriptDelete() to free them!!!
 */
int ServScriptStart(char *filename, char **argv, int argc)
{
	int i, n;
	struct stat stat_buf;


	/* Sanitize total. */
	if(total_serv_scripts < 0)
	    total_serv_scripts = 0;


	/* Check for available pointer. */
	for(i = 0; i < total_serv_scripts; i++)
	{
	    if(serv_script[i] == NULL)
		break;
	}
	if(i < total_serv_scripts)
	{
	    n = i;
	}
	else
	{
	    n = total_serv_scripts;
	    total_serv_scripts++;

	    serv_script = (serv_script_struct **)realloc(
		serv_script,
		total_serv_scripts * sizeof(serv_script_struct *)
	    );
	    if(serv_script == NULL)
	    {
		total_serv_scripts = 0;
		return(-1);
	    }
	}

	/* Allocate new structure. */
	serv_script[n] = (serv_script_struct *)calloc(
	    1,
	    sizeof(serv_script_struct)
	);
	if(serv_script[n] == NULL)
	{
	    return(-1);
	}


	/* ********************************************************* */
	/* Open file. */
	if(filename != NULL)
        {
            serv_script[n]->fp = fopen(filename, "r");
            if(serv_script[n]->fp == NULL)
            {
                ServScriptDelete(n);
		return(-1);
            }

	    /* Get file size. */
 	    fstat(fileno(serv_script[n]->fp), &stat_buf);
	    serv_script[n]->filesize = stat_buf.st_size;
        }

	/* Filename. */
	if(filename != NULL)
	{
	    serv_script[n]->filename = StringCopyAlloc(filename);
	}

	/* Set arguments (do not copy). */
	serv_script[n]->argv = argv;
	serv_script[n]->argc = argc;


	return(n);
}


/*
 *	Deletes server script n.
 */
void ServScriptDelete(int n)
{
	int i;
	serv_script_struct *script_ptr;


	if(ServScriptIsAllocated(n))
	    script_ptr = serv_script[n];
	else
	    return;


	/* Free filename. */
#ifdef DEBUG_MEM_FREE
if(script_ptr->filename != NULL)
    printf("Server script %i: Free'ed filename.\n", n);
#endif
	free(script_ptr->filename);
	script_ptr->filename = NULL;

	/* Close file. */
	if(script_ptr->fp != NULL)
	{
	    fclose(script_ptr->fp);
	    script_ptr->fp = NULL;
	}


	/* Free arguments. */
	for(i = 0; i < script_ptr->argc; i++)
	{
#ifdef DEBUG_MEM_FREE
if(script_ptr->argv[i] != NULL)
    printf("Server script %i: Free'ed argument %i.\n", n, i);
#endif
	    free(script_ptr->argv[i]);
	}
#ifdef DEBUG_MEM_FREE
if(script_ptr->argv != NULL)
    printf("Server script %i: Free'ed argument pointers.\n", n);
#endif
	free(script_ptr->argv);
	script_ptr->argv = NULL;

	script_ptr->argc = 0;


	/* Free structure itself. */
#ifdef DEBUG_MEM_FREE
if(script_ptr != NULL)
    printf("Server script %i: Free'ed.\n", n);
#endif
	free(script_ptr);
	script_ptr = NULL;
	serv_script[n] = NULL;


	return;
}


void ServScriptDeleteAll()  
{
	int i;


	for(i = 0; i < total_serv_scripts; i++)
	{
	    ServScriptDelete(i);
	}


	free(serv_script);
	serv_script = NULL;

	total_serv_scripts = 0;


	return;
}


void ServScriptReclaim()
{
	int i, h;


	for(i = 0, h = -1; i < total_serv_scripts; i++)
	{
	    if(serv_script[i] != NULL)
		h = i;
	}

	total_serv_scripts = h + 1;

	if(total_serv_scripts > 0)
	{
	    serv_script = (serv_script_struct **)realloc(
		serv_script,
		total_serv_scripts * sizeof(serv_script_struct *)
	    );
	    if(serv_script == NULL)
	    {
		total_serv_scripts = 0;
		return;
	    }
	}
	else
	{
	    free(serv_script);
	    serv_script = NULL;
	    total_serv_scripts = 0;
	}



	return;
}


/* ****************************************************************** */

void ServScriptDoSend(serv_script_struct *ss)
{
	int i, c, argn;
	int numstr_pos;
const int numstr_len = 48;
	char numstr[numstr_len];

	int sndbuf_pos;
	char sndbuf[CS_DATA_MAX_LEN];

	char *arg_ptr;

const int sndbuf_len = CS_DATA_MAX_LEN - 10;


	if(ss == NULL)
	    return;

	/* Script file closed? */
        if(ss->fp == NULL)
	{
	    /* Set file position to match size so it gets deleted
	     * later.
	     */
	    ss->filepos = ss->filesize;
	    return;
	}


	/* Not time to send? */


	/* ********************************************************** */

	/* Set literal command prefix to sndbuf. */
	sprintf(sndbuf, "%i ", CS_CODE_LITERALCMD);

	/* Set sndbuf position. */
	sndbuf_pos = strlen(sndbuf);

	/* Put data into sndbuf buffer. */
	while(1)
	{
	    /* sndbuf position exceeded allowable sndbuf length? */
	    if(sndbuf_pos >= sndbuf_len)
	    {
                /* Set end characters. */
                sndbuf[sndbuf_pos - 2] = '\n';
		sndbuf[sndbuf_pos - 1] = '\0';

		break;
	    }


	    /* Get next character in file. */
	    c = fgetc(ss->fp);
	    ss->filepos++;


	    /* End of file? */
	    if((c == EOF) ||
               (ss->filepos >= ss->filesize)
	    )
	    {
                /* Set file position to match size so it gets deleted. */
		ss->filepos = ss->filesize;

		/* Set end characters. */
		sndbuf[sndbuf_pos] = '\n';
		if((sndbuf_pos + 1) < sndbuf_len)
		    sndbuf[sndbuf_pos + 1] = '\0';

		break;
	    }

	    /* Backslash? */
	    if(c == '\\')
	    {
		/* Next character is to be accepted literally. */

                /* Get next character in file. */
                c = fgetc(ss->fp);
                ss->filepos++;

		/* End of file? */
                if((c == EOF) ||
                   (ss->filepos >= ss->filesize)
                )
                {
                    /* Set file position to match size so it gets deleted. */
                    ss->filepos = ss->filesize;

                    /* Set end characters. */
                    sndbuf[sndbuf_pos] = '\n'; 
                    if((sndbuf_pos + 1) < sndbuf_len)  
                        sndbuf[sndbuf_pos + 1] = '\0';
 
                    break;
                }

                /* Set character. */
                sndbuf[sndbuf_pos] = (char)c;
                sndbuf_pos++;

		continue;
	    }

	    /* Substitute argument? */
	    if(c == '$')
	    {
                /* Get next character in file. */
                c = fgetc(ss->fp);
                ss->filepos++;

		/* Get number. */
		numstr_pos = 0;
		while(isdigit((char)c) &&
		      (numstr_pos < numstr_len)
		)
 		{
		    numstr[numstr_pos] = (char)c;
		    numstr_pos++;

                    /* Get next character in file. */
                    c = fgetc(ss->fp);
                    ss->filepos++;
		}
		if(numstr_pos < numstr_len)
		    numstr[numstr_pos] = '\0';
		else
		    numstr[numstr_len - 1] = '\0';

		/* Get argument number. */
		argn = atoi(numstr);
		/* Put argument into sndbuf. */
		if((argn >= 0) && (argn < ss->argc))
		{
		    arg_ptr = ss->argv[argn];
		    if(arg_ptr != NULL)
		    {
			for(i = 0; arg_ptr[i] != '\0'; i++)
			{
			    if(sndbuf_pos >= sndbuf_len)
				break;

			    sndbuf[sndbuf_pos] = arg_ptr[i];
			    sndbuf_pos++;
			}
		    }
		}

		/* Continue if argument filled sndbuf. */
		if(sndbuf_pos >= sndbuf_len)
		    continue;
	    }

            /* Newline? */
            if((c == '\n') || 
               (c == '\r')  
            )
            {
                /* Set end characters. */
                sndbuf[sndbuf_pos] = '\n';
                if((sndbuf_pos + 1) < sndbuf_len)
                    sndbuf[sndbuf_pos + 1] = '\0';

                break;
            }

	    /* Set character. */
	    sndbuf[sndbuf_pos] = (char)c;
	    sndbuf_pos++;

	}
	sndbuf[CS_DATA_MAX_LEN - 1] = '\0';


	/* Send sndbuf to connection. */
	NetSendData(sndbuf);


	return;
}


void ServScriptManage()
{
	static int i;


	/* Manage each server script. */
	for(i = 0; i < total_serv_scripts; i++)
	{
	    if(serv_script[i] == NULL)
		continue;

	    /* Send script. */
	    ServScriptDoSend(serv_script[i]);

	    /* Delete if filepos >= filesize. */
	    if(serv_script[i]->filepos >= serv_script[i]->filesize)
	    {
		ServScriptDelete(i);
		ServScriptReclaim();
	    }
	}


	return;
}



/*
 *	Callback function to map the prompt
 *	and set up the initial value to "/servscript <filename> "
 */
int ServScriptDoMapPrompt(char *filename)
{
	char lfilename[PATH_MAX + NAME_MAX];


	if(filename == NULL)
	{


	}
	else
	{
	    strncpy(lfilename, filename, PATH_MAX + NAME_MAX);
	    lfilename[PATH_MAX + NAME_MAX - 1] = '\0';

            if(lg_mesg_win.map_state == 1)
            {
                if(lg_mesg_win.prompt.buf != NULL)
                {  
                 sprintf(lg_mesg_win.prompt.buf, "servscript %s ",
                  lfilename
                 );
                 lg_mesg_win.prompt.buf_pos = strlen(lg_mesg_win.prompt.buf);
                 lg_mesg_win.prompt.buf_vis_pos = 0;

                 PromptChangeName(&lg_mesg_win.prompt, "Client Command:");
                 prompt_mode = PROMPT_CODE_CLIENT_CMD;
                 PromptMap(&lg_mesg_win.prompt);
                 lg_mesg_win.prompt.is_in_focus = 1;
                }
            }
	    else if(bridge_win.map_state == 1)
	    {
	        if(bridge_win.prompt.buf != NULL)
		{
		 sprintf(bridge_win.prompt.buf, "servscript %s ",
		  lfilename
                 );
		 bridge_win.prompt.buf_pos = strlen(bridge_win.prompt.buf);
		 bridge_win.prompt.buf_vis_pos = 0;

                 PromptChangeName(&bridge_win.prompt, "Client Command:");
                 prompt_mode = PROMPT_CODE_CLIENT_CMD;
                 PromptMap(&bridge_win.prompt);
                 bridge_win.prompt.is_in_focus = 1;
		}
	    }
	}


	return(0);
}
