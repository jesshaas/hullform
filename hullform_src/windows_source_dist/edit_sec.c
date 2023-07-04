/* Hullform component - edit_sec.c
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
#undef tc
#include <GL/gl.h>
#include <GL/glu.h>
#else
#include <GL\gl.h>
#include <GL\glu.h>
#endif

HDC hdcBitmap = NULL;
HBITMAP hbmpOld;
HBITMAP LoadBMP(char *file,int *w,int *h);
HBITMAP hDIB = NULL;
int bmWidth,bmHeight;
int scTop,scLeft;	// screen origin of drawn bitmap
int adjusting_bitmap;
int previous_xm,previous_ym;
void SwapBuf(void);

extern REAL GLxshift,GLyshift,GLxscale,GLyscale;

#include <sys/types.h>
#include <sys/stat.h>

#include "graf-hfw.h"

#ifdef linux

/*	X-windows include files		*/

#include <Xm/Xm.h>
#include <Xm/List.h>
#include <Xm/Form.h>
#include <Xm/Text.h>
#include <Xm/MainW.h>
#include <Xm/BulletinB.h>
#include <Xm/ScrolledW.h>
#include <Xm/CascadeB.h>
#include <Xm/RowColumn.h>
#include <Xm/PushB.h>
#include <Xm/Label.h>
#include <Xm/Frame.h>
#include <Xm/MessageB.h>
#include <Xm/ScrollBar.h>
#include <Xm/FileSB.h>
#include <Xm/DialogS.h>
#include <Xm/Separator.h>
#include <Xm/DrawingA.h>
#include <Xm/DrawnB.h>
#include <Xm/DragC.h>
#include <X11/keysym.h>
#include <X11/Xlib.h>

#endif

typedef long CALLBACK WPROC();
void check_graphics(void);
void GDIcircle(REAL xc,REAL yc,REAL rad);

#ifndef STUDENT
int showedittanks;
#endif
int showport,showstbd;

void xor_on(void);
void seteditranges(REAL x);
void makemenu(void);
void drawsec(int editsection);
void draw_edit_line(int il,int offset);
void drawpoint(int is,REAL y,REAL z);
void calc_stringers(int);
extern COLORREF scrcolour[];
void fix_vertical_controls(int i,int j,REAL value);
void line_properties(int editline);
void fairline(REAL *offs,REAL wt[],int st,int en,REAL xst,REAL yst);
void fairlines(int line);
void listseq(int on[],char *p,int n);
extern int *relcont;
extern int *autofair;
int	changeline;
extern int showoverlay;
#ifdef EXT_OR_PROF
void use_hull(int);
#endif

extern	int changed;
extern	int scrollable;
extern	int scaled;
extern	int tankcol;
extern	int numbetw;
int	oldnumbetw;
extern	HDC hDC;
extern	int xleft,ytop,xright,ybottom;
int	edit_scaled = 0;
int	transom_setting;
int	ymousemax,xmousemin;
REAL	del = 0.04;
int	first;
extern	int zoom;
LRESULT	proczoom(HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);
void	pstrbkg(char *s,int col,int bkg,int x,int y);
void	initzoom(void);
void	showincr(REAL del);
HMENU	hEditMenu = NULL;
void	xorcol(int col);
int	savetool;
REAL	zsc;
int	changex;

extern int numtool;
extern void InitToolbar(void);
extern void DestroyToolbar(void);
extern void InitWorkwin(int scroll);
void check_negative_offsets(void);
void EditSectionLeftButtonDown(int xm,int ym);
void EditSectionMouseMove(int xm,int ym);
void EditSectionKeyDown(int keysym);
void EditSectionMouseWheel(int clicks);

#define	MAXEDITUNDO	100

REAL	*yundo;
REAL	*zundo;
short	*undoline;
char	*undosect;
char	*editmode;	/* control (0) or offset (1) */
int	undoindex = 0;
void	saveundo(void);

int	key;
int	lastkey;
int	offset,follow,was_offset;
extern	int xmaxi,ymaxi;
clock_t	curtime,prevtime = 0;
REAL	xri;		/* size limit used repeatedly in section drawing */
REAL	yymin,yymax,zzmin,zzmax;	/* zoomed graphic limits */
REAL	yra,zra;
REAL	xmn1,xmx1,ymn1,ymx1;
extern	REAL xmin,xgslope,ymin,ygslope;
extern	REAL xgorigin,ygorigin;
extern	int	xmaxi,ymaxi;
extern	int	xchar,ychar;
extern	int	xwleft,ywtop;
extern	int	persp;
int	editline = 0;		/* line number */
int linevalue = 0;		/* value of line number being entered */
int	dragging = 0;
int	section_edit = 0;

extern char  	*alsosect;
extern char	*showline;
extern int	*editsectnum;
extern int	*alsosectnum;
extern int	*showlinenum;

int	editsection = 0;
int	shiftdown;
extern int HelpRequest;

void window(char *title,int xorigin,int yorigin,int xsize,int ysize,
	char *winclass,HWND *hWnd);
void GetSizes(HWND);
extern	int	vertpos;
extern	int	horzpos;
REAL	sinaz = 0.0,cosaz = 1.0,sinel = 0.0;
extern	int dragscroll;
REAL	*discard = NULL;
extern int context_id;
int	editinit = 1;

int webinclude = FALSE;

#define TEXTCOL 1
#define	SECTCOL	2
#define LINECOL 3
#define WATLCOL 4
#define	ALTLCOL 5
#define DEVECOL 6
#define EDISCOL 2
#define ALSOCOL 5
#define TANKCOL 7

char EditMenuChar[] = {
	'O','C','F','D','H','R','E',
	'S','U','Z','T','N','M','Q','B'};
char *EditMenuItem[] = {
	"Offset","Control","Follow","Double","Half","Redraw","Edit",
	"Show","Undo","Zoom","Textedit","ceNtre","Master","Quit","Bitmap",""};
void EditSectionRedraw(void);
void EditSectionRepaint(void);

#ifdef linux

void HorzScroll(Widget w, XtPointer client_data, XtPointer call_data);
void VertScroll(Widget w, XtPointer client_data, XtPointer call_data);
void EditHorzScroll(Widget w, XtPointer client_data, XtPointer call_data);
void EditVertScroll(Widget w, XtPointer client_data, XtPointer call_data);
void EditMenuCallback(Widget w, XtPointer client_data, XtPointer call_data);
extern Display *display;
extern GC gc;
extern Widget WorkArea;
extern Window win;
extern Pixel DialogBackground,EditBackground,ButtonBackground,TopShadow,BottomShadow;
extern XmFontList windowfontlist,fontlist;
int EditQuit;

Widget EditMenu = NULL;

#else

extern HMENU hMainMenu;
LRESULT procedit(HWND hWndProc,UINT msg,WPARAM wParam,LPARAM lParam);
extern void GLin(void);
extern HGLRC hRC;

#endif

extern int ScrollBarWidth,ScrollBarHeight;

#ifdef DXFOVL
extern int n_ovl;
extern REAL x_ovl[],y_ovl[],z_ovl[];
#endif

/*	INTERACTIVE HULL SECTION EDITOR.  HULL SECTION IS DISPLAYED ON GRAPHICS	*/
/*	DEVICE, AN EACH PARAMETER CAN BE MODIFIED USING THE CURSOR KEYS	*/

