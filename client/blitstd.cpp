// client/blitstd.cpp
/*
                  Buffer Blitting: Standard Blitting Routines

    Functions:

    BlitBufNormal8()
    BlitBufNormal15()
    BlitBufNormal16()
    BlitBufNormal32()
    BlitBufNormal()

    BlitBufAbsolute8()
    BlitBufAbsolute15()
    BlitBufAbsolute16()
    BlitBufAbsolute32()
    BlitBufAbsolute()

    BlitBufAdditive8()
    BlitBufAdditive15()
    BlitBufAdditive16()
    BlitBufAdditive32()
    BlitBufAdditive()

    BlitBufSolid8()
    BlitBufSolid16()
    BlitBufSolid32()
    BlitBufSolid()

    ---

    These functions perform blitting of two buffers.
    A porition of the source buffer to a position on the destination
    buffer with `zooming' support.

    Zooming is currently limited to 0.01 to 1.00 ranges.

    1-16-00(KB):    Macro'ized inner pixel loops.
                    Added XSWRect and support to clean up data management.
    1-17-00(KB):    Reorganized setup code for scaled blits.
    1-18-00(KB):    Macroized blit preparation code.
    1-19-00(KB):    Debugging changes.
 */

#include <assert.h>
#include "blitting.h"

/*
 *   Checks if tx, ty, sy, and sy are 'on the screen':
 *
 *      The variables beginning with a t means target, and respectively
 *  x, y, width, and height.
 *
 *      The variables beginning with a s means source, and respectively
 *      x, y, width, and height.
 */
 
#ifndef MAX
#define MIN(a,b)        (((a) < (b)) ? (a) : (b))
#define MAX(a,b)	(((a) > (b)) ? (a) : (b))
#endif
/* #define MIN(a,b)        (((a) < (b)) ? (a) : (b)) */
/* #define MAX(a,b)        (((a) > (b)) ? (a) : (b)) */



/* XSWRect definition. */
typedef struct tag_Rect
{
    int     left;
    int     top;
    int     right;
    int     bottom;
} XSWRect;


/* XSWRect utility macro's. */
#define     RECT_WIDTH(a)       ( ((a).right) - ((a).left) )
#define     RECT_HEIGHT(a)      ( ((a).bottom) - ((a).top) )
#define     RECT_VALID(a)       ( RECT_WIDTH(a) > 0 && RECT_HEIGHT(a) > 0 )


/*
    Blit preparation macro.
 */
#define BLIT_PREP                                                               \
    float           Source_Step_X;                                              \
    float           Source_Step_Y;                                              \
    XSWRect         Source;                                                     \
    XSWRect         Dest;                                                       \
    float           Scale = (float)zoom * (float)magnification;                 \
    assert(tar_buf && src_buf);	                                               \
    assert((tar_width != 0) && (src_width != 0));                                 \
    assert((tar_height != 0) && (src_height != 0));                               \
    assert((src_copy_width != 0) && (src_copy_height != 0));                      \
    assert((zoom >= 0.0) && (magnification >= 0.0));                              \
    if((tar_x >= (int)tar_width) || (tar_y >= (int)tar_height))                   \
        return;                                                                 \
    if((src_x >= (int)src_width) || (src_y >= (int)src_height))                   \
        return;                                                                 \
    if(Scale > BLIT_MAGNIFICATION_MAX)						\
	Scale = BLIT_MAGNIFICATION_MAX;    					 \
    Source.left     =   src_x;                                                  \
    Source.top      =   src_y;                                                  \
    Source.right    =   src_x + src_copy_width;                                 \
    Source.bottom   =   src_y + src_copy_height;                                \
    Dest.left       =   tar_x;                                                  \
    Dest.top        =   tar_y;                                                  \
    Dest.right      =   tar_x + static_cast<int>(Scale*src_copy_width);         \
    Dest.bottom     =   tar_y + static_cast<int>(Scale*src_copy_height);        \
    Source_Step_X   = (float)RECT_WIDTH(Source) / (float)RECT_WIDTH(Dest);      \
    Source_Step_Y   = (float)RECT_HEIGHT(Source) / (float)RECT_HEIGHT(Dest);    \
    if( Dest.left < 0 )                                                         \
    {                                                                           \
    	Source.left     -=  static_cast<int>(Dest.left * Source_Step_X);        \
        Dest.left       =   0;                                                  \
        if( Dest.left >= Dest.right )						\
	    return;								\
    }                                                                           \
    if( Dest.top < 0 )                                                          \
    {                                                                           \
        Source.top      -=  static_cast<int>(Dest.top * Source_Step_Y);         \
        Dest.top        =   0;                                                  \
        if( Dest.top >= Dest.bottom )						\
	    return;								\
    }                                                                           \
    if( Dest.right > (int)tar_width )                                           \
    {                                                                           \
        Source.right    +=  static_cast<int>((Dest.right - tar_width) * Source_Step_X);   \
        Dest.right      =   tar_width;                                          \
        if( Dest.right <= Dest.left )						\
	    return;                                   				\
    }                                                                           \
    if( Dest.bottom > (int)tar_height )                                              \
    {                                                                           \
        Source.bottom   +=  static_cast<int>((Dest.bottom-tar_height) * Source_Step_Y); \
        Dest.bottom     =   tar_height;                                         \
        if( Dest.bottom <= Dest.top )						\
	    return;       				                            \
    }                                                                           \
    if( !RECT_VALID(Source) || !RECT_VALID(Dest) )                              \
        return;





