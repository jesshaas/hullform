/* Hullform component - graf_hfw.c
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
#include "graf-hfw.h"

#ifdef linux
#include <GL/gl.h>
#include <GL/glu.h>
#else
#include <GL\gl.h>
#include <GL\glu.h>
#endif

#include <time.h>
#include <string.h>
#include <ctype.h>

extern	HDC	hDC;
#ifndef linux
PAINTSTRUCT ps;
#endif
void GDIdr(REAL x,REAL y);
extern int xleft,xright,ytop,ybottom;
extern int xwleft,xwright,ywtop,ywbottom;
extern int context_id;
void check_pointers(void);
void check_pointer(char *name,FUNC_PTR ff[]);
void GetSizes(HWND);
extern HWND hWndMain;
int winstyle[] = {
	PS_SOLID,PS_DASH,PS_DOT,PS_DASHDOT,PS_DASHDOTDOT};
void CreateScrollBars(void);
void DestroyscrollBars(void);
void setcolour(COLORREF colour,int style,int width);
extern void xorcol(int col);

void GLzoom(REAL,REAL,REAL,REAL);
void SwapBuf(void);

/*	graphics size tables				*/

/* NONE,SYSTEMPRINTER,HEWLETT-PACKARD,TEKTRONIX,REGIS,DXF,METAFILE. "-1" MEANS "ASK WINDOWS" */

#ifdef STUDENT
int xgsize[] = {
	-1,-1,16000};
int ygsize[] = {
	-1,-1,12000};
int ycsize[] = {
	-1,-1,160};
#else
int xgsize[] = {
	-1,-1,7200,4096,540,1000,4608,16000};
int ygsize[] = {
	-1,-1,7200,3072,480,1000,3461,12000};
int ycsize[] = {
	-1,-1,320,100,16,32,92,160};
#endif

/************************************************************************/
/*									*/
/*	Windows graphics drivers					*/
/*									*/
/************************************************************************/

void GDIin()
{
	extern	LOGFONT lf;	/* logical font structure */
	void setfont(LOGFONT *lf);
	if(!hardcopy) {
		hDC = (HDC) GetDC(hWnd);
		setup(1);	/* GDI is device 1 */
		setfont(&lf);
#ifdef linux

#else
		SetROP2(hDC,R2_COPYPEN);
#endif
	}
	GDIcr(0);
}

void GDIen()
{
	HPEN hPen;
	HBRUSH hBrush;
	HGDIOBJ hObj;

	if(!hardcopy) {
		hObj = (HGDIOBJ) GetStockObject(BLACK_BRUSH);
		hBrush = (HBRUSH) SelectObject(hDC,hObj);
		if(hBrush != NULL) DeleteObject(hBrush);
		DeleteObject(hObj);

		hObj = (HGDIOBJ) GetStockObject(BLACK_PEN);
		hPen = (HPEN) SelectObject(hDC,hObj);
		if(hPen != NULL) DeleteObject(hPen);
		DeleteObject(hObj);

		ReleaseDC(hWnd,hDC);
		hDC = NULL;
	}
}

void GDInl()
{
	extern	INT	penup;
	penup = 1;
}

void GDIcl()
{
	RECT rc;
	extern HBRUSH bg;
	HBRUSH locbg;
	extern int repaint;
	extern int scrollable;

	GetClientRect(hWnd,&rc);
	if(hardcopy) {	/* ... always use white background */
		locbg = (HBRUSH) GetStockObject(WHITE_BRUSH);
		FillRect(hDC,&rc,locbg);
		DeleteObject(locbg);
	}
	else {
		FillRect(hDC,&rc,bg);
	}
}

void GDIcr(INT ncol)
{
	extern COLORREF scrcolour[8];
	extern int linestyle[5];
	extern int linewidth[5];
	int icol = max(0,min(4,ncol-2));
#ifdef EXT_OR_PROF
	extern int current_hull;
	if(current_hull != 0) return;	/* one colour for overlay */
#endif

	setcolour(scrcolour[ncol],linestyle[icol],linewidth[icol]);
}

