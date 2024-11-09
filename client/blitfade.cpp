// client/blitfade.cpp
/*
                 Buffer Blitting: Fade and Glow effects

	Functions:

	BlitBufFade8()
        BlitBufFade15()
	BlitBufFade16()
	BlitBufFade32()
	BlitBufFade()

        BlitBufGlow8()
        BlitBufGlow15()
        BlitBufGlow16()
        BlitBufGlow32()
        BlitBufGlow()


	---

 */

#include "blitting.h"

#ifndef MAX
#define MIN(a,b)        (((a) < (b)) ? (a) : (b))
#define MAX(a,b)	(((a) > (b)) ? (a) : (b))
#endif
/* #define MIN(a,b)        (((a) < (b)) ? (a) : (b)) */
/* #define MAX(a,b)        (((a) > (b)) ? (a) : (b)) */


/*
 *                               Fade
 */
void BlitBufFade8(
        u_int8_t *tar_buf,
        int tar_x,
        int tar_y,
        unsigned int tar_width,
        unsigned int tar_height,
        unsigned int tar_apply_width,
        unsigned int tar_apply_height,
        double gamma                    /* 0.0 to 1.0. */
)
{
        int tar_width_bytes;     /* tar_width * BYTES_PER_PIXEL8 */
	int dnext;

        u_int8_t *tar_buf_ptr,
		 *tar_buf_edge, *tar_buf_end;


        /* Error checks. */
        if((tar_buf == NULL) ||
           (tar_width < 1) ||
           (tar_height < 1) ||
           (tar_apply_width < 1) ||
           (tar_apply_height < 1) ||
           (gamma >= 1)
        )
            return;


        /* Sanitize gamma. */
        if(gamma < 0)
            gamma = 0;


        /* Requested fade area totally off screen? */
        if((tar_x >= (int)tar_width) ||
           (tar_y >= (int)tar_height) ||
           ((tar_x + (int)tar_apply_width) <= 0) ||
           ((tar_y + (int)tar_apply_height) <= 0)
        )
            return;


        /* ************************************************************* */  

        /* Calculate bytes per width. */
        tar_width_bytes = (int)tar_width * BYTES_PER_PIXEL8;

        /* Sanitize starts. */
        if(tar_x < 0)
	{
	    tar_apply_width = MAX((int)tar_apply_width + tar_x, 0);
	    tar_x = 0;
	}
        if(tar_y < 0)
	{
	    tar_apply_height = MAX((int)tar_apply_height + tar_y, 0);
	    tar_y = 0;
	}

	if(((int)tar_apply_width + tar_x) > (int)tar_width)
	    tar_apply_width = (int)tar_width - tar_x;

        if(((int)tar_apply_height + tar_y) > (int)tar_height)
            tar_apply_height = (int)tar_height - tar_y;


	/* Set pointer positions. */
	tar_buf_ptr = (u_int8_t *)(&tar_buf[
            (tar_y * tar_width_bytes) +
            (tar_x * BYTES_PER_PIXEL8)
        ]);

	dnext = MAX((int)tar_width - (int)tar_apply_width, 0);
	tar_buf_edge = (u_int8_t *)(tar_buf_ptr + tar_apply_width);

        tar_buf_end = (u_int8_t *)(&tar_buf[
            ((tar_y + (int)tar_apply_height - 1) * tar_width_bytes) +
            ((tar_x + (int)tar_apply_width) * BYTES_PER_PIXEL8)  
        ]);


        /* Begin blitting. */
        while(tar_buf_ptr < tar_buf_end)
        {
	    if(tar_buf_ptr >= tar_buf_edge)
	    {
		tar_buf_ptr += dnext;
		tar_buf_edge = tar_buf_ptr + tar_apply_width;
		continue;
	    }

            /* No transparency check. */

            /* Do fade to this pixel. */
            *tar_buf_ptr++ = PACK8TO8(
		(u_int8_t)(((*tar_buf_ptr & 0xE0)) * gamma),
		(u_int8_t)(((*tar_buf_ptr & 0x1C) << 2) * gamma),
		(u_int8_t)(((*tar_buf_ptr & 0x03) << 5) * gamma)
	    );
        }


        return;
}

