/*
                           XShipWars Client Keymaps

 */

#ifndef KEYMAP_H
#define KEYMAP_H

#include <stdio.h>
#include <sys/types.h>

#include "../include/osw-x.h"


/*
 *	Keymap codes (code corresponds to index num in xsw_keymap).
 */
#define XSW_KM_HELP			0
#define XSW_KM_EXIT			1

#define XSW_KM_TURN_LEFT		2
#define XSW_KM_TURN_RIGHT		3
#define XSW_KM_THROTTLE_INC		4
#define XSW_KM_THROTTLE_DEC		5
#define XSW_KM_THROTTLE_IDLE		6
#define XSW_KM_FIRE_WEAPON		7
#define XSW_KM_OMNI_DIR_THRUST		8
#define XSW_KM_EXTERNAL_DAMPERS		9

#define XSW_KM_LIGHTS_VECTOR		10
#define XSW_KM_LIGHTS_STROBE		11
#define XSW_KM_LIGHTS_LUMINATION	12

#define XSW_KM_WEAPON_FREQ		13
#define XSW_KM_SHIELD_STATE		14
#define XSW_KM_SHIELD_FREQ		15
#define XSW_KM_DMGCTL_TOGGLE		16
#define XSW_KM_CLOAK_STATE		17
#define XSW_KM_SET_INTERCEPT		18
#define XSW_KM_HAIL			19
#define XSW_KM_SET_CHANNEL		20

#define XSW_KM_VIEWSCREEN_ZIN		21
#define XSW_KM_VIEWSCREEN_ZOUT		22
#define XSW_KM_VIEWSCREEN_ZAUTO		23

#define XSW_KM_SCANNER_TOGGLE		24
#define XSW_KM_SCANNER_ZIN		25
#define XSW_KM_SCANNER_ZOUT		26
#define XSW_KM_SCANNER_ZMIN		27
#define XSW_KM_SCANNER_ZMAX		28
#define XSW_KM_SCANNER_ORIENT		29

#define XSW_KM_ENGINE_STATE		30
#define XSW_KM_THROTTLE_MODE		31

#define XSW_KM_WEAPONS_LOCK		32
#define XSW_KM_WEAPONS_UNLOCK		33
#define XSW_KM_WEAPONS_ONLINE		34
#define XSW_KM_SELECT_WEAPONPREV	35
#define XSW_KM_SELECT_WEAPONNEXT	36
#define XSW_KM_SELECT_WEAPON1		37
#define XSW_KM_SELECT_WEAPON2		38
#define XSW_KM_SELECT_WEAPON3		39
#define XSW_KM_SELECT_WEAPON4		40
#define XSW_KM_SELECT_WEAPON5		41
#define XSW_KM_SELECT_WEAPON6		42
#define XSW_KM_SELECT_WEAPON7		43
#define XSW_KM_SELECT_WEAPON8		44
#define XSW_KM_SELECT_WEAPON9		45
#define XSW_KM_SEND_MESSAGE		46

#define XSW_KM_VIEWSCREEN_MARKINGS	47
#define XSW_KM_VIEWSCREEN_LABELS	48
#define XSW_KM_ENERGY_SAVER_MODE	49

#define XSW_KM_NET_INTERVAL_DEC		50
#define XSW_KM_NET_INTERVAL_INC		51
#define XSW_KM_MESG_SCROLL_UP		52
#define XSW_KM_MESG_SCROLL_DOWN		53

#define XSW_KM_MAP_ECONOMY		54
#define XSW_KM_MAP_STARCHART		55

#define XSW_KM_CONNECT			56
#define XSW_KM_DISCONNECT		57
#define XSW_KM_REFRESH			58
#define XSW_KM_CONNECTLAST		59

#define XSW_KM_CLIENT_CMD		60
#define XSW_KM_SERVER_CMD		61

#define XSW_KM_SCREEN_SHOT		62


/*
 *	Keymap conical names.
 */
