// unvedit/uhw.cpp
/*
                           Universe Header Window

	Functions:

	uew_struct *UHW_GETUEW(void *ptr, int *n)

	int UHWIsAllocated(int n)
	int UHWAllocate()
	void UHWDelete(int n)
	void UHWDeleteAll()

	int UHWDoFetch(int n, void *src_ptr)
	int UHWDoApply(int n)

	int UHWOKCB(void *ptr)
	int UHWApplyCB(void *ptr)
	int UHWCancelCB(void *ptr)

	int UHWInit(int n, void *src_ptr)
	void UHWDraw(int n, int amount)
	int UHWManage(int n, event_t *event)
	int UHWManageAll(event_t *event)
	void UHWMap(int n)
	void UHWUnmap(int n)
	void UHWDestroy(int n)

	---

 */
/*
#include <stdio.h>
#include <malloc.h>
#include <string.h>
*/

#ifndef MAX
#define MIN(a,b)        (((a) < (b)) ? (a) : (b))
#define MAX(a,b)	(((a) > (b)) ? (a) : (b))
#endif

#include "../include/string.h"
#include "../include/disk.h"

#include "../include/osw-x.h"
#include "../include/widget.h"

#include "../include/reality.h"
#include "../include/objects.h"

#include "../include/unvmatch.h"
#include "../include/unvmath.h"
#include "../include/unvutil.h"

#include "uew.h"
#include "ue.h"
#include "uhw.h"

namespace static_uhw {
	uhw_struct *delete_uhw;
}
uew_struct *UHW_GETUEW(void *ptr, int *n);

/* #define MIN(a,b)	((a) < (b) ? (a) : (b)) */
/* #define MAX(a,b)	((a) > (b) ? (a) : (b)) */


/*
 *	UHW schedualed to be deleted.
 */
/*
static uhw_struct *delete_uhw;


uew_struct *UHW_GETUEW(void *ptr, int *n);
*/

/*
 *	Returns the pointer to the universe editor window if it exists
 *	and sets n to match its index value.  If no match can be found
 *	then *n is -1 and NULL is returned.
 */
uew_struct *UHW_GETUEW(void *ptr, int *n)
{
	int i;


	if(n != NULL)
	    *n = -1;

	if(ptr == NULL)
	    return(NULL);

	for(i = 0; i < total_uews; i++)
	{
	    if(uew[i] == (uew_struct *)ptr)
	    {
		if(n != NULL)
		    *n = i;

		return(uew[i]);
	    }
	}

	return(NULL);
}

/*
 *	Checks if universe header window n is allocated.
 */
int UHWIsAllocated(int n)
{
	if((uhw == NULL) ||
           (n < 0) || (n >= total_uhws)
	)
	    return(0);
	else if(uhw[n] == NULL)
	    return(0);
	else
	    return(1);
}

/*
 *	Allocates a new universe header window, returning its
 *	index number.
 */
int UHWAllocate()
{
        int i, n;


        if(total_uhws < 0)
            total_uhws = 0;

        for(i = 0; i < total_uhws; i++)
        {
            if(uhw[i] == NULL)
                break;
        }
        if(i < total_uhws)
        {
            n = i;
        }
        else
        {
            n = total_uhws;
            total_uhws++; 
            uhw = (uhw_struct **)realloc(
                uhw,
                total_uhws * sizeof(uhw_struct *)
            );
            if(uhw == NULL)
            {
                total_uhws = 0;
                return(-1);
            }
        }

        uhw[n] = (uhw_struct *)calloc(
            1,
            sizeof(uhw_struct)
        );
        if(uhw[n] == NULL)
        {       
            return(-1);   
        }


        return(n);
}

/*
 *	Deletes universe header window.
 */
void UHWDelete(int n)
{
        if(!UHWIsAllocated(n))
            return;

                
        /* Deallocate resources. */
        UHWDestroy(n);   
            

        /* Free structure itself. */
        free(uhw[n]);
        uhw[n] = NULL; 


	return;
}

/*
 *	Deletes all universe header windows.
 */
void UHWDeleteAll()
{
	int i;


        for(i = 0; i < total_uhws; i++)
            UHWDelete(i);

        free(uhw);
        uhw = NULL;

        total_uhws = 0;


	return;
}


/*
 *	Fetch values from the uew pointed by src_ptr procedure.
 */
