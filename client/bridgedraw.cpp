/*
                              Bridge: Drawing

	Functions:

	void BWDP_BACKGROUND(
	        int obj_num,		Subject object.
		xsw_object_struct *obj_ptr,
		pixmap_t pixmap,
	        int panel_detail
	)

	void BridgeWinDrawPanel(
	        int obj_num,		Subject object.
	        int panel_detail	One of BPANEL_DETAIL_*.
	)

	void BridgeWinDrawAll()

	int BridgeDrawMessages()

	---

	Note: View screen drawing functions are in vsdraw.c

 */

#include "../include/unvmatch.h"
#include "../include/unvmath.h"

#include "blitting.h"
#include "xsw.h"
#include "net.h"


#define RADIANS_TO_DEGREES(r) ((r) * 180 / PI)


/*
 *	Bridge console panels, sizes must match those in
 *	bridgeevent.c.
 */
#define BW_CONSOLE_PANEL_WIDTH	150
#define BW_CONSOLE_PANEL_HEIGHT 130


#define MIN(a,b)	(((a) < (b)) ? (a) : (b))
#define MAX(a,b)	(((a) > (b)) ? (a) : (b))


/*
 *      Bridge window message box character sizes:
 */
#define BRIDGE_MESG_CHAR_WIDTH  7
#define BRIDGE_MESG_CHAR_HEIGHT 14


/*
 *	Maximum chars per console label.
 */
#define BW_LABEL_MAX	80

/*
 *	Bar `curved end' margin, in pixels.
 */
#define BW_BAR_MARGIN	4

/*
 *	Bar concearn coefficient values.
 */
#define BW_COEFF_WARNING	0.58
#define BW_COEFF_DANGER		0.30





/*
 *	Redraws entire background according to panel_detail.
 *
 *	The entire background will be drawn to pixmap.
 */
void BWDP_BACKGROUND(
	int obj_num,		/* Subject object. */
        xsw_object_struct *obj_ptr,
	pixmap_t pixmap,
	int panel_detail
)
{
	int i;
	double z = 1;
	image_t *src_img_ptr = NULL,
		*tar_img_ptr = NULL;
	shared_image_t *shared_tar_img_ptr = NULL;
	int bkg_img_code = -1;
	int isref_num = -1;
	int frame = 12;
	isref_struct *isref_ptr;
	char cannot_draw_bkg = 0;


	/* Get image set referance number and pointer if available. */
	if(obj_ptr != NULL)
	{
	    isref_num = obj_ptr->imageset;
	    if(ISRefIsLoaded(isref_num))
	        isref_ptr = isref[isref_num];
	    else
	        isref_ptr = NULL;
	}
	else
	{
	    isref_ptr = NULL;
	}


	/* ******************************************************* */
	switch(panel_detail)
	{
          /* ***************************************************** */
          /* Player stats panel section 1. */
          case BPANEL_DETAIL_P1:
            /* Get source image pointer. */
            bkg_img_code = IMG_CODE_STATS_CON1;
            if(IMGIsImageNumLoaded(bkg_img_code))
                tar_img_ptr = xsw_image[bkg_img_code]->image;
            else
                tar_img_ptr = NULL;
            OSWPutImageToDrawable(tar_img_ptr, pixmap);
	    break;

          /* ***************************************************** */
          /* Player stats panel section 2 (uses a medium image). */
	  case BPANEL_DETAIL_P2:
	    /* Get target image pointer. */
	    shared_tar_img_ptr = bridge_win.pan_p2_img;
	    if(shared_tar_img_ptr == NULL)
		break;

	    /* Get source image pointer. */
	    bkg_img_code = IMG_CODE_STATS_CON2;
            if(IMGIsImageNumLoaded(bkg_img_code))
                src_img_ptr = xsw_image[bkg_img_code]->image;
            else
                break;
	    if(src_img_ptr == NULL)
		break;

	    BlitBufAbsolute(
		osw_gui[0].depth,
		shared_tar_img_ptr->data,
		reinterpret_cast<u_int8_t *>(src_img_ptr->data),
		0, 0,
		shared_tar_img_ptr->width, shared_tar_img_ptr->height,
		0, 0,
		src_img_ptr->width, src_img_ptr->height,
		src_img_ptr->width, src_img_ptr->height,
		1,	/* Zoom. */
		1	/* Magnification. */
	    );
	    /* Draw vessel image. */
	    if(isref_ptr != NULL)
	    {
		if(frame >= (int)isref_ptr->total_frames)
		    frame = 0;
		if(frame < 0)
		    frame = 0;

		if(isref_ptr->fwidth > BW_CONSOLE_PANEL_WIDTH)
		    z = BW_CONSOLE_PANEL_WIDTH /
			(double)isref_ptr->fwidth;
		if(isref_ptr->fheight > BW_CONSOLE_PANEL_HEIGHT)
                    z = BW_CONSOLE_PANEL_HEIGHT /
			(double)isref_ptr->fheight;


		if(isref_ptr->merge_mode == ISREF_MERGE_ADDITIVE)
		    BlitBufAdditive(
                        osw_gui[0].depth,
                        shared_tar_img_ptr->data,	/* Target. */
                        isref_ptr->image_data,		/* Source. */
                        static_cast<int>(MAX((BW_CONSOLE_PANEL_WIDTH / 2) -
			    (isref_ptr->fwidth * z / 2), 0)),
                        static_cast<int>(MAX((BW_CONSOLE_PANEL_HEIGHT / 2) -
			    (isref_ptr->fheight * z / 2), 0)),
                        shared_tar_img_ptr->width, shared_tar_img_ptr->height,
                        0, frame * isref_ptr->fheight,
                        isref_ptr->width, isref_ptr->height,  
                        isref_ptr->fwidth, isref_ptr->fheight,
                        z, 1
                    );
		else
		    BlitBufNormal(
		        osw_gui[0].depth,
                        shared_tar_img_ptr->data,	/* Target. */
                        isref_ptr->image_data,		/* Source. */
		        static_cast<int>(MAX((BW_CONSOLE_PANEL_WIDTH / 2) -
			    (isref_ptr->fwidth * z / 2), 0)),
                        static_cast<int>(MAX((BW_CONSOLE_PANEL_HEIGHT / 2) -
			    (isref_ptr->fheight * z / 2), 0)),
		        shared_tar_img_ptr->width, shared_tar_img_ptr->height,
		        0, frame * isref_ptr->fheight,
		        isref_ptr->width, isref_ptr->height,
		        isref_ptr->fwidth, isref_ptr->fheight,
		        z, 1, 1
		    );

		/*   Since isref ptr is not NULL it implies the vessel
                 *   is not garbage.  So draw readout outlines.
		 */
		i = IMG_CODE_BPANEL_OL_HULL;
		if(IMGIsImageNumLoaded(i))
		{
		    src_img_ptr = xsw_image[i]->image;
		    if(src_img_ptr != NULL)
		        BlitBufCursor(
			    osw_gui[0].depth,
			    shared_tar_img_ptr->data,
			    reinterpret_cast<u_int8_t *>(src_img_ptr->data),
			    6, 6,	/* Target x, y. */
			    shared_tar_img_ptr->width, shared_tar_img_ptr->height,
			    src_img_ptr->width, src_img_ptr->height,
			    xsw_color.bpol_hull_cv
			);
		}
                i = IMG_CODE_BPANEL_OL_POWER;
                if(IMGIsImageNumLoaded(i))
                {
                    src_img_ptr = xsw_image[i]->image;
                    if(src_img_ptr != NULL)
                        BlitBufCursor(
                            osw_gui[0].depth,
                            shared_tar_img_ptr->data,
                            reinterpret_cast<u_int8_t *>(src_img_ptr->data),
                            119, 6,	/* Target x, y. */
                            shared_tar_img_ptr->width, shared_tar_img_ptr->height,
                            src_img_ptr->width, src_img_ptr->height,
                            xsw_color.bpol_power_cv
                        );
                }
                i = IMG_CODE_BPANEL_OL_VIS;
                if(IMGIsImageNumLoaded(i))
                {
                    src_img_ptr = xsw_image[i]->image;
                    if(src_img_ptr != NULL)
                        BlitBufCursor(
                            osw_gui[0].depth,
                            shared_tar_img_ptr->data,
                            reinterpret_cast<u_int8_t *>(src_img_ptr->data),
                            119, 62,	/* Target x, y. */
                            shared_tar_img_ptr->width, shared_tar_img_ptr->height,
                            src_img_ptr->width, src_img_ptr->height,
                            xsw_color.bpol_vis_cv
                        );
                }
                i = IMG_CODE_BPANEL_OL_SHIELDS;
                if(IMGIsImageNumLoaded(i))
                {
                    src_img_ptr = xsw_image[i]->image;
                    if(src_img_ptr != NULL)
                        BlitBufCursor(
                            osw_gui[0].depth,
                            shared_tar_img_ptr->data,
                            reinterpret_cast<u_int8_t *>(src_img_ptr->data),
                            6, 62,	/* Target x, y. */
                            shared_tar_img_ptr->width, shared_tar_img_ptr->height,
                            src_img_ptr->width, src_img_ptr->height,
                            xsw_color.bpol_shields_cv
                        );
                }
                i = IMG_CODE_BPANEL_OL_DMGCTL;
                if(IMGIsImageNumLoaded(i))
                {
                    src_img_ptr = xsw_image[i]->image;
                    if(src_img_ptr != NULL)
                        BlitBufCursor(
                            osw_gui[0].depth,
                            shared_tar_img_ptr->data,
                            reinterpret_cast<u_int8_t *>(src_img_ptr->data),
                            30, 106,		/* Target x, y. */
                            shared_tar_img_ptr->width, shared_tar_img_ptr->height,
                            src_img_ptr->width, src_img_ptr->height,
                            xsw_color.bpol_dmgctl_cv
                        );
                }
	    }
            OSWPutSharedImageToDrawable(shared_tar_img_ptr, pixmap);
	    break;

	  /* ****************************************************** */
	  /* Player stats panel section 3 (uses a medium image). */
          case BPANEL_DETAIL_P3:
            /* Get target image pointer. */
            shared_tar_img_ptr = bridge_win.pan_p3_img;
            if(shared_tar_img_ptr == NULL)
                break;

            /* Get source image pointer. */
            bkg_img_code = IMG_CODE_STATS_CON3;
            if(IMGIsImageNumLoaded(bkg_img_code))
                src_img_ptr = xsw_image[bkg_img_code]->image;
            else
                break;
            if(src_img_ptr == NULL)
                break;

            BlitBufAbsolute(
                osw_gui[0].depth,
                shared_tar_img_ptr->data,
                reinterpret_cast<u_int8_t *>(src_img_ptr->data),
                0, 0,
                shared_tar_img_ptr->width, shared_tar_img_ptr->height,
                0, 0,
                src_img_ptr->width, src_img_ptr->height,
                src_img_ptr->width, src_img_ptr->height,
                1,      /* Zoom. */ 
                1       /* Magnification. */
            );

	    /* Draw graphics specific to when object is valid. */
	    if(obj_ptr != NULL)
	    {

		/* Left throttle outline. */
                i = IMG_CODE_BPANEL_OL_LTHROTTLE;
                if(IMGIsImageNumLoaded(i))
                {
                    src_img_ptr = xsw_image[i]->image;
                    if(src_img_ptr != NULL)
                        BlitBufCursor(
                            osw_gui[0].depth,
                            shared_tar_img_ptr->data,
                            reinterpret_cast<u_int8_t *>(src_img_ptr->data),
                            6, 35,	/* Target x, y. */
                            shared_tar_img_ptr->width, shared_tar_img_ptr->height,
                            src_img_ptr->width, src_img_ptr->height,
                            xsw_color.bpol_throttle_cv
                        );
                }
                /* Right throttle outline. */
                i = IMG_CODE_BPANEL_OL_RTHROTTLE;
                if(IMGIsImageNumLoaded(i))
                {
                    src_img_ptr = xsw_image[i]->image;
                    if(src_img_ptr != NULL)
                        BlitBufCursor(
                            osw_gui[0].depth,
                            shared_tar_img_ptr->data,
                            reinterpret_cast<u_int8_t *>(src_img_ptr->data),
                            BW_CONSOLE_PANEL_WIDTH - src_img_ptr->width - 6,
			    35,
                            shared_tar_img_ptr->width, shared_tar_img_ptr->height,
                            src_img_ptr->width, src_img_ptr->height,
                            xsw_color.bpol_throttle_cv
                        );
                }

		/* Velocity ruller. */
                BlitBufLine(
                    osw_gui[0].depth,
                    shared_tar_img_ptr->data,
                    6, 8,
                    shared_tar_img_ptr->width, shared_tar_img_ptr->height,
                    (PI / 2),   /* Bearing. */
                    138,
                    1,          /* Broadness. */
                    xsw_color.bp_dark_outline_cv
                );
                BlitBufLine(
                    osw_gui[0].depth,
                    shared_tar_img_ptr->data,
                    6, 7,
                    shared_tar_img_ptr->width, shared_tar_img_ptr->height,
                    0,		/* Bearing. */ 
                    5,
                    1,          /* Broadness. */
                    xsw_color.bp_normal_outline_cv
                );
                BlitBufLine(
                    osw_gui[0].depth,
                    shared_tar_img_ptr->data,
                    144, 7,
                    shared_tar_img_ptr->width, shared_tar_img_ptr->height,
                    0,		/* Bearing. */ 
                    5,
                    1,          /* Broadness. */
                    xsw_color.bp_normal_outline_cv
                );

		/* Thrust vector outline. */
                i = IMG_CODE_BPANEL_OL_THRUSTVECTOR;
                if(IMGIsImageNumLoaded(i))
                {
                    src_img_ptr = xsw_image[i]->image;
                    if(src_img_ptr != NULL)
                        BlitBufCursor(
                            osw_gui[0].depth,
                            shared_tar_img_ptr->data,
                            reinterpret_cast<u_int8_t *>(src_img_ptr->data),
                            40 + (30 / 2) -
				((int)src_img_ptr->width / 2),
			    92 + (30 / 2) -
				((int)src_img_ptr->height / 2),
                            shared_tar_img_ptr->width,
			    shared_tar_img_ptr->height,
                            src_img_ptr->width,
			    src_img_ptr->height,
                            xsw_color.bpol_throttle_cv
                        );
                }

		/* Engine state outline. */
/*
        int x = 75;
        int y = 92;   
        unsigned int width = 40;
        unsigned int height = 30;
 */

	    }
            OSWPutSharedImageToDrawable(shared_tar_img_ptr, pixmap);
            break;

          /* ****************************************************** */
          /* Player stats panel section 4. */
          case BPANEL_DETAIL_P4:
            /* Get source image pointer. */
            bkg_img_code = IMG_CODE_STATS_CON4;
            if(IMGIsImageNumLoaded(bkg_img_code))
                tar_img_ptr = xsw_image[bkg_img_code]->image;
            else
                tar_img_ptr = NULL;
            OSWPutImageToDrawable(tar_img_ptr, pixmap);
            break;

          /* ***************************************************** */
          /* Subject stats panel section 1 (uses a medium image). */
	  case BPANEL_DETAIL_S1:
            /* Get target image pointer. */
            shared_tar_img_ptr = bridge_win.pan_s1_img;
            if(shared_tar_img_ptr == NULL)
                break;

            /* Get source image pointer. */
            bkg_img_code = IMG_CODE_SRO_CON1;
            if(IMGIsImageNumLoaded(bkg_img_code))
                src_img_ptr = xsw_image[bkg_img_code]->image;
            else
                break;
            if(src_img_ptr == NULL)
                break;

            BlitBufAbsolute(
                osw_gui[0].depth,
                shared_tar_img_ptr->data,
                reinterpret_cast<u_int8_t *>(src_img_ptr->data),
                0, 0,
                shared_tar_img_ptr->width, shared_tar_img_ptr->height,
                0, 0, 
                src_img_ptr->width, src_img_ptr->height,
                src_img_ptr->width, src_img_ptr->height,
                1,      /* Zoom. */
                1       /* Magnification. */
            );
	    /* Draw vessel image. */
            if(isref_ptr != NULL)
            {
                if(frame >= (int)isref_ptr->total_frames)
                    frame = 0;
                if(frame < 0)
                    frame = 0;

                if(isref_ptr->fwidth > BW_CONSOLE_PANEL_WIDTH)
                    z = BW_CONSOLE_PANEL_WIDTH /
			(double)isref_ptr->fwidth;
                if(isref_ptr->fheight > BW_CONSOLE_PANEL_HEIGHT)
                    z = BW_CONSOLE_PANEL_HEIGHT /
			(double)isref_ptr->fheight;

                if(isref_ptr->merge_mode == ISREF_MERGE_ADDITIVE)   
                    BlitBufAdditive(
                        osw_gui[0].depth,
                        shared_tar_img_ptr->data,	/* Target. */
                        isref_ptr->image_data,          /* Source. */
                        static_cast<int>(MAX((BW_CONSOLE_PANEL_WIDTH / 2) -
			    (isref_ptr->fwidth * z / 2), 0)),
                        static_cast<int>(MAX((BW_CONSOLE_PANEL_HEIGHT / 2) -
			    (isref_ptr->fheight * z / 2), 0)),
                        shared_tar_img_ptr->width, shared_tar_img_ptr->height,
                        0, frame * isref_ptr->fheight,
                        isref_ptr->width, isref_ptr->height,
                        isref_ptr->fwidth, isref_ptr->fheight,
                        z, 1
                    );
                else                
                    BlitBufNormal(
                        osw_gui[0].depth,
                        shared_tar_img_ptr->data,	/* Target. */
                        isref_ptr->image_data,	/* Source. */
                        static_cast<int>(MAX((BW_CONSOLE_PANEL_WIDTH / 2) -
			    (isref_ptr->fwidth * z / 2), 0)),
                        static_cast<int>(MAX((BW_CONSOLE_PANEL_HEIGHT / 2) -
			    (isref_ptr->fheight * z / 2), 0)),
                        shared_tar_img_ptr->width, shared_tar_img_ptr->height,
                        0, frame * isref_ptr->fheight,
                        isref_ptr->width, isref_ptr->height,
                        isref_ptr->fwidth, isref_ptr->fheight,
                        z, 1, 1
                    );


                /*   Since isref ptr is not NULL it implies the vessel
                 *   is not garbage.  So draw readout outlines.
                 */
                i = IMG_CODE_BPANEL_OL_HULL;
                if(IMGIsImageNumLoaded(i))
                {
                    src_img_ptr = xsw_image[i]->image;
                    if(src_img_ptr != NULL)  
                        BlitBufCursor(
                            osw_gui[0].depth, 
                            shared_tar_img_ptr->data,
                            reinterpret_cast<u_int8_t *>(src_img_ptr->data),
                            6, 16,       /* Target x, y. */
                            shared_tar_img_ptr->width, shared_tar_img_ptr->height,
                            src_img_ptr->width, src_img_ptr->height,
                            xsw_color.bpol_hull_cv
                        );
                }
                i = IMG_CODE_BPANEL_OL_POWER;
                if(IMGIsImageNumLoaded(i))
                {
                    src_img_ptr = xsw_image[i]->image;
                    if(src_img_ptr != NULL)
                        BlitBufCursor(
                            osw_gui[0].depth,
                            shared_tar_img_ptr->data,
                            reinterpret_cast<u_int8_t *>(src_img_ptr->data),
                            119, 16,     /* Target x, y. */  
                            shared_tar_img_ptr->width, shared_tar_img_ptr->height,
                            src_img_ptr->width, src_img_ptr->height,
                            xsw_color.bpol_power_cv
                        );
                }
                i = IMG_CODE_BPANEL_OL_VIS;
                if(IMGIsImageNumLoaded(i)) 
                {
                    src_img_ptr = xsw_image[i]->image;
                    if(src_img_ptr != NULL)   
                        BlitBufCursor(
                            osw_gui[0].depth,
                            shared_tar_img_ptr->data,
                            reinterpret_cast<u_int8_t *>(src_img_ptr->data),
                            119, 72,    /* Target x, y. */
                            shared_tar_img_ptr->width, shared_tar_img_ptr->height,
                            src_img_ptr->width, src_img_ptr->height,
                            xsw_color.bpol_vis_cv
                        );
                } 
                i = IMG_CODE_BPANEL_OL_SHIELDS;
                if(IMGIsImageNumLoaded(i))
                {
                    src_img_ptr = xsw_image[i]->image;
                    if(src_img_ptr != NULL)
                        BlitBufCursor(
                            osw_gui[0].depth,
                            shared_tar_img_ptr->data,
                            reinterpret_cast<u_int8_t *>(src_img_ptr->data),
                            6, 72,      /* Target x, y. */
                            shared_tar_img_ptr->width, shared_tar_img_ptr->height,
                            src_img_ptr->width, src_img_ptr->height,
                            xsw_color.bpol_shields_cv
                        );
                }

                i = IMG_CODE_BPANEL_OL_DMGCTL;
/* Do not draw damage control state for subject object.
                if(IMGIsImageNumLoaded(i))
 */
		if(0)
                {
                    src_img_ptr = xsw_image[i]->image;
                    if(src_img_ptr != NULL)
                        BlitBufCursor(
                            osw_gui[0].depth,
                            shared_tar_img_ptr->data,
                            reinterpret_cast<u_int8_t *>(src_img_ptr->data),
                            30, 106,	/* Target x, y. */
                            shared_tar_img_ptr->width, shared_tar_img_ptr->height,
                            src_img_ptr->width, src_img_ptr->height,
                            xsw_color.bpol_dmgctl_cv
                        );
                }

		/* Draw bearing circle. */
		BlitBufCircle(
		    osw_gui[0].depth,
		    shared_tar_img_ptr->data,
		    (BW_CONSOLE_PANEL_WIDTH / 2),
		    (BW_CONSOLE_PANEL_HEIGHT / 2),
		    shared_tar_img_ptr->width, shared_tar_img_ptr->height,
		    1,
		    45,
		    xsw_color.bp_dark_outline_cv
		);

		/* Draw distance line. */
		BlitBufLine(
		    osw_gui[0].depth,
                    shared_tar_img_ptr->data,
                    40, 129,
		    shared_tar_img_ptr->width, shared_tar_img_ptr->height,
		    (PI / 2),	/* Bearing. */
		    70,
		    1,		/* Broadness. */
		    xsw_color.bp_dark_outline_cv
		);
                BlitBufLine(
                    osw_gui[0].depth,
                    shared_tar_img_ptr->data,
                    40, 128,
                    shared_tar_img_ptr->width, shared_tar_img_ptr->height,
                    0,		/* Bearing. */
                    5,
                    1,          /* Broadness. */
                    xsw_color.bp_normal_outline_cv
                );
                BlitBufLine(
                    osw_gui[0].depth,
                    shared_tar_img_ptr->data,
                    110, 128,
                    shared_tar_img_ptr->width, shared_tar_img_ptr->height,
                    0,		/* Bearing. */
                    5,
                    1,          /* Broadness. */
                    xsw_color.bp_normal_outline_cv
                );
            }
            OSWPutSharedImageToDrawable(shared_tar_img_ptr, pixmap);
	    break;

          /* ***************************************************** */
          /* Subject stats panel section 2. */
          case BPANEL_DETAIL_S2:
            /* Get source image pointer. */
            bkg_img_code = IMG_CODE_SRO_CON2;
            if(IMGIsImageNumLoaded(bkg_img_code))
                tar_img_ptr = xsw_image[bkg_img_code]->image;
            else
                tar_img_ptr = NULL;
            OSWPutImageToDrawable(tar_img_ptr, pixmap);
            break;

          /* ***************************************************** */
          /* Subject stats panel section 3. */
          case BPANEL_DETAIL_S3:
            /* Get target image pointer. */
            shared_tar_img_ptr = bridge_win.pan_s3_img;
            if(shared_tar_img_ptr == NULL)
                break;

            /* Get source image pointer. */
            bkg_img_code = IMG_CODE_SRO_CON3;
            if(IMGIsImageNumLoaded(bkg_img_code))
                src_img_ptr = xsw_image[bkg_img_code]->image;
            else
                break;
            if(src_img_ptr == NULL)
                break;

            BlitBufAbsolute(
                osw_gui[0].depth,
                shared_tar_img_ptr->data,
                reinterpret_cast<u_int8_t *>(src_img_ptr->data),
                0, 0,
                shared_tar_img_ptr->width, shared_tar_img_ptr->height,
                0, 0,
                src_img_ptr->width, src_img_ptr->height,
                src_img_ptr->width, src_img_ptr->height,
                1,      /* Zoom. */
                1       /* Magnification. */
            );
            OSWPutImageToDrawable(src_img_ptr, pixmap);
            break;

          /* ***************************************************** */
	  default:
	    fprintf(stderr,
 "BWDP_BACKGROUND(): Unsupported panel detail code `%i'.\n",
		panel_detail
	    );
	    break;
	}


        if(cannot_draw_bkg)
            OSWClearPixmap(
                pixmap,  
                bridge_win.pan_p1_width,
                bridge_win.pan_p1_height,
                osw_gui[0].black_pix
            );


	return;
}





