#include "pconf.h"


void PConfSetColor(pconf_core_struct *core_ptr, FILE *stream, int color_code);
static platform_struct *PConfMatchPlatformByName(
	platform_struct **platform, int total_platforms,
        const char *name, int *n
);
feature_struct *PConfMatchFeatureByName(
	platform_struct *platform_ptr,
        const char *name, int *n
);
static void PConfFreeCore(pconf_core_struct *core_ptr);
void PConfPrintHR(pconf_core_struct *core_ptr);
static void PConfPrintHelp(const char *prog_name);
static void PConfPrintPlatform(
	pconf_core_struct *core_ptr,
        platform_struct *platform_ptr, int print_level,
	int print_for_review
);
static void PConfPrintPlatforms(
	pconf_core_struct *core_ptr,
	platform_struct **platform, int total_platforms,
	const char *specific_platform,
	int print_level,
	int print_for_review
);
static int PConfPrereqCheck(
	char *platforms_ini, int argc, char **argv
);



/*
 *	Sets the stream for ansi color, color_code must be one of
 *	PCONF_COLOR_*.
 */
void PConfSetColor(pconf_core_struct *core_ptr, FILE *stream, int color_code)
{
	if(stream == NULL)
	    return;

	if(core_ptr != NULL)
	{
	    if(!core_ptr->colors)
		return;
	}

	if(color_code == PCONF_COLOR_DEFAULT)
	    fprintf(stream, "\033[0;39m");
	else if(color_code == PCONF_COLOR_SELECTABLE)
	    fprintf(stream, "\033[4;34m");
        else if(color_code == PCONF_COLOR_HEADING)
            fprintf(stream, "\033[1;m");
/*
            fprintf(stream, "\033[1;39m");
 */
	else
	    fprintf(stream, "\033[1;%im", color_code);

	return;
}

/*
 *	Returns the platform matching the specified name or NULL on failed
 *	match.
 */
static platform_struct *PConfMatchPlatformByName(
        platform_struct **platform, int total_platforms,
        const char *name, int *n
)
{
	int i;
	platform_struct *platform_ptr;

	if(n != NULL)
	    (*n) = -1;

	if((platform == NULL) || (total_platforms < 1) || (name == NULL))
	    return(NULL);

	for(i = 0; i < total_platforms; i++)
	{
	    platform_ptr = platform[i];
	    if(platform_ptr == NULL)
		continue;

	    if(platform_ptr->name == NULL)
		continue;

	    if(!STRCASECMP(platform_ptr->name, name))
	    {
		if(n != NULL)
		    (*n) = i;
		return(platform_ptr);
	    }
	}

	return(NULL);
}

/*
 *      Returns the feature matching the specified name or NULL on failed
 *      match.
 */
feature_struct *PConfMatchFeatureByName(
        platform_struct *platform_ptr,
        const char *name, int *n
)
{
        int i;
        feature_struct *feature_ptr;

        if(n != NULL)
            (*n) = -1;

        if((platform_ptr == NULL) || (name == NULL))
            return(NULL);

        for(i = 0; i < platform_ptr->total_features; i++)
        {
            feature_ptr = platform_ptr->feature[i];
            if(feature_ptr == NULL)
                continue;

            if(feature_ptr->name == NULL)
                continue;

            if(!STRCASECMP(feature_ptr->name, name))
            {
                if(n != NULL)
                    (*n) = i;
                return(feature_ptr);
            }
        }

        return(NULL);
}

/*
 *	Deallocates the core structure and all its allocated resources.
 */
static void PConfFreeCore(pconf_core_struct *core_ptr)
{
	char **s;

#define DO_FREE_STRING	\
{ \
 if((*s) != NULL) \
 { \
  FREE(*s); \
  (*s) = NULL; \
 } \
}

	if(core_ptr == NULL)
	    return;

	PConfFreePlatforms(
	    &core_ptr->platform,
	    &core_ptr->total_platforms
	);

	PConfFreePlatform(core_ptr->selected_platform);
	core_ptr->selected_platform = NULL;

	s = &core_ptr->makefile_output;
	DO_FREE_STRING
	s = &core_ptr->makefile_input_prepend;
	DO_FREE_STRING
	s = &core_ptr->makefile_input_append;
	DO_FREE_STRING

	s = &core_ptr->message_configure_startup;
	DO_FREE_STRING
	s = &core_ptr->message_platform_unsupported;
        DO_FREE_STRING
        s = &core_ptr->message_depend_failed;
        DO_FREE_STRING
        s = &core_ptr->message_success;
        DO_FREE_STRING

        s = &core_ptr->os_name;
        DO_FREE_STRING
        s = &core_ptr->machine_name;
        DO_FREE_STRING

	FREE(core_ptr);
	core_ptr = NULL;

#undef DO_FREE_STRING

	return;
}


