/*
                             Widget Set

	This widget set was originally designed for XShipWars.
	It may be improved but existing styles should not be changed
	without approval from Wolfpack.

 */


#ifndef WIDGETS_H
#define WIDGETS_H


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <time.h>
#include <sys/time.h>


/* File IO. */
#include "fio.h"

/* OS specific definations. */
#include "os.h"

/* Low-level archatecture graphic definations. */
#include "graphics.h"

/* Operating system wrapper. */
#include "osw-x.h"



/* *********************************************************************
 *
 *    Definations for global widget values:
 */

/*
 *	Color closeness for libXpm:
 */
#ifndef XpmDefaultColorCloseness
    #define XpmDefaultColorCloseness    40000
#endif


/* Default window attributes mask. */
#define DEF_WIN_ATTR_MASK \
	CWBackPixmap   | CWBackPixel     | CWBorderPixmap | \
        CWBorderPixel  | CWBitGravity    | CWWinGravity   | \
        CWBackingStore | CWBackingPlanes | CWBackingPixel | \
        CWSaveUnder    | CWEventMask     | CWColormap     | \
        CWCursor


/* Surface colors. */
#define CLSP_SURFACE_NORMAL	"rgbi:0.03/0.03/0.03"
#define CLSP_SURFACE_SELECTED	"rgbi:1.00/0.80/0.98"	/* Selected. */
#define CLSP_SURFACE_SHADOW	"rgbi:0.001/0.001/0.001"
#define CLSP_SURFACE_HIGHLIGHT	"rgbi:0.14/0.14/0.14"
#define CLSP_SURFACE_EDITABLE	"rgbi:0.00/0.00/0.00"	/* Text areas. */

/* Scroll and scale bar colors. */
#define CLSP_SCROLL_BKG		"rgbi:0.00/0.00/0.00"
#define CLSP_SCROLL_FRAME	"rgbi:0.80/0.10/0.80"
#define CLSP_SCROLL_BAR		"rgbi:1.00/0.40/1.00"
#define CLSP_SCROLL_CURSOR	"rgbi:1.00/0.40/1.00"

/* Text colors. */
#define CLSP_TEXT_NORMAL	"rgbi:0.75/0.75/0.75"
#define CLSP_TEXT_EDITABLE      "rgbi:1.00/0.70/0.95"	/* In text areas. */
#define CLSP_TEXT_SELECTED	"rgbi:0.00/0.00/0.00"	/* Selected text. */
#define CLSP_TEXT_DISABLED	"rgbi:0.55/0.55/0.55"

/* Hint window colors. */
#define CLSP_HINT_BKG		"rgbi:0.90/0.90/0.54"
#define CLSP_HINT_TEXT		"rgbi:0.00/0.00/0.00"

/* Progress bar colors. */
#define CLSP_PBAR_TEXT		"rgbi:1.00/0.70/0.85"


/*
 *	Default double click interval in milliseconds:
 */
#define DEF_DOUBLE_CLICK_INT	500


/*
 *	Default hint window map delay in milliseconds:
 */
#define DEF_HINTWIN_MAP_DELAY	1500

/*
 *	Default slow double click for relabeling items
 *	in milliseconds:
 */
#define DEF_RELABEL_ITEM_DELAY	1500

/*
 *	Default popup list repeat delay and interval
 *	in milliseconds:
 */
#define DEF_PULIST_REPEAT_DELAY		8
#define DEF_PULIST_REPEAT_INT		50


/*
 *      Default prompt scroll repeat delay and interval
 *      in milliseconds:
 *
 *	These are NOT the key repeat delay and interval,
 *	that is controlled by the GUI!
 */
#define DEF_PROMPT_REPEAT_DELAY		50
#define DEF_PROMPT_REPEAT_INT		50


/*
 *      Default scroll bar cursor repeat delay and interval
 *	in milliseconds:
 */
#define DEF_SB_REPEAT_DELAY	250
#define DEF_SB_REPEAT_INT	50

/*
 *	Edge scroll delay:
 */
#define DEF_LIST_EDGE_SCROLL_DELAY	300

/*
 *	Default maximum size for windows, pixmaps, and images:
 */
#define DEF_GRAPHICS_MAX_WIDTH	32767
#define DEF_GRAPHICS_MAX_HEIGHT	32767

/*
 *	Default maximum pointer cursor size:
 */
#define DEF_CURSOR_MAX_WIDTH	256
#define DEF_CURSOR_MAX_HEIGHT	256


/*
 *	Widget type codes:
 *
 *	Code to represent widget type, used for the widget
 *	regeristry.
 */
#define WTYPE_CODE_NONE			0	/* Or unknown. */
#define WTYPE_CODE_PUSHBUTTON		10
#define WTYPE_CODE_COLUMLIST		11
#define WTYPE_CODE_DIALOG		12
#define WTYPE_CODE_FILEBROWSER		13
#define WTYPE_CODE_LIST			14
#define WTYPE_CODE_MENU			15
#define WTYPE_CODE_MENUBAR		16
#define WTYPE_CODE_PAGESTEPPER		17
#define WTYPE_CODE_PROGRESSBAR		18
#define WTYPE_CODE_PROMPT		19
#define WTYPE_CODE_PULIST		20
#define WTYPE_CODE_SCALEBAR		21
#define WTYPE_CODE_SCROLLBAR		22
#define WTYPE_CODE_TOGGLEARRAY		23
#define WTYPE_CODE_TOGGLEBTN		24
#define WTYPE_CODE_VIEWER		25



/*
 *	Widget color struct:
 */
typedef struct {

	u_int8_t a, r, g, b;

} WColorStruct;


/*
 *   Widget Cursor structure:
 */
typedef struct {

	cursor_t cursor;

        int x;                  /* Hot point. */
        int y;
        unsigned int width;
        unsigned int height;

	depth_t	depth;		/* Usually 1. */

        WColorStruct color;	/* Foreground color. */

} WCursor;



/* *************************************************************************
 *
 *   Global widget values:
 *
 *	These values are used by (almost) all widgets.
 *	They include text color, selected text color, background pixmap
 *	etc.
 *
 *	These values are initialized by WidgetInitGlobals();
 *	And then cleaned up by WidgetDestroyGlobals();
 */
