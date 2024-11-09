// widgets/wfbrowser.cpp
/*
                              Widget: File Browser

	Functions:

	char *FBrowserGetChild(char *path)
	char *FBrowserGetSelectionName(fbrowser_struct *fb)

	int FBrowserHasMask(char *path)
	char *FBrowserGetPathMask(char *path)
	char *FBrowserGetJustPath(char *path)
	char *FBrowserGetFileSystemString(int fs_type)
	int FBrowserGetFileSystemType(char *fs_name)
	fb_object_struct *FBrowserGetSelObject(fbrowser_struct *fb)
	int FBrowserDoOK(fbrowser_struct *fb)
	int FBrowserApplyCVPrompt(fbrowser_struct *fb)
	int FBrowserChangeDir(
		fbrowser_struct *fb,
		char *path
	)
	int FBrowserGetDeviceListing(fbrowser_struct *fb)
	int FBrowserRefreshList(fbrowser_struct *fb)
	int FBrowserSetOpMesg(
		fbrowser_struct *fb,
		char *title,
		char *ok_btn_name
	)

	int FBrowserDevicesPUListCB(void *ptr)
	int FBrowserMountPBCB(void *ptr)
        int FBrowserUnmountPBCB(void *ptr)

	int FBrowserOKPBCB(void *ptr)
	int FBrowserCancelPBCB(void *ptr)
	int FBrowserRefreshPBCB(void *ptr)

        int FBrowserInit(
                fbrowser_struct *fb,
                int x, int y,
                unsigned int width, unsigned int height,
                char *start_dir,
                int style,
                int (*func_ok)(char *),
                int (*func_cancel)(char *)
        )
	int FBrowserResize(fbrowser_struct *fb)
	int FBrowserDraw(fbrowser_struct *fb, int amount)
	int FBrowserManage(
		fbrowser_struct *fb,
		event_t *event
	)
	void FBrowserMapCVPrompt(fbrowser_struct *fb, int mode)
	void FBrowserUnmapCVPrompt(fbrowser_struct *fb)
	void FBrowserMap(fbrowser_struct *fb)
	void FBrowserMapPath(fbrowser_struct *fb, char *path)
	void FBrowserMapSearchMask(fbrowser_struct *fb, char *pattern)
	void FBrowserUnmap(fbrowser_struct *fb)
	void FBrowserDestroy(fbrowser_struct *fb)


 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
extern "C" {
#include <fnmatch.h>
}

/* Mounting. */
#ifdef __linux__
# include <mntent.h>            /* For getmntent() */
# include <sys/mount.h>

  /*   Need to include linux/fs.h if needed declares are not present,
   *   most newer Linuxes have these declared and won't require
   *   linux/fs.h
   */
# if !defined(MS_RDONLY)
#  include <linux/fs.h>
# endif
#endif


#include "../include/cfgfmt.h"
#include "../include/disk.h"
#include "../include/string.h"
#include "../include/strexp.h"
#include "../include/widget.h"

#ifndef MAX
#define MIN(a,b)        (((a) < (b)) ? (a) : (b))
#define MAX(a,b)	(((a) > (b)) ? (a) : (b))
#endif
/* #define MIN(a,b)	((a) < (b)) ? (a) : (b) */
/* #define MAX(a,b)	((a) > (b)) ? (a) : (b) */



/*
 *	Size constants (in pixels):
 */
#define FB_MIN_WIDTH		420
#define FB_MIN_HEIGHT		320

#define FB_BTN_WIDTH		70
#define FB_BTN_HEIGHT		28

#define FB_CHAR_WIDTH		7
#define FB_CHAR_HEIGHT		14


/* Row height (in pixels) in the file browser's list windows. */
#define FB_ROW_HEIGHT		26

/* Edge margin (in pixels) for the file browser's list windows. */
#define FB_LIST_COLUM_MARGIN	10
#define FB_LIST_ROW_MARGIN	5

/* Directory window width in percent. */
#define FB_DIR_LIST_WIDTH_PERCENT	35


/*   X colum shift increment per directory level (in pixels) for
 *   the file browser's list windows.
 */
#define FB_LIST_COLUM_LEVEL_INC         26


/* Height (in pixels) for the current dirctory readout window. */
#define FB_PROMPT_HEIGHT	30


/*
 *	Change value prompt modes.
 */
#define FB_CVP_MODE_NONE	0	/* Unmapped. */
#define FB_CVP_MODE_RENAME	1
#define FB_CVP_MODE_COPY	2
#define FB_CVP_MODE_MOVE	3
#define FB_CVP_MODE_MODE	4	/* Change mode. */
#define FB_CVP_MODE_RUN		5	/* run program to view selected file. */


/*
 *	Hint messages:
 */
#define HINT_MESG_PARENT_DIR	"Go to parent directory"
#define HINT_MESG_MOUNT		"Mount selected device"
#define HINT_MESG_UNMOUNT	"Unmount selected device"


/*
 *	Drag records:
 */
namespace static_wfbrowser {
	fbrowser_struct *drag_start_fb;

	int drag_start_dir_pos;		/* Atleast one must be -1. */
	int drag_start_file_pos;

	win_t drag_start_w;     /* Which window did drag start on. */
	bool_t drag_active;

	bool_t button1_state;
	bool_t button2_state;
	bool_t button3_state;
}


/*
 *	Drag records:
 */
/*
static fbrowser_struct *drag_start_fb;

static int drag_start_dir_pos;*/		/* Atleast one must be -1. */
/* static int drag_start_file_pos;

static win_t drag_start_w;*/     /* Which window did drag start on. */
/* static bool_t drag_active;

static bool_t button1_state;
static bool_t button2_state;
static bool_t button3_state;
*/


/*
 *	Returns a statically allocated string containing only the child
 *	of the path statement path.
 */
char *FBrowserGetChild(char *path)
{
	static char rtn_str[PATH_MAX];

	strncpy(rtn_str, path, PATH_MAX);
	rtn_str[PATH_MAX - 1] = '\0';

	StripAbsolutePath(rtn_str);

	return(rtn_str);
}


/*
 *	Returns a statically allocated string containing the name
 *	of the item selected by the file browser fb.  Can return NULL
 *	if there is nothing selected.
 */
char *FBrowserGetSelectionName(fbrowser_struct *fb)
{
	int i;
        static char rtn_str[PATH_MAX + NAME_MAX];


	if(fb == NULL)
	    return(NULL);

	if(fb->style == FB_STYLE_SINGLE_LIST)
	{
	    i = fb->sel_object;
	    if(i < 0)
		return(NULL);

	    if(i < fb->dir_list_items)
	    {
                if(fb->dir_list[i] != NULL)
                {
                    strncpy(
                        rtn_str,
                        fb->dir_list[i]->name,
                        PATH_MAX + NAME_MAX   
                    );
                    rtn_str[PATH_MAX + NAME_MAX - 1] = '\0';
		    return(rtn_str);
		}
	    }
	    else
	    {
		i -= fb->dir_list_items;
		if((i >= 0) && (i < fb->file_list_items))
		{
                    if(fb->file_list[i] != NULL)
                    {
                        strncpy(
                            rtn_str,
                            fb->file_list[i]->name,
                            PATH_MAX + NAME_MAX
                        );
                        rtn_str[PATH_MAX + NAME_MAX - 1] = '\0';
                        return(rtn_str);
		    }
                }
	    }
	}
	else	/* FB_STYLE_SPLIT_LIST */
	{
	    i = fb->sel_dir;
	    if((i > -1) &&
	       (i < fb->dir_list_items)
	    )
	    {
	        if(fb->dir_list[i] != NULL)
	        {
		    strncpy(
		        rtn_str,
		        fb->dir_list[i]->name, 
		        PATH_MAX + NAME_MAX
		    );
                    rtn_str[PATH_MAX + NAME_MAX - 1] = '\0';
		    return(rtn_str);
	        }
	    }

            i = fb->sel_file;
            if((i > -1) &&
               (i < fb->file_list_items)
            )
            {
                if(fb->file_list[i] != NULL)
                {
                    strncpy(  
                        rtn_str,
                        fb->file_list[i]->name,
                        PATH_MAX + NAME_MAX
                    );
		    rtn_str[PATH_MAX + NAME_MAX - 1] = '\0';
                    return(rtn_str);
                }
            }
	}


	return(NULL);
}


/*
 *	Checks if path has mask selection characters, determining
 *	if it has a mask or not.
 */
int FBrowserHasMask(char *path)
{
	char *strptr;


	if(path == NULL)
	    return(0);

	strptr = strrchr(path, '/');
	if(strptr == NULL)
	    strptr = path;
	else
	    strptr = strptr + 1;

	while(*strptr != '\0')
	{
	    /* Following wild card characters suggest selection mask. */
	    if((*strptr == '*') ||
	       (*strptr == '?')
	    )
		return(1);

	    strptr++;
	}

	return(0);
}

/*
 *	Returns a statically allocated string containing just
 *	the search path or "*" on error or if none is specified.
 */
char *FBrowserGetPathMask(char *path)
{
	char *strptr;
	static char search_str[PATH_MAX + NAME_MAX];

	if(path == NULL)
	    return("*");

        if(!FBrowserHasMask(path))
            return("*");


	strptr = strrchr(path, '/');
	strncpy(
	    search_str,
	    ((strptr == NULL) ? path : (strptr + 1)),
	    PATH_MAX + NAME_MAX
	);
	search_str[PATH_MAX + NAME_MAX - 1] = '\0';


	return(search_str);
}

/*
 *      Returns a statically allocated string containing just
 *      the path without the search specification.
 */
char *FBrowserGetJustPath(char *path)
{
        char *strptr;
        static char just_path[PATH_MAX + NAME_MAX];


        if(path == NULL)
            return("/");

	if(!FBrowserHasMask(path))
        {
	    strncpy(
		just_path,
		path,
		PATH_MAX + NAME_MAX
	    );
	    just_path[PATH_MAX + NAME_MAX - 1] = '\0';

	    return(just_path);
	}

        strncpy(
            just_path, 
            path,
            PATH_MAX + NAME_MAX
        );
        just_path[PATH_MAX + NAME_MAX - 1] = '\0';

	strptr = strrchr(just_path, '/');
	if(strptr != NULL)
	    *strptr = '\0';


	/* Make sure we still have atleast toplevel. */
	if(just_path[0] == '\0')
	{
	    just_path[0] = '/';
	    just_path[1] = '\0';
	}

        return(just_path);
}


/*
 *	Returns a statically allocated string containing
 *	the OS specific formal name string of the file system
 *	type fs_type.
 *
 *	fs_type can be one of FB_FSTYPE_*.
 *
 *	If the type is unknown, then "unknown" is returned.
 */
char *FBrowserGetFileSystemString(int fs_type)
{
	switch(fs_type)
	{
          case FB_FSTYPE_NFS:
            return("nfs");
            break;

          case FB_FSTYPE_HPFS:
            return("hpfs");
            break;

          case FB_FSTYPE_XIAFS:
            return("xiafs");
            break;

          case FB_FSTYPE_MINIX:
            return("minix");
            break;

	  case FB_FSTYPE_ISO9660:
	    return("iso9660");
	    break;

	  case FB_FSTYPE_MSDOS:
	    return("msdos");
	    break;

          case FB_FSTYPE_PROC:
            return("proc");
            break;

          case FB_FSTYPE_EXT2:
            return("ext2");
            break;

          case FB_FSTYPE_EXT:
            return("ext");
            break;

          case FB_FSTYPE_SWAP:
            return("swap");
            break;
	}


	return("unknown");
}


/*
 *	Returns the file system type code matching the formal
 *	filesystem name fs_name or FB_FSTYPE_UNKNOWN if no match
 *	can be made.
 */
int FBrowserGetFileSystemType(char *fs_name)
{
	if(fs_name == NULL)
	    return(FB_FSTYPE_UNKNOWN);


	if(!strcmp(fs_name, "nfs"))
            return(FB_FSTYPE_NFS);
        else if(!strcmp(fs_name, "hpfs"))
            return(FB_FSTYPE_HPFS);
        else if(!strcmp(fs_name, "xiafs"))
            return(FB_FSTYPE_XIAFS);
        else if(!strcmp(fs_name, "minix"))
            return(FB_FSTYPE_MINIX);
        else if(!strcmp(fs_name, "iso9660"))
            return(FB_FSTYPE_ISO9660);
        else if(!strcmp(fs_name, "msdos"))
            return(FB_FSTYPE_MSDOS);
        else if(!strcmp(fs_name, "proc"))
            return(FB_FSTYPE_PROC);
        else if(!strcmp(fs_name, "ext2"))
            return(FB_FSTYPE_EXT2);
        else if(!strcmp(fs_name, "ext"))
            return(FB_FSTYPE_EXT);
        else if(!strcmp(fs_name, "swap"))
            return(FB_FSTYPE_SWAP);

	else
	    return(FB_FSTYPE_UNKNOWN);
}



/*
 *	Returns the pointer to the selected object or NULL if fb
 *	has no object selected.
 */
fb_object_struct *FBrowserGetSelObject(fbrowser_struct *fb) 
{
	int i;


	if(fb == NULL)
	    return(NULL);


	if(fb->style == FB_STYLE_SINGLE_LIST)
	{
	    i = fb->sel_object;

	    if((i >= 0) && (i < (fb->dir_list_items + fb->file_list_items)))
	    {
		if(i < fb->dir_list_items)
		{
		    return(fb->dir_list[i]);
		}

		i -= fb->dir_list_items;
		if((i >= 0) && (i < fb->file_list_items))
		{
		    return(fb->file_list[i]);
		}
	    }
	}
	else	/* FB_STYLE_SPLIT_LIST */
	{
	    i = fb->sel_dir;
	    if((i >= 0) && (i < fb->dir_list_items))
	    {
		return(fb->dir_list[i]);
	    }

	    i = fb->sel_file;
            if((i >= 0) && (i < fb->file_list_items))
            {
                return(fb->file_list[i]);
            }
	}


	return(NULL);
}

/*
 *	Do OK procedure.
 */
int FBrowserDoOK(fbrowser_struct *fb)
{
	char *strptr, *strptr2;
	char tmp_path[PATH_MAX + NAME_MAX];
	char search_str[PATH_MAX + NAME_MAX];
	struct stat stat_buf;


	if(fb == NULL)
	    return(-1);

	if(fb->prompt.buf == NULL)
	    return(-1);


	/* Is an item selected? */
        strptr = FBrowserGetSelectionName(fb);   
        if(strptr == NULL)
	{
	    /*   No item selected, process location prompt's contents
             *   exclusivly.
             */
	    strncpy(tmp_path, fb->prompt.buf, PATH_MAX + NAME_MAX);
        }
        else 
        {
	    /* Item selected, prefix togeather name and path. */

	    strptr2 = FBrowserGetJustPath(fb->prompt.buf);
	    if(strptr2 == NULL)
		return(-1);

	    /* Strip object name from location prompt path as needed. */
	    if(stat(strptr2, &stat_buf))
            {
		strptr2 = GetParentDir(fb->prompt.buf);
            }
	    else
	    {
		if(!S_ISDIR(stat_buf.st_mode))
		    strptr2 = GetParentDir(fb->prompt.buf);
	    }
            if(strptr2 == NULL)
                return(-1);

            strncpy(tmp_path, strptr2, PATH_MAX + NAME_MAX);
	    tmp_path[PATH_MAX + NAME_MAX - 1] = '\0';

	    strptr2 = PrefixPaths(tmp_path, strptr);
            strncpy(
                tmp_path,
		((strptr2 == NULL) ? "/" : strptr2),
                PATH_MAX + NAME_MAX
            );
	}
        tmp_path[PATH_MAX + NAME_MAX - 1] = '\0';

	/* Get just the path, no selection mask. */
	strptr = FBrowserGetJustPath(tmp_path);
	strptr2 = FBrowserGetPathMask(tmp_path);

	strncpy(
	    tmp_path,
	    ((strptr == NULL) ? "/" : strptr),
	    PATH_MAX + NAME_MAX
	);
	tmp_path[PATH_MAX + NAME_MAX - 1] = '\0';
        strncpy(
	    search_str,
	    ((strptr2 == NULL) ? "*" : strptr2),
	    PATH_MAX + NAME_MAX
	);
	search_str[PATH_MAX + NAME_MAX - 1] = '\0';


	/* See what tmp_path specifies. */
        if(ISPATHDIR(tmp_path))
        {
            /*   Selected object is a dir, change to that dir
             *   and refresh listing.
	     */
            FBrowserChangeDir(fb, tmp_path);
            FBrowserRefreshList(fb);

            /* Must redraw completely. */
	    if(fb->map_state)
	    {
                FBrowserDraw(fb, FBROWSER_DRAW_COMPLETE);
	        PromptDraw(&fb->prompt, PROMPT_DRAW_AMOUNT_TEXTONLY);
	    }
        }
        else
        {
            /*   Selected object is something that should be
             *   returned to the client function.
	     */

	    if(stat(tmp_path, &stat_buf))
	    {
		/* Object does not exist, should we accept that? */
                if(!(fb->options & FB_FLAG_MUST_EXIST))
		{
		    /* Does not have to exist, so report it. */

                    /* Unmap file browser. */
                    if(fb->options & FB_FLAG_CLOSE_ON_OK)
                        FBrowserUnmap(fb);

                    /* Call OK callback function. */
                    if(fb->func_ok != NULL)   
                        fb->func_ok(tmp_path);
		}
	    }
	    else
	    {
		/* Object exists. */

                /* Unmap file browser. */
                if(fb->options & FB_FLAG_CLOSE_ON_OK)
                    FBrowserUnmap(fb);

                /* Call OK callback function. */
                if(fb->func_ok != NULL)
                    fb->func_ok(tmp_path);
	    }
	}


	return(0);
}



