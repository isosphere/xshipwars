/*
                         Program Primary Routines

	Functions:

 	void XSWDoPrintDebug()
 	void XSWDoHelp()
 	void XSWDoVersion()
	void XSWDoHelpMesgWin()

	int XSWIsDescriptorValid(int s)

	int XSWStartServer(char *cmd)
	int XSWLoadOCSN(char *path)  
	int XSWLoadIsrefs(char *path)
	int XSWLoadSS(char *path)

	int XSWLoadColors()
	void XSWFreeColors()

	void XSWGetProgMemory(xsw_mem_stat_struct *buf)
	void XSWReclaimGlobalMemory(bool_t verbose)

	void XSWDoUnfocusAllWindows()
	void XSWDoRestackWindows()
	void XSWManageGUI()
	void XSWManageSound()
	void XSWDoChangeBackgroundMusic()

	int XSWScrollBarCB(scroll_bar_struct *ptr)
 	void XSWHandleSignal(int s)
        void XSWDoResetTimmers()

        int XSWInit(int argc, char *argv[])
	void XSWManage()
        void XSWShutdown()

 	int main(int argc, char *argv[])

	---

 */

#include <signal.h>

#include "../include/tga.h"

#include "../include/unvmain.h"
#include "../include/unvutil.h"
#include "../include/swsoundcodes.h"

#include "keymap.h"
#include "ss.h"

#include "splash.h"
#include "univlist.h"
#include "optwin.h"
#include "keymapwin.h"
#include "jsmapwin.h"
#include "starchartwin.h"
#include "xswinstall.h"

#include "xsw.h"
#include "net.h"


/* Cursor Pixmaps. */
#include "../include/cursors/xsw_scanner_lock.xpm"
#include "../include/cursors/xsw_text.xpm"
#include "../include/cursors/translate.xpm"
#include "../include/cursors/zoom.xpm"


#define MIN(a,b)        (((a) < (b)) ? (a) : (b))
#define MAX(a,b)        (((a) > (b)) ? (a) : (b))



pid_t root_pid;
int runlevel;

time_t cur_millitime;
time_t cur_systime;
time_t lapsed_millitime;
double time_compensation;
int prompt_mode;

xsw_debug_struct debug;
xsw_option_struct option;
xsw_sound_struct sound;
xsw_dname_struct dname;
xsw_fname_struct fname;
xsw_next_struct next;

xsw_fps_counter_struct fps_counter;
xsw_genanim_timmer_struct genanim_timmer[MAX_ANIM_TIMMERS];

xsw_font_struct xsw_font;
xsw_color_struct xsw_color;
xsw_cursor_struct xsw_cursor;

bpanel_btnpos_struct **bpanel_btnpos;
int total_bpanel_btnpos;

xsw_bridge_win_struct bridge_win;
xsw_lg_mesg_win_struct lg_mesg_win;
xsw_qmenu_struct qmenu;
xsw_eco_win_struct eco_win;

dialog_win_struct err_dw;
dialog_win_struct info_dw;

fbrowser_struct pri_fbrowser;
int pri_fb_loadop;

xsw_image_struct **xsw_image;
int total_images;

xsw_pri_mesg_buf_struct pri_mesg_buf[MESG_WIN_TOTAL_MESSAGES];

qmenu_command_struct **qmenu_command;
int total_qmenu_commands;

isref_struct **isref;
int total_isrefs;

xsw_object_struct **xsw_object;
int total_objects;

xsw_object_struct **inrange_xsw_object;
int total_inrange_objects;

vs_object_label_struct **vs_object_label;  
int total_vs_object_labels;

#ifdef JS_SUPPORT
jsmap_struct **jsmap;
int total_jsmaps;
#endif	/* JS_SUPPORT */

xsw_gctl_struct gctl[1];
xsw_local_control_struct local_control;

xsw_warning_struct warning;

char **message_squelch;
int total_message_squelches;

sw_units_struct sw_units;
xsw_sector_legend_struct sector_legend;

scanner_contacts_struct **scanner_contact;   
int total_scanner_contacts;

ocsn_struct **ocsn;                                
int total_ocsns;

xsw_net_parms_struct net_parms;
xsw_loadstat_struct loadstat;

xsw_auto_interval_tune_struct auto_interval_tune;

serv_script_struct **serv_script;
int total_serv_scripts;   

ss_item_struct **ss_item;
int total_ss_items;



/*
 *	Segfault counter:
 */
static int segfault_count;



/*
 * 	Print debugging information.
 *
 *	This function originally writing to detect crashes
 *	and to report variable stats on segfault.  It's not
 *	very useful now however it is still implmented.
 */
void XSWDoPrintDebug()
{
	/* Print debug information. */
	fprintf(stderr,
 "%s %s debug report:\n",
	    PROG_NAME, PROG_VERSION
	);

	fprintf(stderr,
 "runlevel: %i  prompt_mode: %i  connection_state: %i\n",
	    (int)runlevel, (int)prompt_mode,
            (int)net_parms.connection_state
	);

	fprintf(stderr,
 "option.controller: %i  net interval: %i\n",
            (int)option.controller, (int)net_parms.net_int
        );

	fprintf(stderr,
 "player object: %i  total_objects: %i\n",
	    net_parms.player_obj_num, total_objects
	);

	/* Print debug footer. */
	fprintf(stderr, XSW_DEBUG_FOOTER);


	return;
}

/*
 *	Print usage of this program to stdout.
 */
void XSWDoHelp()
{
	printf(PROG_USAGE_MESSAGE);

	return;
}

/*
 *	Print version of this program to stdout.
 */
void XSWDoVersion()
{
	printf(
	    "%s Version %s\n%s\n",
	    PROG_NAME,
	    PROG_VERSION,
	    PROG_COPYRIGHT
	);
}

/*
 *	Prints basic client commands to message windows
 *	(not to stdout).
 */
void XSWDoHelpMesgWin()
{
	char text[512];


	sprintf(text,
"%s = Help  %s = Connect  %s = Disconnect  %s = Refresh\
  %s = Client Command  %s = Server Command",
		OSWGetKeyCodeName(xsw_keymap[XSW_KM_HELP].keycode),
		OSWGetKeyCodeName(xsw_keymap[XSW_KM_CONNECT].keycode),
                OSWGetKeyCodeName(xsw_keymap[XSW_KM_DISCONNECT].keycode),
                OSWGetKeyCodeName(xsw_keymap[XSW_KM_REFRESH].keycode),

                OSWGetKeyCodeName(xsw_keymap[XSW_KM_CLIENT_CMD].keycode),
		OSWGetKeyCodeName(xsw_keymap[XSW_KM_SERVER_CMD].keycode)
	);
	MesgAdd(text, xsw_color.bp_standard_text);

        sprintf(text,
"%s = Set Channel  %s = Send Hail  %s = Send Message  %s = Intercept\
  %s = Shield Frequency",
                OSWGetKeyCodeName(xsw_keymap[XSW_KM_SET_CHANNEL].keycode),
                OSWGetKeyCodeName(xsw_keymap[XSW_KM_HAIL].keycode),
                OSWGetKeyCodeName(xsw_keymap[XSW_KM_SEND_MESSAGE].keycode),
		OSWGetKeyCodeName(xsw_keymap[XSW_KM_SET_INTERCEPT].keycode),
                OSWGetKeyCodeName(xsw_keymap[XSW_KM_SHIELD_FREQ].keycode)
        );
        MesgAdd(text, xsw_color.bp_standard_text);

        sprintf(text,
"%s = Cycle Scan Lock  %s = Unlock Scanner  %s = Shields  %s = Fire Weapon\
  %s = Exit",
		OSWGetKeyCodeName(xsw_keymap[XSW_KM_WEAPONS_LOCK].keycode),
                OSWGetKeyCodeName(xsw_keymap[XSW_KM_WEAPONS_UNLOCK].keycode),
                OSWGetKeyCodeName(xsw_keymap[XSW_KM_SHIELD_STATE].keycode),
                OSWGetKeyCodeName(xsw_keymap[XSW_KM_FIRE_WEAPON].keycode),
                OSWGetKeyCodeName(xsw_keymap[XSW_KM_EXIT].keycode)
        );
        MesgAdd(text, xsw_color.bp_standard_text);


	return;
}

/*
 *	Procedure to check if socket s is valid.
 */
int XSWIsDescriptorValid(int s)
{
        struct timeval t;  
        fd_set writefds;


	if(s < 0)
	    return(0);

	/*   As far as we know, there is no standard way of
	 *   checking if a socket is simply valid.
	 *
	 *   So for now we're going to see if it's valid by checking
	 *   if its writeable.
	 */
        t.tv_sec = 0;
        t.tv_usec = 0;
        FD_ZERO(&writefds);
        FD_SET(s, &writefds);
        if(
            select(
                s + 1,
                NULL, &writefds, NULL,
                &t
            ) == -1
        )
            return(0);


        if(FD_ISSET(s, &writefds))
            return(1);
        else
            return(0);
}

/*
 *	Procedure to run the server.
 */
int XSWStartServer(char *cmd)
{
	pid_t p;
#ifdef __WIN32__
	p = Exec(".\\swserv.exe");
#else
	char *strptr;
	char cwd[PATH_MAX];
	char new_cwd[PATH_MAX];
	char text[PATH_MAX + NAME_MAX + 256];


	if(cmd == NULL)
	    return(-1);

	/* Path must be executeable. */
	if(!ISPATHEXECUTABLE(cmd))
	{
	    sprintf(text,
"Cannot execute:\n\n\
    %s\n\n\
Program or script is not set executeable.\n",
		cmd
	    );
	    printdw(&err_dw, text);

	    return(-1);
	}

	/* Path must be absolute. */
	if(!ISPATHABSOLUTE(cmd))
	{
            sprintf(text,
"Cannot execute:\n\n\
    %s\n\n\
Path not absolute.\n",
                cmd
            );
            printdw(&err_dw, text);

            return(-1);
	}

	/* ************************************************** */

	/* Record current working dir. */
	getcwd(cwd, PATH_MAX);
	cwd[PATH_MAX - 1] = '\0';

	/* Set new working dir. */
	strptr = GetParentDir(cmd);
	if(strptr == NULL)
	    return(-1);
	strncpy(new_cwd, strptr, PATH_MAX);
	new_cwd[PATH_MAX - 1] = '\0';
	chdir(new_cwd);


	/* Run server in background. */
	p = Exec(cmd);


	/* Change back to previous working dir. */
	chdir(cwd);

#endif	/* __WIN32__ */
	return(0);
}

/*
 *	Procedure to reload the Object Create Script Names from
 *	file.
 */
int XSWLoadOCSN(char *path)
{
        char *strptr;
	struct stat stat_buf;
	char tmp_path[PATH_MAX + NAME_MAX];


	if(path == NULL)
	    return(-1);

	if(ISPATHABSOLUTE(path))
	{
	    strncpy(tmp_path, path, PATH_MAX + NAME_MAX);
	}
	else
        {
	    strptr = PrefixPaths(dname.etc, path);
	    if(strptr == NULL)
		return(-1);

            strncpy(tmp_path, strptr, PATH_MAX + NAME_MAX);
	}
	tmp_path[PATH_MAX + NAME_MAX - 1] = '\0';


	/* Does file exist? */
	if(stat(tmp_path, &stat_buf))
	    return(-1);

        if(OCSLoadFromFile(tmp_path))
	    return(-1);

        strncpy(fname.ocsn, tmp_path, PATH_MAX + NAME_MAX);
	fname.ocsn[PATH_MAX + NAME_MAX - 1] = '\0';



	/* Updated resources that would be affected by this change. */

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


	return(0);
}

/*
 *      Procedure to reload the Image Set Referances from
 *      file.
 */
int XSWLoadIsrefs(char *path)
{
        char *strptr;
        char tmp_path[PATH_MAX + NAME_MAX];
        struct stat stat_buf;


        if(path == NULL)
            return(-1);

        if(ISPATHABSOLUTE(path))
        {
            strncpy(tmp_path, path, PATH_MAX + NAME_MAX);
        }
        else
        {
            strptr = PrefixPaths(dname.images, path); 
            if(strptr == NULL)
                return(-1);

            strncpy(tmp_path, strptr, PATH_MAX + NAME_MAX);
        }
        tmp_path[PATH_MAX + NAME_MAX - 1] = '\0';

        /* Does file exist? */
        if(stat(tmp_path, &stat_buf))
            return(-1);

        if(ISRefLoadFromFile(tmp_path))
	    return(-1);

        strncpy(fname.isr, tmp_path, PATH_MAX + NAME_MAX);
        fname.isr[PATH_MAX + NAME_MAX - 1] = '\0';


	return(0);
}