/*
    Pixel loop definition macro.

    Used to define the pixel loop in a consistent manner.
    Requires that PIX_PLOT is set according to requirements.
 */
#define PIX_LOOP(size)                                                              \
{                                                                                   \
    u_int32_t   row,    col;                                                        \
    float       source_y_frac = 0;                                                  \
    for(row=0; row < Dest_Height; row++)                                            \
    {                                                                               \
        float   source_x_frac;                                                      \
        size    *source_ptr_row  = pSource + Source_Stride*(int)source_y_frac;      \
        source_x_frac = 0;                                                          \
        for(col=0; col < Dest_Width; col++)                                         \
        {                                                                           \
            size source = *(source_ptr_row + (int)source_x_frac);                   \
            PIX_PLOT                                                                \
            source_x_frac += Source_Step_X;                                         \
        }                                                                           \
        pDest           += Dest_Stride;                                             \
        source_y_frac   += Source_Step_Y;                                           \
    }                                                                               \
}



/*
 *                          NORMAL BLITTING
 */

/* BlitBufNormal8 */
void BlitBufNormal8(
    u_int8_t        *pDest,
    u_int32_t       Dest_Stride,
    u_int32_t       Dest_Width,
    u_int32_t       Dest_Height,
    u_int8_t        *pSource,
    u_int32_t       Source_Stride,
    float           Source_Step_X,
    float           Source_Step_Y,
    float           Alpha   )
{
    if(Alpha >= 1)
    {
#define PIX_PLOT                \
        {                               \
            if( source != 0x0000 )      \
                pDest[col] = source;    \
        }
        PIX_LOOP(u_int8_t)
#undef PIX_PLOT
    } else
    {
#define PIX_PLOT                                                                                        \
        {                                                                                               \
            if(source != 0x00)                                                                          \
            {                                                                                           \
                int tr, tg, tb;                                                                         \
                tr = (pDest[col]) & 0xE0;                                                               \
                tg = (pDest[col]) & 0x1C;                                                               \
                tb = (pDest[col]) & 0x03;                                                               \
                pDest[col] =    (( (u_int8_t)((Alpha * ((source & 0xE0) - tr)) + tr) ) & 0xE0) |        \
                                (( (u_int8_t)((Alpha * ((source & 0x1C) - tg)) + tg) ) & 0x1C) |        \
                                (( (u_int8_t)((Alpha * ((source & 0x03) - tb)) + tb) ) & 0x03) ;        \
            }                                                                                           \
        }
        PIX_LOOP(u_int8_t)
#undef PIX_PLOT
    }
}



