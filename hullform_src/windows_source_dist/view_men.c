/* Hullform component - view_men.c
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
 
/**********************************************************************/

/*      View menu                                                       */

#include "hulldesi.h"

#define	radian	0.01745329

extern	REAL	x1,x2,yy1,y2;	/* screen graphics limits	*/
extern	REAL	rotn,heelv;		/* hull rotation and heel	*/
extern	REAL	pp;				/* angle of picture plane	*/
void    check_balance(void);
extern	REAL	yview,zview;	/* viewing position		*/
extern	REAL	sinpp,cospp;	/* sine and cosine of picture plane angle */

extern	int	scaled;				/* rescale-required toggle	*/
int		not_scaled_req;
extern	int	hardcopy;			/* hardcopy toggle		*/
extern	int	showcentres;

int		shaded = 0;				/* shaded-surface toggle	*/
REAL	l_azim = -90.0;			/* lighting azimuth		*/
REAL	l_elev = 0.0;			/* lighting elevation		*/
extern	REAL outl_thickness;	/* hull outline thickness	*/
extern	int	surfsp;				/* space required for surface data */
extern	INT	inisec,lassec;
extern	int	endlin;				/* end line, set depending on value of "showtanks */
extern	REAL	PixelRatio;
extern	int	shownumbers,shownames;
extern  int	showoverlay;
extern	int	change_clear(int scroll);
void setranges(REAL xl,REAL xr,REAL yb,REAL yt);
extern int zoom;				// don't close graphics when zooming a view

#ifdef EXT_OR_PROF
void	use_hull(int);
#endif

#ifdef PROF
void draw_dxf_profile(void),draw_dxf_plan(void),draw_dxf_endview(void);

#ifdef DXFOVL
extern void show_dxf_xz(REAL,REAL,REAL,REAL),show_dxf_xy(REAL,REAL,REAL,REAL),
show_dxf_yz(REAL,REAL,REAL,REAL),show_dxf_xyz(REAL xmin,REAL xmax,REAL ymin,REAL ymax);
extern int dxf_overlay;
#endif

#endif

void SwapBuf(void);

MENUFUNC general_orth()
{
	REAL	xa,xb,ya,za,zb,yt1,yb1;
	REAL	xsize,ysize,xs,ys,shift;
	REAL	spare;
	REAL	xl,xr,yb,yt;
	extern	int xmaxi,ymaxi,xleft,xright,ybottom,ytop;
	REAL	ar = (REAL) (xright - xleft) / (REAL) (ybottom - ytop) / PixelRatio;
	extern	REAL Xwidth;
	extern	int hardcopy,fixed_scale;
	extern int scrollable;

	if(count <= 1) return;

#ifndef STUDENT
	endlin = showtanks ? extlin : numlin;
#else
	endlin = numlin;
#endif

	graphics = 1;
	perset(0);

	get_ranges(&xa,&xb,&ya,&za,&zb);

	/*	All three views together				*/

	xs = (xb - xa) + ya;		/* length plus beam	*/
	ys = (zb - za) + ya;		/* depth plus beam	*/
	spare = 0.2 * max(xs,ys);	/* 20% room between and around views*/

	/*	xs,ys is size of plot area on which drawing will be placed;
	xsize,ysize is size of whole plotting surface (>= xs,ys)
	*/
	xs += 3.0 * spare;
	ys += 3.0 * spare;

	if(xs > ar * ys) {	/* long, narrow plot */
		xsize = xs;
		ysize = xs / ar;
	}
	else {		/* deep, narrow plot */
		xsize = ys * ar;
		ysize = ys;
	}

	if(posdir > 0.0) {

		/*	Stem to right: right-hand edge is low co-ordinate */

		xr = xa - spare; /* co-ordinate at right edge of surface */
		xl = xr + xsize; /* co-ordinate at left edge of surface */

	}
	else {

		/*	Stem to left: right-hand edge is high co-ordinate */

		xr = xb + spare;
		xl = xr - xsize;
	}
	yt = za - spare - 0.5*(ysize - ys);
	yb = yt + ysize;

	update_func = general_orth;
	print_func = update_func;
	check_balance();
	(*init)();
	cls(FALSE);

	side_elevation(xl,xr,yb,yt);	// uses xl,xr,yb,yt for window coordinate limits

	if(hardcopy && fixed_scale && posdir > 0.0) Xwidth = - Xwidth;

	shift = -0.5*ya - spare;
	end_elevation(shift,xsize + shift,yb,yt);

	if(hardcopy && fixed_scale && posdir > 0.0) Xwidth = - Xwidth;
	draw_datum();

	yt1 = ya + 0.5*(ysize - ys);
	yb1 = yt1 - ysize;
	top_plan(xl,xr,yt1,yb1);

	show_scale(xl,xr,yt1,yb1);

#ifdef EXT_OR_PROF
	if(showoverlay) {
		use_hull(1);	/* use overlay */
#ifndef STUDENT
		endlin = showtanks ? extlin : numlin;
#else
		endlin = numlin;
#endif
		side_elevation(xl,xr,yb,yt);
		if(hardcopy && fixed_scale && posdir > 0.0) Xwidth = - Xwidth;
		end_elevation(shift,xsize + shift,yb,yt);
		if(hardcopy && fixed_scale && posdir > 0.0) Xwidth = - Xwidth;
		top_plan(xl,xr,yt1,yb1);
		use_hull(0);	/* back to main hull */
	}
#ifdef DXFOVL
	if(dxf_overlay) {
		show_dxf_xz(xl,xr,yb,yt);
		show_dxf_xy(xl,xr,yt1,yb1);
		show_dxf_yz(shift,xsize + shift,yb,yt);
	}
#endif

#endif
	SwapBuf();
}

