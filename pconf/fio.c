#include "pconf.h"


static void PConfLoadPlatformsSanitizeLine(char *s);
void PConfFreePlatform(platform_struct *platform_ptr);
void PConfFreePlatforms(
        platform_struct ***list, int *total          
);
int PConfLoadPlatformsFromFile(
	pconf_core_struct *core_ptr, const char *filename
);
int PConfLoadThisPlatformFromFile(
        pconf_core_struct *core_ptr, const char *filename
);

void PConfGenerateOutputMakefile(
        pconf_core_struct *core_ptr, platform_struct *platform_ptr
);


/*
 *	Strips string s of all '\n' characters and spaces.
 */
static void PConfLoadPlatformsSanitizeLine(char *s)
{
	char *sp;


	if(s == NULL)
	    return;

	/* Strip spaces. */
	STRSTRIP(s);

	/* Strip new lines. */
	sp = s;
	/* Itterate through string. */
	while((*sp) != '\0')
	{
	    /* Is current character a newline? */
	    if(ISCR(*sp))
	    {
		char *sp2 = sp;

		/* Shorten string by one character at current
		 * position.
		 */
		while((*sp2) != '\0')
		{
		    (*sp2) = *(sp2 + 1);
		    sp2++;
		}
		/* Do not itterate to next (since we shortend). */
	    }
	    else
	    {
		sp++;
	    }
	}

	return;
}


/*
 *	Deallocates the given platform structure and its allocated
 *      substructures.
 */
void PConfFreePlatform(platform_struct *platform_ptr)
{
	int n, j;
	feature_struct *feature_ptr;
	depend_struct *depend_ptr;


	if(platform_ptr == NULL)
	    return;

	/* Deallocate all features. */
	for(n = 0; n < platform_ptr->total_features; n++)
	{
		feature_ptr = platform_ptr->feature[n];
		if(feature_ptr == NULL)
		    continue;

		/* Deallocate all library dependancy structures. */
		for(j = 0; j < feature_ptr->total_depends; j++)
		{
		    depend_ptr = feature_ptr->depend[j];
		    if(depend_ptr == NULL)
			continue;

		    FREE(depend_ptr->name);
		    FREE(depend_ptr->description);
		    STRFREEARRAY(
			depend_ptr->path,
			depend_ptr->total_paths
		    );
                    STRFREEARRAY(
                        depend_ptr->grep_string,
                        depend_ptr->total_grep_strings
                    );

                    FREE(depend_ptr->os_name);
                    FREE(depend_ptr->machine_name);

		    FREE(depend_ptr);
		}
		FREE(feature_ptr->depend);
		feature_ptr->depend = NULL;
		feature_ptr->total_depends = 0;


		FREE(feature_ptr->name);
		FREE(feature_ptr->description);
		FREE(feature_ptr->url_homepage);
		FREE(feature_ptr->url_download);
                FREE(feature_ptr->cflags);
                FREE(feature_ptr->inc_dirs);
                FREE(feature_ptr->libs);
                FREE(feature_ptr->lib_dirs);

                FREE(feature_ptr);
	}
	FREE(platform_ptr->feature);
	platform_ptr->feature = NULL;
	platform_ptr->total_features = 0;

	FREE(platform_ptr->name);
	FREE(platform_ptr->description);

	STRFREEARRAY(
            platform_ptr->path_header,
            platform_ptr->total_path_headers
        );
        STRFREEARRAY(
            platform_ptr->path_library,
            platform_ptr->total_path_libraries
        );
        STRFREEARRAY(
            platform_ptr->path_config,
            platform_ptr->total_path_configs
        );
        STRFREEARRAY(
            platform_ptr->path_program,
            platform_ptr->total_path_programs
        );
        STRFREEARRAY(
            platform_ptr->path_data,
            platform_ptr->total_path_datas
        );

	FREE(platform_ptr->prefix);
        FREE(platform_ptr->cflags);
        FREE(platform_ptr->inc_dirs);
        FREE(platform_ptr->libs);
        FREE(platform_ptr->lib_dirs);
        FREE(platform_ptr->cc);
        FREE(platform_ptr->cpp);

	FREE(platform_ptr);

	return;
}

/*
 *      Deallocates the list of platform structures and their allocated
 *      substructures.
 */
void PConfFreePlatforms( 
        platform_struct ***list, int *total
)
{
        int i;


        if((list == NULL) || (total == NULL))
            return;

        /* Deallocate all platforms. */
        for(i = 0; i < (*total); i++)
        {
	    PConfFreePlatform((*list)[i]);
	    (*list)[i] = NULL;
        }

        if((*list) != NULL)
        {
            FREE(*list);
            (*list) = NULL;
        }
        (*total) = 0;

        return;
}


