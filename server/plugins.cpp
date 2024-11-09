/*
                           Plugins Management

	Functions:

	int PluginWObjectCreate(xsw_object_struct ***ptr, int *total, int t)
	void PluginWObjectRecycle(xsw_object_struct ***ptr, int *total, int n)
	int PluginWObjectInVectorRange(
	        xsw_object_struct **ptr, int total,
	        int o1, int o2,
	        double range,
	        double bearing, double variance
	)
	void PluginWObjectSyncClients(xsw_object_struct **ptr, int total, int t)

	int PluginIsLoaded(plugin_id_t id)
	plugin_record_struct *PluginGetPointerFromID(plugin_id_t id)
	plugin_id_t PluginMatchByName(char *name)

	void PluginSetupDataRef(plugin_data_ref_struct *in)

	plugin_id_t PluginLoad(
	        char *path,
		u_int64_t flags,
	        int argc, char *argv[],
	        int condescriptor
	)
	void PluginManage(void)
	void PluginUnload(plugin_id_t id)
	void PluginUnloadAll(void)

	---

 */

#ifdef PLUGIN_SUPPORT

#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include "../include/string.h"
#include "../include/disk.h"

extern "C" {
#ifdef __linux__
# include <dlfcn.h>
#endif
#ifdef __SOLARIS__
# include <dlfcn.h>
#endif
}

#include "plugins.h"

#include "../include/unvmatch.h"
#include "../include/unvmath.h"
#include "../include/unvutil.h"
#include "swserv.h"
#include "net.h"	/* Tempory. */


plugin_record_struct **plugin = NULL;
int total_plugins = 0;


/*
 *	Contains plugin data referances, this structure only
 *	needs to be set up once per call to PluginLoad() but
 *	is used (passed to plugin functions) in PluginManage() and
 *	PluginUnload().
 */
static plugin_data_ref_struct plugin_data_ref;



/*
 *	Plugin wrapper functions.
 */

/* Create an object. */
int PluginWObjectCreate(xsw_object_struct ***ptr, int *total, int t)
{
	/* Let's create the object locally (built in). */
	int i, n;
	xsw_object_struct *obj_ptr;


	/* Search for available pointer. */
	for(i = 0; i < *total; i++)
	{
	    obj_ptr = (*ptr)[i];
	    if(obj_ptr == NULL)
		break;
	    if(obj_ptr->type <= XSW_OBJ_TYPE_GARBAGE)
		break;
	}
	if(i < *total)
	{
	    n = i;	/* Got available pointer. */
	}
	else
	{
	    /* Need to allocate more pointers. */
	    n = *total;
	    *total = *total + 1;

	    *ptr = (xsw_object_struct **)realloc(
		*ptr,
		*total * sizeof(xsw_object_struct *)
	    );
	    if(*ptr == NULL)
	    {
		*total = 0;
		return(-1);
	    }

	    (*ptr)[n] = NULL;
	}

	/* Allocate new object structure as needed. */
	if((*ptr)[n] == NULL)
	{
	    (*ptr)[n] = (xsw_object_struct *)calloc(
		1,
		sizeof(xsw_object_struct)
	    );
	    if((*ptr)[n] == NULL)
		return(-1);
	}

	obj_ptr = (*ptr)[n];

	UNVResetObject(obj_ptr);


	return(n);
}

/* Recycle an object. */
void PluginWObjectRecycle(xsw_object_struct ***ptr, int *total, int n)
{
	if(*ptr == xsw_object)
	{
	    DBRecycleObject(n);
	}
	else
	{
	    if(!UNVIsObjectGarbage(*ptr, *total, n))
		UNVResetObject((*ptr)[n]);
	}

	return;
}

/* Object in vector contact. */
int PluginWObjectInVectorRange(
	xsw_object_struct **ptr, int total,
	int o1, int o2,
	double range,		/* In Real units. */
	double bearing, double variance
)
{
	return(
	    Mu3DInVectorContact(
                ptr, total,
		o1, o2,
		bearing, variance,
		range * 1000	/* Convert to Screen units. */
	    )
	);
}