/*
 *	Player name.
 */
void BWDP_PNAME(
	int obj_num, xsw_object_struct *obj_ptr,
	font_t *font, pixel_t pix,
	pixmap_t pixmap, win_t w,
	int bkg_img_code,
	bool_t put_to_window
)
{
	int x = 3;
	int y = 8;
	unsigned int width = 144;
	unsigned int height = 16;

	image_t *img_ptr;

	char *strptr;
	char text[BW_LABEL_MAX];


	if(IMGIsImageNumLoaded(bkg_img_code))
	    img_ptr = xsw_image[bkg_img_code]->image;
	else
	    return;

	/* Draw up background. */
	OSWPutImageToDrawableSect(
	    img_ptr, pixmap,
	    x, y,
	    x, y,
	    width, height
	);

	/* Draw text. */
	if(obj_ptr != NULL)
	{
	    if(option.show_formal_label)
	    {
	        strptr = DBGetFormalNameStr(obj_num);
	        if(strptr != NULL)
	            strncpy(text, strptr, BW_LABEL_MAX);
	        else
	            strncpy(text, "", BW_LABEL_MAX);
	    }
	    else
	    {
		strncpy(text, obj_ptr->name, BW_LABEL_MAX);
	    }
	    text[BW_LABEL_MAX - 1] = '\0';
	    OSWSetFont(font);
	    OSWSetFgPix(pix);
	    OSWDrawString(
	        pixmap,
	        x + 4,
	        y + 12,
	        text
	    );
	}

	/* Put to window. */
	if(put_to_window)
	{
	    OSWCopyDrawablesCoord(
	        w, pixmap,
	        x, y,
	        width, height,
	        x, y
	    );
	}


	return;
}


/*
 *	Player shield frequency.
 */
void BWDP_PSHIELDFREQ(
        int obj_num, xsw_object_struct *obj_ptr,
        font_t *font, pixel_t pix,
        pixmap_t pixmap, win_t w,
        int bkg_img_code,
        bool_t put_to_window
)
{
        int x = 3;
        int y = 28;
        unsigned int width = 144;
        unsigned int height = 12;

        image_t *img_ptr;

        char text[BW_LABEL_MAX];


        if(IMGIsImageNumLoaded(bkg_img_code))
            img_ptr = xsw_image[bkg_img_code]->image;
        else
            return;

        /* Draw up background. */
        OSWPutImageToDrawableSect(
            img_ptr, pixmap,
            x, y,
            x, y,
            width, height
        );

        /* Draw text. */
	if(obj_ptr != NULL)
	{
	    sprintf(text, "Shield Freq: %.2f kHz",
	        obj_ptr->shield_frequency
	    );
            OSWSetFont(font);
            OSWSetFgPix(pix);
            OSWDrawString(
                pixmap,
                x + 3,
                y + 7,
                text
            );
	}

        /* Put to window. */
        if(put_to_window)
        {
            OSWCopyDrawablesCoord(
                w, pixmap,
                x, y,
                width, height,
                x, y
            );
        }
}


/*
 *	Player weapon frequency.
 */
