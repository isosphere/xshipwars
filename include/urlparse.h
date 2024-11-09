#ifndef URLPARSE_H
#define URLPARSE_H


/*
 *      Network limits (not ANSI C standard, but BSD standard).
 */   
#ifndef HOST_NAME_MAX
# define HOST_NAME_MAX          128
#endif

#ifndef MAX_URL_LEN
# define MAX_URL_LEN            1024
#endif

extern char *StringParseProtocol(char *url);
extern char *StringParseName(char *url);
extern char *StringParsePassword(char *url);
extern char *StringParseAddress(char *url);
extern int StringParsePort(char *url);



#endif /* URLPARSE_H */
