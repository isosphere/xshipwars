// client/blitcursor.cpp
/*
                        Buffer Blitting: Cursor

	Functions:

	BlitBufCursor8()
	BlitBufCursor15()
	BlitBufCursor16()
	BlitBufCursor32()
	BlitBufCursor()

	---

	These functions perform blitting of two buffers.
	A porition of the source buffer to a position on the destination
	buffer with `zooming' support.

	Zooming is currently limited to 0.01 to 1.00 ranges.
 */

#include "blitting.h"

#ifndef MAX
#define MIN(a,b)        (((a) < (b)) ? (a) : (b))
#define MAX(a,b)	(((a) > (b)) ? (a) : (b))
#endif
/* #define MIN(a,b)        (((a) < (b)) ? (a) : (b)) */
/* #define MAX(a,b)        (((a) > (b)) ? (a) : (b)) */



void BlitBufCursor8(
        u_int8_t *tar_buf,
        u_int8_t *src_buf,
        int tar_x,
        int tar_y,
        unsigned int tar_width,
        unsigned int tar_height,
        unsigned int src_width,
        unsigned int src_height,
        WColorStruct color
)
{
        int tar_x_col, tar_y_row;
        int src_x_col, src_y_row;

        int tar_width_bytes;   /* tar_width * BYTES_PER_PIXEL8 */
        int src_width_bytes;   /* src_width * BYTES_PER_PIXEL8 */

        u_int8_t *tar_buf_ptr;
        u_int8_t *src_buf_ptr;

        int r, g, b;


        /* Error checks. */
        if((tar_buf == NULL) ||
           (src_buf == NULL) ||
           (tar_width < 1) ||
           (tar_height < 1) ||
           (src_width < 1) ||
           (src_height < 1)
        )
            return;


        /* Source totally not visable on target? */
        if((tar_x >= (int)tar_width) ||
           (tar_y >= (int)tar_height) ||
           ((tar_x + (int)src_width) < 0) ||
           ((tar_y + (int)src_height) < 0)
        )
            return;


        /* *********************************************************** */

        /* Set up starting numbers. */
        tar_x_col = tar_x;
        tar_y_row = tar_y;
        src_x_col = 0;
        src_y_row = 0;


        /* Precalculate some frequently used values. */
        tar_width_bytes = (int)tar_width * BYTES_PER_PIXEL8;
        src_width_bytes = (int)src_width * BYTES_PER_PIXEL8;


        /* Begin blitting src_buf to tar_buf. */
        while(src_y_row < (int)src_height)
        {
            /* Skip if off the screen. */
            if((tar_x_col < 0) ||
               (tar_y_row < 0) ||
               (tar_x_col >= (int)tar_width) ||
               (tar_y_row >= (int)tar_height)
            )
            {
                /* Increment x colum positions. */
                src_x_col++;
                tar_x_col++;

                /* Increment y row positions. */
                if(src_x_col >= (int)src_width)
                {
                    src_x_col = 0;
                    src_y_row++;

                    tar_x_col = tar_x;
                    tar_y_row++;
                }

                continue;
            }


            /* Get source buf pointer. */
            src_buf_ptr = (u_int8_t *)(&src_buf[
                (src_y_row * src_width_bytes) +
                (src_x_col * BYTES_PER_PIXEL8)
            ]);

            /*
             *   Transparency check.  If all values are 0, then skip.
             */
            if(*src_buf_ptr == 0x00)
            {
                /* Increment x colum positions. */
                src_x_col++;
                tar_x_col++;

                /* Increment y row positions. */
                if(src_x_col >= (int)src_width)
                {
                    src_x_col = 0;
                    src_y_row++;

                    tar_x_col = tar_x;
                    tar_y_row++;
                }
                continue;
            }

            /* Get target buf pointer. */
            tar_buf_ptr = (u_int8_t *)(&tar_buf[
                (tar_y_row * tar_width_bytes) +
                (tar_x_col * BYTES_PER_PIXEL8)
            ]);


            /* Average source to buffer. */
            r = (int)(
                (((((*src_buf_ptr & 0xE0) >> 5) * color.r) >> 8) +
                ((*tar_buf_ptr & 0xE0) >> 5)) << 3)
                / 2;
            g = (int)(
                (((((*src_buf_ptr & 0x1C) >> 2) * color.g) >> 8) +
                ((*tar_buf_ptr & 0x1C) >> 2)) << 2)
                / 2;
            b = (int)(
                ((((*src_buf_ptr & 0x03) * color.b) >> 8) +
                (*tar_buf_ptr & 0x03)) << 2)
                / 2;
            *tar_buf_ptr = PACK8TO8((char)r, (char)g, (char)b);


            /* Increment x colum positions. */
            src_x_col++;
            tar_x_col++;

            /* Increment y row positions. */
            if(src_x_col >= (int)src_width)
            {
                src_x_col = 0;
                src_y_row++;

                tar_x_col = tar_x;
                tar_y_row++;
            }
        }


        return;
}


