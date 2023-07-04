/* Hullform component - hullar.c
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
 
/*	CALCULATE HULL HALF-SECTION IMMERSED AREA.    */

#include "hulldesi.h"

void hullar(INT k,int j1,int j2,REAL *areain,REAL *areaout,
	REAL wl1,REAL sinal,REAL cosal,REAL *hbeam,REAL *wetwid,
	REAL *wetw3,REAL *zwl,INT *line)
{

	/*	K IS SECTION INDEX NUMBER				*/
	/*	j1 and j2 are line index ranges (j1 <= j < j2)		*/
	/*	Normally, j1 = 0 & j2 = numlin, or j1 is the start of	*/
	/*	one tank, j2 the start of the next.			*/
	/*	areain is area excluding lines starting at this section	*/
	/*	areaout is area excluding lines ending at this section	*/
	/*	WL1 IS WATERLEVEL OFFSET				*/
	/*	SINAL,COSAL SPECIFY THE WATERLEVEL SLOPE        	*/
	/*	HBEAM IS THE RETURNED HALF-BEAM				*/
	/*	WETWID IS THE ACCUMULATED WETTED WIDTH, USED IN		*/
	/*	    WATERPLANE CALCULATIONS				*/
	/*	WETW3 IS THE CUBED WETTED WIDTH, USED IN TRANSVERSE	*/
	/*	STABILITY ESTIMATES					*/
	/*	ZWL IS VERTICAL COMPONENT OF WATERLINE INTERSECTION	*/
	/*	LINE IS THE HULL LINE WHERE THE WATERLINE INTERSECTION	*/
	/*	    WAS IDENTIFIED					*/

	INT	    j;
	REAL    dwlinn,dwlout;
	REAL    aa,cc,t1,t2;
	REAL    dummwi,dummw3;
	REAL    a[maxlin],hb[maxlin],c[maxlin],hd[maxlin];
	INT	    outint,preint;
	REAL    area;
	REAL    wl0;
	REAL    ar,hbr,cr,hdr;
	REAL    y0,y1,z0,z1;
	int     k0;			/* k, or zero if k >= maxsec-1 */
	int     lastline = -1;
	extern int tankmode;

	k0 = (k >= maxsec-1) ? 0 : k;

	/*	HALF-BEAM DEFAULTS TO ZERO    */

	*hbeam = 0.0;
	*zwl = wl1;

	/*	WETTED WIDTH AND ITS CUBE INITIALISED AT ZERO    */

	*wetwid = 0.0;
	*wetw3 = 0.0;

	/*	HULL LINE NEXT PAST WHERE WATERLINE INTERSECTION IS FOUND,	*/
	/*	INITIALLY UNDEFINED						*/

	*line = -2;        /* signifies no intersection    */

	/*	"OUTER INTERSECTION SEEN" FLAG    */

	outint = 0;

	if(j1 == 0) {
		y0 = 0.0;
		z0 = zline[0][k];
		y1 = yline[0][k];
		z1 = z0;
	}
	else {
		y0 = yline[j2-1][k];
		z0 = zline[j2-1][k];
		y1 = yline[j1][k];
		z1 = zline[j1][k];
	}

	/*	SUBTRACT AREA OVER DECK OR OVER tank closure	*/

	dwlinn = cosal*z0 + sinal*y0 - wl1;
	dwlout = cosal*z1 + sinal*y1 - wl1;

	/*	If either is immersed, find area over deck        */

	if(fpos(dwlinn) || fpos(dwlout)) {
		inters(	y1-y0,0.0,z0-z1,0.0,sinal,cosal,dwlinn,&t1,&t2);
		preint = outint;
		*areain = -curvar(t1,t2,dwlinn,sinal,cosal,
			y1-y0,0.0,z0-z1,0.0,
			&outint,hbeam,wetwid,wetw3,zwl,
			y0,z0);
		*areaout = *areain;

		*wetwid = fmin(*wetwid);
		*wetw3 = fmin(*wetw3);

		/*	ONLY ACCEPT AN OUTER INTERSECTION IF WATER SLOPES UP TO INTERSECTION    */
		/*	POINT	        					*/

		outint = outint && fpos(sinal);

		if(!preint && outint) {
			if(fnez(t1)) {

				/*	    ... INTERSECTION ON GUNWHALE    */

				*line = 0;
			}
			else {

				/*	    ... INTERSECTION ON DECK    */

				*line = -1;
			}
		}
	}
	else {
		*areain = 0.0;
		*areaout = 0.0;
		t1 = -1.0e16;
		t2 = 1.0e16;
	}

	/*	INITIALISE SKIN ALIGNMENT    */

	aa = 0.0;
	cc = 1.0;
	wl0 = wl1;    /* set waterline to that given        */

	for(j = j1+1 ; j < j2 ; j++) {

		if(stsec[j] > k0 || ensec[j] < k0) continue;

		/*	GET SEGMENT CURVE FORM PARAMETERS    */

		hullpa(k,j,aa,cc,&a[j],&hb[j],&c[j],&hd[j]);

		/*	EVALUATE DISTANCES TO WATERLINE ABOVE OUTER AND INNER ENDS OF SEGMENT    */

		dwlinn = cosal*zline[j][k] + sinal*yline[j][k] - wl0;

		/*	FIND INTERSECTION POINTS WITH THIS CURVE    */

		inters(a[j],hb[j],c[j],hd[j],sinal,cosal,dwlinn,&t1,&t2);

		/*	THUS FIND TOTAL AREA OVER, AND HALF-BEAM IF OUTER INTERSECTION FOUND    */

		preint = outint;
		area = curvar(t1,t2,dwlinn,sinal,cosal,
			a[j],hb[j],c[j],hd[j],
			&outint,hbeam,wetwid,wetw3,zwl,
			yline[j][k],zline[j][k]);

		if(stsec[j] < k0) *areain += area;

		if(ensec[j] > k0) *areaout += area;

		/*	RECORD THE HULL LINE WHERE THE INTERSECTION IS SEEN    */

		/*	NOTE "LINE" IS TWICE THE HULL LINE INDEX, OR ONE LESS    */
		/*	WHEN THE INTERSECTION IS ON THE SURFACE ABOVE THE LINE    */

		if(!preint && outint) {
			*line = j << 1;
			if(t1 > 0.0 && t1 < 1.0) (*line)--;
		}

		/*	SET UP LOWER SLOPE PARAMETERS FOR USE IN CALCULATION FOR NEXT HULL    */
		/*	SEGMENT    */

		tranpa(a[j],hb[j],c[j],hd[j],&aa,&cc);
		if(stsec[j] <= k0 && ensec[j] >= k0) lastline = j;
	}

	/*	ADD AREA OVER BOTTOM FLAT    */

	if(lastline >= 0 && yline[lastline][k] != 0.0 && !tankmode) {
		dwlinn = cosal*zline[lastline][k] - wl0;
		inters(yline[lastline][k],0.0,0.0,0.0,sinal,cosal,dwlinn,&t1,&t2);
		preint = outint;
		area = curvar(t1,t2,dwlinn,sinal,cosal,
			yline[lastline][k],0.0,0.0,0.0,
			&outint,hbeam,wetwid,wetw3,zwl,
			0.0,zline[lastline][k]);

		if(stsec[lastline] < k0) *areain += area;

		if(ensec[lastline] > k0) *areaout += area;

		if(!preint && outint) {
			*line = numlin << 1;
		}
	}

	/*	NOW CARRY AROUND BOTTOM UNTIL OUTER INTERSECTION SEEN, IN CASES    */
	/*	WHERE THE WATERLINE MAY CUT THE LOWER HULL SURFACE ON THE OTHER SIDE    */

	if(j2 <= numlin && !outint && fneg(sinal)) {

		/*	USE DUMMY WIDTH ACCUMULATORS ON OTHER SIDE    */

		dummwi = 0.0;
		dummw3 = 0.0;

		if(fnoz(yline[numlin-1][k])) {
			dwlout = cosal*zline[numlin-1][k] - sinal*yline[numlin-1][k] - wl0;
			inters(yline[numlin-1][k],0.0,0.0,0.0,sinal,cosal,
				dwlout,&t1,&t2);
			preint = outint;
			(void) curvar(t1,t2,dwlout,sinal,cosal,
				yline[numlin-1][k],0.0,0.0,0.0,
				&outint,hbeam,&dummwi,&dummw3,zwl,
				fmin(yline[numlin-1][k]),zline[numlin-1][k]);

			if(!preint && outint) {
				*line = numlin << 1;
			}
		}

		for(j = j2 - 1 ; j > j1 ; j--) {
			if(stsec[j] > k0 || ensec[j] < k0) continue;

			dwlout = cosal*zline[j-1][k] - sinal*yline[j-1][k] - wl0;

			/*	FIND INTERSECTION POINTS WITH THIS CURVE    */

			ar  = a[j] + 2.0 * hb[j];
			hbr = -hb[j];
			cr  = -(c[j] + 2.0 * hd[j]);
			hdr = hd[j];
			inters(ar,hbr,cr,hdr,sinal,cosal,dwlout,&t1,&t2);

			/*	THUS FIND TOTAL AREA OVER, AND HALF-BEAM IF OUTER INTERSECTION FOUND    */

			preint = outint;
			(void) curvar(t1,t2,dwlout,sinal,cosal,ar,hbr,cr,hdr,
				&outint,hbeam,&dummwi,&dummw3,zwl,-yline[j-1][k],zline[j-1][k]);

			if((!preint) && outint) {
				*line = (numlin+numlin-j) << 1;
				if(t1 < 1.0) (*line)--;
				break;
			}
		}
	}
}

