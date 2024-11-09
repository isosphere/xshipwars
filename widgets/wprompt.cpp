// widgets/wprompt.cpp
/*
                      Widget: Text Prompt

	Functions:

	int PROMPT_CHARS_VIS(prompt_window_struct *prompt)
	int PromptDDEStoreASCIISeg(char *buf, int len)
	void PromptChangeName(prompt_window_struct *prompt, char *name)
	int PromptMarkBuffer(prompt_window_struct *prompt, int opt)
	void PromptUnmarkBuffer(prompt_window_struct *prompt, int opt)

	double PromptGetF(prompt_window_struct *prompt)
	int PromptGetI(prompt_window_struct *prompt)
	long PromptGetL(prompt_window_struct *prompt)
	char *PromptGetS(prompt_window_struct *prompt)
	void PromptSetF(prompt_window_struct *prompt, double val)
	void PromptSetI(prompt_window_struct *prompt, int val)
	void PromptSetL(prompt_window_struct *prompt, long val)
	void PromptSetUL(prompt_window_struct *prompt, unsigned long val)
	void PromptSetS(prompt_window_struct *prompt, const char *val)

	void PromptRepeatRecordSet( 
	        prompt_window_struct *prompt,
	        long dsec, int op_code
	)
	void PromptRepeatRecordClear(void)
	void PromptSetNotifyFunction(int (*func_notify)(prompt_window_struct *))

	int PromptInit(
	        prompt_window_struct *prompt,
	        win_t parent,
        	int x, int y,
        	unsigned int width, unsigned int height,
        	int style,
        	const char *name,
        	unsigned int buf_len,
        	int hist_bufs,
        	int (*func_cb)(char *)
	)
        void PROMPT_UNMARK_REDRAW_ALL(prompt_window_struct *except_prompt)
        void PROMPT_UNFOCUS_REDRAW_ALL(prompt_window_struct *except_prompt)
	int PromptDraw(prompt_window_struct *prompt, int amount)
	int PromptManage(prompt_window_struct *prompt, event_t *event)
	void PromptMap(prompt_window_struct *prompt)   
	void PromptUnmap(prompt_window_struct *prompt)   
	void PromptDestroy(prompt_window_struct *prompt)

	int PromptManageRepeat(event_t *event)

	---

 */

#include "../include/widget.h"

#ifndef MAX
#define MIN(a,b)        (((a) < (b)) ? (a) : (b))
#define MAX(a,b)	(((a) > (b)) ? (a) : (b))
#endif
/* #define MIN(a,b)	((a) < (b)) ? (a) : (b)*/
/* #define MAX(a,b)	((a) > (b)) ? (a) : (b) */


/* Is a within inclusive range of min and max. */
#define INRANGEINC(a,min,max) ( (a >= min) && (a <= max) )


/*
 *	Size constants (in pixels):
 */
#define PROMPT_CHAR_WIDTH       7
#define PROMPT_CHAR_HEIGHT      14

#define PROMPT_MARGIN           3

#define PROMPT_MIN_WIDTH        10
#define PROMPT_MIN_HEIGHT       10



/*  
 *      Prompt repeat record:
 */
#define PROMPT_OP_CODE_NONE             0
#define PROMPT_OP_CODE_SCROLL_LEFT      1
#define PROMPT_OP_CODE_SCROLL_RIGHT     2

typedef struct {
 
    prompt_window_struct *prompt;
    long next_repeat;
    int op_code;                /* What to do. */

    int (*func_notify)(prompt_window_struct *);

} prompt_repeat_record_struct;
prompt_repeat_record_struct prompt_repeat_record[1];

namespace static_wprompt {
	int mark_origin_pos;
}



/*
 *	Returns the number of characters visiable at one time
 *	on the prompt.
 */
int PROMPT_CHARS_VIS(prompt_window_struct *prompt)
{
	win_attr_t wattr;
	int cvis;


	if(!IDC() ||
           (prompt == NULL)
	)
	    return(0);

	if((prompt->buf == NULL) ||
           (prompt->text_area == 0)
	)
	    return(0);


	OSWGetWindowAttributes(prompt->text_area, &wattr);
        cvis = MAX(
	    ((int)wattr.width - (PROMPT_MARGIN * 2)) / PROMPT_CHAR_WIDTH,
            0
        );


	return(cvis);
}



/*
 *	Macro to put buf of length len into DDE
 */
int PromptDDEStoreASCIISeg(char *buf, int len)
{
	int bytes_stored = 0;


	/* Error checks. */
	if(!IDC() ||
           (buf == NULL) ||
           (len <= 0)
	)
	    return(0);


	/* Store work memory to DDE. */
	OSWPutDDE(buf, len);
	bytes_stored = len;

	return(bytes_stored);
}



/*
 *	Changes the prompt's name.
 *
 *	If prompt's style is PROMPT_STYLE_NOBORDER then
 *	nothing will be done.
 */
void PromptChangeName(prompt_window_struct *prompt, char *name)
{
	int x = 0;
	int y = 0;
	int len;
	unsigned int width = 0;
	unsigned int height = 0;
	win_attr_t wattr;


	if(!IDC() ||
           (prompt == NULL) ||
           (name == NULL)
	)
	    return;

	if(prompt->toplevel == 0)
	    return;


	/* ******************************************************* */

	/* Change prompt name. */
	len = strlen(name);
	free(prompt->name); prompt->name = NULL;

	prompt->name = (char *)calloc(1, (len + 1) * sizeof(char));
	if(prompt->name == NULL)
	    return;
	strncpy(prompt->name, name, len);


	/* Get attributes and values. */
        len = (prompt->name == NULL) ? 0 : strlen(prompt->name);
        OSWGetWindowAttributes(prompt->toplevel, &wattr);

        /* Toplevel buf pixmap. */
        OSWDestroyPixmap(&prompt->toplevel_buf);  
        if(
            OSWCreatePixmap(
                &prompt->toplevel_buf, 
                wattr.width, 
                wattr.height
            )
        )
            return;

	/* Text area. */
	if((prompt->style == PROMPT_STYLE_FLUSHED) ||
           (prompt->style == PROMPT_STYLE_RAISED)
	)
	{
            x = ((int)len * PROMPT_CHAR_WIDTH) + (2 * PROMPT_MARGIN);
            y = PROMPT_MARGIN;
            width = MAX( (int)wattr.width - (PROMPT_MARGIN * 4)
                - ((int)len * PROMPT_CHAR_WIDTH), 1);
            height = MAX( (int)wattr.height - PROMPT_MARGIN - PROMPT_MARGIN, 1);
	}
	else if(prompt->style == PROMPT_STYLE_NOBORDER)
	{
	    x = 0; y = 0;
	    width = wattr.width;
	    height = wattr.height;
	}
        /* Move and resize text area. */
        OSWMoveResizeWindow(prompt->text_area, x, y, width, height);


        /* Recreate text area pixmap_t buffer. */
	OSWDestroyPixmap(&prompt->text_area_buf);
	if(
	    OSWCreatePixmap(
	        &prompt->text_area_buf,
	        width, height
	    )
	)
	    return;



	/* If currently mapped, then redraw. */
	if((prompt->map_state == 1) &&
	   (prompt->visibility_state != VisibilityFullyObscured)
	)
	{
	    PromptDraw(prompt, PROMPT_DRAW_AMOUNT_COMPLETE);
	}


	return;
}



/*
 *	Mark prompt buffer macro.
 *
 *	opt can be one of the following:
 *
 *	PROMPT_POS_CUR
 *	PROMPT_POS_START
 *	PROMPT_POS_END
 */
int PromptMarkBuffer(prompt_window_struct *prompt, int opt)
{
        int len, cvis;
                
                
        if(prompt == NULL)
            return(-1);
 
        if(prompt->buf == NULL)
            return(-1);


	/* Get length of buffer. */
	len = strlen(prompt->buf);
	if(len < 1)
	    return(0);
	if(len > (int)prompt->buf_len)
	    len = (int)prompt->buf_len;

	/* Mark entire buffer. */
	prompt->buf_sel_start = 0;
	prompt->buf_sel_end = len;


        /* Unmark and redraw other prompts. */
        PROMPT_UNMARK_REDRAW_ALL(prompt);

	/* Store buffer. */
	PromptDDEStoreASCIISeg(prompt->buf, len);

	switch(opt)
	{
	  case PROMPT_POS_START:
	    prompt->buf_pos = 0;
	    break;

	  case PROMPT_POS_END:
            /* Calculate total characters visable on text area. */
            cvis = PROMPT_CHARS_VIS(prompt);
	    prompt->buf_vis_pos = MAX(len - cvis, 0);
	    prompt->buf_pos = len;
	    break;

	  default:	/* PROMPT_POS_CUR */
	    break;
	}


	return(0);
}



/*
 *      Unmark prompt buffer macro.
 *
 *      opt can be one of the following:
 *
 *      PROMPT_POS_CUR
 *      PROMPT_POS_START
 *      PROMPT_POS_END
 */
void PromptUnmarkBuffer(prompt_window_struct *prompt, int opt)
{
        int len, cvis;

            
        if(prompt == NULL)
            return;

        if(prompt->buf == NULL)
            return;


	/* Get length of buffer. */
        len = strlen(prompt->buf);
        if(len < 1)
            return;
        if(len > (int)prompt->buf_len)
            len = (int)prompt->buf_len;

        /* Unmark buffer. */
        prompt->buf_sel_start = -1;
        prompt->buf_sel_end = -1;

        switch(opt)
        {
          case PROMPT_POS_START:
            prompt->buf_pos = 0;     
            break;
           
          case PROMPT_POS_END:
            /* Calculate total characters visable on text area. */
            cvis = PROMPT_CHARS_VIS(prompt);
            prompt->buf_vis_pos = MAX(len - cvis, 0);
            prompt->buf_pos = len;
            break;
        
          default:      /* PROMPT_POS_CUR */
            break;
        }


	return;
}


/*
 *	Get double value from prompt.
 */
double PromptGetF(prompt_window_struct *prompt)
{
        if(prompt == NULL)
            return(0);
        if(prompt->buf == NULL)
            return(0);
            
        return(atof(prompt->buf));
}

/*
 *	Get int value from prompt.
 */
int PromptGetI(prompt_window_struct *prompt)
{
        if(prompt == NULL)
            return(0);
        if(prompt->buf == NULL)
            return(0);

        return(atoi(prompt->buf));
}

