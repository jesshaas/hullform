/* Hullform component - hullpa.c
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
 
/*      return hull bottom or skeg surface parameters   */

#include "hulldesi.h"
extern int *relcont;
int previous_getparam_line;

void hullpa(INT k,INT j,REAL prevwid,REAL  prevris,REAL *a,REAL *hb,REAL *c,REAL *hd)
{
	REAL yl,zl,yl1,zl1;
	REAL x0,z0,zc;
	REAL yc,dy,dz;
	int	pre;

	if(j <= 0) {
		*a = 0.0;
		*hb = 0.0;
		*c = 1.0;
		*hd = 0.0;
		return;
	}

	pre = j-1;
	if(k < maxsec-2) {
		while(pre > 0 &&
			    (stsec[pre] > k || ensec[pre] < k)) pre--;
	}
	else {
		x0 = (xsect[k] > xsect[0]) ? xsect[k] : xsect[0];
		while(pre > 0 &&
			    (x0<xsect[stsec[pre]] || x0>xsect[ensec[pre]]) ) pre--;
	}
	zc = zcont[j][k];
	yc = ycont[j][k];
	yl = yline[j][k];
	zl = zline[j][k];
	yl1 = yline[pre][k];
	zl1 = zline[pre][k];
	dy = yl1 - yl;
	dz = zl - zl1;

	if(!relcont[j]) {
		if(yc == yl1 && zc == zl1) {	// Without these bits, a double waterline intersection with a hull outline can occur twice at the same spot.
			*a = yl1 - yl;
			*hb = 0.0;
			*c = zl - zl1;
			*hd = 0.0;
		} else {
			*a = 2.0*(yc - yl);
			*hb = dy - *a;
			*c = 2.0*(zl - zc);
			*hd = dz - *c;
		}
	} else {

		if(dz == 0.0 && dy == 0.0) {
			*a = 0.0;
			*hb = 0.0;
			*c = 0.0;
			*hd = 0.0;
		} else if(yc == 0.0 && zc == zl1) {
			*a = yl1 - yl;
			*hb = 0.0;
			*c = zl - zl1;
			*hd = 0.0;
		} else {
			zc = zl - zc;    /* control point, distance above base */
			z0 = dz - zc;    /* control point, distance below top */
			yc += dy;
			if(fabs((REAL) prevris) > fabs((REAL) prevwid)*0.0001)
				yc -= prevwid/prevris*z0;
			*a = yc + yc;
			*c = zc + zc;
			*hb = dy - *a;
			*hd = dz - *c;
		}
	}
}

/*	INTER-SEGMENT TRANSFER OF HULL PARAMETERS	*/

void tranpa(REAL a,REAL hb,REAL c,REAL hd,REAL *aa,REAL *cc)
{
	if(fzer(a) && fzer(c)) {
		if(fnoz(hb) || fnoz(hd)) {
			*aa = hb;
			*cc = hd;
		}
		/* note zero-sized curve leaves aa and cc unchanged */
	}
	else {
		*aa = a;
		*cc = c;
	}
}

/*	SOLVE FOR INTERSECTION OF WATERLINE WITH (QUADRATIC) BOTTOM CURVE	*/

void inters(REAL a,REAL hb,REAL c,REAL hd,REAL sina,REAL cosa,REAL dwl,
	REAL *t1,REAL *t2)
{

	/*	A,HB,C,HD ARE QUADRATIC CURVE PARAMETERS (SEE BELOW)	*/
	/*	SINA,COSA SPECIFY WATERLINE SLOPE (SEE BELOW)	*/
	/*	DWL IS PERPENDICULAR DISTANCE FROM BASE OF CURVE TO WATERLINE	*/
	/*	T1,T2 ARE RETURNED VALUES OF CURVE PARAMETER T, T1 >= T2	*/

	/*	WATERLINE IS  Z COSA = Y SINA - DWL		*/
	/*	CURVE IS           Z = C T + (1/2 D) T**2	*/
	/*			   Y = A T + (1/2 B) T**2	*/

	REAL	det,t0,t3,t4,t;

	t4 =      fsub(fmul(cosa,c ),fmul(sina,a));
	t3 = fmul(fsub(fmul(cosa,hd),fmul(sina,hb)),2.0);
	det = fadd( fmul(t4,t4) , fmul(2.0,fmul(t3,dwl)) );

	/*	IF THERE IS A SOLUTION:	*/

	if(det >= 0.0) {
		t0 = fsqr0(det);

		/*		IF THE CURVATURE IS NEGLIGIBLE, THERE IS AT MOST ONE	*/
		/*			SOLUTION					*/

		t = fsub(fabf(t3),fmul( 0.5e-4 , fadd(fabf(t0),fabf(t4)) ));
		if(t < 0.0) {

			/*			IF THERE IS NO VALID SOLUTION:	*/

			if(t4 == 0.0) {
				*t1 = 1.0e+16;
				*t2 = -1.0e+16;
			}
			else {

				/*			OTHERWISE, THERE IS ONE VALID SOLUTION		*/

				*t2 = fdiv(dwl,t4);
				if(t4 > 0.0) {
					*t1 = 1.0e+16;
				}
				else {
					*t1 = *t2;
					*t2 = -1.0e+16;
				}
			}
		}
		else if(fnoz(t3)) {

			/*		IF THE TWO SOLUTIONS ARE NOT IDENTICAL:	*/

			*t1 =      fdiv(fsub(t0,t4),t3);
			*t2 = fmin(fdiv(fadd(t0,t4),t3));

			/*			ENSURE THAT T1 > T2	*/

			if(*t2 > *t1) {
				t = *t2;
				*t2 = *t1;
				*t1 = t;
			}
		}
		else {
			*t1 = 1.0e+16;
			*t2 = -1.0e+16;
		}
	}
	else {

		/*	IF THERE IS NO SOLUTION:	*/

		*t1 = 1.0e+16;
		*t2 = -1.0e+16;
	}
}

/*	Get hull curve parameters for section i and curve between i-1 and i*/

void getparam(int i,int j,REAL *a,REAL *hb,REAL *c,REAL *hd)
{
	REAL aa = 0.0;
	REAL cc = 1.0;
	int il,ip,ix = 0;
	if(j <= 0) {
		*a = 0.0;
		*hb = 0.0;
		*c = 1.0;
		*hd = 0.0;
	}
	else if(stsec[j] <= i && ensec[j] >= i) {
		*a = 0.0;
		*c = 1.0;
		il = j;
		while(il > 1 && relcont[il]) il--;
		while(il <= j) {
			if(stsec[il] <= i && ensec[il] >= i || i >= maxsec-2) {
				ip = ix;
				ix = il;
				hullpa(i,il,aa,cc,a,hb,c,hd);
				tranpa(*a,*hb,*c,*hd,&aa,&cc);
			}
			il++;
		}
		previous_getparam_line = ip;
	}
	else {
		*a = 0.0;
		*hb = 0.0;
		*c = 0.0;
		*hd = 0.0;
	}
}

