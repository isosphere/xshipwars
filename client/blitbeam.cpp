// client/blitbeam.cpp
/*
                        Buffer Blitting: Beam

	Functions:

	BlitBufBeam8()
	BlitBufBeam15()
        BlitBufBeam16()
        BlitBufBeam32()
        BlitBufBeam()

	---

 */

#include "blitting.h"

#ifndef MAX
#define MIN(a,b)        (((a) < (b)) ? (a) : (b))
#define MAX(a,b)	(((a) > (b)) ? (a) : (b))
#endif
/* #define MIN(a,b)        (((a) < (b)) ? (a) : (b)) */
/* #define MAX(a,b)        (((a) > (b)) ? (a) : (b)) */



void BlitBufBeam8(
        u_int8_t *tar_buf,
        int tar_x, int tar_y,
        unsigned int tar_width, unsigned int tar_height,
        double theta,
        unsigned int len,	/* In pixels. */
        unsigned int broadness,	/* In pixels. */
        WColorStruct color
)
{
	int bytes_per_pixel, bytes_per_line;
	double coeff;
	int cx, cy;		/* Coordinate counts along any ray. */
	int cx_end, cy_end;
	int dx, dy;		/* Deltas along any ray. */
	int ray_cnt, ray_max;
	int quadrant;
	double sin_theta, cos_theta;
	u_int8_t *tar_buf_ptr8;
	WColorStruct c;


        /* Error checks. */
        if((tar_buf == NULL) ||
           (tar_width == 0) ||
           (tar_height == 0)
        )
            return;

        /* Totally not visable on target? */
        if(((tar_x + (int)len + (int)broadness) < 0) ||
           ((tar_y + (int)len + (int)broadness) < 0) ||
           ((tar_x - (int)len - (int)broadness) >= (int)tar_width) ||
           ((tar_y - (int)len - (int)broadness) >= (int)tar_height) ||
           ((int)len <= 0)
        )
            return;

        /* Sanitize theta. */
        while(theta >= 6.283185307)
            theta -= 6.283185307;
        while(theta < 0)
            theta += 6.283185307;

	/* Get sin and cos theta in bearing. */
	sin_theta = sin((PI / 2) - theta);
	cos_theta = cos((PI / 2) - theta);


	/* Calculate pixel sizes. */
	bytes_per_pixel = BYTES_PER_PIXEL8;
	bytes_per_line = bytes_per_pixel * (int)tar_width;


	/* Convert all cartesian measures to * 256. The delta's
	 * dx and dy will have the proper signs for the flipped y
	 * coordinates.
	 */
	tar_x <<= 8;
	tar_y <<= 8;
	tar_width <<= 8;
	tar_height <<= 8;
	dx = static_cast<int>(256 * cos_theta);
	dy = static_cast<int>(-256 * sin_theta);	/* Remember Y coords are flipped. */

	if(theta < (PI * 0.5))
	    quadrant = 1;
	else if(theta < (PI * 1))
	    quadrant = 2;
	else if(theta < (PI * 1.5))
	    quadrant = 3;
	else
	    quadrant = 4;


	for(ray_cnt = (int)broadness / -2, ray_max = (int)broadness / 2;
            ray_cnt < ray_max;
	    ray_cnt++
	)
	{
	    /* Calculate additive color values. */
	    if(ray_max > 0)
	    {
	        if(ray_cnt < 0)
		    coeff = 1 + ((double)ray_cnt / (double)ray_max);
	        else
		    coeff = 1 - ((double)ray_cnt / (double)ray_max);
	    }
	    else
		coeff = 1;
/*	    c.a = 0x00;			Don't need this */
	    c.r = static_cast<u_int8_t>(color.r * coeff);
	    c.g = static_cast<u_int8_t>(color.g * coeff);
	    c.b = static_cast<u_int8_t>(color.b * coeff);

	    /* Calculate current ray starting and ending positions. */
            cx = tar_x + (int)(ray_cnt * sin_theta * 256);
            cy = tar_y + (int)(ray_cnt * cos_theta * 256);

            cx_end = cx + (int)((double)len * cos_theta * 256);
            cy_end = cy - (int)((double)len * sin_theta * 256);

	    /* Blit current line by quadrant. */
	    if(quadrant == 1)		/* Quadrant 1. */
	    {
		/* Blit each point on the line. */
		while((cx <= cx_end) &&
                      (cy >= cy_end)	/* Remember Y coords are flipped. */
		)
		{
		    /* Make sure point is in buffer. */
		    if((cx < 0) ||
                       (cy < 0) ||
                       (cx >= (int)tar_width) ||
                       (cy >= (int)tar_height)
		    )
		    {
			cx += dx;
			cy += dy;	/* Remember Y coords are flipped. */
			continue;
		    }

		    /* Get pointer to buffer position. */
		    tar_buf_ptr8 = (u_int8_t *)(&tar_buf[
			((cy >> 8) * bytes_per_line) +
			((cx >> 8) * bytes_per_pixel)
		    ]);

		    *tar_buf_ptr8 = PACK8TO8(
 (u_int8_t)MIN((int)((*tar_buf_ptr8) & 0xe0) + c.r, 0xff),
 (u_int8_t)MIN((int)(((*tar_buf_ptr8) & 0x1c) << 3) + c.g, 0xff),
 (u_int8_t)MIN((int)(((*tar_buf_ptr8) & 0x03) << 6) + c.b, 0xff)
		    );

		    cx += dx;
		    cy += dy;	/* Remember Y coords are flipped. */
		}
	    }
            else if(quadrant == 2)	/* Quadrant 2. */
            {
                /* Blit each point on the line. */
                while((cx <= cx_end) &&
                      (cy <= cy_end)	/* Remember Y coords are flipped. */
                )
                {
                    /* Make sure point is in buffer. */
                    if((cx < 0) ||
                       (cy < 0) ||
                       (cx >= (int)tar_width) ||
                       (cy >= (int)tar_height)
                    ) 
                    {
                        cx += dx;
                        cy += dy;	/* Remember Y coords are flipped. */
                        continue;
                    }

                    /* Get pointer to buffer position. */
                    tar_buf_ptr8 = (u_int8_t *)(&tar_buf[   
                        ((cy >> 8) * bytes_per_line) +
                        ((cx >> 8) * bytes_per_pixel)
                    ]);

                    *tar_buf_ptr8 = PACK8TO8(
 (u_int8_t)MIN((int)((*tar_buf_ptr8) & 0xe0) + c.r, 0xff),
 (u_int8_t)MIN((int)(((*tar_buf_ptr8) & 0x1c) << 3) + c.g, 0xff),
 (u_int8_t)MIN((int)(((*tar_buf_ptr8) & 0x03) << 6) + c.b, 0xff)
                    );

                    cx += dx;
                    cy += dy;	/* Remember Y coords are flipped. */
                }
	    }
            else if(quadrant == 3)	/* Quadrant 3. */
            {
                /* Blit each point on the line. */
                while((cx >= cx_end) &&
                      (cy <= cy_end)	/* Remember Y coords are flipped. */
                )
                {
                    /* Make sure point is in buffer. */
                    if((cx < 0) ||
                       (cy < 0) ||
                       (cx >= (int)tar_width) ||
                       (cy >= (int)tar_height)
                    )
                    {
                        cx += dx;
                        cy += dy;       /* Remember Y coords are flipped. */
                        continue;
                    }

                    /* Get pointer to buffer position. */
                    tar_buf_ptr8 = (u_int8_t *)(&tar_buf[
                        ((cy >> 8) * bytes_per_line) +
                        ((cx >> 8) * bytes_per_pixel)
                    ]);

                    *tar_buf_ptr8 = PACK8TO8(
 (u_int8_t)MIN((int)((*tar_buf_ptr8) & 0xe0) + c.r, 0xff),
 (u_int8_t)MIN((int)(((*tar_buf_ptr8) & 0x1c) << 3) + c.g, 0xff),
 (u_int8_t)MIN((int)(((*tar_buf_ptr8) & 0x03) << 6) + c.b, 0xff)
                    );

                    cx += dx;
                    cy += dy;   /* Remember Y coords are flipped. */
                }
            }
            else if(quadrant == 4)	/* Quadrant 4. */
            {
                /* Blit each point on the line. */
                while((cx >= cx_end) &&
                      (cy >= cy_end)	/* Remember Y coords are flipped. */
                )
                {
                    /* Make sure point is in buffer. */
                    if((cx < 0) ||
                       (cy < 0) ||
                       (cx >= (int)tar_width) ||
                       (cy >= (int)tar_height)
                    ) 
                    {
                        cx += dx;
                        cy += dy;       /* Remember Y coords are flipped. */
                        continue;
                    }

                    /* Get pointer to buffer position. */
                    tar_buf_ptr8 = (u_int8_t *)(&tar_buf[
                        ((cy >> 8) * bytes_per_line) +
                        ((cx >> 8) * bytes_per_pixel)
                    ]);

                    *tar_buf_ptr8 = PACK8TO8(
 (u_int8_t)MIN((int)((*tar_buf_ptr8) & 0xe0) + c.r, 0xff),
 (u_int8_t)MIN((int)(((*tar_buf_ptr8) & 0x1c) << 3) + c.g, 0xff),
 (u_int8_t)MIN((int)(((*tar_buf_ptr8) & 0x03) << 6) + c.b, 0xff)
                    );

                    cx += dx;
                    cy += dy;   /* Remember Y coords are flipped. */
		}
            }
	}
}

