/* Hullform component - dxf_input.c
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

#ifdef PROF


/*    AutoCAD "DXF"-format input of hull data	    */

#ifdef linux
#undef tc
#include <GL/gl.h>
#include <GL/glu.h>
#else
#include <GL\gl.h>
#include <GL\glu.h>
#endif

#ifdef linux
char *searchpath(char *file);
#include <Xm/Text.h>
#endif

int ncount;				// new section count when editing the list of section positions

//	Line types accepted

#define POLYLINE	1
#define	LWPOLYLINE	2
#define MESH		3

void save_hull(int);
REAL min_diff;
REAL xmn,xmx,ymn,ymx,zmn,zmx;

REAL chine_angle = 30.0;
int value_tag = 0;
int invert_verticals = TRUE;
int reverse_offsets = FALSE;
REAL yzero = 0.0;
int yzero_option = 2;
int x_stem_high = FALSE;
int points_between_chines = 3;
int edit_positions = FALSE;
extern char dxf_filename[MAX_PATH];
extern int editinit;
int read_dxf_file(REAL xl_dxf[maxlin][maxsec],REAL yl_dxf[maxlin][maxsec],REAL zl_dxf[maxlin][maxsec],int *nl,int numpt[maxlin]);
void draw_dxf_view();
void set_GL_perspective();
void SwapBuf();

REAL (*xl_dxf)[maxsec] = NULL;
REAL (*yl_dxf)[maxsec] = NULL;
REAL (*zl_dxf)[maxsec] = NULL;
int numpt_dxf[maxlin];
int nl_dxf = 0;
int show_dxf = TRUE;

int join(int j,int i1,int k,int i2)
{
	REAL x = xl_dxf[j][i1]-xl_dxf[k][i2];
	REAL y = yl_dxf[j][i1]-yl_dxf[k][i2];
	REAL z = zl_dxf[j][i1]-zl_dxf[k][i2];
	return (x*x + y*y + z*z < min_diff*min_diff);
}


//	Return fractional distance of point (y,z) along a line (y1,z1) to (y2,z2), "a", and square of distance from the line, "b"

void get_distances(REAL y1,REAL z1,REAL y2,REAL z2,REAL y,REAL z,REAL *a,REAL *b)
{
	REAL dy12,dz12,dy,dz,l,lsq,dsq,dotp;
	dy = y - y1;
	dz = z - z1;
	dsq = dy*dy + dz*dz;
	dy12 = y2 - y1;
	dz12 = z2 - z1;
	lsq = dy12*dy12 + dz12 * dz12;
	if(lsq > 0.0) {
		dotp = dy*dy12 + dz*dz12;
		*a = l = dotp / lsq;
		*b = dsq - dotp*l;
	} else {
		*a = 0.0;
		*b = dsq;
	}
	if(*b > 0.0) *b = sqrt(*b);
}

//	DXF dialog box pushbutton function

int dxf_yes = 0;

int yes(char *mes)
{
	return (MessageBox(hWndMain,mes,"HULLFORM QUERY",MB_YESNO) == IDYES);
}

void inp_dxf(int code,HWND hWndDlg)
{
	char path[MAX_PATH] = "";
	FILE *fp;
	REAL xmin,xmax,ymin,ymax,zmin,zmax;
	int num_polyline,num_mesh,num_lwpolyline;
	char text[200];
	char *p;

	switch(code) {
	case 0:
		if(openfile(path,"rt","Specify DXF Input File","DXF Files (*.dxf)\0*.dxf\0All files\0*.*\0","*.dxf",dxfdirnam,NULL)) {
#ifdef linux
			XmTextSetString(wEdit[3],path);
#else
			SetDlgItemText(hWndDlg,DLGEDIT+3,path);
#endif
		}
		break;
	case 1:
		if(openfile(path,"rt","Specify DXF Input File","DXF Files (*.dxf)\0*.dxf\0All files\0*.*\0","*.dxf",dxfdirnam,&fp)) {
#ifdef linux
			XmTextSetString(wEdit[3],path);
#else
			SetDlgItemText(hWndDlg,DLGEDIT+3,path);
#endif
			xmin = 1.0e+30;
			xmax = -1.0e+30;
			ymin = 1.0e+30;
			ymax = -1.0e+30;
			zmin = 1.0e+30;
			zmax = -1.0e+30;
			num_polyline = 0;
			num_mesh = 0;
			num_lwpolyline = 0;
			while(fgets(text,sizeof(text),fp) != NULL) {
				p = text;
				while(*p) *p++ = toupper(*p);
			}
		}
		break;
	}

}

void dxfinputfunc(int opt,HWND hWndDlg)
{
#ifdef linux
	XmString *xstrtab,xstr;
	extern Widget wListBox[],wEdit[];
	char *text;
#endif
	char sel[40],valtext[40];
	REAL ref,val;
	int i,j,n;

	switch(opt) {
	case 0:		// delete selection
#ifdef linux
		XtVaGetValues(wListBox[0],
			XmNselectedItemCount,&n,
			XmNselectedItems,&xstrtab,NULL);
		while(n-- > 0) {
			xstr = XmStringCopy(*xstrtab++);
			XmListDeleteItem(wListBox[0],xstr);
			XmStringFree(xstr);
			for(i = n+1 ; i < count ; i++) xsect[i-1] = xsect[i];	// may require changing
		}
#else
		n = SendDlgItemMessage(hWndDlg,DLGLBX+0,LB_GETCURSEL,0,0);
		if(n != LB_ERR) SendDlgItemMessage(hWndDlg,DLGLBX+0,LB_DELETESTRING,n,0);
		for(i = n+1 ; i < ncount ; i++) xsect[i-1] = xsect[i];
		ncount--;
#endif
		break;
	case 1:		// add entry
#ifdef linux
		text = XmTextGetString(wEdit[0]);
		xstr = XmStringCreateSimple(text);
		XmListAddItem(wListBox[0],xstr,0);
		XmStringFree(xstr);
		XtFree(text);
#else
		SendDlgItemMessage(hWndDlg,DLGEDIT+0,WM_GETTEXT,40,(LONG)(LPSTR) sel);
		if(sscanf(sel,"%f",&ref) == 1) {
			sprintf(sel,"%.3f",ref);
			n = SendDlgItemMessage(hWndDlg,DLGLBX+0,LB_GETCOUNT,0,0l);
			for(i = 0 ; i < n ; i++) {
				SendDlgItemMessage(hWndDlg,DLGLBX+0,LB_GETTEXT,i,(LPARAM) (LPCSTR) valtext);
				if(sscanf(valtext,"%f",&val) == 1) {
					if(val > ref) {	// add before this entry
						SendDlgItemMessage(hWndDlg,DLGLBX+0,LB_INSERTSTRING,i,(LPARAM) (LPCSTR) sel);
						for(j = ncount ; j > i ; j--) xsect[j] = xsect[j-1];
						xsect[i] = ref;
						ncount++;
						return;
					}
				}
			}
			SendDlgItemMessage(hWndDlg,DLGLBX+0,LB_ADDSTRING,n,(LPARAM) (LPCSTR) sel);
		}
#endif
		break;
	}

}

