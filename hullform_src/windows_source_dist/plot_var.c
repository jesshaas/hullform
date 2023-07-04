/* Hullform component - plot_var.c
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

BOOL CALLBACK PltDlgProc(HWND hWndDlg,unsigned msg,WORD wParam,
	LONG lParam);
int plotvar(HWND hWndDlg,int id);
REAL plotval(HWND hWndDlg,int id);

extern char *helpfile;
extern int dlgresult;
int GetPltDlg(void);
void plong(long value,int nsf);

/*	"PLOT ANYTHING AS FUNCTION OF ANYTHING FOR ANY RANGE OF ANYTHING" */

extern	REAL *xvalue,*yvalue;
extern	char	*var[];
char	xname[12] = "\0";
char	tname[12] = "\0";
char	yname[12] = "\0";
REAL	finval,area;
extern	REAL	xvmin,xvmax;
extern	REAL	yvmin,yvmax;
REAL	xatmaxgz,yatmaxgz;
extern	REAL	xscale,yscale;
void	finish_stats(void);
void	calctankstat(void);
MENUFUNC plot_hydros(void);

//	Parameters for hydrostatics plots

int		xvar = -1;
int		tvar = -1;
int		yvar = -1;
REAL	plot_start = 0.0;
REAL	plot_end = 0.0;
REAL	plot_inc = 1.0;
int		inum = 0;

//	Freeboard terms

int		fb_sect = 1;
int		fb_line = 0;

MENUFUNC plot_all()
{
	static int ifree = 0;			// minimum-freeboard flag
	extern char *var[];
	REAL   value;
	INT    i;
	REAL   sintem,costem,dissav,heesav,wlsav,pitsav,betasav,xcofmsav;
	REAL   calcdisp;
	REAL   fmn;
	extern int balanced;

	graphics = 1;
	gz = zmeta = wetsur = fm = 0.0;	/* may not be used */

	/*	Define plot parameters				*/

	if(!GetPltDlg() || tvar < 0 || xvar < 0 || yvar < 0) return;
	if(yvar == 19 || xvar == 19) {	// Freeboard
		fb_line++;
		if(!getdlg(STATFREE,
			INP_RBN,(void *) &ifree,
			INP_INT,(void *) &fb_sect,
			INP_INT,(void *) &fb_line,-1)) return;
		if(ifree == 0) {
			fb_sect = 0;
			fb_line = 0;
		}
		else if(fb_sect < 0 || fb_sect >= count || fb_line < 1 || fb_line > extlin) {
			message("Index out of range");
			return;
		}
		else {
			fb_line--;
		}
	}

	inum = 0;		// index in tables of values to plot

	/*	SAVE NORMAL VALUES OF HULL PARAMETERS	*/

	sintem   = sina;
	costem   = cosa;
	dissav   = disp;
	heesav   = heel;
	wlsav    = wl;
	pitsav   = pitch;
	betasav  = beta;
	xcofmsav = xcofm;

	/*	Enter values of principal parameters into array		*/

	varval[0] = wl;
	varval[1] = heel;
	varval[2] = disp;
	varval[3] = pitch;
	varval[4] = xcofm;

	/*	Loop through parameters range required			*/

	for(value = plot_start ; value <= plot_end ; value += plot_inc) {

		clrtext();
		pstr("CALCULATIONS FOR ");
		pstr(var[tvar]);
		pstr(" = ");
		prea("%12.3f",value);
		/*	Enter parameter value into table			*/

		varval[tvar] = value;
		if(tvar == 2 && disp < plot_inc*0.01) disp = plot_inc*0.01;

		/*	Get inclination parameters from heel and pitch		*/

		sina = sin(0.01745329*heel);
		cosa = cos(0.01745329*heel);
		beta = tan(0.01745329*pitch);

		/*	If stepping through heel or displacement, balance the hull	*/

		if(tvar == 1 || tvar == 2 || tvar == 4) {
			balanc(&calcdisp,1);
			if(calcdisp < 0.0) return;

			/*	Otherwise, simply calculate parameters for this condition	*/

		}
		else {
			huldis(&calcdisp);
		}
		if(calcdisp <= 0.0) return;

		/*	Only if necessary, find righting moment terms		*/

		if(yvar >= 8 && yvar <= 11 || xvar >= 8 && xvar <= 11)
			findrm();

		finish_stats();

		/*	Only if necessary, find wetted surface			*/

		if(yvar == 13 || xvar == 13)
			wetsur = hulsur(sina,cosa) + hulsur((REAL) (-sina),cosa);

		/*	Only if necessary, find freeboard and overall length	*/

		if((yvar == 19 || xvar == 19) && garea > 0.0 && lwl > 0.0) {
			if(fb_sect <= 0) {	/* if minimum */
				fm = wl-beta*(xsect[0]-yline[fb_line][0])-hwl[0]-
				    cosa*zline[fb_line][0];
				for(i = 1 ; i < count ; i++) {
					fmn = wl-beta*xsect[i]-hwl[i]-
					    cosa*zline[fb_line][i]-
					    sina*yline[fb_line][i];
					if(fmn < fm) fm = fmn;
				}
			}
			else {
				fm = wl-beta*xsect[fb_sect]-hwl[fb_sect]-
				    cosa*zline[fb_line][fb_sect]-
				    sina*yline[fb_line][fb_sect];
			}
		}

		calctankstat();

		/*	Extract required values from the table			*/

		yvalue[inum] = varval[yvar];
		xvalue[inum] = varval[xvar];

		if(++inum >= 100) break;

	}

	/*	Restore original settings			*/

	sina = sintem;
	cosa = costem;
	disp = dissav;
	heel = heesav;
	wl = wlsav;
	pitch = pitsav;
	beta = betasav;
	xcofm = xcofmsav;
	balanced = FALSE;

	plot_hydros();

}

