#include "xsw.h"

int CmdDebug(const char *arg)
{
        char *strptr;
        char larg[256];
        char stringa[256];


        if((arg == NULL) ? 1 : (arg[0] == '\0'))
        {
            sprintf(stringa, "debug level: %i  debug value: %.4f",
                debug.level, debug.val
            );
            MesgAdd(stringa, xsw_color.standard_text);
            MesgAdd("Usage: debug <level>[=value]", xsw_color.standard_text);
        
	    MesgAdd(
 "<levels> can be: DEBUG_LEVEL_NONE = 0, DEBUG_LEVEL_ALL = 1",
                xsw_color.standard_text
	    );
	    MesgAdd(
 "DEBUG_LEVEL_MEMORY = 2, DEBUG_LEVEL_NETWORK = 3",
                xsw_color.standard_text
	    );

            return(0);
        }  
        strncpy(larg, arg, 256);
        larg[255] = '\0';


        strptr = strchr(larg, '=');
        if(strptr != NULL)
            debug.val = atof(strptr + 1);

        if(strptr != NULL)
	    *strptr = '\0';
 
        debug.level = atoi(larg);


        sprintf(
	    stringa,
	    "debug: level %i: value %.4f",
	    debug.level, debug.val
	);
        MesgAdd(stringa, xsw_color.standard_text);

        return(0);
}
