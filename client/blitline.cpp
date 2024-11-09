// client/blitline.cpp
/*
                Buffer Blitting: Streams (Stream Weapons)

	Functions:

	BlitBufLine8()
	BlitBufLine15()
	BlitBufLine16()
	BlitBufLine32()
	BlitBufLine()

        BlitBufLineAdditive8()
	BlitBufLineAdditive15()
        BlitBufLineAdditive16()
        BlitBufLineAdditive32()
        BlitBufLineAdditive()

	---

	For use with blitting stream weapons.

	Also good for broad lines.

 */

#include "blitting.h"

#ifndef MAX
#define MIN(a,b)        (((a) < (b)) ? (a) : (b))
#define MAX(a,b)	(((a) > (b)) ? (a) : (b))
#endif
/* #define MIN(a,b)        (((a) < (b)) ? (a) : (b)) */
/* #define MAX(a,b)        (((a) > (b)) ? (a) : (b)) */

#define WARPTOP(a,b)	(((a) >= (b)) ? 0 : (a))



void BlitBufLine8(
        u_int8_t *tar_buf,
        int tar_x, int tar_y,
        unsigned int tar_width, unsigned int tar_height,
        double theta,
        unsigned int len,
        unsigned int broadness,
        WColorStruct fg_color
)
{
        double dx, dy;
        double yoverx, xovery; /* y / x and x / y */
        double xcount, ycount;
        double x_current, y_current;
        int x_quad_coeff, y_quad_coeff;
        int bx, by;
        int x_screen, y_screen;

        unsigned int quadrant;         /* 0, 1, 2 or 3. */

        int tar_width_bytes;   /* tar_width * BYTES_PER_PIXEL8 */

        u_int8_t *tar_buf_ptr;


        /* Error checks. */
        if((tar_buf == NULL) ||
           (tar_width == 0) ||
           (tar_height == 0)
        )
            return;

        /* Totally not visable on target? */
        if(((tar_x + (int)len + (int)broadness) < 0) ||
           ((tar_y + (int)len + (int)broadness) < 0) ||
           ((tar_x - (int)len + (int)broadness) >= (int)tar_width) ||
           ((tar_y - (int)len + (int)broadness) >= (int)tar_height)
        )
            return;


        /* Offset tar_x and tar_y based on broadness. */
        tar_x -= (int)((double)broadness / 2);
        tar_y -= (int)((double)broadness / 2);

        /* Sanitize theta. */
        while(theta >= 6.283185307)
            theta -= 6.283185307;
        while(theta < 0)
            theta += 6.283185307;

        /* Sanitize len (can't be too big). */
        if((int)len > 10000)
            len = 10000;

	/* Can't be too wide. */
        if((int)broadness > 1024)
            broadness = 1024;
        else if((int)broadness == 0)
            return;


        /* ************************************************************ */

        tar_width_bytes = (int)tar_width * BYTES_PER_PIXEL8;

        dx = sin(theta) * (double)len;
        dy = cos(theta) * (double)len * -1;

        /* Get quadrant coefficient -1 or 1. */
        x_quad_coeff = (dx < 0) ? -1 : 1;
        y_quad_coeff = (dy < 0) ? -1 : 1;

        /* Check which quadrant (clock wise from upper right). */
        if( (x_quad_coeff >= 0) && (y_quad_coeff < 0) )
            quadrant = 0;
        else if( (x_quad_coeff >= 0) && (y_quad_coeff >= 0) )
            quadrant = 1;
        else if( (x_quad_coeff < 0) && (y_quad_coeff >= 0) )
            quadrant = 2;
        else
            quadrant = 3;

        /* Make dy and dx positive. */
        dx *= (double)x_quad_coeff;
        dy *= (double)y_quad_coeff;

        /* Calculate yoverx and xovery. */
        if(dx == 0)
            yoverx = 1;
        else
            yoverx = MIN((dy / dx), 1);

        if(dy == 0)
            xovery = 1;
        else
            xovery = MIN((dx / dy), 1);

        /* Either yoverx or xovery must have some value. */
        if((yoverx <= 0) && (xovery <= 0))
        {
            yoverx = 0.01;      /* Add some value to it. */
        }


        /* *********************************************************** */

        xcount = 0;
        ycount = 0;
        while((xcount <= dx) &&
              (ycount <= dy)
        )
        {
            x_current = (xcount * (double)x_quad_coeff) + tar_x;
            y_current = (ycount * (double)y_quad_coeff) + tar_y;

            /* Draw by quadrant. */
            switch(quadrant)
            {
              case 0:   /* Upper right. */
                /* Top horizontal. */
                by = 0;
                for(bx = 0; bx < (int)broadness; bx++)
                {
                    x_screen = (int)x_current + bx;
                    y_screen = (int)y_current + by;

                    /* If it is on screen, blit it! */
                    if((x_screen >= 0) &&
                       (y_screen >= 0) &&
                       (x_screen < (int)tar_width) &&
                       (y_screen < (int)tar_height)
                    )
                    {
                        tar_buf_ptr = (u_int8_t *)(&tar_buf[
                            (tar_width_bytes * y_screen) +
                            (BYTES_PER_PIXEL8 * x_screen)
                        ]);
                        *tar_buf_ptr =
                            PACK8TO8(fg_color.r, fg_color.g, fg_color.b);
                    }
                }
                /* Right vertical. */
                bx = broadness - 1;
                for(by = 0; by < (int)broadness; by++)
                {
                    x_screen = (int)x_current + bx;
                    y_screen = (int)y_current + by;

                    /* If it is on screen, blit it! */
                    if((x_screen >= 0) &&
                       (y_screen >= 0) &&
                       (x_screen < (int)tar_width) &&
                       (y_screen < (int)tar_height)
                    )
                    {
                        tar_buf_ptr = (u_int8_t *)(&tar_buf[
                            (tar_width_bytes * y_screen) +
                            (BYTES_PER_PIXEL8 * x_screen)
                        ]);
                        *tar_buf_ptr =
                            PACK8TO8(fg_color.r, fg_color.g, fg_color.b);
                    }
                }
                break;

              case 1:   /* Lower right. */
                /* Lower horizontal. */
                by = broadness - 1;
                for(bx = 0; bx < (int)broadness; bx++)
                {
                    x_screen = (int)x_current + bx;
                    y_screen = (int)y_current + by;

                    /* If it is on screen, blit it! */
                    if((x_screen >= 0) &&
                       (y_screen >= 0) &&
                       (x_screen < (int)tar_width) &&
                       (y_screen < (int)tar_height)
                    )
                    {
                        tar_buf_ptr = (u_int8_t *)(&tar_buf[
                            (tar_width_bytes * y_screen) +
                            (BYTES_PER_PIXEL8 * x_screen)
                        ]);
                        *tar_buf_ptr =
                            PACK8TO8(fg_color.r, fg_color.g, fg_color.b);
                    }
                }
                /* Right vertical. */
                bx = broadness - 1;
                for(by = 0; by < (int)broadness; by++)
                {
                    x_screen = (int)x_current + bx;
                    y_screen = (int)y_current + by;

                    /* If it is on screen, blit it! */
                    if((x_screen >= 0) &&
                       (y_screen >= 0) &&
                       (x_screen < (int)tar_width) &&
                       (y_screen < (int)tar_height)
                    )
                    {
                        tar_buf_ptr = (u_int8_t *)(&tar_buf[
                            (tar_width_bytes * y_screen) +
                            (BYTES_PER_PIXEL8 * x_screen)
                        ]);
                        *tar_buf_ptr =
                            PACK8TO8(fg_color.r, fg_color.g, fg_color.b);
                    }
                }
                break;

              case 2:   /* Lower left. */
                /* Lower horizontal. */
                by = broadness - 1;
                for(bx = 0; bx < (int)broadness; bx++)
                {
                    x_screen = (int)x_current + bx;
                    y_screen = (int)y_current + by;

                    /* If it is on screen, blit it! */
                    if((x_screen >= 0) &&
                       (y_screen >= 0) &&
                       (x_screen < (int)tar_width) &&
                       (y_screen < (int)tar_height)
                    )
                    {
                        tar_buf_ptr = (u_int8_t *)(&tar_buf[
                            (tar_width_bytes * y_screen) +
                            (BYTES_PER_PIXEL8 * x_screen)
                        ]);
                        *tar_buf_ptr =
                            PACK8TO8(fg_color.r, fg_color.g, fg_color.b);
                    }
                }
                /* Left vertical. */
                bx = 0;
                for(by = 0; by < (int)broadness; by++)
                {
                    x_screen = (int)x_current + bx;
                    y_screen = (int)y_current + by;

                    /* If it is on screen, blit it! */
                    if((x_screen >= 0) &&
                       (y_screen >= 0) &&
                       (x_screen < (int)tar_width) &&
                       (y_screen < (int)tar_height)
                    )
                    {
                        tar_buf_ptr = (u_int8_t *)(&tar_buf[
                            (tar_width_bytes * y_screen) +
                            (BYTES_PER_PIXEL8 * x_screen)
                        ]);
                        *tar_buf_ptr =
                            PACK8TO8(fg_color.r, fg_color.g, fg_color.b);
                    }
                }
                break;

              default:   /* Upper left. */
                /* Upper horizontal. */
                by = 0;
                for(bx = 0; bx < (int)broadness; bx++)
                {
                    x_screen = (int)x_current + bx;
                    y_screen = (int)y_current + by;

                    /* If it is on screen, blit it! */
                    if((x_screen >= 0) &&
                       (y_screen >= 0) &&
                       (x_screen < (int)tar_width) &&
                       (y_screen < (int)tar_height)
                    )
                    {
                        tar_buf_ptr = (u_int8_t *)(&tar_buf[
                            (tar_width_bytes * y_screen) +
                            (BYTES_PER_PIXEL8 * x_screen)
                        ]);
                        *tar_buf_ptr =
                            PACK8TO8(fg_color.r, fg_color.g, fg_color.b);
                    }
                }
                /* Left vertical. */
                bx = 0;
                for(by = 0; by < (int)broadness; by++)
                {
                    x_screen = (int)x_current + bx;
                    y_screen = (int)y_current + by;

                    /* If it is on screen, blit it! */
                    if((x_screen >= 0) &&
                       (y_screen >= 0) &&
                       (x_screen < (int)tar_width) &&
                       (y_screen < (int)tar_height)
                    )
                    {
                        tar_buf_ptr = (u_int8_t *)(&tar_buf[
                            (tar_width_bytes * y_screen) +
                            (BYTES_PER_PIXEL8 * x_screen)
                        ]);
                        *tar_buf_ptr =
                            PACK8TO8(fg_color.r, fg_color.g, fg_color.b);
                    }
                }
                break;
            }


            xcount += xovery;
            ycount += yoverx;
        }


        return;
}


