#include "../include/string.h"
#include "../include/widget.h"


/*
 *	Size constants (in pixels):
 */
#define CL_DEF_WIDTH		200
#define CL_DEF_HEIGHT		100

#define CL_DEF_ROW_HEIGHT	18

#define CL_HEADING_HEIGHT	20

#define CL_CHAR_WIDTH		7
#define CL_CHAR_HEIGHT		14

#define CL_SPLIT_BAR_WIDTH	2



static int CListGetColumPosition(colum_list_struct *list, int col_num);

static void CListDoDeleteRow(
        colum_list_struct *list, int row_num, int redraw
);
static void CListDoDeleteItem(
        colum_list_struct *list,
        int row_num, int colum_num, int redraw
);


#define MIN(a,b)	(((a) < (b)) ? (a) : (b))
#define MAX(a,b)	(((a) > (b)) ? (a) : (b))


/*
 *	Returns the x coordinate (heading determined) position
 *	of colum col_num on the list.
 */
static int CListGetColumPosition(colum_list_struct *list, int col_num) 
{
	if(list == NULL)
	    return(0);


	if((col_num < 0) ||
           (col_num >= list->total_colums)
	)
	    return(0);

	if(list->colum[col_num] == NULL)
	    return(0);

	return(list->colum[col_num]->x_pos);
}

/*
 *	Returns the number of the first selected row on list
 *	or -1 on error or if nothing is selected.
 */
int CListGetFirstSelectedRow(colum_list_struct *list)
{
	int i;


	if(list == NULL)
	    return(-1);

	/* No rows selected? */
	if(list->total_sel_rows < 1)
	    return(-1);

	/* Get first selected row number. */
	i = list->sel_row[0];

	/* Check if it is valid. */
	if((i < 0) || (i >= list->total_rows))
	    return(-1);

	if(list->row[i] == NULL)
	    return(-1);

	return(i);
}

/*
 *      Returns the number of the last selected row on list
 *      or -1 on error or if nothing is selected.
 */
int CListGetLastSelectedRow(colum_list_struct *list)
{
        int i;


        if(list == NULL)
            return(-1);

        /* No rows selected? */
        if(list->total_sel_rows < 1)
            return(-1);

        /* Get first selected row number. */
        i = list->sel_row[list->total_sel_rows - 1];

        /* Check if it is valid. */
        if((i < 0) || (i >= list->total_rows))
            return(-1);
  
        if(list->row[i] == NULL)
            return(-1);

        return(i);
}

/*
 *	Get pointer to label of item on list
 *	referanced at item position (row_num, colum_num).
 *
 *	Can return NULL if the given values are invalid or the
 *	label is not allocated.
 */
char *CListGetItemLabel( 
	colum_list_struct *list, 
	int row_num, 
	int colum_num 
)
{
	colum_list_row_struct *row_ptr;
	colum_list_item_struct *col_ptr;


	if(list == NULL)
	    return(NULL);

	if((row_num < 0) ||
           (row_num >= list->total_rows)
	)
	    return(NULL);

	row_ptr = list->row[row_num];
	if(row_ptr == NULL)
	    return(NULL);


	if((colum_num < 0) ||
	   (colum_num >= row_ptr->total_items)
	)
	    return(NULL);

	col_ptr = row_ptr->item[colum_num];
	if(col_ptr == NULL)
            return(NULL);

	return(col_ptr->label);
}

/*
 *	Sets colum item colum_num on row row_num to the specified
 *	label (the old label is deallocated if it exists).
 *	Redraws the list if its mapped.
 */
int CListSetItemLabel(
	colum_list_struct *list,
	int row_num,
	int colum_num,
	const char *label
)
{
        colum_list_row_struct *row_ptr;
        colum_list_item_struct *col_ptr;


        if(list == NULL)
            return(-1);

	/* Row valid and allocated? */
        if((row_num < 0) || (row_num >= list->total_rows))
            return(-1);

        row_ptr = list->row[row_num];
        if(row_ptr == NULL)
            return(-1);

	/* Row item valid and allocated? */
        if((colum_num < 0) || (colum_num >= row_ptr->total_items))
            return(-1);

	col_ptr = row_ptr->item[colum_num];
        if(col_ptr == NULL)
            return(-1);

	/* Update this row item. */
	free(col_ptr->label);
	col_ptr->label = StringCopyAlloc(label);

	/* Redraw as needed. */
        if(list->map_state)
            CListDraw(list, CL_DRAW_AMOUNT_LIST);

	return(0);
}

/*
 *      Sets colum item colum_num on row row_num to the specified
 *      labels (each corresponding old label is deallocated if it exists).
 *      Redraws the list if its mapped.
 */
int CListSetRowLabels(
        colum_list_struct *list,
        int row_num,
        const char **label,
        int total_labels
)
{
	int i, m, items_updated;
        colum_list_row_struct *row_ptr;
        colum_list_item_struct *col_ptr;


        if(list == NULL)
            return(-1);

        /* Row valid and allocated? */
        if((row_num < 0) || (row_num >= list->total_rows))
            return(-1);

        row_ptr = list->row[row_num];
        if(row_ptr == NULL)
            return(-1);

	/* Itterate through items on the row, itterate no more than the
	 * given number of labels or the items on the row (whichever is
	 * less).
	 */
	m = MIN(total_labels, row_ptr->total_items);
	items_updated = 0;
	for(i = 0; i < m; i++)
	{
	    col_ptr = row_ptr->item[i];
	    if(col_ptr == NULL)
		continue;

	    /* Update this row item. */
	    free(col_ptr->label);
	    col_ptr->label = StringCopyAlloc(label[i]);

	    items_updated++;
	}

        /* Redraw as needed. */
        if(list->map_state && (items_updated > 0))
            CListDraw(list, CL_DRAW_AMOUNT_LIST);

        return(0);
}

