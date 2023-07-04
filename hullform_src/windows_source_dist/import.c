/* Hullform component - import.c
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

extern char (*sectname)[12];

void realloc_stringers(int);
void redef_transom(void);
void recalc_tanks(void);
void calc_stringers(int strlin);
extern int view_on_load;
extern int *relcont;
extern int *autofair;

#ifdef EXT_OR_PROF
extern int current_hull;
void use_hull(int);
void save_hull(int);
#endif

void load_view(void);
extern HWND hWndMain;
extern REAL Xwidth,Ywidth;
extern int editinit;
extern int changed;
void reset_all(void);
void check_negative_offsets(void);
extern int balanced;
extern int numbetw;
void fair_controls(void);
int read_input(char *text,REAL *result);

#ifdef linux
#include <Xm/Text.h>
#endif

MENUFUNC import()
{
    FILE	*fp;
    static int format = 0;
    static int firstissheer = TRUE;
    static int firstisstem = TRUE;
    static int rounded = TRUE;
    void importfunc(int code,HWND hWndDlg);
    static char text[MAX_PATH] = "";
    int i,j;
    REAL y,z;
    REAL offset[maxlin];
    int j1,j2,dj;
    int i1,i2,di;
    int n;
	extern int *editsectnum;

#ifdef EXT_OR_PROF
    use_hull(MAINHULL);
#endif
    if(getdlg(IMPORT,
	INP_RBN,&format,
	INP_LOG,&firstissheer,
	INP_LOG,&firstisstem,
        INP_RBN,&rounded,
	INP_PBF,importfunc,
	INP_INT,&count,
	INP_INT,&numlin,
	INP_STR,text,-1) && (fp = fopen(text,"rt")) != NULL) {

	realloc_hull(numlin);
	*hullfile = 0;
#ifndef linux

#ifdef HULLSTAT
	SetWindowText(hWndMain,"HULLSTAT");
#else
	SetWindowText(hWndMain,"HULLFORM");
#endif

#endif
	if(firstissheer) {
	    j2 = numlin;
	    j1 = 0;
	    dj = 1;
	} else {
	    j1 = numlin-1;
	    j2 = -1;
	    dj = -1;
	}
	if(firstisstem) {
	    i1 = 0;
	    i2 = count;
	    di = 1;
	} else {
	    i1 = count-1;
	    i2 = -1;
	    di = -1;
	}
	switch(format) {
	  case 0:	/* sections */
	    for(i = i1 ; i != i2 ; i += di) {
		if(! (fscanf(fp,"%s",text) == 1 && read_input(text,&xsect[i])) ) {
		    sprintf(text,"Out of data at start of section %d",i);
		    count = 0;
		    message(text);
		    return;
		}
		for(j = j1 ; j != j2 ; j += dj) {
		    n = fscanf(fp,"%f %f",&yline[j][i],&zline[j][i]);
		    if(n == 1) n = fscanf(fp,",%f",&zline[j][i]);
		    if(n < 1) {
			sprintf(text,"Out of data at section %d, line %d",i,j+1);
			message(text);
			count = 0;
			return;
		    }
		    zline[j][i] *= invert;
		}
	    }
	    break;

	  case 1:	/* buttocks, sections across page */
	    for(i = i1 ; i != i2 ; i += di) {
		if(fscanf(fp,"%f",&xsect[i]) < 1) {
		    sprintf(text,"Out of data reading section position %d",i);
		    count = 0;
		    message(text);
		    return;
		}
	    }
	    for(j = j1 ; j != j2 ; j += dj) {
		if(fscanf(fp,"%f",&y) < 1) {
		    sprintf(text,"Out of data reading y offset of buttock %d",j);
		    count = 0;
		    message(text);
		    return;
		}
		for(i = i1 ; i != i2 ; i += di) {
		    if(fscanf(fp,"%f",&zline[j][i]) < 1) {
			sprintf(text,"Out of data reading vertical offset %d at line %d",i,j+1);
			message(text);
			return;
		    }
		    zline[j][i] *= invert;
		    yline[j][i] = y;
		}
	    }
	    break;

	  case 2:	/* waterlines, sections across page */
	    for(i = i1 ; i != i2 ; i += di) {
		if(fscanf(fp,"%s",text) < 1 || !read_input(text,&xsect[i])) {
		    sprintf(text,"Out of data reading section position %d",i);
		    message(text);
		    return;
		}
	    }
	    for(j = j1 ; j != j2 ; j += dj) {
		if(fscanf(fp,"%s",text) < 1 || !read_input(text,&z)) {
		    sprintf(text,"Out of data reading z offset of waterline %d",j+1);
		    message(text);
		    return;
		}
		z *= invert;
		for(i = i1 ; i != i2 ; i += di) {
		    if(fscanf(fp,"%s",text) < 1 || !read_input(text,&yline[j][i])) {
			count = 0;
			sprintf(text,"Out of data reading vertical offset %d at line %d",i,j+1);
			message(text);
			return;
		    }
		    zline[j][i] = z;
		}
	    }
	    break;

	  case 3:	/* buttocks, sections down page, laterals across page */
	    for(j = j1 ; j != j2 ; j += dj) {
		if(fscanf(fp,"%f",&offset[j]) < 1) {
		    sprintf(text,"Out of data reading lateral offset %d",j+1);
		    message(text);
		    count = 0;
		    return;
		}
	    }
	    for(i = i1 ; i != i2 ; i += di) {
		if(fscanf(fp,"%f",&xsect[i]) < 1) {
		    sprintf(text,"Out of data reading section position %d",i);
		    count = 0;
		    message(text);
		    return;
		}
		for(j = j1 ; j != j2 ; j += dj) {
		    if(fscanf(fp,"%f",&zline[j][i]) < 1) {
			sprintf(text,"Out of data reading vertical offset for section %d at line %d",i,j+1);
			message(text);
			count = 0;
			return;
		    }
		    zline[j][i] *= invert;
		    yline[j][i] = offset[j];
		}
	    }
	    break;

	  case 4:	/* waterlines, sections down page, verticals across page */
	    for(j = j1 ; j != j2 ; j += dj) {
		if(fscanf(fp,"%f",&offset[j]) < 1) {
		    sprintf(text,"Out of data reading vertical offset %d",j+1);
		    count = 0;
		    message(text);
		    return;
		}
		offset[j] *= invert;
	    }
	    for(i = i1 ; i != i2 ; i += di) {
		if(fscanf(fp,"%f",&xsect[i]) < 1) {
		    sprintf(text,"Out of data reading section position %d",i);
		    count = 0;
		    message(text);
		    return;
		}
		for(j = j1 ; j != j2 ; j += dj) {
		    if(fscanf(fp,"%f",&yline[j][i]) < 1) {
			sprintf(text,"Out of data reading vertical offset for section %d at line %d",i,j+1);
			count = 0;
			message(text);
			return;
		    }
		    zline[j][i] = offset[j];
		}
	    }
	    break;
	}
	fclose(fp);
	relcont[0] = FALSE;
	stsec[0] = 0;
	ensec[0] = count-1;
	extlin = numlin;
	stemli = numlin-1;
	for(j = 1 ; j < numlin ; j++) {
	    relcont[j] = TRUE;
	    stsec[j] = 0;
	    ensec[j] = count-1;
	    for(i = 0 ; i < count ; i++) {
		ycont[j][i] = 0.0;
		zcont[j][i] = zline[j-1][i];
	    }
	}

	if(rounded) fair_controls();

