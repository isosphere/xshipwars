/*
                         Message window management

	Functions:

	int MesgWinLongestLine(mesgwin_struct *mw)
	void MesgWinUpdateMark(
		mesgwin_struct *mw,
		int start_x, int start_y,
                int end_x, int end_y
	)
	void MesgWinUnmarkAll(mesgwin_struct *mw)
	void MesgWinPutDDE(mesgwin_struct *mw)

	int MesgWinAdd(
		mesgwin_struct *mw,
		char *string,
		pixel_t color
	)

	int MesgWinInit(mesgwin_struct *mw, int argc, char *argv[])
	void MesgWinResize(mesgwin_struct *mw)
	void MesgWinDraw(mesgwin_struct *mw)
	int MesgWinManage(mesgwin_struct *mw, event_t *event)
	void MesgWinMap(mesgwin_struct *mw)
	void MesgWinUnmap(mesgwin_struct *mw)
	void MesgWinDestroy(mesgwin_struct *mw)

	---


 */

#include <stdio.h>

#include "../include/osw-x.h"
#include "../include/widget.h"

#include "mon.h"
#include "mesgwin.h"


#define MIN(a,b)	((a) < (b) ? (a) : (b))
#define MAX(a,b)	((a) > (b) ? (a) : (b))


#define MESGWIN_MARGIN		10

#define MESGWIN_CHAR_WIDTH	7
#define MESGWIN_CHAR_HEIGHT	14

#define MESGWIN_LINE_SPACING	(MESGWIN_CHAR_HEIGHT + 2)

#define MESGWIN_DEF_WIDTH	640
#define MESGWIN_DEF_HEIGHT	480

#define MESGWIN_DEF_LINES	200	/* Lines. */
#define MESGWIN_DEF_LINE_LEN	256	/* Characters. */


/*
 *      Local pointer drag records:
 */
static int      sel_start_x,
                sel_start_y;
static bool_t   button1_state;
/*
                button2_state,
                button3_state;
 */

int MesgWinLongestLine(mesgwin_struct *mw);
void MesgWinUpdateMark(
	mesgwin_struct *mw,
	int start_x, int start_y,
	int end_x, int end_y
);
void MesgWinUnmarkAll(mesgwin_struct *mw);
void MesgWinPutDDE(mesgwin_struct *mw);


/*
 *	Updates the longest line member in mw.
 */
int MesgWinLongestLine(mesgwin_struct *mw)
{
	int i, n;
	mesgwin_mesg_struct **mesg_ptr;


	if(mw == NULL)
	    return(0);

	mw->longest_line = 0;


	for(i = 0, mesg_ptr = mw->message;
            i < mw->total_messages;
	    i++, mesg_ptr++
	)
	{
	    if(*mesg_ptr == NULL)
		continue;
	    if((*mesg_ptr)->mesg == NULL)
		continue;

	    n = strlen((*mesg_ptr)->mesg);

	    if(n > mw->longest_line)
		mw->longest_line = n;
	}


	return(mw->longest_line);
}


/*
 *	Procedure to mark.
 */
