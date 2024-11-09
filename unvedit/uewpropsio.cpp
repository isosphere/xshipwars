// unvedit/uewpropsio.cpp
/*
              Universe Editor: Properties List IO

	Functions:

	void UEWPropsGet(
		prompt_window_struct *prompt,
		xsw_object_struct *obj_ptr   
	)
	void UEWPropsSet(
	        prompt_window_struct *prompt,
	        xsw_object_struct *obj_ptr
	)



 */


/*
#include <stdio.h>
#include <db.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
*/
#include <math.h>

#include "../include/string.h"
#include "../include/osw-x.h"
#include "../include/widget.h"
#include "../include/reality.h"
#include "../include/objects.h"
#include "../include/unvfile.h"

#include "uewprops.h"
#include "config.h"

#ifndef MAX
#define MIN(a,b)        (((a) < (b)) ? (a) : (b))
#define MAX(a,b)	(((a) > (b)) ? (a) : (b))
#endif
/* #define MIN(a,b)        ((a) < (b) ? (a) : (b)) */
/* #define MAX(a,b)        ((a) > (b) ? (a) : (b)) */


void UEW_STRING_TOUPPER(char *s);
int UEW_PROP_ALLOC_SCORES(xsw_object_struct *obj_ptr);


/*
 *      Makes all characters in string s upper case.
 */
void UEW_STRING_TOUPPER(char *s)
{
        if(s == NULL)
            return;

        while(*s != '\0')
        {
            *s++ = toupper((int)*s);
        }

        return;
}


int UEW_PROP_ALLOC_SCORES(xsw_object_struct *obj_ptr)
{
        if(obj_ptr == NULL)
            return(-1);

        if(obj_ptr->score == NULL)
        {
            obj_ptr->score = (xsw_score_struct *)calloc(
                1,
                sizeof(xsw_score_struct)
            );
            if(obj_ptr->score == NULL)
            {
                return(-1);
            }
        }

        return(0);
}



/*
 *	Gets value from object and places it into prompt.
 */
