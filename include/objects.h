/*
                            ShipWars Objects

 	Contents here should be indiginous to both the client and server
 	with a few exceptions (i.e. removed X support for the server).

 */


#ifndef OBJECTS_H
#define OBJECTS_H

#include <stdio.h>
#include <sys/types.h>

#include "os.h"		/* Apparently needed for some OSes. */

#include "eco.h"


/*
 *   Garbage name string:
 */
#define XSW_OBJ_GARBAGE_NAME	"*Garbage*"

/*
 *   Default XSW Object Permission IDs:
 *
 *	Lower settings give higher permissions. Default values should be
 *	a high number. 5 is a good number for minimal clearance.
 *
 *	Valid values range from 0 to 5.
 */
#define DEFAULT_UID	5
#define DEFAULT_GID	5


/*
 *   Name and Password Lengths:
 *
 *      Includes null terminating byte!    A length of 33 will allow
 *	32 characters plus the null terminating byte.
 *
 */
#define XSW_OBJ_NAME_MAX	33
#define XSW_OBJ_PASSWORD_MAX	33
#define XSW_OBJ_EMPIRE_MAX	9


/*
 *    Object Allocate Ahead:
 *
 *	Allocates objects in 'chunks' of this many.
 *	Recommend value 10.
 */
#define OBJECT_ALLOCATE_AHEAD 10


/*
 *   Bad name characters:
 *
 *	This is a list of characters not permitted in names.
 */
#define INVALID_NAME_CHARACTERS ";,=\n\r\t#"


/*
 *	Maximum Values:
 */
#define MAX_OBJECTS		100000		/* Per server/client. */
#define MAX_WEAPONS		4		/* Per XSW Object. */
#define MAX_CREDITS		2147483647


/*
 *   Default Empire Code:
 */
#define XSW_DEF_EMPIRE_STR	"IND"


/*
 *   Default Shield Frequency:
 *
 *	Newly created or reset objects will have their shield
 *	frequency reset to this value.  This value needs to be
 *	between SWR_FREQ_MIN and SWR_FREQ_MAX defined in reality.h.
 */
#define XSW_DEF_SHIELD_FREQ	180.20


/*
 *   Default Communications Channel:
 *
 *	Newly created or reset objects will have their com
 *	frequency reset to this value.  This value is * 100
 *	of the actual value and needs to be between SWR_FREQ_MIN
 *	and SWR_FREQ_MAX defined in reality.h.
 */
#define XSW_DEF_COM_CHANNEL	31000


/*
 *   Maximum Tractored Objects:
 *
 *	The maximum number of objects that can be tractor'ed by one
 *	object.
 */
#define MAX_TRACTORED_OBJECTS	3
#define MAX_TRACTOR_BEAM_LEN	2.000	/* In XSW Real Units. */



/*
 *   Core XSW Object Type Codes and Names:
 *
 *      These are codes (data type long) to identify each XSW
 *      Object.   They are stored in each object's xsw_object[].type.
 *
 *      XSW_OBJ_TYPE_ERROR is similar to XSW_OBJ_TYPE_GARBAGE
 *      except that it means that xsw_object[] should not be used
 *      (think of it as a bad sector on a disk).
 *
 *      XSW_OBJ_TYPE_GARBAGE is the most common type, it means that
 *      that xsw_object[] contains nothing of value and may be used.
 */
#define XSW_OBJ_TYPE_ERROR              -1  /* Do not use. */
#define XSW_TYPE_NAME_ERROR		"Error"

#define XSW_OBJ_TYPE_GARBAGE            0   /* Free to re/use. */
#define XSW_TYPE_NAME_GARBAGE		"Garbage"

#define XSW_OBJ_TYPE_STATIC             1   /* Primitive, not move. */
#define XSW_TYPE_NAME_STATIC		"Static"

#define XSW_OBJ_TYPE_DYNAMIC            2   /* Moves. */
#define XSW_TYPE_NAME_DYNAMIC		"Dynamic"

