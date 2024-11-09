// unvedit/uew.h

/*
                    Universe Edit Window Structure



 */

#ifndef UEW_H
#define UEW_H


#include "../include/osw-x.h"
#include "../include/widget.h"

#include "../include/objects.h"
#include "../include/isrefs.h"

#include "uewprops.h"
#include "uhw.h"


/*
 *	Default title strings:
 */
#define UEW_DEF_TITLE		"Universe Editor"
#define UEW_DEF_ICON_TITLE	"UnvEdit"


/*
 *	Tool bar button hint messages:
 */
#define UEW_HINT_MESG_NEW	"Create new universe"
#define UEW_HINT_MESG_OPEN	"Open universe (CTRL+O)"
#define UEW_HINT_MESG_SAVE	"Save current universe (CTRL+S)"
#define UEW_HINT_MESG_COPY	"Copy selected object (CTRL+C)"
#define UEW_HINT_MESG_PASTE	"Paste object (CTRL+V)"
#define UEW_HINT_MESG_NEWOBJ	"Insert new object (INS)"
#define UEW_HINT_MESG_WEAPONS	"Edit selected object's weapons (CTRL+W)"
#define UEW_HINT_MESG_ECONOMY	"Edit selected object's economy (CTRL+E)"
#define UEW_HINT_MESG_PRINT	"Print current universe (CTRL+P)"



/*
 *	Size defaults:
 */
#define UEW_DEF_WIDTH		800
#define UEW_DEF_HEIGHT		600

#define UEW_PROMPT_HEIGHT	30

#define UEW_MENUBAR_HEIGHT	30
#define UEW_TOOLBAR_HEIGHT	((24 + 4) + 6)
#define UEW_PREVIEW_HEIGHT	100
#define UEW_SPLIT_WIDTH		5
#define UEW_VIEW_STATUS_HEIGHT	(10 + (UEW_PROMPT_HEIGHT * 3))
#define UEW_STATUS_HEIGHT	25
#define UEW_PBAR_WIDTH		200
#define UEW_PBAR_HEIGHT		12
#define UEW_DEF_VIEW_X_POS	190
#define UEW_DEF_PROPERTIES_X_POS	570


/*
 *	Minimum sizes:
 */
#define UEW_MIN_WIDTH           300
#define UEW_MIN_HEIGHT          UEW_MENUBAR_HEIGHT + UEW_TOOLBAR_HEIGHT +\
				UEW_TOOLBAR_HEIGHT + UEW_VIEW_STATUS_HEIGHT +\
				UEW_STATUS_HEIGHT + 100

/*
 *      Draw amount codes:
 */
#define UEW_DRAW_COMPLETE       0
#define UEW_DRAW_TOOLBAR	1
#define UEW_DRAW_PREVIEW	2
#define UEW_DRAW_VIEW		3
#define UEW_DRAW_VIEW_STATUS	4
#define UEW_DRAW_STATUS		5


/*
 *	Important menu bar item positions:
 */
#define UEW_WINDOW_MENU_NUM	5
#define UEW_HELP_MENU_NUM	6



/*
 *      Menu item codes:
 */
#define UEW_MENU_CODE_NONE	0

#define UEW_MENU_CODE_NEW	1
#define UEW_MENU_CODE_OPEN	2
#define UEW_MENU_CODE_OPENNEW	3	/* Open in new window. */
#define UEW_MENU_CODE_SAVE	4
#define UEW_MENU_CODE_SAVEAS	5
#define UEW_MENU_CODE_PRINT	6

#define UEW_MENU_CODE_EXIT      10

#define UEW_MENU_CODE_UNDO	20
#define UEW_MENU_CODE_COPY	21
#define UEW_MENU_CODE_PASTE	22
#define UEW_MENU_CODE_DELETE	23

#define UEW_MENU_CODE_UNVHEADER	30

#define UEW_MENU_CODE_INSERTNEW		40
#define UEW_MENU_CODE_EDITWEAPONS       41
#define UEW_MENU_CODE_EDITECONOMY	42

