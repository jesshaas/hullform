/* Hullform component - cleara.c
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
 
/*      RETURN WATER-LEVEL CLEARANCE, DISTANCE OUT TO LOWEST POINT (MEASURED    */
/*	IN SENSE TOWARDS PORT GUNWHALE) AND INDEX OF HULL LINE NEXT PAST, OR	*/
/*	AT, LOWEST POINT (STARTING AT PORT GUNWHALE)	*/

/*	NOTE THAT THE RETURNED VALUE WOULD NEED TO BE REDUCED BY	*/
/*	1/COSA, IF IT MATTERED (WHICH IT DOES NOT IN THE CURRENT	*/
/*	IMPLEMENTATION)	*/

/*	K	INDEX IN SECTION TABLE	*/
/*	WL0	WATERLINE OFFSET	*/
/*	YLOW	RETURNED Y-VALUE WHERE LOWEST CLEARANCE WAS MET	*/
/*	ZLOW	RETURNED Z-VALUE WHERE LOWEST CLEARANCE WAS MET	*/
/*	LINNUM	INDEX OF FIRST HULL LINE INBOARD OF OR AT LOWEST POINT	*/
/*	LASNUM	INDEX OF LAST HULL LINE WHICH MATCHES CLEARANCE AT LINNUM	*/

#include "hulldesi.h"

REAL cleara(INT k,REAL wl0,REAL *ylow,REAL *zlow,INT *lastlow,INT *firstlow)
{
	REAL	a[maxlin],hb[maxlin],c[maxlin],hd[maxlin];
	REAL	zmax,yl,zl,aa,cc;
	REAL	cref,yref,zref;
	INT	ll,ln,i;
	int	k0 = (k >= maxsec-1) ? 0 : k;

	/*	INITIALISE PARAMETERS BEING EVALUATED: WATERLINE-RELATIVE OFFSET,	*/
	/*	Y-POSITION AT LOWEST POINT, LINE INDICATOR FOR LOWEST POINT, LINE	*/
	/*	INDICATOR FOR LAST COINCIDENT POINT	*/

	zmax = zline[0][k]*cosa + yline[0][k]*sina;
	yl = yline[0][k];
	zl = zline[0][k];
	ll = 0;
	ln = 0;

	/*	SET UP HULL CURVE COEFFICIENTS	*/

	aa = 0.0;
	cc = 1.0;
	for(i = 1 ; i < numlin ; i++) {
		if(stsec[i] <= k0 && ensec[i] >= k0 || k0 >= count) {
			hullpa(k,i,aa,cc,&a[i],&hb[i],&c[i],&hd[i]);
			tranpa(a[i],hb[i],c[i],hd[i],&aa,&cc);
		}
	}

	/*	FIND WATER-SLOPE-BIASED OFFSETS OF EACH OTHER FIXED POINT ON THE HULL,	*/
	/*	AND THE LOWEST POINT ON THE CURVE SEGMENT ABOVE.  THEN USE THE LOWER	*/

	for(i = 1 ; i < numlin ; i++) {
		if(stsec[i] <= k0 && ensec[i] >= k0 || k0 >= count) {

			/*	SET UP INITIAL TRIAL VALUES USING BOTTOM POINT ON THIS SIDE'S CURVE	*/

			cref = curoff(a[i],hb[i],c[i],hd[i],yline[i][k],zline[i][k],
				&yref,&zref);

			if(fge(cref,zmax)) {
				ll = i+i-1;
				if(fgt(cref,zmax))ln = ll;
				zmax = cref;
				yl = yref;
				zl = zref;
			}

			/*	FIND WATERLINE-RELATIVE OFFSET OF HULL LINE	*/

			cref = zline[i][k]*cosa + yline[i][k]*sina;

			if(fge(cref,zmax)) {
				ll = i+i;
				if(fgt(cref,zmax))ln = ll;
				zmax = cref;
				yl = yline[i][k];
				zl = zline[i][k];
			}
		}
	}

	for(i = numlin - 1 ; i >= 0 ; i--) {
		if(stsec[i] <= k0 && ensec[i] >= k0 || k0 >= count) {

			/*	FIND WATERLINE-RELATIVE OFFSET OF HULL LINE	*/

			cref = fsub(fmul(zline[i][k],cosa),fmul(yline[i][k],sina));

			if(fge(cref,zmax)) {
				ll = (((numlin << 1)-i) << 1) - 2;
				if(fgt(cref,zmax))ln = ll;
				zmax = cref;
				yl = fmin(yline[i][k]);
				zl = zline[i][k];
			}

			/*	FIND BOTTOM POINT ON THIS SIDE'S CURVE	*/

			if(i > 0) {
				cref=curoff(fmin(a[i]),fmin(hb[i]),c[i],hd[i],
					fmin(yline[i][k]),zline[i][k],&yref,&zref);

				if(fge(cref,zmax)) {
					ll = (((numlin << 1)-i) << 1)-1;
					if(fgt(cref,zmax))ln = ll;
					zmax = cref;
					yl = yref;
					zl = zref;
				}
			}
		}
	}

	/*	THESE ARE THE FUNCTION RESULTS: CLEARANCE, WHERE IT OCCURRED, NEXT	*/
	/*	INDEX, FINAL INDEX MATCHING CLEARANCE	*/

	*ylow = yl;
	*zlow = zl;
	*firstlow = ln;
	*lastlow = ll;

	aa = fsub(wl0,zmax);
	return(aa);
}

/*	GET MAXIMUM OFFSET OF HULL CURVE, RELATIVE TO WATERLINE	*/

/*	ARGUMENTS A,HB,C,HD ARE QUADRATIC CURVE PARAMETERS	*/
/*	YK,ZK ARE THE START OFFSETS OF THE CURVE	*/

/*	ROUTINE ALSO RETURNS (YL,ZL), THE COORDINATES OF THE MAXIMUM-OFFSET	*/
/*		POINT	*/

REAL curoff(REAL a,REAL hb,REAL c,REAL hd,REAL yk,REAL zk,REAL *yl,REAL *zl)
{
	REAL	t,div;
	REAL	curoffv;
	REAL	y1,z1;

	div = 2.0 * (cosa*hd - sina*hb);
	if(div != 0.0) {
		t = (sina*a - cosa*c) / div;
		if(t >= 0.0 && t <= 1.0f) {
			y1 = t * (a + t * hb);
			z1 = t * (c + t * hd);
			if(y1*sina - z1*cosa > 0.0) {
				*yl = yk + y1;
				*zl = zk - z1;
				curoffv = cosa*(*zl) + sina*(*yl);
				return(curoffv);
			}
		}
	}
	*yl   = 1.0e+16;
	*zl   = 1.0e+16;
	curoffv = -1.0e+16;
	return(curoffv);
}