void BlitBufLine15(
        u_int8_t *tar_buf,
        int tar_x, int tar_y,
        unsigned int tar_width, unsigned int tar_height,
        double theta,
        unsigned int len,
        unsigned int broadness,
	WColorStruct fg_color
)
{
        double dx, dy;
        double yoverx, xovery; /* y / x and x / y */
        double xcount, ycount;
        double x_current, y_current;
        int x_quad_coeff, y_quad_coeff;
        int bx, by;
        int x_screen, y_screen;
            
        unsigned int quadrant;         /* 0, 1, 2 or 3. */   
            
        int tar_width_bytes;   /* tar_width * BYTES_PER_PIXEL16 */
            
        u_int16_t *tar_buf_ptr;


        /* Error checks. */
        if((tar_buf == NULL) ||
           (tar_width == 0) ||
           (tar_height == 0)
        )
            return;
 
        /* Totally not visable on target? */
        if(((tar_x + (int)len + (int)broadness) < 0) ||
           ((tar_y + (int)len + (int)broadness) < 0) ||
           ((tar_x - (int)len + (int)broadness) >= (int)tar_width) ||
           ((tar_y - (int)len + (int)broadness) >= (int)tar_height)
        )
            return;


        /* Offset tar_x and tar_y based on broadness. */
        tar_x -= (int)((double)broadness / 2);
        tar_y -= (int)((double)broadness / 2);

        /* Sanitize theta. */
        while(theta >= 6.283185307)
            theta -= 6.283185307;
        while(theta < 0)
            theta += 6.283185307;

        /* Sanitize len (can't be too big). */
        if((int)len > 10000)
            len = 10000;

	/* Can't be too broad. */
        if((int)broadness > 1024)
            broadness = 1024;
        else if((int)broadness == 0)
            return;   


        /* ************************************************************ */

        tar_width_bytes = (int)tar_width * BYTES_PER_PIXEL15;
            
        dx = sin(theta) * (double)len;
        dy = cos(theta) * (double)len * -1;
         
        /* Get quadrant coefficient -1 or 1. */
        x_quad_coeff = (dx < 0) ? -1 : 1;
        y_quad_coeff = (dy < 0) ? -1 : 1;
        
        /* Check which quadrant (clock wise from upper right). */
        if((x_quad_coeff >= 0) && (y_quad_coeff < 0))
            quadrant = 0;
        else if((x_quad_coeff >= 0) && (y_quad_coeff >= 0))
            quadrant = 1;
        else if((x_quad_coeff < 0) && (y_quad_coeff >= 0))
            quadrant = 2;
        else
            quadrant = 3;
         
        /* Make dy and dx positive. */
        dx *= (double)x_quad_coeff;
        dy *= (double)y_quad_coeff;
        
        /* Calculate yoverx and xovery. */
        if(dx == 0)
            yoverx = 1;
        else 
            yoverx = MIN((dy / dx), 1);
        
        if(dy == 0)
            xovery = 1;
        else
            xovery = MIN((dx / dy), 1);
            
        /* Either yoverx or xovery must have some value. */
        if((yoverx <= 0) && (xovery <= 0))
        {
            yoverx = 0.01;      /* Add some value to it. */
        }


	/* *********************************************************** */

        xcount = 0;
        ycount = 0;
        while( (xcount <= dx) &&
               (ycount <= dy)
        )
        {
            x_current = (xcount * (double)x_quad_coeff) + tar_x;
            y_current = (ycount * (double)y_quad_coeff) + tar_y;
 
            /* Draw by quadrant. */
            switch(quadrant)
            {
              case 0:   /* Upper right. */
                /* Top horizontal. */
                by = 0;
                for(bx = 0; bx < (int)broadness; bx++)
                {  
                    x_screen = (int)x_current + bx;
                    y_screen = (int)y_current + by;
            
                    /* If it is on screen, blit it! */
                    if((x_screen >= 0) &&
                       (y_screen >= 0) &&
                       (x_screen < (int)tar_width) &&
                       (y_screen < (int)tar_height)
                    )
                    {
                        tar_buf_ptr = (u_int16_t *)(&tar_buf[
                            (tar_width_bytes * y_screen) +
                            (BYTES_PER_PIXEL15 * x_screen) 
                        ]);
                        *tar_buf_ptr =
			    PACK8TO15(fg_color.r, fg_color.g, fg_color.b);
                    }
                }  
                /* Right vertical. */
                bx = broadness - 1;
                for(by = 0; by < (int)broadness; by++)
                {
                    x_screen = (int)x_current + bx;
                    y_screen = (int)y_current + by;
 
                    /* If it is on screen, blit it! */
                    if((x_screen >= 0) &&
                       (y_screen >= 0) &&
                       (x_screen < (int)tar_width) &&
                       (y_screen < (int)tar_height)
                    )  
                    {
                        tar_buf_ptr = (u_int16_t *)(&tar_buf[
                            (tar_width_bytes * y_screen) +
                            (BYTES_PER_PIXEL15 * x_screen)
                        ]);
                        *tar_buf_ptr =
			    PACK8TO15(fg_color.r, fg_color.g, fg_color.b);
                    }
                }
                break;

              case 1:   /* Lower right. */
                /* Lower horizontal. */
                by = broadness - 1;
                for(bx = 0; bx < (int)broadness; bx++)
                {
                    x_screen = (int)x_current + bx;
                    y_screen = (int)y_current + by;
                     
                    /* If it is on screen, blit it! */
                    if((x_screen >= 0) &&
                       (y_screen >= 0) &&
                       (x_screen < (int)tar_width) &&
                       (y_screen < (int)tar_height)
                    )
                    {
                        tar_buf_ptr = (u_int16_t *)(&tar_buf[
                            (tar_width_bytes * y_screen) +
                            (BYTES_PER_PIXEL15 * x_screen)
                        ]);
                        *tar_buf_ptr =
			    PACK8TO15(fg_color.r, fg_color.g, fg_color.b);
                    }
                }
                /* Right vertical. */
                bx = broadness - 1;
                for(by = 0; by < (int)broadness; by++)
                {
                    x_screen = (int)x_current + bx;
                    y_screen = (int)y_current + by;
                     
                    /* If it is on screen, blit it! */
                    if((x_screen >= 0) &&
                       (y_screen >= 0) &&
                       (x_screen < (int)tar_width) &&
                       (y_screen < (int)tar_height)
                    )
                    {
                        tar_buf_ptr = (u_int16_t *)(&tar_buf[
                            (tar_width_bytes * y_screen) +
                            (BYTES_PER_PIXEL15 * x_screen)
                        ]);
                        *tar_buf_ptr =
			    PACK8TO15(fg_color.r, fg_color.g, fg_color.b);
                    }
                }
                break;
                        
              case 2:   /* Lower left. */
                /* Lower horizontal. */
                by = broadness - 1;
                for(bx = 0; bx < (int)broadness; bx++)
                {
                    x_screen = (int)x_current + bx;
                    y_screen = (int)y_current + by;
                     
                    /* If it is on screen, blit it! */
                    if((x_screen >= 0) &&
                       (y_screen >= 0) &&
                       (x_screen < (int)tar_width) &&
                       (y_screen < (int)tar_height)
                    )
                    {
                        tar_buf_ptr = (u_int16_t *)(&tar_buf[
                            (tar_width_bytes * y_screen) +
                            (BYTES_PER_PIXEL15 * x_screen)
                        ]);
                        *tar_buf_ptr =
			    PACK8TO15(fg_color.r, fg_color.g, fg_color.b);
                    }
                }
                /* Left vertical. */
                bx = 0;
                for(by = 0; by < (int)broadness; by++)
                {
                    x_screen = (int)x_current + bx;
                    y_screen = (int)y_current + by;
                     
                    /* If it is on screen, blit it! */
                    if((x_screen >= 0) &&
                       (y_screen >= 0) &&
                       (x_screen < (int)tar_width) &&
                       (y_screen < (int)tar_height)
                    )
                    {
                        tar_buf_ptr = (u_int16_t *)(&tar_buf[
                            (tar_width_bytes * y_screen) +
                            (BYTES_PER_PIXEL15 * x_screen)
                        ]);
                        *tar_buf_ptr =
			    PACK8TO15(fg_color.r, fg_color.g, fg_color.b);
                    }
                }
                break;
                        
              default:   /* Upper left. */
                /* Upper horizontal. */
                by = 0;
                for(bx = 0; bx < (int)broadness; bx++)
                {
                    x_screen = (int)x_current + bx;
                    y_screen = (int)y_current + by;
                     
                    /* If it is on screen, blit it! */
                    if((x_screen >= 0) &&
                       (y_screen >= 0) &&
                       (x_screen < (int)tar_width) &&
                       (y_screen < (int)tar_height)
                    )
                    {
                        tar_buf_ptr = (u_int16_t *)(&tar_buf[
                            (tar_width_bytes * y_screen) +
                            (BYTES_PER_PIXEL15 * x_screen)
                        ]);
                        *tar_buf_ptr =
			    PACK8TO15(fg_color.r, fg_color.g, fg_color.b);
                    }
                }
                /* Left vertical. */
                bx = 0;
                for(by = 0; by < (int)broadness; by++)
                {
                    x_screen = (int)x_current + bx;
                    y_screen = (int)y_current + by;
                     
                    /* If it is on screen, blit it! */
                    if((x_screen >= 0) &&
                       (y_screen >= 0) &&
                       (x_screen < (int)tar_width) &&
                       (y_screen < (int)tar_height)
                    )  
                    {
                        tar_buf_ptr = (u_int16_t *)(&tar_buf[
                            (tar_width_bytes * y_screen) +
                            (BYTES_PER_PIXEL15 * x_screen)
                        ]);
                        *tar_buf_ptr =
			    PACK8TO15(fg_color.r, fg_color.g, fg_color.b);
                    }
                }
                break;
            }

            xcount += xovery;
            ycount += yoverx;
        }


        return;
}

