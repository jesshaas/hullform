/* Hullform component - rollout.c
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

#ifdef PLATEDEV

#include "plate.h"

void put_marks(	REAL (far *xst)[MAXRUL],REAL xa[],REAL ya[],
	REAL (far *xen)[MAXRUL],REAL xb[],REAL yb[],
	REAL t,int numrul,FILE *fp);

/**********************************************************************

FUNCTION NAME:  roll out

DESCRIPTION:    rolls out out the 3-d surface of two chines.

INPUTS:
xstart,ystart[setnum],zstart[setnum] 	series of points on chine a and b which
xend  ,yend  ,zend	make up ruled lines.
rulings[plate_lin]	number of ruled lines on the surface.

OUTPUTS:

CALLS:

*********************************************************************/


MENUFUNC rollout()
{
	REAL	*xedge_a,*yedge_a,*xedge_b,*yedge_b;
	extern	REAL	xmax1,xmin1,ymax1,ymin1;
	extern	REAL	xmax0,xmin0,ymax0,ymin0;
	REAL	xmint,xmaxt,ymint,ymaxt;
	REAL	s,t,r;
	int		numrul = rulings[plate_lin];
	int		i;
	extern int	persp;
	extern int	scaled;
	REAL	plate_area;
	char	messagetext[40];
	extern int	xright,xleft,ytop,ybottom;
	extern REAL	PixelRatio;
	extern int 	repaint;

	graphics = 1;
	update_func = rollout;
	print_func = update_func;
	cls(FALSE);
	(*init)();

	if(numrul == 0) {
		repaint = 0;
		menufunc = NULL;
		update_func = NULL;
		message("Nothing to roll out");
		return;
	}

	if( !memavail((void **) &xedge_a,4*MAXRUL*sizeof(REAL)))  {
		message("Insufficient memory for rollout");
		return;
	}
	yedge_a = &xedge_a[MAXRUL];
	xedge_b = &yedge_a[MAXRUL];
	yedge_b = &xedge_b[MAXRUL];

	get_rollout(xedge_a,yedge_a,xedge_b,yedge_b,
		&xmint,&xmaxt,&ymint,&ymaxt,
		&plate_area);

	if(! scaled) {
		s = 0.75*(xsect[count-1] - xsect[0]);	/* half x-width    */
		t = 0.5*(xmaxt + xmint);		/* mean x-position */
		r = 0.5*(ymaxt + ymint);		/* mean y-position */
		xmin0 = t - s;
		xmax0 = t + s;
		s *= (REAL) (ybottom - ytop) / (REAL) (xright - xleft) * PixelRatio;
		ymin0 = r - s;
		ymax0 = r + s;
		setranges(xmin0,xmax0,ymin0,ymax0);
	}

	persp = 0;				/* turn off perspective */
	(*colour)(2);
	for(i = 0 ; i < numrul ; i++) {
		(*move)(xedge_a[i],yedge_a[i]);
		(*draw)(xedge_b[i],yedge_b[i]);	/* plot out ruling lines */
	}

	(*colour)(3);
	(*newlin)();
	for(i = 0 ; i < numrul ; i ++)	/* plot out rolled surface edge a */
		(*draw)(xedge_a[i],yedge_a[i]);

	(*newlin)();
	for(i = 0 ; i < numrul ; i ++)	/* plot out rolled surface edge b */
		(*draw)(xedge_b[i],yedge_b[i]);

	i = numrul-1;
	(*move)(xedge_a[i],yedge_a[i]);
	(*draw)(xedge_b[i],yedge_b[i]);

	(*colour)(1);
	put_marks(	xstart,xedge_a,yedge_a,
		xend,xedge_b,yedge_b,
		0.03*(xmax0-xmin0),numrul,NULL);
	sprintf(messagetext,"Plate area = %.4f",plate_area);
	(*move)(xmin*0.95 + xmax*0.05,ymin*0.95 + ymax*0.05);
	(*plstr)(messagetext);

	memfree(xedge_a);
	(*endgrf)();
}