void BlitBufFade15(
        u_int8_t *tar_buf,
        int tar_x,
        int tar_y,
        unsigned int tar_width, 
        unsigned int tar_height,
        unsigned int tar_apply_width,
        unsigned int tar_apply_height,
        double gamma			/* 0.0 to 1.0. */
)
{
        int tar_width_bytes;     /* tar_width * BYTES_PER_PIXEL16 */
	int dnext;

        u_int16_t *tar_buf_ptr,	*tar_buf_edge, *tar_buf_end;


        /* Error checks. */
        if((tar_buf == NULL) ||
           (tar_width < 1) ||
           (tar_height < 1) ||
           (tar_apply_width < 1) ||
           (tar_apply_height < 1) ||
           (gamma >= 1)
        )
            return;


        /* Sanitize gamma. */
        if(gamma < 0)
            gamma = 0;


        /* Requested fade area totally off screen? */
        if((tar_x >= (int)tar_width) ||
           (tar_y >= (int)tar_height) ||
           ((tar_x + (int)tar_apply_width) <= 0) ||
           ((tar_y + (int)tar_apply_height) <= 0)
        )
            return;


        /* ************************************************************* */

        /* Calculate bytes per width. */
        tar_width_bytes = (int)tar_width * BYTES_PER_PIXEL15;

        /* Sanitize starts. */
        if(tar_x < 0)
        {
            tar_apply_width = MAX((int)tar_apply_width + tar_x, 0);
            tar_x = 0;
        }  
        if(tar_y < 0)
        {
            tar_apply_height = MAX((int)tar_apply_height + tar_y, 0);
            tar_y = 0;
        }

        if(((int)tar_apply_width + tar_x) > (int)tar_width)
            tar_apply_width = (int)tar_width - tar_x;

        if(((int)tar_apply_height + tar_y) > (int)tar_height)
            tar_apply_height = (int)tar_height - tar_y;


        /* Set pointer positions. */
        tar_buf_ptr = (u_int16_t *)(&tar_buf[
            (tar_y * tar_width_bytes) + 
            (tar_x * BYTES_PER_PIXEL15)
        ]);

        dnext = MAX((int)tar_width - (int)tar_apply_width, 0);
        tar_buf_edge = (u_int16_t *)(tar_buf_ptr + tar_apply_width);

        tar_buf_end = (u_int16_t *)(&tar_buf[
            ((tar_y + (int)tar_apply_height - 1) * tar_width_bytes) +
            ((tar_x + (int)tar_apply_width) * BYTES_PER_PIXEL15)
        ]);


        /* Begin blitting. */
        while(tar_buf_ptr < tar_buf_end)
        {
            if(tar_buf_ptr >= tar_buf_edge)
            {
                tar_buf_ptr += dnext;
                tar_buf_edge = tar_buf_ptr + tar_apply_width;
                continue;
            }

            /* No transparency check. */

            /* Do fade to this pixel. */
            *tar_buf_ptr++ = PACK8TO15(
                (u_int8_t)(((*tar_buf_ptr & 0x7C00) >> 7) * gamma),
                (u_int8_t)(((*tar_buf_ptr & 0x03E0) >> 2) * gamma),
                (u_int8_t)(((*tar_buf_ptr & 0x001F) << 3) * gamma)
            );
        }


        return;
}


