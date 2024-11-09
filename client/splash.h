#ifndef SPLASH_H
#define SPLASH_H

#include "../include/osw-x.h"
#include "../include/widget.h"


/*
 *	Default splash window size:
 *
 *	Used only if splash background image is missing.
 */
#define SW_DEF_WIDTH	320
#define SW_DEF_HEIGHT	240


/*
 *	Progress bar width (coefficient):
 */
#define SW_LS_BAR_WIDTHC	0.9

/*
 *	Progress bar y upper left position (coefficient):
 */
#define SW_LS_BAR_YC		0.78



/*
 *    Splash window:   
 */
typedef struct {

	char map_state;
	char is_in_focus;
	int x, y;
	unsigned int width, height;
	int visibility_state;

	win_t toplevel;
	pixmap_t toplevel_buf;

	image_t *bkg_img;
	image_t *pb_l_img,
		*pb_r_img,
		*pb_t_img;

} splash_win_struct;
extern splash_win_struct splash_win;


extern int SplashInit();
extern int SplashDoUpdateProgress(
        long items, long max_items,
        char *message
);
extern void SplashDestroy();


#endif /* SPLASH_H */