MENUFUNC plan_orth()
{
	REAL	xa,xb,ya,za,zb;
	REAL	size;
	REAL	xm;
	extern	REAL posdir;
	extern	int xmaxi,ymaxi,xleft,xright,ybottom,ytop;
	extern	REAL PixelRatio;
	extern int scrollable;
	REAL	ar = (REAL) (xright - xleft) / (REAL) (ybottom - ytop) / PixelRatio;

	if(count <= 1) return;

#ifndef STUDENT
	endlin = showtanks ? extlin : numlin;
#else
	endlin = numlin;
#endif

	graphics = 1;
	perset(0);

	/*	Plan view						*/

	/*	use previous scale in zoom or hardcopy mode, otherwise	*/
	/*	redefine it						*/

	if(hardcopy) {
		xa = fabf(xmax - xmin);
		ya = ar*(ymax - ymin);
		if(xa > ya) {
			ymax = 0.5*xa/ar;
			ymin = -ymax;
		}
		else {
			xa = 0.5*ya*posdir;
			xm = (xmax+xmin)*0.5;
			xmin = xm + xa;
			xmax = xm - xa;
		}
	}
	else if(! scaled) {
		get_ranges(&xa,&xb,&ya,&za,&zb);
		size = max(xb - xa,ya * ar) * 0.6;
		xm = 0.5*(xb+xa);
		setranges(xm+posdir*size,xm-posdir*size,-size / ar,size / ar);
	}

	update_func = plan_orth;
	print_func = update_func;
	check_balance();
	(*init)();
	cls(FALSE);
	top_plan(xmin,xmax,ymin,ymax);
#ifdef EXT_OR_PROF
	if(showoverlay) {
		use_hull(1);	/* use overlay */
#ifndef STUDENT
		endlin = showtanks ? extlin : numlin;
#else
		endlin = numlin;
#endif

		top_plan(xmin,xmax,ymax,ymin);
		use_hull(0);	/* main hull */
	}
#ifdef DXFOVL
	if(dxf_overlay) show_dxf_xy(xmin,xmax,ymax,ymin);
#endif

#endif
	show_scale(xmin,xmax,ymin,ymax);
	SwapBuf();
}

