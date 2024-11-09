/*
                        Universe File IO
 */

#ifndef UNVFILE_H
#define UNVFILE_H

#include <stdio.h>
#include "objects.h"

extern xsw_object_struct **UNVLoadFromFile(
	char *file,
	int *total,
	unv_head_struct *header_buf,
        void *client_data,
	void (*progress_notify)(void *, int, int)
);
extern int UNVSaveToFile(
	char *file,
	xsw_object_struct **obj_ptr,
	int total,
	unv_head_struct *header_buf,
        void *client_data,
        void (*progress_notify)(void *, int, int)
);

#endif	/* UNVFILE_H */
