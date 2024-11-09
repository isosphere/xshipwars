#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include <unistd.h>
#include <signal.h>
#include <sys/time.h>

#ifndef _USE_BSD
# define _USE_BSD
#endif

#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/wait.h>

#include "../include/os.h"
#include "../include/string.h"
#include "../include/disk.h"
#include "../include/prochandle.h"

#ifdef MEMWATCH
# include "memwatch.h"
#endif


static void child_catcher(int s);
static void *add_signal(int signum, void (*handler)(int));

/* Explode commands. */
char **ExecExplodeCommand(const char *cmd, int *strc);

/* Regular execute. */
void Execute(const char *cmd);

/* Non-blocking executes. */
static pid_t ExecNexus(
        const char *cmd, const char *stdout_path, const char *stderr_path,
        const char *stdout_mode, const char *stderr_mode
);
pid_t Exec(const char *cmd);
pid_t ExecO(const char *cmd, const char *stdout_path);
pid_t ExecOE(const char *cmd, const char *stdout_path, const char *stderr_path);
pid_t ExecAO(const char *cmd, const char *stdout_path);
pid_t ExecAOE(const char *cmd, const char *stdout_path, const char *stderr_path);

/* Blocking executes. */
static pid_t ExecBNexus(
        const char *cmd, const char *stdout_path, const char *stderr_path,
        const char *stdout_mode, const char *stderr_mode
);
pid_t ExecB(const char *cmd);
pid_t ExecBO(const char *cmd, const char *stdout_path);
pid_t ExecBOE(const char *cmd, const char *stdout_path, const char *stderr_path);
pid_t ExecBAO(const char *cmd, const char *stdout_path);
pid_t ExecBAOE(const char *cmd, const char *stdout_path, const char *stderr_path);



/*
 *	Signal handler for SIGCHLD.
 */
static void child_catcher(int s)
{
        int status;

        while(wait3(&status, WNOHANG, (struct rusage *)0) > 0);

	return;
}

/*
 *	BSD style signal(2).
 *
 *	Returns the old signal action handler of type
 *	void (*handler)(int).
 */
static void *add_signal(int signum, void (*handler)(int))
{
	struct sigaction act, old_act;

	act.sa_handler = handler;
	act.sa_flags = 0;

	if(sigaction(signum, &act, &old_act) == -1)
	    return((void *)SIG_ERR);
	else
	    return((void *)old_act.sa_handler);
}

/*
 *	Explodes the command string cmd and returns an array of
 *	dynamically allocated strings.
 *
 *	Arguments inside quotes ('"') with spaces will not be exploded.
 *
 *	Arguments will be delimited by one or more spaces (not tabs).
 *
 *	The calling function must free() the returned strings and the
 *	array.
 */
char **ExecExplodeCommand(const char *cmd, int *strc)
{
        int x, y, z, str_num, src_pos, len;
        char **strv;
	char skip_quotes;
	char c = ' ';	/* Deliminator. */


        if(strc == NULL)
            return(NULL);
        if(cmd == NULL)
        {
            (*strc) = 0;  
            return(NULL);
        }

        /* Reset values. */
        src_pos = 0;
        str_num = 0;
            
        strv = NULL;
        (*strc) = 0;

        len = strlen(cmd);
	skip_quotes = 0;

	/* Skip initial characters. */
	while(src_pos < len)
	{
	    if(cmd[src_pos] != c)
		break;

	    src_pos++;
	}

	/* Begin exploding strings. */
        while(src_pos < len)
        {
	    skip_quotes = 0;

            /* Get length y of this segment x. */
            x = src_pos;
            y = 0;
            while((cmd[x] != c) && (x < len))
            {
		/* Quote grouping skip. */
		if(cmd[x] == '"')
		{
		    skip_quotes = 1;
		    x++;

		    /* Seek to end quote. */
		    while(x < len)
		    {
			if(cmd[x] == '"')
			    break;

			x++;
			y++;
		    }

		    break;
		}

                x++;
                y++;
            }
            /* y is now length of this segment. */

            /* Allocate new string for this segment. */
            str_num = (*strc);
            (*strc) = str_num + 1;
            strv = (char **)realloc(strv, (*strc) * sizeof(char *));
            strv[str_num] = (char *)calloc(1, (y + 1) * sizeof(char));

            /* Copy segment. */
            x = src_pos;
            z = 0;
            while((x < len) && (z < y))
            {
		/* Skip quotes. */
		if(cmd[x] == '"')
		{
		    x++;
		    continue;
		}

                strv[str_num][z] = cmd[x];
                z++;
		x++;
            }
            strv[str_num][y] = '\0';    /* Null terminate. */

            /* Seek next src_pos. */
	    if(y < 1)
		src_pos += 1;
	    else
                src_pos += y;
	    if(skip_quotes)
	    {
                while(src_pos < len)
                {
                    if(cmd[src_pos] == '"')
		    {
			src_pos++;
                        break;
		    }
                    src_pos++;
                }
	    }
	    while(src_pos < len)
            {
		if(cmd[src_pos] != c)
		    break;

		src_pos++;
	    }
        }
        
        return(strv);
}


