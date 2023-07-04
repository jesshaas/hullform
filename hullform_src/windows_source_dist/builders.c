/* Hullform component - builders.c
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
 
/*		WRITE OUT BOATBUILDER'S OFFSETS	*/

#include "hulldesi.h"
#include <string.h>

char metric_format[16]  = "%8.4f%8.4f%6d\n\t";
char british_format[27] = "%3d'%6.2f\" %3d'%6.2f\"%6d\n\t";

INT wrunit(FILE *fp,INT nl,int mode,REAL x,REAL y);
REAL tvalue(REAL dy,REAL dz,REAL a,REAL hb,REAL c,REAL hd);
REAL curvedist(REAL t,REAL *rate);
REAL solvecurve(REAL s0,REAL t,REAL dt);
void stringer_param(REAL a,REAL hb,REAL c,REAL hd);
void make_outline(int i,REAL *tn,REAL outl_inc,int mode,FILE *fp,REAL side);
int normalise(REAL a,REAL c,REAL *al,REAL *cl,REAL *len);
extern REAL mousehole_radius;
void drawarc(REAL xsect,REAL xc,REAL yc,REAL r,REAL start,REAL end,
	REAL side,int mode);

extern REAL b,d;
extern REAL A,B,C;
char *warning = "CAN NOT WRITE TO THIS FILE";
int	outl_places = 4;
REAL	outl_inc = 0.100;
REAL	def_outl_thickness = 0.0;
REAL	*outl_thickness;
REAL	mousehole_radius = 0.0,
notch_width = 0.0,
notch_height = 0.0;
int	mouseholes;
FILE	*builders_fp = NULL;
extern char *builders_filename;
BOOL CALLBACK FrameDlgProc(HWND hWndDlg,unsigned msg,WORD wParam,LONG lParam);

void builders_output()
{
#ifdef DEMO
	not_avail();
#else

	char	input[MAX_PATH];
	REAL	x,x1;
	INT	i,i1;
	int	use[maxsec];
	REAL	*tn;
	void 	def_builders(int code,HWND hWndDlg);
	extern char	*filename;

#ifdef SHAREWARE
	nagbox();
#endif
	if(count <= 0) return;
	strcpy(input,"ALL");
	update_func = NULL;
	cls(0);
	if(!getdlg(BUILDERS,
		INP_REA,(void *) &outl_inc,
		INP_INT,(void *) &outl_places,
		INP_STR,(void *) input,
#ifndef STUDENT
		INP_LOG,(void *) &showstringers,
#else
		INP_LOG,NULL,
#endif
		INP_LOG,(void *) &showframes,
		INP_PBF,def_builders,
		INP_STR,builders_filename,
		-1) ||
		    !multproc(input,use,count)) return;

	if(builders_fp == NULL) {
		builders_fp = fopen(builders_filename,"wt");
		if(builders_fp == NULL) goto errormessage;
	}

	outl_places += 48;
	*(metric_format  + 3) = (char) outl_places;
	*(metric_format  + 8) = (char) outl_places;
	*(british_format + 7) = (char) outl_places;
	*(british_format + 18)= (char) outl_places;
	outl_places += 4;
	*(metric_format  + 1) = (char) outl_places;
	*(metric_format  + 6) = (char) outl_places;
	*(british_format + 5) = (char) outl_places;
	*(british_format + 16)= (char) outl_places;
	outl_places -= 52;

	fprintf(builders_fp,"\t");
	tn = NULL;
	for(i = surfacemode ; i < count ; i++) {
		if(!use[i]) continue;
		if(i > 0 && showframes) tn = outl_thickness;
		x = xsect[i];
		if(numun <= 1) {	/* metric units */
			if(fprintf(builders_fp,"SECTION NUMBER %d AT %8.3f M\n\t",i,posdir*x) < 31)
				goto errormessage;
		}
		else {		/* British units */
			i1 = posdir*x;
			x1 = (x-(float) i1)*12.0;
			if( fprintf(builders_fp,"SECTION NUMBER %d AT %d'%5.2f\"\n\t",i,i1,x1) < 29)
				goto errormessage;
		}
		make_outline(i,tn,outl_inc,-1,builders_fp,1.0);
	}
	fclose(builders_fp);
	builders_fp = NULL;
	return;

errormessage:
	message(warning);
	fclose(builders_fp);
	builders_fp = NULL;
#endif
}

