/* Hullform component - deve_men.c
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
#include <X11/keysym.h>
extern Display *display;

#else

#include <GL\gl.h>
#include <GL\glu.h>

#endif

#include "plate.h"

void set_GL_perspective(void);
void SwapBuf(void);
void GLin(void);

#ifdef PLATEDEV

void xor_on(void);
void xor_off(void);
int alloc_dev(void);
void free_dev(void);
void plot_devstr(int k,int mode,REAL *xrate,REAL *yrate,REAL *zrate,int *num,
	int *stind,int *trind,REAL side);
void scrtrans(REAL xs,REAL ys,REAL zs,int *xa,int *ya);
void highlight_ruling(int k,int i);
void show12(int k,int i,int col);
void draw_anyline(int la,REAL *xrate,REAL *yrate,REAL *zrate,int *numint,
	int *stem_ind,int *tran_ind,int mode,REAL side);
extern int	scaled;
extern void xorcol(int);
int la,lb;
int ruling_index;			/* index of marked ruling */
int old_ruling_index;
extern int undoruling;

int	numinta;		/* number of interpolated sections	*/
int	numintb;		/* number of interpolated sections	*/
int	itransom;		/* transom index in table of rulings	*/
int	tran_ind;		/* index of transom (will be less than	count-1 if that section ignored)	*/
int	stem_ind;		/* index of "stem" (will be more than 0	if stem is "ignore"d			*/
int	num1a;
int	num1b;			/* number of interpolated sections used	*/
int	numrul;			/* number of rulings found		*/
extern int tran_lines;		/* default of 5 lines to transom and stem */

int kdev;

extern	int	indx;
extern	int	plate_lin;	/* higher index of pair of lines enclosing surface to be developed	*/
extern	int	plate_res;	/* "plate resolution":  number of plate	lines per section along hull		*/
extern	REAL	pp;		/* picture plane slope			*/
extern	REAL	sinpp,cospp;	/* sine and cosine of picture plane angle */
extern	REAL	yview,zview;	/* viewing position			*/
extern	REAL	rotn;		/* rotation of image			*/
extern	REAL	x1,x2,yy1,y2;	/* graphics limits in viewport		*/
extern	REAL	xmin0,xmax0,ymin0,ymax0;
extern	REAL	heelv;
REAL	*xrate_a = NULL,*yrate_a = NULL,*zrate_a = NULL;	/* parametric derivatives for x, y and z along chine a */
REAL	*xrate_b = NULL,*yrate_b = NULL,*zrate_b = NULL;	/* parametric derivatives for x, y and z along chine b */
int	together[maxsec];										/* lines together: "ignore"	*/
int	setnum;													/* ruling set number	*/
int	tran_lines;

/*	Calculate development lines			*/