MENUFUNC edit_section(void)
{
	int	   i;
	extern int scaled;
#ifdef linux
	extern Widget Toolbar,hScroll,vScroll,Menubar,mainWindow,MenuHolder;
	Widget w;
	extern XtAppContext app_context;
	XEvent event;
	char chars[12];
	KeySym keysym;
	int xm,ym;
	char *entry;
	void CreateMenuAndToolbar();
	extern int mainw;
	extern Widget glw;
#else
	MSG msg;
	RECT rc,rw;
	int border;
	extern HWND hWndMain;
#endif

	if(count < 2) return;

	update_func = NULL;
	oldnumbetw = numbetw;
	numbetw = 0;
	if(editsection < surfacemode)
		editsection = surfacemode;
	else if(editsection >= count)
		editsection = count-1;

#ifdef EXT_OR_PROF
	use_hull(MAINHULL);
#endif
	context_id = 204;

	if(editinit) {
		if(!multproc(alsosect,alsosectnum,maxsec)) {
			for(i = 0 ; i < count ; i++) alsosectnum[i] = 1;
			for(i = count ; i < maxsec ; i++) alsosectnum[i] = 0;
			strcpy(alsosect,"ALL");
		}
		if(!multproc(showline,showlinenum,maxlin)) {
			for(i = 0 ; i < extlin ; i++) showlinenum[i] = 1;
			for(i = extlin ; i < maxlin ; i++) showlinenum[i] = 0;
			strcpy(showline,"ALL");
		}
		editinit = 0;
	}

#ifdef STUDENT
	endlin = extlin;
#else
	showedittanks = showport || showstbd;
	endlin = showedittanks ? extlin : numlin;
	transom_setting = transom;
	transom = 0;
#endif

	if(		!memavail((void *) &discard,maxsec*maxext*4) ||
		    !memavail((void *) &yundo,MAXEDITUNDO*sizeof(REAL)) ||
		    !memavail((void *) &zundo,MAXEDITUNDO*sizeof(REAL)) ||
		    !memavail((void *) &undoline,MAXEDITUNDO*sizeof(short)) ||
		    !memavail((void *) &undosect,MAXEDITUNDO*sizeof(char)) ||
		    !memavail((void *) &editmode,MAXEDITUNDO*sizeof(char))) {
		message("No free memory for this function");
		return;
	}

	scrollable = TRUE;
	changeline = -1;
	first = 1;		/* used in section drawing when screen is first displayed */
	key = -1;
	lastkey = -1;
	offset = 1;
	adjusting_bitmap = FALSE;
	follow = 0;
	zoom = 0;
	perset(0);
	dragging = 0;
	scaled = FALSE;
	edit_scaled = FALSE;
	dragscroll = 0;
	vertpos = 57.293*atan2(yview,zview);
	horzpos = -rotn;
	if(horzpos < -90)
		horzpos = -90;
	else if(horzpos > 90)
		horzpos = 90;
	if(vertpos < -90)
		vertpos = -90;
	else if(vertpos > 90)
		vertpos = 90;
	sinaz = sin((double) (-horzpos) * 0.01745329);
	cosaz = cos((double) (-horzpos) * 0.01745329);
	sinel = sin((double) (-vertpos) * 0.01745329);
	zoom = 0;

#ifdef linux

	XtUnmanageChild(MenuHolder);

	/*	Create the edit menu if necessary (on first use)	*/

	if(EditMenu == NULL) {
		EditMenu = XmVaCreateSimpleMenuBar(
			mainWindow,
			"editmenu",
			XmNborderWidth,0,
			XmNheight,32,
			XmNbackground,ButtonBackground,
			XmNborderColor,ButtonBackground,
			XmNtopShadowColor,TopShadow,
			XmNbottomShadowColor,BottomShadow,
			XmNfontList,windowfontlist,
			XmNwidth,mainw,
			XmNborderWidth,0,
			XmNmarginHeight,2,
			XmNmarginWidth,1,
			XmNallowResize,True,
			XmNattachment,XmATTACH_FORM,
			NULL);

		i = -1;
		while( *(entry = EditMenuItem[++i]) != 0) {
			w = XtVaCreateManagedWidget(entry,
				xmCascadeButtonWidgetClass,
				EditMenu,
				XmNwidth,50,
				XmNpushButtonEnabled,True,
				XmNbackground,ButtonBackground,
				XmNborderColor,ButtonBackground,
				XmNtopShadowColor,TopShadow,
				XmNbottomShadowColor,BottomShadow,
				XmNfontList,windowfontlist,
				NULL);
			XtAddCallback(w,XmNactivateCallback,EditMenuCallback,(XtPointer) i);
		}
	}
	XtManageChild(EditMenu);
    XmMainWindowSetAreas(mainWindow,EditMenu,NULL,hScroll,vScroll,WorkArea);

	cls(True);
	XtVaSetValues(hScroll,XmNvalue,horzpos,NULL);
	XtVaSetValues(vScroll,XmNvalue,vertpos,NULL);
	XtRemoveCallback(hScroll,XmNvalueChangedCallback,HorzScroll,NULL);
	XtRemoveCallback(vScroll,XmNvalueChangedCallback,VertScroll,NULL);
	XtAddCallback(hScroll,XmNvalueChangedCallback,EditHorzScroll,NULL);
	XtAddCallback(vScroll,XmNvalueChangedCallback,EditVertScroll,NULL);
	XSetFunction(display,gc,GXxor);

	EditQuit = False;
    (*init)();
	EditSectionRepaint();
	section_edit = 1;
	i = 0;

	while(!EditQuit) {
		XtAppNextEvent(app_context,&event);
		if(event.type == KeyPress) {
			XLookupString(&(event.xkey),chars,sizeof(chars),&keysym,NULL);
			shiftdown = event.xkey.state == 1;
			EditSectionKeyDown((int) keysym);
		}
		else if(event.xkey.window == win || (scrdev == 0 && event.xkey.window == XtWindow(glw))) {
			if(event.type == ButtonPress) {
				if(event.xbutton.button == Button1) {	/* left button - drag */
					EditSectionLeftButtonDown(event.xbutton.x,event.xbutton.y);
				}
				else {					/* right button - properties */
					xm = event.xbutton.x;
					ym = event.xbutton.y;
					if(offset == 1)	editline = nearest_offset(&i,&xm,&ym);
					else if(offset == 0) editline = nearest_control(&i,&xm,&ym);
					editline = abs(editline);
					line_properties(editline);
				}
			}
			else if(event.type == ButtonRelease) {
				if(event.xbutton.button == Button1) {		/* left button - drag */
					dragging = 0;
				}
			}
			else if(event.type == MotionNotify) {
				if(dragging)
					EditSectionMouseMove(event.xmotion.x,event.xmotion.y);
			}
			else {
				XtDispatchEvent(&event);
			}
		}
		else {
			XtDispatchEvent(&event);
		}
		XFlush(display);
	}
	XtRemoveCallback(hScroll,XmNvalueChangedCallback,EditHorzScroll,NULL);
	XtRemoveCallback(vScroll,XmNvalueChangedCallback,EditVertScroll,NULL);
	XtAddCallback(hScroll,XmNvalueChangedCallback,HorzScroll,NULL);
	XtAddCallback(vScroll,XmNvalueChangedCallback,VertScroll,NULL);
	XSetFunction(display,gc,GXcopy);

	XtUnmanageChild(EditMenu);
	XtManageChild(MenuHolder);
    XmMainWindowSetAreas(mainWindow,MenuHolder,NULL,hScroll,vScroll,WorkArea);
	cls(True);

#else

	savetool = numtool;
	DestroyToolbar();
	numtool = 0;
	InitToolbar();

	SetScrollRange(hWnd,SB_VERT,-90,90,0);
	SetScrollRange(hWnd,SB_HORZ,-90,90,0);

	SetScrollPos(hWnd,SB_VERT,vertpos,TRUE);
	SetScrollPos(hWnd,SB_HORZ,horzpos,TRUE);

	(*init)();
	xmousemin = 0;
	ymousemax = GetSystemMetrics(SM_CYSCREEN);
	undoindex = 0;

//	Load menu with shortened names if window is narrow

	GetClientRect(hWndMain,&rc);
	hEditMenu = (HMENU) LoadMenu(hInst,rc.right - rc.left < 930 ? "EDITGRAP_MEN" : "EDITGREX_MEN");
	DestroyMenu(hMainMenu);
	SetMenu(hWndMain,hEditMenu);

	PostMessage(hWnd,WM_PAINT,0,0L);
	section_edit = 1;

#endif

}

void EditSectionRedraw()
{
	int i,j,k;
	REAL yref,zref,x,y,z,dx,dy,dz,y0;
	REAL dxchar,dychar;
	REAL x1,y1,ya,aa,wl1,dx1,t1,chaoff;
	extern void xor_off(void);
#ifdef PROF
	extern REAL xmin0,xmax0,ymin0,ymax0;
#else
	REAL xmin0,xmax0,ymin0,ymax0;
#endif

#ifdef linux
	extern Window win;
	extern GC gc;
#endif

	first = 1;
	xor_off();
	(*clrgrf)();
#ifdef linux
	if(hDIB != NULL) {
		XCopyArea(display,
		(Pixmap) hDIB,	// source Pixmap data (a Pixmap is a "drawable")
		win,		// destination Window (also a "drawable")
		(GC) gc,
		0,			// x-coordinate of upper-left corner of source rectangle
		0,			// y-coordinate of upper-left corner of source rectangle
		bmWidth,	// width of source rectangle
		bmHeight,	// height of source rectangle
		scLeft,		// x-coordinate of upper-left corner of dest. rect.
		scTop);		// y-coordinate of upper-left corner of dest. rect.
	}
#else
	if(hdcBitmap != NULL) {
		BitBlt(
		hDC,
		scLeft,		// x-coordinate of upper-left corner of dest. rect.
		scTop,		// y-coordinate of upper-left corner of dest. rect.
		bmWidth,	// width of source rectangle
		bmHeight,	// height of source rectangle
		hdcBitmap,	// handle of source device context
		0,			// x-coordinate of upper-left corner of source rectangle
		0,			// y-coordinate of upper-left corner of source rectangle
		SRCCOPY);
	}
#endif
	xor_on();

	/*	FIND PLOTTING EXTREMITIES	*/

	if(!edit_scaled) {
		yymin = 1.0e+30;
		yymax = 0.0;
		zzmin = 1.0e+30;
		zzmax = -1.0e+30;
		for(j = 0 ; j < endlin ; j++) {
			for(i = 0 ; i < count ; i++) {
				if((alsosectnum[i] || editsectnum[i]) &&
					    i >= stsec[j] && i <= ensec[j]) {
					if(surfacemode || i) {
						yref = yline[j][i]*cosaz + xsect[i] * sinaz;
					}
					else {
						yref = (xsect[0]-yline[j][i])*sinaz;
					}
					if(yymax < yref) yymax = yref;
					if(yymin > yref) yymin = yref;
					zref = zline[j][i] + xsect[i] * sinel;
					if(zref < zzmin) zzmin = zref;
					if(zref > zzmax) zzmax = zref;
				}
			}
		}

		yymax += 0.0001;
		yymin -= 0.0001;
		zzmax += 0.0001;
		zzmin -= 0.0001;

		xri = 1.05*yymax - 0.05*yymin;

		/*	THUS SET PLOTTING RANGE	*/

		yymax += 0.25*(yymax - yymin);   /* extra space for curvature plot	*/
		zref = 0.20*(zzmax - zzmin);	/* extra space for convenience */
		zzmax += zref;
		zzmin -= zref;

		persp = 0;
		yra = yymax-yymin;
		zra = zzmax-zzmin;

		/*	SELECT WHICH OF Y-RANGE OR Z-RANGE LIMITS PLOTTING SCALE	*/

		y = (float)(xright - xleft);
		z = (float)(ybottom - ytop);
		if(y != 0.0)
			dy = yra / y;
		else
			dy = 1.0;
		if(z != 0.0)
			dz = zra / z;
		else
			dz = 1.0;

		if(dy > dz) {
			zra = dy * z;
			zzmin = (zzmin+zzmax-zra)*0.5;
			zzmax = zzmin + zra;
		}
		else {
			yra = dz * y;
			yymin = (yymin+yymax-yra)*0.5;
			yymax = yymin + yra;
		}
		dy = 0.05*yra;

		/* These define the range of coordinates across a full-hull view */
		/*	Save the un-zoomed view limits				*/
		xmin0 = yymin - dy;
		xmax0 = yymax + dy;
		ymin0 = zzmax + dy;
		ymax0 = zzmin - dy;

		/* These define the current range of coordinates		*/
		xmn1 = xmin0;
		xmx1 = xmax0;
		ymn1 = ymin0;
		ymx1 = ymax0;

		setranges(xmn1,xmx1,ymn1,ymx1);

		edit_scaled = TRUE;

	}
	else {
		xmn1 = xmin;
		xmx1 = xmax;
		ymn1 = ymin;
		ymx1 = ymax;
	}
	persp = 0;

	if(xgslope != 0.0) {
		dxchar = xchar / xgslope; /* character horizontal spacing	*/
		dychar = ychar / xgslope; /* character vertical size	*/
	}
	else {
		dxchar = 1.0;
		dychar = 1.0;
	}

	/*	The factor zsc keeps the on-screen size of ruling line
	intersection crosses constant
	*/
	if(xmax0 != xmin0)
		zsc = (xmx1 - xmn1)/(xmax0 - xmin0);
	else
	    zsc = 1.0;

	/*	Derive axis labels intervals from maximum dimensions	*/
	dx = 0.25f * yymax;
	dx = axincr(dx);
	dy = 0.10f * zra;
	dy = axincr(dy);

	/*	y0 is to be start value for labels	*/

	if(dy != 0.0)
		y0 = dy*((int) ((0.9*zzmin+0.1*zzmax)/dy));
	else
		y0 = 0.0;
	if(zzmin < 0.0) y0 = y0 - dy;

	/*	DRAW LATERAL AXIS	*/

	dy = 0.5 * dychar;	/* tick size */
	y1 = y0 + dy;
	xorcol(TEXTCOL);
	seteditranges(0.0);	// no transformation of range used for plotting
	(*newlin)();
	for(x = 0.0 ; x < yymax ; x += dx) {
		(*draw)(x,y0);
		(*draw)(x,y1);
		(*move)(x,y0);
	}

	dx = axincr(yymax);    /* axis label increment */
	ya = y0 - dy - dychar; /* place labels one tick length above axis */

	for(x = 0.0 ; x < yymax ; x += dx) {
		x1 = x - dxchar * (x < 10.0 ? 1.5 : 2.0);
		(*move)(x1,ya);
		if(dx >= 1.0)
			plint((int) x,3);
		else
			plfix(x,4,1);
	}

	/*	DRAW VERTICAL AXIS	*/

	dy = axincr(zra*0.25);
	(*newlin)();
	y1 = zzmax + 0.5f * dy;
	for(y = y0 ; y < y1 ; y += dy) {
		(*draw)(0.0,y);
		(*draw)(dxchar,y);
		(*move)(0.0,y);
	}

	/*	LABEL VERTICAL AXIS	*/

	/*	x is to be position at which labels are written	*/
	x = -5.0f * dxchar;

	chaoff = 0.5f * dychar * invert;
	dy = axincr(zra);
	if(dy != 0.0)
		y1 = dy * ((int) (y0 / dy));
	else
		y1 = 1.0;
	if(y0 > 0.0) y1 += dy;
	for(y = y1 ; y < zzmax ; y += dy) {
		(*move)(x,y - chaoff);
		if(dy >= 1.0) {
			aa = y * invert + 0.5f;
			plint((int) (200.0+aa)-200,4);
		}
		else {
			plfix(y*invert,4,1);
		}
	}

	/*	DRAW WATERLINE	*/

	wl1 = wl - (beta*xsect[editsection] + hwl[editsection]);
	xorcol(WATLCOL);
	(*move)(wl1*sina,wl1*cosa);
	(*draw)(yymax*cosa-wl1*sina,wl1*cosa+yymax*sina);
	(*plstr)("WL");

	/*	DRAW "ALSO PLOT" SECTIONS	*/

	xorcol(ALSOCOL);
	for(i = surfacemode ; i < count ; i++) {
		if(i != editsection && alsosectnum[i]) drawsec(i);
	}

/*	For some nutty reason, we have do do this twice under X11	*/
#ifdef linux
	xorcol(EDISCOL);
	drawsec(editsection);
#endif

	/*	Draw the edit section in xor mode using the edit section colour */

	draw_edit_sect(editsection,yra,offset,&first,zsc);

#ifndef STUDENT
	seteditranges(xsect[editsection]);	// plot ranges set for the edit section
	dx1 = zsc*yra*0.01f;
	xorcol(DEVECOL);
	for(i = 1 ; i < extlin ; i++) {
		if((k = developed[i]) >= 0) {
			for(j = 0 ; j < rulings[i] ; j++) {
				x = xstart[k][j] - xsect[editsection];
				y = xend  [k][j] - xsect[editsection];
				if(x*y < 0.0) {
					t1 = y/(y-x);
					x = yend[k][j]+t1*(ystart[k][j]-yend[k][j]);
					y = zend[k][j]+t1*(zstart[k][j]-zend[k][j]);
					(*move)(x-dx1,y);
					(*draw)(x+dx1,y);
					(*move)(x,y+dx1);
					(*draw)(x,y-dx1);
				}
			}
		}
	}
#endif

#ifdef DXFOVL
	(*colour)(9);
	(*newlin)();
	xor_off();

	for(i = 0 ; i < n_ovl ; i++) {
		if(x_ovl[i] > 1.0e+29 || y_ovl[i] < -1.0e+29 || z_ovl[i] < -1.0e+29) {
			(*newlin)();
		} else if(y_ovl[i] < 0.0) {
			seteditranges(x_ovl[i]);	// plot ranges set for the edit section
			(*draw)(-y_ovl[i],-z_ovl[i]);
		}
	}
	xor_on();
#endif

	/*	Draw lines and control point lines	*/

	xorcol(ALTLCOL);
	if(changeline < 0) {
		j = 0;
		k = endlin;
	}
	else {
		j = changeline;
		k = changeline+1;
	}
	changeline = -1;
	seteditranges(0.0);
	for( ; j < endlin ; j++) {
		if(j == numlin) xorcol(TANKCOL);

		if(showlinenum[j]) {
			draw_edit_line(j,TRUE);
			if(offset == 0 && j < k) draw_edit_line(j,FALSE);
		}
	}

	if(scrdev == 0) SwapBuf();
}

