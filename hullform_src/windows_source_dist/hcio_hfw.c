/* Hullform component - hcio_hfw.c
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
#include "graf-hfw.h"

extern	INT	device;
extern	INT	xcursor,ycursor;
extern	INT	ichan;
HDC	ScreenDC;
extern HDC hDC;
#ifndef linux
HGLOBAL	hcpyDevMode,hcpyDevNames;
#endif
void init_winprint(void);
void do_hcpy(int dev);
int graphic_screen(void);
int text_screen(void);
int errorstate = 0;
extern LOGFONT prfont;
void charsize(int *wpchar,int *hpchar);
extern int pointsize;
int saveheight;
#ifdef DEMO
#ifdef HULLSTAT
#define GRAPHICSOK
#endif
#else
#define GRAPHICSOK
#endif

/*	GRAPHICS LINE INITIALISATION	*/

void opengr(int filemode,char dir[])
{
#ifndef GRAPHICSOK
	not_avail();
#else
	extern	INT	chan;
	char	wr_err[80];
#ifdef linux
	extern	int	_fmode;
#endif
	extern	int	hcpydev;
	char	grfile[256];
	char	*p;
#ifdef linux
	char	*dirch = "/";
#else
	char	*dirch = "\\";
#endif

	errorstate = 0;

	_fmode = filemode;	/* has no effect under linux (dummy variable defined in linux_subs.c */
	if(strchr(cur_port,*dirch) != 0) {
		strcpy(grfile,cur_port);
	}
	else {
		strcpy(grfile,dir);
		p = strchr(grfile,0) - 1;
		if(*p != *dirch) strcat(grfile,dirch);
		strcat(grfile,cur_port);
	}

#ifdef linux
	chan = open(grfile,O_RDWR | O_CREAT | O_TRUNC,S_IREAD|S_IWRITE);
#else
	chan = creat(grfile,S_IREAD|S_IWRITE);
#endif
	if(chan < 0) {
		hcpydev = NOHARDCOPY;
		sprintf(wr_err,"Can not write to %s.\nOutput will be discarded",
			cur_port);
		message(wr_err);
		errorstate = 1;
	}
	waitcursor();
#endif
}

/*	close graphics channel			*/

void closgr()
{
	extern int chan;
	close(chan);
	arrowcursor();
}

/*	WRITE CHARACTER STRING DIRECT TO GRAPHICS DEVICE	*/

/*	... first form - length implict				*/

void outst(char *str)
{
	outstr(str,strlen(str));
}

/*	... second form - length explicit (necessary if sending nulls)	*/

void outstr(char *str,int l)
{
	extern int	chan;
	char   *out_err = "Output to plot device failed.\n Select OK to try again,  Cancel to abort";

	if(errorstate) return;

	while(write(chan,str,l) != l) {
		if(MessageBox(hWnd,out_err,"OUTPUT ERROR!",MB_OKCANCEL) ==
			    IDCANCEL) {
			errorstate = 1;
			break;
		}
	}
}

void await(INT time)
{
	clock_t start;
	int t100;
	char text[200];
	start = clock();
	do {
		t100 = ((clock()-start)*100)/CLOCKS_PER_SEC;
	} while(t100 < time);
}

/*	Windows system printer initialisation */

