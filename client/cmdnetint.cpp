#include "xsw.h"
#include "net.h"


int CmdNetInterval(const char *arg)
{
        char stringa[80];


        /* Not connected? */
        if(net_parms.connection_state == CON_STATE_NOT_CONNECTED)
        {
            MesgAdd(
		"Interval: Not connected.",
		xsw_color.bp_standard_text
	    );
            return(-1);
        }

        /* Print current interval? */
        if((arg == NULL) ? 1 : ((*arg) == '\0'))
        {
            sprintf(stringa,
		"interval: %ld milliseconds",
                net_parms.net_int
	    );
            MesgAdd(stringa, xsw_color.bp_standard_text);
            return(0);
        }

        /* Set new interval. */
        net_parms.net_int = atol(arg);

        /* Sanitize net interval. */
        if(net_parms.net_int > MAX_SERVER_UPDATE_INT)
            net_parms.net_int = MAX_SERVER_UPDATE_INT;
        if(net_parms.net_int < MIN_SERVER_UPDATE_INT)
            net_parms.net_int = MIN_SERVER_UPDATE_INT;


        /* Send interval to server. */
        NetSendSetInterval();

        /* Redraw all bridge windows. */
	BridgeWinDrawAll();

        /* Print new interval. */
        sprintf(stringa,
	    "interval: %ld milliseconds",
            net_parms.net_int
	);
        MesgAdd(stringa, xsw_color.bp_standard_text);


        return(0);
}
