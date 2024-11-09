/*
	                Joystick Wrapper Library

	Event based joystick device portability library.

	An example programs should acompany the package that this
	header file came in.

        http://wolfpack.twu.net/libjsw

 */

#ifndef JSW_H
#define JSW_H

#include <time.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif


/*
 *	Version of libjsw:
 */
#define JSWVersion		"1.4.1"
#define JSWVersionMajor		1
#define JSWVersionMinor		4
#define JSWVersionRelease	1


/*
 *	OS Specific Definations:
 */
#ifdef __linux__
# include <linux/joystick.h>
# define JSDefaultDevice		"/dev/js0"
# define JSDefaultCalibration		".joystick"
#endif

/* Assume defaults for device and calibration file.
 *
 * Note that newer Linuxes use /dev/input/js# for USB joysticks,
 * but still use /dev/js# for regular joysticks.
 */
#ifndef JSDefaultDevice
# define JSDefaultDevice		"/dev/js0"
#endif

#ifndef JSDefaultCalibration
# define JSDefaultCalibration		".joystick"
#endif



/*
 *	Default range values (in raw units):
 */
#ifndef JSDefaultMin
# define JSDefaultMin		0
#endif

#ifndef JSDefaultMax
# define JSDefaultMax		1000
#endif

#ifndef JSDefaultCenter
# define JSDefaultCenter	500
#endif

#ifndef JSDefaultNullZone
# define JSDefaultNullZone	100
#endif

#ifndef JSDefaultTolorance
# define JSDefaultTolorance	1
#endif


/*
 *	Joystick device flags:
 */
#define JSFlagIsInit		(1 << 1)
#define JSFlagNonBlocking	(1 << 2)

/*
 *	Axis flags:
 */
#define JSAxisFlagFlipped	(1 << 1)	/* Flip values. */
#define JSAxisFlagIsHat		(1 << 2)	/* Is a hat. */
#define JSAxisFlagTolorance	(1 << 3)	/* Use error connection. */


/*
 *	Error codes:
 */
#define JSSuccess	0
#define JSError		1
#define JSBadValue	2
#define JSNoAccess	3
#define JSNoBuffers	4


/*
 *	Event codes:
 */
#define JSNoEvent	0
#define JSGotEvent	1

/*
 *	Button state codes:
 */
#define JSButtonStateOff	0
#define JSButtonStateOn		1

#define JSButtonChangedStateNone	0	/* No change. */
#define JSButtonChangedStateOffToOn	1	/* Up to down position. */
#define JSButtonChangedStateOnToOff	2	/* Down to up position. */


/*
 *	Joystick axis structure:
 */
typedef struct {

	/* Public. */

	/* Axis ranges and bounds (all raw units). */
	int cur, prev;		/* Current and previous position. */
	int min, cen, max;	/* Bounds. */
	int nz;			/* Null zone, in raw units from cen. */
	int tolorance;		/* Precision snap in raw units
				 * (used only if JSAxisFlagTolorance is
				 * set).
				 */

	/* Flags, any of JSAxisFlag*. */
	unsigned int flags;

	/* Time stamp of current (newest) event and last event (in ms). */
	time_t time, last_time;

} js_axis_struct;

/*
 *	Joystick button structure:
 */
typedef struct {

	/* Current and previous button states, one of JSButtonState*. */
	int state, prev_state;

	/* Changed state from previous call to JSUpdate(), one of
	 * JSButtonChangedState*.
	 */
	int changed_state;

        /* Time stamp of current (newest) event and last event (in ms). */
        time_t time, last_time;

} js_button_struct;

/*
 *	Loaded joystick caliberation ahd resources structure:
 */
typedef struct {

	/* Public, read only. */
	char *name;			/* Descriptive name. */

	js_axis_struct **axis;		/* Axises. */
	int total_axises;

	js_button_struct **button;	/* Buttons. */
	int total_buttons;

	char *device_name;		/* Device name. */
	char *calibration_file;		/* Associated caliberation file. */

	/* Private, do not access. */
	int fd;
	unsigned int flags;
	unsigned int driver_version;

} js_data_struct;


/*
 *	Joystick device attributes structure:
 */
typedef struct {

	char *name;			/* Descriptive name. */
	char *device_name;		/* Device name. */

	/* Specifies that this joystick device is configured and
	 * caliberated properly. Otherwise opening this joystick
	 * may produce distorted values.
	 */
	int is_configured;

	/* If true then suggests device may already be opened. */
	int is_in_use;

	/* Device may not disconnected at the moment, turned off, or
	 * driver not loaded for it but it may be configured
	 * (see is_configured for that information).
	 */
	int not_accessable;

} js_attribute_struct;


#ifdef __linux__
/*
 *      Loads caliberation information from file who's format for Linux
 *	joystick caliberation information.
 *
 *	Only used internally by this library and jscalibrator, most
 *	programs do not need to worry about this.
 */
#if defined(__cplusplus) || defined(c_plusplus)
extern "C" int JSLoadCalibrationLinux(js_data_struct *jsd);
#else
extern int JSLoadCalibrationLinux(js_data_struct *jsd);
#endif

/*
 *      Gets a list of caliberated devices found in the specified
 *      caliberation file.
 *
 *      The returned list of strings and the pointer array must be
 *      free()'ed by the calling function.
 */
#if defined(__cplusplus) || defined(c_plusplus)
extern "C" char **JSLoadDeviceNamesLinux(
        int *total,
        const char *caliberation
);
#else
extern char **JSLoadDeviceNamesLinux(
        int *total,
        const char *caliberation
);
#endif

#endif	/* __linux__ */


/*
 *	Checks if the joystick is initialized.
 */
