/* Hullform component - graphtek.c
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

char	outbuf[132];
int	nword,last[4],tmode;

void tek0in(void);

/*********************************************************************	*/

/*	TEKTRONIX DRIVER						*/

/*********************************************************************	*/

/*	4662 screen clear presumes paper change			*/

void tektcl()
{
	char *paper = "Pause for paper change: press\nany key to proceed ...";
	message(paper);
}

/*	4010 screen clear uses 4010 command			*/

void tek0cl()
{
	outst("\33\14\37");
}

/*	4662 initialise:  as for 4010, plus block mode initialisation */

void tektin()
{
    extern	char	outbuf[132];
    extern	int	tmode;
    outbuf[0] = 0x33;
    outbuf[1] = 'A';
    outbuf[2] = '(';
    tek0in();
    tmode = 1;
    outst("\33AH125\33AG200\32");
    tektat(86);
#ifndef DEMO
    hcpy_summ();/* show hull info on hardcopy */
#endif
}

/*	4010 initialisation			*/

void tek0in()
{
	extern	INT	nword,tmode;
	INT	i;

	opengr(O_BINARY,filedirnam);
	for(i = 0 ; i < 4 ; i++)
			last[i] = -1;
	tmode = 0;
	nword = 3;
}

/*	close routine same for both drivers		*/

void tekten()
{
	tektse();
	closgr();
}

/*	new line routines also identical		*/

void tektnl()
{
	tektch(29,1);
}

/*	as are string transmission routines		*/

void tektst(char *str)
{
	tektch(31,1);
	while(*str) tektch(*str++,1);
}

/*	move routines also same				*/

void tektmv(REAL x,REAL y)
{
	tektch(29,1);
	trans(&x,&y);
	tektdr(x,y);
	return;

}

/*	draw routines identical				*/

void tektdr(REAL x,REAL y)
{
	extern	INT	last[4];
	INT	ix,iy,ii;
	INT	cur0,cur1,cur2,cur3;

	trans(&x,&y);

	ix = max(0,min(4095,(int) x));
	iy = max(0,min(3071,(int) y));

	cur0 = ((iy >> 7) & 0x1f) | 0x20;
	cur1 = ((iy << 2) & 0x0c) | (ix & 3) | 0x60;
	cur2 = ((iy >> 2) & 0x1f) | 0x60;
	cur3 = ((ix >> 7) & 0x1f) | 0x20;

	if(last[0] != cur0)	tektch(cur0,1);
	if(last[1] != cur1)	tektch(cur1,1);
	if(last[2] != cur2 || last[1] != cur1 || last[3] != cur3)
				tektch(cur2,1);
	if(last[3] != cur3)	tektch(cur3,1);
	ii = ((ix >> 2) & 0x1f) | 0x40;
				tektch(ii,1);

	last[0] = cur0;
	last[1] = cur1;
	last[2] = cur2;
	last[3] = cur3;
}

void tektcr(int col)
{
	return;	/* no colour */
}

/*	character buffer management	*/

void tektch(INT iarg,INT ok)
{
	extern	char	outbuf[];
	extern	int	nword;

	if(nword >= 100 && ok != 0) {
		tektse();
		nword = 3;
	}
	outbuf[nword++] = (char) iarg;
}

/*	block mode data transmission:	*/

void tektse()
{
	outstr(outbuf + 3,nword - 3);
}

/*	heading of style 1 commands: [esc] DEVICE a	*/

void tektat(INT i)
{
	tektch(27,1);
	tektch('A',0);
	tektch(i,0);
}

