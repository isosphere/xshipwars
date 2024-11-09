#include "pconf.h"

void FREE(void *p);
int STRLEN(const char *s);
int STRISYES(const char *s);
char *STRDUP(const char *s);
const char *STRCHR(const char *s, int c);
void STRSTRIP(char *s);
int STRCASECMP(const char *s1, const char *s2);
int STRPFX(const char *str, const char *pfx);
int STRCASEPFX(const char *str, const char *pfx);
extern char *STRLISTAPPEND(char ***list, int *total, const char *s);
extern void STRFREEARRAY(char **list, int total);

char *GETENV(const char *parameter);

FILE *FOPEN(const char *path, const char *mode);
void FCLOSE(FILE *fp);
int FGETC(FILE *fp);
int FPUTC(int c, FILE *fp);
void FSEEKNEXTLINE(FILE *fp);
void FSEEKPASTSPACES(FILE *fp);
char *FGETLINE(FILE *fp);
char *FSEEKNEXTPARAMETER(
        FILE *fp, char *buf, char comment, char delim 
);



/*
 *	Safer form of free().
 */
void FREE(void *p)
{
	if(p == NULL)
	    return;

	free(p);

	return;
}

/*
 *	Returns length of string, does not distingush error.
 */
int STRLEN(const char *s)
{
	int len = 0;

	if(s == NULL)
	    return(0);

	while((*s) != '\0')
	{
	    len++;
	    s++;
	}

	return(len);
}

/*
 *	Returns true if string s implies a `yes'.
 */
int STRISYES(const char *s)
{
	if(s == NULL)
	    return(0);

	while(ISBLANK(*s))
	    s++;

	/* Yes? */
	if(toupper(*s) == 'Y')
	    return(1);
	/* On? */
	if(STRCASEPFX(s, "ON"))
	    return(1);
	/* True? */
	if(toupper(*s) == 'T')
	    return(1);
	/* All else non-zero number? */
	if((*s) != '0')
	    return(1);

	return(0);
}

/*
 *	Safer form of strdup().
 */
char *STRDUP(const char *s)
{
	int len;
	char *sr = NULL, *sr_ptr;

	if(s == NULL)
	    return(sr);

	len = STRLEN(s);
	sr = sr_ptr = (char *)malloc((len + 1) * sizeof(char));
	if(sr == NULL)
	    return(sr);

	while((*s) != '\0')
	{
	    *sr_ptr++ = *s++;
	}
	(*sr_ptr) = '\0';

	return(sr);
}

/*
 *	Returns the position of c in s or NULL on no match.
 */
const char *STRCHR(const char *s, int c)
{
	if((s == NULL) || ((char)c == '\0'))
	    return(NULL);

	while((*s) != '\0')
	{
	    if((*s) == (char)c)
		return(s);
	    s++;
	}

	return(NULL);
}

/*
 *      Strips blank characters leading and tailing string s.
 */
void STRSTRIP(char *s)
{
        int tar, src, lead, tail;

        if(s == NULL)
            return;
        if((*s) == '\0')
            return;

        /* Strip leading blank characters. */
        lead = 0;
        while(ISBLANK(s[lead]))
            lead++;

        if(lead > 0)
        {
            for(tar = 0, src = lead; s[src] != '\0'; tar++, src++)
                s[tar] = s[src];

            s[tar] = '\0';

            /* Calculate tail position. */
            tail = (tar > 0) ? tar - 1 : 0;
        }
        else
        {
            /* Calculate tail position. */
            tar = STRLEN(s);
            tail = (tar > 0) ? tar - 1 : 0;
        }

        /* Strip tailing blank characters. */
        for(tar = tail; tar >= 0; tar--)
        {
            if(ISBLANK(s[tar]))  
                s[tar] = '\0';
            else
                break;
        }

        return;
}


/*
 *	Returns 1 for no match and 0 for match.
 */
int STRCASECMP(const char *s1, const char *s2)
{
        if((s1 == NULL) ||
           (s2 == NULL)
        )
            return(1);  /* False. */

        while((*s1) && (*s2))
        {
            if(toupper(*s1) != toupper(*s2))
                return(1);      /* False. */

            s1++;
            s2++;
        }

        if((*s1) == (*s2))
            return(0);	/* True. */
        else
            return(1);	/* False. */
}

/*
 *      Returns true if pfx is a prefix of str (case insensitive),
 *	otherwise returns false.
 */
