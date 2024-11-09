/*
                         X Operating System Wrapper

	Macros:



	Init:

	int OSWGUIConnect(int argc, char *argv[])
	void OSWGUIDisconnect()
	int OSWLoadKeyCodes()


	Keycodes:

	char OSWGetASCIIFromKeyCode(
		key_event_t ke,
		bool_t shift,
		bool_t alt,
		bool_t ctrl
	)
	char *OSWGetKeyCodeName(keycode_t keycode)
	void OSWManageEvent(event_t *event)
        int OSWIsModifierKey(keycode_t keycode)


	GUI systems and runtime:

	void OSWGUISync(bool_t discard)
	void OSWGetPointerCoords(
	        win_t w,
	        int *root_x, int *root_y,
	        int *wx, int *wy
	)
	void OSWGrabPointer(
		win_t grab_w,
		bool_t events_rel_grab_w,
		eventmask_t eventmask,
		int pointer_mode, int keyboard_mode,
		win_t confine_w,
		cursot_t cursor
	)
	void OSWUngrabPointer()
	void OSWGUIFree(void **ptr)
	void *OSWFetchDDE(int *bytes)
	void OSWPutDDE(void *buf, int bytes)
	visual_t *OSWGetVisualByCriteria(
	        long vinfo_mask,
	        visual_info_t criteria_vinfo
	)
	visual_t *OSWGetVisualByID(visual_id_t vid) 
	void OSWKBAutoRepeatOff()
        void OSWKBAutoRepeatOn()


	Fonts:

	int OSWLoadFont(font_t **font, const char *fontname)
	font_t *OSWQueryCurrentFont()
	void OSWSetFont(font_t *font)
	void OSWUnloadFont(font_t **font)


	Pixels (color allocation):

	int OSWLoadPixelRGB(pixel_t *pix_rtn,
		u_int8_t r,
		u_int8_t g,
		u_int8_t b
	)
        int OSWLoadPixelHSL(pixel_t *pix_rtn,
                u_int8_t h,
                u_int8_t s,
                u_int8_t l
        )
	int OSWLoadPixelCLSP(pixel_t *pix_rtn, char *clsp)
	void OSWDestroyPixel(pixel_t *pix_ptr)

	void OSWSetFgPix(pixel_t pix)


	Cursor (pointer cursor):

	cursor_t OSWLoadBasicCursor(cur_code_t code)
	void OSWSetWindowCursor(win_t w, cursor_t cursor)
	void OSWUnsetWindowCursor(win_t w)
	void OSWDestroyCursor(cursor_t *cursor)


	Events:

	int OSWEventsPending()
	void OSWWaitNextEvent(event_t *event)
        void OSWWaitPeakEvent(event_t *event)
	bool_t OSWCheckMaskEvent(eventmask_t eventmask, event_t *event)
	void OSWWaitWindowEvent(
		win_t w, eventmask_t event_mask, event_t *event
	)
	bool_t OSWCheckWindowEvent(
		win_t w, eventmask_t event_mask, event_t *event
	)
	void OSWPutBackEvent(event_t *event)
	int OSWSendEvent(
	        eventmask_t mask,
	        event_t *event,   
	        bool_t propagate
	)
	int OSWPurgeAllEvents()
	int OSWPurgeOldMotionEvents()
	int OSWPurgeTypedEvent(int event_type)
	int OSWPurgeWindowTypedEvent(win_t w, eventtype_t event_type)

	int OSWIsEventDestroyWindow(win_t w, event_t *event)


	Window:

	int OSWCreateWindow(
	        win_t *w,
	        win_t parent,
	        int x, int y,
	        unsigned int width, unsigned int height
	)
        int OSWCreateInputWindow(
                win_t *w,
                win_t parent,
                int x, int y,
                unsigned int width, unsigned int height
        )
	void OSWDestroyWindow(win_t *w)
	bool_t OSWDrawableIsWindow(drawable_t d)
	void OSWSetWindowIcon(win_t w, pixmap_t icon, bool_t has_transparency)
	void OSWSetWindowWMProperties(
	        win_t w,
	        char *title,
	        char *icon_title,
	        pixmap_t icon,
	        bool_t wm_sets_coordinates,
		int x, int y,
	        unsigned int min_width, unsigned int min_height,
	        unsigned int max_width, unsigned int max_height,
	        int frame_style,
	        char **argv, int argc
	)
	void OSWSetWindowInput(win_t w, eventmask_t eventmask)
	void OSWSetTransientFor(win_t wbum, win_t wshelter)
	void OSWMapWindow(win_t w)
	void OSWMapRaised(win_t w)
	void OSWMapSubwindows(win_t w)
	void OSWUnmapWindow(win_t w)
	void OSWRestackWindows(win_t *w, int num_w)
	int OSWIconifyWindow(win_t w)
	void OSWReparentWindow(win_t w, win_t parent);
	void OSWMoveWindow(win_t w, int x, int y)
	void OSWResizeWindow(win_t w, unsigned int width, unsigned int height)
	void OSWMoveResizeWindow(win_t w, int x, int y,
	        unsigned int width, unsigned int height)
	void OSWSetWindowTitle(Window w, char *title)
	void OSWClearWindow(win_t w)
	void OSWSetWindowBkg(win_t w, pixel_t pix, pixmap_t pixmap)
	bool_t OSWGetWindowAttributes(win_t w, win_attr_t *wattr)
	int OSWGetWindowRootPos(win_t w, int *x, int *y);
	win_t OSWGetWindowParent(win_t w)
	bool_t OSWCheckWindowAncestory(win_t grand_parent, win_t grand_child)


	Pixmap (graphics buffer):
  
	int OSWCreatePixmap(pixmap_t *pixmap, unsigned int width,
		unsigned int height)
	void OSWDestroyPixmap(pixmap_t *pixmap)
	bool_t OSWDrawableIsPixmap(drawable_t d)
	bool_t OSWGetPixmapAttributes(
		pixmap_t pixmap,
		 pixmap_attr_t *pattr
	)
	void OSWClearPixmap(
		pixmap_t pixmap,
		unsigned int width, unsigned int height, pixel_t pix
	)

	Images:

	int OSWCreateImage(
	        image_t **image,
	        unsigned int width, unsigned int height
	)
	void OSWDestroyImage(image_t **image)


	Shared images:

	int OSWCreateSharedImage(
	        shared_image_t **image,
	        unsigned int width,
	        unsigned int height
	)
	void OSWDestroySharedImage(
	        shared_image_t **image
	)


	Graphics conversions:

	pixmap_t OSWCreatePixmapFromImage(image_t *image)


	Graphics IO:

        image_t *OSWGetImage(drawable_t d, int x, int y,
		unsigned int width, unsigned int height)
	void OSWPutImageToDrawable(image_t *image, drawable_t d)
	void OSWPutImageToDrawablePos(
		image_t *image, drawable_t d,
		int tar_x, int tar_y
	)
	void OSWPutImageToDrawableSect(
	        image_t *image, drawable_t d,
	        int tar_x, int tar_y,
	        int src_x, int src_y,
	        unsigned int width, unsigned int height
	)
	void OSWPutSharedImageToDrawable(
		shared_image_t *image,
		drawable_t d
	)
	void OSWPutSharedImageToDrawablePos(
		shared_image_t *image, drawable_t d,
		int tar_x, int tar_y
	)
	void OSWPutSharedImageToDrawableSect(
	        shared_image_t *image, drawable_t d,
	        int tar_x, int tar_y,
	        int src_x, int src_y,
	        unsigned int width, unsigned int height
	)
	void OSWSyncSharedImage(shared_image_t *image, drawable_t d)
	void OSWPutBufferToWindow(win_t w, gbuf_t gbuf)
	void OSWCopyDrawables(
	    drawable_t tar_d,
	    drawable_t src_d,
	    unsigned int width,
	    unsigned int height
	)
        void OSWCopyDrawablesCoord(
            drawable_t tar_d,
            drawable_t src_d,
	    int tar_x, int tar_y,
	    unsigned int width, unsigned int height,
            int src_x, int src_y
        )


	Drawing:

	void OSWDrawString(drawable_t d, int x, int y, const char *string)
	void OSWDrawStringLimited(drawable_t d, int x, int y,
		const char *string, int len
	)
	void OSWDrawLine(drawable_t d, int start_x, int start_y,
	        int end_x, int end_y)
	void OSWDrawRectangle(drawable_t d, int x, int y,
	        unsigned int width, unsigned int height)
	void OSWDrawSolidRectangle(drawable_t d, int x, int y,
	        unsigned int width, unsigned int height)
	void OSWDrawArc(drawable_t d, int x, int y,
	        unsigned int width, unsigned int height,
	        double position_angle,
	        double terminal_angle
	)
	void OSWDrawSolidArc(drawable_t d, int x, int y,
	        unsigned int width, unsigned int height,
	        double position_angle,
	        double terminal_angle
	)


	Misc:

	int OSWGUIErrorHandler(
		Display *dpy,
		XErrorEvent *error_event
	)
	int OSWGUISimpleErrorHandler(
		Display *dpy,
		XErrorEvent *error_event
	)

	---

*/

#include <stdio.h>
#include <stdlib.h>   
#include <string.h>
#include <unistd.h>
#include <sys/types.h>


/* OS Makeup. */

#include "../include/os.h"

/* Byte packing and bytes per pixel. */

#include "../include/graphics.h"
#include "../include/string.h"

namespace static_osw_x {
	int gui_error_code;

	char *keytable[] = {
		"tab",		(char *)'\t',
		"space",        (char *)' ',
		"exclam",       (char *)'!',
		"quotedbl",     (char *)'"',
		"numbersign",   (char *)'#',
		"dollar",       (char *)'$',
		"percent",      (char *)'%',
		"ampersand",    (char *)'&',
		"apostrophe",   (char *)'\'',
		"quoteright",   (char *)'"',
		"parenleft",    (char *)'(',
		"parenright",   (char *)')',
		"asterisk",     (char *)'*',
		"plus",         (char *)'+',
		"comma",        (char *)',',
		"minus",        (char *)'-',
		"period",       (char *)'.',
		"slash",        (char *)'/',
		"colon",        (char *)':',
		"semicolon",    (char *)';',
		"less",         (char *)'<',
		"equal",        (char *)'=',
		"greater",      (char *)'>',
		"question",     (char *)'?',
		"at",           (char *)'@',
		"bracketleft",  (char *)'[',
		"backslash",    (char *)'\\',
		"bracketright", (char *)']',
		"asciicircum",  (char *)'^',
		"underscore",   (char *)'_',
		"grave",        (char *)'\0',
		"quoteleft",    (char *)'"',
		"braceleft",    (char *)'{',
		"bar",          (char *)'|',
		"braceright",   (char *)'}',
		"asciitilde",   (char *)'~',

		"0",            (char *)'0',
		"1",            (char *)'1',
		"2",            (char *)'2',
		"3",            (char *)'3',
		"4",            (char *)'4',
		"5",            (char *)'5',
		"6",            (char *)'6',
		"7",            (char *)'7',
		"8",            (char *)'8',
		"9",            (char *)'9',

		"A",            (char *)'A',
		"B",            (char *)'B',
		"C",            (char *)'C',
		"D",            (char *)'D', 
		"E",            (char *)'E',
		"F",            (char *)'F',
		"G",            (char *)'G',
		"H",            (char *)'H',
		"I",            (char *)'I',
		"J",            (char *)'J', 
		"K",            (char *)'K',
		"L",            (char *)'L',
		"M",            (char *)'M', 
		"N",            (char *)'N',
		"O",            (char *)'O',
		"P",            (char *)'P',
		"Q",            (char *)'Q',
		"R",            (char *)'R',
		"S",            (char *)'S', 
		"T",            (char *)'T',
		"U",            (char *)'U',
		"V",            (char *)'V',
		"W",            (char *)'W',
		"X",            (char *)'X',
		"Y",            (char *)'Y',
		"Z",            (char *)'Z',

		"a",            (char *)'a',
		"b",            (char *)'b',
		"c",            (char *)'c',
		"d",            (char *)'d',
		"e",            (char *)'e',
		"f",            (char *)'f',
		"g",            (char *)'g',
		"h",            (char *)'h',
		"i",            (char *)'i',
		"j",            (char *)'j',
		"k",            (char *)'k',
		"l",            (char *)'l',
		"m",            (char *)'m',
		"n",            (char *)'n',
		"o",            (char *)'o',
		"p",            (char *)'p',
		"q",            (char *)'q',
		"r",            (char *)'r',
		"s",            (char *)'s',
		"t",            (char *)'t',
		"u",            (char *)'u',
		"v",            (char *)'v',
		"w",            (char *)'w',
		"x",            (char *)'x',
		"y",            (char *)'y',
		"z",            (char *)'z'
	};
}

#include "../include/osw-x.h"

/* Needed for OSWSetWindowWMProperties(). */
#include "../include/MwmUtil.h"

#ifndef MAX
#define MIN(a,b)        (((a) < (b)) ? (a) : (b))
#define MAX(a,b)	(((a) > (b)) ? (a) : (b))
#endif
/* #define MIN(a,b)        ((a) < (b) ? (a) : (b)) */
/* #define MAX(a,b)        ((a) > (b) ? (a) : (b)) */



osw_gui_struct osw_gui[1];
osw_keycode_struct osw_keycode;
osw_atom_struct osw_atom;



/*
 *	XShmGetEventBase() needs to be prototyped,
 *	it is apparently not defined in any of the available
 *	header files.
 */
//extern int XShmGetEventBase(Display *display);
extern "C" int XShmGetEventBase(Display *display);


/*
 *	GUI Error handlers.
 */
int OSWGUIErrorHandler(Display *dpy, XErrorEvent *error_event);
int OSWGUISimpleErrorHandler(Display *dpy, XErrorEvent *error_event);
/* GUI error code, set by the GUI error handlers. */
//static int gui_error_code;


/*
 *	GUI wrapper's local event manager.
 */
void OSWManageEvent(event_t *event);

/*
 *	GUI wrapper's function for setting WM icons on toplevel windows.
 */
void OSWSetWindowIcon(win_t w, pixmap_t icon, bool_t has_transparency);



/*
 *      Keysym name to character value table, even numbers are pointers
 *      to strings while odd numbers are pointers who's value is a
 *      character value.
 *
static char *keytable[] = {
	"tab",		(char *)'\t',
        "space",        (char *)' ',
        "exclam",       (char *)'!',
        "quotedbl",     (char *)'"',
        "numbersign",   (char *)'#',
        "dollar",       (char *)'$',
        "percent",      (char *)'%',
        "ampersand",    (char *)'&',
        "apostrophe",   (char *)'\'',
        "quoteright",   (char *)'"',
        "parenleft",    (char *)'(',
        "parenright",   (char *)')',
        "asterisk",     (char *)'*',
        "plus",         (char *)'+',
        "comma",        (char *)',',
        "minus",        (char *)'-',
        "period",       (char *)'.',
        "slash",        (char *)'/',
        "colon",        (char *)':',
        "semicolon",    (char *)';',
        "less",         (char *)'\0',
        "equal",        (char *)'=',
        "greater",      (char *)'>',
        "question",     (char *)'?',
        "at",           (char *)'@',
        "bracketleft",  (char *)'[',
        "backslash",    (char *)'\\',
        "bracketright", (char *)']',
        "asciicircum",  (char *)'^',
        "underscore",   (char *)'_',
        "grave",        (char *)'\0',
        "quoteleft",    (char *)'"',
        "braceleft",    (char *)'{',
        "bar",          (char *)'|',
        "braceright",   (char *)'}',
        "asciitilde",   (char *)'~',

        "0",            (char *)'0',
        "1",            (char *)'1',
        "2",            (char *)'2',
        "3",            (char *)'3',
        "4",            (char *)'4',
        "5",            (char *)'5',
        "6",            (char *)'6',
        "7",            (char *)'7',
        "8",            (char *)'8',
        "9",            (char *)'9',

        "A",            (char *)'A',
        "B",            (char *)'B',
        "C",            (char *)'C',
        "D",            (char *)'D', 
        "E",            (char *)'E',
        "F",            (char *)'F',
        "G",            (char *)'G',
        "H",            (char *)'H',
        "I",            (char *)'I',
        "J",            (char *)'J', 
        "K",            (char *)'K',
        "L",            (char *)'L',
        "M",            (char *)'M', 
        "N",            (char *)'N',
        "O",            (char *)'O',
        "P",            (char *)'P',
        "Q",            (char *)'Q',
        "R",            (char *)'R',
        "S",            (char *)'S', 
        "T",            (char *)'T',
        "U",            (char *)'U',
        "V",            (char *)'V',
        "W",            (char *)'W',
        "X",            (char *)'X',
        "Y",            (char *)'Y',
        "Z",            (char *)'Z',

        "a",            (char *)'a',
        "b",            (char *)'b',
        "c",            (char *)'c',
        "d",            (char *)'d',
        "e",            (char *)'e',
        "f",            (char *)'f',
        "g",            (char *)'g',
        "h",            (char *)'h',
        "i",            (char *)'i',
        "j",            (char *)'j',
        "k",            (char *)'k',
        "l",            (char *)'l',
        "m",            (char *)'m',
        "n",            (char *)'n',
        "o",            (char *)'o',
        "p",            (char *)'p',
        "q",            (char *)'q',
        "r",            (char *)'r',
        "s",            (char *)'s',
        "t",            (char *)'t',
        "u",            (char *)'u',
        "v",            (char *)'v',
        "w",            (char *)'w',
        "x",            (char *)'x',
        "y",            (char *)'y',
        "z",            (char *)'z'
};
*/


/*
 *	Graphics blitting lookup table generator.
 *	called by OSWGUIConnect().
 */
u_int16_t gRLookup[256];
u_int16_t gGLookup[256];
u_int16_t gBLookup[256];
void OSWGenerateColorLookupTable(u_int32_t BitDepth)
{
	int i;


        switch( BitDepth )
        {
          case 15:
            for(i = 0; i < 256; i++)
            {
                gRLookup[i] = (i & 0xF8) << 7;
                gGLookup[i] = (i & 0xF8) << 2;
                gBLookup[i] = i >> 3;
            }
            break;

          case 16:
            for(i = 0; i < 256; i++)
            {
                gRLookup[i] = (i & 0xF8) << 8;
                gGLookup[i] = (i & 0xFC) << 3;
                gBLookup[i] = i >> 3;
            }
            break;

          default:
	    break;
        }

        return;
}

/*
 *	Connect to GUI.
 */
