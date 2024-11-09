// unvedit/printwin.cpp



/*
                              Print Window

	double PrintWinCmToPixels(double x)
	double PrintWinInchToPixels(double x)
	double PrintWinPixelsToCm(double x)
	double PrintWinPixelsToInch(double x)

	void PrintWinGetPaperSizePixels(
		print_win_struct *pw,
		unsigned int *paper_width,
		unsigned int *paper_height
	)

	image_t *PrintWinRotateImage(image_t *image, depth_t d)
	image_t *PrintWinCreateSpool(print_win_struct *pw)
	int PrintWinDoPrint(print_win_struct *pw)

	int PrintWinCancelPBCB(void *ptr)
	int PrintWinPrintPBCB(void *ptr)

	int PrintWinInit()
	int PrintWinDraw(int amount)
	int PrintWinManage(event_t *event)
	void PrintWinMap()
	void PrintWinDoMapValues(uew_struct *src_uew)
	void PrintWinUnmap()
	void PrintWinDestroy()

	---


 */
/*
#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
#include <string.h>
*/
#include <sys/stat.h>

#include "../include/string.h"
#include "../include/disk.h"

#include "../include/osw-x.h"
#include "../include/widget.h"
#include "../include/printerw.h"

#include "../include/reality.h"
#include "../include/objects.h"
#include "../include/unvmatch.h"
#include "../include/unvutil.h"

#include "uew.h"
#include "ue.h"

namespace static_printwin {
	bool_t preview_drag_state;
}

#include "printwin.h"

#ifndef MAX
#define MIN(a,b)        (((a) < (b)) ? (a) : (b))
#define MAX(a,b)	(((a) > (b)) ? (a) : (b))
#endif

/* #define MIN(a,b)	((a) < (b) ? (a) : (b)) */
/* #define MAX(a,b)	((a) > (b) ? (a) : (b)) */


//static bool_t preview_drag_state;




/*
 *	Unit conversions:
 */

double PrintWinCmToPixels(double x)
{
        return(x * 72 / 2.56);
}

double PrintWinInchToPixels(double x)
{
        return(x * 72);
}

double PrintWinPixelsToCm(double x)
{
        return(x / 72 * 2.54);
}

double PrintWinPixelsToInch(double x)
{
        return(x / 72);
}


/*
 *	Fetches paper size specified on printer window pw.
 */
void PrintWinGetPaperSizePixels(
	print_win_struct *pw,
	unsigned int *paper_width,
	unsigned int *paper_height
)
{
	if((pw == NULL) ||
	   (paper_width == NULL) ||
           (paper_height == NULL)
	)
	    return;


	switch(pw->paper_size_tba.armed_tb)
        {
	  /* A4 (21.0 x 29.7 cm), 8.27 x 11.69 inch. */
	  case 3:
            *paper_width = static_cast<unsigned int>(PrintWinInchToPixels(8.26772));
            *paper_height = static_cast<unsigned int>(PrintWinInchToPixels(11.69291));
	    break;

	  /* Executive, 7.5 x 10 inch. */
          case 2:
            *paper_width = static_cast<unsigned int>(PrintWinInchToPixels(7.5));
            *paper_height = static_cast<unsigned int>(PrintWinInchToPixels(10));
            break;

	  /* Legal, 8.5 x 14 inch. */
	  case 1:
	    *paper_width = static_cast<unsigned int>(PrintWinInchToPixels(8.5));
	    *paper_height = static_cast<unsigned int>(PrintWinInchToPixels(14));
	    break;

	  /* Legal, 8.5 by 11 inch. */
          default:
            *paper_width = static_cast<unsigned int>(PrintWinInchToPixels(8.5));
            *paper_height = static_cast<unsigned int>(PrintWinInchToPixels(11));
            break;
        }


	return;
}


/*
 *	Creates a duplicate of image rotated 90 degrees
 *	counter-clockwise.
 */
image_t *PrintWinRotateImage(image_t *image, depth_t d)
{
	int tar_bytes_per_line, src_bytes_per_line;
	int tar_row, tar_col, src_row, src_col;
	unsigned int tar_width, tar_height, src_width, src_height;

	u_int8_t *tar, *src;

	u_int8_t *tar_ptr8, *src_ptr8;
	u_int16_t *tar_ptr16, *src_ptr16;
	u_int32_t *tar_ptr32, *src_ptr32;

	image_t *new_image = NULL;


	if(image == NULL)
	    return(NULL);


	src_width = image->width;
	src_height = image->height;

	/* Create new image with swapped width and height. */
	if(OSWCreateImage(&new_image, src_height, src_width))
	    return(NULL);

	tar_width = new_image->width;
	tar_height = new_image->height;

	tar = (u_int8_t *)new_image->data;
	src = (u_int8_t *)image->data;

	tar_row = (int)tar_height - 1;
	tar_col = 0;
	src_row = 0;
	src_col = 0;


	/* 8 bits. */
	if(d == 8)
	{
	    tar_bytes_per_line = tar_width * BYTES_PER_PIXEL8;
	    src_bytes_per_line = src_width * BYTES_PER_PIXEL8;

	    while((src_row < (int)src_height) &&
                  (tar_col < (int)tar_width)
	    )
	    {
		tar_ptr8 = (u_int8_t *)(&tar[
		    (tar_row * tar_bytes_per_line) +
                    (tar_col * BYTES_PER_PIXEL8)
		]);

		src_ptr8 = (u_int8_t *)(&src[
                    (src_row * src_bytes_per_line) +
                    (src_col * BYTES_PER_PIXEL8)
                ]);

		*tar_ptr8 = *src_ptr8;

		src_col++;
		tar_row--;
		if(src_col >= (int)src_width)
		{
		    src_row++;
		    src_col = 0;
		}
		if(tar_row < 0)
		{
		    tar_col++;
		    tar_row = (int)tar_height - 1;
		}
	    }
	}
	/* 15 or 16 bits. */
        else if((d == 15) ||
                (d == 16)
	)
        {
            tar_bytes_per_line = tar_width * BYTES_PER_PIXEL16;
            src_bytes_per_line = src_width * BYTES_PER_PIXEL16;

            while((src_row < (int)src_height) &&
                  (tar_col < (int)tar_width)
            )
            {
                tar_ptr16 = (u_int16_t *)(&tar[
                    (tar_row * tar_bytes_per_line) +
                    (tar_col * BYTES_PER_PIXEL16)
                ]);

                src_ptr16 = (u_int16_t *)(&src[
                    (src_row * src_bytes_per_line) +
                    (src_col * BYTES_PER_PIXEL16)
                ]);
        
                *tar_ptr16 = *src_ptr16;

                src_col++;
                tar_row--;
                if(src_col >= (int)src_width)
                {
                    src_row++;
                    src_col = 0;
                }
                if(tar_row < 0)
                {
                    tar_col++;
                    tar_row = (int)tar_height - 1;
                }  
            }
        }
        /* 24 or 32 bits. */  
        else if((d == 24) ||
                (d == 32)
        )
        {
            tar_bytes_per_line = tar_width * BYTES_PER_PIXEL32;
            src_bytes_per_line = src_width * BYTES_PER_PIXEL32;

            while((src_row < (int)src_height) &&
                  (tar_col < (int)tar_width)
            )
            {
               tar_ptr32 = (u_int32_t *)(&tar[
                    (tar_row * tar_bytes_per_line) +
                    (tar_col * BYTES_PER_PIXEL32)
                ]);
                    
                src_ptr32 = (u_int32_t *)(&src[
                    (src_row * src_bytes_per_line) +
                    (src_col * BYTES_PER_PIXEL32)
                ]);
        
                *tar_ptr32 = *src_ptr32;
         
                src_col++;
                tar_row--;
                if(src_col >= (int)src_width)
                {
                    src_row++;
                    src_col = 0;
                }
                if(tar_row < 0)
                {
                    tar_col++;
                    tar_row = (int)tar_height - 1;
                }  
            }
        }


	return(new_image);
}


/*
 *	Procedure to create and draw up a spool image based
 *	on the uew referanced in the given pw.
 *
 *	Draw routine procedure calculations should be identical
 *	in this function as in function UEWViewDraw().
 */
