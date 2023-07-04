/* Hullform component - toolbar.c
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

#include <gl\gl.h>
#include <gl\glu.h>

extern HDC hDC;
extern int *Bitmap;

typedef LRESULT CALLBACK WPROC();
BOOL CALLBACK ToolbarDlgProc(HWND hWndDlg,unsigned msg,WORD wParam,
	LONG lParam);
void window(HWND hParent,char *title,int xorigin,int yorigin,int xsize,int ysize,
	char *winclass,DWORD style,HWND *hWnd);
int InitWin(HANDLE hInstance,WPROC *WinProc,int style,
	char *winclass,char *menu,char *icon);
void DrawTool(HWND hwnd,int down);
extern char *helpfile;
int horzmin = -180,horzmax = 180,vertmin = -90,vertmax = 90;
extern int scrollable;
extern HINSTANCE hInstLib;
int ColourMessage(unsigned msg,WORD wParam,LONG lParam,BOOL *hBrush);
void CreateScrollBars(void);
void DestroyScrollBars(void);

extern int numtool;
extern char (*tbtext)[33];

extern UINT *tool_id;
extern HWND *ToolWnd;
int addtoolbar;
extern int toolwidth,toolheight;
int toolloc;
void DestroyToolbar(void);
void InitToolbar(void);
void InitWorkwin(int scroll);
int tbleft,tbright,tbtop,tbbottom;
extern int xleft,xright,ybottom,ytop,xmaxi,ymaxi;
extern int winl,wint,winw,winh;
extern int ToolDn;
extern HWND hWndMain;
extern HWND hWndToolbar;
extern HWND hWndWorkwin;
HWND hWndChild;
extern HWND hWndArrange;
extern int repaint;
extern int horzpos,vertpos;
extern REAL yview,zview,rotn;
HBITMAP xpmToBitmap(HDC hdc,int ind);
void GLen(void);


extern HGLRC hRC;
extern int viewlist;

void free_context(void);

void DrawTool(HWND hwnd,int down)
{
	HDC hdcMemory;
	HBITMAP hbmpMyBitmap, hbmpOld;
	extern HDC hDC;
	extern int toolloc;
	int i,loc;
	extern RECT clrect;
	HDC hdc;

	for(i = 0 ; i < numtool ; i++) {
		if(hwnd == (HWND) -1 && ToolWnd[i] != NULL || hwnd == ToolWnd[i]) {
			hdc = GetDC(ToolWnd[i]);
			hdcMemory = CreateCompatibleDC(hdc);

			hbmpMyBitmap = LoadBitmap(hInst,down ? "TOOL_DN_BMP" : "TOOL_UP_BMP");
			hbmpOld = SelectObject(hdcMemory, hbmpMyBitmap);
			BitBlt(hdc,0,0,32,32,hdcMemory, 0, 0, SRCCOPY);
			SelectObject(hdcMemory, hbmpOld);
			DeleteObject(hbmpMyBitmap);
			hbmpMyBitmap = xpmToBitmap(hdc,Bitmap[i]);
			SelectObject(hdcMemory, hbmpMyBitmap);
			loc = down ? 5 : 4;
			BitBlt(hdc,loc,loc,24,24,hdcMemory, 0, 0, SRCCOPY);
			SelectObject(hdcMemory, hbmpOld);
			DeleteObject(hbmpMyBitmap);
			DeleteDC(hdcMemory);
			ReleaseDC(ToolWnd[i],hdc);
			ToolDn = down ? i : -1;
			if(hwnd != (HWND) -1) break;
		}
	}
}

/*	Create toolbar window		*/