/*
 *	Instructs the sound server to load the new sound
 *	schemes file.
 */
int XSWLoadSS(char *path)
{
        char *strptr;
        char tmp_path[PATH_MAX + NAME_MAX];
        struct stat stat_buf;


        if(path == NULL)
            return(-1);

        if(ISPATHABSOLUTE(path))
        {
            strncpy(tmp_path, path, PATH_MAX + NAME_MAX);
        }
        else
        {
            strptr = PrefixPaths(dname.sounds, path);
            if(strptr == NULL)
                return(-1);

            strncpy(tmp_path, strptr, PATH_MAX + NAME_MAX);
        }
        tmp_path[PATH_MAX + NAME_MAX - 1] = '\0';

        /* Does file exist? */
        if(stat(tmp_path, &stat_buf))
            return(-1);

        if(SSLoadFromFile(tmp_path))
	    return(-1);

        strncpy(fname.sound_scheme, tmp_path, PATH_MAX + NAME_MAX);
        fname.sound_scheme[PATH_MAX + NAME_MAX - 1] = '\0';


	return(0);
}


/*
 *	Macro used by XSWLoadColors() to load colors.
 */
int XSW_LOAD_PIXEL_RGB(pixel_t *pix, WColorStruct c)
{
	return(OSWLoadPixelRGB(pix, c.r, c.g, c.b));
}

/*
 *      Procedure to load all GUI pixel colors used by this program.
 */
int XSWLoadColors()
{
/* Tempory xsw_color structure referance. */
#ifdef CS_TREF
#warning CS_TREF already defined, this is a problem!
#endif
#define CS_TREF xsw_color

	/* GUI must be connected before being able to load colors. */
	if(!IDC())
	    return(-1);


	/* Standard text colors. */
	XSW_LOAD_PIXEL_RGB(
	    &CS_TREF.standard_text,
	    CS_TREF.standard_text_cv
	);
        XSW_LOAD_PIXEL_RGB(
            &CS_TREF.bold_text,
            CS_TREF.bold_text_cv
        );
        XSW_LOAD_PIXEL_RGB(
            &CS_TREF.withdrawn_text,
            CS_TREF.withdrawn_text_cv
        );

        /* Bridge console panel readout colors. */
        XSW_LOAD_PIXEL_RGB(
            &CS_TREF.bp_standard_text,
            CS_TREF.bp_standard_text_cv
        );
        XSW_LOAD_PIXEL_RGB(
            &CS_TREF.bp_bold_text,
            CS_TREF.bp_bold_text_cv 
        );
        XSW_LOAD_PIXEL_RGB(
            &CS_TREF.bp_withdrawn_text,
            CS_TREF.bp_withdrawn_text_cv
        );

        XSW_LOAD_PIXEL_RGB(
            &CS_TREF.bp_light_outline,
            CS_TREF.bp_light_outline_cv
        );
        XSW_LOAD_PIXEL_RGB(
            &CS_TREF.bp_normal_outline,
            CS_TREF.bp_normal_outline_cv
        );
        XSW_LOAD_PIXEL_RGB(
            &CS_TREF.bp_dark_outline,
            CS_TREF.bp_dark_outline_cv
        );

        /* Warning, danger, and critical colors. */
        XSW_LOAD_PIXEL_RGB(
            &CS_TREF.bp_warning,
            CS_TREF.bp_warning_cv
        );
        XSW_LOAD_PIXEL_RGB(
            &CS_TREF.bp_danger,
            CS_TREF.bp_danger_cv
        );
        XSW_LOAD_PIXEL_RGB(
            &CS_TREF.bp_critical,
            CS_TREF.bp_critical_cv
        );

        /* Friendly, unknown, and hostile IFF colors. */
        XSW_LOAD_PIXEL_RGB(
            &CS_TREF.bp_friendly,
            CS_TREF.bp_friendly_cv
        );
        XSW_LOAD_PIXEL_RGB(
            &CS_TREF.bp_unknown,
            CS_TREF.bp_unknown_cv
        );
        XSW_LOAD_PIXEL_RGB(
            &CS_TREF.bp_hostile,
            CS_TREF.bp_hostile_cv
        );


        /* Bridge console panel readout outline colors. */
        XSW_LOAD_PIXEL_RGB(
            &CS_TREF.bpol_hull,
            CS_TREF.bpol_hull_cv
        );
        XSW_LOAD_PIXEL_RGB(
            &CS_TREF.bpol_power,
            CS_TREF.bpol_power_cv
        );
        XSW_LOAD_PIXEL_RGB(
            &CS_TREF.bpol_vis,
            CS_TREF.bpol_vis_cv
        );
        XSW_LOAD_PIXEL_RGB(
            &CS_TREF.bpol_shields,
            CS_TREF.bpol_shields_cv
        );
        XSW_LOAD_PIXEL_RGB(
            &CS_TREF.bpol_dmgctl,
            CS_TREF.bpol_dmgctl_cv
        );
        XSW_LOAD_PIXEL_RGB(
            &CS_TREF.bpol_throttle,
            CS_TREF.bpol_throttle_cv
        );
        XSW_LOAD_PIXEL_RGB(
            &CS_TREF.bpol_throttle_rev,
            CS_TREF.bpol_throttle_rev_cv
        );

        /* Scanner marks. */
        XSW_LOAD_PIXEL_RGB(
            &CS_TREF.scmark_unknown,
            CS_TREF.scmark_unknown_cv
        );
        XSW_LOAD_PIXEL_RGB(
            &CS_TREF.scmark_locked,
            CS_TREF.scmark_locked_cv 
        );
        XSW_LOAD_PIXEL_RGB(
            &CS_TREF.scmark_weapon,
            CS_TREF.scmark_weapon_cv
        );
        XSW_LOAD_PIXEL_RGB(
            &CS_TREF.scmark_home, 
            CS_TREF.scmark_home_cv
        );
        XSW_LOAD_PIXEL_RGB(
            &CS_TREF.scmark_area, 
            CS_TREF.scmark_area_cv
        );


	/* Keymap edit window. */
        XSW_LOAD_PIXEL_RGB(
            &CS_TREF.keymap_query_bg,
            CS_TREF.keymap_query_bg_cv
        );
        XSW_LOAD_PIXEL_RGB(
            &CS_TREF.keymap_query_fg,
            CS_TREF.keymap_query_fg_cv
        );

	/* Starchart. */
        XSW_LOAD_PIXEL_RGB(
            &CS_TREF.chart_bg,
            CS_TREF.chart_bg_cv
        );
        XSW_LOAD_PIXEL_RGB(
            &CS_TREF.chart_grid,
            CS_TREF.chart_grid_cv
        );
        XSW_LOAD_PIXEL_RGB(
            &CS_TREF.chart_sector_grid,
            CS_TREF.chart_sector_grid_cv
        );
        XSW_LOAD_PIXEL_RGB(
            &CS_TREF.chart_cross_hairs,
            CS_TREF.chart_cross_hairs_cv
        );


	return(0);
#undef CS_TREF
}

/*
 *	Procedure to free all GUI pixel colors used by this program.
 */
void XSWFreeColors()
{
	if(!IDC())
	    return;


	OSWDestroyPixel(&xsw_color.standard_text);
        OSWDestroyPixel(&xsw_color.bold_text);
        OSWDestroyPixel(&xsw_color.withdrawn_text);

        OSWDestroyPixel(&xsw_color.bp_standard_text);
        OSWDestroyPixel(&xsw_color.bp_bold_text);
        OSWDestroyPixel(&xsw_color.bp_withdrawn_text);

        OSWDestroyPixel(&xsw_color.bp_light_outline);
        OSWDestroyPixel(&xsw_color.bp_normal_outline);
        OSWDestroyPixel(&xsw_color.bp_dark_outline);

        OSWDestroyPixel(&xsw_color.bp_warning);
        OSWDestroyPixel(&xsw_color.bp_danger);
        OSWDestroyPixel(&xsw_color.bp_critical);

        OSWDestroyPixel(&xsw_color.bp_friendly);
        OSWDestroyPixel(&xsw_color.bp_unknown);
        OSWDestroyPixel(&xsw_color.bp_hostile);

        OSWDestroyPixel(&xsw_color.bpol_hull);
        OSWDestroyPixel(&xsw_color.bpol_power);
        OSWDestroyPixel(&xsw_color.bpol_vis);
        OSWDestroyPixel(&xsw_color.bpol_shields);
        OSWDestroyPixel(&xsw_color.bpol_dmgctl);
        OSWDestroyPixel(&xsw_color.bpol_throttle);
        OSWDestroyPixel(&xsw_color.bpol_throttle_rev);

        OSWDestroyPixel(&xsw_color.scmark_unknown);
        OSWDestroyPixel(&xsw_color.scmark_locked);
        OSWDestroyPixel(&xsw_color.scmark_weapon);
        OSWDestroyPixel(&xsw_color.scmark_home);
        OSWDestroyPixel(&xsw_color.scmark_area);

        OSWDestroyPixel(&xsw_color.keymap_query_bg);
        OSWDestroyPixel(&xsw_color.keymap_query_fg);

        OSWDestroyPixel(&xsw_color.chart_bg);
        OSWDestroyPixel(&xsw_color.chart_grid);
        OSWDestroyPixel(&xsw_color.chart_sector_grid);
        OSWDestroyPixel(&xsw_color.chart_cross_hairs);

	return;
}


/*
 *	Get dynamically allocated memory statistics used by
 *	this program.
 */
