#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "../include/string.h"
#include "../include/urlparse.h"


/*
 *      Returns a statically allocated string indicating the
 *      protocol name or NULL on error.
 */
char *StringParseProtocol(char *url)
{
        char *strptr;
        static char rtnstr[MAX_URL_LEN];


        /* Error checks. */
        if(url == NULL)
            return(NULL);


        /* Copy urlstr to rtnstr. */  
        strncpy(rtnstr, url, MAX_URL_LEN);
        rtnstr[MAX_URL_LEN - 1] = '\0';


        /* Must be atleast one character long. */
        if(rtnstr[0] == '\0')
            return(NULL);

        /* Search for "://". */
	strptr = strstr(rtnstr, "://");
        if(strptr != NULL)
        {
            *strptr = '\0';
        }

	StringStripSpaces(rtnstr);


        return(rtnstr);
}



/*
 *      Returns a statically allocated string containing the
 *      name from url or NULL on error or if there was no name found.
 */
char *StringParseName(char *url)
{
        char workstr[MAX_URL_LEN];
        char *strptr;
        static char rtnstr[MAX_URL_LEN];


        /* Error checks. */
        if(url == NULL)
            return(NULL);


        /* Copy urlstr to rtnstr. */
        strncpy(rtnstr, url, MAX_URL_LEN);
        rtnstr[MAX_URL_LEN - 1] = '\0';   

        /* Must be atleast one character long. */
        if(rtnstr[0] == '\0')
            return(NULL);
         

        /* Search for "://". */
	strptr = strstr(rtnstr, "://");
        if(strptr != NULL)
        {
            strptr += 3; /* strlen("://") */
            strncpy(workstr, strptr, MAX_URL_LEN);
            workstr[MAX_URL_LEN - 1] = '\0';
            strncpy(rtnstr, workstr, MAX_URL_LEN);
            rtnstr[MAX_URL_LEN - 1] = '\0';
        }

        /* Search for "@". */
	strptr = strchr(rtnstr, '@');
        if(strptr != NULL)
        {
            *strptr = '\0';
        }   
        else
        {
            return(NULL);
        }   
         

        /* Search for ":". */
	strptr = strchr(rtnstr, ':');
        if(strptr != NULL)
        {
            *strptr = '\0';
        }

	StringStripSpaces(rtnstr);


        return(rtnstr);
}



/*
 *      Returns a statically allocated string containing the
 *      password from url or NULL on error or if there was no
 *      password found.
 */
char *StringParsePassword(char *url)
{
        char workstr[MAX_URL_LEN];
        char *strptr;
        static char rtnstr[MAX_URL_LEN];
            
        
        /* Error checks. */
        if(url == NULL)  
            return(NULL);


        /* Copy urlstr to rtnstr. */
        strncpy(rtnstr, url, MAX_URL_LEN);
        rtnstr[MAX_URL_LEN - 1] = '\0';

        /* Must be atleast one character long. */
        if(rtnstr[0] == '\0')
            return(NULL);
        

        /* Search for "://". */
	strptr = strstr(rtnstr, "://");
        if(strptr != NULL)
        {
            strptr += 3; /* strlen("://") */
            strncpy(workstr, strptr, MAX_URL_LEN);
            workstr[MAX_URL_LEN - 1] = '\0';
            strncpy(rtnstr, workstr, MAX_URL_LEN);
            rtnstr[MAX_URL_LEN - 1] = '\0';
        }

        /* Search for "@". */
	strptr = strchr(rtnstr, '@');
        if(strptr != NULL)
        {
            *strptr = '\0';
        }
        else
        {
            return(NULL);  
        }


        /* Search for ":". */
	strptr = strchr(rtnstr, ':');
        if(strptr != NULL)
        {
            strptr += 1; /* strlen(":") */
            strncpy(workstr, strptr, MAX_URL_LEN);
            workstr[MAX_URL_LEN - 1] = '\0';
            strncpy(rtnstr, workstr, MAX_URL_LEN);
            rtnstr[MAX_URL_LEN - 1] = '\0';
        }

        StringStripSpaces(rtnstr);


        return(rtnstr);
}