void InitToolbar()
{
	int toolx,tooly;
	int xtools,ytools;
	int j;
	RECT rca,rcb;
	extern HWND hWndMain;
	extern int arranging;
	extern int xleft,xright,ytop,ybottom;
	int ntool = numtool;
	extern HBRUSH tb;
	extern HGLRC hRC;

	/*	Get main (parent) window coordinates		*/

	if(!GetWindowRect(hWndMain,&rcb) || !GetClientRect(hWndMain,&rca)) {
		message("Could not get window size");
		rca.left = 0;
		rca.right = 640;
		rca.top = 0;
		rca.bottom = 480;
		rcb.left = 10;
		rcb.right = 630;
		rcb.top = 10;
		rcb.bottom = 460;
	}

	if(ntool > 0) {

		/*	Find dimensions of toolbar parent window, and workspace window	*/

		toolheight = 32;
		toolwidth = 32;

		xtools = (rca.right - rca.left) / toolwidth;
		ytools = (rca.bottom - rca.top) / toolheight;
		if(xtools == 0 || ytools == 0) return;	// minimised application

		switch (toolloc) {
		case 0:	/* top */
		default:
			tbleft = rca.left;
			tbtop = rca.top;
			tbright = rca.right;
			if(ntool > xtools) ntool = xtools;
			tbbottom = rca.top + toolheight * ((ntool-1) / xtools + 1);
			winl = rca.left;
			wint = tbbottom;
			winw = rca.right - rca.left;
			winh = rca.bottom - tbbottom;
			break;
		case 2:	/* left */
			tbleft = rca.left;
			if(ntool > ytools) ntool = ytools;
			tbtop = rca.top;
			tbright = rca.left + toolwidth;// * ((ntool-1) / ytools + 1);
			tbbottom = rca.bottom;
			winl = tbright;
			wint = rca.top;
			winw = rca.right - tbright;
			winh = rca.bottom - rca.top;
			break;
		case 1:	/* bottom */
			if(ntool > xtools) ntool = xtools;
			tbleft = rca.left;
			tbtop = rca.bottom - toolheight;// * ((ntool-1) / xtools + 1);
			tbright = rca.right;
			tbbottom = rca.bottom;
			winl = rca.left;
			wint = rca.top;
			winw = rca.right - rca.left;
			winh = tbtop - rca.top;
			break;
		case 3:	/* right */
			if(ntool > ytools) ntool = ytools;
			tbleft = rca.right - toolwidth;// * ((ntool-1) / ytools + 1);
			tbtop = rca.top;
			tbright = rca.right;
			tbbottom = rca.bottom;
			winl = rca.left;
			wint = rca.top;
			winw = tbleft - rca.left;
			winh = rca.bottom - rca.top;
			break;
		}

		/*	Create the Toolbar window				*/

		if(hWndToolbar) DestroyToolbar();
		hWndToolbar = CreateWindow("TOOLBAR","",WS_CHILD,tbleft,tbtop,
			tbright-tbleft,tbbottom-tbtop,hWndMain,NULL,hInst,NULL);
		if(!hWndToolbar) {
			message("Can not create toolbar");
			PostQuitMessage(0);
			return;
		}
		else {
			ShowWindow(hWndToolbar,SW_SHOWNORMAL);
			UpdateWindow(hWndToolbar);
		}
		SetCursor(LoadCursor(NULL,IDC_ARROW));

		/*	Locate and create all tool button windows		*/

		toolx = 0;
		tooly = 0;
		for(j = 0 ; j < ntool ; j++) {
			ToolWnd[j] = CreateWindow("TOOLBAR","",WS_CHILD | BS_OWNERDRAW | CS_DBLCLKS,toolx,tooly,toolwidth,toolheight,hWndToolbar,NULL,hInst,NULL);
//			window(hWndToolbar,"",toolx,tooly,toolwidth,toolheight,"TOOLBAR",WS_CHILD | BS_OWNERDRAW,&ToolWnd[j]);
			if(ToolWnd[j] == NULL) {
				message("Failed to create ToolBar item");
			}
			else {
				ShowWindow(ToolWnd[j],SW_SHOWNORMAL);
				UpdateWindow(ToolWnd[j]);
				if(toolloc < 2) {	/* top (0) or bottom (1) */
					toolx += toolwidth;
					if(toolx > tbright-tbleft-toolwidth) {
						toolx = 0;
						tooly += toolheight;
					}
				}
				else {	/* left (2) or right (3) */
					tooly += toolheight;
					if(tooly > tbbottom-tbtop-toolheight) {
						tooly = tbtop;
						toolx += toolwidth;
					}
				}
			}
		}
		for(j = ntool ; j < numtool ; j++) ToolWnd[j] = NULL;

		if(arranging) SetDlgItemText(hWndArrange,100,"Select the next item, or\npress the right mouse button");
		InvalidateRect(hWndToolbar,NULL,0);
		ToolDn = -1;
	}
	else {
		tbleft = rca.left;
		tbtop = rca.top;
		tbright = rca.right;
		tbbottom = rca.top;
		winl = rca.left;
		wint = tbbottom;
		winw = rca.right - rca.left;
		winh = rca.bottom - tbbottom;
	}
}

