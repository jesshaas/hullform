/* Hullform component - stringer.c
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

REAL A,B,C;
REAL b,d;
REAL divisor,sqdiv;
REAL sqC;
REAL u0,u1;

#ifdef PROF

REAL curvedist(REAL t,REAL *rate);
REAL solvecurve(REAL s0,REAL t,REAL dt);
void stringer_param(REAL a,REAL hb,REAL c,REAL hd);
int realloc_stringers(int k);
void calc_stringers(int strlin);
int xinters(REAL x0,REAL a,REAL b,REAL c,REAL *x,REAL x1);
INT wrxyzunit(FILE *fp,INT nl,REAL x,REAL y,REAL z);
void false_devel(void);
void remove_stringers(int strlin);
char *floattext(REAL);

#ifndef STUDENT
int  strlin = -1;
int  strind = 0;
extern int  *strmode;
#define str_count str_interv
#define str_firstfrac str_firstint
#endif

extern int numbetw;
int currmode = 0;
extern REAL def_str_width;
extern REAL def_str_thick;
extern int def_str_direc;
extern REAL def_str_interv;
extern REAL def_str_firstint;
extern int def_str_count;
extern REAL def_str_firstfrac;

#ifndef STUDENT

#ifdef linux

void str_change_boxes(Widget w,XtPointer client_data,XtPointer call_data)
{
	int n = (int) client_data;
	extern Widget wEdit[],wLabel[];
	XmString xstr;
	char text[20];
	char *floatext(REAL x);

	switch (n) {
	case 0:	// Uniform intervals
	    xstr = XmStringCreateSimple("Number of stringers");
    	XtVaSetValues(wLabel[3],XmNlabelString,xstr,NULL);
	    XmStringFree(xstr);
	    xstr = XmStringCreateSimple("Fraction to first");
    	XtVaSetValues(wLabel[4],XmNlabelString,xstr,NULL);
	    XmStringFree(xstr);
	    sprintf(text,"%d",def_str_count);
		xstr = XmStringCreateSimple(text);
    	XtVaSetValues(wEdit[3],XmNlabelString,xstr,NULL);
	    XmStringFree(xstr);
		break;
	case 1:	// Fixed number
	    xstr = XmStringCreateSimple("Interval");
    	XtVaSetValues(wLabel[3],XmNlabelString,xstr,NULL);
	    XmStringFree(xstr);
	    xstr = XmStringCreateSimple("First interval");
    	XtVaSetValues(wLabel[4],XmNlabelString,xstr,NULL);
	    XmStringFree(xstr);
		xstr = XmStringCreateSimple(floattext(def_str_firstint));
    	XtVaSetValues(wEdit[3],XmNlabelString,xstr,NULL);
	    XmStringFree(xstr);
		break;
	}
}

void str_init(HWND hDlg)
{
	extern Widget wRadioButton[];
	XtAddCallback(wRadioButton[3],XmNarmCallback,str_change_boxes,(XtPointer) 0);
	XtAddCallback(wRadioButton[4],XmNarmCallback,str_change_boxes,(XtPointer) 1);
}

#endif

/*	Form stringers for a pair of hull lines		*/

MENUFUNC form_stringers()
{
	int code;
	extern void strfunc(int);
	extern int dlgresult;
#ifdef linux
	int n;
	REAL val;
#else
	DLGPROC StringerDlgProc(HWND hWndDlg,unsigned msg,WORD wParam,LONG lParam);
#endif

	if(strlin < 0) strlin = stemli;	/* default is initially stem line */

	cls(FALSE);

	/*	Find line pair whose stringers are to be calculated, the distance
	interval between stringers, and the sense of insertion (up from
	bottom, down from top)
	*/

	update_func = NULL;
#ifdef linux
	n = strlin + 1;
	if(currmode == 1)
		val = def_str_count;
	else
		val = def_str_firstint;
	dlgresult = getdlg(EDITSTRI,
		INP_INT,(void *) &n,
		INP_REA,(void *) &def_str_width,
		INP_REA,(void *) &def_str_thick,
		INP_RBN,(void *) &def_str_direc,
		INP_RBN,(void *) &currmode,
		INP_REA,(void *) &val,
		INP_REA,(void *) &def_str_interv,
		INP_INI,(void *) str_init,
		-1);
#else
	code = DialogBox(hInst,(char *) EDITSTRI,hWnd,(DLGPROC) StringerDlgProc);
	if(code < 0) (void) MessageBox(hWnd,"EDITSTRI","Could not create dialog box",MB_OK);
#endif
	if(!dlgresult) return;

	str_dir[strlin] = def_str_direc;
	strmode[strlin] = currmode;
	str_firstint[strlin] = def_str_firstint;
	str_interv[strlin] = currmode == 1 ? def_str_count : def_str_interv;
	str_thk[strlin] = def_str_thick;
	str_wid[strlin] = def_str_width;
	calc_stringers(strlin);
	showstringers = 1;
}

