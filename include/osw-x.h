// include/osw-x.h

/*
                Operating System Wrapper for

                      X Window Systems


	Functions here are incomplete, working on it.

	                             --WolfPack

 */

#ifndef OSW_X_H
#define OSW_X_H

#include <sys/types.h>

/* Shared memory. */
#ifdef USE_XSHM
# include <sys/shm.h>
# include <sys/ipc.h>
#endif /* USE_XSHM */


/* X */
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <X11/cursorfont.h>
#include <X11/Xproto.h>
#include <X11/Xatom.h>

/* Xext (X Extensions) */
#include <X11/extensions/shape.h>

/* MIT Shared Memory Extension. */
#ifdef USE_XSHM
# include <X11/extensions/XShm.h>
#endif /* USE_XSHM */

/* Catch any undefined stuff (mostly for Solaris). */
#include "os.h"



/*
 *    PI Constants:
 *
 *	Needed for angle calculations in arc draw functions.
 */
#ifndef PI
# define PI	3.141592654
#endif


/*
 *    Event mask presets:
 *
 *	Default values of masks indicating the events to be
 *	recieved in calls to OSWSetWindowInput().
 */
#define OSW_EVENTMASK_TOPLEVEL	ExposureMask        | ButtonPressMask | \
                                ButtonReleaseMask   | KeyPressMask    | \
                                KeyReleaseMask      | FocusChangeMask | \
				StructureNotifyMask | VisibilityChangeMask

#define OSW_EVENTMASK_STD	ExposureMask      | ButtonPressMask | \
                                ButtonReleaseMask | KeyPressMask    | \
                                KeyReleaseMask

#define OSW_EVENTMASK_BUTTON	ExposureMask      | ButtonPressMask | \
                                ButtonReleaseMask

#define OSW_EVENTMASK_MOVEABLE	ExposureMask      | ButtonPressMask | \
                                ButtonReleaseMask | PointerMotionMask


/*
 *    Syncronization interval (in milliseconds):
 */
#define OSW_SYNC_SLEEP_INTERVAL		1


/*
 *    Window style types:
 *
 *	For OSWSetWindowWMProperties().
 *
 *	X does not have these defined nor supports window frame
 *	control. OSWSetWindowWMProperties() will need to play
 *	certain 'tricks' with the window manager to get the proper
 *	frame style.
 */
#define WindowFrameStyleStandard	0
#define WindowFrameStyleNaked		1
#define WindowFrameStyleFixed		2
#define WindowFrameStyleFixedFrameOnly	3
#define WindowFrameStyleNoClose		4	/* Dosen't always work. */




/* *****************************************************************
 *
 *                        OSW Structures
 */

/*
 *    Font structure:
 */
typedef struct {

	/* Can be 0 for non fixed sized fonts. */
	unsigned int char_width, char_height;

	/* Total number of different characters. */
	int total_chars;

	/* Actual GUI's font type structure (private). */
	XFontStruct *actual;

} OSWFontStruct;


/*
 *    Shared image structure:
 */
typedef struct {

	/* Header info. */
	char in_progress;	/* True if currently being `put'. */

	int byte_order;		/* LSBFirst or MSBFirst. */
	int xoffset;		/* Num of pixels offset in X direction. */
	int format;		/* XYBitmap, XYPixmap, or ZPixmap. */
	u_int8_t *data;		/* Pointer to image data. */

	int bytes_per_line;	/* Accelarator to next line. */
	int bits_per_pixel;	/* Bits per pixel (for ZPixmap). */
	int bitmap_pad;		/* 8, 16, 32 either XY or ZPixmap */

	unsigned int width, height;
	unsigned int depth;	/* Depth of image in bits. */

	unsigned long red_mask;		/* Bits in Z arrangment. */
	unsigned long green_mask;
	unsigned long blue_mask;


	/* Actual ximage, native to GUI (PRIVATE!). */
	XImage *ximage;

	/* Shared memory info, native to GUI (PRIVATE!). */
#ifdef USE_XSHM
	XShmSegmentInfo	shminfo;
#else
	char		shminfo;
#endif

} OSWSharedImageStruct;
  



