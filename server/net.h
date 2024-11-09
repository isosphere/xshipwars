/*
                 ShipWars Network Parsing and Sending
 */

#ifndef NET_H
#define NET_H


extern int NetIsSocketWritable(int s, int *error_level);
extern int NetIsSocketReadable(int s, int *error_level);
extern int NetConGuests(void);

extern int NetManageNewConnections(int socket);
extern void NetCloseConnection(int condescriptor);

extern int NetSendDataToConnection(
	int condescriptor,
	char *data,
	int priority
);
extern void NetDoSend(int condescriptor, char *sndbuf);
extern int NetManageSend(void);

extern int NetHandleExtCmd(int condescriptor, char *arg);
extern int NetManageRecvConnection(int condescriptor);
extern int NetManageRecv(void);


extern int NetHandleSetCloak(int condescriptor, char *arg);
extern int NetSendSetCloak(int condescriptor);

extern int NetHandleSetChannel(int condescriptor, char *arg);
extern int NetSendSetChannel(int condescriptor,
        int object_num, int channel
);

extern int NetHandleComMesg(int condescriptor, char *arg);
extern int NetSendComMesg(
        int condescriptor,
        int src_obj, int tar_obj,
        double bearing, int channel,
        char *message
);

extern int NetHandleCreateObject(int condescriptor, char *arg);
extern int NetSendCreateObject(int condescriptor, int object_num);

extern int NetHandleLiveMessage(int condescriptor, char *mesg);
extern int NetSendLiveMessage(int condescriptor, char *mesg);
extern int ConNotify(int condescriptor, char *mesg);

extern int NetHandleSysMessage(int condescriptor);
extern int NetSendSysMessage(
        int condescriptor,
        int code,               /* One of CS_SYSMESG_* */
        char *mesg
);

extern int NetHandleSetDmgCtl(int condescriptor, char *arg);
extern int NetSendSetDmgCtl(int condescriptor);

extern int NetHandleReqValues(int condescriptor, char *arg);
extern int NetSendEcoSetValues(int condescriptor, int object_num);
extern int NetSendEcoSetProductValues(
	int condescriptor,
	int object_num,
	int product_num
);
extern int NetHandleEcoBuy(int condescriptor, char *arg);
extern int NetHandleEcoSell(int condescriptor, char *arg);

extern int NetHandleWormHoleEnter(int condescriptor, char *arg);

extern int NetHandleSetImageSet(int condescriptor, char *arg);
extern int NetHandleSetSoundSet(int condescriptor, char *arg);
extern int NetHandleSetOCSN(int condescriptor, char *arg);
extern int NetSendSetImageSet(int condescriptor, char *filename);
extern int NetSendSetSoundSet(int condescriptor, char *filename);
extern int NetSendSetOCSN(int condescriptor, char *filename);

extern int NetHandleSetEngine(int condescriptor, char *arg);
extern int NetSendSetEngine(int condescriptor, int object_num);

extern int NetHandleFireWeapon(int condescriptor, char *arg);
extern int NetSendFireWeapon(int condescriptor);

extern int NetHandleHail(int condescriptor, char *arg);
extern int NetSendHail(
        int condescriptor, 
        int src_obj, int tar_obj,
        double bearing, int channel   
);

extern int NetHandleSetInterval(int condescriptor, char *arg);
extern int NetSendSetInterval(int condescriptor);

extern int NetHandleSetIntercept(int condescriptor, char *arg);
extern int NetSendSetIntercept(int condescriptor);

extern int NetHandleLighting(int condescriptor, char *arg);
extern int NetSendLighting(int condescriptor);

extern int NetHandleLogin(int condescriptor, char *arg);
extern int NetSendLogin(int condescriptor);

extern int NetHandleLogout(int condescriptor);
extern int NetSendLogout(int condescriptor);

extern int NetHandleNotifyDestroy(int condescriptor, char *arg);
extern int NetSendNotifyDestroy(
        int condescriptor,
        int reason,
	int destroyed_obj,
	int destroyer_obj,
	int destroyer_obj_owner
);

