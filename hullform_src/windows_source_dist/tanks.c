/* Hullform component - tanks.c
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
 
#undef DEBUG

#ifdef PROF

#include "hulldesi.h"

extern int changed;
void recalc_tanks(void);
void pjust(REAL value);
void calctankstat(void);
MENUFUNC tank_calibration(int);
char *notanks = "No tanks defined";
int shape = 0;
void save_hull(int);

int tankloc = 2;	/* default both sides */
REAL toplevel = 0.0;
REAL baselevel = 9999.0;
REAL insideoffset = 0.0;
REAL outsideoffset = 9999.0;
REAL skincorr = 0.0;
REAL horizontal_intersection(int i,int j,REAL z,REAL skincorr,REAL *al,REAL *cl,REAL *a,REAL *b,REAL *c,REAL *d,REAL *yret,REAL *zret);
REAL vertical_intersection(int i,int j,REAL y,REAL skincorr,REAL *al,REAL *cl,REAL *a,REAL *b,REAL *c,REAL *d);
int  side_offset(int i,int j,int firstline,REAL tanktop     ,REAL baselevel    ,REAL sideoffset,REAL skincorr,REAL *al,REAL *cl,REAL *y,REAL *z,REAL *yc,REAL *zc);
int base_top_offset(int i,int j,int firstline,REAL insideoffset,REAL outsideoffset,REAL baselevel ,REAL skincorr,REAL *al,REAL *cl,REAL *y,REAL *z,REAL *yc,REAL *zc);
int jtank;
char *fixto7(REAL value);

void inc_jtank(void)
{
	int i;
	jtank++;
	if(jtank >= extlin) {
		realloc_hull(jtank+2);
		extlin = jtank+1;
		for(i = 0 ; i < count ; i++) yline[jtank][i] = -1.0;
	}
}

void add_corner_line(REAL yvalue,REAL zvalue,int i,int *corner_line_index,int first_section,int first_line)
{
	int ii,jj,j1,extra_lines,prev_extlin;
	REAL y,z;

	/*	If this corner has been inserted previously, add lines until the current line matches the previously-defined line.	*/
	/*	If it has not been inserted previously, or it was inserted at a lower-indexed line, just add the one set.			*/

	do {
		inc_jtank();
		yline[jtank][i] = yvalue;
		zline[jtank][i] = zvalue;
		ycont[jtank][i] = yvalue;
		zcont[jtank][i] = zvalue;
	}
	while(jtank < *corner_line_index);

	/*	If the offsets have been entered at a higher index than previously, add offsets to all earlier sections, to ensure	*/
	/*	the line indices match																								*/

	if(i > first_section && jtank > *corner_line_index) {	/* missing line intersections previously - add ones to all previous sections, matching the first offsets of each section  */

		/*	Add extra hull lines, corresponding to the additional lines required	*/

		if(*corner_line_index < 0)
			extra_lines = 1;							/* not previously defined - add one extra line for it			*/
		else
			extra_lines = jtank - *corner_line_index;	/* previously defined - add lines from previous to current		*/
		prev_extlin = extlin;
		realloc_hull(extlin+extra_lines + 1);
		extlin += extra_lines;

		for(jj = prev_extlin ; jj < extlin ; jj++) {	/* mark the new lines as undefined */
			for(ii = 0 ; ii < count ; ii++) yline[jj][ii] = -1.0;
		}

		if(*corner_line_index < first_line)
			j1 = jtank;
		else
			j1 = *corner_line_index;

		for(ii = first_section ; ii < i ; ii++) {

			/*	Preserve the offsets for the line which is to be duplicated in the generated gap	*/

			y = yline[j1][ii];
			z = zline[j1][ii];

			/*	For each preceding tank section, move existing lines to higher indices, with line "*corner_line_index" going to line jtank	*/

			for(jj = extlin-1 ; jj >= j1 ; jj--) {
				yline[jj+extra_lines][ii] = yline[jj][ii];
				zline[jj+extra_lines][ii] = zline[jj][ii];
				ycont[jj+extra_lines][ii] = ycont[jj][ii];
				zcont[jj+extra_lines][ii] = zcont[jj][ii];
			}

			/*	Insert the preserved offsets into the gap	*/

			for(jj = j1 ; jj < jtank ; jj++) {
				yline[jj][ii] = y;
				zline[jj][ii] = z;
				ycont[jj][ii] = y;
				zcont[jj][ii] = z;
			}
		}
	}
	*corner_line_index = jtank;
}

void add_tank_lines(int *cutindex,int inisec,int cursec,int *prev_lines)
{
	int i,j,jd;
	while(jtank < *cutindex) {
		inc_jtank();
		yline[jtank][cursec] = yline[jtank-1][cursec];
		zline[jtank][cursec] = zline[jtank-1][cursec];
		ycont[jtank][cursec] = yline[jtank-1][cursec];
		zcont[jtank][cursec] = zline[jtank-1][cursec];
	}
	*cutindex = jtank;
}

/****************************************************************/

/*	Floodability analysis section				*/

void tankedit(int code,HWND hWndDlg);