/* *****************************************************************
 *
 *                         Data Types
 */

/*
 *    Atom type:
 */
#define atom_t		Atom

/*
 *    Boolean type:
 */
#define bool_t		Bool

/*
 *    Color map type:
 */
#define colormap_t	Colormap

/*
 *    Cursor type:
 */
#define cursor_t	Cursor

/*
 *    Cursor code type:
 */
#define cur_code_t	unsigned int

/*
 *	Depth type:
 */
#define depth_t		int

/*
 *    Display pointer type:
 */
#define display_t	Display

/*
 *    Drawable type (win_t and pixmap_t):
 */
#define drawable_t	Drawable

/*
 *    Key event type:
 */
#define key_event_t	XKeyEvent

/*
 *    Event type:
 */
#define event_t		XEvent

/*
 *    Event mask type:
 */
#define eventmask_t	long

/*
 *    Event type type:
 */
#define eventtype_t	int

/*
 *    Font structure type:
 */
#define font_t		OSWFontStruct

/*
 *    Graphics buffer type (same as pixmap_t):
 */
#define gbuf_t		Pixmap

/*
 *    Graphics context type:
 */
#define gc_t		GC

/*
 *    Graphics context value type:
 */
#define gc_val_t	XGCValues

/*
 *    Image type:
 */
#define image_t		XImage

/*
 *    Key code (actual hardware code) type:
 */
#define keycode_t	unsigned int

/*
 *    Key sym code (portable key defination code) type:
 */
#define keysym_t	KeySym

/*
 *    Pixel type:
 */
#define pixel_t		unsigned long  

/*
 *    Pixmap type:
 */
#define pixmap_t	Pixmap

/*
 *    Screen number type:
 */
#define screen_num_t	int

/*
 *    Screen pointer type:
 */
#define screen_ptr_t	Screen

/*
 *    Shared image type
 */
#define shared_image_t	OSWSharedImageStruct

/* 
 *    Size hints structure type:
 */
#define sizehints_t	XSizeHints

/*
 *    Visiblity code type:
 */
#define visibility_t	int

/*
 *    Visual type:
 */
#define visual_t	Visual

/*
 *    Visual info type:
 */
#define visual_info_t	XVisualInfo

/*
 *    Visual info mask type:
 */
#define visual_info_mask_t	long

/*
 *    Visual ID code type:
 */
#define visual_id_t	VisualID   

/*
 *    Window type:
 */
#define win_t		Window

/*
 *    Window attributes structure type:
 */
#define win_attr_t	XWindowAttributes




/*
 *	Pixmap attributes structure:
 */
typedef struct {

        win_t root_win;

        unsigned int width, height;
	unsigned int border_width;

	depth_t depth;		/* Operative depth. */

} pixmap_attr_t;


#ifdef OSW_USE_NATIVE_EVENT_TYPE
/*
 *      Native event type, under construction, do NOT compile in!
 */

typedef struct {

	int type;
	unsigned long serial;	/* # of last request processed by server */
	bool_t send_event;	/* true if this came from a SendEvent */

	display_t *display;	/* Display the event was read from */
	win_t window;

} any_event_t

typedef struct {

	int type;		/* KeyPress or KeyRelease */
	unsigned long serial;	/* # of last request processed by server */
	bool_t send_event;	/* True if this came from a SendEvent request */
	display_t *display;	/* Display the event was read from */
        win_t window;		/* Event window it is reported relative to */
	win_t root;		/* Root window that the event occurred on */
	win_t subwindow;	/* child window */

	Time time;		/* In milliseconds */
	int x, y;		/* Pointer x, y coordinates in event window */
	int x_root, y_root;	/* Coordinates relative to root */
	unsigned int state;	/* Key or button mask */
	unsigned int keycode;	/* Key code of the key in question. */
	bool_t same_screen;	/* Same screen flag. */
} key_event_t;


