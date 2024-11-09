#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

//extern char *tzname[2];

#include <time.h>
#include "../include/os.h"

#ifdef __MSW__

#else
# include <sys/time.h>
#endif

#include "../include/cfgfmt.h"
#include "../include/cs.h"
#include "../include/string.h"

#ifdef MEMWATCH
# include "memwatch.h"
#endif


/* Sort function used by StringQSort(). */
static int SORT(const void *a, const void *b);

/* Basic string handling. */
int strlinelen(const char *s);
int strlongestline(const char *s);
int strlines(const char *s);
#ifdef __MSW__
int strcasecmp(const char *s1, const char *s2);
#endif	/* __MSW__ */

const char *strseekblank(const char *s);
//char *strcasestr(const char *haystack, const char *needle);
int strpfx(const char *str, const char *pfx);
int strcasepfx(const char *str, const char *pfx);
void strtoupper(char *s);
void strtolower(char *s);
char *strcatalloc(char *orig, const char *new_str);
void substr(char *s, const char *token, const char *val);
void strset(char *s, char c, int n);
void strpad(char *s, int n);
void straddflag(char *s, const char *flag, char operation, int len);

char *StringCopyAlloc(const char *str);
void *MemoryCopyAlloc(const void *ptr, int nbytes);
char **StringCopyArray(const char **strv, int strc);
void StringStripSpaces(char *s);
char **StringQSort(char **strings, int nitems);
char *StringTailSpaces(char *string, int len);
void StringShortenFL(char *string, int limit);
void StringFreeArray(char **strv, int strc);

/* Configuration value string checking. */
int StringIsYes(const char *string);
int StringIsComment(const char *s, char c);
char *StringCfgParseParm(const char *string);
char *StringCfgParseValue(const char *string);

/* Other types of string parsing. */
int StringParseStdColor(
	const char *string,
	u_int8_t *r_rtn, u_int8_t *g_rtn, u_int8_t *b_rtn
);
int StringParseIP(
	const char *string,
	u_int8_t *c1, u_int8_t *c2, u_int8_t *c3, u_int8_t *c4
);

/* ShipWars CyberSpace protocol string parsing. */
int StringGetNetCommand(const char *str);
char *StringGetNetArgument(const char *str);

/* Time string handling. */
char *StringCurrentTimeFormat(const char *format);
char *StringTimeFormat(const char *format, time_t seconds);
char *StringFormatTimePeriod(time_t seconds);


/*
 *	Returns the length in bytes, of the line s.
 *	The end point character must be either a '\n', '\r', or
 *	'\0'.
 */
int strlinelen(const char *s)
{
	int i = 0;


	if(s == NULL)
	    return(0);

	while((*s != '\0') &&
              (*s != '\n') &&
              (*s != '\r')
	)
	{
	    i++;
	    s++;
	}

	return(i);
}


/*
 *	Returns the length of the longest line in string s.
 */
int strlongestline(const char *s)
{
	int n;
	int longest = 0;


        if(s == NULL)
	    return(longest);

	while(1)
	{
	    n = strlinelen(s);

	    if(n > longest)
		longest = n;

	    s += n;

	    if(*s == '\0')
		break;

	    s++;
	}

	return(longest);
}

/*
 *	Returns the number of '\r' or '\n' characters + 1
 *	in string s.
 *
 *	If the first character is '\0' in string s or string s
 *	is NULL, then 0 will be returned.
 */
int strlines(const char *s)
{
	int lines = 0;


        if(s == NULL)
	    return(lines);
	if(*s == '\0')
	    return(lines);

	/* Must increment first line. */
	lines++;

	while(*s != '\0')
	{
	    if((*s == '\r') ||
               (*s == '\n')
	    )
		lines++;

	    s++;
	}

	return(lines);
}

#ifdef __MSW__
/*
 *	Works just like the UNIX strcasecmp(). Returns 1 for no match
 *	and 0 for match.
 */
int strcasecmp(const char *s1, const char *s2)
{
	if((s1 == NULL) ||
           (s2 == NULL)
	)
	    return(1);	/* False. */

	while((*s1) && (*s2))
	{
	    if(toupper(*s1) != toupper(*s2))
		return(1);	/* False. */

	    s1++;
	    s2++;
	}
	if(*s1 == *s2)
	    return(0);	/* True. */
	else
	    return(1);	/* False. */
}
#endif