void setcolour(COLORREF colour,int style,int width)
{
	HPEN hPen;
	HBRUSH hBrush;
	float r,g,b;

	if(scrdev == 1) {
		hPen = (HPEN) SelectObject(hDC,CreatePen(winstyle[style],width,colour));
		if(hPen != NULL) DeleteObject(hPen);
		hBrush = (HBRUSH) SelectObject(hDC,CreateSolidBrush(colour));
		if(hBrush != NULL) DeleteObject(hBrush);
	} else {
		r = ((float) GetRValue(colour)) / 255.0;
		g = ((float) GetGValue(colour)) / 255.0;
		b = ((float) GetBValue(colour)) / 255.0;
		glColor3f(r,g,b);
	}
}

void GDImv(REAL x,REAL y)
{
	extern	INT	penup;

	penup = 1;
	GDIdr(x,y);
}

void GDIdr(REAL x1, REAL y1)
{
	extern REAL	xcurr,ycurr;
	REAL x,y;
	extern int	penup;
	extern int	ymaxi,xscr,yscr;

	x = x1;
	y = y1;
	trans(&x,&y);
	if( (x >= -30000.0) && (x < 30000.0) && (y >= -30000.0) && (y <= 30000.0) ) {
		xcurr = x;
		ycurr = y;
		xscr = (int) x;
		yscr = ymaxi - (int) y;

		if(!penup)
			LineTo(hDC,xscr,yscr);
		else
		    MoveToEx(hDC,xscr,yscr,NULL);
		penup = 0;
	}
	else {
		penup = 1;
	}
}

void GDIcircle(REAL xc,REAL yc,REAL rad)
{
	REAL xl = xc - rad;
	REAL xr = xc + rad;
	REAL yb = yc + rad;
	REAL yt = yc - rad;
	trans(&xl,&yb);
	trans(&xr,&yt);
	Ellipse(hDC,(int) xl,(int) yt,(int) xr,(int) yb);
}

void GDIst(char *s)
{
	extern int xscr,yscr;
	extern COLORREF scrcolour[];
	extern int xmaxi,ymaxi;

#ifdef linux

#else
	SetBkMode(hDC,TRANSPARENT);
	SetTextColor(hDC,scrcolour[1]);
#endif
	if(xscr >= 0 && xscr < xmaxi && yscr >= 0 && yscr < ymaxi)
		TextOut(hDC,xscr,yscr,s,strlen(s));
}

/*****	graphics function tables			*****/

/*	NONE,SYSTEMPRINTER,HPGL,TEKTRONIX,REGIS,DXFFILE	*/

/*
#define GLmv nullxy
#define GLdr nullxy
#define GLin null
#define GLen null
#define GLcl null
#define GLcr nulli
#define GLnl null
#define GLst nulls
*/

extern void null(),nullxy(REAL x,REAL y),nulli(int i),nulls(char *s);
extern void GLmv(REAL x,REAL y),GDImv(REAL x,REAL y),hewlmv(REAL x,REAL y),
tektmv(REAL x,REAL y),regimv(REAL x,REAL y),adxfmv(REAL x,REAL y);
extern void GLdr(REAL x,REAL y),GDIdr(REAL x,REAL y),hewldr(REAL x,REAL y),
TEKTdr(REAL x,REAL y),regidr(REAL x,REAL y),adxfdr(REAL x,REAL y);
extern void GLin(),GDIin(),hewlin(),tektin(),regiin(),adxfin(),metain();
extern void GLen(),GDIen(),hewlen(),tekten(),regien(),adxfen(),null();
extern void GLcl(),GDIcl(),hewlcl(),tektcl(),regicl(),adxfcl();
extern void GLcr(int),GDIcr(int),hewlcr(int),tektcr(int),regicr(int),adxfcr(int);
extern void GLnl(),GDInl(),hewlnl(),tektnl(),reginl(),adxfnl();
extern void GLst(char *s),GDIst(char *s),hewlst(char *s),tektst(char *s),
regist(char *s),adxfst(char *s);

