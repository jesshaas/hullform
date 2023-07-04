/* Hullform component - edit_men.c
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

#ifdef linux

#include <Xm/Xm.h>
#include <Xm/Text.h>
#include <X11/Intrinsic.h>

#endif

extern char *helpfile;
extern int changed;
void get_int_angle(int j,REAL *c,REAL *s);
void fix_round_ends(void);
void recalc_tanks(void);
extern int graphic_edit;
void reset_all(void);
INT	nr;
extern int *relcont;
void line_properties(int);
void save_hull(int choice);

#ifndef STUDENT
extern char (*sectname)[12];
#endif
int dlgresult;

extern char *ignline;
extern int *ignore;

extern int showcentres,shownumbers,shownames;

#ifndef STUDENT
void check_invalid_end(void);
#endif
int ColourMessage(unsigned msg,WORD wParam,LONG lParam,BOOL *hBrush);

#ifdef linux
int AddSectDialog(void);
int DelSectDialog(void);
int InsLineDialog(void);
int RemLineDialog(void);
#else
BOOL CALLBACK EditAddDlgProc(HWND hWndDlg,unsigned msg,WORD wParam,LONG lParam);
BOOL CALLBACK EditDelDlgProc(HWND hWndDlg,unsigned msg,WORD wParam,LONG lParam);
BOOL CALLBACK EditInsDlgProc(HWND hWndDlg,unsigned msg,WORD wParam,LONG lParam);
BOOL CALLBACK EditRemDlgProc(HWND hWndDlg,unsigned msg,WORD wParam,LONG lParam);
#endif

void free_context(void);

/*      Edit menu functions and function table                          */

MENUFUNC add_section()
{
	INT	i,j,k;
	REAL xtemp;
	int		code;
	int	prev_scrdev = scrdev;

	scrdev = 1;

	/*	ENSURE SECTION TABLE WILL NOT BE OVERFILLED.  NOTE THAT WE NEED	*/
	/*	SPACE FOR THE SECTION, AND SPACE TO OPEN A "HOLE" FOR IT LATER -*/
	/*	HENCE "maxsec-2".						*/

	if(count >= maxsec-2) {
		message("SECTION TABLE FULL");

	}
	else if(count == 0) {	/* must use 'New hull' first,  */
		/* to allocate memory for data */
		message("Use 'File, New hull' to create the first three sections");

	}
	else {

		strcpy(ignline,"NONE");
#ifdef linux
		dlgresult = AddSectDialog();
#else

		code = DialogBox(hInst,(char *) EDITADD,hWnd,(DLGPROC) EditAddDlgProc);
		if(code < 0) (void) MessageBox(hWnd,"EDITADD","Could not create dialog box",MB_OK);

#endif
		if(dlgresult) {

			multproc(ignline,ignore,maxsec);
			xtemp = xsect[0];
			if(!surfacemode) xtemp -= yline[stemli][0];
			if(xsect[maxsec-1] <= xtemp)
				message("Sections within the stem are intended for internal use by Hullform's algorithms. Their use in general design work is NOT SUPPORTED! You have been warned!");

			/*	USE EXISTING LINES TO CALCULATE SPLINE-FIT OFFSETS HERE	*/

			for(i = 0 ; i < extlin ; i++) {
				refit(i,xsect,yline[i],zline[i],ycont[i],zcont[i],maxsec-1,maxsec-1);
			}

			changed = 1;

			message("You should now use the Edit menu's 'Edit a Section' facility to finish the new section.");

			for(i = 1 ; i < count ; i++) {
				if(xsect[i] >= xsect[maxsec-1]) break;
			}

			if(i < count && xsect[i] == xsect[maxsec-1]) {

				message("CANNOT INSERT ONTO EXISTING SECTION");

			}
			else {

				/*		MAKE SPACE HERE	*/

				for(j = count ; j > i ; j--) {
					xsect[j] = xsect[j-1];
#ifndef STUDENT
					strcpy(sectname[j],sectname[j-1]);
#endif
					master[j] = master[j-1];
					for(k = 0 ; k < extlin ; k++) {
						yline[k][j] = yline[k][j-1];
						zline[k][j] = zline[k][j-1];
						ycont[k][j] = ycont[k][j-1];
						zcont[k][j] = zcont[k][j-1];
						linewt[k][j] = linewt[k][j-1];
					}
				}
#ifndef STUDENT
				sprintf(sectname[i],"%d",i);
#endif

				/*		Then alter start and end section for each existing line	*/

				for(k = 0 ; k < extlin ; k++) {
					if(stsec[k] >= i) stsec[k]++;
					if(ensec[k] >= i ||
						    i == count && ensec[k] == i-1) ensec[k]++;
				}

				/*		THEN INSERT IT	*/

				xsect[i] = xsect[maxsec-1];
				for(k = 0 ; k < extlin ; k++) {
					if(stsec[k] > i || ensec[k] < i) continue;
					yline[k][i] = yline[k][maxsec-1];
					zline[k][i] = zline[k][maxsec-1];
					ycont[k][i] = ycont[k][maxsec-1];
					zcont[k][i] = zcont[k][maxsec-1];
					linewt[k][i] = 1.0;
				}

				count++;
#ifndef STUDENT
				check_invalid_end();
#endif
			}		/* end non-match of existing section test*/
		}
		reset_all();
#ifndef STUDENT
		redef_transom();
		recalc_tanks();
#endif
#ifdef EXT_OR_PROF
		save_hull(MAINHULL);
#endif
	}
	scrdev = prev_scrdev;
	setup(scrdev);
}

#ifndef linux

