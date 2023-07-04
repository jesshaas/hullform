/* Hullform component - graphwmf.c
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
 
/*	Windows metafile driver.

	Integers are explicitly declared "short", because bit placement
	is commonly crucial
*/
#include "hulldesi.h"

void movmem(void *,void *,int);

#ifdef EXT_OR_PROF

#define MAXHANDLE 90
#define winxmax 9600
#define winymax 9600

void wmfsetpen(short int  width,short int  style,short int  r,short int  g,short int  b,short int  *pen,
	short int  *brush,short int  deleteold);
void wmfpen(short int  width,short int  style,short int  r,short int  g,short int  b);
void wmfaddobject(short int *index);
void wmfdeleteobject(short int );
void writewmf(short int  data[],short int  count);
void wmflimits(short int  x,short int  y);
void wmfsetfont(short int  width,short int  height,char *name,short int  bold,short int  italic,
	short int  changerot,REAL rotval);

short int  handle[MAXHANDLE];
short int  maxobjects;
short int  maxrecord;		/* maximum record length */
extern int penup;
extern REAL xcurr,ycurr;
extern int xscr,yscr;

#define MAXBUF 2048
char *wordbuf;		/* output buffer */

short int  numbuf;		/* size of output buffer (even number of characters) */
short int  xlimleft; 		/* picture limits, to go in placeable header */
short int  ylimtop;
short int  xlimright;
short int  ylimbottom;

short int  wordsize;		/* size of file in words */

short int  penindex;		/* index of current pen in object handle table */
short int  brushindex;		/* index of current brush in object handle table */
short int  fontindex;		/* index of current font in object handle table */

short int  curwid;		/* character attributes */
short int  curhei;
short int  curbol;
short int  curita;
short int  currot;
char *curfon;

short int  curpenwid;		/* line attributes */
short int  cursty;
short int  curr,curg,curb;

/*------------------------------------------------------------------------*/

/*	Initialise the metafile in placeable format
*/

void wmfin()
{
    short int  i;
    unsigned short int  placeableheader[11] = {0xcdd7,0x9ac6,0,0,0,0,0,576,0,0,0x5a11};
    unsigned short int  header[9] = {1,9,0x300,0xffff,0xffff,2,0xffff,0xffff,0};

    unsigned short int  setwin[50] = {
		5,0,0x20b,0,0,		/* window origin	*/
		5,0,0x20c,4608,3456,	/* window extent	*/
		4,0,0x103,8,		/* mapmode anistropic	*/
		5,0,0x201,0xffff,0x00ff,/* background colour	*/
		5,0,0x209,0,0,		/* text colour		*/
		4,0,0x107,3,		/* stretchbltmode	*/
		4,0,0x104,13,		/* drawing mode		*/
		4,0,0x106,1,		/* polyfill mode	*/
		4,0,0x102,2,		/* background mode	*/
		5,0,0x20b,0,0,		/* window origin	*/
		5,0,0x20c,0,0};		/* window extent	*/
    extern int penup;
    extern int pointsize;
    extern int xchar,ychar;
    extern LOGFONT prfont;	/* logical font structure */
    short int  xc,yc;

    if(	!memavail((void *) &wordbuf,(long) MAXBUF) ||
		!memavail((void *) &curfon,128l)) {
	    message("No memory for graphics buffers");
	    return;
    }
    opengr(O_BINARY,filedirnam);

/*	Plot presumes a 260 x 195 mm surface (A4 landscape mode with margin)
*/
    placeableheader[5] = winymax;
    placeableheader[6] = winxmax;

    numbuf = 0;		/* start with an empty output buffer		*/
    wordsize = 0;

    writewmf(placeableheader,11);
			/* write the placeable Metafile header	*/
    writewmf(header,9);	/* write the Metafile header		*/
    wordsize = 9;
    maxobjects = 0;

    setwin[48] = placeableheader[5];	/* window limits */
    setwin[49] = placeableheader[6];

    writewmf(setwin,50);	/* set drawing properties	*/

    maxrecord = 5;		/* maximum record size so far	*/

/*	Unset all handle table entries and object indices in the table
*/
    for(i = 0 ; i <= MAXHANDLE ; i++) handle[i] = FALSE;
    penindex = -1;
    brushindex = -1;
    fontindex = -1;

/*	Default character attributes	*/

/*	Default pen attributes
*/
    curpenwid = 0;
    cursty = 0;
    curr = 0;
    curg = 0;
    curb = 255;

    xlimleft = 32767;
    xlimright = 0;
    ylimtop = 32767;
    ylimbottom = 0;

    penup = 1;
    xc = (short int) prfont.lfWidth;
    yc = (short int) prfont.lfHeight;
    if(yc < 0) yc = (short int) (-(3*yc+2)/4);
    if(xc < 0) xc = (short int) (-(3*xc+2)/4);
    if(yc == 0) yc = 12;
    if(xc == 0 || xc > yc) xc = (short int) ((yc*6+5)/10);
    wmfsetfont((short int) ((xc*576)/60),(short int) ((yc*576)/60),
		prfont.lfFaceName,(short int) prfont.lfWeight,
	(short int) prfont.lfItalic,FALSE,0.0);
    ychar = 8 * yc;
    xchar = (48 * xc) / 10;
}

