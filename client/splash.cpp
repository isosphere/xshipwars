/*
                    Splash Window Handling

	Functions:

	void SplashDoFadeIn(win_t w, image_t *src_img)

	int SplashInit()
	int SplashDrawProgressBar(
	        splash_win_struct *sp,
	        double pos
	)
	int SplashDoUpdateProgress(
		long items,
		long max_items,
		char *message
	)
	void SplashDestroy()

	---

 */

#include "../include/swsoundcodes.h"

#include "blitting.h"

#include "xsw.h"
#include "splash.h"
#include "imagepaths.h"


/* Default icon data. */
#include "ct_data/icons/xsw.h"


#define MIN(a,b)	((a) < (b) ? (a) : (b))
#define MAX(a,b)	((a) > (b) ? (a) : (b))


splash_win_struct splash_win;


/*
 *	Does the opening fadein affect.
 */
void SplashDoFadeIn(win_t w, image_t *src_img)
{
	int buf_len8;
	int x;
	double gamma;

	shared_image_t *tar_img;

	unsigned int width, height;

	u_int8_t *tar_xidp8, *src_xidp8;
        u_int16_t *tar_xidp16, *src_xidp16;
	u_int32_t *tar_xidp32, *src_xidp32;


	/* Error checks. */
	if(!IDC() ||
	   (w == 0) ||
           (src_img == NULL)
	)
	    return;

	/* Get width and height from source image. */
	width = src_img->width;
	height = src_img->height;
	if((width < 2) ||
	   (height < 2)
	)
	    return;


	/* ********************************************************* */

	/* Check if source image depth is correct. */
	if((src_img->depth != 8) &&
	   (src_img->depth != 15) &&
	   (src_img->depth != 16) &&
	   (src_img->depth != 24) &&
           (src_img->depth != 32)
	)
	    return;


	/* ********************************************************* */
	/* Create shared target image. */
	if(
	    OSWCreateSharedImage(
		&tar_img,
		width,
		height
	    )
	)
	    return;

        /* Calculate buf_len8. */
        buf_len8 = tar_img->bytes_per_line * tar_img->height;


        /* ********************************************************* */

	/* Check if images have data allocated. */
	if((src_img->data == NULL) ||
	   (tar_img->data == NULL)
	)
	    return;

	/* Map window. */
        OSWMapRaised(w);
	splash_win.map_state = 1;
        splash_win.is_in_focus = 1;
        splash_win.visibility_state = VisibilityUnobscured;


	/* Fade in. */

	/* 8 bits. */
	if(osw_gui[0].depth == 8)
	{
            gamma = 0.0;
            while(gamma < 1.0)
            {
                tar_xidp8 = (u_int8_t *)tar_img->data;
                src_xidp8 = (u_int8_t *)src_img->data;
                for(x = 0; x < buf_len8; x += BYTES_PER_PIXEL8)
                    *tar_xidp8++ = *src_xidp8++;

                BlitBufFade(
                    osw_gui[0].depth,
                    tar_img->data,
                    0, 0,
                    tar_img->width, tar_img->height,
                    tar_img->width, tar_img->height,
                    gamma
                );

                OSWPutSharedImageToDrawable(tar_img, w);
                OSWGUISync(False);

                usleep(8000);

                /* Increment gamma. */   
                gamma += 0.10;
            }
	}
        /* 15 bits. */
        else if(osw_gui[0].depth == 15)
        {
            gamma = 0.0;
            while(gamma < 1.0)
            {
                tar_xidp16 = (u_int16_t *)tar_img->data;
                src_xidp16 = (u_int16_t *)src_img->data;
                for(x = 0; x < buf_len8; x += BYTES_PER_PIXEL15)
                    *tar_xidp16++ = *src_xidp16++;

                BlitBufFade(
                    osw_gui[0].depth,
                    tar_img->data,
                    0, 0,
                    tar_img->width, tar_img->height,
                    tar_img->width, tar_img->height,
                    gamma
                );

                OSWPutSharedImageToDrawable(tar_img, w);
                OSWGUISync(False);

                usleep(8000);

                /* Increment gamma. */
                gamma += 0.10;
            }
        }
	/* 16 bits. */
	else if(osw_gui[0].depth == 16)
	{
            gamma = 0.0;
            while(gamma < 1.0)
            {
		tar_xidp16 = (u_int16_t *)tar_img->data;
		src_xidp16 = (u_int16_t *)src_img->data;
		for(x = 0; x < buf_len8; x += BYTES_PER_PIXEL16)
		    *tar_xidp16++ = *src_xidp16++;

		BlitBufFade(
		    osw_gui[0].depth,
		    tar_img->data,
		    0, 0,
		    tar_img->width, tar_img->height,
		    tar_img->width, tar_img->height,
		    gamma
		);

		OSWPutSharedImageToDrawable(tar_img, w);
                OSWGUISync(False);

                usleep(8000);

                /* Increment gamma. */
                gamma += 0.10;
	    }
	}
	/* 24 or 32 bits. */
	else if((osw_gui[0].depth == 24) ||
	        (osw_gui[0].depth == 32)
	)
	{
	    gamma = 0.0;
	    while(gamma < 1.0)
	    {
                tar_xidp32 = (u_int32_t *)tar_img->data;
                src_xidp32 = (u_int32_t *)src_img->data;
                for(x = 0; x < buf_len8; x += BYTES_PER_PIXEL32)
                    *tar_xidp32++ = *src_xidp32++;

                BlitBufFade(
                    osw_gui[0].depth,   
                    tar_img->data,
                    0, 0,
                    tar_img->width, tar_img->height,
                    tar_img->width, tar_img->height,
                    gamma
                );

		OSWPutSharedImageToDrawable(tar_img, w);
                OSWGUISync(False);

	        usleep(8000);

	        /* Increment gamma. */
	        gamma += 0.10;
	    }
	}

        /* ********************************************************* */
	/* Destroy all allocated memory for src_ximage. */

	/* Destroy shared target image. */
	OSWSyncSharedImage(tar_img, w);
	OSWDestroySharedImage(&tar_img);


        /*   Must set its map state to 0 so it gets mapped when drawn
	 *   again.
	 */
        splash_win.map_state = 0;


	return;
}


