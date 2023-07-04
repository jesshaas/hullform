/* Hullform component - revs_hfw.c
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
 
/*	Windows/X11 version of revsec routine	*/

#include "hulldesi.h"

#ifdef linux

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <Xm/Text.h>

#define VK_SHIFT XK_Shift_L
#define VK_F1 XK_F1
#define VK_F2 XK_F2
#define VK_TAB XK_Tab
#define VK_UP XK_Up
#define VK_DOWN XK_Down
#define VK_LEFT XK_Left
#define VK_RIGHT XK_Right
#define VK_ESCAPE XK_Escape
#define VK_RETURN XK_Return

#endif

static int revsection;					/* edit section number */
static int first,last;
#ifdef linux
void puteb(Widget hWnd,REAL value);
void geteb(Widget hWnd,REAL *value);
void revsRead(Widget w);
Widget MakeDialog(Widget Wshell,char *DialogName,int xloc,int yloc,int width,int height);
Widget MakeDialogShell(char *title);
Widget ComboBoxWidget(Widget Wdialog,int x,int y, int w, int h,int multiple);
Widget ButtonWidget(Widget Wdialog,char *text,int x,int y, int w, int h);
Widget LabelWidget(Widget Wdialog,char *text,int centre,int x,int y, int w, int h);
Widget TextWidget(Widget Wdialog,int x,int y, int w, int h);
Widget ListWidget(Widget Wdialog,int x,int y, int w, int h,int multiple);
Widget CheckBoxWidget(Widget Wdialog,char *text,int x,int y, int w, int h);
Widget RadioButtonWidget(Widget Wdialog,char *text,int x,int y, int w, int h);
#else
void puteb(HWND hWnd,int id,REAL value);
void geteb(HWND hWnd,int id,REAL *value);
void revsRead(HWND hWndDlg);
BOOL CALLBACK RevProc(HWND hWndDlg,unsigned msg,WPARAM wParam,LPARAM lParam);
#endif

extern char *helpfile;
extern int context_id;
extern char (*sectname)[12];

#ifndef STUDENT
void check_invalid_end();
#endif

int ColourMessage(unsigned msg,WORD wParam,LONG lParam,BOOL *hBrush);

#ifdef linux
Widget linux_DIREEDIT(
	Widget wLine[10],Widget wOffset[4][10],Widget wRel[10],Widget *wSection,Widget *wPosition,
	Widget *wName,Widget *wBack,Widget *wForw,Widget *wUndo,
	Widget *wOk,Widget *wCancel,Widget *wHelp);

Widget wLine[10],wOffset[4][10],wRel[10],wSection,wPosition,wName,wBack,wForw,wUndo;
extern Widget wOk,wCancel,wHelp;
void revsBack(Widget w, XtPointer client_data, XtPointer call_data);
void revsForw(Widget w, XtPointer client_data, XtPointer call_data);
void revsUndo(Widget w, XtPointer client_data, XtPointer call_data);
void revsClose(Widget w, XtPointer client_data, XtPointer call_data);
void revsHelp(Widget w, XtPointer client_data, XtPointer call_data);
void revsInc(Widget w, XtPointer client_data, XtPointer call_data);
void revsDec(Widget w, XtPointer client_data, XtPointer call_data);
void revsFocus(Widget w, XtPointer client_data, XtPointer call_data);
extern XtAppContext app_context;
Widget MakeDialogShell(char *title);
Widget MakeDialog(Widget Wshell,char *title,int x, int y, int w,int h);
void revsWrite();

Window FocusWindow;

#endif

void check_invalid_end(void);
/*
int InitWin(HINSTANCE hInstance,LRESULT CALLBACK  *WinProc,int style,
	char *class,char *menu,char *icon,HBRUSH hbr);
*/
void	window(HWND hParent,char *title,int xorigin,int yorigin,int xsize,int ysize,
	char *winclass,DWORD style,HWND *hWndRet);
MSG msg;
int end_dialog;

