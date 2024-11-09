/*
                     Universe Object Matching

 */

#ifndef UNVMATCH_H
#define UNVMATCH_H

#include "objects.h"


extern int UNVIsObjectGarbage(
	xsw_object_struct **xsw_object,
        int total_objects,
	int n
);
extern int UNVIsObjectNameIndex(const char *name);
extern double UNVGetObjectVisibility(
        xsw_object_struct **xsw_object,
        int total_objects,
        int n
);
extern double UNVGetObjectVisibilityPtr(xsw_object_struct *obj_ptr);
extern int MatchWeaponsLock(
	xsw_object_struct **xsw_object,
	int total_objects,
	int ref_obj,
	int start_obj,
	double range
);
extern int MatchIntercept(
	xsw_object_struct **xsw_object,
        int total_objects,
	int object_num,
	const char *arg
);
extern int MatchInterceptByNumber(
        xsw_object_struct **xsw_object,
        int total_objects,  
	int ref_obj, int tar_obj
);
extern int MatchObjectByName(
	xsw_object_struct **xsw_object,
        int total_objects,
        const char *name, int type
);
extern int MatchIFF(
	xsw_object_struct **xsw_object,
	int total_objects,
	int ref_obj, int tar_obj
);
extern int MatchIFFPtr(
	xsw_object_struct *ref_obj_ptr,
	xsw_object_struct *tar_obj_ptr
);


#endif	/* UNVMATCH_H */