/*
 *	Prints horizontal rule.
 */
void PConfPrintHR(pconf_core_struct *core_ptr)
{
	int i, len = 80;

	if(core_ptr->screen_width > 0)
	    len = core_ptr->screen_width;

	for(i = 0; i < len; i++)
	    FPUTC('-', stdout);
	FPUTC('\n', stdout);

	return;
}


/*
 *	Prints help for this program as the name specified by prog_name.
 *
 *	Note that we do not show the need for the first argument since we
 *	are printing the usage designed to be shown when called from the
 *	configure script.
 */
static void PConfPrintHelp(const char *prog_name)
{
	if(prog_name == NULL)
	    prog_name = "pconf";

	printf(
"Usage: ./configure [platform] [modifiers] [options]\n"
	);
	printf(
"       ./configure [options]\n"
        );
	printf(
"\n\
    Where [platform] specifies the name of the platform to configure for (use\n\
    the --listall argument to see a complete list of supported platforms).\n\
\n\
    Where [modifiers] can be any of the following modifications of features\n\
    or parameters (use the --listall argument to see a list of features\n\
    and parameters of each platform):\n\
\n\
        --enable=<feature>      Enables the specified <feature>.\n\
        --disable=<feature>     Disables the specified <feature>.\n\
        --<parameter>=<value>   Override <parameter> with new <value>,\n\
                                example: --prefix=/usr/local/\n\
\n\
    Where [options] can be any of the following:\n\
\n\
        --list[=<platform>]     Show descriptions and features of each\n\
                                platform (or just a specific platform if\n\
                                <platform> is specified).\n\
        --listall[=<platform>]  Show descriptions, features, dependancies,\n\
                                and all other details of each platform\n\
                                (or just a specific platform if <platform>\n\
                                is specified).\n\
        --v                     Prints extra verbose messages (also displays\n\
                                post configuration report).\n\
        --interactive           Use interactive mode when needed.\n\
        --no-warnings           Do not print warning messages.\n\
        --no-colors             Disable ANSI color.\n\
        --force                 Ignore errors when possible.\n\
        --ignore-environments   Do not use any environment variable values.\n\
        --help                  Prints (this) help screen and exits.\n\
        --version               Prints version information and exits.\n\
\n\
    Examples:\n\
\n\
        ./configure --listall\n\
\n\
        ./configure Linux --prefix=/usr/local/\n\
\n\
        ./configure UNIX -v --prefix=/usr --CFLAGS=\"-O2 -g -Wall\"\n\
\n"
	);

	return;
}


/*
 *	Prints the given platform.
 *
 *	The given print_level determines how much info to print
 *	out:
 *
 *	0	Print just platform names (if any).
 *	1	Print platform names, desc, and available features.
 *	2	Print platform names, desc, features, and their
 *		dependancies.
 *	3	Print everything including global search paths and other
 *		misc info.
 */