void BlitBufLine16(
        u_int8_t *tar_buf,
        int tar_x, int tar_y,
        unsigned int tar_width, unsigned int tar_height,
        double theta,
        unsigned int len,
        unsigned int broadness,
	WColorStruct fg_color
)
{
        double dx, dy;
        double yoverx, xovery; /* y / x and x / y */
        double xcount, ycount;
        double x_current, y_current;
        int x_quad_coeff, y_quad_coeff;
        int bx, by;
        int x_screen, y_screen;
            
        unsigned int quadrant;         /* 0, 1, 2 or 3. */   
            
        int tar_width_bytes;   /* tar_width * BYTES_PER_PIXEL16 */
            
        u_int16_t *tar_buf_ptr;


        /* Error checks. */
        if((tar_buf == NULL) ||
           (tar_width == 0) ||
           (tar_height == 0)
        )
            return;
 
        /* Totally not visable on target? */
        if(((tar_x + (int)len + (int)broadness) < 0) ||
           ((tar_y + (int)len + (int)broadness) < 0) ||
           ((tar_x - (int)len + (int)broadness) >= (int)tar_width) ||
           ((tar_y - (int)len + (int)broadness) >= (int)tar_height)
        )
            return;


        /* Offset tar_x and tar_y based on broadness. */
        tar_x -= (int)((double)broadness / 2);
        tar_y -= (int)((double)broadness / 2);

        /* Sanitize theta. */
        while(theta >= 6.283185307)
            theta -= 6.283185307;
        while(theta < 0)
            theta += 6.283185307;

        /* Sanitize len (can't be too big). */
        if((int)len > 10000)
            len = 10000;

	/* Can't be too broad. */
        if((int)broadness > 1024)
            broadness = 1024;
        else if((int)broadness == 0)
            return;   


        /* ************************************************************ */

        tar_width_bytes = (int)tar_width * BYTES_PER_PIXEL16;
            
        dx = sin(theta) * (double)len;
        dy = cos(theta) * (double)len * -1;
         
        /* Get quadrant coefficient -1 or 1. */
        x_quad_coeff = (dx < 0) ? -1 : 1;
        y_quad_coeff = (dy < 0) ? -1 : 1;
        
        /* Check which quadrant (clock wise from upper right). */
        if((x_quad_coeff >= 0) && (y_quad_coeff < 0))
            quadrant = 0;
        else if((x_quad_coeff >= 0) && (y_quad_coeff >= 0))
            quadrant = 1;
        else if((x_quad_coeff < 0) && (y_quad_coeff >= 0))
            quadrant = 2;
        else
            quadrant = 3;
         
        /* Make dy and dx positive. */
        dx *= (double)x_quad_coeff;
        dy *= (double)y_quad_coeff;
        
        /* Calculate yoverx and xovery. */
        if(dx == 0)
            yoverx = 1;
        else 
            yoverx = MIN((dy / dx), 1);
        
        if(dy == 0)
            xovery = 1;
        else
            xovery = MIN((dx / dy), 1);
            
        /* Either yoverx or xovery must have some value. */
        if((yoverx <= 0) && (xovery <= 0))
        {
            yoverx = 0.01;      /* Add some value to it. */
        }


	/* *********************************************************** */

        xcount = 0;
        ycount = 0;
        while( (xcount <= dx) &&
               (ycount <= dy)
        )
        {
            x_current = (xcount * (double)x_quad_coeff) + tar_x;
            y_current = (ycount * (double)y_quad_coeff) + tar_y;
 
            /* Draw by quadrant. */
            switch(quadrant)
            {
              case 0:   /* Upper right. */
                /* Top horizontal. */
                by = 0;
                for(bx = 0; bx < (int)broadness; bx++)
                {  
                    x_screen = (int)x_current + bx;
                    y_screen = (int)y_current + by;
            
                    /* If it is on screen, blit it! */
                    if((x_screen >= 0) &&
                       (y_screen >= 0) &&
                       (x_screen < (int)tar_width) &&
                       (y_screen < (int)tar_height)
                    )
                    {
                        tar_buf_ptr = (u_int16_t *)(&tar_buf[
                            (tar_width_bytes * y_screen) +
                            (BYTES_PER_PIXEL16 * x_screen) 
                        ]);
                        *tar_buf_ptr =
			    PACK8TO16(fg_color.r, fg_color.g, fg_color.b);
                    }
                }  
                /* Right vertical. */
                bx = broadness - 1;
                for(by = 0; by < (int)broadness; by++)
                {
                    x_screen = (int)x_current + bx;
                    y_screen = (int)y_current + by;
 
                    /* If it is on screen, blit it! */
                    if((x_screen >= 0) &&
                       (y_screen >= 0) &&
                       (x_screen < (int)tar_width) &&
                       (y_screen < (int)tar_height)
                    )  
                    {
                        tar_buf_ptr = (u_int16_t *)(&tar_buf[
                            (tar_width_bytes * y_screen) +
                            (BYTES_PER_PIXEL16 * x_screen)
                        ]);
                        *tar_buf_ptr =
			    PACK8TO16(fg_color.r, fg_color.g, fg_color.b);
                    }
                }
                break;

              case 1:   /* Lower right. */
                /* Lower horizontal. */
                by = broadness - 1;
                for(bx = 0; bx < (int)broadness; bx++)
                {
                    x_screen = (int)x_current + bx;
                    y_screen = (int)y_current + by;
                     
                    /* If it is on screen, blit it! */
                    if((x_screen >= 0) &&
                       (y_screen >= 0) &&
                       (x_screen < (int)tar_width) &&
                       (y_screen < (int)tar_height)
                    )
                    {
                        tar_buf_ptr = (u_int16_t *)(&tar_buf[
                            (tar_width_bytes * y_screen) +
                            (BYTES_PER_PIXEL16 * x_screen)
                        ]);
                        *tar_buf_ptr =
			    PACK8TO16(fg_color.r, fg_color.g, fg_color.b);
                    }
                }
                /* Right vertical. */
                bx = broadness - 1;
                for(by = 0; by < (int)broadness; by++)
                {
                    x_screen = (int)x_current + bx;
                    y_screen = (int)y_current + by;
                     
                    /* If it is on screen, blit it! */
                    if((x_screen >= 0) &&
                       (y_screen >= 0) &&
                       (x_screen < (int)tar_width) &&
                       (y_screen < (int)tar_height)
                    )
                    {
                        tar_buf_ptr = (u_int16_t *)(&tar_buf[
                            (tar_width_bytes * y_screen) +
                            (BYTES_PER_PIXEL16 * x_screen)
                        ]);
                        *tar_buf_ptr =
			    PACK8TO16(fg_color.r, fg_color.g, fg_color.b);
                    }
                }
                break;
                        
              case 2:   /* Lower left. */
                /* Lower horizontal. */
                by = broadness - 1;
                for(bx = 0; bx < (int)broadness; bx++)
                {
                    x_screen = (int)x_current + bx;
                    y_screen = (int)y_current + by;
                     
                    /* If it is on screen, blit it! */
                    if((x_screen >= 0) &&
                       (y_screen >= 0) &&
                       (x_screen < (int)tar_width) &&
                       (y_screen < (int)tar_height)
                    )
                    {
                        tar_buf_ptr = (u_int16_t *)(&tar_buf[
                            (tar_width_bytes * y_screen) +
                            (BYTES_PER_PIXEL16 * x_screen)
                        ]);
                        *tar_buf_ptr =
			    PACK8TO16(fg_color.r, fg_color.g, fg_color.b);
                    }
                }
                /* Left vertical. */
                bx = 0;
                for(by = 0; by < (int)broadness; by++)
                {
                    x_screen = (int)x_current + bx;
                    y_screen = (int)y_current + by;
                     
                    /* If it is on screen, blit it! */
                    if((x_screen >= 0) &&
                       (y_screen >= 0) &&
                       (x_screen < (int)tar_width) &&
                       (y_screen < (int)tar_height)
                    )
                    {
                        tar_buf_ptr = (u_int16_t *)(&tar_buf[
                            (tar_width_bytes * y_screen) +
                            (BYTES_PER_PIXEL16 * x_screen)
                        ]);
                        *tar_buf_ptr =
			    PACK8TO16(fg_color.r, fg_color.g, fg_color.b);
                    }
                }
                break;
                        
              default:   /* Upper left. */
                /* Upper horizontal. */
                by = 0;
                for(bx = 0; bx < (int)broadness; bx++)
                {
                    x_screen = (int)x_current + bx;
                    y_screen = (int)y_current + by;
                     
                    /* If it is on screen, blit it! */
                    if((x_screen >= 0) &&
                       (y_screen >= 0) &&
                       (x_screen < (int)tar_width) &&
                       (y_screen < (int)tar_height)
                    )
                    {
                        tar_buf_ptr = (u_int16_t *)(&tar_buf[
                            (tar_width_bytes * y_screen) +
                            (BYTES_PER_PIXEL16 * x_screen)
                        ]);
                        *tar_buf_ptr =
			    PACK8TO16(fg_color.r, fg_color.g, fg_color.b);
                    }
                }
                /* Left vertical. */
                bx = 0;
                for(by = 0; by < (int)broadness; by++)
                {
                    x_screen = (int)x_current + bx;
                    y_screen = (int)y_current + by;
                     
                    /* If it is on screen, blit it! */
                    if((x_screen >= 0) &&
                       (y_screen >= 0) &&
                       (x_screen < (int)tar_width) &&
                       (y_screen < (int)tar_height)
                    )  
                    {
                        tar_buf_ptr = (u_int16_t *)(&tar_buf[
                            (tar_width_bytes * y_screen) +
                            (BYTES_PER_PIXEL16 * x_screen)
                        ]);
                        *tar_buf_ptr =
			    PACK8TO16(fg_color.r, fg_color.g, fg_color.b);
                    }
                }
                break;
            }

            xcount += xovery;
            ycount += yoverx;
        }


        return;
}