image_t *PrintWinCreateSpool(print_win_struct *pw)
{
        pixmap_t pixmap;
        image_t *new_img;

        int x, y, i, sw, sh, swv, shv;
        int pos, inc;
	int len;
        long sect_cur;
        long sect_x_min, sect_x_max, sect_y_min, sect_y_max;
        long d_sect_x, d_sect_y;
	double radius;

	char *strptr;
        uew_struct *uew_ptr = NULL;
        shared_image_t *img;   
        xsw_object_struct **obj_ptr;
        char text[256];

        pixel_t pix_bg,
                pix_sect_grid,
                pix_grid,
		pix_obj_mark,
		pix_area,
		pix_obj_text;

	font_t *prev_font;


	if(pw == NULL)
	    return(NULL);


        for(i = 0; i < total_uews; i++)
        {
            if(uew[i] == NULL)
                continue;
        
            if(uew[i] == (uew_struct *)pw->src)
                uew_ptr = uew[i];
        }
        if(uew_ptr == NULL)
	{
            fprintf(stderr,
 "PrintWinCreateSpool(): Source window non-existant, cannot create spool.\n"
            );
            return(NULL);
	}

        prev_font = OSWQueryCurrentFont();


	/* ********************************************************** */

	/* Get pointer to uew's view window image. */
        img = uew_ptr->view_img;
        if(img == NULL)
	{
	    fprintf(stderr,
 "PrintWinCreateSpool(): View image buffer not allocated.\n"
	    );
            fprintf(stderr,   
 "PrintWinCreateSpool(): Cannot generate spool without any source data.\n"
            );

            return(NULL);
	}

	/* Create a pixmap the same size as the image. */
	if(OSWCreatePixmap(&pixmap, img->width, img->height))
	{
            fprintf(stderr,
 "PrintWinCreateSpool(): Cannot allocate tempory graphics buffer.\n"
            );

	    return(NULL);
	}

	/* Allocate pixel colors. */
	OSWLoadPixelRGB(&pix_bg, 0xff, 0xff, 0xff);
	OSWLoadPixelRGB(&pix_sect_grid, 0x80, 0x80, 0x80);
        OSWLoadPixelRGB(&pix_grid, 0xb0, 0xb0, 0xb0);
        OSWLoadPixelRGB(&pix_obj_mark, 0x00, 0x00, 0x00);
        OSWLoadPixelRGB(&pix_area, 0x80, 0x80, 0x80);
        OSWLoadPixelRGB(&pix_obj_text, 0x00, 0x00, 0x00);


        /* Sanitize zoom. */
        if(uew_ptr->zoom < UEW_VIEW_ZOOM_MIN)
            uew_ptr->zoom = UEW_VIEW_ZOOM_MIN;
        if(uew_ptr->zoom > UEW_VIEW_ZOOM_MAX)
            uew_ptr->zoom = UEW_VIEW_ZOOM_MAX;


        /* Clear pixmap. */
	OSWClearPixmap(
	    pixmap,
	    img->width, img->height,
	    pix_bg
	);


        /* ********************************************************* */
	/* Draw grids (not sector grids). */
        OSWSetFgPix(pix_grid);

	/* Vertical grids. */
        inc = static_cast<int>(option.grid_spacing * uew_ptr->zoom);
        swv = ((int)img->width / inc);
        x = static_cast<int>((((option.grid_spacing / 2) - uew_ptr->cx) * uew_ptr->zoom) +
            ((int)img->width / 2));

        if(inc >= 20)
        {
            for(pos = x % inc; pos < (int)img->width; pos += inc)
            {
                if(pos < 0)
                    continue;

		OSWDrawLine(
		    pixmap,
		    pos, 0,
		    pos, (int)img->height - 1
		);
            }
        }

	/* Horizontal grids. */
        inc = static_cast<int>(option.grid_spacing * uew_ptr->zoom);
        shv = ((int)img->height / inc);
        y = static_cast<int>((((option.grid_spacing / 2) - uew_ptr->cy) * uew_ptr->zoom) +
            ((int)img->height / 2));  

        if(inc >= 20)
        {
            for(pos = ((int)img->height - y) % inc;
                pos < (int)img->height;
                pos += inc
            )
            {
                if(pos < 0)
                    continue;

                OSWDrawLine(
                    pixmap,
                    0, pos,
                    (int)img->width - 1, pos
                );
            }
        }       


        /* ********************************************************* */
        /* Draw vertical sector grids. */

        OSWSetFgPix(pix_sect_grid);
        OSWSetFont(ue_font.view_label);

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

        for(pos = x, inc = sw; pos < (int)img->width; pos += inc, sect_cur++)
        {
            if(pos < 0)
                continue;

            OSWDrawLine(
                pixmap,
                pos, 0,
                pos, (int)img->height - 1
            );

            if(pw->label_geometry_tb.state)
	    {
                sprintf(text, "%ld", sect_cur);
                OSWDrawString(
		    pixmap,
		    pos + 7,
		    (14 / 2) + 7,
		    text
		);
	    }
        }


	/* Horizontal sector grids. */

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

        sect_cur = uew_ptr->sect_y + shv;

        for(pos = y, inc = sh; pos < (int)img->height; pos += inc, sect_cur--)
        {
            if(pos < 0)
                continue;

            OSWDrawLine(
                pixmap,
                0, pos,
                (int)img->width - 1, pos
            );

            if(pw->label_geometry_tb.state)
	    {
                sprintf(text, "%ld", sect_cur);
                OSWDrawString(
		    pixmap,
		    6, pos + (14 / 2) + 7,
		    text
	        );
	    }
        }
                
        /* ********************************************************* */
        /* Draw objects. */

        OSWSetFont(ue_font.view_obj_label);
            
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

	    /* Draw only selected objects? */
	    if(pw->objects_pulist.sel_item == 1)
	    {
		/* Selected object(s). */
		if(!UEWIsObjectSelected(uew_ptr, i))
		    continue;
	    }
	    else if(pw->objects_pulist.sel_item == 2)
	    {
		/* Astronomical objects. */
		if(((*obj_ptr)->type != XSW_OBJ_TYPE_AREA) &&
                   ((*obj_ptr)->type != XSW_OBJ_TYPE_HOME)
		)
		    continue;
	    }


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
	    OSWSetFgPix(pix_obj_mark);
            OSWDrawArc(
		pixmap,
		x - 3,
		y - 3,
		6,
		6,
		0, 2 * PI
	    );

            /* Area type objects? */
            if((*obj_ptr)->type == XSW_OBJ_TYPE_AREA)
            {
                OSWSetFgPix(pix_area);

		radius = (double)(*obj_ptr)->size / 1000 * uew_ptr->zoom;

                OSWDrawArc(
                    static_cast<int>(pixmap),
                    static_cast<int>(x - radius),
                    static_cast<unsigned int>(y - radius),
                    static_cast<unsigned int>(radius * 2),
                    static_cast<unsigned int>(radius * 2),
                    0, 2 * PI
                );
            }

	    /* Draw text message. */
	    if(pw->obj_labels_tb.state)
	    {
                OSWSetFgPix(pix_obj_text);
	        strptr = UNVGetObjectFormalName(*obj_ptr, i);
	        if(strptr != NULL)
	        {
		    len = strlen(strptr);
	            OSWDrawString(
		        pixmap,
		        x - (len * 7 / 2),
		        y - 6,
	                strptr
	            );
		}
	    }
        }


        /* Do not draw center cross (not needed on a print out). */


	/* Draw center of position text. */
	if(pw->label_geometry_tb.state)
	{
            sprintf(
		text,
                "Zoom: %.4f",
                uew_ptr->zoom
            );
            OSWSetFont(ue_font.view_label);
            OSWDrawString(
	        pixmap,
                12, (int)img->height - 10,
                text
            );
	}


	/* Get new image from pixmap. */
	new_img = OSWGetImage(
	    pixmap,
	    0, 0,
	    img->width, img->height
	);
	if(new_img == NULL)
	{
            fprintf(stderr,
 "PrintWinCreateSpool(): Graphics buffer conversion failed.\n"
            );
	}

	/* Destroy pixmap, no longer needed. */
	OSWDestroyPixmap(&pixmap);


	/* Unload pixels. */
        OSWDestroyPixel(&pix_bg);
        OSWDestroyPixel(&pix_sect_grid);
        OSWDestroyPixel(&pix_grid);
        OSWDestroyPixel(&pix_obj_mark);
        OSWDestroyPixel(&pix_area);
        OSWLoadPixelRGB(&pix_obj_text, 0x00, 0x00, 0x00);

	OSWSetFont(prev_font);


	return(new_img);
}


