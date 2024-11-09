/*
                        Viewscreen: Object Labels

	Functions:

	int VSLabelIsAllocated(int n)
	int VSLabelIsLoaded(int n)
	int VSLabelGetByPointer(xsw_object_struct *obj_ptr)

	int VSLabelGetHighest()

        int VSLabelAdd(
	        char *text,  
	        WColorStruct fg_color,
	        WColorStruct bg_color,
	        font_t *font,
	        xsw_object_struct *obj_ptr
	)
	void VSLabelDelete(int n)
	void VSLabelDeleteByObjectPtr(xsw_object_struct *obj_ptr)
	void VSLabelDeleteAll()
	void VSLabelReclaimMemory()

	---

	These labels are used to show the object's name on the
	viewscreen.

	The label number should be the object's number.


 */

#include "xsw.h"


/*
 *	Character sizes in pixels.
 */
#define VSLABEL_CHAR_WIDTH	6
#define VSLABEL_CHAR_HEIGHT	10


/*
 *	Checks if label n is allocated.
 */
int VSLabelIsAllocated(int n)
{
	if((vs_object_label == NULL) ||
           (n < 0) ||
           (n >= total_vs_object_labels)
	)
	    return(0);
	else if(vs_object_label[n] == NULL)
	    return(0);
	else
	    return(1);
}

/*
 *	Checks if label n is allocated and loaded.
 */
int VSLabelIsLoaded(int n)
{
	if(!VSLabelIsAllocated(n))
	    return(0);
	else if(vs_object_label[n]->image == NULL)
	    return(0);
	else
	    return(1);
}

/*
 *	Returns the label number matching the object pointer.
 *	If no label can be found, -1 is returned.
 *	If a index number of 0 or greater is returned, that label
 *	can be assumed allocated and loaded.
 */
int VSLabelGetByPointer(xsw_object_struct *obj_ptr)
{
	int i;
	vs_object_label_struct **ptr, *label_ptr;


	if(obj_ptr == NULL)
	    return(-1);


	for(i = 0, ptr = vs_object_label;
            i < total_vs_object_labels;
            i++, ptr++
	)
	{
	    label_ptr = *ptr;
	    if(label_ptr == NULL)
		continue;

	    if(label_ptr->obj_ptr == obj_ptr)
	    {
		if(VSLabelIsLoaded(i))
		    return(i);
		else
		    continue;
	    }
	}

	return(-1);
}

/*
 *	Returns the highest allocated label number that has an image
 *	loaded or -1 for none.
 */
int VSLabelGetHighest()
{
	int i, n;
        vs_object_label_struct **ptr, *label_ptr;


	for(i = 0, n = -1, ptr = vs_object_label;
	    i < total_vs_object_labels;
	    i++, ptr++
	)
	{
	    label_ptr = *ptr;
	    if(label_ptr == NULL)
		continue;

	    if(label_ptr->image == NULL)
		continue;

	    n = i;
	}

	return(n);
}

/*
 *	Adds (allocates as needed) a new or replace existing label.
 */
