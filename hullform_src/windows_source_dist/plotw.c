/* Hullform component - plotw.c
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

/*	Plot Waterline

side	is -1 when plotting port side, 1 when plotting starboard side
watlev	is initial waterline offset
dwl	is waterline offset increment
mode	is 0-3 for end, side, plan or perspective views
pltopp	is 1 is line may be plotted on the oppositer side, else 0
*/

void plotw(REAL side,REAL watlev,REAL dwl,INT mode)
{
	extern int	numbetwl,numbetw;
	int		numperl = numbetwl+ 1;
	int		nj = 1 + numperl*(numlin-1);
	REAL	(far *cl)[maxsec];
	REAL	(far *xl)[maxsec];
	REAL	(far *yl)[maxsec];
	REAL	(far *zl)[maxsec];
	char	(far *thru)[maxsec][2];
	long	arraysize = nj * sizeof(*cl);
	REAL	aa,cc,a,hb,c,hd;
	REAL	x,y,z,t,zc;
	int		i,j,k,l;
	REAL	ssina = sina * side;
	REAL	dt = 1.0/((REAL) numperl);

	int	search,is_below,was_below,first;
	int	sides;
	int imax,jmax,imin,jmin,idsear,jdsear,ip,jp,ii,jj,iip,jjp,id,jd;
	int im,jm,km,is;
	REAL xc1,yc1,zc1;
	REAL xnew,ynew,znew;
	int no_more;
	int ok = 0;
	REAL hwlocal;
	int contour_index;

	/*	Allocate working memory needed		*/

	if(memavail((void *) &cl,arraysize)) {
		if(memavail((void *) &xl,arraysize)) {
			if(memavail((void *) &yl,arraysize)) {
				if(memavail((void *) &zl,arraysize)) {
					if(memavail((void *) &thru,(long) nj*sizeof(*thru))) {
						ok = 1;
					}
					else {
						memfree(thru);
					}
				}
				else {
					memfree(yl);
				}
			}
			else {
				memfree(xl);
			}
		}
		else {
			memfree(cl);
		}
	}

	if(!ok) {
		message("No memory for contour grid calculation.\nA smaller number of drawing points per\nline may help");
		return;
	}

	/*	Create y, z and vertical offset tables		*/

	for(i = surfacemode ; i < count ; i++) {
		aa = 0.0;
		cc = 1.0;
		x = xsect[i];
		y = yline[0][i];
		z = zline[0][i];
		if(!surfacemode && i == 0) {
			x -= y;
			y = yline[stemli][1];
		}
		hwlocal = beta* x + hwl[i];
		zc = z * cosa + y * ssina + hwlocal;
		l = 0;
		for(j = 1 ; j < numlin ; j++) {
			if(stsec[j] <= i && ensec[j] >= i) {
				hullpa(i,j,aa,cc,&a,&hb,&c,&hd);
				tranpa(a,hb,c,hd,&aa,&cc);
				t = 1.0;
				for(k = 0 ; k < numperl ; k++) {
					xl[l][i] = x;
					yl[l][i] = y;
					zl[l][i] = z;
					cl[l++][i] = zc;
					t -= dt;
					z = zline[j][i] - t*(c+t*hd);
					y = yline[j][i] + t*(a+t*hb);
					if(!surfacemode && i == 0) {
						x = xsect[0] - y;
						y = yline[stemli][1];
						hwlocal = beta*x + hwl[i];
					}
					zc = z*cosa + y*ssina + hwlocal;
				}
			}
			else {
				for(k = 0 ; k < numperl ; k++) {
					xl[l][i] = x;
					yl[l][i] = y;
					zl[l][i] = z;
					cl[l++][i] = zc;
				}
			}
		}
		xl[l][i] = x;
		yl[l][i] = y;
		zl[l][i] = z;
		cl[l][i] = zc;
	}

	/*	SCAN RANGE OF REQUIRED CONTOURS					*/

	no_more = 0;
	contour_index = 0;
	while(!no_more) {

		/*	RESET MARKER ARRAY.						*/

		memset(thru,0,2*nj*maxsec);

		/*	INITIALISE SCAN INDEX						*/

		i = surfacemode;
		j = 1;
		was_below = cl[j][i] < watlev;
		no_more = 1;

		/*	THESE DELIMIT THE CURRENT EDGES OF THE RECTANGULAR "SPIRAL" SEARCH	*/
		/*		PATTERN							*/

		imax = count;		/* THESE WILL					*/
		jmax = nj;		/*  BE DECREMENTED				*/
		imin = surfacemode-1;	/*   AND INCREMENTED BEFORE THEY ARE USED	*/
		jmin = 0;		/* (JMIN IS USED FIRST)				*/

		xc1 = yc1= zc1 = 0.0;

		/*	THESE ARE THE INITIAL INDEX CHANGES ALONG THE SEARCH RECTANGLE. WE */
		/*	START AT POINT (0,1), GOING DOWN TO (0,0)			*/

		idsear = 0;
		jdsear = -1;

		/*	START LOOKING FOR NEW CONTOUR					*/

ad0001:
#ifdef EXT_OR_PROF
		if(mode == 7) {	// 3-D polyline required
			seqend();
			ensure_polyline('C',++contour_index,mode);
		} else {
			(*newlin)();
		}
#else
		(*newlin)();
#endif
		first = 1;
		search = 1;

		/*	CONTINUE SEARCH MODE.  POINTERS TO SEARCH POSITION AROUND SPIRAL ARE	*/
		/*		I AND J, POINTERS TO LAST SEARCH POSITION ARE IP AND JP.	*/

ad0008:
		ip = i;
		jp = j;
		i += idsear;
		j += jdsear;

		/*	TEST WHETHER TURN IS DUE.  IF SO, RESET OVERFLOWED SEARCH INDEX, TURN	*/
		/*	SEARCH DIRECTION AND RESET NEXT EDGE OF SEARCH RECTANGLE	*/

		if(i > imax) {
			i--;
			j++;
			idsear = 0;
			jdsear = 1;
			jmax--;
		}
		else if(i < imin) {
			i++;
			j--;
			idsear = 0;
			jdsear = -1;
			jmin++;
		}
		else if(j > jmax) {
			j--;
			i--;
			idsear = -1;
			jdsear = 0;
			imin++;
		}
		else if(j < jmin) {
			j++;
			i++;
			idsear = 1;
			jdsear = 0;
			imax--;
		}

		/*	THE RESET WILL CONTRACT THE SPIRAL TO ZERO AT THE END OF THE SEARCH.	*/
		/*	THUS, A CHECK WHETHER THE ARRAY HAS BEEN FULLY COVERED AT THIS STAGE	*/
		/*	TESTS FOR COMPLETION OF THE SEARCH.				*/

		if(jmin >= jmax || imin >= imax) {
			watlev += dwl;
			continue;
		}

		/*	WHILE TRACKING A CONTOUR, THE TRACKING POINTERS ARE II AND JJ.  TESTS	*/
		/*	ARE APPLIED TO THESE, SO THEY MUST BE ASSIGNED I AND J WHEN PERFORMING	*/
		/*	A SEARCH ALONG THE CONTOUR					*/

		ii = i;
		jj = j;
		iip = ip;
		jjp = jp;
		id = idsear;
		jd = jdsear;
		sides = 0;

ad0005:
		is_below  = cl[jj][ii]   < watlev;
		was_below = cl[jjp][iip] < watlev;

		/*	IM AND JM ARE POSITION IN THE BEEN-THROUGH-HERE-ALREADY ARRAY "THRU"	*/

		im = min(ii,iip);
		jm = min(jj,jjp);
		km = abs(ii-iip);

		if(thru[jm][im][km] || is_below == was_below)
		{

			/*	NO VALID INTERSECTION - KEEP SEARCHING FOR A CONTOUR ...	*/

			if(search) goto ad0008;

			/*	... OR KEEP SEARCHING FOR THE OTHER INTERSECTION ON THIS GRID BOX	*/

			/*	IF NUMBER OF SIDES REACHES 4 WITHOUT THE LOCATION OF AN EXIT,	*/
			/*	A CLOSED LOOP IS INDICATED.					*/

			if(++sides >= 4) {

				/*	CLOSED CONTOUR CALLED WHEN TRACKING POINTS EXHAUSTED.		*/

				pltsel(xc1,yc1,zc1,mode);
				no_more = 0;
				goto ad0001;

			}
			else {

				/*	SCAN DIRECTION ROTATED CLOCKWISE FOR NEXT TEST			*/

				is = id;
				id = jd;
				jd = -is;

				iip = ii;
				jjp = jj;
				ii = ii+id;
				jj = jj+jd;
				goto ad0005;

			}
		}
		else {

			/*	WHEN INTERSECTION FOUND, MARK THIS LINE SEGMENT AS "DRAWN THROUGH",	*/
			/*	CANCEL SEARCH MODE, SWITCH SENSE OF SCAN AROUND GRID BOX, RESET	*/
			/*	SIDES SEARCHED AROUND BOX TO ZERO AND INTERPOLATE TO FIND INTERSECTION	*/

			thru[jm][im][km] = 1;
			search = 0;
			sides = 0;

			a = cl[jj][ii] - cl[jjp][iip];
			if(a != 0.0) a = (cl[jj][ii] - watlev) / a;
			c = 1.0 - a;

			xnew =  c*xl[jj][ii] + a*xl[jjp][iip];
			ynew = (c*yl[jj][ii] + a*yl[jjp][iip]) * side;
			znew =  c*zl[jj][ii] + a*zl[jjp][iip];

			if(first) {

				/* STORE FIRST POINT ON CONTOUR				*/
				xc1 = xnew;
				yc1 = ynew;
				zc1 = znew;
				first = 0;
			}
			pltsel(xnew,ynew,znew,mode);/* DRAW CONTOUR TO THE POINT		*/
			no_more = 0;

			/*	ENSURE SEARCH CONTINUES IN A CLOCKWISE SENSE, FROM THE PREVIOUS	*/
			/*	POINT									*/

			is = jd;
			jd = id;
			id = -is;

			ii = iip+id;
			jj = jjp+jd;

			if(ii < count && ii >= surfacemode && jj < nj && jj >= 0) goto ad0005;

		}

		goto ad0001;	/* AND START LOOKING FOR NEXT CONTOUR	*/

	}
	memfree(cl);
	memfree(xl);
	memfree(yl);
	memfree(zl);
	memfree(thru);
#ifdef EXT_OR_PROF
	if(mode == 7) seqend();	// end any polyline
#endif
}