/*------------------------------------------------------------------------*/

/*	End Metafile	*/

void wmfen()
{
    union {
		short word[2];
		long dword;
    } w;
    short int  header[70];
    static short int  trailer[3] = {3,0,0};
    short int  checksum;
    short int  i;
    extern int chan;
    char *pheader = (char *) header;

/*  Delete all leftover objects whose handles are left in the handle table
*/
    for(i = 0 ; i < maxobjects ; i++) {
		if(handle[i]) wmfdeleteobject(i);
    }

/*	Write the end-file code (zero)
*/
    writewmf(trailer,3);

/*	Flush the Metafile buffer
*/
    outstr(wordbuf,numbuf);

/*	Reread the first record
*/
    if(lseek(chan,0,SEEK_SET) != 0) perror("wmfen lseek");
    if(read(chan,pheader,140) < 140) perror("wmfen read");

    header[3] = max(0,xlimleft-1);
    header[4] = max(0,ylimtop-1);
    header[5] = (short) min(32767,(21l*(long) (xlimright+1))/20l);
    header[6] = (short) min(32767,(21l*(long) (ylimbottom+1))/20l);

/*	Calculate the XOR checksum of the first 10 words, and
	place it in the 11th
*/
    checksum = header[0];
    for(i = 1 ; i < 10 ; i++) checksum ^= header[i];
    header[10] = checksum;

/*	Add the size of the file in words, the maximum number of objects
	used and the maximum record size
*/
    w.dword = wordsize;
    header[14] = (short int) w.word[0];
    header[15] = (short int) w.word[1];

    header[16] = maxobjects;

    w.dword = maxrecord;
    header[17] = (short int) w.word[0];
    header[18] = (short int) w.word[1];

/*	Redefine the window origin and extent
*/
    header[63] = header[4];	/* 63 */
    header[64] = header[3];	/* 64 */
    header[68] = (short int) (header[6]-header[4]+1);	/* 68 */
    header[69] = (short int) (header[5]-header[3]+1);	/* 69 */

    if(lseek(chan,0,SEEK_SET) != 0) perror("wmfen final seek");
    if(write(chan,pheader,140) < 140) perror("wmfen final write");

    closgr();
    memfree(wordbuf);
    memfree(curfon);
}

/*------------------------------------------------------------------------*/

/*	Management of the object handle table:
	Find an unused handle
*/
void wmfaddobject(short int *index)
{
    short int  i;
    for(i = 0 ; i < MAXHANDLE ; i++) {
		if(!handle[i]) {
		    if(i >= maxobjects) maxobjects = (short int) (i+1);
			handle[i] = TRUE;
		    *index = i;
		    return;
		}
    }
    message("Out of object handles");
}

/*	Delete an existing handle	*/

void wmfdeleteobject(short int  index)
{
    static short int  out[] = {4,0,0x01f0,0};

    if(index < 0) {
		return;
    } else if(!handle[index]) {
		message("Object was already deleted");
    } else {
		out[3] = index;
		writewmf(out,4);
		if(maxrecord < 4) maxrecord = 4;
		handle[index] = FALSE;
    }
}

/*	new line: remember for use in next "draw()" call	*/

void wmfnl()
{
    extern int penup;
    penup = 1;
}

void wmfcl()
{
    return;
}

void wmfmv(REAL x,REAL y)
{
    extern int penup;
    penup = 1;
    wmfdr(x,y);
}

/*------------------------------------------------------------------------*/

/*	Draw a line to the current point	*/

