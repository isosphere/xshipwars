#include "xsw.h"



int CmdVersion(const char *arg)
{
        char stringa[256];


        sprintf(
	    stringa,
            "%s - Version %s.",
            PROG_NAME,
            PROG_VERSION
        );
        MesgAdd(stringa, xsw_color.standard_text);

        return(0);
}