void BlitBufLine32(
        u_int8_t *tar_buf,
        int tar_x, int tar_y,
        unsigned int tar_width, unsigned int tar_height,
        double theta,
        unsigned int len,
        unsigned int broadness,
        WColorStruct fg_color
)
{
        double dx, dy;
        double yoverx, xovery; /* y / x and x / y */
        double xcount, ycount;
        double x_current, y_current;
        int x_quad_coeff, y_quad_coeff;
        int bx, by;
        int x_screen, y_screen;
            
        unsigned int quadrant;         /* 0, 1, 2 or 3. */   
            
        int tar_width_bytes;   /* tar_width * BYTES_PER_PIXEL32 */
            
        u_int32_t *tar_buf_ptr;


        /* Error checks. */
        if((tar_buf == NULL) ||
           (tar_width == 0) ||
           (tar_height == 0)
        )
            return;
 
        /* Totally not visable on target? */
        if(((tar_x + (int)len + (int)broadness) < 0) ||
           ((tar_y + (int)len + (int)broadness) < 0) ||
           ((tar_x - (int)len + (int)broadness) >= (int)tar_width) ||
           ((tar_y - (int)len + (int)broadness) >= (int)tar_height)
        )
            return;


        /* Offset tar_x and tar_y based on broadness. */
        tar_x -= (int)((double)broadness / 2);
        tar_y -= (int)((double)broadness / 2);
            
        
        /* Sanitize theta. */
        while(theta >= 6.283185307)
            theta -= 6.283185307;
        while(theta < 0)
            theta += 6.283185307;
         
        
        /* Sanitize len (can't be too big). */
        if((int)len > 10000)
            len = 10000;

        /* Can't be too broad. */
        if((int)broadness > 1024)
            broadness = 1024;
        else if((int)broadness == 0)
            return;   
        
            
        /* ************************************************************ */

        tar_width_bytes = (int)tar_width * BYTES_PER_PIXEL32;

        dx = sin(theta) * (double)len;
        dy = cos(theta) * (double)len * -1;

        /* Get quadrant coefficient -1 or 1. */
        x_quad_coeff = (dx < 0) ? -1 : 1;
        y_quad_coeff = (dy < 0) ? -1 : 1;
        
        /* Check which quadrant (clock wise from upper right). */
        if((x_quad_coeff >= 0) && (y_quad_coeff < 0))
            quadrant = 0;
        else if((x_quad_coeff >= 0) && (y_quad_coeff >= 0))
            quadrant = 1;
        else if((x_quad_coeff < 0) && (y_quad_coeff >= 0))
            quadrant = 2;
        else
            quadrant = 3;
         
        /* Make dy and dx positive. */
        dx *= (double)x_quad_coeff;
        dy *= (double)y_quad_coeff;
        
        /* Calculate yoverx and xovery. */
        if(dx == 0)
            yoverx = 1;
        else 
            yoverx = MIN((dy / dx), 1);
        
        if(dy == 0)
            xovery = 1;
        else
            xovery = MIN((dx / dy), 1);
            

        /* Either yoverx or xovery must have some value. */
        if((yoverx <= 0) && (xovery <= 0))
        {
            yoverx = 0.01;      /* Add some value to it. */
        }

	/* ********************************************************* */

        xcount = 0;
        ycount = 0;
        while((xcount <= dx) &&
              (ycount <= dy)
        )
        {
            x_current = (xcount * (double)x_quad_coeff) + tar_x;
            y_current = (ycount * (double)y_quad_coeff) + tar_y;
 
            /* Draw by quadrant. */
            switch(quadrant)
            {
              case 0:   /* Upper right. */
                /* Top horizontal. */
                by = 0;
                for(bx = 0; bx < (int)broadness; bx++)
                {  
                    x_screen = (int)x_current + bx;
                    y_screen = (int)y_current + by;

                    /* If it is on screen, blit it! */
                    if((x_screen >= 0) &&
                       (y_screen >= 0) &&
                       (x_screen < (int)tar_width) &&
                       (y_screen < (int)tar_height)
                    )
                    {
                        tar_buf_ptr = (u_int32_t *)(&tar_buf[
                            (tar_width_bytes * y_screen) +
                            (BYTES_PER_PIXEL32 * x_screen) 
                        ]);
                        *tar_buf_ptr =
                            PACK8TO32(fg_color.a, fg_color.r,
			        fg_color.g, fg_color.b);
                    }
                }  
                /* Right vertical. */
                bx = broadness - 1;
                for(by = 0; by < (int)broadness; by++)
                {
                    x_screen = (int)x_current + bx;
                    y_screen = (int)y_current + by;
 
                    /* If it is on screen, blit it! */
                    if((x_screen >= 0) &&
                       (y_screen >= 0) &&
                       (x_screen < (int)tar_width) &&
                       (y_screen < (int)tar_height)
                    )  
                    {
                        tar_buf_ptr = (u_int32_t *)(&tar_buf[
                            (tar_width_bytes * y_screen) +
                            (BYTES_PER_PIXEL32 * x_screen)
                        ]);
                        *tar_buf_ptr =
                            PACK8TO32(fg_color.a, fg_color.r,
				fg_color.g, fg_color.b);
                    }
                }
                break;

              case 1:   /* Lower right. */
                /* Lower horizontal. */
                by = broadness - 1;
                for(bx = 0; bx < (int)broadness; bx++)
                {
                    x_screen = (int)x_current + bx;
                    y_screen = (int)y_current + by;
                     
                    /* If it is on screen, blit it! */
                    if((x_screen >= 0) &&
                       (y_screen >= 0) &&
                       (x_screen < (int)tar_width) &&
                       (y_screen < (int)tar_height)
                    )
                    {
                        tar_buf_ptr = (u_int32_t *)(&tar_buf[
                            (tar_width_bytes * y_screen) +
                            (BYTES_PER_PIXEL32 * x_screen)
                        ]);
                        *tar_buf_ptr =
                            PACK8TO32(fg_color.a, fg_color.r,
				fg_color.g, fg_color.b);
                    }
                }
                /* Right vertical. */
                bx = broadness - 1;
                for(by = 0; by < (int)broadness; by++)
                {
                    x_screen = (int)x_current + bx;
                    y_screen = (int)y_current + by;
                     
                    /* If it is on screen, blit it! */
                    if((x_screen >= 0) &&
                       (y_screen >= 0) &&
                       (x_screen < (int)tar_width) &&
                       (y_screen < (int)tar_height)
                    )
                    {
                        tar_buf_ptr = (u_int32_t *)(&tar_buf[
                            (tar_width_bytes * y_screen) +
                            (BYTES_PER_PIXEL32 * x_screen)
                        ]);
                        *tar_buf_ptr =
                            PACK8TO32(fg_color.a, fg_color.r,
				fg_color.g, fg_color.b);
                    }
                }
                break;
                        
              case 2:   /* Lower left. */
                /* Lower horizontal. */
                by = broadness - 1;
                for(bx = 0; bx < (int)broadness; bx++)
                {
                    x_screen = (int)x_current + bx;
                    y_screen = (int)y_current + by;
                     
                    /* If it is on screen, blit it! */
                    if((x_screen >= 0) &&
                       (y_screen >= 0) &&
                       (x_screen < (int)tar_width) &&
                       (y_screen < (int)tar_height)
                    )
                    {
                        tar_buf_ptr = (u_int32_t *)(&tar_buf[
                            (tar_width_bytes * y_screen) +
                            (BYTES_PER_PIXEL32 * x_screen)
                        ]);
                        *tar_buf_ptr =
                            PACK8TO32(fg_color.a, fg_color.r,
				fg_color.g, fg_color.b);
                    }
                }
                /* Left vertical. */
                bx = 0;
                for(by = 0; by < (int)broadness; by++)
                {
                    x_screen = (int)x_current + bx;
                    y_screen = (int)y_current + by;
                     
                    /* If it is on screen, blit it! */
                    if((x_screen >= 0) &&
                       (y_screen >= 0) &&
                       (x_screen < (int)tar_width) &&
                       (y_screen < (int)tar_height)
                    )
                    {
                        tar_buf_ptr = (u_int32_t *)(&tar_buf[
                            (tar_width_bytes * y_screen) +
                            (BYTES_PER_PIXEL32 * x_screen)
                        ]);
                        *tar_buf_ptr =
                            PACK8TO32(fg_color.a, fg_color.r,
				fg_color.g, fg_color.b);
                    }
                }
                break;
                        
              default:   /* Upper left. */
                /* Upper horizontal. */
                by = 0;
                for(bx = 0; bx < (int)broadness; bx++)
                {
                    x_screen = (int)x_current + bx;
                    y_screen = (int)y_current + by;
                     
                    /* If it is on screen, blit it! */
                    if((x_screen >= 0) &&
                       (y_screen >= 0) &&
                       (x_screen < (int)tar_width) &&
                       (y_screen < (int)tar_height)
                    )
                    {
                        tar_buf_ptr = (u_int32_t *)(&tar_buf[
                            (tar_width_bytes * y_screen) +
                            (BYTES_PER_PIXEL32 * x_screen)
                        ]);
                        *tar_buf_ptr =
                            PACK8TO32(fg_color.a, fg_color.r,
				fg_color.g, fg_color.b);
                    }
                }
                /* Left vertical. */
                bx = 0;
                for(by = 0; by < (int)broadness; by++)
                {
                    x_screen = (int)x_current + bx;
                    y_screen = (int)y_current + by;
                     
                    /* If it is on screen, blit it! */
                    if((x_screen >= 0) &&
                       (y_screen >= 0) &&
                       (x_screen < (int)tar_width) &&
                       (y_screen < (int)tar_height)
                    )  
                    {
                        tar_buf_ptr = (u_int32_t *)(&tar_buf[
                            (tar_width_bytes * y_screen) +
                            (BYTES_PER_PIXEL32 * x_screen)
                        ]);
                        *tar_buf_ptr =
                            PACK8TO32(fg_color.a, fg_color.r,
				fg_color.g, fg_color.b);
                    }
                }
                break;
            }

            xcount += xovery;
            ycount += yoverx;
        }


        return;
}



void BlitBufLine(
        unsigned int d,
        u_int8_t *tar_buf,
        int tar_x, int tar_y,
        unsigned int tar_width, unsigned int tar_height,
        double theta,
        unsigned int len,
        unsigned int broadness,
        WColorStruct fg_color
)
{
	switch(d)
	{
	  case 32:
            BlitBufLine32(
                tar_buf,
                tar_x,
                tar_y,
                tar_width,
                tar_height,
                theta,
                len,
                broadness,
                fg_color
	    );
	    break;

          case 24:
            BlitBufLine32(
                tar_buf,
                tar_x,
                tar_y,
                tar_width,
                tar_height,
                theta,
                len,
                broadness,
                fg_color
            );
            break;

          case 16:
            BlitBufLine16(
                tar_buf,
                tar_x,
                tar_y,
                tar_width,
                tar_height,
                theta,
                len,
                broadness,
                fg_color
            );
            break;

          case 15:
            BlitBufLine15(
                tar_buf,
                tar_x,
                tar_y,
                tar_width,
                tar_height,
                theta,
                len,
                broadness,
                fg_color
            );
            break;

          case 8:
            BlitBufLine8(
                tar_buf,
                tar_x,
                tar_y,
                tar_width,
                tar_height,
                theta,
                len,
                broadness,
                fg_color
            );
            break;
	}

	return;
}



/*
 *                           Additive
 */