/*
 *	Returns pointer in string s which is the first blank character
 *	(a space or tab). Can return NULL if no blank character is found.
 */
const char *strseekblank(const char *s)
{
	if(s == NULL)
	    return(NULL);

	while(!ISBLANK(*s) && ((*s) != '\0'))
	    s++;

	if((*s) == '\0')
	    return(NULL);
	else
	    return(s);
}

/*
 *	Case insensitive version of strstr(). Returns the pointer to
 *	needle in haystack if found or NULL on no match.
 */
// char *strcasestr(const char *haystack, const char *needle)
// {
// 	const char *strptr1, *strptr2, *strptr3;

//         if((haystack == NULL) || (needle == NULL))
//             return(NULL);

// 	/* Get starting position in haystack as strptr1. */
//         strptr1 = haystack;
// 	/* Itterate strptr1 in haystack till end of string. */
//         while((*strptr1) != '\0')
//         {
//             strptr2 = needle;	/* Get starting position of needle. */

// 	    /* Character in needle and haystack at same position the same
// 	     * (case insensitive ofcourse).
// 	     */
//             if(toupper(*strptr1) == toupper(*strptr2))
//             {
//                 strptr3 = strptr1;      /* Record starting position. */

//                 strptr1++;
//                 strptr2++;

//                 /* Continue itterating through haystack. */
//                 while((*strptr1) != '\0')
//                 {
// 		    /* Needle string ended? */
//                     if((*strptr2) == '\0')
//                     {
//                         return((char *)strptr3);
//                     }
// 		    /* Characters differ (case insensitive)? */
//                     else if(toupper(*strptr1) != toupper(*strptr2))
//                     {
//                         strptr1++;
//                         break;
//                     }
// 		    /* Characters are still matching. */
//                     else
//                     {
//                         strptr1++;
// 			strptr2++;
//                     }
//                 }
// 		/* End of needle reached, we got a match. */
//                 if((*strptr2) == '\0')
//                     return((char *)strptr3);
//             }
//             else
//             {
//                 strptr1++; 
//             }
//         }

//         return(NULL);
// }

/*
 *	Returns 1 if pfx is a prefix of str.
 *
 *	Returns 0 if pfx is not a prefix of str or if an error
 *	occured.
 */
int strpfx(const char *str, const char *pfx)
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

int strcasepfx(const char *str, const char *pfx)
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
 *	Sets all characters in string s to upper.
 */
void strtoupper(char *s)
{
	if(s == NULL)
	    return;

        while(*s != '\0')
            *s++ = toupper(*s);

	return;
}

void strtolower(char *s)
{
	if(s == NULL)
            return;

	while(*s != '\0')
	    *s++ = tolower(*s);

	return;
}

/*
 *	Takes the given string orig and reallocates it with enough
 *	memory to hold itself and the new string plus a null terminating
 *	byte at the end.
 *
 *	Returns the new pointer which needs to be free()'ed by the calling
 *	function.
 *
 *	The given pointer orig should never be referanced again after this
 *	call.
 */
char *strcatalloc(char *orig, const char *new_str)
{
	int orig_len, new_len;
	char *new_rtn = NULL;


	/* If new string is NULL, then return original pointer since we
	 * have changed nothing.
	 */
	if(new_str == NULL)
	    return(orig);

	/* Calculate lengths of original and new. */
	orig_len = ((orig == NULL) ? 0 : strlen(orig));
	new_len = strlen(new_str);
	if((orig_len + new_len) < 0)
	{
	    orig_len = 0;
	    new_len = 0;
	}

	/* Reallocate new string return. */
	new_rtn = (char *)realloc(
	    orig, (orig_len + new_len + 1) * sizeof(char)
	);
	if(new_rtn != NULL)
	{
	    /* Original given string was NULL? */
	    if(orig == NULL)
		(*new_rtn) = '\0';

	    /* Cat new string to the new return string. */
	    strcat(new_rtn, new_str);
	}

	return(new_rtn);
}

/*
 *	Substitutes all occurances of string token in string s
 *	with string val.  String s must have enough capacity for
 *	all substitutions.
 *
 *	Example: substr("Hi there %name!", "%name", "Kattie")
 *	Turns into: "Hi there Kattie!"
 */
