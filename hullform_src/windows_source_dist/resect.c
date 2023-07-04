/* Hullform component - resect.c
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
 
/*	RE-SECTION HULL	*/

#include "hulldesi.h"

#ifdef linux

#include <X11/keysym.h>
#include <Xm/Text.h>
#include <X11/Intrinsic.h>

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

#endif

void reset_all(void);
void save_hull(int);

extern int changed;
int resect_option = 0; /* 0 means retain, 1 means interpolate, 2 means specify */
int newcou;
REAL xnew[maxsec];
int i;
REAL yprev;
int iprev;
int is,ns;
int istem;
REAL dxs,xx;
extern char (*sectname)[12];
#ifndef STUDENT
void check_invalid_end(void);
#endif

#ifdef linux
Widget Wdialog;
Widget newEDITRESE(
Widget wLabel[],Widget wEdit[],	Widget wRadioButton[],Widget wCheckBox[],Widget wPushButton[],
Widget wListBox[],Widget wComboBox[],Widget *wOk,Widget *wCancel,Widget *wHelp);
extern Widget wLabel[20],wEdit[20],wCheckBox[20],wPushButton[20],wListBox[20],
	wRadioButton[20],wScrollText[20],wComboBox[20];

#else

#endif

void intefunc(int code,HWND hWndDlg)
{
	char textdata[60];
	int i,j,n;
	REAL x, dx;
#ifdef linux
	char *text;
	XmString xstr;
	int *pos;
#else
	int pos[1];
#endif
	REAL prev;

	switch(code) {
	  case 0:	/* uniform intervals from retained stem */
#ifdef linux
		sscanf( (text = XmTextGetString(wEdit[0])) ,"%d",&n);
		XtFree(text);
		XmListDeleteAllItems(wListBox[0]);
#else
		n = GetDlgItemInt(hWndDlg,DLGEDIT+0,&i,0);
		if(i == 0) n = 0;
		SendDlgItemMessage(hWndDlg,DLGLBX+0,LB_RESETCONTENT,0,0l);
#endif
		if(n <= 1) break;
		dx = (xsect[count-1] - xsect[0]) / (REAL) (n-1);
		x = xsect[0];
		if(dx > 0.0) {
			for(i = 0 ; i < n ; i++) {
				sprintf(textdata," %d at %.4f",i,x);
				xnew[i] = x;
#ifdef linux
				xstr = XmStringCreateSimple(textdata);
				XmListAddItem(wListBox[0],xstr,0);
				XmStringFree(xstr);
#else
				SendDlgItemMessage(hWndDlg,DLGLBX+0,LB_ADDSTRING,0,(LPARAM) (LPCSTR) textdata);
#endif
				x += dx;
			}
		}
		newcou = n;
		resect_option = 0;
		break;
	  case 1:	/* uniform intervals and interpolated positions in stem */
#ifdef linux
		sscanf( (text = XmTextGetString(wEdit[0])) ,"%d",&n);
		XtFree(text);
		XmListDeleteAllItems(wListBox[0]);
#else
		n = GetDlgItemInt(hWndDlg,DLGEDIT+0,&i,0);
		if(i == 0) n = 0;
		SendDlgItemMessage(hWndDlg,DLGLBX+0,LB_RESETCONTENT,0,0l);
#endif
		if(n <= numlin) {
			message("You need a section count greater than the line count, to accommodate sections within the stem.");
			break;
		}
		prev = -1.0e+30;
		i = 0;
		for(j = 0 ; j < numlin-1 ; j++) {
#ifdef PROF
			if(stsec[j] > 0) continue;
#endif
			x = xsect[0] - yline[j][0];
			if(x > prev + 0.0002) {
				xnew[i] = x;
				sprintf(textdata," %d at %.4f (start line %d)",i++,x,j+1);
#ifdef linux
				xstr = XmStringCreateSimple(textdata);
				XmListAddItem(wListBox[0],xstr,0);
				XmStringFree(xstr);
#else
				SendDlgItemMessage(hWndDlg,DLGLBX+0,LB_ADDSTRING,0,(LPARAM) (LPCSTR) textdata);
#endif
				prev = x;
			}
		}
		x = prev;
		if(n > i+1) {
			dx = (xsect[count-1] - prev) / (REAL) (n-i);
			while(i < n) {
				x += dx;
				xnew[i] = x;
				sprintf(textdata," %d at %.4f",i++,x);
#ifdef linux
				xstr = XmStringCreateSimple(textdata);
				XmListAddItem(wListBox[0],xstr,0);
				XmStringFree(xstr);
#else
				SendDlgItemMessage(hWndDlg,DLGLBX+0,LB_ADDSTRING,0,(LPARAM) (LPCSTR) textdata);
#endif
			}
		}
		newcou = n;
		resect_option = 1;
		break;
	  case 2:	/* add position */
#ifdef linux
		i = sscanf( (text = XmTextGetString(wEdit[2])),"%f",&x);
#else
		GetDlgItemText(hWndDlg,DLGEDIT+2,textdata,sizeof(textdata));
		i = sscanf(textdata,"%f",&x);
#endif
		if(i >= 1) {
			sprintf(textdata," %d at %.4f",0,xnew[0]);
#ifdef linux
			XmListDeleteAllItems(wListBox[0]);
			xstr = XmStringCreateSimple(textdata);
			XmListAddItem(wListBox[0],xstr,0);
			XmStringFree(xstr);
#else
			SendDlgItemMessage(hWndDlg,DLGLBX+0,LB_RESETCONTENT,0,0l);
			SendDlgItemMessage(hWndDlg,DLGLBX+0,LB_ADDSTRING,0,(LPARAM) (LPCSTR) textdata);
#endif
			for(i = 1 ; i < newcou ; i++) {
				if(x < xnew[i] && x > xnew[i-1]) {
					for(j = newcou ; j > i ; j--) xnew[j] = xnew[j-1];
					xnew[i] = x;
					x = 1.0e+30;
					newcou++;
				}
				sprintf(textdata," %d at %.4f",i,xnew[i]);
#ifdef linux
				xstr = XmStringCreateSimple(textdata);
				XmListAddItem(wListBox[0],xstr,0);
				XmStringFree(xstr);
#else
				SendDlgItemMessage(hWndDlg,DLGLBX+0,LB_ADDSTRING,0,(LPARAM) (LPCSTR) textdata);
#endif
			}
		}
		sprintf(textdata,"%d",newcou);
#ifdef linux
		XtFree(text);
		XmTextSetString(wEdit[0],textdata);
#else
		SetDlgItemText(hWndDlg,DLGEDIT +0,textdata);
#endif
		break;

	  case 3:	/* delete selections */
#ifdef linux
		XmListGetSelectedPos(wListBox[0],&pos,&n);
#else
		pos[0] = SendDlgItemMessage(hWndDlg,DLGLBX+0,LB_GETCURSEL,0,0L) + 1;
		n = 1;
#endif
		if(pos[0] > 0) {
			for(j = 0 ; j < n ; j++) {
				for(i = pos[j] ; i < newcou ; i++) xnew[i-1] = xnew[i];
				newcou--;
			}
#ifdef linux
			XmListDeleteAllItems(wListBox[0]);
#else
			SendDlgItemMessage(hWndDlg,DLGLBX+0,LB_RESETCONTENT,0,0l);
#endif
			for(i = 0 ; i < newcou ; i++) {
				sprintf(textdata," %d at %.4f",i,xnew[i]);
#ifdef linux
				xstr = XmStringCreateSimple(textdata);
				XmListAddItem(wListBox[0],xstr,0);
				XmStringFree(xstr);
#else
				SendDlgItemMessage(hWndDlg,DLGLBX+0,LB_ADDSTRING,0,(LPARAM) (LPCSTR) textdata);
#endif
			}
#ifdef linux
			free(pos);
			sprintf(textdata,"%d",newcou);
			XmTextSetString(wEdit[0],textdata);
#else
			SetDlgItemInt(hWndDlg,DLGEDIT+0,newcou,0);
#endif
		}
		break;
	  case 4:	/* reset */
#ifdef linux
		XmListDeleteAllItems(wListBox[0]);
#else
		SendDlgItemMessage(hWndDlg,DLGLBX+0,LB_RESETCONTENT,0,0l);
#endif
		for(i = 0 ; i < count ; i++) {
			sprintf(textdata," %d at %.4f",i,xsect[i]);
#ifdef linux
			xstr = XmStringCreateSimple(textdata);
			XmListAddItem(wListBox[0],xstr,0);
			XmStringFree(xstr);
#else
			SendDlgItemMessage(hWndDlg,DLGLBX+0,LB_ADDSTRING,0,(LPARAM) (LPCSTR) textdata);
#endif
		}
		break;
	}
}

