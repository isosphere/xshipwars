/*
                       Joystick Mapping Management

	Functions:

	int JSMapIsAllocated(int n)
	int JSMapAllocate()
	void JSMapSyncWithData(jsmap_struct *jsm)
	void JSMapDelete(int n)
	void JSMapDeleteAll()

	---

	Handles joystick map structures.  Joystick map structures
	determine which joystick axis or button performs which action
	for the program.

	JSMaps are private to this program and not in any way comming
	from libjsw.

 */

#ifdef JS_SUPPORT

#include "xsw.h"


/*
 *	Checks if jsmap structure n is allocated.
 */
int JSMapIsAllocated(int n)
{
	if((jsmap == NULL) ||
           (n < 0) ||
           (n >= total_jsmaps)
	)
	    return(0);
	else if(jsmap[n] == NULL)
	    return(0);
	else
	    return(1);
}


/*
 *	Allocate a new jsmap structure. Returns the new jsmap stucture
 *	index number on success or -1 on error.
 */
int JSMapAllocate()
{
	int i, n;
	jsmap_struct *jsmap_ptr;


	if(total_jsmaps < 0)
	    total_jsmaps = 0;


	for(i = 0; i < total_jsmaps; i++)
	{
	    if(jsmap[i] == NULL)
		break;
	}
	if(i < total_jsmaps)
	{
	    n = i;
	}
	else
	{
	    n = total_jsmaps;

	    total_jsmaps++;
	    jsmap = (jsmap_struct **)realloc(
		jsmap,
		total_jsmaps * sizeof(jsmap_struct *)
	    );
	    if(jsmap == NULL)
	    {
		total_jsmaps = 0;
		return(-1);
	    }

	    jsmap[n] = NULL;
	}

	/* Allocate structure as needed. */
	if(jsmap[n] == NULL)
	{
	    jsmap[n] = (jsmap_struct *)calloc(
		1,
		sizeof(jsmap_struct)
	    );
	    if(jsmap[n] == NULL)
	        return(-1);
	}


	jsmap_ptr = jsmap[n];


	/* Reset values. */
	jsmap_ptr->jsd.fd = -1;



	return(n);
}

/*
 *	Deletes or allocates more substructures on joystick map
 *	jsm based on how many axises and buttons exist on the
 *	actual joystick data.
 */
