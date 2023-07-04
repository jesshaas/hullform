/* Hullform component - spline.c
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

#define	ZERO	1.0E-16

/*	CUBIC SPLINE ROUTINE.  AUTHOR P. RYE, APRIL 1986	*/

/*	FITS A CUBIC SPLINE THROUGH THE N POINTS (t(i),x(i)), SUPPLIED BY	*/
/*	THE USER IN ARRAYS t AND x, AND RETURNS, IN ARRAY XS, NS VALUES	*/
/*	EVALUATED AT ORDINATES SPECIFIED IN THE ARRAY TS.	*/

/*	RUN1 AND RUN2 ARE CURVATURE RUN-OUT FACTORS,  RUN1 = CURVATURE AT	*/
/*	POINT 1 / CURVATURE AT POINT 2, WHILE RUN2 IS CORRESPONDING FACTOR FOR	*/
/*	THE OTHER END	*/

void spline(	REAL t[],REAL x[],REAL w[],INT n,
	REAL ts[],REAL xs[],INT ns,
	REAL run1,REAL run2)
{
	REAL *derivs;		/* array of derivative estimators,	*/
	/* to be discarded in this form of call	*/
	if(!memavail((void *) &derivs,maxsec*maxext*sizeof(REAL))) {
		message("Heap overflow in spline routine");
	}
	else {
		spline1(t,x,w,n,ts,xs,ns,run1,run2,derivs);
		memfree(derivs);
	}
}

void spline1(	REAL t[],REAL x[],REAL w[],INT nt,
	REAL ts[],REAL xs[],INT ns,
	REAL run1,REAL run2,REAL *deriv)
{
	REAL *a,*b,*c;
	if(!memavail((void *) &a,maxsec*maxext*3*sizeof(REAL))) {
		message("Heap overflow in spline routine");
	}
	else {
		b = &a[maxsec*maxext];
		c = &b[maxsec*maxext];

		spline2(t,x,w,nt,ts,xs,ns,run1,run2,deriv,a,b,c);
		memfree(a);
	}
}