#ifdef EXT_OR_PROF
REALFUNC_PTR movep[] = {
	GLmv,GDImv,hewlmv,tektmv,regimv,adxfmv,wmfmv,GDImv,NULL};
REALFUNC_PTR move;

REALFUNC_PTR drawp[] = {
	GLdr,GDIdr,hewldr,tektdr,regidr,adxfdr,wmfdr,GDIdr,NULL};
REALFUNC_PTR draw;

FUNC_PTR initp[] = {
	GLin,GDIin,hewlin,tektin,regiin,adxfin,wmfin,null,NULL};
FUNC_PTR init;

FUNC_PTR endgrfp[] = {
	GLen,GDIen,hewlen,tekten,regien,adxfen,wmfen,null,NULL};
FUNC_PTR endgrf;

FUNC_PTR clrgrfp[] = {
	GLcl,GDIcl,hewlcl,tektcl,regicl,adxfcl,wmfcl,GDIcl,NULL};
FUNC_PTR clrgrf;

INTFUNC_PTR colourp[] = {
	GLcr,GDIcr,hewlcr,tektcr,regicr,adxfcr,wmfcr,GDIcr,NULL};
INTFUNC_PTR colour;

FUNC_PTR newlinp[] = {
	GLnl,GDInl,hewlnl,tektnl,reginl,adxfnl,wmfnl,GDInl,NULL};
FUNC_PTR newlin;

STRFUNC_PTR plstrp[] = {
	GLst,GDIst,hewlst,tektst,regist,adxfst,wmfst,GDIst,NULL};
STRFUNC_PTR plstr;
#else
REALFUNC_PTR movep[] = {
	GLmv,GDImv,GDImv};
REALFUNC_PTR move;
REALFUNC_PTR drawp[] = {
	GLdr,GDIdr,GDIdr};
REALFUNC_PTR draw;
FUNC_PTR initp[] = {
	GLin,GDIin,null};
FUNC_PTR init;
FUNC_PTR endgrfp[] = {
	GLen,GDIen,null};
FUNC_PTR endgrf;
FUNC_PTR clrgrfp[] = {
	GLcl,GDIcl,GDIcl};
FUNC_PTR clrgrf;
INTFUNC_PTR colourp[] = {
	GLcr,GDIcr,GDIcr};
INTFUNC_PTR colour;
FUNC_PTR newlinp[] = {
	GLnl,GDInl,GDInl};
FUNC_PTR newlin;
STRFUNC_PTR plstrp[] = {
	GLst,GDIst,GDIst};
STRFUNC_PTR plstr;
#endif

/*	Once the graphics device is defined, the correct function is
set up using statements of form

*move = movep[device];

and then invoking the relevant subroutine directly using, e.g.,

(*move)(x,y);
*/

/********************************************************/

/*	Summary of hull to which output applies		*/

void hcpy_summ()
{
#ifdef DEMO
	return;
#else
	extern REAL xmin,xmax,ymin,ymax;
	REAL x,y,dy;
	extern REAL xgorigin,xgslope,ygorigin,ygslope;
	time_t secs;
	extern int persp;
	char *t;
	char line[MAX_PATH];
	extern int hcpydev;
	extern int ychar,ymaxi;

	if(hcpydev == 6) return;	/* not for Windows metafile */

	persp = 0;
	(*colour)(2);
	dy = 1.2 * (REAL) ychar / (REAL) ymaxi * (ymax - ymin);
	y = ymax - dy;
	x = 0.98 * xmin + 0.02 * xmax;
	(*move)(x,y);
	(*plstr)(hullfile);
	y -= dy;
	(*move)(x,y);
	sprintf(line,"disp %10.3f, xcb %8.3f, zcb %8.3f",disp,xcofb,zcofb);
	(*plstr)(line);
	y -= dy;
	(*move)(x,y);
	time(&secs);
	t = ctime(&secs);
	*(t + strlen(t) - 1) = 0;
	(*plstr)(t);
#endif
}

/*	Text output in fixed-point decimal notation of real number	*/