void JSMapSyncWithData(jsmap_struct *jsm)
{
	int i;
	js_data_struct *jsd;


	/* Joystick map structure must be valid. */
	if(jsm == NULL)
	    return;

	/* Get pointer to joystick data structure. */
	jsd = &jsm->jsd;

	/* Joystick not initialized? */
	if(jsd->fd < 0)
	{
	    fprintf(
		stderr,
		"JSMapSyncWithData(): Warning: Joystick data not initialized.\n"
	    );
	}


	/* *********************************************************** */
	/* Sanitize axis totals. */
	if(jsm->total_axises < 0)
	    jsm->total_axises = 0;
	if(jsd->total_axises < 0)
	    jsd->total_axises = 0;

	/* Joystick map has too many axises? */
	if(jsm->total_axises > jsd->total_axises)
	{
	    /* Free excess axis maps on joystick map. */
	    for(i = jsm->total_axises - 1; i >= jsd->total_axises; i--)
		free(jsm->axis[i]);

	    /* Set new total and reallocate pointers on joystick map. */
	    jsm->total_axises = jsd->total_axises;
	    if(jsm->total_axises > 0)
	    {
		jsm->axis = (jsmap_axis_struct **)realloc(
		    jsm->axis,
		    jsm->total_axises * sizeof(jsmap_axis_struct *)
		);
		if(jsm->axis == NULL)
		{
		    jsm->total_axises = 0;
		}
	    }
            else
            {
                free(jsm->axis);
                jsm->axis = NULL;  

                jsm->total_axises = 0;
            }
	}
	/* Joystick map has too few axises? */
	else if(jsm->total_axises < jsd->total_axises)
	{
	    /* Allocate more axises on joystick map. */
	    jsm->axis = (jsmap_axis_struct **)realloc(
                jsm->axis,
		jsd->total_axises * sizeof(jsmap_axis_struct *)
	    );
	    if(jsm->axis == NULL)
	    {
		jsm->total_axises = 0;
		return;
	    }

	    /* Allocate eacn new axis map on joystick map. */
	    for(i = jsm->total_axises; i < jsd->total_axises; i++)
	    {
		jsm->axis[i] = (jsmap_axis_struct *)calloc(
		    1,
		    sizeof(jsmap_axis_struct)
		);
		if(jsm->axis[i] == NULL)
		    continue;

		/* Reset values. */
		jsm->axis[i]->op_code = JSMAP_AXIS_OP_NONE;

	    }

	    /* Set new total axises on joystick map. */
	    jsm->total_axises = jsd->total_axises;
	}

        /* *********************************************************** */
        /* Sanitize button totals. */
        if(jsm->total_buttons < 0)
            jsm->total_buttons = 0;
        if(jsd->total_buttons < 0)
            jsd->total_buttons = 0;

	/* Joystick map has too many buttons? */
	if(jsm->total_buttons > jsd->total_buttons)
        {
            /* Free excess button maps on joystick map. */        
            for(i = jsm->total_buttons - 1; i >= jsd->total_buttons; i--)
                free(jsm->button[i]);

            /* Set new total and reallocate pointers on joystick map. */
            jsm->total_buttons = jsd->total_buttons;
            if(jsm->total_buttons > 0)
            {
                jsm->button = (jsmap_button_struct **)realloc(
                    jsm->button,
                    jsm->total_buttons * sizeof(jsmap_button_struct *)
                );
                if(jsm->button == NULL) 
                {
                    jsm->total_buttons = 0;
                }
            }
	    else
	    {
		free(jsm->button);
		jsm->button = NULL;

		jsm->total_buttons = 0;
	    }
        }
        /* Joystick map has too few buttons? */
        else if(jsm->total_buttons < jsd->total_buttons)
        {
            /* Allocate more buttons on joystick map. */
            jsm->button = (jsmap_button_struct **)realloc(
                jsm->button,
                jsd->total_buttons * sizeof(jsmap_button_struct *)
            );  
            if(jsm->button == NULL)
            {       
                jsm->total_buttons = 0;
                return;
            }         

            /* Allocate eacn new button map on joystick map. */
            for(i = jsm->total_buttons; i < jsd->total_buttons; i++)
            {
                jsm->button[i] = (jsmap_button_struct *)calloc(
                    1,
                    sizeof(jsmap_button_struct)
                );
                if(jsm->button[i] == NULL)
                    continue;
        
                /* Reset values. */
                jsm->button[i]->keycode = 0;
                jsm->button[i]->state = 0;
            }

            /* Set new total buttons on joystick map. */
            jsm->total_buttons = jsd->total_buttons;
        }


	return;
}


void JSMapDelete(int n)
{
	int i;


	if(JSMapIsAllocated(n))
	{
	    /* Free device name. */
	    free(jsmap[n]->device_name);

	    /* Close associated joystick as needed. */
	    JSClose(&jsmap[n]->jsd);

	    /* Free all axis mappings. */
	    for(i = 0; i < jsmap[n]->total_axises; i++)
	        free(jsmap[n]->axis[i]);
	    free(jsmap[n]->axis);

	    /* Free all button mappings. */
            for(i = 0; i < jsmap[n]->total_buttons; i++)
                free(jsmap[n]->button[i]);
            free(jsmap[n]->button);

	    free(jsmap[n]);
	    jsmap[n] = NULL;
	}


	return;
}


void JSMapDeleteAll()
{
	int i;


	for(i = 0; i < total_jsmaps; i++)
	    JSMapDelete(i);

	free(jsmap);
	jsmap = NULL;

	total_jsmaps = 0;


	return;
}


#endif	/* JS_SUPPORT */
