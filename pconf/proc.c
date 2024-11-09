#include "pconf.h"


static int PConfEnableFeature(
	pconf_core_struct *core_ptr, platform_struct *platform_ptr,
	const char *name
);
static int PConfDisableFeature(
        pconf_core_struct *core_ptr, platform_struct *platform_ptr,
        const char *name
);                      
void PConfApplyArgsToPlatform(
	pconf_core_struct *core_ptr, platform_struct *platform_ptr,
	int argc, char *argv[]
);

static int PConfScanFileString(
	const char *filename, const char *s
);
static char *PConfDependMatchFile(
	platform_struct *platform_ptr, int type, const char *path,
	struct stat *stat_buf
);
static int PConfDependCheck(
	pconf_core_struct *core_ptr,
	platform_struct *platform_ptr, depend_struct *depend_ptr,
	const char *feature_name,
	int dep_cur, int dep_total
);
int PConfCheckDepends(
	pconf_core_struct *core_ptr, platform_struct *platform_ptr
);


/*
 *	Enables the feature on the given platform specified by name,
 *	marking its must_exist value to PCONF_MUST_EXIST_YES.
 *
 *	Returns 0 on success, -1 on error, -2 no such feature.
 */
static int PConfEnableFeature(
        pconf_core_struct *core_ptr, platform_struct *platform_ptr,
        const char *name
)
{
	int feature_num;
	feature_struct *feature_ptr;

	if((core_ptr == NULL) || (platform_ptr == NULL) || (name == NULL))
	    return(-1);

	feature_ptr = PConfMatchFeatureByName(
	    platform_ptr, name, &feature_num
	);
	if(feature_ptr == NULL)
	    return(-2);

	feature_ptr->must_exist = PCONF_MUST_EXIST_YES;

	return(0);
}

/*
 *      Disables the feature on the given platform specified by name,
 *      marking its must_exist value to PCONF_MUST_EXIST_NO.
 *
 *      Returns 0 on success, -1 on error, -2 no such feature, -3
 *	if the feature's must_exist was initially PCONF_MUST_EXIST_YES.
 */
static int PConfDisableFeature(
        pconf_core_struct *core_ptr, platform_struct *platform_ptr,
        const char *name
)
{
        int feature_num;
        feature_struct *feature_ptr;

        if((core_ptr == NULL) || (platform_ptr == NULL) || (name == NULL))
            return(-1);

        feature_ptr = PConfMatchFeatureByName(
            platform_ptr, name, &feature_num
        );
        if(feature_ptr == NULL)
            return(-2);

	/* Cannot disable this? */
	if(feature_ptr->must_exist == PCONF_MUST_EXIST_YES)
	    return(-3);

        feature_ptr->must_exist = PCONF_MUST_EXIST_NO;

        return(0);
}



/*
 *	Applies environments and given arguments to the given platform's
 *	values.
 */
