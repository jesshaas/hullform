/* Hullform component - strake.c
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

int strake_line = 1;
REAL strake_width = 0.10;
int strake_orientation = 0;		// perpendicular = 0; fixed angle = 1
REAL strake_angle = 0.0;
REAL inside_top = 0.03;
REAL inside_bottom = -0.03;
REAL outside_top = 0.01;
REAL outside_bottom = -0.01;

void save_hull(int);

void strake()
{
	int i,jj,j,ext;
	REAL A,B,a1,hb1,c1,hd1,a2,hb2,c2,hd2,t,t1,t2,yl,zl;

	strake_line++;
	if(getdlg(STRAKE,
	INP_INT,(void *) &strake_line,
	INP_REA,(void *) &strake_width,
	INP_REA,(void *) &inside_top,
	INP_REA,(void *) &inside_bottom,
	INP_REA,(void *) &outside_top,
	INP_REA,(void *) &outside_bottom,
	INP_RBN,(void *) &strake_orientation,
	INP_REA,(void *) &strake_angle,
	-1)) {
		if(extlin >= maxlin - 6) {
			message("Insufficient lines available");
			return;
		}
#ifdef PROF
		if(strake_line > numlin) {
			message("Can't add a strake to a tank");
			return;
		}
#endif
		ext = 3;	// extra lines needed
		realloc_hull(extlin+ext+5);	// add lines needed temporarily, at extlin, extlin+1, extlin+2 and possibly extlin+3, also leaving room for the strake lines
		strake_line--;
		for(i = stsec[strake_line] ; i <= ensec[strake_line] ; i++) {

			if(strake_line > 0) {
				getparam(i,strake_line,&a1,&hb1,&c1,&hd1);
			} else {
				a1 = 0.0;
				hb1 = 0.0;
				c1 = 0.0;
				hd1 = 0.0;
			}
			if(strake_line < numlin-1) {
				getparam(i,strake_line+1,&a2,&hb2,&c2,&hd2);
			} else {
				a2 = 0.0;
				hb2 = 0.0;
				c2 = 0.0;
				hd2 = 0.0;
			}

//	Find and normalise orientation vector (A,B)

			if(strake_orientation == 1) {	// fixed
				A = cos(0.01745329*strake_angle);
				B = sin(0.01745329*strake_angle);
			} else {						// perpendicular to surface
				B = -(a1 + a2+2.0*hb2);
				A = c1 + c2+2.0*hd2;
				t = A*A + B*B;
				if(t > 0.0) {
					t = sqrt(t);
					A /= t;
					B /= t;
				}
			}

//	Find intersection of upper edge of strake with the hull

			inters(a1,hb1,c1,hd1,B,A,inside_top,&t1,&t2);
			if(t2 >= 0.0 && t2 < 1.0)
				t = t2;
			else if(t1 >= 0.0 && t1 < 1.0)
				t = t1;
			else
				t = 0.0;
			jj = extlin + ext;	// inside top
			yl = yline[strake_line][i];
			zl = zline[strake_line][i];
			yline[jj][i] = yl + t*(a1 + t*hb1);
			zline[jj][i] = zl - t*(c1 + t*hd1);
			ycont[jj][i] = yline[jj][i];
			zcont[jj][i] = zline[jj][i];
			jj++;				// outside top
			yline[jj][i] = yl + strake_width*A - outside_top*B;
			zline[jj][i] = zl - strake_width*B - outside_top*A;
			ycont[jj][i] = yline[jj][i];
			zcont[jj][i] = zline[jj][i];
			jj++;				// outside bottom
			yline[jj][i] = yl + strake_width*A - outside_bottom*B;
			zline[jj][i] = zl - strake_width*B - outside_bottom*A;
			ycont[jj][i] = yline[jj][i];
			zcont[jj][i] = zline[jj][i];
			inters(a2+2.0*hb2,hb2,c2+2.0*hd2,hd2,B,A,inside_bottom,&t1,&t2);
			if(t1 > -1.0 && t1 <= 0.0)
				t = t1;
			else if(t2 > -1.0 && t2 <= 0.0)
				t = t2;
			else
				t = 0.0;
			jj++;				// inside bottom
			yline[jj][i] = yl + t*(a2+2.0*hb2 + t*hb2);
			zline[jj][i] = zl - t*(c2+2.0*hd2 + t*hd2);
			ycont[jj][i] = yline[jj][i];
			zcont[jj][i] = zline[jj][i];

//	Put the next line's control point in the "absolute" position

			ycont[strake_line + 1][i] = yline[strake_line + 1][i] + 0.5*(1.0 - t)*a2;
			zcont[strake_line + 1][i] = zline[strake_line + 1][i] - 0.5*(1.0 - t)*c2;

			for(j = extlin+ext ; j <= jj ; j++) linewt[j][i] = 1.0;
		}
		relcont[strake_line + 1] = FALSE;
		for(j = extlin+ext ; j <= jj ; j++) {
			stsec[j] = stsec[strake_line];
			ensec[j] = ensec[strake_line];
#ifndef STUDENT
			radstem[j] = radstem[strake_line];
#endif
			relcont[j] = FALSE;
		}
		for(j = extlin-1 ; j > strake_line ; j--) copyline(j+ext,j,0.0);
		for(j = strake_line, jj = extlin+ext ; j <= strake_line+ext ; j++, jj++) copyline(j,jj,0.0);
		extlin += ext;
		numlin += ext;
		stemli += ext;
	}
#ifdef PROF
	save_hull(MAINHULL);
	redef_transom();
#endif
}

#endif