extern int editlines;
int maxlines;

void revsec(int ns)
{
#ifdef linux
	extern int getdlg_proceed;
	Widget Wdialog;
	XEvent event;
	int	j;
	extern int force_proceed;
#else
	FARPROC	lpDlgProc;
#endif
	int		i;
	extern int	changed;

	cls(TRUE);

	if(ns < surfacemode) ns = surfacemode;

	if(ns >= count) ns = count-1;
	revsection = ns;
	first = 0;
	last = min(extlin,editlines);
	maxlines = last;
	for(i = 0 ; i < extlin ; i++) {
		yline[i][maxsec+2] = yline[i][revsection];
		zline[i][maxsec+2] = zline[i][revsection];
		ycont[i][maxsec+2] = ycont[i][revsection];
		zcont[i][maxsec+2] = zcont[i][revsection];
	}
#ifdef linux

	Wdialog = linux_DIREEDIT(wLine,wOffset,wRel,&wSection,&wPosition,&wName,
		&wBack,&wForw,&wUndo,&wOk,&wCancel,&wHelp);
	XtAddCallback(wBack,XmNactivateCallback,revsBack,(XtPointer) 0);
	XtAddCallback(wForw,XmNactivateCallback,revsForw,(XtPointer) 0);
	XtAddCallback(wUndo,XmNactivateCallback,revsUndo,(XtPointer) 0);
	XtAddCallback(wOk,XmNactivateCallback,revsClose,(XtPointer) 1);
	XtAddCallback(wCancel,XmNactivateCallback,revsClose,(XtPointer) -1);
	XtAddCallback(wHelp,XmNactivateCallback,revsHelp,(XtPointer) 0);
	/*	This code is required to work around an apparent "weakness" in window
	management of LessTif / Motif
	*/
	for(i = 0 ; i < 4 ; i++) {
		for(j = 0 ; j < maxlines ; j++) {
			XtAddCallback(wOffset[i][j],XmNfocusCallback,revsFocus,(XtPointer) 0);
		}
	}
	XtAddCallback(wSection,XmNfocusCallback,revsFocus,(XtPointer) 0);
	XtAddCallback(wPosition,XmNfocusCallback,revsFocus,(XtPointer) 0);
	XtAddCallback(wName,XmNfocusCallback,revsFocus,(XtPointer) 0);
	revsWrite();

	getdlg_proceed = 0;
	force_proceed = 0;
	while(getdlg_proceed == 0 && force_proceed == 0) {
		XtAppNextEvent(app_context,&event);
		if(event.type == KeyPress) {

			/*	This code is required to work around an apparent "weakness" in window
			management of LessTif / Motif
			*/
			event.xkey.window = FocusWindow;
		}
		XtDispatchEvent(&event);
	}

	if(getdlg_proceed == 1) revsRead(Wdialog);
	XtUnmanageChild(Wdialog);
	XtUnmanageChild(XtParent(Wdialog));
	if(getdlg_proceed == 1) {
		changed = 1;
	}

#else

	lpDlgProc = (FARPROC) MakeProcInstance((FARPROC) RevProc,hInst);
	if(DialogBox(hInst,(char *) DIREEDIT,hWnd,(DLGPROC) lpDlgProc) == -1) {
		message("Could not create edit dialog box");
	}
	else {
		PostMessage(hWnd,WM_PAINT,0,0L);
		changed = 1;
	}

#endif

#ifndef STUDENT
	recalc_transom();
	check_invalid_end();
#endif
}

#ifdef linux

