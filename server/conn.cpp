/*
                         Connection Management

	Functions:

	int ConIsAllocated(int condescriptor)
	int ConIsConnected(int condescriptor)
	int ConIsLoggedIn(int condescriptor)

	int ConGetByObject(int object_num)
	int ConGetTop()
	void ConReclaim()
	void ConReset(int condescriptor)
	int ConCreateNew()
	void ConRecycle(int condescriptor)
	void ConDeleteAll()

	--- 

 */

#include "swserv.h"
#include "net.h"


/*
 *	Checks if condescriptor is allocated.
 */
int ConIsAllocated(int condescriptor)
{
        if((condescriptor < 0) ||
           (condescriptor >= total_connections) ||
           (connection == NULL)
        )
            return(0);
        else if(connection[condescriptor] == NULL)
            return(0);
	else
	    return(1);
}

/*
 *	Checks if condescriptor is connected.
 */
int ConIsConnected(int condescriptor)
{
	if(!ConIsAllocated(condescriptor))
	    return(0);
	else if(connection[condescriptor]->socket < 0)
	    return(0);
	else
	    return(1);
}

/*
 *	Checks if condescriptor is connected and logged in,
 *	by checking if the connection is allocated and its
 *	object is not -1.
 *
 *	Does not check if connection's object is valid.
 */
int ConIsLoggedIn(int condescriptor)
{
        /* Make sure condescriptor is valid. */
        if(!ConIsConnected(condescriptor))
            return(0);
        else if(connection[condescriptor]->object_num > -1)
            return(1);
	else
            return(0);
}

/*
 *	Gets first condescriptor that referances the given object
 *	number.
 *
 *	Returns -1 on error or no match or the connection descriptor
 *	(which can be assumed valid and connected).
 */
int ConGetByObject(int object_num)
{
	int i;
	connection_struct **ptr, *con_ptr;


	if(object_num < 0)
	    return(-1);

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

	    if(con_ptr->object_num == object_num)
		break;
	}
	if(i < total_connections)
	    return(i);
	else
	    return(-1);
}

/*
 *	Reclaims connection structure memory.
 */
void ConReclaim()
{
	int i, h;
	connection_struct **ptr, *con_ptr;


	if(total_connections < 0)
	    total_connections = 0;


	for(i = 0, h = -1, ptr = connection;
            i < total_connections;
            i++, ptr++
	)
	{
	    con_ptr = *ptr;
	    if(con_ptr == NULL)
		continue;

	    if(con_ptr->socket < 0)
		continue;

	    h = i;
	}


	for(i = total_connections - 1; i > h; i--)
	{
            free(connection[i]);
            connection[i] = NULL;
	}

	total_connections = h + 1;


        /* Reallocate or free pointer array. */
	if(total_connections > 0)
	{
            connection = (connection_struct **)realloc(
	        connection,
	        total_connections * sizeof(connection_struct *)
            );
            if(connection == NULL)
            {
	        total_connections = 0;
                return;
            }
        }
	else
	{
	    free(connection);
	    connection = NULL;

	    total_connections = 0;
	}


	return;
}

/*
 *	Returns highest con_descriptor number that is connected or
 *	-1 if there are no connections.
 */
int ConGetTop()
{
	int i, h;
	connection_struct **ptr, *con_ptr;


	for(i = 0, h = -1, ptr = connection;
            i < total_connections;
	    i++, ptr++
	)
	{
	    con_ptr = *ptr;
	    if(con_ptr == NULL)
		continue;

	    if(con_ptr->socket < 0)
		continue;

	    h = i;
	}


	return(h);
}

/*
 *	Reset connection structure values, deleting any
 *	allocated substructures but not the structure itself.
 *
 *	Connection's socket will NOT be disconnected.
 */
void ConReset(int condescriptor)
{
	connection_struct *con_ptr;


	if(ConIsAllocated(condescriptor))
	    con_ptr = connection[condescriptor];
	else
	    return;


        /* Reset connection values. */
	con_ptr->client_type = 0;
	con_ptr->is_guest = 0;

	con_ptr->socket = -1;
	con_ptr->conhost[0] = '\0';
	con_ptr->object_num = -1;
	con_ptr->contime = 0;
	con_ptr->bytes_recieved = 0;
	con_ptr->bytes_sent = 0;
	con_ptr->errors_recieved = 0;
	con_ptr->errors_sent = 0;

	con_ptr->badlogins = 0;

	con_ptr->obj_ud_interval = DEF_NET_UPDATE_INT;
	con_ptr->obj_ud_next = cur_millitime;

	con_ptr->conbuf_items = 0;


	return;
}



/*
 *	Delete all connections, closing any connected sockets of
 *	connections that are still connected.
 */
void ConDeleteAll()
{
	int i;
	connection_struct **ptr, *con_ptr;


	/* Close all connections first. */
	for(i = 0, ptr = connection;
            i < total_connections;
            i++, ptr++
	)
	{
	    con_ptr = *ptr;
            if(con_ptr == NULL)
                continue;

            /* Close connection standardly if connected. */
            if(con_ptr->socket > -1)
                NetCloseConnection(i);

            /* Reset connection values. */
            ConReset(i);
	}


	/* Deallocate all connections. */
        for(i = 0; i < total_connections; i++)
            free(connection[i]);

        free(connection);
        connection = NULL;

        total_connections = 0;


	return;
}

/*
 *	Create a new connection structure, return its number or
 *	-1 on error.
 *
 *	Also increments global highest_connection as needed.
 */
int ConCreateNew()
{
	int i, n;
	connection_struct **ptr, *con_ptr;


	/* Sanitize total. */
	if(total_connections < 0)
	    total_connections = 0;


	/* Search for available structure already allocated. */
	for(i = 0, ptr = connection;
	    i < total_connections;
	    i++, ptr++
	)
	{
	    con_ptr = *ptr;
	    if(con_ptr == NULL)
		break;

	    if(con_ptr->socket < 0)
	        break;
	}
	if(i < total_connections)
	{
            /* Got available pointer. */
	    n = i;
	}
	else
	{
            /* Need to allocate more pointers. */
	    n = total_connections;
	    total_connections++;

            connection = (connection_struct **)realloc(
	        connection,
	        total_connections * sizeof(connection_struct *)
	    );
            if(connection == NULL)
            {
	        total_connections = 0;
                return(-1);
            }

            /* Reset new pointer to NULL so it gets allocated below. */
	    connection[n] = NULL;
	}

	/* Allocate structure as needed. */
	if(connection[n] == NULL)
	{
            connection[n] = (connection_struct *)calloc(
		1,
		sizeof(connection_struct)
	    );
            if(connection[n] == NULL)
                return(-1);
        }


	/* Reset new connection. */
	ConReset(n);


        /* Increment global variable highest connection as
	 * needed.
	 */
        if(n >= highest_connection)
            highest_connection = n + 1;


	return(n);
}

/*
 *	Recycles connection.
 *
 *	Values will be reset, but socket will not be disconnected if
 *	it is still connected.
 */
void ConRecycle(int condescriptor)
{
	/* Make sure condescriptor is valid. */
	if(!ConIsAllocated(condescriptor))
	    return;

        /* Reset connection (closing any connected sockets). */
        ConReset(condescriptor);


	return;
}