/* Sync object values with clients. */
void PluginWObjectSyncClients(xsw_object_struct **ptr, int total, int n)
{
	int i, w, con_obj_num;
	connection_struct **cptr, *con_ptr;
	xsw_object_struct *obj_ptr, *con_obj_ptr;


        if(ptr != xsw_object)
	    return;

	if(DBIsObjectGarbage(n))
	    return;
	else
	    obj_ptr = xsw_object[n];


	/* Go through each connection. */
	for(i = 0, cptr = connection;
            i < total_connections;
            i++, cptr++
	)
	{
	    con_ptr = *cptr;
	    if(con_ptr == NULL)
		continue;

	    con_obj_num = con_ptr->object_num;
	    if(DBIsObjectGarbage(con_obj_num))
		continue;
	    else
		con_obj_ptr = xsw_object[con_obj_num];

	    /* Object in range with connection object? */
	    if(!Mu3DInRangePtr(
		obj_ptr, con_obj_ptr,
		con_obj_ptr->scanner_range
	    ))
		continue;

	    NetSendObjectMaximums(i, n);
            NetSendObjectValues(i, n);
	    NetSendObjectName(i, n);

            for(w = 0; w < obj_ptr->total_weapons; w++)
		NetSendWeaponValues(i, n, w);
	}

        return;
}


/*
 *	Checks if the plugin ID is allocated and loaded.
 */
int PluginIsLoaded(plugin_id_t id)
{
	if((plugin == NULL) ||
           (id < 0) ||
           (id >= total_plugins)
	)
	    return(0);
	else if(plugin[id] == NULL)
	    return(0);
	else
	    return(1);
}

/*
 *	Gets pointer to the plugin record structure given the ID.
 *
 *	Can return NULL if the ID reffers to an unloaded plugin.
 */
plugin_record_struct *PluginGetPointerFromID(plugin_id_t id)
{
	if(PluginIsLoaded(id))
	    return(plugin[id]);
	else
	    return(NULL);
}

/*
 *	Matches a plugin record by its name, returns the id of
 *	that plugin record which can be considered valid or -1
 *	on no match.
 */
plugin_id_t PluginMatchByName(char *name)
{
	int i;
	plugin_record_struct *plugin_ptr;


	if(name == NULL)
	    return(-1);

	for(i = 0; i < total_plugins; i++)
	{
	    plugin_ptr = plugin[i];
	    if(plugin_ptr == NULL)
		continue;

	    /* Skip if library not loaded. */
	    if(plugin_ptr->handle == NULL)
                continue;

	    /* Skip if has no name. */
	    if(plugin_ptr->name == NULL)
		continue;

	    /* Skip if name dosen't match. */
	    if(strcasecmp(plugin_ptr->name, name))
		continue;

	    return(i);
	}

	return(-1);
} 


/*
 *	Sets up referance pointers and data for the given plugin
 *	data referance structure.
 */
void PluginSetupDataRef(plugin_data_ref_struct *in)
{
	in->cur_millitime = &cur_millitime;
	in->cur_systime = &cur_systime;
	in->lapsed_millitime = &lapsed_millitime;

	in->time_compensation = &time_compensation;

	in->xsw_object = &xsw_object;
	in->total_objects = &total_objects;

	in->connection = &connection;
	in->total_connections = &total_connections;

	in->con_notify_fptr = ConNotify;
	in->obj_is_garbage = UNVIsObjectGarbage;
	in->obj_create = PluginWObjectCreate;
	in->obj_recycle = PluginWObjectRecycle;
        in->obj_in_same_sector = Mu3DInSameSector;
        in->obj_in_contact = Mu3DInContact;
        in->obj_in_range = Mu3DInRange;
        in->obj_in_vector_range = PluginWObjectInVectorRange;
        in->obj_sync_clients = PluginWObjectSyncClients;

	return;
}