void def_builders(int code,HWND hWndDlg)
{
#ifdef linux
	void FrameDialog(void);
#endif


	switch(code) {

		/*	Frames button	*/

	case 0:

#ifdef linux
		FrameDialog();
#else
		(void) DialogBox(hInst,(char *) FRAME,hWnd,(DLGPROC) FrameDlgProc);
		if(code < 0) (void) MessageBox(hWnd,"FRAME","Could not create dialog box",MB_OK);
#endif

#ifdef PROF
		mouseholes = mousehole_radius > 0.0;
#endif
		break;

		/*	Browse button	*/

	case 1:
		if(builders_fp != NULL) fclose(builders_fp);
		if(openfile(builders_filename,"wt","Select Builders' Output File",
			"text files(*.txt)\0*.txt\0all files(*.*)\0*.txt",
			"*.txt",filedirnam,&builders_fp)) {
#ifdef linux
			XmTextSetString(wEdit[3],builders_filename);
#else
			SetDlgItemText(hWndDlg,DLGEDIT+3,builders_filename);
#endif
		}
		break;
	}
}

#ifndef linux

BOOL CALLBACK FrameDlgProc(HWND hWndDlg,unsigned msg,WORD wParam,LONG lParam)
{
	void centre_dlg(HWND);
	char text[80];
	int i;
	extern int context_id,HelpUsed;
	BOOL colret;
	extern char *helpfile;
	int ColourMessage(unsigned msg,WORD wParam,LONG lParam,BOOL *hBrush);
	REAL value;

	if(ColourMessage(msg,wParam,lParam,&colret)) return colret;

	switch(msg) {

	case WM_INITDIALOG:
		centre_dlg(hWndDlg);
#ifdef PROF
		sprintf(text,"%.4f",mousehole_radius);
		SetDlgItemText(hWndDlg,DLGEDIT+0,text);
		sprintf(text,"%.4f",notch_width);
		SetDlgItemText(hWndDlg,DLGEDIT+1,text);
		sprintf(text,"%.4f",notch_height);
		SetDlgItemText(hWndDlg,DLGEDIT+2,text);
#endif
		sprintf(text,"%.4f",def_outl_thickness);
		SetDlgItemText(hWndDlg,DLGEDIT+3,text);

		SendDlgItemMessage(hWndDlg,DLGLBX,LB_RESETCONTENT,0,0l);
		for(i = 1 ; i < numlin ; i++) {
			sprintf(text,"%02d-%02d: %.4f",i,i+1,outl_thickness[i]);
			SendDlgItemMessage(hWndDlg,DLGLBX,LB_ADDSTRING,0,(LPARAM) (LPCSTR) text);
		}
		break;

	case WM_COMMAND:
		switch(wParam) {
		case IDOK:
#ifdef PROF
			GetDlgItemText(hWndDlg,DLGEDIT + 0,text,sizeof(text));
			sscanf(text,"%f",&mousehole_radius);
			GetDlgItemText(hWndDlg,DLGEDIT + 1,text,sizeof(text));
			sscanf(text,"%f",&notch_width);
			GetDlgItemText(hWndDlg,DLGEDIT + 2,text,sizeof(text));
			sscanf(text,"%f",&notch_height);
#endif
			for(i = 1 ; i < numlin ; i++) {
				SendDlgItemMessage(hWndDlg,DLGLBX,LB_GETTEXT,i-1,(LPARAM) (LPCSTR) text);
				sscanf(text+7,"%f",&outl_thickness[i]);
			}
			EndDialog(hWndDlg,TRUE);
			break;

		case IDCANCEL:
			EndDialog(hWndDlg,0);
			break;

		case IDHELP:
			context(context_id);
			break;

		case DLGPBF:
			GetDlgItemText(hWndDlg,DLGEDIT+3,text,sizeof(text));
			if(sscanf(text,"%f",&value) == 1) {
				for(i = 1 ; i < numlin ; i++) {
					if(SendDlgItemMessage(hWndDlg,DLGLBX,LB_GETSEL,i-1,0L) != 0) {
						SendDlgItemMessage(hWndDlg,DLGLBX,LB_DELETESTRING,i-1,0L);
						sprintf(text,"%02d-%02d: %.4f",i,i+1,value);
						SendDlgItemMessage(hWndDlg,DLGLBX,LB_INSERTSTRING,i-1,(LPARAM) (LPCSTR) text);
					}
				}
			}
			else {
				message("Invalid entry in edit box");
			}
			break;
		}
		break;
	}
	return 0;
}