void BlitBufLineAdditive8(
        u_int8_t *tar_buf,
        int tar_x, int tar_y,
        unsigned int tar_width, unsigned int tar_height,
        double theta,
        unsigned int len,
        unsigned int broadness,
        WColorStruct fg_color
)
{
        double dx, dy;
        double yoverx, xovery; /* y / x and x / y */
        double xcount, ycount;
        double x_current, y_current;
        int x_quad_coeff, y_quad_coeff;
        int bx, by;
        int x_screen, y_screen;

        unsigned int quadrant;         /* 0, 1, 2 or 3. */

        int tar_width_bytes;   /* tar_width * BYTES_PER_PIXEL8 */

        u_int8_t *tar_buf_ptr;


        /* Error checks. */
        if((tar_buf == NULL) ||
           (tar_width == 0) ||
           (tar_height == 0)
        )
            return;

        /* Totally not visable on target? */
        if(((tar_x + (int)len + (int)broadness) < 0) ||
           ((tar_y + (int)len + (int)broadness) < 0) ||
           ((tar_x - (int)len + (int)broadness) >= (int)tar_width) ||
           ((tar_y - (int)len + (int)broadness) >= (int)tar_height)
        )
            return;


        /* Offset tar_x and tar_y based on broadness. */
        tar_x -= (int)((double)broadness / 2);
        tar_y -= (int)((double)broadness / 2);

        /* Sanitize theta. */
        while(theta >= 6.283185307)
            theta -= 6.283185307;
        while(theta < 0)
            theta += 6.283185307;

        /* Sanitize len (can't be too big). */
        if((int)len > 10000)
            len = 10000;

	/* Can't be too wide. */
        if((int)broadness > 1024)
            broadness = 1024;
        else if((int)broadness == 0)
            return;


        /* ************************************************************ */

        tar_width_bytes = (int)tar_width * BYTES_PER_PIXEL8;

        dx = sin(theta) * (double)len;
        dy = cos(theta) * (double)len * -1;

        /* Get quadrant coefficient -1 or 1. */
        x_quad_coeff = (dx < 0) ? -1 : 1;
        y_quad_coeff = (dy < 0) ? -1 : 1;

        /* Check which quadrant (clock wise from upper right). */
        if( (x_quad_coeff >= 0) && (y_quad_coeff < 0) )
            quadrant = 0;
        else if( (x_quad_coeff >= 0) && (y_quad_coeff >= 0) )
            quadrant = 1;
        else if( (x_quad_coeff < 0) && (y_quad_coeff >= 0) )
            quadrant = 2;
        else
            quadrant = 3;

        /* Make dy and dx positive. */
        dx *= (double)x_quad_coeff;
        dy *= (double)y_quad_coeff;

        /* Calculate yoverx and xovery. */
        if(dx == 0)
            yoverx = 1;
        else
            yoverx = MIN((dy / dx), 1);

        if(dy == 0)
            xovery = 1;
        else
            xovery = MIN((dx / dy), 1);

        /* Either yoverx or xovery must have some value. */
        if((yoverx <= 0) && (xovery <= 0))
        {
            yoverx = 0.01;      /* Add some value to it. */
        }


        /* *********************************************************** */

        xcount = 0;
        ycount = 0;
        while((xcount <= dx) &&
              (ycount <= dy)
        )
        {
            x_current = (xcount * (double)x_quad_coeff) + tar_x;
            y_current = (ycount * (double)y_quad_coeff) + tar_y;

            /* Draw by quadrant. */
            switch(quadrant)
            {
              case 0:   /* Upper right. */
                /* Top horizontal. */
                by = 0;
                for(bx = 0; bx < (int)broadness; bx++)
                {
                    x_screen = (int)x_current + bx;
                    y_screen = (int)y_current + by;

                    /* If it is on screen, blit it! */
                    if((x_screen >= 0) &&
                       (y_screen >= 0) &&
                       (x_screen < (int)tar_width) &&
                       (y_screen < (int)tar_height)
                    )
                    {
                        tar_buf_ptr = (u_int8_t *)(&tar_buf[
                            (tar_width_bytes * y_screen) +
                            (BYTES_PER_PIXEL8 * x_screen)
                        ]);
                        *tar_buf_ptr =
                            PACK8TO8(
 (u_int8_t)MIN((int)((*tar_buf_ptr) & 0xe0) + fg_color.r, 0xff),
 (u_int8_t)MIN((int)(((*tar_buf_ptr) & 0x1c) << 3) + fg_color.g, 0xff),
 (u_int8_t)MIN((int)(((*tar_buf_ptr) & 0x03) << 6) + fg_color.b, 0xff)
			    );
                    }
                }
                /* Right vertical. */
                bx = broadness - 1;
                for(by = 0; by < (int)broadness; by++)
                {
                    x_screen = (int)x_current + bx;
                    y_screen = (int)y_current + by;

                    /* If it is on screen, blit it! */
                    if((x_screen >= 0) &&
                       (y_screen >= 0) &&
                       (x_screen < (int)tar_width) &&
                       (y_screen < (int)tar_height)
                    )
                    {
                        tar_buf_ptr = (u_int8_t *)(&tar_buf[
                            (tar_width_bytes * y_screen) +
                            (BYTES_PER_PIXEL8 * x_screen)
                        ]);
                        *tar_buf_ptr =
                            PACK8TO8(
 (u_int8_t)MIN((int)((*tar_buf_ptr) & 0xe0) + fg_color.r, 0xff),
 (u_int8_t)MIN((int)(((*tar_buf_ptr) & 0x1c) << 3) + fg_color.g, 0xff),
 (u_int8_t)MIN((int)(((*tar_buf_ptr) & 0x03) << 6) + fg_color.b, 0xff)
			    );
                    }
                }
                break;

              case 1:   /* Lower right. */
                /* Lower horizontal. */
                by = broadness - 1;
                for(bx = 0; bx < (int)broadness; bx++)
                {
                    x_screen = (int)x_current + bx;
                    y_screen = (int)y_current + by;

                    /* If it is on screen, blit it! */
                    if((x_screen >= 0) &&
                       (y_screen >= 0) &&
                       (x_screen < (int)tar_width) &&
                       (y_screen < (int)tar_height)
                    )
                    {
                        tar_buf_ptr = (u_int8_t *)(&tar_buf[
                            (tar_width_bytes * y_screen) +
                            (BYTES_PER_PIXEL8 * x_screen)
                        ]);
                        *tar_buf_ptr =
                            PACK8TO8(
 (u_int8_t)MIN((int)((*tar_buf_ptr) & 0xe0) + fg_color.r, 0xff),
 (u_int8_t)MIN((int)(((*tar_buf_ptr) & 0x1c) << 3) + fg_color.g, 0xff),
 (u_int8_t)MIN((int)(((*tar_buf_ptr) & 0x03) << 6) + fg_color.b, 0xff)
			    );
                    }
                }
                /* Right vertical. */
                bx = broadness - 1;
                for(by = 0; by < (int)broadness; by++)
                {
                    x_screen = (int)x_current + bx;
                    y_screen = (int)y_current + by;

                    /* If it is on screen, blit it! */
                    if((x_screen >= 0) &&
                       (y_screen >= 0) &&
                       (x_screen < (int)tar_width) &&
                       (y_screen < (int)tar_height)
                    )
                    {
                        tar_buf_ptr = (u_int8_t *)(&tar_buf[
                            (tar_width_bytes * y_screen) +
                            (BYTES_PER_PIXEL8 * x_screen)
                        ]);
                        *tar_buf_ptr =
                            PACK8TO8(
 (u_int8_t)MIN((int)((*tar_buf_ptr) & 0xe0) + fg_color.r, 0xff),
 (u_int8_t)MIN((int)(((*tar_buf_ptr) & 0x1c) << 3) + fg_color.g, 0xff),
 (u_int8_t)MIN((int)(((*tar_buf_ptr) & 0x03) << 6) + fg_color.b, 0xff)
			    );
                    }
                }
                break;

              case 2:   /* Lower left. */
                /* Lower horizontal. */
                by = broadness - 1;
                for(bx = 0; bx < (int)broadness; bx++)
                {
                    x_screen = (int)x_current + bx;
                    y_screen = (int)y_current + by;

                    /* If it is on screen, blit it! */
                    if((x_screen >= 0) &&
                       (y_screen >= 0) &&
                       (x_screen < (int)tar_width) &&
                       (y_screen < (int)tar_height)
                    )
                    {
                        tar_buf_ptr = (u_int8_t *)(&tar_buf[
                            (tar_width_bytes * y_screen) +
                            (BYTES_PER_PIXEL8 * x_screen)
                        ]);
                        *tar_buf_ptr =
                            PACK8TO8(
 (u_int8_t)MIN((int)((*tar_buf_ptr) & 0xe0) + fg_color.r, 0xff),
 (u_int8_t)MIN((int)(((*tar_buf_ptr) & 0x1c) << 3) + fg_color.g, 0xff),
 (u_int8_t)MIN((int)(((*tar_buf_ptr) & 0x03) << 6) + fg_color.b, 0xff)
			    );
                    }
                }
                /* Left vertical. */
                bx = 0;
                for(by = 0; by < (int)broadness; by++)
                {
                    x_screen = (int)x_current + bx;
                    y_screen = (int)y_current + by;

                    /* If it is on screen, blit it! */
                    if((x_screen >= 0) &&
                       (y_screen >= 0) &&
                       (x_screen < (int)tar_width) &&
                       (y_screen < (int)tar_height)
                    )
                    {
                        tar_buf_ptr = (u_int8_t *)(&tar_buf[
                            (tar_width_bytes * y_screen) +
                            (BYTES_PER_PIXEL8 * x_screen)
                        ]);
                        *tar_buf_ptr =
                            PACK8TO8(
 (u_int8_t)MIN((int)((*tar_buf_ptr) & 0xe0) + fg_color.r, 0xff),
 (u_int8_t)MIN((int)(((*tar_buf_ptr) & 0x1c) << 3) + fg_color.g, 0xff),
 (u_int8_t)MIN((int)(((*tar_buf_ptr) & 0x03) << 6) + fg_color.b, 0xff)
			    );
                    }
                }
                break;

              default:   /* Upper left. */
                /* Upper horizontal. */
                by = 0;
                for(bx = 0; bx < (int)broadness; bx++)
                {
                    x_screen = (int)x_current + bx;
                    y_screen = (int)y_current + by;

                    /* If it is on screen, blit it! */
                    if((x_screen >= 0) &&
                       (y_screen >= 0) &&
                       (x_screen < (int)tar_width) &&
                       (y_screen < (int)tar_height)
                    )
                    {
                        tar_buf_ptr = (u_int8_t *)(&tar_buf[
                            (tar_width_bytes * y_screen) +
                            (BYTES_PER_PIXEL8 * x_screen)
                        ]);
                        *tar_buf_ptr =
                            PACK8TO8(
 (u_int8_t)MIN((int)((*tar_buf_ptr) & 0xe0) + fg_color.r, 0xff),
 (u_int8_t)MIN((int)(((*tar_buf_ptr) & 0x1c) << 3) + fg_color.g, 0xff),
 (u_int8_t)MIN((int)(((*tar_buf_ptr) & 0x03) << 6) + fg_color.b, 0xff)
			    );
                    }
                }
                /* Left vertical. */
                bx = 0;
                for(by = 0; by < (int)broadness; by++)
                {
                    x_screen = (int)x_current + bx;
                    y_screen = (int)y_current + by;

                    /* If it is on screen, blit it! */
                    if((x_screen >= 0) &&
                       (y_screen >= 0) &&
                       (x_screen < (int)tar_width) &&
                       (y_screen < (int)tar_height)
                    )
                    {
                        tar_buf_ptr = (u_int8_t *)(&tar_buf[
                            (tar_width_bytes * y_screen) +
                            (BYTES_PER_PIXEL8 * x_screen)
                        ]);
                        *tar_buf_ptr =
                            PACK8TO8(
 (u_int8_t)MIN((int)((*tar_buf_ptr) & 0xe0) + fg_color.r, 0xff),
 (u_int8_t)MIN((int)(((*tar_buf_ptr) & 0x1c) << 3) + fg_color.g, 0xff),
 (u_int8_t)MIN((int)(((*tar_buf_ptr) & 0x03) << 6) + fg_color.b, 0xff)
			    );
                    }
                }
                break;
            }

            xcount += xovery;
            ycount += yoverx;
        }


        return;
}


