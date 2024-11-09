// widgets/wtogglearray.cpp
/*

                    Widget: Toggle Button Array

	Functions:

	int TgBtnArrayInit(
        	toggle_button_array_struct *tba,
        	win_t parent,
        	int x, int y,
        	unsigned int nbtns,
        	int start_sel_btn,
        	char **names,
        	unsigned int nnames,
        	int alignment
	)

	int TgBtnArrayManage(
        	toggle_button_array_struct *tba,
        	event_t *xevent
	)
	int TgBtnArrayDraw(
        	toggle_button_array_struct *tba
	)

	void TgBtnArrayMap(toggle_button_array_struct *tba)
	void TgBtnArrayUnmap(toggle_button_array_struct *tba)

	void TgBtnArrayDestroy(toggle_button_array_struct *tba)

	---



*/

#include "../include/widget.h"

#ifndef MAX
#define MIN(a,b)        (((a) < (b)) ? (a) : (b))
#define MAX(a,b)	(((a) > (b)) ? (a) : (b))
#endif
/* #define MIN(a,b)	((a) < (b)) ? (a) : (b) */
/* #define MAX(a,b)	((a) > (b)) ? (a) : (b) */


/*
 *	Size constants (in pixels):
 */
#define TB_MARGIN	2

#define TB_MIN_WIDTH	((2 * TB_MARGIN) + 2)
#define TB_MIN_HEIGHT	((2 * TB_MARGIN) + 2)

#define TB_CHAR_WIDTH   7
#define TB_CHAR_HEIGHT  14


#define TBA_MARGIN	2

#define TBA_MIN_WIDTH	((2 * TB_MARGIN) + 2)
#define TBA_MIN_HEIGHT	((2 * TB_MARGIN) + 2)

/* Sizes of characters (in pixels). */
#define TBA_CHAR_WIDTH   7
#define TBA_CHAR_HEIGHT  14



/*
 *	Initializes a toggle button array.
 */
