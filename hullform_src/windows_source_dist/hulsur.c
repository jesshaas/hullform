/* Hullform component - hulsur.c
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
 
/*    FIND AREA OF WETTED SURFACE, FROM HULL CENTREPLANE TO WATERLINE	*/

/*    SINAL/COSAL    IS SLOPE OF WATERLINE, TRANSVERSE TO HULL AXIS	*/

#include "hulldesi.h"

extern int webinclude;

int whole_surface = FALSE;	// temporarily set TRUE to get whole hull surface
REAL	surf_xsum,surf_zsum;

REAL hulsur(REAL sinal,REAL cosal)
{
	INT    k,l0,l1 = 0;
	REAL    area;
	REAL    t0,t1,dt0,dt1;
	REAL   t0max,t1max;
	int    i,i0,i1;
	REAL    a0[maxlin],hb0[maxlin],c0[maxlin],hd0[maxlin];
	REAL    a1[maxlin],hb1[maxlin],c1[maxlin],hd1[maxlin];
	REAL    xyz0[4],xyz1[4],xyz2[4];
	REAL    aa,cc;
	REAL    w0,w1;
	int     pl0[maxlin],pl1[maxlin];	/* prior line */

	surf_xsum = 0.0;
	surf_zsum = 0.0;

	area = 0.0;
	a1[0] = 0.0;

	/*	These are dummies, but required for stability with some numeric processors	*/

	pl1[1] = 0;
	a1[1]  = 0.0;
	hb1[1] = 0.0;
	c1[1]  = 0.0;
	hd1[1] = 0.0;

	for(k = 0 ; k < count ; k++) {

		/*    FIND HULL CURVE COEFFICIENTS	*/

		/*    THE "0" VALUES REFER TO THE PREVIOUS SECTION, THE "1" VALUES TO THE */
		/*    CURRENT ONE							*/

		/*	DECK COEFFICIENTS	*/

		a0[0] = a1[0];
		hb0[0] = 0.0;
		c0[0] = 0.0;
		hd0[0] = 0.0;
		a1[0] = fmin(yline[0][k]);
		hb1[0] = 0.0;
		c1[0] = 0.0;
		hd1[0] = 0.0;
		aa = 0.0;
		cc = 1.0;
		pl0[0] = -1;
		pl1[0] = -1;

		/*	HULL SEGMENT COEFFICIENTS	*/

		l0 = l1;
		l1 = 0;
		for(i = 1 ; i < numlin ; i++) {
			pl0[i] = pl1[i];
			a0[i] = a1[i];
			hb0[i] = hb1[i];
			c0[i] = c1[i];
			hd0[i] = hd1[i];
			pl1[i] = -1;
			if(stsec[i] > k || ensec[i] < k) continue;
			hullpa(k,i,aa,cc,&a1[i],&hb1[i],&c1[i],&hd1[i]);
			tranpa(a1[i],hb1[i],c1[i],hd1[i],&aa,&cc);
			pl1[i] = l1;
			l1 = i;
		}

		/*    AT FIRST SECTION, SKIP REST OF CODE WHEN INITIAL VALUES HAVE	*/
		/*    BEEN SET UP				*/

		if(k == 0) continue;

		/*    After the following lines, (xyz0[0],xyz0[1],xyz0[2]) is the	*/
		/*    base of the previous section, and xyz0[3] is the waterlevel	*/
		/*    clearance of the point			*/

		setxyz(l0,k-1,0.0,xyz0,sinal,cosal,0.0,0.0,0.0,0.0);

		/*    After the following lines, (xyz1[0],xyz1[1],xyz1[2]) is the	*/
		/*    base of the current section, and xyz1[3] is the waterlevel	*/
		/*    clearance of the point			*/

		setxyz(l1,k  ,0.0,xyz1,sinal,cosal,0.0,0.0,0.0,0.0);

		/*    Find area to centreline			*/

		aa = xyz1[0] - xyz0[0];    /* x-distance */
		cc = xyz1[2] - xyz0[2];    /* z-distance */
		w0 = xyz0[1];
		w1 = xyz1[1];
		aa = sqrt(aa*aa + cc*cc);
		/* thus slant distance */
		if(xyz0[3] > 0.0) {
			if(xyz1[3] < 0.0) {
				t0 = xyz1[3] / (xyz1[3] - xyz0[3]);
				aa *= t0;
				w0 = w1+t0*(w0 - w1);
			}
			else {
				aa = 0.0;	/* no immersed area */
			}
		}
		else {
			if(xyz1[3] > 0.0) {
				t0 = fdiv(xyz0[3],fsub(xyz0[3],xyz1[3]));
				aa = fmul(aa,t0);
				w1 = fadd(w0,fmul(t0,fsub(w1,w0)));
			}
		}
		aa = fmul(aa,fmul(0.5,fadd(w0,w1)));
		/* multiply by mean width to give area */
		area = fadd(area,aa);

		i1 = l1;    /* line index on current section	*/
		i0 = l0;    /* line index on previous section	*/
		t0 = 0.0;
		t1 = 0.0;
		dt0 = (float) (i0 - pl0[i0]);
		dt0 = 0.1 / dt0;
		t0max = 1.0 - 0.5*dt0;
		dt1 = (float) (i0 - pl0[i0]);
		dt1 = 0.1 / dt1;
		t1max = 1.0 - 0.5*dt1;

		while(i0 > 0 && i1 > 0) {

			if(t0 >= t0max) {
				i0 = pl0[i0];
				if(i0 >= 0) {
					t0 -= 1.0;
					dt0 = (float) (i0 - pl0[i0]);
					dt0 = 0.1 / dt0;
				}
				else {
					break;
				}
			}

			if(t1 >= t1max) {
				i1 = pl1[i1];
				if(i1 >= 0) {
					t1 -= 1.0;
					dt1 = (float) (i0 - pl0[i0]);
					dt1 = 0.1 / dt1;
				}
				else {
					break;
				}
			}

			t0 += dt0;
			t1 += dt1;

			/*    After the following lines, (xyz2[0],xyz2[1],xyz2[2]) is the	*/
			/*    base of the previous line and section.  xyz2[3] is the	*/
			/*    waterlevel clearance of the point, on return from "surfar".	*/

			setxyz(i0,k-1,t0,xyz2,sinal,cosal,a0[i0],hb0[i0],c0[i0],hd0[i0]);

			/*    THIS CALL CALCULATES THE AREA OF THE TRIANGLE BETWEEN	*/
			/*    XYZ0, XYZ1 AND XYZ2, THE POINT ON THE HULL BOTTOM CURVE	*/
			/*    CORRESPONDING TO THE CURRENT VALUE OF "T", AT THE PREVIOUS	*/
			/*    SECTION.  AFTERWARDS, XYZ2 THE POINT FOUND BECOMES XYZ0.	*/

			aa = surfar(xyz0,xyz1,xyz2);
			area = fadd(area,aa);

			/*    THIS CALL CALCULATES THE AREA OF THE TRIANGLE BETWEEN	*/
			/*    XYZ0, XYZ1 AND THE POINT XYZ2, ON THE HULL		*/
			/*    BOTTOM CURVE CORRESPONDING TO THE CURRENT VALUE OF "T", AT THE	*/
			/*    CURRENT SECTION.  THE POINT FOUND BECOMES XYZ1.		*/

			setxyz(i1,k,t1,xyz2,sinal,cosal,a1[i1],hb1[i1],c1[i1],hd1[i1]);
			aa = surfar(xyz1,xyz0,xyz2);
			area = fadd(area,aa);

		}
	}

	if(whole_surface) {
		if(area > 0.0) {
			surLCG = surf_xsum / area;
			surVCG = surf_zsum / area;
		} else {
			surLCG = -999.9;
			surVCG = -999.9;
		}
	}
	return(area);
}

