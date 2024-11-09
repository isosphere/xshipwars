/*

	Functions:

	int SiteBanIsAllocated(int n)

	int SiteBanAdd(
	        siteban_ip_union *ip,
	        int restrict
	)
	int SiteBanRemoveIP(siteban_ip_union *ip)

	void SiteBanDelete(int n)
	void SiteBanDeleteAll()


 */

#include <stdio.h>
#include <malloc.h>
#include <string.h>

#include "siteban.h"


siteban_struct **siteban;
int total_sitebans;


/*
 *	Checks if the site ban entry number is allocated.
 */
int SiteBanIsAllocated(int n)
{
	if((siteban == NULL) ||
           (n < 0) ||
           (n >= total_sitebans)
	)
	    return(0);
	else if(siteban[n] == NULL)
	    return(0);
	else
	    return(1);
}


/*
 *	Checks if ip is banned in accordance with the restrict
 *	level.
 */
int SiteBanIsBanned(siteban_ip_union *ip, int restrict)
{
	int i;
        siteban_struct **ptr;


        for(i = 0, ptr = siteban;
            i < total_sitebans;
            i++, ptr++
        )
        {
            if(*ptr == NULL)
                continue;

/* Just check if ip matches, do not check restrict level for now... */
            if((*ptr)->ip.whole == ip->whole)
                return(1);
        }


	return(0);
}


/*
 *	Adds ip to the site ban list.
 */
int SiteBanAdd(
	siteban_ip_union *ip,
	int restrict		/* Just 0 for now. */
)
{
	int i, n;
	siteban_struct **ptr;


	if(ip == NULL)
	    return(-1);


	/* Sanitize total. */
	if(total_sitebans < 0)
	    total_sitebans = 0;


	/* Check if IP is already in the list. */
	for(i = 0, ptr = siteban;
            i < total_sitebans;
            i++, ptr++
	)
	{
	    if(*ptr == NULL)
		continue;

	    if((*ptr)->ip.whole == ip->whole)
	    {
		/* Just update entry. */
		(*ptr)->restrict = restrict;

		return(0);
	    }
	}

        /* Check for available entry. */
        for(i = 0, ptr = siteban;
            i < total_sitebans;
            i++, ptr++
        )
        {
            if(*ptr == NULL)
                break;
 	}
	if(i < total_sitebans)
	{
	    n = i;
	}
	else
	{
	    n = total_sitebans;
	    total_sitebans++;

	    siteban = (siteban_struct **)realloc(
		siteban,
		total_sitebans * sizeof(siteban_struct *)
	    );
	    if(siteban == NULL)
	    {
		total_sitebans = 0;
		return(-1);
	    }

	    siteban[n] = NULL;
	}

	/* Allocate new structure. */
	siteban[n] = (siteban_struct *)calloc(
	    1,
	    sizeof(siteban_struct)
	);
	if(siteban[n] == NULL)
	{
	    return(-1);
	}


	siteban[n]->ip.whole = ip->whole;
	siteban[n]->restrict = restrict;


	return(0);
}


/*
 *	Removes ip from site ban list. Returns 0 if successfully
 *	removed or -1 if there was no such ip or error.
 *
 *	Warning: siteban pointer array will be reallocated
 *	in this function!
 */
int SiteBanRemoveIP(siteban_ip_union *ip)
{
        int i, n, d = 0;


	if(ip == NULL)
	    return(-1);


	if(total_sitebans < 0)
	    total_sitebans = 0;


        for(i = 0; i < total_sitebans; i++)
        {
            if(siteban[i] == NULL)
                continue;

	    if(siteban[i]->ip.whole == ip->whole)
	    {
		SiteBanDelete(i);
		d++;

		/* Shift entries and reallocate. */
		total_sitebans--;

		for(n = i; n < total_sitebans; n++)
		    siteban[n] = siteban[n + 1];

		if(total_sitebans > 0)
		{
		    siteban = (siteban_struct **)realloc(
			siteban,
			total_sitebans * sizeof(siteban_struct *)
		    );
		    if(siteban == NULL)
		    {
			total_sitebans = 0;
			return(-1);
		    }
		}
		else
		{
		    free(siteban);
		    siteban = NULL;

		    total_sitebans = 0;
		}
	    }
        }


	return((d > 0) ? 0 : -1);
}


void SiteBanDelete(int n)
{
	if(!SiteBanIsAllocated(n))
	    return;


	free(siteban[n]);
	siteban[n] = NULL;

	return;
}

void SiteBanDeleteAll()
{
	int i;

	for(i = 0; i < total_sitebans; i++)
	    SiteBanDelete(i);

	free(siteban);
	siteban = NULL;

	total_sitebans = 0;


	return;
}
