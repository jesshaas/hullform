/* Hullform component - drasec.c
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

/*	DRAW SECTION.  THIS ROUTINE, AND OTHERS, IS MOVED TO A SUBROUTINE	*/
/*	TO ALLOW READY CHANGES IN THE PARAMETRIC MODEL IN FUTURE.  K IS	*/
/*	THE SECTION NUMBER, AND "FAC" MAY BE 1 OR -1, CORRESPONDING TO	*/
/*	OPPOSITE SIDES OF THE HULL	*/

extern int endlin;
extern int stalin;
extern int tankcol;
void xorcol(int);
#ifndef STUDENT
extern int showport,showstbd;
#endif

/*	"right" is not used in student editions. It can be 1, 0 or -1 */

void drasec(INT k,int right,REAL fac,int mode)
{
	INT	 j,i,jp;
	extern int numbetwl;
	int numperl = numbetwl+1;
	REAL DT = -1.0/((REAL) numperl);
	REAL DT2 = DT+DT;
	REAL a,hb,c,hd,aa,cc;
	REAL a1,b1,c1,d1;
	REAL x1,y1;
	REAL xd,yd;
	REAL dt,dt2;
	int startline = -1;
	int surfaces;

#ifdef PROF
	REAL xt;
	int is_inside;
	int possible_int;
	REAL xs = xsect[k];
	int kt = 0;			/* transom table index */
	extern int numtran;
	REAL xp,xc = xtran[0];
	int was_inside = !transom || fle(xs,xc);

	tank = 0;
#endif

	if(mode <= 3 || mode == 10 || mode == 11) (*newlin)();

	aa = 0.0;
	cc = 1.0;
	if(endlin < numlin) endlin = numlin;

	jp = stalin;
#ifndef STUDENT
	while(jp < extlin && (stsec[jp] > k || ensec[jp] < k)) {
		jp++;
	}
	j = jp+1;
#else
	j = 1;
#endif

	x1 = fac * yline[jp][k];
	y1 = zline[jp][k];
	surfaces = 1;	// number of surfaces between plotted lines, normally 1 but greater when lines between are partial

	while(j < endlin) {

		/*	check for tank starting at this line ...			*/

#ifndef STUDENT

		if(j == fl_line1[tank]) {

			/*      End the previous outline, and close the tank if necessary	*/

			pltsel(0.0,x1,y1,mode);
			if(startline >= 0)
				pltsel(0.0,fac*yline[startline][k],zline[startline][k],mode);

			/*	Skip to the next tank on this side	*/

			/*	right = -1 indicates starboard side tanks should not be plotted
			right = 1 indicates port side tanks should not be plotted
			right = 0 indicates all tanks should be plotted
			*/
			for( ; tank < ntank ; tank++) {
				j = fl_line1[tank];
				if(stsec[j] > k || ensec[j] < k) continue;
				if(fl_right[tank]) {
					if(right != -1 || showstbd) break;
				} else {
					if(right != 1 || showport) break;
				}
			}

			if(tank >= ntank) return;/*break;*/
			if(tankcol > 0 && tank < ntank) {
				xorcol(tankcol);
				tankcol = 0;
			}

			x1 = fac*yline[j][k];
			y1 = zline[j][k];

			if(mode <= 3 || mode >= 10) {	/* normal graphics */
				(*newlin)();
			}
			else if(mode <= 7) {		/* DXF section output */
				seqend();
				ensure_polyline('S',k,mode);
			}
			aa = 0.0;		/* re-initialise curve	*/
			cc = 1.0;
			startline = j;
			jp = j-1;
			tank++;
			j++;
		}

		if( (stsec[j] <= k && ensec[j] >= k) || (k >= maxsec-2 &&
			    xs >= xsect[stsec[j]] && xs <= xsect[ensec[j]]) ) {

			possible_int = transom && j < numlin;
#endif

			getparam(k,j,&a,&hb,&c,&hd);
			/* get curve parameters		*/

			/*	If the section curvature is zero, calculation need only look	*/
			/*	at either end							*/

			if(
	#ifndef  STUDENT
			possible_int ||
	#endif
			    zcont[j][k] != zline[jp][k] || ycont[j][k] != (relcont[j] ? 0.0 : yline[jp][k])
				    ) {
				dt = DT / surfaces;
				dt2 = DT2 / surfaces;
				i = numbetwl + numperl*(surfaces-1);
			}
			else {
				dt = -1.0;
				dt2 = -2.0;
				i = 0;
			}

			/*	The parameters below are used to speed the calculation of the	*/
			/*	quadratic terms in the hull curve co-ordinates			*/

			b1 = hb * dt2 * fac;
			d1 = hd * dt2;
			a1 = fac*dt*(a + hb*dt) + b1;
			c1 =     dt*(c + hd*dt) + d1;

			xd = b1 * dt;		/* changes between	*/
			yd = d1 * dt;		/* steps of x1 and y1	*/

			do {
#ifndef STUDENT
				if(was_inside)
					pltsel(0.0,x1,y1,mode);

				if(possible_int && ++kt < numtran) {
					xp = xc;
					xc = xtran[kt];
					is_inside = (xs <= xc);
					if(was_inside != is_inside) {	/* transom intersection */
						xt = (xs-xp)/(xc-xp);
						pltsel(0.0,	fac*(ytran[kt-1]+xt*(ytran[kt]-ytran[kt-1])),
									ztran[kt-1]+xt*(ztran[kt]-ztran[kt-1]),mode);
//						pltsel(0.0,x1+xt*a1,y1-xt*c1,mode);
//						if(was_inside) (*newlin)();
						was_inside = is_inside;
					}
				}
#else
				pltsel(0.0,x1,y1,mode);
#endif
				x1 += a1;
				y1 -= c1;
				a1 += xd;
				c1 += yd;
			}
			while(i--);

			tranpa(a,hb,c,hd,&aa,&cc);
			jp = j;
			surfaces = 1;
			j++;
#ifndef STUDENT
		}
		else {

			/*	Move to the next drawable outline segment	*/

			while(j < endlin && ((stsec[j] > k || ensec[j] < k) ||
				    (k >= maxsec-2 && xs < xsect[stsec[j]] && xs > xsect[ensec[j]]) ) ) {
				surfaces++;
				j++;
			}
		}
#endif
	}

#ifndef STUDENT
	if(was_inside) {
#endif
		pltsel(0.0,x1,y1,mode);
		if(startline >= 0)
			pltsel(0.0,fac*yline[startline][k],zline[startline][k],mode);

#ifndef STUDENT
	}
#endif
	stalin = 0;
}