/*
 *	Loads the plugin, allocating a new plugin data structure
 *	and loading the actual plugin.
 *
 *	The given path must be absolute.
 *
 *	Return values are:
 *
 *	0 or greater	The allocated plugin index number/ID.
 *	-1		Invalid path or plugin not found.
 *	-2		Missing init, manage, or shutdown function.
 *	-3		Other error.
 *	-4		Init function called returned -1.
 */
plugin_id_t PluginLoad(
        char *path,
	u_int64_t flags,
        int argc, char *argv[],
        int condescriptor
)
{
	int i, status;
	char *strptr;
	plugin_id_t n;
	plugin_record_struct *plugin_ptr;

	int largc;
	char **largv;
	void *handle = NULL;

	int (*init_fptr)(int, char **, int, plugin_data_ref_struct *) = NULL;
	int (*manage_fptr)(plugin_data_ref_struct *) = NULL;
	void (*shutdown_fptr)(plugin_data_ref_struct *) = NULL;

	struct stat stat_buf;


	if(path == NULL)
	    return(-1);

	if(stat(path, &stat_buf))
	    return(-1);


	/* Set up plugin input data referances, technically this only
	 * needs to be set up once, but since we don't load plugins
	 * that often, we won't be wasting too much cpu.
	 */
	PluginSetupDataRef(&plugin_data_ref);


	/* Attempt to load plugin. */
#if defined(__linux__) || defined(__SOLARIS__)
	handle = dlopen(
	    path,
	    RTLD_LAZY |
	    RTLD_GLOBAL
	);
#endif
	if(handle == NULL)
	{
            if(sysparm.log_errors)
                LogAppendLineFormatted(
		    fname.primary_log,
                    "Could not load plugin:"
                );
            if(sysparm.log_errors)
                LogAppendLineFormatted(
                    fname.primary_log,
		    path
		);
#if defined(__linux__) || defined(__SOLARIS__)
	    strptr = dlerror();
            if(sysparm.log_errors)
                LogAppendLineFormatted(
                    fname.primary_log,
		    strptr
		);
#endif

	    return(-3);
	}

	/* Get and check if all required functions exist in
	 * plugin.
	 */
#if defined(__linux__) || defined(__SOLARIS__)
	// Dan S: Not sure if there is a less complicated way to typecast this under C++.
        init_fptr = ( int(*)(int, char **, int, plugin_data_ref_struct *) )dlsym(handle, SWPLUGIN_INIT_FUNCNAME);
	manage_fptr = ( int(*)(plugin_data_ref_struct *) )dlsym(handle, SWPLUGIN_MANAGE_FUNCNAME);
	shutdown_fptr = ( void(*)(plugin_data_ref_struct *) )dlsym(handle, SWPLUGIN_SHUTDOWN_FUNCNAME);
#endif
	if((init_fptr == NULL) ||
	   (manage_fptr == NULL) ||
	   (shutdown_fptr == NULL)
	)
	{
	    /* Could not resolve functions, so unload. */
#if defined(__linux__) || defined(__SOLARIS__)
	    dlclose(handle);
#endif

	    return(-2);
	}


	/* Copy arguments. */
	if(argc > 0)
	{
	    largc = argc;
	    largv = (char **)calloc(
		largc,
		sizeof(char *)
	    );
	    if(argv == NULL)
	    {
		largc = 0;
	    }
	    else
	    {
		for(i = 0; i < largc; i++)
		    largv[i] = StringCopyAlloc(argv[i]);
	    }
	}
	else
	{
	    largc = 0;
	    largv = NULL;
	}

        /* Call initialize function. */
	status = (*init_fptr)(
	    largc, largv,
	    condescriptor, &plugin_data_ref
	);
	if(status != 0)
	{
	    /* Init function reported error, so deallocate and close
	     * anything that we did.
	     */
#if defined(__linux__) || defined(__SOLARIS__)
	    dlclose(handle);
#endif	/* __linux__ */

	    StringFreeArray(largv, largc);

	    return(-4);
	}



	/* ******************************************************** */

	/* Sanitize total. */
	if(total_plugins < 0)
	    total_plugins = 0;


	/* Search for available pointer in array. */
	for(i = 0; i < total_plugins; i++)
	{
	    if(plugin[i] == NULL)
		break;
	}
	if(i < total_plugins)
	{
	    /* Got available pointer. */
	    n = i;
	}
	else
	{
	    /* Need to allocate more pointers. */
	    n = total_plugins;

	    total_plugins++;
	    plugin = (plugin_record_struct **)realloc(
		plugin,
		total_plugins * sizeof(plugin_record_struct *)
	    );
	    if(plugin == NULL)
	    {
		total_plugins = 0;

#if defined(__linux__) || defined(__SOLARIS__)
                dlclose(handle);
#endif
                StringFreeArray(largv, largc);

		return(-3);
	    }

	    /* Reset new pointer to NULL so it gets allocated below. */
	    plugin[n] = NULL;
	}

	/* Allocate structure as needed. */
	if(plugin[n] == NULL)
	{
	    plugin[n] = (plugin_record_struct *)calloc(
		1,
		sizeof(plugin_record_struct)
	    );
	    if(plugin[n] == NULL)
	    {
#if defined(__linux__) || defined(__SOLARIS__)
                dlclose(handle);
#endif  /* __linux__ */
                StringFreeArray(largv, largc);

		return(-3);
	    }
	}

	/* Get pointer to plugin. */
	plugin_ptr = plugin[n];


	/* Set values to plugin record structure. */
	plugin_ptr->name = StringCopyAlloc(path);

	plugin_ptr->argc = largc;
	plugin_ptr->argv = largv;

	plugin_ptr->handle = handle;
	plugin_ptr->flags = flags;

	plugin_ptr->init_fptr = init_fptr;
	plugin_ptr->manage_fptr = manage_fptr;
	plugin_ptr->shutdown_fptr = shutdown_fptr;


	return(n);
}