#define XSW_OBJ_TYPE_CONTROLLED         3   /* AI Controllable. */
#define XSW_TYPE_NAME_CONTROLLED	"Controlled"

#define XSW_OBJ_TYPE_PLAYER             4   /* Controlled by player. */
#define XSW_TYPE_NAME_PLAYER		"Player"

#define XSW_OBJ_TYPE_WEAPON             5   /* Regular weapons fire object; */
                                            /* torpedo, pulse, etc. */
#define XSW_TYPE_NAME_WEAPON		"Weapon"

#define XSW_OBJ_TYPE_STREAMWEAPON       6   /* Stream weapons; phazer beams. */
#define XSW_TYPE_NAME_STREAMWEAPON	"StreamWeapon"

#define XSW_OBJ_TYPE_SPHEREWEAPON	7   /* Expanding sphere. */
#define XSW_TYPE_NAME_SPHEREWEAPON	"SphereWeapon"

#define XSW_OBJ_TYPE_HOME               10  /* Starbases, planets, etc. */
#define XSW_TYPE_NAME_HOME		"Home"

#define XSW_OBJ_TYPE_AREA               11  /* Space weather phoemonia;  */
                                            /* nebulas, plasma storms... */
#define XSW_TYPE_NAME_AREA		"Area"

#define XSW_OBJ_TYPE_ANIMATED           12  /* Explosions. */
#define XSW_TYPE_NAME_ANIMATED		"Animated"

#define XSW_OBJ_TYPE_WORMHOLE		13  /* Formerly known as gates. */
#define XSW_TYPE_NAME_WORMHOLE		"Wormhole"

#define XSW_OBJ_TYPE_ELINK		14  /* External server link. */
#define XSW_TYPE_NAME_ELINK		"Elink"

/* more to come... */


/*
 *   Location Type Codes:
 *
 *	These are location types which are used by objects of type
 *	XSW_OBJ_TYPE_AREA to determine what kind of 'area' they
 *	produce.
 */
#define XSW_LOC_TYPE_SPACE		0
#define XSW_LOC_TYPE_NAME_SPACE		"Space"

#define XSW_LOC_TYPE_NOTIFY		1
#define XSW_LOC_TYPE_NAME_NOTIFY	"Notify"

#define XSW_LOC_TYPE_NEBULA		2
#define XSW_LOC_TYPE_NAME_NEBULA	"Nebula"


/*
 *   IFF (Is Friend or Foe) codes:
 */
#define IFF_UNKNOWN	0	/* Undeterminable/unknown/error. */
#define IFF_FRIENDLY	1	/* Friend. */
#define IFF_HOSTILE	2	/* Foe. */

/*
 *   Engine states:
 */
#define ENGINE_STATE_NONE	-1
#define ENGINE_STATE_OFF	0
#define ENGINE_STATE_STARTING	1
#define ENGINE_STATE_ON		2


/*
 *   Shield generator states:
 */
#define SHIELD_STATE_NONE	-1
#define SHIELD_STATE_DOWN	0
#define SHIELD_STATE_UP		1


/*
 *   Cloak states:
 */
#define CLOAK_STATE_NONE	-1
#define CLOAK_STATE_DOWN	0
#define CLOAK_STATE_UP		1

/*
 *   Damage control states:
 */
#define DMGCTL_STATE_OFF	0
#define DMGCTL_STATE_ON		1


/*
 *   Weapon emission type codes:
 *
 *      These determine the firing style of the weapon.
 */
#define WEPEMISSION_STREAM       0      /* Supply dependant on power left. */
#define WEPEMISSION_PROJECTILE   1      /* Limited supply. */
#define WEPEMISSION_PULSE        2      /* Supply dependant on power left. */

#define WEPEMISSION_NAME_STREAM		"Stream"
#define WEPEMISSION_NAME_PROJECTILE	"Projectile"
#define WEPEMISSION_NAME_PULSE		"Pulse"

