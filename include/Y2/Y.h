/*
	                  Y Sound Systems

	         Client to Server Protocol Definations


        All Y client programs needs to #include this file and link to
        the libY2 library.

	For contact and programming information, see:

	http://wolfpack.twu.net/YIFF

 */

#ifndef Y_H
#define Y_H

#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif


/*
 *	Basic True and False:
 */
#ifndef False
# define False  0
#endif

#ifndef True
# define True   1
#endif


/*
 *      Types:
 */
typedef char            Boolean;
typedef unsigned long   YID;
typedef unsigned long   YMask;
typedef double          Coefficient;
typedef long            YDataPosition;
typedef long            YDataLength;


/* IP address union. */
typedef union {
	u_int32_t whole;
	u_int8_t charaddr[4];
} YIPUnion;


/* 
 *      All purpose NULL YID:
 */
#define YIDNULL         (YID)0


/*
 *	Full path to disk object name max:
 */
#define YPathMax	1024

/*
 *      Maximum audio mode name length:
 */
#define YAudioNameMax	256


/*
 *	Maximum vendor name length:
 */
#define YVendorNameMax	64


/*
 *	Network data receive buffer:
 *
 *	This value must be able to hold several network data segments,
 *	each segment should not be bigger than about 1000 bytes at most.
 *	So 30000 bytes could hold about 30 huge events.  Though most event
 *	segments are only about 100 bytes or less though.
 */
#define YNetRecvBufLen	30000

/*
 *	Maximum allowed queued YIFF events per (client) connection:
 *
 *	Note that the number of queued events per connection
 *	is allocated to match the number queued events. The entire
 *	number of YQueuedEventsMax queued events is not allocated
 *	initially.
 */
#define YQueuedEventsMax	500


/*
 *      Number of values per mixer channel device:
 */
#define YMixerValues    2


/*
 *	Sound object data format type codes:
 *
 *	This indicates the type of data the sound object contains
 *	and the type of sound object itself.
 *
 *	They are not the format types of the sound object as a
 *	file on non-volatile storage (ie on disk).
 */
#define SndObjTypeNone		0
#define SndObjTypeDSP		1	/* Digital audio sample. */
#define SndObjTypeMIDI		2


/*
 *	Major operation codes (also YEvent types):
 */
#define YAudioChange		1
# define YAudioChangePreset		0
# define YAudioChangeValues		1
#define YCycleChange		2
#define YDisconnect		3
#define YSetHost		4
# define YSetHostAdd			0
# define YSetHostRemove			1
#define YListHosts		5
# define YListHostsGet			0
# define YListHostsSet			1
#define YMixerChannel		6
# define YMixerChannelGet		0
# define YMixerChannelSet		1
#define YListMixers		7
# define YListMixersGet			0
# define YListMixersSet			1
#define YSoundObjectPlay	8
#define YSoundObjectKill	9
#define YSoundObjectAttributes	10
# define YSoundObjectAttributesGet	0
# define YSoundObjectAttributesSet	1
#define YShutdown		11
#define YSync			12
#define YAudioStats		13	/* Audio device stats. */
# define YAudioStatsGet			0
# define YAudioStatsSet			1
#define YServerStats		14
# define YServerStatsGet		0
# define YServerStatsSet		1
#define YListAudioModes		15
# define YListAudioModesGet		0
# define YListAudioModesSet		1
#define YSoundObjectPlayValues  16
# define YSoundObjectPlayValuesGet      0
# define YSoundObjectPlayValuesSet      1

#ifdef __cplusplus
}
#endif

#endif	/* Y_H */