static void PConfPrintPlatform(
	pconf_core_struct *core_ptr,
        platform_struct *platform_ptr, int print_level,
	int print_for_review
)
{
	int n, j;
	feature_struct *feature_ptr;
	depend_struct *depend_ptr;


	if(platform_ptr == NULL)
	    return;

#define PLATFORMS_INDENT	"    "

	/* Print platform name? */
	if(print_level >= 0)
	{
		if(print_level >= 1)
		{
		    PConfSetColor(core_ptr, stdout, PCONF_COLOR_HEADING);
		    printf("Platform");
		    PConfSetColor(core_ptr, stdout, PCONF_COLOR_DEFAULT);
		    printf(": ");
		    PConfSetColor(core_ptr, stdout, PCONF_COLOR_SELECTABLE);
		    printf("%s\n", platform_ptr->name);
		    PConfSetColor(core_ptr, stdout, PCONF_COLOR_DEFAULT);
		}
		else
		{
		    printf("%s\n",
                        platform_ptr->name
                    );
		}

		/* Description (if any). */
		if((platform_ptr->description != NULL) &&
                   (print_level >= 1)
		)
		{
		    printf(
			PLATFORMS_INDENT "Description: %s\n",
			platform_ptr->description
		    );
		}

                /* PREFIX (if any). */
                if((platform_ptr->prefix != NULL) &&
                   (print_level >= 3)
                )
                {
                    printf(
                        PLATFORMS_INDENT "PREFIX: %s\n",
                        platform_ptr->prefix
                    );
                }
                /* CFLAGS (if any). */
                if((platform_ptr->cflags != NULL) &&
                   (print_level >= 3)   
                )
                {
                    printf(
                        PLATFORMS_INDENT "CFLAGS: %s\n",
                        platform_ptr->cflags
                    );
                }
                /* INC_DIRS (if any). */
                if((platform_ptr->inc_dirs != NULL) &&
                   (print_level >= 3)
                )
                {
                    printf(
                        PLATFORMS_INDENT "INC_DIRS: %s\n",
                        platform_ptr->inc_dirs
                    );
                }
                /* LIBS (if any). */
                if((platform_ptr->libs != NULL) &&
                   (print_level >= 3)
                )
                {
                    printf(
                        PLATFORMS_INDENT "LIBS: %s\n",
                        platform_ptr->libs
                    );
                }
                /* LIB_DIRS (if any). */
                if((platform_ptr->lib_dirs != NULL) &&
                   (print_level >= 3)
                )
                {
                    printf(
                        PLATFORMS_INDENT "LIB_DIRS: %s\n",
                        platform_ptr->lib_dirs
                    );
                }
                /* CC (if any). */
                if((platform_ptr->cc != NULL) &&
                   (print_level >= 3)
                )
                {
                    printf(
                        PLATFORMS_INDENT "CC: %s\n",
                        platform_ptr->cc
                    );
                }
                /* CPP (if any). */
                if((platform_ptr->cpp != NULL) &&
                   (print_level >= 3)
                )
                {
                    printf(
                        PLATFORMS_INDENT "CPP: %s\n",
                        platform_ptr->cpp
                    );
                }
	}

	/* Print features on this platform? */
	if(print_level >= 1)
	{
#define FEATURES_INDENT "        "

		for(n = 0; n < platform_ptr->total_features; n++)
		{
		    feature_ptr = platform_ptr->feature[n];
		    if(feature_ptr == NULL)
			continue;

		    if(feature_ptr->name != NULL)
		    {
                        PConfSetColor(core_ptr, stdout, PCONF_COLOR_HEADING);
                        printf(PLATFORMS_INDENT "Feature");
                        PConfSetColor(core_ptr, stdout, PCONF_COLOR_DEFAULT);
                        printf(": ");
			PConfSetColor(core_ptr, stdout, PCONF_COLOR_SELECTABLE);
			printf("%s", feature_ptr->name);
			PConfSetColor(core_ptr, stdout, PCONF_COLOR_DEFAULT);
			/* Do not print newline after feature name, the must have
			 * value will be added after it.
			 */
		    }

		    /* Print must exist in terms of printing for review. */
		    if(print_for_review)
		    {
			/* We're printing for review, meaning configuration has 
			 * already completed. We should print its enabled/disabled
			 * state.
			 */
			printf(" (");
			switch(feature_ptr->must_exist)
                        {
                          case PCONF_MUST_EXIST_NO:	/* Disabled. */
			    PConfSetColor(core_ptr, stdout, PCONF_COLOR_WARNING);
                            printf("Disabled");
			    break;

                          case PCONF_MUST_EXIST_YES:	/* Enabled. */
                            PConfSetColor(core_ptr, stdout, PCONF_COLOR_SUCCESS);
                            printf("Enabled");
                            break;

                          default:	/* No set properly. */
                            PConfSetColor(core_ptr, stdout, PCONF_COLOR_FAILURE);
                            printf("Undefined");
                            break;
			}
			PConfSetColor(core_ptr, stdout, PCONF_COLOR_DEFAULT);
			printf(")\n");
		    }
		    else
		    {
		        printf(" (");
                        switch(feature_ptr->must_exist)
		        {
		          case PCONF_MUST_EXIST_NO:
			    PConfSetColor(core_ptr, stdout, PCONF_COLOR_FAILURE);
			    printf("Not Checked");
			    PConfSetColor(core_ptr, stdout, PCONF_COLOR_DEFAULT);
                            printf(") To Enable: --enable=\"%s\"\n", 
                                feature_ptr->name
                            );
			    break;

                          case PCONF_MUST_EXIST_PREFERRED:
			    PConfSetColor(core_ptr, stdout, PCONF_COLOR_WARNING);
                            printf("Preferred");
			    PConfSetColor(core_ptr, stdout, PCONF_COLOR_DEFAULT);
			    printf(") To Disable: --disable=\"%s\"\n",
				feature_ptr->name
			    );
                            break;

                          case PCONF_MUST_EXIST_YES:
			    PConfSetColor(core_ptr, stdout, PCONF_COLOR_SUCCESS);
                            printf("Required");
			    PConfSetColor(core_ptr, stdout, PCONF_COLOR_DEFAULT);
			    printf(")\n");
                            break;
		        }
		    }

		    /* Print feature description. */
		    if((print_level >= 2) && (feature_ptr->description != NULL))
		    {
                        printf(
                            FEATURES_INDENT "Description: %s\n",
                            feature_ptr->description
                        );
		    }

		    /* Print feature home page url. */
                    if((print_level >= 3) && (feature_ptr->url_homepage != NULL))
                    {
                        printf(
                            FEATURES_INDENT "URL Home Page: %s\n",
                            feature_ptr->url_homepage
                        );
                    }
                    /* Print feature download url. */
                    if((print_level >= 3) && (feature_ptr->url_download != NULL))
                    {
                        printf(
                            FEATURES_INDENT "URL Download: %s\n",
                            feature_ptr->url_download
                        );
                    }


		    /* Print library dependancies for this feature? */
		    if(print_level >= 2)
		    {
#define DEPEND_INDENT	"            "
			if(feature_ptr->total_depends > 0)
			    printf(
				FEATURES_INDENT "Depends On:\n"
			    );

                        for(j = 0; j < feature_ptr->total_depends; j++)
                        {
			    depend_ptr = feature_ptr->depend[j];
			    if(depend_ptr == NULL)
				continue;

                            if(depend_ptr->name != NULL)
                                printf(
                                    DEPEND_INDENT "%s\n",
                                    depend_ptr->name
                                );
			}
#undef DEPEND_INDENT
		    }
		}
#undef FEATURES_INDENT
	}

	return;
}

