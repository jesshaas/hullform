/* Hullform component - view_su1.c
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

void draw_perspective(REAL side1,REAL side2);
void draw_view(REAL x1,REAL x2,REAL y1,REAL y2,
	INT inisec,INT lassec,int endlin,REAL side1,REAL side2,
	REAL heel0,REAL pitchv,REAL rotn,INT secton[],INT lineon[],
	REAL wate_int,REAL diag_int,REAL diag_angle,REAL butt_int);
extern int shownumbers,shownames;
REAL ndxstem(void);
void make_outline(int i,REAL tn,REAL del,int mode,FILE *fp,REAL side);
extern REAL outl_thickness;
void show_centres(int,int,int,int mode);
int start_at_waterline = TRUE;
extern int showcentres;
extern char (*sectname)[12];

void check_balance(void);
void show_numbers(int);

void set_GL_perspective(void);
#ifdef linux
#undef tc
#include <GL/gl.h>
#include <GL/glu.h>
#endif

#ifdef PROF
void draw_dxf_plan(void),draw_dxf_profile(void),draw_dxf_endview(void);
#endif

/************************************************************************/

/*	Service routines for view menu					*/

/*	Plan view of hull			*/

void top_plan(REAL xxmin,REAL xxmax,REAL yymin,REAL yymax)
{
	extern int	xleft,ytop,xright,ybottom,xmaxi,ymaxi;
	INT	i,nl,it;
	REAL ystem;
	REAL s1,s2,yt;
	REAL maxoff,minoff,y,x;
	REAL dummy[maxsec*maxext];
	int	ignore;
#ifdef PROF
	extern int numtran;
#endif

	(*colour)(3);
	setranges(xxmin,xxmax,yymin,yymax);

#ifdef PROF
	draw_dxf_plan();
#endif
	/*	DRAW PLAN VIEW OF STEM(S)	*/

	if(disp > 0.0) {
		ystem = yline[stemli][1];
		(*move)(posdir*(xsect[0]-dxstem()),ystem);
		(*draw)(posdir*(xsect[0]-ndxstem()),ystem);
		if(ystem != 0.0) {
			(*move)(posdir*(xsect[0]-dxstem()),-ystem);
			(*draw)(posdir*(xsect[0]-ndxstem()),-ystem);
		}
	}

	/*	PLOT HULL LINES ON BOTH SIDES	*/

#ifndef STUDENT
	tank = 0;
#endif
	s1 = -1.0;
	s2 = 1.0;
	for(nl = 0 ; nl < endlin ; nl++) {
#ifndef STUDENT
		if(nl == fl_line1[tank]) {
			if(fl_right[tank]) {
				s1 = -1.0;
				s2 = -1.0;
			}
			else {
				s1 = 1.0;
				s2 = 1.0;
			}
			tank++;
		}
#endif
		if(lineon[nl]) draw_line(nl,s1,s2,1,0,count-1,99,dummy,dummy,dummy,&ignore,&ignore,&ignore,1);
	}

	/*	plot transom			*/

	(*colour)(2);
#ifndef STUDENT
	draw_transom(-1.0f,1.0f,1);
#endif

	/*	PLOT SECTIONS ON BOTH SIDES, AS REQUIRED	*/

#ifdef PROF
	it = 0;
#endif
	for(i = 1 ; i < count ; i++) {
		maxoff = -1.0e+30;
		if(secton[i]) {
			for(nl = 0 ; nl < numlin ; nl++) {
				if(stsec[nl] <= i && ensec[nl] >= i) {
					if(yline[nl][i] > maxoff) maxoff = yline[nl][i];
				}
			}
#ifdef PROF
			while(it < numtran && xtran[it] < xsect[i]) it++;
			if(it > 0 && it < numtran) {
				s1 = xtran[it] - xtran[it-1];
				if(s1 > 0.0) {
					s1 = (xtran[it] - xsect[i]) / s1;
					yt = ytran[it] - s1*(ytran[it] - ytran[it-1]);
					if(yt < maxoff) maxoff = yt;
				}
			}
#endif
			(*colour)(2);
			(*move)(xsect[i],-maxoff);
			(*draw)(xsect[i],maxoff);
			show_numbers(i);
		}
	}

	/*	PLOT HULL DIAGONALS REQUESTED	*/

	if(diag_int > 0.0) {
		(*colour)(6);
		pltdia(diag_int,diag_angle,1,-1.0);
		pltdia(diag_int,diag_angle,1, 1.0);
	}

	/*	PLOT WATERLINES REQUESTED	*/

	if(wate_int > 0.0) {
		(*colour)(4);
		offset_range(&maxoff,&minoff,0.0,wate_int);
		y = start_at_waterline ? wl : wate_int * ((int) (minoff/wate_int));
		plotw(1.0,y,wate_int,1);
		plotw(-1.0,y,wate_int,1);
	}

	/*	Plot buttock lines		*/

	if(butt_int > 0.0) {
		(*colour)(5);
		offset_range(&maxoff,&minoff,90.0,butt_int);
		x = posdir*(xsect[0] - dxstem());
		for(y = minoff ; y < maxoff ; y += butt_int) {
			(*move)(x,y);
			(*draw)(xsect[count-1],y);
			(*move)(x,-y);
			(*draw)(xsect[count-1],-y);
		}
	}

#ifndef STUDENT
	draw_stringers(-1.0,1.0,1,999);
#endif
	if(showcentres) show_centres(TRUE,TRUE,FALSE,1);
}