void BlitBufCursor15(
        u_int8_t *tar_buf,
        u_int8_t *src_buf,
        int tar_x,
        int tar_y,
        unsigned int tar_width,
        unsigned int tar_height,
        unsigned int src_width,
        unsigned int src_height,
	WColorStruct color
)
{
        int tar_x_col, tar_y_row;
        int src_x_col, src_y_row;

        int tar_width_bytes;	/* tar_width * BYTES_PER_PIXEL15 */
        int src_width_bytes;	/* src_width * BYTES_PER_PIXEL15 */

        u_int16_t *tar_buf_ptr;
        u_int16_t *src_buf_ptr;


        /* Error checks. */
        if((tar_buf == NULL) ||
           (src_buf == NULL) ||
           (tar_width < 1) ||
           (tar_height < 1) ||
           (src_width < 1) ||
           (src_height < 1)
        )
            return;


        /* Source totally not visable on target? */
        if((tar_x >= (int)tar_width) ||
           (tar_y >= (int)tar_height) ||
           ((tar_x + (int)src_width) < 0) ||
           ((tar_y + (int)src_height) < 0)
        )
            return;


        /* *********************************************************** */

        /* Set up starting numbers. */
        tar_x_col = tar_x;
        tar_y_row = tar_y;
        src_x_col = 0;
        src_y_row = 0;


	/* Precalculate some frequently used values. */
        tar_width_bytes = (int)tar_width * BYTES_PER_PIXEL15;
        src_width_bytes = (int)src_width * BYTES_PER_PIXEL15;


        /* Begin blitting src_buf to tar_buf. */
        while(src_y_row < (int)src_height)
        {
            /* Skip if off the screen. */
            if((tar_x_col < 0) ||
               (tar_y_row < 0) ||
               (tar_x_col >= (int)tar_width) ||
               (tar_y_row >= (int)tar_height)
	    )
	    {
		/* Increment x colum positions. */
		src_x_col++;
		tar_x_col++;

		/* Increment y row positions. */
                if(src_x_col >= (int)src_width)
                {
                    src_x_col = 0;
                    src_y_row++;

                    tar_x_col = tar_x;   
                    tar_y_row++;
                }
                    
                continue;
	    }


	    /* Get source buf pointer. */
            src_buf_ptr = (u_int16_t *)(&src_buf[
                (src_y_row * src_width_bytes) +
                (src_x_col * BYTES_PER_PIXEL15)
            ]);

            /*
             *   Transparency check.  If all values are 0, then skip.
             */
            if(*src_buf_ptr == 0x0000)
	    {
                /* Increment x colum positions. */
                src_x_col++;
                tar_x_col++;

                /* Increment y row positions. */
                if(src_x_col >= (int)src_width)
                {
                    src_x_col = 0;
                    src_y_row++;

                    tar_x_col = tar_x;
                    tar_y_row++;
                }
                continue;
	    }

            /* Get target buf pointer. */
            tar_buf_ptr = (u_int16_t *)(&tar_buf[
                (tar_y_row * tar_width_bytes) +
                (tar_x_col * BYTES_PER_PIXEL15)
            ]);

/*
urrr rrgg gggb bbbb

Duff: WTF do those ending bitshifts do?

Taura: The `>> 8' is to decrease the multiple effect that *
 the color value (a value from 0 to 255) caused)

*/
	    /* Average source to buffer. */
	    *tar_buf_ptr = PACK8TO15(
		(u_int8_t)(
((((u_int32_t)((*src_buf_ptr & 0x7C00) >> 7) * (u_int32_t)color.r) >> 8) +
(u_int32_t)((*tar_buf_ptr & 0x7C00) >> 7)) / 2
		),
		(u_int8_t)(
((((u_int32_t)((*src_buf_ptr & 0x03E0) >> 2) * (u_int32_t)color.g) >> 8) +
(u_int32_t)((*tar_buf_ptr & 0x03E0) >> 2)) / 2
                ),
                (u_int8_t)(
((((u_int32_t)((*src_buf_ptr & 0x001F) << 3) * (u_int32_t)color.b) >> 8) +
(u_int32_t)((*tar_buf_ptr & 0x001F) << 3)) / 2
		)
	    );


            /* Increment x colum positions. */
            src_x_col++;
            tar_x_col++;

            /* Increment y row positions. */
            if(src_x_col >= (int)src_width)
            {   
                src_x_col = 0;
                src_y_row++;

                tar_x_col = tar_x;
                tar_y_row++;
            }
	}


	return;
}


