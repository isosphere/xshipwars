/*
                         Economy Window

	Functions:

	char *EcoWinGetRealNameFromOCSN(char *s)
	void EcoWinDeleteAllRealNames()

	void EcoWinUnfocusPrompts()
	int EcoWinDoAddInventory(
		int customer_obj, int proprietor_obj,
		xsw_ecoproduct_struct product
	)
	void EcoWinDoDeleteInventory()

	int EcoWinRefreshPBCB(void *ptr)
        int EcoWinBuyPBCB(void *ptr)
        int EcoWinSellPBCB(void *ptr)
	int EcoWinClosePBCB(void *ptr)

	int EcoWinInit()
	int EcoWinResize()
	int EcoWinDraw()
	int EcoWinManage(event_t *event)
	void EcoWinMap()	
	void EcoWinUnmap()
	void EcoWinDestroy()

	---

 */

#include "../include/unvmatch.h"
#include "../include/xsw_ctype.h"

#include "xsw.h"
#include "net.h"


#define EW_TITLE	"Buy or Sell"		/* Need better title. */
#define EW_TITLE_ICON	"Buy or Sell"

#define EW_WIDTH	512
#define EW_HEIGHT	480

#define EW_BTN_WIDTH	70
#define EW_BTN_HEIGHT	28

#define EW_NAME_PROMPT_NAME	"Proprietor:"
#define EW_VALUE_PROMPT_NAME	"Quantity:"


#define MIN(a,b)	((a) < (b)) ? (a) : (b)
#define MAX(a,b)	((a) > (b)) ? (a) : (b)


/* Real name structure. */
typedef struct {

	char *name;

} real_name_struct;



char *EcoWinGetRealNameFromOCSN(char *s);
void EcoWinDeleteAllRealNames();


/*
 *	Checks if string s is a real name token string
 *	and if so returns a statically allocated string
 *	containing the parsed name.  Returns s if s is not
 *	a real name token string.
 */
char *EcoWinGetParsedName(char *s)
{
	char *strptr;
	static char rtn_str[ECO_PRODUCT_NAME_MAX];


	if(s == NULL)
	    return(NULL);

	/* Skip leading spaces. */
	while(ISBLANK(*s))
	    s++;

	/* Not a token string? */
	if(*s != '$')
	{
	    strncpy(rtn_str, s, ECO_PRODUCT_NAME_MAX);
	    rtn_str[ECO_PRODUCT_NAME_MAX - 1] = '\0';
	    return(rtn_str);
	}

	/* Get delimiter character ':'. */
	strptr = strchr(s, ':');
	if(strptr == NULL)
	{   
            strncpy(rtn_str, s, ECO_PRODUCT_NAME_MAX);
            rtn_str[ECO_PRODUCT_NAME_MAX - 1] = '\0';
            return(rtn_str);
        }

	strncpy(
	    rtn_str,
	    strptr + 1,
	    ECO_PRODUCT_NAME_MAX
	);
	rtn_str[ECO_PRODUCT_NAME_MAX - 1] = '\0';

	return(rtn_str);
}


/*
 *	Procedure to delete all real names in inventory list.
 */
void EcoWinDeleteAllRealNames()
{
	int i, n;
	colum_list_struct *list;
	colum_list_row_struct *row_ptr;
	real_name_struct *rn_ptr;


	list = &eco_win.inventory;

	/* Go through each row on the list. */
	for(i = 0; i < list->total_rows; i++)
	{
	    row_ptr = list->row[i];
	    if(row_ptr == NULL)
		continue;

	    /* Go through each item on the row. */
	    for(n = 0; n < row_ptr->total_items; n++)
	    {
		/* Get pointer to real name structure on item. */
		rn_ptr = (real_name_struct *)CListGetItemDataPtr(
		    list,
		    i,	/* Row. */
		    n	/* Colum. */
		);
		if(rn_ptr == NULL)
		    continue;

		/* Deallocate real name structure. */
		free(rn_ptr->name);
		free(rn_ptr);

		/* Reset client_data on item. */
		CListSetItemDataPtr(list, i, n, NULL);
	    }
	}

	return;
}


/*
 *	Unfocus all prompts on economy window.
 */
void EcoWinUnfocusPrompts()
{
	eco_win.proprietor_prompt.is_in_focus = 0;
	eco_win.inventory.is_in_focus = 0;
	eco_win.amount_prompt.is_in_focus = 0;

	return;
}

/*
 *	Procedure to add an inventory item to the eco window.
 *
 *	If proprietor_obj is not the same object or is -1
 *	then all inventory items in the list are deleted.
 */
