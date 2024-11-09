/*
                           Print Window

 */

#ifndef PRINTWIN_H
#define PRINTWIN_H


#include "../include/osw-x.h"
#include "../include/widget.h"

#include "uew.h"


/*
 *	Title:
 */
#define PW_DEF_TITLE		"Print"
#define PW_DEF_ICON_TITLE	"Print"

/*
 *	Sizes:
 */
#define PW_WIDTH		600
#define PW_HEIGHT		480

#define PW_BUTTON_WIDTH		70
#define PW_BUTTON_HEIGHT	28

#define PW_PROMPT_HEIGHT	30


/*
 *	Misc defaults:
 */
#define PW_DEF_PRINT_CMD	"lpr %file"
#define PW_DEF_SPOOL_DIR	"/tmp"

/*
 *	Draw amount codes:
 */
#define PW_DRAW_AMOUNT_COMPLETE		0
#define PW_DRAW_AMOUNT_PREVIEW		1


/*
 *	Units codes:
 */
#define PW_UNITS_PIXELS		0
#define PW_UNITS_INCH		1
#define PW_UNITS_CM		2

typedef struct {

	char map_state;
	char is_in_focus;
	int x, y;
	unsigned int width, height;

	int units;

	win_t toplevel;
	pixmap_t toplevel_buf;

	win_t preview;
	image_t *preview_img;

	pixel_t		paper_bg_pix,
			paper_fg_pix;

	toggle_button_array_struct	print_to_tba;

	prompt_window_struct	print_cmd_prompt,
				spool_dir;

	popup_list_struct	objects_pulist;

	prompt_window_struct	x_prompt,
				y_prompt,
				width_prompt,
				height_prompt;

	toggle_button_struct	obj_labels_tb,
				label_geometry_tb;


        toggle_button_array_struct      paper_size_tba,
					color_mode_tba,
					orientation_tba;

        push_button_struct      print_btn, 
                                cancel_btn;

	/* Pointer to source uew. */
	void *src;

} print_win_struct;
extern print_win_struct print_win;


/* In printwin.c */
extern image_t *PrintWinRotateImage(image_t *image, depth_t d);
extern image_t *PrintWinCreateSpool(print_win_struct *pw);
extern int PrintWinDoPrint(print_win_struct *pw);

extern int PrintWinCancelPBCB(void *ptr);
extern int PrintWinPrintPBCB(void *ptr);

extern int PrintWinInit();
extern int PrintWinDraw(int amount);
extern int PrintWinManage(event_t *event);
extern void PrintWinMap();
extern void PrintWinDoMapValues(uew_struct *src_uew);
extern void PrintWinUnmap();
extern void PrintWinDestroy();

#endif	/* PRINTWIN */