typedef struct {
	int type;		/* ButtonPress or ButtonRelease */
	unsigned long serial;	/* # of last request processed by server */
	bool_t send_event;	/* true if this came from a SendEvent request */
	display_t *display;	/* Display the event was read from */
	win_t window;		/* ``event'' window it is reported relative to */
	win_t root;		/* root window that the event occurred on */
	win_t subwindow;	/* child window */

	Time time;		/* In milliseconds */
	int x, y;		/* pointer x, y coordinates in event window */
	int x_root, y_root;	/* coordinates relative to root */
	unsigned int state;	/* key or button mask */
	unsigned int button;	/* detail */
	bool_t same_screen;	/* same screen flag */
} button_event_t;






typedef struct {

	any_event_t	any;
	key_event_t	key;
	button_event_t	button;

} event_t;

#endif



/*
 *	GUI Resources Structure:
 *
 *	Contains values for the connection and settings of
 *	the GUI.  osw_gui is primarly for internal use.
 */
typedef struct {

	/* Default pre allocated resources by OSW (read only). */
	font_t			*std_font,
				*bold_font;

	unsigned int		display_width, display_height;
	pixel_t			black_pix, white_pix;

	cursor_t		std_cursor;
	win_t			root_win;	/* Aka desktop. */

        u_int64_t		red_mask,
				green_mask,
				blue_mask;


	/* Currently set values (read only). */
	font_t			*current_font;
	pixel_t			current_fg_pix,
				current_bg_pix,
				current_alpha_pix;

	/* Modifier key states. */
	bool_t			alt_key_state,
				ctrl_key_state,
				shift_key_state;

	bool_t			caps_lock_on,
				num_lock_on,
				scroll_lock_on;

	/* Default (not standard) values. These values are
	 * fetched from the command line arguments.
	 */
	font_t	*def_font;
	pixel_t def_bg_pix, def_fg_pix;
	int def_toplevel_x, def_toplevel_y;
	unsigned int def_toplevel_width, def_toplevel_height;
	bool_t def_geometry_set;
	bool_t def_use_shm;
	bool_t def_sync_shm;

	/* Other info (read only). */
	char *vendor_id;
	int vendor_version;
	int vendor_release_version;


	/* ********************************************************** */
	/* Private (only GUI and wrapper functions may touch). */
	display_t	*display;

	screen_num_t	scr_num;
	screen_ptr_t	*scr_ptr;

	visual_t	*visual;
	visual_id_t	visual_id;

	depth_t		depth,		/* Operative bit depth. */
			actual_depth;	/* Actual bit depth. */
	int		z_bytes;	/* Actual bytes per pixel. */

	bool_t		we_created_colormap;
	colormap_t	colormap;

	gc_t		gc;
	gc_val_t	gc_values;

#ifdef USE_XSHM
        unsigned long	shm_completion_event_code; /* Event code + base. */
#endif	/* USE_XSHM */

} osw_gui_struct;
extern osw_gui_struct osw_gui[1];



/*
 *    OSW Key codes:
 */
