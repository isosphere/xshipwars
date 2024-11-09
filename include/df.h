/*
                           Device Free

	Gets listing of devices and their current capacities.
 */

#ifndef DF_H
#define DF_H


#include <stdio.h>
#include <sys/types.h>
#include <limits.h>


/*
 *   Command to run the df program:
 *
 *	Should include required arguments.
 */
#if defined(__FreeBSD__)
# define DF_CMD         "df -k"
#endif

#if defined(__HPUX__)
# define DF_CMD         "df -Pk"
#endif

#if defined(__linux__)
# define DF_CMD		"df -P --no-sync"
#endif

#if defined(__SOLARIS__)
# define DF_CMD         "df -k"
#endif

/* All else, assume POSIX responsive. */
#ifndef DF_CMD
# define DF_CMD		"df -P"
#endif


typedef struct {

	char mounted_on[NAME_MAX + PATH_MAX];
	char filesystem[NAME_MAX + PATH_MAX];

	/* Size stats, in multiples of 1024 bytes. */
	off_t total;
	off_t used;
	off_t available;

} df_stat_struct;


extern df_stat_struct **DiskFreeGetListing(int *total);
extern void DiskFreeDeleteListing(df_stat_struct **df_stat, int total);

#endif	/* DF_H */
