/*
                          Network Data Sending

 	Functions:

 	int NetSendData(char *buf)
 	int NetSendGlobalMessage(char *message)
 	int NetSendDisconnect()
	int NetSendRefresh()
 	int NetSendSetInterval()
 	int NetSendExec(char *arg)

        int NetSendSetImageSet(char *arg)
        int NetSendSetSoundSet(char *arg)
        int NetSendSetOCSN(char *arg)

	int NetSendObjectSect(int object_num)
 	int NetSendPoseObj(int object_num)
 	int NetSendObjectThrottle(int object_num)
 	int NetSendObjectValues(int object_num)
	int NetSendSetLighting(int object_num, xswo_lighting_t lighting)
	int NetSendSetChannel(int object_num, int channel)
	int NetSendWeaponsState(int object_num, int state)
 	int NetSendSelectWeapon(int object_num, int selected_weapon)
 	int NetSendSetShields(
		int object_num, int shield_state,
		double shield_frequency
	)
 	int NetSendSetDmgCtl(int object_num, char damage_control)
 	int NetSendSetCloak(int object_num, char cloak_state)
 	int NetSendWeaponsLock(int object_num, int tar_object_num)
 	int NetSendWeaponsUnlock(int object_num)
	int NetSendReqName(int object_num)
 	int NetSendIntercept(int object_num, const char *arg)
	int NetSendSetEngine(int object_num, int engine_state)
 	int NetSendFireWeapon(int object_num, double freq)
	int NetSendTractorBeamLock(int src_obj, int tar_obj)
	int NetSendHail(int src_obj, int tar_obj, int channel)
	int NetSendComMessage(
		int src_obj, int tar_obj,
		int channel, char *message
	)
	int NetSendWormHoleEnter(int src_obj, int tar_obj)
	int NetSendELinkEnter(int src_obj, int tar_obj)
	int NetSendWeaponDisarm(int src_obj, int tar_obj)

	int NetSendEcoReqValues(int src_obj, int tar_obj)
	int NetSendEcoBuy(
		int customer_obj,
		int proprietor_obj,
		xsw_ecoproduct_struct product
	)
        int NetSendEcoSell(
                int customer_obj,
                int proprietor_obj,
                xsw_ecoproduct_struct product
        )

	---


 */

#include <sys/types.h>
#include <sys/socket.h>

#include "../include/string.h"

#include "../include/netio.h"
#include "../include/swnetcodes.h"
#include "../include/unvmath.h"

#include "xsw.h"
#include "net.h"


#define MIN(a,b)        (((a) < (b)) ? (a) : (b))
#define MAX(a,b)        (((a) > (b)) ? (a) : (b))


int NetSendData(char *buf)
{
	int bytes_sent, len;


	if(buf == NULL)
	    return(-1);


	/* Check if socket is sendable. */
	if(NetIsSocketWritable(net_parms.socket, NULL))
	{
	    /* Socket is writeable. */

	    /* Reduce bad send count. */
	    if(net_parms.bad_send_count > 0)
		net_parms.bad_send_count--;

	    /* Get min length of buf. */
	    len = MIN(strlen(buf), CS_DATA_MAX_LEN);

	    /* Send buffer. */
            bytes_sent = send(net_parms.socket, buf, len, 0);

	    /* Record bytes sent on load stats. */
	    if(bytes_sent > 0)
                loadstat.sx_interval += bytes_sent;
	}
	else
	{
	    /* Socket is not writeable. */

	    /* Increment bad send count. */
	    net_parms.bad_send_count++;

	    /* Check if too many bad sends. */
            if(net_parms.bad_send_count > 14)
            {
                printdw(&err_dw,
"Too many socket write errors!\n\n\
There have been too many failed write attempts through\n\
the socket connecting to the server.  It would appear\n\
that the socket may have been disconnected or lost.\n\
Please try reconnecting.\n"
                );
        
                /*   Force close connection, reset net parms then do
		 *   disconnect procedure.
		 */
                NetResetParms();
		XSWDoDisconnect();
	    }


	    /* Return indicating error. */
	    return(-3);
	}


	return(0);
}