void BlitBufFade16(
        u_int8_t *tar_buf,
        int tar_x,
        int tar_y,
        unsigned int tar_width, 
        unsigned int tar_height,
        unsigned int tar_apply_width,
        unsigned int tar_apply_height,
        double gamma			/* 0.0 to 1.0. */
)
{
        int tar_width_bytes;     /* tar_width * BYTES_PER_PIXEL16 */
	int dnext;

        u_int16_t *tar_buf_ptr,	*tar_buf_edge, *tar_buf_end;


        /* Error checks. */
        if((tar_buf == NULL) ||
           (tar_width < 1) ||
           (tar_height < 1) ||
           (tar_apply_width < 1) ||
           (tar_apply_height < 1) ||
           (gamma >= 1)
        )
            return;


        /* Sanitize gamma. */
        if(gamma < 0)
            gamma = 0;


        /* Requested fade area totally off screen? */
        if((tar_x >= (int)tar_width) ||
           (tar_y >= (int)tar_height) ||
           ((tar_x + (int)tar_apply_width) <= 0) ||
           ((tar_y + (int)tar_apply_height) <= 0)
        )
            return;


        /* ************************************************************* */

        /* Calculate bytes per width. */
        tar_width_bytes = (int)tar_width * BYTES_PER_PIXEL16;

        /* Sanitize starts. */
        if(tar_x < 0)
        {
            tar_apply_width = MAX((int)tar_apply_width + tar_x, 0);
            tar_x = 0;
        }  
        if(tar_y < 0)
        {
            tar_apply_height = MAX((int)tar_apply_height + tar_y, 0);
            tar_y = 0;
        }

        if(((int)tar_apply_width + tar_x) > (int)tar_width)
            tar_apply_width = (int)tar_width - tar_x;

        if(((int)tar_apply_height + tar_y) > (int)tar_height)
            tar_apply_height = (int)tar_height - tar_y;


        /* Set pointer positions. */
        tar_buf_ptr = (u_int16_t *)(&tar_buf[
            (tar_y * tar_width_bytes) + 
            (tar_x * BYTES_PER_PIXEL16)
        ]);

        dnext = MAX((int)tar_width - (int)tar_apply_width, 0);
        tar_buf_edge = (u_int16_t *)(tar_buf_ptr + tar_apply_width);

        tar_buf_end = (u_int16_t *)(&tar_buf[
            ((tar_y + (int)tar_apply_height - 1) * tar_width_bytes) +
            ((tar_x + (int)tar_apply_width) * BYTES_PER_PIXEL16)
        ]);


        /* Begin blitting. */
        while(tar_buf_ptr < tar_buf_end)
        {
            if(tar_buf_ptr >= tar_buf_edge)
            {
                tar_buf_ptr += dnext;
                tar_buf_edge = tar_buf_ptr + tar_apply_width;
                continue;
            }

            /* No transparency check. */

            /* Do fade to this pixel. */
            *tar_buf_ptr++ = PACK8TO16(
                (u_int8_t)(((*tar_buf_ptr & 0xF800) >> 8) * gamma),
                (u_int8_t)(((*tar_buf_ptr & 0x07E0) >> 3) * gamma),
                (u_int8_t)(((*tar_buf_ptr & 0x001F) << 3) * gamma)
            );
        }


        return;
}



void BlitBufFade32(
        u_int8_t *tar_buf,
        int tar_x,
        int tar_y,
        unsigned int tar_width,
        unsigned int tar_height,
        unsigned int tar_apply_width,
        unsigned int tar_apply_height,
        double gamma                    /* 0.0 to 1.0. */
)
{
        int tar_width_bytes;     /* tar_width * BYTES_PER_PIXEL32 */
        int dnext;
        u_int32_t *tar_buf_ptr, *tar_buf_edge, *tar_buf_end;


        /* Error checks. */
        if((tar_buf == NULL) ||
           (tar_width < 1) ||
           (tar_height < 1) ||
           (tar_apply_width < 1) ||
           (tar_apply_height < 1) ||
           (gamma >= 1)
        )
            return;


        /* Sanitize gamma. */
        if(gamma < 0)
            gamma = 0;


        /* Requested fade area totally off screen? */
        if((tar_x >= (int)tar_width) ||
           (tar_y >= (int)tar_height) ||
           ((tar_x + (int)tar_apply_width) <= 0) ||
           ((tar_y + (int)tar_apply_height) <= 0)
        )
            return;


        /* ************************************************************* */

        /* Calculate bytes per width. */
        tar_width_bytes = (int)tar_width * BYTES_PER_PIXEL32;

        /* Sanitize starts. */
        if(tar_x < 0)
        {
            tar_apply_width = MAX((int)tar_apply_width + tar_x, 0);
            tar_x = 0;
        }
        if(tar_y < 0)
        {
            tar_apply_height = MAX((int)tar_apply_height + tar_y, 0);
            tar_y = 0;
        }

        if(((int)tar_apply_width + tar_x) > (int)tar_width)
            tar_apply_width = (int)tar_width - tar_x;

        if(((int)tar_apply_height + tar_y) > (int)tar_height)
            tar_apply_height = (int)tar_height - tar_y;


        /* Set pointer positions. */
        tar_buf_ptr = (u_int32_t *)(&tar_buf[
            (tar_y * tar_width_bytes) +
            (tar_x * BYTES_PER_PIXEL32)
        ]);
            
        dnext = MAX((int)tar_width - (int)tar_apply_width, 0);
        tar_buf_edge = (u_int32_t *)(tar_buf_ptr + tar_apply_width);

        tar_buf_end = (u_int32_t *)(&tar_buf[
            ((tar_y + (int)tar_apply_height - 1) * tar_width_bytes) +
            ((tar_x + (int)tar_apply_width) * BYTES_PER_PIXEL32)
        ]);


        /* Begin blitting. */
        while(tar_buf_ptr < tar_buf_end)
        {
            if(tar_buf_ptr >= tar_buf_edge)
            {
                tar_buf_ptr += dnext;
                tar_buf_edge = tar_buf_ptr + tar_apply_width;
                continue;
            }

            /* No transparency check. */

            /* Do fade to this pixel. */
            *tar_buf_ptr++ = PACK8TO32(
		(u_int8_t)((*tar_buf_ptr & 0xff000000) >> 24),
                (u_int8_t)(((*tar_buf_ptr & 0x00ff0000) >> 16) * gamma),
                (u_int8_t)(((*tar_buf_ptr & 0x0000ff00) >> 8) * gamma),
                (u_int8_t)((*tar_buf_ptr & 0x000000ff) * gamma)
            );
        }


        return;
}