/*	PLOT SIDE ELEVATION OF HULL	*/

void side_elevation(REAL xl,REAL xr,REAL yb,REAL yt)
{
	extern int	xleft,ytop,xright,ybottom,xmaxi,ymaxi;
	REAL	lstern,lstem;
	REAL	zt;
	REAL	zsmax,zsmin;
	int		i,nl;
	REAL	maxoff,minoff,y,x;
	REAL	dummy[maxsec*maxext];
	int		ignore;

	/*	STEM PROFILE	*/

	(*colour)(3);
	if(disp >= 0.0) {
		setranges(xl-xsect[0],xr-xsect[0],yb,yt);
		drasec(0,0,-1.0,0);
	}
	setranges(xl,xr,yb,yt);

#ifdef PROF
	draw_dxf_profile();
#endif

	/*	HULL LINES	*/

	(*colour)(3);
	for(nl = 0 ; nl < endlin ; nl++) {
		if(lineon[nl]) draw_line(nl,1.0f,1.0f,2,0,count-1,99,dummy,dummy,dummy,&ignore,&ignore,&ignore,1);
	}

	/*	SECTIONS IF REQUESTED	*/

	(*colour)(2);
#ifndef STUDENT
	transom &= (stransom != 0.0);
#endif

	for(i = surfacemode ; i < count ; i++) {
		if(secton[i]) {
			zsmax = -1.0e+30;
			zsmin = 1.0e+30;
			for(nl = 0 ; nl < extlin ; nl++) {
				if(stsec[nl] <= i && ensec[nl] >= i) {
					zt = zline[nl][i];
					if(zt < zsmin) zsmin = zt;
					if(zt > zsmax) zsmax = zt;
				}
			}
#ifndef STUDENT
			if(transom && xsect[i] > xtran[0]) {
				y = (xsect[i]*ctransom-dtransom)/stransom;
				zt = ztran[0];
				if(fpos(stransom)) {
					if(zt < zsmin) zsmin = zt;
					if(zsmin < y) zsmin = y;
				}
				else {
					if(zsmax > zt) zsmax = zt;
					if(zsmax > y) zsmax = y;
				}
			}
#endif
			(*colour)(2);
			(*move)(xsect[i],zsmax);
			(*draw)(xsect[i],zsmin);
			show_numbers(i);
		}
	}

	/*	plot transom			*/

#ifndef STUDENT
	(*colour)(2);
	draw_transom(1.0f,1.0f,2);
#endif

	/*	HULL DIAGONALS IF REQUIRED	*/

	(*colour)(6);
	pltdia(diag_int,diag_angle,2,-1.0);

	/*	buttocks			*/

	(*colour)(5);
	pltdia(butt_int,90.0,2,-1.0);


	/*	draw waterlines				*/

	(*colour)(4);
	lstem = xsect[0] - dxstem();
	lstern = xsect[count-1]*1.05-lstem*0.05;
	if(wate_int > 0.0) {
		offset_range(&maxoff,&minoff,0.0,wate_int);
		y = start_at_waterline ? wl : wate_int*((int) (minoff/wate_int));
		for( ; y < maxoff ; y += wate_int) {
			(*move)(lstem,y-beta*xsect[0]-hwl[0]);
			for(i = 0 ; i < count ; i++) (*draw)(xsect[i],y-beta*xsect[i]-hwl[i]);
			(*draw)(lstern,y-beta*lstern-hwl[count-1]);
		}
	} else {
		(*move)(lstem,wl-beta*xsect[0]-hwl[0]);
		for(i = 0 ; i < count ; i++) (*draw)(xsect[i],wl-beta*xsect[i]-hwl[i]);
		(*draw)(lstern,wl-beta*lstern-hwl[count-1]);
	}

#ifndef STUDENT
	draw_stringers(1.0,1.0,2,999);
#endif
	if(showcentres) show_centres(TRUE,TRUE,TRUE,2);
}

