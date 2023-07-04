/* Hullform component - graphdxf.c
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
 
/*      General AutoCAD "DXF"-format graphics driver                    */

#include "hulldesi.h"

#ifdef EXT_OR_PROF

int	dxfpoints;
REAL	xdxf[2],ydxf[2];
static char colst[5] = "8\n0\n";
void dxfend(void);

FILE *findopen(char *name);

/*      screen-clear ignored    */

void adxfcl()
{
	return;
}

/*      initialise      */

void adxfin()
{
	FILE   *hp;			/* header file descriptor */
	int    i;
	char   line[81];

	opengr(O_TEXT,dxfdirnam);

	/*	Header lines		*/

	outst("0\nSECTION\n2\nHEADER\n");

	hp = findopen("header.dxf");
	if(hp != NULL) {
		while(fgets(line,80,hp) != NULL) outst(line);
		fclose(hp);
	}

	outst("0\nENDSEC\n");

	/*	Tables						*/

	outst("0\nSECTION\n2\nTABLES\n");

	hp = findopen("tables.dxf");
	if(hp != NULL) {
		while(fgets(line,80,hp) != NULL) outst(line);
		fclose(hp);
	}

	outst("0\nTABLE\n2\nLAYER\n70\n4\n");
	for(i = 1 ; i <= 4 ; i++) {
		sprintf(line,
			"0\nLAYER\n2\n%d\n70\n0\n62\n%d\n6\nCONTINUOUS\n",i,i);
		outst(line);
	}
	outst("0\nENDTAB\n0\nENDSEC\n");
	outst("0\nSECTION\n2\nENTITIES\n");
	dxfpoints = 0;
#ifndef DEMO
	hcpy_summ();/* show hull info on hardcopy */
#endif
}

/*      close up: nothing to do except end off the file and     */
/*      close graphics device                                   */

void adxfen()
{
	dxfend();
	outst("0\nENDSEC\n0\nEOF\n");
	closgr();
}

/*      new line        */

void adxfnl()
{
	extern int chan;
	if(chan > 0) dxfend();
	dxfpoints = 0;
}

/*      Colour: colours 1, 2 and 3 are pens 1, 2 and 3 (pen 2 is default if     */
/*      colour not selected)    */

void adxfcr(INT ncol)
{
	extern int current_hull;

	if(current_hull != 0) return;	/* one colour for overlay */
	*(colst + 2) = (char) min('4','1' + ncol);   /* layer C1, C2 ...     */
}

/*      write a character string as a DXF text entity           */

void adxfst(char *s)
{
	char   str[MAX_PATH];
	extern REAL xcurr,ycurr;
	extern int  count;
	extern REAL	xmin,xmax,ymin,ymax;
	extern int	xsize,ysize,xmaxi,ymaxi;
	extern int	pointsize;
	extern int	pointsize;

	REAL csize =  0.0125*fabs(xmax - xmin) * (REAL) pointsize / 12.0;
	REAL height = 0.025*fabs(ymax - ymin) * (REAL) pointsize / 12.0;

	dxfend();

	sprintf(str,
		"  0\nTEXT\n8\n%c\n 10\n%.4f\n 20\n%.4f\n40\n%.4f\n1\n%s\n",
		'2',xcurr,ycurr-height,csize,s);
	outst(str);
	xcurr += csize * ((float) strlen(s));
}

/*      Fix transformation so that absolute values can be output        */

void fix_trans()
{
	extern REAL     xgorigin,ygorigin;
	extern REAL     xgslope,ygslope;
	extern REAL     xmin,ymin,xmax,ymax;

	if(fgt(xmax,xmin)) {
		xgorigin = fmin(xmin);
		xgslope = 1.0;
	}
	else {
		xgorigin = xmin;
		xgslope = -1.0;
	}
	if(fgt(ymax,ymin)) {
		ygorigin = fmin(ymin);
		ygslope = 1.0;
	}
	else {
		ygorigin = ymin;
		ygslope = -1.0;
	}
}

/*      Move to a point */

void adxfmv(REAL x,REAL y)
{
	extern REAL	xcurr,ycurr;
	extern int	penup;
	extern int	chan;

	if(chan > 0) dxfend();
	fix_trans();
	trans(&x,&y);
	xcurr = x;
	ycurr = y;
	xdxf[0] = x;
	ydxf[0] = y;
	dxfpoints =1;
}

/*      Draw a line     */

void adxfdr(REAL x,REAL y)
{
	extern REAL     xcurr,ycurr;
	char str[MAX_PATH];

	fix_trans();
	trans(&x,&y);

	if(dxfpoints < 2) {
		xdxf[dxfpoints] = x;
		ydxf[dxfpoints] = y;
	}
	else {
		if(dxfpoints == 2) {

			/*	More than two points - use polyline			*/

			outst("0\nPOLYLINE\n");
			outst(colst);
			outst("66\n1\n");/* vertices follow      */
			outst("0\nVERTEX\n");
			outst(colst);
			sprintf(str,"10\n%.4f\n20\n%.4f\n",xdxf[0],ydxf[0]);
			outst(str);

			outst("0\nVERTEX\n");
			outst(colst);
			sprintf(str,"10\n%.4f\n20\n%.4f\n",xdxf[1],ydxf[1]);
			outst(str);
		}
		outst("0\nVERTEX\n");
		outst(colst);
		sprintf(str,"10\n%.4f\n20\n%.4f\n",x,y);
		outst(str);
	}

	xcurr = x;
	ycurr = y;
	dxfpoints++;
}

void dxfend()
{
	char str[MAX_PATH];
	if(dxfpoints == 2) {
		outst("0\nLINE\n");
		outst(colst);
		outst("62\n7\n");
		sprintf(str,"10\n%.4f\n20\n%.4f\n",xdxf[0],ydxf[0]);
		outst(str);
		sprintf(str,"11\n%.4f\n21\n%.4f\n",xdxf[1],ydxf[1]);
		outst(str);
	}
	else if(dxfpoints > 2) {
		outst("0\nSEQEND\n");
	}
	dxfpoints = 0;
}

FILE *findopen(char *name)
{
	FILE *fp = NULL;
	char *p;
#ifdef linux
	char *searchpath(char *);
#endif

	chdir(dxfdirnam);
	p = searchpath(name);
	if(p != NULL) fp = fopen(p,"rt");
	return fp;
}

#endif
