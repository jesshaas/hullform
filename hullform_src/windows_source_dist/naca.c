/* Hullform component - naca.c
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

void add_a_section(REAL);
extern char (*sectname)[12];
extern REAL (*linewt)[maxsec+4];
void check_invalid_end(void);
int index_section(REAL x,int addsec);
extern int changed;
void save_hull(int);
int section_error(int sec);
void fair_controls(void);

MENUFUNC naca()
{
	static int line1;
	static int line2 = -1;
	static REAL in1,in2,en1,en2;
	static int addsec = TRUE;
	static REAL span = 1.0;
	REAL fr;
	REAL t,t1;
	char nnnn[MAX_PATH];
	REAL x,y,z,dz;
	int i,j;
	int j1,j2,is;
#ifdef PROF
	int ie;
#endif
	int insec1,ensec1;
	int insec2,ensec2;
	static REAL naca = 0025.0;
	REAL xf,xb,dxf,dxb;
	extern int numbetw;
	REAL stemrad;
	REAL rstem[maxlin];
	REAL dydzp[maxsec];
	REAL chord;

#ifdef PROF
	if(ntank > 0) {
		if(!getdlg(HAVETANK,-1)) return;
		realloc_hull(numlin);
		ntank = 0;
		extlin = numlin;
		stemli = numlin-1;
		fl_line1[0] = numlin;
	}
#endif

	if(line2 < 0) {
		line1 = stemli+2;
		line2 = min(maxlin,line1+4);
		in1 = xsect[count/2-1];
		en1 = xsect[count/2+1];
		in2 = in1+1;
		en2 = en1;
		naca = 12;
		span = (xsect[count-1]-xsect[0]+dxstem()) / 6.0;
	}

	sprintf(nnnn,"%02.0f",naca);

	if(getdlg(NACA,
		INP_INT,(void *) &line1,
		INP_REA,(void *) &in1,
		INP_REA,(void *) &en1,
		INP_INT,(void *) &line2,
		INP_REA,(void *) &in2,
		INP_REA,(void *) &en2,
		INP_LOG,(void *) &addsec,
		INP_REA,(void *) &span,
		INP_STR,(void *) &nnnn,-1)) {
		if(sscanf(nnnn,"%f",&naca) != 1 || naca <= 0.0) {
			message("Invalid NACA foil number");
			return;
		}
		if(span <= 0.0) {
			message("Span must be positive.");
			return;
		}
		j1 = line1 - 1;
		j2 = line2 - 1;
		if(j1 >= j2-1) {
			message("Need at least three lines, for a\nminimum of two profiles and one\nbounding line.");
			return;
		}

		if(in1 < xsect[0]) {
			message("Starting X of first line must be aft of base of stem.");
			return;
		} else if(in1 > in2) {
			message("Starting X of first line must not be aft of starting X of last line.");
			return;
		} else if(in1 > xsect[count-1]) {
			message("Ending X of first line must not be aft of stern.");
			return;
		}

		if(line2 > numlin) {
			realloc_hull(line2);
			for(j = extlin ; j < line2; j++) copyline(j,numlin-1,0.0);
			extlin = numlin = line2;
			stemli = numlin-1;
#ifdef PROF
			fl_line1[0] = extlin;
#endif
		}

		/*	Locate sections, and add them if requested	*/

		insec1 = index_section(in1,addsec);
		if(section_error(insec1)) return;

		ensec1 = index_section(en1,addsec);
		if(section_error(ensec1)) return;

		if(addsec) {
			insec1 = index_section(in1,addsec);	/* repeat, in case section added */
			if(section_error(insec1)) return;
		}

		xf = xsect[insec1];
		xb = xsect[ensec1];
		insec2 = index_section(in2,addsec);
		if(section_error(insec2)) return;

		ensec2 = index_section(en2,addsec);
		if(section_error(ensec2)) return;

		if(addsec) {
			insec2 = index_section(in2,addsec);	/* repeat, in case section added */
			if(section_error(insec2)) return;
		}

		fr = 1.0/(j2 - j1 - 1);
		dxf =  (xsect[insec2] - xf)*fr;
		dxb =  (xsect[ensec2] - xb)*fr;
		for(j = j1+1 ; j < j2 ; j++) {
			xf += dxf;
			xb += dxb;
			(void) index_section(xf,addsec);
			if(section_error(0)) return;

			(void) index_section(xb,addsec);
			if(section_error(0)) return;
		}

		/*	Repeat the index detection, because new sections may have changed indices	*/

		if(addsec) {
			insec1 = index_section(in1,addsec);
			if(section_error(insec1)) return;
			ensec1 = index_section(en1,addsec);
			if(section_error(ensec1)) return;
		}

		dz =  span*fr;
		for(i = surfacemode ; i < count ; i++) {
			for(j = j1 ; j <= j2 ; j++) {
				yline[j][i] = -1.0;		/* clear all */
				ycont[j][i] = -1.0e+10;
				linewt[j][i] = 0.0;
			}
		}

		xf = xsect[insec1];
		xb = xsect[ensec1];
		z = zline[j1][insec1];		/* base is start of first line */
		for(j = j1 ; j < j2 ; j++) {
			is = -1;

			t = 0.2969*naca/20.0;
			chord = xb - xf;
			stemrad = t*t*chord/2.0;
			rstem[j] = stemrad;
#ifndef STUDENT
			radstem[j] = stemrad;
#endif

			for(i = surfacemode ; i < count ; i++) {
				x = (xsect[i] - xf)/chord;
				if(x >= -0.001 && x <= 1.001) {
					y = naca/20.0*chord*(0.2969*sqrt(max(x,0.0))+x*(-0.126+x*(-0.3516+x*(0.2843-0.1015*x))));
					yline[j][i] = y;
					zline[j][i] = z;
					if(i < count-1 && xsect[i+1] <= xb) {
						t = (0.5*(xsect[i]+xsect[i+1])-xf)/chord;
#ifdef PROF
						if(t > 0.0)
							linewt[j][i] = (-0.2969/(4.0*t*sqrt(t))-0.7032+1.7058*x-1.218*x*x) / (-0.289625);
						else
							linewt[j][i] = 1.0;
#else
						if(t > 0.0) {
							if(is > 0) {
								linewt[j][i] = (-0.2969/(4.0*t*sqrt(t))-0.7032+1.7058*x-1.218*x*x) / (-0.289625);
							}
							else {
								linewt[j][i] = 1000.0;
							}
						}
						else {
							linewt[j][i] = 1.0;
						}
#endif
					}
					else {
						linewt[j][i] = 1.0;
					}
#ifdef PROF
					if(is < 0) is = i;
					ie = i;
#endif

					/*	Locate interior control points	*/

					if(x > 0.0) {
						t1 = 0.14845/sqrt(x)+(-0.126+x*(-0.7032+x*(0.8529-0.406*x)));
						t = (y*(dxb-dxf)/chord + (naca/20.0)*t1*(dxf*(x-1.0)-dxb*x))/dz;	/* dy/dz, normally negative */
						if(j > j1) {
							if(fabs(t - dydzp[i]) > 0.001) {
								zcont[j][i] = (yline[j][i]-yline[j-1][i]+zline[j-1][i]*dydzp[i]-zline[j][i]*t) / (dydzp[i]-t);
							}
							else {
								zcont[j][i] = 0.5*(z + zline[j-1][i]);
							}
							if(j == j1+1) {	/* absolute control point required here */
								ycont[j][i] = max(0.0,yline[j1][i] + (zcont[j][i]-zline[j1][i])*dydzp[i]);
							}
						}
						dydzp[i] = t;
					}
					else {
						zcont[j][i] = z;
						if(j == j1+1 && dydzp[i] != -1.0e+10) {
							ycont[j][i] = yline[j1][i] + (z-zline[j1][i])*dydzp[i];
						}
					}

				}
				else {
					dydzp[i] = -1.0e+10;
				}
			}

			relcont[j] = (j > j1+1);
			xf += dxf;
			xb += dxb;
			z += dz;
		}

		/*	Construct the lower bounding line					*/

		for(i = surfacemode ; i < count ; i++) {
			for(j = j2-1 ; j > j1 ; j--) if(yline[j][i] >= 0.0) break;
			if(xsect[i] <= in1) {
				z = zline[j1][i];
			}
			else if(xsect[i] <= in2) {
				if(in2 > in1) {
					z = zline[j1][insec1] + (xsect[i]-in1)/(in2-in1)*span;
				}
				else {
					z = zline[j1][insec1] + ((REAL)(i-insec1))/((REAL) (insec2-insec1))*span;
				}
				if(j == j1) {
					t = (xsect[i] - in1)/(en1 - in1);
					y = yline[j1][i];
					t1 = 0.14845/sqrt(t)+(-0.126+t*(-0.7032+t*(0.8529-0.406*t)));
					t = (y*(dxb-dxf)/(en1-in1) + (naca/20.0)*t1*(dxf*(t-1.0)-dxb*t))/dz;	/* dy/dz, normally negative */
#ifndef PROF
					ycont[j1+1][i] = yline[j1][i] + (z-zline[j1][i])*t;
#else
					ycont[j2][i] = yline[j1][i] + (z-zline[j1][i])*t;
#endif
				}
			}
			else if(xsect[i] <= en2) {
				z = zline[j1][insec1] + span;
			}
			else if(xsect[i] <= en1) {
				if(en1 > en2)
					z = zline[j1][insec1] + (en1-xsect[i])/(en1-en2)*span;
				else
				    z = zline[j1][insec1] + ((REAL)(ensec1-i))/((REAL) (ensec1-ensec2))*span;
			}
			else {
				z = zline[j1][i];
			}
			zline[j2][i] = z;
			zcont[j2][i] = z;
			yline[j2][i] = 0.0;
			linewt[j2][i] = 0.0;	/* no fairing of this line */
		}

		/*	Fill vacant locations with dummy values		*/

		for(j = j1 ; j <= j2 ; j++) {
#ifdef PROF
			is = -1;
#endif
			for(i = surfacemode ; i < count ; i++) {
				if(yline[j][i] < 0.0) {
					zline[j][i] = zline[j2][i];
					zcont[j][i] = zline[j2][i];
					yline[j][i] = yline[j2][i];
					linewt[j][i] = 0.0;
#ifdef PROF
				}
				else {
					if(is < 0) is = i;
					ie = i;
#endif
				}
				if(ycont[j][i] == -1.0e+10) ycont[j][i] = 0.0;
			}
#ifdef PROF
			stsec[j] = is;
			ensec[j] = ie;
#else
			stsec[j] = 0;
			ensec[j] = count-1;
#endif
		}
		relcont[j2] = 	FALSE;
