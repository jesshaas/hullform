/* Hullform component - overlay.c
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
 
int force_linear;

#ifdef EXT_OR_PROF

#include "hulldesi.h"

#ifdef linux

#include <X11/X.h>

#endif

REAL tvalue(REAL dy,REAL dz,REAL a,REAL hb,REAL c,REAL hd);
int current_hull = MAINHULL;
void save_hull(int);
void use_hull(int);
void read_hullfile(FILE *fp,char *hullfile,int all);
void edit_surface_set(void);
extern int showoverlay;
int merging;
void setranges(REAL xl,REAL xr,REAL yb,REAL yt);
void perform_merge(void);
void xorcol(int);
REAL section_curve_intersection(REAL y1,REAL z1,REAL a1,REAL hb1,REAL c1,REAL hd1,
	REAL y2,REAL z2,REAL a2,REAL hb2,REAL c2,REAL hd2);
extern REAL xnew[];
extern int newcou;
void remove_stringers(int);
REAL SolveCubic(REAL,REAL,REAL,REAL);

#ifndef STUDENT
void recalc_tanks(void);
#endif

REAL tvalue(REAL dy,REAL dz,REAL a,REAL hb,REAL c,REAL hd);
void inters(REAL a,REAL hb,REAL c,REAL hd,REAL sina,REAL cosa,REAL dwl,REAL *t1,REAL *t2);
int best_solution(REAL sol2[],int nsol);
void striptopath(char *path);
void save_surface_set(void);
extern REAL disp_main,disp_overlay;
int prev_error;

/*	This routine, in builders.c, returns to "t" value for
a pair of offsets (dy,dz) along a hull curve
*/
REAL tvalue(REAL dy,REAL dz,REAL a,REAL hb,REAL c,REAL hd);

#define OVERLAYCOLOUR 9

MENUFUNC overlay_hull()
{
	extern int  changed;
	FILE	*fp;

	if(openfile(hullfile_overlay,"rt",
		"Open an Overlay Design","hull data files(*.HUD)\0*.hud\0",
		"*.hud",dirnam,&fp)) {
		use_hull(OVERLAYHULL);
		read_hullfile(fp,hullfile_overlay,FALSE);
		use_hull(MAINHULL);
		showoverlay = TRUE;
	}
}


void save_hull(int choice)
{
	switch(choice) {
	case OVERLAYHULL:
		numlin_overlay = numlin;
		stemli_overlay = stemli;
		extlin_overlay = numlin;	/* no tanks in overlay */
		count_overlay = count;
		xsect_overlay = xsect;
		yline_overlay = yline;
		ycont_overlay = ycont;
		zline_overlay = zline;
		zcont_overlay = zcont;
		stsec_overlay = stsec;
		ensec_overlay = ensec;
		radstem_overlay = radstem;
		relcont_overlay = relcont;
		autofair_overlay = autofair;
		linewt_overlay = linewt;
		surfacemode_overlay = surfacemode;
#ifndef STUDENT
		transom = 0;
		fl_line1[0] = numlin;
#endif
		hullfile_overlay = hullfile;
		disp_overlay = disp;
		break;

	case MAINHULL:
		numlin_main = numlin;
		stemli_main = stemli;
		extlin_main = extlin;
		count_main = count;
		xsect_main = xsect;
		yline_main = yline;
		ycont_main = ycont;
		zline_main = zline;
		zcont_main = zcont;
		stsec_main = stsec;
		ensec_main = ensec;
		radstem_main = radstem;
		relcont_main = relcont;
		autofair_main = autofair;
		linewt_main = linewt;
#ifndef STUDENT
		transom = atransom != 0.0;
		fl_line1[0] = numlin;
#endif
		surfacemode_main = surfacemode;
		hullfile_main = hullfile;
		disp_main = disp;
		break;
	}
}

void use_hull(int choice)
{
	if(choice != current_hull) save_hull(current_hull);
	switch(choice) {
	case MAINHULL:
		numlin = numlin_main;
		stemli = stemli_main;
		extlin = extlin_main;
		count = count_main;
		xsect = xsect_main;
		yline = yline_main;
		ycont = ycont_main;
		zline = zline_main;
		zcont = zcont_main;
		stsec = stsec_main;
		ensec = ensec_main;
		radstem = radstem_main;
		relcont = relcont_main;
		autofair = autofair_main;
		linewt = linewt_main;
#ifndef STUDENT
		fl_line1[0] = numlin;
#endif
		surfacemode = surfacemode_main;
		hullfile = hullfile_main;
		disp = disp_main;
		break;

	case OVERLAYHULL:
		(*colour)(OVERLAYCOLOUR);
		numlin = numlin_overlay;
		stemli = stemli_overlay;
		extlin = numlin_overlay;	/* no tanks in overlay */
		count = count_overlay;
		xsect = xsect_overlay;
		yline = yline_overlay;
		ycont = ycont_overlay;
		zline = zline_overlay;
		zcont = zcont_overlay;
		stsec = stsec_overlay;
		ensec = ensec_overlay;
		radstem = radstem_overlay;
		relcont = relcont_overlay;
		autofair = autofair_overlay;
		linewt = linewt_overlay;
#ifndef STUDENT
		fl_line1[0] = numlin;
#endif
		surfacemode = surfacemode_overlay;
		hullfile = hullfile_overlay;
		disp = disp_overlay;
		break;
	}
	current_hull = choice;
	endlin = extlin;
}

void do_merge(int jv,int jm,int use_overlay,int merge_down);