MENUFUNC dxf_input()
{
	REAL xl[maxlin][maxsec];
	REAL yl[maxlin][maxsec];
	REAL zl[maxlin][maxsec];
	int chine_point[maxsec];
	int line_index[maxlin][maxsec];
	int numpt[maxlin],maxpt;
	int il,nl;
	REAL xlo[maxlin],xhi[maxlin];
	REAL xs[maxsec];
	int jsect[maxlin];
	int i,j,k,m,n,ii;
	REAL a,b,c,d,l,sa,sb,theta,x;
	REAL maxoff,minoff;
	REAL aveoff,numoff,sumoff,numoffall,ysum,ysummax,ysummin,ysumj,ysumn,sumy[maxlin];
	REAL y1,y2,z1,z2,ny,nz,yr,zr,dy1,dz1,dy2,dz2,ym,zm,lsq;
	REAL x_stem,x_stern;
	int offsets_available[maxsec];	// TRUE for any section read from file, rather than an interpolated one
	char text[500];
	static int allow_curvature = 0;
	struct {
		int index;       	/* index of result in table */
		char *string;    	/* pointer to result */
		char *table[maxsec]; /* table of strings for listbox, null string terminator */
	}
	poslist;
	char sectpos[maxsec][20];
	REAL position_sum[maxlin];
	extern int numbetw;
	REAL offset_sign;
	int M;
	int reverse;
	int nxmn,nxmx,nymn,nymx,nzmn,nzmx;
	extern int scaled;

	if(xl_dxf == NULL) {
		if(!memavail((void *) &xl_dxf,sizeof(REAL) * maxlin * maxsec * 3)) {
			message("Free memory low - try restarting your operating system");
			xl_dxf = NULL;
			return;
		}
		yl_dxf = (REAL (*)[maxsec]) &xl_dxf[maxlin][0];
		zl_dxf = (REAL (*)[maxsec]) &yl_dxf[maxlin][0];
	}

	numbetw = 0;	// don't plot point between sections - result is likely to be non-fair
	cls(FALSE);
	update_func = NULL;

//	Open the required DXF input file

	chdir(dxfdirnam);

retry:
	if(!getdlg(DXFINPUT,
		INP_LOG,&x_stem_high,
		INP_LOG,&reverse_offsets,
		INP_LOG,&invert_verticals,
		INP_RBN,&yzero_option,
		INP_REA,&yzero,
		INP_RBN,&value_tag,

		INP_LOG,&allow_curvature,
		INP_LOG,&edit_positions,
		INP_INT,&points_between_chines,
		INP_REA,&chine_angle,
		INP_PBF,inp_dxf,
		INP_STR,(void *) &dxf_filename[0],
		INP_LOG,&show_dxf,
		-1)) return;

//	Try to read the specified DXF file

//	The "_dxf" variables are those exactly as read

	if(!read_dxf_file(xl_dxf,yl_dxf,zl_dxf,&nl,numpt)) return;

	nl_dxf = nl;
	for(j = 0 ; j < nl ; j++) numpt_dxf[j] = numpt[j];

#ifdef PROF
	ntank = 0;
#endif
	if(nl <= 0) {
		message("No supported line types were seen in the input DXF file.");
		return;
	}
	offset_sign = reverse_offsets ? -1.0 : 1.0;

//	Try to make sense of the dimensions

	xmx = ymx = zmx = -1.0e+30;
	xmn = ymn = zmn = 1.0e+30;
	for(j = 0 ; j < nl ; j++) {
		for(i = 0 ; i < numpt[j] ; i++) {
			if(xmx < xl_dxf[j][i]) xmx = xl_dxf[j][i];
			if(xmn > xl_dxf[j][i]) xmn = xl_dxf[j][i];
			if(ymx < yl_dxf[j][i]) ymx = yl_dxf[j][i];
			if(ymn > yl_dxf[j][i]) ymn = yl_dxf[j][i];
			if(zmx < zl_dxf[j][i]) zmx = zl_dxf[j][i];
			if(zmn > zl_dxf[j][i]) zmn = zl_dxf[j][i];
		}
	}
	a = xmx - xmn;
	b = ymx - ymn;
	c = zmx - zmn;

//	Check longest dimension

	if(b > a && b > c && value_tag != 1 && value_tag != 4)
		if(yes("Your second (2x) dimension is longest - it looks like your co-ordinate choice should have been Y,X,Z or Z,X,Y\n\nDo you want to change your choice?")) goto retry;

	if(c > a && c > b && value_tag != 2 && value_tag != 5)
		if(yes("Your third (3x) dimension is longest - it looks like your co-ordinate choice should have been Z,Y,X or Y,Z,X\n\nDo you want to change your choice?")) goto retry;

//	If the hull symmetrical about some zero?

	min_diff = 0.01 * fabs(a);
	if(fabs(zmx + zmn) < min_diff)
		if(yes("The range of the third (3x) dimension is symmetrical about zero, suggesting this is actually the lateral offset of a symmetrical design\n\nDo you want to change your choice to X,Z,Y or Z,X,Y?")) goto retry;
	else if(fabs(xmx + xmn) < min_diff)
		if(yes("The range of the first (1x) dimension is symmetrical about zero, suggesting this is actually the lateral offset of a symmetrical design\n\nDo you want to change your choice to Y,X,Z or Y,Z,X?")) goto retry;
	else if(fabs(ymx + ymn) < min_diff && !(yzero_option == 2 && yzero != 0.0) )
		if(yes("The range of the second (2x) dimension is symmetrical about zero, supporting your choice that this is the lateral offset of a symmetrical design, but you have not set the zero plane at 0.0.\n\nDo you want to change your choice to Y,X,Z or Y,Z,X?")) goto retry;

//	The hull centreplane may be marked by a lot of line starts or ends

	nxmn = nxmx = nymn = nymx = nzmn = nzmx = 0;
	for(j = 0 ; j < nl ; j++) {
		i = numpt[j]-1;
		if(fabs(xl_dxf[j][0] - xmn) < min_diff && fabs(xl_dxf[j][i] - xmn) < min_diff) nxmn++;
		if(fabs(xl_dxf[j][0] - xmx) < min_diff && fabs(xl_dxf[j][i] - xmx) < min_diff) nxmx++;
		if(fabs(yl_dxf[j][0] - ymn) < min_diff && fabs(yl_dxf[j][i] - ymn) < min_diff) nymn++;
		if(fabs(yl_dxf[j][0] - ymx) < min_diff && fabs(yl_dxf[j][i] - ymx) < min_diff) nymx++;
		if(fabs(zl_dxf[j][0] - zmn) < min_diff && fabs(zl_dxf[j][i] - zmn) < min_diff) nzmn++;
		if(fabs(zl_dxf[j][0] - zmx) < min_diff && fabs(zl_dxf[j][i] - zmx) < min_diff) nzmx++;
	}

/*	Value_tag =	0	X Y Z
				1	X Z Y
				2	Y Z X
				3	Y X Z
				4	Z X Y
				5	Z Y X
*/
	if(nxmn > 0 && value_tag != 2 && value_tag != 3 &&
	yes("Some lines start and end at the minimum '1x' coordinate value, suggesting this may mark the hull centreplane.\n\nDo you want to change your choice to Y,X,Z or Y,Z,X, with Y coordinate at zero plane set to 'min'?")) {
		yzero_option = 0;
		reverse_offsets = FALSE;
		goto retry;
	}
	if(nxmx > 0 && value_tag != 2 && value_tag != 3 &&
	yes("Some lines start and end at the maximum '1x' coordinate value, suggesting this may mark the hull centreplane.\n\nDo you want to change your choice to Y,X,Z or Y,Z,X, with Y coordinate at zero plane set to 'max'?")) {
		yzero_option = 1;
		reverse_offsets = TRUE;
		goto retry;
	}
	if(nymn > 0) {
		if(value_tag != 0 && value_tag != 5 && yzero_option != 0 &&
		yes("Some lines start and end at the minimum '2x' coordinate value, suggesting this may mark the hull centreplane.\n\nDo you want to change your choice to Y,X,Z or Y,Z,X, with Y coordinate at zero plane set to 'min'?")) {
			yzero_option = 0;
			reverse_offsets = FALSE;
			goto retry;
		}
		else if(yzero_option != 0 &&
		yes("Some lines start and end at the minimum '2x' coordinate value, suggesting this may mark the hull centreplane.\n\nDo you want to change your Y coordinate at zero plane setting to 'min'?")) {
			yzero_option = 0;
			reverse_offsets = FALSE;
			goto retry;
		}
	}
	if(nymx > 0 && yzero_option != 1) {
		if(value_tag != 0 && value_tag != 5 &&
		yes("Some lines start and end at the maximum '2x' coordinate value, suggesting this may mark the hull centreplane.\n\nDo you want to change your choice to Y,X,Z or Y,Z,X, with Y coordinate at zero plane set to 'max'?")) {
			yzero_option = 1;
			reverse_offsets = TRUE;
			goto retry;
		}
		else if(
		yes("Some lines start and end at the maximum '2x' coordinate value, suggesting this may mark the hull centreplane.\n\nDo you want to change your Y coordinate at zero plane setting to 'max'?")) {
			yzero_option = 1;
			reverse_offsets = TRUE;
			goto retry;
		}
	}
	if(nzmn > 0) {
		if(value_tag != 1 && value_tag != 2 &&
		yes("Some lines start and end at the minimum '3x' coordinate value, suggesting this may mark the hull centreplane.\n\nDo you want to change your choice to Y,Z,X or X,Z,Y, with Y coordinate at zero plane set to 'min'?")) {
			yzero_option = 0;
			reverse_offsets = FALSE;
			goto retry;
		}
		else if(yzero_option != 0 &&
		yes("Some lines start and end at the minimum '3x' coordinate value, suggesting this may mark the hull centreplane.\n\nDo you want to change your Y coordinate at zero plane setting to 'min'?")) {
			yzero_option = 0;
			reverse_offsets = FALSE;
			goto retry;
		}
	}
	if(nzmx > 0) {
		if(value_tag != 1 && value_tag != 2 &&
		yes("Some lines start and end at the maximum '3x' coordinate value, suggesting this may mark the hull centreplane.\n\nDo you want to change your choice to Y,Z,X or X,Z,Y, with Y coordinate at zero plane set to 'max'?")) {
			yzero_option = 1;
			reverse_offsets = TRUE;
			goto retry;
		}
		else if(yzero_option != 1 &&
		yes("Some lines start and end at the maximum '3x' coordinate value, suggesting this may mark the hull centreplane.\n\nDo you want to change your Y coordinate at zero plane setting to 'max'?")) {
			yzero_option = 1;
			reverse_offsets = TRUE;
			goto retry;
		}
	}

//	Switch coordinates to match user specification

	maxoff = -1.0e+30;
	minoff = 1.0e+30;
	for(j = 0 ; j < nl ; j++) {
		for(i = 0 ; i < numpt[j] ; i++) {
			a = xl_dxf[j][i];
			b = yl_dxf[j][i];
			c = zl_dxf[j][i];
			switch (value_tag) {
			case 0:
				xl[j][i] = a;
				yl[j][i] = b;
				zl[j][i] = c;
				break;
			case 1:
				xl[j][i] = a;
				yl[j][i] = c;
				zl[j][i] = b;
				break;
			case 2:
				xl[j][i] = c;
				yl[j][i] = a;
				zl[j][i] = b;
				break;
			case 3:
				xl[j][i] = b;
				yl[j][i] = a;
				zl[j][i] = c;
				break;
			case 4:
				xl[j][i] = b;
				yl[j][i] = c;
				zl[j][i] = a;
				break;
			case 5:
				xl[j][i] = c;
				yl[j][i] = b;
				zl[j][i] = a;
				break;
			}
			if(yl[j][i] > maxoff) maxoff = yl[j][i];
			if(yl[j][i] < minoff) minoff = yl[j][i];
			xl_dxf[j][i] = xl[j][i];
			yl_dxf[j][i] = yl[j][i];
			zl_dxf[j][i] = zl[j][i];
		}
	}

//	Get numeric value for yzero

	if(yzero_option == 0) {
		yzero = minoff;
	} else if(yzero_option == 1) {
		yzero = maxoff;
	}	// else use the value read from the edit box

	for(i = 0 ; i < nl ; ) {
		for(j = 0 ; j < numpt[i] ; ) {
			if(yl[i][j] != -1.0) {

//	Transform hullform lateral coordinates (zero at centreplane, offsets positive)
				yl[i][j] = offset_sign*(yl[i][j] - yzero);

//	If the resulting offset is negative, remove it from the line

				if(yl[i][j] < 0.0) {
					for(k = j+1 ; k < numpt[i] ; k++) {
						xl[i][k-1] = xl[i][k];
						yl[i][k-1] = yl[i][k];
						zl[i][k-1] = zl[i][k];
					}
					numpt[i] --;
				} else {
					j++;
				}
			} else {
				j++;
			}
		}

//	Remove any line which was entirely negative from the table

		if(numpt[i] <= 0) {
			for(j = i+1 ; j < nl ; j++) {
				numpt[j-1] = numpt[j];
				for(k = 0 ; k < numpt[j] ; j++) {
					xl[j-1][k] = xl[j][k];
					yl[j-1][k] = yl[j][k];
					zl[j-1][k] = zl[j][k];
				}
			}
			nl--;
		} else {
			i++;
		}
	}

	for(j = 0 ; j < nl_dxf ; j++) {
		for(i = 0 ; i < numpt_dxf[j] ; i++) yl_dxf[j][i] = offset_sign*(yl_dxf[j][i] - yzero);
	}

//	Now remove any zero-length lines - that is, any lines deleted in the above process

	for(j = nl-1 ; j >= 0 ; j--) {
		if(numpt[j] == 0) {
			for(k = j+1 ; k < nl ; k++) {
				numpt[k-1] = numpt[k];
				for(i = 0 ; i < numpt[k] ; i++) {
					xl[k-1][i] = xl[k][i];
					yl[k-1][i] = yl[k][i];
					zl[k-1][i] = zl[k][i];
				}
			}
			nl--;
		}
	}

//	Find extent of each line, and maximum and minimum lateral offsets

	x_stem = 1.0e+30;
	x_stern = -1.0e+30;
	for(j = 0 ; j < nl ; j++) {
		xlo[j] = 1.0e+30;		// initialise x-range of this line
		xhi[j] = -1.0e+30;
		for(i = 0 ; i < numpt[j] ; i++) {
			x = xl[j][i];
			if(x < xlo[j]) xlo[j] = x;
			if(x > xhi[j]) xhi[j] = x;
		}

		if(xhi[j] > x_stern) x_stern = xhi[j];
		if(xlo[j] < x_stem) x_stem = xlo[j];
	}

	if(invert_verticals) {	// hull was upside-down in its coordinate system
		for(j = 0 ; j < nl ; j++) {
			for(i = 0 ; i < numpt[j] ; i++) {
				zl[j][i] = -zl[j][i];
			}
		}
	}

//	Identify sections as those lines extending over less than 1% or the hull length

	min_diff = 0.01 * fabs(x_stern - x_stem);	// minimum difference in x-positions for a section

	//	Reverse x-values if requested

	if(x_stem_high) {
		for(j = 0 ; j < nl ; j++) {
			for(i = 0 ; i < numpt[j] ; i++) {
				if(yl[j][i] > maxoff) maxoff = yl[j][i];
				if(yl[j][i] < minoff) minoff = yl[j][i];
				xl[j][i] = x_stern - xl[j][i];
			}
			x = x_stern - xhi[j];
			xhi[j] = x_stern - xlo[j];
			xlo[j] = x;
		}
		x_stern = x_stern - x_stem;
		x_stem = 0.0;
		for(j = 0 ; j < nl_dxf ; j++) {
			for(i = 0 ; i < numpt_dxf[j] ; i++) xl_dxf[j][i] = x_stern - xl_dxf[j][i];
		}
	}

	//	Remove "other side" lines. We do this by finding the average lateral offset over
	//	all lines. We then test all pairs of lines, and for any pair where the average of its
	//	individual offsets is constant and equals the average, we remove the line with the
	//	lesser offsets.

	//	In cases where any line is removed, we subtract the original average offset to place
	//	the presumed centreline on y=0 plane

	//	Calculate average offset of all lines. This is used to identify the hull centreline

	aveoff = 0.0;	// sum of offsets of all lines
	sumoff = 0.0;	// sum of all offsets
	numoff = 0.0;
	numoffall = 0.0;
	maxpt = 0;
	for(j = 0 ; j < nl ; j++) {
		if(maxpt < numpt[j]) maxpt = numpt[j];
		jsect[j] = -1;
		sumoff += yl[j][i];
		numoffall += 1.0;
		if(xhi[j] - xlo[j] > min_diff) {
			for(i = 0 ; i < numpt[j] ; i++) {
				aveoff += yl[j][i];
				numoff += 1.0;
			}
		}
	}

	if(numoff <= 0.0)
	{
		for(j = 0 ; j < nl ; j++) {
			for(i = numpt[j] ; i < maxpt ; i++) {
				xl[j][i] = 0.0;
				yl[j][i] = -1.0;
				zl[j][i] = 0.0;
			}
		}
		m = maxpt;
		if(points_between_chines < 0) points_between_chines = 0;

//	If no longitudinal lines are found, investigate the possibility that all lines are frames.

//	Firstly, remove all lines without constant x values

		for(j = 0 ; j < nl ; j++) {
			a = xl[j][0];
			for(i = 1 ; i < numpt[j] ; i++) {
				if(a != xl[j][i]) {
					for(k = j+1 ; k < nl ; k++) {
						for(i = 0 ; i < numpt[k] ; i++) {
							xl[k][i] = xl[k+1][i];
							yl[k][i] = yl[k+1][i];
							zl[k][i] = zl[k+1][i];
						}
						numpt[k] = numpt[k+1];
					}
					nl--;
					j--;
				}
			}
		}
		if(nl < 2) {
			message("Less than two usable frames found.");
			return;
		}

//	Next, sort the frames in ascending order

		for(k = nl ; k > 1 ; k--) {
			for(j = 1 ; j < k ; j++) {
				if(xl[j-1][0] > xl[j][0]) {
					for(i = 0 ; i < maxpt ; i++) {
						a = xl[j-1][i];
						xl[j-1][i] = xl[j][i];
						xl[j][i] = a;
						a = yl[j-1][i];
						yl[j-1][i] = yl[j][i];
						yl[j][i] = a;
						a = zl[j-1][i];
						zl[j-1][i] = zl[j][i];
						zl[j][i] = a;
					}
					i = numpt[j-1];
					numpt[j-1] = numpt[j];
					numpt[j] = i;
				}
			}
		}

//	Next, check the direction of rotation of the along-frame vector. If it is anticlockwise, reverse the order of the points

		for(i = 0 ; i < nl ; i++) {
			theta = 0.0;
			for(k = 1 ; k < numpt[i]-1 ; k++) {
				a = yl[i][k]   - yl[i][k-1];
				b = zl[i][k]   - zl[i][k-1];
				c = yl[i][k+1] - yl[i][k];
				d = zl[i][k+1] - zl[i][k];
				sb = a*d - b*c;
				sa = a*c + b*d;
				if(sa != 0.0 || sb != 0.0) theta += atan2(sb,sa);
			}
			if(theta < 0.0) {
				for(j = 0, k = numpt[i]-1 ; k > j ; k--, j++) {
					a = yl[i][k];
					yl[i][k] = yl[i][j];
					yl[i][j] = a;
					a = zl[i][k];
					zl[i][k] = zl[i][j];
					zl[i][j] = a;
				}
			}
		}

//	Identify apparent chine points

		extlin = 2;
		realloc_hull(extlin);
		for(i = 0 ; i < nl ; i++)
			for(j = 0 ; j < extlin ; j++) yline[j][i] = -1.0;

		for(i = 0 ; i < nl ; i++) {
			yline[0][i] = yl[i][0];
			zline[0][i] = zl[i][0];
			ycont[0][i] = yl[i][0];
			zcont[0][i] = zl[i][0];
			chine_point[0] = TRUE;
			chine_point[numpt[i]-1] = TRUE;
			for(k = 1 ; k < numpt[i]-1 ; k++) {
				a = yl[i][k]   - yl[i][k-1];
				b = zl[i][k]   - zl[i][k-1];
				c = yl[i][k+1] - yl[i][k];
				d = zl[i][k+1] - zl[i][k];
				sb = a*d - b*c;
				sa = a*c + b*d;
				if(sa != 0.0 || sb != 0.0) {
					l = fabs(57.293*atan2(sb,sa));
					chine_point[k] = (l > chine_angle);
				} else {
					chine_point[k] = FALSE;
				}
			}
			for(k = numpt[i] ; k < maxpt ; k++) chine_point[k] = FALSE;

//	Calculate control points, or add points to fill out the required number between chines

			j = 0;		// previous chine point
			n = 0;		// offset point index
			M = points_between_chines + 1;
			il =0;
			line_index[i][0] = 0;
			d = 0.0;
			for(k = 1 ; k < numpt[i] ; k++) {
				a = yl[i][k] - yl[i][k-1];
				b = zl[i][k] - zl[i][k-1];
				d += sqrt(a*a + b*b);
				if(chine_point[k]) {

					// Get the offsets of these lines

					y1 = yl[i][j];	// previous offset point
					z1 = zl[i][j];
					y2 = yl[i][k];	// current offset point
					z2 = zl[i][k];

					if(allow_curvature) {

						n++;			// hull offset index
						if(n >= extlin) {
							extlin = n+1;
							realloc_hull(extlin);
							for(m = 0 ; m < nl ; m++) yline[n][m] = -1.0;
						}
						yline[n][i] = y2;
						zline[n][i] = z2;

//	If we allow curvature, we add no points between chine lines, but estimate a control point

						//	Calculate a transverse normal vector - note that zline has opposite sense to zl

						nz = z2 - z1;
						ny = y2 - y1;
						b = -1.0;		// to be maximum distance of a point from a straight outline between j and k
						lsq = ny*ny+nz*nz;
						if(lsq > 0.0) {	// should always be the case
							l = sqrt(lsq);
							nz /= l;
							ny /= l;	// (ny, nz) is normalised vector from point 1 to point 2, in hull coordinates
										// (nz,-ny) is normal vector to this vector

							//	Now check each point on the section outline, calculate its distance from the line from
							//	j to k, and if it is less than twice the length of the line away,
							//	compare it to the maximum. If it exceeds the maximum, save it

							for(m = j+1 ; m < k ; m++) {
								yr = yl[i][m];
								zr = zl[i][m];
								if(yr == -1.0) continue;

								// "a" is perpendicular distance of this vertex from the line between points 1 and 2

								sa = (yr - y1)*nz - (zr - z1)*ny;
								a = fabs(sa);

								// "c" is perpendicular distance of this vertex from the perpendicular at point 1
								c  = (yr - y1)*ny + (zr - z1)*nz;

								// "d" is perpendicular distance of this vertex from the perpendicular at point 2
								d  = (yr - y2)*ny + (zr - z2)*nz;
								min_diff = 0.2 * a;
								if( c >= min_diff && d <= -min_diff && a > b && a <= 0.5*l ) {
									b = a;
									sb = sa;
									ym = yr;
									zm = zr;
								}
							}
							if(b >= 0.0) {
								ycont[n][i] = ym + nz * sb;
								zcont[n][i] = zm - ny * sb;
							} else {
								ycont[n][i] = y2;
								zcont[n][i] = z2;
							}
						}

					} else {

//	No curvature  - add specified number of points between lines

						sa = d / (REAL) M;				// "d" is the estimated distance between the current pair of chine points
						a = sa;
						c = 0.0;
						sb = yl[i][j+1] - yl[i][j];
						b  = zl[i][j+1] - zl[i][j];
						l = sqrt(sb*sb + b*b);			// new distance along hull curve
						il = n + points_between_chines;
						m = j;
						while(1) {
							if(m == k || fabs(c - a) < fabs(l - a)) {	// c-a represents distance from offset to required position, l-a is distance from next to required
								if(n > il) break;
								if(++n >= extlin) {
									extlin = n+1;
									realloc_hull(extlin);
									for(ii = 0 ; ii < nl ; ii++) yline[n][ii] = -1.0;
								}
								yline[n][i] = yl[i][m];
								zline[n][i] = zl[i][m];
								ycont[n][i] = yl[i][m];
								zcont[n][i] = zl[i][m];
								a += sa;
							} else if(++m < k) {
								c = l;						// previous distance along hull curve
								sb = yl[i][m+1] - yl[i][m];
								b  = zl[i][m+1] - zl[i][m];
								l += sqrt(sb*sb + b*b);		// new distance along hull curve
							}
						}
					}
					j = k;
					d = 0.0;
				}
			}

		}

		count = nl;
		numlin = extlin;
		for(j = 0 ; j < extlin ; j++) {
			stsec[j] = 0;
			ensec[j] = count-1;	// default end, also indicates not ended
		}
		for(i = 1 ; i < count ; i++) xsect[i] = xl[i-1][0];
		xsect[0] = 2.0*xsect[1] - xsect[2];

		for(i = 0 ; i < count ; i++) {
			for(k = extlin - 1 ; k >= 0 && yline[k][i] < 0.0 ; k--) ;
			for(j = k+1 ; j < extlin ; j++) {
				yline[j][i] = yline[k][i];
				zline[j][i] = zline[k][i];
				ycont[j][i] = yline[k][i];
				zcont[j][i] = zline[k][i];
			}
		}
		for(i = 0 ; i < count ; i++) {
			for(j = 0 ; j < extlin ; j++) {
				linewt[j][i] = 1.0;
			}
		}

//	Test each pair of lines, to remove mirrored duplicates

		aveoff = sumoff / numoffall;
		for(n = 1 ; n < numlin ; n++) {
			for(j = 0 ; j < numlin ; ) {	// either j is incremented or numlin is decremented, both inside the loop

//	If number of points on each of the tested pair match, test to see if lines are of mirrored planforms and identical z values

				if(j != n && stsec[j] == stsec[n] && ensec[j] == ensec[n]) {
					ysummax = -1.0e+30;
					ysummin = 1.1e+30;
					ysumj = 0.0;
					ysumn = 0.0;
					for(i = stsec[j] ; i < ensec[j] ; i++) {
						if(fabs(zline[j][i] - zline[n][i]) > min_diff) break;	// z values differ - not mirrored
						ysum = yline[j][i] + yline[n][i];
						ysumj += yline[j][i];
						ysumn += yline[n][i];
						if(ysum > ysummax) ysummax = ysum;	// ysum should be constant for mirrored planforms
						if(ysum < ysummin) ysummin = ysum;
					}
					if(i >= stsec[j] && fabs(ysummax - ysummin) < min_diff && fabs((ysummax+ysummin)*0.5 - aveoff) < min_diff) {
						for(k = ysumn < ysumj ? n : j ; k < numlin-1 ; k++) {
							for(i = stsec[k+1] ; i < ensec[k+1] ; i++) copyline(k,k+1,0.0);
							xhi[k] = xhi[k+1];
							xlo[k] = xlo[k+1];
						}
						numlin--;
						n--;
					}
					else {
						j++;
					}
				}
				else {
					j++;
				}
			}
		}
		stemli = numlin -1 ;

		goto skip;
	}
	else {

		aveoff /= numoff;

//	Test each pair of lines, to remove mirrored duplicates

		for(n = 1 ; n < nl ; n++) {
			for(j = 0 ; j < n ; ) {

//	If number of points on each of the tested pair match, test to see if lines are of mirrored planforms and identical z values

				if(j != n  && numpt[j] == numpt[n]) {
					ysummax = -1.0e+30;
					ysummin = 1.0e+30;
					ysumj = 0.0;
					ysumn = 0.0;
					for(i = 0 ; i < numpt[j] ; i++) {
						if(fabs(xl[j][i] - xl[n][i]) > min_diff) break;	// x values differ - not mirrored
						if(fabs(zl[j][i] - zl[n][i]) > min_diff) break;	// z values differ - not mirrored
						ysum = yl[j][i] + yl[n][i];
						ysumj += yl[j][i];
						ysumn += yl[n][i];
						if(ysum > ysummax) ysummax = ysum;	// ysum should be constant for mirrored planforms
						if(ysum < ysummin) ysummin = ysum;
					}
					if(i >= numpt[j] && fabs(ysummax - ysummin) < min_diff && fabs((ysummax+ysummin)*0.5 - aveoff) < min_diff) {
						for(k = ysumn < ysumj ? n : j ; k < nl-1 ; k++) {
							for(i = 0 ; i < numpt[k+1] ; i++) {
								xl[k][i] = xl[k+1][i];
								yl[k][i] = yl[k+1][i];
								zl[k][i] = zl[k+1][i];
							}
							numpt[k] = numpt[k+1];
							xhi[k] = xhi[k+1];
							xlo[k] = xlo[k+1];
						}
						nl--;
						n--;
					}
					else {
						j++;
					}
				}
				else {
					j++;
				}
			}
		}
	}

//	hull lines extend along the total length of the hull, sections along near-zero length

	count = 1;
	xs[0] = x_stem;
	sumy[0] = 0.0;
	jsect[0] = -1;
	m = nl+1;
	for(n = 0 ; n < nl ; ) {
		x = xhi[n] - xlo[n];
		if(x < min_diff) {	// line is a section

			xs[count] = 0.5*(xhi[n] + xlo[n]);
			m--;

//	Determine whether the section is drawn sheer to keel (hullform style) or in reverse

			if(numpt[n] > 2) {

//	This code measures the total rotation of the along-section vector. Clockwise rotation
//	corresponds to the normal direction, anticlockwise to the reverse direction

				a = 0.0;
				for(i = 2 ; i < numpt[n] ; i++) {
					y2 = yl[n][i] - yl[n][i-1];
					z2 = zl[n][i] - zl[n][i-1];
					y1 = yl[n][i-1] - yl[n][i-2];
					z1 = zl[n][i-1] - zl[n][i-2];
					yr = z2*y1 - y2*z1;	// cross product gives sin term
					zr = y1*y2 + z1*z2;	// dot product give cos term
					if(yr != 0.0 || zr != 0.0) a += atan2(yr,zr);
				}
				reverse = a > 0.0;
			} else {
				reverse = yl[n][1] - zl[n][1] > yl[n][0] - zl[n][0];
			}
			a = 0.0;
			for(j = 0 ; j < numpt[n] ; j++) {	// preserve this outline at top of table
				i = reverse ? numpt[n] - 1 - j : j;
				xl[m][i] = xl[n][j];
				yl[m][i] = yl[n][j];
				zl[m][i] = zl[n][j];
				a += yl[n][j];
			}
			sumy[count] = a;
			numpt[m] = numpt[n];
			jsect[count] = m;	// remember where it was put
			count++;

// remove this "line" from the table of longitudinal lines

			for(j = n ; j < nl-1 ; j++) {
				for(i = 0 ; i < numpt[j+1] ; i++) {
					xl[j][i] = xl[j+1][i];
					yl[j][i] = yl[j+1][i];
					zl[j][i] = zl[j+1][i];
				}
				xhi[j] = xhi[j+1];
				xlo[j] = xlo[j+1];
				numpt[j] = numpt[j+1];
			}
			nl--;
			numpt[nl] = 0;
		}
		else {
			n++;
		}
	}

	if(nl == 1) {
		message("Only one longitudinal line was seen. At least two are needed to define a hull.");
		return;
	} else if(nl == 0) {
		message("No longitudinal lines were seen.");
		return;
	}

//	Add sections wherever a line does not end or start at an existing section

	for(j = 0 ; j < nl ; j++) {
		for(i = 0 ; i < count ; i++) {
			if(fabs(xhi[j] - xs[i]) < min_diff) break;
		}
		if(i >= count) {
			if(count >= maxsec) break;
			jsect[count] = -1;
			xs[count++] = xhi[j];
		}
		for(i = 0 ; i < count ; i++) {
			if(fabs(xlo[j] - xs[i]) < min_diff) break;
		}
		if(i >= count) {
			if(count >= maxsec) break;
			jsect[count] = -1;
			xs[count++] = xlo[j];
		}
	}

	//	Sort the sections

	for(i = count ; i > 0 ; i--) {
		for(j = 1 ; j < i ; j++) {
			if(xs[j-1] > xs[j]) {
				a = xs[j-1];
				xs[j-1] = xs[j];
				xs[j] = a;
				a = sumy[j-1];
				sumy[j-1] = sumy[j];
				sumy[j] = a;
				k = jsect[j-1];
				jsect[j-1] = jsect[j];
				jsect[j] = k;
			}
		}
	}

	//	Remove duplicates from the list of transverse sections (no need to delete their data)

	for(i = 1 ; i < count ; ) {
		if(fabs(xs[i-1]-xs[i]) < min_diff) {

			//	Remove the section with the lower offsets

			for(j = sumy[i-1] < sumy[i] ? i-1 : i ; j < count-1 ; j++) {
				xs[j] = xs[j+1];
				jsect[j] = jsect[j+1];
				sumy[j] = sumy[j+1];
			}
			count--;
		}
		else {
			i++;
		}
	}

	//	(sumy is not subsequently used)

	for(i = 0 ; i < maxsec ; i++) offsets_available[i] = FALSE;

	if(edit_positions) {
		poslist.string = text;
		poslist.index = 0;
		for(i = 0 ; i < count ; i++) {
			sprintf(sectpos[i],"%.3f",xs[i]);
			poslist.table[i] = sectpos[i];
			xsect[i] = xs[i];
		}
		poslist.table[count] = "";
		ncount = count;

		text[0] = 0;
		if(getdlg(DXFPOSITIONS,
		INP_LBX,(void *) &poslist,
		INP_PBF,dxfinputfunc,
		INP_STR,(void *) text,
		-1)) {
			i = 0;	// index in table of sections from file
			j = 0;	// index in table of required sections
			while(j < ncount && i < count) {
				if(xsect[j] > xs[i])
					i++;
				else if(xsect[j] < xs[i])
					j++;
				else {
					offsets_available[j] = jsect[i];
					i++;
					j++;
				}
			}
			count = ncount;
		}
	} else {
		for(i = 0 ; i < count ; i++) {
			xsect[i] = xs[i];
			offsets_available[i] = jsect[i];
		}
	}

	//	Construct hull data

	if(nl > 0) {
		realloc_hull(nl);
	}
	else {
		message("No hull lines seen");
		count = 0;
		return;
	}
	numlin = extlin = nl;
	stemli = numlin - 1;

//	Interpolate line offsets at section positions, to give final offsets

	for(j = 0 ; j < nl ; j++) {
		position_sum[j] = 0.0;
		n = numpt[j];
		stsec[j] = -1;
		ensec[j] = -1;
		relcont[j] = FALSE;
		for(i = 0 ; i < count ; i++) {
			k = 0;
			while(1) {
				if(xl[j][k] == xsect[i]) break;
				if(++k == n) break;
				if((xl[j][k] >= xsect[i]) != (xl[j][k-1] >= xsect[i])) break;
			}
			if(k < n) {	// section cuts line
				if(xl[j][k] == xsect[i]) {
					yline[j][i] = yl[j][k];
					zline[j][i] = -zl[j][k];
					ycont[j][i] = yl[j][k];
					zcont[j][i] = -zl[j][k];
				}
				else {
					a = (xl[j][k] - xsect[i]) / (xl[j][k] - xl[j][k-1]);
					b = 1.0 - a;
					yline[j][i] =   b*yl[j][k] + a*yl[j][k-1];
					zline[j][i] = -(b*zl[j][k] + a*zl[j][k-1]);
					ycont[j][i] =   b*yl[j][k] + a*yl[j][k-1];
					zcont[j][i] = -(b*zl[j][k] + a*zl[j][k-1]);
				}
				if(stsec[j] < 0 || xsect[i] < xsect[stsec[j]]) stsec[j] = i;
				if(ensec[j] < 0 || xsect[i] > xsect[ensec[j]]) ensec[j] = i;
			}
			linewt[j][i] = 1.0;

//	Find index on section where line cuts the section, to help in defining order of hull lines from sheer to keel

			ym = yline[j][i];
			zm = -zline[j][i];	// ?
			m = jsect[i];
			sb = 1.0e+30;
			sa = 0.0;
			for(k = 1 ; k < numpt[m] ; k++) {
				get_distances(yl[m][k-1],zl[m][k-1],yl[m][k],zl[m][k],ym,zm,&a,&b);
				if(a > -0.1 && a < 1.1 && b < sb) {
					sb = b;
					sa = a + (k-1);

				}
			}

//	Total the sum of section positions for all lines, to allow later sorting into the correct order from sheerline to keel

			position_sum[j] += sa;

		}
		for(i = maxsec ; i < maxsec+2 ; i++) {
			yline[j][i] = 0.0;
			zline[j][i] = 0.0;
			ycont[j][i] = 0.0;
			zcont[j][i] = 0.0;
		}
	}

//	The order of the lines should run from sheer to keel - i.e., increasing position_sum values

	for(k = numlin ; k > 0 ; k--) {
		for(j = 1 ; j < k ; j++) {
			if(position_sum[j] < position_sum[j-1]) {
				copyline(numlin,j,0.0);
				copyline(j,j-1,0.0);
				copyline(j-1,numlin,0.0);
				a = position_sum[j];
				position_sum[j] = position_sum[j-1];
				position_sum[j-1] = a;
			}
		}
	}

	//	Adjust control points where possible

	if(allow_curvature) {
		for(i = 0 ; i < count ; i++) {
			m = offsets_available[i];	// find location in master table of this sections' offsets
			if(m >= 0) {	// allow for later non-availability of data

				// Find the first line, j, cutting this section

				for(j = 0 ; j < numlin ; j++)
					if(stsec[j] <= i && ensec[j] >= i) break;
				if(j >= numlin) continue;
				while(1) {

					// Find the next line, k, cutting this section

					for(k = j+1 ; k < numlin ; k++)
						if(stsec[k] <= i && ensec[k] >= i) break;
					if(k >= numlin) break;

					// Get the offsets of these lines

					y1 = yline[j][i];
					z1 = zline[j][i];
					y2 = yline[k][i];
					z2 = zline[k][i];

					//	Calculate a transverse normal vector - note that zline has opposite sense to zl

					nz = z2 - z1;
					ny = y2 - y1;
					b = -1.0;
					lsq = ny*ny+nz*nz;
					if(lsq > 0.0) {
						l = sqrt(lsq);
						nz /= l;
						ny /= l;	// (ny, nz) is normalised vector from point 1 to point 2, in hull coordinates
						// (nz,-ny) is normal vector to this vector

						//	Now check each point on the section outline, requiring each to be within the pair
						//	of lines running perpendicular to the line between points 1 and 2. If one is, calculate
						//	its distance from that line, and if it is less than twice the length of the line away,
						//	compare it to the maximum. If it exceeds the maximum, save it

						for(n = 0 ; n < numpt[m] ; n++) {
							yr =  yl[m][n];
							zr = -zl[m][n];
							if(yr == -1.0) continue;

							// "a" is perpendicular distance of this vertex from the line between points 1 and 2
							//	NOTE we are using vector (nz,-ny)

							sa = (yr - y1)*nz - (zr - z1)*ny;
							a = fabs(sa);

							// "c" is perpendicular distance of this vertex from the perpendicular at point 1
							c = (yr - y1)*ny + (zr - z1)*nz;

							// "d" is perpendicular distance of this vertex from the perpendicular at point 2
							d = (yr - y2)*ny + (zr - z2)*nz;
							min_diff = 0.2 * a;
							if( c >= min_diff && d <= -min_diff && a > b && a <= 0.5*l ) {
								b = a;
								sb = sa;
								ym = yr;
								zm = zr;
							}
						}
						if(b >= 0.0) {
							ycont[k][i] = ym + nz * sb;
							zcont[k][i] = zm - ny * sb;
						}
					}
					j = k;
				}
			}
		}
	}

skip:
	for(i = 0 ; i < count ; i++) master[i] = TRUE;

	for(j = 0 ; j < numlin ; j++) {
		if(stsec[j] >= ensec[j]) {	// line of zero length - discard
			for(k = j+1 ; k < numlin ; k++) copyline(k-1,k,0.0);
			numlin--;
		}
	}

	extlin = numlin;
#ifdef PROF
	fl_line1[0] = numlin;
#endif
	if(numlin > 0) {
		realloc_hull(numlin);
	}
	else {
		message("No hull lines seen");
		count = 0;
	}
	save_hull(MAINHULL);
}

