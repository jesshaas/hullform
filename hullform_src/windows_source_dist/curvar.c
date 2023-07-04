/* Hullform component - curvar.c
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
 
/*	CALCULATE AREA OVER CURVE FROM POINTS T1 TO T2.			*/

/*	T1 AND T2 ARE CURVE PARAMETERS AT ENDS OF CURVE			*/
/*	DWL INN IS DISTANCE TO WATERLINE OVER INNER ("T2") END OF CURVE	*/
/*	SINAL, COSAL ARE WATER SLOPE PARAMETERS				*/
/*	A,HB,C,HD ARE CURVE COEFFICIENTS				*/
/*	OUT INT IS SET .TRUE. WHEN AN OUTER WATERLINE INTERSECTION IS DETECTED*/
/*		WITHIN THE CURVE					*/
/*	HBEAM, ZWL ARE Y- AND Z-COORDINATES WHERE OUTER INTERSECTION IS FOUND*/
/*	WETWID IS WETTED WIDTH CONTRIBUTED BY THIS HULL SEGMENT		*/
/*	WETW3 IS THE CONTRIBUTION TO THE CUBE OF THE WETTED WIDTH BY WETWID*/
/*	YORIG,ZORIG SPECIFY THE INNER END OF THE CURVE			*/

#include "hulldesi.h"

extern int tankmode;

REAL curvar(REAL t1,REAL t2,REAL dwlinn,REAL sinal,REAL cosal,
	REAL a,REAL hb,REAL c,REAL hd,INT *outint,
	REAL *hbeam,REAL *wetwid,REAL *wetw3,REAL *zwl,
	REAL yorig,REAL zorig)
{

	INT	int_t1,int_t2,req_1;
	REAL	areat1,areat2,area1;
	REAL	curvarea;
	REAL	ylim2,zlim2,ylim2s;
	REAL	ylim1,zlim1,ylim1s;

	/*	EVALUATE THREE INTERSECTION AREAS AS REQUIRED	*/

	int_t1 = t1 > 0.0f && t1 <= 1.0f;
	int_t2 = t2 > 0.0f && t2 <= 1.0f;

	/*	REQUIRE AREA TO 1 IF WATER LIES OVER INNER END AND THERE ARE TWO
	INTERSECTIONS, OR IF IT DOES NOT, AND THERE IS ONE
	*/
	req_1 = (dwlinn >= 0.0) == (int_t1 == int_t2);

	if(int_t2)areat2 = areato(t2,sinal,cosal,dwlinn,a,hb,c,hd);
	if(req_1) area1  = areato(1.,sinal,cosal,dwlinn,a,hb,c,hd);

	/*	IF OUTER INTERSECTION IS OUTSIDE THE ENDS OF THIS CURVE:	*/

	if(!int_t1) {

		/*	    AND INNER INTERSECTION IS OUTSIDE THE ENDS OF THE CURVE TOO	*/

		if(!int_t2) {

			/*		THEN HAVE EITHER COMPLETE IMMERSION, OR COMPLETE	*/
			/*		CLEARANCE, OF THE CURVE	*/

			if(fpos(dwlinn)) {
				curvarea = area1;
			}
			else {
				curvarea = 0.0;
			}
		}
		else {

			/*		IF THERE IS NO OUTER INTERSECTION, BUT AN INNER ONE:	*/

			ylim2 = yorig + t2 * (a + t2*hb);
			zlim2 = zorig - t2 * (c + t2*hd);
			ylim2s = cosal*ylim2 - sinal*zlim2;

			if(fneg(dwlinn)) {

				/*		IF INNER END IS CLEAR, THE OUTER END MUST BE SUBMERGED	*/

				curvarea = area1 - areat2;
				*wetwid -= ylim2s;
				*wetw3 -= ylim2s*ylim2s*ylim2s;
				if(!(*outint) && tankmode) {
					*hbeam = ylim2;
					*zwl  = zlim2;
					*outint = 1;
				}

			}
			else {

				/*		IF INNER END IS SUBMERGED, OUTER END IS CLEAR	*/

				curvarea = areat2;
				*wetwid += ylim2s;
				*wetw3 += ylim2s*ylim2s*ylim2s;
				if(!(*outint) && !tankmode) {
					*hbeam = ylim2;
					*zwl  = zlim2;
					*outint = 1;
				}
			}
		}
	}
	else {

		/*	... IF OUTER INTERSECTION IS WITHIN THE ENDS OF THE CURVE	*/

		areat1 = areato(t1,sinal,cosal,dwlinn,a,hb,c,hd);

		ylim1 = yorig + t1 * (a + t1 * hb);
		zlim1 = zorig - t1 * (c + t1 * hd);
		ylim1s = cosal * ylim1 - sinal * zlim1;
		if(int_t2) {

			/*	INNER AND OUTER INTERSECTION BOTH WITHIN ENDS	*/

			curvarea = areat1 - areat2;
			ylim2 = yorig + t2 * (a + t2 * hb);
			zlim2 = zorig - t2 * (c = t2 * hd);
			ylim2s = cosal * ylim2 - sinal * zlim2;
			if(fpos(dwlinn)) {

				/*	INNER AND OUTER ENDS OF SEGMENT IMMERSED	*/

				curvarea = area1 - curvarea;
				*wetwid -= ylim1s - ylim2s;
				*wetw3  -= ylim1s*ylim1s*ylim1s - ylim2s*ylim2s*ylim2s;
			}
			else {

				/*	Inner and outer ends of segment clear of water	*/

				if(!(*outint)) {
					if(tankmode) {
						*hbeam = ylim2;
						*zwl  = zlim2;
					}
					else {
						*hbeam = ylim1;
						*zwl  = zlim1;
					}
					*outint = 1;
				}
				*wetwid += ylim1s - ylim2s;
				*wetw3 += ylim1s*ylim1s*ylim1s - ylim2s*ylim2s*ylim2s;
			}
		}
		else {

			/*	OUTER INTERSECTION WITHIN, BUT INNER INTERSECTION OUTSIDE ENDS	*/

			if(fneg(dwlinn)) {

				/*	INNER END CLEAR OF WATER, OUTER END IMMERSED	*/

				curvarea = area1 - areat1;
				*wetwid -= ylim1s;
				*wetw3  -= ylim1s*ylim1s*ylim1s;
				if(!(*outint) && tankmode) {
					*hbeam = ylim1;
					*zwl  = zlim1;
					*outint = 1;
				}
			}
			else {

				/*	INNER END IMMERSED	*/

				curvarea = areat1;
				*wetwid += ylim1s;
				*wetw3  += ylim1s*ylim1s*ylim1s;
				if(!(*outint) && !tankmode) {
					*hbeam = ylim1;
					*zwl  = zlim1;
					*outint = 1;
				}
			}
		}
	}
	return(curvarea);
}