void BlitBufFade(
	unsigned int d,
        u_int8_t *tar_buf,
	int tar_x,
	int tar_y,
        unsigned int tar_width,
        unsigned int tar_height,
        unsigned int tar_apply_width,
	unsigned int tar_apply_height,
        double gamma			/* 0.1 to 1.0. */
)
{
	switch(d)
	{
          case 32:
            BlitBufFade32(
                tar_buf,
                tar_x,
                tar_y,
                tar_width,
                tar_height,
                tar_apply_width,
                tar_apply_height,
                gamma                    /* 0.1 to 1.0. */
            );
            break;

	  case 24:
            BlitBufFade32(
                tar_buf,
                tar_x,
                tar_y,
                tar_width,
                tar_height,
                tar_apply_width,
                tar_apply_height,
                gamma                    /* 0.1 to 1.0. */
	    );
	    break;

          case 16:
            BlitBufFade16(
                tar_buf,
                tar_x, 
                tar_y,
                tar_width,
                tar_height,
                tar_apply_width,
                tar_apply_height,
                gamma                    /* 0.1 to 1.0. */
            );
            break;

          case 15:
            BlitBufFade15(
                tar_buf,
                tar_x, 
                tar_y,
                tar_width,
                tar_height,
                tar_apply_width,
                tar_apply_height,
                gamma                    /* 0.1 to 1.0. */
            );
            break;

	  case 8:
            BlitBufFade8(
                tar_buf,
                tar_x,
                tar_y,
                tar_width,
                tar_height,
                tar_apply_width,
                tar_apply_height,
                gamma                    /* 0.1 to 1.0. */
            );
            break;
	}


	return;
}



/*
 *                           Glow
 */