#endif

void make_outline(int i,REAL *thickness,REAL outl_inc,int mode,FILE *fp,REAL side)
{
	extern int	numbetwl;
	int		j,jj;
	REAL	a[maxlin],hb[maxlin],c[maxlin],hd[maxlin],b,d,b2,d2;
	REAL	tn,prev_tn;
#ifndef STUDENT
	int 	stringers;
	int		jstr;
	REAL	tstr,cstr,sstr;
	int		next_stringer;
	REAL	ystringer,zstringer;
	int		lasstr;
#endif
	REAL	t;
	REAL	tem1,tem2,div;
	REAL	t1;
	REAL	dist,distincr;
	REAL	xc,yc;
	REAL	dist0;
	int	started = FALSE;
	REAL	dy,dz,dyc,dzc;
	REAL	tstart,tend,next_tstart,t_next;
	REAL	aa,cc;
	REAL	rad = mousehole_radius;
	REAL	ast,aen;/* start and end angles for mousehole drawing */
	int		jk;
	int		more_stringers;
#ifdef EXT_OR_PROF
	extern int	current_hull;
#endif
	REAL	an,cn,al,cl;
	REAL	a1,a2,c1,c2;
	REAL	len;
	int 	j1,j2;
#ifdef EXT_OR_PROF
	if(current_hull != 0) return;
#endif

	if(mode >= 0 && mode < 4)(*colour)(4);

	/*	We need to terminate drawing of on curve of the section's outline
	where it joins the next curve down. So we need to know the details
	of the next curve while drawing the current one. We therefore
	must pre-calculate section curve parameters
	*/
	aa = 0.0;
	cc = 1.0;

	jj = -1;		/* line index for builder's output data */
#ifdef PROF
	mouseholes = mousehole_radius > 0.0 && i > 0;
#else
	mouseholes = FALSE;
#endif
	ast = 3.1415927;	/* angle to start of hole	*/
	jk = -1;		/* index of last outline segment for which */
	/* a radius was drawn	*/
	if(thickness != NULL)
		tn = thickness[0];
	else
		tn = 0.0;

	/*	Default curve parameters	*/

	for(j = 0 ; j <= numlin ; j++) {
		a[j] = 0.0;
		c[j] = 1.0;
		hb[j] = 0.0;
		hd[j] = 0.0;
	}

	/*	set first line index "undefined"	*/

	j1 = j = -1;
	next_tstart = 1.0;

	for(j2 = 0 ; j2 <= numlin ; j2++) {

		/*	Find hull curve parameters	*/

		prev_tn = tn;	/* thickness for surface above j1 */
		if(j2 < numlin) {

			/*	Skip lines which do not pass through this section	*/

			if(stsec[j2] > i || ensec[j2] < i) continue;

			hullpa(i,j2,aa,cc,&a[j2],&hb[j2],&c[j2],&hd[j2]);
			tranpa(a[j2],hb[j2],c[j2],hd[j2],&aa,&cc);

			/*	Define outline thickness				*/

			if(thickness != NULL) tn = thickness[j2];
		}

		/*	Wait until the line at the top of the curve (j1) and the line at the
		base of the curve (j) are both defined
		*/
		if(j1 < 0) goto push_indices;

		b = 2.0*hb[j];
		d = 2.0*hd[j];
		b2 = 2.0*hb[j2];
		d2 = 2.0*hd[j2];

		/*	Locate the first stringer, if any	*/

#ifndef STUDENT
		stringers = numstr[j] > 0;
		if(stringers) {
			jstr = inistr[j];
			lasstr = jstr + numstr[j];
			if(!str_dir[j]) {
				jstr = lasstr;
				lasstr = inistr[j]-1;
			}
			else {
				jstr--;
			}
		}
		else {
			jstr = 0;
			lasstr = 0;
		}
		tstr = -1.0e+36;
#endif

		/*	Find and normalise the tangent vector along the outline,
		at the base of the current curve.
		*/
		if(j == 0) {
			al = 0.0;
			cl = 1.0;
		} else if(normalise(a[j]+0.10*hb[j],c[j]+0.10*hd[j],&a1,&c1,&len)) {
			if(normalise(a[j]+0.20*hb[j],c[j]+0.20*hd[j],&a2,&c2,&len)) {
				al = 2.0*a1 - a2;
				cl = 2.0*c1 - c2;
			} else {
				al = a1;
				cl = c1;
			}
		} else {
			goto push_indices;
		}

		/*	Hence find the angle to the top end of the mousehole	*/

		ast = atan2(cl,al);

		/*	Normalise the tangent vector along the outline, at the
		top of the next curve.
		*/
		if(j2 < numlin) {
			if(normalise(a[j2]+1.90*hb[j2],c[j2]+1.90*hd[j2],&a1,&c1,&len)) {
				if(normalise(a[j2]+1.80*hb[j2],c[j2]+1.80*hd[j2],&a2,&c2,&len)) {
					an = 2.0*a2 - a1;
					cn = 2.0*c2 - c1;
				} else {
					an = a1;
					cn = c1;
				}
			} else {
				continue;	/* skip curve if it can not be handled */
			}

			/*	The mousehole is drawn at the base of the current curve - calculate
			the angle to its bottom end
			*/
			aen = atan2(cn,an);
		} else {	/* symmetrical at keel */
			an = al;
			cn = -cl;
			aen = -1.5707963;
		}

		/*	Find the vector (dy,dz) to the point where the
		frame outlines for next and current curves meet
		*/
		t = cn * al - cl * an;
		if(fabs(t) > 0.02) {	// nonzero limit required to handle fine-angle intersections of different thicknesses
			dz = (cn * prev_tn - cl * tn) / t;
			dy = (an * prev_tn - al * tn) / t;
		}
		else {	/* parallel - presume current thickness */
			dz =  tn * an;
			dy = -tn * cn;
		}

		xc = yline[j][i] + dy;
		yc = zline[j][i] - dz;

#ifdef PROF
		if(mouseholes)
			tend = tvalue(dy+rad*al,dz+rad*cl,a[j],hb[j],c[j],hd[j]);
		else
#endif
			tend = tvalue(dy,dz,a[j],hb[j],c[j],hd[j]);

		/*	Move previously-calculated starting curve parameter	*/

		tstart = next_tstart;

		t = 1.0;

		/*	Initialise distance measurement routine			*/
		stringer_param(a[j],hb[j],c[j],hd[j]);

		dist = curvedist(tstart,&tem1); /* tem1 is ignored here */
		dist0 = curvedist(tend,&tem1);

		if(dist < dist0) dist = dist0;

		/*	If there is not enough room, forget the stringers	*/

#ifdef PROF
		tem1 = fabs(dist-dist0)*0.75;
		if(tem1 < numstr[j] * str_thk[j]) stringers = FALSE;

		next_stringer = stringers;	/* request for next stringer */
#endif
		if(C == 0.0) {
			distincr = dist-dist0;
		}
		else if(outl_inc > 0.0) {
			distincr = outl_inc;
		}
		else {
			distincr = (dist - dist0) / ((REAL)(numbetwl+1));
		}
		if(started) dist -= distincr;

		do {
			t = solvecurve(dist,t,1.0);
			tem1 = curvedist(t,&tem2);
			if(t < tend || dist <= dist0) {
				t = tend;
#ifndef STUDENT
				next_stringer = 0;//stringers;
#endif
			}

#ifndef STUDENT
find_next_stringer:
			if(next_stringer) {

//	Move to the next stringer on this surface. The loop cycles while stringers end before or start after this section

				do {
					if(str_dir[j])
						jstr++;
					else
						jstr--;
					more_stringers = (str_dir[j] ? jstr < lasstr : jstr > lasstr);
				}
				while( more_stringers && (ststrx[jstr] > xsect[i] || enstrx[jstr] < xsect[i])) ;
				if(!more_stringers) {
					tstr = -1.0e+36;	// out of stringers
				}
				else {
					ystringer = ystr[jstr][i];
					zstringer = zstr[jstr][i];
					sstr = sin(astr[jstr][i]);/* astr is from horiz.*/
					cstr = cos(astr[jstr][i]);

					tem1 = ystringer - yline[j][i];
					tem2 = zline[j][i] - zstringer;
					t1 = 0.5*str_thk[j] + notch_width;
					tstr  = tvalue(tem1+sstr*t1,tem2-cstr*t1,a[j],hb[j],c[j],hd[j]);	// t-value at top edge of stringer cutout
				}
				next_stringer = 0;
			}

			/*	If the point to be plotted next is beyond the next stringer, draw
			stringer cutout
			*/
			if(t <= tstr) {
				next_stringer = 1;	/* request location for next stringer */

				t1 = 0.5*str_thk[j];
				tem1 = ystringer - yline[j][i];
				tem2 = zline[j][i] - zstringer;

				ystringer += cstr*prev_tn;
				zstringer -= sstr*prev_tn;
				if(!wrunit(fp,-1,mode,
					side*(ystringer + sstr*(t1+notch_width)),
					zstringer + cstr*(t1+notch_width))) goto errormessage;	/* write failure */

				if(notch_width > 0.0 && notch_height > 0.0) {
					if(!wrunit(fp,-1,mode,
						side*(ystringer + sstr*t1 + cstr*notch_height),
						zstringer + cstr*t1 - sstr*notch_height)) goto errormessage;	/* write failure */
				}

				if(!wrunit(fp,-1,mode,
					side*(ystringer + sstr*t1 + cstr*str_wid[j]),
					zstringer + cstr*t1 - sstr*str_wid[j])) goto errormessage;

				if(!wrunit(fp,-1,mode,
					side*(ystringer - sstr*t1 + cstr*str_wid[j]),
					zstringer - cstr*t1 - sstr*str_wid[j])) goto errormessage;	/* write failure */

				if(!wrunit(fp,-1,mode,
					side*(ystringer - sstr*t1),
					zstringer - cstr*t1)) goto errormessage;	/* write failure */

				t  = tvalue(tem1-sstr*t1,tem2+cstr*t1,a[j],hb[j],c[j],hd[j]);

				started = TRUE;
				goto find_next_stringer;	/* in case there is another before "t" is met	*/

			}
			else {
#endif
				if(t <= tend) break;

				/*	Find surface alignment for along-curve and perpendicular vectors */

				if(t > 0.98 &&	normalise(a[j]+0.95*b,c[j]+0.95*d,&a1,&c1,&len) &&
								normalise(a[j]+0.90*b,c[j]+0.90*d,&a2,&c2,&len)) {
					tem1 = 2.0*a2 - a1;
					tem2 = 2.0*c2 - c1;
				} else {
					tem1 = a[j] + b*t;	/* horizontal component */
					tem2 = c[j] + d*t;	/* vertical component */
				}
				div = fsqr0(tem1*tem1+tem2*tem2);
				if(div > 0.0 || prev_tn == 0.0) {

					if(div > 0.0) {
						dyc = tem2/div;
						dzc = tem1/div;
					}
					else {
						dyc = 0.0;
						dzc = 0.0;
					}

					if(t < 0.999)
						jj = j+1;
					else
						jj = j;
					tem1 = yline[j][i]+t*(a[j]+t*hb[j])-prev_tn*dyc;
					tem2 = zline[j][i]-t*(c[j]+t*hd[j])-prev_tn*dzc;
					if(!wrunit(fp,jj,mode,side*tem1,tem2)) goto errormessage;	/* write failure */
					jj = -1;

					started = 1;
//					if(t < tend) break;
				}
#ifndef STUDENT
			}
			if(jstr == lasstr) stringers = FALSE;
#endif

			dist -= distincr;
		}
		while(t <= 1.1 && t >= tend);

#ifdef PROF
		if(mouseholes) {

			/*	Draw the mousehole at the base of this section segment.	*/

			drawarc(0.0,xc,yc,rad,ast,aen,side,mode);

			/*	Calculate the curve parameter value where curve plotting may start, on the current curve.
			*/
			next_tstart = 1.0 - tvalue(dy-rad*an,dz-rad*cn,-a[j2]-b2,-hb[j2],-c[j2]-d2,-hd[j2]);

		}
		else
#endif
		    {

			if(!wrunit(fp,j+1,mode,side*xc,yc)) goto errormessage;
//			next_tstart = 1.0 - tvalue(dy,dz,-a[j2]-b2,-hb[j2],-c[j2]-d2,-hd[j2]);
			next_tstart = tvalue(a[j2] + hb[j2] + dy,c[j2] + hd[j2] + dz,a[j2],hb[j2],c[j2],hd[j2]);
		}

push_indices:
		j1 = j;
		j = j2;
	}

	/*	Draw the last hole	*/

#ifdef PROF
	if(mouseholes && jk >= 0) {
		al = a[jk];
		cl = c[jk];
		t1 = fsqr0(al*al+cl*cl);
		if(t1 > 0.0 && al != 0.0) {
			if(yline[jk][i] <= 0.0) {
				drawarc(0.0,0.0,zline[jk][i]-prev_tn*t1/fabs(al),
					rad,atan2(cl,al),-1.5707963,side,mode);
			}
			else {
				drawarc(0.0,yline[jk][i]+prev_tn*t1/fabs(cl),zline[jk][i],
					rad,atan2(cl,al),-3.1415927,side,mode);
			}
		}
	}
#endif
	return;

errormessage:
	message(warning);
	fclose(builders_fp);
}