void substr(char *s, const char *token, const char *val)
{
        int i, tl, vl;
        char *strptr1, *strptr2, *strptr3, *strptr4;


        if((s == NULL) ||
           (token == NULL)
        )
            return;

	if(val == NULL)
	    val = "";

	/* Token string must not be empty. */
        if(*token == '\0')
            return;

	/* Token and value strings may not have the same content. */
	if(!strcmp(val, token))
	    return;

	/* Get lengths of token and value strings. */
	tl = strlen(token);
        vl = strlen(val);

	/* Set strptr1 to begining of string s. */
	strptr1 = s;

	/* Begin substituting. */
        while(1)
        {
	    /* Seek next instance of token string. */
            strptr1 = strstr(strptr1, token);
            if(strptr1 == NULL)
                break;

            /* Calculate end pointer of strptr1. */
            i = strlen(strptr1);
            strptr2 = strptr1 + i;

	    if(tl > vl)
	    {
		/* Token string is longer than value string. */

		/* Calculate starting pointer positions. */
		strptr3 = strptr1 + vl;
		strptr4 = strptr1 + tl;

		/* Shift tailing portion of string. */
		while(strptr4 <= strptr2)
		    *strptr3++ = *strptr4++;
	    }
	    else if(tl < vl)
	    {
		/* Token string is less than value string. */

		/* Calculate starting pointer positions. */
		strptr3 = strptr2;
		strptr4 = strptr2 + vl - tl;

                /* Shift tailing portion of string. */
                while(strptr3 > strptr1) 
                    *strptr4-- = *strptr3--;
	    }

	    memcpy(strptr1, val, vl);

	    /* Increment strptr1 past length of value. */
	    strptr1 = strptr1 + vl;
        }

        return;
}

#ifndef __MSW__
/*
 *	Sets the first n characters of string s to be the
 *	value of c.  A null character will be tacked on at the end.
 *
 *	String s must have enough storage for n characters plus the
 *	null terminating character tacked on at the end.
 */
void strset(char *s, char c, int n)
{
	int i;


	if(s == NULL)
	    return;

	for(i = 0; i < n; i++)
	    s[i] = c;

	s[i] = '\0';

	return;
}
#endif

#ifndef __MSW__
/*
 *	Same as strset(), except always sets the first n characters
 *	of string s to the ' ' character. Tacks on a null terminating
 *	character at the end.
 */
void strpad(char *s, int n)
{
	strset(s, ' ', n);

	return;
}
#endif

/*
 *      Concatonates flag string, putting in the operation character
 *      if there is one or more flags already existing in s.
 * 
 *      "Close | Open" "Inventory" '|'
 * 
 *      Becomes:
 * 
 *      "Close | Open | Inventory"
 *	Dan S: Renaming arg 3 from "operator" to "operation", "operator" is a C+ key word.
 */
void straddflag(char *s, const char *flag, char operation, int len)
{
        int s_len, flag_len;


        if((s == NULL) ||   
           (flag == NULL) ||
           (len < 1)
        )
            return;

        s_len = strlen(s);
        flag_len = strlen(flag);

        /* Put operation after last flag if there is one. */
        if((s_len > 0) &&
           ((len - s_len) > 3)
        )
        {
            int i;

            i = s_len;
  
            s[i] = ' '; i++;
            s[i] = operation; i++;
            s[i] = ' '; i++;
            s[i] = '\0';
   
            s_len += 3;
        }
 
        /* Concatonate flag string. */
        if((len - s_len - 1) > 0)
            strncat(s, flag, len - s_len - 1);

        /* Null terminuate just in case. */
        s[len - 1] = '\0';

        return;
}


/*
 *	Returns an allocated string which is a copy of the given
 *	string str.   Can return NULL on error.
 */
char *StringCopyAlloc(const char *str)
{
        int len;
        char *strptr = NULL;


        if(str == NULL)
            return(strptr);

        len = strlen(str);
	if(len < 0)
	    len = 0;

        strptr = (char *)malloc((len + 1) * sizeof(char));
        if(strptr != NULL)
	{
            strncpy(strptr, str, len);
	    strptr[len] = '\0';
	}

        return(strptr);
}

/*
 *	Allocates a new segment of memory of length specified by
 *	nbytes with identical content as specified by ptr.
 *
 *	The given ptr must be atleast nbytes long.
 */