#ifndef linux

DLGPROC StringerDlgProc(HWND hWndDlg,unsigned msg,WORD wParam,LONG lParam)
{
	void centre_dlg(HWND);
	char text1[40],text2[40];
	int j,n;
	extern int dlgresult;
	extern int context_id,HelpUsed;
	BOOL colret;
	extern char *helpfile;

	switch(msg) {

	case WM_INITDIALOG:
		centre_dlg(hWndDlg);
		SetDlgItemInt(hWndDlg,DLGEDIT+0,strlin+1,TRUE);
		SetDlgItemText(hWndDlg,DLGEDIT+1,floattext(def_str_width));
		SetDlgItemText(hWndDlg,DLGEDIT+2,floattext(def_str_thick));
		CheckRadioButton(hWndDlg,DLGRBN1,DLGRBN1+1,DLGRBN1+def_str_direc);
		CheckRadioButton(hWndDlg,DLGRBN2,DLGRBN2+2,DLGRBN2+currmode);
		if(currmode == 1) {	/* fixed number of stringers */
			SetDlgItemInt(hWndDlg,DLGEDIT+3,def_str_count,TRUE);
			SetDlgItemText(hWndDlg,DLGEDIT+4,floattext(def_str_firstint));
			SetDlgItemText(hWndDlg,DLGTEXT+3,"&Number of stringers");
			SetDlgItemText(hWndDlg,DLGTEXT+4,"&Fraction to first");
		} else {			/* fixed interval */
			SetDlgItemText(hWndDlg,DLGEDIT+3,floattext(def_str_interv));
			SetDlgItemText(hWndDlg,DLGEDIT+4,floattext(def_str_firstint));
			SetDlgItemText(hWndDlg,DLGTEXT+3,"&Interval");
			SetDlgItemText(hWndDlg,DLGTEXT+4,"&First interval");
		}
		SetFocus(GetDlgItem(hWndDlg,DLGEDIT+0));
		SendDlgItemMessage(hWndDlg,DLGEDIT+0,EM_SETSEL,(WPARAM) 0,(LPARAM) -1);
		break;

	case WM_RBUTTONDOWN:
		goto cancel;

	case WM_COMMAND:
		switch(wParam) {
		case IDOK:
ok:
			n = GetDlgItemInt(hWndDlg,DLGEDIT+0,NULL,TRUE) - 1;
			if(n >= 1 && n < numlin) {
				strlin = n;
			} else {
				message("Invalid line index");
				dlgresult = FALSE;
				return 0;
			}
			GetDlgItemText(hWndDlg,DLGEDIT+1,text1,sizeof(text1));
			if(sscanf(text1,"%f",&def_str_width) < 1) {
				message("Invalid stringer width");
				dlgresult = FALSE;
				return 0;
			}
			GetDlgItemText(hWndDlg,DLGEDIT+2,text1,sizeof(text1));
			if(sscanf(text1,"%f",&def_str_thick) < 1) {
				message("Invalid stringer thickness");
				dlgresult = FALSE;
				return 0;
			}
			def_str_direc = IsDlgButtonChecked(hWndDlg,DLGRBN1) != BST_CHECKED;

			GetDlgItemText(hWndDlg,DLGEDIT+3,text1,sizeof(text1));
			GetDlgItemText(hWndDlg,DLGEDIT+4,text2,sizeof(text2));
			currmode = IsDlgButtonChecked(hWndDlg,DLGRBN2) != BST_CHECKED;
			if(currmode == 1) {	/* fixed number of stringers */
				sscanf(text1,"%d",&def_str_count);
			} else {			/* fixed interval */
				sscanf(text1,"%f",&def_str_interv);
			}
			sscanf(text2,"%f",&def_str_firstint);
			if(def_str_firstint < 0.0) def_str_firstint = 0.0;

		case IDCANCEL:
cancel:
			dlgresult = (wParam == IDOK);
			EndDialog(hWndDlg,dlgresult);
			break;

		case IDHELP:	/* Help */
			context(context_id);
			HelpUsed = 1;
			break;

		case DLGRBN1:
		case DLGRBN1+1:
			CheckRadioButton(hWndDlg,DLGRBN1,DLGRBN1+1,wParam);
			break;

		case DLGRBN2:
		case DLGRBN2+1:
			CheckRadioButton(hWndDlg,DLGRBN2,DLGRBN2+2,wParam);
			if(wParam == DLGRBN2+1) {	/* fixed number of stringers */
				SetDlgItemText(hWndDlg,DLGTEXT+3,"&Number of stringers");
				SetDlgItemText(hWndDlg,DLGTEXT+4,"&Fraction to first");
			} else {			/* fixed interval */
				SetDlgItemText(hWndDlg,DLGTEXT+3,"&Interval");
				SetDlgItemText(hWndDlg,DLGTEXT+4,"&First interval");
			}
			break;

		case DLGINC:
		case DLGDEC:
			j = GetDlgItemInt(hWndDlg,DLGEDIT,&n,TRUE);
			if(n != 0) {	/* success */
				if(wParam == DLGINC) {
					if(j < numlin-1) j++;
				} else {
					if(j > 2) j--;
				}
			}
			SetDlgItemInt(hWndDlg,DLGEDIT,j,TRUE);
			SetFocus(GetDlgItem(hWndDlg,DLGEDIT));
			SendDlgItemMessage(hWndDlg,DLGEDIT,EM_SETSEL,(WPARAM) 0,(LPARAM) -1);
			break;

		}
		break;
	}
	return 0;
}

