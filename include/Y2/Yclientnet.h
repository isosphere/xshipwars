/*
                          Y Sound Systems

                 Client to Server Network Functions

	This header file is only needed to build the libY2 library,
	there is no need to #include this file in any Y client program.

        For contact and programming information, see:

        http://wolfpack.twu.net/YIFF

 */

#ifndef YCLIENTNET_H
#define YCLIENTNET_H

#include <stdio.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "Y.h"
#include "Ylib.h"


/*
 *	Standard inputs and protocol for inputs of all YNetParse*()
 *	functions:
 */
#define YCNP_STD_INPUTS_PROTO \
        YConnection *con, \
        YEvent *event, \
        const u_int8_t *buf, \
        u_int32_t chunk_length, \
        u_int16_t major_op_code, \
        u_int16_t minor_op_code

#define YCNP_STD_INPUTS \
        con, \
        event, \
        buf, \
        chunk_length, \
        major_op_code, \
        minor_op_code


extern int YNetSendAudioChangePreset(
	YConnection *con,
	const char *audio_mode_name
);
extern int YNetSendAudioChangeValues(
	YConnection *con,
        int sample_size,
        int channels,
        YDataLength sample_rate,
        int direction,
        int allow_fragmenting,
        int num_fragments,
        int fragment_size
);
extern int YNetParseAudioChange(YCNP_STD_INPUTS_PROTO);

extern int YNetSendCycleChange(YConnection *con, long cycle_us);
extern int YNetParseCycleChange(YCNP_STD_INPUTS_PROTO);

extern int YNetSendDisconnect(YConnection *con, int reason);
extern int YNetParseDisconnect(YCNP_STD_INPUTS_PROTO);

extern int YNetSendSetHost(
	YConnection *con,
	u_int16_t minor_op_code,
	YIPUnion *ip
);
extern int YNetParseSetHost(YCNP_STD_INPUTS_PROTO);

extern int YNetSendSetMixerChannel(
        YConnection *con,
        int mixer_code,
        Coefficient value1,
        Coefficient value2
);
extern int YNetSendGetMixerChannel(YConnection *con, int mixer_code);
extern int YNetParseMixerChannel(YCNP_STD_INPUTS_PROTO);

extern int YNetSendSoundPlay(
        YConnection *con,
	YID yid,
        const char *path,
        YDataPosition pos,
        YVolumeStruct *volume,
	int sample_rate,
        int repeats
);
extern int YNetParseSoundPlay(YCNP_STD_INPUTS_PROTO);

extern int YNetSendSoundKill(YConnection *con, YID yid);
extern int YNetParseSoundKill(YCNP_STD_INPUTS_PROTO);

extern int YNetSendGetSoundObjectAttributes(
        YConnection *con,
        const char *path
);
extern int YNetParseSoundObjectAttributes(YCNP_STD_INPUTS_PROTO);

extern int YNetSendShutdown(YConnection *con, int reason);
extern int YNetParseShutdown(YCNP_STD_INPUTS_PROTO);

extern int YNetSendSync(YConnection *con, long cycle_ahead_us);
extern int YNetParseSync(YCNP_STD_INPUTS_PROTO);

extern int YNetSendGetAudioStats(YConnection *con);
extern int YNetParseAudioStats(YCNP_STD_INPUTS_PROTO);

extern int YNetSendGetServerStats(YConnection *con);
extern int YNetParseServerStats(YCNP_STD_INPUTS_PROTO);

extern int YNetSendListAudioModes(YConnection *con);
extern int YNetParseListAudioModes(YCNP_STD_INPUTS_PROTO);

extern int YNetSendSetSoundObjectPlayValues(
        YConnection *con,
        YEventSoundPlay *value
);
extern int YNetParseSetSoundObjectPlayValues(YCNP_STD_INPUTS_PROTO);


extern int YNetParse(
        YConnection *con,
        YEvent *event,
        const u_int8_t *buf,
        u_int32_t chunk_length,
        u_int16_t major_op_code,
        u_int16_t minor_op_code
);

extern int YNetRecv(YConnection *con);


#ifdef __cplusplus
}
#endif

#endif	/* YCLIENTNET_H */
