// client/blitptlt.cpp
/*
                        Buffer Blitting: Point Light

	Functions:

	BlitBufPointLight8()
	BlitBufPointLight16()
	BlitBufPointLight32()
	BlitBufPointLight()

	---

 */

#include "blitting.h"

#ifndef MAX
#define MIN(a,b)        (((a) < (b)) ? (a) : (b))
#define MAX(a,b)	(((a) > (b)) ? (a) : (b))
#endif
/* #define MIN(a,b)        (((a) < (b)) ? (a) : (b)) */
/* #define MAX(a,b)        (((a) > (b)) ? (a) : (b)) */


#define DIAMETER		1	/* Change this later. */


void BlitBufPointLight8(
        u_int8_t *tar_buf,
        int tar_x, int tar_y,
        unsigned int tar_width, unsigned int tar_height,
        double radius,
        WColorStruct light_color
)
{
        int tar_x_col, tar_y_row;
        int tar_width_bytes;     /* tar_width * BYTES_PER_PIXEL8 */
        int tar_width_check, tar_height_check;

        u_int8_t *tar_buf_ptr;
  

        /* Error checks. */
        if((tar_buf == NULL) ||
           (tar_width < 1) ||   
           (tar_height < 1)
        )
            return;
   

        /* Check if off screen. */
        if((tar_x >= (int)tar_width) ||
           (tar_y >= (int)tar_height) ||
           ((tar_x + DIAMETER) < 0) ||
           ((tar_y + DIAMETER) < 0)
        )
            return;


        /* *********************************************************** */

        /* Calculate bytes per width. */
        tar_width_bytes = (int)tar_width * BYTES_PER_PIXEL8;

        /* Calculate check positions. */
        tar_width_check = tar_x + DIAMETER;
        tar_height_check = tar_y + DIAMETER;
 
        /* Sanitize starting coordinates. */
        if(tar_x < 0) tar_x = 0;
        if(tar_y < 0) tar_y = 0;

        /* Sanitize checks. */
        if(tar_width_check > (int)tar_width)
            tar_width_check = tar_width;
        if(tar_height_check > (int)tar_height)
            tar_height_check = tar_height;
           
        /* Set starting values. */
        tar_x_col = tar_x;
        tar_y_row = tar_y;
   

        /* Begin blitting to target buffer. */
        while(tar_y_row < tar_height_check)
        {
            /* Skip if off the screen. */
            if((tar_x_col >= (int)tar_width) ||
               (tar_y_row >= (int)tar_height)
            )
            {
                /* Next position. */
                tar_x_col++;
                if(tar_x_col >= tar_width_check)
                {
                    tar_x_col = tar_x;
                    tar_y_row++;
                }
                continue;
            }
 
            /* Get pointer in target buffer. */
            tar_buf_ptr = (u_int8_t *)(&tar_buf[
                (tar_y_row * tar_width_bytes) +
                (tar_x_col * BYTES_PER_PIXEL8)
            ]);
  
            *tar_buf_ptr = (u_int8_t)PACK8TO8(
                light_color.r,
                light_color.g,
                light_color.b
            );
        
        
            /* Next position. */
            tar_x_col++;
            if(tar_x_col >= tar_width_check)  
            {
                tar_x_col = tar_x;
                tar_y_row++;
            }
        }
             
             
        return;
}

