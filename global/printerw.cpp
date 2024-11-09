// global/printerw.cpp
/*
                          Printer Wrapper
 */

#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>

#include "../include/os.h"
#include "../include/strexp.h"
#include "../include/string.h"
#include "../include/disk.h"
#include "../include/prochandle.h"


#include "../include/osw-x.h"	/* Need to know about image_t. */

#include "../include/printerw.h"



double CM_TO_PIXELS(double x)
{
	return(x * 72 / 2.56);
}


double INCH_TO_PIXELS(double x)
{
	return(x * 72);
}


/*
 *	Writes a PS file based onthe given image and parm data.
 *	The file fp must already be fopen()ed prior to calling
 *	this function and assumed to be positioned at the beginning
 *	of the file.  The file must be opened in write only mode.
 */
int PrinterWritePSImage(
	FILE *fp,
	image_t *image,
	int page,		/* Page number (starts from 0). */
	printer_parm_struct *parm
)
{
	int w, h;
	u_int8_t *ptr8;
        u_int16_t *ptr16;
        u_int32_t *ptr32;
	int col, line_max;


	if((fp == NULL) ||
           (image == NULL) ||
           (parm == NULL) ||
	   (page < 0)
	)
	    return(PrinterBadValue);


	/* ********************************************************* */
	/* Begin writing .ps file. */


	/* Headings. */

	/* Format identifier. */
        fprintf(
	    fp,
"%%!PS-Adobe-2.0 EPSF-2.0\n"
	);
	/* Title. */
        fprintf(
            fp, 
"%%%%Title: %s\n",
	    parm->title
        );
	/* Creator. */
        fprintf(
            fp,
"%%%%Creator: %s\n",
            "Generic Printer Wrapper Code"
        );
	/* Bounding box. */
	switch(parm->units)
	{
	  case PrinterUnitCentimeters:
            fprintf(
               fp,
"%%%%BoundingBox: %i %i %i %i\n",
                (int)CM_TO_PIXELS(parm->x),
                (int)CM_TO_PIXELS(parm->y),
                (int)CM_TO_PIXELS(parm->x) + (int)image->width,
                (int)CM_TO_PIXELS(parm->y) + (int)image->height
            );
	    break;

          case PrinterUnitInches:
            fprintf(
               fp,
"%%%%BoundingBox: %i %i %i %i\n",
                (int)INCH_TO_PIXELS(parm->x),
                (int)INCH_TO_PIXELS(parm->y),
                (int)INCH_TO_PIXELS(parm->x) + (int)image->width,
                (int)INCH_TO_PIXELS(parm->y) + (int)image->height
            );
            break;

          default:	/* PrinterUnitPixels */
            fprintf(
                fp,
"%%%%BoundingBox: %i %i %i %i\n",
                (int)parm->x,
		(int)parm->y,
		(int)parm->x + (int)image->width,
		(int)parm->y + (int)image->height
            );
            break;
	}
	/* Pages. */
        fprintf(
            fp,
"%%%%Pages: %i\n",
            parm->pages        
        );
	/* Fonts? */
        fprintf(
            fp,   
"%%%%DocumentFonts:\n"
	);
	/* End comments? */
        fprintf(
            fp,   
"%%%%EndComments\n"
	);
	/* End prolog? */
	fprintf(
            fp,
"%%%%EndProlog\n\n"
	);


	/* Page number. */
	fprintf(
            fp,
"%%%%Page: %i %i\n\n",
	    page + 1,
	    parm->pages
        );

	/* Remember original state? */
        fprintf(   
            fp,
"%% remember original state\n"
	);
        fprintf(   
            fp,
"/origstate save def\n\n"
	);

	/* Temp dictionary? */
	fprintf(
            fp,
"%% build a temporary dictionary\n"
	);
        fprintf(
            fp,
"20 dict begin\n\n"
	);

	/* Define string to hold a scanline's worth of data. */
        fprintf(
            fp,
"%% define string to hold a scanline's worth of data\n"
        );
	if(parm->color_mode == PrinterColorModeColor)
            fprintf(
                fp,
"/pix %i string def\n\n",
	        (int)image->width * 3
            );
	else
            fprintf(   
                fp,
"/pix %i string def\n\n",
                (int)image->width
            );


        /* Define space for color conversions. */
	fprintf(
            fp,
"%% define space for color conversions\n"
	);
        fprintf(
            fp,
"/grays %i string def  %% space for gray scale line\n",
	    (int)image->width
	);
        fprintf(
            fp,
"/npixls 0 def\n"
	);
	fprintf(
            fp,
"/rgbindx 0 def\n\n"
	);

	/* Lower left corner. */
	fprintf(
            fp,
"%% lower left corner\n"
	);
        switch(parm->units)
        {
          case PrinterUnitCentimeters:
            fprintf(
               fp,
"%i %i translate\n\n",
                (int)CM_TO_PIXELS(parm->x),
                (int)CM_TO_PIXELS(parm->height) -
                    (int)CM_TO_PIXELS(parm->y) - (int)image->height
            );
            break;

          case PrinterUnitInches:
            fprintf(
               fp,
"%i %i translate\n\n",
                (int)INCH_TO_PIXELS(parm->x),
                (int)INCH_TO_PIXELS(parm->height) -
                    (int)INCH_TO_PIXELS(parm->y) - (int)image->height
            );
            break;

          default:	/* PrinterUnitPixels */
            fprintf(
               fp,
"%i %i translate\n\n",
                (int)parm->x,
                (int)parm->height - (int)parm->y - (int)image->height
            );
            break;
	};


	/* Size of image (on paper, in 1/72inch coords). */
        fprintf(
            fp,
"%% size of image (on paper, in 1/72inch coords)\n"
	);
	fprintf(
            fp,
"%.5f %.5f scale\n\n",
	    (double)image->width * 0.9993,
	    (double)image->height * 0.9993
	);

	/* Define `colorimage' if printing in color mode. */
	if(parm->color_mode == PrinterColorModeColor)
	{
            fprintf(
                fp,   
"%% define 'colorimage' if it isn't defined\n\
%%   ('colortogray' and 'mergeprocs' come from xwd2ps\n\
%%     via xgrab)\n"
	    );
            fprintf(
                fp,
"/colorimage where   %% do we know about 'colorimage'?\n\
  { pop }           %% yes: pop off the 'dict' returned\n\
  {                 %% no:  define one\n\
    /colortogray {  %% define an RGB->I function\n\
      /rgbdata exch store    %% call input 'rgbdata'\n\
      rgbdata length 3 idiv\n\
      /npixls exch store\n\
      /rgbindx 0 store\n\
      0 1 npixls 1 sub {\n\
        grays exch\n\
        rgbdata rgbindx       get 20 mul    %% Red\n\
        rgbdata rgbindx 1 add get 32 mul    %% Green\n\
        rgbdata rgbindx 2 add get 12 mul    %% Blue\n\
        add add 64 idiv      %% I = .5G + .31R + .18B\n\
        put\n\
        /rgbindx rgbindx 3 add store\n\
      } for\n\
      grays 0 npixls getinterval\n\
    } bind def\n\n"
	    );

            fprintf(
                fp,
"%% Utility procedure for colorimage operator.\n\
%% This procedure takes two procedures off the\n\
%% stack and merges them into a single procedure.\n\n"
	    );

            fprintf(
                fp,
"/mergeprocs { %% def\n\
      dup length\n\
      3 -1 roll\n\
      dup\n\
      length\n\
      dup\n\
      5 1 roll\n\
      3 -1 roll\n\
      add\n\
      array cvx\n\
      dup\n\
      3 -1 roll\n\
      0 exch\n\
      putinterval\n\
      dup\n\
      4 2 roll\n\
      putinterval\n\
    } bind def\n\n"
	    );

            fprintf(
                fp,
"/colorimage { %% def\n\
      pop pop     %% remove 'false 3' operands\n\
      {colortogray} mergeprocs\n\
      image\n\
    } bind def\n\
  } ifelse          %% end of 'false' case\n\n"
	    );
	}

	/* Dimensions of data. */
	fprintf(
            fp,
"%% dimensions of data\n"
	);
	switch(parm->color_mode)
	{
	  case PrinterColorModeColor:
            fprintf(
                fp,
"%i %i %i\n",
	        (int)image->width, (int)image->height, 8
	    );
	    break;

          case PrinterColorModeGreyScale:
            fprintf(
                fp,
"%i %i %i\n",
                (int)image->width, (int)image->height, 8
            );
            break;

	  default:	/* PrinterColorModeBlackAndWhite */
            fprintf(
                fp,
"%i %i %i\n",
                (int)image->width, (int)image->height, 1
            );
            break;
	}

        /* Mapping matrix. */
        fprintf(
            fp,
"%% mapping matrix\n"
        );
        fprintf(
            fp,
"[%i %i %i %i %i %i]\n",
	    (int)image->width,
	    0,
	    0,
	    (int)image->height * -1,
	    0,
	    (int)image->height
	);

	fprintf(
            fp,
"{currentfile pix readhexstring pop}\n"
	);

	/* Use color image if in color mode. */
        if(parm->color_mode == PrinterColorModeColor)
            fprintf(
                fp,
"false 3 colorimage\n\n"
	    );
	else
            fprintf(
                fp,
"image\n\n"
	    );


	/* Write image data. */

	/* 8 bits. */
	if(image->depth == 8)
	{
	    ptr8 = (u_int8_t *)image->data;

	    col = 0;
	    line_max = 70;

	    for(h = 0; h < (int)image->height; h++)
	    {
	        for(w = 0; w < (int)image->width; w++)
	        {
		    /*   Write image pixel.
		     *   rrgg gbbb
		     */
		    if(parm->color_mode == PrinterColorModeColor)
		    {
			fprintf(fp, "%.2x",
			    (u_int8_t)((*ptr8) & 0xE0));
                        fprintf(fp, "%.2x",
			    (u_int8_t)(((*ptr8) & 0x1C) << 3));
                        fprintf(fp, "%.2x",
			    (u_int8_t)(((*ptr8) & 0x03) << 6));

                        col += 6;
		    }
		    else if(parm->color_mode == PrinterColorModeGreyScale)
		    {
                        fprintf(fp, "%.2x",
			    (u_int8_t)((*ptr8) & 0xE0));

                        col += 2;
		    }
		    ptr8++;

		    if(col >= line_max)
		    {
			col = 0;
			fputc('\n', fp);
		    }
		}

                col = 0;
                fputc('\n', fp);
	    }

	}
	/* 15 bits. */
        else if(image->depth == 15)
        {
            ptr16 = (u_int16_t *)image->data;
 
            col = 0;
            line_max = 70;

            for(h = 0; h < (int)image->height; h++)
            {
                for(w = 0; w < (int)image->width; w++)
                {    
                    /*   Write image pixel.
                     *   arrr rrgg gggb bbbb
                     */
                    if(parm->color_mode == PrinterColorModeColor)
                    {
                        fprintf(fp, "%.2x",
			    (u_int8_t)(((*ptr16) & 0x7C00) >> 7));
                        fprintf(fp, "%.2x",
			    (u_int8_t)(((*ptr16) & 0x03E0) >> 2));
                        fprintf(fp, "%.2x",
			    (u_int8_t)(((*ptr16) & 0x001F) << 3));

                        col += 6;
                    }
                    else if(parm->color_mode == PrinterColorModeGreyScale)
                    {
                        fprintf(fp, "%.2x", 
                            (u_int8_t)(((*ptr16) & 0x7C00) >> 7));

                        col += 2;
                    }
                    ptr16++;

                    if(col >= line_max)
                    {
                        col = 0;
                        fputc('\n', fp);
                    }
                }

                col = 0;
                fputc('\n', fp);
            }
        }
        /* 16 bits. */
        else if(image->depth == 16)
        {
            ptr16 = (u_int16_t *)image->data;

            col = 0;
            line_max = 70;

            for(h = 0; h < (int)image->height; h++)
            {
                for(w = 0; w < (int)image->width; w++)
                {
                    /*   Write image pixel.
                     *   rrrr rggg gggb bbbb
                     */
                    if(parm->color_mode == PrinterColorModeColor)
                    {
                        fprintf(fp, "%.2x",
                            (u_int8_t)(((*ptr16) & 0xF800) >> 8));
                        fprintf(fp, "%.2x",
                            (u_int8_t)(((*ptr16) & 0x07E0) >> 3));
                        fprintf(fp, "%.2x",
                            (u_int8_t)(((*ptr16) & 0x001F) << 3));

			col += 6;
                    }
                    else if(parm->color_mode == PrinterColorModeGreyScale)
                    {
                        fprintf(fp, "%.2x",
                            (u_int8_t)(((*ptr16) & 0xF800) >> 8));

			col += 2;
                    }
                    ptr16++;

                    if(col >= line_max)
                    {
                        col = 0;
                        fputc('\n', fp);
                    }
                }

		col = 0;
                fputc('\n', fp);
            }
        }
	/* 24 or 32 bits. */
        else if((image->depth == 24) ||
                (image->depth == 32)
	)
        {
            ptr32 = (u_int32_t *)image->data;
                     
            col = 0; 
            line_max = 70;

            for(h = 0; h < (int)image->height; h++)
            {
                for(w = 0; w < (int)image->width; w++)
                {
                    /*   Write image pixel.
                     *   aaaa aaaa rrrr rrrr gggg gggg bbbb bbbb
                     */
                    if(parm->color_mode == PrinterColorModeColor)
                    {
                        fprintf(fp, "%.2x",
                            (u_int8_t)(((*ptr32) & 0x00FF0000) >> 16));
                        fprintf(fp, "%.2x",
                            (u_int8_t)(((*ptr32) & 0x0000FF00) >> 8));
                        fprintf(fp, "%.2x",
                            (u_int8_t)((*ptr32) & 0x000000FF));

                        col += 6;
                    }
                    else if(parm->color_mode == PrinterColorModeGreyScale)
                    {
                        fprintf(fp, "%.2x",
                            (u_int8_t)(((*ptr32) & 0x00FF0000) >> 16));

                        col += 2;
                    }
                    ptr32++;

                    if(col >= line_max)
                    {
                        col = 0;
                        fputc('\n', fp);
                    }
                }

                col = 0;
                fputc('\n', fp);
            }
        }

        fputc('\n', fp);


        /* Show page. */
        fprintf(
            fp,
"showpage\n\n"
        );           

        /* Stop using temporary dictionary. */
        fprintf(
            fp,
"%% stop using temporary dictionary\n"
        );
        fprintf(
            fp, 
"end\n\n"
        );
 
        /* Restore originate state. */
        fprintf(
            fp,
"%% restore original state\n"
        );
        fprintf(
            fp,
"origstate restore\n\n"
        );

        /* Trailer. */
        fprintf(
            fp,
"%%%%Trailer\n"
        );



	return(PrinterSuccess);
}


