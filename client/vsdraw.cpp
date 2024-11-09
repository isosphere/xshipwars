/*
                            Viewscreen Drawing

	Functions:

        void ScannerDraw(
                int object_num,
                win_t w,
                shared_image_t *image,
                double win_x, double win_y,
                double height, double width,
                double scan_scale
        )
	void ScannerUpdateLabels(int object_num)

	void VSDrawSectorTicks(
	        shared_image_t *image,
	        double x_rel,
	       	double y_rel,
	        int x_vtrans,
	        int y_vtrans
	)

        void VSDrawLockCursor(
  		xsw_object_struct *src_obj_ptr,
  		xsw_object_struct *tar_obj_ptr,
                shared_image_t *image,
                double x_rel,
                double y_rel,
                double tar_size
        )

        void VSDrawStreamWeapon(
		xsw_object_struct *obj_ptr,
                shared_image_t *image,
                double x_rel,
                double y_rel
        )

        void VSDrawHeadingArrow(
                shared_image_t *image,
                double arrow_heading,
                int x_center,
                int y_center
        )

        void VSDrawStarfield(
                shared_image_t *image,
                double x_rel,
                double y_rel,
		int x_vtrans,
		int y_vtrans,
		double zoom
        )

        void VSDrawViewscreenBackground(
		xsw_object_struct camera_obj_ptr,
		shared_image_t *image,
		double x_rel,
		double y_rel,
		int x_vtrans,
		int y_vtrans,
		double zoom
        )

	void VSDrawStandardObject(
	        xsw_object_struct *player_obj_ptr,
	        xsw_object_struct *obj_ptr,
	        shared_image_t *image,
	        double x_rel,		In XSW Real units.
	        double y_rel,
	        int x_vtrans,		In XSW Real units.
	        int y_vtrans,
		double zoom
	)

        void VSDrawViewScreen(
		int camera_obj_num,
                win_t w,
                shared_image_t *image,
		double zoom
        )

	void VSDrawUpdateWeaponLabel(
	        image_t **image,
	        pixmap_t pixmap
	)
	void VSDrawUpdateNetstatsLabel(
	        image_t **image,
	        pixmap_t pixmap
	)

  	---


 */

#include "../include/unvmath.h"

#include "blitting.h"
#include "xsw.h"
#include "net.h"


#define MIN(a,b)	((a) < (b) ? (a) : (b))
#define MAX(a,b)	((a) > (b) ? (a) : (b))


/*
 *	Global glow object pointer, for use in function
 *	VSDrawViewscreenBackground().  It is reset
 *	to NULL in function VSDrawViewScreen().
 *
 *	The object can be considered valid and non-garbage if
 *	the pointer is not NULL.
 */
static xsw_object_struct *glow_obj_ptr;


/*
 *	Draws a scanner display on buffer w_buf and puts that on window w.
 */
void ScannerDraw(
	int object_num,		/* Relative to this object */
	win_t w,
        shared_image_t *image,
	int win_x, int win_y,	/* Translation of scanner. */
	double scan_scale	/* Multiply all coordinates by this */
)
{
	int i;
	char camera_in_nebula = 0;
	unsigned int width, height, longest_dim;
	double rel_x, rel_y;
	double x, y;
	double rx, ry;
	double sintheta, costheta;
	double scanner_range;
	double scanner_factor;	/* XSW Real units to scanner units factor. */

	xsw_object_struct	*camera_obj_ptr,
				*locked_obj_ptr,
				*obj_ptr, **ptr;
	image_t *img_ptr;
	WColorStruct color;
	osw_gui_struct *gui;


	gui = &osw_gui[0];

	/* Error checks. */
	if(!IDC() ||
           (w == 0) ||
	   (image == NULL)
	)
	    return;


	/* Get widths and heights. */
	width = image->width;
	height = image->height;

	longest_dim = MAX(width, height);


	/* ************************************************************* */

#if defined(X_H) && defined(USE_XSHM)
        /* Is the shared image currently being `put'? */
        if(image->in_progress && gui->def_use_shm)
        {
            /* Do not continue with viewscreen draw procedure,
             * BridgeManage() will set the viewscreen image's
             * in_progress back to False whe the shared image put
             * completed.
             */
            return;
        }
#endif  /* defined(X_H) && defined(USE_XSHM) */


	/* Draw scanner background. */
	if(IMGIsImageNumLoaded(IMG_CODE_SCANNER))
	{
	    img_ptr = xsw_image[IMG_CODE_SCANNER]->image;
	    BlitBufAbsolute(
		gui->depth,
		image->data,
		reinterpret_cast<u_int8_t *>(img_ptr->data),
		0, 0,
		image->width, image->height,
		0, 0,
		img_ptr->width, img_ptr->height,
		img_ptr->width, img_ptr->height,
		1.0,	/* Zoom. */
		1.0	/* Magnification. */
	    );
	}


	/* Is object valid? */
	if(DBIsObjectGarbage(object_num))
	{
	    OSWPutSharedImageToDrawable(image, w);
	    return;
	}
	else
	{
	    /* Get pointer to camera object and what it is locked on. */
	    camera_obj_ptr = xsw_object[object_num];

	    if(DBIsObjectGarbage(camera_obj_ptr->locked_on))
		locked_obj_ptr = NULL;
	    else
		locked_obj_ptr = xsw_object[camera_obj_ptr->locked_on];
	}


	/* Sanitize scan_scale. */
	if(scan_scale > 1)
	    scan_scale = 1;
	else if(scan_scale < 0.001)
	    scan_scale = 0.001;

	/* Get effective scanner range. */
	scanner_range = MAX(camera_obj_ptr->scanner_range, 0);
	if(option.scanner_limiting)
	{
	    if(camera_obj_ptr->loc_type == XSW_LOC_TYPE_NEBULA)
	    {
	        scanner_range = scanner_range * VISIBILITY_NEBULA;
		camera_in_nebula = 1;
	    }
	}

	/* Get scanner_factor. */
	if(scanner_range > 0)
	    scanner_factor = ((double)((int)longest_dim / 2) /
		scanner_range) / scan_scale;
	else
            scanner_factor = (double)((int)longest_dim / 2) /
		scan_scale;


	/* Sanitize scanner_factor. */
	if(scanner_factor < 0)
	   scanner_factor = 0.01;


	/* *********************************************************** */

	/* Draw visual range circle. */
	rx = (double)bridge_win.viewscreen_width / 2000 /
	    bridge_win.viewscreen_zoom * scanner_factor;
	if(rx > 5)
	{
            color.r = 0xf0;
            color.g = 0x90;
            color.b = 0xda;
	    BlitBufCircle(
		gui->depth,
		image->data,
		win_x + ((int)width / 2),
		win_y + ((int)height / 2),
		width, height,
		1,
		rx,
		color
	    );
	}

	/* Draw max range circle. */
	rx = (double)longest_dim / 2 / scan_scale;
        if(rx < (int)longest_dim)
        {
            color.r = 0x88;
            color.g = 0x30;
            color.b = 0x86;
            BlitBufCircle(
                gui->depth,
                image->data,
                win_x + ((int)width / 2),
                win_y + ((int)height / 2),
                width, height,
                1,
                rx,
                color
            );  
        }

        /* Draw half range circle. */
        rx = (double)longest_dim / 4 / scan_scale;
        if(rx < (int)longest_dim)
        {
            color.r = 0x70;
            color.g = 0x30;
            color.b = 0x78;
            BlitBufCircle(
                gui->depth,
                image->data,
                win_x + ((int)width / 2), 
                win_y + ((int)height / 2),
                width, height,
                1, 
                rx,  
                color
            );
        }




	/* Draw scanning range label. */
	if(bridge_win.scanner_range_label != NULL)
	{
	    img_ptr = bridge_win.scanner_range_label;

	    BlitBufNormal(
		gui->depth,
		image->data,
		reinterpret_cast<u_int8_t *>(img_ptr->data),
		7, (int)height - 20,
		image->width, image->height,
		0, 0,
                img_ptr->width, img_ptr->height,
                img_ptr->width, img_ptr->height,
                1.0,	/* Zoom. */
		1.0,	/* Visibility. */
		1.0	/* Magnification. */
	    );
	}
	/* Draw location flags label. */
        if(bridge_win.scanner_loc_label != NULL)
        {
            img_ptr = bridge_win.scanner_loc_label;

            BlitBufNormal(
                gui->depth,
                image->data,
                reinterpret_cast<u_int8_t *>(img_ptr->data),
                2, 2,
                image->width, image->height,
                0, 0,
                img_ptr->width, img_ptr->height,
                img_ptr->width, img_ptr->height,
                1.0,	/* Zoom. */
                1.0,	/* Visibility. */
		1.0	/* Magnification. */
            );
        }


	/* Get relative position (in XSW real units). */
	rel_x = camera_obj_ptr->x;
	rel_y = camera_obj_ptr->y;


	/* Get sintheta and costheta. */
	sintheta = sin(camera_obj_ptr->heading);
	costheta = cos(camera_obj_ptr->heading);


	/* Begin drawing each object in range. */
	for(i = 0, ptr = inrange_xsw_object;
            i < total_inrange_objects;
	    i++, ptr++
	)
	{
	    obj_ptr = *ptr;
	    if(obj_ptr == NULL)
		continue;

	    /* Skip following types of objects. */
	    if((obj_ptr->type <= XSW_OBJ_TYPE_GARBAGE) ||
               (obj_ptr->type == XSW_OBJ_TYPE_STREAMWEAPON) ||
               (obj_ptr->type == XSW_OBJ_TYPE_SPHEREWEAPON)
	    )
	        continue;

            /* Skip outdated objects (except for camera). */
            if((obj_ptr->last_updated + OBJECT_OUTDATED_TIMEOUT)
               < cur_millitime
	    )
	    {
		if(obj_ptr != camera_obj_ptr)
                    continue;
	    }

	    /* Skip if obj is not in same sector as camera obj. */
	    if((obj_ptr->sect_x != camera_obj_ptr->sect_x) ||
               (obj_ptr->sect_y != camera_obj_ptr->sect_y) ||
               (obj_ptr->sect_z != camera_obj_ptr->sect_z)
	    )
		continue;

	    /* If object is in nebula and we are not, do not draw it. */
	    if((obj_ptr->type != XSW_OBJ_TYPE_AREA) &&
               (obj_ptr->loc_type == XSW_LOC_TYPE_NEBULA)
	    )
	    {
		if(!camera_in_nebula)
		    continue;
	    }


	    /* Is object in the scanner? */
	    x = obj_ptr->x - rel_x;
	    y = obj_ptr->y - rel_y;
	    if(Mu3DDistance(x, y, 0) >
               (scanner_range *
               DBGetObjectVisibilityPtr(obj_ptr))
	    )
		continue;


	    /* Change color and image depending on type of object. */
	    img_ptr = NULL;
	    if(locked_obj_ptr == obj_ptr)
	    {
		memcpy(&color, &xsw_color.scmark_locked_cv, sizeof(WColorStruct));
		if(IMGIsImageNumLoaded(IMG_CODE_SCMARK_LOCKED))
		    img_ptr = xsw_image[IMG_CODE_SCMARK_LOCKED]->image;
	    }
            else if(obj_ptr->type == XSW_OBJ_TYPE_WEAPON)
	    {
                memcpy(&color, &xsw_color.scmark_weapon_cv, sizeof(WColorStruct));
                if(IMGIsImageNumLoaded(IMG_CODE_SCMARK_WEAPON))
                    img_ptr = xsw_image[IMG_CODE_SCMARK_WEAPON]->image;
	    }
            else if((obj_ptr->type == XSW_OBJ_TYPE_HOME) ||
                    (obj_ptr->type == XSW_OBJ_TYPE_WORMHOLE) ||
                    (obj_ptr->type == XSW_OBJ_TYPE_ELINK)
            )
            {
                memcpy(&color, &xsw_color.scmark_home_cv, sizeof(WColorStruct));
                if(IMGIsImageNumLoaded(IMG_CODE_SCMARK_HOME))
                    img_ptr = xsw_image[IMG_CODE_SCMARK_HOME]->image;
            }
            else if(obj_ptr->type == XSW_OBJ_TYPE_AREA)  
            {
                memcpy(&color, &xsw_color.scmark_area_cv, sizeof(WColorStruct));
                if(IMGIsImageNumLoaded(IMG_CODE_SCMARK_AREA))
                    img_ptr = xsw_image[IMG_CODE_SCMARK_AREA]->image;
            }
	    else
	    {
		/* Default to static and other. */
                memcpy(&color, &xsw_color.scmark_unknown_cv, sizeof(WColorStruct));
                if(IMGIsImageNumLoaded(IMG_CODE_SCMARK_UNKNOWN))
                    img_ptr = xsw_image[IMG_CODE_SCMARK_UNKNOWN]->image;
	    }

	    /* Calculate x and y position to center of object. */
	    x = (obj_ptr->x - rel_x) * scanner_factor;
	    y = (obj_ptr->y - rel_y) * scanner_factor;

	    switch(bridge_win.scanner_orient)
	    {
              case SCANNER_ORIENT_LOCAL:
                rx = (costheta * x) + (sintheta * -1 * y);  
                ry = ((sintheta * x) + (costheta * y)) * -1;
                break;

	      default:	/* SCANNER_ORIENT_GC */
		rx = x;
		ry = y * -1;
	        break;
	    }

	    x = rx + (int)(width / 2);
	    x = x + win_x;
	    if(x > (win_x + (int)width))
		continue;
            if(x < win_x)
                continue;

            y = ry + (int)(height / 2);
            y = y + win_y;
            if(y > (win_y + (int)height))
                continue;
            if(y < win_y)
                continue;



	    /*   x and y are now center of object relative to
             *   object_num which is center of plane.
	     */


	    /* Draw the 'marker' for the object. */
	    if(img_ptr != NULL)
	    {
                BlitBufNormal(
                    gui->depth,
                    image->data,
                    reinterpret_cast<u_int8_t *>(img_ptr->data),
                    static_cast<int>(x - ((int)img_ptr->width / 2)),
		    static_cast<int>(y - ((int)img_ptr->height / 2)),
                    image->width, image->height,
                    0, 0,
                    img_ptr->width, img_ptr->height,
                    img_ptr->width, img_ptr->height,
                    1.0,	/* Zoom. */
                    1.0,
		    1.0		/* Magnification. */
                );
	    }

	   /* Don't draw line for the following objects. */
           if((obj_ptr->type == XSW_OBJ_TYPE_STATIC) ||
              (obj_ptr->type == XSW_OBJ_TYPE_HOME) ||
              (obj_ptr->type == XSW_OBJ_TYPE_AREA) ||
              (obj_ptr->type == XSW_OBJ_TYPE_ANIMATED) ||
              (obj_ptr->type == XSW_OBJ_TYPE_ELINK)
	   )
	      continue;

	   switch(bridge_win.scanner_orient)
	   {
             case SCANNER_ORIENT_LOCAL:
                BlitBufLine(
                    gui->depth,
                    (u_int8_t *)image->data,
                    static_cast<int>(x),
		    static_cast<int>(y),
                    image->width, image->height,
                    obj_ptr->heading -
			camera_obj_ptr->heading,
                    9,
                    1,
                    color
                );
                break;

	     default:	/* SCANNER_ORIENT_GC */
                BlitBufLine(
                    gui->depth,
                    (u_int8_t *)image->data,
                    static_cast<int>(x),
		    static_cast<int>(y),
                    image->width, image->height, 
                    obj_ptr->heading,
                    9,
                    1,
                    color
                );
		break;
	    }
	}


        /* Put shared image to window. */
#if defined(X_H) && defined(USE_XSHM)
        if(gui->def_use_shm)
        {
            XShmPutImage(
                gui->display,
                w,  
                gui->gc,
                image->ximage,
                0, 0,
                0, 0,  
                image->ximage->width, image->ximage->height,
                True
            );
            image->in_progress = True;
        }
        else
        {
            OSWPutSharedImageToDrawable(image, w);
        }
#else
        OSWPutSharedImageToDrawable(image, w);
#endif  /* defined(X_H) && defined(USE_XSHM) */


	return;
}