void revsWrite()
{
	int i,j,n;
	char text[40];
	int enable;
	XmString xstr;

	sprintf(text,"%d",revsection);
	XmTextSetString(wSection,text);
	puteb(wPosition,xsect[revsection]);
#ifndef STUDENT
	if(!sectname[revsection][0]) sprintf(sectname[revsection],"%d",revsection);
	XmTextSetString(wName,sectname[revsection]);
#endif

	for(n = 0, i = first; n < maxlines && i < last ; n++, i++) {	/* for each line of the dialog box */
		sprintf(text,"%2d:",i+1);
		xstr = XmStringCreateSimple(text);
		XtVaSetValues(wLine[n],XmNlabelString,xstr,NULL);
		XmStringFree(xstr);

		/*	Offset points are defined along the length of the line	*/

		enable = revsection >= stsec[i] && revsection <= ensec[i];
		if(enable) {
			puteb(wOffset[0][n],yline[i][revsection]);
			puteb(wOffset[1][n],invert*zline[i][revsection]);
			puteb(wOffset[2][n],ycont[i][revsection]);
			xstr = XmStringCreateSimple(i == 0 ? "" : relcont[i] ? "Rel" : "Abs");
			XtVaSetValues(wRel[n],XmNlabelString,xstr,NULL);
			XmStringFree(xstr);
			puteb(wOffset[3][n],invert*zcont[i][revsection]);
		}
		else {
			for(j = 0 ; j < 4 ; j++) XmTextSetString(wOffset[j][n],"");
			xstr = XmStringCreateSimple("");
			XtVaSetValues(wRel[n],XmNlabelString,xstr,NULL);
			XmStringFree(xstr);
		}
		for(j = 0 ; j < 4 ; j++) XmTextSetEditable(wOffset[j][n],enable);
	}

	/*	Disable all remaining edit boxes				*/

	xstr = XmStringCreateSimple("");
	while(n < maxlines) {
		for(j = 0 ; j < 4 ; j++) {
			XmTextSetString(wOffset[j][n],"");
			XmTextSetEditable(wOffset[j][n],False);
		}
		XtVaSetValues(wRel[n],XmNlabelString,xstr,NULL);
		n++;
	}
	XmStringFree(xstr);
}

void revsBack(Widget w, XtPointer client_data, XtPointer call_data)
{
	revsRead(XtParent(w));
	first = max(0,first-maxlines);
	last = min(extlin,first+maxlines);
	revsWrite();
}

void revsForw(Widget w, XtPointer client_data, XtPointer call_data)
{
	revsRead(XtParent(w));
	last = min(extlin, last + maxlines);
	first = max(0,last - maxlines);
	revsWrite();
}

void revsUndo(Widget w, XtPointer client_data, XtPointer call_data)
{
	int i;

	for(i = 0 ; i < extlin ; i++) {
		yline[i][revsection] = yline[i][maxsec+2];
		zline[i][revsection] = zline[i][maxsec+2];
		ycont[i][revsection] = ycont[i][maxsec+2];
		zcont[i][revsection] = zcont[i][maxsec+2];
	}
	revsWrite();
}

void revsClose(Widget w, XtPointer client_data, XtPointer call_data)
{
	extern int force_proceed;

	force_proceed = (int) client_data;
}

void revsHelp(Widget w, XtPointer client_data, XtPointer call_data)
{
	context(context_id);
}

#else

