/*
                        Universe Edit Window Management

	Functions:

	char *UnivEditWinTimeAgoStr(time_t cur, time_t last)
	void UnivEditUnfocusPrompts()
	int UnivEditApplyChanges()

	int UnivEditOKPBCB(void *ptr)
	int UnivEditCancelPBCB(void *ptr)
	int UnivEditApplyPBCB(void *ptr)
	int UnivEditTouchPBCB(void *ptr)

	int UnivEditWinInit()
	int UnivEditWinDraw()
	int UnivEditWinManage(event_t *event)
	void UnivEditWinMap()
	void UnivEditWinUnmap()
	void UnivEditWinDestroy()

	---

 */

#include "univlist.h"
#include "xsw.h"



/* Fixed size for the universe edit window. */
#define UNIV_EDIT_WIDTH		440
#define UNIV_EDIT_HEIGHT	280

#define UE_BTN_WIDTH		70
#define UE_BTN_HEIGHT		28

/* Default values. */
#define MISSING_ALIAS_STR	"?"
#define MISSING_URL_STR		"swserv://localhost:1701"



xsw_univ_edit_win_struct univ_edit_win;



/*
 *	Returns a statically allocated string containing the
 *	last time connected in verbose form.
 */
char *UnivEditWinTimeAgoStr(time_t cur, time_t last)
{
	time_t dt, l;
	int len2;
	struct tm *tm_ptr;
	#define len 256
	static char s[len];


	/* Never visited? */
	if(last <= 0)
	    return("Never");


	/* Calculate delta time. */
	dt = cur - last;
	if(dt < 0)
	    dt = 0;


	/* Less than an hour ago? */
	if(dt < 3600)
	{
	    sprintf(
		s,
		"Less than one hour ago"	/* Be simplistic. */
	    );
	}
	/* Less than a day? */
	else if(dt < 86400)
	{
	    l = dt / 3600;
	    sprintf(
                s,
		"%ld %s ago",
		l,
		((l > 1) ? "hours" : "hour")
	    );
	}
	/* Less than a week? */
        else if(dt < (86400 * 7))
        {
            l = dt / 86400;
            if(l > 1)
                sprintf(
                    s,  
                    "%ld days ago",
                    l
                );
            else
                sprintf(
                    s,
                    "Yesterday"
                );
        }
	/* More than a week. */
	else
	{
	    tm_ptr = localtime(&last);
	    if(tm_ptr != NULL)
		len2 = strftime(
		    s,
		    len,
		    "%c",
		    tm_ptr
		);
	    else
		len2 = 0;

	    if(len2 >= len)
		len2 = len - 1;
	    if(len2 < 0)
		len2 = 0;

	    s[len2] = '\0';
	}

	return(s);
}



/*
 *	Procedure to unfocus all prompts.
 */
void UnivEditUnfocusPrompts()
{
	univ_edit_win.alias_prompt.is_in_focus = 0;
        univ_edit_win.url_prompt.is_in_focus = 0;
        univ_edit_win.comments_prompt.is_in_focus = 0;


	return;
}


/*
 *	Applies changes to the universe entry referanced
 *	in univ_edit_win.univ_entry_num.
 */
int UnivEditApplyChanges()
{
	int univ_entry_num, list_entry_num;
	char *strptr;
	univ_entry_struct *univ_entry_ptr;
	list_window_struct *list;


	/* Get pointer to list widget on universe list. */
        list = &univ_list_win.list;

	/* Get valid universe entry number. */
	univ_entry_num = univ_edit_win.univ_entry_num;
	if(UnivIsAllocated(univ_entry_num))
	    univ_entry_ptr = univ_entry[univ_entry_num];
	else
	    return(-1);


	/* Alias. */
	strptr = PromptGetS(&univ_edit_win.alias_prompt);
	free(univ_entry_ptr->alias);
	univ_entry_ptr->alias = StringCopyAlloc(strptr);

        /* URL. */
	strptr = PromptGetS(&univ_edit_win.url_prompt);
        free(univ_entry_ptr->url);
	univ_entry_ptr->url = StringCopyAlloc(strptr);

        /* Comments. */
	strptr = PromptGetS(&univ_edit_win.comments_prompt);
	free(univ_entry_ptr->comments);
	univ_entry_ptr->comments = StringCopyAlloc(strptr);


        /* Change name and icon on item on list widget. */
        list_entry_num = ListGetItemNumByDataPointer(
            list,
            univ_entry_ptr
        ); 
        if(list_entry_num > -1)
        {
	    /* Set new name. */
	    strptr = PromptGetS(&univ_edit_win.alias_prompt);
            free(list->entry[list_entry_num]->name);
            list->entry[list_entry_num]->name = StringCopyAlloc(strptr);

	    /* Set new icon. */
            list->entry[list_entry_num]->image = ULWGetProperListItemIcon(
                univ_entry_ptr
            );
        }


	return(0);
}