typedef struct {

	/* Indicates these global values are initialized. */
	char is_init;	/* 1 = yes, 0 = no. */


	/* Force black and white mode. */
	bool_t force_mono;


	/* Cursors. */
	WCursor	*std_arrow_wcr,	/* The standard arrow pointer. */
		*h_split_wcr,	/* Horizontal split. */
		*v_split_wcr,	/* Vertical split. */
		*drag_item_wcr,
		*drag_file_wcr,
		*text_wcr,
		*no_way_wcr;	/* Circle with diagonal slash. */

	/* Fonts. */
	font_t	*pbtn_font,
		*menu_font,
		*prompt_label_font,
		*prompt_text_font,
		*scale_bar_font,
		*std_font;

 	/* The standard background pixmap (tiled). */
	pixmap_t	std_bkg_pm,
			std_icon_pm;	/*   For the icon pixmap in
                 			 *   XSetStandardProperties().
			         	 */

	/* Icons used in lists, dialogs and other places. */
	image_t *std_bkg_img,		/* Standard background. */
		*menu_bkg_img,
		*hint_bkg_img;

	image_t	*btn_unarmed_img,	/* Button backgrounds. */
		*btn_armed_img,
		*btn_highlighted_img;

	image_t	*scalebar_h_img,	/* Scalebar handles. */
		*scalebar_v_img;

	image_t *pulist_map_icon_img;	/*   Icon image on the map button
                                         *   of the popup list widget.
					 */
	image_t *goto_parent_img;
	image_t *browse_files_img;		/* Browse files icon. */

	image_t	*mount_img,
		*unmount_img;

	image_t	*diricon_normal_img,
		*diricon_selected_img,

		*execicon_normal_img,
		*execicon_selected_img,

		*fileicon_normal_img,
		*fileicon_selected_img,

		*linkicon_normal_img,
		*linkicon_selected_img,

		*pipeicon_normal_img,
		*pipeicon_selected_img,

		*stditem_normal_img,
		*stditem_selected_img;


	image_t *toggle_btn_unarmed_img,	/* Toggle buttons. */
		*toggle_btn_armed_img;

	image_t	*tab_normal_img,		/* Tabs. */
		*tab_selected_img;


	/* `Surface' edge colors. */
	pixel_t	surface_normal_pix,	/* Overrided by std_bkg. */
		surface_editable_pix,
		surface_selected_pix,
		surface_shadow_pix,
		surface_highlight_pix;

	/* Scrollbar and scalebar colors. */
	pixel_t	scroll_bkg_pix,
		scroll_frame_pix,
                scroll_bar_pix,
                scroll_cursor_pix;

	/* Text colors. */
	pixel_t	normal_text_pix,
		editable_text_pix,
                selected_text_pix,
		disabled_text_pix;

	/* Hint window colors. */
	pixel_t	hint_bkg_pix,
		hint_text_pix;

	/* Progress bar colors.*/
	pixel_t pbar_text_pix;


	/* Double click interval (in milliseconds). */
	time_t double_click_int;

	/* Hint window map delay (in milliseconds). */
	time_t hintwin_map_delay;

	/* Slow double click delay for relabeling items (in milliseconds). */
	time_t relabel_item_delay;

	/* Popup list repeat delay and interval (in milliseconds). */
	time_t pulist_repeat_delay;
	time_t pulist_repeat_interval;

	/* Prompt repeat delay and interval (in milliseconds). */
	time_t prompt_repeat_delay;
	time_t prompt_repeat_interval;

	/* Scroll bar repeat delay and interval (in milliseconds). */
	time_t sb_repeat_delay;
	time_t sb_repeat_interval;

	/* List edge select scroll delay (in milliseconds). */
	time_t list_edge_scroll_delay;

	/* Maximum window, pixmap, image, etc size. */
	unsigned int max_width, max_height;

} widget_global_struct;
extern "C" widget_global_struct widget_global;

/*
 *	Hint window:
 *
 *	This small window will be managed by the widget system,
 *	it displays a short (user set) message about the usage of a
 *	widget.
 */
typedef struct {

        char map_state;
        char is_in_focus;
        visibility_t visibility_state;
        int x, y;
        unsigned int width, height;
        bool_t disabled;
        font_t *font;

	time_t next_map;	/* Next time to be mapped in ms or 0 for
				 * none.
				 */

	win_t toplevel;
	pixmap_t toplevel_buf;

	win_t ref_win;	/* Referance window. */

} hint_win_struct;
extern "C" hint_win_struct hint_win;

/*
 *	Hint window messages and records:
 */
typedef struct {

	win_t ref_win;		/* Referance window. */

	font_t *font;
	char *mesg;

} hint_win_data_struct;
extern "C" hint_win_data_struct **hint_win_data;
extern "C" int total_hint_win_datas;


/* ********************************************************************
 *
 *                     Individual Widget Structures
 *
 */

/*
 *	Push Button Widget:
 */
/* Maximum hotkeys. */
#define PBTN_MAX_HOTKEYS	16

/* Alignment codes. */
#define PBTN_TALIGN_CENTER      0
#define PBTN_TALIGN_LEFT        1
#define PBTN_TALIGN_RIGHT       2

typedef struct {

        char map_state;
        char is_in_focus;
        visibility_t visibility_state;
        int x, y;
        unsigned int width, height;
        bool_t disabled;
        font_t *font;
	void *prev, *next;

	win_t toplevel;
	pixmap_t toplevel_buf;

	/* Display as `default button' if True. */
	bool_t is_default;

        /* Button state. */
#define PBTN_UNARMED            0
#define PBTN_ARMED              1
#define PBTN_HIGHLIGHTED        2
        char state;

	/* Button background images, these are local (not shared). */
	image_t *unarmed_img,
		*armed_img,
		*highlighted_img;

	win_t parent;

	char hotkey[PBTN_MAX_HOTKEYS + 1];	/* NULL terminated. */

	char *label;		/* Can be NULL. */
	int label_align;	/* Defaults to PBTN_TALIGN_CENTER. */

	image_t *image;		/* Image to be used as label. */

	void *client_data;
	int (*func_cb)(void *);	/* Callback function, can be NULL. */

} push_button_struct;



/*
 *	Toggle Button Widget:
 *
 *	Toggle buttons are on/off buttons to indicate a boolean value.
 */
#define TGBTN_DRAW_AMOUNT_COMPLETE	0
#define TGBTN_DRAW_AMOUNT_BUTTON	1
#define TGBTN_DRAW_AMOUNT_LABEL		2

typedef struct
{
        char map_state;
        char is_in_focus;
        visibility_t visibility_state;
        int x, y;
        unsigned int width, height;
        bool_t disabled;
        font_t *font;
        void *prev, *next;

	win_t toplevel;
	pixmap_t toplevel_buf;

	char *label;	/* Label of toggle button. */

	bool_t state;	/* Toggle button state (not map state). */

} toggle_button_struct;


/*
 *	Toggle Button Array Widget:
 *
 *	An array of Toggle Button Widgets.
 */
#define TGBTN_ARRAY_ALIGN_VERTICAL	0
#define TGBTN_ARRAY_ALIGN_HORIZONTAL	1
#define TGBTN_ARRAY_ALIGN_CASCADE	2

typedef struct
{
        char map_state;
        char is_in_focus;
        visibility_t visibility_state;
        int x, y;
        unsigned int width, height;
        bool_t disabled;
        font_t *font;
        void *prev, *next;

	win_t toplevel;

	toggle_button_struct **tb;
	int total_tbs;

	int armed_tb;

} toggle_button_array_struct;



/*
 *	Progress Bar Widget:
 */
#define PBAR_COMPLETION_HOLD		0
#define PBAR_COMPLETION_WRAP		1
#define PBAR_COMPLETION_UNMAP		2
#define PBAR_COMPLETION_FLASH		3

typedef struct
{
        char map_state;
        char is_in_focus;
        visibility_t visibility_state;
        int x, y;
        unsigned int width, height;
        bool_t disabled;
        font_t *font;
        void *prev, *next;

	win_t toplevel;
	pixmap_t toplevel_buf;
	image_t *image_buf;

	/* Progress values. */
	double	current,	/* Current progress. */
		min,		/* Minimum value. */
		max;		/* Maximum value. */

	char *label;		/* Label (optional) */

	int completion_action;	/* One of PBAR_COMPLETION_*. */

} progress_bar_struct;



/*
 *	Dialog Widget:
 *
 *	A window parented to root (desktop) which can be resized and moved
 *	and contain a message with an icon.
 */
typedef struct
{
	char map_state;
	char is_in_focus;
	visibility_t visibility_state;
	int x, y;
	unsigned int width, height;
	bool_t disabled;
	font_t *font;
        void *prev, *next;

	win_t toplevel;
	pixmap_t toplevel_buf;

	push_button_struct dismiss_btn;

	/* Automatic dismiss (not supported yet). */
	char auto_dismiss;
	long next_auto_dismiss;

	/* Message. */   
	char *mesg;
	int len, total_lines, longest_line;

	/* Icon. */
	image_t *icon_img;

} dialog_win_struct;



/*
 *	Prompt Widget:
 *
 *	Single line text areas.
 *
 *	NOTE: Do not confuse prompt with dialog windows, they are
 *	different.
 */

/* Definations of draw amount in PromptDraw(). */
#define PROMPT_DRAW_AMOUNT_COMPLETE     0
#define PROMPT_DRAW_AMOUNT_TEXTONLY     1