void plfix(REAL a,INT w,INT d)
{
	char	data[21];
	sprintf(data,"%*.*f",w,d,a);
	(*plstr)(data);
}

/*	Text output of integer	*/

void plint(INT num,INT w)
{
	char	data[20];
	sprintf(data,"%*d",w,num);
	(*plstr)(data);
}

void plong(long num,INT w)
{
	char	data[20];
	sprintf(data,"%*ld",w,num);
	(*plstr)(data);
}

/*	x- and y-transformation to screen coordinates			*/

/*	The returned values of x and y are relative to the lower left-	*/
/*	hand corner of the current drawing box				*/

void trans(REAL *x0,REAL *y0)
{
	extern INT persp;
	extern REAL xpersp,ypersp,zpersp;
	extern REAL axx,axy,axz,ayx,ayy,ayz,azx,azy,azz;
	REAL div;
	extern REAL sinpp,cospp;
	extern REAL xgorigin,ygorigin,xgslope,ygslope;
	extern REAL yview,zview;
	REAL xp,yp,zp,xt,yt,zt,zr;

	/*	perspective projection parameters	*/

	if(persp) {
		xp = xpersp + *x0;
		yp = ypersp + *y0;
		zp = zpersp;
		xt = axx * xp + axy * yp + axz * zp;
		yt = ayx * xp + ayy * yp + ayz * zp;
		zt = azx * xp + azy * yp + azz * zp;
		div = sinpp*(yview-yt) + cospp*(zview-zt);
		if(div != 0.0) {
			zr = (sinpp*yview+cospp*zview) / div;
			*y0 = (zview*yt - yview*zt) / div;
			*x0 = zr*xt;
		}
		else {
			*x0 = xt;
			*y0 = yt;
		}
	}
	*x0 = xgorigin + *x0 * xgslope;
	*y0 = ygorigin + *y0 * ygslope;
}

/*	Full plotter device initialisation	*/

int init_device(int devnum)
{
	extern	int	default_hardcopy_device;
	extern	int	hcpydev;
#ifdef EXT_OR_PROF
	int ndev;
	void browse_plot_directory(int code,HWND hWndDlg);
#endif

	if(hardcopy) {
		strcpy(cur_port,port);
#ifdef EXT_OR_PROF
		ndev = devnum - 1;
		if(devnum > 1) {
			if(ndev < 0) {
				message("You have not specified a plotter.\nUse the configure menu to do this");
				return(0);
			}
			else if(getdlg(CONFPORT,
					INP_STR,(void *) cur_port,
					INP_RBN,(void *) &ndev,
					INP_PBF,browse_plot_directory,
					-1)) {
					hcpydev = ndev + 1;
				}
			else {
				return(0);
			}
		}
#else
		hcpydev = devnum;
#endif
	}

#ifndef DEMO
	setup(hcpydev);		/* set general device parameters*/
#endif
	return(1);
}

/*	Set up device addresses and parameters, without screen-clear	*/
/*	etc.								*/

void setup(int devnum)
{
	extern	int	xright,xleft,ybottom,ytop;
	extern	int	xmaxi,ymaxi;
	extern	int	persp;
	extern	REAL	PixelRatio;
	TEXTMETRIC	tm;
	extern	int	xchar,ychar;
	REAL		size;

/*
	if(device == 0 && devnum == 1) {
		message("swap back");
	}
*/
	device = devnum;

#ifdef PROF
	init	= initp[devnum];
	endgrf	= endgrfp[devnum];
	clrgrf	= clrgrfp[devnum];
	colour	= colourp[devnum];
	move	= movep[devnum];
	draw	= drawp[devnum];
	newlin	= newlinp[devnum];
	plstr	= plstrp[devnum];
#else
	init	= initp[devnum];
	endgrf	= endgrfp[devnum];
	clrgrf	= clrgrfp[devnum];
	colour	= colourp[devnum];
	move	= movep[devnum];
	draw	= drawp[devnum];
	newlin	= newlinp[devnum];
	plstr	= plstrp[devnum];
#endif

	if(xgsize[devnum] < 0) {	// retain previous (full device size) - GetSizes may adjust later
		GetTextMetrics(hDC,&tm);
		ychar = tm.tmHeight;
		xchar = tm.tmAveCharWidth;
		xmaxi = GetDeviceCaps(hDC,HORZRES);
		ymaxi = GetDeviceCaps(hDC,VERTRES);
	} else {
		xmaxi = xgsize[devnum];
		ymaxi = ygsize[devnum];
	}
	if(hardcopy) {
		xright = xmaxi;
		xleft = 0;
		ybottom = ymaxi;
		ytop = 0;
	}
	else {
		GetSizes(hWnd);
		size = (REAL) GetDeviceCaps(hDC,LOGPIXELSY);
		if(size != 0.0)
			PixelRatio = (REAL) GetDeviceCaps(hDC,LOGPIXELSX)/
				size;
		else
			PixelRatio = 1.0;
	}
	box((REAL) (xright - xleft),(REAL) (ybottom - ytop));
	persp = 0;
}