typedef struct {

	/* Row 1. */
	keycode_t	esc,

			f1,
			f2,
			f3,
			f4,
			f5,
			f6,
			f7,
			f8,
			f9,
			f10,
			f11,
			f12,

                        f13,
                        f14,
                        f15,
                        f16,
                        f17,
                        f18,
                        f19,
                        f20,
                        f21,
                        f22,
                        f23,
                        f24,

                        f25,
                        f26,
                        f27,
                        f28,
                        f29,
                        f30,
                        f31,
                        f32,
                        f33,
                        f34,
                        f35;


	/* Row 2. */
        keycode_t       tilde,
                        num_1,
                        num_2,
                        num_3,
                        num_4,
                        num_5,
                        num_6,
                        num_7,
                        num_8,
                        num_9,
                        num_0,
			colon,
			lessthan,
			greaterthan,
			questionmark,
			at,
			underscore,
			braketleft,
			braketright,
			quoteleft,
			exclamation,
			numbersign,
			dollarsign,
			percent,
			ampersand,
			apostrophe,
			parenleft,
			parenright,
			asterisk,
			plus,
                        minus,          /* aka underscore. */
                        equal,          /* aka plus. */
			brace_left,
			brace_right,
			bar,
                        backslash,      /* aka pipe. */
                        backspace;

	/* Row 3. */
	keycode_t	tab,
			alpha_q,
			alpha_w,
			alpha_e,
                        alpha_r,
                        alpha_t,
                        alpha_y,
                        alpha_u,
                        alpha_i,
                        alpha_o,
                        alpha_p,
                        alpha_Q,   
                        alpha_W,  
                        alpha_E,   
                        alpha_R, 
                        alpha_T,
                        alpha_Y,
                        alpha_U,
                        alpha_I,   
                        alpha_O,
                        alpha_P,
			enter;

	/* Row 4. */
	keycode_t	caps_lock,
			alpha_a,
                        alpha_s,
                        alpha_d,
                        alpha_f,
                        alpha_g,
                        alpha_h,
                        alpha_j,
                        alpha_k,
                        alpha_l,
                        alpha_A,
                        alpha_S,
                        alpha_D,
                        alpha_F,
                        alpha_G,
                        alpha_H,
                        alpha_J,
                        alpha_K,
                        alpha_L,
			semicolon,
			quote;

	/* Row 5. */
	keycode_t	shift_left,
			alpha_z,
                        alpha_x,
                        alpha_c,
                        alpha_v,
                        alpha_b,
                        alpha_n,
                        alpha_m,
                        alpha_Z,
                        alpha_X,
                        alpha_C,
                        alpha_V,
                        alpha_B,
                        alpha_N,
                        alpha_M,
                        comma,
			period,
			slash,
			shift_right;

	/* Row 6. */
	keycode_t	ctrl_left,
			alt_left,
			win95_start,	/* Micro$oft keyboards have this. */
			space,
			alt_right,
			ctrl_right;


	/* Secondary section. */
	keycode_t	print_screen,	/* aka sys_req. */
			scroll_lock,
			pause;		/* aka break. */

	keycode_t	insert,
			home,
			page_up;

	keycode_t	ddelete,
			end,
			page_down;

	keycode_t	cursor_up,	/* Cursors (clock wise order). */
			cursor_right,
			cursor_down,
			cursor_left;


	/* Number pad. */
	keycode_t	num_lock,
			np_slash,
                        np_asterisk,
                        np_minus,
                        np_add,
                        np_enter,

                        np_1,
                        np_2,
                        np_3,
                        np_4,
                        np_5,
                        np_6,
                        np_7,
                        np_8,
                        np_9,
                        np_0,		/* Aka np_insert. */
			np_period;	/* Aka np_delete. */

} osw_keycode_struct;
extern osw_keycode_struct osw_keycode;


/*
 *	Property atoms:
 *
 *	Unique to X Window Systems.
 */
typedef struct {

	atom_t		_motif_wm_all_clients,
			_motif_wm_hints,
			_motif_wm_info,
			_motif_wm_menu,
			_motif_wm_messages,
			_motif_wm_offset,
			_motif_wm_query,

			/* Standard window manager. */
			wm_delete_window,
			wm_protocols,
			wm_save_yourself,
			wm_state,
			wm_take_focus,

			_xrootpmap_id,
			_xrootcolor_pixel,

			/* Enlightenment window manager. */
			enl_msg,
			enlightenment_desktop,
			enlightenment_comms;

} osw_atom_struct;
extern osw_atom_struct osw_atom;


/*
 *	Even and odd macros:
 */
#define IS_NUM_EVEN(n)	(!((long)(n) & 1))
#define IS_NUM_ODD(n)	((long)(n) & 1)

/*
 *	Is display connected macro.
 */
#define IDC()	(osw_gui[0].display != NULL)


/* ******************************************************************
 *
 *                            Functions
 *
 */

/* GUI Init. */
extern int OSWGUIConnect(int argc, char *argv[]);
extern void OSWGUIDisconnect(void);
extern int OSWLoadKeyCodes(void);


/* Keyboard IO. */
extern char OSWGetASCIIFromKeyCode(
        key_event_t *ke,
        bool_t shift,
        bool_t alt,
        bool_t ctrl
);
extern char *OSWGetKeyCodeName(keycode_t keycode);
extern int OSWIsModifierKey(keycode_t keycode);
extern void OSWKBAutoRepeatOff(void);
extern void OSWKBAutoRepeatOn(void);