/*
 *	Proecdure to run the print command.
 */
int PrinterRunPrint(
	char *cmd,
	char *filename
)
{
#define PrinterFileToken	"%file"

	char *strptr;
	int i, n, len;

	char **strv;
	int strc;


	if(filename == NULL)
	    return(PrinterBadValue);


	if(cmd == NULL)
	    cmd = PrinterDefPrintCmd;

	if(cmd[0] == '\0')
	    return(PrinterSuccess);


	/* Allocate new command. */
	n = 0;
	strptr = strstr(cmd, PrinterFileToken);
	while(strptr != NULL)
	{
	    n++;
	    strptr = strstr(strptr + 1, PrinterFileToken);
	}

	strptr = cmd;
	len = strlen(cmd) + (strlen(filename) * n) + 10;
	cmd = (char *)malloc((len + 1) * sizeof(char));
	if(cmd == NULL)
	    return(PrinterNoBuffers);
	strcpy(cmd, strptr);


	/* Substitute. */
	substr(cmd, PrinterFileToken, filename);


	/* Explode command. */
	strv = strchrexp(cmd, ';', &strc);
	for(i = 0; i < strc; i++)
	{
	    if(strv[i] == NULL)
		continue;

	    ExecB(strv[i]);

	    free(strv[i]);
	}
	free(strv);




	/* Free new allocated command string. */
	free(cmd);


	return(PrinterSuccess);
}


