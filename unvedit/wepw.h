#ifndef WEPW_H
#define WEPW_H

#include "../include/objects.h"

#include "../include/osw-x.h"
#include "../include/widget.h"



/*
 *      Default title strings:
 */
#define WEPW_DEF_TITLE		"Object Weapons"
#define WEPW_DEF_ICON_TITLE	"Weapons"

#define WEPW_MARGIN		5

#define WEPW_WIDTH		360
#define WEPW_HEIGHT		420

#define WEPW_PROMPT_HEIGHT	30

#define WEPW_BUTTON_WIDTH	70
#define WEPW_BUTTON_HEIGHT	28


typedef struct {

        char map_state;
        char is_in_focus;
        int x, y;
        unsigned int width, height;

	bool_t has_changes;

        pixmap_t toplevel_buf;
        win_t toplevel;


	colum_list_struct	list;

	push_button_struct	create_wep_btn,
				delete_wep_btn;

	prompt_window_struct	flags_prompt,
				ocs_code_prompt,
				emission_type_prompt,
				amount_prompt,
				max_prompt,
				power_prompt,
				range_prompt,
				create_power_prompt,
				delay_prompt,
				/* No last_used. */
				use_sound_code_prompt,
				fire_sound_code_prompt,
				hit_sound_code_prompt,
				recover_sound_code_prompt;

        push_button_struct	close_btn;

	/* Pointer to source universe editor window. */
	void *src;

	/* Source object number. */
	int src_obj_num;
	xsw_object_struct *src_obj_ptr;

} wepw_struct;

extern wepw_struct **wepw;
extern int total_wepws;



extern int WepWGetLastSelected(wepw_struct *wepw_ptr);
extern xsw_object_struct *WepWGetObjectPtr(
        wepw_struct *wepw_ptr
);
extern xsw_weapons_struct *WepWGetWeaponPtr(
	wepw_struct *wepw_ptr,
	int wep_num
);

extern int WepWIsAllocated(int n);
extern int WepWAllocate();
extern void WepWDelete(int n);
extern void WepWDeleteAll();

extern int WepWGetAllValues(wepw_struct *wepw_ptr);
extern int WepWGetValues(wepw_struct *wepw_ptr, int wep_num);
extern int WepWSetValues(wepw_struct *wepw_ptr, int wep_num);

extern int WepWListCB(void *ptr);

extern int WepWCreateCB(void *ptr);
extern int WepWDeleteCB(void *ptr);
extern int WepWCloseCB(void *ptr);

extern int WepWInit(int n, void *src_ptr);
extern int WepWDraw(int n, int amount);
extern int WepWManage(int n, event_t *event);
extern int WepWManageAll(event_t *event);
extern void WepWMap(int n);
extern void WepWDoMapValues(int n, void *src, int obj_num);
extern void WepWUnmap(int n);
extern void WepWDestroy(int n);


#endif	/* WEPW_H */