void edit_tank_options(int code,HWND hWndDlg)
{
	int i,l,n;
	int *pos;
	char line[200];
	MENUFUNC add_tank(void);
#ifdef linux
	XmString *xstrtab,xstr;
	extern Widget wListBox[];
#endif

	switch(code) {
	case 0:		// add tank
		n = ntank;
		add_tank();
		if(ntank > n) {
#ifdef linux
			xstr = XmStringCreateSimple(tankdesc[n]);
			XmListAddItem(wListBox[0],xstr,0);
			XmStringFree(xstr);
#else
			SendDlgItemMessage(hWndDlg,DLGLBX + 0,LB_ADDSTRING,0,(LPARAM) (LPCSTR) tankdesc[n]);
			SendDlgItemMessage(hWndDlg,DLGLBX + 0,LB_SETCURSEL,n,0L);
#endif
		}
		break;
	case 1:		// delete tank
#ifdef linux

		XmListGetSelectedPos(wListBox[0],(int **) &pos,(int *) &n);
		n = (n > 0 ? *pos : 0) - 1;
		XtFree((char *) pos);
		if(n >= 0)
#else
		if( (n = SendDlgItemMessage(hWndDlg,DLGLBX + 0,LB_GETCURSEL,0,0)) != LB_ERR)
#endif
		{
			i = fl_line1[n];
			l = fl_line1[n+1] - fl_line1[n];
			sprintf(line,
				"Tank %s, number %d, has %d lines, and extends\nfrom section %d to section %d.\n\nIs it the one you want to delete?",
				tankdesc[n],n+1,l,stsec[i],ensec[i]);
			if(MessageBox(hWnd,line,"Confirm Tank Deletion",MB_YESNO) == IDYES) {
				ntank--;
				extlin -= l;
				for(i = fl_line1[n] ; i < extlin ; i++) copyline(i,i+l,0.0);
				for(i = n ; i < ntank ; i++) {
					fl_line1[i] = fl_line1[i+1] - l;
					strcpy(tankdesc[i],tankdesc[i+1]);
					fl_fract[i] = fl_fract[i+1];
					fl_spgra[i] = fl_spgra[i+1];
					fl_right[i] = fl_right[i+1];
					fl_fixed[i] = fl_fixed[i+1];
					fl_perm[i]  = fl_perm[i+1];
				}
				fl_line1[ntank] = extlin;
				changed = 1;
				recalc_transom();
				recalc_tanks();
#ifdef linux
				XmListDeleteItemsPos(wListBox[0],1,n+1);
#else
				SendDlgItemMessage(hWndDlg,DLGLBX + 0,LB_DELETESTRING,n,0);
#endif
				tank = n;
			}
		}
		break;
	case 2:		// edit tank
#ifdef linux
		XmListGetSelectedPos(wListBox[0],&i,&n);
		if(n == 1) {
			tank = i+1;
			tankedit(code,hWndDlg);
		}
#else
		if( (n = SendDlgItemMessage(hWndDlg,DLGLBX + 0,LB_GETCURSEL,0,0)) != LB_ERR) {
			tank = n+1;
			tankedit(code,hWndDlg);
		}
#endif
		break;
	}
}

MENUFUNC edit_tanks(void)
{
	int i,j,l;
	char spgr[10],contents[30];
	REAL cont,perc,perm;
	struct {
		int index;					/* index of result in table (not used) */
		char *string;				/* pointer to result (not used) */
		char *table[MAXTANKS+1];	/* table of strings for listbox, null string terminator */
	}
	xlist;
	char text[MAX_PATH] = "";

	for(i = 0 ; i < ntank ; i++) {
		xlist.table[i] = tankdesc[i];
	}
	xlist.table[i] = "";
	xlist.index = -1;
	xlist.string = text;
	(void) getdlg(EDITTANKS,
	INP_LBX,(void *) &xlist,
	INP_PBF,(void *) edit_tank_options,
	-1);
}

/*	add a tank to the tank table				*/

#define NONE	0
#define	BOTTOM	1
#define	OUTSIDE	2
#define	TOP		3
#define	INSIDE	4

