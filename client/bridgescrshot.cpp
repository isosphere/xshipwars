/*
                             Bridge: Screen Shot

	Functions:

	int BridgeDoScreenShot(
	        char *save_dir,   
	        int start_x, int start_y,
	        int end_x, int end_y
	)

	---

 */

#include "../include/tga.h"
#include "blitting.h"

#include "xsw.h"


void BridgeSSGetAndBlit(win_t src_w, image_t *tar_img);

/*
 *	Macro to get image from w and put it on tar_img.
 */
void BridgeSSGetAndBlit(win_t src_w, image_t *tar_img)
{
	win_attr_t wattr;
	image_t *grabed_image;


	if((src_w == 0) ||
           (tar_img == NULL)
	)
	    return;


        OSWGetWindowAttributes(src_w, &wattr);
        grabed_image = OSWGetImage(
            src_w,
            0, 0, wattr.width, wattr.height
        );
        if(grabed_image != NULL)
        {
            BlitBufAbsolute(
                osw_gui[0].depth,
                reinterpret_cast<u_int8_t *>(tar_img->data),
                reinterpret_cast<u_int8_t *>(grabed_image->data),
                wattr.x, wattr.y,
                tar_img->width, tar_img->height,
                0, 0,
                grabed_image->width, grabed_image->height,
                grabed_image->width, grabed_image->height,
                1, 1
            );

            OSWDestroyImage(&grabed_image);
        }
                

	return;
}


/*
 *	Procedure to take a screen shot of the bridge window.
 *	The save_dir is the directory to save the new screen shot
 *	image file to.
 *
 *	Currently the coordinate inputs are not used.
 */
int BridgeDoScreenShot(
	char *save_dir,
	int start_x, int start_y,
	int end_x, int end_y
)
{
	int i, status;
	struct stat stat_buf;
	char *strptr;
	image_t *final_image;
	win_attr_t top_wattr;
	tga_data_struct tga_data;
	char text[PATH_MAX + NAME_MAX + 80];
	char new_filename[PATH_MAX + NAME_MAX];
	char lsave_dir[PATH_MAX + NAME_MAX];


	if(!IDC() ||
           (save_dir == NULL)
	)
	    return(-1);


	if(!bridge_win.map_state)
	    return(-2);


	strncpy(lsave_dir, save_dir, PATH_MAX + NAME_MAX);
	lsave_dir[PATH_MAX + NAME_MAX - 1] = '\0';
	strptr = PathSubHome(lsave_dir);
        if(strptr != NULL)
        {
            strncpy(lsave_dir, strptr, PATH_MAX + NAME_MAX);
            lsave_dir[PATH_MAX + NAME_MAX - 1] = '\0';
        }


	/* Make sure lsave_dir exists and is a dir. */
	if(stat(lsave_dir, &stat_buf))
	{
            sprintf(text,
                "%s: No such directory.",
		lsave_dir
            );
            MesgAdd(text, xsw_color.bold_text);
	    return(-1);
	}
	if(!S_ISDIR(stat_buf.st_mode))
	{
            sprintf(text,
                "%s: Not a directory.", 
                lsave_dir
            );
            MesgAdd(text, xsw_color.bold_text);
            return(-1);
        }

	/* Generate new_filename. */
	strptr = PrefixPaths(lsave_dir, "xsw");
	if(strptr == NULL)
	    return(-1);
	for(i = 0; i < 100000; i++)
	{
	    sprintf(new_filename, "%s%i.tga",
		strptr, i
	    );
	    /* Check if file does not exist. */
	    if(stat(new_filename, &stat_buf))
		break;
	}
	if(i >= 100000)
	{
	    MesgAdd("Out of tempory names.", xsw_color.bold_text);
	    return(-1);
	}


	/* Get attributes of toplevel. */
	OSWGetWindowAttributes(bridge_win.toplevel, &top_wattr);
	if((top_wattr.width == 0) ||
           (top_wattr.height == 0)
	)
	    return(-1);

	/* Create final image buffer (same size as toplevel). */
	if(OSWCreateImage(&final_image, top_wattr.width, top_wattr.height))
	{
	    MesgAdd(
		"Cannot create tempory buffers for screen shot.",
		xsw_color.bold_text
	    );
	    return(-1);
	}


	/* Get images from windows. */
        BridgeSSGetAndBlit(bridge_win.pan_p1, final_image);
        BridgeSSGetAndBlit(bridge_win.pan_p2, final_image);
        BridgeSSGetAndBlit(bridge_win.pan_p3, final_image);
        BridgeSSGetAndBlit(bridge_win.pan_p4, final_image);
        BridgeSSGetAndBlit(bridge_win.viewscreen, final_image);
        BridgeSSGetAndBlit(bridge_win.scanner, final_image);
	BridgeSSGetAndBlit(bridge_win.pan_s1, final_image);
        BridgeSSGetAndBlit(bridge_win.pan_s2, final_image);
        BridgeSSGetAndBlit(bridge_win.pan_s3, final_image);
        BridgeSSGetAndBlit(bridge_win.mesg_box, final_image);



	/* Save to tga image. */
	tga_data.fp = NULL;

	tga_data.id_field_len = 0;
	tga_data.cmap_type = 0;
	tga_data.img_type = TgaDataTypeURGB;

        tga_data.cmap_origin = 0;
        tga_data.cmap_length = 0;
        tga_data.cmap_size = 24;

	tga_data.x = 0;
	tga_data.y = 0;
	tga_data.width = final_image->width;
	tga_data.height = final_image->height;
	tga_data.depth = 24;
	tga_data.descriptor = 0;

	tga_data.file_size = 0;
	tga_data.data_size = 0;

	tga_data.cur_load_pixel = -1;
	tga_data.header_data = NULL;
	tga_data.data = reinterpret_cast<u_int8_t *>(final_image->data);
	tga_data.data_depth = final_image->depth;

	status = TgaWriteToFile(
           new_filename,
	   &tga_data,
           24		/* 24 bits on file. */
	);
	if(status != TgaSuccess)
	{
            MesgAdd(
                "Error saving screen shot to file.",
                xsw_color.bold_text
            );
	}
	else
	{
	    sprintf(text,
		"Saved screen shot to file: %s",
		new_filename
	    );
	    MesgAdd(text, xsw_color.standard_text);
	}

	OSWDestroyImage(&final_image);


	return(0);
}