void XSWGetProgMemory(xsw_mem_stat_struct *buf)
{
	int i;
	long subtotal = 0;

	int events;
	xsw_image_struct **img_ptr;
	univ_entry_struct **uve_ptr;
	xsw_object_struct **obj_ptr;
	isref_struct **isref_ptr;
	ocsn_struct **ocsn_ptr;
	vs_object_label_struct **vsol_ptr;
	scanner_contacts_struct **sc_ptr;


	if(buf == NULL)
	    return;


	/* GUI Events. */
        if(IDC())
        {
	    events = OSWEventsPending();
            buf->gui = (events * sizeof(event_t *)) +
                (events * sizeof(event_t));
	}
	else
	{
	    buf->gui = 0;
	}
	subtotal += buf->gui;


        /* Sound events. */
        buf->sound = 0;
	subtotal += buf->sound;


	/* Program images. */
	for(buf->images = 0, i = 0, img_ptr = xsw_image;
            i < total_images;
            i++, img_ptr++
	)
	{
	    if(*img_ptr == NULL)
		continue;

	    if((*img_ptr)->filename != NULL)
		buf->images += strlen((*img_ptr)->filename) + 1;

	    if((*img_ptr)->image != NULL)
	    {
		buf->images += (
		    (*img_ptr)->image->width * (*img_ptr)->image->height *
		    MAX(osw_gui[0].depth >> 3, 1)
		);
	    }

	    buf->images += sizeof(xsw_image_struct);
	}
        buf->images += total_images * sizeof(xsw_image_struct *);

	subtotal += buf->images;


	/* Universe entries. */
        for(buf->univ_entries = 0, i = 0, uve_ptr = univ_entry;
            i < total_univ_entries;
            i++, uve_ptr++
	)
        {
	    if(*uve_ptr == NULL)
		continue;

            if((*uve_ptr)->alias != NULL)
            {
                buf->univ_entries += strlen((*uve_ptr)->alias) + 1;
            }
            if((*uve_ptr)->url != NULL)
            {
                buf->univ_entries += strlen((*uve_ptr)->url) + 1;
            }
            if((*uve_ptr)->comments != NULL)
            {
                buf->univ_entries += strlen((*uve_ptr)->comments) + 1;
            }
	    buf->univ_entries += sizeof(univ_entry_struct);
        }

	buf->univ_entries += total_univ_entries * sizeof(univ_entry_struct *);
        subtotal += buf->univ_entries;


	/* XSW Objects. */
        for(buf->objects = 0, i = 0, obj_ptr = xsw_object;
            i < total_objects;
	    i++, obj_ptr++
	)
        {
	    if(*obj_ptr == NULL)
		continue;

	    if((*obj_ptr)->elink != NULL)
		buf->objects += (strlen((*obj_ptr)->elink) + 1) *
		    sizeof(char);

	    if((*obj_ptr)->weapons != NULL)
	    {
		buf->objects += ((*obj_ptr)->total_weapons *
		    sizeof(xsw_weapons_struct *)) +
		    ((*obj_ptr)->total_weapons *
		    sizeof(xsw_weapons_struct));
	    }
	    if((*obj_ptr)->tractored_object != NULL)
	    {
		buf->objects += (*obj_ptr)->total_tractored_objects *
		    sizeof(int);
	    }
	    if((*obj_ptr)->score != NULL)
	    {
		buf->objects += sizeof(xsw_score_struct);
	    }
            if((*obj_ptr)->eco != NULL)
	    {
		buf->objects += sizeof(xsw_ecodata_struct);
	    }
	    buf->objects += sizeof(xsw_object_struct);
	}

        buf->objects += total_objects * sizeof(xsw_object_struct *);
        subtotal += buf->objects;


	/* Image set referances. */
        for(buf->isrefs = 0, i = 0, isref_ptr = isref;
            i < total_isrefs;
	    i++, isref_ptr++
	)
        {
	    if(*isref_ptr == NULL)
		continue;

	    if((*isref_ptr)->filename != NULL)
	    {
		buf->isrefs += strlen((*isref_ptr)->filename) + 1;
	    }
            if((*isref_ptr)->lib_data != NULL)
            {
		/* Library data not NULL implies image data is unique to
                 * this isref (not shared).
		 */
/* Need to calculate image data? */
		if((*isref_ptr)->image_data != NULL)
		    buf->isrefs += (
			(*isref_ptr)->width * (*isref_ptr)->height * 
		        MAX(osw_gui[0].depth >> 3, 1)
		    );
	    }
            if((*isref_ptr)->point_light != NULL)
            {
                buf->isrefs += ((*isref_ptr)->total_point_lights *
                    sizeof(isref_point_light_struct *)) +
                    ((*isref_ptr)->total_point_lights *
                    sizeof(isref_point_light_struct));
            }

	    buf->isrefs += sizeof(isref_struct);
	}
	buf->isrefs += total_isrefs * sizeof(isref_struct *);
        subtotal += buf->isrefs;


        /* Object create script names. */
        for(buf->ocsns = 0, i = 0, ocsn_ptr = ocsn;
            i < total_ocsns;
            i++, ocsn_ptr++
        )
	{
	    if(*ocsn_ptr == NULL)
		continue;

	    buf->ocsns += sizeof(ocsn_struct);
	}
        buf->ocsns += total_ocsns * sizeof(ocsn_struct *);
        subtotal += buf->ocsns;


	/* Viewscreen object labels. */
	for(buf->vs_labels = 0, i = 0, vsol_ptr = vs_object_label;
            i < total_vs_object_labels;
            i++, vsol_ptr++
	)
	{
	    if(*vsol_ptr == NULL)
		continue;

	    if((*vsol_ptr)->image != NULL)
	    {
                buf->vs_labels += (
		    (*vsol_ptr)->image->width * (*vsol_ptr)->image->height *
                    MAX(osw_gui[0].depth >> 3, 1)
		);
	    }
            buf->vs_labels += sizeof(vs_object_label_struct);
	}
	buf->vs_labels += total_vs_object_labels *
	    sizeof(vs_object_label_struct *);
	subtotal += buf->vs_labels;


	/* Scanner contacts. */
        for(buf->scanner_contacts = 0, i = 0, sc_ptr = scanner_contact;
            i < total_scanner_contacts;
            i++, sc_ptr++
	)
        {
            if(*sc_ptr == NULL)
		continue;

	    buf->scanner_contacts += sizeof(scanner_contacts_struct);
	}
        buf->scanner_contacts += total_scanner_contacts *
            sizeof(scanner_contacts_struct *);
	subtotal += buf->scanner_contacts;


	/* Total. */
	buf->total = subtotal;


	return;
}


/*
 *	Procedure to reclaim globally allocated memory.
 */
void XSWReclaimGlobalMemory(bool_t verbose)
{
        if(verbose) 
            MesgAdd(
                "Client: Reclaiming memory...",
                xsw_color.standard_text
            );

	DBReclaim();
	ISRefReclaimMemory();
	OCSReclaimMemory();
	VSLabelReclaimMemory();
	DBInRangeUpdate(net_parms.player_obj_num);

	if(verbose)
            MesgAdd(
		"Client: Memory reclaim complete.",
		xsw_color.standard_text
	    );

        /* Mark last time memory was reclaimed. */
        next.memory_clean = cur_millitime + MEMORY_CLEAN_INTERVAL;


	return;
}


/*
 *	Procedure to set all XSW toplevel windows out of focus.
 */
void XSWDoUnfocusAllWindows()
{
	bridge_win.is_in_focus = 0;
	lg_mesg_win.is_in_focus = 0;
	splash_win.is_in_focus = 0;
	univ_list_win.is_in_focus = 0;
	univ_edit_win.is_in_focus = 0;
	options_win.is_in_focus = 0;
	keymap_win.is_in_focus = 0;
#ifdef JS_SUPPORT
	jsmap_win.is_in_focus = 0;
#endif	/* JS_SUPPORT */
	eco_win.is_in_focus = 0;
	starchart_win.is_in_focus = 0;

	/* Quick menu should be skipped. */

	err_dw.is_in_focus = 0;
	info_dw.is_in_focus = 0;
	pri_fbrowser.is_in_focus = 0;


	return;
}


/*
 *      Procedure to restack all XSW windows.
 */
void XSWDoRestackWindows()
{
/*
	const int num_w = 8;
 */
	win_t w[8];

	w[0] = splash_win.toplevel;
	w[1] = eco_win.toplevel;
	w[2] = keymap_win.toplevel;
	w[3] = options_win.toplevel;
	w[4] = univ_edit_win.toplevel;
	w[5] = univ_list_win.toplevel;
	w[6] = lg_mesg_win.toplevel;
	w[7] = bridge_win.toplevel;

/*
	OSWRestackWindows(w, num_w);
 */

	return;
}


/*
 *	Manage GUI events.
 */
void XSWManageGUI()
{
	char need_continue;
	int events_handled;
	int total_events_handled = 0;
	event_t event;
	keycode_t keycode;



	/* Manage GUI events. */
	while(OSWEventsPending() > 0)
	{
	    need_continue = 0;
	    events_handled = 0;

	    /* Fetch new event. */
	    OSWWaitNextEvent(&event);


	    /*   Skip keyboard keys for game controller if bridge window
             *   is in focus and controller type is set to keyboard.
             */
	    if(1)
            {
		/* Is bridge window in focus, event is a key press or
                 * release and is not in any prompt mode?
		 */
		if(bridge_win.is_in_focus &&
                   (prompt_mode == PROMPT_CODE_NONE) &&
                   ((event.type == KeyPress) ||
                    (event.type == KeyRelease)
                   )
		)
		{
		    keycode = event.xkey.keycode;

		    /* Is the key code any of the game controller keys. */
		    if((keycode == xsw_keymap[XSW_KM_TURN_LEFT].keycode) ||
                       (keycode == xsw_keymap[XSW_KM_TURN_RIGHT].keycode) ||
                       (keycode == xsw_keymap[XSW_KM_THROTTLE_INC].keycode) ||
                       (keycode == xsw_keymap[XSW_KM_THROTTLE_DEC].keycode) ||
                       (keycode == xsw_keymap[XSW_KM_THROTTLE_IDLE].keycode) ||
                       (keycode == xsw_keymap[XSW_KM_FIRE_WEAPON].keycode) ||
                       (keycode == xsw_keymap[XSW_KM_OMNI_DIR_THRUST].keycode) ||
                       (keycode == xsw_keymap[XSW_KM_EXTERNAL_DAMPERS].keycode)
                    )
		    {
			/*   Put this event back so that next call to
                         *   GCtlUpdate() recieves this event.
			 */
                        OSWPutBackEvent(&event);
                        break;
		    }
		}
	    }


	    /* Handle event by type. */
	    switch(event.type)
	    {
              /* ***************************************************** */
              case FocusIn:
                /*   Take all XSW created windows out of focus,
                 *   management functions will see which window comes
		 *   into focus.
		 */
                XSWDoUnfocusAllWindows();
		break;

              /* ***************************************************** */
	      case ClientMessage:
                /*  This is needed since after a ClientMessage event
                 *  certain Windows report eronious parents for
                 *  XQueryTree();
                 */
/* Let's try it without this check just for now, maybe we can get
   away with it.

                OSWPurgeAllEvents();
 */
 		break;
	    }

	    if(need_continue)
		continue;

	    /* ******************************************************** */

	    /* Bridge window. */
	    if(events_handled <= 0)
	        events_handled += BridgeManage(&event);

            /* Quick menu. */
	    if(events_handled <= 0)
                events_handled += QMenuManage(&event);

            /* Large message win. */
	    if(events_handled <= 0)
                events_handled += LMesgWinManage(&event);

            /* Universe list window. */
            if(events_handled <= 0)
                events_handled += UnivListManage(&event);

	    /* Universe edit window. */
            if(events_handled <= 0)
                events_handled += UnivEditWinManage(&event);

	    /* Economy window. */
            if(events_handled <= 0)
                events_handled += EcoWinManage(&event);

            /* Options window. */
            if(events_handled <= 0)    
                events_handled += OptWinManage(&event);

            /* Key mapping window. */
            if(events_handled <= 0)
                events_handled += KeymapWinManage(&event);

#ifdef JS_SUPPORT
	    /* Joystick mapping window. */
            if(events_handled <= 0)
                events_handled += JSMWManage(&event);
#endif	/* JS_SUPPORT */

	    /* Starchart window. */
	    if(events_handled <= 0)
                events_handled += SChtManage(&starchart_win, &event);

            /* Error dialog window. */
            if(events_handled <= 0)
                events_handled += DialogWinManage(&err_dw, &event);

            /* Info dialog window. */
            if(events_handled <= 0)
                events_handled += DialogWinManage(&info_dw, &event);

            /* Primary file browser. */
            if(events_handled <= 0)
                events_handled += FBrowserManage(&pri_fbrowser, &event);




	    /* Allow widget event manager to see event. */
	    WidgetManage(&event);


	    /* Add up total events handled so far. */
	    total_events_handled += events_handled;
	}


        /* ******************************************************** */
        /* Allow widget set to manage itself once per loop. */
        WidgetManage(NULL);


	return;
}

/*
 *	Manage sound server events.
 */
void XSWManageSound()
{
	int total_events_handled = 0;


	/* Manage sound events. */
        total_events_handled += SoundManageEvents();


	return;
}

/*
 *	Procedure to update background music, this should be called
 *	whenever the background music *may* need changing.
 */
void XSWDoChangeBackgroundMusic()
{
	int code = SOUND_CODE_BKG_STANDARD;
	xsw_object_struct *obj_ptr, *tar_obj_ptr;


	/* Check if main menu on bridge window is mapped. */
	if((bridge_win.cur_page == NULL) ? 0 : bridge_win.cur_page->map_state)
	{
	    /* Main menu on bridge window is mapped. */

	    code = SOUND_CODE_BKG_MAINMENU;
	}
	else
	{
	    /* View screen is mapped. */

	    code = SOUND_CODE_BKG_STANDARD;

	    if(net_parms.player_obj_ptr == NULL)
	    {
		/* Player object not valid, could be still logging in. */

	    }
	    else
	    {
		/* Player object is valid. */

		/* Get pointer to player object. */
		obj_ptr = net_parms.player_obj_ptr;

                /* Check if in nebula. */
                if(obj_ptr->loc_type == XSW_LOC_TYPE_NEBULA)
                    code = SOUND_CODE_BKG_MYSTY;

		/* Check if locked onto something. */
		if(DBIsObjectGarbage(obj_ptr->locked_on))
		{
		    /* Player object is not locked on anything. */

		}
		else
		{
		    /* Player object is locked on something. */
		    tar_obj_ptr = xsw_object[obj_ptr->locked_on];

		    /* Check if locked on target is controlled,
                     * weapon, or player.
		     */
		    if((tar_obj_ptr->type == XSW_OBJ_TYPE_CONTROLLED) ||
		       (tar_obj_ptr->type == XSW_OBJ_TYPE_PLAYER) ||
                       (tar_obj_ptr->type == XSW_OBJ_TYPE_WEAPON) ||
                       (tar_obj_ptr->type == XSW_OBJ_TYPE_STREAMWEAPON) ||
                       (tar_obj_ptr->type == XSW_OBJ_TYPE_SPHEREWEAPON)
		    )
		        code = SOUND_CODE_BKG_FIGHTING;


		}
	    }
	}

	/* Change background music (as needed). */
	if(code != sound.bkg_mood_code)
	    SoundChangeBackgroundMusic(code, 0, 0);


	return;
}

