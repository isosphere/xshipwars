/*
                       Game Controller Management

	Functions:

	int GCtlInit(int controller)
	void GCtlUpdate()
	void GCtlShutdown()

	---

	These functions serve as a front end for other functions
	to easilly tell what controller state (not needing to know
	which type of controller is in use) the currect controller
	is in by checking the values of the members of the gctl
	structures.

 */

#include "keymap.h"

#include "xsw.h"


/*
 *	Switches to and initializes controller.
 *	Previous controller will be automatically shut down.
 *
 *	Returns non zero on error.
 */
int GCtlInit(int controller)
{
	int i, status, prev_controller;
	char text[PATH_MAX + NAME_MAX + 10];


	/* Record previous controller type. */
	prev_controller = option.controller;


	/* *********************************************************** */
	/* First shut down current controllers (as needed). */

	/* Shutdown pointer. */

	/* Shutdown keyboard. */

#ifdef JS_SUPPORT
	/* Shutdown joystick. */
	for(i = 0; i < total_jsmaps; i++)
	{
	    if(jsmap[i] == NULL)
		continue;

            JSClose(&jsmap[i]->jsd);
	}
#endif	/* JS_SUPPORT */



        /* *********************************************************** */
	/* Initialize new controller. */
	switch(controller)
	{
#ifdef JS_SUPPORT
	  case CONTROLLER_JOYSTICK:
	    for(i = 0; i < total_jsmaps; i++)
            {
                if(jsmap[i] == NULL)
                    continue;

		if(jsmap[i]->device_name == NULL)
		    continue;

		/* initialize. */
                status = JSInit(
                    &jsmap[i]->jsd,
                    jsmap[i]->device_name,
                    fname.js_calib,
		    JSFlagNonBlocking
                );
		if(status != JSSuccess)
		{
		    JSClose(&jsmap[i]->jsd);
		    sprintf(
			text,
	"Cannot initialize joystick %i: %s\n",
			i,
			jsmap[i]->device_name
		    );
		    MesgAdd(text, xsw_color.bold_text);
		}
		else
		{
                    JSMapSyncWithData(jsmap[i]);
		}
            }
	    break;
#endif /* JS_SUPPORT */

	  case CONTROLLER_POINTER:
	    /* Not supported yet. */
	    return(-2);

	    break;

	  default: /* Default to CONTROLLER_KEYBOARD. */
	    /* No initializing of keyboard needed. */

	    break;
	}


	/*   Need to update new controller being used in global
	 *   option.controller
	 */
	option.controller = controller;


	/* Reset gctls. */
	memset(&gctl[0], 0x00, sizeof(xsw_gctl_struct));


	return(0);
}



/*
 *	Gets controller state and updates all gctl structures.
 *
 *	Global settings such as keymappings and throttle settings
 *	are taken into account and calculated!
 *
 */
