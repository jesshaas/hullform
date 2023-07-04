/* Hullform component - newhull8.c
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

int getround(int rounded[]);
extern int *relcont;
void realloc_stringers(int);
int newcount;
void edit_alte(void);
#ifdef EXT_OR_PROF
void use_hull(int);
void save_hull(int);
#endif
extern int *autofair;
void fair_controls(void);

/*	start a new hull from scratch			*/

MENUFUNC new_hull()
{
	REAL	temp,temp1,temp2,ctemp2;
	int		i,j,i0,i1,i2;
	extern int	helpind,help[];
	REAL	maxbeam,tranbeam,draft,stemfb,sternfb;
	int		sectform,atwaterline,atkeel;
	int		rounded[maxlin];
	REAL	z[4];
	REAL	length;
	REAL	rcont,angle;
	int		hulltype;
	extern HWND	hWndMain;
	REAL	stemrake;
	extern int *editsectnum;
	REAL	x,dx;
	void	edit_rese_perform(void);
	extern	int newcou;
	extern	REAL xnew[];
#ifndef STUDENT
	void	remove_stringers(int);
#endif

#ifdef linux
	void changemode(Widget w,XtPointer client_data,XtPointer call_data);
#else
	
	void changemode(int,HWND);
#endif

#ifdef EXT_OR_PROF
	use_hull(MAINHULL);
#endif

	if(count != 0) {
		if(MessageBox(hWnd,"Do you want to delete the existing hull from memory?","CAUTION",
			MB_YESNO) == IDNO) return;
	}

#ifndef STUDENT
	realloc_stringers(0);
	for(j = 0 ; j < maxlin ; j++) numstr[j] = 0;
#endif

	count = 0;		/* remove old hull completely	*/
	hullfile[0] = 0;

#ifndef linux

#ifdef HULLSTAT
	SetWindowText(hWndMain,"HULLSTAT");
#else
	SetWindowText(hWndMain,"HULLFORM");
#endif

#endif

	/*	Determine overall hull parameters and dimensions	*/

	numlin = 2;
	length = numun < 2 ? 15.0 : 50.0;	/* overall length */
	maxbeam = 0.3 * length;				/* maximum beam */
	tranbeam = 0.18 * length;			/* transom beam */
	stemrake =  0.05 * length;			/* stem rake */
	draft = 0.05 * length;				/* maximum draft */
	stemfb = 0.08 * length;				/* stem freeboard */
	sternfb = 0.06 * length;			/* stern freeboard */
	sectform = 1;						/* chine default 0, rounded default 1, individual 2 */
	atkeel = 1;							/* baseline defaults to keel */
	hulltype = 0;
	newcount = 12;
	surfacemode = 0;

	if(getdlg(EDITNEWH,
		INP_INT,(void *) &numlin,
		INP_INT,(void *) &newcount,
		INP_REA,(void *) &length,
		INP_REA,(void *) &maxbeam,
		INP_REA,(void *) &tranbeam,
		INP_REA,(void *) &stemrake,
		INP_REA,(void *) &draft,
		INP_REA,(void *) &stemfb,
		INP_REA,(void *) &sternfb,
#ifdef EXT_OR_PROF
		INP_PBF,(void *) changemode,
#endif
		INP_RBN,(void *) &hulltype,
		INP_RBN,(void *) &sectform,
		INP_RBN,(void *) &atkeel,
		-1)) {
		atwaterline = !atkeel;
		if(surfacemode) stemrake = 0.0;

		if(numlin < 2) {
			message("At least two hull lines are needed");
			return;
		}

		if(numlin > maxlin) {
			message("Too many hull lines nominated");
			return;
		}

		if(newcount <= 1) {
			message("Need at least two sections");
			return;
		}

		if(length - stemrake <= 0.0) {
			message("Length along keel must be positive");
			return;
		}

		if(atwaterline)
			wl = 0.0;
		else
			wl = -draft;

		if(sectform < 2) {
			for(i = 0 ; i < numlin ; i++) rounded[i] = sectform;
		}
		else {
			getround(rounded);
		}

		i0 = surfacemode;
		i1 = surfacemode+1;
		i2 = surfacemode+2;

		z[i0] = wl;
		z[i1] = wl;
		z[i2] = wl;

		/*	Negative displacement is surface mode flag	*/

		if(surfacemode) {
			disp = -1.0;
		}
		else {
			disp = 0.0;
			z[i1] += draft;
			if(hulltype == 2) z[i2] += 0.75*draft;
		}

		/*	array reallocations			*/

		extlin = numlin;
		if(!realloc_hull(extlin)) return;

		/*	Sheerline vertical offsets		*/
		zline[0][i0] = wl - stemfb;
		zline[0][i1] = wl - 0.5*(stemfb+sternfb);
		zline[0][i2] = wl - sternfb;

		/*	Sheerline lateral offsets		*/
		yline[0][i0] = surfacemode ? 0.50 * maxbeam : stemrake;
		yline[0][i1] = 0.50 * maxbeam;
		yline[0][i2] = 0.50 * tranbeam;

		/*	Section positions			*/
		xsect[i0] = surfacemode ? 0.0 : yline[0][0];
		xsect[i1] = 0.5*length;
		xsect[i2] = length;

		for(j = 0 ; j < numlin ; j++) {
#ifndef STUDENT
			radstem[j] = 0.0;
#endif
			stsec[j] = i0;		/* default start and end sections */
			ensec[j] = i2;
			relcont[j] = 1;
		}

		angle = 1.5707963 / ((float) (numlin-1));
		rcont = 1.0/cos(0.5*angle);
		for(j = 1 ; j < numlin ; j++) {
			temp2 = angle * (float) j;

			/*	    ctemp1 = cos(temp2 - 0.5*angle);	*/
			ctemp2 = sin(temp2 - 0.5*angle);

			temp1 = cos(temp2);
			temp2 = sin(temp2);

			for(i = i0 ; i <= i2 ; i++) {	/* ... for each section */

				/*		Get y- and z- offsets from sheerline and keel, using	*/
				/*		sine/cosine factors					*/
				yline[j][i] = temp1 * yline[0][i];
				zline[j][i] = zline[0][i] + temp2 * (z[i] - zline[0][i]);
				ycont[j][i] = 0.0;
				if(rounded[j])
					zcont[j][i] = zline[0][i] + rcont*ctemp2*(z[i] - zline[0][i]);
				else
				    zcont[j][i] = zline[j-1][i];
			}
		}

		/*	stem section (data ignored in surface mode, but left for completeness)	*/

		if(surfacemode) {
			for(j = 0 ; j < numlin ; j++) {
				yline[j][0] = 0.0;
				zline[j][0] = 0.0;
				ycont[j][0] = 0.0;
				zcont[j][0] = 0.0;
			}
		}
		else {
			temp1 = 1.0 / ((float) (numlin - 1));
			temp = 1.0;
			for(j = 1 ; j < numlin ; j++) {
				temp -= temp1;
				yline[j][0] = yline[0][0] * temp;
				zline[j][0] = (zline[0][0] - wl) * temp + wl;
				ycont[j][0] = 0.0;
				zcont[j][0] = 0.5*(zline[j][0]+zline[j-1][0]);
			}
			ycont[1][0] = 0.5*(yline[1][0]-yline[0][0]);
		}

		stemli = numlin -1;
		count = surfacemode + 3;	/* new hull only available when details completed */

		/*	clear dummy values for line 1			*/

		for(i = i0 ; i <= i2 ; i++) {
			ycont[0][i] = 0.0;
			zcont[0][i] = zline[0][i];
		}

		xsect[maxsec] = 0.0;
		xsect[maxsec+1] = 0.0;
		extlin = numlin;

		/*	Set up end curve factors, depending on hull type	*/

		switch(hulltype) {
		case 0:	/* yacht */
			for(j = 0 ; j < numlin ; j++) {
				yline[j][maxsec] = 0.0;
				yline[j][maxsec+1] = 0.0;
				zline[j][maxsec] = 0.0;
				zline[j][maxsec+1] = 0.0;
			}
			zcont[0][numlin-1] = zline[0][numlin-1];
			if(numlin > 2)
				ycont[0][numlin-1] = 0.0;
			else
			    ycont[0][1] = yline[0][1]-yline[0][0];
			break;
		case 1:	/* workboat */
			for(j = 0 ; j < numlin ; j++) {
				yline[j][maxsec] = 5.0;
				yline[j][maxsec+1] = 1.0;
				zline[j][maxsec] = 5.0;
				zline[j][maxsec+1] = 5.0;
			}
			break;
		case 2:	/* planing */
			for(j = 0 ; j < numlin ; j++) {
				yline[j][maxsec] = 2.0;
				yline[j][maxsec+1] = 0.0;
				zline[j][maxsec] = 5.0;
				zline[j][maxsec+1] = 0.0;
			}
			break;
		}

#ifndef STUDENT

		/*	default is no floodable tanks, and no transom		*/

		ntank = 0;
		fl_line1[0] = numlin;
		for(j = 1 ; j < maxlin+3 ; j++) developed[j] = -1;
		for(j = 1 ; j < maxlin ; j++) radstem[j] = 0.0;

		atransom = 0.0;
		stransom = 0.0;
		ctransom = 1.0;
		transom = 0;
		dztransom = 0.0;
		redef_transom();	/* remove transom		*/
		recalc_transom();
#endif

		for(i = 0 ; i < count ; i++) {
			master[i] = TRUE;
			editsectnum[i] = TRUE;
		}
		for(j = 0 ; j < extlin ; j++) {
			autofair[j] = TRUE;
			for(i = 0 ; i < count ; i++) linewt[j][i] = 1.0;
		}
		strcpy(alsosect,"ALL");

		dx = (xsect[2] - xsect[0]) / (REAL) (newcount-1);
		x = xsect[0];
		for(i = 0 ; i < newcount ; i++) {
			xnew[i] = x;
			x += dx;
		}
		newcou = newcount;
		edit_rese_perform();

		/*	Final adjustment for chine hull		*/

		for(j = 1 ; j < numlin ; j++) {
			if(!rounded[j]) {
				for(i = 0 ; i < count ; i++) {
					if(zcont[j][i] < zline[j-1][i]) zcont[j][i] = zline[j-1][i];
				}
			}
		}

		newcount = 0;
		for(i = 0 ; i < count ; i++) {
			master[i] = FALSE;
			editsectnum[i] = TRUE;
		}
		master[surfacemode] = TRUE;
		master[count/2] = TRUE;
		master[count-1] = TRUE;
#ifdef EXT_OR_PROF
		save_hull(MAINHULL);
#endif
	}
}