#define PROMPT_STYLE_FLUSHED		0
#define PROMPT_STYLE_RAISED		1
#define PROMPT_STYLE_NOBORDER		2

typedef struct
{
        char map_state;
        char is_in_focus;
        visibility_t visibility_state;
        int x, y;
        unsigned int width, height;
        bool_t disabled;
        font_t *font;
        void *prev, *next;

	int style;		/* One of PROMPT_STYLE_*. */

	win_t toplevel;
	pixmap_t toplevel_buf;

	win_t text_area;
	pixmap_t text_area_buf;

	/* Label printed just to the left of the text field window. */
	char *name;

	/* Main text buffer. */
	char *buf;
	unsigned int buf_len;

	int buf_pos;		/* Position of cursor in buf (can be -1). */
	int buf_vis_pos;	/* Scrolled position in buf, char units. */

	int buf_sel_start;	/* Start byte of selected text (-1 for none). */
	int buf_sel_end;	/* End byte of selected text (-1 for none). */

	/* History buffer. */
	char **hist_buf;
	int total_hist_bufs;	/* Total number of history buffers. */
	int hist_buf_pos;	/* Which history buffer we last recalled, */


	int (*func_cb)(char *);	/* Callback function, can be NULL. */

} prompt_window_struct;



/*
 *	Scale Bar Widget:
 *
 *	(Not to be confused with scroll bars.)
 */
#define SCALEBAR_ORIENT_HORIZONTAL	0
#define SCALEBAR_ORIENT_VERTICAL	1

#define SCALEBAR_STYLE_STANDARD	0
#define SCALEBAR_STYLE_STANDARD_VALUE	1	/* Value shown. */
#define SCALEBAR_STYLE_FLUSHED	2	/* Simple, similar to scrollbar style. */

#define SCALEBAR_BAR_STANDARD_WIDTH	26
#define SCALEBAR_BAR_FLUSHED_WIDTH	12

typedef struct
{
        char map_state;
        char is_in_focus;
        visibility_t visibility_state;
        int x, y;
        unsigned int width, height;
        bool_t disabled;
        font_t *font; 
        void *prev, *next;

	int style;	/* One of SCALEBAR_STYLE_* */
	int ticks;	/* Number of ticks to draw. */

	double pos;	/* Position in user defined units. */
	double pos_min;	/* In user defined units. */
	double pos_max;	/* In user defined units. */

	int orientation;	/* One of SCALEBAR_ORIENT_* */
	bool_t flip_pos;	/* Flip position. */

	bool_t btn_state;	/* Button press state on slide bar. */

	win_t toplevel;
	pixmap_t toplevel_buf;
	pixmap_t bkg_buf;

	unsigned int length;         /* Length of scalebar in pixels. */

        /* Function to call (if not NULL) on select. */
        void *client_data;
        int (*func_cb)(void *);

} scale_bar_struct;  
 
  
/*
 *	Scroll Bar Widget:
 */
#define SCROLLBAR_CURSOR_BTN_WIDTH 16
#define SCROLLBAR_CURSOR_BTN_HEIGHT 16
#define SCROLLBAR_XBAR_HEIGHT 16  
#define SCROLLBAR_YBAR_WIDTH 16
#define SCROLLBAR_CURSOR_INC 20

typedef struct
{
        char map_state;
        char is_in_focus;
        visibility_t visibility_state;
        int x, y;
        unsigned int width, height;
        bool_t disabled;
        font_t *font; 
        void *prev, *next;

	/* x and y scroll positions. */
	int	x_win_pos,
		y_win_pos;  

	/* Original ButtonPress deltas to scroll bar position. */
	int	x_origin_delta,
		y_origin_delta;

	/* Toplevel, one for each bar. */
	win_t	x_toplevel,
		y_toplevel;

	/* Horizontal and vertical scroll bar slide channels. */
	win_t	x_bar,
		y_bar;
	pixmap_t	x_bar_buf,
			y_bar_buf;
	bool_t	x_bar_button_state,
		y_bar_button_state;

	/* Cursor buttons. */
	win_t	x_left,
		x_right;
	pixmap_t	x_left_buf,
			x_right_buf;
	win_t	y_up,
		y_down;
	pixmap_t	y_up_buf,
			y_down_buf;

} scroll_bar_struct;


/*
 *	Popup list:
 *
 *	(Superceeds pull-down list).
 */
#define PULIST_DRAW_AMOUNT_COMPLETE	0
#define PULIST_DRAW_AMOUNT_LABEL	1
#define PULIST_DRAW_AMOUNT_PULIST	2

#define PULIST_POPUP_CENTER	0
#define PULIST_POPUP_UP		1
#define PULIST_POPUP_DOWN	2

typedef struct {

	char *name;
	bool_t disabled;

} popup_list_item_struct;

typedef struct {

        char map_state;
        char is_in_focus;
        visibility_t visibility_state;
        int x, y;
        unsigned int width, height;
        bool_t disabled;
        font_t *font;  
        void *prev, *next;

	/* Selected item label. */
	win_t toplevel;
	pixmap_t toplevel_buf;
	push_button_struct map_btn;

	/* Popup list. */
	char popup_map_state;

	win_t popup_toplevel;
	pixmap_t popup_toplevel_buf;

	win_t popup_list;
	pixmap_t popup_list_buf;
        int popup_list_vis_items;       /*   Items visable on popup list.   
                                         *   Also determines popup list
                                         *   height.
                                         */

	/* Popup list size and scroll position. */
	unsigned int list_max_width, list_max_height;
	int list_y_pos;			/* Scrolled position. */

	/* Items. */
	popup_list_item_struct **item;
	int total_items;
	int sel_item, prev_sel_item;

	/* Options. */
	int direction;	/* One of PULIST_POPUP_* */

        /* Function to call (if not NULL) on select. */
        void *client_data;
        int (*func_cb)(void *);

} popup_list_struct;



/*
 *	List window widget:
 *
 *	(Bulletin board style).
 */
#define LIST_ENTRY_TYPE_NORMAL		0
#define LIST_ENTRY_TYPE_FOLDER		1
#define LIST_ENTRY_TYPE_HR		2	/* Horizontal rule. */

#define LW_CVP_MODE_NONE		0
#define LW_CVP_MODE_RENAME		1

typedef struct
{
	/* Type of entry. */
	int type;

	/* Label name. */
	char *name;

	/* Pointer to icon image, must be deallocated by client. */
	image_t *image;

	/* Pointer to client data. */
	void *data_ptr;

} list_window_entry_struct;     /* List window entry structure. */

typedef struct
{
        char map_state;
        char is_in_focus;
        visibility_t visibility_state;
        int x, y;
        unsigned int width, height;
        bool_t disabled;
        font_t *font;  
        void *prev, *next;

	win_t toplevel;
	pixmap_t toplevel_buf;
	scroll_bar_struct sb;

	bool_t allow_drag; 
	unsigned int char_width, char_height;
	unsigned int row_height;

	list_window_entry_struct **entry;
	int	total_entries,
		entry_pos;	/* Selected entry, can be -1. */

	/* Margins (in pixels). */
	int	left_margin,
		top_margin;

	/* Drag cursor being used (private). */
	WCursor *drag_cursor;

	/* Change values prompt. */
	prompt_window_struct cv_prompt;
	int cv_prompt_mode;
	int cv_prompt_entry_pos;	/* Modifying values for this entry. */

	/* Function to call (if not NULL) on enter or double click . */
        void *client_data;
        int (*func_cb)(void *);

} list_window_struct;


/*
 *	Colum list widget:
 */
#define CL_DRAW_AMOUNT_COMPLETE		0
#define CL_DRAW_AMOUNT_LIST		1
#define CL_DRAW_AMOUNT_HEADING		2

typedef struct {

        char *label;

	font_t *font;
	pixel_t pixel;
	unsigned int attr;

	void *client_data;

} colum_list_item_struct;

