// unvedit/aboutwin.h

#ifndef ABOUTWIN_H
#define ABOUTWIN_H

#include "../include/osw-x.h"
#include "../include/widget.h"


#define AW_COPYRIGHT_MESG	\
"Copyright (C) 1997-2001 WolfPack Entertainment."

#define AW_URL_MESG		\
"http://wolfpack.twu.net/ShipWars/"

#define AW_LICENSE_MESG		\
"Distribute in accordance with the GNU Public License."


#define AW_TITLE	"About Universe Editor"
#define AW_ICON_TITLE	"About"


#define AW_WIDTH	380
#define AW_HEIGHT	280

#define AW_BUTTON_WIDTH		80
#define AW_BUTTON_HEIGHT	28

#define AW_DRAW_AMOUNT_COMPLETE		0


/*
 *	About window structure.
 */
typedef struct {

	char map_state;
	char is_in_focus;
	int x, y;
	unsigned int width, height;
	font_t *font;

	win_t toplevel;
	pixmap_t toplevel_buf;

	push_button_struct dismiss_btn;

} about_win_struct;
extern about_win_struct about_win;


extern int AboutWinDismissPBCB(void *ptr);

extern int AboutWinInit();
extern void AboutWinDraw(int amount);
extern int AboutWinManage(event_t *event);
extern void AboutWinMap();
extern void AboutWinUnmap();
extern void AboutWinDestroy();


#endif	/* ABOUTWIN_H */
