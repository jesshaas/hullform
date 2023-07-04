/* Hullform component - drawline.c
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
void get_int_angle(int j,REAL *c,REAL *s);

/*	draw a longitudinal hull line in any mode		*/

void draw_line(int il,REAL side1,REAL side2,int mode,int inisec,
	int lassec,int swap,REAL *xrate,REAL *yrate,REAL *zrate,
	int *num,int *stem_ind,int *tran_ind,int all)
{
	int sts;
	int ens = ensec[il];
	REAL side;
	REAL xx1,yy1,zz1;
#ifdef EXT_OR_PROF
	REAL s,c;		/* rounded stem sine and cosine */
#endif
	REAL xfinal;
	REAL ystem = surfacemode ? 0.0 : yline[stemli][max(min(1,ensec[stemli]),stsec[stemli])];
	REAL xforw;
	REAL *xint,*yint,*zint;
	int numint;
	int i,i0,i1,ii;
	extern int numbetwl;
	REAL xsave,ysave;
	extern int indx;
	REAL t,dt;
	REAL delx,dely,delz;
	REAL fac,c0,yy2;

#ifdef EXT_OR_PROF
	extern int numbetw; /* number of points to plot between sections */
	int numperl = max(numbetw,numbetwl) + 1;
#endif

#ifndef STUDENT
	REAL xx0,yy0,zz0;	/* rounded stem and transom terms */
	REAL xround,yround,xcentre;
	REAL stemrad = radstem[il];
	int maxonround = min(maxext*2,numbetwl+1);
	int inside;
	REAL ttransom;
	REAL xtr,ytr,ztr;
	REAL xe;
	int itr;
#else
#ifdef EXT_OR_PROF
#define stemrad 0.0
#endif
#endif
	extern int *ignore;

	sts = stsec[il];
	if(sts < surfacemode) {
		sts = 1;
		stsec[il] = sts;
	}
	if(count <= 1 || ens <= sts) return;
#ifndef PLATEDEV
	indx = 0;
#endif

#ifndef STUDENT
	xe = xsect[count-1];
	if(surfacemode) {
		i = sts;
	}
	else {
		i = max(sts,1);
	}
	inside = yline[il][i] < ystem;	/* inside transom flag */
	if(ctransom != 0.0)
		ttransom = stransom / ctransom;
	else
	    ttransom = 1.0e+30;
	if(il < extlin && transom) {
		i = (numbetwl+1)*il;
		xtr = xtran[i];
		ytr = ytran[i];
		ztr = ztran[i];	/* coordinates of end of line */
	}
	else {
		xtr = 1.0e+10;
	}
#endif
	if(mode == 8) {
		for(i = 0 ; i < count ; i++) secton[i] = !ignore[i];
	}

#ifdef EXT_OR_PROF
	i = maxsec*numperl + 1;
#else
	i = maxsec + 1;
#endif
	if(!memavail((void *) &xint,3*i*sizeof(REAL))) {
		message("Insufficient space for line interpolation");
		return;
	}
	yint = &xint[i];
	zint = &yint[i];

	xforw = xsect[sts];
	if(!surfacemode && sts == 0) xforw -= yline[il][0];

	if(sts >= swap || inisec >= swap) side2 = side1;
#ifdef EXT_OR_PROF
	swap = (swap - sts) * (numbetw + 1);
#else
	swap -= sts;
#endif

	if(inisec < stsec[il]) inisec = stsec[il];
	if(lassec > ensec[il]) lassec = ensec[il];

	*stem_ind = -1;
	*tran_ind = -1;

#ifndef STUDENT
	get_int_angle(il,&c,&s);	/* find cos and sin of angle */
	c0 = c;				/*	to intersection point */
	t = fmul(stemrad,fsub(1.0,c));	/* x-offset from stemhead to intersection point */
	xcentre = fadd(xforw,stemrad);	/* centre of stem round */
	xround = fadd(xforw,t);		/* where stem round commences*/
	yround = fadd(ystem,fmul(stemrad,s));

	/*	If there are any points requested between sections, interpolate using
	the spline routine to get the required plotting set, otherwise simply
	copy the values over.
	*
	We must also call interp_lines when operating this routine in modes 8
	and 9 (constructing boundaries for plate development), because we
	need the rate tables xrate, yrate and zrate.
	*/
#else
#ifdef EXT
	c = s = 0.7071;	/* any values will do to keep the compiler quiet */
#endif
#endif

#ifdef EXT_OR_PROF
	if(numbetw > 0 || mode == 8 || mode == 9) {
#ifndef STUDENT
		ii = indx + maxonround;
#else
		ii = indx;
#endif
		interp_lines(il,sts,ens,&numint,
			yline[il],zline[il],yline[il][0],
			xint,yint,zint,c,s,stemrad,
			&xrate[ii],&yrate[ii],&zrate[ii],
			stem_ind,tran_ind,all);
	}
	else {
#endif
		ii = indx;
		xsave = xsect[0];
		ysave = yline[il][sts];		/* preserve stem values */
		if(!surfacemode) {
			xsect[0] = xforw;			/* stemhead */
			if(inisec == 0) yline[il][inisec] = ystem;
#ifndef STUDENT
			if(s != 0.0) yline[il][inisec] += t / s;
			/* y-offset to intersection point */
#endif
		}

		/*	Observe that the stem point will not be included,
		if inisec > 0
		*/
		i0 = 0;
		for(i = sts ; i <= ens ; i++) {
			if((all || secton[i]) && (i == 0 || fge(xsect[i],xsect[0]))) {
				if(*stem_ind < 0) *stem_ind = i;	/* first index */
				*tran_ind = i+1;

				/*	The rate values are not used in this context, but assigning an
				undefined real value gives some systems problems.
				*/
				xrate[i0] = 1.0;
				yrate[i0] = 0.0;
				zrate[i0] = 0.0;

				xint[i0] = xsect[i];
				yint[i0] = yline[il][i];
				zint[i0++] = zline[il][i];
			}
		}

		/*	Restore stem details		*/

		xsect[0] = xsave;
		yline[il][sts] = ysave;
		numint = i0;
#ifdef EXT_OR_PROF
	}
#endif

#ifndef STUDENT
	if(fnoz(stemrad)) {
		dt = (REAL) maxonround;
		dt = fdiv(1.5707963,dt);
	}
#endif
	for(side = side2 ; side >= side1 ; side -= 2.0) {
		(*newlin)();
		i = 0;
		i1 = ii;
#ifndef STUDENT
		if(fzer(stemrad)) goto quitloop;
		/* no stem round - skip following section */
		i++;	/* index of "next" spline point in offset tables */
		t = 0.0;	/* initialise angle on stem round */
		i0 = i1+1;	/* index of "next" spline point in rate tables */
		c = 1.0;	/* initialise cosine of angle on stem round */
		xx0 = xint[0];
		delx = xint[i] - xint[i-1];
		dely = fsub(yint[i],yint[i-1]);
		delz = fsub(zint[i],zint[i-1]);

		/*	trace stem round until tangent point has been passed	*/

		while(fgt(c,c0)) {

			/*	ensure current point on stem round is stemward of next point on
			spline curve
			*/
			while(fgt(xx0,xint[i])) {
				if(++i >= numint) goto quitloop;
				i1++;
				i0++;
			}

			/*	yy0 is the offset on the circle; yy1 is the offset on the spline
			curve; yy2 is the offset on the tangential straight line
			*/
			s = sqrt(1.0 - c*c);
			if(inside) s = -s;
			if(fpos(delx)) {
				yy0 = ystem + s*stemrad;
				fac = (xx0 - xint[i-1]) / delx;
				zz0 = zint[i-1] + fac*delz;
				yy1 = yint[i-1] + fac*dely;
				if(xsect[sts+1] != xround) {
					yy2 = (xx0 - xround) / (xsect[sts+1] -xround);
					yy2 = yround + yy2*(yline[il][sts+1] - yround);
				}
				else {
					yy2 = yround;
				}
				yy0 = yy0 + yy1 - yy2;

				if(mode >= 8 && mode < 10) {
					xrate[indx] = s;/* "indx" is altered in pltsel if required */
					yrate[indx] = c + s*(yrate[i0]*fac + yrate[i0-1]*(1.0 - fac));
					zrate[indx] = s*(zrate[i0]*fac + zrate[i0-1]*(1.0 - fac));
				}
				pltsel(xx0,side*yy0,zz0,mode);
			}

			t += dt;
			c = cos(t);
			xx0 = xcentre - c*stemrad;
		}
quitloop:
#endif
		xfinal = 1.01*xsect[ens] - 0.01*xsect[ens-1];
		for( ; i < numint && fle(xint[i],xfinal) ; i++) {
			xx1 = xint[i];
			yy1 = fmul(side,yint[i]);
			zz1 = zint[i];
			xrate[indx] = xrate[i1];
			yrate[indx] = yrate[i1];
			zrate[indx] = zrate[i1++];
#ifndef STUDENT
			if(xx1 > xtr) {	/* the drawn line has extended aft of the transom */

				pltsel(xtr,fmul(side,ytr),ztr,mode);
				if(transom && rtransom != 0.0) {
					yy2 = ytr - ystem;		/* distance from centreline of keel */
					t = yy2 / (REAL) numperl;	/* may be negative */
					c = rtransom * rtransom;
					s = xtr - fsqr0(c-yy2*yy2) / ctransom;	/* back end of curve */
					i0 = 0;				/* set to 1 if transom reaches end of hull */
					for(itr = 0 ; itr < numperl ; itr++) {
						yy2 -= t;
						xx1 = s + fsqr0(c - yy2*yy2) / ctransom;
						if(xx1 < xe) {	/* inside end of hull */
							pltsel(xx1,side*(ystem+yy2),ztr,mode);
						}
						else {
							if(!i0) {	/* first time, plot intersection at stern */
								i0 = 1;
								yy1 = (xe - s)*ctransom;
								yy1 = fsqr0(c - yy1*yy1);
								pltsel(xe,side*(yy1+ystem),ztr,mode);
							}
							zz1 = ztr - (xx1 - xe) / ttransom;
							pltsel(xe,side*(yy2 + ystem),zz1,mode);
						}
					}
				}
				break;
			}
#endif

			/*	otherwise, plot the point					*/

			if(yint[i] >= -100000.0)
				pltsel(xx1,yy1,zz1,mode);
			else
			    (*newlin)();

			/*	swap to the other side at the widest point (end elevation view)	*/

			if(i == swap) {
				(*newlin)();
				pltsel(xx1,-yy1,zz1,mode);
				side = side1;
			}
		}
	}

	*num = indx;
#ifndef STUDENT
	indx = 0;
#endif

	/*	Clean up and return		*/

	memfree((void *) xint);
}