void DestroyToolbar()
{
	int j;
	for(j = 0 ; j < numtool ; j++) {
		if(ToolWnd[j] != NULL) {
			if(IsWindow(ToolWnd[j])) {
				DestroyWindow(ToolWnd[j]);
				ToolWnd[j] = NULL;
			}
			else {
				message("Invalid window handle");
			}
		}
	}
	DestroyWindow(hWndToolbar);
	hWndToolbar = NULL;
}

MENUFUNC set_toolloc()
{
	DestroyToolbar();
	(void) getdlg(TOOLLOC,INP_RBN,(void *) &toolloc,-1);
	InitToolbar();
	InitWorkwin(FALSE);
}

MENUFUNC set_toolbar()
{
	FARPROC	lpDlgProc;
	addtoolbar = 1;
	lpDlgProc = MakeProcInstance((FARPROC) ToolbarDlgProc,hInst);
	if(DialogBox(hInst,(char *) TOOLBAR,hWndMain,(DLGPROC) lpDlgProc) < 0)
		message("Could not create Tool Bar dialog box");

}

MENUFUNC delete_toolbar()
{
	FARPROC	lpDlgProc;
	addtoolbar = 0;
	lpDlgProc = MakeProcInstance((FARPROC) ToolbarDlgProc,hInst);
	if(DialogBox(hInst,(char *) TOOLDEL,hWndMain,(DLGPROC) lpDlgProc) < 0)
		message("Could not create Tool Bar dialog box");
}

int indsel = -1;