/* GUI systems and runtime. */
extern void OSWGUISync(bool_t discard);
extern void OSWGetPointerCoords(
        win_t w,
        int *root_x, int *root_y,
        int *wx, int *wy  
);
extern int OSWGrabPointer(
        win_t grab_w,
        bool_t events_rel_grab_w,	/* Relative to grab_w if True. */
        eventmask_t eventmask,
        int pointer_mode,       /* GrabModeSync or GrabModeAsync. */
        int keyboard_mode,      /* GrabModeSync or GrabModeAsync. */
        win_t confine_w,        /* Can be None. */
        cursor_t cursor         /* Can be None. */
);
extern void OSWUngrabPointer(void);
extern void OSWGUIFree(void **ptr);
extern void *OSWFetchDDE(int *bytes);
extern void OSWPutDDE(void *buf, int bytes);
extern visual_t *OSWGetVisualByCriteria(
        visual_info_mask_t vinfo_mask,
        visual_info_t criteria_vinfo
);
extern visual_t *OSWGetVisualByID(visual_id_t vid);


/* Fonts. */
extern int OSWLoadFont(font_t **font, const char *fontname);
extern font_t *OSWQueryCurrentFont(void);
extern void OSWSetFont(font_t *font);
extern void OSWUnloadFont(font_t **font);


/* Color allocation. */
extern int OSWLoadPixelRGB(
	pixel_t *pix_rtn,
	u_int8_t r,
	u_int8_t g,
	u_int8_t b
);
extern int OSWLoadPixelHSL(
	pixel_t *pix_rtn,
	u_int8_t h,
	u_int8_t s,
	u_int8_t l
);
extern int OSWLoadPixelCLSP(pixel_t *pix_rtn, char *clsp);
extern void OSWDestroyPixel(pixel_t *pix_ptr);
extern void OSWSetFgPix(pixel_t pix);

/* Pointer cursors. */
extern cursor_t OSWLoadBasicCursor(cur_code_t code);
extern void OSWSetWindowCursor(win_t w, cursor_t cursor);
extern void OSWUnsetWindowCursor(win_t w);
extern void OSWDestroyCursor(cursor_t *cursor);

/* Events. */
extern int OSWEventsPending(void);
extern void OSWWaitNextEvent(event_t *event);
extern void OSWWaitPeakEvent(event_t *event);
extern bool_t OSWCheckMaskEvent(eventmask_t eventmask, event_t *event);
extern void OSWWaitWindowEvent(
	win_t w, eventmask_t event_mask, event_t *event
);
extern bool_t OSWCheckWindowEvent(
	win_t w, eventmask_t event_mask, event_t *event
);
extern void OSWPutBackEvent(event_t *event);
extern int OSWSendEvent(
        eventmask_t mask,
        event_t *event,
        bool_t propagate
);
extern int OSWPurgeAllEvents(void);
extern int OSWPurgeOldMotionEvents(void);
extern int OSWPurgeTypedEvent(eventtype_t event_type);
extern int OSWPurgeWindowTypedEvent(win_t w, eventtype_t event_type);
extern int OSWIsEventDestroyWindow(win_t w, event_t *event);


/* Windows. */
extern int OSWCreateWindow(
	win_t *w,
	win_t parent,
	int x, int y,
	unsigned int width, unsigned int height
);
extern int OSWCreateInputWindow(
        win_t *w,
        win_t parent,
        int x, int y,
        unsigned int width, unsigned int height
);
extern void OSWDestroyWindow(win_t *w);
extern bool_t OSWDrawableIsWindow(drawable_t d);
extern void OSWSetWindowWMProperties(
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
);
extern void OSWSetWindowInput(win_t w, eventmask_t eventmask);
extern void OSWSetTransientFor(win_t wbum, win_t wshelter);
extern void OSWMapWindow(win_t w);
extern void OSWMapRaised(win_t w);
extern void OSWMapSubwindows(win_t w);  
extern void OSWUnmapWindow(win_t w);
extern void OSWRestackWindows(win_t *w, int num_w);
extern int OSWIconifyWindow(win_t w);
extern void OSWReparentWindow(win_t w, win_t parent);
extern void OSWMoveWindow(win_t w, int x, int y);
extern void OSWResizeWindow(win_t w, unsigned int width, unsigned int height);
extern void OSWMoveResizeWindow(win_t w, int x, int y,
	unsigned int width, unsigned int height);