/*
 *	Destroys and recreates (updates) the scanner labels.
 *
 *	If object_num is garbage, then the labels will be destroyed.
 */
void ScannerUpdateLabels(int object_num)
{
	char text[128];
	double cur_range, max_range;
	xsw_object_struct *obj_ptr;
	osw_gui_struct *gui;


	gui = &osw_gui[0];

	/* Destroy scanner label images. */
	OSWDestroyImage(&bridge_win.scanner_range_label);
	OSWDestroyImage(&bridge_win.scanner_loc_label);


	/* Check if object is valid. */
	if(DBIsObjectGarbage(object_num))
	    return;
	else
	    obj_ptr = xsw_object[object_num];


        /* Scanner range label. */
	if(obj_ptr->loc_type == XSW_LOC_TYPE_NEBULA)
	{
	    /* Object is in a nebula. */
            cur_range = obj_ptr->scanner_range *
                bridge_win.scanner_zoom * VISIBILITY_NEBULA;
            max_range = obj_ptr->scanner_range * VISIBILITY_NEBULA;
	}
	else
	{
            /* Object is in normal space. */
            cur_range = obj_ptr->scanner_range *
                bridge_win.scanner_zoom;
            max_range = obj_ptr->scanner_range;
	}
        switch(option.units)
        {
          case XSW_UNITS_ENGLISH:
            cur_range = ConvertRUToAU(&sw_units, cur_range);
            max_range = ConvertRUToAU(&sw_units, max_range);
            if(cur_range < 10)
                sprintf(text, "%.4fau(%.2fau)",
                    cur_range, max_range
                );
            else
                sprintf(text, "%.2fau(%.2fau)",
                    cur_range, max_range
                );
            break;

          case XSW_UNITS_METRIC:
            cur_range = ConvertRUToAU(&sw_units, cur_range);
            max_range = ConvertRUToAU(&sw_units, max_range);
            if(cur_range < 10)
                sprintf(text, "%.4fau(%.2fau)",
                    cur_range, max_range
                );
            else
                sprintf(text, "%.2fau(%.2fau)",
                    cur_range, max_range
                );
	    break;

          default:
            sprintf(text, "%.2fru(%.2fru)",
                cur_range, max_range
            );
            break;
        }
        bridge_win.scanner_range_label = WidgetCreateImageText(
            text,
            xsw_font.console_standard,
            6, 10,
            xsw_color.bp_standard_text,
            gui->black_pix
        );


        /* Scanner location type label. */
        if(obj_ptr->loc_type == XSW_LOC_TYPE_NEBULA)
        {
            bridge_win.scanner_loc_label = WidgetCreateImageText(
                "(Nebula)",
                xsw_font.console_standard,
                6, 10,
                xsw_color.bp_standard_text,
                gui->black_pix
            );
	}
	else
	{
            bridge_win.scanner_loc_label = WidgetCreateImageText(
                NULL,
                xsw_font.console_standard,
                6, 10,
                xsw_color.bp_standard_text,
                gui->black_pix
            );
	}


	return;
}



/*
 *	Draws sector boundary ticks.
 */
void VSDrawSectorTicks(
	shared_image_t *image,
	double x_rel,		/* Offset in XSW real units. */
        double y_rel,
        int x_vtrans,		/* Velocity translation in pixels. */
        int y_vtrans
)
{
	unsigned int width, height;
	int half_width, half_height;
	double	sect_width_min,
		sect_width_max,
		sect_height_min,
		sect_height_max;

	WColorStruct color;
	double gamma;
	double x_pos, y_pos;


	/* Get widths and heights. */
	width = image->width;
	height = image->height;
	half_width = (int)width / 2;
	half_height = (int)height / 2;


	/* Get sector min and maxs in screen units. */
	sect_width_min = sector_legend.x_min;
	sect_width_max = sector_legend.x_max;
	sect_height_min = sector_legend.y_min;
	sect_height_max = sector_legend.y_max;



        /* Set foreground color of weapons lock. */
	if(genanim_timmer[ANIM_TIMMER_MEDIUMGLOW].count_max != 0)
	    gamma = (double)genanim_timmer[ANIM_TIMMER_MEDIUMGLOW].count /
                (double)genanim_timmer[ANIM_TIMMER_MEDIUMGLOW].count_max;
	else
	    gamma = 0.5;
        if(gamma > 0.5)
            gamma = 1 - gamma;
        gamma *= 2;


        /* Set color. */
        color.a = xsw_color.lock_cursor.a;
        color.r = static_cast<u_int8_t>(xsw_color.lock_cursor.r * gamma);
        color.g = static_cast<u_int8_t>(xsw_color.lock_cursor.g * gamma);
        color.b = static_cast<u_int8_t>(xsw_color.lock_cursor.b * gamma);

	/* X ticks. */
	x_pos = ((sect_width_min - x_rel) * 1000 * 
	    bridge_win.viewscreen_zoom) - x_vtrans + half_width;
	if((x_pos >= 0) && (x_pos < (int)width))
	{
            BlitBufLine(
                osw_gui[0].depth,
                image->data,
		static_cast<int>(x_pos), 25,
		image->width, image->height,
                0,
                25,         /* Size must be constant. */
                2,
                color
            );
	}

        x_pos = ((sect_width_max - x_rel) * 1000 *
            bridge_win.viewscreen_zoom) - x_vtrans + half_width;
        if((x_pos >= 0) && (x_pos < (int)width))
        {
            BlitBufLine(
                osw_gui[0].depth,
                image->data,
                static_cast<int>(x_pos), 25,
                image->width, image->height,
                0,
                25,         /* Size must be constant. */
                2,
                color
            );
	}

	/* Y ticks. */
        y_pos = (int)height - (
	    ((sect_height_min - y_rel) * 1000 *
            bridge_win.viewscreen_zoom) - y_vtrans + half_height
	);
        if((y_pos >= 0) && (y_pos < (int)height))
        {
            BlitBufLine(
                osw_gui[0].depth,
                image->data,
                0, static_cast<int>(y_pos),
                image->width, image->height,
                1.570796327,
                25,         /* Size must be constant. */
                2,
                color
            );
        }     

        y_pos = (int)height - (
            ((sect_height_max - y_rel) * 1000 *
            bridge_win.viewscreen_zoom) - y_vtrans + half_height
        );
        if((y_pos >= 0) && (y_pos < (int)height))
        {
            BlitBufLine(
                osw_gui[0].depth,
                image->data,
                0, static_cast<int>(y_pos),
                image->width, image->height,
                1.570796327,
                25,         /* Size must be constant. */
                2,
                color  
            );
        }



	return;
}