/*
 *      Prints list of platforms.
 *
 *      If no platforms exist, then "No platforms defined." will be
 *      printed.
 *
 *      The given print_level determines how much info to print
 *      out:
 *
 *      0       Print just platform names (if any).
 *      1       Print platform names, desc, and available features.
 *      2       Print platform names, desc, features, and their
 *              dependancies.
 *      3       Print everything including global search paths and other
 *              misc info.
 */
static void PConfPrintPlatforms(
	pconf_core_struct *core_ptr,
        platform_struct **platform, int total_platforms,
        const char *specific_platform,
        int print_level,
	int print_for_review
)
{
        int i;
        platform_struct *platform_ptr;
        int platforms_printed = 0;

        /* No platforms defined? */
        if((platform == NULL) || (total_platforms < 1))
        {
            printf(
"No platforms defined.\n"
            );
            return;
        }

        /* Not printing for a specific platform? */
        if((specific_platform == NULL) && (total_platforms > 0))
            printf("Available platforms:\n\n");

        /* Itterate through all platforms. */
        for(i = 0; i < total_platforms; i++)
        {
            platform_ptr = platform[i];
            if(platform_ptr == NULL)
                continue;

            /* No name implies skip this platform. */
            if(platform_ptr->name == NULL)
                continue;

            /* Printing for specific platform? */
            if(specific_platform != NULL)
            {
                /* If names don't match then skip. */
                if(STRCASECMP(specific_platform, platform_ptr->name))
                    continue;
            }

	    /* Print this platform. */
	    PConfPrintPlatform(
		core_ptr, platform_ptr, print_level,
		print_for_review
	    );

            /* Print separator if printing above level 0. */
            if(print_level > 0)
                printf("\n");

            /* Increment platforms printed counter. */
            platforms_printed++;

#undef PLATFORMS_INDENT
        }

        /* No platforms printed? */
        if(platforms_printed == 0)
        {
            if(specific_platform != NULL)
                printf("No such platform \"%s\"\n", specific_platform);
        }

        return;
}


