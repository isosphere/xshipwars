#include <stdio.h>
#include <malloc.h>

#include "mon.h"


int MonIsAllocated(int n)
{
	if((monitor == NULL) ||
           (n < 0) ||
           (n >= total_monitors)
	)
	    return(0);
	else if(monitor[n] == NULL)
	    return(0);
	else
	    return(1);
}

int MonAllocate()
{
        int i, n;


        if(total_monitors < 0)
            total_monitors = 0;

        for(i = 0; i < total_monitors; i++)
        {
            if(monitor[i] == NULL)
                 break;
        }
        if(i < total_monitors)
        {
            n = i;
        }
        else
        {
            n = total_monitors;
            total_monitors++;   
            monitor = (monitor_struct **)realloc(
                monitor,  
                total_monitors * sizeof(monitor_struct *)
            );
            if(monitor == NULL)
            {
                total_monitors = 0;
                return(-1);
            }
        }

        monitor[n] = (monitor_struct *)calloc(
            1,   
            sizeof(monitor_struct)
        );
        if(monitor[n] == NULL)
        {
            return(-1);   
        }


        return(n);
}

void MonDelete(int n)
{
        if(!MonIsAllocated(n))
            return;


        /* Deallocate resources. */
        MonDestroy(monitor[n]);
                
                
        /* Free structure itself. */
        free(monitor[n]);
        monitor[n] = NULL;

        return;
}

void MonDeleteAll()
{
        int i;


        for(i = 0; i < total_monitors; i++)
            MonDelete(i);

        free(monitor);
        monitor = NULL;

        total_monitors = 0;


        return;
}

void MonReclaim()
{
	int i, h;


	for(i = 0, h = -1; i < total_monitors; i++)
	{
	    if(monitor[i] == NULL)
		continue;

	    if(monitor[i]->toplevel == 0)
		continue;

	    h = i;
	}

	total_monitors = h + 1;

	if(total_monitors > 0)
	{
            monitor = (monitor_struct **)realloc(
                monitor,
                total_monitors * sizeof(monitor_struct *)
            );
            if(monitor == NULL)
            {
                total_monitors = 0;
                return;
            }
        }
	else
	{
	    free(monitor);
	    monitor = NULL;

	    total_monitors = 0;
	}


	return;
}