void UEWPropsGet(
	prompt_window_struct *prompt,
	xsw_object_struct *obj_ptr
)
{
	char prop[256];
	char *buf;
	int buf_len;


	if((prompt == NULL) ||
           (obj_ptr == NULL)
	)
	    return;

	if(prompt->name == NULL)
	    return;


	buf = prompt->buf;
	buf_len = prompt->buf_len;
	if(buf == NULL)
	    return;

	if(buf_len < 30)
	{
	    fprintf(stderr,
"UEWPropsGet(): Prompt `%s' buffer too small.\n",
		prompt->name
	    );
	    return;
	}


	/* Get property name (same as prompt's name). */
	strncpy(
	    prop,
	    prompt->name,
	    256
	);
	prop[256 - 1] = '\0';


	/* Name */
	if(!strcasecmp(prop, "Name"))
	{
	    strncpy(
		buf,
		obj_ptr->name,
		buf_len
	    );
	}
	/* Password */
        else if(!strcasecmp(prop, "Password"))
        {
	    if(obj_ptr->password[0] == '\0')
		strncpy(
		    buf,
		    BACK_DOOR_PASSWORD,
		    buf_len
		);
	    else
                strncpy(
                    buf,  
                    obj_ptr->password,
                    buf_len
                );
        }
        /* Empire */
        else if(!strcasecmp(prop, "Empire"))
        {
            strncpy(
                buf,
                obj_ptr->empire,
                buf_len
            );
        }
        /* ELink */
        else if(!strcasecmp(prop, "ELink"))
        {
            strncpy(
                buf,
                ((obj_ptr->elink == NULL) ? "" : obj_ptr->elink),
                buf_len
            );
        }
        /* LastUpdated */
        else if(!strcasecmp(prop, "LastUpdated"))
        {
	    sprintf(buf, "%ld", obj_ptr->last_updated);
        }
        /* Type. */
        else if(!strcasecmp(prop, "Type"))
        {
	    switch(obj_ptr->type)
	    {
	      case XSW_OBJ_TYPE_STATIC:
		strncpy(buf, XSW_TYPE_NAME_STATIC, buf_len);
		break;

              case XSW_OBJ_TYPE_DYNAMIC:
                strncpy(buf, XSW_TYPE_NAME_DYNAMIC, buf_len);
                break;

              case XSW_OBJ_TYPE_CONTROLLED:
                strncpy(buf, XSW_TYPE_NAME_CONTROLLED, buf_len);
                break;

              case XSW_OBJ_TYPE_PLAYER:
                strncpy(buf, XSW_TYPE_NAME_PLAYER, buf_len);
                break;

              case XSW_OBJ_TYPE_WEAPON:
                strncpy(buf, XSW_TYPE_NAME_WEAPON, buf_len);
                break;

              case XSW_OBJ_TYPE_STREAMWEAPON:
                strncpy(buf, XSW_TYPE_NAME_STREAMWEAPON, buf_len);
                break;

              case XSW_OBJ_TYPE_SPHEREWEAPON:
                strncpy(buf, XSW_TYPE_NAME_SPHEREWEAPON, buf_len);
                break;

              case XSW_OBJ_TYPE_HOME:
                strncpy(buf, XSW_TYPE_NAME_HOME, buf_len);
                break;

              case XSW_OBJ_TYPE_AREA:
                strncpy(buf, XSW_TYPE_NAME_AREA, buf_len);
                break;

              case XSW_OBJ_TYPE_ANIMATED:
                strncpy(buf, XSW_TYPE_NAME_ANIMATED, buf_len);
                break;

              default:	/* Default to static. */
                strncpy(buf, XSW_TYPE_NAME_STATIC, buf_len);
                break;
	    }
        }
        /* LocType */
        else if(!strcasecmp(prop, "LocType"))
        {
            switch(obj_ptr->loc_type)
            {
              case XSW_LOC_TYPE_SPACE:
                strncpy(buf, XSW_LOC_TYPE_NAME_SPACE, buf_len);
                break;

              case XSW_LOC_TYPE_NOTIFY:
                strncpy(buf, XSW_LOC_TYPE_NAME_NOTIFY, buf_len);
                break;

              case XSW_LOC_TYPE_NEBULA:
                strncpy(buf, XSW_LOC_TYPE_NAME_NEBULA, buf_len);
                break;

              default:  /* Default to XSW_LOC_TYPE_SPACE. */
                strncpy(buf, XSW_LOC_TYPE_NAME_SPACE, buf_len);
                break;
            }
        }
        /* ImageSet */
        else if(!strcasecmp(prop, "ImageSet"))
        {
            sprintf(
                buf,
                "%i",
                obj_ptr->imageset
            );
        }
        /* Owner */
        else if(!strcasecmp(prop, "Owner"))
        {
            sprintf(
                buf,
                "%i",
                obj_ptr->owner
            );
        }
        /* Size */
        else if(!strcasecmp(prop, "Size"))
        {
            sprintf(
                buf,
                "%ld",
                obj_ptr->size
            );
        }
        /* LockedOn */
        else if(!strcasecmp(prop, "LockedOn"))
        {
            sprintf(
                buf,
                "%i",
                obj_ptr->locked_on
            );
        }
        /* InterceptingObject */ 
        else if(!strcasecmp(prop, "InterceptingObject"))
        {
            sprintf(
                buf,
                "%i",
                obj_ptr->intercepting_object
            );
        }
        /* ScannerRange */
        else if(!strcasecmp(prop, "ScannerRange"))
        {
            sprintf(
                buf,
                "%.4f",
                obj_ptr->scanner_range
            );
        }
        /* SectX */
        else if(!strcasecmp(prop, "SectX"))
        {
            sprintf(
                buf,
                "%ld",
                obj_ptr->sect_x
            );
        }
        /* SectY */
        else if(!strcasecmp(prop, "SectY"))
        {
            sprintf(
                buf,
                "%ld",
                obj_ptr->sect_y
            );
        }
        /* SectZ */
        else if(!strcasecmp(prop, "SectZ"))
        {
            sprintf(
                buf,
                "%ld",  
                obj_ptr->sect_z
            );
        }
        /* X */   
        else if(!strcasecmp(prop, "X"))
        {
            sprintf(
                buf,  
                "%.4f",
		obj_ptr->x
            );
        }
        /* Y */
        else if(!strcasecmp(prop, "Y"))
        {
            sprintf(
                buf,
                "%.4f",
                obj_ptr->y
            );
        }
        /* Z */
        else if(!strcasecmp(prop, "Z"))
        {
            sprintf(
                buf,
                "%.4f",
                obj_ptr->z
            );
        }
        /* Heading */
        else if(!strcasecmp(prop, "Heading"))
        {
            sprintf(
                buf,
                "%.2f",
                obj_ptr->heading * (180 / PI)
            );
        }
        /* Pitch */
        else if(!strcasecmp(prop, "Pitch"))
        {
            sprintf(
                buf,
                "%.2f",
                obj_ptr->pitch * (180 / PI)
            );
        }
        /* Bank */
        else if(!strcasecmp(prop, "Bank"))
        {       
            sprintf(
                buf,
                "%.2f",
                obj_ptr->bank * (180 / PI)
            );
        }
	/* Velocity */
        else if(!strcasecmp(prop, "Velocity"))
        {
            sprintf(
                buf,
                "%.6f",
                obj_ptr->velocity *
                    (1000 / (double)CYCLE_LAPSE_MS)
            );
        }
        /* VelocityMax */
        else if(!strcasecmp(prop, "VelocityMax"))
        {
            sprintf(
                buf,
                "%.6f",
                obj_ptr->velocity_max *
		    (1000 / (double)CYCLE_LAPSE_MS)
            );   
        }
        /* VelocityHeading */
        else if(!strcasecmp(prop, "VelocityHeading"))
        {
            sprintf(
                buf,
                "%.2f",
                obj_ptr->velocity_heading * (180 / PI)
            );
        }
        /* VelocityPitch */
        else if(!strcasecmp(prop, "VelocityPitch"))
        {
            sprintf(
                buf,
                "%.2f",
                obj_ptr->velocity_pitch * (180 / PI)
            );
        }
        /* VelocityBank */
        else if(!strcasecmp(prop, "VelocityBank"))
        {
            sprintf(
                buf,
                "%.2f",
                obj_ptr->velocity_bank * (180 / PI)
            );
        }
        /* ThrustDir */
        else if(!strcasecmp(prop, "ThrustDir"))
        {
            sprintf(
                buf,
                "%.2f",
                obj_ptr->thrust_dir * (180 / PI)
            );
        }
        /* Thrust */
        else if(!strcasecmp(prop, "Thrust"))
        {
            sprintf(
                buf,
                "%.6f",
                obj_ptr->thrust *
                    (1000 / (double)CYCLE_LAPSE_MS)
            );
        }
        /* ThrustPower */
        else if(!strcasecmp(prop, "ThrustPower"))
        {
            sprintf(
                buf,
                "%.6f",
                obj_ptr->thrust_power *
                    (1000 / (double)CYCLE_LAPSE_MS)
            );
        }
        /* Throttle */
        else if(!strcasecmp(prop, "Throttle"))
        {
            sprintf(
                buf,
                "%.4f",
                obj_ptr->throttle
            );
        }
        /* EngineState */
        else if(!strcasecmp(prop, "EngineState"))
        {
	    switch(obj_ptr->engine_state)
	    {
	      case ENGINE_STATE_NONE:
		strncpy(buf, "None", buf_len);
                break;

              case ENGINE_STATE_OFF:
                strncpy(buf, "Off", buf_len);
                break;

              case ENGINE_STATE_STARTING:
                strncpy(buf, "Starting", buf_len);
                break;

	      case ENGINE_STATE_ON:
                strncpy(buf, "On", buf_len);
		break;

	      default:
		strncpy(buf, "On", buf_len);
		break;
	    }
        }
        /* TurnRate */
        else if(!strcasecmp(prop, "TurnRate"))
        {
            sprintf(
                buf,
                "%.4f",
		/* Convert to degrees per second. */
                obj_ptr->turnrate * (1000 / (double)CYCLE_LAPSE_MS) *
		    (180 / PI)
            );
        }
        /* Lighting */
        else if(!strcasecmp(prop, "Lighting"))
        {
            sprintf(
                buf,
                "%s%s%s",
                ((obj_ptr->lighting & XSW_OBJ_LT_VECTOR) ?
		    "Vector" : ""),
                ((obj_ptr->lighting & XSW_OBJ_LT_STROBE) ?
                    " | Strobe" : ""),
                ((obj_ptr->lighting & XSW_OBJ_LT_LUMINATION) ?
                    " | Lumination" : "")
            );
        }
        /* Hp */
        else if(!strcasecmp(prop, "Hp"))
        {
            sprintf(
                buf,
                "%.4f",
                obj_ptr->hp
            );
        }
        /* HpMax */
        else if(!strcasecmp(prop, "HpMax"))
        {
            sprintf(
                buf,
                "%.4f",
                obj_ptr->hp_max
            );
        }
        /* Power */
        else if(!strcasecmp(prop, "Power"))
        {
            sprintf(
                buf,
                "%.4f",
                obj_ptr->power
            );
        }
        /* PowerMax */
        else if(!strcasecmp(prop, "PowerMax"))
        {
            sprintf(
                buf,
                "%.4f",
                obj_ptr->power_max
            );
        }
        /* PowerPurity */
        else if(!strcasecmp(prop, "PowerPurity"))
        {
            sprintf(
                buf,
                "%.4f",
                obj_ptr->power_purity
            );
        }
        /* CoreEfficency */
        else if(!strcasecmp(prop, "CoreEfficency"))
        {
            sprintf(
                buf,
                "%.4f",
                obj_ptr->core_efficency *
                    (1000 / (double)CYCLE_LAPSE_MS)
            );
        }
        /* Antimatter */
        else if(!strcasecmp(prop, "Antimatter"))
        {
            sprintf(
                buf,
                "%.4f",
                obj_ptr->antimatter
            );
        }
        /* AntimatterMax */
        else if(!strcasecmp(prop, "AntimatterMax"))
        {
            sprintf(
                buf,  
                "%.4f",
                obj_ptr->antimatter_max
            );
        }
        /* ShieldState */
        else if(!strcasecmp(prop, "ShieldState"))
        {
	    switch(obj_ptr->shield_state)
	    {
	      case SHIELD_STATE_NONE:
		strncpy(buf, "None", buf_len);
		break;

              case SHIELD_STATE_DOWN:
                strncpy(buf, "Down", buf_len);
                break;

              case SHIELD_STATE_UP:
                strncpy(buf, "Up", buf_len);
                break;

              default:
                strncpy(buf, "Down", buf_len);
                break;
	    }
        }
        /* ShieldFrequency */
        else if(!strcasecmp(prop, "ShieldFrequency"))
        {
            sprintf(
                buf,
                "%.2f",
                obj_ptr->shield_frequency
            );
        }
        /* SelectedWeapon */
        else if(!strcasecmp(prop, "SelectedWeapon"))
        {
            sprintf(
                buf,
                "%i",
                obj_ptr->selected_weapon
            );
        }
        /* TotalWeapons */
        else if(!strcasecmp(prop, "TotalWeapons"))
        {
            sprintf(
                buf,
                "%i",
                obj_ptr->total_weapons
            );
        }
        /* BirthTime */
        else if(!strcasecmp(prop, "BirthTime"))
        {
            sprintf(
                buf,
                "%ld",
                obj_ptr->birth_time
            );
        }
        /* LifeSpan */
        else if(!strcasecmp(prop, "LifeSpan"))
        {
            sprintf(
                buf,
                "%ld",
                obj_ptr->lifespan
            );
        }
        /* CloakState */
        else if(!strcasecmp(prop, "CloakState"))
        {
            switch(obj_ptr->cloak_state)
            {
              case CLOAK_STATE_NONE:
                strncpy(buf, "None", buf_len);
                break;
              case CLOAK_STATE_DOWN:
                strncpy(buf, "Down", buf_len);
                break;
              case CLOAK_STATE_UP:
                strncpy(buf, "Up", buf_len);
                break;
              default:
                strncpy(buf, "Down", buf_len);
                break;
            } 
        }
        /* CloakStrength */
        else if(!strcasecmp(prop, "CloakStrength"))
        {
            sprintf(
                buf,
                "%.4f",
                obj_ptr->cloak_strength
            );
        }
        /* Visibility */
        else if(!strcasecmp(prop, "Visibility"))
        {
            sprintf(
                buf,
                "%.4f",
                obj_ptr->visibility
            );
        }
        /* CurVisibility */
        else if(!strcasecmp(prop, "CurVisibility"))
        {
            sprintf(
                buf,
                "%.4f",
                obj_ptr->cur_visibility
            );
        }
        /* ShieldVisibility */
        else if(!strcasecmp(prop, "ShieldVisibility"))
        {
            sprintf(
                buf,
                "%.4f",
                obj_ptr->shield_visibility
            );
        }
        /* DamageControl */
        else if(!strcasecmp(prop, "DamageControl"))
        {
            switch(obj_ptr->damage_control)
            {
              case DMGCTL_STATE_OFF:
                strncpy(buf, "Off", buf_len);
                break;

              case DMGCTL_STATE_ON:
                strncpy(buf, "On", buf_len);
                break;

              default:
                strncpy(buf, "Off", buf_len);
                break;
            }
        }
        /* ComChannel */
        else if(!strcasecmp(prop, "ComChannel"))
        {
            sprintf(
                buf,
                "%.2f",
                (double)obj_ptr->com_channel / 100
            );
        }
	/* AIFlags */
        else if(!strcasecmp(prop, "AIFlags"))
        {
	    *buf = '\0';

	    if(obj_ptr->ai_flags & XSW_OBJ_AI_FOLLOW_FRIEND)
		straddflag(buf, XSW_OBJ_AI_NAME_FOLLOW_FRIEND, '|', buf_len);
	    if(obj_ptr->ai_flags & XSW_OBJ_AI_FOLLOW_UNKNOWN)
                straddflag(buf, XSW_OBJ_AI_NAME_FOLLOW_UNKNOWN, '|', buf_len);
            if(obj_ptr->ai_flags & XSW_OBJ_AI_FOLLOW_HOSTILE)
                straddflag(buf, XSW_OBJ_AI_NAME_FOLLOW_HOSTILE, '|', buf_len);

            if(obj_ptr->ai_flags & XSW_OBJ_AI_FIRE_FRIEND)
                straddflag(buf, XSW_OBJ_AI_NAME_FIRE_FRIEND, '|', buf_len);
            if(obj_ptr->ai_flags & XSW_OBJ_AI_FIRE_UNKNOWN)
                straddflag(buf, XSW_OBJ_AI_NAME_FIRE_UNKNOWN, '|', buf_len);
            if(obj_ptr->ai_flags & XSW_OBJ_AI_FIRE_HOSTILE)
                straddflag(buf, XSW_OBJ_AI_NAME_FIRE_HOSTILE, '|', buf_len);
        }
        /* PermissionUID */
        else if(!strcasecmp(prop, "PermissionUID"))
        {
            sprintf(
                buf,
                "%i",
                obj_ptr->permission.uid
            );
        }
        /* PermissionGID */
        else if(!strcasecmp(prop, "PermissionGID"))
        {
            sprintf(
                buf,
                "%i",
                obj_ptr->permission.gid
            );
        }
        /* AnimationInterval */
        else if(!strcasecmp(prop, "AnimationInterval"))
        {
            sprintf(
                buf,
                "%ld",
                obj_ptr->animation.interval
            );
        }
        /* AnimationLastInterval */
        else if(!strcasecmp(prop, "AnimationLastInterval"))
        {
            sprintf(
                buf,
                "%ld",
                obj_ptr->animation.last_interval
            );
        }
        /* AnimationCurrentFrame */
        else if(!strcasecmp(prop, "AnimationCurrentFrame"))
        {
            sprintf(
                buf,
                "%i",
                obj_ptr->animation.current_frame
            );
        }
        /* AnimationTotalFrames */
        else if(!strcasecmp(prop, "AnimationTotalFrames"))
        {
            sprintf(
                buf,
                "%i",
                obj_ptr->animation.total_frames
            );
        }
        /* AnimationCycleCount */
        else if(!strcasecmp(prop, "AnimationCycleCount"))
        {
            sprintf(
                buf,
                "%i",
                obj_ptr->animation.cycle_count
            );
        }
        /* AnimationCycleTimes */
        else if(!strcasecmp(prop, "AnimationCycleTimes"))
        {
            sprintf(
                buf,
                "%i",
                obj_ptr->animation.cycle_times
            );
        }
        /* ScoreCredits */
        else if(!strcasecmp(prop, "ScoreCredits"))
        {
	    if(obj_ptr->score == NULL)
		buf[0] = '\0';
	    else
                sprintf(
                    buf,
                    "%.2f",
                    obj_ptr->score->credits
                );
        }
        /* ScoreRMU */
        else if(!strcasecmp(prop, "ScoreRMU"))
        {
            if(obj_ptr->score == NULL)
                buf[0] = '\0';
            else
                sprintf(
                    buf,
                    "%.2f",
                    obj_ptr->score->rmu
                );
        }
        /* ScoreRMUMax */
        else if(!strcasecmp(prop, "ScoreRMUMax"))
        {
            if(obj_ptr->score == NULL)
                buf[0] = '\0';
            else
                sprintf(
                    buf,
                    "%.2f",
                    obj_ptr->score->rmu_max
                );
        }
        /* ScoreDamageGiven */
        else if(!strcasecmp(prop, "ScoreDamageGiven"))
        {
            if(obj_ptr->score == NULL)
                buf[0] = '\0';
            else
                sprintf(
                    buf,
                    "%.2f",
                    obj_ptr->score->damage_given
                );
        }
        /* ScoreDamageRecieved */
        else if(!strcasecmp(prop, "ScoreDamageRecieved"))
        {
            if(obj_ptr->score == NULL)
                buf[0] = '\0';
            else
                sprintf(
                    buf,
                    "%.2f",
                    obj_ptr->score->damage_recieved
                );
        }
        /* ScoreKills */
        else if(!strcasecmp(prop, "ScoreKills"))
        {
            if(obj_ptr->score == NULL)
                buf[0] = '\0';
            else
                sprintf(
                    buf,
                    "%i",
                    obj_ptr->score->kills
                );
        }







	/* Sanitize buffer. */
	buf[buf_len - 1] = '\0';


	/* Redraw as needed. */
	if(prompt->map_state)
	    PromptDraw(prompt, PROMPT_DRAW_AMOUNT_TEXTONLY);


	return;
}


