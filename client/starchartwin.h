/*
                            Starchart Window
 */

#ifndef STARCHARTWIN_H
#define STARCHARTWIN_H

#include "../include/osw-x.h"
#include "../include/widget.h"

#include "../include/objects.h"


/*
 *	Draw amount codes:
 */
#define SCHT_DRAW_AMOUNT_COMPLETE	0
#define SCHT_DRAW_AMOUNT_VIEW		1


/*
 *	Starchart window structure:
 */
typedef struct {

	char map_state;
	int x, y;
	unsigned int width, height;
	char is_in_focus;
	visibility_t visibility_state;
	bool_t disabled;

	win_t toplevel;

        menu_bar_struct mb;

	win_t view;
	pixmap_t view_buf;

	menu_struct qmenu;	/* Quick menu on viewer. */

	prompt_window_struct	filter_prompt,
				x_prompt,
				y_prompt,
				sect_x_prompt,
				sect_y_prompt;

	double zoom;

	toggle_button_struct	follow_player_tb;

	push_button_struct	zoom_in_btn,
				zoom_out_btn,
				jump_to_player_btn;

	progress_bar_struct	pbar;

	void *ref_win;		/* Pointer to bridge window. */

	unv_head_struct header;	/* Header for universe file. */
	xsw_object_struct **object;
	int total_objects;
	int selected_object;

	/* Measuring (in units of pixels). */
	bool_t	measuring;
	int	measure_start_x, measure_start_y,
		measure_end_x, measure_end_y;

} starchart_win_struct;
extern "C" starchart_win_struct starchart_win;


extern "C" int SChtMatchObjectPosition(
        starchart_win_struct *cht,
        int x, int y
);
extern "C" void SChtUpdateFilter(
	starchart_win_struct *cht,
	char *filter
);
extern "C" int SChtMenuCB(void *data, int op_code);
extern "C" int SChtZoomInBtnCB(void *data);
extern "C" int SChtZoomOutBtnCB(void *data);
extern "C" int SChtJumpToPlayerBtnCB(void *data);
extern "C" int SChtClearCB(void *data);
extern "C" void SChtLoadProgressCB(void *data, int n, int m);
extern "C" void SChtOverlayChartCB(
	starchart_win_struct *cht,
	char *path
);
extern "C" void SChtSaveChartCB(
        starchart_win_struct *cht,
        char *path
);
extern "C" void SChtTimeoutCB(starchart_win_struct *cht);

extern "C" int SChtAddObject(
        starchart_win_struct *cht,
        int object_num,
        int type, int isref_num, long size,
        long sect_x, long sect_y, long sect_z,
        double x, double y, double z,
        double heading, double pitch, double bank
);
extern "C" int SChtSetObjectName(
        starchart_win_struct *cht,
        char *name
);
extern "C" int SChtSetObjectEmpire(
        starchart_win_struct *cht,
        char *empire
);
extern "C" int SChtRecycleObject(
        starchart_win_struct *cht,
        int object_num
);

extern "C" int SChtInit(starchart_win_struct *cht);
extern "C" void SChtResize(starchart_win_struct *cht);
extern "C" void SChtDraw(starchart_win_struct *cht, int amount);
extern "C" int SChtManage(starchart_win_struct *cht, event_t *event);
extern "C" void SChtMap(starchart_win_struct *cht);
extern "C" void SChtUnmap(starchart_win_struct *cht);
extern "C" void SChtDestroy(starchart_win_struct *cht);



#endif	/* STARCHARTWIN_H */
