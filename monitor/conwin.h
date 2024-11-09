#ifndef CONWIN_H
#define CONWIN_H


#include "../include/osw-x.h"
#include "../include/widget.h"


typedef struct {

	char map_state;
	char is_in_focus;
	int x, y;
	unsigned int width, height;
	visibility_t visibility_state;

	win_t toplevel;
	pixmap_t toplevel_buf;

	prompt_window_struct	address_prompt,
				port_prompt,
				name_prompt,
				password_prompt;

	push_button_struct	connect_btn,
				cancel_btn;

	void *client_data;
	int (*func_cb)(void *, char *, char *, char *, char *);

} con_win_struct;


extern int ConWinConnectPBCB(void *ptr);
extern int ConWinCancelPBCB(void *ptr);

extern int ConWinInit(
	con_win_struct *cw,
	void *client_data,
	int (*func_cb)(void *, char *, char *, char *, char *)
);
extern void ConWinDraw(con_win_struct *cw);
extern int ConWinManage(con_win_struct *cw, event_t *event);
extern void ConWinMap(con_win_struct *cw);
extern void ConWinUnmap(con_win_struct *cw);
extern void ConWinDestroy(con_win_struct *cw);


#endif	/* CONWIN_H */