void spline2(	REAL t[],REAL x[],REAL wt[],INT nt,
	REAL ts[],REAL xs[],INT ns,
	REAL run1,REAL run2,
	REAL deriv[],REAL a[],REAL b[],REAL c[])
{
	REAL s[maxsec*maxext],h[maxsec*maxext];
	REAL w[maxsec*maxext];
	INT	 i,j,n;
	REAL div,temp,diff;
	REAL prevt = t[0];
	REAL prevx = x[0];
	REAL tmax,tmin;
	REAL sixZ;
	static int first_message = 1;
	extern int force_linear;

	/*	t-INTERVALS AND SLOPES ("S" USED TEMPORARILY TO HOLD SLOPES)	*/

	n = 0;
	for(i = 1 ; i < nt ; i++) {
		if(t[i] > prevt) {
			h[n] = t[i]-prevt;
			if(h[n] < 0.0001) {
				if(first_message) {
					message("Section separation too small!");
					first_message = 0;
				}
				h[n] = 0.0001;
			}

			w[n] = force_linear ? 0.0 : wt[i-1];
			s[n] = (x[i]-prevx) / h[n];	/* used to be s[n++] = .., but this did not compile correctly */
			prevt = t[i];
			prevx = x[i];
			n++;
		}
	}

	w[n] = wt[nt-1];

	n++;
	if(n <= 1) {
		if(n <= 0) x[0] = 0.0;
		for(i = 0 ; i < ns ; i++) {
			xs[i] = x[0];
			a[i] = 0.0;
			b[i] = 0.0;
			c[i] = 0.0;
			deriv[i] = 0.0;
		}
		return;
	}

	/*	SOLVE EQUATIONS FOR S BY DOUBLE-SCAN PROCEDURE.  ASSUME
	S(i+1)  =  A(i) S(i) + B(i), SCAN DOWN FOR P'S AND Q'S, THEN SCAN
	UP FOR S VALUES.

	RECURSION RELATIONS FOR P'S AND Q'S.  OBTAINED BY
	SUBSTITUTING S(i+1) = A(i) S(i) + B(i) INTO

	h(i-1) S(i-1) + 2 (h(i-1) + h(i)) S(i) + h(i) S(i+1)

	= 6 ( (x(i+1)-x(i))/h(i) - (x(i)-x(i-1))/h(i-1) )

	TO GIVE A LINEAR EXPRESSION FOR S(i) IN TERMS OF S(i-1),
	AND HENCE VALUES FOR A(i-1) AND B(i-1) IN

	S(i)  =  A(i-1) S(i-1) + B(i-1).
	*/
	a[n-2] = run2; /* a[n-2] relates to curvature at point n-1 */
	b[n-2] = 0.0;

	for(i = n - 2 ; i > 0 ; i--) {
		sixZ = 6.0*(s[i]-s[i-1]);
		div = 2.0*(h[i]*w[i] + h[i-1]*w[i-1]) + a[i]*h[i]*w[i];
		if(fabs(div) > ZERO) {
			a[i-1] = -h[i-1]*w[i-1]/div;
			b[i-1] = (sixZ - b[i]*h[i]*w[i])/div;
		}
		else  {
			temp = h[i] * w[i];
			if(fabs(temp) > ZERO) b[i] = (sixZ - h[i-1]*w[i-1]*s[i-1])/temp;
			a[i-1] = run2;
			b[i-1] = 0.0;
		}
	}

	/*	s[0] is second derivative at bow, and s[1] obeys

	s[1] = a[0]*s[0]+b[0]
	and	s[1] = s[0] / run1

	so	a[0]*s[0] + b[0] = s[0] / run1
	i.e.,	a[0]*run1*s0 + b[0]*run1 = s[0]

	so	s[0] = b[0]*run1/(1 - run1*a[0])
	*/
	temp = 1.0 - run1 * a[0];
	if(fabs(temp) > ZERO) {
		s[0] = b[0]*run1/temp;
	}
	else {
		s[0] = 0.0;
	}

	/*	NOW FIND OTHER S VALUES	*/

	for(i = 1 ; i < n ; i++) s[i] = a[i-1]*s[i-1]+b[i-1];

	/*	DERIVE CUBIC POLYNOMIAL CO-EFFICIENTS FROM S VALUES	*/

	for(i = 0 ; i < n-1 ; i++) {

		a[i] = w[i]*(s[i+1]-s[i])/(6.0*h[i]);
		b[i] = w[i]*s[i]*0.5;

		/*	This relation ensures continuity of the offset at each node */
		c[i] = (x[i+1]-x[i])/h[i] - h[i]*(a[i]*h[i] + b[i]);
	}

	/*	CALCULATE X VALUES FOR SUPPLIED T'S	*/

	/*	The values are assumed to be in increasing or decreasing order	*/

	if(ns > 1 && ts[ns-1] > ts[0]) {
		j = n - 2;
		for(i = ns - 1 ; i >= 0 ; i--) {
			while(j > 0 && ts[i] < t[j]) j--;
			diff = ts[i]-t[j];
			temp = c[j]+diff*(b[j]+diff*a[j]);
			xs[i] = x[j]+diff*temp;
			deriv[i] = c[j] + diff * (2. * b[j] + 3. * diff * a[j]);
		}
	}
	else {
		j = 0;
		tmax = t[nt-1];
		tmin = t[0];
		for(i = 0 ; i < ns ; i++) {
			if(ts[i] > tmax) {
				xs[i] = x[nt-1];
				deriv[i] = 0.0;
			}
			else if(ts[i] < tmin) {
				xs[i] = x[0];
				deriv[i] = 0.0;
			}
			else {
				while(j < n-2 && ts[i] > t[j+1]) j++;
				diff = ts[i]-t[j];
				temp = c[j]+diff*(b[j]+diff*a[j]);
				xs[i] = x[j]+diff*temp;
				deriv[i] = c[j] + diff * (2. * b[j] + 3. * diff * a[j]);
			}
		}
	}
}
