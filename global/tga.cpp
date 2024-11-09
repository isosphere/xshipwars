#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "../include/os.h"
#include "../include/fio.h"
#include "../include/tga.h"

#ifdef MEMWATCH
# include "memwatch.h"
#endif


int TgaQueryVersion(int *major_rtn, int *minor_rtn);
void TgaReportError(
	const char *filename, const char *reason, int how_bad
);

int TgaReadHeaderFromFile(const char *filename, tga_data_struct *td);
int TgaReadHeaderFromData(const u_int8_t *data, tga_data_struct *td);

int TgaReadFromFile(
	const char *filename,
	tga_data_struct *td,
	unsigned int depth
);
int TgaReadFromData(
	const u_int8_t *data,
	tga_data_struct *td,
	unsigned int depth
);

int TgaStartReadPartialFromFile(
	const char *filename,
	tga_data_struct *td,
	unsigned int depth
);
int TgaReadPartialFromFile(
	tga_data_struct *td,
	unsigned int depth,
	unsigned int n_pixels
);

int TgaWriteToFile(
	const char *filename,
	tga_data_struct *td,
	unsigned int depth
);

void TgaDestroyData(tga_data_struct *td);

int TgaTestFile(const char *filename);
 

#ifndef MIN
# define MIN(a,b)	(((a) < (b)) ? (a) : (b))
#endif
#ifndef MAX
# define MAX(a,b)	(((a) > (b)) ? (a) : (b))
#endif


/*
 *	`Almost' black pixel values, needed for rounded off
 *	`almost' black pixels on file that decrease in depth.
 *	Depth that need this are 8, 15, and 16.
 */
#define ALMOSTBLACKPIX8		((u_int8_t)(1 << 6) |\
                                 (u_int8_t)(1 << 3) |\
                                 (u_int8_t)(1 << 0))
#define ALMOSTBLACKPIX15	((u_int16_t)(1 << 10) |\
                                 (u_int16_t)(1 << 5)  |\
                                 (u_int16_t)(1 << 0))
#define ALMOSTBLACKPIX16	((u_int16_t)(1 << 11) |\
                                 (u_int16_t)(1 << 5) |\
                                 (u_int16_t)(1 << 0))


/*
 *	Returns the version number.
 */
int TgaQueryVersion(int *major_rtn, int *minor_rtn)
{
	/* Major version number. */
	if(major_rtn != NULL)
	    *major_rtn = TgaVersionMajor;

	/* Minor version number. */
	if(minor_rtn != NULL)
	    *minor_rtn = TgaVersionMinor;


	return(0);
}



/*
 *	Error reporting function.
 */
void TgaReportError(
	const char *filename,
	const char *reason,
	int how_bad
)
{

	/* Error heading. */
	switch(how_bad)
	{
	  case TgaErrorLevelCritical:
	    fprintf(stderr, "Targa Library Critical error:\n");
	    break;

	  case TgaErrorLevelModerate:
            fprintf(stderr, "Targa Library Moderate error:\n");
            break;

	  case TgaErrorLevelMinor:
            fprintf(stderr, "Targa Library Minor error:\n");
            break;

          case TgaErrorLevelWarning:
            fprintf(stderr, "Targa Library Warning:\n");
            break;
 
	  default:
            fprintf(stderr, "Targa Library Error:\n");
	    break;
	}

	/* Filename. */
	if(filename != NULL)
	{
	    fprintf(stderr, "   Filename: %s\n", filename);
	}

	/* Reason. */
	if(reason != NULL)
	{
	    fprintf(stderr, "   Reason: %s\n", reason);
	}

	return;
}


/*
 *	Reads the tga header information from filename and sets
 *	appropriate header values in td.
 */
int TgaReadHeaderFromFile(const char *filename, tga_data_struct *td)
{
	FILE *fp;
	struct stat statbuf;
	int fpos;
	int i;
	char stringa[1024];


	/* Error checks. */
	if((filename == NULL) ||
           (td == NULL)
	)
	    return(TgaNoBuffers);


        /* Clear tga data struct. */
        memset(td, 0, sizeof(tga_data_struct));


	/* Check if file exists and if so, get stats. */
	if(stat(filename, &statbuf))
	    return(TgaNoFile);

	/* Set file size. */
        td->file_size = statbuf.st_size;
	/* Size too small? */
        if(td->file_size < TgaHeaderLength)
            return(TgaBadHeader);


	/* Open file. */
        fp = FOpen(filename, "rb");
        if(fp == NULL)
            return(TgaNoAccess); 


	/* ***************************************************************** */

	/* Allocate memory for header_data. */
	td->header_data = (u_int8_t *)calloc(
	    1,
	    TgaHeaderLength * sizeof(u_int8_t)
	);
	if(td->header_data == NULL)
	    return(TgaNoBuffers);


	/* Read header. */
        for(fpos = 0; fpos < (int)statbuf.st_size; fpos++)
        {
	    /* Don't read past header. */
	    if(fpos >= TgaHeaderLength)
		break;

	    /* Get a character from tga file. */
            i = fgetc(fp);
	    if(i == EOF)
		break;

	    /* Copy i to header_data. */
            td->header_data[fpos] = (u_int8_t)i;


            /* Image ID field length (value is in bytes). */
	    if(fpos == 0x00)
	    {
		td->id_field_len = (u_int8_t)i;
                continue;
	    }
	    /* Color map type. */
            else if(fpos == 0x01)
            {
                td->cmap_type = (u_int8_t)i;
                continue;
            }
	    /* Image data type code. */
            else if(fpos == 0x02)
            {
		td->img_type = (u_int8_t)i;
                continue;
            }
	    /* Colormap Specification - Colormap Origin */
            else if(fpos == 0x03)
            {
		td->cmap_origin = (u_int32_t)i;

                /* Get next char. */
                fpos++;
                i = fgetc(fp);

		/* fpos may not exceeding allocated header memory. */
		if(fpos < TgaHeaderLength)
		{
                    /* Copy c to header_data. */
                    td->header_data[fpos] = (u_int8_t)i;
		}

                td->cmap_origin += (u_int32_t)((u_int32_t)i << 8);

		continue;
	    }
            /* Colormap Specification - Colormap Length */
            else if(fpos == 0x05)
            {
                td->cmap_length = (u_int32_t)i;

                /* Get next char. */
                fpos++;
                i = fgetc(fp);

                /* fpos may not exceeding allocated header memory. */
                if(fpos < TgaHeaderLength)
                {
                    /* Copy c to header_data. */
                    td->header_data[fpos] = (u_int8_t)i;
		}

                td->cmap_length += (u_int32_t)((u_int32_t)i << 8);

                continue;
            }
            /* Colormap Specification - Colormap Size */
            else if(fpos == 0x07)
            {
                td->cmap_size = (u_int32_t)i;

                continue;
            }
            /* Image Specification - X Origin */
            else if(fpos == 0x08)
            {
                td->x = i;

                /* Get next char. */
                fpos++;
                i = fgetc(fp);

                /* fpos may not exceeding allocated header memory. */
                if(fpos < TgaHeaderLength)
                {
                    /* Copy c to header_data. */
                    td->header_data[fpos] = (u_int8_t)i;
		}

                td->x += ((u_int16_t)i << 8);

                continue;
            }
            /* Image Specification - Y Origin */
            else if(fpos == 0x0a)
            {
                td->y = i;

                /* Get next char. */
                fpos++;
                i = fgetc(fp);

                /* fpos may not exceeding allocated header memory. */
                if(fpos < TgaHeaderLength)
                {
                    /* Copy c to header_data. */
                    td->header_data[fpos] = (u_int8_t)i;
		}

                td->y += ((u_int16_t)i << 8);

                continue;
            }
            /* Image Specification - Width */
            else if(fpos == 0x0c)
            {
                td->width = i;

		/* Get next char. */
                fpos++;
                i = fgetc(fp);

                /* fpos may not exceeding allocated header memory. */
                if(fpos < TgaHeaderLength)
                {
                    /* Copy c to header_data. */
                    td->header_data[fpos] = (u_int8_t)i;
		}

                td->width += ((u_int16_t)i << 8);

                continue;
            }
            /* Image Specification - Height */
            else if(fpos == 0x0e)
            {
                td->height = i;

		/* Get next char. */
                fpos++;
                i = fgetc(fp);

                /* fpos may not exceeding allocated header memory. */
                if(fpos < TgaHeaderLength)
                {
                    /* Copy c to header_data. */
                    td->header_data[fpos] = (u_int8_t)i;
		}

                td->height += ((u_int16_t)i << 8);

                continue;
            }
            /* Image Specification - Depth. */
            else if(fpos == 0x10)
            {
                td->depth = (u_int8_t)i;

                continue;
            }
            /* Image Specification - Descriptor (Attributes). */
            else if(fpos == 0x11)
            {
                td->descriptor = (u_int8_t)i; 

                continue;
            }
	    /* Color map data (how many bytes per pixel in data field). */

	    /*   According to the specification the location of this byte
	     *   varies in the file.  Since we are not supporting color-
	     *   mapped images right now, it's safe to skip this.
	     */
/*
	    else if(fpos == 0x??)
	    {
		continue;
	    }
*/

        }


	/* Set bits_per_pixel in accordance with given depth.
	 * (See farther above for reason).
         * Remember that this the bits per pixel ON FILE (not in memory).
	 */
	if(td->depth == 8)
	    td->bits_per_pixel = 8;
	else if(td->depth == 24)
	    td->bits_per_pixel = 24;
	else if(td->depth == 32)
	    td->bits_per_pixel = 32;
	else
	    td->bits_per_pixel = 24; /* Default to 24 bits. */



        /* Close the file. */
        FClose(fp);
	fp = NULL;


	/* ***************************************************************** */

	/* Check and make sure width, height and bit depth are valid. */
	if(td->width == 0)
	{
	    TgaReportError(
		filename,
		"Width of image is less than 1 pixel.",
		TgaErrorLevelModerate
	    );
	    return(TgaBadValue);
	}
        if(td->height == 0)
        {
            TgaReportError(
                filename,
                "Height of image is less than 1 pixel.",
                TgaErrorLevelModerate
            );
            return(TgaBadValue);
        }
        if((td->depth != 1) &&
           (td->depth != 8) &&
           (td->depth != 16) &&
           (td->depth != 24) &&
           (td->depth != 32)
	)
        {
            TgaReportError(
                filename,
                "Invalid bit depth.",
                TgaErrorLevelWarning
            );
	    /* This is just a warning. */
        }


	/* Calculate and verify data size. */
	td->data_size = (long)td->file_size - TgaHeaderLength -
	    (long)td->id_field_len;

	if((int)td->data_size !=
           ((int)td->width * (int)td->height * (int)(td->depth >> 3))
	)
	{
	    /* Warn about inconsistant size. */
	    sprintf(stringa,
 "Image data size %i less than header indicated size %i.\n",
		(int)td->data_size,
		((int)td->width * (int)td->height * (int)(td->depth >> 3))
	    );
	    TgaReportError(
                filename,
                stringa,
                TgaErrorLevelWarning
            );
	}


	return(TgaSuccess);
}


/*
 *      Reads the tga header information from data in memory
 *	and sets the appropriate header values in td.
 */