/*    CALCULATE IMMERSED SURFACE AREA WITHIN TRIANGLE DELIMITED BY	*/
/*    COORDINATES C0,C1 AND C2			*/

REAL surfar(REAL c0[],REAL c1[],REAL c2[])
{
	REAL    ca[4],cb[4];
	REAL    Surfar;
	INT    i;

	Surfar = 0.0;

	if(fpos(c0[3])) {

		/*    ... if the first corner is not immersed ...		*/

		if(fpos(c1[3])) {

			/*    ... and the second corner is not immersed ...		*/

			if(fnez(c2[3])) {

				/*    ... then if the third is immersed, interpolate to find corners	*/
				/*    of the immersed sub-triangle, and add its area to the total	*/

				linint(c0,c2,ca);
				linint(c1,c2,cb);
				Surfar += triar(ca,cb,c2);
			}
		}
		else {

			/*    If the first corner is not immersed, and the second is immersed	*/

			/*    ... firstly, find the point on the line between where the	*/
			/*    waterline intersects			*/

			linint(c0,c1,ca);
			if(fpos(c2[3])) {

				/*    then if the third point is not immersed, interpolate to find	*/
				/*    waterline intersection on line from second to third, and find	*/
				/*    area of triangle below, which adds to total		*/

				linint(c1,c2,cb);
				Surfar += triar(ca,cb,c1);
			}
			else {

				/*    if the third is immersed, find intersection on line from	*/
				/*    first to third, then add area of whole,triangle, less that of	*/
				/*    triangle above waterline			*/

				linint(c0,c2,cb);
				Surfar += triar(ca,cb,c0);
			}
		}
	}
	else {

		/*    If the first corner is immersed ...		*/

		if(fpos(c1[3])) {

			/*    ... and the second is not immersed ...		*/
			/*    ... find the waterline intersection point between:	*/

			linint(c0,c1,ca);
			if(fpos(c2[3])) {

				/*    If the third point is not immersed, interpolate to find second	*/
				/*    waterline intersection, and add area below these ends	*/

				linint(c0,c2,cb);
				Surfar += triar(ca,cb,c0);
			}
			else {

				/*    else, if third point is immersed, add whole area, less that	*/
				/*    of the triangle out of the water		*/

				linint(c1,c2,cb);
				Surfar += triar(c0,c1,c2) - triar(c1,ca,cb);
			}
		}
		else {

			/*    First corner immersed, and second also		*/
			/*    ... will need to add total area in either case ...	*/

			Surfar += triar(c0,c1,c2);
			if(fpos(c2[3])) {

				/*    ... and if third point is not immersed, interpolate to find	*/
				/*    the waterline intersections on adjacent lines, and subtract	*/
				/*    the area enclosed, above the waterline		*/

				linint(c0,c2,ca);
				linint(c1,c2,cb);
				Surfar -= triar(ca,cb,c2);
			}
		}
	}

	for(i = 0 ; i < 4 ; i++) c0[i] = c2[i];

	return(Surfar);
}