int TgBtnArrayInit(
        toggle_button_array_struct *tba,
        win_t parent,
        int x, int y,
        unsigned int nbtns,
        int start_sel_btn,
	char **names,
	unsigned int nnames,
	int alignment
)
{
	int i, status, x_pos, y_pos;
	int longest_line;
	unsigned int width, height;
	image_t *ximage = NULL;


	if((tba == NULL) ||
	   (parent == 0) ||
	   (names == NULL) ||
	   (nbtns == 0)
	)
	    return(-1);


	/* Check if number of names is the same as tbs. */
	if(nbtns != nnames)
	{
	    fprintf(stderr,
"TgBtnArrayInit(): Warning: Number of buttons not same as number of names.\n"
	    );
	}


	/* Reset values. */
	ximage = widget_global.toggle_btn_unarmed_img;

	tba->map_state = 0;
	tba->visibility_state = VisibilityFullyObscured;
	tba->x = x;
	tba->y = y;
	tba->is_in_focus = 0;
	tba->disabled = False;
	tba->font = widget_global.std_font;
	tba->next = NULL;
	tba->prev = NULL;

	tba->total_tbs = nbtns;
	tba->armed_tb = start_sel_btn;


	/* Calculate width and height. */
	switch(alignment)
	{
	  /* Vertical alignment. */
	  case TGBTN_ARRAY_ALIGN_VERTICAL:
	    longest_line = 0;
	    for(i = 0; i < (int)nnames; i++)
	    {
		if(names[i] == NULL)
		    break;

		longest_line = MAX((int)strlen(names[i]), (int)longest_line);
	    }
	    width = ((ximage == NULL) ? 16 : ximage->width) +
                (int)(TB_CHAR_WIDTH * longest_line) + (int)(TB_MARGIN * 3);
	    height = (((ximage == NULL) ? 16 : ximage->height) +
                (int)(TB_MARGIN * 2)) * nbtns;
            break;

	  /* Horizontal alignment. */
	  case TGBTN_ARRAY_ALIGN_HORIZONTAL:
	    width = 0;
            for(i = 0; i < (int)nbtns; i++)
            {
                if(i >= (int)nnames)
                    break;

		width += (int)(strlen(names[i]) * TB_CHAR_WIDTH) +
		    (int)(TB_MARGIN * 5) +
                    ((ximage == NULL) ? 16 : ximage->width);
            }
	    height =  ((ximage == NULL) ? 16 : ximage->height) +
                (int)(TB_MARGIN * 2);
            break;

	  /* Cascade alignment (not supported yet). */
	  case TGBTN_ARRAY_ALIGN_CASCADE:
            fprintf(stderr,
                "TgBtnArrayInit(): Error: Alignment %i not supported.\n",
                alignment
            );
	    return(-1);
	    break;

	  default:
	    fprintf(stderr,
		"TgBtnArrayInit(): Error: Alignment %i not supported.\n",
		alignment
	    );
	    return(-1);
	    break;
	}

	tba->width = MAX((int)width, TBA_MIN_WIDTH);
	tba->height = MAX((int)height, TBA_MIN_HEIGHT);


	/* ******************************************************** */

	/* Initialize toplevel. */
	if(
	    OSWCreateWindow(
	        &tba->toplevel,
                parent,
                tba->x, tba->y,
                tba->width, tba->height
	    )
        )
	    return(-1);

	OSWSetWindowBkg(tba->toplevel, 0, widget_global.std_bkg_pm);

	/* Create toggle buttons. */
	tba->tb = (toggle_button_struct **)calloc(
	    tba->total_tbs,
	    sizeof(toggle_button_struct *)
	);
	if(tba->tb == NULL)
	{
	    tba->total_tbs = 0;
	    return(-1);
	}

	switch(alignment)
	{
          /* Vertical alignment. */
          case TGBTN_ARRAY_ALIGN_VERTICAL:
	    x_pos = 0;
	    y_pos = 0;

	    for(i = 0; i < (int)tba->total_tbs; i++)
	    {
		tba->tb[i] = (toggle_button_struct *)calloc(1,
		    sizeof(toggle_button_struct));
		if(tba->tb[i] == NULL)
		{
		    return(-1);
		}

		if(i >= (int)nnames)
		{
		    status = TgBtnInit(
			tba->tb[i],
			tba->toplevel,
			x_pos, y_pos,
			False,
			NULL
		    );
		}
		else
		{
                    status = TgBtnInit(
                        tba->tb[i],
                        tba->toplevel,
                        x_pos, y_pos,
                        False,
                        names[i]
                    );
		}

		/* Failed to initialize a toggle button? */
		if(status != 0)
		{
		    fprintf(stderr,
        "TgBtnArrayInit(): Error: Unable to create toggle button %i.\n",
			i
		    );
		    return(-1);
		}

		y_pos += ((ximage == NULL) ? 16 : ximage->height) +
                    (int)(TB_MARGIN * 2);
	    }
	    break;

          /* Horizontal alignment. */
          case TGBTN_ARRAY_ALIGN_HORIZONTAL:
            x_pos = 0;
            y_pos = 0;
                 
            for(i = 0; i < (int)tba->total_tbs; i++)
            {
                tba->tb[i] = (toggle_button_struct *)calloc(1,
                    sizeof(toggle_button_struct));
                if(tba->tb[i] == NULL)
                {
                    return(-1);
                }
                      
                if(i >= (int)nnames)
                {
                    status = TgBtnInit(
                        tba->tb[i],
                        tba->toplevel,
                        x_pos, y_pos,
                        False,
                        NULL   
                    );
		    x_pos += (int)(TB_MARGIN * 5) +
                        ((ximage == NULL) ? 16 : ximage->width);
                }
                else
                {
                    status = TgBtnInit(
                        tba->tb[i],
                        tba->toplevel, 
                        x_pos, y_pos,
                        False,
                        names[i]
                    );
		    x_pos += ((int)strlen(names[i]) * TB_CHAR_WIDTH) +
                        (int)(TB_MARGIN * 5) +
                        ((ximage == NULL) ? 16 : ximage->width);
                }


                /* Failed to initialize a toggle button? */
                if(status != 0)
                {
                    fprintf(stderr,
        "TgBtnArrayInit(): Error: Unable to create toggle button %i.\n",
                        i
                    );
                    return(-1);
                }
            }
            break;

          /* Cascade alignment (not supported yet). */ 
          case TGBTN_ARRAY_ALIGN_CASCADE:
            break;


	  default:
	    break;
	}


	/* Set selected armed button. */
	for(i = 0; i < (int)tba->total_tbs; i++)
	{
	    if(tba->tb[i] == NULL)
	        continue;

	    if(i == tba->armed_tb)
		tba->tb[i]->state = True;
	    else
		tba->tb[i]->state = False;
	}


	/* Add widget to regeristry. */
	WidgetRegAdd((void *)tba, WTYPE_CODE_TOGGLEARRAY);


	return(0);
}


/*
 *	Redraw toggle button array.
 */
int TgBtnArrayDraw(toggle_button_array_struct *tba)
{
	int i;


	if(tba == NULL)
	    return(-1);


        /* Map as needed. */
        if(!tba->map_state)
        {
            /* Map as needed. */
            OSWMapWindow(tba->toplevel);

            tba->map_state = 1;
            tba->visibility_state = VisibilityUnobscured;

	    /* Must manually redraw each toggle button widget. */
	    for(i = 0; i < tba->total_tbs; i++)
	    {
		if(tba->tb[i] == NULL)
		    continue;

		if((int)tba->armed_tb == i)
		    tba->tb[i]->state = True;
		else
		    tba->tb[i]->state = False;

		tba->tb[i]->map_state = 0;
		TgBtnDraw(tba->tb[i], TGBTN_DRAW_AMOUNT_COMPLETE);
	    }
        }


	return(0);
}