/* BlitBufNormal15 */
void BlitBufNormal15(
    u_int16_t       *pDest,
    u_int32_t       Dest_Stride,
    u_int32_t       Dest_Width,
    u_int32_t       Dest_Height,
    u_int16_t       *pSource,
    u_int32_t       Source_Stride,
    float           Source_Step_X,
    float           Source_Step_Y,
    float           Alpha   )
{
    if(Alpha >= 1)
    {
#define PIX_PLOT                \
        {                               \
            if( source != 0x0000 )      \
                pDest[col] = source;    \
        }

        PIX_LOOP(u_int16_t)
#undef PIX_PLOT
    } else
    {
#define PIX_PLOT                                                                                \
        {                                                                                               \
            if(source != 0x0000)                                                                        \
            {                                                                                           \
                int tr, tg, tb;                                                                         \
                tr = (pDest[col]) & 0x7C00;                                                             \
                tg = (pDest[col]) & 0x03E0;                                                             \
                tb = (pDest[col]) & 0x001F;                                                             \
                pDest[col] =    (( (u_int16_t)((Alpha * ((source & 0x7C00) - tr)) + tr) ) & 0x7C00) |   \
                                (( (u_int16_t)((Alpha * ((source & 0x03E0) - tg)) + tg) ) & 0x03E0) |   \
                                (( (u_int16_t)((Alpha * ((source & 0x001F) - tb)) + tb) ) & 0x001F) ;   \
            }                                                                                           \
        }
        PIX_LOOP(u_int16_t)
#undef PIX_PLOT
    }
}



/* BlitBufNormal16 */
void BlitBufNormal16(
    u_int16_t       *pDest,
    u_int32_t       Dest_Stride,
    u_int32_t       Dest_Width,
    u_int32_t       Dest_Height,
    u_int16_t       *pSource,
    u_int32_t       Source_Stride,
    float           Source_Step_X,
    float           Source_Step_Y,
    float           Alpha   )
{
    if(Alpha >= 1)
    {
#define PIX_PLOT                \
        {                               \
            if( source != 0x0000 )      \
                pDest[col] = source;    \
        }
        PIX_LOOP(u_int16_t)
#undef PIX_PLOT
    } else
    {
#define PIX_PLOT                                                                                \
        {                                                                                               \
            if(source != 0x0000)                                                                        \
            {                                                                                           \
                int tr, tg, tb;                                                                         \
                tr = (pDest[col]) & 0xF800;                                                             \
                tg = (pDest[col]) & 0x07E0;                                                             \
                tb = (pDest[col]) & 0x001F;                                                             \
                pDest[col] =    (( (u_int16_t)((Alpha * ((source & 0xF800) - tr)) + tr) ) & 0xF800) |   \
                                (( (u_int16_t)((Alpha * ((source & 0x07E0) - tg)) + tg) ) & 0x07E0) |   \
                                (( (u_int16_t)((Alpha * ((source & 0x001F) - tb)) + tb) ) & 0x001F) ;   \
            }                                                                                           \
        }
        PIX_LOOP(u_int16_t)
#undef PIX_PLOT
    }
}



/* BlitBufNormal32 */
void BlitBufNormal32(
    u_int32_t       *pDest,
    u_int32_t       Dest_Stride,
    u_int32_t       Dest_Width,
    u_int32_t       Dest_Height,
    u_int32_t       *pSource,
    u_int32_t       Source_Stride,
    float           Source_Step_X,
    float           Source_Step_Y,
    float           Alpha   )
{
    if(Alpha >= 1)
    {
#define PIX_PLOT                \
        {                               \
            if( source != 0x00000000 )  \
                pDest[col] = source;    \
        }
        PIX_LOOP(u_int32_t)
#undef PIX_PLOT
    } else
    {
#define PIX_PLOT                                                                                    \
        {                                                                                                   \
            if(source != 0x0000)                                                                            \
            {                                                                                               \
                int ta, tr, tg, tb;                                                                         \
                ta = (pDest[col]) & 0xFF000000;                                                             \
                tr = (pDest[col]) & 0x00FF0000;                                                             \
                tg = (pDest[col]) & 0x0000FF00;                                                             \
                tb = (pDest[col]) & 0x000000FF;                                                             \
                pDest[col] =    (( (u_int16_t)((Alpha * ((source & 0xFF000000) - ta)) + tr) ) & 0xFF000000) | \
                                (( (u_int16_t)((Alpha * ((source & 0x00FF0000) - tr)) + tr) ) & 0x00FF0000) | \
                                (( (u_int16_t)((Alpha * ((source & 0x0000FF00) - tg)) + tg) ) & 0x0000FF00) | \
                                (( (u_int16_t)((Alpha * ((source & 0x000000FF) - tb)) + tb) ) & 0x000000FF) ; \
            }                                                                                               \
        }
        PIX_LOOP(u_int32_t)
#undef PIX_PLOT
    }
}