int UHWDoFetch(int n, void *src_ptr)
{
        uhw_struct *uhw_ptr;
	uew_struct *uew_ptr;
	const int len = 256;
	char text[len];


        if(UHWIsAllocated(n))
            uhw_ptr = uhw[n];
        else
            return(-1);


	/* Look for source uew. */
	uhw_ptr->src = src_ptr;
	uew_ptr = UHW_GETUEW(uhw_ptr->src, NULL);
	if(uew_ptr == NULL)
	{
	    uhw_ptr->src = NULL;
	    return(0);
	}


	/* Begin fetching values. */

	/* Title. */
	PromptSetS(
	    &uhw_ptr->title_prompt,
	    uew_ptr->unv_header.title
	);

	/* Real units to Astronomical units. */
        PromptSetF(
            &uhw_ptr->convert_ru_to_au_prompt,
            uew_ptr->unv_header.ru_to_au
        );

	/* Lost and found owner. */
	sprintf(text, "#%i", uew_ptr->unv_header.lost_found_owner);
        PromptSetS(&uhw_ptr->lost_found_owner_prompt, text);

	/* ISR File. */
        PromptSetS(
            &uhw_ptr->isr_prompt,
            uew_ptr->unv_header.isr
        );
        /* OCSN File. */
        PromptSetS(
            &uhw_ptr->ocsn_prompt,             
            uew_ptr->unv_header.ocsn
        );
        /* SS File. */
        PromptSetS(
            &uhw_ptr->ss_prompt,             
            uew_ptr->unv_header.ss
        );

	/* Player start position. */
	UNVLocationFormatString(
	    text,
	    &uew_ptr->unv_header.player_start_sect_x,
            &uew_ptr->unv_header.player_start_sect_y,
            &uew_ptr->unv_header.player_start_sect_z,
            &uew_ptr->unv_header.player_start_x,
            &uew_ptr->unv_header.player_start_y,
            &uew_ptr->unv_header.player_start_z,
	    len
	);
	PromptSetS(&uhw_ptr->player_start_pos_prompt, text);

	/* Player start direction. */
        UNVDirectionFormatString(
	    text,
	    &uew_ptr->unv_header.player_start_heading,
	    &uew_ptr->unv_header.player_start_pitch,
	    &uew_ptr->unv_header.player_start_bank,
	    len
	);
	PromptSetS(&uhw_ptr->player_start_dir_prompt, text);

        /* Guest start position. */
        UNVLocationFormatString(
            text,
            &uew_ptr->unv_header.guest_start_sect_x,
            &uew_ptr->unv_header.guest_start_sect_y,
            &uew_ptr->unv_header.guest_start_sect_z,
            &uew_ptr->unv_header.guest_start_x,
            &uew_ptr->unv_header.guest_start_y,
            &uew_ptr->unv_header.guest_start_z,
            len
        );
        PromptSetS(&uhw_ptr->guest_start_pos_prompt, text);

        /* Guest start direction. */
        UNVDirectionFormatString(
            text,
            &uew_ptr->unv_header.guest_start_heading,
            &uew_ptr->unv_header.guest_start_pitch,
            &uew_ptr->unv_header.guest_start_bank,
            len
        );
        PromptSetS(&uhw_ptr->guest_start_dir_prompt, text);


	/* Record source. */
	uhw_ptr->src = src_ptr;


	return(0);
}



/*
 *	Apply procedure.
 */