void BlitBufLineAdditive15(
        u_int8_t *tar_buf,
        int tar_x, int tar_y,
        unsigned int tar_width, unsigned int tar_height,
        double theta,
        unsigned int len,
        unsigned int broadness,
	WColorStruct fg_color
)
{
        double dx, dy;
        double yoverx, xovery; /* y / x and x / y */
        double xcount, ycount;
        double x_current, y_current;
        int x_quad_coeff, y_quad_coeff;
        int bx, by;
        int x_screen, y_screen;
            
        unsigned int quadrant;         /* 0, 1, 2 or 3. */   
            
        int tar_width_bytes;   /* tar_width * BYTES_PER_PIXEL16 */
            
        u_int16_t *tar_buf_ptr;


        /* Error checks. */
        if((tar_buf == NULL) ||
           (tar_width == 0) ||
           (tar_height == 0)
        )
            return;
 
        /* Totally not visable on target? */
        if(((tar_x + (int)len + (int)broadness) < 0) ||
           ((tar_y + (int)len + (int)broadness) < 0) ||
           ((tar_x - (int)len + (int)broadness) >= (int)tar_width) ||
           ((tar_y - (int)len + (int)broadness) >= (int)tar_height)
        )
            return;


        /* Offset tar_x and tar_y based on broadness. */
        tar_x -= (int)((double)broadness / 2);
        tar_y -= (int)((double)broadness / 2);

        /* Sanitize theta. */
        while(theta >= 6.283185307)
            theta -= 6.283185307;
        while(theta < 0)
            theta += 6.283185307;

        /* Sanitize len (can't be too big). */
        if((int)len > 10000)
            len = 10000;

	/* Can't be too broad. */
        if((int)broadness > 1024)
            broadness = 1024;
        else if((int)broadness == 0)
            return;   


        /* ************************************************************ */

        tar_width_bytes = (int)tar_width * BYTES_PER_PIXEL15;
            
        dx = sin(theta) * (double)len;
        dy = cos(theta) * (double)len * -1;
         
        /* Get quadrant coefficient -1 or 1. */
        x_quad_coeff = (dx < 0) ? -1 : 1;
        y_quad_coeff = (dy < 0) ? -1 : 1;
        
        /* Check which quadrant (clock wise from upper right). */
        if((x_quad_coeff >= 0) && (y_quad_coeff < 0))
            quadrant = 0;
        else if((x_quad_coeff >= 0) && (y_quad_coeff >= 0))
            quadrant = 1;
        else if((x_quad_coeff < 0) && (y_quad_coeff >= 0))
            quadrant = 2;
        else
            quadrant = 3;
         
        /* Make dy and dx positive. */
        dx *= (double)x_quad_coeff;
        dy *= (double)y_quad_coeff;
        
        /* Calculate yoverx and xovery. */
        if(dx == 0)
            yoverx = 1;
        else 
            yoverx = MIN((dy / dx), 1);
        
        if(dy == 0)
            xovery = 1;
        else
            xovery = MIN((dx / dy), 1);
            
        /* Either yoverx or xovery must have some value. */
        if((yoverx <= 0) && (xovery <= 0))
        {
            yoverx = 0.01;      /* Add some value to it. */
        }


	/* *********************************************************** */

        xcount = 0;
        ycount = 0;
        while( (xcount <= dx) &&
               (ycount <= dy)
        )
        {
            x_current = (xcount * (double)x_quad_coeff) + tar_x;
            y_current = (ycount * (double)y_quad_coeff) + tar_y;

            /* Draw by quadrant. */
            switch(quadrant)
            {
              case 0:   /* Upper right. */
                /* Top horizontal. */
                by = 0;
                for(bx = 0; bx < (int)broadness; bx++)
                {  
                    x_screen = (int)x_current + bx;
                    y_screen = (int)y_current + by;

                    /* If it is on screen, blit it! */
                    if((x_screen >= 0) &&
                       (y_screen >= 0) &&
                       (x_screen < (int)tar_width) &&
                       (y_screen < (int)tar_height)
                    )
                    {
                        tar_buf_ptr = (u_int16_t *)(&tar_buf[
                            (tar_width_bytes * y_screen) +
                            (BYTES_PER_PIXEL15 * x_screen) 
                        ]);
                        *tar_buf_ptr =
			    PACK8TO15(
 (u_int8_t)MIN((int)(((*tar_buf_ptr) & 0x7C00) >> 7) + fg_color.r, 0xff),
 (u_int8_t)MIN((int)(((*tar_buf_ptr) & 0x03E0) >> 2) + fg_color.g, 0xff),
 (u_int8_t)MIN((int)(((*tar_buf_ptr) & 0x001F) << 3) + fg_color.b, 0xff)
			    );
                    }
                }  
                /* Right vertical. */
                bx = broadness - 1;
                for(by = 0; by < (int)broadness; by++)
                {
                    x_screen = (int)x_current + bx;
                    y_screen = (int)y_current + by;

                    /* If it is on screen, blit it! */
                    if((x_screen >= 0) &&
                       (y_screen >= 0) &&
                       (x_screen < (int)tar_width) &&
                       (y_screen < (int)tar_height)
                    )  
                    {
                        tar_buf_ptr = (u_int16_t *)(&tar_buf[
                            (tar_width_bytes * y_screen) +
                            (BYTES_PER_PIXEL15 * x_screen)
                        ]);
                        *tar_buf_ptr =
			    PACK8TO15(
 (u_int8_t)MIN((int)(((*tar_buf_ptr) & 0x7C00) >> 7) + fg_color.r, 0xff),
 (u_int8_t)MIN((int)(((*tar_buf_ptr) & 0x03E0) >> 2) + fg_color.g, 0xff),    
 (u_int8_t)MIN((int)(((*tar_buf_ptr) & 0x001F) << 3) + fg_color.b, 0xff)
			    );
                    }
                }
                break;

              case 1:   /* Lower right. */
                /* Lower horizontal. */
                by = broadness - 1;
                for(bx = 0; bx < (int)broadness; bx++)
                {
                    x_screen = (int)x_current + bx;
                    y_screen = (int)y_current + by;

                    /* If it is on screen, blit it! */
                    if((x_screen >= 0) &&
                       (y_screen >= 0) &&
                       (x_screen < (int)tar_width) &&
                       (y_screen < (int)tar_height)
                    )
                    {
                        tar_buf_ptr = (u_int16_t *)(&tar_buf[
                            (tar_width_bytes * y_screen) +
                            (BYTES_PER_PIXEL15 * x_screen)
                        ]);
                        *tar_buf_ptr =
			    PACK8TO15(
 (u_int8_t)MIN((int)(((*tar_buf_ptr) & 0x7C00) >> 7) + fg_color.r, 0xff),
 (u_int8_t)MIN((int)(((*tar_buf_ptr) & 0x03C0) >> 2) + fg_color.g, 0xff),
 (u_int8_t)MIN((int)(((*tar_buf_ptr) & 0x001F) << 3) + fg_color.b, 0xff)
			    );
                    }
                }
                /* Right vertical. */
                bx = broadness - 1;
                for(by = 0; by < (int)broadness; by++)
                {
                    x_screen = (int)x_current + bx;
                    y_screen = (int)y_current + by;

                    /* If it is on screen, blit it! */
                    if((x_screen >= 0) &&
                       (y_screen >= 0) &&
                       (x_screen < (int)tar_width) &&
                       (y_screen < (int)tar_height)
                    )
                    {
                        tar_buf_ptr = (u_int16_t *)(&tar_buf[
                            (tar_width_bytes * y_screen) +
                            (BYTES_PER_PIXEL15 * x_screen)
                        ]);
                        *tar_buf_ptr =
			    PACK8TO15(
 (u_int8_t)MIN((int)(((*tar_buf_ptr) & 0x7C00) >> 7) + fg_color.r, 0xff),
 (u_int8_t)MIN((int)(((*tar_buf_ptr) & 0x07E0) >> 2) + fg_color.g, 0xff),
 (u_int8_t)MIN((int)(((*tar_buf_ptr) & 0x001F) << 3) + fg_color.b, 0xff)
			    );
                    }
                }
                break;

              case 2:   /* Lower left. */
                /* Lower horizontal. */
                by = broadness - 1;
                for(bx = 0; bx < (int)broadness; bx++)
                {
                    x_screen = (int)x_current + bx;
                    y_screen = (int)y_current + by;

                    /* If it is on screen, blit it! */
                    if((x_screen >= 0) &&
                       (y_screen >= 0) &&
                       (x_screen < (int)tar_width) &&
                       (y_screen < (int)tar_height)
                    )
                    {
                        tar_buf_ptr = (u_int16_t *)(&tar_buf[
                            (tar_width_bytes * y_screen) +
                            (BYTES_PER_PIXEL15 * x_screen)
                        ]);
                        *tar_buf_ptr =
			    PACK8TO15(
 (u_int8_t)MIN((int)(((*tar_buf_ptr) & 0xf800) >> 8) + fg_color.r, 0xff),
 (u_int8_t)MIN((int)(((*tar_buf_ptr) & 0x07E0) >> 3) + fg_color.g, 0xff),    
 (u_int8_t)MIN((int)(((*tar_buf_ptr) & 0x001F) << 3) + fg_color.b, 0xff)
			    );
                    }
                }
                /* Left vertical. */
                bx = 0;
                for(by = 0; by < (int)broadness; by++)
                {
                    x_screen = (int)x_current + bx;
                    y_screen = (int)y_current + by;

                    /* If it is on screen, blit it! */
                    if((x_screen >= 0) &&
                       (y_screen >= 0) &&
                       (x_screen < (int)tar_width) &&
                       (y_screen < (int)tar_height)
                    )
                    {
                        tar_buf_ptr = (u_int16_t *)(&tar_buf[
                            (tar_width_bytes * y_screen) +
                            (BYTES_PER_PIXEL15 * x_screen)
                        ]);
                        *tar_buf_ptr =
			    PACK8TO15(
 (u_int8_t)MIN((int)(((*tar_buf_ptr) & 0x7C00) >> 8) + fg_color.r, 0xff),
 (u_int8_t)MIN((int)(((*tar_buf_ptr) & 0x03E0) >> 3) + fg_color.g, 0xff),    
 (u_int8_t)MIN((int)(((*tar_buf_ptr) & 0x001F) << 3) + fg_color.b, 0xff)
			    );
                    }
                }
                break;

              default:   /* Upper left. */
                /* Upper horizontal. */
                by = 0;
                for(bx = 0; bx < (int)broadness; bx++)
                {
                    x_screen = (int)x_current + bx;
                    y_screen = (int)y_current + by;

                    /* If it is on screen, blit it! */
                    if((x_screen >= 0) &&
                       (y_screen >= 0) &&
                       (x_screen < (int)tar_width) &&
                       (y_screen < (int)tar_height)
                    )
                    {
                        tar_buf_ptr = (u_int16_t *)(&tar_buf[
                            (tar_width_bytes * y_screen) +
                            (BYTES_PER_PIXEL15 * x_screen)
                        ]);
                        *tar_buf_ptr =
			    PACK8TO15(fg_color.r, fg_color.g, fg_color.b);
                    }
                }
                /* Left vertical. */
                bx = 0;
                for(by = 0; by < (int)broadness; by++)
                {
                    x_screen = (int)x_current + bx;
                    y_screen = (int)y_current + by;

                    /* If it is on screen, blit it! */
                    if((x_screen >= 0) &&
                       (y_screen >= 0) &&
                       (x_screen < (int)tar_width) &&
                       (y_screen < (int)tar_height)
                    )  
                    {
                        tar_buf_ptr = (u_int16_t *)(&tar_buf[
                            (tar_width_bytes * y_screen) +
                            (BYTES_PER_PIXEL15 * x_screen)
                        ]);
                        *tar_buf_ptr =
			    PACK8TO15(
 (u_int8_t)MIN((int)(((*tar_buf_ptr) & 0x7C00) >> 7) + fg_color.r, 0xff),
 (u_int8_t)MIN((int)(((*tar_buf_ptr) & 0x03E0) >> 2) + fg_color.g, 0xff),    
 (u_int8_t)MIN((int)(((*tar_buf_ptr) & 0x001F) << 3) + fg_color.b, 0xff)
			    );
                    }
                }
                break;
            }

            xcount += xovery;
            ycount += yoverx;
        }


        return;
}


