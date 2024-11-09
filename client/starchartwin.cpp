/*
                               Starchart Window

	Functions:

	int SChtMatchObjectPosition(
		starchart_win_struct *cht,
		int x, int y
	)
	void SChtUpdateFilter(
		starchart_win_struct *cht,
		char *filter
	)
	int SChtMenuCB(void *data, int op_code)
	int SChtZoomInBtnCB(void *data)
	int SChtZoomOutBtnCB(void *data)
	int SChtJumpToPlayerBtnCB(void *data)
	int SChtClearCB(void *data)

	void SChtLoadProgressCB(void *data, int n, int m)
	void SChtOverlayChartCB(
	        starchart_win_struct *cht,
	        char *path
	)
	void SChtSaveChartCB(
	        starchart_win_struct *cht,
	        char *path
	)
	void SChtTimeoutCB(starchart_win_struct *cht)

        int SChtAddObject(
		starchart_win_struct *cht,
		int object_num,
		int type, int isref_num, long size,
		long sect_x, long sect_y, long sect_z,
		double x, double y, double z,
		double heading, double pitch, double bank
        )
	int SChtSetObjectName(
		starchart_win_struct *cht,
		char *name
	)
        int SChtSetObjectEmpire(
		starchart_win_struct *cht,
		char *empire
	)
	int SChtRecycleObject(
		starchart_win_struct *cht,
		int object_num
	)

	int SChtInit(starchart_win_struct *cht)
	void SChtResize(starchart_win_struct *cht)
	void SChtDraw(starchart_win_struct *cht, int amount)
	int SChtManage(starchart_win_struct *cht, event_t *event)
	void SChtMap(starchart_win_struct *cht)
	void SChtUnmap(starchart_win_struct *cht)
	void SChtDestroy(starchart_win_struct *cht)

	---

 */

#include "../include/unvmatch.h"
#include "../include/unvmath.h"
#include "../include/unvutil.h"
#include "../include/unvfile.h"
#include "../include/xsw_ctype.h"
#include "starchartwin.h"
#include "xsw.h"
#include "net.h"

#include <regex.h>


#define MIN(a,b)        (((a) < (b)) ? (a) : (b))
#define MAX(a,b)        (((a) > (b)) ? (a) : (b))


/*
 *	Starchart window menu codes:
 */
#define SCHT_MENU_CODE_NONE		0
#define SCHT_MENU_CODE_OVERLAY		10
#define SCHT_MENU_CODE_SAVE		11
#define SCHT_MENU_CODE_SAVEAS		12
#define SCHT_MENU_CODE_CLEAR		13
#define SCHT_MENU_CODE_CLOSE		20


#define SCHT_MENU_CODE_ZOOM_IN		30
#define SCHT_MENU_CODE_ZOOM_OUT		31
#define SCHT_MENU_CODE_ZOOM_MAX		32
#define SCHT_MENU_CODE_ZOOM_MIN		33

#define SCHT_MENU_CODE_JUMP_TO_PLAYER	34

#define SCHT_MENU_CODE_FOLLOW_PLAYER_TOGGLE	35
#define SCHT_MENU_CODE_SHOW_LABELS_TOGGLE	36

#define SCHT_MENU_CODE_INTERCEPT	37


/*
 *	Menu item positions:
 */
#define SCHT_MENU_ITEM_POS_FOLLOW_PLAYER	1, 7
#define SCHT_MENU_ITEM_POS_SHOW_LABELS		1, 8

/*
 *	Deliminator character for filter argument:
 */
#define SCHT_FILTER_DELIMINATOR_CHR	':'

/*
 *	Default filter argument:
 */
#define SCHT_DEF_FILTER		"n:* e:*"


/*
 *	Timming intervals (in milliseconds):
 */
#define SCHT_PLAYER_FOLLOW_INT	1000
#define SCHT_FILTER_UPDATE_INT	500


/*
 *      Default sizes:
 */     
#define SCHT_DEF_WIDTH          400
#define SCHT_DEF_HEIGHT         380

#define SCHT_MIN_WIDTH		100
#define SCHT_MIN_HEIGHT		100

#define SCHT_MARGIN		5

#define SCHT_MENUBAR_HEIGHT	30

#define SCHT_PROMPT_WIDTH	120
#define SCHT_PROMPT_HEIGHT	30

#define SCHT_VIEW_BTN_WIDTH	30
#define SCHT_VIEW_BTN_HEIGHT	30

#define SCHT_PBAR_HEIGHT	12

/* Object marker diameter in pixels. */
#define SCHTObjectMarkerDiameter	10


/*
 *	Button hint messages:
 */
#define SCHT_HINT_ZOOM_IN		"Zoom in (+)"
#define SCHT_HINT_ZOOM_OUT		"Zoom out (-)"
#define SCHT_HINT_JUMP_TO_PLAYER	"Jump to player (J)"

/*
 *	Zoom limits:
 */
#define SCHT_VIEW_ZOOM_MIN	0.003
#define SCHT_VIEW_ZOOM_MAX	1.000
#define SCHT_VIEW_DEF_ZOOM	0.300


/*
 *	Star chart object flags:
 */
#define SCHT_OBJ_FLAG_FILTEROUT		(1 << 1)


/*
 *	Button press state records:
 */
static bool_t	view_trans_state,
		view_zoom_state;

static starchart_win_struct *last_cht;

/*
 *	Next timmings (in milliseconds):
 */
static time_t	next_player_follow = 0,
		next_filter_update = 0;




starchart_win_struct starchart_win;



/*
 *	Matches object based on the given coordinates in pixels.
 *
 *	Returns the matched object or -1 on no match.
 */
int SChtMatchObjectPosition(
	starchart_win_struct *cht,
	int x, int y
)
{
        int i, obj_x = 0, obj_y = 0, sw, sh, swv, shv;
	double cur_x, cur_y, sect_width, sect_height;
	long cur_sect_x, cur_sect_y;
        long sect_x_min, sect_x_max, sect_y_min, sect_y_max;
        long d_sect_x, d_sect_y;
	xsw_object_struct **obj_ptr;
        win_attr_t wattr;

	const int obj_marker_dim = SCHTObjectMarkerDiameter;


        if(cht == NULL)
            return(-1);

	OSWGetWindowAttributes(cht->view, &wattr);


        /* Sanitize zoom. */
        if(cht->zoom < SCHT_VIEW_ZOOM_MIN)
            cht->zoom = SCHT_VIEW_ZOOM_MIN;
        if(cht->zoom > SCHT_VIEW_ZOOM_MAX)
            cht->zoom = SCHT_VIEW_ZOOM_MAX;

	/* Get sector size and positions. */
        sect_width = sector_legend.x_len;
        sect_height = sector_legend.y_len;

        cur_x = PromptGetF(&cht->x_prompt);
        cur_y = PromptGetF(&cht->y_prompt);

        cur_sect_x = PromptGetL(&cht->sect_x_prompt);
        cur_sect_y = PromptGetL(&cht->sect_y_prompt);


	/* Zoomed width of a sector. */
        sw = MAX((int)(sect_width * cht->zoom), 1);

        /* Sectors visable on width. */
        if(sw != 0)
            swv = ((int)wattr.width / sw);
        else
            swv = 0;

        /* Zoomed height of a sector. */
        sh = MAX((int)(sect_height * cht->zoom), 1);

        /* Sectors visable on height. */
        if(sh != 0)
            shv = ((int)wattr.height / sh);
        else
            shv = 0;

        /*   Calculate sector bounds and add `good measure'.
         *   Note that swv and shv are already calculated.
         */
        sect_x_min = cur_sect_x - (long)swv - 1;
        sect_x_max = cur_sect_x + (long)swv + 1;
        sect_y_min = cur_sect_y - (long)shv - 1;
        sect_y_max = cur_sect_y + (long)shv + 1;


        /* Go through objects. */
        for(i = 0, obj_ptr = cht->object; 
            i < cht->total_objects;
            i++, obj_ptr++
        )
        {
            if(*obj_ptr == NULL)
                continue;
  
            /* Object is garbage? */   
            if((*obj_ptr)->type <= XSW_OBJ_TYPE_GARBAGE)
                continue;

	    /* Filtered out? */
	    if((*obj_ptr)->client_options & SCHT_OBJ_FLAG_FILTEROUT)
		continue;
        
            /* Object in bounds? */
            if(((*obj_ptr)->sect_x < sect_x_min) ||
               ((*obj_ptr)->sect_x > sect_x_max) ||
               ((*obj_ptr)->sect_y < sect_y_min) ||
               ((*obj_ptr)->sect_y > sect_y_max)
            )
                continue;

            /* Calculate sector deltas. */
            d_sect_x = (*obj_ptr)->sect_x - cur_sect_x;
            d_sect_y = (*obj_ptr)->sect_y - cur_sect_y;
        
            /* Calculate X coordinate position in units of pixels. */
            if(d_sect_x == 0)
            {
                obj_x = static_cast<int>(((*obj_ptr)->x - cur_x) * cht->zoom);
            }
            else if(d_sect_x < 0)
            {
                obj_x = static_cast<int>((((sect_width / 2) - (*obj_ptr)->x) +
                        ((d_sect_x + 1) * sect_width * -1) +
                        (cur_x + (sect_width / 2)))
                        * cht->zoom * -1);
            }
            else if(d_sect_x > 0)
            {
                obj_x = static_cast<int>((((sect_width / 2) - cur_x) +
                        ((d_sect_x - 1) * sect_width) +
                        ((*obj_ptr)->x + (sect_width / 2)))
                        * cht->zoom);
            }
            obj_x += ((int)wattr.width / 2);
        
            /* Calculate Y coordinate position in units of pixels. */
            if(d_sect_y == 0)
            {
                obj_y = static_cast<int>(((*obj_ptr)->y - cur_y) * cht->zoom);
            }
            else if(d_sect_y < 0)
            {
                obj_y = static_cast<int>((((sect_height / 2) - (*obj_ptr)->y) +
                        ((d_sect_y + 1) * sect_height * -1) +
                        (cur_y + (sect_height / 2)))
                        * cht->zoom * -1);
            }
            else if(d_sect_y > 0)
            {
                obj_y = static_cast<int>((((sect_height / 2) - cur_y) +
                        ((d_sect_y - 1) * sect_height) +
                        ((*obj_ptr)->y + (sect_height / 2)))
                        * cht->zoom);
            }
            obj_y = (((int)wattr.height / 2) - obj_y);
                        
                        
            /*   Check if x and y are within object's coordinates
             *   in pixels.
             */
            if((x >= (obj_x - (obj_marker_dim / 2))) &&
               (x < (obj_x + (obj_marker_dim / 2))) &&
               (y >= (obj_y - (obj_marker_dim / 2))) &&
               (y < (obj_y + (obj_marker_dim / 2)))
            )
                return(i);
        }

        return(-1);
}

/*
 *	Updates star chart window's objects' filter flag with
 *	the new filter to apply.
 */
