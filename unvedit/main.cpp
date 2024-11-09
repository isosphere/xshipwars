// unvedit/main.cpp
/*
                          Program Primary Routines

	Functions:

	int UEDoEmergencySaveAll()
	void UESignalHandler(int s)
	int UEScrollBarCB(scroll_bar_struct *sb)

	int UEGetMemoryStats(ue_memory_stats_struct *buf)

	int UEFileBrowserOKCB(char *path)
	int UEFileBrowserCancelCB(char *path)
	int UEComfWinStdManageCB(event_t *event)
	int UECreateUEW(int argc, char *argv[])
	int UECreateUHW(int argc, char *argv[])
	int UECreateWepW(int argc, char *argv[])

	void UEResetTimmers()

	int UEInit(int argc, char *argv[])
	void UEManage()
	void UEShutdown()

	int main(int argc, char *argv[])

	---

 */
/*
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
*/
#include <unistd.h>
#include <signal.h>
#include <sys/stat.h>

#include "../include/strexp.h"
#include "../include/string.h"
#include "../include/disk.h"
#include "../include/os.h"
#include "../include/osw-x.h"
#include "../include/widget.h"
#include "../include/objects.h"
#include "../include/isrefs.h"
#include "../include/unvmain.h"
#include "../include/unvmatch.h"
#include "../include/unvutil.h"
#include "../include/unvfile.h"

#include "rcfile.h"
#include "printwin.h"
#include "optwgen.h"
#include "ecow.h"
#include "wepw.h"
#include "uhw.h"
#include "uew.h"
#include "aboutwin.h"
#include "keymap.h"
#include "ue.h"

#include "../include/cursors/h_split.xpm"
#include "../include/cursors/std_arrow.xpm"
#include "../include/cursors/translate.xpm"
#include "../include/cursors/v_split.xpm"
#include "../include/cursors/xsw_scanner_lock.xpm"
#include "../include/cursors/zoom.xpm"

#include "../include/icons/info.h"
#include "../include/icons/error.h"
#include "../include/icons/question.h"
#include "../include/icons/warning.h"

uew_struct **uew;
int total_uews;
uew_struct *delete_uew;

uhw_struct **uhw;
int total_uhws;

wepw_struct **wepw;
int total_wepws;



ue_fname_struct fname;
ue_dname_struct dname;

ue_option_struct option;

about_win_struct about_win;

ecow_struct **ecow;             
int total_ecows;

optwgen_struct optwgen;
print_win_struct print_win;

keymap_struct keymap[TOTAL_KEYMAPS];

int runlevel;
time_t cur_millitime;

ue_font_struct ue_font;
ue_cursor_struct ue_cursor;
ue_image_struct ue_image;

comfirm_win_struct comfwin;
dialog_win_struct dialog;

fbrowser_struct file_browser;
void *file_browser_src_ptr;
int file_browser_op_code;



/*
 *	Procedure to do emergancy save all.
 */
int UEDoEmergencySaveAll()
{
	int i, status;
	char *dir_name;
	char *tmp_name;
	struct stat stat_buf;


	for(i = 0; i < total_uews; i++)
	{
	    if(uew[i] == NULL)
		continue;

	    /* No objects to save? */
	    if(uew[i]->total_objects <= 0)
		continue;


	    /* Get user's home dir. */
	    dir_name = getenv("HOME");
	    if(dir_name == NULL)
		dir_name = "/tmp";
	    if(stat(dir_name, &stat_buf))
		dir_name = "/";

	    /* Create a tempory file name. */
	    tmp_name = tempnam(dir_name, "univ");
	    if(tmp_name == NULL)
		continue;

            /* Update universe header. */
            strncpy(
                uew[i]->unv_header.version,
                PROG_VERSION,
                UNV_TITLE_MAX
            );
            uew[i]->unv_header.version[UNV_TITLE_MAX - 1] = '\0';

            /* Save universe. */
            status = UNVSaveToFile(
                tmp_name,
                uew[i]->object,  
                uew[i]->total_objects,
                &uew[i]->unv_header,
                NULL,
		NULL
            );
            if(status < 0)
            {
		fprintf(
		    stderr,
		    "%s: Emergency save failed.\n",
		    tmp_name
		);
            }
	    else
            {
                fprintf(
                    stderr,
                    "%s: Emergency save successful.\n",
		    tmp_name  
                );
            } 

	    free(tmp_name);
	    tmp_name = NULL;
	}



	return(0);
}


