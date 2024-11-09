#include "xsw.h"


int CmdExit(const char *arg)
{
	const char *strptr;

	/* Check for exit option. */
	if(arg != NULL)
	{
	    /* No save on exit. */
	    strptr = strstr(arg, "nosave");
	    if(strptr != NULL)
	    {
		option.save_on_exit = 0;
	    }
            /* No save on exit. */
            strptr = strstr(arg, "no_save");
            if(strptr != NULL)
            {
                option.save_on_exit = 0;
            }

	    /* Yes comfermation (first letter must be a `y'). */
	    if(strcasepfx(arg, "y") ||
               strcasepfx(arg, "exit") ||
               strcasepfx(arg, "quit")
	    )
	    {
                /*   Switch runlevel to 1 to notify rest of program to
                 *   begin shutdown prcoess.
                 */
                runlevel = 1;
	    }
	}

        return(0);
}