void SChtUpdateFilter(
	starchart_win_struct *cht,
	char *filter			/* Filter expression string. */
)
{
        int i, total, is_and;
        xsw_object_struct *obj_ptr, **ptr;
        char *strptr, *strptr2, *strptr3, *filter_end;
const int len = 256;
        char name[len];
	int and_name = 0;
        char empire[len];
	int and_empire = 0;
	regex_t preg;


	if((cht == NULL) ||
	   (filter == NULL)
	)
	    return;


	/* Reset name and empire patterns. */
	*name = '\0';
	*empire = '\0';


	/* Seek null character position in filter. */
	filter_end = filter;
	while(*filter_end != '\0')
	    filter_end++;

	/* Parse filter string. */
	strptr = filter;
	while(ISBLANK(*strptr))
	    strptr++;

	while(strptr < filter_end)
	{
	    /* Check for `and' operator. */
	    if(*strptr == '&')
	    {
		is_and = 1;
		strptr++;
	    }
	    else if(*strptr == '|')
	    {
		is_and = 0;
		strptr++;
	    }
	    else
	    {
		is_and = 0;
	    }
            if(strptr >= filter_end)
                break;

	    /* Get pointer to pattern string buffer. */
	    if(*strptr == 'e')	/* Empire. */
	    {
		strptr2 = empire;
		and_empire = is_and;
	    }
	    else		/* Name (all else). */
	    {
		strptr2 = name;
		and_name = is_and;
	    }
	    *strptr2 = '\0';	/* Reset pattern string buffer. */

	    /* Seek immediate deliminator in filter. */
	    strptr = strchr(strptr, SCHT_FILTER_DELIMINATOR_CHR);
	    if(strptr != NULL)
	    {
		strptr++;

		strncpy(strptr2, strptr, len);
		strptr2[len - 1] = '\0';

		strptr3 = strchr(strptr2, SCHT_FILTER_DELIMINATOR_CHR);
		if(strptr3 != NULL)
		{
		    *strptr3 = '\0';
		    strptr3 = strrchr(strptr2, ' ');
		    if(strptr3 != NULL)
			*strptr3 = '\0';
		}

                /* Seek next deliminator in filter. */
                strptr = strchr(strptr, SCHT_FILTER_DELIMINATOR_CHR);
		if(strptr == NULL)
		{
		    break;
		}
		else
		{
		    while(strptr > filter)
		    {
		        if(ISBLANK(*(strptr - 1)))
			    break;
			strptr--;
		    }
		    if(strptr <= filter)
			break;
		}
	    }
	    else
	    {
		break;
	    }
	}

	/* Empire pattern must always be uppercase. */
	strtoupper(empire);



	/* Begin filtering objects by name first. */
        if((*name != '\0') &&
           !regcomp(&preg, name, REG_EXTENDED | REG_ICASE | REG_NOSUB)
	)
        {
	    for(i = 0, total = cht->total_objects, ptr = cht->object;
                i < total;
                i++, ptr++
	    )
	    {
	        obj_ptr = *ptr;
	        if(obj_ptr == NULL)
		    continue;

	        /* Filter name if specified. */
	        if((*name != '\0') &&
                   (*(obj_ptr->name) != '\0')
	        )
	        {
		    if(regexec(
		        &preg,
		        obj_ptr->name,
		        0, NULL,
		        0
		    ))
		        obj_ptr->client_options |= SCHT_OBJ_FLAG_FILTEROUT;
		    else
		        obj_ptr->client_options &= ~SCHT_OBJ_FLAG_FILTEROUT;
	        }
	    }
	    regfree(&preg);
	}


        /* Begin filtering objects by empire. */
        if((*empire != '\0') &&
           !regcomp(&preg, empire, REG_EXTENDED | REG_ICASE | REG_NOSUB)
	)
	{
            for(i = 0, total = cht->total_objects, ptr = cht->object;
                i < total;
                i++, ptr++
            )
            {
                obj_ptr = *ptr;
                if(obj_ptr == NULL)
                    continue;

		/* And empire and already filtered out? */
		if(and_empire &&
                   (obj_ptr->client_options & SCHT_OBJ_FLAG_FILTEROUT)
		)
		    continue;

                /* Filter empire if specified. */
                if(*empire != '\0')
                {
                    if(regexec(
                        &preg,
                        ((*(obj_ptr->empire) == '\0') ?
			    XSW_DEF_EMPIRE_STR : obj_ptr->empire),
                        0, NULL,
                        0
                    ))
		        obj_ptr->client_options |= SCHT_OBJ_FLAG_FILTEROUT;
		    else
                        obj_ptr->client_options &= ~SCHT_OBJ_FLAG_FILTEROUT;
                }
	    }
            regfree(&preg);
	}


	return;
}


/*
 *	Menu callback.
 */
int SChtMenuCB(void *data, int op_code)
{
	starchart_win_struct *cht;


	cht = (starchart_win_struct *)data;
	if(cht == NULL)
	    return(-1);

	switch(op_code)
	{
	  case SCHT_MENU_CODE_NONE:
	    break;

	  /* Open star chart overlay. */
	  case SCHT_MENU_CODE_OVERLAY:
	    XSWMapFB(NULL, PRI_FB_LOADOP_STARCHART_OVERLAY);
	    break;

	  /* Save as. */
	  case SCHT_MENU_CODE_SAVEAS:
	    XSWMapFB(NULL, PRI_FB_SAVEOP_STARCHART);
	    break;

          /* Clear objects in star chart window. */
          case SCHT_MENU_CODE_CLEAR:
            SChtClearCB(cht);
            break;

	  /* Unmap star chart window. */
	  case SCHT_MENU_CODE_CLOSE:
	    SChtUnmap(cht);
	    break;

	  /* Zoom in. */
	  case SCHT_MENU_CODE_ZOOM_IN:
	    SChtZoomInBtnCB(cht);
	    break;

          /* Zoom out. */
          case SCHT_MENU_CODE_ZOOM_OUT:
            SChtZoomOutBtnCB(cht);
            break;

          /* Zoom max. */
          case SCHT_MENU_CODE_ZOOM_MAX:
	    cht->zoom = SCHT_VIEW_ZOOM_MAX;
            SChtZoomInBtnCB(cht);
            break;

          /* Zoom min. */
          case SCHT_MENU_CODE_ZOOM_MIN:
            cht->zoom = SCHT_VIEW_ZOOM_MIN;
            SChtZoomOutBtnCB(cht);
            break;

	  /* Jump to player. */
	  case SCHT_MENU_CODE_JUMP_TO_PLAYER:
	    SChtJumpToPlayerBtnCB(cht);
	    break;

	  /* Follow player. */
	  case SCHT_MENU_CODE_FOLLOW_PLAYER_TOGGLE:
	    cht->follow_player_tb.state = 
		MenuBarGetItemToggleState(
		    &cht->mb,
		    SCHT_MENU_ITEM_POS_FOLLOW_PLAYER
		);
	    if(cht->follow_player_tb.map_state)
		TgBtnDraw(&cht->follow_player_tb, TGBTN_DRAW_AMOUNT_COMPLETE);
	    break;

	  /* Show labels. */
	  case SCHT_MENU_CODE_SHOW_LABELS_TOGGLE:
	    if(cht->map_state)
		SChtDraw(cht, SCHT_DRAW_AMOUNT_VIEW);
	    break;

	  /* Intercept. */
	  case SCHT_MENU_CODE_INTERCEPT:
	    if(cht->selected_object > -1)
	    {
		int selected_object;
		xsw_object_struct *obj_ptr;

		selected_object = cht->selected_object;
		if(!UNVIsObjectGarbage(
		    cht->object,
		    cht->total_objects,
		    selected_object
		))
		{
		    obj_ptr = cht->object[selected_object];
		    if(*(obj_ptr->name) != '\0')
		    {
			NetSendIntercept(
			    net_parms.player_obj_num,
			    obj_ptr->name
			);
		    }
		}
	    }
	    break;
	}


	return(0);
}

/*
 *	Zoom in button callback.
 */
int SChtZoomInBtnCB(void *data)
{
	starchart_win_struct *cht;


	cht = (starchart_win_struct *)data;
	if(cht == NULL)
	    return(-1);

	cht->zoom += (0.1 * cht->zoom);
	if(cht->zoom > SCHT_VIEW_ZOOM_MAX)
	    cht->zoom = SCHT_VIEW_ZOOM_MAX;

        SChtDraw(cht, SCHT_DRAW_AMOUNT_VIEW);

	return(0);
}

/*
 *	Zoom out button callback.
 */
int SChtZoomOutBtnCB(void *data)
{
        starchart_win_struct *cht;


        cht = (starchart_win_struct *)data;
        if(cht == NULL)
            return(-1);

        cht->zoom -= (0.1 * cht->zoom);
        if(cht->zoom < SCHT_VIEW_ZOOM_MIN)
            cht->zoom = SCHT_VIEW_ZOOM_MIN;

        SChtDraw(cht, SCHT_DRAW_AMOUNT_VIEW);

        return(0);
}

/*
 *	Jump to player button callback.
 */
int SChtJumpToPlayerBtnCB(void *data)
{
        starchart_win_struct *cht;
	xsw_object_struct *obj_ptr;


        cht = (starchart_win_struct *)data;
        if(cht == NULL)
            return(-1);

	obj_ptr = net_parms.player_obj_ptr;
	if(obj_ptr == NULL)
	{
	    printdw(
		&err_dw,
"Player object does not exist, make sure that you are currently\n\
connected and logged in."
	    );
	}
	else
	{
	    PromptSetF(&cht->x_prompt, obj_ptr->x);
            PromptSetF(&cht->y_prompt, obj_ptr->y);
            PromptSetI(&cht->sect_x_prompt, obj_ptr->sect_x);
            PromptSetI(&cht->sect_y_prompt, obj_ptr->sect_y);

            SChtDraw(cht, SCHT_DRAW_AMOUNT_VIEW);
	}

	return(0);
}

/*
 *	Deletes all objects in star chart.
 */
int SChtClearCB(void *data)
{
        starchart_win_struct *cht;
  
  
        cht = (starchart_win_struct *)data;
        if(cht == NULL)
            return(-1);

        /* Delete all objects in chart. */
        UNVDeleteAllObjects(cht->object, cht->total_objects);
        cht->object = NULL;
        cht->total_objects = 0;
	cht->selected_object = -1;
         
        /* Reset header. */
        memset(&cht->header, 0x00, sizeof(unv_head_struct));
        cht->header.ru_to_au = sw_units.ru_to_au;	/* Use global ru to au. */

	/* Redraw as needed. */
	if(cht->map_state)
           SChtDraw(cht, SCHT_DRAW_AMOUNT_VIEW);

	return(0);
}


/*
 *	Load universe file progress callback.
 */
void SChtLoadProgressCB(void *data, int n, int m)
{
	starchart_win_struct *cht;
	double value;


	cht = (starchart_win_struct *)data;
	if(cht == NULL)
	    return;

	if(cht->map_state)
	{
	    if(m > 0)
		value = (double)n / (double)m;
	    else
		value = 0;

	    PBarUpdate(&cht->pbar, value, NULL);
	}

	return;
}

/*
 *	Overlay chart callback.
 */