/*
 *      Convience function to change a directory for a given file
 *      browser structure.
 *
 *      The call to ChangeDirRel() will be made here.  This is just
 *      to call that and adjust the prompt as needed.
 */
int FBrowserChangeDir(
        fbrowser_struct *fb,
        char *path
)
{       
        char *newpath;
	char *strptr, *strptr2;
	char cur_path[PATH_MAX + NAME_MAX];
	char search_str[PATH_MAX + NAME_MAX];


        if((fb == NULL) ||
           (path == NULL)
        )   
            return(-1);

        if(fb->prompt.buf == NULL)
            return(-1);


	/* Get path and search string. */
	strptr = FBrowserGetJustPath(fb->prompt.buf);
	strptr2 = FBrowserGetPathMask(fb->prompt.buf);

	strncpy(
	    cur_path,
	    ((strptr == NULL) ? "/" : strptr),
	    PATH_MAX + NAME_MAX
	);
	cur_path[PATH_MAX + NAME_MAX - 1] = '\0';

        strncpy(
            search_str,
            ((strptr2 == NULL) ? "*" : strptr2),
            PATH_MAX + NAME_MAX
        );
        search_str[PATH_MAX + NAME_MAX - 1] = '\0';


        /* One path information must be absolute. */
        if(!ISPATHABSOLUTE(cur_path) &&
           !ISPATHABSOLUTE(path)
        )   
            return(-1);

        /* Parse given paths and get new path. */
        newpath = ChangeDirRel(cur_path, path);
        if(newpath == NULL)
            return(-1);


	strptr = PrefixPaths(newpath, search_str);

        /* Copy newpath to location prompt buffer. */
	PromptSetS(&fb->prompt, strptr);
	PromptMarkBuffer(&fb->prompt, PROMPT_POS_END);

        /* newpath is allocated from ChangeDirRel(), free it. */
        free(newpath);
	newpath = NULL;
        
 
                
        return(0);
}



/*
 *      Apply changes from cv prompt.
 */
int FBrowserApplyCVPrompt(fbrowser_struct *fb)
{
        int i, len, status;

        char tar_path[PATH_MAX + NAME_MAX];     /* Target. */
        char src_path[PATH_MAX + NAME_MAX];     /* Source. */

	char new_name[PATH_MAX + NAME_MAX];
	char old_name[PATH_MAX + NAME_MAX];

	char *strptr;
	struct stat stat_buf;
	fb_object_struct **obj_ptr;


        if(fb == NULL)
            return(-1);
        if((fb->prompt.buf == NULL) ||
           (fb->cv_prompt.buf == NULL) ||
           (fb->cv_prompt_target == NULL)
        )
            return(-1);

	/* The cv_prompt_target must be an absolute path. */
	if(!ISPATHABSOLUTE(fb->cv_prompt_target))
	    return(-3);
	/* Location prompt must also be an absolute path. */
        if(!ISPATHABSOLUTE(fb->prompt.buf))
	    return(-3);


        /* *********************************************************** */
	StringStripSpaces(fb->cv_prompt.buf);
        status = 0;

        switch(fb->cv_prompt_mode)
        {
          /* ********************************************************* */
          case FB_CVP_MODE_RENAME:
            /* Target name must not be absolute. */
            if(ISPATHABSOLUTE(fb->cv_prompt.buf))
                return(-2);


	    /* Record old name. */
	    strptr = FBrowserGetChild(fb->cv_prompt_target);
	    strncpy(old_name, strptr, PATH_MAX + NAME_MAX);
            old_name[PATH_MAX + NAME_MAX - 1] = '\0';

	    /* Get source paths. */
	    strncpy(src_path, fb->cv_prompt_target, PATH_MAX + NAME_MAX);
	    src_path[PATH_MAX + NAME_MAX - 1] = '\0';

	    /* Get target path. */
	    strptr = GetParentDir(fb->cv_prompt_target);
	    strncpy(
		tar_path,
		(strptr == NULL) ? "/" : strptr,
		PATH_MAX + NAME_MAX
	    );
	    tar_path[PATH_MAX + NAME_MAX - 1] = '\0';

	    strptr = PrefixPaths(tar_path, fb->cv_prompt.buf);
	    if(strptr == NULL)
		return(-1);
            strncpy(tar_path, strptr, PATH_MAX + NAME_MAX);
	    tar_path[PATH_MAX + NAME_MAX - 1] = '\0';


            /* Make sure target path dosen't exist. */
	    if(!stat(tar_path, &stat_buf))
	    {
		/* Target path exists, cannot rename to it. */
		return(-2);
	    }

            /* Check if source path exists and if we have access to it. */
/*
            if(!lstat(src_path, &stat_buf))
	    {
		if(S_IWUSR(stat_buf.mode) ||
                   S_IWGRP(stat_buf.mode) ||
                   S_IWOTH(stat_buf.mode) ||
		)
		{
 */
	    status = rename(src_path, tar_path);
	    if(!status)
	    {
		/* Update name on lists. */
	        strncpy(new_name, fb->cv_prompt.buf, PATH_MAX + NAME_MAX);
	        new_name[PATH_MAX + NAME_MAX - 1] = '\0';

	        for(i = 0, obj_ptr = fb->dir_list;
                    i < fb->dir_list_items;
                    i++, obj_ptr++
	        )
	        {
		    if(*obj_ptr == NULL) continue;
		    if((*obj_ptr)->name == NULL) continue;

		    if(!strcmp((*obj_ptr)->name, old_name))
		    {
		        free((*obj_ptr)->name);
                        (*obj_ptr)->name = StringCopyAlloc(new_name);

                        len = strlen(new_name);
                        (*obj_ptr)->width = 30 + (len * FB_CHAR_WIDTH);
		    }
	        }
                for(i = 0, obj_ptr = fb->file_list;
                    i < fb->file_list_items;
                    i++, obj_ptr++
                )
                {
                    if(*obj_ptr == NULL) continue;
                    if((*obj_ptr)->name == NULL) continue;

                    if(!strcmp((*obj_ptr)->name, old_name))
                    {
                        free((*obj_ptr)->name);
                        (*obj_ptr)->name = StringCopyAlloc(new_name);

	                len = strlen(new_name);
                        (*obj_ptr)->width = 30 + (len * FB_CHAR_WIDTH);
                    }
                }
	    }

	    FBrowserUnmapCVPrompt(fb);
	    FBrowserDraw(fb, FBROWSER_DRAW_COMPLETE);

            break;

          /* ********************************************************* */ 
          default:
            break;
        }

                     
            
        return(status);
}


/*
 *	Frees previous mounted directory information on fb
 *	and fetches new listing.
 */
int FBrowserGetDeviceListing(fbrowser_struct *fb)
{
#ifdef __linux__

#ifndef _PATH_FSTAB
# define _PATH_FSTAB	"/etc/fstab"
#endif

#ifndef _PATH_MOUNTS
# define _PATH_MOUNTS	"/proc/mounts"
#endif


        int i;

	int prev_sel_item;	/* Previous selected item on pulist. */
            
        FILE *fp;
	struct mntent *me;
        fb_device_struct **dev_ptr;

          
        if(fb == NULL)
            return(-1);


	/* ********************************************************** */
        
        /* Free previous device listing. */
        for(i = 0, dev_ptr = fb->device;
            i < fb->total_devices;
            i++, dev_ptr++
        )
        {
            if(*dev_ptr == NULL) continue;

	    free((*dev_ptr)->name);        
            free((*dev_ptr)->dev);
            free((*dev_ptr)->mounted_path);

            free(*dev_ptr);
        }
        free(fb->device);
        fb->device = NULL;
        fb->total_devices = 0;


	/* Record selected item on popup list. */
	prev_sel_item = fb->devices_pulist.sel_item;

	/* Free listing in the devices popup list. */
	PUListDeleteAllItems(&fb->devices_pulist);


        /* *********************************************************** */
        /* Read _PATH_FSTAB (filesystem configuration file). */

        fp = setmntent(_PATH_FSTAB, "r");
        if(fp == NULL)
            return(-1);

	for(me = getmntent(fp); me != NULL; me = getmntent(fp))
	{
	    /* Skip if mount dir is not absolute path. */
	    if(!ISPATHABSOLUTE(me->mnt_dir))
                continue;

            /* Allocate more device pointers. */
            fb->total_devices++;
            fb->device = (fb_device_struct **)realloc(
                fb->device,
                fb->total_devices * sizeof(fb_device_struct *)
            );
            if(fb->device == NULL)
            {
                fb->total_devices = 0;
		continue;
            }

	    /* Allocate new device structure. */
            i = (int)fb->total_devices - 1;
            fb->device[i] = (fb_device_struct *)calloc(
		1,
		sizeof(fb_device_struct)
            );
            if(fb->device[i] == NULL)
            {
                fb->total_devices = i;

                continue;
            }
            fb->device[i]->fs_type = FB_FSTYPE_UNKNOWN;
            fb->device[i]->readable = False;
            fb->device[i]->writeable = False;
            fb->device[i]->mounted = False;


            /*   Name, since no name is available so use
             *   mount dir as the name.
	     */
            fb->device[i]->name = StringCopyAlloc(me->mnt_dir);

	    /* Device name. */
            fb->device[i]->dev = StringCopyAlloc(me->mnt_fsname);

            /* Set mounted directory name. */ 
            fb->device[i]->mounted_path = StringCopyAlloc(me->mnt_dir);

            /* Add mounted directory name to popup list. */
	    PUListAddItem(
		&fb->devices_pulist,
		me->mnt_dir,
		False
	    );

            /* See which file system it is. */
	    fb->device[i]->fs_type = FBrowserGetFileSystemType(me->mnt_type);

	    /* Read and write options. */
	    fb->device[i]->readable = True;
	    fb->device[i]->writeable =
		(strstr(me->mnt_opts, "ro") == NULL) ? True : False;

	}

	/* Close file. */
        endmntent(fp);


        /* *********************************************************** */
        /* Read _PATH_MOUNTS (what devices are currently mounted). */
        
        fp = setmntent(_PATH_MOUNTS, "r");
        if(fp == NULL)
            return(-1);
 
        for(me = getmntent(fp); me != NULL; me = getmntent(fp))
        {
            for(i = 0, dev_ptr = fb->device;
                i < fb->total_devices;
                i++, dev_ptr++
            )
	    {
		if(*dev_ptr == NULL) continue;
		if((*dev_ptr)->mounted_path == NULL) continue;

		if(strcmp((*dev_ptr)->mounted_path, me->mnt_dir))
		    continue;

		/* This device is mounted since it's listed. */
		(*dev_ptr)->mounted = True;

		/* Read and write. */
                (*dev_ptr)->readable =
		    (strchr(me->mnt_opts, 'r') == NULL) ? False : True;
                (*dev_ptr)->writeable =
		    (strchr(me->mnt_opts, 'w') == NULL) ? False : True;
	    }
        }

        /* Close file. */
        endmntent(fp);


        /* Restore previously selected item number on popup list. */
	fb->devices_pulist.sel_item = prev_sel_item;
             
#endif /* __linux__ */
             
            
        return(0);
}



/*
 *	Frees previous directory and files listing on fb
 *	and fetches new listing.
 */
