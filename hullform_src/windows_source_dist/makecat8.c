/* Hullform component - makecat8.c
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
 
#ifdef PROF

#include "hulldesi.h"
extern void recalc_tanks(void);

void converthull()
{
	static REAL shift = 0.0;
	REAL a,hb,c,hd;
	int i,j,jj;

	if(getdlg(CONVERT,INP_REA,(void *) &shift,-1)) {
		numlin = extlin;
		extlin = 2 * extlin - 1;
		if(!realloc_hull(extlin)) {
			message("No free memory for hull expansion");
			extlin = numlin;
			return;
		}
		for(j = numlin, jj = numlin - 2 ; jj >= 0 ; j++, jj--) {
			for(i = stsec[jj] ; i <= ensec[jj] ; i++) {

				/*	Hull frames moved outwards by "shift"; stem section remains
				the same							*/

				if(i != 0)
					yline[j][i] = shift - yline[jj][i];
				else
				    yline[j][i] = yline[jj][i];

				/*	Vertical coordinates copied unaltered				*/

				zline[j][i] = zline[jj][i];
				zcont[j][i] = zcont[jj+1][i];
				linewt[j][i] = linewt[jj][i];
				if(relcont[jj]) {
					getparam(i,jj+1,&a,&hb,&c,&hd);
					if(i != 0)
						ycont[j][i] = shift - (yline[jj+1][i] + 0.5*a);
					else
					    ycont[j][i] = yline[jj+1][i] + 0.5*a;
				}
				else if(i != 0) {
					ycont[j][i] = shift - ycont[jj+1][i];
				}
				else {
					ycont[j][i] = ycont[jj+1][i];
				}
				relcont[j] = FALSE;
			}
			radstem[j] = radstem[jj];
			stsec[j] = stsec[jj];
			ensec[j] = ensec[jj];
		}
		for(j = 0 ; j < numlin ; j++) {
			for(i = max(1,stsec[j]) ; i <= ensec[j] ; i++) {
				yline[j][i] += shift;
				if(j != 0 && !relcont[j]) ycont[j][i] += shift;
			}
		}
		numlin = extlin;
		fl_line1[0] = extlin;
	}
	redef_transom();
	recalc_tanks();
}

void movehull()
{
	int i,j;
	REAL shift = 0.0;
	if(getdlg(MOVE,INP_REA,(void *) &shift,-1)) {
		for(j = 0 ; j < extlin ; j++) {
			for(i = 1 ; i < count ; i++) {
				yline[j][i] += shift;
				if(!relcont[j]) ycont[j][i] += shift;
			}
		}
	}
}

#endif