void null()
{
	return;		/* empty routine used when no device available */
}

#pragma argsused
void nullxy(REAL x,REAL y)
{
	return;		/* empty routine used when no device available */
}

#pragma argsused
void nulli(int i)
{
	return;		/* empty routine used when no device available */
}

#pragma argsused
void nulls(char *s)
{
	return;		/* empty routine used when no device available */
}

/*	get zoom area	*/

#ifndef STUDENT

int xzleft,xzright,yztop,yzbottom;
int *xsh,*ysh;
int dxzoom,dyzoom;
extern REAL xmin0,xmax0,ymin0,ymax0;
extern HPEN hPen;
HPEN   hPenOld;
static int buttondown = FALSE;

int zoomable_view(void)
{
	/*	Permitted zoom screens */

	extern void plan_orth(),elev_orth(),end_orth(),
	full_persp(),port_persp(),starb_persp(),
	view_devel(),rollout(),edit_section(),
	shell_expansion(),EditSectionRepaint(),view_devel();
	static FUNC_PTR zoomok[] = {
		plan_orth,
		elev_orth,
		end_orth,
		full_persp,
		port_persp,
		starb_persp,
		view_devel,
		rollout,
		view_devel,
		EditSectionRepaint,
		shell_expansion,
		NULL		};
	FUNC_PTR *p = zoomok;

	if(!IsBadCodePtr((FARPROC) update_func)) {
		while(*p != NULL) {
			if(*p == update_func) return 1;
			p++;
		}
	}
//	message("Zoom not available for this screen");
	return 0;
}

void initzoom()
{
	extern int	xmaxi,ymaxi;
	extern REAL	xmin0,xmax0,ymin0,ymax0;
	int	xm,ym;
	long	xp,yp;
	extern int	zoom,scaled;
	RECT rc;

	if(!zoomable_view()) return;

	GetWindowRect(hWnd,&rc);
	xwleft = rc.left;
	xwright = rc.right;
	ywtop = rc.top;
	ywbottom = rc.bottom;

	dxzoom = (xwright - xwleft) / 88;
	dyzoom = (ywbottom - ywtop) / 66;

	xp = xright >> 1;
	yp = ybottom >> 1;
	xm = xright >> 2;
	ym = ybottom >> 2;

	xzleft   = xp - xm;
	xzright  = xp + xm;
	yzbottom = yp + ym;
	yztop    = yp - ym;

	xsh = &xzleft;
	ysh = &yztop;		/* corner to be moved defaults to top left */

	zoom = 1;
	(*init)();
	drawrect(xzleft,xzright,yzbottom,yztop);
	buttondown = FALSE;
	scaled = TRUE;
	context_id = 312;		/* zoom help id */
}

#ifndef linux