void *MemoryCopyAlloc(const void *ptr, int nbytes)
{
	void *ptr2 = NULL;


	if(nbytes < 1)
	    return(ptr2);

	ptr2 = malloc(nbytes);
	if(ptr2 != NULL)
	{
	    if(ptr == NULL)
		memset(ptr2, 0x00, nbytes);
	    else
		memcpy(ptr2, ptr, nbytes);
	}

	return(ptr2);
}

/*
 *	Makes a duplicate of the given array and each pointed to string.
 *
 *	Returns the newly allocated pointer array pointing to a new
 *	array (of the same order and size) of coppied strings.
 */
char **StringCopyArray(const char **strv, int strc)
{
	int i;
	char **strv_rtn;

	if((strv == NULL) || (strc < 1))
	    return(NULL);

	strv_rtn = (char **)malloc(
	    strc * sizeof(char *)
	);
	if(strv_rtn == NULL)
	    return(NULL);

	for(i = 0; i < strc; i++)
	{
	    strv_rtn[i] = ((strv[i] == NULL) ?
		NULL : strdup((const char *)strv[i])
	    );
	}

	return(strv_rtn);
}


/*
 *	Strips blank characters leading and tailing string s.
 */
void StringStripSpaces(char *s)
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
            tar = strlen(s);
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
 *	This function is used by StringQSort() as the sort function.
 */
static int SORT(const void *a, const void *b)
{
        char *x, *y;
        x = *((char **)a);
        y = *((char **)b);
        return(strcmp(x, y));
}

/*
 *	Sorts given string array strings using the qsort() methoid.
 *	Returns NULL on error and strings on success.
 */
char **StringQSort(char **strings, int nitems)
{
        if(strings == NULL)
            return(NULL);
        
        if(nitems <= 0)
            return(NULL);

        qsort(strings, nitems, sizeof(char *), SORT);
        
        return(strings);
}


/*
 *	Puts space characters at the end of the given string's current
 *	contents up to the specified length len. The allocation of string
 *	must be len + 1.
 */
char *StringTailSpaces(char *string, int len)
{
	int x;
	int prev_len;

	if(string == NULL)
	    return(NULL);

	/* Length of given string assumed to have len + 1 to accomidate
	 * for a null terminating byte.
	 */
	string[len] = '\0';
	prev_len = strlen(string);

	for(x = prev_len; x < len; x++)
	    string[x] = ' ';

	return(string);
}


/*
 *      Shortens string to number of characters limit.
 *      If the limit is > 3 then three '.' characters are prepended.
 *
 *      If length of string is <= to limit, then nothing is done,
 *	example (using 10 as the limit):
 *
 *	"Hello there kitty!" becomes "... kitty!"
 */
void StringShortenFL(char *string, int limit)
{
        int ol;
        char *strptr1, *strptr2;


        if(string == NULL)
            return;

        if(limit < 0)
        {
            if(*string != '\0')
                *string = '\0';

            return;
        }

        ol = strlen(string);
        if(ol > limit)
        {
            strptr1 = string;
            strptr2 = &string[ol - limit];

            while(*strptr2 != '\0')
                *strptr1++ = *strptr2++;

            if(limit >= 3)
            {
                strptr1 = string;
                strptr2 = &string[3];
                while(strptr1 < strptr2)
                    *strptr1++ = '.';
           }

            string[limit] = '\0';
        }

        return;
}


/*
 *	Frees an array of strings.  strc indicates the number of
 *	allocated strings in the array pointer strv.
 *
 *	If strv is NULL and/or strc is <= 0 then nothing will be
 *	done.
 */
void StringFreeArray(char **strv, int strc)
{
	int i;


	if(strv == NULL)
	    return;


	/* Free each string in array. */
	for(i = 0; i < strc; i++)
	    free(strv[i]);

	/* Free array pointers. */
	free(strv);


	return;
}




/*
 *	Checks if the string is a "yes" (and standard variations
 *	accepted and checked as well).
 */
int StringIsYes(const char *string)
{
	if(string == NULL)
	    return(0);

	/* Skip leading spaces. */
	while(ISBLANK(*string))
	    string++;

	/* Is first char a number from 0 to 9 (for "0" or "1")? */
	if(isdigit(*string))
	{
	    if((*string) != '0')
		return(1);
	    else
		return(0);
	}
	/* Is first char a 'o' (for "on" or "off")? */
	else if(toupper(*string) == 'O')
	{
	    /* Check second char, is it an 'n'? */
            if(toupper(string[1]) == 'N')
                return(1);
            else
                return(0);
	}
	/* Else check first char for "yes" or "no". */
	else
	{
	    if(toupper(*string) == 'Y')
	        return(1);
	    else
	        return(0);
	}

	return(0);
}

