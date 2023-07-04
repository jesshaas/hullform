/* Hullform component - drag_men.c
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

#ifdef linux
#undef tc
#include <GL/gl.h>
#include <GL/glu.h>
#else
#include <GL\gl.h>
#include <GL\glu.h>
#endif

void SwapBuf(void);

#ifdef PROF

extern	int	hcpydev;
extern	int	speedunit;
int define_prop();

REAL	speed = 6.0;
void clrline(int line);
void setranges(REAL xl,REAL xr,REAL yb,REAL yt);
void find_savbro(void);
void find_holtrop(void);
REAL gerrit96(REAL speed,INT ind);
void find_gerrit96(void);
void do_balance(void);

#define NUMSCHEME 6
int dragscheme[] = {
	0,0,0,0,0,0};
char *dragtext[] = {
	"Gerritsma 1981",
	"Gerritsma 1996",
	"Holtrop & Mennen",
	"Oortmerssen",
	"Savitsky",
	"Savitsky & Brown"};
REAL_FUNC dragfunc[] = {
	gerrit,gerrit96,holtro,oortme,savits,prepla};

/* Gerritsma, Holtrop&Mennen, Oortmerssen, Savitsky, Savitsky&Brown */

FUNC_PTR find_sub[] = {
	find_gerrit,
	find_gerrit96,
	find_oortmerrssen,
	find_holtrop,
	find_savitsky,
	find_savbro,
};

FILE *dbg;

/*      Drag menu                                                       */

void set_speed()
{
	extern int speedunit;
	int	temp = speedunit;
	extern REAL speed;
	REAL sp = speed;

	if(getdlg(DRAGSPEE,
		INP_REA,(void *) &sp,
		INP_MEN,(void *) &temp,-1)) {
		speedunit = temp;
		speed = sp;
		if(speed <= 0.0) speed = 0.1;
	}
}

