#include "xsw.h"



int CmdHelp(const char *arg)
{
	char larg[256];


        /* Print commands list. */
	if((arg == NULL) ? 1 : ((*arg) == '\0'))
	{
	    MesgAdd(
 "aint     con      discon   help     int      lname    log",
		xsw_color.bp_standard_text
            );
            MesgAdd(
 "lpass    mem      refresh  script   set      synctime version",
                xsw_color.bp_standard_text
            );
            MesgAdd(
 "For additional help on a perticular command type `help [command]'",
                xsw_color.bp_standard_text
            );

	    return(0);
	}

	strncpy(larg, arg, 256);
	larg[256 - 1] = '\0';
	StringStripSpaces(larg);

	/* Automatic interval. */
	if(strpfx(larg, "autoi") ||
           strpfx(larg, "aint")
	)
	{
            MesgAdd(
 "aint [on|off]",
                xsw_color.bp_standard_text
            );
            MesgAdd(
 "Toggles automatic network stream interval (auto interval) tunning on/off.",
                xsw_color.bp_standard_text
            );
	}
	/* Connect. */
	else if(strpfx(larg, "con"))
        {
            MesgAdd(
 "con [url]",
                xsw_color.bp_standard_text
            );  
            MesgAdd(
 "Connects to universe specified in the [url]. If no argument is given then",
                xsw_color.bp_standard_text
            );
            MesgAdd(
 "the universe list window will be mapped.",
                xsw_color.bp_standard_text
            );
        }
        /* Disconnect. */
        else if(strpfx(larg, "dis"))
        {
            MesgAdd(
 "discon",
                xsw_color.bp_standard_text
            );
            MesgAdd(
 "Disconnects from current connected universe.",
               xsw_color.bp_standard_text
            );
        }
        /* Exit/quit. */
/* No longer used, see key mapping function for exit which maps the
   exit prompt.
        else if(strpfx(larg, "exi") ||
                strpfx(larg, "qu")
        )
        {
            MesgAdd(
 "exit",
                xsw_color.bp_standard_text
            );
            MesgAdd(
 "Exits the program (requires comfermation).",
                xsw_color.bp_standard_text
            );
        }
 */
        /* Help. */
        else if(strpfx(larg, "h") ||
                strpfx(larg, "?")
        )
        {
            MesgAdd(
 "help [command]",
                xsw_color.bp_standard_text
            );
            MesgAdd(
 "Prints detailed help about [command] or if [command] is not given,",
                xsw_color.bp_standard_text
            );
            MesgAdd(
 "then a list of commands available on this program is printed.",
                xsw_color.bp_standard_text
            );
        }
	/* Interval. */
        else if(strpfx(larg, "int"))
	{
            MesgAdd(
 "int [milliseconds]",
                xsw_color.bp_standard_text
            );
            MesgAdd(
 "Adjusts the network streaming interval in milliseconds. This command",
                xsw_color.bp_standard_text
            );
            MesgAdd(
 "is only available when automatic interval tuning is off.",
                xsw_color.bp_standard_text
            );
	}
        /* Login name. */
        else if(strpfx(larg, "login_n") ||
                strpfx(larg, "loginn") ||
                strpfx(larg, "lna")
        )
        {
            MesgAdd(
 "lname [login_name]",
                xsw_color.bp_standard_text
            );
            MesgAdd(
 "Sets the default login name to be used when connecting with a URL",
                xsw_color.bp_standard_text
            );
            MesgAdd(
 "that specifies no login name.",
                xsw_color.bp_standard_text
            );
        }       
        /* Log. */
        else if(!strcmp(larg, "log"))
        {
            MesgAdd(
 "log [on|off] [filename]", 
                xsw_color.bp_standard_text
            );
            MesgAdd(
 "Turns logging on or off.  If [filename] is specified when turning logging",
                xsw_color.bp_standard_text
            );
            MesgAdd(
 "on, then logging will be saved to the file [filename].", 
                xsw_color.bp_standard_text
            );
        }
        /* Login password. */
        else if(strpfx(larg, "login_p") ||
                strpfx(larg, "loginp") ||
                strpfx(larg, "lpa")
        )
        {
            MesgAdd(
 "lpass [login_password]",
                xsw_color.bp_standard_text
            );
            MesgAdd(
 "Sets the default login password to be used when connecting with a URL",
                xsw_color.bp_standard_text
            );
            MesgAdd(
 "that specifies no login password.",
                xsw_color.bp_standard_text 
            );
        }
        /* Memory. */
        else if(strpfx(larg, "mem"))
        {
            MesgAdd(
 "mem [operation]",
                xsw_color.bp_standard_text
            );
            MesgAdd(
 "Prints memory used by this program. If [operation] is specified, then",
                xsw_color.bp_standard_text
            );
            MesgAdd(
 "that memory operation is performed. Available values for [operation] are;",
                xsw_color.bp_standard_text
            );
            MesgAdd(
 "refresh, reload.",
                xsw_color.bp_standard_text
            );
        }
        /* Refresh. */
        else if(strpfx(larg, "ref"))
        {
            MesgAdd(
 "refresh",
                xsw_color.bp_standard_text
            );
            MesgAdd(
 "Refreshes local universe listing and other related information/resources",
                xsw_color.bp_standard_text
            );
            MesgAdd(
 "(must be connected to use this command).",
                xsw_color.bp_standard_text
            );
        }
	/* Script. */
        else if(strpfx(larg, "serversc") ||
                strpfx(larg, "scr")
        )
	{
            MesgAdd(
 "script [filename] [arg...]",
                xsw_color.bp_standard_text
            );
            MesgAdd(
 "Uploads a script file [filename] to the server.  If no arguments",
                xsw_color.bp_standard_text
            );
            MesgAdd(
 "are given the file browser is mapped to select a script file.",
                xsw_color.bp_standard_text
            );
	}
        /* Set. */
        else if(!strcmp(larg, "set") ||
                !strcmp(larg, "se")
        )
        {
            MesgAdd(
 "set [parameter][=<value>]",
                xsw_color.bp_standard_text
            );
            MesgAdd(
 "Sets [parameter] to <value>.  If no arguments are given, then a list",
                xsw_color.bp_standard_text
            );
            MesgAdd(
 "of available [parameter]s and their current <value>s are listed.",
                xsw_color.bp_standard_text
            );
        }
        /* Synctime. */
        else if(strpfx(larg, "sync"))
        {
            MesgAdd(
 "synctime",
                xsw_color.bp_standard_text
            );
            MesgAdd(
 "Reset and syncronize timmers, this action is performed automatically",
                xsw_color.bp_standard_text
            );
            MesgAdd(
 "by the program as needed. You may need to use this manual command if",
                xsw_color.bp_standard_text
            );
            MesgAdd(
 "timming appears extremely off sync (which rarly occures).",
                xsw_color.bp_standard_text
            );
        }
        /* Version. */
        else if(strpfx(larg, "ver"))
        {
            MesgAdd(
 "version",
                xsw_color.bp_standard_text 
            );
            MesgAdd(
 "Prints the version number of this program.",
                xsw_color.bp_standard_text
            );
        }


	else
	{
	    char text[1024];

	    /* No help topic available. */
	    sprintf(
		text,
		"No help topic available for command `%s'",
		larg
	    );
            MesgAdd(
		text,
                xsw_color.bp_standard_text
            );
	}


        return(0);
}