typedef struct {

	colum_list_item_struct **item;
	int total_items;

} colum_list_row_struct;

/* Colum heading structure. */
typedef struct {

	char *heading;		/* Heading label. */

        font_t *font;
        pixel_t pixel;
        unsigned int attr;

	int x_pos;		/* In pixels. */

} colum_list_colum_struct;

#define CL_FLAG_ALLOW_DRAG		(1 << 0)
#define CL_FLAG_ALLOW_MULTI_SELECT	(1 << 1)

typedef struct {

        char map_state;
        char is_in_focus;
        visibility_t visibility_state;
        int x, y;
        unsigned int width, height;
        bool_t disabled;
        font_t *font;
        void *prev, *next;

	unsigned int option;
        unsigned int row_height;

	win_t toplevel;

	/* Heading. */
	win_t heading;
	pixmap_t heading_buf;

	win_t heading_split_bar;
	bool_t heading_split_indrag;	/* Dragging heading colum? */
	int heading_drag_colum;		/* Which colum is being dragged or
                                         * which colum split is pointer
                                         * over.
                                         */

	/* List. */
	win_t list;
        pixmap_t list_buf;
	scroll_bar_struct sb;		/* For the list. */

	/* Colum headings. */
	colum_list_colum_struct **colum;
	int total_colums;

	/* Each row. */
	colum_list_row_struct **row;
	int total_rows;

	/* Selected rows. */
	int *sel_row;
	int total_sel_rows;

        /* Function to call (if not NULL) on enter or double click . */
        void *client_data;
        int (*func_cb)(void *);

} colum_list_struct;

    
/*  
 *	Menu:
 */
/* Menu item type codes. */
#define MENU_ITEM_TYPE_ENTRY		0
#define MENU_ITEM_TYPE_TOGGLEENTRY	1
#define MENU_ITEM_TYPE_FOLDER		2	/* Reffers to another menu. */
#define MENU_ITEM_TYPE_HR		3	/* Horizontal rule. */
#define MENU_ITEM_TYPE_COMMENT		4	/* Does nothing when selected. */

/* Menu item flags. */
#define menu_item_flags_t	unsigned long

#define MENU_ITEM_FLAG_TOGGLED		(1 << 1)	/* Set if toggled. */
#define MENU_ITEM_FLAG_ACCEL_ALT	(1 << 2)
#define MENU_ITEM_FLAG_ACCEL_CTRL	(1 << 3)
#define MENU_ITEM_FLAG_ACCEL_SHIFT	(1 << 4)

/* Menu items structure. */
typedef struct {

	int type;	/* One of MENU_ITEM_TYPE_*. */
	menu_item_flags_t flags;	/* Any of MENU_ITEM_FLAG_*. */
	char *name;	/* Label. */
	image_t *icon;	/* Shared. */
	int id_code;	/* Client specified code to identify this
			 * menu item.
			 */
	char accelerator;	/* Accelerator key. */

} menu_item_struct;

typedef struct {

        char map_state;
        char is_in_focus;
        visibility_t visibility_state;
        int x, y;
        unsigned int width, height;
        bool_t disabled;
        font_t *font;
        void *prev, *next;

	win_t toplevel;
	pixmap_t toplevel_buf;

	int row_height;     /* In pixels, must be 1 or greater. */
	int char_width;

	menu_item_struct **item;
	int selected_item;
	int total_items;

	/*   The function to call (if not NULL) to execute a menu item by
	 *   its ID code.   The passed information is the void *client_data
	 *   pointer and then the int id_code.
	 */
        void *client_data;
	int (*func_cb)(void *, int);

} menu_struct;



/*
 *	Menu Bar:
 */

/* Menu bar item structure. */
#define MENUBAR_ITEM_HOTKEYS_MAX	24
typedef struct {

	char *name;
	char hotkeys[MENUBAR_ITEM_HOTKEYS_MAX];

	int x, y;
	unsigned int width, height;

	menu_struct *menu;

} menu_bar_item_struct;

typedef struct {

        char map_state;
        char is_in_focus;
        visibility_t visibility_state;
        int x, y;  
        unsigned int width, height;
        bool_t disabled;
        font_t *font;
        void *prev, *next;

        win_t toplevel;   
        pixmap_t toplevel_buf;

	menu_bar_item_struct **item;
	int total_items;
	int sel_item;

	int (*func_cb)(void *, int);
	void *client_data;

} menu_bar_struct;


/*
 *	File Browser:
 */
/* Draw amount codes for FBrowserDraw() */
#define FBROWSER_DRAW_COMPLETE		0
#define FBROWSER_DRAW_DIRLIST		1
#define FBROWSER_DRAW_FILELIST		2
#define FBROWSER_DRAW_ALLLISTS		3
#define FBROWSER_DRAW_PROMPT		4
#define FBROWSER_DRAW_BUTTONS		5
#define FBROWSER_DRAW_SCROLLBARS	6

/* Option flags. */
#define FB_FLAG_WRITE_PROTECT		(1 << 0)
#define FB_FLAG_CLOSE_ON_OK		(1 << 1)
#define FB_FLAG_CLOSE_ON_CANCEL		(1 << 2)
#define FB_FLAG_MUST_EXIST		(1 << 3)

/* Styles. */
#define FB_STYLE_SPLIT_LIST		0	/* Split dir and file win. */
#define FB_STYLE_SINGLE_LIST		1	/* Single list window. */

/* Filesystem types. */
#define FB_FSTYPE_UNKNOWN		0
#define FB_FSTYPE_SWAP			1	/* Universal swap. */
#define FB_FSTYPE_EXT			2
#define FB_FSTYPE_EXT2			3
#define FB_FSTYPE_PROC			4
#define FB_FSTYPE_MSDOS			5	/* Includes Win9* VFS */
#define FB_FSTYPE_ISO9660		6	/* CDRom. */
#define FB_FSTYPE_MINIX			7
#define FB_FSTYPE_XIAFS			8
#define FB_FSTYPE_HPFS			9
#define FB_FSTYPE_NFS			10


/* File browser object structure. */
typedef struct {

	char *name;
	int x, y;
	unsigned int width, height;

	mode_t mode;		/* Type and permissions. */
	uid_t uid;		/* User ID of owner */
	gid_t gid;		/* Group ID of owner */
	off_t size;		/* Total size, in blocks. */
	time_t atime;		/* time of last access. */
	time_t ctime;		/* time of creation. */
	time_t mtime;		/* time of last modification. */

} fb_object_struct;

typedef struct {

	char *name;		/* Friendly name. */
	char *dev;		/* Device name. */
	char *mounted_path;	/* Path that it is mounted on. */

	int fs_type;		/* File system type code. */

	bool_t	readable,
		writeable,
		mounted;

} fb_device_struct;

typedef struct {

        char map_state;
        char is_in_focus;
        visibility_t visibility_state;
        int x, y;
        unsigned int width, height;
        bool_t disabled;
        font_t *font;
        void *prev, *next;

	u_int64_t options;
	int style;

	win_t toplevel;
	pixmap_t toplevel_buf;

	push_button_struct	ok_btn,
				cancel_btn,
				refresh_btn,

				parent_dir_btn,
				mount_btn,
				unmount_btn;


	/* Directory window (for FB_STYLE_SPLIT_LIST). */
	win_t dir_win; 
	pixmap_t dir_win_buf;
	scroll_bar_struct dir_win_sb;

	/* File window (for FB_STYLE_SPLIT_LIST). */
	win_t file_win;
	pixmap_t file_win_buf;
	scroll_bar_struct file_win_sb;

        /* List window (for FB_STYLE_SINGLE_LIST). */
        win_t list_win;
        pixmap_t list_win_buf;
        scroll_bar_struct list_win_sb;
	unsigned int list_max_width;

	/* Current location prompt. */
	prompt_window_struct prompt;

	/* Change value prompt. */
	prompt_window_struct cv_prompt;
	char *cv_prompt_target;			/* Target object. */
	int cv_prompt_mode;


	/* Devices list. */
	fb_device_struct **device;
	int total_devices;

	popup_list_struct devices_pulist;


	/* Directories list. */
	fb_object_struct **dir_list;
        int dir_list_items;

	/* Files list. */
	fb_object_struct **file_list;
	int file_list_items;

	/* Selected item positions (can be -1 for none). */
	int	sel_dir,	/* For FB_STYLE_SPLIT_LIST. */
		sel_file,	/* For FB_STYLE_SPLIT_LIST. */
		sel_object;	/* For FB_STYLE_SINGLE_LIST. */


	/* Function to call when OK button is pressed (can be NULL). */
	int (*func_ok)(char *);

	/* Function to call when Cancel button is pressed (can be NULL). */
	int (*func_cancel)(char *);

} fbrowser_struct;