int NetSendGlobalMessage(char *message)
{
	int len;
	char local_mesg[CS_MESG_MAX];
	char sndbuf[CS_DATA_MAX_LEN];


	/* Make sure message is valid. */
	if(message == NULL)
	    return(-1);
	len = strlen(message);
	if(len < 1)
	    return(0);


	/* Copy message to local_mesg. */
	strncpy(local_mesg, message, CS_MESG_MAX);
	local_mesg[CS_MESG_MAX - 1] = '\0';
	StringStripSpaces(local_mesg);


	/*
	 *   CS_CODE_LIVEMESSAGE format:
	 *
	 *	message
	 */
	sprintf(sndbuf, "%i %s\n",
		CS_CODE_LIVEMESSAGE,
		local_mesg
	);

	NetSendData(sndbuf);

	return(0);
}

int NetSendDisconnect()
{
	char sndbuf[CS_DATA_MAX_LEN];


	/* Send disconnect only if socket is opened. */
	if(net_parms.socket > -1)
	{
	    sprintf(sndbuf, "%i\n",
		CS_CODE_LOGOUT
	    );
	    NetSendData(sndbuf);
	}


	return(0);
}


/*
 *	Sends a request for refresh.
 */
int NetSendRefresh()
{
	char sndbuf[CS_DATA_MAX_LEN];

         
        /* Send refresh request only if socket is opened. */
        if(net_parms.socket > -1)
        {
            sprintf(sndbuf, "%i\n",
                CS_CODE_REFRESH
            );
            NetSendData(sndbuf);
        }


        return(0);
}


/*
 *	Sends a set network interval.
 */
int NetSendSetInterval()
{
	char sndbuf[CS_DATA_MAX_LEN];


        /*
         *   NET_CMD_SETINTERVAL format is as follows:
         *
         *      interval
         */
	sprintf(sndbuf, "%i %ld\n",
		CS_CODE_INTERVAL,
		net_parms.net_int
	);
        NetSendData(sndbuf);


	return(0);
}


/*
 *	Sends a literal command.
 */
int NetSendExec(char *arg)
{
	char larg[CS_MESG_MAX];
        char sndbuf[CS_DATA_MAX_LEN];


	if(arg == NULL)
	    return(-1);
	if(arg[0] == '\0')
	    return(-2);


	strncpy(larg, arg, CS_MESG_MAX);
	larg[CS_MESG_MAX - 1] = '\0';


        /* 
         *   NET_CMD_EXEC format is as follows:
         *
         *      argument
         */
        sprintf(sndbuf, "%i %s\n",
                CS_CODE_LITERALCMD,
                larg
        );
        NetSendData(sndbuf);


	return(0);
}

/*
 *	Sends a request for the current image set file name.
 */
int NetSendSetImageSet(char *arg)
{
        char sndbuf[CS_DATA_MAX_LEN];


        /*
         *   Format CS_CODE_IMAGESET:
         *
         *      path
         */
        sprintf(sndbuf, "%i\n",
		CS_CODE_IMAGESET
        );
        NetSendData(sndbuf);


        return(0);
} 


/*
 *      Sends a request for the current sound set file name.
 */
int NetSendSetSoundSet(char *arg)
{         
        char sndbuf[CS_DATA_MAX_LEN];

          
        /*
         *   Format CS_CODE_SOUNDSET:
         *
         *   *Client only sends code with no arguments*
         */
        sprintf(sndbuf, "%i\n", 
                CS_CODE_SOUNDSET
        );
        NetSendData(sndbuf);


        return(0); 
}


/*
 *      Sends a request for the current object create script
 *	names file name.
 */
int NetSendSetOCSN(char *arg)
{
        char sndbuf[CS_DATA_MAX_LEN];


        /*
         *   Format SWEXTCMD_SETOCSN:
         *
         *   *Client only sends code with no arguments*
         */
        sprintf(sndbuf, "%i %i\n",
		CS_CODE_EXT,
                SWEXTCMD_SETOCSN
        );
        NetSendData(sndbuf);


        return(0);
}


/*
 *	Sends a set object sector.
 */