/*	Checks if required arguments have been recieved, returns non-zero
 *	if there is something missing.
 *
 *	Return values are as follows:
 *
 *	0	Success
 *	-1	Missing platforms_ini or cannot open it
 *	-2	Insufficient info, suggest print help
 *	-3	Missing platform name argument, suggest print help
 */
static int PConfPrereqCheck(
        char *platforms_ini, int argc, char **argv
)
{
	FILE *fp;

	/* No platforms.ini file? */
	if(platforms_ini == NULL)
	{
	    /* Better have two arguments atleast then. */
	    if(argc < 2)
		return(-2);
	    else
		platforms_ini = argv[1];
	}
	else
	{
	    /* Platforms.ini file given, but check if command line
	     * specifies alternate one.
	     */
	    if(argc > 1)
		platforms_ini = argv[1];
	}

	/* Check if we can open the platforms.ini file. */
	fp = FOPEN(platforms_ini, "rb");
	if(fp == NULL)
	{
	    return(-1);
	}
	else
	{
	    FCLOSE(fp);
	}

	/* Did we get a third argument, the platform? */
	if(argc < 3)
	    return(-3);


	/* All checks passed. */
	return(0);
}
        

/*
 *	Return codes:
 *
 *	0	Success
 *	1	Invalid value
 *	2	Missing or unspecified resource
 *	3	System error
 *	4	User info (ie help, version, or list platforms)
 */
