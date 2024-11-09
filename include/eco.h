/*
                             Economy

	These definations pertain to economy specific to the
	ShipWars server.

	See `objects.h' for economy unit type and structure
	definations.

 */

#ifndef ECO_H
#define ECO_H


/*
 *	Economy product names:
 *
 *	(Case is not sensitive in matching)
 */
#define ECO_PROD_NAME_ANTIMATTER	"Antimatter"
#define ECO_PROD_NAME_REPAIRHULL	"Hull Repair"
#define ECO_PROD_NAME_RMU		"Raw Material"

#define ECO_PROD_NAME_OPMPFX		"$OPM"


/*
 *      Economy flags:
 */
#define ECO_FLAG_OPEN           (1 << 0)        /* Opened for business
                                                 * (should always be true).
                                                 */
#define ECO_FLAG_BUY_OK         (1 << 1)        /* Can buy from me. */
#define ECO_FLAG_SELL_OK        (1 << 2)        /* Can sell to me. */
#define ECO_FLAG_TRADE_OK       (1 << 3)        /* Can trade with me. */
#define ECO_FLAG_INTRODUCE_OK   (1 << 4)        /* Create from thin air. */

#define ECO_FLAG_NAME_OPEN              "OPEN"
#define ECO_FLAG_NAME_BUY_OK            "BUY_OK"
#define ECO_FLAG_NAME_SELL_OK           "SELL_OK"
#define ECO_FLAG_NAME_TRADE_OK          "TRADE_OK"
#define ECO_FLAG_NAME_INTRODUCE_OK      "INTRODUCE_OK"

/* 
 *      Economy maximums:
 */                                              
#define ECO_PRODUCTS_MAX        32
#define ECO_PRODUCT_NAME_MAX    81


/*
 *	Maximum transaction range (in XSW Real units):
 */
#define ECO_MAX_TRANSACTION_RANGE	1.0


/*
 *      Economy flags mask type:
 */
typedef unsigned long eco_flags_t;




#endif /* ECO_H */