int FBrowserRefreshList(fbrowser_struct *fb)
{
        int i, n;
        int len, longest;
        int col_pos, row_num, max_rows;
        char *strptr;
        char **dent;
        struct stat stat_buf;
        char tmp_filename[PATH_MAX + NAME_MAX];
	char tmp_path[PATH_MAX + NAME_MAX];
	char search_string[PATH_MAX + NAME_MAX];
        fb_object_struct **obj_ptr;
        win_attr_t wattr;
                  
            
        if(fb == NULL)
            return(-1);
        if(fb->prompt.buf == NULL)
            return(-1);

	/* Get just the path and search string. */
	strptr = FBrowserGetJustPath(fb->prompt.buf);
	strncpy(
	    tmp_path,
	    ((strptr == NULL) ? "/" : strptr),
	    PATH_MAX + NAME_MAX
	);
	tmp_path[PATH_MAX + NAME_MAX - 1] = '\0';

	strptr = FBrowserGetPathMask(fb->prompt.buf);
        strncpy(
	    search_string,
	    ((strptr == NULL) ? "*" : strptr),
            PATH_MAX + NAME_MAX
        );
        search_string[PATH_MAX + NAME_MAX - 1] = '\0';


        /* Location prompt must reffer to an existing directory. */
        if(!ISPATHDIR(tmp_path))
            return(-2);


        /* ********************************************************* */

        /* Free old dir listing. */
        for(i = 0, obj_ptr = fb->dir_list;
            i < (int)fb->dir_list_items;
            i++, obj_ptr++
        )
        {
            if(*obj_ptr == NULL)
		continue;
 
            free((*obj_ptr)->name);
            free(*obj_ptr);
        }
        free(fb->dir_list);
        fb->dir_list = NULL;
 
        fb->dir_list_items = 0;
        fb->sel_dir = -1;
        
        
        /* Free old file listing. */
        for(i = 0, obj_ptr = fb->file_list;
            i < fb->file_list_items;
            i++, obj_ptr++
        )   
        {
            if(*obj_ptr == NULL)
		continue;

            free((*obj_ptr)->name);
            free(*obj_ptr);
        }
        free(fb->file_list);
        fb->file_list = NULL;

        fb->file_list_items = 0;
        fb->sel_file = -1;

        fb->sel_object = -1;

            
        /* Get dir new names. */
        dent = GetDirEntNames(tmp_path);
        /* Got nothing? */
        if(dent == NULL)
            return(0);
            
            
        /* Sort dents. */
        i = 0;
        while(dent[i] != NULL)
            i++;
        StringQSort(dent, i);
        
        
        /* ******************************************************** */
        /* Add dirs to dir list and add files to file list. */
 
        i = 0;
        while(dent[i] != NULL)
        {
            /* Get full path for tmp_filename. */
            strptr = PrefixPaths(tmp_path, dent[i]);
            strncpy(
                tmp_filename,
                (strptr == NULL) ? "/" : strptr,
                PATH_MAX + NAME_MAX
            );
            tmp_filename[PATH_MAX + NAME_MAX - 1] = '\0';

            /* Get lstats (do not follow links. */
            if(lstat(tmp_filename, &stat_buf))
            {
		/* Dosen't exist?!  Okay skip it. */
                i++; continue;
            }

            /* Skip current dir ".". */
            if(!strcmp(dent[i], "."))
            {
                i++; continue;
            }
            
            /* Accept parent notation entry? */
            if(fb->style == FB_STYLE_SINGLE_LIST)
            { 
                /* In FB_STYLE_SINGLE_LIST style, always skip. */
                if(!strcmp(dent[i], ".."))
                {
                    i++; continue;
                }
            }
            else
            {
                /* Skip parent if toplevel. */
                if(!strcmp(tmp_path, "/") &&
                   !strcmp(dent[i], "..")
                )
                {
                    i++; continue;
                }
            }
                
              
            /* Check what type this entry is (follow links). */
            if(ISPATHDIR(tmp_filename))
            {
                /* Add to directory list. */  
                fb->dir_list_items += 1;
                fb->dir_list = (fb_object_struct **)realloc(
		    fb->dir_list,
                    fb->dir_list_items * sizeof(fb_object_struct *)
                );
                if(fb->dir_list == NULL)
                {
                    fb->dir_list_items = 0;
                    i++; continue;
                }
            
                /* Calculate n as current dir entry. */
                n = fb->dir_list_items - 1;
              
                fb->dir_list[n] = (fb_object_struct *)calloc(
                    1,
                    sizeof(fb_object_struct)
                );
                if(fb->dir_list[n] == NULL)
                {
                    i++; continue;
                }
                obj_ptr = &(fb->dir_list[n]); 
                
                /* Set dir entry values. */
                (*obj_ptr)->name = dent[i];
                (*obj_ptr)->mode = stat_buf.st_mode;
                (*obj_ptr)->uid = stat_buf.st_uid;
                (*obj_ptr)->gid = stat_buf.st_gid;
                (*obj_ptr)->size = stat_buf.st_size;
                (*obj_ptr)->atime = stat_buf.st_atime;
                (*obj_ptr)->ctime = stat_buf.st_ctime;
                (*obj_ptr)->mtime = stat_buf.st_mtime;
            }
            else
            {
                /* Name of file matched in search string? */
/*
		if(fnmatch(search_string, dent[i], 0))
 */
		if(fnmatch(
		    static_cast< char* >(search_string),
		    static_cast< char* >(dent[i]),
		    static_cast< int >(0)
		))
		{
		    /* Need to free this entry name. */
		    free(dent[i]);
		    dent[i] = NULL;

		    i++; continue;
		}

                /* Add to files list. */
                fb->file_list_items += 1;
                fb->file_list = (fb_object_struct **)realloc(
                    fb->file_list,
                    fb->file_list_items * sizeof(fb_object_struct *)
                );
                if(fb->file_list == NULL)  
                {
                    fb->file_list_items = 0;
                    i++; continue;
                }
                
                /* Calculate y as current file entry. */
                n = fb->file_list_items - 1;
                      
                fb->file_list[n] = (fb_object_struct *)calloc(
                    1,
                    sizeof(fb_object_struct)
                );
                if(fb->file_list[n] == NULL)
                {
                    i++; continue;
                }
                obj_ptr = &(fb->file_list[n]);
                
                /* Set file entry values. */
                (*obj_ptr)->name = dent[i];
                (*obj_ptr)->mode = stat_buf.st_mode;
                (*obj_ptr)->uid = stat_buf.st_uid;  
                (*obj_ptr)->gid = stat_buf.st_gid;
                (*obj_ptr)->size = stat_buf.st_size;  
                (*obj_ptr)->atime = stat_buf.st_atime;
                (*obj_ptr)->ctime = stat_buf.st_ctime;
                (*obj_ptr)->mtime = stat_buf.st_mtime;
            }
                
                
            i++;
        }
                    
                  
        /* Calculate positions for items (for FB_STYLE_SINGLE_LIST). */
        if(fb->style == FB_STYLE_SINGLE_LIST)
        {
            OSWGetWindowAttributes(fb->list_win, &wattr);
            max_rows = MAX(((int)wattr.height / FB_ROW_HEIGHT) - 1, 1);
                
            longest = 0;
            col_pos = 0;
            row_num = 0;
                
            /* Directories. */
            for(i = 0, obj_ptr = fb->dir_list;
                i < fb->dir_list_items;
                i++, obj_ptr++
            )
            {
                if(*obj_ptr == NULL)
		    continue;
                if((*obj_ptr)->name == NULL)
                    continue;

                /* Get name length and update longest as needed. */
                len = strlen((*obj_ptr)->name);
                if(len > longest)
                    longest = len;
                
                (*obj_ptr)->x = col_pos;
                (*obj_ptr)->y = row_num * FB_ROW_HEIGHT;
                (*obj_ptr)->width =  30 + (len * FB_CHAR_WIDTH);
                (*obj_ptr)->height = FB_ROW_HEIGHT;   
             
                row_num++;
                if(row_num >= max_rows)
                {
                    row_num = 0;
                    
                    /* Increment colum position. */
                    col_pos += 26 /* for the image. */ +
                               (longest * FB_CHAR_WIDTH) +
                               FB_LIST_COLUM_MARGIN;
            
                    longest = 0;
                }
            }
            
            /* Files and others. */
            for(i = 0, obj_ptr = fb->file_list;
                i < fb->file_list_items;
                i++, obj_ptr++
            )
            {
                if(*obj_ptr == NULL)
		    continue;
                if((*obj_ptr)->name == NULL)
                    continue;

                /* Get name length and update longest as needed. */
                len = strlen((*obj_ptr)->name);
                if(len > longest)
                    longest = len;
                
                (*obj_ptr)->x = col_pos;
                (*obj_ptr)->y = row_num * FB_ROW_HEIGHT;
                (*obj_ptr)->width =  30 + (len * FB_CHAR_WIDTH);
                (*obj_ptr)->height = FB_ROW_HEIGHT;
                
                row_num++;
                if(row_num >= max_rows)
                {
                    row_num = 0;
                 
                    /* Increment colum position. */
                    col_pos += 26 /* for the image. */ +
                               (longest * FB_CHAR_WIDTH) +
                               FB_LIST_COLUM_MARGIN;
                              
                    longest = 0;
                }
            }

            /* Recalculate list_max_width. */
            fb->list_max_width = col_pos + 26 +
                                 (longest * FB_CHAR_WIDTH) +
                                 (2 * FB_LIST_COLUM_MARGIN) + 10;
        }


        /*   Free pointers on dent, but not each item pointer since
         *   they were transfered to the fb's directory and file list so
         *   they should remain allocated.
         */     
        free(dent); dent = NULL;


	/* Reset scroll positions. */
	fb->dir_win_sb.x_win_pos = 0;
        fb->dir_win_sb.y_win_pos = 0;
        fb->file_win_sb.x_win_pos = 0;
        fb->file_win_sb.y_win_pos = 0;
        fb->list_win_sb.x_win_pos = 0;
        fb->list_win_sb.y_win_pos = 0;


        return(0);
}


/*
 *	Set operations messages on file browser fb.
 *
 *	title is the message that will appear on the file browser's
 *	title bar.
 *
 *	ok_btn_name is the label to put on the ok button.
 */
int FBrowserSetOpMesg(
	fbrowser_struct *fb,
	char *title,
	char *ok_btn_name 
)
{
	if(fb == NULL)
	    return(-1);

	if(title != NULL)
	{
	    OSWSetWindowTitle(fb->toplevel, title);
	}

	if(ok_btn_name != NULL)
	{
	    PBtnChangeLabel(
		&fb->ok_btn,
                FB_BTN_WIDTH,
                FB_BTN_HEIGHT,
		ok_btn_name,
		PBTN_TALIGN_CENTER,
		NULL
	    );
	}


	return(0);
}



/*
 *	Devices popup list select callback handler.
 */
int FBrowserDevicesPUListCB(void *ptr)
{
	int i;
	char *strptr;
	fb_device_struct *dev_ptr;
	fbrowser_struct *fb;


	if(ptr == NULL)
	    return(-1);


	/* Get pointer to file browser. */
	fb = (fbrowser_struct *)WidgetRegIsRegistered(
	    ptr, WTYPE_CODE_FILEBROWSER
	);
	if(fb == NULL)
	    return(0);


	/* Get device name as strptr from selected item on pulist. */
	strptr = PUListGetSelItemName(&fb->devices_pulist);
	if(strptr == NULL)
	    return(0);


	/* Get device pointer. */
	for(i = 0, dev_ptr = NULL; i < fb->total_devices; i++)
	{
	    if(fb->device[i] == NULL)
		continue;
	    if(fb->device[i]->mounted_path == NULL)
		continue;

	    if(!strcmp(fb->device[i]->mounted_path, strptr))
	    {
		dev_ptr = fb->device[i];
		break;
	    }
	}
	if(dev_ptr == NULL)
	{
	    /* No match. */
	    return(0);
	}


	/* ******************************************************* */
	/* Change dir. */
	FBrowserUnmapCVPrompt(fb);
	FBrowserChangeDir(fb, dev_ptr->mounted_path);
	FBrowserRefreshList(fb);

	FBrowserDraw(fb, FBROWSER_DRAW_COMPLETE);
	PromptDraw(&fb->prompt, PROMPT_DRAW_AMOUNT_COMPLETE);


	return(0);
}


/*
 *	Mount push button callback.
 */
int FBrowserMountPBCB(void *ptr)
{
        int i, status;
	char *strptr;

	char need_continue;

	char *filesystemtype = NULL;
	unsigned long rwflag = 0;

        fb_device_struct *dev_ptr;
        fbrowser_struct *fb;


	if(ptr == NULL)
	    return(-1);

	/* Get file browser pointer. */
	fb = (fbrowser_struct *)ptr;

        /* Get device name as strptr from selected item on pulist. */
        strptr = PUListGetSelItemName(&fb->devices_pulist);
        if(strptr == NULL)
            return(0);

        /* Get device pointer. */
        for(i = 0, dev_ptr = NULL; i < fb->total_devices; i++)
        {
            if(fb->device[i] == NULL)
		continue;
            if(fb->device[i]->mounted_path == NULL)
		continue;

            if(!strcmp(fb->device[i]->mounted_path, strptr))
            {
                dev_ptr = fb->device[i];
                break;
            }
        }
        if(dev_ptr == NULL)
        {
            /* No match. */
            return(0);
        }


        /* ******************************************************* */
        /* Mount device. */

#ifdef __linux__
	/* Get formal name of file system. */
	filesystemtype = FBrowserGetFileSystemString(dev_ptr->fs_type);

	/* Set magic number and mount flags. */
	rwflag = ((dev_ptr->writeable) ? 0 : MS_RDONLY) |
                 (MS_MGC_VAL);

	while(1)
	{
	    need_continue = 0;

	    status = mount(
	        /* Device. */
	        dev_ptr->dev,

	        /* Directory to mount on. */
	        dev_ptr->mounted_path,

	        /* Formal name of filesystem. */
	        filesystemtype,

	        /* Flags. */
		rwflag,

	        /* Data pointer. */
	        NULL	/* Since flags is 0, this is omitted. */
	    );
            if(status)
            {
                printf("Linux kernel: ");
	        switch(errno)
	        {
	          case EROFS:
/*
                    printf("Device \"%s\" is read-only.\n",
		        dev_ptr->dev
                    );
 */

                    /*   Set magic number and mount flags to read only
		     *   and try mounting again.
		     */
                    rwflag = (MS_RDONLY) | (MS_MGC_VAL);

		    need_continue = 1;
                    break;

	          case ENODEV:
		    printf("Filesystem \"%s\" not supported.\n",
		        filesystemtype
		    );
		    break;

	          case ENOTBLK:
                    printf("Device \"%s\" is not a block device.\n",
                        dev_ptr->dev
                    );
                    break;

                  case EBUSY:
                    printf("Device \"%s\" is already mounted or busy.\n",
                        dev_ptr->dev
                    );
                    break;

                  case EINVAL:
                    printf("Device \"%s\" is has an invalid superblock or is already mounted.\n",
                        dev_ptr->dev
                    );
                    break;

                  case EFAULT:
                    printf("Argument passed to mount() points to invalid address space.\n"
                    );
                    break;

                  case ENOMEM:
                    printf("Out of memory.\n");
                    break;

                  case ENAMETOOLONG:
                    printf("Path name is too long.\n");
                    break;

                  case ENOENT:
                    printf("Path name \"%s\" is bad or has a missing compoent\n",
		        dev_ptr->mounted_path
                    );
                    break;

                  case ENOTDIR:
                    printf("Component in \"%s\" is not a directory.\n",
		        dev_ptr->mounted_path
                    );
                    break;

                  case EACCES:
                    printf("Cannot access \"%s\", check permissions.\n",
                        dev_ptr->mounted_path
                    );
                    break;

                  case ENXIO:
                    printf("Major number of device \"%s\" is out of range (forgot to insert removeable media?).\n",
                        dev_ptr->dev
                    );
                    break;

                  case EMFILE: 
                    printf("Device \"%s\" is a dummy device and dummy device table is full.\n",
                        dev_ptr->dev
                    );
                    break;

	          default:
		    printf("Error code: `%i'\n", errno);
		    break;
	        }

		if(need_continue)
		    continue;
	    }

	    break;
        }
#endif	/* __linux__ */


        /* Refresh all listings and redraw. */
	FBrowserGetDeviceListing(fb);
	FBrowserRefreshList(fb);
	FBrowserDraw(fb, FBROWSER_DRAW_COMPLETE);

	/* Redraw devices popup list. */
	PUListDraw(&fb->devices_pulist, PULIST_DRAW_AMOUNT_COMPLETE);


	return(0);
}



/*
 *	Unmount push button callback.
 */
int FBrowserUnmountPBCB(void *ptr)   
{
        int i, status;
        char *strptr;
        fb_device_struct *dev_ptr;
        fbrowser_struct *fb;


        if(ptr == NULL)
            return(-1);

        /* Get file browser pointer. */
        fb = (fbrowser_struct *)ptr;

        /* Get device name as strptr from selected item on pulist. */
        strptr = PUListGetSelItemName(&fb->devices_pulist);
        if(strptr == NULL)
            return(0);

        /* Get device pointer. */
        for(i = 0, dev_ptr = NULL; i < fb->total_devices; i++)
        {
            if(fb->device[i] == NULL) continue;
            if(fb->device[i]->mounted_path == NULL) continue;

            if(!strcmp(fb->device[i]->mounted_path, strptr))
            {
                dev_ptr = fb->device[i];
                break;
            }
        }
        if(dev_ptr == NULL)
	{
            /* No match. */
            return(0);
        }

        /* ******************************************************* */
        /* Unmount device. */

#ifdef __linux__
	/* Do not unmount root!! */
	if(!strcmp(dev_ptr->mounted_path, "/"))
	    return(0);

        status = umount(dev_ptr->mounted_path);
	if(status &&
           (errno != EINVAL)	/* Ignore nothing mounted error. */
	)
	{
            printf("Linux kernel: ");
            switch(errno)
            {
              case EINVAL:
                printf("Nothing mounted on directory \"%s\".\n",
		    dev_ptr->mounted_path
                );
                break;

              default:
                printf("Error code: `%i'\n", errno);
                break;
	    }
	}
#endif	/* __linux__ */

        /* Refresh all listings and redraw. */
        FBrowserGetDeviceListing(fb);
        FBrowserRefreshList(fb);
        FBrowserDraw(fb, FBROWSER_DRAW_COMPLETE);

        /* Redraw devices popup list. */
        PUListDraw(&fb->devices_pulist, PULIST_DRAW_AMOUNT_COMPLETE);


	return(0);
}

/*
 *	OK button callback.
 */
int FBrowserOKPBCB(void *ptr)
{
	fbrowser_struct *fb;


	if(ptr == NULL)
	    return(-1);

	fb = (fbrowser_struct *)ptr;

	return(FBrowserDoOK(fb));
}


/*
 *	Cancel button callback.
 */
int FBrowserCancelPBCB(void *ptr)
{
        fbrowser_struct *fb;


        if(ptr == NULL)
            return(-1);

        fb = (fbrowser_struct *)ptr;


        /* Call cancel function if not NULL. */
        if(fb->func_cancel != NULL)  
            fb->func_cancel(fb->prompt.buf);

        /* Unmap the file browser. */
        if(fb->options & FB_FLAG_CLOSE_ON_CANCEL)
            FBrowserUnmap(fb);


	return(0);
}