/*
 *   Weapon flags.
 */
#define XSW_WEP_FLAG_NO_FIRE_SOUND	(1 << 2)
#define XSW_WEP_FLAG_FIXED		(1 << 3)	/* No rotate. */

#define XSW_WEP_FLAG_NAME_NO_FIRE_SOUND	"NoFireSound"
#define XSW_WEP_FLAG_NAME_FIXED		"Fixed"


/*
 *    Lighting flags:
 *
 *	Vector lights, strobes, etc.
 *
 *	Set in member lighting of each object.
 */
#define XSW_OBJ_LT_VECTOR	(1 << 0)
#define XSW_OBJ_LT_STROBE	(1 << 1)
#define XSW_OBJ_LT_LUMINATION	(1 << 2)	/* Head lights. */

#define XSW_OBJ_LT_NAME_VECTOR		"VECTOR"
#define XSW_OBJ_LT_NAME_STROBE		"STROBE"
#define XSW_OBJ_LT_NAME_LUMINATION	"LUMINATION"

/*
 *	AI (controllable objects) AI flags:
 *
 *	Set in member ai_flags of each object.
 */
#define XSW_OBJ_AI_FOLLOW_FRIEND	(1 << 1)
#define XSW_OBJ_AI_FOLLOW_UNKNOWN	(1 << 2)
#define XSW_OBJ_AI_FOLLOW_HOSTILE	(1 << 3)
#define XSW_OBJ_AI_FOLLOW_ANY		(XSW_OBJ_AI_FOLLOW_FRIEND | \
					XSW_OBJ_AI_FOLLOW_UNKNOWN | \
					XSW_OBJ_AI_FOLLOW_HOSTILE)
#define XSW_OBJ_AI_FIRE_FRIEND	(1 << 8)
#define XSW_OBJ_AI_FIRE_UNKNOWN	(1 << 9)
#define XSW_OBJ_AI_FIRE_HOSTILE	(1 << 10)
#define XSW_OBJ_AI_FIRE_ANY	(XSW_OBJ_AI_FIRE_FRIEND | \
				XSW_OBJ_AI_FIRE_UNKNOWN | \
				XSW_OBJ_AI_FIRE_HOSTILE)

#define XSW_OBJ_AI_NAME_FOLLOW_FRIEND	"FOLLOW_FRIEND"
#define XSW_OBJ_AI_NAME_FOLLOW_UNKNOWN	"FOLLOW_UNKNOWN"
#define XSW_OBJ_AI_NAME_FOLLOW_HOSTILE	"FOLLOW_HOSTILE"

#define XSW_OBJ_AI_NAME_FIRE_FRIEND   "FIRE_FRIEND"
#define XSW_OBJ_AI_NAME_FIRE_UNKNOWN  "FIRE_UNKNOWN"
#define XSW_OBJ_AI_NAME_FIRE_HOSTILE  "FIRE_HOSTILE"


/*
 *	Universe header:
 */
#define UNV_TITLE_MAX 128
typedef struct {

	char title[UNV_TITLE_MAX];
	char version[UNV_TITLE_MAX];	/* Version string. */

	char isr[PATH_MAX + NAME_MAX];	/* Image set referances. */
	char ocsn[PATH_MAX + NAME_MAX];	/* Object create scripts. */
	char ss[PATH_MAX + NAME_MAX];	/* Sound scheme. */

	/* Unit conversions. */
	double ru_to_au;

	/* Lost and found owner. */
	int lost_found_owner;

	/* New player start position. */
	long	player_start_sect_x,
		player_start_sect_y,
		player_start_sect_z;

	double	player_start_x,
		player_start_y,
		player_start_z;

	double	player_start_heading,
		player_start_pitch,
		player_start_bank;

	/* Guest start position. */
	long	guest_start_sect_x,
		guest_start_sect_y,
		guest_start_sect_z;

	double	guest_start_x,
		guest_start_y,
		guest_start_z;

	double	guest_start_heading,
		guest_start_pitch,
		guest_start_bank;

} unv_head_struct;


