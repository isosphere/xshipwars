/*
                     Options Window Definations
 */

#ifndef OPTWIN_H
#define OPTWIN_H

#include "../include/osw-x.h"
#include "../include/widget.h"


/*
 *    Primary File browser mode codes (for options window functions only).
 */
#define FB_MODE_CODE_ISRefs     1
#define FB_MODE_CODE_OCSNs      2
#define FB_MODE_CODE_SS         3       /* Sound schemes. */
#define FB_MODE_CODE_JSCalib    4

/* The length, in bytes, of a string containing a numbered value. */
#ifndef NUM_STR_LEN
# define NUM_STR_LEN 24
#endif


/* Options window button sizes (in pixels). */   
#define OPTWIN_BTN_WIDTH        70
#define OPTWIN_BTN_HEIGHT       28
  
/* General margin. */
#define OPTWIN_MARGIN           16

/* Options window tab codes. */
#define OPTWIN_TAB_CODE_GENERAL         0
#define OPTWIN_TAB_CODE_GRAPHICS        1
#define OPTWIN_TAB_CODE_SOUNDS          2
#define OPTWIN_TAB_CODE_NET             3
#define OPTWIN_TAB_CODE_MISC            4

/* Default width and height of options window. */
#define OPTWIN_DEF_WIDTH                640
#define OPTWIN_DEF_HEIGHT               480

        
/* Character sizes (in pixels). */
#define OPTWIN_CHAR_WIDTH       7
#define OPTWIN_CHAR_HEIGHT      14


/* Name position in drawn outlines. */
#define OPTWIN_OUTLINE_NAME_POS        0.10    /* Coefficient. */


/*
 *	Hint messages:
 */
#define OPTWIN_MAP_KEYBOARD_HINT	"Configure keyboard operations"
#ifdef JS_SUPPORT
# define OPTWIN_MAP_JOYSTICK_HINT	"Configure joystick operations"
#endif	/* JS_SUPPORT */

#define OPTWIN_AUTO_ZOOM_HINT		"Automatically adjust viewscreen zoom"
#define OPTWIN_LOCAL_UPDATES_HINT	"Have client derive object movements"
#define OPTWIN_SCANNER_CONTACTS_HINT	"Notify about objects entering and leaving scanner"


#define OPTWIN_PASSIVE_IMAGE_LOADING_HINT	"Enables loading of graphics asynchronously"

#define OPTWIN_SHOW_SERVER_ERRORS_HINT	"Display message dialogs from server"
#define OPTWIN_SHOW_NET_ERRORS_HINT	"Print segments of unparseable (error) data"

#define OPTWIN_AUTO_INTERVAL_HINT	"Automatically adjust network streaming interval"


/*
 *	Options window structure:
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

	/* Buttons. */
	push_button_struct	ok_btn,
				cancel_btn,
				apply_btn,
				defaults_btn,
				save_btn;

	int tab;	/* Which tab is selected. */

	bool_t has_modifications;

	/* Tabs. */
	win_t	general_tab,
		graphics_tab,
		sounds_tab,
		network_tab,
		misc_tab;
	/* Tabs buffer. */
	pixmap_t	tab_win_buf;


	/* General. */
	popup_list_struct		control_type_pul;
	toggle_button_array_struct	throttle_mode_tba;
	toggle_button_struct		auto_zoom_tb;
	toggle_button_struct		local_updates_tb;
	toggle_button_struct		notify_scanner_contacts_tb;

	prompt_window_struct		isref_name_prompt;
	prompt_window_struct		ocs_name_prompt;
	prompt_window_struct		ss_name_prompt;

	push_button_struct		isref_name_browse_btn;
	push_button_struct		ocs_name_browse_btn;
	push_button_struct		ss_name_browse_btn;

#ifdef JS_SUPPORT
	push_button_struct		jsmap_btn;
#endif	/* JS_SUPPORT */
        push_button_struct              keymap_btn;

	/* Graphics. */
	toggle_button_struct		show_viewscreen_marks_tb;
	toggle_button_array_struct	show_viewscreen_labels_tba;
	toggle_button_array_struct	show_formal_label_tba;
	toggle_button_struct		async_image_loading_tb;
	prompt_window_struct		async_image_pixels_prompt;

	/* Sounds. */
	toggle_button_array_struct	sounds_tba;
	toggle_button_struct		music_tb;
	toggle_button_array_struct	server_type_tba;
	prompt_window_struct		sound_server_prompt;
	prompt_window_struct		sound_con_arg_prompt;
	push_button_struct		sound_test_btn;
	toggle_button_struct		flip_stereo_tb;

	/* Net. */
	prompt_window_struct		def_address_prompt;
	prompt_window_struct		def_port_prompt;
	toggle_button_struct		show_net_errors_tb;
	toggle_button_struct		show_server_errors_tb;
	prompt_window_struct		max_net_load_prompt;
	prompt_window_struct		net_int_prompt;
	toggle_button_struct		auto_interval_tb;

	/* Misc. */
	prompt_window_struct		xsw_local_toplevel_path_prompt;
	prompt_window_struct		xsw_toplevel_path_prompt;
	prompt_window_struct		xsw_etc_path_prompt;  
	prompt_window_struct		xsw_images_path_prompt;
	prompt_window_struct		xsw_sounds_path_prompt;
        prompt_window_struct            xsw_downloads_path_prompt;

#ifdef JS_SUPPORT
	prompt_window_struct		js_calib_path_prompt;
	push_button_struct		js_calib_path_browse_btn;
#endif /* JS_SUPPORT */
	push_button_struct		refresh_memory_btn;
	popup_list_struct		units_pul;
    
} xsw_options_win_struct;
extern xsw_options_win_struct options_win;




#endif /* OPTWIN_H */