void GCtlUpdate(int controller_type)
{
	/* Previous key states. */
	static char	pks_turn_left,
			pks_turn_right,
			pks_throttle_inc,
			pks_throttle_dec,
			pks_throttle_idle,
			pks_fire_weapon,
			pks_omni_dir_thrust,
			pks_external_dampers;

        int i, n;

	double	d1, d2,
		pks_x_accel, pks_y_accel;

	/* For keyboard event handling from GUI. */
	event_t event;
	char key_state;		/* 0 = released, 1 = pressed. */

#ifdef JS_SUPPORT
	jsmap_struct *jsmap_ptr;
	jsmap_axis_struct *jsmap_axis_ptr;
	jsmap_button_struct *jsmap_button_ptr;
#endif	/* JS_SUPPORT */


	while(1)
	{
#ifdef JS_SUPPORT
	    /* Go through each joystick mapping. */
	    for(i = 0; i < total_jsmaps; i++)
	    {
		jsmap_ptr = jsmap[i];

		if(jsmap_ptr == NULL)
		    continue;

		if(JSUpdate(&jsmap_ptr->jsd) != JSGotEvent)
		    continue;

		/* Go through each axis. */
		for(n = 0; n < jsmap_ptr->total_axises; n++)
		{
		    jsmap_axis_ptr = jsmap_ptr->axis[n];

		    if(jsmap_axis_ptr == NULL)
			continue;

		    switch(jsmap_axis_ptr->op_code)
		    {
		      case JSMAP_AXIS_OP_TURN:
			gctl[0].turn = JSGetAxisCoeffNZ(
			    &jsmap_ptr->jsd,
			    n		/* Axis number on jsd should match. */
			);
			break;

                      case JSMAP_AXIS_OP_THROTTLE:
			switch(option.throttle_mode)
			{
			  case THROTTLE_MODE_BIDIRECTIONAL:
			    /* Bi directional mode. */
                            gctl[0].throttle = JSGetAxisCoeffNZ(
                                &jsmap_ptr->jsd,
                                n	/* Axis number on jsd should match. */
                            );
			    break;

			  case THROTTLE_MODE_INCREMENTAL:
                            gctl[0].throttle = JSGetAxisCoeffNZ(
                                &jsmap_ptr->jsd,
                                n       /* Axis number on jsd should match. */
                            );
			    break;

			  default:
			    /* Normal mode. */
                            gctl[0].throttle = (JSGetAxisCoeff(
                                &jsmap_ptr->jsd,
                                n       /* Axis number on jsd should match. */
                            ) + 1) / 2;
			    break;
			}
			break;

                      case JSMAP_AXIS_OP_THRUST_DIR:
			gctl[0].thrust_dir = JSGetAxisCoeffNZ(
                            &jsmap_ptr->jsd,
                            n	/* Axis number on jsd should match. */
                        );
			break;

                      case JSMAP_AXIS_OP_AIM_WEAPON_HEADING:
                        gctl[0].aim_weapon_heading = JSGetAxisCoeffNZ(
                            &jsmap_ptr->jsd,
                            n   /* Axis number on jsd should match. */
                        );
                        break;

                      case JSMAP_AXIS_OP_AIM_WEAPON_PITCH:
                        gctl[0].aim_weapon_pitch = JSGetAxisCoeffNZ(
                            &jsmap_ptr->jsd,
                            n   /* Axis number on jsd should match. */
                        );
                        break;

                      case JSMAP_AXIS_OP_SCANNER_ZOOM:
			d1 = bridge_win.scanner_sb.pos;
			d2 = bridge_win.scanner_sb.pos_max -
			    bridge_win.scanner_sb.pos_min;
                        bridge_win.scanner_sb.pos = (JSGetAxisCoeff(
                            &jsmap_ptr->jsd, n
                        ) - 1) / -2 * d2;
			/* Any change? */
			if(d1 != bridge_win.scanner_sb.pos)
			{
                            /* Sanitize values. */
                if(bridge_win.scanner_sb.pos < bridge_win.scanner_sb.pos_min)
                    bridge_win.scanner_sb.pos = bridge_win.scanner_sb.pos_min;  
		else if(bridge_win.scanner_sb.pos > bridge_win.scanner_sb.pos_max)
                    bridge_win.scanner_sb.pos = bridge_win.scanner_sb.pos_max;

			    ScannerSBCB(&bridge_win.scanner_sb);
			    ScaleBarDraw(&bridge_win.scanner_sb);
			}
                        break;

                      case JSMAP_AXIS_OP_VS_ZOOM:
			d1 = bridge_win.viewscreen_zoom;
                        bridge_win.viewscreen_zoom = (JSGetAxisCoeff(
			    &jsmap_ptr->jsd, n
			) - 1) / -2;
			if(d1 != bridge_win.viewscreen_zoom)
			{
			    if(bridge_win.viewscreen_zoom < VS_ZOOM_MIN)
                                bridge_win.viewscreen_zoom = VS_ZOOM_MIN;
			    else if(bridge_win.viewscreen_zoom > VS_ZOOM_MAX)
			        bridge_win.viewscreen_zoom = VS_ZOOM_MAX;

			}
                        break;
		    }
		}
                /* Go through each button. */
                for(n = 0; n < jsmap_ptr->total_buttons; n++)
                {
                    jsmap_button_ptr = jsmap_ptr->button[n];

                    if(jsmap_button_ptr == NULL)
                        continue;

		    if(n >= jsmap_ptr->jsd.total_buttons)
			break;
		    if(jsmap_ptr->jsd.button[n] == NULL)
			continue;


		    /* Change in button state? */
		    if(jsmap_ptr->jsd.button[n]->state == JSButtonStateOn)
		    {
			if(jsmap_button_ptr->state)
			    continue;
			else
			    jsmap_button_ptr->state = True;
/*
printf("Button %i: ON\n", n);
 */
		    }
		    else
		    {
                        if(jsmap_button_ptr->state)
                            jsmap_button_ptr->state = False;
			else
			    continue;
/*
printf("Button %i: OFF\n", n);
 */
		    }

		    /* Format a GUI KeyPress event to be sent to the GUI
		     * as if a keyboard key was pressed.
		     */
                    event.type = ((jsmap_button_ptr->state) ?
			KeyPress : KeyRelease
		    );
		    event.xany.window = bridge_win.toplevel;
		    event.xkey.keycode = jsmap_button_ptr->keycode;

                    /* Send a KeyPress event. */
		    OSWSendEvent(
			KeyPressMask | KeyReleaseMask,
			&event,
			True
		    );
		}
	    }
#endif /* JS_SUPPORT */

	    /* ********************************************************* */
	    /* Keyboard events. */

	    /*   Break if GUI is not connected or the bridge window is
             *   not in focus.
	     */
	    if(!bridge_win.is_in_focus ||
               (prompt_mode != PROMPT_CODE_NONE)
	    )
		break;

	    while(OSWEventsPending() > 0)
	    {
		/* Check and get Key events only, break if no more exist. */
	        if(!OSWCheckMaskEvent(
		    KeyPressMask | KeyReleaseMask,
		    &event
		   )
		)
		     break;

		/* Get key state. */
		key_state = ((event.type == KeyPress) ? 1 : 0);


                /* Turn left. */
                if(event.xkey.keycode == xsw_keymap[XSW_KM_TURN_LEFT].keycode)
                {
                    if(key_state)
                    {
			OSWKBAutoRepeatOff();
		        pks_turn_left = 1;
                    }
                    else
                    {
		        pks_turn_left = 0;
                    }
                }
                /* Turn right. */
                else if(event.xkey.keycode == xsw_keymap[XSW_KM_TURN_RIGHT].keycode)
                {
		    if(key_state)
		    {
                        OSWKBAutoRepeatOff();
                        pks_turn_right = 1;
		    }
		    else
                    {
			pks_turn_right = 0;
                    }
                }
	        /* Throttle up. */
                else if(event.xkey.keycode == xsw_keymap[XSW_KM_THROTTLE_INC].keycode)
                {
                    if(key_state)
                    {
                        OSWKBAutoRepeatOff();
		        pks_throttle_inc = 1;
                    }
                    else
                    {
		        pks_throttle_inc = 0;
                    }
                }
                /* Throttle down. */
                else if(event.xkey.keycode == xsw_keymap[XSW_KM_THROTTLE_DEC].keycode)
                {
                    if(key_state)
                    {
                        OSWKBAutoRepeatOff();
                        pks_throttle_dec = 1;
                    }
                    else
                    {
                        pks_throttle_dec = 0;
                    }
                }
		/* Throttle idle. */
                else if(event.xkey.keycode == xsw_keymap[XSW_KM_THROTTLE_IDLE].keycode)
                {
                    if(key_state)
                    {
                        OSWKBAutoRepeatOff();
                        pks_throttle_idle = 1;
                    }
                    else
                    {
                        pks_throttle_idle = 0;
                    }
                }
                /* Fire weapon. */
                else if(event.xkey.keycode == xsw_keymap[XSW_KM_FIRE_WEAPON].keycode)
                {
                    if(key_state)
                    {
                        gctl[0].fire_weapon = 1;
                        OSWKBAutoRepeatOff();
			pks_fire_weapon = 1;
                    }
                    else
                    {
                        gctl[0].fire_weapon = 0;
                        pks_fire_weapon = 0;
                    }
                }
                /* Omni directrional thrusters. */
		else if(event.xkey.keycode == xsw_keymap[XSW_KM_OMNI_DIR_THRUST].keycode)
		{
                    if(key_state)
                    {
                        gctl[0].omni_dir_thrust = 1;
                        OSWKBAutoRepeatOff();
                        pks_omni_dir_thrust = 1;
                    }
                    else
                    {
                        gctl[0].omni_dir_thrust = 0;
                        pks_omni_dir_thrust = 0;
                    }
		}
                /* External dampers. */
                else if(event.xkey.keycode == xsw_keymap[XSW_KM_EXTERNAL_DAMPERS].keycode)
                {
                    if(key_state)
                    {
                        gctl[0].external_dampers = 1;
                        OSWKBAutoRepeatOff();
                        pks_external_dampers = 1;
                    }
                    else
                    {
                        gctl[0].external_dampers = 0;
                        pks_external_dampers = 0;
                    }
                }
		else
		{
		    OSWPutBackEvent(&event);
		    break;
		}
	    }

	    /* Turn auto repeat back on only when all states false. */
	    if(!pks_turn_left &&
               !pks_turn_right &&
               !pks_throttle_inc &&
               !pks_throttle_dec &&
               !pks_throttle_idle &&
               !pks_fire_weapon &&
               !pks_omni_dir_thrust &&
               !pks_external_dampers
	    )
		OSWKBAutoRepeatOn();


	    /* ********************************************************* */
	    /*   Update game controller states for axis oriented
             *   functions (only when specifically in keyboard mode).
             */
	    if(controller_type == CONTROLLER_KEYBOARD)
	    {
                if(osw_gui[0].shift_key_state)
                    pks_x_accel = gctl[0].aim_weapon_heading;
                else
		    pks_x_accel = gctl[0].turn;

	        /* Turn left (x). */
	        if(pks_turn_left)
	        {
		    pks_x_accel -= 0.02 * time_compensation;
	        }
	        /* Turn right (x). */
	        else if(pks_turn_right)
	        {
		    pks_x_accel += 0.02 * time_compensation;
	        }
	        /* Turn left (x) decelleration. */
	        else if(pks_x_accel < 0)
	        {
		    pks_x_accel += 0.02 * time_compensation;
	        }
	        /* Turn right (x) decelleration. */
                else if(pks_x_accel > 0)
                {
                    pks_x_accel -= 0.02 * time_compensation;
                }

	        /* Sanitize accelerations (x). */
	        if(pks_x_accel > 1)
		    pks_x_accel = 1;
	        if(pks_x_accel < -1)
		    pks_x_accel = -1;
	        if((pks_x_accel > (-0.01 * time_compensation)) &&
                   (pks_x_accel < (0.01 * time_compensation))
	        )
		    pks_x_accel = 0;

		if(osw_gui[0].shift_key_state)
		    gctl[0].aim_weapon_heading = pks_x_accel;
		else
		    gctl[0].turn = pks_x_accel;


                pks_y_accel = gctl[0].throttle;

	        /* Turottle increase (y). */
                if(pks_throttle_inc)
                {
                    pks_y_accel += ((osw_gui[0].shift_key_state) ? 0.01 : 0.002)
		        * time_compensation;
                    if(pks_y_accel > 1)
                        pks_y_accel = 1;
                }
	        /* Throttle decrease (y). */
                if(pks_throttle_dec)
                {
                    pks_y_accel -= ((osw_gui[0].shift_key_state) ? 0.01 : 0.002)
		        * time_compensation;
		    if(pks_y_accel < -1)
		        pks_y_accel = -1;
                }
	        /* Throttle null zone (y), if was not 0. */
	        if((pks_y_accel < 0.005) &&
                   (pks_y_accel > -0.005) &&
                   (gctl[0].throttle != 0)
	        )
		    pks_y_accel = 0;

		/* Throttle set to idle? */
		if(pks_throttle_idle)
		    pks_y_accel = 0;

	        gctl[0].throttle = pks_y_accel;


		/* Thrust dir. */
		gctl[0].thrust_dir = 0;
	    }

            break;
	}	/* while(1) */



	return;
}



/*
 *	Closes all game controllers.
 */
void GCtlShutdown()
{
	int i;


#ifdef JS_SUPPORT
	/* Close all joysticks. */
	for(i = 0; i < total_jsmaps; i++)
	{
	    if(jsmap[i] == NULL)
		continue;

	    JSClose(&jsmap[i]->jsd);
	}
#endif	/* JS_SUPPORT */

	/* Close pointer device. */

	/* Set global options controller setting to something sane. */
	option.controller = CONTROLLER_KEYBOARD;


	return;
}