/*
 *	Signal handler.
 */
void UESignalHandler(int s)
{

	switch(s)
	{
          /* ****************************************************** */
          case SIGINT:
	    UEDoEmergencySaveAll();
            runlevel = 1;
            break;

          /* ****************************************************** */
          case SIGTERM:
	    UEDoEmergencySaveAll();
            runlevel = 1;
            break;

          /* ****************************************************** */
          case SIGQUIT:
	    UEDoEmergencySaveAll();
            runlevel = 1;
            break;

          /* ****************************************************** */
          case SIGABRT:
	    UEDoEmergencySaveAll();
            runlevel = 1;
            break;
            
          /* ****************************************************** */
          case SIGKILL:
	    UEDoEmergencySaveAll();
            runlevel = 1;
            break;

          /* ****************************************************** */
          case SIGPIPE:
            signal(SIGPIPE, UESignalHandler);
            break;

          /* ****************************************************** */
          case SIGSEGV:
	    fprintf(stderr, "Segmentation fault.\n");
	    UEDoEmergencySaveAll();
            runlevel = 1;
            break;

          /* ****************************************************** */
          case SIGCONT:
            signal(SIGCONT, UESignalHandler);
            break;

          /* ****************************************************** */
	  default:
	    break;
	}


	return;
}


/*
 *	Scroll bar callback handler.
 */
int UEScrollBarCB(scroll_bar_struct *sb)
{
	int i;


	for(i = 0; i < total_uews; i++)
	{
	    if(uew[i] == NULL)
		continue;

	    if(&uew[i]->props_sb == sb)
	    {
/*
		UEWPropsDraw(uew[i], 0);
 */
	    }
	}


	return(0);
}


/*
 *	Calculates the amount of dynamic memory used by this program.
 */
int UEGetMemoryStats(ue_memory_stats_struct *buf)
{
	int i, n, j;
	uew_struct *uew_ptr;
	xsw_object_struct *obj_ptr;
	isref_struct *isref_ptr;


	if(buf == NULL)
	    return(-1);


	buf->total = 0;
	buf->uew = 0;
	buf->universe = 0;
	buf->isrefs = 0;


	/* Universe editors. */
	for(i = 0; i < total_uews; i++)
	{
	    uew_ptr = uew[i];

	    if(uew_ptr == NULL)
		continue;

	    buf->uew += sizeof(uew_struct);

	    /* Universe objects. */
	    for(n = 0; n < uew_ptr->total_objects; n++)
	    {
		obj_ptr = uew_ptr->object[n];

		if(obj_ptr == NULL)
		    continue;

                buf->universe += sizeof(xsw_object_struct);

		/* Object weapon. */
		for(j = 0; j < obj_ptr->total_weapons; j++)
		{
		    if(obj_ptr->weapons[j] == NULL)
			continue;

		    buf->universe += sizeof(xsw_weapons_struct);
		}
		buf->universe += (sizeof(xsw_weapons_struct *) *
                    obj_ptr->total_weapons);

		/* Object scores. */
		if(obj_ptr->score != NULL)
		    buf->universe += sizeof(xsw_score_struct);

		/* Object economy. */
		if(obj_ptr->eco != NULL)
		{
		    buf->universe += sizeof(xsw_ecodata_struct);
		    for(j = 0; j < obj_ptr->eco->total_products; j++)
		    {
			if(obj_ptr->eco->product[j] == NULL)
			    continue;

			buf->universe += sizeof(xsw_ecoproduct_struct);
		    }
		    buf->universe += (sizeof(xsw_ecoproduct_struct *) *
			obj_ptr->eco->total_products);
		}
	    }
	    buf->universe += (uew_ptr->total_objects *
		sizeof(xsw_object_struct *));


	    /* Isrefs. */
            for(n = 0; n < uew_ptr->total_isrefs; n++)
            {
		isref_ptr = uew_ptr->isref[n];
		if(isref_ptr == NULL)
		    continue;

		buf->isrefs += sizeof(isref_struct);

		/* Image data not NULL implies it is loaded. */
		if(isref_ptr->image_data != NULL)
		{
		    buf->isrefs += ((osw_gui[0].depth << 3) *
			isref_ptr->width * isref_ptr->height
		    );
		}

		for(j = 0; j < isref_ptr->total_point_lights; j++)
		{
		    if(isref_ptr->point_light[j] == NULL)
			continue;

		    buf->isrefs += sizeof(isref_point_light_struct);
		}
		buf->isrefs += (isref_ptr->total_point_lights *
		    sizeof(isref_point_light_struct *));
	    }
            buf->isrefs += (uew_ptr->total_isrefs *
                sizeof(isref_struct *));

	}
	buf->uew += (total_uews * sizeof(uew_struct *));


	/* Calculate total. */
	buf->total = buf->uew + buf->universe + buf->isrefs;


	return(0);
}