static char *xsw_keymap_name[] = {
        "Help",
        "Exit",

        "TurnLeft",
        "TurnRight",
        "ThrottleIncrease",
        "ThrottleDecrease",
	"ThrottleIdle",
        "FireWeapon",
        "OmniDirectionalThrust",
	"ExternalDampers",

        "LightsVector",
        "LightsStrobe",
        "LightsLumination",

        "WeaponFreq",
        "ShieldState",
        "ShieldFreq",
        "DamageControl",
        "CloakState",
        "SetIntercept",
        "Hail",
        "SetChannel",

        "ViewscreenZoomIn",
        "ViewscreenZoomOut",
        "ViewscreenAutoZoom",

        "ScannerToggle",
        "ScannerZoomIn",
        "ScannerZoomOut",
        "ScannerZoomMin",
        "ScannerZoomMax",
        "ScannerOrient",
        
        "EngineState",
        "ThrottleMode",

        "WeaponsLock",
        "WeaponsUnlock",

        "WeaponsOnline",
        "SelectWeaponPrev",
	"SelectWeaponNext",
        "SelectWeapon1",
        "SelectWeapon2",
        "SelectWeapon3",
        "SelectWeapon4",  
        "SelectWeapon5",
        "SelectWeapon6",
        "SelectWeapon7",
        "SelectWeapon8",
        "SelectWeapon9",

        "SendMessage",

        "ViewscreenMarkings",
        "ViewscreenLabels",
        "EnergySaverMode",

        "NetIntervalDecrement",
        "NetIntervalIncrement",

	"MessageScrollUp",
	"MessageScrollDown",

	"MapEconomy",
	"MapStarChart",

        "NetConnect",
        "NetDisconnect",
        "NetRefresh",
        "NetConnectLast",

	"ClientCommand",
        "ServerCommand",

	"ScreenShot"
};

/*
 *	Total keymap entries, use number of keymap_name members
 *	as total.
 */
#define TOTAL_XSW_KEYMAPS	(sizeof(xsw_keymap_name) / sizeof(char *))



/*
 *      Keymap alias names.
 */
#define XSW_KEYMAP_ALIAS	{	\
        "Help",				\
        "Exit",				\
					\
        "Turn Left",			\
        "Turn Right",			\
        "Throttle Increase",		\
        "Throttle Decrease",		\
	"Throttle Idle",		\
        "Fire Weapon",			\
        "Omni Directional Thrust",	\
	"External Dampers",		\
					\
        "Lights Vector",		\
        "Lights Strobe",		\
        "Lights Lumination",		\
					\
        "Weapon Freq",			\
        "Shield State",   		\
        "Shield Freq",			\
        "Damage Control",		\
        "Cloak State",			\
        "Set Intercept",		\
        "Hail",				\
        "Set Channel",			\
					\
        "Viewscreen Zoom In",		\
        "Viewscreen Zoom Out",		\
        "Viewscreen Auto Zoom",		\
					\
        "Scanner Toggle",		\
        "Scanner Zoom In",		\
        "Scanner Zoom Out",		\
        "Scanner Zoom Min",		\
        "Scanner Zoom Max",		\
        "Scanner Orient",		\
					\
        "Engine State",			\
        "Throttle Mode",		\
					\
        "Weapons Lock",			\
        "Weapons Unlock",		\
					\
        "Weapons Online",		\
	"Select Weapon Prev",		\
	"Select Weapon Next",		\
        "Select Weapon 1",		\
        "Select Weapon 2",		\
        "Select Weapon 3",		\
        "Select Weapon 4",  		\
        "Select Weapon 5",		\
        "Select Weapon 6",		\
        "Select Weapon 7",		\
        "Select Weapon 8",		\
        "Select Weapon 9",		\
					\
        "Send Message",			\
					\
        "Viewscreen Markings",		\
        "Viewscreen Labels",		\
        "Energy Saver Mode", 		\
					\
        "Net Interval Decrement",	\
        "Net Interval Increment",	\
					\
        "Message Scroll Up",		\
        "Message Scroll Down",		\
					\
	"Map Economy",			\
	"Map Star Chart",		\
					\
        "Net Connect",			\
        "Net Disconnect",		\
        "Net Refresh",			\
        "Net Connect Last",		\
					\
	"Client Command",		\
	"Server Command",		\
					\
	"Screen Shot"			\
}