LRESULT proczoom(HWND hWndProc,UINT msg,WPARAM wParam,LPARAM lParam)
{
	extern REAL	xgorigin,xgslope,ygorigin,ygslope;
	extern int	xleft,ytop,ybottom,xright;
	extern REAL xmin,xmax,ymin,ymax;
	REAL	xmn,xmx,ymn,ymx;
	int		xm,ym;
	long	xp,yp;
	extern int	xmaxi,ymaxi;
	int		dx1,dx2,dy1,dy2;
	extern int	zoom;
	PAINTSTRUCT ps;
	extern int	undoruling,section_edit;
	extern int	repaint,scaled,edit_scaled;
	extern REAL GLscale;
	GLint	viewport[4];
	GLdouble modelview[16];
	GLdouble projection[16];
	GLdouble posX,posY,posZ;
	REAL	x,y,xsc,ysc;
	static int xprev,yprev;

	switch(msg) {

	case WM_SIZE:
		PostMessage(hWnd,WM_PAINT,0,0l);
		GetSizes(hWnd);
		goto abortzoom;

	case WM_PAINT:
		BeginPaint(hWndProc,&ps);
		if(!IsBadCodePtr((FARPROC) update_func)) (*update_func)();
		drawrect(xzleft,xzright,yzbottom,yztop);
		EndPaint(hWndProc,&ps);
		SetFocus(hWndProc);
		break;

	case WM_RBUTTONDOWN:
	case WM_CLOSE:
	case WM_DESTROY:
		goto abortzoom;

	case WM_KEYDOWN:
		switch(wParam) {
		case VK_F1:		/* help */
			context(context_id);
			break;

		case VK_ESCAPE:
abortzoom:
			xmin = xmin0;
			xmax = xmax0;
			ymin = ymin0;
			ymax = ymax0;
			GLscale = 1.0;
			goto exitzoom;

		case VK_RETURN:
			xm = xzright  - xzleft;
			ym = yzbottom - yztop;
			xp = (long)xm * (ywbottom-ywtop);
			yp = (long)ym * (xwright -xwleft);
			if(xp < yp)
				xm = yp/(ybottom-ytop);
			else
				ym = xp/(xright-xleft);

			xp = (xzleft+xzright) >> 1;
			yp = (yzbottom+yztop) >> 1;
			xm >>= 1;
			ym >>= 1;
			xzleft   = xp-xm;
			xzright  = xp+xm;
			yzbottom = yp+ym;
			yztop    = yp-ym;
			if(xgslope == 0.0) xgslope = 1.0;
			if(ygslope == 0.0) ygslope = 1.0;

			xmn = ((float) xzleft  - xgorigin)/xgslope;
			xmx = ((float) xzright - xgorigin)/xgslope;
			ymn = ((float) (ymaxi - yzbottom) - ygorigin)/ygslope;
			ymx = ((float) (ymaxi - yztop)    - ygorigin)/ygslope;

			if(scrdev == 0) {
				GLzoom(xmn,xmx,ymn,ymx);
			} else {
				setranges(xmn,xmx,ymn,ymx);
			}

			scaled = TRUE;
exitzoom:
			zoom = 0;

			if(!undoruling) {
				InvalidateRect(hWnd,NULL,FALSE);
				edit_scaled = TRUE;
				repaint = TRUE;
				PostMessage(hWndProc,WM_PAINT,0,0l);
			}
			break;

		case VK_TAB:		/* other corner toggle */
			if(xsh == &xzleft) {
				xsh = &xzright;
				ysh = &yzbottom;
			}
			else {
				xsh = &xzleft;
				ysh = &yztop;
			}
			break;

		case VK_DOWN:
			if(scrdev == 1) drawrect(xzleft,xzright,yzbottom,yztop);
			if(*ysh < ymaxi-dyzoom) (*ysh) += dyzoom;
			drawrect(xzleft,xzright,yzbottom,yztop);
			break;

		case VK_UP:
			if(scrdev == 1) drawrect(xzleft,xzright,yzbottom,yztop);
			if(*ysh > dyzoom) (*ysh) -= dyzoom;
			drawrect(xzleft,xzright,yzbottom,yztop);
			break;

		case VK_LEFT:
			if(scrdev == 1) drawrect(xzleft,xzright,yzbottom,yztop);
			if(*xsh > dxzoom) (*xsh) -= dxzoom;
			drawrect(xzleft,xzright,yzbottom,yztop);
			break;

		case VK_RIGHT:
			if(scrdev == 1) drawrect(xzleft,xzright,yzbottom,yztop);
			if(*xsh < xmaxi-dxzoom) (*xsh) += dxzoom;
			drawrect(xzleft,xzright,yzbottom,yztop);
			break;
		}
		break;

	case WM_LBUTTONDOWN:

		/*	left-hand key is "drag" key					*/

		/*	first nearest of corners, and place cursor there		*/

		xm = LOWORD(lParam);
		ym = HIWORD(lParam);
		dx1 = fabs(xm - xzleft);
		dy1 = fabs(ym - yztop);
		dx2 = fabs(xzright - xm);
		dy2 = fabs(yzbottom - ym);
		if(dx1 < dx2)	xsh = &xzleft;
		else			xsh = &xzright;
		if(dy1 < dy2)	ysh = &yztop;
		else			ysh = &yzbottom;

		if(scrdev == 1) {
			SetCursorPos(*xsh + xwleft,*ysh + ywtop);
		} else {
			xsc = (xmax - xmin) / (REAL) (xwright - xwleft);	// scale from window to GL coordinates
			x = xmin + (REAL) *xsh * xsc;
			ysc = (ymax - ymin) / (REAL) (ywbottom - ywtop);
			y = ymax - (REAL) *ysh * ysc;
			glGetDoublev( GL_MODELVIEW_MATRIX, modelview );
			glGetDoublev( GL_PROJECTION_MATRIX, projection );
			glGetIntegerv( GL_VIEWPORT, viewport );
			gluProject(x,y,0.0,modelview,projection,viewport,&posX,&posY,&posZ);
			xprev = (int) posX;
			yprev = viewport[3] - (int) posY;
			SetCursorPos(xwleft + xprev,ywtop + yprev);
		}
		buttondown = TRUE;
		break;

	case WM_LBUTTONUP:
		buttondown = FALSE;
		break;

	case WM_MOUSEMOVE:
		if(wParam == MK_LBUTTON || buttondown) {
			if(scrdev == 1) {
				drawrect(xzleft,xzright,yzbottom,yztop);
				*xsh = LOWORD(lParam);
				*ysh = HIWORD(lParam);
			} else {
				xm = LOWORD(lParam);
				ym = HIWORD(lParam);
				*xsh += xm - xprev;
				*ysh += ym - yprev;
				xprev = xm;
				yprev = ym;
/*
				xsc = (xmax - xmin) / (REAL) (xwright - xwleft);
				x = xmin + (REAL) xm * xsc;
				ysc = (ymax - ymin) / (REAL) (ywbottom - ywtop);
				y = ymax - (REAL) ym * ysc;
				glGetDoublev( GL_MODELVIEW_MATRIX, modelview );
				glGetDoublev( GL_PROJECTION_MATRIX, projection );
				glGetIntegerv( GL_VIEWPORT, viewport );
				gluProject(x,y,0.0,modelview,projection,viewport,&posX,&posY,&posZ);
				*xsh = (int) posX;
				*ysh = viewport[3] - (int) posY;
*/
			}
			drawrect(xzleft,xzright,yzbottom,yztop);
		}
		break;

	default:
		return DefWindowProc(hWndProc,msg,wParam,lParam);
	}
	return((LRESULT) 0);
}