/*
 *	Draw scanner lock cursor over subject object tar_obj_ptr.
 */
void VSDrawLockCursor(
        xsw_object_struct *src_obj_ptr,
        xsw_object_struct *tar_obj_ptr,
        shared_image_t *image,
        double x_rel,
        double y_rel,
        double tar_size		/* In pixels. */
)
{
	WColorStruct color;
	image_t *src_image = NULL;
	double gamma;
	double screen_x, screen_y;	/* In pixels. */

        double theta1, theta2;
	double displace_len;


        /* Calculate gamma. */
        gamma = (double)genanim_timmer[ANIM_TIMMER_MEDIUMGLOW].count /
            (double)genanim_timmer[ANIM_TIMMER_MEDIUMGLOW].count_max;
        if(gamma > 0.5)
            gamma = 1 - gamma;
        gamma *= 2;
            
 
        /* Set color. */
        color.a = xsw_color.lock_cursor.a;
        color.r = static_cast<u_int8_t>(xsw_color.lock_cursor.r * gamma);
        color.g = static_cast<u_int8_t>(xsw_color.lock_cursor.g * gamma);
        color.b = static_cast<u_int8_t>(xsw_color.lock_cursor.b * gamma);


        screen_x = (double)image->width / 2;
        screen_y = (double)image->height / 2;


        /* Check if subject object is in range. */
	if(Mu3DInRangePtr(src_obj_ptr, tar_obj_ptr,
	    (screen_x / 800) / bridge_win.viewscreen_zoom)
	)
	{
	    /* Within range, draw cursor. */

	    /* Check which cursor to use. */
	    if((tar_obj_ptr->type == XSW_OBJ_TYPE_CONTROLLED) ||
               (tar_obj_ptr->type == XSW_OBJ_TYPE_PLAYER)
	    )
	    {
                if(IMGIsImageNumAllocated(IMG_CODE_VSMARK_VESSEL))
                    src_image = xsw_image[IMG_CODE_VSMARK_VESSEL]->image;
	    }
	    else
	    {
                if(IMGIsImageNumAllocated(IMG_CODE_VSMARK_OBJECT))
		    src_image = xsw_image[IMG_CODE_VSMARK_OBJECT]->image;
	    }
            if(src_image == NULL)
                return;

            /* Blit cursor. */
            BlitBufCursor(
                osw_gui[0].depth,
                image->data,
                reinterpret_cast<u_int8_t *>(src_image->data),
                static_cast<int>(x_rel - (int)((int)src_image->width / 2)),
                static_cast<int>(y_rel - (int)((int)src_image->height / 2)),
                image->width, image->height,
                src_image->width, src_image->height,
                color
            );
	}
	/* Not in scanner range, but check if in same sector. */
	else if(Mu3DInSameSectorPtr(tar_obj_ptr, src_obj_ptr))
	{
	    theta1 = MuCoordinateDeltaVector(
		tar_obj_ptr->x - src_obj_ptr->x,
		tar_obj_ptr->y - src_obj_ptr->y
	    );

            /* Get theta2. */
            theta2 = 6.2831853 - theta1;

            displace_len = MAX(
		((double)image->width * 0.40) *
                bridge_win.viewscreen_zoom,
		80
	    );

            /* Blit arrow lines. */
            BlitBufLine(
                osw_gui[0].depth,
                image->data,
                static_cast<int>(screen_x - MuPolarRotX(theta2, displace_len)),
                static_cast<int>(screen_y - MuPolarRotY(theta2, displace_len)),
                image->width, image->height,
                theta1 - 2.7,
                10,         /* Size must be constant. */
                2,
                color  
            );
            BlitBufLine(
                osw_gui[0].depth,
                image->data,
                static_cast<int>(screen_x - MuPolarRotX(theta2, displace_len)),
                static_cast<int>(screen_y - MuPolarRotY(theta2, displace_len)),
                image->width, image->height,
                theta1 + 2.7,
                10,     /* Size must be constant. */
                2,
                color
            );
	}


	return;
}



/*
 *	Draws stream weapons, including sphere weapons.
 */
void VSDrawStreamWeapon(
	xsw_object_struct *obj_ptr,
        shared_image_t *image,
        double x_rel,
        double y_rel
)
{
	WColorStruct *fg_color;


	if(obj_ptr == NULL)
	    return;

	if(obj_ptr->size <= 0)
	    return;

	/*
	 * Drawing Stream Weapons:
	 *
	 *   Objects of type XSW_OBJ_TYPE_STREAMWEAPON do not have imagesets,
	 *   instead they are drawn with Xlib draw commands (not pixmaps).
	 */
        switch(obj_ptr->imageset)
	{
	    /* Green beam. */
	    case ISREF_STREAMWEAPON_GREEN:
		fg_color = &(xsw_color.stream_green);
		break;

            /* Purple beam. */
            case ISREF_STREAMWEAPON_PURPLE:
                fg_color = &(xsw_color.stream_purple);
                break;

	    /* Orange beam. */
	    case ISREF_STREAMWEAPON_ORANGE:
                fg_color = &(xsw_color.stream_orange);
                break;

	    /* Default to Yellow Beam */
	    default:
		fg_color = &(xsw_color.stream_yellow);
		break;
	}

	/* Blit stream weapon. */
        if(obj_ptr->type == XSW_OBJ_TYPE_STREAMWEAPON)
	{
	    BlitBufBeam(
	        osw_gui[0].depth,
	        image->data,
	        static_cast<int>(x_rel),
	        static_cast<int>(y_rel),
	        image->width,
	        image->height,
	        obj_ptr->heading,
	        static_cast<int>(obj_ptr->size * bridge_win.viewscreen_zoom),
	        static_cast<int>(4 + (4 * bridge_win.viewscreen_zoom)),
	        *fg_color
	    );
	}
	/* Blit sphere weapon. */
	else if(obj_ptr->type == XSW_OBJ_TYPE_SPHEREWEAPON)
	{
            BlitBufCircle(
                osw_gui[0].depth,
                image->data,
                static_cast<int>(x_rel),
                static_cast<int>(y_rel),
                image->width,
                image->height,
                4,		/* Thickness. */
                obj_ptr->size * bridge_win.viewscreen_zoom,	/* Rad. */
                *fg_color
            );
	}


	return;
}


/*
 *	Draw heading and weapon aim arrows.
 */
void VSDrawHeadingArrow(
        shared_image_t *image,
        double arrow_heading,	/* In radians. */
        int x_center,
        int y_center
)
{
	depth_t depth;
	double r, theta, gamma;
	double sin_theta, cos_theta;
	WColorStruct c;
	xsw_genanim_timmer_struct *ani;


	depth = osw_gui[0].depth;
	ani = &genanim_timmer[ANIM_TIMMER_MEDIUMGLOW];

        /* Calculate and set color for heading arrow. */
	if(ani->count_max > 0)
	    gamma = (double)ani->count / (double)ani->count_max;
	else
	    gamma = 0;
	if(gamma > 0.5)
	    gamma = 1.5 - gamma;
	else
	    gamma += 0.5;

	c.a = xsw_color.heading_arrow.a;
	c.r = static_cast<u_int8_t>(xsw_color.heading_arrow.r * gamma);
	c.g = static_cast<u_int8_t>(xsw_color.heading_arrow.g * gamma);
	c.b = static_cast<u_int8_t>(xsw_color.heading_arrow.b * gamma);


	/* Calculate arrow heading as theta. */
	arrow_heading = SANITIZERADIANS(arrow_heading);
	theta = (2 * PI) - arrow_heading;

	/* Calculate radius from center to arrow in Screen units. */
	r = MAX(
	    ((double)image->width * 0.28) *
	    bridge_win.viewscreen_zoom,
	    50
	);

	/* Calculate trig scalar (not unit coefficients). */
	sin_theta = sin(theta) * r;
	cos_theta = cos(theta) * r;

	/* Draw heading arrow. */
	BlitBufLine(
	    depth,
	    image->data,
	    static_cast<int>(x_center - sin_theta),
	    static_cast<int>(y_center - cos_theta),
	    image->width, image->height,
	    arrow_heading - 2.7,
	    10,
	    2,
	    c
	);
        BlitBufLine(
            depth,
            image->data,
            static_cast<int>(x_center - sin_theta),
            static_cast<int>(y_center - cos_theta),
            image->width, image->height,
            arrow_heading + 2.7,
            10,     /* Size must be constant. */
            2,
            c
	);


	/* Draw weapon aim heading? */
	if(local_control.weapons_online)
	{
            c.a = xsw_color.bp_danger_cv.a;
            c.r = static_cast<u_int8_t>(xsw_color.bp_danger_cv.r * gamma);
            c.g = static_cast<u_int8_t>(xsw_color.bp_danger_cv.g * gamma);
            c.b = static_cast<u_int8_t>(xsw_color.bp_danger_cv.b * gamma);

	    /* Recalculate arrow heading. */
            arrow_heading = SANITIZERADIANS(
		arrow_heading +
		local_control.weapon_fire_heading
	    );
            theta = (2 * PI) - arrow_heading;

            /* Calculate radius from center to arrow in Screen units. */
            r = MAX(
                ((double)image->width * 0.32) *
                bridge_win.viewscreen_zoom,
                50
            );


            /* Calculate trig scalar (not unit coefficients). */
            sin_theta = sin(theta) * r;
            cos_theta = cos(theta) * r;

            /* Draw heading arrow. */
            BlitBufLine(
                depth,
                image->data,
                static_cast<int>(x_center - sin_theta),
                static_cast<int>(y_center - cos_theta),
                image->width, image->height,
                arrow_heading - 2.7,
                10,
                2,
                c
            );
            BlitBufLine(
                depth,
                image->data,
                static_cast<int>(x_center - sin_theta),
                static_cast<int>(y_center - cos_theta),
                image->width, image->height,
                arrow_heading + 2.7,
                10,     /* Size must be constant. */
                2,
                c
            );
	}

	/* Arrow heading is now innacurate. */


	return;
}


/*
 *	Draws `starfield' to the buffer in image.
 */
