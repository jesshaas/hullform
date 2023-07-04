/* Hullform component - statedit.c
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
 
/*	Graphical edit of loads and immersion points	*/

#ifdef PROF

#include "hulldesi.h"
#include "hst-help.h"

int viewmode = 1;/*	0 means end elevation		*/
		/*	1       plan view		*/
		/*	2	side elevation		*/
int statedit_mode = 0;
extern int context_id;

void showhelp(char *);

#ifdef linux

#include <Xm/Xm.h>
#include <Xm/MainW.h>

extern Widget topLevel;
extern XmFontList fontlist,windowfontlist,messagefontlist;
typedef void (BUTTONFUNC) (Widget w, XtPointer client_data, XtPointer call_data);
void CreateMenu(Widget menuBar,char *MenuName,char *Text[],BUTTONFUNC *func[],int menulevel[],int n);
BUTTONFUNC StatEditProf,StatEditTop,StatEditEnd,StatEditLoad,StatEditImme,StatEditProp,
		StatEditDoub,StatEditHalf,StatEditQuit;
char *StatEditMenuItem[] = {"Profile","Topview","Endview","Load","Immersion-point",
	"Properties","Double","Half","Quit"};
BUTTONFUNC *StatEditFunc[] = {StatEditProf,StatEditTop,StatEditEnd,StatEditLoad,
	StatEditImme,StatEditProp,StatEditDoub,StatEditHalf,StatEditQuit};
int StatEditLevel[] = {0,0,0,0,0,0,0,0,0};
extern Pixel DialogBackground,EditBackground,ButtonBackground,TopShadow,BottomShadow;

#else

void GetSizes(HWND);
HMENU hStatEditMenu;
HWND hWndSave;
extern HDC hDC;
HDC hDCSave;
HBRUSH hBrushOrigStat;
HPEN hPenOrigStat;

#endif

int firstuse;
void plotload(REAL,REAL,REAL,REAL);
void plotimme(REAL,REAL);
REAL square(REAL x);
void showvalues(REAL y,REAL z);
void showincr(REAL del);
void fillrect(int,int,int,int,int);
int strindx(char *s,int c);

extern	REAL	xload[MAXCOND][MAXLOAD];
extern	REAL	yload[MAXCOND][MAXLOAD];
extern	REAL	zload[MAXCOND][MAXLOAD];
extern	REAL	mload[MAXCOND][MAXLOAD];
extern	REAL	wload[MAXLOAD];
extern	REAL	lload[MAXLOAD];
extern	REAL	hload[MAXLOAD];
extern	REAL	defxload[MAXLOAD];
extern	REAL	defyload[MAXLOAD];
extern	REAL	defzload[MAXLOAD];
extern	REAL	defmload[MAXLOAD];
extern	char	loaddesc[MAXLOAD][41];
extern	REAL	ximme[MAXIMME];
extern	REAL	yimme[MAXIMME];
extern	REAL	zimme[MAXIMME];
extern	char	immedesc[MAXIMME][41];
extern	REAL	tankperc[MAXCOND][MAXTANK];
extern	REAL	tankspgr[MAXCOND][MAXTANK];

extern	int	numimme;
extern	int	numload;
extern	int	numcond;
extern	int	condnum;
extern	int	shiftdown;
extern	REAL	startwidth;

int loadmove = -1;
int immemove = -1;
int statkb;
REAL statsign = 1.0;