int TgBtnArrayManage(
        toggle_button_array_struct *tba,
        event_t *event
)
{
	int x, y, z;
	int events_handled = 0;


	if((tba == NULL) ||
	   (event == NULL)
	)
	    return(events_handled);

	if(!tba->map_state &&
           (event->type != MapNotify)
	)
	    return(events_handled);


	switch(event->type)
	{
	  /* ****************************************************** */
	  case ButtonPress:
	    /* Mange ButtonPress here, do not call TgBtnManage(). */

	    /* Skip if entire toggle button array is disabled. */
	    if(tba->disabled)
		return(events_handled);


	    y = 0;
            for(x = 0; x < (int)tba->total_tbs; x++)
            {
		if(tba->tb[x] == NULL)
		    continue;

		if(tba->tb[x]->disabled)
		    continue;

		if(event->xany.window == tba->tb[x]->toplevel)
		{
		    y = 1;
		    break;
		}
            }
	    if(y == 1)
	    {
		tba->armed_tb = x;

		for(z = 0; z < (int)tba->total_tbs; z++)
		{
                    if(tba->tb[z] == NULL)
                        continue;

		    if(z == x)
		    {
			tba->tb[z]->state = True;
		    }
		    else
		    {
			tba->tb[z]->state = False;
		    }

		    TgBtnDraw(
			(toggle_button_struct *)tba->tb[z],
			TGBTN_DRAW_AMOUNT_BUTTON
		    );
		}

		events_handled++;
		return(events_handled);
	    }
	    break;

	  /* ****************************************************** */
	  case Expose:
	    if(event->xany.window == tba->toplevel)
	    {
		TgBtnArrayDraw(tba);

		/* Draw all it's toggle buttons. */
                for(x = 0; x < (int)tba->total_tbs; x++)
                {
		    if(tba->tb[x] == NULL)
			continue;

                    TgBtnDraw(
                        tba->tb[x],
                        TGBTN_DRAW_AMOUNT_COMPLETE
                    );
		}

		events_handled++;
		return(events_handled);
	    }
	    break;

          /* ****************************************************** */
	  case VisibilityNotify:
            if(event->xany.window == tba->toplevel)
	    {
		tba->visibility_state = event->xvisibility.state;

		events_handled++;
		return(events_handled);
	    }
	    break;
	}


	/* Manage events on each toggle button. */
	if(events_handled <= 0)
	{
	    events_handled = 0;

	    for(x = 0; x < (int)tba->total_tbs; x++)
            {
		events_handled += TgBtnManage(
                    tba->tb[x],
                    event
                );

		if(events_handled > 0)
		    break;
	    }
	}


	return(events_handled);
}


/*
 *	Map toggle button array.
 */
void TgBtnArrayMap(toggle_button_array_struct *tba)
{
        if(tba == NULL)
            return;

	tba->map_state = 0;
	TgBtnArrayDraw(tba);

	return;
}

/*
 *	Unmap toggle button array.
 */
void TgBtnArrayUnmap(toggle_button_array_struct *tba)
{
	if(tba == NULL)
	    return;

	OSWUnmapWindow(tba->toplevel);
	tba->map_state = 0;

	return;
}


/*
 *	Destroy toggle button array.
 */
void TgBtnArrayDestroy(toggle_button_array_struct *tba)
{
	int i;


	if(tba == NULL)
	    return;


        /* Delete widget from regeristry. */
        WidgetRegDelete(tba);


	/* Destroy each toggle button. */
	for(i = 0; i < tba->total_tbs; i++)
	{
	    if(tba->tb[i] == NULL)
		continue;

	    /* Destroy toggle button. */
	    TgBtnDestroy(tba->tb[i]);

	    /* Free toggle button structure. */
	    free(tba->tb[i]);
	}
	free(tba->tb);
	tba->tb = NULL;

	tba->total_tbs = 0;
	tba->armed_tb = -1;


	if(IDC())
	{
	    /* Destroy main parrent. */
	    OSWDestroyWindow(&tba->toplevel);
	}

	tba->map_state = 0;
        tba->visibility_state = VisibilityFullyObscured;
	tba->is_in_focus = 0;
        tba->x = 0;
        tba->y = 0;
        tba->width = 0;
        tba->height = 0;
	tba->disabled = False;
	tba->font = NULL;
        tba->next = NULL;
        tba->prev = NULL;


	return;
}