#ifdef linux
void changemode(Widget w,XtPointer client_data,XtPointer call_data)
{
	XmString xstr;
	extern Widget wLabel[],wPushButton[];
	if(surfacemode) {
		xstr = XmStringCreateSimple("Now in normal mode");
		XtVaSetValues(wLabel[0],XmNlabelString,xstr,NULL);
		XmStringFree(xstr);
		xstr = XmStringCreateSimple("cHange to surface mode");
		XtVaSetValues(wPushButton[0],XmNlabelString,xstr,NULL);
		XmStringFree(xstr);
		surfacemode = 0;
	}
	else {
		xstr = XmStringCreateSimple("Now in surface mode");
		XtVaSetValues(wLabel[0],XmNlabelString,xstr,NULL);
		XmStringFree(xstr);
		xstr = XmStringCreateSimple("cHange to normal mode");
		XtVaSetValues(wPushButton[0],XmNlabelString,xstr,NULL);
		XmStringFree(xstr);
		surfacemode = 1;
	}
}
#else

void changemode(int code,HWND hWndDlg)
{
	if(surfacemode) {
		SetDlgItemText(hWndDlg,DLGSTAT+0,"Now in normal mode");
		SetDlgItemText(hWndDlg,DLGPBF+0,"c&Hange to surface mode");
		surfacemode = 0;
		EnableWindow(GetDlgItem(hWndDlg,DLGEDIT+5),TRUE);
		EnableWindow(GetDlgItem(hWndDlg,DLGTEXT+5),TRUE);
	}
	else {
		SetDlgItemText(hWndDlg,DLGSTAT+0,"Now in surface mode");
		SetDlgItemText(hWndDlg,DLGPBF+0,"c&Hange to normal mode");
		surfacemode = 1;
		EnableWindow(GetDlgItem(hWndDlg,DLGEDIT+5),FALSE);
		EnableWindow(GetDlgItem(hWndDlg,DLGTEXT+5),FALSE);
	}
}
#endif

int getround(int rounded[])
{
	int i,result;
	char message[40];

	rounded[0] = 1;
	for(i = 1 ; i < numlin ; i++) {

		sprintf(message,"Is curve from line %d to %d rounded?",i,i+1);
		result = MessageBox(hWnd,message,"Definition of inter-line curvature",
			MB_YESNOCANCEL);
		if(result == IDCANCEL) return 0;
		rounded[i] = (result == IDYES);
	}
	return 1;
}