MENUFUNC calc_devel()
{
#ifdef PROF
	extern	int transom;
	int	old_transom = transom;
#endif

	extern int *ignore;
	int	i,j,k;

	REAL	a,hb,c,hd;	/* section curvature parameters	*/
	REAL	temp,tempx;
	REAL	value,del;

	REAL	prev,cur;
	REAL	norm_x,norm_y,norm_z;	/* normal vector components */
	extern	int xmaxi,ymaxi,xchar,ychar;
	REAL	xs;
	extern	int strlin,strind;
	clock_t start;

	if(count <= 0) return;

	(*init)();
	cls(FALSE);

	la = plate_lin;
	lb = plate_lin-1;		/* line indices */
	del = 0.999/((float) (max(1,tran_lines-1)));

	itransom = -1;			/* default is no transom ruling */
#ifdef PROF
	transom = 0;			/* temporarily remove transom slope */
#endif

	/*	allocate or reallocate memory for table of parametric rates	*/

	if(!alloc_dev()) return;

	/*	Force ignorance of all pairs of sections for which the lines	*/
	/*	coincide.  (Note that "ignore" is invoked only if adjacent	*/
	/*	sections are marked to be ignored)				*/

	for(i = max(stsec[la],stsec[lb]) ; i <= min(ensec[la],ensec[lb]) ; i++)
		together[i] = yline[la][i] == yline[lb][i] && zline[la][i] == zline[lb][i];

	waitcursor();

	/*	Set up x-values along lower chine (a)			*/

	/*	"la" is higher-indexed line */

	if(stsec[la] == stsec[lb])	/* lines start at same section - look for points along transverse curve */
		indx = tran_lines;
	else
		indx = 0;
	draw_anyline(la,xrate_a,yrate_a,zrate_a,&numinta,
		&stem_ind,&tran_ind,8,1.0);
	num1a = numinta - tran_lines;

	/*	Set up x-values along upper chine (b)			*/

	/*	"lb" is lower-indexed line */

	indx = 0;
	draw_anyline(lb,xrate_b,yrate_b,zrate_b,&num1b,&k,&k,9,1.0);
	if(ensec[la] == ensec[lb])	/* lines end at same section - look for points along transverse curve */
		numintb = num1b + tran_lines;
	else
	    numintb = num1b;

	/*	For stem curve section, y = ystem and yrate = 0, x and z are	*/
	/*	quadratic with interpolation parameter, xrate and zrate are	*/
	/*	linear.								*/

	if(stsec[la] == stsec[lb]) {
#ifdef PROF
		if(la < extlin) {
#endif
			getparam(stem_ind,la,&a,&hb,&c,&hd);
			xs = xsect[stem_ind];
#ifdef PROF
		}
		else {	/* stringer code */
			a = yline[lb][stem_ind] - yline[la][stem_ind];
			hb = 0.0;
			c = zline[la][stem_ind] - zline[lb][stem_ind];
			hd = 0.0;
			j = inistr[strlin] + strind;
			xs = ststrx[j];
		}
#endif
		value = 1.0;/* "value" ranges between 1 and 0 down the curve */
		tempx = hb + hb;
		temp = hd + hd;

		if(!ignore[stem_ind]) {

			for(j = 0 ; j < tran_lines ; j++) {

				/*	Find coordinates and derivatives along stem section curve	*/

				if(stem_ind == 0) {	/* true stem */
					xint_a[j] = xs-yline[la][0]-value*(a+value*hb);
					yint_a[j] = yline[stemli][1];
					xrate_a[j]= a + value*tempx;
					yrate_a[j]= 0.0;
				}
				else {		/* line ends at transverse section */
					xint_a[j] = xs;
					yint_a[j] = yline[la][stem_ind]+value*(a+value*hb);
					xrate_a[j]= 0.0;
					yrate_a[j]= -a - value*tempx;
				}
				zint_a[j] = zline[la][stem_ind]-value*(c+value*hd);
				zrate_a[j]= c+value*temp;
				value -= del;
			}
		}
	}

	/*	For transom curve section, x = xsect[count-1], xrate = 0, y	*/
	/*	and z are quadratic with interpolation parameter, yrate and 	*/
	/*	zrate are linear.						*/

	if(ensec[la] == ensec[lb] && !ignore[tran_ind]) {
		if(la < extlin) {
			getparam(tran_ind,la,&a,&hb,&c,&hd);
			xs = xsect[tran_ind];
		}
		else {
			a = yline[lb][tran_ind] - yline[la][tran_ind];
			hb = 0.0;
			c = zline[la][tran_ind] - zline[lb][tran_ind];
			hd = 0.0;
		}
		getparam(tran_ind,la,&a,&hb,&c,&hd);
		value = 0.0;/* "value" ranges between 1 and 0 down the curve */
		for(j = numintb-1 ; j >= num1b ; j--) {

			/*		Find coordinates and derivatives along transom	*/
			/*			section curve				*/

			xint_b[j] = xsect[tran_ind];
			yint_b[j] = yline[la][tran_ind]+value*(a+value*hb);
			zint_b[j] = zline[la][tran_ind]-value*(c+value*hd);
			xrate_b[j]= 0.0;
			yrate_b[j]= -a - value*(hb+hb);
			zrate_b[j]=  c + value*(hd+hd);
			value += del;
		}
	}

	/*	*** GET THE NORMAL VECTOR OF A PT ***

	1.	Find the vector from an initial point on chine B to another
	point on chine a.
	2.	Using this vector and the tangent vector at a point on
	chine a, determine the normal vector.
	3.	Through change of sign of the dot product of the normal
	vector and the tangent on chine b, a ruled line for a
	developable surface can be traced.

	This line is then interpolated to.

	*/

	numrul = 1;		/* ruling line array index.  If no	*/
	/* ruled lines are found, numrul = 1	*/

	i = numinta - 1;
	xstart[setnum][0] = xint_a[i];
	ystart[setnum][0] = yint_a[i];
	zstart[setnum][0] = zint_a[i];

	j = numintb - 1;
	xend[setnum][0] = xint_b[j];
	yend[setnum][0] = yint_b[j];
	zend[setnum][0] = zint_b[j];

	for(j = numintb-1 ; j >= 0 ; j--) {

		/*	getnorm takes end co-ordinates (?int_?), and parametric		*/
		/*	derivatives along chine a (?rate_?), and returns a normal	*/
		/*	vector at chine a, (norm_x,norm_y,norm_z)			*/

		i = numinta - 1;
		getnorm(&norm_x,&norm_y,&norm_z,
			xrate_a[i],yrate_a[i],zrate_a[i],
			xint_b[j],yint_b[j],zint_b[j],
			xint_a[i],yint_a[i],zint_a[i]);

			/*	determine the current dot product between this normal vector	*/
			/*	and the tangent along chine b.									*/

		prev =	(norm_x*xrate_b[j])+
			    (norm_y*yrate_b[j])+
		    	(norm_z*zrate_b[j]);

		/*	Search along all interpolated points on chine a,	*/
		/*	(xint,yint_a,zint_a)					*/

		for(i = numinta-2 ; i >= 0 ; i --) {

			/*	Find the vector on chine a normal to chine a and the line from	*/
			/*	chine b.							*/

			/*	getnorm takes end co-ordinates (?int_?), and parametric		*/
			/*	derivatives along chine a (?rate_?), and returns a normal	*/
			/*	vector at chine a, (norm_x,norm_y,norm_z)			*/

			getnorm(&norm_x,&norm_y,&norm_z,
				xrate_a[i],yrate_a[i],zrate_a[i],
				xint_b[j],yint_b[j],zint_b[j],
				xint_a[i],yint_a[i],zint_a[i]);

			/*	determine the current dot product between this normal vector	*/
			/*	and the tangent along chine b.									*/

			cur =	(norm_x*xrate_b[j])+
				    (norm_y*yrate_b[j])+
			    	(norm_z*zrate_b[j]);

			/*	if DOT sign has changed, then call subroutine intchia, that	*/
			/*	interpolates point on chine a.					*/

			if(prev*cur < 0.0 || cur == 0.0) {

				/*	Can get more than one ruling per point, so must have extra	*/
				/*	watch point here						*/

				if(numrul >= MAXRUL) {
					message("RULING LINE LIMIT EXCEEDED\nThis suggests the surface is\nprobably not developable");
					goto breakout;
				}

				/*	If a valid ruling line is passed, find best location estimator	*/
				/*	by interpolation between those on either side			*/

				if(cur == 0.0)
					temp = 1.0;
				else if(prev != cur)
					temp = prev/(prev-cur);
				else
					temp = 0.5;

				xstart[setnum][numrul] = xint_a[i+1]+(xint_a[i]-xint_a[i+1])*temp;
				ystart[setnum][numrul] = yint_a[i+1]+(yint_a[i]-yint_a[i+1])*temp;
				zstart[setnum][numrul] = zint_a[i+1]+(zint_a[i]-zint_a[i+1])*temp;

				xend[setnum][numrul] = xint_b[j];
				yend[setnum][numrul] = yint_b[j];
				zend[setnum][numrul] = zint_b[j];

				/*	Save index of first line from transom outer end			*/

				if(itransom < 0 && j == num1b-1) itransom = numrul;

				if(cur != 0.0 || numrul > 0 &&
					    (xend[setnum][numrul] != xend[setnum][numrul-1] ||
					    yend[setnum][numrul] != yend[setnum][numrul-1] ||
					    zend[setnum][numrul] != zend[setnum][numrul-1]) ) numrul++;

				/*	If at the transom inner end, use only one intersection - will	*/
				/*	be a "dummy", anyway						*/

				if(j == numinta-1) break;
			}
			prev = cur;
		}
	}
	xstart[setnum][numrul] = xint_a[0];	/* last must be at stem */
	ystart[setnum][numrul] = yint_a[0];
	zstart[setnum][numrul] = zint_a[0];

	xend  [setnum][numrul] = xint_b[0];
	yend  [setnum][numrul] = yint_b[0];
	zend  [setnum][numrul] = zint_b[0];

breakout:
	numrul++;
	arrowcursor();

	rulings[la] = numrul;

#ifdef PROF
	transom = old_transom;		/* restore transom status	*/
#endif

	update_func = null;
	arrowcursor();
	if(MessageBox(hWnd,"Do you want to view the result?","Calculation completed",MB_YESNO) == IDYES) {
		scaled = FALSE;
		view_devel();
	}

}

