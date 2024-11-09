// client/blittile.cpp
/*
                   Buffer Blitting: Blit Tile (for backgrounds)

	Functions:

	BlitBufTile8()
	BlitBufTile16()		Same as 15 bits
	BlitBufTile32()
	BlitBufTile()

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

#define WARPTOZERO(a,b)	(((a) >= (b)) ? 0 : (a))




void BlitBufTile8(
        u_int8_t *tar_buf,
        u_int8_t *src_buf,
        int offset_x, int offset_y,
        unsigned int tar_width,
        unsigned int tar_height,
        unsigned int src_width,
        unsigned int src_height,
        double zoom,
	double magnification
)
{
        int src_x_start, src_y_start;

        int tar_x_col, tar_y_row;
        int src_x_col, src_y_row;

        unsigned int src_x_col_inc, src_y_row_inc;

        int tar_width_bytes;     /* tar_width * BYTES_PER_PIXEL8 */
        int src_width_bytes;     /* src_width * BYTES_PER_PIXEL8 */

        int tar_width2, tar_height2;
	u_int8_t	*tar_buf_ptr8, *src_buf_ptr8,
			*tar_buf_edge8, *tar_buf_end8;


        /* Error checks. */
        if((tar_buf == NULL) ||
           (src_buf == NULL) ||
           ((int)tar_width < 1) ||
           ((int)tar_height < 1) ||
           ((int)src_width < 1) ||
           ((int)src_height < 1) ||
           (zoom <= 0)
        )
            return;

        /* Sanitize zoom. */
        if(zoom > 1)
            zoom = 1;
               
        /* Sanitize magnification. */
        if(magnification < 1)
            magnification = 1;


        /* Get widths in units of bytes (must calculate before conversion). */
        tar_width_bytes = (int)tar_width * BYTES_PER_PIXEL8;
        src_width_bytes = (int)src_width * BYTES_PER_PIXEL8;

        /* Calculate zoom applyed target width and height. */
        tar_width2 = static_cast<int>((int)tar_width / zoom);
        tar_height2 = static_cast<int>((int)tar_height / zoom);

        /* Calculate src_x_start and src_y_start. */
        src_x_start = static_cast<int>(((offset_x / zoom) -
            ((int)tar_width2 / 2)) / 
            magnification);
        src_y_start = static_cast<int>(((offset_y / zoom) +
            ((int)tar_height2 / 2)) /
            magnification);


        /* Calculate source starting positions. */
        if(src_x_start < 0)
            src_x_start = (src_x_start % (int)src_width) + (int)src_width;
        else
            src_x_start = src_x_start % (int)src_width;

        if(src_y_start < 0)
            src_y_start = (src_y_start % (int)src_height) * -1;
        else
            src_y_start = (int)src_height - (src_y_start % (int)src_height);

        /* Sanitize source starting positions. */
        if(src_x_start < 0)
            src_x_start = (int)src_width - 1;
        else if(src_x_start >= (int)src_width)
            src_x_start = 0;
        if(src_y_start < 0)
            src_y_start = (int)src_height - 1;
        else if(src_y_start >= (int)src_height)
            src_y_start = 0;


        src_x_col = src_x_start;   
        src_y_row = src_y_start;


        /* Convert source units to * 256. */
        src_x_start <<= 8;  
        src_y_start <<= 8;
        src_width <<= 8;
        src_height <<= 8;
        src_x_col <<= 8;
        src_y_row <<= 8;

        /* Set source increments in units of * 256. */
        src_x_col_inc = MAX((unsigned int)(256 / magnification / zoom), 1);
        src_y_row_inc = MAX((unsigned int)(256 / magnification / zoom), 1);

        /* Set target starting positions. */
        tar_x_col = 0;
        tar_y_row = 0;


        /* Get starting buffer pointers. */
        tar_buf_ptr8 = (u_int8_t *)tar_buf;
        tar_buf_edge8 = (u_int8_t *)(tar_buf_ptr8 + tar_width);
        tar_buf_end8 = (u_int8_t *)(tar_buf_ptr8 +
            (tar_width * tar_height));

        src_buf_ptr8 = (u_int8_t *)(&src_buf[
            ((src_y_row >> 8) * (int)src_width_bytes) + 
            ((src_x_col >> 8) * BYTES_PER_PIXEL8)
        ]);


        /* Begin blitting. */
        while(tar_buf_ptr8 < tar_buf_end8)
        {
            /* Copy pixel and increment tar_buf_ptr8. */ 
            *tar_buf_ptr8++ = *src_buf_ptr8;


            /* Update pixel positions. */
            tar_x_col++;
            src_x_col = WARPTOZERO(
                (int)(src_x_col + (int)src_x_col_inc), 
                (int)src_width
            );

            /* Target new line? */
            if(tar_buf_ptr8 >= tar_buf_edge8)
            {
                tar_buf_edge8 = (u_int8_t *)(tar_buf_edge8 + tar_width);

                src_x_col = src_x_start;        /* Reset to proper start. */
                src_y_row = WARPTOZERO(
                    (int)(src_y_row + (int)src_y_row_inc),
                    (int)src_height
                );
            }

            src_buf_ptr8 = (u_int8_t *)(&src_buf[
                ((src_y_row >> 8) * (int)src_width_bytes) +
                ((src_x_col >> 8) * BYTES_PER_PIXEL8)
            ]);
        }


        return;
}