int UHWDoApply(int n)
{
	char *strptr;
	char title[UNV_TITLE_MAX + 256];

        int uew_num, total;
	uhw_struct *uhw_ptr;
        uew_struct *uew_ptr;


	if(UHWIsAllocated(n))
	    uhw_ptr = uhw[n];
	else
	    return(-1);


        /* Look for source uew. */
        uew_ptr = UHW_GETUEW(uhw_ptr->src, NULL);
        if(uew_ptr == NULL)
        {
            uhw_ptr->src = NULL;
            return(0);
        }
	/* Get uew_num. */
	for(uew_num = 0; uew_num < total_uews; uew_num++)
	{
	    if(uew[uew_num] == NULL)
		continue;

	    if(uew[uew_num] == uew_ptr)
		break;
	}
	if(uew_num >= total_uews)
	    uew_num = -1;

	/* Apply values to uew from uhw. */

	/* Title. */
	strptr = PromptGetS(&uhw_ptr->title_prompt);
	if(strptr != NULL)
	    strncpy(uew_ptr->unv_header.title, strptr, UNV_TITLE_MAX);
	uew_ptr->unv_header.title[UNV_TITLE_MAX - 1] = '\0';

	/* Real units to Astronomical units. */
	uew_ptr->unv_header.ru_to_au = PromptGetF(
	    &uhw_ptr->convert_ru_to_au_prompt
	);

        /* Lost and found owner. */
	uew_ptr->unv_header.lost_found_owner = MatchObjectByName(
	    uew_ptr->object, uew_ptr->total_objects,
	    PromptGetS(&uhw_ptr->lost_found_owner_prompt), -1
	);

        /* ISR File. */
	strptr = PromptGetS(&uhw_ptr->isr_prompt);
        if(strptr != NULL)
            strncpy(uew_ptr->unv_header.isr, strptr, PATH_MAX + NAME_MAX);
        uew_ptr->unv_header.isr[PATH_MAX + NAME_MAX - 1] = '\0';

        /* OCSN File. */
        strptr = PromptGetS(&uhw_ptr->ocsn_prompt);
        if(strptr != NULL)
            strncpy(uew_ptr->unv_header.ocsn, strptr, PATH_MAX + NAME_MAX);
        uew_ptr->unv_header.ocsn[PATH_MAX + NAME_MAX - 1] = '\0';

        /* SS File. */
        strptr = PromptGetS(&uhw_ptr->ss_prompt);
        if(strptr != NULL)
            strncpy(uew_ptr->unv_header.ss, strptr, PATH_MAX + NAME_MAX);
        uew_ptr->unv_header.ss[PATH_MAX + NAME_MAX - 1] = '\0';

	/* Player start position. */
	UNVParseLocation(
	    PromptGetS(&uhw_ptr->player_start_pos_prompt),
            &uew_ptr->unv_header.player_start_sect_x,
            &uew_ptr->unv_header.player_start_sect_y,
            &uew_ptr->unv_header.player_start_sect_z,
            &uew_ptr->unv_header.player_start_x,
            &uew_ptr->unv_header.player_start_y,
            &uew_ptr->unv_header.player_start_z
	);

	/* Player start direction. */
	UNVParseDirection(
	    PromptGetS(&uhw_ptr->player_start_dir_prompt),
	    &uew_ptr->unv_header.player_start_heading,
            &uew_ptr->unv_header.player_start_pitch,
            &uew_ptr->unv_header.player_start_bank
	);

        /* Guest start position. */
        UNVParseLocation(
            PromptGetS(&uhw_ptr->guest_start_pos_prompt),
            &uew_ptr->unv_header.guest_start_sect_x,
            &uew_ptr->unv_header.guest_start_sect_y, 
            &uew_ptr->unv_header.guest_start_sect_z,
            &uew_ptr->unv_header.guest_start_x,
            &uew_ptr->unv_header.guest_start_y,
            &uew_ptr->unv_header.guest_start_z 
        );
 
        /* Guest start direction. */
        UNVParseDirection(
            PromptGetS(&uhw_ptr->guest_start_dir_prompt),
            &uew_ptr->unv_header.guest_start_heading,
            &uew_ptr->unv_header.guest_start_pitch,  
            &uew_ptr->unv_header.guest_start_bank
        );


	/* Reset has changes marker on uhw. */
        uhw_ptr->has_changes = False;

	/* Set has changes mark on uew. */
	if(!uew_ptr->has_changes)
	    uew_ptr->has_changes = True;


	/* Update title on uew. */
	sprintf(title, "%s: %s",
	    UEW_DEF_TITLE,
	    uew_ptr->unv_header.title
	);
	OSWSetWindowTitle(
	    uew_ptr->toplevel,
	    title
	);
	if(uew_ptr->map_state)
	    UEWDraw(uew_num, UEW_DRAW_COMPLETE);


	/* Reload isrefs. */
        ISRefDeleteAll(uew_ptr->isref, uew_ptr->total_isrefs);
        uew_ptr->isref = NULL;   
        uew_ptr->total_isrefs = 0;

	uew_ptr->isref = ISRefLoadFromFile(
            uew_ptr->unv_header.isr,
            &total,
	    dname.images
	);
	uew_ptr->total_isrefs = total;



	/* Need to update window menus on uews. */
	UEWDoUpdateWindowMenus();


	return(0);
}


/*
 *	OK button callback handler.
 */
int UHWOKCB(void *ptr)
{
	int i, uhw_num;
        uhw_struct *uhw_ptr;


	for(i = 0, uhw_ptr = NULL, uhw_num = -1; i < total_uhws; i++)
	{
	    if(uhw[i] == NULL)
		continue;

	    if(uhw[i] == (uhw_struct *)ptr)
	    {
		uhw_ptr = (uhw_struct *)ptr;
		uhw_num = i;
		break;
	    }
	}
	if(uhw_ptr == NULL)
	    return(-1);


	UHWDoApply(uhw_num);
	UHWUnmap(uhw_num);


	return(0);
}

/*
 *	Apply button callback handler.
 */
int UHWApplyCB(void *ptr)
{
        int i, uhw_num;
        uhw_struct *uhw_ptr;


        for(i = 0, uhw_ptr = NULL, uhw_num = -1; i < total_uhws; i++)
        {
            if(uhw[i] == NULL)
                continue;

            if(uhw[i] == (uhw_struct *)ptr)
            {
                uhw_ptr = (uhw_struct *)ptr;
                uhw_num = i;
                break;
            }
        }
        if(uhw_ptr == NULL)
            return(-1);


        UHWDoApply(uhw_num);


        return(0);
}