/*
 *	XSW Object data types:
 */
typedef u_int64_t xswo_option_flags_t;	/* Client/server proprietery flags. */

typedef u_int64_t xswo_weapon_flags_t;	/* Weapon flags. */
typedef u_int8_t xswo_lighting_t;	/* Object lighting flags. */

typedef double xswo_credits_t;		/* Universal credits (money). */
typedef double xswo_rmu_t;		/* Raw material units. */
typedef u_int64_t xswo_ai_flags_t;	/* Artificial intel (controlled) flags. */


/*
 *	Vector compoent structure:
 */
typedef struct {

	double i, j, k;

} xsw_vector_compoent_struct;


/*
 *	Economy product structure:
 */
typedef struct {

	char		name[ECO_PRODUCT_NAME_MAX];

	/* Cost of product per 1 unit. */
        xswo_credits_t	sell_price,	/* Price if being bought from customer. */
                        buy_price;	/* Price if being sold to customer. */
	double 		amount,		/* How much in sellable stock. */
			amount_max;	/* Maximum possible in stock. */

} xsw_ecoproduct_struct;

/*
 *	Economy data structure:
 */
typedef struct
{
	eco_flags_t flags;		/* General flags. */

	/* Coefficient value (ie 1.2 would be 20%). */
	double	tax_general,
		tax_friend,
		tax_hostile;

	xsw_ecoproduct_struct **product;
	int total_products;

} xsw_ecodata_struct;


/*
 *	Score structure:
 */
typedef struct
{
	xswo_credits_t credits;	/* Universal credits (money). */
	xswo_rmu_t	rmu,		/* Raw material units. */
			rmu_max;	/* Maximum raw material units. */
	double damage_given;
	double damage_recieved;
	int kills;

} xsw_score_struct;


/*
 *	Permission structure:
 */
typedef struct
{
	int uid;		/* User ID, 0 is highest. */
	int gid;		/* Group ID, 0 is highest. */

} permission_struct;


/*
 *	Weapons Structure:
 *
 *      Each XSW Object structure has this.
 */
typedef struct
{
	xswo_weapon_flags_t flags;

	int ocs_code;	 /* Object create script to use to fire this weapon. */
	int emission_type;  /* Type of emission (stream, pulse, projectile) */

	int amount;	/* Current amount (projectile weapons only). */
	int max;	/* Total amount (projectile weapons only). */

	double power;	/* Amount of damage to be caused per unit hit. */
	long range;	/* Determines WEPEMISSION_STREAM type weapon
			 * length (size) and inrange of target for AI
			 * controlled objects. In XSW Screen (pixel) units.
			 */

        double create_power;	/* Power needed to create unit fire. */

        long delay;          /* In milliseconds, time between uses. */
        long last_used;      /* In milliseconds. time last used. */

	/* Sound codes. */
	int	use_sound_code,		/* Just before firing. */
		fire_sound_code,	/* Fire sound. */
		hit_sound_code,		/* When an object is hit. */
		recover_sound_code;	/* When dead wep is recovered. */

        char name[XSW_OBJ_NAME_MAX];

} xsw_weapons_struct;



/*
 *	Animations Structure:
 *
 *      Each XSW Object structure has this.
 */
typedef struct
{
	long interval;		/* Milliseconds between frames. */
	long last_interval;	/* Last millitime frame was inc. */

	int current_frame;	/* Current frame number. */
	int total_frames;       /* Total number of frames. */

	int cycle_count;	/* Times repeated. */
	int cycle_times;        /* Total times to repeat. */

} xsw_animation_struct;



/*
 *	Core XSW Object Structure:
 *
 *	All XSW Objects have these members.
 */
