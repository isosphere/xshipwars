/*
                Universe Objects Management Utility
 */

#ifndef UNVUTIL_H
#define UNVUTIL_H

#include <stdio.h>
#include "objects.h"

extern "C" int UNVAllocScores(xsw_object_struct *obj_ptr);
extern "C" int UNVAllocObjectWeapons(
	xsw_object_struct *obj_ptr, int total_weapons
);
extern "C" int UNVAllocEco(xsw_object_struct *obj_ptr);

extern "C" char *UNVGetObjectFormalName(
	xsw_object_struct *obj_ptr,
	int obj_num
);
extern "C" xsw_object_struct *UNVDupObject(xsw_object_struct *obj_ptr);
extern "C" void UNVResetObject(xsw_object_struct *obj_ptr);
extern "C" void UNVDeleteObject(xsw_object_struct *obj_ptr);
extern "C" void UNVDeleteAllObjects(xsw_object_struct **obj_ptr, int total);

extern "C" void UNVParseLocation(
	const char *s,
	long *sect_x, long *sect_y, long *sect_z,
	double *x, double *y, double *z
);
extern "C" void UNVLocationFormatString(
        char *s,
        const long *sect_x, const long *sect_y, const long *sect_z,
        const double *x, const double *y, const double *z,
        int len
);

extern "C" void UNVParseDirection(  
        const char *s,
        double *heading, double *pitch, double *bank
);
extern "C" void UNVDirectionFormatString(
        char *s,
        const double *heading, const double *pitch, const double *bank,
        int len
);


#endif	/* UNVUTIL_H */
