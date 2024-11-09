/*
                        Basic XShipWars Images

 	Functions:

	int IMGIsImageNumAllocated(int image_num)
	int IMGIsImageNumLoaded(int image_num)

	int IMGResize(
	        int image_num, 
	        unsigned int width,
	        unsigned int height
	)
	int IMGAllocateExplicit(int image_num)
	int IMGLoadImage(int image_num, char *filename)
	int IMGLoadImageData(int image_num, unsigned char *data)
	void IMGUnload(int image_num)

	int IMGDoLoadAll()
	void IMGDoUnloadAll()

 	---

	IMPORTANT: Image Set Referances are not managed here,
	these functions are soly for managing internal images.


 */


#include "xsw.h"
#include "splash.h"

#include "imagepaths.h"


/*
 *	Compile time image datas:
 */
/* Icons. */
#include "ct_data/icons/error.h"
#include "ct_data/icons/univ_std.h"
#include "ct_data/icons/univ_haslogin.h"
#include "ct_data/icons/univ_old.h"
#include "ct_data/icons/univ_unknown.h"
#include "ct_data/icons/info.h"

/* Banners. */
#include "ct_data/images/esm.h"		/* Energy saver mode. */



/*
 *	Checks if image_num is allocated.
 */
int IMGIsImageNumAllocated(int image_num)
{
	if((xsw_image == NULL) ||
           (image_num < 0) ||
           (image_num >= total_images)
	)
	    return(0);
        if(xsw_image[image_num] == NULL)
            return(0);
	else
	    return(1);
}


/*
 *	Checks if image of image_num is loaded.
 */
int IMGIsImageNumLoaded(int image_num)
{
	if(!IMGIsImageNumAllocated(image_num))
	    return(0);
	else if(xsw_image[image_num]->image == NULL)
	    return(0);
	else
	    return(1);
}



/*
 *	Changes the size of a loaded image.
 */
int IMGResize(
        int image_num,
        unsigned int width,
        unsigned int height
)
{
	image_t *old_image = NULL;
	image_t *new_image = NULL;


	/* Make sure image is allocated and loaded. */
	if(IMGIsImageNumLoaded(image_num))
	    old_image = xsw_image[image_num]->image;
	else
	    return(-1);


	if(old_image == NULL)
	    return(-1);


	/* Create a new image of new size. */
	if(
	    OSWCreateImage(
		&new_image,
		width, height
	    )
	)
	    return(-1);


	WidgetResizeImageBuffer(
	    osw_gui[0].depth,
	    reinterpret_cast<u_int8_t *>(new_image->data),  /* Target. */
	    reinterpret_cast<u_int8_t *>(old_image->data),  /* Source. */
	    new_image->width, new_image->height,
	    old_image->width, old_image->height
	);


	/* Destroy old image. */
	OSWDestroyImage(&old_image);


	/* Set pointer on image structure to point to new image. */
	xsw_image[image_num]->image = new_image;


	return(0);
}


/*
 *	Allocates an image structure explicitly.
 */
int IMGAllocateExplicit(int image_num)
{
	int i, prev_total_images;


	/* Error checks. */
	if(image_num < 0)
	    return(-1);

	/* Already allocated? */
	if(IMGIsImageNumAllocated(image_num))
	{
	    /* Unload as needed. */
	    IMGUnload(image_num);

	    return(0);
	}

	/* **************************************************** */

	/* Record previous total. */
	prev_total_images = total_images;

	/* Adjust global variable total_images. */
	total_images = image_num + 1;


	/* Reallocate pointers. */
	xsw_image = (xsw_image_struct **)realloc(
	    xsw_image,
	    total_images * sizeof(xsw_image_struct *)
	);
	if(xsw_image == NULL)
	{
	    fprintf(stderr,
   "IMGAllocateExplicit(): Error allocating %i pointers.\n",
		total_images
	    );
	    total_images = 0;
	    return(-1);
	}

	/* Allocate each new entry. */
	for(i = prev_total_images; i < total_images; i++)
	{
	    xsw_image[i] = (xsw_image_struct *)calloc(
		1,
		sizeof(xsw_image_struct)
	    );
	    if(xsw_image[i] == NULL)
	    {
                fprintf(stderr,
   "IMGAllocateExplicit(): Error allocating structure %i.\n",
                    i
                );
		continue;
	    }

	    xsw_image[i]->load_state = 0;
	    xsw_image[i]->filename = NULL;
	    xsw_image[i]->image = NULL;
	}


	return(0);
}