#endif


/*	Calculate stringer forms	*/

void calc_stringers(int strlin)
{
	int i,j,j1,ii,jj,k,sts,ens;
	int isec,insec,lsec;
	REAL ds,ds1;
	REAL AQ,BQ;
	REAL a,hb,c,hd;
	REAL y0,y1,ys,x0;
	REAL t,dt;
	REAL s0,s,st;
	REAL ss;
	int stringer;
	REAL dist[maxsec];
	REAL as[maxsec],bs[maxsec],cs[maxsec];
	REAL a0,hb0,c0,hd0;

	/*	If there is no stringer to calculate, just return quietly
	and leave any error message to the caller
	*/

	if(str_interv[strlin] <= 0.0) return;

//	Get stem parameters

	getparam(0,strlin,&a0,&hb0,&c0,&hd0);

	remove_stringers(strlin);

//	Calculate size of stringer table. "k" receives the total number of stringers already defined

	k = 0;
	for(j = 0 ; j < extlin ; j++) k += numstr[j];

//	Stringers are always added at the end of the table

	inistr[strlin] = k;
	numstr[strlin] = 0;

	if(!realloc_stringers(k+1)) return;

//	Initialise the first stringer-start index to indicate that the first line has not been allocated.

//	A -1 value indicates no stringers yet defined.

	ststr[k] = -1;

//	Scan along all sections between the selected pair of lines

//	The first section "isec" is the first shared by the line and its predecessor. The last section "lsec" is the last shared

	lsec = min(ensec[strlin],ensec[strlin-1]);
	insec = max(stsec[strlin],stsec[strlin-1]);
	if(insec < 1) insec = 1;
	isec = -1;	// becomes previous section

//	Define stringers between the first section (or section 1) and the last

	for(i = insec ; i <= lsec ; i++) {

		k = inistr[strlin];		// index in stringer table where stringer data are to go

		/*	Get curvature parameters		*/
		getparam(i,strlin,&a,&hb,&c,&hd);

		/*	Initialise stringer calculation parameters in static memory
		*/
		stringer_param(a,hb,c,hd);

		/*	Stringer mode 0 is uniform intervals from either end, with
		an arbitrary start interval.

		Mode 1 is uniform stringer count. The start interval is an
		arbitrary fraction of the calculated stringer spacing for
		the current section
		*/
		if(strmode[strlin] == 0) {	/* constant interval	*/
			ds = str_interv[strlin];
			if(ds <= 0.0) ds = 1.0;
			ds1 = str_firstint[strlin];
		}
		else {			/* constant count	*/
			ds = (curvedist(1.0,&st) - curvedist(0.0,&st))/
				((double) (str_count[strlin])+str_firstfrac[strlin]);
			ds1 = ds * str_firstfrac[strlin];
		}

		/*	str_dir = 1 implies lower- to higher-indexed line	*/
		if(str_dir[strlin]) {
			t = 1.0;
			ds = -ds;
			ds1 = -ds1;
		}
		else {
			t = 0.0;
		}

		/*	If the line is straight, check that it is not of zero length
		If it is not, make the parameter step exactly that required.
		Otherwise, initialise the along-curve distance
		*/
		if(C == 0.0) {	/* C = b^2 + d^2 - curvature terms	*/
			if(A == 0.0) {
				dist[i] = 0.0;
				goto skiploop;
			}
			dt = ds / sqrt(A);
		}
		s0 = curvedist(t,&st) + ds1;
		t = 0.0;
		/* distance at start point on curve		*/

		/* 	Repeat stringer calculations while on this curve	*/

		do {
			t = solvecurve(s0,t,dt);

			/*	"stringer" indicates whether a stringer is possible	*/
			/*	    stringer = str_dir[strlin] ? t > 0.0001 : t < 0.9999;	*/
			stringer = t >= -0.0001 && t <= 1.0001;

			if(stringer) {

				/*	There is an intersection of the stringer between the current
				line pair at this section.

				k is current stringer index. On the first occurrence of this
				stringer, set up initial stringer parameters.
				*/
				if(ststr[k] < 0) {
					if(!realloc_stringers(k+1)) return;
					ststr[k] = i;	/* start at this section */
					ststrx[k] = xsect[i];
					ststr[k+1] = -1;	/* indicate next is undefined */
					numstr[strlin]++;
					for(ii = 0 ; ii < i ; ii++) ystr[k][ii] = -1000000.0;/* THIS SHOULDN'T BE NEEDED */
				}

				/*	Define location and orientation at this section		*/
				ystr[k][i] = yline[strlin][i] + t*(a+hb*t);
				zstr[k][i] = zline[strlin][i] - t*(c+hd*t);
				astr[k][i] = atan2(a+b*t,-c-d*t);

				/*	This is, for now, the last section of the stringer	*/
				enstr[k] = i;
				k++;
			}
			s0 += ds;	/* This is the next value of s we want	*/
		}
		while(stringer);

		/*	Add to the set of curve distances between hull lines.
		Note that this line only defines 'dist" from index
		1 onward, at most.					*/

		dist[i] = fabs(curvedist(1.0,&s0) - curvedist(0.0,&s0));
skiploop:
		/*	Fill the y-array with null points, to prevent their plotting	*/

		while(k < inistr[strlin] + numstr[strlin]) {
			ystr[k++][i] = -1000000.0;
		}

		if(isec < 0) isec = i;
	}

	for(ii = 0 ; ii < insec ; ii++) {
		for(k = inistr[strlin] ; k < inistr[strlin]+numstr[strlin] ; k++) ystr[k][ii] = -1000000.0;
	}

	/*	For stringer ends on hull lines, find spline fit to curve
	distances between lines, then solve for intersections on
	current stringer.
	*/

	/*	"j" is index of line which stringer may intersect */
	j = str_dir[strlin] ? strlin : strlin-1;

	/*	Save end points on lines	*/
	ys = yline[j][0];
	y0 = yline[strlin-1][0];
	y1 = yline[strlin][0];
	x0 = xsect[0];

	yline[j][0] = yline[stemli][1];
	xsect[0] -= y0;

	if(strmode[strlin] == 0) {	/* constant interval */
		ds = str_interv[strlin];
		st = str_firstint[strlin];
	}
	else {			/* constant count */
		ds = 1.0/((double) (str_count[strlin])+str_firstfrac[strlin]);
		st = ds * str_firstfrac[strlin];
	}

	/*	Look for end points before first and beyond last section
	for all stringers
	*/
	for(k = inistr[strlin] ; ststr[k] >= 0 ; k++) {

		i = ststr[k];	/* ststr[k] is never less than 1 at this stage */

		/*	Can have stem intersection if both lines start at the stem */

		if(stsec[strlin] == 0 && stsec[strlin-1] == 0 && i == 1) {

			/*	Find the alignment of the stringer at section 1, and use this
			forward to the stem
			*/
			jj = enstr[k]-ststr[k];
			if(numbetw > 0) {
				for(j1 = 0 ; j1 <= enstr[k] ; j1++) bs[j1] = 1.0;/* spline weights */
				spline2(&xsect[1],&zstr[k][1],bs,jj+1,&s0,&ss,0,0.0,0.0,&s0,as,bs,cs);
			}
			else {
				for(j1 = 1 ; j1 <= jj ; j1++) {
					as[j1] = 0.0;
					bs[j1] = 0.0;
					cs[j1] = (zstr[k][j1+1] - zstr[k][j1])/(xsect[j1+1] - xsect[j1]);
				}
			}

			/*	We use z = zstr[k][i] + cs[0]*(x-ststrx[k]) for the stringer,
			and x = x0-y1 - a0*t - hb0*t*t, so

			z = zstr[k][i] + cs[0]*(xsect[0]-a0*t-hb0*t*t-ststrx[k])

			For the stem curve, we use

			z = zline[strlin][0] - c0*t - hd0*t*t

			We need their intersection
			*/
			if(xinters(zstr[k][i]+cs[0]*(x0-y1-ststrx[k])- zline[strlin][0],0.0,
				-cs[0]*hb0 + hd0,-cs[0]*a0 + c0,&t,xsect[0]) && t >= 0.0 && t <= 1.0) {
				ystr[k][0] = yline[stemli][1];
				zstr[k][0] = zline[strlin][0] - t*(c0 + hd0*t);
				astr[k][0] = atan(-1.0);
				ststr[k] = 0;
				ststrx[k] = x0 - y1 - t*(a0 + hb0*t);
			}

			/*	Non-stem intersections			*/

		}
		else if(i > isec && strmode[strlin] == 0) { /* constant interval */

			/*	Call spline function to obtain spline curve parameters as, bs, cs
			for distance between lines (no fitted values returned) */
			if(numbetw > 0) {
				for(jj = isec ; jj <= lsec ; jj++) bs[jj] = 1.0;	/* spline weights */
				spline2(&xsect[isec],&dist[isec],&bs[isec],lsec-isec+1,
					&s0,&st,0,0.0,0.0,&s0,&as[isec],&bs[isec],&cs[isec]);
			}
			else {
				as[i-1] = 0.0;
				bs[i-1] = 0.0;
				cs[i-1] = (dist[i] - dist[i-1])/(xsect[i] - xsect[i-1]);
			}

			if(strmode[strlin] == 0) {
				ds1 = st;
			}
			else {
				ds1 = st * dist[i];
			}

			if(xinters(dist[i-1]-ds1,as[i-1],bs[i-1],cs[i-1],&s,0.0) && s > 0.0) {
				s += xsect[i-1];
				if(s >= 1.1*xsect[i-1]-0.1*xsect[i]) {

					/*	"j" refers to the upper or lower line, as required	*/
					sts = stsec[j];
					ens = ensec[j];
					ststr[k] = --i;
					ststrx[k] = s;
					if(numbetw > 0) {
						for(jj = isec ; jj <= lsec ; jj++) bs[jj] = 1.0;
						spline(&xsect[sts],&yline[j][sts],&bs[sts],
							ens - sts + 1,&s,&ystr[k][i],1,
							yline[j][maxsec],yline[j][maxsec+1]);
						spline(&xsect[sts],&zline[j][sts],&bs[sts],
							ens - sts + 1,&s,&zstr[k][i],1,
							zline[j][maxsec],zline[j][maxsec+1]);
					}
					else {
						t = (s - xsect[i])/(xsect[i+1] - xsect[i]);
						ystr[k][i] = yline[j][i] + t*(yline[j][i+1]-yline[j][i]);
						zstr[k][i] = zline[j][i] + t*(zline[j][i+1]-zline[j][i]);
					}
					if(i >= 1) {
						getparam(i,strlin,&a,&hb,&c,&hd);
						if(str_dir[strlin]) {
							AQ = a;
							BQ = -c;
						}
						else {
							AQ = a+hb+hb;
							BQ = -c-hd-hd;
						}
						if(AQ == 0.0 && BQ == 0.0)
							astr[k][i] = atan2(0.0,-1.0);
						else
							astr[k][i] = atan2(AQ,BQ);
					}
					else {
						astr[k][i] = atan2(0.0,-1.0);
					}
				}
			}

		}
		else {	/* constant count */
			ststrx[k] = xsect[i];
		}

		if(enstr[k] < lsec) {
			i = enstr[k];
			if(numbetw > 0) {
				for(jj = isec ; jj <= lsec ; jj++) bs[jj] = 1.0;	/* spline weights */
				spline2(&xsect[isec],&dist[isec],&bs[isec],lsec-isec+1,
					&s0,&st,0,0.0,0.0,&s0,&as[isec],&bs[isec],&cs[isec]);
			}
			else {
				as[i] = 0.0;
				bs[i] = 0.0;
				cs[i] = (dist[i+1] - dist[i])/(xsect[i+1] - xsect[i]);
			}

			if(xinters(dist[i]-st,as[i],bs[i],cs[i],&s,0.5*(xsect[i+1]-xsect[i]))) {
				s += xsect[i];
				enstr[k] = ++i;
				enstrx[k] = s;

				/*	"j" is line which stringer intersects		*/

				xsect[0] = x0 - ys;
				sts = stsec[j];
				ens = ensec[j];
				if(numbetw > 0) {
					for(j1 = sts ; j1 <= ens ; j1++) bs[j1] = 1.0;
					spline(&xsect[sts],&yline[j][sts],&bs[sts],ens - sts + 1,&s,
						&ystr[k][i],1,yline[j][maxsec],yline[j][maxsec+1]);

					spline(&xsect[sts],&zline[j][sts],&bs[sts],ens - sts + 1,&s,
						&zstr[k][i],1,zline[j][maxsec],zline[j][maxsec+1]);
				}
				else {
					t = (s - xsect[i-1])/(xsect[i] - xsect[i-1]);	/* note I has been incremented */
					ystr[k][i] = yline[j][i-1] + t*(yline[j][i]-yline[j][i-1]);
					zstr[k][i] = zline[j][i-1] + t*(zline[j][i]-zline[j][i-1]);
				}

				getparam(lsec,strlin,&a,&hb,&c,&hd);
				if(str_dir[strlin]) {
					AQ = a;
					BQ = -c;
				}
				else {
					AQ = a+hb+hb;
					BQ = -c-hd-hd;
				}
				if(AQ == 0.0 && BQ == 0.0)
					astr[k][i] = atan2(0.0,-1.0);
				else
					astr[k][i] = atan2(AQ,BQ);
			}
			else {
				enstrx[k] = xsect[i];
			}
		}
		else {
			enstrx[k] = xsect[lsec]; //xsect[count-1];
		}
		st += ds;
	}

	xsect[0] = x0;
	yline[strlin-1][0] = y0;
	yline[strlin][0] = y1;
}