BOOL CALLBACK EditAddDlgProc(HWND hWndDlg,unsigned msg,WORD wParam,LONG lParam)
{
	void centre_dlg(HWND);
	void GetSizes(HWND);
	char text[80];
	char *p;
	int i;
	extern HWND hWndMain;
	extern HDC hDC;
	extern REAL xgslope,xmin;
	static HWND hSave;
	extern int context_id,HelpUsed;
	extern int scrollable;
	extern HBRUSH bg;
	BOOL colret;
	static int	grdev;
	int		save_showcentres,save_shownumbers,save_shownames;

	if(ColourMessage(msg,wParam,lParam,&colret)) return colret;

	switch(msg) {

	case WM_INITDIALOG:
		hSave = hWnd;
		grdev = scrdev;
		free_context();
		scrdev = 1;
		setup(scrdev);
		scrollable = FALSE;
		hWnd = GetDlgItem(hWndDlg,300);
		hDC = GetDC(hWnd);
		GetSizes(hWnd);
		centre_dlg(hWndDlg);
		SetDlgItemText(hWndDlg,DLGEDIT+1,ignline);
		SetDlgItemText(hWndDlg,DLGEDIT+0,"");
		SendDlgItemMessage(hWndDlg,DLGLBX,LB_RESETCONTENT,0,0l);
		for(i = 0 ; i < count ; i++) {
			sprintf(text," %d at %.4f",i,xsect[i]);
			SendDlgItemMessage(hWndDlg,DLGLBX,LB_ADDSTRING,0,(LPARAM) (LPCSTR) text);
		}
		break;

	case WM_PAINT:
		save_showcentres = showcentres;
		save_shownumbers = shownumbers;
		save_shownames = shownames;
		showcentres = FALSE;
		shownumbers = TRUE;
		shownames = FALSE;
		elev_orth();
		showcentres = save_showcentres;
		shownumbers = save_shownumbers;
		shownames = save_shownames;
		update_func = NULL;
		break;

	case WM_COMMAND:
		switch(wParam) {
		case IDOK:
			GetDlgItemText(hWndDlg,DLGEDIT,text,sizeof(text));
			sscanf(text,"%f",&xsect[maxsec-1]);
			GetDlgItemText(hWndDlg,DLGEDIT+1,ignline,MAX_PATH);

		case IDCANCEL:
			dlgresult = (wParam == IDOK);
			EndDialog(hWndDlg,dlgresult);
			ReleaseDC(hWnd,hDC);
			hDC = NULL;
			hWnd = hSave;
			scrdev = grdev;
			setup(scrdev);
			break;

		case IDHELP:	/* Help */
			context(context_id);
			break;

		}
		break;

	case WM_LBUTTONUP:
		i = LOWORD(lParam) - 310;
		if(i > 0 && i <= 400) {
			sprintf(text,"%.4f",xmin + ((REAL) i + 0.5) / xgslope);
			p = strchr(text,0);
			while(*--p != '.') if(*p != '0') break;
			if(*p != '.') p++;
			*p = 0;
			SetDlgItemText(hWndDlg,DLGEDIT,text);
		}
		break;
	}
	return 0;
}

#endif

int delsel = 0;

MENUFUNC delete_section()
{

	/*	REMOVE REQUESTED SECTION AND CLOSE UP TABLE	*/

	INT	i,j,k;
	char text[80];
	REAL xxstem;
	int		code;
	int	prev_scrdev = scrdev;

	scrdev = 1;

#ifdef linux
	dlgresult = DelSectDialog();

#else

	code = DialogBox(hInst,(char *) EDITDELE,hWnd,(DLGPROC) EditDelDlgProc);
	if(code < 0) (void) MessageBox(hWnd,"EDITDELE","Could not create dialog box",MB_OK);

#endif

	if(dlgresult) {

		nr = delsel;
		if(nr == 0) {

			/*	Special warning if request is to delete stem			*/

			i = MessageBox(hWnd,"Do you really mean to delete the stem?",
				"Delete a Section",MB_YESNO) == IDYES;

		}
		else {

			/*	General warning for other sections				*/

			xxstem = xsect[0] - dxstem();
			sprintf(text,
				"Do you mean station %d, at %.3f %s from the stem end?",nr,
				xsect[nr]-xxstem,lenun[numun]);
			i = MessageBox(hWnd,text,"Delete a Section",MB_YESNO) == IDYES;
		}

		if(i) {
			count--;
			for(j = nr ; j < count ; j++) {
				xsect[j] = xsect[j+1];
#ifndef STUDENT
				strcpy(sectname[j],sectname[j+1]);
#endif
				master[j] = master[j+1];
				for(k = 0 ; k < extlin ; k++) {
					yline[k][j] = yline[k][j+1];
					zline[k][j] = zline[k][j+1];
					ycont[k][j] = ycont[k][j+1];
					zcont[k][j] = zcont[k][j+1];
					linewt[k][j] = linewt[k][j+1];
				}
			}

			/*	Then alter start and end section for each existing line	*/

			for(k = 0 ; k < extlin ; k++) {
				if(stsec[k] >  nr) stsec[k]--;
				if(ensec[k] >= nr) ensec[k]--;
			}
			changed = 1;
			reset_all();
		}
	}
#ifndef STUDENT
	redef_transom();
	recalc_tanks();
#endif

#ifdef EXT_OR_PROF
	save_hull(MAINHULL);
#endif
	scrdev = prev_scrdev;
	setup(scrdev);
}

#ifndef linux

BOOL CALLBACK EditDelDlgProc(HWND hWndDlg,unsigned msg,WORD wParam,LONG lParam)
{
	void centre_dlg(HWND);
	void GetSizes(HWND);
	char text[80];
	int i,in;
	extern HWND hWndMain;
	extern HDC hDC;
	extern REAL xgslope,xmin;
	static HWND hSave;
	extern int context_id,HelpUsed;
	extern int scrollable;
	RECT rc;
	REAL x,xd,xm;
	BOOL colret;
	static int	grdev;
	int		save_showcentres,save_shownumbers,save_shownames;

	if(ColourMessage(msg,wParam,lParam,&colret)) return colret;
	switch(msg) {
	case WM_INITDIALOG:
		hSave = hWnd;
		free_context();
		hWnd = GetDlgItem(hWndDlg,300);
		hDC = GetDC(hWnd);
		grdev = scrdev;
		scrdev = 1;
		setup(1);
		scrollable = FALSE;
		GetSizes(hWnd);
		SendDlgItemMessage(hWndDlg,DLGLBX,LB_RESETCONTENT,0,0l);
		for(i = 0 ; i < count ; i++) {
			sprintf(text," %d at %.4f",i,xsect[i]);
			SendDlgItemMessage(hWndDlg,DLGLBX,LB_ADDSTRING,0,(LPARAM) (LPCSTR) text);
		}
		SendDlgItemMessage(hWndDlg,DLGLBX,LB_SETCURSEL,0,0L);
		break;

	case WM_PAINT:
		save_showcentres = showcentres;
		save_shownumbers = shownumbers;
		save_shownames = shownames;
		showcentres = FALSE;
		shownumbers = TRUE;
		shownames = FALSE;
		elev_orth();
		showcentres = save_showcentres;
		shownumbers = save_shownumbers;
		shownames = save_shownames;
		update_func = NULL;
		break;

	case WM_LBUTTONDBLCLK:
		wParam = IDOK;

	case WM_COMMAND:
		switch(wParam) {
		case IDOK:
			delsel = SendDlgItemMessage(hWndDlg,DLGLBX,LB_GETCURSEL,0,0L);

		case IDCANCEL:
			dlgresult = (wParam == IDOK);
			EndDialog(hWndDlg,dlgresult);
			ReleaseDC(hWnd,hDC);
			hDC = NULL;
			hWnd = hSave;
			scrdev = grdev;
			setup(scrdev);
			break;

		case IDHELP:	/* Help */
			context(context_id);
			break;

		}
		break;

	case WM_LBUTTONUP:
		GetWindowRect(hWnd,&rc);
		i = rc.left;
		GetWindowRect(hWndDlg,&rc);
		i -= rc.left;
		x = xmin + ((REAL)(LOWORD(lParam) - i) + 0.5) / xgslope;
		xm = 1.0e+30;
		in = 0;
		for(i = 0 ; i < count ; i++) {
			xd = fabs(x - xsect[i]);
			if(xd < xm) {
				in = i;
				xm = xd;
			}
		}
		SendDlgItemMessage(hWndDlg,DLGLBX,LB_SETCURSEL,in,0L);
		break;
	}
	return 0;
}
#endif