int NetSendObjectSect(int object_num)
{
        char sndbuf[CS_DATA_MAX_LEN];
        xsw_object_struct *obj_ptr;


        if(DBIsObjectGarbage(object_num))
            return(-1);
        else
            obj_ptr = xsw_object[object_num];

        /*
         *   SWEXTCMD_SETOBJSECT format:
         *
         *      object_num
         *      sect_x sect_y sect_z
         */
        sprintf(sndbuf,
"%i %i\
 %i %ld %ld %ld\n",
                CS_CODE_EXT,
		SWEXTCMD_SETOBJSECT,

                object_num,
                obj_ptr->sect_x,
                obj_ptr->sect_y,
                obj_ptr->sect_z
        );
        NetSendData(sndbuf);


	return(0);
}



int NetSendPoseObj(int object_num)
{
	char sndbuf[CS_DATA_MAX_LEN];
	xsw_object_struct *obj_ptr;


	if(DBIsObjectGarbage(object_num))
	    return(-1);
 	else
	    obj_ptr = xsw_object[object_num];

        /*
	 *   CS_CODE_POSEOBJ format:
	 *
	 *	object_num,
	 *	type, isref_num, size,
	 *	x, y, z,
	 *	heading, pitch, bank,
	 *	velocity,
	 *	velocity_heading, velocity_pitch, velocity_bank,
	 *	current_frame
	 */
        sprintf(sndbuf,
"%i %i\
 %i %i %ld\
 %.4f %.4f %.4f\
 %.4f %.4f %.4f\
 %.4f\
 %.4f %.4f %.4f\
 %i\n",
                CS_CODE_POSEOBJ,
                object_num,

                obj_ptr->type,
                obj_ptr->imageset,
		obj_ptr->size,

                obj_ptr->x,
                obj_ptr->y,
                obj_ptr->z,

                obj_ptr->heading,
                obj_ptr->pitch,
		obj_ptr->bank,

                obj_ptr->velocity,

                obj_ptr->velocity_heading,
                0.0000,
                0.0000,

		obj_ptr->animation.current_frame
        );
        NetSendData(sndbuf);
                
                
        return(0);
}



int NetSendObjectThrottle(int object_num)
{
	char sndbuf[CS_DATA_MAX_LEN];
	xsw_object_struct *obj_ptr;


	if(DBIsObjectGarbage(object_num))
	    return(0);
	else
	    obj_ptr = xsw_object[object_num];


	/*
	 *	SWEXTCMD_SETTHROTTLE format:
	 *
	 *	object_num, throttle
	 */
	sprintf(sndbuf,
	    "%i %i %i %.4f\n",

	    CS_CODE_EXT,
	    SWEXTCMD_SETTHROTTLE,

	    object_num,
	    (double)((obj_ptr->engine_state == ENGINE_STATE_ON) ?
                obj_ptr->throttle : 0)
	);
        NetSendData(sndbuf);


	return(0);
}



int NetSendObjectValues(int object_num)
{
	char sndbuf[CS_DATA_MAX_LEN];
	xsw_object_struct *obj_ptr;


        if(DBIsObjectGarbage(object_num))
            return(0);
        else
            obj_ptr = xsw_object[object_num];


	/*   This function was originally used to send the throttle
         *   position of the object to the server.  We now have
	 *   NetSendObjectThrottle() to do that with less bandwidth.
	 */

sndbuf[0] = '\0';


/*
        NetSendData(sndbuf);
*/

        return(0);
}


/*
 *	Send set lighting.
 */
int NetSendSetLighting(int object_num, xswo_lighting_t lighting)
{
        char sndbuf[CS_DATA_MAX_LEN];
 

        /*
         *	SWEXTCMD_SETLIGHTING format:
         *
         *      object_num lighting
         */  
        sprintf(sndbuf, "%i %i %i %i\n",
                CS_CODE_EXT,
                SWEXTCMD_SETLIGHTING,

		object_num,
		lighting
	);
        NetSendData(sndbuf);


	/* Update locally. */
        if(!DBIsObjectGarbage(object_num) &&
           option.local_updates
        )
        {
            xsw_object[object_num]->lighting = lighting;
        }


	return(0);
}


