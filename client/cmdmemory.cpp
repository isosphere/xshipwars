#include "xsw.h"

#define THIS_CMD_NAME	"memory"

 
/*
 *      Print memory stats, reload configuration, or refresh and reclaim
 *	memory.
 */
int CmdMemory(const char *arg)
{
	int events;
	xsw_mem_stat_struct buf;
        char text[PATH_MAX + NAME_MAX + 512];
        

        /* Get memory stats. */
        XSWGetProgMemory(&buf);


        /* Calculate memory consumed events. */
        events = OSWEventsPending();
 
        /* Print memory statistics. */
        if((arg == NULL) ? 1 : (arg[0] == '\0'))
        {
            sprintf(text,   
 "         total   intimgs    isrefs      objs     ocsns"
            );
            MesgAdd(text, xsw_color.standard_text);

            sprintf(text,
 "Mem:\
 %9ld\
 %9ld\
 %9ld\
 %9ld\
 %9ld",
                buf.total,
                buf.images,
                buf.isrefs,
                buf.objects,
                buf.ocsns
            );
            MesgAdd(text, xsw_color.standard_text);
            
            sprintf(text,
 "Ent:\
 %9d\
 %9d\
 %9d\
 %9d\
 %9d",
                (int)(total_objects + total_isrefs + total_ocsns),
                total_images,
                total_isrefs,
                total_objects,
                total_ocsns
            );
            MesgAdd(text, xsw_color.standard_text);

            sprintf(text,
		"Usage: `%s [reload|refresh]'",
		THIS_CMD_NAME
            );
            MesgAdd(text, xsw_color.standard_text);
        }
        /* ********************************************************** */
        /* Reload files into memory? */
        else if(!strcasecmp(arg, "reload"))
        {
            sprintf(text,
    "Client: Memory reload: Main page configuration from `%s'...",
                fname.main_page
            );
            MesgAdd(text, xsw_color.standard_text);
            PageLoadFromFile(
                &bridge_win.main_page,
                bridge_win.viewscreen, bridge_win.viewscreen_image,
                fname.main_page
            );

            sprintf(text,
    "Client: Memory reload: Destroyed page configuration from `%s'...",
                fname.destroyed_page
            );
            MesgAdd(text, xsw_color.standard_text);
            PageLoadFromFile(
                &bridge_win.destroyed_page,
                bridge_win.viewscreen, bridge_win.viewscreen_image,
                fname.destroyed_page
            );

                
            sprintf(text,
    "Client: Memory reload: Image set referances from `%s'...",
                fname.isr
            );
            MesgAdd(text, xsw_color.standard_text);
            ISRefLoadFromFile(fname.isr);

            sprintf(text,
    "Client: Memory reload: Object create script names from: `%s'...",
                fname.ocsn
            );
            MesgAdd(text, xsw_color.standard_text);
            OCSLoadFromFile(fname.ocsn);

            sprintf(text,
    "Client: Memory reload: Sound scheme from: `%s'...",
                fname.sound_scheme
            );
            MesgAdd(text, xsw_color.standard_text);
            SSLoadFromFile(fname.sound_scheme);

            sprintf(text,
    "Client: Memory reload: Completed."
            );
            MesgAdd(text, xsw_color.standard_text);
        }

        /* ********************************************************** */
        /* Refresh objects in memory. */
        else if(!strcasecmp(arg, "refresh"))
        {
            XSWReclaimGlobalMemory(True);
        }


        /* ********************************************************** */
 
        /* Recreate the selected weapon viewscreen label. */
        VSDrawUpdateWeaponLabel(
            &bridge_win.vs_weapon_image,
            bridge_win.vs_weapon_buf
        );
 


        return(0);
}
