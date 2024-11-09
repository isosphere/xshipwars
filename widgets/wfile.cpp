// widgets/wfile.cpp
/*
                             Widget: File IO


	Functions:

	image_t *WidgetLoadImageFromTgaFile(char *filename)
	image_t *WidgetLoadImageFromTgaData(u_int8_t *data)

	pixmap_t WidgetLoadPixmapFromTgaFile(char *filename)
        pixmap_t WidgetLoadPixmapFromTgaData(u_int8_t *data)


        image_t *WidgetLoadImageFromXpmFile(char *filename)
	image_t *WidgetLoadImageFromXpmData(char **data)

	pixmap_t WidgetLoadPixmapFromXpmFile(char *filename)
        pixmap_t WidgetLoadPixmapFromXpmData(char **data)

	---


*/


#include "../include/widget.h"
#include "../include/tga.h"     /* For tga loading in wfile.c */

#include <X11/xpm.h>

#ifndef MAX
#define MIN(a,b)        (((a) < (b)) ? (a) : (b))
#define MAX(a,b)	(((a) > (b)) ? (a) : (b))
#endif
/* #define MIN(a,b)        (((a) < (b)) ? (a) : (b)) */
/* #define MAX(a,b)        (((a) > (b)) ? (a) : (b)) */


/*
 *      Loads an image from a TGA file.
 *      The image's depth will be that of the GUI's.
 */
image_t *WidgetLoadImageFromTgaFile(char *filename)
{
        tga_data_struct *td;
        image_t *image = NULL;
	void *ptr = NULL;
        off_t img_data_len = 0;
        int status = 0;


	/* Errors. */
	if(!IDC() ||
           (filename == NULL)
	)
	    return(image);


	/* Allocate tga data structure. */
	td = (tga_data_struct *)malloc(sizeof(tga_data_struct));
	if(td == NULL)
	    return(image);

	/* Load tga image from file. */
        status = TgaReadFromFile(filename, td, osw_gui[0].depth);
	if(status != TgaSuccess)
	{
	    fprintf(stderr, "WidgetLoadImageFromTgaFile(): %s: ", filename);
            switch(status)
            {
              case TgaNoBuffers:
                fprintf(stderr, "No buffers.\n");
                break;

              case TgaBadHeader:
                fprintf(stderr, "Bad header.\n");
                break;

              case TgaBadValue:
                fprintf(stderr, "Bad value.\n");
                break;

              case TgaNoFile:
                fprintf(stderr, "No such file.\n");
                break;

              case TgaNoAccess:
                fprintf(stderr, "No access.\n");
                break;

              default:
		fprintf(stderr, "\n");
                break;
	    }

	    TgaDestroyData(td);
	    free(td);

            return(image);
        }


        /*   Copy data in td to GUI image.
	 *
         *   Targa library loads data to memory in ZPixmap format
         *   of the requested depth.
	 */

	switch(osw_gui[0].depth)
	{
	  /* 8 bits. */
	  case 8:
	    if(OSWCreateImage(&image, td->width, td->height))
            {
	        fprintf(
		    stderr,
		    "%s: Error: Cannot allocate GUI image.\n",
		    filename
		);
            }
	    else
            {
	        img_data_len = MIN(
		    (int)(image->width * image->height * BYTES_PER_PIXEL8),
		    (int)(td->width * td->height * BYTES_PER_PIXEL8)
                );

		ptr = memcpy(
		    image->data,        /* Target. */
		    td->data,            /* Source. */
		    img_data_len
		);
            }
	    break;

	  /* 15 or 16 bits. */
	  case 15:
	  case 16:
            if(OSWCreateImage(&image, td->width, td->height))
            {
                fprintf(stderr,
                    "%s: Error: Cannot allocate GUI image.\n",
                    filename
                );
            }
            else
            {
                img_data_len = MIN(
                    (int)(image->width * image->height * BYTES_PER_PIXEL16),
                    (int)(td->width * td->height * BYTES_PER_PIXEL16)
                );

                ptr = memcpy(
                    image->data,	/* Target. */
                    td->data,		/* Source. */
                    img_data_len
                );
            }
	    break;

          /* 24 or 32 bits. */
	  case 24:
	  case 32:
	    if(OSWCreateImage(&image, td->width, td->height))
	    {
                fprintf(stderr,
                    "%s: Error: Cannot allocate GUI image.\n",
                    filename
                );
            } 
	    else
	    {
                img_data_len = MIN(
                    (int)(image->width * image->height * BYTES_PER_PIXEL32),
		    (int)(td->width * td->height * BYTES_PER_PIXEL32)
		);

                ptr = memcpy(
                    image->data,	/* Target. */
                    td->data,		/* Source. */
                    img_data_len
                );
	    }
        }


        /* Destroy tga data. */
        TgaDestroyData(td);
	free(td);


        return(image);
}


/*
 *	Loads an image from TGA data in memory.
 *	The image's depth will be that of the GUI's.
 */