void drawrect(int xleft,int xright,int ybottom,int ytop)
{
	REAL xl,xr,yb,yt,xsc,ysc;
	extern int scaled;

	if(scrdev == 1) {
		SetROP2(hDC,R2_XORPEN);
		xorcol(1);
		MoveToEx(hDC,xleft,ytop,NULL);
		LineTo(hDC,xright,ytop);
		LineTo(hDC,xright,ybottom);
		LineTo(hDC,xleft,ybottom);
		LineTo(hDC,xleft,ytop);
		SetROP2(hDC,R2_COPYPEN);
	} else {
		scaled = FALSE;
		(*update_func)();
		GLcr(1);	// Normal colour under GL

//	Arguments are screen coordinates; transform to GL coordinates at the zero plane
//		xwleft , etc, are window boundary coordinates

		xsc = (xmax - xmin) / (REAL) (xwright - xwleft);	// scale from window to GL coordinates
		xl = xmin + (REAL) xleft * xsc;
		xr = xmin + (REAL) xright * xsc;
		ysc = (ymax - ymin) / (REAL) (ywbottom - ywtop);
		yb = ymax - (REAL) ybottom * ysc;
		yt = ymax - (REAL) ytop * ysc;

//	Must square up the view to display the rectangle

		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glRotatef(0.0,0.0,1.0,0.0);

		glEnable(GL_LINE_STIPPLE);
		glLineStipple(8,0xAAAA);

		glBegin(GL_LINE_LOOP);
		glVertex3f(xl,yb,0.0);
		glVertex3f(xr,yb,0.0);
		glVertex3f(xr,yt,0.0);
		glVertex3f(xl,yt,0.0);
		glEnd();

		glDisable(GL_LINE_STIPPLE);

		SwapBuf();
	}
}
#endif

