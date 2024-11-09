// global/unvfile.cpp
/*
                            Universe File IO

	Functions:

	xsw_object_struct UNVLoadFromFile(
	        char *file,
	        int *total,
	        unv_head_struct *header_buf,
	        void *client_data,
	        void (*progress_notify)(void *, int, int)
	)
	int UNVSaveToFile(
	        char *file,
	        xsw_object_struct **obj_ptr,
	        int total,
	        unv_head_struct *header_buf,
		void *client_data,
		void (*progress_notify)(void *, int, int)
	)

	---

 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <malloc.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>

#include "../include/string.h"
#include "../include/strexp.h"
#include "../include/cfgfmt.h"
#include "../include/fio.h"
#include "../include/reality.h"
#include "../include/objects.h"

#include "../include/unvmain.h"
#include "../include/unvutil.h"
#include "../include/unvmatch.h"
#include "../include/unvmath.h"
#include "../include/unvfile.h"



/*
 *	Loads objects from file.  The loaded objects are dynamically
 *	allocated and need to be deallocated with a call to
 *	UNVDeleteAllObjects().
 */
xsw_object_struct **UNVLoadFromFile(
        char *file,
        int *total,
	unv_head_struct *header_buf,
	void *client_data,
	void (*progress_notify)(void *, int, int)
)
{
	int i, n, prev;
	char *strptr, *strptr2;
        off_t filesize;
	FILE *fp;
	struct stat stat_buf;

	char parm[CFG_PARAMETER_MAX];
	char val[CFG_VALUE_MAX];

	int lines_read = 0;

	int object_num = -1;
	int weapons_count = 0;
	int product_num = 0;

	xsw_ecodata_struct *eco_ptr;
	xsw_object_struct *obj_ptr;
	xsw_object_struct **ptr = NULL;



	if(total == NULL)
	    return(ptr);

	*total = 0;

	if(file == NULL)
	    return(ptr);
        if(stat(file, &stat_buf))
        {
            fprintf(stderr,
                "%s: No such file.\n",
                file
            );
            return(ptr);
        }

	/* Get size of file. */
        filesize = stat_buf.st_size;

	/* Open file. */
	fp = fopen(file, "r");
	if(fp == NULL)
	{
            fprintf(stderr,
		"%s: Cannot open.\n",
                file
            );
            return(ptr);
	}


	/* Reset header. */
	if(header_buf != NULL)
	{
            strncpy(header_buf->title, "Untitled", UNV_TITLE_MAX);
            header_buf->title[UNV_TITLE_MAX - 1] = '\0';

            header_buf->isr[0] = '\0';
            header_buf->ocsn[0] = '\0';
            header_buf->ss[0] = '\0';
	}


	/* ************************************************ */

	/* Call notify progress function. */
	if(progress_notify != NULL)
	    progress_notify(
		client_data,
		(int)ftell(fp),
		(int)filesize
	    );


        strptr = NULL;

	while(1)
	{
            /* Free previous line and allocate/read next line. */
            free(strptr); strptr = NULL;
            strptr = FReadNextLineAllocCount(
                fp, UNIXCFG_COMMENT_CHAR, &lines_read
            );
            if(strptr == NULL) break;

            /* Fetch parameter. */
            strptr2 = StringCfgParseParm(strptr);
            if(strptr2 == NULL) continue;
            strncpy(parm, strptr2, CFG_PARAMETER_MAX);
            parm[CFG_PARAMETER_MAX - 1] = '\0';

            /* Fetch value. */
            strptr2 = StringCfgParseValue(strptr);
	    /* Set value to "0" if NULL pointer was returned. */
            if(strptr2 == NULL) strptr2 = "0";
            strncpy(val, strptr2, CFG_VALUE_MAX);
            val[CFG_VALUE_MAX - 1] = '\0';


            /* Call notify progress function. */
            if(progress_notify != NULL)
                progress_notify(
                    client_data,
                    (int)ftell(fp),
                    (int)filesize
                );


	    /* ***************************************************** */
	    /* BeginHeader */
	    if(!strcasecmp(parm, "BeginHeader"))
	    {
                while(1)
                {
                    /* Free previous line and allocate/read next line. */
                    free(strptr); strptr = NULL;
                    strptr = FReadNextLineAllocCount(
                        fp, UNIXCFG_COMMENT_CHAR, &lines_read
                    );
                    if(strptr == NULL) break;

                    /* Fetch parameter. */
                    strptr2 = StringCfgParseParm(strptr);
                    if(strptr2 == NULL) continue;
                    strncpy(parm, strptr2, CFG_PARAMETER_MAX);
                    parm[CFG_PARAMETER_MAX - 1] = '\0';

                    /* Fetch value. */
                    strptr2 = StringCfgParseValue(strptr);
                    /* Set value to "0" if NULL pointer was returned. */ 
                    if(strptr2 == NULL) strptr2 = "0";
                    strncpy(val, strptr2, CFG_VALUE_MAX);
                    val[CFG_VALUE_MAX - 1] = '\0';


                    /* Title */
                    if(!strcasecmp(parm, "Title"))
                    {
			if(header_buf != NULL)
			{
                            strncpy(
				header_buf->title,
				val,
				UNV_TITLE_MAX
			    );
			    header_buf->title[UNV_TITLE_MAX - 1] = '\0';
			}
                    }
		    /* Version. */
                    else if(!strcasecmp(parm, "Version"))
                    {
                        if(header_buf != NULL)
                        {
                            strncpy(
                                header_buf->version,
                                val,
                                UNV_TITLE_MAX
                            );
                            header_buf->version[UNV_TITLE_MAX - 1] = '\0';
                        }
                    }
		    /* UnitConversionRUTOAU */
		    else if(!strcasecmp(parm, "UnitConversionRUTOAU"))
		    {
                        if(header_buf != NULL)
                        {
			    header_buf->ru_to_au = atof(val);
			    if(header_buf->ru_to_au <= 0)
			    {
			        fprintf(stderr,
 "%s: Line %i: UnitConversionRUTOAU `%s' out of range.\n",
                                    file, lines_read, val
                                );
			        header_buf->ru_to_au = DEF_UNITCONV_RU_TO_AU;
			    }
		        }
		    }
                    /* ImageSetFile */
                    else if(!strcasecmp(parm, "ImageSetFile"))
                    {
                        if(header_buf != NULL)
                        {
                            strncpy(header_buf->isr, val, PATH_MAX + NAME_MAX);
                            header_buf->isr[PATH_MAX + NAME_MAX - 1] = '\0';
			}
                    }
                    /* OCSNFile */
                    else if(!strcasecmp(parm, "OCSNFile"))
                    {
                        if(header_buf != NULL)
                        {
                            strncpy(header_buf->ocsn, val, PATH_MAX + NAME_MAX);
                            header_buf->ocsn[PATH_MAX + NAME_MAX - 1] = '\0';
			}
                    }
                    /* SoundSchemeFile */
                    else if(!strcasecmp(parm, "SoundSchemeFile"))
                    {
			if(header_buf != NULL)
                        {
                            strncpy(header_buf->ss, val, PATH_MAX + NAME_MAX);
                            header_buf->ss[PATH_MAX + NAME_MAX - 1] = '\0';
                        }
		    }
		    /* LostFoundOwner */
		    else if(!strcasecmp(parm, "LostFoundOwner"))
		    {
                        if(header_buf != NULL)
                        {
                            header_buf->lost_found_owner = atoi(val);
			}
		    }

		    /* PlayerStartSectX */
                    else if(!strcasecmp(parm, "PlayerStartSectX"))
		    {
                        header_buf->player_start_sect_x = atol(val);
                    }
                    /* PlayerStartSectY */
                    else if(!strcasecmp(parm, "PlayerStartSectY"))
                    {
                        header_buf->player_start_sect_y = atol(val);
                    }
                    /* PlayerStartSectZ */
                    else if(!strcasecmp(parm, "PlayerStartSectZ"))
                    {
                        header_buf->player_start_sect_z = atol(val);
                    }
                    /* PlayerStartX */
                    else if(!strcasecmp(parm, "PlayerStartX"))
                    {
                        header_buf->player_start_x = atof(val);
                    }
                    /* PlayerStartY */
                    else if(!strcasecmp(parm, "PlayerStartY"))
                    {
                        header_buf->player_start_y = atof(val);
                    }
                    /* PlayerStartZ */
                    else if(!strcasecmp(parm, "PlayerStartZ"))
                    {       
                        header_buf->player_start_z = atof(val);
                    }

                    /* PlayerStartHeading */
                    else if(!strcasecmp(parm, "PlayerStartHeading"))
                    {
                        header_buf->player_start_heading = atof(val);
                    }
                    /* PlayerStartPitch */
                    else if(!strcasecmp(parm, "PlayerStartPitch"))
                    {
                        header_buf->player_start_pitch = atof(val);
                    }
                    /* PlayerStartBank */
                    else if(!strcasecmp(parm, "PlayerStartBank"))
                    {
                        header_buf->player_start_bank = atof(val);
                    }

                    /* GuestStartSectX */
                    else if(!strcasecmp(parm, "GuestStartSectX"))
                    {
                        header_buf->guest_start_sect_x = atol(val);
                    }
                    /* GuestStartSectY */
                    else if(!strcasecmp(parm, "GuestStartSectY"))
                    {
                        header_buf->guest_start_sect_y = atol(val);
                    }
                    /* GuestStartSectZ */
                    else if(!strcasecmp(parm, "GuestStartSectZ"))
                    {
                        header_buf->guest_start_sect_z = atol(val);
                    }
                    /* GuestStartX */
                    else if(!strcasecmp(parm, "GuestStartX"))
                    {
                        header_buf->guest_start_x = atof(val);
                    }
                    /* GuestStartY */
                    else if(!strcasecmp(parm, "GuestStartY"))
                    {
                        header_buf->guest_start_y = atof(val);
                    }
                    /* GuestStartZ */
                    else if(!strcasecmp(parm, "GuestStartZ"))
                    {
                        header_buf->guest_start_z = atof(val);
                    }

                    /* GuestStartHeading */
                    else if(!strcasecmp(parm, "GuestStartHeading"))
                    {
                        header_buf->guest_start_heading = atof(val);
                    }
                    /* GuestStartPitch */
                    else if(!strcasecmp(parm, "GuestStartPitch"))
                    {
                        header_buf->guest_start_pitch = atof(val);
                    }
                    /* GuestStartBank */
                    else if(!strcasecmp(parm, "GuestStartBank"))
                    {
                        header_buf->guest_start_bank = atof(val);
                    }


                    /* EndHeader */
                    else if(!strcasecmp(parm, "EndHeader"))
                    {
                        break;
                    }
                }
		continue;
	    }
            /* ***************************************************** */
            /* BeginXSWObject */
	    else if(!strcasecmp(parm, "BeginXSWObject") ||
	            !strcasecmp(parm, "BeginObject")
	    )
	    {
		/* Get object number. */
		object_num = atol(val);
		if((object_num < 0) || (object_num >= MAX_OBJECTS))
		{
                    fprintf(stderr,
           "%s: Line %i: Object number `%i' out of range.\n",
			file, lines_read, object_num
		    );
		    continue;
                }

		/* Allocate a new object in memory. */

		/* Need to allocate more object pointers? */
		if(object_num >= *total)
		{
		    prev = *total;
		    *total = object_num + 1;

		    ptr = (xsw_object_struct **)realloc(
			ptr,
			*total * sizeof(xsw_object_struct *)
		    );
		    if(ptr == NULL)
		    {
			*total = 0;
			continue;
		    }

		    /* Allocate each new object. */
		    for(i = prev; i < *total; i++)
		    {
			ptr[i] = (xsw_object_struct *)calloc(
                            1,
                            sizeof(xsw_object_struct)
                        );
			UNVResetObject(ptr[i]);
		    }
		}
		/* Allocate object structure as needed. */
		else if(ptr[object_num] == NULL)
		{
		    ptr[object_num] = (xsw_object_struct *)calloc(
			1,
			sizeof(xsw_object_struct)
		    );
		    if(ptr[object_num] == NULL)
		    {
			continue;
		    }
		}
		else
		{
                    fprintf(stderr,
              "%s: Line %i: Warning: Redefining object #%i.\n",
                        file, lines_read, object_num
                    );
		}

		/* Get pointer to object. */
		obj_ptr = ptr[object_num];

		UNVResetObject(obj_ptr);

		/* Reset weapons count. */
		weapons_count = 0;

		while(1)
                {
                    /* Free previous line and allocate/read next line. */
                    free(strptr); strptr = NULL;
                    strptr = FReadNextLineAllocCount(
                        fp, UNIXCFG_COMMENT_CHAR, &lines_read
                    );
                    if(strptr == NULL) break;

                    /* Fetch parameter. */
                    strptr2 = StringCfgParseParm(strptr);
                    if(strptr2 == NULL) continue;
                    strncpy(parm, strptr2, CFG_PARAMETER_MAX);   
                    parm[CFG_PARAMETER_MAX - 1] = '\0';

                    /* Fetch value. */
                    strptr2 = StringCfgParseValue(strptr);
                    /* Set value to "0" if NULL pointer was returned. */
                    if(strptr2 == NULL) strptr2 = "0";
                    strncpy(val, strptr2, CFG_VALUE_MAX);
                    val[CFG_VALUE_MAX - 1] = '\0';


		    /* Name */
		    if(!strcasecmp(parm, "Name"))
		    {
                        strncpy(
			    obj_ptr->name,
			    val,
			    XSW_OBJ_NAME_MAX
			);
			obj_ptr->name[XSW_OBJ_NAME_MAX - 1] = '\0';
		    }
                    /* Password */
                    else if(!strcasecmp(parm, "Password"))
                    {
                        strncpy(
			    obj_ptr->password,
			    val,
			    XSW_OBJ_PASSWORD_MAX
			);
			obj_ptr->password[
			    XSW_OBJ_PASSWORD_MAX - 1] = '\0';
                    }
                    /* Empire */
                    else if(!strcasecmp(parm, "Empire"))
                    {
			strncpy(
			    obj_ptr->empire,
			    val,
			    XSW_OBJ_EMPIRE_MAX
			);
			obj_ptr->empire[
			    XSW_OBJ_EMPIRE_MAX - 1] = '\0';

			/* Make sure upper case. */
			strtoupper(obj_ptr->empire);
                    }
		    /* ELink */
                    else if(!strcasecmp(parm, "ELink"))
                    {
			free(obj_ptr->elink);
			obj_ptr->elink = StringCopyAlloc(val);
                    }
                    /* Type */  
                    else if(!strcasecmp(parm, "Type"))
                    {
			obj_ptr->type = atoi(val);

			/* Type must be a non-garbage. */
			if(obj_ptr->type <= XSW_OBJ_TYPE_GARBAGE)
			{
			    obj_ptr->type =
			        XSW_OBJ_TYPE_STATIC;
			}
		    }
                    /* LocType */
                    else if(!strcasecmp(parm, "LocType"))
                    {
                        obj_ptr->loc_type = atoi(val);

                        /* Location type must be valid. */
                        if(obj_ptr->loc_type <
			    XSW_LOC_TYPE_SPACE)
                        {
                            obj_ptr->loc_type =
                                XSW_LOC_TYPE_SPACE;
                        }
                    }
                    /* ImageSet */
                    else if(!strcasecmp(parm, "ImageSet"))
                    {
                        obj_ptr->imageset = atoi(val);
                    }
                    /* Owner */
                    else if(!strcasecmp(parm, "Owner"))
                    {
                        obj_ptr->owner = atoi(val);
			if(obj_ptr->owner >= MAX_OBJECTS)
			    obj_ptr->owner = -1;
                    }
                    /* Size */
                    else if(!strcasecmp(parm, "Size"))
                    {
                        obj_ptr->size = atol(val);
			if(obj_ptr->size < 1)
			    obj_ptr->size = 1;
                    }
                    /* LockedOn */
                    else if(!strcasecmp(parm, "LockedOn"))
                    {
			obj_ptr->locked_on = atoi(val);
                        if(obj_ptr->locked_on >= MAX_OBJECTS)
                            obj_ptr->locked_on = -1;
                    }
                    /* InterceptingObject */
                    else if(!strcasecmp(parm, "InterceptingObject"))
                    {
                        obj_ptr->intercepting_object = atoi(val);
                        if(obj_ptr->intercepting_object >=
                            MAX_OBJECTS)
                            obj_ptr->intercepting_object = -1;
                    }
		    /* ScannerRange */
                    else if(!strcasecmp(parm, "ScannerRange"))
                    {
                        obj_ptr->scanner_range =
			    atof(val);
			if(obj_ptr->scanner_range < 1)
			    obj_ptr->scanner_range = 1;
                    }
		    /* SectX */
                    else if(!strcasecmp(parm, "SectX"))
                    {
                        obj_ptr->sect_x = atol(val);
                    }
                    /* SectY */
                    else if(!strcasecmp(parm, "SectY"))
                    {
                        obj_ptr->sect_y = atol(val);
                    }
                    /* SectZ */
                    else if(!strcasecmp(parm, "SectZ"))
                    {
                        obj_ptr->sect_z = atol(val);
                    }
                    /* CoordX */
                    else if(!strcasecmp(parm, "CoordX"))
                    {
                        obj_ptr->x = atof(val);
                    }
                    /* CoordY */
                    else if(!strcasecmp(parm, "CoordY"))
                    {
                        obj_ptr->y = atof(val);
                    }
                    /* CoordZ */
                    else if(!strcasecmp(parm, "CoordZ"))
                    {
                        obj_ptr->z = atof(val);
                    }
                    /* ObjectHeading (in radians). */
                    else if(!strcasecmp(parm, "ObjectHeading"))
                    {
                        obj_ptr->heading = atof(val);
			MuSetUnitVector2D(
			    &obj_ptr->attitude_vector_compoent,
			    obj_ptr->heading
			);
                    }
                    /* ObjectPitch (in radians). */
                    else if(!strcasecmp(parm, "ObjectPitch"))
                    {
                        obj_ptr->pitch = atof(val);
/* Update attitude vector compoents? */
                    }
                    /* ObjectBank (in radians). */
                    else if(!strcasecmp(parm, "ObjectBank"))
                    {
                        obj_ptr->bank = atof(val);
/* Update attitude vector compoents? */
                    }
                    /* VelocityHeading (in radians) */
                    else if(!strcasecmp(parm, "VelocityHeading"))
                    {
                        obj_ptr->velocity_heading = atof(val);
                        MuSetUnitVector2D(
                            &obj_ptr->momentum_vector_compoent,
                            obj_ptr->velocity_heading
                        );
                    }
                    /* VelocityPitch (in radians) */
                    else if(!strcasecmp(parm, "VelocityPitch"))
                    {
                        obj_ptr->velocity_pitch = atof(val);
/* Update momentum vector compoents? */
                    }
                    /* VelocityBank (in radians) */
                    else if(!strcasecmp(parm, "VelocityBank"))
                    {
                        obj_ptr->velocity_bank = atof(val);
/* Update momentum vector compoents? */
                    }
                    /* VelocityMax */
                    else if(!strcasecmp(parm, "VelocityMax"))
                    {
                        obj_ptr->velocity_max = atof(val);
                    }
                    /* Velocity */
                    else if(!strcasecmp(parm, "Velocity"))
                    {
                        obj_ptr->velocity = atof(val);
                    }
                    /* ThrustDir */
                    else if(!strcasecmp(parm, "ThrustDir"))
                    {
                        obj_ptr->thrust_dir = atof(val);
                    }
                    /* ThrustPower */
                    else if(!strcasecmp(parm, "ThrustPower"))
                    {
                        obj_ptr->thrust_power = atof(val);
                    }
		    /* Thrust */
                    else if(!strcasecmp(parm, "Thrust"))
                    {
                        obj_ptr->thrust = atof(val);
                    }
                    /* Throttle */
                    else if(!strcasecmp(parm, "Throttle"))
                    {
                        obj_ptr->throttle = atof(val);
                    }
                    /* EngineState */
                    else if(!strcasecmp(parm, "EngineState"))
                    {
                        obj_ptr->engine_state = atoi(val);
                    }
                    /* TurnRate */
                    else if(!strcasecmp(parm, "TurnRate"))
                    {
                        obj_ptr->turnrate = atof(val);
                    }
                    /* Lighting */
                    else if(!strcasecmp(parm, "Lighting"))
                    {
                        obj_ptr->lighting = atoi(val);
                    }
                    /* BrakesState */
                    else if(!strcasecmp(parm, "BrakesState"))
                    {
			/* No longer supported. */
                    }
                    /* BrakesPower */
                    else if(!strcasecmp(parm, "BrakesPower"))
                    {
                        /* No longer supported. */
                    }
		    /* HitPointsMax */
                    else if(!strcasecmp(parm, "HitPointsMax"))
                    {
                        obj_ptr->hp_max = atof(val);
                    }
                    /* HitPoints */
                    else if(!strcasecmp(parm, "HitPoints"))
                    {
                        obj_ptr->hp = atof(val);
                    }
                    /* PowerPurity */
                    else if(!strcasecmp(parm, "PowerPurity"))
                    {
                        obj_ptr->power_purity = atof(val);
                    }
                    /* PowerMax */
                    else if(!strcasecmp(parm, "PowerMax"))
                    {
                        obj_ptr->power_max = atof(val);
                    }
                    /* Power */
                    else if(!strcasecmp(parm, "Power"))
                    {
                        obj_ptr->power = atof(val);
                    }
                    /* CoreEfficency */
                    else if(!strcasecmp(parm, "CoreEfficency"))
                    {
                        obj_ptr->core_efficency = atof(val);
                    }
                    /* AntimatterMax */
                    else if(!strcasecmp(parm, "AntimatterMax"))
                    {
                        obj_ptr->antimatter_max = atof(val);
                    }
                    /* Antimatter */
                    else if(!strcasecmp(parm, "Antimatter"))
                    {
                        obj_ptr->antimatter = atof(val);
                    }
                    /* ShieldState */
                    else if(!strcasecmp(parm, "ShieldState"))
                    {
                        obj_ptr->shield_state = atoi(val);
                    }
                    /* ShieldFrequency */
                    else if(!strcasecmp(parm, "ShieldFrequency"))
                    {
                        obj_ptr->shield_frequency = atof(val);
                    }
		    /* SelectedWeapon */
                    else if(!strcasecmp(parm, "SelectedWeapon"))
                    {
                        obj_ptr->selected_weapon = atoi(val);
                    }
                    /* TotalWeapons */
                    else if(!strcasecmp(parm, "TotalWeapons"))
                    {
			if(UNVAllocObjectWeapons(obj_ptr, atoi(val)))
			{
                            fprintf(stderr,
                        "%s: Line %i: Error allocating %s weapons.\n",
                                file, lines_read, val
                            );

			    obj_ptr->total_weapons = 0;
			    obj_ptr->selected_weapon = -1;
			}
                    }
                    /* BirthTime */
                    else if(!strcasecmp(parm, "BirthTime"))
                    {
                        obj_ptr->birth_time = atol(val);
                    }
                    /* LifeSpan */
                    else if(!strcasecmp(parm, "LifeSpan"))
                    {
                        obj_ptr->lifespan = atol(val);
                    }
		    /* CreationOCS */
		    else if(!strcasecmp(parm, "CreationOCS"))
                    {
                        obj_ptr->creation_ocs = atoi(val);
                    }
                    /* CloakState */
                    else if(!strcasecmp(parm, "CloakState"))
                    {
                        obj_ptr->cloak_state = atoi(val);
                    }
                    /* CloakStrength */
                    else if(!strcasecmp(parm, "CloakStrength"))
                    {
                        obj_ptr->cloak_strength = atof(val);
			if(obj_ptr->cloak_strength < 0)
			    obj_ptr->cloak_strength = 0;
                    }
                    /* ShieldVisibility */
                    else if(!strcasecmp(parm, "ShieldVisibility"))
                    {
                        obj_ptr->shield_visibility = atof(val);
			if(obj_ptr->shield_visibility < 0)
			    obj_ptr->shield_visibility = 0;
                    }
                    /* CurrentVisibility */
                    else if(!strcasecmp(parm, "CurrentVisibility"))
                    {
                        obj_ptr->cur_visibility = atof(val);
			if(obj_ptr->cur_visibility < 0)
			    obj_ptr->cur_visibility = 0;
                    }
                    /* Visibility */
                    else if(!strcasecmp(parm, "Visibility"))
                    {
                        obj_ptr->visibility = atof(val);
			if(obj_ptr->visibility < 0)
			    obj_ptr->visibility = 0;
                    }
                    /* DamageControl */
                    else if(!strcasecmp(parm, "DamageControl"))
                    {
                        obj_ptr->damage_control = atoi(val);
                    }
		    /* ComChannel */
                    else if(!strcasecmp(parm, "ComChannel"))
                    {
                        obj_ptr->com_channel = atoi(val);
			if(((double)obj_ptr->com_channel / 100) > SWR_FREQ_MAX)
			    obj_ptr->com_channel = static_cast<int>(SWR_FREQ_MAX / 100);
                        else if(((double)obj_ptr->com_channel / 100) < SWR_FREQ_MIN)
                            obj_ptr->com_channel = static_cast<int>(SWR_FREQ_MIN / 100);
                    }
                    /* AIFlags */
                    else if(!strcasecmp(parm, "AIFlags"))
                    {
                        obj_ptr->ai_flags = atol(val);
                    }

		    /* *************************************************** */
		    /* ScoreCredits */
		    else if(!strcasecmp(parm, "ScoreCredits"))
		    {
			if(!UNVAllocScores(obj_ptr))
			{
			    obj_ptr->score->credits = atof(val);
			}
		    }
                    /* ScoreRMUMax */
                    else if(!strcasecmp(parm, "ScoreRMUMax"))
                    {
                        if(!UNVAllocScores(obj_ptr))
                        {
                            obj_ptr->score->rmu_max = atof(val);
			}
                    }
                    /* ScoreRMU */
                    else if(!strcasecmp(parm, "ScoreRMU"))
                    {
                        if(!UNVAllocScores(obj_ptr))
                        {   
                            obj_ptr->score->rmu = atof(val);
			}
                    }
                    /* ScoreDamageGiven */
                    else if(!strcasecmp(parm, "ScoreDamageGiven") ||
                            !strcasecmp(parm, "ScoreDamageGivin")
		    )
                    {
                        if(!UNVAllocScores(obj_ptr))
                        {
                            obj_ptr->score->damage_given = atof(val);
			}
                    }
                    /* ScoreDamageRecieved */
                    else if(!strcasecmp(parm, "ScoreDamageRecieved"))
                    {
                        if(!UNVAllocScores(obj_ptr))
                        {   
                            obj_ptr->score->damage_recieved = atof(val);
			}
                    }
                    /* ScoreKills */
                    else if(!strcasecmp(parm, "ScoreKills"))
                    {
                        if(!UNVAllocScores(obj_ptr))
                        {
                            obj_ptr->score->kills = atoi(val);
			}
                    }
                    /* *************************************************** */
                    /* PermissionUID */
                    else if(!strcasecmp(parm, "PermissionUID"))
                    {
                        obj_ptr->permission.uid = atoi(val);
                    }
                    /* PermissionGID */
                    else if(!strcasecmp(parm, "PermissionGID"))
                    {
                        obj_ptr->permission.gid = atoi(val);
                    }
                    /* *************************************************** */
                    /* AnimationLastInterval */
                    else if(!strcasecmp(parm, "AnimationLastInterval"))
                    {
                        obj_ptr->animation.last_interval = atol(val);
                    }
                    /* AnimationInterval */
                    else if(!strcasecmp(parm, "AnimationInterval"))
                    {
                        obj_ptr->animation.interval = atol(val);
                    }
                    /* AnimationCurrentFrame */
                    else if(!strcasecmp(parm, "AnimationCurrentFrame"))
                    {
                        obj_ptr->animation.current_frame = atol(val);
                    }
                    /* AnimationTotalFrames */
                    else if(!strcasecmp(parm, "AnimationTotalFrames"))
                    {
                        obj_ptr->animation.total_frames = atol(val);
                    }
                    /* AnimationCycleCount */
                    else if(!strcasecmp(parm, "AnimationCycleCount"))
                    {
                        obj_ptr->animation.cycle_count = atol(val);
                    }
                    /* AnimationCycleTimes */
                    else if(!strcasecmp(parm, "AnimationCycleTimes"))
                    {
                        obj_ptr->animation.cycle_times = atol(val);
                    }
		    /* **************************************************** */
		    /* BeginWeapon */
		    else if(!strcasecmp(parm, "BeginWeapon"))
		    {
			while(1)
                        {
			    /* Free previous line and allocate/read next line. */
                            free(strptr); strptr = NULL;
                            strptr = FReadNextLineAllocCount(
                                fp, UNIXCFG_COMMENT_CHAR, &lines_read
                            );
                            if(strptr == NULL) break;

                            /* Fetch parameter. */
                            strptr2 = StringCfgParseParm(strptr);
                            if(strptr2 == NULL) continue;
                            strncpy(parm, strptr2, CFG_PARAMETER_MAX);
                            parm[CFG_PARAMETER_MAX - 1] = '\0';

                            /* Fetch value. */
                            strptr2 = StringCfgParseValue(strptr); 
                            /* Set value to "0" if NULL pointer was returned. */
                            if(strptr2 == NULL) strptr2 = "0";
                            strncpy(val, strptr2, CFG_VALUE_MAX);
                            val[CFG_VALUE_MAX - 1] = '\0';


			    /* Flags */
                            if(!strcasecmp(parm, "Flags"))
                            {
                                if(weapons_count < obj_ptr->total_weapons)
				     obj_ptr->weapons[weapons_count]->flags
                                        = atol(val);
                            }
                            /* EmissionType */
                            else if(!strcasecmp(parm, "EmissionType"))
                            {
                                if(weapons_count < obj_ptr->total_weapons)
                                    obj_ptr->weapons[weapons_count]->emission_type
			                = atoi(val);
                            }
                            /* OCSCode */
                            else if(!strcasecmp(parm, "OCSCode"))
                            {
                                if(weapons_count < obj_ptr->total_weapons)
                                    obj_ptr->weapons[weapons_count]->ocs_code
                                        = atoi(val);
                            }
                            /* Amount */
                            else if(!strcasecmp(parm, "Amount"))
                            {
                                if(weapons_count < obj_ptr->total_weapons)
                                    obj_ptr->weapons[weapons_count]->amount
                                        = atoi(val);
                            }
                            /* Max */
                            else if(!strcasecmp(parm, "Max"))
                            {
                                if(weapons_count < obj_ptr->total_weapons)
                                    obj_ptr->weapons[weapons_count]->max
                                        = atoi(val);
                            }
                            /* CreatePower */
                            else if(!strcasecmp(parm, "CreatePower"))
                            {
                                if(weapons_count < obj_ptr->total_weapons)
                                    obj_ptr->weapons[weapons_count]->create_power
                                        = atof(val);
                            }
                            /* Power */
                            else if(!strcasecmp(parm, "Power"))
                            {
                                if(weapons_count < obj_ptr->total_weapons)
                                    obj_ptr->weapons[weapons_count]->power
                                        = atof(val);
                            }
                            /* Range */
                            else if(!strcasecmp(parm, "Range"))
                            {
                                if(weapons_count < obj_ptr->total_weapons)
                                    obj_ptr->weapons[weapons_count]->range
                                        = atol(val);
                            }
                            /* Delay */
                            else if(!strcasecmp(parm, "Delay"))
                            {
                                if(weapons_count < obj_ptr->total_weapons)
                                    obj_ptr->weapons[weapons_count]->delay
                                        = atol(val);
                            }
                            /* LastUsed */
                            else if(!strcasecmp(parm, "LastUsed"))
                            {
                                if(weapons_count < obj_ptr->total_weapons)
                                    obj_ptr->weapons[weapons_count]->last_used
                                        = atol(val);
			    }
                            /* FireSoundCode */
                            else if(!strcasecmp(parm, "FireSoundCode"))
                            {
                                if(weapons_count < obj_ptr->total_weapons)
				    obj_ptr->weapons[weapons_count]->fire_sound_code
					= atoi(val);
                            }
			    /* EndWeapon */
			    else if(!strcasecmp(parm, "EndWeapon"))
			    {
			        weapons_count++;
			        break;
			    }
                            /* Unsupported property */
                            else
                            {
                                fprintf(stderr,
                      "%s: Line %i: Warning: Unsupported parameter `%s'\n",
                                    file, lines_read, parm
                                );
                            }
		        }
		    }
		    /* ************************************************ */
		    /* BeginEcoData */
                    else if(!strcasecmp(parm, "BeginEcoData"))
                    {
			/* Allocate economy structure on object as needed. */
		        if(UNVAllocEco(obj_ptr))
                            continue;

		        eco_ptr = obj_ptr->eco;
		        if(eco_ptr == NULL)
			    continue;

                        while(1)
                        {
                            /* Free previous line and allocate/read next line. */
                            free(strptr); strptr = NULL;
                            strptr = FReadNextLineAllocCount(
                                fp, UNIXCFG_COMMENT_CHAR, &lines_read
                            );
                            if(strptr == NULL) break;

                            /* Fetch parameter. */
                            strptr2 = StringCfgParseParm(strptr);
                            if(strptr2 == NULL) continue;
                            strncpy(parm, strptr2, CFG_PARAMETER_MAX);
                            parm[CFG_PARAMETER_MAX - 1] = '\0';

                            /* Fetch value. */
                            strptr2 = StringCfgParseValue(strptr);
                            /* Set value to "0" if NULL pointer was returned. */
                            if(strptr2 == NULL) strptr2 = "0";          
                            strncpy(val, strptr2, CFG_VALUE_MAX);
                            val[CFG_VALUE_MAX - 1] = '\0';


                            /* Flags */
		            if(!strcasecmp(parm, "Flags"))
                            {
                                eco_ptr->flags = atol(val);
                            }
                            /* TaxGeneral */
                            else if(!strcasecmp(parm, "TaxGeneral"))
                            {
                                eco_ptr->tax_general = atof(val);
                                if(eco_ptr->tax_general < 0.0) 
                                    eco_ptr->tax_general = 0.0;
                            }
                            /* TaxFriend */
                            else if(!strcasecmp(parm, "TaxFriend"))
                            {
                                eco_ptr->tax_friend = atof(val);
                                if(eco_ptr->tax_friend < 0.0)
                                    eco_ptr->tax_friend = 0.0;
                            }
                            /* TaxHostile */
                            else if(!strcasecmp(parm, "TaxHostile"))
                            {
                                eco_ptr->tax_hostile = atof(val);
                                if(eco_ptr->tax_hostile < 0.0)
                                    eco_ptr->tax_hostile = 0.0;
                            }
			    /* **************************************** */
			    /* BeginEcoProduct */
                            else if(!strcasecmp(parm, "BeginEcoProduct"))
                            {
                                /* Allocate an eco product. */
                                if(eco_ptr->total_products < 0)
                                  eco_ptr->total_products = 0;

                                product_num = eco_ptr->total_products;
                                eco_ptr->total_products++;

                                eco_ptr->product = (xsw_ecoproduct_struct **)realloc(
                                  eco_ptr->product,
                                  eco_ptr->total_products * sizeof(xsw_ecoproduct_struct *)
                                );
                                if(eco_ptr->product == NULL)
                                {
                                  eco_ptr->total_products = 0;
                                  continue;
                                }

                                eco_ptr->product[product_num] = (xsw_ecoproduct_struct *)
                                    calloc(1, sizeof(xsw_ecoproduct_struct));
                                if(eco_ptr->product[product_num] == NULL)
                                {
                                  continue;
                                }

                                while(1)
                                {
                                  /* Free previous line and allocate/read next line. */
                                  free(strptr); strptr = NULL;
                                  strptr = FReadNextLineAllocCount(
                                    fp, UNIXCFG_COMMENT_CHAR, &lines_read
                                  );
                                  if(strptr == NULL) break;

                                  /* Fetch parameter. */
                                  strptr2 = StringCfgParseParm(strptr);  
                                  if(strptr2 == NULL) continue;   
                                  strncpy(parm, strptr2, CFG_PARAMETER_MAX);
                                  parm[CFG_PARAMETER_MAX - 1] = '\0';

                                  /* Fetch value. */
                                  strptr2 = StringCfgParseValue(strptr);
                                  /* Set value to "0" if NULL pointer was returned. */
                                  if(strptr2 == NULL) strptr2 = "0";
                                  strncpy(val, strptr2, CFG_VALUE_MAX);
                                  val[CFG_VALUE_MAX - 1] = '\0';


                                  /* Name */
                                  if(!strcasecmp(parm, "Name"))
                                  {
                                    strncpy(
                                      eco_ptr->product[product_num]->name,
                                      val,
                                      ECO_PRODUCT_NAME_MAX
                                    );
                   eco_ptr->product[product_num]->name[ECO_PRODUCT_NAME_MAX - 1] = '\0';
                                  }
                                  /* SellPrice */
                                  else if(!strcasecmp(parm, "SellPrice"))
                                  {
                                    eco_ptr->product[product_num]->sell_price = atof(val);
                                  }
                                  /* BuyPrice */
                                  else if(!strcasecmp(parm, "BuyPrice"))
                                  {
                                    eco_ptr->product[product_num]->buy_price = atof(val);
                                  }
                                  /* Amount */
                                  else if(!strcasecmp(parm, "Amount"))
                                  {
                                    eco_ptr->product[product_num]->amount = atof(val);
                                  }
                                  /* AmountMax */
                                  else if(!strcasecmp(parm, "AmountMax"))
                                  {
                                    eco_ptr->product[product_num]->amount_max = atof(val);
                                  }
                                  /* EndEcoProduct */
                                  else if(!strcasecmp(parm, "EndEcoProduct"))
                                  {
                                    break;
                                  }

                                }
                            }	/* BeginEcoProduct */
 
		            /* EndEcoData */
		            else if(!strcasecmp(parm, "EndEcoData"))
		            {
/*
                          DBSortEconomyProducts(object_num);
 */
			        break;
		            }
		        }
		    }

                    /* ***************************************************** */
                    /* EndXSWObject */
                    else if(!strcasecmp(parm, "EndXSWObject"))
                    {
                        break;
                    }
                    /* Unsupported property */
		    else
		    {
		        fprintf(stderr,
		  "%s: Line %i: Warning: Unsupported parameter `%s'\n",
                            file, lines_read, parm
	                );
		    }
                }
	    }
	}


        /* Call notify progress function. */
        if(progress_notify != NULL)
            progress_notify(
                client_data,
                (int)filesize,
		(int)filesize
	    );
                                  

	/* Close the file. */
	fclose(fp);
                                  

	/* Make sure lost and found owner is valid. */
	if(header_buf != NULL)
	{
	    i = header_buf->lost_found_owner;

	    if((i < 0) || (i >= *total))
	        n = 1;
	    else if(ptr[i] == NULL)
		n = 1;
	    else if(ptr[i]->type != XSW_OBJ_TYPE_PLAYER)
		n = 0;	/* Let this be okay for now, star charts may have no
			 * players for instance.
			 */
	    else
		n = 0;

	    if(n)
	    {
	        fprintf(
		    stderr,
    "%s: Warning: Lost and found owner object #%i is invalid.\n",
		    file, i
	        );
	        header_buf->lost_found_owner = 0;
	    }
	}


	return(ptr);
}