/*
 *      Cancel button callback handler.
 */
int UHWCancelCB(void *ptr)
{ 
        int i, uhw_num;
        uhw_struct *uhw_ptr;

 
        for(i = 0, uhw_ptr = NULL, uhw_num = -1; i < total_uhws; i++)
        {
            if(uhw[i] == NULL)
                continue;
        
            if(uhw[i] == (uhw_struct *)ptr)
            {
                uhw_ptr = (uhw_struct *)ptr;
                uhw_num = i;
                break;
            }
        }
        if(uhw_ptr == NULL)
            return(-1);
        
        
        UHWUnmap(uhw_num);


        return(0);
}


/*
 *	Initializes a universe header window.
 */
int UHWInit(int n, void *src_ptr)
{
	int x, y;
	uhw_struct *uhw_ptr;

        char hotkey[PBTN_MAX_HOTKEYS];
	win_attr_t wattr;
            
         
        if(UHWIsAllocated(n))
            uhw_ptr = uhw[n];
        else
            return(-1);


	uhw_ptr->map_state = 0;
	uhw_ptr->is_in_focus = 0;
	uhw_ptr->x = 0;
	uhw_ptr->y = 0;
	uhw_ptr->width = UHW_WIDTH;
	uhw_ptr->height = UHW_HEIGHT;

	uhw_ptr->has_changes = False;
	uhw_ptr->src = NULL;



        /* ******************************************************** */
        /* Create toplevel. */
        if(
            OSWCreateWindow(
                &uhw_ptr->toplevel,
                osw_gui[0].root_win,
                uhw_ptr->x, uhw_ptr->y,
                uhw_ptr->width, uhw_ptr->height
            )
        )
            return(-1);

        OSWSetWindowWMProperties(
            uhw_ptr->toplevel,
            UHW_DEF_TITLE,
            UHW_DEF_ICON_TITLE,
            ((ue_image.unvedit_icon_pm == 0) ?
                widget_global.std_icon_pm : ue_image.unvedit_icon_pm),
            False,			/* Let WM set coordinates? */
            uhw_ptr->x, uhw_ptr->y,
            uhw_ptr->width, uhw_ptr->height,
            uhw_ptr->width, uhw_ptr->height,
            WindowFrameStyleFixed,
            NULL, 0
        );

        OSWSetWindowInput(
            uhw_ptr->toplevel,
            OSW_EVENTMASK_TOPLEVEL
        );

        WidgetCenterWindow(uhw_ptr->toplevel, WidgetCenterWindowToRoot);
        OSWGetWindowAttributes(uhw_ptr->toplevel, &wattr);
        uhw_ptr->x = wattr.x;
        uhw_ptr->y = wattr.y;
        uhw_ptr->width = wattr.width;
        uhw_ptr->height = wattr.height;


	/* Title prompt. */
	x = UHW_MARGIN;
	y = UHW_MARGIN;
        if(PromptInit(
            &uhw_ptr->title_prompt,
            uhw_ptr->toplevel,
	    x, y,
            MAX(uhw_ptr->width - (2 * UHW_MARGIN), 100),
            UHW_PROMPT_HEIGHT,
            PROMPT_STYLE_FLUSHED,
            "Title:",
            UNV_TITLE_MAX,
            0,
            NULL
        ))
            return(-1);

	/* Convert ru to au prompt. */
	y += UHW_PROMPT_HEIGHT;
        if(PromptInit(
            &uhw_ptr->convert_ru_to_au_prompt,
            uhw_ptr->toplevel,
            x, y, 
            MAX(uhw_ptr->width - (2 * UHW_MARGIN), 100),
            UHW_PROMPT_HEIGHT,
            PROMPT_STYLE_FLUSHED,
            "RU To AU:",
            30,
            0,
            NULL
        ))
            return(-1);

        /* Lost and found owner prompt. */
	y += UHW_PROMPT_HEIGHT;
        if(PromptInit(
            &uhw_ptr->lost_found_owner_prompt,
            uhw_ptr->toplevel,
	    x, y,
            MAX(uhw_ptr->width - (2 * UHW_MARGIN), 100),
            UHW_PROMPT_HEIGHT,
            PROMPT_STYLE_FLUSHED,
            "Lost/Found Owner:",
            XSW_OBJ_NAME_MAX + 20,
            0,
            NULL
        ))
            return(-1);

        /* ISR prompt. */
	y += UHW_PROMPT_HEIGHT + UHW_MARGIN;
        if(PromptInit(
            &uhw_ptr->isr_prompt,
            uhw_ptr->toplevel,
            x, y,
	    MAX(uhw_ptr->width - (2 * UHW_MARGIN), 100),
            UHW_PROMPT_HEIGHT,
            PROMPT_STYLE_FLUSHED,
            "ISR File:",
            PATH_MAX + NAME_MAX,
            0,
            NULL   
        ))
            return(-1);

        /* OSCN prompt. */
	y += UHW_PROMPT_HEIGHT;
        if(PromptInit(
            &uhw_ptr->ocsn_prompt,
            uhw_ptr->toplevel,
            x, y,
            MAX(uhw_ptr->width - (2 * UHW_MARGIN), 100),
            UHW_PROMPT_HEIGHT,
            PROMPT_STYLE_FLUSHED,
            "OCSN File:",
            PATH_MAX + NAME_MAX,
            0,
            NULL
        ))
            return(-1);

        /* SS prompt. */
        y += UHW_PROMPT_HEIGHT;
        if(PromptInit(
            &uhw_ptr->ss_prompt,
            uhw_ptr->toplevel,
            x, y,
            MAX(uhw_ptr->width - (2 * UHW_MARGIN), 100),
            UHW_PROMPT_HEIGHT,
            PROMPT_STYLE_FLUSHED,
            "SS File:",
            PATH_MAX + NAME_MAX,
            0,
            NULL   
        ))
            return(-1);


	/* Player start position. */
        y += UHW_PROMPT_HEIGHT + UHW_MARGIN;
        if(PromptInit(
            &uhw_ptr->player_start_pos_prompt,
            uhw_ptr->toplevel,
            x, y,
            MAX(uhw_ptr->width - (2 * UHW_MARGIN), 100),
            UHW_PROMPT_HEIGHT,
            PROMPT_STYLE_FLUSHED,
            "Player Start Position:",
            256,
            0,
            NULL
        ))  
            return(-1);

        /* Player start direction. */
        y += UHW_PROMPT_HEIGHT;
        if(PromptInit(
            &uhw_ptr->player_start_dir_prompt,
            uhw_ptr->toplevel,
            x, y,
            MAX(uhw_ptr->width - (2 * UHW_MARGIN), 100),
            UHW_PROMPT_HEIGHT,
            PROMPT_STYLE_FLUSHED,
            "Player Start Direction:", 
            256,
            0,
            NULL
        ))
            return(-1);

        /* Guest start position. */
        y += UHW_PROMPT_HEIGHT + UHW_MARGIN;
        if(PromptInit(
            &uhw_ptr->guest_start_pos_prompt,
            uhw_ptr->toplevel,
            x, y,
            MAX(uhw_ptr->width - (2 * UHW_MARGIN), 100),
            UHW_PROMPT_HEIGHT,
            PROMPT_STYLE_FLUSHED,
            "Guest Start Position:",
            256,
            0,
            NULL
        ))
            return(-1);

        /* Guest start direction. */
        y += UHW_PROMPT_HEIGHT;
        if(PromptInit(
            &uhw_ptr->guest_start_dir_prompt,
            uhw_ptr->toplevel,
            x, y,
            MAX(uhw_ptr->width - (2 * UHW_MARGIN), 100),
            UHW_PROMPT_HEIGHT,
            PROMPT_STYLE_FLUSHED,
            "Guest Start Direction:",
            256,
            0,
            NULL
        ))
            return(-1);


	/* Link prompts togeather. */
        uhw_ptr->title_prompt.next = &uhw_ptr->convert_ru_to_au_prompt;
        uhw_ptr->title_prompt.prev = &uhw_ptr->guest_start_dir_prompt;

        uhw_ptr->convert_ru_to_au_prompt.next = &uhw_ptr->lost_found_owner_prompt;
        uhw_ptr->convert_ru_to_au_prompt.prev = &uhw_ptr->title_prompt;

        uhw_ptr->lost_found_owner_prompt.next = &uhw_ptr->isr_prompt;
        uhw_ptr->lost_found_owner_prompt.prev = &uhw_ptr->convert_ru_to_au_prompt;


        uhw_ptr->isr_prompt.next = &uhw_ptr->ocsn_prompt;
        uhw_ptr->isr_prompt.prev = &uhw_ptr->lost_found_owner_prompt;

        uhw_ptr->ocsn_prompt.next = &uhw_ptr->ss_prompt;
        uhw_ptr->ocsn_prompt.prev = &uhw_ptr->isr_prompt;

        uhw_ptr->ss_prompt.next = &uhw_ptr->player_start_pos_prompt;
        uhw_ptr->ss_prompt.prev = &uhw_ptr->ocsn_prompt;


        uhw_ptr->player_start_pos_prompt.next = &uhw_ptr->player_start_dir_prompt;
        uhw_ptr->player_start_pos_prompt.prev = &uhw_ptr->ss_prompt;

        uhw_ptr->player_start_dir_prompt.next = &uhw_ptr->guest_start_pos_prompt;
        uhw_ptr->player_start_dir_prompt.prev = &uhw_ptr->player_start_pos_prompt;

        uhw_ptr->guest_start_pos_prompt.next = &uhw_ptr->guest_start_dir_prompt;
        uhw_ptr->guest_start_pos_prompt.prev = &uhw_ptr->player_start_dir_prompt;

        uhw_ptr->guest_start_dir_prompt.next = &uhw_ptr->title_prompt;
        uhw_ptr->guest_start_dir_prompt.prev = &uhw_ptr->guest_start_pos_prompt;


        /* Buttons. */   
        if(
            PBtnInit(
                &uhw_ptr->ok_btn,
                uhw_ptr->toplevel,
                10,
                (int)uhw_ptr->height - 10 - UHW_BUTTON_HEIGHT,
                UHW_BUTTON_WIDTH, UHW_BUTTON_HEIGHT,
                "OK",
                PBTN_TALIGN_CENTER,
                NULL,
                (void *)uhw_ptr,
                UHWOKCB
            )
        )
            return(-1);
        hotkey[0] = '\n';
        hotkey[1] = '\0';
        PBtnSetHotKeys(&uhw_ptr->ok_btn, hotkey);

        if(
            PBtnInit(
                &uhw_ptr->apply_btn,
                uhw_ptr->toplevel,
                ((int)uhw_ptr->width / 2) - (UHW_BUTTON_WIDTH / 2),
                (int)uhw_ptr->height - 10 - UHW_BUTTON_HEIGHT,
                UHW_BUTTON_WIDTH, UHW_BUTTON_HEIGHT,
                "Apply",
                PBTN_TALIGN_CENTER,
                NULL,
                (void *)uhw_ptr,
                UHWApplyCB
            )   
        )
            return(-1);

        if(
            PBtnInit(
                &uhw_ptr->cancel_btn, 
                uhw_ptr->toplevel,
                uhw_ptr->width - 10 - UHW_BUTTON_WIDTH,
                (int)uhw_ptr->height - 10 - UHW_BUTTON_HEIGHT,
                UHW_BUTTON_WIDTH, UHW_BUTTON_HEIGHT,
                "Cancel",
                PBTN_TALIGN_CENTER,
                NULL,
                (void *)uhw_ptr,
                UHWCancelCB
            )
        )
            return(-1);

        hotkey[0] = 0x1b;
        hotkey[1] = '\0';
        PBtnSetHotKeys(&uhw_ptr->cancel_btn, hotkey);


	/* Need to update window menus on uews. */
	UEWDoUpdateWindowMenus();


	return(0);
}