void SChtOverlayChartCB(
        starchart_win_struct *cht,
        char *path
)
{
	char unv_title[80];
	char win_title[256];


	if(cht == NULL)
	    return;


	/* Load new universe file as chart. */
	if(path != NULL)
	{
	    xsw_object_struct **new_object;
	    int i, n, new_total, prev_total;

	    new_object = UNVLoadFromFile(
		path,
		&new_total,
		&cht->header,		/* Header. */
		cht,			/* Client data. */
		SChtLoadProgressCB	/* Progress notify CB. */
	    );

	    /* Get new title from chart file. */
	    strncpy(unv_title, cht->header.title, 80);
	    unv_title[80 - 1] = '\0';

	    /* Merge new objects into array. */
	    prev_total = cht->total_objects;
	    cht->total_objects += new_total;

	    if(cht->total_objects > 0)
	    {
	        cht->object = (xsw_object_struct **)realloc(
		    cht->object,
		    cht->total_objects * sizeof(xsw_object_struct *)
	        );
		if(cht->object == NULL)
		{
		    cht->total_objects = 0;
		    return;
		}

		/* Referance newly allocated objects. */
		for(i = prev_total, n = 0; i < cht->total_objects; i++, n++)
		    cht->object[i] = new_object[n];

		/* Free new object pointer array. */
		free(new_object);
		new_object = NULL;

		/* Schedual filter. */
		next_filter_update = cur_millitime + SCHT_FILTER_UPDATE_INT;
	    }
	}
	else
	{
	    *unv_title = '\0';
	}

	/* Set new title. */
	sprintf(
	    win_title,
	    "%s Star Chart: %s",
	    PROG_NAME, unv_title
	);
	OSWSetWindowTitle(cht->toplevel, win_title);

	/* Redraw everything. */
	if(cht->map_state)
	    SChtDraw(cht, SCHT_DRAW_AMOUNT_COMPLETE);

	return;
}

/*
 *	Save chart callback.
 */
void SChtSaveChartCB(
        starchart_win_struct *cht,
        char *path  
)
{
                
        if(cht == NULL)
            return;


        /* Is path specified? */
        if(path != NULL)
        {
	    int status;
	    char tmp_path[PATH_MAX + NAME_MAX];
	    char text[PATH_MAX + NAME_MAX + 80];


	    strncpy(tmp_path, path, PATH_MAX + NAME_MAX);
	    tmp_path[PATH_MAX + NAME_MAX - 1] = '\0';
	    status = UNVSaveToFile(
                tmp_path,
                cht->object,
		cht->total_objects,
                &cht->header,		/* Header. */
		cht,			/* Client data. */
		SChtLoadProgressCB	/* Progress callback. */
            );
	    if(status)
	    {
		sprintf(text,
		    "Error saving chart to:\n\n%s",
		    tmp_path
		);
		printdw(&err_dw, text);
	    }
	    else
	    {
/* Don't notify of success. */
/*
                sprintf(text,
                    "Saved objects in Star Chart to:\n\n%s",
                    tmp_path
                );
                printdw(&info_dw, text);
 */
	    }
	}

        /* Redraw everything. */
        if(cht->map_state)
            SChtDraw(cht, SCHT_DRAW_AMOUNT_COMPLETE);

        return;
}


/*
 *	Starchart timeout callback.
 */
void SChtTimeoutCB(starchart_win_struct *cht)
{
	static time_t last_time = 0;


	/* Update timming. */
	if(last_time > cur_millitime)
	{
	    next_player_follow = 0;
	    next_filter_update = 0;
	}
	last_time = cur_millitime;


	if(cht == NULL)
	    return;


	/* Time to update player following? */
	if(next_player_follow <= cur_millitime)
	{
	    xsw_object_struct *obj_ptr;

	    obj_ptr = net_parms.player_obj_ptr;
	    if((obj_ptr != NULL) && (cht->follow_player_tb.state))
	    {
		PromptSetF(&cht->x_prompt, obj_ptr->x);
                PromptSetF(&cht->y_prompt, obj_ptr->y);

                PromptSetI(&cht->sect_x_prompt, obj_ptr->sect_x);
                PromptSetI(&cht->sect_y_prompt, obj_ptr->sect_y);

		if(cht->map_state)
		    SChtDraw(cht, SCHT_DRAW_AMOUNT_VIEW);


	    }

	    /* Schedual next. */
	    next_player_follow = cur_millitime + SCHT_PLAYER_FOLLOW_INT;
	}

	/* Time to update filter? */
	if(next_filter_update > 0)
	{
	    if(next_filter_update <= cur_millitime)
	    {
                SChtUpdateFilter(cht, PromptGetS(&cht->filter_prompt));

		if(cht->map_state)
		    SChtDraw(cht, SCHT_DRAW_AMOUNT_VIEW);

		/* Reset next filter update to 0, so it dosen't get
		 * updated again.
		 */
		next_filter_update = 0;
	    }
	}

	return;
}


/*
 *	Adds (appends) object to star chart.
 */
int SChtAddObject(
	starchart_win_struct *cht,
	int object_num,
	int type, int isref_num, long size,
        long sect_x, long sect_y, long sect_z,
        double x, double y, double z,
        double heading, double pitch, double bank
)
{
	int n;
	xsw_object_struct *obj_ptr;


	if(cht == NULL)
	    return(-1);


	/* Sanitize total. */
	if(cht->total_objects < 0)
	    cht->total_objects = 0;



	/* Add (append) object to star chart. */
	n = cht->total_objects;
	cht->total_objects++;
	cht->object = (xsw_object_struct **)realloc(
	    cht->object,
	    cht->total_objects * sizeof(xsw_object_struct *)
	);
	if(cht->object == NULL)
	{
	    cht->total_objects = 0;
	    return(-1);
	}

	/* Allocate new object. */
	cht->object[n] = (xsw_object_struct *)calloc(
	    1,
	    sizeof(xsw_object_struct)
	);
	obj_ptr = cht->object[n];
	if(obj_ptr == NULL)
	    return(-1);

	/* Reset new object. */
	UNVResetObject(obj_ptr);


	/* Set new values on object. */
	obj_ptr->type = type;
	obj_ptr->imageset = isref_num;
	obj_ptr->size = size;

	obj_ptr->sect_x = sect_x;
        obj_ptr->sect_y = sect_y;
        obj_ptr->sect_z = sect_z;

        obj_ptr->x = x;
        obj_ptr->y = y;
        obj_ptr->z = z;

	obj_ptr->heading = heading;
        obj_ptr->pitch = pitch;
        obj_ptr->bank = bank;

	MuSetUnitVector2D(
	    &obj_ptr->attitude_vector_compoent,
	    obj_ptr->heading
        );

	/* Update units. */
	cht->header.ru_to_au = sw_units.ru_to_au;

        /* Redraw star chart as needed. */
	if(cht->map_state)
	    SChtDraw(cht, SCHT_DRAW_AMOUNT_VIEW);

	return(0);
}

/*
 *	Sets name for last object in star chart.
 */
int SChtSetObjectName(
	starchart_win_struct *cht,
	char *name
)
{
        int n;
        xsw_object_struct *obj_ptr;
          
        
        if(cht == NULL)
            return(-1);

	/* Get last object. */
	n = cht->total_objects - 1;
	if(UNVIsObjectGarbage(
	    cht->object,
	    cht->total_objects,
	    n
	))
	    return(-1);
	else
	    obj_ptr = cht->object[n];

	/* Set object's name. */
	strncpy(
	    obj_ptr->name,
	    name,
	    XSW_OBJ_NAME_MAX
	);
	obj_ptr->name[XSW_OBJ_NAME_MAX - 1] = '\0';


        /* Schedual filter update. */
	next_filter_update = cur_millitime + SCHT_FILTER_UPDATE_INT;


	/* Redraw star chart as needed. */
        if(cht->map_state)
            SChtDraw(cht, SCHT_DRAW_AMOUNT_VIEW);

	return(0);
}

/*
 *	Sets empire for last object in star chart.     
 */
int SChtSetObjectEmpire(
	starchart_win_struct *cht,
	char *empire
)
{
        int n;
        xsw_object_struct *obj_ptr;


        if(cht == NULL)
            return(-1);

        /* Get last object. */
        n = cht->total_objects - 1;
        if(UNVIsObjectGarbage(
            cht->object,
            cht->total_objects,
            n
        ))
            return(-1);
        else
            obj_ptr = cht->object[n];

        /* Set object's empire. */
        strncpy(
            obj_ptr->empire,
            empire,
            XSW_OBJ_EMPIRE_MAX  
        );
        obj_ptr->empire[XSW_OBJ_EMPIRE_MAX - 1] = '\0';


        /* Schedual filter update. */
	next_filter_update = cur_millitime + SCHT_FILTER_UPDATE_INT;


        /* Redraw star chart as needed. */
        if(cht->map_state)
            SChtDraw(cht, SCHT_DRAW_AMOUNT_VIEW);

	return(0);
}

/*
 *	Recycles an object on the star chart (not supported).
 */
int SChtRecycleObject(
	starchart_win_struct *cht,
	int object_num
)
{




	return(0);
}



/*
 *	Initializes the starchart window.
 */
