#include <stdlib.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>

#include "../include/os.h"

#ifdef __MSW__
# include <io.h>	/* Needed by _findfirst(), _findnext(), etc. */
#else
# include <unistd.h>
# include <sys/time.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>

#ifndef USE_GETDENTS
# ifndef __MSW__
# include <dirent.h>
# endif
#endif /* not USE_GETDENTS */

#if defined(__linux__)
# include <linux/limits.h>

# ifdef USE_GETDENTS
#  include <linux/dirent.h>   
# endif
#endif

#include "../include/fio.h"
#include "../include/string.h"
#include "../include/disk.h"

#ifdef MEMWATCH
# include "memwatch.h"
#endif


int FILEHASEXTENSION(const char *filename);
int ISPATHABSOLUTE(const char *path);
int NUMDIRCONTENTS(const char *path);

int COMPARE_PARENT_PATHS(const char *path, const char *parent);

int ISPATHDIR(const char *path);
int ISLPATHDIR(const char *path);
int ISPATHEXECUTABLE(const char *path);

int rmkdir(const char *path, mode_t mode);

char *PathSubHome(char *path);
char **GetDirEntNames(const char *parent);
char *ChangeDirRel(const char *cpath, const char *npath);
void StripAbsolutePath(char *path);
void StripParentPath(char *path, const char *parent);
char *PrefixPaths(const char *parent, const char *child);
char *GetAllocLinkDest(const char *link);
char *GetParentDir(const char *path);
void SimplifyPath(char *path);

int FileCountLines(const char *filename);
int DirHasSubDirs(const char *path);

int CopyObject(
	char *target, char *source,
	int (*comferm_func)(const char *, const char *)
);


/*
 *	Checks if there is a '.' character in the file name.
 *
 *	Skips the very first '.' character if any.
 */
int FILEHASEXTENSION(const char *filename)
{
	if(filename == NULL)
	    return(0);

	filename++;	/* Skip first char. */
	if(strchr(filename, '.') == NULL)
	    return(0);
	else
	    return(1);
}

int ISPATHABSOLUTE(const char *path)
{
#ifdef __MSW__
	char *strptr;

	if(path == NULL)
	    return(0);

	/* Has drive notation? */
	strptr = strchr(path, ':');
	if(strptr == NULL)
	{
	    /* No drive notation, check if first char is a '\\'. */
#if defined(__cplusplus) || defined(c_plusplus)
	    while(ISBLANK(*path))
#else
	    while(ISBLANK((int)(*path)))
#endif
		path++;

	    return(*path == '\\');
	}
	else
	{
	    /* Has drive notation, check if first char past : is a '\\'. */
	    return((*(strptr + 1)) == '\\');
	}
#else
	if(path == NULL)
            return(0);

	// Dan S: typecast is due to const.
#if defined(__cplusplus) || defined(c_plusplus)
	while(ISBLANK(reinterpret_cast<char>(*path)))
#else
	while(ISBLANK(*path))
#endif
	    path++;

	return((*path) == DIR_DELIMINATOR);
#endif
}

/*
 *	Returns the number of entries in a dir.
 */
int NUMDIRCONTENTS(const char *path)
{
	int i, items;
	char **dname;


	if(path == NULL)
	    return(0);

	if(!ISPATHDIR(path))
	    return(0);

	dname = GetDirEntNames(path);
	if(dname == NULL)
	    return(0);


	items = 0;
	for(i = 0; dname[i] != NULL; i++)
	{
	    if(dname[i] == NULL)
		continue;

	    /* Skip this and parent notation entries. */
	    if(!strcmp(dname[i], ".") ||
               !strcmp(dname[i], "..")
	    )
	    {
		/* Free the name. */
                free(dname[i]);
		dname[i] = NULL;
		continue;
	    }


	    /* Increment count. */
	    items++;

	    /* Free the name. */
	    free(dname[i]);
	    dname[i] = NULL;
	}
	free(dname);
	dname = NULL;

	return(items);
}