void BlitBufGlow8(
        u_int8_t *tar_buf,
        int tar_x,
        int tar_y,
        unsigned int tar_width,
        unsigned int tar_height,
        unsigned int tar_apply_width,
        unsigned int tar_apply_height,
        WColorStruct color
)
{
        int tar_width_bytes;     /* tar_width * BYTES_PER_PIXEL8 */
	int dnext;

        u_int8_t *tar_buf_ptr,
		 *tar_buf_edge, *tar_buf_end;


        /* Error checks. */
        if((tar_buf == NULL) ||
           (tar_width < 1) ||
           (tar_height < 1) ||
           (tar_apply_width < 1) ||
           (tar_apply_height < 1)
        )
            return;


        /* Requested fade area totally off screen? */
        if((tar_x >= (int)tar_width) ||
           (tar_y >= (int)tar_height) ||
           ((tar_x + (int)tar_apply_width) <= 0) ||
           ((tar_y + (int)tar_apply_height) <= 0)
        )
            return;


        /* ************************************************************* */  

        /* Calculate bytes per width. */
        tar_width_bytes = (int)tar_width * BYTES_PER_PIXEL8;

        /* Sanitize starts. */
        if(tar_x < 0)
	{
	    tar_apply_width = MAX((int)tar_apply_width + tar_x, 0);
	    tar_x = 0;
	}
        if(tar_y < 0)
	{
	    tar_apply_height = MAX((int)tar_apply_height + tar_y, 0);
	    tar_y = 0;
	}

	if(((int)tar_apply_width + tar_x) > (int)tar_width)
	    tar_apply_width = (int)tar_width - tar_x;

        if(((int)tar_apply_height + tar_y) > (int)tar_height)
            tar_apply_height = (int)tar_height - tar_y;


	/* Set pointer positions. */
	tar_buf_ptr = (u_int8_t *)(&tar_buf[
            (tar_y * tar_width_bytes) +
            (tar_x * BYTES_PER_PIXEL8)
        ]);

	dnext = MAX((int)tar_width - (int)tar_apply_width, 0);
	tar_buf_edge = (u_int8_t *)(tar_buf_ptr + tar_apply_width);

        tar_buf_end = (u_int8_t *)(&tar_buf[
            ((tar_y + (int)tar_apply_height - 1) * tar_width_bytes) +
            ((tar_x + (int)tar_apply_width) * BYTES_PER_PIXEL8)  
        ]);


        /* Begin blitting. */
        while(tar_buf_ptr < tar_buf_end)
        {
	    if(tar_buf_ptr >= tar_buf_edge)
	    {
		tar_buf_ptr += dnext;
		tar_buf_edge = tar_buf_ptr + tar_apply_width;
		continue;
	    }

            /* No transparency check. */

            /*   Add glow to this pixel.
	     *   rrgg gbbb
	     */
	    *tar_buf_ptr++ = PACK8TO8(
		(u_int8_t)MIN(
		    ((*tar_buf_ptr & 0xE0)) + color.r,
		    0xff
		),
                (u_int8_t)MIN(
                    ((*tar_buf_ptr & 0x1C) << 2) + color.g,
                    0xff
                ),
                (u_int8_t)MIN(
                    ((*tar_buf_ptr & 0x03) << 5) + color.b,
                    0xff
                )
	    );
        }


        return;
}

void BlitBufGlow15(
        u_int8_t *tar_buf,
        int tar_x,
        int tar_y,
        unsigned int tar_width, 
        unsigned int tar_height,
        unsigned int tar_apply_width,
        unsigned int tar_apply_height,
        WColorStruct color
)
{
        int tar_width_bytes;     /* tar_width * BYTES_PER_PIXEL16 */
	int dnext;

        u_int16_t *tar_buf_ptr,	*tar_buf_edge, *tar_buf_end;


        /* Error checks. */
        if((tar_buf == NULL) ||
           (tar_width < 1) ||
           (tar_height < 1) ||
           (tar_apply_width < 1) ||
           (tar_apply_height < 1)
        )
            return;


        /* Requested fade area totally off screen? */
        if((tar_x >= (int)tar_width) ||
           (tar_y >= (int)tar_height) ||
           ((tar_x + (int)tar_apply_width) <= 0) ||
           ((tar_y + (int)tar_apply_height) <= 0)
        )
            return;


        /* ************************************************************* */

        /* Calculate bytes per width. */
        tar_width_bytes = (int)tar_width * BYTES_PER_PIXEL15;

        /* Sanitize starts. */
        if(tar_x < 0)
        {
            tar_apply_width = MAX((int)tar_apply_width + tar_x, 0);
            tar_x = 0;
        }  
        if(tar_y < 0)
        {
            tar_apply_height = MAX((int)tar_apply_height + tar_y, 0);
            tar_y = 0;
        }

        if(((int)tar_apply_width + tar_x) > (int)tar_width)
            tar_apply_width = (int)tar_width - tar_x;

        if(((int)tar_apply_height + tar_y) > (int)tar_height)
            tar_apply_height = (int)tar_height - tar_y;


        /* Set pointer positions. */
        tar_buf_ptr = (u_int16_t *)(&tar_buf[
            (tar_y * tar_width_bytes) + 
            (tar_x * BYTES_PER_PIXEL15)
        ]);

        dnext = MAX((int)tar_width - (int)tar_apply_width, 0);
        tar_buf_edge = (u_int16_t *)(tar_buf_ptr + tar_apply_width);

        tar_buf_end = (u_int16_t *)(&tar_buf[
            ((tar_y + (int)tar_apply_height - 1) * tar_width_bytes) +
            ((tar_x + (int)tar_apply_width) * BYTES_PER_PIXEL15)
        ]);


        /* Begin blitting. */
        while(tar_buf_ptr < tar_buf_end)
        {
            if(tar_buf_ptr >= tar_buf_edge)
            {
                tar_buf_ptr += dnext;
                tar_buf_edge = tar_buf_ptr + tar_apply_width;
                continue;
            }

            /* No transparency check. */

            /*   Add glow to this pixel.
             *   arrr rrgg gggb bbbb
             */
            *tar_buf_ptr++ = PACK8TO15(
                (u_int8_t)MIN(
                    ((*tar_buf_ptr & 0x7C00) >> 7) + color.r,
                    0xff
                ),
                (u_int8_t)MIN(
                    ((*tar_buf_ptr & 0x03E0) >> 2) + color.g,
                    0xff
                ),
                (u_int8_t)MIN(
                    ((*tar_buf_ptr & 0x001F) << 3) + color.b,
                    0xff
                )
            );   
        }


        return;
}