/*
 *	Button callbacks.
 */
int UnivEditOKPBCB(void *ptr)
{
	/* Apply changes. */
	UnivEditApplyChanges();

	/* Unmap universe edit window. */
	UnivEditWinUnmap();

	/* Redraw the universe list window as needed. */
	if(univ_list_win.map_state)
	    UnivListDraw(ULW_DRAW_AMOUNT_COMPLETE);


	return(0);
}


int UnivEditCancelPBCB(void *ptr)
{
	/* Unmap universe edit window. */
        UnivEditWinUnmap();

        return(0);
}


int UnivEditApplyPBCB(void *ptr)
{
        /* Apply changes. */
        UnivEditApplyChanges();

        /* Redraw the universe list window as needed. */
        if(univ_list_win.map_state)
            UnivListDraw(ULW_DRAW_AMOUNT_COMPLETE);


        return(0); 
}


int UnivEditTouchPBCB(void *ptr)
{
	int univ_entry_num;

	univ_entry_num = univ_edit_win.univ_entry_num;
        if(UnivIsAllocated(univ_entry_num))
	{
	    univ_entry[univ_entry_num]->last_connected =
		time(NULL);

            /* Redraw edit window. */
            UnivEditWinDraw();
	}


        return(0);
}


/* ******************************************************************* */

/*
 *	 Initialize the universe edit window.
 */
