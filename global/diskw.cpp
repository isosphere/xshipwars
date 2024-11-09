/*
						 Disk and Directory Handling

	int FILEHASEXTENSION(char *filename)
	int ISPATHABSOLUTE(char *path)
	int ISPATHTOPLEVEL(char *path)
	int NUMDIRCONTENTS(char *path)

	int ISDESCRWRITEABLE(int fd)
	int ISDESCRREADABLE(int fd)

	int COMPARE_PARENT_PATHS(char *path, char *parent)

	int ISPATHDIR(char *path)
	int ISLPATHDIR(char *path)
	int ISPATHEXECUTABLE(char *path)

	char *PathSubHome(char *path)
	char **GetDirEntNames(char *parent)
	char *ChangeDirRel(char *cpath, char *npath)
	void StripAbsolutePath(char *path)
	char *PrefixPaths(char *parent, char *child)
	char *GetAllocLinkDest(char *link)
	char *GetParentDir(char *path)
	void SimplifyPath(char *path)

	int FileCountLines(char *filename)
	int DirHasSubDirs(char *path)

	int CopyObject(char *target, char *source, int (*comferm_func)(char *, char *))



 */

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <errno.h>
extern "C" int errno;
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#ifndef USE_GETDENTS
	#include <dirent.h>
#endif /* not USE_GETDENTS */

#if defined(__linux__)
	#include <linux/limits.h>

	#ifdef USE_GETDENTS
		#include <linux/dirent.h>   
	#endif
#endif

#include "../include/xsw_ctype.h"
#include "../include/os.h"
#include "../include/string.h"
#include "../include/disk.h"


int FILEHASEXTENSION(char *filename)
{
	if ( filename == NULL )	return(0);
	if ( strchr(filename + 1, '.') == NULL ) return(0);
	return(1);
}

int ISPATHABSOLUTE(char *path)
{
	if ( path == NULL )	return(0);
	while(ISBLANK(*path)) path++; /* goto start of string */
	return(*path == PATH_SEP_CHAR || *(path + 1) == ':');
}



int ISPATHTOPLEVEL(char *path)
{

	register int x;

	if ( path == NULL )	return(0);

	while(ISBLANK(*path)) path++; /* goto start of string */
	if (*(path+1) == ':') path+= 3; /* skip drive if present */
	else
		if ( *path++ != PATH_SEP_CHAR ) 
			return(0);
	switch ( *path ) {
		case ' '  :
		case '\t' :
		case '\0' :
		case '\n' :
		case '\r' :
			return(1);
	}
	return(0);
}



/*
 *	Returns the number of entries in a dir.
 */
// Dan S: Disabling, const char * arg should be the correct version,
// this one is a duplicate.
#if false
int NUMDIRCONTENTS(char *path)
{
	int i, items;
	char **dname;


	if ( path == NULL )	return(0);
	if ( !ISPATHDIR(path) )	return(0);

	dname = GetDirEntNames(path);
	if ( dname == NULL ) return(0);


	items = 0;
	for ( i = 0; dname[i] != NULL; i++ ) {
		if ( dname[i] == NULL )	continue;
		/* Skip this and parent notation entries. */
		if ( !strcmp(dname[i], ".") || !strcmp(dname[i], "..") ) {
			free(dname[i]); dname[i] = NULL; 	/* Free the name. */
			continue;
		}

		items++; 		/* Increment count. */
        free(dname[i]); dname[i] = NULL; /* Free the name. */
	}
	free(dname); dname = NULL;
	return(items);
}
#endif	// if false


int ISDESCRWRITEABLE(int fd)
{
	static int status;
	static struct timeval tv;
	static fd_set writefds;


	if ( fd < 0 ) return(0);

	errno = 0;
	tv.tv_sec = 0;
	tv.tv_usec = 0;
	FD_ZERO(&writefds);
	FD_SET(fd, &writefds);
	status = select(fd + 1, NULL, &writefds, NULL, &tv);
	return(FD_ISSET(fd, &writefds));
}