void MesgWinUpdateMark(
	mesgwin_struct *mw,
	int start_x, int start_y,
	int end_x, int end_y
)
{
        int i, len;
        int x, y;
        int start_line, end_line;
        int delta_lines;
	mesgwin_mesg_struct **mesg;


	if(mw == NULL)
	    return;

	mesg = mw->message;


	/* Add x scroll bar position to x coordinates. */
	start_x += mw->sb.x_win_pos;
	end_x += mw->sb.x_win_pos;


        /* Swap start and end as needed. */
        if(((start_x + 1) * (start_y + 1)) >
           ((end_x + 1) * (end_y + 1))
        )
        {
            x = start_x;
            y = start_y;

            start_x = end_x;
            start_y = end_y;
             
            end_x = x;
            end_y = y;
        }


        /* Get starting line. */
        start_line = (start_y - MESGWIN_MARGIN +
	    mw->sb.y_win_pos) / MESGWIN_LINE_SPACING;
        if(start_line >= mw->total_messages)
            start_line = mw->total_messages - 1;
        if(start_line < 0)
            start_line = 0;


        /* Calculate end line. */
        end_line = (end_y - MESGWIN_MARGIN +
            mw->sb.y_win_pos) / MESGWIN_LINE_SPACING;
        if(end_line >= mw->total_messages)
            end_line = mw->total_messages - 1;
        if(end_line < 0)
            end_line = 0;


        /* Calculate delta lines. */
        delta_lines = end_line - start_line;

        /*   Begin marking.
         *
         *   Remember that start_line is equal or greater than
         *   end_line.
         */

        /* Single line mark? */
        if(start_line == end_line)
        {
            len = strlen(mesg[start_line]->mesg);

            x = (start_x - MESGWIN_MARGIN) / MESGWIN_CHAR_WIDTH;
            if(x >= len) x = len - 1;
            if(x < 0) x = 0;
            mesg[start_line]->sel_start = x;

            x = (end_x - MESGWIN_MARGIN) / MESGWIN_CHAR_WIDTH; 
            if(x >= len) x = len - 1;
            if(x < 0) x = 0;
            mesg[start_line]->sel_end = x;
        }
        else if(start_line < end_line)
        {
            /* Start line. */
            x = (start_x - MESGWIN_MARGIN) / MESGWIN_CHAR_WIDTH;
            len = strlen(mesg[start_line]->mesg);
            if(x >= len) x = len - 1;
            if(x < 0) x = 0;

            mesg[start_line]->sel_start = x;
            mesg[start_line]->sel_end = len - 1;

            /* Mark lines in between. */
            for(i = start_line + 1; i < end_line; i++)
            {
                mesg[i]->sel_start = 0;
                mesg[i]->sel_end = MAX(
                    strlen(mesg[i]->mesg) - 1,
                    0
                );
            }

            /* End line. */
            x = (end_x - MESGWIN_MARGIN) / MESGWIN_CHAR_WIDTH;
            len = strlen(mesg[end_line]->mesg);
            if(x >= len) x = len - 1;
            if(x < 0) x = 0;

            mesg[end_line]->sel_start = 0;
            mesg[end_line]->sel_end = x;
        }


        return;
}

/*
 *	Unmarks all lines in message window.
 */
void MesgWinUnmarkAll(mesgwin_struct *mw)
{
	int i;
	mesgwin_mesg_struct **ptr;


	if(mw == NULL)
	    return;


	for(i = 0, ptr = mw->message;
            i < mw->total_messages;
	    i++, ptr++
	)
	{
	    if(*ptr == NULL)
		continue;

	    (*ptr)->sel_start = -1;
	    (*ptr)->sel_end = -1;
	}


	return;
}


void MesgWinPutDDE(mesgwin_struct *mw)
{
        int i, delta;
        char *buf = NULL;
	mesgwin_mesg_struct **mesg;
        int buf_pos = 0;
        int buf_len = 0;


	if(mw == NULL)
	    return;


        for(i = 0, mesg = mw->message;
            i < mw->total_messages;
            i++, mesg++
	)
        {
	    if(*mesg == NULL)
		continue;
	    if((*mesg)->mesg == NULL)
		continue;

            if(((*mesg)->sel_start < 0) ||
               ((*mesg)->sel_end < 0)
            )
                continue; 
        
            /* Sanitize selected start and end positions. */
            if((*mesg)->sel_start >= (*mesg)->mesg_len)
                (*mesg)->sel_start = (*mesg)->mesg_len - 1;
            if((*mesg)->sel_end >= (*mesg)->mesg_len)
                (*mesg)->sel_end = (*mesg)->mesg_len - 1;

            /* Get delta size of selection. */
            delta = (*mesg)->sel_end - (*mesg)->sel_start;
            /* delta must be 1 or greater. */
            if(delta <= 0)
                continue;
            /* Need to add 1 to delta. */
            delta += 1;
            if(((*mesg)->sel_start + delta) >= (*mesg)->mesg_len)
                delta = (*mesg)->mesg_len - (*mesg)->sel_start - 1;
            if(delta <= 0)
                continue;

            /* If this is second or greater line, add \n to buffer. */
            if(buf_len > 0)
            {
                buf_len += 1;
                buf = (char *)realloc(buf, buf_len * sizeof(char));
                if(buf == NULL)
                    return;
                buf_pos = buf_len - 1;

                buf[buf_pos] = '\n';
            }
               
            /* Increase buffer size. */
            buf_pos = buf_len;
            buf_len += delta;
            buf = (char *)realloc(buf, buf_len * sizeof(char));
            if(buf == NULL)
                return;
        
            memcpy(
                &buf[buf_pos],  /* Target. */
                &(*mesg)->mesg[
                    (*mesg)->sel_start   /* Source. */
                ],
                delta
            );
        }


        OSWPutDDE(buf, buf_len);

        free(buf);
        buf = NULL;


	return;
}