/*
 *	Refresh button callback.
 */
int FBrowserRefreshPBCB(void *ptr)
{
        fbrowser_struct *fb;


        if(ptr == NULL)
            return(-1);

        fb = (fbrowser_struct *)ptr;


        FBrowserGetDeviceListing(fb);
        FBrowserRefreshList(fb);

        FBrowserDraw(fb, FBROWSER_DRAW_COMPLETE);
        PromptDraw(&fb->prompt, PROMPT_DRAW_AMOUNT_COMPLETE);
        PUListDraw(&fb->devices_pulist, PULIST_DRAW_AMOUNT_COMPLETE);


	return(0);
}


/*
 *	Initializes file browser widget.
 */
int FBrowserInit(
        fbrowser_struct *fb,
        int x, int y,
        unsigned int width, unsigned int height,
	char *start_dir,
	int style,
        int (*func_ok)(char *),
        int (*func_cancel)(char *)
)
{
	char cwd[PATH_MAX];
	win_attr_t wattr;


	if((osw_gui[0].root_win == 0) ||
	   (fb == NULL)
	)
	    return(-1);


	if((int)width < FB_MIN_WIDTH)
	    width = FB_MIN_WIDTH;
	if((int)width > (int)osw_gui[0].display_width)
	    width = osw_gui[0].display_width;

	if((int)height < FB_MIN_HEIGHT)
	    height = FB_MIN_HEIGHT;
	if((int)height > (int)osw_gui[0].display_height)
	    height = osw_gui[0].display_height;



	/* Reset values. */
	memset(fb, 0x00, sizeof(fbrowser_struct));

	fb->map_state = 0;
        fb->visibility_state = VisibilityFullyObscured;
        fb->is_in_focus = 0;
	fb->x = x;
	fb->y = y;
	fb->width = width;
	fb->height = height;
	fb->font = OSWQueryCurrentFont();
	fb->next = NULL;
	fb->prev = NULL;

	fb->style = style;


	/* Create toplevel. */
	if(
            OSWCreateWindow(
                &fb->toplevel,
                osw_gui[0].root_win,
                fb->x, fb->y,
                fb->width, fb->height
	    )
        )
            return(-1);
	OSWSetWindowInput(fb->toplevel, OSW_EVENTMASK_TOPLEVEL);

        /* WM properties. */
        OSWSetWindowWMProperties(
            fb->toplevel,
            "File Browser",		/* Title. */
            "File Browser",		/* Icon title. */
            widget_global.std_icon_pm,	/* Icon. */
            False,			/* Let WM set coordinates? */
            /* Coordinates. */
            fb->x, fb->y,
            /* Min width and height. */
            FB_MIN_WIDTH, FB_MIN_HEIGHT,
            /* Max width and height. */
	    osw_gui[0].display_width, osw_gui[0].display_height,
            WindowFrameStyleStandard,
            NULL, 0
        );
        OSWSetWindowBkg(fb->toplevel, 0, widget_global.std_bkg_pm);


	/* OK button. */
	if(
	    PBtnInit(
		&fb->ok_btn,
		fb->toplevel,
                10,
                (int)fb->height - (FB_BTN_HEIGHT + 10),
                FB_BTN_WIDTH, 
                FB_BTN_HEIGHT,
		"OK",
		PBTN_TALIGN_CENTER,
		NULL,
		(void *)fb,
		FBrowserOKPBCB
	    )
	)
	    return(-1);

        /* Cancel button. */
        if(
            PBtnInit(
                &fb->cancel_btn,
                fb->toplevel,
                ((int)fb->width / 2) - (FB_BTN_WIDTH / 2),
                (int)fb->height - (FB_BTN_HEIGHT + 10),
                FB_BTN_WIDTH,
		FB_BTN_HEIGHT,
                "Cancel",  
                PBTN_TALIGN_CENTER,
                NULL,
                (void *)fb,
		FBrowserCancelPBCB
            )
        )
            return(-1);

        /* Refresh button. */
        if(
            PBtnInit(
                &fb->refresh_btn,
                fb->toplevel,
                (int)fb->width - (FB_BTN_WIDTH + 10),
                (int)fb->height - (FB_BTN_HEIGHT + 10),
                FB_BTN_WIDTH,
                FB_BTN_HEIGHT,
                "Refresh",
                PBTN_TALIGN_CENTER,
                NULL,
                (void *)fb,
		FBrowserRefreshPBCB
            )
        )
            return(-1);


	/* ********************************************************* */

	/* Check style. */
	if(fb->style == FB_STYLE_SINGLE_LIST)
	{
	    /* Devices popup list. */
	    if(
		PUListInit(
		    &fb->devices_pulist,
		    fb->toplevel,
		    154, 10,
		    MAX((int)fb->width - 86 - 154, 40),
		    28,
		    7,
		    PULIST_POPUP_DOWN,
		    (void *)fb,
		    FBrowserDevicesPUListCB
		)
	    )
		return(-1);

	    /* Parent dir button. */
            if(     
                PBtnInit(
                    &fb->parent_dir_btn,
                    fb->toplevel,
                    10,
                    10,
		    28, 28,
                    (widget_global.force_mono) ?
			"<-" : NULL,
                    PBTN_TALIGN_CENTER,
		    (widget_global.force_mono) ? NULL :
                        widget_global.goto_parent_img,
                    fb,
		    NULL
                )
            )
                return(-1);
	    PBtnSetHintMessage(
		&fb->parent_dir_btn,
		HINT_MESG_PARENT_DIR
	    );


            /* Mount button. */
            if(
                PBtnInit(
                    &fb->mount_btn,
                    fb->toplevel,
                    (int)fb->width - 86 + 10,
                    10,
                    28, 28,
                    (widget_global.force_mono) ?
                        "MNT" : NULL,
                    PBTN_TALIGN_CENTER,
		    (widget_global.force_mono) ? NULL :
                        widget_global.mount_img,
                    fb,
		    FBrowserMountPBCB 
                )
            )
                return(-1);
            PBtnSetHintMessage(  
                &fb->mount_btn,
                HINT_MESG_MOUNT
            );


            /* Unmount button.. */
            if(
                PBtnInit(
                    &fb->unmount_btn,
                    fb->toplevel,
                    (int)fb->width - 86 + 48,
                    10,
                    28, 28,
                    (widget_global.force_mono) ?
                        "UMNT" : NULL,
                    PBTN_TALIGN_CENTER,
                    (widget_global.force_mono) ? NULL :
                        widget_global.unmount_img,
                    fb,
		    FBrowserUnmountPBCB
                )
            )
                return(-1);
            PBtnSetHintMessage(
                &fb->unmount_btn,
                HINT_MESG_UNMOUNT
            );

            /* List window. */
            if(
                OSWCreateWindow(
                    &fb->list_win,
                    fb->toplevel,
                    10,
                    50,
                    MAX(fb->width - 20, 2),
                    MAX(fb->height - 50 - FB_BTN_HEIGHT -
			FB_PROMPT_HEIGHT - 45, 2)
                )
            )
                return(-1);
            OSWSetWindowInput(fb->list_win, ButtonPressMask |
                ButtonReleaseMask | PointerMotionMask | ExposureMask
            );
            OSWGetWindowAttributes(fb->list_win, &wattr);

            /* Scroll bars for list window. */
            if(
                SBarInit(
                    &fb->list_win_sb, fb->list_win,
                    wattr.width, wattr.height
                )
            )
                return(-1);
	}
	else	/* Split window style. */
	{
	    /* Directory list window. */
	    if(
	        OSWCreateWindow(
                    &fb->dir_win,
                    fb->toplevel,
                    10,
                    25,
                    static_cast<unsigned int>(MAX(((double)fb->width *
			FB_DIR_LIST_WIDTH_PERCENT / 100), 2)),
		    MAX((int)fb->height - FB_PROMPT_HEIGHT -
			FB_BTN_HEIGHT - 75, 2)
                )
	    )
                return(-1);
            OSWSetWindowInput(fb->dir_win, ButtonPressMask | 
                ButtonReleaseMask | PointerMotionMask | ExposureMask
	    );
	    OSWGetWindowAttributes(fb->dir_win, &wattr);

	    /* Scroll bars for dir list window. */
	    if(
		SBarInit(
	            &fb->dir_win_sb, fb->dir_win,
	            wattr.width, wattr.height
	        )
	    )
	        return(-1);


            /* File list window. */
            if(
		OSWCreateWindow(
                    &fb->file_win,
                    fb->toplevel,
                    ((int)fb->width *
                        FB_DIR_LIST_WIDTH_PERCENT / 100) + 20,
                    25,
                    static_cast<unsigned int>(MAX(((double)fb->width *
                        (100 - FB_DIR_LIST_WIDTH_PERCENT) / 100) - 30, 2)),
                    MAX((int)fb->height - FB_PROMPT_HEIGHT -
                        FB_BTN_HEIGHT - 75, 2)
	        )
            )
                return(-1);
            OSWSetWindowInput(fb->file_win, ButtonPressMask |
                ButtonReleaseMask | PointerMotionMask | ExposureMask);
            OSWGetWindowAttributes(fb->file_win, &wattr);

            /* Scroll bars for file list window. */
	    if(
                SBarInit(
	            &fb->file_win_sb, fb->file_win,
	            wattr.width, wattr.height
		)
            )
                return(-1);
	}


	/* Location prompt. */
	if(
	    PromptInit(
	        &fb->prompt, 
	        fb->toplevel,
                10,
                (int)fb->height - FB_PROMPT_HEIGHT -
                    FB_BTN_HEIGHT - 35,
                MAX((int)fb->width - 20, 50),
                FB_PROMPT_HEIGHT,
	        PROMPT_STYLE_FLUSHED,
	        "Location:",
	        PATH_MAX + NAME_MAX,	/* Buffer length. */
	        10,				/* Number of history buffers. */
		NULL
            )
	)
	    return(-1);

	/* Change values prompt is initialized when needed. */
        fb->cv_prompt_target = NULL;
        fb->cv_prompt_mode = FB_CVP_MODE_NONE;



	/* *********************************************************** */

	/* Reset dir and file list. */
	fb->dir_list = NULL;
	fb->dir_list_items = 0;

	fb->file_list = NULL;
	fb->file_list_items = 0;

	fb->sel_dir = -1;
	fb->sel_file = -1;
	fb->sel_object = -1;

	if((start_dir != NULL) && (fb->prompt.buf != NULL))
	{
	    /* Make sure start_dir exists. */
	    if(!ISPATHDIR(start_dir))
	    {
		/* Use current working directory as start_dir. */
		getcwd(cwd, PATH_MAX);
		start_dir = cwd;
	    }

	    /* Copy starting directory to location prompt buffer. */
	    strncpy(fb->prompt.buf, start_dir, fb->prompt.buf_len);
	    fb->prompt.buf[fb->prompt.buf_len - 1] = '\0';

	    /* Must copy starting directory to history buffer as well! */
	    if(fb->prompt.total_hist_bufs > 0)
	    {
		strncpy(fb->prompt.hist_buf[0], fb->prompt.buf,
		    (int)fb->prompt.buf_len);
		fb->prompt.hist_buf[0][fb->prompt.buf_len - 1] = '\0';
	    }

	    /* Move location prompt cursor position to end of text value. */
	    fb->prompt.buf_pos = (fb->prompt.buf == NULL) ?
                0 : strlen(fb->prompt.buf);
	}
	else
	{
	    /* Start directory is NULL, use current working directory. */
	    getcwd(cwd, PATH_MAX);
	    if( (cwd != NULL) && (fb->prompt.buf != NULL) )
	    {
		strncpy(fb->prompt.buf, cwd, (int)fb->prompt.buf_len);
	    }
	    else if(fb->prompt.buf != NULL)
	    {
		strncpy(fb->prompt.buf, "/", (int)fb->prompt.buf_len);
	    }
	    fb->prompt.buf[fb->prompt.buf_len - 1] = '\0';
	}



	/* Set function callbacks. */
	fb->func_ok = func_ok;
	fb->func_cancel = func_cancel;


	/* Set defaultoptions. */
	fb->options =	FB_FLAG_WRITE_PROTECT |
			FB_FLAG_CLOSE_ON_OK |
			FB_FLAG_CLOSE_ON_CANCEL;


	/* Add this widget to the regeristry. */
	WidgetRegAdd((void *)fb, WTYPE_CODE_FILEBROWSER);


	return(0);
}

/*
 *	Resizes file browser widget.
 */