void PConfApplyArgsToPlatform(
        pconf_core_struct *core_ptr, platform_struct *platform_ptr,
        int argc, char *argv[]
)
{
	int i, status;
	const char *arg_ptr;

	if((core_ptr == NULL) || (platform_ptr == NULL))
	    return;

	/* Can we initially get values from environment variables? */
	if(!core_ptr->ignore_environments)
	{
	    const char *cstrptr;

	    /* Prefix. */
	    cstrptr = GETENV("PREFIX");
	    if(cstrptr != NULL)
	    {
                FREE(platform_ptr->prefix);
                platform_ptr->prefix = STRDUP(cstrptr);
	    }
            /* Compiler flags. */
            cstrptr = GETENV("CFLAGS");
            if(cstrptr != NULL)
            {
                FREE(platform_ptr->cflags);
                platform_ptr->cflags = STRDUP(cstrptr);
            }
            /* Include directories. */
            cstrptr = GETENV("INC_DIRS");
            if(cstrptr != NULL)
            {
                FREE(platform_ptr->inc_dirs);
                platform_ptr->inc_dirs = STRDUP(cstrptr);
            }
            /* Libraries. */
            cstrptr = GETENV("LIBS");
            if(cstrptr != NULL)
            {
                FREE(platform_ptr->libs);
                platform_ptr->libs = STRDUP(cstrptr);
            }
            /* Library directories. */
            cstrptr = GETENV("LIB_DIRS");
            if(cstrptr != NULL)
            {
                FREE(platform_ptr->lib_dirs);
                platform_ptr->lib_dirs = STRDUP(cstrptr);
            }
            /* C compiler. */
            cstrptr = GETENV("CC"); 
            if(cstrptr != NULL)
            {
                FREE(platform_ptr->cc);
                platform_ptr->cc = STRDUP(cstrptr);
            }
            /* C++ compiler. */
            cstrptr = GETENV("CPP");
            if(cstrptr != NULL)
            {
                FREE(platform_ptr->cpp);
                platform_ptr->cpp = STRDUP(cstrptr);
            }
	    /* Parse LD library paths. */
	    cstrptr = GETENV("LD_LIBRARY_PATH");
            if(cstrptr != NULL)
            {
		char *new_path, *strptr2;

		/* Itterate through value string. */
		while((*cstrptr) != '\0')
		{
		    /* Seek past ' ' and ':' characters. */
		    while(ISBLANK(*cstrptr) || ((*cstrptr) == ':'))
			cstrptr++;

		    /* Skip "-L" prefixes. */
		    if(STRPFX(cstrptr, "-L"))
		    {
			cstrptr += STRLEN("-L");
		    }

		    /* Copy string at position of cstrptr. */
		    new_path = STRDUP(cstrptr);
		    if(new_path != NULL)
		    {
			/* Deliminate at next ' ' or ':' if any. */
                        strptr2 = (char *)STRCHR((char *)new_path, ':');
                        if(strptr2 != NULL)   
                            (*strptr2) = '\0';
			strptr2 = (char *)STRCHR((char *)new_path, ' ');
			if(strptr2 != NULL)
			    (*strptr2) = '\0';

			/* Add this to the library path. */
			STRLISTAPPEND(
			    &platform_ptr->path_library,
			    &platform_ptr->total_path_libraries,
			    new_path
			);

			if(core_ptr->verbose)
			    printf(
 "Adding platform's library path: %s\n",
				new_path
			    );


			FREE(new_path);
			new_path = NULL;
		    }

		    /* Seek cstrptr to next argument, seeking to
		     * next ' ' or ':' character.
		     */
		    while(!ISBLANK(*cstrptr) && ((*cstrptr) != ':') &&
                          ((*cstrptr) != '\0')
		    )
			cstrptr++;
		}
            }
	}

	/* Itterate through each argument. */
	for(i = 0; i < argc; i++)
	{
	    arg_ptr = (const char *)argv[i];
	    if(arg_ptr == NULL)
		continue;

	    /* Skip any args that don't begin with - or --. */
	    if((*arg_ptr) != '-')
		continue;
	    /* Seek past '-' characters. */
	    while((*arg_ptr) == '-')
		arg_ptr++;

	    /* Enable or with? */
	    if(STRCASEPFX(arg_ptr, "enable") ||
               STRCASEPFX(arg_ptr, "with")
	    )
	    {
		/* Enable what? */
		const char *cstrptr = STRCHR(arg_ptr, '=');
		if(cstrptr == NULL)
		    cstrptr = STRCHR(arg_ptr, '-');
		if(cstrptr != NULL)
		{
		    cstrptr++;
		    while(ISBLANK(*cstrptr))
			cstrptr++;

		    status = PConfEnableFeature(
			core_ptr, platform_ptr, cstrptr
		    );
		    switch(status)
		    {
		      case -2:
			if(core_ptr->warnings)
                        {   
                            fprintf(stderr, "%s: ", cstrptr);                   
                            PConfSetColor(core_ptr, stderr,
                                PCONF_COLOR_WARNING
                            );
                            fprintf(stderr,
                                "No such feature to enable.\n" 
                            );
                        }
			break;
		      case -1:
                        if(core_ptr->warnings)
			{
			    fprintf(stderr, "%s: ", cstrptr);
			    PConfSetColor(core_ptr, stderr,
                                PCONF_COLOR_FAILURE
                            );
                            fprintf(stderr,
                                "Error enabling this feature.\n"
                            );
			}
                        break;
		    }
                    PConfSetColor(core_ptr, stderr,
                        PCONF_COLOR_DEFAULT
                    );
		}
                else
                {
                    if(core_ptr->warnings)
                        fprintf(
                            stderr,
            "--%s: Argument must have a value separated by a '='.\n",
                            arg_ptr
                        );
                }
	    }
	    /* Disable or without? */
	    else if(STRCASEPFX(arg_ptr, "disable") ||
                    STRCASEPFX(arg_ptr, "without") ||
                    STRCASEPFX(arg_ptr, "with-out") ||
                    STRCASEPFX(arg_ptr, "with_out")
            )
            {
                /* Disable what? */
                const char *cstrptr = STRCHR(arg_ptr, '=');
                if(cstrptr == NULL)
                    cstrptr = STRCHR(arg_ptr, '-');
                if(cstrptr != NULL)
                {
                    cstrptr++;
                    while(ISBLANK(*cstrptr))
                        cstrptr++;
 
                    status = PConfDisableFeature(
                        core_ptr, platform_ptr, cstrptr
                    );
                    switch(status)
                    {
		      case -3:
			if(core_ptr->warnings)
			{
			    fprintf(stderr, "%s: ", cstrptr);
			    PConfSetColor(core_ptr, stderr,
				PCONF_COLOR_FAILURE
			    );
                            fprintf(
                                stderr,
                       "This feature is not allowed to be disabled.\n"
                            );
			}
                        break;

                      case -2:   
                        if(core_ptr->warnings)
                        {
                            fprintf(stderr, "%s: ", cstrptr);
                            PConfSetColor(core_ptr, stderr,
                                PCONF_COLOR_WARNING
                            );
                            fprintf(
                                stderr,
                                "No such feature to disable.\n"
                            );
			}
                        break;

                      case -1:
			fprintf(stderr, "%s: ", cstrptr);
                        PConfSetColor(core_ptr, stderr,
                            PCONF_COLOR_FAILURE
                        );
			fprintf(
			    stderr,
    "Error encountered while attempting to disable feature.\n"
                        );
                        break;
                    }
                    PConfSetColor(core_ptr, stderr,
                        PCONF_COLOR_DEFAULT
                    );
                }
		else
		{
		    if(core_ptr->warnings)
			fprintf(
			    stderr,
 "--%s: Argument must have a value separated by a '=' or '-' character.\n",
			    arg_ptr
			);
		}
            }
            /* Platform PREFIX? */
            else if(STRCASEPFX(arg_ptr, "PREFIX") ||
                    STRCASEPFX(arg_ptr, "PlatformPREFIX")
            )
            {
                const char *cstrptr = STRCHR(arg_ptr, '=');
                if(cstrptr != NULL)
                {
                    cstrptr++;
                    while(ISBLANK(*cstrptr))
                        cstrptr++;

		    FREE(platform_ptr->prefix);
		    platform_ptr->prefix = STRDUP(cstrptr);
		}
                else            
                {               
                    if(core_ptr->warnings)
                        fprintf(
                            stderr,
            "--%s: Argument must have a value separated by a '='.\n",
                            arg_ptr
                        );  
                }               
	    }
            /* Platform CFLAGS? */
            else if(STRCASEPFX(arg_ptr, "CFLAGS") ||
                    STRCASEPFX(arg_ptr, "PlatformCFLAGS")
            )
            {
                const char *cstrptr = STRCHR(arg_ptr, '=');
                if(cstrptr != NULL)
                {   
                    cstrptr++;
                    while(ISBLANK(*cstrptr))
                        cstrptr++;

                    FREE(platform_ptr->cflags);
                    platform_ptr->cflags = STRDUP(cstrptr);
                }
                else
                {
                    if(core_ptr->warnings)
                        fprintf(
                            stderr,
            "--%s: Argument must have a value separated by a '='.\n",
                            arg_ptr
                        );
                }
            }
            /* Platform INC_DIRS? */
            else if(STRCASEPFX(arg_ptr, "INC_DIRS") ||
                    STRCASEPFX(arg_ptr, "PlatformINC_DIRS") ||
                    STRCASEPFX(arg_ptr, "INCDIRS") ||
                    STRCASEPFX(arg_ptr, "PlatformINCDIRS") ||
                    STRCASEPFX(arg_ptr, "INC_DIR") ||
                    STRCASEPFX(arg_ptr, "PlatformINC_DIR") ||
                    STRCASEPFX(arg_ptr, "INCDIR") ||
                    STRCASEPFX(arg_ptr, "PlatformINCDIR")
            )
            {
                const char *cstrptr = STRCHR(arg_ptr, '=');
                if(cstrptr != NULL)
                {
                    cstrptr++;
                    while(ISBLANK(*cstrptr))
                        cstrptr++;

                    FREE(platform_ptr->inc_dirs);
                    platform_ptr->inc_dirs = STRDUP(cstrptr);
                }
                else
                {
                    if(core_ptr->warnings)
                        fprintf(
                            stderr,
            "--%s: Argument must have a value separated by a '='.\n",
                            arg_ptr
                        );
                }
            }
            /* Platform LIBS? */
            else if(STRCASEPFX(arg_ptr, "LIBS") ||
                    STRCASEPFX(arg_ptr, "PlatformLIBS")
            )
            {
                const char *cstrptr = STRCHR(arg_ptr, '=');
                if(cstrptr != NULL)
                {
                    cstrptr++;
                    while(ISBLANK(*cstrptr))
                        cstrptr++;

                    FREE(platform_ptr->libs);
                    platform_ptr->libs = STRDUP(cstrptr);
                }
                else
                {
                    if(core_ptr->warnings)
                        fprintf(
                            stderr,
            "--%s: Argument must have a value separated by a '='.\n",
                            arg_ptr
                        );
                }
            }
            /* Platform LIB_DIRS? */
            else if(STRCASEPFX(arg_ptr, "LIB_DIRS") ||
                    STRCASEPFX(arg_ptr, "PlatformLIB_DIRS") ||
                    STRCASEPFX(arg_ptr, "LIBDIRS") ||
                    STRCASEPFX(arg_ptr, "PlatformLIBDIRS") ||
                    STRCASEPFX(arg_ptr, "LIB_DIR") ||
                    STRCASEPFX(arg_ptr, "PlatformLIB_DIR") ||
                    STRCASEPFX(arg_ptr, "LIBDIR") ||
                    STRCASEPFX(arg_ptr, "PlatformLIBDIR")
            )
            {
                const char *cstrptr = STRCHR(arg_ptr, '=');
                if(cstrptr != NULL)
                {
                    cstrptr++;
                    while(ISBLANK(*cstrptr))
                        cstrptr++;

                    FREE(platform_ptr->lib_dirs);
                    platform_ptr->lib_dirs = STRDUP(cstrptr);
                }
                else
                {
                    if(core_ptr->warnings)
                        fprintf(
                            stderr,
            "--%s: Argument must have a value separated by a '='.\n",
                            arg_ptr
                        );
                }
            }
	}

	return;
}