int EcoWinDoAddInventory(
	int customer_obj,
	int proprietor_obj,	/* Can be -1. */
        xsw_ecoproduct_struct product
)
{
	int i, status;
	const char *label;
	xsw_object_struct *proprietor_obj_ptr = NULL;
	colum_list_struct *list;
	real_name_struct *rn_ptr;


	list = &eco_win.inventory;


	/* Target object -1 or different from last? */
	if((proprietor_obj < 0) ||
           (eco_win.proprietor_obj != proprietor_obj)
	)
	{
	    /* Changed proprietor object, delete all invetory items. */

	    /* Delete all rows from inventory list. */
	    EcoWinDeleteAllRealNames();		/* Delete real names first. */
	    CListDeleteAllRows(list);

	    /* Get pointer to proprietor object if valid. */
	    if(!DBIsObjectGarbage(proprietor_obj))
		proprietor_obj_ptr = xsw_object[proprietor_obj];

            /* Change name in proprietor prompt. */
	    PromptSetS(
		&eco_win.proprietor_prompt,
		((proprietor_obj_ptr == NULL) ? "" : proprietor_obj_ptr->name)
	    );
	    PromptMarkBuffer(
		&eco_win.proprietor_prompt,
		PROMPT_POS_END
	    );
	}

	/* Do not continue if proprietor_obj is -1. */
	if(proprietor_obj < 0)
	{
	    /* Set proprietor object on eco window to none. */
	    eco_win.proprietor_obj = -1;

	    /* Redraw inventory list as needed. */
            if(eco_win.map_state)
                CListDraw(&eco_win.inventory, CL_DRAW_AMOUNT_COMPLETE);

	    return(0);
	}
	else
	{
            /* Set new proprietor object. */
            eco_win.proprietor_obj = proprietor_obj;
	}


	/* Check if product is already in inventory list. */
	for(i = 0; i < list->total_rows; i++)
	{
	    /* Get pointer to label. */
	    label = (const char *)CListGetItemLabel(
		list,
		i,	/* Row. */
		0	/* Colum. */
	    );
	    if(label == NULL)
		continue;

	    /* Get pointer to real name. */
            rn_ptr = (real_name_struct *)CListGetItemDataPtr(
		list,
		i,	/* Row. */
		0	/* Column. */
            );
            if(rn_ptr == NULL)
                continue;


	    /* Check real names. */
	    if(rn_ptr->name == NULL)
		continue;
            if(!strcasecmp(rn_ptr->name, product.name))
		break;
	}
	if(i < list->total_rows)
	{
	    /* List item already exists, update it. */
	    const char *val[5];
	    const int nval = 5;
	    char	sell_price_text[256],
			buy_price_text[256],
			amount_text[256],
			amount_max_text[256];


	    /* Name. */
	    val[0] = EcoWinGetParsedName(product.name);

            /* Sell price. */
	    sprintf(sell_price_text, "%.2f", product.sell_price);
	    val[1] = sell_price_text;

            /* Buy price. */
	    sprintf(buy_price_text, "%.2f", product.buy_price);
	    val[2] = buy_price_text;

            /* Amount. */
            if(product.amount_max >= 0)
                sprintf(amount_text, "%.2f", product.amount);
            else
                sprintf(amount_text, "-");
	    val[3] = amount_text;

            /* Amount max. */
	    if(product.amount_max >= 0)
                sprintf(amount_max_text, "%.2f", product.amount_max);
            else
                strcpy(amount_max_text, "nolimit");
	    val[4] = amount_max_text;

	    /* Update labels on row i. */
	    CListSetRowLabels(
		list, i, val, nval
	    );
	}
	else
	{
            const char *val[5];
            const int nval = 5;
            char        sell_price_text[256],
                        buy_price_text[256],
                        amount_text[256],
                        amount_max_text[256];


	    /* List item does not exist, allocate (append) it. */
	    if(CListAddRow(list, -1))
	        return(-1);

	    /* Get highest row index i on list. */
	    i = list->total_rows - 1;
	    if(i < 0)
		return(-1);

	    /* Get parsed product name. */
	    val[0] = EcoWinGetParsedName(product.name);

	    /* Sell price. */
	    sprintf(sell_price_text, "%.2f", product.sell_price);
	    val[1] = sell_price_text;

            /* Buy price. */
            sprintf(buy_price_text, "%.2f", product.buy_price);
	    val[2] = buy_price_text;

	    /* Amount. */
            if(product.amount_max >= 0)
                sprintf(amount_text, "%.2f", product.amount);
            else
                sprintf(amount_text, "-");
	    val[3] = amount_text;

            /* Amount max. */
            if(product.amount_max >= 0)
                sprintf(amount_max_text, "%.2f", product.amount_max);
            else
                sprintf(amount_max_text, "nolimit");
	    val[4] = amount_max_text;

	    /* Append these items to row i. */
	    status = CListAddItems(
                list,
                val, nval,
                osw_gui[0].std_font,
                widget_global.editable_text_pix,
                0,
                i		/* Row number. */
            );

	    /* Allocate and record real product name for this row if
	     * appending of the items were successful.
	     */
            if(!status)
            {
                rn_ptr = (real_name_struct *)calloc(
                    1, sizeof(real_name_struct)
                );
                if(rn_ptr == NULL)
                    return(-1);

                rn_ptr->name = StringCopyAlloc(product.name);

                /* Set item's client data to point to the real name
                 * struct we just allocated on the first item of the
		 * row.
                 */
                CListSetItemDataPtr(list, i, 0, rn_ptr);
            }

	}

	return(0);
}