int TgaReadHeaderFromData(const u_int8_t *data, tga_data_struct *td)
{
	int fpos, i;
	const u_int8_t *data_ptr;


	/* Error checks. */
	if((data == NULL) ||
           (td == NULL)
	)
	    return(TgaNoBuffers);


        /* Clear td struct. */
        memset(td, 0, sizeof(tga_data_struct));

	data_ptr = (u_int8_t *)data;


        /* These are unknown for headers read from data. */
        td->file_size = 0;
        td->data_size = 0;


        /* Allocate memory for header_data. */
        td->header_data = (u_int8_t *)calloc(
            1,
            TgaHeaderLength * sizeof(u_int8_t)
        );
        if(td->header_data == NULL)
            return(TgaNoBuffers);


        /* Read header. */
        for(fpos = 0; fpos < TgaHeaderLength; fpos++)
        {
            /* Get a character from tga data. */
            i = *data_ptr++;


            /* Copy i to header_data. */
            td->header_data[fpos] = (u_int8_t)i;


            /* Image ID field length (value is in bytes). */
            if(fpos == 0x00)
            {
                td->id_field_len = (u_int8_t)i;
                continue;
            }
            /* Color map type. */
            else if(fpos == 0x01)
            {
                td->cmap_type = (u_int8_t)i;
                continue;
            }
            /* Image type code. */
            else if(fpos == 0x02)
            {
                td->img_type = (u_int8_t)i;
                continue;
            }
            /* Colormap Specification - Colormap Origin */
            else if(fpos == 0x03)
            {
                td->cmap_origin = (u_int32_t)i;

                /* Get next char. */
                fpos++;
		i = *data_ptr++;

                /* fpos may not exceeding allocated header memory. */
                if(fpos < TgaHeaderLength)
                {
                    /* Copy c to header_data. */
                    td->header_data[fpos] = (u_int8_t)i;
                }

                td->cmap_origin += (u_int32_t)((u_int32_t)i << 8);

                continue;
            }
            /* Colormap Specification - Colormap Length */
            else if(fpos == 0x05)
            {
                td->cmap_length = (u_int32_t)i;

                /* Get next char. */
                fpos++;
                i = *data_ptr++;

                /* fpos may not exceeding allocated header memory. */
                if(fpos < TgaHeaderLength)
                {
                    /* Copy c to header_data. */
                    td->header_data[fpos] = (u_int8_t)i;
                }

                td->cmap_length += (u_int32_t)((u_int32_t)i << 8);

                continue;
            }
            /* Colormap Specification - Colormap Size */
            else if(fpos == 0x07)
            {
                td->cmap_size = (u_int32_t)i;

                continue;
            }
            /* Image Specification - X Origin */
            else if(fpos == 0x08)
            {
                td->x = i;

                /* Get next char. */
                fpos++;
                i = *data_ptr++;

                /* fpos may not exceeding allocated header memory. */
                if(fpos < TgaHeaderLength)
                {
                    /* Copy c to header_data. */
                    td->header_data[fpos] = (u_int8_t)i;
                }

                td->x += ((u_int16_t)i << 8);

                continue;
            }
            /* Image Specification - Y Origin */
            else if(fpos == 0x0a)
            {
                td->y = i;

                /* Get next char. */
                fpos++;
                i = *data_ptr++;

                /* fpos may not exceeding allocated header memory. */
                if(fpos < TgaHeaderLength)
                {
                    /* Copy c to header_data. */
                    td->header_data[fpos] = (u_int8_t)i;
                }

                td->y += ((u_int16_t)i << 8);

                continue;
            }
            /* Image Specification - Width */
            else if(fpos == 0x0c)
            {
                td->width = i;

                /* Get next char. */
                fpos++;
                i = *data_ptr++;

                /* fpos may not exceeding allocated header memory. */
                if(fpos < TgaHeaderLength)
                {
                    /* Copy c to header_data. */
                    td->header_data[fpos] = (u_int8_t)i;
                }

                td->width += ((u_int16_t)i << 8);

                continue;
            }
            /* Image Specification - Height */
            else if(fpos == 0x0e)
            {
                td->height = i;

                /* Get next char. */
                fpos++;
                i = *data_ptr++;

                /* fpos may not exceeding allocated header memory. */
                if(fpos < TgaHeaderLength)
                {
                    /* Copy c to header_data. */
                    td->header_data[fpos] = (u_int8_t)i;
                }

                td->height += ((u_int16_t)i << 8);

                continue;
            }
            /* Image Specification - Depth. */
            else if(fpos == 0x10)
            {
                td->depth = (u_int8_t)i;

                continue;
            }
            /* Image Specification - Descriptor (Attributes). */
            else if(fpos == 0x11)
            {
                td->descriptor = (u_int8_t)i; 
    
                continue;
            }
            /* Color map data (how many bytes per pixel in data field). */

            /*   According to the specification the location of this byte
             *   varies in the file.  Since we are not supporting color-
             *   mapped images right now, it's safe to skip this.
             */
/*
            else if(fpos == 0x??)
            {
                continue;
            }
*/

        }

        /* Set bits_per_pixel in accordance with given depth.
         * (See farther above for reason).
         * Remember that this the bits per pixel in data.
         */
	if(td->depth == 8)
	    td->bits_per_pixel = 8;
        else if(td->depth == 24)
            td->bits_per_pixel = 24;
        else if(td->depth == 32)
            td->bits_per_pixel = 32;
        else
            td->bits_per_pixel = 24; /* Default to 24 bits. */


        /* ***************************************************************** */
        /* Check and make sure width, height and bit depth are valid. */

	/* Width. */
        if(td->width == 0)
        {
            TgaReportError(
                "Tga data",
                "Width of image is less than 1 pixel.",
                TgaErrorLevelModerate
            );
            return(TgaBadValue);
        }

	/* Height. */
        if(td->height == 0)
        {
            TgaReportError(
                "Tga data",
                "Height of image is less than 1 pixel.",
                TgaErrorLevelModerate
            );
            return(TgaBadValue);
        }

	/* Bit depth. */
        if((td->depth != 1) &&
           (td->depth != 8) &&
           (td->depth != 16) &&
           (td->depth != 24) &&
           (td->depth != 32)
        )
        {
            TgaReportError(
                "Tga data",
                "Invalid bit depth.",
                TgaErrorLevelWarning
            );
            /* This is just a warning. */
        }


	/* Calculate file size and data size. */
	td->data_size = td->width * td->height * (td->depth >> 3);
	td->file_size = (long)td->data_size + TgaHeaderLength +
	    (long)td->id_field_len;


	return(TgaSuccess);
}



/*
 *	Loads the image from file.
 *
 *	No need to load header prior to call to this
 *	function.
 */
