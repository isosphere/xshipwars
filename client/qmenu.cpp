/*
                           Quick Menu Management

	Functions:

	void QMenuAllocDefaultCommands(void)

	int QMenuHandleCB(void *client_data, int op_code)

	int QMenuInit(void)
	int QMenuDraw(void)
	int QMenuManage(event_t *xevent)
	void QMenuMap(void)
	void QMenuUnmap(void)
	void QMenuDeleteAllCommands(void)
	void QMenuDestroy(void)

	---


 */

#include "xsw.h"


/*
 *	Procedure to load default quick menu commands.
 */
void QMenuAllocDefaultCommands(void)
{
	int i = 0;
	int total = 6;
	qmenu_command_struct *cmd_ptr;


	/* Free commands if they are already loaded. */
	if(total_qmenu_commands > 0)
	    QMenuDeleteAllCommands();


	/* Allocate pointers. */
	total_qmenu_commands = total;
	qmenu_command = (qmenu_command_struct **)realloc(
	    qmenu_command,
	    total_qmenu_commands * sizeof(qmenu_command_struct *)
	);
	if(qmenu_command == NULL)
	{
	    total_qmenu_commands = 0;
	    return;
	}


	/* Begin allocating default commands. */

	/* Heading. */
	qmenu_command[i] = (qmenu_command_struct *)calloc(
            1,
            sizeof(qmenu_command_struct)
        );
	cmd_ptr = qmenu_command[i];
	if(cmd_ptr != NULL)
	{
	    char text[256];


	    cmd_ptr->type = MENU_ITEM_TYPE_COMMENT;
	    cmd_ptr->action = XSW_ACTION_NONE;

	    sprintf(text, "%s Commands", PROG_NAME);
	    cmd_ptr->name = StringCopyAlloc(text);
	}

	/* HR. */
	i++;
        qmenu_command[i] = (qmenu_command_struct *)calloc(
            1,
            sizeof(qmenu_command_struct)
        );
        cmd_ptr = qmenu_command[i];
        if(cmd_ptr != NULL)
	{
	    cmd_ptr->type = MENU_ITEM_TYPE_HR;
	    cmd_ptr->action = XSW_ACTION_NONE;
	}

	/* Connect... */
	i++;
        qmenu_command[i] = (qmenu_command_struct *)calloc(
            1,
            sizeof(qmenu_command_struct)
        );
        cmd_ptr = qmenu_command[i];
        if(cmd_ptr != NULL)
        {
            cmd_ptr->type = MENU_ITEM_TYPE_ENTRY;
	    cmd_ptr->action = XSW_ACTION_CONNECT;
	    cmd_ptr->name = StringCopyAlloc("Connect...");
	}

	/* Refresh. */
	i++;
        qmenu_command[i] = (qmenu_command_struct *)calloc(
            1,
            sizeof(qmenu_command_struct)
        );
        cmd_ptr = qmenu_command[i];
        if(cmd_ptr != NULL)
        {
            cmd_ptr->type = MENU_ITEM_TYPE_ENTRY;
	    cmd_ptr->action = XSW_ACTION_REFRESH;
            cmd_ptr->name = StringCopyAlloc("Refresh");
	}

	/* Auto interval toggle. */
	i++;
        qmenu_command[i] = (qmenu_command_struct *)calloc(
            1,
            sizeof(qmenu_command_struct)
        );
        cmd_ptr = qmenu_command[i];
        if(cmd_ptr != NULL)
        {
            cmd_ptr->type = MENU_ITEM_TYPE_ENTRY;
	    cmd_ptr->action = XSW_ACTION_AINT;
	    cmd_ptr->name = StringCopyAlloc("Auto Interval Toggle");
        }

        /* Disconnect. */
        i++;
        qmenu_command[i] = (qmenu_command_struct *)calloc(
            1,
            sizeof(qmenu_command_struct)
        );
        cmd_ptr = qmenu_command[i];
        if(cmd_ptr != NULL)
        {
            cmd_ptr->type = MENU_ITEM_TYPE_ENTRY;
            cmd_ptr->action = XSW_ACTION_DISCONNECT;
            cmd_ptr->name = StringCopyAlloc("Disconnect");
        }

	return;
}

/*
 *      Quick menu callback handler.
 */
int QMenuHandleCB(void *client_data, int op_code)
{
        XSWActionCB(
            &bridge_win,        /* Pointer to window structure. */
            NULL,               /* Data pointer. */
            op_code             /* Action. */
        );
          
        return(0);
}

/*
 *	Initialize quick menu.
 */
int QMenuInit(void)
{
	int i;
	qmenu_command_struct *cmd_ptr;


	/* Initialize menu. */
	if(MenuInit(
	    &qmenu.menu,
	    osw_gui[0].root_win,
	    QMenuHandleCB,
	    &qmenu
	))
            return(-1);


	/* Check if quick menu commands are defined (loaded from
	 * configuration file).
	 */
	if(total_qmenu_commands <= 0)
	{
	    /* No defined entries loaded from configuartion, define
	     * default commands.
	     */
	    QMenuAllocDefaultCommands();
	}


        for(i = 0; i < total_qmenu_commands; i++)
	{
	    cmd_ptr = qmenu_command[i];
	    if(cmd_ptr == NULL)
		continue;

	    MenuAddItem(
		&qmenu.menu,
		cmd_ptr->name,
		cmd_ptr->type,
		NULL,		/* No icon. */
		cmd_ptr->action,
		-1		/* Append. */
	    );
	}


	return(0);
}

/*
 *	Redraws the quick menu.
 */
int QMenuDraw(void)
{
	MenuMap(&qmenu.menu);

	return(0);
}

/*
 *	Manages the quick menu.
 */
int QMenuManage(event_t *event)
{
	int events_handled = 0;


	if(event == NULL)
	    return(0);

        /* Manage menu. */
	if(events_handled == 0)
	    events_handled += MenuManage(&qmenu.menu, event);

	return(events_handled);
}

/*
 *	Maps the quick menu.
 */
void QMenuMap(void)
{
	int x, y;

	/* Get pointer position. */
	OSWGetPointerCoords(
	    osw_gui[0].root_win,
	    NULL, NULL,
	    &x, &y
	);

	/* Move the menu's toplevel window. */
	qmenu.menu.x = x;
	qmenu.menu.y = y;

	/* Map the quick menu. */
	QMenuDraw();

	return;
}

/*
 *	Unmaps the quick menu.
 */
void QMenuUnmap(void)
{
	if(!IDC())
	    return;

	MenuUnmap(&qmenu.menu);

	return;
}

/*
 *	Deallocate all quick menu commands.
 */
void QMenuDeleteAllCommands(void)
{
	int i;
	qmenu_command_struct *cmd_ptr;

	for(i = 0; i < total_qmenu_commands; i++)
	{
	    cmd_ptr = qmenu_command[i];
	    if(cmd_ptr == NULL)
		continue;

	    free(cmd_ptr->name);
	    OSWDestroyImage(&cmd_ptr->icon);

	    free(cmd_ptr);
	}

        free(qmenu_command);
        qmenu_command = NULL;

	total_qmenu_commands = 0;


	return;
}

/*
 *	Destroys quick menu, deallocating its menu widget and
 *	the commands list.
 */
void QMenuDestroy(void)
{
        /* Free all allocated Quick Menu commands. */
        QMenuDeleteAllCommands();

	if(IDC())
	{
	    /* Destroy the menu widget. */
	    MenuDestroy(&qmenu.menu);
	}


	return;
}