/*
 *	Widget scroll bar callback handler.
 */
int XSWScrollBarCB(scroll_bar_struct *ptr)
{
	if(ptr == NULL)
	    return(-1);


	/* Check which windows need to be redrawn due to scroll
	 * bar value change.
	 */
	if(ptr == &bridge_win.mesg_box_sb)
	    BridgeDrawMessages();
	else if(ptr == &lg_mesg_win.scroll_bar)
	    LMesgWinDraw();
	else if(ptr == &univ_list_win.list.sb)
	    ListWinDraw(&univ_list_win.list);


	return(0);
}



/*
 *	Signal handler.
 */
void XSWHandleSignal(int s)
{

	switch(s)
	{
	  /* ****************************************************** */
	  case SIGHUP:
	    signal(SIGHUP, XSWHandleSignal);
            NetResetParms();
            XSWDoDisconnect();
            break;

          /* ****************************************************** */
          case SIGINT:
	    runlevel = 1;
	    break;

          /* ****************************************************** */
          case SIGQUIT:
            runlevel = 1;
            break;

          /* ****************************************************** */
          case SIGABRT:
            runlevel = 1;
            break;

          /* ****************************************************** */
          case SIGKILL:
            runlevel = 1;
            break;

          /* ****************************************************** */
          case SIGPIPE:
	    signal(SIGPIPE, XSWHandleSignal);

	    /* Check which socket was lost. */

	    /* Connection to universe socket. */
	    if(net_parms.socket > -1)
	    {
		if(!XSWIsDescriptorValid(net_parms.socket))
		{
		    NetResetParms();
		    XSWDoDisconnect();
		    MesgAdd(
               "Got SIGPIPE: Connection to universe broken.",
			xsw_color.bp_bold_text
		    );
		}
	    }
/* Connection to sound server. */
            break;

          /* ****************************************************** */
          case SIGSEGV:
            /* Increment segfault counter. */
            segfault_count++;
            if(segfault_count >= 2)
                exit(-1);

	    /* Do debug procedure. */
	    XSWDoPrintDebug();

	    runlevel = 1;
            break;

          /* ****************************************************** */
	  case SIGTERM:
            runlevel = 1;
            break;

          /* ****************************************************** */
          case SIGSTOP:
            signal(SIGSTOP, XSWHandleSignal);
            runlevel = 1;
            break;

          /* ****************************************************** */
          case SIGCONT:
            signal(SIGCONT, XSWHandleSignal);
	    break;

          /* ****************************************************** */
          default:
            fprintf(stderr,
		"Got unknown signal %i, no response performed.\n",
		s
	    );
            break;
	}


	return;
}



/*
 *	Resets all global timmers.
 */
void XSWDoResetTimmers()
{
	int i, n;
	xsw_object_struct **obj_ptr;
	isref_struct **isref_ptr;


	/* Global event and task timmers. */
	memset(&next, 0x00, sizeof(xsw_next_struct));


	/* Bridge window viewscreen redraw interval.
         * This may be need to be reset because a non reset current
	 * time calculation may set this value too high.
	 */
	bridge_win.viewscreen_int = 0;


        /* Global animation timmers. */
        for(i = 0; i < MAX_ANIM_TIMMERS; i++)
            genanim_timmer[i].next = 0;


        /* Auto interval tunning. */
        auto_interval_tune.next = 0;

        /* FPS timmer. */
        fps_counter.fcount = 0;
        fps_counter.lfcount = 0;
        fps_counter.next = 0;   


	/* Timmers on XSW Objects. */
        for(i = 0, obj_ptr = xsw_object;
            i < total_objects;
            i++, obj_ptr++
	)
        {
	    if(*obj_ptr == NULL)
		continue;

            if((*obj_ptr)->type <= XSW_OBJ_TYPE_GARBAGE)
                continue;

	    /* Last network update. */
            (*obj_ptr)->last_updated = 0;

	    /* Birth time, since this is in milliseconds midnight
	     * we need this to be reset too.  But it may cause certain
	     * objects to live a bit longer.
	     */
	    (*obj_ptr)->birth_time = 0;

	    /* Last animation frame. */
	    (*obj_ptr)->animation.last_interval = 0;

	    /* Weapon last use. */
            for(n = 0; n < (*obj_ptr)->total_weapons; n++)
            {
		if((*obj_ptr)->weapons[n] == NULL)
		    continue;

                (*obj_ptr)->weapons[n]->last_used = 0;
            }
        }


	/* Timmers on isrefs. */
	for(i = 0, isref_ptr = isref;
            i < total_isrefs;
            i++, isref_ptr++
	)
	{
	    if(*isref_ptr == NULL)
		continue;

	    /* Strobe timmer on point lights. */
	    for(n = 0; n < (*isref_ptr)->total_point_lights; n++)
	    {
		if((*isref_ptr)->point_light[n] == NULL)
		    continue;

		(*isref_ptr)->point_light[n]->strobe_next = 0;
	    }
	}



	return;
}

/*
 *	Initializes all resources for this program.
 *
 *	This function should be called only once, because it
 *	resets certain global pointer variables that are assumed
 *	to never have been allocated.
 */
