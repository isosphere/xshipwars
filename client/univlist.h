/*
       Universe List, Universe List window, and Universe Edit window



 */

#ifndef UNIVLIST_H
#define UNIVLIST_H

#include <sys/types.h>

#include "../include/osw-x.h"
#include "../include/widget.h"


/*
 *	Universe list window redraw codes:
 */
#define ULW_DRAW_AMOUNT_COMPLETE        0
#define ULW_DRAW_AMOUNT_STATUS          1
#define ULW_DRAW_AMOUNT_LIST            2



/*
 *	Universe List Entries:
 */
typedef struct
{
	int x, y;			/* Reserved. */
	unsigned int width, height;	/* Reserved. */

	char *alias;		/* Alias (name of this universe). */
	char *url;		/* URL to this universe. */
        unsigned long last_connected;	/* In systime seconds. */
        char *comments;

	unsigned long times_connected;

} univ_entry_struct;
extern univ_entry_struct **univ_entry;
extern int total_univ_entries;



/*
 *	Universe list window structure:
 */
typedef struct {

    char map_state;
    int x, y;
    unsigned int width, height;
    char is_in_focus;
    visibility_t visibility_state;
    bool_t disabled;
  
    win_t toplevel;
   
    /* List. */
    list_window_struct list;
    
    /* URL Preview. */
    win_t status;
    pixmap_t status_buf;
    
    /* Menu bar. */ 
    menu_bar_struct mb;
    
    /* Right click menu. */
    menu_struct menu;

} xsw_univ_list_win_struct;
extern xsw_univ_list_win_struct univ_list_win;


/*
 *	Edit universe window structure:
 */
typedef struct {

    char map_state;
    int x, y;
    unsigned int width, height;
    char is_in_focus;
    visibility_t visibility_state;
    bool_t disabled;

    win_t toplevel;

    int univ_entry_num;         /* Currently editing this universe entry. */

    push_button_struct ok_btn;
    push_button_struct cancel_btn;
    push_button_struct apply_btn; 
    push_button_struct touch_btn; 

    prompt_window_struct alias_prompt;
    prompt_window_struct url_prompt;  
    prompt_window_struct comments_prompt;
    
} xsw_univ_edit_win_struct;
extern xsw_univ_edit_win_struct univ_edit_win;




/* In univeditwin.c */
extern void UnivEditUnfocusPrompts(void);
extern int UnivEditApplyChanges(void);

extern int UnivEditOKPBCB(void *ptr);
extern int UnivEditCancelPBCB(void *ptr);
extern int UnivEditApplyPBCB(void *ptr);
extern int UnivEditTouchPBCB(void *ptr);

extern int UnivEditWinInit(void);
extern int UnivEditWinDraw(void);
extern int UnivEditWinManage(event_t *event);
extern void UnivEditWinMap(void);
extern void UnivEditWinMap(void);
extern void UnivEditWinUnmap(void);
extern void UnivEditWinDestroy(void);


/* In univlist.c */
extern int UnivIsAllocated(int n);

extern int UnivAdd(
        char *alias,
        char *url,
        time_t last_connected,
        char *comments,
        int pos
);
extern void UnivDelete(int n);
extern void UnivDeleteAll(void);


/* In univlistfile.c */
extern int UnivListLoadFromFile(char *filename);
extern int UnivListSaveToFile(char *filename);


/* In univlistwin.c */
extern int ULWIsEntryInList(int univ_entry_num);
extern int ULWIsListItemInUniv(int list_entry_num);
extern image_t *ULWGetProperListItemIcon(
	univ_entry_struct *unv_entry_ptr
);

extern void UnivEntrySyncWithList(void);
extern univ_entry_struct *UnivListGetUnivEntryFromList(
        list_window_struct *list
);

extern int UnivListDoConnect(void);
extern int UnivListDoSelect(void *ptr);

extern int UnivListInit(void);
extern int UnivListResize(void);
extern int UnivListDraw(int amount);
extern int UnivListManage(event_t *event);
extern void UnivListMap(void);
extern void UnivListUnmap(void);
extern void UnivListDestroy(void);

extern int UnivListMenuCB(void *client_data, int op_code);


#endif	/* UNIVLIST_H */