/*
 *	Procedure to delete all items on economy window's
 *	inventory list (including each item's real name
 *	structure).
 */
void EcoWinDoDeleteInventory()
{
	xsw_ecoproduct_struct product;


	memset(&product, 0x00, sizeof(xsw_ecoproduct_struct));

	/* Delete all inventory when add input for both
	 * objects is -1.
	 */
	EcoWinDoAddInventory(
	    -1,		/* Source object. */
	    -1,		/* Target object. */
	    product
	);


	return;
}


int EcoWinRefreshPBCB(void *ptr)
{
	int len, proprietor_obj = -1;
	const char *proprietor_text;
	char text[256 + XSW_OBJ_NAME_MAX];


	/* Get value in proprietor_prompt. */
	proprietor_text = PromptGetS(&eco_win.proprietor_prompt);
	if(proprietor_text == NULL)
	    return(0);

	/* Check if name of proprietor object is too long. */
	len = (int)strlen(proprietor_text);
	if(len > XSW_OBJ_NAME_MAX)
	{
            printdw(
		&err_dw,
		"Proprietor object name length is too long!"
	    );
	    return(0);
	}

	/* Match new proprietor object. */
	if(((*proprietor_text) == '\0') ||
           !strcasecmp(proprietor_text, "me")
	)
	    proprietor_obj = net_parms.player_obj_num;
	else
	    proprietor_obj = MatchObjectByName(
		xsw_object, total_objects,
		proprietor_text,
		-1			/* Any type. */
	    );

	/* Unable to match proprietor object? */
	if(proprietor_obj < 0)
	{
	    sprintf(
		text,
		"Cannot find object:\n\n    `%s'",
		proprietor_text
	    );
	    printdw(&err_dw, text);
	}
	else
	{
	    /* Delete all inventory list items. */
	    EcoWinDoDeleteInventory();

	    /* Request new economy values from server. */
	    NetSendEcoReqValues(
		net_parms.player_obj_num,
		proprietor_obj
	    );
	}

	return(0);
}

/*
 *	Buy button callback.
 */
int EcoWinBuyPBCB(void *ptr)
{
	int i, n;
	char *strptr;
	colum_list_struct *list;
	colum_list_row_struct *row_ptr;
	real_name_struct *rn_ptr;
	xsw_ecoproduct_struct product;


	if(eco_win.proprietor_obj < 0)
	    return(0);


	/* Get amount from amount prompt. */
	strptr = eco_win.amount_prompt.buf;
	if(strptr != NULL)
	    product.amount = atof(strptr);
	else
	    product.amount = 0;


	/* Go through selected product items. */
	list = &eco_win.inventory;
	for(i = 0; i < list->total_sel_rows; i++)
	{
	    n = list->sel_row[i];
	    if((n < 0) || (n >= list->total_rows))
		continue;
	    row_ptr = list->row[n];
	    if(row_ptr == NULL)
		continue;

	    if(row_ptr->total_items < 1)
		continue;
	    if(row_ptr->item[0] == NULL)
		continue;

	    /* Get real name of product. */
	    rn_ptr = (real_name_struct *)row_ptr->item[0]->client_data;
	    if(rn_ptr == NULL)
		continue;

	    strptr = rn_ptr->name;
	    if(strptr != NULL)
		strncpy(product.name, strptr, ECO_PRODUCT_NAME_MAX);
	    else
		continue;
	    product.name[ECO_PRODUCT_NAME_MAX - 1] = '\0';

	    /* All other product information is not used. */

	    /* Buy. */
	    NetSendEcoBuy(
	        net_parms.player_obj_num,
	        eco_win.proprietor_obj,
	        product
	    );
	}


        return(0);
}