MENUFUNC plot_hydros()
{
	char	str[40];
	char	text[40];
	REAL	xgz,ygz;
	int		i;
	REAL	dval;

	/*	PLOT THE RESULTS	*/

	(*init)();
	(*colour)(3);
	graph(xvalue,yvalue,inum,var[xvar],var[yvar],1.0e+30,1.0e+30);

	/*	If plotting righting moment from zero heel, show integrated	*/
	/*	area on plot, and maximum GZ value.				*/

	if(yvar == 8 && xvalue[0] == 0.0 && inum > 0) {
		area = 0.0;
		for(i = 1 ; i < inum ; i++) {
			finval = xvalue[i];
			if(yvalue[i] < 0.0) {
				dval = plot_inc*yvalue[i-1]/(yvalue[i-1]-yvalue[i]);
				area = area+yvalue[i-1]*dval*0.5;
				finval = xvalue[i-1]+dval;
				break;
			}
			else {
				area = area+plot_inc * (yvalue[i]+yvalue[i-1]) * 0.5;
			}
		}

		sprintf(str,"Area to %.1f deg = %.2f",finval,area);
		(*move)(-14.0,-11.0);
		(*plstr)(str);

		ygz = (yatmaxgz-yvmin)*yscale ;
		xgz = (xatmaxgz-xvmin)*xscale;
		(*move)(-xvmin*xscale,ygz);
		(*draw)(xgz,ygz);
		(*draw)(xgz,-yvmin*yscale);
		if(scrdev == 0)
			(*move)(0.02*(xvmax-xvmin)*xscale-xvmin*xscale,ygz+0.015*(yvmax-yvmin)*yscale);
		else
			(*move)(0.02*(xvmax-xvmin)*xscale-xvmin*xscale,ygz+0.07*(yvmax-yvmin)*yscale);
		sprintf(text,"Max. GZ %.2f %s @ %.1f deg.",yatmaxgz,lenun[numun],xatmaxgz);
		(*plstr)(text);
	}
	(*endgrf)();
	update_func = plot_hydros;
	print_func = update_func;
}

/*	EVALUATE AND PLOT X-Y CURVE FOR HULL				*/

