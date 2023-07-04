/* Hullform component - graphgl.c
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
#include <GL/glx.h>
#else
#include <GL\gl.h>
#include <GL\glu.h>
#endif

#include <signal.h>

extern HDC hDC;
int GLfontBase = 0;
void GLcl(void);
void check_gl(char *context);
extern int viewlist,wireframe_list;
void GLnl(void);
extern REAL	yview,zview;
GLdouble zNear,zFar,fovy,aspect,dview;
REAL GLscale = 1.0;
REAL GLxshift = 0.0;
REAL GLyshift = 0.0;
HDC Startup_hDC = NULL;	// copy of hDC retained so it can be retrieved after temporary
						// use of GDI in dialog box windows

/****************************************************************/
/*																*/
/*	OpenGL graphics drivers										*/
/*																*/
/****************************************************************/

#ifdef linux

XFontStruct *fontstruct;
XmFontList fontlist;
Display	*display;
extern Widget mainWindow;

extern LOGFONT lf;

void MakeFont(char *fontname)
{
	unsigned int first,last;
	Font id;
	XFontStruct *fontInfo;
    extern Widget glw;
    extern int GLfontBase;
    static int listcount;
    int n;

	fontInfo = XLoadQueryFont(XtDisplay(glw),fontname);
	if (fontInfo == NULL) {
		fprintf(stderr, "no font found\n");
		exit(0);
	}
	id = fontInfo->fid;
	first = fontInfo->min_char_or_byte2;
	last = fontInfo->max_char_or_byte2;
	listcount = last - first + 1;
	if(GLfontBase > 0) glDeleteLists(GLfontBase,listcount);

	GLfontBase = glGenLists(listcount);
	if (GLfontBase == 0) {
		fprintf(stderr, "out of display lists\n");
		exit(0);
	}
	glXUseXFont(id, first, listcount, GLfontBase+first);
}

#else

HGLRC hRC = 0;
PIXELFORMATDESCRIPTOR pfd = {
	sizeof(PIXELFORMATDESCRIPTOR),   // size of this pfd
	1,                     // version number
	PFD_DRAW_TO_WINDOW |   // support window
	PFD_SUPPORT_OPENGL |   // support OpenGL
	PFD_DOUBLEBUFFER,      // double buffered
	PFD_TYPE_RGBA,         // RGBA type
	24,                    // 24-bit color depth
	0, 0, 0, 0, 0, 0,      // color bits ignored
	0,                     // no alpha buffer
	0,                     // shift bit ignored
	0,                     // no accumulation buffer
	0, 0, 0, 0,            // accum bits ignored
	32,                    // 32-bit z-buffer
	0,                     // no stencil buffer
	0,                     // no auxiliary buffer
	PFD_MAIN_PLANE,        // main layer
	0,                     // reserved
	0, 0, 0                // layer masks ignored
};

#endif

extern COLORREF scrcolour[];
int pixelformat = 0;

void GLin(void)
{
	extern LOGFONT lf;		/* logical font structure */
	void setfont(LOGFONT *lf);
#ifndef linux
	RECT rc;
	if(hDC == NULL) {
		hDC = GetDC(hWnd);
	}
#endif
	setup(0);

#ifndef linux
	if(hRC == NULL) {
		if(pixelformat <= 0) {
			pixelformat = GetPixelFormat(hDC);
			if(pixelformat <= 0) pixelformat = ChoosePixelFormat(hDC,&pfd);
		}
		if(pixelformat == 0) {
			message("No pixel format - exit");
			PostQuitMessage(0);
			return;
		}
		if(SetPixelFormat(hDC, pixelformat,&pfd) == FALSE) {
			message("Can't set pixel format - exit");
			PostQuitMessage(0);
			return;
		}
		hRC = wglCreateContext(hDC);
		if(hRC == NULL) {
			message("wglCreateContext failed");
			PostQuitMessage(0);
			return;
		}
		if(!wglMakeCurrent(hDC, hRC)) {
			message("Can't make current - exit");
			PostQuitMessage(0);
			return;
		}
	}
#endif

	if(viewlist == 0) {
		viewlist = glGenLists(258);
		wireframe_list = viewlist + 1;
		setfont(&lf);
#ifdef linux
        MakeFont(lf.lfFaceName);
#else
		GLfontBase = wireframe_list + 1;
		wglUseFontBitmaps(hDC,0,256,GLfontBase);
#endif
		glListBase(GLfontBase);
	}

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glEnable(GL_LINE_SMOOTH);

	setfont(&lf);

#ifndef linux
	GetClientRect(hWnd,&rc);
	glViewport((GLint) rc.left,(GLint) rc.top,(GLint) rc.right - rc.left,(GLint) rc.bottom - rc.top);
#endif
}

