// widgets/whintwin.cpp
/*
                       Widget: Hint window management


	Functions:

	int HintWinIsDataAllocated(int n)
	int HintWinGetNumByWin(win_t ref_win)
	int HintWinInList(win_t ref_win)

	int HintWinInit()
        int HintWinDraw()
        int HintWinManage(event_t *event)
	void HintWinMap()
	void HintWinUnmap()
	void HintWinDestroy()

        int HintWinAddMessage(
                win_t ref_win,
                win_t parent_win,
                int x, int y,
                const char *mesg
        )
        void HintWinDeleteMessage(win_t ref_win)
        void HintWinDeleteAllMessages()
	int HintWinChangeMesg(win_t ref_win, const char *mesg)

	int HintWinSetSchedual(
	        long d_msec,
	        win_t ref_win
	)
	int HintWinSetSchedualMessage(
	        long d_msec,
	        win_t ref_win, 
	        const char *mesg
	)

	---



 */


#include "../include/string.h"
#include "../include/widget.h"


/*
 *	Size constants (in pixels):
 */
#define HW_CHAR_WIDTH	7
#define HW_CHAR_HEIGHT	14



/*
 *	Checks if hint window data number n is allocated.
 */
int HintWinIsDataAllocated(int n)
{
	if((hint_win_data == NULL) ||
           (n < 0) ||
           (n >= total_hint_win_datas)
	)
	    return(0);
	else if(hint_win_data[n] == NULL)
	    return(0);
	else
	    return(1);
}


/*
 *	Get hint window data number by referance window.
 */
int HintWinGetNumByWin(win_t ref_win)
{               
        int i;  
	hint_win_data_struct **ptr;


	if(ref_win == 0)
	    return(-1);
 
        for(i = 0, ptr = hint_win_data;
            i < total_hint_win_datas;
            i++, ptr++
	)
        {
            if(*ptr == NULL)
		continue;
            if((*ptr)->ref_win == ref_win)
		break;
        }
        if(i < total_hint_win_datas)
            return(i);
        else 
            return(-1);
}


/*
 *	Checks if ref_win already has a hint win data set
 *	for it.
 */
int HintWinInList(win_t ref_win)
{
        int i;
	hint_win_data_struct **ptr;


        if(ref_win == 0)
            return(0);

        for(i = 0, ptr = hint_win_data;
            i < total_hint_win_datas;
            i++, ptr++
	)
        {
            if(*ptr == NULL)
                continue;
            if((*ptr)->ref_win == ref_win)
                break;
        }
        if(i < total_hint_win_datas)
            return(1);
        else
            return(0);
}


/*
 *	Initializes the hint window.
 */
int HintWinInit()
{

	if(!IDC() ||
           (osw_gui[0].root_win == 0)
	)
	    return(-1);


	/* Reset values. */
	hint_win.map_state = 0;
        hint_win.x = 0;
        hint_win.y = 0;
        hint_win.width = 80;
        hint_win.height = 30;
        hint_win.visibility_state = VisibilityFullyObscured;
        hint_win.is_in_focus = 0;
        hint_win.disabled = False;
	hint_win.font = OSWQueryCurrentFont();

        hint_win.next_map = 0;


	/* ******************************************************* */

	if(
	    OSWCreateWindow(
		&hint_win.toplevel,
		osw_gui[0].root_win,
		hint_win.x, hint_win.y,
		hint_win.width, hint_win.height
	    )
	)
	    return(-1);

	/*   Client programs need to know about MotionNotify even on
	 *   the hint window.
	 */
	OSWSetWindowInput(
	    hint_win.toplevel,
	    PointerMotionMask | ExposureMask
	);

	/* WM Properties. */
	OSWSetWindowWMProperties(
	    hint_win.toplevel,
            "Hint window",
            "Hint window",
            widget_global.std_icon_pm,
            False,
            hint_win.x, hint_win.y,
            hint_win.width, hint_win.height,
            hint_win.width, hint_win.height,
            WindowFrameStyleNaked,
	    NULL, 0
	);


	hint_win.ref_win = 0;
	

	return(0);
}

/*
 *	Manages the hint window.
 */
int HintWinManage(event_t *event)
{
	int events_handled = 0;
	time_t ct;


	/*
         *   Note: The event information is not used and not needed here.
	 */

	/* If not mapped, check if it needs to be mapped. */
	if(!hint_win.map_state && (hint_win.next_map > 0))
	{
	    ct = MilliTime();
	    if(ct >= hint_win.next_map)
	    {
		HintWinMap();
		hint_win.next_map = 0;
	    }
	}


	return(events_handled);
}


/*
 *	Redraws the hint window.
 */
