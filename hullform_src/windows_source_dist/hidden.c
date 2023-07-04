/* Hullform component - hidden.c
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

#ifdef EXT_OR_PROF

#ifdef linux

#include <X11/X.h>
#include <X11/Xlib.h>

#else

extern HDC hDC;

#endif
void wmftri(int x0,int y0,int x1,int y1,int x2,int y2,int pen,int brush);
extern int device;

void get_int_angle(int j,REAL *c,REAL *s);
/* stem-curve intercept angle function */
int find_inters_point(int j,REAL *x12s,REAL *y12s,REAL *z12s);
#define	radian	0.01745329

REAL	*xv1 = NULL,
*yv1 = NULL,
*zv1 = NULL;	/* first vertex of triangle	*/

REAL	*xv2 = NULL,
*yv2 = NULL,
*zv2 = NULL;	/* second vertex of triangle	*/

REAL	*xv3 = NULL,
*yv3 = NULL,
*zv3 = NULL;	/* third vertex of triangle	*/

REAL	*dist;		/* distance from viewer to triangle	*/
int	*trindx;	/* index to sorted triangles		*/
REAL	floatn;		/* number of triangles per surface	*/
int	plotright,plotleft;
extern	REAL	sinpp,cospp;
extern	REAL	yview,zview;
extern	REAL	axx,axy,axz,ayx,ayy,ayz,azx,azy,azz;
extern	REAL	pp;
extern	REAL	rotn,heelv;
extern	REAL	ypersp,zpersp;
extern	int	persp;
extern	int	numbetwl;
int	jtr;

int	numshade;
int	shade;
REAL	cshade;

void wmfdeleteobject(int);
void wmfsetpen(int width,int style,int r,int g,int b,int *pen,
	int *brush,int deleteold);

