/* Hullform component - graphsub.c
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

void seteditranges(REAL x);

void GLin(void);

#ifdef linux
#undef tc
#include <GL/gl.h>
#include <GL/glu.h>
#else
#include <GL\gl.h>
#include <GL\glu.h>
#endif

extern REAL sinaz,cosel;

/*************************************************************************	*/

/*	define parameters for plotting within a rectangular region	*/

/*************************************************************************	*/

extern REAL xmax,xmin,ymax,ymin,xbox,ybox;

void box(REAL x1,REAL y1)
{
	extern REAL	xbox,ybox;
	xbox = x1;
	ybox = y1;
	maketrans();
}

void setranges(REAL xl,REAL xr,REAL yb,REAL yt)
{
	extern REAL	xmin,xmax,ymin,ymax;	 /* image limits, perhaps zoomed */
	extern REAL	xmin0,xmax0,ymin0,ymax0; /* image limits unzoomed */

	if(fpos(posdir)) {			/* if stem right */
		xmin = xl;
		xmax = xr;
	}
	else {				/* if stem left */
		xmax = xr;
		xmin = xl;
	}

	ymin = yb;
	ymax = yt;

	xmin0 = xmin;		/* "0" suffix is non-zoomed limit */
	xmax0 = xmax;
	ymin0 = ymin;
	ymax0 = ymax;
	maketrans();
}

void maketrans()
{
	extern	REAL	xgorigin,ygorigin;
	extern	REAL	xgslope,ygslope;
	extern	REAL	xmin,xmax;
	extern	REAL	ymin,ymax;
	extern	REAL	Xwidth,Xfactor,Yfactor;
	extern	REAL	hardcopy_Xwidth,hardcopy_Ywidth;
	REAL	Ywidth;
	extern	int	device;
	extern	int	fixed_scale;
	extern	int scaled;
	REAL	xw;
	extern int xright,xleft,ybottom,ytop;

	if(hardcopy) {
		xw = xmax - xmin;
		if(xw == 0.0) {
			message(" X picture limits invalid");
			return;
		}
		if(!fixed_scale)
			Xwidth = xw;
		else
			Xwidth = hardcopy_Xwidth;
		Ywidth = (ymax - ymin)/xw * Xwidth;
		xgslope = xbox / Xwidth;
		if(Ywidth == 0.0) {
			message(" Y picture limits invalid");
			return;
		}
		ygslope = ybox / Ywidth;
		if(device != 7) {	/* fixed scaling for metafiles */
			xgslope *= Xfactor;
			ygslope *= Yfactor;
		}
	}
	else {
		Xwidth = xmax - xmin;
		Ywidth = ymax - ymin;
		if(Xwidth == 0.0) Xwidth = 0.01;
		if(Ywidth == 0.0) Ywidth = 0.01;
		xgslope = xbox / Xwidth;
		ygslope = ybox / Ywidth;
		if(xgslope == 0.0) xgslope = 1.0;
		if(ygslope == 0.0) ygslope = 1.0;
	}
	xgorigin = - xmin * xgslope;
	ygorigin = - ymin * ygslope;
	if(scrdev == 0) {
		glEnd();
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho((GLdouble) xmin,(GLdouble) xmax,(GLdouble) ymin,(GLdouble) ymax,(GLdouble) -10000.0,(GLdouble) 10000.0);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
	}
}

/*************************************************************************	*/

/*	perspective translation routines	*/

/*************************************************************************	*/

/*	define perspective projection plane	*/