#endif

/*=============================================================	*/

/*	MULTIPLE INPUT OF INTEGERS TO SET ARRAY LOCATIONS .TRUE. OR .FALSE.	*/

/*	FORMAT MAY BE "N,N,N, ..." OR "N:N,N,N:N ..."				*/

/*	Inputs	inp		string to receive input						*/
/*			maxs	maximum number of values in array "relax"	*/
/*	Outputs	relax	array of logicals, summary of on/off states	*/

int multin(char *prompt,int relax[],int maxs)
{
	char inp[300];
	listseq(relax,inp,maxs);
	if(!getdlg(GETLINE,
		INP_STR,(void *) inp,
		0,(void *) prompt,-1)) return(0);
	return(multproc(inp,relax,maxs));
}

int multproc(char lin[],int table[],int maxtab)
{
	INT	i,n,i1,i2;
	char *impr_char = "IMPROPER CHARACTER IN INPUT";
	int	result;
	char *line = (char *) lin;

	for(i = 0 ; i < MAX_PATH ; i++) {
		if(lin[i] == '\0') break;
	}
	if(i >= MAX_PATH) strcpy(line,"ALL");

	for(i = 0 ; i < maxtab ; i++) table[i] = 0;
	result = 1;
	strupr(line);

	if(strstr(line,"NONE") != NULL) {

		;	/* NONE is valid input */

	}
	else if(strstr(line,"ALL") != NULL) { /* ALL is valid too */

		for(i = 0 ; i < maxtab ; i++) table[i] = 1;

	}
	else {

		/*	INITIAL RANGE VALUE DEFAULTS TO 0	*/

		i1 = 0;

		/*	SCAN THROUGH INPUT LINE:	*/

		while(*line) {

			n = *line;

			/*	SPACE OR COMMA: LOOP BACK FOR NEXT CHARACTER	*/

			if(n == ' ' || n == ',') {
				line++;

				/*	DIGIT:	*/

			}
			else if(n >= '0' && n <= '9') {
				i1 = nread(&line);
				if(i1 < maxtab && i1 >= 0) table[i1] = 1;

				/*	COLON:	*/

			}
			else if(n == ':') {

				/*		READ SECOND VALUE OF RANGE:	*/

				line++;
				i2 = nread(&line);

				/*		DEFAULT IS END OF RANGE		*/

				if(i2 >= maxtab || i2 < 0) i2 = maxtab - 1;

				/*		START VALUE IS LAST ONE READ, OR 0 IF NONE PRIOR	*/

				while(++i1 <= i2) table[i1] = 1;
			}
			else {
				message(impr_char);
				result = 0;
				break;
			}
		}
	}
	return(result);
}

INT nread(char **str)
{
	INT	val = 0;
	INT	c;
	char *init;
	init = *str;
	while(isdigit( ( c = **str) )) {
		val = 10*val + (c - '0');
		(*str)++;
	}
	if(init != *str)
		return(val);
	else
	    return(-1);	/* error code */
}