int ISDESCRREADABLE(int fd)
{
	static int status;
	static struct timeval tv;
	static fd_set readfds;


	if ( fd < 0 ) return(0);
    errno = 0;
	tv.tv_sec = 0;
	tv.tv_usec = 0;
	FD_ZERO(&readfds); 
	FD_SET(fd, &readfds);
	status = select(fd + 1, &readfds, NULL, NULL, &tv);

	return(FD_ISSET(fd, &readfds)); 
}



/*
 *      Returns 1 if path has the parents of parent.
 *
 *      "/usr/X11/bin/xplay" "/usr/X11/bin"
 *      will return true. 
 *
 *      Spaces must be stripped from both!
 */
int COMPARE_PARENT_PATHS(char *path, char *parent)
{
	int len_path, len_parent;

	if ((path == NULL) || (parent == NULL)) return(0);

	len_path = strlen(path);
	len_parent = strlen(parent);

	/* Cannot compare if path is not absolute. */
	if (!ISPATHABSOLUTE(path) || !ISPATHABSOLUTE(parent)) return(0);
    
	/* Reduce length of parent for tailing /'s. */
	while (len_parent > 0) 
	{
		if (parent[len_parent - 1] == PATH_SEP_CHAR)
			len_parent--;
		else
			break;
	}

    /* definately not if parent is longer then path. */
	if (len_path < len_parent) return(0);
    
	/* Compare, but just to the length of the parent. */
	if (strncmp(path, parent, len_parent)) return(0);
    return(1);
}



int ISPATHDIR(char *path)
{
	struct stat stat_buf;

    if (path == NULL) return(0); /* Input error checks. */
    if ( stat(path, &stat_buf) ) return(0); /* Check if exists. */
	return((S_ISDIR(stat_buf.st_mode)) ? 1 : 0);
}



int ISLPATHDIR(char *path)
{
	struct stat stat_buf;

    if (path == NULL) return(0); /* Error checks. */
    if (lstat(path, &stat_buf)) return(0); /* Check if exists. */
    return((S_ISDIR(stat_buf.st_mode)) ? 1 : 0);
}



int ISPATHEXECUTABLE(char *path)
{
	struct stat stat_buf;

	if (path == NULL) return(0);
    if (stat(path, &stat_buf)) return(0); /* Get stats. */
    if ((S_IXUSR & stat_buf.st_mode) | (S_IXGRP & stat_buf.st_mode) | (S_IXOTH & stat_buf.st_mode))
		return(1);
    return(0);
}


/*
 *	Replaces any occurances of "~" in path with user's home
 *	dir.
 *
 *	Returns statically allocated string.
 */
