/*
 *	cconf header file, all #included resources and prototypes used
 *	by pconf must be specified here.
 *
 *	None of pconf's sources #include anything or assume anything
 *	defined that is not in here.
 *
 *	See README and AUTHORS file for details of purpose and 
 *	maintainers.
 */
#ifndef PCONF_H
#define PCONF_H

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>

#ifndef ISCR
# define ISCR(c)	(((c) == '\n') || ((c) == '\r'))
#endif

#ifndef ISBLANK
# define ISBLANK(c)	(((c) == ' ') || ((c) == '\t'))
#endif

#ifndef DIR_DELIMINATOR
# define DIR_DELIMINATOR	'/'
#endif


/* pconf specific definations, adjust as needed. */

#define PROG_NAME		"Compiler Configurator"
#define PROG_VERSION		"0.0.1"
#define PROG_VERSION_MAJOR	0
#define PROG_VERSION_MINOR	0
#define PROG_VERSION_RELEASE	1


/* Configuration file comment character. */
#define PCONF_CFG_COMMENT_CHAR	'#'

/* Configuration file parameter and value deliminator character. */
#define PCONF_CFG_DELIM_CHAR	'='


/* ANSI color codes. */
#define PCONF_COLOR_DEFAULT	0
#define PCONF_COLOR_HEADING	-1
#define PCONF_COLOR_SELECTABLE	-3
#define PCONF_COLOR_SUCCESS	32
#define PCONF_COLOR_FAILURE	31
#define PCONF_COLOR_WARNING	33

/* Must exist codes. */
#define PCONF_MUST_EXIST_NO		0	/* Disable. */
#define PCONF_MUST_EXIST_PREFERRED	1	/* Enable if exists. */
#define PCONF_MUST_EXIST_YES		2	/* Must exist or fail. */


/* Dependancy structure. */
typedef struct {

	/* Dependent entity type, one of PCONF_DEP_TYPE_*.
	 * This determines which search paths on the platform structure
	 * to use for finding paths specified in this structure.
	 */
#define PCONF_DEP_TYPE_OTHER	0
#define PCONF_DEP_TYPE_DATA	1
#define PCONF_DEP_TYPE_CONFIG	2
#define PCONF_DEP_TYPE_LIBRARY	3
#define PCONF_DEP_TYPE_HEADER	4
#define PCONF_DEP_TYPE_PROGRAM	5

#define PCONF_DEP_TYPE_OS	10	/* Operating system type. */
#define PCONF_DEP_TYPE_MACHINE	11	/* Machine type. */

	int type;

        /* Name of this dependent entity (can be arbitary). */
        char *name;

	/* Verbose description of this dependent entity. */
	char *description;

        /* This dependent entity must exist, one of PCONF_MUST_EXIST_*
	 * (but not PCONF_MUST_EXIST_PREFERRED).
	 */
        int must_exist;

        /* Entity file names to search for (the same file name may
	 * need to be specified multiple times, see func_name member).
	 */
        char **path;
        int total_paths;

        /* Strings to search for corresponding index of files. Each
	 * (non-empty) string is searched for in the specified paths
	 * in the path member (if it is not a directory).
	 */
        char **grep_string;
        int total_grep_strings;


	/* Operating system name, used only if type is
	 * PCONF_DEP_TYPE_OS (preferably value fetched from uname).
	 */
	char *os_name;

	/* Machine name, used only if type is
	 * PCONF_DEP_TYPE_MACHINE (preferably value fetched from uname).
	 */
	char *machine_name;

} depend_struct;

/* Feature structure, optional requirements for any given platform. */
typedef struct {

        /* Name of this feature (can be arbitary). */
        char *name;

        /* Verbose description of this feature. */
        char *description;

	/* URL of home page. */
	char *url_homepage;
	/* URL of download location. */
	char *url_download;

        /* This feature must exist, one of PCONF_MUST_EXIST_*. */
        int must_exist;

        /* List of dependant entities. */
        depend_struct **depend;
        int total_depends;

        /* Extra info to append. */
        char *cflags;
	char *inc_dirs;
        char *libs;
        char *lib_dirs;

} feature_struct;

