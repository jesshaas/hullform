/* Hullform component - hlsm_hf8.c
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
 
/************************************************************************/

/*	SMOOTH HULL LINES, HULLFORM 8+					*/

/*	SMOOTH ANY OF THE PARAMETERS USED TO FORM SECTIONS		*/

/************************************************************************/

#include "hulldesi.h"
#define	degree	57.2958

#define PROCESSED TRUE
#define UNPROCESSED FALSE

#ifdef linux

/*	X-windows include files		*/

#include <Xm/Xm.h>
#include <Xm/List.h>
#include <Xm/Form.h>
#include <Xm/Text.h>
#include <Xm/MainW.h>
#include <Xm/BulletinB.h>
#include <Xm/ScrolledW.h>
#include <Xm/CascadeB.h>
#include <Xm/RowColumn.h>
#include <Xm/PushB.h>
#include <Xm/Label.h>
#include <Xm/Frame.h>
#include <Xm/MessageB.h>
#include <Xm/ScrollBar.h>
#include <Xm/FileSB.h>
#include <Xm/DialogS.h>
#include <Xm/Separator.h>
#include <Xm/DrawingA.h>
#include <Xm/DrawnB.h>
#include <Xm/DragC.h>
#include <X11/keysym.h>
#include <X11/Xlib.h>

#define VK_SHIFT XK_Shift_L
#define VK_F1 XK_F1
#define VK_F2 XK_F2
#define VK_TAB XK_Tab
#define VK_UP XK_Up
#define VK_DOWN XK_Down
#define VK_LEFT XK_Left
#define VK_RIGHT XK_Right
#define VK_ESCAPE XK_Escape

#undef tc

Widget ButtonWidget(Widget widget,char *text,int x,int y,int w,int h);
Widget LabelWidget(Widget widget,char *text,int centre,int x,int y,int w,int h);
void HorzScroll(Widget w, XtPointer client_data, XtPointer call_data);
void VertScroll(Widget w, XtPointer client_data, XtPointer call_data);
void EditHorzScroll(Widget w, XtPointer client_data, XtPointer call_data);
void SmooVertScroll(Widget w, XtPointer client_data, XtPointer call_data);
void SmooMenuCallback(Widget w, XtPointer client_data, XtPointer call_data);
extern Display *display;
extern GC gc;
extern Widget WorkArea;
extern Window win;
extern Pixel DialogBackground,EditBackground,ButtonBackground,TopShadow,BottomShadow;
extern XmFontList windowfontlist;
int SmooQuit;
Widget SmooMenu = NULL;

extern Display *display;
extern GC gc;

#else

typedef LRESULT CALLBACK WPROC();
BOOL CALLBACK SmooDlgProc(HWND hWndDlg,unsigned msg,WORD wParam,LONG lParam);
LRESULT CALLBACK SmoothProcedure(HANDLE hWnd,UINT msg,WPARAM wParam,LPARAM lParam);
int ColourMessage(unsigned msg,WORD wParam,LONG lParam,BOOL *colret);
INT WindowProcedure(HWND hWndProc,UINT msg,WPARAM wParam,LPARAM lParam);

#endif

int SmooDlg(int *seldir,int *linesel);
void window(HWND hParent,char *title,int xorigin,int yorigin,int xsize,int ysize,
	char *winclass,DWORD style,HWND *hWndRet);
void movepoint(int follow,int ix,int linnum,REAL linesel,
	int funnum,REAL curmin,REAL curmax,REAL offbot,REAL offtop);

void GetSize(HWND hWnd);
void check_negative_offsets(void);

extern HDC hDC;
char comchar[] = {
	'S','P','F','R','V','C','I','A','U','M','L','H','D','O','Q',0};
int mousedrag = 0;
REAL run1,run2;
void showvalues(REAL x,REAL y);
void listseq(int *master,char *masterstring,int n);
void get_weights(int);
void listin(char *line,REAL table[]);
void centre_dlg(HWND hWndDlg);
void get_int_angle(int j,REAL *cstem,REAL *sstem);
void redraw_smooth(REAL *curmax,REAL *curmin,REAL *offbot,REAL *offtop,int *line_also,int smooth_scaled);
void GetSizes(HWND);
void linesm(REAL offset[]);
extern char *helpfile;
extern int context_id;
extern int HelpUsed;
void line_properties(int);
void xorcol(int);
void fix_vertical_controls(int i,int j,REAL value);
#ifndef STUDENT
REAL	cstem,sstem;
#endif

#ifdef PROF
extern int zoom;
#endif

extern void null();
void pstrbkg(char *s,int col,int bkg,int x,int y);

extern	char	*alsolines;
extern	char	*masterstring;
extern	char	*inpsmoo;
extern	char	*relsmoo;
extern	int 	*ignsmoo;
extern	int		*relax;
extern	REAL	offsetb[];
extern	int		*line_also;
extern	REAL	*linesave;
extern	int 	dlgresult;
static	int		ch1,ch2;
extern	REAL	xgorigin,xgslope,ygorigin,ygslope,xmin,ymin;
extern	int		xmaxi,ymaxi;

void smoo_diff(void);

extern	int changed;
extern	COLORREF scrcolour[];
extern	int ybottom,xchar,ychar;
REAL	*xs;	/* section positions for the current line of points */

int	xswleft = 0;
int yswtop = 0;

#define	INICOL	1
#define	ALTCOL	3

char	*smoo_opt[]={
	"lateral offset",			/* 0 */
	"vertical offset",			/* 1 */
	"lateral control offset",		/* 2 */
	"vertical control offset",		/* 3 */
	"angle to previous line",		/* 4 */
	"angle to next line",			/* 5 */
	"angle to previous control point",	/* 6 */
	"angle to next control point",		/* 7 */
	"distance to previous line",		/* 8 */
	"distance to next line",		/* 9 */
	"distance to previous control point",	/* 10 */
	"distance to next control point"	/* 11 */
};
FUNC_PTR smoo_func[] = {
	smoo_off ,smoo_off ,smoo_off ,smoo_off,
	smoo_diff,smoo_diff,smoo_diff,smoo_diff,
	smoo_diff,smoo_diff,smoo_diff,smoo_diff};

char	useprev[] = {
	0,0,1,1, 1,0,1,0, 1,0,1,0};
char	usenext[] = {
	0,0,0,0, 0,1,0,1, 0,1,0,1};

#ifdef linux

char *SmooMenuItem[] = {
	"Smooth","Pull","Follow","Redraw","Value","Curve","Ignore",
	"Also","Undo","Master","fLexibility","Half","Double",
	"prOperties","Quit",""};
extern int force_proceed;

#endif

void SmooLineLeftButtonDown(int x);
void SmooLineMouseMove(int y);
int		linnum = 0;	   	/* number of hull line being smoothed	*/
int		funnum = 0;		/* function number from table */
int		lin;
extern	int	follow;
int		st,en;			/* start and end sections for current line */
int		linesel = 1;
int		seldir = 0;

REAL	*wb;

int		pull;
int		smoosect;
REAL	delta0;
REAL	ysave;
int		maxlen;
int		num = 0;
int		SmooQuit;
REAL	curmin,curmax,offbot,offtop;
extern	int shiftdown;

int	line;
void smoofunc(int funnum);
void smoocode(int fn,HWND hWndDlg);
extern int repaint;
extern int dlgboxpos;
extern int scaled;
extern int numbetw;

#ifdef linux

#include <GL/gl.h>
#include <GL/glu.h>
extern Widget glw;

#else

#include <GL\gl.h>
#include <GL\glu.h>

#endif

void SwapBuf(void)
{
	int n;
	if(scrdev == 0) {
#ifdef linux
		glEnd();
		glXSwapBuffers(XtDisplay(glw),XtWindow(glw));
        XFlush(display);
        XSetInputFocus(display,win,RevertToNone,CurrentTime);
#else
		if(hDC != NULL) {
			glEnd();
			SwapBuffers(hDC);
		}
#endif

	} else {
#ifdef linux
        (*endgrf)();
#else
		if(scrdev != 1) (*endgrf)();
#endif
    }
}

/*	Fair the altered line automatically if required, then plot it	*/

void DrawSmooth(void)
{
	int ns = 0;
	int j;
	REAL xtemp[maxsec],offtem[maxsec],wt[maxsec];

	for(j = st ; j <= en ; j++) {
		if(v7mode || master[j]) {
			xtemp[ns] = xs[j];
			wt[ns] = wb[j];
			offtem[ns++] = offsetb[j];
		}
	}
	if(ns >= 2) {
		if(autofair[linnum]) {
			spline(xtemp,offtem,wt,ns,&xs[st],&offsetb[st],en-st+1,run1,run2);
		}
		if(scrdev == 1) {
			plotline(curmin,curmax,offbot,offtop,offsetb,INICOL);			// GDI - just draw the line
		}
		else {
			redraw_smooth(&curmax,&curmin,&offbot,&offtop,line_also,TRUE);	// GL - draw the whole view
			SwapBuf();
		}
	}
}