void statedit(void)
{
    int i;
    extern int scrollable;
#ifdef linux
    Arg		argval[40];		/* Widget command arguments */
    int		argnum;			/* widget command count */
    Widget	StatEdit,StatEditMenu;
#else
    extern HWND hWndMain;
    void window(HWND hParent,char *title,int xorigin,int yorigin,int xsize,int ysize,
	char *winclass,DWORD style,HWND *hWnd);
#endif

    context_id = HST_EDIT_GRAPHIC;
    firstuse = TRUE;
    update_func = null;
    for(i = 0 ; i < numload ; i++) zload[condnum][i] *= -1.0;
    for(i = 0 ; i < numimme ; i++) zimme[i] *= -1.0;
    statkb = TRUE;
    loadmove = -1;
    immemove = -1;
    statedit_mode = TRUE;
    scrollable = FALSE;

#ifdef linux

    StatEdit = XtVaCreateManagedWidget(
	"Hullform",
	xmMainWindowWidgetClass,
	topLevel,
	XmNmarginHeight,4,
	XmNmarginWidth,4,
	XmNwidth,640,
	XmNheight,480,
	XmNtitle,"Edit Static Loads",
	XmNbackground,EditBackground,
	NULL);
    XtManageChild(StatEdit);

/*	Create the menu bar and submenus	*/

    StatEditMenu = XmVaCreateSimpleMenuBar(
	StatEdit,
	"menuBar",
	XmNborderWidth,0,
	XmNbackground,ButtonBackground,
	XmNborderColor,ButtonBackground,
	XmNtopShadowColor,TopShadow,
	XmNbottomShadowColor,BottomShadow,
	XmNfontList,windowfontlist,
	NULL);

    CreateMenu(StatEditMenu,"StatEditMenu",StatEditMenuItem,StatEditFunc,StatEditLevel,9);

#else
    hStatEditMenu = LoadMenu(hInst,"STATEDIT");
    hWndSave = hWnd;
    hDCSave = hDC;
    window(hWndMain,"Edit Statics Items",
	CW_USEDEFAULT,CW_USEDEFAULT,
	640,480,"HFCLASS",WS_OVERLAPPEDWINDOW,&hWnd);
    if(hWnd == NULL) {
	hWnd = hWndSave;
	message("Edit Window creation failed");
	return;
    }

    SetMenu(hWnd,hStatEditMenu);
    GetSizes(hWnd);
    (*init)();
    hDC = GetDC(hWnd);
#endif

}

REAL *xl,*zl,*ll,*hl,*xi,*zi;
REAL statdel = 0.1;
REAL xmouse,ymouse;

#ifdef linux

void StatEditProf(Widget w, XtPointer client_data, XtPointer call_data)
{
    elev_orth();
    xl = xload[condnum];
    zl = zload[condnum];
    ll = lload;
    hl = hload;
    xi = ximme;
    zi = zimme;
    statsign = posdir;
}

void StatEditTop(Widget w, XtPointer client_data, XtPointer call_data)
{
    plan_orth();
    xl = xload[condnum];
    zl = yload[condnum];
    ll = lload;
    hl = wload;
    xi = ximme;
    zi = yimme;
    statsign = posdir;
}

void StatEditEnd(Widget w, XtPointer client_data, XtPointer call_data)
{
    startwidth = 1.0e+30;
    end_orth();
    xl = yload[condnum];
    zl = zload[condnum];
    ll = wload;
    hl = hload;
    xi = yimme;
    zi = zimme;
    statsign = 1.0;
    setranges(xmax,xmin,ymin,ymax);	/* swap limits */
}

void StatEditLoad(Widget w, XtPointer client_data, XtPointer call_data)
{
    if(numload > 0) {
	if(loadmove < 0) loadmove = 0;
		immemove = -1;
		statkb = TRUE;
    }
}

void StatEditImme(Widget w, XtPointer client_data, XtPointer call_data)
{
    if(numimme > 0) {
	if(immemove < 0) immemove = 0;
	loadmove = -1;
	statkb = TRUE;
/*	goto setobject;	*/
    }
}

void StatEditProp(Widget w, XtPointer client_data, XtPointer call_data)
{
    REAL t;
    if(loadmove >= 0) {
	t = -zload[condnum][loadmove];
#ifndef linux
	InvalidateRect(hWndProc,NULL,0);
#endif
	if(getdlg(LOADPROP,
		0,loaddesc[loadmove],
		INP_REA,&xload[condnum][loadmove],
		INP_REA,&yload[condnum][loadmove],
		INP_REA,&t,
		INP_REA,&mload[condnum][loadmove],
		INP_REA,&lload[loadmove],
		INP_REA,&wload[loadmove],
		INP_REA,&hload[loadmove],
		-1))
	    zload[condnum][loadmove] = -t;
	loadmove = -1;

    } else if(immemove >= 0) {
	t = -zimme[immemove];
#ifndef linux
	InvalidateRect(hWndProc,NULL,0);
#endif
	if(getdlg(IMMEPROP,
		0,immedesc[immemove],
		INP_REA,&ximme[immemove],
		INP_REA,&yimme[immemove],
		INP_REA,&t,-1))
	    zimme[immemove] = -t;
	immemove = -1;
    }
}