/* BlitBufNormal- General dispatch function. */
void BlitBufNormal(
    unsigned int    d,
    u_int8_t        *tar_buf,
    u_int8_t        *src_buf,
    int             tar_x,
    int             tar_y,
    unsigned int    tar_width,
    unsigned int    tar_height,
    int             src_x,
    int             src_y,
    unsigned int    src_width,
    unsigned int    src_height,
    unsigned int    src_copy_width,
    unsigned int    src_copy_height,
    double          zoom,                                            /* 0.1 to 1.0 supported. */
    double          vis,
    double          magnification )
{
    BLIT_PREP

    switch(d)
    {
    case 8:
        {
            u_int8_t    *pDest      = ((u_int8_t*)tar_buf) + Dest.left + Dest.top*tar_width;
            u_int8_t    *pSource    = ((u_int8_t*)src_buf) + Source.left + Source.top*src_width;
            BlitBufNormal8(     pDest,
                tar_width,
                RECT_WIDTH(Dest),
                RECT_HEIGHT(Dest),
                pSource,
                src_width,
                Source_Step_X,
                Source_Step_Y,
                (float)vis );
        }
        break;

    case 15:
        {
            u_int16_t   *pDest      = ((u_int16_t*)tar_buf) + Dest.left + Dest.top*tar_width;
            u_int16_t   *pSource    = ((u_int16_t*)src_buf) + Source.left + Source.top*src_width;
            BlitBufNormal15(    pDest,
                tar_width,
                RECT_WIDTH(Dest),
                RECT_HEIGHT(Dest),
                pSource,
                src_width,
                Source_Step_X,
                Source_Step_Y,
                (float)vis );
        }
        break;

    case 16:
        {
            u_int16_t   *pDest      = ((u_int16_t*)tar_buf) + Dest.left + Dest.top*tar_width;
            u_int16_t   *pSource    = ((u_int16_t*)src_buf) + Source.left + Source.top*src_width;
            BlitBufNormal16(    pDest,
                tar_width,
                RECT_WIDTH(Dest),
                RECT_HEIGHT(Dest),
                pSource,
                src_width,
                Source_Step_X,
                Source_Step_Y,
                (float)vis );
        }
        break;

    case 24:
    case 32:
        {
            u_int32_t   *pDest      = ((u_int32_t*)tar_buf) + Dest.left + Dest.top*tar_width;
            u_int32_t   *pSource    = ((u_int32_t*)src_buf) + Source.left + Source.top*src_width;
            BlitBufNormal32(    pDest,
                tar_width,
                RECT_WIDTH(Dest),
                RECT_HEIGHT(Dest),
                pSource,
                src_width,
                Source_Step_X,
                Source_Step_Y,
                (float)vis );
        }
        break;
    }
}



/*
 *                         ABSOLUTE BLITTING
 *
 *   Same as normal blitting except that it does not check for
 *   transparency.
 */
void BlitBufAbsolute8(
    u_int8_t        *pDest,
    u_int32_t       Dest_Stride,
    u_int32_t       Dest_Width,
    u_int32_t       Dest_Height,
    u_int8_t        *pSource,
    u_int32_t       Source_Stride,
    float           Source_Step_X,
    float           Source_Step_Y   )
{
#define PIX_PLOT                \
    {                               \
        pDest[col] = source;        \
    }
    PIX_LOOP(u_int8_t)
#undef PIX_PLOT
}