/*
 *	Load image.
 */
int IMGLoadImage(int image_num, char *filename)
{
	int status;
	char *strptr;
	char parsed_filename[PATH_MAX + NAME_MAX];
	xsw_image_struct *img_ptr = NULL;


	/* Error checks. */
	if(!IDC())
	{
            fprintf(stderr,
                "IMGLoadImage(): Display not connected.\n"
            );
            return(-1);
	}
	if(image_num < 0)
	{
            fprintf(stderr,
    "IMGLoadImage(): Cannot allocate negative image structure.\n"
            );
	    return(-1);
	}
	if(filename == NULL)
	{
            fprintf(stderr,
    "IMGLoadImage(): Cannot load image from a (null) filename.\n"
	    );
	    return(-1);
	}


	/*   Make sure file name is absolute, if not then prefix global
         *   images directory to it.
	 */
	if(!ISPATHABSOLUTE(filename))
	    strptr = PrefixPaths(dname.images, filename);
	else
	    strptr = filename;
	if(strptr == NULL)
	{
	    fprintf(stderr,
    "IMGLoadImage(): Unable to interprite directory `%s'.\n",
		filename
            );
            return(-1);
	}
	strncpy(parsed_filename, strptr, PATH_MAX + NAME_MAX);
	parsed_filename[PATH_MAX + NAME_MAX - 1] = '\0';


	/* ************************************************************** */
	/*   Allocate image_num as needed, if it is already allocated
	 *   then unload it.
	 */

        if(IMGIsImageNumAllocated(image_num))
        {
            /* Unload and free its resources. */
            IMGUnload(image_num);

	    img_ptr = xsw_image[image_num];
        }
	else
        {
	    /* Allocate new image structure explicitly. */
	    status = IMGAllocateExplicit(image_num);
	    if(status)
		return(-1);
	    else
		img_ptr = xsw_image[image_num];
	}
	if(img_ptr == NULL)
	    return(-1);


	/* **************************************************************** */

	/* Allocate new parsed_filename to image_num. */
	img_ptr->filename = StringCopyAlloc(parsed_filename);
	if(img_ptr->filename == NULL)
	{
            fprintf(stderr,
   "IMGLoadImage(): Memory allocation error for filename: %s\n",
                filename
            );

	    /* Mark it as not loaded. */
            img_ptr->load_state = 0;

            return(-1);
	}


	/* Load image. */
	img_ptr->image = WidgetLoadImageFromTgaFile(img_ptr->filename);
	if(img_ptr->image == NULL)
	{
	    fprintf(stderr,
                "IMGLoadImage(): Cannot load image: %s.\n",
		filename
	    );

            /* Unload and free all resources. */
            IMGUnload(image_num);

	    return(-1);
	}


	/* Mark iamge as loaded. */
	img_ptr->load_state = 1;


	return(0);
}


/*
 *	Load image_num from data.
 */
int IMGLoadImageData(int image_num, unsigned char *data)
{        
        int status;
	
	
        /* Error checks. */
        if(!IDC())  
        {
            fprintf(stderr,
                "IMGLoadImage(): Display not connected.\n"
            );
            return(-1);
        }
        if(image_num < 0)
        {
            fprintf(stderr,
    "IMGLoadImage(): Cannot allocate negative image structure.\n"
            );
            return(-1);
        }
        if(data == NULL)
        {
            fprintf(stderr,
    "IMGLoadImage(): Cannot load image from NULL data pointer.\n"
            );
            return(-1);
        }


        /* ************************************************************** */
         
        /* Check if internal image structure image_num is allocated. */
        if(IMGIsImageNumAllocated(image_num))
	{
	    /* Unload and free all resources. */
	    IMGUnload(image_num);
	}
	else
        {
            /* Allocate. */
            status = IMGAllocateExplicit(image_num);
            if(status) return(-1);
        }


        /* **************************************************************** */
 
	/* Do not allocate filename. */


        /* Load image. */
        xsw_image[image_num]->image = WidgetLoadImageFromTgaData(
	    data
	);
        if(xsw_image[image_num]->image == NULL)
        {
            fprintf(stderr,
                "IMGLoadImage(): Cannot load image data: 0x%.8x\n",
                (u_int32_t)data
            );

	    /* Unload and free all resources. */
	    IMGUnload(image_num);

            return(-1);
        }


        /* Mark as loaded. */
        xsw_image[image_num]->load_state = 1;
            
         
        return(0);
}



