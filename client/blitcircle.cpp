// client/blitcircle.cpp
/*
                        Buffer Blitting: Circle

	Functions:

	BlitBufCircle8()
	BlitBufCircle15()
	BlitBufCircle16()
	BlitBufCircle32()
	BlitBufCircle()

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


/* Maximum radius, in pixels. */
#define MAX_RADIUS	2000


/*
 *                     CIRCLE BLITTING
 */
void BlitBufCircle8(
        u_int8_t *tar_buf,
        int tar_x,
        int tar_y,
        unsigned int tar_width,
        unsigned int tar_height,
        double thickness,
        double radius,
        WColorStruct fg_color
)
{

        u_int8_t *tar_buf_ptr;

        int tar_width_bytes;   /* tar_width * BYTES_PER_PIXEL8 */

        int x, y;
        int q, q0, r;
        int cx, cy;



        /* Error checks. */
        if( (tar_buf == NULL) ||
            (tar_width < 1) ||
            (tar_height < 1)
        )
            return;


        /* Sanitize radius. */
        r = static_cast<int>((radius < 0) ? (radius * -1) : (radius));
        if(r > MAX_RADIUS)
            r = MAX_RADIUS;


        /* Source totally not visable on target? */
        if( ((tar_x + r) < 0) ||
            ((tar_y + r) < 0) ||
            ((tar_x - r) >= (int)tar_width) ||
            ((tar_y - r) >= (int)tar_height)
        )
            return;


        q = q0 = r * r;
        x = 0;
        y = r;

        tar_width_bytes = tar_width * BYTES_PER_PIXEL8;

        while(x <= y)
        {
            if(q > q0)
            {
                /* Outside circle. */
                y--;
                q -= 2 * y;
            }
            else
            {
                /* Inside circle. */
                x++;
                q += 2 * x;
            }


            /* Draw for primary arc. */

            /* Translate. */
            cx = tar_x + x;
            cy = tar_y + y;

            /* Check if 'on screen'. */
            if((cx >= 0) && (cx < (int)tar_width) &&
               (cy >= 0) && (cy < (int)tar_height)
            )
            {
                tar_buf_ptr = (u_int8_t *)(&tar_buf[
                    (cy * tar_width_bytes) + (cx * BYTES_PER_PIXEL8)
                ]);

                *tar_buf_ptr = PACK8TO8(fg_color.r, fg_color.g, fg_color.b);
            }


            /* Draw for vertical mirror. */

            /* Translate. */
            cx = tar_x + x;
            cy = tar_y - y;

            /* Check if 'on screen'. */
            if((cx >= 0) && (cx < (int)tar_width) &&
               (cy >= 0) && (cy < (int)tar_height)
            )
            {
                tar_buf_ptr = (u_int8_t *)(&tar_buf[
                    (cy * tar_width_bytes) + (cx * BYTES_PER_PIXEL8)
                ]);

                *tar_buf_ptr = PACK8TO8(fg_color.r, fg_color.g, fg_color.b);
            }


            /* Draw for horizontal mirror. */

            /* Translate. */
            cx = tar_x - x;
            cy = tar_y + y;

            /* Check if 'on screen'. */
            if((cx >= 0) && (cx < (int)tar_width) &&
               (cy >= 0) && (cy < (int)tar_height)
            )
            {
                tar_buf_ptr = (u_int8_t *)(&tar_buf[
                    (cy * tar_width_bytes) + (cx * BYTES_PER_PIXEL8)
                ]);

                *tar_buf_ptr = PACK8TO8(fg_color.r, fg_color.g, fg_color.b);
            }


            /* Draw for 180 degree rotation. */

            /* Translate. */
            cx = tar_x - x;
            cy = tar_y - y;

            /* Check if 'on screen'. */
            if((cx >= 0) && (cx < (int)tar_width) &&
               (cy >= 0) && (cy < (int)tar_height)
            )
            {
                tar_buf_ptr = (u_int8_t *)(&tar_buf[
                    (cy * tar_width_bytes) + (cx * BYTES_PER_PIXEL8)
                ]);

                *tar_buf_ptr = PACK8TO8(fg_color.r, fg_color.g, fg_color.b);
            }




            /* ***************************************************** */

            /* Draw for secondary arc. */

            /* Translate. */
            cx = tar_x + y;
            cy = tar_y + x;

            /* Check if 'on screen'. */
            if((cx >= 0) && (cx < (int)tar_width) &&
               (cy >= 0) && (cy < (int)tar_height)
            )
            {
                tar_buf_ptr = (u_int8_t *)(&tar_buf[
                    (cy * tar_width_bytes) + (cx * BYTES_PER_PIXEL8)
                ]);

                *tar_buf_ptr = PACK8TO8(fg_color.r, fg_color.g, fg_color.b);
            }


            /* Draw for vertical mirror. */

            /* Translate. */
            cx = tar_x + y;
            cy = tar_y - x;

            /* Check if 'on screen'. */
            if((cx >= 0) && (cx < (int)tar_width) &&
               (cy >= 0) && (cy < (int)tar_height)
            )
            {
                tar_buf_ptr = (u_int8_t *)(&tar_buf[
                    (cy * tar_width_bytes) + (cx * BYTES_PER_PIXEL8)
                ]);

                *tar_buf_ptr = PACK8TO8(fg_color.r, fg_color.g, fg_color.b);
            }


            /* Draw for horizontal mirror. */

            /* Translate. */
            cx = tar_x - y;
            cy = tar_y + x;

            /* Check if 'on screen'. */
            if((cx >= 0) && (cx < (int)tar_width) &&
               (cy >= 0) && (cy < (int)tar_height)
            )
            {
                tar_buf_ptr = (u_int8_t *)(&tar_buf[
                    (cy * tar_width_bytes) + (cx * BYTES_PER_PIXEL8)
                ]);

                *tar_buf_ptr = PACK8TO8(fg_color.r, fg_color.g, fg_color.b);
            }


            /* Draw for 180 degree rotation. */

            /* Translate. */
            cx = tar_x - y;
            cy = tar_y - x;

            /* Check if 'on screen'. */
            if((cx >= 0) && (cx < (int)tar_width) &&
               (cy >= 0) && (cy < (int)tar_height)
            )
            {
                tar_buf_ptr = (u_int8_t *)(&tar_buf[
                    (cy * tar_width_bytes) + (cx * BYTES_PER_PIXEL8)
                ]);

                *tar_buf_ptr = PACK8TO8(fg_color.r, fg_color.g, fg_color.b);
            }
        }
}