/*	PLOT END ELEVATION OF HULL	*/

REAL startwidth = 0.0;

void end_elevation(REAL xl,REAL xr,REAL yb,REAL yt)
{
	extern int	xleft,ytop,xright,ybottom,xmaxi,ymaxi;
	int		i,j,iswap,nl;
	REAL	width,ystem,side;
	REAL	maxoff,minoff,t,y,x,x1,y1;
	REAL	Ytop,Ybottom;
	REAL	c,s;
	int		right;
	REAL	dummy[maxsec*maxext];
	int		ignore;
	FILE	*fp = NULL;
	extern	int ychar;
	REAL	yshift;
	extern	int pointsize;
	extern	int showport,showstbd,showtanks;

	showport = showstbd = showtanks;

	setranges(xl,xr,yb,yt);
	yshift = (REAL) pointsize * fabs(xr - xl) *0.001;

#ifdef PROF
	draw_dxf_endview();
#endif

	/*	DRAW STEM AND KEEL LINE	*/

	(*colour)(3);
	ystem = surfacemode ? 0.0 : yline[stemli][1];
	(*newlin)();
	for(i = 0 ; i < numlin ; i++) {
		if(stsec[i] == 0) (*draw)(ystem,zline[i][0]);
	}

	/*	FIND WIDEST POINT	*/

	width = startwidth;
	startwidth = 0.0;
	iswap = 999;
	for(j = 0 ; j < numlin ; j++) {
		for(i = max(1,stsec[j]) ; i <= ensec[j] ; i++) {
			if(yline[j][i] > width) {
				width = yline[j][i];
				iswap = i;
			}
		}
	}

	/*	ALWAYS DRAW THE SWAP SECTION ON BOTH SIDES	*/

	if(iswap < count && secton[iswap]) {
		(*colour)(2);
		drasec(iswap,-1,1.0,0);
#ifdef EXT_OR_PROF
		if(showframes) {
			(*newlin)();
			make_outline(iswap,outl_thickness,0.0,0,fp,1.0);
			(*colour)(2);
		}
#endif
	}

	/*	DRAW THE HULL LINES	*/

	(*colour)(3);
	for(nl = 0 ; nl < numlin ; nl++) {
		if(lineon[nl]) draw_line(nl,-1.0,1.0,0,0,count-1,iswap,dummy,dummy,dummy,&ignore,&ignore,&ignore,1);
	}
#ifndef STUDENT
	if(endlin > numlin) {
		for(tank = 0 ; tank < ntank ; tank++) {
			for(nl = fl_line1[tank] ; nl < fl_line1[tank+1] ; nl++) {
				if(lineon[nl] && stsec[nl] < iswap) {
					side = fl_right[tank] ? -1.0 : 1.0;
					draw_line(nl,side,side,0,stsec[nl],ensec[nl],
						-1,dummy,dummy,dummy,&ignore,&ignore,&ignore,1);
				}
			}
		}
	}
#endif

	/*	plot transom			*/

	(*colour)(2);
#ifndef STUDENT
	draw_transom(-1.0f,-1.0f,0);
#endif

	/*	IF OTHER SECTIONS WERE REQUESTED, DRAW THEM TOO	*/

	side = 1.0;
	showstbd = FALSE;
	showport = FALSE;
	right = -1;
	for(i = 1 ; i < count ; i++) {
		if(secton[i]) {
			if(i == iswap) {
				side = -1.0;
				right = 1;
			}
			(*colour)(2);
			drasec(i,right,side,0);
			if(iswap > count) drasec(i,-right,-side,0);
			if(showframes) {
				(*newlin)();
				make_outline(i,outl_thickness,0.0,0,fp,side);
			}
			if(shownumbers || shownames)
				(*move)(side*yline[0][i],zline[0][i] - yshift);
			show_numbers(i);
		}
	}

	/*	show waterlines at x = 0		*/

	width *= 1.15;

	if(wate_int > 0.0) {
		(*colour)(4);
		offset_range(&maxoff,&minoff,0.0,wate_int);
		y = start_at_waterline ? wl : wate_int*((int) (minoff/wate_int));
		for( ; y < maxoff ; y += wate_int) {
			(*move)(-width,y);
			(*draw)(width,y);
		}
	}

	/*	draw diagonals					*/

	s = sin(0.01745329 * diag_angle);
	c = cos(0.01745329 * diag_angle);
	(*colour)(6);
	offset_range(&Ybottom,&Ytop,0.0,0.01);
	if(diag_int > 0.0 && c != 0.0 && s != 0.0) {
		/* define range of offsets to use */
		offset_range(&maxoff,&minoff,-diag_angle,diag_int);
		/* define extreme limits of diagonals */
		for(t = minoff ; t < maxoff ; t += diag_int) {
			y1 = (t + width * s) / c;
			if(y1 > Ybottom) {
				y1 = Ybottom;
				x1 = (y1 * c - t) / s;
			}
			else {
				x1 = width;
			}
			y = t / c;
			if(y < Ytop) {
				y = Ytop;
				x = (y * c - t) / s;
			}
			else {
				x = 0.0;
			}
			(*move)(x1,y1);
			(*draw)(x,y);
			(*move)(-x,y);
			(*draw)(-x1,y1);
		}
	}

	/*	draw buttocks					*/

	if(butt_int > 0.0) {
		(*colour)(5);
		offset_range(&maxoff,&minoff,90.0,butt_int);
		/* just for max and min values of vertical offset */
		for(t = minoff ; t < maxoff ; t += butt_int) {
			(*move)(t,Ybottom);
			(*draw)(t,Ytop);
			(*move)(-t,Ybottom);
			(*draw)(-t,Ytop);
		}
	}

#ifndef STUDENT
	draw_stringers(-1.0,1.0,0,iswap);
#endif
	if(showcentres) show_centres(TRUE,FALSE,TRUE,0);
}