/*
 *	Unloads data from image_num.
 *	Frees all resources including the filename.
 */
void IMGUnload(int image_num)
{
	if(!IMGIsImageNumAllocated(image_num))
	    return;


	/* Free file name. */
#ifdef DEBUG_MEM_FREE
if(xsw_image[image_num]->filename != NULL)
    printf("Internal Image %i: Free'ed filename (unload).\n", image_num);
#endif
	free(xsw_image[image_num]->filename);
	xsw_image[image_num]->filename = NULL;


	/* Free image. */
	if(IDC())
	{
#ifdef DEBUG_MEM_FREE
if(xsw_image[image_num]->image != NULL)
    printf("Internal Image %i: Free'ed image (unload).\n", image_num);
#endif
	    OSWDestroyImage(&xsw_image[image_num]->image);
	}

	/* Mark as unloaded. */
	xsw_image[image_num]->load_state = 0;


	return;
}


/*
 *	Procedure to initialize internal images.
 */
int IMGDoLoadAll()
{
        int status;

	int imgs_loaded = 0;
	const int total_imgs = 45;	/* Total to be loaded. */


	/* Check if display is connected. */
	if(!IDC())
	{
	    fprintf(stderr,
		"IMGDoLoadAll(): Error: Display not connected.\n"
	    );
	    return(-1);
	}


	/* ************************************************************** */

        /*   Startup splash background is not loaded since it is already
         *   loaded by the splash window.  The splash window at this point
	 *   should already be initialized.
	 */
	/* IMG_CODE_STARTUP_BKG */


        /* Viewscreen marks. */
        SplashDoUpdateProgress(imgs_loaded, total_imgs,
            "Loading Image: Cursors...");
        status = IMGLoadImage(IMG_CODE_VSMARK_OBJECT, IMGPATH_VS_TMARK_OBJECT);
        if(status)
	    return(-1);
	imgs_loaded++;

        SplashDoUpdateProgress(imgs_loaded, total_imgs,
            "Loading Image: Cursors...");
        status = IMGLoadImage(IMG_CODE_VSMARK_VESSEL, IMGPATH_VS_TMARK_VESSEL);
        if(status)
	    return(-1);
	imgs_loaded++;

        SplashDoUpdateProgress(imgs_loaded, total_imgs,
            "Loading Image: Cursors...");
        status = IMGLoadImage(IMG_CODE_VSMARK_INCOMINGFIRE, IMGPATH_VS_MARK_INCOMINGFIRE);
        if(status)
            return(-1);
        imgs_loaded++;


        SplashDoUpdateProgress(imgs_loaded, total_imgs,
            "Loading Image: Cursors...");
        status = IMGLoadImage(IMG_CODE_VS_WEP_PROJECTILE, IMGPATH_VS_WEP_PROJECTILE);
        if(status)
            return(-1);
        imgs_loaded++;

        SplashDoUpdateProgress(imgs_loaded, total_imgs,
            "Loading Image: Cursors...");
        status = IMGLoadImage(IMG_CODE_VS_WEP_PULSE, IMGPATH_VS_WEP_PULSE);
        if(status)
            return(-1);
        imgs_loaded++;

        SplashDoUpdateProgress(imgs_loaded, total_imgs,
            "Loading Image: Cursors...");
        status = IMGLoadImage(IMG_CODE_VS_WEP_STREAM, IMGPATH_VS_WEP_STREAM);
        if(status)
            return(-1);
        imgs_loaded++;


	/* Scanner marks. */
        SplashDoUpdateProgress(imgs_loaded, total_imgs,
            "Loading Image: Cursors...");
        status = IMGLoadImage(IMG_CODE_SCMARK_UNKNOWN, IMGPATH_SCMARK_UNKNOWN);
        if(status)
	    return(-1);
	imgs_loaded++;

        SplashDoUpdateProgress(imgs_loaded, total_imgs,
            "Loading Image: Cursors...");
        status = IMGLoadImage(IMG_CODE_SCMARK_LOCKED, IMGPATH_SCMARK_LOCKED);
        if(status)
	    return(-1);
	imgs_loaded++;

        SplashDoUpdateProgress(imgs_loaded, total_imgs,
            "Loading Image: Cursors...");
        status = IMGLoadImage(IMG_CODE_SCMARK_WEAPON, IMGPATH_SCMARK_WEAPON);
        if(status)
	    return(-1);
	imgs_loaded++;

        SplashDoUpdateProgress(imgs_loaded, total_imgs,
            "Loading Image: Cursors...");
        status = IMGLoadImage(IMG_CODE_SCMARK_HOME, IMGPATH_SCMARK_HOME);
        if(status)
	    return(-1);
	imgs_loaded++;

        SplashDoUpdateProgress(imgs_loaded, total_imgs,
            "Loading Image: Cursors...");
        status = IMGLoadImage(IMG_CODE_SCMARK_AREA, IMGPATH_SCMARK_AREA);
        if(status)
	    return(-1);
	imgs_loaded++;


	/* XSW/Bridge desktop icon. */
        SplashDoUpdateProgress(imgs_loaded, total_imgs,
            "Loading Image: Icons...\n");
	status = IMGLoadImage(IMG_CODE_XSW_ICON, IMGPATH_ICON_BRIDGE);
        if(status)
	    return(-1);
	imgs_loaded++;

        /* Economy window desktop icon. */
        SplashDoUpdateProgress(imgs_loaded, total_imgs,
            "Loading Image: Icons...\n");
        status = IMGLoadImage(IMG_CODE_ECONOMY_ICON, IMGPATH_ICON_ECONOMY);
        if(status) 
            return(-1);
        imgs_loaded++;

        /* Options window desktop icon. */
        SplashDoUpdateProgress(imgs_loaded, total_imgs,
            "Loading Image: Icons...\n");
        status = IMGLoadImage(IMG_CODE_OPTIONS_ICON, IMGPATH_ICON_OPTIONS);
        if(status)
            return(-1);
        imgs_loaded++;

        /* Universe list and edit window desktop icon. */
        SplashDoUpdateProgress(imgs_loaded, total_imgs,
            "Loading Image: Icons...\n");
        status = IMGLoadImage(IMG_CODE_UNIV_ICON, IMGPATH_ICON_UNIVERSE);
        if(status)
            return(-1);
        imgs_loaded++;

        /* Starchart window desktop icon. */
        SplashDoUpdateProgress(imgs_loaded, total_imgs,
            "Loading Image: Icons...\n");
        status = IMGLoadImage(IMG_CODE_STARCHART_ICON, IMGPATH_ICON_STARCHART);
        if(status)
            return(-1);
        imgs_loaded++;


        /* Galaxy icon. */
        SplashDoUpdateProgress(imgs_loaded, total_imgs,
            "Loading Image: Icons...\n");
        status = IMGLoadImageData(IMG_CODE_UNIV_STD_ICON, univ_std_tga);
        if(status)
	    return(-1);
	imgs_loaded++;

        SplashDoUpdateProgress(imgs_loaded, total_imgs,
            "Loading Image: Icons...\n");
        status = IMGLoadImageData(IMG_CODE_UNIV_HASLOGIN_ICON, univ_haslogin_tga);
        if(status)
	    return(-1);
	imgs_loaded++;

        SplashDoUpdateProgress(imgs_loaded, total_imgs,
            "Loading Image: Icons...\n");
        status = IMGLoadImageData(IMG_CODE_UNIV_OLD_ICON, univ_old_tga);
        if(status)
            return(-1);   
        imgs_loaded++;

        SplashDoUpdateProgress(imgs_loaded, total_imgs,
            "Loading Image: Icons...\n");
        status = IMGLoadImageData(IMG_CODE_UNIV_UNKNOWN_ICON, univ_unknown_tga);
        if(status)
            return(-1);
        imgs_loaded++;


        /* Error icon. */
        SplashDoUpdateProgress(imgs_loaded, total_imgs,
            "Loading Image: Icons...\n");
        status = IMGLoadImageData(IMG_CODE_ERROR_ICON, error_tga);
        if(status)
	   return(-1);
	imgs_loaded++;
            
        /* Info icon. */
        SplashDoUpdateProgress(imgs_loaded, total_imgs,
            "Loading Image: Icons...\n");
        status = IMGLoadImageData(IMG_CODE_INFO_ICON, info_tga);
        if(status)
	    return(-1);
	imgs_loaded++;
  

        /* Energy saver mode banner. */
        SplashDoUpdateProgress(imgs_loaded, total_imgs,
            "Loading Image: Banners...");
        status = IMGLoadImageData(IMG_CODE_ENERGY_SAVER_MODE, esm_tga);
        if(status)
	    return(-1);
	imgs_loaded++;


	/* Stats console. */
	SplashDoUpdateProgress(imgs_loaded, total_imgs,
            "Loading Image: Bridge Consoles...");
        status = IMGLoadImage(IMG_CODE_STATS_CON1, IMGPATH_BPANEL_L1);
        if(status)
	    return(-1);
	imgs_loaded++;

        SplashDoUpdateProgress(imgs_loaded, total_imgs,
            "Loading Image: Bridge Consoles...");
        status = IMGLoadImage(IMG_CODE_STATS_CON2, IMGPATH_BPANEL_L2);
        if(status)
	    return(-1);
	imgs_loaded++;

        SplashDoUpdateProgress(imgs_loaded, total_imgs,
            "Loading Image: Bridge Consoles...");
        status = IMGLoadImage(IMG_CODE_STATS_CON3, IMGPATH_BPANEL_L3);
        if(status)
	    return(-1);
	imgs_loaded++;

        SplashDoUpdateProgress(imgs_loaded, total_imgs,
            "Loading Image: Bridge Consoles...");
        status = IMGLoadImage(IMG_CODE_STATS_CON4, IMGPATH_BPANEL_L4);
        if(status)
	    return(-1);
	imgs_loaded++;


        SplashDoUpdateProgress(imgs_loaded, total_imgs,
            "Loading Image: Bridge Consoles...");
        status = IMGLoadImage(IMG_CODE_SCANNER, IMGPATH_BPANEL_R1);
        if(status)
	    return(-1);
	imgs_loaded++;

	/* Scanner readout. */
        SplashDoUpdateProgress(imgs_loaded, total_imgs,
            "Loading Image: Bridge Consoles...");
        status = IMGLoadImage(IMG_CODE_SRO_CON1, IMGPATH_BPANEL_R2);
        if(status)
	    return(-1);
	imgs_loaded++;

        SplashDoUpdateProgress(imgs_loaded, total_imgs,
            "Loading Image: Bridge Consoles...");
        status = IMGLoadImage(IMG_CODE_SRO_CON2, IMGPATH_BPANEL_R3);
        if(status)
	    return(-1);
	imgs_loaded++;

        SplashDoUpdateProgress(imgs_loaded, total_imgs,
            "Loading Image: Bridge Consoles...");
        status = IMGLoadImage(IMG_CODE_SRO_CON3, IMGPATH_BPANEL_R4);
        if(status)
	    return(-1);
	imgs_loaded++;

        SplashDoUpdateProgress(imgs_loaded, total_imgs,
            "Loading Image: Bridge Consoles...");
        status = IMGLoadImage(IMG_CODE_MESG_CON, IMGPATH_BPANEL_MESG);
        if(status)
	    return(-1);
	imgs_loaded++;


	/* Bridge panel outlines. */
        SplashDoUpdateProgress(imgs_loaded, total_imgs,
            "Loading Image: Bridge Consoles...");
        status = IMGLoadImage(IMG_CODE_BPANEL_OL_HULL,
	    IMGPATH_BPANEL_OL_HULL);
        if(status)
            return(-1);
        imgs_loaded++;

        SplashDoUpdateProgress(imgs_loaded, total_imgs,
            "Loading Image: Bridge Consoles...");
        status = IMGLoadImage(IMG_CODE_BPANEL_OL_POWER,
	    IMGPATH_BPANEL_OL_POWER);
        if(status)
            return(-1);
        imgs_loaded++;

        SplashDoUpdateProgress(imgs_loaded, total_imgs,
            "Loading Image: Bridge Consoles...");
        status = IMGLoadImage(IMG_CODE_BPANEL_OL_VIS,
	    IMGPATH_BPANEL_OL_VIS);
        if(status)
            return(-1);
        imgs_loaded++;

        SplashDoUpdateProgress(imgs_loaded, total_imgs,
            "Loading Image: Bridge Consoles...");
        status = IMGLoadImage(IMG_CODE_BPANEL_OL_SHIELDS,
	    IMGPATH_BPANEL_OL_SHIELDS);
        if(status)
            return(-1);
        imgs_loaded++;

        SplashDoUpdateProgress(imgs_loaded, total_imgs,
            "Loading Image: Bridge Consoles...");
        status = IMGLoadImage(IMG_CODE_BPANEL_OL_DMGCTL,
	    IMGPATH_BPANEL_OL_DMGCTL);
        if(status)
            return(-1);
        imgs_loaded++;


        SplashDoUpdateProgress(imgs_loaded, total_imgs,
            "Loading Image: Bridge Consoles...");
        status = IMGLoadImage(IMG_CODE_BPANEL_OL_LTHROTTLE,
	    IMGPATH_BPANEL_OL_THROTTLE_L);
        if(status)
            return(-1);
        imgs_loaded++; 

        SplashDoUpdateProgress(imgs_loaded, total_imgs,
            "Loading Image: Bridge Consoles...");
        status = IMGLoadImage(IMG_CODE_BPANEL_OL_RTHROTTLE,
	    IMGPATH_BPANEL_OL_THROTTLE_R);
        if(status)
            return(-1);
        imgs_loaded++; 

        SplashDoUpdateProgress(imgs_loaded, total_imgs,
            "Loading Image: Bridge Consoles...");
        status = IMGLoadImage(IMG_CODE_BPANEL_OL_THRUSTVECTOR,
            IMGPATH_BPANEL_OL_THRUSTVECTOR);
        if(status)
            return(-1);
        imgs_loaded++;


	/* Starchart window. */
        SplashDoUpdateProgress(imgs_loaded, total_imgs,
            "Loading Image: Star Chart Icons...");
        status = IMGLoadImage(IMG_CODE_SCHT_ZOOM_IN, IMGPATH_SCHT_ZOOM_IN);
        if(status)
            return(-1);
        imgs_loaded++; 

        SplashDoUpdateProgress(imgs_loaded, total_imgs,
            "Loading Image: Star Chart Icons...");
        status = IMGLoadImage(IMG_CODE_SCHT_ZOOM_OUT, IMGPATH_SCHT_ZOOM_OUT);
        if(status)
            return(-1);
        imgs_loaded++; 

        SplashDoUpdateProgress(imgs_loaded, total_imgs,
            "Loading Image: Star Chart Icons...");
        status = IMGLoadImage(IMG_CODE_SCHT_JUMP_TO_PLAYER,
	    IMGPATH_SCHT_JUMP_TO_PLAYER);
        if(status)
            return(-1);
        imgs_loaded++; 



        /*   Large message screen background is laoded as needed
         *   later on.
	 */
	/* IMG_CODE_MESG_SCR_BKG */


	/* Lens flare 1. */
        SplashDoUpdateProgress(imgs_loaded, total_imgs,
            "Loading Image: Misc...");
        status = IMGLoadImage(IMG_CODE_LENSFLARE1,
            IMGPATH_LENSFLARE1);
        if(status)
            return(-1);
        imgs_loaded++;

        /* Lens flare 2. */
        SplashDoUpdateProgress(imgs_loaded, total_imgs,
            "Loading Image: Misc...");
        status = IMGLoadImage(IMG_CODE_LENSFLARE2,
            IMGPATH_LENSFLARE2);
        if(status)
            return(-1);
        imgs_loaded++;

        /* Strobe glow 1. */
        SplashDoUpdateProgress(imgs_loaded, total_imgs,
            "Loading Image: Misc...");
        status = IMGLoadImage(IMG_CODE_STROBEGLOW1,
            IMGPATH_STROBEGLOW1);
        if(status)
            return(-1);
        imgs_loaded++;


	return(0);
}



/*
 *	Procedure to free all images.
 */
void IMGDoUnloadAll()
{
	int i;


#ifdef DEBUG_MEM_FREE
printf("Internal Image: Deleting %i...\n", total_images);
#endif

        for(i = 0; i < total_images; i++)
        {
            if(xsw_image[i] == NULL)
		continue;


	    /* Deallocate all substructures. */
	    IMGUnload(i);

	    /* Free structure itself. */
#ifdef DEBUG_MEM_FREE
if(xsw_image[i] != NULL) 
    printf("Internal Image %i: Free'ed.\n", i);
#endif
            free(xsw_image[i]);
            xsw_image[i] = NULL;
        }

#ifdef DEBUG_MEM_FREE   
if(xsw_image != NULL)
    printf("Internal Image pointers: Free'ed.\n");
#endif
        free(xsw_image);
        xsw_image = NULL;

        total_images = 0;


	return;
}
