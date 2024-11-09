#include "xsw.h"


int CmdServScript(const char *arg)
{
	int i, n;
	char filename[PATH_MAX + NAME_MAX];
	char stringa[PATH_MAX + NAME_MAX + 256];
	char *strptr, *strptr2;

	char **argv = NULL;
	int argc = 0;

        /* Map file browser? */
        if((arg == NULL) ? 1 : ((*arg) == '\0'))
        {
	    XSWMapFB(NULL, PRI_FB_LOADOP_SERVER_SCRIPT);
	    return(0);
	}


	strncpy(filename, arg, PATH_MAX + NAME_MAX);
	filename[PATH_MAX + NAME_MAX - 1] = '\0';
    strptr = strchr(filename, ' ');
	if(strptr != NULL) *strptr = '\0';

	strptr2 = PathSubHome(filename);
	if(strptr2 != NULL)
	{
	    strncpy(filename, strptr2, PATH_MAX + NAME_MAX);
	    filename[PATH_MAX + NAME_MAX - 1] = '\0';
	}

	/* Get arguments if any. */
	strptr = (char*) strchr(arg, ' ');
	if(strptr != NULL)
	{
	    argv = ExecExplodeCommand(strptr, &argc);
	    if(argc < 0)
		argc = 0;
	}

	/* Set first argument to be that of the filename. */
	argc++;
	argv = (char **)realloc(argv, argc * sizeof(char *));
	if(argv == NULL)
	    argc = 0;
	for(i = argc - 1, n = 0; i > n; i--)
	    argv[i] = argv[i - 1];

	if((argv != NULL) && (argc >= 1))
	    argv[0] = StringCopyAlloc(filename);


	/* Start server script. */
	i = ServScriptStart(filename, argv, argc);
	if(ServScriptIsAllocated(i))
	{
	    sprintf(stringa, "Sending: %s...", filename);
	    MesgAdd(stringa, xsw_color.standard_text);
	}
	else
	{
            sprintf(stringa, "%s: Cannot send.", filename);
            MesgAdd(stringa, xsw_color.standard_text);
	}

	/* Do not deallocate argv, ServScript*() functions will do that */


	return(0);
}