BOOL CALLBACK ToolbarDlgProc(HWND hWndDlg,unsigned msg,WORD wParam,LONG lParam)
{
	MEASUREITEMSTRUCT *m;
	DRAWITEMSTRUCT *d;
	static int nID;
	HDC hdcMemory;
	HBITMAP hbmpMyBitmap, hbmpOld;
	int width,height,left,top;
	extern int xmaxi,ymaxi;
	RECT rect;
	int i,n;
	static int curr,currsub;
	extern int context_id;
	HMENU hMenu;
	char text[MAX_PATH];
	extern int toolwidth,toolheight;
	extern HDC hDC;
	extern COLORREF scrcolour[];
	char *p;
	static int canexit;
	BOOL colret;

	if(ColourMessage(msg,wParam,lParam,&colret)) return colret;

	switch(msg) {
	case WM_MEASUREITEM:
		m = (MEASUREITEMSTRUCT *) lParam;
		m->CtlType = ODT_LISTBOX;
		m->itemID = nID++;
		m->itemWidth = 26;	/* a little space between bitmaps */
		m->itemHeight = 24;
		return TRUE;

	case WM_DRAWITEM:
		d = (DRAWITEMSTRUCT *) lParam;
		if(d->CtlID == DLGLBX+0 && (d->itemAction == ODA_DRAWENTIRE || d->itemAction == ODA_SELECT)) {
			hdcMemory = CreateCompatibleDC(d->hDC);
			if(addtoolbar) {
				hbmpMyBitmap = xpmToBitmap(d->hDC,d->itemID + 1);
			}
			else {
				hbmpMyBitmap = xpmToBitmap(d->hDC,Bitmap[d->itemID]);
			}
			hbmpOld = SelectObject(hdcMemory, hbmpMyBitmap);
			BitBlt(d->hDC,d->rcItem.left,d->rcItem.top,24,24,hdcMemory,0,0,
				d->itemAction == ODA_SELECT && d->itemState == ODS_SELECTED ? NOTSRCCOPY : SRCCOPY);
			SelectObject(hdcMemory, hbmpOld);
			DeleteObject(hbmpMyBitmap);
			DeleteDC(hdcMemory);
			break;
		}
		break;

	case WM_INITDIALOG:
		GetWindowRect(hWndDlg,&rect);
		width = rect.right - rect.left;
		height = rect.bottom - rect.top;
		GetWindowRect(hWnd,&rect);
		left = (rect.left+rect.right - width)/2;
		top = (rect.bottom+rect.top - height)/2;
		(void) MoveWindow(hWndDlg,left,top,width,height,0);
		nID = 0;
		indsel = -1;
		canexit = TRUE;

		/*	Build Internal Bitmap listbox	*/

		for(i = 0 ; i < (addtoolbar ? NUMTOOL : numtool) ; i++)
			SendDlgItemMessage(hWndDlg,DLGLBX+0,LB_ADDSTRING,0,(LPARAM) (LPCSTR) text);

		/*	Build Main Menu listbox		*/

		if(addtoolbar) {
			hMenu = GetMenu(hWndMain);
			n = GetMenuItemCount(hMenu);
			*text = 0;
			for(i = 0 ; i < n ; i++) {
				GetMenuString(hMenu,i,text,39,MF_BYPOSITION);
				if((p = strchr(text,'&')) != NULL) strcpy(p,p+1);
				SendDlgItemMessage(hWndDlg,DLGLBX+1,LB_ADDSTRING,0,(LPARAM) (LPCSTR) text);
			}
			SendDlgItemMessage(hWndDlg,DLGLBX+1,LB_SETCURSEL,0,0L);

			/*	Build Initial Menu Item list box	*/

			wParam = DLGLBX+1;

		}
		else {
			break;
		}

	case WM_COMMAND:
		switch(wParam) {

			/*	Select item menu	*/

		case DLGLBX+1:	/* selection from main menu list box	*/
			if(!canexit) break;
			SendDlgItemMessage(hWndDlg,DLGLBX+2,LB_RESETCONTENT,0,0);
			curr = (int) SendDlgItemMessage(hWndDlg,DLGLBX+1,LB_GETCURSEL,0,0);
			hMenu = GetSubMenu(GetMenu(hWndMain),curr);
			n = GetMenuItemCount(hMenu);
			for(i = 0 ; i < n ; i++) {
				if(GetMenuString(hMenu,i,text,39,MF_BYPOSITION) > 0) {
					if((p = strchr(text,'&')) != NULL) strcpy(p,p+1);
				}
				else {
					strcpy(text,"-------------------");
				}
				SendDlgItemMessage(hWndDlg,DLGLBX+2,LB_ADDSTRING,0,(LPARAM) (LPCSTR) text);
			}
			SendDlgItemMessage(hWndDlg,DLGLBX+2,LB_SETCURSEL,0,0L);
			PostMessage(GetDlgItem(hWndDlg,DLGLBX+2),WM_LBUTTONUP,1,1L);
			break;

		case DLGLBX+2:	/* selection in secondary menu list box	*/
			if(!canexit) break;
			SendDlgItemMessage(hWndDlg,DLGLBX+3,LB_RESETCONTENT,0,0);
			currsub = (int) SendDlgItemMessage(hWndDlg,DLGLBX+2,LB_GETCURSEL,0,0);
			hMenu = GetSubMenu(GetMenu(hWndMain),curr);	/* secondary menu handle */
			hMenu = GetSubMenu(hMenu,currsub);	/* tertiary (side) menu handle */
			if(hMenu != NULL) {
				n = GetMenuItemCount(hMenu);
				for(i = 0 ; i < n ; i++) {
					if(GetMenuString(hMenu,i,text,39,MF_BYPOSITION) > 0) {
						if((p = strchr(text,'&')) != NULL) strcpy(p,p+1);
						SendDlgItemMessage(hWndDlg,DLGLBX+3,LB_ADDSTRING,0,(LPARAM) (LPCSTR) text);
					}
				}
			}
			SendDlgItemMessage(hWndDlg,DLGLBX+3,LB_SETCURSEL,0,0L);
			PostMessage(GetDlgItem(hWndDlg,DLGLBX+3),WM_LBUTTONUP,1,1L);
			SendDlgItemMessage(hWndDlg,DLGLBX+1,LB_GETTEXT,curr,(LPARAM) (LPCSTR)text);
			p = strchr(text,0);
			strcpy(p,", ");
			p += 2;
			SendDlgItemMessage(hWndDlg,DLGLBX+2,LB_GETTEXT,currsub,(LPARAM) (LPCSTR) p);
			SetDlgItemText(hWndDlg,DLGEDIT +0,text);
			break;

		case DLGLBX+3:	/* selection in tertiary menu list box	*/
			if(!canexit) break;
			SendDlgItemMessage(hWndDlg,DLGLBX+1,LB_GETTEXT,curr,(LPARAM) (LPCSTR)text);
			p = strchr(text,0);
			strcpy(p,", ");
			p += 2;
			n = (int) SendDlgItemMessage(hWndDlg,DLGLBX+3,LB_GETCURSEL,0,0);
			SendDlgItemMessage(hWndDlg,DLGLBX+3,LB_GETTEXT,n,(LPARAM) (LPCSTR) p);
			p = strchr(text,0);
			if(p - text > 32) {
				p -= 4;
				while(p != text) {
					if(isspace(*--p)) break;
				}
				strcpy(p," ...");
			}
			SetDlgItemText(hWndDlg,DLGEDIT +0,text);
			break;

		case IDOK:
ok:
			if(!canexit) break;

			/*	Get index of selection in icon list box		*/

			i = (int) SendDlgItemMessage(hWndDlg,DLGLBX+0,LB_GETCURSEL,0,0);
			if(i < 0) i = indsel;
			if(i < 0) {
				message("No icon selected ..\n\nPlease try again");
				return 0;
			}
			else if(addtoolbar) {

				/*	Adding an item		*/

				GetDlgItemText(hWndDlg,DLGEDIT +0,tbtext[numtool],33);
				Bitmap[numtool] = i+1;

				/*	Get index of selection in menu item list box	*/

				/*	Firstly, check the submenu list box		*/
				n = (int) SendDlgItemMessage(hWndDlg,DLGLBX+3,LB_GETCURSEL,0,0);
				/*	Then check the menu list box			*/
				i = (int) SendDlgItemMessage(hWndDlg,DLGLBX+2,LB_GETCURSEL,0,0);
				if(i < 0 || curr < 0) {
					message("No menu item selected ..\n\nPlease try again");
				}
				else {
					hMenu = GetSubMenu(GetMenu(hWndMain),curr);
					if(n >= 0) {
						hMenu = GetSubMenu(hMenu,currsub);
						i = n;
					}
					tool_id[numtool] = GetMenuItemID(hMenu,i);
					numtool++;
				}
			}
			else {

				/*	Removing an item	*/

				while(++i < numtool) {
					tool_id[i-1] = tool_id[i];
					Bitmap[i-1] = Bitmap[i];
					strcpy(tbtext[i-1],tbtext[i]);
				}
				DestroyWindow(ToolWnd[--numtool]);
				ToolWnd[numtool] = NULL;
			}
			DestroyToolbar();
			InitToolbar();
			InitWorkwin(FALSE);
			EndDialog(hWndDlg,TRUE);
			return 0;

		case IDCANCEL:
cancel:
			if(!canexit) break;
			EndDialog(hWndDlg,0);
			return 0;

		case IDHELP:	/* Help */
			if(!canexit) break;
			context(context_id);
			break;

		default:
			if(wParam >= DLGTEXT && wParam <= DLGTEXT+2)
				SetFocus(GetDlgItem(hWndDlg,DLGLBX-DLGTEXT+wParam));
			break;
		}
		break;

	case WM_CHAR:
		if(wParam == 13)
			goto ok;
		else if(wParam == 27)
			goto cancel;
		break;

	case WM_RBUTTONDOWN:
		goto cancel;

	default:
		break;
	}
	return(FALSE);
}

