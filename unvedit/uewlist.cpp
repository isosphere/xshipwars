// unvedit/uewlist.cpp
/*
                        Universe Editor: Lists

	Functions:

	void UEWUpdateObjectList(uew_struct *uew_ptr)

	void UEWObjectListScrollToItem(
	        uew_struct *uew_ptr,
	        int obj_num
	)

	void UEWUpdateObjectListItemName(
	        uew_struct *uew_ptr,
	        int obj_num
	)


	---

 */
/*
#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
#include <string.h>
*/
#include "../include/string.h"
#include "../include/osw-x.h"
#include "../include/widget.h"

#include "../include/reality.h"
#include "../include/objects.h"
#include "../include/unvmatch.h"
   
#include "ue.h"
#include "uew.h"


/*
 *	Updates the objects list to match values of objects pointers.
 *	All items in colum list will be unselected.
 */
void UEWUpdateObjectList(uew_struct *uew_ptr)
{
	int i, n;
	char string[128];
	xsw_object_struct **obj_ptr;
	colum_list_struct *list_ptr;
	colum_list_row_struct *row_ptr;


	if(uew_ptr == NULL)
	    return;

	list_ptr = &uew_ptr->objects_list;


	/*   Delete all existing entries in colum list and
         *   unselect all.
	 */
	CListUnselectAllRows(list_ptr);
	CListDeleteAllRows(list_ptr);


	/* Add objects list to colum list. */
	for(i = 0,	/* Object number. */
            n = 0,	/* List row position. */
            obj_ptr = uew_ptr->object;
            i < uew_ptr->total_objects;
            i++, obj_ptr++
	)
	{
	    if(*obj_ptr == NULL)
		continue;

	    if((*obj_ptr)->type <= XSW_OBJ_TYPE_GARBAGE)
                continue;


	    if(CListAddRow(list_ptr, -1) < 0)
		continue;

	    /* Get pointer to newly allocater row. */
	    row_ptr = list_ptr->row[n];

	    sprintf(string, "#%i", i);
	    if(
	        CListAddItem(
		    list_ptr,
		    string,
                    OSWQueryCurrentFont(),
		    widget_global.editable_text_pix,
		    0,
		    n
	        ) < 0
	    )
	    {
		n++;
		continue;
	    }

	    /*   Set client data pointer on first item on row to
	     *   index value of it's associated XSW object.
	     */
	    row_ptr->item[0]->client_data = (void *)i;

            CListAddItem(
                list_ptr,
                (*obj_ptr)->name,
                OSWQueryCurrentFont(),
                widget_global.editable_text_pix,
                0,
                n
            );
 

	    n++;	/* Increment row number. */
	}	


	if(uew_ptr->map_state)
	     CListDraw(&uew_ptr->objects_list, CL_DRAW_AMOUNT_COMPLETE);


	return;
}


/*
 *	Scrolls object list to obj_num.
 */
void UEWObjectListScrollToItem(
        uew_struct *uew_ptr,
        int obj_num
)
{
	int i, m, n;
	colum_list_struct *list;
	colum_list_item_struct *item;
	win_attr_t wattr;


	if(uew_ptr == NULL)
	    return;

	list = &uew_ptr->objects_list;


	for(i = 0, n = -1; i < list->total_rows; i++)
	{
	    if(list->row[i] == NULL)
		continue;

	    if(list->row[i]->total_items < 1)
		continue;

	    item = list->row[i]->item[0];
	    if(item == NULL)
		continue;

	    if((int)item->client_data == obj_num)
	    {
		n = i;
		break;
	    }
	}

	/* Could not find row? */
	if(n == -1)
	    return;

	OSWGetWindowAttributes(list->list, &wattr);
	list->sb.y_win_pos = (n * (int)list->row_height) -
            (int)wattr.height;

	m = (list->total_rows * (int)list->row_height) -
	    (int)wattr.height;
	if(list->sb.y_win_pos > m)
	    list->sb.y_win_pos = m;
	if(list->sb.y_win_pos < 0)
	    list->sb.y_win_pos = 0;


	return;
}


/*
 *	Updates the name of the object on the objects list.
 */
void UEWUpdateObjectListItemName(
        uew_struct *uew_ptr,
        int obj_num
)
{
	int i;
	colum_list_struct *list;
	colum_list_item_struct *item;


	if(uew_ptr == NULL)
	    return;

	list = &uew_ptr->objects_list;

	if(UEWIsObjectGarbage(uew_ptr, obj_num))
	    return;


	for(i = 0; i < list->total_rows; i++)
	{
	    if(list->row[i] == NULL)
		continue;
	    if(list->row[i]->total_items < 2)
		continue;

	    item = list->row[i]->item[0];
	    if(item == NULL)
		continue;


	    if((int)item->client_data == obj_num)
	    {
                item = list->row[i]->item[1];
                if(item == NULL)
                    continue;

		free(item->label);
		item->label =
		    StringCopyAlloc(uew_ptr->object[obj_num]->name
		);
	    }
	}



	return;
}