int XSWInit(int argc, char *argv[])
{
	int i, n, x, y, z;
	bool_t b;
	char *strptr;
	char stringa[MAX_URL_LEN + 512];
	char cwd[PATH_MAX];
	char home_dir[PATH_MAX];
	char local_sw_dir[PATH_MAX];
        struct stat stat_buf;
        WColorStruct color;


        /* First thing to do on initialization is to reset the
         * segfault counter.
         */
        segfault_count = 0;


	/* Initialize time zone. */
        tzset();

#ifdef __WIN32__
	/* Change working dir to be the user's home dir. */
	chdir(CWD_STR);

	/* Get current, home and local shipwars directories. */
	strcpy(cwd, CWD_STR);
	strcpy(home_dir, CWD_STR);
	strcpy(local_sw_dir, CWD_STR);
#else
	/* Change working dir to be the user's home dir. */
	strptr = getenv("HOME");
	if(strptr != NULL)
	    chdir(strptr);

	/* Get current, home and local shipwars directories. */
        getcwd(cwd, PATH_MAX);
        cwd[PATH_MAX - 1] = '\0';

	strptr = getenv("HOME");
	strncpy(home_dir, ((strptr == NULL) ? cwd : strptr), PATH_MAX);
	home_dir[PATH_MAX - 1] = '\0';

	if(ISPATHABSOLUTE(DEF_LOCAL_SHIPWARS_DIR))
	{
	    strncpy(local_sw_dir, DEF_LOCAL_SHIPWARS_DIR, PATH_MAX);
	}
	else
	{
	    strptr = PrefixPaths(home_dir, DEF_LOCAL_SHIPWARS_DIR);
	    strncpy(
		local_sw_dir,
		((strptr == NULL) ? home_dir : strptr),
		PATH_MAX
	    );
	}
	local_sw_dir[PATH_MAX - 1] = '\0';
#endif	/* __WIN32__ */


	/* Reset/fetch global variables to their default values. */
	root_pid = getpid();

	debug.level = DEBUG_LEVEL_NONE;
	debug.val = 0;

	cur_millitime = MilliTime();
	cur_systime = time(NULL);
	lapsed_millitime = 0;

	time_compensation = 1.0;

	next.update_check = cur_millitime;
	next.viewscreen = cur_millitime;
        next.consoles = cur_millitime;
        next.memory_clean = cur_millitime + MEMORY_CLEAN_INTERVAL;
        next.lplayer_pos_send = cur_millitime;

	prompt_mode = PROMPT_CODE_NONE;

	strncpy(dname.startup, cwd, PATH_MAX);
        dname.startup[PATH_MAX - 1] = '\0';
	strncpy(dname.home, home_dir, PATH_MAX);
        dname.home[PATH_MAX - 1] = '\0';

	strptr = PrefixPaths(home_dir, DEF_LOCAL_SHIPWARS_DIR);
        strncpy(
	    dname.ltoplevel,
	    ((strptr == NULL) ? cwd : strptr),
	    PATH_MAX
	);
	dname.ltoplevel[PATH_MAX - 1] = '\0';

	strncpy(dname.toplevel, DEF_XSW_TOPLEVEL_DIR, PATH_MAX);
	dname.toplevel[PATH_MAX - 1] = '\0';
        strncpy(dname.etc, DEF_XSW_ETC_DIR, PATH_MAX);
        dname.etc[PATH_MAX - 1] = '\0';
	strncpy(dname.images, DEF_XSW_IMAGES_DIR, PATH_MAX);
        dname.images[PATH_MAX - 1] = '\0';
	strncpy(dname.sounds, DEF_XSW_SOUNDS_DIR, PATH_MAX);
	dname.sounds[PATH_MAX - 1] = '\0';
	strncpy(dname.downloads, home_dir, PATH_MAX);
        dname.downloads[PATH_MAX - 1] = '\0';
        strncpy(dname.starchart, local_sw_dir, PATH_MAX);
        dname.starchart[PATH_MAX - 1] = '\0';

	fps_counter.fcount = 0;
	fps_counter.lfcount = 0;
	fps_counter.interval = 1000;	/* In milliseconds. */
	fps_counter.next = cur_millitime + fps_counter.interval;

	xsw_font.std = NULL;
	xsw_font.std_bold = NULL;
	xsw_font.console_heading = NULL;
	xsw_font.console_standard = NULL;
	xsw_font.console_message = NULL;

	/* Check if keymap structures are consistant in size. */
	if(1)
	{
	    const char *xsw_keymap_alias[] = XSW_KEYMAP_ALIAS;
	    const char *keymap_item_desc[] = XSW_KEYMAP_ITEM_DESC;

	    if((TOTAL_XSW_KEYMAPS !=
		(sizeof(xsw_keymap_alias) / sizeof(char *))) ||
	       (TOTAL_XSW_KEYMAPS !=
		(sizeof(keymap_item_desc) / sizeof(char *)))
	    )
	    {
		fprintf(stderr, "Keymap structure sizes inconsistant.\n");
		return(-1);
	    }
	}

	option.rc_version_major = PROG_VERSION_MAJOR;
	option.rc_version_minor = PROG_VERSION_MINOR;

	option.units = XSW_UNITS_XSW;
	option.async_image_loading = 1;
	option.async_image_pixels = 500000;

	option.def_scanner_orient = SCANNER_ORIENT_LOCAL;

	option.log_client = 0;
	option.log_net = 0;
	option.log_errors = 0;

	option.async_redraws = 1;
	option.show_viewscreen_marks = 1;
	option.show_viewscreen_labels = 3;
        option.show_lens_flares = 1;
        option.show_strobe_glow = 1;
        option.show_nebula_glow = 1;
	option.show_formal_label = 1;
	option.show_net_errors = 0;
        option.show_server_errors = 1;
        option.local_updates = 1;
        option.auto_zoom = 1;
	option.sounds = XSW_SOUNDS_NONE;
	option.music = 0;
	option.display_events = 1;
        option.controller = CONTROLLER_KEYBOARD;
	option.cmd_line_set_controller = 0;
        option.throttle_mode = THROTTLE_MODE_NORMAL;
#ifdef JS_SUPPORT
	option.focus_out_js_close = 0;
#endif /* JS_SUPPORT */
	option.energy_saver_mode = 0;
	option.scanner_limiting = 1;
	option.notify_scanner_contacts = 1;

	option.auto_map_eco_win = 0;
	option.auto_map_univ_list_win = 1;
	option.clear_chart_on_connect = 1;
	option.save_on_exit = 1;

#ifdef HAVE_Y2
        sound.server_type = SNDSERV_TYPE_YIFF;
#else
	sound.server_type = SNDSERV_TYPE_NONE;
#endif	/* HAVE_Y2 */

	strncpy(
	    sound.start_cmd,
	    DEF_SOUND_SERVER_START_CMD,
	    PATH_MAX + NAME_MAX
	);
	sound.start_cmd[PATH_MAX + NAME_MAX - 1] = '\0';
        strncpy(
            sound.con_arg,
            DEF_SOUND_SERVER_CONNECT_ARG,
            MAX_URL_LEN
        );
        sound.con_arg[MAX_URL_LEN - 1] = '\0';
	sound.con_data = NULL;

	local_control.weapons_online = 1;
	local_control.weapon_freq = 160.00;
	local_control.weapon_yield = 1.0;
	local_control.weapon_fire_heading = 0.0;
	local_control.weapon_fire_pitch = 0.0;

	warning.weapons_lock = 0;
	warning.incoming_fire = 0;

	sw_units.ru_to_au = DEF_UNITCONV_RU_TO_AU;

	sector_legend.x_max = SECTOR_SIZE_X_MAX;
        sector_legend.x_min = SECTOR_SIZE_X_MIN;
        sector_legend.y_max = SECTOR_SIZE_Y_MAX;
        sector_legend.y_min = SECTOR_SIZE_Y_MIN;
        sector_legend.z_max = SECTOR_SIZE_Z_MAX;
        sector_legend.z_min = SECTOR_SIZE_Z_MIN;
	sector_legend.x_len = sector_legend.x_max - sector_legend.x_min;
        sector_legend.y_len = sector_legend.y_max - sector_legend.y_min;
        sector_legend.z_len = sector_legend.z_max - sector_legend.z_min;

	bpanel_btnpos = NULL;
	total_bpanel_btnpos = 0;

	message_squelch = NULL;
	total_message_squelches = 0;

	scanner_contact = NULL;
	total_scanner_contacts = 0;

	qmenu_command = NULL;
	total_qmenu_commands = 0;

	vs_object_label = NULL;
	total_vs_object_labels = 0;

	ocsn = NULL;
	total_ocsns = 0;

	xsw_object = NULL;
	total_objects = 0;

	inrange_xsw_object = NULL;
	total_inrange_objects = 0;

	isref = NULL;
	total_isrefs = 0;

	xsw_image = NULL;
	total_images = 0;

	serv_script = NULL;
	total_serv_scripts = 0;

	ss_item = NULL;
	total_ss_items = 0;

#ifdef JS_SUPPORT
	jsmap = NULL;
	total_jsmaps = 0;
#endif	/* JS_SUPPORT */

	/* Reset player object to none (-1). */
	DBSetPlayerObject(-1);


	/* Set default file names. */
        strncpy(
            fname.rc,
            PrefixPaths(local_sw_dir, DEF_XSW_RCFILE),
            PATH_MAX + NAME_MAX
        );
        fname.rc[PATH_MAX + NAME_MAX - 1] = '\0';
	strncpy(
	    fname.universe_list,
	    PrefixPaths(local_sw_dir, DEF_UNIVERSES_FILE),
            PATH_MAX + NAME_MAX
        );
        fname.universe_list[PATH_MAX + NAME_MAX - 1] = '\0';

#ifdef JS_SUPPORT
	strncpy(
	    fname.js_calib,
	    PrefixPaths(home_dir, DEF_JS_CALIBRATION_FILE),
	    PATH_MAX + NAME_MAX
	);
	fname.js_calib[PATH_MAX + NAME_MAX - 1] = '\0';
#endif /* JS_SUPPORT */

	strncpy(
            fname.main_page,
            PrefixPaths(cwd, DEF_MAIN_PAGE_FILE),
            PATH_MAX + NAME_MAX
	);
	fname.main_page[PATH_MAX + NAME_MAX - 1] = '\0';

        strncpy(
            fname.destroyed_page,
            PrefixPaths(cwd, DEF_DESTROYED_PAGE_FILE),
            PATH_MAX + NAME_MAX
        );
        fname.destroyed_page[PATH_MAX + NAME_MAX - 1] = '\0';

	strncpy(
            fname.ocsn,
            PrefixPaths(cwd, DEF_XSW_OCSN_FILE),
	    PATH_MAX + NAME_MAX
        );
	fname.ocsn[PATH_MAX + NAME_MAX - 1] = '\0';

	strncpy(
            fname.isr,
            PrefixPaths(cwd, DEF_XSW_ISREF_FILE),
	    PATH_MAX + NAME_MAX
        );
	fname.isr[PATH_MAX + NAME_MAX - 1] = '\0';

        strncpy(
            fname.sound_scheme,
            PrefixPaths(cwd, DEF_XSW_SOUND_SCHEME_FILE),   
            PATH_MAX + NAME_MAX
        );
        fname.sound_scheme[PATH_MAX + NAME_MAX - 1] = '\0';

        strncpy(
            fname.log,
            PrefixPaths(local_sw_dir, DEF_XSW_LOG_FILE),
            PATH_MAX + NAME_MAX
        );
        fname.log[PATH_MAX + NAME_MAX - 1] = '\0';


	net_parms.connection_state = CON_STATE_NOT_CONNECTED;
	net_parms.socket = -1;

	strptr = getenv("LOGNAME");
	if(strptr != NULL)
	    strncpy(net_parms.login_name, strptr, XSW_OBJ_NAME_MAX);
	else
	    memset(net_parms.login_name, '\0', XSW_OBJ_NAME_MAX);
	net_parms.login_name[XSW_OBJ_NAME_MAX - 1] = '\0';

	memset(net_parms.login_password, '\0', XSW_OBJ_PASSWORD_MAX);

        net_parms.is_address_set = 0;
	net_parms.con_start = 0;
        net_parms.warn15 = 0;
        net_parms.warn30 = 0;
        net_parms.warn45 = 0;
	net_parms.login_got_lplayer = 0;
        net_parms.login_got_position = 0;
        net_parms.login_got_sector = 0;

	strncpy(net_parms.address, DEF_SWSERV_ADDRESS, MAX_URL_LEN);
	net_parms.address[MAX_URL_LEN - 1] = '\0';

	net_parms.port = DEF_SWSERV_PORT;

        net_parms.net_int = SERVER_DEF_INT;
        net_parms.disconnect_send_count = 0;
	net_parms.bad_send_count = 0;

	memset(net_parms.co_data, 0x00, CS_DATA_MAX_LEN);
	net_parms.co_data_len = 0;


	univ_entry = NULL;
	total_univ_entries = 0;


	loadstat.net_load_max = DEF_NET_LOAD_MAX;
	loadstat.rx_interval = 0;
	loadstat.sx_interval = 0;
        loadstat.rx_ilast = 0;
        loadstat.sx_ilast = 0;

	auto_interval_tune.state = 1;
	auto_interval_tune.interval = AINT_DEF_TUNE_INT;
	auto_interval_tune.next = cur_millitime +
	    auto_interval_tune.interval;


	/* Set up global color values. */
	xsw_color.heading_arrow.a = 0x00;
        xsw_color.heading_arrow.r = 0x00;
        xsw_color.heading_arrow.g = 0xf0;
        xsw_color.heading_arrow.b = 0xff;

        xsw_color.lock_arrow.a = 0x00;
        xsw_color.lock_arrow.r = 0xff;
        xsw_color.lock_arrow.g = 0xff;
        xsw_color.lock_arrow.b = 0x10;

        xsw_color.lock_cursor.a = 0x00;
        xsw_color.lock_cursor.r = 0xff;
        xsw_color.lock_cursor.g = 0xff;
        xsw_color.lock_cursor.b = 0x10;

	xsw_color.stream_green.a = 0x00;
        xsw_color.stream_green.r = 0x00;
        xsw_color.stream_green.g = 0xff;
        xsw_color.stream_green.b = 0x78;

        xsw_color.stream_yellow.a = 0x00;
        xsw_color.stream_yellow.r = 0xff;
        xsw_color.stream_yellow.g = 0xf4;
        xsw_color.stream_yellow.b = 0x80;

	xsw_color.stream_orange.a = 0x00;   
        xsw_color.stream_orange.r = 0xff;
        xsw_color.stream_orange.g = 0x90;
        xsw_color.stream_orange.b = 0x30;

        xsw_color.stream_purple.a = 0x00;
        xsw_color.stream_purple.r = 0xff;
        xsw_color.stream_purple.g = 0x70;
        xsw_color.stream_purple.b = 0xff;

        xsw_color.shield_blue.a = 0x00;
        xsw_color.shield_blue.r = 0xa0;
        xsw_color.shield_blue.g = 0xe0;
        xsw_color.shield_blue.b = 0xff;

        xsw_color.star_glow.a = 0x00;
        xsw_color.star_glow.r = 0xff;
        xsw_color.star_glow.g = 0xff;
        xsw_color.star_glow.b = 0xff;


	/* Initialize universe object management. */
	UNVInit(argc, argv);


	/* Setup global animation timmer values. */
        genanim_timmer[ANIM_TIMMER_GENERAL].count = 0;
        genanim_timmer[ANIM_TIMMER_GENERAL].count_max =
	    ANIM_TIMMER_MAXCOUNT_GENERAL;
        genanim_timmer[ANIM_TIMMER_GENERAL].next = cur_millitime;
        genanim_timmer[ANIM_TIMMER_GENERAL].interval = 250;

        genanim_timmer[ANIM_TIMMER_SHORTGLOW].count = 0;
        genanim_timmer[ANIM_TIMMER_SHORTGLOW].count_max =
            ANIM_TIMMER_MAXCOUNT_SHORTGLOW;
        genanim_timmer[ANIM_TIMMER_SHORTGLOW].next = cur_millitime;
        genanim_timmer[ANIM_TIMMER_SHORTGLOW].interval = 125;

        genanim_timmer[ANIM_TIMMER_MEDIUMGLOW].count = 0;
        genanim_timmer[ANIM_TIMMER_MEDIUMGLOW].count_max =
            ANIM_TIMMER_MAXCOUNT_MEDIUMGLOW;
        genanim_timmer[ANIM_TIMMER_MEDIUMGLOW].next = cur_millitime;
        genanim_timmer[ANIM_TIMMER_MEDIUMGLOW].interval = 250;

        genanim_timmer[ANIM_TIMMER_LONGGLOW].count = 0;
        genanim_timmer[ANIM_TIMMER_LONGGLOW].count_max =
            ANIM_TIMMER_MAXCOUNT_LONGGLOW;
        genanim_timmer[ANIM_TIMMER_LONGGLOW].next = cur_millitime;
        genanim_timmer[ANIM_TIMMER_LONGGLOW].interval = 250;

        genanim_timmer[ANIM_TIMMER_SHORTBLINK].count = 0;
        genanim_timmer[ANIM_TIMMER_SHORTBLINK].count_max =
            ANIM_TIMMER_MAXCOUNT_SHORTBLINK;
        genanim_timmer[ANIM_TIMMER_SHORTBLINK].next = cur_millitime;
        genanim_timmer[ANIM_TIMMER_SHORTBLINK].interval = 300;

        genanim_timmer[ANIM_TIMMER_LONGBLINK].count = 0;
        genanim_timmer[ANIM_TIMMER_LONGBLINK].count_max =
            ANIM_TIMMER_MAXCOUNT_LONGBLINK;
        genanim_timmer[ANIM_TIMMER_LONGBLINK].next = cur_millitime;
        genanim_timmer[ANIM_TIMMER_LONGBLINK].interval = 750;


	/* Handle command line parameters. */
	for(i = 1; i < argc; i++)
	{
	    /* Help. */
            if(strcasepfx(argv[i], "--h") ||
               strcasepfx(argv[i], "-h") ||
               strcasepfx(argv[i], "-?") ||
               strcasepfx(argv[i], "/?") ||
               strcasepfx(argv[i], "?")
	    )
            {
		XSWDoHelp();
		return(-4);
            }
            /* Version. */
            else if(strcasepfx(argv[i], "--ver") ||
		    strcasepfx(argv[i], "-ver")
	    )
            {
                XSWDoVersion();
                return(-4);
            }
	    /* RCFile. */
	    else if(strcasepfx(argv[i], "--rc") ||
                    strcasepfx(argv[i], "-rc") ||
                    strcasepfx(argv[i], "-f")
	    )
	    {
		i++;
		if(i < argc)
		{
		    strncpy(fname.rc, argv[i], NAME_MAX + PATH_MAX);
		    fname.rc[NAME_MAX + PATH_MAX - 1] = '\0';
		}
		else
		{
		    fprintf(stderr,
			"%s: Missing filename.\n",
			argv[i - 1]
		    );
		    return(-2);
		}
            }
	    /* Controller type. */
            else if(strcasepfx(argv[i], "--con") ||
                    strcasepfx(argv[i], "-c")
	    )
            {
                i++;
                if(i < argc)
		{
		    if(strcasepfx(argv[i], "k"))
                    {
                        option.controller = CONTROLLER_KEYBOARD;
                        option.cmd_line_set_controller = 1;
                    }
#ifdef JS_SUPPORT
		    else if(strcasepfx(argv[i], "j"))
		    {
		        option.controller = CONTROLLER_JOYSTICK;
                        option.cmd_line_set_controller = 1;
		    }
#endif /* JS_SUPPORT */
                    else if(strcasepfx(argv[i], "p") ||
		            strcasepfx(argv[i], "m")
		    )
                    {
                        option.controller = CONTROLLER_POINTER;
			option.cmd_line_set_controller = 1;
                    }
		    else
		    {
		        fprintf(stderr,
                            "%s: Unsupported controller type.\n",
                            argv[i]
                        );
		    }
		}
		else
		{
                    fprintf(stderr,
                        "%s: Missing argument.\n",
                        argv[i - 1]
                    );
		}
            }
	    /* Site address and port. */
	    else if(strcasepfx(argv[i], "swserv://"))
	    {
		/* Get login name. */
		strptr = StringParseName(argv[i]);
		if(strptr != NULL)
		{
		    strncpy(net_parms.login_name, strptr, XSW_OBJ_NAME_MAX);
		    net_parms.login_name[XSW_OBJ_NAME_MAX - 1] = '\0';
		}
		/* Get login password. */
                strptr = StringParsePassword(argv[i]);
                if(strptr != NULL)
                {
                    strncpy(net_parms.login_password, strptr, XSW_OBJ_PASSWORD_MAX);
                    net_parms.login_password[XSW_OBJ_PASSWORD_MAX - 1] = '\0';
                }
		/* Get address. */
		strptr = StringParseAddress(argv[i]);
		if(strptr != NULL)
		{
		    strncpy(net_parms.address, strptr, MAX_URL_LEN);
		    net_parms.address[MAX_URL_LEN - 1] = '\0';
		}
		else
		{
		    fprintf(stderr, "%s: %s: Bad URL.\n",
			argv[0], argv[i]
		    );
		}
		/* Get port. */
		n = StringParsePort(argv[i]);
		if(n > -1)
		    net_parms.port = n;

		/* Mark address being set at startup. */
                net_parms.is_address_set = 2;
	    }
        }


	/* Load program configuration from the configuration file,
	 * look for it in the expected locations.
	 */

	/* In current or command line specified location? */
	if(stat(fname.rc, &stat_buf))
	{
	    /* Not in current or command line specified location. */

	    /* Look in local shipwars directory. */
	    strptr = PrefixPaths(local_sw_dir, DEF_XSW_RCFILE);
	    strncpy(
		fname.rc,
		((strptr == NULL) ? DEF_XSW_RCFILE : strptr),
		PATH_MAX + NAME_MAX
	    );
	    fname.rc[PATH_MAX + NAME_MAX - 1] = '\0';

	    if(stat(fname.rc, &stat_buf))
	    {
		char src_path[PATH_MAX + NAME_MAX];

		/*   Configuration file does not exist in:
		 *
                 *   1. Command line specified directory.
                 *   2. Local program directory.
		 */

                /* Attempt to install global configuration file. */
		strptr = PrefixPaths(dname.etc, DEF_XSW_RCFILE);
		strncpy(
		    src_path,
		    ((strptr == NULL) ? DEF_XSW_RCFILE : strptr),
		    PATH_MAX + NAME_MAX
		);
		src_path[PATH_MAX + NAME_MAX - 1] = '\0';

                XSWDoInstallObject(
                    local_sw_dir,	/* Target. */
                    src_path,		/* Source. */
                    S_IRUSR | S_IWUSR | S_IXUSR,
                    XSW_INST_OBJ_FLAG_DIR |	/* Options. */
		    XSW_INST_OBJ_FLAG_RECURSIVE,
                    NULL	/* Data argument. */
                );
		XSWDoInstallObject(
		    fname.rc,	/* Target. */
		    src_path,	/* Source. */
		    S_IRUSR | S_IWUSR,
		    0,		/* Options. */
		    NULL	/* Data argument. */
		);
/*
		chmod(fname.rc, S_IRUSR | S_IWUSR);
 */
	    }
	}
	/* Load configuration file. */
	RCLoadFromFile(fname.rc);


	/* Check if universe list file exists, this file location is
	 * defined when we loaded the rcfile above.
	 */
	if(stat(fname.universe_list, &stat_buf))
	{
	    char src_path[PATH_MAX + NAME_MAX];


            strptr = PrefixPaths(dname.etc, DEF_UNIVERSES_FILE);
            strncpy(
                src_path,
                ((strptr == NULL) ? DEF_UNIVERSES_FILE : strptr),
                PATH_MAX + NAME_MAX
            );
            src_path[PATH_MAX + NAME_MAX - 1] = '\0';

            XSWDoInstallObject(
                fname.universe_list,	/* Target. */
                src_path,		/* Source. */
                S_IRUSR | S_IWUSR,
                0,			/* Options. */
                NULL			/* Data argument. */
            );
	}	
	UnivListLoadFromFile(fname.universe_list);


	/* Now that configuration values have been loaded, check
	 * if the required directories exist.
	 */
	if(stat(dname.toplevel, &stat_buf))
	{
	    fprintf(stderr,
"Error: Cannot access directory: %s\n", dname.toplevel);
	    fprintf(stderr,
"This is the global data files directory for %s.\n", PROG_NAME);
	    fprintf(stderr,
"Please check if it exists and that its permission settings allow access.\n");
            fprintf(stderr,
"If this directory cannot be found, please reinstall %s.\n", PROG_NAME);
	    return(1);
	}
	else if(stat(dname.images, &stat_buf))
	{
            fprintf(stderr,
"Error: Cannot access directory: %s\n", dname.images);
            fprintf(stderr,
"This directory contains the images for %s.\n", PROG_NAME);
            fprintf(stderr,
"Please check if it exists and that its permission settings allow access.\n");
	    fprintf(stderr,
"If this directory cannot be found, please reinstall %s.\n", PROG_NAME);
            return(1);
	}
        else if(stat(dname.sounds, &stat_buf))
        {
/* Do not warn if this directory does not exist.
            fprintf(stderr,
"Error: Cannot access directory: %s\n", dname.sounds);
            fprintf(stderr,
"This directory contains the sounds for %s.\n", PROG_NAME);
            fprintf(stderr,
"Please check if it exists and that its permission settings allow access.\n");
            fprintf(stderr,
"If this directory cannot be found, please reinstall %s.\n", PROG_NAME);
            return(1);
 */
        }
        else if(stat(dname.etc, &stat_buf))
        {
            fprintf(stderr,
"Error: Cannot access directory: %s\n", dname.etc);
            fprintf(stderr,
"This is the global etc directory for %s.\n", PROG_NAME);
            fprintf(stderr,
"Please check if it exists and that its permission settings allow access.\n");
            fprintf(stderr,
"If this directory cannot be found, please reinstall %s.\n", PROG_NAME);
	    return(1);
        }


        /* ******************************************************** */
	/* Apply command line overrides. */
	if(option.cmd_line_set_controller)
	{
	    if(GCtlInit(option.controller))
	    {
                fprintf(stderr,
"There was an error encountered while initializing one or more\n\
game controller device(s).   Check your configuration to ensure\n\
that your controller (ie: keyboard, joystick, pointer, etc) is\n\
set up properly.\n\
\n\
You may change the controller device that is to be used by editing\n\
your configuration file .xshipwarsrc and change the value for\n\
parameter `ControlType' to something such as `keyboard'.\n"
                );
	    }
	}


	/* ******************************************************** */
	/* Initialize sound. */
	if((option.sounds > XSW_SOUNDS_NONE) &&
	   (sound.server_type > SNDSERV_TYPE_NONE)
	)
	{
	    if(SoundInit())
            {
		/* An error occured while starting sound server. */
                fprintf(stderr,
"Your configuration specified sound support, however there was an\n\
error initializing the sound server.  Verify the sound settings are\n\
correct and that your sound server is set up properly.\n"
                );

		/* Turn sounds off. */
		option.sounds = XSW_SOUNDS_NONE;
            }
	    else
	    {
		/* Successfully initialized sound. */

		/* Do not change background music just yet. */
	    }
	}


        /* *********************************************************** */
        /* Connect to GUI. */
	if(OSWGUIConnect(argc, argv))
	    return(-1);


	/* Make sure that the depth is supported. */
#ifdef X_H
        if((osw_gui[0].depth != 8) &&
	   (osw_gui[0].depth != 15) &&
           (osw_gui[0].depth != 16) &&
           (osw_gui[0].depth != 24) &&
           (osw_gui[0].depth != 32)
        )
        {
            fprintf(stderr,
                "Error: Depth %i bits unsupported.\n",
		osw_gui[0].depth
            );

            fprintf(stderr,
"\nThis program requires that Depth be set to one of the following\n\
values:\n\
\n\
    8 bits\n\
    15 bits\n\
    16 bits\n\
    24 bits\n\
    32 bits\n\
\n\
To correct this problem, specify your Depth value to one of the above\n\
values in your X server's configuration file.\n"
            );

            return(-1);
        }
#endif	/* X_H */


#ifdef X_H
	/* Check if visual class is TrueColor or DirectColor. */
	if(osw_gui[0].visual != NULL)
	{
#if defined(__cplusplus) || defined(c_plusplus)
	    i = osw_gui[0].visual->c_class;
#else
	    i = osw_gui[0].visual->class;
#endif

	    if((i != TrueColor) &&
               (i != PseudoColor)
	    )
	    {
		switch(i)
		{
		  case StaticGray:
		    strptr = "StaticGray";
		    break;

		  case GrayScale:
		    strptr = "GrayScale";
		    break;

                  case StaticColor:
                    strptr = "StaticColor";
                    break;

                  case PseudoColor:
                    strptr = "PseudoColor";
                    break;

                  case TrueColor:
                    strptr = "TrueColor";
                    break;

                  case DirectColor:
                    strptr = "DirectColor";
                    break;

		  default:
		    strptr = "Unknown";
		    break;
 		}

		fprintf(stderr,
 "Warning: Visual class %s incompatable for rendering.\n",
		    strptr
		);

                fprintf(stderr,
"\nThis program runs best with a Visual class of TrueColor. To correct\n\
this problem, specify your Visual value to \"TrueColor\" in your X server's\n\
configuration file.\n"
		);
	    }
	}
#endif	/* X_H */

        /*   Load and set fonts (although a default font is already set
	 *   in the OSWGUIConnect() call, we would still like to set
	 *   our own font).
	 */
	if(OSWLoadFont(&xsw_font.std, "7x14"))
	    return(-1);
        if(OSWLoadFont(&xsw_font.std_bold, "7x14"))
            return(-1);

        if(OSWLoadFont(&xsw_font.console_heading, "7x14"))
            return(-1);
        if(OSWLoadFont(&xsw_font.console_standard, "6x10"))
            return(-1);
        if(OSWLoadFont(&xsw_font.console_message, "7x14"))
            return(-1);

	/* Set foreground font. */
/*
	OSWSetFont(xsw_font.std);
 */


	/* Load color pixels. */
	if(XSWLoadColors())
            return(-1);


	/* ********************************************************** */
	/* Load cursors. */

	color.a = 0x00;
	color.r = 0xff;
	color.g = 0xc0;
	color.b = 0xfa;

	xsw_cursor.scanner_lock = WidgetCreateCursorFromData(
	    xsw_scanner_lock_xpm,
	    8, 8,
	    color
	);
	if(xsw_cursor.scanner_lock == NULL)
	    return(-1);

        xsw_cursor.text = WidgetCreateCursorFromData(
            xsw_text_xpm,
            8, 8,
            color
        );
        if(xsw_cursor.text == NULL)
            return(-1);

	/* No wait cursor yet. */
	xsw_cursor.wait = NULL;

        xsw_cursor.translate = WidgetCreateCursorFromData(
            translate_xpm,
            8, 8,
            color
        );
        if(xsw_cursor.translate == NULL)
            return(-1);

        xsw_cursor.zoom = WidgetCreateCursorFromData(
            zoom_xpm,
            8, 8,
            color
        );
        if(xsw_cursor.zoom == NULL)
            return(-1);


	/* ********************************************************** */
	/* Keyboard control settings. */

	/* *** Dosen't work under most GUI's so skip it. *** */


	/* ********************************************************* */
#ifdef X_H
#ifdef USE_XSHM
        /* Check for MIT SHM extension availablility. */
	if(XQueryExtension(osw_gui[0].display, "MIT-SHM", &x, &x, &x))
	{
	    if(XShmQueryVersion(osw_gui[0].display, &y, &z, &b) == True)
	    {
/*
	        printf("XShm extention version %d.%d %s shared pixmaps\n",
	            y, z, (b == True) ? "with" : "without"
	        );
*/
	    }
	    else
	    {
		fprintf(stderr,
	            "%s: Error: MIT-SHM library not found.\n",
		    argv[0]
		);
	        fprintf(stderr,
"\nThis program requires standard System V style and MIT Shared\n\
Memory Extensions to produce instant graphic updates to the\n\
X server.   Your X server and operating system does not indicate\n\
it supports the above required library extensions.\n"
	        );

        	return(-1);
	    }
	}
#endif	/* USE_XSHM */
#endif	/* X_H */


        /* ********************************************************* */
        /* Initialize window widgets, groups and base images. */

	/* Must initialized the widget globals before initializing
         * any widgets.
	 */
	if(WidgetInitGlobals(argc, argv))
	{
	    fprintf(stderr,
"\nThe program failed in initializing it's internal widget set.\n\
The most common problem is due to a misplaced file.  Check to\n\
make sure that all files for this program are installed properly.\n\
You may need to reinstall this program's data files if one of\n\
the files has been permanently lost.\n"
	    );
            return(-1);
	}

	/* Set function to recieve widget's scroll bar updates. */
	SBarSetNotifyFunction(XSWScrollBarCB);


	/* Initialize the error dialog window. */
	if(
	    DialogWinInit(
	        &err_dw,
	        osw_gui[0].root_win,
	        350,
	        200,
	        NULL
	    )
	)
	    return(-1);
	OSWSetWindowTitle(err_dw.toplevel, "XShipWars: Error");

        /* Initialize the info dialog window. */
	if(
            DialogWinInit(
                &info_dw,
                osw_gui[0].root_win,
                350,
                200,
                NULL
            )
	)
	    return(-1);
        OSWSetWindowTitle(info_dw.toplevel, "XShipWars: Information");


        /*
         *   Initialize splash window set before loading the
	 *   program's images.
         */
        if(SplashInit())
	    return(-1);


        /*   Load program images:
	 *
	 *   This must be performed first before any of the other
	 *   window sets are initialized (except for the load status
	 *   window which is initialized before the images).
	 */
        if(IMGDoLoadAll())
        {
	    fprintf(stderr,
"There was an error loading the program's images.\n\
 Since most images are external, please check and verify that\n\
 you have the images installed in their proper locations.\n"
	    );
	    return(-1);
        }

/* Set icon for error dialog. */
if(IMGIsImageNumAllocated(IMG_CODE_ERROR_ICON))
    err_dw.icon_img = xsw_image[IMG_CODE_ERROR_ICON]->image;
/*
DialogWinDraw(&err_dw, NULL);
*/

/* Set icon for info dialog. */
if(IMGIsImageNumAllocated(IMG_CODE_INFO_ICON) == 1)
    info_dw.icon_img = xsw_image[IMG_CODE_INFO_ICON]->image;
/*
DialogWinDraw(&info_dw, NULL);
*/


        /* Load OCSNs from file. */
        if(OCSLoadFromFile(fname.ocsn))
        {
            fprintf(stderr,
"An error occured while reading the OCS names file. This may\n\
cause some name labels and images to be displayed improperly.\n"
            );
        }

        /* Load isrefs from file. */
        if(ISRefLoadFromFile(fname.isr))
        {
            fprintf(stderr,
                "%s: Error loading image set referances.\n",
                fname.isr
            );
            fprintf(stderr,
"This file is required for the program to know which actual\n\
images files belong to which `image set referance code' and the\n\
properties of that actual image file."
            );
/* Error dialog here. */
            return(-1);
        }

	/* Bridge window set. */
	if(BridgeWinInit(argc, argv))
	    return(-1);

	/* Main page configuration (load from file) */
        if(PageLoadFromFile(
		&bridge_win.main_page,
		bridge_win.viewscreen,
		bridge_win.viewscreen_image,
                fname.main_page
        ))
	{
	    fprintf(stderr,
  "Warning: Error occured while reading the main page configuration file.\n"
	    );
	}

        /* Destroyed page configuration (load from file) */
        if(PageLoadFromFile(
                &bridge_win.destroyed_page,
                bridge_win.viewscreen,
                bridge_win.viewscreen_image,
                fname.destroyed_page
        ))
        {
            fprintf(stderr,
  "Warning: Error occured while reading the destroyed page configuration file.\n"
            );
        }


	/* Universe list window set. */
	if(UnivListInit())
	    return(-1);

        /* Universe edit window set. */
        if(UnivEditWinInit())
            return(-1);

	/* Large message screen window set. */
	if(LMesgWinInit())
            return(-1);

	/* Options window. */
	if(OptWinInit())
            return(-1);

	/* Keymap window. */
        if(KeymapWinInit())
            return(-1);

#ifdef JS_SUPPORT
	/* Joystick mapping window. */
	if(JSMWInit())
	    return(-1);
#endif	/* JS_SUPPORT */

	/* Quick menu (on the bridge window). */
	if(QMenuInit())
            return(-1);

	/* Economy window. */
	if(EcoWinInit())
            return(-1);

	/* Starchart window. */
	if(SChtInit(&starchart_win))
	    return(-1);


	/* Primary file browser. */
	if(
	    FBrowserInit(
	        &pri_fbrowser,
	        0, 0,
	        0, 0,
	        cwd,
	        FB_STYLE_SINGLE_LIST,
	        XSWFBCBOk,
	        XSWFBCBCancel
	    )
	)
            return(-1);
	pri_fb_loadop = PRI_FB_LOADOP_NONE;


	/* ******************************************************* */
	/* Purge all GUI events, this is to get rid of any user
	 * inputs that were accumulated due to user impatience during
	 * startup.
	 */
	OSWPurgeAllEvents();


        /* Map bridge window group. */
	BridgeWinMap();


        /* ******************************************************* */
        /* Print greeting message. */
        sprintf(stringa,
	    "%s Version %s",
	    PROG_NAME,
	    PROG_VERSION
	);
        MesgAdd(stringa, xsw_color.standard_text);

/* Too spammy for 5 line average message console
        sprintf(
	    stringa,
	    PROG_COPYRIGHT
	);
        MesgAdd(stringa, xsw_color.standard_text);
 */

        sprintf(stringa,
            "(Press %s for help.)",
	    OSWGetKeyCodeName(xsw_keymap[XSW_KM_HELP].keycode)
        );
        MesgAdd(stringa, xsw_color.standard_text);


	/* ******************************************************** */
	/* Connect at startup? */
	if(net_parms.is_address_set == 2)
	{
	    sprintf(stringa,
		"connect swserv://%s:%s@%s:%i",
		net_parms.login_name,
		net_parms.login_password,
		net_parms.address,
		net_parms.port
	    );
	    CmdHandleInput(stringa);

            /* Destroy load status window. */
            SplashDestroy();
	}
	else
	{
            /* Destroy load status window. */
            SplashDestroy();

	    /* Map the universe list window. */
	    if(option.auto_map_univ_list_win)
	        UnivListMap();
	}


	/* Initialize reality engine. */
	REngInit();


	/* ********************************************************* */
	/* Select signals to watch for. */

	signal(SIGHUP, XSWHandleSignal);
	signal(SIGINT, XSWHandleSignal);
        signal(SIGQUIT, XSWHandleSignal);
        signal(SIGABRT, XSWHandleSignal);
        signal(SIGKILL, XSWHandleSignal);
        signal(SIGSEGV, XSWHandleSignal);
        signal(SIGPIPE, XSWHandleSignal);
        signal(SIGTERM, XSWHandleSignal);
        signal(SIGCONT, XSWHandleSignal);
        signal(SIGSTOP, XSWHandleSignal);


	return(0);
}