void BlitBufAbsolute16(
    u_int16_t       *pDest,
    u_int32_t       Dest_Stride,
    u_int32_t       Dest_Width,
    u_int32_t       Dest_Height,
    u_int16_t       *pSource,
    u_int32_t       Source_Stride,
    float           Source_Step_X,
    float           Source_Step_Y   )
{
#define PIX_PLOT                \
    {                               \
        pDest[col] = source;        \
    }
    PIX_LOOP(u_int16_t)
#undef PIX_PLOT
}



void BlitBufAbsolute32(
    u_int32_t       *pDest,
    u_int32_t       Dest_Stride,
    u_int32_t       Dest_Width,
    u_int32_t       Dest_Height,
    u_int32_t       *pSource,
    u_int32_t       Source_Stride,
    float           Source_Step_X,
    float           Source_Step_Y   )
{
#define PIX_PLOT                \
    {                               \
        pDest[col] = source;        \
    }
    PIX_LOOP(u_int32_t)
#undef PIX_PLOT
}



void BlitBufAbsolute(
    unsigned int    d,
    u_int8_t        *tar_buf,
    u_int8_t        *src_buf,
    int             tar_x,
    int             tar_y,
    unsigned int    tar_width,
    unsigned int    tar_height,
    int             src_x,
    int             src_y,
    unsigned int    src_width,
    unsigned int    src_height,
    unsigned int    src_copy_width,
    unsigned int    src_copy_height,
    double          zoom,                                            /* 0.1 to 1.0 supported. */
    double          magnification    )
{
    BLIT_PREP

        switch(d)
    {
    case 8:
        {
            u_int8_t    *pDest      = ((u_int8_t*)tar_buf) + Dest.left + Dest.top*tar_width;
            u_int8_t    *pSource    = ((u_int8_t*)src_buf) + Source.left + Source.top*src_width;
            BlitBufAbsolute8(   pDest,
                tar_width,
                RECT_WIDTH(Dest),
                RECT_HEIGHT(Dest),
                pSource,
                src_width,
                Source_Step_X,
                Source_Step_Y   );
        }
        break;

    case 15:
    case 16:
        {
            u_int16_t   *pDest      = ((u_int16_t*)tar_buf) + Dest.left + Dest.top*tar_width;
            u_int16_t   *pSource    = ((u_int16_t*)src_buf) + Source.left + Source.top*src_width;
            BlitBufAbsolute16(  pDest,
                tar_width,
                RECT_WIDTH(Dest),
                RECT_HEIGHT(Dest),
                pSource,
                src_width,
                Source_Step_X,
                Source_Step_Y   );
        }
        break;

    case 24:
    case 32:
        {
            u_int32_t   *pDest      = ((u_int32_t*)tar_buf) + Dest.left + Dest.top*tar_width;
            u_int32_t   *pSource    = ((u_int32_t*)src_buf) + Source.left + Source.top*src_width;
            BlitBufAbsolute32(  pDest,
                tar_width,
                RECT_WIDTH(Dest),
                RECT_HEIGHT(Dest),
                pSource,
                src_width,
                Source_Step_X,
                Source_Step_Y   );
        }
        break;
    }
}



/*
 *                                  Additive
 */
void BlitBufAdditive8(
    u_int8_t        *pDest,
    u_int32_t       Dest_Stride,
    u_int32_t       Dest_Width,
    u_int32_t       Dest_Height,
    u_int8_t        *pSource,
    u_int32_t       Source_Stride,
    float           Source_Step_X,
    float           Source_Step_Y   )
{
#define PIX_PLOT                                \
    {                                               \
        int r, g, b;                                \
        int dest = pDest[col];                      \
        r = MIN((source&0xE0) + (dest&0xE0), 0xE0); \
        g = MIN((source&0x1C) + (dest&0x1C), 0x1C); \
        b = MIN((source&0x03) + (dest&0x03), 0x03); \
        pDest[col] = r | g | b;                     \
    }
    PIX_LOOP(u_int8_t)
#undef PIX_PLOT
}