/*    AREA OF TRIANGLE BETWEEN POINTS A, B AND C	*/

REAL triar(REAL A[],REAL B[],REAL C[])
{
	REAL    a,b,c,s;
	REAL	tri_area;

	/*	Any web on the centreline is presumed fictitious	*/

	if(A[1] < 0.00001 && B[1] < 0.00001 && C[1] < 0.00001 && !webinclude) return 0.0;

	a = sqrt(sq(A[0],B[0])+sq(A[1],B[1])+sq(A[2],B[2]));
	b = sqrt(sq(B[0],C[0])+sq(B[1],C[1])+sq(B[2],C[2]));
	c = sqrt(sq(C[0],A[0])+sq(C[1],A[1])+sq(C[2],A[2]));
	s = 0.5*(a+b+c);

	tri_area = sqrt(max(0.0,s*(s-a)*(s-b)*(s-c)));
	if(whole_surface) {
		surf_xsum += tri_area * (A[0] + B[0] + C[0])*0.333333;
		surf_zsum += tri_area * (A[2] + B[2] + C[2])*0.333333;
	}
	return tri_area;
}

REAL sq(REAL a,REAL b)
{
	REAL temp;

	temp = a - b;
	return(temp*temp);
}

/*    LINEAR INTERPOLATION BETWEEN COORDINATES C0 AND C1, TO C2	AT ZERO WATERLINE CLEARANCE */

void linint(REAL c0[],REAL c1[],REAL c2[])
{
	INT    i;
	REAL    fac;

	fac = c0[3] / (c0[3] - c1[3]);
	for(i = 0 ; i < 3 ; i++)
		c2[i] = c0[i] + (c1[i] - c0[i])*fac;
}

void setxyz(int l,int i,REAL t,REAL xyz[4],REAL sinal,REAL cosal,REAL a,REAL hb,REAL c,REAL hd)
{
	xyz[0] = xsect[i];
	xyz[1] = yline[l][i] + t*(a+t*hb);
	xyz[2] = zline[l][i] - t*(c+t*hd);
	if(i == 0) {
		xyz[0] -= xyz[1];
		xyz[1] = yline[stemli][1];
	}
	if(whole_surface)
		xyz[3] = -1.0;
	else
		xyz[3] = (wl - beta*xyz[0] - hwl[i]) - (cosal*xyz[2] + sinal*xyz[1]);
}

