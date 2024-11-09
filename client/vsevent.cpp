/*
                       View Screen: Event Handling

	Functions:

	int ScannerButtonMatch(event_t *event)
	int ScannerSBCB(void *ptr)

	void VSChangePage(page_struct *new_page)
	int VSButtonMatch(event_t *event)
	int VSManage(event_t *event)
	void VSMap()
	void VSUnmap()
	void VSDestroy()

	---

	Scanner events are handled here as well.

 */

#include "../include/unvmath.h"

#include "xsw.h"
#include "net.h"


#define MIN(a,b)	(((a) < (b)) ? (a) : (b))
#define MAX(a,b)	(((a) > (b)) ? (a) : (b))


/*
 *	Matches an object by a button press coordinate on the scanner.
 *
 *      Returns matched object number or -1 for no match.
 */
int ScannerButtonMatch(event_t *event)
{
        int x, y;               /* Coordinates of ButtonPress. */
        char player_in_nebula = 0;
        int i, object_num, matched;
        xsw_object_struct *obj_ptr, *tar_obj_ptr, **ptr;
        double width, height;
        double scan_scale;      /* Scanner zoom. */

        double rel_x, rel_y; 
        double dx, dy;
        double rx, ry;
        double sintheta, costheta;
        double scanner_range;
        double scanner_factor;	/* Real units to scanner units factor. */
double win_x = 0;
double win_y = 0;

  
        if(event == NULL)
            return(-1);

        /* Event type must be ButtonPress. */
        if(event->type != ButtonPress)
        {
            fprintf(stderr,
"ScannerButtonMatch(): Event type not ButtonPress.\n"
            );
            return(-1);
        }

	/* Get pointer and number of player object. */
	object_num = net_parms.player_obj_num;
	if(DBIsObjectGarbage(object_num))
	    return(-1);
	else
	    obj_ptr = xsw_object[object_num];

/* Need to subtract x and y scanner translation? */
        x = event->xbutton.x;
        y = event->xbutton.y;
        width = bridge_win.scanner_width;
        height = bridge_win.scanner_height;
        scan_scale = bridge_win.scanner_zoom;


        /* Sanitize scan_scale. */
        if(scan_scale > 1.0)
            scan_scale = 1;
        else if(scan_scale < 0.001)
            scan_scale = 0.001;

        /* Get effective scanner range. */
        scanner_range = MAX(obj_ptr->scanner_range, 0);
	if(option.scanner_limiting)
	{
            if(obj_ptr->loc_type == XSW_LOC_TYPE_NEBULA)
	    {
                scanner_range = scanner_range * VISIBILITY_NEBULA;
		player_in_nebula = 1;
	    }
        }


        /* Get scanner_factor. */
        if(height >= width)
        {
            if(scanner_range > 0)
            {
                scanner_factor = ((height / 2) /
                    scanner_range) / scan_scale;
            }
            else
            {
                scanner_factor = (height / 2) / scan_scale;
            }
        }
        else
        {
            if(scanner_range > 0)
            {
                scanner_factor = ((width / 2 ) /
                    scanner_range) / scan_scale;
            }
            else
            {
                scanner_factor = (width / 2) / scan_scale;
            }
        }
            
        /* Sanitize scanner_factor. */
        if(scanner_factor < 0)   
           scanner_factor = 0.01;


        /* Get relative position (in XSW real units). */
        rel_x = obj_ptr->x;
        rel_y = obj_ptr->y;


        /* Get sintheta and costheta. */
        sintheta = sin(obj_ptr->heading);
        costheta = cos(obj_ptr->heading);


	/* Go through entire objects list. */
        for(matched = -1, i = 0, ptr = xsw_object;
            i < total_objects;
            i++, ptr++
        )
        {
	    tar_obj_ptr = *ptr;
	    if(tar_obj_ptr == NULL)
		continue;

            /* Skip following object types. */
            if((tar_obj_ptr->type <= XSW_OBJ_TYPE_GARBAGE) ||
               (tar_obj_ptr->type == XSW_OBJ_TYPE_STREAMWEAPON) ||
               (tar_obj_ptr->type == XSW_OBJ_TYPE_SPHEREWEAPON)
	    )
                continue;

            /* If target object is in nebula and player object is not
	     * then do not check for it.
	     */
            if((tar_obj_ptr->type != XSW_OBJ_TYPE_AREA) &&
               (tar_obj_ptr->loc_type == XSW_LOC_TYPE_NEBULA)
            )
            {
                if(!player_in_nebula)
                    continue;
            }

            /* Don't match player object itself. */
            if(tar_obj_ptr == obj_ptr)
                continue;

            /* Skip of objects are not in the same sector. */
            if(!Mu3DInSameSectorPtr(obj_ptr, tar_obj_ptr))
                continue;

            /* Skip outdated objects. */
            if((tar_obj_ptr->last_updated + OBJECT_OUTDATED_TIMEOUT) <
               cur_millitime
            )
                continue;

	    /* Is object in scanner range? */
            dx = tar_obj_ptr->x - rel_x;
            dy = tar_obj_ptr->y - rel_y;
            if(Mu3DDistance(dx, dy, 0) >
              (scanner_range * DBGetObjectVisibilityPtr(tar_obj_ptr))
            )
                continue; 


            /* Calculate dx and dy position to center of object_count. */
            dx = (tar_obj_ptr->x - rel_x) * scanner_factor;
            dy = (tar_obj_ptr->y - rel_y) * scanner_factor;   

            switch(bridge_win.scanner_orient)
            {
	      case SCANNER_ORIENT_LOCAL:
                rx = (costheta * dx) + (sintheta * -1 * dy);  
                ry = ((sintheta * dx) + (costheta * dy)) * -1;
                break;

              default:	/* SCANNER_ORIENT_GC */
                rx = dx;
                ry = dy * -1;
                break;
	    }

            dx = rx + (width / 2);
            dx = dx + win_x;
            if(dx > (win_x + width))
		continue;
            if(dx < win_x)
		continue;

            dy = ry + (height / 2);
            dy = dy + win_y;
            if(dy > (win_y + height))   
		continue;
            if(dy < win_y)
		continue;

            /* dx and dy are now center of target object relative to
             * player object which is center of plane.
             */
            if((x >= (dx - 4)) &&
               (x <= (dx + 4)) &&
               (y >= (dy - 4)) &&
               (y <= (dy + 4))
            )
            {
                matched = i;
                break;
            }
        }


        return(matched);
}