void BWDP_PWEAPONFREQ(
        int obj_num, xsw_object_struct *obj_ptr,
        font_t *font, pixel_t pix,
        pixmap_t pixmap, win_t w,
        int bkg_img_code,
	bool_t put_to_window
)
{
        int x = 3;
        int y = 42;
        unsigned int width = 144;
        unsigned int height = 12;

        image_t *img_ptr;

        char text[BW_LABEL_MAX];


        if(IMGIsImageNumLoaded(bkg_img_code))
            img_ptr = xsw_image[bkg_img_code]->image;
        else
            return;

        /* Draw up background. */
        OSWPutImageToDrawableSect(
            img_ptr, pixmap,
            x, y,
            x, y,
            width, height
        );

        /* Draw text. */
        if(obj_ptr != NULL)
        {
            sprintf(text, "Weapon Freq: %.2f kHz",
                local_control.weapon_freq
            );
            OSWSetFont(font);
            OSWSetFgPix(pix);
            OSWDrawString(
                pixmap,
                x + 3,
                y + 7,
                text
            );
        }


        /* Put to window. */
        if(put_to_window)
        {
            OSWCopyDrawablesCoord(
                w, pixmap,
                x, y,
                width, height,
                x, y
            );
        }
}


/*
 *	Player communications channel.
 */
void BWDP_PCOMCHANNEL(
        int obj_num, xsw_object_struct *obj_ptr,
        font_t *font, pixel_t pix,
        pixmap_t pixmap, win_t w,
        int bkg_img_code,
        bool_t put_to_window
)
{
        int x = 3;
        int y = 54;
        unsigned int width = 144;
        unsigned int height = 12;

        image_t *img_ptr;

        char text[BW_LABEL_MAX];


        if(IMGIsImageNumLoaded(bkg_img_code))
            img_ptr = xsw_image[bkg_img_code]->image;
        else
            return;

        /* Draw up background. */
        OSWPutImageToDrawableSect(
            img_ptr, pixmap,
            x, y,
            x, y,
            width, height
        );

        /* Draw text. */
        if(obj_ptr != NULL)
        {
            sprintf(text, "Com Channel: %.2f kHz",
                (double)obj_ptr->com_channel / 100.0
            );
            OSWSetFont(font);
            OSWSetFgPix(pix);
            OSWDrawString(
                pixmap,
                x + 3,
                y + 7,
                text
            );
        }

        /* Put to window. */
        if(put_to_window)
        {
            OSWCopyDrawablesCoord(
                w, pixmap,
                x, y,
                width, height,
                x, y
            );
        }
}


/*
 *	Player hull.
 */
void BWDP_PHULL(
        int obj_num, xsw_object_struct *obj_ptr,
        font_t *font, pixel_t pix,
        pixmap_t pixmap, win_t w,
        int bkg_img_code,
        bool_t put_to_window
)
{
        int x = 6; 
        int y = 6;
        unsigned int width = 8;
        unsigned int height = 50;
	double c;

        shared_image_t *img_ptr;


	img_ptr = bridge_win.pan_p2_img;
	if(img_ptr == NULL)
            return;

        /* Draw up background. */
        OSWPutSharedImageToDrawableSect(
            img_ptr, pixmap,
            x, y,
            x, y,
            width, height
        );

        /* Draw bar. */
        if(obj_ptr != NULL)
        {
	    if(obj_ptr->hp_max != 0)
	        c = MAX((double)obj_ptr->hp / (double)obj_ptr->hp_max, 0);
	    else
		c = 0;


            if(c > BW_COEFF_WARNING)
                OSWSetFgPix(pix);
            else if(c > BW_COEFF_DANGER) 
                OSWSetFgPix(xsw_color.bp_warning);
            else
                OSWSetFgPix(xsw_color.bp_danger);
            OSWDrawSolidRectangle(
                pixmap,
                x,
                static_cast<int>(y + BW_BAR_MARGIN +
                    (((int)height - (2 * BW_BAR_MARGIN)) *
                     (double)(1 - c)
                    ))
		,
                6,
                static_cast<unsigned int>(((int)height - (2 * BW_BAR_MARGIN)) * c)
            );
        }

        /* Put to window. */
        if(put_to_window)
        {
            OSWCopyDrawablesCoord(
                w, pixmap,
                x, y,
                width, height,
                x, y
            );
        }


        return;
}

/*              
 *	Player core power.
 */
void BWDP_PPOWER(
        int obj_num, xsw_object_struct *obj_ptr,
        font_t *font, pixel_t pix,
        pixmap_t pixmap, win_t w,
        int bkg_img_code,
        bool_t put_to_window
)
{
        int x = 136;
        int y = 6;
        unsigned int width = 8;
        unsigned int height = 50;
        double c;

        shared_image_t *img_ptr;


        img_ptr = bridge_win.pan_p2_img;
        if(img_ptr == NULL)
            return;

        /* Draw up background. */
        OSWPutSharedImageToDrawableSect(
            img_ptr, pixmap,
            x, y,
            x, y,
            width, height
        );

        /* Draw bar. */
        if(obj_ptr != NULL)
        {
            if(obj_ptr->power_max != 0)
                c = MAX((double)obj_ptr->power / (double)obj_ptr->power_max, 0);
            else
                c = 0;

            if(c > BW_COEFF_WARNING)
                OSWSetFgPix(pix);
            else if(c > BW_COEFF_DANGER) 
                OSWSetFgPix(xsw_color.bp_warning);
            else
                OSWSetFgPix(xsw_color.bp_danger);
            OSWDrawSolidRectangle(
                pixmap,
                x + 1,
                static_cast<int>(y + BW_BAR_MARGIN +
                    (((int)height - (2 * BW_BAR_MARGIN)) *
                     (double)(1 - c)
                    ))
                ,
                6,
                static_cast<unsigned int>(((int)height - (2 * BW_BAR_MARGIN)) * c)
            );
        }

        /* Put to window. */
        if(put_to_window)
        {
            OSWCopyDrawablesCoord(
                w, pixmap,
                x, y, 
                width, height,
                x, y  
            );   
        }


        return;   
}

/*              
 *	Player shields.
 */
void BWDP_PSHIELDS(
        int obj_num, xsw_object_struct *obj_ptr,
        font_t *font, pixel_t pix,
        pixmap_t pixmap, win_t w,
        int bkg_img_code,
        bool_t put_to_window
)
{
        int x = 6;
        int y = 62;
        unsigned int width = 8;
        unsigned int height = 50;
        double c;

        shared_image_t *img_ptr;
                  
            
        img_ptr = bridge_win.pan_p2_img;
        if(img_ptr == NULL)   
            return; 
              
        /* Draw up background. */
        OSWPutSharedImageToDrawableSect(
            img_ptr, pixmap,
            x, y,
            x, y,
            width, height
        );

        /* Draw bar. */
        if(obj_ptr != NULL)
        {
            if(obj_ptr->power_max != 0)
                c = MAX((double)obj_ptr->power / (double)obj_ptr->power_max, 0);
            else
                c = 1;

            OSWSetFgPix(pix);
	    if(obj_ptr->shield_state >= 0)
	    {
		if(obj_ptr->shield_state == 1)
		{
		    if(c > BW_COEFF_WARNING)
			OSWSetFgPix(pix);
		    else if(c > BW_COEFF_DANGER)
			OSWSetFgPix(xsw_color.bp_warning);
		    else
			OSWSetFgPix(xsw_color.bp_danger);
                    OSWDrawSolidRectangle(
                        pixmap,
                        x,
	 	        static_cast<int>(y + ((int)height / 2) -
                            ((((int)height - (2 * BW_BAR_MARGIN)) * c) / 2)),
                        6,
                        static_cast<unsigned int>(((int)height - (2 * BW_BAR_MARGIN)) * c)
                    );
		}
	    }
        }

        /* Put to window. */
        if(put_to_window)
        {
            OSWCopyDrawablesCoord(
                w, pixmap,
                x, y, 
                width, height,
                x, y  
            );   
        }


        return;   
}

/*              
 *	Player visibility.
 */
void BWDP_PVIS(
        int obj_num, xsw_object_struct *obj_ptr,
        font_t *font, pixel_t pix,
        pixmap_t pixmap, win_t w,
        int bkg_img_code,
        bool_t put_to_window
)
{
        int x = 136;
        int y = 62;
        unsigned int width = 8;
        unsigned int height = 50;
        double c;

        shared_image_t *img_ptr;


        img_ptr = bridge_win.pan_p2_img;
        if(img_ptr == NULL)   
            return; 
              
        /* Draw up background. */
        OSWPutSharedImageToDrawableSect(
            img_ptr, pixmap,
            x, y,
            x, y,
            width, height
        );

        /* Draw bar. */
        if(obj_ptr != NULL)
        {
            c = MAX(DBGetObjectVisibility(obj_num), 0);

            OSWSetFgPix(pix);
            OSWDrawSolidRectangle(
                pixmap,
                x + 1,
                static_cast<int>(y + BW_BAR_MARGIN +
                    (((int)height - (2 * BW_BAR_MARGIN)) *
                     (double)(1 - c)
                    ))
                ,
                6,
                static_cast<unsigned int>(((int)height - (2 * BW_BAR_MARGIN)) * c)
            );
        }

        /* Put to window. */
        if(put_to_window)
        {
            OSWCopyDrawablesCoord(
                w, pixmap,
                x, y, 
                width, height,
                x, y  
            );   
        }


        return;   
}


/*
 *	Player damage control.
 */
void BWDP_PDMGCTL(
        int obj_num, xsw_object_struct *obj_ptr,
        font_t *font, pixel_t pix,
        pixmap_t pixmap, win_t w,
        int bkg_img_code,
        bool_t put_to_window
)
{
        int x = 30;
        int y = 123;
        unsigned int width = 42;
        unsigned int height = 7;
        double c;

        shared_image_t *img_ptr;


        img_ptr = bridge_win.pan_p2_img;
        if(img_ptr == NULL)   
            return; 

        /* Draw up background. */
        OSWPutSharedImageToDrawableSect(
            img_ptr, pixmap,
            x, y,
            x, y,
            width, height
        );

        /* Draw bar. */
        if(obj_ptr != NULL)
        {
	    if(obj_ptr->damage_control == 1)
	    {
                if(obj_ptr->power_max != 0)
                    c = (double)obj_ptr->power /
			(double)obj_ptr->power_max;
                else
                    c = 1;

                OSWSetFgPix(pix);
                OSWDrawSolidRectangle(
                    pixmap,
                    x + BW_BAR_MARGIN,
		    y + 2,
                    static_cast<unsigned int>(((int)width - (2 * BW_BAR_MARGIN)) * c),
		    6
                );
	    }
        }

        /* Put to window. */
        if(put_to_window)
        {
            OSWCopyDrawablesCoord(
                w, pixmap,
                x, y, 
                width, height,
                x, y  
            );   
        }


        return;   
}


/*
 *	Subject name.
 */
void BWDP_SNAME(
        int obj_num, xsw_object_struct *obj_ptr,
        font_t *font, pixel_t pix,
        pixmap_t pixmap, win_t w,
        int bkg_img_code,
        bool_t put_to_window
)
{
        int x = 3;
        int y = 0;
        unsigned int width = 144;
        unsigned int height = 16;

        shared_image_t *img_ptr;  

        char *strptr;
        char text[BW_LABEL_MAX];


        img_ptr = bridge_win.pan_s1_img;

        /* Draw up background. */
        OSWPutSharedImageToDrawableSect(
            img_ptr, pixmap,
            x, y,
            x, y,
            width, height
        );

        /* Draw text. */
        if(obj_ptr != NULL)
        {
            switch(option.show_formal_label)
            {
	      /* Always. */
	      case 2:
                strptr = DBGetFormalNameStr(obj_num);
                if(strptr != NULL)
                    strncpy(text, strptr, BW_LABEL_MAX);
                else
                    strncpy(text, "", BW_LABEL_MAX);
		break;

	      /* As needed. */
              case 1:
		if(net_parms.player_obj_num == obj_ptr->owner)
		{
                    strptr = DBGetFormalNameStr(obj_num);
                    if(strptr != NULL)
                        strncpy(text, strptr, BW_LABEL_MAX);
                    else
                        strncpy(text, "", BW_LABEL_MAX);
		}
		else
		{
                    strncpy(text, obj_ptr->name, BW_LABEL_MAX);
		}
		break;

	      /* Never. */
	      default:
                strncpy(text, obj_ptr->name, BW_LABEL_MAX);
		break;
            }
            text[BW_LABEL_MAX - 1] = '\0';
            OSWSetFont(font);
            OSWSetFgPix(pix);
            OSWDrawString(
                pixmap,
                x + 4,
                y + 12,
                text
            );
        }

        /* Put to window. */
        if(put_to_window)
        {
            OSWCopyDrawablesCoord(
                w, pixmap,
                x, y,
                width, height,
                x, y
            );
        }


        return;
}

/*
 *	Subject hull.
 */
void BWDP_SHULL(
        int obj_num, xsw_object_struct *obj_ptr,
        font_t *font, pixel_t pix,
        pixmap_t pixmap, win_t w,
        int bkg_img_code,
        bool_t put_to_window
)
{
        int x = 6;
        int y = 16;
        unsigned int width = 8;
        unsigned int height = 50;
        double c;

        shared_image_t *img_ptr;


        img_ptr = bridge_win.pan_s1_img;
        if(img_ptr == NULL)
            return;

        /* Draw up background. */
        OSWPutSharedImageToDrawableSect(
            img_ptr, pixmap,
            x, y,
            x, y,
            width, height
        );
                
        /* Draw bar. */
        if(obj_ptr != NULL)
        {
            if(obj_ptr->hp_max != 0)
                c = MAX((double)obj_ptr->hp / (double)obj_ptr->hp_max, 0);
            else
                c = 0;

 
            if(c > BW_COEFF_WARNING)
                OSWSetFgPix(pix);
            else if(c > BW_COEFF_DANGER)
                OSWSetFgPix(xsw_color.bp_warning);
            else  
                OSWSetFgPix(xsw_color.bp_danger);
            OSWDrawSolidRectangle(
                pixmap,
                x,
                static_cast<int>(y + BW_BAR_MARGIN +
                    (((int)height - (2 * BW_BAR_MARGIN)) *
                     (double)(1 - c)
                    ))
                ,
                6, 
                static_cast<unsigned int>(((int)height - (2 * BW_BAR_MARGIN)) * c)
            );
        }
        
        /* Put to window. */
        if(put_to_window)
        {
            OSWCopyDrawablesCoord(
                w, pixmap,
                x, y,  
                width, height,
                x, y
            );
        }
            
                
        return;
}

/*
 *	Subject core power.
 */
