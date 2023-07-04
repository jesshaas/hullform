/* Hullform component - balanc.c
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
 
/*	Balance the hull for given heel, centre of mass and displacement */

#include "hulldesi.h"
void clrline(int);
extern int balanced;
extern int persp;
void SwapBuf(void);

void balanc(REAL *calcdisp,int show)
{
	INT	 ni,i,j;
	REAL oldwl;		/* old waterline offset			*/
	REAL pitmom;	/* pitching moment			*/
	REAL chtrim;	/* change of trim			*/
	REAL dzp;		/* change of waterline due to pitch	*/
	REAL dz;		/* change of waterline due to displacement */
	REAL limit;		/* upper limitfor acceptable accuracy	*/
	REAL wlmax,wlmin,ymax;
	REAL dzstem,dzstrn;	/* change of waterline at stem and stern */
	REAL length;

	if(disp <= 0.0) {
		message("Design has no displacement.\nYou must set the displacement and LCG\nbefore using this menu option");
		*calcdisp = -1.0;
		return;
	}

	waitcursor();

	oldwl = wl;
	length = xsect[count-1] - xsect[0];
	limit = 1.0e-4 * (xsect[count-1] - xsect[0]);

	/*	BALANCE FOR GIVEN DISPLACEMENT	*/

	/*	THE SCHEME BELOW IS ESSENTIALLY NEWTON-RAPHSON ITERATION, ALTHOUGH	*/
	/*	IT LOOKS UNUSUALLY OBVIOUS IN THIS APPLICATION	*/

	/*	UP TO 10 ITERATIONS, BUT RARELY ARE MORE THAN 3 OR 4 NEEDED:	*/

	if(!persp) {
		(*init)();
		clrline(2);
		clrline(1);
		clrline(0);
		pstr("\n\nBow and stern waterline offset changes:");
		if(!show) pstr("\n");
	}

	for(ni = 0 ; ni < 18 ; ni++) {

		/*	CALCULATE TOTAL HULL DISPLACEMENT, FROM BOTH SIDES OF HULL	*/

		huldis(calcdisp);
reset:
		if(wplane <= 0.0 || *calcdisp <= 0.0 || mct <= 0.0) {

			wlmax = -1.0e+16;
			wlmin = 1.0e+16;
			ymax = 0.0;
			for(j = 0 ; j < numlin ; j++) {
				for(i = stsec[j] ; i <= ensec[j] ; i++) {
					if(zline[j][i] > wlmax) wlmax = zline[j][i];
					if(zline[j][i] < wlmin) wlmin = zline[j][i];
					if(yline[j][i] > ymax) ymax = yline[j][i];
				}
			}
			loa = xsect[count-1] - xsect[0] + dxstem();
			wl = 0.5*(wlmax+wlmin);
			pitch = 0.0;
			beta = 0.0;
			dzstem = 99.99;
			dzstrn = 99.99;

		}
		else {

			/*	BALANCE THE HULL AGAINST PITCHING MOMENTS.  IF XCOFM IS AHEAD OF	*/
			/*	(LESS THAN) XCOFB, PITCH THE BOW DOWN	*/

			/*			PITCHING MOMENT (VOLUME*LENGTH UNITS)	*/

			pitmom = (xcofm - xlcf)*disp - (xcofb - xlcf)* (*calcdisp);

			/*	Add tank moments to find the total pitching moment, subtract
			tank masses to ensure that the difference between calcdisp and
			the lightship displacement is the correct buoyant disequilibrium.	*/

#ifndef STUDENT
			for(j = 0 ; j < ntank ; j++) {
				pitmom += tankmass[j] * (tanklcg[j] - xlcf);
				*calcdisp -= tankmass[j];	/* (rather than add to disp, which should be a constant) */
			}
#endif

			/*			CHANGE OF TRIM (RADIAN, MCT IN VOLUME*LENGTH UNITS	*/
			/*							PER RADIAN)	*/
			chtrim = pitmom / mct;
			if(ni > 7) chtrim *= 0.5;
			beta = beta + chtrim;
			pitch = atan(beta) * 57.293;

			/*	LCF IS NOT AT X=0, SO MUST CHANGE VERTICAL DISPLACEMENT.	*/
			/*	IF LCF IS ASTERN, AND PITCH IS UP, PITCH INCREASES WL,		*/
			/*	SO ADD THIS TO TOTAL CHANGE					*/

			dzp = xlcf * chtrim;

			/*	IF THE DISPLACEMENT FOR THE CURRENT WATERLINE IS TOO GREAT, ADJUST BY	*/
			/*	THE RATIO OF THE EXCESS VOLUME TO THE WATERPLANE AREA	*/

			dz = ((*calcdisp - disp) / densit[numun]) / wplane;
			if(ni > 7) dz *= 0.5;
			wl = wl + dz + dzp;

			/*	SHOW STEM AND STERN CHANGE, FOR CONFIRMATION OF CONVERGENCE	*/

			dzstem = dz + (xlcf - xsect[0        ]) * chtrim;
			dzstrn = dz + (xlcf - xsect[count - 1]) * chtrim;
			if(fabs(dzstem) > length || fabs(dzstrn) > length) {
				wplane = 0.0;
				goto reset;
			}
		}

		if(!persp) {
			if((ni % 3) == 0) pstrx(lx,show ? "\n:" : "");
			prea("%8.3f",dzstem);
			prea("%8.3f :",dzstrn);
		}

		if(fabs(dzstem) < limit && fabs(dzstrn) < limit) {
			balanced = TRUE;
			break;
		}

		if(fabs(beta) > 1.0 && ni > 4) {
			message("CENTRE OF MASS BADLY PLACED\n\nBALANCING ABORTED");
			wl = oldwl;
			pitch = 0.0;
			beta = 0.0;
			break;
		}
	}
	arrowcursor();
	if(!balanced) message("WARNING\nThe hull balancing process has failed.\nStatics results are not reliable");

    SwapBuf();
}