/*
 *	Opens the file for reading and returns the number of occurances of
 *	s found in the file. File must be a regular file.
 */
static int PConfScanFileString(
        const char *filename, const char *s
)
{
	int c, matches = 0;
	FILE *fp;

	if((filename == NULL) || (s == NULL))
	    return(matches);

	if((*s) == '\0')
	    return(matches);


	fp = FOPEN(filename, "rb");
	if(fp == NULL)
	    return(matches);

	do
	{
	    c = FGETC(fp);
	    if(c == EOF)
		break;

	    /* Got start of matching string? */
	    if(c == (*s))
	    {
		/* Get next character and match string position. */
		const char *sp = s + 1;
		c = FGETC(fp);

		/* Itterate sp. */
		while((*sp) != '\0')
		{
		    if(((*sp) != c) || (c == EOF))
			break;

		    sp++;
		    c = FGETC(fp);
		}

		/* Got a match if sp itterated to end. */
		if((*sp) == '\0')
		    matches++;
	    }

	} while(1);


	FCLOSE(fp);

	return(matches);
}


/*
 *	Itterates through a list of paths on the given platform
 *	structure and matches each one with the given path.
 *
 *	Type specifies the depend type, one of PCONF_DEP_TYPE_*.
 *
 *	On success, returns a dynamically allocated full path with
 *	must be deallocated by the calling function. Can return NULL on
 *	error.
 */
