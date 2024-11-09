// widgets/wreg.cpp
/*
                       Widget: Regeristration


	Functions:

        void WidgetRegReclaim()
        void WidgetRegDeleteAll()

	void WidgetRegIsRegistered(void *ptr, int type)

	int WidgetRegAdd(void *ptr, int type)
	void WidgetRegDelete(void *ptr)




	---

	These functions are used internally by this widget system
	to track complex widgets.   Often used for knowing which
	complex widget to handle for various timmed callbacks
	such as scroll bar or prompt repeat notification.

	These functions are of no interest by the external code.

 */

#include "../include/widget.h"


void WidgetRegReclaim()   
{
	int i, n;


        for(i = 0, n = -1; i < widget_reg.total_entries; i++)
        {
            if(widget_reg.entry[i] != NULL)
                n = i;
        }
        widget_reg.total_entries = n + 1;
        if(widget_reg.total_entries > 0)
        {   
            widget_reg.entry = (widget_reg_entry_struct **)realloc(
		widget_reg.entry,
                widget_reg.total_entries * sizeof(widget_reg_entry_struct *)
	    );
            if(widget_reg.entry == NULL)
            {
                widget_reg.total_entries = 0;
            }
        }
        else
        {
            free(widget_reg.entry);
            widget_reg.entry = NULL;
            widget_reg.total_entries = 0;
        }


	return;
}


void WidgetRegDeleteAll() 
{
	int i;

	for(i = 0; i < widget_reg.total_entries; i++)
	{
	    if(widget_reg.entry[i] == NULL) continue;


	    free(widget_reg.entry[i]);
	}
	free(widget_reg.entry);
	widget_reg.entry = NULL;

	widget_reg.total_entries = 0;


	return;
}


/*
 *	Checks if pointer to widget is in regeristry matching
 *	type.  Returns ptr on successful match or NULL if no match
 *	can be made.
 */
void *WidgetRegIsRegistered(void *ptr, int type)
{
	int i, m;
	widget_reg_entry_struct **wentry;


	if(ptr == NULL)
	    return(NULL);


	m = widget_reg.total_entries;
	wentry = widget_reg.entry;

	for(i = 0; i < m; i++, wentry++)
	{
	    if(*wentry == NULL)
		continue;

	    /* Does ptr match? */
	    if((*wentry)->ptr == ptr)
	    {
		/* Pointers match, does the type match? */
		if((*wentry)->type == type)
		    return(ptr);
	    }
	}


	return(NULL);
}



int WidgetRegAdd(void *ptr, int type)
{
	int i, n;


	if(ptr == NULL)
	    return(-1);


	/* Sanitize total. */
	if(widget_reg.total_entries < 0)
	    widget_reg.total_entries = 0;

	/* Search for available position. */
	for(i = 0; i < widget_reg.total_entries; i++)
	{
	    if(widget_reg.entry[i] == NULL)
		break;
	}
	if(i < widget_reg.total_entries)
	{
	    n = i;
	}
	else
	{
	    /* Need to allocate more pointers. */
	    n = widget_reg.total_entries;

	    widget_reg.total_entries++;
	    widget_reg.entry = (widget_reg_entry_struct **)realloc(
		widget_reg.entry,
		widget_reg.total_entries * sizeof(widget_reg_entry_struct *)
	    );
	    if(widget_reg.entry == NULL)
	    {
		widget_reg.total_entries = 0;
		return(-1);
	    }
	}

	/* Allocate new structure. */
	widget_reg.entry[n] = (widget_reg_entry_struct *)calloc(
	    1,
	    sizeof(widget_reg_entry_struct)
	);
	if(widget_reg.entry[n] == NULL)
	{
	    return(-1);
	}


	/* Set new values. */
	widget_reg.entry[n]->ptr = ptr;
	widget_reg.entry[n]->type = type;


	return(0);
}

void WidgetRegDelete(void *ptr)
{
	int i;


	if(ptr == NULL)
	    return;


	/* Delete from regeristry. */
	for(i = 0; i < widget_reg.total_entries; i++)
	{
	    if(widget_reg.entry[i] == NULL)
		continue;

	    if(widget_reg.entry[i]->ptr == ptr)
	    {
		free(widget_reg.entry[i]);
		widget_reg.entry[i] = NULL;
	    }
	}


	/* Reclaim regeristry memory. */
	WidgetRegReclaim();


	return;
}





