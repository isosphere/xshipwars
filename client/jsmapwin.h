/*
                         Joystick Mapping Window
 */

#ifdef JS_SUPPORT

#ifndef JSMAPWIN_H
#define JSMAPWIN_H

#include "../include/osw-x.h"
#include "../include/widget.h"


#define JSMW_DEF_WIDTH		400
#define JSMW_DEF_HEIGHT		550

#define JSMW_BTN_WIDTH		70
#define JSMW_BTN_HEIGHT		28

#define JSMW_MARGIN		10
#define JSMW_PROMPT_HEIGHT	30

#define JSMW_CHAR_WIDTH		7
#define JSMW_CHAR_HEIGHT	14

#define JSMW_SCANKEY_WIN_HEIGHT	((4 * 16) + (2 * JSMW_MARGIN))

#define JSMW_JS_LIST_HEIGHT	120
#define JSMW_BUTTON_LIST_HEIGHT	150

/*
 *	Draw amount codes:
 */
#define JSMW_DRAW_AMOUNT_COMPLETE	0
#define JSMW_DRAW_AMOUNT_SCANKEY	1

/*
 *	Joystick mapping window structure:
 */
typedef struct {

	char map_state;
	int x, y;
	unsigned int width, height;
	char is_in_focus;
	visibility_t visibility_state;
	bool_t disabled;

	win_t toplevel;
	pixmap_t toplevel_buf;


	colum_list_struct	js_list;

	push_button_struct	add_btn,
				remove_btn;


	prompt_window_struct	device_prompt;
	push_button_struct	initialize_btn;

	prompt_window_struct	turn_axis_prompt,
				throttle_axis_prompt,
				thrust_dir_axis_prompt,
				vs_zoom_axis_prompt,
				scanner_zoom_axis_prompt,
				aim_weapon_heading_prompt;

	colum_list_struct	buttons_list;
	push_button_struct	scan_key_btn;
	bool_t			scanning_key;
	win_t			scan_key_win;

	push_button_struct	ok_btn, apply_btn, cancel_btn;
    
} xsw_jsmap_win_struct;
extern xsw_jsmap_win_struct jsmap_win;


/*
 *	Functions:
 */
extern "C" int JSMWLoadAll();
extern "C" int JSMWApplyAll();

extern "C" int JSMWDoSelectJoystick(int n);
extern "C" int JSMWDoApplyJoystick(int n);

extern "C" int JSMWAddPBCB(void *ptr);
extern "C" int JSMWRemovePBCB(void *ptr);
extern "C" int JSMWInitPBCB(void *ptr);

extern "C" int JSMWJoystickListCB(void *ptr);
extern "C" int JSMWButtonsListCB(void *ptr);
extern "C" int JSMWScanKeyPBCB(void *ptr);

extern "C" int JSMWOKPBCB(void *ptr);
extern "C" int JSMWApplyPBCB(void *ptr);
extern "C" int JSMWCancelPBCB(void *ptr);

extern "C" int JSMWInit();
extern "C" void JSMWResize();
extern "C" int JSMWDraw(int amount);
extern "C" int JSMWManage(event_t *event);
extern "C" void JSMWMap();
extern "C" void JSMWMapValues();
extern "C" void JSMWUnmap();
extern "C" void JSMWDestroy();



#endif	/* JSMAPWIN_H */

#endif	/* JS_SUPPORT */

