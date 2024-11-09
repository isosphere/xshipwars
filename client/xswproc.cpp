/*
                              XSW Procedures

	Functions:

	int XSWDoConnect(const char *url)
        int XSWDoConnectLast(void)
        void XSWDoDisconnect(void)
        void XSWDoRefresh(void)

	void XSWDoChangeSector(int object_num)

        void XSWDoHit(
		int src_obj, int tar_obj, int owner_obj,
		double structure_damage, double shield_damage,
		double bearing
	)
        void XSWDoDestroyed(
		int src_obj, int tar_obj, int owner_obj,
		int reason
	)

	---

	Do not confuse these `events' with GUI or Sound events.

	These events are to handle things that happen in the universe,
	such as the destruction of a vessel or a weapon hitting another
	object.

 */

#include <sys/types.h>
#include <sys/stat.h>

#include "../include/disk.h"
#include "../include/swsoundcodes.h"

#include "../include/unvmath.h"
#include "../include/unvutil.h"


#include "univlist.h"
#include "starchartwin.h"
#include "xsw.h"
#include "net.h"



/*
 *      Procedure to start a connection to the server.
 */
int XSWDoConnect(const char *url)
{
        int status, univ_entry_num;
        char *strptr;
        char text[MAX_URL_LEN + 256];
        char local_url[MAX_URL_LEN];
        char title[256];

        
        if(url == NULL)
            return(-1);
           
        /* Copy over URL string. */
        strncpy(local_url, url, MAX_URL_LEN);
        local_url[MAX_URL_LEN - 1] = '\0';

        StringStripSpaces(local_url);

                 
        /* Go through universe list first. */
        for(univ_entry_num = 0;
            univ_entry_num < total_univ_entries;
            univ_entry_num++
        )
        {
            if(univ_entry[univ_entry_num] == NULL)
                continue;
            if(univ_entry[univ_entry_num]->alias == NULL)
                continue;

            /* Match local URL with alias name of universe entry. */
            if(!strcasecmp(local_url, univ_entry[univ_entry_num]->alias))
                break;
        }
        if(univ_entry_num < total_univ_entries)
        {
            /* Got universe entry alias match! */
        
            /* Check if address is valid from URL. */
            strptr = StringParseAddress(univ_entry[univ_entry_num]->url);
            if(strptr == NULL)
            {
                sprintf(text,
                    "%s: Cannot parse address.",
                    univ_entry[univ_entry_num]->url
                );
                MesgAdd(text, xsw_color.standard_text);

                sprintf(text,
                    "Cannot parse address:\n   %s\n",
                    univ_entry[univ_entry_num]->url
                );
                printdw(&err_dw, text);
        
                return(-1);
            }
         
            /* Get login name from URL. */
            strptr = StringParseName(univ_entry[univ_entry_num]->url);
            if(strptr != NULL)
            {
                strncpy( 
                    net_parms.login_name,
                    strptr,
                    XSW_OBJ_NAME_MAX
                );
                net_parms.login_name[XSW_OBJ_NAME_MAX - 1] = '\0';
            }
         
            /* Get password from URL. */
            strptr = StringParsePassword(univ_entry[univ_entry_num]->url);
            if(strptr != NULL)
            {
                strncpy(
                    net_parms.login_password,
                    strptr,  
                    XSW_OBJ_PASSWORD_MAX
                );
                net_parms.login_password[XSW_OBJ_PASSWORD_MAX - 1] = '\0';
            }

            /* Get address. */
            strptr = StringParseAddress(univ_entry[univ_entry_num]->url);
            strncpy(
                net_parms.address,
                strptr,
                MAX_URL_LEN
            );
            net_parms.address[MAX_URL_LEN - 1] = '\0';
         
            /* Get port number. */
            if(StringParsePort(univ_entry[univ_entry_num]->url) > -1) 
                net_parms.port = StringParsePort(univ_entry[univ_entry_num]->url);
            else
                net_parms.port = DEF_SWSERV_PORT;
        }
        else
        {
            /* No alias found, parse local_url. */
                
            /* Check if address is valid from URL. */
            strptr = StringParseAddress(local_url);
            if(strptr == NULL)
            {
                sprintf(text, 
                    "%s: Cannot parse address.",
                    local_url
                );
                MesgAdd(text, xsw_color.standard_text);
                    
                sprintf(text,
                    "Cannot parse address:\n   %s\n",
                    local_url
                );
                printdw(&err_dw, text);
            
                return(-1);
            }
                
            /* Get login name from URL. */
            strptr = StringParseName(local_url);
            if(strptr != NULL)
            {
                strncpy(
                    net_parms.login_name,
                    strptr,
                    XSW_OBJ_NAME_MAX
                );
                net_parms.login_name[XSW_OBJ_NAME_MAX - 1] = '\0';
            }
         
            /* Get password from URL. */
            strptr = StringParsePassword(local_url);
            if(strptr != NULL)
            {
                strncpy(
                    net_parms.login_password,
                    strptr,   
                    XSW_OBJ_PASSWORD_MAX
                );
                net_parms.login_password[XSW_OBJ_PASSWORD_MAX - 1] = '\0';
            }
                    
            /* Get address. */
            strptr = StringParseAddress(local_url);  
            strncpy(
                net_parms.address,
                strptr,
                MAX_URL_LEN
            );
            net_parms.address[MAX_URL_LEN - 1] = '\0';
                
            /* Get port number. */
            if(StringParsePort(local_url) > -1) 
                net_parms.port = StringParsePort(local_url);
            else
                net_parms.port = DEF_SWSERV_PORT;
        }
                    
                    
        /* *********************************************************** */
                
        /* Disconnect if already connected. */
        if(net_parms.socket > -1)
        {
            NetSendDisconnect();
            
            DBDeleteAllObjects();
            NetResetParms();
                    
           /* Clear economy window entry. */
           EcoWinDoDeleteInventory();   
        }
                
             
        /*   NetOpenConnection will print all errors and set up
         *   net_parm variables.
         */
        status = NetOpenConnection(net_parms.address, net_parms.port);
        if(status < 0)
        {
            /* Change bridge window title. */
            sprintf(
                title,
                "%s: Untitled",
                PROG_NAME
            );
            OSWSetWindowTitle(bridge_win.toplevel, title);  
        }
        else
        {
            /* Successful connect. */
                    
            char tmp_addr[80];
            char text[256];
        
        
            /* Change bridge window title. */
            strncpy(tmp_addr, net_parms.address, 80);
            tmp_addr[80 - 1] = '\0';
         
            sprintf(
                title,
                "%s: %s",
                PROG_NAME,
                tmp_addr
            );  
            OSWSetWindowTitle(bridge_win.toplevel, title);
        
            sprintf(
                text,
                "Connecting to %s...",
                tmp_addr
            );
            MesgAdd(text, xsw_color.standard_text);

	    /* Clear starchart on successful connect? */
	    if(option.clear_chart_on_connect)
                SChtClearCB(&starchart_win);
        }
                
        /* Completely redraw bridge window. */
        BridgeWinDrawAll();
              
            
        return(status);
}
         
