// global/ctype.cpp
// See also: include/xsw_ctype.h
/*
	Character type recognition

	These are `makeup functions' for operating systems that don't
	support them.


	Functions:

	bool isblank(char c)
	bool isblank(int c)
 */

#include <ctype.h>
#include "../include/xsw_ctype.h"

//#if defined(_AIX_) || (__SOLARIS__) || defined(__CYGWIN32__)
// Dan S: I found the function signature for isblank(int) in linux libs
// to be inaccurate. Somewhere they are using so many macros to provide
// portability that the actual library is unable to link, with or without
// extern "C", using the man page indicated signature. The function is so
// simple we may as well use this, complete with an overloaded char version.
// If this is not used with C++, libc will have this already, so not needed for C.
// See also: include/xsw_ctype.h, a header declaring these functions.
#if defined(__cplusplus) || defined (c_plusplus)
bool isblankChar(char c)
{
	return((c == ' ') || (c == '\t'));
}

bool isblankInt(int c)
{
	return(( c == static_cast<int>(' ') ) || ( c == static_cast<int>('\t') ));
}

#else
bool isblankChar(char c)
{
	return((c == ' ') || (c == '\t'));
}

bool isblankInt(int c)
{
	return(( c == (int)(' ') ) || ( c == (int)('\t') ));
}

#endif


/*
 *      ANSI C requires atleast one dummy function in an
 *      entirly conditional module.
 */
void ctype_dummy_func()
{
	int x;

	x = 1;

	return;
}