int OSWGUIConnect(int argc, char *argv[])
{
	osw_gui_struct *gui;
	int i, status;
	u_int8_t r, g, b;
	visual_id_t vid = 0x00;
	atom_t mwm_hints_atom;
	char *host = NULL;
	char *fg_pix_str = NULL;
        char *bg_pix_str = NULL;
	char *def_font_name = NULL;
	visual_t *vis_ptr = NULL;
/*
	visual_info_t vinfo;
	visual_id_t vid;
 */
        depth_t depth = 0;
	XColor xc;


	/* Get pointer to gui connection structure. */
	gui = &osw_gui[0];

	/* Reset values. */
        gui->std_font = NULL;
	gui->bold_font = NULL;

        gui->display_width = 0;
        gui->display_height = 0;

        gui->black_pix = 0;
        gui->white_pix = 0;

        gui->std_cursor = 0;
        gui->root_win = 0;

        gui->current_font = NULL;
        gui->current_fg_pix = 0;
        gui->current_bg_pix = 0;
        gui->current_alpha_pix = 0;


        gui->def_font = NULL;
        gui->def_bg_pix = 0;
        gui->def_fg_pix = 0;
        gui->def_toplevel_x = 0;
        gui->def_toplevel_y = 0;
        gui->def_toplevel_width = 0;
        gui->def_toplevel_height = 0;
        gui->def_geometry_set = False;
#ifdef USE_XSHM
	gui->def_use_shm = True;
	gui->def_sync_shm = True;	/* Default to True, so mandatory syncing. */
#else
	gui->def_use_shm = False;
	gui->def_sync_shm = False;
#endif

        gui->vendor_id = NULL;
        gui->vendor_version = 0;
        gui->vendor_release_version = 0;


	/* Parse arguments. */
	for(i = 0; i < argc; i++)
	{
	    if(argv[i] == NULL)
		continue;

	    /* Display. */
	    if(!strcmp(argv[i], "--display") ||
               !strcmp(argv[i], "--dpy") ||
               !strcmp(argv[i], "-display") ||
               !strcmp(argv[i], "-dpy")
	    )
	    {
		i++;
		if(i < argc)
		{
		    host = argv[i];
		}
		else
		{
		    fprintf(
			stderr,
			"%s: Requires argument.\n",
			argv[i - 1]
		    );
		}
	    }
	    /* Visual. */
            else if(!strcmp(argv[i], "--visual") ||
                    !strcmp(argv[i], "-visual")
	    )
	    {
                i++;
                if(i < argc)
                {
		    sscanf(argv[i], "%ld", &vid);
                }
                else
                {
                    fprintf(
                        stderr,
                        "%s: Requires argument.\n",
                        argv[i - 1]
                    );
                }
	    }
	    /* Depth. */
	    else if(!strcmp(argv[i], "--depth") ||
                    !strcmp(argv[i], "-depth")
            )
	    {
                i++;
                if(i < argc)
                {
		    depth = atoi(argv[i]);
                }
                else
                {
                    fprintf(
                        stderr,
                        "%s: Requires argument.\n",
                        argv[i - 1]
                    );
                }
            }
            /* Geometry. */
            else if(!strcmp(argv[i], "--geometry") ||
                    !strcmp(argv[i], "-geometry")
            )
            {
                i++;
                if(i < argc)
                {
                    XParseGeometry(argv[i],
                        &gui->def_toplevel_x,
                        &gui->def_toplevel_y,
                        &gui->def_toplevel_width,
                        &gui->def_toplevel_height
                    );
                    gui->def_geometry_set = True;
		}
		else
		{
                    fprintf(
                        stderr,
                        "%s: Requires argument.\n",
                        argv[i - 1]
                    );
		}
            }
            /* Default font (not the standard font). */
            else if(!strcmp(argv[i], "--font") ||
                    !strcmp(argv[i], "-font") ||
                    !strcmp(argv[i], "--fn") || 
                    !strcmp(argv[i], "-fn")
            )
            {
                i++;
                if(i < argc)
                {
		    def_font_name = argv[i];
                }
                else
                {
                    fprintf(
                        stderr,
                        "%s: Requires argument.\n",
                        argv[i - 1]
                    );
                }
            }
	    /* Background pixel. */
            else if(!strcmp(argv[i], "--background") ||
                    !strcmp(argv[i], "-background") ||
                    !strcmp(argv[i], "--bg") ||
                    !strcmp(argv[i], "-bg")
            )
            {
                i++;
                if(i < argc)
                {
		    bg_pix_str = argv[i];
                }
                else
                {
                    fprintf(
                        stderr,
                        "%s: Requires argument.\n",
                        argv[i - 1]
                    );
                }
            }           
            /* Foreground pixel. */
            else if(!strcmp(argv[i], "--foreground") ||
                    !strcmp(argv[i], "-foreground") ||
                    !strcmp(argv[i], "--fg") ||
                    !strcmp(argv[i], "-fg")      
            )
            {
                i++;
                if(i < argc)
                {
                    fg_pix_str = argv[i];
                }
                else
                {   
                    fprintf(
                        stderr,
                        "%s: Requires argument.\n",
                        argv[i - 1]
                    );
                }
            }
            /* No shared memory. */
            else if(!strcmp(argv[i], "--no_shm") ||
                    !strcmp(argv[i], "-no_shm") ||
                    !strcmp(argv[i], "--noshm") ||
                    !strcmp(argv[i], "-no_shm") ||
                    !strcmp(argv[i], "--no_xshm") ||
                    !strcmp(argv[i], "-no_xshm") ||
                    !strcmp(argv[i], "--noxshm") ||
                    !strcmp(argv[i], "-noxshm")
            )
	    {
		gui->def_use_shm = False;
	    }
            /* Auto sync shared memory puts. */
            else if(!strcmp(argv[i], "--sync_xshm") ||
                    !strcmp(argv[i], "-sync_xshm") ||
                    !strcmp(argv[i], "--sync_shm") ||
                    !strcmp(argv[i], "-sync_shm")
            )
            {
                gui->def_sync_shm = True;
            }
	}


        /* ********************************************************* */
	/* Open connection to display. */
	status = -1;
	while(1)
	{
	    /* First try given input host address to connect to. */
	    gui->display = XOpenDisplay(host);
            if(gui->display != NULL)
            {
		status = 0;	/* Success. */
		break;
	    }

	    /* Check if host address was NULL (unspecified). */
	    if(host == NULL)
	    {
		/* Try enviroment valriable. */
	        gui->display = XOpenDisplay(getenv("DISPLAY"));
	        if(gui->display != NULL)
	        {
                    status = 0;	/* Success. */
                    break;
                }

		/* All else fails, try explicitly setting localhost. */
		gui->display = XOpenDisplay("127.0.0.1:0.0");
                if(gui->display != NULL)
                {
                    status = 0;	/* Success. */
                    break;
                }
	    }

	    /* Failed. */
	    break;
	}
	/* Connect failed? */
	if(status)
	{
            /* Print reason for failed connect. */
            fprintf(stderr, "Cannot connect to X server: ");
            if(getenv("DISPLAY") == NULL)
                fprintf(stderr, "`DISPLAY' environment variable not set.");
            else
                fprintf(stderr, "%s", XDisplayName(NULL));
            fprintf(stderr, "\n");

            /* Print verbose help. */
            fprintf(stderr,
"\nCheck to make sure that your X server is running and that your\n\
enviroment variable `DISPLAY' is set properly.\n"
            );

            return(1);
        }


        /* ********************************************************* */
        /* Get default values (as needed). */
        gui->scr_num = DefaultScreen(gui->display);
        gui->scr_ptr = DefaultScreenOfDisplay(gui->display);
        gui->visual = DefaultVisualOfScreen(gui->scr_ptr);
	gui->visual_id = XVisualIDFromVisual(gui->visual);

        gui->depth = ((depth == 0) ?
	    DefaultDepthOfScreen(gui->scr_ptr) : depth
	);
	switch(gui->depth)
	{
	  case 15:
	    gui->actual_depth = 16;
	    break;
	  case 24:
            gui->actual_depth = 32;
	    break;
	  default:
	    gui->actual_depth = gui->depth;
	    break;
	}
	gui->z_bytes = (gui->actual_depth >> 3);
	if(gui->z_bytes < 1)
	{
	    fprintf(stderr,
 "OSWGUIConnect(): Warning: Bytes per pixel is %i, fixing to 1.\n",
		gui->z_bytes
	    );
	    gui->z_bytes = 1;
	}

        gui->root_win = DefaultRootWindow(gui->display);
        if(gui->root_win == 0)
            fprintf(stderr,
                "Cannot obtain root window ID.\n"
            );

/*
vinfo.depth = depth;
vinfo.class = TrueColor;
vptr = GETVISUALBYCRITERIA(
	VisualDepthMask | VisualClassMask,
	vinfo
);
if(vptr != NULL)
{
    vid = XVisualIDFromVisual(vptr);
    printf("Visual ID: 0x%.8x\n", vid);

    printf("0x%.8x 0x%.8x\n", vptr, visual);
}
*/

        gui->display_width = DisplayWidth(
	    gui->display,
	    gui->scr_num
	);
        gui->display_height = DisplayHeight(
	    gui->display,
	    gui->scr_num
	);

	gui->black_pix = BlackPixelOfScreen(gui->scr_ptr);
	gui->white_pix = WhitePixelOfScreen(gui->scr_ptr);

        gui->gc = DefaultGC(gui->display, gui->scr_num);

	/* Check if visual class is PseudoColor. */
	// Dan S: The defined(__cplusplus) || defined(c_plusplus) causes redefinition
	// of "class" to be "c_class" (for C++ portability). There is no functional difference.
	if(gui->visual->c_class == PseudoColor)
	{
	    /* Visual class is PseudoColor, we'll need to create
	     * a private colormap because the we need to handle
             * image_t (XImages) in TrueColor.
	     */
            switch(gui->depth)
            {
              case 8:
                gui->colormap = XCreateColormap(
                    gui->display,
                    gui->root_win,
                    gui->visual,
                    AllocNone
		);
                xc.flags = DoRed | DoGreen | DoBlue;
                for(i = 0; i < 256; i++)
                {
                    xc.blue = ((i & 0x03) << (5 + 8));
                    xc.green = ((i & 0x1C) << (3 + 8));
                    xc.red = ((i & 0xE0) << (0 + 8));
                    XAllocColor(gui->display, gui->colormap, &xc);
		}
		XInstallColormap(gui->display, gui->colormap);
                gui->we_created_colormap = True;
 		break;

	      /* Don't know how to handle other depths of PseudoColor,
	       * so just use default color map even tho it may mess up
	       * colors.
	       */
	      default:
		gui->colormap = DefaultColormap(gui->display, gui->scr_num);
                gui->we_created_colormap = False;
		break;
	    }
	}
	else
	{
	    gui->colormap = DefaultColormap(gui->display, gui->scr_num);
	    gui->we_created_colormap = False;
	}

	gui->std_cursor = OSWLoadBasicCursor(XC_left_ptr);
        if(gui->std_cursor == 0)
            fprintf(stderr,
                "Cannot obtain standard cursor.\n"
            );


	/* Change visual? */
	if(vid != 0x00)
	{
	    vis_ptr = OSWGetVisualByID(vid);
	    if(vis_ptr == NULL)
	    {
		fprintf(stderr,
		    "Cannot obtain visual 0x%.2x.\n",
		    (int)vid
		);
	    }
	    else
	    {
	        gui->visual = vis_ptr;
	    }
	}

	gui->red_mask = gui->visual->red_mask;
        gui->green_mask = gui->visual->green_mask;
        gui->blue_mask = gui->visual->blue_mask;

#ifdef USE_XSHM
	/* Shared image put completion event code. */
	gui->shm_completion_event_code = XShmGetEventBase(gui->display) +
	    ShmCompletion;
#endif	/* USE_XSHM */


        /* ********************************************************* */
        /* Default foreground and background pixels. */
	StringParseStdColor(
	    (bg_pix_str == NULL) ? "#000000" : bg_pix_str,
	    &r, &g, &b
	);
	OSWLoadPixelRGB(&gui->def_bg_pix, r, g, b);

        StringParseStdColor(
            (fg_pix_str == NULL) ? "#ffffff" : fg_pix_str,
            &r, &g, &b
        );
        OSWLoadPixelRGB(&gui->def_fg_pix, r, g, b);



        /* ********************************************************* */
	/* Load standard and default fonts. */

	/* Standard font. */
	status = OSWLoadFont(&gui->std_font, "7x14");
	if(status)
	    fprintf(stderr,
		"%s: Unable to load font.\n",
		"7x14"
	    );

	/* Standard bold font. */
	status = OSWLoadFont(&gui->bold_font, "7x14");
        if(status)
            fprintf(stderr,
                "%s: Unable to load font.\n",
                "7x14"
            );

	/* Default font (the one specified from command line -fn). */
	status = OSWLoadFont(
	    &gui->def_font,
	    (def_font_name == NULL) ? "7x14" : def_font_name
	);
        if(status)
            fprintf(stderr,
                "%s: Unable to load font.\n",
                (def_font_name == NULL) ? "7x14" : def_font_name
            );
	/* Load something incase of error. */
        if(gui->def_font == NULL)
            OSWLoadFont(&gui->def_font, "7x14");


        /* ********************************************************* */
	/* Set up default foreground values. */
	OSWSetFgPix(gui->white_pix);
        OSWSetFont(gui->std_font);


        /* ********************************************************* */
	/* Load key codes from OS's keysyms. */
	OSWLoadKeyCodes();


	/* ********************************************************* */
	/* Load property atoms (unique to X Window Systems). */

	/* Check if MWM is supported. */
        mwm_hints_atom = XInternAtom(
	    gui->display,
	    _XA_MWM_HINTS,
	    False
	);
        if(mwm_hints_atom == None)
	{
	    fprintf(stderr,
 "OSWGUIConnect(): Warning: Window manager does not support %s.\n",
		_XA_MWM_HINTS
	    );
        }



        /* Motif protocol$. */

        osw_atom._motif_wm_all_clients = XInternAtom(
            gui->display,
            "_MOTIF_WM_ALL_CLIENTS",
            False
        );

        osw_atom._motif_wm_hints = XInternAtom(
            gui->display,
            "_MOTIF_WM_HINTS",
            False
        );

        osw_atom._motif_wm_info = XInternAtom(
            gui->display,
            "_MOTIF_WM_INFO",
            False
        );

        osw_atom._motif_wm_menu = XInternAtom(
            gui->display,
            "_MOTIF_WM_MENU",
            False
        );

        osw_atom._motif_wm_messages = XInternAtom(
            gui->display,
            "_MOTIF_WM_MESSAGES",
            False
        );

        osw_atom._motif_wm_offset = XInternAtom(
            gui->display,
            "_MOTIF_WM_OFFSET",
            False
        );

        osw_atom._motif_wm_query = XInternAtom(
            gui->display,
            "_MOTIF_WM_QUERY",
            False
        );


        /* Standard management protocols. */
        osw_atom.wm_delete_window = XInternAtom(
            gui->display,
            "WM_DELETE_WINDOW",
            False
        );

        osw_atom.wm_protocols = XInternAtom(
            gui->display,
            "WM_PROTOCOLS",
            False
        );  

        osw_atom.wm_save_yourself = XInternAtom(
            gui->display,
            "WM_SAVE_YOURSELF",
            False
        );
        
        osw_atom.wm_state = XInternAtom(
            gui->display,
            "WM_STATE",
            False
        );

        osw_atom.wm_take_focus = XInternAtom(   
            gui->display,
            "WM_TAKE_FOCUS",
            False
        );


        /* Root stuff. */
        osw_atom._xrootpmap_id = XInternAtom(
            gui->display,
            "_XROOTPMAP_ID",   
            False
        );

        osw_atom._xrootcolor_pixel = XInternAtom(
            gui->display,
            "_XROOTCOLOR_PIXEL",
            False
        );

        /* Enlightenment WM Protocols. */
        osw_atom.enl_msg = XInternAtom( 
            gui->display,
            "_XROOTCOLOR_PIXEL",
            False
        );

        osw_atom.enlightenment_desktop = XInternAtom(
            gui->display,
            "ENLIGHTENMENT_DESKTOP",
            False
        );

        osw_atom.enlightenment_comms = XInternAtom(
            gui->display,
            "ENLIGHTENMENT_COMMS",
            False
        );

        /* ******************************************************* */

	/* Set up graphics bit packing lookup table. */
	OSWGenerateColorLookupTable(gui->depth);


	/* ******************************************************* */

	/* Set to standard error handler. */
	XSetErrorHandler(OSWGUIErrorHandler);

	/* Reset error code. */
	static_osw_x::gui_error_code = 0;


	return(0);
}



/*
 *	Disconnect from GUI.
 */
void OSWGUIDisconnect()
{
	int i, n;
	pixel_t *pixel_array;
        osw_gui_struct *gui;


	if(IDC())
	{
	    gui = &osw_gui[0];

	    /* Purge all events. */
	    OSWPurgeAllEvents();


	    /* Free resources. */
	    OSWDestroyPixel(&gui->def_bg_pix);
	    OSWDestroyPixel(&gui->def_fg_pix);
	    OSWUnloadFont(&gui->def_font);

	    /* Free color map as needed. */
	    if(gui->we_created_colormap)
	    {
		switch(gui->depth)
		{
		  case 8:
		    n = 256;
		    pixel_array = (pixel_t *)malloc(n * sizeof(pixel_t));
		    if(pixel_array != NULL)
		    {
		        for(i = 0; i < n; i++)
			    pixel_array[i] = (pixel_t)i;

			XFreeColors(
                            gui->display,
                            gui->colormap,
                            pixel_array,	/* Pixel array. */
                            n,			/* Number of pixels. */
			    0			/* Planes */
		        );
		        free(pixel_array);
		    }
		    break;
		}

		XUninstallColormap(gui->display, gui->colormap);
		XFreeColormap(gui->display, gui->colormap);
		gui->we_created_colormap = False;
	    }

	    OSWUnloadFont(&gui->std_font);
	    OSWUnloadFont(&gui->bold_font);


	    /* Close connection to server. */
	    XSetCloseDownMode(gui->display, DestroyAll);
            XCloseDisplay(gui->display);


	    free(gui->vendor_id);
	    gui->vendor_id = NULL;

	    gui->def_use_shm = False;
            gui->def_geometry_set = False;
            gui->def_toplevel_width = 0;
            gui->def_toplevel_height = 0;
            gui->def_toplevel_x = 0;
            gui->def_toplevel_y = 0;

            gui->red_mask = 0;
            gui->green_mask = 0;
            gui->blue_mask = 0;

            gui->display_width = 0;
            gui->display_height = 0;
            gui->black_pix = 0;
            gui->white_pix = 0;
            gui->std_cursor = 0;
            gui->root_win = 0;
            gui->current_font = NULL;
            gui->current_fg_pix = 0;
            gui->current_bg_pix = 0;
            gui->current_alpha_pix = 0;

	    gui->alt_key_state = False;
            gui->ctrl_key_state = False;
            gui->shift_key_state = False;
            gui->caps_lock_on = False;
            gui->num_lock_on = False;
            gui->scroll_lock_on = False;

            gui->display = NULL;
            gui->scr_num = 0;
            gui->scr_ptr = NULL;
            gui->visual = NULL;
            gui->visual_id = 0;
            gui->depth = 0;
	    gui->actual_depth = 0;
	    gui->z_bytes = 0;

	    memset(&gui->colormap, 0x00, sizeof(colormap_t));
	    memset(&gui->gc, 0x00, sizeof(gc_t));
	}


	return;
}



/*
 *	Loads the OSW keycodes from the keysyms.
 */