void do_merge(int jv,int jm,int use_overlay_sections,int merge_down)
{
	int i,j,n,is,ie,jmain,jovrl,jmerge,ms,me,vs,ve,im,iv;
	int stsv,ensv,sts;
	REAL av[maxsec],hbv[maxsec],cv[maxsec],hdv[maxsec];
	REAL a,hb,c,hd;
	REAL t,tt,y1,z1;
	REAL xstart_overlay = xsect_overlay[1];
	int lines;
	void edit_rese_perform(void);
	extern REAL (*yline_merge)[maxsec+4];
	extern REAL (*zline_merge)[maxsec+4];
	extern REAL (*ycont_merge)[maxsec+4];
	extern REAL (*zcont_merge)[maxsec+4];
	extern int *stsec_merge,*ensec_merge;
	extern REAL (*linewt_merge)[maxsec+4];
	extern REAL *radstem_merge;
	extern int *relcont_merge;
	extern int *autofair_merge;
	int use_overlay[maxsec],use_main[maxsec];
	REAL forward_intersection = 1.0e+10,aft_intersection = 1.0e+10;
	REAL deriv[maxsec];
	REAL am[maxsec],bm[maxsec],cm[maxsec];
	REAL bv[maxsec];
	int new_overlay_start = -1;
	int new_overlay_end = -1;
	REAL start_overlay,end_overlay;
	extern int numbetw;

	if(--jm < 1 || --jv < 1) {
		message("Indices must be 2 or greater");
		return;
	}

#ifndef STUDENT
	for(i = 0 ; i < extlin_main ; i++) remove_stringers(i);
	showstringers = FALSE;
#endif

	start_overlay = xsect_overlay[surfacemode_overlay];
	end_overlay = xsect_overlay[count_overlay - 1];

	/*	Generate sections for merged hull	*/

	j = surfacemode_overlay;	/* index to overlay sections - no 0 if surface mode */

	/*	Discard any overlay sections forward of the stem	*/

	while(xsect_overlay[j] <= xsect_main[0] && j < count_overlay) j++;
	xnew[0] = xsect_main[0];

	if(merge_down) {

		/*	If the stem line of the overlay intersects the stem line of the main hull,
		add a section at their intersection point				*/

		use_hull(MAINHULL);
		ms = stsec[stemli];
		me = ensec[stemli];
		spline2(&xsect[ms],&zline[stemli][ms],&linewt[stemli][ms],me-ms+1,&a,&a,0,
			zline[stemli][maxsec],zline[stemli][maxsec+1],deriv,&am[ms],&bm[ms],&cm[ms]);
		use_hull(OVERLAYHULL);
		vs = stsec[stemli];
		ve = ensec[stemli];
		spline2(&xsect[vs],&zline[stemli][vs],&linewt[stemli][vs],ve-vs+1,&a,&a,0,
			zline[stemli][maxsec],zline[stemli][maxsec+1],deriv,&av[vs],&bv[vs],&cv[vs]);
		im = ms+1;
		iv = vs+1;
		while(im < count_main    && xsect_main[im]    < xsect_overlay[vs]) im++;
		while(iv < count_overlay && xsect_overlay[iv] < xsect_main[ms]) iv++;

		/*	"im" and "iv" are now the aft ends of line segments whose crossing may be tested	*/

		while(1) {
			c = xsect_overlay[iv-1] - xsect_main[im-1];

			/*	The value of "a" before addition is a position relative to overlay section iv-1		*/
			a = xsect_overlay[iv-1] +
				SolveCubic(am[im-1]-av[iv-1],
					bm[im-1]+ 3.0*am[im-1]*c - bv[iv-1],
					cm[im-1]+(3.0*am[im-1]*c+2.0*bm[iv-1])*c - cv[iv-1],
					zline_main[stemli_main][im-1]+((am[im-1]*c+bm[im-1])*c+cm[im-1])*c - zline_overlay[stemli_overlay][iv-1]);
			if(a >= xsect_overlay[iv-1]-0.0001 && a < xsect_overlay[iv]+0.0001) {
				if(forward_intersection >= 0.9e+10) {
					forward_intersection = a;	// where lines cross
					start_overlay = a;
				}
				else {	// second time, it is the aft intersection
					aft_intersection = a;
					end_overlay = a;
					break;
				}
			}
			if(xsect_overlay[iv] < xsect_main[im])
				iv++;
			else
				im++;
			if(iv >= count_overlay || im >= count_main) break;
		}
	}

	/*	Create the table of new sections, including forward and aft stem line intersections	*/

	n = 1; /* count of interpolated sections */
	xsect_overlay[count_overlay] = 1.0e+37;
	for(i = 1 ; i < count_main && n < maxsec-2 ; i++) {
		if(use_overlay_sections) { /* if using overlay sections, insert those before the next main hull section */
			while(xsect_overlay[j] < xsect_main[i] && n < maxsec-3) {
				if(xsect_overlay[j] > forward_intersection) {
					new_overlay_start = n;
					xnew[n++] = forward_intersection;
					forward_intersection = 1.0e+10;
				}
				if(xsect_overlay[j] > aft_intersection) {
					new_overlay_end = n;
					xnew[n++] = aft_intersection;
					aft_intersection = 1.0e+10;
				}
				xnew[n++] = xsect_overlay[j++];
			}
			if(xsect_main[i] == xsect_overlay[j]) j++;	/* skip any duplicated overlay section */
		}
		if(xsect_main[i] > forward_intersection) {
			xnew[n++] = forward_intersection;
			forward_intersection = 1.0e+10;
		}
		if(xsect_main[i] > aft_intersection) {
			xnew[n++] = aft_intersection;
			aft_intersection = 1.0e+10;
		}
		xnew[n++] = xsect_main[i];
	}

	newcou = n;

	use_hull(OVERLAYHULL);
	force_linear = TRUE;
	edit_rese_perform();	/* re-section the overlay hull to the new section positions */
	force_linear = FALSE;
	save_hull(OVERLAYHULL);

	for(j = 0 ; j < numlin_overlay ; j++) if(stsec_overlay[j] == 0) stsec_overlay[j] = 1;

	/*	After the above, section zero on the overlay is a transverse,
	not a longitudinal, section.
	*/
	use_hull(MAINHULL);

	surfacemode = disp_main < 0.0;

	/*	Interpolate main hull to new section set, if necessary	*/

	if(use_overlay_sections) {
		use_hull(MAINHULL);
		force_linear = TRUE;
		edit_rese_perform();
		force_linear = FALSE;
		save_hull(MAINHULL);
	}

	/*	Make all control points absolute	*/
	/* OVERLAYHULL = 1; MAINHULL = 0 */

	for(n = MAINHULL ; n <= OVERLAYHULL ; n++) {
		use_hull(n);
		for(j = 1 ; j < extlin ; j++) {
			if(relcont[j]) {
				for(i = stsec[j] ; i <= ensec[j] ; i++) {
					getparam(i,j,&a,&hb,&c,&hd);
					ycont[j][i] = yline[j][i] + 0.5*a;
					zcont[j][i] = zline[j][i] - 0.5*c;
				}
				relcont[j] = FALSE;
			}
		}
	}

	/*	Get (new) overlay start and end sections, and start and
	end sections for main hull
	*/
	stsv = stsec_overlay[jv];
	ensv = ensec_overlay[jv];

	/*	Allocate memory to the hull-merge arrays. Later, this
	memory will become the main hull memory, with the original
	main hull memory released.
	*/
	use_hull(MAINHULL);
	if(merge_down) {
		lines = jm + 1 + (extlin_overlay - jv);
	}
	else {
		lines = max(0,jv-jm) + extlin + 1;
	}
	i = sizeof(*yline)*lines;
	if(	!memavail((void *) &yline_merge,i) ||
		    !memavail((void *) &ycont_merge,i) ||
		    !memavail((void *) &zline_merge,i) ||
		    !memavail((void *) &zcont_merge,i) ||
		    !memavail((void *) &radstem_merge,maxlin*sizeof(REAL)) ||
		    !memavail((void *) &relcont_merge,(maxlin+3)*sizeof(int)) ||
		    !memavail((void *) &autofair_merge,(maxlin+3)*sizeof(int)) ||
		    !memavail((void *) &linewt_merge,i) ||
		    !memavail((void *) &stsec_merge,maxlin*sizeof(int)) ||
		    !memavail((void *) &ensec_merge,maxlin*sizeof(int)) ) {
		message("No memory for hull merging operation");
		memfree(yline_merge);
		memfree(ycont_merge);
		memfree(zline_merge);
		memfree(zcont_merge);
		memfree(stsec_merge);
		memfree(ensec_merge);
		memfree(radstem_merge);
		memfree(autofair_merge);
		memfree(relcont_merge);
		memfree(linewt_merge);
		return;
	}

	/*	In Merge-up Mode:

	Lines 0 to jv-1 of the overlay will form lines max(0,jm-jv) to max(jv,jm)-1
	of the new hull, the surfaces jv of the overlay and jm of
	the main will intersect to form line max(jm,jv) of the new hull,
	and lines jm to extlin-1 of the main hull will form lines
	max(jm,jv)+1 to max(0,jv-jm)+extlin of the new hull.

	An exception to this rule occurs when the overlay process
	generates partial lines. Then, lines max(0,jm-jv) to max(jv,jm)-1 of the main
	hull remain as lines max(0,jv-jm) to max(jm,jv)-1 of the merged hull,
	beyond the ends of the overlay hull lines. If jv >= jm,
	lines jm to jv become partial lines.

	Before and after the ends of the overlay lines, lines from
	0 to jm-1 from the main hull go to the merge hull.

	If jv > jm, there are not enough lines in the main hull to
	fill the vacant offsets, so the needed parts of lines 0 to
	jm - 1 from the main hull become lines jv - jm to jv - 1 of
	the merged hull. Lines 0 to jv - jm - 1 become partial lines.

	The merge hull line index is n,
	the overlay hull line index is jovrl,
	The main hull line index is jmain

	In Merge-Down Mode:

	Lines 0 to jm-1 of the main hull remain untouched
	Lines jv onward of the overlay hull become lines jm+1 onward of the merged hull
	*/

	if(merge_down) {
		jmerge = jm;
		jmain = 0;
		jovrl = jv - (jm+1);
	}
	else {
		jmerge = max(jm,jv);	/* intersection line in new hull */
		jmain = jm - (jmerge+1);
		jovrl = jv - jmerge;
	}
	for(n = 0 ; n < lines ; n++, jovrl++, jmain++) {
		for(i = 0 ; i < count ; i++) {
			use_overlay[i] = FALSE;
			use_main[i] = FALSE;
		}
#ifdef PROF
		if(jmain >= 0)
			radstem_merge[n] = radstem_main[jmain];
		else
		    radstem_merge[n] = radstem_main[jovrl];
#endif
		if(jmain >= 0)
			autofair_merge[n] = autofair_main[jmain];
		else
		    autofair_merge[n] = autofair_main[jovrl];

		/*	Use the main whenever overlay is not available and the main is */
		if(jmain >= 0 && jmain < extlin_main) {
			for(i = stsec_main[jmain] ; i <= ensec_main[jmain] ; i++) use_main[i] = TRUE;
		}

		/*	Use the overlay for overlay lines jv onward in merge_down mode, 0 to jm-1 otherwise	*/

		/*	if( (merge_down && jovrl >= jv) || (!merge_down && n < jmerge && jovrl >= 0 && jovrl <= jm) ) {	*/
		if( (merge_down && jovrl >= jv) || (!merge_down && n < jmerge && jovrl >= 0 && jovrl <= jv) ) {
			for(i = stsec_overlay[jovrl] ; i <= ensec_overlay[jovrl] ; i++) {
				if(xsect_overlay[i] >= start_overlay && xsect_overlay[i] <= end_overlay) use_overlay[i] = TRUE;
			}
			radstem_merge[n] = radstem_overlay[jovrl];
			autofair_merge[n] = autofair_overlay[jovrl];
		}
		is = maxsec;
		ie = 0;
		relcont_merge[n] = FALSE;

		/*	Insert the non-interpolated sections: from main before the merge
		lines, from overlay after.
		*/
		for(i = 0 ; i < count ; i++) {
			if(use_overlay[i]) {
				if(is == maxsec) is = i;
				ie = i;
				yline_merge[n][i] = yline_overlay[jovrl][i];
				zline_merge[n][i] = zline_overlay[jovrl][i];
				ycont_merge[n][i] = ycont_overlay[jovrl][i];
				zcont_merge[n][i] = zcont_overlay[jovrl][i];
				linewt_merge[n][i] = linewt_overlay[jovrl][i];
			}
			else if(use_main[i]) {
				if(is == maxsec) is = i;
				ie = i;
				yline_merge[n][i] = yline_main[jmain][i];
				zline_merge[n][i] = zline_main[jmain][i];
				ycont_merge[n][i] = ycont_main[jmain][i];
				zcont_merge[n][i] = zcont_main[jmain][i];
				linewt_merge[n][i] = linewt_main[jmain][i];
			}
			else {
			}
		}
		stsec_merge[n] = is;
		ensec_merge[n] = ie;
	}

	/*	Intersection of overlay section 0 (transverse) with
	main hull stem
	*/
	i = stsec_overlay[0];
	if(stsec_main[jm] == 0 && xstart_overlay <= xsect_main[0]) {

		/*	If the start point of the base line on the main hull
		lies at the stem, there may be an intersection of the
		overlay surface with the stem section. If so, we need
		to find the coordinates of the new line on the stem.

		NOTE: There is no overlay stem considered. Overlay section 0
		may be there, but it is ignored.
		*/
		getparam(0,jm,&a,&hb,&c,&hd);	/* We are still using the main hull */

		/*	Find the intersection of overlay line 0 with the main hull stem section.	*/
		y1 = xsect_overlay[i+1]-xsect_overlay[i];
		z1 = zline_overlay[0][i+1]-zline_overlay[0][i];
		t = section_curve_intersection(
			xsect_main[0]-xsect_overlay[i],zline_overlay[0][i],y1,0.0,z1,0.0,
			yline_main[jm][0],zline_main[jm][0],a,hb,c,hd);
		if(t >= -0.0001) {
			stsec_merge[jmerge] = 0;
			yline_merge[jmerge][0] = yline_main[jm][0] + t*(a+t*hb);
			zline_merge[jmerge][0] = zline_main[jm][0] - t*(c+t*hd);
			zcont_merge[jmerge][0] = zline_merge[jmerge-1][0];
			ycont_merge[jmerge][0] = yline_merge[jmerge-1][0];
			t = tvalue(yline_merge[jmerge][0]-yline_merge[jmerge+1][0],
				zline_merge[jmerge+1][0]-zline_merge[jmerge][0],
				a,hb,c,hd);
			tt = 1.0-t;
			/*	    ycont_merge[jmerge+1][0] = ycont_merge[jmerge+1][0]*t + yline_merge[jmerge+1][0]*tt;
			zcont_merge[jmerge+1][0] = zcont_merge[jmerge+1][0]*t + zline_merge[jmerge+1][0]*tt;	*/
			ycont_merge[jmerge+1][0] = ycont_main[jm][0]*t + yline_main[jm-1][0]*tt;
			zcont_merge[jmerge+1][0] = zcont_main[jm][0]*t + zline_main[jm-1][0]*tt;
		}
		for(j = 0 ; j < jmerge ; j++) {
			yline_merge[j][0] = yline_merge[jmerge][0];
			zline_merge[j][0] = zline_merge[jmerge][0];
			ycont_merge[j][0] = yline_merge[jmerge][0];
			zcont_merge[j][0] = zline_merge[jmerge][0];
		}
	}

	sts = stsec_merge[jm];
	if(sts <= 0) sts = 1;
	prev_error = FALSE;

	/*	Get shape parameters for the overlay hull surface. We do this
	in advance, to save having to swap hulls twice each loop.
	*/
	use_hull(OVERLAYHULL);
	for(i = stsv ; i <= ensv ; i++) {
		getparam(i,jv,&av[i],&hbv[i],&cv[i],&hdv[i]);
	}

	use_hull(MAINHULL);
	for(i = 0 ; i < count ; i++) {

		/*	Find intersection points for each section on the main hull	*/

		if(i >= stsv && i <= ensv && xsect[i] >= start_overlay && xsect[i] <= end_overlay) {
			getparam(i,jm,&a,&hb,&c,&hd);
			if(merge_down) {
				t = section_curve_intersection(
					yline_overlay[jv][i],zline_overlay[jv][i],av[i],hbv[i],cv[i],hdv[i],
					yline_main   [jm][i],zline_main   [jm][i],a,hb,c,hd);
				if(t >= -0.0001) {
					tt = 1.0 - t;
					yline_merge[jmerge][i] = yline_main[jm][i] + t*(a + t*hb);
					zline_merge[jmerge][i] = zline_main[jm][i] - t*(c + t*hd);
					zcont_merge[jmerge][i] = zcont_main[jm][i]*tt + zline_main[jm-1][i]*t;
					ycont_merge[jmerge][i] = ycont_main[jm][i]*tt + yline_main[jm-1][i]*t;
					t = tvalue(yline_merge[jmerge][i]-yline_merge[jmerge+1][i],
						zline_merge[jmerge+1][i]-zline_merge[jmerge][i],av[i],hbv[i],cv[i],hdv[i]);
					tt = 1.0 - t;
					ycont_merge[jmerge+1][i] = ycont_merge[jmerge+1][i]*t + yline_merge[jmerge+1][i]*tt;
					zcont_merge[jmerge+1][i] = zcont_merge[jmerge+1][i]*t + zline_merge[jmerge+1][i]*tt;
				} else {
					yline_merge[jmerge][i] = yline_main[jm-1][i];
					zline_merge[jmerge][i] = zline_main[jm-1][i];
					zcont_merge[jmerge][i] = zcont_main[jm-1][i];
					ycont_merge[jmerge][i] = ycont_main[jm-1][i];
				}
			}
			else {
				t = section_curve_intersection(
					yline_main   [jm][i],zline_main   [jm][i],a,hb,c,hd,
					yline_overlay[jv][i],zline_overlay[jv][i],av[i],hbv[i],cv[i],hdv[i]);
				if(t >= -0.0001) {
					tt = 1.0 - t;
					yline_merge[jmerge][i] = yline_overlay[jv][i] + t*(av[i] + t*hbv[i]);
					zline_merge[jmerge][i] = zline_overlay[jv][i] - t*(cv[i] + t*hdv[i]);
					zcont_merge[jmerge][i] = zcont_overlay[jv][i]*tt + zline_overlay[jv-1][i]*t;
					ycont_merge[jmerge][i] = ycont_overlay[jv][i]*tt + yline_overlay[jv-1][i]*t;
					t = tvalue(yline_merge[jmerge][i]-yline_merge[jmerge+1][i],
						zline_merge[jmerge+1][i]-zline_merge[jmerge][i],a,hb,c,hd);
					tt = 1.0 - t;
					ycont_merge[jmerge+1][i] = ycont_merge[jmerge+1][i]*t + yline_merge[jmerge+1][i]*tt;
					zcont_merge[jmerge+1][i] = zcont_merge[jmerge+1][i]*t + zline_merge[jmerge+1][i]*tt;
				} else {
					yline_merge[jmerge][i] = yline_main[0][i];
					zline_merge[jmerge][i] = zline_main[0][i];
					zcont_merge[jmerge][i] = zcont_main[0][i];
					ycont_merge[jmerge][i] = ycont_main[0][i];
				}
			}
			relcont_merge[jmerge] = FALSE;
			relcont_merge[jmerge+1] = FALSE;
		} else if(merge_down) {
			for(j = jmerge ; j < lines ; j++) {
				yline_merge[j][i] = yline_main[jm][i];
				zline_merge[j][i] = zline_main[jm][i];
				zcont_merge[j][i] = zline_main[jm][i];
				if(j == jmerge)
					ycont_merge[j][i] = ycont_main[jm][i];
				else
				    ycont_merge[j][i] = 0.0;
			}
		} else {
			for(j = 0 ; j < jmerge ; j++) {
				yline_merge[j][i] = yline_main[jm-1][i];
				zline_merge[j][i] = zline_main[jm-1][i];
				zcont_merge[j][i] = zline_main[jm-1][i];
				ycont_merge[j][i] = ycont_main[jm-1][i];
			}
		}
	}

	/*	Deallocate the main hull's memory	*/

	memfree(yline_main);
	memfree(ycont_main);
	memfree(zline_main);
	memfree(zcont_main);
	memfree(stsec_main);
	memfree(ensec_main);
	memfree(radstem_main);
	memfree(relcont_main);
	memfree(linewt_main);

	/*	Move the merge hull locations to the main hull pointers
	*/
	yline_main = yline_merge;
	ycont_main = ycont_merge;
	zline_main = zline_merge;
	zcont_main = zcont_merge;
	stsec_main = stsec_merge;
	ensec_main = ensec_merge;
	radstem_main = radstem_merge;
	autofair_main = autofair_merge;
	relcont_main = relcont_merge;
	linewt_main = linewt_merge;

	for(i = 0 ; i < count ; i++) {
		ycont_main[0][i] = 0.0;
		zcont_main[0][i] = 0.0;
	}
	for(j = 1 ; j <= jmerge ; j++) {
		ycont_main[j][0] = yline_main[j-1][0];
		zcont_main[j][0] = zline_main[j-1][0];
	}

	/*	Set merge pointers to NULL, to ensure that the originally
	allocated memory is no longer associated with them.
	*/
	yline_merge = NULL;
	ycont_merge = NULL;
	zline_merge = NULL;
	zcont_merge = NULL;
	stsec_merge = NULL;
	ensec_merge = NULL;
	radstem_merge = NULL;
	autofair_merge = NULL;
	relcont_merge = NULL;
	linewt_merge = NULL;

	j = lines - extlin_main;
	numlin_main +=j;
	stemli_main += j;
	extlin_main = lines;
	use_hull(MAINHULL);
	(void) realloc_hull(extlin); /* to ensure transom arrays are correct */

#ifdef PROF
	if(merge_down) {
		if(new_overlay_start >= 0) stsec[stemli] = new_overlay_start;
		if(new_overlay_end   >= 0) ensec[stemli] = new_overlay_end;
	}
#else
	for(j = 0 ; j < extlin ; j++) {
		if(new_overlay_start > 0 && new_overlay_end > 0) {
			for(i = 0 ; i < count ; i++) {
				yline[j][i] = 0.0;
				zline[j][i] = zline[numlin-1][i];
				ycont[j][i] = 0.0;
				zcont[j][i] = zline[j][i];
				if(i == new_overlay_start-1) i = new_overlay_end;
			}
		}
		stsec[j] = 0;
		ensec[j] = count-1;
	}
	numbetw = 0;
#endif

#ifndef STUDENT
	redef_transom();
	recalc_tanks();
#endif
	surfacemode = 0;
	save_hull(MAINHULL);
}