MENUFUNC do_plate()
{
	int line = plate_lin + 1;
	extern char *ignline;
	extern int *ignore;
	extern int numbetw;
	int nsave = numbetw;

	if(getdlg(PLATEDLG,
	INP_INT,(void *) &line,
	INP_INT,(void *) &tran_lines,
	INP_STR,(void *) ignline,
	INP_INT,(void *) &numbetw,
	-1) && multproc(ignline,ignore,maxsec))
    {
		line--;
		if(line <= 0 || line >= numlin) {
			message("Invalid line index");
			line = 1;
		}
		else {
			plate_lin = line;
			calc_devel();
			numbetw = nsave;
		}
	}
}

/*	View and pre-edit development rulings		*/

void view_dev(int);

MENUFUNC view_devel()
{
	extern int rotatable;

	update_func = view_devel;
	print_func = view_devel;
	cls(TRUE);
	view_dev(TRUE);	// can't SwapBuf because line overdrawing sometimes follows
	SwapBuf();
	rotatable = TRUE;
}

void show_undo_screen(void);

void view_dev(int choose)
{
	extern int	dlgboxpos;
	extern int	repaint;
	REAL	save_heel;
	REAL	temp;
	int		last;
	extern int	persp;
	int		i,j,k,l,m;
	static int	sel = 0;
	extern int	scrollable;
	REAL	xspare,yspare;
#define	radian	0.01745329

#ifdef linux
	Dimension	width,height;
	extern Widget mainWindow;
#else
	RECT rc;
#endif

	save_heel = heelv;
	heelv = 180.0;

restart:

	if(undoruling) {
		if(scrdev == 0) {
			GLin();
#ifdef linux
			XtVaGetValues(mainWindow,XmNwidth,&width,XmNheight,&height,NULL);
			glOrtho((GLdouble) 0.0,(GLdouble) width,(GLdouble) 0.0,(GLdouble) height,(GLdouble) -10000.0,(GLdouble) 10000.0);
#else
			GetWindowRect(hWnd,&rc);
			glOrtho((GLdouble) rc.left,(GLdouble) rc.right,(GLdouble) rc.bottom,(GLdouble) rc.top,(GLdouble) -10000.0,(GLdouble) 10000.0);
#endif
		}
		pstrxy(0,0,"\n\nSelect rulings using cursor keys or mouse, Escape key to end.");
	}

	if(! scaled) {

		/*	Find range of image across field of view.			*/

		/*	"scale_view" works out potential range of image at		*/
		/*	azimuth "rotn", viewing position (yview,zview), for yview	*/
		/*	"up", zview "out of screen", it uses, modifies i and j - the	*/
		/*	first and last section indices					*/

		/*	the subroutine sets static parameters x1,x2,yy1,y2, which	*/
		/*	define the size of the field of view				*/

		scale_view(rotn,pp,yview,zview,&i,&j);
		xspare = 0.1*(x2-x1);
		yspare = 0.1*(y2-yy1);
		setranges(x1-xspare,x2+xspare,yy1-yspare,y2+yspare);
		if(scrdev == 0) set_GL_perspective();
	}

	graphics = 1;
	update_func = view_devel;
	xor_off();

	for(j = 1 ; j <= extlin+2 ; j++) {
		if((k = developed[j]) >= 0) {

			kdev = k;

//	Draw bordering hull lines without transom effect

			(*colour)(3);
#ifdef PROF
			transom = 0;
#endif
			draw_anyline(j  ,xrate_a,yrate_a,zrate_a,&l,&l,&l,3,-1.0);
			draw_anyline(j-1,xrate_a,yrate_a,zrate_a,&l,&l,&l,3,-1.0);
#ifdef PROF
			transom = atransom != 0.0;
#endif
			last = -99;

			/*	plot ruled lines between chine a and chine b		*/

			for(i = 0 ; i < rulings[j] ; i++) {

				/*	eliminate multiple rulings from any one point		*/

				if(choose && i > 0 && feq(xend[k][i],xend[k][i-1]) &&
					    feq(yend[k][i],yend[k][i-1]) &&
					    feq(zend[k][i],zend[k][i-1])) {

					if(last != i-1)
						draw_ruling(k,i-1,2);	/* draw first */
					draw_ruling(k,i,2);			/* draw second */
					highlight_ruling(k,i-1);	/* highlight first */
					highlight_ruling(k,i); 		/* highlight second */
					show12(k,i,1);

					dlgboxpos = 2;				/* bottom left corner */
#ifdef linux
					XFlush(display);
#else
					update_func = null;			// don't rewrite the view under the dialog box (NULL would clear the screen)
#endif
					if(scrdev == 0) {
						glEnd();
						SwapBuf();
					}
					l = getdlg(DEVEUNDO,INP_RBN,(void *) &sel,-1);

					if(scrdev == 1) {
						highlight_ruling(k,i);		/* unhighlight both rulings */
						highlight_ruling(k,i-1);
						show12(k,i,0);
					}

					if(l) {
						persp = 1;
						l = i - 1;
						if(sel == 2) {					/* ... if neither */
							m = 2;						/* move data back 2 */
							if(scrdev == 1) {
								draw_ruling(k,i,2);		/* undraw both */
								draw_ruling(k,i-1,2);
							}
						}
						else {
							m = 1;						/* move data back 1 */
							if(scrdev == 1)
								draw_ruling(k,i-sel,2);	/* undraw the other*/
							if(sel == 0) l++;			/* retain first: copy onto second */
							last = i-1;
						}

						/*		Shift data arrays back				*/

						for(l += m ; l < rulings[j] ; l++) {
							xstart[k][l-m] = xstart[k][l];
							ystart[k][l-m] = ystart[k][l];
							zstart[k][l-m] = zstart[k][l];
							xend  [k][l-m] = xend  [k][l];
							yend  [k][l-m] = yend  [k][l];
							zend  [k][l-m] = zend  [k][l];
						}
						rulings[j] -= m;
						if(itransom > i) itransom -= m;
						/* keep track of transom end */

						/*	Note that the loop index is being changed here	*/

						i--;
					}
					else {	/* end "getdlg" conditional	*/
						goto breakplot;
					}
				}
				else {
					draw_ruling(k,i,2);
					last = i;
				}	/* end matching-ends conditional*/
			}		/* end loop through all rulings	*/
		}
	}

breakplot:
	heelv = save_heel;
}

