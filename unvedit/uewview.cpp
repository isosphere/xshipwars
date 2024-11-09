// unvedit/uewview.cpp
/*
              Universe Editor Window: View Window Management

	Functions:

	int UEWViewSelectObject(int n, int x, int y)
	void UEWDoSetViewStatusObjectValues(int n, xsw_object_struct *obj_ptr)
	void UEWViewDraw(int n)

	---

 */
/*
#include <stdio.h>
#include <malloc.h>
#include <db.h>
*/
#include <math.h>

#include "../include/graphics.h"
#include "../include/os.h"
#include "../include/osw-x.h"
#include "../include/widget.h"
#include "../include/reality.h"
#include "../include/objects.h"

#include "blitting.h"
#include "uew.h"
#include "ue.h"

#ifndef MAX
#define MIN(a,b)        (((a) < (b)) ? (a) : (b))
#define MAX(a,b)	(((a) > (b)) ? (a) : (b))
#endif
/* #define MIN(a,b)	((a) < (b) ? (a) : (b)) */
/* #define MAX(a,b)	((a) > (b) ? (a) : (b)) */


/*
 *	Matches object on uew, only returns match.
 *	Does not actually select the object.
 */
int UEWViewSelectObject(int n, int x, int y)
{
	int i, obj_x = 0, obj_y = 0, sw, sh, swv, shv;
        long sect_x_min, sect_x_max, sect_y_min, sect_y_max;
	long d_sect_x, d_sect_y;
        uew_struct *uew_ptr;
	shared_image_t *img;
        xsw_object_struct **obj_ptr;


        if(UEWIsAllocated(n))
            uew_ptr = uew[n];
        else
            return(-1);

	img = uew_ptr->view_img;
	if(img == NULL)
	    return(-1);


        /* Sanitize zoom. */
        if(uew_ptr->zoom < UEW_VIEW_ZOOM_MIN)
            uew_ptr->zoom = UEW_VIEW_ZOOM_MIN;
        if(uew_ptr->zoom > UEW_VIEW_ZOOM_MAX)
            uew_ptr->zoom = UEW_VIEW_ZOOM_MAX;


        /* Zoomed width of a sector. */
        sw = MAX((int)(uew_ptr->sect_width * uew_ptr->zoom), 1);
            
        /* Sectors visable on width. */
        if(sw != 0)
            swv = ((int)img->width / sw);
        else
            swv = 0;

        /* Zoomed height of a sector. */
        sh = MAX((int)(uew_ptr->sect_height * uew_ptr->zoom), 1);
         
        /* Sectors visable on height. */
        if(sh != 0)
            shv = ((int)img->height / sh);
        else
            shv = 0;

        /*   Calculate sector bounds and add `good measure'.
         *   Note that swv and shv are already calculated.
         */ 
        sect_x_min = uew_ptr->sect_x - (long)swv - 1;
        sect_x_max = uew_ptr->sect_x + (long)swv + 1;
        sect_y_min = uew_ptr->sect_y - (long)shv - 1;
        sect_y_max = uew_ptr->sect_y + (long)shv + 1;


	/* Go through objects on uew. */
	for(i = 0, obj_ptr = uew_ptr->object;
            i < uew_ptr->total_objects;
            i++, obj_ptr++
	)
	{
	    if(*obj_ptr == NULL)
		continue;

            /* Object is garbage? */
            if((*obj_ptr)->type <= XSW_OBJ_TYPE_GARBAGE)
                continue;

            /* Object in bounds? */
            if(((*obj_ptr)->sect_x < sect_x_min) ||
               ((*obj_ptr)->sect_x > sect_x_max) ||
               ((*obj_ptr)->sect_y < sect_y_min) ||
               ((*obj_ptr)->sect_y > sect_y_max)
            )
                continue;

            /* Calculate sector deltas. */
            d_sect_x = (*obj_ptr)->sect_x - uew_ptr->sect_x;
            d_sect_y = (*obj_ptr)->sect_y - uew_ptr->sect_y;

            /* Calculate X coordinate position in units of pixels. */
            if(d_sect_x == 0)
            {
                obj_x = static_cast<int>(((*obj_ptr)->x - uew_ptr->cx) * uew_ptr->zoom);
            }
            else if(d_sect_x < 0)
            {
                obj_x = static_cast<int>((((uew_ptr->sect_width / 2) - (*obj_ptr)->x) +
                        ((d_sect_x + 1) * uew_ptr->sect_width * -1) +
                        (uew_ptr->cx + (uew_ptr->sect_width / 2)))
                        * uew_ptr->zoom * -1);
            }
            else if(d_sect_x > 0)
            {
                obj_x = static_cast<int>((((uew_ptr->sect_width / 2) - uew_ptr->cx) +
                        ((d_sect_x - 1) * uew_ptr->sect_width) +
                        ((*obj_ptr)->x + (uew_ptr->sect_width / 2)))
                        * uew_ptr->zoom);
            }
            obj_x += ((int)img->width / 2);

            /* Calculate Y coordinate position in units of pixels. */
            if(d_sect_y == 0)
            {
                obj_y = static_cast<int>(((*obj_ptr)->y - uew_ptr->cy) * uew_ptr->zoom);
            }
            else if(d_sect_y < 0)
            {
                obj_y = static_cast<int>((((uew_ptr->sect_height / 2) - (*obj_ptr)->y) +
                        ((d_sect_y + 1) * uew_ptr->sect_height * -1) +
                        (uew_ptr->cy + (uew_ptr->sect_height / 2)))
                        * uew_ptr->zoom * -1);
            }
            else if(d_sect_y > 0)
            {
                obj_y = static_cast<int>((((uew_ptr->sect_height / 2) - uew_ptr->cy) +
                        ((d_sect_y - 1) * uew_ptr->sect_height) +
                        ((*obj_ptr)->y + (uew_ptr->sect_height / 2)))
                        * uew_ptr->zoom);
            }
            obj_y = (((int)img->height / 2) - obj_y);


	    /*   Check if x and y are within object's coordinates
	     *   in pixels.
	     */
	    if((x >= (obj_x - 4)) &&
               (x < (obj_x + 4)) &&
               (y >= (obj_y - 4)) &&
               (y < (obj_y + 4))
	    )
		return(i);
	}





	return(-1);
}