int TgaReadFromFile(
	const char *filename,
	tga_data_struct *td,
	unsigned int depth
)
{
	FILE *fp;
	int i, status;
	unsigned int bytes_per_pixel;
	char stringa[256];

        int fpos;
        int data_pos;

        u_int8_t *data_ptr8;
	u_int16_t *data_ptr16;
	u_int32_t *data_ptr32;

	u_int8_t pix[4], r, g, b;

	int pix_total;
	int colum_count, row_count;


	/* Make sure filename and td are valid. */
        if((filename == NULL) ||
           (td == NULL)
        )
            return(TgaBadValue);


	/* Make sure the GIVEN depth for the DESTINATION buffer is one
	 * that is supported.
	 */
	if((depth != 8) &&
           (depth != 15) &&
           (depth != 16) &&
           (depth != 24) &&
           (depth != 32)
	)
	{
	    sprintf(stringa,
		"Requested destination buffer depth %i is not supported.",
		depth
	    );
            TgaReportError(filename, stringa, TgaErrorLevelCritical);
            return(TgaBadValue);
	}
	/* Sanitize GIVEN depth,  if it is 24 then it should be 32. */
	if(depth == 24)
	    depth = 32;


	/* ********************************************************** */

	/* Fetch header information for td. */
	status = TgaReadHeaderFromFile(filename, td);
	if(status != TgaSuccess)
	    return(status);

	/* Make sure the SOURCE file bit depth is supported by this
         * function.
	 */
        if((td->depth != 8) &&
           (td->depth != 24) &&
           (td->depth != 32)
        )
        {
            sprintf(stringa,
                "Image file depth %i is not supported.",
                td->depth
            );
            TgaReportError(filename, stringa, TgaErrorLevelCritical);
            return(TgaBadValue);
        }

	/* Open the file. */
        fp = FOpen(filename, "rb");
        if(fp == NULL)
            return(TgaNoAccess);

        /* Get member data_depth since it represents the LOADED data
         * in number of bytes per pixel IN MEMORY.   (Do not confuse this
         * with member bytes_per_pixel which indicates bytes per pixel on
	 * FILE.)
         */
        td->data_depth = depth;	/* In bits per pixel */


	/* Calculate exact target bytes per pixel. */
	switch(td->data_depth)
	{
          case 24:
            bytes_per_pixel = BYTES_PER_PIXEL24;
            break;

	  case 15:
	    bytes_per_pixel = BYTES_PER_PIXEL15;
	    break;

	  default:
	    bytes_per_pixel = (td->data_depth >> 3);
	    break;
	}
	/* Allocate memory for image data, we can use malloc() since
	 * this is not async loading.
	 */
        td->data = (u_int8_t *)malloc(
            td->width * td->height * bytes_per_pixel
	);
	if(td->data == NULL)
	    return(TgaNoBuffers);


	/* Skip header. */
	for(fpos = 0; fpos < (TgaHeaderLength + td->id_field_len); fpos++)
	{
	    i = fgetc(fp);
	    if(i == EOF)
	    {
                TgaReportError(filename,
		    "Header length is too short.",
		    TgaErrorLevelCritical
		);
	        return(TgaBadHeader);
	    }
	}
	/* fpos is now at TgaHeaderLength + td->id_field_len. */


	/* *************************************************************** */

	/* Begin reading the image data from file. */

	/* Check which encoding style the image data on file is in. */
	if(td->descriptor & TgaDescBitFliped)
	{
	    /* READ RIGHTSIDE UP. */

            pix_total = td->width * td->height;
            colum_count = 0;
            row_count = 0;
            data_pos = row_count * td->width * bytes_per_pixel;

	    /* Read by DESTINATION buffer depth. */
	    switch(td->data_depth)
	    {
	      /* 32 bits. */
	      case 32:
                while((fpos < (int)td->file_size) &&
                      (row_count < (int)td->height)
                )
                {
		    /* Get pointer in destination buffer. */
		    data_ptr32 = (u_int32_t *)(&td->data[data_pos]);

                    /* Get pixel color values depending on 
                     * bits_per_pixel ON FILE.
                     */
		    switch(td->bits_per_pixel)
		    {
		      case 32:
                        pix[0] = (u_int8_t)fgetc(fp);
                        pix[1] = (u_int8_t)fgetc(fp);
                        pix[2] = (u_int8_t)fgetc(fp);
			pix[3] = (u_int8_t)fgetc(fp);
			*data_ptr32 = PACK8TO32(pix[3], pix[2], pix[1], pix[0]);
                        break;

		      case 8:
                        pix[0] = (u_int8_t)fgetc(fp);
                        *data_ptr32 = PACK8TO32(0x00, pix[0], pix[0], pix[0]);
			break;

		      default:	/* Default to 24-bits. */
			pix[0] = (u_int8_t)fgetc(fp);
                	pix[1] = (u_int8_t)fgetc(fp);
                	pix[2] = (u_int8_t)fgetc(fp);
			*data_ptr32 = PACK8TO32(0x00, pix[2], pix[1], pix[0]);
			break;
		    }

		    /* Increment data position in destination buffer. */
                    data_pos += BYTES_PER_PIXEL32;

                    /* Increment colum_count. */
                    colum_count++;

                    if(colum_count >= (int)td->width)
                    {
                        colum_count = 0;
                        row_count++;
                        data_pos = row_count * td->width * BYTES_PER_PIXEL32;
                    }
		}
		break;

	      /* 16 bits. */
	      case 16:
                while((fpos < (int)td->file_size) &&
                      (row_count < (int)td->height) 
                )
                {
                    /* Get pointer in destination buffer. */
                    data_ptr16 = (u_int16_t *)(&td->data[data_pos]);

                    /* Get pixel color values depending on 
                     * bits_per_pixel ON FILE.
                     */
                    switch(td->bits_per_pixel)
                    {
                      case 32:
                        pix[0] = (u_int8_t)fgetc(fp);
                        pix[1] = (u_int8_t)fgetc(fp);
                        pix[2] = (u_int8_t)fgetc(fp);
			pix[3] = (u_int8_t)fgetc(fp);
			*data_ptr16 = PACK8TO16(pix[2], pix[1], pix[0]);
                        break;

                      case 8:
                        pix[0] = (u_int8_t)fgetc(fp);
                        *data_ptr16 = PACK8TO16(pix[0], pix[0], pix[0]);
                        break;

                      default:  /* Default to 24-bits. */
                        pix[0] = (u_int8_t)fgetc(fp);
                        pix[1] = (u_int8_t)fgetc(fp);
                        pix[2] = (u_int8_t)fgetc(fp);
			*data_ptr16 = PACK8TO16(pix[2], pix[1], pix[0]);
                        break;
                    }
		    /* `Almost' black pixel roundoff check. */
		    if(*data_ptr16 == 0x0000)
		    {
			if(pix[0] || pix[1] || pix[2])
			    *data_ptr16 = ALMOSTBLACKPIX16;
		    }

                    /* Increment data position in destination buffer. */
                    data_pos += BYTES_PER_PIXEL16;

                    /* Increment colum_count. */
                    colum_count++;

                    if(colum_count >= (int)td->width)
                    {
                        colum_count = 0;
                        row_count++;
                        data_pos = row_count * td->width * BYTES_PER_PIXEL16;
                    }
                }
		break;

              /* 15 bits. */
              case 15:
                while((fpos < (int)td->file_size) &&
                      (row_count < (int)td->height)
                )
                {
                    /* Get pointer in destination buffer. */
                    data_ptr16 = (u_int16_t *)(&td->data[data_pos]);

                    /* Get pixel color values depending on
                     * bits_per_pixel ON FILE.
                     */
                    switch(td->bits_per_pixel)
                    {
                      case 32:
                        pix[0] = (u_int8_t)fgetc(fp);
                        pix[1] = (u_int8_t)fgetc(fp);
                        pix[2] = (u_int8_t)fgetc(fp);
                        pix[3] = (u_int8_t)fgetc(fp);
                        *data_ptr16 = PACK8TO15(pix[2], pix[1], pix[0]);
                        break;

                      case 8:
                        pix[0] = (u_int8_t)fgetc(fp);
                        *data_ptr16 = PACK8TO15(pix[0], pix[0], pix[0]);
                        break;

                      default:  /* Default to 24-bits. */
                        pix[0] = (u_int8_t)fgetc(fp);
                        pix[1] = (u_int8_t)fgetc(fp);
                        pix[2] = (u_int8_t)fgetc(fp);
                        *data_ptr16 = PACK8TO15(pix[2], pix[1], pix[0]);
                        break;
                    }
                    /* `Almost' black pixel roundoff check. */
                    if(*data_ptr16 == 0x0000)
                    {
                        if(pix[0] || pix[1] || pix[2])
                            *data_ptr16 = ALMOSTBLACKPIX15;
                    }

                    /* Increment data position in destination buffer. */
                    data_pos += BYTES_PER_PIXEL15;

                    /* Increment colum_count. */
                    colum_count++;
                 
                    if(colum_count >= (int)td->width)
                    {
                        colum_count = 0;
                        row_count++;
                        data_pos = row_count * td->width * BYTES_PER_PIXEL15;
                    }
                }
                break;

              /* 8 bits. */
              case 8:
                while((fpos < (int)td->file_size) &&
                      (row_count < (int)td->height)
                )
                {
                    /* Get pointer in destination buffer. */
                    data_ptr8 = (u_int8_t *)(&td->data[data_pos]);

                    /* Get pixel color values depending on
                     * bits_per_pixel ON FILE.
                     */
                    switch(td->bits_per_pixel)
                    {
                      case 32:
                        r = (u_int8_t)fgetc(fp);
                        pix[0] = TgaDitherRedPixel8(r, colum_count, row_count);
                        g = (u_int8_t)fgetc(fp);
                        pix[1] = TgaDitherGreenPixel8(g, colum_count, row_count);
                        b = (u_int8_t)fgetc(fp);
                        pix[2] = TgaDitherBluePixel8(b, colum_count, row_count);
                        pix[3] = (u_int8_t)fgetc(fp);
                        *data_ptr8 = PACK8TO8(pix[2], pix[1], pix[0]);
                        break;

                      case 8:
                        pix[0] = (u_int8_t)fgetc(fp);
                        *data_ptr8 = PACK8TO8(pix[0], pix[0], pix[0]);
                        break;

                      default:  /* Default to 24-bits. */
                        r = (u_int8_t)fgetc(fp);
                        pix[0] = TgaDitherRedPixel8(r, colum_count, row_count);
                        g = (u_int8_t)fgetc(fp);
                        pix[1] = TgaDitherGreenPixel8(g, colum_count, row_count);
                        b = (u_int8_t)fgetc(fp);
                        pix[2] = TgaDitherBluePixel8(b, colum_count, row_count);
                        *data_ptr8 = PACK8TO8(pix[2], pix[1], pix[0]);
                        break;
                    }
                    /* `Almost' black pixel roundoff check. */
                    if(*data_ptr8 == 0x00)
                    { 
                        if(pix[0] || pix[1] || pix[2])
                            *data_ptr8 = ALMOSTBLACKPIX8;
                    }

                    /* Increment data position in destination buffer. */
                    data_pos += BYTES_PER_PIXEL8;

                    /* Increment colum_count. */
                    colum_count++;
             
                    if(colum_count >= (int)td->width)
                    {
                        colum_count = 0;
                        row_count++;
                        data_pos = row_count * td->width * BYTES_PER_PIXEL8;
                    }
                }
		break;
            }
	}
	else
	{
	    /* READ UPSIDE DOWN. */

            pix_total = td->width * td->height;
            colum_count = 0;
            row_count = (int)td->height - 1;
            data_pos = row_count * td->width * bytes_per_pixel;

            /* Read by DESTINATION buffer depth. */
            switch(td->data_depth)
            {
	      /* 32 bits. */
              case 32:
	        while((fpos < (int)td->file_size) &&
                      (row_count >= 0)
	        )
	        {
                    /* Get pointer in destination buffer. */
                    data_ptr32 = (u_int32_t *)(&td->data[data_pos]);

                    /* Get pixel color values depending on
		     * bits_per_pixel ON FILE.
		     */
                    switch(td->bits_per_pixel)
                    { 
                      case 32:
                        pix[0] = (u_int8_t)fgetc(fp);
                        pix[1] = (u_int8_t)fgetc(fp);
                        pix[2] = (u_int8_t)fgetc(fp);
                        pix[3] = (u_int8_t)fgetc(fp);
			*data_ptr32 = PACK8TO32(pix[3], pix[2], pix[1], pix[0]);
                        break;

                      case 8:
                        pix[0] = (u_int8_t)fgetc(fp);
                        *data_ptr32 = PACK8TO32(0x00, pix[0], pix[0], pix[0]);
                        break;

                      default:  /* Default to 24-bits. */
                        pix[0] = (u_int8_t)fgetc(fp);
                        pix[1] = (u_int8_t)fgetc(fp);
                        pix[2] = (u_int8_t)fgetc(fp);
                        *data_ptr32 = PACK8TO32(0x00, pix[2], pix[1], pix[0]);
                        break;
                    }

                    /* Increment data position in destination buffer. */
                    data_pos += BYTES_PER_PIXEL32;

	            /* Increment colum_count. */
	            colum_count++;

	            if(colum_count >= (int)td->width)
	            {
		        colum_count = 0;
		        row_count--;
		        data_pos = row_count * td->width * BYTES_PER_PIXEL32;
	            }
		}
		break;

              /* 16 bits. */
              case 16:
                while((fpos < td->file_size) &&
                      (row_count >= 0)
                )
                {
                    /* Get pointer in destination buffer. */
                    data_ptr16 = (u_int16_t *)(&td->data[data_pos]);

                    /* Get pixel color values depending on
                     * bits_per_pixel ON FILE. 
                     */
                    switch(td->bits_per_pixel)
                    {
                      case 32:
                        pix[0] = (u_int8_t)fgetc(fp);
                        pix[1] = (u_int8_t)fgetc(fp);
                        pix[2] = (u_int8_t)fgetc(fp);
                        pix[3] = (u_int8_t)fgetc(fp);
			*data_ptr16 = PACK8TO16(pix[2], pix[1], pix[0]);
                        break;

                      case 8:
                        pix[0] = (u_int8_t)fgetc(fp);
                        *data_ptr16 = PACK8TO16(pix[0], pix[0], pix[0]);
                        break;

                      default:  /* Default to 24-bits. */
                        pix[0] = (u_int8_t)fgetc(fp);
                        pix[1] = (u_int8_t)fgetc(fp);
                        pix[2] = (u_int8_t)fgetc(fp);
                        *data_ptr16 = PACK8TO16(pix[2], pix[1], pix[0]);
                        break;
                    }
                    /* `Almost' black pixel roundoff check. */
                    if(*data_ptr16 == 0x0000)
                    {
                        if(pix[0] || pix[1] || pix[2])
                            *data_ptr16 = ALMOSTBLACKPIX16;
                    }

                    /* Increment data position in destination buffer. */
                    data_pos += BYTES_PER_PIXEL16;

                    /* Increment colum_count. */
                    colum_count++;

                    if(colum_count >= (int)td->width)
                    {
                        colum_count = 0;
                        row_count--;
                        data_pos = row_count * td->width * BYTES_PER_PIXEL16;
                    }
                }
                break;

              /* 15 bits. */
              case 15:
                while((fpos < td->file_size) &&
                      (row_count >= 0)
                )
                {
                    /* Get pointer in destination buffer. */
                    data_ptr16 = (u_int16_t *)(&td->data[data_pos]);

                    /* Get pixel color values depending on
                     * bits_per_pixel ON FILE.
                     */
                    switch(td->bits_per_pixel)
                    {
                      case 32:
                        pix[0] = (u_int8_t)fgetc(fp);
                        pix[1] = (u_int8_t)fgetc(fp);
                        pix[2] = (u_int8_t)fgetc(fp);
                        pix[3] = (u_int8_t)fgetc(fp);
                        *data_ptr16 = PACK8TO15(pix[2], pix[1], pix[0]);
                        break;

                      case 8:
                        pix[0] = (u_int8_t)fgetc(fp);
                        *data_ptr16 = PACK8TO15(pix[0], pix[0], pix[0]);
                        break;

                      default:  /* Default to 24-bits. */
                        pix[0] = (u_int8_t)fgetc(fp);
                        pix[1] = (u_int8_t)fgetc(fp);
                        pix[2] = (u_int8_t)fgetc(fp);
                        *data_ptr16 = PACK8TO15(pix[2], pix[1], pix[0]);
                        break;
                    }
                    /* `Almost' black pixel roundoff check. */
                    if(*data_ptr16 == 0x0000) 
                    {
                        if(pix[0] || pix[1] || pix[2])
                            *data_ptr16 = ALMOSTBLACKPIX15;
                    }

                    /* Increment data position in destination buffer. */
                    data_pos += BYTES_PER_PIXEL15;

                    /* Increment colum_count. */
                    colum_count++;

                    if(colum_count >= (int)td->width)
                    {
                        colum_count = 0;
                        row_count--;
                        data_pos = row_count * td->width * BYTES_PER_PIXEL15;
                    }
                }
                break;

              /* 8 bits. */
              case 8:
                while((fpos < (int)td->file_size) &&
                      (row_count >= 0)
                )
                {
                    /* Get pointer in destination buffer. */
                    data_ptr8 = (u_int8_t *)(&td->data[data_pos]);

                    /* Get pixel color values depending on
                     * bits_per_pixel ON FILE.
                     */
                    switch(td->bits_per_pixel)
                    {
                      case 32:
                        r = (u_int8_t)fgetc(fp);
                        pix[0] = TgaDitherRedPixel8(r, colum_count, row_count);
                        g = (u_int8_t)fgetc(fp);
                        pix[1] = TgaDitherGreenPixel8(g, colum_count, row_count);
                        b = (u_int8_t)fgetc(fp);
                        pix[2] = TgaDitherBluePixel8(b, colum_count, row_count);
                        pix[3] = (u_int8_t)fgetc(fp);
                        *data_ptr8 = PACK8TO8(pix[2], pix[1], pix[0]);
                        break;

                      case 8:
                        pix[0] = (u_int8_t)fgetc(fp);
                        *data_ptr8 = PACK8TO8(pix[0], pix[0], pix[0]);
                        break;

                      default:  /* Default to 24-bits. */
                        r = (u_int8_t)fgetc(fp);
                        pix[0] = TgaDitherRedPixel8(r, colum_count, row_count);
                        g = (u_int8_t)fgetc(fp);
                        pix[1] = TgaDitherGreenPixel8(g, colum_count, row_count);
                        b = (u_int8_t)fgetc(fp);
                        pix[2] = TgaDitherBluePixel8(b, colum_count, row_count);
                        *data_ptr8 = PACK8TO8(pix[2], pix[1], pix[0]);
                        break;
                    }
                    /* `Almost' black pixel roundoff check. */
                    if(*data_ptr8 == 0x00)
                    {
                        if(pix[0] || pix[1] || pix[2])
                            *data_ptr8 = ALMOSTBLACKPIX8;
                    }

                    /* Increment data position in destination buffer. */
                    data_pos += BYTES_PER_PIXEL8;

                    /* Increment colum_count. */
                    colum_count++;

                    if(colum_count >= (int)td->width)
                    {
                        colum_count = 0;
                        row_count--;
                        data_pos = row_count * td->width * BYTES_PER_PIXEL8;
                    }
                }
                break;
	    }
	}


	/* Close the file. */
        FClose(fp);


	return(TgaSuccess);
}