int MesgWinAdd(   
	mesgwin_struct *mw,
	char *string,
	pixel_t color
) 
{
	int i, n, l;
	int longest = 0;
	mesgwin_mesg_struct *src_ptr, *tar_ptr;


	if((mw == NULL) ||
	   (string == NULL)
	)
	    return(-1);


	/* Shift messages. */
	n = mw->total_messages - 1;
	for(i = 0; i < n; i++)
	{
	    tar_ptr = mw->message[i];
            src_ptr = mw->message[i + 1];

	    if((src_ptr == NULL) ||
               (tar_ptr == NULL)
	    )
		continue;

	    if((src_ptr->mesg == NULL) ||
               (tar_ptr->mesg == NULL)
	    )
                continue;

	    memcpy(
		tar_ptr->mesg,
		src_ptr->mesg,
		MIN(tar_ptr->mesg_len,
                    src_ptr->mesg_len
                ) * sizeof(char)
	    );

	    /* Update longest line. */
	    l = strlen(tar_ptr->mesg);
	    if(l > longest)
		longest = l;

            tar_ptr->pixel = src_ptr->pixel;
            /* Do not copy over mesg len. */
            tar_ptr->sel_start = src_ptr->sel_start;
            tar_ptr->sel_end = src_ptr->sel_end;
	}

	i = mw->total_messages - 1;
	if(i < 0)
	    tar_ptr = NULL;
	else
	    tar_ptr = mw->message[i];
	if(tar_ptr == NULL)
	    return(-1);

	if(tar_ptr->mesg == NULL)
	    return(-1);

	strncpy(
	    tar_ptr->mesg,
            string,
	    tar_ptr->mesg_len
	);
	tar_ptr->mesg[tar_ptr->mesg_len - 1] = '\0';


        l = strlen(tar_ptr->mesg);
        if(l > longest)
            longest = l;

	tar_ptr->sel_start = -1;
	tar_ptr->sel_end = -1;

	tar_ptr->pixel = color;


	/* Update longest line on mw. */
	mw->longest_line = longest;


	if(mw->map_state)
	{
            SBarDraw(
                &mw->sb,
                mw->width, mw->height,
                (mw->longest_line * MESGWIN_CHAR_WIDTH) + 30,
                (mw->total_messages * MESGWIN_LINE_SPACING) + 30
            );

	    MesgWinDraw(mw);
	}

	return(0);
}


