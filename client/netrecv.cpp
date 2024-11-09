/*
                   Network Data Recieving Functions

 	Functions:

 	int NetHandleLogin(char *arg)
 	int NetHandleDisconnect(char *arg)

 	int NetHandleLiveMesg(char *arg)
 	int NetHandleSysMesg(char *arg)

 	int NetHandleWhoAmI(char *arg)
	int NetHandleSetUnits(char *arg)
	int NetHandleSetImageSet(char *arg)
        int NetHandleSetSoundSet(char *arg)
	int NetHandleSetForward(char *arg)
        int NetHandleSetOCSN(char *arg)
 	int NetHandleCreateObject(char *arg)
 	int NetHandleRecycleObject(char *arg)
	int NetHandleSetObjSect(char *arg)
        int NetHandleSetFObjSect(char *arg)
 	int NetHandleSetPoseObj(char *arg)
 	int NetHandleSetFPoseObj(char *arg)
 	int NetHandleSetObjectValues(char *arg)
 	int NetHandleSetObjectMaximums(char *arg)
 	int NetHandleSetName(char *arg)
 	int NetHandleSetShieldVis(char *arg)
	int NetHandleSetChannel(char *arg)
	int NetHandleSetScore(char *arg)
	int NetHandleSetEngine(char *arg)
	int NetHandleNotifyHit(char *arg)
	int NetHandleNotifyDestroy(char *arg)
	int NetHandleTractorBeamLock(char *arg)
	int NetHandleHail(char *arg)
	int NetHandleComMessage(char *arg)
	int NetHandleWeaponDisarm(char *arg)
	int NetHandleEcoSetValues(char *arg)
	int NetHandleEcoSetProductValues(char *arg)
	int NetHandleStarChartAddObject(char *arg)
        int NetHandleStarChartSetObjectName(char *arg)
        int NetHandleStarChartSetObjectEmpire(char *arg)
        int NetHandleStarChartRecycleObject(char *arg)

 	int NetHandlePlaySound(char *arg)

	int NetHandleSetWeaponValues(char *arg)

 	int NetHandleExtCmd(char *arg);
 	int NetHandleRecv()


 */

#include <stdlib.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/time.h>

#include "../include/string.h"
#include "../include/disk.h"

#include "../include/netio.h"
#include "../include/swnetcodes.h"
#include "../include/swsoundcodes.h"

#include "../include/unvmath.h"
#include "../include/unvutil.h"

#include "xsw.h"
#include "net.h"

#include "starchartwin.h"


#define MIN(a,b)	(((a) < (b)) ? (a) : (b))
#define MAX(a,b)	(((a) > (b)) ? (a) : (b))



/*
 *	Handles a send login request from server.
 *
 *	arg is not used.
 */
int NetHandleLogin(char *arg)
{
	int status;
	char sndbuf[CS_DATA_MAX_LEN];
	char text[CS_DATA_MAX_LEN + 256];


	/* Format login. */
	sprintf(
	    sndbuf,
	    "%i %s;%s;%i\n",
	    CS_CODE_LOGIN,
	    net_parms.login_name,
	    net_parms.login_password,
	    CLIENT_TYPE_CODE
	);
	status = NetSendData(sndbuf);

	if(status)
	{
	    sprintf(text, "Error: Unable to send login.");
	    MesgAdd(text, xsw_color.bp_standard_text);
	}
	else
	{
/*
	    sprintf(text,
		"Sending login name and password for: %s",
		net_parms.login_name
	    );
	    MesgAdd(text, xsw_color.bp_standard_text);
*/
	}

        /*  
         *   We've sent our name and password, that does not mean we are
         *   logged in yet.   We wait for a NET_CMD_WHOAMI response from
         *   server.   Once we recieve a NET_CMD_WHOAMI, then we are   
         *   logged in.
         *
         *   IMPORTANT: The connection_state will be set to
         *   CON_STATE_CONNECTED once we receive all of the following:
         *
         *      * player object number
         *      * position of player object
         *      * sector of player object
         */


	return(status);
}

/*
 *	Handles a disconnect request from server.
 */
int NetHandleDisconnect(char *arg)
{
	/* Close socket and reset all network parameters. */
	NetResetParms();

	/* Must call XSWDoDisconnect() after NetResetParms(). */
	XSWDoDisconnect();

	return(0);
}

/*
 *	Handles a live message.
 */
int NetHandleLiveMesg(char *arg)
{
	int len;
	char larg[CS_MESG_MAX];
	char text[CS_MESG_MAX + 48];


	if(arg == NULL)
	    return(-1);
	len = strlen(arg);
	if(len < 1)
	    return(0);

	/* Copy arg to larg. */
	strncpy(larg, arg, CS_MESG_MAX);
	larg[CS_MESG_MAX - 1] = '\0';

	/* Print message. */
	MesgAdd(larg, xsw_color.bp_standard_text);

        /* Log live message. */
	sprintf(text, "%s\n", larg);
        if(option.log_net)
            LogAppendLineFormatted(fname.log, text);


	return(0);
}

/*
 *	Handles a systems message.
 */
int NetHandleSysMesg(char *arg)
{
	int sys_cmd;
	char *strptr;
	char sys_arg[CS_DATA_MAX_LEN];


	/* Get system message command code. */
	sys_cmd = StringGetNetCommand(arg);

	/* Get system message argument. */
	strptr = StringGetNetArgument(arg);
	if(strptr == NULL)
	    return(-1);
	strncpy(sys_arg, strptr, CS_DATA_MAX_LEN);
	sys_arg[CS_DATA_MAX_LEN - 1] = '\0';


	/* Handle by system message command code. */
	switch(sys_cmd)
	{
	  case CS_SYSMESG_CODE_LOGINFAIL:
	    if(net_parms.connection_state != CON_STATE_CONNECTED)
	    {
                printdw(&err_dw, sys_arg);
	    }
	    else
	    {
		if(option.show_server_errors)
		    printdw(&err_dw, sys_arg);
	    }
	    break;

	  case CS_SYSMESG_CODE_LOGINSUCC:
	    /* Ignore this. */
	    break;

	  case CS_SYSMESG_CODE_ABNDISCON:
            if(!option.show_server_errors)
                break;

	    break;

	  case CS_SYSMESG_CODE_BADVALUE:
            if(!option.show_server_errors)
                break;

            break;

	  case CS_SYSMESG_CODE_BADARG:
            if(!option.show_server_errors)
                break;

            break;

	  case CS_SYSMESG_CODE_WARNING:
	    if(!option.show_server_errors)
                break;
	    printdw(&err_dw, sys_arg);
            break;

	  default:	/* Other, assume error. */
            if(!option.show_server_errors)
                break;
	    printdw(&err_dw, sys_arg);
	    break;
	}


	return(0);
}

/*
 *	Handles a who am I setting from the server, this basically
 *	sets the player object number.
 */
int NetHandleWhoAmI(char *arg)
{
        char *strptr, *strptr2;
	int object_num = -1;
        char name[XSW_OBJ_NAME_MAX];
        char text[64];


	/* Reset strings. */
	*name = '\0';
	*text = '\0';

        /*
         *    Format CS_CODE_WHOAMI:
         *
         *      "object_num;name"
         */

        strptr = arg;

        /* Get object number. */
        if(strptr != NULL)
        {
            strncpy(text, strptr, 64);
            text[64 - 1] = '\0';

            strptr2 = strchr(text, CS_STRING_DELIMINATOR_CHAR);
            if(strptr2 != NULL)
                *strptr2 = '\0';

            object_num = atoi(text);

            /* Seek next. */
            strptr = strchr(strptr, CS_STRING_DELIMINATOR_CHAR);
            if(strptr != NULL)  
                strptr++;
        }

        /* Get object name. */
        if(strptr != NULL)
        {
            strncpy(name, strptr, XSW_OBJ_NAME_MAX);
            name[XSW_OBJ_NAME_MAX - 1] = '\0';

	    strptr2 = strchr(name, CS_STRING_DELIMINATOR_CHAR);
            if(strptr2 != NULL)
                *strptr2 = '\0';

            /* Seek next. */  
            strptr = strchr(strptr, CS_STRING_DELIMINATOR_CHAR);
            if(strptr != NULL) 
                strptr++;
        }



	/* Make sure object number is valid. */
        if((object_num < 0) || (object_num >= MAX_OBJECTS))
            return(-1);

	/* Check if player object number was previously not set. */
	if((net_parms.player_obj_num < 0) &&
           (object_num > -1)
	)
	{
	    /* Update background music when we get player object. */
	    if(option.music)
		XSWDoChangeBackgroundMusic();
	}


	/* Set global player object referances (if object is valid). */
	DBSetPlayerObject(object_num);

	/* Set global player object number explicitly incase the object
	 * was invalid and DBSetPlayerObject() would set it to -1.
	 */
	net_parms.player_obj_num = object_num;

	/* Mark that we got the player object referance. */
	net_parms.login_got_lplayer = 1;


	/* Set name. */
/* Do not set, object might not exist yet.
	strncpy(xsw_object[object_num]->name, name, XSW_OBJ_NAME_MAX);
	xsw_object[object_num]->name[XSW_OBJ_NAME_MAX - 1] = '\0';
*/

	/* The player object may not have been created yet, we do
	 * not create it in this function.
	 *
	 * The reason is because we do not know the isref_num
	 * thus we do not have enough information to pass to
	 * DBCreateExplicitObject().
	 *
	 * The player object will be created when the minimal
	 * information has been recieved in one of the recieve
	 * handling functions.
	 */
	if(!DBIsObjectGarbage(net_parms.player_obj_num))
	{
            /* Redraw bridge console panels as needed. */
            BridgeWinDrawPanel(object_num, BPANEL_DETAIL_P2);
/*
            BridgeWinDrawPanel(object_num, BPANEL_DETAIL_P3);
 */
	}


	return(0);
}

/*
 *	Handles a set units.
 */
int NetHandleSetUnits(char *arg)
{
	double ru_to_au;


        /*
         *   Format SWEXTCMD_SETUNITS:
         *
         *	ru_to_au
         */
	sscanf(arg,
"%lf",
		&ru_to_au
	);


	if(ru_to_au > 0)
	    sw_units.ru_to_au = ru_to_au;

	/* Need to redraw all windows. */
	BridgeWinDrawAll();

	return(0);
}

/*
 *	Handles a set image set file.
 */
int NetHandleSetImageSet(char *arg)
{
	char *strptr;
	struct stat stat_buf;
	char filename[PATH_MAX + NAME_MAX];
	char text[PATH_MAX + NAME_MAX + 512];


        /*
         *   Format CS_CODE_IMAGESET:
         *
         *      path
         */
	strptr = arg;
	strncpy(filename, strptr, PATH_MAX + NAME_MAX);
	filename[PATH_MAX + NAME_MAX - 1] = '\0';


	/* Check if already loaded. */
	if(ISPATHABSOLUTE(filename))
	{
            /* Paths different? */
	    if(strcmp(fname.isr, filename))
	    {
                /* Does file exist? */
                if(stat(filename, &stat_buf))
		{
		    sprintf(text,
 "Cannot find universe required image set referance file:\n\n    %s",
			filename
		    );
		    if(option.show_net_errors)
		        printdw(&err_dw, text);
                    return(-1);
		}
                /* Is it a directory? */
		if(S_ISDIR(stat_buf.st_mode))
                {
                    sprintf(text,
 "Cannot find universe required image set referance file:\n\n    %s",
                        filename
                    );
		    if(option.show_net_errors)
			printdw(&err_dw, text);
                    return(-1);
                }

		strncpy(fname.isr, filename, PATH_MAX + NAME_MAX);
		fname.isr[PATH_MAX + NAME_MAX - 1] = '\0';
		ISRefLoadFromFile(fname.isr);
	    }
	}
	else
	{
	    strptr = PrefixPaths(dname.images, filename);
	    if(strptr == NULL)
		return(-1);
	    strncpy(filename, strptr, PATH_MAX + NAME_MAX);
            filename[PATH_MAX + NAME_MAX - 1] = '\0';

            /* Paths different? */
            if(strcmp(fname.isr, filename))
            {
		/* Does file exist? */
                if(stat(filename, &stat_buf))
                {   
                    sprintf(text,
 "Cannot find universe required image set referance file:\n\n    %s",
                        filename
                    );
		    if(option.show_net_errors)
                        printdw(&err_dw, text);
                    return(-1);
                }
		/* Is it a directory? */
                if(S_ISDIR(stat_buf.st_mode))
                {
                    sprintf(text,
 "Cannot find universe required image set referance file:\n\n    %s",
                        filename
                    );
		    if(option.show_net_errors)
                        printdw(&err_dw, text);
                    return(-1);
                }

                strncpy(fname.isr, filename, PATH_MAX + NAME_MAX);
                fname.isr[PATH_MAX + NAME_MAX - 1] = '\0';
                ISRefLoadFromFile(fname.isr);
            }
	}


	return(0);
}


/*
 *	Handles a set sound set file.
 */
