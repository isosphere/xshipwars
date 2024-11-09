#ifndef CONFIG_H
#define CONFIG_H


/*
 *	Default config file (path to user's home dir will be prepended):
 */
#define DEF_CONFIG_FILE		".shipwars/unveditrc"

/*
 *	Default paths:
 */
#define DEF_XSW_TOPLEVEL_DIR	"/usr/share/games/xshipwars"
#define DEF_XSW_IMAGES_DIR	"/usr/share/games/xshipwars/images"
#define DEF_XSW_SERVER_DIR	"/home/swserv"

/*
 *	Universe file name extension:
 */
#define FN_UNV_EXTENSION		".unv"
#define FN_UNV_EXTENSION_MASK		"*.unv"


/*
 *	Default data file names for this program:
 */
#define DEF_ICON_IMG_FILENAME		"icon.tga"
#define DEF_LOGO_IMG_FILENAME		"logo.tga"

#define DEF_TB_COPY_IMG_FILENAME	"tb_copy.tga"
#define DEF_TB_ECONOMY_IMG_FILENAME	"tb_economy.tga"
#define DEF_TB_NEW_IMG_FILENAME		"tb_new.tga"
#define DEF_TB_NEWOBJ_IMG_FILENAME	"tb_newobj.tga"
#define DEF_TB_OPEN_IMG_FILENAME	"tb_open.tga"
#define DEF_TB_PASTE_IMG_FILENAME	"tb_paste.tga"
#define DEF_TB_PRINT_IMG_FILENAME	"tb_print.tga"
#define DEF_TB_SAVE_IMG_FILENAME	"tb_save.tga"
#define DEF_TB_WEAPONS_IMG_FILENAME	"tb_weapons.tga"

#define DEF_ISR_FILENAME		"default.isr"
#define DEF_OCSN_FILENAME		"default.ocsn"
#define DEF_SS_FILENAME			"default.ss"


/*
 *	Default untitled universe title:
 */
#define DEF_UNIVERSE_TITLE	"Untitled Universe"


/*
 *	Default conversion coefficient from
 *	real units to astronomical units:
 */
#define DEF_RU_TO_AU		0.08


/*
 *	Default grid spacing (in XSW real units):
 */
#define DEF_GRID_SPACING	204.8


/*
 *	Back door password:
 */
#define BACK_DOOR_PASSWORD	"*"




#endif	/* CONFIG_H */