BOOL CALLBACK RevProc(HWND hWndDlg,unsigned msg,WPARAM wParam,
	LPARAM lParam)
{
	char text[20];
	int i,j,id,baseval,y;
	int enable;
	int firstsel;
	static int init;
	BOOL colret;
	int vertint;
#define Child	WS_CHILD | WS_TABSTOP | WS_VISIBLE

	switch(msg) {
	case WM_INITDIALOG:

		vertint = max(22, 480 / (maxlines + 10));
		for(i = 0 ; i < maxlines ; i++) {
			baseval = 1000 + i * 10;
			y = 68 + i * vertint;
			CreateWindow("STATIC"," ",  SS_LEFT | WS_CHILD | WS_VISIBLE,  8,y   ,32,16,hWndDlg,(HMENU) baseval,    hInst,NULL);
			CreateWindowEx(WS_EX_CLIENTEDGE,"EDIT"," ",    ES_LEFT | Child ,                60,y   ,80,20,hWndDlg,(HMENU) (baseval + 1),hInst,NULL);
			CreateWindowEx(WS_EX_CLIENTEDGE,"EDIT"," ",    ES_LEFT | Child ,               160,y   ,80,20,hWndDlg,(HMENU) (baseval + 2),hInst,NULL);
			CreateWindowEx(WS_EX_CLIENTEDGE,"EDIT"," ",    ES_LEFT | Child ,               260,y   ,80,20,hWndDlg,(HMENU) (baseval + 3),hInst,NULL);
			CreateWindowEx(WS_EX_CLIENTEDGE,"EDIT"," ",    ES_LEFT | Child ,               384,y   ,80,20,hWndDlg,(HMENU) (baseval + 4),hInst,NULL);
			CreateWindow("STATIC","Rel",SS_LEFT | WS_CHILD | WS_VISIBLE,348,y   ,34,16,hWndDlg,(HMENU) (baseval + 5),hInst,NULL);
		}

		y += 32;
		MoveWindow(GetDlgItem(hWndDlg,204),       6,y-6,48,28,TRUE);	// Forw.
		MoveWindow(GetDlgItem(hWndDlg,IDOK),     90,y,80,28,TRUE);	// Ok
		MoveWindow(GetDlgItem(hWndDlg,IDCANCEL),180,y,80,28,TRUE);	// Cancel
		MoveWindow(GetDlgItem(hWndDlg,IDHELP),  270,y,80,28,TRUE);	// Help
		MoveWindow(GetDlgItem(hWndDlg,198),     374,y,80,28,TRUE);	// Undo

		SetWindowPos(hWndDlg,HWND_TOP,0,0,478,y+68,SWP_NOMOVE);
rewrite:
		firstsel = -1;
		init = 1;
		SetDlgItemInt(hWndDlg,105,revsection,TRUE);
		puteb(hWndDlg,202,xsect[revsection]);
		sprintf(text,"Section %d,",revsection);
		SendDlgItemMessage(hWndDlg,211,WM_SETTEXT,0,(LONG)(LPSTR) text);
		if(!sectname[revsection][0]) sprintf((char *) sectname[revsection],"%d",revsection);
		SendDlgItemMessage(hWndDlg,205,WM_SETTEXT,0,(LONG)(LPSTR) sectname[revsection]);

//	The edit box id's run 1001,1002,1003,1004 / 1011,1012,1013,1014 / 1021,1022,1023,1024 / ...

		id = 1000;
		for(i = first; i < last ; i++) {	/* for each line of the dialog box */
			sprintf(text,"%2d:",i+1);
			SendDlgItemMessage(hWndDlg,id,WM_SETTEXT,0,(LONG)(LPSTR) text);

			/*	Offset points are defined along the length of the line	*/

			enable = revsection >= stsec[i] && revsection <= ensec[i];
			EnableWindow(GetDlgItem(hWndDlg,id + 1),enable);
			EnableWindow(GetDlgItem(hWndDlg,id + 2),enable);
			if(enable) {
				if(firstsel < 0) firstsel = id+1;
				puteb(hWndDlg,id+1,yline[i][revsection]);
				puteb(hWndDlg,id+2,invert*zline[i][revsection]);
			}
			else {
				SendDlgItemMessage(hWndDlg,id+1,WM_SETTEXT,0,(LONG)(LPSTR) "");
				SendDlgItemMessage(hWndDlg,id+2,WM_SETTEXT,0,(LONG)(LPSTR) "");
			}

			EnableWindow(GetDlgItem(hWndDlg,id + 3),enable);
			EnableWindow(GetDlgItem(hWndDlg,id + 4),enable);
			if(enable) {
				puteb(hWndDlg,id + 3,ycont[i][revsection]);
				puteb(hWndDlg,id + 4,invert*zcont[i][revsection]);
				SendDlgItemMessage(hWndDlg,id + 5,WM_SETTEXT,0,
					i == 0 ? (LONG)(LPSTR) "" : relcont[i] ? (LONG)(LPSTR) "Rel" : (LONG)(LPSTR) "Abs" );
			}
			else {
				SendDlgItemMessage(hWndDlg,id + 3,WM_SETTEXT,0,(LONG)(LPSTR) "");
				SendDlgItemMessage(hWndDlg,id + 4,WM_SETTEXT,0,(LONG)(LPSTR) "");
				SendDlgItemMessage(hWndDlg,id + 5,WM_SETTEXT,0,(LONG)(LPSTR) "");
			}
			id += 10;
		}

		/*	Enable backward button if first index revsection not zero; enable
		forward button if last index revsection not highest of hull		*/

		EnableWindow(GetDlgItem(hWndDlg,203),first > 0);
		EnableWindow(GetDlgItem(hWndDlg,204),last < extlin);

		/*	Disable all remaining edit boxes				*/

		while(id < 1000 + 10*maxlines) {
			SendDlgItemMessage(hWndDlg,id,WM_SETTEXT,0,(LONG)(LPSTR) "");
			for(i = 1 ; i <= 4 ; i++) {
				SendDlgItemMessage(hWndDlg,id + i,WM_SETTEXT,0,(LONG)(LPSTR) "");
				EnableWindow(GetDlgItem(hWndDlg,id + i),FALSE);
			}
			SendDlgItemMessage(hWndDlg,id + 5,WM_SETTEXT,0,(LONG)(LPSTR) "");
			id += 10;
		}
		SetFocus(GetDlgItem(hWndDlg,firstsel));
		SendDlgItemMessage(hWndDlg,firstsel,EM_SETSEL,(WPARAM) 0,(LPARAM) -1);
		BringWindowToTop(hWndDlg);
		init = 0;
		break;

	case WM_COMMAND:
		switch(LOWORD(wParam)) {
		case 212:	/* position */
			SetFocus(GetDlgItem(hWndDlg,202));
			SendDlgItemMessage(hWndDlg,202,EM_SETSEL,(WPARAM) 0,
				(LPARAM) -1);
			break;
		case 203:	/* backward scroll */
			revsRead(hWndDlg);
			first = max(0,first-maxlines);
			last = min(extlin,first+maxlines);
			goto rewrite;
		case 204:	/* forward scroll */
			revsRead(hWndDlg);
			last = min(extlin, last + maxlines);
			first = max(0,last - maxlines);
			goto rewrite;
		case IDCANCEL:/* Cancel */
		case 197:	/* Undo */
			for(i = 0 ; i < extlin ; i++) {
				yline[i][revsection] = yline[i][maxsec+2];
				zline[i][revsection] = zline[i][maxsec+2];
				ycont[i][revsection] = ycont[i][maxsec+2];
				zcont[i][revsection] = zcont[i][maxsec+2];
			}
			if(LOWORD(wParam) == IDCANCEL) {
				EndDialog(hWndDlg,1);
				end_dialog = 1;
				break;
			}
			else {
				goto rewrite;
			}
		case IDHELP:	/* help */
			context(context_id);
			break;
		case 105:	/* change index */
			if(!init && HIWORD(wParam) == EN_CHANGE) {
				j = GetDlgItemInt(hWndDlg,105,&i,TRUE);
				if(i && j >= surfacemode && j < count) {
					revsRead(hWndDlg);
					revsection = j;
					for(i = 0 ; i < extlin ; i++) {	/* change saved offset table */
						yline[i][maxsec+2] = yline[i][revsection];
						zline[i][maxsec+2] = zline[i][revsection];
						ycont[i][maxsec+2] = ycont[i][revsection];
						zcont[i][maxsec+2] = zcont[i][revsection];
					}
					goto rewrite;
				}
			}
			break;

		case DLGDEC:
		case DLGINC:
			revsRead(hWndDlg);
			j = GetDlgItemInt(hWndDlg,105,&i,TRUE);
			if(i != 0) {	/* success */
				if(wParam == DLGINC) {
					if(j < count-1) j++;
				}
				else {
					if(j > surfacemode) j--;
				}
				SetDlgItemInt(hWndDlg,105,j,TRUE);
				revsection = j;
			}
			SetFocus(GetDlgItem(hWndDlg,105));
			SendDlgItemMessage(hWndDlg,105,EM_SETSEL,(WPARAM) 0,
				(LPARAM) -1);
			goto rewrite;

		case IDOK:
			wParam = VK_RETURN;
			goto finish;
		}
		break;

	case WM_KEYDOWN:
finish:
		switch(wParam) {

		case VK_F2:	/* quick save */
			save_file();
			break;

		case VK_RETURN:
		case VK_ESCAPE:
			revsRead(hWndDlg);
			EndDialog(hWndDlg,1);
			end_dialog = 1;
			break;
		}
		break;
	}
	return 0;
}