void EditSectionRepaint(void)
{
	EditSectionRedraw();
	SwapBuf();
}

#ifdef linux

#define VK_SHIFT XK_Shift_L
#define VK_F1 XK_F1
#define VK_F2 XK_F2
#define VK_TAB XK_Tab
#define VK_UP XK_Up
#define VK_DOWN XK_Down
#define VK_LEFT XK_Left
#define VK_RIGHT XK_Right
#define VK_ESCAPE XK_Escape

#endif

#ifdef PROF
void tankselect(int code,HWND hWndDlg)
{
	int tank,l1,l2,i,i1,i2;
	struct {
		int index;					/* index of result in table (not used) */
		char *string;				/* pointer to result (not used) */
		char *table[MAXTANKS+1];	/* table of strings for listbox, null string terminator */
	}
	xlist;
	char text[MAX_PATH] = "";
#ifdef linux
	XmString *xstrtab,xstr;
	extern Widget wCheckBox[];
#endif


	for(i = 0 ; i < ntank ; i++) xlist.table[i] = tankdesc[i];
	xlist.table[i] = "";
	xlist.index = -1;
	xlist.string = text;
	if(getdlg(TANKSELECT,INP_LBX,(void *) &xlist,-1) && xlist.index >= 0 && xlist.index < ntank) {
		tank = xlist.index;
		l1 = fl_line1[tank];
		l2 = fl_line1[tank+1];
		i1 = stsec[l1];
		i2 = ensec[l1];
		sprintf(text,"%d:%d",i1,i2);
#ifdef linux
		xstr = XmStringCreateSimple(text);
		XtVaSetValues(wEdit[0],XmNlabelString,xstr,NULL);
		XmStringFree(xstr);

		if(wEdit[1] != 0) {	// "Show" dialog box has secobnd edit box
			sprintf(text,"%d:%d",l1+1,l2+1);
			xstr = XmStringCreateSimple(text);
			XtVaSetValues(wEdit[1],XmNlabelString,xstr,NULL);
			XmStringFree(xstr);
			if(fl_right[tank])
		    	XtVaSetValues(wCheckBox[1],XmNset,1,NULL);
			else
		    	XtVaSetValues(wCheckBox[0],XmNset,1,NULL);
		} else {
			showstbd = fl_right[tank];
			showport = !showstbd;
		}
#else
		SetDlgItemText(hWndDlg,DLGEDIT+0,text);
		if(GetDlgItem(hWndDlg,DLGEDIT + 1) != NULL) {	// "Show" dialog box has secobnd edit box
			sprintf(text,"%d:%d",l1+1,l2+1);
			SetDlgItemText(hWndDlg,DLGEDIT+1,text);
			if(fl_right[tank])
				SendDlgItemMessage(hWndDlg,DLGLOG + 1,BM_SETCHECK,BST_CHECKED,0l);
			else
				SendDlgItemMessage(hWndDlg,DLGLOG + 0,BM_SETCHECK,BST_CHECKED,0l);
		} else {
			showstbd = fl_right[tank];
			showport = !showstbd;
		}
#endif
		for(i = 0 ; i < count ; i++) editsectnum[i] = FALSE;
		for(i = i1 ; i <= i2 ; i++) editsectnum[i] = TRUE;
		editsection = max(i1,min(i2,editsection));
		master[i1] = TRUE;
		master[i2] = TRUE;
		endlin = extlin;
		showedittanks = TRUE;
	}
}
#endif