int OSWLoadKeyCodes()
{
	display_t *d;


	if(!IDC())
	    return(-1);


	d = osw_gui[0].display;


	/*	Note: Rows reffer to the row count, top to bottom on
	 *	a US Qwerty keyboard.  Other keyboards may vary
	 *	however row is just use for programer comments.
	 */

	/* Row 1. */
	osw_keycode.esc = XKeysymToKeycode(d, XK_Escape);
        osw_keycode.f1 = XKeysymToKeycode(d, XK_F1);
        osw_keycode.f2 = XKeysymToKeycode(d, XK_F2);
        osw_keycode.f3 = XKeysymToKeycode(d, XK_F3);
        osw_keycode.f4 = XKeysymToKeycode(d, XK_F4);
        osw_keycode.f5 = XKeysymToKeycode(d, XK_F5);
        osw_keycode.f6 = XKeysymToKeycode(d, XK_F6);
        osw_keycode.f7 = XKeysymToKeycode(d, XK_F7);
        osw_keycode.f8 = XKeysymToKeycode(d, XK_F8);
        osw_keycode.f9 = XKeysymToKeycode(d, XK_F9);
        osw_keycode.f10 = XKeysymToKeycode(d, XK_F10);
        osw_keycode.f11 = XKeysymToKeycode(d, XK_F11);
        osw_keycode.f12 = XKeysymToKeycode(d, XK_F12);

        osw_keycode.f13 = XKeysymToKeycode(d, XK_F13);
        osw_keycode.f14 = XKeysymToKeycode(d, XK_F14);
        osw_keycode.f15 = XKeysymToKeycode(d, XK_F15);
        osw_keycode.f16 = XKeysymToKeycode(d, XK_F16);
        osw_keycode.f17 = XKeysymToKeycode(d, XK_F17);
        osw_keycode.f18 = XKeysymToKeycode(d, XK_F18);
        osw_keycode.f19 = XKeysymToKeycode(d, XK_F19);
        osw_keycode.f20 = XKeysymToKeycode(d, XK_F20);
        osw_keycode.f21 = XKeysymToKeycode(d, XK_F21);
        osw_keycode.f22 = XKeysymToKeycode(d, XK_F22);
        osw_keycode.f23 = XKeysymToKeycode(d, XK_F23);
        osw_keycode.f24 = XKeysymToKeycode(d, XK_F24);

        osw_keycode.f25 = XKeysymToKeycode(d, XK_F25);
        osw_keycode.f26 = XKeysymToKeycode(d, XK_F26);
        osw_keycode.f27 = XKeysymToKeycode(d, XK_F27);
        osw_keycode.f28 = XKeysymToKeycode(d, XK_F28);
        osw_keycode.f29 = XKeysymToKeycode(d, XK_F29);
        osw_keycode.f30 = XKeysymToKeycode(d, XK_F30);
        osw_keycode.f31 = XKeysymToKeycode(d, XK_F31);
        osw_keycode.f32 = XKeysymToKeycode(d, XK_F32);
        osw_keycode.f33 = XKeysymToKeycode(d, XK_F33);
        osw_keycode.f34 = XKeysymToKeycode(d, XK_F34);
        osw_keycode.f35 = XKeysymToKeycode(d, XK_F35);

	/* Row 2. */
/* XK_quotedbl */
        osw_keycode.tilde = XKeysymToKeycode(d, XK_asciitilde);
        osw_keycode.num_1 = XKeysymToKeycode(d, XK_1);
        osw_keycode.num_2 = XKeysymToKeycode(d, XK_2);
        osw_keycode.num_3 = XKeysymToKeycode(d, XK_3);
        osw_keycode.num_4 = XKeysymToKeycode(d, XK_4);
        osw_keycode.num_5 = XKeysymToKeycode(d, XK_5);
        osw_keycode.num_6 = XKeysymToKeycode(d, XK_6);
        osw_keycode.num_7 = XKeysymToKeycode(d, XK_7);
        osw_keycode.num_8 = XKeysymToKeycode(d, XK_8);
        osw_keycode.num_9 = XKeysymToKeycode(d, XK_9);
        osw_keycode.num_0 = XKeysymToKeycode(d, XK_0);
	osw_keycode.colon = XKeysymToKeycode(d, XK_colon);
	osw_keycode.lessthan = XKeysymToKeycode(d, XK_less);
	osw_keycode.greaterthan = XKeysymToKeycode(d, XK_greater);
	osw_keycode.questionmark = XKeysymToKeycode(d, XK_question);
	osw_keycode.at = XKeysymToKeycode(d, XK_at);
	osw_keycode.underscore = XKeysymToKeycode(d, XK_underscore);
        osw_keycode.braketleft = XKeysymToKeycode(d, XK_bracketleft);
        osw_keycode.braketright = XKeysymToKeycode(d, XK_bracketright);
        osw_keycode.quoteleft = XKeysymToKeycode(d, XK_quoteleft);
        osw_keycode.exclamation = XKeysymToKeycode(d, XK_exclam);
        osw_keycode.numbersign = XKeysymToKeycode(d, XK_numbersign);
        osw_keycode.dollarsign = XKeysymToKeycode(d, XK_dollar);
        osw_keycode.percent = XKeysymToKeycode(d, XK_percent);
        osw_keycode.ampersand = XKeysymToKeycode(d, XK_ampersand);
        osw_keycode.apostrophe = XKeysymToKeycode(d, XK_apostrophe);
	osw_keycode.parenleft = XKeysymToKeycode(d, XK_parenleft);
        osw_keycode.parenright = XKeysymToKeycode(d, XK_parenright);
        osw_keycode.asterisk = XKeysymToKeycode(d, XK_asterisk);
        osw_keycode.plus = XKeysymToKeycode(d, XK_plus);
        osw_keycode.minus = XKeysymToKeycode(d, XK_minus);
        osw_keycode.equal = XKeysymToKeycode(d, XK_equal);
        osw_keycode.brace_left = XKeysymToKeycode(d, XK_braceleft);
        osw_keycode.brace_right = XKeysymToKeycode(d, XK_braceright);
        osw_keycode.bar = XKeysymToKeycode(d, XK_bar);
        osw_keycode.backslash = XKeysymToKeycode(d, XK_backslash);
        osw_keycode.backspace = XKeysymToKeycode(d, XK_BackSpace);

	/* Row 3. */
        osw_keycode.tab = XKeysymToKeycode(d, XK_Tab);
        osw_keycode.alpha_q = XKeysymToKeycode(d, XK_q);
        osw_keycode.alpha_w = XKeysymToKeycode(d, XK_w);
        osw_keycode.alpha_e = XKeysymToKeycode(d, XK_e);
        osw_keycode.alpha_r = XKeysymToKeycode(d, XK_r);
        osw_keycode.alpha_t = XKeysymToKeycode(d, XK_t);
        osw_keycode.alpha_y = XKeysymToKeycode(d, XK_y);
        osw_keycode.alpha_u = XKeysymToKeycode(d, XK_u);
        osw_keycode.alpha_i = XKeysymToKeycode(d, XK_i);
        osw_keycode.alpha_o = XKeysymToKeycode(d, XK_o);
        osw_keycode.alpha_p = XKeysymToKeycode(d, XK_p);
        osw_keycode.alpha_Q = XKeysymToKeycode(d, XK_Q);
        osw_keycode.alpha_W = XKeysymToKeycode(d, XK_W);
        osw_keycode.alpha_E = XKeysymToKeycode(d, XK_E);
        osw_keycode.alpha_R = XKeysymToKeycode(d, XK_R);
        osw_keycode.alpha_T = XKeysymToKeycode(d, XK_T);  
        osw_keycode.alpha_Y = XKeysymToKeycode(d, XK_Y);
        osw_keycode.alpha_U = XKeysymToKeycode(d, XK_U);
        osw_keycode.alpha_I = XKeysymToKeycode(d, XK_I);
        osw_keycode.alpha_O = XKeysymToKeycode(d, XK_O);
        osw_keycode.alpha_P = XKeysymToKeycode(d, XK_P);
        osw_keycode.brace_left = XKeysymToKeycode(d, XK_braceleft);
        osw_keycode.brace_right = XKeysymToKeycode(d, XK_braceright);
        osw_keycode.enter = XKeysymToKeycode(d, XK_Return);

	/* Row 4. */
        osw_keycode.caps_lock = XKeysymToKeycode(d, XK_Caps_Lock);
        osw_keycode.alpha_a = XKeysymToKeycode(d, XK_a);
        osw_keycode.alpha_s = XKeysymToKeycode(d, XK_s);
        osw_keycode.alpha_d = XKeysymToKeycode(d, XK_d);
        osw_keycode.alpha_f = XKeysymToKeycode(d, XK_f);
        osw_keycode.alpha_g = XKeysymToKeycode(d, XK_g);
        osw_keycode.alpha_h = XKeysymToKeycode(d, XK_h);
        osw_keycode.alpha_j = XKeysymToKeycode(d, XK_j);
        osw_keycode.alpha_k = XKeysymToKeycode(d, XK_k);
        osw_keycode.alpha_l = XKeysymToKeycode(d, XK_l);
        osw_keycode.alpha_A = XKeysymToKeycode(d, XK_A);
        osw_keycode.alpha_S = XKeysymToKeycode(d, XK_S);
        osw_keycode.alpha_D = XKeysymToKeycode(d, XK_D);
        osw_keycode.alpha_F = XKeysymToKeycode(d, XK_F);
        osw_keycode.alpha_G = XKeysymToKeycode(d, XK_G);
        osw_keycode.alpha_H = XKeysymToKeycode(d, XK_H);
        osw_keycode.alpha_J = XKeysymToKeycode(d, XK_J);
        osw_keycode.alpha_K = XKeysymToKeycode(d, XK_K);
        osw_keycode.alpha_L = XKeysymToKeycode(d, XK_L);
        osw_keycode.semicolon = XKeysymToKeycode(d, XK_semicolon);
        osw_keycode.quote = XKeysymToKeycode(d, XK_quoteright);
/* XK_quoteleft */

	/* Row 5. */
        osw_keycode.shift_left = XKeysymToKeycode(d, XK_Shift_L);
        osw_keycode.alpha_z = XKeysymToKeycode(d, XK_z);
        osw_keycode.alpha_x = XKeysymToKeycode(d, XK_x);
        osw_keycode.alpha_c = XKeysymToKeycode(d, XK_c);
        osw_keycode.alpha_v = XKeysymToKeycode(d, XK_v);
        osw_keycode.alpha_b = XKeysymToKeycode(d, XK_b);
        osw_keycode.alpha_n = XKeysymToKeycode(d, XK_n);
        osw_keycode.alpha_m = XKeysymToKeycode(d, XK_m);
        osw_keycode.alpha_Z = XKeysymToKeycode(d, XK_Z);
        osw_keycode.alpha_X = XKeysymToKeycode(d, XK_X);
        osw_keycode.alpha_C = XKeysymToKeycode(d, XK_C);
        osw_keycode.alpha_V = XKeysymToKeycode(d, XK_V);
        osw_keycode.alpha_B = XKeysymToKeycode(d, XK_B);
        osw_keycode.alpha_N = XKeysymToKeycode(d, XK_N);
        osw_keycode.alpha_M = XKeysymToKeycode(d, XK_M);
        osw_keycode.comma = XKeysymToKeycode(d, XK_comma);
        osw_keycode.period = XKeysymToKeycode(d, XK_period);
        osw_keycode.slash = XKeysymToKeycode(d, XK_slash);
        osw_keycode.shift_right = XKeysymToKeycode(d, XK_Shift_R);

	/* Row 6. */
        osw_keycode.ctrl_left = XKeysymToKeycode(d, XK_Control_L);
        osw_keycode.alt_left = XKeysymToKeycode(d, XK_Alt_L);
        osw_keycode.win95_start = 0;
        osw_keycode.space = XKeysymToKeycode(d, XK_space);
        osw_keycode.alt_right = XKeysymToKeycode(d, XK_Alt_R);
        osw_keycode.ctrl_right = XKeysymToKeycode(d, XK_Control_R);


	/* Secondary section. */
        osw_keycode.print_screen = XKeysymToKeycode(d, XK_Sys_Req);
/* XK_3270_PrintScreen */
        osw_keycode.scroll_lock = XKeysymToKeycode(d, XK_Scroll_Lock);
        osw_keycode.pause = XKeysymToKeycode(d, XK_Pause);
        osw_keycode.insert = XKeysymToKeycode(d, XK_Insert);
        osw_keycode.home = XKeysymToKeycode(d, XK_Home);
        osw_keycode.page_up = XKeysymToKeycode(d, XK_Page_Up);
        osw_keycode.ddelete = XKeysymToKeycode(d, XK_Delete);
        osw_keycode.end = XKeysymToKeycode(d, XK_End);
        osw_keycode.page_down = XKeysymToKeycode(d, XK_Page_Down);
 
        osw_keycode.cursor_up = XKeysymToKeycode(d, XK_Up);
        osw_keycode.cursor_right = XKeysymToKeycode(d, XK_Right);
        osw_keycode.cursor_down = XKeysymToKeycode(d, XK_Down);
        osw_keycode.cursor_left = XKeysymToKeycode(d, XK_Left);

	/* Number pad. */
        osw_keycode.num_lock = XKeysymToKeycode(d, XK_Num_Lock);
        osw_keycode.np_slash = XKeysymToKeycode(d, XK_KP_Divide);
        osw_keycode.np_asterisk = XKeysymToKeycode(d, XK_KP_Multiply);
        osw_keycode.np_minus = XKeysymToKeycode(d, XK_KP_Subtract);
        osw_keycode.np_add = XKeysymToKeycode(d, XK_KP_Add);
        osw_keycode.np_enter = XKeysymToKeycode(d, XK_KP_Enter);
        osw_keycode.np_1 = XKeysymToKeycode(d, XK_KP_1);
        osw_keycode.np_2 = XKeysymToKeycode(d, XK_KP_2);
        osw_keycode.np_3 = XKeysymToKeycode(d, XK_KP_3);
        osw_keycode.np_4 = XKeysymToKeycode(d, XK_KP_4);
        osw_keycode.np_5 = XKeysymToKeycode(d, XK_KP_5);
        osw_keycode.np_6 = XKeysymToKeycode(d, XK_KP_6);
        osw_keycode.np_7 = XKeysymToKeycode(d, XK_KP_7);
        osw_keycode.np_8 = XKeysymToKeycode(d, XK_KP_8);
        osw_keycode.np_9 = XKeysymToKeycode(d, XK_KP_9);
        osw_keycode.np_0 = XKeysymToKeycode(d, XK_KP_0);
        osw_keycode.np_period = XKeysymToKeycode(d, XK_KP_Decimal);



	return(0);
}


/*
 *	This function is called internally to manage an event.
 *	Each event fetching function needs to call this function
 *	before returning the event to the calling function.
 */
void OSWManageEvent(event_t *event)
{
	osw_gui_struct *gui;
	keycode_t keycode;


	if(event == NULL)
	    return;

	gui = &osw_gui[0];


	switch(event->type)
	{
	  case KeyPress:
	    keycode = event->xkey.keycode;

	    /* Check modifier key? */
	    if((keycode == osw_keycode.alt_left) ||
               (keycode == osw_keycode.alt_right)
	    )
		gui->alt_key_state = True;
	    else if((keycode == osw_keycode.ctrl_left) ||
                    (keycode == osw_keycode.ctrl_right)
            )
		gui->ctrl_key_state = True;
            else if((keycode == osw_keycode.shift_left) ||
                    (keycode == osw_keycode.shift_right)
            )
                gui->shift_key_state = True;

            else if(keycode == osw_keycode.caps_lock)
		gui->caps_lock_on = !gui->caps_lock_on;
            else if(keycode == osw_keycode.num_lock)
                gui->num_lock_on = !gui->num_lock_on;
            else if(keycode == osw_keycode.scroll_lock)
                gui->scroll_lock_on = !gui->scroll_lock_on;

	    break;


          case KeyRelease:
            keycode = event->xkey.keycode;

	    /* Check modifier key? */
            if((keycode == osw_keycode.alt_left) ||
               (keycode == osw_keycode.alt_right)
            )
                gui->alt_key_state = False;
            else if((keycode == osw_keycode.ctrl_left) ||
                    (keycode == osw_keycode.ctrl_right)
            )
                gui->ctrl_key_state = False;
            else if((keycode == osw_keycode.shift_left) ||
                    (keycode == osw_keycode.shift_right)
            )
                gui->shift_key_state = False;

	    break;
	}


	return;
}


/*
 *	Returns an ASCII char from the given keycode or '\0' if
 *	no match could be found.
 */
char OSWGetASCIIFromKeyCode(
	key_event_t *ke,
	bool_t shift,
	bool_t alt,
	bool_t ctrl
)
{
	int i;
        const int total = sizeof(static_osw_x::keytable) / sizeof(char *);
        KeySym keysym;
	osw_gui_struct *gui;
	char *strptr;


	gui = &osw_gui[0];
	if((gui->display == NULL) ||
	   (ke == NULL)
	)
	    return('\0');

	/* Translate key event. */
	keysym = XLookupKeysym(
            ke,
            (int)((gui->shift_key_state) ? 1 : 0)
        );
        if(keysym == NoSymbol)
	    return('\0');

	/* Get keysym name. */
	strptr = XKeysymToString(keysym);
	if(strptr == NULL)
	    return('\0');

        for(i = 0; i < total; i += 2)
        {
            if(!strcmp(static_osw_x::keytable[i], strptr))
                return(
                    (int)(static_osw_x::keytable[i + 1])
                );
        }

        return('\0');
}



/*
 *	Returns a statically allocated string containing the
 *	formal name for the given keycode looked up in the keysyms
 *	listing from the hardware.
 *
 *	The return string will contain a (usually) lower case name
 *	of the keycode or "#123" if the key is undefined where 123
 *	is the keycode.
 *
 *	This function never returns NULL.
 */
char *OSWGetKeyCodeName(keycode_t keycode)
{
	keysym_t keysym;
	static char rtn_str[128];


	/* Return none if keycode is 0. */
	if(keycode == 0)
	{
	    return("#0");
	}

	keysym = XKeycodeToKeysym(osw_gui[0].display, keycode, 0);
	if(keysym == NoSymbol)
	{
	    sprintf(rtn_str, "#%i", keycode);
	    return(rtn_str);
	}

	/* ******************************************************* */
	/* Misc. */
	if(keysym == XK_BackSpace)
	{
	    return("backspace");
	}
        else if(keysym == XK_Tab)
        {
            return("tab");
        }
        else if(keysym == XK_Linefeed)            
        {
            return("linefeed");
        }   
        else if(keysym == XK_Clear)
        {
            return("clear");
        }   
        else if(keysym == XK_Return)
        {
            return("enter");
        }   
        else if(keysym == XK_Pause)
        {
            return("pause");
        }   
        else if(keysym == XK_Scroll_Lock)            
        {
            return("scroll lock");
        }   
        else if(keysym == XK_Sys_Req)            
        {
            return("system request");
        }   
        else if(keysym == XK_Escape)
        {
            return("escape");
        }   
        else if(keysym == XK_Delete)            
        {
            return("delete");
        }   
        else if(keysym == XK_Home)            
        {
            return("home");
        }   
        else if(keysym == XK_End)            
        {
            return("end");
        }   
        else if(keysym == XK_Page_Up)            
        {
            return("page up");
        }   
        else if(keysym == XK_Page_Down)
        {
            return("page down");
        }   
        else if(keysym == XK_Up)
        {
            return("up");
        }   
        else if(keysym == XK_Down)            
        {
            return("down");
        }   
        else if(keysym == XK_Left)            
        {
            return("left");
        }   
        else if(keysym == XK_Right)
        {
            return("right");
        }   
        else if(keysym == XK_Insert)            
        {
            return("insert");
        }   
        else if(keysym == XK_Break)            
        {
            return("break");
        }   
        else if(keysym == XK_Num_Lock)            
        {
            return("num lock");
        }   

	/* ****************************************************** */
	/* Number pad keys. */
        else if(keysym == XK_KP_Space)
        {
            return("np space");
        }
        else if(keysym == XK_KP_Tab)
        {
            return("np tab");
        }
        else if(keysym == XK_KP_Enter)            
        {
            return("np enter");
        }   
        else if(keysym == XK_KP_Home)
        {
            return("np home");
        }   
        else if(keysym == XK_KP_End)            
        {
            return("np end");
        }   
        else if(keysym == XK_KP_Page_Up)            
        {
            return("np page up");
        }   
        else if(keysym == XK_KP_Page_Down)
        {
            return("np page down");
        }   
        else if(keysym == XK_KP_Up)       
        {
            return("np up");
        }   
        else if(keysym == XK_KP_Down)            
        {
            return("np down");
        }   
        else if(keysym == XK_KP_Left)            
        {
            return("np left");
        }   
        else if(keysym == XK_KP_Right)
        {
            return("np right");
        }
        else if(keysym == XK_KP_Insert)
        {
            return("np insert");
        }   
        else if(keysym == XK_KP_Delete)
        {
            return("np delete");
        }   
        else if(keysym == XK_KP_Equal)          
        {
            return("np equal");
        }   
        else if(keysym == XK_KP_Multiply)
        {
            return("np multiply");
        }   
        else if(keysym == XK_KP_Divide)
        {
            return("np divide");
        }
        else if(keysym == XK_KP_Add)
        {
            return("np add");
        }
        else if(keysym == XK_KP_Subtract)
        {
            return("np subtract");
        }
        else if(keysym == XK_KP_Decimal)
        {
            return("np decimal");
        }
        else if(keysym == XK_KP_0)
        {
            return("np 0");
        }
        else if(keysym == XK_KP_1)
        {
            return("np 1");
        }
        else if(keysym == XK_KP_2)
        {
            return("np 2");
        }
        else if(keysym == XK_KP_3)
        {
            return("np 3");
        }
        else if(keysym == XK_KP_4)
        {
            return("np 4");
        }
        else if(keysym == XK_KP_5)
        {
            return("np 5");
        }
        else if(keysym == XK_KP_6)
        {
            return("np 6");
        }
        else if(keysym == XK_KP_7)
        {
            return("np 7");
        }
        else if(keysym == XK_KP_8)
        {
            return("np 8");
        }
        else if(keysym == XK_KP_9)
        {
            return("np 9");
        }

	/* Function keys. */
        else if(keysym == XK_F1)
        {
            return("f1");
        }
        else if(keysym == XK_F2)
        {
            return("f2");
        }
        else if(keysym == XK_F3)
        {
            return("f3");
        }
        else if(keysym == XK_F4)
        {
            return("f4");
        }
        else if(keysym == XK_F5)
        {
            return("f5");
        }
        else if(keysym == XK_F6)
        {
            return("f6");
        }
        else if(keysym == XK_F7)
        {
            return("f7");
        }
        else if(keysym == XK_F8)
        {
            return("f8");
        }
        else if(keysym == XK_F9)
        {
            return("f9");
        }
        else if(keysym == XK_F10)
        {
            return("f10");
        }
        else if(keysym == XK_F11)
        {
            return("f11");
        }
        else if(keysym == XK_F12)
        {
            return("f12");
        }
        else if(keysym == XK_F13)
        {
            return("f13");
        }
        else if(keysym == XK_F14)
        {
            return("f14");
        }
        else if(keysym == XK_F15)
        {
            return("f15");
        }
        else if(keysym == XK_F16)
        {
            return("f16");
        }
        else if(keysym == XK_F17)
        {
            return("f17");
        }
        else if(keysym == XK_F18)
        {
            return("f18");
        }
        else if(keysym == XK_F19)
        {
            return("f19");
        }
        else if(keysym == XK_F20)
        {
            return("f20");
        }
        else if(keysym == XK_F21)
        {
            return("f21");
        }
        else if(keysym == XK_F22)
        {
            return("f22");
        }
        else if(keysym == XK_F23)
        {
            return("f23");
        }
        else if(keysym == XK_F24)
        {
            return("f24");
        }
        else if(keysym == XK_F25)
        {
            return("f25");
        }
        else if(keysym == XK_F26)
        {
            return("f26");
        }
        else if(keysym == XK_F27)
        {
            return("f27");
        }
        else if(keysym == XK_F28)
        {
            return("f28");
        }
        else if(keysym == XK_F29)
        {
            return("f29");
        }
        else if(keysym == XK_F30)
        {
            return("f30");
        }
        else if(keysym == XK_F31)
        {
            return("f31");
        }   
        else if(keysym == XK_F32)
        {
            return("f32");
        }
        else if(keysym == XK_F33)
        {
            return("f33");
        }
        else if(keysym == XK_F34)
        {
            return("f34");
        }
        else if(keysym == XK_F35)
        {
            return("f35");
        }

	/* Modifiers */
        else if(keysym == XK_Shift_L)
        {
            return("shift left");
        }   
        else if(keysym == XK_Shift_R)
        {
            return("shift right");
        }   
        else if(keysym == XK_Control_L)
        {
            return("control left");
        }   
        else if(keysym == XK_Control_R)
        {
            return("control right");
        }   
        else if(keysym == XK_Caps_Lock)
        {
            return("caps lock");
        }   
        else if(keysym == XK_Alt_L)
        {
            return("alt left");
        }   
        else if(keysym == XK_Alt_R)
        {
            return("alt right");
        }

	/* Latin 1 keys. */
        else if(keysym == XK_space)
        {
            return("space");
        }   
        else if(keysym == XK_minus)
        {
            return("minus");
        }   
        else if(keysym == XK_equal)
        {
            return("equal");
        }   
        else if(keysym == XK_backslash)
        {
            return("back slash");
        }   
        else if(keysym == XK_quoteleft)
        {
            return("quote left");
        }   
        else if(keysym == XK_0)
        {
            return("0");
        }   
        else if(keysym == XK_1)
        {
            return("1");
        }   
        else if(keysym == XK_2)
        {
            return("2");
        }   
        else if(keysym == XK_3)
        {
            return("3");
        }   
        else if(keysym == XK_4)
        {
            return("4");
        }
        else if(keysym == XK_5)
        {
            return("5");
        }
        else if(keysym == XK_6)
        {
            return("6");
        }
        else if(keysym == XK_7)
        {
            return("7");
        }
        else if(keysym == XK_8)
        {
            return("8");
        }
        else if(keysym == XK_9)
        {
            return("9");
        }
        else if(keysym == XK_a)
        {
            return("a");
        }
        else if(keysym == XK_b)
        {
            return("b");
        }
        else if(keysym == XK_c)
        {
            return("c");
        }
        else if(keysym == XK_d)
        {
            return("d");
        }
        else if(keysym == XK_e)
        {
            return("e");
        }
        else if(keysym == XK_f)
        {
            return("f");
        }
        else if(keysym == XK_g)
        {
            return("g");
        }
        else if(keysym == XK_h)
        {
            return("h");
        }
        else if(keysym == XK_i)
        {
            return("i");
        }
        else if(keysym == XK_j)
        {
            return("j");
        }
        else if(keysym == XK_k)
        {
            return("k");
        }
        else if(keysym == XK_l)
        {
            return("l");
        }
        else if(keysym == XK_m)
        {
            return("m");
        }
        else if(keysym == XK_n)
        {
            return("n");
        }
        else if(keysym == XK_o)
        {
            return("o");
        }
        else if(keysym == XK_p)
        {
            return("p");
        }
        else if(keysym == XK_q)
        {
            return("q");
        }
        else if(keysym == XK_r)
        {
            return("r");
        }
        else if(keysym == XK_s)
        {
            return("s");
        }
        else if(keysym == XK_t)
        {
            return("t");
        }
        else if(keysym == XK_u)
        {
            return("u");
        }
        else if(keysym == XK_v)
        {
            return("v");
        }
        else if(keysym == XK_w)
        {
            return("w");
        }
        else if(keysym == XK_x)
        {
            return("x");
        }
        else if(keysym == XK_y)
        {
            return("y");
        }
        else if(keysym == XK_z)
        {
            return("z");
        }
        else if(keysym == XK_bracketleft)
        {
            return("bracket left");
        }
        else if(keysym == XK_bracketright)
        {
            return("bracket right");
        }
        else if(keysym == XK_braceleft)
        {
            return("brace left");
        }
        else if(keysym == XK_braceright)
        {
            return("brace right");
        }
        else if(keysym == XK_semicolon)
        {
            return("semicolon");
        }
        else if(keysym == XK_quoteright)
        {
            return("quote right");
        }
        else if(keysym == XK_comma)
        {
            return("comma");
        }
        else if(keysym == XK_period)
        {
            return("period");
        }
        else if(keysym == XK_slash)
        {
            return("slash");
        }


        sprintf(rtn_str, "#%i", keycode);
        return(rtn_str);
}