void UHWDraw(int n, int amount)
{
	win_t w;
	pixmap_t pixmap;
        uhw_struct *uhw_ptr;
	win_attr_t wattr;

            
         
        if(UHWIsAllocated(n))
            uhw_ptr = uhw[n];
        else
            return;


	/* Map as needed. */
	if(!uhw_ptr->map_state)
	{
	    OSWMapRaised(uhw_ptr->toplevel);

	    PromptMap(&uhw_ptr->title_prompt);
            PromptMap(&uhw_ptr->convert_ru_to_au_prompt);
            PromptMap(&uhw_ptr->lost_found_owner_prompt);
            PromptMap(&uhw_ptr->isr_prompt);
            PromptMap(&uhw_ptr->ocsn_prompt);
            PromptMap(&uhw_ptr->ss_prompt);
            PromptMap(&uhw_ptr->player_start_pos_prompt);
            PromptMap(&uhw_ptr->player_start_dir_prompt);
            PromptMap(&uhw_ptr->guest_start_pos_prompt);
            PromptMap(&uhw_ptr->guest_start_dir_prompt);
 
	    PBtnMap(&uhw_ptr->ok_btn);
            PBtnMap(&uhw_ptr->apply_btn);
            PBtnMap(&uhw_ptr->cancel_btn);


	    uhw_ptr->map_state = 1;
	}


	OSWGetWindowAttributes(uhw_ptr->toplevel, &wattr);

	if(uhw_ptr->toplevel_buf == 0)
	{
	    if(OSWCreatePixmap(
		&uhw_ptr->toplevel_buf, wattr.width, wattr.height
	    ))
		return;
	}


	if(1)
	{
            w = uhw_ptr->toplevel;
            pixmap = uhw_ptr->toplevel_buf;
                 
            OSWGetWindowAttributes(w, &wattr);
        
            /* Redraw background. */ 
            if(widget_global.force_mono)
            {
                OSWClearPixmap(
                    pixmap,
                    wattr.width, wattr.height,
                    osw_gui[0].black_pix
                );

                OSWSetFgPix(osw_gui[0].white_pix);
		OSWDrawLine(
		    pixmap,
		    0,
		    (int)wattr.height - 20 - UHW_BUTTON_HEIGHT,
		    wattr.width,
                    (int)wattr.height - 20 - UHW_BUTTON_HEIGHT
		);
            }
            else
            {
                WidgetPutImageTile(
                    pixmap,
                    widget_global.std_bkg_img,
                    wattr.width, wattr.height
                );

                OSWSetFgPix(widget_global.surface_shadow_pix);
                OSWDrawLine(
                    pixmap,
                    0,
                    (int)wattr.height - 25 - UHW_BUTTON_HEIGHT,
                    wattr.width,
                    (int)wattr.height - 25 - UHW_BUTTON_HEIGHT
                );
                OSWSetFgPix(widget_global.surface_highlight_pix);
                OSWDrawLine(
                    pixmap,
                    0,
                    (int)wattr.height - 25 - UHW_BUTTON_HEIGHT + 1,
                    wattr.width,
                    (int)wattr.height - 25 - UHW_BUTTON_HEIGHT + 1
                );
            }


            OSWPutBufferToWindow(w, pixmap);
	}


	return;
}