/*
 *	Keymap description:
 */
#define XSW_KEYMAP_ITEM_DESC	{	\
        "Prints help listing of basic mapped keys and\n\
their respective functions.", \
        "Exits the program (requires comfermation).", \
 \
        "Turns left, or if omni directional thrust is on\n\
then applies thrust towards starboard.", \
        "Turns right, or if omni directional thrust is on\n\
then applies thrust towards port.", \
        "Increases throttle.", \
        "Decreses throttle.", \
	"Sets throttle to idle position.", \
        "Fire/use selected device/equipment/weapon.", \
        "Switches to omni directional thrust mode\n\
when held down.", \
	"Activates external dampers.", \
 \
        "Toggles vector lights on/off.", \
        "Toggles strobe lights on/off.", \
        "Toggles external lumination lights on/off.", \
 \
        "Changes energy weapon's modulation frequency.", \
        "Toggles shield generators on/off.", \
        "Changes shield modulation frequency.", \
        "Toggles damage control on/off.", \
        "Toggles cloaking device on/off.", \
        "Prompt for intercept destination.", \
        "Sends a hail on the current com channel.", \
        "Sets com channel.", \
 \
        "Zooms the viewscreen in.", \
        "Zooms the viewscreen out.", \
        "Have the viewscreen automatically adjust\n\
the zoom as needed.", \
 \
        "Toggles the scanner on/off (this function is\n\
obsolete).", \
        "Zooms the scanner in.", \
        "Zooms the scanner out.", \
        "Zooms the scanner to match the zoom of the\n\
viewscreen.", \
        "Zooms the scanner to its maximum range.", \
        "Toggles scanner orientation; galactic core\n\
orientation/vessel orientation.", \
 \
        "Initialize/shutdown engines.", \
        "Switches throttle scope modes; normal scope\n\
/bi-directional scope. This is often used with\n\
the joystick.", \
 \
        "Locks on the next object within scanner range.", \
        "Unlocks weapons on currently locked object or\n\
unlocks tractor beam if the shift key\n\
is held down", \
 \
        "Toggles all weapons online/offline (often used\n\
as a master padlock to prevent accidental\n\
firing).", \
	"Selects the previous weapon.", \
	"Selects the next weapon.", \
        "Selects weapon 1.", \
        "Selects weapon 2.", \
        "Selects weapon 3.", \
        "Selects weapon 4.", \
        "Selects weapon 5.", \
        "Selects weapon 6.", \
        "Selects weapon 7.", \
        "Selects weapon 8.", \
        "Selects weapon 9.", \
 \
        "Prompts for a message to be sent on the\n\
current com channel.", \
 \
        "Displays/hides viewscreen markings (such as\n\
the vessel direction arrow and scanner\n\
lock cursor).", \
        "Cycles through amount of labeling displayed on\n\
the viewscreen.", \
        "Toggles `energy saver mode' on/off (useful for\n\
conserving energy and network bandwidth\n\
while idling).", \
 \
        "Decrease the network streaming interval,\n\
thus increasing the network load\n\
(useable only when auto interval is off).", \
        "Increases the network streaming inverval,\n\
thus decreasing the network load\n\
(useable only when auto interval is off).", \
 \
        "Scrolls the bridge window's message box up.", \
        "Scrolls the bridge window's message box down.", \
 \
	"Maps the Economy window.", \
	"Maps the Star Chart window.", \
 \
        "Prompt for connection to universe.", \
        "Disconnects from current universe (if\n\
connected).", \
        "Refreshes local universe listing and other\n\
related information/resources (only when\n\
connected).", \
        "Reconnects to the last universe using the\n\
current login information.", \
 \
        "Prompts for a command to be executed on the\n\
client.", \
        "Prompts for a command to be executed on the\n\
server.", \
 \
	"Take screen shot." \
}


/* Keymap structure. */
typedef struct {

        keycode_t keycode;

} xsw_keymap_struct;
extern xsw_keymap_struct xsw_keymap[TOTAL_XSW_KEYMAPS];




#endif	/* KEYMAP_H */
