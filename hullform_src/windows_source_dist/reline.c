/* Hullform component - reline.c
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

/*	re-orient hull lines as buttocks, diagonals or waterlines	*/

#include "hulldesi.h"

extern int changed;

static REAL line_interval = 0.0;/* default value forces definition each time */
static REAL sinsl,cossl;
void save_hull(int);

MENUFUNC edit_orie()
{
    static int numopt = 0;
		/* 0 for buttocks, 1 for diagonals, 2 for waterlines */
    static REAL line_angle = 45.0;
		/* significant only for diagonals */
    REAL	maxoff,minoff,offset;
    int		i,ii,j,l;
    REAL	preva,prevc;
    REAL	newa,newc;
    REAL	prevy,prevz;
    int		intersected;
    extern	REAL	line_interval,sinsl,cossl;
    REAL	a,hb,c,hd,aa,cc;
    REAL	t1,t2;
    REAL	dwl;
    REAL	newy,newz;
    REAL	a1,a2;
    REAL	cos_nonzero;
    char *fail = "Out of lines for expansion";

    extern char *warn;

    if(count > 0) {
	message(warn);

	if(ntank > 0) {
	    if(!getdlg(RELITANK,-1,NULL)) return;
	    (void) realloc_hull(numlin);
	    extlin = numlin;
	    ntank = 0;
        }

	if(getdlg(EDITRELI,
	INP_REA,(void *) &line_interval,
	INP_REA,(void *) &line_angle,
	INP_RBN,(void *) &numopt,-1)) {
	    switch(numopt) {
	      case 0:		/* buttocks */
		line_angle = 90.0;
		sinsl = -1.0;
		cossl = 0.0;
		break;
              case 1:		/* diagonals */
		sinsl = -sin(0.01745329 * line_angle);
		cossl = cos(0.01745329 * line_angle);
                break;
	      case 2:		/* waterlines */
		line_angle = 0.0;
		sinsl = 0.0;
		cossl = 1.0;
		break;
            }
	    cos_nonzero = (cossl == 0.0 ? 0.000001 : cossl);

/*	Don't let through a zero or negative interval		*/

	    if(line_interval <= 0.0) return;

/*	cancel all floodable tanks				*/

	    ntank = 0;
	    extlin = numlin;

/*	get range of offsets at this angle			*/

	    offset_range(&maxoff,&minoff,180.0-line_angle,line_interval);

/*	"minoff" is returned as a multiple of line_interval. We need	*/
/*	to start at the other end - step we out to the maximum offset	*/

	    for(offset = minoff ; offset < maxoff ; offset += line_interval) ;
	    maxoff = offset;
	    relcont[0] = TRUE;

/*	Scan all sections, finding the intersection with each	*/
/*	diagonal where possible					*/

	    for(i = surfacemode ; i < count ; i++) {

/*	Step in from maximum offset				*/

		offset = maxoff;
		l = numlin-1;		/* new line index	*/
		j = 1;			/* old line index	*/

/*	work out section curve alignment at gunwhale		*/

		hullpa(i,1,0.0,1.0,&a,&hb,&c,&hd);
		tranpa(a,hb,c,hd,&aa,&cc);

		preva = a + hb + hb;
		prevc = c + hd + hd;
		if(preva == 0.0 && prevc == 0.0) {
		    preva = a;
		    prevc = c;
		}
		prevy = yline[0][i];
		prevz = zline[0][i];

/*	Step through offsets for each new line, incrementing the new line
	index each time
*/
		while(offset > minoff + 0.5 * line_interval) {
		    offset -= line_interval;
		    l++;
		    intersected = 0;

/*	expand array allocations on first use			*/

		    if(i == surfacemode) {	/* 0 or 1 */
			if(!realloc_hull(extlin)) {
			    message(fail);
			    return;
			}
			stsec[l] = maxsec;
			ensec[l] = 0;
			fl_line1[0] = ++extlin;
			for(ii = maxsec ; ii <= maxsec + 1 ; ii++) {
			    yline[l][maxsec+1] = 0.0;
			    zline[l][maxsec+1] = 0.0;
			    ycont[l][maxsec+1] = 0.0;
			    zcont[l][maxsec+1] = 0.0;
			}
		    }

/*	Scan through the table of old lines				*/

		    while(1) {	/* while j <= numlin-1, but for two lines */
			if(stsec[j] <= i && ensec[j] >= i) {
			    if(i == 0) {
				dwl = offset / cos_nonzero + zline[j][i];
				inters(a,hb,c,hd,0.0,1.0,dwl,&t1,&t2);
			    } else {
				dwl = offset + (sinsl*yline[j][i] + cossl*zline[j][i]);
				inters(a,hb,c,hd,sinsl,cossl,dwl,&t1,&t2);
			    }
			    if(t1 < -0.00001 || t1 > 1.00001) t1 = t2;
			    if(t1 >= -0.00001 && t1 <= 1.00001) {
				if(stsec[l] > i) stsec[l] = i;
				if(ensec[l] < i) ensec[l] = i;
				newa = a + t1 * (hb + hb);
				newc = c + t1 * (hd + hd);
				newy = yline[j][i] + t1 * (t1 * hb + a);
				newz = zline[j][i] - t1 * (t1 * hd + c);
				yline[l][i] = newy;
				zline[l][i] = newz;
				a1 = prevc * newa;
				a2 = preva * newc;
				t2 = a1 - a2;
				if(fabs(t2) > 1.0e-5*fabs(a1 + a2)) {
				    t2 = (prevc*(prevy-newy)-preva*(newz-prevz)) / t2;
				    zcont[l][i] = newz - newc*t2;
				    ycont[l][i] = newy + newa*t2;
				} else {
				    ycont[l][i] = prevy;
				    zcont[l][i] = prevz;
				}
				intersected = 1;
				preva = newa;
				prevc = newc;
				prevy = newy;
				prevz = newz;
				break;/* break out of loop though new lines*/
			    }
			}	/* .. else pass to next line */

/*	If we have searched over the whole section without an intersection,
	exit.
*/
			if(j >= numlin-1) break;

			hullpa(i,++j,aa,cc,&a,&hb,&c,&hd);
			/* no intersection in this part of section yet	*/
			tranpa(a,hb,c,hd,&aa,&cc);
		    }	/* end of loop through new hull lines which may
				intersect above the current old line	*/

/*	If there is no intersection, presume the sheer or stem line	*/
/*	is the correct line to meet					*/

		    if(!intersected) {
			if(dwl < 0.00001) {/* ... if diagonal lies below last line */
			    yline[l][i] = yline[stemli][i];
			    zline[l][i] = zline[stemli][i];
			    zcont[l][i] = zline[stemli][i];
			    ycont[l][i] = yline[stemli][i];
			} else {
			    yline[l][i] = yline[0][i];
			    zline[l][i] = zline[0][i];
			    zcont[l][i] = zline[0][i];
			    ycont[l][i] = yline[0][i];
			    j = 1;
			    hullpa(i,1,0.0,1.0,&a,&hb,&c,&hd);
			    tranpa(a,hb,c,hd,&aa,&cc);
			}
		    }
		}		/* end of loop through offset values */

/*	If any line has intersected this section, make the lateral control
	offset for the last line zero
*/
		if(intersected) {
		    getparam(i,numlin-1,&newa,&hb,&newc,&hd);
		    newy = yline[numlin-1][i];
		    newz = zline[numlin-1][i];
		    a1 = prevc * newa;
		    a2 = preva * newc;
		    t2 = a1 - a2;
		    if(fabs(t2) > 1.0e-5*fabs(a1 + a2)) {
			t2 = (prevc*(prevy-newy)-preva*(newz-prevz)) / t2;
			zcont[numlin-1][i] = newz - newc * t2;
			ycont[numlin-1][i] = newy + newa * t2;
		    } else {
			ycont[l][i] = prevy;
			zcont[l][i] = prevz;
		    }
		}
	    }

/*	Now we have old lines indexed from 0 to numlin-1, and new lines	*/
/*	from numlin to extlin-1. We want to retain the sheerline,	*/
/*	the new lines, and the old keel line.				*/

	    if(!realloc_hull(extlin)) {/* get workspace for copying lines	*/
		message(fail);
	    } else {
		copyline(extlin,numlin-1,0.0);	/* save the keel line	*/
		for(j = numlin, i = 1 ; j <= extlin ; j++, i++) copyline(i,j,0.0);
					/* copy lines and put back the keel line */
		numlin = i;
		extlin = i;
		stemli = --i;
		(void) realloc_hull(extlin);
					/* free the workspace used for the copy */
		for(l = 0 ; l < numlin ; l++) relcont[l] = FALSE;
	    }
	}
    }
	save_hull(MAINHULL);
    recalc_transom();
}

#endif