REAL tvalue(REAL dy,REAL dz,REAL a,REAL hb,REAL c,REAL hd)
{
	REAL square = a*a + c*c;
	REAL sqroot = sqrt(square);
	REAL hbp;
	REAL dyp;
	REAL t,t1;
	REAL s,s1;

	if(sqroot != 0.0) {
		dyp = (a*dy + c*dz)/sqroot;
		hbp = (a*hb + c*hd)/sqroot;
		t = square + 4.0*hbp*dyp;
		if(t >= 0.0) {
			t = sqrt(t);
			if(hbp != 0.0) {
				t1 = (t-sqroot)/(2.0*hbp);
				t  =(-t-sqroot)/(2.0*hbp);
			}
			else if(t > 0.0) {
				return dyp / t;
			}
			else {
				return 0.5;
			}
		}
		else {
			return -1.0;
		}
	}
	else {
		square = hb*hb + hd*hd;
		if(square > 0.0) {
			sqroot = sqrt(square);
			dyp = (hb*dy + hd*dz)/sqroot;
			if(dyp >= 0.0) {
				return sqrt(dyp)/sqroot;
			}
			else {
				return -1.0;
			}
		}
		else {
			return -1.0;
		}
	}
	s = fabs(t - 0.5);
	s1 = fabs(t1 - 0.5);
	if(s < s1)
		return t;
	else
	    return t1;
}