extern int NetHandleNotifyHit(int condescriptor, char *arg);
extern int NetSendNotifyHit(
        int condescriptor,
        int wep_obj, int tar_obj,
        double delta_damage,
        double bearing,         /* tar_obj relative, to wep_obj. */
        double structure_damage,
        double shield_damage
);

extern int NetHandleObjectForcePose(int condescriptor, char *arg);
extern int NetSendObjectForcePose(int condescriptor, int object_num);

extern int NetHandleObjectMaximums(int condescriptor, char *arg);
extern int NetSendObjectMaximums(int condescriptor, int object_num);

extern int NetHandleObjectName(int condescriptor, char *arg);
extern int NetSendObjectName(int condescriptor, int object_num);

extern int NetHandleObjectPose(int condescriptor, char *arg);
extern int NetSendObjectPose(int condescriptor, int object_num);

extern int NetHandleReqObjectSect(int condescriptor, char *arg);
extern int NetHandleObjectSect(int condescriptor, char *arg);
extern int NetSendObjectSect(int condescriptor, int object_num);
extern int NetSendFObjectSect(int condescriptor, int object_num);

extern int NetHandleObjectValues(int condescriptor, char *arg);
extern int NetSendObjectValues(int condescriptor, int object_num);

extern int NetHandleRecycleObject(int condescriptor, char *arg);
extern int NetSendRecycleObject(int condescriptor, int object_num);

extern int NetHandleRefresh(int condescriptor, char *arg);
extern int NetSendRefresh(int condescriptor);

extern int NetHandleScore(int condescriptor, char *arg);
extern int NetSendScore(int condescriptor, int object_num);

extern int NetHandleSetShields(int condescriptor, char *arg);
extern int NetSendSetShields(int condescriptor, int object_num);

extern int NetHandleShieldVis(int condescriptor, char *arg);
extern int NetSendShieldVis(int condescriptor, int object_num);

extern int NetHandlePlaySound(int condescriptor, char *arg);
extern int NetSendPlaySound(
	int condescriptor, int sound_code,
        double left_volume, double right_volume
);

extern int NetSendStarChartAddObject(
        int condescriptor,
        int object_num
);
extern int NetSendStarChartSetObjectName(
        int condescriptor,
        int object_num
);
extern int NetSendStarChartSetObjectEmpire(
        int condescriptor,
        int object_num
);
extern int NetSendStarChartRecycleObject(  
        int condescriptor,
        int object_num
);
extern int NetSendEntireStarChart(int condescriptor);

extern int NetHandleObjectThrottle(int condescriptor, char *arg);
extern int NetSendObjectThrottle(int condescriptor);

extern int NetHandleTractorBeamLock(int condescriptor, char *arg);
extern int NetSendTractorBeamLock(
	int condescriptor,
	long src_obj,
	long tar_obj
);

extern int NetHandleSetUnits(int condescriptor, char *arg);
extern int NetSendUnits(int condescriptor);

extern int NetHandleWeaponDisarm(int condescriptor, char *arg);
extern int NetSendWeaponDisarm(int condescriptor, int src_obj, int tar_obj);

extern int NetHandleSetWeaponsLock(int condescriptor, char *arg);
extern int NetSendSetWeaponsLock(int condescriptor);

extern int NetHandleSelectWeapon(int condescriptor, char *arg);
extern int NetSendSelectWeapon(int condescriptor);

extern int NetHandleSetWeaponsUnlock(int condescriptor, char *arg);
extern int NetSendSetWeaponsUnlock(int condescriptor);

extern int NetHandleWeaponValues(int condescriptor, char *arg);
extern int NetSendWeaponValues(
	int condescriptor,
	int object_num,
	int weapon_num
);

extern int NetHandleWhoAmI(int condescriptor, char *arg);
extern int NetSendWhoAmI(int condescriptor);

extern int NetHandleWormHoleEnter(int condescriptor, char *arg);


#endif	/* NET_H */
