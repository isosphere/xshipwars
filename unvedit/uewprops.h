/*
            Universe Editor Window: Properties Pane

 */




#ifndef UEWPROPS_H
#define UEWPROPS_H


/* Height of each property prompt in pixels. */
#define UEW_PROP_PROMPT_HEIGHT	30


/*
 *	Property parameter names.
 */
#define PROP_NAME	{				\
	"Name",				/* 0 */ 	\
	"Password",					\
	"Empire",					\
	"ELink",					\
	"LastUpdated",					\
	"Type",						\
	"LocType",					\
	"ImageSet",					\
	"Owner",					\
	"Size",						\
	"LockedOn",			/* 10 */	\
	"InterceptingObject",				\
	"ScannerRange",					\
	"SectX",					\
	"SectY",					\
	"SectZ",					\
	"X",						\
	"Y",						\
	"Z",						\
	"Heading",					\
	"Pitch",			/* 20 */	\
	"Bank",						\
	"Velocity",					\
	"VelocityMax",					\
	"VelocityHeading",				\
	"VelocityPitch",				\
	"VelocityBank",					\
	"ThrustDir",					\
	"Thrust",					\
	"ThrustPower",					\
	"Throttle",			/* 30 */	\
	"EngineState",					\
	"TurnRate",					\
	"Lighting",					\
	"Hp",						\
	"HpMax",					\
	"Power",					\
	"PowerMax",					\
	"PowerPurity",					\
	"CoreEfficency",				\
	"Antimatter",			/* 40 */	\
	"AntimatterMax",				\
	"ShieldState",					\
	"ShieldFrequency",				\
	"SelectedWeapon",				\
	"TotalWeapons",					\
	"BirthTime",					\
	"LifeSpan",					\
	"CloakState",					\
	"CloakStrength",				\
	"Visibility",			/* 50 */	\
	"CurVisibility",				\
	"ShieldVisibility",				\
	"DamageControl",				\
	"ComChannel",					\
	"AIFlags",					\
/* Skip tractored objects. */				\
	"PermissionUID",				\
	"PermissionGID",				\
	"AnimationInterval",				\
	"AnimationLastInterval",			\
	"AnimationCurrentFrame",	/* 60 */	\
	"AnimationTotalFrames",				\
	"AnimationCycleCount",				\
	"AnimationCycleTimes",				\
	"ScoreCredits",					\
	"ScoreRMU",					\
	"ScoreRMUMax",					\
	"ScoreDamageGiven",				\
	"ScoreDamageRecieved",				\
	"ScoreKills"			/* 69 */	\
}


/* Total number of property prompts, this should match the number of
 * strings in PROP_NAME.
 */
#define TOTAL_PROP_PROMPTS		70


#endif	/* UEWPROPS_H */