int STRPFX(const char *str, const char *pfx)
{
        /* If either strings is NULL, return false. */
        if((str == NULL) ||
           (pfx == NULL)
        )       
            return(0);

        /* If pfx contains no characters, return false. */
        if((*pfx) == '\0') 
            return(0);  

        /* Begin prefix matching. */
        while((*pfx) != '\0')
        {
            if((*str) != (*pfx))
                return(0);

            str++; pfx++;
        }

        return(1);
}

/*
 *	Returns true if pfx is a prefix of str (case insensitive),
 *	otherwise returns false.
 */
int STRCASEPFX(const char *str, const char *pfx)
{
        /* If either strings is NULL, return false. */
        if((str == NULL) ||
           (pfx == NULL)
        )
            return(0);

        /* If pfx contains no characters, return false. */
        if((*pfx) == '\0')
            return(0);

        /* Begin prefix matching. */
        while((*pfx) != '\0')
        {
            if(toupper(*str) != toupper(*pfx))  
                return(0);

            str++; pfx++;
        }

        return(1);
}

/*
 *	Dupliates string s (may not be NULL) and adds it to the given
 *	list.
 *
 *	Reallocates the given list and increases the total.
 *
 *	Returns pointer to duplicated string or NULL on error.
 */
char *STRLISTAPPEND(char ***list, int *total, const char *s)
{
	int i;
	char *ns;

	if((list == NULL) || (total == NULL) || (s == NULL))
	    return(NULL);

	if((*total) < 0)
	    (*total) = 0;

	ns = STRDUP(s);
	if(ns == NULL)
	    return(NULL);

	i = (*total);
	(*total) = i + 1;

	(*list) = (char **)realloc(
	    *list, (*total) * sizeof(char *)
	);
	if((*list) == NULL)
	{
	    (*total) = 0;
	    FREE(ns);
	    return(NULL);
	}

	(*list)[i] = ns;	

	return(ns);
}

/*
 *	Deallocates the given list of strings.
 */
void STRFREEARRAY(char **list, int total)
{
	int i;

	if(list == NULL)
	    return;

	for(i = 0; i < total; i++)
	    FREE(list[i]);

	FREE(list);

	return;
}


/*
 *	Safe form of getenv().
 */
char *GETENV(const char *parameter)
{
	if(parameter == NULL)
	    return(NULL);

	return(getenv(parameter));
}


/*
 *	Opens a file, returns its handle or NULL on failure. Works
 *	just like ansi fopen().
 */
FILE *FOPEN(const char *path, const char *mode)
{
	if((path == NULL) || (mode == NULL))
	    return(NULL);
	else
	    return(fopen(path, mode));
}

/*
 *	Closes a file opened by FOPEN.
 */
void FCLOSE(FILE *fp)
{
	if(fp == NULL)
	    return;

	fclose(fp);

	return;
}

/*
 *	Safe form of fgetc.
 */
int FGETC(FILE *fp)
{
	if(fp == NULL)
	    return(EOF);
	else
	    return(fgetc(fp));
}

/*
 *	Safe form of fputc.
 */
int FPUTC(int c, FILE *fp)
{
	if(fp == NULL)
	    return(EOF);
	else
	    return(fputc(c, fp));
}


/*
 *	Seeks to next line, escape sequences will be parsed.
 */
void FSEEKNEXTLINE(FILE *fp)
{
        int c;

        if(fp == NULL)
            return;

        do
        {
            c = FGETC(fp);

            /* Escape sequence? */
            if(c == '\\')
                c = FGETC(fp);
            /* New line? */
            else if(ISCR(c))
                break;

        } while(c != EOF);

        return;
}

/*
 *      Seeks fp past any spaces.
 */
void FSEEKPASTSPACES(FILE *fp)
{
        int c;

        if(fp == NULL)
            return;

        while(1)
        {
            c = FGETC(fp);
            if(c == EOF)
                break;
  
            if(ISBLANK(c))
                continue;
         
            fseek(fp, -1, SEEK_CUR);
            break;
        }

        return;
}

/*
 *	String is loaded literally and only escape sequence handled is
 *	the occurance of two characters '\\' '\n' and where the '\n'
 *	character will be saved into the return string.
 *
 *	Spaces will not be striped, the fp will be positioned after the
 *	non-escaped newline or EOF (whichever is encountered first).
 *
 *	Return must be deallocated by calling function.
 */