void BlitBufPointLight15(
        u_int8_t *tar_buf,
        int tar_x, int tar_y,
        unsigned int tar_width, unsigned int tar_height,
        double radius,
        WColorStruct light_color
)
{
	int tar_x_col, tar_y_row;
        int tar_width_bytes;     /* tar_width * BYTES_PER_PIXEL16 */
	int tar_width_check, tar_height_check;

	u_int16_t *tar_buf_ptr;


        /* Error checks. */
        if((tar_buf == NULL) ||
           (tar_width < 1) ||
           (tar_height < 1)
        )
            return;


	/* Check if off screen. */
	if((tar_x >= (int)tar_width) ||
           (tar_y >= (int)tar_height) ||
           ((tar_x + DIAMETER) < 0) ||
           ((tar_y + DIAMETER) < 0)
	)
	    return;


	/* *********************************************************** */

	/* Calculate bytes per width. */
	tar_width_bytes = (int)tar_width * BYTES_PER_PIXEL15;

	/* Calculate check positions. */
	tar_width_check = tar_x + DIAMETER;
	tar_height_check = tar_y + DIAMETER;

	/* Sanitize starting coordinates. */
	if(tar_x < 0) tar_x = 0;
	if(tar_y < 0) tar_y = 0;

	/* Sanitize checks. */
	if(tar_width_check > (int)tar_width)
	    tar_width_check = tar_width;
	if(tar_height_check > (int)tar_height)
	    tar_height_check = tar_height;

	/* Set starting values. */
        tar_x_col = tar_x;
        tar_y_row = tar_y;   


	/* Begin blitting to target buffer. */
	while(tar_y_row < tar_height_check)
	{
            /* Skip if off the screen. */
            if((tar_x_col >= (int)tar_width) ||
               (tar_y_row >= (int)tar_height)
	    )
	    {
                /* Next position. */
		tar_x_col++;
		if(tar_x_col >= tar_width_check)
		{
		    tar_x_col = tar_x;
		    tar_y_row++;
		}
		continue;
	    }

            /* Get pointer in target buffer. */
            tar_buf_ptr = (u_int16_t *)(&tar_buf[  
                (tar_y_row * tar_width_bytes) +
                (tar_x_col * BYTES_PER_PIXEL15)
            ]);

	    *tar_buf_ptr = (u_int16_t)PACK8TO15(
		light_color.r,
		light_color.g,
		light_color.b
	    );


	    /* Next position. */
            tar_x_col++;
            if(tar_x_col >= tar_width_check)
            {
                tar_x_col = tar_x;
                tar_y_row++;
            }
	}


	return;
}


void BlitBufPointLight16(
        u_int8_t *tar_buf,
        int tar_x, int tar_y,
        unsigned int tar_width, unsigned int tar_height,
        double radius,
        WColorStruct light_color
)
{
	int tar_x_col, tar_y_row;
        int tar_width_bytes;     /* tar_width * BYTES_PER_PIXEL16 */
	int tar_width_check, tar_height_check;

	u_int16_t *tar_buf_ptr;


        /* Error checks. */
        if((tar_buf == NULL) ||
           (tar_width < 1) ||
           (tar_height < 1)
        )
            return;


	/* Check if off screen. */
	if((tar_x >= (int)tar_width) ||
           (tar_y >= (int)tar_height) ||
           ((tar_x + DIAMETER) < 0) ||
           ((tar_y + DIAMETER) < 0)
	)
	    return;


	/* *********************************************************** */

	/* Calculate bytes per width. */
	tar_width_bytes = (int)tar_width * BYTES_PER_PIXEL16;

	/* Calculate check positions. */
	tar_width_check = tar_x + DIAMETER;
	tar_height_check = tar_y + DIAMETER;

	/* Sanitize starting coordinates. */
	if(tar_x < 0) tar_x = 0;
	if(tar_y < 0) tar_y = 0;

	/* Sanitize checks. */
	if(tar_width_check > (int)tar_width)
	    tar_width_check = tar_width;
	if(tar_height_check > (int)tar_height)
	    tar_height_check = tar_height;

	/* Set starting values. */
        tar_x_col = tar_x;
        tar_y_row = tar_y;   


	/* Begin blitting to target buffer. */
	while(tar_y_row < tar_height_check)
	{
            /* Skip if off the screen. */
            if((tar_x_col >= (int)tar_width) ||
               (tar_y_row >= (int)tar_height)
	    )
	    {
                /* Next position. */
		tar_x_col++;
		if(tar_x_col >= tar_width_check)
		{
		    tar_x_col = tar_x;
		    tar_y_row++;
		}
		continue;
	    }

            /* Get pointer in target buffer. */
            tar_buf_ptr = (u_int16_t *)(&tar_buf[  
                (tar_y_row * tar_width_bytes) +
                (tar_x_col * BYTES_PER_PIXEL16)
            ]);

	    *tar_buf_ptr = (u_int16_t)PACK8TO16(
		light_color.r,
		light_color.g,
		light_color.b
	    );


	    /* Next position. */
            tar_x_col++;
            if(tar_x_col >= tar_width_check)
            {
                tar_x_col = tar_x;
                tar_y_row++;
            }
	}


	return;
}