void GLnl(void)
{
	extern int penup;
	extern REAL xcurr;
	if(!penup) {
		glEnd();
		penup = TRUE;
	}
	xcurr = -99999.0;
}

//	hRC is maintained throughout the Hullform session - do not use
//	wglMakeCurrent(NULL,NULL) here. (This is done, along with deleting
//	the display lists, at program close)

void GLen(void)
{
#ifdef linux
	extern Widget glw;
#endif

	GLnl();	// end any line
#ifdef linux
	if(glw != 0) glXSwapBuffers(XtDisplay(glw),XtWindow(glw));
#else
	if(hDC != NULL) {
		SwapBuffers(hDC);
		wglDeleteContext(hRC);
		ReleaseDC(hWnd,hDC);
		hDC = NULL;
		hRC = NULL;
	}
#endif
}

void GLcr(int ncol)
{
	extern COLORREF scrcolour[];
	DWORD col;
	extern int linestyle[5];
	extern int linewidth[5];
	extern int penup;
	float r,g,b;

#ifdef PROF
	extern int current_hull;
	if(current_hull != 0) return;	/* one colour for overlay */
#endif

	if(!penup) {
		glEnd();
		penup = TRUE;
	}

	col = scrcolour[ncol];
	r = ((float) GetRValue(col)) / 255.0;
	g = ((float) GetGValue(col)) / 255.0;
	b = ((float) GetBValue(col)) / 255.0;
	glColor3f(r,g,b);

	//	Don't set a width for window background (ncol = 0) or text (ncol = 1)

	if(ncol >= 2) glLineWidth((GLfloat) max(1,linewidth[ncol-2]));
}

void GLcl(void)
{
	extern int repaint;
	extern COLORREF scrcolour[];
	COLORREF col = scrcolour[0];
	float r,g,b;
	extern int gl_xor;

	r = GetRValue(col);
	g = GetGValue(col);
	b = GetBValue(col);
	glClearColor(r / 255.0f,g / 255.0f,b / 255.0f,1.0);
	glClearDepth( 1.0 );
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	repaint = FALSE;
}

void GLdr(REAL x, REAL y)
{
	extern REAL	xcurr,ycurr,zcurr;
	extern int	penup;
	extern int	persp;
	extern REAL	zpersp;

	if(x >= -30000.0 && x < 30000.0 && y >= -30000.0 && y <= 30000.0) {
		if(penup) {
			glBegin(GL_LINE_STRIP);
			if(xcurr > -30000.0) {
				if(persp)
					glVertex3f(-xcurr,-ycurr,zcurr);
				else
				    glVertex2f(xcurr,ycurr);
			}
			penup = FALSE;
		}
		if(persp)
			glVertex3f(-x,-y,zpersp);
		else
		    glVertex2f(x,y);
	}
	else {
		GLnl();
	}
	xcurr = x;
	ycurr = y;
	zcurr = zpersp;
}

void GLmv(REAL x,REAL y)
{
	extern REAL	xcurr,ycurr,zcurr;
	extern REAL	zpersp;
	extern int	penup;

	GLnl();
	xcurr = x;
	ycurr = y;
	zcurr = zpersp;
}

