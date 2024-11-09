/*
                     CPU Load Stat Reading for Linux

	Functions:

	double CPUStatGetCPULoad()

	---

*/

#ifdef __linux__


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <sys/bitypes.h>


/* Stat file containing CPU load information. */
#ifndef CPU_STAT_FILE
    #define CPU_STAT_FILE		"/proc/stat"
#endif


#define STAT_FILE_MAX_SIZE		1024


#define RANGE(min,x,max)	(x > max) ? max : ((x < min) ? min : x)


/*
 *	CPU Statistics Structure:
 */
typedef struct {

	u_int64_t	total,
			user,
			nice,
			system,
			idle;

} cpu_stats_struct;

static cpu_stats_struct cpu_stats_delta;
static cpu_stats_struct cpu_stats_current;
static cpu_stats_struct cpu_stats_last;


/*
 *	Coefficient of previous load:
 */
static double cpu_load_last;


#define ABSOLUTE_VAL(x)		((x) < 0) ? (x * -1) : (x))


double CPUStatGetCPULoad()
{
	FILE *fp;
	int i;
	double x, y, z;

	char buf[STAT_FILE_MAX_SIZE];
	char *strptr;


	/* Error checks. */
	if(access(CPU_STAT_FILE, R_OK) == -1)
	    return(0);


	/* Get data from CPU_STAT_FILE. */
	fp = fopen(CPU_STAT_FILE, "r");
	if(fp == NULL)
	    return(0);

	bzero(buf, STAT_FILE_MAX_SIZE);
	i = fread(buf, sizeof(char), STAT_FILE_MAX_SIZE, fp);
	buf[STAT_FILE_MAX_SIZE - 1] = '\0';

	/* No bytes read or error? */
	if(i <= 0)
	    return(0);

	fclose(fp);
	fp = NULL;


	/* *************************************************** */

	/* Get current stats from buffer. */
	sscanf(buf, "%*s %lu %lu %lu %lu\n",
	    &cpu_stats_current.user,
	    &cpu_stats_current.nice,
	    &cpu_stats_current.system,
	    &cpu_stats_current.idle
	);

	/* Calculate current total. */
	cpu_stats_current.total = cpu_stats_current.user +
            cpu_stats_current.nice + cpu_stats_current.system +
            cpu_stats_current.idle;


	/* Calculate deltas. */
	cpu_stats_delta.total = cpu_stats_current.total - cpu_stats_last.total;
	cpu_stats_delta.user = cpu_stats_current.user - cpu_stats_last.user;
        cpu_stats_delta.nice = cpu_stats_current.nice - cpu_stats_last.nice;
        cpu_stats_delta.system = cpu_stats_current.system - cpu_stats_last.system;
        cpu_stats_delta.idle = cpu_stats_current.idle - cpu_stats_last.idle;


	/* Record currents on last. */
	cpu_stats_last.total = cpu_stats_current.total;
	cpu_stats_last.user = cpu_stats_current.user;
	cpu_stats_last.nice = cpu_stats_current.nice;
	cpu_stats_last.system = cpu_stats_current.system;
	cpu_stats_last.idle = cpu_stats_current.idle;


	/* Calculate current load. */
	y = (double)(cpu_stats_delta.user + cpu_stats_delta.system);
	z = (double)(cpu_stats_delta.nice + cpu_stats_delta.idle);
	if(z > 0)
	    x = y / z;
	else
	    x = 0.99;


	/* Change cannot be too great. */
	if((x - cpu_load_last) > 0.25)
	    x = cpu_load_last + 0.25;
	else if((x - cpu_load_last) < -0.25)
	    x = cpu_load_last - 0.25;


	/* Record current as last. */
	cpu_load_last = (double)RANGE(0, (double)x, 1);

	return(cpu_load_last);
}


#endif /* __linux__ */
