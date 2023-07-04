/* Hullform component - view_sub.c
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

void get_int_angle(int j,REAL *c,REAL *s);

/*	plot hull diagonals (or buttocks for angle of 90 deg.)	*/

/*	interv	is interval between diagonals required
diaval	is angle at which the diagonals are set
mode	is 0-3 for end, side, plan or perspective views
side	is -1 when plotting port side, 1 when plotting starboard side
*/

void pltdia(REAL interv,REAL diaval,INT mode,REAL side)
{
	REAL	minoff,maxoff;
	REAL	sintem,costem;
	REAL	ref;

	if(interv > 0.0) {
		offset_range(&maxoff,&minoff,-diaval,interv);

		/*	Set the heel to match			*/

		sintem = sina;
		costem = cosa;
		ref  = -0.01745329 * side * diaval;
		sina = sin(ref);
		cosa = cos(ref);

		/*	Plot the "waterlines"			*/

		plotw(side,minoff,interv,mode);

		/*	Restore true heel value			*/

		sina = sintem;
		cosa = costem;
	}
}

/*	Work out range of offset values for plotting diagonals or buttocks */
/*	maxoff and minoff are the returned maximum and minimum offsets	*/
/*	diaval is the angle at which diagonals are evaluated (90 for buttocks)*/
/*	interv is required interval: minoff is return as an integer multiple */

void offset_range(REAL *maxoff,REAL *minoff,REAL diaval,REAL interv)
{
	int i,j;
	REAL ref;
	REAL sina,cosa;	/* note these are local variables only here */
	extern int count,extlin,numlin;
	REAL maxmin;	/* the maximum of all section minima */
	REAL secmin;
	if(extlin < numlin) extlin = numlin;

	sina = sin(0.01745329*diaval);
	cosa = cos(0.01745329*diaval);

	maxmin = -1.0e+30;
	*minoff = 1.0e+30;
	*maxoff = maxmin;
	for(i = 1 ; i < count ; i++) {	/* ignore stem */
		secmin = 1.0e+30;
		for(j = 0 ; j < extlin ; j++) {
			ref = sina*yline[j][i] + cosa*zline[j][i];
			if(ref > *maxoff) *maxoff = ref;
			if(ref < secmin) secmin = ref;	/* minimum for this section */
		}
		if(secmin < *minoff) *minoff = secmin;	/* minimum overall */
		if(maxmin <  secmin)  maxmin = secmin;	/* greatest minimum for any transverse section */
	}

	/*    if(fzer(sina)) *minoff = maxmin;	*/
	if(interv > 0.0) {
		i = (int) (*minoff / interv);
		if(*minoff >= 0.0) i++;
		*minoff = ((float) i) * interv;
	}
}

/*    determine whether two lines j0 and j1 are coincident between    */
/*    sections l and l+1			*/

int coincident(int l,int j0,int j1)
{
	int here,next;

	here = yline[j0][l] == yline[j1][l] && zline[j0][l] == zline[j1][l];
	if(l+1 < count)
		next = yline[j0][l+1] == yline[j1][l+1] && zline[j0][l+1] == zline[j1][l+1];
	else
		next = 1;

	return(here && next);
}