/*       
 *      Returns a statically allocated string containing the
 *      address from url or NULL on error or if there was no
 *      address found.
 */      
char *StringParseAddress(char *url)
{
        char workstr[MAX_URL_LEN];
        char *strptr;
        static char rtnstr[MAX_URL_LEN];


        /* Error checks. */
        if(url == NULL)
            return(NULL);


        /* Copy urlstr to rtnstr. */
        strncpy(rtnstr, url, MAX_URL_LEN);
        rtnstr[MAX_URL_LEN - 1] = '\0';

         
        /* Must be atleast one character long. */
        if(rtnstr[0] == '\0')
            return(NULL);


        /* Search for "://". */
	strptr = strstr(rtnstr, "://");
        if(strptr != NULL)
        {
            strptr += 3; /* strlen("://") */
            strncpy(workstr, strptr, MAX_URL_LEN);
            workstr[MAX_URL_LEN - 1] = '\0';
            strncpy(rtnstr, workstr, MAX_URL_LEN);
            rtnstr[MAX_URL_LEN - 1] = '\0';
        }
 
        /* Search for "@". */ 
	strptr = strchr(rtnstr, '@');
        if(strptr != NULL)
        {
            strptr += 1; /* strlen("@") */
            strncpy(workstr, strptr, MAX_URL_LEN);
            workstr[MAX_URL_LEN - 1] = '\0';
            strncpy(rtnstr, workstr, MAX_URL_LEN);
            rtnstr[MAX_URL_LEN - 1] = '\0';
        }
            
        /* Search for ":". */
	strptr = strchr(rtnstr, ':');
        if(strptr != NULL)
        {
            *strptr = '\0';
        }

	StringStripSpaces(rtnstr);


        return(rtnstr);
}



/*
 *      Returns the port number from url or -1 on error or if there
 *      was no port number.
 *
 */
int StringParsePort(char *url)
{
        char localstr[MAX_URL_LEN];
        char workstr[MAX_URL_LEN];
        char *strptr;


        /* Error checks. */
        if(url == NULL)
            return(-1);


        /* Copy urlstr to localstr. */
        strncpy(localstr, url, MAX_URL_LEN);
        localstr[MAX_URL_LEN - 1] = '\0';


        /* Must be atleast one character long. */
        if(localstr[0] == '\0')
            return(-1);


        /* Search for "://". */
	strptr = strstr(localstr, "://");
        if(strptr != NULL)
        {
            strptr += 3; /* strlen("://") */
            strncpy(workstr, strptr, MAX_URL_LEN);
            workstr[MAX_URL_LEN - 1] = '\0';
            strncpy(localstr, workstr, MAX_URL_LEN);
            localstr[MAX_URL_LEN - 1] = '\0';
        }

        /* Search for "@". */
	strptr = strchr(localstr, '@');
        if(strptr != NULL)
        {
            strptr += 1; /* strlen("@") */
            strncpy(workstr, strptr, MAX_URL_LEN);
            workstr[MAX_URL_LEN - 1] = '\0';
            strncpy(localstr, workstr, MAX_URL_LEN);
            localstr[MAX_URL_LEN - 1] = '\0';
        }

        /* Search for ":". */
	strptr = strchr(localstr, ':');
        if(strptr != NULL)
        {
            strptr += 1; /* strlen(":") */
            strncpy(workstr, strptr, MAX_URL_LEN);
            workstr[MAX_URL_LEN - 1] = '\0';
            strncpy(localstr, workstr, MAX_URL_LEN);
            localstr[MAX_URL_LEN - 1] = '\0';
        }

	StringStripSpaces(localstr);


        return(atoi(localstr));
}

