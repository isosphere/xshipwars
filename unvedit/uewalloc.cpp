// unvedit/uewalloc.cpp


/*
#include <stdio.h>
#include <malloc.h>
*/
#include "uew.h"



int UEWIsAllocated(int n)
{
	if((uew == NULL) ||
           (n < 0) ||
           (n >= total_uews)
	)
	    return(0);
	else if(uew[n] == NULL)
	    return(0);
	else
	    return(1);
}


/*
 *	Allocates a new universe edit window structure.
 */
int UEWAllocate()
{
	int i, n;


	if(total_uews < 0)
	    total_uews = 0;

	for(i = 0; i < total_uews; i++)
	{
	    if(uew[i] == NULL)
		break;
	}
	if(i < total_uews)
	{
	    n = i;
	}
	else
	{
	    n = total_uews;
	    total_uews++;
	    uew = (uew_struct **)realloc(
		uew,
		total_uews * sizeof(uew_struct *)
	    );
	    if(uew == NULL)
	    {
		total_uews = 0;
		return(-1);
	    }
	}

	uew[n] = (uew_struct *)calloc(
	    1,
	    sizeof(uew_struct)
	);
	if(uew[n] == NULL)
	{
	    return(-1);
	}



	return(n);
}


void UEWDelete(int n)
{
	if(!UEWIsAllocated(n))
	    return;


	/* Deallocate resources. */
	UEWDestroy(n);


	/* Free structure itself. */
	free(uew[n]);
	uew[n] = NULL;


	return;
}


void UEWDeleteAll()
{
	int i;


	for(i = 0; i < total_uews; i++)
	    UEWDelete(i);

	free(uew);
	uew = NULL;

	total_uews = 0;


	return;
}



