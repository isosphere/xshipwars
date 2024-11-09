/*
                             Memory Free

	Gets statistics on free and total memory.
 */

#ifndef MF_H
#define MF_H


#include <stdio.h>
#include <sys/types.h>
#include <limits.h>

/* Command to run the free program, includes required arguments. */
#if defined(__linux__)
# define FREE_CMD	"free"
#else
# define FREE_CMD	"free"
#endif



#ifndef NAME_MAX
# define NAME_MAX	256
#endif

#ifndef PATH_MAX
# define PATH_MAX	1024
#endif


/*
 *	Memory statistics structure:
 */
typedef struct {

	/* In bytes. */
	off_t	total,
		used,
		free,
		shared,
		buffers,
		cached,
		swap_total,
		swap_used,
		swap_free;

} mf_stat_struct;


extern off_t mf(mf_stat_struct *buf);
extern int MemoryFree(mf_stat_struct *buf);


#endif	/* DF_H */