void BlitBufBeam15(
        u_int8_t *tar_buf,
        int tar_x, int tar_y,
        unsigned int tar_width, unsigned int tar_height,
        double theta,
        unsigned int len,	/* In pixels. */
        unsigned int broadness,	/* In pixels. */
        WColorStruct color
)
{
	int bytes_per_pixel, bytes_per_line;
	double coeff;
	int cx, cy;		/* Coordinate counts along any ray. */
	int cx_end, cy_end;
	int dx, dy;		/* Deltas along any ray. */
	int ray_cnt, ray_max;
	int quadrant;
	double sin_theta, cos_theta;
	u_int16_t *tar_buf_ptr16;
	WColorStruct c;


        /* Error checks. */
        if((tar_buf == NULL) ||
           (tar_width == 0) ||
           (tar_height == 0)
        )
            return;

        /* Totally not visable on target? */
        if(((tar_x + (int)len + (int)broadness) < 0) ||
           ((tar_y + (int)len + (int)broadness) < 0) ||
           ((tar_x - (int)len - (int)broadness) >= (int)tar_width) ||
           ((tar_y - (int)len - (int)broadness) >= (int)tar_height) ||
           ((int)len <= 0)
        )
            return;

        /* Sanitize theta. */
        while(theta >= 6.283185307)
            theta -= 6.283185307;
        while(theta < 0)
            theta += 6.283185307;

	/* Get sin and cos theta in bearing. */
	sin_theta = sin((PI / 2) - theta);
	cos_theta = cos((PI / 2) - theta);


	/* Calculate pixel sizes. */
	bytes_per_pixel = BYTES_PER_PIXEL15;
	bytes_per_line = bytes_per_pixel * (int)tar_width;


	/* Convert all cartesian measures to * 256. The delta's
	 * dx and dy will have the proper signs for the flipped y
	 * coordinates.
	 */
	tar_x <<= 8;
	tar_y <<= 8;
	tar_width <<= 8;
	tar_height <<= 8;
	dx = static_cast<int>(256 * cos_theta);
	dy = static_cast<int>(-256 * sin_theta);	/* Remember Y coords are flipped. */

	if(theta < (PI * 0.5))
	    quadrant = 1;
	else if(theta < (PI * 1))
	    quadrant = 2;
	else if(theta < (PI * 1.5))
	    quadrant = 3;
	else
	    quadrant = 4;


	for(ray_cnt = (int)broadness / -2, ray_max = (int)broadness / 2;
            ray_cnt < ray_max;
	    ray_cnt++
	)
	{
	    /* Calculate additive color values. */
	    if(ray_max > 0)
	    {
	        if(ray_cnt < 0)
		    coeff = 1 + ((double)ray_cnt / (double)ray_max);
	        else
		    coeff = 1 - ((double)ray_cnt / (double)ray_max);
	    }
	    else
		coeff = 1;
/*	    c.a = 0x00;			Don't need this */
	    c.r = static_cast<u_int8_t>(color.r * coeff);
	    c.g = static_cast<u_int8_t>(color.g * coeff);
	    c.b = static_cast<u_int8_t>(color.b * coeff);

	    /* Calculate current ray starting and ending positions. */
            cx = tar_x + (int)(ray_cnt * sin_theta * 256);
            cy = tar_y + (int)(ray_cnt * cos_theta * 256);

            cx_end = cx + (int)((double)len * cos_theta * 256);
            cy_end = cy - (int)((double)len * sin_theta * 256);

	    /* Blit current line by quadrant. */
	    if(quadrant == 1)		/* Quadrant 1. */
	    {
		/* Blit each point on the line. */
		while((cx <= cx_end) &&
                      (cy >= cy_end)	/* Remember Y coords are flipped. */
		)
		{
		    /* Make sure point is in buffer. */
		    if((cx < 0) ||
                       (cy < 0) ||
                       (cx >= (int)tar_width) ||
                       (cy >= (int)tar_height)
		    )
		    {
			cx += dx;
			cy += dy;	/* Remember Y coords are flipped. */
			continue;
		    }

		    /* Get pointer to buffer position. */
		    tar_buf_ptr16 = (u_int16_t *)(&tar_buf[
			((cy >> 8) * bytes_per_line) +
			((cx >> 8) * bytes_per_pixel)
		    ]);

		    *tar_buf_ptr16 = PACK8TO15(
 (u_int8_t)MIN((int)(((*tar_buf_ptr16) & 0x7C00) >> 7) + c.r, 0xff),
 (u_int8_t)MIN((int)(((*tar_buf_ptr16) & 0x03E0) >> 2) + c.g, 0xff),
 (u_int8_t)MIN((int)(((*tar_buf_ptr16) & 0x001F) << 3) + c.b, 0xff)
		    );

		    cx += dx;
		    cy += dy;	/* Remember Y coords are flipped. */
		}
	    }
            else if(quadrant == 2)	/* Quadrant 2. */
            {
                /* Blit each point on the line. */
                while((cx <= cx_end) &&
                      (cy <= cy_end)	/* Remember Y coords are flipped. */
                )
                {
                    /* Make sure point is in buffer. */
                    if((cx < 0) ||
                       (cy < 0) ||
                       (cx >= (int)tar_width) ||
                       (cy >= (int)tar_height)
                    ) 
                    {
                        cx += dx;
                        cy += dy;	/* Remember Y coords are flipped. */
                        continue;
                    }

                    /* Get pointer to buffer position. */
                    tar_buf_ptr16 = (u_int16_t *)(&tar_buf[   
                        ((cy >> 8) * bytes_per_line) +
                        ((cx >> 8) * bytes_per_pixel)
                    ]);

                    *tar_buf_ptr16 = PACK8TO15(
 (u_int8_t)MIN((int)(((*tar_buf_ptr16) & 0x7C00) >> 7) + c.r, 0xff),
 (u_int8_t)MIN((int)(((*tar_buf_ptr16) & 0x03E0) >> 2) + c.g, 0xff),
 (u_int8_t)MIN((int)(((*tar_buf_ptr16) & 0x001F) << 3) + c.b, 0xff)
                    );

                    cx += dx;
                    cy += dy;	/* Remember Y coords are flipped. */
                }
	    }
            else if(quadrant == 3)	/* Quadrant 3. */
            {
                /* Blit each point on the line. */
                while((cx >= cx_end) &&
                      (cy <= cy_end)	/* Remember Y coords are flipped. */
                )
                {
                    /* Make sure point is in buffer. */
                    if((cx < 0) ||
                       (cy < 0) ||
                       (cx >= (int)tar_width) ||
                       (cy >= (int)tar_height)
                    )
                    {
                        cx += dx;
                        cy += dy;       /* Remember Y coords are flipped. */
                        continue;
                    }

                    /* Get pointer to buffer position. */
                    tar_buf_ptr16 = (u_int16_t *)(&tar_buf[
                        ((cy >> 8) * bytes_per_line) +
                        ((cx >> 8) * bytes_per_pixel)
                    ]);

                    *tar_buf_ptr16 = PACK8TO15(
 (u_int8_t)MIN((int)(((*tar_buf_ptr16) & 0x7C00) >> 7) + c.r, 0xff),
 (u_int8_t)MIN((int)(((*tar_buf_ptr16) & 0x03E0) >> 2) + c.g, 0xff),
 (u_int8_t)MIN((int)(((*tar_buf_ptr16) & 0x001F) << 3) + c.b, 0xff)
                    );

                    cx += dx;
                    cy += dy;   /* Remember Y coords are flipped. */
                }
            }
            else if(quadrant == 4)	/* Quadrant 4. */
            {
                /* Blit each point on the line. */
                while((cx >= cx_end) &&
                      (cy >= cy_end)	/* Remember Y coords are flipped. */
                )
                {
                    /* Make sure point is in buffer. */
                    if((cx < 0) ||
                       (cy < 0) ||
                       (cx >= (int)tar_width) ||
                       (cy >= (int)tar_height)
                    ) 
                    {
                        cx += dx;
                        cy += dy;       /* Remember Y coords are flipped. */
                        continue;
                    }

                    /* Get pointer to buffer position. */
                    tar_buf_ptr16 = (u_int16_t *)(&tar_buf[
                        ((cy >> 8) * bytes_per_line) +
                        ((cx >> 8) * bytes_per_pixel)
                    ]);

                    *tar_buf_ptr16 = PACK8TO15(
 (u_int8_t)MIN((int)(((*tar_buf_ptr16) & 0x7C00) >> 7) + c.r, 0xff),
 (u_int8_t)MIN((int)(((*tar_buf_ptr16) & 0x03E0) >> 2) + c.g, 0xff),
 (u_int8_t)MIN((int)(((*tar_buf_ptr16) & 0x001F) << 3) + c.b, 0xff)
                    );

                    cx += dx;
                    cy += dy;   /* Remember Y coords are flipped. */
		}
            }
	}
}