int FBrowserResize(fbrowser_struct *fb)
{
        int i, len, longest;  
        int col_pos, row_num, max_rows;

	win_attr_t wattr;
        fb_object_struct **obj_ptr;


        if(fb == NULL)
            return(-1);

	/* Get new toplevel attributes. */
	OSWGetWindowAttributes(fb->toplevel, &wattr);

	/* No change? */
	if(((int)fb->width == (int)wattr.width) &&
	   ((int)fb->height == (int)wattr.height)
	)
	    return(0);

	/* Set new values. */
	fb->x = wattr.x;
	fb->y = wattr.y;
	fb->width = wattr.width;
	fb->height = wattr.height;


	OSWDestroyPixmap(&fb->toplevel_buf);

	/* Adjust OK button. */
	OSWMoveWindow(
	    fb->ok_btn.toplevel,
	    10,
	    (int)fb->height - FB_BTN_HEIGHT - 10
	);

	/* Adjust cancel button. */
        OSWMoveWindow(
	    fb->cancel_btn.toplevel,
            ((int)fb->width / 2) - (FB_BTN_WIDTH / 2),
            (int)fb->height - FB_BTN_HEIGHT - 10
        );

        /* Adjust refresh button. */
        OSWMoveWindow(fb->refresh_btn.toplevel,
            (int)fb->width - FB_BTN_WIDTH - 10,
            (int)fb->height - FB_BTN_HEIGHT - 10
        );


	/* ********************************************************* */
        /* FB_STYLE_SINGLE_LIST */
	if(fb->style == FB_STYLE_SINGLE_LIST)
	{
	    /* Devices popup list. */
	    OSWMoveResizeWindow(fb->devices_pulist.toplevel,
                154, 10,
                MAX((int)fb->width - 86 - 154, 40),
                28
	    );
	    PUListResize(
		&fb->devices_pulist,
                MAX((int)fb->width - 86 - 154, 40),
		28,
		7
	    );

            /* Parent dir button. */
            OSWMoveWindow(fb->parent_dir_btn.toplevel,
		10,
                10
	    );

            /* Mount button. */
            OSWMoveWindow(fb->mount_btn.toplevel,
                (int)fb->width - 86 + 10,
                10
	    );

	    /* Unmount button. */
            OSWMoveWindow(fb->unmount_btn.toplevel,
                (int)fb->width - 86 + 48,
                10
	    );


	    /* List window. */
	    OSWMoveResizeWindow(
		fb->list_win,
                10,
                50,
                MAX(fb->width - 20, 2),
                MAX(fb->height - 50 - FB_BTN_HEIGHT -
                    FB_PROMPT_HEIGHT - 45, 2)
	    );
	    OSWGetWindowAttributes(fb->list_win, &wattr);
            OSWDestroyPixmap(&fb->list_win_buf);
            SBarResize(&fb->list_win_sb, wattr.width, wattr.height);

            fb->list_win_sb.x_win_pos = 0;
            fb->list_win_sb.y_win_pos = 0;


            /* Calculate positions for items (for FB_STYLE_SINGLE_LIST). */
            max_rows = MAX(((int)wattr.height / FB_ROW_HEIGHT) - 1, 1);

            longest = 0;
            col_pos = 0;
            row_num = 0;
                
            /* Directories. */
            for(i = 0, obj_ptr = fb->dir_list;
                i < fb->dir_list_items;
                i++, obj_ptr++
            )
            {       
                if(*obj_ptr == NULL) continue;
                if((*obj_ptr)->name == NULL) continue;
                
                /* Get name length and update longest as needed. */
                len = strlen((*obj_ptr)->name);   
                if(len > longest)
                    longest = len;
                
                (*obj_ptr)->x = col_pos;
                (*obj_ptr)->y = row_num * FB_ROW_HEIGHT;
                (*obj_ptr)->width =  30 + (len * FB_CHAR_WIDTH);
                (*obj_ptr)->height = FB_ROW_HEIGHT;
                
                row_num++;
                if(row_num >= max_rows)
                {
                    row_num = 0;
        
                    /* Increment colum position. */
                    col_pos += 30 + (longest * FB_CHAR_WIDTH);
        
                    longest = 0;
                }
            }
        
            /* Files and others. */
            for(i = 0, obj_ptr = fb->file_list;
                i < fb->file_list_items;
                i++, obj_ptr++
            )
            {
                if(*obj_ptr == NULL) continue;
                if((*obj_ptr)->name == NULL) continue;

                /* Get name length and update longest as needed. */
                len = strlen((*obj_ptr)->name);
                if(len > longest)
                    longest = len;
                
                (*obj_ptr)->x = col_pos;
                (*obj_ptr)->y = row_num * FB_ROW_HEIGHT;
                (*obj_ptr)->width =  30 + (len * FB_CHAR_WIDTH);
                (*obj_ptr)->height = FB_ROW_HEIGHT;
                
                row_num++;
                if(row_num >= max_rows)
                {
                    row_num = 0;
                 
                    /* Increment colum position. */
                    col_pos += 30 + (longest * FB_CHAR_WIDTH);
                    
                    longest = 0;
                }
            }

            /* Recalculate list_max_width. */
            fb->list_max_width = col_pos + 26 +
                                 (longest * FB_CHAR_WIDTH) +
                                 (2 * FB_LIST_COLUM_MARGIN) + 10;
	}
	/* ********************************************************* */
	/* FB_STYLE_SPLIT_LIST */
	else
	{
	    /* Directory list window. */
	    OSWMoveResizeWindow(
		fb->dir_win,
                10,
                25,
                static_cast<unsigned int>(MAX(((double)fb->width *
		    FB_DIR_LIST_WIDTH_PERCENT / 100), 2)),
                MAX((int)fb->height - FB_PROMPT_HEIGHT -
		    FB_BTN_HEIGHT - 75, 2)
	    );
	    OSWGetWindowAttributes(fb->dir_win, &wattr);
	    OSWDestroyPixmap(&fb->dir_win_buf);
	    SBarResize(&fb->dir_win_sb, wattr.width, wattr.height);

	    fb->dir_win_sb.x_win_pos = 0;
            fb->dir_win_sb.y_win_pos = 0;


	    /* File list window. */
	    OSWMoveResizeWindow(fb->file_win,
                ((int)fb->width * FB_DIR_LIST_WIDTH_PERCENT / 100) + 20,
                25,
                static_cast<unsigned int>(MAX(((double)fb->width *
                     (100 - FB_DIR_LIST_WIDTH_PERCENT) / 100) - 30, 2)),
                MAX((int)fb->height - FB_PROMPT_HEIGHT -
		    FB_BTN_HEIGHT - 75, 2)
	    );
            OSWGetWindowAttributes(fb->file_win, &wattr);
            OSWDestroyPixmap(&fb->file_win_buf);
            SBarResize(&fb->file_win_sb, wattr.width, wattr.height);

            fb->file_win_sb.x_win_pos = 0;
            fb->file_win_sb.y_win_pos = 0;
	}

	/* Location prompt. */
	OSWMoveResizeWindow(fb->prompt.toplevel,
            10,
            (int)fb->height - FB_PROMPT_HEIGHT - FB_BTN_HEIGHT - 35,
            MAX((int)fb->width - 20, 50),
            FB_PROMPT_HEIGHT
	);


	return(0);
}


/*
 *	Redraws file browser widget.
 */