void BWDP_SPOWER( 
        int obj_num, xsw_object_struct *obj_ptr,
        font_t *font, pixel_t pix,
        pixmap_t pixmap, win_t w,
        int bkg_img_code,
        bool_t put_to_window
)
{
        int x = 136; 
        int y = 16;
        unsigned int width = 8;
        unsigned int height = 50;
        double c;

        shared_image_t *img_ptr;


        img_ptr = bridge_win.pan_s1_img;
        if(img_ptr == NULL)
            return;
                
        /* Draw up background. */
        OSWPutSharedImageToDrawableSect(
            img_ptr, pixmap,
            x, y,
            x, y,
            width, height
        );

        /* Draw bar. */
        if(obj_ptr != NULL)
        {
            if(obj_ptr->power_max != 0)
                c = MAX((double)obj_ptr->power / (double)obj_ptr->power_max, 0);
            else
                c = 0;
        
            if(c > BW_COEFF_WARNING)
                OSWSetFgPix(pix);
            else if(c > BW_COEFF_DANGER)
                OSWSetFgPix(xsw_color.bp_warning);
            else
                OSWSetFgPix(xsw_color.bp_danger);
            OSWDrawSolidRectangle(
                pixmap,
                x + 1,
                static_cast<int>(y + BW_BAR_MARGIN +
                    (((int)height - (2 * BW_BAR_MARGIN)) *
                     (double)(1 - c)
                    ))
                ,
                6,
                static_cast<unsigned int>(((int)height - (2 * BW_BAR_MARGIN)) * c)
            );  
        }
        
        /* Put to window. */
        if(put_to_window)
        {
            OSWCopyDrawablesCoord(
                w, pixmap,
                x, y,
                width, height,
                x, y
            );
        }
                
            
        return;
}       

/*
 *	Subject shields.
 */
void BWDP_SSHIELDS(
        int obj_num, xsw_object_struct *obj_ptr,
        font_t *font, pixel_t pix,
        pixmap_t pixmap, win_t w,
        int bkg_img_code,
        bool_t put_to_window
)
{
        int x = 6;   
        int y = 72;
        unsigned int width = 8;
        unsigned int height = 50;
        double c;

        shared_image_t *img_ptr;


        img_ptr = bridge_win.pan_s1_img;
        if(img_ptr == NULL)
            return;

        /* Draw up background. */
        OSWPutSharedImageToDrawableSect(
            img_ptr, pixmap,
            x, y,
            x, y,
            width, height
        );

        /* Draw bar. */
        if(obj_ptr != NULL)
        {
            if(obj_ptr->power_max != 0)
                c = MAX((double)obj_ptr->power / (double)obj_ptr->power_max, 0);
            else
                c = 0;

            OSWSetFgPix(pix);
            if(obj_ptr->shield_state >= SHIELD_STATE_NONE)
            {
                if(obj_ptr->shield_state == SHIELD_STATE_UP)
                {
                    if(c > BW_COEFF_WARNING)
                        OSWSetFgPix(pix);
                    else if(c > BW_COEFF_DANGER)
                        OSWSetFgPix(xsw_color.bp_warning);
                    else
                        OSWSetFgPix(xsw_color.bp_danger);
                    OSWDrawSolidRectangle(
                        pixmap,
                        x,
                        static_cast<int>(y + ((int)height / 2) -
                            ((((int)height - (2 * BW_BAR_MARGIN)) * c) / 2)),
                        6,
                        static_cast<unsigned int>(((int)height - (2 * BW_BAR_MARGIN)) * c)
                    );
                }
            }
        }

        /* Put to window. */
        if(put_to_window)
        {
            OSWCopyDrawablesCoord(
                w, pixmap,
                x, y,
                width, height,
                x, y
            );
        }


        return;
}

/*
 *	Subject visibility.
 */
void BWDP_SVIS(
        int obj_num, xsw_object_struct *obj_ptr,
        font_t *font, pixel_t pix,
        pixmap_t pixmap, win_t w,
        int bkg_img_code,
        bool_t put_to_window
)
{
        int x = 136;
        int y = 72;
        unsigned int width = 8;
        unsigned int height = 50;
        double c;

        shared_image_t *img_ptr;


        img_ptr = bridge_win.pan_s1_img;
        if(img_ptr == NULL)
            return;

        /* Draw up background. */
        OSWPutSharedImageToDrawableSect(
            img_ptr, pixmap,
            x, y,
            x, y,
            width, height
        );

        /* Draw bar. */
        if(obj_ptr != NULL)
        {
            c = MAX(DBGetObjectVisibility(obj_num), 0);

            OSWSetFgPix(pix);
            OSWDrawSolidRectangle(
                pixmap,
                x + 1,   
                static_cast<int>(y + BW_BAR_MARGIN +
                    (((int)height - (2 * BW_BAR_MARGIN)) *
                     (double)(1 - c)
                    ))
                ,  
                6,
                static_cast<unsigned int>(((int)height - (2 * BW_BAR_MARGIN)) * c)
            );   
        }

        /* Put to window. */
        if(put_to_window)
        {
            OSWCopyDrawablesCoord(
                w, pixmap,
                x, y,
                width, height,   
                x, y
            );
        }


        return;
}

/*
 *	Subject bearing.
 */
void BWDP_SBEARING(
        int obj_num, xsw_object_struct *obj_ptr,
        font_t *font, pixel_t pix,
        pixmap_t pixmap, win_t w,
        int bkg_img_code,
        bool_t put_to_window
)
{
        int x = ((BW_CONSOLE_PANEL_WIDTH / 2) - 50);
        int y = ((BW_CONSOLE_PANEL_HEIGHT / 2) - 50);
        unsigned int width = (50 * 2);
        unsigned int height = (50 * 2);

	char in_same_sector;
	double dx1, dy1;
        double dx2, dy2;
	double bearing, d;

        char text[80];
        shared_image_t *img_ptr;

	int ref_obj_num;	/* Relative to obj_num. */
	xsw_object_struct *ref_obj_ptr;


	/* Get referance object. */
	ref_obj_num = net_parms.player_obj_num;


        img_ptr = bridge_win.pan_s1_img;
        if(img_ptr == NULL)
            return;
        
        /* Draw up background. */
        OSWPutSharedImageToDrawableSect(
            img_ptr, pixmap,
            x, y,
            x, y,
            width, height
        );


        /* Draw bar. */
        if(obj_ptr != NULL)
        {
	    if(!DBIsObjectGarbage(ref_obj_num))
	    {
		ref_obj_ptr = xsw_object[ref_obj_num];

		/* Check if objects are in same sector. */
		if((obj_ptr->sect_x == ref_obj_ptr->sect_x) &&
                   (obj_ptr->sect_y == ref_obj_ptr->sect_y) &&
                   (obj_ptr->sect_z == ref_obj_ptr->sect_z)
                )
		    in_same_sector = 1;
		else
		    in_same_sector = 0;

		/* Get bearing. */
		if(in_same_sector)
		    bearing = MuCoordinateDeltaVector(
			obj_ptr->x - ref_obj_ptr->x,
			obj_ptr->y - ref_obj_ptr->y
		    );
		else
                    bearing = MuCoordinateDeltaVector(
                        obj_ptr->sect_x - ref_obj_ptr->sect_x,
                        obj_ptr->sect_y - ref_obj_ptr->sect_y 
                    );

		/* Adjust bearing if scanner is oriented to object. */
		if(bridge_win.scanner_orient == SCANNER_ORIENT_LOCAL)
		{
		    bearing = SANITIZERADIANS(
			bearing - ref_obj_ptr->heading
		    );
		}

		dx1 = MuPolarRotX(bearing, 45);
                dy1 = MuPolarRotY(bearing, 45);
                dx2 = MuPolarRotX(bearing, 50);
                dy2 = MuPolarRotY(bearing, 50);

		/* Draw bearing tick. */
		OSWSetFgPix(pix);
		OSWDrawLine(
		    pixmap,
		    static_cast<int>(x + (width / 2) + dx1),
		    static_cast<int>(y + (height / 2) - dy1),
		    static_cast<int>(x + (width / 2) + dx2),
                    static_cast<int>(y + (height / 2) - dy2)
		);


		/* Draw distance label. */
		if(in_same_sector)
		{
                    d = Mu3DDistance(
                        obj_ptr->x - ref_obj_ptr->x,
                        obj_ptr->y - ref_obj_ptr->y,
                        obj_ptr->z - ref_obj_ptr->z
                    );

                    switch(option.units)
                    {
                      case XSW_UNITS_ENGLISH:
                        d = ConvertRUToAU(&sw_units, d);
                        sprintf(text, "%.3f au", d);
                        break;

                      case XSW_UNITS_METRIC:
                        d = ConvertRUToAU(&sw_units, d);
                        sprintf(text, "%.3f au", d);
                        break;

                      default:
                        sprintf(text, "%.2f ru", d);
                        break;
                    }

                    OSWSetFont(font);
                    OSWDrawString(
                        pixmap,
                        x + 26,
			(int)(y + ((int)height * 0.86)),
/*                        static_cast<int>(y + (height * 0.86)), */
                        text
                    );
		}
	    }
	}


        /* Put to window. */
        if(put_to_window)
        {
            OSWCopyDrawablesCoord(
                w, pixmap,
                x, y,
                width, height,
                x, y
            );
        }


        return;
}

/*
 *	Subject distance.
 */
void BWDP_SDISTANCE(
        int obj_num, xsw_object_struct *obj_ptr,
        font_t *font, pixel_t pix,
        pixmap_t pixmap, win_t w,
        int bkg_img_code,
        bool_t put_to_window
) 
{
        int x = 40;
        int y = 123;
        unsigned int width = 70;
        unsigned int height = 7;
	double c, d;

        shared_image_t *img_ptr;

	int ref_obj_num;	/* Relative to obj_num. */
        xsw_object_struct *ref_obj_ptr;


        /* Get referance object. */
        ref_obj_num = net_parms.player_obj_num;


        img_ptr = bridge_win.pan_s1_img;
        if(img_ptr == NULL)
            return;

        /* Draw up background. */
        OSWPutSharedImageToDrawableSect(
            img_ptr, pixmap,
            x, y,
            x, y,
            width, height   
        );


        /* Draw bar. */
        if(obj_ptr != NULL)
        {
            if(!DBIsObjectGarbage(ref_obj_num))
            {
                ref_obj_ptr = xsw_object[ref_obj_num];
        
                /* Get distance d. */
                if((obj_ptr->sect_x == ref_obj_ptr->sect_x) &&
                   (obj_ptr->sect_y == ref_obj_ptr->sect_y) &&
                   (obj_ptr->sect_z == ref_obj_ptr->sect_z)
                )
		{
		    d = Mu3DDistance(
			obj_ptr->x - ref_obj_ptr->x,
			obj_ptr->y - ref_obj_ptr->y,
			obj_ptr->z - ref_obj_ptr->z
		    );
		    c = MIN(d / ref_obj_ptr->scanner_range, 1);
		    if(c < 0)
			c = 0;

                    OSWSetFgPix(pix);
                    OSWDrawLine(
                        pixmap, 
                        static_cast<int>(x + (c * (double)width)),
                        y,
                        static_cast<int>(x + (c * (double)width)),
                        y + 5
                    );
		}
	    }
	}

        /* Put to window. */
        if(put_to_window)
        {
            OSWCopyDrawablesCoord(   
                w, pixmap,
                x, y, 
                width, height,
                x, y
            );
        }


	return;
}


/*
 *	Subject damage control.
 */
void BWDP_SDMGCTL(
        int obj_num, xsw_object_struct *obj_ptr,
        font_t *font, pixel_t pix,
        pixmap_t pixmap, win_t w,
        int bkg_img_code,
        bool_t put_to_window
)
{
        int x = 30;
        int y = 122;
        unsigned int width = 42;
        unsigned int height = 8;
        double c;

        shared_image_t *img_ptr;   


        img_ptr = bridge_win.pan_s1_img;
        if(img_ptr == NULL)
            return;

        /* Draw up background. */
        OSWPutSharedImageToDrawableSect(
            img_ptr, pixmap,
            x, y,
            x, y,
            width, height
        );

        /* Draw bar. */
        if(obj_ptr != NULL)
        {
            if(obj_ptr->damage_control == DMGCTL_STATE_ON)
            {
                if(obj_ptr->power_max != 0)
                    c = (double)obj_ptr->power /
                        (double)obj_ptr->power_max;
                else
                    c = 1;  
 
                OSWSetFgPix(pix);
                OSWDrawSolidRectangle(
                    pixmap,
                    x + BW_BAR_MARGIN,
                    y + 2,
                    static_cast<unsigned int>(((int)width - (2 * BW_BAR_MARGIN)) * c),
                    6
                );
            }
        }
        
        /* Put to window. */
        if(put_to_window)
        {       
            OSWCopyDrawablesCoord(
                w, pixmap,
                x, y,
                width, height,
                x, y
            );
        }

        return;
}



/*
 *	Player intercepting object name.
 */
void BWDP_PINAME(
        int obj_num, xsw_object_struct *obj_ptr,
        font_t *font, pixel_t pix,
        pixmap_t pixmap, win_t w,
        int bkg_img_code,   
        bool_t put_to_window
)
{
        int x = 3;
        int y = 0;
        unsigned int width = 144;
        unsigned int height = 12;

	int i_obj_num;
        char *strptr;
        char text[80 + XSW_OBJ_NAME_MAX];

        image_t *img_ptr;


        if(IMGIsImageNumLoaded(bkg_img_code))
            img_ptr = xsw_image[bkg_img_code]->image;
        else
            return;

        /* Draw up background. */
        OSWPutImageToDrawableSect(
            img_ptr, pixmap,
            x, y,
            x, y,
            width, height
        );

        /* Draw texts. */
        if(obj_ptr != NULL)
        {
            i_obj_num = obj_ptr->intercepting_object;
            if(!DBIsObjectGarbage(i_obj_num))
            {
                OSWSetFont(font);
                OSWSetFgPix(pix);

                switch(option.show_formal_label)
                {
                  /* Always. */  
                  case 2:
                    strptr = DBGetFormalNameStr(i_obj_num);
                    sprintf(text, "I: %s",
                        (strptr == NULL) ? "" : strptr
                    );
                    break;

                  /* As needed. */
                  case 1:
                    if(obj_num == xsw_object[i_obj_num]->owner)
                    {
                        strptr = DBGetFormalNameStr(i_obj_num);
                        sprintf(text, "I: %s",
                            (strptr == NULL) ? "" : strptr
                        );
                    }
                    else
                    {
                        sprintf(text, "I: %s",
                            xsw_object[i_obj_num]->name
                        );
                    }
                    break;

                  /* Never. */
                  default:
                    sprintf(text, "I: %s",
                        xsw_object[i_obj_num]->name
                    );
                    break;
                }    

                OSWDrawString(
                    pixmap,
                    x + 3,
                    y + 7,
                    text
                );  
            }
        }

        /* Put to window. */
        if(put_to_window)
        {
            OSWCopyDrawablesCoord(
                w, pixmap,
                x, y,
                width, height,
                x, y 
            );
        }

        return;
}