/*
 *	Sets prompt values on view status with values from obj_ptr;
 */
void UEWDoSetViewStatusObjectValues(int n, xsw_object_struct *obj_ptr)
{
	char *strptr;
	int obj_is_valid;
        uew_struct *uew_ptr;
	prompt_window_struct *prompt;


        if(UEWIsAllocated(n))
            uew_ptr = uew[n];
        else
            return;


	/* Skip if not mapped. */
	if(!uew_ptr->map_state)
	    return;


	/* Check if object is valid. */
	if(obj_ptr == NULL)
	    obj_is_valid = 0;
	else if(obj_ptr->type <= XSW_OBJ_TYPE_GARBAGE)
	    obj_is_valid = 0;
	else
	    obj_is_valid = 1;


	if(obj_is_valid)
	{
	    prompt = &uew_ptr->obj_cx_prompt;
	    strptr = prompt->buf;
	    if(strptr != NULL)
	        sprintf(strptr, "%.2f", obj_ptr->x);
	    PromptUnmarkBuffer(prompt, PROMPT_POS_END);
	    PromptDraw(prompt, PROMPT_DRAW_AMOUNT_TEXTONLY);

            prompt = &uew_ptr->obj_cy_prompt;
            strptr = prompt->buf;
            if(strptr != NULL)
                sprintf(strptr, "%.2f", obj_ptr->y);
            PromptUnmarkBuffer(prompt, PROMPT_POS_END);
            PromptDraw(prompt, PROMPT_DRAW_AMOUNT_TEXTONLY);

            prompt = &uew_ptr->obj_cz_prompt;
            strptr = prompt->buf;
            if(strptr != NULL)
                sprintf(strptr, "%.2f", obj_ptr->z);
            PromptUnmarkBuffer(prompt, PROMPT_POS_END);
            PromptDraw(prompt, PROMPT_DRAW_AMOUNT_TEXTONLY);


            prompt = &uew_ptr->obj_sect_x_prompt;
            strptr = prompt->buf;
            if(strptr != NULL)
                sprintf(strptr, "%ld", obj_ptr->sect_x);
            PromptUnmarkBuffer(prompt, PROMPT_POS_END);
            PromptDraw(prompt, PROMPT_DRAW_AMOUNT_TEXTONLY);

            prompt = &uew_ptr->obj_sect_y_prompt;
            strptr = prompt->buf;
            if(strptr != NULL)
                sprintf(strptr, "%ld", obj_ptr->sect_y);
            PromptUnmarkBuffer(prompt, PROMPT_POS_END);
            PromptDraw(prompt, PROMPT_DRAW_AMOUNT_TEXTONLY);

            prompt = &uew_ptr->obj_sect_z_prompt;
            strptr = prompt->buf;
            if(strptr != NULL)
                sprintf(strptr, "%ld", obj_ptr->sect_z);
            PromptUnmarkBuffer(prompt, PROMPT_POS_END);
            PromptDraw(prompt, PROMPT_DRAW_AMOUNT_TEXTONLY);


            prompt = &uew_ptr->obj_name_prompt;
            strptr = prompt->buf;
            if(strptr != NULL)
	    {
                strncpy(
		    strptr,
		    obj_ptr->name,
		    XSW_OBJ_NAME_MAX
		);
		strptr[XSW_OBJ_NAME_MAX - 1] = '\0';
	    }
            PromptUnmarkBuffer(prompt, PROMPT_POS_END);
            PromptDraw(prompt, PROMPT_DRAW_AMOUNT_TEXTONLY);

            prompt = &uew_ptr->obj_heading_prompt;
            strptr = prompt->buf;
            if(strptr != NULL)
                sprintf(strptr, "%.2f", obj_ptr->heading * (180 / PI));
            PromptUnmarkBuffer(prompt, PROMPT_POS_END);
            PromptDraw(prompt, PROMPT_DRAW_AMOUNT_TEXTONLY);

            prompt = &uew_ptr->obj_size_prompt;
            strptr = prompt->buf;
            if(strptr != NULL)
                sprintf(strptr, "%ld", obj_ptr->size);
            PromptUnmarkBuffer(prompt, PROMPT_POS_END);
            PromptDraw(prompt, PROMPT_DRAW_AMOUNT_TEXTONLY);
	}
	else
	{
            prompt = &uew_ptr->obj_cx_prompt;
            strptr = prompt->buf;
            if(strptr != NULL)   
                strptr[0] = '\0';
            PromptUnmarkBuffer(prompt, PROMPT_POS_END);
            PromptDraw(prompt, PROMPT_DRAW_AMOUNT_TEXTONLY);

            prompt = &uew_ptr->obj_cy_prompt;
            strptr = prompt->buf;
            if(strptr != NULL)
                strptr[0] = '\0';
            PromptUnmarkBuffer(prompt, PROMPT_POS_END);
            PromptDraw(prompt, PROMPT_DRAW_AMOUNT_TEXTONLY);

            prompt = &uew_ptr->obj_cz_prompt;
            strptr = prompt->buf;
            if(strptr != NULL)   
                strptr[0] = '\0';
            PromptUnmarkBuffer(prompt, PROMPT_POS_END);
            PromptDraw(prompt, PROMPT_DRAW_AMOUNT_TEXTONLY);


            prompt = &uew_ptr->obj_sect_x_prompt;
            strptr = prompt->buf;
            if(strptr != NULL)
                strptr[0] = '\0';
            PromptUnmarkBuffer(prompt, PROMPT_POS_END);
            PromptDraw(prompt, PROMPT_DRAW_AMOUNT_TEXTONLY);

            prompt = &uew_ptr->obj_sect_y_prompt;
            strptr = prompt->buf;
            if(strptr != NULL)
                strptr[0] = '\0';
            PromptUnmarkBuffer(prompt, PROMPT_POS_END);
            PromptDraw(prompt, PROMPT_DRAW_AMOUNT_TEXTONLY);

            prompt = &uew_ptr->obj_sect_z_prompt;
            strptr = prompt->buf;
            if(strptr != NULL)
                strptr[0] = '\0';
            PromptUnmarkBuffer(prompt, PROMPT_POS_END);
            PromptDraw(prompt, PROMPT_DRAW_AMOUNT_TEXTONLY);


            prompt = &uew_ptr->obj_name_prompt;
            strptr = prompt->buf;
            if(strptr != NULL)
                strptr[0] = '\0';
            PromptUnmarkBuffer(prompt, PROMPT_POS_END);
            PromptDraw(prompt, PROMPT_DRAW_AMOUNT_TEXTONLY);

            prompt = &uew_ptr->obj_heading_prompt;
            strptr = prompt->buf;
            if(strptr != NULL)
                strptr[0] = '\0';
            PromptUnmarkBuffer(prompt, PROMPT_POS_END);
            PromptDraw(prompt, PROMPT_DRAW_AMOUNT_TEXTONLY);

            prompt = &uew_ptr->obj_size_prompt;
            strptr = prompt->buf;
            if(strptr != NULL)
                strptr[0] = '\0';
            PromptUnmarkBuffer(prompt, PROMPT_POS_END);
            PromptDraw(prompt, PROMPT_DRAW_AMOUNT_TEXTONLY);
	}



	return;
}