int SChtInit(starchart_win_struct *cht)
{
	int i, n, x, y;
        pixmap_t pixmap;
	image_t *img_ptr;
        win_attr_t wattr;
	menu_struct *menu;
        char title[256];


	if(cht == NULL)
	    return(0);

        /* Reset values. */
        cht->map_state = 0;
        cht->is_in_focus = 0;
        cht->visibility_state = VisibilityFullyObscured;
        cht->disabled = False;

        /* Positions may have been fetched from configuration file,
         * check if they are valid and sanitize.
         */
	if(cht->width == 0)
	    cht->width = SCHT_DEF_WIDTH;
	if(cht->height == 0)
            cht->height = SCHT_DEF_HEIGHT;

	if(cht->width < SCHT_MIN_WIDTH)
	    cht->width = SCHT_MIN_WIDTH;
        else if(cht->width > osw_gui[0].display_width)
	    cht->width = osw_gui[0].display_width;

        if(cht->height < SCHT_MIN_HEIGHT)
            cht->height = SCHT_MIN_HEIGHT;
        else if(cht->height > osw_gui[0].display_height)
            cht->height = osw_gui[0].display_height;


        /* Create toplevel window. */
        if(
            OSWCreateWindow(
                &cht->toplevel,
                osw_gui[0].root_win,
                cht->x,
                cht->y,
                cht->width,
                cht->height
            )
        )
            return(-1);
/*
        cht->toplevel_buf = 0;
 */
        OSWSetWindowInput(cht->toplevel, OSW_EVENTMASK_TOPLEVEL);

        WidgetCenterWindow(cht->toplevel, WidgetCenterWindowToRoot);
        OSWGetWindowAttributes(cht->toplevel, &wattr);
        cht->x = wattr.x;
        cht->y = wattr.y;
        cht->width = wattr.width;
        cht->height = wattr.height;

        /* WM properties. */
	sprintf(title, "%s Star Chart", PROG_NAME);
        if(IMGIsImageNumAllocated(IMG_CODE_STARCHART_ICON))
            pixmap = OSWCreatePixmapFromImage(
                xsw_image[IMG_CODE_STARCHART_ICON]->image
            );
        else
            pixmap = widget_global.std_icon_pm;
        OSWSetWindowWMProperties(
            cht->toplevel,
            title,		/* Title. */
            "Star Chart",	/* Icon title. */
            pixmap,		/* Icon. */
            False,		/* Let WM set coordinates? */
            /* Coordinates. */
            cht->x, cht->y,
            /* Min width and height. */  
            SCHT_MIN_WIDTH, SCHT_MIN_HEIGHT,
            /* Max width and height. */
            osw_gui[0].display_width, osw_gui[0].display_height,
            WindowFrameStyleStandard,
            NULL, 0
        );

	/* Menu bar. */
	if(MenuBarInit(
	    &cht->mb,
            cht->toplevel,
            0, 0,
            cht->width, SCHT_MENUBAR_HEIGHT,
            SChtMenuCB,
            cht
        ))
            return(-1);

        if(MenuBarAddItem(
            &cht->mb,
            -1,		/* Append. */
            "Chart",
            0, 0,
            0, 0
        ))
            return(-1);

        if(MenuBarAddItem(
            &cht->mb,
            -1,         /* Append. */
            "View",
            cht->mb.item[0]->x +
                (int)cht->mb.item[0]->width,
	    0,
            0, 0
        ))
            return(-1);

        /* Chart menu. */  
        i = 0;
        if(MenuBarIsItemAllocated(&cht->mb, i))
	    menu = cht->mb.item[i]->menu;
	else
            return(-1);
        MenuBarAddItemMenuItem(
            &cht->mb, i,
            "Overlay...",
            MENU_ITEM_TYPE_ENTRY,
            NULL,
            SCHT_MENU_CODE_OVERLAY,
            -1
        );
        MenuBarAddItemMenuItem(
            &cht->mb, i,  
            "Save As...",
            MENU_ITEM_TYPE_ENTRY,
            NULL,  
            SCHT_MENU_CODE_SAVEAS,
            -1
        );
        MenuBarAddItemMenuItem(
            &cht->mb, i,
            "Clear",
	    MENU_ITEM_TYPE_ENTRY,
            NULL,
	    SCHT_MENU_CODE_CLEAR,
            -1
        );
        MenuBarAddItemMenuItem(
            &cht->mb, i,
            NULL,
            MENU_ITEM_TYPE_HR,
            NULL,
            SCHT_MENU_CODE_NONE,
            -1
        );
        MenuBarAddItemMenuItem(
            &cht->mb, i,
            "Close",
            MENU_ITEM_TYPE_ENTRY,
            NULL,
            SCHT_MENU_CODE_CLOSE,
            -1
        );

	/* View menu. */
        i = 1;
        if(MenuBarIsItemAllocated(&cht->mb, i))
            menu = cht->mb.item[i]->menu;
	else
            return(-1);
        MenuBarAddItemMenuItem(
            &cht->mb, i,
            "Zoom In",
            MENU_ITEM_TYPE_ENTRY,
            NULL,
            SCHT_MENU_CODE_ZOOM_IN,
            -1
        );
        MenuBarAddItemMenuItem(
            &cht->mb, i,
            "Zoom Out",
            MENU_ITEM_TYPE_ENTRY,
            NULL,
            SCHT_MENU_CODE_ZOOM_OUT,
            -1
        );
        MenuBarAddItemMenuItem(  
            &cht->mb, i,
            "Zoom Maximum",
            MENU_ITEM_TYPE_ENTRY,
            NULL,
            SCHT_MENU_CODE_ZOOM_MAX,
            -1
        );
        MenuBarAddItemMenuItem(
            &cht->mb, i,
            "Zoom Minimum",
            MENU_ITEM_TYPE_ENTRY,
            NULL,
            SCHT_MENU_CODE_ZOOM_MIN,
            -1
        );
        MenuBarAddItemMenuItem(
            &cht->mb, i,
            NULL,
            MENU_ITEM_TYPE_HR, 
            NULL,
            SCHT_MENU_CODE_NONE,
            -1
        );
        MenuBarAddItemMenuItem(
            &cht->mb, i,
            "Jump To Player",
            MENU_ITEM_TYPE_ENTRY,
            NULL,
            SCHT_MENU_CODE_JUMP_TO_PLAYER,
            -1
        );
	n = MenuGetItemNumberByID(menu, SCHT_MENU_CODE_JUMP_TO_PLAYER);
        MenuSetItemAccelerator(menu, n, 'j', 0);

        MenuBarAddItemMenuItem(
            &cht->mb, i,
            NULL,
            MENU_ITEM_TYPE_HR,
            NULL,
            SCHT_MENU_CODE_NONE, 
            -1   
        );
        MenuBarAddItemMenuItem(
            &cht->mb, i,
            "Follow Player",  
            MENU_ITEM_TYPE_TOGGLEENTRY,
            NULL,
            SCHT_MENU_CODE_FOLLOW_PLAYER_TOGGLE,
            -1
	);
        n = MenuGetItemNumberByID(menu, SCHT_MENU_CODE_FOLLOW_PLAYER_TOGGLE);
        MenuSetItemAccelerator(menu, n, 'f', 0);
        MenuBarSetItemToggleState(
            &cht->mb,
            SCHT_MENU_ITEM_POS_FOLLOW_PLAYER,
            False
        );
        MenuBarAddItemMenuItem(
            &cht->mb, i,
            "Show Labels",
            MENU_ITEM_TYPE_TOGGLEENTRY,
            NULL,
            SCHT_MENU_CODE_SHOW_LABELS_TOGGLE,
            -1   
        );
        n = MenuGetItemNumberByID(menu, SCHT_MENU_CODE_SHOW_LABELS_TOGGLE);
        MenuSetItemAccelerator(menu, n, 'l', 0);
        MenuBarSetItemToggleState(
            &cht->mb,
            SCHT_MENU_ITEM_POS_SHOW_LABELS,
            True
	);


	/* View window. */
        if(
            OSWCreateWindow(
                &cht->view,
		cht->toplevel,
                0,
		SCHT_MENUBAR_HEIGHT + (2 * SCHT_MARGIN) +
		    SCHT_VIEW_BTN_HEIGHT,
                cht->width,
                (int)cht->height - (2 * SCHT_PROMPT_HEIGHT) -
                    (5 * SCHT_MARGIN) - SCHT_MENUBAR_HEIGHT -
		    SCHT_VIEW_BTN_HEIGHT
            )
        )
            return(-1);
        cht->view_buf = 0;
        OSWSetWindowInput(
	    cht->view,
	    ButtonPressMask | ButtonReleaseMask | PointerMotionMask |
	    ExposureMask
	);


        /* Right-click menu. */
	menu = &cht->qmenu;
        if(MenuInit(
            menu,
            osw_gui[0].root_win,
            SChtMenuCB,
            cht
        ))
            return(-1);

        MenuAddItem(
            menu,
            "Intercept",
            MENU_ITEM_TYPE_ENTRY,
            NULL,
            SCHT_MENU_CODE_INTERCEPT,
            -1
        );
/*
        MenuBarAddItemMenuItem(
            menu,
            NULL,
            MENU_ITEM_TYPE_HR,
            NULL,
            SCHT_MENU_CODE_NONE,
            -1
        );
        MenuAddItem(
            menu,
            "Show Labels",
            MENU_ITEM_TYPE_TOGGLEENTRY,
            NULL,
            SCHT_MENU_CODE_SHOW_LABELS_TOGGLE,
            -1
        );
        n = MenuGetItemNumberByID(menu, SCHT_MENU_CODE_SHOW_LABELS_TOGGLE);
	MenuSetItemState(menu, n, True);
*/


	/* Buttons in tool bar. */
	i = IMG_CODE_SCHT_ZOOM_OUT;
        if(IMGIsImageNumAllocated(i))
            img_ptr = xsw_image[i]->image;
        else
            img_ptr = NULL; 
	if(PBtnInit(
	    &cht->zoom_out_btn,
	    cht->toplevel,
            (0 * SCHT_VIEW_BTN_WIDTH) + (1 * SCHT_MARGIN),
	    SCHT_MENUBAR_HEIGHT + (1 * SCHT_MARGIN),
	    SCHT_VIEW_BTN_WIDTH, SCHT_VIEW_BTN_HEIGHT,
	    ((img_ptr == NULL) ? "-" : NULL),
	    PBTN_TALIGN_CENTER,
	    img_ptr,
	    cht,
	    SChtZoomOutBtnCB
	))
	    return(-1);
	PBtnSetHintMessage(
            &cht->zoom_out_btn,
            SCHT_HINT_ZOOM_OUT
        );

        i = IMG_CODE_SCHT_ZOOM_IN;
        if(IMGIsImageNumAllocated(i))
            img_ptr = xsw_image[i]->image;
        else
            img_ptr = NULL;
        if(PBtnInit(
            &cht->zoom_in_btn,
            cht->toplevel,            
            (1 * SCHT_VIEW_BTN_WIDTH) + (2 * SCHT_MARGIN),
            SCHT_MENUBAR_HEIGHT + (1 * SCHT_MARGIN),      
            SCHT_VIEW_BTN_WIDTH, SCHT_VIEW_BTN_HEIGHT,
            ((img_ptr == NULL) ? "+" : NULL),
            PBTN_TALIGN_CENTER,
            img_ptr,
            cht,
            SChtZoomInBtnCB
        ))
            return(-1);
        PBtnSetHintMessage(
            &cht->zoom_in_btn,
            SCHT_HINT_ZOOM_IN
        );

        i = IMG_CODE_SCHT_JUMP_TO_PLAYER;
        if(IMGIsImageNumAllocated(i))
            img_ptr = xsw_image[i]->image;
        else
            img_ptr = NULL;
        if(PBtnInit(
            &cht->jump_to_player_btn,
            cht->toplevel,
            (2 * SCHT_VIEW_BTN_WIDTH) + (3 * SCHT_MARGIN),
            SCHT_MENUBAR_HEIGHT + (1 * SCHT_MARGIN),
            SCHT_VIEW_BTN_WIDTH, SCHT_VIEW_BTN_HEIGHT,
            ((img_ptr == NULL) ? "J" : NULL),
            PBTN_TALIGN_CENTER,
            img_ptr,
            cht,
            SChtJumpToPlayerBtnCB
        ))
            return(-1);
        PBtnSetHintMessage(
            &cht->jump_to_player_btn,
            SCHT_HINT_JUMP_TO_PLAYER
        );


	/* Filter prompt. */
        if(
            PromptInit(
                &cht->filter_prompt,
                cht->toplevel,
                (3 * SCHT_VIEW_BTN_WIDTH) + (4 * SCHT_MARGIN),
                SCHT_MENUBAR_HEIGHT + (1 * SCHT_MARGIN) +
		    (SCHT_VIEW_BTN_HEIGHT / 2) -
		    (SCHT_PROMPT_HEIGHT / 2),
                MAX(
		    (int)cht->width - (3 * SCHT_VIEW_BTN_WIDTH) -
		    (5 * SCHT_MARGIN),
		    0
		),
		SCHT_PROMPT_HEIGHT,
                PROMPT_STYLE_FLUSHED,
                "Filter:",
                256,
                0,
                NULL
            )
        )
            return(-1);
        PromptSetS(&cht->filter_prompt, SCHT_DEF_FILTER);

        /* Coordinate prompts. */
        if(
            PromptInit(
                &cht->x_prompt,
                cht->toplevel,
                (1 * SCHT_MARGIN) + (0 * SCHT_PROMPT_WIDTH),
                (int)cht->height - (2 * SCHT_PROMPT_HEIGHT) -
                    (2 * SCHT_MARGIN),
		SCHT_PROMPT_WIDTH, SCHT_PROMPT_HEIGHT,
                PROMPT_STYLE_FLUSHED,
                "X:",
                20,
                0,
                NULL
            )
        )
            return(-1);
	PromptSetF(&cht->x_prompt, 0);

        if(
            PromptInit(
                &cht->y_prompt,
                cht->toplevel,
                (2 * SCHT_MARGIN) + (1 * SCHT_PROMPT_WIDTH),
                (int)cht->height - (2 * SCHT_PROMPT_HEIGHT) -
                    (2 * SCHT_MARGIN),
                SCHT_PROMPT_WIDTH, SCHT_PROMPT_HEIGHT,
                PROMPT_STYLE_FLUSHED,
                "Y:",
                20,
                0,
                NULL
            )
        )
            return(-1);
        PromptSetF(&cht->y_prompt, 0);
 
/*
        if(
            PromptInit(
                &cht->z_prompt,
                cht->toplevel, 
                (3 * SCHT_MARGIN) + (2 * SCHT_PROMPT_WIDTH),
                (int)cht->height - (2 * SCHT_PROMPT_HEIGHT) -
                    (2 * SCHT_MARGIN),
                SCHT_PROMPT_WIDTH, SCHT_PROMPT_HEIGHT,
                PROMPT_STYLE_FLUSHED,
                "Z:",
                20,  
                0, 
                NULL
            )   
        )    
            return(-1);
 */

        if(
            PromptInit(
                &cht->sect_x_prompt,
                cht->toplevel,
                (1 * SCHT_MARGIN) + (0 * SCHT_PROMPT_WIDTH),
                (int)cht->height - (1 * SCHT_PROMPT_HEIGHT) -
                    (1 * SCHT_MARGIN),
                SCHT_PROMPT_WIDTH, SCHT_PROMPT_HEIGHT,
                PROMPT_STYLE_FLUSHED,
                "Sect X:",
                20,  
                0, 
                NULL
            )   
        )    
            return(-1);
        PromptSetI(&cht->sect_x_prompt, 0);
 
        if(
            PromptInit(
                &cht->sect_y_prompt,
                cht->toplevel,
                (2 * SCHT_MARGIN) + (1 * SCHT_PROMPT_WIDTH),
                (int)cht->height - (1 * SCHT_PROMPT_HEIGHT) -
                    (1 * SCHT_MARGIN),
                SCHT_PROMPT_WIDTH, SCHT_PROMPT_HEIGHT,
                PROMPT_STYLE_FLUSHED,
                "Sect Y:",
                20,
                0,  
                NULL
            )
        )
            return(-1);
	PromptSetI(&cht->sect_y_prompt, 0);

/*
        if(
            PromptInit(
                &cht->sect_z_prompt,
                cht->toplevel,
                (3 * SCHT_MARGIN) + (2 * SCHT_PROMPT_WIDTH),
                (int)cht->height - (1 * SCHT_PROMPT_HEIGHT) -
                    (1 * SCHT_MARGIN),
                SCHT_PROMPT_WIDTH, SCHT_PROMPT_HEIGHT,
                PROMPT_STYLE_FLUSHED,
                "Sect Y:",
                20,
                0, 
                NULL
            )   
        )    
            return(-1);
 */

	/* Link prompts togeather. */
	cht->x_prompt.prev = &cht->sect_y_prompt;
        cht->x_prompt.next = &cht->y_prompt;
	cht->y_prompt.prev = &cht->x_prompt;
	cht->y_prompt.next = &cht->sect_x_prompt;

        cht->sect_x_prompt.prev = &cht->y_prompt;
        cht->sect_x_prompt.next = &cht->sect_y_prompt;
        cht->sect_y_prompt.prev = &cht->sect_x_prompt;
        cht->sect_y_prompt.next = &cht->x_prompt;



	/* Follow player toggle. */
	if(TgBtnInit(
	    &cht->follow_player_tb,
	    cht->toplevel,
            (3 * SCHT_MARGIN) + (2 * SCHT_PROMPT_WIDTH),
            (int)cht->height - (2 * SCHT_PROMPT_HEIGHT) -
                (2 * SCHT_MARGIN),
	    False,
	    "Follow Player"
        ))
	    return(-1);

	/* Progress bar. */
        x = (3 * SCHT_MARGIN) + (2 * SCHT_PROMPT_WIDTH);
        y = (int)cht->height - (1 * SCHT_PROMPT_HEIGHT) -
            (1 * SCHT_MARGIN) + (SCHT_PROMPT_HEIGHT / 2) -
            (SCHT_PBAR_HEIGHT / 2);
	if(PBarInit(
	    &cht->pbar,
	    cht->toplevel,
            x, y,
            MAX((int)cht->width - x - (2 * SCHT_MARGIN), 100),
            SCHT_PBAR_HEIGHT,
            0, 0, 1,
            NULL,
            PBAR_COMPLETION_HOLD
        ))
	    return(-1);


	/* Reset other values. */
	cht->zoom = SCHT_VIEW_DEF_ZOOM;
	cht->ref_win = NULL;

	cht->object = NULL; 
	cht->total_objects = 0;
	cht->selected_object = -1;

        cht->measuring = False;
        cht->measure_start_x = 0;
        cht->measure_start_y = 0; 
        cht->measure_end_x = 0;
        cht->measure_end_y = 0;


	/* Reset universe header. */
        memset(&cht->header, 0x00, sizeof(unv_head_struct));
	cht->header.ru_to_au = sw_units.ru_to_au;


	return(0);
}