/*
 *	Procedure to do printing.
 */
int PrintWinDoPrint(print_win_struct *pw)
{
        int i, status;
	unsigned int width, height;
	char *strptr;
        unsigned int paper_width, paper_height;
	printer_parm_struct parm;
	struct stat stat_buf;

	image_t *img_ptr, *tmp_img_ptr;
	int uew_num = -1;
        uew_struct *uew_ptr = NULL;

	char spool_file[PATH_MAX + NAME_MAX];
	char text[PATH_MAX + NAME_MAX + 256];


        if(pw == NULL)
	    return(-1);


        for(i = 0; i < total_uews; i++)
        {
            if(uew[i] == NULL)
                continue;

            if(uew[i] == (uew_struct *)pw->src)
	    {
                uew_ptr = uew[i];
		uew_num = i;
	    }
        }
        if(uew_ptr == NULL)
            return(-1);

	/* Make sure spool dir exists. */
	if(stat(PromptGetS(&pw->spool_dir), &stat_buf))
	{
	    sprintf(text,
"Invalid spool directory:\n\n\
    %s\n",
		PromptGetS(&pw->spool_dir)
	    );
	    printdw(&dialog, text);
	    return(-1);
	}
	if(!S_ISDIR(stat_buf.st_mode))
        {
            sprintf(text,
"Invalid spool directory:\n\n\
    %s\n",
                PromptGetS(&pw->spool_dir)
            );
            printdw(&dialog, text);
            return(-1);
        }

	/* Print size cannot be zero. */
	if((PromptGetF(&pw->width_prompt) <= 0) ||
	   (PromptGetF(&pw->height_prompt) <= 0)
	)
	{
            sprintf(text,
 "Document dimension cannot be zero or negative.\n"
            );
            printdw(&dialog, text);
            return(-1);  
        }


        /* Get paper width and height (in pixels). */
        PrintWinGetPaperSizePixels(pw, &paper_width, &paper_height);


	/* ********************************************************** */
	/* Set up printer parameters. */

	parm.options = 0;

	parm.title[0] = '\0';
	parm.header[0] = '\0';
	parm.footer[0] = '\0';

	parm.pages = 1;

	/* Set color mode. */
	switch(pw->color_mode_tba.armed_tb)
	{
	  case 1:
	    parm.color_mode = PrinterColorModeColor;
	    break;

	  default:
	    parm.color_mode = PrinterColorModeGreyScale;
            break;
	}

	/* Set units. */
	switch(pw->units)
	{
	  case PW_UNITS_CM:
	    parm.units = PrinterUnitCentimeters;
	    break;

	  case PW_UNITS_INCH:
            parm.units = PrinterUnitInches;
            break;

	  default:	/* PW_UNITS_PIXELS */
	    parm.units = PrinterUnitPixels;
	    break;
	}

	/* Get x and y offsets from upper left corner (no unit conv) */
	parm.x = PromptGetF(&pw->x_prompt);
	parm.y = PromptGetF(&pw->y_prompt);

	/* Set paper size. */
        switch(pw->units)
        {
	  case PW_UNITS_CM:
	    parm.width = PrintWinPixelsToCm(paper_width);
            parm.height = PrintWinPixelsToCm(paper_height);
	    break;

          case PW_UNITS_INCH:
            parm.width = PrintWinPixelsToInch(paper_width);
            parm.height = PrintWinPixelsToInch(paper_height);
            break;

          default:	/* PW_UNITS_PIXELS */
            parm.width = (double)paper_width;
            parm.height = (double)paper_height;
            break;
	}

	/* Destination. */
	switch(pw->print_to_tba.armed_tb)
	{
	  case 1:
	    parm.dest = PrinterDestinationFile;
	    break;

	  default:
	    parm.dest = PrinterDestinationPrinter;
	}


	/* ****************************************************** */
	/* Create spool image. */
	img_ptr = PrintWinCreateSpool(pw);
	if(img_ptr == NULL)
	    return(-1);


	/* ****************************************************** */
	/* Resize spool image. */
	switch(pw->units)
	{
	  case PW_UNITS_CM:
	    width = static_cast<unsigned int>(PrintWinCmToPixels(
		PromptGetF(&pw->width_prompt)
	    ));
            height = static_cast<unsigned int>(PrintWinCmToPixels(
                PromptGetF(&pw->height_prompt)
            ));
	    break;

          case PW_UNITS_INCH:
            width = static_cast<unsigned int>(PrintWinInchToPixels(
                PromptGetF(&pw->width_prompt)
            ));
            height = static_cast<unsigned int>(PrintWinInchToPixels(
                PromptGetF(&pw->height_prompt)
            ));
            break;

          default:
            width = static_cast<unsigned int>(PromptGetI(&pw->width_prompt));
            height = static_cast<unsigned int>(PromptGetF(&pw->height_prompt));
            break;
	}

	if(OSWCreateImage(&tmp_img_ptr, width, height))
	{
	    OSWDestroyImage(&img_ptr);
	    return(-1);
	}

	WidgetResizeImageBuffer(
            osw_gui[0].depth,
            reinterpret_cast<u_int8_t *>(tmp_img_ptr->data),	/* Target. */
            reinterpret_cast<u_int8_t *>(img_ptr->data),	/* Source. */
            tmp_img_ptr->width,
            tmp_img_ptr->height,
            img_ptr->width,   
            img_ptr->height
        );
	OSWDestroyImage(&img_ptr);

	img_ptr = tmp_img_ptr;


        /* Rotate spool image if orientation is set to landscape. */
        if(pw->orientation_tba.armed_tb == 1)
        {
	    tmp_img_ptr = PrintWinRotateImage(
		img_ptr,
		osw_gui[0].depth
	    );
	    if(tmp_img_ptr == NULL)
	    {
		OSWDestroyImage(&img_ptr);
		return(-1);
	    }

	    OSWDestroyImage(&img_ptr);

	    img_ptr = tmp_img_ptr;
        }


        /* Check if image is out of bounds of the size of the paper. */
	switch(pw->units)
	{
	  case PW_UNITS_CM:
	    width = static_cast<int>(PrintWinPixelsToCm((int)img_ptr->width));
	    height = static_cast<int>(PrintWinPixelsToCm((int)img_ptr->height));
	    break;

          case PW_UNITS_INCH:
            width = static_cast<int>(PrintWinPixelsToInch((int)img_ptr->width));
            height = static_cast<int>(PrintWinPixelsToInch((int)img_ptr->height));
            break;

          default:
            width = img_ptr->width;
            height = img_ptr->height;
            break;
	}

        if(((int)parm.x < 0) ||
           ((int)parm.y < 0) ||   
           (((int)parm.x + (int)width) > (int)parm.width) ||
           (((int)parm.y + (int)height) > (int)parm.height)
	)
	{
            comfwin.option &= ~ComfirmOptionCancel;
            comfwin.option &= ~ComfirmOptionAll;
            status = ComfWinDoQuery(&comfwin,
"Image geometry may be partially or entirly out\n\
of the bounds of the paper. Continue printing?"
            );
	    if(status == ComfirmCodeNo)
	    {
		OSWDestroyImage(&img_ptr);
		return(-1);
	    }
	}
         


	/* ******************************************************** */

	/* Print to file or printer? */
	if(pw->print_to_tba.armed_tb == 1)
	{
	    /* Print to file. */

	    /* Get tmp file name. */
	    strptr = tempnam(
		PromptGetS(&pw->spool_dir),
		"print_spool"
	    );

	    if(strptr != NULL)
	    {
                strncpy(
                    spool_file,
		    strptr,
		    PATH_MAX + NAME_MAX
		);
		spool_file[PATH_MAX + NAME_MAX - 1] = '\0';

	        status = PrinterPrintImage(
                    img_ptr,
                    spool_file,
		    PromptGetS(&pw->print_cmd_prompt),
		    &parm
		);
		if(status == PrinterSuccess)
		{
		    sprintf(text,
		        "Printed spool file `%s'.",
		        spool_file
		    );
		    UEWDoSetStatusMesg(uew_num, text);
		}
		else
		{
                    sprintf(text,
                        "Print failed."
                    );
                    UEWDoSetStatusMesg(uew_num, text);

		    sprintf(text,
                        "Print failed, error code `%i'.\n",
			status
                    );
		    printdw(&dialog, text);

                    OSWDestroyImage(&img_ptr);

                    /* Free tmp name. */
		    free(strptr);
		    strptr = NULL;

		    return(-1);
		}
	    }

            /* Free tmp name. */
	    free(strptr);
	    strptr = NULL;
	}
	else
	{
	    /* Print to printer. */

            /* Get tmp file name. */
            strptr = tempnam(
                PromptGetS(&pw->spool_dir),
                "print_spool"
            );

            if(strptr != NULL)
            {
                strncpy(
                    spool_file,
                    strptr,
                    PATH_MAX + NAME_MAX
                );
                spool_file[PATH_MAX + NAME_MAX - 1] = '\0';
                        
                status = PrinterPrintImage(
                    img_ptr,
                    spool_file,
                    PromptGetS(&pw->print_cmd_prompt),
                    &parm
                );
                if(status == PrinterSuccess)
                {
                    sprintf(text,
                        "Printed universe."
                    );
                    UEWDoSetStatusMesg(uew_num, text);
                }
                else
                {
                    sprintf(text,
                        "Print failed."
                    );
                    UEWDoSetStatusMesg(uew_num, text);

                    sprintf(text,
                        "Print failed, error code `%i'.\n",
                        status
                    );
                    printdw(&dialog, text);

                    OSWDestroyImage(&img_ptr);

                    /* Free tmp name. */                 
                    free(strptr);
                    strptr = NULL;

                    return(-1);
                }
            }

	    /* Free tmp name. */
            free(strptr);
            strptr = NULL;
	}


	/* Destroy spool image. */
	OSWDestroyImage(&img_ptr);



        return(0);
}