void GLst(char *s)
{
	extern REAL	xcurr,ycurr,zcurr;
	extern int persp;
	extern int penup;
	extern COLORREF scrcolour[];
	GLfloat prevcol[4];
	DWORD col;
	float r,g,b;

	if(!penup) {
		glEnd();
		penup = TRUE;
	}
	glGetFloatv(GL_CURRENT_COLOR,prevcol);

	col = scrcolour[1];						// text colour
	r = ((float) GetRValue(col)) / 255.0;
	g = ((float) GetGValue(col)) / 255.0;
	b = ((float) GetBValue(col)) / 255.0;
	glColor3f(r,g,b);
	if(persp)
		glRasterPos3f(-xcurr,-ycurr,zcurr);
	else
	    glRasterPos2f(xcurr,ycurr);
	glPushMatrix();
	glListBase(GLfontBase);
	glCallLists(strlen(s), GL_UNSIGNED_BYTE,(GLubyte *) s);
	glPopMatrix();

	glColor3fv(prevcol);
}

void check_gl(char *context)
{
	int n;
	extern int penup;

	if(!penup) {
		glEnd();
		penup = TRUE;
	}
	while((n = glGetError()) != GL_NO_ERROR) {

		(void) MessageBox(hWndMain,(LPCTSTR) gluErrorString(n),context,MB_OK);

	}
}

//	Set up for 3-D perspective view

void set_GL_perspective()
{
	static REAL xv,yv,zv,tany,tanz;
	extern REAL axx,axy,axz,ayx,ayy,ayz,azx,azy,azz;
	extern REAL heelv;
	REAL xt,yt,zt,x,y,z;
	static REAL aymin,aymax,azmin,azmax,dxmax,dxmin,zmin,zmax,ys;
	static REAL avex,avey,avez;
	int i,j,k;
	extern int scaled;
	REAL fovx;
	extern REAL sinpp,cospp;

	if(scrdev == 1) return;	// GDI - skip

	if(! scaled) {

		/*	Find extremes of y and z for full length of the hull	*/
		dxmin = aymin = azmin = zNear = zmin = 1.0e+16;
		dxmax = aymax = azmax = zFar = zmax = -zmin;
		ys = 0.0;
		for(j = 0 ; j < numlin ; j++) {
			for(i = max(1,stsec[j]) ; i <= ensec[j] ; i++) {
				if(ys < yline[j][i]) ys = yline[j][i];
				zt = zline[j][i];
				if(zmin > zt) zmin = zt;
				if(zmax < zt) zmax = zt;
			}
		}

		/*	Define transformation matrix	*/

		perspp(0.0,0.0,0.0,pitch,rotn,heelv);
		dview = sqrt(yview*yview + zview*zview);
		x = xsect[0] - dxstem();
		avex = avey = avez = 0.0;
		for(i = 0 ; i < 2 ; i++)  {
			y = -ys;
			for(j = 0 ; j < 2 ; j++) {
				z = zmin;
				for(k = 0 ; k < 2 ; k++) {

					/*	Transform to scene coordinates		*/

					yt = axx * y + axy * z + axz * x;
					zt = ayx * y + ayy * z + ayz * x;
					xt = azx * y + azy * z + azz * x;

					avex += xt;
					avey += yt;
					avez += zt;

					/*	Transform to viewing coordinates	*/

					xv = dview + cospp*xt - sinpp*zt;
					yv = -yt;
					zv = cospp*zt + sinpp*xt;

					if(xv > zFar) zFar = xv;
					if(xv < zNear) zNear = xv;

					/*	Evaluate angular position, and update limits if they are exceeded */

					tany = yv / xv;
					tanz = zv / xv;

					if(tany < aymin) aymin = tany;
					if(tany > aymax) aymax = tany;
					if(tanz < azmin) azmin = tanz;
					if(tanz > azmax) azmax = tanz;
					if(xv < dxmin) dxmin = xv;
					if(xv > dxmax) dxmax = xv;

					z = zmax;
				}
				y = ys;
			}
			x = xsect[count-1];
		}
		avex /= 8.0;
		avey /= 8.0;
		avez /= 8.0;

		x = (aymax - aymin)*0.1;
		y = (azmax - azmin)*0.1;
		aymax += x;
		aymin -= x;
		azmax += y;
		azmin -= y;

		if(zNear < 0.0) zNear = 0.01 * zFar;
		if(ymax > ymin)
			aspect = (xmax - xmin)/(ymax - ymin);
		else
		    aspect = 1.0;

		xt = (aymax - aymin) - (azmax - azmin)*aspect;
		if(xt > 0.0) {
			xt *= 0.5/aspect;
			azmax += xt;
			azmin -= xt;
		}
		else {
			xt *= 0.5;
			aymax -= xt;
			aymin += xt;
		}

		xmax = aymax * dview;
		xmin = aymin * dview;
		ymax = azmax * dview;
		ymin = azmin * dview;

		fovy = atan(azmax - azmin);
		zNear *= 0.5;
		zFar *= 2.0;
		dview = 0.5*(dxmax + dxmin);

	}
	glEnd();
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	fovy *= GLscale;
	GLscale = 1.0;
	gluPerspective(57.2958*fovy,aspect,zNear,zFar);

	gluLookAt(	0.0,					 yview,					  zview,
				0.5*(xmin+xmax),0.5*(ymin+ymax),0.0,
				0.0,					 zview/dview,			  -yview/dview);

	//	Orient the hull. The transformations are applied to the view in the
	//	reverse order to which they appear in the code - i.e., heel first.

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glRotatef((GLfloat) -rotn ,(GLfloat) 0.0,(GLfloat) 1.0,(GLfloat) 0.0);
	glRotatef((GLfloat) -pitch,(GLfloat) 1.0,(GLfloat) 0.0,(GLfloat) 0.0);
	glRotatef((GLfloat) -heel ,(GLfloat) 0.0,(GLfloat) 0.0,(GLfloat) 1.0);

}