void graph(REAL x[],REAL y[],INT n,char *xaxis,char *yaxis,REAL x0,REAL y0)
{
	extern	int	xtopleft,ytopleft,xbotright,ybotright,xmaxi,yvmaxi;
	REAL	xv,yv,extra;
	REAL	dx,dy;
	REAL	dxv,dyv;
	REAL	xm,ym;
	INT	ndpx,ndx,nsfx,ndpy,ndy,nsfy;
	REAL	xl;
	INT	l;
	INT	i;
	char	*nodata = "NO DATA TO PLOT";
	char	onechar[2] = " ";
	extern	int xleft,xright,ybottom,ytop;
	extern REAL PixelRatio;
	extern int repaint;
	extern REAL xgslope,ygslope;
	extern int xchar,ychar;
	REAL	dylabel;
	REAL	dxlabel;
	REAL	yshift;
	REAL	ar = (REAL) (xright - xleft) / (REAL) (ybottom - ytop) / PixelRatio;
#ifndef linux
	extern HDC hDC;
	HFONT hFont;
	HFONT OldFont;
	TEXTMETRIC tm;
	extern LOGFONT lf;
	LOGFONT rotf;
	extern int xchar,ychar;
#endif

	/*	SET UP SCREEN	*/

	cls(FALSE);
	perset(0);
	(*colour)(1);

	if(n <= 1) {
		message(nodata);
		return;
	}

	/*	FIND DATA LIMITS	*/

	xvmin = x0;
	xvmax = -xvmin;
	yvmin = y0;
	yvmax = xvmax;
	for(i = 0 ; i < n ; i++) {
		xv = x[i];
		yv = y[i];
		if(xvmin > xv)xvmin = xv;
		if(xvmax < xv)xvmax = xv;
		if(yvmin > yv)yvmin = yv;
		if(yvmax < yv) {
			yvmax = yv;
			xatmaxgz = xv;
		}
	}
	yatmaxgz = yvmax;
	if(xvmax <= xvmin) xvmax = xvmin + 1.0;
	if(yvmax <= yvmin) yvmax = yvmin + 1.0;

	/*	SET RANGE LIMITS TO ROUNDED VALUES	*/

	dxv = axincr(xvmax-xvmin);
	dyv = axincr(yvmax-yvmin);

	xm = (float) ((int) (xvmin/dxv)) * dxv;
	if(xvmin < 0.0) {
		xvmin = xm-dxv;
	}
	else {
		xvmin = xm;
	}

	xm = ((int)(xvmax/dxv))*dxv;
	if(xvmax > 0.0 && xm < xvmax-dxv*0.01) {
		xvmax = xm+dxv;
	}
	else {
		xvmax = xm;
	}

	ym = ((int) (yvmin/dyv)) * dyv;
	if(yvmin < 0.0) {
		yvmin = ym-dyv;
	}
	else {
		yvmin = ym;
	}

	ym = ((int) (yvmax/dyv)) * dyv;
	if(yvmax >= 0.0 && ym < yvmax-dyv*0.01) {
		yvmax = ym+dyv;
	}
	else {
		yvmax = ym;
	}

	xscale = 50.0/(xvmax-xvmin);
	yscale = 50.0/(yvmax-yvmin);
	dx = dxv * xscale;
	dy = dyv * yscale;
	if(ar < 1.0) {
		extra = 40.0/ar - 40.0;
		setranges(-15.0,65.0,-17.0-extra,63.0+extra);
	}
	else {
		extra = 40.0*ar - 40.0;
		setranges(-15.0-extra,65.0+extra,-17.0,63.0);
	}

	/*	DECIMAL PLACES,NUMBER OF DIGITS ABOVE DECIMAL POINT, AND TOTAL	*/
	/*	NUMBER OF FIGURES NEEDED	*/

	ndpx = - (int) log10(dxv);
	if(dxv < 1.0) ndpx++;
	ndx = log10(max(1.0,xvmax-xvmin))+2;
	nsfx = ndx+ndpx+1;

	ndpy = - (int) log10(dyv);
	if(dyv < 1.0) ndpy++;
	ndy = log10(max(1.0,yvmax-yvmin))+2;
	nsfy = ndy+ndpy+1;

	/*	LABEL X-AXIS	*/

	xm = xvmin;
	dxlabel = xchar/xgslope;
	dylabel = 0.8*ychar/ygslope;
	yshift = (scrdev == 0 ? 0.0 : dylabel);
	for(xv = 0.0 ; xv < 50.1; xv += dx) {
		(*move)(xv,0.0);
		(*draw)(xv,1.5);
		if(ndpx >= 1) {
			(*move)(xv-0.5*dxlabel*(nsfx + 1),yshift - 1.5*dylabel);
			plfix(xm,nsfx,ndpx);
		}
		else {
			(*move)(xv-0.5*dxlabel*(ndx + 1),yshift - 1.5*dylabel);
			plong((int) xm - (fneg(xm) ? 0.5 : -0.5),ndx);
		}
		xm += dxv;
	}

	/*	IF ZERO Y-VALUE IS IN RANGE, DRAW X-AXIS		*/

	if(yvmax > 0.0 && yvmin < 0.0) {
		(*move)(0.0,-yvmin*yscale);
		(*draw)(50.0,-yvmin*yscale);
	}

	/*	LABEL Y-AXIS	*/

	ym = yvmin;
	for(yv = 0.0 ; yv < 50.1 ; yv += dy) {
		(*move)(0.0,yv);
		(*draw)(1.5,yv);
		if(ndpy >= 1) {
			xl = - dxlabel*(float) (2 + nsfy);
			(*move)(xl,yv + yshift - 0.5*dylabel);
			plfix(ym,nsfy,ndpy);
		}
		else {
			xl = - dxlabel*(float) (2 + ndy);
			(*move)(xl,yv + yshift - 0.5*dylabel);
			plong( (long) (ym + (ym >= 0.0 ? 0.5 : -0.5)),ndy);
		}
		ym += dyv;
	}

	/*	WRITE AXIS TITLES	*/

	l = strlen(xaxis);
	(*move)(25.0-0.5*l,yshift - 3.0*dylabel);
	(*plstr)(xaxis);

	l = strlen(yaxis);
	xv = xl - 2.5;
#ifdef linux
	yv = 25.0 + yshift - 0.5 * dylabel * l;
	while(*yaxis) {
		(*move)(xv,yv);
		*onechar = *yaxis++;
		(*plstr)(onechar);
		yv -= dylabel;
	}
#else
	if(scrdev == 0) {
		yv = 25.0 + dylabel * l;
		while(*yaxis) {
			(*move)(xv,yv);
			*onechar = *yaxis++;
			(*plstr)(onechar);
			yv -= dylabel;
		}
	} else {
		memcpy((void *) &rotf,(void *) &lf,sizeof(LOGFONT));
		rotf.lfOrientation = 900;
		rotf.lfEscapement = 900;
		hFont = (HFONT) CreateFontIndirect(&rotf);
		OldFont =  (HFONT) SelectObject(hDC,hFont);
		(*move)(xv,25.0 - l*0.5*dxlabel);
		(*plstr)(yaxis);
		OldFont =  (HFONT) SelectObject(hDC,OldFont);
		if(OldFont != NULL) DeleteObject(OldFont);
	}
#endif

	/*	DRAW PLOT BOX OUTLINE	*/

	(*move)(0.0,0.0);
	(*draw)(0.0,50.0);
	(*draw)(50.0,50.0);
	(*draw)(50.0,0.0);
	(*draw)(0.0,0.0);

	/*	FINALLY, SHOW PROVIDED LINE	*/

	(*newlin)();
	(*colour)(1);
	for(i = 0 ; i < n ; i++)
		(*draw)((x[i]-xvmin)*xscale,(y[i]-yvmin)*yscale);
}

