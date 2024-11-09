#include "swserv.h"
#include "net.h"


int CmdHelp(int condescriptor, const char *arg)
{
        char sndbuf[CS_DATA_MAX_LEN];


        sprintf(sndbuf,
 "boot  create  createp  chown  disk  ecoprodcreate  ecoproddelete  ecoprodset  eta"
        );
        NetSendLiveMessage(condescriptor, sndbuf);

        sprintf(sndbuf,
 "examine  find  help  id  kill  link memory  netstat  plugin  ps  recycle  recycleplayer"
        );
        NetSendLiveMessage(condescriptor, sndbuf);

        sprintf(sndbuf,
 "save  score  set  shutdown  siteban  synctime  tune  unrecycle  wall  who"
        );
        NetSendLiveMessage(condescriptor, sndbuf);


        return(0);
}