//	Show the developed surface view with text message on-screen

void show_undo_screen(void)
{
	scaled = FALSE;
	cls(FALSE);			// mouse positioning is more robust without the scroll bars
	view_dev(FALSE);
	SwapBuf();
}

/*	Undo a user-specified ruling			*/

#ifdef linux

#define VK_SHIFT XK_Shift_L
#define VK_F1 XK_F1
#define VK_F2 XK_F2
#define VK_TAB XK_Tab
#define VK_UP XK_Up
#define VK_DOWN XK_Down
#define VK_LEFT XK_Left
#define VK_RIGHT XK_Right
#define VK_ESCAPE XK_Escape
#define VK_RETURN XK_Return

extern Window win;
extern Display *display;

#endif

extern int undoruling;
extern int shiftdown;

void UndoKeyUp(int key)
{
	switch(key) {
	case VK_SHIFT:
		shiftdown = FALSE;
		break;
	}
}

void UndoKeyDown(int key)
{
	int l;
	extern int dlgboxpos;

	switch(key) {
	case VK_SHIFT:
		shiftdown = TRUE;
		break;

	case VK_DOWN:
down:
		if(ruling_index > 0) ruling_index--;
		goto showline;

	case VK_TAB:
		if(shiftdown) goto down;
	case VK_UP:
		if(ruling_index < rulings[plate_lin]-1) ruling_index++;
showline:
		if(scrdev == 1) {
			if(old_ruling_index >= 0) highlight_ruling(kdev,old_ruling_index);	/* undraw any old highlight */
			highlight_ruling(kdev,ruling_index);								/* redraw it as highlight */
		} else {
			cls(FALSE);
			scaled = FALSE;
			view_dev(FALSE);
			highlight_ruling(kdev,ruling_index);
			SwapBuf();
		}
		break;

	case VK_RETURN:

		/*	Request confirmation, and delete if confirmed		*/

		dlgboxpos = 2;
		l = getdlg(QUERYUNDO,-1,NULL);
		if(ruling_index >= 0) {
			if(scrdev == 1) {
				highlight_ruling(kdev,ruling_index); 	/* un-highlight line */
				if(l) draw_ruling(kdev,ruling_index,2);	/* remove line only if requested */
			}
			if(l) {
				for(l = ruling_index + 1 ; l < rulings[plate_lin] ; l++) {
					xstart[kdev][l-1] = xstart[kdev][l];
					ystart[kdev][l-1] = ystart[kdev][l];
					zstart[kdev][l-1] = zstart[kdev][l];
					xend  [kdev][l-1] = xend  [kdev][l];
					yend  [kdev][l-1] = yend  [kdev][l];
					zend  [kdev][l-1] = zend  [kdev][l];
				}
				rulings[plate_lin] --;
				if(itransom > ruling_index) itransom --;
			}
			if(scrdev == 0) show_undo_screen();
		}
		break;

	case VK_ESCAPE:
		undoruling = FALSE;
		update_func = NULL;
		cls(FALSE);
		SwapBuf();
		break;
	}
}