MENUFUNC elev_orth()
{
	REAL	xa,xb,ya,za,zb;
	static	REAL	size;
	static	REAL	xm,zm;
	extern	REAL posdir;
	extern	int xmaxi,ymaxi,xleft,xright,ybottom,ytop;
	extern	REAL PixelRatio;
	extern	int scrollable;
	REAL	ar = (REAL) (xright - xleft) / (REAL) (ybottom - ytop) / PixelRatio;

	if(count <= 1) return;

#ifndef STUDENT
	endlin = showtanks ? extlin : numlin;
#else
	endlin = numlin;
#endif

	graphics = 1;
	perset(0);

	/*	Side elevation						*/

	if(hardcopy) {
		xa = fabf(xmax - xmin);
		ya = ar*(ymax - ymin);
		if(xa > ya) {
			ya = 0.5*xa/ar;
			ymin = zm + ya;
			ymax = zm - ya;
		}
		else {
			xa = 0.5*ya*posdir;
			xmin = xm + xa;
			xmax = xm - xa;
		}
	}
	else if(! scaled) {
		get_ranges(&xa,&xb,&ya,&za,&zb);
		xm = xb - xa;
		zm = (zb - za) * ar;
		size = max( xm, zm ) * 0.6;
		xm = 0.5*(xb+xa);
		zm = 0.5*(zb+za);
		setranges(xm+posdir*size,xm-posdir*size,zm + size / ar,zm - size /ar);
	}
	update_func = elev_orth;
	print_func = update_func;
	check_balance();
	(*init)();
	cls(FALSE);
	side_elevation(xmin,xmax,ymin,ymax);

#ifdef EXT_OR_PROF
	if(showoverlay) {
		use_hull(OVERLAYHULL);
#ifndef STUDENT
		endlin = showtanks ? extlin : numlin;
#else
		endlin = numlin;
#endif
		side_elevation(xmin,xmax,ymin,ymax);
		use_hull(MAINHULL);
	}
#ifdef DXFOVL
	if(dxf_overlay) show_dxf_xz(xmin,xmax,ymin,ymax);
#endif

#endif
	show_scale(xmin,xmax,ymin,ymax);
	draw_datum();
	SwapBuf();
}

MENUFUNC end_orth()
{
	REAL	xa,xb,ya,za,zb;
	static	REAL	size;
	static	REAL	zm;
	extern	REAL posdir;
	extern	int xmaxi,ymaxi,xleft,xright,ybottom,ytop;
	extern	REAL PixelRatio;
	extern int scrollable;
	REAL	ar = (REAL) (xright - xleft) / (REAL) (ybottom - ytop) / PixelRatio;

	if(count <= 1) return;

#ifndef STUDENT
	endlin = showtanks ? extlin : numlin;
#else
	endlin = numlin;
#endif

	graphics = 1;
	get_ranges(&xa,&xb,&ya,&za,&zb);
	perset(0);

	/*	End elevation						*/

	if(hardcopy) {
		xa = fabf(xmax - xmin);
		ya = ar*(ymax - ymin);
		if(xa > ya) {
			ya = 0.5*xa/ar;
			ymin = zm + ya;
			ymax = zm - ya;
		}
		else {
			xa = 0.5*ya*posdir;
			xmin = -xa;
			xmax =  xa;
		}
	}
	else if(! scaled) {
		size = max(ya,(zb - za) * ar) * 0.6;
		zm = 0.5*(zb+za);
		setranges(-size,size,zm+size/ar,zm-size/ar);
	}
	update_func = end_orth;
	print_func = update_func;
	check_balance();
	(*init)();
	cls(FALSE);
	end_elevation(xmin,xmax,ymin,ymax);

#ifdef EXT_OR_PROF
	if(showoverlay) {
		use_hull(1);	/* use overlay */
#ifndef STUDENT
		endlin = showtanks ? extlin : numlin;
#else
		endlin = numlin;
#endif
		end_elevation(xmin,xmax,ymin,ymax);
		use_hull(0);	/* main hull */
	}
#ifdef DXFOVL
	if(dxf_overlay) show_dxf_yz(xmin,xmax,ymin,ymax);
#endif

#endif
	show_scale(xmin,xmax,ymin,ymax);
	draw_datum();
	SwapBuf();
}

MENUFUNC full_persp()
{
	update_func = full_persp;
	draw_persp(-1.0,1.0);
	print_func = update_func;
}



int temp_gl = FALSE;
HWND save_gdi_hwnd = NULL;
HDC save_gdi_hdc;
HMENU ShadeViewMenu;
extern HDC hDC;
extern int repaint;
extern int GL_surfaces_need_redraw;
extern HWND hWndView;