void VSDrawStarfield(
        shared_image_t *image,
        double x_rel,		/* In XSW real units. */
        double y_rel,
	int x_vtrans,		/* In pixels. */
	int y_vtrans,
	double zoom
)
{
	depth_t depth;
        long x, y, mult1, mult2, mod2;
        unsigned int layer_width, layer_height;
        unsigned int width, height;

        /* mult1, mult2, mod2 and the two magic arrays (below) are used
         * to scatter the stars a little bit.  Originally, this function
         * didn't use iteration, so each star drawing was written out.
         * this was changed to make the code more manageable, and easier to
         * optimize.  To understand the function, watch these variables(esp.),
         * and how they change during each iteration.
         */

        long x_offset, y_offset;
        int mod, i, j, k = 3;
        double layer_multiple, z;

        WColorStruct color;

/* NUM_STARS_DRAWN */ 
        static int magic_x[] = {0, 1540, 320, 712, 10,
                2000, 3729,  395, 2350, 6900, 8129, 6275, 4900, 20};

        static int magic_y[] = {0, 1240, 2040, 1080, 250,
                4250, 3258, 3572, 5533, 1450, 1180, 4572, 450,  3250}; 


	/* Set depth. */
	depth = osw_gui[0].depth;

        /* Set color. */
        color.a = 0x00;
        color.r = 0xff;
        color.g = 0xff;
        color.b = 0xff; 

        /* Reset values. */
        mod = 0;
        mult1 = 6;
        mult2 = 2;
        mod2 = 2;
        z = zoom;


        /* ******************************************************** */ 

	/* Get widths and heights. */
	width = image->width;
	height = image->height;

         
        /* Set offsets. */
        x_offset = static_cast<long int>(((z / VS_ZOOM_MIN) - 1) * (double)(width / 2));
        y_offset = static_cast<long int>(((z / VS_ZOOM_MIN) - 1) * (double)(height / 2));


        /* recalculate x_rel and y_rel */
	x_rel = (x_rel * 1000 * z) - x_vtrans;
        y_rel = (y_rel * 1000 * z) + y_vtrans;


        /* Begin drawing stars. */
        for(j = 0; j <= 2; j++)
	{
	    layer_multiple = mult1 * mult2 * z;
/*          if(layer_multiple <= 0) */
            if(layer_multiple < 0.01)
                layer_multiple = 0.01;

            layer_width = (unsigned int)((double)width * layer_multiple);
            layer_height = (unsigned int)((double)height * layer_multiple);

            for(i = 0; i <= k; i++)
	    {
                x = static_cast<long int>((long)x_rel + ((double)magic_x[i + mod] * z));
                y = static_cast<long int>((long)y_rel + ((double)magic_y[i + mod] * z));

                x %= (long)layer_width;   
                y %= (long)layer_height;
                if(x < 0)
                    x += (long)layer_width;
                if(y < 0)
                    y += (long)layer_height;

                x = (long)((double)((long)layer_width - x) / mod2) - x_offset;
                y = (long)(y / mod2) - y_offset;

                /* Draw the `star'. */
                BlitBufPointLight(
                    depth,
                    image->data,
                    x, y,
                    width, height,
                    1,		/* Radius. */
                    color	/* Color. */
                );
            }

            mod2 *= 2;
            if(k == 3)
                k = 4;

            if(j == 0)
	    {
                mult2 += 2;
                mod = 4;
            }
            else if(j == 1)
	    {
                mult1 += 2;
                mod += 5;
            }
        }
        return;
}


/*
 *	Draws the background with respect to the camera object.
 *	If camera object is NULL then background will just be cleared.
 */
void VSDrawViewscreenBackground(
	xsw_object_struct *camera_obj_ptr,	/* Center to object. */
        shared_image_t *image,
        double x_rel,		/* Offset in XSW real units. */
        double y_rel,
        int x_vtrans,		/* Velocity translation in pixels. */
        int y_vtrans,
	double zoom
)
{
        int i, isref_num;
        unsigned int width, height;

	xsw_animation_struct *ani_ptr;
	unsigned int longest_dim;	/* Bigger of width or height. */
	isref_struct *isref_ptr;
	xsw_object_struct **obj_ptr;
	xsw_object_struct *tile_obj_ptr = NULL;

	double d = 0;
	double glow_obj_distance = 0;

	double coeff;
	WColorStruct color;


	/* Error checks. */
	if(image == NULL)
	    return;


	/* Get widths and heights. */
	width = image->width;
	height = image->height;


	/* If camera_obj_ptr is NULL then just clear background. */
	if(camera_obj_ptr == NULL)
	{
            color.a = 0x00;
            color.r = 0x00;
            color.g = 0x00;
            color.b = 0x00;

	    BlitBufSolid(
		osw_gui[0].depth,
		image->data,
		image->width,
		image->height,
		color
	    );

	    return;
	}

	/* Get longest dimension. */
	longest_dim = ((width > height) ? width : height);


	/* ************************************************************ */
	/*   Search for objects in the inrange list that have their
         *   isrefs display in background.
         */
	for(i = 0, obj_ptr = inrange_xsw_object;
	    i < total_inrange_objects;
	    i++, obj_ptr++
	)
	{
	    if(*obj_ptr == NULL)
		continue;
	    if((*obj_ptr)->type <= XSW_OBJ_TYPE_GARBAGE)
		continue;

	    /* Are objects valid and in the same sector? */
	    if((camera_obj_ptr->sect_x != (*obj_ptr)->sect_x) ||
               (camera_obj_ptr->sect_y != (*obj_ptr)->sect_y) ||
               (camera_obj_ptr->sect_z != (*obj_ptr)->sect_z)
	    )
		continue;

	    /* Skip objects with unloaded image sets. */
	    isref_num = (*obj_ptr)->imageset;
	    if(!ISRefIsLoaded(isref_num))
		continue;
	    else
		isref_ptr = isref[isref_num];


	    /* Background layer placement tiled? */
            if(isref_ptr->layer_placement == ISREF_LAYER_BG_TILED)
            {
		if(Mu3DInContactPtr(camera_obj_ptr, *obj_ptr))
		    tile_obj_ptr = *obj_ptr;
            }
	    /* Special effects: Star glow? */
	    if(isref_ptr->effects & ISREF_EFFECTS_STARGLOW)
	    {
		/* Get distance appart in screen units. */
                d = Mu3DDistance(
                    (*obj_ptr)->x - camera_obj_ptr->x,
                    (*obj_ptr)->y - camera_obj_ptr->y,
                    (*obj_ptr)->z - camera_obj_ptr->z
                    ) * 1000 * zoom
                ;
                /* Within sight? */
                if(d <= longest_dim)
                {
		    /*   Set this object to be the `glow object'
		     *   (main light source).  This object is valid,
		     *   so therefore later referances to glow_obj_ptr
		     *   can be assured the object is valid
		     *   untill the VSDrawViewscreen() function scope
		     *   has finished execution.
		     */
		    glow_obj_ptr = *obj_ptr;
		    glow_obj_distance = d;
		}
	    }
            /* Special effects: Fade in? */ 
            if(isref_ptr->effects & ISREF_EFFECTS_FADEINGLOW)
            {
                /* Get distance appart in screen units. */
                d = Mu3DDistance(
                    (*obj_ptr)->x - camera_obj_ptr->x,
                    (*obj_ptr)->y - camera_obj_ptr->y,
                    (*obj_ptr)->z - camera_obj_ptr->z
                    ) * 1000 * zoom
                ;
                /* Within sight? */
                if(d <= longest_dim)
                {
                    glow_obj_ptr = *obj_ptr;
		    ani_ptr = &glow_obj_ptr->animation;

		    /* Adjust distance based on animation frame. */
                    if(ani_ptr->total_frames != 0)
                        glow_obj_distance = d + (longest_dim *
                            (double)(ani_ptr->total_frames -
                                ani_ptr->current_frame) /
                            (double)ani_ptr->total_frames);
                    else
                        glow_obj_distance = d;
                }
            }
            /* Special effects: Fade out? */   
            if(isref_ptr->effects & ISREF_EFFECTS_FADEOUTGLOW)
            {
                /* Get distance appart in screen units. */
                d = Mu3DDistance(
                    (*obj_ptr)->x - camera_obj_ptr->x,
                    (*obj_ptr)->y - camera_obj_ptr->y,
                    (*obj_ptr)->z - camera_obj_ptr->z
                    ) * 1000 * zoom
                ;
                /* Within sight? */
                if(d <= longest_dim)
                {
                    glow_obj_ptr = *obj_ptr;
                    ani_ptr = &glow_obj_ptr->animation;

                    if(ani_ptr->total_frames != 0)
                        glow_obj_distance = d + (longest_dim *
                            (double)(ani_ptr->current_frame + 1) /
                            (double)ani_ptr->total_frames);
                    else
                        glow_obj_distance = d;
                }
            }
	}


	/* **************************************************** */
	/* Draw background by style. */

        /* Nebula tile background. */
        if(tile_obj_ptr != NULL)
        {
            /* Load tile imageset as needed. */
            isref_num = tile_obj_ptr->imageset;
            if(!ISRefIsLoaded(isref_num))
            {
                if(ISRefLoad(isref_num))
		    return;
            }

            /* Get pointer to isref. */
            isref_ptr = isref[isref_num];

            /* Draw tiled nebula background. */
            BlitBufTile(
                image->depth,
		image->data,
                isref_ptr->image_data,
                static_cast<int>((camera_obj_ptr->x * 1000 * zoom) - x_vtrans),
                static_cast<int>((camera_obj_ptr->y * 1000 * zoom) - y_vtrans),
                width, height,
                isref_ptr->width,	/* Entire isref size. */
                isref_ptr->height,
                zoom,				/* Zoom. */
		isref_ptr->magnification	/* Magnification. */
            );

	    /* Glow object in sight too? */
	    if((glow_obj_ptr != NULL) &&
	       option.show_nebula_glow
	    )
	    {
                /* Calculate glow coefficient. */
                if(longest_dim != 0)
                    coeff = MAX(
			(double)((double)longest_dim - glow_obj_distance) /
                        (double)longest_dim,
			0
		    );
                else
                    coeff = 1;

		color.a = 0x00;
		color.r = static_cast<u_int8_t>(coeff * xsw_color.star_glow.r);
                color.g = static_cast<u_int8_t>(coeff * xsw_color.star_glow.g);
                color.b = static_cast<u_int8_t>(coeff * xsw_color.star_glow.b);

		BlitBufGlow(
                    osw_gui[0].depth,
                    image->data,
		    0, 0,
                    image->width, image->height,
		    image->width, image->height,
                    color
		);
	    }
        }
        /* Glow object. */
        else if(glow_obj_ptr != NULL)
        {
            /* Calculate colors. */
	    if(longest_dim != 0)
	        coeff = MAX((double)((double)longest_dim - glow_obj_distance) /
	            (double)longest_dim, 0);
	    else
		coeff = 1;

            color.a = xsw_color.star_glow.a;
            color.r = static_cast<u_int8_t>(xsw_color.star_glow.r * coeff);
            color.g = static_cast<u_int8_t>(xsw_color.star_glow.g * coeff);
            color.b = static_cast<u_int8_t>(xsw_color.star_glow.b * coeff);

            /*   Draw solid background of glow color, this will clear
	     *   the background and set its glow in one sweep.
	     */
            BlitBufSolid(
                osw_gui[0].depth,
                image->data,
                image->width,
                image->height,
                color
            );

            /* Draw starfield. */
            VSDrawStarfield(
                image,
                x_rel,
                y_rel,
                x_vtrans,
                y_vtrans,
		zoom
            );
        }
        /* All else draw standard background. */
        else
        {
            color.a = 0x00;
            color.r = 0x00;
            color.g = 0x00;
            color.b = 0x00;

            /* Draw normal black space background. */
            BlitBufSolid(
                osw_gui[0].depth,
                image->data,
                image->width,
                image->height,
                color
            );

            /* Draw starfield. */
            VSDrawStarfield(
                image,
                x_rel,
                y_rel,
                x_vtrans,
                y_vtrans,
		zoom
            );
        }

	return;
}