void UndoLeftButtonDown(int xm,int ym)
{
	int i,l,minoffset;
	int xa,ya,xb,yb;
	REAL dot,x,y,t,len;

	/*	left-hand key is tag key					*/

	minoffset = 32767;
	for(i = 0 ; i < rulings[plate_lin] ; i++) {

		/*	find upright screen coordinates of ends of ruling line		*/

		scrtrans(xstart[kdev][i],ystart[kdev][i],zstart[kdev][i],&xa,&ya);
		scrtrans(xend  [kdev][i],yend  [kdev][i],zend  [kdev][i],&xb,&yb);

		/*	express coordinate of end "b" of line, and mouse pointer,	*/
		/*	relative to end "a" of the line.				*/

		xb -= xa;		// coordinates of b relative to a
		yb -= ya;
		xa = xm - xa;	// coordinates of mouse relative to a
		ya = ym - ya;

		/*	vector cross product of lines to "b" and mouse gives twice	*/
		/*	the area of the triangle with base "a-b" and vertex the mouse	*/
		/*	pointer.							*/

		l = ya * xb - xa * yb;
		if(l < 0) l = -l;	/* positive area	*/

		/*	vector dot product, to give distance along line "a-b" of mouse.	*/
		/*	Length can be negative or greater than length of a-b			*/

		dot = xa * xb + ya * yb;

		/*	find length of line "a-b"					*/

		x = (float) xb;
		y = (float) yb;
		len = sqrt(x * x + y * y);

		/*	divide cross product by area by length to give distance from	*/
		/*	line "a-b" of mouse						*/

		/*	divide dot product by length to give position along line "a-b"	*/
		/*	of mouse							*/

		if(len > 0) {
			l /= len;
			dot /= len;
		}
		else {
			x = (float) xa;
			y = (float) ya;
			l = sqrt(x * x + y * y);
			dot = 0.0;
		}

		/*	mouse must be adjacent to line for minimum to be relevant	*/

		if(l < minoffset && dot <= len && dot >= 0.0) {
			minoffset = l;
			ruling_index = i;
		}
	}
	if(scrdev == 0) {
		cls(FALSE);
		scaled = FALSE;
		view_dev(FALSE);
		highlight_ruling(kdev,ruling_index);	/* redraw it as highlight */
		SwapBuf();
	} else {
		highlight_ruling(kdev,ruling_index);	/* redraw it as highlight */
	}
#ifndef linux
	UndoKeyDown(VK_RETURN);
#endif
}

MENUFUNC undo_ruling()
{
#ifdef linux
	extern XtAppContext app_context;
	XEvent event;
	KeySym keysym;
	char chars[12];
#endif
	REAL save_heel = heelv;
	heelv = 180.0;

	undoruling = 1;
	scaled = TRUE;
	ruling_index = -1;
	show_undo_screen();

#ifdef linux

//	Under Linux, keybaord and mouse button handle messages internally

	while(undoruling) {
		XtAppNextEvent(app_context,&event);
		if(event.type == KeyPress) {
			XLookupString(&(event.xkey),chars,sizeof(chars),&keysym,NULL);
			UndoKeyDown((int) keysym);
		}
		else if(event.xkey.window == win && event.type == ButtonPress) {
			if(event.xbutton.button == Button1) {
				UndoLeftButtonDown(event.xbutton.x,event.xbutton.y);
			}
			else {
				undoruling = 0;
			}
		}
		else {
			XtDispatchEvent(&event);
		}
		XFlush(display);
	}
#endif
}

#ifndef linux

LRESULT	procundo(HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	PAINTSTRUCT ps;
	extern int undoruling;
	void GetSizes(HWND);

	old_ruling_index = ruling_index;
	switch(msg) {

	case WM_PAINT:
		BeginPaint(hWnd,&ps);
		show_undo_screen();
		EndPaint(hWnd,&ps);
		break;

	case WM_KEYDOWN:
		UndoKeyDown((int) wParam);
		break;

	case WM_KEYUP:
		UndoKeyUp((int) wParam);
		break;

	case WM_LBUTTONDOWN:
		UndoLeftButtonDown(LOWORD(lParam),HIWORD(lParam));
		break;

	case WM_COMMAND:
		PostMessage(hWnd,msg,wParam,lParam);	// re-queue message from menu system

	case WM_RBUTTONDOWN:
	case WM_CLOSE:
	case WM_DESTROY:
		undoruling = 0;
		cls(FALSE);
		SwapBuf();
		break;

	case WM_SIZE:
		PostMessage(hWnd,WM_PAINT,0,0L);
		GetSizes(hWnd);
		break;

	default:
		return DefWindowProc(hWnd,msg,wParam,lParam);

	}	/* end switch */
	return 0l;
}