int UHWManage(int n, event_t *event)
{
	keycode_t keycode;
        int events_handled = 0;
        uhw_struct *uhw_ptr;


	static_uhw::delete_uhw = NULL;
            
         
        if(UHWIsAllocated(n))
            uhw_ptr = uhw[n];
        else
            return(events_handled);


	if(event == NULL)
	    return(events_handled);

	if(!uhw_ptr->map_state &&
	   (event->type != MapNotify)
	)
	    return(events_handled);


	switch(event->type)
	{
          /* ******************************************************* */
	  case KeyPress:
	    if(!uhw_ptr->is_in_focus)
		break;

	    keycode = event->xkey.keycode;

	    /* Enter. */
	    if((keycode == osw_keycode.enter) ||
               (keycode == osw_keycode.np_enter)
	    )
	    {
		UHWOKCB((void *)uhw_ptr);

                /* Delete this uhw as needed. */
                if(static_uhw::delete_uhw == uhw_ptr)
		{
                    UHWDelete(n);
		    UEWDoUpdateWindowMenus();
		}

		events_handled++;
		return(events_handled);
	    }
	    break;

	  /* ******************************************************* */
          case KeyRelease:
          
            break;

          /* ******************************************************* */
	  case Expose:
	    if(event->xany.window == uhw_ptr->toplevel)
	    {
		events_handled++;
	    }
	    break;

          /* ******************************************************** */
          case UnmapNotify:
            if(event->xany.window == uhw_ptr->toplevel)
            {
                events_handled++;
                return(events_handled);
            }
            break;

          /* ******************************************************** */
          case MapNotify:
            if(event->xany.window == uhw_ptr->toplevel)
            {
                events_handled++;
                return(events_handled);
            }
            break;

          /* ******************************************************* */
	  case FocusIn:
            if(event->xany.window == uhw_ptr->toplevel)
            {
		uhw_ptr->is_in_focus = 1;

                events_handled++;
		return(events_handled);
            }
            break;

          /* ******************************************************* */
          case FocusOut: 
            if(event->xany.window == uhw_ptr->toplevel)
            {
                uhw_ptr->is_in_focus = 0;

                events_handled++;
                return(events_handled);
            }
            break;

          /* ******************************************************* */
          case ClientMessage:
            if(OSWIsEventDestroyWindow(uhw_ptr->toplevel, event))
            {
		UHWCancelCB((void *)uhw_ptr);

                /* Delete this uhw as needed. */
                if(static_uhw::delete_uhw == uhw_ptr)
		{
                    UHWDelete(n);
                    UEWDoUpdateWindowMenus();
		}

                events_handled++;
                return(events_handled);
            }
            break;
	}

	if(events_handled > 0)
	{
	    UHWDraw(n, 0);
	}


	if(events_handled == 0)
	    events_handled += PromptManage(&uhw_ptr->title_prompt, event);
        if(events_handled == 0)
            events_handled += PromptManage(&uhw_ptr->convert_ru_to_au_prompt, event);
        if(events_handled == 0)
            events_handled += PromptManage(&uhw_ptr->lost_found_owner_prompt, event);
        if(events_handled == 0)
            events_handled += PromptManage(&uhw_ptr->isr_prompt, event);
        if(events_handled == 0)
            events_handled += PromptManage(&uhw_ptr->ocsn_prompt, event);
        if(events_handled == 0)
            events_handled += PromptManage(&uhw_ptr->ss_prompt, event);

        if(events_handled == 0)
            events_handled += PromptManage(&uhw_ptr->player_start_pos_prompt, event);
        if(events_handled == 0)
            events_handled += PromptManage(&uhw_ptr->player_start_dir_prompt, event);
        if(events_handled == 0)
            events_handled += PromptManage(&uhw_ptr->guest_start_pos_prompt, event);
        if(events_handled == 0)
            events_handled += PromptManage(&uhw_ptr->guest_start_dir_prompt, event);


        if(events_handled == 0)
            events_handled += PBtnManage(&uhw_ptr->ok_btn, event);
        if(events_handled == 0)
            events_handled += PBtnManage(&uhw_ptr->apply_btn, event);
        if(events_handled == 0)
            events_handled += PBtnManage(&uhw_ptr->cancel_btn, event);


	/* Delete this uhw as needed. */
	if(static_uhw::delete_uhw == uhw_ptr)
	{
	    UHWDelete(n);
            UEWDoUpdateWindowMenus();
	}


	return(events_handled);
}