#endif

/*	Solve for t in x0 + c.t + b.t^2 +a.t^3 = 0	*/

int xinters(REAL x0,REAL a,REAL b,REAL c,REAL *x,REAL x1)
{
	REAL slope,dx;
	int count;

	*x = x1;
	for(count = 0 ; count < 20 ; count++) {
		slope = c + (*x)*(2.0*b + 3.0*a*(*x));
		if(slope != 0.0) {
			dx = (x0 + (*x)*(c + (*x)*(b + (*x)*a)))/slope;
		}
		else {
			dx = 0.1;
		}
		*x -= dx;
		if(fabs(dx) < 0.0001) return 1;
	}
	return 0;
}

#ifndef STUDENT

/*	Write all stringer (x,y,z) information to a text file	*/

MENUFUNC write_stringers()
{
#ifdef DEMO
	return;
#else
	FILE *fp;
	int i,j,s;
#ifdef SHAREWARE
	nagbox();
#endif
	cls(FALSE);
	if(strlin < 0 || !open_text(&fp,filedirnam,"*.txt")) return;

	/*	Write the information to the file	*/

	for(s = 0 ; s < extlin ; s++) {
		for(j = inistr[s] ; j < inistr[s]+numstr[s] ; j++) {
			fprintf(fp,"LINES %d-%d, STRINGER %d",s,s+1,j + inistr[s] + 1);
			i = ststr[j];
			if(!wrxyzunit(fp,i,ststrx[j],ystr[j][i],zstr[j][i])) return;
			if(ststrx[j] != xsect[i]) fprintf(fp,"*");
			for(i++ ; i < enstr[j] ; i++) {
				if(!wrxyzunit(fp,i,xsect[i],ystr[j][i],zstr[j][i])) return;
			}
			i = enstr[j];
			if(!wrxyzunit(fp,i,enstrx[j],ystr[j][i],zstr[j][i])) return;
			if(enstrx[j] != xsect[i]) fprintf(fp,"*");
			fprintf(fp,"\n");
		}
	}
	fclose(fp);
#endif
}