int UnivEditWinInit()
{
	int status;
	int x, y;
	unsigned int width, height;
	pixmap_t pixmap;
	win_attr_t wattr;

	x = 0;
	y = 0;
	width = UNIV_EDIT_WIDTH;
	height = UNIV_EDIT_HEIGHT;


	/* Set values. */
	univ_edit_win.map_state = 0;
	univ_edit_win.x = x;
        univ_edit_win.y = y;
        univ_edit_win.width = width;
        univ_edit_win.height = height;
        univ_edit_win.is_in_focus = 0;
        univ_edit_win.visibility_state = VisibilityFullyObscured;
        univ_edit_win.disabled = False;

        univ_edit_win.univ_entry_num = -1;


	/* Create toplevel. */
	if(
	    OSWCreateWindow(
	        &univ_edit_win.toplevel,
	        osw_gui[0].root_win,
	        x, y,
	        width, height
	    )
	)
	    return(-1);
        OSWSetWindowInput(univ_edit_win.toplevel, OSW_EVENTMASK_TOPLEVEL);
	WidgetCenterWindow(univ_edit_win.toplevel, WidgetCenterWindowToRoot);

	OSWGetWindowAttributes(univ_edit_win.toplevel, &wattr);
	univ_edit_win.x = wattr.x;
        univ_edit_win.y = wattr.y;
        univ_edit_win.width = wattr.width;
        univ_edit_win.height = wattr.height;

        /* WM properties. */  
        if(IMGIsImageNumAllocated(IMG_CODE_UNIV_ICON))
        {
            pixmap = OSWCreatePixmapFromImage( 
                xsw_image[IMG_CODE_UNIV_ICON]->image
            );
        }
        else
        {
            pixmap = widget_global.std_icon_pm;
        }    
        OSWSetWindowWMProperties(
            univ_edit_win.toplevel,
            "Universe Properties",	/* Title. */
            "Properties",		/* Icon title. */
            pixmap,			/* Icon. */
            False,			/* Let WM set coordinates? */
            /* Coordinates. */
            univ_edit_win.x, univ_edit_win.y,
            /* Min width and height. */
            UNIV_EDIT_WIDTH, UNIV_EDIT_HEIGHT,
            /* Max width and height. */
	    UNIV_EDIT_WIDTH, UNIV_EDIT_HEIGHT,
            WindowFrameStyleFixed,
            NULL, 0
        );
        OSWSetWindowBkg(univ_edit_win.toplevel, 0,
            widget_global.std_bkg_pm);



	/* *********************************************************** */
	/* Initialize widgets. */

	x = 10;
	y = (int)univ_edit_win.height - UE_BTN_HEIGHT - 10;
	status = PBtnInit(
	    &univ_edit_win.ok_btn,
	    univ_edit_win.toplevel,
	    x, y,
	    UE_BTN_WIDTH, UE_BTN_HEIGHT,
	    "OK",
	    PBTN_TALIGN_CENTER,
            NULL,
	    (void *)&univ_edit_win.ok_btn,
	    UnivEditOKPBCB
	);
	if(status)
	    return(-1);

        x = (int)univ_edit_win.width - UE_BTN_WIDTH - 10;
        status = PBtnInit(
            &univ_edit_win.cancel_btn,
            univ_edit_win.toplevel,
            x, y,
            UE_BTN_WIDTH, UE_BTN_HEIGHT,
            "Cancel",
            PBTN_TALIGN_CENTER,
            NULL,
            (void *)&univ_edit_win.cancel_btn,
	    UnivEditCancelPBCB
        );
        if(status)
            return(-1);

        status = PBtnInit(
            &univ_edit_win.touch_btn,
            univ_edit_win.toplevel,
            UNIV_EDIT_WIDTH - 110,
	    (int)univ_edit_win.height - (2 * UE_BTN_HEIGHT) - (4 * 10),
            UE_BTN_WIDTH, UE_BTN_HEIGHT,
            "Touch",
            PBTN_TALIGN_CENTER,
            NULL,
            (void *)&univ_edit_win.touch_btn,
            UnivEditTouchPBCB
        );
        if(status)
            return(-1);

        x = ((int)univ_edit_win.width / 2) - (UE_BTN_WIDTH / 2);
         status = PBtnInit(
            &univ_edit_win.apply_btn,
            univ_edit_win.toplevel,
            x, y,
            UE_BTN_WIDTH, UE_BTN_HEIGHT,
            "Apply",
            PBTN_TALIGN_CENTER,
            NULL,
            (void *)&univ_edit_win.apply_btn,
            UnivEditApplyPBCB
        );
        if(status)
            return(-1);


	/* Alias Prompt. */
        status = PromptInit(
            &univ_edit_win.alias_prompt, 
            univ_edit_win.toplevel,
            10,
	    (1 * 10) + (0 * 30),
	    (int)univ_edit_win.width - 20,
	    30,
            PROMPT_STYLE_FLUSHED,
            "Alias:",
            UNIV_MAX_ALIAS_LEN,
	    0,
            NULL
        );
        if(status != 0)
            return(-1);

        /* URL Prompt. */
        status = PromptInit(
            &univ_edit_win.url_prompt,
            univ_edit_win.toplevel,
            10,
            (2 * 10) + (1 * 30),
            (int)univ_edit_win.width - 20,
            30,
            PROMPT_STYLE_FLUSHED,
            "URL:",
            UNIV_MAX_URL_LEN,
            0,
            NULL
        );
        if(status != 0)
            return(-1);

        /* Comments Prompt. */
        status = PromptInit(
            &univ_edit_win.comments_prompt,
            univ_edit_win.toplevel,
            10,
            (3 * 10) + (2 * 30),
            (int)univ_edit_win.width - 20,
            30,
            PROMPT_STYLE_FLUSHED,
            "Comments:",
            UNIV_MAX_COMMENT_LEN,
            0,
            NULL
        );
        if(status)
            return(-1);

	/* Link prompts togeather. */
	univ_edit_win.alias_prompt.next = &univ_edit_win.url_prompt;
	univ_edit_win.alias_prompt.prev = &univ_edit_win.comments_prompt;

        univ_edit_win.url_prompt.next = &univ_edit_win.comments_prompt;
        univ_edit_win.url_prompt.prev = &univ_edit_win.alias_prompt;

        univ_edit_win.comments_prompt.next = &univ_edit_win.alias_prompt;
        univ_edit_win.comments_prompt.prev = &univ_edit_win.url_prompt;


	return(0);
}