/*
 *	Resizes the starchart window.
 */
void SChtResize(starchart_win_struct *cht)
{
	int x, y;
	win_attr_t wattr;


	if(cht == NULL)
	    return;

        /* Get new size and check for change. */
        OSWGetWindowAttributes(cht->toplevel, &wattr);
        if((cht->width == (unsigned int)wattr.width) &&
           (cht->height == (unsigned int)wattr.height)
        )
            return;

        /* Set new size values. */
        cht->x = wattr.x;
        cht->y = wattr.y;
        cht->width = wattr.width;  
        cht->height = wattr.height;


        /* Recreate toplevel buffer. */
/*
        OSWDestroyPixmap(&cht->toplevel_buf);


 */

	/* Menu bar. */
	OSWMoveResizeWindow(
	    cht->mb.toplevel,
	    0, 0,
	    cht->width, SCHT_MENUBAR_HEIGHT
	);
	MenuBarResize(&cht->mb);


	/* Buttons. */
	OSWMoveWindow(
	    cht->zoom_out_btn.toplevel,
            (0 * SCHT_VIEW_BTN_WIDTH) + (1 * SCHT_MARGIN),
            SCHT_MENUBAR_HEIGHT + (1 * SCHT_MARGIN)
	);
        OSWMoveWindow(
            cht->zoom_in_btn.toplevel,
            (1 * SCHT_VIEW_BTN_WIDTH) + (2 * SCHT_MARGIN),
            SCHT_MENUBAR_HEIGHT + (1 * SCHT_MARGIN)   
        );
        OSWMoveWindow(
            cht->jump_to_player_btn.toplevel,
            (2 * SCHT_VIEW_BTN_WIDTH) + (3 * SCHT_MARGIN),
            SCHT_MENUBAR_HEIGHT + (1 * SCHT_MARGIN)   
        );

	/* Filter prompt. */
        OSWMoveResizeWindow(
	    cht->filter_prompt.toplevel,
	    (3 * SCHT_VIEW_BTN_WIDTH) + (4 * SCHT_MARGIN),
            SCHT_MENUBAR_HEIGHT + (1 * SCHT_MARGIN) +    
                (SCHT_VIEW_BTN_HEIGHT / 2) -
                (SCHT_PROMPT_HEIGHT / 2),
            MAX(
                (int)cht->width - (3 * SCHT_VIEW_BTN_WIDTH) -
                (5 * SCHT_MARGIN),
                100
            ),
            SCHT_PROMPT_HEIGHT
	);
	PromptMap(&cht->filter_prompt);


	/* Resize view. */
	OSWMoveResizeWindow(
	    cht->view,
            0,
            SCHT_MENUBAR_HEIGHT + (2 * SCHT_MARGIN) + 
                SCHT_VIEW_BTN_HEIGHT,
            cht->width,
            (int)cht->height - (2 * SCHT_PROMPT_HEIGHT) -
                (5 * SCHT_MARGIN) - SCHT_MENUBAR_HEIGHT -
                SCHT_VIEW_BTN_HEIGHT
	);
	OSWGetWindowAttributes(cht->view, &wattr);

	OSWDestroyPixmap(&cht->view_buf);
	OSWCreatePixmap(&cht->view_buf, wattr.width, wattr.height);




        /* Coordinate prompts. */
        OSWMoveWindow(
	    cht->x_prompt.toplevel,
            (1 * SCHT_MARGIN) + (0 * SCHT_PROMPT_WIDTH),
            (int)cht->height - (2 * SCHT_PROMPT_HEIGHT) -
                (2 * SCHT_MARGIN)
        );

        OSWMoveWindow(   
            cht->y_prompt.toplevel,
            (2 * SCHT_MARGIN) + (1 * SCHT_PROMPT_WIDTH),
            (int)cht->height - (2 * SCHT_PROMPT_HEIGHT) -
                (2 * SCHT_MARGIN)
        );
/*
        OSWMoveWindow(   
            cht->z_prompt.toplevel,
            (3 * SCHT_MARGIN) + (2 * SCHT_PROMPT_WIDTH),
            (int)cht->height - (2 * SCHT_PROMPT_HEIGHT) -
                (2 * SCHT_MARGIN)
        );
 */

        OSWMoveWindow(   
            cht->sect_x_prompt.toplevel,
            (1 * SCHT_MARGIN) + (0 * SCHT_PROMPT_WIDTH),
            (int)cht->height - (1 * SCHT_PROMPT_HEIGHT) -
                (1 * SCHT_MARGIN)
        );

        OSWMoveWindow(
            cht->sect_y_prompt.toplevel,
            (2 * SCHT_MARGIN) + (1 * SCHT_PROMPT_WIDTH),
            (int)cht->height - (1 * SCHT_PROMPT_HEIGHT) -
                (1 * SCHT_MARGIN)
        );  
/*  
        OSWMoveWindow(
            cht->sect_z_prompt.toplevel,
            (3 * SCHT_MARGIN) + (2 * SCHT_PROMPT_WIDTH),
            (int)cht->height - (1 * SCHT_PROMPT_HEIGHT) -
                (1 * SCHT_MARGIN)
        );
 */


	/* Follow player toggle. */
	OSWMoveWindow(
            cht->follow_player_tb.toplevel,
            (3 * SCHT_MARGIN) + (2 * SCHT_PROMPT_WIDTH),
            (int)cht->height - (2 * SCHT_PROMPT_HEIGHT) -
                (2 * SCHT_MARGIN)
        );

	/* Progress bar. */
	x = (3 * SCHT_MARGIN) + (2 * SCHT_PROMPT_WIDTH);
	y = (int)cht->height - (1 * SCHT_PROMPT_HEIGHT) -
            (1 * SCHT_MARGIN) + (SCHT_PROMPT_HEIGHT / 2) -
            (SCHT_PBAR_HEIGHT / 2);
	OSWMoveResizeWindow(
            cht->pbar.toplevel,
	    x, y,
            MAX((int)cht->width - x - (2 * SCHT_MARGIN), 100),
            SCHT_PBAR_HEIGHT
        );
	PBarResize(&cht->pbar);

	return;
}

/*
 *	Redraws the starchart window.
 */
