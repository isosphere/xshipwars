#include "../include/unvmatch.h"
#include "../include/swnetcodes.h"
#include "swserv.h"
#include "net.h"


int NetHandleObjectValues(int condescriptor, char *arg)
{
	int object_num, con_object_num;

	int loc_type;        
        int locked_on;
        int intercepting_object;

	int thrust_rev_state;
        float thrust_dir;
	float thrust;
        float throttle;

        int lighting;	/* Real type is xswo_lighting_t. */
        float hp;
        float power;
        float antimatter;
        int shield_state;

	int selected_weapon;
        int cloak_state;
        float cloak_strength;
        float visibility;

        int damage_control;

        xsw_object_struct *obj_ptr;


        con_object_num = connection[condescriptor]->object_num;
        if(DBIsObjectGarbage(con_object_num))
            return(-1);
        else
            obj_ptr = xsw_object[con_object_num];


        /*
         *   SWEXTCMD_STDOBJVALS format:
         *
         *      object_num,
         *      loc_type, locked_on, intercepting_object,
         *      thrust_rev_state, thrust_dir, thrust, throttle,
         *      lighting, hp, power,
         *      antimatter, shield_state, selected_weapon, cloak_state,
         *      cloak_strength, visibility, damage_control
         */
        sscanf(arg,
"%i\
 %i %i %i\
 %i %f %f %f\
 %i %f %f\
 %f %i %i %i\
 %f %f %i",

                &object_num,

                &loc_type,
                &locked_on, 
                &intercepting_object,

                &thrust_rev_state,
                &thrust_dir,
                &thrust,
                &throttle,

                &lighting,
                &hp,
                &power,

                &antimatter,
                &shield_state,
                &selected_weapon,
                &cloak_state,

                &cloak_strength,
                &visibility,
                &damage_control
        ); 


        /* Connection must own object. */
        if(object_num != con_object_num)
            return(-3);


        /* Set throttle. */
        if(throttle < 0.0) 
            throttle = 0.0;
        else if(throttle > 1.0)
            throttle = 1.0;

        obj_ptr->throttle = throttle;


        /* *** Everything else is tossed away. *** */


        return(0);
}


int NetSendObjectValues(int condescriptor, int object_num)
{
	xsw_object_struct *obj_ptr;
	char sndbuf[CS_DATA_MAX_LEN];


        if(DBIsObjectGarbage(object_num))
            return(-1);
        else
            obj_ptr = xsw_object[object_num];


        /*
         *      SWEXTCMD_STDOBJVALS format:
         *
         *      object_num,
         *      loc_type, locked_on, intercepting_object,
         *      thrust_rev_state, thrust_dir, thrust, throttle,
         *      lighting, hp, power,
         *      antimatter, shield_state, selected_weapon, cloak_state,
         *      cloak_strength, visibility, damage_control
         */
        sprintf(sndbuf,
"%i %i %i\
 %i %i %i\
 %i %.4f %.4f %.4f\
 %i %.4f %.4f\
 %.4f %i %i\
 %i %.4f %.4f %i\n",

                CS_CODE_EXT,
                SWEXTCMD_STDOBJVALS,

                object_num,
        
                obj_ptr->loc_type,
                obj_ptr->locked_on,
                obj_ptr->intercepting_object,

                0,		/* thrust_rev_state, no longer used. */
                obj_ptr->thrust_dir,
                obj_ptr->thrust,
                obj_ptr->throttle,

                obj_ptr->lighting,
                obj_ptr->hp,
                obj_ptr->power,

                obj_ptr->antimatter,
                obj_ptr->shield_state,
                obj_ptr->selected_weapon,

                obj_ptr->cloak_state,
                obj_ptr->cloak_strength,
                obj_ptr->visibility,
                obj_ptr->damage_control
        );
        NetDoSend(condescriptor, sndbuf);


        return(0);
}
