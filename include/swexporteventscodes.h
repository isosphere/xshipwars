/*
	Code values for ShipWars server Events export stats format.

	These values are used to interprite the format of the
	exported file EventsExportFile specified in the server
	configuration.

	This file contains events of public interest intended to be
	read and displayed by cgi scripts for WWW viewing.

 */

#ifndef SWEXPORTEVENTSCODES_H
#define SWEXPORTEVENTSCODES_H


/*
 *	Events export stats file format contains a new line seperated
 *	list of entries. Each entry is a line of the format:
 *
 *	<systime>;<type>;<data>\n
 *
 *	Where:
 *
 *	<systime> is the system time stamp in seconds.
 *
 *	<type> is the type code of that entry (and determines
 *	what <data> should be). <type> is one of EVST_TYPE_* #defined
 *	farther below.
 *
 *	<data> depends on what <type> is, typically it is a message.
 *
 *	\n is a new line character marking the end of the entry.
 */


/*
 *	Events entry type code, determines type of event entry.
 */
#define EVST_TYPE_MESSAGE		0	/* Standard message. */

#define EVST_TYPE_ARRIVE		10	/* Arrive/landed/docked. */
#define EVST_TYPE_DEPART		11	/* Departed. */

#define EVST_TYPE_STREAMWEAPON_FIRED	20	/* Should be ignored. */
#define EVST_TYPE_PULSEWEAPON_FIRED	21
#define EVST_TYPE_PROJECTILEWEAPON_FIRED	22

#define EVST_TYPE_STREAMWEAPON_HIT	30
#define EVST_TYPE_PULSEWEAPON_HIT	31
#define EVST_TYPE_PROJECTILEWEAPON_HIT	32

#define EVST_TYPE_DESTROYEDOBJECT	40	/* Object got destroyed. */
#define EVST_TYPE_DESTROYEDPLAYER	41	/* Player got destroyed. */
#define EVST_TYPE_DESTROYEDWEAPON	42	/* Ignore this. */




#endif	/* SWEXPORTEVENTSCODES_H */