/*	CALCULATE AREA OVER CURVE, OUT TO PARAMETER VALUE T.	*/

REAL areato(REAL t,REAL sinal,REAL cosal,REAL dwl,
	REAL a,REAL hb,REAL c,REAL hd)
{
	REAL	dy,dz,width,extra,area1,area2,area3,t1,t2,t3;

	dy = fmul(t,fadd(a,fmul(t,hb)));
	dz = fmul(t,fadd(c,fmul(t,hd)));

	/*	AREA IN TRAPEZIUM ABOVE AND CLEAR OF CURVE	*/

	width = fadd(fmul(sinal,dz),fmul(cosal,dy));
	extra = fsub(fmul(cosal,dz),fmul(sinal,dy));
	area1 = fmul(fsub(dwl,fmul(0.5,extra)),width);

	/*	AREA IN TRIANGLE ABOUT CURVE	*/

	area2 = fmul(fmul(0.5,dy),dz);

	/*	AREA BELOW CURVE	*/

	/*	area3 = t*t*( a*c              /2.0+
	t*  ((a*hd + hb*c *2.0)/3.0+
	t*           hb*hd*2.0 /4.0));	*/

	t1 = fmul(fmul(a,c),0.5);
	t2 = fmul(0.333333,fadd( fmul(a,hd) , fmul(fmul(hb,c),2.0) ));
	t3 = fmul(fmul(hb,hd),0.5);

	area3 = fmul( fmul(t,t) , fadd(t1, fmul(t,fadd(t2,fmul(t,t3))) ));

	/*	HENCE TOTAL	*/

	t1 = fsub(fadd(area1,area2),area3);
	return(t1);
}
