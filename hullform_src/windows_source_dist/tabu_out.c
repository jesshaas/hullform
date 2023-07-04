/* Hullform component - tabu_out.c
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

/*	write tabular data to text device		*/

int use_draft = 0;
int hydro_table = 0;
extern int stabvar;
REAL vstart = 0.0, vend = 0.0, vincr = 1.0;
REAL st_heel = 0.0, en_heel = 0.0,in_heel = 1.0;
void vardef(int,HWND);
extern int tabusel[MAXSEL];
void centre_dlg(HWND hWndDlg);
char *fixto7(REAL value);
extern char *var[];
extern char *helpfile;	/* Program help file name */
extern int context_id;	/* help ID code for this dialog box */
extern int HelpUsed;	/* set TRUE is help system has been used */
int sep = 1;		/* 0 = spaces, 1 = tabs, 2 = comma	*/
void finish_stats(void);
int ColourMessage(unsigned msg,WORD wParam,LONG lParam,BOOL *hBrush);

BOOL CALLBACK TabDlgProc(HWND hWndDlg,unsigned msg,WORD wParam,
	LONG lParam);

MENUFUNC tabular_output()
{
#ifdef DEMO
	not_avail();
#else
	REAL	calcdisp;
	REAL	save_disp,save_heel,save_wl;
	REAL	value;
	FILE	*fp;
	int		i,j,k;
	REAL	zmax;
	extern REAL varsign[];
	char	leadch[2] = "\0\0";
	char	*p;
	extern	int tabunowarning;
	int		save_id = context_id;
	char	sepch;

#ifdef SHAREWARE
	nagbox();
#endif
	if(getdlg(STABTABU,
		INP_REA,(void *) &vstart,
		INP_REA,(void *) &vend,
		INP_REA,(void *) &vincr,
		INP_RBN,(void *) &use_draft,
		INP_RBN,(void *) &hydro_table,
		INP_RBN,(void *) &sep,
		INP_PBF,(void *) vardef,-1)) {

		if(!hydro_table) {
			if(!getdlg(STABRANG,
				INP_REA,(void *) &st_heel,
				INP_REA,(void *) &en_heel,
				INP_REA,(void *) &in_heel,-1)) return;
		}

		value = invert * zcofm;
		context_id = 4072;
		if(!tabunowarning && !getdlg(TABUWARN,
			INP_REA,(void *) &xcofm,
			INP_REA,(void *) &value,
			INP_REA,(void *) &disp,
			INP_LOG,(void *) &tabunowarning,-1)) return;
		zcofm = invert * value;
		context_id = save_id;

		if(sep == 1)
			sepch = '\t';
		else if(sep == 2)
			sepch = ',';
		else
		    sepch = ' ';
		leadch[0] = sepch;

		if(!open_text(&fp,filedirnam,"*.txt")) return;

		save_disp = disp;
		save_heel = heel;
		save_wl = wl;

		if(hydro_table) {
			leadch[0] = '\0';
			st_heel = 0.0;
			en_heel = 0.0;
			in_heel = 1.0;
			for(i = 0 ; (j = tabusel[i]) >= 0 ; i++) {
				fprintf(fp,"%s%s",leadch,var[j]);
				k = strlen(var[j]);
				if(sep == 0) while(k++ < 7) fprintf(fp," ");
				leadch[0] = sepch;
			}
		}
		else {	/* stability table */
			switch(stabvar) {
			case 0:
				fprintf(fp,"Righting moment, units of %s %s\n\n",masun[numun],lenun[numun]);
				break;
			case 1:
				fprintf(fp,"Righting lever (GZ), units of %s\n\n",lenun[numun]);
				break;
			case 2:
				fprintf(fp,"Righting arm (KN), units of %s\n\n",lenun[numun]);
				break;
			}
			fprintf(fp,"%s   HEEL (degrees) ...\n",use_draft ? "DRAFT" : "DISPL");
			if(sep == 0) fprintf(fp,"        ");
			for(heel = st_heel ; heel < en_heel+0.5*in_heel ; heel += in_heel) {
				fprintf(fp,sep ? "%c%.1f" : "%c%-7.1f",sepch,heel);
			}
		}

		/*	calculate maximum z value, so wl = zmax - draft */

		zmax = -1.0e+30;
		for(j = 0 ; j < numlin ; j++) {
			for(i = stsec[j] ; i <= ensec[j] ; i++) {
				value = zline[j][i] + beta * xsect[i] + hwl[i];
				if(zmax < value) zmax = value;
			}
		}

		if(vincr == 0.0) {
			message("Infinite loop requested");
			return;
		}
		else if(vincr < 0.0) {
			value = vstart;
			vstart = vend;
			vend = value;
			vincr = -vincr;
		}
		loa = xsect[count-1] - xsect[0] + dxstem();

		for(value = vstart ; value < vend+0.5*vincr ; value += vincr) {

			if(!hydro_table) {
				p = fixto7(value);
				fprintf(fp,"\n%s",p);
				if(!sep) {
					k = strlen(p);
					while(k++ < 7) fprintf(fp," ");
				}
			}

			for(heel = st_heel ; heel < en_heel+0.5*in_heel ;
			heel += in_heel) {
				    sina = sin(0.01745329*heel);
				cosa = cos(0.01745329*heel);

				clrtext();
				if(!use_draft) {
					disp = value;
					if(disp <= 0.0) disp = 0.0001;
					preaxy(lx,2,"DISPLACEMENT = %12.3f",disp);
					prea(",  HEEL = %6.2f",heel);
					balanc(&calcdisp,1);
					if(calcdisp < 0.0) return;
				}
				else {
					wl = zmax - value;
					preaxy(lx,2,"DRAFT = %12.3f",value);
					prea(",  HEEL = %6.2f",heel);
					huldis(&calcdisp);
					disp = volu*densit[numun];
				}

				if(disp > 0.0) {
					finish_stats();
				}
				else {
					gz = 0.0;
					zmeta = 0.0;
					zcofb = 0.0;
					mct = 0.0;
				}

				if(hydro_table) {
					xmid = xentry + 0.5*lwl;
					leadch[0] = '\n';
					for(i = 0 ; (j = tabusel[i]) >= 0 ; i++) {
						p = fixto7(varval[j]*varsign[j]);
						fprintf(fp,"%s%s",leadch,p);
						if(!sep) {
							k = strlen(p);
							while(k++ < 7) fprintf(fp," ");
						}
						leadch[0] = sepch;
					}
				}
				else {	/* stability table */
					p = fixto7(stabvar == 0 ? gz*disp :
						stabvar == 1 ? gz : kn);
					fprintf(fp,"%s%s",leadch,p);
					if(!sep) {
						k = strlen(p);
						while(k++ < 7) fprintf(fp," ");
					}
					leadch[0] = sepch;
				}
			}
		}
		fprintf(fp,"\n");
		fclose(fp);
		disp = save_disp;
		heel = save_heel;
		wl = save_wl;
		sina = sin(0.01745329*heel);
		cosa = cos(0.01745329*heel);
	}
#endif
}