MENUFUNC add_tank(void)
{
	int lines;		/* number of lines for defined tank	*/
	int i,j,k;		/* section and line indices		*/
	int sec1,sec2;		/* start and end sections		*/
	int success;		/* function success code		*/
	REAL yc,zc,rad,phase,dphase;
	char *ovf = "Too many lines generated. Tank not added.";
	char *noroom = "All tank definition space used.";
	char *notallowed = "Out of allowed section range";
	char text[MAX_PATH];
	REAL depth = 0.0,width = 0.0,inside = 0.0,base = 0.0;
	static int mirror_value = 1;
	int mirror;

	REAL tanktop,dz,t1,t2,len,al,cl,ap,cp,an,hbn,cn,hdn,ah,ch;
	REAL t;
	REAL aa,cc,a,hb,c,hd,b,d,yref,zref;
	int jprev,firstline,prev_lines,jj,jx;
	int normalise(REAL a,REAL c,REAL *al,REAL *cl,REAL *len);
	int vertical_cut;
	int was_left,was_right,was_above,was_below;
	int is_left,is_right,is_above,is_below;
	int now_inside,ever_inside;
	REAL yl,zl;
	int inside_top_line,inside_bottom_line,outside_top_line,outside_bottom_line;
	int topleftindex,bottomleftindex,bottomrightindex,toprightindex;
	int topcutindex[8],bottomcutindex[8],leftcutindex[8],rightcutindex[9];	// allow up to 8 cuts of each tank surface
	int topcutcount,bottomcutcount,leftcutcount,rightcutcount;
	void save_hull(int);
	REAL topval,rightval;
	int last_line;
	int last_intersection;
	int first_intersection;

	if(ntank == 0) extlin = numlin;

	if(ntank >= MAXTANKS) {
		message(noroom);
		return;
	}

	sec1 = 1;
	sec2 = count-1;
	lines = 4;
	sprintf(text,"TANK %d",ntank+1);
	t1 = invert * baselevel;

#ifdef DEBUG
	sec1=7;
	sec2=9;
	shape = 3;
	insideoffset = 0.65;
	tankloc = 2;
	toplevel = 0.5;
#endif

	if(getdlg(EDITTANK,
		INP_INT,(void *) &sec1,
		INP_INT,(void *) &sec2,
		INP_STR,(void *) text,
		INP_RBN,(void *) &shape,
		INP_INT,(void *) &lines,
		INP_REA,(void *) &width,
		INP_REA,(void *) &depth,
		INP_REA,(void *) &inside,
		INP_REA,(void *) &base,
		INP_INT,(void *) &mirror_value,
		INP_RBN,(void *) &tankloc,
		INP_REA,(void *) &toplevel,
		INP_REA,(void *) &insideoffset,
		INP_REA,(void *) &t1,
		INP_REA,(void *) &outsideoffset,
		INP_REA,(void *) &skincorr,
	-1)) {
		if(		sec1 <= 0 || sec1 >= count-1 ||
			    sec2 <= sec1 || sec2 >= count ||
			    extlin + lines > maxlin) {
			message(notallowed);
			success = 0;
		}
		else {
			success = 1;
		}
	}
	else {
		success = 0;
	}

	if(success) {
		fl_line1[ntank] = extlin;/* starts at extra line count	*/
		firstline = extlin;
		mirror = mirror_value - 1;
		switch(shape) {
		case 0:
			extlin += lines;
			break;
		case 1:
			extlin += 4;
			break;
		case 2:
			if(mirror < 0 || mirror >= ntank) {
				message("Invalid index for mirrored tank");
				return;
			}
			extlin += fl_line1[mirror+1] - fl_line1[mirror];
			break;
		}
		/* extends extra line count	*/
		fl_line1[ntank+1] = extlin;
		fl_right[ntank] = 1;	/* default right side	*/
		fl_fixed[ntank] = 1;	/* default is "fixed" */
		fl_fract[ntank] = 100.0;
		fl_perm[ntank] = 1.0;
		fl_spgra[ntank] = 1.0;
		fl_walev[ntank] = 0.0;
		*(text+79) = 0;
		strcpy(tankdesc[ntank],text);
		if(no_expand()) {
			message(ovf);
			extlin -= lines;
			success = 0;
		}
		else {
			for(j = fl_line1[ntank] ; j < extlin ; j++) {
				stsec[j] = sec1;
				ensec[j] = sec2;
				relcont[j] = FALSE;
				for(i = 0 ; i < maxsec+2 ; i++) {
					if(i == count) i = maxsec;
					yline[j][i] = 0.0;
					zline[j][i] = 0.0;
					ycont[j][i] = 0.0;
					zcont[j][i] = 0.0;
					linewt[j][i] = 1.0;

				}
			}
			if(shape == 1) {
				for(i = sec1 ; i <= sec2 ; i++) {
					j = fl_line1[ntank];
					yline[j  ][i] = inside;
					yline[j+1][i] = inside + width;
					yline[j+2][i] = inside + width;
					yline[j+3][i] = inside;
					zline[j  ][i] = -base;
					zline[j+1][i] = -base;
					zline[j+2][i] = -(base + depth);
					zline[j+3][i] = -(base + depth);
					ycont[j  ][i] = yline[j+3][i];
					ycont[j+1][i] = yline[j  ][i];
					ycont[j+2][i] = yline[j+1][i];
					ycont[j+3][i] = yline[j+2][i];
					zcont[j  ][i] = zline[j+3][i];
					zcont[j+1][i] = zline[j  ][i];
					zcont[j+2][i] = zline[j+1][i];
					zcont[j+3][i] = zline[j+2][i];
				}
			}
			else if(shape == 0) {
				dphase = 6.2831854 / ((REAL) lines);
				for(i = sec1 ; i <= sec2 ; i++) {
					yc = yline[0][i] * 0.5;
					zc =  (zline[numlin - 1][i] + zline[0][i]) * 0.5;
					rad = (zline[numlin - 1][i] - zline[0][i]) * 0.25;
					if(rad < 0.1) rad = 0.1;
					phase = 0.5 * dphase;
					k = fl_line1[tank+1]-1;
					for(j = fl_line1[ntank] ; j <= k ; j++) {
						yline[j][i] = yc - rad * cos(phase);
						zline[j][i] = zc + rad * sin(phase);
						if(j != fl_line1[tank]) {
							ycont[j][i] = yline[j-1][i];
							zcont[j][i] = zline[j-1][i];
						}
						phase += dphase;
					}
					j = fl_line1[tank];
					ycont[j][i] = yline[k][i];
					zcont[j][i] = zline[k][i];
				}
			}
			else if(shape == 2) {
				for(j = fl_line1[ntank], k = fl_line1[mirror] ; j < extlin ; j++, k++) {
					stsec[j] = stsec[k];
					ensec[j] = ensec[k];
					for(i = stsec[j] ; i <= ensec[j] ; i++) {
						yline[j][i] = yline[k][i];
						zline[j][i] = zline[k][i];
						ycont[j][i] = ycont[k][i];
						zcont[j][i] = zcont[k][i];
						linewt[j][i] = linewt[k][i];
					}
					relcont[j] = relcont[k];
					radstem[j] = radstem[k];
				}
				fl_right[ntank] = !fl_right[mirror];
				fl_fixed[ntank] = fl_fixed[mirror];
				fl_perm[ntank]  = fl_perm[mirror];
				fl_spgra[ntank] = fl_spgra[mirror];
				fl_line1[ntank+1] = j;
			}
			else {	/* shape == 3 */
				tanktop = invert * toplevel;
				baselevel = invert * t1;
				prev_lines = -1;
				inside_top_line = inside_bottom_line = outside_top_line = outside_bottom_line = -1;
				topleftindex = bottomleftindex = bottomrightindex = toprightindex = -1;	/* indices of lines previously marking corners of tank */
				for(i = 0 ; i < 8 ; i++) {
					topcutindex[i] = bottomcutindex[i] = leftcutindex[i] = rightcutindex[i] = -1;		/* indicates of lines cutting faces of tank */
				}
				if(tankloc == 2) insideoffset = 0.0;	/* if a tank extends across the hull, the central offset must be on the axis */
				for(i = sec1 ; i <= sec2 ; i++) {
					last_intersection = NONE;
					first_intersection = NONE;
					topcutcount = bottomcutcount = leftcutcount = rightcutcount = 0;	// counters of cuts on each tank face
					jtank = firstline-1;
					jprev = numlin-1;
					if(stsec[jprev] <= i && ensec[jprev] >= i) {
						yl = yline[jprev][i];
						zl = zline[jprev][i];
						if(yl > 0.0) {	// offset keel line
							do {
								add_corner_line(insideoffset,tanktop  ,i,&inside_top_line   ,sec1,firstline);
							} while(jtank < topleftindex);
							topleftindex = jtank;
							inc_jtank();
							yline[jtank][i] = 0.0;
							zline[jtank][i] = zl - skincorr;
							ycont[jtank][i] = 0.0;
							zcont[jtank][i] = zl - skincorr;
							al = 1.0;
							cl = 0.0;
							ap = 1.0;
							cp = 0.0;
						} else {
							getparam(i,jprev,&a,&hb,&c,&hd);
							al = a;
							cl = c;
							ap = a+2.0*hb;
							cp = c+2.0*hd;
						}
					} else {
						al = 1.0;
						cl = 0.0;
						ap = 1.0;
						cp = 0.0;
					}
					was_left  = yline[numlin-1][i] <= insideoffset;
					was_right = yline[numlin-1][i] >= outsideoffset;
					was_above = zline[numlin-1][i] <= tanktop;
					was_below = zline[numlin-1][i] >= baselevel;

					ever_inside = FALSE;
					for(j = numlin-2 ; j >= 0 ; j--) {	/* tank lines feed off hull lines, but order is reversed */
						if(stsec[j] <= i && ensec[j] >= i) {
							is_left  = yline[j][i] <= insideoffset;
							is_right = yline[j][i] >= outsideoffset;
							is_above = zline[j][i] <= tanktop;
							is_below = zline[j][i] >= baselevel;
							now_inside = !is_left && !is_right && !is_above && !is_below;
							getparam(i,j,&a,&hb,&c,&hd);
							yl = yline[j][i];
							zl = zline[j][i];
							last_line = j;

//	Check for an intersection with the inside face of the tank between lines j and j+1

							if(side_offset(i,j,firstline,tanktop,baselevel,insideoffset,skincorr,&al,&cl,&yl,&zl,&yc,&zc)) {
								inc_jtank();
								yline[jtank][i] = yl;
								zline[jtank][i] = zl;
								if(is_left) {
									ycont[jtank][i] = yc;
									zcont[jtank][i] = zc;
								} else {
									ycont[jtank][i] = yline[jtank-1][i];
									zcont[jtank][i] = zline[jtank-1][i];
								}
								add_tank_lines(&leftcutindex[leftcutcount],sec1,i,&prev_lines);
								was_left = is_left;
								was_below = FALSE;
								was_above = FALSE;
								ever_inside = TRUE;
								if(first_intersection == NONE)
									first_intersection = INSIDE;
								else
									last_intersection = INSIDE;
								if(leftcutcount < 8) leftcutcount++;
							}

//	Check for an intersection with the bottom face of the tank between lines j and j+1

//	al,cl are the curve parameters from yl,zl to line j. They are modified when an intersection is found, because yl and zl are also modified
//	yl,zl are returned, calculated offsets for the intersection
//	yc,zc are the returned, calculated control offsets for the tank line

							if(base_top_offset(i,j,firstline,insideoffset,outsideoffset,baselevel,skincorr,&al,&cl,&yl,&zl,&yc,&zc)) {
								inc_jtank();
								yline[jtank][i] = yl;
								zline[jtank][i] = zl;
								if(was_below) {
									ycont[jtank][i] = yline[jtank-1][i];
									zcont[jtank][i] = zline[jtank-1][i];
								} else {
									ycont[jtank][i] = yc;
									zcont[jtank][i] = zc;
								}
								add_tank_lines(&bottomcutindex[bottomcutcount],sec1,i,&prev_lines);
								was_below = is_below;
								was_left = FALSE;
								was_right = FALSE;
								ever_inside = TRUE;
								if(first_intersection == NONE)
									first_intersection = BOTTOM;
								else
									last_intersection = BOTTOM;
								if(bottomcutcount < 8) bottomcutcount++;
							}

//	Check for an intersection with the outside face of the tank between lines j and j+1

							if(side_offset(i,j,firstline,tanktop,baselevel,outsideoffset,skincorr,&al,&cl,&yl,&zl,&yc,&zc)) {

								/*	If the first intersection is on the right, add lines at the top and bottom inside corners, and the outside bottom corner		*/

								inc_jtank();
								yline[jtank][i] = yl;
								zline[jtank][i] = zl;
								if(was_right) {
									ycont[jtank][i] = yline[jtank-1][i];
									zcont[jtank][i] = zline[jtank-1][i];
								} else {
									ycont[jtank][i] = yc;
									zcont[jtank][i] = zc;
								}
								add_tank_lines(&rightcutindex[rightcutcount],sec1,i,&prev_lines);
								was_right = is_right;
								was_left = FALSE;
								ever_inside = TRUE;
								if(first_intersection == NONE)
									first_intersection = OUTSIDE;
								else
									last_intersection = OUTSIDE;
								if(rightcutcount < 8) rightcutcount++;
							}

//	Check for an intersection with the top face of the tank between lines j and j+1

							if(base_top_offset(i,j,firstline,insideoffset,outsideoffset,tanktop,skincorr,&al,&cl,&yl,&zl,&yc,&zc)) {

								/*	If the first intersection is on the topt, add lines at the top and bottom inside corners, and the outside bottom corner		*/

								inc_jtank();
								yline[jtank][i] = yl;
								zline[jtank][i] = zl;
								ycont[jtank][i] = yc;	//	yline[jtank-1][i];
								zcont[jtank][i] = zc;	//	zline[jtank-1][i];
								add_tank_lines(&topcutindex[topcutcount],sec1,i,&prev_lines);
								was_left = FALSE;	/* the curve is inside the tank */
								was_right = FALSE;
								ever_inside = TRUE;
								if(first_intersection == NONE)
									first_intersection = TOP;
								else
									last_intersection = TOP;
								if(topcutcount < 8) topcutcount++;
							}
							was_above = is_above;

							now_inside = !is_left && !is_right && !is_above && !is_below;

							if(now_inside) {	// hull surface is inside the tank at this line - add point to tank at hull line
								ever_inside = TRUE;

/*	We want the tank outline point corresponding to line j		*/

								/*	Find and normalise the tangent vector at the "j" end of the between from lines j+1 and j			*/
								/*	For the first point inside the tank, al and cl come from the offset routine. for subsequent ones,	*/
								/*	they come from the previous values of an and cn.													*/

								/* (al,cl) is the tangent to the tank on the curve from the previous offset point to the one corresponding to line j	*/

								if(!normalise(al,cl,&al,&cl,&len)) {	/* al, cl may be changed in corner code */
									al = 1.0;
									cl = 0.0;
								}

								if(!normalise(ap,cp,&ap,&cp,&len)) {	/* al, cl may be changed in corner code */
									ap = 1.0;
									cp = 0.0;
								}

								/*	Normalise the already-found tangent vector at the "j" end of the curve between lines j and j-1	*/

								if(!normalise(a,c,&an,&cn,&len) && !normalise(a+0.02*hb,c+0.02*hd,&an,&cn,&len)) continue;

								/*	Calculate the vector equiangular from both outline curve segments meeting at offset point j	*/

								if(!normalise(-cn-cp,an+ap,&t1,&t2,&len) && !normalise(-cn,an,&t1,&t2,&len)) continue;

								/*	Hence locate the line position	*/

								len = fabs(t1*cn - t2*an);	// sine of angle between
								if(len < 0.05) len = 0.05;
								yl = yline[j][i] + t1*skincorr/len;
								zl = zline[j][i] - t2*skincorr/len;
								if(zl > tanktop && zl < baselevel && yl < outsideoffset && yl > insideoffset) {
									inc_jtank();
									yline[jtank][i] = max(0.0,yl);
									zline[jtank][i] = zl;

								/*	Locate the control point for the current tank line	*/

									len = cp*al - ap*cl;
									if(fabs(len) > 0.05) {
										t2 = ( (yline[jtank][i] - yline[jtank-1][i])*cp - (zline[jtank-1][i] - zline[jtank][i])*ap ) / len;
										ycont[jtank][i] = max(0.0,yline[jtank-1][i] + t2*al);
										zcont[jtank][i] = zline[jtank-1][i] - t2*cl;
									}
									else {
										ycont[jtank][i] = yline[jtank-1][i];
										zcont[jtank][i] = zline[jtank-1][i];
									}
								}
								al = an;
								cl = cn;
								ap = a + 2.0*hb;
								cp = c + 2.0*hd;
							}
							jprev = j;
						}
					}	/* end loop through hull lines */

//	We now have all intersections of the tank outline with the hull, and all points between these. We now must close the tank

					if(!ever_inside) {

//	simple rectilinear tank - hull surface never inside the tank
						do {
							add_corner_line(insideoffset,baselevel,i,&inside_bottom_line,sec1,firstline);
						} while(jtank < bottomleftindex);
						bottomleftindex = jtank;
						do {
							add_corner_line(outsideoffset,baselevel,i,&outside_bottom_line,sec1,firstline);
						} while(jtank < bottomrightindex);
						bottomrightindex = jtank;
						do {
							add_corner_line(outsideoffset,tanktop,i,&outside_top_line,sec1,firstline);
						} while(jtank < toprightindex);
						toprightindex = jtank;
						do {
							add_corner_line(insideoffset,tanktop  ,i,&inside_top_line   ,sec1,firstline);
						} while(jtank < topleftindex);
						topleftindex = jtank;

//	(Hull totally inside the tank handled earlier)

					} else {

						if(last_intersection == NONE) {		// only an inside cut
							last_intersection = TOP;
							topval = zline[0][i];
						} else {
							topval = tanktop;
						}

//	close the tank from the last intersection to the first,

						for(j = last_intersection ; j != first_intersection ; j = j % 4 + 1) {
							switch(j) {
							case BOTTOM:
								do {
									add_corner_line(outsideoffset,baselevel,i,&outside_bottom_line,sec1,firstline);
								} while(jtank < bottomrightindex);
								bottomrightindex = jtank;
								break;
							case OUTSIDE:
								do {
									add_corner_line(outsideoffset,tanktop,i,&outside_top_line,sec1,firstline);
								} while(jtank < toprightindex);
								toprightindex = jtank;
								break;
							case TOP:
								do {
									add_corner_line(insideoffset,topval,i,&inside_top_line   ,sec1,firstline);
								} while(jtank < topleftindex);
								topleftindex = jtank;
								break;
							case INSIDE:
								do {
									add_corner_line(insideoffset,baselevel,i,&inside_bottom_line,sec1,firstline);
								} while(jtank < bottomleftindex);
								bottomleftindex = jtank;
								break;
							}
						}
					}
					prev_lines = jtank + 1;
				}

				for(i = sec1 ; i <= sec2 ; i++) {
					for(j = firstline ; j < extlin ; j++) {
						if(yline[j][i] < 0.0) {
							yline[j][i] = yline[firstline][i];
							zline[j][i] = zline[firstline][i];
							ycont[j][i] = yline[firstline][i];
							zcont[j][i] = zline[firstline][i];
						}
					}
				}

				/*	Copy the offsets to the other side, where a symmetrical tank is requested	*/

				if(tankloc == 2) {
					jx = extlin - firstline - 1;	/* number of extra lines = number of tank lines, less one for non-duplicated top centre line */
					jtank = extlin - 1;				/* highest index of tank lines created so far */
					extlin += jx;
					realloc_hull(extlin);			/* allocate extra lines needed */

					for(i = sec1 ; i <= sec2 ; i++) {

						/*	Copy in mirrors of existing lines	*/

						for(j = extlin-1, jj = firstline ; j > jtank ; j--, jj++) {
							yline[j][i] = -yline[jj][i];
							zline[j][i] = zline[jj][i];
							ycont[j][i] = -ycont[jj+1][i];
							zcont[j][i] = zcont[jj+1][i];
						}
					}
				}
			}		/* end loop through sections */

			fl_line1[ntank] = firstline;
			for(j = firstline ; j < extlin ; j++) {
				stsec[j] = sec1;
				ensec[j] = sec2;
				for(i = stsec[j] ; i <= ensec[j] ; i++) linewt[j][i] = 1.0;
				relcont[j] = FALSE;
				radstem[j] = 0.0;
			}
			fl_right[ntank] = tankloc > 0;
			fl_fixed[ntank] = 1;
			fl_fract[ntank] = 100.0;
			fl_perm[ntank]  = 1.0;
			fl_spgra[ntank] = 1.0;
			fl_line1[ntank+1] = extlin;
		}			/* end shape == 3 option */
	}
	tank = ntank;
	if(success) {
		save_hull(MAINHULL);
		ntank++;
		recalc_transom();
		recalc_tanks();
		changed = 1;
	}
}

