// unvedit/comfwin.h

/*
                          Comfirmation dialog

 */

#ifndef COMFWIN_H
#define COMFWIN_H

#include <stdio.h>

#include "../include/osw-x.h"
#include "../include/widget.h"



#define CW_BUTTON_WIDTH		70
#define CW_BUTTON_HEIGHT	28


/*
 *	Comfirmation options.
 */
#define ComfirmOptionAll	(1 << 1)	/* Show all button. */
#define ComfirmOptionCancel	(1 << 2)	/* Show help button. */

/*
 *	Comfermation codes.
 */
#define ComfirmCodeError	-1
#define ComfirmCodeNo		0
#define ComfirmCodeYes		1
#define ComfirmCodeAll		2
#define ComfirmCodeCancel	3



typedef struct {

	char map_state;
	char is_in_focus;
	int x, y;
	unsigned int width, height;

	unsigned long option;

	win_t toplevel;
	pixmap_t toplevel_buf;

	image_t *icon;
	char *mesg;

	push_button_struct	no_btn,
				yes_btn,
				all_btn,
				cancel_btn;

	void *client_data;
	int (*std_gui_manage_func)(event_t *);

} comfirm_win_struct;

extern int comf_last_result;

/* comfwin.c */
extern int ComfWinDoQuery(
	comfirm_win_struct *cw,
        char *mesg
);

extern int ComfWinInit(
	comfirm_win_struct *cw,
	image_t *icon,
	void *client_data,
	int (*std_gui_manage_func)(event_t *)
);
extern void ComfWinDraw(comfirm_win_struct *cw);
extern int ComfWinManage(comfirm_win_struct *cw, event_t *event);
extern void ComfWinMap(comfirm_win_struct *cw);
extern void ComfWinMapMesg(comfirm_win_struct *cw, char *mesg);
extern void ComfWinUnmap(comfirm_win_struct *cw);
extern void ComfWinDestroy(comfirm_win_struct *cw);



#endif	/* COMFWIN_H */