/*	Solve for t1 and t2 the two equations

y1 + a1*t1 + hb1*t1^2 = y2 + a2*t2 + hb2*t2^2
z1 - c1*t1 - hd1*t1^2 = z2 - c2*t2 - hb2*c2^2

and return the solutions for t2.
*/
REAL section_curve_intersection(
	REAL y1,REAL z1,REAL a1,REAL hb1,REAL c1,REAL hd1,
	REAL y2,REAL z2,REAL a2,REAL hb2,REAL c2,REAL hd2)
{
	REAL A2,C2,A1,C1,A,B,C;
	REAL t1,t2,t2a,t2b,t1a,t1b,t2start;
	REAL b1 = 2.0*hb1;
	REAL b2 = 2.0*hb2;
	REAL d1 = 2.0*hd1;
	REAL d2 = 2.0*hd2;
	REAL det;
	int count = 0;
	REAL t10 = 0.1,t20 = 0.1;

	t2 = 0.5 /* t20 */ ;
	do {
		t2start = t2;
		A2 = a2 + b2*t2;
		C2 = c2 + d2*t2;
		A = hb1*C2 - hd1*A2;
		B = a1 *C2 - c1 *A2;
		C = A2*(z1-z2) + C2*(y1 - y2) + t2*(c2*A2 - a2*C2 + t2*(hd2*A2 - hb2*C2));
		if(A != 0.0) {
			det = B*B - 4.0*A*C;
			if(det >= 0.0) {
				det = sqrt(det);
				t1a = ( det - B)/(2.0*A);
				t1b = (-det - B)/(2.0*A);
				if(fabs(t1a - 0.5) < fabs(t1b - 0.5))
					t1 = t1a;
				else
					t1 = t1b;
			}
			else {
				t10 += 0.1;
				t1 = t10;
			}
		}
		else if(B != 0.0) {
			t1 = -C / B;
		}
		else {
			t10 += 0.1;
			t1 = t10;
		}
		if(t1 < -0.1 || t1 > 1.1) t1 = 0.5;

		A1 = a1 + b1*t1;
		C1 = c1 + d1*t1;
		A = hb2*C1 - hd2*A1;
		B = a2 *C1 - c2 *A1;
		C = A1*(z2-z1) + C1*(y2 - y1) + t1*(c1*A1 - a1*C1 + t1*(hd1*A1 - hb1*C1));
		if(A != 0.0) {
			det = B*B - 4.0*A*C;
			if(det >= 0.0) {
				det = sqrt(det);
				t2a = ( det - B)/(2.0*A);
				t2b = (-det - B)/(2.0*A);
				if(fabs(t2a - 0.5) < fabs(t2b - 0.5))
					t2 = t2a;
				else
					t2 = t2b;
			}
			else {
				t2 = t20;
				t20 += 0.1;
			}
		}
		else if(B != 0.0) {
			t2 = -C / B;
		}
		else {
			t2 = t20;
			t20 += 0.1;
		}
		if(t2 < -0.1 || t2 > 1.1) t2 = 0.5;
	}
	while(fabs(t2 - t2start) > 0.0001 && count++ < 10) ;

	if(count >= 10)
		return -1.0;
	else
	    return t2;
}