void BlitBufGlow16(
        u_int8_t *tar_buf,
        int tar_x,
        int tar_y,
        unsigned int tar_width, 
        unsigned int tar_height,
        unsigned int tar_apply_width,
        unsigned int tar_apply_height,
        WColorStruct color
)
{
        int tar_width_bytes;     /* tar_width * BYTES_PER_PIXEL16 */
	int dnext;

        u_int16_t *tar_buf_ptr,	*tar_buf_edge, *tar_buf_end;


        /* Error checks. */
        if((tar_buf == NULL) ||
           (tar_width < 1) ||
           (tar_height < 1) ||
           (tar_apply_width < 1) ||
           (tar_apply_height < 1)
        )
            return;


        /* Requested fade area totally off screen? */
        if((tar_x >= (int)tar_width) ||
           (tar_y >= (int)tar_height) ||
           ((tar_x + (int)tar_apply_width) <= 0) ||
           ((tar_y + (int)tar_apply_height) <= 0)
        )
            return;


        /* ************************************************************* */

        /* Calculate bytes per width. */
        tar_width_bytes = (int)tar_width * BYTES_PER_PIXEL16;

        /* Sanitize starts. */
        if(tar_x < 0)
        {
            tar_apply_width = MAX((int)tar_apply_width + tar_x, 0);
            tar_x = 0;
        }  
        if(tar_y < 0)
        {
            tar_apply_height = MAX((int)tar_apply_height + tar_y, 0);
            tar_y = 0;
        }

        if(((int)tar_apply_width + tar_x) > (int)tar_width)
            tar_apply_width = (int)tar_width - tar_x;

        if(((int)tar_apply_height + tar_y) > (int)tar_height)
            tar_apply_height = (int)tar_height - tar_y;


        /* Set pointer positions. */
        tar_buf_ptr = (u_int16_t *)(&tar_buf[
            (tar_y * tar_width_bytes) + 
            (tar_x * BYTES_PER_PIXEL16)
        ]);

        dnext = MAX((int)tar_width - (int)tar_apply_width, 0);
        tar_buf_edge = (u_int16_t *)(tar_buf_ptr + tar_apply_width);

        tar_buf_end = (u_int16_t *)(&tar_buf[
            ((tar_y + (int)tar_apply_height - 1) * tar_width_bytes) +
            ((tar_x + (int)tar_apply_width) * BYTES_PER_PIXEL16)
        ]);


        /* Begin blitting. */
        while(tar_buf_ptr < tar_buf_end)
        {
            if(tar_buf_ptr >= tar_buf_edge)
            {
                tar_buf_ptr += dnext;
                tar_buf_edge = tar_buf_ptr + tar_apply_width;
                continue;
            }

            /* No transparency check. */

            /*   Add glow to this pixel.
             *   rrr rrggg gggb bbbb
             */
            *tar_buf_ptr++ = PACK8TO16(
                (u_int8_t)MIN(
                    ((*tar_buf_ptr & 0xF800) >> 8) + color.r,
                    0xff
                ),
                (u_int8_t)MIN(
                    ((*tar_buf_ptr & 0x07E0) >> 3) + color.g,
                    0xff
                ),
                (u_int8_t)MIN(
                    ((*tar_buf_ptr & 0x001F) << 3) + color.b,
                    0xff  
                )
            );
        }


        return;
}