/*
 *	Scanner scale bar zoom callback handler.
 */
int ScannerSBCB(void *ptr)
{
	int player_obj_num;
	scale_bar_struct *sb;


	sb = (scale_bar_struct *)ptr;
	if(sb == NULL)
	    return(-1);

	player_obj_num = net_parms.player_obj_num;


	/* Get new zoom. */
        bridge_win.scanner_zoom = (double)sb->pos / 100;
	if(bridge_win.scanner_zoom < 0.001)
            bridge_win.scanner_zoom = 0.001;
	if(bridge_win.scanner_zoom > 1)
	    bridge_win.scanner_zoom = 1;


        /* Recreate scanner labels. */
        ScannerUpdateLabels(player_obj_num);

        /* Redraw scanner. */
        ScannerDraw(
            player_obj_num,
            bridge_win.scanner,
            bridge_win.scanner_image,
            0,
            0,
            bridge_win.scanner_zoom
        );
  

	return(0);
}


/* **************************************************************** */

/*
 *	Procedure to change the page being displayed on the
 *	viewscreen.
 */
void VSChangePage(page_struct *new_page)
{
	int mapped = 0;
	win_t w;
	shared_image_t *image;


	if(new_page == NULL)
	    return;

	w = bridge_win.viewscreen;
	image = bridge_win.viewscreen_image;


	/* Record if page was currently mapped. */
	if(bridge_win.cur_page != NULL)
	{
	    if(bridge_win.cur_page->map_state)
		mapped = 1;
	}

	/* Unmap as needed. */
	if(mapped)
	    PageUnmap(bridge_win.cur_page, w, image);

	/* Set pointer to new current page. */
	bridge_win.cur_page = new_page;

	/* Map new page if previous was mapped. */
	if(mapped)
	    PageMap(bridge_win.cur_page, w, image);


	return;
}

/*
 *      Handles event assumed to have occured on a viewscreen
 *      and to be a ButtonPress event.
 *      Returns the object that is matched or -1 on error.
 */