/*
 *	Master management procedure function for this program.
 *
 *	This function should be called once per loop during normal
 *	running (while(runlevel >= 2)).
 *
 *	Note that sleeping is not handled by this function, the
 *	controlling loop needs to call the proper sleep function to
 *	keep the cycle maximized and prevent the program from hogging
 *	the cpu.
 */
void XSWManage()
{
	int i, player_obj_num, tar_obj_num;
	long new_millitime;
	char text[256];


	/* Update timmings. */
	cur_systime = time(NULL);
	new_millitime = MilliTime();

        /*   Reset timmers if new millitime is less than
         *   cur_millitime.
         */
	if(new_millitime < cur_millitime)
	{
	    /* Reset global timmers. */
            XSWDoResetTimmers();

            lapsed_millitime = 0;
            time_compensation = 1.0;
	}
	else
	{
	    lapsed_millitime = new_millitime - cur_millitime;
	    time_compensation = (double)(
                (double)lapsed_millitime / (double)CYCLE_LAPSE_MS
            );
            if(time_compensation < 1)
                time_compensation = 1;
	}

	/* Now set global variable cur_millitime. */
	cur_millitime = new_millitime;


	/* Increment general animation timmers. */
	for(i = 0; i < MAX_ANIM_TIMMERS; i++)
	{
	    /* Skip if timmer is not due for increment. */
	    if(genanim_timmer[i].next > cur_millitime)
		continue;

	    genanim_timmer[i].count++;
	    genanim_timmer[i].next = cur_millitime +
		genanim_timmer[i].interval;

	    if(genanim_timmer[i].count >= genanim_timmer[i].count_max)
	        genanim_timmer[i].count = 0;
	}


        /* *********************************************************** */
	/* Manage events. */

	/* Game controller update. */
	if(option.controller == CONTROLLER_KEYBOARD)
	{
	    /*   Game controller is set to keyboard, so do
             *   update only if bridge window is in focus and
             *   the prompt mode is set to none.
	     */
	    if((prompt_mode == PROMPT_CODE_NONE) &&
	       (bridge_win.is_in_focus == 1)
	    )
	        GCtlUpdate(option.controller);
	}
	else
	{
	    GCtlUpdate(option.controller);
	}


	/* Handle GUI events. */
	XSWManageGUI();

	/* Handle sound events. */
	XSWManageSound();


	/* ****************************************************** */
	/* Handle network. */

	switch(net_parms.connection_state)
	{
	  /* Connected and logged in? */
	  case CON_STATE_CONNECTED:

	    /* Check for incoming recieved data. */
	    NetHandleRecv();

            /* Adjust automatic interval tunning. */
	    if(auto_interval_tune.next <= cur_millitime)
	    {
		AIntvTuneHandleAdjust();

		/* Schedual next auto interval tune. */
		auto_interval_tune.next = cur_millitime +
		    auto_interval_tune.interval;
	    }

	    /* Send out local player object position to server. */
	    if(next.lplayer_pos_send <= cur_millitime)
            {
		/* Get referance to player object number. */
		player_obj_num = net_parms.player_obj_num;


		/* Send sector position. */
		NetSendObjectSect(player_obj_num);

		/* Pose object (coordinates). */
		NetSendPoseObj(player_obj_num);

		/* Throttle position. */
		NetSendObjectThrottle(player_obj_num);

		/* Schedual next player position send. */
		next.lplayer_pos_send = cur_millitime +
		    net_parms.net_int;
	    }

	    /*   Manage server script upload.  Timming is handled
	     *   by ServScriptManage().
	     */
	    ServScriptManage();
	    break;

          /* Logging in? */
          case CON_STATE_NEGOTIATING:
	    /* Warn if logging in is taking longer than it should. */
	    if(((net_parms.con_start + 15) <= cur_systime) &&
               !net_parms.warn15
	    )
	    {
		MesgAdd(
"Waited 15 seconds for server response, still waiting...",
		    xsw_color.standard_text
		);
		net_parms.warn15 = 1;
	    }
            else if(((net_parms.con_start + 30) <= cur_systime) &&
               !net_parms.warn30
            )
            {
                MesgAdd(
"Waited 30 seconds for server response, still waiting...",
                    xsw_color.standard_text
                );
                net_parms.warn30 = 1;
            }
            else if(((net_parms.con_start + 45) <= cur_systime) &&
                    !net_parms.warn45
            )
            {
		sprintf(text,
"Waited 45 seconds (press %s a few times to give up)...",
		    OSWGetKeyCodeName(xsw_keymap[XSW_KM_DISCONNECT].keycode)
		);
		MesgAdd(text, xsw_color.standard_text);
                net_parms.warn45 = 1;
            }

            /*   Handle incoming data (just checking for
             *   login or disconnect).
             */
            NetHandleRecv();
	    break;

	  /* Not connected. */
	  default:	/* CON_STATE_NOT_CONNECTED */
	    break;
        }


	/* ****************************************************** */
	/* Reality Engine: Update XSW Objects. */

	REngManage();


        /* ****************************************************** */
        /* Image set referance management. */  

        ISRefManage();


	/* ****************************************************** */
	/* Redraw GUI windows that need to be updated regularly. */

	/* Viewscreen. */
	if((option.energy_saver_mode) ?
	    (next.viewscreen <= cur_millitime) :
            1
	)
	{
            /* Get referance to player object number. */
            player_obj_num = net_parms.player_obj_num;


            /* Increment frames per second counter. */
            fps_counter.fcount++;
            if(fps_counter.next <= cur_millitime)
            {
                fps_counter.lfcount = fps_counter.fcount;
                fps_counter.fcount = 0;
                fps_counter.next = cur_millitime +
                    fps_counter.interval;
            }

	    /* Redraw viewscreen. */
	    if(bridge_win.viewscreen_vis_state != VisibilityFullyObscured)
		VSDrawViewScreen(
		    player_obj_num,
		    bridge_win.viewscreen,
		    bridge_win.viewscreen_image,
		    bridge_win.viewscreen_zoom
		);

	    /* Redraw scanner. */
	    if(bridge_win.scanner_vis_state != VisibilityFullyObscured)
                ScannerDraw(
                    player_obj_num,
                    bridge_win.scanner,
                    bridge_win.scanner_image,
                    0,
                    0,
                    bridge_win.scanner_zoom
                );

            /*   Redraw bridge console panels that need to be redrawn
	     *   per every loop.
	     */
	    if((net_parms.connection_state == CON_STATE_CONNECTED) &&
               (net_parms.player_obj_ptr != NULL)
	    )
	    {
		tar_obj_num = net_parms.player_obj_ptr->locked_on;

                BridgeWinDrawPanel(
		    player_obj_num,
		    BPANEL_DETAIL_PTHROTTLE
		);
                BridgeWinDrawPanel(
                    player_obj_num,
                    BPANEL_DETAIL_PVELOCITY
                );
                BridgeWinDrawPanel(
                    player_obj_num,
                    BPANEL_DETAIL_PHEADING
                );
		BridgeWinDrawPanel(
		    player_obj_num,
		    BPANEL_DETAIL_PTHRUSTVECTOR
		);

		/* Subject object. */
		if(tar_obj_num > -1)
		{
                    BridgeWinDrawPanel(
                        tar_obj_num,
                        BPANEL_DETAIL_SBEARING
                    );
		    BridgeWinDrawPanel(
			tar_obj_num,
			BPANEL_DETAIL_SDISTANCE
		    );
		}
	    }


	    /* Calculate and schedual next viewscreen draw time. */
	    if(option.energy_saver_mode)
	    {
		/* In energy saver mode, draw slower. */
		bridge_win.viewscreen_int = MAX(
		    MilliTime() - cur_millitime + 1000,
		    0
		);

		next.viewscreen = MilliTime() + bridge_win.viewscreen_int;
	    }
	    else
	    {
		/* Calculate how above redraws took. */
		bridge_win.viewscreen_int = MAX(
		    MilliTime() - cur_millitime - CYCLE_LAPSE_MS,
		    0
		);
		next.viewscreen = cur_millitime +
		    bridge_win.viewscreen_int;
	    }
	}


        /* Redraw consoles and scanner contact check. */
        if(next.consoles <= cur_millitime)
        {
	    /* Get referance to player object number. */
	    player_obj_num = net_parms.player_obj_num;


	    /* Update sensor contacts notify. */
	    ScHandleContacts(player_obj_num);

	    /* Redraw player object stats. */
            BridgeWinDrawPanel(player_obj_num, BPANEL_DETAIL_PNAME);
            BridgeWinDrawPanel(player_obj_num, BPANEL_DETAIL_PSHIELDFREQ);
            BridgeWinDrawPanel(player_obj_num, BPANEL_DETAIL_PWEAPONFREQ);
            BridgeWinDrawPanel(player_obj_num, BPANEL_DETAIL_PCOMCHANNEL);

	    BridgeWinDrawPanel(player_obj_num, BPANEL_DETAIL_PHULL);
            BridgeWinDrawPanel(player_obj_num, BPANEL_DETAIL_PPOWER);
            BridgeWinDrawPanel(player_obj_num, BPANEL_DETAIL_PSHIELDS);
            BridgeWinDrawPanel(player_obj_num, BPANEL_DETAIL_PVIS);
            BridgeWinDrawPanel(player_obj_num, BPANEL_DETAIL_PDMGCTL);

            BridgeWinDrawPanel(player_obj_num, BPANEL_DETAIL_PINAME);
            BridgeWinDrawPanel(player_obj_num, BPANEL_DETAIL_PWLOCK);
            BridgeWinDrawPanel(player_obj_num, BPANEL_DETAIL_PWEAPONS); 

            BridgeWinDrawPanel(player_obj_num, BPANEL_DETAIL_PANTIMATTER);
/*
            BridgeWinDrawPanel(player_obj_num, BPANEL_DETAIL_PENGINESTATE);
            BridgeWinDrawPanel(player_obj_num, BPANEL_DETAIL_PTHRUSTVECTOR);
 */
	    if(net_parms.player_obj_ptr != NULL)
		tar_obj_num = net_parms.player_obj_ptr->locked_on;
	    else
		tar_obj_num = -1;

            BridgeWinDrawPanel(tar_obj_num, BPANEL_DETAIL_SNAME);
            BridgeWinDrawPanel(tar_obj_num, BPANEL_DETAIL_SEMPIRE);
            BridgeWinDrawPanel(tar_obj_num, BPANEL_DETAIL_SHULL);
            BridgeWinDrawPanel(tar_obj_num, BPANEL_DETAIL_SPOWER);
            BridgeWinDrawPanel(tar_obj_num, BPANEL_DETAIL_SSHIELDS);
            BridgeWinDrawPanel(tar_obj_num, BPANEL_DETAIL_SVIS);
            BridgeWinDrawPanel(tar_obj_num, BPANEL_DETAIL_SBEARING);
            BridgeWinDrawPanel(tar_obj_num, BPANEL_DETAIL_SDISTANCE);
            BridgeWinDrawPanel(tar_obj_num, BPANEL_DETAIL_SDMGCTL);

            BridgeWinDrawPanel(tar_obj_num, BPANEL_DETAIL_SINAME);
            BridgeWinDrawPanel(tar_obj_num, BPANEL_DETAIL_SWLOCK);
            BridgeWinDrawPanel(tar_obj_num, BPANEL_DETAIL_SWEAPONS);

            BridgeWinDrawPanel(tar_obj_num, BPANEL_DETAIL_SANTIMATTER);
            BridgeWinDrawPanel(tar_obj_num, BPANEL_DETAIL_SHEADING);

            /* Update netstats label (do not draw it). */
            if((option.show_viewscreen_labels == 2) ||
               (option.show_viewscreen_labels == 3)
            )
            {
                VSDrawUpdateNetstatsLabel(
                    &bridge_win.net_stats_image,
                    bridge_win.net_stats_buf
                );
            }

	    /* Schedual next time to draw consoles. */
            next.consoles = cur_millitime + CONSOLES_UPDATE_INTERVAL;
        }


	/* ************************************************* */
	/* Starchart. */
        SChtTimeoutCB(&starchart_win);


	/* ************************************************* */
	/* Clean/refresh memory. */
	if(next.memory_clean <= cur_millitime)
	{
	    /* Clean, reclaim, memory and schedual next. */
	    XSWReclaimGlobalMemory(False);

	    /* XSWReclaimGlobalMemory() already scheduals next. */
	}

	return;
}