void show_areas()
{
	REAL	dz2,hb1,hb2,wetwid,disc1,disc2,cl1,cl2;
	REAL	areain[maxsec],areaout[maxsec];
	INT		nl,il,i,j;
	REAL	del,dely,dymin,yprev,dx,delx,x,y1,y2,ym,
	ratio,ylow,zlow;
	extern int	xleft,xright,ytop,ybottom;
	REAL	xl,xr,yt,yb;
	extern int	device,hcpydev;
	REAL	areain1,areaout1;
	REAL	areain2,areaout2;
	extern int	xchar,ychar;
	char	text1[24],text2[12];
	int		interval;

	graphics = 1;
	update_func = show_areas;
	print_func = show_areas;

	if(count <= 1) return;

	/*	area at base of first section					*/

	xsect[maxsec-1] = xsect[0];
	for(j = 0 ; j < numlin ; j++) {
		refit(j,xsect,yline[j],zline[j],ycont[j],zcont[j],maxsec-1,maxsec-1);
	}
	dz2 = wl-(beta*xsect[0]+hwl[0]);
	hullar(maxsec-1,0,numlin,&areain1,&areaout1,dz2, sina,cosa,&hb1,&wetwid,&disc1,&disc2,&il);
	hullar(maxsec-1,0,numlin,&areain2,&areaout2,dz2,-sina,cosa,&hb2,&wetwid,&disc1,&disc2,&il);
	areain[0] = fadd(areain1,areain2);
	areaout[0] = fadd(areaout1,areaout2);
	garea = max(areain[0],areaout[0]);

	/*	find other areas						*/

	for(i = 1 ; i < count ; i++) {
		dz2 = wl-(beta*xsect[i]+hwl[i]);
		hullar(i,0,numlin,&areain1,&areaout1,dz2, sina,cosa,&hb1,&wetwid,&disc1,&disc2,&il);
		hullar(i,0,numlin,&areain2,&areaout2,dz2,-sina,cosa,&hb2,&wetwid,&disc1,&disc2,&il);
		areain[i] = fadd(areain1,areain2);
		areaout[i] = fadd(areaout1,areaout2);
		if(garea < areain[i]) garea = areain[i];
		if(garea < areaout[i]) garea = areaout[i];
	}

	if(garea <= 0.0) {
		message("Hull is clear of water");
		return;
	}

	/*	initialise graphics screen				*/

	perset(0);
	cls(FALSE);
	(*init)();
	(*colour)(2);
	xl = 0.2*(xsect[count-1]-xsect[0]);
	xr = xsect[count-1] + xl;
	xl = xsect[0] - xl;

	yt = 1.3*garea;
	yb = -0.3*garea;

	/*	draw axes						*/

	delx = (REAL) xchar / (REAL) (xright - xleft) * (xr - xl);
	dely = (REAL) ychar / (REAL) (ytop - ybottom) * (yb - yt);
	if(posdir > 0.0) {
		setranges(xl,xr,yb,yt);
		x = xsect[0];
		dx = xsect[count-1];
	}
	else {
		setranges(xr,xl,yb,yt);
		x = xsect[count-1];
		dx = xsect[0];
		delx = -delx;
	}
	(*move)(x,garea);
	(*draw)(x,0.0);
	(*draw)(dx,0.0);
	dz2 = wl-beta*xsect[0]-hwl[0];
	cl2 = cleara(0,dz2,&ylow,&zlow,&nl,&il);
	(*move)(xsect[0],areaout[0]);
	del = 0.1*garea;

	dymin = (1.4/22.0)*garea;
	yprev = -1.0e+16;
	interval = max(1,count/20);
	for(i = 1 ; i < count ; i++) {
		cl1 = cl2;
		dz2 = wl-beta*xsect[i]-hwl[i];
		cl2 = cleara(i,dz2,&ylow,&zlow,&nl,&il);
		if(cl1*cl2 < 0.0) {
			x = (xsect[i]*cl1-xsect[i-1]*cl2)/(cl1-cl2);
			(*draw)(x,0.0);
		}
		y1 = areain[i];
		y2 = areaout[i];
		ym = max(y1,y2);
		x = xsect[i];
		(*draw)(x,y1);
		(*draw)(x,y2);
		ratio = ym/garea;
		if(ratio >= 1.0) yprev = -1.0e+16;
		if(ym > 0.0 && (ym-yprev < -dymin || ym-yprev > dymin)) {
			if((posdir > 0.0) == (ym > areaout[i-1] && ratio < 1.0)) {
				(*move)(x,ym-del);
			}
			else {
				(*move)(x,ym+del+dely);
			}
			sprintf(text1,"%.3f",ym);
			j = (int) (strchr(text1,'.') - text1);
			if(j >= 4) {
				text1[j] = '\0';
			}
			else {
				text1[6] = '\0';
			}
			yprev = ym;
			sprintf(text2," (%.1f%%)",ratio*100.0);
			strcat(text1,text2);
			plstr(text1);
		}
		(*move)(x,max(y1,y2)+del);
		(*draw)(x,min(y1,y2)-del);
		(*move)(x,0.0);
		if(i % interval == 0) {
			(*draw)(x,0.5*del);
			(*move)(x-delx,-2.0*dely);
			plint(i,2);
		}
		(*move)(x,y2);
	}
	(*endgrf)();
}

void find_savitsky()
{
	extern	REAL	tocons[2][2];

	do_balance();
	clrtext();
	if(hardcopy || define_prop())
		(void) savits(tocons[numun >> 1][speedunit]*speed,0);
	update_func = find_savitsky;
	print_func = find_savitsky;
	SwapBuf();
}

void find_savbro()
{
	extern	REAL	tocons[2][2];

	do_balance();
	clrtext();
	(void) prepla(tocons[numun >> 1][speedunit]*speed,0);
	update_func = find_savbro;
	print_func = find_savbro;
	SwapBuf();
}

void find_holtrop()
{
	extern	REAL	tocons[2][2];

	do_balance();
	clrtext();
	(void) holtro(tocons[numun >> 1][speedunit]*speed,0);
	update_func = find_holtrop;
	print_func = find_holtrop;
	SwapBuf();
}

void find_gerrit()
{
	extern	REAL	tocons[2][2];

	do_balance();
	clrtext();
	(void) gerrit(tocons[numun >> 1][speedunit]*speed,0);
	update_func = find_gerrit;
	print_func = find_gerrit;
	SwapBuf();
}