/*
 *	Sell button callback.
 */
int EcoWinSellPBCB(void *ptr)
{
        int i, n;
        char *strptr;
        colum_list_struct *list;
        colum_list_row_struct *row_ptr; 
        real_name_struct *rn_ptr;
        xsw_ecoproduct_struct product;


        if(eco_win.proprietor_obj < 0)
            return(0);


        /* Get amount from amount prompt. */
        strptr = eco_win.amount_prompt.buf;
        if(strptr != NULL)
            product.amount = atof(strptr);
        else
            product.amount = 0;


        /* Go through selected product items. */
        list = &eco_win.inventory;
        for(i = 0; i < list->total_sel_rows; i++)
        {
            n = list->sel_row[i]; 
            if((n < 0) || (n >= list->total_rows))
                continue;
            row_ptr = list->row[n];
            if(row_ptr == NULL)
                continue;

            if(row_ptr->total_items < 1)
                continue;
            if(row_ptr->item[0] == NULL)
                continue; 

            /* Get real name of product. */
            rn_ptr = (real_name_struct *)row_ptr->item[0]->client_data;
            if(rn_ptr == NULL)
                continue;

            strptr = rn_ptr->name;
            if(strptr != NULL)
                strncpy(product.name, strptr, ECO_PRODUCT_NAME_MAX);
            else
                continue;
            product.name[ECO_PRODUCT_NAME_MAX - 1] = '\0';

            /* All other product information is not used. */

            /* Sell. */
            NetSendEcoSell(
                net_parms.player_obj_num,
                eco_win.proprietor_obj,
                product
            );
        }


        return(0);
}


int EcoWinClosePBCB(void *ptr)
{
	EcoWinUnmap();

	return(0);
}