/*
 *      Get long value from prompt.
 */
long PromptGetL(prompt_window_struct *prompt)
{
        if(prompt == NULL)
            return(0);
        if(prompt->buf == NULL)
            return(0);

        return(atol(prompt->buf));
}

/*      
 *	Get pointer to prompt's buffer (can return NULL).
 */
char *PromptGetS(prompt_window_struct *prompt)
{
        if(prompt == NULL)
            return(NULL);

        return(prompt->buf);
}


/*
 *	Set prompt to double value, redraws if mapped.
 */
void PromptSetF(prompt_window_struct *prompt, double val)
{
        char buf[80];


        if(prompt == NULL)
            return;

        if((prompt->buf == NULL) ||
           (prompt->buf_len < 1)
        )
            return;

        sprintf(buf, "%f", (float)val);
        strncpy(
            prompt->buf,
            buf,
            prompt->buf_len
        );
        prompt->buf[prompt->buf_len - 1] = '\0';
/*
	PromptMarkBuffer(prompt, PROMPT_POS_END);
 */
        if(prompt->map_state)
            PromptDraw(prompt, PROMPT_DRAW_AMOUNT_TEXTONLY);

        return;
}

/*
 *	Set prompt to int value, redraws if mapped.
 */
void PromptSetI(prompt_window_struct *prompt, int val)
{
	char buf[80];


        if(prompt == NULL)
            return;

        if((prompt->buf == NULL) || (prompt->buf_len < 1))
	    return;

        sprintf(buf, "%i", val);
	strncpy(prompt->buf, buf, prompt->buf_len);
	prompt->buf[prompt->buf_len - 1] = '\0';
/*
        PromptMarkBuffer(prompt, PROMPT_POS_END);
 */
        if(prompt->map_state)
            PromptDraw(prompt, PROMPT_DRAW_AMOUNT_TEXTONLY);
}

/*
 *      Set prompt to long value, redraws if mapped.
 */
void PromptSetL(prompt_window_struct *prompt, long val)
{
        char buf[80];


        if(prompt == NULL)
            return;

        if((prompt->buf == NULL) || (prompt->buf_len < 1))
            return;

        sprintf(buf, "%ld", val);
        strncpy(prompt->buf, buf, prompt->buf_len);
        prompt->buf[prompt->buf_len - 1] = '\0';
/*
        PromptMarkBuffer(prompt, PROMPT_POS_END);
 */
        if(prompt->map_state)
            PromptDraw(prompt, PROMPT_DRAW_AMOUNT_TEXTONLY);
}

/*
 *      Set prompt to unsigned long value, redraws if mapped.
 */
void PromptSetUL(prompt_window_struct *prompt, unsigned long val)
{
        char buf[80];


        if(prompt == NULL)
            return;

        if((prompt->buf == NULL) || (prompt->buf_len < 1))
            return;

        sprintf(buf, "%lu", val);
        strncpy(prompt->buf, buf, prompt->buf_len);
        prompt->buf[prompt->buf_len - 1] = '\0';
/*
        PromptMarkBuffer(prompt, PROMPT_POS_END);
 */
        if(prompt->map_state)
            PromptDraw(prompt, PROMPT_DRAW_AMOUNT_TEXTONLY);
}

/*
 *	Coppies string value prompt buffer, redraws if mapped.
 */
void PromptSetS(prompt_window_struct *prompt, const char *val) 
{
	if((val == NULL) ||
           (prompt == NULL)
	)
            return;
        if(prompt->buf == NULL)
            return;

	strncpy(
	    prompt->buf,
	    val,
	    prompt->buf_len
	);
/*
        PromptMarkBuffer(prompt, PROMPT_POS_END);
 */
        if(prompt->map_state)
            PromptDraw(prompt, PROMPT_DRAW_AMOUNT_TEXTONLY);

        return;
}



/*
 *	Scheduals a repeat for the prompt.
 */
void PromptRepeatRecordSet(
	prompt_window_struct *prompt,
	long dsec, int op_code
)
{
	prompt_repeat_record[0].prompt = prompt;
	prompt_repeat_record[0].next_repeat = MilliTime() + dsec;
	prompt_repeat_record[0].op_code = op_code;

	return;
}

/*
 *	Clears the prompt repeat record.
 */
void PromptRepeatRecordClear()
{
        prompt_repeat_record[0].prompt = NULL;
        prompt_repeat_record[0].next_repeat = 0;
        prompt_repeat_record[0].op_code = PROMPT_OP_CODE_NONE;
	prompt_repeat_record[0].func_notify = NULL;

	return;
}

/*
 *	Sets the prompt repeat notify handler function.
 */
void PromptSetNotifyFunction(int (*func_notify)(prompt_window_struct *))
{
        prompt_repeat_record[0].func_notify = func_notify;

	return;
}


/*
 *	Initializes a prompt.
 */
int PromptInit(
	prompt_window_struct *prompt,
	win_t parent,
	int x, int y,
	unsigned int width, unsigned int height,
	int style,
	const char *name,
	unsigned int buf_len,
	int hist_bufs,
	int (*func_cb)(char *)
)
{
	int i, len;


	if((parent == 0) || (prompt == NULL))
	    return(-1);


	/* Reset values. */
	prompt->map_state = 0;
	prompt->is_in_focus = 0;
	prompt->x = x;
	prompt->y = y;
        prompt->width = MAX((int)width, PROMPT_MIN_WIDTH);
        prompt->height = MAX((int)height, PROMPT_MIN_HEIGHT);
	prompt->visibility_state = VisibilityFullyObscured;
	prompt->disabled = False;
	prompt->style = style;
	prompt->font = widget_global.prompt_label_font;
	prompt->next = NULL;
	prompt->prev = NULL;


	/* Create toplevel. */
	if(
	    OSWCreateWindow(
	        &prompt->toplevel,
	        parent,
	        prompt->x, prompt->y,
	        prompt->width, prompt->height
	    )
	)
	    return(-1);
	prompt->toplevel_buf = 0;

	OSWSetWindowInput(
	    prompt->toplevel,
	    KeyPressMask | KeyReleaseMask |
	    ExposureMask | VisibilityChangeMask
	);


	/* Create text area. */
	len = ((name == NULL) ? 0 : strlen(name));
	if(
	    OSWCreateWindow(
	        &prompt->text_area,
	        prompt->toplevel,
	        (len * PROMPT_CHAR_WIDTH) + (2 * PROMPT_MARGIN),
	        PROMPT_MARGIN,
	        MAX((int)prompt->width - (PROMPT_MARGIN * 4)
		    - (len * PROMPT_CHAR_WIDTH), 1),
	        MAX((int)prompt->height - (2 * PROMPT_MARGIN), 1)
	    )
        )
            return(-1);

	prompt->text_area_buf = 0;

        OSWSetWindowInput(
	    prompt->text_area,
	    KeyPressMask | KeyReleaseMask |
            ButtonPressMask | ButtonReleaseMask | PointerMotionMask |
	    ExposureMask
	);
	WidgetSetWindowCursor(prompt->text_area, widget_global.text_wcr);


	/* Set name. */
	if(name != NULL)
	    prompt->name = strdup(name);
	else
	    prompt->name = NULL;


	/* Main text buffer. */
	prompt->buf_len = buf_len;
	if(prompt->buf_len > 0)
	{
	    prompt->buf = (char *)calloc(
		prompt->buf_len + 1,
		sizeof(char)
	    );
	    if(prompt->buf == NULL)
		prompt->buf_len = 0;
	}
	else
	{
	    prompt->buf = NULL;
	}

	prompt->buf_pos = 0;
        prompt->buf_vis_pos = 0;
        prompt->buf_sel_start = -1;
        prompt->buf_sel_end = -1;


	/* History buffers. */
	prompt->total_hist_bufs = (int)hist_bufs;
	if(((int)prompt->total_hist_bufs > 0) &&
	   ((int)prompt->buf_len > 0)
	)
	{
	    prompt->hist_buf = (char **)calloc(
		prompt->total_hist_bufs,
		sizeof(char *)
	    );
	    if(prompt->hist_buf == NULL)
	    {
	        prompt->total_hist_bufs = 0;
		return(-1);
	    }
	    for(i = 0; i < (int)prompt->total_hist_bufs; i++)
	    {
		prompt->hist_buf[i] = (char *)calloc(
		    prompt->buf_len + 1,
		    sizeof(char)
		);
		if(prompt->hist_buf[i] == NULL)
		{
		    return(-1);
		}
	    }
	}
	else
	{
	    prompt->hist_buf = NULL;
	}

	prompt->hist_buf_pos = 0;


	/* Set values. */
	prompt->map_state = 0;
	prompt->visibility_state = VisibilityFullyObscured;

	prompt->func_cb = func_cb;	/* Can be NULL. */


	/* Add prompt to wdiget regeristry. */
	WidgetRegAdd(prompt, WTYPE_CODE_PROMPT);


	return(0);
}


/*
 *	Procedure to unmark and redraw all prompts except
 *	except_prompt.
 */
void PROMPT_UNMARK_REDRAW_ALL(prompt_window_struct *except_prompt)
{
	int i, m;
	prompt_window_struct *prompt;
	widget_reg_entry_struct **wentry;


	wentry = widget_reg.entry;
	m = widget_reg.total_entries;

	for(i = 0; i < m; i++, wentry++)
	{
            if(*wentry == NULL)
                continue;

	    if((*wentry)->type != WTYPE_CODE_PROMPT)
		continue;

	    if((*wentry)->ptr == NULL)
		continue;
	    if((*wentry)->ptr == (void *)except_prompt)
		continue;


	    prompt = (prompt_window_struct *)(*wentry)->ptr;

            /* Unmark. */
            PromptUnmarkBuffer(prompt, PROMPT_POS_CUR);

            /* Redraw if already mapped. */
            if(prompt->map_state)
                PromptDraw(prompt, PROMPT_DRAW_AMOUNT_TEXTONLY);
	}


	return;
}


/*
 *	Procedure to unfocus and redraw all prompts except
 *	except_prompt.
 */