/*
 *	Old execute, fork()s off process and uses system() to execute
 *	command.
 *
 *	This function is provided for backwards compatability, but has
 *	security issues because it uses the unsafe system().
 */
void Execute(const char *cmd)
{
	if(cmd == NULL)
	    return;

        /* Set signal to catch finished child proccesses. */
	add_signal(SIGCHLD, child_catcher);

	/* Fork off a process. */
        switch(fork())
        {
	  /* Forking error. */
          case -1:
	    perror("fork");
            exit(1);
	    break;

	  /* We are the child: run then command then exit. */  
          case 0:
            if(system(cmd) == -1)
                exit(-1);
	    else
                exit(0);
	    break;

	  /* We are the parent: do nothing. */        
          default:
            break;
        }

        return;
}


/*              
 *      Main nexus for all Exec*() (non-blocking) functions.
 *
 *	If the given stdout_path and/or stderr_path are not NULL then 
 *	their respective files will be opened and the child's stdout 
 *	and/or stderr streams will be redirected to them.
 *
 *	The current working directory will be changed to the executed
 *	file's parent directory on the child process.
 */
static pid_t ExecNexus(
	const char *cmd,
	const char *stdout_path, const char *stderr_path,
	const char *stdout_mode, const char *stderr_mode
)
{
        int argc;
        char **argv;
        FILE *stdout_fp, *stderr_fp;
	pid_t p;
	char new_cd[PATH_MAX];


        if(cmd == NULL)
            return(0);

        /* Create stdout file (as needed). */
        if((stdout_path != NULL) && (stdout_mode != NULL))
            stdout_fp = fopen(stdout_path, stdout_mode);
        else
            stdout_fp = NULL;

        /* Create stderr file (as needed). */
        if((stderr_path != NULL) && (stderr_mode != NULL))
            stderr_fp = fopen(stderr_path, stderr_mode);
        else
            stderr_fp = NULL;

        /* Explode command string. */
        argv = ExecExplodeCommand(cmd, &argc);
        if((argv == NULL) || (argc < 1))
            return(0);

        /* Set last argument pointer to be NULL. */
        argv = (char **)realloc(argv, (argc + 1) * sizeof(char *));
        if(argv == NULL)
            return(0);
        argv[argc] = NULL;

        /* Get new current dir. */
        if(argc > 0)
        {
            const char *cstrptr = (const char *)GetParentDir(argv[0]);
            strncpy(
                new_cd,
                (cstrptr != NULL) ? cstrptr : "/",
                PATH_MAX
            );
        }
        else
        {
            strncpy(new_cd, "/", PATH_MAX);
        }
        new_cd[PATH_MAX - 1] = '\0';

        /* Set signal to catch finished child proccesses. */
        add_signal(SIGCHLD, child_catcher);

        /* Fork off a process. */
	p = fork();
        switch(p)
        {
          /* Forking error. */
          case -1:
            perror("fork");
            exit(1);
            break;

          /* We are the child: run the command then exit. */
          case 0:
            /* Redirect child's stdout and stderr streams to our
	     * opened output files (if any).
	     */
            if(stdout_fp != NULL)
                dup2(fileno(stdout_fp), fileno(stdout));
            if(stderr_fp != NULL)
                dup2(fileno(stderr_fp), fileno(stderr));

            /* Execute command and arguments. */
            execvp(argv[0], argv);
            exit(0);
            break;

          /* We are the parent: Do nothing. */
          default:
            break;
        }

        /* Free exploded argument strings and array. */
        StringFreeArray(argv, argc);

        /* Close output files. */
        if(stdout_fp != NULL)
            fclose(stdout_fp);
        if(stderr_fp != NULL)
            fclose(stderr_fp);   

	/* Return process id of child. */
        return(p);
}