void BlitBufLineAdditive16(
        u_int8_t *tar_buf,
        int tar_x, int tar_y,
        unsigned int tar_width, unsigned int tar_height,
        double theta,
        unsigned int len,
        unsigned int broadness,
	WColorStruct fg_color
)
{
        double dx, dy;
        double yoverx, xovery; /* y / x and x / y */
        double xcount, ycount;
        double x_current, y_current;
        int x_quad_coeff, y_quad_coeff;
        int bx, by;
        int x_screen, y_screen;
            
        unsigned int quadrant;         /* 0, 1, 2 or 3. */   
            
        int tar_width_bytes;   /* tar_width * BYTES_PER_PIXEL16 */
            
        u_int16_t *tar_buf_ptr;


        /* Error checks. */
        if((tar_buf == NULL) ||
           (tar_width == 0) ||
           (tar_height == 0)
        )
            return;
 
        /* Totally not visable on target? */
        if(((tar_x + (int)len + (int)broadness) < 0) ||
           ((tar_y + (int)len + (int)broadness) < 0) ||
           ((tar_x - (int)len + (int)broadness) >= (int)tar_width) ||
           ((tar_y - (int)len + (int)broadness) >= (int)tar_height)
        )
            return;


        /* Offset tar_x and tar_y based on broadness. */
        tar_x -= (int)((double)broadness / 2);
        tar_y -= (int)((double)broadness / 2);

        /* Sanitize theta. */
        while(theta >= 6.283185307)
            theta -= 6.283185307;
        while(theta < 0)
            theta += 6.283185307;

        /* Sanitize len (can't be too big). */
        if((int)len > 10000)
            len = 10000;

	/* Can't be too broad. */
        if((int)broadness > 1024)
            broadness = 1024;
        else if((int)broadness == 0)
            return;   


        /* ************************************************************ */

        tar_width_bytes = (int)tar_width * BYTES_PER_PIXEL16;
            
        dx = sin(theta) * (double)len;
        dy = cos(theta) * (double)len * -1;
         
        /* Get quadrant coefficient -1 or 1. */
        x_quad_coeff = (dx < 0) ? -1 : 1;
        y_quad_coeff = (dy < 0) ? -1 : 1;
        
        /* Check which quadrant (clock wise from upper right). */
        if((x_quad_coeff >= 0) && (y_quad_coeff < 0))
            quadrant = 0;
        else if((x_quad_coeff >= 0) && (y_quad_coeff >= 0))
            quadrant = 1;
        else if((x_quad_coeff < 0) && (y_quad_coeff >= 0))
            quadrant = 2;
        else
            quadrant = 3;
         
        /* Make dy and dx positive. */
        dx *= (double)x_quad_coeff;
        dy *= (double)y_quad_coeff;
        
        /* Calculate yoverx and xovery. */
        if(dx == 0)
            yoverx = 1;
        else 
            yoverx = MIN((dy / dx), 1);
        
        if(dy == 0)
            xovery = 1;
        else
            xovery = MIN((dx / dy), 1);
            
        /* Either yoverx or xovery must have some value. */
        if((yoverx <= 0) && (xovery <= 0))
        {
            yoverx = 0.01;      /* Add some value to it. */
        }


	/* *********************************************************** */

        xcount = 0;
        ycount = 0;
        while( (xcount <= dx) &&
               (ycount <= dy)
        )
        {
            x_current = (xcount * (double)x_quad_coeff) + tar_x;
            y_current = (ycount * (double)y_quad_coeff) + tar_y;

            /* Draw by quadrant. */
            switch(quadrant)
            {
              case 0:   /* Upper right. */
                /* Top horizontal. */
                by = 0;
                for(bx = 0; bx < (int)broadness; bx++)
                {  
                    x_screen = (int)x_current + bx;
                    y_screen = (int)y_current + by;

                    /* If it is on screen, blit it! */
                    if((x_screen >= 0) &&
                       (y_screen >= 0) &&
                       (x_screen < (int)tar_width) &&
                       (y_screen < (int)tar_height)
                    )
                    {
                        tar_buf_ptr = (u_int16_t *)(&tar_buf[
                            (tar_width_bytes * y_screen) +
                            (BYTES_PER_PIXEL16 * x_screen) 
                        ]);
                        *tar_buf_ptr =
			    PACK8TO16(
 (u_int8_t)MIN((int)(((*tar_buf_ptr) & 0xf800) >> 8) + fg_color.r, 0xff),
 (u_int8_t)MIN((int)(((*tar_buf_ptr) & 0x07E0) >> 3) + fg_color.g, 0xff),
 (u_int8_t)MIN((int)(((*tar_buf_ptr) & 0x001F) << 3) + fg_color.b, 0xff)
			    );
                    }
                }  
                /* Right vertical. */
                bx = broadness - 1;
                for(by = 0; by < (int)broadness; by++)
                {
                    x_screen = (int)x_current + bx;
                    y_screen = (int)y_current + by;

                    /* If it is on screen, blit it! */
                    if((x_screen >= 0) &&
                       (y_screen >= 0) &&
                       (x_screen < (int)tar_width) &&
                       (y_screen < (int)tar_height)
                    )  
                    {
                        tar_buf_ptr = (u_int16_t *)(&tar_buf[
                            (tar_width_bytes * y_screen) +
                            (BYTES_PER_PIXEL16 * x_screen)
                        ]);
                        *tar_buf_ptr =
			    PACK8TO16(
 (u_int8_t)MIN((int)(((*tar_buf_ptr) & 0xf800) >> 8) + fg_color.r, 0xff),
 (u_int8_t)MIN((int)(((*tar_buf_ptr) & 0x07E0) >> 3) + fg_color.g, 0xff),    
 (u_int8_t)MIN((int)(((*tar_buf_ptr) & 0x001F) << 3) + fg_color.b, 0xff)
			    );
                    }
                }
                break;

              case 1:   /* Lower right. */
                /* Lower horizontal. */
                by = broadness - 1;
                for(bx = 0; bx < (int)broadness; bx++)
                {
                    x_screen = (int)x_current + bx;
                    y_screen = (int)y_current + by;

                    /* If it is on screen, blit it! */
                    if((x_screen >= 0) &&
                       (y_screen >= 0) &&
                       (x_screen < (int)tar_width) &&
                       (y_screen < (int)tar_height)
                    )
                    {
                        tar_buf_ptr = (u_int16_t *)(&tar_buf[
                            (tar_width_bytes * y_screen) +
                            (BYTES_PER_PIXEL16 * x_screen)
                        ]);
                        *tar_buf_ptr =
			    PACK8TO16(
 (u_int8_t)MIN((int)(((*tar_buf_ptr) & 0xf800) >> 8) + fg_color.r, 0xff),
 (u_int8_t)MIN((int)(((*tar_buf_ptr) & 0x07E0) >> 3) + fg_color.g, 0xff),
 (u_int8_t)MIN((int)(((*tar_buf_ptr) & 0x001F) << 3) + fg_color.b, 0xff)
			    );
                    }
                }
                /* Right vertical. */
                bx = broadness - 1;
                for(by = 0; by < (int)broadness; by++)
                {
                    x_screen = (int)x_current + bx;
                    y_screen = (int)y_current + by;

                    /* If it is on screen, blit it! */
                    if((x_screen >= 0) &&
                       (y_screen >= 0) &&
                       (x_screen < (int)tar_width) &&
                       (y_screen < (int)tar_height)
                    )
                    {
                        tar_buf_ptr = (u_int16_t *)(&tar_buf[
                            (tar_width_bytes * y_screen) +
                            (BYTES_PER_PIXEL16 * x_screen)
                        ]);
                        *tar_buf_ptr =
			    PACK8TO16(
 (u_int8_t)MIN((int)(((*tar_buf_ptr) & 0xf800) >> 8) + fg_color.r, 0xff),
 (u_int8_t)MIN((int)(((*tar_buf_ptr) & 0x07E0) >> 3) + fg_color.g, 0xff),
 (u_int8_t)MIN((int)(((*tar_buf_ptr) & 0x001F) << 3) + fg_color.b, 0xff)
			    );
                    }
                }
                break;

              case 2:   /* Lower left. */
                /* Lower horizontal. */
                by = broadness - 1;
                for(bx = 0; bx < (int)broadness; bx++)
                {
                    x_screen = (int)x_current + bx;
                    y_screen = (int)y_current + by;

                    /* If it is on screen, blit it! */
                    if((x_screen >= 0) &&
                       (y_screen >= 0) &&
                       (x_screen < (int)tar_width) &&
                       (y_screen < (int)tar_height)
                    )
                    {
                        tar_buf_ptr = (u_int16_t *)(&tar_buf[
                            (tar_width_bytes * y_screen) +
                            (BYTES_PER_PIXEL16 * x_screen)
                        ]);
                        *tar_buf_ptr =
			    PACK8TO16(
 (u_int8_t)MIN((int)(((*tar_buf_ptr) & 0xf800) >> 8) + fg_color.r, 0xff),
 (u_int8_t)MIN((int)(((*tar_buf_ptr) & 0x07E0) >> 3) + fg_color.g, 0xff),    
 (u_int8_t)MIN((int)(((*tar_buf_ptr) & 0x001F) << 3) + fg_color.b, 0xff)
			    );
                    }
                }
                /* Left vertical. */
                bx = 0;
                for(by = 0; by < (int)broadness; by++)
                {
                    x_screen = (int)x_current + bx;
                    y_screen = (int)y_current + by;

                    /* If it is on screen, blit it! */
                    if((x_screen >= 0) &&
                       (y_screen >= 0) &&
                       (x_screen < (int)tar_width) &&
                       (y_screen < (int)tar_height)
                    )
                    {
                        tar_buf_ptr = (u_int16_t *)(&tar_buf[
                            (tar_width_bytes * y_screen) +
                            (BYTES_PER_PIXEL16 * x_screen)
                        ]);
                        *tar_buf_ptr =
			    PACK8TO16(
 (u_int8_t)MIN((int)(((*tar_buf_ptr) & 0xf800) >> 8) + fg_color.r, 0xff),
 (u_int8_t)MIN((int)(((*tar_buf_ptr) & 0x07E0) >> 3) + fg_color.g, 0xff),    
 (u_int8_t)MIN((int)(((*tar_buf_ptr) & 0x001F) << 3) + fg_color.b, 0xff)
			    );
                    }
                }
                break;

              default:   /* Upper left. */
                /* Upper horizontal. */
                by = 0;
                for(bx = 0; bx < (int)broadness; bx++)
                {
                    x_screen = (int)x_current + bx;
                    y_screen = (int)y_current + by;

                    /* If it is on screen, blit it! */
                    if((x_screen >= 0) &&
                       (y_screen >= 0) &&
                       (x_screen < (int)tar_width) &&
                       (y_screen < (int)tar_height)
                    )
                    {
                        tar_buf_ptr = (u_int16_t *)(&tar_buf[
                            (tar_width_bytes * y_screen) +
                            (BYTES_PER_PIXEL16 * x_screen)
                        ]);
                        *tar_buf_ptr =
			    PACK8TO16(fg_color.r, fg_color.g, fg_color.b);
                    }
                }
                /* Left vertical. */
                bx = 0;
                for(by = 0; by < (int)broadness; by++)
                {
                    x_screen = (int)x_current + bx;
                    y_screen = (int)y_current + by;

                    /* If it is on screen, blit it! */
                    if((x_screen >= 0) &&
                       (y_screen >= 0) &&
                       (x_screen < (int)tar_width) &&
                       (y_screen < (int)tar_height)
                    )  
                    {
                        tar_buf_ptr = (u_int16_t *)(&tar_buf[
                            (tar_width_bytes * y_screen) +
                            (BYTES_PER_PIXEL16 * x_screen)
                        ]);
                        *tar_buf_ptr =
			    PACK8TO16(
 (u_int8_t)MIN((int)(((*tar_buf_ptr) & 0xf800) >> 8) + fg_color.r, 0xff),
 (u_int8_t)MIN((int)(((*tar_buf_ptr) & 0x07E0) >> 3) + fg_color.g, 0xff),    
 (u_int8_t)MIN((int)(((*tar_buf_ptr) & 0x001F) << 3) + fg_color.b, 0xff)
			    );
                    }
                }
                break;
            }

            xcount += xovery;
            ycount += yoverx;
        }


        return;
}


