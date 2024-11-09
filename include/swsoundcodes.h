/*
                   XShipWars: Standard Sound Codes

	These are standardized sound codes.

	For the client, each sound code corresponds
	to a sound scheme code number on the client.

 */


#ifndef SWSOUNDCODES_H
#define SWSOUNDCODES_H


/* Standard sounds, 0 to 9. */
#define SOUND_CODE_DEFAULT		0	/* All purpose default sound
                                                 * when no other sound is
                                                 * available.
						 */
#define SOUND_CODE_BUTTONPRESS		1	/* Button press. */
#define SOUND_CODE_STD_ERROR		2	/* Standard error. */
#define SOUND_CODE_XSW_LOGO		3	/* "The XSW Sound" */
#define SOUND_CODE_MENU_HIGHLIGHT	4	/* Highlight item on menu. */
#define SOUND_CODE_MENU_SELECT		5	/* Select item on menu. */


/* Beeps and blips, 10 to 29. */
#define SOUND_CODE_ATTENTION_BEEP1	10
#define SOUND_CODE_ATTENTION_BEEP2      11
#define SOUND_CODE_ATTENTION_BEEP3      12

#define SOUND_CODE_INCOMING_MESG_BEEP	15
#define SOUND_CODE_SCAN_BEEP		16
#define SOUND_CODE_CONTACTS_BEEP	17
#define SOUND_CODE_HAIL_OUTGOING	18
#define SOUND_CODE_HAIL_INCOMING	19

/* Operations sounds. */
#define SOUND_CODE_ENGINES_ON		20
#define SOUND_CODE_ENGINES_OFF		21
#define SOUND_CODE_CLOAK_UP		22
#define SOUND_CODE_CLOAK_DOWN		23


/* Fire weapons. */
#define SOUND_CODE_FIRE_STREAM          30
#define SOUND_CODE_FIRE_PULSE           31
#define SOUND_CODE_FIRE_PROJECTILE      32

/* Colissions and explosions. */
#define SOUND_CODE_HIT_SHIELDS          53
#define SOUND_CODE_HIT_STRUCTURE        54
#define SOUND_CODE_OHIT_SHIELDS		55
#define SOUND_CODE_OHIT_STRUCTURE	56


/* Background mood music. */
#define SOUND_CODE_BKG_STANDARD		100
#define SOUND_CODE_BKG_EXPLORING	101
#define SOUND_CODE_BKG_FIGHTING		102
#define SOUND_CODE_BKG_MYSTY		103	/* Ie, in nebula. */
#define SOUND_CODE_BKG_MAINMENU		104
#define SOUND_CODE_BKG_GOTDESTROYED	105
/* More tom come... */


#endif /* SWSOUNDCODES_H */