pid_t Exec(const char *cmd)
{
	return(ExecNexus(cmd, NULL, NULL, NULL, NULL));
}

pid_t ExecO(const char *cmd, const char *stdout_path)
{
	return(ExecNexus(cmd, stdout_path, NULL, "w", NULL));
}

pid_t ExecOE(const char *cmd, const char *stdout_path, const char *stderr_path)
{
        return(ExecNexus(cmd, stdout_path, stderr_path, "w", "w"));
}

pid_t ExecAO(const char *cmd, const char *stdout_path)
{
        return(ExecNexus(cmd, stdout_path, NULL, "a", NULL));
}

pid_t ExecAOE(const char *cmd, const char *stdout_path, const char *stderr_path)
{
        return(ExecNexus(cmd, stdout_path, stderr_path, "a", "a"));
}


/*
 *      Main nexus for all ExecB*() (blocking) functions.
 *
 *      If the given stdout_path and/or stderr_path are not NULL then
 *      their respective files will be opened and the child's stdout
 *      and/or stderr streams will be redirected to them.
 *
 *      The current working directory will be changed to the executed
 *      file's parent directory on the child process.
 */
static pid_t ExecBNexus(
        const char *cmd,
        const char *stdout_path, const char *stderr_path,
        const char *stdout_mode, const char *stderr_mode
)
{
	int status, argc;
	char **argv;
	FILE *stdout_fp, *stderr_fp;
	pid_t p;


        if(cmd == NULL)
            return(0);

	/* Create stdout file (as needed). */
	if((stdout_path != NULL) && (stdout_mode != NULL))
	    stdout_fp = fopen(stdout_path, stdout_mode);
	else
	    stdout_fp = NULL;

        /* Create stderr file (as needed). */
        if((stderr_path != NULL) && (stderr_mode != NULL))
            stderr_fp = fopen(stderr_path, stderr_mode);
        else
            stderr_fp = NULL;

	/* Explode command string. */
	argv = ExecExplodeCommand(cmd, &argc);
	if((argv == NULL) || (argc < 1))
	    return(0);

        /* Set last argument pointer to be NULL. */
	argv = (char **)realloc(argv, (argc + 1) * sizeof(char *));
	if(argv == NULL)
            return(0);
	argv[argc] = NULL;

	/* Fork off a process. */
	p = fork();
        switch(p)
        {
          /* Forking error. */
          case -1:
            perror("fork");
            exit(1);
            break;

          /* We are the child: run the command then exit. */
          case 0:
            /* Redirect child's stdout and stderr streams to our
             * opened output files (if any).
             */
	    if(stdout_fp != NULL)
	        dup2(fileno(stdout_fp), fileno(stdout));
	    if(stderr_fp != NULL)
                dup2(fileno(stderr_fp), fileno(stderr));

	    /* Execute command and arguments. */
	    execvp(argv[0], argv);
	    exit(0);
            break;

          /* We are the parent: wait for child to finish. */
          default:
	    wait(&status);
            break;
        }

        /* Free exploded argument strings and array. */
        StringFreeArray(argv, argc);

	/* Close output files. */
	if(stdout_fp != NULL)
	    fclose(stdout_fp);
	if(stderr_fp != NULL)
	    fclose(stderr_fp);

	return(p);
}

pid_t ExecB(const char *cmd)
{
        return(ExecBNexus(cmd, NULL, NULL, NULL, NULL));
}

pid_t ExecBO(const char *cmd, const char *stdout_path)
{
        return(ExecBNexus(cmd, stdout_path, NULL, "w", NULL));
}

pid_t ExecBOE(const char *cmd, const char *stdout_path, const char *stderr_path)
{
        return(ExecBNexus(cmd, stdout_path, stderr_path, "w", "w"));
}

pid_t ExecBAO(const char *cmd, const char *stdout_path)
{
        return(ExecBNexus(cmd, stdout_path, NULL, "a", NULL));
}

pid_t ExecBAOE(const char *cmd, const char *stdout_path, const char *stderr_path)
{
        return(ExecBNexus(cmd, stdout_path, stderr_path, "a", "a"));
}