int FBrowserDraw(fbrowser_struct *fb, int amount)
{
	int dlist_y_pos, flist_y_pos;
	int x_level;	/* Directory level from 0. */
	int flist_x_filesize_pos;
	int i, x, y, z;
	win_attr_t wattr;

	image_t *ximage;
	fb_object_struct **obj_ptr;
	char stringa[100];	/* Size and permissions string. */
        font_t *prev_font;


        if(fb == NULL)
            return(-1);


	/* Map as needed. */
	if(!fb->map_state)
	{
            /* Reget device, directory, and file listing. */
	    FBrowserGetDeviceListing(fb);
            FBrowserRefreshList(fb);


            OSWMapRaised(fb->toplevel);

	    if(fb->style == FB_STYLE_SINGLE_LIST)
	    {
                OSWMapWindow(fb->list_win);

		PUListMap(&fb->devices_pulist);
		PBtnMap(&fb->parent_dir_btn);
                PBtnMap(&fb->mount_btn);
                PBtnMap(&fb->unmount_btn);

                OSWGetWindowAttributes(fb->list_win, &wattr);
                SBarDraw(
                    &fb->list_win_sb,
                    wattr.width, 
                    wattr.height,
                    fb->list_max_width,
                    wattr.height
                );
	    }
	    else
	    {
	        OSWMapWindow(fb->dir_win);
	        OSWMapWindow(fb->file_win);

                OSWGetWindowAttributes(fb->dir_win, &wattr);
                SBarDraw(
                    &fb->dir_win_sb,
                    wattr.width,
                    wattr.height,
                    wattr.width, 
                    (FB_ROW_HEIGHT * fb->dir_list_items) + FB_ROW_HEIGHT
                );
                fb->dir_win_sb.x_win_pos = 0;
                fb->dir_win_sb.y_win_pos = 0;

                OSWGetWindowAttributes(fb->file_win, &wattr);
                SBarDraw(
                    &fb->file_win_sb,
                    wattr.width,
                    wattr.height,
                    wattr.width, 
                    (FB_ROW_HEIGHT * fb->file_list_items) + FB_ROW_HEIGHT
                );
                fb->file_win_sb.x_win_pos = 0;
                fb->file_win_sb.y_win_pos = 0;
	    }

	    PBtnMap(&fb->ok_btn);
	    PBtnMap(&fb->cancel_btn);
	    PBtnMap(&fb->refresh_btn);

	    PromptMap(&fb->prompt);


	    fb->map_state = 1;
	    fb->visibility_state = VisibilityUnobscured;

	    /* Change draw amount to complete. */
	    amount = FBROWSER_DRAW_COMPLETE;
	}

	/* ********************************************************* */

        prev_font = OSWQueryCurrentFont();
        OSWSetFont(fb->font);

	/* Toplevel. */
	if(amount == FBROWSER_DRAW_COMPLETE)
	{
            OSWGetWindowAttributes(fb->toplevel, &wattr);

	    if(fb->toplevel_buf == 0)
	    {
		if(
		    OSWCreatePixmap(
			&fb->toplevel_buf,
			wattr.width, wattr.height
		    )
		)
		    return(-1);
	    }

	    /* Draw HR's and labels. */
	    if(widget_global.force_mono)
	    {
		OSWClearPixmap(fb->toplevel_buf,
		    wattr.width, wattr.height,
		    osw_gui[0].black_pix
		);

                OSWSetFgPix(osw_gui[0].white_pix);

                OSWDrawLine(fb->toplevel_buf,
                    5,
                    (int)fb->height - FB_BTN_HEIGHT - 31,
                    (int)fb->width - 10,
                    (int)fb->height - FB_BTN_HEIGHT - 31
                );

                if(fb->style == FB_STYLE_SINGLE_LIST)
		{
                    OSWDrawString(
			fb->toplevel_buf,
                        54,
                        28,
                        "Mount points:"
                    );
		}
		else
                {
                    OSWDrawString(fb->toplevel_buf,
                        14,
                        18,
                        "Directories"
                    );
                
                    OSWDrawString(fb->toplevel_buf,
                        ((int)fb->width * 
                            FB_DIR_LIST_WIDTH_PERCENT / 100) + 23,
                        18,
                        "Files"
                    );
                }
	    }
	    else
	    {
                WidgetPutImageTile(
		    fb->toplevel_buf,
		    widget_global.std_bkg_img,
                    wattr.width, wattr.height
                );

	        OSWSetFgPix(widget_global.surface_shadow_pix);
	        OSWDrawLine(fb->toplevel_buf,
	            0,
	            (int)fb->height - FB_BTN_HEIGHT - 26,
	            fb->width,
	            (int)fb->height - FB_BTN_HEIGHT - 26
	        );
                OSWSetFgPix(widget_global.surface_highlight_pix);
                OSWDrawLine(fb->toplevel_buf,
                    0,
                    (int)fb->height - FB_BTN_HEIGHT - 25,
                    fb->width,
                    (int)fb->height - FB_BTN_HEIGHT - 25
                );

                if(fb->style == FB_STYLE_SINGLE_LIST)
                {
                    OSWSetFgPix(widget_global.normal_text_pix);
                    OSWDrawString(
                        fb->toplevel_buf,
                        54,
                        28,
                        "Mount points:"
                    );
                }
                else
		{
                    OSWSetFgPix(widget_global.normal_text_pix);
                    OSWDrawString(fb->toplevel_buf,
                        14,
                        18,
		        "Directories"
                    );

                    OSWDrawString(fb->toplevel_buf,
                        ((int)fb->width * 
                            FB_DIR_LIST_WIDTH_PERCENT / 100) + 23,
		        18,
                        "Files"
                    );
		}
	    }

            OSWPutBufferToWindow(fb->toplevel, fb->toplevel_buf);
	}

        /* ********************************************************** */
        /* Draw list (FB_STYLE_SINGLE_LIST only). */
        if(((amount == FBROWSER_DRAW_COMPLETE) ||
            (amount == FBROWSER_DRAW_DIRLIST) ||
            (amount == FBROWSER_DRAW_FILELIST) ||
            (amount == FBROWSER_DRAW_ALLLISTS)) &&
           (fb->style == FB_STYLE_SINGLE_LIST)
        )
	{
            /* Get window attributes. */
            OSWGetWindowAttributes(fb->list_win, &wattr);

            /* Recreate buffers. */
            if(fb->list_win_buf == 0)
            {
                if(OSWCreatePixmap(&fb->list_win_buf,
                    wattr.width, wattr.height)
                )
                    return(-1);
            }

            /* Clear buffer. */
            OSWSetFgPix(widget_global.surface_editable_pix);
            OSWDrawSolidRectangle(
		fb->list_win_buf,
                0, 0,
                wattr.width, wattr.height
            );


	    /* Get starting dir item number. */
            for(i = 0, obj_ptr = fb->dir_list;
                i < fb->dir_list_items;
                i++, obj_ptr++
            )
	    {
	        if(*obj_ptr == NULL) continue;

                if((int)((*obj_ptr)->x + (int)(*obj_ptr)->width +
                    FB_LIST_COLUM_MARGIN) >=
                    (int)fb->list_win_sb.x_win_pos
		)
		    break;
	    }

	    if(i < fb->dir_list_items)
	    {
	        for(obj_ptr = &(fb->dir_list[i]);
                    i < fb->dir_list_items;
                    i++, obj_ptr++
	        )
		{
		    if(*obj_ptr == NULL) continue;
		    if((*obj_ptr)->name == NULL) continue;
		    if(((*obj_ptr)->x + FB_LIST_COLUM_MARGIN -
                        fb->list_win_sb.x_win_pos) >
                        (int)wattr.width
		    )
			break;

		    /* Is this directory selected? */
		    if(fb->sel_object == i)
		    {
                        /* Selected directory entry. */
			if(widget_global.force_mono)
                            OSWSetFgPix(osw_gui[0].white_pix);
                        else
                            OSWSetFgPix(widget_global.surface_selected_pix);

                        OSWDrawSolidRectangle(
			    fb->list_win_buf,
                            (*obj_ptr)->x + FB_LIST_COLUM_MARGIN -
				fb->list_win_sb.x_win_pos + 26,
			    (*obj_ptr)->y + FB_LIST_ROW_MARGIN,
			    MAX((*obj_ptr)->width - 26, 2),
			    (*obj_ptr)->height
			);

                        if(widget_global.force_mono)
                            OSWSetFgPix(osw_gui[0].black_pix);              
                        else
		            OSWSetFgPix(widget_global.selected_text_pix);

                        if(S_ISLNK((*obj_ptr)->mode))
                            ximage = widget_global.linkicon_selected_img;
                        else
                            ximage = widget_global.diricon_selected_img;
		    }
		    else
		    {
                        if(widget_global.force_mono)
                            OSWSetFgPix(osw_gui[0].white_pix);
                        else
                            OSWSetFgPix(widget_global.editable_text_pix);

                        if(S_ISLNK((*obj_ptr)->mode))
                            ximage = widget_global.linkicon_normal_img;
                        else
                            ximage = widget_global.diricon_normal_img;
		    }
                    OSWPutImageToDrawablePos(
                        ximage,
                        (drawable_t)fb->list_win_buf,
                        (*obj_ptr)->x + FB_LIST_COLUM_MARGIN -
                            fb->list_win_sb.x_win_pos,
			(*obj_ptr)->y + FB_LIST_ROW_MARGIN
                    );
                    OSWDrawString(
			fb->list_win_buf,
                        (*obj_ptr)->x + FB_LIST_COLUM_MARGIN -
                            fb->list_win_sb.x_win_pos + 26 + 2,
                        (*obj_ptr)->y + FB_LIST_ROW_MARGIN
                            + (FB_ROW_HEIGHT / 2) + 5,
                        (*obj_ptr)->name
                    );
		}
	    }


            /* Get starting file item number. */
            for(i = 0, obj_ptr = fb->file_list;
                i < fb->file_list_items;
                i++, obj_ptr++
            )
            {
                if(*obj_ptr == NULL) continue;

                if((int)((*obj_ptr)->x + (int)(*obj_ptr)->width +
                    FB_LIST_COLUM_MARGIN) >=
                    (int)fb->list_win_sb.x_win_pos
                )
                    break;
            }

            if(i < fb->file_list_items)  
            {
                for(obj_ptr = &(fb->file_list[i]);
                    i < fb->file_list_items;
                    i++, obj_ptr++
                )
                {
                    if(*obj_ptr == NULL) continue;
                    if((*obj_ptr)->name == NULL) continue;
                    if(((*obj_ptr)->x + FB_LIST_COLUM_MARGIN - 
                        fb->list_win_sb.x_win_pos) >
                        (int)wattr.width
                    )
                        break;

		    /* Is file selected? */
                    if(fb->sel_object == (i + fb->dir_list_items))
		    {
                        /* Selected file entry. */
                        if(widget_global.force_mono) 
                            OSWSetFgPix(osw_gui[0].white_pix);
                        else
                            OSWSetFgPix(widget_global.surface_selected_pix);
                     
                        OSWDrawSolidRectangle(
                            fb->list_win_buf,
                            (*obj_ptr)->x + FB_LIST_COLUM_MARGIN -
                                fb->list_win_sb.x_win_pos + 26,
                            (*obj_ptr)->y + FB_LIST_ROW_MARGIN,
                            MAX((*obj_ptr)->width - 26, 2),
                            (*obj_ptr)->height
                        );
                            
                        if(widget_global.force_mono)
                            OSWSetFgPix(osw_gui[0].black_pix);
                        else
                            OSWSetFgPix(widget_global.selected_text_pix);

                        if(S_ISLNK((*obj_ptr)->mode))
                            ximage = widget_global.linkicon_selected_img;
                        else if((S_IXUSR & (*obj_ptr)->mode) ||
                                (S_IXGRP & (*obj_ptr)->mode) ||
                                (S_IXOTH & (*obj_ptr)->mode)
                        )
                            ximage = widget_global.execicon_selected_img;
                        else if(S_ISREG((*obj_ptr)->mode))
                            ximage = widget_global.fileicon_selected_img;
                        else if(S_ISCHR((*obj_ptr)->mode) ||
                                S_ISBLK((*obj_ptr)->mode) ||
                                S_ISFIFO((*obj_ptr)->mode) ||
                                S_ISSOCK((*obj_ptr)->mode)
                        )
                            ximage = widget_global.pipeicon_selected_img;
                        else
                            ximage = widget_global.fileicon_selected_img;
		    }
		    else
		    {
                        if(widget_global.force_mono)
                            OSWSetFgPix(osw_gui[0].white_pix);
                        else
                            OSWSetFgPix(widget_global.editable_text_pix);

                        if(S_ISLNK((*obj_ptr)->mode))
                            ximage = widget_global.linkicon_normal_img;
                        else if((S_IXUSR & (*obj_ptr)->mode) ||
                                (S_IXGRP & (*obj_ptr)->mode) ||
                                (S_IXOTH & (*obj_ptr)->mode)
                        )
                            ximage = widget_global.execicon_normal_img;
                        else if(S_ISREG((*obj_ptr)->mode))
                            ximage = widget_global.fileicon_normal_img;
                        else if(S_ISCHR((*obj_ptr)->mode) ||
                                S_ISBLK((*obj_ptr)->mode) ||
                                S_ISFIFO((*obj_ptr)->mode) ||
                                S_ISSOCK((*obj_ptr)->mode)
                        )
                            ximage = widget_global.pipeicon_normal_img;
                        else
                            ximage = widget_global.fileicon_normal_img;
		    }
                    OSWPutImageToDrawablePos(
                        ximage,
                        (drawable_t)fb->list_win_buf,
                        (*obj_ptr)->x + FB_LIST_COLUM_MARGIN -
                            fb->list_win_sb.x_win_pos,
                        (*obj_ptr)->y + FB_LIST_ROW_MARGIN
                    );
                    OSWDrawString(
                        fb->list_win_buf,
                        (*obj_ptr)->x + FB_LIST_COLUM_MARGIN -
                            fb->list_win_sb.x_win_pos + 26 + 2,
                        (*obj_ptr)->y + 5 + FB_LIST_ROW_MARGIN
                            + (FB_ROW_HEIGHT / 2),
                        (*obj_ptr)->name
                    );
                }
            }

            WidgetFrameButtonPixmap(fb->list_win_buf, True,
                wattr.width, wattr.height,
                widget_global.surface_highlight_pix,
                widget_global.surface_shadow_pix
            );

            OSWPutBufferToWindow(fb->list_win, fb->list_win_buf);
	}

	/* ********************************************************** */
	/* Draw directory list. */
	if(((amount == FBROWSER_DRAW_COMPLETE) ||
            (amount == FBROWSER_DRAW_DIRLIST) ||   
            (amount == FBROWSER_DRAW_ALLLISTS)) &&
           (fb->style == FB_STYLE_SPLIT_LIST)
	)
	{
	    /* Get window attributes. */
	    OSWGetWindowAttributes(fb->dir_win, &wattr);

            /* Recreate buffers. */ 
            if(fb->dir_win_buf == 0)
            {
                if(OSWCreatePixmap(&fb->dir_win_buf,
                    wattr.width, wattr.height)
                )
                    return(-1);
            }


	    /* Clear buffer. */
	    OSWSetFgPix(widget_global.surface_editable_pix);
	    OSWDrawSolidRectangle(fb->dir_win_buf,
	        0, 0,
	        wattr.width, wattr.height
	    );


	    if((fb->dir_list != NULL) && (fb->dir_list_items > 0))
	    {
	        dlist_y_pos = MAX(FB_ROW_HEIGHT - 4, 0);

	        /* Calculate starting row x. */
	        x = fb->dir_win_sb.y_win_pos / FB_ROW_HEIGHT;
	        if(x < 0) x = 0;

	        /* Calculate total possible rows z visable on window. */
	        z = static_cast<int>(((double)wattr.height / FB_ROW_HEIGHT) + 1);

	        y = 0;
	        x_level = 0; /* Always 0 for now, since no dir tree support. */
	        while(x < fb->dir_list_items)
	        {
		    if(y >= z)
		        break;

		    if(fb->dir_list[x] == NULL)
		    {
		        /* Skip unallocated dir entries. */
		        x++; continue;
		    }
		    if(fb->dir_list[x]->name == NULL)
		    {
                        /* Skip unallocated dir entries. */
                        x++; continue;
		    }

                    if(fb->sel_dir == x)
		    {
		        /* Selected directory entry. */
                        OSWSetFgPix(widget_global.surface_selected_pix);
                        OSWDrawSolidRectangle(fb->dir_win_buf,
                            (x_level * FB_LIST_COLUM_LEVEL_INC) +
			    FB_LIST_COLUM_LEVEL_INC + FB_LIST_COLUM_MARGIN
                            + 6,
                            dlist_y_pos - 12,
                            MAX(
			        (((int)strlen(fb->dir_list[x]->name) * 7) + 3),
			        7
			    ),
                            MAX(FB_ROW_HEIGHT - 2, 1)
                        );

		        /* See which ximage to use. */
		        if(S_ISLNK(fb->dir_list[x]->mode))
			    ximage = widget_global.linkicon_selected_img;
		        else
			    ximage = widget_global.diricon_selected_img;

                        OSWSetFgPix(widget_global.selected_text_pix);
		    }
                    else
		    {
                        /* See which ximage to use. */
                        if(S_ISLNK(fb->dir_list[x]->mode))
                            ximage = widget_global.linkicon_normal_img;
                        else
                            ximage = widget_global.diricon_normal_img;

		        /* Normal directory entry. */
                        OSWSetFgPix(widget_global.editable_text_pix);
		    }

		    OSWPutImageToDrawablePos(
		        ximage,
		        (drawable_t)fb->dir_win_buf,
		        (x_level * FB_LIST_COLUM_LEVEL_INC) +
                            FB_LIST_COLUM_MARGIN,
		        dlist_y_pos - 13
		    );

		    OSWDrawString(fb->dir_win_buf,
                        (x_level * FB_LIST_COLUM_LEVEL_INC) +
                        FB_LIST_COLUM_LEVEL_INC + FB_LIST_COLUM_MARGIN + 6,
		        dlist_y_pos + 4,
		        fb->dir_list[x]->name
		    );

		    dlist_y_pos += FB_ROW_HEIGHT;
		    x++;
		    y++;
	        }
	    }
	    WidgetFrameButtonPixmap(fb->dir_win_buf, True,
	        wattr.width, wattr.height,
	        widget_global.surface_highlight_pix,
	        widget_global.surface_shadow_pix
	    );

	    OSWPutBufferToWindow(fb->dir_win, fb->dir_win_buf);
	}


	/* ********************************************************** */
        /* Draw file list. */
        if(((amount == FBROWSER_DRAW_COMPLETE) ||
            (amount == FBROWSER_DRAW_FILELIST) ||
            (amount == FBROWSER_DRAW_ALLLISTS)) &&
           (fb->style == FB_STYLE_SPLIT_LIST)
        )
	{
            /* Get window attributes. */
            OSWGetWindowAttributes(fb->file_win, &wattr);

 	    /* Recreate buffers. */           
            if(fb->file_win_buf == 0)
            {
                if(OSWCreatePixmap(&fb->file_win_buf,
                    wattr.width, wattr.height)
                )
                    return(-1);
            }

 
	    /* Calculate flist_x_filesize_pos. */
	    flist_x_filesize_pos = 0;
	    for(x = 0; x < (int)fb->file_list_items; x++)
	    {
	        if(fb->file_list[x] == NULL)
		    continue;
	        if(fb->file_list[x]->name == NULL)
		    continue;

	        y = (strlen(fb->file_list[x]->name) * 8) + 50;
	        if(y > flist_x_filesize_pos)
		    flist_x_filesize_pos = y;
	    }
 
            /* Clear buffer. */
            OSWSetFgPix(widget_global.surface_editable_pix);
            OSWDrawSolidRectangle(fb->file_win_buf,
                0, 0,
                wattr.width, wattr.height
            );

            if((fb->file_list != NULL) && (fb->file_list_items > 0))
            {
                flist_y_pos = MAX(FB_ROW_HEIGHT - 4, 0);

	        /* Calculate starting row x. */
                x = fb->file_win_sb.y_win_pos / FB_ROW_HEIGHT;
                if(x < 0) x = 0;  

                /* Calculate total possible rows y visable on window. */
                z = static_cast<int>(((double)wattr.height / FB_ROW_HEIGHT) + 1);

	        y = 0;
                while(x < fb->file_list_items)
                {
                    if(y >= z)
                        break;

                    if(fb->file_list[x] == NULL)
		    {
		        /* Skip unallocated file entries. */
                        x++; continue;
	            }
                    if(fb->file_list[x]->name == NULL)
                    {
                        /* Skip unallocated file entries. */
                        x++; continue;
                    }

		    if(fb->sel_file == x)
		    {
		        /* Selected file entry. */
		        OSWSetFgPix(widget_global.surface_selected_pix);

		        OSWDrawSolidRectangle(fb->file_win_buf,
                            FB_LIST_COLUM_LEVEL_INC + FB_LIST_COLUM_MARGIN
                            + 6,
			    flist_y_pos - 12,
			    MAX(
                                (((int)strlen(fb->file_list[x]->name) * 7) + 3),
                                7
                            ),
			    MAX(FB_ROW_HEIGHT - 2, 1)
		        );

                        /* Choose which ximage to use depending on mode type. */
                        if(S_ISLNK(fb->file_list[x]->mode))
                            ximage = widget_global.linkicon_selected_img;
                        else if((S_IXUSR & fb->file_list[x]->mode) ||
                                (S_IXGRP & fb->file_list[x]->mode) ||
                                (S_IXOTH & fb->file_list[x]->mode)
                        )
                            ximage = widget_global.execicon_selected_img;
                        else if(S_ISREG(fb->file_list[x]->mode))
                            ximage = widget_global.fileicon_selected_img;
                        else if(S_ISCHR(fb->file_list[x]->mode) ||
                                S_ISBLK(fb->file_list[x]->mode) ||
                                S_ISFIFO(fb->file_list[x]->mode) ||
                                S_ISSOCK(fb->file_list[x]->mode)
                        )
                            ximage = widget_global.pipeicon_selected_img;
		        else
                            ximage = widget_global.fileicon_selected_img;

		        OSWSetFgPix(widget_global.selected_text_pix);
		    }
		    else
		    {
                        /* Choose which ximage to use depending on mode type. */
                        if(S_ISLNK(fb->file_list[x]->mode))
                            ximage = widget_global.linkicon_normal_img;
                        else if((S_IXUSR & fb->file_list[x]->mode) ||
                                (S_IXGRP & fb->file_list[x]->mode) ||
                                (S_IXOTH & fb->file_list[x]->mode)
                        )
                            ximage = widget_global.execicon_normal_img;
                        else if(S_ISREG(fb->file_list[x]->mode))
                            ximage = widget_global.fileicon_normal_img;
		        else if(S_ISCHR(fb->file_list[x]->mode) ||
                                S_ISBLK(fb->file_list[x]->mode) ||
                                S_ISFIFO(fb->file_list[x]->mode) ||
                                S_ISSOCK(fb->file_list[x]->mode)
		        )
                            ximage = widget_global.pipeicon_normal_img;
		        else
                            ximage = widget_global.fileicon_normal_img;

		        /* Normal file entry. */
		        OSWSetFgPix(widget_global.editable_text_pix);
		    }

		    /* Put icon. */
		    OSWPutImageToDrawablePos(
		        ximage,
		        (drawable_t)fb->file_win_buf,
		        FB_LIST_COLUM_MARGIN,
		        flist_y_pos - 13
		    );

		    /* Draw name. */
		    OSWDrawString(fb->file_win_buf,
                        FB_LIST_COLUM_LEVEL_INC + FB_LIST_COLUM_MARGIN + 6,
                        flist_y_pos + 4,
                        fb->file_list[x]->name
                    );

		    /* Draw filesize and permissions. */
		    sprintf(stringa, "%i", (int)fb->file_list[x]->size);
		    StringTailSpaces(stringa, 14);
		    stringa[14] = (fb->file_list[x]->mode & S_IRUSR) ? 'r' : '-';
                    stringa[15] = (fb->file_list[x]->mode & S_IWUSR) ? 'w' : '-';
                    stringa[16] = (fb->file_list[x]->mode & S_IXUSR) ? 'x' : '-';
                    stringa[17] = (fb->file_list[x]->mode & S_IRGRP) ? 'r' : '-';
                    stringa[18] = (fb->file_list[x]->mode & S_IWGRP) ? 'w' : '-';
                    stringa[19] = (fb->file_list[x]->mode & S_IXGRP) ? 'x' : '-';
                    stringa[20] = (fb->file_list[x]->mode & S_IROTH) ? 'r' : '-';
                    stringa[21] = (fb->file_list[x]->mode & S_IWOTH) ? 'w' : '-';
                    stringa[22] = (fb->file_list[x]->mode & S_IXOTH) ? 'x' : '-';
                    stringa[23] = '\0';

		    OSWSetFgPix(widget_global.editable_text_pix);
		    OSWDrawString(fb->file_win_buf,
		        flist_x_filesize_pos,
		        flist_y_pos + 4,
		        stringa
		    );

                    flist_y_pos += FB_ROW_HEIGHT;
                    x++;
	            y++;
                }
            }

            WidgetFrameButtonPixmap(fb->file_win_buf, True,
                wattr.width, wattr.height,
                widget_global.surface_highlight_pix,
                widget_global.surface_shadow_pix
            );

	    OSWPutBufferToWindow(fb->file_win, fb->file_win_buf);
	}

        /* ********************************************************** */
        /* Draw scroll bars. */
        if((amount == FBROWSER_DRAW_COMPLETE) ||
	   (amount == FBROWSER_DRAW_SCROLLBARS)
	)
	{
	    if(fb->style == FB_STYLE_SINGLE_LIST)
	    {
                OSWGetWindowAttributes(fb->list_win, &wattr);
                SBarDraw(
                    &fb->list_win_sb,
                    wattr.width,
                    wattr.height,
                    fb->list_max_width,
                    wattr.height
                );
	    }
	    else
	    {
                OSWGetWindowAttributes(fb->dir_win, &wattr);
                SBarDraw(
                    &fb->dir_win_sb,  
                    wattr.width,
                    wattr.height,
                    wattr.width,  
                    (FB_ROW_HEIGHT * fb->dir_list_items) + FB_ROW_HEIGHT
                );
                OSWGetWindowAttributes(fb->file_win, &wattr);
                SBarDraw(
                    &fb->file_win_sb,
                    wattr.width,
                    wattr.height,
                    wattr.width,
                    (FB_ROW_HEIGHT * fb->file_list_items) + FB_ROW_HEIGHT
                );
	    }
	}

        OSWSetFont(prev_font);


	return(0);
}

/*
 *	Manages file browser widget.
 */