void wmfdr(REAL x,REAL y)
{
#undef recsiz
#define recsiz	5
    short int  out[recsiz];

    trans(&x,&y);
    if(x >= 0.0 && x < 10000.0 && y >= 0.0 && y < 10000.0) {
		xscr = x;
		out[4] = (short int) xscr;
		yscr = winymax-y;
		out[3] = (short int) yscr;
		wmflimits((short int) xscr,(short int) yscr);
		out[0] = recsiz;
		out[1] = 0;
		if(penup) {
		    out[2] = 0x0214;
		    penup = FALSE;
		} else {
		    out[2] = 0x0213;
		}
		writewmf(out,recsiz);
		if(maxrecord < recsiz) maxrecord = recsiz;
    } else {
		penup = TRUE;
    }
}

/*------------------------------------------------------------------------*/

void wmflimits(short int  x,short int  y)
{
    if(x > xlimright) xlimright = x;
    if(x < xlimleft) xlimleft = x;
    if(y < ylimtop) ylimtop = y;
    if(y > ylimbottom) ylimbottom = y;
}

/*----------------------------------------------------------------------*/

/*       Metafile line attributes are width, style and colour.

	Style codes are

	SOLID		0
	DASH		1
	DOT			2
	DASHDOT		3
	DASHDOTDOT	4
	NULL		5	(not supported here)
	INSIDEFRAME	6	(not supported here)
*/

void wmfcr(int  num)
{
    extern int winstyle[],linestyle[];
    extern COLORREF scrcolour[];
    extern int current_hull;

    if(current_hull != 0) return;	/* one colour for overlay */
    wmfpen(-1,(short int) winstyle[linestyle[max(0,min(4,num-2))]],
	(short int) GetRValue(scrcolour[num]),
	(short int) GetGValue(scrcolour[num]),
	(short int) GetBValue(scrcolour[num]));
}

/*----------------------------------------------------------------------*/

void wmfpen(short int width,short int style,short int r,short int g,short int b)
{
#undef recsiz
#define recsiz	28
    short int pen,brush;
    wmfsetpen((short int) width,(short int) style,(short int) r,
	(short int) g,(short int) b,&pen,&brush,1);
}

void wmfsetpen(short int  width,short int  style,short int  r,short int  g,short int  b,short int  *pen,
	short int  *brush,short int  deleteold)
{
    short int  oldpenindex,oldbrushindex;
    short int  out[recsiz] =
		{8,0,0x02fa,0,0,0,0,0,
		4,0,0x012d,0,
		7,0,0x02fc,0,0,0,0,
		4,0,0x012d,0,
		5,0,0x0209,0,0};

/*	Save the old handle indices, and get new ones
*/
    oldpenindex = penindex;
    oldbrushindex = brushindex;

    wmfaddobject(&penindex);
    wmfaddobject(&brushindex);
    *pen = penindex;
    *brush = brushindex;

/*	The factor of 8 is required to produce the correct line width
	in Microsoft Draw
*/
    if(width >=  0) curpenwid = (short int) (width*8);
    if(style >=  0) cursty = style;
    if(r >= 0 || g >= 0 || b >= 0) {
		curr = r;
		curg = g;
		curb = b;
    }

    out[3] = cursty;
    out[4] = curpenwid;
    out[6] = (short int) (((curg << 8) & 0xff00) | curr);
    out[7] = curb;

    out[11] = penindex;

    out[16] = out[6];
    out[17] = out[7];

    out[22] = brushindex;

    out[26] = out[6];
    out[27] = out[7];

/*	Write the pen- and brush-creation commands, with the
	select-object commands for each
*/
    writewmf(out,recsiz);
    if(maxrecord < 8) maxrecord = 8;

/*	Delete the previous pen and brush objects
*/
    if(deleteold) {
		wmfdeleteobject(oldpenindex);
		wmfdeleteobject(oldbrushindex);
    }
}

/*------------------------------------------------------------------------*/

/*	Select drawing font
*/

