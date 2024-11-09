#include "../include/unvmatch.h"
#include "../include/swnetcodes.h"
#include "swserv.h"
#include "net.h"



int NetHandleReqObjectSect(int condescriptor, char *arg)
{
	int object_num;


        /*
         *      SWEXTCMD_REQSECT format:
         *
         *      object_num
         */
        sscanf(arg, "%i",
                &object_num
        );


        if(!DBIsObjectGarbage(object_num))
        {
            NetSendObjectSect(condescriptor, object_num);
        }


        return(0);
}


int NetHandleObjectSect(int condescriptor, char *arg)
{
        int object_num, con_object_num;  
/*	long sect_dx, sect_dy, sect_dz; */
        long sect_x, sect_y, sect_z;
        xsw_object_struct *obj_ptr;


        /* Get object_num. */
        con_object_num = connection[condescriptor]->object_num;

            
        /*
         *   SWEXTCMD_SETOBJSECT format:
         *
         *      object_num
         *      sect_x sect_y sect_z
         */
        sscanf(arg, "%i %ld %ld %ld",
                &object_num,

                &sect_x,
                &sect_y,
                &sect_z
        );

        if(DBIsObjectGarbage(object_num))
            return(-1);
	else
            obj_ptr = xsw_object[object_num];


        /* Connection must own object. */
	if(con_object_num != object_num)
	    return(-3);

	/* Change the object's sector and update related resources. */
	DBObjectDoSetSector(
	    object_num,
	    sect_x, sect_y, sect_z,
	    1			/* Allow wrapping. */
	);

        return(0);
}       



int NetSendObjectSect(int condescriptor, int object_num)
{
        char sndbuf[CS_DATA_MAX_LEN];
        xsw_object_struct *obj_ptr;


        if(DBIsObjectGarbage(object_num))
            return(-1);
        else
            obj_ptr = xsw_object[object_num];


        /*
         *   SWEXTCMD_SETOBJSECT format:
         *      
         *      object_num
         *      sect_x sect_y sect_z
         */
        sprintf(sndbuf,
"%i %i\
 %i %ld %ld %ld\n",
                CS_CODE_EXT,
                SWEXTCMD_SETOBJSECT,

                object_num,
                obj_ptr->sect_x,
                obj_ptr->sect_y,
                obj_ptr->sect_z
        );
        NetDoSend(condescriptor, sndbuf);


        return(0);
}


int NetSendFObjectSect(int condescriptor, int object_num)
{
        char sndbuf[CS_DATA_MAX_LEN];
        xsw_object_struct *obj_ptr;


        if(DBIsObjectGarbage(object_num))
            return(-1);
        else
            obj_ptr = xsw_object[object_num];
           
          
        /*
         *      SWEXTCMD_SETFOBJSECT format:
         *      
         *      object_num
         *      sect_x sect_y sect_z
         */
        sprintf(sndbuf,
"%i %i\
 %i %ld %ld %ld\n",
                CS_CODE_EXT,
                SWEXTCMD_SETFOBJSECT,

                object_num,
                obj_ptr->sect_x,
                obj_ptr->sect_y,
                obj_ptr->sect_z
        );
        NetDoSend(condescriptor, sndbuf);


        return(0);
}
