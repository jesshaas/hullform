/* Hullform component - findrm.c
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

void findrm()
{
	REAL rm1,rm2,dz2,zsum1,zsum2,cl1,cl2,ylow,zlow;
	INT	 nl,il,i,j,line;
	REAL zsum,zsumav,delx,drm,x1;
	REAL rm;
#ifndef STUDENT
	REAL tankzsum,watlev,tankmoment;
	REAL tankms;
	int	 j1,j2;
#endif
	int ensave[maxlin];
	int iend;

	/*	CALCULATE RIGHTING MOMENT BY TRAPEZOIDAL INTEGRATION ALONG HULL.*/

	/*	NOTE THAT, SINCE RIGHTING MOMENT SCALES WITH BEAM**3, IT IS CRITICALLY	*/
	/*	DEPENDENT ON THE BROAD SECTIONS.  THIS MEANS THAT WE MUST BIAS THE	*/
	/*	INTEGRATION ASSUMING LINEAR LENGTH SCALE VARIATION BETWEEN STATIONS,	*/
	/*	RATHER THAN LINEAR RIGHTING MOMENT VARIATION	*/

	rm = 0.0;
	zsum = 0.0;
	for(j = 0 ; j < numlin ; j++) ensave[j] = ensec[j];

	/*	STEM INTEGRATION:	*/

	rm2 = 0.0;
	dz2 = wl-(beta*xsect[0]+hwl[0]);
	zsum2 = 0.0;
	cl2 = cleara(0,dz2,&ylow,&zlow,&nl,&il);

	line = 0;
	do {
		iend = min(maxsec,count + numlin - line)-1;
		i = iend;
		do {
			xsect[i--] = xsect[0] - yline[line++][0];
		}
		while(i >= count);

		for(j = 0 ; j < numlin ; j++) {
			if(stsec[j] <= 0) {
				refit(j,xsect,yline[j],zline[j],ycont[j],zcont[j],count,iend);
				ensec[j] = iend - j;
			}
		}
		x1 = xsect[0] - yline[0][0];
		for(j = iend ; j >= count ; j--) {
			delx = xsect[j] - x1;
			if(delx > 0.0) {
				drm = calcrm(j,0,numlin,1,1,wl,&rm1,&rm2,&cl1,&cl2,
					&zsum1,&zsum2,&zsumav);
				x1 = xsect[j];
				rm += drm*delx;
				zsum += zsumav*delx;
			}
		}
	}
	while(line < numlin);

	for(j = 0 ; j < numlin ; j++) ensec[j] = ensave[j];

	/*	THERE MAY BE AN EXTRA SEGMENT, IF USER HAS SPECIFIED LAST STEM "Y"	*/
	/*	VALUE TO BE NONZERO	*/

	delx = xsect[1] - xsect[0] + yline[numlin-1][0];

	/*	CALCULATE TERMS ALONG MAIN BODY OF HULL	USING CURRENT WATERLINE "wl" */

	for(i = 1 ; i < count ; i++) {
		drm = calcrm(i,0,numlin,1,1,wl,&rm1,&rm2,&cl1,&cl2,
			&zsum1,&zsum2,&zsumav);
		rm += drm*delx;
		zsum += zsumav*delx;
		if(i < count - 1) delx = xsect[i+1]-xsect[i];
	}
	rm *= densit[numun];

	/*	Calculate floodable tank contributions	*/

#ifndef STUDENT
	tankms = 0.0;
	for(tank = 0 ; tank < ntank ; tank++) {
		j1 = fl_line1[tank];
		j2 = fl_line1[tank+1];
		watlev = fl_fixed[tank] > 0 ? fl_walev[tank] : wl;
		rm2 = 0.0;
		cl2 = 0.0;
		zsum2 = 0.0;
		zsumav = 0.0;
		delx = 0.0;
		tankmoment = 0.0;
		tankzsum = 0.0;
		j = fl_right[tank];
		for(i = stsec[j1] ; i <= ensec[j1] ; i ++) {
			drm = calcrm(i,j1,j2,!j,j,watlev,&rm1,&rm2,&cl1,&cl2,&zsum1,&zsum2,&zsumav);

			/*	Accumulate tank moment (divided by density)	*/
			tankmoment += drm*delx;

			/*	Only include contribution to zcofb sum if the tank is
			leaky - when the contribution is negative		*/
			if(fl_fixed[tank] == 0) zsum += zsumav*delx;
			tankzsum += zsumav*delx;

			if(i < count - 1) delx = xsect[i+1]-xsect[i];
		}
		if(fl_fixed[tank] == 0) {		// leaky
			tankmass[tank] = fl_perm[tank] * fl_volum[tank] * fl_spgra[tank]/spgrav*densit[numun];
			tankmoment *= fl_perm[tank];

		} else {
			if(fl_fixed[tank] == 1) {		// fixed volume, not percent
				tankmass[tank] = fl_fract[tank] * fl_spgra[tank]/spgrav*densit[numun];	// fl_fract is the volume in the tank
			} else if (fl_fixed[tank] == 2) {	// percent volume
				tankmass[tank] = fl_fract[tank] * 0.01 * fl_perm[tank] * fl_volum[tank] * fl_spgra[tank]/spgrav*densit[numun];
			}
			tankmoment *= fl_perm[tank]*fl_spgra[tank]/spgrav;
			tankms += tankmass[tank];
		}

		rm += tankmoment*densit[numun];
		if(fl_volum[tank] != 0.0) {
			if(fl_fixed[tank] == 1) {
				tankvcg[tank] = -tankzsum/fl_fract[tank];
			} else if (fl_fixed[tank] == 2) {
				tankvcg[tank] = -tankzsum/(fl_fract[tank] * 0.01 * fl_volum[tank]);
			} else {
				tankvcg[tank] = -tankzsum/fl_volum[tank];
			}
		} else {
		    tankvcg[tank] = 0.0;
		}
		tankmom[tank] = tankmoment;
	}
#endif

	if(disp != 0.0 && volu != 0.0) {
#ifndef STUDENT
		zcofb = zsum / ((disp+tankms) / densit[numun]);
#else
		zcofb = zsum / (disp / densit[numun]);
#endif
		/* vertical centre of buoyancy */
		rm += disp * zcofm * sina;	/* include mass contribution to stability */
		rm1 = fabs(rm);
		rm2 = 0.0005*disp*bmax;

		/*	If hull is heeled, find metacentre from righting moment	*/

#ifndef STUDENT
		gz = rm / (disp + tankms);
#else
		gz = rm / disp;
#endif
		kn = gz-zcofm*sina;
		if(rm1 > rm2 && sina != 0.0) {
			zmeta = -kn/sina;
		}
		else {
#ifdef STUDENT
			zmeta = zcofb - sarm/disp;
#else
			zmeta = zcofb - sarm/(disp + tankms);
#endif
		}
	}
	else {
		zmeta  = 999.999;
		zcofb  = 999.999;
		gz     = 999.999;
		kn     = 999.999;
	}
}