void wmfsetfont(short int width,short int height,char *name,
short int bold,short int italic,short int changerot,REAL rotval)
{
    short int oldfontindex,l;
    short int out[4] = {4,0,0x012d,0};
    unsigned static char outb[64] = {
	0,0,0,0,0xfb,0x02,		/* CreateFont header			*/
	0x98,0xfe,0,0,0,0,0,0,	/* font height and width,escapement,orientation	*/
	0,						/* weight (400 = normal, 700 = bold)	*/
	0,						/* italic if nonzero			*/
	0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0};

    if(*name) strcpy(curfon,name);
    if(width > 0) {
		curwid = width;

/*	The factors on font width are empirically found, to ensure proper
	font aspect ratio. The reasons for the values are not clear.
*/
		if(strstr(curfon,"Arial") != NULL || strstr(curfon,"Helv") != NULL
		|| strstr(curfon,"Times") != NULL) curwid = (short int) ((curwid*64)/100);

    }
    if(height > 0) curhei = (short int) (abs(height));
    if(bold == 0) {
		curbol = 400;
    } else if(bold == 1) {
		curbol = 700;
    }
    if(italic >= 0) curita = italic;

    if(changerot) currot = (short int) (((long) (rotval*10.0)) % 3600l);

    outb[6] = (char) curhei;
    outb[7] = (char) (curhei >> 8);
    outb[8] = (char) curwid;
    outb[9] = (char) (curwid >> 8);
    outb[10] = (char) currot;
    outb[11] = (char) (currot >> 8);
    outb[12] = outb[10];
    outb[13] = outb[11];
    outb[14] = (char) curbol;
    outb[15] = (char) (curbol >> 8);
    outb[16] = (char) curita;
    strcpy(outb+24,curfon);
    l = (short int) ((strlen(curfon)+26)/2);
    outb[0] = (char) l;

/*	Write the create-font command
*/
    writewmf((short int  *) outb,l);
    if(maxrecord < l) maxrecord = l;

    oldfontindex = fontindex;
    wmfaddobject(&fontindex);

/*	Write the select-object command to select the new font
*/
    out[3] = fontindex;
    writewmf(out,4);
    if(maxrecord < 4) maxrecord = 4;

/*	Write the command to delete the previous font
*/
    wmfdeleteobject(oldfontindex);
}

/*------------------------------------------------------------------------*/

/*	Write a Metafile character string
*/
void wmfst(char *string)
{
    union {
		short int out[256];
		char outb[512];
    } w;
    char strhead[6] = {0,0,0,0,0x21,0x05};
    short int l;
    extern int xchar,ychar;

    l = (short int) (min(244,strlen(string)));

/*	Add header bytes	*/

    movmem(strhead,w.outb,6);

/*	Add string length	*/

    w.outb[6] = (char) l;
    w.outb[7] = (char) 0;

/*	Add text		*/

    movmem(string,w.outb+8,l);

/*	Add one space if needed, to make string length even	*/

    if(l & 1) w.outb[8+(l++)] = ' ';

/*	Add position		*/

    w.outb[ 8+l] = (char) yscr;	/* receives low byte only	*/
    w.outb[ 9+l] = (char) (yscr >> 8);
    w.outb[10+l] = (char) xscr;
    w.outb[11+l] = (char) (xscr >> 8);

/*	Extend picture width if necesary	*/

    wmflimits((short int) xscr,(short int) yscr);
    wmflimits((short int) xscr,(short int) (yscr+ychar));
    wmflimits((short int) (xscr+l*xchar),(short int) yscr);
    wmflimits((short int) (xscr+l*xchar),(short int) (yscr+ychar));

    xscr = xscr+xchar*l;

/*	Write the bytes		*/

    l = (short int) ((13+l)/2);
    w.outb[0] = (char) l;
    writewmf(w.out,l);
    if(maxrecord < l) maxrecord = l;
}

void writewmf(short int  data[12],short int  count)
{
    wordsize += count;
    count <<= 1;
    if(numbuf + count > MAXBUF) {
		outstr(wordbuf,numbuf);
		numbuf = 0;
    }
    movmem((char *) data,wordbuf+numbuf,count);
    numbuf += count;
}

/*------------------------------------------------------------------------*/

/*	Shading routine
*/

void wmftri(int x0,int y0,int x1,int y1,int x2,int y2,int pen,int brush)
{
#undef recsiz
#define recsiz 18
    static short int  out[recsiz] =
	{4,0,0x12d,0,4,0,0x12d,0,10,0,0x324,3,0,0,0,0,0,0};
    extern int xmaxi;

    out[3] = (short int) pen;
    out[7] = (short int) brush;
    out[12] = (short int) x0;
    out[13] = (short int) y0;
    out[14] = (short int) x1;
    out[15] = (short int) y1;
    out[16] = (short int) x2;
    out[17] = (short int) y2;
    writewmf(out,recsiz);
    if(maxrecord < 10) maxrecord = 10;
    wmflimits((short int) x0,(short int) y0);
    wmflimits((short int) x1,(short int) y1);
    wmflimits((short int) x2,(short int) y2);
}


#endif