void SmooLineKeyDown(int key)
{
	int		i,j;
	char	input[128];
	int		ns;
	REAL	xtemp[maxsec],offtem[maxsec+2],offfit[maxsec+2];
	REAL	wt[maxsec];
	REAL	x,frac;
	char	*s,*p;
	int		set[maxsec];
	REAL	value;
	int		was_rel;

	pull = TRUE;

	/*	Check for section number input before other options	*/

	key = toupper(key);
	i = key;
	if(i >= '0' && i <= '9' || i == 8) {	/* digit or backspace */
		if(i == 8) {
			num = 0;
		}
		num = num * 10 + (i - '0');
		if(num >= 0 && num < count)
			smoosect = num;
		else
		    num = 0;
		sprintf(input,"%s at %d                  ",follow ? "Follow" : "Pull",smoosect);
		if(scrdev == 0) redraw_smooth(&curmax,&curmin,&offbot,&offtop,line_also,TRUE);
		pstrbkg(input,1,0,0,0);
		SwapBuf();
	}
	else {
		num = 0;
		if(key == VK_ESCAPE) key = 'Q';		/* ESC translates to Quit */
#ifdef linux
		for(i = 0 ; *SmooMenuItem[i] != 0 ; i++) {
			if(strchr(SmooMenuItem[i],key) != NULL) {
				context_id = 2070 + i;
				break;
			}
		}
#else
		for(i = 0 ; comchar[i] != 0 ; i++) {
			if(comchar[i] == key) {
				context_id = 2070 + i;
				break;
			}
		}
#endif

		switch(key) {
		case 'S':	/* smooth */
			dlgboxpos = 1;
			if(multin("Enter sections to smooth:",relax,maxsec)) {

				/*	TAKE A WORKING COPY OF REQUIRED LINE, OMITTING SECTIONS TO BE	*/
				/*	RELAXED								*/

				ns = 0;
				for(j = st ; j <= en ; j++) {
					if(!ignsmoo[j] && (!relax[j] || !v7mode && master[j])) {
						xtemp[ns] = xs[j];
						if(j > 0) wt[ns-1] = wb[j-1];
						wt[ns] = wb[j];
						offtem[ns++] = offsetb[j];
					}
				}

				/*	INSERT FITTED VALUES	*/

				if(ns <= 1) {
					message("NEED AT LEAST TWO REMAINING POINTS");
					break;
				}
				else {
					spline(xtemp,offtem,wt,ns,&xs[st],&offfit[st],en-st+1,run1,run2);
					if(scrdev == 1) {
						plotline(curmin,curmax,offbot,offtop,offfit,ALTCOL);
					}
					else {
						redraw_smooth(&curmax,&curmin,&offbot,&offtop,line_also,TRUE);
						plotline(curmin,curmax,offbot,offtop,offfit,ALTCOL);
						SwapBuf();
					}

					/*	FRACTION TO INCLUDE MAY BE VARIED	*/

					frac = 1.0;
					dlgboxpos = 1;
					i = getdlg(INPFRAC,INP_REA,(void *) &frac,-1);
					if(!i) break;

					/*		MAY NOT ALTER STEM LATERAL OFFSET BY SMOOTHING	*/

					if(funnum == 0 && st == 0) {
						j = 1;
					}
					else {
						j = st;
					}
					for(i = j ; i <= en ; i++) {
						if(!ignsmoo[i]) {
							x = frac*offfit[i]+offsetb[i]*(1.0-frac);
							offsetb[i] = x;
						}
					}
				}	/* end sufficient-points condition */
			}	/* end valid-input condition */
			goto redraw;

		case 'P':	/* pull */
		case 'F':	/* follow */
			if(key == 'F' && linnum == 0 && funnum == 0) {
				message("The follow option is not available for line 1");
			}
			else {
				follow = key == 'F';
				delta0 = 0.0400;
				pull = 1;
				sprintf(input,"%s .. enter index    ",follow ? "Follow" : "Pull",smoosect);
				if(scrdev == 0) redraw_smooth(&curmax,&curmin,&offbot,&offtop,line_also,TRUE);
				pstrbkg(input,1,0,0,0);
				SwapBuf();
			}
			break;

		case 'R':	/* redraw */
redraw:
			redraw_smooth(&curmax,&curmin,&offbot,&offtop,line_also,FALSE);
			SwapBuf();
			break;

		case 'V':	/* value */
			*input = '\0';
			if(getdlg(INPVALUE,INP_STR,input,-1)) {
				s = input;
				while(isspace(*s)) s++;
				if(*s != 0) {
					p = s;
					while(!isspace(*p) && *p != 0) p++;
					*p++ = 0;
					if(multproc(s,set,maxsec) && sscanf(p,"%f",&value) > 0) {
						/* i.e., if valid and not stem lateral offset */
						if(funnum == 1 || funnum == 3) value *= invert;
						if(scrdev == 1)
							plotline(curmin,curmax,offbot,offtop,offsetb,INICOL);
						for(i = 0 ; i < count ; i++) {
							if(set[i]) movepoint(FALSE,i,linnum,
								value-offsetb[i],funnum,curmin,curmax,offbot,offtop);
						}
						goto redraw;
					}
				}
				message("Syntax error in input line");
			}
			break;

		case 'C':	/* curve */
			if(getdlg(CURVFACT,INP_REA,&run1,INP_REA,&run2,-1)) {
				offsetb[maxsec]   = run1;
				offsetb[maxsec+1] = run2;
			}
			break;

		case 'I':	/* ignore */
			(void) multin("Enter sections to ignore: ",ignsmoo,maxsec);
			break;

		case 'A':	/* also */

			(void) multin("Also plot which lines:",line_also,maxlin);
			goto redraw;

		case 'U':	/* undo */
			for(i = st ; i <= en ; i++) offsetb[i] = linesave[i];
			goto redraw;

		case 'M':

			/* define master sections */
			master[0] = TRUE;
			listseq(master,masterstring,count);
			if(getdlg(MASTER,
				INP_STR,(void *) masterstring,-1)) {
				(void) multproc(masterstring,master,maxsec);
			}
			master[0] = TRUE;
			goto redraw;

		case 'L':	/* Flexibility */
			get_weights(linnum);
			for(i = st ; i <= en ; i++) wb[i] = linewt[linnum][i];
			goto redraw;

		case 'H':	/* Half */
			if(delta0 > 0.0000001) delta0 *= 0.5;
			shopul(delta0);
			break;

		case 'D':	/* Double */
			if(delta0 < 100000.0) delta0 *= 2.0;
			shopul(delta0);
			break;

		case 'O':	/* line properties */
			was_rel = relcont[linnum];
			line_properties(linnum);
			if(was_rel != relcont[linnum] && funnum == 2) {
				for(i = st ; i <= en ; i++) offsetb[i] = ycont[linnum][i];
				goto redraw;
			}
			break;

		case 'Q':	/* quit */
			SmooQuit = 1;
			break;
		}
	}
}

#ifdef linux
void SmooMenuCallback(Widget w,XtPointer client_data,XtPointer call_data)
{
	SmooLineKeyDown(comchar[(int) client_data]);
}
#endif

void SmooLineLeftButtonDown(int xm)
{
	REAL x,least;
	int i,ym;
	REAL dist;
#ifdef linux
	extern Widget glw;
#endif
	GLint viewport[4];
	GLdouble modelview[16];
	GLdouble projection[16];
	GLfloat winY;
	GLdouble posX,posY,posZ;

	num = 0;

	/*	left-hand key is "drag" key: find nearest section, go to it */

	if(!mousedrag) {
		least = 1.0e+30;
		if(scrdev == 0) {
			glGetDoublev( GL_MODELVIEW_MATRIX, modelview );
			glGetDoublev( GL_PROJECTION_MATRIX, projection );
			glGetIntegerv( GL_VIEWPORT, viewport );
			gluUnProject((GLfloat) xm,(GLfloat) 0.0,(GLfloat) 1.0, modelview, projection, viewport, &posX, &posY, &posZ);
			x = posX;
		} else {
			x = xmin + ((REAL) xm + 0.5) / xgslope;
		}
		for(i = st ; i <= en ; i++) {
			dist = fabs(xs[i] - x);
			if(dist < least) {
				smoosect = i;
				least = dist;
			}
		}
		if(scrdev == 0) {
			gluProject((GLfloat) xs[smoosect],(GLfloat) offsetb[smoosect],(GLfloat) 1.0, modelview, projection, viewport,
					&posX, &posY, &posZ);
			xm = posX;
			ym = posY;
		} else {
			xm = (int) ((xs[smoosect] - xmin) * xgslope);
			ym = (int) ((ymax - offsetb[smoosect]) * ygslope);
		}

		/*	(xm,ym) are now window coordinates of point to which mouse is	*/
		/*	to be moved							*/

		if(v7mode || master[smoosect]) {
#ifdef linux
			if(scrdev == 0)
				XWarpPointer(display,None,XtWindow(glw),0,0,0,0,xm,ym);
			else
				XWarpPointer(display,None,win,0,0,0,0,xm,ym);
#else
			SetCursorPos(xm + xswleft,ym + yswtop);
#endif
			mousedrag = 1;
		}
	}
}

void SmooLineMouseMove(int ym)
{
	REAL delta;
	GLint viewport[4];
	GLdouble modelview[16];
	GLdouble projection[16];
	GLfloat winY;
	GLdouble posX,posY,posZ;

	if(mousedrag) {

#ifdef linux

#endif

//	GDI mode - "undraw" the old line
		if(scrdev == 1) plotline(curmin,curmax,offbot,offtop,offsetb,INICOL);
		/*	find physical coordinates corresponding to the new point	*/
#ifdef linux
		if(scrdev == 0) {
			glGetDoublev( GL_MODELVIEW_MATRIX, modelview );
			glGetDoublev( GL_PROJECTION_MATRIX, projection );
			glGetIntegerv( GL_VIEWPORT, viewport );
			winY = (float)viewport[3] - (float) (ym);
			gluUnProject((GLfloat) 0.0, winY,(GLfloat) 1.0, modelview, projection, viewport, &posX, &posY, &posZ);
			delta = posY - offsetb[smoosect];
		} else {
			delta = ((REAL) (ymaxi - ym) - ygorigin) / ygslope - offsetb[smoosect];
		}
#else
		delta = (((REAL) (ymaxi - (ym+yswtop) )) - ygorigin) / ygslope - offsetb[smoosect];
#endif
		movepoint(follow,smoosect,linnum,delta,funnum,curmin,curmax,offbot,offtop);
		DrawSmooth();
	}
}

/*	This is the routine called when the menu selection "Smooth lines" is made
*/

