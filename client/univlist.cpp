/*
                Universe List Entry Management

	Functions:

	int UnivIsAllocated(int n)

	int UnivAdd(
		char *alias,
		char *url,
	        time_t last_connected,
	        char *comments,
	        int pos
	)        
	void UnivDelete(int n)
	void UnivDeleteAll()

	---

 */

#include "univlist.h"
#include "xsw.h"


#define MIN(a,b)        (((a) < (b)) ? (a) : (b))
#define MAX(a,b)        (((a) > (b)) ? (a) : (b))


univ_entry_struct **univ_entry;
int total_univ_entries;


/*
 *	Checks if universe entry n is allocated.
 */
int UnivIsAllocated(int n)
{
        if((univ_entry == NULL) ||
           (n < 0) ||   
           (n >= total_univ_entries)
        )
            return(0);
        else
	    return((univ_entry[n] == NULL) ? 0 : 1);
}

/*
 *	Create a new universe entry and set given values to it.
 *
 *	Returns the new entry number position on successs or -1 on
 *	error.
 */
int UnivAdd(
	char *alias,
	char *url,
	time_t last_connected,
	char *comments,
	int pos
)
{
        // Dan S: renamed from "new" to "new_entry" for C++ key word compatibility.
        int i, new_entry;
        univ_entry_struct *univ_ptr;


        /* Sanitize total. */
        if(total_univ_entries < 0)
            total_univ_entries = 0;

        /* Increase total. */
        total_univ_entries++;


        /* Check which entry style. */
        if(pos < 0)
        {
            /* Append entry. */
            
            /* Calculate new entry index. */
            new_entry = total_univ_entries - 1;

            /* Allocate new entry. */
            univ_entry = (univ_entry_struct **)realloc(
		univ_entry,
                total_univ_entries * sizeof(univ_entry_struct *)
            );
            if(univ_entry == NULL)
            {
                total_univ_entries = 0;
                return(-1);
            }  
              
            univ_entry[new_entry] = (univ_entry_struct *)calloc(
		1,
                sizeof(univ_entry_struct)
            );
            if(univ_entry[new_entry] == NULL)
            {
                total_univ_entries--;
                return(-1);
            }
        }
        else
        {
            /* Insert. */

            /* Sanitize pos. */
            if(pos >= total_univ_entries)
                pos = total_univ_entries - 1;

            /* Allocate new entry. */
            univ_entry = (univ_entry_struct **)realloc(
		univ_entry,
                total_univ_entries * sizeof(univ_entry_struct *)
            );
            if(univ_entry == NULL)
            {
                total_univ_entries = 0;
                return(-1);
            }
                
            univ_ptr = (univ_entry_struct *)calloc(
		1,
                sizeof(univ_entry_struct)
            );
            if(univ_ptr == NULL)
            {
                total_univ_entries--;
                return(-1);
            }

            /* Shift entries. */
            for(i = total_univ_entries - 1; i > pos; i--)
                univ_entry[i] = univ_entry[i - 1];

            /* Adjust new entry number. */
            new_entry = pos;

            /* Set new index to point to newly allocated pointer. */
            univ_entry[new_entry] = univ_ptr;
        }


        /* New allocation successful? If so, get pointer to it. */
        if(UnivIsAllocated(new_entry))
	    univ_ptr = univ_entry[new_entry];
	else
            return(-1);


        /* Set values. */
	univ_ptr->alias = StringCopyAlloc(alias);
        univ_ptr->url = StringCopyAlloc(url);
        univ_ptr->last_connected = last_connected;
	univ_ptr->comments = StringCopyAlloc(comments);


        return(new_entry);
}

/*
 *	Deletes universe entry n.
 */
void UnivDelete(int n)
{
        univ_entry_struct *entry_ptr;


        if(UnivIsAllocated(n))
            entry_ptr = univ_entry[n];
        else
            return;


        /* Free alias. */
        free(entry_ptr->alias);
        entry_ptr->alias = NULL;

        /* Free URL. */
        free(entry_ptr->url);
        entry_ptr->url = NULL;

        /* Reset last connected. */
        entry_ptr->last_connected = 0;

        /* Free comments. */
        free(entry_ptr->comments);
        entry_ptr->comments = NULL;


        free(entry_ptr);
        entry_ptr = NULL;


        return;
}

/*
 *	Procedure to delete all universe entries.
 */
void UnivDeleteAll()
{
        int i;


        for(i = 0; i < total_univ_entries; i++)
            UnivDelete(i);

        free(univ_entry);
        univ_entry = NULL;

        total_univ_entries = 0;


        return;
}        

