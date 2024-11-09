#include "xsw.h"
#include "keymap.h"


/*
 *      Front end to parse command input.
 */
int CmdHandleInput(const char *input)
{
        char *strptr;

        char stringa[CLIENT_CMD_MAX];
        char stringb[CLIENT_CMD_MAX];
        char arg[CLIENT_CMD_MAX];
        char command[CLIENT_CMD_MAX];

        int status = 0;


        /* Input command must be atleast 1 character long. */
        if(input == NULL)
            return(-1);
        if((*input) == '\0')
            return(-1);
         
        /* Copy input string to stringa and sanitize stringa. */
        strncpy(stringa, input, CLIENT_CMD_MAX);
        stringa[CLIENT_CMD_MAX - 1] = '\0';
	StringStripSpaces(stringa);        

        strncpy(stringb, stringa, CLIENT_CMD_MAX);
        stringb[CLIENT_CMD_MAX - 1] = '\0';
        strncpy(stringa, stringb, CLIENT_CMD_MAX);
        stringa[CLIENT_CMD_MAX - 1] = '\0';
 
        stringa[CLIENT_CMD_MAX - 1] = '\0';
        strptr = strchr(stringa, '\n');
        if(strptr != NULL)
            *strptr = '\0';

 
        /* ******************************************************** */
        /* Begin parsing. */

	strptr = strchr(stringa, ' ');
        if(strptr != NULL)
        {
            strncpy(arg, (strptr + 1), CLIENT_CMD_MAX);
            arg[CLIENT_CMD_MAX - 1] = '\0';
            *strptr = '\0';
            strncpy(command, stringa, CLIENT_CMD_MAX);
            command[CLIENT_CMD_MAX - 1] = '\0';
        }
        else
        {
            strncpy(arg, "", CLIENT_CMD_MAX);
            strncpy(command, stringa, CLIENT_CMD_MAX);
            command[CLIENT_CMD_MAX - 1] = '\0';
        }
	StringStripSpaces(command);


        /* ****************************************************** */
        /* Handle command. */

        /* Debug. */
        if(!strcasecmp(command, "debug"))
        {
            status = CmdDebug(arg);
        }
        /* Test. */
        else if(!strcasecmp(command, "test"))
        {
            status = CmdTest(arg);
        }
        /* Help. */
        else if(!strcasecmp(command, "help") ||
                !strcasecmp(command, "hel") ||
                !strcasecmp(command, "he") ||
                !strcasecmp(command, "h") ||
                !strcasecmp(command, "?")
        )
        {
            status = CmdHelp(arg);
        }
        /* Version. */
        else if(!strcasecmp(command, "version") ||
                !strcasecmp(command, "versio") ||
                !strcasecmp(command, "versi") ||
                !strcasecmp(command, "vers") ||
                !strcasecmp(command, "ver")
        )
        {
            status = CmdVersion(arg);
        }
        /* Memory. */
        else if(!strcasecmp(command, "memory") ||
                !strcasecmp(command, "memor") ||
                !strcasecmp(command, "memo") ||
                !strcasecmp(command, "mem")
        )
        {
            status = CmdMemory(arg);
        }
        /* Auto interval tune state. */  
        else if(!strcasecmp(command, "autointerval") ||
                !strcasecmp(command, "autointerva") ||
                !strcasecmp(command, "autointerv") ||
                !strcasecmp(command, "autointer") ||
                !strcasecmp(command, "autointe") ||
                !strcasecmp(command, "autoint") ||
                !strcasecmp(command, "autoi") ||
                !strcasecmp(command, "aint")
        )
        {
            status = CmdAutoInterval(arg);
        }
	/* Script. */
        else if(!strcasecmp(command, "serverscript") ||
                !strcasecmp(command, "servscript") ||
                !strcasecmp(command, "script") ||
                !strcasecmp(command, "scrip") ||
                !strcasecmp(command, "scri") ||
                !strcasecmp(command, "scr") ||
                !strcasecmp(command, "sc")
        )
        {
            status = CmdServScript(arg);
        }
        /* Set. */
        else if(!strcasecmp(command, "set") ||
                !strcasecmp(command, "se") ||
                !strcasecmp(command, "s")
        )
        {
            status = CmdSet(arg);
        }
        /* Login name. */
        else if(!strcasecmp(command, "lname") ||
                !strcasecmp(command, "lnam") ||
                !strcasecmp(command, "lna") ||
                !strcasecmp(command, "ln")
        )
        {
            status = CmdLoginName(arg); 
        }
        /* Login password. */
        else if(!strcasecmp(command, "lpassword") ||
                !strcasecmp(command, "lpasswor") ||
                !strcasecmp(command, "lpasswo") ||
                !strcasecmp(command, "lpassw") ||
                !strcasecmp(command, "lpass") ||
                !strcasecmp(command, "lpas") ||
                !strcasecmp(command, "lpa") ||
                !strcasecmp(command, "lp") ||
                !strcasecmp(command, "lpasswd")   
        )
        {
            status = CmdLoginPassword(arg);
        }
	/* Log. */
        else if(!strcasecmp(command, "log"))
	{
	    status = CmdLog(arg);
	}
        /* Connect to server. */
        else if(!strcasecmp(command, "connect") ||
                !strcasecmp(command, "connec") ||
                !strcasecmp(command, "conne") ||
                !strcasecmp(command, "conn") ||
                !strcasecmp(command, "con") ||
                !strcasecmp(command, "online") ||
                !strcasecmp(command, "ol")
        )
        {
            status = CmdConnect(arg);
        }
        /* Disconnect. */
        else if(!strcasecmp(command, "disconnect") ||
                !strcasecmp(command, "disconnec") ||
                !strcasecmp(command, "disconne") ||
                !strcasecmp(command, "disconn") ||
                !strcasecmp(command, "discon") ||
                !strcasecmp(command, "disco") ||
                !strcasecmp(command, "disc") ||
                !strcasecmp(command, "dis") ||
                !strcasecmp(command, "bye") ||
                !strcasecmp(command, "handup") || 
                !strcasecmp(command, "hup")
        )
        {
            status = CmdDisconnect(arg);
        }
        /* Refresh. */
        else if(!strcasecmp(command, "refresh") ||
                !strcasecmp(command, "refres") ||
                !strcasecmp(command, "refre") ||
                !strcasecmp(command, "refr") ||
                !strcasecmp(command, "ref")
        )
        {
            status = CmdRefresh(arg);
        }
        /* Interval. */
        else if(!strcasecmp(command, "interval") ||
                !strcasecmp(command, "interva") ||
                !strcasecmp(command, "interv") ||
                !strcasecmp(command, "inter") ||
                !strcasecmp(command, "inte") ||
                !strcasecmp(command, "int")
        )
        {
            status = CmdNetInterval(arg);
        }
        /* Quit/exit. */
        else if(!strcasecmp(command, "quit") ||  
                !strcasecmp(command, "qui") ||  
                !strcasecmp(command, "qu") ||  
                !strcasecmp(command, "q") ||  
                !strcasecmp(command, "exit") ||
                !strcasecmp(command, "exi")
        )
        {
	    /* No longer in use, see the key mapping which activates the
	     * mapping of the exit prompt.
	     */
	    sprintf(stringa,
		"This command is depreciated, to exit press `%s'.",
		OSWGetKeyCodeName(xsw_keymap[XSW_KM_EXIT].keycode)
	    );
	    MesgAdd(stringa, xsw_color.standard_text);
        }
        /* Synctime. */
        else if(!strcasecmp(command, "synctime") ||
                !strcasecmp(command, "synctim") ||
                !strcasecmp(command, "syncti") ||
                !strcasecmp(command, "synct") ||
                !strcasecmp(command, "sync") ||
                !strcasecmp(command, "syn") ||
                !strcasecmp(command, "sy")
        )
        {
            status = CmdSynctime(arg);
        }
        else 
        {
            sprintf(stringa,
                "%s: no such command or macro.",
                command
            );
            MesgAdd(stringa, xsw_color.standard_text);
        }


        return(status);
}
