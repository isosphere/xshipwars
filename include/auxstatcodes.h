/*
                            AUX Stat Codes

	Stat codes used in identifying the data segements sent
	through AUX pipes from the ShipWars server.

	Data sent through the AUX pipes from the ShipWars server
	contain information about the status of the server.

 */

#ifndef AUXSTATCODES_H
#define AUXSTATCODES_H

#include "os.h"		/* Need to know about pipe limits. */


/*
 *	Maximum amount of data allowed per cacheing.
 */
#define AUXSTAT_MAX_LEN		(1024 * 10)


/*
 *	Maximum segment length, must be less than or equal
 *	to AUXSTAT_MAX_LEN.
 */
#define AUXSTAT_SEGMENT_MAX	1024

#if AUXSTAT_SEGMENT_MAX > AUXSTAT_MAX_LEN
# undef AUXSTAT_SEGMENT_MAX
# define AUXSTAT_SEGMENT_MAX	AUXSTAT_MAX_LEN
#endif	/* AUXSTAT_SEGMENT_MAX > AUXSTAT_MAX_LEN */


/*
 *	AUX Stat Prefix Codes:
 *
 *	These codes are prefix before each stat data segment sent
 *	through the AUX pipes.
 *
 *	Each data segment is delimited by a '\n' and/or '\0'
 *	character(s).  The syntax for each data segment is:
 *
 *		"<code>: <data>\n"
 *
 *	Example:
 *
 *		"UNVTITLE: Generic Universe\n"
 */

/* Client to server. */

/* sname;spassword */
#define STAT_PFX_LOGIN		"LOGIN"



/* Server to client. */

/* itotalcon iguestcon */
#define STAT_PFX_CONNECTIONS	"CONNECTIONS"

/* lmem_total lmemcon lmemobj */
#define STAT_PFX_MEMORY		"MEMORY"

/* smesg */
#define STAT_PFX_MESSAGE	"MESSAGE"

/* lnextsave lnextexport */
#define STAT_PFX_NEXT		"NEXT"

/* stitle */
#define STAT_PFX_TITLE		"TITLE"

/* itotalobj */
#define STAT_PFX_OBJECTS	"OBJECTS"

/* ipid */
#define STAT_PFX_PID		"PID"

/* */
#define STAT_PFX_REQUESTLOGIN	"REQUESTLOGIN"

/* luptime */
#define STAT_PFX_UPTIME		"UPTIME"



#endif /* AUXSTATCODES_H */