void SChtDraw(starchart_win_struct *cht, int amount)
{
	if(cht == NULL)
	    return;


	/* Map as needed. */
	if(!cht->map_state)
	{
	    OSWMapRaised(cht->toplevel);
	    MenuBarMap(&cht->mb);
            OSWMapWindow(cht->view);
	    PBtnMap(&cht->zoom_in_btn);
            PBtnMap(&cht->zoom_out_btn);
            PBtnMap(&cht->jump_to_player_btn);
            PromptMap(&cht->filter_prompt);
            TgBtnMap(&cht->follow_player_tb);
            PBarMap(&cht->pbar);
            PromptMap(&cht->x_prompt);
            PromptMap(&cht->y_prompt);
/*          PromptMap(&cht->z_prompt); */
            PromptMap(&cht->sect_x_prompt);
            PromptMap(&cht->sect_y_prompt);
/*	    PromptMap(&cht->sect_z_prompt); */

	    cht->map_state = 1;
	}


	/* Redraw toplevel? */
	if(amount == SCHT_DRAW_AMOUNT_COMPLETE)
	{
	    win_t w;
	    win_attr_t wattr;


	    w = cht->toplevel;
	    OSWGetWindowAttributes(w, &wattr);

            /* Redraw background. */
            if(widget_global.force_mono)
            {
                OSWClearPixmap(
                    w,
                    wattr.width, wattr.height,
                    osw_gui[0].black_pix
                );
            }
            else
            {
                WidgetPutImageTile(
                    w,
                    widget_global.std_bkg_img,
                    wattr.width, wattr.height
                );
            }

	}

	/* Redraw view? */
	if((amount == SCHT_DRAW_AMOUNT_COMPLETE) ||
	   (amount == SCHT_DRAW_AMOUNT_VIEW)
	)
	{
            long cur_sect_x, cur_sect_y;
            double cur_x, cur_y, sect_width, sect_height;
	    double zoom, grid_spacing;

            int x, y, i, sw, sh, swv, shv;
            int pos, inc;
            long sect_cur;
            long sect_x_min, sect_x_max, sect_y_min, sect_y_max;
            long d_sect_x, d_sect_y;
	    int total;
	    xsw_object_struct *obj_ptr, **ptr;
	    bool_t draw_object_labels;
            win_t w;
            pixmap_t pixmap;
	    font_t *prev_font;
            win_attr_t wattr;

            const int obj_marker_dim = SCHTObjectMarkerDiameter;
 

	    w = cht->view;
	    pixmap = cht->view_buf;

	    prev_font = OSWQueryCurrentFont();
	    OSWGetWindowAttributes(w, &wattr);

	    /* Recreate pixmap as needed. */
	    if(pixmap == 0)
	    {
		if(OSWCreatePixmap(
		    &cht->view_buf, wattr.width, wattr.height
		))
		    return;

		pixmap = cht->view_buf;
	    }

	    /* Show object labels? */
            draw_object_labels =
                MenuBarGetItemToggleState(
                    &cht->mb,
                    SCHT_MENU_ITEM_POS_SHOW_LABELS
                );


	    /* Sanitize zoom. */
            if(cht->zoom < SCHT_VIEW_ZOOM_MIN)
                cht->zoom = SCHT_VIEW_ZOOM_MIN;
            if(cht->zoom > SCHT_VIEW_ZOOM_MAX)
                cht->zoom = SCHT_VIEW_ZOOM_MAX;
	    zoom = cht->zoom;

	    grid_spacing = 200;

	    sect_width = sector_legend.x_len;
	    sect_height = sector_legend.y_len;

	    cur_x = PromptGetF(&cht->x_prompt);
            cur_y = PromptGetF(&cht->y_prompt);

	    cur_sect_x = PromptGetL(&cht->sect_x_prompt);
            cur_sect_y = PromptGetL(&cht->sect_y_prompt);


	    /* ***************************************************** */
	    /* Clear background. */
	    OSWClearPixmap(
		pixmap,
		wattr.width, wattr.height,
		xsw_color.chart_bg
	    );

            /* ***************************************************** */
            /* Draw grids (not sector grids). */

            inc = static_cast<int>(grid_spacing * zoom);
            swv = ((int)wattr.width / inc);
            x = static_cast<int>(((grid_spacing / 2) - cur_x) * zoom) +
                ((int)wattr.width / 2);

            if(inc >= 20)
            {
		OSWSetFgPix(xsw_color.chart_grid);
                for(pos = x % inc; pos < (int)wattr.width; pos += inc)
                {
                    if(pos < 0)
                        continue;

                    OSWDrawLine(
			pixmap,
			pos, 0,
			pos, (int)wattr.height - 1
                    );
                }
            }


            inc = static_cast<int>(grid_spacing * zoom);
            shv = ((int)wattr.height / inc);
            y = static_cast<int>((((grid_spacing / 2) - cur_y) * zoom) +
                ((int)wattr.height / 2));

            if(inc >= 20)
            {
		OSWSetFgPix(xsw_color.chart_grid);
                for(pos = ((int)wattr.height - y) % inc;
                    pos < (int)wattr.height;
                    pos += inc
                )
                {
                    if(pos < 0)
                        continue;

                    OSWDrawLine(
                        pixmap,
                        0, pos,
                        (int)wattr.width - 1, pos
                    );
                }
            }


            /* ***************************************************** */
            /* Draw vertical sector grids. */

            /* Zoomed width of a sector. */
            sw = MAX((int)(sect_width * zoom), 1);

            /* Sectors visable on width. */
            if(sw != 0)
                swv = ((int)wattr.width / sw);
            else
                swv = 0;

            /* Position of starting grid line. */
            x = static_cast<int>((((sect_width / 2) - cur_x) * zoom)
                + ((int)wattr.width / 2) - (swv * sw));

            sect_cur = cur_sect_x - swv + 1;

            OSWSetFgPix(xsw_color.chart_sector_grid);

            for(pos = x, inc = sw;
                pos < (int)wattr.width;
                pos += inc, sect_cur++
	    )
	    {
                if(pos < 0)
                    continue;

		OSWDrawLine(
		    pixmap,
		    pos, 0,
		    pos, (int)wattr.height - 1
		);
	    }


            /* ***************************************************** */
	    /* Draw horizontal sector grids. */

            /* Zoomed height of a sector. */
            sh = MAX((int)(sect_height * zoom), 1);

	    /* Sectors visable on height. */
            if(sh != 0)
                shv = ((int)wattr.height / sh);
            else
                shv = 0;

            /* Position of starting grid line. */
            y = static_cast<int>(((int)wattr.height -
                ((((sect_height / 2) - cur_y) * zoom)
                + ((int)wattr.height / 2))) - (shv * sh));

            sect_cur = cur_sect_y + shv;

            OSWSetFgPix(xsw_color.chart_sector_grid);

            for(pos = y, inc = sh;
                pos < (int)wattr.height;
                pos += inc, sect_cur--
	    )
	    {
		if(pos < 0)
                    continue;

                OSWDrawLine( 
                    pixmap,
                    0, pos,
                    (int)wattr.width - 1, pos
                );
	    }


            /* ***************************************************** */
            /* Draw objects. */

            /* Calculate sector bounds and add `good measure'.
             * Note that swv and shv are already calculated.
             */
            sect_x_min = cur_sect_x - (long)swv - 1;
            sect_x_max = cur_sect_x + (long)swv + 1;
            sect_y_min = cur_sect_y - (long)shv - 1;
            sect_y_max = cur_sect_y + (long)shv + 1;

	    OSWSetFont(xsw_font.console_standard);

	    /* Begin drawing each object, draw total number of
	     * objects + 1, the last + 1 will be the player object.
	     */
            for(i = 0, ptr = cht->object, total = cht->total_objects + 1;
                i < total;
                i++, ptr++
            )
            {
		/* Last object + 1 is to be treated as player object. */
		if(i == (total - 1))
		{
		    obj_ptr = net_parms.player_obj_ptr;
                    if(obj_ptr == NULL)
                        continue;
                    if(obj_ptr->type <= XSW_OBJ_TYPE_GARBAGE)
                        continue;
		}
		else
		{
		    obj_ptr = *ptr;
		    if(obj_ptr == NULL)
			continue;
                    if(obj_ptr->type <= XSW_OBJ_TYPE_GARBAGE)
                        continue;

                    /* Filtered out? */
                    if(obj_ptr->client_options & SCHT_OBJ_FLAG_FILTEROUT)
                        continue;
		}

                /* Object in bounds? */
                if((obj_ptr->sect_x < sect_x_min) ||
                   (obj_ptr->sect_x > sect_x_max) ||
                   (obj_ptr->sect_y < sect_y_min) ||
                   (obj_ptr->sect_y > sect_y_max)
                )
		    continue;

                /* Calculate sector deltas. */
                d_sect_x = obj_ptr->sect_x - cur_sect_x;
                d_sect_y = obj_ptr->sect_y - cur_sect_y;


                /* Calculate X coordinate position. */
                if(d_sect_x == 0)
                {
                    x = static_cast<int>((obj_ptr->x - cur_x) * zoom);
                }
                else if(d_sect_x < 0)
                {
                    x = static_cast<int>((((sect_width / 2) - obj_ptr->x) +
                        ((d_sect_x + 1) * sect_width * -1) +
                        (cur_x + (sect_width / 2)))
                        * zoom * -1);
                }
                else if(d_sect_x > 0)
                {
                    x = static_cast<int>((((sect_width / 2) - cur_x) +
                        ((d_sect_x - 1) * sect_width) +
                        (obj_ptr->x + (sect_width / 2)))
                        * zoom);
                }
                x += ((int)wattr.width / 2);

                /* Calculate Y coordinate position. */
                if(d_sect_y == 0)
                {       
                    y = static_cast<int>((obj_ptr->y - cur_y) * zoom);
                }
                else if(d_sect_y < 0)
                {
                    y = static_cast<int>((((sect_height / 2) - obj_ptr->y) +
                        ((d_sect_y + 1) * sect_height * -1) +
                        (cur_y + (sect_height / 2)))
                        * zoom * -1);
                }
                else if(d_sect_y > 0)
                {
                    y = static_cast<int>((((sect_height / 2) - cur_y) +
                        ((d_sect_y - 1) * sect_height) +
                        (obj_ptr->y + (sect_height / 2)))
                        * zoom);
                }
                y = (((int)wattr.height / 2) - y);


		/* Is this the player object? */
		if(obj_ptr == net_parms.player_obj_ptr)
		{
		    /* Draw cross hairs representing player object. */

		    OSWSetFgPix(xsw_color.chart_cross_hairs);

		    /* Y coordinate bar. */
                    OSWDrawLine(
                        pixmap,
                        0, y, (int)wattr.width, y
                    );
		    /* Is x coordinate off screen? */
		    if(x >= ((int)wattr.width - 2))
		    {
			/* Draw right edge arrow. */
			OSWDrawLine(
			    pixmap,
			    (int)wattr.width - 6, y - 4,
			    (int)wattr.width - 2, y
			);
			OSWDrawLine(
                            pixmap,
                            (int)wattr.width - 6, y + 4,
                            (int)wattr.width - 2, y
                        );
		    }
                    else if(x < 2)
                    {
			/* Draw left edge arrow. */
                        OSWDrawLine(
                            pixmap,
                            5, y - 4,
                            1, y    
                        );
                        OSWDrawLine(
                            pixmap,
                            5, y + 4,
                            1, y    
                        );
                    }

		    /* X coordinate bar. */
                    OSWDrawLine(
                        pixmap,
                        x, 0, x, (int)wattr.height
                    );
                    /* Is y coordinate off screen? */
                    if(y >= ((int)wattr.height - 2))
                    {
                        /* Draw bottom edge arrow. */
                        OSWDrawLine(
                            pixmap,
                            x - 4, (int)wattr.height - 6,
                            x, (int)wattr.height - 2
                        );
                        OSWDrawLine(
                            pixmap,
                            x + 4, (int)wattr.height - 6,
                            x, (int)wattr.height - 2
                        );
                    }
                    else if(y < 2)
                    {
                        /* Draw top edge arrow. */
                        OSWDrawLine(
                            pixmap,
                            x - 4, 5,
                            x, 1
                        );
                        OSWDrawLine(
                            pixmap,
                            x + 4, 5,
                            x, 1
                        );
		    }
		}
		else
		{
		    /* Is this object selected? */
		    if(cht->selected_object == i)
		    {
                        double d, r;
			xsw_object_struct *obj_ptr2;
                        sw_units_struct u;
                        char *units_name_ptr;
			char text[XSW_OBJ_NAME_MAX + XSW_OBJ_EMPIRE_MAX + 80];


                        /* Set scanner lock color. */
                        OSWSetFgPix(xsw_color.scmark_locked);

			/* Draw name and position of this object. */
			sprintf(text, "(%s) %s",
			    obj_ptr->empire,
			    obj_ptr->name
			);
			OSWDrawString(pixmap, 5, 16, text);

			obj_ptr2 = net_parms.player_obj_ptr;
			if(obj_ptr2 != NULL)
			{
			    /* Draw bearing and distance only if in same
			     * sector.
			     */
			    if(Mu3DInSameSectorPtr(obj_ptr, obj_ptr2))
			    {
				r = MuCoordinateDeltaVector(
				    obj_ptr->x - obj_ptr2->x,
				    obj_ptr->y - obj_ptr2->y
				);
				r = RADTODEG(r);

				/* Calculate distance in XSW Real units. */
				d = Mu3DDistance(
				    obj_ptr->x - obj_ptr2->x,
                                    obj_ptr->y - obj_ptr2->y,
				    obj_ptr->z - obj_ptr2->z
				);

                                /* Convert units. */
                                u.ru_to_au = cht->header.ru_to_au;
                                switch(option.units)  
                                {
                                  case XSW_UNITS_ENGLISH:
                                    d = ConvertRUToAU(&u, d);
				    units_name_ptr = "au";
				    break;

				  case XSW_UNITS_METRIC:
				    d = ConvertRUToAU(&u, d);
                                    units_name_ptr = "au";
                                    break;

                                  default:
				    units_name_ptr = "ru";
				    break;
				}

		if(d > 1000)
                    sprintf(text, "Brg: %.0f'  Dist: %.0f %s",
                        r, d, units_name_ptr);
                else if(d > 100)
                    sprintf(text, "Brg: %.0f'  Dist: %.1f %s",
                        r, d, units_name_ptr);
                else if(d > 10)
                    sprintf(text, "Brg: %.1f'  Dist: %.2f %s",
                        r, d, units_name_ptr);
                else
                    sprintf(text, "Brg: %.2f'  Dist: %.4f %s",
                        r, d, units_name_ptr);


                                OSWDrawString(pixmap, 5, 28, text);
			    }
			}
		    }
		    else
		    {
			/* Set color depending on type. */
		        switch(obj_ptr->type)
		        {
                          case XSW_OBJ_TYPE_AREA:
                            OSWSetFgPix(xsw_color.scmark_area);
			    break;

			  case XSW_OBJ_TYPE_HOME:
			    OSWSetFgPix(xsw_color.scmark_home);
			    break;

			  default:
			    OSWSetFgPix(xsw_color.scmark_unknown);
			    break;
		        }
		    }
                    /* Area type objects? */
                    if(obj_ptr->type == XSW_OBJ_TYPE_AREA)
                    {
			int radius;	/* In pixels. */

			radius = static_cast<int>((double)MAX(obj_ptr->size, 0) /
			    1000 * zoom);

		        /* Draw cross to represent center of area object. */
			OSWDrawLine(
			    pixmap,
			    x - (obj_marker_dim / 2), y,
			    x + (obj_marker_dim / 2), y
			);
			OSWDrawLine(
			    pixmap,
			    x, y - (obj_marker_dim / 2),
			    x, y + (obj_marker_dim / 2)
			);

			/* Draw circle represending size of area object. */
			OSWDrawArc(
			    pixmap,
			    x - radius, y - radius,
			    2 * radius, 2 * radius,
			    0, 2 * PI
			);
		    }
		    else
		    {
		        /* Draw circle to represent object. */
                        OSWDrawArc(
                            pixmap,
                            x - (obj_marker_dim / 2),
			    y - (obj_marker_dim / 2),
                            obj_marker_dim, obj_marker_dim,
                            0, 2 * PI
                        );
		    }

		    /* Name. */
		    if((*obj_ptr->name != '\0') &&
		        draw_object_labels
		    )
		    {
		        int len_name, len_empire;
		        char name[XSW_OBJ_NAME_MAX + XSW_OBJ_EMPIRE_MAX + 80];

			len_name = strlen(obj_ptr->name);
			len_empire = strlen(obj_ptr->empire);

			if((len_name + len_empire) <
                           (XSW_OBJ_NAME_MAX + XSW_OBJ_EMPIRE_MAX + 80)
		        )
			    sprintf(name,
			        "(%s) %s",
			        obj_ptr->empire, obj_ptr->name
		            );
		        else
			    strncpy(name, obj_ptr->name, XSW_OBJ_NAME_MAX);
			name[XSW_OBJ_NAME_MAX + XSW_OBJ_EMPIRE_MAX + 80 - 1] = '\0';

			OSWDrawString(
			    pixmap,
			    x - 5,
			    y - 10,
			    name
                        );
		    }
		}

	    }

	    /* Draw measuring line. */
	    if(cht->measuring)
	    {
		double d, r;
		sw_units_struct u;
		char *units_name_ptr;
		char text[256];

		/* Calculate distance in XSW Real units. */
		d = hypot(
		    cht->measure_end_x - cht->measure_start_x,
		    cht->measure_end_y - cht->measure_start_y
		) / zoom;

		r = MuCoordinateDeltaVector(
		    cht->measure_end_x - cht->measure_start_x,
		    cht->measure_start_y - cht->measure_end_y
                );
		r = RADTODEG(r);

		/* Convert units. */
		u.ru_to_au = cht->header.ru_to_au;
                switch(option.units)
                {
                  case XSW_UNITS_ENGLISH:
                    d = ConvertRUToAU(&u, d);
		    units_name_ptr = "au";
                    break;

                  case XSW_UNITS_METRIC:
                    d = ConvertRUToAU(&u, d);
                    units_name_ptr = "au";
                    break;

                  default:
                    units_name_ptr = "ru";
		    break;
		}

		if(d > 1000)
		    sprintf(text, "Brg: %.0f'  Dist: %.0f %s",
			r, d, units_name_ptr);
                else if(d > 100)
                    sprintf(text, "Brg: %.0f'  Dist: %.1f %s",
			r, d, units_name_ptr);
                else if(d > 10)
                    sprintf(text, "Brg: %.1f'  Dist: %.2f %s",
			r, d, units_name_ptr);
                else
                    sprintf(text, "Brg: %.2f'  Dist: %.4f %s",
			r, d, units_name_ptr);

                OSWSetFgPix(xsw_color.chart_cross_hairs);
		OSWDrawString(
                    pixmap,
                    5,
		    (int)wattr.height - 8,
		    text
                );
		OSWDrawLine(
		    pixmap,
		    cht->measure_start_x, cht->measure_start_y,
		    cht->measure_end_x, cht->measure_end_y
		);
	    }

            /* Draw zoom value label. */
            if(view_zoom_state && (cht == last_cht))
	    {
                char text[256];

		sprintf(text, "Zoom: %.0f%%",
		    cht->zoom * 100.0
		);
                OSWSetFgPix(xsw_color.chart_cross_hairs);
                OSWDrawString(
                    pixmap,
                    (int)wattr.width - (6 * 14),
                    (int)wattr.height - 8,
                    text
                );
	    }

	    /* Draw frame. */
            WidgetFrameButtonPixmap(
                pixmap,
                True,
                wattr.width, wattr.height,
                widget_global.surface_highlight_pix,
                widget_global.surface_shadow_pix
            );

	    OSWPutBufferToWindow(w, pixmap);

	    OSWSetFont(prev_font);
	}
}