int ind;
extern int inisec,lassec;
REAL tmin;

MENUFUNC insert_line()
{
	INT	i,j;
	REAL a,hb,c,hd,yl,zl;
	int code;
	int prev_scrdev = scrdev;

	scrdev = 1;

	if(count <= 0 || extlin >= maxlin) return;

	ind = 1;
	tmin = 0.5;
	inisec = 0;
	lassec = count-1;

#ifdef linux
	dlgresult = InsLineDialog();

#else

	code = DialogBox(hInst,(char *) INSERTLI,hWnd,(DLGPROC) EditInsDlgProc);
	if(code < 0) (void) MessageBox(hWnd,"INSERTLI","Could not create dialog box",MB_OK);

#endif

	/*	Begin common code	*/

	if(!dlgresult) {
		scrdev = prev_scrdev;
		setup(scrdev);
		return;
	}

	/*	expand array allocations			*/

	if(!realloc_hull(extlin+1)) return;

	/*	MAKE GAP BETWEEN "N-2" AND "N-1", AND INTERPOLATE, */
	/*	ASSUMING SOFT CHINE				 */

	for(j = extlin ; j >= ind ; j--) copyline(j,j-1,0.0);

	ind--;
	if(inisec < surfacemode) inisec = surfacemode;
	if(lassec >= count) lassec = count-1;
#ifndef STUDENT
	stsec[ind] = inisec;
	ensec[ind] = lassec;
#else
	stsec[ind] = 0;
	ensec[ind] = count-1;
#endif

	relcont[ind+1]  = TRUE;
	for(i = inisec ; i <= lassec ; i++) {
		getparam(i,ind,&a,&hb,&c,&hd);

		/*	LOCAL Y AND Z VALUES FOR NEW LINE, AND VERTICAL VALUES FOR NEW	*/
		/*		CONTROL POINTS						*/

		yl = tmin*(a+tmin*hb);
		zl = tmin*(c+tmin*hd);

		/*	HENCE NEW LINE VALUES						*/

		yline[ind][i] = yline[ind+1][i]+yl;
		zline[ind][i] = zline[ind+1][i]-zl;

		/*	AND NEW CONTROL VALUES	*/

		if(relcont[ind]) {
			ycont[ind][i] *= 1.0-tmin;
		}
		else {
			ycont[ind][i] = (1.0-tmin)*ycont[ind][i] + tmin*yline[ind-1][i];
		}
		zcont[ind][i]   = zline[ind+1][i] - (0.5*c*(1.0-tmin)+(c+hd)*tmin);
		linewt[ind][i] = 1.0;

		ycont[ind+1][i] = 0.0;
		zcont[ind+1][i] = zline[ind+1][i] - 0.5*c*tmin;
	}

	/*	STEM LINE MAY NEED TO BE BE INCREMENTED	*/

	if(ind <= stemli+1) stemli++;

	numlin++;
	extlin++;
#ifndef STUDENT
	for(i = 0 ; i <= ntank ; i++) {
		if(fl_line1[i] >= ind) fl_line1[i]++;
	}
	redef_transom();
	recalc_tanks();
#endif
	changed = 1;

#ifdef EXT_OR_PROF
	save_hull(MAINHULL);
#endif
	scrdev = prev_scrdev;
	setup(scrdev);
}

#ifdef linux

#else