#ifndef STUDENT
	realloc_stringers(0);
	ststr[0] = -1;
	ntank = 0;
	fl_line1[0] = numlin;
#endif
	surfacemode = 0;

	for(i = 0 ; i < count ; i++) {
	    sprintf(sectname[i],"%d",i);
	    master[i] = TRUE;
	}
	for(j = 0 ; j < extlin ; j++) {
	    autofair[j] = FALSE;
#ifndef STUDENT
	    numstr[j] = 0;
	    radstem[j] = 0.0;
	    strmode[j] = -1;
#endif
	    for(i = 0 ; i < count ; i++) linewt[j][i] = 1.0;

	    for(i = maxsec ; i < maxsec+2 ; i++) {
		yline[j][i] = 0.0;
		zline[j][i] = 0.0;
		ycont[j][i] = 0.0;
		zcont[j][i] = 0.0;
	    }
	}
#ifdef PLATEDEV
	for(j = 0 ; j < extlin+2 ; j++) developed[j] = -1;
#endif
	*hullfile = 0;
#ifndef linux

#ifdef HULLSTAT
	SetWindowText(hWndMain,"HULLSTAT");
#else
	SetWindowText(hWndMain,"HULLFORM");
#endif

#endif

#ifndef STUDENT
	numruled = 0;
	rtransom = 0.0;
	atransom = 0.0;
	redef_transom();
	recalc_tanks();