/*	This routine interpolates the y- and z-offsets of a hull line
at the section x-positions, and "numbetw" points between. No
account of stem curve is taken, except to project the tangent
point of the line on the stem radius, to define a new y-offset
at the stem.
*/
#ifdef EXT_OR_PROF
void interp_lines(int jj,int sts,int ens,int *numint,
	REAL ytem[],REAL ztem[],REAL delx,
	REAL xvalue[],REAL yvalue[],REAL zvalue[],REAL c,REAL s,REAL r,
	REAL *xrate,REAL *yrate,REAL *zrate,
	int *stem_ind,int *tran_ind,int all)
{
	REAL	tem1,tem2;
	REAL	x,dx;
	int	i,j,k;
	extern	int numbetw;

	tem1 = xsect[0];	/* save first section position */
	tem2 = ytem[sts];	    	/* save whatever was in ytem */

	if(sts == 0) {
		if(!surfacemode) {
			xsect[0] -= delx;		/* move position to start of line */
			ytem[0] = yline[stemli][1];
		}
	}
	else if(r != 0.0) {
		ytem[sts] = yline[stemli][sts];
		ztem[sts] = zline[stemli][sts];
	}
	if(fnoz(r) && fnoz(s)) ytem[sts] += r*(1.0-c)/s;

	/*	Define x-coordinates at which points will be interpolated	*/

	k = 0;
	for(i = sts+1 ; i <= ens ; i++) {
		if((all || secton[i-1] && secton[i]) && xsect[i-1] >= xsect[0]
			    && fge(xsect[i],xsect[0])) {
			if(*stem_ind < 0) *stem_ind = i-1;	/* first index */
			*tran_ind = i;
			x = xsect[i-1];
			dx = (xsect[i] - x)/((float) (numbetw+1));
			if(dx > 0.0) {
				for(j = 0 ; j <= numbetw ; j++) {
					xrate[k] = 1.0;
					xvalue[k++] = x;
					x += dx;
				}
			}
		}
	}
	xrate[k] = 1.0;
	xvalue[k++] = x;

	*numint = k;	/* number of interpolated sections */

	/*    interpolate y- and z- curves		*/

	spline1(&xsect[sts],&ytem[sts],&linewt[jj][sts],ens - sts + 1,xvalue,yvalue,
		*numint,yline[jj][maxsec],yline[jj][maxsec+1],yrate);
	spline1(&xsect[sts],&ztem[sts],&linewt[jj][sts],ens - sts + 1,xvalue,zvalue,
		*numint,zline[jj][maxsec],zline[jj][maxsec+1],zrate);

	/*    restore original source array values	*/

	xsect[0] = tem1;
	ytem[sts] = tem2;
}
#endif