void BlitBufCircle15(
        u_int8_t *tar_buf,
        int tar_x,
        int tar_y,
        unsigned int tar_width,
        unsigned int tar_height,
        double thickness,
        double radius,
        WColorStruct fg_color
)           
{

	u_int16_t *tar_buf_ptr;

	int tar_width_bytes;	/* tar_width * BYTES_PER_PIXEL16 */

	int x, y;
	int q, q0, r;
	int cx, cy;



	/* Error checks. */
	if( (tar_buf == NULL) ||
	    (tar_width < 1) ||
	    (tar_height < 1)
	)
	    return;


	/* Sanitize radius. */
	r = static_cast<int>((radius < 0) ? (radius * -1) : (radius));
	if(r > MAX_RADIUS)
	    r = MAX_RADIUS;


        /* Source totally not visable on target? */
        if( ((tar_x + r) < 0) ||
            ((tar_y + r) < 0) ||
            ((tar_x - r) >= (int)tar_width) ||
            ((tar_y - r) >= (int)tar_height)
        )
            return;


	q = q0 = r * r;
	x = 0;
	y = r;

	tar_width_bytes = tar_width * BYTES_PER_PIXEL15;

	while(x <= y)
	{
	    if(q > q0)
	    {
		/* Outside circle. */
		y--;
		q -= 2 * y;
	    }
	    else
	    {
		/* Inside circle. */
                x++;
                q += 2 * x;
	    }


	    /* Draw for primary arc. */

	    /* Translate. */
	    cx = tar_x + x;
	    cy = tar_y + y;

	    /* Check if 'on screen'. */
	    if((cx >= 0) && (cx < (int)tar_width) &&
               (cy >= 0) && (cy < (int)tar_height)
	    )
	    {
		tar_buf_ptr = (u_int16_t *)(&tar_buf[
		    (cy * tar_width_bytes) + (cx * BYTES_PER_PIXEL15)
		]);

		*tar_buf_ptr = PACK8TO15(fg_color.r, fg_color.g, fg_color.b);
	    }


            /* Draw for vertical mirror. */

            /* Translate. */
            cx = tar_x + x;
            cy = tar_y - y;

            /* Check if 'on screen'. */
            if((cx >= 0) && (cx < (int)tar_width) &&
               (cy >= 0) && (cy < (int)tar_height)
            )
            {
                tar_buf_ptr = (u_int16_t *)(&tar_buf[
                    (cy * tar_width_bytes) + (cx * BYTES_PER_PIXEL15)
                ]);
        
                *tar_buf_ptr = PACK8TO15(fg_color.r, fg_color.g, fg_color.b);
            } 
 

            /* Draw for horizontal mirror. */

            /* Translate. */
            cx = tar_x - x;
            cy = tar_y + y;

            /* Check if 'on screen'. */
            if((cx >= 0) && (cx < (int)tar_width) &&
               (cy >= 0) && (cy < (int)tar_height)
            )
            {
                tar_buf_ptr = (u_int16_t *)(&tar_buf[
                    (cy * tar_width_bytes) + (cx * BYTES_PER_PIXEL15)
                ]);
                
                *tar_buf_ptr = PACK8TO15(fg_color.r, fg_color.g, fg_color.b);
            }


            /* Draw for 180 degree rotation. */
 
            /* Translate. */
            cx = tar_x - x;
            cy = tar_y - y;
                
            /* Check if 'on screen'. */
            if((cx >= 0) && (cx < (int)tar_width) &&
               (cy >= 0) && (cy < (int)tar_height)
            )       
            {      
                tar_buf_ptr = (u_int16_t *)(&tar_buf[
                    (cy * tar_width_bytes) + (cx * BYTES_PER_PIXEL15)
                ]);
            
                *tar_buf_ptr = PACK8TO15(fg_color.r, fg_color.g, fg_color.b);
            }


            /* ***************************************************** */

            /* Draw for secondary arc. */
 
            /* Translate. */
            cx = tar_x + y;
            cy = tar_y + x;

            /* Check if 'on screen'. */
            if((cx >= 0) && (cx < (int)tar_width) &&
               (cy >= 0) && (cy < (int)tar_height)
            )
            {
                tar_buf_ptr = (u_int16_t *)(&tar_buf[
                    (cy * tar_width_bytes) + (cx * BYTES_PER_PIXEL15)
                ]);
             
                *tar_buf_ptr = PACK8TO15(fg_color.r, fg_color.g, fg_color.b);
            }


            /* Draw for vertical mirror. */

            /* Translate. */
            cx = tar_x + y;
            cy = tar_y - x;
            
            /* Check if 'on screen'. */
            if((cx >= 0) && (cx < (int)tar_width) &&
               (cy >= 0) && (cy < (int)tar_height)
            )
            {
                tar_buf_ptr = (u_int16_t *)(&tar_buf[
                    (cy * tar_width_bytes) + (cx * BYTES_PER_PIXEL15)
                ]);
             
                *tar_buf_ptr = PACK8TO15(fg_color.r, fg_color.g, fg_color.b);
            }


            /* Draw for horizontal mirror. */

            /* Translate. */
            cx = tar_x - y;
            cy = tar_y + x;

            /* Check if 'on screen'. */
            if((cx >= 0) && (cx < (int)tar_width) &&
               (cy >= 0) && (cy < (int)tar_height)
            )
            {
                tar_buf_ptr = (u_int16_t *)(&tar_buf[
                    (cy * tar_width_bytes) + (cx * BYTES_PER_PIXEL15)
                ]);
             
                *tar_buf_ptr = PACK8TO15(fg_color.r, fg_color.g, fg_color.b);
            }


            /* Draw for 180 degree rotation. */

            /* Translate. */
            cx = tar_x - y;
            cy = tar_y - x;

            /* Check if 'on screen'. */
            if((cx >= 0) && (cx < (int)tar_width) &&
               (cy >= 0) && (cy < (int)tar_height)
            )
            {
                tar_buf_ptr = (u_int16_t *)(&tar_buf[
                    (cy * tar_width_bytes) + (cx * BYTES_PER_PIXEL15)
                ]);
             
                *tar_buf_ptr = PACK8TO15(fg_color.r, fg_color.g, fg_color.b);
            }
	}
}



