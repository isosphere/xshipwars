/*
                             Buffer Blitting
 */

#ifndef BLITTING_H
#define BLITTING_H

#include <sys/types.h>
#include <math.h>

#include "../include/graphics.h"
#include "../include/widget.h"		/* Need to know about WColorStruct. */

#include "../include/os.h"


/*
 *	Magnification bounds:
 */
#define BLIT_MAGNIFICATION_MIN	1
#define BLIT_MAGNIFICATION_MAX	255


/* In blitbeam.c */
extern void BlitBufBeam(
        unsigned int d,
        u_int8_t *tar_buf,
        int tar_x, int tar_y,
        unsigned int tar_width, unsigned int tar_height,
        double theta,
        unsigned int len,
        unsigned int broadness,
        WColorStruct color
);

/* In blitcircle.c */
extern void BlitBufCircle(
        unsigned int d,
        u_int8_t *tar_buf,
        int tar_x,
        int tar_y,
        unsigned int tar_width,
        unsigned int tar_height,
        double thickness,               /* In pixels. */
        double radius,                  /* In pixels. */
        WColorStruct fg_color
);

/* In blitstd.c */
extern void BlitBufNormal(
	unsigned int d,
        u_int8_t *tar_buf,
        u_int8_t *src_buf,
        int tar_x,
        int tar_y,
        unsigned int tar_width,
        unsigned int tar_height,
        int src_x,
        int src_y,
        unsigned int src_width,
        unsigned int src_height,
        unsigned int src_copy_width,
        unsigned int src_copy_height,
        double zoom,			/* 0.1 to 1.0 supported. */
	double vis,			/* 0.0 to 1.0 supported. */
	double magnification		/* 1 to 255 supported. */
);

extern void BlitBufAbsolute(
        unsigned int d,
        u_int8_t *tar_buf,
        u_int8_t *src_buf,
        int tar_x,
        int tar_y,
        unsigned int tar_width,
        unsigned int tar_height,
        int src_x,
        int src_y,
        unsigned int src_width,
        unsigned int src_height,
        unsigned int src_copy_width,
        unsigned int src_copy_height,
        double zoom,			/* 0.1 to 1.0 supported. */
        double magnification            /* 1 to 255 supported. */
);

extern void BlitBufAdditive(
        unsigned int d,
        u_int8_t *tar_buf,
        u_int8_t *src_buf,
        int tar_x,
        int tar_y,
        unsigned int tar_width,
        unsigned int tar_height,
        int src_x,
        int src_y,
        unsigned int src_width,
        unsigned int src_height,
        unsigned int src_copy_width,
        unsigned int src_copy_height,
        double zoom,			/* 0.1 to 1.0 supported. */
        double magnification            /* 1 to 255 supported. */
);

extern void BlitBufSolid(
	unsigned int d,
	u_int8_t *tar_buf,
	unsigned int tar_width,
	unsigned int tar_height,
	WColorStruct color
);



/* In blitfade.c */
extern void BlitBufFade(
        unsigned int d,
        u_int8_t *tar_buf,
        int tar_x,
        int tar_y,
        unsigned int tar_width,
        unsigned int tar_height,
        unsigned int tar_apply_width,
        unsigned int tar_apply_height,
        double gamma			/* 0.1 to 1.0. */
);
extern void BlitBufGlow(
        unsigned int d,
        u_int8_t *tar_buf,
        int tar_x,
        int tar_y,
        unsigned int tar_width,
        unsigned int tar_height,
        unsigned int tar_apply_width,
        unsigned int tar_apply_height,
        WColorStruct color		/* 0.1 to 1.0. */
);


/* In blitptlt.c (Point light) */
extern void BlitBufPointLight(
	unsigned int d,
        u_int8_t *tar_buf,
        int tar_x, int tar_y,
        unsigned int tar_width, unsigned int tar_height,
	double radius,
	WColorStruct light_color
);


/* In blitline.c */
extern void BlitBufLine(
        unsigned int d,
        u_int8_t *tar_buf,
        int tar_x, int tar_y,  
        unsigned int tar_width, unsigned int tar_height,
        double theta,
        unsigned int len,
        unsigned int broadness,
        WColorStruct fg_color
);
extern void BlitBufLineAdditive(
        unsigned int d,
        u_int8_t *tar_buf,
        int tar_x, int tar_y,
        unsigned int tar_width, unsigned int tar_height,
        double theta,
        unsigned int len,
        unsigned int broadness,
        WColorStruct fg_color
);

/* In blittile.c */
extern void BlitBufTile(
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
);


/* In blitcursor.c */
extern void BlitBufCursor(
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
);






#endif /* BLITTING_H */
