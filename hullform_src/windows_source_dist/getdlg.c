/* Hullform component - getdlg.c
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
 
/*	Universal dialog box driver

Copyright Blue Peter Marine Systems
*/

#include <windows.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include <stdio.h>
#include "getdlg.h"

void message(char *);
void context(int);
void context_str(char *);

#define	IDHELP	9

/*	DialogProc	*/
#define PROCESSED TRUE
#define UNPROCESSED FALSE

#define REAL float

extern HANDLE hInst;	/* Program instance	*/
extern HWND hWnd;	/* Parent window	*/
extern char *helpfile;	/* Program help file name */
extern int context_id;	/* help ID code for this dialog box */
extern char *context_string;
extern int HelpUsed;	/* set TRUE is help system has been used */
HWND   PrevFocus;	/* last item which had focus - used by pushbutton functions */
HWND   PrevDialogBox = NULL;
extern HBRUSH hBkgd,hWhite;

void update_all(HWND);
void cls();

int ixs;
int getdlg_proceed;
int	numarg;
int	itemcode[40];
void	*itemaddr[40];
int	itemtype[40];
int	dlgresult;
BOOL CALLBACK DlgProc(HWND hWndDlg,unsigned msg,WORD wParam,
	LONG lParam);
typedef	void	(*INTFUNC_PTR) (int,HWND);
typedef	void	(*INIFUNC_PTR) (HWND);
extern int dlgboxpos;
int num_inp;
int num_static;
int focusitem;
int numarg;
int lastPBF;

int checked_rbn[3];
int lastRB[3];
int checkedRB[3];
int lastRBN[3];
int rbcount = 0;
int canexit;

extern int ddemode;
extern char *par[];
void centre_dlg(HWND hWndDlg);
int ColourMessage(unsigned msg,WORD wParam,LONG lParam,BOOL *hBrush);

int getdlg(int dlg_name,...)
{
	va_list	ap;
	int		code;
	int		*intp;
	REAL	*reap;
	char	*strp;
	void	*p;
	int		i = 1;
	int		ixs;
	dlglistbox *lbx;
	int		ix;
	int		j;
	char	extname[40];

	numarg = 0;
	if(PrevDialogBox != NULL) return(-1);

	va_start(ap,dlg_name);
	if(!ddemode) {
		while((code = va_arg(ap,int)) >= 0) {
			itemaddr[numarg]   = va_arg(ap,void *);
			itemtype[numarg++] = code;
		}
		va_end(ap);
	}
	else {
		while((code = va_arg(ap,int)) >= 0) {
			i++;	/* next DDE argument */
			p = va_arg(ap,void *);
			itemaddr[numarg] = p;
			itemtype[numarg++] = code;
			switch(code) {
			case INP_INT:
			case INP_RBN:
			case INP_LOG:
				if(p != NULL) {
					intp = (int *) p;
					sscanf(par[i],"%d",intp);
					if(i == 2) ixs = *intp - 1;
				}
				break;
			case INP_INX:
			case INP_RBX:
				if(p != NULL) {
					intp = (int *) p;
					sscanf(par[i],"%d",&intp[ixs]);
				}
				break;
			case INP_REA:
				if(p != NULL) {
					reap = (REAL *) p;
					sscanf(par[i],"%f",reap);
				}
				break;
			case INP_REX:
				if(p != NULL) {
					reap = (REAL *) p;
					sscanf(par[i],"%f",&reap[ixs]);
				}
				break;
			case INP_STR:
				if(p != NULL) strcpy((char *) p,par[i]);
				break;
			case INP_STX:
				if(p != NULL) {
					strp = (char *) p;
					strcpy(strp+MAX_PATH*ixs,par[i]);
				}
				break;
			case INP_LBX:
			case INP_CBX:
				if(p != NULL) {
					lbx = (dlglistbox *) p;
					ix = LB_ERR;
					for(j = 0 ; *(lbx->table[j]) != 0 ; j++) {
						if(strcmp(par[0],lbx->table[j]) == 0) {
							ix = j;
							break;
						}
					}
					lbx->index = ix;
				}
				break;
			}
		}
		va_end(ap);
	}
	dlgresult = -1;	/* undefined */
	code = DialogBox(hInst,(char *) dlg_name,hWnd,(void *) DlgProc);
	if(code < 0) message("Could not create dialog box");
	PrevDialogBox = NULL;
	dlgboxpos = 0;
	return(dlgresult);
}