#ifndef DEMO

void vardef(int opt,HWND hWndDlg)
{
	FARPROC lpDlgProc;
	int code;
	extern int context_id;
#ifdef linux
	char dummy[MAX_PATH] = "";
	int i,j,k,result;
	struct {
		int index;       /* index of result in table */
		char *string;    /* pointer to result */
		char *table[numvar+1]; /* table of strings for listbox, null string terminator */
	}
	availist,selist;
	int iavail,isel;
	void vardef_func(int,HWND);
	int used[numvar];
	int numsel;
	XmString *xstrtab,xstr;
	char *str;
	int *pos;
	extern Widget wListBox[];
#endif

	if(opt == 1) {	/* hydrostatic table variables */
		context_id = 4070;
#ifdef linux
		iavail = 0;
		isel = 0;
		for(i = 0 ; i < numvar ; i++) used[i] = 0;
		for(i = 0 ; i < MAXSEL ; i++) {
			j = tabusel[i];
			if(tabusel[i] >= 0) {
				selist.table[isel++] = var[j];
				used[j] = 1;
			}
		}
		for(i = 0 ; i < numvar ; i++) {
			if(!used[i]) availist.table[iavail++] = var[i];
		}
		selist.table[isel] = "";
		availist.table[iavail] = "";

		if(getdlg(TABUVARI,
			INP_LBX,(void *) &availist,
			INP_LBX,(void *) &selist,
			INP_PBF,(void *) vardef_func,-1)) {
			i = 0;
			XtVaGetValues(wListBox[1],
				XmNitemCount,&numsel,
				XmNitems,&xstrtab,NULL);
			for(j = 0 ; j < numsel ; j++) {
				xstr = XmStringCopy(*xstrtab++);
				if(XmStringGetLtoR(xstr,XmSTRING_DEFAULT_CHARSET,&str)) {
					for(k = 0 ; k < numvar ; k++) {
						if(strcmp(str,var[k]) == 0) {
							tabusel[i++] = k;
							break;
						}
					}
					XtFree(str);
				}
				XmStringFree(xstr);
			}
			while(i < MAXSEL) tabusel[i++] = -1;
		}


#else
		code = DialogBox(hInst,(char *) TABUVARI,hWnd,(DLGPROC) TabDlgProc);
		if(code < 0) (void) MessageBox(hWnd,"DIALOG BOX ERROR","Could not create dialog box",MB_OK);
#endif
	}
	else {		/*  stability parameter */
		context_id = 4071;
		(void) getdlg(STABVARI,INP_RBN,(void *) &stabvar,-1);
	}
}

