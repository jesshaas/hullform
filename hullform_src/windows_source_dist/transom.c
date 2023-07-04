/* Hullform component - transom.c
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
 
#ifdef PROF

#include "hulldesi.h"

extern int changed;
int curve_inters(REAL x0,REAL ax,
	REAL y0,REAL ay,REAL by,REAL cy,
	REAL z0,REAL az,REAL bz,REAL cz,
	int *inside0,int *inside1,
	REAL *tvalue,REAL *xvalue,REAL *yvalue,REAL *zvalue);

/*	define transom parameters				*/

MENUFUNC def_transom()
{
	extern int transom;
	extern REAL dztransom,atransom,dtransom;
	int tran = transom;
	extern int numbetwl;

	(void) getdlg(EDITTRAN,
		INP_REA,(void *) &dztransom,
		INP_REA,(void *) &atransom,
		INP_REA,(void *) &rtransom,
		INP_LOG,(void *) &transom,
		-1);
	if(numbetwl < 3) {
		message("The number of points you were plotting between lines was less than 3.\n\nTo permit some accuracy in transom calculations, the value has been increased to 3.\n\nYou may find you need to increase the number further");
		numbetwl = 3;
	}
	redef_transom();
}

MENUFUNC redef_transom()
{
#define RAD 0.01745329

	extern REAL	atransom,dztransom,dtransom,
	ctransom,stransom,ztransom;

	stransom = sin(atransom * RAD);
	ctransom = cos(atransom * RAD);

	ztransom = zline[stemli][count-1] - dztransom;
	dtransom = xsect[count-1] * ctransom - ztransom * stransom;

	transom = stransom != 0.0;

	recalc_transom();

}

/*	Find the intersection point of a nominated (x,y,z) curve with the
curved transom
*/

int curve_inters(REAL x0,REAL ax,
	REAL y0,REAL ay,REAL by,REAL cy,
	REAL z0,REAL az,REAL bz,REAL cz,
	int *inside0,int *inside1,
	REAL *tvalue,REAL *xvalue,REAL *yvalue,REAL *zvalue)
{
	REAL F,Ft,Fx,Fxt,Fz,Fzt,Fy,Fyt;
	REAL axc = ax*ctransom;
	REAL azs = az*stransom;
	REAL bzs = bz*stransom;
	REAL czs = cz*stransom;
	REAL R2 = rtransom*rtransom;
	REAL x = x0 - xsect[count-1];
	REAL z = z0 - ztransom;
	REAL xc = ctransom*x;
	REAL zs = stransom*z;
	REAL Z = xc - zs;
	REAL X = axc - azs;
	REAL Y;
	REAL dt;
	int tries = 20;
	REAL t = 0.5;
	REAL t1;
	int cut;
	REAL ys = y0 - yline[stemli][1];

	realloc_hull(extlin);

	if(rtransom == 0.0) {		/* flat transom */
		if(stransom == 0.0) {	/* vertical flat transom */
			if(ax != 0.0) {
				t = -x / ax;	/* longitudinal line */
			}
			else {
				t = 0.0;	/* section */
			}
			*inside0 = 1;
			*inside1 = 1;
			cut = 0;
		}
		else {
			if(cz != 0.0) {	/* full cubic - iterative solution */
				t = 0.5;
				do {
					Ft = X - t*(3.0*t*czs + 2.0*bzs);
					if(Ft != 0.0) {
						F = Z + X*t - t*t*(czs*t + bzs);
						dt = F / Ft;
					}
					else {
						dt = 0.01;
					}
					t -= dt;
				}
				while(tries-- && fabs(dt) > 0.005);

			}
			else if(bz != 0.0) {	/* quadratic */
				Y = X*X + 4.0*bzs*Z;
				if(Y > 0.0 && bzs != 0.0) {
					t = (X - sqrt(Y)) / (2.0*bzs);
				}
				else {
					t = -1.0e+30;
				}
			}
			else {			/* linear */
				if(Z != 0.0)
					t = -Z / X;
				else
					t = -1.0e+30;
			}
			*inside0 = ctransom * x < stransom * z;
			*inside1 = (ctransom + x * ax) < stransom * (z + az + bz + cz);
			cut = t >= 0.0 && t <= 1.0;
		}
	}
	else {	/* curved transom */
		do {
			Fx  = x + t*ax;
			Fxt = ax;
			Fy  = ys + t*(ay + t*(by + t*cy));
			Fyt = ay + t*(2.0*by + 3.0*t*cy);
			Fz  =  z + t*(az + t*(bz + t*cz));
			Fzt = az + t*(2.0*bz + 3.0*t*cz);
			t1  = ctransom*Fx - stransom*Fz + rtransom;
			F   = t1*t1 + Fy*Fy - R2;
			Ft  = 2.0 * (t1*(ctransom*Fxt - stransom*Fzt) + Fy*Fyt);
			if(Ft != 0.0)
				dt = F / Ft;
			else
				dt = 0.01;
			if(tries > 10)
				t -= dt;
			else
				t -= 0.5*dt;
		}
		while(fabs(dt) > 0.005 && tries--);
		cut = t > 0.0 && t <= 1.0 && t1 > 0.0;
		*inside0 = (t > 0.0) == (Ft > 0.0);
		*inside1 = (t > 1.0) == (Ft > 0.0);
	}
	if(cut) {
		*tvalue = t;
		*xvalue = x0 + t*ax;
		*yvalue = y0 + t*(t*(t*cy + by) + ay);
		*zvalue = z0 + t*(t*(t*cz + bz) + az);
	}
	return(cut);
}

