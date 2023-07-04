/* Hullform component - calcar.c
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
 
/*      CALCULATE AREA FOR WHOLE HULL SECTION                   */
/*								*/
/*	This routine is called either with j1 = 0 & j2 = numlin	*/
/*	or with j1 the start of one set of tank lines, j2 the	*/
/*	start of the next					*/

#include "hulldesi.h"

void calcar(INT i,int j1,int j2,int left,int right,
	REAL *area1,	/* area of previous section (set to input value of area2) */
	REAL *area2,	/* calculated area of next section */
	REAL *hbeam1,	/* previous half-beam */
	REAL *hbeam2,	/* next half-beam */
	REAL *cl1,		/* previous waterline clearance */
	REAL *cl2,		/* next waterline clearance */
	REAL xref1,		/* x-value at previous section */
	REAL xref2,		/* x-value at next section */
	REAL *rsum,		/* righting moment per unit length and radian of heel */
	REAL *volume,	/* displaced volume of water */
	REAL *calcdisp,	/* calculated displacement */
	REAL *totmom,	/* integrated sectional area with respect to length */
	REAL *watpla,
	REAL wll,		/* waterline offset to use */
	REAL densty,	/* liquid densty */
	REAL *wllen,	/* waterline length */
	REAL *xsta,		/* waterline entry point */
	REAL *xend,		/* waterline exit point */
	REAL *sumlcf,	/* integrated width with respect to length squared */
	REAL *summct,	/* integrated width with respect to length cubed, times density */
	REAL *ww1,		/* previous wetted width */
	REAL *ww2,		/* current wetted width */
	REAL *rm1,		/* previous righting moment per unit length */
	REAL *rm2,		/* current righting moment per unit length */
	REAL *sumysqdif,/* summed difference of squared edge offsets */
	REAL *sumyoffs)	/* summed edge offsets */
{
	REAL	dz2,hb1,hb2,wetw1,wetw2,wwcub1,wwcub2,zw1,zw2;
	INT		nl,il,j;
	REAL	wwcube,ylow,zlow,t,y,z,zref,wplus,wminus;
	REAL	xref,x1,x2,y1,y2,delx;
	REAL	term1,term2,term3,term4;
	REAL	areainl,areaoutl;
	REAL	areainr,areaoutr;
	REAL	volincr;
	extern int	tankmode;

	/*	SAVE PREVIOUS CROSS-SECTIONAL AREA, HALF-BEAM,	*/
	/*	WETTED HULL WIDTH AND WATERLINE CLEARANCE	*/

	(*area1) = (*area2);
	(*hbeam1) = (*hbeam2);
	(*ww1) = (*ww2);
	*cl1 = *cl2;

	/*	GET CURRENT VALUES	*/

	dz2 = wll - (beta*xsect[i] + hwl[i]);	/* waterline offset */
	if(right) {
		hullar(i,j1,j2,&areainl,&areaoutl,dz2, sina,cosa,&hb1,&wetw1,&wwcub1,&zw1,&il);
	}
	else {
		areainl = areaoutl = wetw1 = hb1 = wwcub1 = zw1 = 0.0;
	}
	if(left) {
		hullar(i,j1,j2,&areainr,&areaoutr,dz2,-sina,cosa,&hb2,&wetw2,&wwcub2,&zw2,&il);
	}
	else {
		areainr = areaoutr = wetw2 = hb2 = wwcub2 = zw2 = 0.0;
	}
	*area2 = areainl + areainr;

	/*	WETTED WIDTH OF HULL	*/

	(*ww2) = wetw1 + wetw2;
	wwcube = wwcub1 + wwcub2;

	/*	CALCULATE HALF-BEAM ALONG WATERLINE	*/

	if(!right) {	/* left only */
		*hbeam2 = cosa*hb2 + sina*zw2;
	}
	else if(!left) {	/* right only */
		*hbeam2 = cosa*hb1 - sina*zw1;
	}
	else {		/* both (no "neither" call) */
		*hbeam2 = cosa*(hb2+hb1) + sina*(zw2-zw1);
	}

	/*	TEST TO SEE IF IT EXPANDS WATERLINE BEAM TERM	*/

	if(*hbeam2 > bwl) bwl = *hbeam2;

	/*	TEST TO SEE IF SECTION AREA INCREASES MAXIMUM SO FAR	*/

	if(*area2 > garea) garea = *area2;

	/*	EVALUATE CURRENT WATERLINE CLEARANCE OF SECTION	*/

	(*cl2) = cleara(i,dz2,&ylow,&zlow,&nl,&il);

	/*	TEST TO SEE WHETHER CLEARANCE INCREASES DRAUGHT (WHEN NEGATIVE)	*/

	if(*cl2 < tc) tc = *cl2;

	/*	CORRECT FOR LENGTH REDUCTION WHERE WATERLINE INTERSECTS KEEL.	*/

	if((*cl1) > 0.0) {
		if((*cl2) <= 0.0) {
			t = *cl2 / (*cl2 - *cl1);
			xref = xref2 - (xref2 - xref1) * t;
			x1 = xref;
			if((*xend) < xref)(*xend) = xref;	/* from xref2 15/9/00 */
			if((*xsta) > xref)(*xsta) = xref;	/* from xref1 15/9/00 */
			if(xentry < -1.0e+16) xentry = xref;

//	Find wetted width at the waterline entry - nonzero for a scow shape

			wplus = wminus = 0.0;
			for(j = j1 ; j < j2 ; j++) {
				y = yline[j][i] - (yline[j][i]-yline[j][i-1])*t;
				z = zline[j][i] - (zline[j][i]-zline[j][i-1])*t;
				zref = z*cosa + y*sina;
				if(zref > dz2 - 0.001 && y > wplus) wplus = y;
				zref = z*cosa - y*sina;
				if(zref > dz2 - 0.001 && y > wminus ) wminus = y;
			}
			*ww1 = wplus + wminus;
		}
		else {
			x1 = xref2;
		}
		x2 = xref2;
	}
	else {
		if((*cl2) > 0.0) {
			t = *cl1 / (*cl1 - *cl2);
			(*xend) = xref1 + (xref2 - xref1) * t;
			x2 = (*xend);

//	Find wetted width at the waterline exit - nonzero for a scow shape

			wplus = wminus = 0.0;
			for(j = j1 ; j < j2 ; j++) {
				y = yline[j][i-1] + (yline[j][i]-yline[j][i-1])*t;
				z = zline[j][i-1] + (zline[j][i]-zline[j][i-1])*t;
				zref = z*cosa + y*sina;
				if(zref > dz2 - 0.001 && y > wplus) wplus = y;
				zref = z*cosa - y*sina;
				if(zref > dz2 - 0.001 && y > wminus ) wminus = y;
			}
			*ww2 = wplus + wminus;
		}
		else {
			if((*xsta) > xref)(*xsta) = xref;	/* from xref1 15/9/00 */
			if((*xend) < xref)(*xend) = xref;	/* from xref2 15/9/00 */
			x2 = xref2;
		}
		x1 = xref1;
	}
	delx = x2 - x1;
	(*rm1) = (*rm2);
	(*rm2) = wwcube * densty;	/* "per unit length" */

	if(delx != 0.0) {

		if(tankmode) {
			y1 = *hbeam1 + *ww1;	/* *ww1, *ww2 negative */
			y2 = *hbeam2 + *ww2;
			*sumysqdif += 0.5*delx*(*hbeam2 * *hbeam2 + *hbeam1* *hbeam1
				    - y2*y2 - y1*y1);
			*sumyoffs += 0.5*delx*(*hbeam2 + *hbeam1 + y2 + y1);
		}

		/*	THIS IS THE RIGHTING MOMENT PER UNIT LENGTH AND RADIAN OF HEEL	*/

		*rsum += 0.166667*delx*(*rm1 + *rm2);

		/*	HENCE TOTAL VOLUME AND DISPLACEMENT SO FAR	*/

		volincr = 0.5 * delx * (*area1 + *area2);
		*volume += volincr;
		*calcdisp += densty * volincr;

		/*	AND TOTAL PITCHING MOMENT SO FAR, ABOUT X=0 REFERENCE (FIRST	*/
		/*	MOMENT OF SECTIONAL AREAS)					*/

		term1 = *area1*x2 - *area2*x1;
		term2 = *area2 - *area1;
		term3 = x1 + x2;
		term4 = 0.333333*(x2*x2 + x1*term3);
		*totmom += densty*(0.5*term3*term1 + term2*term4);
		*watpla += 0.5*delx*(*ww1 + *ww2);

		/*	AND SUM OF FIRST MOMENT OF WATERPLANE AREA, TO FIND CENTRE OF FLOTATION
		*/
		term1 = *ww1*x2 - *ww2*x1;
		term2 = *ww2 - *ww1;

		*sumlcf += 0.5*term3*term1 + term2*term4;

		/*	AND SUM OF SECOND MOMENT OF WATERPLANE AREA, TO FIND MOMENT
		TO CHANGE TRIM ONE RADIAN
		*/
		*summct += densty*(term4*term1+0.25*term2*(x2*x2+x1*x1)*term3);

		/*	AND WATERLINE LENGTH	*/

		*wllen += delx;

	}

	/*	area to be used next time is "area out", not "area in"	*/

	*area2 = areaoutl + areaoutr;
}