/*
 *      Procedure to connect to the `last universe' using the
 *      last specified login name and password.
 */
int XSWDoConnectLast(void)
{       
        int status;
        char title[256];
            
         
        /* Connect using last address and port. */
        status = NetOpenConnection(
            net_parms.address,
            net_parms.port
        );
        if(status < 0)
        {
            /* Could not connect. */
            
            /* Change bridge window title. */
            sprintf(
                title,  
                "%s: Untitled",
                PROG_NAME
            );
            OSWSetWindowTitle(bridge_win.toplevel, title);
        }
        else
        {     
            /* Successful connect. */

            char tmp_addr[80];
            char text[256];
  

            /* Change bridge window title. */  
            strncpy(tmp_addr, net_parms.address, 80);
            tmp_addr[80 - 1] = '\0';
        
            sprintf(
                title,  
                "%s: %s",
                PROG_NAME,
                tmp_addr
            );
            OSWSetWindowTitle(bridge_win.toplevel, title);
            
            sprintf(
                text, 
                "Connecting to %s...",
                tmp_addr
            );
            MesgAdd(text, xsw_color.standard_text);

            /* Clear starchart on successful connect? */
            if(option.clear_chart_on_connect)
                SChtClearCB(&starchart_win);
        }
                
                
        /* Completely redraw bridge window. */
        BridgeWinDrawAll();
            
         
        return(0);
}
            
/*
 *      Procedure to disconnect from server.
 *
 *      If not connected, then network resources are deallocated
 *      and reset.
 *
 *      Each call to this function will chalk up one disconnect
 *      send count.  If too many disconnect send counts, then
 *      this function will send a force disconnect and
 *      deallocate and reset all network resources.
 */