/*
 *	Flush events and sync with GUI.
 */
void OSWGUISync(bool_t discard)
{
	if(!IDC())
	    return;

	XSync(osw_gui[0].display, discard);

	return;
}


/*
 *	Get pointer coordinates.
 */
void OSWGetPointerCoords(
        win_t w,
        int *root_x, int *root_y,
        int *wx, int *wy
)
{
        static win_t root_return, child_return;
        static int root_x_return, root_y_return;
        static int win_x_return, win_y_return;
        static unsigned int mask_return;

        if(root_x != NULL)
            *root_x = 0;  
        if(root_y != NULL)
            *root_y = 0;
            
        if(wx != NULL)
            *wx = 0;
        if(wy != NULL)
            *wy = 0;


        if(!IDC() ||
           (w == 0)
        )
            return;

        XQueryPointer(
            osw_gui[0].display,
            w, 
            &root_return,
            &child_return,
            &root_x_return, &root_y_return,
            &win_x_return, &win_y_return,
            &mask_return
        );

        if(root_x != NULL)
            *root_x = root_x_return;
        if(root_y != NULL)
            *root_y = root_y_return;
  
        if(wx != NULL)
            *wx = win_x_return;
        if(wy != NULL)
            *wy = win_y_return;


        return;
}

/*
 *	Checks if the keycode is a modifier key.
 *
 *	Modifier keys are considered as follows:
 *
 *	alt, ctrl, and shift
 *
 *	Warning, Return may be incorrect if not connected to GUI.
 */
int OSWIsModifierKey(keycode_t keycode)
{
	if((keycode == osw_keycode.alt_left) ||
           (keycode == osw_keycode.alt_right) ||
           (keycode == osw_keycode.ctrl_left) ||
           (keycode == osw_keycode.ctrl_right) ||
           (keycode == osw_keycode.shift_left) ||
           (keycode == osw_keycode.shift_right)
	)
	    return(1);
	else
	    return(0);
}


/*
 *	Grabs the pointer in respect to grab_w and confined to window
 *	confine_w.
 *
 *	owner_events indicates whether the pointer events are to
 *	be reported as usual or reported with respect to the
 *	grab_w window if selected by the event mask.
 */
int OSWGrabPointer(
	win_t grab_w,
	bool_t events_rel_grab_w,
	eventmask_t eventmask,
	int pointer_mode,	/* GrabModeSync or GrabModeAsync. */
	int keyboard_mode,	/* GrabModeSync or GrabModeAsync. */
	win_t confine_w,	/* Can be None. */
	cursor_t cursor 	/* Can be None. */
)
{

	if(!IDC() ||
           (grab_w == 0)
	)
	    return(!GrabSuccess);

	return(
	    XGrabPointer(
		osw_gui[0].display,
		grab_w,
		!events_rel_grab_w,
		eventmask,
		pointer_mode,
		keyboard_mode,
		confine_w,
		cursor,
		CurrentTime
	    )
	);
}


/*
 *	Ungrabs the pointer.
 */
void OSWUngrabPointer()
{
	if(!IDC())
	    return;

	XUngrabPointer(osw_gui[0].display, CurrentTime);


	return;
}


/*
 *	Frees GUI allocated resource.  Memory allocated by GUI
 *	should be free'ed by a call to this general-purpose function.
 */
void OSWGUIFree(void **ptr)
{
	if(!IDC() ||
           (ptr == NULL)
	)
	    return;

	if(*ptr != NULL)
	{
	    XFree(*ptr);
	    *ptr = NULL;
	}

	return;
}



/*
 *	Fetch (get) contents of dynamic data exchange buffer.
 */
void *OSWFetchDDE(int *bytes)
{
	void *ptr;

	if(!IDC() ||
           (bytes == NULL)
	)
	    return(NULL);

	ptr = (void *)XFetchBytes(osw_gui[0].display, bytes);
	if(ptr == NULL)
	{
	    *bytes = 0;
	}

	return(ptr);
}



/*
 *	Put (store) dynamic data exchange buffer.
 */
void OSWPutDDE(void *buf, int bytes)
{
        if(!IDC() ||
           (buf == NULL) ||
           (bytes <= 0)
        )
            return;

	XStoreBytes(osw_gui[0].display, (char *)buf, bytes);

	return;
}



/*
 *      Returns a visual pointer by a match given in the criteria
 *      for a visual info. 
 */
visual_t *OSWGetVisualByCriteria(
        visual_info_mask_t vinfo_mask,
        visual_info_t criteria_vinfo
)
{               
        int num_rtn;
        visual_info_t *vinfo_rtn;
        visual_t *visual_rtn;
   

        if(!IDC())
            return(NULL);

        vinfo_rtn = XGetVisualInfo(
            osw_gui[0].display,
            vinfo_mask,
            &criteria_vinfo,
            &num_rtn
        );

        if((num_rtn <= 0) || (vinfo_rtn == NULL))
            return(NULL);


        /* Got match. */
        visual_rtn = vinfo_rtn->visual;

	/* Need to free the vinfo return. */        
        XFree(vinfo_rtn);


	/* Return the visual pointer. */
        return(visual_rtn);
}           



visual_t *OSWGetVisualByID(visual_id_t vid)
{
	int x;
	visual_info_t vinfo_ref;
	visual_info_t *vinfo_rtn;
	visual_t *vptr;


	if(!IDC())
	    return(NULL);


	vinfo_ref.visualid = vid;
	vinfo_rtn = XGetVisualInfo(
	    osw_gui[0].display,
	    VisualIDMask,
	    &vinfo_ref,
	    &x
	);
	if(vinfo_rtn == NULL)
	    return(NULL);

	/* Get visual pointer. */
	vptr = vinfo_rtn->visual;

	/* Free visual info return. */
	XFree(vinfo_rtn);
	vinfo_rtn = NULL;


	return(vptr);
}



/*
 *	Turns keyboard autorepeat off.
 */
void OSWKBAutoRepeatOff()
{
	if(!IDC())
	    return;

	XAutoRepeatOff(osw_gui[0].display);

	return;
}


/*
 *      Turns keyboard autorepeat on.
 */
void OSWKBAutoRepeatOn()
{
        if(!IDC())
            return;

        XAutoRepeatOn(osw_gui[0].display);

        return;
}



/*
 *	Load font.
 */
int OSWLoadFont(font_t **font, const char *fontname)
{
	if(!IDC() ||
           (fontname == NULL) ||
           (font == NULL)
	)
	    return(-1);


	/* Allocate structure for GUI. */
	*font = (font_t *)calloc(1, sizeof(font_t));
	if(*font == NULL)
	    return(-1);


	/* Load the font. */
	(*font)->actual = XLoadQueryFont(osw_gui[0].display, fontname);
	if((*font)->actual == NULL)
	{
	    /* Load failure, free structure for GUI. */
	    free(*font);
	    *font = NULL;

	    return(-1);
	}
	else
	{
            (*font)->char_width = 0;
            (*font)->char_height = 0;
            (*font)->total_chars = (*font)->actual->n_properties;

	    return(0);
	}
}


/*
 *	Returns the current font pointer set in the graphics
 *	context foreground or NULL on error.
 */
font_t *OSWQueryCurrentFont()
{
	return(osw_gui[0].current_font);
}


/*
 *	Set font for the graphics context foreground.
 */
void OSWSetFont(font_t *font)
{
	if(!IDC() ||
           (font == NULL)
	)
	    return;
	if(font->actual == NULL)
	    return;

	/* No change in font pointers? */
	if(font == osw_gui[0].current_font)
	    return;

	/* Set new font. */
	XSetFont(
	    osw_gui[0].display,
	    osw_gui[0].gc,
	    font->actual->fid
	);

	/* Record current font. */
	osw_gui[0].current_font = font;


	return;
}


/*
 *	Unload font.
 */
void OSWUnloadFont(font_t **font)
{
        if(!IDC() ||
           (font == NULL)
        )
            return;
	if(*font == NULL)
	    return;


	/* Unset current_font as needed. */
	if(*font == osw_gui[0].current_font)
	{
	    osw_gui[0].current_font = NULL;
	}


	/* Unload font. */
	if((*font)->actual != NULL)
	{
	    XFreeFont(
		osw_gui[0].display,
		(*font)->actual
	    );

	    (*font)->actual = NULL;
	}

	/* Free GUI's structure. */
	free(*font);
	*font = NULL;


	return;
}


/*
 *	Load a color pixel by RGB values.
 *
 *	Valid values for r, g, and b range from 0 to 255.
 *
 *      Returns 0 on success, -1 on general error, and -2 on
 *      ambiguous or missing data.
 */
int OSWLoadPixelRGB(
	pixel_t *pix_rtn,
	u_int8_t r,
	u_int8_t g,
	u_int8_t b
)
{
	int i;
        XColor c;
	osw_gui_struct *gui;


	gui = &osw_gui[0];


        if(pix_rtn == NULL)
            return(-1);

        if(!IDC())
        {
            *pix_rtn = 0;
            return(-1);
        }


	if(gui->we_created_colormap)
	{
	    /* We created color map, fetch pixel from our values. */
	    switch(gui->depth)
	    {
	      case 8:
		*pix_rtn = gui->black_pix;

                for(i = 0; i < 256; i++)
                {
                    c.blue = ((i & 0x03) << (5));
                    c.green = ((i & 0x1C) << (3));
                    c.red = ((i & 0xE0) << (0));

		    if((c.blue >= b) &&
                       (c.green >= g) &&
                       (c.red >= r)
		    )
		    {
			*pix_rtn = (pixel_t)i;
			break;
		    }
                }
		if(i >= 256)
		    *pix_rtn = 255;
		return(0);
		break;

	      default:
                *pix_rtn = gui->black_pix;
		return(0);
		break;
	    }
	}
	else
	{
	    /* Using default color map. */

            /* Set colors to be allocated. */
	    c.flags = DoRed | DoGreen | DoBlue;
            c.red = ((u_int16_t)r << 8);
            c.green = ((u_int16_t)g << 8);
            c.blue = ((u_int16_t)b << 8);

            /* Allocate color. */
            if(XAllocColor(gui->display, gui->colormap, &c))
            {
                /* True, successful pixel allocation. */
                *pix_rtn = c.pixel;
                return(0);
            }

            /* False, failed allocation, use global black_pix. */
            *pix_rtn = gui->black_pix;
	}

        return(-1);
}


/*
 *      Load a color pixel by HSL values.
 *
 *      Valid values for h, s, and l range from 0 to 255.
 * 
 *      Returns 0 on success, -1 on general error, an\nd -2 on
 *      ambiguous or missing data.
 */
int OSWLoadPixelHSL(
	pixel_t *pix_rtn,
	u_int8_t h,
	u_int8_t s,
	u_int8_t l 
)
{


        return(-1);
}



/*
 *	Load pixel by given color specification string (CLSP).
 *	Returns zero on success, -1 on
 *
 *	The format for clsp is "rgbi:%lf/%lf/%lf".
 *	(The input format for clsp is native to Xlib.)
 *
 *	Returns 0 on success, -1 on general error, an\nd -2 on
 *	ambiguous or missing data.
 */
int OSWLoadPixelCLSP(pixel_t *pix_rtn, char *clsp)
{
	XColor c;


	if(pix_rtn == NULL)
	    return(-1);

	if(!IDC() ||
           (clsp == NULL)
	)
	{
	    *pix_rtn = 0;
	    return(-1);
	}


	switch(
	    XParseColor(
		osw_gui[0].display,
		osw_gui[0].colormap,
		clsp,
		&c
	    )
	)
	{
	  case BadColor:
            fprintf(stderr,
                "OSWLoadPixelCLSP(): %s: BadColor\n",
                clsp
            );
	    *pix_rtn = 0;
	    return(-1);
            break;

	  case BadValue:
            fprintf(stderr,
                "OSWLoadPixelCLSP(): %s: BadValue\n",
                clsp
            );
	    *pix_rtn = 0;
	    return(-1);
            break;

	  default:
	    return(OSWLoadPixelRGB(
		pix_rtn,
                (c.red >> 8),
                (c.green >> 8),
                (c.blue >> 8)
	    ));
	    break;
	}

	return(0);
}

/*
 *	Set foreground pixel.
 */
void OSWSetFgPix(pixel_t pix)
{
        if(!IDC())
	    return;

	XSetForeground(osw_gui[0].display, osw_gui[0].gc, pix);

	/* Record current fg pixel. */
	osw_gui[0].current_fg_pix = pix;


	return;
}

/*
 *	Deallocates a loaded pixel.
 */
void OSWDestroyPixel(pixel_t *pix_ptr)
{
	osw_gui_struct *gui;


	gui = &osw_gui[0];

	if(!IDC() ||
           (pix_ptr == NULL)
	)
	    return;


	/* Leave default black and white pixels alone. */
	if((*pix_ptr == osw_gui[0].black_pix) ||
           (*pix_ptr == osw_gui[0].white_pix)
	)
	    return;

	/* Unset current pixels as needed. */
	if(osw_gui[0].current_fg_pix == *pix_ptr)
	    osw_gui[0].current_fg_pix = 0;


	/* Free pixel. */
	if(*pix_ptr != 0)
	{
	    /* When we create the colormap, the pixels are not
	     * allocated, therefore we do not need to free them.
	     * See function OSWLoadPixelRGB() for example.
	     */
	    if(!gui->we_created_colormap)
                XFreeColors(
		    osw_gui[0].display,
		    osw_gui[0].colormap,
		    pix_ptr,	/* Pixel array. */
		    1,		/* Number of pixels. */
		    0		/* Planes */
	        );

	    /* Reset pixel value to 0. */
	    *pix_ptr = 0;
	}


	return;
}


/*
 *	Load basic cursor.
 */
cursor_t OSWLoadBasicCursor(cur_code_t code)
{
	if(!IDC())
	    return(0);

	return(XCreateFontCursor(osw_gui[0].display, code));
}

/*
 *      Set cursor to window.
 */
void OSWSetWindowCursor(win_t w, cursor_t cursor)
{
        if(!IDC() ||
           (w == 0) ||
	   (cursor == 0)
	)
            return;

	XDefineCursor(osw_gui[0].display, w, cursor);

        return;
}

/*
 *	Unset cursor on window, window now gets cursor defined on its
 *	parent.
 */
void OSWUnsetWindowCursor(win_t w)
{
        if(!IDC() ||
           (w == 0)
        )
            return;

	XUndefineCursor(osw_gui[0].display, w);

        return;
}

/*
 *	Destroy a cursor.
 */
void OSWDestroyCursor(cursor_t *cursor)
{
        if(!IDC() ||
	   (cursor == NULL)
	)
            return;

	if(*cursor != 0)
	{
	    XFreeCursor(osw_gui[0].display, *cursor);
	    *cursor = 0;
	}

	return;
}



/*
 *	Check events.
 */
int OSWEventsPending()
{
	if(!IDC())
	    return(0);

	return(XPending(osw_gui[0].display));
}


/*
 *	Block execution untill next event is recieved.
 *	Recieved event will be placed at the address of event.
 */
void OSWWaitNextEvent(event_t *event)
{
	if(event == NULL)
	    return;
	else
	    memset(event, 0x00, sizeof(event_t));

        if(!IDC())
	    return;

	XNextEvent(osw_gui[0].display, event);
	OSWManageEvent(event);

	return;
}


