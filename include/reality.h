/*
                Official ShipWars Reality Constants


	Notation legend:

	ms = milliseconds
	us = microseconds

	rl = real life
	ru = XSW real units

	su = screen units


	Unit conversion legend:

	1 XSW second = 1 real life second

	1 screen unit = 1 pixel (at 1:1 zoom)
	1 screen unit = 0.001 XSW real units
	1 screen unit = 8 meters

	1 XSW real unit = 1000 screen units
	1 XSW real unit = 8000 meters

	1 meter = 0.125 screen units
	1 meter = 0.000125 XSW real units


 */


#ifndef REALITY_H
#define REALITY_H


/*
 *   Cyclic Time Lapse:
 *
 *	The main loop should usleep() for CYCLIC_MICROTIME_LAPSE
 *	microseconds.   All speed calculations are pre these numbers.
 *
 *	In addition, catchup time for speed should be multiplied by the
 *	differance of this value.  Example:
 *	Suppose you calculated x to be 50 (meters per second).
 *	Then you multiply x with ( last_millitime / CYCLIC_MILLITIME_LAPSE )
 *	to correct x for any time lost in the previous loop.
 *	last_millitime in this example would be the millitime of the last
 *	loop.
 */
#define CYCLE_LAPSE_MS 	16
#define CYCLE_LAPSE_US	16000


/*
 *   Sector sizes:
 *
 *	In XSW real units.  Total length of one sector is 163840.
 */
#define SECTOR_SIZE_X_MIN	-1024
#define SECTOR_SIZE_X_MAX	1024
#define SECTOR_SIZE_Y_MIN	-1024
#define SECTOR_SIZE_Y_MAX	1024
#define SECTOR_SIZE_Z_MIN	-1024
#define SECTOR_SIZE_Z_MAX	1024


/*
 *   Drag Coefficents:
 */
#define SUBLIGHT_DRAG_COEF  0.0
#define WARP_DRAG_COEF      0.009
#define TRANSWARP_DRAG_COEF 0.03


/*
 *	Velocity thresholds:
 */
#define WARP_THRESHOLD       0.04
#define TRANSWARP_THRESHOLD  0.3


/*
 *	Frequency ranges (in kHz):
 *
 *	(Note: Frequency compare resolution is 2 decimal places).
 */
#define SWR_FREQ_MIN		1.00
#define SWR_FREQ_MAX		900.00


/*
 *	Short-range (non subspace) communications range:
 *
 *	Radius range for sending regular communication messages and
 *	hails.  Units are in XSW Real units.
 */
#define COM_SHORT_RANGE_MAX	100


/*
 *	Shield phaze visibility interval:
 *
 *	In milliseconds.
 */
#define SHIELD_VISIBILITY_INTERVAL	3000


/*
 *	Visibility in a nebula:
 */
#define VISIBILITY_NEBULA	0.15


/*
 *	Worm hole enter delay:
 *
 *	Cannot re-enter a worm hole untill this delay has expired
 *	(in milliseconds).
 */
#define WORMHOLE_ENTER_DELAY	5000

/*
 *	Maximum switchable throttle power:
 *
 *	Cannot switch throttle to reverse if throttle setting is
 *	greater than this value.
 */
#define THROTTLE_MAX_SWITCH	0.3

   
/*
 *	PI Constants:
 */
#ifndef PI
    #define PI		3.141592654
#endif

#ifndef TWOPI
    #define TWOPI	6.283185307
#endif

#ifndef HALFPI
    #define HALFPI	1.570796327
#endif



/*
 *	Unit conversion defaults:
 *
 *	Must be greater than 0!
 *
 *	1 AU = 149,600,000 km
 */
#define DEF_UNITCONV_RU_TO_AU	0.1


/*
 *	Unit conversions:
 */
typedef struct {                           

        double ru_to_au;        /* SW Real units to Astronomical units. */

} sw_units_struct;



/* In unitconv.c */
extern double ConvertRUToAU(sw_units_struct *units, double x);
extern double ConvertRUToSU(sw_units_struct *units, double x);

extern double ConvertVelocityRUPCToRUPS(sw_units_struct *units, double x);
extern double ConvertVelocityRUPCToAUPS(sw_units_struct *units, double x);



#endif /* REALITY_H */
