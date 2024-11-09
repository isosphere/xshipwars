/*
                        Image paths and values

	Global images directory should be prepended by program
	to all paths here that are not absolute paths.

	Note: Image codes are in xsw.h.

 */

#ifndef IMAGEPATHS_H
#define IMAGEPATHS_H

#include "../include/disk.h"

/* Icons. */
#define IMGPATH_ICON_BRIDGE	"client"PATH_SEP_STR"icon_bridge.tga"
#define IMGPATH_ICON_ECONOMY	"client"PATH_SEP_STR"icon_economy.tga"
#define IMGPATH_ICON_OPTIONS	"client"PATH_SEP_STR"icon_options.tga"
#define IMGPATH_ICON_UNIVERSE	"client"PATH_SEP_STR"icon_universe.tga"
#define IMGPATH_ICON_STARCHART	"client"PATH_SEP_STR"icon_starchart.tga"


/* Splash background. */
#define IMGPATH_SPLASH_BKG	"client"PATH_SEP_STR"splash_bkg.tga"
#define IMGPATH_SPLASH_PB_L	"client"PATH_SEP_STR"splash_pb_l.tga"
#define IMGPATH_SPLASH_PB_R	"client"PATH_SEP_STR"splash_pb_r.tga"
#define IMGPATH_SPLASH_PB_T	"client"PATH_SEP_STR"splash_pb_t.tga"


/* Viewscreen superimposed target marks. */
#define IMGPATH_VS_TMARK_OBJECT	"client"PATH_SEP_STR"vs_tmark_object.tga"
#define IMGPATH_VS_TMARK_VESSEL	"client"PATH_SEP_STR"vs_tmark_vessel.tga"
#define IMGPATH_VS_MARK_INCOMINGFIRE	"client"PATH_SEP_STR"vs_mark_incoming_fire.tga"
#define IMGPATH_VS_WEP_PROJECTILE	"client"PATH_SEP_STR"vs_wep_projectile.tga"
#define IMGPATH_VS_WEP_PULSE	"client"PATH_SEP_STR"vs_wep_pulse.tga"
#define IMGPATH_VS_WEP_STREAM	"client"PATH_SEP_STR"vs_wep_stream.tga"

/* Scanner marks. */
#define IMGPATH_SCMARK_UNKNOWN	"client"PATH_SEP_STR"scanner_mark_unknown.tga"
#define IMGPATH_SCMARK_LOCKED	"client"PATH_SEP_STR"scanner_mark_locked.tga"
#define IMGPATH_SCMARK_WEAPON	"client"PATH_SEP_STR"scanner_mark_weapon.tga"
#define IMGPATH_SCMARK_HOME	"client"PATH_SEP_STR"scanner_mark_home.tga"
#define IMGPATH_SCMARK_AREA	"client"PATH_SEP_STR"scanner_mark_area.tga"

/* Bridge console panel backgrounds. */
#define IMGPATH_BPANEL_L1	"client"PATH_SEP_STR"bpanel_l1.tga" 
#define IMGPATH_BPANEL_L2	"client"PATH_SEP_STR"bpanel_l2.tga"
#define IMGPATH_BPANEL_L3	"client"PATH_SEP_STR"bpanel_l3.tga"   
#define IMGPATH_BPANEL_L4	"client"PATH_SEP_STR"bpanel_l4.tga"
#define IMGPATH_BPANEL_R1	"client"PATH_SEP_STR"bpanel_r1.tga"
#define IMGPATH_BPANEL_R2	"client"PATH_SEP_STR"bpanel_r2.tga"
#define IMGPATH_BPANEL_R3	"client"PATH_SEP_STR"bpanel_r3.tga"
#define IMGPATH_BPANEL_R4	"client"PATH_SEP_STR"bpanel_r4.tga"
#define IMGPATH_BPANEL_MESG	"client"PATH_SEP_STR"bpanel_mesg.tga"

/* Bridge console panel outlines. */
#define IMGPATH_BPANEL_OL_HULL	"client"PATH_SEP_STR"bpanel_ol_hull.tga"
#define IMGPATH_BPANEL_OL_POWER	"client"PATH_SEP_STR"bpanel_ol_power.tga"
#define IMGPATH_BPANEL_OL_VIS	"client"PATH_SEP_STR"bpanel_ol_vis.tga"
#define IMGPATH_BPANEL_OL_SHIELDS	"client"PATH_SEP_STR"bpanel_ol_shields.tga"
#define IMGPATH_BPANEL_OL_DMGCTL	"client"PATH_SEP_STR"bpanel_ol_dmgctl.tga"

#define IMGPATH_BPANEL_OL_THROTTLE_L	"client"PATH_SEP_STR"bpanel_ol_throttlel.tga"
#define IMGPATH_BPANEL_OL_THROTTLE_R	"client"PATH_SEP_STR"bpanel_ol_throttler.tga"

#define IMGPATH_BPANEL_OL_THRUSTVECTOR	"client"PATH_SEP_STR"bpanel_ol_thrustvector.tga"

/* Starchart icons. */
#define IMGPATH_SCHT_ZOOM_IN	"client"PATH_SEP_STR"zoom_in.tga"
#define IMGPATH_SCHT_ZOOM_OUT	"client"PATH_SEP_STR"zoom_out.tga"
#define IMGPATH_SCHT_JUMP_TO_PLAYER	"client"PATH_SEP_STR"jump_to_player.tga"

/* Large message screen background. */
#define IMGPATH_MESG_SCR_BKG	"client"PATH_SEP_STR"mesg_scr_bkg.tga"


/* Misc. */
#define IMGPATH_LENSFLARE1	"client"PATH_SEP_STR"lensflare1.tga"
#define IMGPATH_LENSFLARE2	"client"PATH_SEP_STR"lensflare2.tga"

#define IMGPATH_STROBEGLOW1	"client"PATH_SEP_STR"strobeglow1.tga"



#endif	/* IMAGEPATHS_H */