void XSWDoDisconnect(void)
{
        int deallocate_resources = 0;
        char text[256];
            
            
        /* Check if currently connected. */
        if(net_parms.socket > -1)
        {
            /* Currently connected, send disconnect. */
            NetSendDisconnect();
            
            /* Increment disconnect send count. */
            net_parms.disconnect_send_count++;
                
            /* Force disconnect if disconnect send count was sent more
             * than 3 times.
             */
            if(net_parms.disconnect_send_count > 3)
                deallocate_resources = 1;
        }
        else
        {
            /* Not connected, reset all network resources and deallocate
             * other resources.
             */
            deallocate_resources = 1;
        }

        /* Deallocate all network related resources? */
        if(deallocate_resources)
        {
            /* Delete all objects. This will also reset the player
             * object referance, delete all scanner contacts, and
             * reset inrange objects list.
             */
            DBDeleteAllObjects();
            
            /* Reset network parameters. */
            NetResetParms();
        
            /* Change bridge window title. */
            sprintf(text, "%s: Untitled", PROG_NAME);  
            OSWSetWindowTitle(bridge_win.toplevel, text);
            
            /* Clear economy window entries. */   
            EcoWinDoDeleteInventory();
                
            /* Reset game controller positions. */
            gctl[0].turn = 0;
            gctl[0].throttle = 0;
            gctl[0].thrust_dir = 0;
                
            warning.incoming_fire = 0;
            warning.weapons_lock = 0;
         
            local_control.weapon_fire_heading = 0;
            local_control.weapon_fire_pitch = 0;
        }
            
        /* Completely redraw bridge window. */
        BridgeWinDrawAll();
        
        
        return;
}
             
/*
 *      Does a network refresh sequence.
 */
void XSWDoRefresh(void)
{
        /* Clear starchart. */
        SChtClearCB(&starchart_win);

        /* Delete object labels. */
        VSLabelDeleteAll();

        /* Delete scanner contacts. */
        ScDeleteAll();

        /* Update inrange objects list. */
        DBInRangeUpdate(net_parms.player_obj_num);

        /* Check if currently connected. */
        if(net_parms.socket > -1)
        {
            /* Send refresh to server. */
            NetSendRefresh();
        }

        /* Completely redraw bridge window. */
        BridgeWinDrawAll();

        return;
}


/*
 *	Performs a procedure whenever an object changes sector.
 */
void XSWDoChangeSector(int object_num)
{
	xsw_object_struct *obj_ptr;


	if(DBIsObjectGarbage(object_num))
	    return;
	else
	    obj_ptr = xsw_object[object_num];


	/* Check if this is the player object. */
	if(obj_ptr == net_parms.player_obj_ptr)
	{
	    /* Perform this procedure whenever the player object changes
	     * sectors.
	     */

            ScHandleContacts(object_num);
            DBReclaim();
            DBInRangeUpdate(object_num);
        }

	return;
}

/*
 *      Procedure to perform when an object is destroyed.
 */
