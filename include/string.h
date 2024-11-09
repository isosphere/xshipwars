/*
            String Examination and Manipulation Functions
 */

#ifndef STRING_H
#define STRING_H	/* NOT to be confused with standard _STRING_H */

#include <sys/types.h>
#include "../include/os.h"

#ifndef ISBLANK
# define ISBLANK(c)	(((c) == ' ') || ((c) == '\t'))
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* Basic string handling. */
extern int strlinelen(const char *s);
extern int strlongestline(const char *s);
extern int strlines(const char *s);
#ifdef __MSW__
extern int strcasecmp(const char *s1, const char *s2);
#endif
extern const char *strseekblank(const char *s);
//extern char *strcasestr(const char *haystack, const char *needle);
extern int strpfx(const char *str, const char *pfx);
extern int strcasepfx(const char *str, const char *pfx);
extern void strtoupper(char *s);
extern void strtolower(char *s);
extern char *strcatalloc(char *orig, const char *new_str);
extern void substr(char *s, const char *token, const char *val);
#ifndef __MSW__
extern void strset(char *s, char c, int n);
#endif
extern void strpad(char *s, int n);
extern void straddflag(char *s, const char *flag, char operation, int len);

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" char *StringCopyAlloc(const char *str);
#else
extern char *StringCopyAlloc(const char *str);
#endif

extern void *MemoryCopyAlloc(const void *ptr, int nbytes);
extern char **StringCopyArray(const char **strv, int strc);
extern void StringStripSpaces(char *s);
extern char **StringQSort(char **strings, int nitems);
extern char *StringTailSpaces(char *string, int len);
extern void StringShortenFL(char *string, int limit);
extern void StringFreeArray(char **strv, int strc);

/* UNIX configruation file format string handling. */
#if defined(__cplusplus) || defined(c_plusplus)
extern "C" int StringIsYes(const char *string);
extern "C" int StringIsComment(const char *s, char c);
extern "C" char *StringCfgParseParm(const char *string);
extern "C" char *StringCfgParseValue(const char *string);
#else
extern int StringIsYes(const char *string);
extern int StringIsComment(const char *s, char c);
extern char *StringCfgParseParm(const char *string);
extern char *StringCfgParseValue(const char *string);
#endif

/* Misc string parsing. */
extern int StringParseStdColor(
	const char *s,
	u_int8_t *r_rtn,
	u_int8_t *g_rtn,
	u_int8_t *b_rtn
);
extern int StringParseIP(
        const char *s,
        u_int8_t *c1,
        u_int8_t *c2,
        u_int8_t *c3,
        u_int8_t *c4
);

/* ShipWars CyberSpace network protocol string functions. */
extern int StringGetNetCommand(const char *str);
extern char *StringGetNetArgument(const char *str);

/* Time string formatting. */
extern char *StringCurrentTimeFormat(const char *format);
extern char *StringTimeFormat(const char *format, time_t seconds);
extern char *StringFormatTimePeriod(time_t seconds);


#ifdef __cplusplus
}
#endif

#endif /* STRING_H */