void draw_persp(REAL side1,REAL side2)
{
	REAL xspare,yspare;
	extern int zoom;
	REAL xa,ya;
	static REAL zm,xm;
	extern int xleft,xright,ybottom,ytop;
	REAL ar = (REAL) (xright - xleft) / (REAL) (ybottom - ytop) / PixelRatio;
	extern int scrollable;
#ifndef linux
	RECT rc;
	extern HINSTANCE hInst;
	void window(HWND hParent,char *title,int xorigin,int yorigin,int xsize,int ysize,char *winclass,DWORD style,HWND *hWndRet);
#endif

#ifdef PROF
	extern	REAL (*xl_dxf)[maxsec];
	void draw_dxf_view(void);
#endif

/*	Option of showing a shaded view in a temporary GL window	*/

#ifndef linux
	if(scrdev == 1 && temp_gl && shaded && hWndView == NULL) {
		save_gdi_hwnd = hWnd;
		save_gdi_hdc = hDC;
		scrdev = 0;
		GetWindowRect(hWnd,&rc);
		setup(0);
		hWnd = CreateWindow(
			"HFCLASS",
			"Shaded Surface View",
			WS_OVERLAPPEDWINDOW,
			rc.left+10,rc.top+50,rc.right - rc.left-20,rc.bottom - rc.top - 50,
			hWnd,
			NULL,
			hInst,
			NULL);
		if(hWnd) {
			hDC = GetDC(hWnd);
			hWndView = hWnd;
			ShadeViewMenu = LoadMenu(hInst,"SHADEVIEW");
			SetMenu(hWnd,ShadeViewMenu);
			PostMessage(hWnd,WM_PAINT,0,0l);
			repaint = TRUE;
			GL_surfaces_need_redraw = TRUE;
			ShowWindow(hWnd,SW_SHOWNORMAL);
			UpdateWindow(hWnd);
		}
	}
#endif
	graphics = 1;

#ifndef STUDENT
	endlin = showtanks ? extlin : numlin;
#else
	endlin = numlin;
#endif

	pp = atan2(yview,zview)/radian;
	sinpp = sin(radian*pp);
	cospp = cos(radian*pp);
	sina = sin(radian*rotn);
	cosa = cos(radian*rotn);

	if(hardcopy) {
		xa = fabf(xmax - xmin);
		ya = ar*(ymax - ymin);
		if(xa > ya) {
			ya = 0.5*xa/ar;
			ymin = zm - ya;
			ymax = zm + ya;
		}
		else {
			xa = 0.5*ya*posdir;
			xmin = xm - xa;
			xmax = xm + xa;
		}
	}
	else if(! scaled) {
		scale_view(rotn,pp,yview,zview,&inisec,&lassec);
		xspare = 0.1*(x2-x1);
		yspare = 0.1*(y2-yy1);
		xm = 0.5*(x2+x1);		// for use in hardcopy output calculations
		zm = 0.5*(y2+yy1);
		setranges(x1-xspare,x2+xspare,yy1-yspare,y2+yspare);
	}

	check_balance();
	(*init)();
	cls(TRUE);
	draw_view(xmin,xmax,ymin,ymax,
		inisec,lassec,endlin,side1,side2,
		heel,pitch,rotn,secton,lineon,
		wate_int,diag_int,diag_angle,butt_int);
#ifdef EXT_OR_PROF
	if(showoverlay && !shaded) {
		use_hull(1);	/* use overlay */
		if(inisec == 1) inisec = 0;
#ifndef STUDENT
		endlin = showtanks ? extlin : numlin;
#else
		endlin = numlin;
#endif

		draw_view(xmin,xmax,ymin,ymax,
			inisec,min(count-1,lassec),endlin,side1,side2,
			heel,pitch,rotn,secton,lineon,
			wate_int,diag_int,diag_angle,butt_int);
		use_hull(0);	/* main hull */
	}
#ifdef DXFOVL
	if(dxf_overlay) show_dxf_xyz(xmin,xmax,ymin,ymax);
#endif

#endif
	scaled = TRUE;
#ifdef PROF
	if(xl_dxf != NULL) draw_dxf_view();
#endif
	SwapBuf();
}

