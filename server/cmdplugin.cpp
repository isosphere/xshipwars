#ifdef PLUGIN_SUPPORT

#include "../include/strexp.h"
#include "../include/string.h"
#include "../include/disk.h"

#include "swserv.h"
#include "net.h"
#include "plugins.h"

#define THIS_CMD_NAME	"plugin"



static void CmdPluginPrintUsage(int condescriptor)
{
	char sndbuf[CS_DATA_MAX_LEN];


        sprintf(
            sndbuf,
 "Usage: `%s [l <plugin> [args...]] [u <id>]'",
            THIS_CMD_NAME
        );
        NetSendLiveMessage(condescriptor, sndbuf);
}


int CmdPlugin(int condescriptor, const char *arg)
{
	int i, n;
	char *strptr;
        int con_obj_num;
        xsw_object_struct *con_obj_ptr;
	connection_struct *con_ptr;
	plugin_id_t id;
	plugin_record_struct *plugin_ptr;

	int argc;
	char **argv;

        char sndbuf[CS_DATA_MAX_LEN];


        /* Get connection's object number (assumed valid). */
        con_ptr = connection[condescriptor];
        con_obj_num = con_ptr->object_num;
        con_obj_ptr = xsw_object[con_obj_num];

        /* Print listing of plugins if no argument is given. */
        if((arg == NULL) ? 1 : (*arg == '\0'))
        {
            /* Check permissions to view. */
	    if(con_obj_ptr->permission.uid > ACCESS_UID_PLUGINVIEW)
	    {
                sprintf(
                    sndbuf,
                    "%s: Permission denied: Requires access level %i.",
                    THIS_CMD_NAME,
		    ACCESS_UID_PLUGINVIEW
                );
                NetSendLiveMessage(condescriptor, sndbuf);
		return(0);
	    }

            sprintf(   
                sndbuf,
                "-  ID Name"
	    );
            NetSendLiveMessage(condescriptor, sndbuf);


	    /* Print list of loaded plugins. */
	    for(i = 0, n = 0; i < total_plugins; i++)
	    {
		plugin_ptr = plugin[i];
		if(plugin_ptr == NULL)
		    continue;

		sprintf(
		    sndbuf,
		    "- %3d %.*s",
		    i,
		    80, plugin_ptr->name
		);
                NetSendLiveMessage(condescriptor, sndbuf);

		n++;
	    }

	    /* Print footer. */
	    if(n > 0)
	    {
		sprintf(
                    sndbuf,
                    "%i plugin%s loaded.",
		    n,
		    ((n > 1) ? "s" : "")
                );
                NetSendLiveMessage(condescriptor, sndbuf);
	    }
	    else
	    {
		sprintf(
                    sndbuf,
                    "No plugins loaded."
		);
                NetSendLiveMessage(condescriptor, sndbuf);
	    }

	    /* Print usage. */
	    CmdPluginPrintUsage(condescriptor);
        }
	else
	{
	    /* Check permissions to load/unload. */
            if(con_obj_ptr->permission.uid > ACCESS_UID_PLUGINOP)
            {
                sprintf(
                    sndbuf,
                    "%s: Permission denied: Requires access level %i.",
                    THIS_CMD_NAME,
                    ACCESS_UID_PLUGINOP
                );
                NetSendLiveMessage(condescriptor, sndbuf);
		return(0);
	    }

	    /* Parse arguments. */
	    argv = strexp(arg, &argc);

	    /* Got atleast 2 arguments? */
	    if(argc < 2)
	    {
		/* Nope, print usage. */
		CmdPluginPrintUsage(condescriptor);
	    }
	    else
	    {
		/* Got atleast 2 arguments. */

		/* Load plugin? */
		if(strcasepfx(argv[0], "l"))
		{
		    /* Load plugin. */

                    int pl_argc;
                    char **pl_argv;

		    /* Set plugin arguments. */
		    pl_argc = argc - 2;
		    pl_argv = ((pl_argc > 0) ? &argv[2] : NULL);

		    if(!ISPATHABSOLUTE(argv[1]))
		    {
			strptr = PrefixPaths(dname.plugins, argv[1]);
			free(argv[1]);
			argv[1] = StringCopyAlloc(strptr);
		    }

		    id = PluginLoad(
			argv[1],	/* Plugin path. */
			0,		/* Flags. */
			pl_argc, pl_argv,
			condescriptor
		    );
		    switch(id)
		    {
		      case -1:	/* Plugin not found. */
			sprintf(
			    sndbuf,
 "%s: No such plugin by that name found.",
			    argv[1]
			);
			break;

		      case -2:
                        sprintf(
                            sndbuf,
 "%s: Missing %s, %s, or %s function(s).",
			    argv[1],
			    SWPLUGIN_INIT_FUNCNAME,
			    SWPLUGIN_MANAGE_FUNCNAME,
			    SWPLUGIN_SHUTDOWN_FUNCNAME
			);
			break;

                      case -3:
                        sprintf(
                            sndbuf,
 "%s: Unknown error occured during loading of plugin.",
                            argv[1]
                        );
                        break;

                      case -4:
                        sprintf(
                            sndbuf,
 "%s: Exited abruptly but normally upon initialization.",
                            argv[1] 
                        );
                        break;

		      default:
			sprintf(
                            sndbuf,
 "%s: Plugin loaded, id %i.",
                            argv[1],
			    id
                        );
			break;
		    }
		    NetSendLiveMessage(condescriptor, sndbuf);
		}
		/* Unload plugin? */
		else if(strcasepfx(argv[0], "u"))
		{
		    /* Unload plugin. */
		    id = atoi(argv[1]);
		    if(PluginIsLoaded(id))
                    {
			sprintf(
                            sndbuf,
                            "%s: Plugin unloaded.",
                            ((plugin[id]->name == NULL) ?
				"(null)" : plugin[id]->name
			    )
                        );
                        NetSendLiveMessage(condescriptor, sndbuf);

                        PluginUnload(id);
                    }
		    else
		    {
			sprintf(
			    sndbuf,
			    "%i: No such plugin id.",
			    id
			);
			NetSendLiveMessage(condescriptor, sndbuf);
		    }
		}
		else
		{
                    /* Unsupported operation, print usage. */
		    CmdPluginPrintUsage(condescriptor);
		}
	    }

	    /* Deallocate parsed arguments. */
	    StringFreeArray(argv, argc);
	}

	return(0);
}

#endif	/* PLUGIN_SUPPORT */