/*
 *      Returns 1 if path has the parents of parent.
 *
 *      "/usr/X11/bin/xplay" "/usr/X11/bin"
 *      will return true. 
 *
 *      Spaces must be stripped from both!
 */
int COMPARE_PARENT_PATHS(const char *path, const char *parent)
{
        int len_path, len_parent;


        if((path == NULL) ||
           (parent == NULL)
        )   
            return(0);
  
        len_path = strlen(path);
        len_parent = strlen(parent);
            
        /* Cannot compare if path is not absolute. */
        if((*path) != DIR_DELIMINATOR)
            return(0);
        if((*parent) != DIR_DELIMINATOR)
            return(0);


        /* Reduce length of parent for tailing DIR_DELIMINATOR chars. */
        while(len_parent > 0)
        {   
            if(parent[len_parent - 1] == DIR_DELIMINATOR)
                len_parent--;
            else
                break;
        }


        /* definately not if parent is longer then path. */
        if(len_path < len_parent)
            return(0);

        /* Compare, but just to the length of the parent. */
        if(strncmp(path, parent, len_parent))
            return(0);
        else
            return(1);
}

int ISPATHDIR(const char *path)
{
	struct stat stat_buf;

	/* Input error checks. */
	if(path == NULL)
	    return(0);

	/* Check if exists. */
	if(stat(path, &stat_buf))
	    return(0);

        return((S_ISDIR(stat_buf.st_mode)) ? 1 : 0);
}

int ISLPATHDIR(const char *path)
{
#ifdef __MSW__
	return(ISPATHDIR(path));
#else
        struct stat stat_buf;


	if(path == NULL)
            return(0);

        /* Check if exists (do not follow links). */
        if(lstat(path, &stat_buf))
            return(0);

        return((S_ISDIR(stat_buf.st_mode)) ? 1 : 0);
#endif
}

int ISPATHEXECUTABLE(const char *path)
{
#ifdef __MSW__
	char *strptr;


	if(path == NULL)
	    return(0);

	strptr = strrchr(path, '.');
	if(strptr == NULL)
	    return(0);
	else
	    strptr++;

	/* Check known MSW extensions for being executeable. */
	if(!strcasecmp(strptr, "exe"))
	    return(1);
	else if(!strcasecmp(strptr, "bat"))
	    return(1);
	else if(!strcasecmp(strptr, "com"))
	    return(1);
	else
	    return(0);
#else
        struct stat stat_buf;


        if(path == NULL)
            return(0);

	/* Get stats. */
        if(stat(path, &stat_buf))
            return(0);


	if((S_IXUSR & stat_buf.st_mode) |
           (S_IXGRP & stat_buf.st_mode) |
           (S_IXOTH & stat_buf.st_mode)
	)
            return(1);
	else
	    return(0);
#endif
}

/*
 *	Creates the directory specified by path and any parent paths that
 *	do not exist. Returns -1 on error, or 0 on success.
 */
int rmkdir(const char *path, mode_t m)
{
	char *strptr;
	struct stat stat_buf;
	char fullpath[PATH_MAX + NAME_MAX];


	if(path == NULL)
	    return(-1);


	/* Need to get full absolute path. */
	if(ISPATHABSOLUTE(path))
        {
            strncpy(fullpath, path, PATH_MAX + NAME_MAX);
        }
	else
	{
	    char cwd[PATH_MAX];

	    /* Not absolute path, so get current directory and prefix
	     * it to the given path.
	     */
	    if(getcwd(cwd, PATH_MAX) == NULL)
		return(-1);
	    cwd[PATH_MAX - 1] = '\0';

	    strptr = PrefixPaths(cwd, path);
            strncpy(
		fullpath,
		((strptr == NULL) ? path : strptr),
                PATH_MAX + NAME_MAX
	    );
	}
	fullpath[PATH_MAX + NAME_MAX - 1] = '\0';

	/* Full path has now been coppied to our local variable fullpath.
	 * Next we will modify the fullpath variable by replacing each
	 * encountered DIR_DELIMINATOR to check if the path exists
	 * and if not, create it.
	 */


        /* Get pointer to beginning of fullpath but skip
         * first character since it's a DIR_DELIMINATOR.
         */
	strptr = (char *)(fullpath + 1);
	while(strptr != NULL)
	{
	    /* Seek to next deliminator, updating strptr. If there
	     * are no more deliminators then strptr will be NULL.
	     */
	    strptr = strchr(strptr, DIR_DELIMINATOR);
	    if(strptr != NULL)
		(*strptr) = '\0';

	    if(stat(fullpath, &stat_buf))
            {
#ifdef __MSW__
		if(mkdir(fullpath))
		    return(-1);
#else
                if(mkdir(fullpath, m))
		    return(-1);
#endif
            }

	    /* If pointer to next deliminator is valid then reset it
	     * back to DIR_DELIMINATOR and increment strptr.
	     */
            if(strptr != NULL)
            {
                (*strptr) = DIR_DELIMINATOR;
                strptr++;
            }
        }

	return(0);
}