void EditSectionKeyDown(int keysym)
{
	int key = toupper(keysym);
	int i,j,k;
	REAL del1;
	char text[MAX_PATH];
	char masterstring[MAX_PATH];
	void xor_on(void),xor_off(void);
	RECT rc;
#ifdef linux
	extern Widget mainWindow;
	int w,h;
#endif

	lastkey = key;
	adjusting_bitmap = FALSE;
	if(offset == -1) offset = 1;
	switch(keysym) {
#ifndef linux
	case VK_SHIFT:
		shiftdown = TRUE;
		break;
#endif

	case VK_F1:	/* context-sensitive help */
		context(context_id);
		linevalue = 0;
#ifdef linux
		edit_scaled = (hDIB != NULL);
#else
		edit_scaled = (hdcBitmap != NULL);
#endif
		EditSectionRepaint();
		break;

	case VK_F2:	/* quick save */
		save_file();
		linevalue = 0;
		break;

	case VK_TAB:	/* XK_Tab maps to this */
#ifdef linux
	case XK_KP_Tab:			/* shift-tab sometimes maps to this */
	case XK_ISO_Left_Tab:	/* shift-tab sometimes maps to this */
#endif

		i = editsection;
		if(shiftdown) {
			while(--editsection >= surfacemode) {
				if((v7mode || master[editsection]) && editsectnum[editsection]) break;
			}
			if(editsection < 0) editsection = i;
		}
		else {
			while(++editsection < count) {
				if((v7mode || master[editsection]) && editsectnum[editsection]) break;
			}
			if(editsection >= count) editsection = i;
		}

/*	Undraw the old section in highlighted colour	*/

		if(scrdev == 1) {
			draw_edit_sect(i,yra,offset,&first,zsc);
			if(alsosectnum[i]) {

/*	If required, draw the old section in base colour	*/
				xorcol(ALSOCOL);
				drawsec(i);
			}

/*	If required, undraw the new section in base colour	*/
			if(alsosectnum[editsection]) {
				xorcol(ALSOCOL);
				drawsec(editsection);
			}
/*	Draw the new section in highlighted colour	*/
			draw_edit_sect(editsection,yra,offset,&first,zsc);

			sprintf(text,"Section %d, line %d   ",editsection,editline+1);
			pstrbkg(text,1,0,0,0);
		} else {
			sprintf(text,"Section %d, line %d   ",editsection,editline+1);
			pstrbkg(text,1,0,0,0);
			EditSectionRepaint();
		}

		linevalue = 0;
		break;

	case VK_UP:
	case VK_DOWN:
		if(editline >= 0) {
			saveundo();

			/*	DOWN- OR UP-ARROW				*/

			del1 = (keysym == VK_DOWN) ? del : -del;
			if(scrdev == 1) {
				for(i = surfacemode ; i < count ; i++) {
					setcolour(scrcolour[ALSOCOL] ^ scrcolour[0],v7mode || !master[i],0);
					if(i != editsection && alsosectnum[i]) drawsec(i);
				}
				draw_edit_sect(editsection,yra,offset,&first,zsc);
				draw_edit_line(editline,offset);
			}
			if(offset == 1) {
				if(!follow) {
					fix_vertical_controls(editsection,editline,zline[editline][editsection] + del1);
					setall(zline,editsection,editline,zline[editline][editsection] + del1);
				}
				else if(editline < extlin) {
					movpoi(editsection,editline,editline+1,1.0 - del1);
				}
				if(scrdev == 1) showvalues(yline[editline][editsection],zline[editline][editsection]*invert);

			}
			else if(editline > 0) {

				/*	Z-CONTROL: MOVE TOWARDS UPPER END ON UPARROW, AWAY ON DOWNARROW	*/

				zcont[editline][editsection] += del1;
				if(scrdev == 1) showvalues(	ycont[editline][editsection],zcont[editline][editsection]);
			}
			fairlines(editline);
			if(scrdev == 1) {
				for(i = surfacemode ; i < count ; i++) {
					setcolour(scrcolour[ALSOCOL] ^ scrcolour[0],v7mode || !master[i],0);
					if(i != editsection && alsosectnum[i]) drawsec(i);
				}
				draw_edit_sect(editsection,yra,offset,&first,zsc);
				draw_edit_line(editline,offset);
			}
			if(scrdev == 0) {
				edit_scaled = FALSE;
				EditSectionRepaint();
			}
		}
		linevalue = 0;
		break;

	case VK_LEFT:
	case VK_RIGHT:
		if(editline >= 0) {
			saveundo();

			del1 = (keysym == VK_RIGHT) ? del : -del;
			if(scrdev == 1) {
				for(i = surfacemode ; i < count ; i++) {
					setcolour(scrcolour[ALSOCOL] ^ scrcolour[0],v7mode || !master[i],0);
					if(i != editsection && alsosectnum[i]) drawsec(i);
				}
				draw_edit_sect(editsection,yra,offset,&first,zsc);
				draw_edit_line(editline,offset);
			}
			if(offset == 1) {
				if(!follow) {
					setall(yline,editsection,editline,yline[editline][editsection]+del1);
				}
				else if(editline > 0) {
					movpoi(editsection,editline,editline,del1);
				}
				if(scrdev == 1) showvalues(yline[editline][editsection],
					zline[editline][editsection]*invert);

			}
			else if(editline > 0) {

				/*	Y-CONTROL: MOVE TOWARDS LOWER END ON LEFTARROW, AWAY ON RIGHTARROW	*/
				ycont[editline][editsection] += del1;
				if(scrdev == 1) showvalues(ycont[editline][editsection],zcont[editline][editsection]);
			}
			fairlines(editline);
			if(scrdev == 1) {
				for(i = surfacemode ; i < count ; i++) {
					setcolour(scrcolour[ALSOCOL] ^ scrcolour[0],v7mode || !master[i],0);
					if(i != editsection && alsosectnum[i]) drawsec(i);
				}
				draw_edit_sect(editsection,yra,offset,&first,zsc);
				draw_edit_line(editline,offset);
			}
			if(scrdev == 0) {
				edit_scaled = FALSE;
				EditSectionRepaint();
			}
		}
		linevalue = 0;
		break;

	default:
keypress:
		if(key >= '0' && key <= '9') {
			i = key - '0';
			linevalue = 10 * linevalue + i;
			editline = linevalue - 1;
			if(i <= extlin) {
				sprintf(text,"Section %d, line %d   ",editsection,linevalue);
				if(scrdev == 1) pstrbkg(text,1,0,0,0);
			}
#ifndef STUDENT
		}
		else if(key == 'Z') {		/* zoom */
			update_func = EditSectionRepaint;
			context_id = 2050;
			edit_scaled = (hdcBitmap != NULL);
			linevalue = 0;
			initzoom();
#endif
		}
		else if(key == 'O' || key == 'F' || key == 'C') {

			was_offset = offset;
			offset = key != 'C';
			follow = key == 'F';
			context_id = offset ? 2041 : follow ? 2043 : 2042;
			if(scrdev == 1) {
				draw_edit_sect(editsection,yra,offset,&first,zsc);
				draw_edit_sect(editsection,yra,offset,&first,zsc);

			/*	If changing from offset to control mode, draw in the control lines.
			If changing from control to offset mode, draw out the control lines	*/
				if(was_offset != offset) {
					xorcol(WATLCOL);
					for(i = 0 ; i < endlin ; i++) if(showlinenum[i]) draw_edit_line(i,FALSE);
				}
			} else {
				EditSectionRepaint();
			}
			linevalue = 0;
		}
		else if(key == 'D') {
			/*						DOUBLE CHANGE INCREMENT	*/
			del *= 2.0;
			showincr(del);
			context_id = 2044;
			linevalue = 0;
		}
		else if(key == 'H') {
			/*						HALVE CHANGE INCREMENT	*/
			del *= 0.5;
			showincr(del);
			context_id = 2045;
			linevalue = 0;

		}
		else if(key == 'R') {
			/*						REDRAW SECTION	*/
#ifdef linux
			edit_scaled = (hDIB != NULL);
#else
			edit_scaled = (hdcBitmap != NULL);
#endif
			context_id = 2046;
			EditSectionRepaint();
			linevalue = 0;

		}
		else if(key == 'E') {
			/*						Edit sections	*/
			listseq(editsectnum,text,count);
			if(getdlg(EDITSECTIONS,
			INP_STR,(void *) text,
#ifdef PROF
			INP_PBF,tankselect,
#endif
			-1) && multproc(text,editsectnum,MAX_PATH)) {
				context_id = 2047;
				for(i = surfacemode ; i < count ; i++) {
					if(editsectnum[i]) {
						if(editsection < i) editsection = i;
						break;
					}
				}
				for(i = count-1 ; i >= surfacemode ; i--) {
						if(editsectnum[i]) {
					if(editsection > i) editsection = i;
						break;
					}
				}
				EditSectionRepaint();
				linevalue = 0;
			}

		}
		else if(key == 'S') {
			/*					Show other sections	*/
			context_id = 2048;
			if(getdlg(EDITSHOW,
				INP_STR,(void *) alsosect,
				INP_STR,(void *) showline,
#ifdef PROF
				INP_LOG,(void *) &showport,
				INP_LOG,(void *) &showstbd,
				INP_PBF,(void *) tankselect,
#endif

			-1)) {
				if(!multproc(alsosect,alsosectnum,maxsec))
					message("Invalid shown section specification");
				if(!multproc(showline,showlinenum,maxlin)) {
					message("Invalid shown line specification");
				}
				else {
					memcpy(showlinenum,showlinenum+1,extlin*sizeof(int));
				}
#ifndef STUDENT
				showedittanks = showport || showstbd;
				endlin = showedittanks ? extlin : numlin;
#endif
			}
			EditSectionRepaint();
			linevalue = 0;

			/*	Undo alterations					*/

		}
		else if(key == 'U') {

			context_id = 2049;
			if(undoindex > 0) {
				undoindex--;
				i = undosect[undoindex];
				j = undoline[undoindex];

				if(editmode[undoindex]) {
					yline[j][i] = yundo[undoindex];
					fix_vertical_controls(i,j,zundo[undoindex]);
					zline[j][i] = zundo[undoindex];
				}
				else {
					ycont[j][i] = yundo[undoindex];
					zcont[j][i] = zundo[undoindex];
				}
				fairlines(j);
				edit_scaled = FALSE;
				EditSectionRepaint();
			}
			linevalue = 0;
			break;

		}
		else if(key == 'T') {
			context_id = 2051;
			revsec(editsection);
			linevalue = 0;
			EditSectionRepaint();
			break;

		}
		else if(key == 'N') {
			context_id = 2052;
			sinaz = 0.0;
			cosaz = 1.0;
			sinel = 0.0;
			horzpos = 0;
			vertpos = 0;
			edit_scaled = (hdcBitmap != NULL);
			EditSectionRepaint();
			linevalue = 0;
			break;

		}
		else if(key == 'M') {

			/* define master sections */
			context_id = 2053;
			master[0] = TRUE;
			listseq(master,masterstring,count);
			if(getdlg(MASTER,INP_STR,(void *) masterstring,-1)) {
				multproc(masterstring,master,maxsec);
			}
			master[0] = TRUE;
			master[count-1] = TRUE;
			linevalue = 0;
			EditSectionRepaint();
			break;

		}
		else if(key == 'Q' || key == VK_ESCAPE) {
			context_id = 2055;
			xor_off();
#ifdef linux
			EditQuit = 1;
#else
			procedit(hWnd,WM_QUIT,0,0);
#endif
			linevalue = 0;
#ifdef linux
			if(hDIB != NULL) {
				XFreePixmap(display,(Pixmap) hDIB);
			}
#else
			if(hdcBitmap != NULL) {
				DeleteObject(hDIB);
				DeleteDC(hdcBitmap);
				hdcBitmap = NULL;
			}
#endif
			break;

		}
		else if(key == 'B') {	// reference bitmap
			context_id = 2054;
			strcpy(masterstring,"hullform.bmp");
#ifdef linux
			if(hDIB != NULL) {
				XFreePixmap(display,(Pixmap) hDIB);
				hDIB = NULL;
			}
			if(openfile(masterstring,"r","Open a reference bitmap","Windows bitmaps (*.BMP)\0*.bmp\0","*.bmp",dirnam,NULL)) {
				hDIB = (HBITMAP) LoadBMP(masterstring,&bmWidth,&bmHeight);
				XtVaGetValues(mainWindow,
					XmNwidth,&w,
					XmNheight,&h,
					NULL);
				scTop = (h - bmHeight)/2;
				scLeft = (w - bmWidth)/2;
			}
#else
			if(hdcBitmap != NULL) {
				DeleteObject(hDIB);
				DeleteDC(hdcBitmap);
				hdcBitmap = NULL;
			}
			if(openfile(masterstring,"r","Open a reference bitmap","Windows bitmaps (*.BMP)\0*.bmp\0","*.bmp",dirnam,NULL)) {
				hDIB = (HBITMAP) LoadBMP(masterstring,&bmWidth,&bmHeight);
				hdcBitmap = CreateCompatibleDC(hDC);
				hbmpOld = SelectObject(hdcBitmap,hDIB);
				GetClientRect(hWnd,&rc);
				scTop = (rc.bottom - bmHeight)/2;
				scLeft = (rc.right - bmWidth)/2;
				(void) getdlg(MOVE_BITMAP,-1);
			}
#endif
			offset = -1;
		}
		break;
	}
}

void EditSectionLeftButtonDown(int xm,int ym)
{
	int i = editsection;
	int j = editline;
	char text[80];
#ifdef linux
	extern Widget glw;
#else
	RECT rc;
#endif

	dragging = TRUE;
	if(offset == 1)
		editline = nearest_offset (&editsection,&xm,&ym);
	else if(offset == 0)
	    editline = nearest_control(&editsection,&xm,&ym);
	else {	// dragging image corner
		adjusting_bitmap = TRUE;
		previous_xm = xm;
		previous_ym = ym;
		return;
	}

	if(editsection < 0) {
		dragging = FALSE;	// need to cancel mouse-drag to allow picking up of point again
		if(MessageBox(hWnd,"You have tried to select a point on a non-master section. Do you want this section to be a master section?",
			"YOU CAN NOT EDIT A NON-MASTER SECTION",MB_ICONQUESTION | MB_YESNO) == IDYES) {
			editsection = -editsection;
			master[editsection] = TRUE;
			edit_scaled = FALSE;
		}
		else {
			editline = j;
			editsection = i;
		}
		return;
	}

	saveundo();

	/*	(xm,ym) are now window coordinates of point to which mouse is	*/
	/*	to be moved							*/

#ifdef linux
	if(scrdev == 0)
		XWarpPointer(display,None,XtWindow(glw),0,0,0,0,xm,ym);
	else
		XWarpPointer(display,None,win,0,0,0,0,xm,ym);
#else
	GetWindowRect(hWnd,&rc);
	SetCursorPos(xm + rc.left,ym + rc.top);
#endif
	if(scrdev == 1) {	// GDI
		xor_on();
		if(editline != j) {
			xorcol(LINECOL);
			draw_edit_line(j,TRUE);
			xorcol(ALTLCOL);
			draw_edit_line(j,TRUE);
			draw_edit_line(editline,TRUE);
			xorcol(LINECOL);
			draw_edit_line(editline,TRUE);
		}

		if(editsection != i) {

		/*	undraw the old ...	*/

			draw_edit_sect(i,yra,offset,&first,zsc);
			if(alsosectnum[i]) {
				xorcol(ALSOCOL);
				drawsec(i);
			}

		/*	draw the new ...	*/

			if(alsosectnum[editsection]) {
				xorcol(ALSOCOL);
				drawsec(editsection);
			}
			draw_edit_sect(editsection,yra,offset,&first,zsc);
		}
	}

	sprintf(text,"Section %d, line %d   ",editsection,editline+1);
	pstrbkg(text,1,0,0,0);
}

void EditSectionRightButtonDown(int xm,int ym)
{
	int i = 0;

	if(offset == -1) {	// mark origin
		scTop -= (ygorigin - (ymaxi - ym));
		scLeft -= (xm - xgorigin);
		EditSectionRepaint();
		return;
	}

	if(offset == 1)
		editline = nearest_offset(&i,&xm,&ym);
	else
		editline = nearest_control(&i,&xm,&ym);
	editline = abs(editline);
	line_properties(editline);
}