void BlitBufCursor16(
        u_int8_t *tar_buf,
        u_int8_t *src_buf,
        int tar_x,
        int tar_y,
        unsigned int tar_width,
        unsigned int tar_height,
        unsigned int src_width,
        unsigned int src_height,
	WColorStruct color
)
{
        int tar_x_col, tar_y_row;
        int src_x_col, src_y_row;

        int tar_width_bytes;	/* tar_width * BYTES_PER_PIXEL16 */
        int src_width_bytes;	/* src_width * BYTES_PER_PIXEL16 */

        u_int16_t *tar_buf_ptr;
        u_int16_t *src_buf_ptr;


        /* Error checks. */
        if((tar_buf == NULL) ||
           (src_buf == NULL) ||
           (tar_width < 1) ||
           (tar_height < 1) ||
           (src_width < 1) ||
           (src_height < 1)
        )
            return;


        /* Source totally not visable on target? */
        if((tar_x >= (int)tar_width) ||
           (tar_y >= (int)tar_height) ||
           ((tar_x + (int)src_width) < 0) ||
           ((tar_y + (int)src_height) < 0)
        )
            return;


        /* *********************************************************** */

        /* Set up starting numbers. */
        tar_x_col = tar_x;
        tar_y_row = tar_y;
        src_x_col = 0;
        src_y_row = 0;


	/* Precalculate some frequently used values. */
        tar_width_bytes = (int)tar_width * BYTES_PER_PIXEL16;
        src_width_bytes = (int)src_width * BYTES_PER_PIXEL16;


        /* Begin blitting src_buf to tar_buf. */
        while(src_y_row < (int)src_height)
        {
            /* Skip if off the screen. */
            if((tar_x_col < 0) ||
               (tar_y_row < 0) ||
               (tar_x_col >= (int)tar_width) ||
               (tar_y_row >= (int)tar_height)
	    )
	    {
		/* Increment x colum positions. */
		src_x_col++;
		tar_x_col++;

		/* Increment y row positions. */
                if(src_x_col >= (int)src_width)
                {
                    src_x_col = 0;
                    src_y_row++;

                    tar_x_col = tar_x;   
                    tar_y_row++;
                }
                    
                continue;
	    }


	    /* Get source buf pointer. */
            src_buf_ptr = (u_int16_t *)(&src_buf[
                (src_y_row * src_width_bytes) +
                (src_x_col * BYTES_PER_PIXEL16)
            ]);

            /*
             *   Transparency check.  If all values are 0, then skip.
             */
            if(*src_buf_ptr == 0x0000)
	    {
                /* Increment x colum positions. */
                src_x_col++;
                tar_x_col++;

                /* Increment y row positions. */
                if(src_x_col >= (int)src_width)
                {
                    src_x_col = 0;
                    src_y_row++;

                    tar_x_col = tar_x;
                    tar_y_row++;
                }
                continue;
	    }

            /* Get target buf pointer. */
            tar_buf_ptr = (u_int16_t *)(&tar_buf[
                (tar_y_row * tar_width_bytes) +
                (tar_x_col * BYTES_PER_PIXEL16)
            ]);

/*
rrrr rggg gggb bbbb
*/

            /* Average source to buffer. */
            *tar_buf_ptr = PACK8TO16(
                (u_int8_t)(
((((u_int32_t)((*src_buf_ptr & 0xF800) >> 8) * (u_int32_t)color.r) >> 8) +
(u_int32_t)((*tar_buf_ptr & 0xF800) >> 8)) / 2
                ),
                (u_int8_t)(
((((u_int32_t)((*src_buf_ptr & 0x07E0) >> 3) * (u_int32_t)color.g) >> 8) +
(u_int32_t)((*tar_buf_ptr & 0x07E0) >> 3)) / 2
                ),
                (u_int8_t)(
((((u_int32_t)((*src_buf_ptr & 0x001F) << 3) * (u_int32_t)color.b) >> 8) +
(u_int32_t)((*tar_buf_ptr & 0x001F) << 3)) / 2
                )
            );


            /* Increment x colum positions. */
            src_x_col++;
            tar_x_col++;

            /* Increment y row positions. */
            if(src_x_col >= (int)src_width)
            {   
                src_x_col = 0;
                src_y_row++;

                tar_x_col = tar_x;
                tar_y_row++;
            }
	}


	return;
}