/*
 *	Replaces any occurances of "~" in path with value of the
 *	HOME enviroment variable.
 *
 *	Returns statically allocated string.
 */
char *PathSubHome(char *path)
{
        int i, n;
        char *strptr, *strptr2;
        static char rtn_path[PATH_MAX];


        if(path == NULL)
            return(NULL);

        /* Get enviroment value of HOME. */
        strptr = getenv("HOME");
        if(strptr == NULL)
            strptr = "/";

        /* Copy input path to return path. */
        strncpy(rtn_path, path, PATH_MAX);
        rtn_path[PATH_MAX - 1] = '\0';


        /* Search for '~'. */ 
        i = 0;
        strptr2 = rtn_path;
        while(*strptr2 != '~')   
        {
            if(*strptr2 == '\0')
                return(rtn_path);

            i++;   
            strptr2++;
        }
        n = PATH_MAX - i - 1;   /* Characters to append. */

        /* Set home value at position n. */
        strncpy(strptr2, strptr, n);

        /* Copy rest. */
        i = strlen(rtn_path);
        n = PATH_MAX - i - 1;
        if(n < 1)
            return(rtn_path);

        strptr = strstr(path, "~") + 1;
        strptr2 = &rtn_path[i];

        strncpy(strptr2, strptr, n);
        rtn_path[PATH_MAX - 1] = '\0';

        return(rtn_path);
}


/*
 *	Returns a pointer to an array of pointers to strings.
 *	Last pointer points to NULL.
 *
 *	Returns NULL on error.   All contents are dynamically
 *	allocated and MUST BE FREED by calling program.
 */
#ifdef USE_GETDENTS
char **GetDirEntNames(const char *parent)
{
	int x, bytes_read, fd;

        struct dirent *dirp_origin;
        struct dirent *dirp_pos;
	char *dirp_raw;
	long dirp_memsize;
        long next_dirent;

	int total_dnames;
	char **dname;


	if(parent == NULL)
	    return(NULL);

        /* Check if parent is a directory. */
        if(!ISPATHDIR(parent))
            return(NULL);


        /* Open parent. */
        fd = open(parent, O_RDONLY);
        if(fd < 0)
            return(NULL);


	/* Begin reading dirs. */
	next_dirent = 0;
	total_dnames = 0;
	dname = NULL;
	dirp_origin = NULL;
	dirp_memsize = sizeof(struct dirent);
	while(1)
	{
	    /* Reopen file descriptor. */
	    close(fd); fd = -1;
            fd = open(parent, O_RDONLY);
	    if(fd < 0)
		break;

	    /* Increment number of directory entry names gotten. */
	    total_dnames++;

	    /* Allocate more directory entry name pointers. */
            dname = (char **)realloc(dname, total_dnames * sizeof(char *));

	    /* Allocate more dent pointers. */
	    dirp_origin = (struct dirent *)realloc(dirp_origin,
		dirp_memsize);
	    /* Reset to prevent segfault. */
	    memset(dirp_origin, 0, dirp_memsize);

	    /* Get more directory entries. */
	    bytes_read = getdents((unsigned int)fd, dirp_origin,
		dirp_memsize);
	    if(bytes_read <= 0)
		break;

	    /* Get raw FAT position pointer. */
	    dirp_raw = (char *)dirp_origin;
	    dirp_pos = (struct dirent *)(&dirp_raw[next_dirent]);
	    if(dirp_pos->d_reclen == 0)
		break;

	    /* Get next dir entry position. */
	    next_dirent += dirp_pos->d_reclen;
	    dirp_memsize += dirp_pos->d_reclen;	/* More memory needed. */

	    /* Allocate memory for new directory entry name. */
	    dname[total_dnames - 1] = (char *)calloc(
		1,
		(strlen(dirp_pos->d_name) + 1) * sizeof(char)
	    );
	    strcpy(dname[total_dnames - 1], dirp_pos->d_name);
	}

	/* Free dents pointer. */
	free(dirp_origin); dirp_origin = NULL;

	/* Close the directory. */
	if(fd > -1)
	{
	    close(fd);
	    fd = -1;
	}

	/* Last entry for directory entry names must be NULL. */
	if(total_dnames > 0)
	    dname[total_dnames - 1] = NULL;
	else
	    dname[0] = NULL;


	return(dname);
}