/*
 *      Loads the image from data in memory.
 *
 *      No need to load header prior to call to this      
 *      function.
 */
int TgaReadFromData(
	const u_int8_t *data,
        tga_data_struct *td,
        unsigned int depth
)
{
	int status;
	unsigned int bytes_per_pixel;
	char stringa[256];
	int fpos;
	u_int8_t *data_ptr;

        int data_pos;
	u_int8_t *data_ptr8;
        u_int16_t *data_ptr16;
        u_int32_t *data_ptr32;

        u_int8_t pix[4], r, g, b;

        int pix_total;
        int colum_count, row_count;


	/* Error checks. */
	if(data == NULL)
	    return(TgaNoBuffers);


        /*   GIVEN depth for DESTINATION buffer must be one that
         *   we support.
         */
        if((depth != 8) &&
           (depth != 15) &&
           (depth != 16) &&
           (depth != 24) &&
           (depth != 32)
        )
        {
            sprintf(stringa,
                "Requested destination buffer bit depth %i not supported.",
                depth
            );
            TgaReportError("Tga data", stringa, TgaErrorLevelCritical);
            return(TgaBadValue);
        }
        /*
         *   Sanitize bit depth if it is 24, then it should be 32.
         */
        if(depth == 24)
            depth = 32;


        /* ********************************************************** */

	/* Fetch header information for td. */
	status = TgaReadHeaderFromData(data, td);
	if(status != TgaSuccess)
	    return(status);


        /*
         *   Make sure the SOURCE data bit depth is supported by this
         *   function.
         */
        if((td->depth != 24) &&
           (td->depth != 32)
        )
        {
            sprintf(stringa,
                "Requested source file bit depth %i not supported.",
                td->depth
            );
            TgaReportError("Tga data", stringa, TgaErrorLevelCritical);
            return(TgaBadValue);
        }



	/* Get data pointer. */
	data_ptr = (u_int8_t *)data;


        /*  Set member data_depth since it represents the LOADED data
         *  in number of bytes per pixel IN MEMORY.   (Do not confuse this
         *  with bytes_per_pixel which indicates bytes per pixel on FILE.)
         */
        td->data_depth = depth; /* In bits per pixel */


        /* Calculate exact bytes per pixel. */
        switch(td->data_depth)
        {  
          case 24:
            bytes_per_pixel = BYTES_PER_PIXEL24;
            break;

          case 15:
            bytes_per_pixel = BYTES_PER_PIXEL15;
            break;
          
          default:
            bytes_per_pixel = (td->data_depth >> 3);
            break;
        }

	/* Allocate memory for image data. */
        td->data = (u_int8_t *)calloc(
	    1,
            td->width * td->height *
            bytes_per_pixel * sizeof(u_int8_t)
        );
        if(td->data == NULL)
            return(TgaNoBuffers);


        /* Skip header. */
	data_ptr += (TgaHeaderLength + td->id_field_len);
	fpos = TgaHeaderLength + td->id_field_len;


        /* *************************************************************** */

        /* Begin reading the image data from file. */

        /* Check which encoding style the image data on file is in. */
        if(td->descriptor & TgaDescBitFliped)
        {
            /* READ RIGHTSIDE UP. */

            pix_total = td->width * td->height;
            colum_count = 0;
            row_count = 0;
            data_pos = row_count * td->width * bytes_per_pixel;

            /* Check which bit padding to read to. */
            switch(td->data_depth)
            {
              /* 32 bits. */
              case 32:
                while((fpos < (int)td->file_size) &&
                      (row_count < (int)td->height)
                )   
                {
                    /* Get pointer in destination buffer. */
                    data_ptr32 = (u_int32_t *)(&td->data[data_pos]);

                    /* Get pixel color values depending on 
                     * bits_per_pixel ON FILE.
                     */
                    switch(td->bits_per_pixel)
                    {
                      case 32:
                        pix[0] = (u_int8_t)*data_ptr++;
                        pix[1] = (u_int8_t)*data_ptr++;
                        pix[2] = (u_int8_t)*data_ptr++;
                        pix[3] = (u_int8_t)*data_ptr++;
                        *data_ptr32 = PACK8TO32(pix[3], pix[2], pix[1], pix[0]);
                        break;

                      case 8:
                        pix[0] = (u_int8_t)*data_ptr++;
                        *data_ptr32 = PACK8TO32(0x00, pix[0], pix[0], pix[0]);
                        break;

                      default:  /* Default to 24-bits. */
                        pix[0] = (u_int8_t)*data_ptr++;
                        pix[1] = (u_int8_t)*data_ptr++;
                        pix[2] = (u_int8_t)*data_ptr++;
                        *data_ptr32 = PACK8TO32(0x00, pix[2], pix[1], pix[0]);
                        break;
                    }

                    /* Increment data position in destination buffer. */
                    data_pos += BYTES_PER_PIXEL32;

                    /* Increment colum_count. */
                    colum_count++;

                    if(colum_count >= (int)td->width)
                    {
                        colum_count = 0;
                        row_count++;
                        data_pos = row_count * (int)td->width * BYTES_PER_PIXEL32;
                    }
                }
                break;

              /* 16 bits. */
              case 16:
                while((fpos < (int)td->file_size) &&
                      (row_count < (int)td->height) 
                )
                {
                    /* Get pointer in destination buffer. */
                    data_ptr16 = (u_int16_t *)(&td->data[data_pos]);

                    /* Get pixel color values depending on 
                     * bits_per_pixel ON FILE.
                     */
                    switch(td->bits_per_pixel)
                    {
                      case 32:
                        pix[0] = (u_int8_t)*data_ptr++;
                        pix[1] = (u_int8_t)*data_ptr++;
                        pix[2] = (u_int8_t)*data_ptr++;
                        pix[3] = (u_int8_t)*data_ptr++;
                        *data_ptr16 = PACK8TO16(pix[2], pix[1], pix[0]);
                        break;

                      case 8:
                        pix[0] = (u_int8_t)*data_ptr++;
                        *data_ptr16 = PACK8TO16(pix[0], pix[0], pix[0]);
                        break;

                      default:  /* Default to 24-bits. */
                        pix[0] = (u_int8_t)*data_ptr++;
                        pix[1] = (u_int8_t)*data_ptr++;
                        pix[2] = (u_int8_t)*data_ptr++;
                        *data_ptr16 = PACK8TO16(pix[2], pix[1], pix[0]);
                        break;
                    }
                    /* `Almost' black pixel roundoff check. */
                    if(*data_ptr16 == 0x0000)
                    {
                        if(pix[0] || pix[1] || pix[2])
                            *data_ptr16 = ALMOSTBLACKPIX16;
                    }

                    /* Increment data position in destination buffer. */
                    data_pos += BYTES_PER_PIXEL16;


                    /* Increment colum_count. */
                    colum_count++;

                    if(colum_count >= (int)td->width)
                    {
                        colum_count = 0;
                        row_count++;
                        data_pos = row_count * (int)td->width * BYTES_PER_PIXEL16;
                    }
                }
                break;

              /* 15 bits. */
              case 15:
                while((fpos < (int)td->file_size) &&
                      (row_count < (int)td->height)
                )
                {
                    /* Get pointer in destination buffer. */
                    data_ptr16 = (u_int16_t *)(&td->data[data_pos]);

                    /* Get pixel color values depending on
                     * bits_per_pixel ON FILE.
                     */
                    switch(td->bits_per_pixel)
                    {
                      case 32:
                        pix[0] = (u_int8_t)*data_ptr++;
                        pix[1] = (u_int8_t)*data_ptr++;
                        pix[2] = (u_int8_t)*data_ptr++;
                        pix[3] = (u_int8_t)*data_ptr++;
                        *data_ptr16 = PACK8TO15(pix[2], pix[1], pix[0]);
                        break;

                      case 8:
                        pix[0] = (u_int8_t)*data_ptr++;
                        *data_ptr16 = PACK8TO15(pix[0], pix[0], pix[0]);
                        break;

                      default:  /* Default to 24-bits. */
                        pix[0] = (u_int8_t)*data_ptr++;
                        pix[1] = (u_int8_t)*data_ptr++;
                        pix[2] = (u_int8_t)*data_ptr++;
                        *data_ptr16 = PACK8TO15(pix[2], pix[1], pix[0]);
                        break;
                    }
                    /* `Almost' black pixel roundoff check. */
                    if(*data_ptr16 == 0x0000)
                    {
                        if(pix[0] || pix[1] || pix[2])
                            *data_ptr16 = ALMOSTBLACKPIX15;
                    }

                    /* Increment data position in destination buffer. */
                    data_pos += BYTES_PER_PIXEL15;

                    /* Increment colum_count. */
                    colum_count++;

                    if(colum_count >= (int)td->width)
                    {
                        colum_count = 0;
                        row_count++;
                        data_pos = row_count * (int)td->width * BYTES_PER_PIXEL15;
                    }
                }
                break;

              /* 8 bits. */
              case 8:
                while((fpos < (int)td->file_size) &&
                      (row_count < (int)td->height)
                )
                {
                    /* Get pointer in destination buffer. */
                    data_ptr8 = (u_int8_t *)(&td->data[data_pos]);

                    /* Get pixel color values depending on
                     * bits_per_pixel ON FILE.
                     */
                    switch(td->bits_per_pixel)
                    {
                      case 32:
                        r = (u_int8_t)*data_ptr++;
                        pix[0] = TgaDitherRedPixel8(r, colum_count, row_count);
                        g = (u_int8_t)*data_ptr++;
                        pix[1] = TgaDitherGreenPixel8(g, colum_count, row_count);
                        b = (u_int8_t)*data_ptr++;
                        pix[2] = TgaDitherBluePixel8(b, colum_count, row_count);
                        pix[3] = (u_int8_t)*data_ptr++;
                        *data_ptr8 = PACK8TO8(pix[2], pix[1], pix[0]);
                        break;

                      case 8:
                        pix[0] = (u_int8_t)*data_ptr++;
                        *data_ptr8 = PACK8TO8(pix[0], pix[0], pix[0]);
                        break;

                      default:  /* Default to 24-bits. */
                        r = (u_int8_t)*data_ptr++;
                        pix[0] = TgaDitherRedPixel8(r, colum_count, row_count);
                        g = (u_int8_t)*data_ptr++;
                        pix[1] = TgaDitherGreenPixel8(g, colum_count, row_count);
                        b = (u_int8_t)*data_ptr++;
                        pix[2] = TgaDitherBluePixel8(b, colum_count, row_count);
                        *data_ptr8 = PACK8TO8(pix[2], pix[1], pix[0]);
                        break;
                    }
                    /* `Almost' black pixel roundoff check. */
                    if(*data_ptr8 == 0x00)
                    {
                        if(pix[0] || pix[1] || pix[2])
                            *data_ptr8 = ALMOSTBLACKPIX8;
                    }

                    /* Increment data position in destination buffer. */
                    data_pos += BYTES_PER_PIXEL8;

                    /* Increment colum_count. */
                    colum_count++;

                    if(colum_count >= (int)td->width)
                    {
                        colum_count = 0;
                        row_count++;
                        data_pos = row_count * (int)td->width * BYTES_PER_PIXEL8;
                    }
                }
                break;

            }
        }
        else
        {
            /* READ UPSIDE DOWN. */

            pix_total = td->width * td->height;
            colum_count = 0;
            row_count = (int)td->height - 1;
            data_pos = row_count * td->width * bytes_per_pixel;

            /* Check which bit padding to read to. */
            switch(td->data_depth)
            {
              /* 32 bits. */
              case 32:
                while((fpos < (int)td->file_size) &&
                      (row_count >= 0)
                )
                {
                    /* Get pointer in destination buffer. */
                    data_ptr32 = (u_int32_t *)(&td->data[data_pos]);

                    /* Get pixel color values depending on
                     * bits_per_pixel ON FILE.
                     */
                    switch(td->bits_per_pixel)
                    { 
                      case 32:
                        pix[0] = (u_int8_t)*data_ptr++;
                        pix[1] = (u_int8_t)*data_ptr++;
                        pix[2] = (u_int8_t)*data_ptr++;
                        pix[3] = (u_int8_t)*data_ptr++;
                        *data_ptr32 = PACK8TO32(pix[3], pix[2], pix[1], pix[0]);
                        break;

                      case 8:
                        pix[0] = (u_int8_t)*data_ptr++;
                        *data_ptr32 = PACK8TO32(0x00, pix[0], pix[0], pix[0]);
                        break;

                      default:  /* Default to 24-bits. */
                        pix[0] = (u_int8_t)*data_ptr++;
                        pix[1] = (u_int8_t)*data_ptr++;
                        pix[2] = (u_int8_t)*data_ptr++;
                        *data_ptr32 = PACK8TO32(0x00, pix[2], pix[1], pix[0]);
                        break;
                    }

                    /* Increment data position in destination buffer. */
                    data_pos += BYTES_PER_PIXEL32;

                    /* Increment colum_count. */
                    colum_count++;

                    if(colum_count >= (int)td->width)
                    {
                        colum_count = 0;
                        row_count--;
                        data_pos = row_count * (int)td->width * BYTES_PER_PIXEL32;
                    }
                }
                break;

              /* 16 bits. */
              case 16:
                while((fpos < (int)td->file_size) &&
                      (row_count >= 0)
                )
                {       
                    /* Get pointer in destination buffer. */
                    data_ptr16 = (u_int16_t *)(&td->data[data_pos]);

                    /* Get pixel color values depending on
                     * bits_per_pixel ON FILE. 
                     */
                    switch(td->bits_per_pixel)
                    {
                      case 32:
                        pix[0] = (u_int8_t)*data_ptr++;
                        pix[1] = (u_int8_t)*data_ptr++;
                        pix[2] = (u_int8_t)*data_ptr++;
                        pix[3] = (u_int8_t)*data_ptr++;
                        *data_ptr16 = PACK8TO16(pix[2], pix[1], pix[0]);
                        break;

                      case 8:
                        pix[0] = (u_int8_t)*data_ptr++;
                        *data_ptr16 = PACK8TO16(pix[0], pix[0], pix[0]);
                        break;

                      default:  /* Default to 24-bits. */
                        pix[0] = (u_int8_t)*data_ptr++;
                        pix[1] = (u_int8_t)*data_ptr++;
                        pix[2] = (u_int8_t)*data_ptr++;
                        *data_ptr16 = PACK8TO16(pix[2], pix[1], pix[0]);
                        break;
                    }
                    /* `Almost' black pixel roundoff check. */
                    if(*data_ptr16 == 0x0000)
                    {
                        if(pix[0] || pix[1] || pix[2])
                            *data_ptr16 = ALMOSTBLACKPIX16;
                    }

                    /* Increment data position in destination buffer. */
                    data_pos += BYTES_PER_PIXEL16;

                    /* Increment colum_count. */
                    colum_count++;
                        
                    if(colum_count >= (int)td->width)
                    {
                        colum_count = 0;
                        row_count--;
                        data_pos = row_count * (int)td->width * BYTES_PER_PIXEL16;
                    }
                }   
                break;

              /* 15 bits. */  
              case 15:
                while((fpos < (int)td->file_size) &&   
                      (row_count >= 0)
                )
                {
                    /* Get pointer in destination buffer. */
                    data_ptr16 = (u_int16_t *)(&td->data[data_pos]);

                    /* Get pixel color values depending on
                     * bits_per_pixel ON FILE.
                     */
                    switch(td->bits_per_pixel)
                    {
                      case 32:
                        pix[0] = (u_int8_t)*data_ptr++;
                        pix[1] = (u_int8_t)*data_ptr++;
                        pix[2] = (u_int8_t)*data_ptr++;
                        pix[3] = (u_int8_t)*data_ptr++;
                        *data_ptr16 = PACK8TO15(pix[2], pix[1], pix[0]);
                        break;

                      case 8:
                        pix[0] = (u_int8_t)*data_ptr++;
                        *data_ptr16 = PACK8TO15(pix[0], pix[0], pix[0]);
                        break;

                      default:  /* Default to 24-bits. */
                        pix[0] = (u_int8_t)*data_ptr++;
                        pix[1] = (u_int8_t)*data_ptr++;
                        pix[2] = (u_int8_t)*data_ptr++;
                        *data_ptr16 = PACK8TO15(pix[2], pix[1], pix[0]);
                        break;
                    }
                    /* `Almost' black pixel roundoff check. */
                    if(*data_ptr16 == 0x0000)
                    {
                        if(pix[0] || pix[1] || pix[2])
                            *data_ptr16 = ALMOSTBLACKPIX15;
                    }

                    /* Increment data position in destination buffer. */
                    data_pos += BYTES_PER_PIXEL15;

                    /* Increment colum_count. */
                    colum_count++;

                    if(colum_count >= (int)td->width)
                    {
                        colum_count = 0;
                        row_count--;
                        data_pos = row_count * (int)td->width * BYTES_PER_PIXEL15;
                    }
                }
                break;

              /* 8 bits. */
              case 8:
                while((fpos < (int)td->file_size) &&
                      (row_count >= 0)
                )
                {
                    /* Get pointer in destination buffer. */
                    data_ptr8 = (u_int8_t *)(&td->data[data_pos]);

                    /*   Get pixel color values depending on
                     *   bits_per_pixel ON FILE.
                     */
                    switch(td->bits_per_pixel)
                    {
                      case 32:
                        r = (u_int8_t)*data_ptr++;
                        pix[0] = TgaDitherRedPixel8(r, colum_count, row_count);
                        g = (u_int8_t)*data_ptr++;
                        pix[1] = TgaDitherGreenPixel8(g, colum_count, row_count);
                        b = (u_int8_t)*data_ptr++;
                        pix[2] = TgaDitherBluePixel8(b, colum_count, row_count);
                        pix[3] = (u_int8_t)*data_ptr++;
                        *data_ptr8 = PACK8TO8(pix[2], pix[1], pix[0]);
                        break;

                      case 8:
                        pix[0] = (u_int8_t)*data_ptr++;
                        *data_ptr8 = PACK8TO8(pix[0], pix[0], pix[0]);
                        break;

                      default:  /* Default to 24-bits. */
                        r = (u_int8_t)*data_ptr++;
                        pix[0] = TgaDitherRedPixel8(r, colum_count, row_count);
                        g = (u_int8_t)*data_ptr++;
                        pix[1] = TgaDitherGreenPixel8(g, colum_count, row_count);
                        b = (u_int8_t)*data_ptr++;
                        pix[2] = TgaDitherBluePixel8(b, colum_count, row_count);
                        *data_ptr8 = PACK8TO8(pix[2], pix[1], pix[0]);
                        break;
                    }
                    /* `Almost' black pixel roundoff check. */
                    if(*data_ptr8 == 0x00)
                    {
                        if(pix[0] || pix[1] || pix[2])
                            *data_ptr8 = ALMOSTBLACKPIX8;
                    }

                    /* Increment data position in destination buffer. */
                    data_pos += BYTES_PER_PIXEL8;


                    /* Increment colum_count. */
                    colum_count++;
                       
                    if(colum_count >= (int)td->width)
                    {
                        colum_count = 0;
                        row_count--;
                        data_pos = row_count * (int)td->width * BYTES_PER_PIXEL8;
                    }
                }
                break; 

            }
        }


	return(TgaSuccess);
}