/*
 *	Front end to draw an object.
 *
 *	Procedure to draw standard object.  Checks if object is
 *	special in any way that it needs to be drawn differently
 *	and calls appropriate function.
 *
 *	Both player_obj_ptr and obj_ptr must be valid.
 */
void VSDrawStandardObject(
	xsw_object_struct *player_obj_ptr,
	xsw_object_struct *obj_ptr,
        shared_image_t *image,
        double x_rel,		/* In XSW real units. */
        double y_rel,
	int x_vtrans,		/* In units of pixels. */
        int y_vtrans,
	double zoom
)
{
	int i, half_width, half_height;
	unsigned int width, height;
        int frame_num = 0;
        double x, y, distance;
	double vis = 1;
        double theta;
	double gamma;
	double delta_x, delta_y, delta_z;

	/*   Upper-left corner position of object image relative to
	 *   viewscreen image (in units of pixels).
	 */
	double	x_fpos,
		y_fpos;

	/*   Center position of object image relative to the upper-left
         *   corner of viewscreen image (in units of pixels).
	 */
	double	x_cfpos,
                y_cfpos;

	char is_strobe;
	int isref_num, label_num;
	image_t *img_ptr;
        xsw_object_struct *tar_obj_ptr;
	isref_struct *isref_ptr;
	vs_object_label_struct *label_ptr;
	isref_point_light_struct **point_light_ptr;
	WColorStruct color;


        /* Skip outdated objects or objects with too low of a
         * visibility (except for the player object).
         */
	if(obj_ptr != player_obj_ptr)
	{
            if((obj_ptr->last_updated + OBJECT_OUTDATED_TIMEOUT) <=
		cur_millitime
	    )
                return;

	    /* Check if `virtually' invisible. */
	    if(DBGetObjectVisibilityPtr(obj_ptr) <= 0.05)
		return;
	}


        /* Get widths and heights (image is assumed valid. */
        width = image->width;
        height = image->height;

	half_width = (int)width / 2;
	half_height = (int)height / 2;


	/* Get isref number and pointer, load as needed. */
	isref_num = obj_ptr->imageset;
	if(ISRefIsAllocated(isref_num))
	{
	    isref_ptr = isref[isref_num];

	    /* Load image as needed. */
	    if(isref_ptr->option & ISREF_OPT_NO_IMAGE)
	    {
		/* Isref has no image (possibly a stream weapon). */
		isref_ptr = NULL;
	    }
	    else if(!ISRefIsLoaded(isref_num))
	    {
                if(ISRefLoad(isref_num))
                    return;
	    }
	}
	else
	{
	    return;
	}


	/*   Calculate position relative from the upper left corner
	 *   of the viewscreen image to the center of object
         *   (in units of pixels).
	 */
	x_cfpos = ((obj_ptr->x - x_rel) * 1000 * zoom) -
            x_vtrans + half_width;

	y_cfpos = (int)height - (
	    ((obj_ptr->y - y_rel) * 1000 * zoom) -
            y_vtrans + half_height
	);


        /*   Check if the object is way off the view screen.
         *   Do *not* take into account velocity translation
         *   x_vtrans and y_vtrans distance is in XSW Screen units.
         */
        distance = Mu3DDistance(
            x_cfpos - (width / 2),
            y_cfpos - (height / 2),
            0
        );
/*   May need to be increased for stream weapons,
 *   since they have a long range from their center.
 */
        if(distance > ((width + height) * 2))
            return;


        /* Calculate x_fpos, y_fpos and then get frame_num. */
	if(isref_ptr == NULL)
	{
            x_fpos = x_cfpos;
            y_fpos = y_cfpos;
	}
	else
	{
	    x_fpos = x_cfpos -
	        ((isref_ptr->fwidth * isref_ptr->magnification) *
                zoom / 2);

	    y_fpos = y_cfpos -
                ((isref_ptr->fheight * isref_ptr->magnification) *
                zoom / 2);


	    /* Get frame_num. */
	    switch(isref_ptr->frame_determinant)
	    {
	      /* By heading. */
	      case ISREF_FDETERMINE_BY_HEADING:
	        theta = 0.19634954;
	        for(frame_num = 0;
		    frame_num < (int)isref_ptr->total_frames;
                    frame_num++
	        )
                {
                    if(obj_ptr->heading < theta)
                        break;
		    else
                        theta += 0.39269908;
                }
	        break;

	      /* By animated frame number. */
	      case ISREF_FDETERMINE_BY_ANIMATION:
	        frame_num = obj_ptr->animation.current_frame;
	        break;

	      /* Everything else defaults to frame_num 0. */
	      default:
                frame_num = 0;
	        break;
	    }

            /* Sanitize frame_num (yes this is needed). */
	    if(frame_num < 0)
	        frame_num = 0;
            if(frame_num >= (int)isref_ptr->total_frames)
                frame_num = 0;
	}


	/* ************************************************************* */

	/* Draw stream or sphere weapon. */
        if((obj_ptr->type == XSW_OBJ_TYPE_STREAMWEAPON) ||
           (obj_ptr->type == XSW_OBJ_TYPE_SPHEREWEAPON)
	)
        {
	    /* Draw stream weapon. */
	    VSDrawStreamWeapon(
		obj_ptr,
		image,
		x_cfpos, y_cfpos
	    );

	    /* Do not continue further with stream weapons. */
	    return;
        }
	/* Draw standard image. */
        else if(isref_ptr != NULL)
        {
	    /* Draw by merge mode. */
	    switch(isref_ptr->merge_mode)
	    {
	      /* Additive merge. */
	      case ISREF_MERGE_ADDITIVE:
		BlitBufAdditive(
		    osw_gui[0].depth,
                    image->data,
                    isref_ptr->image_data,
                    static_cast<int>(x_fpos),
		    static_cast<int>(y_fpos),
                    image->width, image->height,
                    0, frame_num * isref_ptr->fheight,
                    isref_ptr->width, isref_ptr->height, 
                    isref_ptr->fwidth, isref_ptr->fheight,
		    zoom,
		    isref_ptr->magnification
                );
		break;

	      /* Normal merge. */
	      default:
		/* Need to calculate visibility a bit non-standardly,
		 * so we do not use DBGetObjectVisibityPtr().
		 */
		vis = ((obj_ptr->visibility <= 0) ? 0 :
                    (obj_ptr->cur_visibility / obj_ptr->visibility)
		);
		if(vis >= 0.9)
		    vis = 1;

		if(obj_ptr->loc_type == XSW_LOC_TYPE_NEBULA)
		    vis = vis * zoom / 2;

                BlitBufNormal(
                    osw_gui[0].depth,
                    image->data,
                    isref_ptr->image_data,
                    static_cast<int>(x_fpos),
		    static_cast<int>(y_fpos),
                    image->width, image->height,
                    0, frame_num * isref_ptr->fheight,
                    isref_ptr->width, isref_ptr->height,  
                    isref_ptr->fwidth, isref_ptr->fheight,
                    zoom,
                    vis,
                    isref_ptr->magnification
                );
		break;
	    }


	    /* Draw marker over object if its visibility is too low.
	     * This will not reveil other objects besides the player
	     * since that is checked for at the top of this function.
	     */
	    if((vis <= 0.5) && (obj_ptr == player_obj_ptr))
            {
                /* Draw circle over object, so we can see where it is. */
                if(option.show_viewscreen_marks)
                {
                    /* Calculate visibility marker color. */
                    color.a = static_cast<u_int8_t>(xsw_color.visibility_marker.a * (1 - vis));
                    color.r = static_cast<u_int8_t>(xsw_color.visibility_marker.r * (1 - vis));
                    color.g = static_cast<u_int8_t>(xsw_color.visibility_marker.g * (1 - vis));
                    color.b = static_cast<u_int8_t>(xsw_color.visibility_marker.b * (1 - vis));

                    BlitBufCircle(
                        osw_gui[0].depth,
                        image->data,
                        static_cast<int>(x_cfpos),
			static_cast<int>(y_cfpos),
                        width, height,
                        1,
                        (double)(obj_ptr->size + 2) * zoom,
                        color
                    );
                }
	    }
        }

	/* Do not continue further if isref_ptr is NULL. */
	if(isref_ptr == NULL)
	    return;


	/* Draw tractor beam if object is tractoring another object. */
	if(obj_ptr->total_tractored_objects > 0)
	{
            /* Calculate gamma. */
            gamma = (double)genanim_timmer[ANIM_TIMMER_SHORTGLOW].count /
                (double)genanim_timmer[ANIM_TIMMER_SHORTGLOW].count_max;
            if(gamma > 0.5)
                gamma = 1 - gamma;
            gamma *= 2;

	    color.a = 0x00;
	    color.r = static_cast<u_int8_t>(0x10 * gamma);
	    color.g = static_cast<u_int8_t>(0x50 * gamma);
	    color.b = static_cast<u_int8_t>(0xff * gamma);


	    for(i = 0; i < obj_ptr->total_tractored_objects; i++)
	    {
		if(DBIsObjectGarbage(obj_ptr->tractored_object[i]))
		    continue;
		else
		    tar_obj_ptr = xsw_object[obj_ptr->tractored_object[i]];

		/* Check if object is valid and in range. */
		if(!Mu3DInRangePtr(
		    obj_ptr,
		    tar_obj_ptr,
		    2 * MAX_TRACTOR_BEAM_LEN
		))
		    continue;


		delta_x = tar_obj_ptr->x - obj_ptr->x;
		delta_y = tar_obj_ptr->y - obj_ptr->y;
		delta_z = tar_obj_ptr->z - obj_ptr->z;

		BlitBufBeam(
		    osw_gui[0].depth,
		    image->data,
		    static_cast<int>(x_cfpos),
		    static_cast<int>(y_cfpos),
		    width, height,
		    MuCoordinateDeltaVector(delta_x, delta_y),
		    static_cast<unsigned int>(Mu3DDistance(delta_x, delta_y, delta_z) *
			1000 * zoom),
		    static_cast<unsigned int>(6 + (6 * bridge_win.viewscreen_zoom)),
		    color
		);
	    }
	}

	/* Draw point lights (vector lights and strobes). */
	if(vis > 0.05)
	{
	    /* frame_num should already be calculated. */
            theta = frame_num * 0.39269908;

	    for(i = 0, point_light_ptr = isref_ptr->point_light;
                i < isref_ptr->total_point_lights;
                i++, point_light_ptr++
	    )
	    {
		if(*point_light_ptr == NULL)
		    continue;

		/* Is this point light a strobe? */
		if(((*point_light_ptr)->strobe_off_int > 0) &&
                   ((*point_light_ptr)->strobe_on_int > 0)
		)
		{
		    /* Object has strobes on? */
		    if(!(obj_ptr->lighting & XSW_OBJ_LT_STROBE))
			continue;

		    /* Strobe in off state? */
		    if(!((*point_light_ptr)->strobe_state))
		        continue;

		    is_strobe = 1;
		}
		else
		{
		    /* Not strobe, check if lights on object are on. */
                    if(!(obj_ptr->lighting & XSW_OBJ_LT_VECTOR))
                        continue;

		    is_strobe = 0;
		}

		/* Get color of light or strobe. */
		color.a = static_cast<u_int8_t>((*point_light_ptr)->a * vis);
                color.r = static_cast<u_int8_t>((*point_light_ptr)->r * vis);
                color.g = static_cast<u_int8_t>((*point_light_ptr)->g * vis);
                color.b = static_cast<u_int8_t>((*point_light_ptr)->b * vis);

		x = x_cfpos +
                    ((sin(SANITIZERADIANS(
                     (*point_light_ptr)->theta + theta)) *
                     (*point_light_ptr)->radius) *
                     zoom
                    );

                y = y_cfpos -
                    ((cos(SANITIZERADIANS(
                     (*point_light_ptr)->theta + theta)) *
                     (*point_light_ptr)->radius) *
                     zoom
		    );


		BlitBufPointLight(
		    osw_gui[0].depth,
		    image->data,
		    static_cast<int>(x),
		    static_cast<int>(y),
		    width, height,
		    1,
		    color
		);

		/* Blit strobe glow. */
		if(is_strobe)
		{
		    /* Is the object in a nebula? */
		    if((obj_ptr->loc_type == XSW_LOC_TYPE_NEBULA) &&
                       option.show_strobe_glow &&
                       option.show_nebula_glow
		    )
		    {
                      if(IMGIsImageNumAllocated(IMG_CODE_STROBEGLOW1))
                      {
                        img_ptr = xsw_image[IMG_CODE_STROBEGLOW1]->image;

                        BlitBufAdditive(
                            osw_gui[0].depth,
                            image->data,
                            reinterpret_cast<u_int8_t *>(img_ptr->data),
                            static_cast<int>(x - ((int)img_ptr->width / 2)),
                            static_cast<int>(y - ((int)img_ptr->height / 2)),
                            image->width, image->height,   
                            0, 0,
                            img_ptr->width, img_ptr->height,
                            img_ptr->width, img_ptr->height,
                            1,          /* Zoom. */
                            1           /* Magnification. */
                        );
                      }
		    }
		    else if(option.show_strobe_glow)
		    {
                      if(IMGIsImageNumAllocated(IMG_CODE_STROBEGLOW1))
		      {
			img_ptr = xsw_image[IMG_CODE_STROBEGLOW1]->image;

                        BlitBufAdditive(
                            osw_gui[0].depth,
                            image->data,
                            reinterpret_cast<u_int8_t *>(img_ptr->data),
			    static_cast<int>(x - ((int)img_ptr->width / 2)),
			    static_cast<int>(y - ((int)img_ptr->height / 2)),
                            image->width, image->height,
                            0, 0,
                            img_ptr->width, img_ptr->height,
                            img_ptr->width, img_ptr->height,
                            1,		/* Zoom. */
                            1		/* Magnification. */
                        );
		      }
		    }
		}	/* Blit strobe glow. */

	    }
	}


        /* Draw shield visibility. */
        if(obj_ptr->shield_visibility > 0.001)
        {
            /* Calculate shield color. */
            color.a = static_cast<u_int8_t>(xsw_color.shield_blue.a *
                obj_ptr->shield_visibility);
            color.r = static_cast<u_int8_t>(xsw_color.shield_blue.r *
                obj_ptr->shield_visibility);
            color.g = static_cast<u_int8_t>(xsw_color.shield_blue.g *
                obj_ptr->shield_visibility);
            color.b = static_cast<u_int8_t>(xsw_color.shield_blue.b *
                obj_ptr->shield_visibility);

            BlitBufCircle(
                osw_gui[0].depth,
                image->data,
		static_cast<int>(x_cfpos),
		static_cast<int>(y_cfpos),
                width, height,
                1,
                (double)(obj_ptr->size + 2) * zoom,
                color
            );
        }



        /* Draw viewscreen object name labels. */
        if((option.show_viewscreen_labels == 1) ||
           (option.show_viewscreen_labels == 3)
	)
	{
	    label_num = VSLabelGetByPointer(obj_ptr);
	    if((label_num >= 0) &&
               (obj_ptr->type != XSW_OBJ_TYPE_WEAPON) &&
               (obj_ptr->type != XSW_OBJ_TYPE_STREAMWEAPON) &&
               (obj_ptr->type != XSW_OBJ_TYPE_SPHEREWEAPON)
	    )
	    {
		label_ptr = vs_object_label[label_num];

		BlitBufNormal(
		    osw_gui[0].depth,
		    image->data,
		    reinterpret_cast<u_int8_t *>(label_ptr->image->data),
		    static_cast<int>(x_cfpos - (obj_ptr->size * zoom)),
		    static_cast<int>(y_cfpos - (obj_ptr->size * zoom)),
		    image->width, image->height,
		    0, 0,
                    label_ptr->image->width,
                    label_ptr->image->height,
		    label_ptr->image->width, 
                    label_ptr->image->height,
		    1.0,
		    1.0,
		    1.0
                );
            }
	}

	return;
}