void PROMPT_UNFOCUS_REDRAW_ALL(prompt_window_struct *except_prompt)
{
        int i, m;
        prompt_window_struct *prompt;
        widget_reg_entry_struct **wentry;


        wentry = widget_reg.entry;
        m = widget_reg.total_entries;

        for(i = 0; i < m; i++, wentry++)
        {
            if(*wentry == NULL)
                continue;
   
            if((*wentry)->type != WTYPE_CODE_PROMPT)
                continue;

            if((*wentry)->ptr == NULL)
                continue;
            if((*wentry)->ptr == (void *)except_prompt)
                continue;
            prompt = (prompt_window_struct *)(*wentry)->ptr;

	    /* Unfocus. */
	    prompt->is_in_focus = 0;

	    /* Redraw if already mapped. */
	    if(prompt->map_state)
		PromptDraw(prompt, PROMPT_DRAW_AMOUNT_COMPLETE);
        }


	return;
}


/*
 *	Draws prompt.
 */
int PromptDraw(prompt_window_struct *prompt, int amount)
{
	int x = 0, y = 0, z;
	unsigned int width = 1, height = 1;
	unsigned int len;	/* strlen of prompt->buf. */

	int char_draw_count;
	unsigned int total_chars_visible;
        win_attr_t wattr;
        font_t *prev_font;


        if(prompt == NULL)
            return(-1);


        /* Record previous font. */
        prev_font = OSWQueryCurrentFont();


	/* Map as needed. */
	if(!prompt->map_state)
	{
	    len = (prompt->name == NULL) ? 0 : strlen(prompt->name);
	    OSWGetWindowAttributes(prompt->toplevel, &wattr);

	    /* Resize pixmap. */
	    OSWDestroyPixmap(&prompt->toplevel_buf);
	    if(
	        OSWCreatePixmap(
		    &prompt->toplevel_buf,
		    wattr.width,
		    wattr.height
		)
	    )
	        return(-1);

	    /* Get values for resizing text area. */
	    if((prompt->style == PROMPT_STYLE_RAISED) ||
               (prompt->style == PROMPT_STYLE_FLUSHED)
	    )
	    {
	        x = ((int)len * PROMPT_CHAR_WIDTH) + (2 * PROMPT_MARGIN);
	        y = PROMPT_MARGIN;
	        width = MAX( (int)wattr.width - (PROMPT_MARGIN * 4)
                    - ((int)len * PROMPT_CHAR_WIDTH), 1);
	        height = MAX( (int)wattr.height - PROMPT_MARGIN - PROMPT_MARGIN, 1);
	    }
	    else if(prompt->style == PROMPT_STYLE_NOBORDER)
	    {
		x = 0; y = 0;
		width = wattr.width;
		height = wattr.height;
	    }

	    /* Move and resize text area. */
	    OSWMoveResizeWindow(prompt->text_area, x, y, width, height);


	    /* Recreate text area pixmap_t buffer. */
	    OSWDestroyPixmap(&prompt->text_area_buf);
            if(
		OSWCreatePixmap(
                    &prompt->text_area_buf,
                    width, height
                )
            )
                return(-1);


	    /* Scroll to end. */
	    len = (prompt->buf == NULL) ? 0 : strlen(prompt->buf);
	    OSWGetWindowAttributes(prompt->text_area, &wattr);
	    total_chars_visible = (unsigned int)MAX(
                ((int)wattr.width - (PROMPT_MARGIN * 2)) / PROMPT_CHAR_WIDTH,
                0
            );
            prompt->buf_vis_pos = MAX(
		(int)len - (int)total_chars_visible,
		(int)0
	    );
	    if(prompt->buf_pos < prompt->buf_vis_pos)
		prompt->buf_pos = prompt->buf_vis_pos;

	    /* Map and raise. */
	    OSWMapRaised(prompt->toplevel);
	    OSWMapWindow(prompt->text_area);

	    /* Mark as mapped. */
	    prompt->map_state = 1;
	    prompt->visibility_state = VisibilityUnobscured;

	    amount = PROMPT_DRAW_AMOUNT_COMPLETE;
	}


        /* Recreate buffers. */
        if(prompt->toplevel_buf == 0)
        {
            OSWGetWindowAttributes(prompt->toplevel, &wattr);
            if(OSWCreatePixmap(&prompt->toplevel_buf,
                wattr.width, wattr.height)
            )
                return(-1);
        }
        if(prompt->text_area_buf == 0)
        {
            OSWGetWindowAttributes(prompt->text_area, &wattr);
            if(OSWCreatePixmap(&prompt->text_area_buf,
                wattr.width, wattr.height)
            )
                return(-1);
        }


	/* Draw background and name label. */
	if((amount == PROMPT_DRAW_AMOUNT_COMPLETE) &&
           (prompt->style != PROMPT_STYLE_NOBORDER)
	)
	{
	    OSWGetWindowAttributes(prompt->toplevel, &wattr);

	    if(widget_global.force_mono)
		OSWClearPixmap(
		    prompt->toplevel_buf,
                    wattr.width, wattr.height,
                    osw_gui[0].black_pix
		);
            else
	        WidgetPutImageTile(
		    prompt->toplevel_buf,
		    widget_global.std_bkg_img,
		    wattr.width, wattr.height
	        );

	    /* Draw label. */
	    if(prompt->name != NULL)
	    {
	        if(widget_global.force_mono)
	        {
	            OSWSetFgPix(osw_gui[0].white_pix);
	        }
	        else
	        {
	            if(prompt->disabled)
		        OSWSetFgPix(widget_global.disabled_text_pix);
	            else
		        OSWSetFgPix(widget_global.normal_text_pix);
	        }
		OSWSetFont(widget_global.prompt_label_font);
	        OSWDrawString(
		    prompt->toplevel_buf,
		    PROMPT_MARGIN,
		    ((int)wattr.height / 2) + 5,
		    prompt->name
	        );
	    }

            /* Draw frame on toplevel. */
	    if(prompt->style == PROMPT_STYLE_RAISED)
            {
                WidgetFrameButtonPixmap(
		    prompt->toplevel_buf, False,
		    wattr.width, wattr.height,
                    (widget_global.force_mono) ? osw_gui[0].white_pix :
                        widget_global.surface_highlight_pix,
                    (widget_global.force_mono) ? osw_gui[0].white_pix :
                        widget_global.surface_shadow_pix
                );
            }


	    OSWPutBufferToWindow(prompt->toplevel, prompt->toplevel_buf);
	}


	/* ********************************************************* */

	/* Get size of text_area. */
	OSWGetWindowAttributes(prompt->text_area, &wattr);

	/* Clear text area pixmap buffer. */
        OSWClearPixmap(
            prompt->text_area_buf,
            wattr.width, wattr.height,
            (widget_global.force_mono) ?
		osw_gui[0].black_pix : widget_global.surface_editable_pix
        );

        /* Get maximum possible characters visible on text_area. */
	total_chars_visible = (unsigned int)MAX(
	    ((int)wattr.width - (PROMPT_MARGIN * 2)) / PROMPT_CHAR_WIDTH,
	    0
	);

	/* Get length of buffer or 0 if no buffer. */
	len = (prompt->buf == NULL) ? 0 : strlen(prompt->buf);


	/* Sanitize selection range. */
        if(prompt->buf_sel_start > prompt->buf_sel_end)
	{
	    prompt->buf_sel_start = -1;
	    prompt->buf_sel_end = -1;
	}
	if(prompt->buf_sel_start >= (int)len)
	{
            prompt->buf_sel_start = -1;
            prompt->buf_sel_end = -1;
	}
        if(prompt->buf_sel_end >= (int)len)
        {
            prompt->buf_sel_end = MAX((int)len - 1, 0);
        }

	/* Sanitize visible position. */
	if(((int)prompt->buf_len - (int)total_chars_visible) >= 0)
	{
	    prompt->buf_vis_pos = MIN(
	        prompt->buf_vis_pos,
	        (int)((int)prompt->buf_len - (int)total_chars_visible)
	    );
	}
	else
	{
	    prompt->buf_vis_pos = 0;
	}

	/* Draw text. */
	x = 0;
	y = 0;
	z = 0;
	char_draw_count = 0;
	if((prompt->buf_sel_start >= 0) &&
	   (prompt->buf_sel_end >= 0) &&
	   (prompt->buf_sel_start <= prompt->buf_sel_end)
	)
	{
	    /* Draw with selection highlight. */

	    /* Draw first unhighlighted section first. */
	    x = prompt->buf_vis_pos;
	    y = MAX(
		prompt->buf_sel_start - prompt->buf_vis_pos,
		0
	    );
	    if(y >= ((int)total_chars_visible - (int)char_draw_count))
		y = (int)total_chars_visible - (int)char_draw_count - 1;
	    if((x + y) >= (int)prompt->buf_len)
		y = ((int)prompt->buf_len - 1) - x;

	    if(INRANGEINC(x, 0, (int)prompt->buf_len - 1) &&
               INRANGEINC(x + y, 0, (int)prompt->buf_len - 1) &&
	       (y > 0) &&
               (prompt->buf != NULL) &&
               (char_draw_count <= (int)total_chars_visible)
	    )
	    {
		if(widget_global.force_mono)
                    OSWSetFgPix(osw_gui[0].white_pix);
		else
                    OSWSetFgPix(widget_global.editable_text_pix);

                OSWSetFont(widget_global.prompt_text_font);
		OSWDrawStringLimited(prompt->text_area_buf,
                    (char_draw_count * PROMPT_CHAR_WIDTH) + PROMPT_MARGIN,
                    ((int)wattr.height / 2) + 5,
                    &(prompt->buf[x]), y
                );
	    }
	    char_draw_count += y;

	    /* Draw selected text. */
            x = MAX(prompt->buf_sel_start, prompt->buf_vis_pos);
            y = MAX(
                (prompt->buf_sel_end - x) + 1,
                0
            );
            if(y > ((int)total_chars_visible - (int)char_draw_count))
                y = (int)total_chars_visible - (int)char_draw_count;
            if( (x + y) >= (int)prompt->buf_len )
                y = ((int)prompt->buf_len - 1) - x;

            if( INRANGEINC(x, 0, (int)prompt->buf_len - 1) &&
                INRANGEINC(x + y, 0, (int)prompt->buf_len - 1) &&
                (y > 0) &&
                (prompt->buf != NULL) &&
                (char_draw_count <= (int)total_chars_visible)
            )
            {
                if(widget_global.force_mono)
                    OSWSetFgPix(osw_gui[0].white_pix);
		else
                    OSWSetFgPix(widget_global.surface_selected_pix);

		OSWDrawSolidRectangle((drawable_t)prompt->text_area_buf,
		    (char_draw_count * PROMPT_CHAR_WIDTH) + PROMPT_MARGIN,
		    PROMPT_MARGIN,
		    (y * PROMPT_CHAR_WIDTH),
		    MAX( (int)wattr.height - PROMPT_MARGIN - PROMPT_MARGIN,
			1)
		);

                OSWSetFont(widget_global.prompt_text_font);
                if(widget_global.force_mono)
                    OSWSetFgPix(osw_gui[0].black_pix);
		else
                    OSWSetFgPix(widget_global.selected_text_pix);
                OSWDrawStringLimited(prompt->text_area_buf,
                    (char_draw_count * PROMPT_CHAR_WIDTH) + PROMPT_MARGIN,
                    ((int)wattr.height / 2) + 5,
                    &(prompt->buf[x]), y
                );
            }   
            char_draw_count += y;

            /* Draw last unhighlighted section. */
            x = MAX( (int)prompt->buf_sel_end + 1, 0);
            y = MAX( (int)total_chars_visible - (int)char_draw_count, 0);
            if( (x + y) > (int)len )
                y = ((int)len) - x;

            if( INRANGEINC(x, 0, (int)prompt->buf_len - 1) &&
                INRANGEINC(x + y, 0, (int)prompt->buf_len - 1) &&
                (y > 0) &&
                (prompt->buf != NULL) &&
                (char_draw_count <= (int)total_chars_visible)
            )
            {
                OSWSetFont(widget_global.prompt_text_font);
                if(widget_global.force_mono)
                    OSWSetFgPix(osw_gui[0].white_pix);
		else
                    OSWSetFgPix(widget_global.editable_text_pix);
                OSWDrawStringLimited(prompt->text_area_buf,
                    (char_draw_count * PROMPT_CHAR_WIDTH) + PROMPT_MARGIN,
                    ((int)wattr.height / 2) + 5,
                    &(prompt->buf[x]), y
                );
            }
	}
	else
	{
	    x = prompt->buf_vis_pos;
	    y = MIN((int)total_chars_visible,
		    (int)strlen(&(prompt->buf[x]))
	    );
	    if((x + y) >= (int)prompt->buf_len )
		y = ((int)prompt->buf_len - 1) - x;

            if(INRANGEINC(x, 0, (int)prompt->buf_len - 1) &&
               INRANGEINC(x + y, 0, (int)prompt->buf_len - 1) &&
               (y > 0) &&
               (prompt->buf != NULL) &&
               (char_draw_count <= (int)total_chars_visible)
            )
            {
                OSWSetFont(widget_global.prompt_text_font);
                if(widget_global.force_mono)
                    OSWSetFgPix(osw_gui[0].white_pix);
		else
                    OSWSetFgPix(widget_global.editable_text_pix);
                OSWDrawStringLimited(prompt->text_area_buf,
                    (char_draw_count * PROMPT_CHAR_WIDTH) + PROMPT_MARGIN,
                    ((int)wattr.height / 2) + 5,
                    &(prompt->buf[x]), y
                );
            }
            char_draw_count += y;
	}

	/* Draw text (not pointer) cursor. */
	if(INRANGEINC((int)prompt->buf_pos, 0, (int)prompt->buf_len - 1) &&
	   (prompt->is_in_focus == 1) &&
           ((prompt->buf_sel_start < 0) ||
            (prompt->buf_sel_end < 0) ||
            (prompt->buf_sel_start == prompt->buf_sel_end)
	   )
	)
	{
	    x = ((prompt->buf_pos - prompt->buf_vis_pos) *
		PROMPT_CHAR_WIDTH) + PROMPT_MARGIN;

            if(widget_global.force_mono)
                OSWSetFgPix(osw_gui[0].white_pix);
	    else
	        OSWSetFgPix(widget_global.editable_text_pix);
	    OSWDrawLine(
		prompt->text_area_buf,
		x, PROMPT_MARGIN + 1,
		x, (int)wattr.height - PROMPT_MARGIN - 1
	    );
            OSWDrawLine(
                prompt->text_area_buf,
                x - 2, PROMPT_MARGIN + 1,
                x + 2, PROMPT_MARGIN + 1
            );
            OSWDrawLine(
                prompt->text_area_buf,
                x - 2, (int)wattr.height - PROMPT_MARGIN - 1,
                x + 2, (int)wattr.height - PROMPT_MARGIN - 1
            );
	}


	/* Draw depressed frame around text area. */
	if(prompt->style != PROMPT_STYLE_NOBORDER)
	{
/*
	    OSWGetWindowAttributes(prompt->text_area, &wattr);
*/

	    WidgetFrameButtonPixmap(
	        prompt->text_area_buf,
	        True,
	        wattr.width, wattr.height,
                (widget_global.force_mono) ? osw_gui[0].white_pix :
                    widget_global.surface_highlight_pix,
                (widget_global.force_mono) ? osw_gui[0].white_pix :
                    widget_global.surface_shadow_pix
	    );
	}

        OSWPutBufferToWindow(prompt->text_area, prompt->text_area_buf);

        OSWSetFont(prev_font);


	return(0);
}