#else /* USE_GETDENTS */

char **GetDirEntNames(const char *parent)
{
#ifdef __MSW__
	long ffh;		/* Find file handle. */
	struct _finddata_t d;	/* Find data return structure. */
	char prev_cwd[PATH_MAX];


	/* Record previous current working dir. */
	getcwd(prev_cwd, PATH_MAX);
	prev_cwd[PATH_MAX - 1] = '\0';

	/* Change to parent dir. */
	chdir(parent);

	/* Find first file in specified directory */
	ffh = _findfirst("*", &d);
	if(ffh == -1L)
	{
	    return(NULL);
	}
	else
	{
	    /* Found first file in directory. */
	    int i = 0;
	    char **rtn_names = (char **)malloc(sizeof(char *));


	    if(rtn_names == NULL)
	    {
		_findclose(ffh);
		return(NULL);
	    }

	    rtn_names[i] = StringCopyAlloc(d.name);
	    i++;

	    /* Find additional (if any) files in directory. */
            while(_findnext(ffh, &d) == 0)
	    {
		rtn_names = (char **)realloc(rtn_names, (i + 1) * sizeof(char *));
		if(rtn_names == NULL)
		{
		    _findclose(ffh);
		    return(NULL);
		}

		rtn_names[i] = StringCopyAlloc(d.name);
		i++;
	    }

	    /* Close find file handle and its resources. */
	    _findclose(ffh);

	    /* Allocate last pointer to be NULL. */
	    rtn_names = (char **)realloc(rtn_names, (i + 1) * sizeof(char *));
	    if(rtn_names != NULL)
	    {
		rtn_names[i] = NULL;
	    }

	    return(rtn_names);
	}
#else
        int i;
        DIR *dir;
        struct dirent *de;
        char **rtn_names;


        if(parent == NULL)  
            return(NULL);

	/* Open dir. */
        dir = opendir(parent);
        if(dir == NULL)
            return(NULL);


        /* Allocate rtn_names pointers. */
        rtn_names = NULL;
 
        for(i = 0; 1; i++)
        {
            /* Reallocate rtn_names pointers. */
            rtn_names = (char **)realloc(
		rtn_names,
		(i + 1) * sizeof(char *)
	    );
            if(rtn_names == NULL)
            {
                closedir(dir);
                dir = NULL;  
                return(NULL);
            }

	    /* Get next dir entry. */
            de = (struct dirent *)readdir(dir);
            if(de == NULL)
                break;

	    rtn_names[i] = StringCopyAlloc(de->d_name);
            if(rtn_names[i] == NULL)
                break;
        }

        closedir(dir);
        dir = NULL;

        /* Set last entry to NULL. */
        rtn_names[i] = NULL;

        return(rtn_names);
#endif	/* NOT __MSW__ */
}
#endif /* USE_GETDENTS */