MENUFUNC write_outline()
{
#ifdef DEMO
	not_avail();
#else
	REAL	*xedge_a,*yedge_a,*xedge_b,*yedge_b;
	extern	REAL xmax1,xmin1,ymax1,ymin1;
	REAL	t;
	int numrul = rulings[plate_lin];
	int i;
	FILE *fp;
	char *wr_err = "CAN NOT WRITE TO TEXT DEVICE";

#ifdef SHAREWARE
	nagbox();
#endif
	if(!memavail((void **) &xedge_a,4*MAXRUL*sizeof(REAL)))  {
		message("Insufficient memory for rollout");
		return;
	}
	yedge_a = &xedge_a[MAXRUL];
	xedge_b = &yedge_a[MAXRUL];
	yedge_b = &xedge_b[MAXRUL];

	get_rollout(xedge_a,yedge_a,xedge_b,yedge_b,
		&xmin1,&xmax1,&ymin1,&ymax1,&t);

	if(open_text(&fp,filedirnam,"*.txt")) {
		for(i = 0 ; i < numrul ; i++) {
			if(fprintf(fp,"%3d %8.4f %8.4f :%9.4f %8.4f\n",i,
				xedge_a[i],yedge_a[i],xedge_b[i],yedge_b[i]) < 0) {
				message(wr_err);
				break;
			}
		}
		fprintf(fp,"Section points on upper and lower lines:\n");
		put_marks(xstart,xedge_a,yedge_a,
			xend  ,xedge_b,yedge_b,
			0.0,numrul,fp);

		fclose(fp);
	}
	memfree(xedge_a);
#endif
}

void get_rollout(REAL *xedge_a,REAL *yedge_a,
	REAL *xedge_b,REAL *yedge_b,
	REAL *xmin1,REAL *xmax1,REAL *ymin1,REAL *ymax1,
	REAL *plate_area)
{
	REAL	vecta[3],
	vectb[3],
	a=0.0,
	b=0.0;

	REAL r,s,t;
	REAL numer,denom,length;
	int k,i;
	int setnum = developed[plate_lin];
	int numrul = rulings[plate_lin];

	*plate_area = 0.0;

	if(setnum < 0) return;	/* nothing to use if not developed */

	/*     initialise for first triangle	*/

	r = xstart[setnum][0] - xend[setnum][0];
	s = ystart[setnum][0] - yend[setnum][0];
	t = zstart[setnum][0] - zend[setnum][0];
	b = sqrt(r*r+s*s+t*t);	/* distance between start points is length */
							/* of first ruled line			   */

	/*	The first line drawn connects the start points of both edges	*/
	/*	of the surface to be drawn					*/

	xedge_a[0] = 0.0;
	yedge_a[0] = 0.0;	/* initial point of first edge is (0,0)	*/

	xedge_b[0] = 0.0;
	yedge_b[0] = b;	/* initial point of second edge is (0,b) */

	for(k = 1; k < numrul ; k ++) {

		/*	find vectors for two known sides of first triangle	*/

		/*	get the vector "vecta", joining the edges which also	*/
		/*	forms part of the prior triangle			*/

		getvector(vecta,xstart[setnum][k-1],xend[setnum][k-1],
			ystart[setnum][k-1],yend[setnum][k-1],
			zstart[setnum][k-1],zend[setnum][k-1]);

		/*	get the vector "vectb", which also joins the edges but	*/
		/*	forms the other side of the triangle (the commmon point	*/
		/*	is end point "k-1"					*/

		getvector(vectb,xstart[setnum][k],xend[setnum][k-1],
			ystart[setnum][k],yend[setnum][k-1],
			zstart[setnum][k],zend[setnum][k-1]);

		/*      function getab returns distance out (a) and up (b) of third	*/
		/*	point on flat surface, from vectors a and b			*/

		getab(&a,&b,vecta,vectb);
		if(xstart[setnum][k] > xstart[setnum][k-1]) a = -a;

		/*      Call getpoint, which finds, given a,b, 2-d points on chine	*/
		/*	a and b	*/

		getpoint(	&xedge_a[k] ,&yedge_a[k],
			xedge_a[k-1],yedge_a[k-1],
			xedge_b[k-1],yedge_b[k-1],
			a,b,plate_area);

		/*        now do the second triangle	*/
		/*        find two vectors that make up second triangle	*/

		getvector(vecta,xend[setnum][k-1],xstart[setnum][k],
			yend[setnum][k-1],ystart[setnum][k],
			zend[setnum][k-1],zstart[setnum][k]);
		/* line across between edges, and along one on edge a */
		getvector(vectb,xend[setnum][k],xstart[setnum][k],
			yend[setnum][k],ystart[setnum][k],
			zend[setnum][k],zstart[setnum][k]);
		/* line across between edges */

		getab(&a,&b,vecta,vectb);
		a = fabs(a);
		/* (b,a) is vector b in co-ordinate system based on vector a */

		getpoint(	&xedge_b[k],&yedge_b[k],
			xedge_b[k-1],yedge_b[k-1],
			xedge_a[k],yedge_a[k],
			-a,b,plate_area);

	}

	/*	work out alignment for plate	*/

	numer=(yedge_a[0]+yedge_b[0])-(yedge_a[numrul-1]+yedge_b[numrul-1]);
	denom=(xedge_a[0]+xedge_b[0])-(xedge_a[numrul-1]+xedge_b[numrul-1]);
	length = sqrt(numer*numer + denom*denom);
	if(fnoz(length)) {
		a = numer / length;
		b = denom / length;
	}
	else {
		a = 0.0;
		b = 1.0;
	}	/* The transformation to horizontally-aligned points is	*/
	/* now x' = b.x + a.y, y' = b.y - a.x			*/

	/*	Transform and find range of transformed data			*/

	*xmax1 = *ymax1 = -1.0e+30;
	*xmin1 = *ymin1 = 1.0e+30;
	for(i = 0 ; i < numrul ; i++) {
		s = xedge_a[i];
		t = yedge_a[i];
		r = b * t - a * s;
		s = b * s + a * t;
		xedge_a[i] = s;
		yedge_a[i] = r;
		if(flt(*xmax1,s)) *xmax1 = s;
		if(fgt(*xmin1,s)) *xmin1 = s;
		if(flt(*ymax1,r)) *ymax1 = r;
		if(fgt(*ymin1,r)) *ymin1 = r;

		s = xedge_b[i];
		t = yedge_b[i];
		r = b * t - a * s;
		s = b * s + a * t;
		xedge_b[i] = s;
		yedge_b[i] = r;
		if(flt(*xmax1,s)) *xmax1 = s;
		if(fgt(*xmin1,s)) *xmin1 = s;
		if(flt(*ymax1,r)) *ymax1 = r;
		if(fgt(*ymin1,r)) *ymin1 = r;
	}
}