#ifdef PROF
		stsec[j2] = stsec[j1];
		ensec[j2] = ensec[j1];
#else
		stsec[j2] = 0;
		ensec[j2] = count-1;
#endif
	}

	changed = TRUE;
#ifdef EXT_OR_PROF
	save_hull(MAINHULL);
#endif
#ifdef PROF
	recalc_transom();
#endif

#ifndef PROF
	numbetw = 0;
#endif
}

/*	Add a section to the hull	*/

void add_a_section(REAL xadd)
{
	int i,j,k;
	extern int changed;

	/*	USE EXISTING LINES TO CALCULATE SPLINE-FIT OFFSETS HERE	*/

	xsect[maxsec-1] = xadd;
	for(i = extlin-1 ; i >= 0 ; i--) {
		refit(i,xsect,(REAL *) yline[i],(REAL *) zline[i],(REAL *) ycont[i],(REAL *) zcont[i],maxsec-1,maxsec-1);
	}

	changed = TRUE;

	/*		MAKE SPACE HERE	*/

	for(i = 1 ; i < count ; i++) {
		if(xsect[i] >= xsect[maxsec-1]) break;
	}

	for(j = count ; j > i ; j--) {
		xsect[j] = xsect[j-1];
#ifndef STUDENT
		strcpy(sectname[j],sectname[j-1]);
#endif
		master[j] = master[j-1];
		for(k = 0 ; k < extlin ; k++) {
			yline[k][j] = yline[k][j-1];
			zline[k][j] = zline[k][j-1];
			ycont[k][j] = ycont[k][j-1];
			zcont[k][j] = zcont[k][j-1];
			linewt[k][j] = linewt[k][j-1];
		}
	}
#ifndef STUDENT
	sprintf(sectname[i],"%d",i);
#endif

	/*		Then alter start and end section for each existing line	*/

	for(k = 0 ; k < extlin ; k++) {
		if(stsec[k] >= i) stsec[k]++;
		if(ensec[k] >= i ||
			    i == count && ensec[k] == i-1) ensec[k]++;
	}

	/*		THEN INSERT IT	*/

	xsect[i] = xsect[maxsec-1];
	for(k = 0 ; k < extlin ; k++) {
		if(stsec[k] > i || ensec[k] < i) continue;
		yline[k][i] = yline[k][maxsec-1];
		zline[k][i] = zline[k][maxsec-1];
		ycont[k][i] = ycont[k][maxsec-1];
		zcont[k][i] = zcont[k][maxsec-1];
		linewt[k][i] = 1.0;
	}

	count++;
#ifndef STUDENT
	check_invalid_end();
#endif
}		/* end non-match of existing section test*/

int index_section(REAL x,int addsec)
{
	int i;
	for(i = surfacemode ; i < count ; i++) {
		if(fabs(xsect[i]-x) < 0.0001) {
			break;
		}
		else if(xsect[i] > x) {
			if(addsec) add_a_section(x);
			break;
		}
	}
	return (i < count ? i : -1);
}

int section_error(int sec)
{
	int error = count >= maxsec || sec < 0;
	if(error) {
		message("Error attempting to locate section position.\nReload or renew original design.");
		count = 0;
	}
	return error;
}