int VSButtonMatch(event_t *event)
{
        double x_rel, y_rel;
        int x_vtrans, y_vtrans;
	unsigned int width, height;
        double distance;
	int object_num;
        xsw_object_struct **obj_ptr, *camera_obj_ptr;
                             

        if(event == NULL)
	    return(-1);


        width = bridge_win.viewscreen_width;
        height = bridge_win.viewscreen_height;


        /* Skip if player object is garbage. */
	camera_obj_ptr = net_parms.player_obj_ptr;
        if(camera_obj_ptr == NULL)
            return(-1);


        /* Calculate x_vtrans and y_vtrans (in units of pixels). */
        if(camera_obj_ptr->velocity_max > 0)
        {
            x_vtrans = static_cast<int>(MuPolarRotX(
                camera_obj_ptr->velocity_heading,
                camera_obj_ptr->velocity /
                camera_obj_ptr->velocity_max *
                (double)((int)width / 2.5)
            ));
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
        
         
        /* Calculate x_rel and y_rel. */
	x_rel = ((double)(event->xbutton.x - ((int)width / 2) +
	    x_vtrans) / 1000 / bridge_win.viewscreen_zoom) +
	    camera_obj_ptr->x;

        y_rel = ((double)((int)height - event->xbutton.y - 
            ((int)height / 2) + y_vtrans) / 1000 /
            bridge_win.viewscreen_zoom) + camera_obj_ptr->y;


        /* Scan through objects. */
        for(object_num = 0, obj_ptr = xsw_object;
            object_num < total_objects;
            object_num++, obj_ptr++
        )
        {
            if((*obj_ptr) == NULL)
                continue;
                
            /* Skip following object types. */
            if(((*obj_ptr)->type <= XSW_OBJ_TYPE_GARBAGE) ||
               ((*obj_ptr)->type == XSW_OBJ_TYPE_STREAMWEAPON) ||
               ((*obj_ptr)->type == XSW_OBJ_TYPE_SPHEREWEAPON) ||
               ((*obj_ptr)->type == XSW_OBJ_TYPE_AREA)
            )
                continue;
         
            /* Don't match itself. */
            if(*obj_ptr == camera_obj_ptr)
                continue;
   
            /* Skip if object is not in the same sector. */
            if(!Mu3DInSameSectorPtr(camera_obj_ptr, *obj_ptr))
                continue;

            /* Skip outdated objects. */
            if(((*obj_ptr)->last_updated + OBJECT_OUTDATED_TIMEOUT)
                < cur_millitime
            )
                continue;
        
            distance = Mu3DDistance(
                (*obj_ptr)->x - x_rel,
                (*obj_ptr)->y - y_rel,
                0
            ) * 1000;

            if(distance < (double)(*obj_ptr)->size)
                break;
        }
        if(object_num < total_objects)
            return(object_num);
	else
	    return(-1);
}



int VSManage(event_t *event)
{
	static int events_handled;


	if(event == NULL)
	    return(0);


	/* ****************************************************** */

	events_handled = 0;




	return(events_handled);
}



void VSMap()  
{
	/* Already mapped? */
	if(bridge_win.viewscreen_map_state)
	    return;


        /* Change cursor. */
	if(xsw_cursor.scanner_lock != NULL)
	{
            OSWSetWindowCursor(
	        bridge_win.viewscreen,
                xsw_cursor.scanner_lock->cursor
	    );
	}

	/* Change selected watched events. */
	OSWSetWindowInput(bridge_win.viewscreen,
            ButtonPressMask | VisibilityChangeMask
	);


	/* Mark as mapped. */
	bridge_win.viewscreen_map_state = 1;

#if defined(X_H) && defined(USE_XSHM)
	/* Reset viewscreen image `put' completion mark to False. */
	if(bridge_win.viewscreen_image != NULL)
	{
	    if(osw_gui[0].def_use_shm)
	    {
		bridge_win.viewscreen_image->in_progress = False;
	    }
	}
#endif	/* defined(X_H) && defined(USE_XSHM) */


	return;
}



void VSUnmap()
{
	/* Already unmapped? */
        if(!bridge_win.viewscreen_map_state)
            return;

	/* Mark as unmapped. */
	bridge_win.viewscreen_map_state = 0;


	return;
}



void VSDestroy()
{




	return;
}