/*
 *	Performs a `change directory' operation on the given path
 *	statements cpath and npath.
 *
 *	cpath is treated as your current directory and npath is
 *	treated as the operation.
 *
 *	Returns NULL on error or an allocated NULL terminated string on
 *	success.  The calling function must free the returned pointer.
 */
char *ChangeDirRel(const char *cpath, const char *npath)
{
        int len;
        char *rtn_str, *strptr;


        /* If both cpath and npath are NULL, return allocated cwd. */
        if((cpath == NULL) &&
           (npath == NULL)
        )
        {           
            len = PATH_MAX;
            rtn_str = (char *)malloc((len + 1) * sizeof(char));
            if(rtn_str == NULL)
                return(NULL);

            if(getcwd(rtn_str, len) == NULL)
	    {
		free(rtn_str);
                return(NULL);
	    }
	    else
	    {
		rtn_str[len] = '\0';
		return(rtn_str);
	    }
        }
        /* If only npath is NULL, return allocated cpath. */
        if((cpath != NULL) && (npath == NULL))
        {
            len = strlen(cpath);
            rtn_str = (char *)malloc((len + 1) * sizeof(char));
            if(rtn_str == NULL)
                return(NULL);

            strncpy(rtn_str, cpath, len);
	    rtn_str[len] = '\0';

            return(rtn_str);
        }
        /* If only cpath is NULL, return allocated cwd. */
        if((npath != NULL) && (cpath == NULL))
        {
            len = PATH_MAX;
            rtn_str = (char *)malloc((len + 1) * sizeof(char));
            if(rtn_str == NULL)
                return(NULL);

            if(getcwd(rtn_str, len) == NULL)
            {
                free(rtn_str); 
                return(NULL);
            }
            else
            {
                rtn_str[len] = '\0';
                return(rtn_str);
            }
        }


        /* If cpath is not absolute, then return cwd. */
        if(!ISPATHABSOLUTE(cpath))
        {
            len = PATH_MAX;
            rtn_str = (char *)malloc((len + 1) * sizeof(char));
            if(rtn_str == NULL)
                return(NULL);

            if(getcwd(rtn_str, len) == NULL)
            {
                free(rtn_str);
                return(NULL);
            }
            else
            {
                rtn_str[len] = '\0';
                return(rtn_str);
            }
        }

        /* If npath is ".", then just return cpath. */
        if(!strcmp(npath, "."))
        {
            len = strlen(cpath);
            rtn_str = (char *)malloc((len + 1) * sizeof(char));
            if(rtn_str == NULL)
                return(NULL);

            strncpy(rtn_str, cpath, len);
	    rtn_str[len] = '\0';

            return(rtn_str);
        }

        /* If npath is an absolute directory, then change to it. */
        if(ISPATHABSOLUTE(npath))
        {
            len = strlen(npath);
            rtn_str = (char *)malloc((len + 1) * sizeof(char));
            if(rtn_str == NULL)
                return(NULL);

            strncpy(rtn_str, npath, len);
	    rtn_str[len] = '\0';

            return(rtn_str);
        }


	/* npath is a relative notation. */

	/* Prefix paths. */
	strptr = PrefixPaths(cpath, npath);
	if(strptr == NULL)
	    return(NULL);

        /* Allocate return string. */
        len = strlen(strptr);
        rtn_str = (char *)malloc((len + 1) * sizeof(char));
        if(rtn_str == NULL)
            return(NULL);

	strncpy(rtn_str, strptr, len);
	rtn_str[len] = '\0';


	/* Simplify return path, this removes any "whatever/.."s. */
	SimplifyPath(rtn_str);


	return(rtn_str);
}

/*
 *	Strips the pointed storage path of its absolute path,
 *	leaving it with just the final destination name.
 */
