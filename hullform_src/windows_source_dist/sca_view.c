/* Hullform component - sca_view.c
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
 
/*	determine scale for perspective plot				*/

#include "hulldesi.h"

void scale_view(REAL rotn,REAL pp,REAL yview,REAL zview,INT *inisec,INT *lassec)
{
	int			i,j;
	extern REAL	x1,x2,yy1,y2,heelv;
	REAL		zmin,zmax,zref;
	REAL		xs,ys;
	REAL		cosr = cosd(rotn);
	extern REAL	xgslope,ygslope,xgorigin,ygorigin;
	extern REAL PixelRatio;
	extern int	xmaxi,ymaxi;
	REAL		ar;
	extern int	persp;

	/*	Skip routine if no hull defined		*/
	if(count < 2) return;

	/*	Find maximum possible range of displayable sections	*/
	*inisec = -1;
	for(i = surfacemode ; i < count ; i++) {
		if(zview+xsect[i]*cosr > 0.0) {
			if(*inisec < 0) *inisec = i;
			*lassec = i;
		}
	}

	/*	Find extremes of y and z for full length of the hull	*/
	zmin = 1.0e+16;
	zmax = -zmin;
	ys = 0.0;
	for(j = 0 ; j < numlin ; j++) {
		for(i = max(1,stsec[j]) ; i <= ensec[j] ; i++) {
			if(ys < yline[j][i]) ys = yline[j][i];
			zref = zline[j][i];
			if(zmin > zref) zmin = zref;
			if(zmax < zref) zmax = zref;
		}
	}
	x1 = yy1 = 1.0e+30;
	x2 = y2 = -1.0e+30;

	/*	Set unit transformation for the window (noting that this routine
	is used to define these values later)
	*/
	xgorigin = ygorigin = 0.0;
	xgslope  = ygslope = 1.0;
	heelv = heel + 180.0;
	perspp(0.0,0.0,0.0,pitch,rotn,heelv);

#ifdef EXT_OR_PROF
	if(surfacemode)
		xs = xsect[1];
	else
#endif
		xs = xsect[0]-dxstem();
	vtran(xs,-ys,zmin);
	vtran(xs, ys,zmin);
	vtran(xs,-ys,zmax);
	vtran(xs, ys,zmax);

	/*	STERN X-POSITION	*/

	xs = xsect[count-1];
	vtran(xs,-ys,zmin);
	vtran(xs, ys,zmin);
	vtran(xs,-ys,zmax);
	vtran(xs, ys,zmax);

	/*	Aspect ratio - ratio of picture width to height	*/
	ar = (REAL) xmaxi / ((REAL) ymaxi * PixelRatio);
	xs = 0.5*((x2 - x1) - (y2 - yy1)*ar);
	if(xs > 0.0 ){
		xs /= ar;
		y2 += xs;
		yy1 -= xs;
	}
	else {
		x2 -= xs;
		x1 += xs;
	}
}

void vtran(REAL x0,REAL y0,REAL z0)
{
	extern REAL	x1,x2,yy1,y2,zpersp;

	zpersp = -x0;
	trans(&y0,&z0);
	if(y0 > x2) x2 = y0;
	if(y0 < x1) x1 = y0;
	if(z0 < yy1) yy1 = z0;
	if(z0 > y2) y2 = z0;
}