#ifndef DEMO
INT wrxyzunit(FILE *fp,INT nl,REAL x,REAL y,REAL z)
{
	INT	i1,i2,i3;
	REAL	x1,x2,x3;
	extern	REAL	invert;
	extern	INT	numun;

	if(x < 0.0) return(1);	/* don't write out negative offsets */
	z *= invert;		/* convert to user's convention		*/

	if(numun <= 1) {
		if(fprintf(fp,"\n%8.3f%8.3f%8.3f%6d",x,y,z,nl) < 21) return(0);
	}
	else {
		i1 = x;
		x1 = (x - (float) i1)*12.0;
		i2 = y;
		x2 = (y - (float) i2)*12.0;
		if(i2 < 0) x2 = -x2;
		i3 = z;
		x3 = (z - (float) i3)*12.0;
		if(fprintf(fp,"\n%3d'%6.2f\" %3d'%6.2f\" %3d'%6.2f\"%6d",i1,x1,i2,x2,i3,x3,nl) < 34) return(0);
	}
	return(1);
}
#endif

MENUFUNC develop_stringer()
{
	int sts,ens;
	REAL stx,enx;
	int i,j;
	int other;
	int truedev = 0;
	REAL strwid;

	cls(FALSE);
	if(strlin < 0) {
		for(i = 0 ; i < extlin ; i++) {
			if(numstr[i] > 0) {
				strlin = i;
				break;
			}
		}
		if(strlin < 0) return;
	}

	i = strlin + 1;
	j = strind + 1;
	if(!getdlg(DEVESTRI,
		INP_INT,(void *) &i,
		INP_INT,(void *) &j,
		INP_RBN,(void *) &truedev,
		-1)) return;

	strlin = i - 1;
	strind = j - 1;

	strwid = str_wid[strlin];

	if(numstr[strlin] == 0) {
		message("No stringers defined\nfor this line pair");
		return;
	}

	if(j > numstr[strlin]) {
		message("This stringer not defined\nfor this line pair");
		return;
	}

	if(strwid <= 0.0) {
		message("Stringer width must be positive");
		return;
	}

	j = inistr[strlin] + strind;
	plate_lin = extlin+2;
	other = extlin + 1;
	sts = ststr[j];
	ens = enstr[j];
	stx = xsect[sts];
	enx = xsect[ens];
	stsec[other] = stsec[plate_lin] = sts;
	ensec[other] = ensec[plate_lin] = ens;
	xsect[sts] = ststrx[j];
	xsect[ens] = enstrx[j];
	relcont[plate_lin] = TRUE;

	for(i = sts ; i <= ens ; i++) {
		yline[plate_lin][i] = ystr[j][i];
		zline[plate_lin][i] = zstr[j][i];
		ycont[plate_lin][i] = yline[plate_lin-1][i];
		zcont[plate_lin][i] = zline[plate_lin-1][i];
		yline[other    ][i] = ystr[j][i] + cos(astr[j][i])*strwid;
		zline[other    ][i] = zstr[j][i] - sin(astr[j][i])*strwid;
	}
	yline[other    ][maxsec+1] = yline[other    ][maxsec] = 0.0;	/* end curves */
	zline[plate_lin][maxsec+1] = zline[plate_lin][maxsec] = 0.0;
	radstem[other    ] = 0.0;
	radstem[plate_lin] = 0.0;

	if(truedev) {
		calc_devel();
	}
	else {
		false_devel();
	}

	xsect[sts] = stx;
	xsect[ens] = enx;
}