/*
 *	Get pointer to client data of item on list
 *	referanced at item position (row_num, colum_num).
 *
 *	Can return NULL if the given values are invalid or the
 *	client data pointer is NULL.
 */
void *CListGetItemDataPtr(
        colum_list_struct *list,
        int row_num,
        int colum_num
)
{
        colum_list_row_struct *row_ptr;
        colum_list_item_struct *col_ptr;


        if(list == NULL)
            return(NULL);

        if((row_num < 0) ||
           (row_num >= list->total_rows)
	)
            return(NULL);

        row_ptr = list->row[row_num];
        if(row_ptr == NULL)
            return(NULL);


        if((colum_num < 0) ||
           (colum_num >= row_ptr->total_items)
        )
            return(NULL);

        col_ptr = row_ptr->item[colum_num];
        if(col_ptr == NULL)
            return(NULL);

        return(col_ptr->client_data);
}

/*
 *	Sets colum item's client data pointer.
 */
int CListSetItemDataPtr(  
        colum_list_struct *list,
        int row_num,
        int colum_num,
        void *client_data
)
{
        colum_list_row_struct *row_ptr;
        colum_list_item_struct *col_ptr;


        if(list == NULL)
            return(-1);


        /* Check if row is allocated. */
        if((row_num < 0) ||
           (row_num >= list->total_rows)
        )
            return(-1);

        row_ptr = list->row[row_num];
        if(row_ptr == NULL)
            return(-1);


        /* Check if colum item is allocated. */
        if((colum_num < 0) ||
           (colum_num >= row_ptr->total_items)
        )
            return(-1);

        col_ptr = row_ptr->item[colum_num];
        if(col_ptr == NULL)
            return(-1);

        col_ptr->client_data = client_data;


        /* Redraw as needed. */
        if(list->map_state)
            CListDraw(list, CL_DRAW_AMOUNT_LIST);


        return(0);
}


/*
 *	Checks if row_num is selected on list.
 */
int CListIsRowSelected(
	colum_list_struct *list,
        int row_num
)
{
	int i;


	if(list == NULL)
	    return(0);

	if(list->sel_row == NULL)
	    return(0);

	if((row_num < 0) ||
           (row_num >= list->total_rows)
	)
	    return(0);

	for(i = 0; i < list->total_sel_rows; i++)
	{
	    if(list->sel_row[i] == row_num)
		return(1);
	}

	return(0);
}


/*
 *	Selects row_num on list, redraws it if mapped.
 */
int CListSelectRow(
        colum_list_struct *list,
        int row_num
)
{
	int n;


        if(list == NULL)
            return(-1);

        /* Make sure row_num is valid. */
        if((row_num < 0) ||
           (row_num >= list->total_rows)
        )
            return(-1);

	/* Is row already selected? */
	if(CListIsRowSelected(list, row_num))
	    return(0);


	/* Select this row. */
	if(list->total_sel_rows < 0)
	    list->total_sel_rows = 0;
	n = list->total_sel_rows;
	list->total_sel_rows++;

	list->sel_row = (int *)realloc(
	    list->sel_row,
	    list->total_sel_rows * sizeof(int)
	);
	if(list->sel_row == NULL)
	{
	    list->total_sel_rows = 0;
	    return(-1);
	}

	list->sel_row[n] = row_num;


        /* Redraw as needed. */
        if(list->map_state)
            CListDraw(list, CL_DRAW_AMOUNT_LIST);


	return(0);
}


/*
 *	Unselects all rows on list, redraws it if mapped.
 */
void CListUnselectAllRows(colum_list_struct *list)
{
	if(list == NULL)
	    return;

	free(list->sel_row);
	list->sel_row = NULL;
	list->total_sel_rows = 0;


        /* Redraw as needed. */
        if(list->map_state)
            CListDraw(list, CL_DRAW_AMOUNT_LIST);


	return;
}

/*
 *	Allocates (appends) a new heading to the list, redrawing
 *	the list if mapped.
 */
int CListAddHeading(  
	colum_list_struct *list,
        const char *heading,
        font_t *font,
        pixel_t pixel,
	unsigned int attr,
        int start_pos		/* In pixels. */
)
{
	int n;


	if(list == NULL)
	    return(-1);

	if(list->total_colums < 0)
	    list->total_colums = 0;


	n = list->total_colums;
	list->total_colums++;

	list->colum = (colum_list_colum_struct **)realloc(
	    list->colum,
	    list->total_colums * sizeof(colum_list_colum_struct *)
	);
	if(list->colum == NULL)
	{
	    list->total_colums = 0;
	    return(-1);
	}

	list->colum[n] = (colum_list_colum_struct *)calloc(
	    1, sizeof(colum_list_colum_struct)
	);
	if(list->colum[n] == NULL)
	{
            list->total_colums = n;
	    return(-1);
	}

	/* Set heading values. */
	list->colum[n]->heading = StringCopyAlloc(heading);
	list->colum[n]->font = font;
	list->colum[n]->pixel = pixel;
	list->colum[n]->attr = attr;
	list->colum[n]->x_pos = start_pos;

        /* Redraw as needed. */
        if(list->map_state)
            CListDraw(list, CL_DRAW_AMOUNT_COMPLETE);

	return(n);
}


/*
 *	Delete heading on list, redrawing the list if mapped.
 */