int UnivEditWinDraw()
{
	int y;
	char *strptr;
	char timestr[256];
	int entry_num;
	univ_entry_struct *univ_entry_ptr;
	win_t w;
	win_attr_t wattr;


	/* Map as needed. */
	if(!univ_edit_win.map_state)
	{
	    OSWMapRaised(univ_edit_win.toplevel);

	    PBtnMap(&univ_edit_win.ok_btn);
            PBtnMap(&univ_edit_win.cancel_btn);
            PBtnMap(&univ_edit_win.apply_btn);
            PBtnMap(&univ_edit_win.touch_btn);

	    PromptMap(&univ_edit_win.alias_prompt);
            PromptMap(&univ_edit_win.url_prompt);
            PromptMap(&univ_edit_win.comments_prompt);

	    univ_edit_win.map_state = 1;
	    univ_edit_win.visibility_state = VisibilityUnobscured;
	}


	if(1)
	{
	    w = univ_edit_win.toplevel;

	    OSWGetWindowAttributes(w, &wattr);

	    OSWClearWindow(w);

	    /* Draw statistics text. */
            entry_num = univ_edit_win.univ_entry_num;
	    if(UnivIsAllocated(entry_num))
	    {
	        univ_entry_ptr = univ_entry[entry_num];
                OSWSetFgPix(widget_global.normal_text_pix);

	        /* Draw last connected string. */
	        strptr = UnivEditWinTimeAgoStr(
		    cur_systime,
		    univ_entry_ptr->last_connected
	        );
	        sprintf(
		    timestr,
		    "Last Visited: %s",
		    strptr
		);
		OSWDrawString(
		    w,
		    14,
		    ((3 * 30) + (4 * 10) + 12 + 5),
		    timestr
	        );
	    }

	    /* Draw horizontal rule. */
	    y = (int)wattr.height - UE_BTN_HEIGHT - 25;
	    OSWSetFgPix(widget_global.surface_shadow_pix);
	    OSWDrawLine(w, 0, y, wattr.width, y);

            OSWSetFgPix(widget_global.surface_highlight_pix);
            OSWDrawLine(w, 0, y + 1, wattr.width, y + 1);
	}


	return(0);
}



int UnivEditWinManage(event_t *event)
{
	int events_handled = 0;


        if(event == NULL)
	    return(events_handled);


	if(!univ_edit_win.map_state &&
           (event->type != MapNotify)
	)
            return(events_handled);


	switch(event->type)
	{
	  /* ********************************************************* */
	  case KeyPress:
	    /* Skip if not in focus. */
	    if(!univ_edit_win.is_in_focus)
		return(events_handled);

	    /* Enter key. */
            if((event->xkey.keycode == osw_keycode.enter) ||
               (event->xkey.keycode == osw_keycode.np_enter)
            )
	    {
		UnivEditOKPBCB(NULL);

		events_handled++;
		return(events_handled);
	    }
            /* Escape. */
            else if(event->xkey.keycode == osw_keycode.esc)
            {
		UnivEditCancelPBCB(NULL);

                events_handled++;
                return(events_handled);
            }
            break;

          /* ********************************************************* */
          case KeyRelease:
            /* Skip if not in focus. */
            if(!univ_edit_win.is_in_focus)
                return(events_handled);
 
	    break;

          /* ********************************************************* */
	  case ButtonPress:
	    if(event->xany.window == univ_edit_win.toplevel)
	    {

	    }
	    break;

          /* ********************************************************* */
          case Expose:
            if(event->xany.window == univ_edit_win.toplevel)
            {
		UnivEditWinDraw();

		events_handled++;
		return(events_handled);
	    }
	    break;

          /* ********************************************************* */
          case UnmapNotify:
            if(event->xany.window == univ_edit_win.toplevel)
            {
                UnivEditWinUnmap();

                events_handled++;
                return(events_handled);
            }
            break;

          /* ********************************************************* */
          case MapNotify:
            if(event->xany.window == univ_edit_win.toplevel)
            {
		if(!univ_edit_win.map_state)
                    UnivEditWinMap();

                events_handled++;
                return(events_handled);
            }
            break;

          /* ********************************************************* */
          case VisibilityNotify:
            if(event->xany.window == univ_edit_win.toplevel)
            {
                univ_edit_win.visibility_state = event->xvisibility.state;
 
                events_handled++;
                return(events_handled);
            }
            break;

          /* ********************************************************* */
          case FocusOut:
            if(event->xany.window == univ_edit_win.toplevel)
            {
                univ_edit_win.is_in_focus = 0;

                events_handled++;
                return(events_handled);
            }
            break;

          /* ********************************************************* */
          case FocusIn:
            if(event->xany.window == univ_edit_win.toplevel)
            {
                univ_edit_win.is_in_focus = 1;

                events_handled++;
		return(events_handled);
            }
            break;

          /* ********************************************************* */
          case ClientMessage:
	    if(OSWIsEventDestroyWindow(univ_edit_win.toplevel, event))
            {
		/* Unmap universe edit window. */
                UnivEditWinUnmap();

                events_handled++;
                return(events_handled);
            }
            break;
	}

	/* Redraw if event was handled. */
	if(events_handled > 0)
	{
	    UnivEditWinDraw();
	}


	/* Manage widgets. */
	if(events_handled == 0)
            events_handled += PBtnManage(&univ_edit_win.ok_btn, event);

        if(events_handled == 0)
	    events_handled += PBtnManage(&univ_edit_win.cancel_btn, event);

        if(events_handled == 0)
            events_handled += PBtnManage(&univ_edit_win.apply_btn, event);

        if(events_handled == 0)
            events_handled += PBtnManage(&univ_edit_win.touch_btn, event);


        if(events_handled == 0)
            events_handled += PromptManage(&univ_edit_win.alias_prompt, event);

        if(events_handled == 0)
            events_handled += PromptManage(&univ_edit_win.url_prompt, event);

        if(events_handled == 0)
            events_handled += PromptManage(&univ_edit_win.comments_prompt, event);



	return(events_handled);
}