/*
 *	Cancel button callback function.
 */
int PrintWinCancelPBCB(void *ptr)
{
        print_win_struct *pw;
        
        pw = &print_win;



	PrintWinUnmap();

        return(0);
}

int PrintWinPrintPBCB(void *ptr) 
{
	int status;
        print_win_struct *pw;

        pw = &print_win;


	status = PrintWinDoPrint(pw);

	if(status == 0)
            PrintWinUnmap();

	return(0);
}

/*
 *	Print button callback function.
 */
int PrintWinInit()
{
        char hotkey[PBTN_MAX_HOTKEYS];
	win_attr_t wattr;
	print_win_struct *pw;
	char *strarray[10];

	pw = &print_win;


        pw->map_state = 0;
        pw->is_in_focus = 0;
        pw->x = 0;
        pw->y = 0;
        pw->width = PW_WIDTH;
        pw->height = PW_HEIGHT;

	pw->units = PW_UNITS_INCH;
        pw->src = NULL;

	pw->preview_img = NULL;


        /* ******************************************************** */
        /* Create toplevel. */
        if(
            OSWCreateWindow(
                &pw->toplevel,
                osw_gui[0].root_win,
                pw->x, pw->y,
                pw->width, pw->height
            )
        )
            return(-1);

        OSWSetWindowWMProperties(
            pw->toplevel,
            PW_DEF_TITLE,
            PW_DEF_ICON_TITLE,
            ((ue_image.unvedit_icon_pm == 0) ?
                widget_global.std_icon_pm : ue_image.unvedit_icon_pm),
            False,                      /* Let WM set coordinates? */
            pw->x, pw->y,
            pw->width, pw->height,
            pw->width, pw->height,
            WindowFrameStyleFixed,
            NULL, 0   
        );

        OSWSetWindowInput(
            pw->toplevel,
            OSW_EVENTMASK_TOPLEVEL
        );

        WidgetCenterWindow(pw->toplevel, WidgetCenterWindowToRoot);
        OSWGetWindowAttributes(pw->toplevel, &wattr);
        pw->x = wattr.x;
        pw->y = wattr.y;
        pw->width = wattr.width;
        pw->height = wattr.height;


	/* Create preview window. */
        if(
            OSWCreateWindow(
                &pw->preview,
                pw->toplevel,
		10, 10, 10, 10
            )
        )  
            return(-1);
        OSWSetWindowInput(
            pw->preview,
            ExposureMask | ButtonPressMask | ButtonReleaseMask |
		PointerMotionMask
        );

	/* Load paper pixel colors. */
	OSWLoadPixelRGB(
	    &pw->paper_bg_pix,
	    0xff,
	    0xff,
	    0xff
	);
        OSWLoadPixelRGB(
            &pw->paper_fg_pix,
	    0x00,
	    0x00,
	    0x00
	);



	/* Print to toggle button array. */
	strarray[0] = "Printer";
	strarray[1] = "File";
        strarray[2] = NULL;
	if(
	    TgBtnArrayInit(
                &pw->print_to_tba,
                pw->toplevel,
                130, 10,
                2,
                0,		/* Initially selected button. */
                strarray,
                2,
                TGBTN_ARRAY_ALIGN_HORIZONTAL
	    )
        )
	    return(-1);

	/* Print command prompt. */
        if(
            PromptInit(
                &pw->print_cmd_prompt,
                pw->toplevel,
                10, 50,
                (int)pw->width - 20,
                PW_PROMPT_HEIGHT,
                PROMPT_STYLE_FLUSHED,
                "  Print Command:",
                PATH_MAX + NAME_MAX,
                0,
                NULL
            )
        )
            return(-1);
	PromptSetS(
	    &pw->print_cmd_prompt,
	    PW_DEF_PRINT_CMD
	);

        /* Spool dir. */
        if(
            PromptInit(
                &pw->spool_dir,
                pw->toplevel,
                10, 50 + PW_PROMPT_HEIGHT + 10,
                (int)pw->width - 20,
                PW_PROMPT_HEIGHT,
                PROMPT_STYLE_FLUSHED,
                "Spool Directory:",
                PATH_MAX + NAME_MAX,
                0,
                NULL
            )
        )
            return(-1);
        PromptSetS(
            &pw->spool_dir,
            PW_DEF_SPOOL_DIR
        );

	/* Amount of objects. */
	if(
	    PUListInit(
		&pw->objects_pulist,
		pw->toplevel,
                20, 170,
                ((int)pw->width / 3) - 25, 0,
                3,		/* Items visible when poped. */
                PULIST_POPUP_DOWN,
                (void *)&pw->objects_pulist,
                NULL
	    )
        )
	    return(-1);
	PUListAddItem(
            &pw->objects_pulist,
	    "All Objects",
	    False
        );
        PUListAddItem(
            &pw->objects_pulist,
            "Selected Objects",
            False
        );
        PUListAddItem(
            &pw->objects_pulist,
            "Astronomical Objects",
            False
        );


	/* Orientation. */
        strarray[0] = "Portrait";
        strarray[1] = "Landscape";
        strarray[2] = NULL;
        if(
            TgBtnArrayInit(
                &pw->orientation_tba,
                pw->toplevel,
                ((int)pw->width / 3 * 2) + 15,
		170,
                2,
                0,              /* Initially selected button. */
                strarray,
                2,
                TGBTN_ARRAY_ALIGN_VERTICAL
            )
        )
            return(-1);
	

	/* Color mode. */
        strarray[0] = "GreyScale";
        strarray[1] = "Color"; 
        strarray[2] = NULL;
        if(
            TgBtnArrayInit(
                &pw->color_mode_tba,
                pw->toplevel,
                ((int)pw->width / 3 * 2) + 15,
                235,
                2,
                0,              /* Initially selected button. */
                strarray,
                2,
                TGBTN_ARRAY_ALIGN_VERTICAL
            )
        )
            return(-1);

	/* Paper size. */
        strarray[0] = "Letter (8.5 x 11 in)";
        strarray[1] = "Legal (8.5 x 14 in)";
        strarray[2] = "Executive (7.5 x 10 in)";
        strarray[3] = "A4 (21.0 x 29.7 cm)";
        strarray[4] = NULL;
        if(
            TgBtnArrayInit(
                &pw->paper_size_tba,
                pw->toplevel,
                ((int)pw->width / 3 * 2) + 15,
                300,
                4,
                0,              /* Initially selected button. */
                strarray,
                4,
                TGBTN_ARRAY_ALIGN_VERTICAL
            )
        )
            return(-1);


	/* Paper position prompts. */
        if(
            PromptInit(
                &pw->x_prompt,
                pw->toplevel,
                10, 210,
                160,
                PW_PROMPT_HEIGHT,
                PROMPT_STYLE_FLUSHED,
                "     X:",
                40,
                0,
                NULL
            )
        )
            return(-1);
        PromptSetF(
            &pw->x_prompt,
            0
        );

        if(
            PromptInit(
                &pw->y_prompt,
                pw->toplevel,
                10, 240,
                160,
                PW_PROMPT_HEIGHT,
                PROMPT_STYLE_FLUSHED,
                "     Y:",
                40,
                0,
                NULL
            )
        )
            return(-1);
        PromptSetF(
            &pw->y_prompt,
            0
        );

        if(
            PromptInit(
                &pw->width_prompt,
                pw->toplevel,
                10, 280,
                160,
                PW_PROMPT_HEIGHT,
                PROMPT_STYLE_FLUSHED,
                " Width:",
                40,
                0,
                NULL
            )
        )
            return(-1);
        PromptSetF(
            &pw->width_prompt,
            10
        );

        if(
            PromptInit(
                &pw->height_prompt,
                pw->toplevel,
                10, 310,
                160,
                PW_PROMPT_HEIGHT,
                PROMPT_STYLE_FLUSHED,
                "Height:",
                40, 
                0,
                NULL
            )
        )
            return(-1);
        PromptSetF(
            &pw->height_prompt,
            10
        );

	/* Show object labels. */
        if(
            TgBtnInit(
                &pw->obj_labels_tb,
                pw->toplevel,
                10, 350,
		True,
		"Label Objects"
            )
        )
            return(-1);

        /* Label geometry. */
        if(
            TgBtnInit(  
                &pw->label_geometry_tb,
                pw->toplevel,
                10, 380,
                ((option.label_geometry) ? True : False),
                "Label Geometry"
            )
        )
            return(-1);
 

	/* Print button. */
        if(
            PBtnInit(
                &pw->print_btn,
                pw->toplevel,
                10,
                (int)pw->height - 10 - PW_BUTTON_HEIGHT,
                PW_BUTTON_WIDTH, PW_BUTTON_HEIGHT,
                "Print",
                PBTN_TALIGN_CENTER,
                NULL,
                (void *)pw,
                PrintWinPrintPBCB
            )
        )   
            return(-1);
        hotkey[0] = '\n'; 
        hotkey[1] = '\0';
        PBtnSetHotKeys(&pw->print_btn, hotkey);

	/* Cancel button. */
        if(
            PBtnInit(
                &pw->cancel_btn,
                pw->toplevel,
                (int)pw->width - 10 - PW_BUTTON_WIDTH,
                (int)pw->height - 10 - PW_BUTTON_HEIGHT,
                PW_BUTTON_WIDTH, PW_BUTTON_HEIGHT,
                "Cancel",
                PBTN_TALIGN_CENTER,
                NULL,
                (void *)pw,
                PrintWinCancelPBCB
            )
        )
            return(-1);
	hotkey[0] = 0x1b;
	hotkey[1] = '\0';
	PBtnSetHotKeys(&pw->cancel_btn, hotkey); 








	return(0);
}


