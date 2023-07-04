/* Hullform component - hullmome.c
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
 
/*	SINGLE-SIDE RIGHTING MOMENT	*/

/*	FUNCTION "RIGHTM" CALLS FUNCTION "SEC MOM" TO GET THE MOMENTS FROM	*/
/*	EACH SEGMENT OF THE HULL SECTION, OVER THE DECK AND HULL BOTTOM CURVES.	*/
/*	"SEC MOM" CALLS FUNCTION "CURMOM", WHICH EVALUATES THE INTEGRALS OVER	*/
/*	THE CURVES THEMSELVES.	*/

#include "hulldesi.h"

void rightm(int k,int j1,int j2,REAL sinal,REAL cosal,
		REAL wl1,REAL *zsum,REAL *rmin,REAL *rmout)
{
/*	k is section index			*/
/*	j1 and j2 is the line number range over	*/
/*		which the moment is evaluated	*/
/*	SINAL,COSAL SPECIFY THE WATERLINE SLOPE	*/
/*	K IS INDEX IN HULL SECTION TABLE	*/
/*	WL1 IS LOCAL WATERLINE OFFSET		*/
/*	rmin is righting moment per unit length for non-commencing lines */
/*	rmout is same for non-terminating lines	*/
/*	zsum is area * depth sum, used to find centre of buoyancy */

    INT		i;
    REAL	a,hb,c,hd,aa,cc;
    REAL	v;
    REAL	wl0;
    int		k0 = (k >= maxsec-1) ? 0 : k;
    REAL	y0,z0,y1,z1;

/*	INITIALISE AREA * DEPTH SUM	*/

    *zsum = 0.0;

    y1 = yline[j1][k];
    z1 = zline[j1][k];
    if(j1 == 0) {
	y0 = 0.0;
	z0 = z1;
    } else {
	y0 = yline[j2-1][k];
	z0 = zline[j2-1][k];
    }

/*	SUBTRACT ANY CONTRIBUTION DUE TO OVER-DECK WATER (USUALLY ZERO)	*/

/*    v = secmom(y0,z0,y1-y0,0.0,z0-z1,0.0,sinal,cosal,wl1,zsum);*/
    v = secmom(y1,z1,y0-y1,0.0,z1-z0,0.0,sinal,cosal,wl1,zsum);
    *rmin = v;
    *rmout = v;

/*	GET HULL SEGMENT CURVE PARAMETERS, AND THEN EVALUATE RIGHTING	*/
/*	MOMENT FOR EACH HULL CURVE SEGMENT				*/

    aa = 0.0;
    cc = 1.0;
    wl0 = wl1;	/* set waterline to that given		*/

    for(i = j1 + 1 ; i < j2 ; i++) {

	if(stsec[i] > k0 || ensec[i] < k0) continue;
	hullpa(k,i,aa,cc,&a,&hb,&c,&hd);

	v = secmom(yline[i][k],zline[i][k],a,hb,c,hd,sinal,cosal,wl0,zsum);

	if(stsec[i] < k0) *rmin = fadd(*rmin,v);

	if(ensec[i] > k0) *rmout = fadd(*rmout,v);

	tranpa(a,hb,c,hd,&aa,&cc);
    }

/*	ADD CONTRIBUTION DUE TO WIDTH OF SKEG BASE	*/

    if(j2 == numlin && yline[numlin-1][k] != 0.0) {
	v = secmom(0.0,zline[numlin-1][k],yline[numlin-1][k],
				0.0,0.0,0.0,sinal,cosal,wl0,zsum);
	*rmin = fadd(*rmin,v);
	*rmout = fadd(*rmout,v);

    }
}

/*=======================================================================	*/

/*	SECONDARY MOMENT CALCULATION	*/