void StripAbsolutePath(char *path)
{
        char *strptr1, *strptr2;


        if(path == NULL)
            return;

        if((*path) != DIR_DELIMINATOR)
            return;

        strptr1 = path;

        /* Seek to end. */
        while((*strptr1) != '\0')
            strptr1++;

        /* Seek back, skipping tailing DIR_DELIMINATOR chars. */
        strptr1--;
        if(strptr1 < path)
            strptr1 = path;
        while((strptr1 > path) &&
              (*strptr1 == DIR_DELIMINATOR)
        )
            strptr1--;

        /* Seek back untill a DIR_DELIMINATOR char. */
        if(strptr1 < path)
            strptr1 = path;
        while((strptr1 > path) &&
              ((*strptr1) != DIR_DELIMINATOR)
        )
            strptr1--;

        strptr1++;      /* Go forward one char. */

        if(strptr1 < path)
            strptr1 = path;

        /* Copy child to beginning. */
        strptr2 = path;
        while((*strptr1) != '\0')
        {
            (*strptr2) = (*strptr1);

            strptr1++;
            strptr2++;
        }
        (*strptr2) = '\0';

        /* Make sure path contains atleast DIR_DELIMINATOR. */
        if((*path) == '\0')
        {
            path[0] = DIR_DELIMINATOR;
            path[1] = '\0';
        }

        return;
}

/*
 *	Checks if parent is under the specified path, if it
 *	is then it will be removed. Example:
 *
 *	path = "/var/spool/mail"
 *	parent = "/var"
 *
 *	Then path becomes "spool/mail"
 */
void StripParentPath(char *path, const char *parent)
{
	char *strptr;


	if((path == NULL) ||
           (parent == NULL)
	)
	    return;

	/* Both given paths must be absolute. */
	if(!ISPATHABSOLUTE(path) ||
           !ISPATHABSOLUTE(parent)
	)
	    return;

	if(!COMPARE_PARENT_PATHS(path, parent))
	    return;

	/* Remove parent from path. */
	substr(path, parent, "");

	/* Need to remove any prefixing DIR_DELIMINATOR characters. */
	while((*path) == DIR_DELIMINATOR)
	{
	    strptr = path;
	    while(*strptr != '\0')
		*strptr++ = *(strptr + 1);
	}

	/* If path is empty, it implies the parent and path were
	 * the same.
	 */
	if(*path == '\0')
	    strcpy(path, parent);

	return;
}

/*
 *	Returns a statically allocated string containing the
 *	prefixed paths of the parent and child.
 *
 *	If child is absolute, then the statically allocated string
 *	will contain the child path.
 *
 *	The parent path must be absolute.
 */
char *PrefixPaths(const char *parent, const char *child)
{
        char *strptr1, *strend;
	const char *strptr2;
        static char rtn_path[PATH_MAX];


        if((parent == NULL) ||
           (child == NULL) ||
           (parent == child)
        )
            return("/");

        /* If child is absolute, copy child and return. */
        if((*child) == DIR_DELIMINATOR)
        {
            strncpy(rtn_path, child, PATH_MAX);
            rtn_path[PATH_MAX - 1] = '\0';

            return(rtn_path);
        }

        /* Calculate strend. */
        strend = rtn_path + PATH_MAX;


        /* Copy parent. */
        strncpy(rtn_path, parent, PATH_MAX);
        rtn_path[PATH_MAX - 1] = '\0';

        /* Seek to end of rtn_path. */
        strptr1 = rtn_path;
        while((*strptr1) != '\0')
            strptr1++;

        /* Insert deliminating DIR_DELIMINATOR char. */
        if(strptr1 > rtn_path)
        {
            if(*(strptr1 - 1) != DIR_DELIMINATOR)
            {
                (*strptr1) = DIR_DELIMINATOR;
                strptr1++;
            }
        }
        else
        {
            strptr1 = rtn_path;
        }

        /* Copy child. */
        strptr2 = child;
        while((strptr1 < strend) &&
              (*strptr2 != '\0')
        )
        {
            *strptr1 = *strptr2;

            strptr1++;
            strptr2++;
        }
        if(strptr1 < strend)
            *strptr1 = '\0';
        else
            rtn_path[PATH_MAX - 1] = '\0';

        /* Make sure rtn_path contains atleast "/". */
        if(*rtn_path == '\0')
        {
            rtn_path[0] = DIR_DELIMINATOR;
            rtn_path[1] = '\0';
        }

        return(rtn_path);
}


