/*
	              ShipWars Plugin Sample Source


	To compile this, type:

	cc hellouniverse.c -o hellouniverse -shared -g


	This program won't run by itself, but is intented to be used
	as a plugin for the ShipWars Server.

	---

	This is a typical `hello world' (or `hello universe') piece
	of code.

	When this program is started, the very first function that
	will be called is SWPLUGIN_INIT_FUNC. If SWPLUGIN_INIT_FUNC
	returns 0, then the loading of this plugin will be considered
	successful.

	Upon successful loading, the function SWPLUGIN_MANAGE_FUNC
	will be called once per `cycle' (which is very often).

	If SWPLUGIN_MANAGE_FUNC ever returns -1, then the plugin
	will be unloaded.

	When this plugin is just about to be unloaded, the function
	SWPLUGIN_SHUTDOWN_FUNC will be called just before the actual
	unloading.


	See ../../plugins.h for prototypes and inputs for the above
	mentioned functions.

	NOTE: The functions below have been wrapped with extern "C".
	When using C instead of C++, this should be removed.
 */


#include "../include/plugins.h"


/*
 *	This is the initialize function, it is the first function
 *	called right after a successful loading of this plugin.
 *
 *	If this function returns 0, then the ShipWars Server will
 *	consider it a successful load and SWPLUGIN_MANAGE_FUNC will be
 *	called once per cycle afterwards.
 *
 *	If this function returns -1, then this plugin will be
 *	immediatly unloaded.
 *
 *	Take note of the input of this function, note the prototype.
 *	All ShipWars Server plugins must have a specific input
 *	for the init, manage, and shutdown functions.
 *	How to use the input will be shown on more intermediate
 *	examples, not in this one.
 */
extern "C" {
SWPLUGIN_INIT_FUNCRTN SWPLUGIN_INIT_FUNC(SWPLUGIN_INIT_FUNCINPUT)
{
	xsw_object_struct *obj_ptr;
	int (*con_notify)(int, char *);


	/* Got to have input. */
	if(in == NULL)
	    return(-1);


	/* Get pointer to connection notify function (see
	 * ../../plugins.h for all available members of this
	 * structure).
	 */
	con_notify = in->con_notify_fptr;

	/* The condescriptor is the connection ID for whoever
	 * initialized this plugin. Watch out though, condescriptor
	 * can be -1 if whoever initialized us wants to be
	 * anonymous or that this plugin was loaded when the server
	 * first started up.
	 */
	if(condescriptor >= 0)
		con_notify(
			condescriptor,
			"HelloUniverse!"
		);

	/* Must return 0 for success, this means that
	 * initialization was successful. If -1 was returned
	 * then it means initialization was unsuccessfuly and
	 * the SWPLUGIN_MANAGE_FUNC and SWPLUGIN_SHUTDOWN_FUNC
	 * will not be called.
	 */
	return(0);
}

/*
 *	This function is called once per `cycle' (and that's very
 *	often) if SWPLUGIN_INIT_FUNC returned 0.
 *
 *	If this function returns -1, then this plugin will be unloaded.
 *	SWPLUGIN_SHUTDOWN_FUNC will be called just before the actual
 *	unloading.
 *
 *	If this function returns 0, then it will be called again
 *	very soon.
 */
SWPLUGIN_MANAGE_FUNCRTN SWPLUGIN_MANAGE_FUNC(SWPLUGIN_MANAGE_FUNCINPUT)
{
	return(-1);
}

/*
 *	This function is called just before the plugin is unloaded,
 *	now is your last chance to deallocate any resources that
 *	the code in this plugin has allocated.
 */
SWPLUGIN_SHUTDOWN_FUNCRTN SWPLUGIN_SHUTDOWN_FUNC(SWPLUGIN_SHUTDOWN_FUNCINPUT)
{
	return;
}

}	// extern "C"