int wrunit(FILE *fp,int nl,int mode,REAL xp,REAL yp)
{
	int		i1,i2;
	REAL	x1,x2;
	extern REAL	invert;
	extern int	numun,inches;
	char	format[40];
	char	*p;

	yp *= invert;		/* convert to user's convention	*/

	if(mode == -1) {

		/*	If line index is negative, do not show it	*/

		strcpy(format,numun <= 1? metric_format : british_format);
		if(nl < 0) {
			p = strstr(format,"%6d");
			strcpy(p,p+3);
		}

		if(!inches || numun <= 1) {

			/*	By the C calling convention, the last argument is ignored
			if the format string does not provide for it
			*/
			if(fprintf(fp,format,xp,yp,nl) < 13) return 0;
		}
		else {
			i1 = xp;
			x1 = (xp - (float) i1)*12.0;
			i2 = yp;
			x2 = (yp - (float) i2)*12.0;
			if(i2 < 0) x2 = -x2;
			if(fprintf(fp,format,i1,x1,i2,x2,nl) < 22) return 0;
		}
	}
	else {
		pltsel(0.0,xp,invert*yp,mode);
	}
	return 1;
}

#ifdef PROF
void drawarc(REAL xsect,REAL xc,REAL yc,REAL r,REAL start,REAL end,
	REAL side,int mode)
{
	REAL a,da;
	REAL xa,ya;
	extern int penup;
	REAL range;
	REAL xend;
	int i,n;

	end += 3.1415927;
	xend = xc+r*cos(end);
	if(xend < 0.0 && r > 0.0) {
		a = -xc / r;
		if(a >= -1.0 && a <= 1.0) end = acos(a);
	}

	range = end - start;

	if(range > 0.0) {
		da = 0.1;
		n = 10.0 * range;
	}
	else {
		da = -0.1;
		n = -10.0 * range;
	}
	for(i = 0, a = start ; i < n ; i++, a += da) {
		/*	pltsel(xsect,side*(xc+r*cos(a)),yc-r*sin(a),mode);	*/
		xa = xc+r*cos(a);
		ya = yc-r*sin(a);
		pltsel(xsect,side*xa,ya,mode);
	}
	/*	pltsel(xsect,side*(xc+r*cos(end)),yc-r*sin(end),mode);	*/
	xa = xc+r*cos(end);
	ya = yc-r*sin(end);
	pltsel(xsect,side*xa,ya,mode);
}
#endif

int normalise(REAL a,REAL c,REAL *al,REAL *cl,REAL *div)
{
	*al = a;
	*cl = c;
	*div = fsqr0((*al)*(*al) + (*cl)*(*cl));
	if(*div <= 0.000001) return 0;
	*al /= *div;
	*cl /= *div;
	return 1;
}

