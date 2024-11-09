#include "xsw.h"
#include "net.h"
 
 
int CmdLoginName(const char *arg)
{
        char stringa[XSW_OBJ_NAME_MAX + 256];


        /* Print current value? */
        if((arg == NULL) ? 1 : ((*arg) == '\0'))
        {
            sprintf(stringa, "login_name: %s", net_parms.login_name);
            MesgAdd(stringa, xsw_color.bp_standard_text);

            return(0);
        }


        /* Set the new login name. */
        strncpy(net_parms.login_name, arg, XSW_OBJ_NAME_MAX);
        net_parms.login_name[XSW_OBJ_NAME_MAX - 1] = '\0';
	StringStripSpaces(net_parms.login_name);

        /* Print new value. */
        sprintf(stringa, "login_name: %s", net_parms.login_name);
        MesgAdd(stringa, xsw_color.bp_standard_text);


        return(0);
}
