/*
                         XSW Action Callback

	Functions:

	void XSWActionCB(void *, void *data, int action)

	---

 */

#include "keymap.h"
#include "univlist.h"
#include "optwin.h"
#include "keymapwin.h"
#include "starchartwin.h"

#ifdef JS_SUPPORT
# include "jsmapwin.h"
#endif  /* JS_SUPPORT */

#include "xsw.h"
#include "net.h"


/*
 *	The ptr specifies the pointer to the XSW window
 *	structure (ie bridge_win, eco_win, keymap_win) or NULL if
 *	not available.
 *
 *	The data specifies a special argument which is determined by
 *	the action code (see below).
 *
 *	The action code specifies the action to perform, it also
 *	determines what data should be.
 */
void XSWActionCB(void *ptr, void *data, int action)
{
	int con_state, player_obj_num;
	xsw_object_struct *player_obj_ptr;


	/* Get global variable values. */
	con_state = net_parms.connection_state;
	player_obj_num = net_parms.player_obj_num;
	player_obj_ptr = net_parms.player_obj_ptr;


	switch(action)
	{
	  /* ******************************************************* */
	  case XSW_ACTION_NONE:
	    /* Do nothing. */
	    break;

          /* ******************************************************* */
          /* Run server. */
	  case XSW_ACTION_RUN_SERVER:
            XSWMapFB(NULL, PRI_FB_LOADOP_RUN_SERVER);
	    break;

          /* Server script. */
          case XSW_ACTION_SERVER_SCRIPT:
            XSWMapFB(NULL, PRI_FB_LOADOP_SERVER_SCRIPT);
            break;

          /* Load OCSN file. */ 
          case XSW_ACTION_LOAD_OCSN:
            XSWMapFB(NULL, PRI_FB_LOADOP_OCSN);
            break;

          /* Load Image Set Referance file. */   
          case XSW_ACTION_LOAD_ISREF:
            XSWMapFB(NULL, PRI_FB_LOADOP_ISREF);   
            break;
         
          /* Load Sound Scheme file */
          case XSW_ACTION_LOAD_SS:
            XSWMapFB(NULL, PRI_FB_LOADOP_SS);
            break;

	  /* Load starchart. */
	  case XSW_ACTION_LOAD_STARCHART_OVERLAY:
            XSWMapFB(NULL, PRI_FB_LOADOP_STARCHART_OVERLAY);
	    break;

          /* ******************************************************* */
          /* Connect to universe. */
          case XSW_ACTION_CONNECT:
	    if(data == NULL)
	    {
		/* No data specified, so map universe list window
		 * to query user for selection.
		 */
		UnivListMap();
	    }
	    else
	    {
		XSWDoConnect((char *)data);
	    }
            break;

	  /* Connect to the `last' universe. */
          case XSW_ACTION_CONNECT_LAST:
	    XSWDoConnectLast();
            break;

	  /* Disconnect. */
          case XSW_ACTION_DISCONNECT:
            XSWDoDisconnect();
            break;

	  /* Refresh. */
          case XSW_ACTION_REFRESH:
            XSWDoRefresh();
            break;

	  /* Toggle auto interval on/off. */
          case XSW_ACTION_AINT:
            if(auto_interval_tune.state)
                CmdAutoInterval("off");
            else
                CmdAutoInterval("on");
            break;


          /* ******************************************************* */
          /* Toggle shields raised/lowered. */
          case XSW_ACTION_SHIELDS:
            if(player_obj_ptr != NULL)
            {
                switch(player_obj_ptr->shield_state)
                {
                  case SHIELD_STATE_DOWN:
                    NetSendSetShields(
                        player_obj_num,
                        SHIELD_STATE_UP,
                        player_obj_ptr->shield_frequency
                    );
                    break;

                  default:
                    NetSendSetShields(
                        player_obj_num,
                        SHIELD_STATE_DOWN,
                        player_obj_ptr->shield_frequency
                    );
                    break;
                }
            }
            break;

	  /* Toggle cloak raised/lowered. */
          case XSW_ACTION_CLOAK:
            if(player_obj_ptr != NULL)
            {
                NetSendSetCloak(
                    player_obj_num,
                    ((player_obj_ptr->cloak_state == CLOAK_STATE_UP) ?
                        CLOAK_STATE_DOWN : CLOAK_STATE_UP
		    )
                );
            }
            break;

          /* Toggle damage control on/off. */
          case XSW_ACTION_DMGCTL:
            if(player_obj_ptr != NULL) 
            {
                NetSendSetDmgCtl(
                    player_obj_num,
                    ((player_obj_ptr->damage_control == DMGCTL_STATE_ON) ?
                        DMGCTL_STATE_OFF : DMGCTL_STATE_ON
		    )
                );
            }
            break;

	  /* Lock weapons on next object. */
          case XSW_ACTION_WLOCKNEXT:
            NetSendWeaponsLock(player_obj_num, -2);
            break;

	  /* Unlock weapons. */
          case XSW_ACTION_WUNLOCK:
            NetSendWeaponsUnlock(player_obj_num);
            break;


          /* ******************************************************* */
          /* Syctonize timmings. */
          case XSW_ACTION_SYNCTIME:
            CmdSynctime("");
            break;

	  /* Cycle through viewscreen labeling. */
          case XSW_ACTION_DISPLABELS:
            option.show_viewscreen_labels++;
            if(option.show_viewscreen_labels > 3)
                option.show_viewscreen_labels = 0;

            /* Redraw viewscreen. */
            VSDrawViewScreen(
                net_parms.player_obj_num,
                bridge_win.viewscreen,
                bridge_win.viewscreen_image,
                bridge_win.viewscreen_zoom
            );
            break;

	  /* Display memory. */
          case XSW_ACTION_MEMORY:
            CmdMemory("");
            break;

	  /* Map options window. */
          case XSW_ACTION_OPTIONS:
            OptWinDoMapValues();
            break;

	  /* Edit keymappings. */
          case XSW_ACTION_EDIT_KEYMAP:
            KeymapWinDoMapValues();
            break;

	  /* Exit with comfermation. */
          case XSW_ACTION_COMFERM_EXIT:
	    if(ptr == (void *)&bridge_win)
	    {
		prompt_window_struct *prompt;

		prompt = &bridge_win.prompt;

		prompt_mode = PROMPT_CODE_EXIT;
		PromptChangeName(prompt, "Exit?:");
                PromptMap(prompt);
                prompt->is_in_focus = 1;
	    }
            break;

	  /* Exit. */
          case XSW_ACTION_EXIT:
            runlevel = 1;
            break;

	  /* Map economy window. */
          case XSW_ACTION_ECONOMY:
            if(!eco_win.map_state)
            {
                EcoWinDoDeleteInventory();
                EcoWinMap();
            }
            if(player_obj_ptr != NULL)
            {
		int object_num;

                object_num = player_obj_ptr->locked_on;
                if(object_num < 0)
                {
                    /* Not locked on anything, just request score. */
                    NetSendEcoReqValues(player_obj_num, object_num);
                } 
                else
                {
                    /* Locked on an object, request name and eco info. */
                    NetSendReqName(object_num);
                    NetSendEcoReqValues(player_obj_num, object_num);
                } 
            }       
            break;

	  /* Map joystick map window. */            
          case XSW_ACTION_EDIT_JSMAP:
#ifdef JS_SUPPORT 
            JSMWMapValues();
#endif  /* JS_SUPPORT */
            break;

	  /* Map starchart window. */
	  case XSW_ACTION_STARCHART:
	    SChtMap(&starchart_win);
	    break;

          /* ******************************************************* */
          /* Send who command to server. */
          case XSW_ACTION_WHO:
            NetSendExec("who");
            break;

	  /* Send netstat command to server. */
          case XSW_ACTION_NETSTAT:
            NetSendExec("netstat me");
            break;

	  /* Send memory command to server. */
          case XSW_ACTION_RMEMORY:
            NetSendExec("memory");
            break;




          /* ********************************************************* */
          /* Links to other pages. */
          case XSW_ACTION_GOTO_MAIN:
	    if(ptr == (void *)&bridge_win)
	    {
		VSChangePage(&bridge_win.main_page);
	    }
            break;

          case XSW_ACTION_GOTO_DESTROYED:
            if(ptr == (void *)&bridge_win)
            {
		VSChangePage(&bridge_win.destroyed_page);
	    }
            break;

          /* ******************************************************* */
          default:
	    if(1)
	    {
		char text[256];

		sprintf(text,
"Unsupported action code:\n\n\
    %i",
		    action
		);
		printdw(&err_dw, text);
	    }
            break;
	}


	return;
}