int NetHandleSetSoundSet(char *arg)
{
        char *strptr;
        struct stat stat_buf;
	char filename[PATH_MAX + NAME_MAX];
        char text[PATH_MAX + NAME_MAX + 512]; 


        /*
         *   Format CS_CODE_SOUNDSET:
         *
         *      path
         */
        strptr = arg;
        strncpy(filename, strptr, PATH_MAX + NAME_MAX);
        filename[PATH_MAX + NAME_MAX - 1] = '\0';


        /* Check if already loaded. */
        if(ISPATHABSOLUTE(filename))
        {
            /* Paths different? */
            if(strcmp(fname.sound_scheme, filename))
            {
		/* Does file exist? */
                if(stat(filename, &stat_buf))
                {
                    sprintf(text,
 "Cannot find universe required sound scheme:\n\n    %s",
                        filename
                    );
		    if(option.show_net_errors)
                        printdw(&err_dw, text);
                    return(-1);
                }
                /* Is it a directory? */
                if(S_ISDIR(stat_buf.st_mode))
                {
                    sprintf(text,
 "Cannot find universe required sound scheme:\n\n    %s",
                        filename
                    );
		    if(option.show_net_errors)
                        printdw(&err_dw, text);
                    return(-1);
                }

                strncpy(fname.sound_scheme, filename, PATH_MAX + NAME_MAX);
                fname.sound_scheme[PATH_MAX + NAME_MAX - 1] = '\0';
                SSLoadFromFile(fname.sound_scheme);
            }
        }       
        else
        {
            strptr = PrefixPaths(dname.sounds, filename);
            if(strptr == NULL)
                return(-1);
            strncpy(filename, strptr, PATH_MAX + NAME_MAX);
            filename[PATH_MAX + NAME_MAX - 1] = '\0';
 
	    /* Paths different? */
            if(strcmp(fname.sound_scheme, filename))
            {
		/* Does it exist? */
		if(stat(filename, &stat_buf))
                {   
                    sprintf(text,
 "Cannot find universe required sound scheme:\n\n    %s",
                        filename
                    );
                    if(option.show_net_errors)
                        printdw(&err_dw, text);
                    return(-1);
                }
                /* Is it a directory? */
                if(S_ISDIR(stat_buf.st_mode))
                {
                    sprintf(text,
 "Cannot find universe required sound scheme:\n\n    %s",
                        filename
                    );
		    if(option.show_net_errors)
                        printdw(&err_dw, text);
                    return(-1);
                }

                strncpy(fname.sound_scheme, filename, PATH_MAX + NAME_MAX);
                fname.sound_scheme[PATH_MAX + NAME_MAX - 1] = '\0';
                SSLoadFromFile(fname.sound_scheme);
            }
        }


        return(0);
}

/*
 *	Handles a forward.
 */
int NetHandleSetForward(char *arg)
{
	char *strptr;
	char url[5 * CS_DATA_MAX_LEN];
	char address[CS_DATA_MAX_LEN];
	char login_name[CS_DATA_MAX_LEN];
	char login_password[CS_DATA_MAX_LEN];
	int port;


        /*
         *   CS_CODE_FORWARD forward:
         *
         *      url
         */
        strncpy(url, arg, CS_DATA_MAX_LEN);
	url[CS_DATA_MAX_LEN - 1] = '\0';


	/* Parse address (must be specified). */
        strptr = StringParseAddress(url);
        if(strptr == NULL)
	    return(-1);
	else
	    strncpy(address, strptr, CS_DATA_MAX_LEN);
	address[CS_DATA_MAX_LEN - 1] = '\0';

	/* Parse port. */
	port = StringParsePort(url);
	if(port < 0)
	    port = net_parms.port;

	/* Parse login name. */
	strptr = StringParseName(url);
        if(strptr == NULL)
            strncpy(login_name, net_parms.login_name, CS_DATA_MAX_LEN);
	else
	    strncpy(login_name, strptr, CS_DATA_MAX_LEN);
	login_name[CS_DATA_MAX_LEN - 1] = '\0';

	/* Parse login password. */
	strptr = StringParsePassword(url);
        if(strptr == NULL)
	    strncpy(login_password, net_parms.login_password, CS_DATA_MAX_LEN);
	else
	    strncpy(login_password, strptr, CS_DATA_MAX_LEN);
	login_password[CS_DATA_MAX_LEN - 1] = '\0';


	/* Format new url and connect. */
	sprintf(url, "swserv://%s:%s@%s:%i",
	    login_name,
	    login_password,
	    address,
	    port
	);
	XSWDoConnect(url);


	return(0);
}

/*
 *	Handles a set object create script names file.
 */
int NetHandleSetOCSN(char *arg)
{
        char *strptr;
        char loaded_new = 0;
        struct stat stat_buf;
	char filename[PATH_MAX + NAME_MAX];
        char text[PATH_MAX + NAME_MAX + 512]; 


	/*
	 *   Format SWEXTCMD_SETOCSN:
	 *
	 *	path
         */
        strptr = arg;
        strncpy(filename, strptr, PATH_MAX + NAME_MAX);
        filename[PATH_MAX + NAME_MAX - 1] = '\0';


        /* Check if already loaded. */
        if(ISPATHABSOLUTE(filename))
        {
            /* Paths different? */
            if(strcmp(fname.ocsn, filename))
            {
		/* Does file exist? */
                if(stat(filename, &stat_buf))
                {
                    sprintf(text,
 "Cannot find universe required OCSN file:\n\n    %s",
                        filename
                    );
		    if(option.show_net_errors)
                        printdw(&err_dw, text);
                    return(-1);
                }
                /* Is it a directory? */
                if(S_ISDIR(stat_buf.st_mode))
                {
                    sprintf(text,
 "Cannot find universe required OCSN file:\n\n    %s",
                        filename
                    );
		    if(option.show_net_errors)
                        printdw(&err_dw, text);
                    return(-1);
                }

                strncpy(fname.ocsn, filename, PATH_MAX + NAME_MAX);
                fname.ocsn[PATH_MAX + NAME_MAX - 1] = '\0';
                OCSLoadFromFile(fname.ocsn);

		loaded_new = 1;
            }
        }
        else
        {
            strptr = PrefixPaths(dname.etc, filename);
            if(strptr == NULL)
                return(-1);
            strncpy(filename, strptr, PATH_MAX + NAME_MAX);
            filename[PATH_MAX + NAME_MAX - 1] = '\0';

            /* Paths different? */
            if(strcmp(fname.ocsn, filename))
            {
		/* Does file exist? */
                if(stat(filename, &stat_buf))
                {   
                    sprintf(text,
 "Cannot find universe required OCSN file:\n\n    %s",                
                        filename
                    );
		    if(option.show_net_errors)
                        printdw(&err_dw, text);
                    return(-1);
                }
                /* Is it a directory? */
                if(S_ISDIR(stat_buf.st_mode))
                {
                    sprintf(text,
 "Cannot find universe required OCSN file:\n\n    %s",
                        filename  
                    );
		    if(option.show_net_errors)
                        printdw(&err_dw, text);
                    return(-1);
                }
  
                strncpy(fname.ocsn, filename, PATH_MAX + NAME_MAX);
                fname.ocsn[PATH_MAX + NAME_MAX - 1] = '\0';
                OCSLoadFromFile(fname.ocsn);

		loaded_new = 1;
            }
        }


	/* Update resources needed when a new OCSN is loaded. */
	if(loaded_new)
	{
	    /* Remove weapons from player object so they get recreated. */
	    if(!DBIsObjectGarbage(net_parms.player_obj_num))
		UNVAllocObjectWeapons(
		    net_parms.player_obj_ptr,
		    0
		);

            /* Recreate the selected weapon viewscreen label. */
            VSDrawUpdateWeaponLabel(
               &bridge_win.vs_weapon_image,
               bridge_win.vs_weapon_buf
            );
	}


	return(0);
}


/*
 *	Handles a create object request.
 */
int NetHandleCreateObject(char *arg)
{
	char set_back_player_obj = 0;
	char loaded_isref = 0;
	int object_num;
        xsw_object_struct *obj_ptr;
	int status;

	int type;
	int isref_num;
	int owner;
	long size;

	int locked_on;
        int intercepting_object;
        double scanner_range;

	long sect_x, sect_y, sect_z;
	double x, y, z;
        double heading, pitch, bank;
	double velocity;
	double velocity_heading, velocity_pitch, velocity_bank;

	int current_frame, total_frames, cycle_times;
	long anim_int;


        /*
         *   CS_CODE_CREATEOBJ format:
         *
         *      object_num,
	 *	type, isref_num, owner, size,
	 *	locked_on, intercepting_object, scanner_range,
	 *	sect_x, sect_y, sect_z,
	 *	x, y, z,
         *      heading, pitch, bank,
         *      velocity,
         *      velocity_heading, velocity_pitch, velocity_bank,
         *      current_frame, anim_int, total_frames, cycle_times
         */
        sscanf(arg,
"%i\
 %i %i %i %ld\
 %i %i %lf\
 %ld %ld %ld\
 %lf %lf %lf\
 %lf %lf %lf\
 %lf\
 %lf %lf %lf\
 %i %ld %i %i",

                &object_num,

		&type,
                &isref_num,
                &owner,
                &size,

                &locked_on,
                &intercepting_object,
                &scanner_range,

                &sect_x,
                &sect_y,
                &sect_z,

                &x,
                &y,
                &z,

                &heading,
                &pitch,
                &bank,

                &velocity,

                &velocity_heading,
                &velocity_pitch,
                &velocity_bank,

                &current_frame,
		&anim_int,
		&total_frames,
		&cycle_times
        );


        /* object_num must be valid. */
        if((object_num < 0) || (object_num >= MAX_OBJECTS))
            return(-1);

        /* isref_num must be a valid number. */
        if((isref_num < 0) || (isref_num >= ISREF_MAX))
            return(-1);


	/* If the object already exists, we must recycle it first. */
	if(!DBIsObjectGarbage(object_num))
	{
	    /* Need to make note to set back referance to player object
	     * after creation since we are about to recycle it.
	     */
	    if(object_num == net_parms.player_obj_num)
	        set_back_player_obj = 1;

            /* Recycle the object. */
            DBRecycleObject(object_num);
	}


        /* Now create new object. */
        status = DBCreateExplicitObject(
            object_num,
            isref_num,
            type,
            owner,
            x,
            y,
            z,
            heading,
            pitch,
            bank
        );
        if(status)
        {
            fprintf(stderr,
                "NetHandleCreateObject(): Cannot create object %i.\n",
                object_num
            );
            return(-1);
        }

        /* Get obj_ptr. */
        obj_ptr = xsw_object[object_num];


	/* Set back player object referance as needed. */
	if(set_back_player_obj)
	    DBSetPlayerObject(object_num);


	/* Set sector. */
	obj_ptr->sect_x = sect_x;
        obj_ptr->sect_y = sect_y;
        obj_ptr->sect_z = sect_z;


        /*   Need to add a little antimatter so reality engine will
         *   allow this object to move if it is intended to move.
	 */
        obj_ptr->antimatter_max = 10;
        obj_ptr->antimatter = 10;


	/* Is this the player object? */
        if(object_num == net_parms.player_obj_num)
	{
	    /*   Player object is intercepting something, let server
	     *   set its attitude.
	     */
	    if(obj_ptr->intercepting_object > -1)
            {
                obj_ptr->heading = heading;
                obj_ptr->pitch = pitch;
		obj_ptr->bank = bank;
		MuSetUnitVector2D(
		    &obj_ptr->attitude_vector_compoent,
		    obj_ptr->heading
		);
            }


	    /* Do not continue further if connected. */
            if(net_parms.connection_state == CON_STATE_CONNECTED)
            {
                BridgeWinDrawAll();

                return(0);
            }
	    /* Not yet connected, still logging in. */
            else
            {
                net_parms.login_got_position = 1;
                net_parms.login_got_sector = 1;

                if(net_parms.login_got_lplayer == 1)
                {
                    net_parms.connection_state = CON_STATE_CONNECTED;

                    /* Set player object referance. */
                    DBSetPlayerObject(object_num);
		}
            }
        }


        /* Isref need to be loaded? */
        if(!ISRefIsLoaded(isref_num))
	{
	    /*   Load isref, disregard error since isref's resources
	     *   are not used here.
	     */
	    ISRefLoad(isref_num);

	    loaded_isref = 1;
	}
          
 
        /* Set new location and apperance values on object. */
	obj_ptr->type = type;

        obj_ptr->imageset = isref_num;
        obj_ptr->owner = owner;
        obj_ptr->size = size;

        obj_ptr->locked_on = ((DBIsObjectGarbage(locked_on)) ? -1 : locked_on);
        obj_ptr->intercepting_object = intercepting_object;
        obj_ptr->scanner_range = scanner_range;

        obj_ptr->velocity = velocity;
        obj_ptr->velocity_heading = velocity_heading;
	obj_ptr->velocity_pitch = velocity_pitch;
	obj_ptr->velocity_bank = velocity_bank;
        MuSetUnitVector2D(
            &obj_ptr->momentum_vector_compoent,
            obj_ptr->velocity_heading
        );

        obj_ptr->animation.current_frame = current_frame;    
        obj_ptr->animation.interval = anim_int;
        obj_ptr->animation.total_frames = total_frames;
	obj_ptr->animation.cycle_count = 0;
	obj_ptr->animation.cycle_times = cycle_times;
	obj_ptr->animation.last_interval = MilliTime();

        /* Refresh the last time it was updated. */
        obj_ptr->last_updated = MilliTime();


	/* Is this a newly created player object? */
	if(object_num == net_parms.player_obj_num)
	{
            /* Redraw entire bridge. */
            BridgeWinDrawAll();
	}


	/* Return number of object that was created. */
	return(object_num);
}