/*
 *	Returns true if string is a comment in accordance with
 *	the UNIX configuration file format.
 *
 *	The comment character c should be
 *	(but does not have to be) UNIXCFG_COMMENT_CHAR.
 */
int StringIsComment(const char *s, char c)
{
	/* Is string NULL? */
        if(s == NULL)
            return(0);

	/* Skip leading spaces. */
	while(ISBLANK(*s))
	    s++;

	if((*s) == c)
	    return(1);
	else
	    return(0);
}

/*
 *	Returns the parameter section of string which should comform to
 *	the standard configuration format of "<parameter>=<value>" as
 *	a statically allocated string or NULL on error.
 */
char *StringCfgParseParm(const char *string)
{
	int x, y;
        int got_parm_start;
        static char parameter[CFG_PARAMETER_MAX];


        /* Is string empty? */
        if(string == NULL)
            return(NULL);
        if(((*string) == '\0') ||
           ((*string) == '\r') ||
           ((*string) == '\n')
        )
            return(NULL);

        /* Is string a comment? */
        if(StringIsComment(string, UNIXCFG_COMMENT_CHAR))
            return(NULL);

        /* Begin fetching parameter from string. */
        got_parm_start = 0;
        for(x = 0, y = 0;
            (x < CFG_STRING_MAX) && (y < CFG_PARAMETER_MAX);
            x++
	)
        {
	    /* Skip newline escape sequences. */
	    if((string[x] == '\\') &&
               ((x + 1) < CFG_STRING_MAX)
	    )
	    {
		if((string[x + 1] == '\n') || (string[x + 1] == '\r'))
	        {
		    x++;
	            continue;
	        }
	    }

	    /* Skip other escape sequences. */
	    if(string[x] == '\\')
            {   
                x++;
                if(x >= CFG_STRING_MAX)
                    break;
            }


	    /* End on NULL, new line, or delimiter. */
            if((string[x] == '\0') ||
               (string[x] == '\r') ||
               (string[x] == '\n') || 
               (string[x] == CFG_PARAMETER_DELIMITER)
            )
            {
                parameter[y] = '\0';
                break;
            }   

            if(got_parm_start == 0)
            {
                if((string[x] == ' ') ||
                   (string[x] == '\t')
                )
                    continue;
                else
                    got_parm_start = 1;
            }

            parameter[y] = string[x];
            y++;
        }

        /* Null terminate parameter. */  
        parameter[CFG_PARAMETER_MAX - 1] = '\0';
	StringStripSpaces(parameter);

        return(parameter);
}

/*
 *      Returns the value section of string which should comform to
 *      the standard configuration format of "<parameter>=<value>".
 *
 *      The returned string will never be NULL and will be striped of
 *      tailing or leading spaces.
 */
char *StringCfgParseValue(const char *string)
{
        int x, y, got_value;
        static char value[CFG_VALUE_MAX];


        /* Is string empty? */
        if(string == NULL)
            return("");
	if((string[0] == '\0') ||
           (string[0] == '\r') ||
           (string[0] == '\n')
        )
            return("");

        /* Is string a comment? */
        if(StringIsComment(string, UNIXCFG_COMMENT_CHAR))
            return("");

        /* Does string have a delimiter? */
        if(strchr(string, CFG_PARAMETER_DELIMITER) == NULL)
            return("");


        /* Begin fetching value from string. */
        got_value = 0;
        for(x = 0, y = 0;
            (x < CFG_STRING_MAX) && (y < CFG_VALUE_MAX);
            x++
	)
        {
            /* Skip newline escape sequences. */
            if((string[x] == '\\') &&
               ((x + 1) < CFG_STRING_MAX)
            )
            {
                if((string[x + 1] == '\n') || (string[x + 1] == '\r'))
                {
                    x++;
                    continue;
                }
            }

            /* Skip other escape sequences. */
            if(string[x] == '\\')
	    {
                x++;
		if(x >= CFG_STRING_MAX)
		    break;
	    }

	    /* Stop on newline or NULL. */
            if((string[x] == '\0') || 
               (string[x] == '\r') ||
               (string[x] == '\n')
            )
            {
                value[y] = '\0';
                break;
            }
        
            if(got_value == 0)
            {
                if(string[x] == CFG_PARAMETER_DELIMITER)
                {
                    got_value = 1;
                    continue;
                }
                else
                {
                    continue;
                }
            }
                
        
            value[y] = string[x];
            y++;
        }

        /* Null terminate value. */
        value[CFG_VALUE_MAX - 1] = '\0';
	StringStripSpaces(value);        

        return(value);
}


