/*
                      U N I V E R S E   E D I T O R


	Copyright (C) 1997-2001 WolfPack Entertainment

 */

#ifndef UE_H
#define UE_H

#include <time.h>

#include "../include/osw-x.h"
#include "../include/widget.h"
#include "../include/isrefs.h"

#include "comfwin.h"
#include "config.h"


/*
 *	Program name and version:
 */
#define PROG_NAME	"Universe Editor"
#define PROG_VERSION	"1.34.0"

#define PROG_VERSION_MAJOR	1
#define PROG_VERSION_MINOR	34
#define PROG_VERSION_RELEASE	0



/*
 *	Program help message:
 */
#define PROG_HELP_MESG	"\
Usage: unvedit [file] [options] [GUI_options]\n\
\n\
    [file] specifies the name of the universe file to load at\n\
    startup.\n\
\n\
    [options] can be any of the following:\n\
\n\
        --help                  Prints (this) help screen and exits.\n\
        --version               Prints version information and exits.\n\
\n\
    Most customizations can be performed in the options menu.\n\
\n\
    Command line options override any options in the\n\
    configuration file.\n\
\n\
    [GUI_options] can be any options standard to your GUI, consult\n\
    your GUI's manual for available options.\n\
\n"


/*
 *	Program copyright message:
 */
#define PROG_COPYRIGHT	"\
Copyright (C) 1997-2001 WolfPack Entertainment.\n\
This program is protected by international copyright laws and treaties,\n\
distribution and/or modification of this software in violation of the\n\
GNU Public License is strictly prohibited. Violators will be prosicuted\n\
to the fullest extent of the law."


/*
 *	Files:
 */
typedef struct {

	char rc[PATH_MAX + NAME_MAX]; 

} ue_fname_struct;
extern ue_fname_struct fname;

/*
 *	Directories:
 */
typedef struct {

	char toplevel[PATH_MAX];	/* XSW data toplevel dir. */

	char images[PATH_MAX];
        char server[PATH_MAX];

} ue_dname_struct;
extern ue_dname_struct dname;


/*
 *	File browser op codes:
 */
#define UE_FB_OP_CODE_NONE	0
#define UE_FB_OP_CODE_OPEN	1
#define UE_FB_OP_CODE_OPENNEW	2
#define UE_FB_OP_CODE_SAVEAS	3


/*
 *	Global options:
 */
typedef struct {

	int	rc_version_major,
		rc_version_minor,
		rc_version_release;

	char label_geometry;
	char show_grid;
	double grid_spacing;	/* In XSW real units. */

#ifndef FontNameMax
# define FontNameMax	256
#endif
	char view_font_name[FontNameMax];
	char view_object_label_font_name[FontNameMax];

	char show_preview_image;
	char animate_images;

} ue_option_struct;
extern ue_option_struct option;



/*
 *	Global runlevel:
 */
extern int runlevel;


/*
 *	Current time in milliseconds
 */
extern time_t cur_millitime;


/*
 *	Fonts:
 */
typedef struct {

	font_t	*view_obj_label,
		*view_label;

} ue_font_struct;
extern ue_font_struct ue_font;


/*
 *	Cursors:
 */
typedef struct {

	WCursor	*standard,
		*translate,
		*zoom,
		*h_split,
		*v_split,
		*scanner_lock;

} ue_cursor_struct;
extern ue_cursor_struct ue_cursor;


/*
 *	Images:
 */
typedef struct {

	pixmap_t unvedit_icon_pm;

	image_t *error,
		*info,
		*question,
		*tb_copy,
		*tb_economy,
		*tb_new,
		*tb_newobj,
		*tb_open,
		*tb_paste,
		*tb_print,
		*tb_save,
		*tb_weapons,
		*unvedit_icon,
		*unvedit_logo,
		*warning;

} ue_image_struct;
extern ue_image_struct ue_image;



/*
 *	Comfiermation window:
 */
extern comfirm_win_struct comfwin;

/*
 *	Dialog widget:
 */
extern dialog_win_struct dialog;

/*
 *	File browser:
 */
extern fbrowser_struct file_browser;
extern void *file_browser_src_ptr;	/* Source that mapped fb. */
extern int file_browser_op_code;	/* What is fb suppose to do. */


/*
 *	Universe editor memory stats structure:
 */
typedef struct {

	unsigned long	total,
			uew,
			universe,
			isrefs;

} ue_memory_stats_struct;


/* In main.c */
extern int UEDoEmergencySaveAll();
extern void UESignalHandler(int s);
extern int UEScrollBarCB(scroll_bar_struct *sb);

extern int UEGetMemoryStats(ue_memory_stats_struct *buf);

extern int UEFileBrowserOKCB(char *path);
extern int UEFileBrowserCancelCB(char *path);
extern int UEComfWinStdManageCB(event_t *event);
extern int UECreateUEW(int argc, char *argv[]);
extern int UECreateUHW(int argc, char *argv[]);
extern int UECreateWepW(int argc, char *argv[]);
extern int UECreateEcoW(int argc, char *argv[]);

extern void UEResetTimmers();

extern int UEInit(int argc, char *argv[]);
extern void UEManage();
extern void UEShutdown();


/* In isrefs.c */
extern int ISRefIsLoaded(isref_struct *isref_ptr);
extern int ISRefLoad(isref_struct *isref_ptr);
extern void ISRefUnload(isref_struct *isref_ptr);
extern void ISRefDelete(isref_struct *isref_ptr);
extern void ISRefDeleteAll(isref_struct **isref_ptr, int total);
extern void ISRefManage(isref_struct **isref_ptr, int total);

/* In isrefsfile.c */
extern isref_struct *ISRefAllocate();
extern isref_struct **ISRefLoadFromFile(
        char *file,
        int *total,
        char *image_dir
);


/* In timming.c */
extern "C" long MilliTime(void);


#endif	/* UE_H */