void draw_datum()
{
	extern REAL xmin,xmax;

	(*colour)(1);
	(*move)(xmin,0.0);
	(*draw)(xmax,0.0);
}

/*	show drawing scale on plot			*/

void show_scale(REAL xleft,REAL xright,REAL ybottom,REAL ytop)
{
	extern int xchar,ychar,xmaxi,ymaxi,pointsize;
	REAL	y0 = 0.9 * ybottom + 0.1 * ytop;	/* base of tick	*/
	REAL	y1 = y0 + 0.02 * (ytop - ybottom);	/* top of tick	*/
	REAL	x1 = 0.94 * xleft + 0.06 * xright;	/* axis start	*/
	REAL	x2 = 0.375*(xright - xleft);		/* axis length	*/
	REAL	dx = axincr(fabs(x2));				/* label interval	*/
	REAL	x;									/* loop variable	*/
	REAL	xx;									/* x-location in loop	*/
	REAL	dxchar = (xright - xleft)* (REAL)xchar/((REAL)xmaxi);
	REAL	dychar = (ytop - ybottom)* (REAL)ychar/((REAL)ymaxi);
	int		reversed = xleft > xright;			/* direction flag	*/
	int		ndp,nsf;							/* decimal places */
	REAL	xoffset,yoffset;

	ndp = -log10(dx);
	if(dx < 1.0) ndp++;
	nsf = ndp+3;
	if(ndp <= 0)
		xoffset = dxchar;
	else
		xoffset = ((REAL) nsf)*0.5*dxchar;
	if(scrdev == 0)
		yoffset = dychar;
	else
		yoffset = 0.25*dychar;

	if(reversed) dx = -dx;
	x2 += 0.5 * dx;
	(*colour)(1);
	(*newlin)();
	for(x = 0.0 ; reversed ? x > x2 : x < x2 ; x += dx) {
		xx = x + x1;
		(*draw)(xx,y0);
		(*draw)(xx,y1);
		(*move)(xx-xoffset,y0-yoffset);
		if(ndp <= 0)
			plint(abs((int) x),2);
		else
			plfix(fabs(x),nsf,ndp);
		(*move)(xx,y0);
	}
}

/*    draw perspective view in specified box		*/