/**********************************************************************

FUNCTION NAME:   get vect

DESCRIPTION:     gets a <x,y,z> vector of two points of a line.

INPUTS:          ax,ay,az REAL point of line a.
bx,by,by REAL point of line b.

OUTPUTS:         vect[] REAL contains the x,y,z co-ordinate
in the first 3 elements;

CALLS:           none.

*********************************************************************/
void getvector(REAL vect[],
	REAL ax,REAL bx,      /* point a and b of line */
	REAL ay,REAL by,
	REAL az,REAL bz)
{
	vect[0] = ax - bx;                 /* subtract first point from last */
	vect[1] = ay - by;
	vect[2] = az - bz;
}

/*	Plot section marks on rollout view				*/
/*	"t" is required tick size on the line of marks			*/
/*	"numrul" is the number of ruling lines provided			*/

void put_marks(	REAL (far *xst)[MAXRUL],REAL xa[],REAL ya[],
	REAL (far *xen)[MAXRUL],REAL xb[],REAL yb[],
	REAL t,int numrul,FILE *fp)
{
	int ia,ib,i,k,dk,i1,i2;
	REAL a,b,r,s,div,Xa,Xb,Ya,Yb;
	REAL x1[maxsec],y1[maxsec],x2[maxsec],y2[maxsec];
	int n1;
	int setnum = developed[plate_lin];

	ib = ia = numrul-2;
	n1 = 0;
	for(i = 0 ; i < count ; i++) {

		/*	Move edge indices ia and ib so that they point to lines
		which terminate sternward of the section	*/
		while(ia > 0 && xsect[i] > xst[setnum][ia]) ia--;
		while(ib > 0 && xsect[i] > xen[setnum][ib]) ib--;

		/*	Interpolate to find start point, on edge a	*/
		s = xst[setnum][ia+1] - xst[setnum][ia];
		if(s != 0.0)
			s = (xsect[i] - xst[setnum][ia])/s;
		else
			continue;
		if(s < 0.0 || s > 1.0) continue;
		Xa = xa[ia] + (xa[ia+1] - xa[ia]) * s;
		Ya = ya[ia] + (ya[ia+1] - ya[ia]) * s;

		/*	Interpolate to find end point, on edge b	*/
		s = (xsect[i] - xen[setnum][ib])/
			(xen[setnum][ib+1] - xen[setnum][ib]);
		if(s < 0.0 || s > 1.0) continue;
		Xb = xb[ib] + (xb[ib+1] - xb[ib]) * s;
		Yb = yb[ib] + (yb[ib+1] - yb[ib]) * s;

		if(i > 0 && xsect[i] != xst[setnum][ib]) {	// this section had no ruling line - presume straight
			a = yline[plate_lin][i] - yline[plate_lin-1][i];
			b = zline[plate_lin][i] - zline[plate_lin-1][i];
			r = sqrt(a*a + b*b);
			a = Xa - Xb;
			b = Ya - Yb;
			s = sqrt(a*a + b*b);
			if(s > 0.0) {
				r /= s;
				a = fabsf(xst[setnum][ia] - xst[setnum][ia+1]);
				b = fabsf(xen[setnum][ia] - xst[setnum][ia+1]);

				if(a < b) {
					Xb = Xa + r*(Xb - Xa);
					Yb = Ya + r*(Yb - Ya);
				} else {
					Xa = Xb + r*(Xa - Xb);
					Ya = Yb + r*(Ya - Yb);
				}
			}
		}

		if(fp != NULL) {
			fprintf(fp,"%2d:%8.3f %8.3f, %8.3f %8.3f\n",i,Xa,Ya,Xb,Yb);
		}
		else {
			if(i % 5 == 0) {
				(*move)(Xa,Ya-t-t);
				plint(i,(i < 10) ? 1 : (i < 100) ? 2 : 3);
			}
			(*move)(Xa,Ya-t);
			(*draw)(Xa,Ya);

			if(ia != ib) {
				if(ia < ib) {
					dk = 1;
				}
				else {
					dk = -1;
				}
				i1 = ia;
				if(xen[setnum][ia] > xst[setnum][ia]) i1 += dk;
				i2 = ib;
				if(xen[setnum][ib] < xst[setnum][ib]) i2 -= dk;

				for(k = i1 ; k != i2 ; k += dk) {
					div = xend[setnum][k] - xstart[setnum][k];
					if(div != 0.0) {
						r = (xsect[i] - xstart[setnum][k])/div;
						if(r >= 0.0 && r <= 1.0) {
							a = xa[k] + r*(xb[k] - xa[k]);
							b = ya[k] + r*(yb[k] - ya[k]);
							(*draw)(a,b);
						}
					}
				}
			}
			(*draw)(Xb,Yb);
			(*draw)(Xb,Yb+t);
			x1[n1] = Xa;
			y1[n1] = Ya;
			x2[n1] = Xb;
			y2[n1] = Yb;
			n1++;
		}
	}
	(*newlin)();
	for(i = 0 ; i < n1 ; i++) (*draw)(x1[i],y1[i]);
	(*newlin)();
	for(i = 0 ; i < n1 ; i++) (*draw)(x2[i],y2[i]);
}