extern void OSWSetWindowTitle(Window w, char *title);
extern void OSWClearWindow(win_t w);
extern void OSWSetWindowBkg(win_t w, pixel_t pix, pixmap_t pixmap);
extern bool_t OSWGetWindowAttributes(win_t w, win_attr_t *wattr);
extern int OSWGetWindowRootPos(win_t w, int *x, int *y);
extern win_t OSWGetWindowParent(win_t w);
extern bool_t OSWCheckWindowAncestory(win_t grand_parent, win_t grand_child);

/* Pixmap / graphics buffer. */
extern int OSWCreatePixmap(
	pixmap_t *pixmap,
	unsigned int width, unsigned int height
);
extern void OSWDestroyPixmap(pixmap_t *pixmap);
extern bool_t OSWDrawableIsPixmap(drawable_t d);
extern bool_t OSWGetPixmapAttributes(pixmap_t pixmap, pixmap_attr_t *pattr);
extern void OSWClearPixmap(
	pixmap_t pixmap,
	unsigned int width, unsigned int height, pixel_t pix
);

/* Image. */
extern int OSWCreateImage(
        image_t **image,
        unsigned int width, unsigned int height
);
extern void OSWDestroyImage(image_t **image);

extern int OSWCreateSharedImage(
        shared_image_t **image,
        unsigned int width,
        unsigned int height
);
extern void OSWDestroySharedImage(
        shared_image_t **image
);

/* Graphics conversion. */
extern pixmap_t OSWCreatePixmapFromImage(image_t *image);

/* Image IO. */
extern image_t *OSWGetImage(
	drawable_t d, int x, int y,
        unsigned int width, unsigned int height
);
extern void OSWPutImageToDrawable(image_t *image, drawable_t d);
extern void OSWPutImageToDrawablePos(
	image_t *image, drawable_t d,
	int tar_x, int tar_y
);
extern void OSWPutImageToDrawableSect(
        image_t *image, drawable_t d,
        int tar_x, int tar_y,
        int src_x, int src_y,
        unsigned int width, unsigned int height
);

extern void OSWPutSharedImageToDrawable(
	shared_image_t *image,
	drawable_t d
);
extern void OSWPutSharedImageToDrawablePos(
	shared_image_t *image, drawable_t d,
	int tar_x, int tar_y
);
extern void OSWPutSharedImageToDrawableSect(
        shared_image_t *image, drawable_t d,
        int tar_x, int tar_y,
        int src_x, int src_y,
        unsigned int width, unsigned int height
);

extern void OSWSyncSharedImage(shared_image_t *image, drawable_t d);

extern void OSWPutBufferToWindow(win_t w, gbuf_t gbuf);
extern void OSWCopyDrawables(
	drawable_t tar_d, drawable_t src_d,
	unsigned int width, unsigned int height
);
extern void OSWCopyDrawablesCoord(
        drawable_t tar_d,
        drawable_t src_d,
        int tar_x, int tar_y,
        unsigned int width, unsigned int height,
        int src_x, int src_y
);


/* Drawing. */
extern void OSWDrawString(drawable_t d, int x, int y, const char *string);
extern void OSWDrawStringLimited(
	drawable_t d, int x, int y, const char *string,	int len
);
extern void OSWDrawLine(drawable_t d, int start_x, int start_y,
	int end_x, int end_y);
extern void OSWDrawRectangle(drawable_t d, int x, int y,
	unsigned int width, unsigned int height);
extern void OSWDrawSolidRectangle(drawable_t d, int x, int y,
	unsigned int width, unsigned int height);
extern void OSWDrawArc(drawable_t d, int x, int y,
	unsigned int width, unsigned int height,
	double position_angle,
        double terminal_angle
);
extern void OSWDrawSolidArc(drawable_t d, int x, int y,
        unsigned int width, unsigned int height,
        double position_angle,
        double terminal_angle
);


#endif /* OSW_X_H */