void BlitBufBeam16(
        u_int8_t *tar_buf,
        int tar_x, int tar_y,
        unsigned int tar_width, unsigned int tar_height,
        double theta,
        unsigned int len,	/* In pixels. */
        unsigned int broadness,	/* In pixels. */
        WColorStruct color
)
{
	int bytes_per_pixel, bytes_per_line;
	double coeff;
	int cx, cy;		/* Coordinate counts along any ray. */
	int cx_end, cy_end;
	int dx, dy;		/* Deltas along any ray. */
	int ray_cnt, ray_max;
	int quadrant;
	double sin_theta, cos_theta;
	u_int16_t *tar_buf_ptr16;
	WColorStruct c;


        /* Error checks. */
        if((tar_buf == NULL) ||
           (tar_width == 0) ||
           (tar_height == 0)
        )
            return;

        /* Totally not visable on target? */
        if(((tar_x + (int)len + (int)broadness) < 0) ||
           ((tar_y + (int)len + (int)broadness) < 0) ||
           ((tar_x - (int)len - (int)broadness) >= (int)tar_width) ||
           ((tar_y - (int)len - (int)broadness) >= (int)tar_height) ||
           ((int)len <= 0)
        )
            return;

        /* Sanitize theta. */
        while(theta >= 6.283185307)
            theta -= 6.283185307;
        while(theta < 0)
            theta += 6.283185307;

	/* Get sin and cos theta in bearing. */
	sin_theta = sin((PI / 2) - theta);
	cos_theta = cos((PI / 2) - theta);


	/* Calculate pixel sizes. */
	bytes_per_pixel = BYTES_PER_PIXEL16;
	bytes_per_line = bytes_per_pixel * (int)tar_width;


	/* Convert all cartesian measures to * 256. The delta's
	 * dx and dy will have the proper signs for the flipped y
	 * coordinates.
	 */
	tar_x <<= 8;
	tar_y <<= 8;
	tar_width <<= 8;
	tar_height <<= 8;
	dx = static_cast<int>(256 * cos_theta);
	dy = static_cast<int>(-256 * sin_theta);	/* Remember Y coords are flipped. */

	if(theta < (PI * 0.5))
	    quadrant = 1;
	else if(theta < (PI * 1))
	    quadrant = 2;
	else if(theta < (PI * 1.5))
	    quadrant = 3;
	else
	    quadrant = 4;


	for(ray_cnt = (int)broadness / -2, ray_max = (int)broadness / 2;
            ray_cnt < ray_max;
	    ray_cnt++
	)
	{
	    /* Calculate additive color values. */
	    if(ray_max > 0)
	    {
	        if(ray_cnt < 0)
		    coeff = 1 + ((double)ray_cnt / (double)ray_max);
	        else
		    coeff = 1 - ((double)ray_cnt / (double)ray_max);
	    }
	    else
		coeff = 1;
/*	    c.a = 0x00;			Don't need this */
	    c.r = static_cast<u_int8_t>(color.r * coeff);
	    c.g = static_cast<u_int8_t>(color.g * coeff);
	    c.b = static_cast<u_int8_t>(color.b * coeff);

	    /* Calculate current ray starting and ending positions. */
            cx = tar_x + (int)(ray_cnt * sin_theta * 256);
            cy = tar_y + (int)(ray_cnt * cos_theta * 256);

            cx_end = cx + (int)((double)len * cos_theta * 256);
            cy_end = cy - (int)((double)len * sin_theta * 256);

	    /* Blit current line by quadrant. */
	    if(quadrant == 1)		/* Quadrant 1. */
	    {
		/* Blit each point on the line. */
		while((cx <= cx_end) &&
                      (cy >= cy_end)	/* Remember Y coords are flipped. */
		)
		{
		    /* Make sure point is in buffer. */
		    if((cx < 0) ||
                       (cy < 0) ||
                       (cx >= (int)tar_width) ||
                       (cy >= (int)tar_height)
		    )
		    {
			cx += dx;
			cy += dy;	/* Remember Y coords are flipped. */
			continue;
		    }

		    /* Get pointer to buffer position. */
		    tar_buf_ptr16 = (u_int16_t *)(&tar_buf[
			((cy >> 8) * bytes_per_line) +
			((cx >> 8) * bytes_per_pixel)
		    ]);

		    *tar_buf_ptr16 = PACK8TO16(
 (u_int8_t)MIN((int)(((*tar_buf_ptr16) & 0xf800) >> 8) + c.r, 0xff),
 (u_int8_t)MIN((int)(((*tar_buf_ptr16) & 0x07E0) >> 3) + c.g, 0xff),
 (u_int8_t)MIN((int)(((*tar_buf_ptr16) & 0x001F) << 3) + c.b, 0xff)
		    );

		    cx += dx;
		    cy += dy;	/* Remember Y coords are flipped. */
		}
	    }
            else if(quadrant == 2)	/* Quadrant 2. */
            {
                /* Blit each point on the line. */
                while((cx <= cx_end) &&
                      (cy <= cy_end)	/* Remember Y coords are flipped. */
                )
                {
                    /* Make sure point is in buffer. */
                    if((cx < 0) ||
                       (cy < 0) ||
                       (cx >= (int)tar_width) ||
                       (cy >= (int)tar_height)
                    ) 
                    {
                        cx += dx;
                        cy += dy;	/* Remember Y coords are flipped. */
                        continue;
                    }

                    /* Get pointer to buffer position. */
                    tar_buf_ptr16 = (u_int16_t *)(&tar_buf[   
                        ((cy >> 8) * bytes_per_line) +
                        ((cx >> 8) * bytes_per_pixel)
                    ]);

                    *tar_buf_ptr16 = PACK8TO16(
 (u_int8_t)MIN((int)(((*tar_buf_ptr16) & 0xf800) >> 8) + c.r, 0xff),
 (u_int8_t)MIN((int)(((*tar_buf_ptr16) & 0x07E0) >> 3) + c.g, 0xff),
 (u_int8_t)MIN((int)(((*tar_buf_ptr16) & 0x001F) << 3) + c.b, 0xff)
                    );

                    cx += dx;
                    cy += dy;	/* Remember Y coords are flipped. */
                }
	    }
            else if(quadrant == 3)	/* Quadrant 3. */
            {
                /* Blit each point on the line. */
                while((cx >= cx_end) &&
                      (cy <= cy_end)	/* Remember Y coords are flipped. */
                )
                {
                    /* Make sure point is in buffer. */
                    if((cx < 0) ||
                       (cy < 0) ||
                       (cx >= (int)tar_width) ||
                       (cy >= (int)tar_height)
                    )
                    {
                        cx += dx;
                        cy += dy;       /* Remember Y coords are flipped. */
                        continue;
                    }

                    /* Get pointer to buffer position. */
                    tar_buf_ptr16 = (u_int16_t *)(&tar_buf[
                        ((cy >> 8) * bytes_per_line) +
                        ((cx >> 8) * bytes_per_pixel)
                    ]);

                    *tar_buf_ptr16 = PACK8TO16(
 (u_int8_t)MIN((int)(((*tar_buf_ptr16) & 0xf800) >> 8) + c.r, 0xff),
 (u_int8_t)MIN((int)(((*tar_buf_ptr16) & 0x07E0) >> 3) + c.g, 0xff),
 (u_int8_t)MIN((int)(((*tar_buf_ptr16) & 0x001F) << 3) + c.b, 0xff)
                    );

                    cx += dx;
                    cy += dy;   /* Remember Y coords are flipped. */
                }
            }
            else if(quadrant == 4)	/* Quadrant 4. */
            {
                /* Blit each point on the line. */
                while((cx >= cx_end) &&
                      (cy >= cy_end)	/* Remember Y coords are flipped. */
                )
                {
                    /* Make sure point is in buffer. */
                    if((cx < 0) ||
                       (cy < 0) ||
                       (cx >= (int)tar_width) ||
                       (cy >= (int)tar_height)
                    ) 
                    {
                        cx += dx;
                        cy += dy;       /* Remember Y coords are flipped. */
                        continue;
                    }

                    /* Get pointer to buffer position. */
                    tar_buf_ptr16 = (u_int16_t *)(&tar_buf[
                        ((cy >> 8) * bytes_per_line) +
                        ((cx >> 8) * bytes_per_pixel)
                    ]);

                    *tar_buf_ptr16 = PACK8TO16(
 (u_int8_t)MIN((int)(((*tar_buf_ptr16) & 0xf800) >> 8) + c.r, 0xff),
 (u_int8_t)MIN((int)(((*tar_buf_ptr16) & 0x07E0) >> 3) + c.g, 0xff),
 (u_int8_t)MIN((int)(((*tar_buf_ptr16) & 0x001F) << 3) + c.b, 0xff)
                    );

                    cx += dx;
                    cy += dy;   /* Remember Y coords are flipped. */
		}
            }
	}
}