/*
 *	File browser OK callback.
 */
int UEFileBrowserOKCB(char *path)
{
	int i;
	uew_struct *uew_ptr = NULL;


	/* No referance window to file browser operation. */
	if(file_browser_src_ptr == NULL)
	    return(0);

	/*   Find which window structure the file browser is to
	 *   operate on.
	 */
	for(i = 0; i < total_uews; i++)
	{
	    if((void *)uew[i] == file_browser_src_ptr)
	    {
		uew_ptr = uew[i];
		break;
	    }
	}

	/* Warning, uew_ptr may be NULL. */


	/* Handle file browser operation. */
	switch(file_browser_op_code)
	{
	  case UE_FB_OP_CODE_OPEN:
	    UEWDoOpen(uew_ptr, path);
	    file_browser_src_ptr = NULL;
            file_browser_op_code = UE_FB_OP_CODE_NONE;
	    break;

          case UE_FB_OP_CODE_OPENNEW:
            UEWDoOpenNew(uew_ptr, path); 
            file_browser_src_ptr = NULL;
            file_browser_op_code = UE_FB_OP_CODE_NONE;
            break;

          case UE_FB_OP_CODE_SAVEAS:
            UEWDoSaveAs(uew_ptr, path); 
            file_browser_src_ptr = NULL;
            file_browser_op_code = UE_FB_OP_CODE_NONE;
            break;

	  case UE_FB_OP_CODE_NONE:
            file_browser_src_ptr = NULL;
            file_browser_op_code = UE_FB_OP_CODE_NONE;
	    break;

          default:
            file_browser_src_ptr = NULL;
            file_browser_op_code = UE_FB_OP_CODE_NONE;
            break;
	}


	return(0);
}


/*
 *	File browser cancel callback.
 */
int UEFileBrowserCancelCB(char *path)
{



        file_browser_src_ptr = NULL;
        file_browser_op_code = UE_FB_OP_CODE_NONE;

        return(0);
}


/*
 *	Standard GUI management for event while querying
 *	in the comfiermation window.
 */
int UEComfWinStdManageCB(event_t *event)
{
	int events_handled = 0;


	/* Manage all universe edit windows. */
	events_handled += UEWManageAll(event);
	events_handled += UHWManageAll(event);
        events_handled += WepWManageAll(event);
        events_handled += EcoWManageAll(event);
        events_handled += PrintWinManage(event);
        events_handled += OptWGenManage(event);
        events_handled += AboutWinManage(event);


	/* External widgets. */
        events_handled += DialogWinManage(
            &dialog,
            event
        );
        events_handled += FBrowserManage(
            &file_browser,
            event
        );


	return(events_handled);
}


/*
 *	Procedure to allocate and initialize a new universe
 *	editor window.
 */
int UECreateUEW(int argc, char *argv[])
{
	// Dan S: renamed from "new" to "new_ue" for C++ key word compatibility.
	int new_ue, status;

	new_ue = UEWAllocate();
	if(!UEWIsAllocated(new_ue))
	    return(-1);

	status = UEWInit(new_ue);
	if(status)
	    return(-1);


	/* Do not map it. */


	return(new_ue);
}


/*
 *	Procedure to allocate and initialize a new universe
 *      header window.
 */
int UECreateUHW(int argc, char *argv[])
{
	// Dan S: renamed from "new" to "new_uhw" for C++ key word compatibility.
        int new_uhw, status;


        new_uhw = UHWAllocate();
        if(!UHWIsAllocated(new_uhw))
            return(-1);

        status = UHWInit(new_uhw, NULL);
        if(status)
            return(-1);  


        /* Do not map it. */


        return(new_uhw);
}

/*
 *      Procedure to allocate and initialize a new weapons
 *      window.
 */