/* Platform structure. */
typedef struct {

        /* Name of this platform, this value is used to match value
         * of platform from command line.
         */
        char *name;

	/* Verbose description of this platform. */
	char *description;

        /* List of features. */
        feature_struct **feature;
        int total_features;

	/* Search paths for header files. */
	char **path_header;
	int total_path_headers;

	/* Search paths for library files. */
	char **path_library;
	int total_path_libraries;

        /* Search paths for configuration files. */
        char **path_config;
        int total_path_configs;

        /* Search paths for program files. */
        char **path_program;
        int total_path_programs;

        /* Search paths for data and other files, these paths
	 * are used when a search path needs to be specified for
	 * an object that dosen't really fall into the above
	 * categories.
	 */
        char **path_data;
        int total_path_datas;


	/* Global options for this platform. */
	char *prefix;
	char *cflags;
	char *inc_dirs;
	char *libs;
	char *lib_dirs;
        char *cc;               /* C compiler. */
        char *cpp;              /* C++ compiler. */

} platform_struct;

/* PConf core structure. */
typedef struct {

	int verbose;	/* Print extra verbose messages. */
	int warnings;	/* Print warnings (if any). */
	int colors;	/* Enable ansi colors. */
	int interactive;
	int force;	/* Keep going even if errors encountered. */
	int ignore_environments;	/* Ignore environment variables. */

	int screen_width;	/* 0 for undefined. */

	/* Loaded list of supported platforms. */
	platform_struct **platform;
	int total_platforms;

	/* Pointer to selected platform. */
	platform_struct *selected_platform;

	/* Output Makefile path. */
	char *makefile_output;

	/* Input Makefile paths. */
	char *makefile_input_prepend;
	char *makefile_input_append;

	/* Messages. */
	char *message_configure_startup;
	char *message_platform_unsupported;
	char *message_depend_failed;
	char *message_success;

	/* Values fetched from the thisplatform.ini. */
	char *os_name;
	char *machine_name;

} pconf_core_struct;


/* In fio.c */
extern void PConfFreePlatform(platform_struct *platform_ptr);
extern void PConfFreePlatforms( 
        platform_struct ***list, int *total
);
extern int PConfLoadPlatformsFromFile(
        pconf_core_struct *core_ptr, const char *filename
);
extern int PConfLoadThisPlatformFromFile(
        pconf_core_struct *core_ptr, const char *filename
);
extern void PConfGenerateOutputMakefile(
        pconf_core_struct *core_ptr, platform_struct *platform_ptr
);

/* In main.c */
extern void PConfSetColor(
	pconf_core_struct *core_ptr, FILE *stream, int color_code
);
extern void PConfPrintHR(pconf_core_struct *core_ptr);
extern feature_struct *PConfMatchFeatureByName(
	platform_struct *platform_ptr,
	const char *name, int *n
);


/* In proc.c */
extern void PConfApplyArgsToPlatform(
        pconf_core_struct *core_ptr, platform_struct *platform_ptr,
        int argc, char *argv[]
);
extern int PConfCheckDepends(
        pconf_core_struct *core_ptr, platform_struct *platform_ptr
);

/* In utils.c */
extern void FREE(void *p);
extern int STRLEN(const char *s);
extern int STRISYES(const char *s);
extern char *STRDUP(const char *s);
extern const char *STRCHR(const char *s, int c);
extern void STRSTRIP(char *s);
extern int STRCASECMP(const char *s1, const char *s2);
extern int STRPFX(const char *str, const char *pfx);
extern int STRCASEPFX(const char *str, const char *pfx);
extern char *STRLISTAPPEND(char ***list, int *total, const char *s);
extern void STRFREEARRAY(char **list, int total);

extern char *GETENV(const char *parameter);

extern FILE *FOPEN(const char *path, const char *mode);
extern void FCLOSE(FILE *fp);
extern int FGETC(FILE *fp);
extern int FPUTC(int c, FILE *fp);
extern void FSEEKNEXTLINE(FILE *fp);
extern void FSEEKPASTSPACES(FILE *fp);
extern char *FGETLINE(FILE *fp);
extern char *FSEEKNEXTPARAMETER(
	FILE *fp, char *buf, char comment, char delim
);

#endif	/* PCONF_H */