void BlitBufAdditive15(
    u_int16_t       *pDest,
    u_int32_t       Dest_Stride,
    u_int32_t       Dest_Width,
    u_int32_t       Dest_Height,
    u_int16_t       *pSource,
    u_int32_t       Source_Stride,
    float           Source_Step_X,
    float           Source_Step_Y   )
{
#define PIX_PLOT                                        \
    {                                                       \
        int r, g, b;                                        \
        u_int32_t dest  = (u_int32_t)pDest[col];            \
        r = (source & 0x00007C00) + (dest & 0x00007C00);    \
        g = (source & 0x000003E0) + (dest & 0x000003E0);    \
        b = (source & 0x0000001F) + (dest & 0x0000001F);    \
        pDest[col] =    (u_int16_t)(MIN(r, 0x00007C00) |    \
                                    MIN(g, 0x000003E0) |    \
                                    MIN(b, 0x0000001F) );   \
    }
    PIX_LOOP(u_int16_t)
#undef PIX_PLOT
}



void BlitBufAdditive16(
    u_int16_t       *pDest,
    u_int32_t       Dest_Stride,
    u_int32_t       Dest_Width,
    u_int32_t       Dest_Height,
    u_int16_t       *pSource,
    u_int32_t       Source_Stride,
    float           Source_Step_X,
    float           Source_Step_Y   )
{
#define PIX_PLOT                                        \
    {                                                       \
        int r, g, b;                                        \
        u_int32_t dest  = (u_int32_t)pDest[col];            \
        r = (source & 0x0000F800) + (dest & 0x0000F800);    \
        g = (source & 0x000007E0) + (dest & 0x000007E0);    \
        b = (source & 0x0000001F) + (dest & 0x0000001F);    \
        pDest[col] =    (u_int16_t)(MIN(r, 0x0000F800) |    \
                                    MIN(g, 0x000007E0) |    \
                                    MIN(b, 0x0000001F) );   \
    }
    PIX_LOOP(u_int16_t)
#undef PIX_PLOT
}



void BlitBufAdditive32(
    u_int32_t       *pDest,
    u_int32_t       Dest_Stride,
    u_int32_t       Dest_Width,
    u_int32_t       Dest_Height,
    u_int32_t       *pSource,
    u_int32_t       Source_Stride,
    float           Source_Step_X,
    float           Source_Step_Y   )
{
#define PIX_PLOT                \
    {                               \
        int r, g, b;                                        \
        u_int32_t dest  = (u_int32_t)pDest[col];            \
        r = (source & 0x00FF0000) + (dest & 0x00FF0000);    \
        g = (source & 0x0000FF00) + (dest & 0x0000FF00);    \
        b = (source & 0x000000FF) + (dest & 0x000000FF);    \
        pDest[col] =    (u_int16_t)(MIN(r, 0x00FF0000) |    \
                                    MIN(g, 0x0000FF00) |    \
                                    MIN(b, 0x000000FF) );   \
    }
    PIX_LOOP(u_int32_t)
#undef PIX_PLOT
}



void BlitBufAdditive(
    unsigned int    d,
    u_int8_t        *tar_buf,
    u_int8_t        *src_buf,
    int             tar_x,
    int             tar_y,
    unsigned int    tar_width,
    unsigned int    tar_height,
    int             src_x,
    int             src_y,
    unsigned int    src_width,
    unsigned int    src_height,
    unsigned int    src_copy_width,
    unsigned int    src_copy_height,
    double          zoom,                                            /* 0.1 to 1.0 supported. */
    double          magnification    )
{
    BLIT_PREP

        switch(d)
    {
    case 8:
        {
            u_int8_t    *pDest      = ((u_int8_t*)tar_buf) + Dest.left + Dest.top*tar_width;
            u_int8_t    *pSource    = ((u_int8_t*)src_buf) + Source.left + Source.top*src_width;
            BlitBufAdditive8(   pDest,
                tar_width,
                RECT_WIDTH(Dest),
                RECT_HEIGHT(Dest),
                pSource,
                src_width,
                Source_Step_X,
                Source_Step_Y   );
        }
        break;

    case 15:
        {
            u_int16_t   *pDest      = ((u_int16_t*)tar_buf) + Dest.left + Dest.top*tar_width;
            u_int16_t   *pSource    = ((u_int16_t*)src_buf) + Source.left + Source.top*src_width;
            BlitBufAdditive15(  pDest,
                tar_width,
                RECT_WIDTH(Dest),
                RECT_HEIGHT(Dest),
                pSource,
                src_width,
                Source_Step_X,
                Source_Step_Y   );
        }
        break;

    case 16:
        {
            u_int16_t   *pDest      = ((u_int16_t*)tar_buf) + Dest.left + Dest.top*tar_width;
            u_int16_t   *pSource    = ((u_int16_t*)src_buf) + Source.left + Source.top*src_width;
            BlitBufAdditive16(  pDest,
                tar_width,
                RECT_WIDTH(Dest),
                RECT_HEIGHT(Dest),
                pSource,
                src_width,
                Source_Step_X,
                Source_Step_Y   );
        }
        break;

    case 24:
    case 32:
        {
            u_int32_t   *pDest      = ((u_int32_t*)tar_buf) + Dest.left + Dest.top*tar_width;
            u_int32_t   *pSource    = ((u_int32_t*)src_buf) + Source.left + Source.top*src_width;
            BlitBufAdditive32(  pDest,
                tar_width,
                RECT_WIDTH(Dest),
                RECT_HEIGHT(Dest),
                pSource,
                src_width,
                Source_Step_X,
                Source_Step_Y   );
        }
        break;
    }
}