int PrintWinDraw(int amount)
{
	int i;
	int x, y;
	unsigned int width, height;
	unsigned int paper_width, paper_height;
	double coeff, width_coeff, height_coeff;

	win_t w;
	pixmap_t pixmap;
	image_t *img_ptr, *tmp_img_ptr;
	font_t *prev_font;
	win_attr_t wattr;
        print_win_struct *pw;

        
        pw = &print_win;


	/* Map as needed. */
	if(!pw->map_state)
	{
            OSWMapRaised(pw->toplevel);
	    OSWMapWindow(pw->preview);
	    pw->map_state = 1;

	    amount = PW_DRAW_AMOUNT_COMPLETE;


            PBtnMap(&pw->cancel_btn);
            PBtnMap(&pw->print_btn);

            TgBtnArrayMap(&pw->orientation_tba);
            TgBtnArrayMap(&pw->color_mode_tba);
            TgBtnArrayMap(&pw->paper_size_tba);

            TgBtnMap(&pw->obj_labels_tb);
            TgBtnMap(&pw->label_geometry_tb);

            PromptMap(&pw->x_prompt);
            PromptMap(&pw->y_prompt);
            PromptMap(&pw->width_prompt);
            PromptMap(&pw->height_prompt);

            PUListMap(&pw->objects_pulist);

            PromptMap(&pw->spool_dir);
            PromptMap(&pw->print_cmd_prompt);

            TgBtnArrayMap(&pw->print_to_tba);


	    /* Destroy preview document image to force recreation. */
	    OSWDestroyImage(&pw->preview_img);
	}


	/* Recreate toplevel buffer. */
	if(pw->toplevel_buf == 0)
	{
	    OSWGetWindowAttributes(pw->toplevel, &wattr);
	    if(OSWCreatePixmap(
		&pw->toplevel_buf, wattr.width, wattr.height
	    ))
		return(-1);
	}

	prev_font = OSWQueryCurrentFont();



	/* Redraw background? */
        if(amount == PW_DRAW_AMOUNT_COMPLETE)
        {
            w = pw->toplevel;
            pixmap = pw->toplevel_buf; 
                  
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
            }
	    else
            {
                WidgetPutImageTile( 
                    pixmap,
                    widget_global.std_bkg_img,
                    wattr.width, wattr.height
                );

		/* Draw upper HR. */
                OSWSetFgPix(widget_global.surface_shadow_pix);
                OSWDrawLine(  
                    pixmap, 
                    0,
                    135,
                    wattr.width,
                    135
                );
                OSWSetFgPix(widget_global.surface_highlight_pix);
                OSWDrawLine(
                    pixmap,
                    0,
                    136,
                    wattr.width,
                    136
                );

		/* Draw lower HR. */
                OSWSetFgPix(widget_global.surface_shadow_pix); 
                OSWDrawLine(   
                    pixmap,
                    0,
                    (int)wattr.height - 25 - PW_BUTTON_HEIGHT,
                    wattr.width,
                    (int)wattr.height - 25 - PW_BUTTON_HEIGHT
                );
                OSWSetFgPix(widget_global.surface_highlight_pix);
                OSWDrawLine(
                    pixmap,
                    0,
                    (int)wattr.height - 24 - PW_BUTTON_HEIGHT,
                    wattr.width,
                    (int)wattr.height - 24 - PW_BUTTON_HEIGHT
                );

                OSWSetFgPix(widget_global.normal_text_pix);
	    }

            OSWDrawString(
                pixmap,
                62, 25,
                "Print To:"
            );
            OSWDrawString(
                pixmap,
                12, 163,
                "Print:"
            );
            OSWDrawString(
                pixmap, 
                ((int)pw->width / 3 * 2) + 5, 163,
                "Orientation:"
            );
            OSWDrawString(
                pixmap,
                ((int)pw->width / 3 * 2) + 5, 230,
                "Color Mode:"
            );
            OSWDrawString(
                pixmap,
                ((int)pw->width / 3 * 2) + 5, 296,
                "Paper Size:"
            );


            OSWPutBufferToWindow(w, pixmap);  
        }


        /* Redraw preview? */
        if((amount == PW_DRAW_AMOUNT_COMPLETE) ||
	   (amount == PW_DRAW_AMOUNT_PREVIEW)
	)
        {
            w = pw->preview;
            pixmap = pw->toplevel_buf;


	    /* Calculate bounds of paper. */
	    PrintWinGetPaperSizePixels(pw, &paper_width, &paper_height);

	    coeff = (double)paper_height / (double)paper_width;

	    width = MAX(((int)pw->width / 3) - 20, 10);
	    height = static_cast<unsigned int>(MAX((int)width * coeff, 10));

	    i = (int)pw->height - 150 - PW_BUTTON_HEIGHT - 40;
	    if((int)height > i)
	    {
		height = i;
		width = static_cast<unsigned int>(MAX((int)height / coeff, 10));
	    }

	    x = ((int)pw->width / 2) - ((int)width / 2);
	    y = (((int)pw->height - 150 - PW_BUTTON_HEIGHT - 30) / 2) -
		((int)height / 2) + 150;

	    /* Move preview iwndow. */
	    OSWMoveResizeWindow(
		pw->preview,
		x, y,
		width, height
	    );

	    OSWGetWindowAttributes(pw->preview, &wattr);

            OSWClearPixmap(
                pixmap,
                wattr.width, wattr.height,
                (widget_global.force_mono) ? osw_gui[0].white_pix :
		    pw->paper_bg_pix
            );


	    /* Draw outline of printed area. */
	    if(pw->orientation_tba.armed_tb == 1)
	    {
		/* Landscape. */

		switch(pw->units)
		{
		  case PW_UNITS_CM:
                    x = static_cast<int>(PrintWinCmToPixels(
                        PromptGetF(&pw->x_prompt)
                    ) * (double)wattr.width / (double)paper_width);
                    y = static_cast<int>(PrintWinCmToPixels(
                        PromptGetF(&pw->y_prompt)
                    ) * (double)wattr.height / (double)paper_height);

                    width_coeff = PrintWinCmToPixels(
                        PromptGetF(&pw->height_prompt)) / (double)paper_width;
                    height_coeff = PrintWinCmToPixels(
                        PromptGetF(&pw->width_prompt)) / (double)paper_height;

                    width = static_cast<unsigned int>(width_coeff * (double)wattr.width);
                    height = static_cast<unsigned int>(height_coeff * (double)wattr.height);
		    break;

                  case PW_UNITS_INCH:
                    x = static_cast<int>(PrintWinInchToPixels(
                        PromptGetF(&pw->x_prompt)
                        ) * (double)wattr.width / (double)paper_width);
                    y = static_cast<int>(PrintWinInchToPixels(
                        PromptGetF(&pw->y_prompt)
                        ) * (double)wattr.height / (double)paper_height);
 
                    width_coeff = PrintWinInchToPixels(
                        PromptGetF(&pw->height_prompt)) / (double)paper_width;
                    height_coeff = PrintWinInchToPixels(
                        PromptGetF(&pw->width_prompt)) / (double)paper_height;

                    width = static_cast<unsigned int>(width_coeff * (double)wattr.width);
                    height = static_cast<unsigned int>(height_coeff * (double)wattr.height);
                    break;

                  default:	/* PW_UNITS_PIXELS */
                    x = static_cast<int>(PromptGetF(&pw->x_prompt) * (double)wattr.width /
                        (double)paper_width);
                    y = static_cast<int>(PromptGetF(&pw->y_prompt) * (double)wattr.height /
                        (double)paper_height);

                    width_coeff = PromptGetF(&pw->height_prompt) / (double)paper_width;
                    height_coeff = PromptGetF(&pw->width_prompt) / (double)paper_height;

                    width = static_cast<unsigned int>(width_coeff * (double)wattr.width);
                    height = static_cast<unsigned int>(height_coeff * (double)wattr.height);
                    break;
		}
	    }
	    else
            {
		/* Portrait. */

                switch(pw->units)
                {
                  case PW_UNITS_CM:
                    x = static_cast<int>(PrintWinCmToPixels(
                        PromptGetF(&pw->x_prompt)
                        ) * (double)wattr.width / (double)paper_width);
                    y = static_cast<int>(PrintWinCmToPixels(
                        PromptGetF(&pw->y_prompt)
                        ) * (double)wattr.height / (double)paper_height);

                    width_coeff = PrintWinCmToPixels(
                        PromptGetF(&pw->width_prompt)) / (double)paper_width;
                    height_coeff = PrintWinCmToPixels(
                        PromptGetF(&pw->height_prompt)) / (double)paper_height;

                    width = static_cast<unsigned int>(width_coeff * (double)wattr.width); 
                    height = static_cast<unsigned int>(height_coeff * (double)wattr.height);
                    break;

                  case PW_UNITS_INCH:
                    x = static_cast<int>(PrintWinInchToPixels(
                        PromptGetF(&pw->x_prompt)
                        ) * (double)wattr.width / (double)paper_width);
                    y = static_cast<int>(PrintWinInchToPixels(
                        PromptGetF(&pw->y_prompt)
                        ) * (double)wattr.height / (double)paper_height);

                    width_coeff = PrintWinInchToPixels(
                        PromptGetF(&pw->width_prompt)) / (double)paper_width;
                    height_coeff = PrintWinInchToPixels(
                        PromptGetF(&pw->height_prompt)) / (double)paper_height;

                    width = static_cast<unsigned int>(width_coeff * (double)wattr.width);
                    height = static_cast<unsigned int>(height_coeff * (double)wattr.height);
                    break;

                  default:      /* PW_UNITS_PIXELS */
                    x = static_cast<int>(PromptGetF(&pw->x_prompt) * (double)wattr.width /
                        (double)paper_width);
                    y = static_cast<int>(PromptGetF(&pw->y_prompt) * (double)wattr.height /
                        (double)paper_height);

		    width_coeff = PromptGetF(&pw->width_prompt) / (double)paper_width;
		    height_coeff = PromptGetF(&pw->height_prompt) / (double)paper_height;

                    width = static_cast<unsigned int>(width_coeff * (double)wattr.width);
                    height = static_cast<unsigned int>(height_coeff * (double)wattr.height);
                    break;
                }
            }

	    /* Draw preview document. */
	    if((width > 0) &&
               (height > 0)
	    )
	    {
                /* Recreate preview image as needed. */
                if(pw->preview_img == NULL)
                {
                    img_ptr = PrintWinCreateSpool(pw);
		    if(img_ptr == NULL)
		    {
			fprintf(stderr,
 "PrintWinDraw(): Failed to create spool image.\n"
			);
			return(-1);
		    }

		    /* Rotate image. */
                    if(pw->orientation_tba.armed_tb == 1)
                    {
                        tmp_img_ptr = PrintWinRotateImage(
                            img_ptr,
                            osw_gui[0].depth
                        );
                        if(tmp_img_ptr == NULL)
                        {
                            OSWDestroyImage(&img_ptr);

                            fprintf(stderr,
 "PrintWinDraw(): Failed to rotate image.\n"
                            );

                            return(-1);
                        }

                        OSWDestroyImage(&img_ptr);

                        img_ptr = tmp_img_ptr;
                    }
 

		    if(OSWCreateImage(&pw->preview_img, width, height))
		    {
			OSWDestroyImage(&img_ptr);

                        fprintf(stderr,
 "PrintWinDraw(): Failed to create new image.\n"
                        );

			return(-1);
		    }

		    WidgetResizeImageBuffer(
			osw_gui[0].depth,
			reinterpret_cast<u_int8_t *>(pw->preview_img->data),	/* Target. */
			reinterpret_cast<u_int8_t *>(img_ptr->data),		/* Source. */
			pw->preview_img->width,
			pw->preview_img->height,
			img_ptr->width,
			img_ptr->height
		    );

		    OSWDestroyImage(&img_ptr);
		}

		OSWPutImageToDrawablePos(
		    pw->preview_img, pixmap,
		    x, y
		);

	        OSWSetFgPix((widget_global.force_mono) ?
		    osw_gui[0].black_pix : pw->paper_fg_pix
	        );
	        OSWDrawRectangle(
		    pixmap,
		    x, y, width, height
	        );
	    }

	    /* Draw frame. */
	    WidgetFrameButtonPixmap(
		pixmap,
		False,
		wattr.width, wattr.height,
		((widget_global.force_mono) ? osw_gui[0].white_pix :
		    widget_global.surface_highlight_pix),
		((widget_global.force_mono) ? osw_gui[0].black_pix :
                    widget_global.surface_shadow_pix)
	    );

            OSWPutBufferToWindow(w, pixmap);
	}



	OSWSetFont(prev_font);


        return(0);
}