int MesgWinInit(mesgwin_struct *mw, int argc, char *argv[])
{
	int i;
	mesgwin_mesg_struct *mesg_ptr;
	win_attr_t wattr;

	if(mw == NULL)
	    return(-1);


	mw->map_state = 0;
	mw->is_in_focus = 0;
	mw->visibility_state = VisibilityFullyObscured;

        /* Get position and sizes. */
        mw->x = osw_gui[0].def_toplevel_x;
        mw->y = osw_gui[0].def_toplevel_y;
        mw->width = MESGWIN_DEF_WIDTH;
        mw->height = MESGWIN_DEF_HEIGHT;

	mw->bkg = NULL;


	/* Parse arguments. */
        for(i = 1; i < argc; i++)
        {
            if(argv[i] == NULL)
                continue;


	}


        /* Create toplevel. */
        if(
            OSWCreateWindow(
                &mw->toplevel,
                osw_gui[0].root_win,
                mw->x, mw->y,
                mw->width, mw->height
            )
        )
            return(-1);

	WidgetCenterWindow(mw->toplevel, WidgetCenterWindowToRoot);
	OSWGetWindowAttributes(mw->toplevel, &wattr);
	mw->x = wattr.x;
	mw->y = wattr.y;
	mw->width = wattr.width;
	mw->height = wattr.height;

        OSWSetWindowWMProperties(
            mw->toplevel,
            "Messages",
	    "Messages",
            ((mon_image.monitor_icon_pm == 0) ?
                widget_global.std_icon_pm : mon_image.monitor_icon_pm),
            False,	/* Let WM set coordinates? */
            mw->x, mw->y,
            100, 100,
            osw_gui[0].display_width, osw_gui[0].display_height,
            WindowFrameStyleStandard,
            NULL, 0
        );
        OSWSetWindowInput(
            mw->toplevel,
            OSW_EVENTMASK_TOPLEVEL | ButtonPressMask | ButtonReleaseMask |
            PointerMotionMask | ExposureMask
        );
	WidgetSetWindowCursor(mw->toplevel, mon_cursor.text);

	/* Scroll bars. */
	if(
	    SBarInit(
		&mw->sb,
		mw->toplevel,
		mw->width, mw->height
	    )
	)
	    return(-1);


	/* Allocate memory for messages. */
	mw->total_messages = MESGWIN_DEF_LINES;
	mw->message = (mesgwin_mesg_struct **)calloc(
	    1,
	    mw->total_messages * sizeof(mesgwin_mesg_struct *)
	);
	if(mw->message == NULL)
	{
	    mw->total_messages = 0;

	    MesgWinDestroy(mw);
	    return(0);
	}

	for(i = 0; i < mw->total_messages; i++)
	{
	    mw->message[i] = (mesgwin_mesg_struct *)calloc(
		1,
		sizeof(mesgwin_mesg_struct)
	    );
	    if(mw->message[i] == NULL)
		continue;

	    mesg_ptr = mw->message[i];

	    mesg_ptr->mesg_len = MESGWIN_DEF_LINE_LEN;
	    mesg_ptr->mesg = (char *)calloc(
		1,
		mesg_ptr->mesg_len * sizeof(char)
	    );
	    if(mesg_ptr->mesg == NULL)
		mesg_ptr->mesg_len = 0;

	    mesg_ptr->sel_start = -1;
	    mesg_ptr->sel_end = -1;

	    mesg_ptr->pixel = osw_gui[0].black_pix;
	}




	return(0);
}


void MesgWinResize(mesgwin_struct *mw)
{
	win_attr_t wattr;   


	if(mw == NULL)
            return;

	OSWGetWindowAttributes(mw->toplevel, &wattr);

	/* No change? */
	if(((unsigned int)wattr.width == mw->width) &&
           ((unsigned int)wattr.height == mw->height)
	)
	    return;

	mw->x = wattr.x;
        mw->y = wattr.y;
        mw->width = wattr.width;
        mw->height = wattr.height;




	SBarResize(
	    &mw->sb,
	    mw->width, mw->height
	);

	OSWSyncSharedImage(mw->bkg, mw->toplevel_buf);
	OSWDestroySharedImage(&mw->bkg);
	
	OSWDestroyPixmap(&mw->toplevel_buf);


	return;
}


