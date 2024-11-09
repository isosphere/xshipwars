/*
                    ShipWars Network Parsing and Sending
 */

#ifndef NET_H
#define NET_H

#include "../include/urlparse.h"
#include "../include/cs.h"
#include "../include/objects.h"


/*
 *	Connection state codes:
 */
#define CON_STATE_NOT_CONNECTED         0       /* Not connected. */
#define CON_STATE_NEGOTIATING           1       /* Logging in. */
#define CON_STATE_CONNECTED             2       /* Logged in. */


/*
 *	Network parameters:
 */
typedef struct {

        /* Connection state, one of CON_STATE_*. */
	int connection_state;

        /* Socket to server, -1 implies no socket and not connected. */
        int socket;

        /* Referances to player object, can be NULL. */
        int player_obj_num;
        xsw_object_struct *player_obj_ptr;

        /* Login name and password. */
        char login_name[XSW_OBJ_NAME_MAX];
        char login_password[XSW_OBJ_PASSWORD_MAX];

        /*   Address set level:
         *
         *   0 = not set.
         *   1 = set in config file.
         *   2 = set at command line.
         */
        int is_address_set;

        /* When connection was first established in systime seconds. */
        time_t con_start;

        /* Login delay warning records. */
        char    warn15,
                warn30,
                warn45;

        /* Login information reception check list. */
        char    login_got_lplayer,
                login_got_position,
                login_got_sector;

        /* Address and port number of connection to server. */
        char address[MAX_URL_LEN];
        int port;  

        /*   Network stream sync refresh interval, in milliseconds.
         *   Interval in which client data (such as local player
         *   object positions) are sent to the server.
         */
        long net_int;

        /* Times disconnect was sent. */
        int disconnect_send_count;

        /* Bad send() counter. */
        int bad_send_count;

        /*   Carry over data, data that is left over from
         *   one recv() without a segment delimiter character
         *   that is then put at the beginning of the next recv().
         */
        char co_data[CS_DATA_MAX_LEN];
        int co_data_len; 

} xsw_net_parms_struct;
extern xsw_net_parms_struct net_parms;



/* In net.c */
extern int NetOpenConnection(char *host, int port);
extern void NetResetParms(void);


/* In netrecv.c */ 
extern int NetHandleRecv(void);


/* In netsend.c */
extern int NetSendData(char *outbounddata);
extern int NetSendGlobalMessage(char *message);
extern int NetSendDisconnect(void);
extern int NetSendRefresh(void);
extern int NetSendSetInterval(void);
extern int NetSendExec(char *arg);
extern int NetSendSetImageSet(char *arg);
extern int NetSendSetSoundSet(char *arg);
extern int NetSendSetOCSN(char *arg);
extern int NetSendPoseObj(int object_num);
extern int NetSendObjectSect(int object_num);
extern int NetSendObjectThrottle(int object_num);
extern int NetSendObjectValues(int object_num);
extern int NetSendSetLighting(int object_num, xswo_lighting_t lighting);
extern int NetSendSetChannel(int object_num, int channel);
extern int NetSendWeaponsState(int object_num, int state);
extern int NetSendSelectWeapon(int object_num, int selected_weapon);
extern int NetSendSetShields(
	int object_num, int shield_state, double shield_frequency
);
extern int NetSendSetDmgCtl(int object_num, char damage_control);
extern int NetSendSetCloak(int object_num, char cloak_state);
extern int NetSendWeaponsLock(int object_num, int tar_object_num);
extern int NetSendWeaponsUnlock(int object_num);
extern int NetSendIntercept(int object_num, const char *arg);
extern int NetSendSetEngine(int object_num, int engine_state);
extern int NetSendReqName(int object_num);
extern int NetSendFireWeapon(int object_num, double freq);
extern int NetSendTractorBeamLock(int src_obj, int tar_obj);
extern int NetSendHail(int src_obj, int tar_obj, int channel);
extern int NetSendComMessage(
	int src_obj, int tar_obj,
	int channel, char *message
);
extern int NetSendWormHoleEnter(int src_obj, int tar_obj);
extern int NetSendELinkEnter(int src_obj, int tar_obj);
extern int NetSendWeaponDisarm(int src_obj, int tar_obj);

extern int NetSendEcoReqValues(int src_obj, int tar_obj);
extern int NetSendEcoBuy(
        int customer_obj,
        int proprietor_obj,
        xsw_ecoproduct_struct product
);
extern int NetSendEcoSell(
        int customer_obj,
        int proprietor_obj,
        xsw_ecoproduct_struct product
);

#endif /* NET_H */