/*
 *	Player lock on name.
 */
void BWDP_PWLOCK(
        int obj_num, xsw_object_struct *obj_ptr,
        font_t *font, pixel_t pix,
        pixmap_t pixmap, win_t w,
        int bkg_img_code,
        bool_t put_to_window
)
{
        int x = 3;
        int y = 14;
        unsigned int width = 144;
        unsigned int height = 12;

	int locked_on;
	char *strptr;
        char text[80 + XSW_OBJ_NAME_MAX];

        image_t *img_ptr;


        if(IMGIsImageNumLoaded(bkg_img_code))
            img_ptr = xsw_image[bkg_img_code]->image;
        else
            return;

        /* Draw up background. */
        OSWPutImageToDrawableSect(
            img_ptr, pixmap,
            x, y,
            x, y,
            width, height
        );

        /* Draw texts. */
        if(obj_ptr != NULL) 
        {
	    locked_on = obj_ptr->locked_on;
	    if(!DBIsObjectGarbage(locked_on))
	    {
                OSWSetFont(font);
                OSWSetFgPix(pix);

                switch(option.show_formal_label)
                {
                  /* Always. */
                  case 2:
                    strptr = DBGetFormalNameStr(locked_on);
		    sprintf(text, "WL: %s",
			(strptr == NULL) ? "" : strptr
		    );
                    break;

                  /* As needed. */
                  case 1:   
                    if(obj_num == xsw_object[locked_on]->owner)
                    {
                        strptr = DBGetFormalNameStr(locked_on);
                        sprintf(text, "WL: %s",
                            (strptr == NULL) ? "" : strptr
                        );
                    }
                    else
                    {
                        sprintf(text, "WL: %s",
			    xsw_object[locked_on]->name
                        );
                    }
                    break;

                  /* Never. */
                  default:
                    sprintf(text, "WL: %s",
                        xsw_object[locked_on]->name
                    );
                    break;
		}

                OSWDrawString(
                    pixmap,
                    x + 3,
                    y + 7,
                    text
                );
	    }
	}

        /* Put to window. */
        if(put_to_window)   
        {
            OSWCopyDrawablesCoord(
                w, pixmap,
                x, y,
                width, height,   
                x, y
            );
        }

        return;
}


/*
 *	Player weapons.
 */
void BWDP_PWEAPONS(
        int obj_num, xsw_object_struct *obj_ptr,
        font_t *font, pixel_t pix,
        pixmap_t pixmap, win_t w,
        int bkg_img_code,
        bool_t put_to_window
)
{
        int x = 3;
        int y = 28;
        unsigned int width = 144;
        unsigned int height = 100;

	int i, x_pos, y_pos;
	char text[80 + XSW_OBJ_NAME_MAX];

        image_t *img_ptr;
	xsw_weapons_struct **ptr, *wep_ptr;


        if(IMGIsImageNumLoaded(bkg_img_code))
            img_ptr = xsw_image[bkg_img_code]->image;
        else
            return;

        /* Draw up background. */
        OSWPutImageToDrawableSect(
            img_ptr, pixmap,
            x, y,
            x, y,
            width, height
        );


        /* Note: foreground color is not set by value of pix. */





        /* Draw weapons listing. */
        if(obj_ptr != NULL)
	{
            OSWSetFont(font);

	    /* Draw weapons yield bar. */
	    if(local_control.weapons_online)
	    {
		const unsigned int	yield_bar_width = 80,
					yield_bar_height = 6;

		if(local_control.weapon_yield > 0.35)
		    OSWSetFgPix(xsw_color.bp_danger);
		else
		    OSWSetFgPix(xsw_color.bp_warning);

		x_pos = static_cast<int>(local_control.weapon_yield *
		    (double)yield_bar_width);
		OSWDrawSolidRectangle(
		    pixmap,
		    x + 30,
		    y + (14 / 2) - ((int)yield_bar_height / 2),
		    x_pos, 6
		);

                OSWSetFgPix(xsw_color.bp_standard_text);
                OSWDrawString(
                    pixmap,
                    x + 3,
                    y + 9,
                    "Yld:"
                );
		sprintf(text, "%.0f%%", local_control.weapon_yield * 100.0);
                OSWDrawString(
                    pixmap,
                    x + 30 + (int)yield_bar_width + 3,
                    y + 9,
                    text
                );  

	    }
	    else
	    {
		OSWSetFgPix(xsw_color.bp_withdrawn_text);
	    }

	    /* Set starting y position for drawing weapons listing. */
	    y_pos = y + 14;

	    for(i = 0, ptr = obj_ptr->weapons;
                i < obj_ptr->total_weapons;
                i++, ptr++
	    )
	    {
		wep_ptr = *ptr;
		if(wep_ptr == NULL)
		    continue;

		switch(wep_ptr->emission_type)
		{
		  case WEPEMISSION_PULSE:
		    sprintf(text, "%s: %.0f%%",
			wep_ptr->name,
                        (obj_ptr->power_max == 0) ?
                            0 :
                            obj_ptr->power / obj_ptr->power_max *
                            100
		    );
		    break;

		  case WEPEMISSION_PROJECTILE:
                    sprintf(text, "%s: %d(%d)",
                        wep_ptr->name,
			wep_ptr->amount,
			wep_ptr->max
                    );
		    break;

		  default:	/* WEPEMISSION_STREAM */
                    sprintf(text, "%s: %.0f%%",
                        wep_ptr->name,
                        (obj_ptr->power_max == 0) ?
                            0 :
                            obj_ptr->power / obj_ptr->power_max *
                            100
                    );
		    break;
		}
                if(local_control.weapons_online)
                {
                    if(obj_ptr->selected_weapon == i)
                        OSWSetFgPix(xsw_color.bp_bold_text);
                    else
                        OSWSetFgPix(xsw_color.bp_standard_text);
                }
                OSWDrawString(
                    pixmap,
                    x + 3,
                    y_pos + 9,
                    text
                );
                y_pos += 14;
	    }
	}


        /* Put to window. */
        if(put_to_window)
        {
            OSWCopyDrawablesCoord(   
                w, pixmap,
                x, y,
                width, height,
                x, y
            );
        }
            
        
        return;
}





/*
 *	Subject empire.
 */
void BWDP_SEMPIRE(
        int obj_num, xsw_object_struct *obj_ptr,
        font_t *font, pixel_t pix,
        pixmap_t pixmap, win_t w,
        int bkg_img_code,
        bool_t put_to_window
)
{
        int x = 14;    
        int y = 3;
        unsigned int width = 122;
        unsigned int height = 10;

	image_t *img_ptr;   

        int i, x2;
        char text[BW_LABEL_MAX];


        if(IMGIsImageNumLoaded(bkg_img_code))
            img_ptr = xsw_image[bkg_img_code]->image;
        else
            return; 
              
        /* Draw up background. */
        OSWPutImageToDrawableSect(
            img_ptr, pixmap,
            x, y,
            x, y,
            width, height
        );

        /* Draw empire text. */
        if((obj_ptr != NULL) && (net_parms.player_obj_ptr != NULL))
        {
            /* Set pixel color. */
	    switch(MatchIFFPtr(obj_ptr, net_parms.player_obj_ptr))
	    {
              case IFF_FRIENDLY:
		pix = xsw_color.bp_friendly;
		break;

	      case IFF_HOSTILE:
		pix = xsw_color.bp_hostile;
		break;

	      default:	/* IFF_UNKNOWN */
		pix = xsw_color.bp_unknown;
		break;
	    }

            /* Empire string empty? */
            if(*obj_ptr->empire != '\0')
            {
                /* Format empire string. */
                strncpy(text, "(", BW_LABEL_MAX);
                i = strlen(text);
                strncat(
                    text,
                    obj_ptr->empire,
                    MAX((BW_LABEL_MAX - 1) - i, 0)
                );
                i = strlen(text);
                strncat(   
                    text,
                    ")",  
                    MAX((BW_LABEL_MAX - 1) - i, 0)
                );
                text[BW_LABEL_MAX - 1] = '\0';
                i = strlen(text);
                
                /* Calculate left position of empire string,
                 * assume width of character is 6 pixels.
                 */
                x2 = ((int)width / 2) - (i * 6 / 2) + x;

                /* Draw empire string. */
                OSWSetFont(font);
                OSWSetFgPix(pix);
                OSWDrawString(
                    pixmap,
                    x2,
                    y + 8,
                    text
                );
            }
        }

        /* Put to window. */
        if(put_to_window)
        {
            OSWCopyDrawablesCoord(
                w, pixmap,
                x, y,
                width, height,
                x, y
            );
        }

        return;
}

/*
 *	Subject weapons.
 */
void BWDP_SWEAPONS(
        int obj_num, xsw_object_struct *obj_ptr,
        font_t *font, pixel_t pix,
        pixmap_t pixmap, win_t w,
        int bkg_img_code,
        bool_t put_to_window
)
{
        int x = 3;
        int y = 48;	/* Leaving room for stuff at the top. */
        unsigned int width = 144;
        unsigned int height = 102;
        
        int i, n, y_pos;       
        char text[80 + XSW_OBJ_NAME_MAX];
            
        image_t *img_ptr; 
        xsw_weapons_struct **ptr, *wep_ptr;


        if(IMGIsImageNumLoaded(bkg_img_code))
            img_ptr = xsw_image[bkg_img_code]->image;
        else
            return;

        /* Draw up background. */
        OSWPutImageToDrawableSect(
            img_ptr, pixmap,
            x, y,
            x, y,
            width, height
        );


	/* Note: foreground color is not set by value of pix. */


        /* Draw texts. */
        if(obj_ptr != NULL)
        {
            OSWSetFont(font);

            y_pos = 0;
            for(i = 0, ptr = obj_ptr->weapons;
                i < obj_ptr->total_weapons;
                i++, ptr++
            )
            {
		wep_ptr = *ptr;
                if(wep_ptr == NULL)
                    continue;

                switch(wep_ptr->emission_type)
                {
                  case WEPEMISSION_PULSE:
                    n = OCSGetByCode(wep_ptr->ocs_code);
                    if(n < 0)
                        break;
                    sprintf(text, "%s: %.0f%%",
                        ocsn[n]->name,
                        (obj_ptr->power_max == 0) ?
                            0 :
                            obj_ptr->power / obj_ptr->power_max *
                            100
                    );   
                    if(obj_ptr->selected_weapon == i)
                        OSWSetFgPix(xsw_color.bp_bold_text);
                    else
                        OSWSetFgPix(xsw_color.bp_standard_text);
                    OSWDrawString(
                        pixmap,
                        x + 3,
                        y + y_pos + 7,
                        text
                    );
                    y_pos += 14;
                    break;

                  case WEPEMISSION_PROJECTILE:
                    n = OCSGetByCode(wep_ptr->ocs_code);
                    if(n < 0)
                        break;
                    sprintf(text, "%s: %d(%d)",
                        ocsn[n]->name,
                        wep_ptr->amount,
                        wep_ptr->max
                    );
                    if(obj_ptr->selected_weapon == i)
                        OSWSetFgPix(xsw_color.bp_bold_text);
                    else   
                        OSWSetFgPix(xsw_color.bp_standard_text);
                    OSWDrawString(
                        pixmap,
                        x + 3,
                        y + y_pos + 7,
                        text
                    );
                    y_pos += 14;
                    break;

                  default:      /* WEPEMISSION_STREAM */
                    n = OCSGetByCode(wep_ptr->ocs_code);
                    if(n < 0)
                        break;
                    sprintf(text, "%s: %.0f%%",
                        ocsn[n]->name,
                        (obj_ptr->power_max == 0) ?
                            0 :
                            obj_ptr->power / obj_ptr->power_max *
                            100
                    );
                    if(obj_ptr->selected_weapon == i)
                        OSWSetFgPix(xsw_color.bp_bold_text);
                    else
                        OSWSetFgPix(xsw_color.bp_standard_text);
                    OSWDrawString(
                        pixmap,
                        x + 3,
                        y + y_pos + 7,
                        text
                    );
                    y_pos += 14;
                    break;
                }
            }
        }

        /* Put to window. */
        if(put_to_window)
        {
            OSWCopyDrawablesCoord(
                w, pixmap,   
                x, y,
                width, height,
                x, y
            );
        }
}



/*
 *	Player throttle.
 */
void BWDP_PTHROTTLEL(
        int obj_num, xsw_object_struct *obj_ptr,
        font_t *font, pixel_t pix,
        pixmap_t pixmap, win_t w,
        int bkg_img_code,
        bool_t put_to_window
)
{
        int x = 6;
        int y = 35;
        unsigned int width = 10;
        unsigned int height = 90;
	double c;

        shared_image_t *img_ptr;
             
                
        img_ptr = bridge_win.pan_p3_img;
        if(img_ptr == NULL)
            return; 

        /* Draw up background. */
        OSWPutSharedImageToDrawableSect(
            img_ptr, pixmap,
            x, y,
            x, y,
            width, height
        );

        /* Draw bar. */
        if(obj_ptr != NULL) 
        {
	    if((obj_ptr->thrust_dir >= (PI * 0.4)) &&
               (obj_ptr->thrust_dir <= (PI * 1.6))
	    )
	        OSWSetFgPix(pix);
	    else
                OSWSetFgPix(xsw_color.bpol_throttle_rev);

	    c = obj_ptr->throttle;

            OSWDrawSolidRectangle(
                pixmap,
                x,
                (unsigned int)(y + BW_BAR_MARGIN +
                    (((int)height - (2 * BW_BAR_MARGIN)) *
                     (double)(1.0 - c)
                    )),
                8,
                (unsigned int)(((int)height - (2 * BW_BAR_MARGIN)) * c)
            );
	}

        /* Put to window. */
        if(put_to_window)
        {
            OSWCopyDrawablesCoord(
                w, pixmap,
                x, y,   
                width, height,
                x, y  
            );
        }
}