/*
 *	Front end procedure to draw viewscreen.
 */
void VSDrawViewScreen(
	int camera_obj_num,
	win_t w,
	shared_image_t *image,
	double zoom
)
{
	int i, player_obj_num;
	int half_width, half_height;
	unsigned int width, height;
	double x, y, distance, size;
        double new_zoom;

	/* Offset to camera_obj_num (in XSW real units). */
	double x_rel, y_rel;

	/*   Translation caused by velocity of player object
	 *   (in units of pixels).
	 */
	int x_vtrans, y_vtrans;

	char still_incoming_fire;

	image_t *tmp_img_ptr;
	xsw_object_struct	*camera_obj_ptr,
				*locked_obj_ptr,
				*player_obj_ptr;
        xsw_object_struct	**obj_ptr;
	osw_gui_struct *gui;


	/* Get pointer to GUI structure. */
	gui = &osw_gui[0];

        /* Error checks. */
        if((w == 0) ||
	   (image == NULL)
	)
            return;

	/* Reset global glow_obj_ptr. */
	glow_obj_ptr = NULL;


	/* Get widths and heights of viewscreen image. */
	width = image->width;
	height = image->height;

	half_width = (int)width / 2;
	half_height = (int)height / 2;


	/*   If player object or camera object is invalid then
	 *   do not draw objects, draw either fade out or main menu.
	 */
	if((net_parms.player_obj_ptr == NULL) ||
           DBIsObjectGarbage(camera_obj_num)
	)
	{
	    /* Player object is garbage which implies that we arn't
             * connected.  Check if previous connection state was also
	     * not connected.
             */
	    if(net_parms.connection_state == CON_STATE_NOT_CONNECTED)
	    {
		/* Not connected. */

		/* Set fade out gamma to 0 if energy saver mode is on. */
		if((bridge_win.viewscreen_gamma > 0) &&
                   option.energy_saver_mode
		)
		    bridge_win.viewscreen_gamma = 0;

		/* Do fadeout as needed. */
		if(bridge_win.viewscreen_gamma > 0)
		{
                    /* Fade out. */
                    BlitBufFade(
                        gui->depth,
                        image->data,
                        0, 0,
                        image->width, image->height,
                        image->width, image->height,
                        bridge_win.viewscreen_gamma
                    );

                    /* Decrement viewscreen gamma (fade out). */
                    bridge_win.viewscreen_gamma -= MIN(
                        0.016 * time_compensation, 0.25
                    );

                    /* Draw energy saver mode label. */
                    if(option.energy_saver_mode)
                    {
                        if(IMGIsImageNumAllocated(IMG_CODE_ENERGY_SAVER_MODE))
                        {
                            tmp_img_ptr = xsw_image[IMG_CODE_ENERGY_SAVER_MODE]->image;
                                
                            BlitBufNormal(
                                gui->depth,
                                image->data,
                                reinterpret_cast<u_int8_t *>(tmp_img_ptr->data),
                 
                                half_width -
                                (int)(tmp_img_ptr->width / 2),
                                static_cast<int>((double)height * 0.6),
                 
                                width, height,
                                0, 0,
                                tmp_img_ptr->width,
                                tmp_img_ptr->height,
                                tmp_img_ptr->width,
                                tmp_img_ptr->height,
                                1.0,    /* Zoom. */
                                1.0,    /* Visibility. */
                                1.0     /* Magnification. */
                            );
                        }
                    }
   
                    OSWPutSharedImageToDrawable(image, w);
		}
		else
		{
		    /* Fade out has completed. */

		    /* Unmap viewscreen as needed. */
		    if(bridge_win.viewscreen_map_state)
			VSUnmap();

		    /* Map page. */
                    if(bridge_win.cur_page != NULL)
		    {
			if(!bridge_win.cur_page->map_state)
                            PageMap(bridge_win.cur_page, w, image);

			if(option.music)
			    XSWDoChangeBackgroundMusic();
		    }

/* Mapping page already puts to window.
                    OSWPutSharedImageToDrawable(image, w);
 */
                }
	    }

	    /* Return, nothing else to do. */
            return;
	}
	else
	{
	    /* Player and camera objects are valid. */

	    /* Get player object number. */
	    player_obj_num = net_parms.player_obj_num;

	    /* Get pointers to objects. */
	    player_obj_ptr = net_parms.player_obj_ptr;
	    camera_obj_ptr = xsw_object[camera_obj_num];

	    /* Get pointer to locked on object if available. */
	    i = player_obj_ptr->locked_on;
	    if(DBIsObjectGarbage(i))
		locked_obj_ptr = NULL;
	    else
		locked_obj_ptr = xsw_object[i];
	}


	/* Connected? */
	if(net_parms.connection_state == CON_STATE_CONNECTED)
	{
	    /* Unmap current page. */
	    if(bridge_win.cur_page != NULL)
	    {
	        PageUnmap(bridge_win.cur_page, w, image);

                /* Update background music when we unmap then main
		 * menu.
		 */
		if(option.music)
                    XSWDoChangeBackgroundMusic();
	    }

	    /* Map viewscreen as needed. */
	    if(!bridge_win.viewscreen_map_state)
	        VSMap();
	}


#if defined(X_H) && defined(USE_XSHM)
	/* Is the shared image currently being `put'? */
	if(image->in_progress && gui->def_use_shm)
	{
	    /* Do not continue with viewscreen draw procedure,
	     * BridgeManage() will set the viewscreen image's
	     * in_progress back to False whe the shared image put
	     * completed.
	     */
	    return;
	}
#endif	/* defined(X_H) && defined(USE_XSHM) */

        /* ********************************************************* */
	/* Viewscreen auto zoom, adjust the global
	 * bridge_win.viewscreen_zoom if option.auto_zoom is set.
	 */
	if(option.auto_zoom)
	{
	    /* Is locked object valid and in same sector? */
	    if(Mu3DInSameSectorPtr(camera_obj_ptr, locked_obj_ptr))
	    {
		distance = Mu3DDistance(
		    locked_obj_ptr->x - camera_obj_ptr->x,
                    locked_obj_ptr->y - camera_obj_ptr->y,
                    locked_obj_ptr->z - camera_obj_ptr->z
		) + ((double)locked_obj_ptr->size / 1000);

		if(distance <= camera_obj_ptr->scanner_range)
		{
		    /* Calculate new zoom factor. */
		    new_zoom = ((double)image->width / 2500) / distance;

		    if(((new_zoom - bridge_win.viewscreen_zoom) > VS_ZOOM_INC) ||
                       ((bridge_win.viewscreen_zoom - new_zoom) > VS_ZOOM_INC)
		    )
		    {
			if(bridge_win.viewscreen_zoom > new_zoom)
			{
			    bridge_win.viewscreen_zoom -= VS_ZOOM_INC *
				time_compensation;
			    if(bridge_win.viewscreen_zoom < new_zoom)
				bridge_win.viewscreen_zoom = new_zoom;
			}
			else
			{
			    bridge_win.viewscreen_zoom += VS_ZOOM_INC *
				time_compensation;
                            if(bridge_win.viewscreen_zoom > new_zoom)
                                bridge_win.viewscreen_zoom = new_zoom;
			}
		    }
		    else
		    {
			bridge_win.viewscreen_zoom = new_zoom;
		    }

		    /* Sanitize zoom. */
                    if(bridge_win.viewscreen_zoom > VS_ZOOM_MAX)
                        bridge_win.viewscreen_zoom = VS_ZOOM_MAX;
		    if(bridge_win.viewscreen_zoom < VS_ZOOM_MIN)
			bridge_win.viewscreen_zoom = VS_ZOOM_MIN;
		}
	    }
	    else
	    {
		/* Do not adjust zoom if nothing locked on. */

	    }
	}

	/* Need to reget zoom if it was changed above. */
	zoom = bridge_win.viewscreen_zoom;


	/* ********************************************************* */

	/*   Get relative values to be applied to all object
	 *   coordiates being drawn (in XSW real units).
	 */
        x_rel = camera_obj_ptr->x;
        y_rel = camera_obj_ptr->y;

	/*   Get velocity translation to be applied to all object
	 *   coordinates (in units of pixels).
	 */
	if(camera_obj_ptr->velocity_max > 0)
	{
	    /* x_vtrans */
	    x_vtrans = static_cast<int>(MuPolarRotX(
		camera_obj_ptr->velocity_heading,
		camera_obj_ptr->velocity /
		camera_obj_ptr->velocity_max *
		(double)((int)width / 2.5)
	    ));
            /* y_vtrans */
            y_vtrans = static_cast<int>(MuPolarRotY(
                camera_obj_ptr->velocity_heading,
                camera_obj_ptr->velocity /
                camera_obj_ptr->velocity_max *
		(double)((int)height / 2.5)
	    ));
	}
	else
	{
	    x_vtrans = 0;
	    y_vtrans = 0;
	}


	/* Draw background. */
	VSDrawViewscreenBackground(
            camera_obj_ptr,
            image,
	    x_rel,
	    y_rel,
	    x_vtrans,
	    y_vtrans,
	    zoom
	);


	/* ********************************************************** */

	/*   Draw home and area objects first then other objects.
	 *   Note that the inrange objects pointers are already
	 *   sorted to the above criteria.
	 */
	still_incoming_fire = 0;

	for(i = 0, obj_ptr = inrange_xsw_object;
	    i < total_inrange_objects;
	    i++, obj_ptr++
	)
	{
            /* Skip error, garbage, and area objects. */
            if(*obj_ptr == NULL)
		continue;
	    if((*obj_ptr)->type <= XSW_OBJ_TYPE_GARBAGE)
		continue;
            if((*obj_ptr)->type == XSW_OBJ_TYPE_AREA)
                continue;


	    /* Do incoming weapons fire check here. */
	    if(warning.incoming_fire)
	    {
                /* Projectile weapons check. */
                if(((*obj_ptr)->type == XSW_OBJ_TYPE_WEAPON) &&
                   ((*obj_ptr)->locked_on > -1)
                )
                {
                    if((*obj_ptr)->locked_on == player_obj_num)
                        still_incoming_fire = 1;
                }
	    }
	    else
	    {
		/* Projectile weapons check. */
		if(((*obj_ptr)->type == XSW_OBJ_TYPE_WEAPON) &&
                   ((*obj_ptr)->locked_on > -1)
		)
		{
		    if((*obj_ptr)->locked_on == player_obj_num)
		    {
			warning.incoming_fire = 1;
			still_incoming_fire = 1;

/* Make warning sound? */

		    }
		}
	    }


	    /* Draw object. */
            VSDrawStandardObject(
		camera_obj_ptr,
		*obj_ptr,
		image,
		x_rel, y_rel,
		x_vtrans, y_vtrans,
		zoom
	    );
	}

	/* Keep incoming fire warning? */
	if(warning.incoming_fire)
	{
	    /* Draw incoming fire warning cursor over player object. */
	    if(still_incoming_fire)
	    {
		if(IMGIsImageNumAllocated(IMG_CODE_VSMARK_INCOMINGFIRE))
		{
                    tmp_img_ptr = xsw_image[IMG_CODE_VSMARK_INCOMINGFIRE]->image;

		    BlitBufCursor(
                        gui->depth,
                        image->data,
                        reinterpret_cast<u_int8_t *>(tmp_img_ptr->data),
                        half_width - x_vtrans -
			    ((int)tmp_img_ptr->width / 2),
                        half_height + y_vtrans -
                            ((int)tmp_img_ptr->height / 2),
                        width, height,
			tmp_img_ptr->width, tmp_img_ptr->height,
                        xsw_color.bp_warning_cv
		    );
		}
	    }
	    else
	    {
		/* No longer warning for incoming fire, switch it off. */
		warning.incoming_fire = 0;
	    }
	}


	/*   Draw lens flares if global glow_obj_ptr is not NULL.
	 *   Since glow_obj_ptr is not NULL, it can be assumed
	 *   valid, inrange, and non-garbage.
	 */
	if((glow_obj_ptr != NULL) &&
	   option.show_lens_flares
	)
	{
	    /*   Get deltas from 0, 0 of viewscreen image.  This is
	     *   to pull a few math tricks so that it's easier to
	     *   calculate the positions of the flares using calc
	     *   instead of trig.
	     */
	    x = ((glow_obj_ptr->x - camera_obj_ptr->x) * 1000 *
		zoom) - x_vtrans;
            y = ((glow_obj_ptr->y - camera_obj_ptr->y) * 1000 *
                zoom) - y_vtrans;

	    /* Lensflares with star in middle. */
            if(IMGIsImageNumAllocated(IMG_CODE_LENSFLARE1))
	    {
                tmp_img_ptr = xsw_image[IMG_CODE_LENSFLARE1]->image;

		distance = 0.7;
		size = 0.35;	/* Must be 1 or less. */
                BlitBufAdditive(
                    gui->depth,
                    image->data,
                    reinterpret_cast<u_int8_t *>(tmp_img_ptr->data),
                    static_cast<int>((x * distance) + half_width -
                        ((int)tmp_img_ptr->width / 2 * size)),
                    static_cast<int>((int)height - ((y * distance) + half_height) -
                        ((int)tmp_img_ptr->height / 2 * size)),
                    image->width, image->height,
                    0, 0,
                    tmp_img_ptr->width, tmp_img_ptr->height,
                    tmp_img_ptr->width, tmp_img_ptr->height,
                    size,	/* Zoom. */
                    1		/* Magnification. */
                );

                distance = 0.1;
                size = 0.18;	/* Must be 1 or less. */
                BlitBufAdditive(
                    gui->depth,
                    image->data,
                    reinterpret_cast<u_int8_t *>(tmp_img_ptr->data),
                    static_cast<int>((x * distance) + half_width -
                        ((int)tmp_img_ptr->width / 2 * size)),
                    static_cast<int>((int)height - ((y * distance) + half_height) -
                        ((int)tmp_img_ptr->height / 2 * size)),
                    image->width, image->height,
                    0, 0,
                    tmp_img_ptr->width, tmp_img_ptr->height,
                    tmp_img_ptr->width, tmp_img_ptr->height,
                    size,       /* Zoom. */
                    1           /* Magnification. */
                );

                distance = -0.9;
                size = 0.9;	/* Must be 1 or less. */
                BlitBufAdditive(
                    gui->depth,
                    image->data,
                    reinterpret_cast<u_int8_t *>(tmp_img_ptr->data),
                    static_cast<int>((x * distance) + half_width -   
                        ((int)tmp_img_ptr->width / 2 * size)),
                    static_cast<int>((int)height - ((y * distance) + half_height) -
                        ((int)tmp_img_ptr->height / 2 * size)),
                    image->width, image->height,
                    0, 0,
                    tmp_img_ptr->width, tmp_img_ptr->height,
                    tmp_img_ptr->width, tmp_img_ptr->height,
                    size,       /* Zoom. */
                    1           /* Magnification. */
                );

                distance = -1.2;
                size = 0.33;	/* Must be 1 or less. */
                BlitBufAdditive(
                    gui->depth,
                    image->data,
                    reinterpret_cast<u_int8_t *>(tmp_img_ptr->data),
                    static_cast<int>((x * distance) + half_width -
                        ((int)tmp_img_ptr->width / 2 * size)),
                    static_cast<int>((int)height - ((y * distance) + half_height) -
                        ((int)tmp_img_ptr->height / 2 * size)),
                    image->width, image->height,
                    0, 0,
                    tmp_img_ptr->width, tmp_img_ptr->height,
                    tmp_img_ptr->width, tmp_img_ptr->height,
                    size,	/* Zoom. */
                    1		/* Magnification. */
                );

	    }

	    /* Lens flares with hollow center (rings). */
	    if(IMGIsImageNumAllocated(IMG_CODE_LENSFLARE2))
            {
                tmp_img_ptr = xsw_image[IMG_CODE_LENSFLARE2]->image;

                distance = 0.64;
                size = 0.70;     /* Must be 1 or less. */
                BlitBufAdditive(
                    gui->depth,
                    image->data,
                    reinterpret_cast<u_int8_t *>(tmp_img_ptr->data),
                    static_cast<int>((x * distance) + half_width -
                        ((int)tmp_img_ptr->width / 2 * size)),
                    static_cast<int>((int)height - ((y * distance) + half_height) -
                        ((int)tmp_img_ptr->height / 2 * size)),
                    image->width, image->height,
                    0, 0,
                    tmp_img_ptr->width, tmp_img_ptr->height,
                    tmp_img_ptr->width, tmp_img_ptr->height,
                    size,       /* Zoom. */
                    1           /* Magnification. */
                );

                distance = -1.4;
                size = 1.0;	/* Must be 1 or less. */
                BlitBufAdditive(
                    gui->depth,
                    image->data,
                    reinterpret_cast<u_int8_t *>(tmp_img_ptr->data),
                    static_cast<int>((x * distance) + half_width -
                        ((int)tmp_img_ptr->width / 2 * size)),
                    static_cast<int>((int)height - ((y * distance) + half_height) -
                        ((int)tmp_img_ptr->height / 2 * size)),
                    image->width, image->height,
                    0, 0,
                    tmp_img_ptr->width, tmp_img_ptr->height,
                    tmp_img_ptr->width, tmp_img_ptr->height,
                    size,       /* Zoom. */
                    1           /* Magnification. */
                );
	    }
	}



        /*   Draw viewscreen markings, this should be done after
	 *   all the `effects' are drawn so they don't obscure the
	 *   markines.
	 */
        if(option.show_viewscreen_marks)
        {
	    /* Sector ticks. */
	    VSDrawSectorTicks(
		image,
		x_rel,
		y_rel,
		x_vtrans,
		y_vtrans
	    );


            /* Lock cursor. */
            if(locked_obj_ptr != NULL)
	    {
                VSDrawLockCursor(
                    camera_obj_ptr,
                    locked_obj_ptr,
                    image,
		    ((locked_obj_ptr->x - x_rel) * 1000 *
                    zoom) - x_vtrans + half_width,
		    (int)height - (
                     ((locked_obj_ptr->y - y_rel) * 1000 *
                     zoom) - y_vtrans + half_height
		    ),
                    locked_obj_ptr->size
                );
            }

	    /* Heading arrow. */
            VSDrawHeadingArrow(
                image,
                camera_obj_ptr->heading,
                half_width - x_vtrans,
                half_height + y_vtrans
            );
        }


        /* Do fade in of view screen if connected and logged in
         * and bridge_win.viewscreen_gamma is less than 1.
         */
        if((net_parms.connection_state == CON_STATE_CONNECTED) &&
           (bridge_win.viewscreen_gamma < 1)
        )
        {
            BlitBufFade(
                gui->depth,
                image->data,
                0, 0,   
                image->width, image->height,
                image->width, image->height,
                bridge_win.viewscreen_gamma
            );

	    if(option.energy_saver_mode)
		bridge_win.viewscreen_gamma = 1;
	    else
                bridge_win.viewscreen_gamma += MIN(
		    0.016 * time_compensation,
                    0.25
	        );
        }



	/* Draw energy saver mode label? */
	if(option.energy_saver_mode)
	{
	    if(IMGIsImageNumAllocated(IMG_CODE_ENERGY_SAVER_MODE))
	    {
		tmp_img_ptr = xsw_image[IMG_CODE_ENERGY_SAVER_MODE]->image;

	        BlitBufNormal(
		    gui->depth,
		    image->data,
		    reinterpret_cast<u_int8_t *>(tmp_img_ptr->data),

		    half_width -
                    ((int)tmp_img_ptr->width / 2),
		    static_cast<int>((double)height * 0.6),

                    image->width, image->height,
		    0, 0,
		    tmp_img_ptr->width,
		    tmp_img_ptr->height,
		    tmp_img_ptr->width,
                    tmp_img_ptr->height,
		    1.0,	/* Zoom. */
		    1.0,	/* Visibility. */
		    1.0		/* Magnification. */
		);
	    }
	}


        /* Draw net stats label. */
        if((option.show_viewscreen_labels == 2) ||
           (option.show_viewscreen_labels == 3)   
        )    
	{
	    tmp_img_ptr = bridge_win.net_stats_image;
	    if(tmp_img_ptr != NULL)
                BlitBufNormal(
                    gui->depth,
                    image->data,
                    reinterpret_cast<u_int8_t *>(tmp_img_ptr->data),
                    (int)width - (int)tmp_img_ptr->width,
                    (int)height - (int)tmp_img_ptr->height - 16,
                    image->width, image->height,
                    0, 0,
                    tmp_img_ptr->width, tmp_img_ptr->height,
                    tmp_img_ptr->width, tmp_img_ptr->height,
                    1.0,	/* Zoom. */
	            1.0,	/* Visibility. */
		    1.0
                );

            tmp_img_ptr = bridge_win.vs_weapon_image;
            if(tmp_img_ptr != NULL)
                BlitBufNormal(
                    gui->depth,
                    image->data,
                    reinterpret_cast<u_int8_t *>(tmp_img_ptr->data),
                    10, 10,		/* Upper left corner. */
                    image->width, image->height,
                    0, 0,
                    tmp_img_ptr->width, tmp_img_ptr->height,
                    tmp_img_ptr->width, tmp_img_ptr->height,
                    1.0,        /* Zoom. */    
                    1.0,        /* Visibility. */
                    1.0
                );
	}


        /* Put shared image to window. */
#if defined(X_H) && defined(USE_XSHM)
	if(gui->def_use_shm)
	{
            XShmPutImage(
                gui->display,
                w,
                gui->gc,
                image->ximage,
                0, 0,
                0, 0,
                image->ximage->width, image->ximage->height,
                True
            );
	    image->in_progress = True;
	}
	else
	{
	    OSWPutSharedImageToDrawable(image, w);
	}
#else
	OSWPutSharedImageToDrawable(image, w);
#endif	/* defined(X_H) && defined(USE_XSHM) */


	return;
}