int read_dxf_file(REAL xl_dxf[maxlin][maxsec],REAL yl_dxf[maxlin][maxsec],REAL zl_dxf[maxlin][maxsec],int *nl,int numpt[maxlin])
{
	FILE *fp;
	int vertex_watch,expect_coords;
	int have_x,have_y,have_z;
	char text[MAX_PATH];
	char *p;
	int code;
	int i,j,k,m,n,ii,il;
	int polyline_def;
	REAL z_offset = 0.0;
	int linemode = 0;
	int reverse;
	int numeric;
	REAL lwz,x;
	int M,N;				/* mesh sizes */
	REAL x_extr,y_extr,z_extr;
	int closed;
	int use_interval,use_counter;

	if( (fp = fopen((char *) dxf_filename,"rt")) == NULL) {
		message("Can not open DXF file for input");
		return 0;
	}

	for(i = 0 ; i < maxsec ; i++) {
		for(j = 0 ; j < maxlin ; j++) {
			yl_dxf[j][i] = -1.0;
			zl_dxf[j][i] = -1.0;
		}
	}
	for(i = 0 ; i < maxlin ; i++) numpt[i] = 0;

//	Read all POLYLINE, LWPOLYLINEs and any MESH

	*nl = 0;
	editinit = TRUE;	// ensure next edit re-sets the table of editable sections
	vertex_watch = FALSE;
	expect_coords = FALSE;
	use_interval = 1;
	use_counter = 0;
	while(fgets(text,sizeof(text),fp) != NULL) {
new_entity:
		if( (p = strchr(text,'\n')) != NULL) *p = 0;
		p = text;

//	If the line holds numeric data, read it now and flag the presence of numeric data

		numeric = TRUE;
		while(*p) {
			if(!isspace(*p) && !isdigit(*p)) numeric = FALSE;
			*p = toupper(*p);
			p++;
		}
		if(numeric) {
			if(sscanf(text,"%d",&code) != 1) code = -1;
		} else {
			code = -1;
		}
		if(strncmp(text,"POLYLINE",8) == 0) {

//	Start handling a POLYLINE

			il = *nl;
			linemode = POLYLINE;
			vertex_watch = TRUE;	// watching for POLYLINE vertices
			polyline_def = TRUE;
			z_offset = 0.0;
			i = -1;
			if((*nl)+1 >= maxlin) {
				message("Too many lines: excess discarded.");
				il--;
				(*nl)--;
				break;
			}
		}
		else if(strncmp(text,"LWPOLYLINE",10) == 0) {
			il = *nl;
			(*nl)++;
			linemode = LWPOLYLINE;
			lwz = 0.0;
			expect_coords = TRUE;	// coordinates follow immediately
			polyline_def = FALSE;	// not defining a POLYLINE
			have_x = FALSE;
			have_y = FALSE;
			i = 0;
			if((*nl)+1 >= maxlin) {
				message("Too many lines: excess discarded.");
				il--;
				(*nl)--;
				break;
			}

//	LWPOLYLINE has a fixed extrusion ("other") coordinate, which may be an x, y or z value
			x_extr = 0.0;
			y_extr = 0.0;
			z_extr = 1.0;
		}
		else if(polyline_def && code == 70) {

//	tag value 70 indicates POLYLINE properties follow

			if(fgets(text,sizeof(text),fp) == NULL) {
				message("Premature end of DXF file");
				break;
			}
			sscanf(text,"%d",&n);
			closed = (n & 1) != 0;				/* line or mesh is closed */
			if((n & 16) != 0) linemode = MESH;	/* polygon mesh */
			M = -1;
			N = -1;
		}
		else if(polyline_def && code == 71) {	/* M vertex count for a MESH */
			if(fgets(text,sizeof(text),fp) == NULL) {
				message("Premature end of DXF file");
				break;
			}
			sscanf(text,"%d",&M);
			if(M > maxsec) {
				message("Too many longitudinal lines on polygon mesh");
				break;
			}
		}
		else if(polyline_def && code == 72) {	/* N vertex count for a MESH */
			if(fgets(text,sizeof(text),fp) == NULL) {
				message("Premature end of DXF file");
				break;
			}
			sscanf(text,"%d",&N);
			if(N > maxsec) {
				message("Too many transverse lines on polygon mesh");
				break;
			}
		}
/*
		else if(polyline_def && code == 30 && !vertex_watch) {
			if(fgets(text,sizeof(text),fp) == NULL) {
				message("Premature end of DXF file");
				break;
			}
			sscanf(text,"%f",&z_offset);
		}
*/
		else if(linemode == LWPOLYLINE) {
			if(fgets(text,sizeof(text),fp) == NULL) {
				message("Premature end of DXF file");
				break;
			}
			switch(code) {
			case 0:	// end of line - transform using extrusion vector
				x = sqrt(x_extr*x_extr + y_extr*y_extr + z_extr*z_extr);
				if(x > 0.0) {
					x_extr /= x;
					y_extr /= x;
					z_extr /= x;
				} else {
					z_extr = 1.0;	// all zeroes return to default
				}
				if(fabs(x_extr) < 0.1) x_extr = 0.0;
				if(fabs(y_extr) < 0.1) y_extr = 0.0;
				if(fabs(z_extr) < 0.1) z_extr = 0.0;
				if(	x_extr != 0.0 && (y_extr != 0.0 || z_extr != 0.0) ||
					y_extr != 0.0 && (x_extr != 0.0 || z_extr != 0.0) ||
					z_extr != 0.0 && (x_extr != 0.0 || y_extr != 0.0) ) {
					message("Extrusion direction must point along the x- , y- or z-axis");
					il--;
					(*nl)--;
				} else if(fabs(x_extr) != 0.0) {	// +/-z goes to x, y goes to z, x goes to y
					for(ii = 0 ; ii < numpt[il] ; ii++) {
						x = xl_dxf[il][ii];
						xl_dxf[il][ii] = x_extr * zl_dxf[il][ii];
						zl_dxf[il][ii] = yl_dxf[il][ii];
						yl_dxf[il][ii] = -x;
						x = xl_dxf[il][ii];
					}
				} else if(fabs(y_extr) != 0.0) {	// z goes to +- y, x untouched
					for(ii = 0 ; ii < numpt[il] ; ii++) {
						x = yl_dxf[il][ii];
						yl_dxf[il][ii] = y_extr * zl_dxf[il][ii];
						zl_dxf[il][ii] = -x;
						x = xl_dxf[il][ii];
					}
				}									// z_extr remains - no changes needed
				goto new_entity;
			case 90:	// number of points on lwpolyline
				sscanf(text,"%d",&numpt[il]);
				if(numpt[il] == 32768) {
					il--;		// unsupported type
					(*nl)--;
					linemode = 0;
				}
				break;
			case 38:	//	Read elevation - fixed Z value
				sscanf(text,"%f",&lwz);
				break;
			case 210:
				sscanf(text,"%f",&x_extr);
				break;
			case 220:
				sscanf(text,"%f",&y_extr);
				break;
			case 230:
				sscanf(text,"%f",&z_extr);
				break;
			case 10:
				if(expect_coords) sscanf(text,"%f",&xl_dxf[il][i]);
				have_x = TRUE;
				goto test_for_both;
			case 20:
				if(expect_coords) sscanf(text,"%f",&yl_dxf[il][i]);
				have_y = TRUE;
test_for_both:
				if(have_x && have_y) {
					zl_dxf[il][i] = lwz;	// add third coordinate when the other two have been defined
					use_counter++;
					if(use_counter >= use_interval) {
						i++;
						use_counter = 0;
					}
					expect_coords = i < numpt[il];	// end of coordinates when all points have been read (see case 90, above)
					have_x = FALSE;
					have_y = FALSE;
				}
				break;
			}
		}
		else if(linemode != 0 && strncmp(text,"SEQEND",6) == 0) {	// end of a POLYLINE
			numpt[il] = i+1;
			(*nl)++;
			vertex_watch = FALSE;
			linemode = 0;
		}
		else if(vertex_watch && strncmp(text,"VERTEX",6) == 0) {

//	POLYLINE or MESH vertex

			if(expect_coords) {	/* if expecting coordinates, not a vertex, not all coordinates were provided */
				message("The file included a less-than-three dimensional POLYLINE, so was invalid for conversion");
				il = 0;
				return 0;
			}
			if(polyline_def) {
				if(linemode == MESH && (M < 0 || N < 0)) {
					message("Polygon mesh defined without size specification");
					break;
				}
				polyline_def = FALSE;
			}
			have_x = FALSE;
			have_y = FALSE;
			have_z = FALSE;
			expect_coords = TRUE;
			if(use_counter >= use_interval) {
				i++;
				use_counter = 0;
			}
			if(i >= maxsec) {
				use_interval *= 2;
				for(k = 1, j = 2 ; j < i ; j+= 2, k++) {
					xl_dxf[il][k] = xl_dxf[il][j];
					yl_dxf[il][k] = yl_dxf[il][j];
					zl_dxf[il][k] = zl_dxf[il][j];
				}
				i = j;
//				message("A POLYLINE was too long, and has been truncated");
//				vertex_watch = FALSE;
			}
			if(linemode == MESH && i >= M) {
				numpt[il] = i+1;
				i = 0;
				il = *nl;
				i = -1;
				(*nl)++;
				if(*nl >= maxlin) {
					message("Too many lines: excess discarded.");
					il--;
					(*nl)--;
					break;
				}
			}
		}
		else if(expect_coords) {
			if(code == 10) {
				fgets(text,sizeof(text),fp);
				if(sscanf(text,"%f",&xl_dxf[il][i]) == 1) have_x = TRUE;
			} else if(code == 20) {
				fgets(text,sizeof(text),fp);
				if(sscanf(text,"%f",&yl_dxf[il][i]) == 1) have_y = TRUE;
			} else if(code == 30) {
				fgets(text,sizeof(text),fp);
				if(sscanf(text,"%f",&zl_dxf[il][i]) == 1) have_z = TRUE;
				zl_dxf[il][i] -= z_offset;
			}
			expect_coords = !(have_x & have_y & have_z);
		}
	}

	fclose(fp);

	return 1;
}


