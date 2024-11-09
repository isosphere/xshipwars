#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/string.h"
#include "../include/objects.h"

#include "../include/unvmain.h"
#include "../include/unvutil.h"
#include "../include/unvmath.h"
#include "../include/unvmatch.h"
#include "../include/unvfile.h"


#define PROG_NAME	"unvtocht"
#define PROG_VERSION	"0.1"

#define boolean_t	unsigned char
#define filter_flags_t	unsigned long

#define FILTER_FLAG_STATIC	(1 << 1)
#define FILTER_FLAG_DYNAMIC	(1 << 2)
#define FILTER_FLAG_CONTROLLED	(1 << 3)
#define FILTER_FLAG_PLAYER	(1 << 4)
#define FILTER_FLAG_WEAPON	(1 << 5)
#define FILTER_FLAG_HOME	(1 << 6)
#define FILTER_FLAG_AREA	(1 << 7)
#define FILTER_FLAG_ANIMATED	(1 << 8)
#define FILTER_FLAG_WORMHOLE	(1 << 9)
#define FILTER_FLAG_ELINK	(1 << 10)


static boolean_t	verbose = 0,
			allow_compress = 0;


void LoadProgressCB(void *data, int i, int total)
{
        if(verbose)
        {
            printf("\r%s: Parsing universe %.0lf%%... ",
                (char *)data,
                (double)i /
		(double)((total == 0) ? 1 : total) * 100
            );
        }
	return;
} 


void SaveProgressCB(void *data, int i, int total)
{
	if(verbose)
	{
	    printf("\r%s: Generating star chart %.0lf%%... ",
		(char *)data,
		(double)i /
		((total == 0) ? 1 : total) * 100
	    );
	}
        return;
}


void PrintUsage(void)
{
	printf(
"Usage: %s <infile.unv> <outfile.cht> [filter_flags] [options]\n\
\n\
    <infile.unv> specifies the universe file to be loaded as\n\
    referance for generating a star chart from.\n\
\n\
    <outfile.cht> specifies the star chart file to be generated.\n\
    If the file exists, then it will be overwritten.\n\
\n\
    [filter_flags] can be any sequence of the following characters\n\
    which correspond to an ShipWars object type:\n\
\n\
        s                       Static\n\
        d                       Dynamic\n\
        c                       Controlled (AI)\n\
        p                       Player\n\
        w                       Weapon (all types)\n\
        h                       Home\n\
        a                       Area\n\
        n                       Animated\n\
        g                       Worm Hole\n\
        l                       External Link\n\
\n\
    Default value is: shag\n\
\n\
    [options] can be any of the following:\n\
\n\
        -v                      Verbosly list objects processed.\n\
	-c                      Allow compression and reindexing.\n\
        --help                  Prints (this) help screen and exits.\n\
        --version               Prints version information and exits.\n\
\n",
	    PROG_NAME
	);

	return;
}

void DoFilterObjects(
	xsw_object_struct ***ptr,
	int *total,
	filter_flags_t flags
)
{
	int i, n;
	boolean_t delete_me;
	xsw_object_struct *obj_ptr;


	if((ptr == NULL) ||
           (total == NULL)
	)
	    return;


	/* Delete filtered out object types. */
	for(i = 0; i < *total; i++)
        {
            obj_ptr = (*ptr)[i];
	    delete_me = 0;

	    if(obj_ptr->type <= XSW_OBJ_TYPE_GARBAGE)
		delete_me = 1;
	    else if((obj_ptr->type == XSW_OBJ_TYPE_STATIC) &&
                    !(flags & FILTER_FLAG_STATIC)
	    )
                delete_me = 1;
            else if((obj_ptr->type == XSW_OBJ_TYPE_DYNAMIC) &&
                    !(flags & FILTER_FLAG_DYNAMIC)
            )
                delete_me = 1;
            else if((obj_ptr->type == XSW_OBJ_TYPE_CONTROLLED) &&
                    !(flags & FILTER_FLAG_CONTROLLED)
            )
                delete_me = 1;
            else if((obj_ptr->type == XSW_OBJ_TYPE_PLAYER) &&
                    !(flags & FILTER_FLAG_PLAYER)
            )
                delete_me = 1;
            else if((obj_ptr->type == XSW_OBJ_TYPE_WEAPON) &&
                    !(flags & FILTER_FLAG_WEAPON)
            )
                delete_me = 1;
            else if((obj_ptr->type == XSW_OBJ_TYPE_STREAMWEAPON) &&
                    !(flags & FILTER_FLAG_WEAPON)
            )
                delete_me = 1;
            else if((obj_ptr->type == XSW_OBJ_TYPE_SPHEREWEAPON) &&
                    !(flags & FILTER_FLAG_WEAPON)
            )
                delete_me = 1;
            else if((obj_ptr->type == XSW_OBJ_TYPE_HOME) &&
                    !(flags & FILTER_FLAG_HOME)
            )
                delete_me = 1;
            else if((obj_ptr->type == XSW_OBJ_TYPE_AREA) &&
                    !(flags & FILTER_FLAG_AREA)
            )
                delete_me = 1;
            else if((obj_ptr->type == XSW_OBJ_TYPE_ANIMATED) &&
                    !(flags & FILTER_FLAG_ANIMATED)
            )
                delete_me = 1;
            else if((obj_ptr->type == XSW_OBJ_TYPE_WORMHOLE) &&
                    !(flags & FILTER_FLAG_WORMHOLE)
            )
                delete_me = 1;
            else if((obj_ptr->type == XSW_OBJ_TYPE_ELINK) &&
                    !(flags & FILTER_FLAG_ELINK)
            )
                delete_me = 1;


	    if(delete_me)
	    {
		UNVDeleteObject(obj_ptr);
		(*ptr)[i] = NULL;
	    }
	    else
	    {
		if(verbose)
		    printf("%s: Imported.\n",
			UNVGetObjectFormalName(obj_ptr, i)
		    );
	    }
	}


	/* Allow compressing and reindexing? */
	if(allow_compress)
	{
	    int old_total;

	    /* Record previous total. */
	    old_total = *total;
	    if(old_total < 0)
		old_total = 0;

	    /* Shift away NULL pointers. */
	    for(i = 0; i < *total;)
	    {
	        obj_ptr = (*ptr)[i];
	        if(obj_ptr != NULL)
		{
		    i++;
	            continue;
		}

	        /* Object pointer is NULL, so shift. */
	        for(n = i; n < (*total - 1); n++)
		    (*ptr)[n] = (*ptr)[n + 1];

	        *total = *total - 1;

		if(verbose)
                    printf("\rCompressing %.0lf%%... ",
			(double)i / (double)(*total) * 100
		    );
	    }
	    if(verbose)
		printf(
"\rCompressing complete, achieved %.0lf%% conservation.\n",
		    (double)(old_total - *total) /
		    (double)((old_total == 0) ? 1 : old_total) * 100
		);

	    /* Reallocate pointers. */
	    if(*total > 0)
	    {
	        *ptr = (xsw_object_struct **)realloc(
		    *ptr,
		    *total * sizeof(xsw_object_struct *)
	        );
	        if(*ptr == NULL)
	        {
	            *total = 0;
		    return;
	        }
	    }
	    else
	    {
	        free(*ptr);
	        *ptr = NULL;

	        *total = 0;
	    }
	}

	return;
}