void BlitBufBeam32(
        u_int8_t *tar_buf,
        int tar_x, int tar_y,
        unsigned int tar_width, unsigned int tar_height,
        double theta,
        unsigned int len,	/* In pixels. */
        unsigned int broadness,	/* In pixels. */
        WColorStruct color
)
{
	int bytes_per_pixel, bytes_per_line;
	double coeff;
	int cx, cy;		/* Coordinate counts along any ray. */
	int cx_end, cy_end;
	int dx, dy;		/* Deltas along any ray. */
	int ray_cnt, ray_max;
	int quadrant;
	double sin_theta, cos_theta;
	u_int32_t *tar_buf_ptr32;
	WColorStruct c;


        /* Error checks. */
        if((tar_buf == NULL) ||
           (tar_width == 0) ||
           (tar_height == 0)
        )
            return;

        /* Totally not visable on target? */
        if(((tar_x + (int)len + (int)broadness) < 0) ||
           ((tar_y + (int)len + (int)broadness) < 0) ||
           ((tar_x - (int)len - (int)broadness) >= (int)tar_width) ||
           ((tar_y - (int)len - (int)broadness) >= (int)tar_height) ||
           ((int)len <= 0)
        )
            return;

        /* Sanitize theta. */
        while(theta >= 6.283185307)
            theta -= 6.283185307;
        while(theta < 0)
            theta += 6.283185307;

	/* Get sin and cos theta in bearing. */
	sin_theta = sin((PI / 2) - theta);
	cos_theta = cos((PI / 2) - theta);


	/* Calculate pixel sizes. */
	bytes_per_pixel = BYTES_PER_PIXEL32;
	bytes_per_line = bytes_per_pixel * (int)tar_width;


	/* Convert all cartesian measures to * 256. The delta's
	 * dx and dy will have the proper signs for the flipped y
	 * coordinates.
	 */
	tar_x <<= 8;
	tar_y <<= 8;
	tar_width <<= 8;
	tar_height <<= 8;
	dx = static_cast<int>(256 * cos_theta);
	dy = static_cast<int>(-256 * sin_theta);	/* Remember Y coords are flipped. */

	if(theta < (PI * 0.5))
	    quadrant = 1;
	else if(theta < (PI * 1))
	    quadrant = 2;
	else if(theta < (PI * 1.5))
	    quadrant = 3;
	else
	    quadrant = 4;


	for(ray_cnt = (int)broadness / -2, ray_max = (int)broadness / 2;
            ray_cnt < ray_max;
	    ray_cnt++
	)
	{
	    /* Calculate additive color values. */
	    if(ray_max > 0)
	    {
	        if(ray_cnt < 0)
		    coeff = 1 + ((double)ray_cnt / (double)ray_max);
	        else
		    coeff = 1 - ((double)ray_cnt / (double)ray_max);
	    }
	    else
		coeff = 1;
/*	    c.a = 0x00;			Don't need this */
	    c.r = static_cast<u_int8_t>(color.r * coeff);
	    c.g = static_cast<u_int8_t>(color.g * coeff);
	    c.b = static_cast<u_int8_t>(color.b * coeff);

	    /* Calculate current ray starting and ending positions. */
            cx = tar_x + (int)(ray_cnt * sin_theta * 256);
            cy = tar_y + (int)(ray_cnt * cos_theta * 256);

            cx_end = cx + (int)((double)len * cos_theta * 256);
            cy_end = cy - (int)((double)len * sin_theta * 256);

	    /* Blit current line by quadrant. */
	    if(quadrant == 1)		/* Quadrant 1. */
	    {
		/* Blit each point on the line. */
		while((cx <= cx_end) &&
                      (cy >= cy_end)	/* Remember Y coords are flipped. */
		)
		{
		    /* Make sure point is in buffer. */
		    if((cx < 0) ||
                       (cy < 0) ||
                       (cx >= (int)tar_width) ||
                       (cy >= (int)tar_height)
		    )
		    {
			cx += dx;
			cy += dy;	/* Remember Y coords are flipped. */
			continue;
		    }

		    /* Get pointer to buffer position. */
		    tar_buf_ptr32 = (u_int32_t *)(&tar_buf[
			((cy >> 8) * bytes_per_line) +
			((cx >> 8) * bytes_per_pixel)
		    ]);

		    *tar_buf_ptr32 = PACK8TO32(
 (u_int8_t)(((*tar_buf_ptr32) & 0xff000000) >> 24),
 (u_int8_t)MIN((int)(((*tar_buf_ptr32) & 0x00ff0000) >> 16) + c.r, 0xff),
 (u_int8_t)MIN((int)(((*tar_buf_ptr32) & 0x0000ff00) >> 8) + c.g, 0xff),
 (u_int8_t)MIN((int)((*tar_buf_ptr32) & 0x000000ff) + c.b, 0xff)
		    );

		    cx += dx;
		    cy += dy;	/* Remember Y coords are flipped. */
		}
	    }
            else if(quadrant == 2)	/* Quadrant 2. */
            {
                /* Blit each point on the line. */
                while((cx <= cx_end) &&
                      (cy <= cy_end)	/* Remember Y coords are flipped. */
                )
                {
                    /* Make sure point is in buffer. */
                    if((cx < 0) ||
                       (cy < 0) ||
                       (cx >= (int)tar_width) ||
                       (cy >= (int)tar_height)
                    ) 
                    {
                        cx += dx;
                        cy += dy;	/* Remember Y coords are flipped. */
                        continue;
                    }

                    /* Get pointer to buffer position. */
                    tar_buf_ptr32 = (u_int32_t *)(&tar_buf[   
                        ((cy >> 8) * bytes_per_line) +
                        ((cx >> 8) * bytes_per_pixel)
                    ]);

                    *tar_buf_ptr32 = PACK8TO32(
 (u_int8_t)(((*tar_buf_ptr32) & 0xff000000) >> 24),
 (u_int8_t)MIN((int)(((*tar_buf_ptr32) & 0x00ff0000) >> 16) + c.r, 0xff),
 (u_int8_t)MIN((int)(((*tar_buf_ptr32) & 0x0000ff00) >> 8) + c.g, 0xff),
 (u_int8_t)MIN((int)((*tar_buf_ptr32) & 0x000000ff) + c.b, 0xff)
                    );

                    cx += dx;
                    cy += dy;	/* Remember Y coords are flipped. */
                }
	    }
            else if(quadrant == 3)	/* Quadrant 3. */
            {
                /* Blit each point on the line. */
                while((cx >= cx_end) &&
                      (cy <= cy_end)	/* Remember Y coords are flipped. */
                )
                {
                    /* Make sure point is in buffer. */
                    if((cx < 0) ||
                       (cy < 0) ||
                       (cx >= (int)tar_width) ||
                       (cy >= (int)tar_height)
                    )
                    {
                        cx += dx;
                        cy += dy;       /* Remember Y coords are flipped. */
                        continue;
                    }

                    /* Get pointer to buffer position. */
                    tar_buf_ptr32 = (u_int32_t *)(&tar_buf[
                        ((cy >> 8) * bytes_per_line) +
                        ((cx >> 8) * bytes_per_pixel)
                    ]);

                    *tar_buf_ptr32 = PACK8TO32(
 (u_int8_t)(((*tar_buf_ptr32) & 0xff000000) >> 24),
 (u_int8_t)MIN((int)(((*tar_buf_ptr32) & 0x00ff0000) >> 16) + c.r, 0xff),
 (u_int8_t)MIN((int)(((*tar_buf_ptr32) & 0x0000ff00) >> 8) + c.g, 0xff),
 (u_int8_t)MIN((int)((*tar_buf_ptr32) & 0x000000ff) + c.b, 0xff)
                    );

                    cx += dx;
                    cy += dy;   /* Remember Y coords are flipped. */
                }
            }
            else if(quadrant == 4)	/* Quadrant 4. */
            {
                /* Blit each point on the line. */
                while((cx >= cx_end) &&
                      (cy >= cy_end)	/* Remember Y coords are flipped. */
                )
                {
                    /* Make sure point is in buffer. */
                    if((cx < 0) ||
                       (cy < 0) ||
                       (cx >= (int)tar_width) ||
                       (cy >= (int)tar_height)
                    ) 
                    {
                        cx += dx;
                        cy += dy;       /* Remember Y coords are flipped. */
                        continue;
                    }

                    /* Get pointer to buffer position. */
                    tar_buf_ptr32 = (u_int32_t *)(&tar_buf[
                        ((cy >> 8) * bytes_per_line) +
                        ((cx >> 8) * bytes_per_pixel)
                    ]);

                    *tar_buf_ptr32 = PACK8TO32(
 (u_int8_t)(((*tar_buf_ptr32) & 0xff000000) >> 24),
 (u_int8_t)MIN((int)(((*tar_buf_ptr32) & 0x00ff0000) >> 16) + c.r, 0xff),
 (u_int8_t)MIN((int)(((*tar_buf_ptr32) & 0x0000ff00) >> 8) + c.g, 0xff),
 (u_int8_t)MIN((int)((*tar_buf_ptr32) & 0x000000ff) + c.b, 0xff)
                    );

                    cx += dx;
                    cy += dy;   /* Remember Y coords are flipped. */
		}
            }
	}
}

void BlitBufBeam(
        unsigned int d,
        u_int8_t *tar_buf,
        int tar_x, int tar_y,
        unsigned int tar_width, unsigned int tar_height,
        double theta,
        unsigned int len,	/* In pixels. */
        unsigned int broadness,	/* In pixels. */
        WColorStruct color
)
{
	switch(d)
	{
	  case 32:
	  case 24:
            BlitBufBeam32(
                tar_buf, tar_x, tar_y, tar_width, tar_height,
                theta, len, broadness, color
            );
            break;

	  case 16:
	    BlitBufBeam16(
		tar_buf, tar_x, tar_y, tar_width, tar_height,
		theta, len, broadness, color
	    );
	    break;

          case 15:
            BlitBufBeam15(
                tar_buf, tar_x, tar_y, tar_width, tar_height,
                theta, len, broadness, color
            );
            break;

          case 8:
            BlitBufBeam8(
                tar_buf, tar_x, tar_y, tar_width, tar_height,
                theta, len, broadness, color
            );
            break;
	}

	return;
}