void BlitBufCursor32(
        u_int8_t *tar_buf,
        u_int8_t *src_buf,
        int tar_x,
        int tar_y,
        unsigned int tar_width,
        unsigned int tar_height,
        unsigned int src_width,
        unsigned int src_height,
        WColorStruct color
)
{
        int tar_x_col, tar_y_row;
        int src_x_col, src_y_row;

        int tar_width_bytes;   /* tar_width * BYTES_PER_PIXEL32 */
        int src_width_bytes;   /* src_width * BYTES_PER_PIXEL32 */

        u_int32_t *tar_buf_ptr;
        u_int32_t *src_buf_ptr;

	u_int32_t r, g, b;


        /* Error checks. */ 
        if((tar_buf == NULL) ||
           (src_buf == NULL) ||
           (tar_width < 1) ||
           (tar_height < 1) ||
           (src_width < 1) ||
           (src_height < 1)
        )
            return;


        /* Source totally not visable on target? */
        if((tar_x >= (int)tar_width) ||
           (tar_y >= (int)tar_height) ||
           ((tar_x + (int)src_width) < 0) ||
           ((tar_y + (int)src_height) < 0)
        )
	    return;


        /* *********************************************************** */

        /* Set up starting numbers. */
        tar_x_col = tar_x;
        tar_y_row = tar_y;
        src_x_col = 0;
        src_y_row = 0;
                
   
        /* Precalculate some frequently used values. */
        tar_width_bytes = (int)tar_width * BYTES_PER_PIXEL32;
        src_width_bytes = (int)src_width * BYTES_PER_PIXEL32;
                    
                    
        /* Begin blitting src_buf to tar_buf. */
        while(src_y_row < (int)src_height)
        {
            /* Skip if off the screen. */
            if((tar_x_col < 0) ||
               (tar_y_row < 0) ||
               (tar_x_col >= (int)tar_width) ||
               (tar_y_row >= (int)tar_height)
            )
            {
                /* Increment x colum positions. */
                src_x_col++;
                tar_x_col++;
               
                /* Increment y row positions. */
                if(src_x_col >= (int)src_width)
                {
                    src_x_col = 0;
                    src_y_row++;
             
                    tar_x_col = tar_x;
                    tar_y_row++;
                }
             
                continue;
            }
                 
                    
            /* Get source buf pointer. */
            src_buf_ptr = (u_int32_t *)(&src_buf[
                (src_y_row * src_width_bytes) +
                (src_x_col * BYTES_PER_PIXEL32)
            ]);  
             
            /*
             *   Transparency check.  If all values are 0, then skip.
             */
            if(*src_buf_ptr == 0x00000000)
            {
                /* Increment x colum positions. */
                src_x_col++;
                tar_x_col++;
 
                /* Increment y row positions. */
                if(src_x_col >= (int)src_width)
                {
                    src_x_col = 0;
                    src_y_row++;
            
                    tar_x_col = tar_x;
                    tar_y_row++;
                }   
                continue;
            }
                
            /* Get target buf pointer. */
            tar_buf_ptr = (u_int32_t *)(&tar_buf[
                (tar_y_row * tar_width_bytes) +
                (tar_x_col * BYTES_PER_PIXEL32)
            ]);


            /* Average source to buffer. */
            r = (u_int32_t)(
                ((((*src_buf_ptr & 0x00FF0000) >> 16) * color.r) >> 8) +
                ((*tar_buf_ptr & 0x00FF0000) >> 16))
                / 2;
            g = (u_int32_t)(
                ((((*src_buf_ptr & 0x0000FF00) >> 8) * color.g) >> 8) +
                ((*tar_buf_ptr & 0x0000FF00) >> 8))
                / 2;
            b = (u_int32_t)(
                (((*src_buf_ptr & 0x000000FF) * color.b) >> 8) +
                (*tar_buf_ptr & 0x000000FF))
                / 2;
            *tar_buf_ptr = PACK8TO32(
		(u_int8_t)0x00,
                (u_int8_t)r,
		(u_int8_t)g,
		(u_int8_t)b
	    );
 
        
            /* Increment x colum positions. */
            src_x_col++;
            tar_x_col++;

            /* Increment y row positions. */
            if(src_x_col >= (int)src_width)
            {
                src_x_col = 0;
                src_y_row++;
        
                tar_x_col = tar_x;
                tar_y_row++;
            }
        }
        
        
        return;
}