BOOL CALLBACK EditInsDlgProc(HWND hWndDlg,unsigned msg,WORD wParam,LONG lParam)
{
	void centre_dlg(HWND);
	void GetSizes(HWND);
	int i,j;
	char text[80];
	extern HWND hWndMain;
	extern HDC hDC;
	extern REAL xgslope,ygslope,xmin,ymin;
	static HWND hSave;
	extern int context_id,HelpUsed;
	extern int scrollable;
	REAL x,y,xd,yd;
	REAL a,b,hb,c,d,hd,xr,yr,A,B,C,D,det,t1,t2,dsq,dsqmin,s;
	int jmin;
	REAL SolveCubic(REAL,REAL,REAL,REAL);
	BOOL colret;
	static int grdev;
	int		save_showcentres,save_shownumbers,save_shownames;

	if(ColourMessage(msg,wParam,lParam,&colret)) return colret;

	switch(msg) {
	case WM_INITDIALOG:
		hSave = hWnd;
		free_context();
		hWnd = GetDlgItem(hWndDlg,300);
		hDC = GetDC(hWnd);
		grdev = scrdev;
		scrdev = 1;
		setup(1);
		scrollable = FALSE;
		GetSizes(hWnd);
		centre_dlg(hWndDlg);
		SetDlgItemInt(hWndDlg,DLGEDIT+0,ind,TRUE);
		sprintf(text,"%.4f",tmin);
		SetDlgItemText(hWndDlg,DLGEDIT+1,text);
#ifndef STUDENT
		SetDlgItemInt(hWndDlg,DLGEDIT+2,inisec,TRUE);
		SetDlgItemInt(hWndDlg,DLGEDIT+3,lassec,TRUE);
#endif
		break;

	case WM_PAINT:
		save_showcentres = showcentres;
		save_shownumbers = shownumbers;
		save_shownames = shownames;
		showcentres = FALSE;
		shownumbers = TRUE;
		shownames = FALSE;
		end_orth();
		showcentres = save_showcentres;
		shownumbers = save_shownumbers;
		shownames = save_shownames;
		update_func = NULL;
		break;

	case WM_LBUTTONDBLCLK:
		wParam = IDOK;

	case WM_COMMAND:
		switch(wParam) {
		case IDOK:
			ind = GetDlgItemInt(hWndDlg,DLGEDIT+0,&i,TRUE);
#ifndef STUDENT
			inisec = GetDlgItemInt(hWndDlg,DLGEDIT+2,&i,TRUE);
			lassec = GetDlgItemInt(hWndDlg,DLGEDIT+3,&i,TRUE);
#endif
			GetDlgItemText(hWndDlg,DLGEDIT+1,text,sizeof(text));
			sscanf(text,"%f",&tmin);

		case IDCANCEL:
			dlgresult = (wParam == IDOK);
			EndDialog(hWndDlg,dlgresult);
			ReleaseDC(hWnd,hDC);
			hDC = NULL;
			hWnd = hSave;
			scrdev = grdev;
			setup(scrdev);
			break;

		case IDHELP:	/* Help */
			context(context_id);
			break;

		}
		break;

	case WM_LBUTTONUP:
		x = fabs(((REAL)(LOWORD(lParam) - 322) + 0.5) / xgslope);
		y = ymax - ((REAL)(HIWORD(lParam) - 24) +0.5) / ygslope;
		dsqmin=1.0e+30;
		tmin = 0.5;
		jmin = 1;
		for(j = 1 ; j < extlin ; j++) {
			dsqmin+= 1.0e-30;
			for(i = max(1,stsec[j]) ; i <= ensec[j] ; i++) {
				getparam(i,j,&a,&hb,&c,&hd);
				b = 2.0*hb;
				d = 2.0*hd;
				A = b*hb + d*hd;
				B = 1.5*(a*b + c*d);
				xr = x - yline[j][i];
				yr = zline[j][i] - y;
				C = a*a + c*c - b*xr - d*yr;
				D = - a*xr - c*yr;
				s = SolveCubic(A,B,C,D);
				if(s >= 0.0 && s <= 1.0) {
					xd = s*(a+hb*s) - xr;
					yd = s*(c+hd*s) - yr;
					dsq = xd*xd + yd*yd;
					if(dsq < dsqmin) {
						tmin = s;
						jmin = j;
						dsqmin = dsq;
					}
				}

				C = A*s*s + B*s +c;
				B = A*s + B;
				if(A != 0.0) {
					det = B*B - 4.0*A*C;
					if(det >= 0.0) {
						t2 = sqrt(det);
						t1 = (-B + t2)/(2.0*A);
						if(t1 >= 0.0 && t1 <= 1.0) {
							xd = t1*(a+hb*t1) - xr;
							yd = t1*(c+hd*t1) - yr;
							dsq = xd*xd + yd*yd;
							if(dsq < dsqmin) {
								tmin = t1;
								jmin = j;
								dsqmin = dsq;
							}
						}

						t1 = (-B - t2)/(2.0*A);
						xd = t1*(a+hb*t1) - xr;
						if(t1 >= 0.0 && t1 <= 1.0) {
							yd = t1*(c+hd*t1) - yr;
							dsq = xd*xd + yd*yd;
							if(dsq < dsqmin) {
								tmin = t1;
								jmin = j;
								dsqmin = dsq;
							}
						}
					}
				}
				else if(B != 0.0) {	/* straight line */
					t1 = -C / B;
					if(t1 >= 0.0 && t1 <= 1.0) {
						xd = t1*(a+hb*t1) - xr;
						yd = t1*(c+hd*t1) - yr;
						dsq = xd*xd + yd*yd;
						if(dsq < dsqmin) {
							tmin = t1;
							jmin = j;
							dsqmin = dsq;
						}
					}
				}
			}
		}
		ind = jmin+1;
		SetDlgItemInt(hWndDlg,DLGEDIT+0,ind,TRUE);
		sprintf(text,"%.4f",tmin);
		SetDlgItemText(hWndDlg,DLGEDIT+1,text);
		break;
	}
	return 0;
}
#endif

/*	solve A t^3 + B t^2 + C t + D = 0	*/

REAL SolveCubic(REAL A,REAL B,REAL C,REAL D)
{
	REAL t1 = 0.5;
	REAL t;
	REAL slope;
	int n = 0;

	do {
		t = t1;
		slope = t*(t*3.0*A + 2.0*B) + C;
		if(slope != 0.0)
			t1 = t - (t*(t*(t*A + B) + C) + D) / slope;
		else
		    t1 += 0.1;
	}
	while(fabs(t1 - t) > 0.0001 && n++ < 10);
	return t1;
}

MENUFUNC remove_line()
{
	int	i,j;
	int		code;
	int prev_scrdev = scrdev;

	scrdev = 1;

	if(count <= 0) return;

#ifdef linux
	dlgresult = RemLineDialog();
#else

	code = DialogBox(hInst,(char *) REMOVELI,hWnd,(DLGPROC) EditRemDlgProc);
	if(code < 0) (void) MessageBox(hWnd,"REMOVELI","Could not create dialog box",MB_OK);

#endif

	if(!dlgresult) {
		scrdev = prev_scrdev;
		setup(scrdev);
		return;
	}

	ind--;
	if(stemli == ind) {
		i = stemli;
		if(!getdlg(REMOSTLI,INP_INT,(void *) &i,-1) || i <= 0 || i > numlin) return;
		stemli = i-1;
#ifndef STUDENT
		fix_round_ends();
		save_hull(MAINHULL);
#endif
	}

	if(ind < numlin) numlin--;
	extlin--;
#ifndef STUDENT
	for(i = 0 ; i <= ntank ; i++) {
		if(fl_line1[i] > ind) fl_line1[i]--;
	}
#endif
	for(j = ind ; j < extlin ; j++) copyline(j,j+1,0.0);
	if(stemli >= ind) stemli--;
	changed = 1;

	/*	Free memory space not used		*/

	if(!realloc_hull(extlin)) return;
#ifndef STUDENT
	fl_line1[ntank] = extlin;
	redef_transom();
	recalc_tanks();
#endif
	scrdev = prev_scrdev;
	setup(scrdev);
}

#ifdef linux

#else