/*	Calculate the z-offset where the skin intersects a vertical line */

/*	On an intersection, also return the tangent vector (al,cl) at the intersection	*/

REAL vertical_intersection(int i,int j,REAL y,REAL skincorr,REAL *al,REAL *cl,REAL *a,REAL *b,REAL *c,REAL *d)
{
	REAL hb,hd,t,t1,t2,ap,cp;

	getparam(i,j+1,a,&hb,c,&hd);
	*b = hb+hb;
	*d = hd+hd;
	inters(*a,hb,*c,hd,-1.0,0.0,y - yline[j+1][i],&t1,&t2);
	t = (t1 < 0.0 || t1 > 1.0) ? t2 : t1;
	if(t < 0.0 || t > 1.0) {
		t2 = 1.0e+30;
	}
	else {
		ap = *a + t*(*b);
		cp = *c + t*(*d);
		t1 = sqrt(ap*ap + cp*cp);
		if(ap != 0.0)
			t2 = zline[j+1][i] - t*((*c) + t*hd) - t1*skincorr/ap;
		else
		    t2 = 1.0e+30;
		*al = ap;
		*cl = cp;
	}
	return t2;
}

/*	At call, (al,cl) is the tangent from the previous intersection, (y,z) is its coordinate			*/
/*	On return, (al,cl) is the tangent at the new intersection, and (y,z) is the new coordinate pair	*/