void BlitBufTile16(
        u_int8_t *tar_buf,
        u_int8_t *src_buf,
        int offset_x, int offset_y,
        unsigned int tar_width,
        unsigned int tar_height,
        unsigned int src_width,
        unsigned int src_height,
        double zoom,
	double magnification
)
{
	int src_x_start, src_y_start;

	int tar_x_col, tar_y_row;
	int src_x_col, src_y_row;

	unsigned int src_x_col_inc, src_y_row_inc;

	int tar_width_bytes;     /* tar_width * BYTES_PER_PIXEL16 */
	int src_width_bytes;     /* src_width * BYTES_PER_PIXEL16 */

	int tar_width2, tar_height2;
	u_int16_t	*tar_buf_ptr16, *src_buf_ptr16,
			*tar_buf_edge16, *tar_buf_end16;


        /* Error checks. */
        if((tar_buf == NULL) ||
           (src_buf == NULL) ||
           ((int)tar_width < 1) ||
           ((int)tar_height < 1) ||
           ((int)src_width < 1) ||
           ((int)src_height < 1) ||
           (zoom <= 0)
        )
            return;

        /* Sanitize zoom. */   
	if(zoom > 1)
	    zoom = 1;

	/* Sanitize magnification. */
	if(magnification < 1)
	    magnification = 1;

        /* Get widths in units of bytes (must calculate before conversion). */
        tar_width_bytes = (int)tar_width * BYTES_PER_PIXEL16;
        src_width_bytes = (int)src_width * BYTES_PER_PIXEL16;

	/* Calculate zoom applyed target width and height. */
	tar_width2 = static_cast<int>((int)tar_width / zoom);
	tar_height2 = static_cast<int>((int)tar_height / zoom);

	/* Calculate src_x_start and src_y_start. */
	src_x_start = static_cast<int>(((offset_x / zoom) -
	    ((int)tar_width2 / 2)) /
            magnification);
	src_y_start = static_cast<int>(((offset_y / zoom) +
	    ((int)tar_height2 / 2)) /
            magnification);


	/* Calculate source starting positions. */
	if(src_x_start < 0)
	    src_x_start = (src_x_start % (int)src_width) + (int)src_width;
	else
	    src_x_start = src_x_start % (int)src_width;

	if(src_y_start < 0)
            src_y_start = (src_y_start % (int)src_height) * -1;
	else
	    src_y_start = (int)src_height - (src_y_start % (int)src_height);

	/* Sanitize source starting positions. */
        if(src_x_start < 0)
            src_x_start = (int)src_width - 1; 
        else if(src_x_start >= (int)src_width)
            src_x_start = 0;
	if(src_y_start < 0)
	    src_y_start = (int)src_height - 1;
	else if(src_y_start >= (int)src_height)
	    src_y_start = 0;


	src_x_col = src_x_start;
	src_y_row = src_y_start;


	/* Convert source units to * 256. */
	src_x_start <<= 8;
	src_y_start <<= 8;
	src_width <<= 8;
	src_height <<= 8;
	src_x_col <<= 8;
	src_y_row <<= 8;

	/* Set source increments in units of * 256. */
	src_x_col_inc = MAX((unsigned int)(256 / magnification / zoom), 1);
	src_y_row_inc = MAX((unsigned int)(256 / magnification / zoom), 1);

	/* Set target starting positions. */
        tar_x_col = 0;
        tar_y_row = 0;


	/* Get starting buffer pointers. */
        tar_buf_ptr16 = (u_int16_t *)tar_buf;
	tar_buf_edge16 = (u_int16_t *)(tar_buf_ptr16 + tar_width);
        tar_buf_end16 = (u_int16_t *)(tar_buf_ptr16 +
	    (tar_width * tar_height));

        src_buf_ptr16 = (u_int16_t *)(&src_buf[
            ((src_y_row >> 8) * (int)src_width_bytes) +
	    ((src_x_col >> 8) * BYTES_PER_PIXEL16)
	]);


	/* Begin blitting. */
        while(tar_buf_ptr16 < tar_buf_end16)
        {
            /* Copy pixel and increment tar_buf_ptr16. */
            *tar_buf_ptr16++ = *src_buf_ptr16;


            /* Update pixel positions. */
            tar_x_col++;
	    src_x_col = WARPTOZERO(
		(int)(src_x_col + (int)src_x_col_inc),
		(int)src_width
	    );

	    /* Target new line? */
            if(tar_buf_ptr16 >= tar_buf_edge16)
            {
		tar_buf_edge16 = (u_int16_t *)(tar_buf_edge16 + tar_width);

                src_x_col = src_x_start;	/* Reset to proper start. */
		src_y_row = WARPTOZERO(
		    (int)(src_y_row + (int)src_y_row_inc),
		    (int)src_height
		);
            }

            src_buf_ptr16 = (u_int16_t *)(&src_buf[
                ((src_y_row >> 8) * (int)src_width_bytes) +
                ((src_x_col >> 8) * BYTES_PER_PIXEL16)
            ]);
        }


        return;
}