void EditSectionMouseMove(int xm,int ym)
{
	REAL x,y,x1;
	REAL a,hb,c,hd;
	int j;
	GLint viewport[4];
	GLdouble modelview[16];
	GLdouble projection[16];
	GLfloat winY;
	GLdouble posX,posY,posZ;

	if(adjusting_bitmap) {	// bitmap dragging mode
		scLeft += xm - previous_xm;
		scTop += ym - previous_ym;
		EditSectionRepaint();
		previous_xm = xm;
		previous_ym = ym;
		return;
	}

	seteditranges(xsect[editsection]);
	if(xgslope == 0.0) xgslope = 1.0;
	if(ygslope == 0.0) ygslope = 1.0;

	if(scrdev == 0) {
		glGetDoublev( GL_MODELVIEW_MATRIX, modelview );
		glGetDoublev( GL_PROJECTION_MATRIX, projection );
		glGetIntegerv( GL_VIEWPORT, viewport );
		winY = (float)viewport[3] - (float) (ym);
		gluUnProject((GLfloat) xm, winY,(GLfloat) 1.0, modelview, projection, viewport, &posX, &posY, &posZ);
		x = posX;
		y = posY;
	} else {
		x = xmin + ((REAL) xm + 0.5) / xgslope;
		y = ymax - ((REAL) ym + 0.5) / ygslope;
	}
	curtime = clock();
	if(curtime - prevtime > CLK_TCK/5) {
		showvalues(x,y * invert);
		prevtime = curtime;
	}
	if(dragging && editline >= 0) {
		changex = TRUE;
		if(surfacemode || editsection > 0) {
			if(fabs(cosaz) > 0.1745329)
				x /= cosaz;
			else
				changex = FALSE;
		}
		else if(editsection == 0) {
			if(fabs(sinaz) > 0.1745329) {
				x = - (x-yline[stemli][1]*cosaz)/sinaz;
				y = y + x*sinel;
			}
			else {
				changex = FALSE;
			}
		}
		else {
			return;
		}

		/*	hence calculate new hull form parameters		*/

		if(scrdev == 1) {	// GDI
			xor_on();
			xorcol(offset == 1 ? LINECOL : WATLCOL);
			draw_edit_sect(editsection,yra,offset,&first,zsc);
			draw_edit_line(editline,offset);
		}
		if(offset) {
			if(changex) yline[editline][editsection] = x;
			fix_vertical_controls(editsection,editline,y);
			zline[editline][editsection] = y;
		}
		else {
			zcont[editline][editsection] = y;
			j = editline;
			while(--j > 0) {
				if(stsec[j] <= editsection && ensec[j] >= editsection) break;
			}
			if(stsec[editline] <= editsection && ensec[editline] >= editsection && relcont[editline]) {
				if(editline > 0) {
					getparam(editsection,j,&a,&hb,&c,&hd);
				}
				else {
					c = 1.0;
					a = 0.0;
				}
				if(c != 0.0) {
					x1 = yline[j][editsection] - a/c*(y - zline[j][editsection]);
					x -= x1;
				}
			}
			if(changex) ycont[editline][editsection] = x;
		}
		if(scrdev == 1) {
			xorcol(offset ? LINECOL : WATLCOL);
			draw_edit_sect(editsection,yra,offset,&first,zsc);
			fairlines(editline);
			draw_edit_line(editline,offset);
		} else {
			seteditranges(0.0);	// needed for OpenGL scaling
			fairlines(editline);
			EditSectionRepaint();
		}
	}
}

void EditSectionMouseWheel(int clicks)
{
	; // no function yet
}

#ifdef linux

void EditHorzScroll(Widget w, XtPointer client_data, XtPointer call_data)
{
	XmScrollBarCallbackStruct *sc = (XmScrollBarCallbackStruct *) call_data;

	horzpos = sc->value;
	if(horzpos >=  180) horzpos -= 360;
	if(horzpos <= -180) horzpos += 360;

	sinaz = sin((double) (-horzpos) * 0.01745329);
	cosaz = cos((double) (-horzpos) * 0.01745329);
	edit_scaled = (hdcBitmap != NULL);
	EditSectionRepaint();
}

void EditVertScroll(Widget w, XtPointer client_data, XtPointer call_data)
{
	REAL dist,angle;
	XmScrollBarCallbackStruct *sc = (XmScrollBarCallbackStruct *) call_data;

	vertpos = sc->value;
	if(vertpos >=  90) vertpos = -90;
	if(vertpos <= -90) vertpos = 90;

	sinel = sin((double) (-vertpos) * 0.01745329);
	edit_scaled = (hdcBitmap != NULL);
	EditSectionRepaint();
}

void EditMenuCallback(Widget w, XtPointer client_data, XtPointer call_data)
{
	EditSectionKeyDown(EditMenuChar[(int) client_data]);
}


#else

LRESULT procedit(HWND hWndProc,UINT msg,WPARAM wParam,LPARAM lParam)
{

	int    i,j;
	int    xm,ym;
	PAINTSTRUCT ps;

	switch(msg) {
	case WM_SIZE:
		GetSizes(hWndProc);
		PostMessage(hWndProc,WM_COMMAND,204,0);		// Re-initialise Edit Sections after window rebuild
		goto quitpoint;

	case WM_INITMENUPOPUP:
		context_id = 2041 + LOWORD(lParam);
		break;

	case WM_ENTERIDLE:
		if((wParam == MSGF_MENU) && (GetKeyState(VK_F1) & 0x8000)) {
			HelpRequest = 1;
			PostMessage(hWnd,WM_KEYDOWN,VK_RETURN,0L);
		}
		else {
			return(DefWindowProc(hWndProc,msg,wParam,lParam));
		}
		break;

	case WM_VSCROLL:
		switch(LOWORD(wParam)) {
		case SB_ENDSCROLL:
		case SB_LINEUP:
		case SB_LINEDOWN:
			if(wParam == SB_LINEDOWN && vertpos < 90)
				vertpos++;
			else if(wParam == SB_LINEUP && vertpos > -90)
				vertpos--;
			SetScrollPos(hWnd,SB_VERT,vertpos,TRUE);
			sinel = sin((double) (-vertpos) * 0.01745329);
			if(wParam != SB_ENDSCROLL || dragscroll) {
				dragscroll = 0;
				PostMessage(hWnd,WM_PAINT,0,0l);
			}
			break;
		case SB_THUMBPOSITION:
			vertpos = (short int) HIWORD(wParam);
			dragscroll = 1;
			break;
		default:
			return(DefWindowProc(hWndProc,msg,wParam,lParam));
		}
		break;

	case WM_HSCROLL:
		switch(LOWORD(wParam)) {
		case SB_ENDSCROLL:
		case SB_LINERIGHT:
		case SB_LINELEFT:
			if(wParam == SB_LINERIGHT && horzpos < 90)
				horzpos++;
			else if(wParam == SB_LINELEFT && horzpos > -90)
				horzpos--;
			SetScrollPos(hWnd,SB_HORZ,horzpos,TRUE);
			sinaz = sin((double) (-horzpos) * 0.01745329);
			cosaz = cos((double) (-horzpos) * 0.01745329);
			if(wParam != SB_ENDSCROLL || dragscroll) {
				dragscroll = 0;
				edit_scaled = (hdcBitmap != NULL);
				PostMessage(hWnd,WM_PAINT,0,0l);
			}
			break;
		case SB_THUMBPOSITION:
			horzpos = (short int) HIWORD(wParam);
			dragscroll = 1;
			break;
		default:
			return(DefWindowProc(hWndProc,msg,wParam,lParam));
		}
		break;

	case WM_PAINT:
		if(hWndProc == hWnd) {
			InvalidateRect(hWndProc,NULL,TRUE);
			BeginPaint(hWndProc,&ps);
			SetScrollPos(hWnd,SB_HORZ,horzpos,TRUE);
			SetScrollPos(hWnd,SB_VERT,vertpos,TRUE);

			EditSectionRepaint();

			EndPaint(hWndProc,&ps);
		}
		else {
			return(DefWindowProc(hWndProc,msg,wParam,lParam));
		}
		break;

		/*	End PAINT code	*/

		/*	Key pressed:	*/

	case WM_KEYUP:
		if(wParam == VK_SHIFT) {
			shiftdown = FALSE;
		}
		else {
			return(DefWindowProc(hWndProc,msg,wParam,lParam));
		}
		break;

	case WM_KEYDOWN:
		EditSectionKeyDown((int) wParam);
		break;

	case WM_COMMAND:
		EditSectionKeyDown(EditMenuChar[wParam - 101]);
		break;

	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
		dragging = FALSE;
		break;

	case WM_LBUTTONDOWN:
		if(hWndProc != hWnd) return(DefWindowProc(hWndProc,msg,wParam,lParam));
		EditSectionLeftButtonDown(LOWORD(lParam),HIWORD(lParam));
		break;

	case WM_RBUTTONDOWN:
		if(hWndProc != hWnd) return(DefWindowProc(hWndProc,msg,wParam,lParam));
		EditSectionRightButtonDown(LOWORD(lParam),HIWORD(lParam));
		break;

	case WM_MOUSEMOVE:
		arrowcursor();
		if(hWndProc != hWnd) return(DefWindowProc(hWndProc,msg,wParam,lParam));
		if(dragging) {
			EditSectionMouseMove(LOWORD(lParam),HIWORD(lParam));
		}
		break;

	case WM_MOUSEWHEEL:
		i = (short) HIWORD(wParam);
		EditSectionMouseWheel(i);	/* number of wheel clicks */
		break;

	case WM_CLOSE:
	case WM_DESTROY:
	case WM_QUIT:
quitpoint:
		zoom = 0;
		scaled = FALSE;
		numbetw = oldnumbetw;
		changed |= undoindex > 0;
#ifdef PROF
		transom = transom_setting;
		recalc_transom();
		for(j = 1 ; j < numlin ; j++) {
			if(strmode[j] >= 0) calc_stringers(j);
		}
#endif
		section_edit = 0;
		memfree(discard);
		memfree(yundo);
		memfree(zundo);
		memfree(undoline);
		memfree(undosect);
		memfree(editmode);

		if(scrdev == 1) SetROP2(hDC,R2_COPYPEN);

		hMainMenu = LoadMenu(hInst,"HULLFORM_MEN");
		DestroyMenu(hEditMenu);
		SetMenu(hWndMain,hMainMenu);
		makemenu();
		numtool = savetool;
		scrollable = TRUE;
		InitToolbar();
		InitWorkwin(FALSE);
		cls(FALSE);
		update_func = NULL;
		check_negative_offsets();
		break;

	default:
		return(DefWindowProc(hWndProc,msg,wParam,lParam));
	}
	return(0l);
}

#endif