BOOL CALLBACK EditRemDlgProc(HWND hWndDlg,unsigned msg,WORD wParam,LONG lParam)
{
	void centre_dlg(HWND);
	void GetSizes(HWND);
	int i,j;
	char text[80];
	extern HWND hWndMain;
	extern HDC hDC;
	extern REAL xgslope,ygslope,xmin,ymin;
	static HWND hSave;
	extern int context_id,HelpUsed;
	extern int scrollable;
	REAL x,y,xr,yr,dsq,dsqmin;
	int jmin;
	BOOL colret;
	static int grdev;
	int		save_showcentres,save_shownumbers,save_shownames;

	if(ColourMessage(msg,wParam,lParam,&colret)) return colret;

	switch(msg) {
	case WM_INITDIALOG:
		hSave = hWnd;
		free_context();
		hWnd = GetDlgItem(hWndDlg,300);
		hDC = GetDC(hWnd);
		grdev = scrdev;
		scrdev = 1;
		setup(1);
		scrollable = FALSE;
		GetSizes(hWnd);
		centre_dlg(hWndDlg);
		sprintf(text,"Hull lines range from 1 to %2d.",extlin);
		SetDlgItemText(hWndDlg,DLGSTAT,text);
		SetDlgItemInt(hWndDlg,DLGEDIT+0,ind,TRUE);
		break;

	case WM_PAINT:
		save_showcentres = showcentres;
		save_shownumbers = shownumbers;
		save_shownames = shownames;
		showcentres = FALSE;
		shownumbers = TRUE;
		shownames = FALSE;
		end_orth();
		showcentres = save_showcentres;
		shownumbers = save_shownumbers;
		shownames = save_shownames;
		update_func = NULL;
		break;

	case WM_LBUTTONDBLCLK:
		wParam = IDOK;

	case WM_COMMAND:
		switch(wParam) {
		case IDOK:
			ind = GetDlgItemInt(hWndDlg,DLGEDIT+0,&i,TRUE);

		case IDCANCEL:
			dlgresult = (wParam == IDOK);
			EndDialog(hWndDlg,dlgresult);
			ReleaseDC(hWnd,hDC);
			hDC = NULL;
			hWnd = hSave;
			scrdev = grdev;
			setup(scrdev);
			break;

		case IDHELP:	/* Help */
			context(context_id);
			break;

		}
		break;

	case WM_LBUTTONUP:

		/*	The constant used for x is the sum of the dialog box position plus half its width in pixels	*/
		/*	The constant for y is its vertical position, in pixels	*/

		x = fabs(((REAL)(LOWORD(lParam) - 302) + 0.5) / xgslope);
		y = ymax - ((REAL)(HIWORD(lParam) - 28) +0.5) / ygslope;
		dsqmin=1.0e+30;
		for(j = 0 ; j < extlin ; j++) {
			for(i = max(1,stsec[j]) ; i <= ensec[j] ; i++) {
				xr = x - yline[j][i];
				yr = zline[j][i] - y;
				dsq = xr*xr + yr*yr;
				if(dsq < dsqmin) {
					jmin = j;
					dsqmin = dsq;
				}
			}
		}
		ind = jmin+1;
		SetDlgItemInt(hWndDlg,DLGEDIT+0,ind,TRUE);
		break;
	}
	return 0;
}
#endif

/*	Copy hull line data from one hull line number to another	*/

void copyline(INT destin,INT source,float del)
{
	INT	i;

	for(i = 0 ; i < count ; i++) {
		yline[destin][i] = yline[source][i] + del;
		zline[destin][i] = zline[source][i];
		ycont[destin][i] = ycont[source][i];
		zcont[destin][i] = zcont[source][i];
		linewt[destin][i] = linewt[source][i];
	}
	yline[destin][maxsec+1] = yline[source][maxsec+1];
	zline[destin][maxsec+1] = zline[source][maxsec+1];
	ycont[destin][maxsec+1] = ycont[source][maxsec+1];
	stsec[destin] = stsec[source];
	ensec[destin] = ensec[source];
	lineon[destin] = lineon[source];
#ifndef STUDENT
	radstem[destin] = radstem[source];
#endif
	if(source > 0 && destin > 0) relcont[destin] = relcont[source];
	changed = 1;
}

MENUFUNC edit_alte_new()
{
	extern INT stemli;
	int newst = stemli + 1;
	if(surfacemode) return;

	if(getdlg(STEMLINE,INP_INT,(void *) &newst,-1)) {
		stemli = --newst;
		changed = 1;
#ifndef STUDENT
		save_hull(MAINHULL);
		redef_transom();
		fix_round_ends();
		recalc_tanks();
#endif
	}
}

int any_radius_changed;

void change_radius(int code,HWND hWndDlg)
{
	int i;
	REAL rad;
	char text[40];
#ifdef linux
	int success;
	char *s;
	int *pos;
	extern Widget wListBox[];
	XmString xstr;
#endif

	if(code == 0) {
#ifdef linux
		strncpy(text,(s = XmTextGetString(wEdit[0])),sizeof(text));
		XtFree(s);
		if(sscanf(text,"%f",&rad) == 1) {
			XmListGetSelectedPos(wListBox[0],(int **) &pos,(int *) &success) - 1;
			i = pos[0];
			XtFree((char *) pos);
			if(i > 0 && i <= count && success > 0) {
				radstem[i-1] = rad;
				XmListDeleteItemsPos(wListBox[0],1,i);
				sprintf(text,"%d: %.3f",i,rad);
				xstr = XmStringCreateSimple(text);
				XmListAddItem(wListBox[0],xstr,i);
				XmStringFree(xstr);
				any_radius_changed = TRUE;
			}
		}
#else
		SendDlgItemMessage(hWndDlg,DLGEDIT,WM_GETTEXT,MAX_PATH,(LONG)(LPSTR) text);
		if(sscanf(text,"%f",&rad) == 1) {
			i = SendDlgItemMessage(hWndDlg,DLGLBX,LB_GETCURSEL,0,0L);
			if(i >= 0 && i < count) {
				radstem[i] = rad;
				SendDlgItemMessage(hWndDlg,DLGLBX,LB_DELETESTRING,i,0l);
				sprintf(text,"%d: %.3f",i+1,rad);
				SendDlgItemMessage(hWndDlg,DLGLBX,LB_INSERTSTRING,(WPARAM) i,(LPARAM) (LPCTSTR) text);
				any_radius_changed = TRUE;
			}
		}
#endif
	}

}

#ifdef PROF
MENUFUNC radius_stem()
{
	struct {
		int index;					/* index of result in table (not used) */
		char *string;				/* pointer to result (not used) */
		char *table[MAXTANKS+1];	/* table of strings for listbox, null string terminator */
	}
	xlist;
	int i;
	char text[MAX_PATH] = "";
	char listtext[maxlin][40];

	if(surfacemode) return;
	for(i = 0 ; i < extlin ; i++) {
		sprintf(listtext[i],"%d: %.3f",i+1,radstem[i]);
		xlist.table[i] = listtext[i];
	}
	xlist.table[extlin] = "";
	xlist.index = -1;
	xlist.string = text;

	any_radius_changed = FALSE;
	if(getdlg(STEMRADI,
	INP_LBX,(void *) &xlist,
	INP_INT,(void *) &text,
	INP_PBF,(void *) change_radius,
	-1) && any_radius_changed) {
		redef_transom();
		fix_round_ends();
		recalc_tanks();
#ifndef STUDENT
		check_invalid_end();
#endif
	}
}