#pragma argsused
BOOL CALLBACK DlgProc(HWND hWndDlg,unsigned msg,WORD wParam,
	LONG lParam)
{
	int success;
	int i,j,state,base;
	int m1,m2,m3;
	int *intp;
	REAL *reap;
	char *strp;
	char *strpp;
	char text[MAX_PATH];
	static int num_log;
	static int firstint = -1;
	int savefirstint;
	static int focusrespond;
	static INTFUNC_PTR rbfunc;
	INIFUNC_PTR inifunc;
	INTFUNC_PTR localfunc;
	int	localcode[40];
	void *localaddr[40];
	int	localtype[40];
	int localnumarg;
	int locallastRBN[3],locallastPBF;
	dlglistbox *lbx;
	int lbxnum;
	static int usenext;
	int localchecked_rbn[3];
	BOOL colret;

	if(ColourMessage(msg,wParam,lParam,&colret)) return colret;

	switch(msg) {

	case WM_INITDIALOG:

		centre_dlg(hWndDlg);
		PrevDialogBox = hWndDlg;
		num_inp = 0;
		firstint = -1;
		num_static = 0;
		for(i = 0 ; i < 3 ; i++) lastRBN[i] = 0;
		focusitem = 0;
		focusrespond = 1;
		canexit = 1;
		rbcount = 0;
		num_log = 0;
		lbxnum = 0;
		usenext = 0;
		for(i = 0 ; i < numarg ; i++) {
			switch(itemtype[i]) {
				case(INP_INT):
				intp = (int *) itemaddr[i];
				if(intp != NULL) {
					sprintf(text,"%d",*intp);
					strp = text;
					if(firstint < 0) firstint = i;
					goto rest_of_inp;
				} else {
					num_inp++;
				}
				break;

			case INP_INX:
				intp = (int *) itemaddr[0]; /* address of index */
				if(intp != NULL) {
					ixs = *intp - 1; /* NOTE BIAS OF ONE */
					/* inx is now index of value in the provided array */
					intp = (int *) itemaddr[i]; /* address of array */
					sprintf(text,"%d",intp[ixs]);
					strp = text;
					goto rest_of_inp;
				} else {
					num_inp++;
				}
				break;

			case INP_REX:
				intp = (int *) itemaddr[0]; /* address of index */
				if(intp != NULL) {
					ixs = *intp - 1; /* NOTE BIAS OF ONE */
					reap = (REAL *) itemaddr[i]; /* address of array */
					sprintf(text,"%.4f",reap[ixs]);
					goto next_of_real;
				} else {
					num_inp++;
				}
				break;

			case(INP_REA):
				reap = (REAL *) itemaddr[i];
				if(reap != NULL) {
					sprintf(text,"%.4f",*reap);
next_of_real:
					strp = text + strlen(text);
					while(*--strp != '.') if(*strp != '0') break;
					if(*strp != '.') strp++;
					*strp = 0;
					strp = text;
					goto rest_of_inp;
				} else {
					num_inp++;
				}
				break;

			case(INP_STR):
				strp = (char *) itemaddr[i];
rest_of_inp:	if(strp != NULL) {
					itemcode[i] = DLGEDIT + (num_inp++);
					SetDlgItemText(hWndDlg,itemcode[i],strp);
					if(itemtype[i] && !focusitem) focusitem = itemcode[i];
				} else {
					num_inp++;
				}
				break;

			case INP_STX:
				intp = (int *) itemaddr[0];
				if(intp != NULL) ixs = *intp - 1; /* NOTE BIAS OF ONE */
				/* inx is now index of value in the provided array */
				strpp = (char *) itemaddr[i]; /* address of array */
				if(strp != NULL) strp = strpp+MAX_PATH*ixs;
				goto rest_of_inp;

			case(INP_RBN):	/* radiobutton number is argument */
			case(INP_RBX):	/* radiobutton indexed off item 1 */
				base = DLGRBN1 + rbcount*10;
				itemcode[i] = base;
				for(lastRBN[rbcount] = base ;
				lastRBN[rbcount] < base+10 ; lastRBN[rbcount]++) {
					    if(GetDlgItem(hWndDlg,lastRBN[rbcount]) == NULL) break;
				}
				lastRBN[rbcount]--;
				switch(itemtype[i]) {
				case INP_RBN:
					intp = (int *) itemaddr[i];
					if(intp != NULL) {
						CheckRadioButton(hWndDlg,base,lastRBN[rbcount],base + *intp);
						checked_rbn[rbcount] = *intp;
					}
					break;
				case INP_RBX:
					intp = (int *) itemaddr[0]; /* address of index */
					ixs = *intp - 1; /* NOTE BIAS OF ONE */
					/* inx is now index of value in the provided array */
					intp = (int *) itemaddr[i]; /* address of array */
					if(intp != NULL) {
						CheckRadioButton(hWndDlg,base,lastRBN[rbcount],base + intp[ixs]);
						checked_rbn[rbcount] = *intp;
					}
					break;
				}
				rbcount++;
				break;

				case(INP_PBF):
				rbfunc = (INTFUNC_PTR) itemaddr[i];
				for(lastPBF = DLGPBF ; lastPBF < DLGPBF+40 ; lastPBF++) {
					if(GetDlgItem(hWndDlg,lastPBF) == NULL) break;
				}
				break;

				case(INP_INI):
				inifunc = (INIFUNC_PTR) itemaddr[i];
				(*inifunc)(hWndDlg);
				break;

				case(INP_LOG):
				case(INP_CBR):
				itemcode[i] = DLGLOG + (num_log++);
				if(itemtype[i] == INP_LOG) {
					intp = (int *) itemaddr[i];
					if(intp != NULL) SendDlgItemMessage(hWndDlg,itemcode[i],BM_SETCHECK,*intp,0l);
				}
				else {
					reap = (REAL *) itemaddr[i];
					if(reap != NULL) SendDlgItemMessage(hWndDlg,itemcode[i],BM_SETCHECK,(int) *reap,0l);
				}
				break;

			case INP_LBX:
				m1 = LB_RESETCONTENT;
				m2 = LB_ADDSTRING;
				m3 = LB_SETCURSEL;
				goto procbox;
			case INP_CBX:
				m1 = CB_RESETCONTENT;
				m2 = CB_ADDSTRING;
				m3 = CB_SETCURSEL;
procbox:
				lbx = (dlglistbox *) itemaddr[i];
				SendDlgItemMessage(hWndDlg,DLGLBX+lbxnum,m1,0,0l);
				for(j = 0 ; *(lbx->table[j]) != 0 ; j++)
					SendDlgItemMessage(hWndDlg,DLGLBX+lbxnum,m2,
						0,(LPARAM) (LPCSTR) lbx->table[j]);
				if(lbx->index >= 0) {
					SendDlgItemMessage(hWndDlg,DLGLBX+lbxnum,m3,lbx->index,0L);
					if(m1 == CB_RESETCONTENT) SetDlgItemText(hWndDlg,DLGLBX+lbxnum,lbx->table[lbx->index]);
				}
				lbxnum++;
				break;

				case(0):
				SetDlgItemText(hWndDlg,DLGSTAT + (num_static++),(LPSTR) itemaddr[i]);
				break;

			}
		}

		/*	Set the focus onto the edit box item, and select all of its text. */

		if(focusitem) {
			SetFocus(GetDlgItem(hWndDlg,focusitem));
			SendDlgItemMessage(hWndDlg,focusitem,EM_SETSEL,(WPARAM) 0,
				(LPARAM) -1);
		}
		else {
			SetFocus(GetDlgItem(hWndDlg,IDOK));
		}

		if(ddemode) SetTimer(hWndDlg,1,2000,NULL);
		return TRUE;

	case WM_LBUTTONDBLCLK:
		goto ok;

	case WM_COMMAND:
		switch(wParam) {
		case IDOK:
ok:
			if(canexit) {
				update_all(hWndDlg);
				EndDialog(hWndDlg,TRUE);
				if(dlgresult == -1) dlgresult = 1;
				getdlg_proceed = 1;
			}
			break;

		case IDCANCEL:
cancel:
			if(canexit) {
				EndDialog(hWndDlg,0);
				if(dlgresult == -1) dlgresult = 0;
			}
			break;

		case IDHELP:	/* Help */
			if(context_id > 0)
				context(context_id);
			else
				context_str(context_string);
			HelpUsed = 1;
			break;

		case DLGINC:
		case DLGDEC:
			if(firstint >= 0) {
				j = GetDlgItemInt(hWndDlg,itemcode[firstint],&i,TRUE);
				if(i != 0) {	/* success */
					if(wParam == DLGINC)
						j++;
					else
						j--;
				}
				SetDlgItemInt(hWndDlg,itemcode[firstint],j,TRUE);
				SetFocus(GetDlgItem(hWndDlg,itemcode[firstint]));
				SendDlgItemMessage(hWndDlg,itemcode[firstint],
					EM_SETSEL,(WPARAM) 0,(LPARAM) -1);
			}
			break;

		default:
			if(wParam >= DLGRBN1 && wParam < DLGRBN3+10) {
				if(wParam >= DLGRBN3)
					i = 2;
				else if(wParam >= DLGRBN2)
					i = 1;
				else
				    i = 0;
				CheckRadioButton(hWndDlg,DLGRBN1+10*i,lastRBN[i],wParam);
				checked_rbn[i] = wParam - (DLGRBN1+10*i);

			}
			else if(wParam >= DLGPBF && wParam < lastPBF) {
				/* function called from pushbutton */
				canexit = 0;
				PrevFocus = GetFocus();
				update_all(hWndDlg);
				for(i = 0 ; i < 3 ; i++) {
					locallastRBN[i] = lastRBN[i];
					localchecked_rbn[i] = checked_rbn[i];
				}
				locallastPBF = lastPBF;
				for(i = 0 ; i < numarg ; i++) {
					localcode[i] = itemcode[i];
					localaddr[i] = itemaddr[i];
					localtype[i] = itemtype[i];
				}
				localnumarg = numarg;
				localfunc = rbfunc;
				EnableWindow(hWndDlg,FALSE);
				PrevDialogBox = NULL;
				savefirstint = firstint;
				(*rbfunc)(wParam - DLGPBF,hWndDlg);
				EnableWindow(hWndDlg,TRUE);
				SetFocus(hWndDlg);
				SetFocus(PrevFocus);
				for(i = 0 ; i < localnumarg ; i++) {
					itemcode[i] = localcode[i];
					itemaddr[i] = localaddr[i];
					itemtype[i] = localtype[i];
				}
				for(i = 0 ; i < 3 ; i++) {
					lastRBN[i] = locallastRBN[i];
					checked_rbn[i] = localchecked_rbn[i];
				}
				lastPBF = locallastPBF;
				rbfunc = localfunc;
				numarg = localnumarg;
				firstint = savefirstint;
				canexit = 1;
			}
			else if(wParam >= DLGLOG && wParam < DLGLOG + 40) {
				state = (int) SendDlgItemMessage(hWndDlg,wParam,BM_GETCHECK,0,0l);
				SendDlgItemMessage(hWndDlg,wParam,BM_SETCHECK,!state,0l);
			}
			else if(wParam >= DLGLBX && wParam < DLGLBX + lbxnum) {
				if(HIWORD(lParam) == LBN_DBLCLK) goto ok;
			}
			else {
				return UNPROCESSED;
			}
			break;
		}
		return PROCESSED;

		/*	When the index edit box loses focus, update any cross-indexed
		parameter.
		*/
	case WM_SETFOCUS:
		if(wParam == DLGEDIT && focusrespond) {
			focusrespond = 0;
			ixs = GetDlgItemInt(hWndDlg,DLGEDIT + 1,&success,0)-1;
			if(success) {
				for(i = 0 ; i < numarg ; i++) {
					if(itemtype[i] == INP_INX) {
						intp = (int *) itemaddr[i];
						if(intp != NULL && ixs >= 0) sprintf(text,"%d",intp[ixs]);
						SetDlgItemText(hWndDlg,itemcode[i],text);
						SendDlgItemMessage(hWndDlg,focusitem,EM_SETSEL,
							(WPARAM) 0,(LPARAM) -1);
					}
					else if(itemtype[i] == INP_REX) {
						reap = (REAL *) itemaddr[i];
						if(reap != NULL && ixs >= 0) sprintf(text,"%.4f",reap[ixs]);
						strp = text + strlen(text);
						while(*--strp != '.')
							if(*strp != '0') break;
						if(*strp != '.') strp++;
						*strp = 0;
						SetDlgItemText(hWndDlg,itemcode[i],text);
						SendDlgItemMessage(hWndDlg,focusitem,EM_SETSEL,
							(WPARAM) 0,(LPARAM) -1);
					}
					else if(itemtype[i] == INP_STX) {
						strpp = (char *) itemaddr[i];
						if(strpp != NULL && ixs >= 0) SetDlgItemText(hWndDlg,itemcode[i],strpp+MAX_PATH*ixs);
						SendDlgItemMessage(hWndDlg,focusitem,EM_SETSEL,
							(WPARAM) 0,(LPARAM) -1);
					}
					else if(itemtype[i] == INP_RBX) {
						base = itemcode[i];
						j = base % 10;
						intp = (int *) itemaddr[i]; /* address of array */
						if(intp != NULL) CheckRadioButton(hWndDlg,base,base+j,base + intp[ixs]);
						j = (base-DLGRBN1)/10;
						checked_rbn[j] = intp[ixs];
					}
					else {
						return UNPROCESSED;
					}
				}
			}
			else {
				return UNPROCESSED;
			}
			focusrespond = 1;
			return PROCESSED;
		}
		else {
			return UNPROCESSED;
		}

	case WM_CHAR:
		if(wParam == 13)
			goto ok;
		else if(wParam == 27)
			goto cancel;
		else
			return UNPROCESSED;

	case WM_RBUTTONDOWN:
		goto cancel;

	case WM_TIMER:
		KillTimer(hWndDlg,1);
		EndDialog(hWndDlg,TRUE);
		return PROCESSED;

	}
	return UNPROCESSED;
}