int side_offset(int i,int j,int firstline,REAL tanktop,REAL baselevel,REAL sideoffset,REAL skincorr,REAL *al,REAL *cl,REAL *y,REAL *z,REAL *yc,REAL *zc)
{
	REAL t,t2,a,b,c,d,ap,cp,len;
	extern int jtank;
	t = vertical_intersection(i,j,sideoffset,skincorr,&ap,&cp,&a,&b,&c,&d);	// was j-1
	if(t > tanktop && t < baselevel) {
		if(jtank == firstline) {
			*yc = *y;
			*zc = *z;
		}
		else {
			len = cp * (*al) - ap * (*cl);
			if(fabs(len) > 0.05) {
				t2 = ( (t - *z)*(*al) + (sideoffset - *y)*(*cl) ) / len;
				*yc = max(0.0,sideoffset + t2*ap);
				*zc = t - t2*cp;
			}
			else {
				*yc = sideoffset;
				*zc = t;
			}
		}
		*al = ap;
		*cl = cp;
		*y = sideoffset;
		*z = t;
		return TRUE;
	}
	else {
		return FALSE;
	}
}

/*	Calculate the y-offset where the skin intersects a horizontal line */

/*	On an intersection, also return the tangent vector (al,cl) at the intersection	*/

REAL horizontal_intersection(int i,int j,REAL z,REAL skincorr,REAL *al,REAL *cl,REAL *a,REAL *b,REAL *c,REAL *d,REAL *yret,REAL *zret)
{
	REAL hb,hd,t,t1,t2,ap,cp,hbp,hdp;
	REAL y0,z0,y1,z1,div;

	getparam(i,j+1,a,&hb,c,&hd);
	*b = hb+hb;
	*d = hd+hd;

//	Find locations of ends of inside curve

	ap = (*a) + (*b);
	cp = (*c) + (*d);
	t = sqrt((*a)*(*a) + (*c)*(*c));
	if(t > 0.0) {
		y0 = yline[j+1][i] - skincorr*(*c)/t;
		if(y0 < 0.0) y0 = 0.0;
		z0 = zline[j+1][i] - skincorr*(*a)/t;
		t = sqrt(ap*ap + cp*cp);
		if(t <= 0.0) {
			ap = (*a) + 0.5*(*b);
			cp = (*c) + 0.5*(*d);
			t = sqrt(ap*ap + cp*cp);
		}
		if(t > 0.0) {
			y1 = yline[j][i] - skincorr*cp/t;
			if(y1 < 0.0) y1 = 0.0;
			z1 = zline[j][i] - skincorr*ap/t;

//	Now work out parameters of curve between

			div = (*a)*cp - ap*(*c);
			if(div != 0.0)
				t = ((z1 - z0)*ap - (y0 - y1)*cp) / div;
			else
				t = 0.5;
			ap = 2.0*(*a)*t;
			hbp = (y1 - y0) - ap;
			cp = 2.0*(*c)*t;
			hdp = (z0 - z1) - cp;

//	Find the curve parameter where the intersection occurs

			inters(ap,hbp,cp,hdp,0.0,1.0,z0 - z,&t1,&t2);
			t = (t1 < 0.0 || t1 > 1.0) ? t2 : t1;
			if(t < 0.0 || t > 1.0) {
				t2 = 1.0e+30;	// no intersection
				*yret = y0;
				*zret = z0;
			}
			else {
				t2 = y0 + t*(ap + t*hbp);
				*al = ap + t*(hbp + hbp);
				*cl = cp + t*(hdp + hdp);
				*yret = y0 + t*(ap + hbp*t);
				*zret = z0 - t*(cp + hdp*t);
			}
		} else {
			t2 = 1.0e+30;	// no orientation at top
			*yret = y0;
			*zret = z0;
		}
	} else {
		t2 = 1.0e+30;		// no orientation at bottom
		*yret = yline[j+1][i];
		*zret = zline[j+1][i];
	}
	return t2;
}