/*
 *	Recreates the image pointed to as the weapon stats label.
 *
 *      Important: Where image points to may be changed.
 */
void VSDrawUpdateWeaponLabel(
        image_t **image,
        pixmap_t pixmap
)
{
	char text[XSW_OBJ_NAME_MAX + 80];
	unsigned int width, height;
	image_t *new_image, *img_ptr;

	int sel_weapon;
	xsw_object_struct *obj_ptr;
	xsw_weapons_struct *wep_ptr;
	int ocsn_num;


        /* Error checks. */
        if((image == NULL) ||
           (pixmap == 0)
        )
            return;
        if(*image == NULL)
            return;


        /* ************************************************************** */
            
        /* Record width and height from image. */
        width = (*image)->width;
        height = (*image)->height;
            
        /* Clear and draw pixmap. */
        OSWClearPixmap(
            pixmap,
            width,
            height,
            osw_gui[0].black_pix
        );

	/* Is player object valid? */
	if((net_parms.player_obj_ptr != NULL) &&
           local_control.weapons_online
	)
	{
	    obj_ptr = net_parms.player_obj_ptr;
	    sel_weapon = obj_ptr->selected_weapon;

	    /* Is selected weapon valid? */
	    if((sel_weapon >= 0) && (sel_weapon < obj_ptr->total_weapons))
	    {
		wep_ptr = obj_ptr->weapons[sel_weapon];

		if(wep_ptr != NULL)
		{
		    img_ptr = NULL;

		    /* Get OCSN number. */
		    ocsn_num = OCSGetByCode(wep_ptr->ocs_code);
		    /* Get OCSN specified icon. */
		    if(ocsn_num > -1)
			img_ptr = ocsn[ocsn_num]->icon;

		    /* Get name label and default image (if needed). */
		    switch(wep_ptr->emission_type)
		    {
		      case WEPEMISSION_PROJECTILE:
                        if(IMGIsImageNumAllocated(IMG_CODE_VS_WEP_PROJECTILE) &&
                           (img_ptr == NULL)
			)
                            img_ptr = xsw_image[
				IMG_CODE_VS_WEP_PROJECTILE
			    ]->image;
                        sprintf(text, "%s: %d",
                            wep_ptr->name,
                            wep_ptr->amount
                        );
			break;

                      case WEPEMISSION_PULSE:
                        if(IMGIsImageNumAllocated(IMG_CODE_VS_WEP_PULSE) &&
                           (img_ptr == NULL)
                        )
                            img_ptr = xsw_image[
                                IMG_CODE_VS_WEP_PULSE
                            ]->image;
                        strncpy(
                            text,
                            wep_ptr->name,
                            XSW_OBJ_NAME_MAX
                        );
                        break;

                      case WEPEMISSION_STREAM:
                        if(IMGIsImageNumAllocated(IMG_CODE_VS_WEP_STREAM) &&
                           (img_ptr == NULL)
                        )
                            img_ptr = xsw_image[
                                IMG_CODE_VS_WEP_STREAM
                            ]->image;
                        strncpy(
                            text,
                            wep_ptr->name,
                            XSW_OBJ_NAME_MAX
                        );
                        break;
		    }
		    text[XSW_OBJ_NAME_MAX + 80 - 1] = '\0';

		    /* Draw weapon icon background. */
		    OSWPutImageToDrawablePos(
			img_ptr, pixmap,
			0, 0
		    );

		    /* Draw name label. */
		    OSWSetFgPix(xsw_color.standard_text);
                    OSWDrawString(
                        pixmap,
                        4, (int)height - 5,
                        text
                    );
		}
	    }
	}

        /* Get new image from pixmap. */
        new_image = OSWGetImage(
            pixmap,
            0, 0,
            width, height
        );
        if(new_image == NULL)
            return;

        /* Destroy old image and set new. */
        OSWDestroyImage(image);

        *image = new_image;


	return;
}