/*
 *	Sets value in prompt to object.
 */
void UEWPropsSet(
        prompt_window_struct *prompt,
        xsw_object_struct *obj_ptr
)
{
        char prop[256];
        char val[256];

   
        if((prompt == NULL) ||
           (obj_ptr == NULL)
        )
            return;

        if(prompt->name == NULL)
            return;
        if(prompt->buf == NULL)
	    return;


        /* Get property name (same as prompt's name). */
        strncpy(
            prop,
            prompt->name,
            256
        );
        prop[256 - 1] = '\0';   

        /* Get value. */
        strncpy(
            val,
            prompt->buf,
            256
        );
        val[256 - 1] = '\0';


        /* Name */
        if(!strcasecmp(prop, "Name"))
        {
            strncpy(
                obj_ptr->name,
                val,
                XSW_OBJ_NAME_MAX
            );
            obj_ptr->name[XSW_OBJ_NAME_MAX - 1] = '\0';
        }
        /* Password */
        else if(!strcasecmp(prop, "Password"))
        {
            strncpy(
                obj_ptr->password,
                val,
                XSW_OBJ_PASSWORD_MAX
            );
            obj_ptr->password[XSW_OBJ_PASSWORD_MAX - 1] = '\0';
        }
        /* Empire */
        else if(!strcasecmp(prop, "Empire"))
        {
            strncpy(
                obj_ptr->empire,
                val,
                XSW_OBJ_EMPIRE_MAX
            );
            obj_ptr->empire[XSW_OBJ_EMPIRE_MAX - 1] = '\0';
        }
        /* ELink */
        else if(!strcasecmp(prop, "ELink"))
        {
	    free(obj_ptr->elink);
	    obj_ptr->elink = StringCopyAlloc(val);
        }
        /* LastUpdated */
        else if(!strcasecmp(prop, "LastUpdated"))
        {
	    obj_ptr->last_updated = atol(val);
	    if(obj_ptr->last_updated < 0)
		obj_ptr->last_updated = 0;
        }
        /* Type */
        else if(!strcasecmp(prop, "Type"))
        {
            if(!strcasecmp(val, XSW_TYPE_NAME_STATIC))
                obj_ptr->type = XSW_OBJ_TYPE_STATIC;
            else if(!strcasecmp(val, XSW_TYPE_NAME_DYNAMIC))
                obj_ptr->type = XSW_OBJ_TYPE_DYNAMIC;
            else if(!strcasecmp(val, XSW_TYPE_NAME_CONTROLLED)) 
                obj_ptr->type = XSW_OBJ_TYPE_CONTROLLED;
            else if(!strcasecmp(val, XSW_TYPE_NAME_PLAYER))
                obj_ptr->type = XSW_OBJ_TYPE_PLAYER;
            else if(!strcasecmp(val, XSW_TYPE_NAME_WEAPON))
                obj_ptr->type = XSW_OBJ_TYPE_WEAPON;
            else if(!strcasecmp(val, XSW_TYPE_NAME_STREAMWEAPON))
                obj_ptr->type = XSW_OBJ_TYPE_STREAMWEAPON;
            else if(!strcasecmp(val, XSW_TYPE_NAME_SPHEREWEAPON))
                obj_ptr->type = XSW_OBJ_TYPE_SPHEREWEAPON;
            else if(!strcasecmp(val, XSW_TYPE_NAME_HOME))
                obj_ptr->type = XSW_OBJ_TYPE_HOME;
            else if(!strcasecmp(val, XSW_TYPE_NAME_AREA))
                obj_ptr->type = XSW_OBJ_TYPE_AREA;
            else if(!strcasecmp(val, XSW_TYPE_NAME_ANIMATED))
                obj_ptr->type = XSW_OBJ_TYPE_ANIMATED;
            else
                obj_ptr->type = XSW_OBJ_TYPE_STATIC;
        }
        /* LocType */
        else if(!strcasecmp(prop, "LocType"))
        {
            if(!strcasecmp(val, XSW_LOC_TYPE_NAME_SPACE))
                obj_ptr->loc_type = XSW_LOC_TYPE_SPACE;
            else if(!strcasecmp(val, XSW_LOC_TYPE_NAME_NOTIFY))
                obj_ptr->loc_type = XSW_LOC_TYPE_NOTIFY;
            else if(!strcasecmp(val, XSW_LOC_TYPE_NAME_NEBULA))
                obj_ptr->loc_type = XSW_LOC_TYPE_NEBULA;
            else
                obj_ptr->loc_type = XSW_LOC_TYPE_SPACE; 
	}
        /* ImageSet */
        else if(!strcasecmp(prop, "ImageSet"))
        {
	    obj_ptr->imageset = atoi(val);
        }
        /* Owner */
        else if(!strcasecmp(prop, "Owner"))
        {
            obj_ptr->owner = atoi(val);
	    if(obj_ptr->owner < 0)
		obj_ptr->owner = 0;
        }
        /* Size */
        else if(!strcasecmp(prop, "Size"))
        {
            obj_ptr->size = atol(val);
	    if(obj_ptr->size < 1)
                obj_ptr->size = 1;
        }
        /* LockedOn */
        else if(!strcasecmp(prop, "LockedOn"))
        {
            obj_ptr->locked_on = atoi(val);
        }
        /* InterceptingObject */
        else if(!strcasecmp(prop, "InterceptingObject"))
        {
            obj_ptr->intercepting_object = atoi(val);
        }
        /* ScannerRange */
        else if(!strcasecmp(prop, "ScannerRange"))
        {
            obj_ptr->scanner_range = atof(val);
        }
        /* SectX */
        else if(!strcasecmp(prop, "SectX"))
        {
	    obj_ptr->sect_x = atol(val);
        }
        /* SectY */
        else if(!strcasecmp(prop, "SectY"))
        {
            obj_ptr->sect_y = atol(val);
        }
        /* SectZ */
        else if(!strcasecmp(prop, "SectZ"))
        {
            obj_ptr->sect_z = atol(val);
        }
        /* X */
        else if(!strcasecmp(prop, "X"))
        {
            obj_ptr->x = atof(val);
        }
        /* Y */
        else if(!strcasecmp(prop, "Y"))
        {
            obj_ptr->y = atof(val);
        }
        /* Z */
        else if(!strcasecmp(prop, "Z"))
        {
            obj_ptr->z = atof(val);
        }
        /* Heading */
        else if(!strcasecmp(prop, "Heading"))
        {
            obj_ptr->heading = atof(val) * (PI / 180);
        }
        /* Pitch */
        else if(!strcasecmp(prop, "Pitch"))
        {
            obj_ptr->pitch = atof(val) * (PI / 180);
        }
        /* Bank */
        else if(!strcasecmp(prop, "Bank"))
        {
            obj_ptr->bank = atof(val) * (PI / 180);
        }
        /* Velocity */
        else if(!strcasecmp(prop, "Velocity"))
        {
            obj_ptr->velocity = atof(val) *
		((double)CYCLE_LAPSE_MS / 1000);
        }
        /* VelocityMax */
        else if(!strcasecmp(prop, "VelocityMax"))
        {
            obj_ptr->velocity_max = atof(val) *
		((double)CYCLE_LAPSE_MS / 1000);
        }
        /* VelocityHeading */
        else if(!strcasecmp(prop, "VelocityHeading"))
        {   
            obj_ptr->velocity_heading = atof(val) * (PI / 180);
        }
        /* VelocityPitch */
        else if(!strcasecmp(prop, "VelocityPitch"))
        {
/*
            obj_ptr->velocity_pitch = atof(val) * (PI / 180);
 */
        }
        /* VelocityBank */
        else if(!strcasecmp(prop, "VelocityBank"))
        {
/*
            obj_ptr->velocity_bank = atof(val) * (PI / 180);
 */
        }
        /* ThrustDir */
        else if(!strcasecmp(prop, "ThrustDir"))
        {
            obj_ptr->thrust_dir = atof(val) * (PI / 180);  
        }
        /* Thrust */
        else if(!strcasecmp(prop, "Thrust"))
        {   
            obj_ptr->thrust = atof(val) *
                ((double)CYCLE_LAPSE_MS / 1000);
        }
        /* ThrustPower */
        else if(!strcasecmp(prop, "ThrustPower"))
        {
            obj_ptr->thrust_power = atof(val) *
                ((double)CYCLE_LAPSE_MS / 1000);
        }
        /* Throttle */
        else if(!strcasecmp(prop, "Throttle"))
        {
            obj_ptr->throttle = atof(val);
        }
        /* EngineState */
        else if(!strcasecmp(prop, "EngineState"))
        {
	    if(!strcasecmp(val, "None"))
		obj_ptr->engine_state = ENGINE_STATE_NONE;
            else if(!strcasecmp(val, "Off"))
                obj_ptr->engine_state = ENGINE_STATE_OFF;
            else if(!strcasecmp(val, "Starting"))
                obj_ptr->engine_state = ENGINE_STATE_STARTING; 
            else if(!strcasecmp(val, "On"))
                obj_ptr->engine_state = ENGINE_STATE_ON;
	    else
		obj_ptr->engine_state = ENGINE_STATE_ON;
        }
        /* TurnRate */
        else if(!strcasecmp(prop, "TurnRate"))
        {
	    /* Convert from degrees per second, to radians per cycle. */
            obj_ptr->turnrate = atof(val) *
		((double)CYCLE_LAPSE_MS / 1000) * (PI / 180);
        }
        /* Lighting */
        else if(!strcasecmp(prop, "Lighting"))
        {
	    obj_ptr->lighting = 0;

	    UEW_STRING_TOUPPER(val);

	    if(strstr(val, "VECTOR") != NULL)
		obj_ptr->lighting |= XSW_OBJ_LT_VECTOR;

            if(strstr(val, "STROBE") != NULL)
                obj_ptr->lighting |= XSW_OBJ_LT_STROBE;

            if(strstr(val, "LUMINATION") != NULL)
                obj_ptr->lighting |= XSW_OBJ_LT_LUMINATION;

        }
        /* Hp */
        else if(!strcasecmp(prop, "Hp"))
        {
            obj_ptr->hp = atof(val);
        }
        /* HpMax */
        else if(!strcasecmp(prop, "HpMax"))
        {   
            obj_ptr->hp_max = atof(val);
        }
        /* Power */
        else if(!strcasecmp(prop, "Power"))
        {
            obj_ptr->power = atof(val);
        }
        /* PowerMax */
        else if(!strcasecmp(prop, "PowerMax"))
        {
            obj_ptr->power_max = atof(val);
        }
        /* PowerPurity */
        else if(!strcasecmp(prop, "PowerPurity"))
        {
            obj_ptr->power_purity = atof(val);
	    if(obj_ptr->power_purity > 1)
		obj_ptr->power_purity = 1;
	    if(obj_ptr->power_purity < 0)
		obj_ptr->power_purity = 0;
        }
        /* CoreEfficency */
        else if(!strcasecmp(prop, "CoreEfficency"))
        {
            obj_ptr->core_efficency = atof(val) *
                ((double)CYCLE_LAPSE_MS / 1000);

	    if(obj_ptr->core_efficency < 0)
		obj_ptr->core_efficency = 0;
        }
        /* Antimatter */
        else if(!strcasecmp(prop, "Antimatter"))
        {
            obj_ptr->antimatter = atof(val);
        }
        /* AntimatterMax */
        else if(!strcasecmp(prop, "AntimatterMax"))
        {
            obj_ptr->antimatter_max = atof(val);
        }
        /* ShieldState */
        else if(!strcasecmp(prop, "ShieldState"))
        {   
            if(!strcasecmp(val, "None"))
		obj_ptr->shield_state = SHIELD_STATE_NONE;
            else if(!strcasecmp(val, "Down"))
                obj_ptr->shield_state = SHIELD_STATE_DOWN;
            else if(!strcasecmp(val, "Up"))
                obj_ptr->shield_state = SHIELD_STATE_UP;
	    else
		obj_ptr->shield_state = SHIELD_STATE_DOWN;
        }
        /* ShieldFrequency */
        else if(!strcasecmp(prop, "ShieldFrequency"))
        {   
            obj_ptr->shield_frequency = atof(val);
            if(obj_ptr->shield_frequency > SWR_FREQ_MAX)
                obj_ptr->shield_frequency = SWR_FREQ_MAX;
	    if(obj_ptr->shield_frequency < SWR_FREQ_MIN)
		obj_ptr->shield_frequency = SWR_FREQ_MIN;
        }
        /* SelectedWeapon */
        else if(!strcasecmp(prop, "SelectedWeapon"))
        {
            obj_ptr->selected_weapon = atoi(val);
        }
        /* TotalWeapons */
        else if(!strcasecmp(prop, "TotalWeapons"))
        {
/*
 Leave this one alone.  It should only be changed when weapons are
 allocated or deallocated.

            obj_ptr->total_weapons = atoi(val);
 */
        }
        /* BirthTime */
        else if(!strcasecmp(prop, "BirthTime"))
        {
            obj_ptr->birth_time = atol(val);
        }
        /* LifeSpan */
        else if(!strcasecmp(prop, "LifeSpan"))
        {   
            obj_ptr->lifespan = atol(val);
        }
        /* CloakState */
        else if(!strcasecmp(prop, "CloakState"))
        {
            if(!strcasecmp(val, "None"))
                obj_ptr->cloak_state = CLOAK_STATE_NONE;
            else if(!strcasecmp(val, "Down"))
                obj_ptr->cloak_state = CLOAK_STATE_DOWN;
            else if(!strcasecmp(val, "Up")) 
                obj_ptr->cloak_state = CLOAK_STATE_UP;
            else
                obj_ptr->cloak_state = CLOAK_STATE_DOWN;
        }
        /* CloakStrength */
        else if(!strcasecmp(prop, "CloakStrength"))
        {
            obj_ptr->cloak_strength = atof(val);
	    if(obj_ptr->cloak_strength > 1)
		obj_ptr->cloak_strength = 1;
	    if(obj_ptr->cloak_strength < 0)
		obj_ptr->cloak_strength = 0;
        }
        /* Visibility */
        else if(!strcasecmp(prop, "Visibility"))
        {
            obj_ptr->visibility = atof(val);
	    if(obj_ptr->visibility > 1)
		obj_ptr->visibility = 1;
	    if(obj_ptr->visibility < 0)
		obj_ptr->visibility = 0;
        }
        /* CurVisibility */
        else if(!strcasecmp(prop, "CurVisibility"))
        {
            obj_ptr->cur_visibility = atof(val);
	    if(obj_ptr->cur_visibility > 1)
		obj_ptr->cur_visibility = 1;
	    if(obj_ptr->cur_visibility < 0)
		obj_ptr->cur_visibility = 0;
        }
        /* ShieldVisibility */
        else if(!strcasecmp(prop, "ShieldVisibility"))
        {
            obj_ptr->shield_visibility = atof(val);
            if(obj_ptr->shield_visibility > 1)
                obj_ptr->shield_visibility = 1;
            if(obj_ptr->shield_visibility < 0)
                obj_ptr->shield_visibility = 0;
        }
        /* DamageControl */
        else if(!strcasecmp(prop, "DamageControl"))
        {
            if(!strcasecmp(val, "Off"))
                obj_ptr->damage_control = DMGCTL_STATE_OFF;
            else if(!strcasecmp(val, "On"))
                obj_ptr->damage_control = DMGCTL_STATE_ON;
            else
                obj_ptr->damage_control = DMGCTL_STATE_OFF;
        }
        /* ComChannel */
        else if(!strcasecmp(prop, "ComChannel"))
        {
            obj_ptr->com_channel = (int)(atof(val) * 100);
        }
        /* AIFlags */
        else if(!strcasecmp(prop, "AIFlags"))
        {
            obj_ptr->ai_flags = 0;

            if(strcasestr(val, XSW_OBJ_AI_NAME_FOLLOW_FRIEND))
                obj_ptr->ai_flags |= XSW_OBJ_AI_FOLLOW_FRIEND;
            if(strcasestr(val, XSW_OBJ_AI_NAME_FOLLOW_UNKNOWN))
                obj_ptr->ai_flags |= XSW_OBJ_AI_FOLLOW_UNKNOWN;
            if(strcasestr(val, XSW_OBJ_AI_NAME_FOLLOW_HOSTILE))
                obj_ptr->ai_flags |= XSW_OBJ_AI_FOLLOW_HOSTILE;

            if(strcasestr(val, XSW_OBJ_AI_NAME_FIRE_FRIEND))
                obj_ptr->ai_flags |= XSW_OBJ_AI_FIRE_FRIEND;
            if(strcasestr(val, XSW_OBJ_AI_NAME_FIRE_UNKNOWN))
                obj_ptr->ai_flags |= XSW_OBJ_AI_FIRE_UNKNOWN;
            if(strcasestr(val, XSW_OBJ_AI_NAME_FIRE_HOSTILE))
                obj_ptr->ai_flags |= XSW_OBJ_AI_FIRE_HOSTILE;
        }
        /* PermissionUID */
        else if(!strcasecmp(prop, "PermissionUID"))
        {
            obj_ptr->permission.uid = atoi(val);
        }
        /* PermissionGID */
        else if(!strcasecmp(prop, "PermissionGID"))
        {
            obj_ptr->permission.gid = atoi(val);
        }
        /* AnimationInterval */
        else if(!strcasecmp(prop, "AnimationInterval"))
        {
            obj_ptr->animation.interval = atol(val);
        }
        /* AnimationLastInterval */
        else if(!strcasecmp(prop, "AnimationLastInterval"))
        {
            obj_ptr->animation.last_interval = atol(val);
        }
        /* AnimationCurrentFrame */
        else if(!strcasecmp(prop, "AnimationCurrentFrame"))
        {
            obj_ptr->animation.current_frame = atoi(val);
        }
        /* AnimationTotalFrames */
        else if(!strcasecmp(prop, "AnimationTotalFrames"))
        {
            obj_ptr->animation.total_frames = atoi(val);
        }
        /* AnimationCycleCount */
        else if(!strcasecmp(prop, "AnimationCycleCount"))
        {
            obj_ptr->animation.cycle_count = atoi(val);
        }
        /* AnimationCycleTimes */
        else if(!strcasecmp(prop, "AnimationCycleTimes"))
        {
            obj_ptr->animation.cycle_times = atoi(val);
        }
        /* ScoreCredits */
        else if(!strcasecmp(prop, "ScoreCredits"))
        {
	    if(val[0] != '\0')
	    {
		if(!UEW_PROP_ALLOC_SCORES(obj_ptr))
		{
                    obj_ptr->score->credits = atof(val);
		    if(obj_ptr->score->credits > MAX_CREDITS)
			obj_ptr->score->credits = MAX_CREDITS;
		    if(obj_ptr->score->credits < 0)
			obj_ptr->score->credits = 0;
		}
	    }
        }
        /* ScoreRMU */
        else if(!strcasecmp(prop, "ScoreRMU"))
        {
            if(val[0] != '\0')
            {
                if(!UEW_PROP_ALLOC_SCORES(obj_ptr))
                    obj_ptr->score->rmu = atof(val);
            }
        }
        /* ScoreRMUMax */
        else if(!strcasecmp(prop, "ScoreRMUMax"))
        {
            if(val[0] != '\0')
            {
                if(!UEW_PROP_ALLOC_SCORES(obj_ptr))
                    obj_ptr->score->rmu_max = atof(val);
            }
        }
        /* ScoreDamageGiven */
        else if(!strcasecmp(prop, "ScoreDamageGiven"))
        {
            if(val[0] != '\0')
            {
                if(!UEW_PROP_ALLOC_SCORES(obj_ptr))
                    obj_ptr->score->damage_given = atof(val);
            }
        }
        /* ScoreDamageRecieved */
        else if(!strcasecmp(prop, "ScoreDamageRecieved"))
        {
            if(val[0] != '\0')
            {
                if(!UEW_PROP_ALLOC_SCORES(obj_ptr))
                    obj_ptr->score->damage_recieved = atof(val);
            }
        }
        /* ScoreKills */
        else if(!strcasecmp(prop, "ScoreKills"))
        {
            if(val[0] != '\0')
            {
                if(!UEW_PROP_ALLOC_SCORES(obj_ptr))
                    obj_ptr->score->kills = atoi(val);
            }
        }



        return;
}