void BWDP_PTHROTTLER(
        int obj_num, xsw_object_struct *obj_ptr,
        font_t *font, pixel_t pix,
        pixmap_t pixmap, win_t w, 
        int bkg_img_code,
        bool_t put_to_window
)
{
        int x = 134;
        int y = 35;
        unsigned int width = 10;
        unsigned int height = 90;
	double c;

        shared_image_t *img_ptr;


        img_ptr = bridge_win.pan_p3_img;
        if(img_ptr == NULL)
            return;

        /* Draw up background. */
        OSWPutSharedImageToDrawableSect(
            img_ptr, pixmap,
            x, y,
            x, y,
            width, height
        );

        /* Draw bar. */ 
        if(obj_ptr != NULL)
        {
            if((obj_ptr->thrust_dir >= (PI * 0.4)) &&
               (obj_ptr->thrust_dir <= (PI * 1.6))  
            )   
                OSWSetFgPix(pix);
            else
                OSWSetFgPix(xsw_color.bpol_throttle_rev); 

            c = obj_ptr->throttle;

            OSWDrawSolidRectangle(
                pixmap,
                x + 2,
                (int)(y + BW_BAR_MARGIN +
                    (((int)height - (2 * BW_BAR_MARGIN)) *
                     (double)(1 - c)
                    ))
                ,
                8,
                (unsigned int)(((int)height - (2 * BW_BAR_MARGIN)) * c)
            );
        }

        /* Put to window. */
        if(put_to_window)
        {
            OSWCopyDrawablesCoord(
                w, pixmap,
                x, y,
                width, height,
                x, y
            );
        }


        return;
}

void BWDP_PTHROTTLE(
        int obj_num, xsw_object_struct *obj_ptr,
        font_t *font, pixel_t pix,
        pixmap_t pixmap, win_t w, 
        int bkg_img_code,
        bool_t put_to_window
)
{
	BWDP_PTHROTTLEL(
	    obj_num, obj_ptr,
	    font, pix,
	    pixmap, w,
	    bkg_img_code,
	    put_to_window
	);

	BWDP_PTHROTTLER(
            obj_num, obj_ptr,
            font, pix,
            pixmap, w,
            bkg_img_code,
            put_to_window
        );

	return;
}


/*
 *	Player velocity.
 */
void BWDP_PVELOCITY(
        int obj_num, xsw_object_struct *obj_ptr,
        font_t *font, pixel_t pix,
        pixmap_t pixmap, win_t w,
        int bkg_img_code,
        bool_t put_to_window
)
{
        int x = 6;
        int y = 0;
        unsigned int width = 138;
        unsigned int height = 10;
        double c, s;

	char text[80];
        shared_image_t *img_ptr;


        img_ptr = bridge_win.pan_p3_img;
        if(img_ptr == NULL)
            return;

        /* Draw up background. */
        OSWPutSharedImageToDrawableSect(
            img_ptr, pixmap,
            x, y,
            x, y,
            width, height
        );

	if(obj_ptr != NULL)
	{
	    if(obj_ptr->velocity_max != 0)
	        c = obj_ptr->velocity / obj_ptr->velocity_max;
	    else
		c = 0;

	    /* Tick. */
	    OSWSetFgPix(pix);
	    OSWDrawLine(
		pixmap,
		(int)(x + (width * c)),
		y + 2,
		(int)(x + (width * c)),
                y + 7
	    );

	    /* Label. */
	    switch(option.units)
	    {
	      case XSW_UNITS_ENGLISH:
                s = ConvertVelocityRUPCToAUPS(&sw_units, obj_ptr->velocity);
		if(s < 10)
                    sprintf(text, "%.3f au/s", s);
		else
		    sprintf(text, "%.1f au/s", s);
		break;

	      case XSW_UNITS_METRIC:
                s = ConvertVelocityRUPCToAUPS(&sw_units, obj_ptr->velocity);
                if(s < 10)
                    sprintf(text, "%.3f au/s", s);
                else
                    sprintf(text, "%.1f au/s", s);
		break;

	      default:
                s = ConvertVelocityRUPCToRUPS(&sw_units, obj_ptr->velocity);
                if(s < 10)
                    sprintf(text, "%.3f ru/s", s);
                else 
                    sprintf(text, "%.1f ru/s", s);
		break;
	    }
	    OSWSetFont(font);
	    OSWDrawString(
		pixmap,
		(c > 0.45) ? (x + 6) : (x + (width / 2) - 6),
		y + 7,
		text
	    );
	}


        /* Put to window. */
        if(put_to_window)
        {
            OSWCopyDrawablesCoord(
                w, pixmap,
                x, y,
                width, height,
                x, y
            );
        }
}


/*
 *	Player antimatter (just the bar).
 */
void BWDP_PANTIMATTER(
        int obj_num, xsw_object_struct *obj_ptr,
        font_t *font, pixel_t pix,
        pixmap_t pixmap, win_t w,
        int bkg_img_code,
        bool_t put_to_window
)
{
        int x = 6;
        int y = 20;
        unsigned int width = 138;
        unsigned int height = 8;
        double c;

        shared_image_t *img_ptr;
  
 
        img_ptr = bridge_win.pan_p3_img;
        if(img_ptr == NULL)
            return;

        /* Draw up background. */
        OSWPutSharedImageToDrawableSect(
            img_ptr, pixmap,
            x, y,
            x, y,
            width, height
        );

	/* Draw bar. */
	if(obj_ptr != NULL)
	{
            if(obj_ptr->antimatter_max > 0)
                c = (double)obj_ptr->antimatter /
                    (double)obj_ptr->antimatter_max;
            else
                c = 0;
 
            if(c > BW_COEFF_WARNING)
                OSWSetFgPix(pix);
            else if(c > BW_COEFF_DANGER)
                OSWSetFgPix(xsw_color.bp_warning);
            else   
                OSWSetFgPix(xsw_color.bp_danger);

            OSWDrawSolidRectangle(
                pixmap,
                x + BW_BAR_MARGIN,
                y,
                static_cast<unsigned int>(((int)width - (2 * BW_BAR_MARGIN)) * c),
                6
            );
        }

        /* Put to window. */
        if(put_to_window)
        {
            OSWCopyDrawablesCoord(
                w, pixmap,
                x, y,
                width, height,
                x, y
            );
        }


        return;
}


/*
 *	Player engine state.
 */
void BWDP_PENGINESTATE(
        int obj_num, xsw_object_struct *obj_ptr,
        font_t *font, pixel_t pix,
        pixmap_t pixmap, win_t w,
        int bkg_img_code,
        bool_t put_to_window
)
{
        int x = 75;
        int y = 92;
        unsigned int width = 55;
        unsigned int height = 30;

        shared_image_t *img_ptr;
	char *strptr;


        img_ptr = bridge_win.pan_p3_img;
        if(img_ptr == NULL)
            return;

        /* Draw up background. */
        OSWPutSharedImageToDrawableSect(
            img_ptr, pixmap,
            x, y,
            x, y,
            width, height
        );

	if(obj_ptr != NULL)
	{
	    OSWSetFont(font);
	    OSWSetFgPix(xsw_color.bp_normal_outline);

	    OSWDrawString(
		pixmap,
		x + 4,
		y + 7,
		"Eng:"
	    );

	    switch(obj_ptr->engine_state)
	    {
	      case ENGINE_STATE_ON:
		strptr = "on";
		OSWSetFgPix(xsw_color.bp_light_outline);
		break;

	      case ENGINE_STATE_STARTING:
                strptr = "starting";
                OSWSetFgPix(xsw_color.bp_normal_outline);
                break;

	      default:	/* ENGINE_STATE_OFF and ENGINE_STATE_NONE */
                strptr = "off";
                OSWSetFgPix(xsw_color.bp_dark_outline);
		break;
	    }

            OSWDrawString(
                pixmap,
                x + (width / 2) - (strlen(strptr) * 6 / 2),
                y + 20,  
                strptr
            );

            OSWSetFgPix(xsw_color.bp_normal_outline);
            OSWDrawString(
                pixmap,
                x + (width / 2) - (strlen(strptr) * 6 / 2) - 8,
                y + 20,
                "("
            );
            OSWDrawString(
                pixmap,
                x + (width / 2) + (strlen(strptr) * 6 / 2) + 2,
                y + 20,
                ")"
            );
	}

        /* Put to window. */
        if(put_to_window)
        {
            OSWCopyDrawablesCoord(
                w, pixmap,
                x, y,
                width, height,
                x, y
            );
        }

        return;
}


/*
 *	Player heading and position.
 */
void BWDP_PHEADING(
        int obj_num, xsw_object_struct *obj_ptr,
        font_t *font, pixel_t pix,
        pixmap_t pixmap, win_t w,
        int bkg_img_code,
        bool_t put_to_window
)
{
        int x = 18;
        int y = 32;
        unsigned int width = 114;
        unsigned int height = 60;

        shared_image_t *img_ptr;

	char text[256];


        img_ptr = bridge_win.pan_p3_img;
        if(img_ptr == NULL)
            return;

        /* Draw up background. */
        OSWPutSharedImageToDrawableSect(
            img_ptr, pixmap,
            x, y,
            x, y,
            width, height
        );


	/* Draw text. */
	if(obj_ptr != NULL)
	{
	    OSWSetFont(font);
	    OSWSetFgPix(pix);

            sprintf(text, "AM: %.2f",
                obj_ptr->antimatter
            );
            OSWDrawString(
                pixmap,
                x + 3,
                y + 0 + 7,
                text
            );

	    sprintf(text, "HD: %.2f'",
		RADTODEG(obj_ptr->heading)
	    );
	    OSWDrawString(
		pixmap,
		x + 3,
                y + 12 + 7,
		text
	    );

            sprintf(text, "Sect: %ld %ld %ld",
		obj_ptr->sect_x, obj_ptr->sect_y, obj_ptr->sect_z
	    );
            OSWDrawString(
                pixmap,
                x + 3,   
                y + 24 + 7, 
                text
            );

            sprintf(text, "X: %.2f",
                obj_ptr->x
            );
            OSWDrawString(
                pixmap,
                x + 3,
                y + 36 + 7,
                text
            );

            sprintf(text, "Y: %.2f", 
                obj_ptr->y
            );
            OSWDrawString(
                pixmap,
                x + 3,
                y + 48 + 7,
                text
            );
	}


        /* Put to window. */
        if(put_to_window)
        {
            OSWCopyDrawablesCoord(
                w, pixmap,
                x, y,
                width, height,
                x, y
            );
        }
}


/*
 *	Player thrust vector.
 */
void BWDP_PTHRUSTVECTOR(
        int obj_num, xsw_object_struct *obj_ptr,
        font_t *font, pixel_t pix,
        pixmap_t pixmap, win_t w,
        int bkg_img_code,
        bool_t put_to_window
)
{
        int x = 40;
        int y = 92;
        unsigned int width = 30;
        unsigned int height = 30;

	double dx, dy, dir;

        shared_image_t *img_ptr;


        img_ptr = bridge_win.pan_p3_img;
        if(img_ptr == NULL)
            return;

        /* Draw up background. */
        OSWPutSharedImageToDrawableSect(
            img_ptr, pixmap,
            x, y,
            x, y,
            width, height
        );


	if(obj_ptr != NULL)
	{
	    dir = obj_ptr->thrust_dir;

	    dx = cos((PI / 2) - dir) * 7;
	    dy = sin((PI / 2) - dir) * -7;

	    if((dx < 1) && (dx > -1))
		dx = 0;

	    OSWSetFgPix(pix);
	    OSWDrawLine(
		pixmap,
		x + (width / 2),
		y + (height / 2),
		(int)(x + (width / 2) + dx),
		(int)(y + (height / 2) + dy)
	    );

	    /* External dampers on? */
	    if(gctl[0].external_dampers)
	    {
		OSWDrawArc(
		    pixmap,
		    x,
		    (int)(y - ((int)height * 0.2)),
		    (int)width - 1,
		    (unsigned int)((int)height * 0.4),
		    PI,
		    (2 * PI)
                );
                OSWDrawArc(
                    pixmap,
                    x,
		    (int)(y + (int)height - ((int)height * 0.15)),
                    (int)width - 1,
		    (unsigned int)((int)height * 0.3),
                    PI, (2 * PI)
                );
	    }
	}


        /* Put to window. */
        if(put_to_window)
        {
            OSWCopyDrawablesCoord(
                w, pixmap,
                x, y,
                width, height,
                x, y
            );
        }

        return;
} 



/*
 *	Subject antimatter (just the bar).
 */
void BWDP_SANTIMATTER(
        int obj_num, xsw_object_struct *obj_ptr,   
        font_t *font, pixel_t pix,
        pixmap_t pixmap, win_t w,
        int bkg_img_code,
        bool_t put_to_window 
)
{
        int x = 6;
        int y = 20;
        unsigned int width = 138;
        unsigned int height = 8;
        double c;

        shared_image_t *img_ptr;


        img_ptr = bridge_win.pan_s3_img;
        if(img_ptr == NULL)
            return;

        /* Draw up background. */
        OSWPutSharedImageToDrawableSect(
            img_ptr, pixmap,
            x, y,
            x, y,
            width, height
        );

        /* Draw bar. */
        if(obj_ptr != NULL)
        {
            if(obj_ptr->antimatter_max > 0)
                c = (double)obj_ptr->antimatter /
                    (double)obj_ptr->antimatter_max;
            else
                c = 0;

            if(c > BW_COEFF_WARNING)
                OSWSetFgPix(pix);   
            else if(c > BW_COEFF_DANGER)
                OSWSetFgPix(xsw_color.bp_warning);
            else
                OSWSetFgPix(xsw_color.bp_danger);

            OSWDrawSolidRectangle(
                pixmap,
                x + BW_BAR_MARGIN,
                y, 
                static_cast<unsigned int>(((int)width - (2 * BW_BAR_MARGIN)) * c),
                6
            );   
        }       

        /* Put to window. */
        if(put_to_window)
        {
            OSWCopyDrawablesCoord(
                w, pixmap,
                x, y,
                width, height,   
                x, y
            );
        }


        return;
}

