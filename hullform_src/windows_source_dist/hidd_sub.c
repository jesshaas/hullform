/* Hullform component - hidd_sub.c
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

#ifdef EXT_OR_PROF

extern	REAL	*xv1,*yv1,*zv1;	/* first vertex of triangle	*/
extern	REAL	*xv2,*yv2,*zv2;	/* second vertex of triangle	*/
extern	REAL	*xv3,*yv3,*zv3;	/* third vertex of triangle	*/

extern	REAL	floatn;		/* number of triangles per surface	*/
extern	int	plotright,plotleft;
extern	REAL	sinpp,cospp;
extern	REAL	yview,zview;
extern	REAL	axx,axy,axz,ayx,ayy,ayz,azx,azy,azz;
extern	REAL	pp;
extern	REAL	rotn,heelv;
extern	REAL	ypersp,zpersp;
extern	int	persp;
extern	int	numbetwl;
extern	int	jtr;

extern	int	numshade;
extern	int	shade;
extern	REAL	cshade;

void transform(REAL *x,REAL *y,REAL *z)
{
	REAL xp,yp,zp;
	extern REAL xpersp,ypersp,zpersp;

/*	evaluate transformation coefficients			*/

	xp = *y;
	yp = *z;
	zp = - *x;

	*x = axx*xp + axy*yp + axz*zp;
	*y = ayx*xp + ayy*yp + ayz*zp;
	*z = azx*xp + azy*yp + azz*zp;
}

/*	transform position corner of triangle relative to viewpoint	*/
/*	to screen coordinates						*/

void screen_transform(REAL *x,REAL *y,REAL *z)
{
	REAL div;
	REAL yy,zz;
	REAL zr;
	extern REAL xgorigin,xgslope,ygorigin,ygslope;
	extern REAL sinpp,cospp;

	div = sinpp*(yview - *y) + cospp*(zview - *z);
	if(div != 0.0) {
		zr = (sinpp*yview + cospp*zview) / div;
		zz = (zview*(*y) - yview*(*z)) / div;
		yy = zr * (*x);
	}
	*x = *z;
	*y = xgorigin + yy*xgslope;
	*z = ygorigin + zz*ygslope;
}

/*	Interpolate between clearances of two vertically-separated points,
	to estimate the location between where the clearance is zero
*/
int reinterp(REAL *y1,REAL *z1,REAL *c1,REAL y2,REAL z2,REAL c2)
{
   REAL t1;
   REAL dc;
   dc = *c1 - c2;
   if(dc != 0.0) {
	t1 = *c1 / dc;
	*y1 = *y1 + t1*(y2 - *y1);
	*z1 = *z1 + t1*(z2 - *z1);
	*c1 = 0.0;
	return(1);
   } else {
	return(0);
   }
}

REAL dist_to(REAL x,REAL y1,REAL z1)
{
    REAL y = yview - y1;
    REAL z = zview - z1;
    REAL result = x*x + y*y + z*z;

    return(result);
}

#ifndef STUDENT
int tanknext(int i,int *j,int *tank,int endlin)
{
	int	offrange;

	(*j)++;
	while(*j < endlin) {
	    offrange = stsec[*j] > i || ensec[*j] <= i;
	    if(*j == fl_line1[*tank]) {
		if(offrange) {
		    *j = fl_line1[++(*tank)];
		} else {
		    return(1);
		}
	    } else {
		if(offrange)
			(*j)++;
		else
			break;
	    }
	}
	return(0);
}

/*	If this is the first line of the next tank, set up check of	*/
/*	whether it is on this side					*/