void BlitBufCircle16(
        u_int8_t *tar_buf,
        int tar_x,
        int tar_y,
        unsigned int tar_width,
        unsigned int tar_height,
        double thickness,
        double radius,
        WColorStruct fg_color
)           
{

	u_int16_t *tar_buf_ptr;

	int tar_width_bytes;	/* tar_width * BYTES_PER_PIXEL16 */

	int x, y;
	int q, q0, r;
	int cx, cy;



	/* Error checks. */
	if((tar_buf == NULL) ||
	   (tar_width < 1) ||
	   (tar_height < 1)
	)
	    return;


	/* Sanitize radius. */
	r = static_cast<int>((radius < 0) ? (radius * -1) : (radius));
	if(r > MAX_RADIUS)
	    r = MAX_RADIUS;


        /* Source totally not visable on target? */
        if(((tar_x + r) < 0) ||
           ((tar_y + r) < 0) ||
           ((tar_x - r) >= (int)tar_width) ||
           ((tar_y - r) >= (int)tar_height)
        )
            return;


	q = q0 = r * r;
	x = 0;
	y = r;

	tar_width_bytes = tar_width * BYTES_PER_PIXEL16;

	while(x <= y)
	{
	    if(q > q0)
	    {
		/* Outside circle. */
		y--;
		q -= 2 * y;
	    }
	    else
	    {
		/* Inside circle. */
                x++;
                q += 2 * x;
	    }


	    /* Draw for primary arc. */

	    /* Translate. */
	    cx = tar_x + x;
	    cy = tar_y + y;

	    /* Check if 'on screen'. */
	    if((cx >= 0) && (cx < (int)tar_width) &&
               (cy >= 0) && (cy < (int)tar_height)
	    )
	    {
		tar_buf_ptr = (u_int16_t *)(&tar_buf[
		    (cy * tar_width_bytes) + (cx * BYTES_PER_PIXEL16)
		]);

		*tar_buf_ptr = PACK8TO16(fg_color.r, fg_color.g, fg_color.b);
	    }


            /* Draw for vertical mirror. */

            /* Translate. */
            cx = tar_x + x;
            cy = tar_y - y;

            /* Check if 'on screen'. */
            if((cx >= 0) && (cx < (int)tar_width) &&
               (cy >= 0) && (cy < (int)tar_height)
            )
            {
                tar_buf_ptr = (u_int16_t *)(&tar_buf[
                    (cy * tar_width_bytes) + (cx * BYTES_PER_PIXEL16)
                ]);
        
                *tar_buf_ptr = PACK8TO16(fg_color.r, fg_color.g, fg_color.b);
            } 
 

            /* Draw for horizontal mirror. */

            /* Translate. */
            cx = tar_x - x;
            cy = tar_y + y;

            /* Check if 'on screen'. */
            if((cx >= 0) && (cx < (int)tar_width) &&
               (cy >= 0) && (cy < (int)tar_height)
            )
            {
                tar_buf_ptr = (u_int16_t *)(&tar_buf[
                    (cy * tar_width_bytes) + (cx * BYTES_PER_PIXEL16)
                ]);
                
                *tar_buf_ptr = PACK8TO16(fg_color.r, fg_color.g, fg_color.b);
            }


            /* Draw for 180 degree rotation. */
 
            /* Translate. */
            cx = tar_x - x;
            cy = tar_y - y;
                
            /* Check if 'on screen'. */
            if((cx >= 0) && (cx < (int)tar_width) &&
               (cy >= 0) && (cy < (int)tar_height)
            )
            {
                tar_buf_ptr = (u_int16_t *)(&tar_buf[
                    (cy * tar_width_bytes) + (cx * BYTES_PER_PIXEL16)
                ]);
                *tar_buf_ptr = PACK8TO16(fg_color.r, fg_color.g, fg_color.b);
            }


            /* ***************************************************** */

            /* Draw for secondary arc. */
 
            /* Translate. */
            cx = tar_x + y;
            cy = tar_y + x;

            /* Check if 'on screen'. */
            if((cx >= 0) && (cx < (int)tar_width) &&
               (cy >= 0) && (cy < (int)tar_height)
            )
            {
                tar_buf_ptr = (u_int16_t *)(&tar_buf[
                    (cy * tar_width_bytes) + (cx * BYTES_PER_PIXEL16)
                ]);
             
                *tar_buf_ptr = PACK8TO16(fg_color.r, fg_color.g, fg_color.b);
            }


            /* Draw for vertical mirror. */

            /* Translate. */
            cx = tar_x + y;
            cy = tar_y - x;
            
            /* Check if 'on screen'. */
            if((cx >= 0) && (cx < (int)tar_width) &&
               (cy >= 0) && (cy < (int)tar_height)
            )
            {
                tar_buf_ptr = (u_int16_t *)(&tar_buf[
                    (cy * tar_width_bytes) + (cx * BYTES_PER_PIXEL16)
                ]);
             
                *tar_buf_ptr = PACK8TO16(fg_color.r, fg_color.g, fg_color.b);
            }


            /* Draw for horizontal mirror. */

            /* Translate. */
            cx = tar_x - y;
            cy = tar_y + x;

            /* Check if 'on screen'. */
            if((cx >= 0) && (cx < (int)tar_width) &&
               (cy >= 0) && (cy < (int)tar_height)
            )
            {
                tar_buf_ptr = (u_int16_t *)(&tar_buf[
                    (cy * tar_width_bytes) + (cx * BYTES_PER_PIXEL16)
                ]);
             
                *tar_buf_ptr = PACK8TO16(fg_color.r, fg_color.g, fg_color.b);
            }


            /* Draw for 180 degree rotation. */

            /* Translate. */
            cx = tar_x - y;
            cy = tar_y - x;

            /* Check if 'on screen'. */
            if((cx >= 0) && (cx < (int)tar_width) &&
               (cy >= 0) && (cy < (int)tar_height)
            )
            {
                tar_buf_ptr = (u_int16_t *)(&tar_buf[
                    (cy * tar_width_bytes) + (cx * BYTES_PER_PIXEL16)
                ]);
             
                *tar_buf_ptr = PACK8TO16(fg_color.r, fg_color.g, fg_color.b);
            }
	}
}