/*
 *      Block execution untill next event is recieved.      
 *      Recieved event will be coppied to the address of event,
 *	it will not be removed from the queue.
 */
void OSWWaitPeakEvent(event_t *event)
{
        if(event == NULL)
            return;
	else
	    memset(event, 0x00, sizeof(event_t));

        if(!IDC())
	    return;

        XPeekEvent(osw_gui[0].display, event);
	OSWManageEvent(event);

        return;
}


/*
 *	Checks the event queue for an event type matching the
 *	specified eventmask.
 *
 *	If an event is found, it will be removed from the queue
 *	and coppied to the event, returning True.
 *	If no event is found, then False is returned.
 */
bool_t OSWCheckMaskEvent(eventmask_t eventmask, event_t *event)
{
	bool_t status;


	if(!IDC() ||
           (event == NULL)
        )
	    return(False);


	status = XCheckMaskEvent(
            osw_gui[0].display,
            eventmask,
            event
        );
	if(status)
	    OSWManageEvent(event);

	return(status);
}       


/*
 *	The OSWWaitWindowEvent function searches the event queue for an event
 *	that matches both the specified window and event mask.  When it finds a
 *      match, OSWWaitWindowEvent removes that event from the queue and copies
 *	it into the specified event_t structure.  The other events stored in
 *	the queue are not discarded.  If a matching event is not in the queue,
 *	OSWWaitWindowEvent flushes the output buffer and blocks until one is
 *	received.
 */
void OSWWaitWindowEvent(win_t w, eventmask_t event_mask, event_t *event)
{
        if(event == NULL)
            return;
        else
            memset(event, 0x00, sizeof(event_t));

        if(!IDC() ||
           (w == 0)
	)
            return;

	XWindowEvent(osw_gui[0].display, w, event_mask, event);
	OSWManageEvent(event);

	return;
}



/*
 *	Checks event matching event_mask on window w.
 *	Returns True if matched event is found.
 */
bool_t OSWCheckWindowEvent(win_t w, eventmask_t event_mask, event_t *event)
{
	bool_t status;


        if(event == NULL)
            return(False);
        else
            memset(event, 0x00, sizeof(event_t));
       
        if(!IDC() ||
           (w == 0)
	)
            return(False);
 
	status = XCheckWindowEvent(
	    osw_gui[0].display,
	    w,
	    event_mask,
	    event
	);
	if(status)
	    OSWManageEvent(event);

	return(status);
} 


/*
 *	Puts the given event which was fetched earlier back onto
 *	the queue.   The given event should have been originally
 *	fetched from one of the event recieving functions and not
 *	just `made up'.
 */
void OSWPutBackEvent(event_t *event)
{
	if(!IDC() ||
           (event == NULL)
	)
	    return;

	XPutBackEvent(
	    osw_gui[0].display,
	    event
	);

	return;
}


/*
 *	Sends an event to the GUI, which can then be recieved by
 *	any of the event fetching functions.
 *
 *	Event members xany.window and type needs to be specified
 *	properly in the event_t structure.
 */
int OSWSendEvent(
	eventmask_t mask,
	event_t *event,
	bool_t propagate
)
{
	int status;


	if(!IDC() ||
           (event == NULL)
	)
	    return(-1);

	if(event->xany.window == 0)
	    return(-1);

	status = XSendEvent(
	    osw_gui[0].display,
	    event->xany.window,
	    propagate,
	    mask,
	    event
	);

	if(status)
	    return(0);
	else
	    return(-1);
}



/*
 *	Purges all events (if any) in the queue.
 */
int OSWPurgeAllEvents()
{
	event_t event;
	int i = 0;


        if(!IDC())
	    return(i);

	while(OSWEventsPending() > 0)
	{
	    OSWWaitNextEvent(&event);
	    i++;
	}

        return(i);
}


/*
 *	Removes all MotionNotify events except the most
 *	recent one.
 */
int OSWPurgeOldMotionEvents()
{
	event_t event;
	int i = 0;


	if(!IDC())
	    return(i);


        while(XCheckTypedEvent(osw_gui[0].display, MotionNotify, &event))
        {
            i++;
        }

	/* Put the last MotionNotify event back if there was one. */
	if(i > 0)
	{
	    XPutBackEvent(osw_gui[0].display, &event);
	}


	return(i);
}


/*
 *	Purges event table only of events matching event_type.
 */
int OSWPurgeTypedEvent(int event_type)
{
        event_t event;
	int i = 0;


	if(!IDC())
	    return(i);

	while(XCheckTypedEvent(osw_gui[0].display, event_type, &event))
	{
	    i++;
	}

	return(i);
}



/*      
 *      Purges event table only of events matching event_type and
 *	is for window w.
 */
int OSWPurgeWindowTypedEvent(win_t w, eventtype_t event_type)
{
        event_t event;
        int i = 0;  
	osw_gui_struct *gui;


	gui = &osw_gui[0];

        if(!IDC() || (w == 0))
            return(i);

        while(XCheckTypedWindowEvent(
	    gui->display, w, event_type, &event
	))
            i++;

        return(i);
}


/*
 *	Checks if event is a request to destroy window w.
 */
int OSWIsEventDestroyWindow(win_t w, event_t *event)
{
        if(!IDC() ||
           (w == 0) ||
           (event == NULL)
        )
            return(0);
 

        if((event->xclient.format == 32) &&
           ((Atom)event->xclient.data.l[0] == (Atom)osw_atom.wm_delete_window) &&
           (event->xany.window == w)
        )
            return(1);
        else
            return(0);
}



/*
 *	Create a window.
 */
int OSWCreateWindow(
        win_t *w,
        win_t parent,  
        int x, int y,
        unsigned int width, unsigned int height
)
{
	atom_t atom[10];
	osw_gui_struct *gui;
        XSetWindowAttributes swattr;


	gui = &osw_gui[0];

	if(!IDC() ||
           (w == NULL) ||
           (parent == 0) ||
           (width == 0) ||
           (height == 0)
	)
	    return(-1);


        /* Set up the window attributes. */
        memset(&swattr, 0x00, sizeof(XSetWindowAttributes));

	swattr.background_pixmap = ParentRelative;
        swattr.background_pixel = gui->black_pix;
        swattr.border_pixel = gui->white_pix;
        swattr.bit_gravity = NorthWestGravity;
        swattr.win_gravity = NorthWestGravity;
        swattr.backing_store = WhenMapped;	/* NotUseful, WhenMapped, Always */
        swattr.backing_planes = AllPlanes;
        swattr.backing_pixel = gui->black_pix;
        swattr.save_under = False;
	swattr.event_mask = 0;
	swattr.do_not_propagate_mask = 0;
	swattr.override_redirect = False;
	if(gui->we_created_colormap)
            swattr.colormap = gui->colormap;
	else
	    swattr.colormap = CopyFromParent;
        swattr.cursor = CopyFromParent;
/*	swattr.cursor = gui->std_cursor; */


        /* Create Window. */
        *w = XCreateWindow(
            gui->display,
            parent,
            x, y,
            width, height,
            0,			/* Border (internal, we don't want it). */
            gui->depth,		/* Operative bit depth. */
            InputOutput,	/* InputOutput or InputOnly? */
            gui->visual,	/* Should we use CopyFromParent? */
	    CWBackPixel  | CWBorderPixel | CWBackPixmap | CWBorderPixmap|
            CWBitGravity | CWWinGravity  | CWColormap   | CWBackingStore|
	    CWBackingPlanes | CWBackingPixel | CWOverrideRedirect |
	    CWSaveUnder | CWCursor,
            &swattr		/* Set attributes. */
        );
        if(*w == 0)
            return(-1);

	/* If the parent is root_win, set wm_delete_window property atom. */
	if(parent == gui->root_win)
	{
/*	    event_t rse; */	/* Resize event. */


	    atom[0] = osw_atom.wm_delete_window;

            XSetWMProtocols(
                gui->display,
                *w,
                atom,
                1
            );

	    /* Need to send an event for toplevel window resize. */
/*
	    rse.type = ConfigureNotify;
	    rse.xany.window = *w;
	    rse.xconfigure.x = x;
            rse.xconfigure.y = y;
            rse.xconfigure.width = width;
            rse.xconfigure.height = height;

	    OSWSendEvent(
		ConfigureNotify,
		&rse,
		True
	    );
 */
	}

        
	return(0);
}

/*
 *	Create an input only window.
 */
int OSWCreateInputWindow(
	win_t *w,
	win_t parent,
	int x, int y,
	unsigned int width, unsigned int height
)
{
        osw_gui_struct *gui;
        XSetWindowAttributes swattr;


        gui = &osw_gui[0];

        if(!IDC() ||
           (w == NULL) ||
           (parent == 0) ||
           (width == 0) ||
           (height == 0)
        )
            return(-1);


        /* Set up the window attributes. */
        memset(&swattr, 0x00, sizeof(XSetWindowAttributes));

        swattr.background_pixmap = ParentRelative;
        swattr.background_pixel = gui->black_pix;
        swattr.border_pixel = gui->white_pix;
        swattr.bit_gravity = NorthWestGravity;
        swattr.win_gravity = NorthWestGravity;
        swattr.backing_store = WhenMapped;	/* NotUseful, WhenMapped, Always. */
        swattr.backing_planes = AllPlanes;
        swattr.backing_pixel = gui->black_pix;
        swattr.save_under = False;
        swattr.event_mask = 0;
        swattr.do_not_propagate_mask = 0;
        swattr.override_redirect = False;
        if(gui->we_created_colormap)
            swattr.colormap = gui->colormap;
        else
            swattr.colormap = CopyFromParent;
        swattr.cursor = CopyFromParent;
/*      swattr.cursor = gui->std_cursor; */


        /* Create an input only window. */
        *w = XCreateWindow(
            gui->display,
            parent,
            x, y,
            width, height,
            0,			/* Border (internal, we don't want it). */
            0,			/* Operative bit depth (must be 0). */
            InputOnly,		/* InputOutput or InputOnly? */
            gui->visual,	/* Should we use CopyFromParent? */
	    0,			/* Set attributes mask. */
            &swattr		/* Set attributes. */
        );
        if(*w == 0)
            return(-1);

	return(0);
}

/*
 *	Destroy window.
 */
void OSWDestroyWindow(win_t *w)
{
	if(!IDC() ||
           (w == NULL)
	)
	    return;

	if(*w != 0)
	{
	    XDestroyWindow(osw_gui[0].display, *w);
	    *w = 0;
	}

	return;
}

/*
 *	Checks if the drawable is window.
 */
bool_t OSWDrawableIsWindow(drawable_t d)
{
        int status;
        osw_gui_struct *gui;
        win_attr_t wattr;


        gui = &osw_gui[0];

        if(!IDC() ||
           (d == 0)
        )
            return(False);


        /* Use simple error handler for this operation. */
        static_osw_x::gui_error_code = 0;     /* Reset error code. */
        XSetErrorHandler(OSWGUISimpleErrorHandler);

        /* Get window attributes of d, if this fails then
         * d is a pixmap_t. If this is successful then d is
         * a win_t.
         */
        XGetWindowAttributes(
            gui->display,
            d,
            &wattr
        );
        status = static_osw_x::gui_error_code;        /* Get error code. */

        /* Set back error handler. */
        XSetErrorHandler(OSWGUIErrorHandler);

	return(status ? False : True);
}


/*
 *	GUI wrapper's set window icon function.
 */
void OSWSetWindowIcon(win_t w, pixmap_t icon, bool_t has_transparency)
{
	int x, y, bytes_per_line, bytes_per_pixel;
        GC gc;
        XGCValues gcv;
	image_t *image;
	pixel_t white_pix, black_pix;
	long supplied_return;

        XIconSize *icon_sizes;
        int count, i;
	unsigned int width = 64, height = 64;

	XWMHints wm_hints;
	XClassHint class_hints;
	XSizeHints sz_hints;


        if(!IDC() ||
           (w == 0) ||
	   (icon == 0)
        )
            return;


	/* Set new icon referance in wm hints structure. */
	wm_hints.icon_pixmap = icon;

        /* Set class hint names. */
        class_hints.res_name = "Eterm"; /* Must call it that. */
        class_hints.res_class = "Eterm";


	/* Get original size hints. */
	XGetWMNormalHints(
	    osw_gui[0].display,
	    w,
	    &sz_hints,
	    &supplied_return
	);
	/* Update values. */
        if(!(sz_hints.flags & PResizeInc))
        {
            sz_hints.flags |= PResizeInc;
            sz_hints.width_inc = 1;
            sz_hints.height_inc = 1;
        }
	if(!(sz_hints.flags & PBaseSize))
	{
	    sz_hints.flags |= PBaseSize;
            sz_hints.base_width = 0;
            sz_hints.base_height = 0;
	}

	/* Get valid/suggested icon sizes. */
        if(
	    XGetIconSizes(
		osw_gui[0].display,
		osw_gui[0].root_win,
		&icon_sizes,
		&count
	    )
	)
	{
            for(i = 0; i < count; i++)
	    {
                width = MIN(icon_sizes[i].max_width, 64);
                height = MIN(icon_sizes[i].max_height, 64);
            }
            XFree(icon_sizes);
	}


	/* Create a 1 bit depth pixmap mask. */
        wm_hints.icon_mask = XCreatePixmap(
            osw_gui[0].display,
            osw_gui[0].root_win,
            width, height,
            1                           /* Depth of 1. */
        );

        /* Create tempory GC. */
        white_pix = osw_gui[0].white_pix;
        black_pix = osw_gui[0].black_pix;
        gcv.function = GXcopy;
        gcv.plane_mask = 1;		/* Monochrome. */
        gcv.foreground = white_pix;
        gcv.background = black_pix;
        gcv.line_width = 1;
        gc = XCreateGC(
            osw_gui[0].display,
            wm_hints.icon_mask,
            GCFunction | GCPlaneMask | GCForeground | GCBackground |
                GCLineWidth,
            &gcv
        );
	/* Convert icon to a image and then `copy over' mask. */
	image = OSWGetImage(icon, 0, 0, width, height);
	if(image != NULL)
	{
            u_int8_t *img_data;
            u_int8_t *ptr8;      
            u_int16_t *ptr16;
            u_int32_t *ptr32;
	    pixmap_t pixmap;

            img_data = reinterpret_cast<u_int8_t *>(image->data);
	    pixmap = wm_hints.icon_mask;

            switch(image->depth)
	    {
              /* 8 bits. */
              case 8:
		if(img_data == NULL)
                    break;
		bytes_per_pixel = BYTES_PER_PIXEL8;
                bytes_per_line = image->width * bytes_per_pixel;
                for(y = 0; y < image->height; y++)
                {
                    for(x = 0; x < image->width; x++)
                    {
                        ptr8 = reinterpret_cast<u_int8_t *>(&img_data[
                            (y * bytes_per_line) +
                            (x * bytes_per_pixel)
                        ]);
                        if(*ptr8)
                            XSetForeground(osw_gui[0].display, gc, white_pix);
                        else
                            XSetForeground(osw_gui[0].display, gc, black_pix);
                        XDrawPoint(osw_gui[0].display, pixmap, gc, x, y);
                    }
                }
                break; 

              /* 15 or 16 bits. */
              case 15:
              case 16:
                if(img_data == NULL)
                    break;
                bytes_per_pixel = BYTES_PER_PIXEL16;
                bytes_per_line = image->width * bytes_per_pixel;
                for(y = 0; y < image->height; y++)
                {
                    for(x = 0; x < image->width; x++)
                    { 
                        ptr16 = (u_int16_t *)(&img_data[
                            (y * bytes_per_line) +
                            (x * bytes_per_pixel)
                        ]);
                        if(*ptr16)
                            XSetForeground(osw_gui[0].display, gc, white_pix);
                        else
                            XSetForeground(osw_gui[0].display, gc, black_pix);
                        XDrawPoint(osw_gui[0].display, pixmap, gc, x, y);
                    }
                }
                break;

              /* 24 or 32 bits. */
              case 24:
              case 32:
                if(img_data == NULL)
                    break;
		bytes_per_pixel = BYTES_PER_PIXEL32;
		bytes_per_line = image->width * bytes_per_pixel;
		for(y = 0; y < image->height; y++)
		{       
                    for(x = 0; x < image->width; x++)
                    {
                        ptr32 = (u_int32_t *)(&img_data[
                            (y * bytes_per_line) +
                            (x * bytes_per_pixel)
                        ]);
                        if(*ptr32)
                            XSetForeground(osw_gui[0].display, gc, white_pix);
                        else
                            XSetForeground(osw_gui[0].display, gc, black_pix);
                        XDrawPoint(osw_gui[0].display, pixmap, gc, x, y);
                    }
                }
                break;
	    }

	    OSWDestroyImage(&image);
	}
        XFreeGC(osw_gui[0].display, gc);	/* Free temp GC. */


	/* Create window to be used for icon. */
        wm_hints.icon_window = XCreateSimpleWindow(
	    osw_gui[0].display,
	    w,			/* Parented to w (toplevel?). */
	    0, 0,
	    width, height,
	    0,
	    0L, 0L
	);

	XShapeCombineMask(
	    osw_gui[0].display,
	    wm_hints.icon_window,
	    ShapeBounding,
	    0, 0,
	    wm_hints.icon_mask,
	    ShapeSet
	);
	XSetWindowBackgroundPixmap(
	    osw_gui[0].display,
	    wm_hints.icon_window,
	    wm_hints.icon_pixmap
	);

	/* Extra stuff, may be needed. */
	wm_hints.window_group = w;
	wm_hints.input = True;
	wm_hints.initial_state = NormalState;

	wm_hints.flags = (
	    InputHint |
	    StateHint |
	    WindowGroupHint |
	    IconPixmapHint |
	    IconWindowHint |
	    IconMaskHint
	);


	/* Set new properties. */
	XSetWMProperties(
	    osw_gui[0].display,
	    w,
	    NULL, NULL,
	    (char **)NULL, 0,
	    &sz_hints,		/* Reset original size hints. */
	    &wm_hints,		/* WM icon hints. */
	    &class_hints	/* Nessasary gibberish. */
	);


	return;
}


/*
 *	Sets standard window manager properties for window w.
 */
