/*
                            Sound Scheme Management

	Functions:

	int SSIsAllocated(int n)
	ss_item_struct *SSGetPtr(int n)

	int SSAllocate(char *path)
        int SSAllocateExplicit(int n) 
	void SSDelete(int n)
	void SSDeleteAll()

	---

 */

#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
#include <string.h>

#include "../include/string.h"

#include "ss.h"


/*
 *	Checks if sound scheme item n is valid and allocated.
 */
int SSIsAllocated(int n)
{
        if((n < 0) ||
           (n >= total_ss_items) ||
           (ss_item == NULL)
        )
            return(0);
        else if(ss_item[n] == NULL)
            return(0);
        else
            return(1);
}

/*
 *	Gets pointer to sound scheme item n, can return NULL if the
 *	sound scheme item is not valid or allocated.
 */
ss_item_struct *SSGetPtr(int n)
{
        if(SSIsAllocated(n))
            return(ss_item[n]);
        else
            return(NULL);
}


/*
 *	Allocates a sound scheme item and returns its index number
 *	or -1 on error.
 */
int SSAllocate(char *path)
{
        int i, n;
	ss_item_struct *ss_ptr, **ptr;


	/* Sanitize total. */
	if(total_ss_items < 0)
	    total_ss_items = 0;
 
	/* Check for available pointer. */
        for(i = 0, ptr = ss_item;
            i < total_ss_items;
            i++, ptr++
	)
        {
            if(*ptr == NULL)
                break;
        }
        if(i < total_ss_items)
        {
	    /* Found available pointer. */
            n = i;
        }
        else
        {
	    /* Need to allocate more pointers. */
            n = total_ss_items;
            total_ss_items++;

            ss_item = (ss_item_struct **)realloc(
                ss_item,
                total_ss_items * sizeof(ss_item_struct *)
            );
            if(ss_item == NULL)
            {
                total_ss_items = 0;
                return(-1);
            }

	    /* Reset newly allocated pointer index to NULL. */
	    ss_item[n] = NULL;
        }

	/* Allocate a new sound scheme item. */
        ss_item[n] = (ss_item_struct *)calloc(
	    1, sizeof(ss_item_struct)
        );
        if(ss_item[n] == NULL)
            return(-1);


	/* Get pointer to sound scheme item. */
	ss_ptr = ss_item[n];


        /* Reset values. */
	ss_ptr->path = StringCopyAlloc(path);


        return(n);
}

/*
 *	Allocates a sound scheme item explicitly, returns -1 on error
 *	or 0 on success.
 */
int SSAllocateExplicit(int n)
{
	int i, prev_total;


	/* Requested number must be valid. */
	if(n < 0)
	    return(-1);


	/* Delete sound scheme n if already allocated. */
	SSDelete(n);


	/* Sanitize total. */
	if(total_ss_items < 0)
	    total_ss_items = 0;


	/* Need to allocate more pointers? */
	if(n >= total_ss_items)
	{
	    /* Record previous total and set new total. */
	    prev_total = total_ss_items;
	    total_ss_items = n + 1;

	    /* Allocate more pointers. */
	    ss_item = (ss_item_struct **)realloc(
	        ss_item,
	        total_ss_items * sizeof(ss_item_struct *)
	    );
	    if(ss_item == NULL)
	    {
	        total_ss_items = 0;
	        return(-1);
	    }

	    /* Reset newly allocated pointers. */
	    for(i = prev_total; i < total_ss_items; i++)
		ss_item[i] = NULL;
	}


	ss_item[n] = (ss_item_struct *)calloc(1, sizeof(ss_item_struct));
	if(ss_item[n] == NULL)
	    return(-1);


	return(0);
}

/*
 *	Delete sound scheme item n and all its allocated resources.
 */
void SSDelete(int n)
{
	ss_item_struct *ss_ptr;


        if(SSIsAllocated(n))  
            ss_ptr = ss_item[n];
	else
	    return;


	/* Free resources. */
	free(ss_ptr->path);

	/* Free structure itself. */
        free(ss_ptr);
        ss_item[n] = NULL;

        return;
}

/*
 *	Deallocate all sound scheme items.
 */
void SSDeleteAll()
{
        int i;

        for(i = 0; i < total_ss_items; i++)
            SSDelete(i);

        free(ss_item);
        ss_item = NULL;

        total_ss_items = 0;

        return;
}