void perspp(REAL xp,REAL yp,REAL zp,REAL ax,REAL ay,REAL az)
{
	extern	REAL	ypersp,zpersp;
	extern	REAL	xpersp;		/* actually, now a dummy */
	extern	REAL	angx,angy,angz;
	extern	INT	persp;
	REAL	sx,sy,sz,cx,cy,cz;
	extern	REAL	axx,axy,axz,ayx,ayy,ayz,azx,azy,azz;

	xpersp = xp;
	ypersp = yp;
	zpersp = zp;

	persp = 1;

	if(angx != ax || angy != ay || angz != az) {
		angx = ax;
		angy = ay;
		angz = az;

		cx = cosd(angx);
		cy = cosd(angy);
		cz = cosd(angz);
		sx = sind(angx);
		sy = sind(angy);
		sz = sind(angz);

		axx =  cy*cz;
		ayx =     sz;
		azx =  sy*cz;

		axy = -sx*sy - cx*cy*sz;
		ayy = cx*cz;
		azy =  sx*cy - cx*sy*sz;

		axz = sx*cy*sz - cx*sy;
		ayz = -sx*cz;
		azz = sx*sy*sz + cx*cy;
	}
}

REAL sind(REAL x)
{
	return((REAL) sin(0.01745329* ( (double) x) ) );
}

REAL cosd(REAL y)
{
	return((REAL) cos(0.01745329* ( (double) y) ) );
}

/*	activate or deactivate perspective viewing	*/

void perset(INT l)
{
	extern	INT	persp;
	persp = l;
}

/*	Plot waterline element			*/

void plotp(REAL prevc,REAL currc,REAL prevy,REAL curry,REAL prevz,REAL currz,
	REAL side,INT mode,INT pltopp)
{

	extern	REAL	prevx,currx,rotn,heelv;
	REAL	fac,xwl,ywl,zwl;

	if(prevc != currc) {
		fac = prevc/(prevc-currc);
	}
	else {
		fac = 0.0;
	}
	ywl = fadd(prevy,fmul(fsub(curry,prevy),fac));
	zwl = fadd(prevz,fmul(fsub(currz,prevz),fac));
	xwl = fadd(prevx,fmul(fsub(currx,prevx),fac));
	if(fpoz(ywl) || pltopp) pltsel(xwl,fmul(side,ywl),zwl,mode);
}

/*	plot a point, in selected co-ordinate convention according	*/
/*	to the value of mode:	mode = 	0 means end elevation		*/
/*                                      1       plan view		*/
/*					2	side elevation		*/
/*					3	perspective		*/
/*					4	DXF end elevation	*/
/*					5	DXF plan view		*/
/*					6	DXF side elevation	*/
/*					7	DXF 3-d			*/
/*					8	Plate development def'n	*/
/*					9	z- "     "          "	*/
/*					10	oblique view (section editing) */

#ifdef PROF
void write_ghs(REAL,REAL);
#endif

void pltsel(REAL x,REAL y,REAL z,INT mode)
{
	extern	INT	noline;
	REAL	xx;
	extern	REAL	rotn,heelv,cosaz,sinel;
#ifdef PLATEDEV
	extern	int	indx;
#endif
#ifdef GHS
	extern int num_points;
#endif

	switch(mode) {
	case 0:
		(*draw)(y,z);
		break;
	case 1:
		(*draw)(x,y);
		break;
	case 2:
		(*draw)(x,z);
		break;
	case 3:
		perspp(0.0,0.0,-x,pitch,rotn,heelv);
		(*draw)(y,z);
		break;
#ifdef EXT_OR_PROF
	case 4:
		write_xy(y,-z);
		break;
	case 5:
		write_xy(x,y);
		break;
	case 6:
		write_xy(x,-z);
		break;
	case 7:
		write_xyz(x,y,z);
		break;
#endif

#ifdef PLATEDEV
	case 8:
		xint_a[indx] = x;
		yint_a[indx] = y;
		zint_a[indx] = z;
		indx++;
		break;
	case 9:
		xint_b[indx] = x;
		yint_b[indx] = y;
		zint_b[indx] = z;
		indx++;
		break;
#endif
	case 10:
		(*draw)(y + sinaz*x,z + sinel*x);
		break;
	case 11:
		(*draw)(cosaz*yline[stemli][1] - sinaz*y,z - sinel*y);
		break;
#ifdef PROF
	case 12:
		write_ghs(y,z);
		break;
	case 13:
		num_points++;
		break;
#endif
	}
	noline = 0;
}