int main(int argc, char *argv[])
{
	int i, status;
	const char *arg_ptr;
	int verbose = 0;
	int warnings = 1;
	int colors = 1;
	int interactive = 0;
	int force = 0;
	int ignore_environments = 0;
	char *platforms_ini = NULL;
	char *platform_name = NULL;
	char *specific_platform = NULL;
	int print_platforms_level = -1;
	pconf_core_struct *core_ptr;
	int selected_platform_num;
	platform_struct *selected_platform;


#define DO_FREE_LOCALS	\
{ \
 FREE(platforms_ini); \
 platforms_ini = NULL; \
 \
 FREE(platform_name); \
 platform_name = NULL; \
 \
 FREE(specific_platform); \
 specific_platform = NULL; \
}

        /* Parse independent arguments (ie help and version). */
        for(i = 1; i < argc; i++)
        {
            arg_ptr = (const char *)argv[i];
            if(arg_ptr == NULL)
                continue; 

            /* Help. */
            if(STRCASEPFX(arg_ptr, "--h") ||
               STRCASEPFX(arg_ptr, "-h") ||
               !STRCASECMP(arg_ptr, "-?") ||
               !STRCASECMP(arg_ptr, "/?")
            )
            {
                PConfPrintHelp((argc > 0) ? argv[0] : NULL);
                DO_FREE_LOCALS
                return(4);
            }
            /* Version. */ 
            else if(STRCASEPFX(arg_ptr, "--ver") ||
                    STRCASEPFX(arg_ptr, "-ver")
            )
            {
                printf(
		    "%s Version %s\n",
		    PROG_NAME, PROG_VERSION
		);
                DO_FREE_LOCALS  
                return(4);
            }
	    /* Verbose. */
	    else if(!STRCASECMP(arg_ptr, "-v"))
            {
		verbose = 1;
            }
            /* No warnings. */
            else if(!STRCASECMP(arg_ptr, "--no_warnings") ||
                    !STRCASECMP(arg_ptr, "--no_warn") ||
                    !STRCASECMP(arg_ptr, "--no-warnings") ||
                    !STRCASECMP(arg_ptr, "--no-warn") ||
                    !STRCASECMP(arg_ptr, "-no_warnings") ||
                    !STRCASECMP(arg_ptr, "-no_warn") ||
                    !STRCASECMP(arg_ptr, "-no-warnings") ||
                    !STRCASECMP(arg_ptr, "-no-warn") ||
                    !STRCASECMP(arg_ptr, "--nowarnings") ||
                    !STRCASECMP(arg_ptr, "-nowarn")
	    )
            {
                warnings = 0;
            }
            /* Interactive. */
            else if(!STRCASECMP(arg_ptr, "--interactive") ||
                    !STRCASECMP(arg_ptr, "-interactive") || 
                    !STRCASECMP(arg_ptr, "--i") ||
                    !STRCASECMP(arg_ptr, "-i")
	    )
            {
                interactive = 1;
            }
            /* Force. */
            else if(!STRCASECMP(arg_ptr, "--force") || 
                    !STRCASECMP(arg_ptr, "-force")
            )
            {
                force = 1;
            }
	    /* No colors. */
	    else if(!STRCASECMP(arg_ptr, "--no-colors") ||
                    !STRCASECMP(arg_ptr, "--no-colours") ||
                    !STRCASECMP(arg_ptr, "--no_colors") ||
                    !STRCASECMP(arg_ptr, "--no_colours") ||
                    !STRCASECMP(arg_ptr, "--no-color") ||
                    !STRCASECMP(arg_ptr, "--no-colour") ||
                    !STRCASECMP(arg_ptr, "--no_color") ||
                    !STRCASECMP(arg_ptr, "--no_colour") ||
                    !STRCASECMP(arg_ptr, "-no-colors") ||
                    !STRCASECMP(arg_ptr, "-no-colours") ||
                    !STRCASECMP(arg_ptr, "-no_colors") ||
                    !STRCASECMP(arg_ptr, "-no_colours") ||
                    !STRCASECMP(arg_ptr, "-no-color") ||
                    !STRCASECMP(arg_ptr, "-no-colour") ||
                    !STRCASECMP(arg_ptr, "-no_color") ||
                    !STRCASECMP(arg_ptr, "-no_colour") ||
                    !STRCASECMP(arg_ptr, "--nocolors") ||
                    !STRCASECMP(arg_ptr, "--nocolours") ||
                    !STRCASECMP(arg_ptr, "--nocolor") ||
                    !STRCASECMP(arg_ptr, "--nocolour") ||
                    !STRCASECMP(arg_ptr, "-nocolors") ||
                    !STRCASECMP(arg_ptr, "-nocolours") ||
                    !STRCASECMP(arg_ptr, "-nocolor") ||
                    !STRCASECMP(arg_ptr, "-nocolour")
            )
            {
                colors = 0;
            }
	    /* Ignore environment variables. */
	    else if(!STRCASECMP(arg_ptr, "--ignore_environments") ||
                    !STRCASECMP(arg_ptr, "-ignore_environments") ||
                    !STRCASECMP(arg_ptr, "--ignore-environments") ||
                    !STRCASECMP(arg_ptr, "-ignore-environments") ||
                    !STRCASECMP(arg_ptr, "--ignoreenvironments") ||
                    !STRCASECMP(arg_ptr, "-ignoreenvironments")
            )
            {
		ignore_environments = 1;
	    }
        }


	/* Make sure we got required arguments. */
	status = PConfPrereqCheck(
	    platforms_ini,
	    argc, argv
	);
	switch(status)
	{
	  case -1:
	    fprintf(
		stderr,
 "%s: Cannot open platforms configuration file.\n",
		(argc > 1) ? argv[1] : platforms_ini
	    );
	    fprintf(
		stderr,
"This file is used to configure generation of the output Makefile\n\
used in compiling the program source, it should be specified as the\n\
first argument. If you cannot locate the platforms configuration file,\n\
notify the vendor that this file is missing.\n"
	    );
	    DO_FREE_LOCALS
	    return(1);
	    break;

	  case -2:
	    /* Print help. */
	    PConfPrintHelp((argc > 0) ? argv[0] : NULL);
	    DO_FREE_LOCALS
	    return(4);
	    break;

	  case -3:
	    /* Need to specify a platform. */
	    printf("You must specify a supported platform to configure for.\n");
	    /* Set print platforms level to 0, this will cause it to print
	     * a brief list of platforms farther below.
	     */
	    print_platforms_level = 0;
	    break;

	  default:
	    break;
	}


	/* Begin parsing arguments. */

	/* Get platforms.ini if specified. */
	if(argc > 1)
	{
	    FREE(platforms_ini);
	    platforms_ini = STRDUP(argv[1]);
	}
	/* Get platform name. */
	if(argc > 2)
	{
	    FREE(platform_name);
            platform_name = STRDUP(argv[2]);
        }

	/* Parse the rest of the arguments. */
	for(i = 2; i < argc; i++)
	{
	    arg_ptr = (const char *)argv[i];
	    if(arg_ptr == NULL)
		continue;

	    /* List all platforms with extended details. */
	    if(STRCASEPFX(arg_ptr, "--list_a") ||
               STRCASEPFX(arg_ptr, "-list_a") ||
               STRCASEPFX(arg_ptr, "--lista") ||
               STRCASEPFX(arg_ptr, "-lista")
	    )
	    {
		const char *cstrptr;

		print_platforms_level = 3;

                /* List specific platform? */
                cstrptr = STRCHR(arg_ptr, '=');
                if(cstrptr != NULL)
                {
                    cstrptr++;
                    while(ISBLANK(*cstrptr))
                        cstrptr++;

                    FREE(specific_platform);
                    specific_platform = STRDUP(cstrptr);
                }
	    }
            /* List platforms. */
            else if(STRCASEPFX(arg_ptr, "--list") ||
                    STRCASEPFX(arg_ptr, "-list")
            )
	    {
		const char *cstrptr;

                print_platforms_level = 1;

		/* List specific platform? */
		cstrptr = STRCHR(arg_ptr, '=');
		if(cstrptr != NULL)
		{
		    cstrptr++;
		    while(ISBLANK(*cstrptr))
			cstrptr++;

		    FREE(specific_platform);
		    specific_platform = STRDUP(cstrptr);
		}
	    }
	    /* If parameter is not parsed yet and its the third one
	     * then take it as the platform name.
	     */
	    else if(i == 2)
	    {
		FREE(specific_platform);
		specific_platform = STRDUP(arg_ptr);
	    }
	}


	/* Allocate program core structure. */
	core_ptr = (pconf_core_struct *)calloc(
	    1, sizeof(pconf_core_struct)
	);
	if(core_ptr == NULL)
	{
	    /* Free allocated resources. */
	    DO_FREE_LOCALS
	    return(3);
	}
	else
	{
	    const char *cstrptr;


	    cstrptr = (const char *)GETENV("COLUMNS");
	    if(cstrptr == NULL)
		cstrptr = (const char *)GETENV("SCREEN_WIDTH");
            if(cstrptr == NULL)
                cstrptr = (const char *)GETENV("SCREENWIDTH");
	    if(cstrptr != NULL)
		core_ptr->screen_width = atoi(cstrptr);

	    core_ptr->verbose = verbose;
	    core_ptr->warnings = warnings;
	    core_ptr->colors = colors;
	    core_ptr->interactive = interactive;
	    core_ptr->force = force;
	    core_ptr->ignore_environments = ignore_environments;
	}

	/* Load platforms configuration. */
	status = PConfLoadPlatformsFromFile(core_ptr, platforms_ini);
	if(status)
	{
	    /* Free allocated resources. */
	    PConfFreeCore(core_ptr);
	    core_ptr = NULL;
            DO_FREE_LOCALS
	    return(1);
	}
	/* We only want to print the platforms and exit? */
	if(print_platforms_level >= 0)
	{
	    PConfPrintPlatforms(
		core_ptr,
		core_ptr->platform, core_ptr->total_platforms,
		specific_platform,
		print_platforms_level,
		0
	    );
	    if(print_platforms_level == 0)
	    {
		printf(
"\n\
For more details and features available on each platform, specify the\n\
--listall argument. To configure for a particular platform, just specify\n\
that platform's name.\n"
		);
	    }

            /* Free allocated resources. */
            PConfFreeCore(core_ptr);
            core_ptr = NULL;
            DO_FREE_LOCALS
            return(4);
	}

	/* Check if selected platform is defined. */
	selected_platform = PConfMatchPlatformByName(
	    core_ptr->platform, core_ptr->total_platforms,
	    specific_platform, &selected_platform_num
	);
	if(selected_platform == NULL)
	{
	    /* Specified platform does not exist. */
	    fprintf(
		stderr,
		"%s: ",
		specific_platform
	    );
	    PConfSetColor(core_ptr, stderr, PCONF_COLOR_FAILURE);
	    fprintf(
                stderr,
                "No such platform.\n"
            );
	    PConfSetColor(core_ptr, stderr, PCONF_COLOR_DEFAULT);
	    if(core_ptr->message_platform_unsupported == NULL)
	    {
	        fprintf(
		    stderr,
		    "Use argument --listall to list available platforms.\n"
		);
	    }
	    else
	    {
		fprintf(   
                    stderr,
                    "%s\n",
		    core_ptr->message_platform_unsupported
                );
	    }

            /* Free allocated resources. */
            PConfFreeCore(core_ptr);
            core_ptr = NULL;
	    DO_FREE_LOCALS
	    return(2);
	}

	/* Move selected platform from the list into the core
	 * structure's selected_platform.
	 */
	core_ptr->selected_platform = selected_platform;

	/* Remove platform pointer from the core's list. */
	if((selected_platform_num >= 0) &&
           (selected_platform_num < core_ptr->total_platforms)
	)
	    core_ptr->platform[selected_platform_num] = NULL;

	/* Deallocate all loaded platforms on the core structure
	 * except for the selected one since we marked it NULL on
	 * the list and it won't get deleted.
	 */
	PConfFreePlatforms(
	    &core_ptr->platform, &core_ptr->total_platforms
	);


	/* Got selected platform `selected_platform' at this point. */

	/* Print configure startup message. */
	if(selected_platform->name != NULL)
	    printf("Configuring for platform `%s'...\n",
		selected_platform->name
	    );
	if(core_ptr->message_configure_startup != NULL)
	    printf("%s\n",
                core_ptr->message_configure_startup
            );


	/* Apply given command line and environment arguments and
	 * apply them to the selected platform.
	 */
	PConfApplyArgsToPlatform(
	    core_ptr, selected_platform,
	    argc, argv
	);


	/* Check if all features and dependencies in the platform
	 * exist.
	 */
	status = PConfCheckDepends(
	    core_ptr, selected_platform
	);
	/* How many failed dependancies did we get? */
	if(status > 0)
	{
	    if(core_ptr->warnings)
	    {
		if(core_ptr->force)
		{
                  fprintf(stderr, "*** ");
                  PConfSetColor(core_ptr, stderr, PCONF_COLOR_FAILURE);
                  fprintf(
                    stderr,
                    "%i failed %s ignored (because --force used)!",
                    status, (status > 1) ? "dependencies" : "dependency"
                  );
                  PConfSetColor(core_ptr, stderr, PCONF_COLOR_DEFAULT);
                  fprintf(stderr, " ***\n");
		}
		else
		{
		  fprintf(stderr, "*** ");
		  PConfSetColor(core_ptr, stderr, PCONF_COLOR_FAILURE);
		  fprintf(
		    stderr,
                    "%i failed %s!",
		    status, (status > 1) ? "dependencies" : "dependency"
		  );
		  PConfSetColor(core_ptr, stderr, PCONF_COLOR_DEFAULT);
		  fprintf(stderr, " ***\n");
		}
	    }

	    /* If not forcing, then we need to exit now. */
	    if(!core_ptr->force)
	    {
		if(core_ptr->message_depend_failed != NULL)
		{
		    fprintf(
			stderr,
			"%s\n",
			core_ptr->message_depend_failed
		    );
		}

		/* Free allocated resources. */
		PConfFreeCore(core_ptr);
		core_ptr = NULL;
		DO_FREE_LOCALS
		return(1);
	    }
	}
	else
	{
	    if(core_ptr->verbose)
	    {
/*
		printf("*** ");
		PConfSetColor(core_ptr, stdout, PCONF_COLOR_SUCCESS);
		printf("All dependencies found!");
		PConfSetColor(core_ptr, stdout, PCONF_COLOR_DEFAULT);
		printf("***\n");
 */
	    }
	}

	/* Print review of compiler configuration. */
	if(core_ptr->verbose)
	{
	    PConfPrintHR(core_ptr);
	    PConfSetColor(core_ptr, stdout, PCONF_COLOR_HEADING);
	    printf("Configuration Review:\n\n");
	    PConfSetColor(core_ptr, stdout, PCONF_COLOR_DEFAULT);
	    PConfPrintPlatform(core_ptr, selected_platform, 3, 1);
	    printf("\n");
	    PConfPrintHR(core_ptr);

	    if(core_ptr->interactive)
	    {

	    }
	}

	/* Generate output Makefile. */
	PConfGenerateOutputMakefile(
	    core_ptr, selected_platform
	);

	/* Print configuration success message. */
        PConfSetColor(core_ptr, stdout, PCONF_COLOR_HEADING);
	printf("\nPlatform configuration completed!\n");
	PConfSetColor(core_ptr, stdout, PCONF_COLOR_DEFAULT);
	/* Print success message? */
        if(core_ptr->message_success != NULL)
	{
	    PConfPrintHR(core_ptr);
            printf("%s\n",
                core_ptr->message_success
            );
	}



	/* Begin shutdown. */

	/* Deallocate core structure. */
	PConfFreeCore(core_ptr);
	core_ptr = NULL;

	/* Free allocated resources. */
	DO_FREE_LOCALS

#undef DO_FREE_LOCALS

	return(0);
}