REAL secmom(REAL y0,REAL z0,
	REAL a,REAL hb,REAL c,REAL hd,
	REAL sinal,REAL cosal,
	REAL wl0,REAL *zsum)
{

/*	Y0 IS HORIZONTAL OFFSET OF INNER END OF CURVE			*/
/*	Z0 IS VERTICAL OFFSET OF INNER END OF CURVE			*/
/*	A,HB,C,HD ARE CURVE FORM PARAMETERS				*/
/*	SINAL, COSAL SPECIFY WATER SLOPE				*/
/*	WL0 IS WATERLINE OFFSET						*/
/*	ZSUM IS DEPTH-WEIGHTED AREA SUM, FOR USE IN FINDING CENTRE OF	*/
/*		BUOYANCY DEPTH						*/

	INT	inner,outer;
	REAL	dwl,t1,t2,s0,s1,s2;
	REAL	zsum0,zsum1,zsum2;
	REAL	secmomv;

/*	EVALUATE WATERLEVEL CLEARANCE OVER INNER END	*/
	dwl = z0*cosal+y0*sinal-wl0;

/*	FIND INTERSECTION POINTS (IF ANY) ON THE CURVED SEGMENT	*/

	inters(a,hb,c,hd,sinal,cosal,dwl,&t1,&t2);

/*	CALCULATE CURVE MOMENTS TO T1, T2 AND OUTER END IF
	THEY WILL BE NEEDED
*/
	outer = (t1 >= 0.0 && t1 <= 1.0);
	if(outer) s1 = curmom(t1,a,hb,c,hd,y0,z0,wl0,sinal,cosal,&zsum1);

	inner = (t2 >= 0.0 && t2 <= 1.0);
	if(inner) s2 = curmom(t2,a,hb,c,hd,y0,z0,wl0,sinal,cosal,&zsum2);

/*	NEED TO FIND TOTAL AREA IF INNER AND OUTER INTERSECTIONS
	BOTH EXIST, OR BOTH DO NOT EXIST, AND WATER COVERS KEEL,
	OR ONLY ONE INTERSECTION EXISTS AND WATER DOES NOT COVER
	KEEL
*/
	if((inner == outer) == (dwl > 0.0))
		s0 = curmom(1.0,a,hb,c,hd,y0,z0,wl0,sinal,cosal,&zsum0);

/*	NOW ADD MOMENT DUE TO TRAPEZOIDAL SECTION OVER THE CURVE,	*/
/*	LESS CONTRIBUTION BELOW CURVE	*/

	if(!outer) {

/*	CASE WHERE OUTER INTERSECTION DOES NOT EXIST:	*/

		if(!inner) {

/*	... AND WHERE INNER INTERSECTION DOES NOT EXIST:	*/

			if(dwl > 0.0) {

/*	HAVE NO INTERSECTION, AND WATER OVER INNER END: WATER COVERS SEGMENT	*/
/*	COMPLETELY:  MOMENT IS THAT FROM INNER END TO INTERSECTION VALUE 1.0	*/

				secmomv = s0;
/*				*zsum += zsum0;			*/
				*zsum = fadd(*zsum,zsum0);
			} else {

/*	NO INTERSECTION, AND WATER BELOW INNER END: WATER ENTIRELY BELOW	*/
/*		SEGMENT, NO MOMENT CONTRIBUTION	*/

				secmomv = 0.0;
			}
		} else {

/*	... CASE WHERE INNER INTERSECTION EXISTS:	*/

			if(dwl >= 0.0) {

/*	WATER OVER INNER END:  CONCAVE CURVE, WATER EXTENDS TO INNER	*/
/*		INTERSECTION	*/
				secmomv = s2;

/*				*zsum += zsum2;			*/
				*zsum = fadd(*zsum,zsum2);
			} else {

/*	WATER BELOW INNER END: WATER EXTENDS BETWEEN INNER INTERSECTION	*/
/*		AND OUTER END						*/
/*				secmomv = s0-s2;			*/
/*				(*zsum) = (*zsum)+zsum0-zsum2;		*/

				secmomv = fsub(s0,s2);
				*zsum = fadd(*zsum,fsub(zsum0,zsum2));
			}
		}
	} else {

/*	CASES WHERE OUTER INTERSECTION DOES EXIST:	*/

		if(inner) {

/*	CASE WHERE INNER INTERSECTION EXISTS:	*/
/*			secmomv = s1-s2;	*/

			secmomv = fsub(s1,s2);
			if(dwl > 0.0) {

/*	WATER OVER INNER END:  WATER EXTENDS FROM INNER END TO INNER	*/
/*		INTERSECTION, THEN FROM OUTER INTERSECTION TO OUTER END	*/
/*				secmomv = s0-secmomv;			*/
/*				*zsum += zsum0-zsum1+zsum2;		*/

				secmomv = fsub(s0,secmomv);
				*zsum = fadd(*zsum,
					fadd(fsub(zsum0,zsum1),zsum2));
			} else {

/*	WATER BELOW INNER END:  WATER COVERS SEGMENT FROM INNER		*/
/*		INTERSECTION TO OUTER INTERSECTION ONLY			*/
/*				*zsum += zsum1-zsum2;			*/

				*zsum = fadd(*zsum,fsub(zsum1,zsum2));
			}
		} else {

/*	OUTER INTERSECTION EXISTS, BUT INNER INTERSECTION DOES NOT:	*/

			if(dwl > 0.0) {

/*	IF WATER IS OVER INNER END, IT MUST COVER SEGMENT OUT TO OUTER	*/
/*		INTERSECTION						*/
/*				*zsum += zsum1;				*/

				secmomv = s1;
				*zsum = fadd(*zsum,zsum1);
			} else {

/*	OTHERWISE, WATER MUST COVER SEGMENT FROM OUTER INTERSECTION TO	*/
/*		THE END OF THE SEGMENT					*/
/*				secmomv = s0-s1;			*/
/*				*zsum += zsum0-zsum1;			*/

				secmomv = fsub(s0,s1);
				*zsum = fadd(*zsum,fsub(zsum0,zsum1));
			}
		}
	}
	return(secmomv);
}
/*	T0 IS INTERSECTION PARAMETER TO WHICH MOMENT IS INTEGRATED	*/
/*	A,HB,C,HD ARE CURVE FORM PARAMETERS				*/
/*	Y0,Z0 ARE COORDINATES OF INNER END OF CURVE			*/
/*	WL0 IS WATERLINE OFFSET						*/
/*	SINAL,COSAL SPECIFY WATERLINE SLOPE				*/
/*	ZSUM IS DEPTH-WEIGHTED AREA SUM, FOR USE IN FINDING CENTRE OF	*/
/*		BUOYANCY DEPTH						*/