#define UEW_MENU_CODE_OPT_GENERAL	50

#define UEW_MENU_CODE_NEW_UEW	60

#define UEW_MENU_CODE_ABOUT	70


/* IMPORTANT: No UEW_MENU_CODE_* value should be in this range. */
#define UEW_MENU_CODE_WINDOW_START	100	/* Window item start. */
#define UEW_MENU_CODE_WINDOW_END	199	/* Window item end. */


/*
 *	View zoom bounsd:
 */
#define UEW_VIEW_DEF_ZOOM	0.082
#define UEW_VIEW_ZOOM_MIN	0.003
#define UEW_VIEW_ZOOM_MAX	1.000


/*
 *	Status message length:
 */
#define UEW_STATUS_MESG_MAX	80


/*
 *	Universe edit window structure:
 */
typedef struct {

	char map_state;
	char is_in_focus;
	int x, y;
	unsigned int width, height;

	unsigned long option;
	char unv_file[PATH_MAX + NAME_MAX];
	bool_t has_changes;

	win_t toplevel;
	pixmap_t toplevel_buf;

	/* Menu bar. */
	menu_bar_struct mb;

	/* Tool bar. */
	win_t toolbar;
	push_button_struct	tb_new_btn,
				tb_open_btn,
				tb_save_btn,
				tb_copy_btn,
				tb_paste_btn,
				tb_newobj_btn,
				tb_weapons_btn,
				tb_economy_btn,
				tb_print_btn;

	/* Pane window x coordinate positions. */
	int	view_pos,
		properties_pos;

	/* Objects list. */
	colum_list_struct objects_list;
	win_t preview;
	shared_image_t *isref_img;	/* Small image of isref to be put
					 * on preview
					 */
	push_button_struct	rewind_btn,	/* Buttons on preview win. */
				stop_btn,	/* Also acts as pause. */
				play_btn;
	scroll_bar_struct	preview_sb;	/* Preview animation sb. */


	/* View. */
	win_t view, view_split_bar, view_split_cur;
	shared_image_t *view_img;
	char view_is_in_focus;
	long sect_x, sect_y, sect_z;	/* Current position. */
	double cx, cy, cz;
	double sect_width, sect_height;
	double zoom;

	/* View position status. */
	win_t view_status;
	prompt_window_struct	obj_cx_prompt,
				obj_cy_prompt,
				obj_cz_prompt,
				obj_sect_x_prompt,
				obj_sect_y_prompt,
				obj_sect_z_prompt,
                                obj_name_prompt,
				obj_heading_prompt,
                                obj_size_prompt;

	/* Properties list. */
        win_t props, props_split_bar, props_split_cur;
	scroll_bar_struct props_sb;
	prompt_window_struct prop_prompt[TOTAL_PROP_PROMPTS];


	/* Status bar. */
	win_t status;
	char status_mesg[UEW_STATUS_MESG_MAX];
	progress_bar_struct pbar;


	/* Unverse header. */
	unv_head_struct unv_header;

	/* Objects. */
	xsw_object_struct **object;
	int total_objects;

	/* Selected objects. */
	int *sel_object;
	int total_sel_objects;

	/* Isrefs */
	isref_struct **isref;
	int total_isrefs;
	int cur_isref;

	/* Isref animation markers. */
	int ani_frame, ani_total_frames;
	long ani_int, ani_next;

} uew_struct;

extern uew_struct **uew;
extern int total_uews;


/*
 *	UEW schedualed for deletion:
 */
extern uew_struct *delete_uew;



/* In uewalloc.c */
extern int UEWIsAllocated(int n);
extern int UEWAllocate();
extern void UEWDelete(int n);
extern void UEWDeleteAll();


/* In uewcb.c */
extern int UEWMenuBarCB(void *ptr, int code);
extern void UEWUnvIOProgressNotifyCB(void *ptr, int cur, int max);
extern int UEWObjectsListCLCB(void *ptr);