/*	This routine calculates the (numbetwl+1)*(numlin-1)+1 points
which define the outline of the transom
*/

void recalc_transom()
{
	int i,j,jj,j1,j0,j2,k,ii,i1,i0,ic;
	REAL t,tt,t1,dummy;
	REAL yt,zt;
	extern int numbetwl,numbetw;
	extern int numtran;
	int numperl = numbetwl + 1;
	REAL dt = 1.0/((REAL) numperl);
	REAL ts;
	REAL *base;
	REAL *a,*hb,*c,*hd,*yl,*zl,*ax,*ay,*by,*cy,*az,*bz,*cz;
	int *jval,*jprev;
	REAL *wt;
	REAL cosstem,sinstem;
	REAL *deriv;
	REAL xsave = xsect[0];
	REAL dx1,dx2,dx3;
	int transom_base_line = -1;
	extern int previous_getparam_line;
	int first_cut = 1;

	i = maxsec+2;
	if(!memavail((void **) &base,17*sizeof(REAL)*i)) {
		message("No free memory - please quit nonessential programs");
		return;
	}
	a  = base;
	hb = a  + i;
	c  = hb + i;
	hd = c  + i;
	yl = hd + i;
	zl = yl + i;
	ax = zl + i;
	ay = ax + i;
	by = ay + i;
	cy = by + i;
	az = cy + i;
	bz = az + i;
	cz = bz + i;
	deriv = cz + i;
	wt = deriv + i;
	jval = (int *) wt + i;
	jprev = jval + i;

	transom = atransom != 0.0;
	k = 0;
	for(i = 0 ; i < count-1 ; i++) ax[i] = xsect[i+1] - xsect[i];

	j0 = -1;
	ts = 1.0;
	for(j = 1 ; j < numlin ; j++) {
		if(ensec[j] == count-1 && j <= stemli) transom_base_line = j;	// highest-indexed line ending at stern, at or before stem line

//	Find the hull curve parameters that apply to this part of the hull surface

		for(i = 0 ; i < count ; i++) {
		    for(jj = j ; jj < numlin ; jj++) {
				if(stsec[jj] <= i && ensec[jj] >= i) break;
			}
			getparam(i,jj,&a[i],&hb[i],&c[i],&hd[i]);
			jval[i] = jj;
			jprev[i] = previous_getparam_line;
		}

//	Loop through all interpolated lines between each defined line pair


		t = ts;
		for(j1 = j0 ; j1 <= numbetwl ; j1++) {
			j0 = 0;

//	Create an interpolated line along the hull, or just draw a flat transom

			for(i = transom ? 0 : count-1 ; i < count ; i++) {
				jj = jval[i];	// index of next line "below" the required interpolated line
				j2 = jprev[i];

				tt = (REAL)(jj - j2);
				t1 = ((REAL)(jj - j) + t)/tt;
				tt = 1.0 - t1;
				yt = yline[jj][i] + t1*(a[i] + t1*hb[i]);
				zt = zline[jj][i] - t1*(c[i] + t1*hd[i]);
				if(transom) {
					yl[i] = yt;
					zl[i] = zt;
					wt[i] = tt*linewt[jj][i] + t1*linewt[j2][i];
				} else if(i == count-1) {
					xtran[k] = xsect[i];
					ytran[k] = yt;
					ztran[k] = zt;
					k++;
				}
			}

//	If there is no raked/sloped transom, the transom outline has beenc defined by now

			if(!transom) continue;

			for(i = maxsec ; i <= maxsec+1 ; i++) {
				yl[i] = tt* yline[jj][i] + t1* yline[jj-1][i];
				zl[i] = tt* zline[jj][i] + t1* zline[jj-1][i];
				wt[i] = tt*linewt[jj][i] + t1*linewt[jj-1][i];
			}

//	Find sine and cosine of tangent angle to stem. This is presumed
//	constant for the whole surface, but the error is probably
//	insignificant by the time the line has met the transom

			get_int_angle(jj,&cosstem,&sinstem);

			xsect[0] = xsave - yl[0];
			yl[0] = yline[stemli][1];
			if(radstem[j] > 0.0 && sinstem != 0.0) yl[0] += radstem[j]*(1.0 - cosstem) / sinstem;

			dummy = 0.0;
			spline2(xsect,yl,wt,count,&dummy,&dummy,0,yl[maxsec],yl[maxsec+1],deriv,cy,by,ay);
			dummy = 0.0;
			spline2(xsect,zl,wt,count,&dummy,&dummy,0,zl[maxsec],zl[maxsec+1],deriv,cz,bz,az);

//	Now search back along the line, to find the intersection point

			xtran[k] = 9999.0;
			ytran[k] = 0.0;
			ztran[k] = 0.0;
			for(i = count - 2 ; i > 0 ; i--) {
				dx1 = ax[i];
				dx2 = dx1*dx1;
				dx3 = dx2*dx1;
				if(numbetw > 0) {
					if(curve_inters(xsect[i],dx1,
						yl[i],ay[i]*dx1,by[i]*dx2,cy[i]*dx3,
						zl[i],az[i]*dx1,bz[i]*dx2,cz[i]*dx3,
						&i0,&i1,&dummy,&xtran[k],&ytran[k],&ztran[k])) break;
				}
				else {	/* straight line interpolation */
					if(curve_inters(xsect[i],dx1,
						yl[i],yl[i+1]-yl[i],0.0,0.0,
						zl[i],zl[i+1]-zl[i],0.0,0.0,
						&i0,&i1,&dummy,&xtran[k],&ytran[k],&ztran[k])) break;
				}
				if(i0) {

					/*	This point in the program is only reached if the line segment is
					totally within the transom - i.e., when the transom is outside
					the last section
					*/
					if(ensec[j] == count-1) {
						if(first_cut) {
							first_cut = 0;

//	First time at the transom, use the intersection of the transom surface with the hull surface

							ic = count-1;
							curve_inters(xsect[ic],0.0,
							yline[j][ic],a[ic],hb[ic],0.0,
							zline[j][ic],-c[ic],-hd[ic],0.0,
							&i0,&i1,&dummy,&xtran[k],&ytran[k],&ztran[k]);
						} else {
							xtran[k] = xsect[count-1];
							ytran[k] = yl[count-1];
							zt = rtransom*rtransom - yl[count-1]*yl[count-1];
							if(zt < 0.0) zt = 0.0;
							ztran[k] = ztransom + (rtransom - sqrt(zt))/stransom;
						}

					} else if(k > 0) {	// use previous point
						xtran[k] = xtran[k-1];
						ytran[k] = ytran[k-1];
						ztran[k] = ztran[k-1];
					} else if(transom_base_line >= 0) {	// use stem line
						xtran[k] = xsect[count-1];
						ytran[k] = yline[count-1][transom_base_line];
						ztran[k] = zline[count-1][transom_base_line];
					} else {	// insert a value that is valid, at least
						xtran[k] = xsect[count-1];
						ytran[k] = 0.0;
						ztran[k] = 0.0;
					}
					break;
				}
			}
			k++;
			t -= dt;
			j0 = 0;
		}
		ts = 1.0 - dt;
	}
	numtran = k;

	/*	Ensure no transom effect for tanks	*/

	while(k <= (extlin-1)*numperl) xtran[k++] = xsect[count-1];

	for(k = numtran-1 ; k >= 0 ; k--) {
		if(xtran[k] < 9999.0) break;
	}
	while(--k >= 0) {
		if(xtran[k] == 9999.0) {
			xtran[k] = xtran[k+1];
			ytran[k] = ytran[k+1];
			ztran[k] = ztran[k+1];
		}
	}

	xsect[0] = xsave;
	memfree(base);

//	FILE *dbg = fopen("transom.txt","wt");
//	for(i = 0 ; i < numtran ; i++) fprintf(dbg,"%d\t%f\t%f\t%f\t\n",i,xtran[i],ytran[i],ztran[i]);
//	fclose(dbg);

}