void OSWSetWindowWMProperties(
        win_t w,
        char *title,
        char *icon_title,
        pixmap_t icon,
	bool_t wm_sets_coordinates,
        int x, int y,
        unsigned int min_width, unsigned int min_height,
        unsigned int max_width, unsigned int max_height,
        int frame_style,
        char **argv, int argc
)
{
	sizehints_t sizehints;
	win_t parent;
	PropMwmHints mwm_prop;		/* WM window decoration control. */
        XSetWindowAttributes swattr;


        if(!IDC() ||
           (w == 0)
        )
            return;


	if(title == NULL)
	    title = "Untitled";
	if(icon_title == NULL) 
            icon_title = "Untitled";


	memset(&sizehints, 0x00, sizeof(sizehints_t));
	if(wm_sets_coordinates)
	{
	    /* Let window manager set coordinates. */
	    sizehints.flags = PPosition | PSize | PMinSize | PMaxSize |
		PResizeInc | PBaseSize;
	    sizehints.x = 0;
	    sizehints.y = 0;
	}
	else
        {
            /* Use given input coordinates. */
            sizehints.flags = USPosition | PSize | PMinSize | PMaxSize |
		PResizeInc | PBaseSize;
            sizehints.x = x;
            sizehints.y = x;
        }

	/* Sanitize sizes. */
	if(max_width > osw_gui[0].display_width)
	    max_width = osw_gui[0].display_width;
	if(max_height > osw_gui[0].display_height)
	    max_height = osw_gui[0].display_height;
	if(max_width < 1)
	    max_width = 1;
	if(max_height < 1)
	    max_height = 1;

	if(min_width > max_width)
	    min_width = max_width;
        if(min_height > max_height)
	    min_height = max_height;

	/* Set sizes. */
        sizehints.min_width = min_width;
        sizehints.min_height = min_height;
        sizehints.max_width = max_width;
        sizehints.max_height = max_height;
        sizehints.width_inc = 1;
        sizehints.height_inc = 1;
        sizehints.base_width = 0;
        sizehints.base_height = 0;

	XSetWMNormalHints(osw_gui[0].display, w, &sizehints);

	/* Set standard properties. */
        XSetStandardProperties(
            osw_gui[0].display,
            w,
            title, icon_title,
            icon,
            argv, argc,
            &sizehints
        );


        /*   Check if this is a toplevel window (a window parented to
	 *   the root window).  If it is, we need to set it's
	 *   `frame' and `decoration' values.
	 */
	parent = OSWGetWindowParent(w);
        if(parent == osw_gui[0].root_win)
        {
	    /* Set up mwm properties. */
            mwm_prop.flags = MWM_HINTS_FUNCTIONS |
			     MWM_HINTS_DECORATIONS;
/*
			     MWM_HINTS_INPUT_MODE |
			     MWM_HINTS_STATUS;
 */

            mwm_prop.functions = (MWM_FUNC_RESIZE   |
                                  MWM_FUNC_MOVE     |
                                  MWM_FUNC_MINIMIZE |
                                  MWM_FUNC_MAXIMIZE |
                                  MWM_FUNC_CLOSE
            );

            mwm_prop.decorations = (MWM_DECOR_BORDER   |
                                    MWM_DECOR_RESIZEH  |
                                    MWM_DECOR_TITLE    |
                                    MWM_DECOR_MENU     |
                                    MWM_DECOR_MINIMIZE |
                                    MWM_DECOR_MAXIMIZE
            );

	    mwm_prop.inputMode = 0;
            mwm_prop.status = 0;

	    /* Set frame style. */
	    switch(frame_style)
	    {
	      /* No frame, no title, no buttons... nothing. */
	      case WindowFrameStyleNaked:
                /* Set up the window attributes. */
		swattr.override_redirect = True;
		XChangeWindowAttributes(
		    osw_gui[0].display,
		    w,
		    CWOverrideRedirect,
		    &swattr
		);
		break;

	      /* Fixed frame. */
	      case WindowFrameStyleFixed:
                mwm_prop.decorations &= ~(MWM_DECOR_RESIZEH);
                mwm_prop.functions &= ~(MWM_FUNC_RESIZE);
		XChangeProperty(
		    osw_gui[0].display,
                    w,
		    osw_atom._motif_wm_hints,
		    osw_atom._motif_wm_hints,
		    32,				/* Bits. */
		    PropModeReplace,
		    (unsigned char *)&mwm_prop,
		    PROP_MWM_HINTS_ELEMENTS
		);
                break;

              /* Fixed frame, no title, no buttons. */
              case WindowFrameStyleFixedFrameOnly:
                mwm_prop.decorations &= ~(MWM_DECOR_RESIZEH  |
                                          MWM_DECOR_TITLE    |
                                          MWM_DECOR_MENU     |
                                          MWM_DECOR_MINIMIZE |
                                          MWM_DECOR_MAXIMIZE
                );
                mwm_prop.functions &= ~(MWM_FUNC_RESIZE   |
                                        MWM_FUNC_MINIMIZE |
                                        MWM_FUNC_MAXIMIZE
                );
                XChangeProperty(
                    osw_gui[0].display,
                    w,
                    osw_atom._motif_wm_hints,
                    osw_atom._motif_wm_hints,
                    32,                         /* Bits. */
                    PropModeReplace,
                    (unsigned char *)&mwm_prop,
                    PROP_MWM_HINTS_ELEMENTS
                );
                break;

              /* No close button or menu item. */
              case WindowFrameStyleNoClose:
                mwm_prop.functions &= ~(MWM_FUNC_CLOSE);
                XChangeProperty(
                    osw_gui[0].display,
                    w,
                    osw_atom._motif_wm_hints,
                    osw_atom._motif_wm_hints,
                    32,                         /* Bits. */
                    PropModeReplace,
                    (unsigned char *)&mwm_prop,
                    PROP_MWM_HINTS_ELEMENTS
                );
		break;

	      /* Standard. */
	      default:	/* WindowFrameStyleStandard */
		break;
	    }
        }

	OSWSetWindowIcon(w, icon, True);


	return;
}


/*
 *	Select events for window w to recieve.
 */
void OSWSetWindowInput(win_t w, eventmask_t eventmask)
{
	if(!IDC() ||
           (w == 0)
	)
	    return;

	XSelectInput(osw_gui[0].display, w, eventmask);

	return;
}

/*
 *	Sets wbum to be a transient window for wshelter.
 *
 *	Both wbum and wshelter need to be created and toplevel
 *	windows (windows parented to the root window).
 */
void OSWSetTransientFor(win_t wbum, win_t wshelter)
{
	if(!IDC() ||
           (wbum == 0) ||
	   (wshelter == 0)
	)
	    return;

	XSetTransientForHint(osw_gui[0].display, wbum, wshelter);

	return;
}


/*
 *	Set window title.
 */
void OSWSetWindowTitle(win_t w, char *title)
{
	if(!IDC() ||
           (w == 0) ||
	   (title == NULL)
	)
	    return;

	XStoreName(osw_gui[0].display, w, title);


	return;
}


/*
 *	Clear / refresh window.
 */
void OSWClearWindow(win_t w)
{
        if(!IDC() ||
           (w == 0)
        )
            return;

	XClearWindow(osw_gui[0].display, w);


	return;
}


/*
 *	Set window background color or tile.
 */
void OSWSetWindowBkg(win_t w, pixel_t pix, pixmap_t pixmap)
{
	if(!IDC() ||
           (w == 0)
	)
	    return;

	if(pixmap == 0)
	    XSetWindowBackground(osw_gui[0].display, w, pix);
	else
	    XSetWindowBackgroundPixmap(osw_gui[0].display, w, pixmap);

	return;
}

/*
 *	Gets window attributes, returns True if successful.
 */
bool_t OSWGetWindowAttributes(win_t w, win_attr_t *wattr)
{
	int status;
	osw_gui_struct *gui;


	gui = &osw_gui[0];

        if(!IDC() ||
           (wattr == NULL)
        )
            return(False);

	if(w == 0)
	{
	    memset(wattr, 0x00, sizeof(win_attr_t));
	    return(False);
	}


        /* Use simple error handler for this operation. */
        static_osw_x::gui_error_code = 0;     /* Reset error code. */
        XSetErrorHandler(OSWGUISimpleErrorHandler);

	/* Get window attributes. */
	XGetWindowAttributes(gui->display, w, wattr);
	status = static_osw_x::gui_error_code;

	/* Set back to use standard error handler. */
	XSetErrorHandler(OSWGUIErrorHandler);

	return(status ? False : True);
}

/*
 *	Get window position relative to root_win.
 *
 *	Returns -2 if information cannot be obtained or is ambiguous.
 *	Returns -1 on general error.
 */
int OSWGetWindowRootPos(win_t w, int *x, int *y)
{
	win_t root_return;
	win_t child_return;
	int root_x_return, root_y_return;
        int win_x_return, win_y_return;
        unsigned int mask_return;
	bool_t b;


	if(!IDC() ||
	   (w == 0)
	)
	    return(-1);


	/*
	 *   Note: XLib has no low-level function to
	 *   obtain a windows' position relative to root.
	 *   We are fetching it through multiple steps through alternative
	 *   function calls.
	 */
	b = XQueryPointer(
	    osw_gui[0].display,
	    w,
	    &root_return, &child_return,
	    &root_x_return, &root_y_return,
            &win_x_return, &win_y_return,
	    &mask_return
	);


	if(b)
	{
            if(x != NULL)
                *x = root_x_return - win_x_return;  
            if(y != NULL)
                *y = root_y_return - win_y_return;
	}
	else
	{
	    /* Pointer is not on same window as w. */
	    if(x != NULL)
		*x = 0;
	    if(y != NULL)
		*y = 0;

	    return(-2);
	}


	return(0);
}



/*
 *	Get parent window id of window w.  Returns 0 on error.
 */
win_t OSWGetWindowParent(win_t w)
{
	int status;
        win_t parent_rtn;
        win_t root_return;   
        win_t *children_return;
        unsigned int nchildren_return;


        /* Error checks. */
        if(!IDC() ||
           (w == 0) ||
           (osw_gui[0].root_win == 0)
        )
            return(0);

        /* Check if w is already root window? */
        if(osw_gui[0].root_win == w)
            return(osw_gui[0].root_win);


        /* Get tree. */
        children_return = NULL;
        status = XQueryTree(
            osw_gui[0].display,
            w,
            (win_t *)&root_return,
            (win_t *)&parent_rtn,
            (win_t **)&children_return,
            (unsigned int *)&nchildren_return
        );
            
   
        /* XQueryTree() returns zero (yes zero) if it failed. */
        if(status == 0)
        {
            fprintf(stderr,  
                "WidgetGetWindowParent(): XQueryTree(): Failed.\n"
            );
            return(0); 
        }

        /* Need to free children_return? */
        if(children_return != NULL)
            XFree((void *)children_return);
            

        return(parent_rtn);
}



/*
 *	Checks if window grand_child is really a decendant of window
 *	grand_parent.
 */
bool_t OSWCheckWindowAncestory(win_t grand_parent, win_t grand_child)
{
        static int count;
        static win_t test_w, old_w;
        static win_t root_w;
        

        /* Error checks. */
        if(!IDC() ||
           (osw_gui[0].root_win == 0)
        )
	    return(False); 

        
        /* Get root window. */
        root_w = osw_gui[0].root_win;
        
        
        /* Check for consiquential match. */
        if(grand_parent == grand_child)
            return(True);
        else if((grand_child == root_w) || (grand_child == 0))
            return(False);
        else if((grand_parent == root_w) || (grand_parent == 0))
            return(False);
            
            
        /* Begin checking for ancestry. */
        test_w = grand_child;
        for(count = 0; count < 16; count++)   
        {
            /* Check if test_w is grand_parent. */
            if(test_w == grand_parent)
                return(True);
  
   
            /* Get test_w parent as test_w. */
            old_w = test_w;
            test_w = OSWGetWindowParent(old_w);
 

            /* Make sure test_w is valid. */
            if(test_w == 0)   
                return(False);
            else if(test_w == root_w)
                return(False);
        }
        
        
        /* Too many recursions, return False. */
        return(False);
}       



/*
 *	Map window w.
 */
void OSWMapWindow(win_t w)
{
	if(!IDC() ||
           (w == 0)
	)
	    return;

	XMapWindow(osw_gui[0].display, w);


	return;
}



/*
 *	Map and raise window w.
 */
void OSWMapRaised(win_t w)
{
/*
	event_t event;
	Window root;
	int x, y;
	unsigned int width, height, border, depth;
 */
	osw_gui_struct *gui;


        if(!IDC() ||
           (w == 0)
        )
            return;

	gui = &osw_gui[0];

	XMapRaised(gui->display, w);
/*
	OSWGUISync(False);
	XGetGeometry(
	    gui->display, w, &root,
	    &x, &y, &width, &height,
	    &border, &depth
	);

	memset(&event, 0x00, sizeof(event_t));
	event.type = ConfigureNotify;
	event.xany.window = w;
        event.xconfigure.x = x;
        event.xconfigure.y = y;
        event.xconfigure.width = width;
	event.xconfigure.height = height;
	if(OSWSendEvent(StructureNotifyMask, &event, False))
	    printf("OSWSendEvent() error.\n");
	else
	    printf("OSWSendEvent() sent.\n");
 */

	return;
}



/*
 *	Map all child windows.
 */
void OSWMapSubwindows(win_t w)
{
        if(!IDC() ||
           (w == 0)
        )
            return; 

	XMapSubwindows(osw_gui[0].display, w);


	return;
}


/*
 *	Unmap window.
 */
void OSWUnmapWindow(win_t w)
{
        if(!IDC() ||
           (w == 0)
        )
            return;


	XUnmapWindow(osw_gui[0].display, w);


	return;
}


/*
 *	Changes the order of stacking of the given windows
 *	in the array w (from top to bottom).
 */
void OSWRestackWindows(win_t *w, int num_w)
{
	int i, n;


        if(!IDC() ||
           (w == NULL) ||
           (num_w <= 0)
        )
            return;


	/* Remove windows with index values of 0. */
	for(i = 0; i < num_w; i++)
	{
	    while((w[i] == 0) && (i < num_w))
	    {
		num_w--;

		for(n = i; n < num_w; n++)
		    w[n] = w[n + 1];
	    }
	}


	XRestackWindows(
	    osw_gui[0].display,
	    w,
	    num_w
	);


	return;
}


/*
 *	Iconifies window.   The window should be a toplevel window.
 */
int OSWIconifyWindow(win_t w)
{
	if(!IDC() ||
           (w == 0)
	)
	    return(-1);


	XIconifyWindow(
	    osw_gui[0].display,
	    w,
	    osw_gui[0].scr_num
	);


	return(0);
}

/*
 *	Reparent window.
 */
void OSWReparentWindow(win_t w, win_t parent)
{
	if(!IDC() ||
           (w == 0) ||
           (parent == 0)
        )
            return;

	XReparentWindow(osw_gui[0].display, w, parent, 0, 0);


	return;
}



/*
 *	Move window.
 */
void OSWMoveWindow(win_t w, int x, int y)
{
	if(!IDC() ||
           (w == 0)
	)
	    return;

	XMoveWindow(osw_gui[0].display, w, x, y);


        return;
}



/*
 *	Resize window.
 */
void OSWResizeWindow(win_t w, unsigned int width, unsigned int height)
{
        if(!IDC() ||
           (w == 0)
        )
            return;

	XResizeWindow(osw_gui[0].display, w, width, height);


        return;
}



/*
 *	Move and resize window in one step.
 */
void OSWMoveResizeWindow(
	win_t w,
	int x, int y,
        unsigned int width, unsigned int height)
{
        if(!IDC() ||
           (w == 0)
        )
            return;

	XMoveResizeWindow(osw_gui[0].display, w, x, y, width, height);


	return;
}



/*
 *	Create a graphics buffer / pixmap.
 */
int OSWCreatePixmap(pixmap_t *pixmap, unsigned int width, unsigned int height)
{
	osw_gui_struct *gui;


	gui = &osw_gui[0];

	if(!IDC() ||
           (gui->root_win == 0) ||
           (pixmap == NULL) ||
           (width == 0) ||
           (height == 0)
	)
	    return(-1);

	*pixmap = XCreatePixmap(
	    gui->display,
	    gui->root_win,
	    width,
	    height,
	    gui->depth		/* Operative depth. */
	);
	if(*pixmap == 0)
	    return(-1);

	return(0);
}

/*
 *	Destroy a graphics buffer / Pixmap.
 */
void OSWDestroyPixmap(pixmap_t *pixmap)
{
        if(!IDC() ||  
           (pixmap == NULL)
        )
	    return;

	if(*pixmap != 0)
	{
	    XFreePixmap(osw_gui[0].display, *pixmap);
	    *pixmap = 0;
	}

	return;
}

/*
 *	Checks if the drawable is a pixmap.
 */
bool_t OSWDrawableIsPixmap(drawable_t d)
{
	int status;
	osw_gui_struct *gui;
	win_attr_t wattr;


	gui = &osw_gui[0];

	if(!IDC() ||
           (d == 0)
        )
	    return(False);


	/* Use simple error handler for this operation. */
	static_osw_x::gui_error_code = 0;	/* Reset error code. */
	XSetErrorHandler(OSWGUISimpleErrorHandler);

	/* Get window attributes of d, if this fails then
	 * d is a pixmap_t. If this is successful then d is
	 * a win_t.
	 */
	XGetWindowAttributes(
	    gui->display,
	    d,
	    &wattr
	);
	status = static_osw_x::gui_error_code;	/* Get error code. */

	/* Set back error handler. */
	XSetErrorHandler(OSWGUIErrorHandler);

	return(status ? True : False);
}

/*
 *	Gets attributes of pixmap.
 */
bool_t OSWGetPixmapAttributes(pixmap_t pixmap, pixmap_attr_t *pattr)
{
        int x, y, status;
        osw_gui_struct *gui;


        gui = &osw_gui[0];

	if(!IDC() ||
	   (pattr == NULL)
	)
	    return(False);

	if(pixmap == 0)
	{
	    memset(pattr, 0x00, sizeof(pixmap_attr_t));
	    return(False);
	}

        /* Use simple error handler for this operation. */
        static_osw_x::gui_error_code = 0;     /* Reset error code. */
        XSetErrorHandler(OSWGUISimpleErrorHandler);

        /* Get pixmap attributes. */
	XGetGeometry(
	    gui->display,
	    pixmap,
	    &(pattr->root_win),		/* Root window. */
	    &x, &y,			/* Coordinates? */
	    &(pattr->width), &(pattr->height),	/* Size. */
	    &(pattr->border_width),	/* Border width. */
	    reinterpret_cast<unsigned int *>(&(pattr->depth))		/* Operative depth. */
        );
        status = static_osw_x::gui_error_code;        /* Get error code. */

        /* Set back to use standard error handler. */
        XSetErrorHandler(OSWGUIErrorHandler);

	return(status ? False : True);
}

/*
 *	Clear a graphics buffer / Pixmap.
 */
void OSWClearPixmap(
	pixmap_t pixmap,
	unsigned int width, unsigned int height,
	pixel_t pix
)
{
        if(!IDC() ||   
           (pixmap == 0) ||
           (width == 0) ||
           (height == 0)
        )
            return;


	OSWSetFgPix(pix);

	OSWDrawSolidRectangle(
	    (drawable_t)pixmap,
	    0, 0,
	    width, height
	);


	return;
}


/*
 *	Create an image.
 */
int OSWCreateImage(
	image_t **image,
        unsigned int width, unsigned int height
)
{
	u_int8_t *data_ptr;
	osw_gui_struct *gui;


	gui = &osw_gui[0];

	/* Error checks. */
        if(!IDC() ||
           (image == NULL) ||
	   (width == 0) ||
	   (height == 0)
        )
            return(-1);

	/* Reset image pointer. */
	*image = NULL;

        /* Width and height must be even number. */
        if(IS_NUM_ODD(width))
	    width += 1;
        if(IS_NUM_ODD(height))
	    height += 1;


	/* Allocate memory for image. */
	data_ptr = (u_int8_t *)malloc(
	    width * height * gui->z_bytes * sizeof(u_int8_t)
	);
	if(data_ptr == NULL)
	    return(-1);


	/* Create XImage. */
        *image = XCreateImage(
	    gui->display,
	    gui->visual,
	    gui->depth,		/* Operative bit depth. */
	    ZPixmap,		/* Format. */
	    0,			/* X offset. */
            (char *)data_ptr,	/* Actual pointer to data. */
            width, height,	/* Size (in pixels). */
	    gui->actual_depth,	/* Bit padding (actual depth). */
	    0			/* Bytes per line (0 = auto calculated). */
        );

        /* Failed to create image? */
        if(*image == NULL)
	{
            return(-1);
	}
	else
	{
	    /* Reset XImage values. */
	    if((*image)->xoffset != 0)
	    {
                fprintf(stderr,
 "OSWCreateImage(): xoffset %i is not 0, fixing.\n",
                    (int)(*image)->xoffset
                );
	        (*image)->xoffset = 0;
	    }
	    if((int)((*image)->bytes_per_line) != (int)(width * gui->z_bytes))
	    {
		fprintf(stderr,
 "OSWCreateImage(): bytes_per_line %i is not %i, fixing.\n",
		    (int)(*image)->bytes_per_line,
		    (int)(width * gui->z_bytes)
		);
	        (*image)->bytes_per_line = width * gui->z_bytes;
	    }
	}

        return(0);
}



/*
 *      Destroys an image.
 */
void OSWDestroyImage(image_t **image)
{
        if(!IDC() ||
           (image == NULL)
        )
            return;
            
            
        if(*image != NULL)
        {
            XDestroyImage(*image);
            *image = NULL;
        }
         

        return;
}


/*
 *	Create shared memory image.
 */
