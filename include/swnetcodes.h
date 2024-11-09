/*
                     Shipwars Extended Network Codes


	These are network codes used in extended CS commands.

	Each one of these codes is prefixed (after a CS_CODE_EXT)
	to each segment of data transmitted to a CS_CODE_EXT.

	Example:

	"80 30 [argument(s)...]"

	80 is CS_CODE_EXT.

 */

#ifndef SWNETCODES_H
#define SWNETCODES_H


/*
 *	Economy magic number:
 *
 *	For authenticating the data segment as actually an
 *	eco buy or sell and not just a network error.
 */
#define ECO_EXCHANGE_MAGIC_NUMBER	20918632



/* ***************************************************************** */

/*
 *      path
 */
#define SWEXTCMD_SETOCSN                10

/*
 *	ru_to_au
 */
#define SWEXTCMD_SETUNITS		11


/*
 *	object_num loc_type locked_on intercepting_object
 *	thrust_rev_state thrust_dir thrust throttle
 *	lighting hp power antimatter shield_state
 *	selected_weapon cloak_state cloak_strength visibility
 *	damage_control
 */
#define SWEXTCMD_STDOBJVALS		30

/*
 *	object_num type imageset owner size scanner_range
 *	velocity_max thrust_power turnrate hp_max power_max
 *	power_purity core_efficency antimatter_max
 *	total_weapons visibility
 */
#define SWEXTCMD_STDOBJMAXS		31

/*
 *	object_num weapon_num
 *	ocs_code emission_type amount max
 *	power range create_power
 *	delay last_used fire_sound_code flags
 */
#define SWEXTCMD_STDWEPVALS		32

/*
 *	object_num;name;empire
 */
#define SWEXTCMD_SETOBJNAME		40

/*
 *	object_num sect_x sect_y sect_z
 */
#define SWEXTCMD_SETOBJSECT		41

/* 
 *      object_num sect_x sect_y sect_z
 */
#define SWEXTCMD_SETFOBJSECT		42

/*
 *	object_num throttle
 */
#define SWEXTCMD_SETTHROTTLE            43

/*
 *	object_num selected_weapon
 */
#define SWEXTCMD_SETWEAPON		44

/*
 *	object_num arg
 *
 *	arg can be the name or number of the object to be intercepted
 *	or "#off" to turn intercept off.
 */
#define SWEXTCMD_SETINTERCEPT		45

/*
 *      object_num tar_object_num
 *
 *      tar_object_num can be -1 for unlock or -2 for lock next.
 */
#define SWEXTCMD_SETWEPLOCK		46

/*
 *	object_num shield_state shield_frequency
 */
#define SWEXTCMD_SETSHIELDS		47

/*
 *	object_num damage_control
 */
#define SWEXTCMD_SETDMGCTL		48

/*
 *	object_num cloak_state
 */
#define SWEXTCMD_SETCLOAK		49

/*
 *	object_num shield_visibility
 */
#define SWEXTCMD_SETSHIELDVIS		50

/*
 *	object_num lighting
 *
 *	Lighting is NOT a single mask value but the lighting's
 *	OR'ed mask value.
 */
#define SWEXTCMD_SETLIGHTING		51

/*
 *	object_num channel
 */
#define SWEXTCMD_SETCHANNEL		52

/*
 *    Client sends:
 *	object_num
 *
 *    Server sends:
 *      object_num
 *      credits rmu rmu_max damage_given damage_recieved kills
 */
#define SWEXTCMD_SETSCORE		54

/* 
 *	object_num engine_state
 */
#define SWEXTCMD_SETENGINE		55


/*
 *	object_num
 */
#define SWEXTCMD_REQNAME                60

/*
 *	object_num
 */
#define SWEXTCMD_REQSECT		61


/*
 *	object_num
 *	sect_x sect_y sect_z
 *	x y z
 *	heading pitch bank
 *	velocity
 *	velocity_heading velocity_pitch velocity_bank
 *	freq yield
 */
#define SWEXTCMD_FIREWEAPON		100

/*
 *	src_obj tar_obj
 */
#define SWEXTCMD_TRACTORBEAMLOCK	101

/*
 *	src_obj tar_obj bearing channel
 */
#define SWEXTCMD_HAIL			102

/*
 *      src_obj tar_obj bearing channel;message
 */
#define SWEXTCMD_COMMESSAGE		103

/*
 *	src_obj tar_obj
 */
#define SWEXTCMD_WORMHOLEENTER		104

/*
 *	src_obj tar_obj
 *
 *	Note: client sends this to server, server sends back a forward
 *	which the client will handle and do the actual bamf.
 */
#define SWEXTCMD_ELINKENTER		105

/*
 *	src_obj tar_obj
 *
 *	When server sends to client, it is considered to be a notify of
 *	a weapon being disarmed.
 */
#define SWEXTCMD_WEPDISARM		106


/*
 *	wep_obj tar_obj total_damage bearing
 *	structure_damage shield_damage
 *
 *	Bearing is relative from tar_obj to wep_obj.
 */
#define SWEXTCMD_NOTIFYHIT		120

/*
 *	reason_code destroyed_obj destroyer_obj
 *	destroyer_obj_owner
 *
 *	If client recieves this and destroyed_obj matches the
 *	client's player object number, then a CS_CODE_LOGOUT
 *	will probably come right after. ;)
 */
#define SWEXTCMD_NOTIFYDESTROY		121


/*
 *	object_num
 */
#define SWEXTCMD_ECO_REQVALUES		140

/*
 *	object_num flags tax_general tax_friend tax_hostile
 */
#define SWEXTCMD_ECO_SETVALUES		141

/*
 *	object_num sell_price buy_price
 *	product_amount product_max;product_name
 */
#define SWEXTCMD_ECO_SETPRODUCTVALUES	142

/*
 *      customer_obj proprietor_obj amount magic_number;product_name
 */
#define SWEXTCMD_ECO_BUY		143

/*
 *      customer_obj proprietor_obj amount magic_number;product_name
 */
#define SWEXTCMD_ECO_SELL		144

/*
 *      customer_obj proprietor_obj amount magic_number;product_name
 */
#define SWEXTCMD_ECO_TRADE		145



/*
 *	object_num
 *	type isref_num size
 *	sect_x sect_y sect_z
 *	x, y, z
 *	heading, pitch, bank
 */
#define SWEXTCMD_STARCHART_ADD_OBJECT	160

/*
 *	object_num;name
 */
#define SWEXTCMD_STARCHART_SET_OBJECT_NAME	161

/*
 *      object_num;empire
 */
#define SWEXTCMD_STARCHART_SET_OBJECT_EMPIRE	162

/*
 *	object_num
 */
#define SWEXTCMD_STARCHART_SET_OBJECT_RECYCLE	163


#endif /* SWNETCODES_H */