/*	Draw the transom in any of plan, elevation, or perspective mode */

/*	side1 and side2 are (provided) side limits, - 1 or 1 each	*/
/*	mode is 0-8, depending on the view being shown			*/

void draw_transom(REAL side1,REAL side2,int mode)
{
	REAL side;
	extern int numbetwl;
	REAL ystem;
	extern int numtran;
	int maxtran = (numbetwl+1)*(numlin-1);
	int i;

	if(transom) {
		for(side = side1 ; side <= side2 ; side += 2.0) {
			(*newlin)();
			for(i = 0 ; i < numtran ; i++) pltsel(xtran[i],side*ytran[i],ztran[i],mode);

//	show centreline of transom

			(*newlin)();
			ystem = side*yline[stemli][1];
			pltsel(xsect[count-1],ystem,ztransom,mode);
			if(atransom > 0.0)
				pltsel((dtransom+stransom*ztran[0])/ctransom,ystem,ztran[0],mode);
			else if(xtran[maxtran] < 9999.0)
				pltsel(xtran[maxtran],ystem,ztran[maxtran],mode);
		}
	}
}


extern int first_transom_warning;

void transom_stats(REAL *sarm1,REAL *volu1,REAL *calcdisp,REAL *xsum,REAL *wplane1,
	REAL wll,REAL density1,REAL *lwl1,REAL *xsta,REAL *xend,REAL *sumlcf,
	REAL *summct,REAL *ww1,REAL *ww2,REAL *rm1,REAL *rm2,REAL *sumysqdif,REAL *sumyoffs,
	REAL *Abelow1,REAL *Aabove1,REAL *AXbelow,REAL *AXabove,REAL *AZbelow,REAL *AZabove)
{
	int it,del_it,i,j,jj,last;
	REAL zcut[maxsec],z;
	REAL ycut[maxsec],y;
	REAL tvalue(REAL dy,REAL dz,REAL a,REAL hb,REAL c,REAL hd);
	REAL t,dy,dz,a,hb,c,hd;
	REAL x1,x2,dz1,dz2,cl,h1,h2,dx,ax,cl1,cl2,dx1,dx2;
	REAL area1,area2,hbeam1,hbeam2,totmom;
	extern int numtran;
	int save_stsec[maxlin],save_ensec[maxlin];

	density1 = -density1;

	if(count >= maxsec) {
		if(first_transom_warning) {
			message("Insufficient free space for transom in data tables:\n\nTransom not included in statics calculations");
			first_transom_warning = FALSE;
		}
		return;
	}
	if(maxsec - count < 10 && first_transom_warning) {
		message("Inadequate free space for transom in data tables:\n\nTransom statics will only be approximate");
		first_transom_warning = FALSE;
	}

	del_it = max(1,numtran / (maxsec - count));
	for(i = count, it = 0 ; it < numtran ; i++, it += del_it) {
		xsect[i] = xtran[it];
		zcut[i] = ztran[it];
		ycut[i] = ytran[it];
	}
	last = i - del_it;

	for(j = 0 ; j < numlin ; j++)
		refit(j,xsect,yline[j],zline[j],ycont[j],zcont[j],count,last);

	for(j = 0 ; j < numlin ; j++) {
		save_stsec[j] = stsec[j];
		save_ensec[j] = ensec[j];
		stsec[j] = count;
		ensec[j] = last;
	}

	dz2 = wll - beta*xsect[count];
	cl2 = cleara(count,dz2,&dy,&dz,&i,&j);
	x1 = xsect[count];
	area2 = 0.0;
	hbeam2 = 0.0;
	for(i = count ; i <= last ; i++) {

//	If transom slope is positive, cut off interpolated sections at the top. Otherwise, cut off their bottoms

		z = zcut[i];	// may be a problem here when the transom is wholly below the section
		y = zcut[i];

//	Find the first line "below" the transom intersection

		for(jj = 1 ; jj < numlin && zline[jj][i] < z ; jj++) ;

//	Obtain curve parameters before the change, to allow recalculation of the control point at the intersection
		if(jj < numlin) {
			getparam(i,jj,&a,&hb,&c,&hd);
			t = tvalue(y - yline[jj][i],zline[jj][i] - z,a,hb,c,hd);
		}
		if(atransom > 0.0) {

			if(jj < numlin) {
				zcont[jj][i] = zline[jj][i] - 0.5*t*c;
				ycont[jj][i] = yline[jj][i] + 0.5*t*a;

//	Set all hull lines above the transom at the intersection of the transom and the hull

				for(j = 0 ; j < jj ; j++) {
					zline[j][i] = z;
					yline[j][i] = y;
					zcont[j][i] = z;
					ycont[j][i] = y;
				}
			}		// otherwise, the transom plane is forward of the hull section - subtract all of it
		} else {

//	Find the curvature of where the transom cuts the hull section

			if(jj < numlin) {
				t = 1.0 - t;
				zcont[jj][i] = zline[jj-1][i] - 0.5*t*(c+2.0*hd);
				ycont[jj][i] = yline[jj-1][i] + 0.5*t*(a+2.0*hb);
				zline[jj][i] = z;
				yline[jj][i] = y;

//	Set all hull lines below the transom at the intersection of the transom and the hull

				for(j = jj+1 ; j < numlin ; j++) {
					zline[j][i] = z;
					yline[j][i] = 0.0;
					zcont[j][i] = z;
					ycont[j][i] = 0.0;
				}

			} else {		// otherwise, the transom plane is behind the hull section - subtract none of it
				continue;
			}
		}

		calcar(i,0,numlin,1,1,
			&area1,&area2,
			&hbeam1,&hbeam2,
			&cl1,&cl2,
			x1,xsect[i],
			xsum,volu1,calcdisp,
			&totmom,wplane1,
			wll,density1,lwl1,
			xsta,xend,sumlcf,summct,
			ww1,ww2,rm1,rm2,
			sumysqdif,sumyoffs);
		dz1 = dz2;					/* dz1 and dz2 are heights of line 0 above waterline, used in profile area calculations */
		dz2 = wll - (cosa*zline[0][i]-sina*yline[0][i] + beta*xsect[i]);
		dz = 0.5*(dz1+dz2);			/* mean "freeboard", positive	*/
		cl = 0.5*(cl1+cl2);			/* mean waterline clearance	*/
		h1 = dz1 - cl1;				/* depth of hull at previous section */
		h2 = dz2 - cl2;				/* depth of hull at current section */
		dx = xsect[i]-x1;			/* length of interval between previous and current sections */

//	All addition / subtraction terms are reversed here

		*AXabove -= 0.5*dx*(h1*(x1+0.33333*dx) + h2*(x1+0.66667*dx));
									/* area*distance contribution if hull remains clear of water */
		if(cl1 > 0.0) {				/* hull commenced clear of water */
			*Aabove1 -= dx*(dz-cl);	/* adjusted later if hull enters water */
			if(cl2 > 0.0) {			/* hull remained clear of water	*/
				*AZabove -= 0.16667*dx*((dz2*dz2 + dz1*dz2 + dz1*dz1) - (cl2*cl2 + cl1*cl2 + cl1*cl1));
									/* ... no change to AZbelow, AXabove or AXbelow */
			}
			else {					/* hull entered water: xsta is waterline entry point */
				dx2 = xsect[i] - *xsta; /* xsta is x-coordinate where hull enters water */
				dx1 = *xsta - x1;
				z = -dx2*0.5*cl2; 	/* cl2 < 0 - area aft of xsta, needs removing from Aabove1 */
				*Abelow1 -= z;		/* positive contribution to area below */
				*Aabove1 += z;		/* remove area not occupied */
				*AZbelow -= z*0.333333*cl2;/* negative contribution (z and cl2 negative) */
				ax = z*(*xsta + 0.66667*dx2); /* positive area term */
				*AXbelow -= ax;		/* positive contribution */
				*AXabove += ax;		/* negative contribution */
				if(dx != 0.0) {		/* normally the case, except for first run through loop */
					dz = dz1+(dz2-dz1)*dx1/dx;
					*AZabove -= 0.16667*(dx1*(dz1*dz1 + dz*dz1 + dz *dz - cl1*cl1) +
						    dx2*(dz *dz  + dz*dz2 + dz2*dz2));
				}
			}
		}
		else {						/* hull commenced in water */
			if(cl2 < 0.0) {			/* hull remained in water */
				*Abelow1 += dx*cl;	/* positive area */
				*AZbelow -= 0.16667*dx*(cl1*cl1+cl1*cl2+cl2*cl2);	/* cl1 and cl2 negative - RHS positive */
				*AZabove -= 0.16667*dx*(dz1*dz1+dz1*dz2+dz2*dz2);
				ax = -0.5*dx*(cl1*(x1+0.33333*dx)+cl2*(x1+0.66667*dx));	/* positive */
				*AXbelow -= ax;		/* ax positive */
				*AXabove += ax;
				*Aabove1 -= dx * cl;	/* cl negative */
			}
			else {		/* hull emerged from water */
				dx2 = xsect[j] - *xend;
				dx1 = *xend - x1;
				z = -dx1*0.5*cl1;			/* positive */
				*Abelow1 -= z;
				*AZbelow += 0.33333*z*cl1;	/* cl1 negative */
				ax = z*(x1+0.33333*dx1);	/* positive area*/
				*AXbelow -= ax;
				*AXabove += ax;
				*Aabove1 += z;
				if(dx != 0.0) {
					dz = dz1+(dz2-dz1)*dx1/dx;
					*AZabove -= 0.16667*(dx2*(dz2*dz2+dz*dz2+dz*dz - cl2*cl2)
						    + dx1*(dz*dz+dz*dz1+dz1*dz1));
				}
			}
		}
		x1 = xsect[j];
	}
	for(j = 0 ; j < numlin ; j++) {
		stsec[j] = save_stsec[j];
		ensec[j] = save_ensec[j];
	}
}

#endif