MENUFUNC edit_alte()
{
	extern int 	ychar;
	extern int	newcount;
	extern int	ybottom;
	extern char *ignline;
	extern int 	*ignore;
	char text[MAX_PATH];
	int i;
	struct {
		int index;				/* index of result in table */
		char *string;			/* pointer to result */
		char *table[maxsec+1];	/* table of strings for listbox, null string terminator */
	} xlist;
	char xtable[maxsec][20];

	message("IT IS STRONGLY RECOMMENDED THAT YOU SAVE\nYOUR CURRENT HULL DATA SET BEFORE PROCEEDING.\nALL EXPLICIT SECTION DATA YOU HAVE WILL\nPROBABLY BE LOST.\n\nPress [F2] to save the hull before proceeding");

	newcou = count;
	is = surfacemode;
	for(i = 0 ; i < count ; i++) {
		sprintf(xtable[i]," %d at %.4f",i,xsect[i]);
		xlist.table[i] = xtable[i];
		xnew[i] = xsect[i];
	}
	newcou = count;
	xlist.table[count] = "";
	xlist.string = text;

	if(getdlg(newEDITRESE,
		INP_LBX,&xlist,
		INP_PBF,intefunc,
		INP_INT,(void *) &newcou,
		INP_STR,(void *) ignline,
		-1) && multproc(ignline,ignore,maxsec) && newcou > 1 && newcou < maxsec) {

		edit_rese_perform();	/* This uses the xnew[] values maintained in intefunc */
#ifndef STUDENT
		recalc_transom();
#endif
		changed = 1;

#ifdef EXT_OR_PROF
		save_hull(MAINHULL);
#endif
	}
}