/*
 *	Sets up td to begin reading tga file filename.
 *
 *	Once this call has been made, you may use
 *	TgaReadPartialFromFile() to begin reading the data.
 *
 *      depth is the bit depth of the destination data format.  Valid
 *      depth values are 8, 16, 24, or 32.  Note that passing 24 will
 *      be the same as 32.
 */
int TgaStartReadPartialFromFile(
        const char *filename,
        tga_data_struct *td,
        unsigned int depth
)
{
	int status;
	unsigned int bytes_per_pixel;
        char stringa[256];


        /* Make sure filename and td are valid. */
        if((filename == NULL) ||
           (td == NULL)
        )
            return(TgaBadValue);

        /*   Make sure the GIVEN depth for the DESTINATION buffer is one
         *   that is supported.
         */
        if((depth != 8) &&
           (depth != 15) &&
           (depth != 16) &&
           (depth != 24) &&
           (depth != 32)
        )
        {
            sprintf(stringa,
                "Requested destination buffer depth %i is not supported.",
                depth
            );
            TgaReportError(filename, stringa, TgaErrorLevelCritical);
            return(TgaBadValue);
        }
        /*
         *   Sanitize GIVEN depth if it is 24, then it should be 32.
         */
        if(depth == 24)
            depth = 32;


        /* ********************************************************* */

        /* Fetch header information for td. */
        status = TgaReadHeaderFromFile(filename, td);
        if(status != TgaSuccess)
            return(status);

        /* Open the file. */
        td->fp = FOpen(filename, "rb");
        if(td->fp == NULL)
            return(TgaNoAccess);


        /*
         *   Make sure the SOURCE file bit depth is supported by this
         *   function.
         */
        if((td->depth != 24) &&
           (td->depth != 32)
        )
        {
            sprintf(stringa,
                "Image file depth %i is not supported.",
                td->depth
            );
            TgaReportError(filename, stringa, TgaErrorLevelCritical);
            return(TgaBadValue);
        }


        /*  Set member data_depth since it represents the LOADED data
         *  in number of bytes per pixel IN MEMORY.   (Do not confuse this
         *  with bytes_per_pixel which indicates bytes per pixel on FILE.)
         */
        td->data_depth = depth; /* In bits per pixel */


        /* Calculate exact bytes per pixel. */
        switch(td->data_depth)
        {  
          case 24:
            bytes_per_pixel = BYTES_PER_PIXEL24;
            break;

          case 15:
            bytes_per_pixel = BYTES_PER_PIXEL15;
            break;

          default:
            bytes_per_pixel = (td->data_depth >> 3);
            break;
        }
	/* Allocate memory for image data. */
        td->data = (u_int8_t *)calloc(
	    1,
            td->width * td->height *
            bytes_per_pixel * sizeof(u_int8_t)
        );
        if(td->data == NULL)
            return(TgaNoBuffers);

	/* Set the loaded pixel bookmark to 0. */
	td->cur_load_pixel = 0;


        return(TgaSuccess);
}