BOOL CALLBACK ArrangeDlgProc(HWND hWndDlg,unsigned msg,WORD wParam,
	LONG lParam);

MENUFUNC arrange_toolbar()
{
	extern int arranging;
	FARPROC	lpDlgProc;
	arranging = 1;
	lpDlgProc = MakeProcInstance((FARPROC) ArrangeDlgProc,hInst);
	hWndArrange = CreateDialog(hInst,(char *) ARRANGE,hWnd,(DLGPROC) lpDlgProc);
	SetDlgItemText(hWndArrange,100,"Arranging toolbar\nPress item to move,\ndrag to new position");
}

BOOL CALLBACK ArrangeDlgProc(HWND hWndDlg,unsigned msg,WORD wParam,
	LONG lParam)
{
	switch(msg) {
	case WM_INITDIALOG:
		break;

	case WM_RBUTTONUP:
		DestroyWindow(hWndDlg);
		break;

	default:
		break;
	}
	return(FALSE);
}


/*	Create the working window	*/

void InitWorkwin(int scroll)
{
	HFONT hFont,OldFont;
	extern LOGFONT lf;
	extern HDC hDC;
	extern int xchar,ychar;
	TEXTMETRIC tm;
	char text[100];
	void GLin(void);
	extern int viewlist;

	/*	Destroy the old window	*/

	if(hWnd) {
		free_context();
		if(scrollable) DestroyScrollBars();
		if(hDC != NULL) {
			ReleaseDC(hDC,hWnd);
			hDC = NULL;
		}
		DestroyWindow(hWnd);
	}
	/*	Create the new window	*/

	hWnd = CreateWindow("HFCLASS","",
		WS_VISIBLE | WS_CHILD | WS_HSCROLL | WS_VSCROLL | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
		winl,wint,winw,winh,
		hWndMain,NULL,hInst,NULL);
	if(hWnd == NULL) {
		message("Failed to create working window");
		return;
	}

	if(scroll)
		CreateScrollBars();
	else
	    DestroyScrollBars();

	/*	Initialise graphics device context(s)	*/

	hDC = GetDC(hWnd);

	if(scrdev == 0) {	/* GL graphics	*/
		GLin();
	} else {
		setup(scrdev);
	}

	hFont = CreateFontIndirect(&lf);
	OldFont =  SelectObject(hDC,hFont);
	if(OldFont != NULL) DeleteObject(OldFont);
	GetTextMetrics(hDC,&tm);
	ychar = tm.tmHeight;
	xchar = tm.tmAveCharWidth;
}

MENUFUNC toolbar_hint(VOID)
{
	extern int HintDelay;
	REAL hd = ((REAL) HintDelay) * 0.001;
	if(getdlg(TOOLHINT,INP_REA,(void *) &hd,-1)) HintDelay = (int) (hd * 1000);
}

void CreateScrollBars(void)
{
	SetScrollRange(hWnd,SB_VERT,-90,90,FALSE);
	SetScrollRange(hWnd,SB_HORZ,-180,180,FALSE);
	vertpos = 57.293*atan2(yview,zview);
	horzpos = -rotn;
	SetScrollPos(hWnd,SB_VERT,vertpos,TRUE);
	SetScrollPos(hWnd,SB_HORZ,horzpos,TRUE);
}

void DestroyScrollBars(void)
{
	SetScrollRange(hWnd,SB_VERT,0,0,TRUE);
	SetScrollRange(hWnd,SB_HORZ,0,0,TRUE);
}

