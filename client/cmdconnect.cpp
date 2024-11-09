#include "xsw.h"
#include "univlist.h"


int CmdConnect(const char *arg)
{
        int status;


        /* If no argument, then map universe list window. */
        if(arg == NULL)
        {
            UnivListMap();
            return(0);
        }
        else if((*arg) == '\0')
        {
            UnivListMap();
            return(0);
        }

        /* Do connection. */
        status = XSWDoConnect(arg);

        return(status);
}