/*
 *  Simple solid buffer blitting
 */
void BlitBufSolid8(
    u_int8_t *tar_buf,
    unsigned int tar_width,
    unsigned int tar_height,
    WColorStruct color    )
{
    u_int8_t v, *ptr, *end;


    if((tar_buf == NULL) ||
        (tar_width == 0) ||
        (tar_height == 0)
        )
        return;


    /* Set value. */
    v = (u_int8_t)PACK8TO8(color.r, color.g, color.b);

    /* Begin setting buffer. */
    for(ptr = (u_int8_t *)tar_buf,
        end = ptr + (tar_width * tar_height);
        ptr < end;
        *ptr++ = v
        );


    return;
}

void BlitBufSolid16(
	u_int8_t *tar_buf,
	unsigned int tar_width,
	unsigned int tar_height,
	u_int16_t color
)
{
/*
	u_int16_t *ptr, *end;

	if((tar_buf == NULL) ||
           (tar_width == 0) ||
           (tar_height == 0)
        )
           return;

        for(ptr = (u_int16_t *)tar_buf,
            end = ptr + (tar_width * tar_height);
            ptr < end;
            *ptr++ = color
        );
 */

/* Trying something here, try to blit at 16 bit depth but
 * with u_int32 pointers.
 */

	u_int32_t v32;
	u_int32_t *ptr32, *end32;
	u_int16_t *ptr16, *end16;
	int total_pixels;


	if((tar_buf == NULL) ||
           (tar_width == 0) ||
           (tar_height == 0)  
        )
           return;

	v32 = (u_int32_t)color | ((u_int32_t)color << 16);
	total_pixels = tar_width * tar_height;

	/* Blit at 4 bytes per pixel, even though depth is really
	 * 2 bytes per pixel. So we calculate the end pointer
	 * at origin + (half of width * height).
	 * Note that we may be short at most three pixels at the end,
	 * no worry, we'll blit the last three later just incase.
	 */
	for(ptr32 = (u_int32_t *)tar_buf,
            end32 = ptr32 + (total_pixels / 2);	/* Half. */
            ptr32 < end32;
            *ptr32++ = v32
        );

	/* Blut the last three pixels that might not have been
	 * blitted in the above pass.
	 */
	ptr16 = (u_int16_t *)tar_buf + total_pixels - 1;
	end16 = (u_int16_t *)tar_buf + total_pixels - 3;
	if(end16 >= (u_int16_t *)tar_buf)
	    for(; ptr16 > end16; *ptr16-- = color);
            

	return;
}


void BlitBufSolid32(
    u_int8_t *tar_buf,
    unsigned int tar_width,
    unsigned int tar_height,
    WColorStruct color
)
{
    u_int32_t v, *ptr, *end;


    if((tar_buf == NULL) ||
        (tar_width == 0) ||
        (tar_height == 0)
        )
        return;


    /* Set value. */
    v = (u_int32_t)PACK8TO32(color.a, color.r, color.g, color.b);

    /* Begin setting buffer. */
    for(ptr = (u_int32_t *)tar_buf,
        end = ptr + (tar_width * tar_height);
        ptr < end;
        *ptr++ = v
        );


    return;
}



