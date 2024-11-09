/*
	Targa Image File Format Library
 */

#ifndef TGA_H
#define TGA_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <sys/types.h>

/* Catch any undefined bit types (for Solaris). */
#include "os.h"



/*
 *   Master version numbers:
 */
#define TgaLibraryVersion	"1.5"
#define TgaVersionMajor		1
#define TgaVersionMinor		5


/*
 *   Error return codes:
 *
 *      These are error codes returned by the TGA Library functions.
 */
#define TgaSuccess              0
#define TgaNoBuffers            1
#define TgaBadHeader            2
#define TgaBadValue             3
#define TgaNoFile               4
#define TgaNoAccess             5

/*
 *   Error severity codes:
 *
 *      Error severity levels used in TgaReportError().
 *
 *	These codes are only used by interal TGA library functions.
 */
#define TgaErrorLevelWarning    0
#define TgaErrorLevelMinor      1
#define TgaErrorLevelModerate   2
#define TgaErrorLevelCritical   3



/*
 *   Length of TGA file header.
 */
#define TgaHeaderLength		0x12

/*
 *   Tga data type codes:
 *
 *	Byte 0x02 of the Tga header has one of these values.
 */
#define TgaDataTypeNoImage		0x00

#define TgaDataTypeUColorMapped		0x01	/* Uses a colormap. */
#define TgaDataTypeURGB			0x02	/* RGB colored. */
#define TgaDataTypeUBW			0x03	/* Actually greyscale. */

#define TgaDataTypeREColorMapped	0x09
#define TgaDataTypeRERGB		0x0a

#define TgaDataTypeCBW			0x0b

#define TgaDataTypeCHuffman		0x20
#define TgaDataTypeCHuffmanQ		0x21


/*
 *   Tga image descriptors bit positions:
 *
 *	Byte 0x12 of the Tga header has bits of these values.
 *
 *	NOTE: To comform to AT&T, all bits in byte 0x12 should be 0.
 *	However this is NOT always the case since some programs do
 *	not comform to this.
 *
 *	In most cases, TgaDescBitFliped is the bit you need to watch out
 *	for.
 */
#define TgaDescBitAttrBit0	0x00    /* Attr bits per pixel. */
#define TgaDescBitAttrBit1	0x00	/* Attr bits per pixel. */
#define TgaDescBitAttrBit2	0x00	/* Attr bits per pixel. */
#define TgaDescBitAttrBit3	0x00	/* Attr bits per pixel. */
#define TgaDescBit4		0x00	/* Bit 4 is reserved. */
#define TgaDescBitFliped	0x20	/* False = upside down image. */
#define TgaDescBitIntLeavFlag0	0x00    /* Data storage interleaving flag. */
#define TgaDescBitIntLeavFlag1	0x00    /* Data storage interleaving flag. */


/*
 *   Byte packing macros:
 */
#ifndef BYTES_PER_PIXEL8
# define BYTES_PER_PIXEL8	1	/* For rrgg gbbb. */
#endif

#ifndef BYTES_PER_PIXEL15
# define BYTES_PER_PIXEL15	2	/* For arrr rrgg gggb bbbb. */
#endif

#ifndef BYTES_PER_PIXEL16
# define BYTES_PER_PIXEL16	2	/* For rrr rrggg gggb bbbb. */
#endif

#ifndef BYTES_PER_PIXEL24
# define BYTES_PER_PIXEL24	4       /* Same as BYTES_PER_PIXEL32 */
#endif

#ifndef BYTES_PER_PIXEL32
# define BYTES_PER_PIXEL32	4
#endif


/*
 *	Bit packing (should be defined first in os.h):
 */
#ifndef PACK8TO8
# define PACK8TO8(r,g,b)	(u_int8_t)((r>>5)<<5)+((g>>5)<<2)+(b>>6)
#endif

#ifndef PACK8TO15
# define PACK8TO15(r,g,b)	(u_int16_t)((r>>3)<<10)+((g>>3)<<5)+(b>>3)
#endif

#ifndef PACK8TO16
# define PACK8TO16(r,g,b)	(u_int16_t)((r>>3)<<11)+((g>>2)<<5)+(b>>3)
#endif

#ifndef PACK8TO32
# define PACK8TO32(a,r,g,b)	(u_int32_t)((a<<24)|(r<<16)|(g<<8)|(b))
#endif


/*
 *   TGA Data Structure.
 *
 */