int realloc_stringers(int k)
{
	int i,result;

	if(k) {
		result = altavail((void far *) &ystr,k*sizeof(*ystr));
		if(result) result = altavail((void far *) &zstr,k*sizeof(*zstr));
		if(result) result = altavail((void far *) &astr,k*sizeof(*astr));
		if(result) result = altavail((void far *) &ststr,(k+1)*sizeof(int));
		if(result) result = altavail((void far *) &enstr,k*sizeof(int));
		if(result) result = altavail((void far *) &ststrx,k*sizeof(REAL));
		if(result) result = altavail((void far *) &enstrx,k*sizeof(REAL));
		if(!result) {
			message("Out of memory in stringer calculation");
		}

	}
	else {
		memfree((void far *) ystr);
		memfree((void far *) zstr);
		memfree((void far *) astr);
		memfree((void far *) enstr);
		memfree((void far *) ststrx);
		memfree((void far *) enstrx);
		ystr = NULL;
		zstr = NULL;
		astr = NULL;
		enstr = NULL;
		ststrx = NULL;
		enstrx = NULL;
		result = altavail((void far *) &ststr,sizeof(int));
		if(result) ststr[0] = -1;
		for(i = 0 ; i < maxlin ; i++) numstr[i] = 0;
	}
	return result;
}

