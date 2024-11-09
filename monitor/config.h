#ifndef CONFIG_H
#define CONFIG_H



/*
 *	Default address:
 */
#define DEF_ADDRESS	"127.0.0.1"

/*
 *	Default AUX Stats port number:
 */
#define DEF_PORT	1702

/*
 *	Default login name and password:
 */
#define DEF_LOGIN_NAME		"Guest"
#define DEF_LOGIN_PASSWORD	"guest"

/*
 *	Default path containing the images:
 */
#define DEF_IMAGES_DIR		"/usr/share/games/xshipwars/images/monitor"

/*
 *	Default toplevel path of the server:
 */
#define DEF_SERVER_DIR		"/home/swserv"


/*
 *	Image file names:
 *
 *	Note: DEF_IMAGES_DIR will be prepended.
 */
#define IMG_NAME_READOUT_BKG	"readout_bkg.tga"
#define IMG_NAME_MESG_BKG	"mesg_bkg.tga"
#define IMG_NAME_BTN0		"btn0.tga"
#define IMG_NAME_BTN1		"btn1.tga"
#define IMG_NAME_BTN2		"btn2.tga"
#define IMG_NAME_MONITOR_ICON	"icon.tga"


/*
 *	AUX pipe recieve check interval (in milliseconds):
 */
#define PIPE_RECV_INT		0

/*
 *	Systems check interval (in milliseconds):
 */
#define SYSTEMS_CHECK_INT	1000


/*
 *	Interval between reconnect retries (in milliseconds):
 */
#define CONNECT_CHECK_INT	5000


/*
 *	Title string when universe is not reachable:
 */
#define NOT_REACHABLE_TITLE_STRING	"Not Connected"



#endif	/* CONFIG_H */
