/*
                              XSW Image Sets

	Image set referances (abbriviated isref) used by the
	client to display objects on the view screen.

 */


#ifndef ISREFS_H
#define ISREFS_H

#include <sys/types.h>

#include "os.h"


/*
 *      Maximum isrefs per client.
 */
#define ISREF_MAX               100000

/*
 *	Maximum frames per isref.
 */
#define ISREF_FRAMES_MAX	16


/*
 *	Option flags:
 */
#define ISREF_OPT_HAS_TRANSPARENCY	(1 << 2)
#define ISREF_OPT_NO_IMAGE		(1 << 3)
#define ISREF_OPT_STAY_LOADED		(1 << 4)


/*
 *	Special isref numbers:
 *
 *	These numbers are for special isrefs that
 *	are handled differently.
 */
#define ISREF_DEFAULT			0
/*
 *	Isrefs 10 to 19 are for stream weapons.
 *	Note: No graphical images are used for these isrefs.
 */
#define ISREF_STREAMWEAPON_YELLOW	10
#define ISREF_STREAMWEAPON_GREEN	11
#define ISREF_STREAMWEAPON_PURPLE	12
#define ISREF_STREAMWEAPON_ORANGE	13
/*
 *	Isrefs 20 to 29 are for special effects (such as explosions).
 */
#define ISREF_EXPLOSION_LARGE		20
#define ISREF_EXPLOSION_SMALL		21


/*
 *	Image format codes:
 */
#define ISREF_FORMAT_CODE_UNKNOWN	0
#define ISREF_FORMAT_CODE_TGA		1


/*
 *	Load progress codes:
 */
#define ISREF_LOAD_PROGRESS_DONE 	0
#define ISREF_LOAD_PROGRESS_LOADING	1


/*
 *	Merge modes:
 *
 *	Determines the mathimatical operation applied to the
 *	isref being blitted on to the buffer.
 */
#define ISREF_MERGE_NORMAL	0
#define ISREF_MERGE_ADDITIVE	1
#define ISREF_MERGE_SUBTRACTIVE	2
#define ISREF_MERGE_MIRAGE	3
#define ISREF_MERGE_MULTIPLY	4

/* 
 *	Frame determinat:
 *
 *	Determines how and where the frame number will be
 *	calculated from.
 */
#define ISREF_FDETERMINE_BY_HEADING	0
#define ISREF_FDETERMINE_BY_ANIMATION	1

/*
 *	Layer Placement
 *
 *      These are codes for determining the value for the member
 *      layer_placement of the isref_struct.
 *
 *	When an isref's layer_placement is set to any of the
 *	background codes then it will be visible as long as
 *	the 'viewport' is within it's size.
 */
#define ISREF_LAYER_FG		0	/* Normal object in foreground. */
#define ISREF_LAYER_BG_TILED	1
#define ISREF_LAYER_BG_STATIC	2

/*
 *	Special effects:
 *
 *      These are codes for determining the value for the member
 *      effects of the isref_struct.
 */
#define	ISREF_EFFECTS_STARGLOW		(1 << 1)
#define ISREF_EFFECTS_FADEINGLOW	(1 << 2)
#define ISREF_EFFECTS_FADEOUTGLOW	(1 << 3)



/*
 *	Data types:
 */
typedef u_int64_t	isref_options_mask_t;
typedef u_int64_t	isref_effects_mask_t;


/*
 *	Point light structure:
 *
 *	Used for drawing vector, hazard, and strobe lights.
 *
 *	Point lights should be drawn after the image is blitted.
 *
 *	The angle theta is relative to the object's north, not
 *	the galactic core north.
 */
typedef struct {

	/* Relative position. */
	double theta;		/* Bearing angle from object center. */
	double radius;		/* In Screen units. */

	/* Color. */
	u_int8_t a, r, g, b;

	/* Strobe intervals, set all to 0 for no strobing. */
	char	strobe_state;		/* True = on, False = off. */
	long	strobe_off_int,		/* In milliseconds. */
		strobe_on_int,		/* In milliseconds. */
		strobe_next;		/* In milliseconds. */

} isref_point_light_struct;


/*
 *	ISRef core structure:
 */
typedef struct {

	char *filename;		/* Absolute path to the image file. */

        int load_progress;	/* One of ISREF_LOAD_PROGRESS_*. */

	/* Options. */
	isref_options_mask_t option;

	/* Calculation methoids. */
	int merge_mode;		/* One of ISREF_MERGE_*. */
	int frame_determinant;	/* One of ISREF_FDETERMINE_*. */
	int layer_placement;	/* One of ISREF_LAYER_*. */
	isref_effects_mask_t effects;	/* Special effects. */

	double magnification;	/* Magnification, must be 1 or greater. */


	/* Size and frame attributes. */
	unsigned int total_frames;

	unsigned int	width,	/* Total width and height. */
			height;

	unsigned int	fwidth,	/* Frame width and height. */
			fheight;


	/*   Allocated structure containing image library data.
	 *   Pointer type determined by format_code.
	 *   This must be freed after calling library structure destroy
	 *   routine (as only it's substructures are deallocated by
	 *   library routine).
	 */
	void *lib_data;

	/* Format code for member lib_data, One of ISREF_FORMAT_CODE_*. */
        int format_code;


 	/*   Pointer to image data in lib_data, do not free this!
	 *   The call to the image library function will do that.
	 */
	u_int8_t *image_data;


	/* Point lights. */
	isref_point_light_struct **point_light;
	int total_point_lights;

} isref_struct;


#endif /* ISREFS_H */