int FBrowserManage(
        fbrowser_struct *fb,   
        event_t *event
)
{
	int i, x, y, z;
	char *strptr;
	char tmp_path[PATH_MAX + NAME_MAX];
	win_attr_t wattr;
	fb_object_struct **obj_ptr;
        int events_handled = 0;

        static long last_click;


	if((event == NULL) ||
	   (fb == NULL)
	)
	    return(events_handled);


	if(!fb->map_state &&
           (event->type != MapNotify)
	)
	    return(events_handled);


	switch(event->type)	
	{
          /* ********************************************************* */
	  case KeyPress:
            /* Do not handle further if out of focus. */
            if(!fb->is_in_focus)
                return(events_handled);


            /* Enter key. */
            if((event->xkey.keycode == osw_keycode.enter) ||
               (event->xkey.keycode == osw_keycode.np_enter)
            )
            {
                if(fb->cv_prompt_mode == FB_CVP_MODE_NONE) 
		{
                    /* Let location prompt handle this event as well. */
                    events_handled += PromptManage(
                        &fb->prompt,
                        event
                    );

                    /* Prompt buffer will be cleared, put back previous. */
                    if((fb->prompt.total_hist_bufs > 0) &&
                       (fb->prompt.buf != NULL)
                    )
                    {
                        strncpy(
                            fb->prompt.buf,
                            fb->prompt.hist_buf[0],
                            fb->prompt.buf_len
                        );
                        fb->prompt.buf[fb->prompt.buf_len - 1] = '\0';

                        PromptMarkBuffer(&fb->prompt, PROMPT_POS_END);
                    }
                
                    /* Redraw prompt completely (again) as needed. */
                    if(fb->map_state)
                        PromptDraw(&fb->prompt, PROMPT_DRAW_AMOUNT_TEXTONLY);
		}

                events_handled++;
                return(events_handled);
            }
            /* Escape key. */
            else if(event->xkey.keycode == osw_keycode.esc)
            {
                events_handled++;
                return(events_handled);
	    }
	    break;

          /* ********************************************************* */
          case KeyRelease:
            /* Do not handle further if out of focus. */
            if(!fb->is_in_focus)
                return(events_handled);

	    /* Enter key. */
	    if((event->xkey.keycode == osw_keycode.enter) ||
               (event->xkey.keycode == osw_keycode.np_enter)
	    )
	    {
		/* In change values mode? */
		if(fb->cv_prompt_mode != FB_CVP_MODE_NONE)
		{
		    /* Do apply change values routine. */
		    FBrowserApplyCVPrompt(fb);
		}
		/* Do OK routine. */
		else
		{
		    FBrowserDoOK(fb);
                    fb->prompt.is_in_focus = 1;
		}

                events_handled++;
                return(events_handled);
	    }
	    /* Escape key. */
	    else if(event->xkey.keycode == osw_keycode.esc)
	    {
		FBrowserCancelPBCB(fb);

                events_handled++;
                return(events_handled);
	    }
	    break;

          /* ********************************************************* */
	  case ButtonPress:
	    /* Bring into focus? */
	    if((event->xany.window == fb->toplevel) ||
               (event->xany.window == fb->dir_win) ||
               (event->xany.window == fb->file_win) ||
               (event->xany.window == fb->list_win)
	    )
	    {
		fb->is_in_focus = 1;

		/* Unfocus scroll bars, right one will be focused below. */
                fb->dir_win_sb.is_in_focus = 0;
                fb->file_win_sb.is_in_focus = 0;
                fb->list_win_sb.is_in_focus = 0;

		fb->prompt.is_in_focus = 0;
		fb->devices_pulist.is_in_focus = 0;

		FBrowserUnmapCVPrompt(fb);
	    }


            /* Change button states. */
            if(event->xbutton.button == Button1)
                static_wfbrowser::button1_state = True;
            if(event->xbutton.button == Button2)
                static_wfbrowser::button2_state = True;
            if(event->xbutton.button == Button3)
                static_wfbrowser::button3_state = True;


	    /* List window (for FB_STYLE_SINGLE_LIST). */
	    if(event->xany.window == fb->list_win)
	    {
                /* Bring its scroll bar into focus. */
                fb->list_win_sb.is_in_focus = 1;

		/* Record previous sel_object z. */
		z = fb->sel_object;

		/* Select object. */
		x = event->xbutton.x - FB_LIST_COLUM_MARGIN +
		    fb->list_win_sb.x_win_pos;
		y = event->xbutton.y - FB_LIST_ROW_MARGIN;
		fb->sel_object = -1;

		/* Go through dirs. */
		for(i = 0, obj_ptr = fb->dir_list;
                    i < fb->dir_list_items;
                    i++, obj_ptr++
		)
		{
		    if(*obj_ptr == NULL) continue;

                    if((x >= (*obj_ptr)->x) &&
                       (y >= (*obj_ptr)->y) &&
                       (x < ((*obj_ptr)->x + (int)(*obj_ptr)->width)) &&
                       (y < ((*obj_ptr)->y + (int)(*obj_ptr)->height))
                    )
                    {
		        fb->sel_object = i;
		        break;
		    }
		}
		/* Go through files. */
		if(fb->sel_object == -1)
		{
                    for(i = 0, obj_ptr = fb->file_list;
                        i < fb->file_list_items;
                        i++, obj_ptr++
		    )
                    {
                        if(*obj_ptr == NULL) continue;

                        if((x >= (*obj_ptr)->x) &&
                           (y >= (*obj_ptr)->y) &&
                           (x < ((*obj_ptr)->x + (int)(*obj_ptr)->width)) &&
                           (y < ((*obj_ptr)->y + (int)(*obj_ptr)->height))
			)
			{
			    /* Remember to offset by dir_list_items. */
                            fb->sel_object = i + fb->dir_list_items;
                            break;
			}
                    }
		}
		if(fb->sel_object >= (fb->file_list_items + fb->dir_list_items))
		    fb->sel_object = -1;

		/* Button2 makes quick select. */
		if(event->xbutton.button == Button2)
		{
                    FBrowserDoOK(fb);

                    /* Reset last_click (just for safe measure). */
                    last_click = 0;

		    events_handled++;
                    return(events_handled);
		}

                /* Double clicked? */
                if((fb->sel_object >= 0) &&
                   (event->xbutton.button == Button1) &&
                   ((last_click + widget_global.double_click_int) >= MilliTime())
                )
                {
		    /* Do `ok' procedure. */
                    if((fb->sel_object == z) &&
                       (fb->sel_object >= 0)
		    )
		    {
                        FBrowserDoOK(fb);

                        /* Reset last_click. */
                        last_click = 0;

                        events_handled++;  
                        return(events_handled);
		    }
		    else
                    {
                        /* Record last_click. */
                        last_click = MilliTime();
                    }
                }
                /* Slow double clicked? */
                else if((fb->sel_object >= 0) &&
                        (event->xbutton.button == Button1) &&
                        ((last_click + widget_global.relabel_item_delay) >= MilliTime())
                )
                {
                    /* Do rename procedure. */
                    if((fb->sel_object == z) &&
                       (fb->sel_object >= 0)
                    )
                    {
                        FBrowserMapCVPrompt(fb, FB_CVP_MODE_RENAME);

                        /* Reset last_click. */
                        last_click = 0;
                    }
                    else
                    {
                        /* Record last_click. */
                        last_click = MilliTime();
                    }
                }
                else
                {
                    /* Record last click. */
                    last_click = MilliTime();
                }
                events_handled++; 
	    }
	    /* Directory list window. */
	    else if(event->xany.window == fb->dir_win)
	    {
                /* Bring its scroll bar into focus. */
                fb->dir_win_sb.is_in_focus = 1;

		/* Select dir. */
		fb->sel_dir =
		    ( (int)fb->dir_win_sb.y_win_pos / FB_ROW_HEIGHT )
		    +
		    ( ((int)event->xbutton.y - 4) / FB_ROW_HEIGHT )
		;

		/* Sanitize. */
		if(fb->sel_dir < 0)
		    fb->sel_dir = -1;
		else if(fb->sel_dir >= (int)fb->dir_list_items)
		    fb->sel_dir = -1;

		/* Must unselect file. */
		fb->sel_file = -1;


		/* Button3 maps change value prompt. */
		if(event->xbutton.button == Button3)
		{
		    if(osw_gui[0].shift_key_state)
                        FBrowserMapCVPrompt(fb, FB_CVP_MODE_MODE);
		    else
		        FBrowserMapCVPrompt(fb, FB_CVP_MODE_RENAME);
		}


		/* Button1 double click. */
		if((fb->sel_dir > -1) &&
		   (event->xbutton.button == Button1) &&
                   ((last_click + widget_global.double_click_int) > MilliTime())
		)
		{
                    FBrowserDoOK(fb);

		    /* Reset last_click. */
		    last_click = 0;

                    events_handled++;
                    return(events_handled);
		}
		else
		{
		    /* Record last click. */
		    last_click = MilliTime();
		}

                events_handled++;
	    }
	    /* File list window. */
            else if(event->xany.window == fb->file_win)
            {
                /* Bring its scroll bar into focus. */
		fb->file_win_sb.is_in_focus = 1;

		/* Select file. */
                fb->sel_file =
                    ( (int)fb->file_win_sb.y_win_pos / FB_ROW_HEIGHT )
                    +
                    ( ((int)event->xbutton.y - 4) / FB_ROW_HEIGHT )
                ;

		/* Sanitize. */
                if(fb->sel_file < 0)
                    fb->sel_file = -1;
                else if(fb->sel_file >= (int)fb->file_list_items) 
                    fb->sel_file = -1;

		/* Must unselect directory. */
		fb->sel_dir = -1;


                /* Button3 maps change value prompt to rename. */
                if(event->xbutton.button == Button3)
                {
                    if(osw_gui[0].shift_key_state)
                        FBrowserMapCVPrompt(fb, FB_CVP_MODE_MODE);
                    else
                        FBrowserMapCVPrompt(fb, FB_CVP_MODE_RENAME);
                }


                /* Double clicked on a file entry? */
                if((fb->sel_file >= 0) &&
		   (event->xbutton.button == Button1) &&
                   ((last_click + widget_global.double_click_int) > MilliTime())
                )
                {
                    FBrowserDoOK(fb);

                    /* Reset last_click. */
                    last_click = 0;

		    events_handled++;
                    return(events_handled);
                }
                else
                {
                    /* Record last click. */
                    last_click = MilliTime();
                }
                events_handled++;
            }
	    break;

          /* ********************************************************* */
	  case ButtonRelease:
            /* Do not manage any button presses if in cv prompt mode. */
            if(fb->cv_prompt_mode != FB_CVP_MODE_NONE)
                break;

            /* Change button states. */
            if(event->xbutton.button == Button1)
                static_wfbrowser::button1_state = False;
            if(event->xbutton.button == Button2)
                static_wfbrowser::button2_state = False;
            if(event->xbutton.button == Button3)
                static_wfbrowser::button3_state = False;


            /* Dir list window. */
            if(event->xany.window == fb->dir_win)
            {
                if( static_wfbrowser::drag_active &&
                    ((int)fb->dir_list_items > 0)
                )
                {


		}
		events_handled++;
            }
	    /* File list window. */
	    else if(event->xany.window == fb->file_win)
	    {
                if( static_wfbrowser::drag_active &&
                    ((int)fb->file_list_items > 0)
                )
                {


                }
		events_handled++;
	    }


	    /* Must reset drag information after a ButtonRelease. */

	    /* Change back cursor shape. */
            OSWSetWindowCursor(static_wfbrowser::drag_start_w, osw_gui[0].std_cursor);

	    static_wfbrowser::drag_start_fb = NULL;
	    static_wfbrowser::drag_start_dir_pos = -1;
	    static_wfbrowser::drag_start_file_pos = -1;

	    static_wfbrowser::drag_start_w = 0;
	    static_wfbrowser::drag_active = False;

	    break;


          /* ********************************************************* */
	  case MotionNotify:
            /* Do not manage any button presses if in cv prompt mode. */
            if(fb->cv_prompt_mode != FB_CVP_MODE_NONE)
                break;

	    /* Drag start on directory list? */
	    if((static_wfbrowser::button1_state) &&
               (event->xany.window == fb->dir_win) &&
               ((int)fb->dir_list_items > 0)
            )
            {
                if(!static_wfbrowser::drag_active)
                {
                    /* Set up drag information. */
/*
		    static_wfbrowser::drag_start_fb = fb;
                    static_wfbrowser::drag_start_dir_pos = fb->sel_dir;
		    static_wfbrowser::drag_start_file_pos = -1;
                    static_wfbrowser::drag_start_w = event->xany.window;
                    static_wfbrowser::drag_active = True;
*/

                    /* Change cursor. */
/*
		    WidgetSetWindowCursor(static_wfbrowser::drag_start_w,
			widget_global.drag_file_wcr);
 */
                }
		events_handled++;
		return(events_handled);
	    }
            /* Drag start on file list? */
            else if( (static_wfbrowser::button1_state) &&
                     (event->xany.window == fb->file_win) &&
                     ((int)fb->file_list_items > 0)
            )
            {
                if(!static_wfbrowser::drag_active)
                {
                    /* Set up drag information. */
/*
                    static_wfbrowser::drag_start_fb = fb;
                    static_wfbrowser::drag_start_dir_pos = -1;
                    static_wfbrowser::drag_start_file_pos = fb->sel_file;
                    static_wfbrowser::drag_start_w = event->xany.window;
                    static_wfbrowser::drag_active = True;
 */
                    /* Change cursor. */
/*
                    WidgetSetWindowCursor(static_wfbrowser::drag_start_w, 
                        widget_global.drag_file_wcr);
 */
                }
                events_handled++;
                return(events_handled);
            }    



	    break;

          /* ********************************************************* */
          case Expose:
            if((event->xany.window == fb->toplevel) ||
               (event->xany.window == fb->dir_win) ||
               (event->xany.window == fb->file_win) ||
               (event->xany.window == fb->list_win)
            )
                events_handled++;

            break;

          /* ******************************************************** */
          case UnmapNotify:
            if(event->xany.window == fb->toplevel)
            {
                FBrowserUnmap(fb);

                events_handled++;
                return(events_handled);
            }
            break;

          /* ******************************************************** */
          case MapNotify:
            if(event->xany.window == fb->toplevel)
            {
                if(!fb->map_state)
                    FBrowserMap(fb);

                events_handled++;
                return(events_handled);
            }
            break;

          /* ********************************************************* */
	  case ConfigureNotify:
            if(event->xany.window == fb->toplevel)
            {
		FBrowserResize(fb);

		/* Must redraw the scroll bars. */
                FBrowserDraw(fb, FBROWSER_DRAW_SCROLLBARS);

		/* Must redraw prompt COMPLETELY (set it as unmapped) */
		PromptMap(&fb->prompt);

                events_handled++;
            }
	    break;

          /* ********************************************************* */
	  case ClientMessage:
	    if(OSWIsEventDestroyWindow(fb->toplevel, event))
	    {
		/* Call cancel function. */
		if(fb->func_cancel != NULL)
		    fb->func_cancel(fb->prompt.buf);

		events_handled++;

		/* Unmap the file browser. */
		if(fb->options & FB_FLAG_CLOSE_ON_CANCEL)
		{
		    FBrowserUnmap(fb);
		    return(events_handled);
		}
	    }
	    break;

	  /* ********************************************************* */
	  case FocusIn:
	    if(event->xany.window == fb->toplevel)
	    {
		fb->is_in_focus = 1;

		events_handled++;
		return(events_handled);
	    }
	    break;

          /* ********************************************************* */
          case FocusOut:
            if(event->xany.window == fb->toplevel)
            {
                fb->is_in_focus = 0;

                events_handled++;
		return(events_handled);
            }
            break;

          /* ********************************************************* */
          case VisibilityNotify:
            if(event->xany.window == fb->toplevel)
            {
                fb->visibility_state = event->xvisibility.state;

                events_handled++;
                return(events_handled);
            }
            break;
	}


        if(events_handled > 0)   
        {
            FBrowserDraw(fb, FBROWSER_DRAW_COMPLETE);
        }


        /* ************************************************* */

	/* Change value prompt. */
	if((events_handled == 0) &&
           (fb->cv_prompt_mode != FB_CVP_MODE_NONE)
	)
	{
            events_handled += PromptManage(
                &fb->cv_prompt,
                event
            );
	}

	/* Location prompt. */
	if(events_handled == 0)
	{
	    strptr = PromptGetS(&fb->prompt);
	    strncpy(
		tmp_path,
		((strptr == NULL) ? "" : strptr),
		PATH_MAX + NAME_MAX
	    );
	    tmp_path[PATH_MAX + NAME_MAX - 1] = '\0';

	    events_handled += PromptManage(
		&fb->prompt,
		event
	    );

	    /* Check if prompt's buffer changed. */
	    if((events_handled > 0) &&
               (fb->prompt.buf != NULL)
	    )
	    {
		if(strcmp(tmp_path, fb->prompt.buf))
		{
		    if(fb->sel_dir >= 0)
		    {
		        fb->sel_dir = -1;
                        FBrowserDraw(fb, FBROWSER_DRAW_DIRLIST);
		    }
                    if(fb->sel_file >= 0)
                    {
                        fb->sel_file = -1;
                        FBrowserDraw(fb, FBROWSER_DRAW_FILELIST);
                    }
                    if(fb->sel_object >= 0)
                    {
                        fb->sel_object = -1;
                        FBrowserDraw(fb, FBROWSER_DRAW_ALLLISTS); 
		    }
                }
	    }
	}


	/* Style specific widgets. */
	if(fb->style == FB_STYLE_SINGLE_LIST)
	{
	    /* Devices popup list. */
            if(events_handled == 0)
                events_handled += PUListManage(
		    &fb->devices_pulist,
		    event
		);

	    /* Parent button. */
            if(events_handled == 0)
            {
                events_handled += PBtnManage(&fb->parent_dir_btn, event);
                if((events_handled > 0) &&
                   ((event->type == ButtonRelease) ||
                    (event->type == KeyRelease)
                   )  
                )
                {
		    FBrowserChangeDir(fb, "..");
		    FBrowserRefreshList(fb);
		    FBrowserDraw(fb, FBROWSER_DRAW_COMPLETE);
		    PromptDraw(&fb->prompt, PROMPT_DRAW_AMOUNT_COMPLETE);

		    return(events_handled);
                }
	    }

	    /* Mount button. */
            if(events_handled == 0)
                events_handled += PBtnManage(&fb->mount_btn, event);

	    /* Unmount button. */
            if(events_handled == 0)
                events_handled += PBtnManage(&fb->unmount_btn, event);


	    /* List window scroll bars. */
            if(events_handled == 0)
            {
                OSWGetWindowAttributes(fb->list_win, &wattr);
                /* Handle scroll bars on dir list. */
                events_handled += SBarManage(
                    &fb->list_win_sb,
                    wattr.width, 
                    wattr.height,
                    fb->list_max_width,
		    wattr.height,
                    event
                );
                /* Redraw list as needed. */
                if(events_handled > 0)
                {
                    FBrowserDraw(fb, FBROWSER_DRAW_ALLLISTS);
                }
	    }
	}
	else
	{
	    /* Dir list and file list window scroll bars. */
	    if(events_handled == 0)
	    {
	        OSWGetWindowAttributes(fb->dir_win, &wattr);
                /* Handle scroll bars on dir list. */
                events_handled += SBarManage(
                    &fb->dir_win_sb,
	            wattr.width,
		    wattr.height,
		    wattr.width,
		    (FB_ROW_HEIGHT * fb->dir_list_items)
		        + (2 * FB_ROW_HEIGHT),
		    event
	        );
                /* Redraw list as needed. */
                if(events_handled > 0)
                { 
                    FBrowserDraw(fb, FBROWSER_DRAW_DIRLIST);
                }

                OSWGetWindowAttributes(fb->file_win, &wattr);
                /* Handle scroll bars on file list. */
                events_handled += SBarManage(
                    &fb->file_win_sb,
                    wattr.width,
                    wattr.height,
                    wattr.width,
                    (FB_ROW_HEIGHT * fb->file_list_items)
		        + (2 * FB_ROW_HEIGHT),
                    event
                );
                /* Redraw list as needed. */
                if(events_handled > 0)
                {
                    FBrowserDraw(fb, FBROWSER_DRAW_FILELIST);
                }
	    }
	}

	/* OK button. */
        if(events_handled == 0)
	    events_handled += PBtnManage(&fb->ok_btn, event);

        /* Cancel button. */
        if(events_handled == 0)
            events_handled += PBtnManage(&fb->cancel_btn, event);

        /* Refresh button. */
        if(events_handled == 0)
            events_handled += PBtnManage(&fb->refresh_btn, event);


	return(events_handled);
}