void MesgWinDraw(mesgwin_struct *mw)
{
	int i;
        int lines_drawn, lines_visable;
        int c_pos;
        int line_x, line_y;
	mesgwin_mesg_struct **mesg;

        shared_image_t *simg_ptr;
	image_t *img_ptr;
        font_t *prev_font;
        win_attr_t wattr;

	win_t w;
	pixmap_t pixmap;


        if(mw == NULL)
            return;


        /* Map as needed. */
        if(!mw->map_state)
        {
            OSWMapRaised(mw->toplevel);
            mw->map_state = 1;

	    OSWGetWindowAttributes(mw->toplevel, &wattr);

	    SBarDraw(
		&mw->sb,
		wattr.width,
		wattr.height,
		(mw->longest_line * MESGWIN_CHAR_WIDTH) + 30,
		(mw->total_messages * MESGWIN_LINE_SPACING) + 30
	    );
        }

        /* Recreate buffers as needed. */
        if(mw->toplevel_buf == 0)
        {
            OSWGetWindowAttributes(mw->toplevel, &wattr);
            if(
                OSWCreatePixmap(&mw->toplevel_buf,
                    wattr.width, wattr.height
            ))
                return;
        }

	if(mw->bkg == NULL)
	{
	    OSWGetWindowAttributes(mw->toplevel, &wattr);
            if(
                OSWCreateSharedImage(
		     &simg_ptr,
                     wattr.width, wattr.height
		)
            )
                return;   

	    if(mon_image.mesg_bkg != NULL)
	    {
		img_ptr = mon_image.mesg_bkg;

		WidgetResizeImageBuffer(
                    osw_gui[0].depth,
                    simg_ptr->data,				 /* Target. */
                    reinterpret_cast<u_int8_t *>(img_ptr->data), /* Source. */
		    simg_ptr->width, simg_ptr->height,
                    img_ptr->width, img_ptr->height
		);
	    }

	    mw->bkg = simg_ptr;
	}

	prev_font = OSWQueryCurrentFont();





        if(1)
        {
	    w = mw->toplevel;
	    pixmap = mw->toplevel_buf;

	    OSWGetWindowAttributes(w, &wattr);
	    OSWSetFont(mon_font.messages);


	    /* Draw background. */
	    if(widget_global.force_mono)
	    {
		OSWClearPixmap(
		    pixmap,
		    wattr.width, wattr.height,
		    osw_gui[0].black_pix
		);
	    }
	    else
	    {
		OSWPutSharedImageToDrawable(mw->bkg, pixmap);
	    }

	    mesg = mw->message;

            /* Get starting line. */
            i = MAX(
                mw->sb.y_win_pos / MESGWIN_LINE_SPACING,
                0
            );

            line_x = MESGWIN_MARGIN - mw->sb.x_win_pos;
            line_y = MESGWIN_MARGIN -
		(mw->sb.y_win_pos % MESGWIN_LINE_SPACING);

            lines_drawn = 0;
            lines_visable = (int)wattr.height / MESGWIN_LINE_SPACING;

            while(lines_drawn < lines_visable)
            {
                if(i >= mw->total_messages)
                    break;

                if((mesg[i]->sel_start < 0) ||
                   (mesg[i]->sel_end < 0)
                )
                {
                    /* Draw unselected text. */
                    if(widget_global.force_mono)
                        OSWSetFgPix(osw_gui[0].white_pix);
                    else
                        OSWSetFgPix(mesg[i]->pixel);
                    OSWDrawString(
                        pixmap,
                        line_x,
                        line_y + ((14 / 2) + 5),
                        mesg[i]->mesg
                    );
                }
                else
                {
                    /* Draw marked background. */
                    if(widget_global.force_mono)
                        OSWSetFgPix(osw_gui[0].white_pix);
                    else
                        OSWSetFgPix(widget_global.surface_selected_pix);
                    OSWDrawSolidRectangle(
                        pixmap,
                        line_x + (mesg[i]->sel_start * MESGWIN_CHAR_WIDTH),
                        line_y,
                        MAX((mesg[i]->sel_end -
                             mesg[i]->sel_start + 1) * MESGWIN_CHAR_WIDTH,
                            2
                        ),
                        MESGWIN_LINE_SPACING
                    );

                    /* Draw first unmarked text. */
                    if(widget_global.force_mono)
                        OSWSetFgPix(osw_gui[0].white_pix);
                    else
                        OSWSetFgPix(mesg[i]->pixel);
                    OSWDrawStringLimited(
                        pixmap,
                        line_x,
                        line_y + ((14 / 2) + 5),
                        mesg[i]->mesg,
                        mesg[i]->sel_start
                    );
                    /* Draw marked text. */
                    c_pos = mesg[i]->sel_start;
                    if((c_pos >= 0) &&
                       (c_pos < mesg[i]->mesg_len)
                    )
                    {
                        if(widget_global.force_mono)
                            OSWSetFgPix(osw_gui[0].black_pix);
                        else
                            OSWSetFgPix(widget_global.selected_text_pix);
                        OSWDrawStringLimited(
                            pixmap,
                            line_x + (mesg[i]->sel_start *
                                MESGWIN_CHAR_WIDTH),
                            line_y + ((14 / 2) + 5),  
                            &mesg[i]->mesg[c_pos],
                            mesg[i]->sel_end -
                                mesg[i]->sel_start + 1
                        );
                    } 
                    /* Draw last unmarked text. */
                    c_pos = mesg[i]->sel_end + 1;
                    if((c_pos >= 0) &&
                       (c_pos < mesg[i]->mesg_len)
                    )
                    {
                        if(widget_global.force_mono)
                            OSWSetFgPix(osw_gui[0].white_pix);
                        else
                            OSWSetFgPix(mesg[i]->pixel);
                        OSWDrawStringLimited(
                            pixmap,
                            line_x + ((mesg[i]->sel_end + 1) *
                               MESGWIN_CHAR_WIDTH),
                            line_y + ((14 / 2) + 5),  
                            &mesg[i]->mesg[c_pos],
                            mesg[i]->mesg_len -   
                                mesg[i]->sel_end
                        );
                    }
                }


                /* Increment positions. */   
                line_y += MESGWIN_LINE_SPACING;
                i++;
                lines_drawn++;
            }

            /* Put buffer to window. */
            OSWPutBufferToWindow(w, pixmap);
	}


	OSWSetFont(prev_font);


	return;
}