int OSWCreateSharedImage(
	shared_image_t **image,
        unsigned int width,
        unsigned int height
)
{
#ifdef USE_XSHM
	osw_gui_struct *gui;


	gui = &osw_gui[0];

	if(gui->def_use_shm)
	{
	    int size_in_bytes;
	    image_t *ximage_ptr;
	    XShmSegmentInfo *shm_seg_ptr;
	    Visual *vis_ptr;


	    if(!IDC() ||
               (image == NULL) ||
               (width == 0) ||
               (height == 0)
            )
                return(-1);

	    /* Reset image pointer. */
	    *image = NULL;

	    /* Width and height must be even number. */
	    if(IS_NUM_ODD(width))
	        width += 1;
	    if(IS_NUM_ODD(height))
	        height += 1;

	    /* Get pointer to visual. */
	    vis_ptr = gui->visual;
	    if(vis_ptr == NULL)
		return(-1);


	    /* Allocate shared_image_t structure. */
	    *image = (shared_image_t *)malloc(
	        sizeof(shared_image_t)
	    );
	    if(*image == NULL)
	        return(-1);

	    /* Get pointer to shm segment info. */
	    shm_seg_ptr = &((*image)->shminfo);

	    /* Clear shared memory info. */
	    memset(
	        shm_seg_ptr,
		0x00,
	        sizeof(XShmSegmentInfo)
	    );

            /* Create shared image. */
            ximage_ptr = XShmCreateImage(
		gui->display,
		gui->visual,		/* Visual. */
		gui->depth,		/* Operative bit depth. */
		XShmPixmapFormat(gui->display),	/* Format. */
		NULL,			/* Actual data. */
		shm_seg_ptr,
		width, height		/* Size (in pixels). */
            );
            if(ximage_ptr == NULL)
	    {
	        free(*image);
	        *image = NULL;
                return(-1);
	    }
	    else
	    {
	        (*image)->ximage = ximage_ptr;
	    }

            /* Copy values from ximage_ptr to shared image base struct. */
	    (*image)->in_progress = False;

            (*image)->byte_order = ximage_ptr->byte_order;
            (*image)->xoffset = ximage_ptr->xoffset;
	    if((*image)->xoffset != 0)
	    {
                fprintf(stderr,
 "OSWCreateSharedImage(): X offset of %i is not 0, fixing.\n",
 (*image)->xoffset
                );
                (*image)->xoffset = 0;
                ximage_ptr->xoffset = 0;
            }
            (*image)->format = ximage_ptr->format;
            (*image)->data = (u_int8_t *)ximage_ptr->data;

            (*image)->bytes_per_line = ximage_ptr->bytes_per_line;
	    if((int)((*image)->bytes_per_line) != (int)(width * gui->z_bytes))
	    {
                fprintf(stderr,
 "OSWCreateSharedImage(): bytes_per_line of %i is not %i, fixing.\n",
 (*image)->bytes_per_line,
 (width * gui->z_bytes)
                );
                (*image)->bytes_per_line = width * gui->z_bytes;
                ximage_ptr->bytes_per_line = width * gui->z_bytes;
	    }
            (*image)->bits_per_pixel = ximage_ptr->bits_per_pixel;
            if((*image)->bits_per_pixel != gui->actual_depth)
	    {
                 fprintf(stderr,
 "OSWCreateSharedImage(): bits_per_pixel of %i is not %i, fixing.\n",
 (*image)->bits_per_pixel,
 gui->actual_depth
                );
                (*image)->bits_per_pixel = gui->actual_depth;
                ximage_ptr->bits_per_pixel = gui->actual_depth;
	    }
/*	    (*image)->bitmap_bit_order = ximage_ptr->bitmap_bit_order; */
            (*image)->bitmap_pad = ximage_ptr->bitmap_pad;
	    if((*image)->bitmap_pad != gui->actual_depth)
	    {
/*
                 fprintf(stderr,
 "OSWCreateSharedImage(): bitmap_pad of %i is not %i, fixing.\n",
 (*image)->bitmap_pad,
 gui->actual_depth
                );
                (*image)->bitmap_pad = gui->actual_depth;
                ximage_ptr->bitmap_pad = gui->actual_depth;
 */
	    }
	    (*image)->width = ximage_ptr->width;
            (*image)->height = ximage_ptr->height;
            (*image)->depth = ximage_ptr->depth;
            (*image)->red_mask = ximage_ptr->red_mask;
            (*image)->green_mask = ximage_ptr->green_mask;
            (*image)->blue_mask = ximage_ptr->blue_mask;

	    /* Check for potential incompatable values. */
	    if(((*image)->red_mask != vis_ptr->red_mask) ||
               ((*image)->green_mask != vis_ptr->green_mask) ||
               ((*image)->blue_mask != vis_ptr->blue_mask)
	    )
		fprintf(stderr,
 "OSWCreateSharedImage(): Incompatable plane masks, image: 0x%.8x visual: 0x%.8x\n",
                    (unsigned int)((*image)->red_mask | (*image)->green_mask |
                        (*image)->blue_mask),
		    (unsigned int)(vis_ptr->red_mask | vis_ptr->green_mask |
                        vis_ptr->blue_mask)
		);


	    /* Calculate size of memory for XImage's graphics in bytes. */
	    size_in_bytes = (ximage_ptr->width * ximage_ptr->height *
                gui->z_bytes) + (ximage_ptr->height * sizeof(u_int32_t));

            /* Get shared memory ID. */
            shm_seg_ptr->shmid = shmget(
                IPC_PRIVATE,
		size_in_bytes,		/* Size. */
                IPC_CREAT | 0777
            );
            if(shm_seg_ptr->shmid < 0)
            {
                perror("Getting shared memory ID");
                return(-1);
            }

            /* Attach shared memory and check for errors. */
            shm_seg_ptr->shmaddr = ximage_ptr->data = (char *)shmat(
                shm_seg_ptr->shmid, 0, 0
	    );
            if(shm_seg_ptr->shmaddr == (char *)-1)
            {
                perror("Attaching shared memory ID");
                return(-1);
            }

            /* Set shared memory as read/write. */
            shm_seg_ptr->readOnly = False;

	    /* Attached shared memory to the GUI. */
            XShmAttach(gui->display, shm_seg_ptr);
	    XSync(gui->display, False);

	    /* Need to update pointer to shared image data. */
            (*image)->data = (u_int8_t *)ximage_ptr->data;


	    /* Mark this shm segment for deletion at once. The segment will
	     * automatically become released when both the server and our
	     * program deatches from it.
	     */
	    shmctl(shm_seg_ptr->shmid, IPC_RMID, NULL);
	}
	else
	{
#endif  /* USE_XSHM */
	    image_t *ximage_ptr;


            if(!IDC() ||
               (image == NULL) ||
               (width == 0) ||
               (height == 0)
            )
                return(-1);


            /* Allocate shared_image_t structure. */
            *image = (shared_image_t *)malloc(
                sizeof(shared_image_t)
            );
            if(*image == NULL)
                return(-1);


	    /* Create regular image. */
	    if(OSWCreateImage(&ximage_ptr, width, height))
	    {
	        free(*image);
	        *image = NULL;

	        return(-1);
	    }
            if(ximage_ptr == NULL)
            {
                free(*image);
                *image = NULL;

                return(-1);
            }
	    else
	    {
	        (*image)->ximage = ximage_ptr;
	    }

            /* Copy values from ximage_ptr to shared image struct header. */
            (*image)->byte_order = ximage_ptr->byte_order;
            (*image)->xoffset = ximage_ptr->xoffset;
            (*image)->format = ximage_ptr->format;
            (*image)->data = (u_int8_t *)ximage_ptr->data;

            (*image)->bytes_per_line = ximage_ptr->bytes_per_line;
            (*image)->bits_per_pixel = ximage_ptr->bits_per_pixel;
            (*image)->bitmap_pad = ximage_ptr->bitmap_pad;

            (*image)->width = ximage_ptr->width;
            (*image)->height = ximage_ptr->height;
            (*image)->depth = ximage_ptr->depth;

            (*image)->red_mask = ximage_ptr->red_mask;
            (*image)->green_mask = ximage_ptr->green_mask;
            (*image)->blue_mask = ximage_ptr->blue_mask;
#ifdef USE_XSHM
	}
#endif  /* USE_XSHM */

	return(0);
}



/*
 *	Destroys a shared image.
 */
void OSWDestroySharedImage(
        shared_image_t **image
)
{
#ifdef USE_XSHM
	XShmSegmentInfo *shm_seg_ptr;
#endif	/* USE_XSHM */
	image_t *ximage_ptr;


#ifdef USE_XSHM
        if(osw_gui[0].def_use_shm)
        {
            if(!IDC() ||
               (image == NULL)
            )
                return;
	    if(*image == NULL)
	        return;
	    if((*image)->ximage == NULL)
	        return;

	    /* Get pointers. */
	    shm_seg_ptr = &(*image)->shminfo;
	    ximage_ptr = (*image)->ximage;


            /* Get statistics of shared memory segment. */
/*	    memset(&shm_stats_buf, 0x00, sizeof(struct shmid_ds));
            shmctl(shm_seg_ptr->shmid, IPC_STAT, &shm_stats_buf);
 */

            /* Detach shared memory from GUI. */
            XShmDetach(osw_gui[0].display, shm_seg_ptr);

            /* Destroy the image. */
	    OSWDestroyImage(&ximage_ptr);
	    (*image)->ximage = NULL;

            /* Detach shared memory from our side. */
            shmdt(shm_seg_ptr->shmaddr);

/*	    shm_stats_buf.shm_nattch = 0; */	/* Explicit set # attachs to 0. */

            /* Mark shm segment as destroyed. */
/*
            shmctl(shm_seg_ptr->shmid, IPC_RMID, &shm_stats_buf);
 */

	    /* Deallocate shared image structure. */
	    free(*image);
	    *image = NULL;
	}
	else
	{
#endif	/* USE_XSHM */
            if(!IDC() ||
               (image == NULL)
            )
                return;
            if(*image == NULL)
                return;

  
            /* Get pointer to image. */
            ximage_ptr = (*image)->ximage;

	    /* Destroy the image normally. */
            OSWDestroyImage(&ximage_ptr);


            /* Deallocate shared image structure. */
            free(*image);
            *image = NULL;
#ifdef USE_XSHM
	}
#endif  /* USE_XSHM */


        return;
}


/*
 *	Create a graphics buffer / pixmap from image.
 */
pixmap_t OSWCreatePixmapFromImage(image_t *image)
{
	pixmap_t pixmap;


        /* Error checks. */
        if(!IDC() ||
           (osw_gui[0].root_win == 0)
	)
            return(0);
        if(image == NULL)
            return(0);
        else if(image->data == NULL)
            return(0);


        /* ************************************************************** */

        /* Create the pixmap. */
	if(
	    OSWCreatePixmap(
		&pixmap,
		image->width,
		image->height
	    )
	)
            return(0);


        /* Copy the image to the pixmap. */
	OSWPutImageToDrawable(image, pixmap);


        return(pixmap);
}


/*
 *	Gets an image from drawable d.
 */
image_t *OSWGetImage(
	drawable_t d,
	int x, int y,
	unsigned int width, unsigned int height
)
{
	int norm_bpl;
	image_t *image;
	u_int8_t *data_ptr;
	u_int8_t *sptr8, *tptr8;
        u_int16_t *sptr16, *tptr16;
        u_int32_t *sptr32, *tptr32;
        osw_gui_struct *gui;


	gui = &osw_gui[0];

	if(!IDC() ||
           (d == 0) ||
           (width == 0) ||
           (height == 0)
	)
	    return(NULL);


	image = XGetImage(
            gui->display,
            d,
            x, y,
	    width, height,
            AllPlanes,
            ZPixmap
        );
	if(image != NULL)
	{
            /* Reset XImage values. */
            image->xoffset = 0;

	    /* Shift pixel data incase extra bits are padded
	     * at the end of each line.
	     */
	    data_ptr = (u_int8_t *)image->data;
	    if(((int)(image->bytes_per_line) != (int)(width * gui->z_bytes)) &&
               (data_ptr != NULL)
	    )
	    {
/*
fprintf(stderr, "GetImage has padding: %i  wh: %i %i  bpl: %i\n",
 image->bitmap_pad,
 image->width, image->height,
 image->bytes_per_line
);
 */
		norm_bpl = image->width * gui->z_bytes;

		switch(gui->depth)
		{
		  case 8:
		    for(y = 0; y < image->height; y++)
		    {
			for(x = 0; x < image->width; x++)
			{
			    tptr8 = (u_int8_t *)(&data_ptr[
				(y * norm_bpl) +
				(x * gui->z_bytes)
			    ]);
			    sptr8 = (u_int8_t *)(&data_ptr[
                                (y * image->bytes_per_line) +
                                (x * gui->z_bytes)
                            ]);

			    if(tptr8 != sptr8)
				*tptr8 = *sptr8;
			}
		    }
		    break;

                  case 15:
		  case 16:
                    for(y = 0; y < image->height; y++)
                    {
                        for(x = 0; x < image->width; x++)
                        {
                            tptr16 = (u_int16_t *)(&data_ptr[
                                (y * norm_bpl) +
                                (x * gui->z_bytes)
                            ]);
                            sptr16 = (u_int16_t *)(&data_ptr[
                                (y * image->bytes_per_line) +
                                (x * gui->z_bytes)
                            ]);

                            if(tptr16 != sptr16)
                                *tptr16 = *sptr16;
                        }
                    }
                    break;

                  case 24:
		  case 32:
                    for(y = 0; y < image->height; y++)
                    {
                        for(x = 0; x < image->width; x++)
                        {
                            tptr32 = (u_int32_t *)(&data_ptr[
                                (y * norm_bpl) +
                                (x * gui->z_bytes)
                            ]);
                            sptr32 = (u_int32_t *)(&data_ptr[
                                (y * image->bytes_per_line) +
                                (x * gui->z_bytes)
                            ]);

                            if(tptr32 != sptr32)
                                *tptr32 = *sptr32;
                        }
                    }
                    break;
		}
	    }
	}

	return(image);
}



/*
 *	Puts an image to a drawable.
 */
void OSWPutImageToDrawable(image_t *image, drawable_t d)
{
        if(!IDC() ||
           (image == NULL) ||
           (d == 0)
        )
            return;


        XPutImage(
            osw_gui[0].display,
            d,
            osw_gui[0].gc,
            image,
            0, 0,
            0, 0,
            image->width, image->height
        );


	return;
}



/*
 *	Puts image to drawable at a specific position.
 */
void OSWPutImageToDrawablePos(
	image_t *image, drawable_t d,
        int tar_x, int tar_y
)
{
        if(!IDC() ||
           (image == NULL) ||
           (d == 0)
        )
            return;


        XPutImage(
            osw_gui[0].display,
            d,
            osw_gui[0].gc,
            image,
            0, 0,
            tar_x, tar_y,
            image->width, image->height
        );


        return;
}


/*
 *	Puts section of image to drawable at a specific position.
 */
void OSWPutImageToDrawableSect(
        image_t *image, drawable_t d,  
        int tar_x, int tar_y,
	int src_x, int src_y,
	unsigned int width, unsigned int height
)
{         
        if(!IDC() ||
           (image == NULL) ||
           (d == 0)
        )
            return;
 
   
        XPutImage(
            osw_gui[0].display,
            d,
            osw_gui[0].gc,
            image,
            src_x, src_y,
            tar_x, tar_y,
            width, height
        );


        return;
}  



/*
 *	Puts a shared image to a drawable.
 *
 *	If USE_XSHM is not defined then this function calls
 *	OSWPutImageToDrawable()
 */
void OSWPutSharedImageToDrawable(shared_image_t *image, drawable_t d)
{
#ifdef USE_XSHM
        event_t xevent;
	osw_gui_struct *gui;


	gui = &osw_gui[0];
        if(gui->def_use_shm)
	{
	    if((gui->display == NULL) ||
               (image == NULL) ||
               (d == 0)
	    )
	        return;

	    if(image->ximage == NULL)
	        return;

            if(gui->def_sync_shm)
            {
		static_osw_x::gui_error_code = 0;     /* Reset error code. */
                XSetErrorHandler(OSWGUISimpleErrorHandler);

                /* Put and sync. */
                XShmPutImage(
                    gui->display,
                    d,
                    gui->gc,
                    image->ximage,
                    0, 0,
                    0, 0,
                    image->ximage->width, image->ximage->height,
                    True
                );

                /* Wait untill image is completly put. */
                while(!static_osw_x::gui_error_code)
		{
		    if(XCheckTypedWindowEvent(
                        gui->display,
			d,
                        gui->shm_completion_event_code,
                        &xevent
                    ))
		        break;
                }
		XSetErrorHandler(OSWGUIErrorHandler);
            }
            else
            {
                XShmPutImage(
                    gui->display,
                    d,
                    gui->gc,
                    image->ximage,
                    0, 0,
                    0, 0,
                    image->ximage->width, image->ximage->height,
                    False
                );
	    }
	}
	else
	{
#endif	/* USE_XSHM */
            if(image == NULL)
                return;

	    OSWPutImageToDrawable(image->ximage, d);
#ifdef USE_XSHM
	}
#endif /* USE_XSHM */

	return;
}


/*
 *      Puts image to drawable at a specific position.
 *
 *      If USE_XSHM is not defined then this function calls
 *      OSWPutImageToDrawablePos()
 */
void OSWPutSharedImageToDrawablePos(
	shared_image_t *image, drawable_t d,
	int tar_x, int tar_y
)
{
#ifdef USE_XSHM
        event_t xevent;
	osw_gui_struct *gui;


	gui = &osw_gui[0];
        if(gui->def_use_shm)
        { 
            if((gui->display == NULL) ||
               (image == NULL) ||
               (d == 0)
            )
                return;

            if(image->ximage == NULL)
                return;

            if(gui->def_sync_shm)
            {
                static_osw_x::gui_error_code = 0;     /* Reset error code. */
                XSetErrorHandler(OSWGUISimpleErrorHandler);

                /* Put and sync. */
                XShmPutImage(
                    gui->display,
                    d,
                    gui->gc,
                    image->ximage,
                    0, 0,   
                    tar_x, tar_y,
                    image->ximage->width, image->ximage->height,
                    True
                );

                /* Wait untill image is completly put. */
                while(!static_osw_x::gui_error_code)
                {   
                    if(XCheckTypedWindowEvent(
                        gui->display,
                        d,
                        gui->shm_completion_event_code,
                        &xevent
                    ))         
                        break;
                }
                XSetErrorHandler(OSWGUIErrorHandler);
	    }
	    else
	    {
                XShmPutImage(
                    gui->display,
                    d,
                    gui->gc,
                    image->ximage,
                    0, 0,
                    tar_x, tar_y,
                    image->ximage->width, image->ximage->height,
                    False
                );
	    }
	}
	else
	{
#endif	/* USE_XSHM */
            if(image == NULL)   
                return;

            OSWPutImageToDrawablePos(image->ximage, d, tar_x, tar_y);
#ifdef USE_XSHM
	}
#endif /* USE_XSHM */

	return;
}


/*
 *      Puts section of image to drawable at a specific position.
 *
 *      If USE_XSHM is not defined then this function calls
 *      OSWPutImageToDrawableSect()
 */
void OSWPutSharedImageToDrawableSect(
        shared_image_t *image, drawable_t d,
        int tar_x, int tar_y,
        int src_x, int src_y,
        unsigned int width, unsigned int height
)       
{
#ifdef USE_XSHM
        event_t xevent;
	osw_gui_struct *gui;


	gui = &osw_gui[0];
        if(gui->def_use_shm)
        {
            if((gui->display == NULL) ||
               (image == NULL) ||
               (d == 0)
            )
	        return;
            if(image->ximage == NULL)
                return;

	    if(gui->def_sync_shm)
	    {
                static_osw_x::gui_error_code = 0;     /* Reset error code. */
                XSetErrorHandler(OSWGUISimpleErrorHandler);

		/* Put and sync. */
                XShmPutImage(
                    gui->display,
                    d,
                    gui->gc,
                    image->ximage,
                    src_x, src_y,
                    tar_x, tar_y,
                    width, height,   
                    True
                );

                /* Wait untill image is completly put. */
                while(!static_osw_x::gui_error_code)
                {
                    if(XCheckTypedWindowEvent(
                        gui->display,
                        d,
                        gui->shm_completion_event_code,
                        &xevent
                    ))
                        break;
                }
                XSetErrorHandler(OSWGUIErrorHandler);
	    }
	    else
	    {
		/* Just put, no sync. */
                XShmPutImage(
                    gui->display,
                    d,
                    gui->gc,
                    image->ximage,
                    src_x, src_y,
                    tar_x, tar_y,
                    width, height,
                    False
                );
	    }
	}
	else
	{  
#endif	/* USE_XSHM */
            if(image == NULL)
                return; 

            OSWPutImageToDrawableSect(
	        image->ximage, d,
	        tar_x, tar_y,
	        src_x, src_y,
	        width, height
	    );
#ifdef USE_XSHM
	}
#endif /* USE_XSHM */

        return;
}


/*
 *	Syncronizes shared image with current thread of execution.
 *
 *	A short delay may occure due to the time needed to sync.
 */