#endif

#ifdef linux
void puteb(Widget w,REAL value)
#else
void puteb(HWND hWnd,int id,REAL value)
#endif
{
	char text[21];
	if(fabs(value) < 10000.0)
		sprintf(text,"%.4f",value);
	else
	    strcpy(text,"ERROR");
#ifdef linux
	XmTextSetString(w,text);
#else
	SetDlgItemText(hWnd,id,text);
#endif

}

#ifdef linux
void geteb(Widget w,REAL *value)
#else
void geteb(HWND hWnd,int id,REAL *value)
#endif
{
#ifdef linux
	char *text;
	text = XmTextGetString(w);
#else
	char text[21];
	GetDlgItemText(hWnd,id,text,20);
#endif
	if(strcmp(text,"ERROR") == 0)
		*value = 0.0;
	else
	    sscanf(text,"%f",value);
#ifdef linux
	XtFree(text);
#endif
}

#ifdef linux
void revsRead(Widget w)
#else
void revsRead(HWND hWndDlg)
#endif
{
	int id,i;
	REAL temp;
#ifndef STUDENT
#ifdef linux
	char *text;
#else
	char text[20];
#endif
#endif

	/*	Get and check the returned section position	*/
#ifdef linux
	geteb(wPosition,&temp);
#else
	geteb(hWndDlg,202,&temp);
#endif
	if(	(revsection >= count-1 || temp < xsect[revsection+1]) &&
		    (revsection <= 0 || temp > xsect[revsection-1])) xsect[revsection] = temp;

	/*	Get the data for each line from "first" to the one before "last"	*/
#ifdef linux
	id = 0;
#else
	id = 1000;
#endif
	for(i = first; i < last ; i++) {

#ifndef STUDENT
		if(revsection >= stsec[i] && revsection <= ensec[i]) {
#endif

#ifdef linux
			geteb(wOffset[0][id],&yline[i][revsection]);
			geteb(wOffset[1][id],&temp);
#else
			geteb(hWndDlg,id+1,&yline[i][revsection]);
			geteb(hWndDlg,id+2,&temp);
#endif
			zline[i][revsection] = invert*temp;
#ifndef STUDENT
		}
		if(revsection >= stsec[i]-1 && revsection <= ensec[i]+1) {
#endif

#ifdef linux
			geteb(wOffset[2][id],&ycont[i][revsection]);
			geteb(wOffset[3][id],&temp);
#else
			geteb(hWndDlg,id+3,&ycont[i][revsection]);
			geteb(hWndDlg,id+4,&temp);
#endif
			zcont[i][revsection] = invert*temp;
#ifndef STUDENT
		}
#endif

#ifdef linux
		id++;
#else
		id += 10;
#endif
	}

#ifndef STUDENT

#ifdef linux
	text = XmTextGetString(wName);
	strncpy(sectname[revsection],text,11);
	sectname[revsection][11] = 0;
	XtFree(text);
#else
	GetDlgItemText(hWndDlg,205,text,12);
	text[11] = 0;
	strcpy(sectname[revsection],text);
#endif

#endif
}