MENUFUNC smooth_line()
{
	extern int xright,xleft,ybottom,ytop;

	if(count < 2) return;
#ifdef linux
	force_proceed = 0;
#endif
	line = linnum + 1;
	funnum = -1;
	update_func = null;	/* prevent rewrite of temporarily modified hull line */

	if(!memavail((void *) &wb,maxsec*sizeof(REAL))) {
		message("Heap overflow in line smoothing routine");
		return;
	}

	cls(0);
	SwapBuf();
	while (getdlg(HULLSMOO,INP_INT,(void *) &line,INP_PBF,(void *) smoocode,-1)) {
		if(funnum >= 0) {
			smoofunc(funnum);
			cls(0);
			SwapBuf();
		}
	};

	memfree(wb);

	if(scrdev == 1) {
#ifdef linux
		XSetFunction(display,gc,GXcopy);
#else
		SetROP2(hDC,R2_COPYPEN);
#endif
	} else {
		SwapBuf();
	}
}

REAL ysave;
REAL ycsave;

void smoocode(int fn,HWND hWndDlg)
{
	extern int canexit;
#ifdef linux
	extern int getdlg_proceed,force_proceed;
	force_proceed = 1;
	getdlg_proceed = 1;
#else
	PostMessage(hWndDlg,WM_COMMAND,IDOK,0);
#endif
	funnum = fn;
	canexit = 1;
}

void smoofunc(int fn)
{
	int i;
	int notok;
	REAL a,hb,c,hd;
	extern int getdlg_proceed;

	funnum = fn;

	if(line < 1 || line > extlin) {
		message("Invalid line index");
		return;
	}
	if(!memavail((void *) &xs,maxsec*sizeof(REAL))) {
		message("No memory for smoothing operations");
		return;
	}

	notok = (line == 1 && useprev[funnum]) || (line == extlin && usenext[funnum]);

	linnum = line - 1;
	ysave  = yline[linnum][0];
	ycsave = ycont[linnum][0];
	st = stsec[linnum];
	en = ensec[linnum];
#ifndef  STUDENT
	get_int_angle(linnum,&cstem,&sstem);
#endif
	for(i = 0 ; i < count ; i++) xs[i] = xsect[i];

	/*	Adjust position of section 0	*/

	if(!surfacemode && st == 0) {
		if(funnum <= 1)	{		/* lateral and vertical offset */
			xs[0] -= yline[linnum][0];
			yline[linnum][0] = yline[stemli][1];
		}
		else if(funnum <= 3) {	/* control points */
			getparam(0,linnum,&a,&hb,&c,&hd);
			xs[0] -= yline[linnum][0] + 0.5*a;
			ycont[linnum][0] = yline[stemli][1];
		}
	}

#ifndef STUDENT
	for(i = 0 ; i < ntank ; i++) {
		if(fl_line1[i] == line) {
			notok = usenext[funnum];
			/* also not OK if operation refers to next, and this is the last line of a tank */
			break;
		}
		else if(fl_line1[i] == linnum) {
			notok = useprev[funnum];
			/* also not OK if operation refers to previous, and this is the first line of a tank */
			break;
		}
	}
#endif
	if(notok) {
		message("Can not perform this operation for your nominated line");
	}
	else {
		(*smoo_func[funnum])();
		changed = TRUE;
	}
	memfree(xs);
	yline[linnum][0] = ysave;
	ycont[linnum][0] = ycsave;
}

/*	This routine is invoked by selection of menu items 0-3		*/

REAL	yssave;

void smoo_off()
{
	extern	int	prev_menu_result;
	extern	int	menu_result;
	REAL	a,t;
	int		i,j;
	REAL	ycssave,ycesave;
	int		restoreS,restoreE;
	REAL	zsave[maxsec];
#ifdef PROF
	REAL	cstem2,sstem2,t1,t2;
#endif

	restoreS = st <= 0;
	restoreE = FALSE;
	ycssave = ycont[linnum][st];
	ycesave = ycont[linnum][en];
	yssave = yline[linnum][st];
	for(i = st ; i <= en ; i++) zsave[i] = zline[linnum][i];

	/*	ADJUST FIRST SECTION POSITION AND FIRST LATERAL
	OFFSET FOR THIS LINE NUMBER
	*/
	if(linnum < numlin && funnum == 0) { /* lateral offset */
		if(!surfacemode) yline[linnum][st] = yline[stemli][1];
#ifndef STUDENT
		if(yline[linnum][st+1] < yline[stemli][st+1]) sstem = -sstem;	/* cstem ,sstem already defined */
		t = radstem[linnum];
		if(sstem != 0.0) t *= (1.0-cstem)/sstem;
		yline[linnum][st] += t;
#endif
	}

#ifndef STUDENT

	/*	Permit control points beyond line ends		*/

	if(funnum >= 2) {	/* control line */

		/*	Partial line			*/

		if(stsec[linnum-1] < st) {	/* xs[0] is already adjusted for section 0 */
			st--;
			if(st <= 0) xs[0] -= ycont[linnum][st];
			ycssave = ycont[linnum][st];
			ycont[linnum][st] = surfacemode ? 0.0 : yline[stemli][1];
			restoreS = TRUE;
		}
		if(ensec[linnum-1] > en) {
			en++;
			xs[en] -= ycont[linnum][en];
			ycesave = ycont[linnum][en];
			ycont[linnum][en] = surfacemode ? 0.0 : yline[stemli][1];
			restoreE = TRUE;
		}
		for(i = st ; i <= en ; i++) wb[i] = 0.5*(linewt[linnum][i]+linewt[linnum-1][i]);

#ifndef STUDENT
		j = linnum-1;
		while(stsec[j] > 0 && j > 0) j--;
		a = zline[linnum][st]-zline[j][st];
		if(a != 0.0) a = (zline[linnum][st]-zcont[linnum][st]) / a;

		t1 = radstem[linnum];
		if(sstem != 0.0) t1 *= (1.0-cstem)/sstem;
		get_int_angle(j,&cstem2,&sstem2);
		t2 = radstem[j];
		if(sstem2 != 0.0) t2 *= (1.0-cstem2)/sstem2;
		yline[linnum][st] += a*t2 + (1.0-a)*t1;
#endif
	}
	else {
		for(i = st ; i <= en ; i++) wb[i] = linewt[linnum][i];
	}
#else
	if(funnum >= 2) {	/* control line */
		for(i = st ; i <= en ; i++) wb[i] = 0.5*(linewt[linnum][i]+linewt[linnum-1][i]);
	}
	else {
		for(i = st ; i <= en ; i++) wb[i] = linewt[linnum][i];
	}
#endif

	/*	CALL SMOOTHING ROUTINE	*/

	linesm(	funnum == 0 ? yline[linnum] :
			funnum == 1 ? zline[linnum] :
			funnum == 2 ? ycont[linnum] :
			zcont[linnum]);

	st = stsec[linnum];
	en = ensec[linnum];
	if(restoreS) ycont[linnum][st] = ycssave;
	if(restoreE) ycont[linnum][en] = ycesave;
	if(!surfacemode) yline[linnum][st] = yssave;

	if(funnum == 1) {
		for(i = st ; i <= en ; i++) {
			t = zline[linnum][i];
			zline[linnum][i] = zsave[i];
			fix_vertical_controls(i,linnum,t);
			zline[linnum][i] = t;
		}
	}
}

/*	Smooth an angle or distance -

4/8		angle/distance to previous line
5/9		angle/distance to next line
6/10	angle/distance to previous control point
7/11	angle/distance to next control point			*/