int PrintWinManage(event_t *event)
{
	keycode_t keycode;
	int events_handled = 0;
        print_win_struct *pw;  
	int i, x, y, px, py;
	static int last_x, last_y;
	unsigned int paper_width, paper_height;
	win_attr_t wattr;

        pw = &print_win;



	if(event == NULL)
            return(events_handled);

        if(!pw->map_state &&
           (event->type != MapNotify)
	)
	    return(events_handled);


	switch(event->type)
	{
	  /* ******************************************************** */
	  case KeyPress:
	    if(!pw->is_in_focus)
		return(events_handled);

            keycode = event->xkey.keycode;


	    break;

          /* ******************************************************** */
          case KeyRelease:
            if(!pw->is_in_focus)
                return(events_handled);

            keycode = event->xkey.keycode;



            break;

          /* ******************************************************** */
          case ButtonPress:
	    if(event->xany.window == pw->preview)
	    {
		last_x = event->xbutton.x;
		last_y = event->xbutton.y;
		static_printwin::preview_drag_state = True;

                events_handled++;
                return(events_handled);
	    }
	    break;

          /* ******************************************************** */
          case ButtonRelease:
            if(event->xany.window == pw->preview)
            {
                last_x = event->xbutton.x;
                last_y = event->xbutton.y;
                static_printwin::preview_drag_state = False;

                events_handled++;
                return(events_handled);
            }
            break;

          /* ******************************************************** */
          case MotionNotify:
            if(event->xany.window == pw->preview)
            {
		if(static_printwin::preview_drag_state)
		{
		    OSWGetWindowAttributes(pw->preview, &wattr);
		    x = event->xmotion.x;
		    y = event->xmotion.y;

                    /* Get paper width and height (in pixels). */
                    PrintWinGetPaperSizePixels(pw, &paper_width, &paper_height);

		    switch(pw->units)
                    {
                      case PW_UNITS_CM:
                        px = static_cast<int>(PrintWinCmToPixels(
                            PromptGetF(&pw->x_prompt)
                        ));
                        py = static_cast<int>(PrintWinCmToPixels(
                            PromptGetF(&pw->y_prompt)
                        ));
 
                        PromptSetF(
                            &pw->x_prompt,
                            PrintWinPixelsToCm(
			        px + (((double)paper_width / (double)wattr.width) *
 				(x - last_x))
			    )
			);
                        PromptSetF(
                            &pw->y_prompt,
                            PrintWinPixelsToCm(
                                py + (((double)paper_height / (double)wattr.height) *
                                (y - last_y))
                            )
                        );
                        break;

                      case PW_UNITS_INCH:
                        px = static_cast<int>(PrintWinInchToPixels(
                            PromptGetF(&pw->x_prompt)
                        ));
                        py = static_cast<int>(PrintWinInchToPixels(
                            PromptGetF(&pw->y_prompt)
                        ));

                        PromptSetF(
                            &pw->x_prompt,
                            PrintWinPixelsToInch(
                                px + (((double)paper_width / (double)wattr.width) *
                                (x - last_x))
                            )
                        );    
                        PromptSetF(
                            &pw->y_prompt,
                            PrintWinPixelsToInch(
                                py + (((double)paper_height / (double)wattr.height) *
                                (y - last_y))
                            )
                        );
                        break;

                      default:      /* PW_UNITS_PIXELS */
                        px = PromptGetI(&pw->x_prompt);
                        py = PromptGetI(&pw->y_prompt);

                        PromptSetI(
                            &pw->x_prompt,
                            static_cast<int>(px + (((double)paper_width / (double)wattr.width) *
                                (double)(x - last_x)))
                        );
                        PromptSetI(
                            &pw->y_prompt,
                            static_cast<int>(py + (((double)paper_height / (double)wattr.height) *
                                (double)(y - last_y)))
                        );
                        break;
		    }

                    PrintWinDraw(PW_DRAW_AMOUNT_PREVIEW);


                    last_x = x;
                    last_y = y;
		}

                events_handled++;
                return(events_handled);
            }
            break;

          /* ******************************************************** */
	  case Expose:
	    if(event->xany.window == pw->toplevel)
	    {
		PrintWinDraw(PW_DRAW_AMOUNT_COMPLETE);

		events_handled++;
		return(events_handled);
	    }
	    break;

          /* ******************************************************** */
	  case FocusIn:
            if(event->xany.window == pw->toplevel)
            {
		pw->is_in_focus = 1;

                events_handled++;
                return(events_handled);
            }
            break;

          /* ******************************************************** */
          case FocusOut:
            if(event->xany.window == pw->toplevel)
            {
                pw->is_in_focus = 0;
         
                events_handled++;
                return(events_handled);
            }
            break;

          /* ******************************************************** */
          case UnmapNotify:
            if(event->xany.window == pw->toplevel)
            {
                PrintWinUnmap();

                events_handled++;
                return(events_handled);
            }
            break;
          
          /* ******************************************************** */
          case MapNotify:
            if(event->xany.window == pw->toplevel) 
            {
                if(!pw->map_state)
                    PrintWinMap();

                events_handled++;
                return(events_handled);
            }
            break;

          /* ******************************************************** */
          case ClientMessage:
            if(OSWIsEventDestroyWindow(pw->toplevel, event))
            {
		PrintWinCancelPBCB(&pw->cancel_btn);

	        events_handled++;
                return(events_handled);
            }
            break;
	}




	if(events_handled == 0)
            events_handled += PBtnManage(&pw->cancel_btn, event);
        if(events_handled == 0)
            events_handled += PBtnManage(&pw->print_btn, event);

        if(events_handled == 0)
	{
	    i = pw->orientation_tba.armed_tb;

            events_handled += TgBtnArrayManage(&pw->orientation_tba, event);
	    if((events_handled > 0) &&
	       (i != pw->orientation_tba.armed_tb)
	    )
	    {
	        OSWDestroyImage(&pw->preview_img);
		PrintWinDraw(PW_DRAW_AMOUNT_PREVIEW);
	    }
	}

        if(events_handled == 0)
        {
            i = pw->color_mode_tba.armed_tb;

            events_handled += TgBtnArrayManage(&pw->color_mode_tba, event);
            if((events_handled > 0) &&
               (i != pw->color_mode_tba.armed_tb)
            )
            {
                OSWDestroyImage(&pw->preview_img);
                PrintWinDraw(PW_DRAW_AMOUNT_PREVIEW);
            }
	}

        if(events_handled == 0)
        {
            i = pw->paper_size_tba.armed_tb;

	    events_handled += TgBtnArrayManage(&pw->paper_size_tba, event);
            if((events_handled > 0) &&
               (i != pw->paper_size_tba.armed_tb)
            )
            {
                OSWDestroyImage(&pw->preview_img);
                PrintWinDraw(PW_DRAW_AMOUNT_PREVIEW);
	    }
	} 

        if(events_handled == 0)
        {
            i = pw->obj_labels_tb.state;

            events_handled += TgBtnManage(&pw->obj_labels_tb, event);
            if((events_handled > 0) &&
               (i != pw->obj_labels_tb.state)
            )
            {
                OSWDestroyImage(&pw->preview_img);
                PrintWinDraw(PW_DRAW_AMOUNT_PREVIEW);
            }
        }
        if(events_handled == 0)
        {
            i = pw->label_geometry_tb.state;

            events_handled += TgBtnManage(&pw->label_geometry_tb, event);
            if((events_handled > 0) &&
               (i != pw->label_geometry_tb.state)
            )
            {
                OSWDestroyImage(&pw->preview_img);
                PrintWinDraw(PW_DRAW_AMOUNT_PREVIEW);
            }
        }

        if(events_handled == 0)
	{
            events_handled += PromptManage(&pw->x_prompt, event);
            if((events_handled > 0) &&
	       (event->type == KeyPress)
	    )
                PrintWinDraw(PW_DRAW_AMOUNT_PREVIEW);
        }
        if(events_handled == 0)
	{
            events_handled += PromptManage(&pw->y_prompt, event);
            if((events_handled > 0) &&
               (event->type == KeyPress)
            )   
                PrintWinDraw(PW_DRAW_AMOUNT_PREVIEW);
        }
        if(events_handled == 0)
	{
            events_handled += PromptManage(&pw->width_prompt, event);
            if((events_handled > 0) &&
               (event->type == KeyPress)
            )
	    {
		OSWDestroyImage(&pw->preview_img);
                PrintWinDraw(PW_DRAW_AMOUNT_PREVIEW);
	    }
        }
        if(events_handled == 0)
	{
            events_handled += PromptManage(&pw->height_prompt, event);
            if((events_handled > 0) &&
               (event->type == KeyPress)
            )
            {
                OSWDestroyImage(&pw->preview_img);
                PrintWinDraw(PW_DRAW_AMOUNT_PREVIEW);
	    }
        }

        if(events_handled == 0)
	{
	    i = pw->objects_pulist.sel_item;

            events_handled += PUListManage(&pw->objects_pulist, event);
            if((events_handled > 0) &&
               (i != pw->objects_pulist.sel_item)
            )
            {
                OSWDestroyImage(&pw->preview_img);
                PrintWinDraw(PW_DRAW_AMOUNT_PREVIEW);
            }
	}

        if(events_handled == 0)
            events_handled += PromptManage(&pw->spool_dir, event);
        if(events_handled == 0)
            events_handled += PromptManage(&pw->print_cmd_prompt, event);

        if(events_handled == 0)
            events_handled += TgBtnArrayManage(&pw->print_to_tba, event);



        return(events_handled);
}