char *PathSubHome(char *path)
{
	int i, n, p, len;
	char home_dir[PATH_MAX];
	char *strptr;
	static char rtn_path[PATH_MAX];


	/* Error checks. */
	if (path == NULL)	return(NULL);

	/* Get home dir. */
	strptr = getenv("HOME");
	if ( strptr == NULL ) return(NULL);

	strncpy(home_dir, strptr, PATH_MAX);
	home_dir[PATH_MAX - 1] = '\0';
	len = strlen(home_dir);

	/* Seek ~. */
	for ( p = 0, n = 1; path[p] != '\0'; p++ ) {
		if ( path[p] == '~' ) {
			n = 0;
			break;
		}
	}

	/* No match? */
	if ( n ) {
		strncpy(rtn_path, path, PATH_MAX);
		rtn_path[PATH_MAX - 1] = '\0';
		return(rtn_path);
	}

	/* Copy over first few chars. */
	for ( i = 0; (i < p) && (i < PATH_MAX); i++ ) {
		rtn_path[i] = path[i];
	}

	/* Sub. */
	for ( i = p, n = 0; (i < PATH_MAX) && (n < len); n++, i++ ) {
		rtn_path[i] = home_dir[n];
	}

	/* Ending chars. */
	for ( i = p + len, n = p + 1; (path[n] != '\0') && (i < PATH_MAX); i++, n++) {
		rtn_path[i] = path[n];
	}
	if ( i < PATH_MAX )
		rtn_path[i] = '\0';
	else
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
char **GetDirEntNames(char *parent)
{
	int x, bytes_read, fd;

	struct dirent *dirp_origin;
	struct dirent *dirp_pos;
	char *dirp_raw;
	long dirp_memsize;
	long next_dirent;

	long total_dnames;
	char **dname;


    if ( parent == NULL ) return(NULL); /* Error checks. */

	/* Must be absolute parent. */
	x = 0;
	while ((parent[x] == ' ') || (parent[x] == '\t')) x++;
	if ( parent[x] != PATH_SEP_CHAR ) return(NULL);

	/* Check if parent is a directory. */
	if ( !ISPATHDIR(parent) ) return(NULL);


	/* ********************************************************* */

	/* Open parent. */
	fd = open(parent, O_RDONLY);
	if ( fd < 0 ) return(NULL);


	/* Begin reading dirs. */
	next_dirent = 0;
	total_dnames = 0;
	dname = NULL;
	dirp_origin = NULL;
	dirp_memsize = sizeof(struct dirent);
	while ( 1 ) {
		/* Reopen file descriptor. */
		close(fd); fd = -1;
		fd = open(parent, O_RDONLY);
		if ( fd < 0 ) break;

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
		if ( bytes_read <= 0 ) break;

		/* Get raw FAT position pointer. */
		dirp_raw = (char *)dirp_origin;
		dirp_pos = (struct dirent *)(&dirp_raw[next_dirent]);
		if ( dirp_pos->d_reclen == 0 ) break;

		/* Get next dir entry position. */
		next_dirent += dirp_pos->d_reclen;
		dirp_memsize += dirp_pos->d_reclen;	/* More memory needed. */

		/* Allocate memory for new directory entry name. */
		dname[total_dnames - 1] = (char *)calloc(1,(strlen(dirp_pos->d_name) + 1) * sizeof(char));
		strcpy(dname[total_dnames - 1], dirp_pos->d_name);
	}

	/* Free dents pointer. */
	free(dirp_origin); dirp_origin = NULL;

	/* Close the directory. */
	if ( fd > -1 ) {
		close(fd);
		fd = -1;
	}

	/* Last entry for directory entry names must be NULL. */
	if ( total_dnames > 0 )
		dname[total_dnames - 1] = NULL;
	else
		dname[0] = NULL;


	return(dname);
}

#else /* USE_GETDENTS */

char **GetDirEntNames(char *parent)
{
	int i, len;
	DIR *dir;
	struct dirent *de;
	char **rtn_names;


	/* Error checks. */
	if ( parent == NULL ) return(NULL);


	/* Open dir. */
	dir = opendir(parent);
	if ( dir == NULL ) return(NULL);


	/* Allocate rtn_names pointers. */
	rtn_names = NULL;

	for ( i = 0; 1; i++ ) {
		/* Reallocate rtn_names pointers. */
		rtn_names = (char **)realloc(rtn_names, (i + 1) * sizeof(char *));
		if ( rtn_names == NULL ) {
			closedir(dir);
			dir = NULL;  
			return(NULL);
		}

		/* Get next dir entry. */
		de = (struct dirent *)readdir(dir);
		if ( de == NULL ) break;

		len = strlen(de->d_name);

		rtn_names[i] = (char *)calloc(1, (len + 1) * sizeof(char));
		if ( rtn_names[i] == NULL ) break;
        strcpy(rtn_names[i], de->d_name);
	}

	closedir(dir);
	dir = NULL;

	/* Set last entry to NULL. */
	rtn_names[i] = NULL;

	return(rtn_names);
}
#endif /* USE_GETDENTS */



/*
 *	Returns the changed path from cpath to npath using standard
 *	change dir rules.
 *
 *	Returns NULL on error, or an allocated NULL terminated string on
 *	success.  The calling program MUST FREE THE RETURNED STRING.
 */
char *ChangeDirRel(char *cpath, char *npath)
{
	int len;
	char *rtn_str;
	int is_cpath_toplevel;


	/* If both cpath and npath are NULL, return allocated cwd. */
	if ((cpath == NULL) && (npath == NULL)) {
		len = PATH_MAX;
		rtn_str = (char *)calloc(1, (len + 1) * sizeof(char));
		if ( rtn_str == NULL ) return(NULL);

		if (getcwd(rtn_str, len) == NULL ) {
			free(rtn_str); rtn_str = NULL;
			return(NULL);
		}

		return(rtn_str);
	}

	/* If npath is NULL, return allocated cpath. */
	if ( (cpath != NULL) && (npath == NULL) ) {
		len = strlen(cpath);
		rtn_str = (char *)calloc(1, (len + 1) * sizeof(char));
		if ( rtn_str == NULL ) return(NULL);
        strcpy(rtn_str, cpath);
		return(rtn_str);
	}

	/* If cpath is NULL, return allocated cwd. */
	if ( cpath == NULL ) {
		len = PATH_MAX;
		rtn_str = (char *)calloc(1, (len + 1) * sizeof(char));
		if ( rtn_str == NULL ) return(NULL);

		if ( getcwd(rtn_str, len) == NULL ) {
			free(rtn_str); rtn_str = NULL;
			return(NULL);
		}

		return(rtn_str);
	}


	/* ******************************************************************* */

	/* Strip spaces on cpath and npath since both are valid. */
	StringStripSpaces(cpath);
	StringStripSpaces(npath);


	/* If cpath is not an absolute path, then return current working dir. */
	if (!ISPATHABSOLUTE(cpath)) {
		len = PATH_MAX;  
		rtn_str = (char *)calloc(1, (len + 1) * sizeof(char));
		if ( rtn_str == NULL ) return(NULL);

		if ( getcwd(rtn_str, len) == NULL ) {
			free(rtn_str); rtn_str = NULL;
			return(NULL);
		}

		return(rtn_str);   
	}

	/* If npath is ".", then return cpath. */
	if (!strcmp(npath, ".")) {
		len = strlen(cpath);
		rtn_str = (char *)calloc(1, (len + 1) * sizeof(char));
		if ( rtn_str == NULL ) return(NULL);
        strcpy(rtn_str, cpath);
		return(rtn_str);
	}


	/* If npath is an absolute directory, then change accordingly. */
	if ( !ISPATHABSOLUTE(npath)) {
		/* Make sure npath is a directory. */
		if ( ISPATHDIR(npath) ) {
			len = strlen(npath);
			rtn_str = (char *)calloc(1, (len + 1) * sizeof(char));
			if ( rtn_str == NULL ) return(NULL);
			strcpy(rtn_str, npath);
			return(rtn_str);
		} else {
			/* npath is not a directory, return cwd instead. */
			len = PATH_MAX;
			rtn_str = (char *)calloc(1, (len + 1) * sizeof(char));
			if ( rtn_str == NULL ) return(NULL);
			if ( getcwd(rtn_str, len) == NULL ) {
				free(rtn_str); rtn_str = NULL;
				return(NULL);
			}

			return(rtn_str);
		}
	}


	/* ******************************************************** */
	/* Change npath as a relative. */

	/* Set check to determine if cpath is toplevel. */
	is_cpath_toplevel = ISPATHTOPLEVEL(cpath);
    rtn_str = NULL;

	/* Change to parent? */
	if (!(strncmp(npath, "..", 2))) {
		/* Return cpath if cpath is toplevel (toplevel has no parent). */
		if ( is_cpath_toplevel ) {
			len = strlen(cpath);
			rtn_str = (char *)calloc(1, (len + 1) * sizeof(char));
			if ( rtn_str == NULL ) return(NULL);
			strcpy(rtn_str, cpath);
			return(rtn_str);   
		}
		/* Change to parent. */
		len = strlen(cpath);
		rtn_str = (char *)calloc(1, (len + 1) * sizeof(char));
		if ( rtn_str == NULL ) return(NULL);

		strcpy(rtn_str, cpath);

		len = strlen(rtn_str) - 1;

		/* Strip "/" at end. */
		while ( len > 0 ) {
			if ( rtn_str[len] != PATH_SEP_CHAR) break;
            rtn_str[len] = '\0';
			len--; 
		}   

		while ( len > 0 ) {
			if (rtn_str[len] == PATH_SEP_CHAR) {
				rtn_str[len] = '\0';
				break;
			}
            rtn_str[len] = '\0';
			len--;
		}
	}
	/* Change to child. */
	else {
		rtn_str = (char *)calloc(1,(strlen(cpath) + strlen(npath) + 2) * sizeof(char));
		if ( rtn_str == NULL ) return(NULL);

		if ( is_cpath_toplevel ) {
			sprintf(rtn_str, PATH_SEP_STR"%s", npath);
		} else {
			len = strlen(cpath) - 1;
			if ( len < 1 ) len = 1;
			if ( cpath[len] == PATH_SEP_CHAR ) {
				sprintf(rtn_str, "%s%s", cpath, npath);
			} else {
				sprintf(rtn_str, "%s"PATH_SEP_STR"%s", cpath, npath);
			}

			/* Strip tailing "/" from rtn_str. */
			len = strlen(rtn_str) - 1;
			while ( len > 0 ) {
				if ( rtn_str[len] != PATH_SEP_CHAR ) break;
                rtn_str[len] = '\0';
				len--;
			}
		}
	}

	return(rtn_str);
}



/*
 *	Strips the pointed storage path of its absolute path,
 *	leaving it with just the final destination name.
 */
void StripAbsolutePath(char *path)
{
	register int x, y;
	register int len;

	/* Error checks. */
	if ( path == NULL )
		return;
	/* If path has no '/' character, then do nothing. */
	if ( strchr(path, '/') == NULL )
		return;


	/* ************************************************************** */

	/* Automatically assume path entry is NOT a dir. */

	/* Get length. */
	len = strlen(path);

	/* Strip tailing /'s. */
	x = len - 1;
	while ( x >= 0 ) {
		if (path[x] == PATH_SEP_CHAR )
			path[x] = '\0';
		else
			break;

		x--;
	}

	/* Get new length. */
	len = strlen(path);

	/* Position x to last /. */
	x = len - 1;
	while ( x >= 0 ) {
		if ( path[x] == PATH_SEP_CHAR )
			break;

		x--;
	}
	if ( x < 0 )
		x = 0;
	x++;

	y = 0;
	while ( x < len ) {
		path[y] = path[x];
		x++; y++;
	}

	path[y] = '\0';


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
// Dan S: temporary disable, disk.cpp has a duplicate of this.
// Likely this will be removed completely in the near future.
#if false
char *PrefixPaths(char *parent, char *child)
{
	int i;
	int parent_len, child_len;
	char lparent[PATH_MAX + NAME_MAX];
	char lchild[PATH_MAX + NAME_MAX];
	static char new_path[PATH_MAX + NAME_MAX];  


	/* Check if givin is previous output. */
	if ( (parent == new_path) || (child == new_path)) {
		fprintf(stderr, "PrefixPaths(): Warning: Input pointer same as previous return pointer.\n");
	}


	/* Reset new path to "". */
	*new_path = '\0';  


	/* If child is NULL, return parent. */
	if ( child == NULL ) {
		strncpy(new_path, ((parent == NULL) ? PATH_SEP_STR : parent), PATH_MAX + NAME_MAX);
		new_path[PATH_MAX + NAME_MAX - 1] = '\0';
        return(new_path);
	}

	/* Record local child path. */
	strncpy(lchild, child, PATH_MAX + NAME_MAX);
	lchild[PATH_MAX + NAME_MAX - 1] = '\0';


	/* If child is absolute then return child. */
	if ( ISPATHABSOLUTE(lchild) ) {
		strncpy(new_path, lchild, PATH_MAX + NAME_MAX);
		new_path[PATH_MAX + NAME_MAX - 1] = '\0';
		return(new_path);
	}

	/* If parent is NULL, then consider it cwd ".\". */
	if ( parent == NULL ) {
		strcpy(new_path, CWD_STR);
		strncat(new_path, lchild, PATH_MAX + NAME_MAX - 1 - 2);
		new_path[PATH_MAX + NAME_MAX - 1] = '\0';
        return(new_path);
	}

	strncpy(lparent, parent, PATH_MAX + NAME_MAX);
	lparent[PATH_MAX + NAME_MAX - 1] = '\0';

	/* Strip spaces. */
	StringStripSpaces(lparent);
	StringStripSpaces(lchild);

	parent_len = strlen(lparent);
	child_len = strlen(lchild);

	/* Make sure lengths are within reason. */
	if ( ((int)parent_len + (int)child_len) > (PATH_MAX + NAME_MAX) ) {
		strncpy(new_path, lparent, PATH_MAX + NAME_MAX);
		new_path[PATH_MAX + NAME_MAX - 1] = '\0';
		return(new_path);
	}

	/* Strip tailing '/' characters from lparent. */
	for ( i = (int)parent_len - 1; i > 0; i-- ) {
		if ( lparent[i] == PATH_SEP_CHAR )
			lparent[i] = '\0';
		else
			break;
	}


	/* Prefix lchild to lparent. */
	if ( parent_len > 1 ) {
		sprintf(new_path, "%s"PATH_SEP_STR"%s", lparent, lchild);
	} else {
		sprintf(new_path, CWD_STR"%s", lchild);
	}

	return(new_path);
}
#endif	// #if false


/*
 *	Returns an allocated string containing the link's destination
 *	or NULL on error or if link is not really a link.
 *
 *	The pointer returned must be free()'ed at some point by the
 *	calling functions.
 */
char *GetAllocLinkDest(char *link)
{
	int bytes_read;
	char *dest;
	struct stat stat_buf;


	if ( link == NULL )	return(NULL);
	if ( lstat(link, &stat_buf) ) return(NULL);
	if ( !S_ISLNK(stat_buf.st_mode) ) return(NULL);

	/* Allocate dest buffer. */
	dest = (char *)calloc(1, (PATH_MAX + NAME_MAX + 1) * sizeof(char));
	if ( dest == NULL )	return(NULL);

	/* Read link's destination. */
	bytes_read = readlink(link, dest, PATH_MAX + NAME_MAX);
	if ( bytes_read <= 0 ) {
		/* Empty dest buffer, return an allocated but empty string. */
		dest[0] = '\0';
		return(dest);
	}

	if ( bytes_read > PATH_MAX + NAME_MAX )
		bytes_read = PATH_MAX + NAME_MAX;
	if ( bytes_read < 0 )  bytes_read = 0;
    /* MUST NULL TERMINATE! */
	dest[bytes_read] = '\0';
    return(dest);
}



/*
 *	Returns the number of lines in the file filename.
 */
int FileCountLines(char *filename)
{
	FILE *fp;
	int i;
	int lines = 0;


	fp = fopen(filename, "r");
	if ( fp == NULL ) return(lines);


	/* Start counting lines. */
	i = fgetc(fp);
	while ( i != EOF ) {
		if ( ((char)i == '\r') || ((char)i == '\n'))
			lines++;

		i = fgetc(fp);
	}

	/* Close file. */
	fclose(fp);
	return(lines);
}


/*
 *	Returns 1 if path is a dir and contains sub dirs.
 *
 *	The given path must be an absolute path.
 */
int DirHasSubDirs(char *path)
{
	char *strptr;
	DIR *dir;
	struct dirent *dent;
	char tmp_path[PATH_MAX + NAME_MAX];
	int status = 0;


	/* Error checks. */
	if ( path == NULL )	return(status);
	if ( !ISPATHDIR(path) )	return(status);

	/* Open dir. */
	dir = opendir(path);
	if ( dir == NULL ) return(status);

	/* Scan through dirs. */
	dent = (struct dirent *)readdir(dir);
	while ( dent != NULL ) {
		/* Skip special dir notations. */
		if ( !strcmp(dent->d_name, ".") || !strcmp(dent->d_name, "..")) {
			dent = (struct dirent *)readdir(dir);
			continue;
		}

		strptr = PrefixPaths(path, dent->d_name);
		if ( strptr == NULL ) continue;

		strncpy(tmp_path, strptr, PATH_MAX + NAME_MAX);
		tmp_path[PATH_MAX + NAME_MAX - 1] = '\0';

		if ( ISPATHDIR(tmp_path) ) {
			status = 1;
			break;
		}

		dent = (struct dirent *)readdir(dir);
	}

	/* Close dir. */
	closedir(dir);
	return(status);
}


/*
 *	Returns a statically allocated string containing the parent
 *	of the given absolute path.
 */
char *GetParentDir(char *path)
{
	int i, len;
	char *strptr;
	static char rtnstr[PATH_MAX];


	if ( path == NULL ) {
		/* Must return a valid path, so toplevel will have to do. */
		return(PATH_SEP_STR);
	}

	/* path must be absolute. */
	if ( !ISPATHABSOLUTE(path) ) {
		/* Must return a valid path, so toplevel will have to do. */
		return(PATH_SEP_STR);
	}


	strncpy(rtnstr, path, PATH_MAX);
	rtnstr[PATH_MAX - 1] = '\0';   
	StringStripSpaces(rtnstr);

	len = strlen(rtnstr);
	i = len - 1;
	strptr = &(rtnstr[i]);

	/* Skip tailing '/' characters. */
	while ( i >= 0 ) {
		if ( *strptr == PATH_SEP_CHAR ) {
			i--;
			strptr--;
		} else {
			break;
		}
	}

	while ( i >= 0 ) {
		if ( *strptr == PATH_SEP_CHAR) {
			*strptr = '\0';
			break;
		}
		i--;
		strptr--;
	}

	if ( rtnstr[0] == '\0' ) return(PATH_SEP_STR);
    return(rtnstr);
}



/*
 *	Simplifies a path statement.
 */
void SimplifyPath(char *path)
{
	int bak_pos, tar_pos;
	int bak_len;
	char *bak_str;


	if (path == NULL) return;
    if ( !ISPATHABSOLUTE(path) ) return; /* Path must be absolute. */

	/* Copy givin path to bak_str. */
	bak_len = strlen(path);
	bak_str = (char *)calloc(1, (bak_len + 1) * sizeof(char));
	if ( bak_str == NULL ) return;
	strcpy(bak_str, path);

	bak_pos = 0;
	tar_pos = 0;

	/* Seek "/.." */
	while ( bak_pos < bak_len ) {
		while ( 1 ) {
			if ( !strncmp(&path[tar_pos], PATH_SEP_STR"..", 3) ) {
				/* Found "/.." in path, rewind tar_pos. */
				tar_pos--;
				if ( tar_pos < 0 )
					tar_pos = 0;

				while ( path[tar_pos] != PATH_SEP_CHAR ) {
					if ( tar_pos < 1 ) {
						tar_pos = 0;
						break;
					}
					tar_pos--;
				}

				/* Move bak_pos forward 3 chars. */
				bak_pos += 3;

				strncpy(&path[tar_pos], &bak_str[bak_pos], bak_len);

			} else {
				break;
			}
		}

		bak_pos++;
		tar_pos++;
	}

	free(bak_str); bak_str = NULL;
    if ( strlen(path) < 1 ) strcpy(path,PATH_SEP_STR);
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
int CopyObject(char *target, char *source, int (*comferm_func)(char *, char *))
{
	int i;
	FILE *tar_fp, *src_fp;
	struct stat stat_buf;


	if ((target == NULL) || (source == NULL)) return(-1);

	/* Check if target exists. */
	if ( !stat(target, &stat_buf) ) {
		if ( comferm_func != NULL ) {
			/* Overwrite? */
			if ( !comferm_func(target, source) )
				return(-3);	/* Don't! */
		}
	}


	/* Truncate target and open source. */
	tar_fp = fopen(target, "wb");
	if ( tar_fp == NULL )
		return(-1);

	src_fp = fopen(source, "rb");
	if ( src_fp == NULL )
		return(-1);


	i = fgetc(src_fp);
	while ( i != EOF ) {
		if ( fputc(i, tar_fp) == EOF )
			break;

		i = fgetc(src_fp);
	}

	fclose(tar_fp);
	fclose(src_fp);
	return(0);
}
