// unvedit/uewdde.cpp



/*
              Universe Editor DDN (Copy and Paste)

	Functions:

	int UEWDDEIsBufferXSWObject(u_int8_t *buf, int len)

	int UEWDDEPutXSWObject(xsw_object_struct *obj_ptr)
	xsw_object_struct *UEWDDEFetchXSWObject()

	---

 */
/*
#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
#include <string.h>
*/
#include "../include/osw-x.h"
#include "../include/widget.h"

#include "../include/reality.h"
#include "../include/objects.h"
#include "../include/unvmatch.h"

#include "ue.h"
#include "uew.h"

#ifndef MAX
#define MIN(a,b)        (((a) < (b)) ? (a) : (b))
#define MAX(a,b)	(((a) > (b)) ? (a) : (b))
#endif

/* #define MIN(a,b)        ((a) < (b) ? (a) : (b)) */
/* #define MAX(a,b)        ((a) > (b) ? (a) : (b)) */



#define UEWDDE_IDENTIFIER_STR	"XSW OBJECT"

#define UEWDDE_HEADER_LEN	32


/*
 *	Checks if the buffer contains an XSW object.
 */
int UEWDDEIsBufferXSWObject(u_int8_t *buf, int len)
{
	/* Header too short? */
	if(len < UEWDDE_HEADER_LEN)
	    return(0);

	if(buf == NULL)
	    return(0);


	if(strncmp((char *)buf, UEWDDE_IDENTIFIER_STR, strlen(UEWDDE_IDENTIFIER_STR)))
	    return(0);

	return(1);
}


/*
 *	Puts the object into the DDE, converting and formatting
 *	its data as needed
 */
int UEWDDEPutXSWObject(xsw_object_struct *obj_ptr)
{
	int i, len, pos;
	int8_t *buf;


	if(obj_ptr == NULL)
	    return(-1);


	/* Calculate memory needed. */

	/* Header. */
	len = UEWDDE_HEADER_LEN;

	/* Core object. */
	len += sizeof(xsw_object_struct);

	/* Weapons. */
	len += (obj_ptr->total_weapons * sizeof(xsw_weapons_struct));

	/* Score. */
	if(obj_ptr->score != NULL)
	    len += sizeof(xsw_score_struct);

	/* Economy. */
	if(obj_ptr->eco != NULL)
	{
	    len += sizeof(xsw_ecodata_struct);
	    len += (obj_ptr->eco->total_products *
                    sizeof(xsw_ecoproduct_struct));
	}


	/* Allocate memory. */
	buf = (int8_t *)malloc(
	    len * sizeof(int8_t)
	);
	if(buf == NULL)
	    return(-1);


	/* ********************************************************* */
	/* Begin copying object data over to buf. */

	pos = 0;

	/* Copy header. */
	i = MIN(strlen(UEWDDE_IDENTIFIER_STR), UEWDDE_HEADER_LEN);
	memcpy(
	    &buf[pos], 
	    UEWDDE_IDENTIFIER_STR,
	    i
	);
	/* Null terminate the identifier string. */
	buf[pos + i] = '\0';

	pos += UEWDDE_HEADER_LEN;


	/* Copy core object values. */
        memcpy(
            &buf[pos],
            obj_ptr,
            sizeof(xsw_object_struct)
        );
	pos += sizeof(xsw_object_struct);


	/* Copy weapons. */
	for(i = 0; i < obj_ptr->total_weapons; i++)
	{
	    if(obj_ptr->weapons[i] != NULL)
	    {
                memcpy(
                    &buf[pos],
                    obj_ptr->weapons[i],
                    sizeof(xsw_weapons_struct)
                );
	    }
	    else
	    {
		memset(
		    &buf[pos],
		    0x00,
		    sizeof(xsw_weapons_struct)
		);
	    }

	    pos += sizeof(xsw_weapons_struct);
	}


        /* Copy scores. */
        if(obj_ptr->score != NULL)
	{
            memcpy(
                &buf[pos],
                obj_ptr->score,
                sizeof(xsw_score_struct)
            );

            pos += sizeof(xsw_score_struct);
	}


	/* Economy. */
	if(obj_ptr->eco != NULL)
	{
            memcpy(
                &buf[pos],
                obj_ptr->eco,
                sizeof(xsw_ecodata_struct)
            );
            pos += sizeof(xsw_ecodata_struct);

	    /* Copy over each eco product. */
	    for(i = 0; i < obj_ptr->eco->total_products; i++)
	    {
		if(obj_ptr->eco->product[i] != NULL)
		{
                    memcpy(
                        &buf[pos],
                        obj_ptr->eco->product[i],
                        sizeof(xsw_ecoproduct_struct)
                    );
                }
                else
                {
                    memset(
                        &buf[pos],
			0x00,
                        sizeof(xsw_ecoproduct_struct)
                    );
                }
                pos += sizeof(xsw_ecoproduct_struct);
	    }
        }


	/* ***************************************************** */

	/* Put to DDE. */
	OSWPutDDE(buf, len);


	/* Free buffer. */
	free(buf);


	return(0);
}