/* ***********************************************************************
 *
 *                          Other Parsing
 */

/*
 *	Parses a standard color string "#rrggbb" where rr, gg,
 *	and bb are in hexidecimal notation.
 *
 *	Returns 0 on success, -1 on general error, and
 *	-2 for incomplete or ambiguous.
 */
int StringParseStdColor(
	const char *s,
	u_int8_t *r_rtn,
	u_int8_t *g_rtn,
	u_int8_t *b_rtn
)
{
        int i;
        int r = 0;
        int g = 0;
        int b = 0;


        if(s == NULL)
	    return(-1);


        /* Red. */
        while((*s == '#') || ISBLANK(*s))
            s++;
        if(!*s)
	    return(-2);

        i = 0;  
        while(isxdigit(*s) && (i < 2))
        {   
            if(isdigit(*s))
                r = (r << 4) + (*s - '0');
            else
                r = (r << 4) + (tolower(*s) - 'a' + 10);

            i++; s++;
        }
        if(r_rtn != NULL)
	    *r_rtn = (u_int8_t)r;

        /* Green. */
        i = 0;
        while(isxdigit(*s) && (i < 2))
        {
            if(isdigit(*s))
                g = (g << 4) + (*s - '0');
            else  
                g = (g << 4) + (tolower(*s) - 'a' + 10);

            i++; s++;
        }
        if(g_rtn != NULL)
	    *g_rtn = (u_int8_t)g;

        /* Blue. */
        i = 0;
        while(isxdigit(*s) && (i < 2))
        {
            if(isdigit(*s))
                b = (b << 4) + (*s - '0');
            else
                b = (b << 4) + (tolower(*s) - 'a' + 10);

            i++; s++;
        }
        if(b_rtn != NULL)
	    *b_rtn = (u_int8_t)b;


	return(0);
}

/*
 *	Parse IP address.
 *
 *      Returns 0 on success, -1 on general error, and
 *      -2 for incomplete or ambiguous.
 */
int StringParseIP(
        const char *s,
        u_int8_t *c1, u_int8_t *c2, u_int8_t *c3, u_int8_t *c4 
)
{
	const char *cstrptr;
	char *strptr;
	char ls[4];


	if(s == NULL)
	    return(-1);

        while(ISBLANK(*s))
            s++;

        if((*s) == '\0')
            return(-2);

	/* First number. */
	if(c1 != NULL)
	{
	    strncpy(ls, s, 4); ls[3] = '\0';

	    strptr = strchr(ls, '.');
	    if(strptr != NULL)
		(*strptr) = '\0';

	    (*c1) = (u_int8_t)atoi(ls);
	}
	cstrptr = strchr(s, '.');
	if(cstrptr == NULL)
	    return(-2);
	s = cstrptr + 1;

        /* Second number. */
        if(c2 != NULL)
        {
            strncpy(ls, s, 4); ls[3] = '\0';

            strptr = strchr(ls, '.');
            if(strptr != NULL) 
                (*strptr) = '\0';

            (*c2) = (u_int8_t)atoi(ls);
        }
        cstrptr = strchr(s, '.');
        if(cstrptr == NULL)
            return(-2);
        s = cstrptr + 1;

        /* Third number. */
        if(c3 != NULL)
        {
            strncpy(ls, s, 4); ls[3] = '\0';

            strptr = strchr(ls, '.');
            if(strptr != NULL)
                (*strptr) = '\0';

            (*c3) = (u_int8_t)atoi(ls);
        }
        cstrptr = strchr(s, '.');
        if(cstrptr == NULL)
            return(-2);
        s = cstrptr + 1;

        /* Fourth number. */
        if(c4 != NULL)
        {
            strncpy(ls, s, 4); ls[3] = '\0';

            strptr = strchr(ls, ' ');	/* Last, look for a space. */
            if(strptr != NULL)
                (*strptr) = '\0';

            (*c4) = (u_int8_t)atoi(ls);
        }

	return(0);
}

