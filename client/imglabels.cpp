/*
                        XSW Image Label Management

	Functions:

	int ImgLabelIsAllocated(xsw_imglabel_struct *label)
	int ImgLabelIsLoaded(xsw_imglabel_struct *label)

	xsw_imglabel_struct *ImgLabelAllocate()
	void ImgLabelReset(xsw_imglabel_struct *label)
	void ImgLabelDestroy(xsw_imglabel_struct *label)

	---

	Image labels are used as general purpose labels through
	out the program (ie in the main menu).


 */


#include "xsw.h"


int ImgLabelIsAllocated(xsw_imglabel_struct *label)
{
	if(label == NULL)
	{
	    return(0);
	}
	else
	{
	    return(1);
	}
}


int ImgLabelIsLoaded(xsw_imglabel_struct *label)
{
	if(!ImgLabelIsAllocated(label))
	{
            return(0);
        }  
        else if(label->image == NULL)
        {
            return(0);
        }
        else
        {
            return(1);
        }
}


/*
 *	Allocates a new image label structure, returns its pointer or
 *	NULL on error.
 */
xsw_imglabel_struct *ImgLabelAllocate()
{
	return(
	    (xsw_imglabel_struct *)calloc(1,
		sizeof(xsw_imglabel_struct)
	    )
	);
}


/*
 *      Deallocates label's substructures but not the label itself.
 */
void ImgLabelReset(xsw_imglabel_struct *label)
{
        if(!ImgLabelIsAllocated(label)) return;

	free(label->filename);

	label->pos_by_percent = 0;
	label->size_by_percent = 0;

	label->x = 0;
	label->y = 0;

        OSWDestroyImage(&label->image);


        return;
}


/*
 *	Deallocates label and all its substructures.
 */
void ImgLabelDestroy(xsw_imglabel_struct *label)
{
	/* Deallocate sub structures. */
	ImgLabelReset(label);

	/* Free label structure itself. */
	free(label);
	label = NULL;


	return;
}