void draw_view(REAL x1,REAL x2,REAL y1,REAL y2,
	INT inisec,INT lassec,int endlin,REAL side1,REAL side2,
	REAL heel0,REAL pitchv,REAL rotn,INT secton[],INT lineon[],
	REAL wate_int,REAL diag_int,REAL diag_angle,REAL butt_int)
{
	extern int shaded;
	INT     i,il,is;
	REAL    ystem,side;
	REAL    s1,s2;
	REAL    aa,cc,a,hb,c,hd;
	extern  REAL    heelv;
	REAL    t;
	REAL    minoff,maxoff;
	extern  REAL    zpersp;
	int     extsave;
	REAL    dummy[maxsec*maxext];
	int    ignore;
	REAL   yshift;
	int    numok;
	extern int pointsize;
	extern int ychar;
#ifdef EXT_OR_PROF
	extern int current_hull,numlin_overlay;
#endif
	extern int numbetw;
	int	   old_numbetw = numbetw;
	extern REAL ygslope;

	setranges(x1,x2,y1,y2);

	yshift = ychar / ygslope;

	pitch = pitchv;
	beta = tan(0.01745329*pitch);
	heel = heel0;
	sina = sind(heel);
	cosa = cosd(heel);
	heelv = heel0 + 180.0;
	perspp(0.0,0.0,0.0,pitch,rotn,heelv);

#ifdef EXT_OR_PROF
	if(shaded) {
		hidden_surface(side1,side2,endlin);
		if(shaded == 1) return;
		numbetw = 0;
	} else {
		set_GL_perspective();
	}
#else
	set_GL_perspective();
#endif

	/*    DRAW STEM    */

	(*colour)(3);
	if(disp >= 0.0 && inisec == 0) {
		ystem = yline[stemli][1];
		for(side = side1 ; side <= side2 ; side += 2.0) {
			(*newlin)();
			s1 = side*ystem;
			aa = 0.0;
			cc = 1.0;
			t = 1.0;
			il = 0;
			while(stsec[il++] > 0) ;
			for( ; il < numlin ; il++) {
				if(stsec[il] <= 0) {
					hullpa(0,il,aa,cc,&a,&hb,&c,&hd);
					s2 = yline[il][0] - xsect[0];
					while(t > -0.05) {
						zpersp = s2 + t*(a+t*hb);
						(*draw)(s1,zline[il][0]-t*(c+t*hd));
						t -= 0.1;
					}
					t = 0.9;
					tranpa(a,hb,c,hd,&aa,&cc);
				}
			}
		}
	}

	/*    DRAW ALL SECTIONS    */

	extsave = extlin;
	extlin = endlin;

	if(disp >= 0.0 && inisec == 0)
		is = 1;
	else
		is = inisec > 0 ? inisec : 1;

	for(i = is ; i <= lassec ; i++) {
		zpersp = fmin(xsect[i]);

		/*	SELECT PERSPECTIVE PLANE FOR THIS SECTION:    */

		if(secton[i]) {

			/*	DRAW THE PORT AND STARBOARD-HAND HALF-SECTIONS AS REQUIRED    */

			for(side = side1 ; side <= side2 ; side += 2.0) {
				(*colour)(2);
				drasec(i,(int) side,side,0);
				if(showframes) {
					(*newlin)();
					make_outline(i,outl_thickness,0.0,0,NULL,side);
				}
				numok = feq(side1,side2) || (rotn < 0.0 && side > 0.0)
					|| (rotn >= 0.0 && side < 0.0);
				if(numok && (shownumbers || shownames)) {
					(*move)(side*yline[0][i],zline[0][i] - yshift);
					show_numbers(i);
				}
			}
		}
	}
	extlin = extsave;

	/*    DRAW WATERLINES FOR PORT AND STARBOARD SIDES ...    */

	if(wate_int > 0.0) {
		(*colour)(4);
		offset_range(&maxoff,&minoff,0.0,wate_int);
		t = start_at_waterline ? wl : wate_int * ((int) (minoff/wate_int));
		for(side = side1 ; side <= side2 ; side += 2.0) plotw(side,t,wate_int,3);
	}

	/*	PLOT HULL DIAGONALS REQUESTED	*/

	if(diag_int > 0.0) {
		(*colour)(6);
		for(side = side1 ; side <= side2 ; side += 2.0)
			pltdia(diag_int,diag_angle,3,side);
	}

	/*	Plot buttock lines		*/

	if(butt_int > 0.0) {
		(*colour)(5);
		for(side = side1 ; side <= side2 ; side += 2.0)
			pltdia(butt_int,90.0,3,side);
	}

	/*    DRAW REQUIRED HULL LINES    */

	(*colour)(3);
#ifndef STUDENT
	tank = 0;
#endif
	s1 = side1;
	s2 = side2;
#ifdef EXT_OR_PROF
	if(current_hull != 0) endlin = numlin_overlay;
#endif

	for(il = 0 ; il < endlin ; il++) {
#ifndef STUDENT
		if(il == fl_line1[tank]) {
			if(fl_right[tank]) {
				s1 = 1.0;
				s2 = side2;
			}
			else {
				s1 = side1;
				s2 = -1.0;
			}
			tank++;
		}
#endif
		if(lineon[il]) draw_line(il,s1,s2,3,inisec,lassec,99,dummy,dummy,dummy,&ignore,&ignore,&ignore,1);
	}

#ifndef STUDENT

	/*	Draw transom		*/

	(*colour)(2);
	draw_transom(side1,side2,3);

	/*	Draw stringers		*/

	draw_stringers(side1,side2,3,999);

#endif
	if(showcentres) show_centres(TRUE,TRUE,TRUE,3);

	numbetw = old_numbetw;
}