int MesgWinManage(mesgwin_struct *mw, event_t *event)
{

	int events_handled = 0;
                
                
        if((mw == NULL) ||
           (event == NULL)
        )
            return(events_handled);

        if(!mw->map_state &&
           (event->type != MapNotify)
        )
            return(events_handled);
        

        /* Manage events for monitor normally. */
        switch(event->type)
        {
          case ButtonPress:
            if(event->xany.window == mw->toplevel)
            {
                /* Put scroll bar into focus. */
                mw->sb.is_in_focus = 1;

                /* Start mark text. */
                if(event->xbutton.button == Button1)
                {
                    MesgWinUnmarkAll(mw);

                    button1_state = True;
                    sel_start_x = event->xbutton.x;
                    sel_start_y = event->xbutton.y;

                    MesgWinDraw(mw);

                    events_handled++;
                }
		return(events_handled);
            }
            break;

          case ButtonRelease:
            if(event->xany.window == mw->toplevel)
            {
                MesgWinPutDDE(mw);
                button1_state = False;

                MesgWinDraw(mw);
                events_handled++;
		return(events_handled);
            }
            break;

          case MotionNotify:
            if(event->xany.window == mw->toplevel)
            {
                /* Mark text. */
                if(button1_state)
                {
                    MesgWinUnmarkAll(mw);

                    MesgWinUpdateMark(
			mw,
                        sel_start_x,
                        sel_start_y,
                        event->xmotion.x,
                        event->xmotion.y
                    );

                    MesgWinDraw(mw);

                    events_handled++;
		    return(events_handled);
                }
            }
            break;

          case Expose:
            if(event->xany.window == mw->toplevel)
            {
		MesgWinDraw(mw);

                events_handled++;
		return(events_handled);
            }
            break;

          case FocusIn:
            if(event->xany.window == mw->toplevel)
            {
                mw->is_in_focus = 1;  
                events_handled++;
                return(events_handled);
            }
            break;

          case FocusOut:
            if(event->xany.window == mw->toplevel)
            {
                mw->is_in_focus = 0;
                events_handled++;
                return(events_handled);
            }
            break;

          case VisibilityNotify:
            if(event->xany.window == mw->toplevel)
            {
                mw->visibility_state = event->xvisibility.state;

                events_handled++;
                return(events_handled);
            }
	    break;

          case MapNotify:
            if(event->xany.window == mw->toplevel)
            {
                if(!mw->map_state)
                    MesgWinMap(mw);

                events_handled++;
                return(events_handled);
            }
            break;

          case UnmapNotify:
            if(event->xany.window == mw->toplevel)
            {
                if(mw->map_state)
                    MesgWinUnmap(mw);

                events_handled++;
                return(events_handled);
            }
            break;

	  case ConfigureNotify:
	    if(event->xany.window == mw->toplevel)
            {
                MesgWinResize(mw);

                events_handled++;
                return(events_handled);
            }
            break;

          case ClientMessage:
            if(OSWIsEventDestroyWindow(mw->toplevel, event))
            {
                MesgWinUnmap(mw);

                events_handled++;  
                return(events_handled);
            }
            break;
        }


	/* Manage scroll bar. */
	if(events_handled == 0)
	{
	    events_handled += SBarManage(
		&mw->sb,
		mw->width, mw->height,
                (mw->longest_line * MESGWIN_CHAR_WIDTH) + 30,
                (mw->total_messages * MESGWIN_LINE_SPACING) + 30,
		event
            );
	    if(events_handled > 0)
	    {
		MesgWinDraw(mw);
	    }
	}


	return(events_handled);
}

