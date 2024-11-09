/*
               Image library (Imlib) operating system wrapper


	Functions:

	int IMLIBOSWInit()
	int IMLIBOSWLoadFileToImage(char *filename, image_t **image)
	void IMLIBOSWShutdown()




 */

#ifdef USE_IMLIB

#include <stdio.h>
#include <db.h>

#include <Imlib.h>

#include "../include/graphics.h"
#include "../include/osw-x.h"
#include "../include/Imlibosw.h"

static imlib_resource_struct imlib_resource;

#ifdef IMLIB_NEED_EVEN_ODD_FUNC

int IS_NUM_EVEN(int n)
{
	int x;
        char str[64];
	char *strptr;


        sprintf(str, "%.1lf", (double)((double)n / 2));
        strptr = strchr(str, '.');
        x = (strptr == NULL) ? 0 : atoi(strptr + 1);

        if(x > 0)
            return(0);  /* Odd. */
        else
            return(1);  /* Even. */
}



int IS_NUM_ODD(int n)
{
        static int x;        
        char str[64];
        static char *strptr;
 

        sprintf(str, "%.1lf", (double)((double)n / 2));
        strptr = strchr(str, '.');
        x = (strptr == NULL) ? 0 : atoi(strptr + 1);
        
        if(x > 0)
            return(1);  /* Odd. */
        else
            return(0);  /* Even. */
}

#endif /* IMLIB_NEED_EVEN_ODD_FUNC */



/* *********************************************************************** */

/*
 *	Initialize Imlib, must be called right after display is
 *	connected to GUI.
 */
int IMLIBOSWInit()
{
	/* Check if display is connected. */
	if(!IDC())
	{
	    fprintf(stderr,
		"IMLIBOSWInit(): GUI display not connected.\n"
	    );
	    return(-1);
	}

	/* Initialize Imlib. */
        imlib_resource.imlib_data = Imlib_init(osw_gui[0].display);
        if(imlib_resource.imlib_data == NULL)
        {
	    /* Failed to initialize Imlib. */
            fprintf(stderr,   
                "IMLIBOSWInit(): Unable to initialize Imlib.\n"
            );
	    return(-1);
        }


	/* Mark Imlib as initialized. */
	imlib_resource.is_init = 1;


	return(0);
}



/*
 *	Load from a file to a GUI native image.
 */
