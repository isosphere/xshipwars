/*
                        Stats Exporting To File

	Functions:

	int ExportConList(char *filename)
	int ExportEvents(
		char *filename,
		int type,
		char *data
	)
	int ExportScores(char *filename)

	---

 */

#include "swserv.h"


/*
 *	Exports list of connected players.
 */
int ExportConList(char *filename)
{
        FILE *fp;
        int i;
        connection_struct **ptr, *con_ptr;


        if(filename == NULL)
            return(-1);
         
        if(*filename == '\0')
            return(0);
                
        fp = fopen(filename, "w");
        if(fp == NULL)
            return(-1);

        /* Header. */
        fprintf(fp,
 "player;duration;residency;client\n"
        );

        /* Go through connections list. */
        for(i = 0, ptr = connection;
            i < total_connections;
            i++, ptr++
        )
        {
            con_ptr = *ptr;
            if(con_ptr == NULL)
                continue;

            if(con_ptr->socket < 0)
                continue;

	    if(con_ptr->object_num < 0)
		continue;

            fprintf(fp,
 "%s;%ld;%i;%i\n",
		DBGetFormalNameStr(con_ptr->object_num),
                cur_systime - con_ptr->contime,
                con_ptr->is_guest,
		con_ptr->client_type
            );
        }

        fclose(fp);


	return(0);
}

/*
 *	Exports a new event, a line appended to the events file.
 */
int ExportEvents(
	char *filename,
	int type,
	char *data
)
{
	FILE *fp;


        if(filename == NULL)
            return(-1);

	if(*filename == '\0')
	    return(0);
        
        fp = fopen(filename, "a");
        if(fp == NULL)
            return(-1);

	fprintf(fp,
	    "%ld;%i;%s\n",
	    cur_systime,
	    type,
	    ((data == NULL) ? "" : data)
	);


	fclose(fp);


	return(0);
}

/*
 *	Exports scores of each player object that has scores.
 */
int ExportScores(char *filename)
{
	FILE *fp;
	long i;
	xsw_object_struct **ptr, *obj_ptr;


	if(filename == NULL)
	    return(-1);

        if((*filename) == '\0')
            return(0);

	fp = fopen(filename, "w");
	if(fp == NULL)
	    return(-1);

	/* Header. */
	fprintf(fp,
 "player;empire;damage_given;damage_recieved;kills;credits;rmu;rmu_max\n"
	);

	/* Go through objects list. */
	for(i = 0, ptr = xsw_object;
            i < total_objects;
            i++, ptr++
	)
	{
	    obj_ptr = *ptr;
	    if(obj_ptr == NULL)
		continue;

	    /* Skip non player objects. */
	    if(obj_ptr->type != XSW_OBJ_TYPE_PLAYER)
		continue;
	    if(obj_ptr->score == NULL)
		continue;

            fprintf(fp,
 "%s;%s;%.2f;%.2f;%i;%.2f;%.2f;%.2f\n",
		obj_ptr->name,
		obj_ptr->empire,
		obj_ptr->score->damage_given,
		obj_ptr->score->damage_recieved,
		obj_ptr->score->kills,
		obj_ptr->score->credits,
		obj_ptr->score->rmu,
		obj_ptr->score->rmu_max
	    );
	}

	fclose(fp);


	return(0);
}