static char *PConfDependMatchFile(
        platform_struct *platform_ptr, int type, const char *path,
	struct stat *stat_buf
)
{
	int i, len1, len2;
	char *base_path, *new_path;
	char **path_list = NULL;
	int total_paths = 0;


	if((platform_ptr == NULL) || (path == NULL))
	    return(NULL);

	switch(type)
	{
	  case PCONF_DEP_TYPE_PROGRAM:
	    path_list = platform_ptr->path_program;
	    total_paths = platform_ptr->total_path_programs;
	    break;
	  case PCONF_DEP_TYPE_HEADER:
	    path_list = platform_ptr->path_header;
            total_paths = platform_ptr->total_path_headers;
            break;
          case PCONF_DEP_TYPE_LIBRARY:
            path_list = platform_ptr->path_library;
            total_paths = platform_ptr->total_path_libraries;
            break;
          case PCONF_DEP_TYPE_CONFIG:
            path_list = platform_ptr->path_config;
            total_paths = platform_ptr->total_path_configs;
            break;
	  case PCONF_DEP_TYPE_OS:
	  case PCONF_DEP_TYPE_MACHINE:
	    /* Should not call this function for checking these 
	     * dependencies.
	     */
	    fprintf(
		stderr,
"PConfDependMatchFile(): Internal error, dependency is not looking for a file.\n"
	    );
	    return(NULL);
	    break;

          default:	/* PCONF_DEP_TYPE_DATA or PCONF_DEP_TYPE_OTHER. */
            path_list = platform_ptr->path_data;
            total_paths = platform_ptr->total_path_datas;
            break;
	}
	/* Empty path list? */
	if((path_list == NULL) || (total_paths < 1))
	    return(NULL);

	/* Itterate through search paths. */
	new_path = NULL;
	len2 = STRLEN(path);
	for(i = 0; i < total_paths; i++)
	{
	    base_path = path_list[i];
	    if(base_path == NULL)
		continue;

	    /* Deallocate previous new path and get new one. */
	    FREE(new_path);
	    new_path = NULL;

	    /* Prefix paths and allocate new path. */
	    len1 = STRLEN(base_path);
	    new_path = (char *)malloc((len1 + len2 + 10) * sizeof(char));
	    if(new_path != NULL)
	    {
		if((len1 > 0) ?
		    (base_path[len1 - 1] != DIR_DELIMINATOR) : 1
		)
		    sprintf(new_path, "%s%c%s",
                        base_path, DIR_DELIMINATOR, path
                    );
		else
		    sprintf(new_path, "%s%s",
			base_path, path
		    );

		/* Exists? */
		if(!stat(new_path, stat_buf))
		    return(new_path);
	    }
	}

	/* Deallocate failed matched new path. */
	FREE(new_path);
	new_path = NULL;

	return(NULL);
}