#ifdef linux

void vardef_func(int code,HWND hWndDlg)
{
	XmString *xstrtab,xstr;
	char *str;
	int *pos,n,c;
	extern Widget wListBox[];

	switch(code) {
	case 0:		/* add to selected */
		XtVaGetValues(wListBox[0],
			XmNselectedItemCount,&n,
			XmNselectedItems,&xstrtab,NULL);
		while(n-- > 0) {
			xstr = XmStringCopy(*xstrtab++);
			XmListDeleteItem(wListBox[0],xstr);
			XmListAddItem(wListBox[1],xstr,0);
			XmStringFree(xstr);
		}
		break;

	case 1:		/* removed from selected */
		XtVaGetValues(wListBox[1],
			XmNselectedItemCount,&n,
			XmNselectedItems,&xstrtab,NULL);
		while(n-- > 0) {
			xstr = XmStringCopy(*xstrtab++);
			XmListDeleteItem(wListBox[1],xstr);
			XmListAddItem(wListBox[0],xstr,0);
			XmStringFree(xstr);
		}
		break;

	case 2:		/* move selection up */
		XtVaGetValues(wListBox[1],
			XmNselectedItemCount,&n,
			XmNselectedItems,&xstrtab,NULL);
		if(n > 1)
			message("Can only move a single selection");
		else if(n == 1) {
			XmListGetSelectedPos(wListBox[1],&pos,&n);
			if(*pos > 1) {
				xstr = XmStringCopy(*xstrtab);
				XmListDeleteItem(wListBox[1],xstr);
				XmListAddItem(wListBox[1],xstr,(*pos) - 1);
				XmListSelectPos(wListBox[1],(*pos) - 1,False);
				XmStringFree(xstr);
			}
			free(pos);
		}
		else
			message("No item selected");
		break;

	case 3:		/* move selection down */
		XtVaGetValues(wListBox[1],
			XmNselectedItemCount,&n,
			XmNitemCount,&c,
			XmNselectedItems,&xstrtab,NULL);
		if(n > 1)
			message("Can only move a single selection");
		else if(n == 1) {
			XmListGetSelectedPos(wListBox[1],&pos,&n);
			if(*pos < c) {
				xstr = XmStringCopy(*xstrtab);
				XmListDeleteItem(wListBox[1],xstr);
				XmListAddItem(wListBox[1],xstr,(*pos) + 1);
				XmListSelectPos(wListBox[1],(*pos) + 1,False);
				XmStringFree(xstr);
			}
			free(pos);
		}
		else
			message("No item selected");
		break;
	}
}

#else