void CListDeleteHeading(
	colum_list_struct *list,
	int colum_num
)
{
	if(list == NULL)
	    return;

        /* Is colum heading allocated? */
 	if((colum_num < 0) ||
           (colum_num >= list->total_colums)
	)
	    return;
	if(list->colum[colum_num] == NULL)
	    return;


	free(list->colum[colum_num]->heading);

	free(list->colum[colum_num]);
	list->colum[colum_num] = NULL;


        /* Redraw as needed. */
        if(list->map_state)
            CListDraw(list, CL_DRAW_AMOUNT_COMPLETE);


	return;
}


/*
 *	Adds a row to the list, does not redraw.
 */
int CListAddRow(
	colum_list_struct *list,
        int row_num			/* Can be -1 for append. */
)
{
        int n;


        if(list == NULL)
            return(-1);
        
        if(list->total_rows < 0)
            list->total_rows = 0;
          

	/* Add or append? */
	if(row_num < 0)
	{
	    /* Append. */
            n = list->total_rows;
            list->total_rows++;

            list->row = (colum_list_row_struct **)realloc(
                list->row,
                list->total_rows * sizeof(colum_list_row_struct *)
            );
            if(list->row == NULL)
            {
                list->total_rows = 0; 
                return(-1);
            }
	}
	else
	{
	    /* Add (insert). */
	    list->total_rows++;

	    if(row_num >= list->total_rows)
		row_num = list->total_rows - 1;

            list->row = (colum_list_row_struct **)realloc(
                list->row,
                list->total_rows * sizeof(colum_list_row_struct *)
            );
            if(list->row == NULL)
            {
                list->total_rows = 0;
                return(-1);
            }

	    /* Shift pointers. */
	    for(n = list->total_rows - 1; n > row_num; n--)
	        list->row[n] = list->row[n - 1];

	    n = row_num;
	}

	list->row[n] = (colum_list_row_struct *)calloc(
	    1,
	    sizeof(colum_list_row_struct)
	);
	if(list->row[n] == NULL)
	    return(-1);


	list->row[n]->item = NULL;
	list->row[n]->total_items = 0;


	return(0);
}


/*
 *	Deletes the row specified by row_num and all its items from the
 *	given list and redraws if specified.
 */
static void CListDoDeleteRow(
	colum_list_struct *list, int row_num, int redraw
)
{
	int i;
	colum_list_row_struct *row_ptr;


        if(list == NULL)
            return;

	if((row_num < 0) || (row_num >= list->total_rows))
	    return;

	row_ptr = list->row[row_num];
	if(row_ptr == NULL)
	    return;

	/* Delete all items on row structure (do not redraw after each
	 * deleted item.
	 */
	for(i = 0; i < row_ptr->total_items; i++)
            CListDoDeleteItem(list, row_num, i, 0);
	free(row_ptr->item);
	row_ptr->item = NULL;
	row_ptr->total_items = 0;


	/* Delete the row structure itself. */
        free(row_ptr);
        list->row[row_num] = row_ptr = NULL;	/* Mark row as deleted. */

        /* Redraw if specified. */
        if(redraw)
            CListDraw(list, CL_DRAW_AMOUNT_COMPLETE);
}

/*
 *      Deletes the row specified by row_num and all its items from the
 *      given list and redraws as needed.
 */
void CListDoDeleteRow(colum_list_struct *list, int row_num)
{
        if(list == NULL)
            return;

	CListDoDeleteRow(list, row_num, list->map_state);
}

/*
 *	Deletes all rows on list, redraws if mapped.
 */
void CListDeleteAllRows(colum_list_struct *list)
{
	int i;


	if(list == NULL)
	    return;

	/* Need to unselect all rows first. */
	CListUnselectAllRows(list);



	/* Delete all rows and their items (but do not redraw after each
	 * delete).
	 */
	for(i = 0; i < list->total_rows; i++)
	    CListDoDeleteRow(list, i, 0);
	free(list->row);
	list->row = NULL;
	list->total_rows = 0;

	/* Move scroll bar to origin. */
	list->sb.x_win_pos = 0;
	list->sb.y_win_pos = 0;

        /* Redraw as needed. */
        if(list->map_state)
            CListDraw(list, CL_DRAW_AMOUNT_COMPLETE);
}


/*
 *	Adds (appends) an item to the list on the row row_num.
 *	row_num must be allocated.
 */
int CListAddItems(
	colum_list_struct *list,
        const char **label,
	int total_labels,
        font_t *font,
        pixel_t pixel,
	unsigned int attr,
        int row_num
)
{
	int i, n, label_num, items_added = 0;
	colum_list_row_struct *row_ptr;
	colum_list_item_struct *col_ptr;


	if(list == NULL)
	    return(-1);

	/* Row must be valid and allocated. */
	if((row_num < 0) || (row_num >= list->total_rows))
	    row_ptr = NULL;
	else
	    row_ptr = list->row[row_num];
	if(row_ptr == NULL)
        {
            fprintf(stderr,
"CListAddItem(): Cannot add item to row %i, row not allocated.\n",
                row_num
            );
            return(-1);
        }

	/* Begin appending the new items to the row. */

	/* Sanitize total. */
	if(row_ptr->total_items < 0)
	    row_ptr->total_items = 0;

	/* Allocate more item pointers on the row. */
	n = row_ptr->total_items;
	row_ptr->total_items += total_labels;

	row_ptr->item = (colum_list_item_struct **)realloc(
	    row_ptr->item,
	    row_ptr->total_items * sizeof(colum_list_item_struct *)
	);
	if(row_ptr->item == NULL)
	{
	    row_ptr->total_items = 0;
	    return(-1);
	}

	/* Allocate each new item. */
	for(i = n, label_num = 0; i < row_ptr->total_items; i++, label_num++)
	{
	    row_ptr->item[i] = col_ptr = (colum_list_item_struct *)calloc(
		1, sizeof(colum_list_item_struct)
	    );
	    if(col_ptr == NULL)
		continue;

	    /* Set up values on this new item. */
	    free(col_ptr->label);
	    col_ptr->label = StringCopyAlloc(label[label_num]);

	    col_ptr->font = font;
	    col_ptr->pixel = pixel;
	    col_ptr->attr = attr;

	    col_ptr->client_data = NULL;

	    items_added++;
	}

        /* Redraw as needed. */
        if(list->map_state && (items_added > 0))
            CListDraw(list, CL_DRAW_AMOUNT_COMPLETE);

	return(0);
}

