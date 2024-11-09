#include "xsw.h"
            
 
int CmdAutoInterval(const char *arg)
{
	char text[256];


        if((arg == NULL) ? 1 : (*arg == '\0'))
        {
	    sprintf(
		text,
		"autointerval current value: %s",
		((auto_interval_tune.state) ? "on" : "off")
	    );
            MesgAdd(text, xsw_color.standard_text);
        }
	else
	{
	    auto_interval_tune.state = StringIsYes(arg);

            sprintf(
                text,
                "autointerval set to: %s",
                ((auto_interval_tune.state) ? "on" : "off")
            );
            MesgAdd(text, xsw_color.standard_text);
	}

        return(0);
}
