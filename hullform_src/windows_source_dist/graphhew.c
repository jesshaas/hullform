/* Hullform component - graphhew.c
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

/**************************************************************************	*/

/*	Hewlett-Packard driver	*/

/**************************************************************************	*/

static REAL xmove,ymove;
static REAL textshift;
int yet_to_move;

void hewlcl()
{
	return;
}

/*	initialise	*/

void hewlin()
{
#ifdef HULLSTAT
	extern	char	*statline;
#endif
	extern	INT	penup;
	char	out[50];
	extern	int	pointsize;
	extern	int	xchar,ychar;
	extern	LOGFONT	prfont;
	REAL	csize = abs((REAL) prfont.lfHeight);

/*	sequence is IN;, followed select pen 2, and characters 1/64 of plot	*/
/*	field across by 1/25th of plot depth down	*/

	penup = 1;
	yet_to_move = 0;
	opengr(O_TEXT,filedirnam);
	sprintf(out,"IN;SP 2;SC 0,9770,0,7200;SR %.2f,%.2f;\n",
	    0.045*csize,0.1*csize);
	ychar = 21.6* csize;
	xchar = 7.625 * (REAL) pointsize;
	textshift = 0.75 * (REAL) ychar;
	outst(out);
#ifndef DEMO
#ifdef HULLSTAT
	if(statline == NULL) hcpy_summ();
#else
	hcpy_summ();/* show hull info on hardcopy */
#endif
#endif
}

/*	close up: nothing to do except home the pen and close graphics device	*/

void hewlen()
{
	outst("PU 0,0;\n");
	closgr();
}

/*	new line: remember for use in next "draw()" call	*/

void hewlnl()
{
	extern	INT	penup;
	penup = 1;
}

/*	Colour: colours 1, 2 and 3 are pens 1, 2 and 3 (pen 2 is default if	*/
/*	colour not selected)	*/

void hewlcr(INT ncol)
{
    char	colstr[8];
    extern int current_hull;

    if(current_hull != 0) return;	/* one colour for overlay */
    sprintf(colstr,"SP%2d;\n",ncol);
    outst(colstr);
}

/*	write a character string	*/

void hewlst(char *str)
{
	char text[26];
	extern int penup;
	if(yet_to_move || !penup) {
	    if(xmove <= 0.0) xmove = 0.0;
	    if(ymove < textshift) ymove = textshift;
	    sprintf(text,"PU %8.1f %8.1f;\n",xmove,ymove - textshift);
	    outst(text);
	    yet_to_move = 0;
	    penup = 1;
	}
	outst("LB");
	outst(str);
	outst("\003\n");
}

/*	Move to a point	*/

void hewlmv(REAL x,REAL y)
{
    extern int penup;
    trans(&x,&y);
    xmove = x;
    ymove = y;
    yet_to_move = 1;
    penup = 0;
}

/*	Draw a line	*/

void hewldr(REAL x,REAL y)
{
	extern	INT	penup;
	char	str[26];

	if(yet_to_move) {
	    if(xmove <= 0.0) xmove = 0.0;
	    if(ymove <= 0.0) ymove = 0.0;
	    sprintf(str,"PU %8.1f %8.1f;\n",xmove,ymove);
	    outst(str);
	    yet_to_move = 0;
	}
	trans(&x,&y);
	if(x <= 0.0) x = 0.0;
	if(y <= 0.0) y = 0.0;
	sprintf(str,"PU %8.1f %8.1f;\n",x,y);
	if(!penup) *(str + 1) = 'D';
	outst(str);
	penup = 0;
	xmove = x;
	ymove = y;
}

#endif
