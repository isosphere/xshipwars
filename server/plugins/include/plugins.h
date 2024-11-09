/*
	Plugins header file for ShipWars Server plugins.

	Don't confuse this plugins.h for the one in the ShipWars Server
	source, they're different.
 */

/* Define PLUGIN_SUPPORT as needed so that ../../plugins.h will know
 * PLUGIN_SUPPORT is defined. Since this header file is included by
 * plugins, they don't know they need to define PLUGIN_SUPPORT.
 * Where as PLUGIN_SUPPORT is only needed as a compile time directive for
 * the ShipWars Server.
 */
#ifndef PLUGIN_SUPPORT
# define PLUGIN_SUPPORT
#endif

#include "../../plugins.h"


/* Other ShipWars related definations. */
#include "../../../include/reality.h"
#include "../../../include/objects.h"
#include "../../../include/swsoundcodes.h"