typedef struct {

	FILE *fp;		/* Set on partial reads only. */

	/* Header data of image fetched from file. */
	u_int8_t id_field_len;
	u_int8_t cmap_type;
	u_int8_t img_type;	/* One of TgaDataType*. */

	u_int32_t cmap_origin;
	u_int32_t cmap_length;
	u_int32_t cmap_size;		/* 16, 24, or 32 allowed. */

	int x, y;			/* Origin of image (on client program?). */
	unsigned int width, height;
	unsigned int depth;		/*   Depth of data store ON FILE.
					 *   Can be 1, 8, 16, 24, or 32.
					 */

	u_int8_t descriptor;		/* Descriptor flags. */

	u_int8_t bits_per_pixel;	/* Original bits per pixel. */

	/* Statistical information. */
	off_t file_size;    /* In bytes, size of entire file. */
	off_t data_size;    /* In bytes, size of image data. */

	int cur_load_pixel;	/* Mark on where we left off
				 * during loading, in units of pixels
				 * (for async loading).
				 */

	/* Loaded data: Data in memory. */
	u_int8_t *header_data;		/* In raw data (exact as on file). */

	u_int8_t *data;			/* In ZPixmap data format. */
	u_int8_t data_depth;		/* Bits per pixel of loaded data. */

} tga_data_struct;


/*
 *   Returns the version number of this library.
 */
extern int TgaQueryVersion(int *major_rtn, int *minor_rtn);

/*
 *   Used internally by TGA library functions for reporting errors.
 */
extern void TgaReportError(const char *filename, const char *reason, int how_bad);


/*
 *   Reads the TGA header from the file filename.
 *
 *   IMPORTANT: You do NOT need to call this function prior to
 *   TgaReadFromFile() or TgaReadFromData().
 */
extern int TgaReadHeaderFromFile(
	const char *filename,
	tga_data_struct *td
);


/*
 *   Reads the TGA header from the unsigned char buffer pointed to by
 *   data.
 *   
 *   IMPORTANT: You do NOT need to call this function prior to
 *   TgaReadFromFile() or TgaReadFromData(). 
 */
extern int TgaReadHeaderFromData(
	const u_int8_t *data,
	tga_data_struct *td
);


/*
 *   Reads the TGA data (including header data) from *filename
 *   to *td.
 *
 *   filename is the name of the TGA file.
 *
 *   td is the structure in which the loaded data will be put into.
 *
 *   depth is the bit depth of the destination data format.  Valid
 *   depth values are 16, 24, or 32.  Note that passing 24 will
 *   be the same as 32.
 *
 *   The loaded format will be compliant to the XImage ZPixmap format.
 *   Thus the data is ready to be used for an XImage's data.
 *
 *   Do NOT call TgaReadHeader() prior to this call!  By calling this
 *   function, both the image and header data will be loaded in one
 *   step.
 */
extern int TgaReadFromFile(
	const char *filename,
	tga_data_struct *td,
	unsigned int depth
);


/*  
 *   Reads the TGA data (including header data) from buffer pointed
 *   by data.
 *
 *   td is the structure in which the loaded data will be put into.
 *   
 *   depth is the bit depth of the destination data format.  Valid
 *   depth values are 8, 16, 24, or 32.  Note that passing 24 will
 *   be the same as 32.
 *
 *   The loaded format will be compliant to the XImage ZPixmap format.
 *   Thus the data is ready to be used for an XImage's data.
 *
 *   Do NOT call TgaReadHeader() prior to this call!  By calling this
 *   function, both the image and header data will be loaded in one
 *   step.
 */
extern int TgaReadFromData(
	const u_int8_t *data,
        tga_data_struct *td,
        unsigned int depth
);


/*
 *      Sets up td to begin reading tga file filename.
 * 
 *      Once this call has been made, you may use
 *      TgaReadPartialFromFile() to begin reading the data.
 * 
 *      depth is the bit depth of the destination data format.  Valid
 *      depth values are 8, 16, 24, or 32.  Note that passing 24 will
 *      be the same as 32.
 */
extern int TgaStartReadPartialFromFile(
	const char *filename,
	tga_data_struct *td,
        unsigned int depth
);


/*
 *   Reads a specific number of pixels from file.
 *
 *   The td must be initialized prior by a call to
 *   TgaStartReadPartialFromFile().
 */
extern int TgaReadPartialFromFile(  
        tga_data_struct *td, 
        unsigned int depth,  
        unsigned int n_pixels
);


/*
 *   Writes the tga image data contained in td to
 *   the file filename at the target depth specified.
 */
extern int TgaWriteToFile(
	const char *filename,
	tga_data_struct *td,
	unsigned int depth
);


/*
 *   Frees all allocated memory in td.
 */
extern void TgaDestroyData(tga_data_struct *td);


/*
 *   Checks if *filename is a valid TGA file, returns
 *   TgaSuccess if it is a valid TGA file or appropriate error
 *   if it is not.
 */
extern int TgaTestFile(const char *filename);



/* 
 *      Dithering functions in tgadither.c (used internally):
 */
extern u_int8_t TgaDitherRedPixel8(
        int RedValue,
        int Xp,
        int Yp
);
extern u_int8_t TgaDitherGreenPixel8(
        int GreenValue,
        int Xp,
        int Yp
); 
extern u_int8_t TgaDitherBluePixel8(
        int BlueValue,
        int Xp,
        int Yp
);


#ifdef __cplusplus
}
#endif

#endif /* TGA_H */