/*
 *	Manages the starchart window.
 */
int SChtManage(starchart_win_struct *cht, event_t *event)
{
	win_t w;
        keycode_t keycode;
        static int last_x, last_y;
	int events_handled = 0;


	if((cht == NULL) ||
           (event == NULL)
	)
	    return(events_handled);

        if(!cht->map_state &&
           (event->type != MapNotify)
        )
            return(events_handled);



        switch(event->type)
        {
          /* ******************************************************** */
          case KeyPress:
            /* Do not continue further if now in focus. */
            if(!cht->is_in_focus)
                return(events_handled);

            keycode = event->xkey.keycode;


            /* Unmap. */
            if(keycode == osw_keycode.esc)
            {
                SChtUnmap(cht);
		events_handled++;
            }
	    /* Enter key (does nothing). */
	    else if((keycode == osw_keycode.enter) ||
                    (keycode == osw_keycode.np_enter)
	    )
	    {
		events_handled++;
	    }
	    if(events_handled > 0)
		return(events_handled);

	    /* If any of the prompts are in focus, do hot handle
	     * these key presses.
	     */
	    if(cht->filter_prompt.is_in_focus ||
               cht->x_prompt.is_in_focus ||
               cht->y_prompt.is_in_focus ||
               cht->sect_x_prompt.is_in_focus ||
               cht->sect_y_prompt.is_in_focus
            )
		break;

	    /* Zoom in. */
	    if(keycode == osw_keycode.equal)
	    {
		SChtZoomInBtnCB(cht);
		events_handled++;
	    }
	    /* Zoom out. */
	    else if(keycode == osw_keycode.minus)
	    {
		SChtZoomOutBtnCB(cht);
		events_handled++;
	    }
	    if(events_handled > 0)
		return(events_handled);
	    break;

          /* ******************************************************** */
          case KeyRelease:
            /* Do not continue further if now in focus. */
            if(!cht->is_in_focus)
                return(events_handled);
                 
            keycode = event->xkey.keycode;

	    break;

          /* ******************************************************** */
          case ButtonPress:
	    w = event->xany.window;

            /* View. */   
            if(event->xany.window == cht->view)
            {
		int selected_object;

		/* Any button press on the view window unmaps the
		 * quick menu.
		 */
                if(cht->qmenu.map_state)
                    MenuUnmap(&cht->qmenu);

		/* Handle by button number. */
		switch(event->xbutton.button)
		{
		  case Button1:	/* Select/Translate. */
		    selected_object = SChtMatchObjectPosition(
			cht,
			event->xbutton.x, event->xbutton.y
		    );
		    if(selected_object < 0)
		    {
                        view_trans_state = True;
		        WidgetSetWindowCursor(w, xsw_cursor.translate);
		    }
		    else
		    {
			cht->selected_object = selected_object;
		    }
		    break;

		  case Button2:	/* Measure. */
                    cht->measuring = True;
                    cht->measure_start_x = event->xbutton.x;
                    cht->measure_end_x = event->xbutton.x;
                    cht->measure_start_y = event->xbutton.y;
                    cht->measure_end_y = event->xbutton.y;
		    break;

		  case Button3:	/* Zoom. */
                    selected_object = SChtMatchObjectPosition(
                        cht,
                        event->xbutton.x, event->xbutton.y
                    );
                    if(selected_object < 0)
                    {
                        view_zoom_state = True;
                        WidgetSetWindowCursor(w, xsw_cursor.zoom);
		    }
		    else
		    {
			int x, y;


			cht->selected_object = selected_object;

			/* Map quick menu at specified position. */
                        OSWGetPointerCoords(
                            osw_gui[0].root_win,
                            &x, &y,
                            NULL, NULL
                        );
                        MenuMapPos(&cht->qmenu, x, y);
		    }

		    break;
		}

                last_x = event->xbutton.x;
                last_y = event->xbutton.y;
                last_cht = cht;

		SChtDraw(cht, SCHT_DRAW_AMOUNT_VIEW);

		events_handled++;
                return(events_handled);
	    }
	    break;

          /* ******************************************************** */
          case ButtonRelease:
	    w = event->xany.window;

            /* View. */
            if(w == cht->view)
            {
		switch(event->xbutton.button)
                {
		  case Button1:	/* Translate. */
                    view_trans_state = False;
		    break;

		  case Button2:	/* Measure. */
		    cht->measuring = False;
		    break;

		  case Button3:	/* Zoom. */
                    view_zoom_state = False;
		    break;
		}

		OSWUnsetWindowCursor(w);
		SChtDraw(cht, SCHT_DRAW_AMOUNT_VIEW);

                events_handled++;
                return(events_handled);
 	    }
	    break;

          /* ******************************************************** */
          case MotionNotify:
	    w = event->xany.window;

            /* View. */
            if(w == cht->view)
            {
		int i, x, y;
		double cur_x, cur_y;
		long cur_sect_x, cur_sect_y;
		double sect_width, sect_height;


                x = event->xmotion.x;
                y = event->xmotion.y;

		sect_width = sector_legend.x_len;
		sect_height = sector_legend.y_len;

                /* Translate. */
                if(view_trans_state &&
                   (cht == last_cht)
                )
		{
                    /* Move coordinates. */
		    cur_x = PromptGetF(&cht->x_prompt);
                    cur_y = PromptGetF(&cht->y_prompt);
		    cur_sect_x = PromptGetL(&cht->sect_x_prompt);
                    cur_sect_y = PromptGetL(&cht->sect_y_prompt);

                    cur_x -= ((x - last_x) / cht->zoom);
                    cur_y += ((y - last_y) / cht->zoom);


                    /* Change to sectors. */  
                    for(i = 0; cur_x > SECTOR_SIZE_X_MAX; i++)
                    {
                        cur_x -= sect_width;
                    }
                    cur_sect_x += i;
                    for(i = 0; cur_x < SECTOR_SIZE_X_MIN; i++)
                    {
                        cur_x += sect_width;
                    }
                    cur_sect_x -= i;

                    for(i = 0; cur_y > SECTOR_SIZE_Y_MAX; i++)
                    {
                        cur_y -= sect_height; 
                    }
                    cur_sect_y += i;
                    for(i = 0; cur_y < SECTOR_SIZE_Y_MIN; i++)
                    {
                        cur_y += sect_height;
                    }
                    cur_sect_y -= i;

                    PromptSetF(&cht->x_prompt, cur_x);
                    PromptSetF(&cht->y_prompt, cur_y);
                    PromptSetI(&cht->sect_x_prompt, cur_sect_x);
                    PromptSetI(&cht->sect_y_prompt, cur_sect_y);

                    events_handled++;
                }

		/* Zoom. */
                if(view_zoom_state &&
                   (cht == last_cht)
                )
                {
                    cht->zoom += ((y - last_y) * 0.0008);

                    if(cht->zoom > SCHT_VIEW_ZOOM_MAX)
                        cht->zoom = SCHT_VIEW_ZOOM_MAX;
                    if(cht->zoom < SCHT_VIEW_ZOOM_MIN)  
                        cht->zoom = SCHT_VIEW_ZOOM_MIN;
                        
                    events_handled++;
                }

                /* Measuring. */
                if(cht->measuring &&
                   (cht == last_cht)  
                )
                {
                    cht->measure_end_x = x;
		    cht->measure_end_y = y;

		    events_handled++;
		}

                /* Update last positions. */
                last_x = x;
                last_y = y; 

                /* Redraw as needed. */
                if(events_handled > 0)
                    SChtDraw(cht, SCHT_DRAW_AMOUNT_VIEW);

                return(events_handled);
	    }
	    break;

	  /* ********************************************************* */
          case Expose:
	    w = event->xany.window;

            if(w == cht->toplevel)
            {
		SChtDraw(cht, SCHT_DRAW_AMOUNT_COMPLETE);
		events_handled++;
	    }
            else if(w == cht->view)
            {
		SChtDraw(cht, SCHT_DRAW_AMOUNT_VIEW);
                events_handled++;
            }
	    if(events_handled > 0)
		return(events_handled);

	    break;

          /* ********************************************************* */
          case VisibilityNotify:
            w = event->xany.window;

            if(w == cht->toplevel)
            {
                cht->visibility_state = event->xvisibility.state;
                events_handled++;
                return(events_handled);
            }
            break;

          /* ********************************************************* */
          case ConfigureNotify:
            w = event->xany.window;

            if(w == cht->toplevel)
            {
                SChtResize(cht);
		SChtDraw(cht, SCHT_DRAW_AMOUNT_COMPLETE);
                events_handled++;
            }
            break;

          /* ********************************************************* */
          case FocusOut:
            w = event->xany.window;

            if(w == cht->toplevel)
            {
                cht->is_in_focus = 0;
                events_handled++;
            }
            break;

          /* ********************************************************* */
          case FocusIn:
            w = event->xany.window;

            if(w == cht->toplevel)
            {
                cht->is_in_focus = 1;
                events_handled++;
            }
            break;

          /* **************************************************** */
          case ClientMessage:
            if(OSWIsEventDestroyWindow(cht->toplevel, event))
            {
                SChtUnmap(cht);

                events_handled++;
                return(events_handled);
            }
            break;
	}


        if(events_handled <= 0)
        {
            events_handled += PromptManage(&cht->filter_prompt, event);
	    if((events_handled > 0) && (event->type == KeyPress))
		next_filter_update = cur_millitime + SCHT_FILTER_UPDATE_INT;
        }

        if(events_handled <= 0)
	{
            events_handled += PromptManage(&cht->x_prompt, event);
	    if((events_handled > 0) && (event->type == KeyPress))
		SChtDraw(cht, SCHT_DRAW_AMOUNT_VIEW);
	}
        if(events_handled <= 0)
	{
            events_handled += PromptManage(&cht->y_prompt, event);
            if((events_handled > 0) && (event->type == KeyPress))
                SChtDraw(cht, SCHT_DRAW_AMOUNT_VIEW);
	}
/*
        if(events_handled <= 0)
            events_handled += PromptManage(&cht->z_prompt, event);
 */
        if(events_handled <= 0)
	{
            events_handled += PromptManage(&cht->sect_x_prompt, event);
            if((events_handled > 0) && (event->type == KeyPress))
                SChtDraw(cht, SCHT_DRAW_AMOUNT_VIEW);
	}
        if(events_handled <= 0)
	{
            events_handled += PromptManage(&cht->sect_y_prompt, event);
            if((events_handled > 0) && (event->type == KeyPress))
                SChtDraw(cht, SCHT_DRAW_AMOUNT_VIEW);
	}
/*      
        if(events_handled <= 0)
            events_handled += PromptManage(&cht->sect_z_prompt, event);
 */
        if(events_handled <= 0)
	{
            events_handled += TgBtnManage(&cht->follow_player_tb, event);
	    if(events_handled > 0)
		MenuBarSetItemToggleState(
		    &cht->mb,
		    SCHT_MENU_ITEM_POS_FOLLOW_PLAYER,
		    cht->follow_player_tb.state
		);
	}

        if(events_handled <= 0)
            events_handled += PBtnManage(&cht->zoom_in_btn, event);
        if(events_handled <= 0)
            events_handled += PBtnManage(&cht->zoom_out_btn, event);
        if(events_handled <= 0)
            events_handled += PBtnManage(&cht->jump_to_player_btn, event);

        if(events_handled <= 0)
            events_handled += MenuBarManage(&cht->mb, event);

        if(events_handled == 0)
        {
            if(cht->qmenu.map_state)     
                events_handled += MenuManage(
                    &cht->qmenu,
                    event
                );
        }

	if(events_handled == 0)
	    events_handled += PBarManage(&cht->pbar, event);


	return(events_handled);
}