/*
 *	Manages all plugins, this function needs to be called
 *	once per loop.
 */
void PluginManage(void)
{
	int i, status;


	for(i = 0; i < total_plugins; i++)
	{
	    if(plugin[i] == NULL)
		continue;

	    /* Management function valid? */
	    if(plugin[i]->manage_fptr != NULL)
	    {
		/* Call management function. */
		status = (*plugin[i]->manage_fptr)(&plugin_data_ref);
		if(status != 0)
		{
		    /* Management function returned error,
		     * so unload this plugin.
		     */
		    PluginUnload(i);
		    continue;
		}
	    }
	}

	return;
}

/*
 *	Unloads the plugin and deallocates its record structure.
 */
void PluginUnload(plugin_id_t id)
{
	plugin_record_struct *plugin_ptr;


	plugin_ptr = PluginGetPointerFromID(id);
	if(plugin_ptr == NULL)
	    return;

	/* Shutdown library if loaded. */
	if(plugin_ptr->handle != NULL)
	{
	    /* Call shutdown function. */
	    if(plugin_ptr->shutdown_fptr != NULL)
		(*plugin_ptr->shutdown_fptr)(&plugin_data_ref);

#if defined(__linux__) || defined(__SOLARIS__)
	    /* Unload library. */
	    dlclose(plugin_ptr->handle);
#endif

	    /* Mark library as unloaded. */
	    plugin_ptr->handle = NULL;
	}

	/* Deallocate names and resources. */
	free(plugin_ptr->name);

	StringFreeArray(plugin_ptr->argv, plugin_ptr->argc);


	/* Deallocate structure itself and set pointer to it NULL. */
	free(plugin[id]);
	plugin[id] = NULL;


	return;
}

/*
 *	Unloads all plugins and deallocates their record structures.
 */
void PluginUnloadAll(void)
{
	int i;


	/* Unload and deallocate last to first. */
	for(i = total_plugins - 1; i >= 0; i--)
	    PluginUnload(i);

	free(plugin);
	plugin = NULL;

	total_plugins = 0;


	return;
}


#endif	/* PLUGIN_SUPPORT */
