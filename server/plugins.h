/*
                      ShipWars Server Plugins Management

	Also contains definations for actual ShipWars Server plugin,
	the header file plugins/include/plugins.h should #include
	this header file.

 */

#ifdef PLUGIN_SUPPORT

#ifndef PLUGINS_H
#define PLUGINS_H

#include "swserv.h"


/*
 *	Definations for both the ShipWars Server and Plugins for
 *	the ShipWars Server below this line.
 */

/*
 *	Plugin ID type:
 *
 *	Note, the plugin ID *is* the plugin index number.
 */
#define plugin_id_t	int


/*
 *	Plugin initialize, manage, and shutdown function names:
 */
#define SWPLUGIN_INIT_FUNCNAME		"swplugin_init"
#define SWPLUGIN_MANAGE_FUNCNAME	"swplugin_manage"
#define SWPLUGIN_SHUTDOWN_FUNCNAME	"swplugin_shutdown"

#define SWPLUGIN_INIT_FUNC		swplugin_init
#define SWPLUGIN_MANAGE_FUNC		swplugin_manage
#define SWPLUGIN_SHUTDOWN_FUNC		swplugin_shutdown

/*
 *	Inputs and prototypes for the initialize, mange, and shutdown
 *	functions:
 */
#define SWPLUGIN_INIT_FUNCPROTO		int, char **, \
                                        int, plugin_data_ref_struct *
#define SWPLUGIN_MANAGE_FUNCPROTO	plugin_data_ref_struct *
#define SWPLUGIN_SHUTDOWN_FUNCPROTO	plugin_data_ref_struct *

#define SWPLUGIN_INIT_FUNCINPUT		int argc, char **argv, \
					int condescriptor, \
					plugin_data_ref_struct *in
#define SWPLUGIN_MANAGE_FUNCINPUT	plugin_data_ref_struct *in
#define SWPLUGIN_SHUTDOWN_FUNCINPUT	plugin_data_ref_struct *in

#define SWPLUGIN_INIT_FUNCRTN		int
#define SWPLUGIN_MANAGE_FUNCRTN		int
#define SWPLUGIN_SHUTDOWN_FUNCRTN	void


/*
 *	Plugin data referance structure:
 *
 *	Contains pointers to global variables and functions on the
 *	ShipWars Server.
 *
 *	Actual ShipWars Server plugins will recieve this structure
 *	passed in the SWPLUGIN_INIT_FUNCNAME(), SWPLUGIN_MANGE_FUNCNAME(),
 *	and SWPLUGIN_SHUTDOWN_FUNCNAME() functions.
 */
typedef struct {

	time_t	*cur_millitime,
		*cur_systime,
		*lapsed_millitime;

	double	*time_compensation;

	xsw_object_struct ***xsw_object;
	int *total_objects;

	connection_struct ***connection;
	int *total_connections;


	/* Notifies connection (input 1) of message (input 2). */
	int (*con_notify_fptr)(int, char *);

	/* Checks if object is valid and non-garbage. First and second
	 * inputs are the objects pointer array and total_objects
	 * (respectivly) and the third input is the object index number.
	 * Returns 0 if object is valid and non-zero if it's invalid.
	 */
	int (*obj_is_garbage)(xsw_object_struct **, int, int);

	/* Creates a new object. It may change the values of the given
	 * pointer to the objects pointer array and total objects
	 * (inputs 1 and 2). The type specified must not be garbage
	 * (input 3). When in doubt, pass 1.
	 * Returns the index number of the newly created object or -1
	 * on error.
	 */
	int (*obj_create)(xsw_object_struct ***, int *, int);

	/* Recycles the object specified by the index value (input 3) from
	 * the given objects pointer array and total (inputs 1 and 2).
	 */
	void (*obj_recycle)(xsw_object_struct ***, int *, int);

	/* Checks if the two objects (inputs 3 and 4) are valid and in the
	 * same sector. The given objects pointer array and total are inputs
	 * 1 and 2.
	 */
	int (*obj_in_same_sector)(xsw_object_struct **, int, int, int);

        /* Checks if the two objects (inputs 3 and 4) are valid, in the
         * same sector, and in contact with each other.
	 * The given objects pointer array and total are inputs 1 and 2.
         */
        int (*obj_in_contact)(xsw_object_struct **, int, int, int);

        /* Checks if the two objects (inputs 3 and 4) are valid, in the 
         * same sector, and within the specified range (input 5) in
	 * Real units. The given objects pointer array and total are inputs   
         * 1 and 2.
         */
        int (*obj_in_range)(
		xsw_object_struct **, int,
		int, int,
		double		/* Distance in Real units. */
	);

        /* Checks if the two objects (inputs 3 and 4) are valid, in the
         * same sector, within the specified range (input 5) in
	 * Real units, if the first object is at the specified bearing
	 * to the second object and within the specified variance in
	 * radians (inputs 6 and 7).
	 * The given objects pointer array and total are inputs 1 and 2.
         */
        int (*obj_in_vector_range)(
		xsw_object_struct **, int,
		int, int,
		double,		/* Distance in Screen units. */
		double, double	/* Bearing and bearing variance, in radians. */	
	);

	/* Sends out values to all connections interested about the
	 * specified object (input 3).
	 * The given objects pointer array and total are inputs 1 and 2.
         */
	void (*obj_sync_clients)(xsw_object_struct **, int, int);

} plugin_data_ref_struct;



/*
 *	Definations for the ShipWars Server only below this line.
 */

/*
 *	Plugin record structure:
 *
 *	Records each loaded plugin.
 */
typedef struct {

	/* Name of plugin (usually plugin's executeable file name). */
	char *name;

	int argc;
	char **argv;

	/* Pointer to actual loaded plugin data. */
	void *handle;
	u_int64_t flags;

	int (*init_fptr)(int, char **, int, plugin_data_ref_struct *);
	int (*manage_fptr)(plugin_data_ref_struct *);
	void (*shutdown_fptr)(plugin_data_ref_struct *);

} plugin_record_struct;

extern plugin_record_struct **plugin;
extern int total_plugins;



extern int PluginIsLoaded(plugin_id_t id);
extern plugin_record_struct *PluginGetPointerFromID(plugin_id_t id);
extern plugin_id_t PluginMatchByName(char *name);

extern plugin_id_t PluginLoad(
	char *path,
	u_int64_t flags,
	int argc, char *argv[],
	int condescriptor
);

extern void PluginManage(void);
extern void PluginUnload(plugin_id_t id);
extern void PluginUnloadAll(void);


#endif	/* PLUGINS_H */

#endif	/* PLUGIN_SUPPORT */