void OSWSyncSharedImage(shared_image_t *image, drawable_t d)
{
#ifdef USE_XSHM
        event_t xevent;
	osw_gui_struct *gui;


	gui = &osw_gui[0];
        if(gui->def_use_shm)
	{
            if((gui->display == NULL) ||
               (image == NULL) ||
               (d == 0)
            )
                return;
	    if(image->ximage == NULL)
	        return;


            static_osw_x::gui_error_code = 0;     /* Reset error code. */
            XSetErrorHandler(OSWGUISimpleErrorHandler);

	    /* Put shared image to drawable (entire size). */
            XShmPutImage(
                gui->display,
                d,
                gui->gc,
                image->ximage,
                0, 0,
                0, 0,
                image->ximage->width,
                image->ximage->height,
                True
            );

            /* Wait untill image is completly put. */
            while(!static_osw_x::gui_error_code)
            {
                if(XCheckTypedWindowEvent(
                    gui->display,
                    d,
                    gui->shm_completion_event_code,
                    &xevent
                ))
                    break;
            }
            XSetErrorHandler(OSWGUIErrorHandler);
	}
#endif /* USE_XSHM */

	return;
}


/*
 *	Puts a graphics buffer (pixmap_t) to window.
 *	Both the graphics buffer and the window should be of the
 *	same size.
 */
void OSWPutBufferToWindow(win_t w, gbuf_t gbuf)
{
	static win_attr_t wattr;


	if(!IDC() ||
           (w == 0) ||
           (gbuf == 0)
        )
	    return;


	OSWGetWindowAttributes(w, &wattr);
/*
	OSWSetWindowBkg(w, 0, (pixmap_t)gbuf);
 */

	OSWCopyDrawables(
	    (drawable_t)w,
	    (drawable_t)gbuf,
	    wattr.width,
	    wattr.height
	);


	return;
}


/*
 *	Copy drawables.
 */
void OSWCopyDrawables(
	drawable_t tar_d,  
        drawable_t src_d,
	unsigned int width,
	unsigned int height
)
{
	if(!IDC() ||
           (tar_d == 0) ||
           (src_d == 0) ||
           (width == 0) ||
           (height == 0)
	)
	    return;


        XCopyArea(
	    osw_gui[0].display,
	    src_d, tar_d,
            osw_gui[0].gc,
	    0, 0, width, height, 0, 0
	);


	return;
}




/*
 *	Copy drawables with coordinates.
 */
void OSWCopyDrawablesCoord(
	drawable_t tar_d,
        drawable_t src_d,
        int tar_x, int tar_y,
        unsigned int width, unsigned int height,
        int src_x, int src_y
)
{
        if(!IDC() ||
           (tar_d == 0) ||
           (src_d == 0) ||
           (width == 0) ||
           (height == 0)
        )
            return;


        XCopyArea(
	    osw_gui[0].display,
	    src_d, tar_d,
	    osw_gui[0].gc,
            src_x, src_y,
	    width, height,
	    tar_x, tar_y
        );
        
        return;
}



/*
 *	Draw string.
 */
void OSWDrawString(drawable_t d, int x, int y, const char *string)
{
	if(!IDC() ||
           (d == 0) ||
           (string == NULL)
	)
	    return;

	XDrawString(
	    osw_gui[0].display, d, osw_gui[0].gc,
	    x, y, string, strlen(string)
	);
}



/*
 *      Draw string limited.
 */
void OSWDrawStringLimited(drawable_t d, int x, int y, const char *string, int len)
{
	int real_len;


        if(!IDC() ||
           (d == 0) ||
           (string == NULL) ||
           (len < 1)
        )
            return;

	real_len = strlen(string);
	if(len > real_len)
	    len = real_len;
 
        XDrawString(
	    osw_gui[0].display, d, osw_gui[0].gc,
            x, y, string, len
        );

        return;
}



/*
 *	Draw line.
 */
void OSWDrawLine(drawable_t d, int start_x, int start_y,
	int end_x, int end_y)
{
        if(!IDC() ||
           (d == 0)
        )
            return;

	XDrawLine(
	    osw_gui[0].display, d, osw_gui[0].gc,
	    start_x, start_y,
	    end_x, end_y
	);

	return;
}


/*
 *	Draw rectangle.
 */
void OSWDrawRectangle(drawable_t d, int x, int y,
	unsigned int width, unsigned int height)
{
        if(!IDC() ||
           (d == 0) ||
           (width == 0) ||
	   (height == 0)
        ) 
            return;

	XDrawRectangle(
	    osw_gui[0].display, d, osw_gui[0].gc,
	    x, y, width, height
	);

	return;
}



/*
 *	Draw solid rectangle.
 */
void OSWDrawSolidRectangle(drawable_t d, int x, int y,
	unsigned int width, unsigned int height)
{
        if(!IDC() ||
           (d == 0) ||
           (width == 0) ||
           (height == 0)
        )
            return;

	XFillRectangle(
	    osw_gui[0].display, d, osw_gui[0].gc,
            x, y, width, height
	);

	return;
}



/*
 *	Draw arc, angles are in radians.
 */
void OSWDrawArc(drawable_t d, int x, int y,
	unsigned int width, unsigned int height,
        double position_angle,
        double terminal_angle
)
{
        if(!IDC() ||
           (d == 0) ||
           (width == 0) ||
           (height == 0)
        )
            return;

	XDrawArc(
	    osw_gui[0].display, d, osw_gui[0].gc,
	    x, y,
	    width, height,
	    static_cast<int>(position_angle * 180 / PI * 64),
	    static_cast<int>(terminal_angle * 180 / PI * 64)
	);

	return;
}



/* 
 *	Draw solid arc, angles are in radians.
 */
void OSWDrawSolidArc(drawable_t d, int x, int y,
	unsigned int width, unsigned int height,
        double position_angle,
        double terminal_angle
)
{
        if(!IDC() ||
           (d == 0) ||   
           (width == 0) ||
           (height == 0)
        ) 
            return;

        XFillArc(
	    osw_gui[0].display, d, osw_gui[0].gc,
            x, y,
            width, height,
            static_cast<unsigned int>(position_angle * 180 / PI * 64),
            static_cast<unsigned int>(terminal_angle * 180 / PI * 64)
        );
 
        return;
}



/*
 *      GUI Error handler, prints GUI error to stdout and
 *	sets variable static_osw_x::gui_error_code to 0 if there
 *      was no error or sets to non-zero to indicate error.
 */
int OSWGUIErrorHandler(
	Display *dpy,
	XErrorEvent *error_event
)
{
	if((dpy == NULL) ||
	   (error_event == NULL)
	)
	    return(0);


	/* Ignore success error code. */
	if(error_event->error_code == Success)
	    return(0);


	/* ******************************************************** */
	/* Ignore these errors. */

	/* Failed GetImage. */
	if(error_event->request_code == X_GetImage)
	    return(0);


        /* ******************************************************** */
        /* Set error code. */
        static_osw_x::gui_error_code = 1;


	/* ********************************************************* */
	/* Print error. */

	fprintf(stderr,
 "\nSubsystems message: X server error:\n\n"
	);

	/* Error type. */
	fprintf(stderr,
 "        Error type: %i\n", error_event->type
	);

	/* Display. */
        fprintf(stderr,
 "        X server display connection pointer: 0x%.8x\n",
	    (u_int32_t)error_event->display
        );

	/* Serial number. */
        fprintf(stderr,
 "        Serial number of failed request: %i\n",
	    (int32_t)error_event->serial
        );

	/* Print error code. */
        fprintf(stderr,
 "        Error code of failed request: %i (",
           (int32_t)error_event->error_code
        );
	switch(error_event->error_code)
	{
	  case BadRequest:
	    fprintf(stderr, "BadRequest");
	    break;

	  case BadValue:
            fprintf(stderr, "BadValue");
            break;

          case BadWindow:
            fprintf(stderr, "BadWindow");
            break;

          case BadPixmap:
            fprintf(stderr, "BadPixmap");
            break;

          case BadAtom:
            fprintf(stderr, "BadAtom");
            break;

          case BadCursor:
            fprintf(stderr, "BadCursor");
            break;

          case BadFont:
            fprintf(stderr, "BadFont");
            break;

          case BadMatch:
            fprintf(stderr, "BadMatch");
            break;

          case BadDrawable:
            fprintf(stderr, "BadDrawable"); 
            break;

          case BadAccess:
            fprintf(stderr, "BadAccess");
            break;

          case BadAlloc:
            fprintf(stderr, "BadAlloc");
            break;

          case BadColor:
            fprintf(stderr, "BadColor");
            break;

          case BadGC:
            fprintf(stderr, "BadGC");
            break;

          case BadIDChoice:
            fprintf(stderr, "BadIDChoice");
            break;

          case BadName:
            fprintf(stderr, "BadName");
            break;

          case BadLength:
            fprintf(stderr, "BadLength");
            break;

          case BadImplementation:
            fprintf(stderr, "BadImplementation");
            break;

	  default:
            fprintf(stderr, "Unknown");
            break;
	}
	fprintf(stderr, ")\n");

	/* Major and minor operation code. */
        fprintf(stderr,
 "        Major op-code of failed request: %i (", error_event->request_code
        );
	switch(error_event->request_code)
	{
	  case X_CreateWindow:
	    fprintf(stderr, "CreateWindow");
	    break;

	  case X_ChangeWindowAttributes:
            fprintf(stderr, "ChangeWindowAttributes");
            break;

          case X_GetWindowAttributes:
            fprintf(stderr, "GetWindowAttributes");
            break;

          case X_DestroyWindow:
            fprintf(stderr, "DestroyWindow");
            break;

          case X_DestroySubwindows:
            fprintf(stderr, "DestroySubwindows");
            break;

          case X_ChangeSaveSet:
            fprintf(stderr, "ChangeSaveSet");
            break;

          case X_ReparentWindow:
            fprintf(stderr, "ReparentWindow");
            break;

          case X_MapWindow:
            fprintf(stderr, "MapWindow");
            break;

          case X_MapSubwindows:
            fprintf(stderr, "MapSubwindows");
            break;

          case X_UnmapWindow:
            fprintf(stderr, "UnmapWindow");
            break;

          case X_UnmapSubwindows:
            fprintf(stderr, "UnmapSubwindows");
            break;

          case X_ConfigureWindow:
            fprintf(stderr, "ConfigureWindow");
            break;

          case X_CirculateWindow:
            fprintf(stderr, "CirculateWindow");
            break;

          case X_GetGeometry:
            fprintf(stderr, "GetGeometry");
            break;

          case X_QueryTree:
            fprintf(stderr, "QueryTree");
            break;

          case X_InternAtom:
            fprintf(stderr, "InternAtom");
            break;

          case X_GetAtomName:
            fprintf(stderr, "GetAtomName");
            break;

          case X_ChangeProperty:
            fprintf(stderr, "ChangeProperty");
            break;

          case X_DeleteProperty:
            fprintf(stderr, "DeleteProperty");
            break;

          case X_GetProperty:
            fprintf(stderr, "GetProperty");
            break;

          case X_ListProperties:
            fprintf(stderr, "ListProperties");
            break;

          case X_SetSelectionOwner:
            fprintf(stderr, "SetSelectionOwner");
            break;

          case X_GetSelectionOwner:
            fprintf(stderr, "GetSelectionOwner");
            break;

          case X_ConvertSelection:
            fprintf(stderr, "ConvertSelection");
            break;

          case X_SendEvent:
            fprintf(stderr, "SendEvent");
            break;

          case X_GrabPointer:
            fprintf(stderr, "GrabPointer");
            break;

          case X_UngrabPointer:
            fprintf(stderr, "UngrabPointer");
            break;

          case X_GrabButton:
            fprintf(stderr, "GrabButton");
            break;

          case X_ChangeActivePointerGrab:
            fprintf(stderr, "ChangeActivePointerGrab");
            break;

          case X_GrabKeyboard:
            fprintf(stderr, "GrabKeyboard");
            break;

          case X_UngrabKeyboard:
            fprintf(stderr, "UngrabKeyboard");
            break;

          case X_GrabKey:
            fprintf(stderr, "GrabKey");
            break;

          case X_UngrabKey:
            fprintf(stderr, "UngrabKey");
            break;

          case X_AllowEvents:
            fprintf(stderr, "AllowEvents");
            break;

          case X_GrabServer:
            fprintf(stderr, "GrabServer");
            break;

          case X_UngrabServer:
            fprintf(stderr, "UngrabServer");
            break;

          case X_QueryPointer:
            fprintf(stderr, "QueryPointer");
            break;

          case X_GetMotionEvents:
            fprintf(stderr, "GetMotionEvents");
            break;

          case X_TranslateCoords:
            fprintf(stderr, "TranslateCoords");
            break;

          case X_WarpPointer:
            fprintf(stderr, "WarpPointer");
            break;

          case X_SetInputFocus:
            fprintf(stderr, "SetInputFocus");
            break;

          case X_GetInputFocus:
            fprintf(stderr, "GetInputFocus");
            break;

          case X_QueryKeymap:
            fprintf(stderr, "QueryKeymap");
            break;

          case X_OpenFont:
            fprintf(stderr, "OpenFont");
            break;

          case X_CloseFont:
            fprintf(stderr, "CloseFont");
            break;

          case X_QueryFont:
            fprintf(stderr, "QueryFont");
            break;

          case X_QueryTextExtents:
            fprintf(stderr, "QueryTextExtents");
            break;

          case X_ListFonts:
            fprintf(stderr, "ListFonts");
            break;

          case X_ListFontsWithInfo:
            fprintf(stderr, "ListFontsWithInfo");
            break;

          case X_SetFontPath:
            fprintf(stderr, "SetFontPath");
            break;

          case X_GetFontPath:
            fprintf(stderr, "GetFontPath");
            break;

          case X_CreatePixmap:
            fprintf(stderr, "CreatePixmap");
            break;

          case X_FreePixmap:
            fprintf(stderr, "FreePixmap");
            break;

          case X_CreateGC:
            fprintf(stderr, "CreateGC");
            break;

          case X_ChangeGC:
            fprintf(stderr, "ChangeGC");
            break;

          case X_CopyGC:
            fprintf(stderr, "CopyGC");
            break;

          case X_SetDashes:
            fprintf(stderr, "SetDashes");
            break;

          case X_SetClipRectangles:
            fprintf(stderr, "SetClipRectangles");
            break;

          case X_FreeGC:
            fprintf(stderr, "FreeGC");
            break;

          case X_ClearArea:
            fprintf(stderr, "ClearArea");
            break;

          case X_CopyArea:
            fprintf(stderr, "CopyArea");
            break;

          case X_CopyPlane:
            fprintf(stderr, "CopyPlane");
            break;

          case X_PolyPoint:
            fprintf(stderr, "PolyPoint");
            break;

          case X_PolyLine:
            fprintf(stderr, "PolyLine");
            break;

          case X_PolySegment:
            fprintf(stderr, "PolySegment");
            break;

          case X_PolyRectangle:
            fprintf(stderr, "PolyRectangle");
            break;

          case X_PolyArc:
            fprintf(stderr, "PolyArc");
            break;

          case X_FillPoly:
            fprintf(stderr, "FillPoly");
            break;

          case X_PolyFillRectangle:
            fprintf(stderr, "PolyFillRectangle");
            break;

          case X_PolyFillArc:
            fprintf(stderr, "PolyFillArc");
            break;

          case X_PutImage:
            fprintf(stderr, "PutImage");
            break;

          case X_GetImage:
            fprintf(stderr, "GetImage");
            break;

          case X_PolyText8:
            fprintf(stderr, "PolyText8");
            break;

          case X_PolyText16:
            fprintf(stderr, "PolyText16");
            break;

          case X_ImageText8:
            fprintf(stderr, "ImageText8");
            break;

          case X_ImageText16:
            fprintf(stderr, "ImageText16");
            break;

          case X_CreateColormap:
            fprintf(stderr, "CreateColormap");
            break;

          case X_FreeColormap:
            fprintf(stderr, "FreeColormap");
            break;

          case X_CopyColormapAndFree:
            fprintf(stderr, "CopyColormapAndFree");
            break;

          case X_InstallColormap:
            fprintf(stderr, "InstallColormap");
            break;

          case X_UninstallColormap:
            fprintf(stderr, "UninstallColormap");
            break;

          case X_ListInstalledColormaps:
            fprintf(stderr, "ListInstalledColormaps");
            break;

          case X_AllocColor:
            fprintf(stderr, "AllocColor");
            break;

          case X_AllocNamedColor:
            fprintf(stderr, "AllocNamedColor");
            break;

          case X_AllocColorCells:
            fprintf(stderr, "AllocColorCells");
            break;

          case X_AllocColorPlanes:
            fprintf(stderr, "AllocColorPlanes");
            break;

          case X_FreeColors:
            fprintf(stderr, "FreeColors");
            break;

          case X_StoreColors:
            fprintf(stderr, "StoreColors");
            break;

          case X_StoreNamedColor:
            fprintf(stderr, "StoreNamedColor");
            break;

          case X_QueryColors:
            fprintf(stderr, "QueryColors");
            break;

          case X_LookupColor:
            fprintf(stderr, "LookupColor");
            break;

          case X_CreateCursor:
            fprintf(stderr, "CreateCursor");
            break;

          case X_CreateGlyphCursor:
            fprintf(stderr, "CreateGlyphCursor");
            break;

          case X_FreeCursor:
            fprintf(stderr, "FreeCursor");
            break;

          case X_RecolorCursor:
            fprintf(stderr, "RecolorCursor");
            break;

          case X_QueryBestSize:
            fprintf(stderr, "QueryBestSize");
            break;

          case X_QueryExtension:
            fprintf(stderr, "QueryExtension");
            break;

          case X_ListExtensions:
            fprintf(stderr, "ListExtensions");
            break;

          case X_ChangeKeyboardMapping:
            fprintf(stderr, "ChangeKeyboardMapping");
            break;

          case X_GetKeyboardMapping:
            fprintf(stderr, "GetKeyboardMapping");
            break;

          case X_ChangeKeyboardControl:
            fprintf(stderr, "ChangeKeyboardControl");
            break;

          case X_GetKeyboardControl:
            fprintf(stderr, "GetKeyboardControl");
            break;

          case X_Bell:
            fprintf(stderr, "Bell");
            break;

          case X_ChangePointerControl:
            fprintf(stderr, "ChangePointerControl");
            break;

          case X_GetPointerControl:
            fprintf(stderr, "GetPointerControl");
            break;

          case X_SetScreenSaver:
            fprintf(stderr, "SetScreenSaver");
            break;

          case X_GetScreenSaver:
            fprintf(stderr, "GetScreenSaver");
            break;

          case X_ChangeHosts:
            fprintf(stderr, "ChangeHosts");
            break;

          case X_ListHosts:
            fprintf(stderr, "ListHosts");
            break;

          case X_SetAccessControl:
            fprintf(stderr, "SetAccessControl");
            break;

          case X_SetCloseDownMode:
            fprintf(stderr, "SetCloseDownMode");
            break;

          case X_KillClient:
            fprintf(stderr, "KillClient");
            break;

          case X_RotateProperties:
            fprintf(stderr, "RotateProperties");
            break;

          case X_ForceScreenSaver:
            fprintf(stderr, "ForceScreenSaver");
            break;

          case X_SetPointerMapping:
            fprintf(stderr, "SetPointerMapping");
            break;

          case X_GetPointerMapping:
            fprintf(stderr, "GetPointerMapping");
            break;

          case X_SetModifierMapping:
            fprintf(stderr, "SetModifierMapping");
            break;

          case X_GetModifierMapping:
            fprintf(stderr, "GetModifierMapping");
            break;

          case X_NoOperation:
            fprintf(stderr, "NoOperation");
            break;

	  default:	/* Assume unsupported and treat as extended. */
            fprintf(stderr, "Unsupported or Extended");
	    break;
	}
	fprintf(stderr, ")\n");


        fprintf(stderr,
 "        Minor op-code of failed request: %i\n",
	    error_event->minor_code
        );

	/* Resource ID. */
        fprintf(stderr,
 "        Resource ID: %i\n",
	    (int32_t)error_event->resourceid
        );



	return(0);
}

/*
 *	GUI Error handler for quick and simple error detection.
 *	Sets local variable static_osw_x::gui_error_code to 0 if there
 *	was no error or sets to non-zero to indicate error.
 */
int OSWGUISimpleErrorHandler(Display *dpy, XErrorEvent *error_event)
{
        if((dpy == NULL) ||
           (error_event == NULL)
        ) 
            return(0);


        /* Ignore success error code. */
        if(error_event->error_code == Success)
            return(0);


        /* ******************************************************** */
        /* Ignore these errors. */

        /* Failed GetImage. */
        if(error_event->request_code == X_GetImage)
            return(0);


        /* ******************************************************** */
        /* Set error code. */
	static_osw_x::gui_error_code = 1;

	return(0);
}