/*
 *	Reads a specific number of pixels from file.
 *
 *	The td must be initialized prior by a call to
 *	TgaStartReadPartialFromFile().
 */
int TgaReadPartialFromFile(
	tga_data_struct *td,
	unsigned int depth,
	unsigned int n_pixels
)
{
	char stringa[256];
	FILE *fp;
	int f_start = 0;	/* Byte # on file to start reading at. */
	int f_end = 0;	        /* Byte # on file to stop reading at.
                                 * (do not read byte f_end either).
			         */

	int fpos;
        int data_pos;
	unsigned int bytes_per_pixel;

        u_int8_t *data_ptr8;
        u_int16_t *data_ptr16;
        u_int32_t *data_ptr32;

        u_int8_t pix[4], r, g, b;

        int pix_total;
        int colum_count, row_count;



	if(td == NULL)
	    return(TgaBadValue);

	if((td->width == 0) ||
           (td->height == 0)
	)
	    return(TgaBadValue);

	/* Is td finished loading? */
	if((td->cur_load_pixel < 0) ||
           (td->fp == NULL)
	)
	    return(TgaSuccess);

	/* Nothing to load? */
	if(n_pixels == 0)
	    return(TgaSuccess);

	/* Buffer not allocated on td? */
	if(td->data == NULL)
	    return(TgaNoBuffers);


        /*   Make sure the DESTINATION depth for the DESTINATION buffer is
         *   one that is supported.
         */
        if((depth != 8) &&
           (depth != 15) &&
           (depth != 16) &&
           (depth != 24) &&
           (depth != 32)
        )
        {
            sprintf(stringa,
                "Requested destination buffer depth %i is not supported.",
                depth
            );
            TgaReportError("loaded segment", stringa, TgaErrorLevelCritical);
            return(TgaBadValue);
        }
        /*
         *   Sanitize DESTINATION depth if it is 24, then it should be 32.
         */
        if(depth == 24)
            depth = 32;

	/* Calculate exact bytes per pixel (in DESTINATION). */
        switch(td->data_depth)
        {
          case 24:	/* Never reached. */
            bytes_per_pixel = BYTES_PER_PIXEL24;
            break;
          
          case 15:
            bytes_per_pixel = BYTES_PER_PIXEL15;
            break;
        
          default:
            bytes_per_pixel = (td->data_depth >> 3);
            break;
        }


        /*
         *   Make sure the SOURCE file bit depth is supported by this
         *   function.
         */
        if((td->depth != 24) &&
           (td->depth != 32)
        )
        {
            sprintf(stringa,
                "Image file depth %i is not supported.",
                td->depth
            );
            TgaReportError("loaded segment", stringa, TgaErrorLevelCritical);
            return(TgaBadValue);
        }


        /* ****************************************************** */

	/* Adjust n_pixels so that we ensure we read atlest
	 * 2 lines.
	 */
	if(n_pixels < (unsigned int)(td->width * 2))
	    n_pixels = (unsigned int)(td->width * 2);

	/* Calculate file positions (we know SOURCE depth is valid). */
	if(td->depth == 24)
	{
	    f_start = (TgaHeaderLength + td->id_field_len) +
		(td->cur_load_pixel * 3);
            f_end = f_start + (n_pixels * 3);
	}
	else if(td->depth == 32)
        {
            f_start = (TgaHeaderLength + td->id_field_len) +
		(td->cur_load_pixel * 4);
	    f_end = f_start + (n_pixels * 4);
        }
	fpos = f_start;

	fp = td->fp;

	/* Move fp to starting position. */
	fseek(fp, f_start, 0);


        /* ************************************************************ */
        /* Begin reading the image data from file. */

        /* Check which encoding style the image data on file is in. */
        if(td->descriptor & TgaDescBitFliped)
        {
            /* READ RIGHTSIDE UP. */

            pix_total = td->width * td->height;
            colum_count = (int)td->cur_load_pixel % (int)td->width;
            row_count = (int)td->cur_load_pixel / (int)td->width;
            data_pos = ((row_count * (int)td->width) + colum_count) *
                       bytes_per_pixel;

            /* Read by DESTINATION buffer depth. */
            switch(td->data_depth)
            {
              /* 32 bits. */
              case 32:
                while((fpos < (int)td->file_size) &&
                      (row_count < (int)td->height) &&
                      (fpos < f_end)	/* Read limited number of bytes. */
                )
                {
                    /* Get pointer in destination buffer. */
                    data_ptr32 = (u_int32_t *)(&td->data[data_pos]);

                    /*   Get pixel color values depending on 
                     *   bits_per_pixel ON FILE.
                     */
                    switch(td->bits_per_pixel)
                    {
                      case 32:
                        pix[0] = (u_int8_t)fgetc(fp);
                        pix[1] = (u_int8_t)fgetc(fp);
                        pix[2] = (u_int8_t)fgetc(fp);
                        pix[3] = (u_int8_t)fgetc(fp);
                        *data_ptr32 = PACK8TO32(pix[3], pix[2], pix[1], pix[0]);
                        fpos += 4;
                        break;

                      default:  /* Default to 24-bits. */
                        pix[0] = (u_int8_t)fgetc(fp);
                        pix[1] = (u_int8_t)fgetc(fp);
                        pix[2] = (u_int8_t)fgetc(fp);
                        *data_ptr32 = PACK8TO32(0x00, pix[2], pix[1], pix[0]);
                        fpos += 3;
                        break;
                    }

                    /* Increment data position in destination buffer. */
                    data_pos += BYTES_PER_PIXEL32;

                    /* Increment colum_count. */
                    colum_count++;

                    if(colum_count >= (int)td->width)
                    {
                        colum_count = 0;
                        row_count++;
                        data_pos = row_count * td->width * BYTES_PER_PIXEL32;
                    }
                }
                break;

              /* 16 bits. */
              case 16:
                while((fpos < (int)td->file_size) &&
                      (row_count < (int)td->height) &&
                      (fpos < f_end)    /* Read limited number of bytes. */
                )
                {
                    /* Get pointer in destination buffer. */
                    data_ptr16 = (u_int16_t *)(&td->data[data_pos]);

                    /*   Get pixel color values depending on 
                     *   bits_per_pixel ON FILE.
                     */
                    switch(td->bits_per_pixel)
                    {
                      case 32:
                        pix[0] = (u_int8_t)fgetc(fp);
                        pix[1] = (u_int8_t)fgetc(fp);
                        pix[2] = (u_int8_t)fgetc(fp);
                        pix[3] = (u_int8_t)fgetc(fp);
                        *data_ptr16 = PACK8TO16(pix[2], pix[1], pix[0]);
                        fpos += 4;
                        break;

                      default:  /* Default to 24-bits. */
                        pix[0] = (u_int8_t)fgetc(fp);
                        pix[1] = (u_int8_t)fgetc(fp);
                        pix[2] = (u_int8_t)fgetc(fp);
                        *data_ptr16 = PACK8TO16(pix[2], pix[1], pix[0]);
                        fpos += 3;
                        break;
                    }
                    /* `Almost' black pixel roundoff check. */
                    if(*data_ptr16 == 0x0000)
                    {
                        if(pix[0] || pix[1] || pix[2])
                            *data_ptr16 = ALMOSTBLACKPIX16;
                    }

                    /* Increment data position in destination buffer. */
                    data_pos += BYTES_PER_PIXEL16;

                    /* Increment colum_count. */
                    colum_count++;

                    if(colum_count >= (int)td->width)
                    {
                        colum_count = 0;
                        row_count++;
                        data_pos = row_count * td->width * BYTES_PER_PIXEL16;
                    }
                }
                break;

	      /* 15 bits. */
              case 15:
                while((fpos < (int)td->file_size) &&
                      (row_count < (int)td->height) &&
                      (fpos < f_end)    /* Read limited number of bytes. */
                )       
                {
                    /* Get pointer in destination buffer. */
                    data_ptr16 = (u_int16_t *)(&td->data[data_pos]);

                    /*   Get pixel color values depending on
                     *   bits_per_pixel ON FILE.
                     */
                    switch(td->bits_per_pixel)
                    {
                      case 32:
                        pix[0] = (u_int8_t)fgetc(fp);
                        pix[1] = (u_int8_t)fgetc(fp);
                        pix[2] = (u_int8_t)fgetc(fp);
                        pix[3] = (u_int8_t)fgetc(fp);
                        *data_ptr16 = PACK8TO15(pix[2], pix[1], pix[0]);
                        fpos += 4;
                        break;

                      default:  /* Default to 24-bits. */
                        pix[0] = (u_int8_t)fgetc(fp);
                        pix[1] = (u_int8_t)fgetc(fp);
                        pix[2] = (u_int8_t)fgetc(fp);
                        *data_ptr16 = PACK8TO15(pix[2], pix[1], pix[0]);
                        fpos += 3;
                        break;
                    }
                    /* `Almost' black pixel roundoff check. */
                    if(*data_ptr16 == 0x0000)
                    {
                        if(pix[0] || pix[1] || pix[2])
                            *data_ptr16 = ALMOSTBLACKPIX15;
                    }

                    /* Increment data position in destination buffer. */
                    data_pos += BYTES_PER_PIXEL15;

                    /* Increment colum_count. */
                    colum_count++;

                    if(colum_count >= (int)td->width)
                    {
                        colum_count = 0;
                        row_count++;
                        data_pos = row_count * td->width * BYTES_PER_PIXEL15;
                    }
                }
                break;

              /* 8 bits. */
              case 8:
                while((fpos < (int)td->file_size) &&
                      (row_count < (int)td->height) &&
                      (fpos < f_end)    /* Read limited number of bytes. */
                )
                {
                    /* Get pointer in destination buffer. */
                    data_ptr8 = (u_int8_t *)(&td->data[data_pos]);

                    /*   Get pixel color values depending on
                     *   bits_per_pixel ON FILE.
                     */
                    switch(td->bits_per_pixel)
                    {
                      case 32:
                        r = (u_int8_t)fgetc(fp);
                        pix[0] = TgaDitherRedPixel8(r, colum_count, row_count);
                        g = (u_int8_t)fgetc(fp);
                        pix[1] = TgaDitherGreenPixel8(g, colum_count, row_count);
                        b = (u_int8_t)fgetc(fp);
                        pix[2] = TgaDitherBluePixel8(b, colum_count, row_count);
                        pix[3] = (u_int8_t)fgetc(fp);
                        *data_ptr8 = PACK8TO8(pix[2], pix[1], pix[0]);
                        fpos += 4;
                        break;

                      default:  /* Default to 24-bits. */
                        r = (u_int8_t)fgetc(fp);
                        pix[0] = TgaDitherRedPixel8(r, colum_count, row_count);
                        g = (u_int8_t)fgetc(fp);
                        pix[1] = TgaDitherGreenPixel8(g, colum_count, row_count);
                        b = (u_int8_t)fgetc(fp);
                        pix[2] = TgaDitherBluePixel8(b, colum_count, row_count);
                        *data_ptr8 = PACK8TO8(pix[2], pix[1], pix[0]);
			fpos += 3;
                        break;
                    }
                    /* `Almost' black pixel roundoff check. */
                    if(*data_ptr8 == 0x00)
                    {
                        if(pix[0] || pix[1] || pix[2])
                            *data_ptr8 = ALMOSTBLACKPIX8;
                    }

                    /* Increment data position in destination buffer. */
                    data_pos += BYTES_PER_PIXEL8;

                    /* Increment colum_count. */
                    colum_count++;

                    if(colum_count >= (int)td->width)
                    {
                        colum_count = 0;
                        row_count++;
                        data_pos = row_count * td->width * BYTES_PER_PIXEL8;
                    }
                }
                break;
            }
        }
        else
        {
            /* READ UPSIDE DOWN. */

            pix_total = td->width * td->height;
            colum_count = (int)td->cur_load_pixel % (int)td->width;
            row_count = MAX(
		(int)td->height - 1 -
                ((int)td->cur_load_pixel / (int)td->width),
		0
	    );
	    /* Go one line down to make sure line gets loaded. */
	    row_count = MIN(row_count, ((int)td->height - 1));

            data_pos = ((row_count * (int)td->width) + colum_count) *
                       bytes_per_pixel;

            /* Read by DESTINATION buffer depth. */
            switch(td->data_depth)
            {
              /* 32 bits. */
              case 32:
                while((fpos < (int)td->file_size) &&
                      (row_count >= 0) &&
                      (fpos < f_end)    /* Read limited number of bytes. */
                )
                {
                    /* Get pointer in destination buffer. */
                    data_ptr32 = (u_int32_t *)(&td->data[data_pos]);

                    /*   Get pixel color values depending on
                     *   bits_per_pixel ON FILE.
                     */
                    switch(td->bits_per_pixel)
                    { 
                      case 32:
                        pix[0] = (u_int8_t)fgetc(fp);
                        pix[1] = (u_int8_t)fgetc(fp);
                        pix[2] = (u_int8_t)fgetc(fp);
                        pix[3] = (u_int8_t)fgetc(fp);
                        *data_ptr32 = PACK8TO32(pix[3], pix[2], pix[1], pix[0]);
                        fpos += 4;
                        break;

                      default:  /* Default to 24-bits. */
                        pix[0] = (u_int8_t)fgetc(fp);
                        pix[1] = (u_int8_t)fgetc(fp);
                        pix[2] = (u_int8_t)fgetc(fp);
                        *data_ptr32 = PACK8TO32(0x00, pix[2], pix[1], pix[0]);
                        fpos += 3;
                        break;
                    }

                    /* Increment data position in destination buffer. */
                    data_pos += BYTES_PER_PIXEL32;

                    /* Increment colum_count. */
                    colum_count++;

                    if(colum_count >= (int)td->width)
                    {
                        colum_count = 0;
                        row_count--;
                        data_pos = row_count * td->width * BYTES_PER_PIXEL32;
                    }
                }
                break;

              /* 16 bits. */
              case 16:
                while((fpos < td->file_size) &&
                      (row_count >= 0) &&
                      (fpos < f_end)    /* Read limited number of bytes. */
                )
                {
                    /* Get pointer in destination buffer. */
                    data_ptr16 = (u_int16_t *)(&td->data[data_pos]);

                    /*   Get pixel color values depending on
                     *   bits_per_pixel ON FILE. 
                     */
                    switch(td->bits_per_pixel)
                    {
                      case 32:
                        pix[0] = (u_int8_t)fgetc(fp);
                        pix[1] = (u_int8_t)fgetc(fp);
                        pix[2] = (u_int8_t)fgetc(fp);
                        pix[3] = (u_int8_t)fgetc(fp);
                        *data_ptr16 = PACK8TO16(pix[2], pix[1], pix[0]);
                        fpos += 4;
                        break;
                    
                      default:  /* Default to 24-bits. */
                        pix[0] = (u_int8_t)fgetc(fp);
                        pix[1] = (u_int8_t)fgetc(fp);
                        pix[2] = (u_int8_t)fgetc(fp);
                        *data_ptr16 = PACK8TO16(pix[2], pix[1], pix[0]);
                        fpos += 3;
                        break;
                    } 
                    /* `Almost' black pixel roundoff check. */
                    if(*data_ptr16 == 0x0000)
                    {
                        if(pix[0] || pix[1] || pix[2])
                            *data_ptr16 = ALMOSTBLACKPIX16;
                    }

                    /* Increment data position in destination buffer. */
                    data_pos += BYTES_PER_PIXEL16;

                    /* Increment colum_count. */
                    colum_count++;

                    if(colum_count >= (int)td->width)
                    {
                        colum_count = 0;
                        row_count--;
                        data_pos = row_count * td->width * BYTES_PER_PIXEL16;
                    }
                }
                break;

              /* 15 bits. */
              case 15:
                while((fpos < td->file_size) &&
                      (row_count >= 0) &&
                      (fpos < f_end)    /* Read limited number of bytes. */
                )
                {
                    /* Get pointer in destination buffer. */
                    data_ptr16 = (u_int16_t *)(&td->data[data_pos]);

                    /*   Get pixel color values depending on
                     *   bits_per_pixel ON FILE.
                     */
                    switch(td->bits_per_pixel)
                    {
                      case 32:
                        pix[0] = (u_int8_t)fgetc(fp);
                        pix[1] = (u_int8_t)fgetc(fp);
                        pix[2] = (u_int8_t)fgetc(fp);
                        pix[3] = (u_int8_t)fgetc(fp);
                        *data_ptr16 = PACK8TO15(pix[2], pix[1], pix[0]);
                        fpos += 4;
                        break;
                        
                      default:  /* Default to 24-bits. */
                        pix[0] = (u_int8_t)fgetc(fp);
                        pix[1] = (u_int8_t)fgetc(fp);
                        pix[2] = (u_int8_t)fgetc(fp);
                        *data_ptr16 = PACK8TO15(pix[2], pix[1], pix[0]);
                        fpos += 3;
                        break;
                    }
                    /* `Almost' black pixel roundoff check. */
                    if(*data_ptr16 == 0x0000)
                    {
                        if(pix[0] || pix[1] || pix[2])
                            *data_ptr16 = ALMOSTBLACKPIX15;
                    }

                    /* Increment data position in destination buffer. */
                    data_pos += BYTES_PER_PIXEL15;

                    /* Increment colum_count. */
                    colum_count++;

                    if(colum_count >= (int)td->width)
                    {
                        colum_count = 0;
                        row_count--;
                        data_pos = row_count * td->width * BYTES_PER_PIXEL15;
                    }
                }
                break;

              /* 8 bits. */
              case 8:
                while((fpos < (int)td->file_size) &&
                      (row_count >= 0) &&
                      (fpos < f_end)    /* Read limited number of bytes. */
                )
                {
                    /* Get pointer in destination buffer. */
                    data_ptr8 = (u_int8_t *)(&td->data[data_pos]);
                        
                    /*   Get pixel color values depending on
                     *   bits_per_pixel ON FILE.
                     */
                    switch(td->bits_per_pixel)
                    {
                      case 32:
                        r = (u_int8_t)fgetc(fp);
                        pix[0] = TgaDitherRedPixel8(r, colum_count, row_count);
                        g = (u_int8_t)fgetc(fp);
                        pix[1] = TgaDitherGreenPixel8(g, colum_count, row_count);
                        b = (u_int8_t)fgetc(fp);
                        pix[2] = TgaDitherBluePixel8(b, colum_count, row_count);
                        pix[3] = (u_int8_t)fgetc(fp);
                        *data_ptr8 = PACK8TO8(pix[2], pix[1], pix[0]);
                        fpos += 4;
                        break;

                      default:  /* Default to 24-bits. */
                        r = (u_int8_t)fgetc(fp);
                        pix[0] = TgaDitherRedPixel8(r, colum_count, row_count);
                        g = (u_int8_t)fgetc(fp);
                        pix[1] = TgaDitherGreenPixel8(g, colum_count, row_count);
                        b = (u_int8_t)fgetc(fp);
                        pix[2] = TgaDitherBluePixel8(b, colum_count, row_count);
                        *data_ptr8 = PACK8TO8(pix[2], pix[1], pix[0]);
                        fpos += 3;
                        break;
                    }
                    /* `Almost' black pixel roundoff check. */
                    if(*data_ptr8 == 0x00)
                    {
                        if(pix[0] || pix[1] || pix[2])
                            *data_ptr8 = ALMOSTBLACKPIX8;
                    }

                    /* Increment data position in destination buffer. */
                    data_pos += BYTES_PER_PIXEL8;

                    /* Increment colum_count. */
                    colum_count++;

                    if(colum_count >= (int)td->width)
                    {
                        colum_count = 0;
                        row_count--;
                        data_pos = row_count * td->width * BYTES_PER_PIXEL8;
                    }
                }
                break;
            }
        }


	/* Update cur_load_pixel. */
	td->cur_load_pixel += n_pixels;

	/* Done loading? */
	if((int)(td->cur_load_pixel >= (int)(td->width * td->height)) ||
	   (fpos >= (int)td->file_size)
	)
	{
	    td->cur_load_pixel = -1;

	    /* Close the file. */
            if(td->fp != NULL)
            {
                FClose(td->fp);
                td->fp = NULL;
            }
	}

	return(TgaSuccess);
}