void BlitBufTile32(
        u_int8_t *tar_buf,
        u_int8_t *src_buf,
        int offset_x, int offset_y,
        unsigned int tar_width,
        unsigned int tar_height,
        unsigned int src_width,
        unsigned int src_height,
        double zoom,
	double magnification
)
{
        int src_x_start, src_y_start;

        int tar_x_col, tar_y_row;
        int src_x_col, src_y_row;

        unsigned int src_x_col_inc, src_y_row_inc;

        int tar_width_bytes;     /* tar_width * BYTES_PER_PIXEL32 */
        int src_width_bytes;     /* src_width * BYTES_PER_PIXEL32 */

        int tar_width2, tar_height2;
	u_int32_t	*tar_buf_ptr32, *src_buf_ptr32,
			*tar_buf_edge32, *tar_buf_end32;


        /* Error checks. */
        if((tar_buf == NULL) ||
           (src_buf == NULL) ||
           ((int)tar_width < 1) ||
           ((int)tar_height < 1) ||
           ((int)src_width < 1) ||
           ((int)src_height < 1) ||
           (zoom <= 0)
        )
            return;

        /* Sanitize zoom. */
        if(zoom > 1)
            zoom = 1;

        /* Sanitize magnification. */
        if(magnification < 1)
            magnification = 1;


        /* Get widths in units of bytes (must calculate before conversion). */
        tar_width_bytes = (int)tar_width * BYTES_PER_PIXEL32;
        src_width_bytes = (int)src_width * BYTES_PER_PIXEL32;

        /* Calculate zoom applyed target width and height. */
        tar_width2 = static_cast<int>((int)tar_width / zoom);
        tar_height2 = static_cast<int>((int)tar_height / zoom);

        /* Calculate src_x_start and src_y_start. */
        src_x_start = static_cast<int>(((offset_x / zoom) -
            ((int)tar_width2 / 2)) / 
            magnification);
        src_y_start = static_cast<int>(((offset_y / zoom) +
            ((int)tar_height2 / 2)) /
            magnification);


        /* Calculate source starting positions. */
        if(src_x_start < 0)
            src_x_start = (src_x_start % (int)src_width) + (int)src_width;
        else
            src_x_start = src_x_start % (int)src_width;

        if(src_y_start < 0)
            src_y_start = (src_y_start % (int)src_height) * -1;
        else
            src_y_start = (int)src_height - (src_y_start % (int)src_height);

        /* Sanitize source starting positions. */
        if(src_x_start < 0)
            src_x_start = (int)src_width - 1;
        else if(src_x_start >= (int)src_width)
            src_x_start = 0;
        if(src_y_start < 0)
            src_y_start = (int)src_height - 1;
        else if(src_y_start >= (int)src_height)   
            src_y_start = 0;


        src_x_col = src_x_start;   
        src_y_row = src_y_start;


        /* Convert source units to * 256. */
        src_x_start <<= 8;  
        src_y_start <<= 8;
        src_width <<= 8;
        src_height <<= 8;
        src_x_col <<= 8;
        src_y_row <<= 8;

        /* Set source increments in units of * 256. */
        src_x_col_inc = MAX((unsigned int)(256 / magnification / zoom), 1);
        src_y_row_inc = MAX((unsigned int)(256 / magnification / zoom), 1);

        /* Set target starting positions. */
        tar_x_col = 0;
        tar_y_row = 0;


        /* Get starting buffer pointers. */
        tar_buf_ptr32 = (u_int32_t *)tar_buf;
        tar_buf_edge32 = (u_int32_t *)(tar_buf_ptr32 + tar_width);
        tar_buf_end32 = (u_int32_t *)(tar_buf_ptr32 +
            (tar_width * tar_height));

        src_buf_ptr32 = (u_int32_t *)(&src_buf[
            ((src_y_row >> 8) * (int)src_width_bytes) + 
            ((src_x_col >> 8) * BYTES_PER_PIXEL32)
        ]);


        /* Begin blitting. */
        while(tar_buf_ptr32 < tar_buf_end32)
        {
            /* Copy pixel and increment tar_buf_ptr32. */ 
            *tar_buf_ptr32++ = *src_buf_ptr32;


            /* Update pixel positions. */
            tar_x_col++;
            src_x_col = WARPTOZERO(
                (int)(src_x_col + (int)src_x_col_inc), 
                (int)src_width
            );

            /* Target new line? */
            if(tar_buf_ptr32 >= tar_buf_edge32)
            {
                tar_buf_edge32 = (u_int32_t *)(tar_buf_edge32 + tar_width);

                src_x_col = src_x_start;        /* Reset to proper start. */
                src_y_row = WARPTOZERO(
                    (int)(src_y_row + (int)src_y_row_inc),
                    (int)src_height
                );
            }

            src_buf_ptr32 = (u_int32_t *)(&src_buf[
                ((src_y_row >> 8) * (int)src_width_bytes) +
                ((src_x_col >> 8) * BYTES_PER_PIXEL32)
            ]);
        }


        return;
}


void BlitBufTile(
        unsigned int d,
        u_int8_t *tar_buf, 
        u_int8_t *src_buf,
        int offset_x, int offset_y,
        unsigned int tar_width,
        unsigned int tar_height,
        unsigned int src_width,
        unsigned int src_height,
        double zoom,
	double magnification
)
{
	switch(d)
	{
	  case 32:
            BlitBufTile32(
                tar_buf, src_buf,
                offset_x, offset_y,
                tar_width, tar_height,
                src_width, src_height,
                zoom,
		magnification
	    );
	    break;

          case 24:
            BlitBufTile32(
                tar_buf, src_buf,
                offset_x, offset_y,
                tar_width, tar_height,
                src_width, src_height,
                zoom,
		magnification
            );
            break;

	  case 16:
          case 15:
            BlitBufTile16(
                tar_buf, src_buf,
                offset_x, offset_y,
                tar_width, tar_height,
                src_width, src_height,
                zoom,
		magnification
            );
            break;

	  case 8:
	    BlitBufTile8(
		tar_buf, src_buf,
                offset_x, offset_y,
                tar_width, tar_height,
                src_width, src_height,
                zoom,
		magnification
	    );
            break;
	}


	return;
}