#if defined(__cplusplus) || defined(c_plusplus)
extern "C" int JSIsInit(js_data_struct *jsd);
#else
extern int JSIsInit(js_data_struct *jsd);
#endif

/*
 *	Fetches attributes for all joysticks accessable (configured
 *	or not) on the system. Does not matter if the joystick is
 *	already opened or not. Returned values need to be free()'ed
 *	with a call to JSFreeAttributeList().
 *
 *	If caliberation (caliberation file) is NULL then some values
 *	may not be set in the attrib structure (ie is_configured and
 *	name).
 */
#if defined(__cplusplus) || defined(c_plusplus)
extern "C" js_attribute_struct *JSGetAttributesList(
	int *total,
	const char *caliberation	/* Caliberation file. */
);
#else
extern js_attribute_struct *JSGetAttributesList(
	int *total,
	const char *caliberation        /* Caliberation file. */
);
#endif

/*
 *	Frees a list of js_attribute_structs and their substructures.
 */
#if defined(__cplusplus) || defined(c_plusplus)
extern "C" void JSFreeAttributesList(js_attribute_struct *list, int total);
#else
extern void JSFreeAttributesList(js_attribute_struct *list, int total);
#endif


/*
 *	Returns the raw unparsed version code of the joystick driver
 *	(not the version of the libjsw library).
 */
#if defined(__cplusplus) || defined(c_plusplus)
extern "C" unsigned int JSDriverVersion(js_data_struct *jsd);
#else
extern unsigned int JSDriverVersion(js_data_struct *jsd);
#endif

/*
 *      Returns the parsed version code of the joystick driver 
 *      (not the version of the libjsw library).
 */
#if defined(__cplusplus) || defined(c_plusplus)
extern "C" int JSDriverQueryVersion(
        js_data_struct *jsd,
        int *major_rtn, int *minor_rtn, int *release_rtn
);
#else
extern int JSDriverQueryVersion(
        js_data_struct *jsd,
        int *major_rtn, int *minor_rtn, int *release_rtn
);
#endif


/*
 *	Checks if the joystick axis is valid and allocated.
 */
#if defined(__cplusplus) || defined(c_plusplus)
extern "C" int JSIsAxisAllocated(js_data_struct *jsd, int n);
#else
extern int JSIsAxisAllocated(js_data_struct *jsd, int n);
#endif

/*
 *	Checks if the joystick button is valid and allocated.
 */
#if defined(__cplusplus) || defined(c_plusplus)
extern "C" int JSIsButtonAllocated(js_data_struct *jsd, int n);
#else
extern int JSIsButtonAllocated(js_data_struct *jsd, int n);
#endif


/*
 *	Gets the coefficient value of axis n, does not take null
 *	zone into account.
 */
#if defined(__cplusplus) || defined(c_plusplus)
extern "C" double JSGetAxisCoeff(js_data_struct *jsd, int n);
#else
extern double JSGetAxisCoeff(js_data_struct *jsd, int n);
#endif

/*
 *	Same as JSGetAxisCoeff, except takes null zone into account.
 */
#if defined(__cplusplus) || defined(c_plusplus)
extern "C" double JSGetAxisCoeffNZ(js_data_struct *jsd, int n);
#else
extern double JSGetAxisCoeffNZ(js_data_struct *jsd, int n);
#endif

/*
 *	Gets the button state of button n, one of JSButtonState*.
 */
#if defined(__cplusplus) || defined(c_plusplus)
extern "C" int JSGetButtonState(js_data_struct *jsd, int n);
#else
extern int JSGetButtonState(js_data_struct *jsd, int n);
#endif

/*
 *      Used by the caliberation program to sync values for tolorance
 *      in error correction specified in the current axis structures
 *      to the low-level joystick driver.
 */
#if defined(__cplusplus) || defined(c_plusplus)
extern "C" void JSResetAllAxisTolorance(js_data_struct *jsd);
#else
extern void JSResetAllAxisTolorance(js_data_struct *jsd);
#endif


/*
 *	Initializes the joystick and stores the new initialized values
 *	into the jsd structure.
 *
 *	If the device is not specified (set to NULL), then it will
 *	be defauled to JSDefaultDevice.
 *
 *	If the calibration file is not specified (set to NULL), then
 *	it will be defaulted to JSDefaultCalibration. The HOME
 *	enviroment value will be used as the prefix to the path of
 *	JSDefaultCalibration. The calibration file does not have to
 *	exist.
 *
 *	Available inputs for flags are any of the or'ed following:
 *
 *	JSFlagNonBlocking		Open joystick in non-blocking mode.
 */
#if defined(__cplusplus) || defined(c_plusplus)
extern "C" int JSInit(
	js_data_struct *jsd,
	const char *device,
	const char *calibration,
	unsigned int flags
);
#else
extern int JSInit(
	js_data_struct *jsd,
	const char *device,
	const char *calibration,
	unsigned int flags
);
#endif

/*
 *	Fetches the next event and updates joystick values specified in
 *	the jsd.  Can return JSNoEvent or JSGotEvent depending on if
 *	an event was recieved.
 */
#if defined(__cplusplus) || defined(c_plusplus)
extern "C" int JSUpdate(js_data_struct *jsd);
#else
extern int JSUpdate(js_data_struct *jsd);
#endif

/*
 *	Shuts down the joystick specified in the jsd that was initialized
 *	by a previous call to JSInit().
 */
#if defined(__cplusplus) || defined(c_plusplus)
extern "C" void JSClose(js_data_struct *jsd);
#else
extern void JSClose(js_data_struct *jsd);
#endif


#ifdef __cplusplus
}
#endif

#endif	/* JSW_H */