/*
 *	Writes the tga image data contained in td to
 *	the file filename at the target depth specified.
 */
int TgaWriteToFile(
	const char *filename,
	tga_data_struct *td,
	unsigned int depth
)
{
	int i, fpos, total_pixels;
	FILE *fp;
	u_int8_t descriptor;

	u_int8_t *img_ptr8;
	u_int16_t *img_ptr16;
	u_int32_t *img_ptr32;


	/* Error checks. */
	if((filename == NULL) ||
           (td == NULL)
	)
	    return(TgaBadValue);

	if((*filename == '\0') ||
           (td->data == NULL)
	)
	    return(TgaBadValue);


	/* Make sure target depth is supported. */
	if((depth != 24) &&
           (depth != 32)
	)
	    return(TgaBadValue);


	/* Create or truncate file for writing. */
	fp = FOpen(filename, "wb");
	if(fp == NULL)
	    return(TgaNoAccess);


	/* Write header. */
	for(fpos = 0; fpos < (TgaHeaderLength + td->id_field_len); fpos++)
	{
	    /* ID Field length. */
	    if(fpos == 0x00)
	    {
		fputc(td->id_field_len, fp);
	    }
	    /* Image type code. */
	    else if(fpos == 0x02)
	    {
	        fputc(TgaDataTypeURGB, fp);
	    }
            /* Width. */
            else if(fpos == 0x0c)
            {
		fputc(
		    ((u_int32_t)td->width & 0xff),
		    fp
		);

		fpos++;
		if(fpos < (TgaHeaderLength + td->id_field_len))
		    fputc(
		        (((u_int32_t)td->width & 0xff00) >> 8),
			fp
		    );
            }
            /* Height. */
            else if(fpos == 0x0e)
            {
                fputc(
                    ((u_int32_t)td->height & 0xff),
                    fp
                );

                fpos++;
                if(fpos < (TgaHeaderLength + td->id_field_len))
                    fputc(
                        (((u_int32_t)td->height & 0xff00) >> 8),
                        fp
                    );
            }
	    /* Depth. */
	    else if(fpos == 0x10)
	    {
		/* Write target depth. */
		fputc((u_int8_t)depth, fp);
	    }
            /* Attributes. */
            else if(fpos == 0x11)
            {
		/* Not flipped. */
		descriptor = TgaDescBitFliped;

                fputc(descriptor, fp);
            }         

	    else
	    {
		fputc(0x00, fp);
	    }
	}


	/* Begin writing image data, check SOURCE depth. */
	total_pixels = td->width * td->height;
	switch(td->data_depth)
	{
	  case 8:
	    for(i = 0, img_ptr8 = (u_int8_t *)td->data;
                i < total_pixels;
                i++, img_ptr8++
	    )
	    {
		/* Check TARGET depth. */
		switch(depth)
		{
                  case 32:
                    /* Blue. */
                    fputc(
                        (((*img_ptr8) & 0x03) << 6),
                        fp
                    );
                    /* Green. */
                    fputc(
                        (((*img_ptr8) & 0x1c) << 3),
                        fp
                    );
                    /* Red. */
                    fputc(
                        (((*img_ptr8) & 0xe0)),
                        fp
                    );
                    /* Alpha. */
                    fputc(0x00, fp);
		    break;

		  default:	/* Default to 24 bits. */
		    /* Blue. */
                    fputc(
                        (((*img_ptr8) & 0x03) << 6),
                        fp
                    );
		    /* Green. */
                    fputc(
                        (((*img_ptr8) & 0x1c) << 3),
                        fp
                    );
		    /* Red. */
		    fputc(
			(((*img_ptr8) & 0xe0)),
			fp
		    );
		    break;
		}
	    }
	    break;

	  case 15:
            for(i = 0, img_ptr16 = (u_int16_t *)td->data;
                i < total_pixels;
                i++, img_ptr16++
            )
            {
                /* Check TARGET depth. */
                switch(depth)
                {
                  case 32:
                    /* Blue. */
                    fputc(
                        (((*img_ptr16) & 0x001f) << 3),
                        fp
                    );
                    /* Green. */
                    fputc(
                        (((*img_ptr16) & 0x03e0) >> 2),
                        fp
                    );
                    /* Red. */
                    fputc(
                        (((*img_ptr16) & 0x7c00) >> 7),
                        fp
                    );
                    /* Alpha. */
                    fputc(
                        (((*img_ptr16) & 0x8000) >> 15),
                        fp
                    );
                    break;

                  default:      /* Default to 24 bits. */
                    /* Blue. */
                    fputc(
                         (((*img_ptr16) & 0x001f) << 3),
                        fp
                    );
                    /* Green. */
                    fputc(
                        (((*img_ptr16) & 0x03e0) >> 2),
                        fp
                    );
                    /* Red. */
                    fputc(
                        (((*img_ptr16) & 0x7c00) >> 7),
                        fp
                    );
                    break;
                }
            }
            break;

          case 16:
            for(i = 0, img_ptr16 = (u_int16_t *)td->data;
                i < total_pixels;
                i++, img_ptr16++
            )
            {
                /* Check TARGET depth. */
                switch(depth)
                {
                  case 32:
                    /* Blue. */
                    fputc(
                        (((*img_ptr16) & 0x001f) << 3),
                        fp
                    );
                    /* Green. */
                    fputc(
                        (((*img_ptr16) & 0x07e0) >> 3),
                        fp
                    );
                    /* Red. */
                    fputc(
                        (((*img_ptr16) & 0xf800) >> 8),
                        fp
                    );
                    /* Alpha. */
                    fputc(0x00, fp);
                    break;

                  default:      /* Default to 24 bits. */
                    /* Blue. */
                    fputc(
                        (((*img_ptr16) & 0x001f) << 3),
                        fp
                    );
                    /* Green. */
                    fputc(
                        (((*img_ptr16) & 0x07e0) >> 3),
                        fp
                    );
                    /* Red. */
                    fputc(
                        (((*img_ptr16) & 0xf800) >> 8),
                        fp
                    );
                    break;
                }
            }
            break;

          case 24:
            for(i = 0, img_ptr32 = (u_int32_t *)td->data;
                i < total_pixels;
                i++, img_ptr32++
            )
            {
                /* Check TARGET depth. */
                switch(depth) 
                {
                  case 32:
                    /* Blue. */
                    fputc(
                        (((*img_ptr32) & 0x000000ff)),
                        fp
                    );
                    /* Green. */
                    fputc(
                        (((*img_ptr32) & 0x0000ff00) >> 8),
                        fp
                    );
                    /* Red. */
                    fputc(
                        (((*img_ptr32) & 0x00ff0000) >> 16),
                        fp
                    );
                    /* Alpha. */
                    fputc(0x00, fp);
                    break;
                    
                  default:      /* Default to 24 bits. */
                    /* Blue. */
                    fputc(
                        (((*img_ptr32) & 0x000000ff)),
                        fp
                    );
                    /* Green. */
                    fputc(
                        (((*img_ptr32) & 0x0000ff00) >> 8),
                        fp
                    );
                    /* Red. */
                    fputc(
                        (((*img_ptr32) & 0x00ff0000) >> 16),
                        fp
                    );
                    break;
                }
            }
            break;  

          case 32:
            for(i = 0, img_ptr32 = (u_int32_t *)td->data;
                i < total_pixels;
                i++, img_ptr32++
            )
            {
                /* Check TARGET depth. */
                switch(depth)   
                {
                  case 32:
                    /* Blue. */
                    fputc(
                        (((*img_ptr32) & 0x000000ff)),
                        fp
                    );
                    /* Green. */
                    fputc(
                        (((*img_ptr32) & 0x0000ff00) >> 8),
                        fp
                    );
                    /* Red. */
                    fputc(
                        (((*img_ptr32) & 0x00ff0000) >> 16),
                        fp
                    );
                    /* Alpha. */
                    fputc(
                        (((*img_ptr32) & 0xff000000) >> 24),
                        fp
                    );
                    break;

                  default:      /* Default to 24 bits. */
                    /* Blue. */
                    fputc(
                        (((*img_ptr32) & 0x000000ff)),
                        fp
                    );
                    /* Green. */ 
                    fputc(
                        (((*img_ptr32) & 0x0000ff00) >> 8),
                        fp
                    );
                    /* Red. */
                    fputc(
                        (((*img_ptr32) & 0x00ff0000) >> 16),
                        fp
                    );
                    break;
                }
            }
            break;
	}


	/* Close file. */
	FClose(fp);
	fp = NULL;


        return(TgaSuccess);
}