/*
 *	Returns a dynamically allocated XSW Object fetched from
 *	the DDE or NULL if there wasn't any.  The returned object
 *	needs to be freed by a call to UnvDeleteObject().
 */
xsw_object_struct *UEWDDEFetchXSWObject()
{
	int i, pos, len;
	u_int8_t *buf;
	xsw_object_struct *obj_ptr;


	/* Fetch buffer from DDE. */
	buf = (u_int8_t *)OSWFetchDDE(&len);
	if((buf == NULL) || (len < 0))
	    return(NULL);

	/* Check if it has the right identifier. */
	if(!UEWDDEIsBufferXSWObject(buf, len))
	{
            OSWGUIFree((void **)&buf);
	    return(NULL);
	}


	/* Create new object. */
	obj_ptr = (xsw_object_struct *)calloc(
	    1,
	    sizeof(xsw_object_struct)
	);
	if(obj_ptr == NULL)
	{
            OSWGUIFree((void **)&buf);
	    return(NULL);
	}

	/* ************************************************** */

        pos = UEWDDE_HEADER_LEN;

	/* Copy object core values. */
	if((len - pos) >= (int)sizeof(xsw_object_struct))
	    memcpy(
	        obj_ptr,
	        &buf[pos],
	        sizeof(xsw_object_struct)
	    );
        pos += sizeof(xsw_object_struct);


        /* Copy object weapons. */
	if(obj_ptr->total_weapons > 0)
	{
	    obj_ptr->weapons = (xsw_weapons_struct **)calloc(
		1,
		obj_ptr->total_weapons * sizeof(xsw_weapons_struct *)
	    );
	    if(obj_ptr->weapons != NULL)
	    {
	        for(i = 0; i < obj_ptr->total_weapons; i++)
	        {
		    if((len - pos) >= (int)sizeof(xsw_weapons_struct))
		    {
			obj_ptr->weapons[i] = (xsw_weapons_struct *)calloc(
			    1,
			    sizeof(xsw_weapons_struct)
			);
			if(obj_ptr->weapons[i] != NULL)
			{
                            memcpy(
                                obj_ptr->weapons[i],
                                &buf[pos],
                                sizeof(xsw_weapons_struct)
                            );
			}
		    }
		    pos += sizeof(xsw_weapons_struct);
		}
	    }
	    else
	    {
		obj_ptr->selected_weapon = -1;
		obj_ptr->total_weapons = 0;
	    }
	}


        /* Copy object scores. */
        if(obj_ptr->score != NULL)
	{
	    obj_ptr->score = (xsw_score_struct *)calloc(
		1,
		sizeof(xsw_score_struct)
	    );
	    if((obj_ptr->score != NULL) &&
               ((len - pos) >= (int)sizeof(xsw_score_struct))
	    )
	    {
                memcpy(
		    obj_ptr->score,
                    &buf[pos],
                    sizeof(xsw_score_struct)
                );
	    }                 
            pos += sizeof(xsw_score_struct);
        }


        /* Copy object economy. */
        if(obj_ptr->eco != NULL)
        {                   
            obj_ptr->eco = (xsw_ecodata_struct *)calloc(
                1,              
                sizeof(xsw_ecodata_struct)
            );
            if(obj_ptr->eco != NULL)
            {
		if((len - pos) >= (int)sizeof(xsw_ecodata_struct)) 
                {
                    memcpy( 
                        obj_ptr->eco,
                        &buf[pos],
                        sizeof(xsw_ecodata_struct)
                    );
		}
                pos += sizeof(xsw_ecodata_struct);

		/* Copy over eco products. */
		if(obj_ptr->eco->total_products > 0)
		{
		    obj_ptr->eco->product = (xsw_ecoproduct_struct **)calloc(
			1,
			obj_ptr->eco->total_products * sizeof(xsw_ecoproduct_struct *)
		    );
		    if(obj_ptr->eco->product != NULL)
		    {
			for(i = 0; i < obj_ptr->eco->total_products; i++)
			{
                            if((len - pos) >= (int)sizeof(xsw_ecoproduct_struct))
                            {
                                obj_ptr->eco->product[i] = (xsw_ecoproduct_struct *)calloc(
                                    1,
                                    sizeof(xsw_ecoproduct_struct)
                                );
                                if(obj_ptr->eco->product[i] != NULL)
                                {
                                    memcpy(
                                        obj_ptr->eco->product[i],
                                        &buf[pos],
                                        sizeof(xsw_ecoproduct_struct)
                                    );
                                }
                            }
                            pos += sizeof(xsw_ecoproduct_struct);
			}
		    }
		    else
		    {
			obj_ptr->eco->total_products = 0;
		    }
		}
		else
		{
		    obj_ptr->eco->product = NULL;
		}
            }
	    else
	    {
                pos += sizeof(xsw_ecodata_struct);
	    }
        }


        /* Free buffer. */
        OSWGUIFree((void **)&buf);


	return(obj_ptr);
}