/*
 *      Subject heading and location.
 */
void BWDP_SHEADING(
        int obj_num, xsw_object_struct *obj_ptr,
        font_t *font, pixel_t pix,
        pixmap_t pixmap, win_t w,
        int bkg_img_code,
        bool_t put_to_window
)
{
        int x = 18;
        int y = 32;   
        unsigned int width = 114;
        unsigned int height = 60;

        shared_image_t *img_ptr;
        char text[256];


        img_ptr = bridge_win.pan_s3_img;
        if(img_ptr == NULL)
            return;

        /* Draw up background. */
        OSWPutSharedImageToDrawableSect(
            img_ptr, pixmap,
            x, y,
            x, y,
            width, height
        );


        /* Draw text. */
        if(obj_ptr != NULL)
        {
            OSWSetFont(font);
            OSWSetFgPix(pix);

            sprintf(text, "AM: %.2f",
                obj_ptr->antimatter
            );
            OSWDrawString(
                pixmap,
                x + 3,
                y + 0 + 7,
                text
            );

            sprintf(text, "HD: %.2f'",
                RADTODEG(obj_ptr->heading)
            );
            OSWDrawString(
                pixmap,
                x + 3,
                y + 12 + 7,
                text
            );

            sprintf(text, "Sect: %ld %ld %ld",
                obj_ptr->sect_x, obj_ptr->sect_y, obj_ptr->sect_z
            );   
            OSWDrawString(
                pixmap,
                x + 3,
                y + 24 + 7,
                text
            );

            sprintf(text, "X: %.2f",
                obj_ptr->x   
            );
            OSWDrawString(
                pixmap,
                x + 3,
                y + 36 + 7,
                text   
            );

            sprintf(text, "Y: %.2f",
                obj_ptr->y
            );
            OSWDrawString(
                pixmap,
                x + 3,
                y + 48 + 7,
                text   
            );
        }


        /* Put to window. */
        if(put_to_window)
        {
            OSWCopyDrawablesCoord(
                w, pixmap,
                x, y,  
                width, height,
                x, y
            );
        }

        return;
}






/*
 *	Front end for drawing bridge window's console panels.
 */
void BridgeWinDrawPanel(
	int obj_num,		/* Subject object. */
	int panel_detail	/* One of BPANEL_DETAIL_*. */
)
{
	font_t	*font,
		*prev_font;
	pixel_t pix;
	pixmap_t pixmap_buf = 0;
	win_t w = 0;
	xsw_object_struct *obj_ptr = NULL;



	/* Object must be validated here. */
	if(DBIsObjectGarbage(obj_num))
	    obj_ptr = NULL;
	else
	    obj_ptr = xsw_object[obj_num];

	/* Record previous font. */
	prev_font = OSWQueryCurrentFont();


	font = xsw_font.console_standard;
	pix = widget_global.editable_text_pix;

	pixmap_buf = bridge_win.pan_buf;


	/* ********************************************************* */
	switch(panel_detail)
	{
          /* ******************************************************* */
	  case BPANEL_DETAIL_P1:
            w = bridge_win.pan_p1;
	    BWDP_BACKGROUND(
		obj_num,
		obj_ptr,
		pixmap_buf,
		panel_detail
	    );
            BWDP_PNAME(
                obj_num, obj_ptr,
                xsw_font.console_heading, xsw_color.bp_bold_text,
                pixmap_buf, w,
                IMG_CODE_STATS_CON1,
		False
            );
            BWDP_PSHIELDFREQ(
                obj_num, obj_ptr,
                font, xsw_color.bp_standard_text,
                pixmap_buf, w,
                IMG_CODE_STATS_CON1,
                False
            );
            BWDP_PWEAPONFREQ(
                obj_num, obj_ptr,
                font, xsw_color.bp_standard_text,
                pixmap_buf, w,
                IMG_CODE_STATS_CON1,
                False
            );
            BWDP_PCOMCHANNEL(
                obj_num, obj_ptr,
                font, xsw_color.bp_standard_text,
                pixmap_buf, w,
                IMG_CODE_STATS_CON1,
                False
            );
	    OSWPutBufferToWindow(w, pixmap_buf);
            break;

          /* ******************************************************* */
          case BPANEL_DETAIL_P2:
            w = bridge_win.pan_p2;
            BWDP_BACKGROUND(
                obj_num,
                obj_ptr,
                pixmap_buf,
                panel_detail
            );
            BWDP_PHULL(
                obj_num, obj_ptr,
                font, xsw_color.bpol_hull,
                pixmap_buf, w,
                IMG_CODE_STATS_CON2,
                False
            );
	    BWDP_PPOWER(
                obj_num, obj_ptr,
                font, xsw_color.bpol_power,
                pixmap_buf, w,
                IMG_CODE_STATS_CON2,
                False
            );
            BWDP_PSHIELDS(
                obj_num, obj_ptr,
                font, xsw_color.bpol_shields,
                pixmap_buf, w,
                IMG_CODE_STATS_CON2,
                False
            );
	    BWDP_PVIS(
                obj_num, obj_ptr,
                font, xsw_color.bpol_vis,
                pixmap_buf, w,
                IMG_CODE_STATS_CON2,
                False
            );
            BWDP_PDMGCTL(
                obj_num, obj_ptr,
                font, xsw_color.bpol_dmgctl,
                pixmap_buf, w,
                IMG_CODE_STATS_CON2,
                False
            );
            OSWPutBufferToWindow(w, pixmap_buf);
            break;

          /* ******************************************************* */
          case BPANEL_DETAIL_P3:
            w = bridge_win.pan_p3;  
            BWDP_BACKGROUND(
                obj_num,
                obj_ptr,
                pixmap_buf,
                panel_detail
            );
            BWDP_PTHROTTLE(
                obj_num, obj_ptr,
                font, xsw_color.bpol_throttle,
                pixmap_buf, w,
                IMG_CODE_STATS_CON3,
                False
            );
            BWDP_PVELOCITY(
                obj_num, obj_ptr,
                font, xsw_color.bp_light_outline,
                pixmap_buf, w,
                IMG_CODE_STATS_CON3,
                False
            );
            BWDP_PANTIMATTER(
                obj_num, obj_ptr,
                font, xsw_color.bpol_power,     /* Use power color for now. */
                pixmap_buf, w,
                IMG_CODE_STATS_CON3,
                False
            );
            BWDP_PENGINESTATE(
                obj_num, obj_ptr,
                font, xsw_color.bp_light_outline,
                pixmap_buf, w,
                IMG_CODE_STATS_CON3,
                False
            );  
            BWDP_PHEADING(
                obj_num, obj_ptr,
                font, xsw_color.bp_standard_text,
                pixmap_buf, w,
                IMG_CODE_STATS_CON3,
                False   
            );
            BWDP_PTHRUSTVECTOR(
                obj_num, obj_ptr,
                font, xsw_color.bp_light_outline,
                pixmap_buf, w,
                IMG_CODE_STATS_CON3,
                False
            );
            OSWPutBufferToWindow(w, pixmap_buf); 
            break;

          /* ******************************************************* */
          case BPANEL_DETAIL_P4:
            w = bridge_win.pan_p4;
            BWDP_BACKGROUND(
                obj_num,
                obj_ptr,
                pixmap_buf,
                panel_detail
            );
            BWDP_PINAME(
                obj_num, obj_ptr,
                font, xsw_color.bp_standard_text,
                pixmap_buf, w,
                IMG_CODE_STATS_CON4,
                False
            );
            BWDP_PWLOCK(
                obj_num, obj_ptr,
                font, xsw_color.bp_standard_text,
                pixmap_buf, w,
                IMG_CODE_STATS_CON4,
                False
            );
            BWDP_PWEAPONS(
                obj_num, obj_ptr,
                font, xsw_color.bp_standard_text,
                pixmap_buf, w,
                IMG_CODE_STATS_CON4,
                False
            );
            OSWPutBufferToWindow(w, pixmap_buf);
            break;

          /* ******************************************************* */
          case BPANEL_DETAIL_S1:
            w = bridge_win.pan_s1;
            BWDP_BACKGROUND(
                obj_num,
                obj_ptr,
                pixmap_buf,
                panel_detail
            );
            BWDP_SNAME(
                obj_num, obj_ptr,
                xsw_font.console_heading, xsw_color.bp_bold_text,
                pixmap_buf, w,
                IMG_CODE_SRO_CON1,
                False
            );
            BWDP_SHULL(
                obj_num, obj_ptr,
                font, xsw_color.bpol_hull,
                pixmap_buf, w,
                IMG_CODE_SRO_CON1,
                False
            );
            BWDP_SPOWER(
                obj_num, obj_ptr,
                font, xsw_color.bpol_power,
                pixmap_buf, w,
                IMG_CODE_SRO_CON1,
                False
            );
            BWDP_SVIS(
                obj_num, obj_ptr,
                font, xsw_color.bpol_vis,
                pixmap_buf, w,
                IMG_CODE_SRO_CON1,
                False
            );
            BWDP_SSHIELDS(  
                obj_num, obj_ptr,
                font, xsw_color.bpol_shields,
                pixmap_buf, w,
                IMG_CODE_SRO_CON1,
                False
            );
            BWDP_SBEARING(
                obj_num, obj_ptr,
                font, xsw_color.bp_light_outline,
                pixmap_buf, w,
                IMG_CODE_SRO_CON1,
                False
            );
            BWDP_SDISTANCE(
                obj_num, obj_ptr,
                font, xsw_color.bp_light_outline,
                pixmap_buf, w,
                IMG_CODE_SRO_CON1,
                False
            );
/*
            BWDP_SDMGCTL(
                obj_num, obj_ptr,
                font, xsw_color.bpol_dmgctl,
                pixmap_buf, w,
                IMG_CODE_SRO_CON1,
                False
            );
 */
            OSWPutBufferToWindow(w, pixmap_buf);
            break;

          /* ******************************************************* */
          case BPANEL_DETAIL_S2:
            w = bridge_win.pan_s2;
            BWDP_BACKGROUND(
                obj_num,
                obj_ptr,
                pixmap_buf,
                panel_detail
            );
            BWDP_SEMPIRE(
                obj_num, obj_ptr, 
                font,
                0,              /* Pixel ignored. */
                pixmap_buf, w,
                IMG_CODE_SRO_CON2,
                False
            );
            BWDP_SWEAPONS(
                obj_num, obj_ptr,
                font,
		0,		/* Pixel ignored. */
                pixmap_buf, w,
                IMG_CODE_SRO_CON2,
                False
            );
            OSWPutBufferToWindow(w, pixmap_buf);
            break;

          /* ******************************************************* */
          case BPANEL_DETAIL_S3:
            w = bridge_win.pan_s3;
            BWDP_BACKGROUND(
                obj_num,
                obj_ptr,
                pixmap_buf,
                panel_detail
            );

	    BWDP_SANTIMATTER(
                obj_num, obj_ptr,
                font, xsw_color.bpol_power,     /* Use power color for now. */
                pixmap_buf, w,
                IMG_CODE_SRO_CON3,
                False
            );
            BWDP_SHEADING(
                obj_num, obj_ptr,   
                font, xsw_color.bp_standard_text,
                pixmap_buf, w,
                IMG_CODE_SRO_CON3,
                False
            );
            OSWPutBufferToWindow(w, pixmap_buf);
            break;


          /* ******************************************************* */
	  case BPANEL_DETAIL_PNAME:
	    w = bridge_win.pan_p1;
	    BWDP_PNAME(
		obj_num, obj_ptr,
		xsw_font.console_heading, xsw_color.bp_bold_text,
		pixmap_buf, w,
		IMG_CODE_STATS_CON1,
		True
	    );
	    break;

          /* ******************************************************* */
          case BPANEL_DETAIL_PSHIELDFREQ:
            w = bridge_win.pan_p1;
            BWDP_PSHIELDFREQ(
                obj_num, obj_ptr,
                font, xsw_color.bp_standard_text,
                pixmap_buf, w,
                IMG_CODE_STATS_CON1,
		True
            );
            break;

          /* ******************************************************* */
          case BPANEL_DETAIL_PWEAPONFREQ:
            w = bridge_win.pan_p1;
            BWDP_PWEAPONFREQ(
                obj_num, obj_ptr,
                font, xsw_color.bp_standard_text,
                pixmap_buf, w,
                IMG_CODE_STATS_CON1,
                True
            );  
            break;
          
          /* ******************************************************* */
          case BPANEL_DETAIL_PCOMCHANNEL:
            w = bridge_win.pan_p1;
            BWDP_PCOMCHANNEL(
                obj_num, obj_ptr,
                font, xsw_color.bp_standard_text,
                pixmap_buf, w,
                IMG_CODE_STATS_CON1,
                True
            );
            break;

	  /* ******************************************************* */
          case BPANEL_DETAIL_PHULL:
            w = bridge_win.pan_p2;
            BWDP_PHULL(
                obj_num, obj_ptr,
                font, xsw_color.bpol_hull,
                pixmap_buf, w,
                IMG_CODE_STATS_CON2,
                True
            );
            break;

          /* ******************************************************* */
          case BPANEL_DETAIL_PPOWER:
            w = bridge_win.pan_p2;
            BWDP_PPOWER(
                obj_num, obj_ptr,
                font, xsw_color.bpol_power,
                pixmap_buf, w,
                IMG_CODE_STATS_CON2,
                True
            );
            break;

          /* ******************************************************* */
          case BPANEL_DETAIL_PSHIELDS:
            w = bridge_win.pan_p2;
	    BWDP_PSHIELDS(  
                obj_num, obj_ptr,
                font, xsw_color.bpol_shields,
                pixmap_buf, w,
                IMG_CODE_STATS_CON2,
                True
            );
            break;

          /* ******************************************************* */
          case BPANEL_DETAIL_PVIS:
            w = bridge_win.pan_p2;
            BWDP_PVIS(
                obj_num, obj_ptr,
                font, xsw_color.bpol_vis,
                pixmap_buf, w,
                IMG_CODE_STATS_CON2,
                True
            );
            break;

          /* ******************************************************* */
          case BPANEL_DETAIL_PDMGCTL:
            w = bridge_win.pan_p2;
            BWDP_PDMGCTL(
                obj_num, obj_ptr,
                font, xsw_color.bpol_dmgctl,
                pixmap_buf, w,
                IMG_CODE_STATS_CON2,
                True
            );
            break;


          /* ******************************************************* */
          case BPANEL_DETAIL_SNAME:
            w = bridge_win.pan_s1;
            BWDP_SNAME(
                obj_num, obj_ptr,
                xsw_font.console_heading, xsw_color.bp_bold_text,
                pixmap_buf, w,
                IMG_CODE_SRO_CON1,
                True
            );  
            break;

          /* ******************************************************* */
          case BPANEL_DETAIL_SHULL:
            w = bridge_win.pan_s1;
            BWDP_SHULL(
                obj_num, obj_ptr,
                font, xsw_color.bpol_hull,
                pixmap_buf, w,
                IMG_CODE_SRO_CON1,
                True
            );
            break;

          /* ******************************************************* */
          case BPANEL_DETAIL_SPOWER:
            w = bridge_win.pan_s1;
            BWDP_SPOWER(
                obj_num, obj_ptr,
                font, xsw_color.bpol_power,
                pixmap_buf, w,
                IMG_CODE_SRO_CON1,
                True
            );
            break;

          /* ******************************************************* */
          case BPANEL_DETAIL_SSHIELDS:
            w = bridge_win.pan_s1;
            BWDP_SSHIELDS(
                obj_num, obj_ptr,
                font, xsw_color.bpol_shields,
                pixmap_buf, w,
                IMG_CODE_SRO_CON1,
                True
            );
            break;

          /* ******************************************************* */
          case BPANEL_DETAIL_SVIS:
            w = bridge_win.pan_s1;
            BWDP_SVIS(
                obj_num, obj_ptr,
                font, xsw_color.bpol_vis,
                pixmap_buf, w,
                IMG_CODE_SRO_CON1,
                True
            );
            break;

          /* ******************************************************* */
          case BPANEL_DETAIL_SBEARING:
            w = bridge_win.pan_s1;
            BWDP_SBEARING(
                obj_num, obj_ptr,
                font, xsw_color.bp_light_outline,
                pixmap_buf, w,
                IMG_CODE_SRO_CON1,
                True
            );
            break;

          /* ******************************************************* */
          case BPANEL_DETAIL_SDISTANCE:
            w = bridge_win.pan_s1;
            BWDP_SDISTANCE(
                obj_num, obj_ptr,
                font, xsw_color.bp_light_outline,
                pixmap_buf, w,
                IMG_CODE_SRO_CON1,
                True
            );
            break;

          /* ******************************************************* */
          case BPANEL_DETAIL_SDMGCTL:
            w = bridge_win.pan_s1;
/*
            BWDP_SDMGCTL(
                obj_num, obj_ptr,
                font, xsw_color.bpol_dmgctl,
                pixmap_buf, w,
                IMG_CODE_SRO_CON1,
                True
            );
 */
            break;


          /* ******************************************************* */
          case BPANEL_DETAIL_PINAME:
            w = bridge_win.pan_p4;
            BWDP_PINAME(   
                obj_num, obj_ptr,
                font, xsw_color.bp_standard_text,
                pixmap_buf, w,
                IMG_CODE_STATS_CON4,
                True
            );
            break;

          /* ******************************************************* */
          case BPANEL_DETAIL_PWLOCK:
            w = bridge_win.pan_p4;
            BWDP_PWLOCK(
                obj_num, obj_ptr,
                font, xsw_color.bp_standard_text,
                pixmap_buf, w,
                IMG_CODE_STATS_CON4,
                True
            );
            break;

          /* ******************************************************* */
          case BPANEL_DETAIL_PWEAPONS:
            w = bridge_win.pan_p4;
            BWDP_PWEAPONS(
                obj_num, obj_ptr,
                font, xsw_color.bp_standard_text,
                pixmap_buf, w,
                IMG_CODE_STATS_CON4,
                True
            );
            break;


          /* ******************************************************* */
          case BPANEL_DETAIL_SINAME:
            w = bridge_win.pan_s2;

	    break;

          /* ******************************************************* */
          case BPANEL_DETAIL_SWLOCK:
            w = bridge_win.pan_s2;  

            break;

          /* ******************************************************* */
          case BPANEL_DETAIL_SEMPIRE:
            w = bridge_win.pan_s2;
            BWDP_SEMPIRE(
                obj_num, obj_ptr,
                font,
                0,		/* Pixel ignored. */
                pixmap_buf, w,
                IMG_CODE_SRO_CON2,
                True
            );
            break;

          /* ******************************************************* */
          case BPANEL_DETAIL_SWEAPONS:
            w = bridge_win.pan_s2;
            BWDP_SWEAPONS(
                obj_num, obj_ptr,
                font,
		0,		/* Pixel ignored. */
                pixmap_buf, w,
                IMG_CODE_SRO_CON2,
                True
            );
            break;


          /* ******************************************************* */ 
          case BPANEL_DETAIL_PTHROTTLE:
	    w = bridge_win.pan_p3;
            BWDP_PTHROTTLE(
                obj_num, obj_ptr,
                font, xsw_color.bpol_throttle,
                pixmap_buf, w,
                IMG_CODE_STATS_CON3,
                True
            );
            break;

          /* ******************************************************* */
          case BPANEL_DETAIL_PVELOCITY:
            w = bridge_win.pan_p3;
            BWDP_PVELOCITY(
                obj_num, obj_ptr,
                font, xsw_color.bp_light_outline,
                pixmap_buf, w,
                IMG_CODE_STATS_CON3,
                True
            );
            break;

          /* ******************************************************* */
          case BPANEL_DETAIL_PANTIMATTER:
            w = bridge_win.pan_p3;
            BWDP_PANTIMATTER(
                obj_num, obj_ptr,
                font, xsw_color.bpol_power,	/* Use power color for now. */
                pixmap_buf, w,
                IMG_CODE_STATS_CON3,
                True
            );
            break;

          /* ******************************************************* */
          case BPANEL_DETAIL_PENGINESTATE:
            w = bridge_win.pan_p3;
            BWDP_PENGINESTATE(
                obj_num, obj_ptr,
                font, xsw_color.bp_light_outline,
                pixmap_buf, w,
                IMG_CODE_STATS_CON3,
                True
            );
            break;

          /* ******************************************************* */
          case BPANEL_DETAIL_PHEADING:
            w = bridge_win.pan_p3;
            BWDP_PHEADING(
                obj_num, obj_ptr,
                font, xsw_color.bp_standard_text,
                pixmap_buf, w,
                IMG_CODE_STATS_CON3,
                True
            );
            break;

          /* ******************************************************* */
          case BPANEL_DETAIL_PTHRUSTVECTOR:
            w = bridge_win.pan_p3;
            BWDP_PTHRUSTVECTOR(
                obj_num, obj_ptr,
                font, xsw_color.bp_light_outline,
                pixmap_buf, w,
                IMG_CODE_STATS_CON3,
                True
            );
            break;


          /* ******************************************************* */
          case BPANEL_DETAIL_SANTIMATTER:
            w = bridge_win.pan_s3;
            BWDP_SANTIMATTER(
                obj_num, obj_ptr,
                font, xsw_color.bpol_power,	/* Use power color for now. */
                pixmap_buf, w,
                IMG_CODE_SRO_CON3,
                True
            );
	    break;

          /* ******************************************************* */
          case BPANEL_DETAIL_SHEADING:
            w = bridge_win.pan_s3;
	    BWDP_SHEADING(
                obj_num, obj_ptr,
                font, xsw_color.bp_standard_text,
                pixmap_buf, w,
                IMG_CODE_SRO_CON3,
                True
            );
            break;



          /* ******************************************************* */
	  default:
/*
	    fprintf(stderr,
		"BridgeWinDrawPanel(): Unknown panel detail code `%i'\n",
		panel_detail
	    );
*/
	    break;
	}


	/* Set back font. */
	OSWSetFont(prev_font);


	return;
}