void UnivEditWinMap()
{
	/* Unfocus all XSW windows. */
	XSWDoUnfocusAllWindows();
	

	/* Unfocus all prompts on universe edit window. */
	UnivEditUnfocusPrompts();

	/* Mark alias prompt and set it into focus. */
	PromptMarkBuffer(&univ_edit_win.alias_prompt, PROMPT_POS_END);
	PromptUnmarkBuffer(&univ_edit_win.url_prompt, PROMPT_POS_END);
	PromptUnmarkBuffer(&univ_edit_win.comments_prompt, PROMPT_POS_END);

	univ_edit_win.map_state = 0;
	UnivEditWinDraw();
	univ_edit_win.is_in_focus = 1;

	/* Set alias prompt in focus. */
        univ_edit_win.alias_prompt.is_in_focus = 1;
        univ_edit_win.url_prompt.is_in_focus = 0;
        univ_edit_win.comments_prompt.is_in_focus = 0;


        /* Restack all XSW windows. */
        XSWDoRestackWindows();


	return;
}



void UnivEditWinUnmap()
{
        PromptUnmap(&univ_edit_win.alias_prompt);
        PromptUnmap(&univ_edit_win.url_prompt);	
	PromptUnmap(&univ_edit_win.comments_prompt);

	PBtnUnmap(&univ_edit_win.ok_btn);
        PBtnUnmap(&univ_edit_win.cancel_btn);
        PBtnUnmap(&univ_edit_win.apply_btn);


	OSWUnmapWindow(univ_edit_win.toplevel);
	univ_edit_win.map_state = 0;
	univ_edit_win.is_in_focus = 0;
	univ_edit_win.visibility_state = VisibilityFullyObscured;


	return;
}



void UnivEditWinDestroy()
{
	if(IDC())
	{
	    PBtnDestroy(&univ_edit_win.ok_btn);
            PBtnDestroy(&univ_edit_win.cancel_btn);
            PBtnDestroy(&univ_edit_win.apply_btn);
	    PBtnDestroy(&univ_edit_win.touch_btn);

	    PromptDestroy(&univ_edit_win.alias_prompt);
            PromptDestroy(&univ_edit_win.url_prompt);
            PromptDestroy(&univ_edit_win.comments_prompt);

	    OSWDestroyWindow(&univ_edit_win.toplevel);
	}

        univ_edit_win.map_state = 0;
        univ_edit_win.x = 0;
        univ_edit_win.y = 0;
        univ_edit_win.width = 0;
        univ_edit_win.height = 0;
        univ_edit_win.is_in_focus = 0;
        univ_edit_win.visibility_state = VisibilityFullyObscured;
        univ_edit_win.disabled = False;

        univ_edit_win.univ_entry_num = -1;


	return;
}