/*	SELECT AXIS INCREMENT IN 1, 2, 3 OR 5-UNIT STEP	*/

REAL axincr(REAL ra)
{
	REAL range,factor,amant,Axincr;
	INT	ilog;

	if(ra > 0.0) {
		range = ra*1.5;
		ilog = log10((double) range);	/* 100 gives 2 */
		if(range < 1.0) ilog--;
		factor = 1.0;
		if(ilog > 0)
			while(ilog--) factor *= 10.0;/* 2 gives 100.0 */
		else
			while(ilog++) factor *= 0.1;

		if(factor > 0.0) {
			amant = range/factor; /* 100.0 gives 1.0 */
			if(amant <= 1.5 && factor > 10.0) {
				Axincr = 0.15;
			}
			else if(amant <= 2.0 && factor > 1.0) {
				Axincr = 0.2;
			}
			else if(amant <= 3.0 && factor > 1.0) {
				Axincr = 0.3;
			}
			else if(amant <= 4.0 && factor > 1.0) {
				Axincr = 0.4;
			}
			else if(amant <= 5.0) {
				Axincr = 0.5;
			}
			else if(amant <= 6.0 && factor > 1.0) {
				Axincr = 0.6;
			}
			else if(amant <= 8.0 && factor > 1.0) {
				Axincr = 0.8;
			}
			else {
				Axincr = 1.0;
			}
			Axincr *= factor;
		}
		else {
			Axincr = 1.0;
		}
	}
	else {
		Axincr = 1.0;
	}
	return(Axincr);
}

void show_centre(REAL x,REAL y,REAL z,char *title,int mode);

void show_centres(int cg,int cf,int m,int mode)
{
	(*colour)(1);
	if(cg) show_centre(xcofm,0.0,zcofm,"CG",mode); /* CG */
	if(heel == 0.0) {
		if(m) show_centre(xcofm,0.0,zmeta,"M",mode); /* M */
		if(cf) show_centre(xlcf,0.0,wl - beta*xlcf,"CF",mode);/* CF */
	}
	if(heel != 90.0)
		show_centre(xcofm,(gz+sina*(zcofb-zcofm))/cosa,zcofb,"B",mode);
}

void show_centre(REAL x,REAL y,REAL z,char *title,int mode)
{
	REAL rad = min(fabs(xmax-xmin),fabs(ymax-ymin))*0.02;
	(*newlin)();
	pltsel(x,y,z+rad,mode);
	pltsel(x,y,z-rad,mode);
	(*newlin)();
	pltsel(x+rad,y,z,mode);
	pltsel(x-rad,y,z,mode);
	(*newlin)();
	pltsel(x,y+rad,z,mode);
	pltsel(x,y-rad,z,mode);
	(*newlin)();
	pltsel(x+rad,y+rad,z+rad,mode);
	(*plstr)(title);
}

void check_balance(void)
{
	void finish_stats(void);
	extern int balanced;
	REAL calcdisp;
	if(showcentres && !balanced) {
		balanc(&calcdisp,1);
		if(calcdisp < 0.0) return;
		balanced = 1;
		finish_stats();
	}
	return;
}

void show_numbers(int i)
{
	char text[20];

	if(i == 0 && surfacemode) return;

	if(!*sectname[i]) sprintf(sectname[i],"%d",i);
	if(shownumbers) {
		if(shownames)
			sprintf(text,"%d: %s",i,sectname[i]);
		else
		    sprintf(text,"%d",i);
		plstr(text);
	}
	else if(shownames) {
		plstr(sectname[i]);
	}
}