void BlitBufCursor(
        unsigned int d,
        u_int8_t *tar_buf,
        u_int8_t *src_buf,
        int tar_x,
        int tar_y,
        unsigned int tar_width,
        unsigned int tar_height,
        unsigned int src_width,
        unsigned int src_height,
	WColorStruct color
)
{
	switch(d)
	{
	  case 32:
	    BlitBufCursor32(
                tar_buf,
                src_buf,
                tar_x,
                tar_y,
                tar_width,
                tar_height,
                src_width,
                src_height,
                color
	    );
	    break;

          case 24:
            BlitBufCursor32(
                tar_buf,
                src_buf,
                tar_x,
                tar_y,
                tar_width,
                tar_height,
                src_width,
                src_height,
                color
            );
            break;

          case 16:
            BlitBufCursor16(
                tar_buf,
                src_buf,
                tar_x,
                tar_y,
                tar_width,
                tar_height,
                src_width,
                src_height,
                color
            );
            break;

          case 15:
            BlitBufCursor15(
                tar_buf,
                src_buf,
                tar_x,
                tar_y,
                tar_width,
                tar_height,
                src_width,
                src_height,
                color
            );
            break;

	  case 8:
            BlitBufCursor8(
                tar_buf,
                src_buf,
                tar_x,
                tar_y,
                tar_width,
                tar_height,
                src_width,
                src_height,
                color
            );
            break;

	}

	return;
}