void check_invalid_end()
{
	int i,j;
	char text[200];
	REAL x,r;
	for(j = 0 ; j < extlin ; j++) {
		i = stsec[j];
		x = xsect[i];
		if(i == 0) x -= yline[j][0];
		r = radstem[j];
		for(i = 0 ; i < count ; i++) {
			if(xsect[i] > x && xsect[i] < x + r) {
				sprintf(text,"Section %d at %.3f intersects the stem curve\nfor line %d, which has radius %.3f and starts at %.3f.\n\nThe stem curve may not be drawn properly.",i,xsect[i],j+1,r,x-r);
				message(text);
				return;
			}
		}
	}
}

#endif

/*	Fair the stem to the keel shape, using spline curve along keel	*/
/*	and adjusting the vertical control factor of the stem line on	*/
/*	the stem section (0)						*/

MENUFUNC fair_stem()
{
	REAL	dummy;	/* dummy area to receive the vertical offset	*/
	REAL	a,hb,c,hd;
	REAL	aa,cc;
	REAL	Z,	/* height of curve		*/
	Y,	/* width of curve		*/
	S,	/* slope of curve at top	*/
	R;	/* slope of curve at bottom	*/
	int	line;
	char *no_fairing = "Can not fair this stem";
	if(surfacemode) return;

	/*	Determine which finite part of the stem curve meets the stem line */

	for(line = stemli ; line > 0 ; line--) {
		if(stsec[line] == 0) {
			if(	yline[line][0] != yline[line-1][0] ||
				    zline[line][0] != zline[line-1][0]) goto found;
		}
	}

	/*	At this point, all lines are conicident at the stem -		*/
	/*	abort the operation						*/

	return;

	/*	This spline call returns values for only one position - the	*/
	/*	stem base (xsect[0])						*/

found:
	spline1(xsect,zline[stemli],linewt[stemli],count,
		&xsect[0],&dummy,1,
		zline[stemli][maxsec],zline[stemli][maxsec+1],
		&R);

	/*	Get existing section curve parameters				*/

	getparam(0,line,&a,&hb,&c,&hd);

	/*	The slope of the curve at its top is (c+d)/(a+b), and the	*/
	/*	position is (c+hd,a+hb): these must remain the same.  The	*/
	/*	slope of the curve at its base is c/a:  this must equal -zrate.	*/

	Y = a + hb;
	Z = c + hd;
	S = Y + hb;
	if(S == 0.0) {
		message(no_fairing);
		return;
	}
	S = (Z + hd) / S;

	/*	Thus we have four equations in four unknowns:

	a + 1/2 b              =  Y
	c + 1/2 d  =  Z
	-Sa - Sb  + c + d      =  0
	-Ra       + c          =  0					*/

	if(R == S) {
		message(no_fairing);
	}
	else {
		a = (Z - S * Y) / (0.5 * (R - S));
		if(fabs(a)*10000.0 <= fabs(Y)) {
			message(no_fairing);
			return;
		}
		else {
			hb = Y - a;
			c = R * a;
			hd = Z - c;

			getparam(0,line-1,&aa,&hb,&cc,&hd);
			if(cc != 0.0 && Z != 0.0) {
				zcont[line][0] = 0.5 * c / Z;
				ycont[line][0] = (aa / cc) * (Z - 0.5 * c) -
				    (Y - 0.5 * a);
			}
		}
	}
	changed = 1;
}

/*	find maximum x-offset of stem section		*/

REAL dxstem()
{
	REAL dx,x;
	INT	j;

	if(!surfacemode) {
		dx = xsect[0] - xsect[count-1];
		for(j = 0 ; j < numlin ; j++) {
			if(stsec[j] <= 0) {
				x = yline[j][0];
				if(x > dx) {
					dx = x;
				}
			}
		}
	}
	else {
		dx = 0.0;
	}
	return(dx);
}

/*	find minimum x-offset of stem section		*/
/*	     -------					*/

REAL ndxstem()
{
	REAL dx,x;
	INT	j;

	dx = 1.0e+30;
	for(j = 0 ; j < numlin ; j++) {
		if(stsec[j] <= 0) {
			x = yline[j][0];
			if(x < dx) dx = x;
		}
	}
	return(dx);
}

static int linereq = 1;

MENUFUNC lineprop()
{
	void linepropf(int code,HWND hWndDlg);

	if(count <= 0) return;
	if(getdlg(EDITPROP,
		INP_INT,(void *) &linereq,
		INP_PBF,linepropf,-1)) {
		changed = 1;
#ifndef STUDENT
		fix_round_ends();
#endif
	}
#ifndef STUDENT
	redef_transom();
	recalc_tanks();
#endif
}

void linepropf(int code,HWND hWndDlg)
{
	if(linereq > 0 && linereq <= extlin)
		line_properties(linereq-1);
	else
	    message("Invalid line index");
}

MENUFUNC convert_units()
{
	extern	INT	numun;
	REAL	conv;
	extern	INT	extlin;
	INT	i,j;
	int	newun = (numun + 2) & 3;

	/*	TABLE OF MASS CONVERSION FACTORS	*/

	REAL	confac[4] = {
		2.2046,0.9841964,0.453597,1.0160573	};

	char conf_conv[50];
	sprintf(conf_conv,"Perform conversion to %s and %s?",lenun[newun],masun[newun]);
	if(MessageBox(hWnd,conf_conv,"Conversion of Measurement Units",
		MB_YESNO) != IDYES) return;

	newun = (numun + 2) & 3;

	/*	UNITS CONVERSION:			*/

	/*	MASS CONVERSION				*/

	disp = disp*confac[numun];

	/*		LENGTH CONVERSION FACTOR	*/

	if(numun < 2) {
		conv = 3.28084;
	}
	else {
		conv = 0.3048;
	}

	/*	ALTER THE SECTION OFFSETS, AND THE LATERAL CONTROL DISTANCE	*/

	for(i = 0 ; i < count ; i++) {
		xsect[i] *= conv;
		for(j = 0 ; j < extlin ; j++) {
			yline[j][i] *= conv;
			zline[j][i] *= conv;
			ycont[j][i] *= conv;
			zcont[j][i] *= conv;
		}
	}
#ifndef STUDENT
	for(j = 0 ; j < numlin ; j++) radstem[j] *= conv;
#endif

	/*		ALTER THE UNIT NUMBER TO SUIT THE CONVERSION	*/

	numun = newun;

	/*		ALTER THE WATERLINE OFFSET	*/

	wl *= conv;

	/*		ALTER THE CENTRE OF MASS POSITION	*/

	xcofm *= conv;
	zcofm *= conv;

#ifndef STUDENT
	dztransom *= conv;
	rtransom *= conv;
	redef_transom();
	recalc_tanks();
#endif
	changed = 1;
	density = densit[numun];
}

int numopt = 0;

/*	Multiply or add to hull dimensions		*/