image_t *WidgetLoadImageFromTgaData(u_int8_t *data)
{
        tga_data_struct *td;
        image_t *image = NULL;
        off_t img_data_len = 0;
        int status = 0;


	/* Error checks. */
	if(!IDC() ||
           (data == NULL)
	)
	    return(image);


        /* Allocate tga data structure. */
        td = (tga_data_struct *)malloc(sizeof(tga_data_struct));
        if(td == NULL)
            return(image);

        /* Load tga image from data. */
        status = TgaReadFromData(data, td, osw_gui[0].depth);
        if(status != TgaSuccess)
        {
            fprintf(stderr, "WidgetLoadImageFromTgaData(): 0x%.8x: ", (u_int32_t)data);
            switch(status)
            {
              case TgaNoBuffers:
                fprintf(stderr, "No buffers.\n");
                break;

              case TgaBadHeader:
                fprintf(stderr, "Bad header.\n");
                break;

              case TgaBadValue:
                fprintf(stderr, "Bad value.\n");
                break;

              case TgaNoAccess:
                fprintf(stderr, "No access.\n");
                break;

              default:
                fprintf(stderr, "\n");
                break;
            }

            TgaDestroyData(td);
            free(td);

            return(image);
        }


        /*   Copy data in td to GUI image.
         *
         *   Targa library loads data to memory in ZPixmap format
         *   of the requested depth.
         */

	switch(osw_gui[0].depth)
	{
	  /* 8 bits. */
	  case 8:
            if(OSWCreateImage(&image, td->width, td->height))
            {
                fprintf(stderr,
                    "0x%.8x: Error: Cannot allocate GUI image.\n",
                    (u_int32_t)data
                );
            }
            else
            {
                img_data_len = MIN(
                    (int)(image->width * image->height * BYTES_PER_PIXEL8),
                    (int)(td->width * td->height * BYTES_PER_PIXEL8)
                );

                memcpy(
                    image->data,	/* Target. */
                    td->data,		/* Source. */
                    img_data_len
                );
            }
	    break;

	  /* 15 or 16 bits. */
	  case 15:
	  case 16:
	    if(OSWCreateImage(&image, td->width, td->height))
            {
                fprintf(stderr,
                    "0x%.8x: Error: Cannot allocate GUI image.\n",
                    (u_int32_t)data
                );
            }
	    else
            {
	        img_data_len = MIN(
		    (int)(image->width * image->height * BYTES_PER_PIXEL16),
		    (int)(td->width * td->height * BYTES_PER_PIXEL16)
		);

	        memcpy(
		    image->data,	/* Target. */
		    td->data,		/* Source. */
		    img_data_len
		);
            }
	    break;

          /* 24 or 32 bits. */
	  case 24:
	  case 32:
	    if(OSWCreateImage(&image, td->width, td->height))
            {
                fprintf(stderr,
                    "0x%.8x: Error: Cannot allocate GUI image.\n",
                    (u_int32_t)data
                );
            }
	    else  
            {
	        img_data_len = MIN(
		    (int)(image->width * image->height * BYTES_PER_PIXEL32),
		    (int)(td->width * td->height * BYTES_PER_PIXEL32)
		);

                memcpy(
                    image->data,	/* Target. */
                    td->data,		/* Source. */
                    img_data_len
                );
            }
        }


        /* Destroy tga data. */
        TgaDestroyData(td);
	free(td);


        return(image);
}


/*
 *      Loads a pixmap from a TGA file.
 *      The pixmap's depth will match that of the GUI's.
 */
pixmap_t WidgetLoadPixmapFromTgaFile(char *filename)
{
        image_t *image;
        pixmap_t pixmap = 0;


        image = WidgetLoadImageFromTgaFile(filename);
        if(image == NULL)
            return(pixmap);

        pixmap = OSWCreatePixmapFromImage(image);
        OSWDestroyImage(&image);

        return(pixmap);
}


/*
 *      Loads a pixmap from TGA data in memory.
 *      The pixmap's depth will match that of the GUI's.
 */
pixmap_t WidgetLoadPixmapFromTgaData(u_int8_t *data)
{
	image_t *image;
	pixmap_t pixmap = 0;


	image = WidgetLoadImageFromTgaData(data);
	if(image == NULL)
	    return(pixmap);

	pixmap = OSWCreatePixmapFromImage(image);
	OSWDestroyImage(&image);

	return(pixmap);
}


/*
 *	Loads an image from an XPM file.
 *	The image's depth will match that of the GUI's.
 */