int HintWinDraw()
{
	int x, y, root_x, root_y;
	unsigned int width, height;
	int data_num;
	win_attr_t wattr;
        font_t *prev_font;
	hint_win_data_struct *hw_data_ptr;


	/* Get associated data number. */
        data_num = HintWinGetNumByWin(hint_win.ref_win);
        if(HintWinIsDataAllocated(data_num))
	    hw_data_ptr = hint_win_data[data_num];
	else
            return(0);

        if(hw_data_ptr->mesg == NULL)
            return(0);


	/* Map as needed. */
	if(!hint_win.map_state)
	{
	    /* Resize hint window to fit message. */
	    width = (strlongestline(hw_data_ptr->mesg) + 2)
                * HW_CHAR_WIDTH;
	    height = (strlines(hw_data_ptr->mesg) * HW_CHAR_HEIGHT) + 6;

	    OSWGetPointerCoords(
		osw_gui[0].root_win,
		&root_x, &root_y,
		&x, &y
	    );

	    OSWMoveResizeWindow(
		hint_win.toplevel,
		x, y + 16, width, height
	    );

	    OSWGetWindowAttributes(hint_win.toplevel, &wattr);
	    hint_win.x = wattr.x;
	    hint_win.y = wattr.y;
            hint_win.width = wattr.width;
            hint_win.height = wattr.height;


	    /* Destroy buffer (will be created below. */
	    OSWDestroyPixmap(&hint_win.toplevel_buf);


	    /* Map window. */
	    OSWMapRaised(hint_win.toplevel);

	    hint_win.map_state = 1;
	    hint_win.visibility_state = VisibilityUnobscured;
	}


        /* ********************************************************* */

        prev_font = OSWQueryCurrentFont();
        OSWSetFont(hint_win.font);


	/* Recreate buffer as needed. */
        OSWGetWindowAttributes(hint_win.toplevel, &wattr);
	if(hint_win.toplevel_buf == 0)
	{
            if(
                OSWCreatePixmap(
                    &hint_win.toplevel_buf,
                    wattr.width, wattr.height
                )
            )
                return(-1);
	}


	/* Redraw background. */
	OSWClearPixmap(
	    hint_win.toplevel_buf,
	    wattr.width, wattr.height,
	    (widget_global.force_mono) ?
            osw_gui[0].white_pix : widget_global.hint_bkg_pix
	);

	if(widget_global.force_mono)
	    OSWSetFgPix(osw_gui[0].black_pix);
	else
	    OSWSetFgPix(widget_global.hint_text_pix);
	OSWDrawString(
	    hint_win.toplevel_buf,
	    HW_CHAR_WIDTH, ((int)wattr.height / 2) + 5,
	    hw_data_ptr->mesg 
	);

	OSWPutBufferToWindow(hint_win.toplevel, hint_win.toplevel_buf);

        OSWSetFont(prev_font);


	return(0);
}


void HintWinMap()
{
	hint_win.map_state = 0;
	HintWinDraw();

	return;
}


void HintWinUnmap()
{
	OSWUnmapWindow(hint_win.toplevel);

	hint_win.map_state = 0;
        hint_win.visibility_state = VisibilityFullyObscured;
        hint_win.is_in_focus = 0;

	/* Stop any pending scheduals. */
	hint_win.next_map = 0;


	/* Destroy buffers. */
        OSWDestroyPixmap(&hint_win.toplevel_buf);


	return;
}


void HintWinDestroy()
{
        /* Delete all hint window messages. */
        HintWinDeleteAllMessages();


        if(IDC())
        {
            OSWDestroyWindow(&hint_win.toplevel);
            OSWDestroyPixmap(&hint_win.toplevel_buf);
        }


        hint_win.map_state = 0;  
        hint_win.visibility_state = VisibilityFullyObscured;
        hint_win.is_in_focus = 0; 
        hint_win.x = 0;
        hint_win.y = 0;
        hint_win.width = 0;
        hint_win.height = 0;
        hint_win.disabled = False;

        hint_win.next_map = 0;        
        hint_win.ref_win = 0;


        return; 
}


/*
 *	Add a new hint message for referance window ref_win.
 *
 *	If ref_win already has a message set, then the older message
 *	will be used and this new message won't ever be shown.
 */