/*
 *	Initializes the splash window.
 */
int SplashInit()
{
        char title[256];
	char *strptr;
	pixmap_t pixmap;
	image_t *image;
	win_attr_t wattr;


	if(!IDC())
	    return(-1);


        /* Load splash background and progress bar images. */
        strptr = PrefixPaths(dname.images, IMGPATH_SPLASH_BKG);
        splash_win.bkg_img = WidgetLoadImageFromTgaFile(strptr);



        /* Set values. */
	splash_win.map_state = 0;
	splash_win.is_in_focus = 0;
	splash_win.visibility_state = VisibilityFullyObscured;
	splash_win.x = 0;
	splash_win.y = 0;
	/* Width and height from splash background image. */
	if(splash_win.bkg_img == NULL)
	{
            splash_win.width = SW_DEF_WIDTH;
            splash_win.height = SW_DEF_HEIGHT;
	}
	else
	{
	    splash_win.width = splash_win.bkg_img->width;
	    splash_win.height = splash_win.bkg_img->height;
	}
	if(splash_win.width < 2)
	    splash_win.width = 2;
        if(splash_win.height < 2)
            splash_win.height = 2;           


        /* Toplevel. */
	if(
	    OSWCreateWindow(
	        &splash_win.toplevel,
	        osw_gui[0].root_win,
                0, 0,
                splash_win.width, splash_win.height
	    )
        )
	    return(-1);
	OSWSetWindowBkg(splash_win.toplevel, osw_gui[0].black_pix, 0);
        OSWSetWindowInput(splash_win.toplevel, OSW_EVENTMASK_TOPLEVEL);
        OSWSetWindowCursor(splash_win.toplevel,
            OSWLoadBasicCursor(XC_watch)
	);
        WidgetCenterWindow(splash_win.toplevel, WidgetCenterWindowToRoot);

	OSWGetWindowAttributes(splash_win.toplevel, &wattr);
	splash_win.x = wattr.x;
	splash_win.y = wattr.y;
	splash_win.width = wattr.width;
	splash_win.height = wattr.height;



	/* Load icon. */
	image = WidgetLoadImageFromTgaData(xsw_icon_tga);
	pixmap = OSWCreatePixmapFromImage(image);
	OSWDestroyImage(&image);


        /* WM properties. */  
        sprintf(title, "%s: Loading...", PROG_NAME);
        OSWSetWindowWMProperties(
            splash_win.toplevel,
            title,              /* Title. */
            PROG_NAME,		/* Icon title. */
            pixmap,             /* Icon. */
            False,   /* Let WM set coordinates? */
            /* Coordinates. */
            splash_win.x, splash_win.y,
            /* Min width and height. */
            splash_win.width, splash_win.height,
            /* Max width and height. */
            splash_win.width, splash_win.height,
            WindowFrameStyleNaked,
            NULL, 0
        );
	/* Do not destroy pixmap. */
	pixmap = 0;


	/* ********************************************************* */

	/* Start XSW logo sound play. */
	if(option.sounds > XSW_SOUNDS_NONE)
	    SoundPlay(
                SOUND_CODE_XSW_LOGO,
		1.00,
		1.00,
		0,
		0
	    );

	/* Do the way-cool opening effect! */
	SplashDoFadeIn(splash_win.toplevel, splash_win.bkg_img);

	/* Set background. */
	if(splash_win.bkg_img != NULL)
	{
            pixmap = OSWCreatePixmapFromImage(splash_win.bkg_img);
	    if(pixmap != 0)
                OSWSetWindowBkg(splash_win.toplevel, 0, pixmap);
	    OSWDestroyPixmap(&pixmap);
	}


	return(0);
}