void starttank(int i,int j,int *tank,int *right,int *left,
	REAL *a1,REAL *hb1,REAL *c1,REAL *hd1,
	REAL *a2,REAL *hb2,REAL *c2,REAL *hd2,
	REAL *x12,REAL *y12,REAL *z12,REAL *c12,
	REAL *x22,REAL *y22,REAL *z22,REAL *c22)
{
	REAL aa,cc;
	int jj;

	*right = plotright && !fl_right[*tank];
	*left  = plotleft  && fl_right[*tank];

/*	Set watch for next tank and create extra curve			*/

	(*tank)++;
	jj = fl_line1[*tank]-1;
	*y12 = yline[jj][i];
	*z12 = zline[jj][i];
	*c12 = fsub(*x12,xtran[jtr]);
	aa = fsub(*y12,yline[j][i]);
	*a1  = fadd(aa,aa);
	*hb1 = fmin(aa);
	cc = fsub(zline[jj][i],*z12);
	*c1  = fadd(cc,cc);
	*hd1 = fmin(cc);

	*y22 = yline[jj][i+1];
	*z22 = zline[jj][i+1];
	*c22 = fsub(*x22,xtran[jtr]);
	aa = fsub(*y22,yline[j][i+1]);
	*a2  = fadd(aa,aa);
	*hb2 = fmin(aa);
	cc = fsub(zline[j][i+1],*z22);
	*c2  = fadd(cc,cc);
	*hd2 = fmin(cc);

}
#endif

/*	Set triangles given 12 corner co-ordinates		*/

void set_tri(REAL x11,REAL y11,REAL z11,
	REAL x12,REAL y12,REAL z12,
	REAL x21,REAL y21,REAL z21,
	REAL x22,REAL y22,REAL z22,
	int *n,int dn)
{
	int i;

	if(fnoz(y11) || fnoz(y12) || fnoz(y21) || fnoz(y22)) {
		i = *n;
		xv1[i] = x11;	yv1[i] = y11;	zv1[i] = z11;
		xv2[i] = x12;	yv2[i] = y12;	zv2[i] = z12;
		xv3[i] = x21;	yv3[i] = y21;	zv3[i] = z21;

		i += dn;
		xv1[i] = x21;	yv1[i] = y21;	zv1[i] = z21;
		xv2[i] = x22;	yv2[i] = y22;	zv2[i] = z22;
		xv3[i] = x12;	yv3[i] = y12;	zv3[i] = z12;

		i += dn;
		*n = i;
	}
}

/*	set x, y and z values to initialise co-ordinates around the hull */

void set_xyz(REAL *x12s,REAL *y12s,REAL *z12s,REAL *c12s,
	REAL *x22s,REAL *y22s,REAL *z22s,REAL *c22s,int i, int j)
{
#ifndef STUDENT
	REAL xx1,r1,len,yy1;
#endif
	REAL ystem;

	if(surfacemode)
	    ystem = 0.0;
	else
	    ystem = yline[stemli][1];

	*x12s = xsect[i];
	*y12s = yline[j][i];
	*z12s = zline[j][i];
	if(i == 0) {
#ifndef STUDENT
	    r1 = radstem[j];
	    if(fnoz(r1)) {
		get_int_angle(j,&xx1,&yy1);
		len = fsub(r1,fmul(r1,xx1));
		xx1  = fsub(*x12s,*y12s);
		*x12s = fadd(xx1,len);
		*y12s = fadd(ystem,fmul(yy1,r1));
		*z12s = fadd(*z12s,
		  fmul(fdiv(len,fsub(xsect[1],xx1)),fsub(zline[j][1],*z12s)));
	    } else {
#endif
		*x12s = fsub(*x12s,*y12s);
		*y12s = ystem;
#ifndef STUDENT
	    }
#endif
	}
	*x22s = xsect[i+1];
	*y22s = yline[j][i+1];
	*z22s = zline[j][i+1];
#ifdef STUDENT
	*c22s = *x22s - xsect[count-1];
	*c12s = *x12s - xsect[count-1];
#else
	*c22s = fsub(*x22s,xtran[jtr]);
	*c12s = fsub(*x12s,xtran[jtr]);
#endif

}

int find_inters_point(int j,REAL *x12s,REAL *y12s,REAL *z12s)
{
    REAL xx1,yy1;
    REAL len;
    REAL x1;
    REAL r1;

#ifndef STUDENT
    r1  = radstem[j];
#else
    r1 = 0.0;
#endif

    get_int_angle(j,&xx1,&yy1);

    x1 = xsect[0];
    if(!surfacemode) x1 -= yline[j][0];

    len = r1 - r1 * xx1;
    *x12s = x1 + len;
    *y12s = yy1*r1;
    if(!surfacemode) *y12s += yline[stemli][1];
    x1 = xsect[1] - x1;
    if(x1 != 0.0) x1 = len / x1;
    *z12s = zline[j][0] + (zline[j][1]-zline[j][0]) * x1;
    return(r1 != 0.0);
}

#endif