image_t *WidgetLoadImageFromXpmFile(char *filename)
{
        int status;
        image_t *image;
        image_t *imagemask;
        XpmAttributes xpmattr;


        /* Error checks. */
        if(!IDC() ||
           (osw_gui[0].root_win == 0) ||
           (filename == NULL)
        )
            return(0);


        /* Set XPM attributes. */
        memset(&xpmattr, 0x00, sizeof(XpmAttributes));
        xpmattr.valuemask = XpmSize | XpmCloseness | XpmDepth;
        xpmattr.closeness = XpmDefaultColorCloseness;
	xpmattr.depth = osw_gui[0].depth;


	/* Load image from XPM file. */
        status = XpmReadFileToImage(
            osw_gui[0].display,
            filename,
            &image,
            &imagemask,
            &xpmattr
        );
        if(status != XpmSuccess)
        {
	    fprintf(stderr, "WidgetLoadImageFromXpmFile(): ");
            fprintf(stderr, "%s: Failed load.\n", filename);

            return(0);
        }

        /* Destroy the mask image. */
	OSWDestroyImage(&imagemask);


	return(image);
}


/*
 *      Loads an image from XPM data in memory.
 *      The image's depth will match that of the GUI's.
 */
image_t *WidgetLoadImageFromXpmData(char **data)
{
        int status;
        image_t *image;
        image_t *imagemask;
        XpmAttributes xpmattr;


        /* Error checks. */
        if(!IDC() ||
           (osw_gui[0].root_win == 0) ||
           (data == NULL)
        )
            return(0);


        /* Set XPM attributes. */   
        memset(&xpmattr, 0x00, sizeof(XpmAttributes));
        xpmattr.valuemask = XpmSize | XpmCloseness | XpmDepth;
        xpmattr.closeness = XpmDefaultColorCloseness;
        xpmattr.depth = osw_gui[0].depth;

	/* Load image from XPM data. */
        status = XpmCreateImageFromData(
            osw_gui[0].display,
            data,
            &image,
            &imagemask,
            &xpmattr
        );
        if(status != XpmSuccess)
        {
            fprintf(stderr, "WidgetLoadImageFromXpmData(): ");
            fprintf(stderr, "0x%.8x: Failed load.\n",
		(u_int32_t)data
	    );

            return(0);
        }

	/* Destroy the mask image. */
        OSWDestroyImage(&imagemask);


        return(image);
}



/*
 *      Loads a pixmap from an XPM file.
 *      The pixmap's depth will match that of the GUI's.
 */
pixmap_t WidgetLoadPixmapFromXpmFile(char *filename)
{
	int status;
        pixmap_t pixmap = 0;
        pixmap_t pixmapmask = 0;
        XpmAttributes xpmattr;


	/* Error checks. */
	if(!IDC() ||
	   (osw_gui[0].root_win == 0) ||
	   (filename == NULL)
	)
	    return(0);


        /* Set XPM attributes. */
	memset(&xpmattr, 0x00, sizeof(XpmAttributes));
	xpmattr.valuemask = XpmSize | XpmCloseness | XpmDepth;
        xpmattr.closeness = XpmDefaultColorCloseness;
	xpmattr.depth = osw_gui[0].depth;


        /* Attempt to read the pixmap data from file. */
	status = XpmReadFileToPixmap(
	    osw_gui[0].display,
	    osw_gui[0].root_win,
	    filename,
	    &pixmap,
	    &pixmapmask,
	    &xpmattr
	);
	if(status != XpmSuccess)
        {
            fprintf(stderr, "WidgetLoadPixmapFromXpmFile(): ");
            fprintf(stderr, "%s: Failed load.\n", filename);

            return(0);
        }

	/* Destroy the mask, we don't need it. */
	OSWDestroyPixmap(&pixmapmask);


	return(pixmap);
}


/*
 *      Loads a pixmap from XPM data in memory.
 *      The pixmap's depth will match that of the GUI's.
 */
pixmap_t WidgetLoadPixmapFromXpmData(char **data)  
{
        int status;  
        pixmap_t pixmap = 0;
        pixmap_t pixmapmask = 0;
        XpmAttributes xpmattr;


	/* Error checks. */
        if(!IDC() ||
           (osw_gui[0].root_win == 0) ||
           (data == NULL)
        )
            return(0);


        /* Set XPM attributes. */
        memset(&xpmattr, 0x00, sizeof(XpmAttributes));
        xpmattr.valuemask = XpmSize | XpmCloseness | XpmDepth;
        xpmattr.closeness = XpmDefaultColorCloseness;
	xpmattr.depth = osw_gui[0].depth;

        /* Attempt to read the pixmap data from file. */
        status = XpmCreatePixmapFromData(
            osw_gui[0].display,
            osw_gui[0].root_win,
            data,
            &pixmap,
            &pixmapmask,
            &xpmattr
        );
        if(status != XpmSuccess)
        {
            fprintf(stderr, "WidgetLoadPixmapFromXpmData(): ");
            fprintf(stderr, "0x%.8x: Failed load.\n",
                (u_int32_t)data
            );

            return(0);
        }

        /* Destroy the mask, we don't need it. */
        OSWDestroyPixmap(&pixmapmask);


        return(pixmap);
}