/*	At call, (al,cl) is the tangent from the previous intersection, (y,z) is its coordinate			*/
/*	On successful return, (al,cl) is the tangent at the new intersection, and (y,z) is the new coordinate pair	*/

int base_top_offset(int i,int j,int firstline,REAL insideoffset,REAL outsideoffset,REAL baselevel,REAL skincorr,
	REAL *al,REAL *cl,REAL *y,REAL *z,REAL *yc,REAL *zc)
{
	REAL t,t2,a,hb,c,hd,len,b,d,ap,cp,yr,zr;
	extern int jtank;

	t = horizontal_intersection(i,j,baselevel,skincorr,&ap,&cp,&a,&b,&c,&d,&yr,&zr);
	if(t > insideoffset && t < outsideoffset) {
		len = cp*(*al) - ap*(*cl);	/* cross product with terms at base (new intersection) end of curve */
		if(fabs(len) > 0.05) {
			t2 = ( (baselevel - *z)*(*al) + (t - *y)*(*cl) ) / len;
			*yc = t + t2*ap;
			*zc = baselevel - t2*cp;
		}
		else {
			*yc = t;
			*zc = baselevel;
		}
		*al = ap;
		*cl = cp;
		*y = t;
		*z = baselevel;
		return TRUE;
	}
	else {
		return FALSE;
	}
}

