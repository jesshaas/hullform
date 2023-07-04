/* Hullform component - gcc_subs.c
 * Copyright (C) 2011 Peter Rye
 * Email: prye@iinet.net.au
 *
 * This code is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed WITHOUT ANY WARRANTY; without 
 * even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Library General
 * Public License for more details.
 *
 * For a copy of the GNU Library General Public License , you can write to:
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */
 
#include "hulldesi.h"

/*	GCC for Windows additional routines	*/

#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>

#ifndef MAX_PATH
#define MAX_PATH 512
#endif

char search[MAX_PATH];

char *searchpath(char *file)
{
	char *dir;
	char *end;
	int fd;
	char pathstr[MAX_PATH];
	struct _finddata_t blk;

	if((fd = _findfirst(file,&blk)) >= 0) {
		_findclose(fd);
		return file;
	}

	strcpy(pathstr,getenv("PATH"));
	dir = pathstr;
	blk.attrib = _A_NORMAL;
	do {
		end = strchr(dir,';');
		if(end != NULL) *end = 0;
		sprintf(search,"%s\\%s",dir,file);
		if((fd = _findfirst(search,&blk)) >= 0) {
			_findclose(fd);
			return search;
		}
		dir = end + 1;
	}
	while(end != NULL);
	return NULL;
}

void movmem(char *src,char *dst,int size)
{
	if(dst > src) {
		dst += size;
		src += size;
		while(size--) *--dst = *--src;
	}
	else {
		while(size--) *dst++ = *src++;
	}
}

#ifndef _FFBLK_DEF
#define _FFBLK_DEF
struct  ffblk   {
	long            ff_reserved;
	long            ff_fsize;
	unsigned long   ff_attrib;
	unsigned short  ff_ftime;
	unsigned short  ff_fdate;
	char            ff_name[256];
};
#endif  /* __FFBLK_DEF */

int find_fd = -1;

int findfirst(char *fullname,struct ffblk *blk,unsigned mode)
{
	struct _finddata_t find;

	if(find_fd >= 0) _findclose(find_fd);

	find_fd = _findfirst(fullname,&find);
	if(find_fd > 1) {

		/*	Success - map _finddata_t to ffblk	*/

		blk->ff_attrib = find.attrib;
		blk->ff_ftime = find.time_write;
		blk->ff_fsize = find.size;
		strcpy(blk->ff_name,find.name);
		return 0;
	}
	else {
		return -1;
	}
}

int findnext(struct ffblk *blk)
{
	int found;
	struct _finddata_t find;

	if(find_fd >= 0) {
		found = _findnext(find_fd,&find);
		if(found == 0) {
			blk->ff_attrib = find.attrib;
			blk->ff_ftime = find.time_write;
			blk->ff_fsize = find.size;
			strcpy(blk->ff_name,find.name);
			return 0;
		} else {
			_findclose(find_fd);
			find_fd = -1;
			return -1;
		}
	}
	else {
		return -1;
	}
}