MENUFUNC port_persp()
{
	update_func = port_persp;
	draw_persp(-1.0,-1.0);
	print_func = update_func;
}

MENUFUNC starb_persp()
{
	update_func = starb_persp;
	draw_persp(1.0,1.0);
	print_func = update_func;
}

void view_options()
{
	void def_builders(int,HWND);
	extern int start_at_waterline;
    extern int numbetwl;
	int line_flag = wate_int > 0.0 ? 0 :
	butt_int > 0.0 ? 1 :
	diag_int > 0.0 ? 2 : 3;
	REAL interv;
	int i;
	extern int show_dxf;

	switch(line_flag) {
	case 0:
		interv = wate_int;
		break;
	case 1:
		interv = butt_int;
		break;
	case 2:
		interv = diag_int;
		break;
	default:
		interv = 0.0;
		break;
	}

	if(getdlg(VIEWOPTI,
		INP_STR,(void *) linestoplot,
		INP_STR,(void *) sectionstoplot,
#ifdef PROF
		INP_LOG,(void *) &showtanks,
		INP_LOG,(void *) &showstringers,
#else
		INP_LOG,NULL,
		INP_LOG,NULL,
#endif
		INP_LOG,(void *) &showframes,
		INP_LOG,(void *) &shownumbers,
		INP_LOG,(void *) &shownames,
		INP_LOG,(void *) &showcentres,
#ifdef EXT_OR_PROF
		INP_LOG,(void *) &showoverlay,
#ifdef PROF
		INP_LOG,(void *) &show_dxf,
#else
		INP_LOG,NULL,
#endif
#else
		INP_LOG,NULL,
		INP_LOG,NULL,
#endif
		INP_REA,(void *) &interv,
		INP_REA,(void *) &diag_angle,
		INP_RBN,(void *) &line_flag,
		INP_LOG,(void *) &start_at_waterline,
		INP_PBF,def_builders,
#ifdef EXT_OR_PROF
		INP_REA,(void *) &l_azim,
		INP_REA,(void *) &l_elev,
		INP_RBN,(void *) &shaded,
#else
		INP_REA,NULL,
		INP_REA,NULL,
		INP_RBN,NULL,
#endif
		INP_REA,(void *) &rotn,
		INP_REA,(void *) &yview,
		INP_REA,(void *) &zview,
		-1)) {
		wate_int = diag_int = butt_int = 0.0;
#ifdef PROF
		showoverlay &= count_overlay >= 2;
		surfsp = 4 * (endlin - 1) * (numbetwl+1) * 4 + 4;
#endif
		switch(line_flag) {
		case 0:
			wate_int = interv;
			break;
		case 1:
			butt_int = interv;
			break;
		case 2:
			diag_int = interv;
			break;
		}
		(void) multproc(sectionstoplot,secton,maxsec);
		(void) multproc(linestoplot,lineon,maxlin);
		for(i = 0 ; i < extlin ; i++) lineon[i] = lineon[i+1];
		if(zview*zview+yview*yview > 1.0e10) message("WARNING:\n\nDistances over 100000 can lead to truncation problems\nin OpenGL shaded surface viewing.");
	}
}

void get_ranges(REAL *xa,REAL *xb,REAL *ya,REAL *za,REAL *zb)
{
	int i,j;
	REAL y,z;

	*ya = 0.0;		/* to be beam */
	*za = 1.0e+30;	/* to be least z */
	*zb = -1.0e+30;	/* to be greatest z */
	for(j = 0 ; j < numlin ; j++) {
		for(i = stsec[j] ; i <= ensec[j] ; i++) {
			z = zline[j][i];
			y = yline[j][i];
			if(y > *ya) *ya = y;
			if(z < *za) *za = z;
			if(z > *zb) *zb = z;
		}
	}
	*xa = xsect[0]-dxstem();	/* lowest x */
	*xb = xsect[count-1];	/* highest x */
	*ya += *ya;			/* make full beam */

	if(showcentres) {
		check_balance();
		if(*zb < zcofm) *zb = zcofm;
		if(*za > zcofm) *za = zcofm;
		if(*zb < zmeta) *zb = zmeta;
		if(*za > zmeta) *za = zmeta;
	}
}