/*
 *	Sends set channel.
 */
int NetSendSetChannel(int object_num, int channel)
{
        char sndbuf[CS_DATA_MAX_LEN];


	/* Sanitize channel. */
	if(((double)channel / 100) > SWR_FREQ_MAX)
	    channel = static_cast<int>(SWR_FREQ_MAX * 100);
	if(((double)channel / 100) < SWR_FREQ_MIN)
	    channel = static_cast<int>(SWR_FREQ_MIN * 100);


        /*
         *      SWEXTCMD_SETCHANNEL format:
         *
         *      object_num channel
         */
        sprintf(sndbuf,
"%i %i\
 %i %i\n",
                CS_CODE_EXT,
                SWEXTCMD_SETCHANNEL,
  
                object_num,
		channel
        );
        NetSendData(sndbuf);


        /* Update locally as needed. */
        if(!DBIsObjectGarbage(object_num) &&
           option.local_updates
        )
        {
            xsw_object[object_num]->com_channel = channel;
        }


        return(0);
}


/*
 *	Send weapon state (weapons online or offline).
 */
int NetSendWeaponsState(int object_num, int state)
{
	/* No network protocol available for this. */

	return(0);
}

/*
 *	Send select weapon.
 */
int NetSendSelectWeapon(int object_num, int selected_weapon)
{
        char sndbuf[CS_DATA_MAX_LEN];


	/* Selected weapon number valid? */
	if((selected_weapon < 0) ||
	   (selected_weapon >= MAX_WEAPONS)
	)
	    return(-1);

        /*
         *   SWEXTCMD_SETWEAPON format:
         *
         *	object_num, selected_weapon
         */
        sprintf(sndbuf,
"%i %i\
 %i %i\n",
		CS_CODE_EXT,
                SWEXTCMD_SETWEAPON,

		object_num,
		selected_weapon
        );
        NetSendData(sndbuf);

	return(0);
}

/*
 *	Sends shield state and frequency of object_num.
 */
int NetSendSetShields(
	int object_num,
	int shield_state,
	double shield_frequency
)
{
	char sndbuf[CS_DATA_MAX_LEN];


	/* Sanitize shield frequency. */
	if(shield_frequency > SWR_FREQ_MAX)
	    shield_frequency = SWR_FREQ_MAX;
	if(shield_frequency < SWR_FREQ_MIN)
	    shield_frequency = SWR_FREQ_MIN;


        /*
         *	SWEXTCMD_SETSHIELDS format:
         *
	 *	object_num, shield_state, shield_frequency
         */
        sprintf(sndbuf,
"%i %i\
 %i %i %.4f\n",

		CS_CODE_EXT,
                SWEXTCMD_SETSHIELDS,

		object_num,
		shield_state,
		shield_frequency
        );
        NetSendData(sndbuf);


        /* Update locally. */
	if(!DBIsObjectGarbage(object_num) &&
           option.local_updates
	)
	{
/*
	    xsw_object[object_num]->shield_state = shield_state;
*/
	    xsw_object[object_num]->shield_frequency = shield_frequency;
	}


	return(0);
}



/*
 *	Send damage control state of object_num.
 */
int NetSendSetDmgCtl(int object_num, char damage_control)
{
        char sndbuf[CS_DATA_MAX_LEN];


        /*
         *	SWEXTCMD_SETDMGCTL format:
         *
	 *	object_num, damage_control
         */
        sprintf(sndbuf,
"%i %i\
 %i %i\n",
		CS_CODE_EXT,
                SWEXTCMD_SETDMGCTL,

		object_num,
		damage_control
        );
        NetSendData(sndbuf);


	/* Update locally. */
        if(!DBIsObjectGarbage(object_num) &&
           option.local_updates
	)
        {
            xsw_object[object_num]->damage_control = damage_control;
        }


        return(0);
}


/*
 *	Send cloak state of object_num.
 */