void smoo_diff()
{
	int		i;
	extern	int	linnum;
	extern	int	lin;
	REAL	a,hb,c,hd;
	REAL	y,z,dy,dz,yold,zold;
	REAL	aa = 0.0,cc = 1.0;
	REAL	t;
	REAL	si,co;
	REAL	offsetb1[maxsec+2];
	REAL	offseta[maxsec];
	int		angles = (funnum <= 7);

	/*	"previous" is TRUE is the relationship is to a previous line or control point	*/

	int		previous = (funnum & 1) == 0;

	/*	"offsets" is TRUE if we are looking at line relationships, FALSE if the
	relationship is to control points
	*/
	int		offsets = funnum == 4 || funnum == 5 || funnum == 8 || funnum == 9;

	/*	"linnum" is the specified line, while "lin" is the higher-indexed of the pair
	of lines involved (it may be the same as linnum)
	*/
	if(funnum == 6 || funnum == 10) {

		/*	No adjustments needed - only the line and its own controls points involved	*/

		lin = linnum;

	}
	else if(previous) {

		/*	previous line or control point */

		st = max(st,stsec[linnum-1]);
		en = min(en,ensec[linnum-1]);
		lin = linnum;			/* "lin" is the line to which control points at allocated */
	}
	else {

		/*	next line or control point */

		st = max(st,stsec[linnum+1]);
		en = min(en,ensec[linnum+1]);
		lin = linnum+1;
	}

	/*	CALCULATE ANGLES IN DEGREES.  THE ANGLE REPRESENTS THAT
	MEASURED OUTWARD FROM THE LINE ABOVE, ABOUT THE LINE
	BELOW
	*/
	for(i = st ; i <= en ; i++) {
		dz = zline[lin][i] - zline[lin-1][i];	/* across control point */
		dy = yline[lin-1][i] - yline[lin][i];
		if(offsets) {	/* offset */
			if(previous) {
				z = dz;
				y = dy;
			}
			else {
				z = -dz;
				y = -dy;
			}
		}
		else {
			getparam(i,lin,&a,&hb,&c,&hd);
			z = 0.5 * c;
			y = 0.5 * a;/* control point relative position */
			if(!previous) {	/* next */
				z -= dz;
				y -= dy;
			}
		}
		wb[i] = 0.5*(linewt[lin-1][i]+linewt[lin][i]);


		if(angles) {	/* angles */
			if(y == 0.0 && z == 0.0) y = 1.0;
			offsetb1[i] = atan2(z,y)*degree;
		}
		else {
			offsetb1[i] = sqrt(z*z+y*y);
		}
		offseta[i] = offsetb1[i];
	}
	offsetb1[maxsec]   = 1.0;
	offsetb1[maxsec+1] = 1.0;

	/*	ALL SEGMENTS ARE VERTICAL AT THE STEM (NOTE THAT THIS CAN
	BE SET TO WHATEVER IS DESIRED IN THE SMOOTHING ROUTINE,
	SINCE THE RETURNED VALUE IS DISCARDED)
	*/
	if(angles) offsetb1[0] = 90.0;

	/*	SMOOTH THE ANGLES	*/

	ysave = yline[linnum][0];
	linesm(offsetb1);

	/*	SUBSIDIARY MENU: SELECT TO WHAT THE CHANGES ARE TO BE APPLIED	*/

	if(SmooDlg(&seldir,&linesel)) {

		/*	seldir= 0	for vertical movement
		1	for horizontal movement
		2	for radial/circular movement (distance / angle)

		linesel=	0	to move the previous offset when relationship is to the previous line
		the current offset when relationship is to the next line
		the control point when relationship is to a control point
		1	otherwise
		*/
		for(i = angles ? max(1,st) : st ; i <= en ; i++) {

			t = offsetb1[i];
			if(angles) {
				si = sind(t);
				co = cosd(t);
			}

			/*	Get curvature parameters for the curve above. "lin" is
			linnum   if we are editing angles/distances to the previous line,
			linnum+1 if we are editing the angles/distances to the next line.
			*/
			getparam(i,lin,&a,&hb,&c,&hd);
			tranpa(a,hb,c,hd,&aa,&cc);	/* aa and cc for tangent details */

			/*	(yold,zold) is the position of the "other" point (apart from that for
			the line specified)
			*/
			if(!offsets) {	/* control points */
				zold = zline[lin][i] - 0.5*c;
				yold = yline[lin][i] + 0.5*a;
			}
			else if(previous) {	/* previous line */
				zold = zline[linnum-1][i];
				yold = yline[linnum-1][i];
			}
			else {			/* next line */
				zold = zline[linnum+1][i];
				yold = yline[linnum+1][i];
			}

			/*	y and z are distance components from reference line (offsets or control points) to
			the specified line
			*/
			z = zold - zline[linnum][i];
			y = yold - yline[linnum][i];

			dz = z;
			dy = y;
			if(seldir == 0) {	/*	Move vertically	*/
				if(!angles) {	/* distances */
					t = t*t - y*y;
					if(t >= 0.0) {
						dz = sqrt(t);
						if(zold < zline[linnum][i]) dz = -dz;
					}
				}
				else if(fabs(co) > 1.0e-4) { /* angles: co = 0.0 if angle to point is vertical */
					dz = - y * (si/co);
				}

			}
			else if(seldir == 1) {	/*	Move horizontally	*/
				if(!angles) {
					t = t*t - z*z;
					if(t >= 0.0) {
						dy = sqrt(t);
						if(yold < yline[linnum][i]) dy = -dy;
					}
				}
				else if(fabs(si) > 1.0e-4) {
					dy = - z * (co/si);
				}

			}
			else {		/* Move radially (distances) or circularly (angles)	*/
				aa = y*y + z*z;
				if(angles) {	/* move circularly */
					t = sqrt(aa);
					dy =   t * co;
					dz = - t * si;
				}
				else {	/* move radially */
					if(aa > 0.0) {
						t /= sqrt(aa);
						dy *= t;
						dz *= t;
					}
				}
			}

			/*	dy and dz now become the change in relative position of the pair of points	*/

			dz -= z;
			dy -= y;

			/*	linesel=	0	to move the previous offset when relationship is to the previous line
			the current offset when relationship is to the next line
			the control point when relationship is to a control point
			1	otherwise
			*/
			if(offsets) {
				if(previous) {
					dz = -dz;
					dy = -dy;
				}

				if(linesel == 0) {	/* move lower-indexed of pair */
					t = zline[lin-1][i] - dz;
					fix_vertical_controls(i,lin-1,t);
					zline[lin-1][i] = t;
					yline[lin-1][i] -= dy;
				}
				else {		/* move higher-indexed of pair */
					t = zline[lin][i] + dz;
					fix_vertical_controls(i,lin,t);
					zline[lin][i] = t;
					yline[lin][i] += dy;
				}
			}
			else {
				if(linesel == 0) {	/* move control point */
					if(previous) {	/* move control point for this line */
						zcont[lin][i] += dz;
						ycont[lin][i] += dy;
					}
					else {		/* move control point for next line */
						zcont[lin][i] += dz;
						ycont[lin][i] += dy;
					}
				}
				else {		/* move line */
					if(previous) {	/* move previous line */
						t = zline[lin][i] - dz;
						fix_vertical_controls(i,lin,t);
						zline[lin][i] = t;
						yline[lin][i] -= dy;
					}
					else {		/* move next line */
						t = zline[lin][i] + dz;
						fix_vertical_controls(i,lin,t);
						zline[lin][i] = t;
						yline[lin][i] += dy;
					}
				}
			}
		}
	}
}

/*	SMOOTH ANY ONE OF THE 8 "LINES" USED IN HULL MODEL	*/