/*
 *	Maps the starchart window.
 */
void SChtMap(starchart_win_struct *cht)
{
	if(cht == NULL)
	    return;

        XSWDoUnfocusAllWindows();

	cht->map_state = 0;
	SChtDraw(cht, SCHT_DRAW_AMOUNT_COMPLETE);
	cht->is_in_focus = 1;

        XSWDoRestackWindows();

	return;
}

/*
 *	Unmaps the starchart window.
 */
void SChtUnmap(starchart_win_struct *cht)
{
        if(cht == NULL)
            return;

        TgBtnUnmap(&cht->follow_player_tb);
	PBarUnmap(&cht->pbar);

	PromptUnmap(&cht->filter_prompt);

	PromptUnmap(&cht->x_prompt);
        PromptUnmap(&cht->y_prompt);
/*	PromptUnmap(&cht->z_prompt); */
        PromptUnmap(&cht->sect_x_prompt);
        PromptUnmap(&cht->sect_y_prompt);
/*	PromptUnmap(&cht->sect_z_prompt); */

        PBtnUnmap(&cht->zoom_in_btn);
        PBtnUnmap(&cht->zoom_out_btn);
        PBtnUnmap(&cht->jump_to_player_btn);

        MenuUnmap(&cht->qmenu);

        OSWUnmapWindow(cht->view);
        OSWDestroyPixmap(&cht->view_buf);

	MenuBarUnmap(&cht->mb);

        OSWUnmapWindow(cht->toplevel);

	return;
}

/*
 *	Destroys the starchart window.
 */
void SChtDestroy(starchart_win_struct *cht)
{
	if(cht == NULL)
	    return;

	if(IDC())
	{
	    TgBtnDestroy(&cht->follow_player_tb);

	    PBarDestroy(&cht->pbar);

            PromptDestroy(&cht->filter_prompt);

	    PromptDestroy(&cht->x_prompt);
            PromptDestroy(&cht->y_prompt);
/*	    PromptDestroy(&cht->z_prompt); */
            PromptDestroy(&cht->sect_x_prompt);
            PromptDestroy(&cht->sect_y_prompt);
/*	    PromptDestroy(&cht->sect_z_prompt); */

            PBtnDestroy(&cht->zoom_in_btn);
            PBtnDestroy(&cht->zoom_out_btn);
            PBtnDestroy(&cht->jump_to_player_btn);

	    MenuDestroy(&cht->qmenu);

            OSWDestroyWindow(&cht->view);
            OSWDestroyPixmap(&cht->view_buf);

	    MenuBarDestroy(&cht->mb);

	    OSWDestroyWindow(&cht->toplevel);
	}


	/* Delete all objects in chart. */
        UNVDeleteAllObjects(cht->object, cht->total_objects);
	cht->object = NULL;
	cht->total_objects = 0;
	cht->selected_object = -1;

	/* Reset values. */
	cht->map_state = 0;
        cht->x = 0;
        cht->y = 0;
        cht->width = 0;
        cht->height = 0;
        cht->is_in_focus = 0;
        cht->visibility_state = VisibilityFullyObscured;
        cht->disabled = False;

	cht->measuring = False;
	cht->measure_start_x = 0;
        cht->measure_start_y = 0;
        cht->measure_end_x = 0;
        cht->measure_end_y = 0;

	return;
}