#endif

MENUFUNC write_devel()
{
#ifdef DEMO
	not_avail();
#else
	int	is,i,j,k;
	REAL	x,y;
	REAL	t1;
	FILE	*fp;

#ifdef SHAREWARE
	nagbox();
#endif
	if(!open_text(&fp,filedirnam,"*.txt")) return;

	for(is = 0 ; is < count ; is++) {
		fprintf(fp,"SECTION %d\n    ",is);
		for(i = 0 ; i < extlin+3 ; i++) {
			if(i > 0 && ((k = developed[i]) >= 0)) {
				for(j = 0 ; j < rulings[i] ; j++) {
					x = xstart[k][j] - xsect[is];
					y = xend  [k][j] - xsect[is];
					if(fneg(x) == fpoz(y)) {
						t1 = y / (y - x);
						x = yend[k][j]+t1*(ystart[k][j]-yend[k][j]);
						y = zend[k][j]+t1*(zstart[k][j]-zend[k][j]);
						if(!wrunit(fp,i+1,-1,x,y)) break;
					}
				}
			}
			if(!wrunit(fp,i+1,-1,yline[i][is],zline[i][is])) break;
		}
	}
	fclose(fp);
#endif
}

/*	write out 3-d endpoints				*/

MENUFUNC write_endpoints()
{
#ifdef DEMO
	not_avail();
#else
	int	 numrul;
	int	 i,j,k;
	FILE *fp;

#ifdef SHAREWARE
	nagbox();
#endif
	if(open_text(&fp,filedirnam,"*.txt")) {
		for(j = 1 ; j < extlin+3 ; j++) {
			if((k = developed[j]) >= 0) {
				numrul = rulings[j];
				for(i = 0 ; i < numrul ; i++) {
					if(fprintf(fp,"%2d %3d %8.4f %8.4f %8.4f : %8.4f %8.4f %8.4f\n",
						j+1,i+1,xstart[k][i],ystart[k][i],zstart[k][i],
						xend[k][i],  yend[k][i],  zend[k][i]) < 0) {
						message("CAN NOT WRITE TO TEXT DEVICE");
						break;
					}
				}
			}
		}
		fclose(fp);
	}
#endif
}

/*	Highlight (or unhighlight) an existing drawn line, presuming GDI has been initialised already */

void highlight_ruling(int k,int i)
{
	if(scrdev == 0) {
		(*colour)(1);
	} else {
		xor_on();
		xorcol(3);
	}
	(*newlin)();
	pltsel(xstart[k][i],-ystart[k][i],zstart[k][i],3);
	pltsel(xend  [k][i],-yend  [k][i],zend  [k][i],3);
	xor_off();	// ignored under OpenGL
}

/*	Draw a ruling, presuming graphics have been initialised already	*/

MENUFUNC draw_ruling(int k,int i,int col)
{
	xor_on();
	xorcol(col);
	(*newlin)();
	pltsel(xstart[k][i],-ystart[k][i],zstart[k][i],3);
	pltsel(xend  [k][i],-yend  [k][i],zend  [k][i],3);
	xor_off();
}

MENUFUNC fix_transom_curve()
{
	REAL	norm_x,norm_y,norm_z;
	REAL	a,hb,c,hd;
	REAL	dy0,dy1;
	int	setnum = developed[plate_lin];
	extern int	changed;

	/*	do nothing if surface not yet developed				*/
	/*	 ... or if not developable at stern				*/

	if(setnum < 0) {
		message("No rulings defined yet.\nUse the Evaluate menu item");
		return;
	}
	else if(itransom < 0) {
		message("No ruling from aft end of upper line ...\nnothing to match curve to.");
		return;
	}

	/*	find normal vector at transom end of the higher-indexed (lower) line	*/

	getnorm(&norm_x,&norm_y,&norm_z,
		xrate_b[num1a-1],yrate_b[num1a-1],zrate_b[num1a-1],
		xend  [setnum][itransom],yend  [setnum][itransom],zend  [setnum][itransom],
		xstart[setnum][itransom],ystart[setnum][itransom],zstart[setnum][itransom]);

	if(norm_y != 0.0) {	/* cannot do adjustment if normal is	*/
		/* vertical at end of line		*/
		getparam(ensec[plate_lin],plate_lin,&a,&hb,&c,&hd);
		dy0 = (0.5*a + hb);
		/* old position in of control point */
		dy1 = (0.5*c + hd) * norm_z / norm_y;
		/* new position in of control point */

		/*	move control point in by the difference			*/

		ycont[plate_lin][ensec[plate_lin]] -= (dy1 - dy0);
//		changed = 1;
	}
	else {
		message("Can not fix transom curve.\nTry relocating control point on stern section");
	}
#ifdef PROF
	recalc_transom();
#endif
}

void scrtrans(REAL xs,REAL ys,REAL zs,int *xa,int *ya)
{
	extern int xmaxi,ymaxi;
	extern REAL rotn,heelv;
	heelv = fadd(heel,180.0);	/* may not yet be defined */

	perspp(0.0,0.0,fmin(xs),0.0,rotn,heelv);
	ys = fmin(ys);
	trans(&ys,&zs);
	*xa = (int) ys;
	*ya = ymaxi - ((int) zs);
}