/*
 *	Draws view window on uew n.
 */
void UEWViewDraw(int n)
{
        int x, y, i, sw, sh, swv, shv;
	int pos, inc;
	long sect_cur;
	long sect_x_min, sect_x_max, sect_y_min, sect_y_max;
	long d_sect_x, d_sect_y;
        uew_struct *uew_ptr;
	win_t w;
	shared_image_t *img;
	image_t *tmp_img;
	pixmap_t text_pm;
	pixel_t text_pix;
	WColorStruct color;
	xsw_object_struct **obj_ptr;
	char text[80];


        if(UEWIsAllocated(n))
            uew_ptr = uew[n];
        else
            return;


	w = uew_ptr->view;
	img = uew_ptr->view_img;

	if((w == 0) ||
           (img == NULL)
	)
	    return;


	/* Create tempory text pixmap. */
	if(OSWCreatePixmap(&text_pm, img->width, 30))
	    return;
	OSWLoadPixelRGB(&text_pix, 0xff, 0xb3, 0xfa);


	/* Sanitize zoom. */
	if(uew_ptr->zoom < UEW_VIEW_ZOOM_MIN)
	    uew_ptr->zoom = UEW_VIEW_ZOOM_MIN;
	if(uew_ptr->zoom > UEW_VIEW_ZOOM_MAX)
	    uew_ptr->zoom = UEW_VIEW_ZOOM_MAX;


	/* Clear background. */
	color.a = 0x00;
	color.r = 0x00;
	color.g = 0x00;
	color.b = 0x00;

	BlitBufSolid(
	    osw_gui[0].depth,
	    img->data,
	    img->width, img->height,
	    color
	);


        /* ********************************************************* */
	/* Draw grids (not sector grids). */

	inc = static_cast<int>(option.grid_spacing * uew_ptr->zoom);
	swv = ((int)img->width / inc);
        x = static_cast<int>((((option.grid_spacing / 2) - uew_ptr->cx) * uew_ptr->zoom) +
            ((int)img->width / 2));

        color.a = 0x00;
        color.r = 0x40;   
        color.g = 0x08;
        color.b = 0x34;

        if(inc >= 20)
        {
            for(pos = x % inc; pos < (int)img->width; pos += inc)
            {
                if(pos < 0)
                    continue;

                BlitBufLine(
                    osw_gui[0].depth,
                    img->data,
                    pos, img->height,
                    img->width, img->height,
                    0, img->height, 1,  
                    color
                );
            }
        }


        inc = static_cast<int>(option.grid_spacing * uew_ptr->zoom);
        shv = ((int)img->height / inc);
        y = static_cast<int>((((option.grid_spacing / 2) - uew_ptr->cy) * uew_ptr->zoom) +
            ((int)img->height / 2));

        color.a = 0x00;  
        color.r = 0x40;
        color.g = 0x08;
        color.b = 0x34;

        if(inc >= 20)
        {
            for(pos = ((int)img->height - y) % inc;
                pos < (int)img->height;
                pos += inc
	    )
            {
                if(pos < 0)
                    continue;

                BlitBufLine(
                    osw_gui[0].depth,
                    img->data,
                    0, pos,
                    img->width, img->height,
                    (PI / 2), img->width, 1,
                    color
                );
            }
        }


	/* ********************************************************* */
	/* Draw vertical sector grids. */

	/* Zoomed width of a sector. */
	sw = MAX((int)(uew_ptr->sect_width * uew_ptr->zoom), 1);

	/* Sectors visable on width. */
	if(sw != 0)
	    swv = ((int)img->width / sw);
	else
	    swv = 0;

	/* Position of starting grid line. */
	x = static_cast<int>((((uew_ptr->sect_width / 2) - uew_ptr->cx) * uew_ptr->zoom)
            + ((int)img->width / 2) - (swv * sw));

	sect_cur = uew_ptr->sect_x - swv + 1;

        color.a = 0x00;
        color.r = 0x80;
        color.g = 0x19;
        color.b = 0x78;

	for(pos = x, inc = sw; pos < (int)img->width; pos += inc, sect_cur++)
	{
	    if(pos < 0)
		continue;

	    BlitBufLine(   
                osw_gui[0].depth,
		img->data,
                pos, img->height,
		img->width, img->height,
		0, img->height, 1,
		color
	    );
	}


        /* ********************************************************* */
        /* Draw horizontal sector grids. */

        /* Zoomed height of a sector. */
        sh = MAX((int)(uew_ptr->sect_height * uew_ptr->zoom), 1);

        /* Sectors visable on height. */
        if(sh != 0)
            shv = ((int)img->height / sh);
        else
            shv = 0;
                
        /* Position of starting grid line. */
        y = static_cast<int>(((int)img->height -
            ((((uew_ptr->sect_height / 2) - uew_ptr->cy) * uew_ptr->zoom)
            + ((int)img->height / 2))) - (shv * sh));

        color.a = 0x00;
        color.r = 0x80;
        color.g = 0x19;
        color.b = 0x78;

        sect_cur = uew_ptr->sect_y + shv;

        for(pos = y, inc = sh; pos < (int)img->height; pos += inc, sect_cur--)
        {
            if(pos < 0)
                continue;

            BlitBufLine(
                osw_gui[0].depth,
                img->data,
                0, pos,
                img->width, img->height,
                (PI / 2), img->width, 1,
                color
            );
	}


        /* ********************************************************* */
        /* Draw objects. */

	/*   Calculate sector bounds and add `good measure'.
	 *   Note that swv and shv are already calculated.
	 */
	sect_x_min = uew_ptr->sect_x - (long)swv - 1;
	sect_x_max = uew_ptr->sect_x + (long)swv + 1;
        sect_y_min = uew_ptr->sect_y - (long)shv - 1;
        sect_y_max = uew_ptr->sect_y + (long)shv + 1;



	for(i = 0, obj_ptr = uew_ptr->object;
            i < uew_ptr->total_objects;
            i++, obj_ptr++
	)
	{
	    if(*obj_ptr == NULL)
		continue;

	    /* Object is garbage? */
	    if((*obj_ptr)->type <= XSW_OBJ_TYPE_GARBAGE)
		continue;

	    /* Object in bounds? */
	    if(((*obj_ptr)->sect_x < sect_x_min) ||
               ((*obj_ptr)->sect_x > sect_x_max) ||
               ((*obj_ptr)->sect_y < sect_y_min) ||
               ((*obj_ptr)->sect_y > sect_y_max)
	    )
		continue;

	    /* Calculate sector deltas. */
	    d_sect_x = (*obj_ptr)->sect_x - uew_ptr->sect_x;
            d_sect_y = (*obj_ptr)->sect_y - uew_ptr->sect_y;

	    /* Calculate X coordinate position. */
	    if(d_sect_x == 0)
	    {
	        x = static_cast<int>(((*obj_ptr)->x - uew_ptr->cx) * uew_ptr->zoom);
	    }
	    else if(d_sect_x < 0)
	    {
		x = static_cast<int>((((uew_ptr->sect_width / 2) - (*obj_ptr)->x) +
                    ((d_sect_x + 1) * uew_ptr->sect_width * -1) +
		    (uew_ptr->cx + (uew_ptr->sect_width / 2)))
		    * uew_ptr->zoom * -1);
	    }
            else if(d_sect_x > 0)
            {
		x = static_cast<int>((((uew_ptr->sect_width / 2) - uew_ptr->cx) +
                    ((d_sect_x - 1) * uew_ptr->sect_width) +
                    ((*obj_ptr)->x + (uew_ptr->sect_width / 2)))
                    * uew_ptr->zoom);
            }
	    x += ((int)img->width / 2);

            /* Calculate Y coordinate position. */
            if(d_sect_y == 0)
            {
                y = static_cast<int>(((*obj_ptr)->y - uew_ptr->cy) * uew_ptr->zoom);
            }
            else if(d_sect_y < 0)
            {
                y = static_cast<int>((((uew_ptr->sect_height / 2) - (*obj_ptr)->y) +
                    ((d_sect_y + 1) * uew_ptr->sect_height * -1) +
                    (uew_ptr->cy + (uew_ptr->sect_height / 2)))
                    * uew_ptr->zoom * -1);
            }
            else if(d_sect_y > 0)
            {
                y = static_cast<int>((((uew_ptr->sect_height / 2) - uew_ptr->cy) +
                    ((d_sect_y - 1) * uew_ptr->sect_height) +
                    ((*obj_ptr)->y + (uew_ptr->sect_height / 2)))
                    * uew_ptr->zoom);
            }   
            y = (((int)img->height / 2) - y);


	    /* Draw object marker. */
	    if(UEWIsObjectSelected(uew_ptr, i))
	    {
                color.a = 0x00;
                color.r = 0xff;
                color.g = 0xff;
                color.b = 0x80;
	    }
	    else
	    {
                color.a = 0x00;
                color.r = 0xff;
                color.g = 0x64;
                color.b = 0xf8;
	    }

	    BlitBufCircle(
		osw_gui[0].depth,
		img->data,
		x, y,
		img->width, img->height,
                1, 4, color
	    );

	    /* Area type objects? */
	    if((*obj_ptr)->type == XSW_OBJ_TYPE_AREA)
	    {
                color.a = 0x00;
                color.r = 0x80;
                color.g = 0x19;
                color.b = 0x78;
             
                BlitBufCircle(
                    osw_gui[0].depth,
                    img->data,
                    x, y,
                    img->width, img->height,
                    1,		/* Thickness. */
		    (*obj_ptr)->size / 1000 * uew_ptr->zoom,
		    color
                );
	    }
	}


 

        /* ********************************************************* */
        /* Draw cross. */

        color.a = 0x00;
        color.r = 0xf8;
        color.g = 0xb3;
        color.b = 0xfa;

        BlitBufLine(
            osw_gui[0].depth,
            img->data,
            ((int)img->width / 2) - 7,
            ((int)img->height / 2),
            img->width, img->height,
            (PI / 2) + 0.0001, 16, 1,
            color
        );
        BlitBufLine(
            osw_gui[0].depth,
            img->data,
            ((int)img->width / 2),
            ((int)img->height / 2) + 8,
            img->width, img->height,
            0, 16, 1,
            color
        );


        /* Draw coordinate text. */
	if(option.label_geometry)
	{
            sprintf(text,
	        "Sect: %ld, %ld  XY: %.2f, %.2f  Zoom: %.4f",
                uew_ptr->sect_x, uew_ptr->sect_y,
                uew_ptr->cx, uew_ptr->cy,
                uew_ptr->zoom
            );
            OSWClearPixmap(text_pm, img->width, 30, osw_gui[0].black_pix);
            OSWSetFgPix(text_pix);
            OSWDrawString(text_pm, 5, (14 / 2) + 5, text);
            tmp_img = OSWGetImage(
                text_pm, 0, 0, img->width, 30
            );
            if(tmp_img != NULL)
                BlitBufNormal(
                    osw_gui[0].depth,
                    (u_int8_t *)img->data,
                    (u_int8_t *)tmp_img->data,
                    5, (int)img->height - 25,
                    img->width, img->height,
                    0, 0,
                    tmp_img->width, tmp_img->height, 
                    tmp_img->width, tmp_img->height,
                    1.0, 1.0, 1.0
                );
	    OSWDestroyImage(&tmp_img);
	}


	/* Put image to window. */
	OSWPutSharedImageToDrawable(img, w);


        /* ********************************************************* */
	/* Destroy text_pm and text_pix. */
	OSWDestroyPixmap(&text_pm);
	OSWDestroyPixel(&text_pix);


	return;
}





