/* Hullform component - shellexp.c
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

void stringer_param(REAL,REAL,REAL,REAL);
REAL curvedist(REAL,REAL *);
extern	int xleft,xright,ybottom,ytop;
extern REAL PixelRatio;
extern int scaled;

MENUFUNC shell_expansion()
{
	REAL (*yoff)[maxsec],width;
	REAL aa,cc,a,hb,c,hd,x,y,xs,ys,xsize,ysize,xf,xb,yl,yr;
	REAL *dummy;
	REAL ignore,step,strloc,xsave;
	int i,j,jj,k,jint,jst,ens;
	extern int numbetw;
	REAL ar = (REAL) (xright - xleft) / (REAL) (ybottom - ytop) / PixelRatio;

	if( !memavail((void **) &yoff,maxlin*sizeof(*yoff)) ||
		!memavail((void **) &dummy,sizeof(*dummy)*maxsec*(numbetw+1)+1)) {
		message("Insufficient memory for shell expansion");
		return;
	}

	graphics = 1;
	update_func = shell_expansion;
	print_func = update_func;
	perset(0);
	(*init)();
	cls(0);

	/*	Find distance out to lines all transverse sections		*/

	y = 0.0;
	for(i = 1 ; i < count ; i++) {
		width = 0.0;
		aa = 0.0;
		cc = 1.0;
		yoff[0][i] = 0.0;
		y = 0.0;
		for(j = 1 ; j <= stemli ; j++) {
			if(stsec[j] <= i && ensec[j] >= i) {
				hullpa(i,j,aa,cc,&a,&hb,&c,&hd);
				tranpa(a,hb,c,hd,&aa,&cc);
				stringer_param(a,hb,c,hd);
				width += curvedist(1.0,&ignore) - curvedist(0.0,&ignore);
			}
			yoff[j][i] = width;
			if(width > y) y = width;
		}
		width += yline[stemli][i];
		for(j = 0 ; j <= stemli ; j++) {
			if(stsec[j] <= i && ensec[j] >= i) yoff[j][i] = width - yoff[j][i];
		}
	}

	/*	Calculate plotting region scale		*/

	x = xsect[0] - dxstem();	/* foremost point */
	xf = xsect[count-1] - x;
	xs = xf*1.2;
	ys = y*1.2;
	if(ar*ys > xs) {
		ysize = ys;
		xsize = ysize * ar;
	}
	else {
		xsize = xs;
		ysize = xsize / ar;
	}
	xf = x - xf*0.1 - 0.5*(xsize-xs);
	xb = xf + xsize;

	ys = 0.5*(ysize - ys);
	yl = -ys;
	yr = yl + ysize;

	(*colour)(3);
	if(posdir < 0.0)
		setranges(xf,xb,yl,yr);
	else
	    setranges(xb,xf,yl,yr);

	(*move)(x,0.0);
	(*draw)(xsect[count-1],0.0);

	/*	Plot the lines		*/

	radstem[extlin] = 0.0;
	for(j = 0 ; j < stemli ; j++) {
		for(i = max(1,stsec[j]) ; i <= ensec[j] ; i++) {
			zline[extlin][i] = yoff[j][i];
			linewt[extlin][i] = linewt[j][i];
		}
		if(stsec[j] <= 0) {
			zline[extlin][0] = zline[stemli][0] - zline[j][0];
			yline[extlin][0] = yline[j][0];
			linewt[extlin][0] = linewt[j][0];
		}
		stsec[extlin] = stsec[j];
		ensec[extlin] = ensec[j];
		draw_line(extlin,1.0,1.0,2,stsec[extlin],ensec[extlin],999,dummy,dummy,dummy,&jj,&jj,&jj,TRUE);
	}

	/*	Show the sections	*/

	(*colour)(2);
	for(i = 1 ; i < count ; i++) {
		xs = xsect[i];
		(*move)(xs,0.0);
		(*draw)(xs,yoff[0][i]);
	}

	/*	Plot stringers		*/

	(*colour)(6);
	for(j = 1 ; j < numlin ; j++) {
		if(numstr[j] > 0) {

			/*	strloc is stringer count (strmode == 1) or position (strmode == 0) across the surface,
			starting at the first fraction or interval, incrementing by the fraction of the surface
			per stringer or the interval respectively.
			*/
			if(strmode[j] == 0) {	/* fixed interval */
				strloc = str_firstfrac[j];
				step = str_interv[j];

				/*	step is negative when stringers run from the start line (jst) inward to the centreline	*/

				if(str_dir[j] == 1) {
					strloc = -strloc;
					step = -step;
				}
			}
			else {
				a = str_firstfrac[j] + str_interv[j];
				strloc = str_firstfrac[j] / a;
				step = 1.0 / a;
			}

			jint = j + str_dir[j] - 1;	/* str_dir = 1 - lower to higher index, 0 - higher to lower */
			jst = j - str_dir[j];

			for(k = inistr[j] ; k < inistr[j] + numstr[j] ; k++) {

				/*	Plot stringer start	*/

				/*	jint is the line which the stringers may intercept, when their interval is constant	*/
				(*newlin)();
				x = ststrx[k];
				i = ststr[k];
				if(strmode[j] == 0 && x != xsect[i]) {	/* strmode = 0 implies constant interval */
					y = yoff[jint][i] + (yoff[jint][i+1]-yoff[jint][i]) * (x-xsect[i])/(xsect[i+1]-xsect[i]);
					(*draw)(x,y);
					i++;
				}

				/*	Plot stringer interior points		*/

				while(i < enstr[k]) {
					if(strmode[j] != 0)	/* constant count, varying interval */
						(*draw)(xsect[i],yoff[jst][i] + strloc*(yoff[jint][i]-yoff[jst][i]));
					else		/* constant interval */
					(*draw)(xsect[i],yoff[jst][i] + strloc);
					i++;
				}

				/*	Plot stringer end			*/

				if(strmode[j] == 0) {	/* fixed spacing */
					x = enstrx[k];
					if(x != xsect[i]) {	/* stringer ends on line jint */
						y = yoff[jint][i-1] + (yoff[jint][i]-yoff[jint][i-1]) * (x-xsect[i-1])/(xsect[i]-xsect[i-1]);
						(*draw)(x,y);
					}
					else {
						(*draw)(x,yoff[jst][i] + strloc);
					}
				}
				else {		/* constant count */
					(*draw)(xsect[i],yoff[jst][i] + strloc*(yoff[jint][i]-yoff[jst][i]));
				}
				strloc += step;
			}
		}
	}

	/* draw the stem */

	(*colour)(2);
	if(posdir < 0.0)
		setranges(xf-xsect[0],xb-xsect[0],zline[stemli][0]-yl,zline[stemli][0]-yr);
	else
	    setranges(xb-xsect[0],xf-xsect[0],zline[stemli][0]-yl,zline[stemli][0]-yr);
	drasec(0,1,-1.0,0);

	(*endgrf)();

	/*	Free working memory before returning	*/

	memfree(yoff);
	memfree(dummy);
	scaled = FALSE;
}

#endif