/*
 *	Procedure to redraw all windows and related resources.
 */
void BridgeWinDrawAll()
{
	int tar_obj, player_obj_num;


	player_obj_num = net_parms.player_obj_num;


	/* Selected weapon stats label on viewscreen. */
        VSDrawUpdateWeaponLabel(
            &bridge_win.vs_weapon_image,
            bridge_win.vs_weapon_buf
        );

	/* Network stats labe on viewscreenl. */
        VSDrawUpdateNetstatsLabel(
            &bridge_win.net_stats_image,
            bridge_win.net_stats_buf
        );

        /* Draw viewscreen. */
	if(bridge_win.viewscreen_vis_state != VisibilityFullyObscured)
            VSDrawViewScreen(
                net_parms.player_obj_num,
		bridge_win.viewscreen,
                bridge_win.viewscreen_image,
		bridge_win.viewscreen_zoom
            );

	/* Scanner labels. */
	ScannerUpdateLabels(player_obj_num);

	/* Scanner. */
	if(bridge_win.scanner_vis_state != VisibilityFullyObscured)
            ScannerDraw(
                player_obj_num,
                bridge_win.scanner,
                bridge_win.scanner_image,
                0,
                0,
                bridge_win.scanner_zoom 
            );


	/* Player stats console panels. */
	if(bridge_win.map_state)
	{
            BridgeWinDrawPanel(player_obj_num, BPANEL_DETAIL_P1);
            BridgeWinDrawPanel(player_obj_num, BPANEL_DETAIL_P2);
            BridgeWinDrawPanel(player_obj_num, BPANEL_DETAIL_P3);
            BridgeWinDrawPanel(player_obj_num, BPANEL_DETAIL_P4);

	    /* Get subject object or -1. */
            if(net_parms.player_obj_ptr == NULL)
                tar_obj = -1;
            else
                tar_obj = net_parms.player_obj_ptr->locked_on;
	    /* Subject stats console panels. */
            BridgeWinDrawPanel(tar_obj, BPANEL_DETAIL_S1);
            BridgeWinDrawPanel(tar_obj, BPANEL_DETAIL_S2);
            BridgeWinDrawPanel(tar_obj, BPANEL_DETAIL_S3);
	}


	return;
}


/*
 *	Redraws the small messages window.
 */
int BridgeDrawMessages()
{
	int i, c_pos, lines_drawn;
	int line_x, line_y;


        /* Redraw background. */
        if(IMGIsImageNumAllocated(IMG_CODE_MESG_CON))
	    WidgetPutImageTile(
		bridge_win.mesg_box_buf,
		xsw_image[IMG_CODE_MESG_CON]->image,
		bridge_win.mesg_box_width,
		bridge_win.mesg_box_height
	    );
	else
	    OSWClearPixmap(
		bridge_win.mesg_box_buf,
		bridge_win.mesg_box_width,
		bridge_win.mesg_box_height,
		osw_gui[0].black_pix
	    );


        /* Get starting line. */
        i = MAX(
	    bridge_win.mesg_box_sb.y_win_pos /
	        (int)bridge_win.line_spacing,
	    0
	);

        line_x = 10;
        line_y = 10 - (bridge_win.mesg_box_sb.y_win_pos %
	    (int)bridge_win.line_spacing
	);


        /* Redraw the lines. */
        lines_drawn = 0;
        while(lines_drawn < 6)
        {
            if(i >= MESG_WIN_TOTAL_MESSAGES)
                break;

            if((pri_mesg_buf[i].sel_start < 0) ||
               (pri_mesg_buf[i].sel_end < 0)
            )
            {
                /* Draw unselected text. */
                OSWSetFgPix(pri_mesg_buf[i].pixel);
                OSWDrawString(
                    bridge_win.mesg_box_buf,
                    line_x,
		    line_y + ((14 / 2) + 5),
                    pri_mesg_buf[i].message
                );
            }
            else
            {
                /* Draw marked background. */
                OSWSetFgPix(widget_global.surface_selected_pix);
                OSWDrawSolidRectangle(
                    (drawable_t)bridge_win.mesg_box_buf,
                    line_x + (pri_mesg_buf[i].sel_start * BRIDGE_MESG_CHAR_WIDTH),
                    line_y - 1,
                    MAX((pri_mesg_buf[i].sel_end -
                         pri_mesg_buf[i].sel_start + 1) * BRIDGE_MESG_CHAR_WIDTH,
                        2
                    ),
                    BRIDGE_MESG_CHAR_HEIGHT + 2
                );

                /* Draw first unmarked text. */
                OSWSetFgPix(pri_mesg_buf[i].pixel);
                OSWDrawStringLimited(
                    (drawable_t)bridge_win.mesg_box_buf,
                    line_x,
                    line_y + ((14 / 2) + 5),
                    pri_mesg_buf[i].message,
                    pri_mesg_buf[i].sel_start
                );
                /* Draw marked text. */
                c_pos = pri_mesg_buf[i].sel_start;
                if((c_pos >= 0) && 
                   (c_pos < MESG_BUF_MAX_MESG_LEN)
                ) 
                {
                    OSWSetFgPix(widget_global.selected_text_pix);
                    OSWDrawStringLimited(
                        (drawable_t)bridge_win.mesg_box_buf,
                        line_x +
                            (pri_mesg_buf[i].sel_start *
                            BRIDGE_MESG_CHAR_WIDTH),
                        line_y + ((14 / 2) + 5),
                        &pri_mesg_buf[i].message[c_pos],
                        pri_mesg_buf[i].sel_end -
                            pri_mesg_buf[i].sel_start + 1
                    );
                }
                /* Draw last unmarked text. */
                c_pos = pri_mesg_buf[i].sel_end + 1;
                if((c_pos >= 0) &&
                   (c_pos < MESG_BUF_MAX_MESG_LEN)
                )
                {
                    OSWSetFgPix(pri_mesg_buf[i].pixel);
                    OSWDrawStringLimited(
                        (drawable_t)bridge_win.mesg_box_buf,
                        line_x +
                            ((pri_mesg_buf[i].sel_end + 1) *
                            BRIDGE_MESG_CHAR_WIDTH),
                        line_y + ((14 / 2) + 5),
                        &pri_mesg_buf[i].message[c_pos],
                        MESG_BUF_MAX_MESG_LEN -
                            pri_mesg_buf[i].sel_end
                    );
                }
            }

            line_y += (int)bridge_win.line_spacing;
            i++;
            lines_drawn++;
        }

        /* Copy the buffer to the window. */
        OSWPutBufferToWindow(bridge_win.mesg_box, bridge_win.mesg_box_buf);

        
        return(0);
}