int UECreateWepW(int argc, char *argv[])
{           
	// Dan S: renamed from "new" to "new_ww" for C++ key word compatibility.
        int new_ww, status;


        new_ww = WepWAllocate();
        if(!WepWIsAllocated(new_ww))
            return(-1);

        status = WepWInit(new_ww, NULL);
        if(status)
            return(-1);
        
        
        /* Do not map it. */
            
                
        return(new_ww);
}


/*
 *	Procedure to allocate and initialize a new economy
 *	window.
 */
int UECreateEcoW(int argc, char *argv[])
{
 	// Dan S: renamed from "new" to "new_ew" for C++ key word compatibility.
        int new_ew, status;


        new_ew = EcoWAllocate();
        if(!EcoWIsAllocated(new_ew))
            return(-1);

        status = EcoWInit(new_ew, NULL);
        if(status)
            return(-1);

        /* Do not map it. */


        return(new_ew);
}


void UEResetTimmers()
{
	int i, n, j;
	isref_struct **isref_ptr;
	uew_struct **uew_ptr;


	/* Go through universe editor windows. */
        for(i = 0, uew_ptr = uew; i < total_uews; i++, uew_ptr++)
        {
            if(*uew_ptr == NULL)
                continue;

	    for(n = 0, isref_ptr = (*uew_ptr)->isref;
                n < (*uew_ptr)->total_isrefs;
                n++
            )
	    {
		if(*isref_ptr == NULL)
		    continue;

		for(j = 0; j < (*isref_ptr)->total_point_lights; j++)
		{
		    if((*isref_ptr)->point_light[j] == NULL)
			continue;

		    (*isref_ptr)->point_light[j]->strobe_next = 0;
		}
	    }

	    (*uew_ptr)->ani_next = 0;
        }



	return;
}


/*
 *	Program primary initializer.
 */
