/*
 *	Example on using the TGA Referance Library.
 *
 *
 *	cc tga.c tgaexample.c -o tgaexample -lX11 -L/usr/X11/lib
 */

#include <stdio.h>
#include "../include/tga.h"

#include <X11/X.h>
#include <X11/Xlib.h>

/*
#include "toggle_btn0.h"
*/


int main(int argc, char *argv[])
{
	int status;
	Display *dpy;
	Window root_w, w;
	XImage *ximage;
	unsigned int depth;
	int scr_num;
	Screen *scr_ptr;
	GC gc;
	event_t xevent;
	tga_data_struct td;

	unsigned char *img_data;
	long img_len8;


	if(argc < 2)
	{
	    fprintf(stderr, "%s: What file do you want to display?\n",
		argv[0]
	    );
	    return(1);
	}


        /* Set up X resources. */
        dpy = XOpenDisplay(NULL);
        scr_num = DefaultScreen(dpy);
        scr_ptr = DefaultScreenOfDisplay(dpy);
        depth = DefaultDepthOfScreen(scr_ptr);
        gc = DefaultGC(dpy, scr_num);
        root_w = DefaultRootWindow(dpy);


	/* Load the image. */
	status = TgaReadFromFile(argv[1], &td, depth);
/*
	status = TgaReadFromData(toggle_btn0, &td, depth);
*/
	if(status != TgaSuccess)
	{
            fprintf(stderr,
		"%s: Cannot read image.\n",
                argv[1]
            );
	    XCloseDisplay(dpy); dpy = NULL;
	    return(1);
	}

	/* Create a segment of data to copy the loaded data to. */
	img_len8 = (long)td.width * (long)td.height *
	    (long)td.data_depth / 8;
	img_data = (unsigned char *)calloc(1, img_len8 * sizeof(unsigned char));
	memcpy(img_data, td.data, img_len8);

	w = XCreateSimpleWindow(dpy, root_w, 0, 0, td.width, td.height, 0,
	    BlackPixel(dpy, scr_num), BlackPixel(dpy, scr_num));
	XSelectInput(dpy, w, KeyPressMask | ButtonReleaseMask
            | ExposureMask);


	ximage = XCreateImage(
	    dpy,
	    DefaultVisual(dpy, scr_num),
	    depth,
	    ZPixmap,
	    0,
	    (char *)img_data,
	    td.width, td.height,
	    td.data_depth,
	    0
	);
	XMapRaised(dpy, w);

	/* We don't need any tga data after this point. */
        TgaDestroyData(&td);

        XPutImage(dpy, (Drawable)w, gc, ximage, 0, 0, 0, 0, ximage->width,
            ximage->height);

	while(1)
	{
	    XNextEvent(dpy, &xevent);

	    XPutImage(dpy, (Drawable)w, gc, ximage, 0, 0, 0, 0, ximage->width,
	        ximage->height);

	    if( (xevent.type == KeyPress) ||
                (xevent.type == ButtonRelease)
            )
		break;
	}


	XDestroyImage(ximage);
	XCloseDisplay(dpy); dpy = NULL;


	return(0);
}