/*
 *	Returns an allocated string containing the link's destination
 *	or NULL on error or if link is not really a link.
 *
 *	The pointer returned must be free()ed at by the
 *	calling function.
 */
char *GetAllocLinkDest(const char *link)
{
#ifdef __MSW__
	return(NULL);
#else
	int bytes_read;
	char *dest;
	struct stat stat_buf;


	if(link == NULL)
	    return(NULL);
	if(lstat(link, &stat_buf))
	    return(NULL);
	if(!S_ISLNK(stat_buf.st_mode))
	    return(NULL);

	/* Allocate dest buffer. */
	dest = (char *)calloc(1, (PATH_MAX + NAME_MAX + 1) * sizeof(char));
	if(dest == NULL)
	    return(NULL);

	/* Read link's destination. */
	bytes_read = readlink(link, dest, PATH_MAX + NAME_MAX);
	if(bytes_read <= 0)
	{
	    /* Empty dest buffer, return an allocated but empty string. */
	    dest[0] = '\0';
	    return(dest);
	}

	if(bytes_read > PATH_MAX + NAME_MAX)
	    bytes_read = PATH_MAX + NAME_MAX;
	if(bytes_read < 0)
	    bytes_read = 0;

	/* Must NULL terminate. */
        dest[bytes_read] = '\0';


	return(dest);
#endif	/* NOT __MSW__ */
}


/*
 *	Returns the number of lines in the file filename.
 */
int FileCountLines(const char *filename)
{
	FILE *fp;
	int i, lines = 0;


	fp = FOpen(filename, "rb");
	if(fp == NULL)
	    return(lines);


	/* Start counting lines. */
	i = fgetc(fp);
	while(i != EOF)
	{
	    if(((char)i == '\r') ||
               ((char)i == '\n')
	    )
		lines++;

	    i = fgetc(fp);
	}

	/* Close file. */
	FClose(fp);


	return(lines);
}


/*
 *	Returns 1 if path is a dir and contains sub dirs.
 *
 *	The given path must be an absolute path.
 */
int DirHasSubDirs(const char *path)
{
#ifdef __MSW__
	return(0);
#else
	char *strptr;
        DIR *dir;
        struct dirent *dent;
        char tmp_path[PATH_MAX + NAME_MAX];
	int status = 0;


	/* Error checks. */
	if(path == NULL) return(status);
	if(!ISPATHDIR(path)) return(status);

	/* Open dir. */
        dir = opendir(path);
        if(dir == NULL) return(status);

	/* Scan through dirs. */
	dent = (struct dirent *)readdir(dir);
	while(dent != NULL)
	{
	    /* Skip special dir notations. */
	    if(!strcmp(dent->d_name, ".") ||
               !strcmp(dent->d_name, "..")
            )
	    {
		dent = (struct dirent *)readdir(dir);
		continue;
	    }

	    strptr = PrefixPaths(path, dent->d_name);
	    if(strptr == NULL) continue;

	    strncpy(tmp_path, strptr, PATH_MAX + NAME_MAX);
	    tmp_path[PATH_MAX + NAME_MAX - 1] = '\0';

	    if(ISPATHDIR(tmp_path))
	    {
		status = 1;
		break;
	    }

	    dent = (struct dirent *)readdir(dir);
	}

	/* Close dir. */
	closedir(dir);

	return(status);
#endif	/* NOT __MSW__ */
}


/*
 *	Returns a statically allocated string containing the parent
 *	of the given absolute path.
 */