/*
 *      Adds (appends) an item to the list on the row row_num.
 *      row_num must be allocated.
 */
int CListAddItem(
        colum_list_struct *list,
        const char *label,
        font_t *font,
        pixel_t pixel,
        unsigned int attr,
        int row_num
)
{
	const char *val[1];
	const int nval = 1;

	val[0] = label;
	return(CListAddItems(
	    list, val, nval, font, pixel, attr, row_num
	));
}

/*
 *      Deletes the row item specified by colum_num on the row specified
 *      by row_num.
 */
static void CListDoDeleteItem(
        colum_list_struct *list,
        int row_num, int colum_num, int redraw
)
{
        colum_list_row_struct *row_ptr;
        colum_list_item_struct *col_ptr;


        if(list == NULL)
            return;

        /* Row valid and allocated? */
        if((row_num < 0) || (row_num >= list->total_rows))
            return;

        row_ptr = list->row[row_num];
        if(row_ptr == NULL)
            return;

        /* Row item valid and allocated? */
        if((colum_num < 0) || (colum_num >= row_ptr->total_items))
            return;
        col_ptr = row_ptr->item[colum_num];
        if(col_ptr == NULL)
            return;

        /* Deallocate memory on item structure. */
        free(col_ptr->label);
        col_ptr->label = NULL;

        /* Deallocate item structure itself. */
        free(col_ptr);
        row_ptr->item[colum_num] = col_ptr = NULL;     /* Mark as deleted. */

        /* Redraw as needed. */
        if(redraw)
            CListDraw(list, CL_DRAW_AMOUNT_COMPLETE);
}

/*
 *	Deletes the row item specified by colum_num on the row specified
 *	by row_num and redraws the list.
 */
void CListDeleteItem(
	colum_list_struct *list,
        int row_num, int colum_num
)
{
	if(list == NULL)
            return;

	CListDoDeleteItem(list, row_num, colum_num, list->map_state);
}


/*
 *	Initializes a colum list widget.
 */
int CListInit(
	colum_list_struct *list,
	win_t parent,
	int x, int y,
        unsigned int width,
        unsigned int height,
        void *client_data,
        int (*func_cb)(void *)
)
{
	win_attr_t wattr;


	if((list == NULL) ||
           (parent == 0)
	)
	    return(-1);


	list->map_state = 0;
	list->is_in_focus = 0;
        list->visibility_state = VisibilityFullyObscured;
        list->x = x;
        list->y = y;
        list->width = (width == 0) ? CL_DEF_WIDTH : width;
        list->height = (height == 0) ? CL_DEF_HEIGHT : height;
        list->disabled = False;
	list->font = OSWQueryCurrentFont();
	list->next = NULL;
	list->prev = NULL;

	list->option = CL_FLAG_ALLOW_DRAG | CL_FLAG_ALLOW_MULTI_SELECT;
	list->row_height = CL_DEF_ROW_HEIGHT;

	list->heading_split_indrag = False;

	list->colum = NULL;
	list->total_colums = 0;

	list->row = NULL;
	list->total_rows = 0;

	list->sel_row = NULL;
	list->total_sel_rows = 0;


	/* Toplevel. */
	if(
	    OSWCreateWindow(
		&list->toplevel,
		parent,
		list->x,
		list->y,
		list->width,
		list->height
	    )
	)
	    return(-1);
        OSWSetWindowInput(
            list->toplevel,
            KeyPressMask | KeyReleaseMask
        );


	/* Heading. */
	if(
            OSWCreateWindow(
                &list->heading,
                list->toplevel,
                0, 0,
                list->width,
                CL_HEADING_HEIGHT
            )
        )
            return(-1);
	OSWSetWindowInput(
	    list->heading,
	    ButtonPressMask | ButtonReleaseMask | PointerMotionMask |
	    ExposureMask
	);

        /* List. */
        if(
            OSWCreateWindow(
                &list->list,
                list->toplevel,
                0, CL_HEADING_HEIGHT,
                list->width,
                (int)list->height - CL_HEADING_HEIGHT
            )
        )
            return(-1);
        OSWSetWindowInput(
            list->list,
            ButtonPressMask | ButtonReleaseMask | PointerMotionMask |
            ExposureMask
        );

	/* List scroll bar. */
	OSWGetWindowAttributes(list->list, &wattr);
	if(
	    SBarInit(
	        &list->sb,
	        list->list,
	        wattr.width,
	        wattr.height
	    )
	)
            return(-1);

	/* Heading split bar. */
        if(
            OSWCreateWindow(
                &list->heading_split_bar,
                list->toplevel,
                0, 0,
                CL_SPLIT_BAR_WIDTH, list->height
            )
        )
            return(-1);
	OSWSetWindowBkg(
		list->heading_split_bar,
		osw_gui[0].white_pix,
		0
	);


	/* Set client data and callback function. */
	list->client_data = client_data;
	list->func_cb = func_cb;


	/* Add widget to regeristry. */
	WidgetRegAdd(list, WTYPE_CODE_COLUMLIST);


	return(0);
}