void BlitBufLineAdditive32(
        u_int8_t *tar_buf,
        int tar_x, int tar_y,
        unsigned int tar_width, unsigned int tar_height,
        double theta,
        unsigned int len,
        unsigned int broadness,
        WColorStruct fg_color
)
{
        double dx, dy;
        double yoverx, xovery; /* y / x and x / y */
        double xcount, ycount;
        double x_current, y_current;
        int x_quad_coeff, y_quad_coeff;
        int bx, by;
        int x_screen, y_screen;
            
        unsigned int quadrant;         /* 0, 1, 2 or 3. */   
            
        int tar_width_bytes;   /* tar_width * BYTES_PER_PIXEL32 */
            
        u_int32_t *tar_buf_ptr;


        /* Error checks. */
        if((tar_buf == NULL) ||
           (tar_width == 0) ||
           (tar_height == 0)
        )
            return;
 
        /* Totally not visable on target? */
        if(((tar_x + (int)len + (int)broadness) < 0) ||
           ((tar_y + (int)len + (int)broadness) < 0) ||
           ((tar_x - (int)len + (int)broadness) >= (int)tar_width) ||
           ((tar_y - (int)len + (int)broadness) >= (int)tar_height)
        )
            return;


        /* Offset tar_x and tar_y based on broadness. */
        tar_x -= (int)((double)broadness / 2);
        tar_y -= (int)((double)broadness / 2);
            
        
        /* Sanitize theta. */
        while(theta >= 6.283185307)
            theta -= 6.283185307;
        while(theta < 0)
            theta += 6.283185307;
         
        
        /* Sanitize len (can't be too big). */
        if((int)len > 10000)
            len = 10000;

        /* Can't be too broad. */
        if((int)broadness > 1024)
            broadness = 1024;
        else if((int)broadness == 0)
            return;   
        
            
        /* ************************************************************ */

        tar_width_bytes = (int)tar_width * BYTES_PER_PIXEL32;

        dx = sin(theta) * (double)len;
        dy = cos(theta) * (double)len * -1;

        /* Get quadrant coefficient -1 or 1. */
        x_quad_coeff = (dx < 0) ? -1 : 1;
        y_quad_coeff = (dy < 0) ? -1 : 1;
        
        /* Check which quadrant (clock wise from upper right). */
        if((x_quad_coeff >= 0) && (y_quad_coeff < 0))
            quadrant = 0;
        else if((x_quad_coeff >= 0) && (y_quad_coeff >= 0))
            quadrant = 1;
        else if((x_quad_coeff < 0) && (y_quad_coeff >= 0))
            quadrant = 2;
        else
            quadrant = 3;
         
        /* Make dy and dx positive. */
        dx *= (double)x_quad_coeff;
        dy *= (double)y_quad_coeff;
        
        /* Calculate yoverx and xovery. */
        if(dx == 0)
            yoverx = 1;
        else 
            yoverx = MIN((dy / dx), 1);
        
        if(dy == 0)
            xovery = 1;
        else
            xovery = MIN((dx / dy), 1);
            

        /* Either yoverx or xovery must have some value. */
        if((yoverx <= 0) && (xovery <= 0))
        {
            yoverx = 0.01;      /* Add some value to it. */
        }

	/* ********************************************************* */

        xcount = 0;
        ycount = 0;
        while((xcount <= dx) &&
              (ycount <= dy)
        )
        {
            x_current = (xcount * (double)x_quad_coeff) + tar_x;
            y_current = (ycount * (double)y_quad_coeff) + tar_y;
 
            /* Draw by quadrant. */
            switch(quadrant)
            {
              case 0:   /* Upper right. */
                /* Top horizontal. */
                by = 0;
                for(bx = 0; bx < (int)broadness; bx++)
                {  
                    x_screen = (int)x_current + bx;
                    y_screen = (int)y_current + by;

                    /* If it is on screen, blit it! */
                    if((x_screen >= 0) &&
                       (y_screen >= 0) &&
                       (x_screen < (int)tar_width) &&
                       (y_screen < (int)tar_height)
                    )
                    {
                        tar_buf_ptr = (u_int32_t *)(&tar_buf[
                            (tar_width_bytes * y_screen) +
                            (BYTES_PER_PIXEL32 * x_screen) 
                        ]);
                        *tar_buf_ptr =
                            PACK8TO32(
 (u_int8_t)(((*tar_buf_ptr) & 0xff000000) >> 24),
 (u_int8_t)MIN((int)(((*tar_buf_ptr) & 0x00ff0000) >> 16) + fg_color.r, 0xff),
 (u_int8_t)MIN((int)(((*tar_buf_ptr) & 0x0000ff00) >> 8) + fg_color.g, 0xff),
 (u_int8_t)MIN((int)((*tar_buf_ptr) & 0x000000ff) + fg_color.b, 0xff)
			    );
                    }
                }  
                /* Right vertical. */
                bx = broadness - 1;
                for(by = 0; by < (int)broadness; by++)
                {
                    x_screen = (int)x_current + bx;
                    y_screen = (int)y_current + by;
 
                    /* If it is on screen, blit it! */
                    if((x_screen >= 0) &&
                       (y_screen >= 0) &&
                       (x_screen < (int)tar_width) &&
                       (y_screen < (int)tar_height)
                    )  
                    {
                        tar_buf_ptr = (u_int32_t *)(&tar_buf[
                            (tar_width_bytes * y_screen) +
                            (BYTES_PER_PIXEL32 * x_screen)
                        ]);
                        *tar_buf_ptr =
                            PACK8TO32(
 (u_int8_t)(((*tar_buf_ptr) & 0xff000000) >> 24),
 (u_int8_t)MIN((int)(((*tar_buf_ptr) & 0x00ff0000) >> 16) + fg_color.r, 0xff),
 (u_int8_t)MIN((int)(((*tar_buf_ptr) & 0x0000ff00) >> 8) + fg_color.g, 0xff),
 (u_int8_t)MIN((int)((*tar_buf_ptr) & 0x000000ff) + fg_color.b, 0xff)
			    );
                    }
                }
                break;

              case 1:   /* Lower right. */
                /* Lower horizontal. */
                by = broadness - 1;
                for(bx = 0; bx < (int)broadness; bx++)
                {
                    x_screen = (int)x_current + bx;
                    y_screen = (int)y_current + by;

                    /* If it is on screen, blit it! */
                    if((x_screen >= 0) &&
                       (y_screen >= 0) &&
                       (x_screen < (int)tar_width) &&
                       (y_screen < (int)tar_height)
                    )
                    {
                        tar_buf_ptr = (u_int32_t *)(&tar_buf[
                            (tar_width_bytes * y_screen) +
                            (BYTES_PER_PIXEL32 * x_screen)
                        ]);
                        *tar_buf_ptr =
                            PACK8TO32(
 (u_int8_t)(((*tar_buf_ptr) & 0xff000000) >> 24),
 (u_int8_t)MIN((int)(((*tar_buf_ptr) & 0x00ff0000) >> 16) + fg_color.r, 0xff),
 (u_int8_t)MIN((int)(((*tar_buf_ptr) & 0x0000ff00) >> 8) + fg_color.g, 0xff),
 (u_int8_t)MIN((int)((*tar_buf_ptr) & 0x000000ff) + fg_color.b, 0xff)
			    );
		    }
                }
                /* Right vertical. */
                bx = broadness - 1;
                for(by = 0; by < (int)broadness; by++)
                {
                    x_screen = (int)x_current + bx;
                    y_screen = (int)y_current + by;

                    /* If it is on screen, blit it! */
                    if((x_screen >= 0) &&
                       (y_screen >= 0) &&
                       (x_screen < (int)tar_width) &&
                       (y_screen < (int)tar_height)
                    )
                    {
                        tar_buf_ptr = (u_int32_t *)(&tar_buf[
                            (tar_width_bytes * y_screen) +
                            (BYTES_PER_PIXEL32 * x_screen)
                        ]);
                        *tar_buf_ptr =
                            PACK8TO32(
 (u_int8_t)(((*tar_buf_ptr) & 0xff000000) >> 24),
 (u_int8_t)MIN((int)(((*tar_buf_ptr) & 0x00ff0000) >> 16) + fg_color.r, 0xff),
 (u_int8_t)MIN((int)(((*tar_buf_ptr) & 0x0000ff00) >> 8) + fg_color.g, 0xff),
 (u_int8_t)MIN((int)((*tar_buf_ptr) & 0x000000ff) + fg_color.b, 0xff)
			     );
                    }
                }
                break;
                        
              case 2:   /* Lower left. */
                /* Lower horizontal. */
                by = broadness - 1;
                for(bx = 0; bx < (int)broadness; bx++)
                {
                    x_screen = (int)x_current + bx;
                    y_screen = (int)y_current + by;

                    /* If it is on screen, blit it! */
                    if((x_screen >= 0) &&
                       (y_screen >= 0) &&
                       (x_screen < (int)tar_width) &&
                       (y_screen < (int)tar_height)
                    )
                    {
                        tar_buf_ptr = (u_int32_t *)(&tar_buf[
                            (tar_width_bytes * y_screen) +
                            (BYTES_PER_PIXEL32 * x_screen)
                        ]);
                        *tar_buf_ptr =
                            PACK8TO32(
 (u_int8_t)(((*tar_buf_ptr) & 0xff000000) >> 24),
 (u_int8_t)MIN((int)(((*tar_buf_ptr) & 0x00ff0000) >> 16) + fg_color.r, 0xff),
 (u_int8_t)MIN((int)(((*tar_buf_ptr) & 0x0000ff00) >> 8) + fg_color.g, 0xff),
 (u_int8_t)MIN((int)((*tar_buf_ptr) & 0x000000ff) + fg_color.b, 0xff)
			    );
                    }
                }
                /* Left vertical. */
                bx = 0;
                for(by = 0; by < (int)broadness; by++)
                {
                    x_screen = (int)x_current + bx;
                    y_screen = (int)y_current + by;
                     
                    /* If it is on screen, blit it! */
                    if((x_screen >= 0) &&
                       (y_screen >= 0) &&
                       (x_screen < (int)tar_width) &&
                       (y_screen < (int)tar_height)
                    )
                    {
                        tar_buf_ptr = (u_int32_t *)(&tar_buf[
                            (tar_width_bytes * y_screen) +
                            (BYTES_PER_PIXEL32 * x_screen)
                        ]);
                        *tar_buf_ptr =
                            PACK8TO32(
 (u_int8_t)(((*tar_buf_ptr) & 0xff000000) >> 24),
 (u_int8_t)MIN((int)(((*tar_buf_ptr) & 0x00ff0000) >> 16) + fg_color.r, 0xff),
 (u_int8_t)MIN((int)(((*tar_buf_ptr) & 0x0000ff00) >> 8) + fg_color.g, 0xff),
 (u_int8_t)MIN((int)((*tar_buf_ptr) & 0x000000ff) + fg_color.b, 0xff)
			    );
                    }
                }
                break;
                        
              default:   /* Upper left. */
                /* Upper horizontal. */
                by = 0;
                for(bx = 0; bx < (int)broadness; bx++)
                {
                    x_screen = (int)x_current + bx;
                    y_screen = (int)y_current + by;
                     
                    /* If it is on screen, blit it! */
                    if((x_screen >= 0) &&
                       (y_screen >= 0) &&
                       (x_screen < (int)tar_width) &&
                       (y_screen < (int)tar_height)
                    )
                    {
                        tar_buf_ptr = (u_int32_t *)(&tar_buf[
                            (tar_width_bytes * y_screen) +
                            (BYTES_PER_PIXEL32 * x_screen)
                        ]);
                        *tar_buf_ptr =
                            PACK8TO32(
 (u_int8_t)(((*tar_buf_ptr) & 0xff000000) >> 24),
 (u_int8_t)MIN((int)(((*tar_buf_ptr) & 0x00ff0000) >> 16) + fg_color.r, 0xff),
 (u_int8_t)MIN((int)(((*tar_buf_ptr) & 0x0000ff00) >> 8) + fg_color.g, 0xff),
 (u_int8_t)MIN((int)((*tar_buf_ptr) & 0x000000ff) + fg_color.b, 0xff)
			    );
                    }
                }
                /* Left vertical. */
                bx = 0;
                for(by = 0; by < (int)broadness; by++)
                {
                    x_screen = (int)x_current + bx;
                    y_screen = (int)y_current + by;

                    /* If it is on screen, blit it! */
                    if((x_screen >= 0) &&
                       (y_screen >= 0) &&
                       (x_screen < (int)tar_width) &&
                       (y_screen < (int)tar_height)
                    )  
                    {
                        tar_buf_ptr = (u_int32_t *)(&tar_buf[
                            (tar_width_bytes * y_screen) +
                            (BYTES_PER_PIXEL32 * x_screen)
                        ]);
                        *tar_buf_ptr =
                            PACK8TO32(
 (u_int8_t)(((*tar_buf_ptr) & 0xff000000) >> 24),
 (u_int8_t)MIN((int)(((*tar_buf_ptr) & 0x00ff0000) >> 16) + fg_color.r, 0xff),
 (u_int8_t)MIN((int)(((*tar_buf_ptr) & 0x0000ff00) >> 8) + fg_color.g, 0xff),
 (u_int8_t)MIN((int)((*tar_buf_ptr) & 0x000000ff) + fg_color.b, 0xff)
			    );
                    }
                }
                break;
            }

            xcount += xovery;
            ycount += yoverx;
        }


        return;
}


void BlitBufLineAdditive(
        unsigned int d,
        u_int8_t *tar_buf,
        int tar_x, int tar_y,
        unsigned int tar_width, unsigned int tar_height,
        double theta,
        unsigned int len,
        unsigned int broadness,
        WColorStruct fg_color
)
{
	switch(d)
	{
	  case 32:
            BlitBufLineAdditive32(
                tar_buf,
                tar_x,
                tar_y,
                tar_width,
                tar_height,
                theta,
                len,
                broadness,
                fg_color
	    );
	    break;

          case 24:
            BlitBufLineAdditive32(
                tar_buf,
                tar_x,
                tar_y,
                tar_width,
                tar_height,
                theta,
                len,
                broadness,
                fg_color
            );
            break;

          case 16:
            BlitBufLineAdditive16(
                tar_buf,
                tar_x,
                tar_y,
                tar_width,
                tar_height,
                theta,
                len,
                broadness,
                fg_color
            );
            break;

          case 15:
            BlitBufLineAdditive15(
                tar_buf,
                tar_x,
                tar_y,
                tar_width,
                tar_height,
                theta,
                len,
                broadness,
                fg_color
            );
            break;

          case 8:
            BlitBufLineAdditive8(
                tar_buf,
                tar_x,
                tar_y,
                tar_width,
                tar_height,
                theta,
                len,
                broadness,
                fg_color
            );
            break;
	}

	return;
}