/*    draw perspective view in specified box		*/

void draw_dxf_view()
{
	int i,j;
	extern REAL zpersp;

	if(show_dxf && nl_dxf > 0) {
		(*colour)(9);
		for(j = 0 ; j < nl_dxf ; j++) {
			(*newlin)();
			for(i = 0 ; i < numpt_dxf[j] ; i++) {
				zpersp = -xl_dxf[j][i];
				(*draw)(yl_dxf[j][i],-zl_dxf[j][i]);
			}
		}
	}
}

void draw_dxf_profile()
{
	int i,j;
	extern REAL zpersp;

	if(show_dxf && nl_dxf > 0) {
		(*colour)(9);
		for(j = 0 ; j < nl_dxf ; j++) {
			(*newlin)();
			for(i = 0 ; i < numpt_dxf[j] ; i++) (*draw)(xl_dxf[j][i],-zl_dxf[j][i]);
		}
	}
}

void draw_dxf_plan()
{
	int i,j;

	if(show_dxf && nl_dxf > 0) {
		(*colour)(9);
		for(j = 0 ; j < nl_dxf ; j++) {
			(*newlin)();
			for(i = 0 ; i < numpt_dxf[j] ; i++) (*draw)(xl_dxf[j][i],yl_dxf[j][i]);
		}
	}
}

void draw_dxf_endview()
{
	int i,j;

	if(show_dxf && nl_dxf > 0) {
		(*colour)(9);
		for(j = 0 ; j < nl_dxf ; j++) {
			(*newlin)();
			for(i = 0 ; i < numpt_dxf[j] ; i++) (*draw)(yl_dxf[j][i],-zl_dxf[j][i]);
		}
	}
}



#endif