/*
 *	Resizes list.
 */
void CListResize(colum_list_struct *list)
{
	win_attr_t wattr;


        if(list == NULL)
            return;

	OSWGetWindowAttributes(list->toplevel, &wattr);
	if(((int)list->width == (int)wattr.width) &&
           ((int)list->height == (int)wattr.height)
	)
	    return;

	list->x = wattr.x;
	list->y = wattr.y;
	list->width = wattr.width;
	list->height = wattr.height;

	/* Make sure it's not too small. */
	if((int)wattr.height < (int)(CL_HEADING_HEIGHT + 4))
	{
	    OSWResizeWindow(
		list->toplevel,
		wattr.width,
		CL_HEADING_HEIGHT + 4
	    );

	    OSWGetWindowAttributes(list->toplevel, &wattr);
            list->width = wattr.width;
            list->height = wattr.height;
	}

	/* Heading. */
	OSWResizeWindow(
	    list->heading,
	    list->width,
	    CL_HEADING_HEIGHT
	);
	OSWDestroyPixmap(&list->heading_buf);

	/* List. */
        OSWMoveResizeWindow(
            list->list,
            0, CL_HEADING_HEIGHT,
	    list->width,
            (int)list->height - CL_HEADING_HEIGHT
        );
        OSWDestroyPixmap(&list->list_buf);

	/* List scroll bars. */
	OSWGetWindowAttributes(list->list, &wattr);
	SBarResize(
	    &list->sb,
	    wattr.width,
	    wattr.height
	);


	/* Heading split bar. */
        OSWMoveResizeWindow(
            list->heading_split_bar,
            0, 0,
            CL_SPLIT_BAR_WIDTH, list->height
        );


	return;
}


/*
 *	Redraws list.
 */
void CListDraw(colum_list_struct *list, int amount)
{
	int i, n;
	int is_selected;
	int x_pos, y_pos;
	int items_drawn, items_visable;
	font_t *prev_font;
	colum_list_row_struct **row_ptr;
	win_attr_t wattr;


	if(list == NULL)
	    return;


	/* Map as needed. */
	if(!list->map_state)
	{
	    OSWMapRaised(list->toplevel);

	    OSWMapWindow(list->heading);
	    OSWMapWindow(list->list);

	    OSWUnmapWindow(list->heading_split_bar);
	    list->map_state = 1;
	    amount = CL_DRAW_AMOUNT_COMPLETE;
	}

	/* Recreate buffers as needed. */
	if(list->heading_buf == 0)
	{
	    OSWGetWindowAttributes(list->heading, &wattr);
	    if(
		OSWCreatePixmap(&list->heading_buf,
		    wattr.width, wattr.height
		)
	    )
		return;
	}
        if(list->list_buf == 0)
        {
            OSWGetWindowAttributes(list->list, &wattr);
            if(
                OSWCreatePixmap(&list->list_buf,
                    wattr.width, wattr.height
                )
            )
                return;
        }

	/* Record previous font. */
        prev_font = OSWQueryCurrentFont();


	/* ******************************************************** */
	/* Draw heading. */
        if((amount == CL_DRAW_AMOUNT_COMPLETE) ||
           (amount == CL_DRAW_AMOUNT_HEADING)
        )
	{
            OSWGetWindowAttributes(list->heading, &wattr);

	    if(widget_global.force_mono)
	    {
                OSWClearPixmap(
                    list->heading_buf,
                    wattr.width,
                    wattr.height,
                    osw_gui[0].white_pix
                );
                OSWSetFgPix(osw_gui[0].black_pix);
	    }
	    else
	    {
		WidgetPutImageTile(
		    list->heading_buf,
		    widget_global.menu_bkg_img,
                    wattr.width,
                    wattr.height
		);
		OSWSetFgPix(widget_global.surface_shadow_pix);
	    }

	    for(i = 0; i < list->total_colums; i++)
	    {
		if(list->colum[i] == NULL)
		    continue;

		x_pos = CListGetColumPosition(list, i);
		OSWSetFont(list->colum[i]->font);

		OSWDrawString(
		    list->heading_buf,
		    x_pos + 6,
		    (CL_HEADING_HEIGHT / 2) + 5,
		    list->colum[i]->heading
		);

		OSWDrawLine(
		    list->heading_buf,
		    x_pos, 0,
		    x_pos, wattr.height
		);
	    }


            OSWPutBufferToWindow(list->heading, list->heading_buf);
	}

        /* ******************************************************** */
	/* Draw list. */ 
	if((amount == CL_DRAW_AMOUNT_COMPLETE) ||
           (amount == CL_DRAW_AMOUNT_LIST)
	)
	{
	    OSWGetWindowAttributes(list->list, &wattr);

            /* Redraw scroll bars if redrawing complete. */
            if(amount == CL_DRAW_AMOUNT_COMPLETE)
                SBarDraw(
                    &list->sb,
                    wattr.width,
                    wattr.height,
                    wattr.width,  
                    (list->total_rows + 1) * (int)list->row_height
                );

	    /* Clear background. */
	    OSWClearPixmap(
		list->list_buf,
		wattr.width,
		wattr.height,
		(widget_global.force_mono) ?
		    osw_gui[0].black_pix :
		    widget_global.surface_editable_pix
	    );


	    if(list->row_height == 0)
		list->row_height = 1;

	    y_pos = 2 -
		(list->sb.y_win_pos % (int)list->row_height);

	    items_drawn = 0;
	    items_visable = ((int)wattr.height / (int)list->row_height)
		+ 1;


	    /* Get row index i. */
	    i = list->sb.y_win_pos / (int)list->row_height;
	    if((i >= 0) && (i < list->total_rows))
	        row_ptr = &(list->row[i]);
	    else
		row_ptr = list->row;


	    while(i < list->total_rows)
	    {
		if(items_drawn > items_visable)
		    break;

		if(*row_ptr == NULL)
		{
		    i++; row_ptr++;
                    y_pos += list->row_height;
		    continue;
		}

		/* Is row selected? */
		is_selected = CListIsRowSelected(list, i);
		if(is_selected)
		{
		    if(widget_global.force_mono)
                        OSWSetFgPix(osw_gui[0].white_pix);
		    else
			OSWSetFgPix(widget_global.surface_selected_pix);

		    OSWDrawSolidRectangle(
			list->list_buf,
			0, y_pos,
			wattr.width,
			list->row_height
		    );
		}
 
		/* Draw each colum item. */
		for(n = 0; n < (*row_ptr)->total_items; n++)
		{
		    if((*row_ptr)->item[n] == NULL)
			continue;

		    if(is_selected)
		    {
			if(widget_global.force_mono)
                            OSWSetFgPix(osw_gui[0].black_pix);
                        else
                            OSWSetFgPix(widget_global.selected_text_pix);
		    }
		    else
		    {
                        if(widget_global.force_mono)
                            OSWSetFgPix(osw_gui[0].white_pix);
 			else
                            OSWSetFgPix((*row_ptr)->item[n]->pixel);
		    }
		    OSWSetFont((*row_ptr)->item[n]->font);


		    /* Get x_pos. */
		    x_pos = CListGetColumPosition(list, n);

		    /* Draw item label. */
		    OSWDrawString(
			list->list_buf,
			x_pos + 3,
			y_pos + 14,
			(*row_ptr)->item[n]->label
		    );
		}

		i++; row_ptr++;
		items_drawn++;
		y_pos += list->row_height;
	    }

            WidgetFrameButtonPixmap(
                list->list_buf,
                True,
                wattr.width,
                wattr.height,
                (widget_global.force_mono) ?
                    osw_gui[0].white_pix :
                    widget_global.surface_highlight_pix,
                (widget_global.force_mono) ?
                    osw_gui[0].white_pix :
                    widget_global.surface_shadow_pix
            );


	    OSWPutBufferToWindow(list->list, list->list_buf);
	}

	OSWSetFont(prev_font);


	return;
}


