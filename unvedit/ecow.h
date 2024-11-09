#ifndef ECOW_H
#define ECOW_H

#include "../include/objects.h"

#include "../include/osw-x.h"
#include "../include/widget.h"



/*
 *      Default title strings:
 */
#define ECOW_DEF_TITLE		"Object Economy"
#define ECOW_DEF_ICON_TITLE	"Economy"


#define ECOW_WIDTH		360
#define ECOW_HEIGHT		460

#define ECOW_PROMPT_HEIGHT	30

#define ECOW_BUTTON_WIDTH	70
#define ECOW_BUTTON_HEIGHT	28


typedef struct {

        char map_state;
        char is_in_focus;
        int x, y;
        unsigned int width, height;

	bool_t has_changes;

        pixmap_t toplevel_buf;
        win_t toplevel;

	prompt_window_struct	flags_prompt,
				tax_general_prompt,
				tax_friend_prompt,
				tax_hostile_prompt;

	colum_list_struct	list;

	push_button_struct	create_prod_btn,
				delete_prod_btn;

	prompt_window_struct	prod_name_prompt,
				prod_buy_prompt,
				prod_sell_prompt,
				prod_amount_prompt,
				prod_max_prompt;

        push_button_struct	delete_obj_eco_btn,
				close_btn;

	/* Pointer to source universe editor window. */
	void *src;

	/* Source object number. */
	int src_obj_num;
	xsw_object_struct *src_obj_ptr;

} ecow_struct;

extern ecow_struct **ecow;
extern int total_ecows;



extern int EcoWGetLastSelected(ecow_struct *ecow_ptr);
extern xsw_object_struct *EcoWGetObjectPtr(ecow_struct *ecow_ptr);

extern int EcoWIsAllocated(int n);
extern int EcoWAllocate();
extern void EcoWDelete(int n);
extern void EcoWDeleteAll();

extern int EcoWGetAllValues(ecow_struct *ecow_ptr);
extern int EcoWGetValues(ecow_struct *ecow_ptr, int prod_num);
extern int EcoWSetValues(ecow_struct *ecow_ptr, int prod_num);

extern int EcoWListCB(void *ptr);

extern int EcoWCreateCB(void *ptr);
extern int EcoWDeleteCB(void *ptr);
extern int EcoWDeleteEcoCB(void *ptr);
extern int EcoWCloseCB(void *ptr);

extern int EcoWInit(int n, void *src_ptr);
extern int EcoWDraw(int n, int amount);
extern int EcoWManage(int n, event_t *event);
extern int EcoWManageAll(event_t *event);
extern void EcoWMap(int n);
extern void EcoWDoMapValues(int n, void *src, int obj_num);
extern void EcoWUnmap(int n);
extern void EcoWDestroy(int n);

void EcoWStringToUpper(char *s);

void EcoWDoPromptFocus(
        prompt_window_struct *src_prompt,
        prompt_window_struct *tar_prompt
);
void EcoWRefocusPrompts(
        ecow_struct *ecow_ptr,
        prompt_window_struct *prompt
);

#endif	/* ECOW_H */