/*
 *	Loads data from platform configuration file specified by filename
 *	to the given core structure.
 *
 *	Returns non-zero on error.
 */
int PConfLoadPlatformsFromFile(
	pconf_core_struct *core_ptr, const char *filename
)
{
	int platform_num, feature_num, depend_num;
        FILE *fp;
        char *buf;
	const char *parm;
	platform_struct *platform_ptr;
	feature_struct *feature_ptr;
	depend_struct *depend_ptr;

#define DO_RESET_CONTEXT_PTRS	\
{ \
 platform_num = -1; \
 platform_ptr = NULL; \
 feature_num = -1; \
 feature_ptr = NULL; \
 depend_num = -1; \
 depend_ptr = NULL; \
}

	if((core_ptr == NULL) || (filename == NULL))
	    return(-1);

/* Delete any existing values on core structure? */

	/* Open platforms configuratino file. */
        fp = FOPEN(filename, "rb");
        if(fp == NULL)
        {
            fprintf(stderr, "%s: Cannot open.\n", filename);
            return(-1);
        }

	/* Begin reading playforms configuration file. */
	buf = NULL;
	DO_RESET_CONTEXT_PTRS
        do
        {
            buf = FSEEKNEXTPARAMETER(
                fp, buf, PCONF_CFG_COMMENT_CHAR, PCONF_CFG_DELIM_CHAR
            );
            if(buf == NULL)
                break;

	    /* Begin handling parameter. */
	    parm = (const char *)buf;

	    /* Version. */
            if(!STRCASECMP(parm, "Version"))
            {
		FSEEKNEXTLINE(fp);
            }
	    /* Author. */
            else if(!STRCASECMP(parm, "Author"))
            {
                FSEEKNEXTLINE(fp);
            }
	    /* Program name. */
            else if(!STRCASECMP(parm, "ProgramName"))
            {
                FSEEKNEXTLINE(fp);
            }

	    /* Add new platform entry? */
	    else if(!STRCASECMP(parm, "Platform"))
            {
                char *val = FGETLINE(fp);

		/* If a platform was already in context, then clean it up
		 * before loading a new one.
		 */
		if(platform_ptr != NULL)
		{
/* Add other platform cleanup stuff here. */
		}


		PConfLoadPlatformsSanitizeLine(val);
		DO_RESET_CONTEXT_PTRS	/* Reset context pointers. */

		/* Allocate a new platform structure. */
		platform_num = core_ptr->total_platforms;
		core_ptr->total_platforms = platform_num + 1;
		core_ptr->platform = (platform_struct **)realloc(
		    core_ptr->platform,
		    core_ptr->total_platforms * sizeof(platform_struct *)
		);
		if(core_ptr->platform == NULL)
		{
		    core_ptr->total_platforms = 0;
		    platform_num = -1;
		}
		else
		{
		    core_ptr->platform[platform_num] = platform_ptr =
			(platform_struct *)calloc(
			    1, sizeof(platform_struct)
		        );
		}

		/* Set platform name. */
		if(platform_ptr != NULL)
		    platform_ptr->name = val;
		else
		    FREE(val);
	    }
            /* Add new feature entry to current platform? */
            else if(!STRCASECMP(parm, "PlatformFeature"))
            {
                char *val = FGETLINE(fp);

                PConfLoadPlatformsSanitizeLine(val);

                depend_num = -1;
                depend_ptr = NULL;

		/* Is a platform currently in context? */
		if(platform_ptr == NULL)
		{
                    fprintf(
                        stderr,
 "%s: Cannot specify parameter %s without a Platform in context.\n",
                        filename, parm
                    );
		}
		else
		{
                    /* Allocate a new feature structure. */
                    feature_num = platform_ptr->total_features;
		    platform_ptr->total_features = feature_num + 1;
                    platform_ptr->feature = (feature_struct **)realloc(
                        platform_ptr->feature,
                        platform_ptr->total_features * sizeof(feature_struct *)
                    );
                    if(platform_ptr->feature == NULL)
                    {
                        platform_ptr->total_features = 0;
                        feature_num = -1;
			feature_ptr = NULL;
		    }
                    else
                    {
                        platform_ptr->feature[feature_num] = feature_ptr =
                            (feature_struct *)calloc(
                                1, sizeof(feature_struct)
                            );
                    }

                    /* Set feature name. */
                    if(feature_ptr != NULL)
		    {
			feature_ptr->name = STRDUP(val);
		    }
		}

		FREE(val);
	    }
            /* Add new library dependancy entry to current feature? */
            else if(!STRCASECMP(parm, "FeatureDependency") ||
                    !STRCASECMP(parm, "FeatureDependent") ||
                    !STRCASECMP(parm, "FeatureDepend")
	    )
            {
                char *val = FGETLINE(fp);

                PConfLoadPlatformsSanitizeLine(val);

                /* Is a feature currently in context? */
                if(feature_ptr == NULL)
                {
                    fprintf(
                        stderr,
 "%s: Cannot specify parameter %s without a PlatformFeature in context.\n",
                        filename, parm
                    );
                }
                else
                {
                    /* Allocate a new depend structure. */
                    depend_num = feature_ptr->total_depends;
                    feature_ptr->total_depends = depend_num + 1;
                    feature_ptr->depend = (depend_struct **)realloc(
                        feature_ptr->depend,
                        feature_ptr->total_depends * sizeof(depend_struct *)
                    );
                    if(feature_ptr->depend == NULL)
                    {
                        feature_ptr->total_depends = 0;
                        depend_num = -1;
                        depend_ptr = NULL;
                    }
                    else
                    {
                        feature_ptr->depend[depend_num] = depend_ptr =
                            (depend_struct *)calloc(
                                1, sizeof(depend_struct)
                            );
                    }

                    /* Set depend name. */
                    if(depend_ptr != NULL)
                    {
                        depend_ptr->name = STRDUP(val);
                    }
                }

                FREE(val);
            }

	    /* Description (for whatever is in context). */
            else if(!STRCASECMP(parm, "Description"))
            {
                char *val = FGETLINE(fp);

                PConfLoadPlatformsSanitizeLine(val);

                /* Handle by context. */
		if(depend_ptr != NULL)
		{
		    FREE(depend_ptr->description);
		    depend_ptr->description = STRDUP(val);
		}
		else if(feature_ptr != NULL)
		{
                    FREE(feature_ptr->description);
                    feature_ptr->description = STRDUP(val);
                }
                else if(platform_ptr != NULL)
                {
                    FREE(platform_ptr->description);
                    platform_ptr->description = STRDUP(val);
                }
		else if(core_ptr->warnings)
		{
                    fprintf(   
                        stderr,
 "%s: Nothing in context for parameter `%s'.\n",
                        filename, parm
                    );
		}
		FREE(val);
	    }
            /* URL of home page (for whatever is in context). */
            else if(!STRCASECMP(parm, "FeatureURLHomePage") ||
                    !STRCASECMP(parm, "URLHomePage")
	    )
            {
                char *val = FGETLINE(fp);

                PConfLoadPlatformsSanitizeLine(val);

                /* Handle by context. */
                if(feature_ptr != NULL)
                {
                    FREE(feature_ptr->url_homepage);
                    feature_ptr->url_homepage = STRDUP(val);
                }
                else if(core_ptr->warnings)
                {
                    fprintf(
                        stderr,
 "%s: Nothing in context for parameter `%s'.\n",
                        filename, parm
                    );
                }
                FREE(val);
            }
            /* URL of download (for whatever is in context). */
            else if(!STRCASECMP(parm, "FeatureURLDownload") ||
                    !STRCASECMP(parm, "URLDownload")
            )
            {
                char *val = FGETLINE(fp);

                PConfLoadPlatformsSanitizeLine(val);

                /* Handle by context. */
                if(feature_ptr != NULL)
                {
                    FREE(feature_ptr->url_download);
                    feature_ptr->url_download = STRDUP(val);
                }
                else if(core_ptr->warnings)
                {
                    fprintf(   
                        stderr,
 "%s: Nothing in context for parameter `%s'.\n",
                        filename, parm
                    );
                }
                FREE(val);
            }

            /* Must exist (for whatever is in context). */
            else if(!STRCASECMP(parm, "MustExist"))
            {
                char *val = FGETLINE(fp);
		int must_exist;

                PConfLoadPlatformsSanitizeLine(val);

		/* Parse must exist string. */
		if((toupper(*val) == 'Y') ||
                   (toupper(*val) == 'T')
		)
		    must_exist = PCONF_MUST_EXIST_YES;
		else if((toupper(*val) == 'P'))
		    must_exist = PCONF_MUST_EXIST_PREFERRED;
		else
		    must_exist = PCONF_MUST_EXIST_NO;

                /* Handle by context. */
                if(depend_ptr != NULL)
                {
                    depend_ptr->must_exist = must_exist;
                }
                else if(feature_ptr != NULL)
                {
                    feature_ptr->must_exist = must_exist;
                }
                else if(core_ptr->warnings)
                {
                    fprintf(
                        stderr,
 "%s: Nothing in context for parameter `%s'.\n",
                        filename, parm
                    );
                }
                FREE(val);
            }

            /* Dependent entity type. */
            else if(!STRCASECMP(parm, "DependentType") ||
                    !STRCASECMP(parm, "DependType")
            )
            {
                char *val = FGETLINE(fp);

                PConfLoadPlatformsSanitizeLine(val);

                /* Handle by context. */
                if(depend_ptr != NULL)
                {
                    if(!STRCASECMP(val, "program"))
                        depend_ptr->type = PCONF_DEP_TYPE_PROGRAM;
                    else if(!STRCASECMP(val, "header"))
                        depend_ptr->type = PCONF_DEP_TYPE_HEADER;
                    else if(!STRCASECMP(val, "library"))
                        depend_ptr->type = PCONF_DEP_TYPE_LIBRARY;
                    else if(!STRCASECMP(val, "config"))
			depend_ptr->type = PCONF_DEP_TYPE_CONFIG;
                    else if(!STRCASECMP(val, "data"))
                        depend_ptr->type = PCONF_DEP_TYPE_DATA;
                    else if(!STRCASECMP(val, "os"))
                        depend_ptr->type = PCONF_DEP_TYPE_OS;
                    else if(!STRCASECMP(val, "machine"))
                        depend_ptr->type = PCONF_DEP_TYPE_MACHINE;
                    else
                        depend_ptr->type = PCONF_DEP_TYPE_OTHER;        
                }
                FREE(val);
            }
	    /* Dependent entity path. */
	    else if(!STRCASECMP(parm, "DependentPath") ||
                    !STRCASECMP(parm, "DependPath")
	    )
            {
                char *val = FGETLINE(fp);

                PConfLoadPlatformsSanitizeLine(val);

                /* Handle by context. */
                if(depend_ptr != NULL)
                {
		    STRLISTAPPEND(
			&depend_ptr->path,
			&depend_ptr->total_paths,
			val
		    );
                }
                FREE(val);
	    }
            /* Dependent entity grep string. */
            else if(!STRCASECMP(parm, "DependentGrepString") ||
                    !STRCASECMP(parm, "DependGrepString")
	    )
            {
                char *val = FGETLINE(fp);

                PConfLoadPlatformsSanitizeLine(val);

                /* Handle by context. */
                if(depend_ptr != NULL)
                {
                    STRLISTAPPEND(
                        &depend_ptr->grep_string,
                        &depend_ptr->total_grep_strings,
                        val
                    );
                }
                FREE(val);
            }
            /* Dependent entity OS name. */
            else if(!STRCASECMP(parm, "DependentOS") ||
                    !STRCASECMP(parm, "DependOS")
            )
            {
                char *val = FGETLINE(fp);

                PConfLoadPlatformsSanitizeLine(val);

                /* Handle by context. */
                if(depend_ptr != NULL)
                {
                    FREE(depend_ptr->os_name);
                    depend_ptr->os_name = STRDUP(val);

                    if(depend_ptr->type != PCONF_DEP_TYPE_OS)
                        fprintf(
                            stderr,
"%s: Warning: Value defined but type of dependency is not a OS.\n",
                            parm
                        );
                }
                FREE(val);
            }
            /* Dependent entity machine name. */
            else if(!STRCASECMP(parm, "DependentMachine") ||
                    !STRCASECMP(parm, "DependMachine")
            )
            {
                char *val = FGETLINE(fp);

                PConfLoadPlatformsSanitizeLine(val);

                /* Handle by context. */
                if(depend_ptr != NULL)
                {
                    FREE(depend_ptr->machine_name);
                    depend_ptr->machine_name = STRDUP(val);

		    if(depend_ptr->type != PCONF_DEP_TYPE_MACHINE)
			fprintf(
			    stderr,
"%s: Warning: Value defined but type of dependency is not a machine.\n",
			    parm
			);
                }
                FREE(val);
            }

	    /* Feature cflags. */
	    else if(!STRCASECMP(parm, "FeatureCFLAGS") ||
                    !STRCASECMP(parm, "FeatureCFLAG")
	    )
            {
                char *val = FGETLINE(fp);

                PConfLoadPlatformsSanitizeLine(val);

                /* Handle by context. */
                if(feature_ptr != NULL)
                {
		    FREE(feature_ptr->cflags);
		    feature_ptr->cflags = STRDUP(val);
                }
                FREE(val);
            }
            /* Feature include directories. */
            else if(!STRCASECMP(parm, "FeatureINC_DIRS") ||
                    !STRCASECMP(parm, "FeatureINCDIRS") ||
                    !STRCASECMP(parm, "FeatureINC_DIR") ||  
                    !STRCASECMP(parm, "FeatureINCDIR")
            )
            {
                char *val = FGETLINE(fp);

                PConfLoadPlatformsSanitizeLine(val);

                /* Handle by context. */
                if(feature_ptr != NULL)
                {
                    FREE(feature_ptr->inc_dirs);
                    feature_ptr->inc_dirs = STRDUP(val);
                }
                FREE(val);
            }
            /* Feature libraries. */
            else if(!STRCASECMP(parm, "FeatureLIBS") ||
                    !STRCASECMP(parm, "FeatureLIB")
	    )
            {
                char *val = FGETLINE(fp);

                PConfLoadPlatformsSanitizeLine(val);

                /* Handle by context. */
                if(feature_ptr != NULL)
                {
                    FREE(feature_ptr->libs);
                    feature_ptr->libs = STRDUP(val);
                }
                FREE(val);
            }
            /* Feature library directories. */
            else if(!STRCASECMP(parm, "FeatureLIB_DIRS") ||
                    !STRCASECMP(parm, "FeatureLIBDIRS") ||
                    !STRCASECMP(parm, "FeatureLIB_DIR") ||
                    !STRCASECMP(parm, "FeatureLIBDIR")
	    )
            {
                char *val = FGETLINE(fp);

                PConfLoadPlatformsSanitizeLine(val);

                /* Handle by context. */
                if(feature_ptr != NULL)
                {
                    FREE(feature_ptr->lib_dirs); 
                    feature_ptr->lib_dirs = STRDUP(val);
                }
                FREE(val);
            }



	    /* Platform search path for header files. */
	    else if(!STRCASECMP(parm, "PlatformSearchPathHeader") ||
                    !STRCASECMP(parm, "PlatformSearchPathInclude") ||
                    !STRCASECMP(parm, "PlatformSearchPathInc")
	    )
            {
                char *val = FGETLINE(fp);

                PConfLoadPlatformsSanitizeLine(val);

                /* Handle by context. */
                if(platform_ptr != NULL) 
                {
                    STRLISTAPPEND(
                        &platform_ptr->path_header,
                        &platform_ptr->total_path_headers,
                        val
                    );
                }
                FREE(val);
            }
            /* Platform search path for library files. */
            else if(!STRCASECMP(parm, "PlatformSearchPathLibrary") ||
                    !STRCASECMP(parm, "PlatformSearchPathLib")
	    )
            {   
                char *val = FGETLINE(fp);

                PConfLoadPlatformsSanitizeLine(val);
                
                /* Handle by context. */
                if(platform_ptr != NULL)
                {
                    STRLISTAPPEND(
                        &platform_ptr->path_library,
                        &platform_ptr->total_path_libraries,
                        val
                    );
                }  
                FREE(val);
            }
            /* Platform search path for configuration files. */
            else if(!STRCASECMP(parm, "PlatformSearchPathConfiguration") ||
                    !STRCASECMP(parm, "PlatformSearchPathConfig") ||
                    !STRCASECMP(parm, "PlatformSearchPathEtc")
            )
            {
                char *val = FGETLINE(fp);

                PConfLoadPlatformsSanitizeLine(val);

                /* Handle by context. */
                if(platform_ptr != NULL)
                {
                    STRLISTAPPEND(
                        &platform_ptr->path_config,
                        &platform_ptr->total_path_configs,
                        val
                    );
                }
                FREE(val);
            }
            /* Platform search path for program files. */
            else if(!STRCASECMP(parm, "PlatformSearchPathProgram") ||
                    !STRCASECMP(parm, "PlatformSearchPathProg") ||
                    !STRCASECMP(parm, "PlatformSearchPathBin")
            )
            {
                char *val = FGETLINE(fp);

                PConfLoadPlatformsSanitizeLine(val);

                /* Handle by context. */
                if(platform_ptr != NULL)
                {
                    STRLISTAPPEND(
                        &platform_ptr->path_program,
                        &platform_ptr->total_path_programs,
                        val
                    );
                }
                FREE(val);
            }
            /* Platform search path for data or other files. */
            else if(!STRCASECMP(parm, "PlatformSearchPathData") ||
                    !STRCASECMP(parm, "PlatformSearchPathOther")
            )
            {
                char *val = FGETLINE(fp);

                PConfLoadPlatformsSanitizeLine(val);

                /* Handle by context. */
                if(platform_ptr != NULL)
                {
                    STRLISTAPPEND(
                        &platform_ptr->path_data,
                        &platform_ptr->total_path_datas,
                        val
                    );
                }
                FREE(val);
            }

	    /* Platform PREFIX. */
            else if(!STRCASECMP(parm, "PlatformPREFIX") ||
                    !STRCASECMP(parm, "PREFIX")
	    )
            {   
                char *val = FGETLINE(fp);

                PConfLoadPlatformsSanitizeLine(val);

                /* Handle by context. */
                if(platform_ptr != NULL)
                {
                    FREE(platform_ptr->prefix);
		    platform_ptr->prefix = STRDUP(val);
                }
                FREE(val);
            }
            /* Platform CFLAGS. */
            else if(!STRCASECMP(parm, "PlatformCFLAGS") || 
                    !STRCASECMP(parm, "CFLAGS")
            )
            {
                char *val = FGETLINE(fp);

                PConfLoadPlatformsSanitizeLine(val);

                /* Handle by context. */
                if(platform_ptr != NULL)
                {
                    FREE(platform_ptr->cflags);
                    platform_ptr->cflags = STRDUP(val);
                }
                FREE(val);
            }
            /* Platform INC_DIRS. */
            else if(!STRCASECMP(parm, "PlatformINC_DIRS") ||
                    !STRCASECMP(parm, "INC_DIRS") ||
                    !STRCASECMP(parm, "PlatformINC_DIR") ||
                    !STRCASECMP(parm, "INC_DIR")
            )
            {
                char *val = FGETLINE(fp);

                PConfLoadPlatformsSanitizeLine(val);

                /* Handle by context. */
                if(platform_ptr != NULL)
                {
                    FREE(platform_ptr->inc_dirs);
                    platform_ptr->inc_dirs = STRDUP(val);
                }
                FREE(val);
            }
            /* Platform LIBS. */
            else if(!STRCASECMP(parm, "PlatformLIBS") ||
                    !STRCASECMP(parm, "LIBS")
            )
            {       
                char *val = FGETLINE(fp);

                PConfLoadPlatformsSanitizeLine(val);

                /* Handle by context. */
                if(platform_ptr != NULL)
                {
                    FREE(platform_ptr->libs);
                    platform_ptr->libs = STRDUP(val);
                }
                FREE(val);
            }
            /* Platform LIB_DIRS. */
            else if(!STRCASECMP(parm, "PlatformLIB_DIRS") ||
                    !STRCASECMP(parm, "LIB_DIRS") ||
                    !STRCASECMP(parm, "PlatformLIB_DIR") ||
                    !STRCASECMP(parm, "LIB_DIR")
            )
            {
                char *val = FGETLINE(fp);
        
                PConfLoadPlatformsSanitizeLine(val);

                /* Handle by context. */
                if(platform_ptr != NULL)
                {
                    FREE(platform_ptr->lib_dirs);
                    platform_ptr->lib_dirs = STRDUP(val);
                }
                FREE(val);
            }
            /* Platform C compiler. */
            else if(!STRCASECMP(parm, "PlatformCC") ||
                    !STRCASECMP(parm, "CC")
            )
            {
                char *val = FGETLINE(fp);

                PConfLoadPlatformsSanitizeLine(val);

                /* Handle by context. */
                if(platform_ptr != NULL)
                {
                    FREE(platform_ptr->cc);
                    platform_ptr->cc = STRDUP(val);
                }
                FREE(val);
            }
            /* Platform C++ compiler. */
            else if(!STRCASECMP(parm, "PlatformCPP") ||
                    !STRCASECMP(parm, "CPP")
            )
            {
                char *val = FGETLINE(fp);

                PConfLoadPlatformsSanitizeLine(val);

                /* Handle by context. */
                if(platform_ptr != NULL)
                {
                    FREE(platform_ptr->cpp);
                    platform_ptr->cpp = STRDUP(val);
                }
                FREE(val);
            }

            /* Output Makefile. */
            else if(!STRCASECMP(parm, "MakefileOutput") ||
                    !STRCASECMP(parm, "MakefileOut")
            )
            {
                char *val = FGETLINE(fp);

                PConfLoadPlatformsSanitizeLine(val);

		FREE(core_ptr->makefile_output);
		core_ptr->makefile_output = STRDUP(val);

                FREE(val);
            }
            /* Input Makefile prepended. */
            else if(!STRCASECMP(parm, "MakefileInputPrepend") ||
                    !STRCASECMP(parm, "MakefileInputPre") ||
		    !STRCASECMP(parm, "MakefileInPrepend") ||
                    !STRCASECMP(parm, "MakefileInPre")
            )
            {
                char *val = FGETLINE(fp);

                PConfLoadPlatformsSanitizeLine(val);

                FREE(core_ptr->makefile_input_prepend);
                core_ptr->makefile_input_prepend = STRDUP(val);

                FREE(val);
            }
            /* Input Makefile append. */
            else if(!STRCASECMP(parm, "MakefileInputAppend") ||
                    !STRCASECMP(parm, "MakefileInAppend")
            )
            {
                char *val = FGETLINE(fp);

                PConfLoadPlatformsSanitizeLine(val);

                FREE(core_ptr->makefile_input_append);
                core_ptr->makefile_input_append = STRDUP(val);

                FREE(val);
            }
	    /* This platform information file. */
            else if(!STRCASECMP(parm, "ThisPlatformInfo"))
            {
                char *val = FGETLINE(fp);

                PConfLoadPlatformsSanitizeLine(val);

		if(PConfLoadThisPlatformFromFile(core_ptr, val))
		{
		    fprintf(stderr,
"%s: Could not read this site platform information file.\n",
			val
		    );
		}

		FREE(val);
            }

            /* Message configure start up. */
            else if(!STRCASECMP(parm, "MessageConfigurationStartup") ||
                    !STRCASECMP(parm, "MessageConfiguringStartup") ||
                    !STRCASECMP(parm, "MessageConfigStartup")
            )
            {
                char *val = FGETLINE(fp);

                PConfLoadPlatformsSanitizeLine(val);

                FREE(core_ptr->message_configure_startup);
                core_ptr->message_configure_startup = STRDUP(val);

                FREE(val);
            }
            /* Message platform unsupported. */
            else if(!STRCASECMP(parm, "MessagePlatformUnsupported"))
            {
                char *val = FGETLINE(fp);

                PConfLoadPlatformsSanitizeLine(val);

                FREE(core_ptr->message_platform_unsupported);
                core_ptr->message_platform_unsupported = STRDUP(val);

                FREE(val);
            }
            /* Message dependency failed. */
            else if(!STRCASECMP(parm, "MessageDependencyFailed") ||
                    !STRCASECMP(parm, "MessageDependentFailed") ||
                    !STRCASECMP(parm, "MessageDependFailed")
	    )
            {
                char *val = FGETLINE(fp);

                PConfLoadPlatformsSanitizeLine(val);

                FREE(core_ptr->message_depend_failed);
                core_ptr->message_depend_failed = STRDUP(val);

                FREE(val);
            }
            /* Message success. */
            else if(!STRCASECMP(parm, "MessageSuccess") ||
                    !STRCASECMP(parm, "MessageSucc")
            )
            {
                char *val = FGETLINE(fp);

                PConfLoadPlatformsSanitizeLine(val);

                FREE(core_ptr->message_success);
                core_ptr->message_success = STRDUP(val);

                FREE(val);
            }



	    else
            {
		if(core_ptr->warnings)
		    fprintf(
			stderr,
			"%s: Unsupported parameter `%s'.\n",
			filename, parm
		    );
                FSEEKNEXTLINE(fp);
            }

        } while(1);

	/* Close platforms file. */
	FCLOSE(fp);
	fp = NULL;

	DO_RESET_CONTEXT_PTRS

#undef DO_RESET_CONTEXT_PTRS

	return(0);
}