void MesgWinMap(mesgwin_struct *mw)
{
	win_attr_t wattr;


        if(mw == NULL)
            return;

	/* Scroll to bottom. */
	OSWGetWindowAttributes(mw->toplevel, &wattr);
	mw->sb.y_win_pos = ((mw->total_messages * MESGWIN_LINE_SPACING)
            + 30) - (int)wattr.height;

	mw->map_state = 0;
	MesgWinDraw(mw);


	return;
}

void MesgWinUnmap(mesgwin_struct *mw)
{
        if(mw == NULL)
            return;


	OSWUnmapWindow(mw->toplevel);
	mw->map_state = 0;
        mw->is_in_focus = 0;
        mw->visibility_state = VisibilityFullyObscured;

	OSWSyncSharedImage(mw->bkg, mw->toplevel_buf);
	OSWDestroySharedImage(&mw->bkg);

	OSWDestroyPixmap(&mw->toplevel_buf);


	return;
}

void MesgWinDestroy(mesgwin_struct *mw)
{
	int i;

        if(mw == NULL)
            return;


	/* Free all messages. */
	for(i = 0; i < mw->total_messages; i++)
	{
	    if(mw->message[i] == NULL)
		continue;

	    free(mw->message[i]->mesg);

	    free(mw->message[i]);
	}
	free(mw->message);
	mw->message = NULL;

	mw->total_messages = 0;


	/* Deallocate resources from GUI. */
	SBarDestroy(&mw->sb);

	OSWSyncSharedImage(mw->bkg, mw->toplevel_buf);
	OSWDestroySharedImage(&mw->bkg);

        OSWDestroyWindow(&mw->toplevel);
        OSWDestroyPixmap(&mw->toplevel_buf);


	mw->map_state = 0;
	mw->is_in_focus = 0;
	mw->x = 0;
	mw->y = 0;
	mw->width = 0;
	mw->height = 0;
	mw->visibility_state = VisibilityFullyObscured;


	return;
}