void draw_stringers(REAL s1,REAL s2,int mode,int iswap)
{
	int i,j,k,is,ie;
	int ignore;
	REAL stx,enx;
	REAL ystem = yline[stemli][1];
	int dxf = (mode >= 4 && mode <= 7);
	int savon[maxsec];
	REAL *dummy;

	if(!showstringers) return;

	if(!memavail((void *) &dummy,maxsec*maxext*4)) return;

	yline[extlin][maxsec] = 2.0;
	zline[extlin][maxsec] = 2.0;
	yline[extlin][maxsec+1] = 1.0;
	zline[extlin][maxsec+1] = 1.0;
	radstem[extlin] = 0.0;

	(*colour)(6);

	for(j = 0 ; j < numlin ; j++) {
		if(numstr[j] > 0) {
			for(k = inistr[j] ; k < inistr[j] + numstr[j] ; k++) {
				is = ststr[k];
				ie = enstr[k];
				stx = xsect[is];
				enx = xsect[ie];
				xsect[is] = ststrx[k];
				xsect[ie] = enstrx[k];
				for(i = is ; i <= ie ; i++) {
					savon[i] = secton[i];
					if(ystr[k][i] < -100000.0) secton[i] = 0;
					yline[extlin][i] = ystr[k][i];
					zline[extlin][i] = zstr[k][i];
					linewt[extlin][i] = 1.0;
				}
				stsec[extlin] = ststr[k];
				ensec[extlin] = enstr[k];
				/*		if(is > 0) yline[stemli][1] = max(0.0,yline[extlin][0]);	*/
				yline[extlin][0] = xsect[0] - ststrx[k];
				yline[extlin][maxsec  ] = 0.0;
				yline[extlin][maxsec+1] = 0.0;
				zline[extlin][maxsec  ] = 0.0;
				zline[extlin][maxsec+1] = 0.0;

				ycont[extlin][maxsec  ] = 0.0;
				ycont[extlin][maxsec+1] = 0.0;
				zcont[extlin][maxsec  ] = 0.0;
				zcont[extlin][maxsec+1] = 0.0;

				if(dxf) ensure_polyline('G',0,mode == 7);
				draw_line(extlin,s1,s2,mode,is,ie,iswap,
					dummy,dummy,dummy,&ignore,&ignore,&ignore,1);
				if(dxf) seqend();
				xsect[is] = stx;
				xsect[ie] = enx;
				for(i = is ; i <= ie ; i++) secton[i] = savon[i];
			}
		}
	}
	yline[stemli][1] = ystem;
	memfree(dummy);
}