MENUFUNC mult_add()
{
	static REAL mulf = 1.0;
	static REAL divf = 1.0;
	static REAL addf = 0.0;
	REAL scf;
	static char usestr[MAX_PATH] = "ALL";
	int use[maxlin];
	extern REAL invert;
	REAL shift;
	INT j,i;

	if(getdlg(EDITMULT,
		INP_STR,(void *) usestr,
		INP_REA,(void *) &mulf,
		INP_REA,(void *) &divf,
		INP_REA,(void *) &addf,
		INP_RBN,(void *) &numopt,
		-1)) {

		if(numopt < 3) {
			if(divf == 0.0) {
				message("Zero divisor entered");
				return;
			}
			scf = mulf / divf;
		}
		else {
			scf = addf;
		}

		multproc(usestr,use,maxlin);

		changed = 1;

		switch(numopt) {
		case 0:				/* multiply vertical */
			for(j = 0 ; j < extlin ; j++) {
				if(use[j+1]) {
					for(i = 0 ; i < count ; i++) {
						zline[j][i] *= scf;
						zcont[j][i] *= scf;
					}
				}
			}
#ifndef STUDENT
			dztransom *= mulf;
#endif
			zcofm *= scf;
			break;

		case 1:				/* multiply lateral */
			for(j = 0 ; j < extlin ; j++) {
				if(use[j+1]) {
					for(i = max(1,stsec[j]) ; i <= ensec[j] ; i++) {
						yline[j][i] *= scf;
						ycont[j][i] *= scf;
					}
#ifndef STUDENT
					radstem[i] *= scf;
#endif
				}
			}
#ifndef STUDENT
			rtransom *= scf;
#endif
			break;

		case 2:				/* multiply longitudinal */
			for(i = 0 ; i < count ; i++) xsect[i] *= scf;
			for(i = 0 ; i < extlin ; i++) {
				yline[i][0] *= scf;
				ycont[i][0] *= scf;
			}
			xcofm *= scf;
			break;

		case 3:				/* shift forwards/backwards */
			for(i = 0 ; i < count ; i++) xsect[i] += scf;
			xcofm += scf;
			break;

		case 4:				/* shift up/down */
			shift = scf * invert;
			for(j = 0 ; j < extlin ; j++) {
				if(use[j+1]) {
					for(i = 0 ; i < count ; i++) {
						zline[j][i] += shift;
						zcont[j][i] += shift;
					}
				}
			}
			zcofm += shift;
			break;
		}
#ifndef STUDENT
		redef_transom();
		recalc_tanks();
#endif
	}
}

/*

Spline-fit a set of offsets (possibly with only one member) to an existing line.
This routine now includes the effects of stem radius, and performs all interpolations at the same time.

*/