/*
Returns
xvar	x-variable index
yvar	y-variable index
tvar	t-variable index
plot_start	start value
plot_end	end value
plot_inc	calculation increment
*/

int GetPltDlg()
{
	char dummy[MAX_PATH] = "";
	int i,j,result;
	struct {
		int index;       /* index of result in table */
		char *string;    /* pointer to result */
		char *table[numvar+1]; /* table of strings for listbox, null string terminator */
	}
	xlist,ylist,tlist;
	char *tp;
	int seq[numvar];
	int ix,iy;

	for(i = 0 ; i < numvar ; i++) {
		xlist.table[i] = var[i];
		tlist.table[i] = var[i];
		seq[i] = i;
	}

	/*	Sort the second two tables		*/

	for(i = 0 ; i < numvar-1 ; i++) {
		for(j = 0 ; j < numvar-1-i ; j++) {
			if(strcmp(xlist.table[j],xlist.table[j+1]) > 0) {
				tp = xlist.table[j+1];
				xlist.table[j+1] = xlist.table[j];
				xlist.table[j] = tp;
				result = seq[j+1];
				seq[j+1] = seq[j];
				seq[j] = result;
			}
		}
	}

	ix = xvar;
	iy = yvar;
	for(i = 0 ; i < numvar ; i++) {
		ylist.table[i] = xlist.table[i];
		if(seq[i] == xvar) ix = i;
		if(seq[i] == yvar) iy = i;
	}
	xlist.table[numvar] = "";
	ylist.table[numvar] = "";
	tlist.table[6] = "";

	xlist.index = ix;
	xlist.string = dummy;
	ylist.index = iy;
	ylist.string = dummy;
	tlist.index = tvar;
	tlist.string = dummy;

	if((result = getdlg(PLOTVARI,
		INP_REA,&plot_start,
		INP_REA,&plot_end,
		INP_REA,&plot_inc,
		INP_CBX,&tlist,
		INP_CBX,&xlist,
		INP_CBX,&ylist,-1)) != 0) {
		xvar = seq[xlist.index];
		yvar = seq[ylist.index];
		tvar = tlist.index;
	}
	return result;
}

#endif
