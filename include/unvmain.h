/*
                   Universe Objects Management Main
 */

#ifndef UNVMAIN_H
#define UNVMAIN_H

#include <stdio.h>
#include "objects.h"


/*
 *	Global garbage object.
 */
extern xsw_object_struct unv_garbage_object;


extern void UNVSetupGarbageObject();

extern int UNVInit(int argc, char *argv[]);
extern void UNVShutdown();

#endif	/* UNVFILE_H */
