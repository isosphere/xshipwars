#ifndef MESGWIN_H
#define MESGWIN_H

        
#include "../include/osw-x.h"
#include "../include/widget.h"

typedef struct {

	int mesg_len;	/* Length of allocation, not current. */
	char *mesg;

	int sel_start, sel_end;

	pixel_t pixel;

} mesgwin_mesg_struct;


typedef struct {

	char map_state;
        char is_in_focus;
        int x, y;
        unsigned int width, height;
        int visibility_state;

        win_t toplevel;
        pixmap_t toplevel_buf;

	scroll_bar_struct sb;

	shared_image_t *bkg;

	mesgwin_mesg_struct **message;
	int total_messages;

	int longest_line;	/* Length of longest line. */

} mesgwin_struct;



extern int MesgWinAdd(
        mesgwin_struct *mw,
        char *string,
        pixel_t color
);

extern int MesgWinInit(mesgwin_struct *mw, int argc, char *argv[]);
extern void MesgWinResize(mesgwin_struct *mw);
extern void MesgWinDraw(mesgwin_struct *mw);
extern int MesgWinManage(mesgwin_struct *mw, event_t *event);
extern void MesgWinMap(mesgwin_struct *mw);
extern void MesgWinUnmap(mesgwin_struct *mw);
extern void MesgWinDestroy(mesgwin_struct *mw);




#endif	/* MESGWIN_H */