void tankedit(int code,HWND hWndDlg)
{
	int i,n,*pos;
#ifdef linux
#include <Xm/Text.h>
	extern Widget wListBox[];
	XmListGetSelectedPos(wListBox[0],(int **) &pos,&n);
	if(n > 0) tank = *pos;
	XtFree((char *) pos);
#endif
	i = tank-1;
	if(i < 0 || i >= ntank) {
		message("Invalid tank index");
		return;
	}

	if(getdlg(EDITSTAT,
		INP_STR,tankdesc[i],
		INP_REA,(void *) &fl_fract[i],
		INP_REA,(void *) &fl_spgra[i],
		INP_REA,(void *) &fl_perm[i],
		INP_RBN,(void *) &fl_right[i],
		INP_RBN,(void *) &fl_fixed[i],
		-1)) {
		if(fl_fixed[i]) {
			if(fl_fract[i] < 0.0)
				fl_fract[0] = 0.0;
			else if(fl_fixed[i] == 2) {	/* % volume */
				if(fl_fract[i] > 100.0) fl_fract[i] = 100.0;
			}
			else {			/* actual volume */
				if(fl_fract[i] > fl_volum[i]) fl_fract[i] = fl_volum[i];
			}
		}
	}
}

/*	Find full volume of all tanks		*/

void recalc_tanks(void)
{
	REAL area1;
	REAL area2 = 0.0;
	REAL ignore;
	REAL x1;
	REAL volume;
	int	 i,j1,j2;
	REAL sumysqdif = 0.0,sumyoffs = 0.0;
	extern int tankmode;

#define WATERLEVEL -1000.0

	for(tank = 0 ; tank < ntank ; tank++) {
		j1 = fl_line1[tank];
		j2 = fl_line1[tank+1];
		i = stsec[j1];
		x1 = xsect[i];
		tankmode = TRUE;
		volume = 0.0;
		for( ; i <= ensec[j1] ; i ++) {
			calcar(i,j1,j2,
				fl_right[tank],!fl_right[tank],
				&area1,&area2,
				&ignore,&ignore,
				&ignore,&ignore,
				x1,xsect[i],
				&ignore,&volume,&ignore,&ignore,&ignore,
				WATERLEVEL,1.0,
				&ignore,&ignore,&ignore,
				&ignore,&ignore,
				&ignore,&ignore,&ignore,&ignore,
				&sumysqdif,&sumyoffs);
			x1 = xsect[i];
		}
		fl_volum[tank] = -volume;
	}
	tank = 0;	/* for safety */
}

REAL tanklevel(int tank,REAL wl0,REAL *gmfac,REAL *xsum,REAL *sumlcf,
	REAL *summct,REAL *tankplane,REAL *vol);

#ifdef linux
#define GDIin xgrin
#define GDIen xgren
#endif

REAL voltab[100],distab[100];
int num_calib;
int calib_mode = 0;
extern int tank;

void show_calibrations(void)
{
	int j;
	char text[200];
	extern int ycursor;
	FILE *fp;
	extern int ychar,ymaxi;

	if(hardcopy && calib_mode != 1) return;

	if(calib_mode == 0) {
		(*init)();
		clrtext();
		pint("\nTank %d: volume, percent full, distance down from full ...",tank+1);
		for(j = 0 ; j < num_calib ; j++) {
			sprintf(text,"\n%.3f cu. %s %.1f %% %.3f %s",voltab[j],lenun[numun],
				voltab[j]/fl_volum[tank]*100.0,distab[j],lenun[numun]);
			pstr(text);
			if(ycursor >= ymaxi - ychar) break;
		}
		(*endgrf)();
	} else if(calib_mode == 1) {
		(*init)();
		graph(distab,voltab,num_calib,"DISTANCE FROM TOP","VOLUME",1.0e+30,0.0);
		(*endgrf)();
	}
}