void BlitBufGlow32(
        u_int8_t *tar_buf,
        int tar_x,
        int tar_y,
        unsigned int tar_width,
        unsigned int tar_height,
        unsigned int tar_apply_width,
        unsigned int tar_apply_height,
        WColorStruct color
)
{
        int tar_width_bytes;     /* tar_width * BYTES_PER_PIXEL32 */
        int dnext;

        u_int32_t *tar_buf_ptr, *tar_buf_edge, *tar_buf_end;


        /* Error checks. */
        if((tar_buf == NULL) ||
           (tar_width < 1) ||
           (tar_height < 1) ||
           (tar_apply_width < 1) ||
           (tar_apply_height < 1)
        )
            return;


        /* Requested fade area totally off screen? */
        if((tar_x >= (int)tar_width) ||
           (tar_y >= (int)tar_height) ||
           ((tar_x + (int)tar_apply_width) <= 0) ||
           ((tar_y + (int)tar_apply_height) <= 0)
        )
            return;


        /* ************************************************************* */

        /* Calculate bytes per width. */
        tar_width_bytes = (int)tar_width * BYTES_PER_PIXEL32;

        /* Sanitize starts. */
        if(tar_x < 0)
        {
            tar_apply_width = MAX((int)tar_apply_width + tar_x, 0);
            tar_x = 0;
        }
        if(tar_y < 0)
        {
            tar_apply_height = MAX((int)tar_apply_height + tar_y, 0);
            tar_y = 0;
        }

        if(((int)tar_apply_width + tar_x) > (int)tar_width)
            tar_apply_width = (int)tar_width - tar_x;

        if(((int)tar_apply_height + tar_y) > (int)tar_height)
            tar_apply_height = (int)tar_height - tar_y;


        /* Set pointer positions. */
        tar_buf_ptr = (u_int32_t *)(&tar_buf[
            (tar_y * tar_width_bytes) +
            (tar_x * BYTES_PER_PIXEL32)
        ]);
            
        dnext = MAX((int)tar_width - (int)tar_apply_width, 0);
        tar_buf_edge = (u_int32_t *)(tar_buf_ptr + tar_apply_width);

        tar_buf_end = (u_int32_t *)(&tar_buf[
            ((tar_y + (int)tar_apply_height - 1) * tar_width_bytes) +
            ((tar_x + (int)tar_apply_width) * BYTES_PER_PIXEL32)
        ]);


        /* Begin blitting. */
        while(tar_buf_ptr < tar_buf_end)
        {
            if(tar_buf_ptr >= tar_buf_edge)
            {
                tar_buf_ptr += dnext;
                tar_buf_edge = tar_buf_ptr + tar_apply_width;
                continue;
            }

            /* No transparency check. */

            /*   Add glow to this pixel.
             */
            *tar_buf_ptr++ = PACK8TO32(
                (u_int8_t)((*tar_buf_ptr & 0xff000000) >> 24),
                (u_int8_t)MIN(
                    ((*tar_buf_ptr & 0x00ff0000) >> 16) + color.r,
                    0xff
                ),
                (u_int8_t)MIN(
                    ((*tar_buf_ptr & 0x0000ff00) >> 8) + color.g,
                    0xff
                ),
                (u_int8_t)MIN(
                    ((*tar_buf_ptr & 0x000000ff)) + color.b,
                    0xff
                ) 
            );
        }


        return;
}



void BlitBufGlow(
	unsigned int d,
        u_int8_t *tar_buf,
	int tar_x,
	int tar_y,
        unsigned int tar_width,
        unsigned int tar_height,
        unsigned int tar_apply_width,
	unsigned int tar_apply_height,
	WColorStruct color
)
{
	switch(d)
	{
          case 32:
            BlitBufGlow32(
                tar_buf,
                tar_x,
                tar_y,
                tar_width,
                tar_height,
                tar_apply_width,
                tar_apply_height,
		color
            );
            break;

	  case 24:
            BlitBufGlow32(
                tar_buf,
                tar_x,
                tar_y,
                tar_width,
                tar_height,
                tar_apply_width,
                tar_apply_height,
		color
	    );
	    break;

          case 16:
            BlitBufGlow16(
                tar_buf,
                tar_x, 
                tar_y,
                tar_width,
                tar_height,
                tar_apply_width,
                tar_apply_height,
		color
            );
            break;

          case 15:
            BlitBufGlow15(
                tar_buf,
                tar_x, 
                tar_y,
                tar_width,
                tar_height,
                tar_apply_width,
                tar_apply_height,
		color
            );
            break;

	  case 8:
            BlitBufGlow8(
                tar_buf,
                tar_x,
                tar_y,
                tar_width,
                tar_height,
                tar_apply_width,
                tar_apply_height,
                color
            );
            break;
	}


	return;
}