/*
 *	File viewer:
 */
#define VIEWER_MODE_ASCII	0
#define VIEWER_MODE_HEX		1

typedef struct {

        char map_state;
        char is_in_focus;
        visibility_t visibility_state;
        int x, y;
        unsigned int width, height;
        bool_t disabled;
        font_t *font;
        void *prev, *next;

	win_t toplevel;

	win_t viewer;
	pixmap_t viewer_buf;
	scroll_bar_struct sb;
	unsigned int max_viewer_width;

	char *filename;		/* Can be NULL. */
	char *buf;		/* Can be NULL. */
	off_t buf_len;
	int lines;		/* Total number of lines. */

	char viewer_mode;

	push_button_struct	close_btn,
				ascii_mode_btn,
				hex_mode_btn;

} file_viewer_struct;


/*
 *	Page Stepper:
 */
typedef struct {

	void *w;	/* Pointer to allocated widget struct. */
	int w_type;	/* Widget type. */

	char *name;	/* Unique name. */

} page_widget_struct;

typedef struct {

	int x, y;

	font_t *font;
	pixel_t pix;
	char **text;		/* Can be NULL. */
	int total_text_lines;

	image_t *image;		/* Can be NULL. */

} page_stepper_label_struct;

typedef struct {

	char map_state;

	page_widget_struct **widget;		/* Widget. */
	int total_widgets;

	page_stepper_label_struct **label;
	int total_labels;

} page_stepper_page_struct;

typedef struct {

        char map_state;
        char is_in_focus;
        visibility_t visibility_state;
        int x, y;
        unsigned int width, height;
        bool_t disabled;
        font_t *font;
        void *prev, *next;

        win_t toplevel;
	pixmap_t toplevel_buf;

	win_t parent;

	page_stepper_page_struct **page;
	int total_pages;

	int cur_page;

	push_button_struct	next_btn,
				prev_btn;

	/* Panel image (shared) */
	image_t *panel_img;

	/* Client data and page change callback function. */
	void *client_data;
	int (*page_change_handler)(void *, int, int);
	int (*exit_handler)(void *);
	int (*finish_handler)(void *);

} page_stepper_struct;


/*
 *	Widget Regeristry:
 *
 *	(Needs to be declared after all widget structures)
 */
typedef struct {

	void *ptr;	/* Shared, do not free. */
	int type;

} widget_reg_entry_struct;

typedef struct {

	widget_reg_entry_struct **entry;
	int total_entries;

} widget_reg_struct;
extern "C" widget_reg_struct widget_reg;


/* *********************************************************************
 *
 *                            FUNCTIONS
 *
 */

/* In wglobal.c */
extern "C" int WidgetInitGlobals(int argc, char *argv[]);
extern "C" void WidgetDestroyGlobals(void);
extern "C" int WidgetManage(event_t *event);


/* In whintwin.c */
extern "C" int HintWinIsDataAllocated(int n);
extern "C" int HintWinGetNumByWin(win_t ref_win);
extern "C" int HintWinInList(win_t ref_win);

extern "C" int HintWinInit(void);
extern "C" int HintWinDraw(void);     
extern "C" int HintWinManage(event_t *event);
extern "C" void HintWinMap(void);     
extern "C" void HintWinUnmap(void);   
extern "C" void HintWinDestroy(void);

extern "C" int HintWinAddMessage(
        win_t ref_win,
        win_t parent_win,
	int x, int y,
        const char *mesg
);
extern "C" int HintWinChangeMesg(win_t ref_win, const char *mesg);
extern "C" void HintWinDeleteMessage(win_t ref_win);
extern "C" void HintWinDeleteAllMessages(void);

extern "C" int HintWinSetSchedual(
        long d_msec,
        win_t ref_win
);
extern "C" int HintWinSetSchedualMessage(
	long d_msec,
	win_t ref_win,
	const char *mesg
);


/* In wfile.c */
extern "C" image_t *WidgetLoadImageFromTgaFile(char *filename);
extern "C" image_t *WidgetLoadImageFromTgaData(u_int8_t *data);

extern "C" pixmap_t WidgetLoadPixmapFromTgaFile(char *filename);
extern "C" pixmap_t WidgetLoadPixmapFromTgaData(u_int8_t *data);

extern "C" image_t *WidgetLoadImageFromXpmFile(char *filename);
extern "C" image_t *WidgetLoadImageFromXpmData(char **data);

extern "C" pixmap_t WidgetLoadPixmapFromXpmFile(char *filename);
extern "C" pixmap_t WidgetLoadPixmapFromXpmData(char **data);


/* In wutils.c */
extern "C" pixel_t WidgetGetPixel(char *clsp);

extern "C" WCursor *WidgetCreateCursorFromFile(
	char *xpmfile,
	int hot_x, int hot_y,
        WColorStruct color
);
extern "C" WCursor *WidgetCreateCursorFromData(
        char **xpmdata,
        int hot_x, int hot_y,
        WColorStruct color
);
extern "C" void WidgetSetWindowCursor(win_t w, WCursor *wcursor);
extern "C" void WidgetDestroyCursor(WCursor **wcursor);

extern "C" void WidgetSetWindowWMSizeHints(win_t w, sizehints_t sizehints);

#define WidgetCenterWindowToParent		0
#define WidgetCenterWindowToRoot		1
#define WidgetCenterWindowToPointer		2
extern "C" void WidgetCenterWindow(win_t w, int relation);

extern "C" void WidgetMap(void *ptr);
extern "C" void WidgetUnmap(void *ptr);
extern "C" void WidgetDestroy(void *ptr);

extern "C" void WidgetResizeImageBuffer(
	depth_t d,
	u_int8_t *tar_buf,
	u_int8_t *src_buf,
	unsigned int tar_width,
	unsigned int tar_height,
	unsigned int src_width,
	unsigned int src_height
);
extern "C" void WidgetPutImageTile(
	drawable_t tar_d, image_t *src_img,
	unsigned int tar_width, unsigned int tar_height
);
extern "C" void WidgetPutPixmapTile(
        drawable_t tar_d, pixmap_t src_pm,
        unsigned int tar_width, unsigned int tar_height,
	unsigned int src_width, unsigned int src_height
);

extern "C" void WidgetFrameButton(win_t w, bool_t state,
	unsigned long fg_pix, unsigned long bg_pix
);
extern "C" void WidgetFrameButtonPixmap(
	pixmap_t pixmap,
	bool_t state,
	unsigned int width, unsigned int height,
        unsigned long fg_pix, unsigned long bg_pix
);
extern "C" image_t *WidgetCreateImageText(
	char *string,
	font_t *font,
	unsigned int font_width, unsigned int font_height,
	pixel_t fg_pix,
	pixel_t bg_pix
);
pixmap_t WidgetPixmapMaskFromImage(image_t *image);
extern "C" void WidgetPutImageNormal(
	drawable_t d,		/* Target. */
	image_t *ximage,	/* Source. */
	int tar_x, int tar_y,
	bool_t allow_transparency
);
extern "C" void WidgetPutImageRaised(
        drawable_t d,		/* Target. */
        image_t *ximage,	/* Source. */
        int tar_x, int tar_y,
        unsigned int altitude
);
extern "C" void WidgetAdjustImageGamma(
        image_t *image,
        double r, double g, double b
);