/*
 *	Procedure to print an image.
 */
int PrinterPrintImage(
        image_t *image,
        char *tmp_file,			/* Can be NULL. */
        char *cmd,			/* Can be NULL. */
        printer_parm_struct *parm
)
{
	int status;
	char *strptr;
        char tmp_name[PATH_MAX + NAME_MAX];

	struct stat stat_buf;
	FILE *fp;


	/* Error checks. */
	if((image == NULL) ||
	   (parm == NULL)
	)
	    return(PrinterBadValue);

	if((image->width == 0) ||
	   (image->height == 0) ||
	   (image->data == NULL)
	)
	    return(PrinterBadValue);

	if((parm->width <= 0) ||
           (parm->height <= 0)
	)
	    return(PrinterBadValue);

	if(parm->pages <= 0)
	    return(PrinterSuccess);


	/* ********************************************************* */
	/* Create tempory .ps file. */

        /* Get tempory file name. */
	if(tmp_file == NULL)
	{
            strptr = tmpnam(NULL);
            if(strptr == NULL)
                return(PrinterError);

	}
	else
	{
	    strptr = tmp_file;
	}
        strncpy(tmp_name, strptr, PATH_MAX + NAME_MAX);
        tmp_name[PATH_MAX + NAME_MAX - 1] = '\0';


	/* Make sure tempory file does not exist. */
	if(!stat(tmp_name, &stat_buf))
	    return(PrinterError);


	/* Create tempory file. */
	fp = fopen(tmp_name, "w");
	if(fp == NULL)
	    return(PrinterError);

	status = PrinterWritePSImage(
	    fp,
	    image,
	    0,
	    parm
	);

	fclose(fp);
	fp = NULL;

	if(status != PrinterSuccess)
	    return(status);


        /* ********************************************************* */
	/* Print command. */

	if(parm->dest == PrinterDestinationPrinter)
	{
	    /* Print to printer. */
	    status = PrinterRunPrint(cmd, tmp_name);

            if(status != PrinterSuccess)
	    {
                unlink(tmp_name);
                return(status);
	    }

	    /*   Since printing to printer and not to file, remove
	     *   file.
	     */
	    unlink(tmp_name);
	}
	else
	{
	    /* Print to file (nothing to do). */

	}


	return(PrinterSuccess);
}