/*
 *	Loads information from the `this platform' configuration file
 *	which specifies information about the platform this program is
 *	running on into the given core structure.
 */
int PConfLoadThisPlatformFromFile(
        pconf_core_struct *core_ptr, const char *filename
)
{
        FILE *fp;
        char *buf;


        if((core_ptr == NULL) || (filename == NULL))
            return(-1);

	fp = FOPEN(filename, "rb");
	if(fp == NULL)
	    return(-1);


	/* Begin reading each line. */

	/* Line 1: OS name. */
	buf = FGETLINE(fp);
	if(buf != NULL)
	{
	    FREE(core_ptr->os_name);
	    core_ptr->os_name = STRDUP(buf);
	}
	FREE(buf);

	/* Line 2: OS version (or release). */
        buf = FGETLINE(fp);
        if(buf != NULL)
        {

        }
        FREE(buf);

	/* Line 3: Machine name. */
        buf = FGETLINE(fp);
        if(buf != NULL)
        {
            FREE(core_ptr->machine_name);
            core_ptr->machine_name = STRDUP(buf);
        }
        FREE(buf);

	/* Ignore additional lines. */


        /* Close platforms file. */
        FCLOSE(fp);
        fp = NULL;

        return(0);
}


/*
 *	Generates the output Makefile from the data on the given
 *	core structure and platform.
 */
