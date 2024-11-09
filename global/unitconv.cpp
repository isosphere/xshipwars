// global/unitconv.cpp
/*
                          Unit Conversions

	Functions: 

	double ConvertRUToAU(sw_units_struct *units, double x)
	double ConvertRUToSU(sw_units_struct *units, double x)

	double ConvertVelocityRUPCToRUPS(sw_units_struct *units, double x)
	double ConvertVelocityRUPCToAUPS(sw_units_struct *units, double x)

	---

	See ../include/reality.h for official conversion table.

 */

#include <stdio.h>
#include <math.h>

#include "../include/reality.h"



/*
 *	Convert XSW real units to Astronomical units.
 */
double ConvertRUToAU(sw_units_struct *units, double x)
{
	if(units == NULL)
	    return(0);
	else
	    return(x * units->ru_to_au);
}

/*
 *	Convert XSW real units to screen units.
 */
double ConvertRUToSU(sw_units_struct *units, double x)
{
	return(x * 1000);
}


/*
 *	Convert velocity in XSW real units per cycle (quantom)
 *	XSW real units per second.
 */
double ConvertVelocityRUPCToRUPS(sw_units_struct *units, double x)
{
        return(
            x * (1000 / (double)CYCLE_LAPSE_MS)
        );
}

/*
 *      Convert velocity in XSW real units per cycle (quantom)
 *      to Astronomical units per second.
 */
double ConvertVelocityRUPCToAUPS(sw_units_struct *units, double x)
{
        if(units == NULL)
            return(0);
        else
            return(
                (x * units->ru_to_au) *	/* Convert real units to meters. */
		(1000 / (double)CYCLE_LAPSE_MS)
            );
}