extern int UEWTBNewPBCB(void *ptr);
extern int UEWTBOpenPBCB(void *ptr);
extern int UEWTBSavePBCB(void *ptr);
extern int UEWTBCopyPBCB(void *ptr);
extern int UEWTBPastePBCB(void *ptr);
extern int UEWTBNewObjPBCB(void *ptr);
extern int UEWTBEconomyPBCB(void *ptr);
extern int UEWTBWeaponsPBCB(void *ptr);
extern int UEWTBPrintPBCB(void *ptr);


/* In uewdde.c */
extern int UEWDDEIsBufferXSWObject(u_int8_t *buf, int len);
extern int UEWDDEPutXSWObject(xsw_object_struct *obj_ptr);
extern xsw_object_struct *UEWDDEFetchXSWObject();


/* In uewlist.c */
extern void UEWUpdateObjectList(uew_struct *uew_ptr);
extern void UEWObjectListScrollToItem(
	uew_struct *uew_ptr,
	int obj_num
);
extern void UEWUpdateObjectListItemName(
	uew_struct *uew_ptr,
	int obj_num
);

/* In uewmacros.c */
extern int UEWGetNumByPtr(uew_struct *uew_ptr);
extern int UEWIsObjectGarbage(uew_struct *uew_ptr, int obj_num);
extern int UEWIsObjectSelected(uew_struct *uew_ptr, int obj_num);
extern int UEWGetLastSelectedObject(uew_struct *uew_ptr);
extern void UEWDoChownAll(
	uew_struct *uew_ptr,
	int old_owner,
	int new_owner
);
extern int UEWDoSelectObject(int n, int obj_num);
extern void UEWDoUnselectAllObjects(int n);
extern int UEWDoSetStatusMesg(int n, char *mesg);
extern void UEWDoUpdateWindowMenus();
extern int UEWDoNew(uew_struct *uew_ptr);
extern int UEWDoOpen(uew_struct *uew_ptr, char *path);   
extern int UEWDoOpenNew(uew_struct *uew_ptr, char *path); 
extern int UEWDoSave(uew_struct *uew_ptr);
extern int UEWDoSaveAs(uew_struct *uew_ptr, char *path);
extern int UEWDoInsertObject(
        uew_struct *uew_ptr,
        xsw_object_struct *obj_ptr
);
extern int UEWDoCopyObject(uew_struct *uew_ptr);
extern int UEWDoPasteObject(uew_struct *uew_ptr);
extern int UEWDoDeleteObject(uew_struct *uew_ptr);


/* In uewmanage.c */
extern int UEWInit(int n);
extern int UEWResize(int n, bool_t force);
extern int UEWDraw(int n, int amount);
extern int UEWManage(int n, event_t *event);
extern int UEWManageAll(event_t *event);
extern void UEWMap(int n);
extern void UEWUnmap(int n);
extern void UEWDestroy(int n);


/* In uewprops.c */
extern void UEWPropsDoGetValues(uew_struct *uew_ptr, int obj_num);
extern void UEWPropsDoSetValues(uew_struct *uew_ptr, int obj_num);

extern int UEWPropsInit(uew_struct *uew_ptr);
extern void UEWPropsResize(uew_struct *uew_ptr);
extern void UEWPropsDraw(uew_struct *uew_ptr, int direction);
extern int UEWPropsManage(uew_struct *uew_ptr, event_t *event);
extern void UEWPropsDestroy(uew_struct *uew_ptr);

/* In uewpropsio.c */
extern void UEWPropsSet(
        prompt_window_struct *prompt,
        xsw_object_struct *obj_ptr
);
extern void UEWPropsGet(
        prompt_window_struct *prompt,
        xsw_object_struct *obj_ptr
);


/* In uewview.c */
extern int UEWViewSelectObject(int n, int x, int y);
extern void UEWDoSetViewStatusObjectValues(int n, xsw_object_struct *obj_ptr);
extern void UEWViewDraw(int n);



#endif	/* UEW_H */