MENUFUNC calibration(void)
{
	extern int tank;
	static int n = 1;
	static REAL holdintvl = 1;
	static int volume_from_depth = 0;	/* if volume is provided, 1 if % is provided, 2 if depth is provided */
	REAL save_fract,wlev;
	int save_fixed;
	REAL intvl;
	REAL ignore;
	REAL surface,volume;
	int first = 1;
	REAL wl1;
	REAL value;
	int i;
	extern int hardcopy;
	FILE *fp;
	REAL minvol;
	char text[200];

	if(ntank <= 0) {
		message(notanks);
		return;
	}

	if(getdlg(STATCALI,
		INP_INT,(void *) &n,
		INP_REA,(void *) &holdintvl,
		INP_RBN,(void *) &calib_mode,
		INP_RBN,(void *) &volume_from_depth,
		-1) && n > 0 && n <= ntank) {
		tank = n-1;
		save_fract = fl_fract[tank];
		save_fixed = fl_fixed[tank];
		intvl = holdintvl;
		fl_fixed[tank] = volume_from_depth < 2;	/* 0 or 1 means "volume or percent gives level" */
		if(volume_from_depth == 1) intvl *= (0.01*fl_volum[tank]);
		volume = fl_volum[tank];

		i = 0;
		wlev = -1.0e+10;
		minvol = volume * 0.001;
		while(i < 100) {
			if(volume_from_depth < 2) fl_fract[tank] = fabs(volume);
			value = tanklevel(tank,wlev,&ignore,&ignore,&ignore,&ignore,&surface,&volume);
			volume = -volume;
			surface = - surface;
			if(volume_from_depth < 2) wlev = value;
			if(first) {
				wlev = value;
				wl1 = wlev;
				first = 0;
			}
			voltab[i] = volume;
			distab[i++] = wlev - wl1;

			if(volume_from_depth == 2) {
				wlev += intvl;
			}
			else {	/* depth from volume */
				volume -= intvl;
			}
			if(volume < minvol) break;
		}
		fl_fract[tank] = save_fract;
		fl_fixed[tank] = save_fixed;

		num_calib = i;
		if(calib_mode == 2) {
			if(open_text(&fp,filedirnam,"*.txt")) {
				fprintf(fp,"Tank %d: volume, percent full, distance down from full ...\n",n);
				for(i = 0 ; i < num_calib ; i++) fprintf(fp,"%.3f\t%.1f\t%.3f\n",
					voltab[i],voltab[i]/fl_volum[tank]*100.0,distab[i]);
				fclose(fp);
			}
			update_func = NULL;
			print_func = NULL;
		} else {
			update_func = show_calibrations;
			print_func = show_calibrations;
			show_calibrations();
		}
	}
}

MENUFUNC tank_summary()
{
	FILE *fp;
	int i;
	REAL tcg;

	if(ntank <= 0) {
		message(notanks);
		return;
	}

	if(open_text(&fp,filedirnam,"*.txt")) {
		fprintf(fp,"Fract'n\tPerm.\tMass\tVCG\tVer.Mom\tLCG\tLCF\tLon.Mom\tTr.FSM\tLo.FSM\tmoment\tTCG\n");
		calctankstat();
		for(i = 0 ; i < ntank ; i++) {
			if(tankmass[i] != 0.0) {
				tcg = tankmom[i]/tankmass[i];
			}
			else {
				tcg = 0.0;
			}
			fprintf(fp,"%d: %s\n",i+1,tankdesc[i]);
			fprintf(fp,"%s",fixto7(fl_fract[i]));
			fprintf(fp,"\t%s",fixto7(fl_perm[i]));
			fprintf(fp,"\t%s",fixto7(tankmass[i]));
			fprintf(fp,"\t%s",fixto7(invert*tankvcg[i]));
			fprintf(fp,"\t%s",fixto7(tankmass[i]*invert*tankvcg[i]));
			fprintf(fp,"\t%s",fixto7(tanklcg[i]));
			fprintf(fp,"\t%s",fixto7(tanklcf[i]));
			fprintf(fp,"\t%s",fixto7(tankmass[i]*tanklcg[i]));
			fprintf(fp,"\t%s",fixto7(tanktfsm[i]));
			fprintf(fp,"\t%s",fixto7(tanklfsm[i]));
			fprintf(fp,"\t%s",fixto7(tankmom[i]));
			fprintf(fp,"\t%s\n",fixto7(tcg));
		}
		fclose(fp);
	}
}

void calctankstat()
{
	REAL fsm;		/* free surface moment */
	REAL xsum;		/* first moment of sectional areas (for CG) */
	REAL xcg;
	REAL locxlcf;
	REAL lfsm;
	REAL tanklcfv,tankmct,tankplane,vol;
	REAL watlev;
	REAL rm1,rm2,cl1,cl2,drm,zsum,zsum1,zsum2,
	zsumav,delx,tankmoment;
	REAL mass;
	REAL ztank;
	int  i,j1,j2,j,k;
	REAL draft = tc;	/* save draft - tank calcs screw it up */

	for(i = 0 ; i < ntank ; i++) {
		watlev = tanklevel(i,wl,&fsm,&xsum,&tanklcfv,&tankmct,
			&tankplane,&vol);
		fsm = - fsm;
		xsum = - xsum;
		tanklcfv = - tanklcfv;
		tankmct = - tankmct;
		tankplane = -tankplane;
		vol = -vol;

		fl_walev[i] = watlev;
		density = densit[numun];
		if(fl_fixed[i]) density *= fl_spgra[i]/spgrav;
		mass = vol * density;
		if(mass != 0.0) {
			xcg = xsum / mass;
		}
		else {
			xcg = -999.9;
		}
		if(tankplane > 0.0) {
			locxlcf = tanklcfv / tankplane;
		}
		else {
			locxlcf = -999.9;	/* no surface */
		}
		lfsm = tankmct + density*locxlcf*locxlcf*tankplane;
		j1 = fl_line1[i];
		j2 = fl_line1[i+1];
		rm2 = 0.0;
		cl2 = 0.0;
		zsum = 0.0;
		zsum2 = 0.0;
		zsumav = 0.0;
		tankmoment = 0.0;
		j = fl_right[i];
		for(k = stsec[j1] ; k <= ensec[j1] ; k++) {
			drm = calcrm(k,j1,j2,!j,j,watlev,&rm1,&rm2,&cl1,&cl2,
				&zsum1,&zsum2,&zsumav);
			if(k > stsec[j1]) {
				delx = xsect[k]-xsect[k-1];
				tankmoment += drm*delx;
				zsum = zsum + zsumav*delx;
			}
		}
		tankmoment *= density;
		if(vol > 0.0) {
			ztank = -zsum / vol;
		}
		else {
			ztank = -999.9;
		}

		mass = fl_fract[i] * 0.01 * fl_volum[i] * fl_perm[i] * density;
		tankmass[i] = mass;
		tankvcg[i] = ztank;
		tanklcg[i] = xcg;
		tanklcf[i] = locxlcf;
		tanktfsm[i] = fsm;
		tanklfsm[i] = lfsm;
		tankmom[i] = tankmoment;
	}
	tc = draft;
}

#endif