BOOL CALLBACK TabDlgProc(HWND hWndDlg,unsigned msg,WORD wParam,
	LONG lParam)
{
	int i,j,k;
	int used[numvar];
	int numsel;
	char sel[10];
	BOOL colret;


	if(ColourMessage(msg,wParam,lParam,&colret)) return colret;

	switch(msg) {

	case WM_INITDIALOG:
		centre_dlg(hWndDlg);
		SendDlgItemMessage(hWndDlg,DLGLBX+0,LB_RESETCONTENT,0,0l);
		SendDlgItemMessage(hWndDlg,DLGLBX+1,LB_RESETCONTENT,0,0l);
		for(i = 0 ; i < numvar ; i++) used[i] = FALSE;
		for(j = 0 ; j < MAXSEL ; j++) {
			i = tabusel[j];
			if(i >= 0) {
				SendDlgItemMessage(hWndDlg,DLGLBX+1,LB_ADDSTRING,
					0,(LPARAM) (LPCSTR) var[i]);
				used[i] = TRUE;
			}
		}
		for(j = 0 ; j < numvar ; j++) {
			if(!used[j]) {
				SendDlgItemMessage(hWndDlg,DLGLBX+0,LB_ADDSTRING,
					0,(LPARAM) (LPCSTR) var[j]);
			}
		}
		break;

	case WM_COMMAND:
		switch(wParam) {
		case IDOK:
			i = 0;
			numsel = SendDlgItemMessage(hWndDlg,DLGLBX+1,
				LB_GETCOUNT,0,0);
			for(j = 0 ; j < numsel ; j++) {
				if(SendDlgItemMessage(hWndDlg,DLGLBX+1,LB_GETTEXT,j,
					(LPARAM) (LPCSTR) sel) > 0) {
					for(k = 0 ; k < numvar ; k++) {
						if(strcmp(sel,var[k]) == 0) {
							tabusel[i++] = k;
							break;
						}
					}
				}
			}
			while(i < MAXSEL) tabusel[i++] = -1;
			EndDialog(hWndDlg,TRUE);
			return 0;

		case IDCANCEL:
			EndDialog(hWndDlg,0);
			return 0;

		case IDHELP:	/* Help */
			context(context_id);
			HelpUsed = 1;
			break;

		case DLGPBF + 0:	/* > */
			i = SendDlgItemMessage(hWndDlg,DLGLBX+0,
				LB_GETCURSEL,0,0);
			if(i != LB_ERR && SendDlgItemMessage(hWndDlg,DLGLBX+0,
				LB_GETTEXT,i,(LPARAM) (LPCSTR) sel) > 0 && SendDlgItemMessage(hWndDlg,
				DLGLBX+1,LB_GETCOUNT,0,0) < MAXSEL) {
				SendDlgItemMessage(hWndDlg,DLGLBX+0,
					LB_DELETESTRING,i,0);
				SendDlgItemMessage(hWndDlg,DLGLBX+1,
					LB_ADDSTRING,0,(LPARAM) (LPCSTR) sel);
				j = SendDlgItemMessage(hWndDlg,DLGLBX+1,
					LB_GETCOUNT,0,0l);
				SendDlgItemMessage(hWndDlg,DLGLBX+1,
					LB_SETCURSEL,j-1,0l);
			}
			break;

		case DLGPBF +1:	/* < */
			i = SendDlgItemMessage(hWndDlg,DLGLBX+1,
				LB_GETCURSEL,0,0);
			if(i != LB_ERR && SendDlgItemMessage(hWndDlg,DLGLBX+1,
				LB_GETTEXT,i,(LPARAM) (LPCSTR) sel) > 0) {
				SendDlgItemMessage(hWndDlg,DLGLBX+1,
					LB_DELETESTRING,i,0);
				SendDlgItemMessage(hWndDlg,DLGLBX+0,
					LB_ADDSTRING,0,(LPARAM) (LPCSTR) sel);
			}
			break;

		case DLGPBF +2:	/* Up */
		case DLGPBF +3:	/* Down */
			i = SendDlgItemMessage(hWndDlg,DLGLBX+1,
				LB_GETCURSEL,0,0l);
			j = SendDlgItemMessage(hWndDlg,DLGLBX+1,LB_GETCOUNT,0,0l);
			if((wParam == DLGPBF+2 && i > 0 || wParam == DLGPBF+3 && i < j-1)
				    && SendDlgItemMessage(hWndDlg,DLGLBX+1,LB_GETTEXT,i,(LPARAM) (LPCSTR) sel) > 0) {
				SendDlgItemMessage(hWndDlg,DLGLBX+1,LB_DELETESTRING,i,0);
				if(wParam == DLGPBF+2)
					i--;
				else
					i++;
				SendDlgItemMessage(hWndDlg,DLGLBX+1,LB_INSERTSTRING,i,(LPARAM) (LPCSTR) sel);
				SendDlgItemMessage(hWndDlg,DLGLBX+1,LB_SETCURSEL,i,0l);
			}
			break;

		case 105:	/* Available */
		case 106:	/* Selected */
			SetFocus(GetDlgItem(hWndDlg,DLGLBX + 106 - wParam));
			break;
		}
		break;
	}
	return(FALSE);
}

#endif

#endif

char *fixto7(REAL value)
{
	static char result[12];
	char *p = result;
	char *q;
	if(value <= 9999999.0 && value >= -999999.0) {
		sprintf(result," %.3f",value);
		while(isspace(*p)) p++;
		q = strchr(p,0);
//		while(*--q == '0') ;
//		if(*q != '.') q++;
		if((int) (q - p) > 7) q = p+7;
		*q = 0;
	}
	else {
		strcpy(result,"*******");
	}
	return p;
}

#endif