void BlitBufCircle32(
        u_int8_t *tar_buf,
        int tar_x,
        int tar_y,
        unsigned int tar_width,
        unsigned int tar_height,
        double thickness,
        double radius,
        WColorStruct fg_color
)
{
	/* Local variables. */
        u_int32_t *tar_buf_ptr;

        int tar_width_bytes;   /* tar_width * BYTES_PER_PIXEL32 */

        int x, y;
        int q, q0, r;
        int cx, cy;


        /* Error checks. */
        if((tar_buf == NULL) ||
           (tar_width < 1) ||
           (tar_height < 1)
        )
            return;


        /* Sanitize radius. */
        r = static_cast<int>((radius < 0) ? (radius * -1) : (radius));
        if(r > MAX_RADIUS)
            r = MAX_RADIUS;


        /* Source totally not visable on target? */
        if(((tar_x + r) < 0) ||
           ((tar_y + r) < 0) ||
           ((tar_x - r) >= (int)tar_width) ||
           ((tar_y - r) >= (int)tar_height)
        )
            return;


        q = q0 = r * r;
        x = 0;
        y = r;

        tar_width_bytes = tar_width * BYTES_PER_PIXEL32;

        while(x <= y)
        {
            if(q > q0)
            {
                /* Outside circle. */
                y--;
                q -= 2 * y;
            }
            else
            {
                /* Inside circle. */
                x++;
                q += 2 * x;
            }


            /* Draw for primary arc. */

            /* Translate. */
            cx = tar_x + x;
            cy = tar_y + y;
            /* Check if 'on screen'. */
            if((cx >= 0) && (cx < (int)tar_width) &&
               (cy >= 0) && (cy < (int)tar_height)
            )
            {
                tar_buf_ptr = (u_int32_t *)(&tar_buf[
                    (cy * tar_width_bytes) + (cx * BYTES_PER_PIXEL32)
                ]);
                *tar_buf_ptr = PACK8TO32(fg_color.a,
		    fg_color.r, fg_color.g, fg_color.b);
            }


            /* Draw for vertical mirror. */

            /* Translate. */
            cx = tar_x + x;
            cy = tar_y - y;

            /* Check if 'on screen'. */
            if((cx >= 0) && (cx < (int)tar_width) &&
               (cy >= 0) && (cy < (int)tar_height)
            )
            {
                tar_buf_ptr = (u_int32_t *)(&tar_buf[
                    (cy * tar_width_bytes) + (cx * BYTES_PER_PIXEL32)
                ]);
                *tar_buf_ptr = PACK8TO32(fg_color.a,
		    fg_color.r, fg_color.g, fg_color.b);
            } 
 

            /* Draw for horizontal mirror. */

            /* Translate. */
            cx = tar_x - x;
            cy = tar_y + y;

            /* Check if 'on screen'. */
            if((cx >= 0) && (cx < (int)tar_width) &&
               (cy >= 0) && (cy < (int)tar_height)
            )
            {
                tar_buf_ptr = (u_int32_t *)(&tar_buf[
                    (cy * tar_width_bytes) + (cx * BYTES_PER_PIXEL32)
                ]);
                *tar_buf_ptr = PACK8TO32(fg_color.a,
                    fg_color.r, fg_color.g, fg_color.b);
            }


            /* Draw for 180 degree rotation. */
 
            /* Translate. */
            cx = tar_x - x;
            cy = tar_y - y;
                
            /* Check if 'on screen'. */
            if((cx >= 0) && (cx < (int)tar_width) &&
               (cy >= 0) && (cy < (int)tar_height)
            )       
            {      
                tar_buf_ptr = (u_int32_t *)(&tar_buf[
                    (cy * tar_width_bytes) + (cx * BYTES_PER_PIXEL32)
                ]);
                *tar_buf_ptr = PACK8TO32(fg_color.a,
                   fg_color.r, fg_color.g, fg_color.b);
            }


            /* ***************************************************** */

            /* Draw for secondary arc. */
 
            /* Translate. */
            cx = tar_x + y;
            cy = tar_y + x;

            /* Check if 'on screen'. */
            if((cx >= 0) && (cx < (int)tar_width) &&
               (cy >= 0) && (cy < (int)tar_height)
            )
            {
                tar_buf_ptr = (u_int32_t *)(&tar_buf[
                    (cy * tar_width_bytes) + (cx * BYTES_PER_PIXEL32)
                ]);
             
                *tar_buf_ptr = PACK8TO32(fg_color.a,
                    fg_color.r, fg_color.g, fg_color.b);
            }


            /* Draw for vertical mirror. */

            /* Translate. */
            cx = tar_x + y;
            cy = tar_y - x;
            
            /* Check if 'on screen'. */
            if((cx >= 0) && (cx < (int)tar_width) &&
               (cy >= 0) && (cy < (int)tar_height)
            )
            {
                tar_buf_ptr = (u_int32_t *)(&tar_buf[
                    (cy * tar_width_bytes) + (cx * BYTES_PER_PIXEL32)
                ]);
                *tar_buf_ptr = PACK8TO32(fg_color.a,
                    fg_color.r, fg_color.g, fg_color.b);
            }


            /* Draw for horizontal mirror. */

            /* Translate. */
            cx = tar_x - y;
            cy = tar_y + x;

            /* Check if 'on screen'. */
            if((cx >= 0) && (cx < (int)tar_width) &&
               (cy >= 0) && (cy < (int)tar_height)
            )
            {
                tar_buf_ptr = (u_int32_t *)(&tar_buf[
                    (cy * tar_width_bytes) + (cx * BYTES_PER_PIXEL32)
                ]);

                *tar_buf_ptr = PACK8TO32(fg_color.a,
                    fg_color.r, fg_color.g, fg_color.b);
            }


            /* Draw for 180 degree rotation. */

            /* Translate. */
            cx = tar_x - y;
            cy = tar_y - x;

            /* Check if 'on screen'. */
            if((cx >= 0) && (cx < (int)tar_width) &&
               (cy >= 0) && (cy < (int)tar_height)
            )
            {
                tar_buf_ptr = (u_int32_t *)(&tar_buf[
                    (cy * tar_width_bytes) + (cx * BYTES_PER_PIXEL32)
                ]);
                *tar_buf_ptr = PACK8TO32(fg_color.a,
		    fg_color.r, fg_color.g, fg_color.b);
            }
        }
}