void show12(int k,int i,int ncol)
{
	extern int xscr,yscr;
	extern HDC hDC;
	extern COLORREF scrcolour[];
	COLORREF save_col = scrcolour[1];

#ifndef linux
	if(scrdev == 1) {
		SetBkMode(hDC,TRANSPARENT);
		SetTextColor(hDC,scrcolour[ncol]);
		scrcolour[1] = scrcolour[ncol];
	} else {
		(*colour)(ncol);
	}

	(*newlin)();
	pltsel(xstart[k][i-1],-ystart[k][i-1],zstart[k][i-1],3);
	(*plstr)("1st");

	(*newlin)();
	pltsel(xstart[k][i],-ystart[k][i],zstart[k][i],3);
	(*plstr)("2nd");

	scrcolour[1] = save_col;
#endif
}

/*	Show rolled-out transom surface		*/

#ifdef PROF

MENUFUNC rollout_transom()
{
	REAL xmin,xmax,ymin,ymax;
	REAL *x,*y;
	REAL a,b,c,d,hb,hd,aa,cc;
	extern int numtran;
	extern int xleft,xright,ybottom,ytop;
	extern REAL PixelRatio;
	REAL ar = (REAL) (xright - xleft) / (REAL) (ybottom - ytop) / PixelRatio;
	int nt,i,j,i0,i1;
	REAL xe = xsect[count-1];
	REAL ye,ze,dummy;
	REAL r2 = rtransom * rtransom;
	extern int persp;

	if(!memavail((void far *) &x,2*numtran*sizeof(REAL))) {
		message("No memory for transom development");
		return;
	}
	else {
		y = x + numtran;
		xmin = 0.0;
		ymin = 1.0e+30;
		xmax = -1.0e+30;
		ymax = xmax;
		nt = 0;

		/*	Calculate outline positions on transom cylinder		*/

		for(i = 0 ; i < numtran && xtran[i] <= xe ; i++) {
			if(fnoz(rtransom)) {
				a = fdiv(ytran[i],rtransom);
				if(a >= -1.0 && a <= 1.0) {
					a = asin(a);
				}
				else {
					a = 1.5707963;
				}
				x[nt] = fmul(rtransom,a);
			}
			else {
				x[nt] = ytran[i];
			}
			y[nt] = fadd(fmul(stransom,xtran[i]),fmul(ctransom,ztran[i]));
			if(flt(x[nt],xmin)) xmin = x[nt];
			if(fgt(x[nt],xmax)) xmax = x[nt];
			if(flt(y[nt],ymin)) ymin = y[nt];
			if(fgt(y[nt],ymax)) ymax = y[nt];
			nt++;
		}
		ye = ytran[numtran-1];	/* defaults should no intersection be found */
		ze = ztran[numtran-1];

		/*	Find intersection of transom curve with last section outline -
		(xe,ye,ze)
		*/
		if(nt < numtran) {	/* ... we have upward curve on transom */
			aa = 0.0;
			cc = 1.0;
			for(j = 1 ; j < numlin ; j++) {
				if(ensec[j] == count-1) {
					hullpa(count-1,j,aa,cc,&a,&hb,&c,&hd);
					if(curve_inters(xe,0.0,
						yline[j][count-1],a,hb,0.0,
						zline[j][count-1],fmin(c),fmin(hd),0.0,
						&i0,&i1,&dummy,&d,&ye,&ze)) break;
					tranpa(a,hb,c,hd,&aa,&cc);
				}
			}
		}

		/*	Plot the calculated outline	*/

		/*	Calculate and show the plot scale	*/

		(*init)();
		cls(FALSE);
		a = 0.7*(xmax-xmin);
		b = 0.7*(ymax-ymin);
		if(b * ar < a) {
			b = a / ar;
		}
		else {
			a = b * ar;
		}
		c = 0.5*(xmax+xmin);
		d = 0.5*(ymax+ymin);
		persp = 0;
		setranges(0.0,2.0*a,0.0,1.0);
		show_scale(0.0,2.0*a,0.0,1.0);
		setranges(c - a,c + a,d + b,d - b);

		/*	Draw the transom end outline		*/

		(*colour)(1);
		(*newlin)();
		for(i = 0 ; i < nt ; i++) (*draw)(x[i],y[i]);

		/*	Plot the curve of the intersection of transom with the last section */

		b = ye * 0.05;
		d = -0.5*b;
		for( ; fgt(ye,d) ; ye = fsub(ye,b)) {
			if(rtransom > 0.0)
				a = fdiv(ye,rtransom);
			else
			    a = 0.0;
			if(a >= -1.0 && a <= 1.0) {
				a = asin(a);
			}
			else {
				a = 1.5707963;
			}
			if(stransom > 0.0) {
				ze = ztransom + (rtransom-fsqr0(max(0.0,r2-ye*ye)))/stransom;
				(*draw)(fmul(rtransom,a),fadd(fmul(stransom,xe),fmul(ctransom,ze)));
			}
		}

		/*	Plot the curve of the intersection of the transom with the deck	*/

		if(ctransom > 0.0) {
			a = xe - (ztransom-ztran[0])*stransom/ctransom;	/* centre of ellipse */
			a = fadd(fmul(ctransom,a),fmul(stransom,ztran[0]));
			b = x[0] * 0.05;
			d = rtransom*stransom/ctransom;
			xe = x[0]*1.02;
			if(fnoz(rtransom)) {
				for(c = 0.0 ; fle(c,xe) ; c = fadd(c,b))
					(*draw)(c,a-d*(1.0-cos(c/rtransom)));
			}
			else {
				(*draw)(0.0,y[0]);
				(*draw)(x[0],y[0]);
			}
		}
		memfree(x);
		SwapBuf();
	}
	update_func = rollout_transom;
	print_func = rollout_transom;
}