int UHWManageAll(event_t *event)
{
	int i;
	int events_handled = 0;


	if(event == NULL)
	    return(events_handled);


	for(i = 0; i < total_uhws; i++)
	{
	    events_handled += UHWManage(i, event);
	}


	return(events_handled);
}
	

void UHWMap(int n)
{
        uhw_struct *uhw_ptr;
            
         
        if(UHWIsAllocated(n))
            uhw_ptr = uhw[n];
        else
            return;


        PromptMarkBuffer(
	    &uhw_ptr->title_prompt,
	    PROMPT_POS_END
	);

	uhw_ptr->title_prompt.is_in_focus = 0;
	uhw_ptr->convert_ru_to_au_prompt.is_in_focus = 0;
        uhw_ptr->lost_found_owner_prompt.is_in_focus = 0;
        uhw_ptr->isr_prompt.is_in_focus = 0;
        uhw_ptr->ocsn_prompt.is_in_focus = 0;
        uhw_ptr->ss_prompt.is_in_focus = 0;
        uhw_ptr->player_start_pos_prompt.is_in_focus = 0;
        uhw_ptr->player_start_dir_prompt.is_in_focus = 0;
        uhw_ptr->guest_start_pos_prompt.is_in_focus = 0;
        uhw_ptr->guest_start_dir_prompt.is_in_focus = 0;

	uhw_ptr->map_state = 0;
	UHWDraw(n, 0);

	uhw_ptr->title_prompt.is_in_focus = 1;


	return;
}