/*
 *	Manages colum list.
 */
int CListManage(colum_list_struct *list, event_t *event)
{
	int i, n, j, x, y_max;
	win_attr_t wattr;
	colum_list_colum_struct **colum_ptr;
	long ct;
	keycode_t keycode;
	bool_t single_select;
        int events_handled = 0;

	static long last_buttonpress = 0;


	if((event == NULL) ||
           (list == NULL)
	)
	    return(events_handled);


	if(!list->map_state &&
           (event->type != MapNotify)
	)
	    return(events_handled);


	switch(event->type)
	{
          /* ******************************************************* */
	  case KeyPress:
	    /* Do not handle any more keys if not in focus. */
	    if(!list->is_in_focus)
		break;

	    keycode = event->xkey.keycode;

	    /* Enter key. */
	    if((keycode == osw_keycode.enter) ||
               (keycode == osw_keycode.np_enter)
	    )
	    {
		if(list->func_cb != NULL)
		    list->func_cb(list->client_data);

		events_handled++;
		return(events_handled);
	    }
	    break;

	  /* ******************************************************* */
	  case KeyRelease:
            /* Do not handle any more keys if not in focus. */
            if(!list->is_in_focus)
                break;

	    break;

          /* ******************************************************* */
          case ButtonPress:
	    /* List window. */
            if(event->xany.window == list->list)
            {
                OSWGetWindowAttributes(list->list, &wattr);

		/* Put into focus on ButtonPress. */
		list->is_in_focus = 1;
		list->sb.is_in_focus = 1;

		/* Allow multiple selections? */
		if(list->option & CL_FLAG_ALLOW_MULTI_SELECT)
		{
		    if(!osw_gui[0].ctrl_key_state &&
		       !osw_gui[0].shift_key_state
		    )
		    {
			CListUnselectAllRows(list);
			single_select = True;
		    }
		    else
		    {
			single_select = False;
		    }
		}
		else
		{
		    CListUnselectAllRows(list);
		    single_select = True;
		}

		/* Get item number to be selected. */
		if(list->row_height != 0)
                    i = (event->xbutton.y + list->sb.y_win_pos - 3) /
		        (int)list->row_height;
		else
		    i = -1;

		/* Select a sequence of items? */
		if((list->option & CL_FLAG_ALLOW_MULTI_SELECT) &&
                   osw_gui[0].shift_key_state
		)
		{
		    if(list->total_sel_rows > 0)
			n = list->sel_row[list->total_sel_rows - 1];
		    else
			n = 0;
		    if((n >= 0) && (n < list->total_rows))
		    {
			if(i == n)
			{
			    /* Same item, don't do anything. */
			}
			else if(i > n)
			{
			    for(j = i; j > n; j--)
				CListSelectRow(list, j);
			}
                        else
                        {
                            for(j = i; j < n; j++)
                                CListSelectRow(list, j);
                        }
		    }
		}
		else
		{
		    /* Regular or multi select. */
		    CListSelectRow(list, i);
		}

		/* Redraw listing. */
		CListDraw(list, CL_DRAW_AMOUNT_LIST);


		/* Check for double click. */
		ct = MilliTime();
		if(((last_buttonpress + widget_global.double_click_int) >= ct) &&
                   (list->total_sel_rows > 0) &&
		   single_select
		)
		{
                    if(list->func_cb != NULL)
                        list->func_cb(list->client_data);

		    last_buttonpress = 0;
		}
		else
		{
		    last_buttonpress = ct;
		}


                /* Scroll as needed if near edge. */
                y_max = ((list->total_rows + 1) *
                    (int)list->row_height) - wattr.height;
		/* Scroll up? */
                if((event->xbutton.y < (int)list->row_height) &&
                   (list->sb.y_win_pos > 0)
                )
                {
                    SBarDraw(
                        &list->sb,
                        wattr.width,
                        wattr.height,
                        wattr.width, 
                        (list->total_rows + 1) * (int)list->row_height
                    );
                    list->sb.y_win_pos -= ((int)wattr.height / 2);

                    /* Sanitize scroll. */
                    if(list->sb.y_win_pos > y_max)
                        list->sb.y_win_pos = y_max;
                    if(list->sb.y_win_pos < 0)
                        list->sb.y_win_pos = 0;

                    usleep(300000);  

                    SBarDraw(
                        &list->sb,
                        wattr.width,
                        wattr.height,
                        wattr.width, 
                        (list->total_rows + 1) * (int)list->row_height
                    );
                    CListDraw(list, CL_DRAW_AMOUNT_LIST);
                }
                else if((event->xbutton.y > ((int)wattr.height - (int)list->row_height)) &&
                        (list->sb.y_win_pos < y_max)
		)
                {
                    SBarDraw(
                        &list->sb,
                        wattr.width,
                        wattr.height,
                        wattr.width,   
                        (list->total_rows + 1) * (int)list->row_height
                    );
                    list->sb.y_win_pos += ((int)wattr.height / 2);

                    /* Sanitize scroll. */
                    if(list->sb.y_win_pos > y_max)
                        list->sb.y_win_pos = y_max;
                    if(list->sb.y_win_pos < 0)
                        list->sb.y_win_pos = 0;

                    usleep(300000);

                    SBarDraw(
                        &list->sb,
                        wattr.width,
                        wattr.height,
                        wattr.width,
                        (list->total_rows + 1) * (int)list->row_height
                    );
                    CListDraw(list, CL_DRAW_AMOUNT_LIST);
		}

		events_handled++;
		return(events_handled);
            }
	    /* Heading bar. */
	    else if(event->xany.window == list->heading)
	    {
		i = list->heading_drag_colum;
		if((i >= 0) &&
                   (i < list->total_colums) &&
                   (i != 0)			/* Don't drag colum 0. */
		)
		{
		    if(list->colum[i] != NULL)
		    {
			list->heading_split_indrag = True;

			OSWMapRaised(list->heading_split_bar);

			OSWGrabPointer(
			    list->toplevel,
			    True,
			    ButtonReleaseMask | PointerMotionMask,
                            GrabModeAsync, GrabModeAsync,
			    list->toplevel,
                            (widget_global.h_split_wcr == NULL) ?
				osw_gui[0].std_cursor :
				widget_global.h_split_wcr->cursor
			);

			x = event->xbutton.x;
                        OSWMoveWindow(
                            list->heading_split_bar,
                            x - (CL_SPLIT_BAR_WIDTH / 2),
                            0
                        );
		    }
		}


		events_handled++;
                return(events_handled);
	    }
	    else
	    {
		/* ButtonPress occured on a window not belonging to
                 * this widget which implies that this widget
                 * is out of focus.
                 */
		if(list->is_in_focus)
		    list->is_in_focus = 0;
	    }
	    break;

          /* ******************************************************* */
          case ButtonRelease:
	    if(event->xany.window == list->toplevel)
	    {
		if(list->heading_split_indrag)
		{
		    /* Was in drag, so set new heading colum position. */

		    x = event->xbutton.x;

                    /* get drag colum number and sanitize x. */   
                    i = list->heading_drag_colum;
                    if((i >= 0) && (i < list->total_colums))
                    {
                        /* Sanitize x, left bound. */
                        if((i - 1) >= 0)
                        {
                            if(list->colum[i - 1] != NULL)
                            {
                                if(x <= list->colum[i - 1]->x_pos)
                                    x = list->colum[i - 1]->x_pos + 1;
                            }
                        }
                        /* Sanitize x, right bound. */
                        if((i + 1) < list->total_colums)
                        {
                            if(list->colum[i + 1] != NULL)
                            {
                                if(x >= list->colum[i + 1]->x_pos)
                                    x = list->colum[i + 1]->x_pos - 1;
                            }
                        }

                        /* Set new position of colum heading i. */
                        if(list->colum[i] != NULL)
			{
			    list->colum[i]->x_pos = x;
			}
                    }

                    events_handled++;


		    /* Heading split bar and stuff will be reset below. */
		}
	    }

	    /* Stop heading drag unconditionally. */
	    if(list->heading_split_indrag)
	    {
		OSWUnmapWindow(list->heading_split_bar);
		OSWUngrabPointer();
		list->heading_split_indrag = False;
	    }
            break;

          /* ******************************************************* */
	  case MotionNotify:
	    /* Heading bar. */
            if((event->xany.window == list->toplevel) ||
               (event->xany.window == list->heading)
	    )
	    {
		if(list->heading_split_indrag)
		{
		    x = event->xmotion.x;

		    /* get drag colum number and sanitize x. */
		    i = list->heading_drag_colum;
		    if((i >= 0) && (i < list->total_colums))
		    {
		        /* Sanitize x, left bound. */
		        if((i - 1) >= 0)
			{
			    if(list->colum[i - 1] != NULL)
			    {
				if(x <= list->colum[i - 1]->x_pos)
				    x = list->colum[i - 1]->x_pos + 1;
			    }
			}
			/* Sanitize x, right bound. */
			if((i + 1) < list->total_colums)
			{
                            if(list->colum[i + 1] != NULL)
                            {
                                if(x >= list->colum[i + 1]->x_pos)
                                    x = list->colum[i + 1]->x_pos - 1;
                            }
                        }
		    }

		    /* Move heading split bar. */
                    OSWMoveWindow(
                        list->heading_split_bar,
                        x - (CL_SPLIT_BAR_WIDTH / 2),
                        0
                    );

		}
		else
		{
		    x = event->xmotion.x;

		    for(i = 0, colum_ptr = list->colum;
                        i < list->total_colums;
		        i++, colum_ptr++
		    )
		    {
		        if(*colum_ptr == NULL)
			    continue;

			if((x > ((*colum_ptr)->x_pos - 2)) &&
                           (x <= ((*colum_ptr)->x_pos + 2))
			)
			{
			    if(list->heading_drag_colum != i)
			    {
				list->heading_drag_colum = i;
                                WidgetSetWindowCursor(
                                    list->heading,
                                    widget_global.h_split_wcr
                                );
			    }
			    break;
			}
		    }
		    if(i >= list->total_colums)
		    {
			if(list->heading_drag_colum >= 0)
			{
                            list->heading_drag_colum = -1;
                            OSWUnsetWindowCursor(list->heading);
			}
		    }
		}
	    }
	    /* List window. */
            else if(event->xany.window == list->list)
            {


	    }
	    break;

          /* ******************************************************* */
          case Expose:
	    if((event->xany.window == list->toplevel) ||
               (event->xany.window == list->heading) ||
               (event->xany.window == list->list)
	    )
	    {
		events_handled++;
	    }
            break;

          /* ******************************************************* */
          case UnmapNotify:
            if(event->xany.window == list->toplevel)
            {
                events_handled++;
                return(events_handled);
            }
            break;

          /* ******************************************************* */
          case MapNotify:
	    if(event->xany.window == list->toplevel)
	    {
		events_handled++;
		return(events_handled);
	    }
	    break;

          /* ******************************************************* */
          case VisibilityNotify:

            break;
	}

	/* Redraw as needed. */
	if(events_handled > 0)
	{
	    CListDraw(list, CL_DRAW_AMOUNT_COMPLETE);
	    return(events_handled);
	}

	/* Scroll bar. */
	if(events_handled == 0)
	{
	    OSWGetWindowAttributes(list->list, &wattr);
	    events_handled += SBarManage(
		&list->sb,
		wattr.width,
		wattr.height,
		wattr.width,
		(list->total_rows + 1) * (int)list->row_height,
		event
	    );

	    if(events_handled > 0)
	    {
		CListDraw(list, CL_DRAW_AMOUNT_LIST);
	    }
	}



	return(events_handled);
}