/*
 *	Manage prompt.
 */
int PromptManage(prompt_window_struct *prompt, event_t *event)
{
	int x, y, z;
	int events_handled = 0;
	int len;

	int total_chars_visible;
	char c;

	win_attr_t wattr;

	static bool_t text_area_button_state;
	static long last_button_press;

	char *tmp_buf;
	int tmp_buf_len;

	keycode_t keycode;
	prompt_window_struct *prompt2;


	if((prompt == NULL) ||
           (event == NULL)
	)
	    return(events_handled);

	if(!prompt->map_state &&
	   (event->type != MapNotify)
	)
	    return(events_handled);


	switch(event->type)
	{
	  /* ********************************************************** */
	  case KeyPress:
	    /* Do not handle KeyPress if not in focus. */
	    if(!prompt->is_in_focus)
		break;

	    keycode = event->xkey.keycode;

	    /* Skip modifier keys. */
	    if(OSWIsModifierKey(keycode))
		break;

	    /* Skip if no text area buffer. */
	    if((prompt->buf == NULL) || ((int)prompt->buf_len <= 0))
		break;


	    /* Calculate total characters visable on window. */
	    OSWGetWindowAttributes(prompt->text_area, &wattr);
            total_chars_visible = PROMPT_CHARS_VIS(prompt);

	    /* Sanitize prompt. */
	    if(prompt->buf_pos < 0)
		prompt->buf_pos = 0;
	    else if(prompt->buf_pos >= (int)prompt->buf_len)
		prompt->buf_pos = (int)prompt->buf_len - 1;


	    /* Tab. */
	    if(keycode == osw_keycode.tab)
	    {
		if(osw_gui[0].shift_key_state)
		    prompt2 = (prompt_window_struct *)prompt->prev;
		else
		    prompt2 = (prompt_window_struct *)prompt->next;

		if(prompt2 != NULL)
		{
		    if(WidgetRegIsRegistered(prompt2, WTYPE_CODE_PROMPT))
		    {
			prompt->is_in_focus = 0;
			prompt2->is_in_focus = 1;
			PromptMarkBuffer(
			    prompt2,
			    PROMPT_POS_END
			);
			if(prompt2->map_state)
			    PromptDraw(prompt2, PROMPT_DRAW_AMOUNT_TEXTONLY);
		    }
		}
		/* This prompt now may be out of focus. */
		events_handled++;
	    }
	    /* Backspace. */
	    else if((keycode == osw_keycode.backspace) &&
                    !prompt->disabled
	    )
	    {
		len = MIN((int)strlen(prompt->buf), (int)prompt->buf_len);

                if((prompt->buf_sel_start >= 0) &&
                   (prompt->buf_sel_end >= 0)
                )
                {
                    /* Delete selected text. */
                    
                    /* Get values. */
                    x = MIN((int)prompt->buf_sel_start, (int)len);
                    y = MIN((int)prompt->buf_sel_end + 1, (int)len);
                    while(y < len)
                    {
                        if(prompt->buf[y] == '\0')
                            break;
                    
                        prompt->buf[x] = prompt->buf[y];
                 
                        x++; y++;
                    }
                    /* Null terminate. */
                    prompt->buf[x] = '\0';
                    
                    /* Get new length. */
                    len = MIN( strlen(prompt->buf), prompt->buf_len );
                    
                    /* Move cursor to position where selection started. */
                    prompt->buf_pos = prompt->buf_sel_start;
                    if(prompt->buf_pos > len)
                        prompt->buf_pos = len;
                    if(prompt->buf_pos < 0)
                        prompt->buf_pos = 0;
                           
                 
                    /* Unset marked selection. */
                    prompt->buf_sel_start = -1;
                    prompt->buf_sel_end = -1;
		}
		else if( (prompt->buf_pos > 0) &&
		         (prompt->buf_pos <= (int)len)
		)
		{
		    /* Erase previous character. */

		    /* Adjust position. */
		    prompt->buf_pos -= 1;

		    /* Scroll as needed. */
		    if(prompt->buf_pos < prompt->buf_vis_pos)
			prompt->buf_vis_pos = prompt->buf_pos;

		    /* Shift all characters. */
		    for(x = prompt->buf_pos; x < (int)prompt->buf_len; x++)
		    {
			if(prompt->buf[x] == '\0')
			    break;

			/*   It's safe to do x + 1 because allocated buffer
			 *   length is prompt->buf_len + 1.
			 */
			prompt->buf[x] = prompt->buf[x + 1];
		    }
		    /* Yes this byte is allocated. */
		    prompt->buf[prompt->buf_len] = '\0';
		}
                events_handled++;
	    }
	    /* Delete a character. */
            else if((keycode == osw_keycode.ddelete) &&
                    !prompt->disabled
	    )
            {
		len = MIN((int)strlen(prompt->buf), (int)prompt->buf_len);

		/* Delete selected text? */
		if((prompt->buf_sel_start >= 0) &&
                   (prompt->buf_sel_end >= 0)
		)
		{
		    /* Delete selected text. */

		    /* Get values. */
		    x = MIN((int)prompt->buf_sel_start, (int)len);
		    y = MIN((int)prompt->buf_sel_end + 1, (int)len);
		    while(y < len)
		    {
			if(prompt->buf[y] == '\0')
			    break;

			prompt->buf[x] = prompt->buf[y];

			x++; y++;
		    }
		    /* Null terminate. */
		    prompt->buf[x] = '\0';

		    /* Get new length. */
		    len = MIN(strlen(prompt->buf), prompt->buf_len);

		    /* Move cursor to position where selection started. */
		    prompt->buf_pos = prompt->buf_sel_start;
		    if(prompt->buf_pos > len)
			prompt->buf_pos = len;
		    if(prompt->buf_pos < 0)
			prompt->buf_pos = 0;

		    /* Scroll as needed. */
		    if(prompt->buf_vis_pos >= (int)len)
		    {
			prompt->buf_vis_pos = MAX(
			    (int)((int)len - (int)total_chars_visible),
			    (int)0
			);
		    }

		    /* Unset marked selection. */
		    prompt->buf_sel_start = -1;
		    prompt->buf_sel_end = -1;
		}
	        /* Delete character. */
                else if((prompt->buf_pos >= 0) &&
		        (prompt->buf_pos < (int)len)
		)
                {
		    /* Delete a character at current cursor position. */

                    /* Shift all characters. */
                    for(x = prompt->buf_pos; x < (int)prompt->buf_len; x++)
                    {
                        if(prompt->buf[x] == '\0')
                            break;
            
                        /*   It's safe to do x + 1 because allocated buffer
                         *   length is prompt->buf_len + 1.
                         */
                        prompt->buf[x] = prompt->buf[x + 1];
                    }
		    /* Null terminate (Yes this byte is allocated). */
                    prompt->buf[prompt->buf_len] = '\0';
		}

                events_handled++;
	    }
	    /* Enter. */
            else if(((keycode == osw_keycode.enter) ||
                     (keycode == osw_keycode.np_enter)
                    ) &&
                    !prompt->disabled
	    )
            {
/* For dumping prompt buffer to STDOUT, commented out. */
/*
printf("\n|");
for(x = 0; x < prompt->buf_len; x++)
{
if(prompt->buf[x] == '\0')
    printf(" ");
else
    printf("%c", prompt->buf[x]);
}
printf("|\n");
*/

		/* Record buffer to history. */
		if((prompt->hist_buf != NULL) &&
                   ((int)prompt->total_hist_bufs > 0) &&
		   ((int)prompt->buf_len > 0)
		)
		{
		    /* Shift the history buffers. */
		    for(x = (int)prompt->total_hist_bufs - 1; x > 0; x--)
		    {
			if((prompt->hist_buf[x] == NULL) ||
			   (prompt->hist_buf[x - 1] == NULL)
			)
			    continue;

			memcpy(
			    prompt->hist_buf[x],
			    prompt->hist_buf[x - 1],
			    (int)prompt->buf_len + 1	/* Yes that much is allocated. */
			);
		    }

		    if(prompt->buf != NULL)
		    {
		        memcpy(prompt->hist_buf[0], prompt->buf,
			    (int)prompt->buf_len + 1);
		    }

		    /* Adjust history buffer position to -1. */
		    prompt->hist_buf_pos = -1;
		}

		/* Call the callback function. */
		if(prompt->func_cb != NULL)
		{
		    prompt->func_cb(prompt->buf);
		}

		/* Clear the buffer. */
		memset(
		    prompt->buf,
		    '\0',
		    prompt->buf_len + 1	/* This is allocated. */
		);
                prompt->buf_pos = 0;
		prompt->buf_vis_pos = 0;
		prompt->buf_sel_start = -1;
                prompt->buf_sel_end = -1;


                events_handled++;

		/* Return, do not continue. */
		return(events_handled);
	    }
	    /* Cursor up. */
	    else if((keycode == osw_keycode.cursor_up) &&
                    !prompt->disabled
	    )
	    {
		/* Recall older history buffer. */
		if((int)prompt->total_hist_bufs > 0)
		{
                    /* Change last recalled history buffer position. */
                    prompt->hist_buf_pos++;
		    if((int)prompt->hist_buf_pos < 0)
		        prompt->hist_buf_pos = 0;
		    if((int)prompt->hist_buf_pos >= (int)prompt->total_hist_bufs)
		        prompt->hist_buf_pos = (int)prompt->total_hist_bufs - 1;

                    /* Recall that history buffer. */
                    if((int)prompt->hist_buf_pos < (int)prompt->total_hist_bufs)
                    {
                        memcpy(
                            prompt->buf,
                            prompt->hist_buf[prompt->hist_buf_pos],
                            (int)prompt->buf_len
                        );
                        prompt->buf_pos = strlen(prompt->buf);
		    }

		    events_handled++;
                }

	    }
	    /* Cursor down. */
            else if((keycode == osw_keycode.cursor_down) &&
                    !prompt->disabled
	    )
            {
		/* Recall newer history buffer. */
		if((int)prompt->total_hist_bufs > 0)
		{
		    /* Change last recalled history buffer position. */
                    prompt->hist_buf_pos--;
                    if((int)prompt->hist_buf_pos < 0)
                        prompt->hist_buf_pos = 0;
                    if((int)prompt->hist_buf_pos >= (int)prompt->total_hist_bufs)
                        prompt->hist_buf_pos = (int)prompt->total_hist_bufs - 1;

		    /* Recall that history buffer. */
		    if((int)prompt->hist_buf_pos < (int)prompt->total_hist_bufs)
		    {
		        memcpy(
			    prompt->buf,
			    prompt->hist_buf[prompt->hist_buf_pos],
			    (int)prompt->buf_len
		        );
		        prompt->buf_pos = strlen(prompt->buf);
		    }

                    events_handled++;
		}

            }
	    /* Cursor left. */
	    else if(keycode == osw_keycode.cursor_left)
	    {
                /* Get length of text value in buffer. */
                len = strlen(prompt->buf);


                /* Check if shift key is pressed. */
                if((prompt->buf_pos > 0) && osw_gui[0].shift_key_state)
		{
		    /* Set select marks. */

                    /* Record origin if previously not marked. */
                    if((prompt->buf_sel_start < 0) &&
                       (prompt->buf_sel_end < 0)
                    )
                    {
                        /* Unmark and redraw other prompts. */
                        PROMPT_UNMARK_REDRAW_ALL(prompt);

                        /*  Previously not marked, make it so only
                         *  current character gets marked.
                         */
			prompt->buf_pos -= 1;

                        prompt->buf_sel_start = prompt->buf_pos;
                        prompt->buf_sel_end = prompt->buf_pos;
                        static_wprompt::mark_origin_pos = prompt->buf_pos;

                        prompt->buf_pos += 1;	/* Need to add 1. */
                    }

		    /* Move the cursor to the left. */
		    prompt->buf_pos -= 1;

                    /* Move selection mark position. */
                    if(prompt->buf_pos < static_wprompt::mark_origin_pos)
                        prompt->buf_sel_start = prompt->buf_pos;
                    else
                        prompt->buf_sel_end = prompt->buf_pos;


		    /* Store the bytes of the selected buffer range. */
		    PromptDDEStoreASCIISeg(
			&(prompt->buf[prompt->buf_sel_start]),
			prompt->buf_sel_end - prompt->buf_sel_start + 1
		    );
                }
                else if(prompt->buf_pos > 0)
                {
                    /* Unset selection mark. */
                    prompt->buf_sel_start = -1;
                    prompt->buf_sel_end = -1;
                
                    /* Move cursor to the right. */   
                    prompt->buf_pos -= 1;
		}

                /* Scroll as needed. */
                if(prompt->buf_pos < prompt->buf_vis_pos)
                    prompt->buf_vis_pos = prompt->buf_pos;

                events_handled++;
	    }
            /* Cursor right. */
            else if(keycode == osw_keycode.cursor_right)
            {
		/* Get length of text value in buffer. */
		len = MIN((int)strlen(prompt->buf), (int)prompt->buf_len);

		/* Mark selection right. */
		if((prompt->buf_pos < len) && osw_gui[0].shift_key_state)
		{
                    /* Record origin if previously not marked. */
                    if((prompt->buf_sel_start < 0) &&
                       (prompt->buf_sel_end < 0)
                    )
                    {
                        /* Unmark and redraw other prompts. */
                        PROMPT_UNMARK_REDRAW_ALL(prompt);

                        prompt->buf_sel_start = prompt->buf_pos;
                        prompt->buf_sel_end = prompt->buf_pos;
                        static_wprompt::mark_origin_pos = prompt->buf_pos;

			/*  Previously not marked, make it so only
			 *  current character gets marked.
			 */
			prompt->buf_pos -= 1;
                    }

		    /* Move cursor to the right. */
		    prompt->buf_pos += 1;
		    if(prompt->buf_pos >= len)
			prompt->buf_pos = (int)len - 1;

		    /* Move selection mark position. */
		    if(prompt->buf_pos < static_wprompt::mark_origin_pos)
			prompt->buf_sel_start = prompt->buf_pos;
		    else
			prompt->buf_sel_end = prompt->buf_pos;


                    /* Store the bytes of the selected buffer range. */
                    PromptDDEStoreASCIISeg(
                        &(prompt->buf[prompt->buf_sel_start]),
                        prompt->buf_sel_end - prompt->buf_sel_start + 1
                    );
		}
		else if(prompt->buf_pos < len)
		{
		    /* Unset selection mark. */
		    prompt->buf_sel_start = -1;
		    prompt->buf_sel_end = -1;

                    /* Move cursor to the right. */
                    prompt->buf_pos += 1;
		}

                /* Scroll as needed. */
                if(prompt->buf_pos >=
                   (prompt->buf_vis_pos + total_chars_visible)
                )
                {
                    prompt->buf_vis_pos = MAX(
                        prompt->buf_pos - (int)total_chars_visible + 1,
                        0
                    );
                }

                events_handled++;
            }
	    /* Home. */
	    else if(keycode == osw_keycode.home)
	    {
		if(osw_gui[0].shift_key_state)
		{
                    /* Record origin if previously not marked. */
                    if((prompt->buf_sel_start < 0) &&
                       (prompt->buf_sel_end < 0)
                    )
                    {
			prompt->buf_pos -= 1;

                        prompt->buf_sel_start = prompt->buf_pos;
                        prompt->buf_sel_end = prompt->buf_pos;
                        static_wprompt::mark_origin_pos = prompt->buf_pos;
                    }

		    prompt->buf_sel_end = static_wprompt::mark_origin_pos;
		    prompt->buf_sel_start = 0;

                    /* Unmark and redraw other prompts. */
                    PROMPT_UNMARK_REDRAW_ALL(prompt);

                    /* Store the bytes of the selected buffer range. */
                    PromptDDEStoreASCIISeg(   
                        &(prompt->buf[prompt->buf_sel_start]),
                        prompt->buf_sel_end - prompt->buf_sel_start + 1
                    );
		}
		else
		{
                    /* Remove selection. */
                    prompt->buf_sel_start = -1;
                    prompt->buf_sel_end = -1;
		}

		/* Set cursor position to 0. */
		prompt->buf_pos = 0;

		/* Scroll as needed. */
                if(prompt->buf_pos < prompt->buf_vis_pos)
                    prompt->buf_vis_pos = prompt->buf_pos;

		events_handled++;
	    }
	    /* End. */
	    else if(keycode == osw_keycode.end)
	    {
                len = MIN((int)strlen(prompt->buf), (int)prompt->buf_len);

                if(osw_gui[0].shift_key_state)
                {
                    /* Record origin if previously not marked. */
                    if((prompt->buf_sel_start < 0) &&
                       (prompt->buf_sel_end < 0)
                    )
                    {
                        prompt->buf_sel_start = prompt->buf_pos;
                        prompt->buf_sel_end = prompt->buf_pos;
                        static_wprompt::mark_origin_pos = prompt->buf_pos;
                    }

                    prompt->buf_sel_start = static_wprompt::mark_origin_pos;
                    prompt->buf_sel_end = (int)len - 1;

                    /* Unmark and redraw other prompts. */
                    PROMPT_UNMARK_REDRAW_ALL(prompt);

                    /* Store the bytes of the selected buffer range. */
                    PromptDDEStoreASCIISeg(   
                        &(prompt->buf[prompt->buf_sel_start]),
                        (int)prompt->buf_sel_end - (int)prompt->buf_sel_start
                        + 1
                    );
                }
		else
		{
                    /* Remove selection. */
                    prompt->buf_sel_start = -1;
                    prompt->buf_sel_end = -1;
		}

		/* Set cursor position to end. */
		prompt->buf_pos = len;

                /* Scroll as needed. */
                if(prompt->buf_pos >
                   (prompt->buf_vis_pos + total_chars_visible)
                ) 
                {
                    prompt->buf_vis_pos = MAX(
                        (int)prompt->buf_pos - (int)total_chars_visible,
                        0
                    );
                }
		events_handled++;
	    }
            /* Shift+insert: Paste DDE buffer. */
            else if((keycode == osw_keycode.insert) &&
                    osw_gui[0].shift_key_state &&
                    !prompt->disabled
            )
            {
                len = MIN((int)strlen(prompt->buf), (int)prompt->buf_len);
                tmp_buf = (char *)OSWFetchDDE(&tmp_buf_len);

                /* Unset selection marks. */
                prompt->buf_sel_start = -1;
                prompt->buf_sel_end = -1; 

                /* Sanitize position. (Remember we have one extra byte.) */
                if(prompt->buf_pos > (int)len)
                    prompt->buf_pos = (int)len;
                if(prompt->buf_pos < 0)
                    prompt->buf_pos = 0;

                /* Check if there is text selection marked. */
                if((tmp_buf != NULL) &&
                   (tmp_buf_len > 0)
                )
                {
                    /* Shift tailing segment of prompt buffer. */
                    x = len;
                    y = len + tmp_buf_len;
                    if(y > (int)prompt->buf_len)
                    {
                        z = y - (int)prompt->buf_len;
                        y = MAX(y - z, 0);
                        x = MAX(x - z, 0);
                    }

                    while(x >= prompt->buf_pos)
                    {
                        prompt->buf[y] = prompt->buf[x];
                        x--; y--;
                    }

                    /* Paste buffer in. */
                    y = MAX(prompt->buf_pos, 0);
                    for(x = 0; x < tmp_buf_len; x++, y++)
                    {
                        if(y >= ((int)prompt->buf_len - 1))
                        {
                            prompt->buf[y] = '\0';
                            break;
                        }

                        prompt->buf[y] = tmp_buf[x];
                    }
                    /* Null terminate. */
                    prompt->buf[prompt->buf_len] = '\0';

                    /* Free ddn buffer. */
                    OSWGUIFree((void **)&tmp_buf);

                    /* Must move cursor ahead. */
                    prompt->buf_pos = MIN(
                        prompt->buf_pos + tmp_buf_len,
                        (int)prompt->buf_len - 1
                    );

                    /* Scroll as needed. */
                    OSWGetWindowAttributes(prompt->text_area, &wattr);
                    total_chars_visible = (unsigned int)MAX(
                        ((int)wattr.width - (PROMPT_MARGIN * 2)) /
                        PROMPT_CHAR_WIDTH,
                        0
                    );
                    if(prompt->buf_pos >
                       (prompt->buf_vis_pos + total_chars_visible)
                    )
                    {
                        prompt->buf_vis_pos = MAX(
                            (int)prompt->buf_pos - (int)total_chars_visible,
                            0
                        );
                    }
                }
                events_handled++;
	    }

	    /* Add a character. */
	    else if(!prompt->disabled)
	    {
		len = MIN((int)strlen(prompt->buf), (int)prompt->buf_len);
		/* Check if there are any selected text. */
                if((prompt->buf_sel_start >= 0) &&
                   (prompt->buf_sel_end >= 0)
                )
                {
                    /* Delete selected text. */

                    /* Get values. */
                    x = MIN((int)prompt->buf_sel_start, (int)len);
                    y = MIN((int)prompt->buf_sel_end + 1, (int)len);
                    while(y < len)
                    {      
                        if(prompt->buf[y] == '\0')
                            break;
                    
                        prompt->buf[x] = prompt->buf[y];
                 
                        x++; y++;
                    }
                    /* Null terminate. */
                    prompt->buf[x] = '\0';
                    
                    /* Get new length. */
                    len = MIN(strlen(prompt->buf), prompt->buf_len);

                    /* Move cursor to position where selection started. */
                    prompt->buf_pos = prompt->buf_sel_start;
                    if(prompt->buf_pos > len)
                        prompt->buf_pos = len;
                    if(prompt->buf_pos < 0)
                        prompt->buf_pos = 0;

                    /* Unset marked selection. */
                    prompt->buf_sel_start = -1;
                    prompt->buf_sel_end = -1;
                }   

		/* Add character. */
		if((prompt->buf_pos >= 0) &&
                   (prompt->buf_pos < ((int)prompt->buf_len - 1))
		)
		{
		    /* Shift all the characters from buf_pos. */
		    for(x = prompt->buf_len; x > prompt->buf_pos; x--)
		    {
			prompt->buf[x] = prompt->buf[x - 1];
		    }
		    prompt->buf[prompt->buf_len] = '\0';

                    /* Add a character. */
                    c = OSWGetASCIIFromKeyCode(
			&event->xkey,
			osw_gui[0].shift_key_state,
			osw_gui[0].alt_key_state,
			osw_gui[0].ctrl_key_state
		    );
		    if(c != '\0')
		    {
		        prompt->buf[prompt->buf_pos] = c;

                        x = prompt->buf_len;
                        prompt->buf[x] = '\0';
                            /* Yes this byte is allocated. */

		        /* Move cursor ahead one character. */
		        prompt->buf_pos += 1;

                        /* Scroll as needed. */
                        if(prompt->buf_pos >
                           (prompt->buf_vis_pos + total_chars_visible)
                        )
                            prompt->buf_vis_pos = MAX(
                                prompt->buf_pos - (int)total_chars_visible,
                                0
                            );
			if(prompt->buf_pos < prompt->buf_vis_pos)
			    prompt->buf_vis_pos = prompt->buf_pos;
                    }
		}
		events_handled++;
	    }

	    /* Draw text area only as needed. */
	    if(events_handled > 0)
	    {
                PromptDraw(
		    prompt,
                    PROMPT_DRAW_AMOUNT_TEXTONLY
                );
                return(events_handled);
	    }

	    break;

          /* ************************************************************* */
	  case KeyRelease:
            /* Do not handle KeyRelease if not in focus. */
            if(!prompt->is_in_focus)
                break;


	    /* Prompt is in focus and KeyReleases should be considered handled. */
	    events_handled++;
	    return(events_handled);

            /* Draw text area only as needed. */
/*
            if(events_handled > 0)
            {
                PromptDraw((prompt_window_struct *)prompt,
                    PROMPT_DRAW_AMOUNT_TEXTONLY
                );
                return(events_handled);
            }
*/

            break;

	  /* ************************************************************* */
	  case ButtonPress:
	    /* Check if ButtonPress makes this prompt in focus. */
	    if((event->xany.window == prompt->toplevel) ||
               (event->xany.window == prompt->text_area)
	    )
	    {
		PROMPT_UNFOCUS_REDRAW_ALL(prompt);
	        prompt->is_in_focus = 1;
	    }
	    else
	    {
		prompt->is_in_focus = 0;
	    }


            /* Skip if no text area buffer. */
            if((prompt->buf == NULL) || ((int)prompt->buf_len <= 0))
                break;

	    /* Button1: Place cursor and mark button as pressed. */
	    if((event->xany.window == prompt->text_area) &&
               (event->xbutton.button == Button1)
	    )
	    {
		/* Set text area button state to True. */
                text_area_button_state = True;

		/* Calculate and set new position of cursor. */
		prompt->buf_pos = (((int)event->xbutton.x - PROMPT_MARGIN)
		    / PROMPT_CHAR_WIDTH) + prompt->buf_vis_pos;

		len = strlen(prompt->buf);
		/* Sanitize position. (Remember we have one extra byte.) */
		if(prompt->buf_pos > (int)len)
		    prompt->buf_pos = (int)len;
		if(prompt->buf_pos < 0)
		    prompt->buf_pos = 0;

		/* Reset selected text markers. */
		prompt->buf_sel_start = -1;
		prompt->buf_sel_end = -1;

		/* Record mark origin position. */
		static_wprompt::mark_origin_pos = MIN(
                    (int)prompt->buf_pos,
		    (int)len - 1
		);


		/* Check double click, marks a `word' segment. */
		if((last_button_press + widget_global.double_click_int)
		    >= MilliTime()
		)
		{
		    /* Seek START of marked segment. */
		    for(x = prompt->buf_pos; x > 0; x--)
		    {
			/* Check if char is not a letter, number, or '_'. */
                        if(!((prompt->buf[x] >= 0x30) && (prompt->buf[x] <= 0x39)) &&
                           !((prompt->buf[x] >= 0x41) && (prompt->buf[x] <= 0x5a)) &&
                           !((prompt->buf[x] >= 0x61) && (prompt->buf[x] <= 0x7a)) &&
                           (prompt->buf[x] != '_')
                        )
			{
			    x++; break;
			}
		    }
		    prompt->buf_sel_start = MAX(x, 0);

                    /* Seek END of marked segment. */
                    for(x = prompt->buf_pos; x < (int)prompt->buf_len; x++)
                    {
			/* Check if char is not a letter, number, or '_'. */
			if(!((prompt->buf[x] >= 0x30) && (prompt->buf[x] <= 0x39)) &&
                           !((prompt->buf[x] >= 0x41) && (prompt->buf[x] <= 0x5a)) &&
                           !((prompt->buf[x] >= 0x61) && (prompt->buf[x] <= 0x7a)) &&
                           (prompt->buf[x] != '_')
			)
			{
                            x--; break;
			}
                    }
                    prompt->buf_sel_end = MIN(x, (int)prompt->buf_len);

		    /* Unmark and redraw other prompts. */
		    PROMPT_UNMARK_REDRAW_ALL(prompt);

                    /* Store the bytes of the selected buffer range. */
                    PromptDDEStoreASCIISeg(
                        &(prompt->buf[prompt->buf_sel_start]),
                        (int)prompt->buf_sel_end - (int)prompt->buf_sel_start
                        + 1
                    );
		}


		/* Record last button press. */
		last_button_press = MilliTime();

                events_handled++;
	    }
            /* Button2: Paste DDE buffer. */
            else if((event->xany.window == prompt->text_area) &&
                    (event->xbutton.button == Button2) &&
                    (!prompt->disabled)
            )
	    {
		len = MIN((int)strlen(prompt->buf), (int)prompt->buf_len);
		tmp_buf = (char *)OSWFetchDDE(&tmp_buf_len);

		/* Unset selection marks. */
		prompt->buf_sel_start = -1;
		prompt->buf_sel_end = -1;

                /* Calculate and set new position of cursor. */
                prompt->buf_pos = (((int)event->xbutton.x - PROMPT_MARGIN)
                    / PROMPT_CHAR_WIDTH) + prompt->buf_vis_pos;
                /* Sanitize position. (Remember we have one extra byte.) */
                if(prompt->buf_pos > (int)len)
                    prompt->buf_pos = (int)len;
                if(prompt->buf_pos < 0)
                    prompt->buf_pos = 0;


		/* Check if there is text selection marked. */
		if((tmp_buf != NULL) &&
		   (tmp_buf_len > 0)
		)
		{
		    /* Shift tailing segment of prompt buffer. */
		    x = len;
		    y = len + tmp_buf_len;
		    if(y > (int)prompt->buf_len)
		    {
			z = y - (int)prompt->buf_len;
			y = MAX(y - z, 0);
			x = MAX(x - z, 0);
		    }

		    while(x >= prompt->buf_pos)
		    {
			prompt->buf[y] = prompt->buf[x];
			x--; y--;
		    }

		    /* Paste buffer in. */
		    y = MAX(prompt->buf_pos, 0);
		    for(x = 0; x < tmp_buf_len; x++, y++)
		    {
			if(y >= ((int)prompt->buf_len - 1))
			{
			    prompt->buf[y] = '\0';
			    break;
			}

			prompt->buf[y] = tmp_buf[x];
		    }
		    /* Null terminate. */
		    prompt->buf[prompt->buf_len] = '\0';

		    /* Free ddn buffer. */
		    OSWGUIFree((void **)&tmp_buf);

		    /* Must move cursor ahead. */
		    prompt->buf_pos = MIN(
			prompt->buf_pos + tmp_buf_len,
			(int)prompt->buf_len - 1
		    );

		    /* Scroll as needed. */
                    OSWGetWindowAttributes(prompt->text_area, &wattr);
                    total_chars_visible = (unsigned int)MAX(
                        ((int)wattr.width - (PROMPT_MARGIN * 2)) /
			PROMPT_CHAR_WIDTH,
                        0
                    );
                    if(prompt->buf_pos >
                       (prompt->buf_vis_pos + total_chars_visible)
                    )
                    {
                        prompt->buf_vis_pos = MAX(
                            (int)prompt->buf_pos - (int)total_chars_visible,
                            0
                        );
                    }
		}
                events_handled++;
	    }
            /* Button3: Set selection mark end (only if there is a start). */
            else if((event->xany.window == prompt->text_area) &&
                    (event->xbutton.button == Button3) &&
               INRANGEINC(prompt->buf_sel_start, 0, (int)prompt->buf_len - 1)
            )
            {
                /* Mark text area button press. */
                text_area_button_state = True;

                /* Record last end mark position. */   
                z = prompt->buf_sel_end;

                /* Calculate and set new position of cursor. */
                prompt->buf_pos = (((int)event->xbutton.x - PROMPT_MARGIN)
                    / PROMPT_CHAR_WIDTH) + prompt->buf_vis_pos;

		/* Ditto for selection mark end. */
		prompt->buf_sel_end = prompt->buf_pos;

                y = strlen(prompt->buf);
                /* Sanitize position. (Remember we have one extra byte.) */
                if(prompt->buf_pos > y) 
                    prompt->buf_pos = y;
                if(prompt->buf_pos < 0)
                    prompt->buf_pos = 0;

		/* Sanitize end. */
                if(prompt->buf_sel_end >= y) 
                    prompt->buf_sel_end = y - 1;
                if(prompt->buf_sel_end < 0) 
                    prompt->buf_sel_end = 0;

                /* Sanitize selection positions. */
                if(prompt->buf_sel_start >= prompt->buf_sel_end)
                {
                    prompt->buf_sel_start = prompt->buf_sel_end;
                    prompt->buf_sel_end = z;

		    /* Must update origin position. */
		    static_wprompt::mark_origin_pos = prompt->buf_sel_end;
                }
		else
		{
		    /* Must update origin position. */
		    static_wprompt::mark_origin_pos = prompt->buf_sel_start;
		}

                /*   No need to unmark other prompts, they already were
		 *   on the start of marking.
		 */

                /* Store the bytes of the selected buffer range. */
                PromptDDEStoreASCIISeg(
                    &(prompt->buf[prompt->buf_sel_start]),
                    (int)prompt->buf_sel_end - (int)prompt->buf_sel_start
                    + 1
                );

                events_handled++;
            }   

            /* Draw text area only as needed. */      
            if(events_handled > 0)
            {
                PromptDraw(prompt, PROMPT_DRAW_AMOUNT_TEXTONLY);
                return(events_handled);
            }
 
            break;

          /* ************************************************************* */
	  case ButtonRelease:
            /* Skip if no text area buffer. */
            if((prompt->buf == NULL) || (prompt->buf_len == 0))
                break;

	    /* Unconditionally set text_area_button_state to False. */
	    if(text_area_button_state)
	    {
		if((event->xany.window == prompt->toplevel) ||
                   (event->xany.window == prompt->text_area)
		)
		{
                    text_area_button_state = False;
            
                    /* Store the bytes of the selected buffer range. */
		    if((prompt->buf_sel_end > -1) &&
                       (prompt->buf_sel_start > -1)
		    )
		    {
                        PromptDDEStoreASCIISeg(
                            &(prompt->buf[prompt->buf_sel_start]),
                            prompt->buf_sel_end - prompt->buf_sel_start + 1
                        );

			PromptDraw(prompt, PROMPT_DRAW_AMOUNT_TEXTONLY);
		    }

                    /* Stop and repeat records. */
                    PromptRepeatRecordClear();

		    events_handled++;
		    return(events_handled);
		}
	    }

	    /* Stop and repeat records. */
	    PromptRepeatRecordClear();

            break;

          /* ************************************************************* */
	  case MotionNotify:
	    /* Do not handle MotionNotify if button not pressed. */
	    if(!text_area_button_state)
		break;

            /* Skip if no text area buffer. */
            if((prompt->buf == NULL) || (prompt->buf_len == 0))
                break;

	    /* Handle mark drag. */
            if((event->xany.window == prompt->text_area) &&
	       (text_area_button_state)
	    )
            {
		OSWGetWindowAttributes(prompt->text_area, &wattr);
		len = strlen(prompt->buf);

		/* Start of marking? */
		if((prompt->buf_sel_end < 0) ||
                   (prompt->buf_sel_start < 0)
		)
		{
		    PROMPT_UNMARK_REDRAW_ALL(prompt);
		}


		/* Scroll left? */
		if((int)event->xmotion.x < 0)
		{
		    /* Scroll one character to the left. */
		    prompt->buf_vis_pos = MAX(
			prompt->buf_vis_pos - 1,
			0
		    );

		    /* Set cursor position. */
		    prompt->buf_pos = prompt->buf_vis_pos;

                    if(prompt->buf_pos < static_wprompt::mark_origin_pos)
                    {
                        /* Marking left. */
                        prompt->buf_sel_start = prompt->buf_pos;
                        prompt->buf_sel_end = static_wprompt::mark_origin_pos;
                    }
                    else
                    {
                        /* Marking right. */
                        prompt->buf_sel_end = prompt->buf_pos;
                        prompt->buf_sel_start = static_wprompt::mark_origin_pos;
                    }

		    /* Schedual next repeat. */
		    PromptRepeatRecordSet(prompt,
			widget_global.prompt_repeat_delay,
			PROMPT_OP_CODE_SCROLL_LEFT
		    );
		}
		/* Scroll right? */
		else if(event->xmotion.x >= (int)wattr.width)
		{
                    /* Calculate total characters visable on window. */
                    total_chars_visible = PROMPT_CHARS_VIS(prompt);

                    /* Scroll one character to the right. */
                    prompt->buf_vis_pos = MIN(
                        prompt->buf_vis_pos + 1,
                        MAX((int)len - total_chars_visible, 0)
                    );
		    /*   Is current length of buffer smaller than
                     *   total_chars_visible?
		     */
		    if(len < total_chars_visible)
			prompt->buf_vis_pos = 0;

                    /* Set cursor position. */
                    prompt->buf_pos = MIN(
			prompt->buf_vis_pos + total_chars_visible,
			(int)len - 1
		    );

                    if(prompt->buf_pos < static_wprompt::mark_origin_pos)
                    {   
                        /* Marking left. */
                        prompt->buf_sel_start = prompt->buf_pos;
                        prompt->buf_sel_end = static_wprompt::mark_origin_pos;
                    }
                    else
                    {
                        /* Marking right. */
                        prompt->buf_sel_end = prompt->buf_pos;
                        prompt->buf_sel_start = static_wprompt::mark_origin_pos;
                    }

                    /* Schedual next repeat. */
                    PromptRepeatRecordSet(prompt,
                        widget_global.prompt_repeat_delay,
                        PROMPT_OP_CODE_SCROLL_RIGHT
                    );
		}		
		/* Normal mark. */
		else
		{
		    /* Calculate new position. */
		    x = MAX(
			(((int)event->xmotion.x - PROMPT_MARGIN)
                            / PROMPT_CHAR_WIDTH) + prompt->buf_vis_pos,
			prompt->buf_vis_pos
		    );
		    if(x < static_wprompt::mark_origin_pos)
		    {
			/* Marking left. */
			prompt->buf_sel_start = x;
			prompt->buf_sel_end = static_wprompt::mark_origin_pos;
			prompt->buf_pos = x;
		    }
		    else
		    {
			/* Marking right. */
			prompt->buf_sel_end = x;
			prompt->buf_sel_start = static_wprompt::mark_origin_pos;
			prompt->buf_pos = x;
		    }

		    /* Stop draw since pointer is back on text area. */
		    PromptRepeatRecordClear();
		}

                /* Sanitize position. (Remember we have one extra byte.) */
		if(prompt->buf_pos > (int)len)
                    prompt->buf_pos = (int)len;
                if(prompt->buf_pos < 0)
                    prompt->buf_pos = 0;

                if(prompt->buf_sel_end >= (int)len)
                    prompt->buf_sel_end = (int)len - 1;
                if(prompt->buf_sel_end < 0)
                    prompt->buf_sel_end = 0;
          
                if(prompt->buf_sel_start >= (int)len)
                    prompt->buf_sel_start = (int)len - 1;
                if(prompt->buf_sel_start < 0)
                    prompt->buf_sel_start = 0;

                events_handled++;
	    }

            /* Draw text area only as needed. */      
            if(events_handled > 0)
            {
                PromptDraw(prompt, PROMPT_DRAW_AMOUNT_TEXTONLY);
                return(events_handled);
            }

            break;

          /* ************************************************************* */
	  case Expose:
	    if(event->xany.window == prompt->toplevel)
	    {
		PromptDraw(prompt, PROMPT_DRAW_AMOUNT_COMPLETE);
	        events_handled++;
		return(events_handled);
	    }
	    else if(event->xany.window == prompt->text_area)
	    {
                PromptDraw(prompt, PROMPT_DRAW_AMOUNT_TEXTONLY);
                events_handled++;
                return(events_handled);
	    }
	    break;

          /* ********************************************************* */
          case VisibilityNotify:
            if(event->xany.window == prompt->toplevel)
            {
                prompt->visibility_state = event->xvisibility.state;
                events_handled++;
                
                /* No need to continue, just return. */
                return(events_handled);
            }
            break;
	}

	/* Redraw as needed. */
	if(events_handled > 0)
	{
	    PromptDraw(prompt, PROMPT_DRAW_AMOUNT_COMPLETE);
	}

	return(events_handled);
}