void init_winprint()
{
#ifdef linux

#else

#ifdef DEMO
	not_avail();
#else
	PRINTDLG pd;
	extern int hcpydev;
	char title[MAX_PATH];
	char *p,*l;
	HFONT hFont;
	HFONT OldFont;
	TEXTMETRIC tm;
	extern int xchar,ychar;
	DOCINFO di;

	/* Set all structure members to zero. */

	memset(&pd, 0, sizeof(PRINTDLG));
	pd.lStructSize = sizeof(PRINTDLG);
	pd.hwndOwner = hWnd;
	pd.Flags = PD_RETURNDC;
	if(PrintDlg(&pd) == 0) {
		message("Printing aborted");
		hcpydev = NOHARDCOPY;
		hardcopy = 0;
	}
	else {
		hardcopy = 1;
		ScreenDC = hDC;
		hcpyDevMode = pd.hDevMode;
		hcpyDevNames = pd.hDevNames;
		if(hcpyDevMode != NULL) GlobalFree(hcpyDevMode);
		if(hcpyDevNames != NULL) GlobalFree(hcpyDevNames);

		hDC = pd.hDC;
		saveheight = prfont.lfHeight;
		prfont.lfHeight = MulDiv(pointsize, GetDeviceCaps(hDC,LOGPIXELSY),72);
		hFont = CreateFontIndirect(&prfont);
		OldFont =  SelectObject(hDC,hFont);
		if(OldFont != NULL) DeleteObject(OldFont);
		GetTextMetrics(hDC,&tm);
		ychar = tm.tmHeight;
		xchar = tm.tmAveCharWidth;

		l = p = hullfile;
		while((p = strchr(p,'\\')) != NULL) {
			l = ++p;
		}
		sprintf(title,"HULLFORM: %s",l);

		di.cbSize = sizeof(di);
		di.lpszDocName = title;
		di.lpszOutput = NULL;
    	di.lpszDatatype = "Hullform Graphic";
		di.fwType = 0;

		StartDoc(hDC,&di);
		StartPage(hDC);
		waitcursor();
		hcpy_summ();/* show hull info on hardcopy */
	}
#endif
#endif
}

void do_print()
{
#ifdef SHAREWARE
	nagbox();
#endif
	do_hcpy(1);
}

#ifdef EXT_OR_PROF
void do_plot()
{
	extern int default_hardcopy_device;
#ifdef SHAREWARE
	nagbox();
#endif
	if(default_hardcopy_device) do_hcpy(default_hardcopy_device);
}
#endif

void do_hcpy(int dev)
{
	extern int hcpydev,hardcopy,default_hardcopy_device;
	int row,column,length,line;
	extern char *text_device;
	extern int windows_print;
	extern int xchar,ychar,xjust;
	char *p,*dot,*text;
	extern	LOGFONT prfont;	/* logical font structure */
	SIZE size;
	extern int fixed_scale;
	int save_scrdev;
	extern int ymaxi;
#ifdef linux
	extern FILE *psf;
	void add_escapes(char *str,char text[]);
	char textdata[500];
#endif
	REAL xmx = xmax,

	xmn = xmin,
	ymx = ymax,
	ymn = ymin;

	save_scrdev = scrdev;
	scrdev = dev;
	setup(dev);

	if(graphic_screen()) {
		waitcursor();
		hardcopy = 1;

#ifdef linux

#ifdef EXT_OR_PROF
		if(dev == 1) dev = 7;	/* 7 points to postscript file driver */
#else
		if(dev == 1) dev = 2;	/* 2 points to postscript file driver */
#endif

#else

#ifndef DEMO
		if(dev == 1) init_winprint();	/* device 1 is windows printer */
#endif

#endif
		if(hardcopy) {	/* hardcopy is cancelled if print setup is aborted */
			hcpydev = dev;
#ifdef DEMO
			if(init_device(dev)) message("Demonstration version:\nno actual output");
#else
			if(init_device(dev) && print_func != NULL) (*print_func)();
#endif
		}
		arrowcursor();
	}

#ifndef DEMO
	else if(text_screen()) {
		if(dev == 1) {
			waitcursor();
#ifdef linux
			hardcopy = 1;
			postin();
#else
			init_winprint();		/* device 1 is windows printer */
#endif
			if(hardcopy) {
#ifdef linux
				column = 72;
				ychar = pointsize;
				row = ymaxi - 72 - ychar;
#else
				column = GetDeviceCaps(hDC,LOGPIXELSX);	/* 1 inch indent */
				row = GetDeviceCaps(hDC,LOGPIXELSY);	/* 1 inch indent */
#endif
				for(line = 0 ; line < 22 ; line++) {
					text = scr_ch[line];
					length = strlen(text);
					if(length > 0) {
						if((p = strchr(text+12,'.')) != NULL) {
							dot = p;
							while(*--p != ' ') ;
							p++;
							length = dot - p;
#ifdef linux
							length *= (6*ychar)/10;
							if(*p == '-') length -= (3*ychar)/10;	// a minus sign is shorter

//	Presume fixed width (fair for numbers)

							add_escapes(p,textdata);
							fprintf(psf,"%d %d M\n(%s) T\n",column + xjust*(6*ychar)/10 - length,row,textdata);
#else
							GetTextExtentPoint(hDC,p,length,&size);
							TextOut(hDC,
								column+xchar*xjust - size.cx,
								row,
								p,
								strlen(p));
#endif
							while(*--p == ' ' && p != text) ;
							length = p - text + 1;
						} else {
							p = strchr(text,0);
						}
#ifdef linux
						*++p = 0;
						add_escapes(text,textdata);
						fprintf(psf,"%d %d M\n(%s) T\n",column,row,textdata);
						*p = ' ';
#else
						TextOut(hDC,column,row,text,length);
#endif
					}
#ifdef linux
					row -= ychar;
#else
					row += ychar;
#endif
				}
			}
			arrowcursor();
#ifdef linux
			posten();
#endif
		}
	}
	else {
		message("Hardcopy from this screen not allowed");
		return;
	}
#endif

#ifndef linux

	if(hardcopy
#ifdef EXT_OR_PROF
	&& dev == 1
#endif
	) {
		EndPage(hDC);
		EndDoc(hDC);
		DeleteDC(hDC);
		hDC = ScreenDC;
		prfont.lfHeight = saveheight;
	}

#else

#endif
	hardcopy = 0;
	xmax = xmx;
	xmin = xmn;
	ymax = ymx;
	ymin = ymn;
	fixed_scale = FALSE;
	scrdev = save_scrdev;
	setup(scrdev);
	(*init)();
}

