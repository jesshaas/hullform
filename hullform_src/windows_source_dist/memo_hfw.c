/* Hullform component - memo_hfw.c
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

#ifdef EXT_OR_PROF
void save_hull(int);
#endif

void movmem(void *src,void *dst, unsigned length);

/*	Allocate memory and keep table of blocks allocated.  Generate	*/
/*	an abort if out of memory					*/

int memavail(void **loc,long size)
{
	int	result;
	HGLOBAL	hMem;
	char	*ploc;
	unsigned int hsize = sizeof(HGLOBAL);

	/*	Known bug in Win32S, GMEM_FIXED locks memory pages	*/

	hMem = GlobalAlloc(GMEM_ZEROINIT,size + hsize);
	result = (hMem != NULL);
	if(!result) {
		memory_abort("memavail");
	}
	else {
		ploc = (void *) GlobalLock(hMem);
		movmem((char *) &hMem,ploc,hsize);
		*loc = ploc + hsize;
		memset(*loc,0,size);
	}
	return(result);
}

/*	Deallocate memory and remove entry from table			*/

void memfree(void *loc)
{
	HGLOBAL hMem;
	char *ploc;
	unsigned size = sizeof(HGLOBAL);

	ploc = loc;
	if(ploc != NULL) {	/* the pointer may be NULL - ignore it */
		movmem(ploc - size,(char *) &hMem,size);
		GlobalUnlock(hMem);
		GlobalFree(hMem);
		loc = NULL;
	}
}

/*	Alter size of allocated memory block				*/

int altavail(void **loc,long size)
{
	int	result;
	HGLOBAL hMem;
	char *ploc;
	long oldsize;
	unsigned hsize = sizeof(HGLOBAL);

	if(*loc == NULL) {	/* new allocation */
		result = memavail(loc,size);
	}
	else {

		ploc = *loc;
		movmem(ploc - hsize,(char *) &hMem,hsize);	// move memory handle to hMem from addresss held below the data

		oldsize = GlobalSize(hMem);
		if(oldsize == 0) {
			memory_abort("altavail initialise");
			return 0;
		}
		oldsize -= sizeof(HGLOBAL);

		/*	Firstly, try to reallocate the block. This may fail - Win32S known bug 		*/

		hMem = GlobalReAlloc(hMem,(unsigned long) size+sizeof(HGLOBAL),
			GMEM_ZEROINIT | GMEM_MOVEABLE);
		result = (hMem != NULL);
		if(result) {

			/*	If this works, update the handle location	*/

			ploc = (void *) GlobalLock(hMem);
			if(ploc != NULL) {
				movmem((char *) &hMem,ploc,sizeof(HGLOBAL));
				*loc = ploc + sizeof(HGLOBAL);
			}
			else {
				memory_abort("altavail lock");
				result = 0;
			}

		}
		else {

			/*	If it doesn't, allocate a new block, and copy to it	*/

			result = memavail(loc,size);
			if(result) {
				movmem(ploc,*loc,oldsize);
				memfree(ploc);
			}
			else {
				memory_abort("altavail alternate strategy");
			}
		}
		if(result && (size > oldsize)) memset(((char *) *loc)+oldsize,0,size-oldsize);
	}
	return(result);
}

int realloc_hull(int size)
{
	int r1,r2,r3,r4,r5;
	long lsize = (long) (size+4) * sizeof(*yline);/* 2 stringers plus workspace */
	void *fp;
	extern int numbetwl;
#ifndef STUDENT
	long maxtran;
#endif
	extern int current_hull;
	int r6;

	fp = (void *) yline;
	r1 = altavail(&fp,lsize);
	yline = (REAL (*)[maxsec+4]) fp;

	fp = (void *) zline;
	r2 = altavail(&fp,lsize);
	zline = (REAL (*)[maxsec+4]) fp;

	fp = (void *) ycont;
	r3 = altavail(&fp,lsize);
	ycont = (REAL (*)[maxsec+4]) fp;

	fp = (void *) zcont;
	r4 = altavail(&fp,lsize);
	zcont = (REAL (*)[maxsec+4]) fp;

	fp = (void *) linewt;
	r6 = altavail(&fp,lsize);
	linewt = (REAL (*)[maxsec+4]) fp;

#ifndef STUDENT

	if(current_hull == 0) {
		maxtran = (size-1) * (numbetwl+1) + 1;
		fp = (void *) xtran;
		r5 = altavail(&fp,3*sizeof(REAL)*maxtran);
		xtran = (REAL *) fp;
		ytran = &xtran[maxtran];
		ztran = &ytran[maxtran];
	}
	else {
		r5 = TRUE;
	}

#else
	r5 = TRUE;
#endif

#ifdef EXT_OR_PROF
	save_hull(current_hull);
#endif
	return(r1 & r2 & r3 & r4 & r5 & r6);
}

void memory_abort(char *where)
{
	extern HWND hWndMain;
	char text[100];

	sprintf(text,"Memory management failure in %s - aborting",where);
	message(text);
	SendMessage(hWndMain,WM_CLOSE,0,0L);
	/* This shoud clear all memory handles before exit */
}

void stkerr()
{
	UINT local = LocalCompact(32767);	/* request more than the heap size, */
	/* to force compacting */
	if(local < 2000)
		message("Free memory less than 2000 bytes.\nSuggest you restart program.");
}