void BlitBufSolid(
    unsigned int d,
    u_int8_t *tar_buf,
    unsigned int tar_width,
    unsigned int tar_height,
    WColorStruct color   )
{
    switch(d)
    {
    case 32:
        BlitBufSolid32(
            tar_buf,
            tar_width,
            tar_height,
            color
            );
        break;

    case 24:
        BlitBufSolid32(
            tar_buf,
            tar_width,
            tar_height,
            color
            );
        break;

    case 16:
        BlitBufSolid16(
            tar_buf,
            tar_width,
            tar_height,
            (u_int16_t)PACK8TO16(color.r, color.g, color.b)
            );
        break;

    case 15:
        BlitBufSolid16(
            tar_buf,
            tar_width,
            tar_height,
            (u_int16_t)PACK8TO15(color.r, color.g, color.b)
            );
        break;

    case 8:
        BlitBufSolid8(
            tar_buf,
            tar_width,
            tar_height,
            color
            );
        break;
    }

    return;
}



#if 0
/*
    Original blitting preparation code prior to macroization.
    !DO NOT DELETE AS THE COMPRESSED MACRO IS NOT EASILLY EDITED!
 */
u_int32_t       Dest_Width;
u_int32_t       Dest_Height;
float           Source_Step_X;
float           Source_Step_Y;

XSWRect         Source;
XSWRect         Dest;

float       Scale = (float)zoom * (float)magnification;

/* Prophylactics. */
assert( tar_buf && src_buf );                                        // Bad buffer involved.
assert( tar_width != 0 && src_width != 0 );                          // Bad width involved.
assert( tar_height != 0 && src_height != 0 );                        // Bad height involved.
assert( src_copy_width != 0 && src_copy_height != 0 );               // Bad copy parameters involved.
assert( zoom >= 0.0 && magnification >= 0.0 );                       // Bad scaling information.

if(tar_x >= tar_width || tar_y >= tar_height)
    return;
if(src_x >= src_width || src_y >= src_height)
    return;

/* Sanitize Scale. */
if(Scale > BLIT_MAGNIFICATION_MAX)    Scale = BLIT_MAGNIFICATION_MAX;
/* if( Scale < BLIT_MAGNIFICATION_MIN )    Scale = BLIT_MAGNIFICATION_MIN; */

assert( Scale==1.0f );

/* Precalculations. */

/* Compute rectangles. */
Source.left     =   src_x;
Source.top      =   src_y;
Source.right    =   src_x + src_copy_width;
Source.bottom   =   src_y + src_copy_height;

Dest.left       =   tar_x;
Dest.top        =   tar_y;
Dest.right      =   tar_x + Scale*src_copy_width;
Dest.bottom     =   tar_y + Scale*src_copy_height;

/*  Source and Dest in standard StretchBlit format.
    This is where we would insert an OpenGL call to perform the blit. */

/* Compute stepping information. */
Source_Step_X   = (float)RECT_WIDTH(Source) / (float)RECT_WIDTH(Dest);
Source_Step_Y   = (float)RECT_HEIGHT(Source) / (float)RECT_HEIGHT(Dest);

/* Clip destination rectangle against buffer size. */
if(Dest.left < 0)
{
    Source.left     -=  Dest.left * Source_Step_X;
    Dest.left       =   0;
}

if(Dest.top < 0)
{
    Source.top      -=  Dest.top * Source_Step_Y;
    Dest.top        =   0;
}

if(Dest.right > tar_width)
{
    Source.right    +=  (Dest.right-tar_width) * Source_Step_X;
    Dest.right      =   tar_width;
}

if(Dest.bottom > tar_height)
{
    Source.bottom   +=  (Dest.bottom-tar_height) * Source_Step_Y;
    Dest.bottom     =   tar_height;
}

/* Revalidate rects. */
if(!RECT_VALID(Source) || !RECT_VALID(Dest))
    return;

#endif