void BlitBufPointLight32(
        u_int8_t *tar_buf,
        int tar_x, int tar_y,
        unsigned int tar_width, unsigned int tar_height,
        double radius,
        WColorStruct light_color
)
{
        int tar_x_col, tar_y_row;
        int tar_width_bytes;     /* tar_width * BYTES_PER_PIXEL32 */
        int tar_width_check, tar_height_check;

        u_int32_t *tar_buf_ptr;


        /* Error checks. */
        if((tar_buf == NULL) ||
           (tar_width < 1) ||
           (tar_height < 1)
        )
            return;


        /* Check if off screen. */
        if((tar_x >= (int)tar_width) ||
           (tar_y >= (int)tar_height) ||
           ((tar_x + DIAMETER) < 0) ||
           ((tar_y + DIAMETER) < 0)
        )
            return;


        /* *********************************************************** */
               
        /* Calculate bytes per width. */
        tar_width_bytes = (int)tar_width * BYTES_PER_PIXEL32;

        /* Calculate check positions. */
        tar_width_check = tar_x + DIAMETER;
        tar_height_check = tar_y + DIAMETER;
                
        /* Sanitize starting coordinates. */
        if(tar_x < 0) tar_x = 0;
        if(tar_y < 0) tar_y = 0;
            
        /* Sanitize checks. */
        if(tar_width_check > (int)tar_width)   
            tar_width_check = tar_width;
        if(tar_height_check > (int)tar_height)
            tar_height_check = tar_height;
             
        /* Set starting values. */
        tar_x_col = tar_x;   
        tar_y_row = tar_y;


        /* Begin blitting to target buffer. */
        while(tar_y_row < tar_height_check)
        {
            /* Skip if off the screen. */
            if((tar_x_col >= (int)tar_width) ||
               (tar_y_row >= (int)tar_height)
            )
            {
                /* Next position. */
                tar_x_col++;  
                if(tar_x_col >= tar_width_check)
                {
                    tar_x_col = tar_x;
                    tar_y_row++;
                }
                continue;
            }


            /* Get pointer in target buffer. */
            tar_buf_ptr = (u_int32_t *)(&tar_buf[
                (tar_y_row * tar_width_bytes) +
                (tar_x_col * BYTES_PER_PIXEL32)
            ]);

            *tar_buf_ptr = (u_int32_t)PACK8TO32(
                light_color.a,
                light_color.r,
                light_color.g,
                light_color.b
            );
        
        
            /* Next position. */
            tar_x_col++;
            if(tar_x_col >= tar_width_check)
            {
                tar_x_col = tar_x;
                tar_y_row++;
            }
        }


	return;
}


void BlitBufPointLight(
        unsigned int d,
        u_int8_t *tar_buf,
        int tar_x, int tar_y,
        unsigned int tar_width, unsigned int tar_height,
        double radius,
        WColorStruct light_color
)
{
        switch(d)
        {
          case 32:
            BlitBufPointLight32(
                tar_buf,
                tar_x,
                tar_y,
                tar_width,
                tar_height,
                radius,
		light_color
            );
            break;
 
          case 24:
            BlitBufPointLight32(
                tar_buf,
                tar_x,
                tar_y,
                tar_width,
                tar_height,
                radius,
                light_color
            );
            break;

          case 16:
            BlitBufPointLight16(
                tar_buf,
                tar_x,
                tar_y,
                tar_width,
                tar_height,
                radius,
                light_color
            );
            break;

          case 15:
            BlitBufPointLight15(
                tar_buf,
                tar_x,
                tar_y,
                tar_width,
                tar_height,
                radius,
                light_color
            );
            break;

          case 8:
            BlitBufPointLight8(
                tar_buf,
                tar_x,
                tar_y,
                tar_width,
                tar_height,
                radius,
                light_color
            );
            break;
	}


	return;
}




