#ifndef KEYMAPWIN_H
#define KEYMAPWIN_H

#include "../include/osw-x.h"
#include "../include/widget.h"



/*
 *	Keymap window structure:
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

	colum_list_struct list;
    
	/* Keymap item description window. */
	win_t desc;
	pixmap_t desc_buf;

	/* Popup window for querying KeyPress scan. */
	win_t key_prompt_win;
	pixmap_t key_prompt_buf;
	bool_t key_prompt_mode;	/* True if in this mode. */

	push_button_struct	ok_btn,
				cancel_btn,
				apply_btn,

				scan_key_btn,
				default_btn,
				default_all_btn;

} xsw_keymap_win_struct;
extern xsw_keymap_win_struct keymap_win;


/* In keymapwin.c */
extern int KeymapWinLoad(void);
extern int KeymapWinApply(void);
extern int KeymapWinSetDefault(int item_num);
extern void KeymapWinPromptKey(void);
extern int KeymapWinCheckDups(bool_t warn);

extern int KeymapWinListCB(void *ptr);
extern int KeymapWinOkPBCB(void *ptr);
extern int KeymapWinCancelPBCB(void *ptr);
extern int KeymapWinApplyPBCB(void *ptr);
extern int KeymapWinScanKeyPBCB(void *ptr);
extern int KeymapWinDefaultPBCB(void *ptr);
extern int KeymapWinDefaultAllPBCB(void *ptr);

extern int KeymapWinInit(void);
extern void KeymapWinResize(void);
extern int KeymapWinDraw(int amount);
extern int KeymapWinManage(event_t *event);
extern void KeymapWinMap(void);
extern void KeymapWinDoMapValues(void);
extern void KeymapWinUnmap(void);
extern void KeymapWinDestroy(void);


#endif	/* KEYMAPWIN_H */