void draw_edit_line(int il,int offset)
{
	int i,j,k;
	int sts = stsec[il];
	int ens = ensec[il];
	REAL *yc,*zc,*yli;
	int stl = sts;
	REAL a,hb,c,hd;
	int numsave = numbetw;

	numbetw = 0;

	seteditranges(0.0);

	if(sts < surfacemode) {
		sts = surfacemode;
		stsec[il] = sts;
	}

	if(offset == 1) {
		if(il >= 0) {
			if(il < numlin) xorcol(LINECOL);
			draw_line(il,cosaz,cosaz,10,sts,ens,999,discard,discard,discard,&i,&i,&i,TRUE);
		}
	}
	else if(il >= 1) {	/* draw line of control points */
		k = extlin+2;		/* where line will be created */
		yc = yline[k];
		zc = zline[k];
		yli = yline[il];	/* source of line data */

		/*	Allow extra point where an extra control point is defined */
		/*	if(stsec[il-1] < sts) sts--;
		if(ensec[il-1] > ens) ens++;	*/
		for(i = sts ; i <= ens ; i++) {
			zc[i] = zcont[il][i];
			if(i < stl && i == 0) {
				yc[i] = yli[il-1]+ycont[il][i];
			}
			else if(relcont[il]) {
				getparam(i,il,&a,&hb,&c,&hd);
				yc[i] = yline[il][i] + 0.5*a;
			}
			else {
				yc[i] = ycont[il][i];
			}
		}
		stsec[k] = sts;
		ensec[k] = ens;

		j = il-1;
		while(stsec[j] > 0 && j > 0) j--;
#ifndef STUDENT
		a = zline[il][sts]-zline[j][sts];
		if(a != 0.0) a = (zline[il][sts]-zcont[il][sts]) / a;
		radstem[k] = a*radstem[j]+(1.0-a)*radstem[il];
#endif
		xorcol(WATLCOL);
		draw_line(k,cosaz,cosaz,10,stsec[k],ensec[k],999,discard,discard,
			discard,&i,&i,&i,TRUE);
	}
	numbetw = numsave;
}

void draw_edit_sect(INT is,REAL yra,INT offset,INT *first,REAL zsc)
{
	int		i,j;
	REAL	dx1,dy1;
	REAL	x,y;
	REAL	aa,cc,a,hb,b,c,hd,d;
	REAL	aa1,cc1;
	REAL	top,gtst,div;
	static	REAL	scale = 1.0;
	REAL	yl,zl;
	REAL	t1,x1,y1;
	REAL	xright;
	int		is0 = (is >= maxsec-1) ? 0 : is;
	extern COLORREF scrcolour[8];
	extern HDC hDC;
	extern REAL xgslope,ygslope;
#ifndef STUDENT
	int		tank;
#endif
#ifdef PLATEDEV
	int k;
#endif

	xorcol(EDISCOL);
	if(is != 0 ) seteditranges(xsect[is]);

	/*	DRAW THE CURRENT FORM OF THE SECTION, AND MARK KEY POINTS	*/

	if(xgslope == 0.0) xgslope = 1.0;
	if(ygslope == 0.0) ygslope = 1.0;

	dx1 = (REAL) xchar / xgslope;
	dy1 = (REAL) ychar / ygslope;

#ifndef STUDENT
	tank = 0;
#endif

//	Show offset points

	for(i = 0 ; i < extlin ; i++) {

#ifndef STUDENT
		while(tank < ntank && i > fl_line1[tank]) tank++;

		if(i == fl_line1[tank]) {
			if(	 fl_right[tank] && !showstbd ||
				    !fl_right[tank] && !showport ) {
				tank++;
				if(tank >= ntank) break;
				i = fl_line1[tank]-1;
				continue;
			}
		}
#endif
		if(stsec[i] > is || ensec[i] < is) continue;

		if(surfacemode || is) {
			x = yline[i][is]*cosaz;
			y = zline[i][is];
		}
		else {
			seteditranges(xsect[0]-yline[i][0]);
			x = cosaz*yline[stemli][1];
			y = zline[i][0];
		}
		(*move)(x-dx1,y);
		(*draw)(x+dx1,y);
		(*move)(x,y+dx1);
		(*draw)(x,y-dx1);
		if(*first) {
			(*move)(x - 2.5 * dx1,y + dy1);
			plint(i+1,2);
		}
	}

	drawsec(is);

	/*	Draw ruling line intersections	*/

#ifdef PLATEDEV
	dx1 = zsc*yra*0.01;
	xorcol(3);
	seteditranges(xsect[is]);
	for(i = 1 ; i < endlin ; i++) {
		if((k = developed[i]) >= 0) {
			for(j = 0 ; j < rulings[i] ; j++) {
				x = xstart[k][j] - xsect[is];
				y = xend  [k][j] - xsect[is];
				if(x*y < 0.0) {
					t1 = y/(y-x);
					x = yend[k][j]+t1*(ystart[k][j]-yend[k][j]);
					y = zend[k][j]+t1*(zstart[k][j]-zend[k][j]);
					(*move)(x-dx1,y);
					(*draw)(x+dx1,y);
					(*move)(x,y-dx1);
					(*draw)(x,y+dx1);
				}
			}
		}
	}
#endif

	/*	DRAW CURVATURE.  NOTE THAT FIRST PASS THROUGH SETS UP SCALING (SCALING	*/
	/*	RECALCULATED WHEN REDRAW REQUESTED)	*/

ad0404:
	aa = 0.0;
	cc = 1.0;
	(*newlin)();
	seteditranges(xsect[is]);
	xright = 0.8*xmax + 0.2*xmin;
	if(!*first) {	/* vertical line */
		xorcol(4);
		(*move)(xright,zline[0][is]);
		(*draw)(xright,zline[extlin-1][is]);
	}
	xorcol(TEXTCOL);
	gtst = -1.0e+10;
#ifndef STUDENT
	k = 0;	/* tank index */
#endif
	for(j = 1 ; j < endlin ; j++) {
#ifndef STUDENT
		if(j == fl_line1[k]) {
			k++;
			aa = 0.0;
			cc = 1.0;
			continue;
		}
#endif
		if(stsec[j] <= is0 && ensec[j] >= is0) {
			hullpa(is,j,aa,cc,&a,&hb,&c,&hd);
			yl = yline[j][is];
			zl = zline[j][is];
			d = hd + hd;
			b = hb + hb;
			if(offset == 0 && !*first) {
				(*newlin)();
				drawpoint(is,yl+a+hb,zl-c-hd);
				drawpoint(is,yl+0.125*(3.0*a-b),zl-0.125*(3.0*c-d));
				(*newlin)();
				drawpoint(is,yl+0.625*a,zl-0.625*c);
				drawpoint(is,yl,zl);
			}
			top = a * d - b * c;
			(*newlin)();
			for(t1 = 1.0 ; t1 > -0.05 ; t1 -= 0.1) {
				aa1 = a+b*t1;
				cc1 = c+d*t1;
				div = aa1*aa1+cc1*cc1;
				if(div > 1.0e-6) {
					x1 = top / (div*fsqr0(div));
					if(*first) { /* SET SCALING ON FIRST PASS */
						x1 = fabs(x1);
						if(gtst < x1) gtst = x1;
					}
					else {	/* DO PLOT ON LATER PASSES */
						x1 *= scale;
						y1 = zline[j][is]-(c+hd*t1)*t1;
						if(fabs(x1) < 100.0f) (*draw)(xright + x1,y1);
					}
				}
			}
			tranpa(a,hb,c,hd,&aa,&cc);
		}
	}

	if(*first) {
		if(yra != 0.0)
			aa = 0.01 / yra;
		else
			aa = 0.001;
		if(gtst < aa) gtst = aa;
		scale = 0.20 * yra / gtst;
		*first = 0;
		goto ad0404;
	}
}

void drawpoint(int is,REAL y,REAL z)
{
	if(is) {
		y *= cosaz;
	}
	else {
		seteditranges(xsect[0]-y);
		y = cosaz*yline[stemli][1];
	}
	(*draw) (y,z);
}

int nearest_offset(int *sect,int *xm,int *ym)
{
	extern REAL	xgslope,ygslope,xmin,ymin;
	int i,j;
	int nearest = -1;
	REAL dist,least,dx,dy,xx,yy,xs,xc,yc,xnear,ynear;
	GLint viewport[4];
	GLdouble modelview[16];
	GLdouble projection[16];
	GLfloat winY;
	GLdouble posX,posY,posZ;

	/*	Translate window pixel coordinates to measurement units	*/

	setranges(xmn1,xmx1,ymn1,ymx1);
	if(scrdev == 0) {
		glGetDoublev( GL_MODELVIEW_MATRIX, modelview );
		glGetDoublev( GL_PROJECTION_MATRIX, projection );
		glGetIntegerv( GL_VIEWPORT, viewport );
		winY = (float) (viewport[3] - *ym);
		gluUnProject((GLfloat) *xm, winY,(GLfloat) 1.0, modelview, projection, viewport, &posX, &posY, &posZ);
		xx = posX;
		yy = posY;
	} else {
		xx = xmin + ((float) *xm + 0.5) / xgslope;
		yy = ymax - ((float) *ym + 0.5) / ygslope;
	}
	least = 1.0e+30;

	for(j = 0 ; j < extlin ; j++) {
		for(i = surfacemode ; i < count ; i++) {
			if(editsectnum[i] && i >= stsec[j] && i <= ensec[j]) {
				xc = yline[j][i];
				yc = zline[j][i];
				if(i) {
					xc = xc * cosaz + xsect[i] * sinaz;
					yc = yc + xsect[i] * sinel;
				}
				else {
					xs = xsect[i]-xc;
					xc = yline[stemli][1]*cosaz + xs*sinaz;
					yc = yc + xs * sinel;
				}
				dx = xc - xx;
				dy = yc - yy;
				dist = dx * dx + dy * dy;
				if(dist < least) {
					nearest = j;
					least = dist;
					*sect = i;
					xnear = xc;
					ynear = yc;
				}
			}
		}
	}
	if(!v7mode && !master[*sect] && stsec[nearest] != *sect && ensec[nearest] != *sect) *sect = -*sect;

	/*	Returned coordinates are window-relative	*/

	if(nearest >= 0) {
		if(scrdev == 0) {
			gluProject((GLfloat) xnear,(GLfloat) ynear,(GLfloat) 1.0, modelview, projection, viewport,&posX, &posY, &posZ);
			*xm = posX;
			*ym = viewport[3] - posY;
		} else {
			*xm = (xnear-xmin)*xgslope;
			*ym = (ymax-ynear)*ygslope;
		}
	}
	return(nearest);
}

int nearest_control(int *sect,int *xm,int *ym)
{
	extern	REAL	xmin,xgslope,ymin,ygslope;
	int j;
	int nearest = -1;
	REAL dist,least,dx,dy,xx,yy,xs,xnear,ynear;
	REAL xc,yc;
	REAL aa,cc,a,hb,c,hd;
	int i;
	GLint viewport[4];
	GLdouble modelview[16];
	GLdouble projection[16];
	GLfloat winY;
	GLdouble posX,posY,posZ;

	setranges(xmn1,xmx1,ymn1,ymx1);
	if(scrdev == 0) {
		glGetDoublev( GL_MODELVIEW_MATRIX, modelview );
		glGetDoublev( GL_PROJECTION_MATRIX, projection );
		glGetIntegerv( GL_VIEWPORT, viewport );
		winY = (float) (viewport[3] - *ym);
		gluUnProject((GLfloat) *xm, winY,(GLfloat) 1.0, modelview, projection, viewport, &posX, &posY, &posZ);
		xx = posX;
		yy = posY;
	} else {
		xx = xmin + ((float) *xm + 0.5) / xgslope;
		yy = ymax - ((float) *ym + 0.5) / ygslope;
	}
	least = 1.0e+30;

	for(i = 0 ; i < count ; i++) {
		aa = 0.0;
		cc = 1.0;
		for(j = 1 ; j < extlin ; j++) {
			if(editsectnum[i] && i >= stsec[j] && i <= ensec[j]) {
				hullpa(i,j,aa,cc,&a,&hb,&c,&hd);
				if(hb != 0.0)
					xc = yline[j][i] + 0.5f * a;
				else
					xc = yline[j-1][i];
				if(hd != 0.0)
					yc = zline[j][i] - 0.5f * c;
				else
					yc = zline[j-1][i];
				if(surfacemode || i) {
					xc = xc * cosaz + xsect[i] * sinaz;
					yc = yc + xsect[i] * sinel;
				}
				else {
					xs = xsect[i] - xc;
					xc = yline[stemli][1]*cosaz + xs*sinaz;
					yc = yc + xs * sinel;
				}
				dx = xc - xx;
				dy = yc - yy;
				dist = dx*dx + dy*dy;
				if(dist < least) {
					nearest = j;
					*sect = i;
					least = dist;
					xnear = xc;
					ynear = yc;
				}
				tranpa(a,hb,c,hd,&aa,&cc);
			}
		}
	}
	if(!v7mode && !master[*sect] && stsec[nearest] != *sect &&
		    ensec[nearest] != *sect) *sect = -*sect;

	if(nearest >= 0) {
		if(scrdev == 0) {
			gluProject((GLfloat) xnear,(GLfloat) ynear,(GLfloat) 1.0, modelview, projection, viewport,
					&posX, &posY, &posZ);
			*xm = posX;
			*ym = viewport[3] - posY;
		} else {
			*xm = (xnear-xmin)*xgslope;
			*ym = (ymax-ynear)*ygslope;
		}
	}
	return(nearest);
}