/*	This dialog initialises the section positions within the
	interpolated stem
*/

MENUFUNC edit_rese_perform()
{
	int	i,j;
	int	ii,jj;
	int	newmaster[maxsec];
	int	inisec,endsec;
	REAL	dxs,xs,dx,xe,xxstem;
	REAL	ystem = yline[stemli][1];
	INT	linnum;
	REAL	c;
	REAL	a,hb,hd,aa,cc,t1,t2;
#ifndef STUDENT
	REAL	stemrad,s,t;
#endif
	REAL	y[maxsec],z[maxsec],yc[maxsec],zc[maxsec];
	int	stnew[maxlin],ennew[maxlin];

/*	Save a copy of the stem section, to use in extending the stem line */

	for(j = 0 ; j < extlin ; j++) {
	    yline[j][maxsec+2] = yline[j][0];
	    zline[j][maxsec+2] = zline[j][0];
	    ycont[j][maxsec+2] = ycont[j][0];
	    zcont[j][maxsec+2] = zcont[j][0];
	}
	xxstem = xsect[0];

/*	Set section names to numeric values	*/

	for(i = 0 ; i < maxsec ; i++) sprintf(sectname[i],"%d",i);

/*	THEN FOR EACH HULL LINE ...	*/

	for(linnum = 0 ; linnum < extlin ; linnum++) {

/*	Set the initial x-position to be at the forward end of the line */

	    xs = xsect[stsec[linnum]] - 0.00001;
	    if(! surfacemode && stsec[linnum] == 0) xs -= yline[linnum][0];
	    xe = xsect[ensec[linnum]] + 0.00001;

/*	Identify start end end sections in the new section set	*/


	    if(surfacemode)
			inisec = 0;
	    else
			inisec = -1;
		while(xnew[++inisec] < xs) ;
	    endsec = newcou;
	    while(xnew[--endsec] > xe) ;

/*	SPLINE-FIT THE OLD CURVES TO THE NEW POINTS	*/

	    refit(linnum,xnew,y,z,yc,zc,inisec,endsec);

/*	Spline-fit the line weights, using the line weights and
	zero end-curve factors
*/
	    spline(&xsect[stsec[linnum]],&linewt[linnum][stsec[linnum]],
			&linewt[linnum][stsec[linnum]],
			ensec[linnum]-stsec[linnum]+1,
			&xnew[inisec],&linewt[linnum][inisec],endsec-inisec+1,
			0.0,0.0);
	    if(endsec <= inisec) {
			endsec = inisec+1;
			if(endsec >= newcou) {
			    endsec = newcou - 1;
			    inisec = endsec - 1;
			}
	    }
	    stnew[linnum] = inisec;
	    ennew[linnum] = endsec;

	    if(!surfacemode && resect_option == 0 && inisec == 0) inisec = 1;
	    for(i = inisec ; i <= endsec ; i++) {
			yline[linnum][i] = y [i];
			zline[linnum][i] = z [i];
			ycont[linnum][i] = yc[i];
			zcont[linnum][i] = zc[i];
	    }
	}

	for(j = 0 ; j < numlin ; j++) {
#ifdef PROF
	    stsec[j] = stnew[j];
	    ensec[j] = ennew[j];
#else
	    stsec[j] = 0;
	    ensec[j] = newcou-1;
#endif
	}

/*	Locate the old master sections at the nearest new sections */

	for(i = 0 ; i < newcou ; i++) newmaster[i] = 0;
	for(i = 0 ; i < count ; i++) {
	    if(master[i]) {
			j = 0;
			t1 = 1.0e+30;
			for(ii = 0 ; ii < newcou ; ii++) {
			    t2 = fabs(xnew[ii] - xsect[i]);
			    if(t2 < t1) {
				t1 = t2;
				j = ii;
			    }
			}
			newmaster[j] = 1;
	    }
	}

	for(i = 0 ; i < newcou ; i++) {
	    xsect[i] = xnew[i];
	    master[i] = newmaster[i];
	}
#ifndef STUDENT
	check_invalid_end();
#endif

/*	UPDATE THE SECTION COUNT	*/

	count = newcou;

/*	Make section zero of zero size if stem has been removed	*/

	if(resect_option == 1 || (!surfacemode && resect_option == 2 && xnew[1] < xxstem)) {
	    for(j = 0 ; j < extlin ; j++) {
			yline[j][0] = 0.0;
			zline[j][0] = zline[0][0];
			ycont[j][0] = 0.0;
	    }

/*	Carry the stem line forward to the stem head	*/

	    j = 0;
	    aa = 0.0;
	    cc = 1.0;
	    for(i = 0 ; i < stsec[stemli] ; i++) {
			dx = xxstem - xsect[i];
			while(yline[j][maxsec+2] > dx && j < numlin-1) {
			    hullpa(maxsec+2,++j,aa,cc,&a,&hb,&c,&hd);
			    tranpa(a,hb,c,hd,&aa,&cc);
			}

			if(j < numlin) {
			    dxs = yline[j][maxsec+2] - dx;
			    inters(a,hb,c,hd,1.0,0.0,dxs,&t1,&t2);
			    if(t1 < -0.00001 || t1 > 1.00001) t1 = t2;
			    if(t1 >= -0.00001 && t1 <= 1.00001) {
				yline[stemli][i] = ystem;
				zline[stemli][i] = zline[j][maxsec+2] - t1*(c + t1 * hd);
				jj = stemli-1;
				while(jj > 0 && stsec[jj] > i) jj--;
				ycont[stemli][i] = relcont[stemli] ? 0.0 : yline[jj][i];
				zcont[stemli][i] = zline[jj][i];
		    }
		}

/*	if the next section is within the stem radius, cancel the radius */

#ifndef STUDENT
		stemrad = radstem[j];
		if(fpos(stemrad)) {
		    get_int_angle(j,&c,&s);
		    t = fmul(stemrad,fsub(1.0,c));
			if(s != 0.0) {
				t = fdiv(t,s);
		    } else {
				t = stemrad;
			}
		    if(flt(xsect[i+1],fadd(xsect[i],t))) {
				radstem[j] = 0.0;
			} else {
				zcont[stemli][i] = 0.0;
		    }
		}
#endif
	    }
	    stsec[stemli] = 0;
		reset_all();
	}
}