int NetSendSetCloak(int object_num, char cloak_state)
{
	char sndbuf[CS_DATA_MAX_LEN];


        /*
         *	SWEXTCMD_SETCLOAK format:
         *
         *	object_num, cloak_state
         */
        sprintf(sndbuf,
"%i %i\
 %i %i\n",
		CS_CODE_EXT,
                SWEXTCMD_SETCLOAK,

		object_num,
		cloak_state
        );
        NetSendData(sndbuf);


        /* Update locally. */
        if(!DBIsObjectGarbage(object_num) &&
           option.local_updates
        )
        {
	    if(xsw_object[object_num]->cloak_state != CLOAK_STATE_NONE)
                xsw_object[object_num]->cloak_state = cloak_state;
        }


        return(0);
}



/*
 *	Send weapons lock for object_num.
 */
int NetSendWeaponsLock(int object_num, int tar_object_num)
{
	char sndbuf[CS_DATA_MAX_LEN];

        /*
         *	SWEXTCMD_SETWEPLOCK format:
         *
	 *	object_num, tar_object_num
	 *
	 *	tar_object_num can be -1 for unlock or -2 for lock next
         */
	sprintf(sndbuf,
"%i %i\
 %i %i\n",

		CS_CODE_EXT,
		SWEXTCMD_SETWEPLOCK,

		object_num,
		tar_object_num
	);
        NetSendData(sndbuf);

        return(0);
}



/*
 *	Send unlock weapons for object_num.
 */
int NetSendWeaponsUnlock(int object_num)
{
	NetSendWeaponsLock(object_num, -1);

        return(0);
}



int NetSendReqName(int object_num)
{
        char sndbuf[CS_DATA_MAX_LEN];


        /*
         *   SWEXTCMD_REQNAME format:
         *
	 *	object_num
         */
        sprintf(sndbuf,
"%i %i\
 %i\n",

                CS_CODE_EXT,
                SWEXTCMD_REQNAME,

		object_num
        );
        NetSendData(sndbuf);

        
        return(0);
}



/*
 *	Sents a intercept object or unintercept object.
 */
int NetSendIntercept(int object_num, const char *arg)
{
	char sndbuf[CS_DATA_MAX_LEN];
	char larg[XSW_OBJ_NAME_MAX];


	/* If arg is NULL, set it to "#off". */
	if(arg == NULL)
	    arg = "#off";

	strncpy(larg, arg, XSW_OBJ_NAME_MAX);
	larg[XSW_OBJ_NAME_MAX - 1] = '\0';
	StringStripSpaces(larg);

        /*
         *	SWEXTCMD_SETINTERCEPT format:
         *
	 *	object_num, arg
         */
        sprintf(sndbuf,
"%i %i\
 %i %s\n",
		CS_CODE_EXT,
                SWEXTCMD_SETINTERCEPT,

		object_num,
		larg
        );
        NetSendData(sndbuf);
 

	/* Update locally as needed. */
	if(option.local_updates)
	{
	    if(object_num == net_parms.player_obj_num)
	    {
		if(!strcasecmp(larg, "#off"))
		{
		    if(!DBIsObjectGarbage(object_num))
			xsw_object[object_num]->intercepting_object = -1;
		}

		BridgeWinDrawPanel(
		    object_num,
		    BPANEL_DETAIL_PINAME
		);
	    }
	}


        return(0);
}


/*
 *	Sends a set engine state.
 */
int NetSendSetEngine(int object_num, int engine_state)
{
        char sndbuf[CS_DATA_MAX_LEN];
	xsw_object_struct *obj_ptr;


        /*
         *   SWEXTCMD_SETENGINE format:
         *
         *      object_num engine_state
         */
        sprintf(sndbuf,
"%i %i\
 %i %i\n",
                CS_CODE_EXT,
                SWEXTCMD_SETENGINE,

                object_num,
                engine_state
        );
        NetSendData(sndbuf);


        /* Update locally as needed. */
        if(option.local_updates)
        {
            if(!DBIsObjectGarbage(object_num))
	    {
		obj_ptr = xsw_object[object_num];
		obj_ptr->engine_state = engine_state;

                if(object_num == net_parms.player_obj_num)
                {
                    BridgeWinDrawPanel(
                        object_num,
                        BPANEL_DETAIL_PENGINESTATE
                    );
		}
            }
        }

        return(0);
}