int UEInit(int argc, char *argv[])
{
	int i, status;
	char *strptr, *strptr2;
	char cwd[PATH_MAX];
	char image_unvedit_path[PATH_MAX + NAME_MAX];
	char tmp_path[PATH_MAX + NAME_MAX];
	char startup_unv_file[PATH_MAX + NAME_MAX];
	WColorStruct color;



	/* Reset startup universe file path. */
	startup_unv_file[0] = '\0';


	/* Get current working directory. */
        getcwd(cwd, PATH_MAX);
        cwd[PATH_MAX - 1] = '\0';

	/* Global options. */
	option.label_geometry = 1;
	option.show_grid = 1;
	option.grid_spacing = DEF_GRID_SPACING;

	strcpy(option.view_font_name, "6x12");
        strcpy(option.view_object_label_font_name, "6x10");

	option.show_preview_image = 1;
	option.animate_images = 1;


	/* Set signal handler. */
        signal(SIGINT, UESignalHandler);
        signal(SIGTERM, UESignalHandler);
        signal(SIGQUIT, UESignalHandler);
        signal(SIGABRT, UESignalHandler);
        signal(SIGKILL, UESignalHandler);
        signal(SIGPIPE, UESignalHandler);
        signal(SIGSEGV, UESignalHandler);
        signal(SIGCONT, UESignalHandler);


	/* Reset globals. */
        strncpy(
            dname.toplevel,
            DEF_XSW_TOPLEVEL_DIR,
            PATH_MAX
        );
        dname.toplevel[PATH_MAX - 1] = '\0';

        strncpy(
            dname.images,
            DEF_XSW_IMAGES_DIR,
            PATH_MAX
        );
        dname.images[PATH_MAX - 1] = '\0';

        strncpy(
            dname.server,
            DEF_XSW_SERVER_DIR,
            PATH_MAX
        );
        dname.server[PATH_MAX - 1] = '\0';


	uew = NULL;
	total_uews = 0;

	uhw = NULL;
	total_uhws = 0;

	wepw = NULL;
	total_wepws = 0;	

	ecow = NULL;
	total_ecows = 0;


	UNVInit(argc, argv);


	/* ******************************************************** */
	/* Parse arguments. */
	for(i = 1; i < argc; i++)
	{
	    if(argv[i] == NULL)
		continue;


	    /* Help. */
	    if(strcasepfx(argv[i], "--h") ||
               strcasepfx(argv[i], "-h") ||
               strcasepfx(argv[i], "-?")
            )
	    {
		printf(PROG_HELP_MESG);
		return(-4);
	    }
            /* Version. */
            else if(strcasepfx(argv[i], "--ver") ||
                    strcasepfx(argv[i], "-ver")
            )
            {
                printf(
		    "%s Version %s\n%s\n",
		    PROG_NAME,
		    PROG_VERSION,
		    PROG_COPYRIGHT
		);
                return(-4);
            }
	    /* All else assume argument is a file path. */
	    else if((argv[i][0] != '-') &&
                    (argv[i][0] != '+')
	    )
	    {
		strncpy(
		    startup_unv_file,
		    argv[i],
		    PATH_MAX + NAME_MAX
		);
		startup_unv_file[PATH_MAX + NAME_MAX - 1] = '\0';

		if(!ISPATHABSOLUTE(startup_unv_file))
		{
		    strptr = PrefixPaths(cwd, startup_unv_file);
		    if(strptr != NULL)
		    {
			strncpy(
			    startup_unv_file,
			    strptr,
			    PATH_MAX + NAME_MAX
			);
			startup_unv_file[PATH_MAX + NAME_MAX - 1] = '\0';
		    }
		}

	    }
	}


	/* Load restart configuration. */
	strptr = getenv("HOME");
	if(strptr == NULL)
	    strptr = "/";
	strptr2 = PrefixPaths(strptr, DEF_CONFIG_FILE);

	strncpy(
	    fname.rc,
	    ((strptr2 == NULL) ? DEF_CONFIG_FILE : strptr2),
	    PATH_MAX + NAME_MAX
	);
	fname.rc[PATH_MAX + NAME_MAX - 1] = '\0';

	if(RCLoadFromFile(fname.rc))
	{
/*
	    fprintf(
		stderr,
		"%s: Error occured while loading configuration.\n",
		fname.rc
	    );
 */
	}



        /* Connect to GUI. */
        status = OSWGUIConnect(argc, argv);
        if(status)
            return(-1);

        /* Initialize widget globals. */  
        status = WidgetInitGlobals(argc, argv);
        if(status)
            return(-1);

	SBarSetNotifyFunction(UEScrollBarCB);

	/* Fonts. */
	OSWLoadFont(
	    &ue_font.view_obj_label,
            option.view_object_label_font_name
	);
	OSWLoadFont( 
            &ue_font.view_label,
            option.view_font_name
        );


	/* Cursors. */
	color.a = 0x00;
	color.r = 0xff;
	color.g = 0xff;
	color.b = 0xff;
	ue_cursor.standard = WidgetCreateCursorFromData(
	    std_arrow_xpm,
	    0, 0,
	    color
	);

        color.a = 0x00;
        color.r = 0xff;
        color.g = 0xb3;
        color.b = 0xfa;
        ue_cursor.translate = WidgetCreateCursorFromData(
	    translate_xpm,
	    8, 8,
	    color
        );

        color.a = 0x00;
        color.r = 0xff;
        color.g = 0xb3;
        color.b = 0xfa;
	ue_cursor.zoom = WidgetCreateCursorFromData(
	    zoom_xpm,
            8, 8,
            color
        );

        color.a = 0x00;
        color.r = 0xff;
        color.g = 0xff;
        color.b = 0xff;
        ue_cursor.h_split = WidgetCreateCursorFromData(
            h_split_xpm,
            8, 8,
            color
        );

        color.a = 0x00;
        color.r = 0xff;
        color.g = 0xff;
        color.b = 0xff;
        ue_cursor.v_split = WidgetCreateCursorFromData(
            v_split_xpm,
            8, 8,
            color
        );

        color.a = 0x00;
        color.r = 0xff;
        color.g = 0xb3;
        color.b = 0xfa;
        ue_cursor.scanner_lock = WidgetCreateCursorFromData(
            xsw_scanner_lock_xpm,
            8, 8,
            color
        );



	/* Images. */
	strptr = PrefixPaths(dname.images, "unvedit");
        strncpy(
            image_unvedit_path,
	    ((strptr == NULL) ? "/" : strptr),
            PATH_MAX + NAME_MAX
        );
	image_unvedit_path[PATH_MAX + NAME_MAX - 1] = '\0';


        ue_image.error = WidgetLoadImageFromTgaData(error_tga);
        ue_image.info = WidgetLoadImageFromTgaData(info_tga);
        ue_image.question = WidgetLoadImageFromTgaData(question_tga);

        strptr = PrefixPaths(image_unvedit_path, DEF_TB_COPY_IMG_FILENAME);
        strncpy(
            tmp_path,
            ((strptr == NULL) ? DEF_TB_COPY_IMG_FILENAME : strptr),
            PATH_MAX + NAME_MAX
        );
	tmp_path[PATH_MAX + NAME_MAX - 1] = '\0';
        ue_image.tb_copy = WidgetLoadImageFromTgaFile(tmp_path);

        strptr = PrefixPaths(image_unvedit_path, DEF_TB_ECONOMY_IMG_FILENAME);
        strncpy(
            tmp_path,
            ((strptr == NULL) ? DEF_TB_ECONOMY_IMG_FILENAME : strptr),
            PATH_MAX + NAME_MAX
        );
        tmp_path[PATH_MAX + NAME_MAX - 1] = '\0';
        ue_image.tb_economy = WidgetLoadImageFromTgaFile(tmp_path);

        strptr = PrefixPaths(image_unvedit_path, DEF_TB_NEW_IMG_FILENAME);
        strncpy(
            tmp_path,
            ((strptr == NULL) ? DEF_TB_NEW_IMG_FILENAME : strptr),
            PATH_MAX + NAME_MAX
        );
        tmp_path[PATH_MAX + NAME_MAX - 1] = '\0';
        ue_image.tb_new = WidgetLoadImageFromTgaFile(tmp_path);

        strptr = PrefixPaths(image_unvedit_path, DEF_TB_NEWOBJ_IMG_FILENAME);
        strncpy(
            tmp_path,
            ((strptr == NULL) ? DEF_TB_NEWOBJ_IMG_FILENAME : strptr),
            PATH_MAX + NAME_MAX
        );
        tmp_path[PATH_MAX + NAME_MAX - 1] = '\0';
        ue_image.tb_newobj = WidgetLoadImageFromTgaFile(tmp_path);

        strptr = PrefixPaths(image_unvedit_path, DEF_TB_OPEN_IMG_FILENAME);
        strncpy(
            tmp_path,
            ((strptr == NULL) ? DEF_TB_OPEN_IMG_FILENAME : strptr),
            PATH_MAX + NAME_MAX
        );
        tmp_path[PATH_MAX + NAME_MAX - 1] = '\0';
        ue_image.tb_open = WidgetLoadImageFromTgaFile(tmp_path);

        strptr = PrefixPaths(image_unvedit_path, DEF_TB_PASTE_IMG_FILENAME);
        strncpy(
            tmp_path,
            ((strptr == NULL) ? DEF_TB_PASTE_IMG_FILENAME : strptr),
            PATH_MAX + NAME_MAX
        );
        tmp_path[PATH_MAX + NAME_MAX - 1] = '\0';
        ue_image.tb_paste = WidgetLoadImageFromTgaFile(tmp_path);

        strptr = PrefixPaths(image_unvedit_path, DEF_TB_PRINT_IMG_FILENAME); 
        strncpy(
            tmp_path,
            ((strptr == NULL) ? DEF_TB_PRINT_IMG_FILENAME : strptr), 
            PATH_MAX + NAME_MAX
        );
        tmp_path[PATH_MAX + NAME_MAX - 1] = '\0';
        ue_image.tb_print = WidgetLoadImageFromTgaFile(tmp_path);

        strptr = PrefixPaths(image_unvedit_path, DEF_TB_SAVE_IMG_FILENAME);
        strncpy(
            tmp_path,
            ((strptr == NULL) ? DEF_TB_SAVE_IMG_FILENAME : strptr),
            PATH_MAX + NAME_MAX
        );
        tmp_path[PATH_MAX + NAME_MAX - 1] = '\0';
        ue_image.tb_save = WidgetLoadImageFromTgaFile(tmp_path);

        strptr = PrefixPaths(image_unvedit_path, DEF_TB_WEAPONS_IMG_FILENAME);
        strncpy(
            tmp_path,
            ((strptr == NULL) ? DEF_TB_WEAPONS_IMG_FILENAME : strptr),
            PATH_MAX + NAME_MAX
        );
        tmp_path[PATH_MAX + NAME_MAX - 1] = '\0';
        ue_image.tb_weapons = WidgetLoadImageFromTgaFile(tmp_path);

        strptr = PrefixPaths(image_unvedit_path, DEF_ICON_IMG_FILENAME);
        strncpy(
            tmp_path,
            ((strptr == NULL) ? DEF_ICON_IMG_FILENAME : strptr),
            PATH_MAX + NAME_MAX
        );
        tmp_path[PATH_MAX + NAME_MAX - 1] = '\0';
        ue_image.unvedit_icon = WidgetLoadImageFromTgaFile(tmp_path);

        strptr = PrefixPaths(image_unvedit_path, DEF_LOGO_IMG_FILENAME);
        strncpy(
            tmp_path,
            ((strptr == NULL) ? DEF_LOGO_IMG_FILENAME : strptr),
            PATH_MAX + NAME_MAX
        );
        tmp_path[PATH_MAX + NAME_MAX - 1] = '\0';
        ue_image.unvedit_logo = WidgetLoadImageFromTgaFile(tmp_path);

	ue_image.warning = WidgetLoadImageFromTgaData(warning_tga);


	ue_image.unvedit_icon_pm = OSWCreatePixmapFromImage(
	    ue_image.unvedit_icon
	);

	/* Comfermation window. */
	if(
	    ComfWinInit(
		&comfwin,
		ue_image.warning,	/* Icon. */
		(void *)&comfwin,
		UEComfWinStdManageCB
	    )
        )
            return(-1);

	/* Print window. */
	if(PrintWinInit())
	    return(-1);

	/* General options window. */
	if(OptWGenInit())
            return(-1);

	/* About window. */
	if(AboutWinInit())
	    return(-1);


	/* Initialize first universe edit window. */
	status = UECreateUEW(argc, argv);
	if(status < 0)
	    return(-1);
	UEWMap(status);



        /* Dialog. */
        status = DialogWinInit(
                &dialog,
                osw_gui[0].root_win,
                0, 0,                   /* Default width and height. */
                ue_image.error		/* Image. */
        );
        if(status)
            return(-1);

        /* File browser. */
        status = FBrowserInit(
                &file_browser,
                0, 0,
                0, 0,
                cwd,                    /* Initial directory. */
                FB_STYLE_SINGLE_LIST,
                UEFileBrowserOKCB,	/* OK callback function. */   
                UEFileBrowserCancelCB	/* Cancel callback function. */
        );
        if(status)
            return(-1);
        /* Remove write protect on file browser. */
        file_browser.options &= ~(FB_FLAG_WRITE_PROTECT);

	file_browser_src_ptr = NULL;
	file_browser_op_code = UE_FB_OP_CODE_NONE;



	/* Load universe to first unv editor at startup. */
	if(UEWIsAllocated(0) &&
           (startup_unv_file[0] != '\0')
	)
	{
            UEWDoOpen(uew[0], startup_unv_file);
	}


	return(0);
}