clock_t lastshow = 0;

void showvalues(REAL y,REAL z)
{
	clock_t now = clock();
	char s[12];
	extern COLORREF scrcolour[];
	if(now - lastshow > CLK_TCK/5) {
#ifdef linux

#else
		sprintf(s,"Y%8.4f",y);
		pstrbkg(s,1,0,0,2);
		if(z > -1.0e+30) {
			sprintf(s,"Z%8.4f",z);
			pstrbkg(s,1,0,0,3);
		}
		lastshow = now;
#endif
	}
}

void showincr(REAL del)
{
	char s[16];
	sprintf(s,"DEL =%8.4f",del);
	pstrbkg(s,1,0,0,1);
}

void pstrbkg(char *s,int col,int bkg,int x,int y)
{
	extern COLORREF scrcolour[];
	extern int xchar,ychar;
	GLfloat x1,y1;
	extern REAL xcurr,ycurr;
	int c,r,g,b;
#ifdef linux
	extern XmFontList fontlist;
	XmString xstr;
	Dimension textw,texth;
#else
	int textw = strlen(s)*xchar;
	int texth = ychar;
	void GLst(char *);
#endif

	x *= xchar;
	if(scrdev == 1)	{	// GDI
#ifdef linux
		xstr = XmStringCreateSimple(s);
		XmStringExtent(fontlist,xstr,&textw,&texth);
		XmStringFree(xstr);
		y = y*texth + 20;
		x += 4;
		XSetFunction(display,gc,GXcopy);
		XSetForeground(display,gc,scrcolour[bkg]);
		XFillRectangle(display,win,gc,x,y-texth,textw,texth);
		XSetForeground(display,gc,scrcolour[col]);
		XDrawString(display,win,gc,x,y,s,strlen(s));
		XSetFunction(display,gc,GXxor);
#else
		y *= ychar;
		SetBkMode(hDC,OPAQUE);
		SetBkColor(hDC,scrcolour[bkg]);
		SetTextColor(hDC,scrcolour[col]);
		TextOut(hDC,x,y,s,strlen(s));
#endif
	} else {			// GL
		x1 = xmin+fabs(x/xgslope);
		y1 = ymax+fabs(y/ygslope);
		c = scrcolour[1];						// text colour
		r = ((float) GetRValue(c)) / 255.0;
		g = ((float) GetGValue(c)) / 255.0;
		b = ((float) GetBValue(c)) / 255.0;
		glColor3f(r,g,b);
		glRasterPos3f(x1,y1,0.0);
		glPushMatrix();
		glCallLists(strlen(s), GL_UNSIGNED_BYTE,s);
		glPopMatrix();
	}
}

/*	SUBROUTINE MOVES A HULL SECTION POINT ACCORDING TO "FOLLOW"	*/
/*	CRITERION.							*/

/*	IS	IS SECTION NUMBER					*/
/*	editline IS LINE NUMBER OF POINT BEING MOVED			*/
/*	IX1	IS THE NUMBER OF THE LINE FORMING THE BASE OF THE HULL CURVE	*/
/*		BEING FOLLOWED (MAY BE editline, OR editline+1)		*/
/*	TVAL	IS CURVE PARAMETER AT POINT BEING "FOLLOW"ED TO.	*/
/*		tval will be close to 1 when the curve below is		*/
/*		being followed and close to 0 if the curve above is	*/
/*		being followed.						*/

void movpoi(INT is,INT editline,INT ix1,REAL tval)
{
	REAL a,hb,c,hd,a0,hb0,c0,hd0;
	REAL del1,del2,ycnew,zcnew;

	/*	FIND HULL CURVE PARAMETERS	*/

	getparam(is,ix1,&a,&hb,&c,&hd);

	/*		FIND COORDINATES OF HULL CURVE HERE	*/

	del1 = tval*(a+tval*hb);	/* lateral distance to new location */
	del2 = tval*(c+tval*hd);	/* vertical distance down to new location */

	/*		HENCE THE NEW Y AND Z VALUES	*/

	setall(yline,is,editline,yline[ix1][is]+del1);
	setall(zline,is,editline,zline[ix1][is]-del2);
	if(editline < extlin-1 && relcont[editline]) zcont[editline+1][is] -= del2;

	/*		NEW CONTROL POINTS		*/

	if(ix1 == editline) {	/* following curve above	*/
		if(!relcont[ix1]) {
			if(ix1 > 0) ycont[ix1][is] = tval*yline[ix1-1][is] + (1.0-tval)*ycont[ix1][is];
		}
		else {
			ycont[ix1][is] *= (1.0-tval);
		}
		if(ix1 > 0) zcont[ix1][is] = tval*zline[ix1-1][is] + (1.0-tval)*zcont[ix1][is];
	}
	else {			/* following curve below	*/
		zcont[ix1][is] = zcnew = zline[ix1][is] - 0.5*c*tval;
		ycnew = yline[ix1][is] + 0.5*a*tval;
		if(!relcont[ix1]) {
			ycont[ix1][is] = ycnew;
		}
		else {
			getparam(is,editline,&a0,&hb0,&c0,&hd0);
			if(c0 != 0.0) ycont[ix1][is] = ycnew - (yline[editline][is] - a0/c0*(zcnew - zline[editline][is]));
		}
	}
}

/*	SET ALL VALUES BELOW A LINE THE SAME, WHERE THEY WERE THE SAME BEFORE,
AND ADJUST CONTROL POINTS ABOVE AND BELOW	*/

void setall(REAL (far *line)[maxsec+4],INT ns,INT nl,REAL value)
{
	int	j;
	int vert = (line == zline);

	for(j = extlin - 1 ; j > nl ; j--) {
		if((stsec[j] <= ns && ensec[j] >= ns) || ns > maxsec-2) {
			if(yline[j][ns] == yline[nl][ns] && zline[j][ns] == zline[nl][ns]) {
				line[j][ns] = value;
				if(vert) zcont[j][ns] = value;
			}
		}
	}
	line[nl][ns] = value;
}

/*	This fix is applied BEFORE the z-offset is changed to "value"	*/

void fix_vertical_controls(int i,int j,REAL value)
{
	REAL dzl,fr;
	if(j > 0 && relcont[j]) {
		dzl = zline[j-1][i]-zline[j][i];
		if(dzl != 0.0) {
			fr = (zcont[j][i]-zline[j][i])/dzl;
			if(fr > 1.0)
				fr = 1.0;
			else if(fr < 0.0)
				fr = 0.0;
			zcont[j][i] = value + fr*(zline[j-1][i]-value);
		}
	}
	if(j < extlin-1 && relcont[j+1]) {
		dzl = zline[j][i]-zline[j+1][i];
		if(dzl != 0.0) {
			fr = (zcont[j+1][i]-zline[j+1][i])/dzl;
			if(fr > 1.0)
				fr = 1.0;
			else if(fr < 0.0)
				fr = 0.0;
			zcont[j+1][i] = zline[j+1][i] + fr*(value-zline[j+1][i]);
		}
	}

}

void xorcol(int col)
{
	if(scrdev == 0)
		setcolour(scrcolour[col],0,0);
	else
		setcolour(scrcolour[col] ^ scrcolour[0],0,0);
}

void saveundo(void)
{
	int i;
	if(undoindex >= MAXEDITUNDO-1) {
		for(i = 0 ; i < undoindex ; i++) {
			yundo[i] = yundo[i+1];
			zundo[i] = zundo[i+1];
			undoline[i] = undoline[i+1];
			undosect[i] = undosect[i+1];
			editmode[i] = editmode[i+1];
		}
		undoindex--;
	}
	if(offset == 1) {
		yundo[undoindex] = yline[editline][editsection];
		zundo[undoindex] = zline[editline][editsection];
	}
	else {
		yundo[undoindex] = ycont[editline][editsection];
		zundo[undoindex] = zcont[editline][editsection];
	}
	undoline[undoindex] = (short) editline;
	undosect[undoindex] = (char) editsection;
	editmode[undoindex] = (char) offset;
	undoindex++;
}

void seteditranges(REAL x)
{
	REAL dx = sinaz*x;
	REAL dy = sinel*x;
	if(xmn1 != xmx1 && ymn1 != ymx1) setranges(xmn1-dx,xmx1-dx,ymn1-dy,ymx1-dy);
}

void drawsec(int editsection)
{

#ifndef STUDENT
	tankcol = 1;
#endif
	if(surfacemode || editsection) {
		seteditranges(xsect[editsection]);
		drasec(editsection,0,cosaz,0);
	}
	else {	/* section 0 */
		seteditranges(xsect[0]);
		drasec(editsection,0,1.0,11);
	}
#ifdef EXT_OR_PROF
	if(showoverlay) {
		use_hull(OVERLAYHULL);
		if(surfacemode || editsection) {
			seteditranges(xsect[editsection]);
			drasec(editsection,0,cosaz,0);
		}
		else {	/* section 0 */
			seteditranges(xsect[0]);
			drasec(editsection,0,1.0,11);
		}
		use_hull(MAINHULL);
	}
#endif
}

/*	This routine warns the user about negative line offsets,
and now also sets lateral control displacements to zero
when the relevant lines are at the same vertical offset
*/

void check_negative_offsets(void)
{
	extern int offsetnowarning;
	static int fixit = TRUE;
	int i,j,i1;
	char where[40];
	int save_id = context_id;
	REAL x1;
	static int webnowarning = FALSE;

	if(surfacemode) return;

	for(j = 0 ; j < numlin ; j++) {
		i1 = max(1,stsec[j]);
		for(i = i1 ; i <= ensec[j] ; i++) {
			if(yline[j][i] < 0.0) {
				context_id = 2058;
				sprintf(where,"line %d, section %d",j+1,i);
				if(offsetnowarning || getdlg(OFFSWARN,
					0,(void *) where,
					INP_LOG,(void *) &offsetnowarning,
					INP_LOG,(void *) &fixit,-1) && fixit) yline[j][i] = 0.0;
				context_id = save_id;
				return;
			}
			else if(j < numlin-1 &&
				    i1 < i && stsec[j+1] < i && ensec[j+1] >= i &&
				    yline[j][i]   == 0.0 && yline[j+1][i]   == 0.0 &&
				    yline[j][i-1] == 0.0 && yline[j+1][i-1] == 0.0 &&
				    (zline[j][i] != zline[j+1][i] || zline[j][i-1] != zline[j+1][i-1]) &&
				    !webnowarning) {
				context_id = 2059;
				(void) getdlg(WEBWARN,INP_LOG,(void *) &webinclude,-1);
				webnowarning = TRUE;
			}
		}
	}

	x1 = xsect[0] - xsect[max(1,count-3)];
	/* maximum negative excursion of section 0 */
	for(j = 0 ; j < extlin ; j++) {
		if(yline[j][0] < x1) {
			message("An offset on the stem section was placed too far sternward. It has been relocated.");
			yline[j][0] = x1 + 0.0001;
		}
	}
	context_id = save_id;
}

