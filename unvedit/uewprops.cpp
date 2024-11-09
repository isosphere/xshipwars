// unvedit/uewprops.cpp
/*
              Universe Editor: Properties List Management

	Functions:

	void UEWPropsDoGetValues(uew_struct *uew_ptr, int obj_num)
	void UEWPropsDoSetValues(uew_struct *uew_ptr, int obj_num)

	int UEWPropsInit(uew_struct *uew_ptr)
	void UEWPropsResize(uew_struct *uew_ptr)
	void UEWPropsDraw(uew_struct *uew_ptr, int direction)
	int UEWPropsManage(uew_struct *uew_ptr, event_t *event)
	void UEWPropsDestroy(uew_struct *uew_ptr)

	---

	Only the prompts are managed.
	Events to the scroll bar are managed but scroll bar
	is not drawn.

 */


#include "../include/osw-x.h"
#include "../include/widget.h"
#include "../include/objects.h"

#include "ue.h"
#include "uew.h"
#include "uewprops.h"

#ifndef MAX
#define MIN(a,b)        (((a) < (b)) ? (a) : (b))
#define MAX(a,b)	(((a) > (b)) ? (a) : (b))
#endif
/* #define MIN(a,b)        ((a) < (b) ? (a) : (b)) */
/* #define MAX(a,b)        ((a) > (b) ? (a) : (b)) */



/*
 *	Procedure to get values from object and set them
 *	to property prompts.  If the object is invalid,
 *	the property prompts will be reset.
 *
 *	Prompts will be redrawn.
 */
void UEWPropsDoGetValues(uew_struct *uew_ptr, int obj_num)
{
        xsw_object_struct *obj_ptr;
	prompt_window_struct *prompt;
	int i;
        

	if(uew_ptr == NULL)
	    return;


        if(UEWIsObjectGarbage(uew_ptr, obj_num))
            obj_ptr = NULL;
        else
            obj_ptr = uew_ptr->object[obj_num];


	if(obj_ptr == NULL)
	{
	    /* Clear prompt values. */
            for(i = 0; i < (int)TOTAL_PROP_PROMPTS; i++)
            {
		prompt = &uew_ptr->prop_prompt[i];
		if(prompt->buf != NULL)
		{
		    prompt->buf[0] = '\0';
		    PromptUnmarkBuffer(prompt, PROMPT_POS_START);

	            if(prompt->map_state)
                        PromptDraw(prompt, PROMPT_DRAW_AMOUNT_TEXTONLY);
		}
            }
	}
	else
	{
	    /* Get values from object and put them into prompts. */
	    for(i = 0; i < (int)TOTAL_PROP_PROMPTS; i++)
	    {
	        UEWPropsGet(&uew_ptr->prop_prompt[i], obj_ptr);
	    }
	}
}


/*
 *	Sets the values from the prompts to the object if
 *	it is valid.
 */
void UEWPropsDoSetValues(uew_struct *uew_ptr, int obj_num)
{
	xsw_object_struct *obj_ptr;
        int i;


        if(uew_ptr == NULL)
            return;


	if(UEWIsObjectGarbage(uew_ptr, obj_num))
	    return;
	else
	    obj_ptr = uew_ptr->object[obj_num];



        /* Get values from object and put them into prompts. */
        for(i = 0; i < (int)TOTAL_PROP_PROMPTS; i++)
            UEWPropsSet(&uew_ptr->prop_prompt[i], obj_ptr);

	/* Need to update name of item on objects colum list. */
        UEWUpdateObjectListItemName(
            uew_ptr,
            obj_num
        );
        CListDraw(&uew_ptr->objects_list, CL_DRAW_AMOUNT_LIST);
}



/*
 *	Allocates all property prompts on uew.
 */
int UEWPropsInit(uew_struct *uew_ptr)
{
	int i, x, y, status;
	win_t parent;
	win_attr_t wattr;
	unsigned int width, height;
	const char *prop_name[] = PROP_NAME;


        if(uew_ptr == NULL)
            return(-1);

        parent = uew_ptr->props;
	if(parent == 0)
	    return(-1);

	OSWGetWindowAttributes(parent, &wattr);

	x = 0;
	y = 0;
	width = MAX(wattr.width - SCROLLBAR_YBAR_WIDTH, 10);
	height = UEW_PROP_PROMPT_HEIGHT;

        for(i = 0; i < (int)TOTAL_PROP_PROMPTS; i++)
        {
            status = PromptInit(
		&uew_ptr->prop_prompt[i],
		parent,
                x, y,
                width, height,
                PROMPT_STYLE_FLUSHED,
                prop_name[i],
		XSW_OBJ_NAME_MAX,
                0,
		NULL
	    );
	    if(status)
		return(-1);
	}

	return(0);
}

/*
 *	Reszes all property prompts.
 */
void UEWPropsResize(uew_struct *uew_ptr)
{
	int i;
	unsigned int width, height;
	win_t parent;
	win_attr_t wattr;


	if(uew_ptr == NULL)
	    return;


        parent = uew_ptr->props;
        if(parent == 0)
            return;

        OSWGetWindowAttributes(parent, &wattr);

        width = MAX(wattr.width - SCROLLBAR_YBAR_WIDTH, 10);
	height = UEW_PROP_PROMPT_HEIGHT;

        for(i = 0; i < (int)TOTAL_PROP_PROMPTS; i++)
        {
	    OSWResizeWindow(
		uew_ptr->prop_prompt[i].toplevel,
		width, height
	    );

	    /* This will cause resources to resize. */
	    if(uew_ptr->prop_prompt[i].map_state)
                PromptMap(
                    &uew_ptr->prop_prompt[i]
                );
        }

	uew_ptr->props_sb.x_win_pos = 0;
	uew_ptr->props_sb.y_win_pos = 0;
}