/*	Merged surface set manipulations	*/

extern char (*surfname)[MAX_PATH];
extern int *surfline,*mainline,*useovl,*mergedn;
char surffile[MAX_PATH] = "";
void update_picklist(char * line,char *dirnam);
void surface_functions(int,HWND);
void change_surface_entry(int,HWND);

struct {
	int index;       /* index of result in table */
	char *string;    /* pointer to result */
	char *table[9];  /* table of strings for listbox, null string terminator */
}
surflist;

extern char *merge_hullfile;

void surfaces()
{
	extern int surfaceset;
	char dummy[256] = "";
	int i;

	/*	Build the list box contents	*/

	i = 0;
#ifndef linux
	strlwr(merge_hullfile);
#endif
	do {
		surflist.table[i] = surfname[i];
	}
	while(surfname[i++][0] != 0);
	surflist.index = 0;
	surflist.string = dummy;
	cls(0);

	/*	Configure the surface set	*/

	(void) getdlg(SURFACE,
		0,(void *) merge_hullfile,
		INP_LBX,(void *) &surflist,
		INP_PBF,surface_functions,-1);
}

void surface_functions(int button,HWND hWndDlg)
{
	int is,i,j;
	char start[MAX_PATH];
	FILE *fp;
	int result;
	char *p,*q;
	void update_picklist(char *hullfile,char *dirnam);
	void load_view(void);
	HDC HDCitem;
	int offset;
	SIZE size;
#ifdef linux
	XmString xstr;
	Widget w;
	extern Widget wListBox[],wLabel[];
	extern int force_proceed;
#endif

	void new_surface_set(void),open_surface_set(void),save_surface_set(void),
	save_surface_set_as(void),merge_all(void);

	switch(button) {
	case 0:	/* new */
		new_surface_set();
		break;
	case 1:	/* open */
		open_surface_set();
		i = 0;
#ifdef linux
fill_list:
		w = wListBox[0];
		XmListDeleteAllItems(w);
		do {
			surflist.table[i] = surfname[i];
			xstr = XmStringCreateSimple(surfname[i]);
			XmListAddItem(w,xstr,0);
			XmStringFree(xstr);
		}
		while(surfname[i++][0] != 0);
		xstr = XmStringCreateSimple(merge_hullfile);
		XtVaSetValues(wLabel[0],XmNlabelString,xstr,NULL);
		XmStringFree(xstr);
#else
		SendDlgItemMessage(hWndDlg,DLGLBX,LB_RESETCONTENT,0,0l);
		while(surfname[i][0] != 0) {
			surflist.table[i] = surfname[i];
			HDCitem = GetDC(GetDlgItem(hWndDlg,DLGLBX));
			offset = -1;
			strcpy(start,surfname[i]);
			do {
				offset ++;
				GetTextExtentPoint(HDCitem,start + offset,strlen(start) - offset,&size);
			} while(size.cx > 400);
			ReleaseDC(hWndDlg,HDCitem);
			if(offset >= 4) {
				strncpy(start + offset - 4,"... ",4);
				SendDlgItemMessage(hWndDlg,DLGLBX,LB_ADDSTRING,0,(LPARAM) (LPCSTR) start + offset - 4);
			} else {
				SendDlgItemMessage(hWndDlg,DLGLBX,LB_ADDSTRING,0,(LPARAM) (LPCSTR) start);
			}
			i++;
		}
		SendDlgItemMessage(hWndDlg,DLGLBX,LB_SETCURSEL,0,0L);
		strlwr(merge_hullfile);
		HDCitem = GetDC(GetDlgItem(hWndDlg,DLGSTAT));
		offset = -1;
		do {
			offset ++;
			GetTextExtentPoint(HDCitem,merge_hullfile + offset,strlen(merge_hullfile) - offset,&size);
		} while(size.cx > 330);
		ReleaseDC(hWndDlg,HDCitem);
		if(offset > 0) {
			strcpy(start,"... ");
			strcpy(start+4,merge_hullfile + offset + 4);
			SetDlgItemText(hWndDlg,DLGSTAT,start);
		} else {
			SetDlgItemText(hWndDlg,DLGSTAT,merge_hullfile);
		}
#endif
		break;
	case 2:	/* save */
		save_surface_set();
		break;
	case 3:	/* save as ... */
		save_surface_set_as();
		break;
	case 4:	/* Find main hull file */
		if(openfile(merge_hullfile,"rt","Open a hull data file",
			"hull data files(*.HUD)\0*.hud\0","*.hud",dirnam,&fp)) {
			strlwr(merge_hullfile);
			fclose(fp);
#ifdef linux
			xstr = XmStringCreateSimple(merge_hullfile);
			XtVaSetValues(wLabel[0],XmNlabelString,xstr,NULL);
			XmStringFree(xstr);
#else
			SetDlgItemText(hWndDlg,DLGSTAT+0,merge_hullfile);
#endif
		}
		break;

	case 5:	/* Open main hull file */
		if(*merge_hullfile != 0) {
			strcpy(hullfile,merge_hullfile);
			if((fp = fopen(hullfile,"r")) != NULL) {
				read_hullfile(fp,hullfile,TRUE);
				if(count > 0) {
					update_picklist(hullfile,dirnam);
					load_view();
#ifdef linux
					xstr = XmStringCreateSimple(merge_hullfile);
					XtVaSetValues(wLabel[0],XmNlabelString,xstr,NULL);
					XmStringFree(xstr);
					force_proceed = 1;
#else
					SetDlgItemText(hWndDlg,DLGSTAT+0,hullfile);
					SendMessage(hWndDlg,IDOK,0,0L);		       /* Exit to edit */
					EndDialog(hWndDlg,TRUE);
#endif
				}
				else {
					message("Hull data file invalid");
				}
			}
			else {
				message("Could not find main hull to edit it");
			}
		}
		else {
			message("No main hull defined");
		}
		break;

	case 6:	/* Move selection up */
	case 7:	/* Move selection down */

#ifdef linux
		/*	"i" is index of selection, "j" is number of entries	*/
		XmListGetSelectedPos(wListBox[0],&i,&is);
		XtVaGetValues(wListBox[0],XmNitemCount,&j,NULL);

		/*	If the selection is successfully retrieved, delete it,
		change the index as requested, and insert it		*/
		if(i >= 1 && i <= j) {
			if(button == 7) {
				if(i == 1) break;
				j = i-1;
			}
			else {
				if(i == j) break;
				j = i+1;
			}
			strcpy(surfname[i],surfname[j]);
			strcpy(surfname[j],start);
			strcpy(start,surfname[j]);
			goto fill_list;
		}
#else
		/*	"i" is index of selection, "j" is number of entries	*/
		i = SendDlgItemMessage(hWndDlg,DLGLBX,LB_GETCURSEL,0,0l);
		j = SendDlgItemMessage(hWndDlg,DLGLBX,LB_GETCOUNT,0,0l);

		/*	If the selection is successfully retrieved, delete it,
		change the index as requested, and insert it		*/
		if(i >= 0 && i < j && SendDlgItemMessage(hWndDlg,
			DLGLBX,LB_GETTEXT,i,(LPARAM) (LPCSTR) start) > 0) {
			if(button == 7) {
				if(i == 0) break;
				j = i-1;
			}
			else {
				if(i == j-1) break;
				j = i+1;
			}
			strcpy(surfname[i],surfname[j]);
			strcpy(surfname[j],start);
			SendDlgItemMessage(hWndDlg,DLGLBX,LB_DELETESTRING,i,0);

			HDCitem = GetDC(GetDlgItem(hWndDlg,DLGLBX));
			offset = -1;
			do {
				offset ++;
				GetTextExtentPoint(HDCitem,start + offset,strlen(start) - offset,&size);
			} while(size.cx > 400);
			ReleaseDC(hWndDlg,HDCitem);
			if(offset > 0) {
				strncpy(start + offset - 4,"... ",4);
				SendDlgItemMessage(hWndDlg,DLGLBX,LB_INSERTSTRING,j,(LPARAM) (LPCSTR) start - 4);
			} else {
				SendDlgItemMessage(hWndDlg,DLGLBX,LB_INSERTSTRING,j,(LPARAM) (LPCSTR) start);
			}
			SendDlgItemMessage(hWndDlg,DLGLBX,LB_SETCURSEL,j,0l);
		}
#endif
		break;

	case 8:	/* Add file */
		is = 0;
		while(surfname[is][0] != 0) is++;
		if(is >= 8) {
			message("Surface limit reached");
			break;
		}
		if(!openfile(surfname[is],"rt",
			"Find a Hull Data file",
			"Hull surface set files (*.hud)\0*.hud\0","*.hud",
			dirnam,NULL)) {
			*surfname[is] = 0;
			break;
		}
#ifdef linux
		xstr = XmStringCreateSimple(surfname[is]);
		XmListAddItem(wListBox[0],xstr,is);
		XmListSelectItem(wListBox[0],xstr,False);
		XmStringFree(xstr);
#else
		strlwr(surfname[is]);
		HDCitem = GetDC(GetDlgItem(hWndDlg,DLGLBX));
		offset = -1;
		strcpy(start,surfname[is]);
		do {
			offset ++;
			GetTextExtentPoint(HDCitem,start + offset,strlen(start) - offset,&size);
		} while(size.cx > 400);
		ReleaseDC(hWndDlg,HDCitem);
		if(offset > 0) {
			strncpy(start + offset - 4,"... ",4);
			SendDlgItemMessage(hWndDlg,DLGLBX,LB_ADDSTRING,0,(LPARAM) (LPCSTR) start - 4);
		} else {
			SendDlgItemMessage(hWndDlg,DLGLBX,LB_ADDSTRING,0,(LPARAM) (LPCSTR) start);
		}
		SendDlgItemMessage(hWndDlg,DLGLBX,LB_SETCURSEL,is,0);
#endif
		surfline[is] = 2;
		mainline[is] = 2;
		useovl[is]   = TRUE;
		mergedn[is] = FALSE;
		goto edit;

	case 9:	/* Remove selection */
#ifndef linux
		SendDlgItemMessage(hWndDlg,DLGLBX,LB_DELETESTRING,surflist.index,0l);
		is = surflist.index;
#endif
		if(is >= 0) {
			do{
				strcpy(surfname[is],surfname[is+1]);
				surfline[is] = surfline[is+1];
				mainline[is] = mainline[is+1];
				useovl[is]   = useovl[is+1];
				mergedn[is]  = mergedn[is+1];
				is++;
			}
			while(surfname[is][0] != 0);
		}
#ifdef linux
		goto fill_list;
#else
		break;
#endif

	case 10:	/* merging parameters */
		is = surflist.index;
		if(is >= 0) {
edit:
			if(getdlg(MERGEOVL,
				INP_INT,(void *) &surfline[is],
				INP_INT,(void *) &mainline[is],
				INP_LOG,(void *) &useovl[is],
				INP_LOG,(void *) &mergedn[is],-1)) {
				if(surfline[is] <= 0 || mainline[is] <= 0) {
					message("Invalid options");
					goto edit;
				}
			}
		}
		break;

	case 11:	/* Edit selection */
		is = surflist.index;
		if(is >= 0) {
			result = (fp = fopen(surfname[is],"r")) != NULL;
			if(!result) {
				message("THIS FILE NOT ACCESSIBLE");
			}
			else {
				strcpy(hullfile,surfname[is]);
				strcpy(dirnam,surfname[is]);
				q = dirnam-1;
				do {
					p = ++q;
					q = strchr(p,'\\');
				}
				while(q != NULL);
				if(p == dirnam)
					strcpy(dirnam,".");
				else
				    *--p = 0;
				read_hullfile(fp,hullfile,FALSE);
				if(count > 0) update_picklist(hullfile,dirnam);
				load_view();
#ifdef linux
				force_proceed = 1;
#else
				PostMessage(hWndDlg,WM_COMMAND,IDOK,0l);
#endif
			}
		}
		break;

	case 12:	/* merge */
		strcpy(hullfile,merge_hullfile);
		merge_all();
		break;
	}
}