int graphic_screen()
{
	FUNC_PTR *p;

	/*	Permitted graphic hardcopy screens */

	extern void general_orth(),plan_orth(),elev_orth(),end_orth(),
	full_persp(),port_persp(),starb_persp();
#ifdef PROF
	extern void plot_hydros(),show_areas(),plot_existing_drag(),
	view_devel(),rollout(),rollout_transom(),
	shell_expansion(),
	calibration();
#endif

	static FUNC_PTR grok[] = {
		general_orth,
		plan_orth,
		elev_orth,
		end_orth,
		full_persp,
		port_persp,
		starb_persp,
#ifdef PLATEDEV
		view_devel,
		rollout,
#endif
#ifdef PROF
		show_areas,
		plot_hydros,
		plot_existing_drag,
		rollout_transom,
		shell_expansion,
		calibration,
#endif
		NULL	};

	p = grok;
	while(*p != NULL) {
		if(*p++ == print_func) return(1);
	}
	return(0);
}

int text_screen()
{
	FUNC_PTR *p;

	extern void	find_float(),find_displ();
#ifndef STUDENT
	extern void find_gerrit(),find_holtrop(),find_oortmerrssen(),
	find_savitsky(),find_savbro(),calibration(),balance_all();
#endif
	static FUNC_PTR prok[] = {
		find_float,
		find_displ,
#ifndef STUDENT
		balance_all,
		find_gerrit,
		find_oortmerrssen,
		find_holtrop,
		find_savitsky,
		find_savbro,
		calibration,
#endif
		NULL	};
	p = prok;
	while(*p != NULL) {
		if(*p++ == print_func) return(1);
	}
	return(0);
}

void charsize(int *w,int *h)
{
	*h = abs(prfont.lfHeight);
	*w = abs(prfont.lfWidth);
	if(*w == 0) *w = 10;
	if(*h == 0) *h = 16;
}