void StatEditDoub(Widget w, XtPointer client_data, XtPointer call_data)
{
    statdel *= 2.0;
    showincr(statdel);
}

void StatEditHalf(Widget w, XtPointer client_data, XtPointer call_data)
{
    statdel *= 0.5;
    showincr(statdel);
}

void StatEditQuit(Widget w, XtPointer client_data, XtPointer call_data)
{
}

#else

LRESULT procstatedit(HWND hWndProc,UINT msg,WPARAM wParam,
LPARAM lParam)
{
    PAINTSTRUCT ps;
    extern int condnum;
    int i,j;
    int xm,ym;
    REAL dsq,dsqmin,t,xmo,ymo;
    extern REAL xgslope,ygslope;
    RECT rw,rc;
    void trans(REAL *x0,REAL *y0);

    switch(msg) {

      case WM_SHOWWINDOW:
	if(firstuse) {
	    firstuse = FALSE;
	    hBrushOrigStat = SelectObject(hDC,GetStockObject(BLACK_BRUSH));
	    hPenOrigStat = SelectObject(hDC,GetStockObject(BLACK_PEN));
	    (*init)();
	    BringWindowToTop(hWndProc);
	}
	break;

      case WM_SIZE:
		GetSizes(hWndProc);
		InvalidateRect(hWndProc,NULL,TRUE);
		break;

      case WM_PAINT:
	BeginPaint(hWndProc,&ps);
	switch (viewmode) {
	  case 0:	/* Profile */
	    elev_orth();
	    xl = xload[condnum];
	    zl = zload[condnum];
	    ll = lload;
	    hl = hload;
	    xi = ximme;
	    zi = zimme;
	    statsign = posdir;
	    break;
	  case 1:	/* Top view */
	    plan_orth();
	    xl = xload[condnum];
	    zl = yload[condnum];
	    ll = lload;
	    hl = wload;
	    xi = ximme;
	    zi = yimme;
	    statsign = posdir;
	    break;
	  case 2:	/* End view */
	    startwidth = 1.0e+30;
	    end_orth();
	    xl = yload[condnum];
	    zl = zload[condnum];
	    ll = wload;
	    hl = hload;
	    xi = yimme;
	    zi = zimme;
	    statsign = 1.0;
		setranges(xmax,xmin,ymin,ymax);	/* swap limits */
	    break;
	}
	for(i = 0 ; i < numload ; i++) plotload(xl[i],zl[i],ll[i],hl[i]);
	for(i = 0 ; i < numimme ; i++) plotimme(xi[i],zi[i]);
	EndPaint(hWndProc,&ps);
	BringWindowToTop(hWndProc);
	goto setobject;

      case WM_COMMAND:
charinput:
	switch(wParam) {
	  case 101:	/*	Profile		*/
	  case 102:	/*	Topview		*/
	  case 103:	/*	Endview		*/
	    viewmode = wParam - 101;
	    InvalidateRect(hWndProc,NULL,0);
	    PostMessage(hWndProc,WM_PAINT,0,0);
	    break;

	  case 104:	/*	Load		*/
	    if(numload > 0) {
		if(loadmove < 0) loadmove = 0;
		immemove = -1;
		statkb = TRUE;
		goto setobject;
	    }
	    break;

	  case 105:	/*	Immersion-point	*/
	    if(numimme > 0) {
		if(immemove < 0) immemove = 0;
		loadmove = -1;
		statkb = TRUE;
		goto setobject;
	    }
	    break;

	  case 106:	/*	Properties	*/
	    goto statprop;

	  case 107: 	/*	Double		*/
	    statdel *= 2.0;
	    showincr(statdel);
	    break;

	  case 108:	/*	Half		*/
	    statdel *= 0.5;
	    showincr(statdel);
	    break;

	  case 109:	/*	Quit		*/
	    goto exitedit;
	}
	break;

/*	Properties of object	*/

      case WM_RBUTTONDOWN:

/*	Select object		*/

      case WM_LBUTTONDOWN:
	statkb = FALSE;		/* mouse mode, not keyboard mode */

/*	Find co-ordinates in length units	*/

	xm = LOWORD(lParam);	/* relative to upper left corner of client area */
	ym = HIWORD(lParam);
	if(xgslope != 0.0)
	    xmouse = xmin + xm / xgslope;
	else
	    xmouse = xmin;
	if(ygslope != 0.0)
	    ymouse = ymax - ym / ygslope;
	else
            ymouse = ymin;

/*	Find nearest object	*/

	dsqmin = 1.0e+30;
	immemove = -1;
        loadmove = -1;
	for(i = 0 ; i < numload ; i++) {
	    if(	xl[i] - 0.5*ll[i] <= xmouse &&
		xl[i] + 0.5*ll[i] >= xmouse &&
		zl[i] - 0.5*hl[i] <= ymouse &&
		zl[i] + 0.5*hl[i] >= ymouse) {
		dsq = square(xl[i] - xmouse) + square(zl[i] - ymouse);
		if(dsq < dsqmin) {
		    loadmove = i;
		    dsqmin = dsq;
		}
	    }
	}
	t = square(0.05*xsect[count-1] - xsect[0]);
	if(dsqmin > t) dsqmin = t;
	for(i = 0 ; i < numimme ; i++) {
	    dsq = square(xi[i] - xmouse) + square(zi[i] - ymouse);
	    if(dsq < dsqmin) {
		loadmove = -1;
		immemove = i;
		dsqmin = dsq;
	    }
	}
	if(msg == WM_RBUTTONDOWN) {

/*	Properties, not movement	*/

statprop:
	    if(loadmove >= 0) {

		t = -zload[condnum][loadmove];
		InvalidateRect(hWndProc,NULL,0);
		if(getdlg(LOADPROP,
		0,loaddesc[loadmove],
		INP_REA,&xload[condnum][loadmove],
		INP_REA,&yload[condnum][loadmove],
		INP_REA,&t,
		INP_REA,&mload[condnum][loadmove],
		INP_REA,&lload[loadmove],
		INP_REA,&wload[loadmove],
		INP_REA,&hload[loadmove],
		-1)) zload[condnum][loadmove] = -t;
		loadmove = -1;

	    } else if(immemove >= 0) {

		t = -zimme[immemove];
		InvalidateRect(hWndProc,NULL,0);
		if(getdlg(IMMEPROP,
		0,immedesc[immemove],
		INP_REA,&ximme[immemove],
		INP_REA,&yimme[immemove],
		INP_REA,&t,-1)) zimme[immemove] = -t;
		zimme[immemove] *= -1.0;
		immemove = -1;

	    }
	} else {

/*	Left Button Down - Place cursor exactly on object		*/

setobject:
	    if(loadmove >= 0) {
		xmouse = xl[loadmove];
		ymouse = zl[loadmove];
	    } else if(immemove >= 0) {
		xmouse = xi[immemove];
		ymouse = zi[immemove];
	    } else {
		break;
	    }
	    xmo = xmouse;
	    ymo = ymouse;
	    trans(&xmo,&ymo);
	    xm = xmo + 0.5;
            ym = ymo + 0.5;
	    GetWindowRect(hWndProc,&rw);	/* returns screen coordinates */
	    GetClientRect(hWndProc,&rc);	/* returns client window sizes */
	    i = ((rw.right-rw.left) - (rc.right-rc.left))/2;
					/* border width */
	    j = rw.bottom - i - ym;
	    i = rw.left + i + xm;
	    SetCursorPos(i,j);
	}
	break;

      case WM_LBUTTONUP:
	statkb = TRUE;
	break;

      case WM_MOUSEMOVE:
	arrowcursor();
	if(!statkb) {
	    xm = LOWORD(lParam);
	    ym = HIWORD(lParam);
	    if(xgslope != 0.0)
		xmouse = xmin + xm / xgslope;
	    else
		xmouse = 0.0;
	    if(ygslope != 0.0)
		ymouse = ymax - ym / ygslope;
	    else
		ymouse = 0.0;
objectmove:
	    if(loadmove >= 0) {
		i = loadmove;
		if(xl[i] != xmouse || zl[i] != ymouse) {
		    plotload(xl[i],zl[i],ll[i],hl[i]);
		    xl[i] = xmouse;
		    zl[i] = ymouse;
		    plotload(xl[i],zl[i],ll[i],hl[i]);
		    showvalues(xl[i],-zl[i]);
		    goto setobject;
		}
	    } else if(immemove >= 0) {
		i = immemove;
		if(xi[i] != xmouse || zi[i] != ymouse) {
		    plotimme(xi[i],zi[i]);
		    xi[i] = xmouse;
		    zi[i] = ymouse;
		    plotimme(xi[i],zi[i]);
		    showvalues(xi[i],-zi[i]);
		    goto setobject;
		}
	    }
	}
	break;

      case WM_CLOSE:
      case WM_DESTROY:
exitedit:
	statedit_mode = FALSE;
	GDIen();
	hDC = hDCSave;
	DestroyMenu(hStatEditMenu);
	DestroyWindow(hWnd);
	hWnd = hWndSave;
	BringWindowToTop(hWnd);
	GetSizes(hWnd);
	for(i = 0 ; i < numload ; i++) zload[condnum][i] *= -1.0;
	for(i = 0 ; i < numimme ; i++) zimme[i] *= -1.0;
	break;

      case WM_CHAR:
	wParam = 101 + strindx("PTELIRDHQ",toupper(wParam));
	if(wParam >= 101) goto charinput;
	break;

      case WM_KEYDOWN:
	switch(wParam) {

	  case VK_SHIFT:
	    shiftdown = TRUE;
	    break;

	  case VK_F1:		/* help */
	    showhelp("hullstat.hlp");
	    break;

	  case VK_ESCAPE:
	    goto exitedit;

	  case VK_TAB:		/* next object toggle */
	    statkb = TRUE;
	    if(shiftdown) {
		if(loadmove >= 0) {
		    if(--loadmove < 0) loadmove = numload-1;
		} else if(immemove >= 0) {
		    if(--immemove < 0) immemove = numimme-1;
		}
	    } else {
		if(loadmove >= 0) {
		    if(++loadmove >= numload) loadmove = 0;
		} else if(immemove >= 0) {
		    if(++immemove >= numimme) immemove = 0;
		}
	    }
	    goto setobject;

	  case VK_DOWN:
	    statkb = TRUE;
	    ymouse += statdel;
	    goto objectmove;

	  case VK_UP:
	    statkb = TRUE;
	    ymouse -= statdel;
	    goto objectmove;

	  case VK_LEFT:
	    statkb = TRUE;
	    xmouse += statsign*statdel;
	    goto objectmove;

	  case VK_RIGHT:
	    statkb = TRUE;
	    xmouse -= statsign*statdel;
	    goto objectmove;
	}
	break;

      case WM_KEYUP:
	if(wParam == VK_SHIFT) {
	    shiftdown = FALSE;
	    break;
	}		/* else return default */

      default:
	return DefWindowProc(hWndProc,msg,wParam,lParam);
    }
    return((LRESULT) 0);
}