void PrintWinMap()
{
        print_win_struct *pw;  
        
        pw = &print_win;


	pw->map_state = 0;
	PrintWinDraw(PW_DRAW_AMOUNT_COMPLETE);


        return;
}


/*
 *	Procedure to fetch values and set up relative to the source
 *	uew and map print window.
 */
void PrintWinDoMapValues(uew_struct *src_uew)
{
	int i;
	unsigned int paper_width, paper_height;
        print_win_struct *pw;  
	shared_image_t *img_ptr;
	uew_struct *uew_ptr = NULL;


        pw = &print_win;

	for(i = 0; i < total_uews; i++)
	{
	    if(uew[i] == NULL)
		continue;

	    if(src_uew == uew[i])
		uew_ptr = uew[i];
	}
	if(uew_ptr == NULL)
	    return;


	/* Get paper width and height (in pixels). */
        PrintWinGetPaperSizePixels(pw, &paper_width, &paper_height);

	/* Get values from uew. */
	pw->src = (void *)uew_ptr;

	if(uew_ptr->view_img != NULL)
	{
	    img_ptr = uew_ptr->view_img;

	    switch(pw->units)
	    {
	      case PW_UNITS_CM:
                PromptSetF(
                    &pw->width_prompt,
                    PrintWinPixelsToCm(img_ptr->width)
                );
                PromptSetF(
                    &pw->height_prompt,
                    PrintWinPixelsToCm(img_ptr->height)
                );

		if(pw->orientation_tba.armed_tb == 1)
		{
		    PromptSetF(&pw->x_prompt,
		        PrintWinPixelsToCm(
			    ((int)paper_width / 2) -
			    ((int)img_ptr->height / 2)
			)
		    );
                    PromptSetF(&pw->y_prompt,
                        PrintWinPixelsToCm(
                            ((int)paper_height / 2) -
                            ((int)img_ptr->width / 2)
			)
                    );
		}
		else
                {
                    PromptSetF(&pw->x_prompt,
                        PrintWinPixelsToCm(
                            ((int)paper_width / 2) - 
                            ((int)img_ptr->width / 2)  
                        )
                    );
                    PromptSetF(&pw->y_prompt,
                        PrintWinPixelsToCm(                
                            ((int)paper_height / 2) -
                            ((int)img_ptr->height / 2)
                        )
                    );   
                }
		break;

              case PW_UNITS_INCH:
                PromptSetF(
                    &pw->width_prompt,
                    PrintWinPixelsToInch(img_ptr->width)
                );  
                PromptSetF(
                    &pw->height_prompt,
                    PrintWinPixelsToInch(img_ptr->height)
                );

                if(pw->orientation_tba.armed_tb == 1)
                {
                    PromptSetF(&pw->x_prompt,
                        PrintWinPixelsToInch(
                            ((int)paper_width / 2) -
                            ((int)img_ptr->height / 2)
                        )
                    );
                    PromptSetF(&pw->y_prompt,
                        PrintWinPixelsToInch(
                            ((int)paper_height / 2) -
                            ((int)img_ptr->width / 2)
                        )
                    );
                }
                else
                {
                    PromptSetF(&pw->x_prompt,
                        PrintWinPixelsToInch(
                            ((int)paper_width / 2) -
                            ((int)img_ptr->width / 2)
                        )
                    );
                    PromptSetF(&pw->y_prompt,
                        PrintWinPixelsToInch(
                            ((int)paper_height / 2) -
                            ((int)img_ptr->height / 2)
                        )
                    );
                }
                break;

              default:	/* PW_UNITS_PIXELS */
                PromptSetI(
                    &pw->width_prompt,
                    img_ptr->width
                );
                PromptSetI(
                    &pw->height_prompt,
                    img_ptr->height
                );

                if(pw->orientation_tba.armed_tb == 1)
                {
                    PromptSetI(&pw->x_prompt,
                        ((int)paper_width / 2) -
                        ((int)img_ptr->height / 2)
                    );
                    PromptSetI(&pw->y_prompt,
                        ((int)paper_height / 2) -
                        ((int)img_ptr->width / 2)
                    );
                }
                else
                { 
                    PromptSetI(&pw->x_prompt,
                        ((int)paper_width / 2) -
                        ((int)img_ptr->width / 2)
                    );
                    PromptSetI(&pw->y_prompt,
                        ((int)paper_height / 2) -
                        ((int)img_ptr->height / 2)
                    );
                }
                break;
	    }
	}






	/* Map. */
	PrintWinMap();


        return;
}


