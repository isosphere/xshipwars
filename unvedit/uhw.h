// unvedit/uhw.h

/*
                        Universe Header Window


 */

#ifndef UHW_H
#define UHW_H

#include "../include/osw-x.h"
#include "../include/widget.h"

#include "../include/objects.h"


/*
 *      Default title strings:
 */
#define UHW_DEF_TITLE		"Universe Header"
#define UHW_DEF_ICON_TITLE      "Header"

#define UHW_MARGIN		5

#define UHW_WIDTH		540
#define UHW_HEIGHT		380

#define UHW_PROMPT_HEIGHT	30

#define UHW_BUTTON_WIDTH	70
#define UHW_BUTTON_HEIGHT	28


/*
 *	Universe header window structure:
 */
typedef struct {

	char map_state;
	char is_in_focus;
	int x, y;
	unsigned int width, height;

	bool_t has_changes;

	pixmap_t toplevel_buf;
	win_t toplevel;

	prompt_window_struct	title_prompt,
				convert_ru_to_au_prompt,
				lost_found_owner_prompt,

				isr_prompt,
				ocsn_prompt,
				ss_prompt,

				player_start_pos_prompt,
				player_start_dir_prompt,

				guest_start_pos_prompt,
				guest_start_dir_prompt;

	push_button_struct	ok_btn,
				apply_btn,
				cancel_btn;

	/* Pointer to source universe editor window. */
	void *src;

} uhw_struct;
extern uhw_struct **uhw;
extern int total_uhws;


/* uhw.c */
extern int UHWIsAllocated(int n);
extern int UHWAllocate();
extern void UHWDelete(int n);
extern void UHWDeleteAll();

extern int UHWDoFetch(int n, void *src_ptr);
extern int UHWDoApply(int n);
extern int UHWOKCB(void *ptr);
extern int UHWApplyCB(void *ptr);
extern int UHWCancelCB(void *ptr);

extern int UHWInit(int n, void *src_ptr);
extern void UHWDraw(int n, int amount);
extern int UHWManage(int n, event_t *event);
extern int UHWManageAll(event_t *event);
extern void UHWMap(int n);
extern void UHWUnmap(int n);
extern void UHWDestroy(int n);

#endif	/* UHW_H */