/*
 *	Maps and sets up the change values prompt on
 *	file browser fb.
 */
void FBrowserMapCVPrompt(fbrowser_struct *fb, int mode)
{
	int x, y;
	unsigned int width, height;
	char *strptr, *strptr2;
	fb_object_struct *obj_ptr;


        if(fb == NULL)
            return;


	/* Already mapped? */
	if(fb->cv_prompt_mode != FB_CVP_MODE_NONE)
	    return;


        /* Initialize change value prompt. */
        if(
            PromptInit(
                &fb->cv_prompt,
                fb->toplevel,
                10,
                ((int)fb->height / 2) - (FB_PROMPT_HEIGHT / 2),
                MAX((int)fb->width - 20, 100),
                FB_PROMPT_HEIGHT,
                PROMPT_STYLE_NOBORDER,
                "New value:",
                PATH_MAX + NAME_MAX,        /* Buffer length. */
                0,                          /* Number of history buffers. */
                NULL
            )
        )
            return;


	if((fb->map_state == 0) ||
           (fb->prompt.buf == NULL) ||
	   (fb->cv_prompt.buf == NULL)  ||
           (fb->options & FB_FLAG_WRITE_PROTECT)
	)
	    return;


	/* Change focus on fb and its widgets. */
	fb->is_in_focus = 1;

        fb->list_win_sb.is_in_focus = 0; 
        fb->dir_win_sb.is_in_focus = 0;
        fb->file_win_sb.is_in_focus = 0;

	fb->prompt.is_in_focus = 0;
	fb->devices_pulist.is_in_focus = 0;


	/* ********************************************************** */

	/* Free previous change values prompt target. */
	free(fb->cv_prompt_target);
	fb->cv_prompt_target = NULL;


	switch(mode)
	{
	  case FB_CVP_MODE_RENAME:
	    if(fb->style == FB_STYLE_SINGLE_LIST)
	    {
		/* Get selected object's name. */
		strptr = FBrowserGetSelectionName(fb);
		if(strptr != NULL)
		{
		    /* Get coordinates. */
		    obj_ptr = FBrowserGetSelObject(fb);
		    if(obj_ptr != NULL)
		    {
		        x = obj_ptr->x - fb->list_win_sb.x_win_pos +
                            FB_LIST_COLUM_MARGIN + 26;
		        y = obj_ptr->y + FB_LIST_ROW_MARGIN;
/*
			width = MAX(obj_ptr->width - 22, 4);
 */
			width = 120;
			height = obj_ptr->height;
		    }
		    else
		    {
			x = 0; y = 0;
			width = 100; height = 30;
                    }

		    /* Record full path. */
		    strptr2 = PrefixPaths(fb->prompt.buf, strptr);
		    if(strptr2 != NULL)
		    {
		        /* Record target object full path. */
			fb->cv_prompt_target = StringCopyAlloc(strptr2);
		    }

		    /* Change cv prompt's buffer. */
		    strncpy(
			fb->cv_prompt.buf,
			strptr,
			fb->cv_prompt.buf_len
		    );
                    PromptUnmarkBuffer(&fb->cv_prompt, PROMPT_POS_END);

		    /*   Move the change values prompt to the right
                     *   position and map it.
		     */
                    fb->cv_prompt_mode = mode;
                    OSWReparentWindow(fb->cv_prompt.toplevel, fb->list_win);
                    OSWMoveResizeWindow(fb->cv_prompt.toplevel, x, y, width, height);
                    PromptMap(&fb->cv_prompt);

                    fb->cv_prompt.is_in_focus = 1;
		}
	    }
	    else
	    {


	    }
	    break;
	}


	return;
}


/*
 *	Unmaps change values prompt on file browser.
 */
void FBrowserUnmapCVPrompt(fbrowser_struct *fb)
{
        if(fb == NULL)
            return;


	/* Free recorded target path. */
	free(fb->cv_prompt_target);
	fb->cv_prompt_target = NULL;

	/* Reset mode. */
	fb->cv_prompt_mode = FB_CVP_MODE_NONE;

	/* Destroy change values prompt. */
        PromptDestroy(&fb->cv_prompt);

	/* Make sure it is out of focus. */
	fb->cv_prompt.is_in_focus = 0;


	return;
}


/*
 *	Maps file browser.
 */
void FBrowserMap(fbrowser_struct *fb)
{
	win_attr_t wattr;


	if(fb == NULL)
	    return;


	/* Unselect dir and file list positions. */
	fb->sel_dir = -1;
	fb->sel_file = -1;
	fb->sel_object = -1;


	/* Move to center. */
	WidgetCenterWindow(fb->toplevel, WidgetCenterWindowToRoot);

	OSWGetWindowAttributes(fb->toplevel, &wattr);
	fb->x = wattr.x;
	fb->y = wattr.y;
	fb->width = wattr.width;
	fb->height = wattr.height;


	/* Map it. */
	fb->map_state = 0;
	FBrowserDraw(fb, FBROWSER_DRAW_COMPLETE);


	/* Reset focuses. */
        fb->dir_win_sb.is_in_focus = 0; 
        fb->file_win_sb.is_in_focus = 0;
        fb->list_win_sb.is_in_focus = 0;

        fb->prompt.is_in_focus = 1;
        fb->devices_pulist.is_in_focus = 0;


	return;
}


/*
 *	Maps the file browser, sets its location prompt
 *	buffer to point to path, and updates the listing
 *	of that path (or if the path is a file, then the path's
 *	parent directory).
 */
void FBrowserMapPath(fbrowser_struct *fb, char *path)
{
	char *strptr;
	char tmp_path[PATH_MAX + NAME_MAX];
	char parent[PATH_MAX + NAME_MAX];


	if(fb == NULL)
	    return;

	if(path == NULL)
	{
	    FBrowserMap(fb);
	    return;
	}

	strptr = PathSubHome(path);
	if(strptr == NULL)
        {
            FBrowserMap(fb); 
            return;
        }
	strncpy(tmp_path, strptr, PATH_MAX + NAME_MAX);
        tmp_path[PATH_MAX + NAME_MAX - 1] = '\0';


	if(ISPATHDIR(tmp_path))
	{
	    /* Is a directory, change to that directory. */
	    FBrowserChangeDir(fb, tmp_path);
            FBrowserRefreshList(fb);
	}
	else
	{
	    /* Is not a directory, get parent and change dir to parent. */

	    strptr = GetParentDir(tmp_path);
	    if(strptr == NULL)
	    {
                FBrowserMap(fb);
                return;
	    }
            strncpy(parent, strptr, PATH_MAX + NAME_MAX);
            parent[PATH_MAX + NAME_MAX - 1] = '\0';

            FBrowserChangeDir(fb, parent);
            FBrowserRefreshList(fb);

	    PromptSetS(&fb->prompt, tmp_path);
            PromptMarkBuffer(&fb->prompt, PROMPT_POS_END);
	}

        FBrowserMap(fb);


	return;
}

/*
 *	Set file browser's search string pattern prepended to
 *	whatever path it currently has, reget listing and map.
 */
void FBrowserMapSearchMask(fbrowser_struct *fb, char *pattern)
{
        char *strptr;
        char tmp_path[PATH_MAX + NAME_MAX];


        if(fb == NULL)
            return;

        if(pattern == NULL)
        {
            FBrowserMap(fb);
            return;
        }

	strptr = FBrowserGetJustPath(fb->prompt.buf);
	strncpy(
	    tmp_path,
	    ((strptr == NULL) ? "/" : strptr),
	    PATH_MAX + NAME_MAX
	);
	tmp_path[PATH_MAX + NAME_MAX - 1] = '\0';

	/* Was a file previously selected? */
	if(!ISPATHDIR(tmp_path))
	{
	    strptr = GetParentDir(tmp_path);
	    strncpy(
		tmp_path,
		((strptr == NULL) ? "/" : strptr),
		PATH_MAX + NAME_MAX
            );
            tmp_path[PATH_MAX + NAME_MAX - 1] = '\0';
	}


	strptr = PrefixPaths(tmp_path, pattern);
	strncpy(
            tmp_path,
            ((strptr == NULL) ? "/" : strptr),
            PATH_MAX + NAME_MAX
        );
        tmp_path[PATH_MAX + NAME_MAX - 1] = '\0';
	PromptSetS(&fb->prompt, tmp_path);
	PromptMarkBuffer(&fb->prompt, PROMPT_POS_END);

	FBrowserRefreshList(fb);

        FBrowserMap(fb);


	return;
}

/*
 *	Unmaps file browser.
 */
void FBrowserUnmap(fbrowser_struct *fb)
{
        if(fb == NULL)
            return;


	/* Unmap everything on this file browser. */
        PBtnUnmap(&fb->refresh_btn);
        PBtnUnmap(&fb->cancel_btn);
	PBtnUnmap(&fb->ok_btn);

	if(fb->style == FB_STYLE_SINGLE_LIST)
	{
	    PUListUnmap(&fb->devices_pulist);
	    PBtnUnmap(&fb->parent_dir_btn);
            PBtnUnmap(&fb->mount_btn);
            PBtnUnmap(&fb->unmount_btn);
	}

	FBrowserUnmapCVPrompt(fb);

	PromptUnmap(&fb->prompt);

	OSWUnmapWindow(fb->toplevel);

	fb->map_state = 0;
	fb->visibility_state = VisibilityFullyObscured;


	/* Unfocus. */
        fb->is_in_focus = 0;
        fb->prompt.is_in_focus = 0;

        fb->dir_win_sb.is_in_focus = 0;
        fb->file_win_sb.is_in_focus = 0;


	/* Destroy buffers. */
        OSWDestroyPixmap(&fb->dir_win_buf);
        OSWDestroyPixmap(&fb->file_win_buf);
        OSWDestroyPixmap(&fb->list_win_buf);
        OSWDestroyPixmap(&fb->toplevel_buf);


	return;
}

/*
 *	Destroys file browser.
 */
void FBrowserDestroy(fbrowser_struct *fb)
{
	int i;


	if(fb == NULL)
	    return;


        /* Delete widget from regeristry. */
        WidgetRegDelete(fb);


	/* Free change values prompt target object path. */
	free(fb->cv_prompt_target);
	fb->cv_prompt_target = NULL;


	if(IDC())
	{
            PromptDestroy(&fb->cv_prompt);
            PromptDestroy(&fb->prompt);

	    PUListDestroy(&fb->devices_pulist);

	    SBarDestroy(&fb->dir_win_sb);
            SBarDestroy(&fb->file_win_sb);
            SBarDestroy(&fb->list_win_sb);

            OSWDestroyWindow(&fb->list_win);
            OSWDestroyPixmap(&fb->list_win_buf);

            OSWDestroyWindow(&fb->dir_win);
	    OSWDestroyPixmap(&fb->dir_win_buf);

            OSWDestroyWindow(&fb->file_win);
            OSWDestroyPixmap(&fb->file_win_buf);

            PBtnDestroy(&fb->unmount_btn);
	    PBtnDestroy(&fb->mount_btn);
            PBtnDestroy(&fb->parent_dir_btn);
            PBtnDestroy(&fb->refresh_btn);
            PBtnDestroy(&fb->cancel_btn);
	    PBtnDestroy(&fb->ok_btn);

            OSWDestroyWindow(&fb->toplevel);
	    OSWDestroyPixmap(&fb->toplevel_buf);
	}


	/* Free dir listing. */
	for(i = 0; i < fb->dir_list_items; i++)
	{
	    if(fb->dir_list[i] == NULL) continue;

	    free(fb->dir_list[i]->name);

	    free(fb->dir_list[i]);
	}
	free(fb->dir_list);
	fb->dir_list = NULL;
	fb->dir_list_items = 0;
	fb->sel_dir = -1;

	/* Free file listing. */
        for(i = 0; i < fb->file_list_items; i++)
        {
            if(fb->file_list[i] == NULL) continue;

            free(fb->file_list[i]->name);

            free(fb->file_list[i]);
        }
        free(fb->file_list);
	fb->file_list = NULL;
	fb->file_list_items = 0;
	fb->sel_file = -1;

	fb->sel_object = -1;


	/* Free device listing. */
	for(i = 0; i < fb->total_devices; i++)
	{
	    if(fb->device[i] == NULL) continue;

	    free(fb->device[i]->name);
	    free(fb->device[i]->dev);
            free(fb->device[i]->mounted_path);

            free(fb->device[i]);
	}
	free(fb->device);
	fb->device = NULL;
	fb->total_devices = 0;



	fb->map_state = 0;
	fb->visibility_state = VisibilityFullyObscured;
        fb->is_in_focus = 0;
        fb->x = 0;
        fb->y = 0;
	fb->width = 0;
	fb->height = 0;
	fb->font = NULL;
        fb->next = NULL;
        fb->prev = NULL;


	return;
}