#endif

void draw_anyline(int la,REAL *xrate,REAL *yrate,REAL *zrate,int *numint,
	int *stem_ind,int *tran_ind,int mode,REAL side)
{
#ifdef PROF
	if(la > extlin)
		plot_devstr(la,mode,xrate,yrate,zrate,numint,stem_ind,tran_ind,side);
	else
#endif
	    draw_line(la,side,side,mode,0,count-1,999,xrate,yrate,zrate,numint,stem_ind,tran_ind,0);
}

#ifdef PROF

void plot_devstr(int k,int mode,REAL *xrate,REAL *yrate,REAL *zrate,int *num,
	int *stind,int *trind,REAL side)
{
	int i,is,ie,j;
	REAL stx,enx;
	extern int strlin,strind;

	yline[extlin][maxsec] = 2.0;
	zline[extlin][maxsec] = 2.0;
	yline[extlin][maxsec+1] = 1.0;
	zline[extlin][maxsec+1] = 1.0;
	radstem[extlin] = 0.0;

	j = inistr[strlin] + strind;
	is = ststr[j];
	ie = enstr[j];
	stx = xsect[is];
	enx = xsect[ie];
	xsect[is] = ststrx[j];
	xsect[ie] = enstrx[j];
	for(i = is ; i <= ie ; i++) {
		yline[extlin][i] = yline[k][i];
		zline[extlin][i] = zline[k][i];
	}
	stsec[extlin] = is;
	ensec[extlin] = ie;
	draw_line(extlin,side,side,mode,is,ie,999,xrate,yrate,zrate,num,stind,trind,0);
	xsect[is] = stx;
	xsect[ie] = enx;
}

#endif

int alloc_dev(void)
{
	int i = sizeof(*xrate_a) * MAXRUL;	/* sizeof() should be 4, here */
	int result;

	result = altavail((void far *) &xrate_a,i) &&
	    altavail((void far *) &yrate_a,i) &&
	    altavail((void far *) &zrate_a,i) &&
	    altavail((void far *) &xrate_b,i) &&
	    altavail((void far *) &yrate_b,i) &&
	    altavail((void far *) &zrate_b,i) &&
	    altavail((void far *) &xint_a,i) &&
	    altavail((void far *) &yint_a,i) &&
	    altavail((void far *) &zint_a,i) &&
	    altavail((void far *) &xint_b,i) &&
	    altavail((void far *) &yint_b,i) &&
	    altavail((void far *) &zint_b,i);

	/*	allocate extra memory for x,y,z start and x,y,z end arrays */

	if(developed[la] < 0) {
		i = sizeof(*xstart) * (numruled+1);
		result &= altavail((void far *) &xstart,i) &&
		    altavail((void far *) &ystart,i) &&
		    altavail((void far *) &zstart,i) &&
		    altavail((void far *) &xend  ,i) &&
		    altavail((void far *) &yend  ,i) &&
		    altavail((void far *) &zend  ,i);

		if(result) {
			setnum = numruled;		/* next set here		*/
			developed[la] = setnum;	/* record where stored		*/
			numruled++;			/* record extra ruling set	*/
		}
	}
	else {
		setnum = developed[la];
	}
	return result;
}

void free_dev(void)
{
	memfree(xrate_a);
	memfree(yrate_a);
	memfree(zrate_a);
	memfree(xrate_b);
	memfree(yrate_b);
	memfree(zrate_b);
	memfree(xint_a);
	memfree(yint_a);
	memfree(zint_a);
	memfree(xint_b);
	memfree(yint_b);
	memfree(zint_b);
	memfree(xstart);
	memfree(ystart);
	memfree(zstart);
	memfree(xend);
	memfree(yend);
	memfree(zend);
}

/*	Calculate "false" development lines for a stringer	*/

void false_devel()
{
	int i,k;

	if(count <= 0) return;

	la = plate_lin;
	lb = plate_lin-1;		/* line indices */

	if(!alloc_dev()) return;

	indx = 0;
	draw_anyline(la,xrate_a,yrate_a,zrate_a,&numinta,&k,&k,8,1.0);
	indx = 0;
	draw_anyline(lb,xrate_b,yrate_b,zrate_b,&k,&k,&k,9,1.0);

	rulings[la] = numinta;

	i = numinta;
	for(k = 0 ; k < numinta ; k++) {
		i--;
		xstart[setnum][k] = xint_a[i];
		ystart[setnum][k] = yint_a[i];
		zstart[setnum][k] = zint_a[i];
		xend[setnum][k] = xint_b[i];
		yend[setnum][k] = yint_b[i];
		zend[setnum][k] = zint_b[i];
	}
}

#endif

extern int gl_xor;

void xor_on()
{
#ifdef linux
	extern Display *display;
	extern GC gc;

	XSetFunction(display,gc,GXxor);
#else
	extern HDC hDC;

	if(scrdev == 1) SetROP2(hDC,R2_XORPEN);

#endif
}

void xor_off()
{
#ifdef linux
	extern Display *display;
	extern GC gc;

	XSetFunction(display,gc,GXcopy);
#else
	extern HDC hDC;

	if(scrdev == 1) SetROP2(hDC,R2_COPYPEN);

#endif
}