void linesm(REAL os[])
{
	extern int	xcursor,ycursor;
	char	text[80];
	HWND	saveWnd = hWnd;
	int		i,j;
	MSG		msg;
	extern HWND	hWndMain;
	RECT	rw,rc;
	int		border,top;
	extern int	numtool;
	int		savetool = numtool;
#ifdef PROF
	int		transom_setting = transom;
	void	calc_stringers(int);
#endif

#ifdef linux
	char	*entry;
	void	CreateMenuAndToolbar();
	extern	Widget hScroll,vScroll,Toolbar,Menubar,mainWindow,MenuHolder;
	extern	XtAppContext app_context;
	KeySym	keysym;
	XEvent	event;
	char	chars[12];
	extern int mainw;
	Widget	w;
	int	xm,ym;
	extern int numtool;
	extern Widget ToolButton[];
	Arg		argval[40];		/* Widget command arguments */
	int		argnum;			/* widget command count */
	extern	Pixmap ToolbarPixmap[40];
	extern	Widget glw;
#else
	void	InitToolbar(void);
	void	DestroyToolbar(void);
	void	InitWorkwin(int scroll);
	HMENU	hSmooMenu;
	extern	HMENU hMainMenu;
	void	makemenu(void);
	extern	int scrollable;

#endif

	if(count < 2) return;

	/*	Save the line details for "Undo", and set the also-plot selection
	to none.
	*/
	for(i = st ; i <= en ; i++) {
		linesave[i] = os[i];
		offsetb[i] = os[i];
	}
	offsetb[maxsec]   = os[maxsec];
	offsetb[maxsec+1] = os[maxsec+1];

	perset(0);
	sprintf(text,"Smoothing %s for line %d",smoo_opt[funnum],linnum+1);
	pull = 0;
	follow = 0;
	mousedrag = 0;
	delta0 = 0.04;
	run1 = os[maxsec],
	run2 = os[maxsec+1];
	smoosect = -1;
#ifdef linux
	hWnd = hWndMain;
	XtUnmanageChild(MenuHolder);
	cls(FALSE);

	if(SmooMenu == NULL) {
		SmooMenu = XmVaCreateSimpleMenuBar(
			mainWindow,
			"smoomenu",
			XmNx,0,
			XmNy,0,
			XmNborderWidth,0,
			XmNheight,32,
			XmNbackground,ButtonBackground,
			XmNborderColor,ButtonBackground,
			XmNtopShadowColor,TopShadow,
			XmNbottomShadowColor,BottomShadow,
			XmNfontList,windowfontlist,
			XmNwidth,mainw,
			XmNborderWidth,0,
			XmNmarginHeight,2,
			XmNmarginWidth,1,
			XmNallowResize,True,
			XmNattachment,XmATTACH_FORM,
			NULL);

		i = -1;
		while( *(entry = SmooMenuItem[++i]) != 0) {
			strcpy(text,entry);
			if(xmaxi < 900) text[4] = 0;
			w = XtVaCreateManagedWidget(text,
				xmCascadeButtonWidgetClass,
				SmooMenu,
				XmNwidth,50,
				XmNpushButtonEnabled,True,
				XmNbackground,ButtonBackground,
				XmNborderColor,ButtonBackground,
				XmNtopShadowColor,TopShadow,
				XmNbottomShadowColor,BottomShadow,
				XmNfontList,windowfontlist,
				NULL);
			XtAddCallback(w,XmNactivateCallback,SmooMenuCallback,(XtPointer) i);
		}
	}
	XtManageChild(SmooMenu);
	hDC = (HDC) GetDC(hWnd);
	setup(scrdev);
	(*init)();
	shiftdown = FALSE;
	redraw_smooth(&curmax,&curmin,&offbot,&offtop,line_also,FALSE);
	SwapBuf();
	SmooQuit = False;

	while(!SmooQuit) {
		XtAppNextEvent(app_context,&event);
		if(event.type == KeyPress) {
			XLookupString(&(event.xkey),chars,sizeof(chars),&keysym,NULL);
			shiftdown = event.xkey.state == 1;
			SmooLineKeyDown((int) keysym);
		}
		else if(event.xkey.window == win) {
			if(event.type == ButtonPress) {
				if(event.xbutton.button == Button1) {	/* left button - drag */
					SmooLineLeftButtonDown(event.xbutton.x);
				}
			}
			else if(event.type == ButtonRelease) {
				if(event.xbutton.button == Button1) {		/* left button - drag */
					mousedrag = 0;
				}
			}
			else if(event.type == MotionNotify) {
				SmooLineMouseMove(event.xmotion.y);
			}
			else {
				XtDispatchEvent(&event);
			}
		}
		else if(scrdev == 0 && event.xkey.window == XtWindow(glw)) {	// GL
			if(event.type == ButtonPress) {
				if(event.xbutton.button == Button1) {	/* left button - drag */
					SmooLineLeftButtonDown(event.xbutton.x);
				}
			}
			else if(event.type == ButtonRelease) {
				if(event.xbutton.button == Button1) {		/* left button - drag */
					mousedrag = 0;
				}
			}
			else if(event.type == MotionNotify) {
				SmooLineMouseMove(event.xmotion.y);
			}
			else {
				XtDispatchEvent(&event);
			}
		}
		else {
			XtDispatchEvent(&event);
		}
		XFlush(display);
	}
	(*endgrf)();
	for(i = st ; i <= en ; i++) os[i] = offsetb[i];
	os[maxsec] = offsetb[maxsec];
	os[maxsec+1] = offsetb[maxsec+1];

	hWnd = saveWnd;

	XSetFunction(display,gc,GXcopy);
	XtUnmanageChild(SmooMenu);

	cls(FALSE);
	update_func = NULL;
	XtManageChild(hScroll);
	XtManageChild(vScroll);
	XtManageChild(MenuHolder);

#else
	savetool = numtool;

	DestroyToolbar();
	numtool = 0;
	InitToolbar();

	GetSizes(hWnd);

	GetWindowRect(hWnd,&rw);	// window screen coordinates (outside of window)
	GetClientRect(hWnd,&rc);

	if(rc.right - rc.left > 960)
		hSmooMenu = LoadMenu(hInst,"HULLSMEX_MEN");
	else if(rc.right - rc.left > 785)
	    hSmooMenu = LoadMenu(hInst,"HULLSMOO_MEN");
	else {
	    hSmooMenu = LoadMenu(hInst,"HULLSMMI_MEN");
//		message("Window width needs to be at least 785 pixels in this mode");
//		goto no_smooth;
	}

	DestroyMenu(hMainMenu);
	SetMenu(hWndMain,hSmooMenu);
	SetROP2(hDC,R2_XORPEN);

	border = ((rw.right-rw.left)-(rc.right-rc.left))/2;
	xswleft = rw.left+border;
	yswtop = rw.bottom-border-(rc.bottom-rc.top);

	(*init)();

	SmooQuit = 0;
	shiftdown = FALSE;
	SetWindowLong(hWnd,GWL_WNDPROC,(long) SmoothProcedure);
	SetWindowLong(hWndMain,GWL_WNDPROC,(long) SmoothProcedure);
	PostMessage(hWnd,WM_PAINT,0,0l);

	while(!SmooQuit && GetMessage(&msg,hWndMain,0,0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	SetWindowLong(hWnd,GWL_WNDPROC,(long) WindowProcedure);
	SetWindowLong(hWndMain,GWL_WNDPROC,(long) WindowProcedure);

	for(i = st ; i <= en ; i++) os[i] = offsetb[i];
	os[maxsec] = offsetb[maxsec];
	os[maxsec+1] = offsetb[maxsec+1];

no_smooth:
	SetROP2(hDC,R2_COPYPEN);
	hMainMenu = LoadMenu(hInst,"HULLFORM_MEN");
	DestroyMenu(hSmooMenu);
	SetMenu(hWndMain,hMainMenu);
	makemenu();

	numtool = savetool;
	InitToolbar();

	update_func = NULL;
#endif

#ifdef PROF
	zoom = 0;
#endif
	scaled = TRUE;
#ifdef PROF
	transom = transom_setting;
	recalc_transom();
	for(j = 1 ; j < numlin ; j++) {
		if(strmode[j] >= 0) calc_stringers(j);
	}
#endif
	numtool = savetool;
	check_negative_offsets();
}

#ifndef linux

LRESULT CALLBACK SmoothProcedure(HANDLE hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	extern int	xmaxi,ymaxi;
	extern REAL	xgslope,ygslope;
	extern int	ychar;
	PAINTSTRUCT ps;
	extern int	ymaxi;
	REAL	delta;
	extern int	in;
	extern int	context_id;
	extern int	dlgboxpos;
	extern REAL	ygorigin;
	void	arrowcursor(void);
	RECT	rc1,rc2;

	switch(msg) {

	case WM_SIZE:
		GetSizes(hWnd);
		break;

	case WM_PAINT:
		BeginPaint(hWnd,&ps);
		redraw_smooth(&curmax,&curmin,&offbot,&offtop,line_also,FALSE);
		SwapBuf();
		EndPaint(hWnd,&ps);

		mousedrag = 0;
		break;

		/*	Translate keystrokes into equivalent commands	*/

	case WM_CHAR:
		SmooLineKeyDown((int) wParam);
		break;

	case WM_COMMAND:
		SmooLineKeyDown((int) comchar[wParam - 101]);
		break;

	case WM_LBUTTONDOWN:
		SmooLineLeftButtonDown((int) LOWORD(lParam));
		break;

	case WM_NCLBUTTONUP:
	case WM_LBUTTONUP:
		num = 0;
		mousedrag = 0;
		if(autofair[linnum]) {
			if(scrdev == 1) plotline(curmin,curmax,offbot,offtop,offsetb,INICOL);
			DrawSmooth();
		}
		break;

	case WM_MOUSEMOVE:
		arrowcursor();

	case WM_NCMOUSEMOVE:

		/*	left-hand key is "drag" key					*/
		if(mousedrag) {
			GetWindowRect(hWnd,&rc1);
			GetClientRect(hWnd,&rc2);
			SmooLineMouseMove((int) HIWORD(lParam) - rc1.top);
		}

		break;

	case WM_KEYUP:
		if((int) wParam == VK_SHIFT) {
			shiftdown = FALSE;
		}
		else {
			return(DefWindowProc(hWnd,msg,wParam,lParam));
		}
		break;

	case WM_KEYDOWN:
		switch((int) wParam) {
		case VK_F1:	/* context-sensitive help */
			context(context_id);
			break;

		case VK_F2:	/* quick save */
			save_file();
			break;

		case VK_SHIFT:
			shiftdown = TRUE;
			break;

		case VK_UP:
		case VK_DOWN:
			if(smoosect >= 0) {
				/*					 DOWN OR UP-ARROW	*/
				if(wParam == VK_UP) {
					delta = -delta0;
				}
				else {
					delta = delta0;
				}
				if(scrdev == 1)
					plotline(curmin,curmax,offbot,offtop,offsetb,INICOL);
				movepoint(follow,smoosect,linnum,delta,funnum,curmin,curmax,offbot,offtop);
				DrawSmooth();
			}
			break;

		case VK_TAB:
			if(smoosect >= 0) {
				xorcol(1);
				(*move)(xs[smoosect],0.0);
				(*draw)(xs[smoosect],offsetb[smoosect]);
				xorcol(2);
				(*move)(xs[smoosect],0.0);
				(*draw)(xs[smoosect],offsetb[smoosect]);
			}
			if(shiftdown) {
				while(smoosect > st) {
					smoosect--;
					if(v7mode || master[smoosect]) break;
				}
			}
			else {
				while(smoosect < en) {
					smoosect++;
					if(v7mode || master[smoosect]) break;
				}
			}
			xorcol(2);
			(*move)(xs[smoosect],0.0);
			(*draw)(xs[smoosect],offsetb[smoosect]);
			xorcol(1);
			(*move)(xs[smoosect],0.0);
			(*draw)(xs[smoosect],offsetb[smoosect]);
			break;

		}
		break;

	case WM_RBUTTONDOWN:
	case WM_NCRBUTTONDOWN:
		if(pull) {
			pull = 0;
			break;
		}

		/*	... else pass through to exit processing ...	*/

	case WM_DESTROY:
	case WM_CLOSE:
	case WM_QUIT:
		SmooQuit = 1;
		break;

	default:
		return(DefWindowProc(hWnd,msg,wParam,lParam));

	}		/* end switch on main command messages */
	return(0l);
}

#endif

/*=======================================================================	*/

/*	PLOT HULL LINE AND ITS CURVATURE ON HULL-SMOOTHING SCREEN	*/

void plotline(REAL ylo1,REAL yhi1,REAL ylo2,REAL yhi2,REAL array[],int col)
{
	extern HDC hDC;
	int	i,n;
	REAL xa;
	REAL t[maxsec],x[maxsec],w[maxsec];
	REAL as[maxsec],bs[maxsec],cs[maxsec],deriv[maxsec];

	xorcol(col);

	n = 0;
	(*newlin)();
	setranges(xmin,xmax,ylo2,yhi2);
	for(i = st ; i <= en ; i++) {
		if(!ignsmoo[i]) {
			t[n] = xs[i];
			x[n] = array[i];
			w[n] = wb[i];
			n++;
			(*draw)(xs[i],array[i]);
		}
		else {
			(*newlin)();
		}
	}

	/*	This call only returns cubic polynomial terms	*/
	spline2(t,x,w,n,t,x,0,run1,run2,deriv,as,bs,cs);

	(*newlin)();
	setranges(xmin,xmax,ylo1,yhi1);
	for(i = 1 ; i < n-1 ; i++) {
		xa = 2.0*bs[i] + 3.0*as[i]*(t[i+1]-t[i]);
		if(xa >= ylo1 && xa <= yhi1) {
			(*draw)(t[i],xa);
			(*draw)(t[i+1],xa);
		}
		else {
			(*newlin)();
		}
	}

	setranges(xmin,xmax,ylo2,yhi2);
}

/*	SHOW THE "PULL" INCREMENT */

void shopul(REAL delta0)
{
	char line[40];
	extern HDC hDC;

	sprintf(line,"Del %7.4f            ",delta0);
	if(scrdev == 0) redraw_smooth(&curmax,&curmin,&offbot,&offtop,line_also,TRUE);
	pstrbkg(line,1,0,0,1);
	SwapBuf();
}

/*	move a section point	*/

void movepoint(int follow,int smoosect,int linnum,REAL linesel,
	int funnum,REAL curmin,REAL curmax,REAL offbot,REAL offtop)
{
	if(!v7mode && !master[smoosect]) {
		if(MessageBox(hWnd,"You have tried to select a point on a non-master section. Do you want this section to be a master section?",
			"YOU CAN NOT EDIT A NON-MASTER SECTION",MB_ICONQUESTION | MB_YESNO) == IDYES) {
			master[smoosect] = TRUE;
		}
		else {
			return;
		}
	}

	if(!follow) {

		/*		"Pull" OPTION					*/
		offsetb[smoosect] += linesel;

		/*		"Follow" OPTION - VALID ONLY FOR OFFSETS	*/
	}
	else if(funnum == 0) {
		movpoi(smoosect,linnum,linnum,linesel);
		offsetb[smoosect] = yline[linnum][smoosect];
	}
	else if(funnum == 1) {
		movpoi(smoosect,linnum,linnum+1,1.0-linesel);
		offsetb[smoosect] = zline[linnum][smoosect];
	}

}

void redraw_smooth(REAL *curmax,REAL *curmin,REAL *offbot,REAL *offtop,int *line_also,int smooth_scaled)
{
	extern HDC hDC;
	extern HPEN hPen,hPenOrig;
	static REAL maxcur,mincur;
	static REAL maxoff,minoff;
	REAL a,a1,a2,offs;
	int i;
	REAL dy,x1,x2,dxx,x,xl,xr,xsize;
	REAL dxst;
	REAL delx,dx;
	int ndp;
	REAL fac;
	REAL dxmin,xprev,xa,ya;
	REAL bot,top;
	int smoosect,j;
	REAL xx;
#ifndef STUDENT
	extern REAL *radstem;
#endif
	extern REAL xgslope,ygslope;

	/*	The paint message redraws the screen			*/

	cls(0);
	if(!smooth_scaled) {
		maxcur = 0.0;
		mincur = 0.0;
		maxoff = 0.0;
		minoff = 0.0;
		a1 = 0.0;
		for(i = st ; i <= en ; i++) {
			if(fabs(offsetb[i]) < 10000.0) {
				if(!ignsmoo[i]) {
					if(offsetb[i] > maxoff) maxoff = offsetb[i];
					if(offsetb[i] < minoff) minoff = offsetb[i];
				}
			}
			if(i < en && xs[i+1] > xs[i]) {
				a2 = (offsetb[i+1]-offsetb[i])/(xs[i+1]-xs[i]);
				if(i>st && !(ignsmoo[i] || ignsmoo[i-1] || ignsmoo[i+1]) && xs[i+1] != xs[i-1]) {
					a = (a2-a1)/(0.5*(xs[i+1]-xs[i-1]));	// d2y/dx2
					if(a > maxcur) maxcur = a;
					if(a < mincur) mincur = a;
				}
				a1 = a2;
			}
		}

		if(linnum == stemli && funnum == 1) {
			for(i = 0 ; i < numlin ; i++) minoff = min(minoff,zline[i][0]);
		}

		*curmin = min(0.0,1.5* mincur-0.00001);
		*curmax = max(0.0,1.5* maxcur+0.00001);
		*offbot = maxoff+0.01;
		*offtop = minoff-0.01;

		/*	adjust ranges to fit each half of plot within half of the screen */

		dy = *offtop - *offbot;
		*offbot -= 1.5*dy;
		*offtop += 0.5*dy;
		dy = *curmax - *curmin;
		*curmax += 1.5*dy;
		*curmin -= 0.5*dy;
	}

	x1 = xs[st];			/* provisional left edge of plot */
	if(linnum == stemli && funnum == 1){/* space to draw stem	*/
		dxst = dxstem();		/* curve if required	*/
		x1 += ysave - dxst;
	}

	x2 = xs[en];
	dxx = (x2 - x1)*0.2;	/* allow 20% extra at each end */

	/*	Depending on whether stem is right or left, set x-limits	*/

	if(posdir < 0.0) {
		xl = x1;
		xr = x2;
	}
	else {
		xl = x2;
		xr = x1;
		dxx = - dxx;
	}
	setranges(xl-dxx,xr+dxx,*offbot,*offtop);
	xsize = xr - xl + dxx + dxx;

#ifdef linux
	XSetFunction(display,gc,GXcopy);
#else
	if(scrdev == 1) SetROP2(hDC,R2_COPYPEN);
#endif

	(*colour)(1);
	(*move)(xl - dxx,0.25 * (*offbot) + 0.75 * (*offtop));
	(*plstr)("VALUES");
	(*move)(xl,0.0);
	(*draw)(xr,0.0);
	(*move)(xl,maxoff);
	(*draw)(xl,minoff);

	/*	LABEL VERTICAL OFFSETS AT SELECTABLE SCALE	*/

	delx = axincr(maxoff-minoff+0.02);
	ndp = -log10((double) delx);
	if(delx < 1.0) ndp++;
	dx = (REAL) xchar / xgslope;
	dy = (REAL) ychar / ygslope;

	/*	FLIP VERTICAL SCALE FOR VERTICAL OFFSETS WHEN INVERT IS SELECTED	*/

	if(funnum == 1 || funnum == 3) {
		fac = invert;
	}
	else {
		fac = 1.0;
	}

	for(x = delx*min(0.0,floor(minoff/delx)) ; x < maxoff ; x += delx) {
		(*move)(xl,x);
		(*draw)(xl+dx,x);
		(*move)(xl-5.0*dx,x + 0.5*dy);
		if(ndp > 0)
			plfix(fac*x,5,ndp);
		else
		    plint(((int)(fac*x+0.5)),5);
	}

	setranges(xl-dxx,xr+dxx,*curmax,*curmin);
	(*move)(xl - dxx,0.5 * (*curmin) + 0.5 * (*curmax));
	(*plstr)("CURVATURE");
	(*move)(xl,0.0);
	(*draw)(xr,0.0);
	(*move)(xl,1.2* mincur -0.00001);
	(*draw)(xl,1.2* maxcur +0.00001);

	ya = 1.5*maxcur;
	dy = 0.15*(maxcur - mincur);
	dxmin = fabs(dx*5.0);
	xprev = -1.0e+16;
	for(i = st ; i <= en ; i++) {
		xa = xs[i];
		(*move)(xa,ya);
		if(xa-xprev >= dxmin) {
			(*draw)(xa,ya-dy);
			if(i > 9)
				(*move)(xa-1.2*dx,ya-dy);
			else
			    (*move)(xa-1.3*dx,ya-dy);
			plint(i,2);
			xprev = xa;
		}
		else {
			(*draw)(xa,ya-0.5*dy);
		}
	}

	/*	PLOT INITIAL OFFSET AND CURVATURE LINES	*/

#ifdef linux
	if(scrdev == 1) XSetFunction(display,gc,GXxor);
#else
	if(scrdev == 1) SetROP2(hDC,R2_XORPEN);
#endif
	xorcol(1);
	setranges(xl-dxx,xr+dxx,*offbot,*offtop);
	plotline(*curmin,*curmax,*offbot,*offtop,offsetb,INICOL);

	/*	PLOT STEM LINE AND CURVATURE IF LINE IS STEM LINE	*/

	if(linnum == stemli && funnum == 1 && st == 0) {
		offs = yline[stemli][0];
		if(!surfacemode) yline[stemli][0] = ysave;
		if(posdir < 0.0) {
			a = dxst+dxx;
			setranges(a,a-xsize,*offbot,*offtop);
//			setranges(a,a-xsize,*curmin,*curmax);
		}
		else {
			a = dxst - dxx;
			setranges(a+xsize,a,*offbot,*offtop);
//			setranges(a+xsize,a,*curmin,*curmax);
		}
		xorcol(3);
		drasec(0,0,1.0,0);
		setranges(xl-dxx,xr+dxx,*curmin,*curmax);
		if(!surfacemode) yline[stemli][0] = offs;
	}

	xorcol(2);
	setranges(xl-dxx,xr+dxx,*offbot,*offtop);
	for(i = st ; i < en ; i++) {
		if(v7mode || master[i]) {
			(*move)(xs[i],0.0);
			(*draw)(xs[i],offsetb[i]);
		}
	}

	/*	SHOW WATERLINE ON VERTICAL OFFSET PLOTS	*/

	if(funnum == 1) {
		xorcol(4);
		(*move)(xl,wl-beta*xl);
		(*draw)(xr,wl-beta*xr);
		xorcol(1);
		(*plstr)("WL");
	}

	if(funnum <= 5) {
		bot = maxoff-minoff;
		top = maxoff+bot;
		bot = minoff-bot;

		for(j = 0 ; j < extlin ; j++) {
			if(line_also[j+1]) {
				smoosect = funnum & 1;
				if(smoosect) {	/* if offset is z-value */
					xx = zline[j][0];
				}
				else {	/* if it is y-value */
					xx = surfacemode ? yline[j][0] : yline[stemli][1];
#ifndef STUDENT
					if(radstem[j] != 0.0) {
						dx = radstem[j];
						if(sstem != 0.0) dx *= (1.0-cstem)/sstem;
						if(yline[j][1] > yline[stemli][1]) {
							xx += dx;
						}
						else {
							xx -= dx;
						}
					}
#endif
				}
				if(xx > bot && xx < top) {
					if(surfacemode)
						(*move)(xsect[0],xx);
					else if(j == linnum)
						(*move)(xsect[0]-yssave,xx);
					else
					    (*move)(xsect[0]-yline[j][0],xx);
				}
				else {
					(*newlin)();
				}

				for(i = 1 ; i < count ; i++) {
					if(smoosect)
						xx = zline[j][i];
					else
						xx = yline[j][i];
					if(xx > bot && xx < top) {
						(*draw)(xsect[i],xx);
					}
					else {
						(*newlin)();
					}
				}
			}
		}
	}
}

void listseq(int *on,char *p,int n)
{
	int i;
	int start = -1;
	int all = TRUE;
	int none = TRUE;
	char *textstart = p;
	*p = 0;
	for(i = 0 ; i < n ; i++) {
		if(! on[i]) {
			all = FALSE;
finish:
			if(start >= 0) {
				if(i > start + 1) {
					sprintf(p,"%d:%d,",start,i-1);
				}
				else {
					sprintf(p,"%d,",start);
				}
				p = strchr(p,0);
				start = -1;
			}
		} else {
			if(start == -1) start = i;
			none = FALSE;
		}
	}
	if(start != -1) goto finish;

	if(all) {
		strcpy(textstart,"ALL");
	} else if(none) {
		strcpy(textstart,"NONE");
	} else {
		*--p = 0;	/* delete last comma */
	}
}

REAL flread(char **str);

void listin(char *line,REAL table[])
{
	int	n;
	REAL w1,w2;
	int i = 0;

	/*	SCAN THROUGH INPUT LINE:	*/

	while(*line) {

		n = *line;

		/*	SPACE OR COMMA: LOOP BACK FOR NEXT CHARACTER	*/

		if(n == ' ' || n == ',') {
			line++;

			/*	DIGIT:	*/

		}
		else if(n == '-' || n == '.' || (n >= '0' && n <= '9')) {
			w1 = flread(&line);
			table[i++] = w1;

			/*	MUTIPLY:	*/

		}
		else if(n == '*') {

			/*		PRECEDING IS A COUNT: READ VALUE REPEATED	*/

			n = w1;
			line++;
			w2 = flread(&line);
			i--;
			while(n-- > 0) table[i++] = w2;

		}
		else {
			message("IMPROPER CHARACTER IN INPUT");
			break;
		}
	}
}

REAL flread(char **str)
{
	char *init = *str;
	REAL result;
	int c;

	while(isdigit(c = **str) || c == '-' || c == '.') (*str)++;
	c = **str;
	**str = '\0';
	if(sscanf(init,"%f",&result) < 1) result = -1.0e+30;
	**str = (char) c;
	return result;
}


/*	Modify line weights for the current line	*/

REAL lineweight = 1.0;

void weight_entry(char *text,int i,REAL wt);
void centre_dlg(HWND hWndDlg);

#ifdef linux

Widget LabelWidget(Widget Wdialog,char *text,int centre,int x,int y, int w, int h);
Widget TextWidget(Widget Wdialog,int x,int y, int w, int h);
Widget ListWidget(Widget Wdialog,int x,int y, int w, int h,int multiple);
Widget ButtonWidget(Widget Wdialog,char *text,int x,int y, int w, int h);

Widget wWeightList,wWeightValue;
int WeightProceed;
extern Widget mainWindow;
extern XtAppContext app_context;
REAL *wtval;

void WeightChangeCB(Widget w, XtPointer client_data, XtPointer call_data)
{
	char *text;
	char textdata[MAX_PATH];
	REAL value;
	XmString xstr;
	int i,i1;

	text = XmTextGetString(wWeightValue);
	if(sscanf(text,"%f",&value) == 1) {
		for(i = stsec[linnum] ; i < ensec[linnum] ; i++) {
			if(XmListPosSelected(wWeightList,i+1)) {
				wtval[i] = value;
				weight_entry(textdata,i,value);
				i1 = i + 1;
				xstr = XmStringCreateSimple(textdata);
				XmListReplacePositions(wWeightList,&i1,&xstr,1);
				XmStringFree(xstr);
			}
		}
	}
	else {
		message("Invalid entry in edit box");
	}
	XtFree(text);
}

void WeightUndoCB(Widget w, XtPointer client_data, XtPointer call_data)
{
	int i;
	char text[MAX_PATH];
	XmString xstr;
	XmListDeleteAllItems(wWeightList);
	for(i = stsec[linnum] ; i < ensec[linnum] ; i++) {
		weight_entry(text,i,linewt[linnum][i]);
		wtval[i] = linewt[linnum][i];
		xstr = XmStringCreateSimple(text);
		XmListAddItem(wWeightList,xstr,0);
		XmStringFree(xstr);
	}
}

void WeightCB(Widget w, XtPointer client_data, XtPointer call_data)
{
	WeightProceed = (int) client_data;
}

void get_weights(int wtline)
{
	int i,pending;
	Widget Wshell,Wdialog,w;
	XmString xstr;
	char text[60];

	if(!memavail((void **) &wtval,sizeof(REAL)*maxsec)) {
		message ("Out of memory");
		return;
	}

	linnum = wtline;

	/*	Create the dialog shell	and window	*/

	Wshell = XmCreateDialogShell(mainWindow,"Change Inter-Section Flexibility",NULL,0);

	Wdialog = XtVaCreateManagedWidget(
		"weights",
		xmBulletinBoardWidgetClass,
		Wshell,
		XmNdialogStyle,XmDIALOG_APPLICATION_MODAL,
		XmNautoUnmanage,False,
		XmNallowOverlap,False,
		XmNnoResize,True,
		XmNmarginWidth,0,
		XmNmarginHeight,0,
		XmNwidth,274,
		XmNheight,258,
		XmNdefaultPosition,False,
		XmNresizePolicy,XmRESIZE_NONE,
		XmNbackground,DialogBackground,
		XmNborderColor,ButtonBackground,
		XmNtopShadowColor,TopShadow,
		XmNbottomShadowColor,BottomShadow,
		NULL);
	centre_dlg(Wdialog);
	XtManageChild(Wdialog);

	wWeightList = ListWidget(Wdialog,12,12,132,200,True);
	w = ButtonWidget(Wdialog,"Change",160,98,100,28);
	XtAddCallback(w,XmNactivateCallback,WeightChangeCB,(XtPointer) 0);
	wWeightValue = TextWidget(Wdialog,160,58,100,24);
	w = ButtonWidget(Wdialog,"Undo",160,140,100,28);
	XtAddCallback(w,XmNactivateCallback,WeightUndoCB,(XtPointer) 0);
	w = LabelWidget(Wdialog,"New value ...",False,162,38,88,16);

	w = ButtonWidget(Wdialog,"Done", 26,220,100,28);
	XtAddCallback(w,XmNactivateCallback,WeightCB,(XtPointer) IDOK);
	w = ButtonWidget(Wdialog,"Help",146,220,100,28);
	XtAddCallback(w,XmNactivateCallback,WeightCB,(XtPointer) IDHELP);

	XmListDeleteAllItems(wWeightList);
	for(i = stsec[linnum] ; i < ensec[linnum] ; i++) {
		weight_entry(text,i,linewt[linnum][i]);
		wtval[i] = linewt[linnum][i];
		xstr = XmStringCreateSimple(text);
		XmListAddItem(wWeightList,xstr,0);
		XmStringFree(xstr);
	}

	WeightProceed = 0;
	while(WeightProceed == 0 | (pending = XtAppPending(app_context)) ) {
		if(pending)
			XtAppProcessEvent(app_context, XtIMAll);
		else
		    usleep(100000);
	}
	XtUnmanageChild(Wdialog);

	if(WeightProceed == 1) {
		for(i = stsec[linnum] ; i < ensec[linnum] ; i++) linewt[linnum][i] = wtval[i];
	}
	memfree(wtval);
}


#else

BOOL CALLBACK WtDlgProc(HWND hWndDlg,unsigned msg,WORD wParam,LONG lParam);

void get_weights(int wtline)
{
	int		code;
	linnum = wtline;
	code = DialogBox(hInst,(char *) WEIGHTS,hWnd,(DLGPROC) WtDlgProc);
	if(code < 0) (void) MessageBox(hWnd,"WEIGHTS","Could not create dialog box",MB_OK);
}

BOOL CALLBACK WtDlgProc(HWND hWndDlg,unsigned msg,WORD wParam,LONG lParam)
{
	char text[40];
	REAL value;
	int i,j,n,i1,i2;
	char *p;
	int isel[maxsec];
	int sel[maxsec];
	static int *was_sel;
	BOOL colret;

	if(ColourMessage(msg,wParam,lParam,&colret)) return colret;

	switch(msg) {

	case WM_INITDIALOG:
		centre_dlg(hWndDlg);
		(void) memavail((void *) &was_sel,maxsec*sizeof(int));
reset:
		SendDlgItemMessage(hWndDlg,DLGLBX,LB_RESETCONTENT,0,0l);
		n = 0;
		for(i = stsec[linnum] ; i < ensec[linnum] ; i++) {
			weight_entry(text,i,linewt[linnum][i]);
			SendDlgItemMessage(hWndDlg,DLGLBX,LB_ADDSTRING,0,(LPARAM) (LPCSTR) text);
			was_sel[n++] = FALSE;
		}
		SendDlgItemMessage(hWndDlg,DLGLBX,LB_SETCURSEL,0,0L);
		SetDlgItemText(hWndDlg,DLGEDIT,"1.0");
		shiftdown = FALSE;
		break;

	case WM_COMMAND:
		switch(wParam) {
		case DLGLBX:
			switch(HIWORD(lParam)) {	/* notify code */

			case LBN_DBLCLK:		/* double click - set selection to value */
				goto setsel;

			case LBN_SELCHANGE:
				if((n = SendDlgItemMessage(hWndDlg,DLGLBX,LB_GETSELITEMS,maxsec,
					(LPARAM) (int *) isel)) != 0) {
					for(i = 0 ; i < count ; i++) sel[i] = FALSE;
					for(i = 0 ; i < n ; i++) {
						sel[ isel[i] ] = TRUE;
					}
					n = ensec[linnum] - stsec[linnum] + 1;
					if( (GetAsyncKeyState(VK_SHIFT) | 1) < 0) {
						i1 = count;
						i2 = -1;
						for(i = 0 ; i < n ; i++) {
							if(was_sel[i]) {
								if(i > i2) i2 = i;
								if(i < i1) i1 = i;
							}
						}
						for(i = 0 ; i < n ; i++) {
							if(sel[i] && !was_sel[i]) {
								if(i < i1 && i1 < count) {
									SendDlgItemMessage(hWndDlg,DLGLBX,LB_SELITEMRANGE,TRUE,MAKELPARAM(i+1,i1-1));
									for(j = i+1 ; j < i1 ; j++) was_sel[j] = TRUE;
									i1 = i;
								}
								else if (i > i2 && i2 >= 0) {
									SendDlgItemMessage(hWndDlg,DLGLBX,LB_SELITEMRANGE,TRUE,MAKELPARAM(i2+1,i-1));
									for(j = i2+1 ; j < i ; j++) was_sel[j] = TRUE;
									i2 = i;
								}
							}
						}
					}
					for(i = 0 ; i < n ; i++) was_sel[i] = sel[i];
				}
				break;

			default:
				return UNPROCESSED;
			}
			break;

		case DLGPBF:		/* set selection */
setsel:
			GetDlgItemText(hWndDlg,DLGEDIT,text,39);
			if(sscanf(text,"%f",&value) == 1) {
				n = 0;
				for(i = stsec[linnum] ; i <= ensec[linnum] ; i++) {
					if(SendDlgItemMessage(hWndDlg,DLGLBX,LB_GETSEL,n,0L) != 0) {
						weight_entry(text,i,value);
						SendDlgItemMessage(hWndDlg,DLGLBX,LB_DELETESTRING,n,0L);
						SendDlgItemMessage(hWndDlg,DLGLBX,LB_INSERTSTRING,n,(LPARAM) (LPCSTR) text);
					}
					n++;
				}
			}
			else {
				message("Invalid entry in edit box");
			}
			break;

		case DLGPBF+1:		/* Undo */
			goto reset;

		case IDOK:
			n = 0;
			for(i = stsec[linnum] ; i <= ensec[linnum] ; i++) {
				if(SendDlgItemMessage(hWndDlg,DLGLBX,LB_GETTEXT,n,(LPARAM) (LPCSTR) text) > 0) {
					p = strchr(text,':')+1;		/* locate value */
					sscanf(p,"%f",&linewt[linnum][i]);	/* read value */
				}
				n++;
			}
			memfree(was_sel);
			EndDialog(hWndDlg,TRUE);
			break;

		case IDCANCEL:
			memfree(was_sel);
			EndDialog(hWndDlg,0);
			break;

		case IDHELP:	/* Help */
			context(context_id);
			HelpUsed = 1;
			break;

		default:
			return UNPROCESSED;

		}	/* end switch (wParam) */
		return PROCESSED;

	default:
		return UNPROCESSED;
	}		/* end switch (msg) */
	return PROCESSED;
}

#endif

static char *weightspaces = "      ";

void weight_entry(char *text,int i,REAL wt)
{
	char *space;
	if(i < 9)
		space = weightspaces;
	else if(i == 9)
		space = weightspaces + 2;
	else
	    space = weightspaces + 4;
	sprintf(text,"%d-%d:%s%.4f",i,i+1,space,wt);
}

/*	define smoothed parameter		*/

#ifdef linux
int SmooExitDialog();
#endif

int SmooDlg(int *seldir,int *linesel)
{
	int		code;
	dlgresult = -1;	/* undefined */

#ifdef linux

	dlgresult = SmooExitDialog();

#else

	ch1 = DLGRBN1 + *seldir;
	ch2 = DLGRBN2 + *linesel;
	code = DialogBox(hInst,(char *) SMOOEXIT,hWnd,(DLGPROC) SmooDlgProc);
	if(code < 0) (void) MessageBox(hWnd,"SMOOEXIT",
		"Could not create dialog box",MB_OK);
#endif

	if(dlgresult) {
		*seldir  = ch1 - DLGRBN1;
		*linesel = ch2 - DLGRBN2;
	}
	return dlgresult;
}

#define PROCESSED TRUE
#define UNPROCESSED FALSE

#ifdef linux

int SmooLineProceed;

void SmooLineCB(Widget w,XtPointer client_data,XtPointer call_data)
{
	SmooLineProceed = (int) client_data;
}

int SmooExitDialog()
{
	void centre_dlg(HWND);
	char line1[80],line2[80];
	Widget Wshell,Wdialog,w;
	extern Widget mainWindow;
	int result;
	int pending;
	Arg argval[40];
	int	argnum;
	extern COLORREF scrcolour[];
	extern Window win;
	extern int xleft,xright,ybottom,ytop;
	extern int xmaxi,ymaxi;
	char *p;
	int i;
	extern XtAppContext app_context;

	/*	Create the dialog shell	and window	*/

	Wshell = XmCreateDialogShell(mainWindow,"Select Change",NULL,0);

	Wdialog = XtVaCreateManagedWidget(
		"selchange",
		xmBulletinBoardWidgetClass,
		Wshell,
		XmNdialogStyle,XmDIALOG_APPLICATION_MODAL,
		XmNautoUnmanage,False,
		XmNallowOverlap,False,
		XmNnoResize,True,
		XmNmarginWidth,0,
		XmNmarginHeight,0,
		XmNwidth,272,
		XmNheight,156,
		XmNdefaultPosition,False,
		XmNresizePolicy,XmRESIZE_NONE,
		XmNbackground,DialogBackground,
		XmNborderColor,ButtonBackground,
		XmNtopShadowColor,TopShadow,
		XmNbottomShadowColor,BottomShadow,
		NULL);
	centre_dlg(Wdialog);
	XtManageChild(Wdialog);

	w = LabelWidget(Wdialog,"Movement direction",False,2,6,112,30);
	w = LabelWidget(Wdialog,"Line to be moved",False,144,6,108,34);

	w = LabelWidget(Wdialog,"Vertical",False,10,44,98,20);
	w = LabelWidget(Wdialog,"Lateral",False,10,68,98,20);
	w = LabelWidget(Wdialog,funnum < 8 ? "Circular" : "Radial",False,10,92,98,20);
	if(funnum == 4 || funnum == 5 || funnum == 8 || funnum == 9) {	/* line interrelationships */
		if((funnum & 1) == 0) {
			sprintf(line1,"Previous line (%d)",linnum);
			sprintf(line2,"This line (%d)",linnum+1);
		}
		else {
			sprintf(line1,"This line (%d)",linnum+1);
			sprintf(line2,"Next line (%d)",linnum+2);
		}
	}
	else {		/* relationship of line to offset point */
		strcpy(line1,"Control point");
		strcpy(line2,"Line");
	}
	w = LabelWidget(Wdialog,line2,False,130,44,136,22);
	w = LabelWidget(Wdialog,line1,False,130,68,136,22);


	w = ButtonWidget(Wdialog,"Ok",     26,126,60,24);
	XtAddCallback(w,XmNactivateCallback,SmooLineCB,(XtPointer) IDOK);
	w = ButtonWidget(Wdialog,"Cancel",106,126,60,24);
	XtAddCallback(w,XmNactivateCallback,SmooLineCB,(XtPointer) IDCANCEL);
	w = ButtonWidget(Wdialog,"Help",  186,126,60,24);
	XtAddCallback(w,XmNactivateCallback,SmooLineCB,(XtPointer) IDHELP);

	SmooLineProceed = 0;
	while(SmooLineProceed == 0 | (pending = XtAppPending(app_context)) ) {
		if(pending)
			XtAppProcessEvent(app_context, XtIMAll);
		else
		    usleep(100000);
	}
	XtUnmanageChild(Wdialog);
}

#else

BOOL CALLBACK SmooDlgProc(HWND hWndDlg,unsigned msg,WORD wParam,LONG lParam)
{
	char line1[25],line2[25];
	BOOL colret;

	if(ColourMessage(msg,wParam,lParam,&colret)) return colret;

	switch(msg) {
	case WM_INITDIALOG:

		centre_dlg(hWndDlg);
		if(funnum == 4 || funnum == 5 || funnum == 8 || funnum == 9) {	/* line interrelationships */
			if((funnum & 1) == 0) {
				sprintf(line1,"&Previous line (%d)",linnum);
				sprintf(line2,"&This line (%d)",linnum+1);
			}
			else {
				sprintf(line1,"&This line (%d)",linnum+1);
				sprintf(line2,"&Next line (%d)",linnum+2);
			}
		}
		else {		/* relationship of line to offset point */
			strcpy(line1,"&Control point");
			strcpy(line2,"&Line");
		}
		SetDlgItemText(hWndDlg,DLGRBN2 +0,line1);
		SetDlgItemText(hWndDlg,DLGRBN2 +1,line2);
		if(funnum < 8) {	/* angles */
			SetDlgItemText(hWndDlg,DLGRBN1 +2,"Circular");
		}
		else {
			SetDlgItemText(hWndDlg,DLGRBN1 +2,"Radial");
		}
		CheckRadioButton(hWndDlg,DLGRBN1+0,DLGRBN1+2,ch1);
		CheckRadioButton(hWndDlg,DLGRBN2+0,DLGRBN2+1,ch2);
		return TRUE;

	case WM_LBUTTONDBLCLK:
		goto ok;

	case WM_COMMAND:
		switch(wParam) {
		case IDOK:
ok:
			EndDialog(hWndDlg,TRUE);
			dlgresult = 1;
			break;

		case IDCANCEL:
			EndDialog(hWndDlg,0);
			dlgresult = 0;
			break;

		case IDHELP:	/* Help */
			context(context_id);
			HelpUsed = 1;
			break;

		case DLGRBN1+0:
		case DLGRBN1+1:
		case DLGRBN1+2:
			CheckRadioButton(hWndDlg,DLGRBN1+0,DLGRBN1+2,wParam);
			ch1 = wParam;
			break;

		case DLGRBN2+0:
		case DLGRBN2+1:
			CheckRadioButton(hWndDlg,DLGRBN2+0,DLGRBN2+1,wParam);
			ch2 = wParam;
			break;

		default:
			return UNPROCESSED;
		}
		break;
	default:
		return UNPROCESSED;
	}
	return PROCESSED;
}

#endif