void UHWUnmap(int n)
{
        uhw_struct *uhw_ptr;
            
         
        if(UHWIsAllocated(n))
            uhw_ptr = uhw[n];
        else
            return;


	OSWUnmapWindow(uhw_ptr->toplevel);
        uhw_ptr->map_state = 0;

	OSWDestroyPixmap(&uhw_ptr->toplevel_buf);


	/* Schedual this uhw for deletion. */
	static_uhw::delete_uhw = uhw_ptr;


	return;
}

/*
 *	Destroys all allocated resources for universe header
 *	window n.
 */
void UHWDestroy(int n)
{
        uhw_struct *uhw_ptr;
            
         
        if(UHWIsAllocated(n))
            uhw_ptr = uhw[n];
        else
            return;


	uhw_ptr->map_state = 0;
	uhw_ptr->is_in_focus = 0;

	uhw_ptr->has_changes = False;
        uhw_ptr->src = NULL;


	if(IDC())
	{
	    PBtnDestroy(&uhw_ptr->ok_btn);
            PBtnDestroy(&uhw_ptr->apply_btn);
            PBtnDestroy(&uhw_ptr->cancel_btn);

            PromptDestroy(&uhw_ptr->title_prompt);
            PromptDestroy(&uhw_ptr->convert_ru_to_au_prompt);
            PromptDestroy(&uhw_ptr->lost_found_owner_prompt);
            PromptDestroy(&uhw_ptr->isr_prompt);
            PromptDestroy(&uhw_ptr->ocsn_prompt);
            PromptDestroy(&uhw_ptr->ss_prompt);
            PromptDestroy(&uhw_ptr->player_start_pos_prompt);
            PromptDestroy(&uhw_ptr->player_start_dir_prompt);
            PromptDestroy(&uhw_ptr->guest_start_pos_prompt);
            PromptDestroy(&uhw_ptr->guest_start_dir_prompt);

	    OSWDestroyPixmap(&uhw_ptr->toplevel_buf);
            OSWDestroyWindow(&uhw_ptr->toplevel);
	}


	return;
}