REAL curmom(	REAL t0,
		REAL a,REAL hb,REAL c,REAL hd,
		REAL y0,REAL z0,
		REAL wl0,
		REAL sinal,REAL cosal,
		REAL *zsum)
{
    REAL	curmomv;
    REAL	b,d,t1,yd,zd,y1,z1;
    REAL	wl00,wl01,y00,y01,sum,area,frac,zval;
    REAL	arundr,yc,zc;

    b = hb+hb;
    d = hd+hd;

    t1 = max(0.0,min(t0,1.0));

/*	YD AND ZD ARE WIDTHS COVERED BY SPECIFIED CURVE		*/

    yd = t1*(a+t1*hb);
    zd = t1*(c+t1*hd);

/*	Y0 AND Z0 ARE COORDINATES OF START OF CURVE.  Y1 AND Z1	*/
/*	ARE THOSE FOR ITS END					*/

    y1 = y0+yd;
    z1 = z0-zd;

/*	WATERLEVEL CLEARANCES ABOVE EACH END			*/

    wl00 = cosal*z0+sinal*y0-wl0;
    wl01 = cosal*z1+sinal*y1-wl0;

/*	FIRSTLY, CALCULATE MOMENT DUE TO TRAPEZOID ABOVE LINE
	FROM INNER TO OUTER END	*/

/*	HORIZONTAL DISTANCES OUT TO EACH END OF CURVE, FROM	*/
/* 	(0,0) REFERENCE						*/

    y00 = cosal*y0-sinal*z0;
    y01 = cosal*y1-sinal*z1;

/*	AREA OF TRAPEZOID					*/
    sum = wl00+wl01;
    area = 0.5*sum*(y01-y00);

/*	"FRAC" IS FRACTION OF WIDTH OF TRAPEZOID OUT TO ITS CENTROID, WHICH	*/
/*	CAN VARY FROM 1/3 TO 2/3	*/

    if(sum != 0.0) {
	frac = 0.333333*(sum+wl01)/sum;
    } else {
	frac = 0.0;
    }

    curmomv = area*(y00+(y01-y00)*frac);

    *zsum = area*((z0-0.5*wl00*cosal)*(1.0-frac)+
		  (z1-0.5*wl01*cosal)*frac);

/*	NOW FIND MOMENT DUE TO TRIANGLE BELOW BASE OF TRAPEZOID,	*/
/*	ALIGNED WITH THE HULL COORDINATES				*/
    area = 0.5*yd*zd;
    zval = z0-0.33333*zd;
    curmomv += area * ((y0 + 0.66667*yd)*cosal-zval*sinal);
    *zsum += area*zval;

/*	NOW REMOVE MOMENT DUE TO AREA UNDER BOTTOM CURVE		*/
/*	CALCULATE AREA UNDER THE CURVE, OUT TO POINT "T1"		*/

    arundr = t1*t1*(a*c/2.0+t1*((a*hd+b*c)/3.0+t1*b*hd/4.0));

    if(fabs(arundr) > 1.0e-30f) {

/*	WE FIND THE Y-CENTROID THROUGH DIVIDING THE MOMENT FUNCTION	*/
/*	"YF" BY THIS AREA						*/
	yc = y0+(t1*t1*t1*(a*c*a/3.0+t1*(a*(hd*a+c*(hb+b))/4.0+
	t1*((a*hd*(hb+b)+c*hb*b)/5.0+t1*hb*hd*b/6.0))))/arundr;

/*	WE FIND THE Z-CENTROID THROUGH DIVIDING THE MOMENT DUE TO THE	*/
/*	RECTANGLE ABOUT THE CURVE, LESS THAT DUE TO AREA OVER ("ZF"),	*/
/*	BY THE AREA UNDER						*/
	zc = (t1*t1*t1*(c*a*c/3.0+t1*(c*(hb*c+a*(hd+d))/4.0+t1*
	((c*hb*(hd+d)+a*hd*d)/5.0+t1*hd*hb*d/6.0))));

	zc = z0-(0.5*yd*zd*zd-zc)/arundr;
    } else {
	yc = y0;
	zc = z0;
    }

/*	THUS WE FINALLY GET THAT DUE TO THE AREA UNDER, WHICH WE	*/
/*	SUBTRACT FROM THE CALCULATED TOTAL				*/
    curmomv -= ( (arundr) * ((yc)*cosal-(zc)*sinal) );
    *zsum -= arundr*zc;

    return(curmomv);
}