int EcoWinInit()
{
	int btn_x, btn_y;
        pixmap_t pixmap;
	win_attr_t wattr;


	if(!IDC())
	    return(-1);


        /* Reset values. */
        eco_win.map_state = 0;
        eco_win.x = 0;
        eco_win.y = 0;
        eco_win.width = EW_WIDTH;
        eco_win.height = EW_HEIGHT;
        eco_win.is_in_focus = 0;
        eco_win.visibility_state = VisibilityFullyObscured;
        eco_win.disabled = False;
        
        eco_win.proprietor_obj = -1;


        /* ******************************************************* */

        /* Toplevel. */
        if(
            OSWCreateWindow(
                &eco_win.toplevel,
                osw_gui[0].root_win,
                eco_win.x, eco_win.y,
                EW_WIDTH, EW_HEIGHT
            )
        )
            return(-1);

	eco_win.toplevel_buf = 0;

	OSWSetWindowInput(eco_win.toplevel, OSW_EVENTMASK_TOPLEVEL);

        WidgetCenterWindow(eco_win.toplevel, WidgetCenterWindowToRoot);
        OSWGetWindowAttributes(eco_win.toplevel, &wattr);
        eco_win.x = wattr.x;
        eco_win.y = wattr.y;
        eco_win.width = wattr.width;
        eco_win.height = wattr.height;


        /* WM properties. */  
        if(IMGIsImageNumAllocated(IMG_CODE_ECONOMY_ICON))
        {
            pixmap = OSWCreatePixmapFromImage( 
                xsw_image[IMG_CODE_ECONOMY_ICON]->image
            );
        }
        else
        {
            pixmap = widget_global.std_icon_pm;
        } 
        OSWSetWindowWMProperties(
            eco_win.toplevel,
            EW_TITLE,		/* Title. */
            EW_TITLE_ICON,	/* Icon title. */
            pixmap,		/* Icon. */
            False,		/* Let WM set coordinates? */
            eco_win.x, eco_win.y,
            100, 100,
            osw_gui[0].display_width, osw_gui[0].display_height,
            WindowFrameStyleStandard,
            NULL, 0
        );

	/* Set this window to be a transient for the bridge window. */
        OSWSetTransientFor(eco_win.toplevel, bridge_win.toplevel);


	/* Refresh button. */
	btn_x = eco_win.width - (1 * EW_BTN_WIDTH) - (1 * 10);
	btn_y = (2 * 10) + (1 * EW_BTN_HEIGHT);
        if(
            PBtnInit(
                &eco_win.refresh_btn,
                eco_win.toplevel, 
                btn_x, btn_y,
                EW_BTN_WIDTH, EW_BTN_HEIGHT,
                "Refresh",
                PBTN_TALIGN_CENTER,
                NULL,
                (void *)&eco_win.refresh_btn,
                EcoWinRefreshPBCB
            )
        )
            return(-1);


	/* Proprietor prompt. */
	if(
	    PromptInit(
		&eco_win.proprietor_prompt,
		eco_win.toplevel,
		0,
		(2 * 10) + EW_BTN_HEIGHT,
		MAX(eco_win.width - (1 * EW_BTN_WIDTH) - (2 * 10), 10),
		30,
		PROMPT_STYLE_FLUSHED,
		"Proprietor:",
		XSW_OBJ_NAME_MAX,
		0,
		NULL
	    )
	)
	    return(-1);

	/* Inventory colum list. */
	if(
	    CListInit(
                &eco_win.inventory,
                eco_win.toplevel,
                0, 55 + EW_BTN_HEIGHT,
		eco_win.width,
		(int)eco_win.height - 175,
		(void *)&eco_win.inventory,
                NULL
	    )
	)
	    return(-1);
	/* Heading for colum list. */
	CListAddHeading(
            &eco_win.inventory,
            "Product",
            osw_gui[0].std_font,
            widget_global.normal_text_pix,
            0,			/* Attributes. */
	    0			/* Start position. */
	);
        CListAddHeading(
            &eco_win.inventory,
            "Sell Price",
            osw_gui[0].std_font,        
            widget_global.normal_text_pix,        
            0,                  /* Attributes. */
            200                 /* Start position. */
        );
        CListAddHeading(
            &eco_win.inventory,
            "Buy Price",
            osw_gui[0].std_font,
            widget_global.normal_text_pix,
            0,                  /* Attributes. */
            280                 /* Start position. */
        );
        CListAddHeading(
            &eco_win.inventory,
            "Amount",        
            osw_gui[0].std_font,        
            widget_global.normal_text_pix,        
            0,                  /* Attributes. */
            360                 /* Start position. */
        );
        CListAddHeading(
            &eco_win.inventory,
            "Total",
            osw_gui[0].std_font,        
            widget_global.normal_text_pix,        
            0,                  /* Attributes. */
            440                 /* Start position. */
        );


        /* Amount prompt. */
        if(
            PromptInit(
                &eco_win.amount_prompt,
                eco_win.toplevel,
                0, 60 + EW_BTN_HEIGHT + (int)eco_win.height - 180,
                eco_win.width, 30,
                PROMPT_STYLE_FLUSHED,
                "Quantity:",
                64,			/* Buffer length. */
                0,
                NULL
            )
        )
            return(-1);
	PromptSetI(&eco_win.amount_prompt, 0);

	/* Link prompts togeather. */
	eco_win.proprietor_prompt.next = &eco_win.amount_prompt;
	eco_win.proprietor_prompt.prev = &eco_win.amount_prompt;

	eco_win.amount_prompt.next = &eco_win.proprietor_prompt;
	eco_win.amount_prompt.prev = &eco_win.proprietor_prompt;


	/* Buy button. */
	btn_x = (0 * EW_BTN_WIDTH) + (1 * 10);
	btn_y = (int)eco_win.height - EW_BTN_HEIGHT - 10;
	if(
	    PBtnInit(
		&eco_win.buy_btn,
		eco_win.toplevel,
		btn_x, btn_y,
		70, EW_BTN_HEIGHT,
		"Buy",
		PBTN_TALIGN_CENTER,
		NULL,
		(void *)&eco_win.buy_btn,
		EcoWinBuyPBCB
	    )
	)
	    return(-1);

        /* Sell button. */
        btn_x = (1 * EW_BTN_WIDTH) + (2 * 10);
        btn_y = (int)eco_win.height - 38;
        if(
            PBtnInit(
                &eco_win.sell_btn,
                eco_win.toplevel,
                btn_x, btn_y,
                70, EW_BTN_HEIGHT,
                "Sell",
                PBTN_TALIGN_CENTER,
                NULL,
                (void *)&eco_win.sell_btn,
                EcoWinSellPBCB
            )
        )
            return(-1);

        /* Close button. */
	btn_x = (int)eco_win.width - EW_BTN_WIDTH - 10;
        if(
            PBtnInit(
                &eco_win.close_btn,
                eco_win.toplevel,
                btn_x, btn_y,                 
                70, EW_BTN_HEIGHT,
                "Close",
                PBTN_TALIGN_CENTER,
                NULL,
                (void *)&eco_win.close_btn,
                EcoWinClosePBCB
            )
        )
            return(-1);




	return(0);
}


