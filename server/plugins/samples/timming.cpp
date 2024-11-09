/*
                      ShipWars Plugin Sample Source


        To compile this, type:

        cc timming.c -o timming -shared -g


        This program won't run by itself, but is intented to be used
        as a plugin for the ShipWars Server.

	---

	This example shows how to calculate timming, by sending
	a joke question and then an answer. The interval between
	the joke and the answer is 5 seconds and the interval
	between the answer and the next joke is 10 seconds.


	First, a short lecture about timming. ShipWars uses millisecond
	resolution timming, meaning that timming is accurate up
	to 1/1000th of a second. The member cur_millitime in the
	plugin_data_ref_struct structure (passed on each function
	that is called by the ShipWars Server) contains a pointer
	to the variable that contains the current time of day in
	milliseconds.

	Now the most important fact of the above that will impact
	you is that the value in that variable will `cycle'
	atleast once every 24 hours. So to compensate for that your
	program needs a function that will reset each `time schedual'
	to 0 whenever the cur_millitime is less than the last
	cur_millitime.

	NOTE: The functions below have been wrapped with extern "C".
        When using C instead of C++, this should be removed.
 */

#include "../include/plugins.h"

extern "C" {
int joke_code;
time_t last_millitime;
time_t next_joke;

static char *joke_list[] = {
	"Why did the chicken cross the star system?",
	"There was a Foster Farms Borg chasing him.",

	"How many Klingons does it take to screw in a light bulb?",
	"Bah! None, Klingons are not afraid of the dark!",

	"Why did the Maqui shoot the Cardassian?",
	"Because he was there.",

	"What do Riker and Picard really do the in the Ready Room?",
	"They behave like mature non-gay adults (and you believe that?)",

	"Who was the very first Borg?",
	"Bill Gates!",

	"How come Mog never spoke in the DS9 series?",
	"He was in a huff ever since Dax rejected him."
};

/*
 *	This function is just for our plugin, it resets the `next'
 *	time when the current time of day has cycled.
 */
void ResetTimmers(void)
{
	next_joke = 0;

	return;
}

SWPLUGIN_INIT_FUNCRTN SWPLUGIN_INIT_FUNC(SWPLUGIN_INIT_FUNCINPUT)
{
	/* Upon initialization, reset our globals. */
	ResetTimmers();
	joke_code = 0;

	/* Get current time of day in milliseconds. */
	last_millitime = *(in->cur_millitime);


	return(0);
}

SWPLUGIN_MANAGE_FUNCRTN SWPLUGIN_MANAGE_FUNC(SWPLUGIN_MANAGE_FUNCINPUT)
{
        int (*con_notify)(int, char *);
	time_t cur_millitime;
#define total_joke_lines	(sizeof(joke_list) / sizeof(char *))


	/* Get pointer to connection notify functino. */
	con_notify = in->con_notify_fptr;


	/* Get current time in milliseconds from ShipWars Server. */
	cur_millitime = *(in->cur_millitime);

	/* Now check if the time `cycled' */
	if(cur_millitime < last_millitime)
		ResetTimmers();

	last_millitime = cur_millitime;


	/* Is it time to print a joke or an answer? */
	if(next_joke <= cur_millitime)
	{
	    if((joke_code >= 0) && (joke_code < total_joke_lines))
		con_notify(-1, joke_list[joke_code]);

	    if(joke_code & 1)
	    {
		/* Odd number (answer). */

		/* Schedual next time to print joke. */
		next_joke = cur_millitime + 10000;
	    }
	    else
	    {
		/* Even number (joke). */

		/* Schedual next time to print answer. */
		next_joke = cur_millitime + 5000;
	    }

	    /* Increment joke code. */
	    joke_code++;

	    /* Done with our jokes? */
	    if(joke_code > total_joke_lines)
	    {
		/* Yup, we're all done! */
		joke_code = 0;

		return(-1);	/* Have the server unload this plugin. */
	    }
	}

	return(0);
}

SWPLUGIN_SHUTDOWN_FUNCRTN SWPLUGIN_SHUTDOWN_FUNC(SWPLUGIN_SHUTDOWN_FUNCINPUT)
{
	return;
}

} // extern "C"