/*
 *	Handles a recycle object request.
 */
int NetHandleRecycleObject(char *arg)
{
	int object_num;


        /*
         *   CS_CODE_RECYCLEOBJ format:
         *
         *      object_num
         */
	sscanf(arg, "%i",
		&object_num
	);

	DBRecycleObject(object_num);

	return(0);
}

/*
 *	Handles a set object sector request.
 */
int NetHandleSetObjSect(char *arg)
{
        int changed_sector = 0;

	int object_num;
	long	sect_x,
		sect_y,
		sect_z;
	xsw_object_struct *obj_ptr;


	/*
	 *   SWEXTCMD_SETOBJSECT format:
	 *
	 *	object_num
	 *	sect_x sect_y sect_z
	 */
	sscanf(arg, "%i %ld %ld %ld",
                &object_num,  
		
		&sect_x,
		&sect_y,
		&sect_z
	);

        if(DBIsObjectGarbage(object_num))
            return(-1);
        else
            obj_ptr = xsw_object[object_num];


        /* Check of sector value changed. */
        if((obj_ptr->sect_x != sect_x) ||
           (obj_ptr->sect_y != sect_y) ||
           (obj_ptr->sect_z != sect_z)
        )
            changed_sector = 1;


	/* If not logged in but connected, then allow server to
	 * set sector position.
	 */
	if(object_num == net_parms.player_obj_num)
	{
	    /* Do not set sector if already logged in. */
	    if(net_parms.connection_state == CON_STATE_CONNECTED)
		return(0);

	    /* Mark that we have recieved sector position. */
	    net_parms.login_got_sector = 1;

	    /*   If other required information about player object
	     *   are recieved, then set connection_state to
	     *   CON_STATE_CONNECTED.
	     */
            if((net_parms.login_got_lplayer == 1) &&
               (net_parms.login_got_position == 1)
            )
	    {
                net_parms.connection_state = CON_STATE_CONNECTED;

                /* Set player object referance. */
                DBSetPlayerObject(object_num);
	    }
        }


	if(changed_sector)
	{
	    obj_ptr->sect_x = sect_x;
	    obj_ptr->sect_y = sect_y;
	    obj_ptr->sect_z = sect_z;
	}

        /* Refresh the last time it was updated. */
        obj_ptr->last_updated = cur_millitime;

	return(0);
}


/*
 *      Handles a forced set object sector request.
 */
int NetHandleSetFObjSect(char *arg)
{
	int changed_sector = 0;

	int object_num;
        long    sect_x,
                sect_y,
                sect_z;
        xsw_object_struct *obj_ptr;
            
          
        /*
         *   SWEXTCMD_SETFOBJSECT format:
         *   
         *      object_num
         *      sect_x sect_y sect_z
         */
        sscanf(arg, "%i %ld %ld %ld",
                &object_num,

                &sect_x,
                &sect_y,
                &sect_z
        );

        if(DBIsObjectGarbage(object_num))
            return(-1);
	else
	    obj_ptr = xsw_object[object_num];


	/* Check of sector value changed. */
	if((obj_ptr->sect_x != sect_x) ||
           (obj_ptr->sect_y != sect_y) ||
           (obj_ptr->sect_z != sect_z)
	)
	    changed_sector = 1;


	if(changed_sector)
	{
	    obj_ptr->sect_x = sect_x;
	    obj_ptr->sect_y = sect_y;
	    obj_ptr->sect_z = sect_z;

	    /* Is this the player object? */
	    if(object_num == net_parms.player_obj_num)
		XSWDoChangeSector(object_num);
	}

        /* Refresh the last time it was updated. */
        obj_ptr->last_updated = cur_millitime;


        return(0);
}


/*
 *	Handles a pose object request, velocity, pitch and coordinates
 *	(except sector) are set by this function.
 *
 *	Player object is not posed here unless it is intercepting
 *	something.
 */
int NetHandleSetPoseObj(char *arg)
{
	char isref_changed = 0;
	char loaded_isref = 0;

	int object_num;

	int type;
	int isref_num;
	long size;

	double x, y, z;
	double heading, pitch, bank;

	double velocity;
	double velocity_heading;
	double velocity_pitch;
        double velocity_bank;

	int current_frame;

	int status;
        xsw_object_struct *obj_ptr;


        /*
         *   CS_CODE_POSEOBJ format:
         *
         *      object_num,
         *      type, isref_num, size,
         *      x, y, z,
         *      heading, pitch, bank,
         *      velocity,
	 *	velocity_heading, velocity_pitch, velocity_bank,
         *      current_frame
         */
	sscanf(arg,
"%i\
 %i %i %ld\
 %lf %lf %lf\
 %lf %lf %lf\
 %lf\
 %lf %lf %lf\
 %i",
		&object_num,

		&type,
		&isref_num,
		&size,

		&x,
		&y,
		&z,

		&heading,
		&pitch,
		&bank,

		&velocity,

		&velocity_heading,
		&velocity_pitch,
		&velocity_bank,

		&current_frame
	);


	/* object_num must be valid (but may not be allocated). */
	if((object_num < 0) || (object_num >= MAX_OBJECTS))
	    return(-1);

	/* isref_num must be a valid number. */
	if((isref_num < 0) || (isref_num >= ISREF_MAX))
	    return(-1);


	/* Has object_num been created yet? */
	if(DBIsObjectGarbage(object_num))
	{
	    status = DBCreateExplicitObject(
		object_num,
		isref_num,
		type,
        	-1,
                x,
                y,
                z,
                heading,
                pitch,
                bank
	    );
	    if(status)
	    {
		fprintf(stderr,
             "NetHandleSetPoseObj(): Cannot create object %i.\n",
		    object_num
		);
		return(-1);
	    }
	    else
	    {
		/* Get obj_ptr. */
		obj_ptr = xsw_object[object_num];

		/* Need to add antimatter to atleast 1. */
		obj_ptr->antimatter_max = 5;
		obj_ptr->antimatter = 5;


		/*   Redraw entire bridge upon object creation if it is
		 *   the player object.
		 */
		if(object_num == net_parms.player_obj_num)
                    BridgeWinDrawAll();
	    }
	}

        /* Get pointer to object. */
        obj_ptr = xsw_object[object_num];


        /* Check if isref number has changed. */
        if(obj_ptr->imageset != isref_num)
	{
            /* Record isref number change and set new number. */
            isref_changed = 1;
            obj_ptr->imageset = isref_num;
	}


        /*
         *   We don't update local player object location or apperance with
         *   server data once we are CON_STATE_CONNECTED.
         *   With the exception that the object is intercepting something.
         */
        if(object_num == net_parms.player_obj_num)
	{
            if(obj_ptr->intercepting_object > -1)
	    {
                obj_ptr->heading = heading;
                obj_ptr->pitch = pitch;
                obj_ptr->bank = bank;
                MuSetUnitVector2D(
                    &obj_ptr->attitude_vector_compoent,
                    obj_ptr->heading
                );
            }

	    /* Redraw nessasary bridge console panels as needed. */
	    if(isref_changed)
	    {
		BridgeWinDrawPanel(
		    object_num,
		    BPANEL_DETAIL_P2
		);
	    }


	    /* Do not continue further if connected. */
	    if(net_parms.connection_state == CON_STATE_CONNECTED)
                return(0);

	    net_parms.login_got_position = 1;

	    if((net_parms.login_got_lplayer == 1) &&
	       (net_parms.login_got_sector == 1)
	    )
	    {
                net_parms.connection_state = CON_STATE_CONNECTED;

                /* Set player object referance. */
                DBSetPlayerObject(object_num);
	    }
        }

	/* Check if player object's locked on this object. */
	if(net_parms.player_obj_ptr != NULL)
	{
            if(object_num == net_parms.player_obj_ptr->locked_on)
            {
                /* Redraw bridge console panels as needed. */
                if(isref_changed)
                {
                    BridgeWinDrawPanel(
                        object_num,
                        BPANEL_DETAIL_S1
                    );
                }
	    }
	}


	/* Isref need to be loaded? */
        if(!ISRefIsLoaded(isref_num))
        {
            /*   Load isref, disregard error since isref's resources     
             *   are not used here.   
             */
            ISRefLoad(isref_num);
            
            loaded_isref = 1;
        }

	/* Set new location and apperance values on object. */
        obj_ptr->type = type;
	/* Already set isref number. */
	obj_ptr->size = size;

        obj_ptr->x = x;
        obj_ptr->y = y;
        obj_ptr->z = z;

        obj_ptr->heading = heading;
        obj_ptr->pitch = pitch;
	obj_ptr->bank = bank;
        MuSetUnitVector2D(
            &obj_ptr->attitude_vector_compoent,
            obj_ptr->heading
        );

        obj_ptr->velocity = velocity;
        obj_ptr->velocity_heading = velocity_heading;
	obj_ptr->velocity_pitch = velocity_pitch;
	obj_ptr->velocity_bank = velocity_bank;
        MuSetUnitVector2D(
            &obj_ptr->momentum_vector_compoent,
            obj_ptr->velocity_heading
        );

	/* Increase total frames if current frame exceeds total. */
	if(current_frame >= obj_ptr->animation.total_frames)
	    obj_ptr->animation.total_frames = current_frame + 1;

	/* Update animation frame only if not updating locally. */
	if(!option.local_updates)
	    obj_ptr->animation.current_frame = current_frame;


	/* Refresh the last time it was updated. */
	obj_ptr->last_updated = (loaded_isref) ? MilliTime() : cur_millitime;



	return(0);
}



/*
 *	Handles a force pose object request, this will make sure the
 *	object including the player object gets posed by server data.
 */
int NetHandleSetFPoseObj(char *arg)
{
	char loaded_isref = 0;
	char isref_changed = 0;

        int object_num;

	int type;
        int isref_num;
        long size;

	double x, y, z;
        double heading, pitch, bank;

        double velocity;
        double velocity_heading;
        double velocity_pitch;
        double velocity_bank;

        int current_frame;

        int status;
	xsw_object_struct *obj_ptr;

                     
        /*
         *   CS_CODE_FORCEPOSEOBJ format:
         *
         *      object_num,
         *      type, isref_num, size,
         *      x, y, z,
         *      heading, pitch, bank,
         *      velocity,
         *      velocity_heading, velocity_pitch, velocity_bank,
         *      current_frame
         */
        sscanf(arg,
"%i\
 %i %i %ld\
 %lf %lf %lf\
 %lf %lf %lf\
 %lf\
 %lf %lf %lf\
 %i",
                &object_num,

                &type,
                &isref_num,
                &size,
                
                &x,
                &y,  
                &z,   
                  
                &heading,
                &pitch,
                &bank,  
                
                &velocity,
            
                &velocity_heading,
                &velocity_pitch,
                &velocity_bank,
                    
                &current_frame
        );


        /* object_num must be valid. */
        if((object_num < 0) || (object_num >= MAX_OBJECTS))
            return(-1);

        /* isref_num must be a valid number. */
        if((isref_num < 0) || (isref_num >= ISREF_MAX))
            return(-1);
             

        /* Has object_num been created yet? */
        if(DBIsObjectGarbage(object_num))
        {
            status = DBCreateExplicitObject(
                object_num,
                isref_num,
                type,
                -1,
                x,
                y,
                z,
                heading,
                pitch,
                bank
            );
            if(status)
            {
                fprintf(stderr,
 "NetHandleSetPoseObj(): DBCreateExplicitObject(): Cannot create object %i.\n",
                    object_num
                );
                return(-1);
            }
            else
            {
                /* Get obj_ptr. */
                obj_ptr = xsw_object[object_num];

                /* Need to add antimatter to atleast 1. */
                obj_ptr->antimatter_max = 5; 
                obj_ptr->antimatter = 5;


                /*   Redraw entire bridge upon object creation if it is
                 *   the player object. 
                 */
                if(object_num == net_parms.player_obj_num)
                    BridgeWinDrawAll();
            }
        }


	/* Get pointer to object. */
	obj_ptr = xsw_object[object_num];


	/* Check if isref number changed. */
	if(obj_ptr->imageset != isref_num)
	{
	    /* Record isref number change and set new number. */
            isref_changed = 1;
	    obj_ptr->imageset = isref_num;
	}


	/* Handle connection state change if any. */
        if((object_num == net_parms.player_obj_num) &&
           (net_parms.connection_state != CON_STATE_CONNECTED)
        )
	{
	    /* This is force pose, player object may be changed. */

	    net_parms.login_got_position = 1;

	    if((net_parms.login_got_lplayer == 1) &&
               (net_parms.login_got_sector == 1)
            )
	    {
                net_parms.connection_state = CON_STATE_CONNECTED;

                /* Set player object referance. */
                DBSetPlayerObject(object_num);
	    }
        }


	/* Is this the player object? */
	if(net_parms.player_obj_ptr == obj_ptr)
	{
            /* Redraw bridge console panels as needed. */
            if(isref_changed)
            {
                BridgeWinDrawPanel(
                    object_num,
                    BPANEL_DETAIL_P2
                );
            }
	}

        /* Check if player object's locked on this object. */
        if(net_parms.player_obj_ptr != NULL)
        {
            if(object_num == net_parms.player_obj_ptr->locked_on)
            {
                /* Redraw bridge console panels as needed. */
                if(isref_changed)
                {
                    BridgeWinDrawPanel(
                        object_num,
                        BPANEL_DETAIL_S1
                    );
                }
            }
        }


        /* Isref need to be loaded? */ 
        if(!ISRefIsLoaded(isref_num))
        {
            /*   Load isref, disregard error since isref's resources     
             *   are not used here.   
             */
            ISRefLoad(isref_num);

            loaded_isref = 1;
        }


        /* Set new location and apperance values on object. */
        obj_ptr->type = type;
	/* Already set isref number. */
        obj_ptr->size = size;

        obj_ptr->x = x;
        obj_ptr->y = y;
        obj_ptr->z = z;

        obj_ptr->heading = heading;
        obj_ptr->pitch = pitch;
	obj_ptr->bank = bank;
	MuSetUnitVector2D(
	    &obj_ptr->attitude_vector_compoent,
	    obj_ptr->heading
	);

        obj_ptr->velocity = velocity;
        obj_ptr->velocity_heading = velocity_heading;
	obj_ptr->velocity_pitch = velocity_pitch;
	obj_ptr->velocity_bank = velocity_bank;
        MuSetUnitVector2D(
            &obj_ptr->momentum_vector_compoent,
            obj_ptr->velocity_heading
        );

        /* Increase total frames if current frame exceeds total. */
        if(current_frame >= obj_ptr->animation.total_frames)
            obj_ptr->animation.total_frames = current_frame + 1;

        /* Update animation frame only if not updating locally. */
        if(!option.local_updates)
            obj_ptr->animation.current_frame = current_frame;

        /* Refresh the last time it was updated. */
        obj_ptr->last_updated = (loaded_isref) ? MilliTime() : cur_millitime;


	return(0);
}