#ifndef STUDENT
void get_int_angle(int j,REAL *c,REAL *s)
{
	int sts = stsec[j];
	REAL r1,x1,c1,y3,t,dt,len,ay;

	r1 = radstem[j];				/* radius of line at stem */
	x1 = xsect[sts];
	if(sts == 0) x1 -= yline[j][0];	/* x-position of start of line */
	c1 = x1 + r1;					/* x-centre of circle */
	y3 = yline[j][sts+1] - yline[stemli][sts+1];
									/* y-offset at nearest section */
	t = xsect[sts+1] - c1;			/* distance from centre to nearest section */
	dt = t*t + y3*y3;				/* squared distance from centre to y-offset*/
	len = dt - r1*r1;
	if(len >= 0.0) {
		len = sqrt(len);			/* distance from nearest section to tangent point on circle, along hull line */
		ay = fabs(y3);
		if(dt > 0.0) {
			*s = (t*len + ay*r1) / dt;
			if(*s <= 1.0 && *s >= -1.0) {
				*c = sqrt(1.0 - (*s)*(*s));
				if(y3 < 0.0) *s = - *s;

			}
			else {
				*s = 0.0;
				*c = 1.0;
			}
		}
		else {
			*s = 0.0;
			*c = 1.0;
		}
	}
	else {
		*s = 0.0;
		*c = 1.0;
	}
}
#else

void get_int_angle(int j,REAL *c,REAL *s)
{
	*c = *s = 0.7017;
}
#endif
