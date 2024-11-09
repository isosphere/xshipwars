#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "../include/os.h"
#include "../include/strexp.h"
#include "../include/prochandle.h"
#include "../include/fio.h"
#include "../include/df.h"


/*
 *	Returns a dynamically allocated array of device listings.
 *	The number of device listings allocated will be indicated
 *	by total.
 */
df_stat_struct **DiskFreeGetListing(int *total)
{
	return(NULL);
}


void DiskFreeDeleteListing(df_stat_struct **df_stat, int total)
{
	return;
}