void new_surface_set()
{
#ifdef linux
#else
	extern HWND hWndMain;
#endif
	surfname[0][0] = '\0';
	numlin = 1;
	extlin = 1;
	use_hull(OVERLAYHULL);
	realloc_hull(extlin);

	use_hull(MAINHULL);
	realloc_hull(extlin);

	count = 0;
	strcpy(hullfile,"(undef)");
#ifdef linux

#else

#ifdef HULLSTAT
	SetWindowText(hWndMain,"HULLFORM+HULLSTAT - (undef)");
#else
	SetWindowText(hWndMain,"HULLFORM - (undef)");
#endif

#endif
}

void open_surface_set()
{
	void do_merge(int jv,int jm,int use_overlay,int merge_down);
	void use_hull(int);
	char line[MAX_PATH];
	char buf[MAX_PATH + 20];
	FILE *fsurf;
	FILE *fp;
	int jv,jm;
	extern int surfaceset;
	int ns = 0;
	int use_overlay;
	int merge_down = FALSE;
	char *p;

	/*	Open the surface file	*/

	if(!openfile(surffile,"rt","Open a hull surface set",
		"hull surface files(*.hss)\0*.hss\0","*.hss",dirnam,&fsurf))
		return;

	/*	Read the name of the primary hull design	 */
	if(fgets(line+18,sizeof(line)-18,fsurf) == NULL) {
		message("Hull surface file is empty");
		fclose(fsurf);
		return;
	}
	if((p = strchr(line+18,'\n')) != NULL) *p = 0;

	/*	Try to open the primary hull file		*/
	if((fp = fopen(line+18,"rt")) == NULL) {
		strncpy(line,"Can not open file ",18);
		message(line);
		fclose(fsurf);
		return;
	}

	/*	Read the primary hull file			*/
	strcpy(merge_hullfile,line+18);
	read_hullfile(fp,merge_hullfile,TRUE);
	if(count > 0) update_picklist(merge_hullfile,dirnam);


	while(fgets(buf,sizeof(buf),fsurf)) {
		p = strstr(buf,".hud ");
		if(p == NULL) {
			message("Invalid surface file");
			break;
		}
		*(p + 4) = 0;
		if(sscanf(p+5,"%d %d %d %d\n",&jm,&jv,&use_overlay,&merge_down) >= 4) {
			if((fp = fopen(buf,"rt")) == NULL) {
				strncpy(line,"Can not open file ",18);
				message(line);
				fclose(fsurf);
				return;
			}
			surfline[ns] = jv;
			mainline[ns] = jm;
			useovl[ns] = use_overlay;
			mergedn[ns] = merge_down;
			strcpy(surfname[ns++],buf);
		}
	}
	fclose(fsurf);
	surfaceset = TRUE;
	surfname[ns][0] = 0;
}

