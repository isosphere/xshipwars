#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <windows.h>


#include "strexp.h"
#include "prochandle.h"
#include "fio.h"
#include "mf.h"


/*
 *	Fetches free and total memory statistics and puts them
 *	at the location pointed to in buf.
 *
 *	Returns 0 on success and -1 on error.
 */
int MemoryFree(mf_stat_struct *buf)
{
	MEMORYSTATUS Mem;

	memset(&Mem,0,sizeof(Mem)); /* Setup win32 struct */
	Mem.dwLength = sizeof(Mem);
	GlobalMemoryStatus(&Mem);

	memset(buf, 0, sizeof(mf_stat_struct)); /* Setup user struct */

	buf->swap_total = Mem.dwTotalPageFile;
	buf->swap_free =  Mem.dwAvailPageFile;
	buf->swap_used =  Mem.dwTotalPageFile - Mem.dwAvailPageFile;

	buf->total = Mem.dwTotalPhys;
	buf->free = Mem.dwAvailPhys;
	buf->used = Mem.dwTotalPhys - Mem.dwAvailPhys;
	return(0);
}

off_t mf(mf_stat_struct *buf)
{
	mf_stat_struct lbuf;

	if(buf == NULL)
	{
		MemoryFree(&lbuf);
        return(lbuf.free);
	}
	else
	{
	    MemoryFree(buf);
	    return(buf->free);
	}
}