int UNVSaveToFile(
        char *file,
        xsw_object_struct **obj_ptr,
        int total,
        unv_head_struct *header_buf,
        void *client_data,
	void (*progress_notify)(void *, int, int)
)
{
	int i, n;
	FILE *fp;
	xsw_weapons_struct *wep_ptr;
	xsw_ecodata_struct *eco_ptr;



	if(file == NULL)
	    return(-1);

	/* Open file for writing. */
	fp = fopen(file, "w");
	if(fp == NULL)
            return(-1);


	/* Write header. */
        fprintf(fp, "# ShipWars Universe File\n");
        fprintf(fp, "#\n\n");

	if(header_buf != NULL)
	{
            fprintf(fp, "# Header\n");
            fprintf(fp, "BeginHeader\n");
            fprintf(fp, "    Title = %s\n", header_buf->title);
            fprintf(fp, "    Version = %s\n", header_buf->version);
            fprintf(fp, "    LostFoundOwner = %i\n",
		header_buf->lost_found_owner
	    );
	    if(header_buf->ru_to_au < 1)
	        fprintf(fp, "    UnitConversionRUTOAU = %.6f\n",
		    header_buf->ru_to_au
		);
	    else
	        fprintf(fp, "    UnitConversionRUTOAU = %.2f\n",
		    header_buf->ru_to_au
		);
            fprintf(fp, "    ImageSetFile = %s\n", header_buf->isr);
            fprintf(fp, "    OCSNFile = %s\n", header_buf->ocsn);
            fprintf(fp, "    SoundSchemeFile = %s\n", header_buf->ss);

            fprintf(fp, "    PlayerStartSectX = %ld\n",
		header_buf->player_start_sect_x);
            fprintf(fp, "    PlayerStartSectY = %ld\n",
                header_buf->player_start_sect_y);
            fprintf(fp, "    PlayerStartSectZ = %ld\n",
                header_buf->player_start_sect_z);
            fprintf(fp, "    PlayerStartX = %f\n",
                header_buf->player_start_x);
            fprintf(fp, "    PlayerStartY = %f\n",
                header_buf->player_start_y);
            fprintf(fp, "    PlayerStartZ = %f\n",
                header_buf->player_start_z);

            fprintf(fp, "    PlayerStartHeading = %f\n",
                header_buf->player_start_heading);
            fprintf(fp, "    PlayerStartPitch = %f\n",
                header_buf->player_start_pitch);
            fprintf(fp, "    PlayerStartBank = %f\n",
                header_buf->player_start_bank);

            fprintf(fp, "    GuestStartSectX = %ld\n",
                header_buf->guest_start_sect_x);
            fprintf(fp, "    GuestStartSectY = %ld\n",
                header_buf->guest_start_sect_y);
            fprintf(fp, "    GuestStartSectZ = %ld\n",
                header_buf->guest_start_sect_z);
            fprintf(fp, "    GuestStartX = %f\n",
                header_buf->guest_start_x);
            fprintf(fp, "    GuestStartY = %f\n",
                header_buf->guest_start_y);
            fprintf(fp, "    GuestStartZ = %f\n",
                header_buf->guest_start_z);

            fprintf(fp, "    GuestStartHeading = %f\n",
                header_buf->guest_start_heading);
            fprintf(fp, "    GuestStartPitch = %f\n",
                header_buf->guest_start_pitch);
            fprintf(fp, "    GuestStartBank = %f\n",
                header_buf->guest_start_bank);

            fprintf(fp, "EndHeader\n\n");
	}


	/* ********************************************************** */
        /* Begin writing objects to file. */
	for(i = 0; i < total; i++, obj_ptr++)
	{
            /* Call notify progress function. */   
            if(progress_notify != NULL)
                progress_notify(
                    client_data, 
                    i + 1,
                    total
                );

	    if(*obj_ptr == NULL)
		continue;

	    if((*obj_ptr)->type <= XSW_OBJ_TYPE_GARBAGE)
		continue;


	    /*   object_count is now of a valid object that needs
	     *   to be saved, begin saving xsw object number
	     *   object_count.
	     */

	    /* Comment. */
	    fprintf(fp, "# Object #%i\n", i);

	    /* BeginXSWObject */
	    fprintf(fp, "BeginXSWObject = %i\n", i);

	    /* Name */
	    fprintf(fp, "    Name = %s\n", (*obj_ptr)->name);

            /* Password */
            fprintf(fp, "    Password = %s\n", (*obj_ptr)->password);

	    /* Empire */
            strtoupper((*obj_ptr)->empire);	/* Make sure upper case. */
            fprintf(fp, "    Empire = %s\n", (*obj_ptr)->empire);

            /* ELink */
	    if((*obj_ptr)->elink != NULL)
		fprintf(fp, "    ELink = %s\n", (*obj_ptr)->elink);

            /* Type */
            fprintf(fp, "    Type = %i\n", (*obj_ptr)->type);

            /* LocType */
            fprintf(fp, "    LocType = %i\n", (*obj_ptr)->loc_type);

            /* ImageSet */
            fprintf(fp, "    ImageSet = %i\n", (*obj_ptr)->imageset);

            /* Owner */
            fprintf(fp, "    Owner = %i\n", (*obj_ptr)->owner);

            /* Size */
            fprintf(fp, "    Size = %ld\n", (*obj_ptr)->size);

            /* LockedOn */
            fprintf(fp, "    LockedOn = %i\n", (*obj_ptr)->locked_on);

            /* InterceptingObject */
            fprintf(fp, "    InterceptingObject = %i\n",
		(*obj_ptr)->intercepting_object
	    );
 
            /* ScannerRange */
            fprintf(fp, "    ScannerRange = %.4f\n",
                (*obj_ptr)->scanner_range
	    );

            /* SectX */
            fprintf(fp, "    SectX = %ld\n", (*obj_ptr)->sect_x);
            /* SectY */
            fprintf(fp, "    SectY = %ld\n", (*obj_ptr)->sect_y);
            /* SectZ */
            fprintf(fp, "    SectZ = %ld\n", (*obj_ptr)->sect_z);

            /* CoordX */
            fprintf(fp, "    CoordX = %.4f\n", (*obj_ptr)->x);
            /* CoordY */
            fprintf(fp, "    CoordY = %.4f\n", (*obj_ptr)->y);
            /* CoordZ */
            fprintf(fp, "    CoordZ = %.4f\n", (*obj_ptr)->z);

            /* ObjectHeading */
            fprintf(fp, "    ObjectHeading = %.4f\n", (*obj_ptr)->heading);  
            /* ObjectPitch */
            fprintf(fp, "    ObjectPitch = %.4f\n", (*obj_ptr)->pitch);
            /* ObjectBank */
            fprintf(fp, "    ObjectBank = %.4f\n", (*obj_ptr)->bank);

            /* VelocityHeading */
            fprintf(fp, "    VelocityHeading = %.4f\n",
                (*obj_ptr)->velocity_heading
	    );  
            /* VelocityPitch */
            fprintf(fp, "    VelocityPitch = %.4f\n",
                (*obj_ptr)->velocity_pitch
	    );
            /* VelocityBank */
            fprintf(fp, "    VelocityBank = %.4f\n",
                (*obj_ptr)->velocity_bank
	    );
                   
            /* VelocityMax */
            fprintf(fp, "    VelocityMax = %.4f\n",
                (*obj_ptr)->velocity_max
	    );
            /* Velocity */
            fprintf(fp, "    Velocity = %.4f\n",
                (*obj_ptr)->velocity
	    );
            /* ThrustDir */
            fprintf(fp, "    ThrustDir = %.4f\n",
                (*obj_ptr)->thrust_dir
	    );
            /* ThrustPower */
            fprintf(fp, "    ThrustPower = %.4f\n",
                (*obj_ptr)->thrust_power
	    );
            /* Thrust */
            fprintf(fp, "    Thrust = %.4f\n",
                (*obj_ptr)->thrust
	    );
            /* Throttle */
            fprintf(fp, "    Throttle = %.4f\n",
                (*obj_ptr)->throttle
	    );
            /* EngineState */
            fprintf(fp, "    EngineState = %i\n",
                (*obj_ptr)->engine_state
	    );
            /* TurnRate */
            fprintf(fp, "    TurnRate = %.4f\n",
                (*obj_ptr)->turnrate
	    );
            /* Lighting */
            fprintf(fp, "    Lighting = %i\n",
                (*obj_ptr)->lighting
	    );
            /* HitPointsMax */
            fprintf(fp, "    HitPointsMax = %.4f\n",
                (*obj_ptr)->hp_max
	    );
            /* HitPoints */
            fprintf(fp, "    HitPoints = %.4f\n",
                (*obj_ptr)->hp
	    );
            /* PowerPurity */
            fprintf(fp, "    PowerPurity = %.4f\n",
                (*obj_ptr)->power_purity
	    );
            /* PowerMax */
            fprintf(fp, "    PowerMax = %.4f\n",
                (*obj_ptr)->power_max
	    );
            /* Power */
            fprintf(fp, "    Power = %.4f\n",
                (*obj_ptr)->power
	    );
            /* CoreEfficency */
            fprintf(fp, "    CoreEfficency = %.4f\n",
                (*obj_ptr)->core_efficency
	    );
            /* AntimatterMax */
            fprintf(fp, "    AntimatterMax = %.4f\n",
                (*obj_ptr)->antimatter_max
	    );
            /* Antimatter */
            fprintf(fp, "    Antimatter = %.4f\n",
                (*obj_ptr)->antimatter
	    );
            /* ShieldState */
            fprintf(fp, "    ShieldState = %i\n",
                (*obj_ptr)->shield_state
	    );
            /* ShieldFrequency */
            fprintf(fp, "    ShieldFrequency = %.4f\n",
                (*obj_ptr)->shield_frequency
	    );

            /* SelectedWeapon */
	    if((*obj_ptr)->selected_weapon >= MAX_WEAPONS)
		(*obj_ptr)->selected_weapon = MAX_WEAPONS - 1;
	    if((*obj_ptr)->selected_weapon < -1)
		(*obj_ptr)->selected_weapon = -1;
            fprintf(fp, "    SelectedWeapon = %i\n",
                (*obj_ptr)->selected_weapon
	    );

            /* TotalWeapons */ 
	    if((*obj_ptr)->total_weapons > MAX_WEAPONS)
		(*obj_ptr)->total_weapons = MAX_WEAPONS;
	    if((*obj_ptr)->total_weapons < 0)
		(*obj_ptr)->total_weapons = 0;
            fprintf(fp, "    TotalWeapons = %i\n",
                (*obj_ptr)->total_weapons
	    );

            /* BirthTime */
            fprintf(fp, "    BirthTime = %ld\n",
                (*obj_ptr)->birth_time
	    );
            /* LifeSpan */
            fprintf(fp, "    LifeSpan = %ld\n",
                (*obj_ptr)->lifespan
	    );
	    /* CreationOCS */
	    fprintf(fp, "    CreationOCS = %i\n",
		(*obj_ptr)->creation_ocs
	    );
            /* CloakState */
            fprintf(fp, "    CloakState = %i\n",
                (*obj_ptr)->cloak_state
	    );
            /* CloakStrength */
            fprintf(fp, "    CloakStrength = %.4f\n",
                (*obj_ptr)->cloak_strength
	    );
            /* ShieldVisibility */
            fprintf(fp, "    ShieldVisibility = %.4f\n",
                (*obj_ptr)->shield_visibility
	    );
	    /* CurrentVisibility */
            fprintf(fp, "    CurrentVisibility = %.4f\n",
                (*obj_ptr)->cur_visibility
	    );
            /* Visibility */
            fprintf(fp, "    Visibility = %.4f\n",
                (*obj_ptr)->visibility
	    );
            /* DamageControl */
            fprintf(fp, "    DamageControl = %i\n",
                (*obj_ptr)->damage_control
	    );
            /* ComChannel */
            fprintf(fp, "    ComChannel = %i\n",
                (*obj_ptr)->com_channel
	    );
	    /* AIFlags */
            fprintf(fp, "    AIFlags = %ld\n",
                (unsigned long)(*obj_ptr)->ai_flags
	    );

	    /* Scores. */
	    if((*obj_ptr)->score != NULL)
	    {
	        /* ScoreCredits */
                fprintf(fp, "    ScoreCredits = %.4f\n",
                    (*obj_ptr)->score->credits
		);
                /* ScoreRMUMax */
                fprintf(fp, "    ScoreRMUMax = %.4f\n",
		    (*obj_ptr)->score->rmu_max
		);
                /* ScoreRMU */
                fprintf(fp, "    ScoreRMU = %.4f\n",
                    (*obj_ptr)->score->rmu
		);
                /* ScoreDamageGiven */
                fprintf(fp, "    ScoreDamageGiven = %.4f\n",
                    (*obj_ptr)->score->damage_given
		);
                /* ScoreDamageRecieved */
                fprintf(fp, "    ScoreDamageRecieved = %.4f\n",
                    (*obj_ptr)->score->damage_recieved
		);
                /* ScoreKills */
                fprintf(fp, "    ScoreKills = %i\n",
                    (*obj_ptr)->score->kills
		);
	    }

	    /* Permission stuff. */
            /* PermissionUID */
            fprintf(fp, "    PermissionUID = %i\n",
                (*obj_ptr)->permission.uid
	    );
            /* PermissionGID */
            fprintf(fp, "    PermissionGID = %i\n",
                (*obj_ptr)->permission.gid
	    ); 

            /* Animation stuff. */
	    /* AnimationLastInterval */
            fprintf(fp, "    AnimationLastInterval = %ld\n",
                (*obj_ptr)->animation.last_interval
	    );

            /* AnimationInterval */
            fprintf(fp, "    AnimationInterval = %ld\n",
                (*obj_ptr)->animation.interval
	    );
            /* AnimationCurrentFrame */
            fprintf(fp, "    AnimationCurrentFrame = %i\n",
                (*obj_ptr)->animation.current_frame
	    );
            /* AnimationTotalFrames */
            fprintf(fp, "    AnimationTotalFrames = %i\n",
                (*obj_ptr)->animation.total_frames
	    );
            /* AnimationCycleCount */
            fprintf(fp, "    AnimationCycleCount = %i\n",
                (*obj_ptr)->animation.cycle_count
	    );
            /* AnimationCycleTimes */
            fprintf(fp, "    AnimationCycleTimes = %i\n",
                (*obj_ptr)->animation.cycle_times
	    );

 	    /* Weapons. */
	    for(n = 0; n < (*obj_ptr)->total_weapons; n++)
	    {
		wep_ptr = (*obj_ptr)->weapons[n];
		if(wep_ptr == NULL)
		    continue;

		fprintf(fp, "    BeginWeapon\n");

                fprintf(fp, "        Flags = %ld\n",
                    (unsigned long)wep_ptr->flags
                );
                fprintf(fp, "        EmissionType = %i\n",
                    wep_ptr->emission_type
		);
                fprintf(fp, "        OCSCode = %i\n",
                    wep_ptr->ocs_code
		);
                fprintf(fp, "        Amount = %i\n",
                    wep_ptr->amount
		);
                fprintf(fp, "        Max = %i\n",
                    wep_ptr->max
		);
                fprintf(fp, "        CreatePower = %.4f\n",
                    wep_ptr->create_power
		);
                fprintf(fp, "        Power = %.4f\n",
                    wep_ptr->power
		);
                fprintf(fp, "        Range = %ld\n",
                    wep_ptr->range
		);
                fprintf(fp, "        Delay = %ld\n",
                    wep_ptr->delay
		);
                fprintf(fp, "        LastUsed = 0\n");	/* Always 0. */
                fprintf(fp, "        FireSoundCode = %i\n",
		    wep_ptr->fire_sound_code
		);

                fprintf(fp, "    EndWeapon\n");
	    }

	    /* Economy. */
	    eco_ptr = (*obj_ptr)->eco;
	    if(eco_ptr != NULL)
	    {
                fprintf(fp, "    BeginEcoData\n");
                fprintf(fp, "        Flags = %ld\n",
		    eco_ptr->flags
		);
                fprintf(fp, "        TaxGeneral = %.4f\n",
                    eco_ptr->tax_general
                );
                fprintf(fp, "        TaxFriend = %.4f\n",
                    eco_ptr->tax_friend
                );
                fprintf(fp, "        TaxHostile = %.4f\n",
                    eco_ptr->tax_hostile
                );

		/* Products. */
		for(n = 0; n < eco_ptr->total_products; n++)
		{
		    if(eco_ptr->product[n] == NULL)
			continue;

                    fprintf(fp, "        BeginEcoProduct\n");
                    fprintf(fp, "            Name = %s\n",
			eco_ptr->product[n]->name
		    );
                    fprintf(fp, "            SellPrice = %.4f\n", 
                        eco_ptr->product[n]->sell_price
                    );
                    fprintf(fp, "            BuyPrice = %.4f\n",
                        eco_ptr->product[n]->buy_price
                    );
                    fprintf(fp, "            Amount = %.4f\n",
                        eco_ptr->product[n]->amount
                    );
                    fprintf(fp, "            AmountMax = %.4f\n",
                        eco_ptr->product[n]->amount_max
                    );

                    fprintf(fp, "        EndEcoProduct\n");
		}

                fprintf(fp, "    EndEcoData\n");
	    }


            /* EndXSWObject */
            fprintf(fp, "EndXSWObject\n\n");
	}


        fclose(fp);   


	return(0);
}