MENUFUNC delete_stringers()
{
	int index = 1;
	int i;
	struct {
		int index;				/* index of result in table */
		char *string;			/* pointer to result (not used) */
		char *table[maxlin];	/* table of strings for listbox, null string terminator */
	}
	xlist;
	char text[MAX_PATH] = "";
	char listtext[maxlin][40];

	cls(FALSE);

	for(i = 1 ; i < numlin ; i++) {
		if(numstr[i] <= 0) {
			sprintf(listtext[i-1],"%d - %d: no stringers defined",i,i+1);
		} else if(strmode[i] == 0){	// constant interval
			sprintf(listtext[i-1],"%d - %d: constant interval = %s",i,i+1,floattext(str_interv[i]));
		} else {
			sprintf(listtext[i-1],"%d - %d: constant count = %d",i,i+1,(int) str_interv[i]);
		}
		xlist.table[i-1] = listtext[i-1];
	}
	xlist.table[numlin-1] = "";
	xlist.index = -1;
	xlist.string = text;

	if(getdlg(DELESTRI,
	INP_LBX,(void *) &xlist,
	-1) ) {
		i = xlist.index;
		if(i >= 0 && i < numlin) remove_stringers(i + 1);
	}
}

void remove_stringers(int strlin)
{
	int nstr = numstr[strlin];
	int istr = inistr[strlin];
	int k = 0;
	int i,j,jj;

	if(nstr > 0) { /* nstr is number of stringers for the current line pair */

		/*	Calculate location of last stringer		*/

		for(j = 0 ; j < extlin ; j++) {
			jj = numstr[j];
			if(jj >= 0) k += jj;
		}

		/*	Close up the table from the initial stringer line (istr)
		to the last (istr+nstr-1)			*/

		for(j = istr, jj = istr + nstr ; jj < k ; j++, jj++) {
			for(i = ststr[jj] ; i <= enstr[jj] ; i++) {
				ystr[j][i] = ystr[jj][i];
				zstr[j][i] = zstr[jj][i];
				astr[j][i] = astr[jj][i];
			}
			ststr [j] = ststr [jj];
			enstr [j] = enstr [jj];
			ststrx[j] = ststrx[jj];
			enstrx[j] = enstrx[jj];
		}
		numstr[strlin] = 0;	/* str_interv was also set zero here - what for? */
		for(j = 0 ; j < extlin ; j++) {
			if(inistr[j] > istr) inistr[j] -= nstr;
		}

		/*	Discard released stringer memory	*/

		k -= nstr;
		if(!realloc_stringers(k)) return;
	}
}
#endif

#endif

/*	Return the distance along the curve (arbitrary origin)	*/

REAL curvedist(REAL t,REAL *deriv)
{
	REAL u,uu,uusum;
	if(C > 0.0) {
		if(divisor == 0.0) {
			u = B + 2.0*C*t;
			uu = fabs(u);
			*deriv = 0.5/sqC*uu;
			return (0.125/(C*sqC))*uu*u;
		}
		else {
			u = u0 + u1*t;
			if(divisor > 0.0) {
				uu = sqrt(u*u + 1.0);
				uusum = (u > -1000.0) ? uu + u : -0.5/u;
				*deriv = sqdiv * uu;
				return 0.5*divisor/sqC*(log(uusum)+u*uu);
			}
			else {
				uu = sqrt(u*u - 1.0);
				uusum = (u > -1000.0) ? uu + u : 0.5/u;
				*deriv = -sqdiv * uu;
				return 0.5*divisor/sqC*(log(uusum)-u*uu);
			}
		}
	}
	else {
		*deriv = u1;
		return t*u1;
	}
}

/*	Precalculate all parameters required in stringer iteration	*/

void stringer_param(REAL a,REAL hb,REAL c,REAL hd)
{
	b = hb + hb;
	d = hd + hd;
	A = a*a + c*c;
	B = 2.0*(a*b+c*d);
	C = b*b + d*d;
	if(C > A*1.0e-8) {	/* if C == 0, the following are not needed */
		sqC = sqrt(C);
		divisor = A - (B*B)/(4.0*C);
		if(divisor > 0.00001) {
			sqdiv = sqrt(divisor);
			u0 = 0.5*B/(sqdiv*sqC);
			u1 = sqC/sqdiv;
		}
		else if(divisor < -0.00001) {
			sqdiv = sqrt(-divisor);
			u0 = 0.5*B/(sqdiv*sqC);
			u1 = sqC/sqdiv;
		}
		else {
			divisor = 0.0;
		}
	}
	else {
		C = 0.0;
		B = 0.0;
		b = 0.0;
		d = 0.0;
		u1 = sqrt(A);
	}
}

REAL solvecurve(REAL s0,REAL t,REAL dt)
{
	int j = 0;
	REAL s,st;
	REAL ignore;
	if(C != 0.0) {	/* if C=0, straight line		*/
		do {
			s = curvedist(t,&st) - s0;
			if(st != 0.0)
				dt = s / st;
			else
				dt = 0.01;
			t -= dt;
		}
		while(++j < 10 && fabs(dt) > 0.0001);
	}
	else if(s0 > 0.0 && A > 0.0) {	/* straight curve segment */
		t = (s0 - curvedist(0.0,&ignore)) / sqrt(A);
	}
	else {
		t = -1.0;
	}
	return t;
}

