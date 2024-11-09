// global/timming.cpp

#include <sys/time.h>
#include <unistd.h>

/*
 *	returns the number of milliseconds since midnight.
 *
 *	Dan S: A note about this peculiar extern. In this context, extern "C"
 *	       merely forces C linkage, it doesn't actually turn it into extern.
 */
extern "C" {
time_t MilliTime(void)
{
	struct timeval tv[1];

	if(gettimeofday(tv, NULL) < 0)
            return(-1);

        return((tv->tv_sec % 86400) * 1000 + tv->tv_usec / 1000);
}
}