/* *********************************************************************
 *
 *                    Widget Management Functions
 */

/* In timming.c */
extern "C" time_t MilliTime(void);
extern "C" time_t UTime(void);


/* In wbutton.c */
extern "C" void PBtnChangeLabel(
	push_button_struct *btn,
	unsigned int width,
	unsigned int height,
	const char *label,
        char label_align,
	image_t *image
);
extern "C" int PBtnInit(
        push_button_struct *btn,  
        win_t parent,
        int x, int y,  
        unsigned int width,
        unsigned int height,
        const char *label,
        char label_align,
	image_t *image,
	void *client_data,
        int (*func_cb)(void *)
);
extern "C" int PBtnSetHotKeys(
        push_button_struct *btn,
        char *hotkeys
);
extern "C" int PBtnSetHintMessage(
	push_button_struct *btn,
        const char *message
);
extern "C" int PBtnDraw(push_button_struct *btn);
extern "C" int PBtnManage(push_button_struct *btn, event_t *event);
extern "C" void PBtnMap(push_button_struct *btn);
extern "C" void PBtnUnmap(push_button_struct *btn);
extern "C" void PBtnDestroy(push_button_struct *btn);


/* In wclist.c */
extern "C" char *CListGetItemLabel(
        colum_list_struct *list,
        int row_num,
        int colum_num
);
extern "C" int CListSetItemLabel(
	colum_list_struct *list,
	int row_num,
	int colum_num,
	const char *label
);
extern "C" int CListSetRowLabels(
        colum_list_struct *list,
        int row_num,
        const char **label,
	int total_labels
);
extern "C" void *CListGetItemDataPtr(
        colum_list_struct *list,
        int row_num,
        int colum_num
);
extern "C" int CListSetItemDataPtr(
        colum_list_struct *list,
        int row_num,
        int colum_num,
        void *client_data
);

extern "C" int CListGetFirstSelectedRow(colum_list_struct *list);
extern "C" int CListGetLastSelectedRow(colum_list_struct *list);
extern "C" int CListIsRowSelected(
	colum_list_struct *list,
        int row_num
);
extern "C" int CListSelectRow(
        colum_list_struct *list,
        int row_num
);
extern "C" void CListUnselectAllRows(colum_list_struct *list);

extern "C" int CListAddHeading(
        colum_list_struct *list,
	const char *heading,
        font_t *font,   
        pixel_t pixel,
        unsigned int attr,
        int start_pos		/* In pixels. */
);
extern "C" void CListDeleteHeading(
        colum_list_struct *list,
        int colum_num
);
extern "C" int CListAddRow(
        colum_list_struct *list,
        int row_num		/* Can be -1 for append. */
);
extern "C" void CListDeleteRow(
        colum_list_struct *list,
	int row_num
);
extern "C" void CListDeleteAllRows(
	colum_list_struct *list
);
extern "C" int CListAddItems(
        colum_list_struct *list,
        const char **label,
        int total_labels,
        font_t *font,
        pixel_t pixel,
        unsigned int attr,
        int row_num
);
extern "C" int CListAddItem(
        colum_list_struct *list,
        const char *label,
        font_t *font,
        pixel_t pixel,
	unsigned int attr,
        int row_num
);
extern "C" void CListDeleteItem(
        colum_list_struct *list,
	int row_num, int colum_num
);

extern "C" int CListInit(
	colum_list_struct *list,
        win_t parent, 
        int x, int y,
        unsigned int width,
        unsigned int height,
        void *client_data,
        int (*func_cb)(void *)  
);
extern "C" void CListResize(colum_list_struct *list);
extern "C" void CListDraw(colum_list_struct *list, int amount);
extern "C" int CListManage(colum_list_struct *list, event_t *event);
extern "C" void CListMap(colum_list_struct *list);
extern "C" void CListUnmap(colum_list_struct *list);
extern "C" void CListDestroy(colum_list_struct *list);


/* In wprogressbar.c */
extern "C" int PBarInit(
	progress_bar_struct *pb,
        win_t parent,
        int x, int y,
        unsigned int width, unsigned int height,
        double start_val,
	double min, double max,
        char *label,
        int completion_action
);
extern "C" void PBarResize(progress_bar_struct *pb);
extern "C" int PBarDraw(progress_bar_struct *pb);
extern "C" int PBarManage(progress_bar_struct *pb, event_t *event);
extern "C" void PBarMap(progress_bar_struct *pb);
extern "C" void PBarUnmap(progress_bar_struct *pb);
extern "C" void PBarDestroy(progress_bar_struct *pb);

extern "C" int PBarUpdate(progress_bar_struct *pb, double value, char *label);




/* In wdialog.c */
extern "C" int DialogWinInit(
        dialog_win_struct *dw,
        win_t parent,
        unsigned int width,
        unsigned int height,
        image_t *icon
);
extern "C" int DialogWinDraw(     
        dialog_win_struct *dw,
        char *mesg
);
extern "C" int DialogWinManage(
        dialog_win_struct *dw, 
        event_t *event
);
extern "C" void DialogWinMap(dialog_win_struct *dw);
extern "C" void DialogWinUnmap(dialog_win_struct *dw);
extern "C" void DialogWinDestroy(
        dialog_win_struct *dw
);      
extern "C" int printdw(
        dialog_win_struct *dw,
        char *mesg  
);


/* In wfbrowser.c */
extern "C" char *GETCHILD(char *path);
extern "C" char *GETSELECTIONNAME(fbrowser_struct *fb);

extern "C" char *FBrowserGetPathMask(char *path);
extern "C" char *FBrowserGetJustPath(char *path);
extern "C" char *FBrowserGetFileSystemString(int fs_type);
extern "C" int FBrowserGetFileSystemType(char *fs_name);
extern "C" fb_object_struct *FBrowserGetSelObject(fbrowser_struct *fb);
extern "C" int FBrowserDoOK(fbrowser_struct *fb);
extern "C" int FBrowserApplyCVPrompt(fbrowser_struct *fb);
extern "C" int FBrowserChangeDir(
	fbrowser_struct *fb,
	char *path
);
extern "C" int FBrowserGetDeviceListing(fbrowser_struct *fb);
extern "C" int FBrowserRefreshList(fbrowser_struct *fb);
extern "C" int FBrowserSetOpMesg(
	fbrowser_struct *fb,
	char *title,
	char *ok_btn_name 
);

extern "C" int FBrowserDevicesPUListCB(void *ptr);
extern "C" int FBrowserMountPBCB(void *ptr);
extern "C" int FBrowserUnmountPBCB(void *ptr);

extern "C" int FBrowserOKPBCB(void *ptr);
extern "C" int FBrowserCancelPBCB(void *ptr);
extern "C" int FBrowserRefreshPBCB(void *ptr);

extern "C" int FBrowserInit(
        fbrowser_struct *fb,
        int x, int y,
        unsigned int width, unsigned int height,
	char *start_dir,
	int style,			/* One of FB_STYLE_* */
	int (*func_ok)(char *),
	int (*func_cancel)(char *)
);
extern "C" int FBrowserResize(fbrowser_struct *fb);
extern "C" int FBrowserDraw(fbrowser_struct *fb, int amount);
extern "C" int FBrowserManage(
        fbrowser_struct *fb,   
        event_t *event
);
extern "C" void FBrowserMapCVPrompt(fbrowser_struct *fb, int mode);
extern "C" void FBrowserUnmapCVPrompt(fbrowser_struct *fb);
extern "C" void FBrowserMap(fbrowser_struct *fb);
extern "C" void FBrowserMapPath(fbrowser_struct *fb, char *path);
extern "C" void FBrowserMapSearchMask(fbrowser_struct *fb, char *pattern);
extern "C" void FBrowserUnmap(fbrowser_struct *fb);
extern "C" void FBrowserDestroy(fbrowser_struct *fb);


