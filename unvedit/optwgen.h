/*
                       General options window
 */

#ifndef OPTWGEN_H
#define OPTWGEN_H

#include "../include/osw-x.h"
#include "../include/widget.h"



/*
 *      Default title strings:
 */
#define OPTWGEN_DEF_TITLE	"General Options"
#define OPTWGEN_DEF_ICON_TITLE	"Options"


#define OPTWGEN_WIDTH		640
#define OPTWGEN_HEIGHT		480

#define OPTWGEN_PROMPT_HEIGHT	30

#define OPTWGEN_BUTTON_WIDTH	70
#define OPTWGEN_BUTTON_HEIGHT	28


#define OPTWGEN_OUTLINE_NAME_POS	0.10    /* Coefficient. */

#define OPTWGEN_CHAR_WIDTH	7
#define OPTWGEN_CHAR_HEIGHT	14


/*
 *	Draw amount codes:
 */
#define OPTWGEN_DRAW_AMOUNT_COMPLETE	0
#define OPTWGEN_DRAW_AMOUNT_TABS	1
#define OPTWGEN_DRAW_AMOUNT_SCANKEY	2

/*
 *	Tab codes:
 */
#define OPTWGEN_TAB_APPERANCE	0
#define OPTWGEN_TAB_FONTS	1
#define OPTWGEN_TAB_IMAGES	2
#define OPTWGEN_TAB_PATHS	3
#define OPTWGEN_TAB_KEYMAP	4

typedef struct {

        char map_state;
        char is_in_focus;
        int x, y;
        unsigned int width, height;

	bool_t has_changes;

	int tab;

        pixmap_t toplevel_buf;
        win_t toplevel;

	push_button_struct	ok_btn,
				apply_btn,
				cancel_btn;

	win_t	apperance_tab_win,
		fonts_tab_win,
		images_tab_win,
		paths_tab_win,
		keymap_tab_win;

	/* On apperance tab. */
	toggle_button_struct	label_geometry_tb;
	toggle_button_struct	show_grid_tb;
	prompt_window_struct	grid_spacing_prompt;

        /* On fonts tab. */
	prompt_window_struct	view_font_prompt;
	prompt_window_struct	view_object_label_prompt;

        /* On images tab. */
        toggle_button_struct    show_preview_image_tb;
	toggle_button_struct	animate_image_tb;

        /* On paths tab. */
	prompt_window_struct	toplevel_path_prompt;
	prompt_window_struct	images_path_prompt;
	prompt_window_struct	server_path_prompt;

	/* Keymaps tab. */
	colum_list_struct	keymaps_list;
	push_button_struct	scan_key_btn;
	win_t			scan_key_win;	/* Prompts for keypress. */
	bool_t			scanning_key;
	pixel_t			scan_key_fg_pix,
				scan_key_bg_pix;

} optwgen_struct;

extern optwgen_struct optwgen;


extern int OptWGenDoSwitchTab(int tab_num);

extern int OptWGenFetchValues();
extern int OptWGenApplyValues();

extern int OptWGenKeymapListDoSet(keycode_t keycode);

extern int OptWGenKeymapListCB(void *ptr);
extern int OptWGenScanKeyPBCB(void *ptr);
extern int OptWGenOKPBCB(void *ptr);
extern int OptWGenApplyPBCB(void *ptr);
extern int OptWGenCancelPBCB(void *ptr);

extern int OptWGenInit();
extern int OptWGenDraw(int amount);
extern int OptWGenManage(event_t *event);
extern void OptWGenMap();
extern void OptWGenDoMapValues();
extern void OptWGenUnmap();
extern void OptWGenDestroy();


#endif	/* WEPW_H */