/*
 *	`Redraws' the property prompts by moving them and
 *	unmapping/mapping them.
 *
 *	direction is a hint towards which direction the
 *	props list was scrolled to just before calling this function:
 *	0 = down, 1 = up.
 *
 *	If unsure, just pass 0 (for down).
 */
void UEWPropsDraw(uew_struct *uew_ptr, int direction)
{
        int i, x, y;
	int f_vis_prompt, l_vis_prompt;
        win_t parent;
        win_attr_t wattr;


        if(uew_ptr == NULL)
            return;

        parent = uew_ptr->props;
        if(parent == 0)
            return;

	OSWGetWindowAttributes(parent, &wattr);


	/* Calculate scrolled positions. */
	f_vis_prompt = MIN(
	    uew_ptr->props_sb.y_win_pos /
	    UEW_PROP_PROMPT_HEIGHT,
	    (int)TOTAL_PROP_PROMPTS - 1
	);
	if(f_vis_prompt < 0)
	    f_vis_prompt = 0;

	l_vis_prompt = MIN(
	    f_vis_prompt + (wattr.height / UEW_PROP_PROMPT_HEIGHT)
		+ 3,
	    (int)TOTAL_PROP_PROMPTS	/* Limit to total, not index. */
	);
	if(l_vis_prompt < 0)
	    l_vis_prompt = 0;


	x = 0;
	y = 0 - (uew_ptr->props_sb.y_win_pos %
	    UEW_PROP_PROMPT_HEIGHT);


        for(i = 0; i < f_vis_prompt; i++)
        {
/*
	    OSWMoveWindow(   
                uew_ptr->prop_prompt[i].toplevel,
                x, 0 - 10 - UEW_PROP_PROMPT_HEIGHT
            );

	    if(uew_ptr->prop_prompt[i].map_state)
 */
		PromptUnmap(&uew_ptr->prop_prompt[i]);
	}

	if(direction)
        {
            /* Scrolled up. */
	    y = y + ((l_vis_prompt - f_vis_prompt) *
		UEW_PROP_PROMPT_HEIGHT) - UEW_PROP_PROMPT_HEIGHT;

            for(i = l_vis_prompt - 1; i >= f_vis_prompt; i--)
            {
                OSWMoveWindow(
                    uew_ptr->prop_prompt[i].toplevel,
                    x, y
                );
                if(!uew_ptr->prop_prompt[i].map_state)  
                    PromptMap(&uew_ptr->prop_prompt[i]);
                
                y -= UEW_PROP_PROMPT_HEIGHT;
            }
        }
	else
	{
	    /* Scrolled down. */
            for(i = f_vis_prompt; i < l_vis_prompt; i++)
            {
	        OSWMoveWindow(
		    uew_ptr->prop_prompt[i].toplevel,
		    x, y
	        );
                if(!uew_ptr->prop_prompt[i].map_state)
                    PromptMap(&uew_ptr->prop_prompt[i]);

	        y += UEW_PROP_PROMPT_HEIGHT;
            }
	}

        for(i = l_vis_prompt; i < UEW_PROP_PROMPT_HEIGHT; i++)
        {
/*
            OSWMoveWindow(
                uew_ptr->prop_prompt[i].toplevel,
                x, 0 - 10 - UEW_PROP_PROMPT_HEIGHT
            );

            if(uew_ptr->prop_prompt[i].map_state)
 */
                PromptUnmap(&uew_ptr->prop_prompt[i]);
        }


	return;
}



/*
 *	Manages all property prompts, plus property window scroll bar.
 */
int UEWPropsManage(uew_struct *uew_ptr, event_t *event)
{
	int i, y;
	int events_handled = 0;
	win_attr_t wattr;


	if((uew_ptr == NULL) ||
           (event == NULL)
	)
	    return(events_handled);


        for(i = 0; i < (int)TOTAL_PROP_PROMPTS; i++)
            events_handled += PromptManage(
                &uew_ptr->prop_prompt[i],
                event
            );

	if(events_handled == 0)
	{
	    /* Record previous y position. */
	    y = uew_ptr->props_sb.y_win_pos;

	    OSWGetWindowAttributes(uew_ptr->props, &wattr);
            events_handled += SBarManage(
                &uew_ptr->props_sb,
                wattr.width,
                wattr.height,
                wattr.width,
                TOTAL_PROP_PROMPTS * UEW_PROP_PROMPT_HEIGHT,
	        event
            );

	    if(events_handled > 0)
	    {
		if(y < uew_ptr->props_sb.y_win_pos)
		    UEWPropsDraw(uew_ptr, 0);
		else
		    UEWPropsDraw(uew_ptr, 1);
	    }
	}


	return(events_handled);
}


/*
 *	Destroys all property prompts on uew.
 */
void UEWPropsDestroy(uew_struct *uew_ptr)
{
	int i;


	if(uew_ptr == NULL)
	    return;

	for(i = 0; i < (int)TOTAL_PROP_PROMPTS; i++)
	    PromptDestroy(&uew_ptr->prop_prompt[i]);
}