char *FGETLINE(FILE *fp)
{
        int c, i, len = 0;
        char *strptr = NULL;
        char *strptr2;

        if(fp == NULL)
            return(strptr);

        /* Begin reading string from file. */

        /* Get first character. */
        c = FGETC(fp);
        if(c == EOF)
            return(strptr);

        /* Read string. */
        while(1)
        {           
            i = len;    /* Current string index i. */
            len++;      /* Current string length. */

            /* Reallocate string buffer. */
            strptr = (char *)realloc(strptr, len * sizeof(char));
            if(strptr == NULL)
                break;

            strptr2 = &(strptr[i]);     /* Pointer to current string index. */
            (*strptr2) = c;             /* Set new character value. */

            /* End of the line? */
            if((c == EOF) ||
               ISCR(c)
            )
            {
                (*strptr2) = '\0';
                break;
            }
            /* Escape sequence? */
            else if(c == '\\')
            {
                /* Read next character after backslash. */
                c = FGETC(fp);
                if(c == EOF)
                {
                    continue;
                }   
                else if(ISCR(c))
                {
                    /* New line, store it as is. */
                    (*strptr2) = c;
                }
                else if(c == '\\')
		{
		    /* Another backslash, store as a single backslash. */
		    (*strptr2) = c;
		}
		else
                {
                    /* All other escaped characters leave as is
                     * it will be set on the next loop.
                     */
                    continue; 
                }

                /* Read next character. */
                c = FGETC(fp);
            }
            /* Regular character. */
            else
            {
                /* Read next character. */
                c = FGETC(fp);
            }
        }

        return(strptr);
}

/*
 *	Fetches the next parameter found at the file position fp.
 *
 *	If buf is NULL then a newly allocated string will be returned
 *	containing the fetched parm. If buf is not NULL, then buf will
 *	be realloc()'ed and returned as a new pointer containing
 *	the fetched parm.
 *
 *	If EOF is reached by the given fp position, then NULL will
 *	be returned and the given buf will have been free()ed by this
 *	function.
 */
char *FSEEKNEXTPARAMETER(
        FILE *fp, char *buf, char comment, char delim 
)
{
#ifndef FREAD_ALLOC_CHUNK_SIZE
# define FREAD_ALLOC_CHUNK_SIZE	8
#endif
        int c, buf_pos = 0, buf_len, buf_inc = FREAD_ALLOC_CHUNK_SIZE;

        if(fp == NULL)
        {
            FREE(buf);
            return(NULL);
        }

        /* Get length of buf (less than actual allocated is okay). */
        buf_len = ((buf == NULL) ? 0 : STRLEN(buf));

        /* Seek past spaces and comments to next parameter. */
        while(1)
        {
            FSEEKPASTSPACES(fp);
            c = FGETC(fp);
            if(c == EOF)
            {
                FREE(buf);
                return(NULL);
            }   
            else if(c == comment)
            {
                FSEEKNEXTLINE(fp);
                continue;
            }
            else if(ISCR(c))
            {
                continue;
            }
            else
            {
                fseek(fp, -1, SEEK_CUR);
                break;
            }
        }

        /* Begin fetching this parm. */
        while(1)
        {
            /* Get next char. */
            c = FGETC(fp);
            if(c == EOF)
            {
                break;
            }   

            /* Blank character reached? */
            if(ISBLANK(c))
            {
                /* Blank char reached, seek past delimiantor and position
                 * fp at beginning of value.
                 */
                if(delim == '\0')
                {
                    FSEEKPASTSPACES(fp);
                }
                else
                {
                    FSEEKPASTSPACES(fp);
             
                    /* Seek to deim or newline. */
                    do
                    {
                        c = FGETC(fp);
                        if((c == EOF) || (c == delim))
                            break;
             
                        if(ISCR(c))
                        {
                            fseek(fp, -1, SEEK_CUR);
                            break;
                        }
        
                    } while(1);
            
                    FSEEKPASTSPACES(fp);
                }
                break;
            }

            /* CR reached? */
            if(ISCR(c))
            {
                fseek(fp, -1, SEEK_CUR);
                break;
            }

            /* Deliminator reached? */  
            if(c == delim)
            {
                FSEEKPASTSPACES(fp);
                break;
            }

            /* Need to allocate buffer? */
            if(buf_pos <= buf_len)
            {
                buf_len += buf_inc;
                            
                buf = (char *)realloc(buf, buf_len * sizeof(char));
                if(buf == NULL)
                {
                    FSEEKNEXTLINE(fp);
                    return(NULL); 
                }
            }

            buf[buf_pos] = (char)c;
            buf_pos++;
        }

        /* Put null terminating byte on buffer. */
        if(c == EOF)
        {
            FREE(buf); 
            buf = NULL;
        }
        else
        {
            if(buf_pos <= buf_len)
            {
                buf_len = buf_pos + 1;
             
                buf = (char *)realloc(buf, buf_len * sizeof(char));
                if(buf == NULL)
                    return(NULL);
            }       
            buf[buf_pos] = '\0';
        }

        return(buf);
}