/*
 *	Maps prompt.
 */
void PromptMap(prompt_window_struct *prompt)
{
        if(prompt == NULL)
            return;

        prompt->map_state = 0;
        PromptDraw(prompt, PROMPT_DRAW_AMOUNT_COMPLETE);

        return;
}

/*
 *	Unmaps prompt.
 */
void PromptUnmap(prompt_window_struct *prompt)
{
	PromptClose(prompt);
	return;
}


/*
 *	Destroy prompt.
 */
void PromptDestroy(prompt_window_struct *prompt)
{
	int i;


	if(prompt == NULL)
	    return;


        /* Delete widget from regeristry. */
        WidgetRegDelete(prompt);


	/*   Clear repeat records that might referance this destroyed
         *   prompt.
         */
	PromptRepeatRecordClear();


	if(IDC())
	{
            OSWDestroyPixmap(&prompt->text_area_buf);
            OSWDestroyWindow(&prompt->text_area);

            OSWDestroyPixmap(&prompt->toplevel_buf);
            OSWDestroyWindow(&prompt->toplevel);
	}


	prompt->map_state = 0;
	prompt->visibility_state = VisibilityFullyObscured;
	prompt->is_in_focus = 0;
	prompt->x = 0;
        prompt->y = 0;
        prompt->width = 0;
        prompt->height = 0;
	prompt->disabled = False;
        prompt->next = NULL;
        prompt->prev = NULL;

	prompt->style = PROMPT_STYLE_FLUSHED;

	prompt->func_cb = NULL;

	/* Free prompt's name. */
	free(prompt->name);
	prompt->name = NULL;

	/* Free prompt's buffer. */
        free(prompt->buf);
	prompt->buf = NULL;
        prompt->buf_len = 0;

        prompt->buf_pos = 0;
        prompt->buf_vis_pos = 0;

        prompt->buf_sel_start = -1;
        prompt->buf_sel_end = -1;

	/* Free history buffers. */
	for(i = 0; i < prompt->total_hist_bufs; i++)
	{
	    if(prompt->hist_buf[i] == NULL)
		continue;

	    free(prompt->hist_buf[i]);
	}
        free(prompt->hist_buf);
	prompt->hist_buf = NULL;
        prompt->total_hist_bufs = 0;

        prompt->hist_buf_pos = 0;


	return;
}



