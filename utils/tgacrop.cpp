#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <sys/stat.h>
#include <string.h>	// for bzero()

#include "../include/xsw_ctype.h"
#include "../include/tga.h"

int ISODD(long l);
void DoCrop(tga_data_struct *td_in, tga_data_struct *td_out, int total_frames);
void MaxBounds(
	u_int8_t *data,
	unsigned int width,
	unsigned int height,
	unsigned int depth,
	unsigned int total_frames,
	int *x1, int *y1,
	int *x2, int *y2
);


int ISODD(long l)
{
	return((l & 1) ? 1 : 0);
}


void MaxBounds(
        u_int8_t *data,            
        unsigned int width,            
        unsigned int height,            
        unsigned int depth,            
        unsigned int total_frames,            
        int *x1, int *y1,
        int *x2, int *y2
)
{
	int f, x, y, fy;
	unsigned int f_height;
	int bytes_per_pixel, bytes_per_line;

	u_int32_t *img_ptr32;


	if(data == NULL)
	    return;


        f_height = height / total_frames;   

	/* Reset bounds. */
	*x1 = width;
	*y1 = f_height;
	*x2 = -1;
	*y2 = -1;



	if((depth == 24) ||
           (depth == 32)
	)
	{
	    bytes_per_pixel = BYTES_PER_PIXEL32;
	    bytes_per_line = BYTES_PER_PIXEL32 * width;

	    /* Go through each frame. */
	    for(f = 0; f < total_frames; f++)
	    {
		/* Go through each line of the frame. */
		for(y = f_height * f, fy = 0;
                    fy < f_height;
                    y++, fy++
		)
		{
		    /* Go through each pixel of the line. */
		    for(x = 0; x < width; x++)
		    {
			/* Is pixel transparent? */
			img_ptr32 = (u_int32_t *)(&data[
			    (y * bytes_per_line) +
                            (x * BYTES_PER_PIXEL32)
			]);
			if((*img_ptr32 & 0x00ffffff) != 0x00000000)
			{
                            if(*x1 > x)
                                *x1 = x;
                            if(*y1 > fy)
                                *y1 = fy;

                            if(*x2 < x) 
                                *x2 = x; 
                            if(*y2 < fy)
                                *y2 = fy;
			}
		    }
		}
/*
printf("Frame %i bounds: %i %i  %i %i\n",
f,
*x1, *y1,
*x2, *y2
);
 */
	    }
	}

	return;
}


void CopyImage(
	unsigned int d,
	u_int8_t *src_buf,
	u_int8_t *tar_buf,
	unsigned int src_width,
	unsigned int src_height,
	unsigned int tar_width,
	unsigned int tar_height,
	unsigned int total_frames,
        int x1, int y1,
        int x2, int y2
)
{
	int frame;
	int tar_x, tar_y, tar_fy;
	int src_fheight, tar_fheight;
	int src_bytes_per_line, tar_bytes_per_line;
	int src_bytes_per_frame, tar_bytes_per_frame;
	u_int32_t *src_buf_ptr32, *tar_buf_ptr32;


	if((src_buf == NULL) ||
           (tar_buf == NULL)
	)
	    return;


        src_fheight = src_height / total_frames;
        tar_fheight = tar_height / total_frames;

	if((tar_width + x1) > src_width)
	{
            fprintf(stderr, "Position out of x axis bounds.\n");
	    return;
	}
        if((tar_height + y1) > src_height)
        {
            fprintf(stderr, "Position out of y axis bounds.\n");
            return;
        }


	if(x1 < 0)
	    x1 = 0;
	if(y1 < 0)
	    y1 = 0;

        tar_x = 0;
        tar_y = 0;
	tar_fy = 0;


        if((d == 24) ||
           (d == 32)
        )
	{
	    src_bytes_per_line = src_width * BYTES_PER_PIXEL32;
            tar_bytes_per_line = tar_width * BYTES_PER_PIXEL32;

            src_bytes_per_frame = src_width * src_fheight *
		BYTES_PER_PIXEL32;
            tar_bytes_per_frame = tar_width * tar_fheight *
		BYTES_PER_PIXEL32;

	    frame = tar_y / tar_fheight;

	    while(tar_y < (int)tar_height)
	    {
		src_buf_ptr32 = (u_int32_t *)(&src_buf[
		    (frame * src_bytes_per_frame) +
                    ((y1 + tar_fy) * src_bytes_per_line) +
		    ((x1 + tar_x) * BYTES_PER_PIXEL32)
		]);

                tar_buf_ptr32 = (u_int32_t *)(&tar_buf[
                    (tar_y * tar_bytes_per_line) +
                    (tar_x * BYTES_PER_PIXEL32)
                ]);

		*tar_buf_ptr32 = *src_buf_ptr32;


		tar_x++;
		if(tar_x >= (int)tar_width)
		{
		    tar_x = 0;
		    tar_y++;

                    frame = tar_y / tar_fheight;

		    tar_fy++;
		    if(tar_fy >= tar_fheight)
			tar_fy = 0;
		}
	    }
	}


	return;
}

