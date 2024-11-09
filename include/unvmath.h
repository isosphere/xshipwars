/*
            Universe Objects Management Math Utilities
 */

#ifndef UNVMATH_H
#define UNVMATH_H


#include "objects.h"



extern double RADTODEG(double radians);
extern double DEGTORAD(double degrees);
extern double SANITIZERADIANS(double radians);
extern double SANITIZEDEGREES(double degrees);

extern double MuPolarRotX(double theta, double r);
extern double MuPolarRotY(double theta, double r);

extern void MuSetUnitVector2D(
        xsw_vector_compoent_struct *v,
        double bearing
);
extern void MuSetUnitVector3D(
        xsw_vector_compoent_struct *v,
        double bearing, double pitch
);

extern void MuVectorAdd(
        double v1_dir, double v1_mag,
        double v2_dir, double v2_mag,
        double *v_dir_rtn, double *v_mag_rtn
);
extern double MuCoordinateDeltaVector(double dx, double dy);
extern double Mu3DDistance(double dx, double dy, double dz);

extern int Mu3DInSectorPtr(
	xsw_object_struct *obj_ptr,
	long sect_x, long sect_y, long sect_z
);
extern int Mu3DInSameSector(
        xsw_object_struct **xsw_object,
        int total_objects,
	int object_num1, int object_num2
);
extern int Mu3DInSameSectorPtr(
        xsw_object_struct *obj1_ptr,
        xsw_object_struct *obj2_ptr
);
extern int Mu3DInContact(
	xsw_object_struct **xsw_object,
	int total_objects,
	int object_num1, int object_num2
);
extern int Mu3DInContactPtr(
        xsw_object_struct *obj1_ptr,
        xsw_object_struct *obj2_ptr
);
extern int Mu3DInVectorContact(
        xsw_object_struct **xsw_object,
        int total_objects,
        int object_num1, int object_num2,
        double heading,
        double heading_variance,
        double range		/* In pixel units. */
);
extern int Mu3DInRange(
        xsw_object_struct **xsw_object,
        int total_objects,
        int object_num1, int object_num2,
        double distance		/* In real units. */
);
extern int Mu3DInRangePtr(
        xsw_object_struct *obj1_ptr,
        xsw_object_struct *obj2_ptr,
        double distance		/* In real units. */
);
extern double MuMaxRangeByVelocity(double v_max, long t);




#endif	/* UNVMATH_H */