int HintWinAddMessage(
	win_t ref_win,
        win_t parent_win,
	int x, int y,
        const char *mesg
)
{
	int data_num;
	int i;
	hint_win_data_struct **ptr;


	/* Is hint already set for ref_win? */
	if(HintWinInList(ref_win))
	{
	    fprintf(stderr,
 "HintWinAddMessage(): Warning: Hint already set for window 0x%.8x\n",
		(u_int32_t)ref_win
	    );
	}


	/* Sanitize total. */
	if(total_hint_win_datas < 0)
	    total_hint_win_datas = 0;


	/* Search for available pointer. */
	for(i = 0, ptr = hint_win_data;
            i < total_hint_win_datas;
            i++, ptr++
        )
	{
	    if(*ptr == NULL)
		break;
	}
	if(i < total_hint_win_datas)
	{
	    data_num = i;
	}
	else
	{
	    data_num = total_hint_win_datas;
	    total_hint_win_datas++;

	    hint_win_data = (hint_win_data_struct **)realloc(
		hint_win_data,
		total_hint_win_datas * sizeof(hint_win_data_struct *)
	    );
	    if(hint_win_data == NULL)
	    {
		total_hint_win_datas = 0;
		return(-1);
	    }

	    hint_win_data[data_num] = NULL;
	}

	/* Allocate new data number. */
	hint_win_data[data_num] = (hint_win_data_struct *)calloc(
	    1,
	    sizeof(hint_win_data_struct)
	);
	if(hint_win_data[data_num] == NULL)
	{
	    return(-1);
	}


	/* Set values. */
	hint_win_data[data_num]->ref_win = ref_win;

        hint_win_data[data_num]->mesg = StringCopyAlloc(mesg);


	return(data_num);
}



/*
 *	Sets a new message for the hint window refferanced by ref_win.
 *	if new_mesg is NULL no operation will be performed.
 */
int HintWinChangeMesg(win_t ref_win, const char *mesg)
{
        int data_num;
	hint_win_data_struct *hw_data_ptr;


	/* Error checks. */
	if((ref_win == 0) ||
           (mesg == NULL)
	)
	    return(-1);

        /* Get match. */
        data_num = HintWinGetNumByWin(ref_win);
        if(HintWinIsDataAllocated(data_num))
	    hw_data_ptr = hint_win_data[data_num];
	else
            return(-1);


	/* Set new message. */
	free(hw_data_ptr->mesg);
	hw_data_ptr->mesg = StringCopyAlloc(mesg);


	return(0);
}



/*
 *	Deletes a hint window message associated with window
 *	ref_win.
 */
void HintWinDeleteMessage(win_t ref_win)
{
	int i;
        hint_win_data_struct **ptr;


	if(ref_win == 0)
	    return;


	/* Delete hint window message that matches referance window. */
	for(i = 0, ptr = hint_win_data;
            i < total_hint_win_datas;
            i++, ptr++
        )
	{
	    if(*ptr == NULL)
		continue;
	    if((*ptr)->ref_win == ref_win)
		break;
	}
	if(i < total_hint_win_datas)
	{
	    /* Got matching referance window, delete message and struct. */

	    free(hint_win_data[i]->mesg);
	    free(hint_win_data[i]);
	    hint_win_data[i] = NULL;
	}


	return;
}


/*
 *	Deletes all hint window messages.
 */
void HintWinDeleteAllMessages()
{
        int i;
	hint_win_data_struct **ptr;


        for(i = 0, ptr = hint_win_data;
            i < total_hint_win_datas;
            i++, ptr++
        )
        {
            if(*ptr == NULL)
		continue;

            free((*ptr)->mesg);
            free(*ptr);
        }

	free(hint_win_data);
	hint_win_data = NULL;

	total_hint_win_datas = 0;


	return;
}


/*
 *	Schedual next hint window mapping from now + d_msec
 *	(in milliseconds) to display hint message for referance
 *	window ref_win.
 */
int HintWinSetSchedual(
        long d_msec,  
        win_t ref_win
)
{
	/* Error checks. */
        if((hint_win.toplevel == 0) ||
           (ref_win == 0)
        )
            return(-1);

	/* d_msec cannot be negative. */
	if(d_msec < 0)
	    d_msec = 0;

        /* Schedual next map and set referace windowl. */
        hint_win.next_map = MilliTime() + d_msec;
        hint_win.ref_win = ref_win;


        return(0);
}


/*
 *	Same as HintWinSetSchedual() except that it changes the message
 *	associated for ref_win, the previous message will be deleted.
 *
 *	If there is no message previously set for ref_win, a new
 *	message will be set.
 */
int HintWinSetSchedualMessage(
        long d_msec,
        win_t ref_win,
	const char *mesg
)
{
	int i;


        /* Error checks. */
        if((hint_win.toplevel == 0) ||
           (ref_win == 0)
        )
            return(-1);

        /* d_msec cannot be negative. */
        if(d_msec < 0)
            d_msec = 0;

	/* Schedual next map and set referace windowl. */
        hint_win.next_map = MilliTime() + d_msec;
        hint_win.ref_win = ref_win;

	/* Does ref_win exist in hint_win_data? */
	i = HintWinGetNumByWin(ref_win);
        if(HintWinIsDataAllocated(i))
        {
            /* Change message for existing hint_win_data. */
            HintWinChangeMesg(ref_win, mesg);
        }
	else
	{
	    /* No hint_win_data exists for ref_win, create one. */
	    HintWinAddMessage(
                ref_win,
		0,
		0, 0,
                mesg
	    );
	}


	return(0);
}




