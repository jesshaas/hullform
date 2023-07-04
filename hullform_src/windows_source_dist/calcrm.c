/* Hullform component - calcrm.c
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

/*	CALCULATE RIGHTING MOMENT PER UNIT LENGTH FOR A SINGLE SECTION	*/

REAL calcrm(INT i,int j1,int j2,int left,int right,REAL wloff,REAL *rm1,
	REAL *rm2,REAL *cl1,REAL *cl2,
	REAL *zsum1,REAL *zsum2,REAL *zsumav)
{
	REAL	dz2,zsuma,zsumb,drm,fac,ylow,zlow;
	INT	nl,il;
	REAL	rminl,rmoutl,rminr,rmoutr;

	*rm1 = *rm2;
	*zsum1 = *zsum2;
	*cl1 = *cl2;

	dz2 = wloff - (beta*xsect[i] + hwl[i]);
	if(right) {
		rightm(i,j1,j2,sina,cosa,dz2,&zsuma,&rminr,&rmoutr);
	}
	else {
		zsuma = 0.0;
		rminr = 0.0;
		rmoutr = 0.0;
	}

	if(left) {
		rightm(i,j1,j2,-sina,cosa,dz2,&zsumb,&rminl,&rmoutl);
	}
	else {
		zsumb = 0.0;
		rminl = 0.0;
		rmoutl = 0.0;
	}

	*rm2 =   rminr - rminl;
	*zsum2 = zsuma + zsumb;	// ???

	*cl2 = cleara(i,dz2,&ylow,&zlow,&nl,&il);

	drm =     (*rm1 + *rm2  )*0.5;
	*zsumav = (*zsum1 + *zsum2)*0.5;

	/*	CORRECT FOR LENGTH REDUCTION WHERE WATERLINE INTERSECTS KEEL	*/

	if(*cl1 > 0.0 && *cl2 < 0.0) {
		fac = *cl2 / (*cl2 - *cl1);
		drm *= fac;
		*zsumav *= fac;
	}
	else if(*cl1 < 0.0 && *cl2 > 0.0) {
		fac = *cl1 / (*cl1 - *cl2);
		drm *= fac;
		*zsumav *= fac;
	}

	*rm2 = rmoutr - rmoutl;

	return(drm);
}