void XSWDoHit(
        int src_obj,	/* Object that hit tar_obj. */
        int tar_obj,	/* Object that was hit. */
        int owner_obj,	/* Owner of src. */
        double structure_damage,
	double shield_damage,
	double bearing	/* In radians, tar_obj to src_obj. */
)
{
	double vol_left, vol_right;
        int sound_code = SOUND_CODE_DEFAULT;
	xsw_object_struct *src_obj_ptr, *tar_obj_ptr, *camera_obj_ptr;
	char text[(2 * XSW_OBJ_NAME_MAX) + 512];


	/* Get pointers to source and target objects. */
	if(DBIsObjectGarbage(src_obj))
	    src_obj_ptr = NULL;
	else
	    src_obj_ptr = xsw_object[src_obj];

        if(DBIsObjectGarbage(tar_obj))
            tar_obj_ptr = NULL;
        else
            tar_obj_ptr = xsw_object[tar_obj];

	/* Get pointer to camera object. */
	camera_obj_ptr = net_parms.player_obj_ptr;
	if(camera_obj_ptr == NULL)
	    return;

        /* Get distance from camera object to target object. */
        if(Mu3DInSameSectorPtr(camera_obj_ptr, tar_obj_ptr))
        {
            vol_left = Mu3DDistance(
                tar_obj_ptr->x - camera_obj_ptr->x,
                tar_obj_ptr->y - camera_obj_ptr->y,
                tar_obj_ptr->z - camera_obj_ptr->z
            );
            /* Max distance is 3.0 real units. */
            vol_left = (3 - vol_left) / 3;
            if(vol_left < 0)
                vol_left = 0;

            vol_right = vol_left;
        }
        else
        {
            vol_left = 0;
            vol_right = 0;
        }


        /* Check if target object is the player object. */
        if(tar_obj == net_parms.player_obj_num)
        {
            /* Notify that we were hit. */
            if(DBIsObjectGarbage(owner_obj))
            {
                sprintf(text,
 "Recieved hit to %s from unknown: %.2f damage to structure,\
 %.2f damage to shields.",
                    DBGetObjectVectorName(bearing),
                    structure_damage, shield_damage
                );
                sound_code = SOUND_CODE_HIT_SHIELDS;
            }
            else
            {
                sprintf(text,
 "Recieved hit to %s from %s: %.2f damage to structure,\
 %.2f damage to shields.",
                    DBGetObjectVectorName(bearing),
                    xsw_object[owner_obj]->name,
                    structure_damage, shield_damage
                );
                sound_code = SOUND_CODE_HIT_STRUCTURE;
            }
            MesgAdd(text, xsw_color.standard_text);

            if((option.sounds > XSW_SOUNDS_NONE) &&
               ((vol_left > 0) || (vol_right > 0))
            )
                SoundPlay(
                    sound_code,
                    vol_left, vol_right,
                    0,
                    0
                );
        }
        /* Check if weapon object owner is the player object. */
        else if(owner_obj == net_parms.player_obj_num)
        {
            /* Notify that we hit an object. */
            if(DBIsObjectGarbage(tar_obj))
            {
                if(shield_damage > 0)
                {
                    sprintf(text,
 "Unknown object hit %s: %.2f damage to structure,\
 %.2f damage to shields.",
                        DBGetObjectVectorName(bearing),
                        structure_damage, shield_damage
                    );
                    sound_code = SOUND_CODE_OHIT_SHIELDS;
                }
                else
                {
                    sprintf(text,
 "Unknown object hit %s: %.2f damage to structure.",
                        DBGetObjectVectorName(bearing),
                        structure_damage
                    );
                    sound_code = SOUND_CODE_OHIT_STRUCTURE;
                }
            }
            else
            {
                if(shield_damage > 0)
                {
                    sprintf(text,
 "%s hit %s: %.2f damage to structure, %.2f damage to shields.",
                        xsw_object[tar_obj]->name,
                        DBGetObjectVectorName(bearing),
                        structure_damage, shield_damage
                    );
                    sound_code = SOUND_CODE_OHIT_SHIELDS;
                }
                else
                {
                    sprintf(text,
 "%s hit %s: %.2f damage to structure.",
                        xsw_object[tar_obj]->name,
                        DBGetObjectVectorName(bearing),
                        structure_damage
                    );
                    sound_code = SOUND_CODE_OHIT_STRUCTURE;
                }
            }
            MesgAdd(text, xsw_color.standard_text);
                 
            if((option.sounds > XSW_SOUNDS_NONE) &&
               ((vol_left > 0) || (vol_right > 0))  
            )
                SoundPlay(
                    sound_code,
                    vol_left, vol_right,
                    0,
                    0
                );
        }
        /* Someone else's weapon hit some object. */
        else
        {
            if(shield_damage > 0)
                sound_code = SOUND_CODE_OHIT_SHIELDS;
            else
                sound_code = SOUND_CODE_OHIT_STRUCTURE;
                      
            if((option.sounds > XSW_SOUNDS_NONE) &&
               ((vol_left > 0) || (vol_right > 0))
            )
                SoundPlay(
                    sound_code,  
                    vol_left, vol_right,
                    0,
                    0
                );
        }
}

/*
 *      Procedure to perform when an object is destroyed.
 */
void XSWDoDestroyed(
        int src_obj,	/* Object that caused destruction. */
        int tar_obj,	/* Object that was destroyed. */
	int owner_obj,	/* Owner of src_obj. */
	int reason	/* Reason code (sent from server). */
)
{
        xsw_object_struct *src_obj_ptr, *tar_obj_ptr;
	char text[(2 * XSW_OBJ_NAME_MAX) + 512];


        /* Get pointers to source and target objects. */
        if(DBIsObjectGarbage(src_obj))
            src_obj_ptr = NULL;
        else
            src_obj_ptr = xsw_object[src_obj];

        if(DBIsObjectGarbage(tar_obj))
            tar_obj_ptr = NULL;
        else
            tar_obj_ptr = xsw_object[tar_obj];


        /* Check if the destroyef object is the player object. */
        if(tar_obj_ptr == net_parms.player_obj_ptr)
        {
            /* Player got destroyed. */

	    /* Change viewscreen page to display the `destroyed'
	     * page, this will not change it immediatly but
	     * when we are disconnected (which is very soon)
	     * the `destroyed' page is the page that will be shown.
	     */
	    VSChangePage(&bridge_win.destroyed_page);


        }     
        else
        {
            /* Some other object got destroyed. */



            sprintf(
                text,
                "%s has been destroyed.",
                ((tar_obj_ptr == NULL) ?
		    "Unknown object" : tar_obj_ptr->name
		)
            );
            MesgAdd(text, xsw_color.standard_text);
        }

        return;
}
        