void BlitBufCircle(
        unsigned int d,
        u_int8_t *tar_buf,
        int tar_x,
        int tar_y,
        unsigned int tar_width,
        unsigned int tar_height,
        double thickness,               /* In pixels. */
        double radius,                  /* In pixels. */
        WColorStruct fg_color
)
{
	switch(d)
	{
	  case 32:
	    BlitBufCircle32(
		tar_buf,
                tar_x,
                tar_y,
                tar_width,
                tar_height,
                thickness,               /* In pixels. */
                radius,                  /* In pixels. */
                fg_color
            );
	    break;

          case 24:
            BlitBufCircle32(
                tar_buf,
                tar_x,
                tar_y,
                tar_width,
                tar_height,
                thickness,               /* In pixels. */
                radius,                  /* In pixels. */
                fg_color
            );
            break;

          case 16:
            BlitBufCircle16(
                tar_buf,
                tar_x,
                tar_y,
                tar_width, 
                tar_height,
                thickness,               /* In pixels. */
                radius,                  /* In pixels. */
                fg_color
            );
            break;

          case 15:
            BlitBufCircle15(
                tar_buf,
                tar_x,
                tar_y,
                tar_width, 
                tar_height,
                thickness,               /* In pixels. */
                radius,                  /* In pixels. */
                fg_color
            );
            break;

          case 8:
            BlitBufCircle8(
                tar_buf,
                tar_x,
                tar_y,
                tar_width,
                tar_height,
                thickness,               /* In pixels. */
                radius,                  /* In pixels. */
                fg_color
            );
            break;

	}

	return;
}



