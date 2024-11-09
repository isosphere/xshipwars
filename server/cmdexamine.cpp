#include "../include/unvmatch.h"
#include "../include/unvmath.h"

#include "swserv.h"
#include "net.h"


#define THIS_CMD_NAME	"examine"


int CmdExamine(int condescriptor, const char *arg)
{
	int i, con_obj_num, obj_num, wep_num;
	char *strptr, *strptr2;        
        xsw_object_struct *obj_ptr, *con_obj_ptr;
        connection_struct *con_ptr;

        char sndbuf[CS_DATA_MAX_LEN];
        char name1[XSW_OBJ_NAME_MAX + 80];
        char name2[XSW_OBJ_NAME_MAX + 80];


        /* Get connection's object number (assumed valid). */
	con_ptr = connection[condescriptor];
        con_obj_num = con_ptr->object_num;
        con_obj_ptr = xsw_object[con_obj_num];


        /* If no argument, print usage. */
        if((arg == NULL) ? 1 : (arg[0] == '\0'))
        {
            sprintf(sndbuf,
                "Usage: `examine <object>'"
            );
            NetSendLiveMessage(condescriptor, sndbuf);

            return(-1);
        }


        /* Check if allowed to examine? */
        if(con_obj_ptr->permission.uid > ACCESS_UID_EXAMINE)
        {
            sprintf(sndbuf,
                "%s: Permission denied: Requires access level %i.",
		THIS_CMD_NAME,
                ACCESS_UID_EXAMINE
            );
            NetSendLiveMessage(condescriptor, sndbuf);
            return(-1);
        }
        
        
        /* Search and get object number. */
        if(!strcasecmp(arg, "me"))
            obj_num = con_obj_num;
        else
            obj_num = MatchObjectByName(
		xsw_object, total_objects,
		arg, -1
	    );

        if(DBIsObjectGarbage(obj_num))
        {
            sprintf(
		sndbuf,
                "%s: No such object.",
                arg
            );
            NetSendLiveMessage(condescriptor, sndbuf);

            return(-1);
        }
        else
        {
            obj_ptr = xsw_object[obj_num];
        }

        /* Check if allowed to examine other? */
        if((obj_ptr->owner != con_obj_num) &&
           (con_obj_ptr->permission.uid > ACCESS_UID_EXAMINEO)
        )
        {
            if(DBIsObjectGarbage(obj_ptr->owner))
            {
                NetSendLiveMessage(condescriptor,
                    "examine: *No owner*"
                );

                return(-1);
            }
            else
            {
                sprintf(sndbuf,
                    "examine: Owner: %s",
                    xsw_object[obj_ptr->owner]->name
                );
                NetSendLiveMessage(condescriptor, sndbuf);

                return(-1);
            }
        }
        /* *********************************************** */
        /* Print information. */


        /* Line 1: Name, type, uid, and credits. */
        strncpy(
	    name1,
	    DBGetFormalNameStr(obj_num),
	    XSW_OBJ_NAME_MAX + 80
	);
        name1[XSW_OBJ_NAME_MAX + 80 - 1] = '\0';
        strncpy(
            name2,
            DBGetFormalNameStr(obj_ptr->owner),
            XSW_OBJ_NAME_MAX + 80
        );
        name2[XSW_OBJ_NAME_MAX + 80 - 1] = '\0';

        sprintf(sndbuf,
 "%s  Owner: %s  Type: %s  UID: %i  Empire: %s  Credits: %0.2f",
            name1,
            name2,
            DBGetTypeName(obj_ptr->type),
            obj_ptr->permission.uid,
	    obj_ptr->empire,
            ((obj_ptr->score == NULL) ? 0 :
                obj_ptr->score->credits)
        );
        NetSendLiveMessage(condescriptor, sndbuf);

	/* Line 1.25: (optional for Elink objects). */
	if(obj_ptr->type == XSW_OBJ_TYPE_ELINK)
	{
	    sprintf(sndbuf,
 "External Link: %s",
		((obj_ptr->elink == NULL) ? "*unlinked*" : obj_ptr->elink)
	    );
            NetSendLiveMessage(condescriptor, sndbuf);
	}

	/* Line 1.5: (optional for AI/Controlled). */
	if(obj_ptr->type == XSW_OBJ_TYPE_CONTROLLED)
	{
	    sprintf(sndbuf,
 "AI Flags: %s %s %s %s %s %s",
		((obj_ptr->ai_flags & XSW_OBJ_AI_FOLLOW_FRIEND) ?
		    XSW_OBJ_AI_NAME_FOLLOW_FRIEND : ""
		),
                ((obj_ptr->ai_flags & XSW_OBJ_AI_FOLLOW_UNKNOWN) ?
                    XSW_OBJ_AI_NAME_FOLLOW_UNKNOWN : ""
                ),
                ((obj_ptr->ai_flags & XSW_OBJ_AI_FOLLOW_HOSTILE) ?
                    XSW_OBJ_AI_NAME_FOLLOW_HOSTILE : ""
                ),
                ((obj_ptr->ai_flags & XSW_OBJ_AI_FIRE_FRIEND) ?
                    XSW_OBJ_AI_NAME_FIRE_FRIEND : ""
                ),
                ((obj_ptr->ai_flags & XSW_OBJ_AI_FIRE_UNKNOWN) ?
                    XSW_OBJ_AI_NAME_FIRE_UNKNOWN : ""
                ),
                ((obj_ptr->ai_flags & XSW_OBJ_AI_FIRE_HOSTILE) ?
                    XSW_OBJ_AI_NAME_FIRE_HOSTILE : ""
                )
	    );
	    NetSendLiveMessage(condescriptor, sndbuf);
	}

        /* Line 2: Sector, position, location type, and size. */
	switch(obj_ptr->loc_type)
	{
	  case XSW_LOC_TYPE_NEBULA:
	    strptr = XSW_LOC_TYPE_NAME_NEBULA;
	    break;

	  case XSW_LOC_TYPE_NOTIFY:
	    strptr = XSW_LOC_TYPE_NAME_NOTIFY;
	    break;

	  default:
	    strptr = XSW_LOC_TYPE_NAME_SPACE;
	    break;
	}

        sprintf(sndbuf,
  "Sector: %ld %ld %ld  Position: %.4f %.4f %.4f  Location Type: %s\
  Size: %ld su",

            obj_ptr->sect_x,
            obj_ptr->sect_y,
            obj_ptr->sect_z,

            obj_ptr->x,
            obj_ptr->y,
            obj_ptr->z,

	    strptr,
            obj_ptr->size
        );
        NetSendLiveMessage(condescriptor, sndbuf);

        /* Line 3: Attitude, velocity vector. */
        sprintf(sndbuf,
 "Attitude: %.2f' %.2f' %.2f'  Velocity vector: %.2f' %.2f' %.2f'\
  Turn Rate: %.4f",

            RADTODEG(obj_ptr->heading),
            RADTODEG(obj_ptr->pitch),
            RADTODEG(obj_ptr->bank),

            RADTODEG(obj_ptr->velocity_heading),
            (double)0,
            (double)0,

	    obj_ptr->turnrate
        );
        NetSendLiveMessage(condescriptor, sndbuf);

        /* Line 4: Velocity, thrust power, throttle, engine state. */
        sprintf(sndbuf,
 "Velocity: %.4f(%.4f)  Thrust power: %.4f  Throttle: %.0f%%\
  Engine state: %i",
            obj_ptr->velocity,
            obj_ptr->velocity_max,
            obj_ptr->thrust_power,
            obj_ptr->throttle * 100,
	    obj_ptr->engine_state
        );
        NetSendLiveMessage(condescriptor, sndbuf);

	/* Line 5: Isref, scanner range, and turn rate. */
        strncpy(
            name1,
            DBGetFormalNameStr(obj_ptr->locked_on),
            XSW_OBJ_NAME_MAX + 80
        );
	name1[XSW_OBJ_NAME_MAX + 80 - 1] = '\0';
        strncpy(
            name2,
            DBGetFormalNameStr(obj_ptr->intercepting_object),
            XSW_OBJ_NAME_MAX + 80
        );
        name2[XSW_OBJ_NAME_MAX + 80 - 1] = '\0';

        sprintf(sndbuf,
 "Imageset: %i  Scanner Range: %.2f ru  Locked on: %s  Intercept: %s",
            obj_ptr->imageset,
            obj_ptr->scanner_range,
	    name1,
	    name2
        );
        NetSendLiveMessage(condescriptor, sndbuf);
                
        /* Line 6: Hull, power, purity, core efficiency. */
        sprintf(sndbuf,
 "Hull: %.2f(%.2f)  Power: %.2f(%.2f)  Purity: %.3f%%\
  Core efficency: %.4f",
            obj_ptr->hp,
            obj_ptr->hp_max,

            obj_ptr->power,
            obj_ptr->power_max,

	    obj_ptr->power_purity * 100,
            obj_ptr->core_efficency
        );
        NetSendLiveMessage(condescriptor, sndbuf);
            
        /* Line 7: Antimatter, shields, cloak and visibility. */
	switch(obj_ptr->shield_state)
	{
	  case SHIELD_STATE_UP:
	    strptr = "Up";
	    break;

          case SHIELD_STATE_DOWN:
            strptr = "Down";
            break;

          default:
            strptr = "None";
            break;
	}
        switch(obj_ptr->cloak_state)
        {
          case CLOAK_STATE_UP:
            strptr2 = "Up";
            break;

          case CLOAK_STATE_DOWN:
            strptr2 = "Down";
            break;

          default:
            strptr2 = "None";
            break;
        }
        sprintf(sndbuf,
 "AM: %.2f(%.2f)  Shields: %s  Cloak: %s  Cloak Strength: %.0f%%\
  Vis: %.0f%%",
            obj_ptr->antimatter,
            obj_ptr->antimatter_max,
	    strptr,
	    strptr2,
            obj_ptr->cloak_strength * 100,
            obj_ptr->visibility * 100
        );
        NetSendLiveMessage(condescriptor, sndbuf);
            
        /* Print weapons. */
        if(obj_ptr->weapons != NULL)
        {
	    xsw_weapons_struct *wep_ptr;


            for(wep_num = 0; wep_num < obj_ptr->total_weapons; wep_num++)
            {
		wep_ptr = obj_ptr->weapons[wep_num];
                if(wep_ptr == NULL)
                    continue;

                sprintf(sndbuf,
 "Wep %i [%s%s]:  OCS: %i  EmtType: %i  Amt: %i(%i)  Pwr: %.2f\
  Rng: %.2f  CPwr: %.2f  Delay: %ld ms  FSndCode: %i",
                    wep_num,
                ((wep_ptr->flags & XSW_WEP_FLAG_NO_FIRE_SOUND) ? "S" : ""),
                ((wep_ptr->flags & XSW_WEP_FLAG_FIXED) ? "F" : ""),
                    wep_ptr->ocs_code,
                    wep_ptr->emission_type,
                    wep_ptr->amount,
                    wep_ptr->max,
                    wep_ptr->power,

                    (double)wep_ptr->range / 1000,
                    wep_ptr->create_power,
		    wep_ptr->delay,
		    wep_ptr->fire_sound_code
                );
                NetSendLiveMessage(condescriptor, sndbuf);
            }
        }
 
        /* Scores. */
        if(obj_ptr->score != NULL)  
        {
            sprintf(sndbuf,
 "Credits: %.2f  RMU: %.2f(%.2f)  DmgGiv: %.2f  DmgRec: %.2f  Kills: %i",
                obj_ptr->score->credits,
                obj_ptr->score->rmu,
                obj_ptr->score->rmu_max,
                obj_ptr->score->damage_given,
                obj_ptr->score->damage_recieved,
                obj_ptr->score->kills
            );
            NetSendLiveMessage(condescriptor, sndbuf);
        }

        /* Economy. */
        if(obj_ptr->eco != NULL)
        {
	    xsw_ecodata_struct *eco_ptr;
	    xsw_ecoproduct_struct *product_ptr;


	    eco_ptr = obj_ptr->eco;

            sprintf(sndbuf,
 "Eco [%s%s%s%s%s]: TaxG: %.0f%%  TaxF: %.0f%%  TaxH: %.0f%%",
		((eco_ptr->flags & ECO_FLAG_OPEN) ? "O" : ""),
                ((eco_ptr->flags & ECO_FLAG_BUY_OK) ? "B" : ""),
                ((eco_ptr->flags & ECO_FLAG_SELL_OK) ? "S" : ""),
                ((eco_ptr->flags & ECO_FLAG_TRADE_OK) ? "T" : ""),
                ((eco_ptr->flags & ECO_FLAG_INTRODUCE_OK) ? "I" : ""),
                (eco_ptr->tax_general - 1) * 100,
                (eco_ptr->tax_friend - 1) * 100,
                (eco_ptr->tax_hostile - 1) * 100
            );
            NetSendLiveMessage(condescriptor, sndbuf);

	    for(i = 0; i < eco_ptr->total_products; i++)
	    {
		product_ptr = eco_ptr->product[i];
		if(product_ptr == NULL)
		    continue;

		if(product_ptr->amount_max > 0)
                    sprintf(sndbuf,
 "EcoProd `%s':  Buy: %.2f  Sell: %.2f Amt: %.2f(%.2f)",
                        product_ptr->name,
                        product_ptr->buy_price,
                        product_ptr->sell_price,
                        product_ptr->amount,
                        product_ptr->amount_max
                    );
		else
                    sprintf(sndbuf,
 "EcoProd `%s':  Buy: %.2f  Sell: %.2f Amt: %.2f(Nolimit)",
                        product_ptr->name,
                        product_ptr->buy_price,
                        product_ptr->sell_price,
                        product_ptr->amount
                    );
                NetSendLiveMessage(condescriptor, sndbuf);
	    }
        }


        return(0);
}