typedef struct
{
        int type;               /* Type of object. */

	xswo_option_flags_t	client_options,
				server_options;

	char name[XSW_OBJ_NAME_MAX];
	char password[XSW_OBJ_PASSWORD_MAX];
	char empire[XSW_OBJ_EMPIRE_MAX];
	char *elink;		/* For elink objects. */

	long last_updated;	/* In milliseconds since last update
				 * by server or client
				 */

	int loc_type;		/* Type of location object is in. */
	int imageset;		/* Image referance code. */
	int owner;		/* Object that owns this object. */
	long size;		/* Radius of object size in screen units. */

	int locked_on;		/* Scanning/locked on object.
				 * Can be -1 for unlocked.
				 */

	int intercepting_object;	/* Object this object is
					 * Intercepting. Can be -1 for
					 * not intercepting.
					 */

	double scanner_range;		/* Radius in XSW real units. */

	long	sect_x,		/* Current sector. */
		sect_y,
		sect_z;

	double	x,		/* Coordinate position in sector. */
		y,
		z;

	double	heading,	/* In radians. */
		pitch,		/* In radians (not implmented yet). */
		bank;		/* In radians (not implmented yet). */
	xsw_vector_compoent_struct attitude_vector_compoent;

	double velocity;
	double velocity_max;
	double	velocity_heading,	/* In radians. */
		velocity_pitch,		/* In radians (not implmented yet). */
		velocity_bank;		/* In radians (not implmented yet). */
	xsw_vector_compoent_struct momentum_vector_compoent;

	double thrust_dir;	/* In radians (for omni directional thrust). */
	double thrust;		/* throttle * thrust_power */
	double thrust_power;	/* Full power of thrust. */
	double throttle;	/* 0.0 to 1.0. */
	char engine_state;	/* One of ENGINE_STATE_*. */
	double turnrate;	/* In radians per cycle. */

	xswo_lighting_t lighting;	/* Vector lights, strobes, etc switches. */

	double	hp,
		hp_max;

	double	power,
		power_max;
	double power_purity;	/* 0.0 to 1.0 */

	double core_efficency;	/* In power units per cycle. */

	double	antimatter,
		antimatter_max;

	int shield_state;	/* One of SHIELD_STATE_*. */
	double shield_frequency;	/* In KHz. */

	int selected_weapon;
	int total_weapons;	/* May not be greater than MAX_WEAPONS. */

	long birth_time;	/* In milliseconds since midnight. */
	long lifespan;		/* How old before dies, less than 0 means
				 * never dies.
				 */
	int creation_ocs;	/* The OCS used to create this object
				 * 0 for none (used for weapons reclaiming).
				 */
 
	int cloak_state;	/* One of CLOAK_STATE_*. */
	double cloak_strength;	/* 0.0 to 1.0, when cloak is on,
				 * cloak_strength reduces visibility.
				 */
	double visibility;	/* Normal visibility: 0.0 to 1.0. */
	double cur_visibility;	/* Actual current visibility. */

	double shield_visibility;	/* When greater than 0,
					 * shields are visibable.
					 */

	int damage_control;	/* One of DMGCTL_STATE_* */

	int com_channel;	/* In <actual> * 100 units. */


	/* Artificial intel flags (for type XSW_OBJ_TYPE_CONTROLLED only). */
	xswo_ai_flags_t ai_flags;


	/* Objects being tractored. */
	int *tractored_object;
	int total_tractored_objects;


	/* Permission structure. */
	permission_struct permission;

	/* Animation structure. */
	xsw_animation_struct animation;

	/* Score structure, dynamically allocated. */
	xsw_score_struct *score;

	/* Weapons structure, limited to MAX_WEAPONS. */
	xsw_weapons_struct **weapons;

	/* Economy structure. */
	xsw_ecodata_struct *eco;

} xsw_object_struct;


#endif /* OBJECTS_H */