/*
 *	Sends a fire weapon.
 */
int NetSendFireWeapon(int object_num, double freq)
{
	double heading, pitch, bank;
        xsw_object_struct *obj_ptr;
	char sndbuf[CS_DATA_MAX_LEN];


	if(DBIsObjectGarbage(object_num))
	    return(-1);
	else
	    obj_ptr = xsw_object[object_num];

	/* Calculate firing direction. */
	heading = SANITIZERADIANS(
	    obj_ptr->heading +
	    local_control.weapon_fire_heading
	);
	pitch = 0;
	bank = 0;

	/* Sanitize frequency. */
	if(freq > SWR_FREQ_MAX)
	    freq = SWR_FREQ_MAX;
	else if(freq < SWR_FREQ_MIN)
	    freq = SWR_FREQ_MIN;

        /*
         *   SWEXTCMD_FIREWEAPON format:
	 *
         *	object_num
         *      sect_x, sect_y, sect_z,
         *      x, y, z,
         *      heading, pitch, bank,
         *      velocity,
         *      velocity_heading, velocity_pitch, velocity_bank,
         *      freq, yield
         */
        sprintf(sndbuf,
"%i %i\
 %i\
 %ld %ld %ld\
 %.4f %.4f %.4f\
 %.4f %.4f %.4f\
 %.4f\
 %.4f %.4f %.4f\
 %.2f %.4f\n",
		CS_CODE_EXT, SWEXTCMD_FIREWEAPON,
		object_num,
		obj_ptr->sect_x, obj_ptr->sect_y, obj_ptr->sect_z,
		obj_ptr->x, obj_ptr->y, obj_ptr->z,
		heading, pitch, bank,	/* Firing direction. */
		obj_ptr->velocity,
		obj_ptr->velocity_heading, 0.00, 0.00,
		freq, local_control.weapon_yield
        );
        NetSendData(sndbuf);

	return(0);
}

/*
 *	Sends tractor beam lock.
 */
int NetSendTractorBeamLock(int src_obj, int tar_obj)
{
        char sndbuf[CS_DATA_MAX_LEN];


        /*
         *   SWEXTCMD_TRACTORBEAMLOCK format:
         *
	 *	src_obj tar_obj
         */
        sprintf(sndbuf,
"%i %i\
 %i %i\n",

                CS_CODE_EXT,
                SWEXTCMD_TRACTORBEAMLOCK,

		src_obj,
		tar_obj
        );
        NetSendData(sndbuf);

	return(0);
}

/*
 *	Sends a hail.
 */
int NetSendHail(int src_obj, int tar_obj, int channel)
{
	char sndbuf[CS_DATA_MAX_LEN];

        /*
         *   SWEXTCMD_HAIL format:
         *
         *      src_obj, tar_obj, bearing, channel
         */
        sprintf(sndbuf,
"%i %i\
 %i %i %.4f %i\n",

                CS_CODE_EXT,
                SWEXTCMD_HAIL,

		src_obj,
		tar_obj,
		0.00,
		channel
	);
        NetSendData(sndbuf);

        return(0);
}

/*
 *	Sends a message on the specified channel.
 */
int NetSendComMessage(
	int src_obj, int tar_obj,
	int channel, char *message
)
{
        char sndbuf[CS_DATA_MAX_LEN];
        char lmesg[CS_MESG_MAX];


        if(message == NULL)
	    return(-1);

        strncpy(lmesg, message, CS_MESG_MAX);
        lmesg[CS_MESG_MAX - 1] = '\0';

	/* Nothing to send? */
	if(*lmesg == '\0')
	    return(0);

        /*
         *   SWEXTCMD_COMMESSAGE format:
         *
         *      src_obj, tar_obj, bearing, channel;message
         */
        sprintf(sndbuf,
"%i %i\
 %i %i %.4f %i;%s\n",
                CS_CODE_EXT,
                SWEXTCMD_COMMESSAGE,

                src_obj,
                tar_obj,	/* Can be -1. */
                0.00,		/* N/a. */
                channel,
                lmesg
        );
        NetSendData(sndbuf);

        return(0);
}

