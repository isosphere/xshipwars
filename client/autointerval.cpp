/*
                             Automatic Interval

	Functions:

	int AIntvTuneHandleAdjust(void)

	---

 */

#include "xsw.h"
#include "net.h"


/*
 *	Handles automatic adjusting of the network interval.
 *
 *	Calls to this function must be timmed correctly.
 */
int AIntvTuneHandleAdjust(void)
{
	long total_bytes, delta;


	/* Must be connected or logging in. */
        if(net_parms.connection_state == CON_STATE_NOT_CONNECTED)
	{
	    /* Not connected or not logged in, so reset counters. */
            loadstat.sx_ilast = loadstat.sx_interval;
	    loadstat.rx_ilast = loadstat.rx_interval;
            loadstat.sx_interval = 0;
            loadstat.rx_interval = 0;

	    return(0);
	}

        /* Is automatic interval tunning on? */
        if(!auto_interval_tune.state)
        {
	    /* Automatic interval tunning is not on, so reset counters. */
            loadstat.sx_ilast = loadstat.sx_interval;
            loadstat.rx_ilast = loadstat.rx_interval;
            loadstat.sx_interval = 0;
            loadstat.rx_interval = 0;
         
            return(0);
        }


	/* Get total bytes recieved and sent. */
	total_bytes = loadstat.rx_interval + loadstat.sx_interval;

	/* Convert total_bytes from per 1000 ms interval to per
	 * auto_interval_tune.interval ms interval. This should not
	 * be needed since auto_interval_tune.interval should be
	 * 1000 ms but we check this just incase.
	 */
	if(auto_interval_tune.interval > 0)
	    total_bytes = total_bytes * 1000 / auto_interval_tune.interval;


	/* Calculate delta of bandwidth and actual total bytes sent and
	 * recieved.
	 */
	delta = total_bytes - loadstat.net_load_max;
	if(delta < 0)
	{
	    /* Under bandwidth, so decrease interval. */
            net_parms.net_int -= 50;
	}
	else
	{
	    /* Over bandwidth, so increase interval. */
            net_parms.net_int += 50;
	}


	/* Sanitize interval. */
        if(net_parms.net_int > MAX_SERVER_UPDATE_INT) 
            net_parms.net_int = MAX_SERVER_UPDATE_INT;
        if(net_parms.net_int < MIN_SERVER_UPDATE_INT)  
            net_parms.net_int = MIN_SERVER_UPDATE_INT;

	/* If in energy saver mode, then interval must be SERVER_DEF_INT
         * or greater.
	 */
	if(option.energy_saver_mode)
	{
	    if(net_parms.net_int < SERVER_DEF_INT)
		net_parms.net_int = SERVER_DEF_INT;
	}


	/* Send interval update to server. */
        NetSendSetInterval();


	/* Reset sx and rx byte counters. */
        loadstat.sx_ilast = loadstat.sx_interval;
        loadstat.rx_ilast = loadstat.rx_interval;

	loadstat.sx_interval = 0;
	loadstat.rx_interval = 0;


	return(0);
}