void PromptClose(prompt_window_struct *prompt)
{
        if(prompt == NULL)
	    return;


	/* Unmap (close) the prompt window windows. */
	OSWUnmapWindow(prompt->toplevel);


	/* Set prompt no longer in focus. */
        prompt->is_in_focus = 0;
	prompt->map_state = 0;
	prompt->visibility_state = VisibilityFullyObscured;


	/* Destroy buffers. */
	OSWDestroyPixmap(&prompt->text_area_buf);
	OSWDestroyPixmap(&prompt->toplevel_buf);


	return;
}


/*
 *	Prompt repeat manager.
 */
int PromptManageRepeat(event_t *event)
{
	win_attr_t wattr;
	int len;
	int total_chars_visible;
        int events_handled = 0;
        prompt_window_struct *prompt;

          
        /*
         *   Note: The event information is not used and not needed here.
         */

        /* If not mapped, check if it needs to be mapped. */
        if(prompt_repeat_record[0].prompt != NULL)
        {
            prompt = prompt_repeat_record[0].prompt;

            /* TIme for repeat? */
            if(MilliTime() >= prompt_repeat_record[0].next_repeat)
            {
		/* Get initial values. */
		len = MIN((int)strlen(prompt->buf), (int)prompt->buf_len);

		/* See which operation to perform. */
		switch(prompt_repeat_record[0].op_code)
		{
		  case PROMPT_OP_CODE_SCROLL_LEFT:
                    /* Scroll one character to the left. */
                    prompt->buf_vis_pos = MAX(
                        prompt->buf_vis_pos - 1,
                        0
                    );
                        
                    /* Set cursor position. */
                    prompt->buf_pos = prompt->buf_vis_pos;

                    if(prompt->buf_pos < static_wprompt::mark_origin_pos)
                    {
                        /* Marking left. */
                        prompt->buf_sel_start = prompt->buf_pos;
                        prompt->buf_sel_end = static_wprompt::mark_origin_pos;
                    }
                    else
                    {
                        /* Marking right. */
                        prompt->buf_sel_end = prompt->buf_pos;
                        prompt->buf_sel_start = static_wprompt::mark_origin_pos;
                    }

		    break;


		  case PROMPT_OP_CODE_SCROLL_RIGHT:
                    /* Calculate total characters visable on window. */
                    OSWGetWindowAttributes(prompt->text_area, &wattr);
                    total_chars_visible = PROMPT_CHARS_VIS(prompt);

                    /* Scroll one character to the right. */
                    prompt->buf_vis_pos = MIN(
                        prompt->buf_vis_pos + 1,
                        MAX((int)len - total_chars_visible, 0)
                    );

                    /* Set cursor position. */
                    prompt->buf_pos = MIN(
                        prompt->buf_vis_pos + total_chars_visible,
                        (int)len - 1
                    );

                    if(prompt->buf_pos < static_wprompt::mark_origin_pos)
                    {
                        /* Marking left. */
                        prompt->buf_sel_start = prompt->buf_pos;
                        prompt->buf_sel_end = static_wprompt::mark_origin_pos;
                    }
                    else
                    {
                        /* Marking right. */
                        prompt->buf_sel_end = prompt->buf_pos;
                        prompt->buf_sel_start = static_wprompt::mark_origin_pos;
                    }

		    break;
		}

                /* Sanitize position. (Remember we have one extra byte.) */
                if(prompt->buf_pos > (int)len)
                    prompt->buf_pos = (int)len;
                if(prompt->buf_pos < 0)
                    prompt->buf_pos = 0;
                     
                if(prompt->buf_sel_end >= (int)len)
                    prompt->buf_sel_end = (int)len - 1;
                if(prompt->buf_sel_end < 0)
                    prompt->buf_sel_end = 0;
                     
                if(prompt->buf_sel_start >= (int)len)
                    prompt->buf_sel_start = (int)len - 1;
                if(prompt->buf_sel_start < 0)
                    prompt->buf_sel_start = 0;

		/* ****************************************************** */
                /* Redraw it. */
                PromptDraw(prompt, PROMPT_DRAW_AMOUNT_TEXTONLY);


                /* Schedual next operation. */
                prompt_repeat_record[0].next_repeat =
                    MilliTime() + widget_global.prompt_repeat_interval;


                events_handled++;
            }
        }


        return(events_handled);
}




