// global/unvmain.cpp
/*
                   Universe Objects Management Main

	Functions:

	void UNVSetupGarbageObject()

	int UNVInit(int argc, char *argv[])
	void UNVShutdown()

	---

 */

#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
          
#include "../include/string.h"
#include "../include/reality.h"   
#include "../include/objects.h"
#include "../include/isrefs.h"

#include "../include/unvmain.h"
#include "../include/unvutil.h"
#include "../include/unvmatch.h"
#include "../include/unvfile.h"



xsw_object_struct unv_garbage_object;


/*
 *	Sets up the global garbage object.
 */
void UNVSetupGarbageObject()
{
        xsw_object_struct *obj_ptr;


        obj_ptr = &unv_garbage_object;


        obj_ptr->type = XSW_OBJ_TYPE_GARBAGE;

        obj_ptr->client_options = 0;
        obj_ptr->server_options = 0;

        memset(obj_ptr->name, '\0', XSW_OBJ_NAME_MAX);
        memset(obj_ptr->password, '\0', XSW_OBJ_PASSWORD_MAX);
        strncpy(
            obj_ptr->empire,
            XSW_DEF_EMPIRE_STR,
            XSW_OBJ_EMPIRE_MAX
        );
	obj_ptr->elink = NULL;
        
        obj_ptr->last_updated = 0; 
        
        obj_ptr->loc_type = XSW_LOC_TYPE_SPACE;
        obj_ptr->imageset = ISREF_DEFAULT;
        obj_ptr->owner = -1;
        obj_ptr->size = 1;
        obj_ptr->locked_on = -1;
        obj_ptr->intercepting_object = -1;

        obj_ptr->scanner_range = 8.0;
        obj_ptr->sect_x = 0;
        obj_ptr->sect_y = 0;
        obj_ptr->sect_z = 0;
        obj_ptr->x = 0;
        obj_ptr->y = 0;
        obj_ptr->z = 0;
        obj_ptr->heading = 0;
        obj_ptr->pitch = 0;
        obj_ptr->bank = 0;
        memset(
	    &obj_ptr->attitude_vector_compoent,
	    0x00,
	    sizeof(xsw_vector_compoent_struct)
	);
        obj_ptr->velocity = 0;
        obj_ptr->velocity_max = 0;
        obj_ptr->velocity_heading = 0;
        obj_ptr->velocity_pitch = 0;
        obj_ptr->velocity_bank = 0;
        memset(
	    &obj_ptr->momentum_vector_compoent,
	    0x00,
	    sizeof(xsw_vector_compoent_struct)
	);
        obj_ptr->thrust_dir = 3.1415927;
        obj_ptr->thrust = 0;
        obj_ptr->thrust_power = 0;
        obj_ptr->thrust_power = 0;
        obj_ptr->throttle = 0;
        obj_ptr->engine_state = ENGINE_STATE_ON;
        obj_ptr->turnrate = 0.0003;
        obj_ptr->hp = 1;
        obj_ptr->hp_max = 1;
        obj_ptr->power = 0;
        obj_ptr->power_max = 0;
        obj_ptr->power_purity = 1.0;
        obj_ptr->core_efficency = 0;
        obj_ptr->antimatter = 0;
        obj_ptr->antimatter_max = 0; 
        obj_ptr->shield_state = SHIELD_STATE_NONE;
        obj_ptr->shield_frequency = XSW_DEF_SHIELD_FREQ;
        obj_ptr->selected_weapon = -1;
        obj_ptr->total_weapons = 0;
        obj_ptr->birth_time = 0;
        obj_ptr->lifespan = -1;
	obj_ptr->creation_ocs = 0;
        obj_ptr->cloak_state = CLOAK_STATE_NONE;
        obj_ptr->cloak_strength = 0.0;
        obj_ptr->shield_visibility = 0.0;
        obj_ptr->visibility = 1.0;
        obj_ptr->cur_visibility = 1.0;
        obj_ptr->damage_control = DMGCTL_STATE_OFF;
        obj_ptr->com_channel = XSW_DEF_COM_CHANNEL;
        obj_ptr->ai_flags = 0;
        
        obj_ptr->tractored_object = NULL;
        obj_ptr->total_tractored_objects = 0;
 
        obj_ptr->permission.uid = DEFAULT_UID;
        obj_ptr->permission.gid = DEFAULT_GID;
        
        obj_ptr->animation.interval = 500;
        obj_ptr->animation.last_interval = 0;
        obj_ptr->animation.current_frame = 0;   
        obj_ptr->animation.total_frames = 1;
        obj_ptr->animation.cycle_count = 0;
        obj_ptr->animation.cycle_times = -1;    /* Loop infinatly. */
        
        obj_ptr->weapons = NULL;
        obj_ptr->score = NULL;
        obj_ptr->eco = NULL;
        
        
        return;
}

/*
 *	Initializes universe object management resources.
 */
int UNVInit(int argc, char *argv[])
{
	/* Reset global garbage object values. */
	UNVSetupGarbageObject();


	return(0);
}


/*
 *	Shuts down and deallocates universe object management
 *	resources.
 */
void UNVShutdown() 
{



	return;
}