char *GetParentDir(const char *path)
{
        int i;
        const char *strptr1;
	char *strptr2;
        static char rtn_path[PATH_MAX];


        if(path == NULL)
            return("/");

        i = 0;
        strptr1 = path;
        strptr2 = rtn_path;
        while(((*strptr1) != '\0') && (i < PATH_MAX))
        {
            (*strptr2) = (*strptr1);

            strptr1++;
            strptr2++;
            i++;
        }
        if(i < PATH_MAX)
            (*strptr2) = '\0';
        else
            rtn_path[PATH_MAX - 1] = '\0';


        strptr2--;
        if(strptr2 < rtn_path)
            strptr2 = rtn_path;
        while((*strptr2) == DIR_DELIMINATOR)
        {
            (*strptr2) = '\0';
            strptr2--;
        }

        /* Removing the child. */
        while((strptr2 > rtn_path) &&
              ((*strptr2) != DIR_DELIMINATOR)
        )
            strptr2--;

        if(strptr2 < rtn_path)
            strptr2 = rtn_path;

        (*strptr2) = '\0';


        /* Make sure rtn_path contains atleast "/". */
        if((*rtn_path) == '\0')
        {
            rtn_path[0] = DIR_DELIMINATOR;
            rtn_path[1] = '\0';
        }

        return(rtn_path);
}

/*
 *	Simplifies path statement path by removing:
 *
 *	1. Any embedded "whatever/.." in it.
 *
 *	Path must be absolute.
 */
void SimplifyPath(char *path)
{
        char *strptr1, *strptr2, *strptr3;


        if(path == NULL)
            return;

	/* Look for a "/..". */
        strptr1 = strstr(path, "/..");
        while(strptr1 != NULL)
        {
	    /* Seek next DIR_DELIMINATOR char or end. */
            strptr2 = strptr1 + 1;
            while(((*strptr2) != '\0') &&
                  ((*strptr2) != DIR_DELIMINATOR)
            )
                strptr2++;

	    /* Seek prev DIR_DELIMINATOR char or start. */
            strptr3 = strptr1 - 1;
            while((strptr3 > path) &&
                  ((*strptr3) != DIR_DELIMINATOR)
            )
                strptr3--;

            if(strptr3 < path)
                strptr3 = path;

	    /* Copy segment. */
            while(*strptr2 != '\0')
            {
                *strptr3 = *strptr2;

                strptr2++;   
                strptr3++;
            }
            *strptr3 = '\0';

	    /* Look for another "/.." if any. */
            strptr1 = strstr(path, "/..");
        }

	/* Make sure path contains atleast a "/". */
        if(*path == '\0')
        {
            path[0] = DIR_DELIMINATOR;
            path[1] = '\0';
        }

        return;
}


/*
 *	Coppies object source to target.
 *
 *	If target exists and comferm_func is not NULL, then
 *	comferm_func will be called, given the target and
 *	source (respectivly) and return is checked.
 *
 *	Return of any true value from comferm_func will mean overwrite,
 *	and return of 0 from comferm_func will mean do not overwrite.
 */
int CopyObject(
	const char *target, const char *source,
	int (*comferm_func)(const char *, const char *)
)
{
	int i;
	FILE *tar_fp, *src_fp;
	struct stat stat_buf;


	if((target == NULL) ||
	   (source == NULL)
	)
	    return(-1);

	/* Check if target exists. */
	if(!stat(target, &stat_buf))
	{
	    if(comferm_func != NULL)
	    {
		/* Overwrite? */
		if(!comferm_func(target, source))
		    return(-3);	/* Don't! */
	    }
	}


	/* Truncate target and open source. */
	tar_fp = FOpen(target, "wb");
	if(tar_fp == NULL)
	    return(-1);

        src_fp = FOpen(source, "rb");
        if(src_fp == NULL)
            return(-1);


	i = fgetc(src_fp);
	while(i != EOF)
	{
	    if(fputc(i, tar_fp) == EOF)
		break;

	    i = fgetc(src_fp);
	}

	FClose(tar_fp);
	FClose(src_fp);



	return(0);
}