/*
 *	Frees all allocated resources in td and resets all its values.
 */
void TgaDestroyData(tga_data_struct *td)
{
	if(td == NULL)
	    return;


	if(td->fp != NULL)
	{
	    FClose(td->fp);
	    td->fp = NULL;
	}

	td->id_field_len = 0;
	td->cmap_type = 0;
	td->img_type = 0;

	td->cmap_origin = 0;
	td->cmap_length = 0;
	td->cmap_size = 0;

        td->x = 0;
        td->y = 0;
        td->width = 0;
	td->height = 0;
	td->depth = 0;

	td->descriptor = 0;

	td->bits_per_pixel = 0;


	/* Stats. */
        td->file_size = 0;  
        td->data_size = 0;  
	td->cur_load_pixel = -1;		/* Do not load. */


	/* Loaded data. */ 
        free(td->header_data);
	td->header_data = NULL;
        free(td->data);
	td->data = NULL;

	td->data_depth = 0;


	return;
}



/*
 *	Checks if the file is really a Targa file,
 *	returns TgaSuccess if it is.
 */
int TgaTestFile(const char *filename)
{
	int status;
	tga_data_struct td;


	/* Read data. */
	status = TgaReadHeaderFromFile(filename, &td);


	/* Destroy td, freeing memory. */
	TgaDestroyData(&td);


	/* Return status. */
	return(status);
}