#endif

/*	Plot loads and immersion points. Note that the
	load and immersion point convention is z-positive,
	while the hull convention is z-negative.
*/
void plotload(REAL x,REAL z,REAL l,REAL h)
{
    extern int ymaxi;
    void drawrect(int xleft,int xright,int ybottom,int ytop);

    REAL xl = x - 0.5*l;
    REAL xr = x + 0.5*l;
    REAL zb = z - 0.5*h;
    REAL zt = z + 0.5*h;
    int il,ir,jb,jt;

    trans(&xl,&zb);
    trans(&xr,&zt);
    il = xl;
    ir = xr;
    jb = ymaxi - (int) zb;
    jt = ymaxi - (int) zt;
    fillrect(il,ir,jb,jt,4);
}

void plotimme(REAL x,REAL z)
{
    extern int ymaxi;
    int ix,iz;
    trans(&x,&z);
    ix = x;
    iz = z;
    fillrect(ix - 4, ix + 4,ymaxi - iz - 4, ymaxi - iz + 4,2);
}

REAL square(REAL x)
{
    return x*x;
}

void fillrect(int xleft,int xright,int ybottom,int ytop,int col)
{
    extern void xorcol(int col);

#ifdef linux


#else

    if(hDC == NULL) hDC = GetDC(hWnd);
    SetROP2(hDC,R2_XORPEN);
    xorcol(col);
    Rectangle(hDC,xleft,ytop,xright,ybottom);
    SetROP2(hDC,R2_COPYPEN);

#endif
}

int strindx(char *s,int c)
{
    char *p;
    p = strchr(s,c);
    if(p == NULL)
	return -1;
    else
	return (int) (p - s);
}

#endif