int EcoWinResize()
{
	int btn_x, btn_y;
	win_attr_t wattr;


	OSWGetWindowAttributes(eco_win.toplevel, &wattr);
	if((eco_win.width == (unsigned int)wattr.width) &&
           (eco_win.height == (unsigned int)wattr.height)
	)
	    return(0);

	eco_win.x = wattr.x;
	eco_win.y = wattr.y;
	eco_win.width = wattr.width;
        eco_win.height = wattr.height;


	/* Refresh button. */
        btn_x = eco_win.width - (1 * EW_BTN_WIDTH) - (1 * 10);
        btn_y = (2 * 10) + (1 * EW_BTN_HEIGHT);
	OSWMoveWindow(
            eco_win.refresh_btn.toplevel,
            btn_x, btn_y
	);

	/* Proprietor prompt. */
	OSWMoveResizeWindow(
	    eco_win.proprietor_prompt.toplevel,
            0,
            (2 * 10) + EW_BTN_HEIGHT,
            MAX(eco_win.width - (1 * EW_BTN_WIDTH) - (2 * 10), 10),
            30
	);
        PromptMap(&eco_win.proprietor_prompt);

	/* Inventory colum list. */
        OSWMoveResizeWindow(
            eco_win.inventory.toplevel,
            0, 55 + EW_BTN_HEIGHT,
            eco_win.width, 
            (int)eco_win.height - 175
	);
	CListResize(&eco_win.inventory);

        /* Amount prompt. */
        OSWMoveResizeWindow(
            eco_win.amount_prompt.toplevel,
            0, 60 + EW_BTN_HEIGHT + (int)eco_win.height - 180,
            eco_win.width, 30
        );
        PromptMap(&eco_win.amount_prompt);


	/* Buy button. */
        btn_x = (0 * EW_BTN_WIDTH) + (1 * 10);
        btn_y = (int)eco_win.height - EW_BTN_HEIGHT - 10;
        OSWMoveWindow(
            eco_win.buy_btn.toplevel,
            btn_x, btn_y
        );

        /* Sell button. */
        btn_x = (1 * EW_BTN_WIDTH) + (2 * 10);
        OSWMoveWindow(
            eco_win.sell_btn.toplevel,
            btn_x, btn_y
        );

	/* Close button. */
        btn_x = (int)eco_win.width - EW_BTN_WIDTH - 10;
        OSWMoveWindow(
            eco_win.close_btn.toplevel,
            btn_x, btn_y
        );


	/* Destroy buffer. */
	OSWDestroyPixmap(&eco_win.toplevel_buf);


	return(0);
}