/* In wpulist.c */
extern "C" int PUListIsItemAllocated(popup_list_struct *list, int n);
extern "C" char *PUListGetSelItemName(popup_list_struct *list);
extern "C" int PUListAddItem(
	popup_list_struct *list,
	char *name,
	bool_t disabled
);
extern "C" void PUListDeleteAllItems(popup_list_struct *list);

extern "C" int PUListInit(
	popup_list_struct *list,
	win_t parent,
	int x, int y,
	unsigned int width, unsigned int height,
	int popup_list_vis_items,
	int direction,
	void *client_data,
	int (*func_cb)(void *)
);
extern "C" int PUListResize(
	popup_list_struct *list,
	unsigned int width, unsigned int height,
	int popup_list_vis_items
);
extern "C" int PUListDraw(popup_list_struct *list, int amount);
extern "C" int PUListManage(popup_list_struct *list, event_t *event);
extern "C" void PUListMap(popup_list_struct *list);
extern "C" void PUListUnmap(popup_list_struct *list);
extern "C" void PUListMapList(popup_list_struct *list);
extern "C" void PUListUnmapList(popup_list_struct *list);
extern "C" void PUListDestroy(popup_list_struct *list);

extern "C" void PUListRepeatRecordSet(
	popup_list_struct *list,     
	long dsec, int op_code
);
extern "C" void PUListRepeatRecordClear(void);
extern "C" int PUListManageRepeat(event_t *event);


/* In wlist.c */
extern "C" int ListGetItemNumByDataPointer(
        list_window_struct *lw,
        void *data_ptr
);
extern "C" int ListIsItemAllocated(
	list_window_struct *lw,
	int n
);

extern "C" int ListAddItem(
        list_window_struct *lw,
        int type,  
        char *name,
        image_t *image,
        int pos,		/* -1 for append. */
        void *data_ptr
);
extern "C" void ListDeleteItem(list_window_struct *lw, int pos);
extern "C" void ListDeleteAllItems(list_window_struct *lw);

extern "C" int ListWinInit(
        list_window_struct *lw,
        win_t parent,
        int x, int y, 
        unsigned int width, unsigned int height
);
extern "C" int ListWinResize(list_window_struct *lw);
extern "C" int ListWinDraw(list_window_struct *lw);
extern "C" int ListWinManage(
        list_window_struct *lw,
        event_t *event
);
extern "C" int ListWinCVPromptApply(list_window_struct *lw);
extern "C" void ListWinMapCVPrompt(list_window_struct *lw, int mode);
extern "C" void ListWinUnmapCVPrompt(list_window_struct *lw);
extern "C" void ListWinMap(list_window_struct *lw);
extern "C" void ListWinUnmap(list_window_struct *lw);
extern "C" void ListWinDestroy(list_window_struct *lw);

extern "C" WCursor *ListWinCreateCursorFromEntry(
        list_window_struct *lw,
        int entry_num
);
extern "C" void ListWinDoDragStart(
        list_window_struct *lw,
        int entry_pos,          /* Drag started on/for this entry. */
        win_t start_w
);
extern "C" void ListWinDoDragStop(list_window_struct *lw);

extern "C" void ListWinRepeatRecordSet(
        list_window_struct *lw,
        long dsec, int op_code
);
extern "C" void ListWinRepeatRecordClear(void);
extern "C" int ListWinManageRepeat(event_t *event);


/* In wmenu.c. */
extern "C" int MenuIsItemAllocated(menu_struct *menu, int n);
extern "C" int MenuGetItemNumberByID(menu_struct *menu, int id_code);
extern "C" bool_t MenuGetItemState(menu_struct *menu, int n);
extern "C" void MenuSetItemState(menu_struct *menu, int n, bool_t state);
extern "C" void MenuSetItemAccelerator(
	menu_struct *menu,
	int n,
	char key,
	menu_item_flags_t flags
);

extern "C" int MenuAddItem(   
        menu_struct *menu,
        char *name,
        int type,
        image_t *icon,
        int id_code,
        int pos
);
extern "C" void MenuDeleteAllItems(menu_struct *menu);

extern "C" int MenuInit(
	menu_struct *menu,
	win_t parent,
	int (*func_cb)(void *, int),	/* Can be NULL.	*/
	void *client_data		/* Can be NULL. */
);
extern "C" int MenuDraw(menu_struct *menu);
extern "C" int MenuManage(
        menu_struct *menu,
	event_t *event
);
extern "C" int MenuClose(menu_struct *menu);
extern "C" void MenuMap(menu_struct *menu);
extern "C" void MenuMapPos(menu_struct *menu, int x, int y);
extern "C" void MenuUnmap(menu_struct *menu);
extern "C" void MenuDestroy(menu_struct *menu);


/* In wmenubar.c */
extern "C" int MenuBarIsItemAllocated(menu_bar_struct *mb, int n);
extern "C" menu_struct *MenuBarGetMenuFromItem(menu_bar_struct *mb, int n);
extern "C" bool_t MenuBarGetItemToggleState(
	menu_bar_struct *mb,
	int menu_num, int item_num
);
extern "C" void MenuBarSetItemToggleState(
        menu_bar_struct *mb,
        int menu_num, int item_num,
        bool_t state
);
extern "C" int MenuBarMatchItemByPos(
        menu_bar_struct *mb,
        int x, int y
);

extern "C" int MenuBarAddItem(
        menu_bar_struct *mb,
        int pos,                /* Can be -1 for append. */
        char *name,
        int x, int y,
        unsigned int width, unsigned int height
);
extern "C" int MenuBarAddItemMenuItem(
        menu_bar_struct *mb,
        int n,                  /* Menu bar item number, must be valid */
        char *name,
        int type,
        image_t *icon,          /* Shared. */
        int id_code,
        int pos                 /* Can be -1 for append. */
);
extern "C" void MenuBarDeleteItem(menu_bar_struct *mb, int n);

extern "C" int MenuBarInit(
        menu_bar_struct *mb,
        win_t parent,
        int x, int y,
        unsigned int width, unsigned int height,
        int (*func_cb)(void *, int),
        void *client_data
);
extern "C" int MenuBarResize(menu_bar_struct *mb);
extern "C" int MenuBarDraw(menu_bar_struct *mb);
extern "C" int MenuBarManage(menu_bar_struct *mb, event_t *event);
extern "C" void MenuBarMap(menu_bar_struct *mb);
extern "C" void MenuBarUnmap(menu_bar_struct *mb);
extern "C" void MenuBarDestroy(menu_bar_struct *mb);


/* In wprompt.c */
#define PROMPT_POS_CUR		0
#define PROMPT_POS_START	1
#define PROMPT_POS_END		2
extern "C" int PromptMarkBuffer(prompt_window_struct *prompt, int opt);
extern "C" void PromptUnmarkBuffer(prompt_window_struct *prompt, int opt);

extern "C" double PromptGetF(prompt_window_struct *prompt);
extern "C" int PromptGetI(prompt_window_struct *prompt);
extern "C" long PromptGetL(prompt_window_struct *prompt);
extern "C" char *PromptGetS(prompt_window_struct *prompt);
extern "C" void PromptSetF(prompt_window_struct *prompt, double val);
extern "C" void PromptSetI(prompt_window_struct *prompt, int val);
extern "C" void PromptSetL(prompt_window_struct *prompt, long val);
extern "C" void PromptSetUL(prompt_window_struct *prompt, unsigned long val);
extern "C" void PromptSetS(prompt_window_struct *prompt, const char *val);