/*
 *	Procedure to draw splash window's progress bar.
 */
int SplashDrawProgressBar(
	splash_win_struct *sp,
	double pos		/* From 0 to 1. */
)
{
	char *strptr;
	image_t *pb_l_img, *pb_r_img, *pb_t_img, *bkg_img;
	image_t *tmp_img;
	int x, y, cx, cx_end;
	unsigned int width, height, bwidth;
	win_attr_t wattr;
	int ticks = 12;		/* Number of ticks. */


	if(sp == NULL)
	    return(-1);


	/* Load background image as needed. */
	if(sp->bkg_img == NULL)
        {
            strptr = PrefixPaths(
                dname.images,
                IMGPATH_SPLASH_BKG
            );
            sp->bkg_img = WidgetLoadImageFromTgaFile(strptr);
        }

	/* Load progress bar images as needed. */
	if(sp->pb_l_img == NULL)
	{
	    strptr = PrefixPaths(
		dname.images,
		IMGPATH_SPLASH_PB_L
	    );
	    sp->pb_l_img = WidgetLoadImageFromTgaFile(strptr);
	}
        if(sp->pb_r_img == NULL) 
        {
            strptr = PrefixPaths(
                dname.images,
                IMGPATH_SPLASH_PB_R
            );
            sp->pb_r_img = WidgetLoadImageFromTgaFile(strptr);
        }
        if(sp->pb_t_img == NULL)
        {
            strptr = PrefixPaths(
                dname.images,
                IMGPATH_SPLASH_PB_T
            );
            sp->pb_t_img = WidgetLoadImageFromTgaFile(strptr);

	    /* Resize ticks image. */
	    if(sp->pb_t_img != NULL)
	    {
	        OSWGetWindowAttributes(sp->toplevel, &wattr);
	        width = static_cast<unsigned int>((int)wattr.width * SW_LS_BAR_WIDTHC);
                width -= (int)((sp->pb_l_img == NULL) ? 0 : sp->pb_l_img->width);
		width -= (int)((sp->pb_r_img == NULL) ? 0 : sp->pb_r_img->width);

		width = MAX((int)width / ticks, 2);
		if(IS_NUM_ODD(width) &&
                   (width > 2)
		)
		    width--;

		if(!OSWCreateImage(
		    &tmp_img,
		    width,
		    sp->pb_t_img->height
		))
		{
		    WidgetResizeImageBuffer(
			osw_gui[0].depth,
			reinterpret_cast<u_int8_t *>(tmp_img->data), /* Target. */
			reinterpret_cast<u_int8_t *>(sp->pb_t_img->data), /* Source. */
                        tmp_img->width,
			tmp_img->height,
                        sp->pb_t_img->width,
                        sp->pb_t_img->height
                    );
		    OSWDestroyImage(&sp->pb_t_img);
		    sp->pb_t_img = tmp_img;
		}
	    }
        }

	bkg_img = sp->bkg_img;
	pb_l_img = sp->pb_l_img;
	pb_r_img = sp->pb_r_img;
	pb_t_img = sp->pb_t_img;


        OSWGetWindowAttributes(sp->toplevel, &wattr);
        width = static_cast<unsigned int>((int)wattr.width * SW_LS_BAR_WIDTHC);
	bwidth = MAX(
	    (int)width - (int)((pb_l_img == NULL) ? 0 : pb_l_img->width) -
                (int)((pb_r_img == NULL) ? 0 : pb_r_img->width),
	    0
	);

	x = ((int)wattr.width - (int)width) / 2;
	y = static_cast<int>((int)wattr.height * SW_LS_BAR_YC);

	height = (int)((pb_l_img == NULL) ? 0 : pb_l_img->height);
	height = MAX(
	    (int)((pb_r_img == NULL) ? 0 : pb_r_img->height),
	    (int)height
	);
	height = MAX(
            (int)((pb_t_img == NULL) ? 0 : pb_t_img->height),
            (int)height
        );


	/* Recreate buffer as needed. */
	if(sp->toplevel_buf == 0)
	{
	    OSWCreatePixmap(
		&sp->toplevel_buf,
		wattr.width, wattr.height
	    );
	}

	/* Clear portion of background. */
	OSWPutImageToDrawableSect(
            bkg_img, sp->toplevel_buf,
            x, y,
	    x, y,
            width, height
        );


	/* Draw ticks. */
	if(pb_t_img != NULL)
	{
	    for(cx = x + (int)((pb_l_img == NULL) ? 0 : pb_l_img->width),
		cx_end =static_cast<int>( cx + ((int)bwidth * pos));
                cx < cx_end;
                cx += MAX((int)pb_t_img->width, 2)
	    )
	    {
		WidgetPutImageNormal(
                    sp->toplevel_buf, pb_t_img,
                    cx - 7, y,
		    True
		);
	    }
	}


	/* Draw ends of bars. */
	if(pb_l_img != NULL)
	    WidgetPutImageNormal(
                sp->toplevel_buf, pb_l_img,
                x, y,
		True
            );

	if(pb_r_img != NULL)
            WidgetPutImageNormal(
                sp->toplevel_buf, pb_r_img,
                x + (int)width - (int)pb_r_img->width, y,
		True
            );


	OSWCopyDrawablesCoord(
            sp->toplevel,
            sp->toplevel_buf,
            x, y,
	    width, height,
	    x, y
        );


	return(0);
}


