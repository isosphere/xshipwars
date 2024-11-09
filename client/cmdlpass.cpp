#include "xsw.h"
#include "net.h"

 
int CmdLoginPassword(const char *arg)
{
        char stringa[XSW_OBJ_PASSWORD_MAX + 256];


        /* Print current value? */
        if((arg == NULL) ? 1 : ((*arg) == '\0'))
        {
            sprintf(stringa, "login_password: %s",
                net_parms.login_password
	    );
            MesgAdd(stringa, xsw_color.bp_standard_text);
            return(0);
        }
         
        /* Set login password. */
        strncpy(net_parms.login_password, arg, XSW_OBJ_PASSWORD_MAX);
        net_parms.login_password[XSW_OBJ_PASSWORD_MAX - 1] = '\0';


        sprintf(stringa, "login_password: %s", net_parms.login_password);
        MesgAdd(stringa, xsw_color.bp_standard_text);


        return(0);
}