void refit(int j,REAL xnew[],REAL ynew[],REAL znew[],REAL ycnew[],REAL zcnew[],INT is,INT ie)
{
	int		i,k,j1,j2,n;
	extern int *ignore;
	REAL	xold[maxsec];
	REAL	yold[maxsec];
	REAL	zold[maxsec];
	REAL	ycold[maxsec];
	REAL	zcold[maxsec];
	REAL	ystem = yline[stemli][1];
	REAL	t;
#ifndef STUDENT
	extern REAL	*radstem;
	REAL	stemrad = radstem[j];
	REAL	dxtan,s;
#endif
	REAL	a,hb,c,hd,t1,t2;
	int		sts = stsec[j];
	int		ens = ensec[j];
	REAL	ycstem;
	REAL	wt[maxsec];
	REAL	xsave = xsect[0];
	REAL	xstart;

	xstart = xsave;
	if(stsec[j] == 0) xstart -= yline[j][0];

/*	Create the arrays to be interpolated from non-ignored sections,
	and adjusting offset at stem to match any stem round
*/
	n = 0;
	for(i = sts ; i <= ens ; i++) {
		if(!ignore[i] && xsect[i] >= xsect[0]) {
			yold[n]  = yline[j][i];
			zold[n]  = zline[j][i];
			ycold[n] = ycont[j][i];
			zcold[n] = zcont[j][i];
			xold[n]  = xsect[i];
			wt[n] = linewt[j][i];
			if(!surfacemode && i == 0) {
				if(relcont[j]) {
					getparam(0,j,&a,&hb,&c,&hd);
					ycstem = yline[j][0] + 0.5*a;
					ycold[n] = 0.0;
				}
				else {
					ycstem = ycont[j][0];
					ycold[n] = ystem;
				}
				xold[n] = xstart;
				yold[n] = ystem;
			}

#ifndef STUDENT
			if(i == sts && stemrad != 0.0) { /* joins at stem line */
				get_int_angle(j,&c,&s);
				dxtan = stemrad*(1.0-c);
				if(s != 0.0)
					t = dxtan / s;
				else
					t = stemrad;

				if(yline[j][sts+1] > ystem)
					yold[n] = ystem + t;
				else
					yold[n] = ystem - t;

				if(relcont[j])
					ycold[n] = 0.0;
				else
					ycold[n] = yold[n];
			}
			else {
				dxtan = 0.0;
				t = 0.0;
			}
#endif
			n++;
		}
	}

	if(n >= 2) {
		k = ie - is + 1;
		spline(xold,yold,wt,n,&xnew[is],&ynew[is],k,yline[j][maxsec],yline[j][maxsec+1]);
		spline(xold,zold,wt,n,&xnew[is],&znew[is],k,zline[j][maxsec],zline[j][maxsec+1]);
		xold[0] = xsave;
		if(!surfacemode && is == 0) xold[0] -= ycstem;
		spline(xold,ycold,wt,n,&xnew[is],&ycnew[is],k,ycont[j][maxsec],ycont[j][maxsec+1]);
		spline(xold,zcold,wt,n,&xnew[is],&zcnew[is],k,zcont[j][maxsec],zcont[j][maxsec+1]);
		if(xnew[is] <= xstart) {
#ifdef PROF
			if(stemrad != 0.0) {
				ynew[is] = ystem;
				zcnew[is] = znew[is];
			}
#endif
			if(is == 0) ycnew[is] = ycstem;
		}

#ifndef STUDENT
		if(!surfacemode && is == 0) xold[0] += (yline[j][0]-t);
#else
		if(is == 0) xold[0] += yline[j][0];
#endif
	}

/*
	if(j > 0) {
		for(i = 0 ; xnew[i] < xsave ; i++) {
			t = xsave - xnew[i];
			k = 1;
			while(k < numlin && (stsec[k] > 0 || t < yline[k][0])) k++;
			k--;
			getparam(0,k,&a,&hb,&c,&hd);
			znew[i] = zline[k][0];
			zcnew[i] = zcont[k][0];
		}
	}
*/

#ifdef PROF
	if(stsec[j] > 0) goto check_controls;
#endif

	/*	Now check the new sections. Any which are located forward of the
	stem base may require special treatment				*/

	for(i = is ; i <= ie ; i++) {

		/*	distance aft of the section from the start of the
		current line.
		*/
		t1 = xnew[i] - xstart;
		if(t1 < 0.0) {

			/*	Section is forward of line end. Default action is to place
			the point in the stem plane, but then we check whether the
			line needs to be used to complete a section above.
			*/
			ynew[i] = ystem;
			znew[i] = zline[j-1][i];	/* 15/9/00 */
			if(j > 0) {

				if(relcont[j])
					ycnew[i] = 0.0;
				else
				    ycnew[i] = yline[j-1][i];	/* work needed here */

				zcnew[i] = zline[j-1][i];	/* presumes line above already done */
			}

			/*	Presume the stem above has been completed if the previous
			line is located on the stem line.
			*/
			if(j > 0 && yline[j-1][i] == ystem) {

				/*	The section above this line is complete: check whether the
				section below this line requires commencement.
				*/
				if(j < stemli) {		/*(required to ensure there is a line below)*/

					/*	"t1" is distance of this section aft of the end of the next
					line down.
					*/
					t1 = xnew[i];
					if(!surfacemode) t1 -= (xsect[0] - yline[j+1][0]);
					if(t1 > 0.0) {

						/*	Section is aft of the end of the next line down. Find the
						distance down where the stem profile intersects this section
						*/
						getparam(0,j+1,&a,&hb,&c,&hd);
						if(hb != 0.0) {	/* normal curve */
							t = a*a - 4.0*hb*t1;	/* note t1 runs backward on section */
							if(t >= 0.0) {
								t = fsqr0(t);
								t1 = (-a + t)/(2.0*hb);
								t2 = (-a - t)/(2.0*hb);
							}
							if(t1 < 0.0 || t1 > 1.0) t1 = t2;
						}
						else if(a != 0.0) {	/* linear solution */
							t1 = -t1 / a;
						}
						else {		/* zero size - do not need */
							t1 = -1.0e+30;	/* give value which leaves znew unchanged */
						}
						if(t1 >= -0.0001 && t1 <= 1.0001) {
							znew[i] = zline[j+1][0] - c*t1 - hd *t1 *t1;
						}
					}
				}
			}
			else {

				/*	Previous line is not on the stem plane ...			*/

				/*	The line end should be located at the base of the part of the
				section above the current z-offset				*/

				/*	Search back to the last line ending forward of the section	*/

				for(k = j ; k > 0 ; k--) {

					/*	If a line above is already on the stem line, the section
					above has already been completed				*/

					if(yline[k-1][0] == ystem) break;

					t = xnew[i] - (xsect[0] - yline[k-1][0]);

					/*	"t" is positive when the line end lies within the stem		*/

					if(t > 0.0) {

						/*	Find the curve parameters for the stem profile below the
						found line							*/

						getparam(0,k,&a,&hb,&c,&hd);

						/*	Solve for the z-offset where the curve crosses the section	*/

						if(hb != 0.0) {
							t = a*a - 4.0*hb*t1;
							if(t >= 0.0) {
								t = fsqr0(t);
								t1 = (-a + t)/(2.0*hb);
								t2 = (-a - t)/(2.0*hb);
							}
							if(t1 < 0.0 || t1 > 1.0) t1 = t2;
						}
						else if(a != 0.0) {
							t1 = -t1 / a;
						}
						else {
							t1 = -1.0e+30;
						}
						if(t1 >= 0.0001 && t1 <= 0.9999) {
							znew[i] = zline[k][0] - c*t1 - hd*t1*t1;
							zcnew[i] = 0.5*(znew[i] + zline[k-1][0]);
							if(relcont[j])
								ycnew[i] = 0.0;
							else
								ycnew[i] = ynew[i];
							break;
						}
					}
					else {
						t1 = t;
					}
				}
			}
		}
		else {

#ifndef STUDENT

			if(t1 < dxtan) {

				/*	Section is within stem round. Pull offset in, to meet rounding	*/

				if(s != 0.0) {
					t2 = (dxtan + c * t1) / s;
				}
				else {
					t2 = 0.0;
				}
				t1 = stemrad - t1;
				t1 = fsqr0(fsub(fmul(stemrad,stemrad),fmul(t1,t1)));
				ynew[i] -= (t2 - t1);
			}

#endif

			/*	If the previous part of the section was vertical on the stem,
			set control point to give horizontal alignment at the top of
			this part of the curve. This handles bulbs, etc.
			*/
			for(j1 = j-1 ; j1 >= 0 ; j1--) {
				if(xsect[stsec[j1]] <= xsect[i]) break;
			}
			for(j2 = j1-1 ; j2 >= 0 ; j2--) {
				if(xsect[stsec[j2]] <= xsect[i]) break;
			}
			if(j2 > 0) {
				if(yline[j2][i] == ystem && yline[j1][i] == ystem && zline[j2][i] > zline[j1][i]) {
					zcont[j][i] = zline[j1][i];
					t = yline[j][i] - yline[j1][i];
					if(fabs(t) > 0.001) {
						t1 = zline[j][i] - zline[j1][i];
						ycont[j][i] = (t1*t1 + t*t)/(2.0*t);
					}
					else {
						ycont[j][i] = 0.0;
					}
				}
			}
		}
	}

	/*	When, of any pair of lines, one has a nonzero stem radius, all sections between
	must have a vertical control offset at the stem line equal to the vertical offset.
	*/
#ifdef PROF
check_controls:
	if(j == stemli) {
		for(k = 1 ; k < numlin ; k++) {
			if(radstem[k-1] > 0.0 || radstem[k] > 0.0) {
				i = 1;
				while(xnew[i] < xsect[stsec[k-1]]) i++;
				while(xnew[i] <= xsect[stsec[k]]) {
					zcnew[i] = znew[i];
					i++;
				}
			}
		}
	}

#endif

}

#ifndef STUDENT
void fix_round_ends()
{
	int i,j,k;
	int warned = 0;
	for(j = 0 ; j < numlin ; j++) {
		i = stsec[j];
		if(radstem[j] != 0.0 && i > 0) {
			if(!warned) message(
				"You have defined a stem radius for a line which\nstarts aft of the stem. The ends of lines from\nthis to the stem line have been made coincident\nwith the stem line at the start section.");
			warned = 1;
			k = j;
			while(k != stemli) {
				yline[k][i] = yline[stemli][i];
				zline[k][i] = zline[stemli][i];
				ycont[k][i] = 0.0;
				zcont[k][i] = 1.0;
				if(k < stemli)
					k++;
				else
					k--;
			}
			if(j < stemli) j = stemli;
		}
	}
}
#endif