void hidden_surface(REAL side1,REAL side2,int endlin)
{
	extern	int	surfsp;		/* space needed for surface data */
	int		i,i0,ii,is,j,k,l,n,nn,jj,in;
	REAL	ystem = surfacemode ? 0.0 : yline[stemli][1];
#ifndef STUDENT
	REAL	c,s,z0;
	int		tank;
	REAL 	c11,c21;
#endif
	int		bottom;
	int		j0;
	int		at_line;

	REAL x11,y11,z11,x12,y12,z12,x21,y21,z21,x22,y22,z22;
	REAL x12s,y12s,z12s,x22s,y22s,z22s,c12s,c22s;
	REAL c12,c22;

	REAL x3,y3,z3;
	REAL xv,yv,zv;

	REAL yy0,zz0,xx0,xx1,xx2,xx3,yy1,yy2,yy3,zz1,zz2,zz3;
	REAL z1,z2;

	extern	REAL	l_azim,l_elev;
#ifndef STUDENT
	extern int	numtran;
#endif
	extern int numbewtl;

	REAL l_x,l_y,l_z;

	REAL a1,hb1,c1,hd1,aa1,cc1;
	REAL a2,hb2 = 0.0,c2,hd2 = 0.0,aa2,cc2;
	REAL t;
	REAL con;
#ifndef STUDENT
	REAL r1,r2;
	REAL len;
	REAL t1,dt1;
	REAL t2,dt2;
	REAL t1max,t2max;
#endif
	int npsec;
	int *seq;	/* sequence of section indices to use */
	REAL dt;
	int p0,p1,p2,p3,p4,p5;
	extern int ymaxi;
	extern int drawcol;
	char r,g,b;
	int left,right;

#define maxshade 40

#ifdef linux
	XPoint point[3];
	XColor curr_def;
	extern Display *display;
	Colormap default_cmap;
	extern int screen_num;
	extern GC gc;
	extern Window win;
	long colhi,collo,col;
	extern long scrcolour[];
#else
	POINT point[3];
	HBRUSH startbrush;
	HPEN startpen;
	COLORREF colhi,collo,col;
	extern COLORREF scrcolour[];
	extern int solidshade;
#endif
	HBRUSH hBrush[maxshade];
	HPEN hPen[maxshade];
	unsigned redhi,bluehi,greenhi;
	unsigned redlo,bluelo,greenlo;
	int assigned = 0;
	int ntr;
	REAL x1,y1,x2,y2;

	void glsurf(REAL s1,REAL s2,int l);

	if(!hardcopy && scrdev == 0) {
		glsurf(side1,side2,endlin);
		return;
	}

#ifdef linux
	default_cmap = DefaultColormap(display,screen_num);
#endif

	/*	Prevent shading attempts on inappropriate devices	*/

	if(hardcopy && device != SYSTEMPRINTER && device != METAFILE && device != CLIPBOARD) return;

	x21 = 0.0;
	y21 = 0.0;
	z21 = 0.0;
	x22 = 0.0;
	y22 = 0.0;
	z22 = 0.0;

	floatn = (float) (numbetwl+1);
	plotright = fneg(side1);
	plotleft  = fpos(side2);

	waitcursor();

	colhi = scrcolour[7];
	collo = scrcolour[8];
	redhi   = GetRValue(colhi);
	greenhi = GetGValue(colhi);
	bluehi  = GetBValue(colhi);
	redlo   = GetRValue(collo);
	greenlo = GetGValue(collo);
	bluelo  = GetBValue(collo);
	numshade = maxshade;

	for(j = 0 ; j < numshade ; j++) {
		r = (char) ((redhi  *j + redlo  *(numshade-j))/numshade);
		g = (char) ((greenhi*j + greenlo*(numshade-j))/numshade);
		b = (char) ((bluehi *j + bluelo *(numshade-j))/numshade);
		if(device == METAFILE) {
			wmfsetpen(0,PS_NULL,(int) r,(int) g,(int) b,
				(int *) &hPen[j],(int *) &hBrush[j],0);
		}
		else {
#ifdef linux
			curr_def.red   = r << 8;
			curr_def.green = g << 8;
			curr_def.blue  = b << 8;
			curr_def.flags = 7;
			if(!XAllocColor(display,default_cmap,&curr_def))
				l = 0;
			else
			    l = curr_def.pixel;
			hPen[j] = (HPEN) l;
#else
			col = RGB(r,g,b);
			if(device < METAFILE && !hardcopy && solidshade)
				col = GetNearestColor(hDC,col);
			if((hBrush[j] = (HBRUSH) CreateSolidBrush(col)) == NULL ||
				    (hPen[j] = (HPEN) CreatePen(PS_NULL,1,col)) == NULL) {
				message("Cannot allocate shades");
				assigned = j;
				goto abort_init;
			}
#endif
		}
	}
	assigned = numshade;
	cshade = (REAL) numshade;
#ifndef linux
	SetPolyFillMode(hDC,WINDING);
	startbrush = (HBRUSH) SelectObject(hDC,hBrush[0]);
	startpen   = (HPEN) SelectObject(hDC,hPen[0]);
#endif

	/*	set up angle transformation coefficients	*/

	perspp(0.0,0.0,0.0,pitch,rotn,heelv);

	npsec = (endlin - 1) * (numbetwl+1) * 4 + 4;/* triangles found on both sides */

	/*	lighting parameters */

	x1 = cosd(l_elev);
	l_x = x1 * sind(l_azim);
	l_z = x1 * cosd(l_azim);
	l_y = sind(l_elev);

	/*	step 1:		obtain working memory				*/

	surfsp = sizeof(*xv1) * npsec;
	if(	!memavail((void far *) &trindx,npsec * sizeof(int)) ||
	#ifndef STUDENT
	    !memavail((void far *) &dist,max(maxsec+1+numtran,npsec) * sizeof(*dist)) ||
		    !memavail((void far *) &seq ,max(maxsec+1+numtran,npsec) * sizeof(*seq)) ||
	#else
	    !memavail((void far *) &dist,max(maxsec+1,npsec) * sizeof(*dist)) ||
		    !memavail((void far *) &seq ,max(maxsec+1,npsec) * sizeof(*seq)) ||
	#endif
	    !memavail((void far *) &xv1,surfsp) ||
		    !memavail((void far *) &yv1,surfsp) ||
		    !memavail((void far *) &zv1,surfsp) ||
		    !memavail((void far *) &xv2,surfsp) ||
		    !memavail((void far *) &yv2,surfsp) ||
		    !memavail((void far *) &zv2,surfsp) ||
		    !memavail((void far *) &xv3,surfsp) ||
		    !memavail((void far *) &yv3,surfsp) ||
		    !memavail((void far *) &zv3,surfsp) ) {

		message("Insufficient memory to develop image");
		return;
	}
	surfsp *= 9;
	surfsp += npsec * sizeof(int) + max(maxsec+1,npsec) * sizeof(*xv1);

	/*	step 2:		define triangles				*/

	persp = 1;	/* force perspective mode */

	/*	sort sections into plotting order				*/

#ifndef STUDENT
	/*	Firstly, define distances to transom surfaces		*/

	for(in = 0 ; in < numtran-1 ; in++) {
		y1 = 0.0;				/* use hull axis */
		z1 = 0.5*(ztran[in+1]+ztran[in]);
		x1 = (ztransom-z1)*stransom/ctransom;
		if(x1 < 0.0) x1 = 0.0;
		x1 = xsect[count-1] - x1;
		transform(&x1,&y1,&z1);
		dist[in] = dist_to(x1,y1,z1);
		seq[in] = in;
	}
#else
	in = 0;
#endif

	/*	Next, define distances to centres of sections		*/

	i0 = numbetwl + 1;
	for(j = count-1 ; j > surfacemode ; j--) {
		x1 = 0.5*(xsect[j] + xsect[j-1]);
		y1 = 0.0;
		z1 = 0.5*(zline[stemli][j] + zline[stemli][j-1]);
		z2 = 0.5*(zline[0][j] + zline[0][j-1]);
#ifndef STUDENT
		if(transom) {
			z0 = (x1*ctransom - dtransom) / stransom;
			if(stransom > 0.0) {
				if(z0 > z2) z2 = z0;
			}
			else {
				if(z0 < z1) z1 = z0;
			}
		}
#endif
		z1 = 0.5*(z1 + z2);
		transform(&x1,&y1,&z1);
		dist[in] = dist_to(x1,y1,z1);
		seq[in] = in;
		in++;
	}

	/*  evaluate stem position, to use in case there is a rounded stem to plot */

	x1 = xsect[0] - dxstem();
	y1 = 0.0;
	z1 = 0.0;
	transform(&x1,&y1,&z1);
	dist[in] = dist_to(x1,y1,z1);
	seq[in] = in;

	/*	Index of dist[] and seq[] ranges from 0 to count-1,		*/
	/*	corresponding to the stem plus count-1 pairs of sections.	*/

	/*	Then sort the section order by distance				*/

	for(i = surfacemode ; i < in ; i++) {
		for(j = in-1 ; j >= i ; j--) {
			ii = seq[j+1];
			is = seq[j];
			if(fgt(dist[ii],dist[is])) {
				seq[j+1] = is;
				seq[j]   = ii;
			}
		}
	}

	/*	Plot the surfaces between each pair of sections in turn		*/

#ifndef STUDENT
	for(i0 = surfacemode ; i0 <= in ; i0++) {
#else
		for(i0 = surfacemode ; i0 < in ; i0++) {
#endif
			jtr = 0;
			i = seq[i0];

			n = 0;		/* to be number of triangles found */
			nn = npsec-1;	/* index to right-side triangles */

			right = plotright;
			left  = plotleft;

			a1 = 0.0;
			c1 = 1.0;
			a2 = 0.0;
			c2 = 1.0;
			aa1 = 0.0;
			cc1 = 1.0;
			aa2 = 0.0;
			cc2 = 1.0;

#ifndef STUDENT
			if(i == in) {		/* rounded stem */

				for(j = 1 ; j < numlin ; j++) {
					x3 = xsect[1];	 /* position of nearest transverse section */

					r1 = radstem[j-1];		/* radius of line at stem */
					r2 = radstem[j];		/* radius of line at stem */
					x1 = xsect[0];
					x2 = xsect[0];
					if(!surfacemode) {
						x1 -= yline[j-1][0];
						x2 -= yline[j][0];		/* x-end of line */
					}
					c1 = x1 + r1;			/* x-centre of circle */
					c2 = x2 + r2;			/* x-centre of circle */

					get_int_angle(j-1,&t1max,&dt1);
					get_int_angle(j  ,&t2max,&dt2);
					t1max = atan2(dt1,t1max);
					t2max = atan2(dt2,t2max);

					dt1 = t1max / floatn;
					dt2 = t2max / floatn;
					for(t1 = 0.0, t2 = 0.0, i = 0 ; i <= numbetwl+1 ; t1 += dt1, t2 += dt2, i++) {

						x11 = x21;
						y11 = y21;
						z11 = z21;

						x12 = x22;
						y12 = y22;
						z12 = z22;

						/*	Calculate upper line co-ordinates	*/

						s = sin(t1);
						c = fsqr0(1.0 - s*s);
						len = r1*c;
						x21 = c1 - len;
						y21 = ystem + r1*s;
						z21 = zline[j-1][0];
						if(x3 != x1) z21 += (x21-x1)/(x3-x1)*(zline[j-1][1]-zline[j-1][0]);

						/*	Calculate lower line co-ordinates	*/

						s = sin(t2);
						c = fsqr0(1.0 - s*s);
						len = r2*c;
						x22 = c2 - len;
						y22 = fadd(ystem,fmul(r2,s));
						z22 = zline[j][0];
						if(x3 != x2) z22 += (x22-x2)/(x3-x2)*(zline[j][1] - zline[j][0]);

						if(t1 != 0.0) {
							if(left) set_tri(x11,y11,z11,x12,y12,z12,
								x21,y21,z21,x22,y22,z22,&n,1);

							if(right) set_tri(x11,-y11,z11,x12,-y12,z12,
								x21,-y21,z21,x22,-y22,z22,&nn,-1);
						}
					}	/* end loop around curve */
				}		/* end loop through hull lines */

			}
			else if(i < numtran-1) {	/* transom */

				ntr = rtransom != 0.0 ? numbetwl+1 : 1;
				con = ntr;
				r2 = rtransom*rtransom;
				y12 = ytran[i] - ystem;
				x12 = xtran[i];
				z12 = ztran[i];
				x22 = xtran[i+1];
				y22 = ytran[i+1] - ystem;
				z22 = ztran[i+1];
				dt1 = y12 / con;
				t1 = x12 - fsqr0(r2 - y12*y12) / ctransom;
				dt2 = y22 / con;
				t2 = x22 - fsqr0(r2 - y22*y22) / ctransom;
				z11 = z12;
				z21 = z22;
				xx0 = 0.5*dt1;
				if(fabs(dt1) > 0.001) {
					do {
						x11 = x12;
						y11 = y12;

						x21 = x22;
						y21 = y22;

						y12 -= dt1;
						x12 = t1 + fsqr0(r2 - y12*y12) / ctransom;

						y22 -= dt2;
						x22 = t2 + fsqr0(r2 - y22*y22) / ctransom;

						if(left)	set_tri(x11,y11+ystem,z11,x12,y12+ystem,z12,
							x21,y21+ystem,z21,x22,y22+ystem,z22,&n,1);
						if(right)	set_tri(x11,-y11-ystem,z11,x12,-y12-ystem,z12,
							x21,-y21-ystem,z21,x22,-y22-ystem,z22,&nn,-1);
					}
					while(--ntr > 0);
				}

			}
			else
#endif
			    if((i = in - i - 1) >= 0) {

				/*	Normal sections					*/
				x1 = xsect[i];
				x2 = xsect[i+1];

				/*	Start values					*/

				/*	The "s" parameters are pre-transom values	*/

				set_xyz(&x12s,&y12s,&z12s,&c12s,&x22s,&y22s,&z22s,&c22s,i,0);

				/*
				The ordering of points on the hull surface is:

				(i)		       (i+1)

				11 ------------------- 21  (j-1)
				\             __..--''	\
				\     __..--''   	\
				\ --''               	\
				12 ------------------- 22  (j)

				*/
#ifndef STUDENT
				tank = 0;
#endif

				t = 0.0;	/* initialise curve parameters to force curve	*/
				at_line = TRUE;/* calculation when plotting loop commences	*/
				j = 0;

				/*	Do loop while line index does not reach endlin and position	*/
				/*	does not meet lower line					*/

				while(1) {
					if(at_line) {
next_pair:
						if(j >= endlin-1) break;

						/*	Scan down to next active line between these sections, watching	*/
						/*	for passage across any tank on the way.				*/

						j0 = j;
#ifndef STUDENT
						if(tanknext(i,&j,&tank,endlin)) {
							starttank(i,j,&tank,&right,&left,
								&a1,&hb1,&c1,&hd1,
								&a2,&hb2,&c2,&hd2,
								&x12s,&y12s,&z12s,&c12s,
								&x22s,&y22s,&z22s,&c22s);
							dt = fdiv(1.0,floatn);
						}
						else
#else
							j++;
#endif
						if(j < endlin) {
							hullpa(i,j,aa1,cc1,&a1,&hb1,&c1,&hd1);
							tranpa(a2,hb2,c2,hd2,&aa2,&cc2);
							hullpa(i+1,j,aa2,cc2,&a2,&hb2,&c2,&hd2);
							tranpa(a1,hb1,c1,hd1,&aa1,&cc1);

							for(jj = 1 ; jj < endlin-1 ; jj++) {
								if(jj != j) {
									if(coincident(i,j,jj) &&
										    coincident(i,j-1,jj+1)) {
										set_xyz(&x12s,&y12s,&z12s,&c12s,
											&x22s,&y22s,&z22s,&c22s,i,j);
										goto next_pair;
									}
								}
							}
							con = (float) (j - j0);
							dt = fdiv(1.0,fmul(con,floatn));
						}
						else {
							break;
						}
						t = fadd(t,1.0);
					}
					t -= dt;
					jtr++;	/* points to lower of a pair of lines */
					at_line = (t+t) < dt;

					/*	(y11,z11) is upper corner of triangle on previous section	*/

					x11 = x12s;
					y11 = y12s;
					z11 = z12s;
#ifndef STUDENT
					c11 = c12s;
#endif

					/*	(y21,z21) is upper corner of triangle on next section		*/

					x21 = x22s;
					y21 = y22s;
					z21 = z22s;
#ifndef STUDENT
					c21 = c22s;
#endif

					/*	(x22,y22,z22) is lower corner of triangle on next section	*/

					x22 = x2;
					y22 = fadd(yline[j][i+1],fmul(t,fadd(a2,fmul(t,hb2))));
					z22 = fsub(zline[j][i+1],fmul(t,fadd(c2,fmul(t,hd2))));
#ifndef STUDENT
					c22 = fsub(x22,xtran[jtr]);
#else
					c22 = fsub(x22,xsect[count-1]);
#endif
					x22s = x22;
					y22s = y22;
					z22s = z22;
					c22s = c22;

					/*	(x12,y12,z12) is lower corner of triangle on previous section	*/

					if(!surfacemode && i == 0) {

						ii = find_inters_point(j-1,&xx0,&yy0,&zz0);
						jj = find_inters_point(j,&xx1,&yy1,&zz1);
						if(ii || jj) {
							x12 = fadd(xx1,fmul(t,fsub(xx0,xx1)));
							y12 = fadd(yy1,fmul(t,fsub(yy0,yy1)));
							z12 = fadd(zz1,fmul(t,fsub(zz0,zz1)));
						}
						else {
							y12 = fadd(yline[j][0],fmul(t,fadd(a1,fmul(t,hb1))));
							x12 = fsub(xsect[0],y12);
							y12 = ystem;
							z12 = fsub(zline[j][0],fmul(t,fadd(c1,fmul(t,hd1))));
						}
					}
					else {
						x12 = x1;
						y12 = fadd(yline[j][i],fmul(t,fadd(a1,fmul(t,hb1))));
						z12 = fsub(zline[j][i],fmul(t,fadd(c1,fmul(t,hd1))));
					}
#ifndef STUDENT
					c12 = fsub(x12,xtran[jtr]);
#else
					c12 = fsub(x12,xsect[count-1]);
#endif

					x12s = x12;
					y12s = y12;
					z12s = z12;
					c12s = c12;

#ifndef STUDENT

					/*	Now check for transom effects					*/

					if(fpos(c21)) {			/* transom inside point 21 */
						if(fpos(c22)) {		/* transom inside point 22 */
							if(fpos(c11)) {		/* transom inside point 11 */
								if(fpos(c12)) {	/* transom inside point 12 */
									continue;	/* no triangles to fill	*/

								}
								else {	/* lower left-corner triangle */
									(void) reinterp(&y11,&z11,&c11,y12,z12,c12);
									x22 = xtran[jtr];
									y22 = ytran[jtr];
									z22 = ztran[jtr];
									c22 = 0.0;
									y21 = y22;
									z21 = z22;
									x21 = x22;
									c21 = 0.0;
								}

							}
							else {
								x21 = xtran[jtr-1];
								y21 = ytran[jtr-1];
								z21 = ztran[jtr-1];
								c21 = 0.0;
								if(fpos(c12)) {/* upper left-corner triangle */
									(void) reinterp(&y12,&z12,&c12,y11,z11,c11);
									y22 = y21;
									z22 = z21;
									x22 = x21;
									c22 = 0.0;
								}
								else {	/* left-side trapezium */
									x22 = xtran[jtr];
									y22 = ytran[jtr];
									z22 = ztran[jtr];
									c22 = 0.0;
								}
							}

						}
						else {	/* inside point 21, outside point 22 */

							if(fpos(c12)){	/* lower right corner triangle */
								(void) reinterp(&y21,&z21,&c21,y22,z22,c22);
								x12 = xtran[jtr-1];
								y12 = ytran[jtr-1];
								z12 = ztran[jtr-1];
								c12 = 0.0;
								x11 = x12;
								y11 = y12;
								z11 = z12;
								c11 = 0.0;
							}
							else if(fpos(c11)) {/* lower side trapezium */
								(void) reinterp(&y21,&z21,&c21,y22,z22,c22);
								(void) reinterp(&y11,&z11,&c11,y12,z12,c12);

							}
							else {/* lower left pentagon */
								x21 = xtran[jtr-1];
								y21 = ytran[jtr-1];
								z21 = ztran[jtr-1];
								c21 = 0.0;
								x22s = x22;
								y22s = y22;
								z22s = z22;
								c22s = c22;
							}
						}
					}
					else {		/* transom outside point 21 */
						if(fpos(c22)) {	/* transom inside point 22 */
							if(fzer(c21)) {
								x22 = xtran[jtr];
								y22 = ytran[jtr];
								z22 = ztran[jtr];
								c22 = 0.0;
							}
							else {
								(void) reinterp(&y22,&z22,&c22,y21,z21,c21);
							}

							if(fpos(c12)) {/* transom inside point 12 */
								if(fpos(c11)) {/* upper right triangle */
									x11 = xtran[jtr-1];
									y11 = ytran[jtr-1];
									z11 = ztran[jtr-1];
									c11 = 0.0;
									y12 = y11;
									z12 = z11;
									x12 = x11;
									c12 = 0.0;
								}
								else {	/* top side trapezium */
									(void) reinterp(&y12,&z12,&c12,y11,z11,c11);
								}
							}
							else {/* top left pentagon covered by first "reinterp" */
								x22s = x22;
								y22s = y22;
								z22s = z22;
								c22s = c22;
							}
						}
						else {	/* transom outside point 22 */
							;	/* no changes needed */
						}
					}
#endif

					/*	End definition of triangles for this section pair		*/

					at_line = flt(fadd(t,t),dt);
					bottom = at_line && j == numlin-1;

					if(left) {
						set_tri(x11,y11,z11,
							x12,y12,z12,
							x21,y21,z21,
							x22,y22,z22,
							&n,1);

						/*	hull bottom				*/

						if(bottom) set_tri(x12,y12,z12,
							x12,0.0,z12,
							x22,y22,z22,
							x22,0.0,z22,
							&n,1);
					}

					if(right) {
						set_tri(x11,fmin(y11),z11,
							x12,fmin(y12),z12,
							x21,fmin(y21),z21,
							x22,fmin(y22),z22,
							&nn,-1);

						/*	hull bottom				*/

						if(bottom) set_tri(x12,fmin(y12),z12,
							x12,0.0,z12,
							x22,fmin(y22),z22,
							x22,0.0,z22,
							&nn,-1);
					}	/* end right-side conditional */
				}		/* end triangle-defining loop */

			}	/* end round stem / normal line option */

			/*	End triangle definition.  Now move down right-side triangles	*/
			/*	to meet left-side triangles in a contiguous array		*/

			for(j = nn+1 ; j < npsec ; j++) {
				xv1[n] = xv1[j];
				xv2[n] = xv2[j];
				xv3[n] = xv3[j];

				yv1[n] = yv1[j];
				yv2[n] = yv2[j];
				yv3[n] = yv3[j];

				zv1[n] = zv1[j];
				zv2[n] = zv2[j];
				zv3[n] = zv3[j];

				n++;
			}

			/*	transform to screen coordinates, calculate distances from	*/
			/*	viewing point, and initialise sorting index table		*/

			for(j = 0 ; j < n ; j++) {

				transform(&xv1[j],&yv1[j],&zv1[j]);
				transform(&xv2[j],&yv2[j],&zv2[j]);
				transform(&xv3[j],&yv3[j],&zv3[j]);

				dist[j] = dist_to(
					fmul(0.25,fadd(fadd(xv1[j],xv2[j]),fmul(2.0,xv3[j]))),
					fmul(0.25,fadd(fadd(yv1[j],yv2[j]),fmul(2.0,yv3[j]))),
					fmul(0.25,fadd(fadd(zv1[j],zv2[j]),fmul(2.0,zv3[j])))
					    );

				if(fpos(rotn))	/* if hull turned to starboard ...  */
					trindx[j] = j;		/* default order up array   */
				else		/* if hull turned to port ...       */
				trindx[j] = (n-1) - j;	/* default order down array */

			}

			/*	sort the triangles					*/

			for(j = 1 ; j < n ; j++) {
				for(k = n-1 ; k >= j ; k--) {
					is = trindx[k-1];
					ii = trindx[k];
					if(flt(dist[is],dist[ii])) {
						trindx[k] = is;
						trindx[k-1] = ii;
					}
				}
			}

			for(l = 0 ; l < n ; l++) {
				k = trindx[l];

				/*	find normal vector		*/

				xx1 = xv1[k];
				yy1 = yv1[k];
				zz1 = zv1[k];
				xx2 = xv2[k];
				yy2 = yv2[k];
				zz2 = zv2[k];
				xx3 = xv3[k];
				yy3 = yv3[k];
				zz3 = zv3[k];

				x1 = fsub(xx2,xx1);
				x2 = fsub(xx3,xx1);
				y1 = fsub(yy2,yy1);
				y2 = fsub(yy3,yy1);
				z1 = fsub(zz2,zz1);
				z2 = fsub(zz3,zz1);

				xv = xx1;
				yv = fsub(yy1,yview);
				zv = fsub(zz1,zview);

				/*	translate and rescale co-ordinates			*/

				screen_transform(&xx1,&yy1,&zz1);
				screen_transform(&xx2,&yy2,&zz2);
				screen_transform(&xx3,&yy3,&zz3);

				/*	step 6: draw each triangle, filling				*/
				/*	according to its orientation					*/

				/*	corners of triangle on screen			*/

				p0 = (int) yy1;
				p1 = ymaxi - (int) zz1;
				p2 = (int) yy2;
				p3 = ymaxi - (int) zz2;
				p4 = (int) yy3;
				p5 = ymaxi - (int) zz3;

				/*	area of triangle on screen: positive if sequence anticlockwise	*/

				j = ((long) p1)*(p2-p4) + ((long) p3)*(p4-p0) + ((long) p5)*(p0-p2);

				if(j != 0) {

					/*	if area nonzero, find surface normal vector		*/

					x3 = fsub(fmul(y2,z1),fmul(y1,z2));
					y3 = fsub(fmul(z2,x1),fmul(z1,x2));
					z3 = fsub(fmul(x2,y1),fmul(x1,y2));

					a1 = fsqr0(fadd(fadd(fmul(x3,x3),fmul(y3,y3)),fmul(z3,z3)));
					/* (length of vector is cross product) */

					/*	"xx1" evaluates the orientation of the surface normal, relative	*/
					/*	to the line to the light source					*/

					xx1 = fdiv(fadd(fmul(x3,l_x),fadd(fmul(y3,l_y),fmul(z3,l_z))),a1);

					/*	"xx1" evaluates the orientation of the surface normal, relative	*/
					/*	to the line from the viewer's position				*/

					xx2 = fdiv(fadd(fmul(x3,xv),fadd(fmul(y3,yv),fmul(z3,zv))),a1);

					/*	"xx2" is of the opposite sign to "xx1" if the viewer sees the	*/
					/*	illuminated side of the surface					*/

					if(fpos(xx2) == fpos(xx1)) {
						xx1 = 0.0;
						shade = 0;
					}
					else {
						xx1 = fabf(xx1);
						xx2 = fmul(xx1,cshade);
						shade = xx2;
						if(shade >= numshade) shade = numshade-1;
					}

					if(device == METAFILE) {
						wmftri(p0,p1,p2,p3,p4,p5,
							(int) hPen[shade],(int) hBrush[shade]);
					}
					else { /* Printer, screen or memory metafile only left */
						point[0].x = p0;
						point[0].y = p1;
						point[1].x = p2;
						point[1].y = p3;
						point[2].x = p4;
						point[2].y = p5;
#ifdef linux
						XSetForeground(display,gc,(int) hPen[shade]);
						XFillPolygon(display,win,gc,point,3,
							Nonconvex,CoordModeOrigin);
#else
						SetPolyFillMode(hDC,WINDING);
						if((HGDIOBJ) SelectObject(hDC,hBrush[shade]) == NULL ||
							    (HGDIOBJ) SelectObject(hDC,hPen[shade]) == NULL) {
							message("Could not select shade in\nhidden surface routine");
							goto quit_point;
						}
						Polygon(hDC,point,3);
#endif
					}
				}		/* end area nonzero conditional */
			}		/* end loop over requested sides */
		}			/* end loop along hull */

quit_point:

		memfree(trindx);
		memfree(dist);
		memfree(seq);
		memfree(xv1);
		memfree(yv1);
		memfree(zv1);
		memfree(xv2);
		memfree(yv2);
		memfree(zv2);
		memfree(xv3);
		memfree(yv3);
		memfree(zv3);

abort_init:
#ifndef linux
		if((HGDIOBJ) SelectObject(hDC,startbrush) == NULL ||
			    (HGDIOBJ) SelectObject(hDC,startpen) == NULL) {
			message("Could not re-select default brush and pen\nin hidden surface routine");
		}

		for(j = 0 ; j < assigned ; j++) {
			if(device == METAFILE) {
				wmfdeleteobject((int) hPen[j]);
				wmfdeleteobject((int) hBrush[j]);
			}
			else {
				if(!DeleteObject(hBrush[j]) || !DeleteObject(hPen[j]))
					message("Could not clean up shades\nin hidden surface routine");
			}
		}
#else
		if(device == METAFILE) {
			for(j = 0 ; j < assigned ; j++) {
				wmfdeleteobject((int) hPen[j]);
				wmfdeleteobject((int) hBrush[j]);
			}
		}
#endif
		arrowcursor();
	}

#endif
