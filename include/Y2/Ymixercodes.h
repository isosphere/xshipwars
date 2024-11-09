/*
                          Y Sound Systems

              Client to Server Mixer Codes and Definations


        Any Y client program that wants to change or fetch mixer
	values needs to #include this file and link to the libY2
	library.

        For contact and programming information, see:

        http://wolfpack.twu.net/YIFF


	Note: These mixer codes and definations are not related to
	the OSS/ALSA mixer codes (even through they appear to be
	similar).

 */

#ifndef YMIXERCODES_H
#define YMIXERCODES_H

#ifdef __cplusplus
extern "C" {
#endif

/*
 *	Mixer channel codes:
 */
#define YMixerCodeVolume	1	/* Master volume. */
#define YMixerCodeSynth		2	/* FM Synth. */
#define YMixerCodePCM		3	/* Primary PCM. */
#define YMixerCodePCM2		4	/* Secondary PCM. */
#define YMixerCodeGainIn	5	/* Input gain. */
#define YMixerCodeGainOut	6	/* Output gain. */
#define YMixerCodeBass		7
#define YMixerCodeTreble	8
#define YMixerCodeCD		9
#define YMixerCodeSpeaker	10	/* Internal PC speaker? */
#define YMixerCodeMic		11	/* Input (not always microphone). */
#define YMixerCodeRec		12	/* Recorder or AUX (input?). */
#define YMixerCodeMix		13
#define YMixerCodeLine		14
#define YMixerCodeLine1		15
#define YMixerCodeLine2		16
#define YMixerCodeLine3		17


/*
 *	Code base offset:
 *
 *	Since the YMixerCode* codes start at 1 and most indexes
 *	would start at 0. You would need to add this value to
 *	the index value to match it with the YMixerCode* code.
 *
 *	Example to convert from index to mixer_code:
 *
 *		mixer_code = index + YMixerCodeBaseOffset
 *
 *	Or from mixer_code to index:
 *
 *		index = mixer_code - YMixerCodeBaseOffset
 */
#define YMixerCodeBaseOffset	1


/*
 *	Mixer channel names (conical):
 */
#define YMixerConicalNames	{\
	"vol", "synth", "pcm", "pcm2", "igain", "ogain",  \
	"bass", "treble", "cd", "speaker", "mic", "rec",  \
	"mix", "line", "line1", "line2", "line3"          \
}


/*
 *      Total number of mixer channels:
 */
#define YTotalMixers		17


#ifdef __cplusplus
}
#endif

#endif	/* YMIXERCODES_H */