void GLzoom(REAL xmn,REAL xmx,REAL ymn,REAL ymx)
{
	REAL fx,fy;
	extern int scaled,persp;

	glEnd();
	fx = (xmx - xmn) / (xmax - xmin);
	fy = (ymx - ymn) / (ymax - ymin);
	if(ymax != ymin)
		aspect = (xmax - xmin)/(ymax - ymin);
	else
	    aspect = 1.0;
	if(fx > fy * aspect) fy = fx / aspect;
	GLscale = fy;
	GLxshift = 0.5*(xmx + xmn - xmax - xmin);
	GLyshift = 0.5*(ymx + ymn - ymax - ymin);

	if(persp) {
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluPerspective(57.2958*GLscale*fovy,aspect,zNear,zFar);
		gluLookAt(	0.0,					 yview,					  zview,
					0.5*(xmin+xmax)+GLxshift,0.5*(ymin+ymax)+GLyshift,0.0,
					0.0,					 zview/dview,			  -yview/dview);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glRotatef((GLfloat) -rotn ,(GLfloat) 0.0,(GLfloat) 1.0,(GLfloat) 0.0);
		glRotatef((GLfloat) -pitch,(GLfloat) 1.0,(GLfloat) 0.0,(GLfloat) 0.0);
		glRotatef((GLfloat) -heel ,(GLfloat) 0.0,(GLfloat) 0.0,(GLfloat) 1.0);
		scaled = TRUE;
	}
	else {
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho((GLdouble) xmn,(GLdouble) xmx,(GLdouble) ymn,(GLdouble) ymx,(GLdouble) -10000.0,(GLdouble) 10000.0);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
	}
	xmin = xmn;
	xmax = xmx;
	ymin = ymn;
	ymax = ymx;
}

#ifndef linux

void free_context(void)
{
	if(scrdev == 0 && hRC != NULL) {
		if(!wglMakeCurrent(NULL,NULL)) {
			message("wglMakeCurrent");
		}
		if(!wglDeleteContext(hRC)) message("wglDeleteContext");
		hRC = NULL;
		if(viewlist > 0) {
			glDeleteLists(viewlist,258);
			viewlist = 0;
			GLfontBase = 0;
		}
	}
	GDIen();	// releases fonts and device context, sets hDC = 0
}

#endif


