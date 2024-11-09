#include "../include/unvmatch.h"
#include "../include/string.h"
#include "swserv.h"
#include "net.h"
 
  
/*
 *      Parses and handles a server command from a connection.
 */
int CmdHandleInput(int condescriptor, const char *cmd)
{
	int status, con_obj_num;
	char *strptr;
	xsw_object_struct *con_obj_ptr;
	connection_struct *con_ptr;

	char arg[CS_DATA_MAX_LEN];    
        char command[CS_DATA_MAX_LEN];
        char sndbuf[CS_DATA_MAX_LEN + 256];
	char text[CS_DATA_MAX_LEN + 256];


	if(cmd == NULL)
	    return(-1);

	/* Skip leading spaces in command. */
	while(ISBLANK(*cmd))
	    cmd++;

	if((*cmd) == '\0')
	    return(-1);


        /* Connection must valid, allocated and connected. */
        if(ConIsConnected(condescriptor))
	    con_ptr = connection[condescriptor];
	else
	    return(-1);

        /* Connection must also be logged in (have a valid object). */
        con_obj_num = con_ptr->object_num;
        if(DBIsObjectGarbage(con_obj_num))
        {
            NetSendLiveMessage(
		condescriptor,
                "You must be logged in to execute a server command."
            );
            return(-1); 
        }
	else
	{
	    con_obj_ptr = xsw_object[con_obj_num];
	}


	/* Begin parsing command. */
        strncpy(command, cmd, CS_DATA_MAX_LEN);
        command[CS_DATA_MAX_LEN - 1] = '\0';

	strptr = strchr(command, ' ');
        if(strptr == NULL)
	{
	    *arg = '\0';
	}
	else
        {
            strncpy(arg, strptr + 1, CS_DATA_MAX_LEN);   
            arg[CS_DATA_MAX_LEN - 1] = '\0';

            *strptr = '\0';
        }

	/* Strip spaces from command. */
	StringStripSpaces(command);

	/* Do not strip spaces from argument. */


        /* Log this command being typed. */
        sprintf(
	    text,
            "%s: Executed: Command: `%s'  Argument: `%s'",
            DBGetFormalNameStr(con_obj_num),
            command,
            arg
        );
        if(sysparm.log_general)
            LogAppendLineFormatted(fname.primary_log, text);


        /* ****************************************************** */
        /* Handle command. */

        /* Debug. */
        if(strcasepfx(command, "debug"))
        {
	    status = CmdDebug(condescriptor, arg);
        }
	/* Help. */
        else if(strcasepfx(command, "h") ||
                strcasepfx(command, "?")
	)
        {
            status = CmdHelp(condescriptor, arg);
        }
        /* Version. */ 
        else if(strcasepfx(command, "ver"))
        {
            status = CmdVersion(condescriptor, arg);
        } 
        /* Sysparm. */
        else if(strcasepfx(command, "sysp") ||
                strcasepfx(command, "tune")
        )
        {
            status = CmdSysparm(condescriptor, arg);
        }
        /* Link. */
        else if(strcasepfx(command, "li"))
        {
            status = CmdLink(condescriptor, arg);
        }
        /* Memory. */
        else if(strcasepfx(command, "mem"))
        {
            status = CmdMemory(condescriptor, arg);
        }
        /* Disk. */
        else if(strcasepfx(command, "disk"))
        {
            status = CmdDisk(condescriptor, arg);
        }
        /* Processes. */
        else if(strcasepfx(command, "ps")) 
        {
            status = CmdPS(condescriptor, arg);
        }
#ifdef PLUGIN_SUPPORT
	/* Plugin. */
	else if(strcasepfx(command, "plugin"))
	{
	    status = CmdPlugin(condescriptor, arg); 
	}
#endif	/* PLUGIN_SUPPORT */
        /* Kill process. */
        else if(strcasepfx(command, "kill"))
        {
            status = CmdKill(condescriptor, arg);
        }
        /* Set. */
        else if(strcasepfx(command, "set"))
        {
            status = CmdSet(condescriptor, arg);
        }
	/* Wall. */
        else if(strcasepfx(command, "wal"))
        {
            status = CmdWall(condescriptor, arg);
        }
        /* Who. */
        else if(strcasepfx(command, "who"))
        {
            status = CmdWho(condescriptor, arg);
        }
        /* Netstat. */
        else if(strcasepfx(command, "nets"))
        {
            status = CmdNetstat(condescriptor, arg);
        }
        /* Score. */
        else if(strcasepfx(command, "score"))
        {
            status = CmdScore(condescriptor, arg);
        }
        /* ETA. */
        else if(strcasepfx(command, "eta"))
        {
            status = CmdETA(condescriptor, arg);
        }
        /* Save. */
        else if(strcasepfx(command, "save")) 
        {
            status = CmdSaveUniverse(condescriptor, arg);
        }
        /* Shutdown. */
        else if(strcasepfx(command, "shutdown"))
        {
            status = CmdShutdown(condescriptor, arg);
        }
        /* Shutdown. */
        else if(strcasepfx(command, "siteb"))
        {
            status = CmdSiteBan(condescriptor, arg);
	}
        /* Boot. */
        else if(strcasepfx(command, "boot"))
        {
            status = CmdBoot(condescriptor, arg);
        }
        /* Sync time. */
        else if(strcasepfx(command, "sync"))
        {
            status = CmdSyncTime(condescriptor, arg);
        }
        /* ID. */
        else if(strcasepfx(command, "id"))
        {
            status = CmdID(condescriptor, arg);
        }
        /* Create player. */
        else if(strcasepfx(command, "createp"))
        {
            status = CmdCreatePlayer(condescriptor, arg);
        }
        /* Create object. */
        else if(strcasepfx(command, "create"))
        {
            status = CmdCreate(condescriptor, arg);
        }
        /* Chown object. */
        else if(strcasepfx(command, "cho"))
        {
            status = CmdChown(condescriptor, arg);
        }
        /* Recycle player. */
        else if(strcasepfx(command, "recyclep"))
        {
            status = CmdRecyclePlayer(condescriptor, arg);
        }
        /* Recycle object. */
        else if(strcasepfx(command, "rec"))
        {
            status = CmdRecycle(condescriptor, arg);
        }
        /* Unrecycle object. */
        else if(strcasepfx(command, "unrec"))
        {
           status = CmdUnrecycle(condescriptor, arg);
        }
        /* Examine object. */
        else if(strcasepfx(command, "ex"))
        {
            status = CmdExamine(condescriptor, arg);
        }
        /* Find. */
        else if(strcasepfx(command, "find"))
        {
            status = CmdFind(condescriptor, arg);
        }
        /* Eco product create. */
        else if(strcasepfx(command, "ecoproductc") ||
                strcasepfx(command, "ecoprodc")
        )
        {
            status = CmdEcoProductCreate(condescriptor, arg);
        }
        /* Eco product create. */
        else if(strcasepfx(command, "ecoproducts") ||
                strcasepfx(command, "ecoprods")
        )
        {
            status = CmdEcoProductSet(condescriptor, arg);
        }
	/* Eco product delete. */
        else if(strcasepfx(command, "ecoproductd") ||
                strcasepfx(command, "ecoprodd")
        )
        {
            status = CmdEcoProductDelete(condescriptor, arg);
        }


        /* Test. */
        else if(!strcasecmp(command, "test")) 
        {
            status = CmdTest(condescriptor, arg);
        }

        /* Unknown command. */
        else
        {
            status = -1;

            sprintf(sndbuf,
                "%s: no such command or macro.",
                command
            );
            NetSendLiveMessage(condescriptor, sndbuf);
        }


        return(status);
}