int IMLIBOSWLoadFileToImage(char *filename, image_t **image)
{
	int col_pos, row_pos;		/* In pixels. */
	char width_odd, height_odd;	/* 1 if true. */
	unsigned int width, height;
	image_t *tar_image;

        u_int8_t r = 0, g = 0, b = 0;

        u_int8_t *src_buf_ptr;

        u_int8_t *tar_buf_ptr8;
        u_int16_t *tar_buf_ptr16;
        u_int32_t *tar_buf_ptr32;

        ImlibImage *imlib_image;


	/* Error checks. */
	if(!imlib_resource.is_init)
	{
	    fprintf(stderr,
		"IMLIBOSWLoadFileToImage(): Imlib is not initialized.\n"
	    );
	    return(-1);
	}
	/* Input error checks. */
	if((filename == NULL) ||
           (image == NULL)
	)
	    return(-1);


	/* ************************************************************* */
	*image = NULL;

	/* Load image using Imlib. */
	imlib_image = Imlib_load_image(
	    imlib_resource.imlib_data,
	    filename
	);
	if(imlib_image == NULL)
	    return(-1);


        /* Get image values. */
        width = imlib_image->rgb_width;
        height = imlib_image->rgb_height;


	/* Odd number check. */
	width_odd = (IS_NUM_ODD(width)) ? 1 : 0;
	height_odd = (IS_NUM_ODD(height)) ? 1 : 0;


	/* Create GUI native image and copy over to. */
	if(OSWCreateImage(&tar_image, width, height))
	{
	    Imlib_destroy_image(imlib_resource.imlib_data, imlib_image);
	    return(-1);
	}


	/*   Note: the GUI native image will (should) always be
         *   even-numbered in dimension.
	 */

	/* Format and copy over to GUI native image. */

	/* 8 bits. */
	if(osw_gui[0].depth == 8)
	{
	    /* Get source buffer pointer, always u_int8_t. */
	    src_buf_ptr = (u_int8_t *)imlib_image->rgb_data;
	    tar_buf_ptr8 = (u_int8_t *)tar_image->data;

	    col_pos = 0;
	    row_pos = 0;
	    while(row_pos < (int)height)
	    {
                r = *src_buf_ptr++;
                g = *src_buf_ptr++;
                b = *src_buf_ptr++;

		*tar_buf_ptr8++ = PACK8TO8(r, g, b);

		col_pos += 1;
		if(col_pos >= (int)width)
		{
		    col_pos = 0;
		    row_pos += 1;

		    /* Add extra pixel on target if width is odd. */
		    if(width_odd)
			*tar_buf_ptr8++ = PACK8TO8(r, g, b);
		}
	    }

	    /* Add extra line if height is odd. */
	    if(height_odd)
	    {
                src_buf_ptr = (u_int8_t *)(&imlib_image->rgb_data[
		    (int)((int)height - 1) * 3
		]);
                tar_buf_ptr8 = (u_int8_t *)(&tar_image->data[
		    (int)height * 3
		]);

		col_pos = 0;
		while(col_pos < (int)width)
		{
                    r = *src_buf_ptr++;
                    g = *src_buf_ptr++;
                    b = *src_buf_ptr++;

                    *tar_buf_ptr8++ = PACK8TO8(r, g, b);

		    col_pos += 1;
		}
                if(width_odd)
                    *tar_buf_ptr8++ = PACK8TO8(r, g, b);
	    }

	}
        /* 16 bits. */
        else if(osw_gui[0].depth == 16)
        {
            /* Get source buffer pointer, always u_int8_t. */
            src_buf_ptr = (u_int8_t *)imlib_image->rgb_data;
            tar_buf_ptr16 = (u_int16_t *)tar_image->data;
                 
            col_pos = 0;
            row_pos = 0;
            while(row_pos < (int)height)
            {
                r = *src_buf_ptr++;
                g = *src_buf_ptr++;
                b = *src_buf_ptr++;
                   
                *tar_buf_ptr16++ = PACK8TO16(r, g, b);
                    
                col_pos += 1;
                if(col_pos >= (int)width)
                {
                    col_pos = 0;
                    row_pos += 1;
                    
                    /* Add extra pixel on target if width is odd. */
                    if(width_odd)
                        *tar_buf_ptr16++ = PACK8TO16(r, g, b);
                }
            }   
                    
            /* Add extra line if height is odd. */
            if(height_odd)   
            {
                src_buf_ptr = (u_int8_t *)(&imlib_image->rgb_data[
                    (int)((int)height - 1) * 3
                ]);
                tar_buf_ptr16 = (u_int16_t *)(&tar_image->data[
                    (int)height * 3
                ]);

                col_pos = 0;
                while(col_pos < (int)width)
                {
                    r = *src_buf_ptr++;
                    g = *src_buf_ptr++;
                    b = *src_buf_ptr++; 
             
                    *tar_buf_ptr16++ = PACK8TO16(r, g, b);
                
                    col_pos += 1;  
                }
                if(width_odd)
                    *tar_buf_ptr16++ = PACK8TO16(r, g, b);
            }
                
        }
        /* 24 or 32 bits. */
        else if((osw_gui[0].depth == 24) ||
                (osw_gui[0].depth == 32)
	)
        {
            /* Get source buffer pointer, always u_int8_t. */
            src_buf_ptr = (u_int8_t *)imlib_image->rgb_data;
            tar_buf_ptr32 = (u_int32_t *)tar_image->data;
                    
            col_pos = 0;
            row_pos = 0;
            while(row_pos < (int)height)
            {
                r = *src_buf_ptr++;
                g = *src_buf_ptr++;
                b = *src_buf_ptr++;
                
                *tar_buf_ptr32++ = PACK8TO32(0x00, r, g, b);
             
                col_pos += 1;
                if(col_pos >= (int)width)
                {   
                    col_pos = 0;
                    row_pos += 1;
                
                    /* Add extra pixel on target if width is odd. */
                    if(width_odd)
                        *tar_buf_ptr32++ = PACK8TO32(0x00, r, g, b);
                }
            }
            /* Add extra line if height is odd. */
            if(height_odd)
            {
                src_buf_ptr = (u_int8_t *)(&imlib_image->rgb_data[
                    (int)((int)height - 1) * 3
                ]);
                tar_buf_ptr32 = (u_int32_t *)(&tar_image->data[
                    (int)height * 3
                ]);
                 
                col_pos = 0;
                while(col_pos < (int)width)
                {
                    r = *src_buf_ptr++;
                    g = *src_buf_ptr++;
                    b = *src_buf_ptr++;
                 
                    *tar_buf_ptr32++ = PACK8TO32(0x00, r, g, b);
        
                    col_pos += 1;
                }
                if(width_odd)
                    *tar_buf_ptr32++ = PACK8TO32(0x00, r, g, b);
            }
        }




	/* ************************************************************ */

	/* Destroy Imlib image. */
        Imlib_destroy_image(imlib_resource.imlib_data, imlib_image);

	/* Set input image pointer to tar_image. */
	*image = tar_image;


	return(0);
}



/*
 *	Shutdown Imlib.
 */
void IMLIBOSWShutdown()
{
	/* Imlib does not need to shutdown. */


	return;
}





#endif /* USE_IMLIB */