void find_gerrit96()
{
	extern	REAL	tocons[2][2];

	do_balance();
	clrtext();
	(void) gerrit96(tocons[numun >> 1][speedunit]*speed,0);
	update_func = find_gerrit96;
	print_func = find_gerrit96;
	SwapBuf();
}

void find_oortmerrssen()
{
	extern	REAL	tocons[2][2];

	do_balance();
	clrtext();
	(void) oortme(tocons[numun >> 1][speedunit]*speed,0);
	update_func = find_oortmerrssen;
	print_func = find_oortmerrssen;
	SwapBuf();
}

/*	produce a drag plot for two formulations		*/

static char speedlabel[14] = "SPEED, ?? / s";
static char forcelabel[12] = "DRAG, ?????";
char *kt = "kt";
REAL		tometr;

MENUFUNC plot_existing_drag(void);
extern REAL	tocons[2][2];
extern int	context_id;
extern REAL	xgslope,ygslope;
extern int	balanced;

int		tindex = 0;
REAL	staval = 0.0,endval = 0.0,delval = 1.0;

void plot_drag()
{
	REAL x1;

	/*	Select parameter to vary, and range				*/
	/*	May use current values only, or a range of displacement or	*/
	/*	centre of mass							*/

	if(!getdlg(DRAGPLOT,
		INP_LOG,(void *) &dragscheme[0],
		INP_LOG,(void *) &dragscheme[1],
		INP_LOG,(void *) &dragscheme[2],
		INP_LOG,(void *) &dragscheme[3],
		INP_LOG,(void *) &dragscheme[4],
		INP_LOG,(void *) &dragscheme[5],
		INP_REA,(void *) &speed,
		INP_RBN,(void *) &speedunit,
		INP_REA,(void *) &staval,
		INP_REA,(void *) &endval,
		INP_REA,(void *) &delval,
		INP_RBN,(void *) &tindex,
		-1)) return;
	graphics = TRUE;

	if(speed <= 0.0) speed = 0.1;
	update_func = plot_existing_drag;
	print_func = plot_existing_drag;

	tometr = tocons[numun >> 1][speedunit];

	if(speedunit == 1) {
		strcpy(speedlabel+7,kt);
	}
	else {
		strcpy(speedlabel+7,lenun[numun]);
		strcat(speedlabel+8,"/s");
	}
	strcpy(forcelabel+6,masun[numun]);

	if(fgt(staval,endval)) {
		x1 = staval;
		staval = endval;
		endval = x1;
	}
	delval = fabs(delval);
	if(tindex == 0) endval = staval;
	plot_existing_drag();
}

/*
Because parameters can be plotted for an arbitrary range of displacements or LCG's, we have to do
the calculations with the plotting
*/