void PrintWinUnmap()
{
        print_win_struct *pw;  
        
        pw = &print_win;


        PBtnUnmap(&pw->cancel_btn);
        PBtnUnmap(&pw->print_btn);

        TgBtnArrayUnmap(&pw->orientation_tba);
	TgBtnArrayUnmap(&pw->color_mode_tba);
        TgBtnArrayUnmap(&pw->paper_size_tba);

        TgBtnUnmap(&pw->label_geometry_tb);
        TgBtnUnmap(&pw->obj_labels_tb);

        PromptUnmap(&pw->x_prompt);
        PromptUnmap(&pw->y_prompt);
        PromptUnmap(&pw->width_prompt);
        PromptUnmap(&pw->height_prompt);

        PUListUnmap(&pw->objects_pulist);
        
        PromptUnmap(&pw->spool_dir);
        PromptUnmap(&pw->print_cmd_prompt);

        TgBtnArrayUnmap(&pw->print_to_tba);

        OSWDestroyImage(&pw->preview_img);

        OSWUnmapWindow(pw->toplevel);

	pw->map_state = 0;
	pw->is_in_focus = 0;


	/* Destroy large uneeded buffers. */
        OSWDestroyPixmap(&pw->toplevel_buf);



	return;
}



void PrintWinDestroy()
{
        print_win_struct *pw;  
        
        pw = &print_win;




	OSWDestroyPixel(&pw->paper_bg_pix);
        OSWDestroyPixel(&pw->paper_fg_pix);


	PBtnDestroy(&pw->cancel_btn);
        PBtnDestroy(&pw->print_btn);

	TgBtnArrayDestroy(&pw->orientation_tba);
	TgBtnArrayDestroy(&pw->color_mode_tba);
        TgBtnArrayDestroy(&pw->paper_size_tba);

        TgBtnDestroy(&pw->label_geometry_tb);
        TgBtnDestroy(&pw->obj_labels_tb);

	PromptDestroy(&pw->x_prompt);
	PromptDestroy(&pw->y_prompt);
        PromptDestroy(&pw->width_prompt);
        PromptDestroy(&pw->height_prompt);

	PUListDestroy(&pw->objects_pulist);

	PromptDestroy(&pw->spool_dir);
        PromptDestroy(&pw->print_cmd_prompt);

        TgBtnArrayDestroy(&pw->print_to_tba);

	OSWDestroyImage(&pw->preview_img);

	OSWDestroyWindow(&pw->preview);
        OSWDestroyWindow(&pw->toplevel);

	OSWDestroyPixmap(&pw->toplevel_buf);


	pw->map_state = 0;
	pw->is_in_focus = 0;
	pw->x = 0;
	pw->y = 0;
        pw->width = 0;
        pw->height = 0;

	pw->units = PW_UNITS_PIXELS;

	pw->src = NULL;


	return;
}