/*
 *	Maps colum list.
 */
void CListMap(colum_list_struct *list)
{
	if(list == NULL)
	    return;


	list->map_state = 0;
	CListDraw(list, CL_DRAW_AMOUNT_COMPLETE);

	list->visibility_state = VisibilityUnobscured;

	return;
}

/*
 *	Unmaps colum list.
 */
void CListUnmap(colum_list_struct *list)
{
        if(list == NULL)
            return; 


	OSWUnmapWindow(list->toplevel);

	OSWDestroyPixmap(&list->heading_buf);
	OSWDestroyPixmap(&list->list_buf);


        list->map_state = 0; 
        list->visibility_state = VisibilityFullyObscured;

        return;
}


/*
 *	Destroys colum list.
 */
void CListDestroy(colum_list_struct *list)
{
	int i;


	if(list == NULL)
	    return;


	/* Delete widget from regeristry. */
	WidgetRegDelete((void *)list);

	if(IDC())
	{
	    OSWDestroyWindow(&list->heading_split_bar);

	    SBarDestroy(&list->sb);

            OSWDestroyPixmap(&list->heading_buf);
            OSWDestroyWindow(&list->heading);

	    OSWDestroyPixmap(&list->list_buf); 
            OSWDestroyWindow(&list->list);

	    OSWDestroyWindow(&list->toplevel);
	}

	/* Set map state to 0 so deletions don't get redrawn. */
	list->map_state = 0;

	/* Delete headings. */
	for(i = 0; i < list->total_colums; i++)
	{
	    CListDeleteHeading(list, i);
	}
	free(list->colum);
	list->colum = NULL;
	list->total_colums = 0;

	/* Delete rows and selections. */
	CListDeleteAllRows(list);


        list->map_state = 0;
        list->x = 0;
        list->y = 0;
        list->width = 0;
        list->height = 0;
        list->visibility_state = VisibilityFullyObscured;
	list->font = NULL;
        list->disabled = False;
        list->next = NULL;
        list->prev = NULL;

	list->option = 0;
	list->row_height = CL_DEF_ROW_HEIGHT;

	list->client_data = NULL;
        list->func_cb = NULL;


	return;
}