/*
 *	Checks dependency on the given depend structure.
 *
 *	Returns false on failure and true on success.
 */
static int PConfDependCheck(
        pconf_core_struct *core_ptr, 
	platform_struct *platform_ptr, depend_struct *depend_ptr,
	const char *feature_name,
	int dep_cur, int dep_total
)
{
	int i, found, missing_depend = 0;
	const char *path_ptr;
	char *new_path;
	const char *cstrptr;
	struct stat stat_buf;


	if((core_ptr == NULL) || (platform_ptr == NULL) || (depend_ptr == NULL))
	    return(0);

	if(core_ptr->verbose)
	    printf("Feature: %s (%i of %i) Dependency: %s...\n",
		feature_name,
		dep_cur + 1, dep_total,
		depend_ptr->name
	    );
	else
	    printf("Checking Dependency: %s...\n", depend_ptr->name);


	/* Special case handling for dependencies that do not look for
	 * files.
	 */
	switch(depend_ptr->type)
	{
	  case PCONF_DEP_TYPE_OS:
	    /* Match operating system name. */
	    cstrptr = (const char *)depend_ptr->os_name;
	    if((cstrptr == NULL) ? 1 : ((*cstrptr) == '\0'))
	    {
		/* No name so impossible to match, return false. */
		return(0);
	    }
	    else
	    {
		if(core_ptr->os_name != NULL)
		{
		    if(!STRCASECMP(core_ptr->os_name, cstrptr))
			return(1);
		    else
			return(0);
		}
		else
		{
		    return(0);
		}
	    }
	    break;

	  case PCONF_DEP_TYPE_MACHINE:
            /* Match machine name. */
            cstrptr = (const char *)depend_ptr->machine_name;
            if((cstrptr == NULL) ? 1 : ((*cstrptr) == '\0'))
            {
                /* No name so impossible to match, return false. */
                return(0);
            }   
            else
            {
                if(core_ptr->machine_name != NULL)
                {
                    if(!STRCASECMP(core_ptr->machine_name, cstrptr))
                        return(1);
                    else
                        return(0);
                }   
                else
                {
                    return(0);
                }
            }
	    break;

	  default:
	    /* All other dependency checks, keep going. */
	    break;
	}

	/* Begin handling all other dependencies that look for files here. */

	/* Itterate through dependency paths. */
	new_path = NULL;
	for(i = 0; i < depend_ptr->total_paths; i++)
	{
	    path_ptr = (const char *)depend_ptr->path[i];
	    if(path_ptr == NULL)
		continue;

	    /* If we're not forcing, then stop checking dependencies
	     * on just one failed dependency.
	     */     
	    if(!core_ptr->force && (missing_depend > 0))
		break;

	    /* Deallocate previous new path. */
	    FREE(new_path);
	    new_path = NULL;

	    /* Reset found marker. */
	    found = 0;

	    /* Check if the path exists on this platform. */
	    new_path = PConfDependMatchFile(
		platform_ptr, depend_ptr->type, path_ptr,
		&stat_buf
	    );
	    if(new_path != NULL)
	    {
		if(core_ptr->verbose)
		{
		    printf("    Scanning: %s...", new_path);
		    fflush(stdout);
		}

		/* Is it a directory? */
#ifdef S_ISDIR
		if(S_ISDIR(stat_buf.st_mode))
#else
		if(0)
#endif
		{
		    /* Accept this as is even if there are
		     * strings to be greped.
		     */
		    if(core_ptr->verbose)
		    {
                        PConfSetColor(core_ptr, stdout,
                            PCONF_COLOR_SUCCESS
                        );
		        printf(" OK ");
			PConfSetColor(core_ptr, stdout,
                            PCONF_COLOR_DEFAULT
                        );
			printf("(Directory)\n");
		    }
		    found = 1;
		}
                /* Is it a character device? */
#ifdef S_ISCHR
                if(S_ISCHR(stat_buf.st_mode))
#else            
                if(0)
#endif
                {
                    /* Accept this as is even if there are
                     * strings to be greped.
                     */
                    if(core_ptr->verbose)
                    {
                        PConfSetColor(core_ptr, stdout,   
                            PCONF_COLOR_SUCCESS
                        );    
                        printf(" OK ");  
                        PConfSetColor(core_ptr, stdout,
                            PCONF_COLOR_DEFAULT
                        );
                        printf("(Character Device)\n");
                    }
		    found = 1;
                }
                /* Is it a block device? */
#ifdef S_ISBLK
                if(S_ISBLK(stat_buf.st_mode))
#else
                if(0)
#endif
                {
                    /* Accept this as is even if there are
                     * strings to be greped.
                     */
                    if(core_ptr->verbose)
                    {
                        PConfSetColor(core_ptr, stdout,   
                            PCONF_COLOR_SUCCESS
                        );    
                        printf(" OK ");  
                        PConfSetColor(core_ptr, stdout,
                            PCONF_COLOR_DEFAULT
                        );
                        printf("(Block Device)\n");
                    }
		    found = 1;
                }
                /* Is it a FIFO pipe? */
#ifdef S_ISFIFO
                if(S_ISFIFO(stat_buf.st_mode))
#else
                if(0)
#endif
                {
                    /* Accept this as is even if there are
                     * strings to be greped.
                     */
                    if(core_ptr->verbose)
                    {
                        PConfSetColor(core_ptr, stdout,   
                            PCONF_COLOR_SUCCESS
                        );    
                        printf(" OK ");  
                        PConfSetColor(core_ptr, stdout,
                            PCONF_COLOR_DEFAULT
                        );
                        printf("(FIFO Pipe)\n");
                    }
		    found = 1;
                }
                /* Is it a socket? */
#ifdef S_ISSOCK
                if(S_ISSOCK(stat_buf.st_mode))
#else
                if(0)
#endif
                {
                    /* Accept this as is even if there are
                     * strings to be greped.
                     */
                    if(core_ptr->verbose)
                    {
                        PConfSetColor(core_ptr, stdout,   
                            PCONF_COLOR_SUCCESS
                        );    
                        printf(" OK ");  
                        PConfSetColor(core_ptr, stdout,
                            PCONF_COLOR_DEFAULT
                        );
                        printf("(Socket)\n");
                    }
		    found = 1;
                }
                /* Is it a regular file? */
#ifdef S_ISREG
                if(S_ISREG(stat_buf.st_mode))
#else
                if(1)
#endif
                {
		    /* Check for corresponding string for grepping on
		     * this dependency structure. If there is one, then
		     * grep for it.
		     */
		    if((i >= 0) && (i < depend_ptr->total_grep_strings))
		    {
			const char *grep_string = depend_ptr->grep_string[i];
			/* Grep string not NULL and not empty? */
			if((grep_string == NULL) ? 0 : ((*grep_string) != '\0'))
			{
			    if(PConfScanFileString(
				new_path, grep_string
			    ) > 0)
			    {
				if(core_ptr->verbose)
                                {
                                    PConfSetColor(core_ptr, stdout,
                                        PCONF_COLOR_SUCCESS
                                    );    
                                    printf(" OK\n");
                                    PConfSetColor(core_ptr, stdout,
                                        PCONF_COLOR_DEFAULT
                                    );
                                }
				found = 1;
			    }
			    else
			    {
				if(core_ptr->verbose)
				{
                                    PConfSetColor(core_ptr, stdout,
                                        PCONF_COLOR_FAILURE
                                    );
                                    printf(" Failed\n");
                                    PConfSetColor(core_ptr, stdout,
                                        PCONF_COLOR_DEFAULT
                                    );
				}

				if(core_ptr->warnings)
				{
                                    PConfSetColor(core_ptr, stderr,
                                        PCONF_COLOR_DEFAULT
                                    );
                                    fprintf(stderr,  "    *** ");
				    PConfSetColor(core_ptr, stderr,
                                        PCONF_COLOR_FAILURE
                                    );
                                    fprintf(
					stderr,
                                    "Could not find `%s' in file %s!",
                                        grep_string, new_path
                                    );
                                    PConfSetColor(core_ptr, stderr,
                                        PCONF_COLOR_DEFAULT
                                    );
                                    fprintf(stderr,  " ***\n");
				}
			    }
			}
			else
			{
			    /* Grep string is empty, imply always match. */
			    found = 1;
			    if(core_ptr->verbose)
			    {
                                PConfSetColor(core_ptr, stdout,
                                    PCONF_COLOR_SUCCESS
                                );
                                printf(" OK\n");
                                PConfSetColor(core_ptr, stdout,
                                    PCONF_COLOR_DEFAULT
                                );
			    }
			}
		    }
		    else
		    {
			/* No grep string, so imply always match. */
			if(core_ptr->verbose)
                        {
                            PConfSetColor(core_ptr, stdout,
                                PCONF_COLOR_SUCCESS
                            );
                            printf(" OK\n");
                            PConfSetColor(core_ptr, stdout,
                                PCONF_COLOR_DEFAULT
                            );
                        }
			/* Mark this dependency as found. */
			found = 1;
		    }
                }

		/* Not found? */
		if(!found)
		{
		    /* Increment missing dependencies count. */
		    missing_depend++;
		}
	    }
	    else
	    {
		/* Object does not exist. */
		if(core_ptr->warnings)
		{
                    PConfSetColor(core_ptr, stderr,
                        PCONF_COLOR_DEFAULT
                    );
		    fprintf(stderr, "    *** ");
		    PConfSetColor(core_ptr, stderr,
                        PCONF_COLOR_WARNING
                    );
		    fprintf(
			stderr,
          "Object `%s' not found in defined platform paths!",
			path_ptr
		    );
		    PConfSetColor(core_ptr, stderr,
                        PCONF_COLOR_DEFAULT
                    );
		    fprintf(stderr, " ***\n");
		}

		/* Increment missing dependency count. */
		missing_depend++;
	    }
	}	/* Itterate through dependency paths. */


	/* Deallocate new path just in case. */
	FREE(new_path);
	new_path = NULL;

	/* All depends met? */
	if(missing_depend <= 0)
	    return(1);
	else
	    return(0);
}