#endif
	reset_all();
	Xwidth = 0.0;
	Ywidth = 0.0;
	changed = 0;
	balanced = 0;
	editinit = 1;
	check_negative_offsets();
	load_view();
	for(i = 0 ; i < count ; i++) editsectnum[i] = TRUE;
	strcpy(alsosect,"ALL");
	wl = 0.0;
	density = densit[numun];
#ifdef EXT_OR_PROF
	save_hull(MAINHULL);
#endif
	numbetw = 0;
    }
}


void importfunc(int code,HWND hWndDlg)
{
    char text[MAX_PATH];
    text[0] = 0;
    if(openfile(text,"rt","Import Hull Data","text files(*.TXT)\0*.txt\0","*.txt",dirnam,NULL)) {
#ifdef linux
	XmTextSetString(wEdit[2],text);
#else
	SetDlgItemText(hWndDlg,DLGEDIT+2,text);
#endif
    }
}

void fair_controls(void)
{
    REAL yc,zc,t,y0,y1,y2,z0,z1,z2,dz01,dz12,dy01,dy12;
    REAL dy[maxlin],dz[maxlin];
    int i,j;

/*	Fit arc of circle through each point, the previous and the next	*/

/*	NOTE: PARTIAL LINES NOT SUPPORTED	*/

    relcont[1] = FALSE;

    for(i = 0 ; i < count ; i++) {
	zcont[0][i] = 0.0;
	ycont[0][i] = 0.0;
	yc = 0.0;
        zc = zline[0][i];
	for(j = 1 ; j < numlin-1 ; j++) {
	    zcont[j][i] = zline[j-1][i];	/* defaults */
	    ycont[j][i] = j == 1 ? yline[j-1][i] : 0.0;
	    y0 = yline[j-1][i];
	    y1 = yline[j  ][i];
	    y2 = yline[j+1][i];
	    z0 = zline[j-1][i];
	    z1 = zline[j  ][i];
	    z2 = zline[j+1][i];
	    dy01 = y1 - y0;
	    dy12 = y2 - y1;
	    dz01 = z1 - z0;
	    dz12 = z2 - z1;
	    dy[j] = dz12;
	    dz[j] = dy12;
	    if(dz12 != 0.0) {
		if(dz01 != 0.0) {
		    t = dy12/dz12 - dy01/dz01;
		    if(t != 0.0) {
			yc = (z2 - z0 - (y1*y1-y0*y0)/dz01 + (y2*y2-y1*y1)/dz12)/(2.0*t);
		    } else {
			yc = -10000.0;
		    }
		    zc = 0.5*(z0+z1) - dy01/dz01*(yc - 0.5*(y0+y1));
		} else {
		    yc = 0.5*(y0+y1);
		    zc = 0.5*(z1+z2) - dy12/dz12*(yc - 0.5*(y1+y2));
		}
	    } else {
		yc = 0.5*(y1+y2);
		if(dz01 != 0.0) {
		    zc = 0.5*(y0+y1) - dz01/dz01*(yc - 0.5*(y0+y1));
		} else {
		    zc = z1 - 10000.0;
		}
	    }

/*	(yc,zc) is now the centre of the arc, so the slope of the surface at (y1,z1)
	is (z1-zc)/(y1-yc). Save these terms for now.
*/
	    dy[j] = z1 - zc;	/* positive outward */
	    dz[j] = y1 - yc;	/* positive upward */
fin:	    if(j == 1) {
		dy[0] = z0 - zc;
		dz[0] = y0 - yc;
	    }
	}
	dy[numlin-1] = z2 - zc;
	dz[numlin-1] = y2 - yc;

/*	Have slopes at each point. Now locate control points	*/

	for(j = 1 ; j < numlin ; j++) {
	    t = dz[j-1]*dy[j] - dz[j]*dy[j-1];
	    if(t != 0.0) {
		zc = (dz[j-1]*dz[j]*(yline[j][i]-yline[j-1][i]) +
			zline[j][i]*dy[j]*dz[j-1] - zline[j-1][i]*dy[j-1]*dz[j])/t;

/*	yc here is the absolute control point location	*/
		if(dz[j] != 0.0)
		    yc = yline[j][i] + dy[j]/dz[j]*(zline[j][i]-zc);
		else if(dz[j-1] != 0.0)
		    yc = yline[j-1][i] + dy[j-1]/dz[j-1]*(zline[j-1][i]-zline[j][i]);
		else
		    yc = yline[j-1][i];
		z1 = zline[j][i] - zline[j-1][i];
		y1 = yline[j-1][i] - yline[j][i];
		z0 = zline[j][i] - zc;
		y0 = yc - yline[j][i];
		t = y1*y1+z1*z1;
		if(t > 0.0) {
		    t = (z0*z1+y0*y1)/t;
		    if(t >= 0.01 && t <= 0.99)
			zcont[j][i] = zc;
		    else
			zcont[j][i] = 0.5*(zline[j][i]+zline[j-1][i]);
		} else {
		    if(relcont[j])
			ycont[j][i] = 0.0;
		    else
			ycont[j][i] = 0.5*(yline[j][i]+yline[j][i]);
		    zcont[j][i] = 0.5*(zline[j][i]+zline[j-1][i]);
		}
	    }
	}
	if(dz[1] != 0.0)
	    ycont[1][i] = yline[1][i] + dy[1]/dz[1]*(zline[1][i]-zcont[1][i]);
	else {
	    ycont[1][i] = 0.5*(yline[1][i]+yline[0][i]);
	    zcont[1][i] = 0.5*(zline[1][i]+zline[0][i]);
	}
	if(zcont[numlin-1][i] > zline[numlin-1][i]) zcont[numlin-1][i] = zline[numlin-1][i];
    }
}

int read_input(char *text,REAL *result)
{
/*
    char *singq,*slash;
    REAL inches,numer,denom;
    char *inchtext,*inchend;
*/

    if(sscanf(text,"%f",result) < 1) return FALSE;
/*
    singq = strchr(text,'\'');
    if(singq != NULL) {
	inchtext = singq + 1;
	while(isspace(*inchtext)) inchtext++;
	if(inchtext != NULL) {
	    if(sscanf(inchtext,"%f",&inches) < 1) return FALSE;
	    inchend = inchtext;
	    while(!isspace(*inchend) && *inchend != 0) inchend++;
	    if(*inchend != 0) {
		slash = strchr(inchend,'/');
		if(slash != NULL) {
		    *slash = 0;
		    if(sscanf(inchend,"%f",&numer) < 1) return FALSE;
		    if(sscanf(slash+1,"%f",&denom) < 1) return FALSE;
		    if(denom <= 0.0) return FALSE;
		    inches += numer / denom;
		} else {
		    return FALSE;
		}
	    }
	    *result += inches*0.08333333;
	}
    }
*/
    return TRUE;
}