/**********************************************************************

FUNCTION NAME: GET POINT

DESCRIPTION:   Determines the co-ordinates (bx,by) on from end points
(a1x,a1y) and (a2x,a2y) of one vector (A) and vector (a,b)
(B) in co-ordinate system for which the first vector is
the "x-axis".

Both line A and line B have the same starting point (a1x,a1y).

ALGORITHM:     STEP 1.   find the angles between the two lines and
the x and y axis.
STEP 2.   get the distance between line a and line b.
STEP 3.   determine the end point of line b.
INPUTS:
a1x,a2y  float   (x,y) start point of line.
a2x,a2y  float   (x,y) end point of line.
adis     float   length of line a.
bdis     float   length of line b to find end point of.

OUTPUTS:    bx       float
by       float    end point of line b.
plate_area float	incremented area of developed plate

CALLS:      none.

*********************************************************************/

void getpoint(float *bx,float *by,float a1x,float a1y,float a2x,float a2y,
	float adis,float bdis,float *plate_area)
{
	float dx,dy,Asq,Alen;
	float lsin,lcos;

	dx = a1x - a2x;		 /* x-distance between ends of A */
	dy = a1y - a2y;		 /* y-distance between ends of A */
	lsin = dy * bdis - dx * adis;/* length A * length B * sin(angle B) */
	lcos = dx * bdis + dy * adis;/* length A * length B * cos(angle B) */
	Asq = dx*dx + dy*dy;	 /* length of vector A, squared */
	if(Asq != 0.0) {
		Alen = fsqr0(Asq);	 /* length of vector A		*/
		*bx = a2x + lcos / Alen;
		*by = a2y + lsin / Alen;
		*plate_area += 0.5 * fabs(adis) * Alen;
	}
	else {
		*bx = a2x + adis;
		*by = a2y + bdis;
	}
}

#endif
