#include <fnmatch.h>

#include "../include/unvutil.h"	// Dan S
#include "../include/unvmatch.h"

#include "swserv.h"
#include "net.h"


#define THIS_CMD_NAME	"sysparm"

#define MIN(a,b)        ((a) < (b) ? (a) : (b))
#define MAX(a,b)        ((a) > (b) ? (a) : (b))


/*
 *	Returns true if str matches parm, using fnmatch() style
 *	wildcard matching.
 */
static int CMDSYSPARMPARMMATCH(const char *str, const char *parm)
{
	return(!fnmatch(str, parm, 0));
}


/*
 *	Returns a statically allocated string containing a verbose
 *	explaination of the delta time in seconds.
 */
static char *CMDSYSPARMFMTDTIME(time_t dt)
{
        static char s[256];


	if(dt < 0)
	    dt = 0;

	if(dt < 60)
	    sprintf(s, "%ld s", dt);
	else if(dt < 3600)
	    sprintf(s, "%ld m", dt / 60);
        else if(dt < 86400)
            sprintf(s, "%ld h", dt / 3600);
	else
            sprintf(s, "%ld d", dt / 86400);

	s[256 - 1] = '\0';
	return(s);
}


/*
 *      Set or view system parameters.
 */
int CmdSysparm(int condescriptor, const char *arg)
{
	int i, matches = 0;
	long dt;
        char *strptr, *parm_name;
	int object_num, con_obj_num;
	xsw_object_struct *con_obj_ptr;
	connection_struct *con_ptr;

        char parm[CS_DATA_MAX_LEN];
        char val[CS_DATA_MAX_LEN];

        char sndbuf[CS_DATA_MAX_LEN + UNV_TITLE_MAX + 512];


	/* Get connection's object number (assumed valid). */
        con_ptr = connection[condescriptor];
        con_obj_num = con_ptr->object_num;
        con_obj_ptr = xsw_object[con_obj_num];


	/* Reset send buffer. */
	*sndbuf = '\0';

        /* Check if allowed to view or set system parameters. */
        if(con_obj_ptr->permission.uid > ACCESS_UID_SYSPARM)
        {
            sprintf(sndbuf,
                "%s: Permission denied: Requires access level %i.",
		THIS_CMD_NAME,
                ACCESS_UID_SYSPARM
            );
            NetSendLiveMessage(condescriptor, sndbuf);
            return(-1);
        }


        /* Parse argument. */
        strptr = (char *) strchr(arg, '=');
        if(strptr == NULL)
	{
	    strncpy(parm, arg, CS_DATA_MAX_LEN);
	    parm[CS_DATA_MAX_LEN - 1] = '\0';
	    StringStripSpaces(parm);

	    *val = '\0';

	    if(*parm == '\0')
	    {
		strncpy(parm, "*", CS_DATA_MAX_LEN);
		parm[CS_DATA_MAX_LEN - 1] = '\0';
	    }
	}
	else
	{
            strncpy(val, strptr + 1, CS_DATA_MAX_LEN);
            val[CS_DATA_MAX_LEN - 1] = '\0';
            StringStripSpaces(val);

            *strptr = '\0';
            strncpy(parm, arg, CS_DATA_MAX_LEN);
            parm[CS_DATA_MAX_LEN - 1] = '\0';
            StringStripSpaces(parm);
	}


	/* Check input parameter through each parameter,
	 * set new value if new value is given.
	 */
	parm_name = "allow_guest";
        if(CMDSYSPARMPARMMATCH(parm, parm_name))
        {
	    matches++;
	    if(*val != '\0')
	    {
		sysparm.allow_guest = StringIsYes(val);
	    }
            sprintf(sndbuf,
                "%s = %s",
                parm_name,
		((sysparm.allow_guest) ? "yes" : "no")
            );
            NetSendLiveMessage(condescriptor, sndbuf);
        }

        parm_name = "cease_fire";
        if(CMDSYSPARMPARMMATCH(parm, parm_name))
        {
            matches++;
            if(*val != '\0')
            {
                sysparm.cease_fire = StringIsYes(val);
            }
            sprintf(sndbuf,
                "%s = %s",
                parm_name,
                ((sysparm.cease_fire) ? "yes" : "no")
            );
            NetSendLiveMessage(condescriptor, sndbuf);
        }

        parm_name = "con_notify"; 
        if(CMDSYSPARMPARMMATCH(parm, parm_name))
        {
            matches++;
            if(*val != '\0')
            {
                sysparm.con_notify = StringIsYes(val);
            }
            sprintf(sndbuf,
                "%s = %s",
                parm_name,
                ((sysparm.con_notify) ? "yes" : "no")
            );
            NetSendLiveMessage(condescriptor, sndbuf);
        }

	parm_name = "convert_ru_to_au";
        if(CMDSYSPARMPARMMATCH(parm, parm_name))
        {
            matches++;
	    if(*val != '\0')
	    {
		sw_units.ru_to_au = atof(val);
		if(sw_units.ru_to_au <= 0)
		    sw_units.ru_to_au = 1;

		/* Update units to all connections. */
		NetSendUnits(-1);
	    }
            sprintf(sndbuf,
                "%s = %.4f",
		parm_name,
                sw_units.ru_to_au
            );
            NetSendLiveMessage(condescriptor, sndbuf);
        }

        parm_name = "dmg_ctl_rate";
        if(CMDSYSPARMPARMMATCH(parm, parm_name))
        {   
            matches++;
            if(*val != '\0')
            {
                sysparm.dmg_ctl_rate = atof(val);
                if(sysparm.dmg_ctl_rate < 0)
                    sysparm.dmg_ctl_rate = 0;
            }
            if(sysparm.dmg_ctl_rate > 0)
                sprintf(sndbuf,
                    "%s = %f",
                    parm_name,
                    sysparm.dmg_ctl_rate
                );
            else
                sprintf(sndbuf,
                    "%s = none",
                    parm_name
                );
            NetSendLiveMessage(condescriptor, sndbuf);
        }

        parm_name = "file_isr";
        if(CMDSYSPARMPARMMATCH(parm, parm_name))
        {
            matches++;
            if(*val != '\0')
            {
                strncpy(
                    unv_head.isr,
                    val,
                    PATH_MAX + NAME_MAX
                );
                unv_head.isr[PATH_MAX + NAME_MAX - 1] = '\0';
            }
            sprintf(sndbuf,
                "%s = %s",
                parm_name,
                unv_head.isr
            );
            NetSendLiveMessage(condescriptor, sndbuf);
        }

        parm_name = "file_ocsn"; 
        if(CMDSYSPARMPARMMATCH(parm, parm_name))
        {
            matches++;
            if(*val != '\0')
            {       
                strncpy(
                    unv_head.ocsn, 
                    val,
                    PATH_MAX + NAME_MAX
                );
                unv_head.ocsn[PATH_MAX + NAME_MAX - 1] = '\0';
            }
            sprintf(sndbuf,
                "%s = %s",
                parm_name,
                unv_head.ocsn   
            );
            NetSendLiveMessage(condescriptor, sndbuf);
        }

        parm_name = "file_ss";
        if(CMDSYSPARMPARMMATCH(parm, parm_name))
        {
            matches++;
            if(*val != '\0')
            {
                strncpy(
                    unv_head.ss,
                    val,
                    PATH_MAX + NAME_MAX
                );
                unv_head.ss[PATH_MAX + NAME_MAX - 1] = '\0';
            }
            sprintf(sndbuf,
                "%s = %s",
                parm_name,
                unv_head.ss
            );
            NetSendLiveMessage(condescriptor, sndbuf);
        }

        parm_name = "guest_login_name";
        if(CMDSYSPARMPARMMATCH(parm, parm_name))
        {
            matches++;
            if(*val != '\0')
            {
		strncpy(sysparm.guest_login_name, val, XSW_OBJ_NAME_MAX);
		sysparm.guest_login_name[XSW_OBJ_NAME_MAX - 1] = '\0';
            }
            sprintf(sndbuf, 
                "%s = %s",
                parm_name,
                sysparm.guest_login_name
            );
            NetSendLiveMessage(condescriptor, sndbuf);
        }

        parm_name = "guest_start_direction";
        if(CMDSYSPARMPARMMATCH(parm, parm_name))
        {
            const int len = 256;
            char text[len];

            matches++;
            if(*val != '\0')
            {
                UNVParseDirection(
                    val,
                    &unv_head.guest_start_heading,
                    &unv_head.guest_start_pitch,
                    &unv_head.guest_start_bank
                );
            }
            UNVDirectionFormatString(
		text,
                &unv_head.guest_start_heading,
                &unv_head.guest_start_pitch,
                &unv_head.guest_start_bank,
		len
            );
            sprintf(sndbuf,
                "%s = %s",
                parm_name,
                text
            );
            NetSendLiveMessage(condescriptor, sndbuf);
	}

        parm_name = "guest_start_location";
        if(CMDSYSPARMPARMMATCH(parm, parm_name))
        {
	    const int len = 256;
	    char text[len];

            matches++;
            if(*val != '\0')
            { 
		UNVParseLocation(
		    val,
		    &unv_head.guest_start_sect_x,
                    &unv_head.guest_start_sect_y,
                    &unv_head.guest_start_sect_z,
                    &unv_head.guest_start_x,
                    &unv_head.guest_start_y,
                    &unv_head.guest_start_z
		);
	    }
	    UNVLocationFormatString(
		text,
		&unv_head.guest_start_sect_x,
                &unv_head.guest_start_sect_y,
                &unv_head.guest_start_sect_z,
                &unv_head.guest_start_x,
                &unv_head.guest_start_y,
                &unv_head.guest_start_z,
		len
	    );
            sprintf(sndbuf,
                "%s = %s",
                parm_name,
                text
            );
            NetSendLiveMessage(condescriptor, sndbuf);
        }

        parm_name = "hide_players";
        if(CMDSYSPARMPARMMATCH(parm, parm_name))
        {
            matches++;
            if(*val != '\0')
            {
                sysparm.hide_players = StringIsYes(val);
            }
            sprintf(sndbuf,
                "%s = %s",
                parm_name,
                ((sysparm.hide_players) ? "yes" : "no")
            );
            NetSendLiveMessage(condescriptor, sndbuf);
        }

        parm_name = "hit_player_bonus";
        if(CMDSYSPARMPARMMATCH(parm, parm_name))
        {
            matches++;
            if(*val != '\0')
            {
                sysparm.hit_player_bonus = atof(val);
		if(sysparm.hit_player_bonus < 0)
		    sysparm.hit_player_bonus = 0;
            }

	    if(sysparm.hit_player_bonus == 0)
                sprintf(sndbuf,
                    "%s = %s",
                    parm_name,
                    "none"
                );
	    else
		sprintf(sndbuf,
                    "%s = %f",
                    parm_name,
                    sysparm.hit_player_bonus
                );
            NetSendLiveMessage(condescriptor, sndbuf);
        }

        parm_name = "homes_destroyable";
        if(CMDSYSPARMPARMMATCH(parm, parm_name))
        {
            matches++;
            if(*val != '\0')
            {
                sysparm.homes_destroyable = StringIsYes(val);
            }
            sprintf(sndbuf,
                "%s = %s",
                parm_name,
                ((sysparm.homes_destroyable) ? "yes" : "no")
            );
            NetSendLiveMessage(condescriptor, sndbuf);
        }

        parm_name = "int_aux_con_stats";
        if(CMDSYSPARMPARMMATCH(parm, parm_name))
        {
            matches++;
            if(*val != '\0')
            {
                dt = atol(val);
		if(dt < 1)
		    dt = 1;
                sysparm.int_aux_con_stats = dt;	/* Milliseconds. */
            }
            sprintf(sndbuf,
                "%s = %ld ms",
                parm_name,
		sysparm.int_aux_con_stats
            );
            NetSendLiveMessage(condescriptor, sndbuf);
        }

        parm_name = "int_memory_clean";
        if(CMDSYSPARMPARMMATCH(parm, parm_name))
        {
            matches++;
            if(*val != '\0')
            {
                dt = atol(val);
                if(dt < 1)
                    dt = 1;
                sysparm.int_memory_clean = dt;	/* Milliseconds. */
            }
            sprintf(sndbuf,
                "%s = %ld ms",
                parm_name,
                sysparm.int_memory_clean
            );
            NetSendLiveMessage(condescriptor, sndbuf);
        }

        parm_name = "int_new_connection_poll"; 
        if(CMDSYSPARMPARMMATCH(parm, parm_name))
        {
            matches++;
            if(*val != '\0')
            {
                dt = atol(val);
                if(dt < 1)
                    dt = 1;
                sysparm.int_new_connection_poll = dt;	/* Milliseconds. */
            }
            sprintf(sndbuf,
                "%s = %ld ms",
                parm_name,
                sysparm.int_new_connection_poll
            );
            NetSendLiveMessage(condescriptor, sndbuf);
        }

        parm_name = "int_object_values";
        if(CMDSYSPARMPARMMATCH(parm, parm_name))
        {
            matches++;
            if(*val != '\0')
            {
                dt = atol(val);
                if(dt < 1) 
                    dt = 1;
                sysparm.int_object_values = dt;	/* Milliseconds. */
            }
            sprintf(sndbuf,
                "%s = %ld ms",
                parm_name,
                sysparm.int_object_values
            );
            NetSendLiveMessage(condescriptor, sndbuf);
        }

        parm_name = "int_os_stats";
        if(CMDSYSPARMPARMMATCH(parm, parm_name))
        {
            matches++;
            if(*val != '\0')
            {
                dt = atol(val);
                if(dt < 1)
                    dt = 1;
                sysparm.int_os_stats = dt;	/* Seconds. */
            }
            sprintf(sndbuf,
                "%s = %ld s  (next: %s)",
                parm_name,
                sysparm.int_os_stats,
		CMDSYSPARMFMTDTIME(next.os_stats - cur_systime)
            );
            NetSendLiveMessage(condescriptor, sndbuf);
        }

        parm_name = "int_stats_export";
        if(CMDSYSPARMPARMMATCH(parm, parm_name))
        {
            matches++;
            if(*val != '\0')
            {
                dt = atol(val);
                if(dt < 1) 
                    dt = 1;
                sysparm.int_stats_export = dt;	/* Seconds. */
            }
            sprintf(sndbuf,
                "%s = %ld s  (next: %s)",
                parm_name,
                sysparm.int_stats_export,
		CMDSYSPARMFMTDTIME(next.stats_export - cur_systime)
            );
            NetSendLiveMessage(condescriptor, sndbuf);
        }

        parm_name = "int_system_check";
        if(CMDSYSPARMPARMMATCH(parm, parm_name))
        {
            matches++;
            if(*val != '\0')
            {
                dt = atol(val);
                if(dt < 1)
                    dt = 1;
                sysparm.int_system_check = dt;	/* Milliseconds. */
            }
            sprintf(sndbuf,
                "%s = %ld ms",
                parm_name,
                sysparm.int_system_check
            );
            NetSendLiveMessage(condescriptor, sndbuf);
        }

        parm_name = "int_unv_save";
        if(CMDSYSPARMPARMMATCH(parm, parm_name))
        {
            matches++;
            if(*val != '\0')
            {
                dt = atol(val);
                if(dt < 1)
                    dt = 1;
                sysparm.int_unv_save = dt;	/* Seconds. */
            }
            sprintf(sndbuf,
                "%s = %ld s  (next: %s)",
                parm_name,
                sysparm.int_unv_save,
		CMDSYSPARMFMTDTIME(next.unv_save - cur_systime)
            );
            NetSendLiveMessage(condescriptor, sndbuf);
        }

        parm_name = "int_weapon_values";
        if(CMDSYSPARMPARMMATCH(parm, parm_name))
        {
            matches++;
            if(*val != '\0')
            {
                dt = atol(val);
                if(dt < 1)
                    dt = 1;
                sysparm.int_weapon_values = dt;	/* Milliseconds. */
            }
            sprintf(sndbuf,
                "%s = %ld ms",
                parm_name,
                sysparm.int_weapon_values
            );
            NetSendLiveMessage(condescriptor, sndbuf);
        }

        parm_name = "killer_gets_credits";
        if(CMDSYSPARMPARMMATCH(parm, parm_name))
        {
            matches++;
            if(*val != '\0')
            {
                sysparm.killer_gets_credits = StringIsYes(val);
            }
            sprintf(sndbuf,
                "%s = %s",
                parm_name,
                ((sysparm.killer_gets_credits) ? "yes" : "no")
            );
            NetSendLiveMessage(condescriptor, sndbuf);
        }

        parm_name = "log_errors"; 
        if(CMDSYSPARMPARMMATCH(parm, parm_name))
        {
            matches++;
            if(*val != '\0')
            {
                sysparm.log_errors = StringIsYes(val);
            }
            sprintf(sndbuf, 
                "%s = %s",
                parm_name,
                ((sysparm.log_errors) ? "yes" : "no")
            );
            NetSendLiveMessage(condescriptor, sndbuf);
        }

        parm_name = "log_events";
        if(CMDSYSPARMPARMMATCH(parm, parm_name))
        {
            matches++;
            if(*val != '\0')
            {
                sysparm.log_events = StringIsYes(val);
            }
            sprintf(sndbuf,
                "%s = %s",
                parm_name,
                ((sysparm.log_events) ? "yes" : "no")
            );
            NetSendLiveMessage(condescriptor, sndbuf);
        }

        parm_name = "log_general";
        if(CMDSYSPARMPARMMATCH(parm, parm_name))
        {
            matches++;
            if(*val != '\0')
            {
                sysparm.log_general = StringIsYes(val);
            }
            sprintf(sndbuf, 
                "%s = %s",
                parm_name,
                ((sysparm.log_general) ? "yes" : "no")
            );
            NetSendLiveMessage(condescriptor, sndbuf);
        }

        parm_name = "log_net";
        if(CMDSYSPARMPARMMATCH(parm, parm_name))
        {
            matches++;
            if(*val != '\0')
            {
                sysparm.log_net = StringIsYes(val);
            }
            sprintf(sndbuf, 
                "%s = %s",
                parm_name,
                ((sysparm.log_net) ? "yes" : "no")
            );
            NetSendLiveMessage(condescriptor, sndbuf);
        }

        parm_name = "login_timeout";
        if(CMDSYSPARMPARMMATCH(parm, parm_name))
        {
            matches++;
            if(*val != '\0')
            {
                dt = atol(val);
                if(dt < 1)
                    dt = 1;
                sysparm.login_timeout = dt;	/* Seconds. */
            }
            sprintf(sndbuf,
                "%s = %ld s",
                parm_name,
                sysparm.login_timeout
            );
            NetSendLiveMessage(condescriptor, sndbuf);
        }

        parm_name = "lost_found_owner";
        if(CMDSYSPARMPARMMATCH(parm, parm_name))
        {
            matches++;
	    if(*val != '\0')
	    {
                object_num = MatchObjectByName(
		    xsw_object, total_objects,
		    val, XSW_OBJ_TYPE_PLAYER
		);
		if(DBIsObjectGarbage(object_num))
		{
                    sprintf(sndbuf,
                        "%s: No such player object.",
                        val
                    );
		}
                else
		{
                    unv_head.lost_found_owner = object_num;
                    sprintf(sndbuf,
                        "%s = %s",
			parm_name,
                        DBGetFormalNameStr(unv_head.lost_found_owner)
		    );
		}
	    }
	    else
	    {
                sprintf(sndbuf,
                    "%s = %s",
                    parm_name,
                    DBGetFormalNameStr(unv_head.lost_found_owner)
                );
	    }
            NetSendLiveMessage(condescriptor, sndbuf);
        }

        parm_name = "max_aux_connections";
        if(CMDSYSPARMPARMMATCH(parm, parm_name))
        {
            matches++;
            if(*val != '\0')
            {
		i = atoi(val);
		if(i < 0)
		    i = 0;
                sysparm.max_aux_connections = i;
            }
            sprintf(sndbuf,
                "%s = %i",
                parm_name,
                sysparm.max_aux_connections
            );
            NetSendLiveMessage(condescriptor, sndbuf);
        }

        parm_name = "max_connections";
        if(CMDSYSPARMPARMMATCH(parm, parm_name))
        {
            matches++;
            if(*val != '\0')
            {
                i = atoi(val);
                if(i < 1)
                    i = 1;
                sysparm.max_connections = i;
            }
            sprintf(sndbuf,
                "%s = %i",
                parm_name,
                sysparm.max_connections
            );
            NetSendLiveMessage(condescriptor, sndbuf);
        }

        parm_name = "max_failed_logins";
        if(CMDSYSPARMPARMMATCH(parm, parm_name))
        {
            matches++;
            if(*val != '\0')
            {
                i = atoi(val);
                if(i < 1)
                    i = 1;
                sysparm.max_failed_logins = i;
            }
            sprintf(sndbuf,
                "%s = %i",
                parm_name,
                sysparm.max_failed_logins
            );
            NetSendLiveMessage(condescriptor, sndbuf);
        }

        parm_name = "max_guests";
        if(CMDSYSPARMPARMMATCH(parm, parm_name))
        {
            matches++;
            if(*val != '\0')
            {
                i = atoi(val);
                if(i < 0)
                    i = 0;
                sysparm.max_guests = i;
            }
            sprintf(sndbuf,
                "%s = %i",
                parm_name,
                sysparm.max_guests
            );
            NetSendLiveMessage(condescriptor, sndbuf);
        }

        parm_name = "mesg_leave";
        if(CMDSYSPARMPARMMATCH(parm, parm_name))
        {
            matches++;
            if(*val != '\0')
            {
		strncpy(sysparm.mesg_leave, val, CS_MESG_MAX);
		sysparm.mesg_leave[CS_MESG_MAX - 1] = '\0';
            }
            sprintf(sndbuf,
                "%s = %s",
                parm_name,
                sysparm.mesg_leave
            );
            NetSendLiveMessage(condescriptor, sndbuf);
        }

        parm_name = "mesg_no_guests";
        if(CMDSYSPARMPARMMATCH(parm, parm_name))
        {
            matches++;
            if(*val != '\0')
            {
                strncpy(sysparm.mesg_no_guests, val, CS_MESG_MAX);
                sysparm.mesg_no_guests[CS_MESG_MAX - 1] = '\0';
            }
            sprintf(sndbuf,
                "%s = %s",
                parm_name,
                sysparm.mesg_no_guests
            );
            NetSendLiveMessage(condescriptor, sndbuf);
        }

        parm_name = "mesg_welcome";
        if(CMDSYSPARMPARMMATCH(parm, parm_name))
        {
            matches++;
            if(*val != '\0')
            {
                strncpy(sysparm.mesg_welcome, val, CS_MESG_MAX);
                sysparm.mesg_welcome[CS_MESG_MAX - 1] = '\0';
            }
            sprintf(sndbuf,
                "%s = %s",
                parm_name,
                sysparm.mesg_welcome
            );
            NetSendLiveMessage(condescriptor, sndbuf);
        }

        parm_name = "mesg_wrong_login";
        if(CMDSYSPARMPARMMATCH(parm, parm_name))
        {
            matches++;
            if(*val != '\0')
            {
                strncpy(sysparm.mesg_wrong_login, val, CS_MESG_MAX);
                sysparm.mesg_wrong_login[CS_MESG_MAX - 1] = '\0';
            }
            sprintf(sndbuf,
                "%s = %s",
                parm_name,
                sysparm.mesg_wrong_login
            );
            NetSendLiveMessage(condescriptor, sndbuf);
        }

        parm_name = "player_start_direction";  
        if(CMDSYSPARMPARMMATCH(parm, parm_name))
        {
            const int len = 256;
            char text[len];
         
            matches++;
            if(*val != '\0')
            {
                UNVParseDirection(
                    val,
                    &unv_head.player_start_heading,
                    &unv_head.player_start_pitch,
                    &unv_head.player_start_bank
                );
            }
            UNVDirectionFormatString(
                text,
                &unv_head.player_start_heading,
                &unv_head.player_start_pitch,
                &unv_head.player_start_bank,
                len
            );
            sprintf(sndbuf,
                "%s = %s",
                parm_name,
                text
            );
            NetSendLiveMessage(condescriptor, sndbuf);
        }

        parm_name = "player_start_location";
        if(CMDSYSPARMPARMMATCH(parm, parm_name))
        {
            const int len = 256;
            char text[len];
                
            matches++;
            if(*val != '\0')
            {
                UNVParseLocation(
                    val,
                    &unv_head.player_start_sect_x,
                    &unv_head.player_start_sect_y,
                    &unv_head.player_start_sect_z,
                    &unv_head.player_start_x,
                    &unv_head.player_start_y,
                    &unv_head.player_start_z
                );
            }
            UNVLocationFormatString(
                text,
                &unv_head.player_start_sect_x,
                &unv_head.player_start_sect_y,
                &unv_head.player_start_sect_z,
                &unv_head.player_start_x,
                &unv_head.player_start_y,
                &unv_head.player_start_z,
                len
            );
            sprintf(sndbuf,
                "%s = %s",
                parm_name,
                text
            );
            NetSendLiveMessage(condescriptor, sndbuf);
        }

        parm_name = "report_destroyed_weapons";
        if(CMDSYSPARMPARMMATCH(parm, parm_name))
        {
            matches++;
            if(*val != '\0')
            {
                sysparm.report_destroyed_weapons = StringIsYes(val);
            }
            sprintf(sndbuf,
                "%s = %s",
                parm_name,
                ((sysparm.report_destroyed_weapons) ? "yes" : "no")
            );
            NetSendLiveMessage(condescriptor, sndbuf);
        }

	parm_name = "send_starchart";
	if(CMDSYSPARMPARMMATCH(parm, parm_name))
        {
            matches++;
            if(*val != '\0')
            {
                sysparm.send_starchart = StringIsYes(val);
            }
            sprintf(sndbuf,
                "%s = %s",
                parm_name,
                ((sysparm.send_starchart) ? "yes" : "no")
            );
            NetSendLiveMessage(condescriptor, sndbuf);
        }

        parm_name = "unv_title";
        if(CMDSYSPARMPARMMATCH(parm, parm_name))
        {
	    matches++;
            if(*val != '\0')
            {
                strncpy(unv_head.title, val, UNV_TITLE_MAX);
                unv_head.title[UNV_TITLE_MAX - 1] = '\0';
            }
            sprintf(sndbuf,
                "%s = %s",
                parm_name,
                unv_head.title
            );
	    NetSendLiveMessage(condescriptor, sndbuf);
        }


	/* No matches? */
	if(matches == 0)
        {
            sprintf(sndbuf,
                "%s: Unsupported sysparm parameter.",
                parm
            );
	    NetSendLiveMessage(condescriptor, sndbuf);
        }


        return(0);
}