void DoCrop(
	tga_data_struct *td_in,
	tga_data_struct *td_out,
	int total_frames
)
{
	int x1, y1, x2, y2;
	int width, height, f_height;


	if((td_in == NULL) ||
           (td_out == NULL)
	)
	    return;


	bzero(td_out, sizeof(tga_data_struct));


	f_height = (int)td_in->height / (int)total_frames;

	/* Get bounds. */
	MaxBounds(
	    td_in->data,
	    td_in->width,
	    td_in->height,
	    td_in->depth,
	    total_frames,
	    &x1, &y1,
	    &x2, &y2
	);

/* Apply offsets? */


	/* Sanitize. */
	if(x1 < 0)
	    x1 = 0;
	if(y1 < 0)
	    y1 = 0;

	if(x2 >= (int)td_in->width)
            x2 = td_in->width;
	if(y2 >= f_height)
	    y2 = f_height;


printf("Final bounds: %i %i  %i %i\n",
x1, y1,
x2, y2
);

	/* Create td out data. */
	width = x2 - x1 + 1;
	height = y2 - y1 + 1;

	if(ISODD(width))
	    width++;
	if(ISODD(height))
            height++;

	td_out->width = width;
	td_out->height = height * total_frames;

	td_out->data_depth = td_in->depth;

	if(td_out->data_depth == 8)
	    td_out->data = (u_int8_t *)calloc(
		1,
		td_out->width * td_out->height * BYTES_PER_PIXEL8
	    );
        else if((td_out->data_depth == 15) ||
                (td_out->data_depth == 16)
	)
            td_out->data = (u_int8_t *)calloc(
		1,
                td_out->width * td_out->height * BYTES_PER_PIXEL16
            );
        else if((td_out->data_depth == 24) ||
                (td_out->data_depth == 32)
	)
            td_out->data = (u_int8_t *)calloc(
		1,
                td_out->width * td_out->height * BYTES_PER_PIXEL32
            );
	if(td_out->data == NULL)
	{
	    fprintf(stderr,
		"Memory allocation error or unsupported depth.\n"
	    );
	}


	/* Copy image. */
	CopyImage(
	    td_in->depth,
	    td_in->data,
	    td_out->data,
            td_in->width, td_in->height,
            td_out->width, td_out->height,
            total_frames,
            x1, y1,
            x2, y2
	);


	return;
}


int main(int argc, char *argv[])
{
	int status;
	int total_frames = 1;
	tga_data_struct td_in, td_out;


	if(argc < 4)
	{
	    printf(
 "usage: tgacrop <in_file> <out_file> [num_frames]\n"
	    );
	    return(1);
	}

	if(argc >= 4)
	    total_frames = atoi(argv[3]);
	if(total_frames < 1)
	    total_frames = 1;

	printf("Reading from `%s'...\n", argv[1]);
	status = TgaReadFromFile(
	    argv[1],
	    &td_in,
	    24
	);
	if(status != TgaSuccess)
	{
	    printf("Tga read error %i.\n", status);

	    TgaDestroyData(&td_in);
	    return(1);
	}

        printf("Cropping...\n");
	DoCrop(
	    &td_in,	/* Input tga. */
	    &td_out,	/* Output buffer */
	    total_frames
	);


        printf("Writing to `%s'...\n", argv[2]);
	status = TgaWriteToFile(
	    argv[2],
            &td_out,
            24
	);
	if(status != TgaSuccess)
        {
            printf("Tga write error %i.\n", status);
        }


	TgaDestroyData(&td_in);
	TgaDestroyData(&td_out);

	printf("Done.\n");


	return(0);
}