void do_save_surface_set(int save_as)
{
	int is;
	FILE *fp;

	if(save_as || *surffile == 0) {
		if(!openfile(surffile,"wt","Open a hull surface set for output",
			"hull surface files(*.hss)\0*.hss\0","*.hss",
			dirnam,&fp)) return;
	}

	if((fp = fopen(surffile,"wt")) != NULL) {
		fprintf(fp,"%s\n",merge_hullfile);
		is = 0;
		while(surfname[is][0] != 0) {
			fprintf(fp,"%s %d %d %d %d\n",surfname[is],
				mainline[is],surfline[is],useovl[is],mergedn[is]);
			is++;
		}
		fclose(fp);
	}
}

void save_surface_set()
{
	do_save_surface_set(FALSE);
}

void save_surface_set_as()
{
	do_save_surface_set(TRUE);
}

void merge_all()
{
	int is = 0;
	FILE *fp;
	char line[280];
	extern HWND hWndMain;

	if((fp = fopen(hullfile,"r")) == NULL) {
		sprintf(line,"Can not open file %s",hullfile);
		message(line);
		return;
	}
	use_hull(MAINHULL);
	read_hullfile(fp,hullfile,FALSE);

	while(surfname[is][0] != 0) {
		if((fp = fopen(surfname[is],"rt")) == NULL) {
			strncpy(line,"Can not open file ",18);
			message(line);
			return;
		}
		use_hull(OVERLAYHULL);
		read_hullfile(fp,surfname[is],FALSE);
		if(count > 0) update_picklist((char *) surfname[is],dirnam);
		use_hull(MAINHULL);
		do_merge(surfline[is],mainline[is],useovl[is],mergedn[is]);
		surfname[is++][0] = 0;
	}
	*hullfile = 0;
	*merge_hullfile = 0;
#ifdef linux

#else

#ifdef HULLSTAT
	SetWindowText(hWndMain,"HULLSTAT");
#else
	SetWindowText(hWndMain,"HULLFORM");
#endif

#endif
}


#endif
