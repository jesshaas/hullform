/* Hullform component - graphreg.c
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

/********************************************************/
/*							*/
/*		ReGIS graphics driver			*/
/*							*/
/********************************************************/

void regicl()
{
	outst("\33[2J\33[H");
}

void regiin()
{
	extern	INT	penup;
	penup = 1;
	opengr(O_TEXT,filedirnam);
	outst("\033PpS(M0(AD)1(AB)2(AR)3(AG))\n");
#ifndef DEMO
	hcpy_summ();/* show hull info on hardcopy */
#endif
}

void regien()
{
	regimv(0.0,0.0);
	closgr();
}

void reginl()
{
	extern	INT	penup;
	penup = 1;
}

void regicr(INT ncol)
{
    char *colstr[] = {"W(I(B))","W(I(R))","W(I(Y))"};
    extern int current_hull;

    if(current_hull != 0) return;	/* one colour for overlay */
    outst(colstr[(ncol - 1) % 3]);
}

void regist(char *ix)
{
	outst("W(I2)W(R)T\"");
	outst(ix);
	outst("\"\n");
}

void regimv(REAL x,REAL y)
{
	extern	INT	penup;
	trans(&x,&y);
	outst("P[");
	wrireg(x,y);
	penup = 0;
}

void wrireg(REAL x,REAL y)
{
	extern	INT	device;
	char	str[12];
	extern	REAL	Yfactor;

	sprintf(str,"%04d,%04d]\n",(int) (x*1.125),
		(int) (479.0*Yfactor - y));
	outst(str);
}

void regidr(REAL x,REAL y)
{
	extern	INT	penup;

	if(penup) {
		regimv(x,y);
	} else {
		trans(&x,&y);
		outst("W(V)V[");
		wrireg(x,y);
	}
}

#endif