/*
 *	Recreates the image pointed to as the network stats label.
 *
 *	Important: Where image points to may be changed.
 */
void VSDrawUpdateNetstatsLabel(
	image_t **image,
        pixmap_t pixmap
)
{
	char string[256];
	unsigned int width, height;
	image_t *new_image;


	/* Error checks. */
	if((image == NULL) ||
	   (pixmap == 0)
	)
	    return;
	if(*image == NULL)
	    return;


	/* ************************************************************** */

	/* Record width and height from image. */
	width = (*image)->width;
	height = (*image)->height;

	/* Clear and draw pixmap. */
	OSWClearPixmap(
	    pixmap, 
	    width,
	    height,
	    osw_gui[0].black_pix
	);

        sprintf(string,
            "Int: %i ms  FPS: %i  SX: %i  RX: %i  Lapse: %i",
            (int)net_parms.net_int,
            (int)fps_counter.lfcount,
            (int)(loadstat.sx_ilast *
            (double)(1000 / (double)auto_interval_tune.interval)),
            (int)(loadstat.rx_ilast *
            (double)(1000 / (double)auto_interval_tune.interval)),
            (int)lapsed_millitime
        );

	OSWSetFgPix(xsw_color.standard_text);
	OSWDrawString(
	    pixmap,
	    8, ((int)height / 2) + 5,
	    string
	);


	/* Get new image from pixmap. */
	new_image = OSWGetImage(
	    pixmap,
	    0, 0,
	    width, height
	);
	if(new_image == NULL)
	    return;


	/* Destroy old image and set new. */
	OSWDestroyImage(image);

	*image = new_image;


	return;
}