#ifdef linux

Widget linux_DIREEDIT(
	Widget wLine[10],Widget wOffset[4][10],Widget wRel[10],Widget *wSection,Widget *wPosition,
	Widget *wName,Widget *wBack,Widget *wForw,Widget *wUndo,
	Widget *wOk,Widget *wCancel,Widget *wHelp)
{
	Widget Wshell,Wdialog,w;
	int i,j,y;

	Wshell = MakeDialogShell("Direct Section Edit");

	Wdialog = MakeDialog(Wshell,"Direct_Section_Edit",80,61,474,400);

	w = LabelWidget(Wdialog,"Lateral",          True,  78,58,46,20);
	w = LabelWidget(Wdialog,"Vertical",         True, 176,58,50,20);
	w = LabelWidget(Wdialog,"Lateral - Rel/Abs",False,278,58,116,20);
	w = LabelWidget(Wdialog,"Vertical",         True, 398,58,54,20);
	w = LabelWidget(Wdialog,"OFFSETS",          False,116,42,66,20);
	w = LabelWidget(Wdialog,"CONTROL POINTS",   True, 270,42,162,20);

	w = LabelWidget(Wdialog,"Section",False,62,14,52,20);
	*wSection = TextWidget(Wdialog,154,10,32,24);

	w = LabelWidget(Wdialog,"Position",False,188,14,58,20);
	*wPosition = TextWidget(Wdialog,246,10,64,24);

	w = LabelWidget(Wdialog,"Name",False,322,14,38,20);
	*wName = TextWidget(Wdialog,368,10,64,24);

	*wBack = ButtonWidget(Wdialog,"Back",6, 36,54,24);
	*wForw = ButtonWidget(Wdialog,"Forw",6,362,54,24);

	*wOk     = ButtonWidget(Wdialog,"Ok",     80,362,70,28);
	*wCancel = ButtonWidget(Wdialog,"Cancel",180,362,70,28);
	*wUndo   = ButtonWidget(Wdialog,"Undo",  280,362,70,28);
	*wHelp   = ButtonWidget(Wdialog,"Help",  380,362,70,28);

	y = 78;
	for(j = 0 ; j < 10 ; j++) {
		wLine[j] = LabelWidget(Wdialog," ",False,8,y,50,20);
		wOffset[0][j] = TextWidget(Wdialog, 60,y,80,24);
		wOffset[1][j] = TextWidget(Wdialog,160,y,80,24);
		wOffset[2][j] = TextWidget(Wdialog,260,y,80,24);
		wRel[j] = LabelWidget(Wdialog,"Rel",False,348,y+4,34,20);
		wOffset[3][j] = TextWidget(Wdialog,386,y,80,24);
		y += 28;
	}

	w = ButtonWidget(Wdialog,"+",128, 4,26,26);
	XtAddCallback(w,XmNactivateCallback,revsInc,(XtPointer) *wSection);
	w = ButtonWidget(Wdialog,"-",128,22,26,26);
	XtAddCallback(w,XmNactivateCallback,revsDec,(XtPointer) *wSection);

	/*	The line below has to be here for the buttons to display properly - wierd! */

	XtVaSetValues(Wdialog,XmNdefaultButton,*wOk,NULL);
	return Wdialog;
}

void revsInc(Widget w,XtPointer client_data,XtPointer call_data)
{
	if(revsection < count-1) {
		revsRead(XtParent(w));
		revsection++;
		revsWrite();
	}
}

void revsDec(Widget w,XtPointer client_data,XtPointer call_data)
{
	if(revsection > 0) {
		revsRead(XtParent(w));
		revsection--;
		revsWrite();
	}
}

/*	This code is required to work around an apparent "weakness" in window
management of LessTif / Motif
*/
void revsFocus(Widget w, XtPointer client_data, XtPointer call_data)
{
	FocusWindow = XtWindow(w);
}

#endif

