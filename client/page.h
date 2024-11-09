/*

                            Page definations

	This file is #included by page.c and pagefile.c.

	The page structure and label structures are in xsw.h

 */

#ifndef PAGE_H
#define PAGE_H

#include "../include/osw-x.h"	/* For image_t */


#define DRAW_AMOUNT_COMPLETE    0


/*
 *      Background image draw rules:
 */
#define PAGE_BKG_DRAW_TOFIT	0
#define PAGE_BKG_DRAW_TILED	1

/* 
 *      Label states:
 *       
 *      Highest must be one less than PAGE_LABELS_PER_LABEL.
 */
#define PAGE_LABEL_STATE_UNARMED	0
#define PAGE_LABEL_STATE_ARMED		1
#define PAGE_LABEL_STATE_HIGHLIGHTED	2


#endif /* PAGE_H */