/*	Fair all four hull line parameters
*/
void fairlines(int editline)
{
	REAL a,hb,c,hd,xst,yst;
#ifndef STUDENT
	REAL s,t0,t1;
#endif
	int i,j,jp;
	int st,en,ist,ien,masterst,masteren;
	REAL yc[maxsec+2],yc0[maxsec+2];
	REAL cwt[maxsec];

	if(!autofair[editline]) return;

	st = stsec[editline];
	if(st < surfacemode) st = surfacemode;
	en = ensec[editline];
	ist = st;
	ien = en;
	masterst = master[ist];
	masteren = master[ien];
	master[ist] = TRUE;
	master[ien] = TRUE;
	if(surfacemode) master[0] = FALSE;

	/*	Fair offsets		*/

	xst = xsect[0];
	if(!surfacemode) xst -= yline[editline][0];

#ifndef STUDENT
	get_int_angle(editline,&c,&s);
	if(yline[editline][st] < yline[stemli][st]) s = -s;
	t0 = radstem[editline];
	if(s != 0.0) t0 *= (1.0-c)/s;
	yst = yline[stemli][1] + t0;
#else
	yst = yline[stemli][1];
#endif

	/*	The last two arguments are not used if st > 0	*/

	fairline(yline[editline],linewt[editline],st,en,xst,yst);
	fairline(zline[editline],linewt[editline],st,en,xst,zline[editline][0]);

	/*	Fair control points				*/

	if(editline > 0) {
		for(jp = editline-1 ; jp > 0 ; jp--) if(stsec[jp] <= st) break;

#ifndef STUDENT
		get_int_angle(jp,&c,&s);
		if(yline[jp][ist+1] < yline[stemli][ist+1]) s = -s;
		t0 = radstem[jp];
		if(s != 0.0) t0 *= (1.0-c)/s;
		get_int_angle(editline,&c,&s);
		if(yline[editline][ist+1] < yline[stemli][ist+1]) s = -s;
		t1 = radstem[editline];
		if(s != 0.0) t1 *= (1.0-c)/s;
		a = zline[editline][ist]-zline[jp][ist];
		yst = yline[stemli][1] + t0;
		if(a != 0.0) yst += (t1-t0)*(zcont[editline][ist]-zline[jp][ist])/a;
#else
		yst = yline[stemli][1];
#endif

		for(i = st ; i <= en ; i++) {
			if(editline > 0) {
				for(j = editline-1 ; j > 0 ; j--) if(stsec[j] <= st) break;
				a = zline[editline][i] - zline[j][i];
				if(a != 0.0) a = (zline[editline][i] - zcont[editline][i]) / a;
				cwt[i] = linewt[editline][i]*(1.0-a) + linewt[j][i]*a;
			}
			else {
				cwt[i] = linewt[editline][i];
			}
		}

		if(relcont[editline]) {
			getparam(0,editline,&a,&hb,&c,&hd);
			xst -= 0.5*a;
			for(i = ist ; i <= ien ; i++) {
				getparam(i,editline,&a,&hb,&c,&hd);
				yc0[i] = yc[i] = yline[editline][i]+0.5*a;
			}
			yc[maxsec]   = ycont[editline][maxsec];
			yc[maxsec+1] = ycont[editline][maxsec+1];
			fairline(yc,cwt,st,en,xst,yst);
			for(i = st ; i <= en ; i++) ycont[editline][i] += yc[i]-yc0[i];
		}
		else {
			if(stsec[jp] < st) st--;
			if(ensec[jp] > en) en++;
			xst = xsect[st] - ycont[editline][st];
			fairline(ycont[editline],cwt,st,en,xst,yst);
		}
		fairline(zcont[editline],linewt[editline],st,en,xst,zcont[editline][0]);
	}
	master[ist] = masterst;
	master[ien] = masteren;
}

/*	Fair any one hull line parameter
*/
void fairline(REAL *offs,REAL wt[],int st,int en,REAL xst,REAL yst)
{
	int i,n;
	REAL tempx[maxsec],tempy[maxsec],newoff[maxsec];
	REAL fwt[maxsec];

	n = 0;
	if(st <= surfacemode) st = surfacemode;
	i = st;
	if(st <= 0) {
		n++;
		i++;
		tempx[0] = xst;
		tempy[0] = yst;
		fwt[0] = wt[0];
	}

	while(i <= en) {
		if(v7mode || master[i]) {
			fwt[n] = wt[i];
			tempx[n] = xsect[i];
			tempy[n++] = offs[i];
		}
		i++;
	}
	if(n >= 2) spline(tempx,tempy,fwt,n,
		&xsect[st+1],&newoff[st+1],en-st,
		offs[maxsec],offs[maxsec+1]);
	for(i = st+1 ; i <= en ; i++) {
		if(v7mode || !master[i]) offs[i] = newoff[i];
	}
}

void linefunc(int,HWND);
void get_weights(int);
void straighten_curve(int);
int wtline;

void line_properties(int editline)
{
	char text[120];
	int i;
	REAL a,hb,c,hd,aa,cc;
	int rel = relcont[editline];
	char masterstring[500];

	wtline = editline;

	listseq(master,masterstring,count);
	sprintf(text,"Line number %d",editline+1);
	if(getdlg(LINEPROP,
		0,text,
#ifdef PROF
		INP_INT,(void *) &stsec[editline],
		INP_INT,(void *) &ensec[editline],
#else
		INP_INT,(void *) NULL,
		INP_INT,(void *) NULL,
#endif
		INP_RBN,(void *) &rel,
		INP_LOG,(void *) &autofair[editline],
		INP_REA,(void *) &yline[editline][maxsec],
		INP_REA,(void *) &yline[editline][maxsec+1],
		INP_REA,(void *) &zline[editline][maxsec],
		INP_REA,(void *) &zline[editline][maxsec+1],
		INP_REA,(void *) &ycont[editline][maxsec],
		INP_REA,(void *) &ycont[editline][maxsec+1],
		INP_REA,(void *) &zcont[editline][maxsec],
		INP_REA,(void *) &zcont[editline][maxsec+1],
		INP_STR,(void *) masterstring,
		INP_PBF,linefunc,-1)) {
		multproc(masterstring,master,count);
		if(editline > 0 && relcont[editline] != rel) {
			sprintf(text,"Are you sure you want to make line %d's\nlateral control offsets %s",editline+1, rel ? "relative" : "absolute");
			if(MessageBox(hWnd,text,"Changing Line Property",
				MB_ICONQUESTION | MB_YESNO) == IDYES) {
				for(i = stsec[editline] ; i <= ensec[editline] ; i++) {
					if(rel) {	/* make relative */
						getparam(i,editline-1,&a,&hb,&c,&hd);
						tranpa(a,hb,c,hd,&aa,&cc);
						if(cc != 0.0) {
							c = yline[editline-1][i] - aa/cc*(zcont[editline][i] - zline[editline-1][i]);
							ycont[editline][i] -= c;
						}
					}
					else {	/* make absolute */
						getparam(i,editline,&a,&hb,&c,&hd);
						ycont[editline][i] = yline[editline][i] + 0.5*a;
					}
				}
				relcont[editline] = rel;
			}
		}
	}
}

void linefunc(int code,HWND hWndDlg)
{
	if(code == 0)
		get_weights(wtline);
	else
	    straighten_curve(wtline);
}

void straighten_curve(int wtline)
{
	int i,j;
	char text[100];
	extern HWND hWndMain;

	if(wtline <= 0) {
		message("This operation has no effect on line 1");
		return;
	}

	sprintf(text,"This operation will remove all curvature between\nline %d and %d. Is that what you want?",wtline,wtline+1);
	if(MessageBox(hWndMain,text,"WARNING",MB_YESNO) == IDNO) return;

	for(i = stsec[wtline] ; i <= ensec[wtline] ; i++) {
		for(j = wtline-1 ; j >= 0 ; j--) {
			if(stsec[j] <= i && ensec[j] >= i) break;
		}
		if(j >= 0) {
			zcont[wtline][i] = zline[j][i];
			if(relcont[wtline])
				ycont[wtline][i] = 0.0;
			else
			    ycont[wtline][i] = yline[j][i];
		}
	}
}

#ifdef linux

typedef struct tagRGBQUAD { // rgbq
    BYTE    rgbBlue;
    BYTE    rgbGreen;
    BYTE    rgbRed;
    BYTE    rgbReserved;
} RGBQUAD;


typedef struct tagBITMAPINFOHEADER{ // bmih
   DWORD  biSize;
   LONG   biWidth;
   LONG   biHeight;
   WORD   biPlanes;
   WORD   biBitCount;
   DWORD  biCompression;
   DWORD  biSizeImage;
   LONG   biXPelsPerMeter;
   LONG   biYPelsPerMeter;
   DWORD  biClrUsed;
   DWORD  biClrImportant;
} BITMAPINFOHEADER;

typedef struct tagBITMAPFILEHEADER { // bmfh
        WORD    bfType;
        DWORD   bfSize;
        WORD    bfReserved1;
        WORD    bfReserved2;
        DWORD   bfOffBits;
} BITMAPFILEHEADER;

typedef struct tagBITMAPINFO { // bmi
   BITMAPINFOHEADER bmiHeader;
   RGBQUAD          bmiColors[1];
} BITMAPINFO;

#endif

HBITMAP LoadBMP(char *file,int *w,int *h)
{
	HBITMAP hBMP;
#ifndef linux
	BITMAPFILEHEADER bmHeader;
	BITMAPINFO bmInfo;
	int fd;
	int bmpsize;
	HGLOBAL hData;
	int n;
	char *data;
#else
	int xhot,yhot;
	Pixmap data;
#endif

#ifdef linux
	if(XReadBitmapFile(display,win,file,w,h,&data,&xhot,&yhot) != None) {
		hBMP = (HBITMAP) data;
	} else {
		hBMP = NULL;
	}
#else
	if( (fd = open(file,O_RDONLY | O_BINARY)) > 0 &&
	read(fd,&bmHeader,sizeof(bmHeader)) == sizeof(BITMAPFILEHEADER) ) {
		bmpsize = bmHeader.bfSize - bmHeader.bfOffBits;
		hData = GlobalAlloc(GMEM_MOVEABLE,bmpsize);
		data = GlobalLock(hData);
		if(read(fd,(char *) &bmInfo,sizeof(bmInfo)) == sizeof(bmInfo)) {
			*w = bmInfo.bmiHeader.biWidth;
			*h = bmInfo.bmiHeader.biHeight;
		}
		lseek(fd,bmHeader.bfOffBits,0);
		if( (n = read(fd,data,bmpsize)) == bmpsize) {
			hBMP = CreateDIBitmap(hDC,&(bmInfo.bmiHeader),CBM_INIT,data,&bmInfo,0);
		} else {
			hBMP = NULL;
		}
		close(fd);
		GlobalUnlock(hData);
		GlobalFree(hData);
	} else {
		hBMP = NULL;
	}
#endif
	return hBMP;
}