/*
 *	Updates the progress bar on the Splash window and redraws
 *	it.
 */
int SplashDoUpdateProgress(long items, long max_items, char *message)
{
	char stringa[256];

        if(!IDC())
            return(-1);


	/* items must be atleast 0. */
	if(items < 0)
	    items = 0;

	/* max_items must be atleast 1. */
	if(max_items < 1)
	    max_items = 1;


	/* Draw progress bar. */
	SplashDrawProgressBar(
            &splash_win,
            (double)items / (double)max_items
	);


	/* Redraw splash_win.toplevel. */
	sprintf(stringa, "Version: %s", PROG_VERSION);
        OSWSetFgPix(osw_gui[0].black_pix);
/*
	OSWClearWindow(splash_win.toplevel);
 */
        OSWDrawString(
	    splash_win.toplevel, 
            205, 230,
	    stringa
	);


	return(0);
}


/*
 *	Destroys the splash window.
 */
void SplashDestroy()
{
	if(IDC())
	{
	    OSWDestroyImage(&splash_win.pb_l_img);
            OSWDestroyImage(&splash_win.pb_r_img);
            OSWDestroyImage(&splash_win.pb_t_img);
            OSWDestroyImage(&splash_win.bkg_img);

	    OSWDestroyPixmap(&splash_win.toplevel_buf);
	    OSWDestroyWindow(&splash_win.toplevel);
	}

	splash_win.map_state = 0;
	splash_win.is_in_focus = 0;
	splash_win.x = 0;
	splash_win.y = 0;
	splash_win.width = 0;
	splash_win.height = 0;
	splash_win.visibility_state = VisibilityFullyObscured;


	return;
}
