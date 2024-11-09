/*
                          Printer Wrapper
 */

#ifndef PRINTERW_H
#define PRINTERW_H

#include <sys/types.h>

#include "../include/os.h"
#include "../include/osw-x.h"   /* Need to know about image_t. */


/*
 *	Flag type:
 */
#ifndef printer_flag_t
# define printer_flag_t		unsigned long
#endif

/*
 *	Error codes:
 */
#define PrinterSuccess			0
#define PrinterError			1	/* General error. */
#define PrinterNoBuffers		2
#define PrinterBadValue			3

/*
 *	Color modes:
 */
#define PrinterColorModeBlackAndWhite	0
#define PrinterColorModeGreyScale	1
#define PrinterColorModeColor		2


/*
 *	Units:
 */
#define PrinterUnitPixels		0
#define PrinterUnitInches		1
#define PrinterUnitCentimeters		2


/*
 *	Print destination.
 */
#define PrinterDestinationPrinter	0
#define PrinterDestinationFile		1


/*
 *	Default initializer command.
 *
 *	Multiple commands can be specified by delimiting the
 *	string with ';' characters.
 *
 *	ie: "modprobe -r ppa;modprobe -r sd_mod;modprobe lp"
 */
#define PrinterDefInitCmd	""

/*
 *	Default print command.
 *
 *	%file is replaced with the name of the tempory file to print.
 *	%printer is replaced with a printer ID name (not needed).
 *	%options is replaced by various other options as needed.
 */
#define PrinterDefPrintCmd	"lpr %file"


/*
 *	Printer title max:
 *
 *	For use with headers and footers too! Units are in bytes.
 */
#define PrinterTitleMax		256


/*
 *    Printer parameter structure:
 */
typedef struct {

	printer_flag_t options;

        char title[PrinterTitleMax];
	char header[PrinterTitleMax];
	char footer[PrinterTitleMax];

	int pages;		/* Total number of pages (rounded up). */
	int color_mode;		/* One of PrinterColorMode*. */

	int units;		/* One of PrinterUnit*. */

	double x, y;		/* Offset from upper left corner. */
	double width, height;	/* Size of paper. */

	int dest;		/* One of PrintDestination*. */

} printer_parm_struct;



extern int PrinterWritePSImage(
        FILE *fp,
        image_t *image,
        int page,	/* Page number (starts from 0). */
        printer_parm_struct *parm
);
extern int PrinterRunPrint(
        char *cmd,			/* Can be NULL. */
	char *filename			/* Must be valid. */
);
extern int PrinterPrintImage(
	image_t *image,
	char *tmp_file,			/* Can be NULL. */
	char *cmd,			/* Can be NULL. */
	printer_parm_struct *parm
);


#endif	/* PRINTERW_H */