void update_all(HWND hWndDlg)
{
	int i,j,ix,err;
	int *intp;
	REAL *reap;
	char *strp;
	char *strpp;
	char text[MAX_PATH];
	dlglistbox *lbx;
	int lbxnum = 0;
	int listbox;
	int rbcount = 0;

	for(i = 0 ; i < numarg ; i++) {
		switch(itemtype[i]) {
		case INP_INT:
		case INP_REA:
		case INP_STR:
		case INP_INX:
		case INP_REX:
		case INP_STX:
			SendDlgItemMessage(hWndDlg,itemcode[i],
				WM_GETTEXT,MAX_PATH,(LONG)(LPSTR) text);
			switch(itemtype[i]) {
			case INP_INT:
				intp = (int *) itemaddr[i];
				if(intp != NULL) {
					sscanf(text,"%d",intp);
					if(i == 0) ixs = *intp - 1;
				}
				break;
			case INP_INX:
				intp = (int *) itemaddr[i];
				if(intp != NULL) sscanf(text,"%d",&intp[ixs]);
				break;
			case INP_REA:
				reap = (REAL *) itemaddr[i];
				if(reap != NULL) sscanf(text,"%f",reap);
				break;
			case INP_REX:
				reap = (REAL *) itemaddr[i];
				if(reap != NULL) sscanf(text,"%f",&reap[ixs]);
				break;
			case INP_STR:
				strp = (char *) itemaddr[i];
				if(strp != NULL) strcpy(strp,text);
				break;
			case INP_STX:
				strpp = (char *) itemaddr[i];
				if(strpp != NULL) strcpy(strpp+MAX_PATH*ixs,text);
				break;
			}
			break;
		case INP_RBN:
		case INP_RBX:
			if(itemtype[i] == INP_RBN) {
				intp = (int *) itemaddr[i];
				if(intp != NULL) *intp = checked_rbn[rbcount];
			}
			else {
				intp = (int *) itemaddr[i];
				if(intp != NULL) intp[ixs] = checked_rbn[rbcount];
			}
			rbcount++;
			break;
		case INP_LOG:
			intp = (int *) itemaddr[i];
			if(intp != NULL) *intp = (int) SendDlgItemMessage(hWndDlg,itemcode[i],BM_GETCHECK,0,0l);
			break;
		case INP_LBX:
		case INP_CBX:
			listbox = (itemtype[i] == INP_LBX);
			lbx = (dlglistbox *) itemaddr[i];
			if(lbx != NULL) {
				if(listbox) {
					ix = SendDlgItemMessage(hWndDlg,DLGLBX+lbxnum,LB_GETCURSEL,0,0L);
					err = LB_ERR;
				} else {
					ix = SendDlgItemMessage(hWndDlg,DLGLBX+lbxnum,CB_GETCURSEL,0,0L);
					err = CB_ERR;
				}
				if(ix == err) {
					if(listbox) {
						GetDlgItemText(hWndDlg,DLGLBX+lbxnum,text,MAX_PATH);
						for(j = 0 ; lbx->table[j] != NULL && *(lbx->table[j]) != 0 ; j++){
							if(strcmp(text,lbx->table[j]) == 0) break;
						}
						lbx->index = j;
						if(!listbox && lbx->string != NULL)
							strcpy(lbx->string,text);
					}
					else {
						lbx->index = -1;
						if(lbx->string != NULL) GetDlgItemText(hWndDlg,DLGLBX+lbxnum,lbx->string,128);
					}
				}
				else {
					lbx->index = ix;
					if(lbx->string != NULL) GetDlgItemText(hWndDlg,DLGLBX+lbxnum,lbx->string,128);
				}
			}
			lbxnum++;
			break;
		}	/* end switch itemtype */
	}	/* end argument loop */
}

void centre_dlg(HWND hWndDlg)
{
	RECT rect;
	int left,top,width,height;

	GetWindowRect(hWndDlg,&rect);
	width = rect.right - rect.left;
	height = rect.bottom - rect.top;
	GetWindowRect(hWnd,&rect);
	GetWindowRect(hWnd,&rect);
	switch(dlgboxpos) {
	case 1:	/* bottom right */
		left = rect.right - width - 40;
		top = rect.bottom - height - 40;
		break;
	case 2:	/* bottom left */
		left = rect.left + 40;
		top = rect.bottom - height - 40;
		break;
	default:	/* centre */
		left = (rect.left+rect.right - width)/2;
		top = (rect.bottom+rect.top - height)/2;
		break;
	}
	(void) MoveWindow(hWndDlg,left,top,width,height,0);
}

char *floattext(REAL value)
{
	static char text[80];
	char *s;
	sprintf(text,"%.4f",value);
	s = text + strlen(text) - 1;
	while(*s == '0') s--;
	if(*s == '.') s--;
	*++s = 0;
	return (char *) text;
}

