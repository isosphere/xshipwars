/*
                          Y Sound Systems

                 Client to Server Library Functions


        All Y client programs needs to #include this file and link to
        the libY2 library.

        For contact and programming information, see:

        http://wolfpack.twu.net/YIFF

 */

#ifndef YLIB_H
#define YLIB_H

#include <stdio.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "Y.h"

/*
 *	Volume data structure:
 */
typedef struct {

	/* Coefficient = (val / ((u_int16_t)-1)) */

	u_int16_t	left,
			right;

} YVolumeStruct;


/*
 *	Event structures:
 */

/* Change mode event structure. */
typedef struct {

	Boolean preset;		/* Use preset? */

	/* If preset is True, these members contain correct info. */
	char mode_name[YAudioNameMax];

	/* If preset is False, these members contain correct info. */
	int sample_size;
	int channels;
	int sample_rate;
	int direction;
	int allow_fragmenting;
	int num_fragments;
	int fragment_size_bytes;

} YEventAudioChange;

/* Y server stats event structure. */
typedef struct {

	int protocol_version_major;
	int protocol_version_minor;

	Coefficient cycle_load;

	/* More items to be added in future. */

} YEventServerStats;

/* Change cycle event structure. */
typedef struct {

	long cycle_us;

} YEventCycleChange;

/* Disconnect event structure. */
typedef struct {

	int reason;	/* Why you were disconnected. */

} YEventDisconnect;

/* Host add or remove event structure. */
typedef struct {

	int op;		/* 0 for remove, 1 for add. */
	YIPUnion ip;

} YEventHost;

/* Sound kill event structure. */
typedef struct {

	YID yid;

} YEventSoundKill;

/* Mixer event structure. */
typedef struct {

	int code;		/* Mixer device code. */

        Coefficient value[YMixerValues];	/* Argument values. */

} YEventMixer;

/* Sound play event structure. */
typedef struct {

/* These flag definations need to match those of the server. */
#define YPlayValuesFlagYID              (1 << 1)
#define YPlayValuesFlagPosition         (1 << 2)
#define YPlayValuesFlagLength           (1 << 3)
#define YPlayValuesFlagRepeats          (1 << 4)
#define YPlayValuesFlagTotalRepeats     (1 << 5)
#define YPlayValuesFlagVolume           (1 << 6)
#define YPlayValuesFlagSampleRate       (1 << 7)

        unsigned long flags;

        YID yid;

        YDataPosition position;         /* Current play position in bytes. */
        YDataLength length;             /* Length of audio data in bytes. */

        int     repeats,                /* Number of repeats so far. */
                total_repeats;          /* Total number of repeats. */

        Coefficient     left_volume,    /* Volume from 0.0 to 1.0. */
                        right_volume;

        int sample_rate;                /* Applied sample rate in Hz. */

} YEventSoundPlay;

/* Sound object attributes event structure. */
typedef struct {

	int format;
	int sample_size;
	int channels;
	int sample_rate;

	char path[YPathMax];

} YEventSoundObjectAttributes;

/* Shutdown event structure. */
typedef struct {

	int reason;	/* Why server shut down. */

} YEventShutdown;

/* Sync event structure. */
typedef struct {

	long cycle_ahead_us;

} YEventSync;

/* Audio stats structure. */ 
typedef struct { 

	int cycle_set;

	long cycle_us;
	long compensated_cycle_us;

	long write_ahead_us;
	long cumulative_latency_us;

	int sample_size;
	int channels;
	int sample_rate;
	int bytes_per_sec;

	Boolean allow_fragments;
	int num_fragments;
	int fragment_size;

	Boolean flip_stereo;
	int direction;

} YEventAudioStats;

/* Main YEvent structure. */
typedef struct {

        int type;       /* Event type code (aka the op code). */

        YEventAudioChange    audio;
	YEventServerStats    serverstats;
        YEventCycleChange    cycle;
        YEventDisconnect     disconnect;
        YEventHost           host;
        YEventSoundKill      kill;
        YEventMixer          mixer;
        YEventSoundPlay      play;
	YEventSoundObjectAttributes	sndobjattributes;
        YEventShutdown       shutdown;
        YEventSync           sync;
        YEventAudioStats     audiostats;

} YEvent;


/*
 *      Connection to YIFF server structure:
 */
typedef struct {

        int fd;

        int we_started_server;		/* 1 if we started the Y server. */

        /* Queued events. */
        int total_queued_events;
        YEvent *queued_event;

        YID prev_generated_yid;

        /* Receive buffer. */
        u_int8_t *buf;
        YDataLength     buf_len,        /* Allocated length of buf. */
                        buf_cont;       /* Contents in buf. */

} YConnection;

/*
 *	Audio mode list structure:
 */
typedef struct {

	char name[YAudioNameMax];

	int sample_rate;
	int channels;
	int sample_size;
	int fragment_size_bytes;
	char direction;
	char allow_fragmenting;
	int num_fragments;

} YAudioModeValuesStruct;



/*
 *	Functions:
 */
extern YConnection *YOpenConnection(
	const char *start_arg,	/* Can be NULL for don't start if needed. */
	const char *con_arg	/* Can be NULL for default address and port. */
);

extern int YSetAudioModeValues(
	YConnection *con,
	int sample_size,
	int channels,
	int sample_rate,
	int direction,		/* 0 = play, only 0 is supported. */
	int allow_fragmenting,
	int num_fragments,
	int fragment_size
);
extern int YChangeAudioModePreset(YConnection *con, const char *name);
extern int YGetAudioStats(YConnection *con, YEventAudioStats *buf);

extern YAudioModeValuesStruct **YGetAudioModes(
        YConnection *con,
        int *count
);
extern void YFreeAudioModesList(YAudioModeValuesStruct **list, int count);

extern int YGetServerStats(YConnection *con, YEventServerStats *buf);

extern long YCalculateCycle(
        YConnection *con,
        int sample_rate, int channels,
        int sample_size, int fragment_size
);
extern int YSetCycle(YConnection *con, long us);
extern void YSyncAll(YConnection *con, Boolean block);

extern int YAddHost(YConnection *con, YIPUnion *ip);
extern int YRemoveHost(YConnection *con, YIPUnion *ip);

extern int YSetMixerChannel(
        YConnection *con,
        int mixer_channel_code,		/* One of YMixerCode*. */
        Coefficient value1,		/* 0 to 1. */
        Coefficient value2		/* 0 to 1. */
);
extern int YGetMixerChannel(
        YConnection *con,
        int mixer_channel_code,		/* One of YMixerCode*. */
        Coefficient *value1,		/* 0 to 1. */
        Coefficient *value2		/* 0 to 1. */
);

extern YID YStartPlaySoundObjectSimple(
        YConnection *con,
        const char *path	/* Path on disk on server's computer. */
);
extern YID YStartPlaySoundObject(
	YConnection *con,
	const char *path,	/* Path on disk on server's computer. */
	YEventSoundPlay *value
);
extern int YGetSoundObjectAttributes(
        YConnection *con,
        const char *path,
        YEventSoundObjectAttributes *buf
);
extern void YSetPlaySoundObjectValues(
	YConnection *con,
	YID yid,
	YEventSoundPlay *value
);
extern void YDestroyPlaySoundObject(YConnection *con, YID yid);

extern void YCloseConnection(YConnection *con, Boolean no_shutdown);
extern void YShutdownServer(YConnection *con);

extern int YGetNextEvent(
	YConnection *con,
	YEvent *event,
	Boolean block
);
extern void YPutBackEvent(YConnection *con, YEvent *event);


#ifdef __cplusplus
}
#endif

#endif	/* YLIB_H */