void PConfGenerateOutputMakefile(
	pconf_core_struct *core_ptr, platform_struct *platform_ptr
)
{
	int i;
	FILE *fp_in, *fp_out;
	const char	*makefile_output,
			*makefile_input_prepend,
			*makefile_input_append;
	feature_struct *feature_ptr;


	if((core_ptr == NULL) || (platform_ptr == NULL))
	    return;

	makefile_output = (const char *)core_ptr->makefile_output;
	makefile_input_prepend = (const char *)core_ptr->makefile_input_prepend;
	makefile_input_append = (const char *)core_ptr->makefile_input_append;

	/* Use defaults as needed. */
	if(makefile_output == NULL)
	    makefile_output = "Makefile";

	/* Generate output Makefile. */
	fp_out = FOPEN(makefile_output, "w");
	if(fp_out == NULL)
	{
	    /* Error generating output Makefile. */
	    fprintf(
		stderr,
		"%s: Cannot create.\n",
		makefile_output
	    );
	    return;
	}


	printf("Generating output Makefile `%s'...\n",
	    makefile_output
	);


	/* Add input prepend file data if any. */
	fp_in = FOPEN(makefile_input_prepend, "rb");
	if(fp_in != NULL)
	{
	    int c;

	    do
	    {
		c = FGETC(fp_in);
		if(c == EOF)
		    break;

		FPUTC(c, fp_out);

	    } while(1);

	    FCLOSE(fp_in);
	    fp_in = NULL;
	}


	/* Begin writing platform specific definations to output
	 * Makefile.
	 */
	/* PREFIX. */
	if(platform_ptr->prefix != NULL)
	{
	    fprintf(fp_out,
		"PREFIX = %s",
		platform_ptr->prefix
	    );
	    FPUTC('\n', fp_out);
	    FPUTC('\n', fp_out);
	}

	/* CFLAGS. */
	fprintf(fp_out,
	    "CFLAGS ="
	);
        if(platform_ptr->cflags != NULL)
            fprintf(fp_out,
                " %s",
                platform_ptr->cflags
            );
	for(i = 0; i < platform_ptr->total_features; i++)
	{
	    feature_ptr = platform_ptr->feature[i];
	    if(feature_ptr == NULL)
		continue;

	    if(feature_ptr->must_exist != PCONF_MUST_EXIST_YES)
		continue;

	    if(feature_ptr->cflags != NULL)
		fprintf(fp_out,
		   " %s",
		    feature_ptr->cflags
		);
	}
        FPUTC('\n', fp_out);
        FPUTC('\n', fp_out);

        /* INC_DIRS. */
        fprintf(fp_out,
            "INC_DIRS ="
        );
        if(platform_ptr->inc_dirs != NULL)
            fprintf(fp_out,
                " %s",
                platform_ptr->inc_dirs
            );
        for(i = 0; i < platform_ptr->total_features; i++)
        {
            feature_ptr = platform_ptr->feature[i];
            if(feature_ptr == NULL)
                continue;

            if(feature_ptr->must_exist != PCONF_MUST_EXIST_YES)
                continue;

            if(feature_ptr->inc_dirs != NULL)
                fprintf(fp_out,
                   " %s",
                    feature_ptr->inc_dirs
                );
        }
        FPUTC('\n', fp_out);
        FPUTC('\n', fp_out);


        /* LIBS. */
        fprintf(fp_out,
            "LIBS ="
        );
        if(platform_ptr->libs != NULL)
            fprintf(fp_out,
                " %s",
                platform_ptr->libs
            );     
        for(i = 0; i < platform_ptr->total_features; i++)
        {         
            feature_ptr = platform_ptr->feature[i];
            if(feature_ptr == NULL)
                continue;
        
            if(feature_ptr->must_exist != PCONF_MUST_EXIST_YES)
                continue;

            if(feature_ptr->libs != NULL)
                fprintf(fp_out,
                   " %s",
                    feature_ptr->libs
                );
        }
        FPUTC('\n', fp_out);
        FPUTC('\n', fp_out);


        /* LIB_DIRS. */
        fprintf(fp_out,
            "LIB_DIRS ="
        );
        if(platform_ptr->lib_dirs != NULL)
            fprintf(fp_out,
                " %s",
                platform_ptr->lib_dirs
            );
        for(i = 0; i < platform_ptr->total_features; i++)
        {
            feature_ptr = platform_ptr->feature[i];
            if(feature_ptr == NULL)
                continue;
                
            if(feature_ptr->must_exist != PCONF_MUST_EXIST_YES)
                continue;
            
            if(feature_ptr->lib_dirs != NULL)
                fprintf(fp_out,
                   " %s",
                    feature_ptr->lib_dirs
                );
        }
        FPUTC('\n', fp_out);
        FPUTC('\n', fp_out);

        /* CC. */
        if(platform_ptr->cc != NULL)
        {
            fprintf(fp_out,
                "CC = %s",
                platform_ptr->cc
            );
            FPUTC('\n', fp_out);
            FPUTC('\n', fp_out);
        }
        /* CPP. */
        if(platform_ptr->cpp != NULL)
        {
            fprintf(fp_out,
                "CPP = %s",
                platform_ptr->cpp
            );
            FPUTC('\n', fp_out);
            FPUTC('\n', fp_out);
        }



        /* Add input append file data if any. */
        fp_in = FOPEN(makefile_input_append, "rb");
        if(fp_in != NULL)
        {
            int c;

            do
            {
                c = FGETC(fp_in);
                if(c == EOF)
                    break;

                FPUTC(c, fp_out);

            } while(1);

            FCLOSE(fp_in);
            fp_in = NULL;
        }


	/* Close output Makefile. */
	FCLOSE(fp_out);
	fp_out = NULL;

	return;
}