extern "C" void PromptRepeatRecordSet(
	prompt_window_struct *prompt,
        long dsec, int op_code
);
extern "C" void PromptRepeatRecordClear(void);
extern "C" void PromptSetNotifyFunction(int (*func_notify)(prompt_window_struct *));

extern "C" int PromptInit(
        prompt_window_struct *prompt,
        win_t parent,
        int x, int y,
        unsigned int width, unsigned int height,
	int style,
        const char *name,
        unsigned int buf_len,
        int hist_bufs,
	int (*func_cb)(char *)
);
extern "C" void PromptChangeName(prompt_window_struct *prompt, char *name);
extern "C" void PROMPT_UNMARK_REDRAW_ALL(prompt_window_struct *except_prompt);
extern "C" void PROMPT_UNFOCUS_REDRAW_ALL(prompt_window_struct *except_prompt);
extern "C" int PromptDraw(prompt_window_struct *prompt, int amount);
extern "C" int PromptManage(prompt_window_struct *prompt, event_t *event);
extern "C" void PromptMap(prompt_window_struct *prompt);
extern "C" void PromptClose(prompt_window_struct *prompt);
extern "C" void PromptUnmap(prompt_window_struct *prompt);
extern "C" void PromptDestroy(prompt_window_struct *prompt);

extern "C" int PromptManageRepeat(event_t *event);


/* In wpstepper.c */
extern "C" int PStepperIsPageAllocated(
	page_stepper_struct *ps,
	int page
);
extern "C" int PStepperGetWidgetPointer(
        page_stepper_struct *ps,
        char *name,
        void **w_ptr,
        int *w_type   
);
extern "C" double PStepperGetWidgetValue(
        page_stepper_struct *ps,
        char *name
);
extern "C" void *PStepperGetWidgetValuePtr(
        page_stepper_struct *ps,
        char *name
);

extern "C" int PStepperPBCB(void *btn);
extern "C" int PStepperPrevPBCB(void *ptr);
extern "C" int PStepperNextPBCB(void *ptr);

extern "C" int PStepperInit(
	page_stepper_struct *ps,
	win_t parent,
        int x, int y,
        unsigned int width, unsigned int height,
	unsigned int total_pages,
	image_t *panel_img,
	void *client_data,
	int (*page_change_handler)(void *, int, int),
	int (*exit_handler)(void *),
	int (*finish_handler)(void *)
);

extern "C" int PStepperDraw(
	page_stepper_struct *ps,
	int amount
);

extern "C" int PStepperManage(
	page_stepper_struct *ps,
	event_t *event
);

extern "C" void PStepperMap(page_stepper_struct *ps);
extern "C" void PStepperUnmap(page_stepper_struct *ps);

extern "C" void PStepperMapPage(page_stepper_struct *ps, int page);
extern "C" void PStepperUnmapPage(page_stepper_struct *ps, int page);

extern "C" void PStepperDestroy(page_stepper_struct *ps);

extern "C" int PStepperAllocatePage(page_stepper_struct *ps);
extern "C" void PStepperDestroyPage(
	page_stepper_struct *ps,
	int page
);

extern "C" int PStepperAllocateLabel(
        page_stepper_struct *ps,
        int page,
        char *text,
	image_t *image,
        font_t *font,
        pixel_t pix,
        int x, int y
);

extern "C" int PStepperAllocateWidget(
	page_stepper_struct *ps,
        int page,
	char *name,
	int w_type,
	int x, int y,
	unsigned int width, unsigned int height,
	int argc, char *argv[]
);
extern "C" void PStepperDestroyWidget(page_widget_struct *pw);


/* In wscalebar.c */
extern "C" int ScaleBarInit(
	scale_bar_struct *sb,
	win_t parent,
	int x, int y,
	unsigned int length,
	int style,
	int ticks,
	int orientation,
	double pos_min, double pos_max, double pos,
	bool_t flip_pos,
        void *client_data,
        int (*func_cb)(void *)
);
extern "C" int ScaleBarDraw(scale_bar_struct *sb);
extern "C" int ScaleBarManage(
        scale_bar_struct *sb,
	event_t *event
);
extern "C" void ScaleBarMap(scale_bar_struct *sb);
extern "C" void ScaleBarUnmap(scale_bar_struct *sb);
extern "C" void ScaleBarDestroy(scale_bar_struct *sb);


/* In wscrollbar.c */
extern "C" void SBarRepeatRecordSet(
	scroll_bar_struct *sb,
        long dsec, int op_code,
	int width, int height,
	int max_width, int max_height
);
extern "C" void SBarRepeatRecordClear(void);
extern "C" void SBarSetNotifyFunction(int (*func_notify)(scroll_bar_struct *));

extern "C" int SBarInit(
	scroll_bar_struct *sb,
	win_t parent,
        unsigned int width, unsigned int height
);
extern "C" int SBarResize(
	scroll_bar_struct *sb,
	unsigned int width,
	unsigned int height
);
extern "C" int SBarDraw(
        scroll_bar_struct *sb,
        int width, int height,
        int max_width, int max_height
);
extern "C" int SBarManage(
        scroll_bar_struct *sb,          /* Scrollbar return. */
        int width, int height,          /* Visibile size of window. */
        int max_width, int max_height,  /* Total size of window. */
	event_t *event
);
extern "C" int SBarManageRepeat(event_t *event);
extern "C" void SBarDestroy(scroll_bar_struct *sb);


/* In wtogglearray.c */
extern "C" int TgBtnArrayInit(
	toggle_button_array_struct *tba,
	win_t parent,
	int x, int y,
	unsigned int nbtns,
	int start_sel_btn,
	char **names,
	unsigned int nnames,
	int alignment
);
extern "C" int TgBtnSetHintMessage(
	toggle_button_struct *tb,
	char *message
);
extern "C" int TgBtnArrayManage(
	toggle_button_array_struct *tba,
	event_t *event
);
extern "C" int TgBtnArrayDraw(
	toggle_button_array_struct *tba
);
extern "C" void TgBtnArrayMap(toggle_button_array_struct *tba);
extern "C" void TgBtnArrayUnmap(toggle_button_array_struct *tba);
extern "C" void TgBtnArrayDestroy(toggle_button_array_struct *tba);


/* In wtogglebtn.c */
extern "C" int TgBtnInit(
        toggle_button_struct *tb,
	win_t parent,
        int x, int y,
        bool_t state,
        char *label 
);
extern "C" int TgBtnDraw(toggle_button_struct *tb, int amount);
extern "C" int TgBtnManage(toggle_button_struct *tb, event_t *event);
extern "C" void TgBtnMap(toggle_button_struct *tb);
extern "C" void TgBtnUnmap(toggle_button_struct *tb);
extern "C" void TgBtnDestroy(toggle_button_struct *tb);


/* In wviewer.c */
extern "C" int ViewerLoadFile(  
        file_viewer_struct *fv,
        char *filename
);
extern "C" int ViewerLoadData(
        file_viewer_struct *fv,
        void *buf,
        int buf_len
);
extern "C" void ViewerUnload(file_viewer_struct *fv);

extern "C" int ViewerInit(
        file_viewer_struct *fv,
        int x, int y,
        unsigned int width, unsigned int height
);
extern "C" int ViewerResize(file_viewer_struct *fv);
extern "C" int ViewerDraw(file_viewer_struct *fv);
extern "C" int ViewerManage(file_viewer_struct *fv, event_t *event);
extern "C" void ViewerMap(file_viewer_struct *fv);
extern "C" void ViewerUnmap(file_viewer_struct *fv);
extern "C" void ViewerDestroy(file_viewer_struct *fv);


/* In wreg.c */
extern "C" void WidgetRegReclaim(void);
extern "C" void WidgetRegDeleteAll(void);

extern "C" void *WidgetRegIsRegistered(void *ptr, int type);

extern "C" int WidgetRegAdd(void *ptr, int type);
extern "C" void WidgetRegDelete(void *ptr);



#endif /* WIDGETS_H */