/* ******************************************************************
 *
 *                            CyberSpace
 */
int StringGetNetCommand(const char *str)
{
	char *strptr;
        static char cmd_str[CS_DATA_MAX_LEN];
            
            
        if(str == NULL)
            return(-1);

        
        strncpy(cmd_str, str, CS_DATA_MAX_LEN);
        cmd_str[CS_DATA_MAX_LEN - 1] = '\0';

        
        /* Get command. */   
        strptr = strchr(cmd_str, ' ');
        if(strptr != NULL)
            *strptr = '\0';

        return(atoi(cmd_str));
}

/*
 *	Returns the argument from str with spaces stripped.
 *	This function never returns NULL.
 */
char *StringGetNetArgument(const char *str)
{
	char *strptr;
	static char arg[CS_DATA_MAX_LEN];

        if(str == NULL)
            return("");

        strncpy(arg, str, CS_DATA_MAX_LEN);
        arg[CS_DATA_MAX_LEN - 1] = '\0';
            
        /* Get argument. */
        strptr = strchr(arg, ' ');
        if(strptr != NULL)
        {
            strptr += 1;
	    StringStripSpaces(strptr);
            return(strptr);
        }

        return("");
}


/* **********************************************************************
 *
 *                          Time Formatting
 *
 */
#ifndef MAX_TIME_STR
# define MAX_TIME_STR	256
#endif

/*
 *	Returns a formatted time string in accordance with given format
 *	format and returns the statically allocated string.
 *
 *	This function will never return NULL.
 */
char *StringCurrentTimeFormat(const char *format)
{
	size_t len;
	time_t current;
        struct tm *tm_ptr;
        static char s[MAX_TIME_STR];


        if(format == NULL)
            return("");
        if((*format) == '\0')
            return("");

	/* Get current time. */
        time(&current);
        tm_ptr = localtime(&current);
	if(tm_ptr == NULL)
	    return("");

        /* Format time string. */
	len = strftime(
	    s,
	    MAX_TIME_STR,
	    format,
	    tm_ptr
	);
	if(len >= MAX_TIME_STR)
	    len = MAX_TIME_STR - 1;
	if(len < 0)
	    len = 0;
	/* Null terminate. */
	s[len] = '\0';


        return(s);
}

/*
 *	Returns statically allocated string containing formatted
 *	values.
 */
char *StringTimeFormat(const char *format, time_t seconds)
{
	size_t len;
	struct tm *tm_ptr;
        static char s[MAX_TIME_STR];


        if(format == NULL)
            return("");
        if((*format) == '\0')
            return("");

        tm_ptr = localtime(&seconds);
	if(tm_ptr == NULL)
	    return("");

        /* Format time string. */
        len = strftime(
            s,
            MAX_TIME_STR,
            format,
            tm_ptr
        );
        if(len >= MAX_TIME_STR)
            len = MAX_TIME_STR - 1;
        if(len < 0)
            len = 0;
        /* Null terminate. */
        s[len] = '\0';


        return(s);
}

/*
 *	Returns a statically allocated string containing a verbose
 *	statement of the delta time seconds.
 */
char *StringFormatTimePeriod(time_t seconds)
{
        static char s[MAX_TIME_STR];


	/* Reset. */
	*s = '\0';


	if(seconds < 60)
	{
	    sprintf(s, "%ld sec%s",
		seconds,
		((seconds > 1) ? "s" : "")
	    );
	}
	else if(seconds < 3600)
	{
	    seconds = seconds / 60;
	    sprintf(s, "%ld min%s",
		seconds,
		((seconds > 1) ? "s" : "")
	    );
	}
	else if(seconds < 86400)
	{
	    seconds = seconds / 3600;
	    sprintf(s, "%ld hour%s",
		seconds,
		((seconds > 1) ? "s" : "")
	    );
	}
	else
	{
	    seconds = seconds / 86400;
	    sprintf(s, "%ld day%s",
		seconds,
		((seconds > 1) ? "s" : "")
	    );
	}

	/* Null terminate just in case. */
	s[MAX_TIME_STR - 1] = '\0';

        return(s);
}