void UEManage()
{
	int i, n, no_uews;
	int events_handled = 0;
	time_t t;
	event_t event;
	uew_struct **uew_ptr;


	/* Update timming. */
	t = MilliTime();
	if(t < cur_millitime)
	{
	    UEResetTimmers();
	}
	cur_millitime = t;


	/* ***************************************************** */
	/* Manage GUI events. */
        while(OSWEventsPending() > 0)
        {   
            /* Reset events counter. */
            events_handled = 0;
                 
            /* Get event. */
            OSWWaitNextEvent(&event);
                 
                
            /*   Let WidgetManage() see this event.
             *   It is not important if the event is handled or not, so
             *   the return value is disgarded.
             */
            WidgetManage(&event);


	    /* Manage all universe edit windows. */
	    events_handled += UEWManageAll(&event);
            events_handled += UHWManageAll(&event);
            events_handled += WepWManageAll(&event);
            events_handled += EcoWManageAll(&event);
            events_handled += PrintWinManage(&event);
            events_handled += OptWGenManage(&event);
	    events_handled += AboutWinManage(&event);

	    events_handled += ComfWinManage(&comfwin, &event);





            /* External widgets. */
            events_handled += DialogWinManage(
                &dialog,
                &event
            );
            events_handled += FBrowserManage(
                &file_browser,
                &event
            );
	}


	/* ******************************************************* */
	/* Load isrefs as needed, update animation. */
	for(i = 0, uew_ptr = uew; i < total_uews; i++, uew_ptr++)
	{
	    if(*uew_ptr == NULL)
		continue;

	    ISRefManage((*uew_ptr)->isref, (*uew_ptr)->total_isrefs);

	    if(((*uew_ptr)->ani_next < cur_millitime) &&
               option.animate_images
	    )
	    {
		/* Record previous frame. */
		n = (*uew_ptr)->ani_frame;

		(*uew_ptr)->ani_frame++;
		if((*uew_ptr)->ani_frame >= (*uew_ptr)->ani_total_frames)
		    (*uew_ptr)->ani_frame = 0;

		/* Redraw preview window. */
		if((*uew_ptr)->map_state &&
		   (n != (*uew_ptr)->ani_frame)
		)
		{
		    UEWDraw(i, UEW_DRAW_PREVIEW);
		}

		/* Schedual next animation frame increment. */
		(*uew_ptr)->ani_next = cur_millitime +
		    (*uew_ptr)->ani_int;
	    }
	}


        /* ******************************************************* */
	/* Switch to runlevel 1 if all windows are gone. */
	for(i = 0, no_uews = 0;
            i < total_uews;
            i++
	)
	{
	    if(uew[i] != NULL)
		break;
	}
	if(i >= total_uews)
	    no_uews = 1;


	if(no_uews)
	    runlevel = 1;



	return;
}