/*
 *	Sends a worm hole enter.
 */
int NetSendWormHoleEnter(int src_obj, int tar_obj)
{
        char sndbuf[CS_DATA_MAX_LEN];

        /*
         *   SWEXTCMD_WORMHOLEENTER format:
         *
         *      src_obj, tar_obj
         */
        sprintf(sndbuf,
"%i %i\
 %i %i\n",
                CS_CODE_EXT,
                SWEXTCMD_WORMHOLEENTER,

                src_obj,
                tar_obj
        );
        NetSendData(sndbuf);

        return(0);
}

/*
 *	Sends an elink enter.
 */
int NetSendELinkEnter(int src_obj, int tar_obj)
{
	char sndbuf[CS_DATA_MAX_LEN];

        /*
         *   SWEXTCMD_ELINKENTER format:
         *
         *      src_obj, tar_obj
         */
        sprintf(sndbuf,
"%i %i\
 %i %i\n",
                CS_CODE_EXT,
                SWEXTCMD_ELINKENTER,

                src_obj,
                tar_obj
        );
        NetSendData(sndbuf);

        return(0);
}

/*
 *	Sends a disarm weapon.
 */
int NetSendWeaponDisarm(int src_obj, int tar_obj)
{
	char sndbuf[CS_DATA_MAX_LEN];


        /*
         *   SWEXTCMD_WEPDISARM format:
	 *
	 *	src_obj tar_obj
	 */
        sprintf(sndbuf,
"%i %i\
 %i %i\n",
                CS_CODE_EXT,
                SWEXTCMD_WEPDISARM,

                src_obj,
		tar_obj
        );
        NetSendData(sndbuf);

        return(0);
}


/*
 *	Requests eco values for tar_obj.
 */
int NetSendEcoReqValues(int src_obj, int tar_obj)
{
        char sndbuf[CS_DATA_MAX_LEN];


        /*
         *   SWEXTCMD_ECO_REQVALUES format:
         *
         *	object_num
         */
        sprintf(sndbuf, 
"%i %i\
 %i\n",
                CS_CODE_EXT,
                SWEXTCMD_ECO_REQVALUES,

		tar_obj
        );  
        NetSendData(sndbuf);

        return(0);
}

/*
 *	Purchases a product from the proprietor to the
 *	customer for product type and amount indicated
 *	in the prodct structure.
 */
int NetSendEcoBuy(
	int customer_obj,
        int proprietor_obj,
        xsw_ecoproduct_struct product
)
{
        char sndbuf[CS_DATA_MAX_LEN];

        /*
         *   SWEXTCMD_ECO_BUY format:
         *
         *	customer_obj proprietor_obj
	 *	amount magic_number;product_name
         */
        sprintf(sndbuf,
"%i %i\
 %i %i %.4f %i;%s\n",
                CS_CODE_EXT,
                SWEXTCMD_ECO_BUY,

                customer_obj,
		proprietor_obj,
		product.amount,
		ECO_EXCHANGE_MAGIC_NUMBER,
		product.name
        );
        NetSendData(sndbuf);


	return(0);
}


/*
 *	Sells a product from the customer to the
 *      proprietor for product type and amount indicated
 *      in the prodct structure.
 */
int NetSendEcoSell(
	int customer_obj,
        int proprietor_obj,
        xsw_ecoproduct_struct product
)  
{       
        char sndbuf[CS_DATA_MAX_LEN];


        /*
         *   SWEXTCMD_ECO_SELL format:
         *
         *      customer_obj proprietor_obj      
         *      amount magic_number;product_name
         */
        sprintf(sndbuf,
"%i %i\
 %i %i %.4f %i;%s\n",
                CS_CODE_EXT,
                SWEXTCMD_ECO_SELL,

                customer_obj,
                proprietor_obj,
                product.amount,
                ECO_EXCHANGE_MAGIC_NUMBER,
                product.name
        );
        NetSendData(sndbuf);


        return(0);  
}       