int EcoWinDraw()
{
	int y, proprietor_obj_num, player_obj_num;
	xsw_object_struct *proprietor_obj_ptr, *player_obj_ptr;
	win_t w;
	pixmap_t pixmap;
	font_t *prev_font;
	win_attr_t wattr;
	char text[512];


	/* Map as needed. */
	if(!eco_win.map_state)
	{
	    OSWMapRaised(eco_win.toplevel);

            PBtnMap(&eco_win.refresh_btn);
            PromptMap(&eco_win.proprietor_prompt);
            CListMap(&eco_win.inventory);
	    PromptMap(&eco_win.amount_prompt);

	    PBtnMap(&eco_win.buy_btn);
            PBtnMap(&eco_win.sell_btn);
            PBtnMap(&eco_win.close_btn);

	    eco_win.visibility_state = VisibilityUnobscured;
	    eco_win.map_state = 1;
	}


	/* ******************************************************** */
	/* Recreate buffers as needed. */

	if(eco_win.toplevel_buf == 0)
	{
	    OSWGetWindowAttributes(eco_win.toplevel, &wattr);
            if(OSWCreatePixmap(&eco_win.toplevel_buf,
		wattr.width, wattr.height)
	    )
		return(-1);
	}


	/* Record previous font. */
	prev_font = OSWQueryCurrentFont();


        /* ******************************************************** */
        /* Redraw toplevel. */
	if(1)
	{
	    w = eco_win.toplevel;
	    pixmap = eco_win.toplevel_buf;

	    /* Get number and pointer to proprietor object. */
	    proprietor_obj_num = eco_win.proprietor_obj;
            if(DBIsObjectGarbage(proprietor_obj_num))
	    {
                proprietor_obj_num = -1;
		proprietor_obj_ptr = NULL;
	    }
	    else
	    {
                proprietor_obj_ptr = xsw_object[proprietor_obj_num];
	    }

	    /* Get number and pointer to player object. */
	    player_obj_num = net_parms.player_obj_num;
	    player_obj_ptr = net_parms.player_obj_ptr;


	    OSWGetWindowAttributes(w, &wattr);


	    /* Redraw background. */
	    if(widget_global.force_mono)
	    {
		OSWClearPixmap(
		    pixmap,
		    wattr.width, wattr.height,
		    osw_gui[0].black_pix
		);

		y = (int)wattr.height - EW_BTN_HEIGHT - 25;
		OSWSetFgPix(osw_gui[0].white_pix);
		OSWDrawLine(
		    pixmap,
		    0, y, wattr.width, y
		);

		OSWSetFgPix(osw_gui[0].white_pix);
	    }
	    else
	    {
		WidgetPutImageTile(
		    pixmap,
		    widget_global.std_bkg_img,
		    wattr.width, wattr.height
		);

		y = (int)wattr.height - EW_BTN_HEIGHT - 25;
                OSWSetFgPix(widget_global.surface_shadow_pix);
                OSWDrawLine(
		    pixmap,
                    0, y, wattr.width, y
                );
                OSWSetFgPix(widget_global.surface_highlight_pix);
                OSWDrawLine(
		    pixmap,
                    0, y + 1, wattr.width, y + 1
                );

                OSWSetFgPix(widget_global.normal_text_pix);
            }   


	    /* Check if player object is valid. */
	    if(player_obj_ptr != NULL)
	    {
		/* Print player's credits. */
		sprintf(
		    text,
		    "      Your Credits: %.2f",
		    ((player_obj_ptr->score == NULL) ? 0 :
			player_obj_ptr->score->credits)
		);
		OSWDrawString(
		    pixmap,
		    8,
		    22,
		    text
		);
	    }

	    /* Check if proprietor object is valid. */
            if(proprietor_obj_ptr != NULL)
            {
		/* Check if economy data structure is allocated. */
                if(proprietor_obj_ptr->eco != NULL)
                {
		    /* Opened or closed. */
                    sprintf(
			text,
			"%s",
                        ((proprietor_obj_ptr->eco->flags & ECO_FLAG_OPEN) ?
			    "Opened" : "Closed"
			)
                    );
                    OSWDrawString(
                        pixmap,
                        280,
                        22,
                        text
                    );

		    /* Tax friend/general/hostile. */
                    sprintf(
			text,
			"Tax: %.0f/%.0f/%.0f%%",
			(double)(proprietor_obj_ptr->eco->tax_friend - 1) * 100,
			(double)(proprietor_obj_ptr->eco->tax_general - 1) * 100,
			(double)(proprietor_obj_ptr->eco->tax_hostile - 1) * 100
                    );
                    OSWDrawString(
                        pixmap,
                        280,
                        40,
                        text
                    );
		}

		/* Check if scores are allocated. */
                if(proprietor_obj_ptr->score != NULL)
                {
		    /* Print proprietor's credits. */
                    sprintf(
			text,
			"Proprietor Credits: %.2f",
                        proprietor_obj_ptr->score->credits
                    );
                    OSWDrawString(
                        pixmap,
                        8,
                        40,
                        text
                    );
                }
            }


	    OSWPutBufferToWindow(w, pixmap);
	}

	/* Set back previous font. */
	OSWSetFont(prev_font);


	return(0);
}