void UEShutdown()
{
	/* Save configuration first. */
	if(fname.rc[0] != '\0')
	    RCSaveToFile(fname.rc);


	/* File browser and dialog widgets. */
        FBrowserDestroy(&file_browser);
        DialogWinDestroy(&dialog);

        /* Comfermation window. */
        ComfWinDestroy(&comfwin);

	/* General options window. */
	OptWGenDestroy();

	/* Print window. */
	PrintWinDestroy();

        /* Economy windows. */
        EcoWDeleteAll();

        /* Weapon windows. */
        WepWDeleteAll();

	/* Universe header windows. */
	UHWDeleteAll();

	/* Universe editor windows. */
	UEWDeleteAll();

	/* About window. */
	AboutWinDestroy();


	/* Images. */
        OSWDestroyPixmap(&ue_image.unvedit_icon_pm);

        OSWDestroyImage(&ue_image.error);
        OSWDestroyImage(&ue_image.info); 
	OSWDestroyImage(&ue_image.question);
	OSWDestroyImage(&ue_image.tb_copy);
	OSWDestroyImage(&ue_image.tb_economy);
	OSWDestroyImage(&ue_image.tb_new);
        OSWDestroyImage(&ue_image.tb_newobj);
        OSWDestroyImage(&ue_image.tb_open);
        OSWDestroyImage(&ue_image.tb_paste);
        OSWDestroyImage(&ue_image.tb_print);
        OSWDestroyImage(&ue_image.tb_save);
        OSWDestroyImage(&ue_image.tb_weapons);
        OSWDestroyImage(&ue_image.unvedit_icon);
        OSWDestroyImage(&ue_image.unvedit_logo);
        OSWDestroyImage(&ue_image.warning);


	/* Cursors. */
        WidgetDestroyCursor(&ue_cursor.scanner_lock);
        WidgetDestroyCursor(&ue_cursor.v_split);
        WidgetDestroyCursor(&ue_cursor.h_split);
        WidgetDestroyCursor(&ue_cursor.zoom);
        WidgetDestroyCursor(&ue_cursor.translate);
	WidgetDestroyCursor(&ue_cursor.standard);

	/* Fonts. */
	OSWUnloadFont(&ue_font.view_obj_label);
	OSWUnloadFont(&ue_font.view_label);


        /*   Free widget globals after all client allocated widgets
         *   have been deallocated.
         */
        WidgetDestroyGlobals();

        /* Disconnect from the GUI. */
        OSWGUIDisconnect();


	/* Universe objects management resources. */
	UNVShutdown();


	return;
}



int main(int argc, char *argv[])
{
	runlevel = 1;

	switch(UEInit(argc, argv))
	{
	  case 0:
	    break;

	  case -4:
            UEShutdown();
            return(0);
	    break;

	  default:
	    UEShutdown();
	    return(1);
	    break;
	}


	runlevel = 2;

        while(runlevel >= 2)
        {
            usleep(8000);

            /*   Must call WidgetManage() once per loop to let
             *   the widget set do what it needs to do.
             */
            WidgetManage(NULL);

            UEManage();
        }


        runlevel = 1;
        UEShutdown();


	return(0);
}