MENUFUNC plot_existing_drag()
{
	char		str[20];
	REAL		maxdrag;
	int			imaxdrag;
	int			ind,i,j;
	REAL		drag_value[NUMSCHEME][40];
	REAL		drag_speed[NUMSCHEME][40];
	INT			drag_count[NUMSCHEME];
	int			indic;	// drag scheme startup flag (1 = startup, 2 = continuing)
	REAL		dsp,sp,av;
	REAL		y1;
	extern REAL	xvmin,yvmin,xscale,yscale;
	REAL		val;
	REAL		savedisp = disp;
	REAL		savexcofm = xcofm;
	extern int	scrcopy;
	REAL		calcdisp;
	int			first_curve;

	maxdrag = 0.0;
	balanced = 0;
	first_curve = 1;
	(*init)();

	//	Loop through the required range of parameter (displacement, LCG) values

	for(val = endval ; val >= staval - 0.5 * delval ; val -= delval) {

		sprintf(str,"%.1f",val);
		if(tindex == 1)		 disp = val;	// displacement range
		else if(tindex == 2) xcofm = val;	// LCG range

		/*	balance the hull if required		*/

		if(tindex > 0 || !balanced) {
			scrcopy = 0;
			for(i = 1 ; i < 4 ; i++) clrline(i);
			balanc(&calcdisp,0);
			if(calcdisp < 0.0) {
				disp = savedisp;
				xcofm = savexcofm;
				return;
			}
			if(volu <= 0.0) continue;
			wetsur = hulsur(sina,cosa)+hulsur(-sina,cosa);
		}

		/*  calculate drag curves for this configuration and the required speeds */

		dsp = speed*0.025;
		for(i = 0 ; i < NUMSCHEME ; i++) {
			drag_count[i] = 0;
		}
		indic = 1;
		for(sp = dsp ; sp < speed+0.5*dsp ; sp += dsp) {
			for(i = 0 ; i < NUMSCHEME ; i++) {
				if(dragscheme[i]) {
					av = (*dragfunc[i])(tometr*sp,indic);
					if(av > 0.0) {
						if(av > maxdrag && first_curve) {
							maxdrag = av;
							imaxdrag = i;
						}
						drag_value[i][drag_count[i]] = av;
						drag_speed[i][drag_count[i]++] = sp;
					}
				}
			}
			indic = 2;
		}

		//	For the first plot set, use the highest plot to call the graphing routine

		if(first_curve) {
			graph(drag_speed[imaxdrag],drag_value[imaxdrag],drag_count[imaxdrag],speedlabel,forcelabel,0.0,0.0);
			//	Show the parameter value if a range is being used (tindex = 1 or 2)
			if(tindex > 0) plstr(str);
		}

		ind = 2;	// colour index
		for(i = 0 ; i < NUMSCHEME ; i++) {	// Add all curves other than the maximum drag one (already plotted)
			if(dragscheme[i]) {
				if(i == imaxdrag)
					(*colour)(1);
				else
				    (*colour)(ind++);

				if(i != imaxdrag || !first_curve) {
					(*newlin)();
					for(j = 0 ; j < drag_count[i] ; j++) {
						(*draw)((drag_speed[i][j]-xvmin)*xscale,(drag_value[i][j]-yvmin)*yscale);
					}
					if(drag_count[i] > 1 && tindex > 0) plstr(str);
				}
			}
		}
		first_curve = 0;
	}

	//	Show legend

	y1 = -15.0;
	ind = 2;
	for(i = 0 ; i < NUMSCHEME ; i++) {
		if(dragscheme[i]) {
			if(i == imaxdrag)
				(*colour)(1);
			else
			    (*colour)(ind++);

			(*move)(-30.0,y1);
			(*draw)(-21.0,y1);
			(*move)(-20.0,y1+1.5);
			(*plstr)(dragtext[i]);
			y1 += 3.0;
		}
	}

	(*endgrf)();
	xcofm = savexcofm;
	disp = savedisp;
}

/*	get new values for epsilon and f for use by Savitsky scheme	*/

int define_prop()
{
	extern REAL	epsilon,dist_down;
	extern int	context_id;
	extern int dlgboxpos;
	context_id = 5002;
	dlgboxpos = 1;
	return getdlg(DRAGSAVI,INP_REA,(void *) &epsilon,
		INP_REA,(void *) &dist_down,-1);
}

#endif

void clrline(int line)
{
	RECT rc;
	extern COLORREF scrcolour[];
	extern int xleft,xright;
	extern HDC hDC;
	extern int ychar;
	extern int ycursor;
	extern HBRUSH bg;
	int col,r,g,b;

	rc.top = ychar * line;

#ifndef linux
	if(scrdev == 1) {
		rc.left = xleft;
		rc.right = xright;
		rc.bottom = rc.top + ychar;
		FillRect(hDC,&rc,bg);
	}
	else {
		glDisable(GL_LIGHTING);
		col = scrcolour[0];
		r = ((float) GetRValue(col)) / 255.0;
		g = ((float) GetGValue(col)) / 255.0;
		b = ((float) GetBValue(col)) / 255.0;
		glColor3i(r,g,b);
		glBegin(GL_POLYGON);
		glVertex2i(xleft, ychar*(line-1));
		glVertex2i(xright,ychar*line);
		glVertex2i(xright,ychar*line);
		glVertex2i(xleft, ychar*(line-1));
		glEnd();
	}
#endif
	ycursor = rc.top;
}