int EcoWinManage(event_t *event)
{
	int events_handled = 0;


	if(event == NULL)
	    return(events_handled);

	if(!eco_win.map_state &&
           (event->type != MapNotify)
	)
	    return(events_handled);


	switch(event->type)
	{
          /* ******************************************************** */
	  case KeyPress:
	    if(!eco_win.is_in_focus)
	        return(events_handled);

	    /* Escape. */
	    if(event->xkey.keycode == osw_keycode.esc)
	    {
		EcoWinClosePBCB(
		    (void *)&eco_win.close_btn
		);

		events_handled++;
	    }
            /* Enter. */
            else if((event->xkey.keycode == osw_keycode.enter) ||
                    (event->xkey.keycode == osw_keycode.np_enter)
	    )
            {
                /* Ignore enter but count it as an event. */

                events_handled++;
            }
	    if(events_handled > 0)
		return(events_handled);

	    break;

          /* ******************************************************** */
          case Expose:
            if(event->xany.window == eco_win.toplevel)
	    {
		events_handled++;
	    }
            break;

          /* ******************************************************** */
          case UnmapNotify:
            if(event->xany.window == eco_win.toplevel)
            {
		EcoWinUnmap();

                events_handled++;
		return(events_handled);
            }
            break;

          /* ******************************************************** */
          case MapNotify:
            if(event->xany.window == eco_win.toplevel)
            {
		if(!eco_win.map_state)
		    EcoWinMap();

                events_handled++;
                return(events_handled);
            }
            break;

          /* ******************************************************** */
          case ClientMessage:
            if(OSWIsEventDestroyWindow(eco_win.toplevel, event))
            {
                EcoWinUnmap();
                events_handled++;
                return(events_handled);
            }
            break;

	  /* ******************************************************** */
          case FocusIn:
            if(event->xany.window == eco_win.toplevel)   
            {
                eco_win.is_in_focus = 1;

                events_handled++;
            }
            break;

          /* ******************************************************** */
          case FocusOut:
            if(event->xany.window == eco_win.toplevel)
            {
		EcoWinUnfocusPrompts();

                eco_win.is_in_focus = 0;

                events_handled++;
            }
            break;

          /* ********************************************************* */
          case ConfigureNotify:
            if(event->xany.window == eco_win.toplevel)
            {
                EcoWinResize();

                events_handled++;
            }
            break;
	}

	/* Redraw as needed. */
	if(events_handled > 0)
	{
	    EcoWinDraw();
	}


	/* ********************************************************* */
	/* Manage widgets. */

        /* Refresh button. */
        if(events_handled == 0)
            events_handled += PBtnManage(&eco_win.refresh_btn, event);

	/* Proprietor prompt. */
	if(events_handled == 0)
            events_handled += PromptManage(&eco_win.proprietor_prompt, event);

        /* Inventory colum list. */   
        if(events_handled == 0)
            events_handled += CListManage(&eco_win.inventory, event);

	/* Amount prompt. */
        if(events_handled == 0)
            events_handled += PromptManage(&eco_win.amount_prompt, event);


	/* Buy button. */
        if(events_handled == 0)
            events_handled += PBtnManage(&eco_win.buy_btn, event);

	/* Sell button. */
        if(events_handled == 0)
            events_handled += PBtnManage(&eco_win.sell_btn, event);

	/* Close button. */
        if(events_handled == 0)
            events_handled += PBtnManage(&eco_win.close_btn, event);


	return(events_handled);
}


void EcoWinMap()
{
        /* Unfocus all XSW windows. */  
        XSWDoUnfocusAllWindows();


	/* Unfocus all eco win prompts. */
	EcoWinUnfocusPrompts();


	/* Map economy window. */
	eco_win.map_state = 0;
	EcoWinDraw();

        eco_win.amount_prompt.is_in_focus = 1;   
	eco_win.is_in_focus = 1;


        /* Restack all XSW windows. */
        XSWDoRestackWindows();


	return;
}


void EcoWinUnmap()
{
        PBtnUnmap(&eco_win.refresh_btn); 
        PromptUnmap(&eco_win.proprietor_prompt);
        CListUnmap(&eco_win.inventory);
        PromptUnmap(&eco_win.amount_prompt);
        PBtnUnmap(&eco_win.buy_btn);
        PBtnUnmap(&eco_win.sell_btn);
        PBtnUnmap(&eco_win.close_btn);

	OSWUnmapWindow(eco_win.toplevel);
	eco_win.map_state = 0;
	eco_win.is_in_focus = 0;
	eco_win.visibility_state = VisibilityFullyObscured;

	/* Destroy large buffers. */
	OSWDestroyPixmap(&eco_win.toplevel_buf);


	return;
}



void EcoWinDestroy()
{

	/* Delete all real names from inventory list items first. */
	EcoWinDeleteAllRealNames();


	if(IDC())
	{
	    PBtnDestroy(&eco_win.buy_btn);
            PBtnDestroy(&eco_win.sell_btn);
            PBtnDestroy(&eco_win.close_btn);

	    PromptDestroy(&eco_win.amount_prompt);
	    CListDestroy(&eco_win.inventory);
            PromptDestroy(&eco_win.proprietor_prompt);

	    PBtnDestroy(&eco_win.refresh_btn);

            OSWDestroyWindow(&eco_win.toplevel);
            OSWDestroyPixmap(&eco_win.toplevel_buf);
	}


	/* Reset values. */
        eco_win.map_state = 0;
        eco_win.x = 0;
        eco_win.y = 0;
        eco_win.width = 0;
        eco_win.height = 0;
        eco_win.is_in_focus = 0;
        eco_win.visibility_state = VisibilityFullyObscured;
	eco_win.disabled = False;

        eco_win.proprietor_obj = -1;


	return;
}