/*
 *	Handles set object values.
 */
int NetHandleSetObjectValues(char *arg)
{
	int object_num;

	int loc_type;
	long locked_on;
	char changed_locked = 0;
	long intercepting_object;
	char changed_intercepting = 0;

	int thrust_rev_state;
	double thrust_dir;
	double thrust;
	double throttle;

	int lighting;	/* Really xswo_lighting_t, which is u_int8_t */
	double hp;
	double power;
	double antimatter;
	int shield_state;

	int selected_weapon;
	char changed_selected_weapon = 0;
	int cloak_state;
	double cloak_strength;
	double visibility;

	int damage_control;

        xsw_object_struct *obj_ptr;


        /*
         *   SWEXTCMD_STDOBJVALS format:
         *
	 *	object_num,
	 *	loc_type, locked_on, intercepting_object,
	 *	thrust_rev_state, thrust_dir, thrust, throttle,
	 *	lighting, hp, power,
	 *	antimatter, shield_state, selected_weapon, cloak_state,
	 *	cloak_strength, visibility, damage_control
         */
        sscanf(arg,
"%i\
 %i %ld %ld\
 %i %lf %lf %lf\
 %i %lf %lf\
 %lf %i %i %i\
 %lf %lf %i",

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
             
                
        /* Has object_num been created yet? */
        if(DBIsObjectGarbage(object_num))
        {
	    /*   Note we do not create the object since this function
	     *   does not get enough information to create an object.
	     */
            return(0);
        }
	else
	    obj_ptr = xsw_object[object_num];


	/* Set object values. */

	/* Locked on object must not be garbage or else do not set. */
	if(DBIsObjectGarbage(locked_on))
	{
	    changed_locked = ((obj_ptr->locked_on == -1) ? 0 : 1);
	    obj_ptr->locked_on = -1;
	}
	else
	{
	    changed_locked = ((obj_ptr->locked_on == locked_on) ? 0 : 1);
	    obj_ptr->locked_on = locked_on;
	}

	changed_intercepting =
	    ((obj_ptr->intercepting_object == intercepting_object) ? 0 : 1);
        obj_ptr->intercepting_object = intercepting_object;

	changed_selected_weapon =
	    ((obj_ptr->selected_weapon == selected_weapon) ? 0 : 1);


	/* Skip velocity and thrust values for player object.
	 *
	 * The player object must not be affected by movement
	 * settings given by server.
	 */
        if(object_num != net_parms.player_obj_num)
	{
	    obj_ptr->loc_type = loc_type;
/*
            obj_ptr->thrust_rev_state = thrust_rev_state;
 */
            obj_ptr->thrust_dir = thrust_dir;
            obj_ptr->thrust = thrust;
            obj_ptr->throttle = throttle;
	}
	obj_ptr->lighting = lighting;
        obj_ptr->hp = hp;
        obj_ptr->power = power;
        obj_ptr->antimatter = antimatter;
        obj_ptr->shield_state = shield_state;
        obj_ptr->selected_weapon = selected_weapon;
        obj_ptr->cloak_state = cloak_state;
        obj_ptr->cloak_strength = cloak_strength;
        obj_ptr->visibility = visibility;
        obj_ptr->damage_control = damage_control;


        /* Refresh the last time it was updated. */
        obj_ptr->last_updated = cur_millitime;


	/* Redraw stats if this was the local object. */
	if(obj_ptr == net_parms.player_obj_ptr)
	{
            BridgeWinDrawPanel(
		net_parms.player_obj_num, BPANEL_DETAIL_PHULL
	    );
            BridgeWinDrawPanel(
		net_parms.player_obj_num, BPANEL_DETAIL_PPOWER
	    );
            BridgeWinDrawPanel(
		net_parms.player_obj_num, BPANEL_DETAIL_PSHIELDS
	    );
            BridgeWinDrawPanel(
		net_parms.player_obj_num, BPANEL_DETAIL_PVIS
	    );
            BridgeWinDrawPanel(
		net_parms.player_obj_num, BPANEL_DETAIL_PDMGCTL
	    );

	    if(changed_intercepting)
                BridgeWinDrawPanel(
		    net_parms.player_obj_num, BPANEL_DETAIL_PINAME
		);

            BridgeWinDrawPanel(
		net_parms.player_obj_num, BPANEL_DETAIL_PWEAPONS
	    );

            BridgeWinDrawPanel(
		net_parms.player_obj_num, BPANEL_DETAIL_PANTIMATTER
	    );
/*
            BridgeWinDrawPanel(
		net_parms.player_obj_num, BPANEL_DETAIL_PENGINESTATE
	    );
            BridgeWinDrawPanel(
		net_parms.player_obj_num, BPANEL_DETAIL_PTHRUSTVECTOR
	    );
 */
	    if(changed_locked)
	    {
                BridgeWinDrawPanel(
		    net_parms.player_obj_num, BPANEL_DETAIL_PWLOCK
		);

                BridgeWinDrawPanel(locked_on, BPANEL_DETAIL_S1);
                BridgeWinDrawPanel(locked_on, BPANEL_DETAIL_S2);
                BridgeWinDrawPanel(locked_on, BPANEL_DETAIL_S3);

		if(option.music)
		    XSWDoChangeBackgroundMusic();
	    }

	    if(changed_selected_weapon)
	    {
                /* Recreate selected weapon stats label on viewscreen. */
                VSDrawUpdateWeaponLabel(
                    &bridge_win.vs_weapon_image,
                    bridge_win.vs_weapon_buf
                );
	    }
	}


	return(0);
}

/*
 *	Handles a set object maximums request.
 */
int NetHandleSetObjectMaximums(char *arg)
{
	char loaded_isref = 0;
	char isref_changed = 0;

	int object_num;
	int status;

	int type;
	int isref_num;
	int owner;
	long size;
	double scanner_range;
	double velocity_max;
	double thrust_power;
	double turnrate;
	double hp_max;
	double power_max;
	double power_purity;
	double core_efficency;
	double antimatter_max;
	int total_weapons;
	double visibility;

	xsw_object_struct *obj_ptr;


        /*
	 *   SWEXTCMD_STDOBJMAXS format:
	 *
	 *	object_num,
	 *	type, isref_num, owner, size, scanner_range,
	 *	velocity_max, thrust_power, turnrate, hp_max, power_max,
	 *	power_purity, core_efficency, antimatter_max, total_weapons, visibility
	 */
        sscanf(arg,
"%i\
 %i %i %i %ld %lf\
 %lf %lf %lf %lf %lf\
 %lf %lf %lf %i %lf",

                &object_num,

                &type,
                &isref_num,
                &owner,
                &size,
                &scanner_range,

                &velocity_max,
                &thrust_power,
                &turnrate,
                &hp_max,
                &power_max,

                &power_purity,
                &core_efficency,
                &antimatter_max,
                &total_weapons,
                &visibility

        );


        /* See if object_num is valid. */
        if((object_num < 0) || (object_num >= MAX_OBJECTS))
            return(-1);

        /* See if isref_num is a valid number. */
        if((isref_num < 0) || (isref_num >= ISREF_MAX))
            return(-1);


        /* Has object_num been created yet? */
        if(DBIsObjectGarbage(object_num))
        {
            status = DBCreateExplicitObject(
                object_num,
                isref_num,
                type,
                owner,
                0,
                0,
                0,
                0,
                0,
                0
            );
            if(status)
            {
                fprintf(stderr,
       "NetHandleSetObjectMaximums(): Error creating object %i.\n",
                    object_num
		);
                return(-1);
            }
	    else
	    {
                /* Redraw bridge console panels if player object was
		 * created here.
		 */
                if(object_num == net_parms.player_obj_num)
		    BridgeWinDrawAll();
	    }
        }


        /* Get pointer to object. */
        obj_ptr = xsw_object[object_num];


        /* Check if isref number changed. */
	if(obj_ptr->imageset != isref_num)
	{
	    isref_changed = 1;
	    obj_ptr->imageset = isref_num;
	}


	/* Is this the player object? */
	if(obj_ptr == net_parms.player_obj_ptr)
	{
	    /* Redraw bridge console panels as needed. */
            if(isref_changed)
            {
                BridgeWinDrawPanel(
                    object_num,
                    BPANEL_DETAIL_P2
                );
            }

	    /* Scanner range change? */
	    if(obj_ptr->scanner_range != scanner_range)
            {
                obj_ptr->scanner_range = scanner_range;

                /* Recreate range label. */
                ScannerUpdateLabels(object_num);
	    }
	}


        /* Isref need to be loaded? */ 
        if(!ISRefIsLoaded(isref_num))
        {
            /*   Load isref, disregard error since isref's resources     
             *   are not used here.   
             */
            ISRefLoad(isref_num);

            loaded_isref = 1;
        }


        /* Set new maximum values on object. */
        obj_ptr->type = type;
	/* Already set isref number. */
        obj_ptr->owner = owner;
        obj_ptr->size = size;
        obj_ptr->scanner_range = scanner_range;

        obj_ptr->velocity_max = velocity_max; 
        obj_ptr->thrust_power = thrust_power;
        obj_ptr->turnrate = turnrate;
        obj_ptr->hp_max = hp_max;
        obj_ptr->power_max = power_max;

        obj_ptr->power_purity = power_purity;
        obj_ptr->core_efficency = core_efficency;
        obj_ptr->antimatter_max = antimatter_max;

	/* Need to reallocate weapons? */
	if(obj_ptr->total_weapons != total_weapons)
	    UNVAllocObjectWeapons(obj_ptr, total_weapons);

        obj_ptr->visibility = visibility;


        /* Refresh the last time it was updated. */
        obj_ptr->last_updated = (loaded_isref) ? MilliTime() : cur_millitime;


	/* Redraw displays. */
	if(obj_ptr == net_parms.player_obj_ptr)
	{
            BridgeWinDrawPanel(
		net_parms.player_obj_num, BPANEL_DETAIL_PHULL
	    );
            BridgeWinDrawPanel(
		net_parms.player_obj_num, BPANEL_DETAIL_PPOWER
	    );
            BridgeWinDrawPanel(
		net_parms.player_obj_num, BPANEL_DETAIL_PSHIELDS
	    );
            BridgeWinDrawPanel(
		net_parms.player_obj_num, BPANEL_DETAIL_PVIS
	    );
            BridgeWinDrawPanel(
		net_parms.player_obj_num, BPANEL_DETAIL_PDMGCTL
	    );

            BridgeWinDrawPanel(
		net_parms.player_obj_num, BPANEL_DETAIL_PINAME
	    );
            BridgeWinDrawPanel(
		net_parms.player_obj_num, BPANEL_DETAIL_PWLOCK
	    );
            BridgeWinDrawPanel(
		net_parms.player_obj_num, BPANEL_DETAIL_PWEAPONS
	    );

            BridgeWinDrawPanel(
		net_parms.player_obj_num, BPANEL_DETAIL_PANTIMATTER
	    );
            BridgeWinDrawPanel(
		net_parms.player_obj_num, BPANEL_DETAIL_PENGINESTATE
	    );
            BridgeWinDrawPanel(
		net_parms.player_obj_num, BPANEL_DETAIL_PTHRUSTVECTOR
	    );
	}


	return(0);
}

/*
 *	Handles a set object name.
 */
int NetHandleSetName(char *arg)
{
	int object_num = -1;
        char *strptr, *strptr2;
        xsw_object_struct *obj_ptr;
	char text[64];
	char name[XSW_OBJ_NAME_MAX];
	char empire[XSW_OBJ_EMPIRE_MAX];


	/* Reset strings. */
	*text = '\0';
	*name = '\0';
	*empire = '\0';

        /*
         *   NET_CMD_SETNAME format:
         *
         *	"object_num;name;other"
         */

	strptr = arg;

	/* Get object number. */
	if(strptr != NULL)
	{
	    strncpy(text, strptr, 64);
	    text[64 - 1] = '\0';

	    strptr2 = strchr(text, CS_STRING_DELIMINATOR_CHAR);
	    if(strptr2 != NULL)
		*strptr2 = '\0';

	    object_num = atoi(text);

	    /* Seek next. */
	    strptr = strchr(strptr, CS_STRING_DELIMINATOR_CHAR);
	    if(strptr != NULL)
		strptr++;
	}

	/* Get name. */
	if(strptr != NULL)
	{
	    strncpy(name, strptr, XSW_OBJ_NAME_MAX);
	    name[XSW_OBJ_NAME_MAX - 1] = '\0';

	    strptr2 = strchr(name, CS_STRING_DELIMINATOR_CHAR);
	    if(strptr2 != NULL)
		*strptr2 = '\0';

            /* Seek next. */
            strptr = strchr(strptr, CS_STRING_DELIMINATOR_CHAR);
            if(strptr != NULL)
                strptr++;
	}

	/* Get empire. */
	if(strptr != NULL)
	{
	    strncpy(empire, strptr, XSW_OBJ_EMPIRE_MAX);
            empire[XSW_OBJ_EMPIRE_MAX - 1] = '\0';

	    strptr2 = strchr(empire, CS_STRING_DELIMINATOR_CHAR);
	    if(strptr2 != NULL)
		*strptr2 = '\0';

            /* Seek next. */
            strptr = strchr(strptr, CS_STRING_DELIMINATOR_CHAR);
            if(strptr != NULL)
                strptr++;
	}


	/* Make sure object_num is valid. */
	if(DBIsObjectGarbage(object_num))
	    return(-1);
	else
	    obj_ptr = xsw_object[object_num];


	/* Set new name for object. */
	strncpy(obj_ptr->name, name, XSW_OBJ_NAME_MAX);
	obj_ptr->name[XSW_OBJ_NAME_MAX - 1] = '\0';

	/* Set new empire for object. */
	if(*empire != '\0')
	{
	    strncpy(obj_ptr->empire, empire, XSW_OBJ_EMPIRE_MAX);
            obj_ptr->empire[XSW_OBJ_EMPIRE_MAX - 1] = '\0';
	}


	/* Create viewscreen label image for that object. */
	if(option.show_formal_label == 2)
	{
            VSLabelAdd(
		DBGetFormalNameStr(object_num),
		xsw_color.vs_label_fg_cv,
                xsw_color.vs_label_bg_cv,
		xsw_font.console_standard,
                obj_ptr
            );
	}
	else if(option.show_formal_label == 1)
	{
	    if(obj_ptr->owner == net_parms.player_obj_num)
                VSLabelAdd(
                    DBGetFormalNameStr(object_num),
                    xsw_color.vs_label_fg_cv,
                    xsw_color.vs_label_bg_cv,
                    xsw_font.console_standard,
                    obj_ptr
                );
	    else
                VSLabelAdd(
                    obj_ptr->name,
                    xsw_color.vs_label_fg_cv,
                    xsw_color.vs_label_bg_cv,
                    xsw_font.console_standard,
                    obj_ptr
                );
	}
	else
	{
            VSLabelAdd(
                obj_ptr->name,
                xsw_color.vs_label_fg_cv,
                xsw_color.vs_label_bg_cv,
                xsw_font.console_standard,
                obj_ptr
            );
	}


	/* Redraw console panels. */
	if(object_num == net_parms.player_obj_num)
	{
	    BridgeWinDrawPanel(
		net_parms.player_obj_num, BPANEL_DETAIL_PNAME
	    );
	}
	else if(net_parms.player_obj_ptr != NULL)
	{
	    if(object_num == net_parms.player_obj_ptr->locked_on)
	    {
                BridgeWinDrawPanel(
		    object_num,
		    BPANEL_DETAIL_SNAME
		);
                BridgeWinDrawPanel(
                    object_num,
                    BPANEL_DETAIL_SEMPIRE
                );
	    }
	}


        /* Refresh the last time it was updated. */
        obj_ptr->last_updated = cur_millitime;


	return(0);
}

/*
 *	Handles a set shield visibility request on an object.
 */
int NetHandleSetShieldVis(char *arg)
{
	int object_num;
	double shield_visibility;
	xsw_object_struct *obj_ptr;


	/*
	 *   SWEXTCMD_SETSHIELDVIS format:
	 *
	 *	object_num shield_visibility
	 */
	sscanf(arg, "%i %lf",
		&object_num,
		&shield_visibility
	);


        /* See if object_num is valid. */
        if(DBIsObjectGarbage(object_num))
            return(-1);
	else
            obj_ptr = xsw_object[object_num];


	/* Set new value(s). */
	obj_ptr->shield_visibility = shield_visibility;


	/* Sanitize values. */
	if(obj_ptr->shield_visibility < 0)
	    obj_ptr->shield_visibility = 0;
	else if(obj_ptr->shield_visibility > 1)
	    obj_ptr->shield_visibility = 1;


        /* Refresh the last time it was updated. */
        obj_ptr->last_updated = cur_millitime;


        return(0);
}

/*
 *	Sets channel for object.
 */
int NetHandleSetChannel(char *arg)
{
	int object_num;
	int channel;
	xsw_object_struct *obj_ptr;


        /*
         *   SWEXTCMD_SETCHANNEL format:
	 *
         *	object_num channel
         */
        sscanf(arg, "%i %i",
                &object_num,
                &channel
        );

        if(DBIsObjectGarbage(object_num))
            return(-1);
        else
            obj_ptr = xsw_object[object_num];


        /* Set new channel. */
        obj_ptr->com_channel = channel;

        /* Refresh the last time it was updated. */
        obj_ptr->last_updated = cur_millitime;


        /* Redraw bridge console panels as needed. */
        if(object_num == net_parms.player_obj_num)
        {
            BridgeWinDrawPanel(
		net_parms.player_obj_num,
		BPANEL_DETAIL_PCOMCHANNEL
	    );
        }


        return(0);
}

/*
 *	Handles a set scores.
 */
int NetHandleSetScore(char *arg)
{
        int object_num;
        xsw_object_struct *obj_ptr;
	xsw_score_struct *score_ptr;

	xswo_credits_t credits;
	xswo_rmu_t rmu, rmu_max;
	double damage_given, damage_recieved;
	int kills;
 

        /*
         *   SWEXTCMD_SETSCORE format:
	 *
	 *    Client sends:
	 *	object_num
	 *
	 *    Server sends:
	 *	object_num
	 *	credits rmu rmu_max damage_given damage_recieved kills
	 */
        sscanf(arg,
		"%i %lf %lf %lf %lf %lf %i",

                &object_num,

                &credits,
		&rmu,
		&rmu_max,
		&damage_given,
		&damage_recieved,
		&kills
        );

        if(DBIsObjectGarbage(object_num))
            return(-1);
	else
	    obj_ptr = xsw_object[object_num];


	/* Allocate scores as needed. */
	if(UNVAllocScores(obj_ptr))
	    return(-1);

	score_ptr = obj_ptr->score;


	/* Set new score values. */
	score_ptr->credits = credits;
	score_ptr->rmu = rmu;
        score_ptr->rmu_max = rmu_max;
        score_ptr->damage_given = damage_given;
        score_ptr->damage_recieved = damage_recieved;
        score_ptr->kills = kills;


	/* Redraw windows as needed. */
	if(eco_win.map_state)
	{
	    EcoWinDraw();
	}

        /* Refresh the last time it was updated. */
        obj_ptr->last_updated = cur_millitime;


	return(0);
}

/*
 *      Handles a set engine state.
 */
int NetHandleSetEngine(char *arg)
{
        int object_num;
	int engine_state;
        xsw_object_struct *obj_ptr;


        /*
         *   SWEXTCMD_SETENGINE format:
	 *
	 *	object_num engine_state
         */
        sscanf(arg,
                "%i %i",
  
                &object_num,
  		&engine_state
        );

        if(DBIsObjectGarbage(object_num))
            return(-1);
        else
            obj_ptr = xsw_object[object_num];


	if(obj_ptr->engine_state != engine_state)
	{
	    obj_ptr->engine_state = engine_state;

	    if(obj_ptr == net_parms.player_obj_ptr)
	    {
                BridgeWinDrawPanel(
                    object_num,
                    BPANEL_DETAIL_PENGINESTATE
                );
	    }
	}


        /* Refresh the last time it was updated. */
        obj_ptr->last_updated = cur_millitime;


        return(0);
}

/*
 *	Handles a hit notify.
 */
int NetHandleNotifyHit(char *arg)
{
	int wep_obj, wep_owner_obj, tar_obj;
	double total_damage, bearing;
	double structure_damage, shield_damage;


        /*
         *   SWEXTCMD_NOTIFYHIT format:
         *
         *      wep_obj tar_obj
	 *	total_damage bearing
	 *	structure_damage shield_damage
         */
        sscanf(arg, "%i %i %lf %lf %lf %lf",
                &wep_obj,
		&tar_obj,
		&total_damage,
		&bearing,
		&structure_damage,
		&shield_damage
        );

        /* Check if wep_obj is valid. */
        if(DBIsObjectGarbage(wep_obj))
            return(0);

	/* Get weapon owner object. */
	wep_owner_obj = xsw_object[wep_obj]->owner;


	XSWDoHit(
	    wep_obj,	/* Object that hit tar_obj. */
	    tar_obj,	/* Object that was hit. */
	    wep_owner_obj,	/* Owner of src. */
	    structure_damage,
	    shield_damage,
	    bearing
	);

	return(0);
}

/*
 *	Handles a destroy notify.
 */
int NetHandleNotifyDestroy(char *arg)
{
	int reason_code;
	int destroyed_obj, destroyer_obj, destroyer_obj_owner;


        /*
         *   SWEXTCMD_NOTIFYDESTROY format:
         *
         *	reason_code
         *	destroyed_obj destroyer_obj,
	 *	destroyer_obj_owner
         */
        sscanf(arg, "%i %i %i %i",
                &reason_code,
                &destroyed_obj,
                &destroyer_obj,
                &destroyer_obj_owner
        );

	XSWDoDestroyed(
	    destroyer_obj,	/* Object that caused destruction. */
	    destroyed_obj,	/* Object that was destroyed. */
	    destroyer_obj_owner,	/* Owner of src_obj. */
	    reason_code		/* Reason code (sent from server). */
	);

	return(0);
}

/*
 *	Handles a set tractor beam lock.
 */
int NetHandleTractorBeamLock(char *arg)
{
	int sobj_num, tobj_num;
	xsw_object_struct *obj_ptr;


        /*
	 *   SWEXTCMD_TRACTORBEAMLOCK format:
         *
         *      src_obj, tar_obj
         */
        sscanf(arg, "%i %i",
                &sobj_num,
                &tobj_num
        );

        /* Check if sobj_num is valid. */
        if(DBIsObjectGarbage(sobj_num))
            return(-1);
	else
	    obj_ptr = xsw_object[sobj_num];

	/* Set tractor beam lock on object sobj_num. */
	DBObjectTractor(sobj_num, tobj_num);

        /* Refresh the last time it was updated. */
        obj_ptr->last_updated = cur_millitime;

        return(0);
}

/*
 *	Handles a hail.
 */
int NetHandleHail(char *arg)
{
        int src_obj, tar_obj;
        double bearing;
	int channel;
        xsw_object_struct *obj_ptr;
	pixel_t pix = 0;
	char text[XSW_OBJ_NAME_MAX + 256];


        /*
         *   SWEXTCMD_HAIL format:
         *
         *	src_obj, tar_obj, bearing, channel
         */
        sscanf(arg, "%i %i %lf %i",
                &src_obj,	/* Who sent the hail. */
                &tar_obj,	/* Who the hail is for. */
		&bearing,
		&channel
        );
 
        /* Check if src_obj is valid. */
        if(DBIsObjectGarbage(src_obj))
            obj_ptr = NULL;
        else
            obj_ptr = xsw_object[src_obj];


	/* Print hail message. */
	if(obj_ptr != NULL)
	{
	    if(src_obj == net_parms.player_obj_num)
	    {
		/* We sent that hail. */

		if(option.sounds >= XSW_SOUNDS_EVENTS)
		    SoundPlay(
			SOUND_CODE_HAIL_OUTGOING,
			1.0, 1.0,
			0,
			0
		    );

                sprintf(text,
                    "Hail sent on channel %.2f",
                    (double)((double)channel / 100)
                );
                pix = xsw_color.bp_standard_text;
	    }
	    else
	    {
                if(option.sounds >= XSW_SOUNDS_EVENTS)
                    SoundPlay(
                        SOUND_CODE_HAIL_INCOMING,
                        1.0, 1.0,
                        0,
                        0
                    );

	        sprintf(text,
                    "%s bearing %.0f' hailing on channel %.2f",
		    obj_ptr->name,
		    (double)RADTODEG(bearing),
		    (double)((double)channel / 100)
	        );
                pix = xsw_color.bp_bold_text;
	    }
	}
	else
        {
            sprintf(text, "Unknown hail bearing %.0f' on channel %.2f",
                RADTODEG(bearing),
                (double)((double)channel / 100)
            );
	    pix = xsw_color.bp_bold_text;
        }
	MesgAdd(text, pix);


        /* Refresh the last time it was updated. */
	if(obj_ptr != NULL)
	    obj_ptr->last_updated = cur_millitime;


        return(0);
}

/*
 *	Handles a com message.
 */
int NetHandleComMessage(char *arg)
{
	char *strptr;
	int src_obj, tar_obj;
        double bearing;
        int channel;
        xsw_object_struct *src_obj_ptr;

        char larg[CS_DATA_MAX_LEN];
        char message[CS_MESG_MAX];
	char stringa[CS_MESG_MAX + XSW_OBJ_NAME_MAX + 256];
	char stringb[CS_MESG_MAX + XSW_OBJ_NAME_MAX + 256];


        strncpy(larg, arg, CS_DATA_MAX_LEN);
        larg[CS_DATA_MAX_LEN - 1] = '\0';


        /*
         *   SWEXTCMD_COMMESSAGE format:
         *
         *      "src_obj tar_obj bearing channel;message"
         */
        strptr = strchr(larg, CS_STRING_DELIMINATOR_CHAR);
        if(strptr == NULL)
	    return(-1);

        strncpy(message, strptr + 1, CS_MESG_MAX);
        message[CS_MESG_MAX - 1] = '\0';
        *strptr = '\0';

        sscanf(larg,
                "%i %i %lf %i",

                &src_obj,       /* Object sending the message. */
                &tar_obj,       /* Object to recieve or -1 for any. */
                &bearing,
                &channel
        );


        /* Need valid source object pointer. */
        if(DBIsObjectGarbage(src_obj))
            src_obj_ptr = NULL;
        else
            src_obj_ptr = xsw_object[src_obj];

	if(src_obj_ptr != NULL)
	{
	    if(src_obj_ptr == net_parms.player_obj_ptr)
                sprintf(stringa,
                    "<%s> -> %.2f: \"%s\"",
                    src_obj_ptr->name,
                    (double)((double)channel / 100),
                    message
                );
	    else
	        sprintf(stringa,
		    "<%s> %.0f'@%.2f: \"%s\"",
		    src_obj_ptr->name,
		    RADTODEG(bearing),
		    (double)((double)channel / 100),
		    message
	        );
	}
	else
	{
            sprintf(stringa,
		"*Unknown* %.0f'@%.2f: \"%s\"",
                RADTODEG(bearing),
		(double)((double)channel / 100),
                message
            );
	}
	MesgAdd(stringa, xsw_color.standard_text);


	/* Log com message. */
	sprintf(stringb, "%s\n", stringa);
	if(option.log_net)
	    LogAppendLineFormatted(fname.log, stringb);


	return(0);
}

/*
 *	Handles a weapon disarmed notify.
 */
int NetHandleWeaponDisarm(char *arg)
{
	int src_obj, tar_obj;


	/*
	 *   SWEXTCMD_WEPDISARM format:
	 *
	 *	src_obj tar_obj
	 */
	sscanf(arg,
		"%i %i",

                &src_obj,
                &tar_obj
	);
	/* Source object needs to be the player object otherwise we
	 * ignore it.
	 */
	if(src_obj != net_parms.player_obj_num)
	    return(0);

	/* Print disarm notify. */
	if(!DBIsObjectGarbage(tar_obj))
	{
	    xsw_object_struct *wep_obj_ptr;
	    char text[XSW_OBJ_NAME_MAX + 256];

	    wep_obj_ptr = xsw_object[tar_obj];


            if(option.show_formal_label == 2)
		sprintf(text, "%s: Disarmed", DBGetFormalNameStr(tar_obj));
	    else if(option.show_formal_label == 1)
                sprintf(text, "%s: Disarmed", DBGetFormalNameStr(tar_obj));
	    else
		sprintf(text, "%s: Disarmed", wep_obj_ptr->name);

	    MesgAdd(text, xsw_color.bp_standard_text);
	}

	return(0);
}

/*
 *	Handles set economy values.
 */
int NetHandleEcoSetValues(char *arg)
{
        int object_num;
	eco_flags_t flags;
	double tax_general, tax_friend, tax_hostile;

	xsw_ecodata_struct *eco_ptr;
        xsw_object_struct *obj_ptr; 


        /*
         *   SWEXTCMD_ECO_SETVALUES format:
         *
         *      object_num flags tax_general tax_friend
	 *	tax_hostile
         */
	sscanf(arg,
		"%i %ld %lf %lf %lf",

		&object_num,
		&flags,
		&tax_general,
		&tax_friend,
		&tax_hostile
	);

	if(DBIsObjectGarbage(object_num))
	    return(-1);
	else
	    obj_ptr = xsw_object[object_num];


        /* Allocate scores and eco (as needed) on object. */
        if(UNVAllocScores(obj_ptr))
            return(-1);
        if(UNVAllocEco(obj_ptr))
            return(-1);

	/* Get pointer to eco structure, make sure it's allocated. */
	eco_ptr = obj_ptr->eco;
	if(eco_ptr == NULL)
	    return(-1);


	/* Set values. */
	eco_ptr->flags = flags;
	eco_ptr->tax_general = tax_general;
	eco_ptr->tax_friend = tax_friend;
	eco_ptr->tax_hostile = tax_hostile;


	return(0);
}

/*
 *	Sets economy product values.
 */
int NetHandleEcoSetProductValues(char *arg)
{
	int object_num;
	char *strptr;
	xsw_object_struct *obj_ptr;
	xsw_ecoproduct_struct product;
        char larg[CS_DATA_MAX_LEN];


        strncpy(larg, arg, CS_DATA_MAX_LEN);
        larg[CS_DATA_MAX_LEN - 1] = '\0';   

        /*
         *   SWEXTCMD_ECO_SETPRODUCTVALUES format:
         *
         *	object_num sell_price buy_price
	 *	product_amount product_max;product_name
         */
	strptr = strchr(larg, CS_STRING_DELIMINATOR_CHAR);
	if(strptr == NULL)
	    return(-1);

	strncpy(product.name, strptr + 1, ECO_PRODUCT_NAME_MAX);
	product.name[ECO_PRODUCT_NAME_MAX - 1] = '\0';

	*strptr = '\0';

        sscanf(larg,
		"%i %lf %lf %lf %lf",

                &object_num,
		&product.sell_price,
		&product.buy_price,
		&product.amount,
		&product.amount_max
        );

        /* Check if object_num is valid. */
        if(DBIsObjectGarbage(object_num))
            return(0);
        else
	    obj_ptr = xsw_object[object_num];

	/* Allocate scores and eco (as needed) on object. */
	if(UNVAllocScores(obj_ptr))
	    return(-1);
	if(UNVAllocEco(obj_ptr))
	    return(-1);

	/* Allocate product on object (as needed) and update values. */
	if(DBCreateObjectEconomyProduct(
	    object_num,
	    product.name,
	    product.sell_price,
	    product.buy_price,
	    product.amount,
	    product.amount_max
	))
	    return(-1);


	/*   Recieving a SWEXTCMD_ECO_SETPRODUCTVALUES implies
	 *   request to map eco window.
	 */
	EcoWinDoAddInventory(
            net_parms.player_obj_num,
	    object_num,
            product
        );

	if(object_num != net_parms.player_obj_num)
	{
	    if(!eco_win.map_state)
	        EcoWinMap();
	}


	return(0);
}


/*
 *	Handles a add object to star chart.
 */
int NetHandleStarChartAddObject(char *arg)
{
	int object_num;
	int type;
	int isref_num;
	long size;
	long sect_x, sect_y, sect_z;
	double x, y, z;
	double heading, pitch, bank;


        /*
         *   SWEXTCMD_STARCHART_ADD_OBJECT format:
         *
	 *      object_num
	 *      type isref_num size
	 *      sect_x sect_y sect_z
	 *      x y z
	 *      heading pitch bank
         */
        sscanf(arg,
"%i\
 %i %i %ld\
 %ld %ld %ld\
 %lf %lf %lf\
 %lf %lf %lf",

                &object_num,
                &type, &isref_num, &size,
		&sect_x, &sect_y, &sect_z,
		&x, &y, &z,
		&heading, &pitch, &bank
	);

	SChtAddObject(
	    &starchart_win,
	    object_num,
	    type, isref_num, size,
	    sect_x, sect_y, sect_z,
	    x, y, z,
	    heading, pitch, bank
	);

	return(0);
}

/*
 *	Handles a set object name to star chart.
 */
int NetHandleStarChartSetObjectName(char *arg)
{
	char *strptr;
	char name[XSW_OBJ_NAME_MAX];


	/* Assume `last' object in star chart. */

	strptr = strchr(arg, CS_STRING_DELIMINATOR_CHAR);
	if(strptr == NULL)
	    return(-1);

	strncpy(name, strptr + 1, XSW_OBJ_NAME_MAX);
	name[XSW_OBJ_NAME_MAX - 1] = '\0';

	SChtSetObjectName(&starchart_win, name);

	return(0);
}

/*
 *	Handles a set object empire on star chart.
 */
int NetHandleStarChartSetObjectEmpire(char *arg)
{
        char *strptr;
        char empire[XSW_OBJ_EMPIRE_MAX];


        /* Assume `last' object in star chart. */

        strptr = strchr(arg, CS_STRING_DELIMINATOR_CHAR);
        if(strptr == NULL)
            return(-1);

        strncpy(empire, strptr + 1, XSW_OBJ_EMPIRE_MAX);
        empire[XSW_OBJ_EMPIRE_MAX - 1] = '\0';

        SChtSetObjectEmpire(&starchart_win, empire);

	return(0);
}

/*
 *	Handles recycle object on star chart.
 */
int NetHandleStarChartRecycleObject(char *arg)
{
	int object_num;


        /*
         *   SWEXTCMD_STARCHART_RECYCLE_OBJECT format:
         *
         *      object_num
         */
        sscanf(arg, "%i",
                &object_num
        );

	SChtRecycleObject(&starchart_win, object_num);

	return(0);
}

/*
 *	Handles a play sound request from server.
 */
int NetHandlePlaySound(char *arg)
{
	int status, sound_code;
	double left_volume, right_volume;


	/* Check sound play level option. */
	if(option.sounds <= XSW_SOUNDS_NONE)
	    return(0);

	/*
	 *   CS_CODE_PLAYSOUND format:
	 *
	 *	sound_code left_volume right_volume
	 */
	sscanf(arg, "%i %lf %lf",
		&sound_code,
		&left_volume,
		&right_volume
	);

	status = SoundPlay(
	    sound_code,
	    left_volume,
	    right_volume,
	    0,		/* Effects. */
	    0		/* Priority. */
	);

	return(status);
}

/*
 *	Handles a set object's weapon value.
 */
int NetHandleSetWeaponValues(char *arg)
{
	char changed_name = 0;
	char changed_amount = 0;
	char *strptr;

	int object_num;
	int weapon_num;
	int ocs_num;

	int ocs_code;
	int emission_type;
	int amount;
	int max;

	double power;
	long range;
	double create_power;

	long delay;
	long last_used;
	int fire_sound_code;
/*	xswo_weapon_flags_t flags; */
	unsigned long flags;

	xsw_object_struct *obj_ptr;
	xsw_weapons_struct *wep_ptr;


        /*
         *   NET_CMD_SETWEAPONVAL format:
         *
         *      object_num, weapon_num,
         *      ocs_code, emission_type, amount, max,
         *      power, range, create_power
         *      delay, last_used, fire_sound_code flags
         */
        sscanf(arg,
"%i %i\
 %i %i %i %i\
 %lf %ld %lf\
 %ld %ld %i %lu",
                &object_num,
                &weapon_num,

		&ocs_code,
		&emission_type,
		&amount,
		&max,

		&power,
		&range,
		&create_power,
                
		&delay,
		&last_used,
		&fire_sound_code,
		&flags
        );


        /* Make sure object_num is valid. */
	if(DBIsObjectGarbage(object_num))
	    return(-1);
	else
	    obj_ptr = xsw_object[object_num];


	/* Make sure weapon_num is valid. */
	if((weapon_num < 0) || (weapon_num >= MAX_WEAPONS))
	    return(-1);


	/* Allocate more weapons as needed. */
	if(weapon_num >= obj_ptr->total_weapons)
	{
	    if(UNVAllocObjectWeapons(obj_ptr, weapon_num + 1))
		return(-1);

	    changed_name = 1;
	    changed_amount = 1;
	}


	/* Get pointer to weapon. */
	wep_ptr = obj_ptr->weapons[weapon_num];

	/* Set weapon values. */
	wep_ptr->flags = (xswo_weapon_flags_t)flags;
	wep_ptr->ocs_code = ocs_code;
        wep_ptr->emission_type = emission_type;

	if(amount != wep_ptr->amount)
	{
	    changed_amount = 1;
            wep_ptr->amount = amount;
	}
        wep_ptr->max = max;

        wep_ptr->power = power;
        wep_ptr->range = range;
        wep_ptr->create_power = create_power;

        wep_ptr->delay = delay;


	/* Get name for weapon type by matching type with OCS. */
	ocs_num = OCSGetByCode(ocs_code);
	if(ocs_num > -1)
	{
	    strptr = ocsn[ocs_num]->name;

	    if(strcmp(wep_ptr->name, strptr))
	    {
	        strncpy(
		    wep_ptr->name,
		    strptr,
		    XSW_OBJ_NAME_MAX
                );
	        wep_ptr->name[XSW_OBJ_NAME_MAX - 1] = '\0';

		changed_name = 1;
	    }
	}
	else
	{
            strptr = "Unknown";

            if(strcmp(wep_ptr->name, strptr))
            {
                strncpy(
                    wep_ptr->name,   
                    strptr,
                    XSW_OBJ_NAME_MAX
                );
                wep_ptr->name[XSW_OBJ_NAME_MAX - 1] = '\0';

                changed_name = 1;
	    }
	}

	/*
	 *   Last used is handled locally since cur_millitime
	 *   on client and server may differ. last_used will be
	 *   set by the client's EventHandleWeaponFire()
	 *   function.
	 */
/*
	wep_ptr->last_used = last_used;
*/


        /* Refresh the last time object was updated. */
        obj_ptr->last_updated = cur_millitime;


	/* Redraw displays. */
	if(object_num == net_parms.player_obj_num)
	{
            BridgeWinDrawPanel(
		net_parms.player_obj_num,
		BPANEL_DETAIL_PWEAPONS
	    );

	    if(changed_name || changed_amount)
	    {
		if(weapon_num == obj_ptr->selected_weapon)
		{
                    /* Recreate selected weapon stats label on viewscreen. */
                    VSDrawUpdateWeaponLabel(
                        &bridge_win.vs_weapon_image,
                        bridge_win.vs_weapon_buf
                    );
		}
	    } 
	}

	return(0);
}

/*
 *	Handle `extended' net commands.
 *
 *	Function is intented to be called from NetHandleRecv().
 */
int NetHandleExtCmd(char *arg)
{
	char *strptr;
        char ext_arg[CS_DATA_MAX_LEN];
        int ext_cmd;


	/* Get command and argument. */

	ext_cmd = StringGetNetCommand(arg);
	if(ext_cmd < 0)
	    return(-1);

	strptr = StringGetNetArgument(arg);
	if(strptr == NULL)
	    return(-1);
	strncpy(ext_arg, strptr, CS_DATA_MAX_LEN);
	ext_arg[CS_DATA_MAX_LEN - 1] = '\0';



	/* ****************************************************** */
	/* Handle external ext_cmd. */

	switch(ext_cmd)
	{
	  /* Systems and enviroment. */
         case SWEXTCMD_SETOCSN:
            if(debug.level == DEBUG_LEVEL_NETWORK)
                fprintf(stderr, "Client: SWEXTCMD_SETOCSN: %s\n", ext_arg);
            NetHandleSetOCSN(ext_arg);
            break;

	  case SWEXTCMD_SETUNITS:
	    if(debug.level == DEBUG_LEVEL_NETWORK)
                fprintf(stderr, "Client: SWEXTCMD_SETUNITS: %s\n", ext_arg);
            NetHandleSetUnits(ext_arg);
            break;


	  /* Standards. */
	  case SWEXTCMD_STDOBJVALS:
            if(debug.level == DEBUG_LEVEL_NETWORK)
                fprintf(stderr, "Client: SWEXTCMD_STDOBJVALS: %s\n", ext_arg);
	    NetHandleSetObjectValues(ext_arg);
	    break;

	  case SWEXTCMD_STDOBJMAXS:
            if(debug.level == DEBUG_LEVEL_NETWORK)
                fprintf(stderr, "Client: SWEXTCMD_STDOBJMAXS: %s\n", ext_arg);
	    NetHandleSetObjectMaximums(ext_arg);
	    break;

	  case SWEXTCMD_STDWEPVALS:
            if(debug.level == DEBUG_LEVEL_NETWORK)
                fprintf(stderr, "Client: SWEXTCMD_STDWEPVALS: %s\n", ext_arg);
            NetHandleSetWeaponValues(ext_arg);
	    break;


	  /* Setting. */
          case SWEXTCMD_SETOBJNAME:
            if(debug.level == DEBUG_LEVEL_NETWORK)
                fprintf(stderr, "Client: SWEXTCMD_SETOBJNAME: %s\n", ext_arg);
	    NetHandleSetName(ext_arg);
	    break;

	  case SWEXTCMD_SETOBJSECT:
            if(debug.level == DEBUG_LEVEL_NETWORK)
                fprintf(stderr, "Client: SWEXTCMD_SETOBJSECT: %s\n", ext_arg);
	    NetHandleSetObjSect(ext_arg);
	    break;

          case SWEXTCMD_SETFOBJSECT:
            if(debug.level == DEBUG_LEVEL_NETWORK)
                fprintf(stderr, "Client: SWEXTCMD_SETFOBJSECT: %s\n", ext_arg);
            NetHandleSetFObjSect(ext_arg);
            break;

	  case SWEXTCMD_SETWEAPON:
            if(debug.level == DEBUG_LEVEL_NETWORK)
                fprintf(stderr, "Client: SWEXTCMD_SETWEAPON: %s\n", ext_arg);
	    /* Client does not handle this command. */
	    break;

          case SWEXTCMD_SETINTERCEPT:
            if(debug.level == DEBUG_LEVEL_NETWORK)
                fprintf(stderr, "Client: SWEXTCMD_SETINTERCEPT: %s\n", ext_arg);
            /* Client does not handle this command. */
            break;

          case SWEXTCMD_SETWEPLOCK:
            if(debug.level == DEBUG_LEVEL_NETWORK)
                fprintf(stderr, "Client: SWEXTCMD_SETWEPLOCK: %s\n", ext_arg);
            /* Client does not handle this command. */
            break;

          case SWEXTCMD_SETSHIELDS:
            if(debug.level == DEBUG_LEVEL_NETWORK)
                fprintf(stderr, "Client: SWEXTCMD_SETSHIELDS: %s\n", ext_arg);
            /* Client does not handle this command. */
            break;

          case SWEXTCMD_SETDMGCTL:
            if(debug.level == DEBUG_LEVEL_NETWORK)
                fprintf(stderr, "Client: SWEXTCMD_SETDMGCTL: %s\n", ext_arg);
            /* Client does not handle this command. */
            break;

          case SWEXTCMD_SETCLOAK:
            if(debug.level == DEBUG_LEVEL_NETWORK)
                fprintf(stderr, "Client: SWEXTCMD_SETCLOAK: %s\n", ext_arg);
            /* Client does not handle this command. */
            break;

	  case SWEXTCMD_SETSHIELDVIS:
            if(debug.level == DEBUG_LEVEL_NETWORK)
                fprintf(stderr, "Client: SWEXTCMD_SETSHIELDVIS: %s\n", ext_arg);
	    NetHandleSetShieldVis(ext_arg);
	    break;

	  case SWEXTCMD_SETLIGHTING:
            if(debug.level == DEBUG_LEVEL_NETWORK)
                fprintf(stderr, "Client: SWEXTCMD_SETLIGHTING: %s\n", ext_arg);
            /* Client does not handle this command. */
            break;

          case SWEXTCMD_SETCHANNEL:
            if(debug.level == DEBUG_LEVEL_NETWORK)
                fprintf(stderr, "Client: SWEXTCMD_SETCHANNEL: %s\n", ext_arg);
	    NetHandleSetChannel(ext_arg);
            break;

	  case SWEXTCMD_SETSCORE:
            if(debug.level == DEBUG_LEVEL_NETWORK)
                fprintf(stderr, "Client: SWEXTCMD_SETSCORE: %s\n", ext_arg);
            NetHandleSetScore(ext_arg);
	    break;

          case SWEXTCMD_SETENGINE:
            if(debug.level == DEBUG_LEVEL_NETWORK)
                fprintf(stderr, "Client: SWEXTCMD_SETENGINE: %s\n", ext_arg);
            NetHandleSetEngine(ext_arg);
            break;


	  /* Request. */
	  case SWEXTCMD_REQNAME:
            if(debug.level == DEBUG_LEVEL_NETWORK)
                fprintf(stderr, "Client: SWEXTCMD_REQNAME: %s\n", ext_arg);
	    /* Client does not handle this command. */
            break;

	  case SWEXTCMD_REQSECT:
            if(debug.level == DEBUG_LEVEL_NETWORK)
                fprintf(stderr, "Client: SWEXTCMD_REQSECT: %s\n", ext_arg);
	    /* Client does not handle this command. */
            break;


	  /* Hit and destroy notifies. */
	  case SWEXTCMD_NOTIFYHIT:
            if(debug.level == DEBUG_LEVEL_NETWORK)
                fprintf(stderr, "Client: SWEXTCMD_NOTIFYHIT: %s\n", ext_arg);
	    NetHandleNotifyHit(ext_arg);
	    break;

	  case SWEXTCMD_NOTIFYDESTROY:
            if(debug.level == DEBUG_LEVEL_NETWORK)
                fprintf(stderr, "Client: SWEXTCMD_NOTIFYDESTROY: %s\n", ext_arg);
            NetHandleNotifyDestroy(ext_arg);
            break;


	  /* Actions. */
	  case SWEXTCMD_FIREWEAPON:
            if(debug.level == DEBUG_LEVEL_NETWORK)
                fprintf(stderr, "Client: SWEXTCMD_FIREWEAPON: %s\n", ext_arg);
            /* Client does not handle this command. */
            break;

	  case SWEXTCMD_TRACTORBEAMLOCK:
            if(debug.level == DEBUG_LEVEL_NETWORK)
                fprintf(stderr, "Client: SWEXTCMD_TRACTORBEAMLOCK: %s\n", ext_arg);
	    NetHandleTractorBeamLock(ext_arg);
	    break;

	  case SWEXTCMD_HAIL:
            if(debug.level == DEBUG_LEVEL_NETWORK)
                fprintf(stderr, "Client: SWEXTCMD_HAIL: %s\n", ext_arg);
	    NetHandleHail(ext_arg);
            break;

          case SWEXTCMD_COMMESSAGE:
            if(debug.level == DEBUG_LEVEL_NETWORK)
                fprintf(stderr, "Client: SWEXTCMD_COMMESSAGE: %s\n", ext_arg);
	    NetHandleComMessage(ext_arg);
            break;

          case SWEXTCMD_WORMHOLEENTER:
            if(debug.level == DEBUG_LEVEL_NETWORK)
                fprintf(stderr, "Client: SWEXTCMD_WORMHOLEENTER: %s\n", ext_arg);
	    /* Client does not handle this command. */
            break;

          case SWEXTCMD_ELINKENTER:
            if(debug.level == DEBUG_LEVEL_NETWORK)
                fprintf(stderr, "Client: SWEXTCMD_ELINKENTER: %s\n", ext_arg);
	    /* Client does not handle this command. */
            break;

	  case SWEXTCMD_WEPDISARM:
	    if(debug.level == DEBUG_LEVEL_NETWORK)
                fprintf(stderr, "Client: SWEXTCMD_WEPDISARM: %s\n", ext_arg);
	    NetHandleWeaponDisarm(ext_arg);
            break;


	  /* Economy. */
	  case SWEXTCMD_ECO_REQVALUES:
	    /* Client does not handle this. */
	    break;

	  case SWEXTCMD_ECO_SETVALUES:
	    NetHandleEcoSetValues(ext_arg);
	    break;

	  case SWEXTCMD_ECO_SETPRODUCTVALUES:
	    NetHandleEcoSetProductValues(ext_arg);
	    break;

	  case SWEXTCMD_ECO_BUY:
            /* Client does not handle this. */
	    break;

          case SWEXTCMD_ECO_SELL:
            /* Client does not handle this. */
            break;


	  /* Star chart. */
	  case SWEXTCMD_STARCHART_ADD_OBJECT:
	    NetHandleStarChartAddObject(ext_arg);
	    break;

	  case SWEXTCMD_STARCHART_SET_OBJECT_NAME:
	    NetHandleStarChartSetObjectName(ext_arg);
	    break;

	  case SWEXTCMD_STARCHART_SET_OBJECT_EMPIRE:
	    NetHandleStarChartSetObjectEmpire(ext_arg);
	    break;

	  case SWEXTCMD_STARCHART_SET_OBJECT_RECYCLE:
	    NetHandleStarChartRecycleObject(ext_arg);
	    break;



	  default:
            if(debug.level == DEBUG_LEVEL_NETWORK)
                fprintf(stderr, "Client: SWEXTCMD_???: %s\n", ext_arg);
	    break;
	}

	return(0);
}

/*
 *	Front end to handle segment(s) of incoming network data
 *	from a connection.
 */
int NetHandleRecv()
{
        int bytes_read;
        struct timeval timeout;
        fd_set readfds;

        char recvbuf[CS_DATA_MAX_BACKLOG];
	int recvbuf_cnt;
	char *recvbuf_ptr;

        char workbuf[CS_DATA_MAX_LEN];
	int workbuf_cnt;
	char *workbuf_ptr;

	char arg[CS_DATA_MAX_LEN];
	int command;

	char text[256];


	/* Is socket valid and contains data to be read? */
        timeout.tv_sec = 0;
        timeout.tv_usec = 0;
	FD_ZERO(&readfds);
	FD_SET(net_parms.socket, &readfds);
	if(
	    select(
		net_parms.socket + 1,
		&readfds, NULL, NULL,
		&timeout
	    ) < 1
	)
	    return(0);


        /* Recieve incoming data. */
	if(net_parms.co_data_len < CS_DATA_MAX_BACKLOG)
	{
	    /* Sanitize net_parms.co_data_len. */
            if(net_parms.co_data_len > CS_DATA_MAX_LEN)
		net_parms.co_data_len = CS_DATA_MAX_LEN;
	    else if(net_parms.co_data_len < 0)
		net_parms.co_data_len = 0;

	    if(net_parms.co_data_len > 0)
	        memcpy(
		    recvbuf,
		    net_parms.co_data,
		    net_parms.co_data_len
	        );
	}
        else
        {
            memset(net_parms.co_data, 0x00, CS_DATA_MAX_LEN);
            net_parms.co_data_len = 0;
	}

        bytes_read = recv(
	    net_parms.socket,
	    &recvbuf[net_parms.co_data_len],
	    CS_DATA_MAX_BACKLOG - net_parms.co_data_len,
	    0
	);
	if(bytes_read == 0)
	{
            /*   When polling of socket says there are bytes to be
             *   read and recv() returns 0, it implies that the
             *   socket has died.
             */

	    MesgAdd(
		"Connection to universe has been brokened.",
		xsw_color.bp_bold_text
	    );

	    /* Force close connection. */
	    net_parms.socket = -1;
	    XSWDoDisconnect();

	    return(-1);
	}

	/* Recieve error? */
	if(bytes_read < 0)
	{
	    /* Handle error. */
            switch(errno)
	    {
	      /* Invalid descriptor. */
	      case EBADF:
                sprintf(text,
                    "NetHandleRecv(): recv(): Invalid descriptor `%i'.",
                    net_parms.socket
                );
                MesgAdd(text, xsw_color.bp_bold_text);

                /* Force close connection. */
                net_parms.socket = -1;
                XSWDoDisconnect();

                return(-1);
                break;

              /* Connection oriented descriptor is not connected. */
	      case ENOTCONN:
                sprintf(text,
 "NetHandleRecv(): recv(): Connection oriented descriptor `%i' not connected.",
                    net_parms.socket
                );
                MesgAdd(text, xsw_color.bp_bold_text);

                /* Force close connection. */
                net_parms.socket = -1;
                XSWDoDisconnect();

                return(-1);
		break;

              /* Descriptor is not really a socket. */
              case ENOTSOCK:
                sprintf(text,
 "NetHandleRecv(): recv(): Descriptor `%i' is not really a socket.",
                    net_parms.socket
                );
                MesgAdd(text, xsw_color.bp_bold_text);

                /* Force close connection. */
                net_parms.socket = -1;
                XSWDoDisconnect();

                return(-1);
                break;

	      /* Requested operation would block. */
              case EWOULDBLOCK:
		/* Non-fatal error. */
		return(0);
                break;

	      /* Interupted by signal. */
              case EINTR:
                MesgAdd(
                    "NetHandleRecv(): recv(): Interupted by signal.",
                    xsw_color.bp_bold_text
                );
                return(0);
                break;

	      /* Broken pipe. */
	      case EPIPE:
                MesgAdd(
                    "Broken pipe.",
                    xsw_color.bp_bold_text
                );

                /* Force close connection. */
                net_parms.socket = -1;
                XSWDoDisconnect();

                return(-1);
                break;

	      /* Recieve buffer pointer out of address space. */
              case EFAULT:
                MesgAdd(
                    "Recieve buffer pointer out of address space.",
                    xsw_color.bp_bold_text
                );

                /* Force close connection. */
                net_parms.socket = -1;
                XSWDoDisconnect();

                return(-1);
                break;

	      /* Connection terminated by peer. */
              case ECONNRESET:
                MesgAdd(
                    "Connection terminated by peer.",
                    xsw_color.bp_bold_text
                );

                /* Force close connection. */
                net_parms.socket = -1;
                XSWDoDisconnect();

                return(-1);
                break;

	      /* Other error. */
              default:
                sprintf(text,
 "NetHandleRecv(): recv(): Unknown error `%i' on descriptor `%i'.",
                    errno, net_parms.socket
                );
                MesgAdd(text, xsw_color.bp_bold_text);

		return(-1);
	        break;
	    }
	}

        /* Increment recieved bytes per interval. */
        loadstat.rx_interval += bytes_read;


	/*   Must add net_parms.co_data_len to bytes_read
	 *   to reflect actual content size of recvbuf.
	 */
	bytes_read += net_parms.co_data_len;


	/* bytes_read cannot be greater than CS_DATA_MAX_BACKLOG. */
	if(bytes_read > CS_DATA_MAX_BACKLOG)
	    bytes_read = CS_DATA_MAX_BACKLOG;


	/* Set null character at the end of recvbuf just to be safe. */
	recvbuf[CS_DATA_MAX_BACKLOG - 1] = '\0';


if(debug.level == DEBUG_LEVEL_NETWORK)
    fprintf(stderr, "Client recieved: %i bytes.\n", bytes_read);




	/* ************************************************************ */
	/* Begin parsing recieve buffer. */

	/* Reset recieve buffer count and pointer. */
        recvbuf_cnt = 0;
	recvbuf_ptr = recvbuf;

        while(recvbuf_cnt < bytes_read)
        {
            /* Reset work buffer count and pointer. */
            workbuf_cnt = 0;
	    workbuf_ptr = workbuf;

            /* Begin fetching data from recvbuf to workbuf. */
            while(1)
            {
                /* Work buffer overflowed? */
                if(workbuf_cnt >= CS_DATA_MAX_LEN)
                {
                    /* Increment recvbuf_cnt to next delimiter char. */
                    while(recvbuf_cnt < bytes_read)
                    {
                        if((*recvbuf_ptr == '\n') ||
                           (*recvbuf_ptr == '\r') ||
                           (*recvbuf_ptr == '\0')
                        )
                        {
                            recvbuf_cnt++;
			    recvbuf_ptr++;
                            break;
                        }
                        recvbuf_cnt++;
                        recvbuf_ptr++;
                    }
		    /*   Null terminating character for workbuf will
                     *   be added farther below.
		     */
                    break;
                }

                /* End of recieve buffer data reached? */
                else if(recvbuf_cnt >= bytes_read)
                {
                    *workbuf_ptr = '\0';

		    /*   Copy workbuf_cnt to net_parms.co_data for
		     *   processing in the next call to this function.
		     */
		    net_parms.co_data_len = MIN(
			workbuf_cnt,
			CS_DATA_MAX_LEN
		    );

		    if(net_parms.co_data_len > 0)
		        memcpy(
			    net_parms.co_data,
                            workbuf,
			    net_parms.co_data_len
		        );

		    /*   Return, don't continue. This is so that the
		     *   the carry over buffer is not reset and its
		     *   values just set above get carryed over to
		     *   the next call to this function.
		     */
		    return(0);

                    break;
                }

		/* Deliminator in recieve buffer encountered? */
                else if((*recvbuf_ptr == '\n') ||
		        (*recvbuf_ptr == '\r') ||
                        (*recvbuf_ptr == '\0')
		)
                {
                    *workbuf_ptr = '\0';

                    recvbuf_cnt++;
                    recvbuf_ptr++;

                    break;
                }

		/* Copy data from recieve buffer to work buffer normally. */
		else
                {
		    *workbuf_ptr = *recvbuf_ptr;

                    recvbuf_cnt++;
                    recvbuf_ptr++;

                    workbuf_cnt++;
                    workbuf_ptr++;
		}
            }

            /* Skip if workbuf is empty. */
            if(*workbuf == '\0')
                continue;

            /* Set null terminating character for workbuf. */
            workbuf[CS_DATA_MAX_LEN - 1] = '\0';


	    /* Get command command. */
	    command = StringGetNetCommand(workbuf);
	    if(command < 0)
		continue;

	    /* Get argument arg. */
	    strncpy(arg, StringGetNetArgument(workbuf), CS_DATA_MAX_LEN);
	    arg[CS_DATA_MAX_LEN - 1] = '\0';


            /* See which command to perform. */
            switch(command)
	    {
	      case CS_CODE_LOGIN:
                if(debug.level == DEBUG_LEVEL_NETWORK)
                    fprintf(stderr, "Client: CS_CODE_LOGIN: %s\n", arg);
		NetHandleLogin(arg);
		break;

              case CS_CODE_LOGOUT:
                if(debug.level == DEBUG_LEVEL_NETWORK)  
                    fprintf(stderr, "Client: CS_CODE_LOGOUT: %s\n", arg);
                NetHandleDisconnect(arg);
                break;

              case CS_CODE_WHOAMI:
                if(debug.level == DEBUG_LEVEL_NETWORK)
                    fprintf(stderr, "Client: CS_CODE_WHOAMI: %s\n", arg);
		NetHandleWhoAmI(arg);
                    break;

	      case CS_CODE_REFRESH:
                if(debug.level == DEBUG_LEVEL_NETWORK)
                    fprintf(stderr, "Client: CS_CODE_REFRESH: %s\n", arg);
                /* Client dosen't handle this. */
		break;

	      case CS_CODE_INTERVAL:
                if(debug.level == DEBUG_LEVEL_NETWORK)
                    fprintf(stderr, "Client: CS_CODE_INTERVAL: %s\n", arg);
		/* Client dosen't handle this. */
                break;

              case CS_CODE_IMAGESET:
		if(debug.level == DEBUG_LEVEL_NETWORK)
                    fprintf(stderr, "Client: CS_CODE_IMAGESET: %s\n", arg);
		NetHandleSetImageSet(arg);
                break;

              case CS_CODE_SOUNDSET:
                if(debug.level == DEBUG_LEVEL_NETWORK)
                    fprintf(stderr, "Client: CS_CODE_SOUNDSET: %s\n", arg);
		NetHandleSetSoundSet(arg);
                break;

	      case CS_CODE_FORWARD:
                if(debug.level == DEBUG_LEVEL_NETWORK)
                    fprintf(stderr, "Client: CS_CODE_FORWARD: %s\n", arg);
                NetHandleSetForward(arg);
                break;

              case CS_CODE_LIVEMESSAGE:
                if(debug.level == DEBUG_LEVEL_NETWORK)
                    fprintf(stderr, "Client: CS_CODE_LIVEMESSAGE: %s\n", arg);
                NetHandleLiveMesg(arg);
                break;

              case CS_CODE_SYSMESSAGE:
                if(debug.level == DEBUG_LEVEL_NETWORK)
                    fprintf(stderr, "Client: CS_CODE_SYSMESSAGE: %s\n", arg);
		NetHandleSysMesg(arg);
                break;

              case CS_CODE_LITERALCMD:
		/* Client dosen't handle this. */
                if(debug.level == DEBUG_LEVEL_NETWORK)
                    fprintf(stderr, "Client: CS_CODE_LITERALCMD: %s\n", arg);
		break;


              case CS_CODE_PLAYSOUND:
                if(debug.level == DEBUG_LEVEL_NETWORK)
                    fprintf(stderr, "Client: CS_CODE_PLAYSOUND: %s\n", arg);
                NetHandlePlaySound(arg);
                break;


	      case CS_CODE_CREATEOBJ:
                if(debug.level == DEBUG_LEVEL_NETWORK)
                    fprintf(stderr, "Client: CS_CODE_CREATEOBJ: %s\n", arg);
                NetHandleCreateObject(arg);
		break;

	      case CS_CODE_RECYCLEOBJ:
                if(debug.level == DEBUG_LEVEL_NETWORK)
                    fprintf(stderr, "Client: CS_CODE_RECYCLEOBJ: %s\n", arg);
                NetHandleRecycleObject(arg);
                break;

	      case CS_CODE_POSEOBJ:
                if(debug.level == DEBUG_LEVEL_NETWORK)
                    fprintf(stderr, "Client: CS_CODE_POSEOBJ: %s\n", arg);
		NetHandleSetPoseObj(arg);
                break;

              case CS_CODE_FORCEPOSEOBJ:
                if(debug.level == DEBUG_LEVEL_NETWORK)
                    fprintf(stderr, "Client: CS_CODE_FORCEPOSEOBJ: %s\n", arg);
		NetHandleSetFPoseObj(arg);
		break;


	      case CS_CODE_EXT:
		NetHandleExtCmd(arg);
		break;

              default:
                if(debug.level == DEBUG_LEVEL_NETWORK)
                    fprintf(stderr, "Client: ???: %s\n", arg);

                /* Print unsupported? */
		if(option.show_net_errors)
		    MesgAdd(arg, xsw_color.standard_text);
		break;
	    }
	}


	/*   Reset net_parms.co_data since parsing was complete
	 *   and successful.
	 */
	net_parms.co_data_len = 0;


        return(0);
}