int VSLabelAdd(
	char *text,
	WColorStruct fg_color,
	WColorStruct bg_color,
	font_t *font,
	xsw_object_struct *obj_ptr
)
{
	int i, k, n;
	int len, len2;
        vs_object_label_struct **ptr, *label_ptr;

	pixel_t fg_pix, bg_pix;
	unsigned int width, height;
	pixmap_t pixmap;
	image_t *image;
	font_t *prev_font;
	char *new_text;


	/* Object pointer must be valid. */
	if(obj_ptr == NULL)
	    return(-1);

	/* Sanitize total. */
	if(total_vs_object_labels < 0)
	    total_vs_object_labels = 0;


	/* Check for available pointer. */
	for(i = 0, k = -1, ptr = vs_object_label;
            i < total_vs_object_labels;
            i++, ptr++
	)
	{
	    label_ptr = *ptr;
	    if(label_ptr == NULL)
	    {
		k = i;
		continue;
	    }

	    /* Check if object already has a label. */
	    if(label_ptr->obj_ptr == obj_ptr)
	    {
		/* Delete this label. */
		VSLabelDelete(i);

		k = i;
		continue;
	    }
	}
	if(k > -1)
	{
	    /* Found available NULL pointer k. */
	    n = k;
	}
	else
	{
	    /* Need to allocate more pointers. */

	    n = total_vs_object_labels;

	    total_vs_object_labels++;	    
	    vs_object_label = (vs_object_label_struct **)realloc(
		vs_object_label,
		total_vs_object_labels * sizeof(vs_object_label_struct *)
	    );
	    if(vs_object_label == NULL)
	    {
		total_vs_object_labels = 0;
		return(-1);
	    }

	    vs_object_label[n] = NULL;
	}

	/* Allocate structure. */
	vs_object_label[n] = (vs_object_label_struct *)calloc(
	    1,
	    sizeof(vs_object_label_struct)
	);
	if(vs_object_label[n] == NULL)
	{
	    return(-1);
	}

	label_ptr = vs_object_label[n];


	/* ************************************************************ */
	/* Set object number and pointer. */

        label_ptr->obj_ptr = obj_ptr;


        /* ************************************************************ */
	/* Create label. */

	/* Make sure name text is valid. */
	if(text == NULL)
	    text = "(null)";

	/* Get lengths of texts. */
	len = strlen(text);
	len2 = strlen(obj_ptr->empire);

	/* Allocate tempory memory for new text. */
	new_text = (char *)malloc((len + len2 + 10) * sizeof(char));
	if(new_text == NULL)
	{
	    VSLabelDelete(n);
	    return(-1);
	}

	/* Format new text. */
	sprintf(new_text, "(%s) %s", obj_ptr->empire, text);


	/* Calculate width and height. */
	len = strlen(new_text);

	width = (len * VSLABEL_CHAR_WIDTH) + 16;
	height = VSLABEL_CHAR_WIDTH + 2;

	/* Width and height must be even numbers! */
	if(IS_NUM_ODD(width))
	    width += 1;
        if(IS_NUM_ODD(height))
	    height += 1;

	if(OSWCreatePixmap(&pixmap, width, height))
	{
            VSLabelDelete(n);
	    free(new_text);
	    return(-1);
	}

        /* Record previous font and set new font. */
        prev_font = OSWQueryCurrentFont();
        OSWSetFont(font);

	/* Allocate pixel. */
	OSWLoadPixelRGB(
	    &fg_pix,
	    fg_color.r,
	    fg_color.g,
	    fg_color.b
	);
        OSWLoadPixelRGB(
            &bg_pix,
            bg_color.r,
            bg_color.g,
            bg_color.b
        );


	/* Draw text to pixmap. */
	OSWClearPixmap(pixmap, width, height, bg_pix);

	OSWSetFgPix(fg_pix);
	OSWDrawString(
	    pixmap,
	    4, ((int)height / 2) + 3,
	    new_text
	);


        /* Set back previous font. */
        OSWSetFont(prev_font);

	/* Unload pixels. */
	OSWDestroyPixel(&fg_pix);
	OSWDestroyPixel(&bg_pix);


	/* Copy graphics from pixmap to image. */
        image = OSWGetImage(pixmap, 0, 0, width, height);
        if(image == NULL)
        {
            OSWDestroyPixmap(&pixmap);
	    free(new_text);
            return(-1);
        }

	/* Set image. */
	label_ptr->image = image;

	/* Destroy the pixmap buffer, not needed anymore. */
	OSWDestroyPixmap(&pixmap);

	/* Deallocate tempory text. */
	free(new_text);


	return(0);
}

/*
 *      Frees all allocated resources of label n and the label
 *      itself.
 */
void VSLabelDelete(int n)
{
        vs_object_label_struct *label_ptr;


	/* Check if label is allocated. */  
        if(VSLabelIsAllocated(n)) 
            label_ptr = vs_object_label[n];
        else
            return;


	/* Do not delete object. */
	label_ptr->obj_ptr = NULL;


        /* Destroy the label image. */
        OSWDestroyImage(&label_ptr->image);


        /* Free the allocated structure. */
        free(vs_object_label[n]);
        vs_object_label[n] = NULL;


        return;
}

/*
 *	Deletes a label matching that of the given object pointer.
 */
void VSLabelDeleteByObjectPtr(xsw_object_struct *obj_ptr)
{
	int i;
        vs_object_label_struct **ptr, *label_ptr;


	if(obj_ptr == NULL)
	    return;


        for(i = 0, ptr = vs_object_label;
            i < total_vs_object_labels;
            i++, ptr++
        )
        {
	    label_ptr = *ptr;
            if(label_ptr == NULL)
                continue;

	    if(label_ptr->obj_ptr == obj_ptr)
		VSLabelDelete(i);
	}

	return;
}


/*           
 *      Free all labels.
 */
void VSLabelDeleteAll() 
{
	int i;


        for(i = 0; i < total_vs_object_labels; i++)
            VSLabelDelete(i);

        free(vs_object_label);
        vs_object_label = NULL;

        total_vs_object_labels = 0;

        return;
}

/*
 *	Deletes unallocated VS pointers.
 */
void VSLabelReclaimMemory()
{
	int i, highest;


	/* Get highest allocated label structure. */
	highest = VSLabelGetHighest();
	/* If no allocated pointers, delete all pointers. */
	if(highest < 0)
	{
	    VSLabelDeleteAll();
	    return;
	}

	/* Delete excess pointers. */
	for(i = highest + 1; i < total_vs_object_labels; i++)
	    VSLabelDelete(i);

	/* Reallocate pointers. */
	total_vs_object_labels = highest + 1;

	if(total_vs_object_labels > 0)
	{
	    vs_object_label = (vs_object_label_struct **)realloc(
	        vs_object_label,
	        total_vs_object_labels * sizeof(vs_object_label_struct *)
	    );
	    if(vs_object_label == NULL)
	    {
	        total_vs_object_labels = 0;
		return;
	    }
	}
	else
	{
	    free(vs_object_label);
	    vs_object_label = NULL;

	    total_vs_object_labels = 0;
	}

	return;
}