int main(int argc, char *argv[])
{
	int i, status;
	char *in_filename = NULL, *out_filename = NULL;
	xsw_object_struct **xsw_object;
	int total;
	unv_head_struct header;
	filter_flags_t filter_flags =	FILTER_FLAG_STATIC |
					FILTER_FLAG_HOME |
					FILTER_FLAG_AREA |
					FILTER_FLAG_WORMHOLE;


	/* Parse arguments. */
	for(i = 1; i < argc; i++)
	{
	    if(argv[i] == NULL)
		continue;


	    if(strcasepfx(argv[i], "--h") ||
               strcasepfx(argv[i], "-h") ||
               strcasepfx(argv[i], "-?")
	    )
	    {
		PrintUsage();
		return(0);
	    }
	    else if(strcasepfx(argv[i], "--version") ||
                    strcasepfx(argv[i], "-version")
	    )
	    {
		printf("%s Version %s\n", PROG_NAME, PROG_VERSION);
		return(0);
	    }
	    else if(strcasepfx(argv[i], "--v") ||
                    strcasepfx(argv[i], "-v")
            )
            {
		verbose = 1;
	    }
            else if(strcasepfx(argv[i], "--c") ||
                    strcasepfx(argv[i], "-c")
            )
            {
                allow_compress = 1;
            }
	    else if(i == 1)
	    {
		in_filename = argv[i];
	    }
            else if(i == 2)
            {
                out_filename = argv[i];
            }
            else if(i == 3)
            {
		filter_flags = 0;

		if(strchr(argv[i], 's'))
		    filter_flags |= FILTER_FLAG_STATIC;
                if(strchr(argv[i], 'd'))
                    filter_flags |= FILTER_FLAG_DYNAMIC;
                if(strchr(argv[i], 'c'))
                    filter_flags |= FILTER_FLAG_CONTROLLED;
                if(strchr(argv[i], 'p'))
                    filter_flags |= FILTER_FLAG_PLAYER;
                if(strchr(argv[i], 'w'))
                    filter_flags |= FILTER_FLAG_WEAPON;
                if(strchr(argv[i], 'h'))
                    filter_flags |= FILTER_FLAG_HOME;
                if(strchr(argv[i], 'a'))
                    filter_flags |= FILTER_FLAG_AREA;
                if(strchr(argv[i], 'n'))
                    filter_flags |= FILTER_FLAG_ANIMATED;
                if(strchr(argv[i], 'g'))
                    filter_flags |= FILTER_FLAG_WORMHOLE;
                if(strchr(argv[i], 'l'))
                    filter_flags |= FILTER_FLAG_ELINK;
            }
	}

	/* Make sure we got the nessasary inputs. */
	if(in_filename == NULL)
	{
	    PrintUsage();
	    return(1);
	}
	if(out_filename == NULL)
	    out_filename = "output.cht";


	/* Load universe file. */
	xsw_object = UNVLoadFromFile(
	    in_filename,
	    &total,
	    &header,
	    in_filename,	/* Data. */
	    LoadProgressCB
	);
	if(verbose)
	    printf("\r%s: Finished parsing universe.\n", in_filename);

	/* Filter. */
	DoFilterObjects(
            &xsw_object,
	    &total,
	    filter_flags
	);

	/* Save to chart file. */
	status = UNVSaveToFile(
            out_filename,
	    xsw_object,
	    total,
	    &header,
	    out_filename,	/* Data. */
	    SaveProgressCB
        );
        if(verbose)
            printf("\r%s: Finished generating star chart.\n", out_filename);

	UNVDeleteAllObjects(xsw_object, total);


	return(0);
}