/*
 *	Checks for dependencies on all enabled features.
 *
 *	Any feature's must_exist marked as PCONF_MUST_EXIST_PREFERRED will
 *	be set to PCONF_MUST_EXIST_YES if they exist or set to
 *	PCONF_MUST_EXIST_NO if they do not exist.
 *
 *	Returns the number of failed dependencies who's must_exist
 *	is set to PCONF_MUST_EXIST_YES.
 */
int PConfCheckDepends(
        pconf_core_struct *core_ptr, platform_struct *platform_ptr
)
{
	int i, failed_depends = 0;
	int depend_progress_ratio = 0;	/* In percent. */
	feature_struct *feature_ptr;


	if((core_ptr == NULL) || (platform_ptr == NULL))
	    return(failed_depends);


	/* Itterate through each feature. */
	for(i = 0; i < platform_ptr->total_features; i++)
	{
	    feature_ptr = platform_ptr->feature[i];
	    if(feature_ptr == NULL)
		continue;

	    /* Update progress ratio. */
	    if(platform_ptr->total_features > 0)
		depend_progress_ratio = (i + 1) * 100 /
		    platform_ptr->total_features;


	    /* Do not want this feature? */
	    if(feature_ptr->must_exist == PCONF_MUST_EXIST_NO)
		continue;

	    /* Would like to have this feature (preffered)? */
	    if(feature_ptr->must_exist == PCONF_MUST_EXIST_PREFERRED)
	    {
		int n, k = 0;
		depend_struct *depend_ptr;

		/* Check if we have dependencies. */
		for(n = 0; n < feature_ptr->total_depends; n++)
		{
		    depend_ptr = feature_ptr->depend[n];
		    if(depend_ptr == NULL)
			continue;

		    if(depend_ptr->must_exist == PCONF_MUST_EXIST_YES)
		    {
			/* Dependency found? */
			if(!PConfDependCheck(
			    core_ptr, platform_ptr, depend_ptr, 
			    feature_ptr->name,
			    i, platform_ptr->total_features
			))
			{
			    k++;
			    /* Do not count this as a failed dependency. */
			}
		    }
		}
		/* No failed dependencies? */
		if(k == 0)
		{
		    if(core_ptr->warnings)
		    {
			printf(
                            "    Preferred feature `%s' ",
			    feature_ptr->name
			);
                        PConfSetColor(core_ptr, stdout,
                            PCONF_COLOR_SUCCESS
                        );
                        printf("detected and enabled");
                        PConfSetColor(core_ptr, stdout,
                            PCONF_COLOR_DEFAULT
                        );
                        printf(".\n");
		    }
		    /* No failed dependencies, so mark this as enabled. */
		    feature_ptr->must_exist = PCONF_MUST_EXIST_YES;
		}
                else
                {
		    /* Got one or more failed dependency. */
		    if(core_ptr->warnings)
		    {
			printf("    Preferred (");
			PConfSetColor(core_ptr, stdout,
                            PCONF_COLOR_SUCCESS
                        );
                        printf("not required");
			PConfSetColor(core_ptr, stdout,
                            PCONF_COLOR_DEFAULT
                        );
			printf(") feature `%s' ",
                            feature_ptr->name
                        );
                        PConfSetColor(core_ptr, stdout,
                            PCONF_COLOR_WARNING
                        );
                        printf("not detected and disabled");
                        PConfSetColor(core_ptr, stdout,
                            PCONF_COLOR_DEFAULT
                        );
                        printf(".\n");
		    }
                    /* Got failed dependencies, so mark this as disabled. */
                    feature_ptr->must_exist = PCONF_MUST_EXIST_NO;
                }
	    }
	    /* Must have this feature? */
	    else if(feature_ptr->must_exist == PCONF_MUST_EXIST_YES)
	    {
                int n, k = 0;
                depend_struct *depend_ptr;

                /* Check if we have dependencies. */
                for(n = 0; n < feature_ptr->total_depends; n++)
                {
                    depend_ptr = feature_ptr->depend[n];
                    if(depend_ptr == NULL)
                        continue;

		    /* If not forcing and we got one or more failed 
		     * dependencies then we should give up immediatly.
		     */
		    if(!core_ptr->force && (failed_depends > 0))
			break;

		    /* Check for dependency of feature only if feature
		     * says the dependency must exist.
		     */
                    if(depend_ptr->must_exist == PCONF_MUST_EXIST_YES)
                    {
                        /* Dependency found? */
                        if(!PConfDependCheck(
			    core_ptr, platform_ptr, depend_ptr,
			    feature_ptr->name,
			    i, platform_ptr->total_features
			))
                        {
			    k++;
                            failed_depends++;
			}
		    }
		}
		/* Got more than one failed dependency? */
		if(k > 0)
		{
                    PConfSetColor(core_ptr, stderr,
                        PCONF_COLOR_DEFAULT
                    );
		    fprintf(stderr, "    *** ");
                    PConfSetColor(core_ptr, stderr,
                        PCONF_COLOR_FAILURE
                    );
		    fprintf(
			stderr,
"Could not find all dependencies for required feature `%s'!",
			feature_ptr->name
		    );
		    PConfSetColor(core_ptr, stderr,
                        PCONF_COLOR_DEFAULT
                    );
		    fprintf(stderr, " ***\n");

                    /* Got failed dependencies so mark this as disabled
		     * if we are not forcing.
		     */
		    if(!core_ptr->force)
			feature_ptr->must_exist = PCONF_MUST_EXIST_NO;

		    /* This is a halting error, halt if not forcing. */
		    if(core_ptr->force)
		    {
			if(core_ptr->verbose && core_ptr->warnings)
			    fprintf(
				stderr,
 "*** Force enabled, continuing dependency check... ***\n"
			    );
		    }
		    else
		    {
			break;
		    }
		}
	    }
	}	/* Itterate through each feature. */


	return(failed_depends);
}