/*
 *	Deallocates all allocated memory and resources of this
 *	program. This function should be called just before
 *	the program exits.
 *
 *	Warning, the configuration file will not be saved by
 *	this function. So a call to RCSaveToFile() should
 *	be made before calling this function if you want to
 *	save the configuration.
 */
void XSWShutdown()
{
	/* Server script upload events. */
	ServScriptDeleteAll();

        /* Close connection properly. */
	XSWDoDisconnect();

        /* GUI resources. */
	if(IDC())
	{
	    /* XSW created windows. */
	    SChtDestroy(&starchart_win);
	    EcoWinDestroy();
            QMenuDestroy();
#ifdef JS_SUPPORT
	    JSMWDestroy();
#endif	/* JS_SUPPORT */
	    KeymapWinDestroy();
	    OptWinDestroy();
	    UnivEditWinDestroy();
            UnivListDestroy();
            LMesgWinDestroy();
            BridgeWinDestroy();

	    DialogWinDestroy(&err_dw);
	    DialogWinDestroy(&info_dw);
            FBrowserDestroy(&pri_fbrowser);


            /* ISRefs. */
            ISRefDeleteAll();

            /* XSW Client Images (not ISRefs). */
            IMGDoUnloadAll();

            /* Viewscreen object labels. */
            VSLabelDeleteAll();

	    /* XSW created widget cursors. */
	    WidgetDestroyCursor(&xsw_cursor.scanner_lock);
	    WidgetDestroyCursor(&xsw_cursor.text);
	    WidgetDestroyCursor(&xsw_cursor.wait);
	    WidgetDestroyCursor(&xsw_cursor.translate);
            WidgetDestroyCursor(&xsw_cursor.zoom);

	    /* Fonts. */
	    OSWUnloadFont(&xsw_font.std);
	    OSWUnloadFont(&xsw_font.std_bold);
            OSWUnloadFont(&xsw_font.console_heading);
            OSWUnloadFont(&xsw_font.console_standard);
            OSWUnloadFont(&xsw_font.console_message);

	    /* Color pixels. */
	    XSWFreeColors();

	    /* Widget globals (must be freed after all widget stuff). */
	    WidgetDestroyGlobals();

	    /* Reset keyboard repeat. */
	    OSWKBAutoRepeatOn();

            /* Disconenct from GUI. */
	    OSWGUIDisconnect();
	}

	/* Reality engine shutdown. */
	REngShutdown();

        /* Shutdown game controller and resources. */
	GCtlShutdown();

#ifdef JS_SUPPORT
	/* Joystick mappings. */
	JSMapDeleteAll();
#endif	/* JS_SUPPORT */

	/* Shutdown sound server and free sound schemes. */
	SoundShutdown();

        /* Delete all object create script names. */
        OCSDeleteAll();

        /* Delete all scanner contacts. */
        ScDeleteAll();

        /* Delete all objects, this has probably already been
	 * done with a call to XSWDoDisconnect(). But we delete
	 * them all here just incase.
	 */
        DBDeleteAllObjects();

	/* Shutdown universe object management. */
	UNVShutdown();


	return;
}



int main(int argc, char *argv[])
{
	int status;


	/* Runlevel 1: Starting. */
	runlevel = 1;


	/* Initialize. */
	status = XSWInit(argc, argv);
	switch(status)
	{
	  /* Success. */
	  case 0:
	    break;

	  /* Just print help or version. */
	  case -4:
	    XSWShutdown();
	    return(0);
	    break;

	  /* Error. */
	  default:
	    XSWShutdown();
	    return(1);
	    break;
	}


	/* Runlevel 2: Normal running. */
	runlevel = 2;
	while(runlevel >= 2)
	{
            /* Sleep. */
	    usleep(CYCLE_LAPSE_US);

	    /* Standard program management. */
	    XSWManage();
	}


	/* Runlevel 1: Begin shutdown sequence. */

        /* Save configuration. */
	if(option.save_on_exit)
	{
	    RCSaveToFile(fname.rc);
	    UnivListSaveToFile(fname.universe_list);
	}

	/* Free global allocated resources and perform proper
	 * proper shutdown sequence.
	 */
	XSWShutdown();


	/* Runlevel 0: Finished */
	runlevel = 0;


	return(0);
}
