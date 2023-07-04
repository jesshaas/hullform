/* Hullform component - conf_men.c
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
#include <Xm/Xm.h>
#include <Xm/Text.h>
#include <X11/Intrinsic.h>
Widget MakeDialog(Widget Wshell,char *DialogName,int xloc,int yloc,int width,int height);
Widget MakeDialogShell(char *title);

// This is a bug workaround

// const unsigned short int *__ctype_b;

#endif

extern	LOGFONT prfont;	/* logical font structure */
#ifdef linux
extern LOGFONT lf;
extern int ToolButtonIndex[40];
extern char ToolButtonHint[][40];
extern int ToolButtonTag[];
extern char browser[];
#else
extern	LOGFONT lf;	/* logical font structure */

struct  ffblk   {
	long            ff_reserved;
	long            ff_fsize;
	unsigned long   ff_attrib;
	unsigned short  ff_ftime;
	unsigned short  ff_fdate;
	char            ff_name[256];
};

int findfirst(char *fullname,struct ffblk *blk,unsigned mode);

#endif

extern int GL_surfaces_need_redraw;
extern int GL_wireframe_need_redraw;
extern int temp_gl;

extern int *editsectnum;
void setfont(LOGFONT *lf);

PRINTDLG pd;
extern int pointsize;
#ifndef linux
HDC GetPrDC(void);
#endif
void	colfunc(int colournum,HWND hWndDlg);
int	colournum = 0;
int assistant;
int editlines = 10;
REAL elimit = 0.01;
int num_dp = 3;

/*	Many of these are not used in lower versions, but are
retained in the program for file compatibility
*/
extern int solidshade;

extern int changed;
extern int hcpydev;
extern REAL Xfactor,Yfactor;
extern REAL pagewidth;
extern int scunit;

extern int addarg[];
extern int reload[];
extern int numprog;

extern REAL xdorigin,xdslope,ydorigin,ydslope;

extern int showtanks,showcentres,showstbd,showport;

extern char (*progtext)[17];
extern char (*progname)[MAX_PATH];
extern char (*tankdesc)[MAX_PATH];

extern char  	*alsosect;
extern char	*showline;

extern	int	numbetw;
extern	int	numbetwl;
extern int numtool;

extern int *Bitmap;
extern char (*tbtext)[33];
extern int linewidth[];
extern REAL mousehole_radius,notch_width,notch_height;
extern int mouseholes;
extern int view_on_load;
extern int HintDelay;

extern UINT *tool_id;
int MapType;
extern int toolloc;

extern int dragscheme[];
extern int linestyle[];

extern int outl_places;
extern REAL outl_inc;
extern REAL def_outl_thickness;

#ifdef EXT_OR_PROF
extern int dxf_maxlayer,dxf_layer_numeric,dxf_dim;
#else
int dxf_maxlayer = 0;
int dxf_layer_numeric = 0;
int dxf_dim = 0;
#endif
extern REAL def_str_width,def_str_thick;
extern int def_str_direc;
extern REAL def_str_interv,def_str_firstint;
extern int showstringers,showcentres;

extern REAL yview,zview;
extern int showframes,shownumbers,showsections,shownames;
extern REAL rotn,l_azim,l_elev;
extern REAL speed,prop_diam,pd_ratio;
extern int twin_screw;
extern REAL epsilon,dist_down;
extern int tran_lines;
extern REAL plot_start,plot_end,plot_inc;
#ifdef PROF
extern int xvar,yvar,tvar;
#else
int xvar = 0,yvar = 0,tvar = 0;
#endif
extern int tabusel[MAXSEL];
extern int stabvar;
extern int numun,inches;
extern RECT rc_startup;

void browse_directories(int code,HWND hWndDlg);
void browse_plot_directory(int code,HWND hWndDlg);
void striptopath(char *path);
static char conftext[MAX_PATH];

char *rhead(FILE *fp,char *header) {
	char *p;
	static char zero = 0;
	if(fgets(conftext,sizeof(conftext),fp) != NULL) {
		if( (p = strchr(conftext,'\n')) != NULL) *p = 0;
		if( (p = strchr(conftext,'\r')) != NULL) *p = 0;
		if(strncmp(header,conftext,strlen(header)) == 0) return conftext+26;
	}
	return &zero;
}

//	Read multiple integers from a text line

int readints(char lin[],int table[],int maxtab)
{
	int	i;
	int n = 0;
	int nread(char **str);
	char *line = (char *) lin;

	while(*line) {
		i = *line;
		if(isspace(i) || i == ',') {
			line++;
		}
		else if(i >= '0' && i <= '9') {
			i = nread(&line);
			if(i < 0) break;
			table[n++] = i;
			if(n == maxtab) break;
		}
		else {
			break;	// invalid character (most often "-")
		}
	}
	return n;
}

MENUFUNC get_config()
{
	extern INT	default_hardcopy_device;
	extern COLORREF scrcolour[];
	extern HDC	hDC;
	extern REAL	invert,posdir;
	extern REAL	ypersp,zpersp;
	extern int	xright,xmaxi;
	int		i,j,k;
	FILE	*fp;
	extern char *cfgfile;
	extern int	xchar,ychar;
	void fixfn(char str[]);
	int readints(char lin[],int table[],int maxtab);
	char buffer[500];

#ifndef linux
	extern RECT rc_startup;
#endif
	LOGFONT	*lp;
	char *p,*e;
	int itemp[10];
	char text[MAX_PATH];

	//	Set default values

	ypersp = 0.0;
	zpersp = 99999.0;
	strcpy(port,"PRN");
	default_hardcopy_device = 0;
	numtool = 0;
	numun = 1;
	inches = 1;
	strcpy(dirnam,".");
	strcpy(filedirnam,".");
	strcpy(dxfdirnam,".");
	invert = 1.0;
	posdir = 1.0;
	spgrav = 1.025;
	Xfactor = 1.0;
	Yfactor = 1.0;
	pagewidth = 210.0;
	scunit = 0;
	numbetw = 0;
	numbetwl = 9;
	numtool = 0;
#ifdef linux
	strcpy(lf.lfFaceName,"-adobe-helvetica-medium-r-normal--14-140-75-75-p-77-iso8859-1");
#endif

	//	read configuration file

	if(*cfgfile != '\0') {
		fp = fopen(cfgfile,"rt");
		if(fp != NULL) {
			if(sscanf(rhead(fp,"Plot device code          "),"%d",&default_hardcopy_device) != 1) goto error;
			if((i = sscanf(rhead(fp,"Plot file name            "),"%s",port)) < 1) goto error;
			if(sscanf(rhead(fp,"Measurement unit code     "),"%d",&numun) != 1) goto error;
			if(sscanf(rhead(fp,"Show inches with feet     "),"%d",&inches) != 1) goto error;

			p =       rhead(fp,"Hull data directory       ");
			if(*p == 0) goto error;
			strcpy(dirnam,p);
			if( (p = strchr(dirnam,'\n')) != NULL) *p = 0;

			p =       rhead(fp,"Text output directory     ");
			if(*p == 0) goto error;
			strcpy(filedirnam,p);
			if( (p = strchr(filedirnam,'\n')) != NULL) *p = 0;

			p =       rhead(fp,"DXF output directory      ");
			if(*p == 0) goto error;
			strcpy(dxfdirnam,p);
			if( (p = strchr(dxfdirnam,'\n')) != NULL) *p = 0;

			if(sscanf(rhead(fp,"Vertical display factor   "),"%f",&invert) != 1) goto error;
			if(sscanf(rhead(fp,"Horizontal display factor "),"%f",&posdir) != 1) goto error;
			if(sscanf(rhead(fp,"Water specific gravity    "),"%f",&spgrav) != 1) goto error;
			if(sscanf(rhead(fp,"Points between sections   "),"%d",&numbetw) != 1) goto error;
			if(sscanf(rhead(fp,"Points between lines      "),"%d",&numbetwl) != 1) goto error;
			if(sscanf(rhead(fp,"Plot shades non-dithered  "),"%d",&solidshade) != 1) goto error;

			if(sscanf(rhead(fp,"Screen colours            "),"%x %x %x %x %x %x %x %x %x\n",
				&scrcolour[0],&scrcolour[1],&scrcolour[2],&scrcolour[3],&scrcolour[4],
				&scrcolour[5],&scrcolour[6],&scrcolour[7],&scrcolour[8]) != 9) goto error;
			p = rhead(fp,"Widths and styles         ");
			if(*p == 0) goto error;
			j = readints(p,itemp,10);
			if(j >= 10) {
				for(j = 0, k = 0 ; k < 5 ; k++) {
					linestyle[k] = itemp[j++];
					linewidth[k] = itemp[j++];
				}
			}

			if(sscanf(rhead(fp,"Output page width         "),"%f",&pagewidth) != 1) goto error;
			if(sscanf(rhead(fp,"Page width unit code      "),"%d",&scunit) != 1) goto error;
			if(sscanf(rhead(fp,"Page X-scale factor       "),"%f",&Xfactor) != 1) goto error;
			if(sscanf(rhead(fp,"Page Y-scale factor       "),"%f",&Yfactor) != 1) goto error;

			/*	The screen font is X-specific, but we use Windows format for the printer font	*/

			lp = &lf;
#ifdef linux
			if(sscanf(rhead(fp,"Screen font               "),"%s",lp->lfFaceName) != 1) goto error;
#else
			if(sscanf((p = rhead(fp,"Screen font               ")),"%d %d %d %d %d %d %d %d %d %d %d %d %d ",
				&lp->lfHeight,&lp->lfWidth,&lp->lfEscapement,
				&lp->lfOrientation,&lp->lfWeight,&lp->lfItalic,
				&lp->lfUnderline,&lp->lfStrikeOut,&lp->lfCharSet,
				&lp->lfOutPrecision,&lp->lfClipPrecision,&lp->lfQuality,
				&lp->lfPitchAndFamily) < 13) goto error;
			p = strchr(p,':');
			if(p != NULL) strcpy(lp->lfFaceName,p+2);
#endif
			lp = &prfont;
			if(sscanf((p = rhead(fp,"Printer font              ")),"%d %d %d %d %d %d %d %d %d %d %d %d %d ",
				(int *) &lp->lfHeight,(int *) &lp->lfWidth,(int *) &lp->lfEscapement,
				(int *) &lp->lfOrientation,(int *) &lp->lfWeight,(int *) &lp->lfItalic,
				(int *) &lp->lfUnderline,(int *) &lp->lfStrikeOut,(int *) &lp->lfCharSet,
				(int *) &lp->lfOutPrecision,(int *) &lp->lfClipPrecision,(int *) &lp->lfQuality,
				(int *) &lp->lfPitchAndFamily) < 13) goto error;
			p = strchr(p,':');
			if(p != NULL) strcpy(lp->lfFaceName,p+2);

			pointsize = (abs(prfont.lfHeight)*3)/4;

			if(sscanf(rhead(fp,"Number of menu programs   "),"%d",&numprog) != 1) goto error;
			for(j = 0 ; j < numprog ; j++) {
				if( *(p = rhead(fp,"Program caption           ")) == 0) goto error;
#ifdef PROF
				strcpy(progtext[j],p);
#endif
				if( *(p = rhead(fp,"Program name              ")) == 0) goto error;
#ifdef PROF
				strcpy(progname[j],p);
#endif
#ifdef PROF
				if(sscanf(rhead(fp,"Argument / reload         "),"%d %d",&addarg[j],&reload[j]) != 2) goto error;
#else
				p = rhead(fp,"Argument / reload         ");
#endif
			}

			if(sscanf(rhead(fp,"Toolbar count, hint delay "),"%d %d",&numtool,&HintDelay) < 1) goto error;
			if(sscanf(rhead(fp,"Toolbar location code     "),"%d",&toolloc) != 1) goto error;
			for(j = 0 ; j < numtool ; j++) {
				p = rhead(fp,"Button");
				if(*p == 0) goto error;
#ifdef linux
				if(sscanf(p,"%d %d",&ToolButtonTag[j],&ToolButtonIndex[j]) < 2) goto error;
				if( (p = strchr(p,'|')) == NULL) goto error;
				if(*++p == ' ') p++;
				strcpy(ToolButtonHint[j],p);
#else
				if(sscanf(p,"%d %d",&tool_id[j],&Bitmap[j]) < 2) goto error;
				e = strchr(p,'\n');
				if(e != NULL) *e = 0;
				p = strchr(p,'|');
				if(p != NULL)
					strcpy(tbtext[j],++p);
				else
				    strcpy(tbtext[j],"");
#endif
			}

			if(*rhead(fp,"Previous four hull data files") == 0) goto error;
			for(j = 0 ; j < 4 ; j++) {
				sprintf(text,"File %2d                   ",j+1);
				strcpy(text,rhead(fp,text));
				if( (p = strchr(text,'\n')) != NULL) *p = 0;
				p = strchr(text,':');
				if(p == NULL) goto error;
				*p = 0;
				strcpy(lastname[j],text);
				strcpy(lastpath[j],++p);
			}
			if(sscanf(rhead(fp,"Builders outline places   "),"%d",&outl_places) != 1) goto error;
			if(sscanf(rhead(fp,"Skin thickness            "),"%f",&def_outl_thickness) != 1) goto error;
			if(sscanf(rhead(fp,"Hole radius               "),"%f",&mousehole_radius) != 1) goto error;
			if(sscanf(rhead(fp,"Notch width               "),"%f",&notch_width) != 1) goto error;
			if(sscanf(rhead(fp,"Notch height              "),"%f",&notch_height) != 1) goto error;
			if(sscanf(rhead(fp,"Distance between points   "),"%f",&outl_inc) != 1) goto error;
			if(sscanf(rhead(fp,"DXF output layers         "),"%d",&dxf_maxlayer) != 1) goto error;
			if(sscanf(rhead(fp,"DXF layer code            "),"%d",&dxf_layer_numeric) != 1) goto error;
			if(sscanf(rhead(fp,"DXF output mode (2D/3D)   "),"%d",&dxf_dim) != 1) goto error;
			if(sscanf(rhead(fp,"Stringer width            "),"%f",&def_str_width) != 1) goto error;
			if(sscanf(rhead(fp,"Stringer thickness        "),"%f",&def_str_thick) != 1) goto error;
			if(sscanf(rhead(fp,"Stringer direction        "),"%d",&def_str_direc) != 1) goto error;
			if(sscanf(rhead(fp,"Stringer interval         "),"%f",&def_str_interv) != 1) goto error;
			if(sscanf(rhead(fp,"Stringer first interval   "),"%f",&def_str_firstint) != 1) goto error;
			if(sscanf(rhead(fp,"Perspective view azimuth  "),"%f",&rotn) != 1) goto error;
			if(sscanf(rhead(fp,"Perspective view vertical "),"%f",&yview) != 1) goto error;
			if(sscanf(rhead(fp,"Perspective view distance "),"%f",&zview) != 1) goto error;
			if(sscanf(rhead(fp,"View to show on opening   "),"%d",&view_on_load) != 1) goto error;

			if(sscanf(rhead(fp,"Show tanks                "),"%d",&showtanks) != 1) goto error;
			if(sscanf(rhead(fp,"Show stringers            "),"%d",&showstringers) != 1) goto error;
			if(sscanf(rhead(fp,"Show frames               "),"%d",&showframes) != 1) goto error;
			if(sscanf(rhead(fp,"Show section numbers      "),"%d",&shownumbers) != 1) goto error;
			if(sscanf(rhead(fp,"Show section names        "),"%d",&shownames) != 1) goto error;
			if(sscanf(rhead(fp,"Show hydrostatic centres  "),"%d",&showcentres) != 1) goto error;

			if(sscanf(rhead(fp,"Shading, light azimuth    "),"%f",&l_azim) != 1) goto error;
			if(sscanf(rhead(fp,"Shading, light elevation  "),"%f",&l_elev) != 1) goto error;

			if(sscanf(rhead(fp,"Drag, upper speed         "),"%f",&speed) != 1) goto error;
			if(sscanf(rhead(fp,"Drag, propellor diameter  "),"%f",&prop_diam) != 1) goto error;
			if(sscanf(rhead(fp,"Drag, pitch/diam. ratio   "),"%f",&pd_ratio) != 1) goto error;
			if(sscanf(rhead(fp,"Drag, presume twin screw  "),"%d",&twin_screw) != 1) goto error;
			if(sscanf(rhead(fp,"Drag, 'epsilon'           "),"%f",&epsilon) != 1) goto error;
			if(sscanf(rhead(fp,"Drag, distance to shaft   "),"%f",&dist_down) != 1) goto error;
			if(sscanf(rhead(fp,"Drag schemes used         "),"%d %d %d %d %d %d",
				&dragscheme[0],&dragscheme[1],&dragscheme[2],&dragscheme[3],&dragscheme[4],&dragscheme[5]) != 6) goto error;

			if(sscanf(rhead(fp,"Plate, transom/stem lines "),"%d",&tran_lines) != 1) goto error;

			if(sscanf(rhead(fp,"Variables plot, start     "),"%f",&plot_start) != 1) goto error;
			if(sscanf(rhead(fp,"Variables plot, end       "),"%f",&plot_end) != 1) goto error;
			if(sscanf(rhead(fp,"Variables plot, increment "),"%f",&plot_inc) != 1) goto error;
			if(sscanf(rhead(fp,"Variables plot, X-axis    "),"%d",&xvar) != 1) goto error;
			if(sscanf(rhead(fp,"Variables plot, Y-axis    "),"%d",&yvar) != 1) goto error;
			if(sscanf(rhead(fp,"Variables plot, ind. var. "),"%d",&tvar) != 1) goto error;

			if(sscanf(rhead(fp,"Tabular output mode       "),"%d",&stabvar) != 1) goto error;
			p = rhead(fp,"Tabular output variables  ");
			if(*p == 0) goto error;
			j = readints(p,tabusel,MAXSEL);
			while(j < MAXSEL) tabusel[j++] = -1;

			if(sscanf(rhead(fp,"OpenGL / GDI graphics     "),"%d",&scrdev) != 1) goto error;
			if(sscanf(rhead(fp,"Edit mode                 "),"%d",&v7mode) != 1) goto error;
			if(sscanf(rhead(fp,"Edit sections             "),"%s",buffer) != 1) {
				for(i = 0 ; i < count ; i++) editsectnum[i] = TRUE;
			}
			else {
				multproc(buffer,editsectnum,count);
			}
			if(sscanf(rhead(fp,"Also-show sections        "),"%s",alsosect) != 1) strcpy(alsosect,"NONE");
			if(sscanf(rhead(fp,"Show lines                "),"%s",showline) != 1) strcpy(showline,"NONE");

			if(sscanf(rhead(fp,"Startup window            "),"%d %d %d %d",&(rc_startup.left),&(rc_startup.top),
				&(rc_startup.right),&(rc_startup.bottom)) != 4) {
				rc_startup.left = 0;
				rc_startup.right = 640;
				rc_startup.top = 0;
				rc_startup.bottom = 400;
			}
			if(sscanf(rhead(fp,"Assistant mode            "),"%d",&assistant) != 1) assistant = 1;
			if(sscanf(rhead(fp,"Text-Edit box lines       "),"%d",&editlines) != 1) editlines = 10;
			if(sscanf(rhead(fp,"E-format limit            "),"%f",&elimit) != 1) elimit = 0.01;
			if(sscanf(rhead(fp,"Decimal places            "),"%d",&num_dp) != 1) num_dp = 3;
#ifdef linux
			(void) sscanf(rhead(fp,"Browser path for help     "),"%s",browser);
#else
			(void) sscanf(rhead(fp,"Browser path for help     "),"%s",text);
#endif
			if(sscanf(rhead(fp,"Temporary GL window       "),"%d",&temp_gl) != 1) temp_gl = FALSE;
			fclose(fp);
		}
	}

	densit[0] = spgrav*1000.0;
	densit[1] = spgrav;
	densit[2] = spgrav*62.5;
	densit[3] = densit[2]/2240.0;
	density = densit[numun];

	GL_surfaces_need_redraw = TRUE;
	GL_wireframe_need_redraw = TRUE;
	return;

error:
	sprintf(text,"Can not read configuration file:\nFailure at\n%s",conftext);
	message(text);
}

void fixfn(char file[])
{
	char *p;
	if((p = strchr(file,'\n')) != NULL) {
		*p = 0;
	}
	else {
		p = strchr(file,0);
	}
	if(*--p == '\\') *p = 0;
}

MENUFUNC save_config()
{
	extern	int	termin;
	extern	INT	default_hardcopy_device;
	extern	int	vgamodel;
	extern	REAL	invert,posdir;
	extern	char	*lenun[],*masun[];
	FILE	*fp;
	extern	int	xright,xmaxi;
	extern	COLORREF scrcolour[];
	extern	HDC hDC;
	char	buffer[500];
	LOGFONT	*lp;
	static char	*noopen = "Could not open configuration file for output.";
	extern char *cfgfile;
	extern int	xchar,ychar;
	int		j,k;
	char	*p;

	if(*cfgfile == '\0') strcpy(cfgfile,"hullform.cfg");
	fp = fopen(cfgfile,"wt");
	if(fp == NULL) {

//	Try the user's local application data area

		p = getenv("LOCALAPPDATA");
		if(p != NULL) {
			strcpy(cfgfile,p);
			strcat(cfgfile,"\\Hullform 9\\hullform.cfg");
			fp = fopen(cfgfile,"wt");
		}
	}

//	Try the user's file search path

	if(fp == NULL) {
		p = searchpath("hullform.cfg");
		if(p != NULL) {
			strcpy(cfgfile,p);
			fp = fopen(cfgfile,"wt");
		}
	}

	if(fp == NULL) {
		message(noopen);
	}
	else {
		fprintf(fp,"Plot device code          %d\n",default_hardcopy_device);
		fprintf(fp,"Plot file name            %s\n",port);
		fprintf(fp,"Measurement unit code     %d\n",numun);
		fprintf(fp,"Show inches with feet     %d\n",inches);

		fprintf(fp,"Hull data directory       %s\n",dirnam);
		fprintf(fp,"Text output directory     %s\n",filedirnam);
		fprintf(fp,"DXF output directory      %s\n",dxfdirnam);

		fprintf(fp,"Vertical display factor   %.0f\n",invert);
		fprintf(fp,"Horizontal display factor %.0f\n",posdir);
		fprintf(fp,"Water specific gravity    %f\n",spgrav);
		fprintf(fp,"Points between sections   %d\n",numbetw);
		fprintf(fp,"Points between lines      %d\n",numbetwl);
		fprintf(fp,"Plot shades non-dithered  %d\n",solidshade);
		fprintf(fp,"Screen colours            %06x %06x %06x %06x %06x %06x %06x %06x %06x\n",
			scrcolour[0],scrcolour[1],scrcolour[2],scrcolour[3],scrcolour[4],
			scrcolour[5],scrcolour[6],scrcolour[7],scrcolour[8]);
		fprintf(fp,"Widths and styles         ");
		for(k = 0 ; k < 5 ; k++) fprintf(fp," %d %d",linestyle[k],linewidth[k]);
		fprintf(fp,"\n");

		fprintf(fp,"Output page width         %f\n",pagewidth);
		fprintf(fp,"Page width unit code      %d\n",scunit);
		fprintf(fp,"Page X-scale factor       %f\n",Xfactor);
		fprintf(fp,"Page Y-scale factor       %f\n",Yfactor);

		lp = &lf;
#ifdef linux
		fprintf(fp,"Screen font               %s\n",lp->lfFaceName);
#else
		fprintf(fp,"Screen font               %d %d %d %d %d %d %d %d %d %d %d %d %d : %s\n",
			lp->lfHeight,lp->lfWidth,lp->lfEscapement,
			lp->lfOrientation,lp->lfWeight,lp->lfItalic,
			lp->lfUnderline,lp->lfStrikeOut,lp->lfCharSet,
			lp->lfOutPrecision,lp->lfClipPrecision,lp->lfQuality,
			lp->lfPitchAndFamily,lp->lfFaceName);
#endif
		lp = &prfont;
		fprintf(fp,"Printer font              %d %d %d %d %d %d %d %d %d %d %d %d %d : %s\n",
			lp->lfHeight,lp->lfWidth,lp->lfEscapement,
			lp->lfOrientation,lp->lfWeight,lp->lfItalic,
			lp->lfUnderline,lp->lfStrikeOut,lp->lfCharSet,
			lp->lfOutPrecision,lp->lfClipPrecision,lp->lfQuality,
			lp->lfPitchAndFamily,lp->lfFaceName);

		fprintf(fp,"Number of menu programs   %d\n",numprog);
		for(j = 0 ; j < numprog ; j++) {
			fprintf(fp,"Program caption           %s\n",progtext[j]);
			fprintf(fp,"Program name              %s\n",progname[j]);
			fprintf(fp,"Argument / reload         %d %d\n",addarg[j],reload[j]);
		}

		fprintf(fp,"Toolbar count, hint delay %d %d\n",numtool,HintDelay);
		fprintf(fp,"Toolbar location code     %d\n",toolloc);
		for(j = 0 ; j < numtool ; j++) {
#ifdef linux
			fprintf(fp,"Button %2d tag,index,hint  %d %d |%s\n",j+1,ToolButtonTag[j],ToolButtonIndex[j],ToolButtonHint[j]);
#else
			fprintf(fp,"Button %2d tag,index,hint  %d %d |%s\n",j+1,tool_id[j],Bitmap[j],tbtext[j]);
#endif
		}

		fprintf(fp,"Previous four hull data files\n");
		for(j = 0 ; j < 4 ; j++) {
			if( (p = strchr(lastname[j],'\n')) != NULL) *p = 0;
			if( (p = strchr(lastpath[j],'\n')) != NULL) *p = 0;
			fprintf(fp,"File %2d                   %s:%s\n",j+1,lastname[j],lastpath[j]);
		}
		fprintf(fp,"Builders outline places   %d\n",outl_places);
		fprintf(fp,"Skin thickness            %f\n",def_outl_thickness);
		fprintf(fp,"Hole radius               %f\n",mousehole_radius);
		fprintf(fp,"Notch width               %f\n",notch_width);
		fprintf(fp,"Notch height              %f\n",notch_height);
		fprintf(fp,"Distance between points   %f\n",outl_inc);
		fprintf(fp,"DXF output layers         %d\n",dxf_maxlayer);
		fprintf(fp,"DXF layer code            %d\n",dxf_layer_numeric);
		fprintf(fp,"DXF output mode (2D/3D)   %d\n",dxf_dim);
		fprintf(fp,"Stringer width            %f\n",def_str_width);
		fprintf(fp,"Stringer thickness        %f\n",def_str_thick);
		fprintf(fp,"Stringer direction        %d\n",def_str_direc);
		fprintf(fp,"Stringer interval         %f\n",def_str_interv);
		fprintf(fp,"Stringer first interval   %f\n",def_str_firstint);
		fprintf(fp,"Perspective view azimuth  %f\n",rotn);
		fprintf(fp,"Perspective view vertical %f\n",yview);
		fprintf(fp,"Perspective view distance %f\n",zview);
		fprintf(fp,"View to show on opening   %d\n",view_on_load);

		fprintf(fp,"Show tanks                %d\n",showtanks);
		fprintf(fp,"Show stringers            %d\n",showstringers);
		fprintf(fp,"Show frames               %d\n",showframes);
		fprintf(fp,"Show section numbers      %d\n",shownumbers);
		fprintf(fp,"Show section names        %d\n",shownames);
		fprintf(fp,"Show hydrostatic centres  %d\n",showcentres);

		fprintf(fp,"Shading, light azimuth    %f\n",l_azim);
		fprintf(fp,"Shading, light elevation  %f\n",l_elev);

		fprintf(fp,"Drag, upper speed         %f\n",speed);
		fprintf(fp,"Drag, propellor diameter  %f\n",prop_diam);
		fprintf(fp,"Drag, pitch/diam. ratio   %f\n",pd_ratio);
		fprintf(fp,"Drag, presume twin screw  %d\n",twin_screw);
		fprintf(fp,"Drag, 'epsilon'           %f\n",epsilon);
		fprintf(fp,"Drag, distance to shaft   %f\n",dist_down);
		fprintf(fp,"Drag schemes used        ");
		for(k = 0 ; k < 6 ; k++) fprintf(fp," %d",dragscheme[k]);
		fprintf(fp,"\n");

		fprintf(fp,"Plate, transom/stem lines %d\n",tran_lines);

		fprintf(fp,"Variables plot, start     %f\n",plot_start);
		fprintf(fp,"Variables plot, end       %f\n",plot_end);
		fprintf(fp,"Variables plot, increment %f\n",plot_inc);
		fprintf(fp,"Variables plot, X-axis    %d\n",xvar);
		fprintf(fp,"Variables plot, Y-axis    %d\n",yvar);
		fprintf(fp,"Variables plot, ind. var. %d\n",tvar);

		fprintf(fp,"Tabular output mode       %d\n",stabvar);
		fprintf(fp,"Tabular output variables ");
		for(k = 0 ; k < MAXSEL ; k++) fprintf(fp," %d",tabusel[k]);
		fprintf(fp,"\n");

		fprintf(fp,"OpenGL / GDI graphics     %d\n",scrdev);
		fprintf(fp,"Edit mode                 %d\n",v7mode);
		listseq(editsectnum,buffer,count);
		fprintf(fp,"Edit sections             %s\n",buffer);
		fprintf(fp,"Also-show sections        %s\n",alsosect);
		fprintf(fp,"Show lines                %s\n",showline);

		fprintf(fp,"Startup window            %d %d %d %d\n",rc_startup.left,rc_startup.top,rc_startup.right,rc_startup.bottom);
		fprintf(fp,"Assistant mode            %d\n",assistant);
		fprintf(fp,"Text-Edit box lines       %d\n",editlines);
		fprintf(fp,"E-format limit            %f\n",elimit);
		fprintf(fp,"Decimal places            %d\n",num_dp);
#ifdef linux
		fprintf(fp,"Browser path for help     %s\n",browser);
#else
		fprintf(fp,"Browser path for help     (not needed)\n");
#endif
		fprintf(fp,"Temporary GL window       %d\n",temp_gl);
		fclose(fp);
	}
}

#ifdef PROF
MENUFUNC set_plotter()
{
	extern int chan;
	extern int default_hardcopy_device;
	int result;

	/* 0 is NONE, plotters run 2 to 5 - convert indices to 0 to 4 */

	result = max(0,default_hardcopy_device - 1);
	if(getdlg(CONFPORT,
		INP_STR,(void *) port,
		INP_RBN,(void *) &result,
		INP_PBF,browse_plot_directory,
		-1)) {
		default_hardcopy_device = result + 1;
	}
	chan = 0;
}

void browse_plot_directory(int code,HWND hWndDlg)
{
	char path[MAX_PATH];
	char start[MAX_PATH];
#ifdef linux
	strncpy(start,XmTextGetString(wEdit[code]),MAX_PATH);
#else
	GetDlgItemText(hWndDlg,DLGEDIT+code,start,MAX_PATH);
#endif
	striptopath(start);
	if(!openfile(path,"wt","Plot Output File","All files (*.*)\0*.*\0","*.*",start,NULL)) return;
	strlwr(path);
#ifdef linux
	XmTextSetString(wEdit[code],path);
#else
	SetDlgItemText(hWndDlg,DLGEDIT+code,path);
#endif
}

#endif

MENUFUNC set_loadview()
{
	(void) getdlg(LOADVIEW,INP_RBN,(void *) &view_on_load,-1);
}

MENUFUNC set_scale()
{
	REAL scale;
	extern REAL pagewidth;
	extern int scunit;
	REAL vw = 1000.0*fabs(xmax - xmin);	/* plot area width in mm */
	REAL pw;
	extern REAL Xwidth;
	extern REAL	hardcopy_Xwidth;
	extern int fixed_scale;

	if(vw <= 0.0) vw = 1.0;
	if(pagewidth <= 0.0) pagewidth = 200.0;
	pw = pagewidth;
	if(Xfactor <= 0.0) Xfactor = 1.0;
	if(Yfactor <= 0.0) Yfactor = 1.0;

	if(numun > 1) vw *= 0.3048;	/* make feet*1000 into millimetres */
	if(scunit == 1) pw *= 25.4;	/* make page size in inches into mm */

	scale = vw / pw;

	if(getdlg(CONFSCAL,
		INP_REA,(void *) &scale,
		INP_REA,(void *) &pagewidth,
		INP_REA,(void *) &Xfactor,
		INP_REA,(void *) &Yfactor,
		INP_MEN,(void *) &scunit,-1)) {

		pw = pagewidth;
		if(scunit == 1) pw *= 25.4;
		if(pw*scale < vw)
			message("BEWARE: the view will\n  overflow the page!");
		if(xmax < xmin) scale = -scale;

		vw = 0.001 * pw * scale;    /* width of plotted region in metres */
		if(numun > 1) vw /= 0.3048; /* or in feet */

		hardcopy_Xwidth = vw;
		fixed_scale = TRUE;
	}
}

MENUFUNC set_directory()
{
#ifdef EXT_OR_PROF
	char input[3][MAX_PATH];
#else
	char input[2][MAX_PATH];
#endif
	struct ffblk dirffblk;
	unsigned int dir = FA_DIREC;
	char *p;
	int i;
	char msg[MAX_PATH];

	strcpy(input[0],dirnam);
	strcpy(input[1],filedirnam);
#ifdef EXT_OR_PROF
	strcpy(input[2],dxfdirnam);
	if(!getdlg(CONFFILE,
		INP_STR,(void *) input[0],
		INP_STR,(void *) input[1],
		INP_STR,(void *) input[2],
		INP_PBF,browse_directories,
		-1)) return;
#define looplimit 3
#else
	if(!getdlg(CONFFILE,
		INP_STR,(void *) input[0],
		INP_STR,(void *) input[1],
		INP_PBF,browse_directories,
		-1)) return;
#define looplimit 2
#endif

	for(i = 0 ; i < looplimit ; i++) {
		p = strchr(input[i],0) - 1;
		if(*p == '\\') *p = 0;		/* trailing backslash can give MS Windows problems */

		if(findfirst(input[i],&dirffblk,dir) == 0) { /* if file found */
			if(!(dirffblk.ff_attrib & dir)) { /* if file is not directory */
				sprintf(msg,"%s is not a directory. Please select another",input[i]);
				message(msg);	/* generate error message */
				return;
			}
		}
		else {
			if(MessageBox(hWnd,"Directory does not exist. Create it?","WARNING",MB_YESNO) == IDYES) {
#ifdef linux
#define mkdircall mkdir(input[i],0x1f)
#else
#define mkdircall mkdir(input[i])
#endif
				if(mkdircall != 0) {
					sprintf(msg,"Can not create directory %s",input[i]);
					message(msg);
				}
			}
		}
	}
	strcpy(dirnam,input[0]);	/* valid directory selected */
	strcpy(filedirnam,input[1]);
#ifdef EXT_OR_PROF
	strcpy(dxfdirnam,input[2]);
#endif
}

void browse_directories(int code,HWND hWndDlg)
{
	char path[MAX_PATH];
	char start[MAX_PATH];
#ifdef linux
	strncpy(start,XmTextGetString(wEdit[code]),MAX_PATH);
#else
	GetDlgItemText(hWndDlg,DLGEDIT+code,start,MAX_PATH);
#endif
	striptopath(start);
	switch(code){
	case 0:		/* hull data file */
		if(!openfile(path,"rt","Select Any File in the Required Hull Data File Directory","Hull data files (*.hud)\0*.hud\0","*.hud",start,NULL)) return;
		break;
	case 1:		/* general file output */
		if(!openfile(path,"rt","Select Any File in the Required File Output Directory","All files\0*.*\0","*.*",start,NULL)) return;
		break;
#ifdef EXT_OR_PROF
	case 2:		/* DXF output directory */
		if(!openfile(path,"rt","Select Any File in the Required DXF Output Directory","DXF Files (*.dxf)\0*.dxf\0All files\0*.*\0","*.dxf",start,NULL)) return;
		break;
#endif
	}
	strlwr(path);
	striptopath(path);
#ifdef linux
	XmTextSetString(wEdit[code],path);
#else
	SetDlgItemText(hWndDlg,DLGEDIT+code,path);
#endif
}

MENUFUNC set_density()
{
	if(getdlg(CONFDENS,INP_REA,(void *) &spgrav,-1)) {
		densit[0] = spgrav*1000.0;
		densit[1] = spgrav;
		densit[2] = spgrav*62.5;
		densit[3] = densit[2]/2240.0;
		density = densit[numun];
	}
}

MENUFUNC set_units()
{
	extern INT numun;
	int	result = numun;
	if(getdlg(CONFUNIT,
		INP_MEN,(void *) &result,
		INP_LOG,(void *) &inches,
		-1) && result >= 0) {
		numun = result;
		changed = 1;
		density = densit[numun];
	}
}

MENUFUNC set_x_positive()
{
	extern REAL posdir;
	int pos = ((1 + (int) posdir)) >> 1;
	if(getdlg(CONFXPOS,INP_RBN,(void *) &pos,-1)) {
		posdir = (REAL) (2 * pos - 1);
	}
}

MENUFUNC set_z_positive()
{
	extern REAL invert;
	int pos = (((int) invert) + 1) >> 1;
	if(getdlg(CONFZPOS,INP_RBN,(void *) &pos,-1)) invert = (REAL) (2 * pos - 1);
}

MENUFUNC set_detail()
{
	extern int numbetw,numbetwl;
#ifndef STUDENT
	void *fp;
	int maxtran;
#endif

	if(getdlg(CONFDETA,
#ifdef EXT_OR_PROF
	INP_INT,(void *) &numbetw,
#else
	INP_INT,(void *) NULL,
#endif
	INP_INT,(void *) &numbetwl,
		-1)) {
		numbetwl = max(0,min(40,numbetwl));
#ifndef STUDENT
		maxtran = max(1,(extlin-1) * (numbetwl+1) + 1);
		fp = (void *) xtran;
		(void) altavail(&fp,3*sizeof(REAL)*maxtran);
		xtran = (REAL *) fp;
		ytran = &xtran[maxtran];
		ztran = &ytran[maxtran];
		recalc_transom();
#endif
	}
}

#ifdef linux
Widget linux_SELFONT(
	Widget wLabel[],Widget wEdit[], Widget wRadioButton[],Widget wCheckBox[],
	Widget wPushButton[],Widget wListBox[],Widget wComboBox[],
	Widget *wOk,Widget *wCancel,Widget *wHelp)
{
	Widget Wshell,Wdialog;
	Wshell = MakeDialogShell("Select Screen Font");
	Wdialog = MakeDialog(Wshell,"Select Screen Font",100,100,400,300);

	wListBox[0] = ListWidget(Wdialog,10,10,380,240,False);

	*wOk = ButtonWidget(Wdialog,"Ok",90,260,60,24);
	XtVaSetValues(Wdialog,XmNdefaultButton,*wOk,NULL);
	*wCancel = ButtonWidget(Wdialog,"Cancel",170,260,60,24);
	*wHelp = ButtonWidget(Wdialog,"Help",250,260,60,24);
	return Wdialog;
}
#endif

MENUFUNC set_screenfont()
{
#ifdef linux
#define maxfonts 500
	extern Display *display;
	int numfonts;
	char **fontname;
	char textfield[MAX_PATH];
	int i;
	struct {
		int index;       /* index of result in table */
		char *string;    /* pointer to result */
		char *table[maxfonts+1]; /* table of strings for listbox, null string terminator */
	}
	flist;

	fontname = XListFonts(display,"*",maxfonts,&numfonts);
	flist.string = textfield;

	flist.index = 0;
	for(i = 0 ; i < numfonts ; i++) {
	    flist.table[i] = fontname[i];
	    if(strcmp(lf.lfFaceName,fontname[i]) == 0) flist.index = i;
	}
	flist.table[i] = "";
	if(getdlg(linux_SELFONT,INP_LBX,&flist,-1) && flist.index > 0) {
		strcpy(lf.lfFaceName,fontname[flist.index]);
	}
	XFreeFontNames(fontname);
	setfont(&lf);
#else
	CHOOSEFONT chfont;
	memset(&chfont,0,sizeof(CHOOSEFONT));
	chfont.lStructSize = sizeof(CHOOSEFONT);
	chfont.hwndOwner = hWnd;
	chfont.lpLogFont = &lf;
	chfont.Flags = CF_SCREENFONTS | CF_INITTOLOGFONTSTRUCT | CF_LIMITSIZE;
	chfont.nFontType = SCREEN_FONTTYPE;
	chfont.nSizeMin = 2;
	chfont.nSizeMax = 24;
	if(ChooseFont(&chfont)) setfont(&lf);
#endif
}

void setfont(LOGFONT *lf)
{
#ifdef linux
	extern XFontStruct *fontstruct;
	extern GC gc;
	extern Display *display;
	extern XmFontList fontlist;
	extern Widget glw;
	Dimension textw,texth;
	XmString xstr;
	extern int xchar,ychar;
    extern int GLfontBase;
    void MakeFont(char *fontname);

	if((fontstruct = XLoadQueryFont(display,lf->lfFaceName)) == NULL) {
		(void) fprintf(stderr,"Cannot open font %s",lf->lfFaceName);
		exit(1);
	}
	XSetFont(display,gc,fontstruct->fid);
	fontlist = XmFontListCreate(fontstruct,XmSTRING_DEFAULT_CHARSET);
	xstr = XmStringCreateSimple("abcdefghijABCDEFGHIJ");
	XmStringExtent(fontlist,xstr,&textw,&texth);
	XmStringFree(xstr);
	xchar = textw / 20;
	ychar = texth;

	if(scrdev == 0) MakeFont(lf->lfFaceName);

#else
	extern HDC hDC;
	HFONT hFont = (HFONT) CreateFontIndirect(lf);
	HFONT OldFont =  (HFONT) SelectObject(hDC,hFont);
	TEXTMETRIC tm;
	extern int xchar,ychar;

	if(OldFont != NULL) DeleteObject(OldFont);
	GetTextMetrics(hDC,&tm);
	ychar = tm.tmHeight;
	xchar = tm.tmAveCharWidth;
#endif
}

MENUFUNC set_printerfont()
{
#ifdef linux
	int size = abs(prfont.lfHeight);
	int bold = prfont.lfWeight > 400;
	int italic = prfont.lfItalic > 0;
	int underline = prfont.lfUnderline > 0;
	int strikeout = prfont.lfStrikeOut > 0;
	if(getdlg(PRFONT,
		INP_STR,(void *) &(prfont.lfFaceName),
		INP_INT,(void *) &size,
		INP_LOG,(void *) &bold,
		INP_LOG,(void *) &italic,
		INP_LOG,(void *) &underline,
		INP_LOG,(void *) &strikeout,
		-1)) {
		prfont.lfHeight = -size;
		prfont.lfWeight = bold ? 700 : 400;
		prfont.lfItalic = italic;
		prfont.lfUnderline = underline;
		prfont.lfStrikeOut = strikeout;
	}
#else
	static CHOOSEFONT chfont;
	extern HDC hDC;
	HDC prDC = GetPrDC();
	if(prDC == NULL) {
		message("No printer selected");
	}
	else {
		memset(&chfont,0,sizeof(CHOOSEFONT));
		chfont.lStructSize = sizeof(CHOOSEFONT);
		chfont.hwndOwner = hWnd;
		chfont.hDC = prDC;
		chfont.lpLogFont = &prfont;
		chfont.Flags = CF_PRINTERFONTS | CF_FORCEFONTEXIST | CF_INITTOLOGFONTSTRUCT | CF_LIMITSIZE;
		chfont.nFontType = BOLD_FONTTYPE | REGULAR_FONTTYPE | ITALIC_FONTTYPE | PRINTER_FONTTYPE;
		chfont.nSizeMin = 2;
		chfont.nSizeMax = 24;
		(void) ChooseFont(&chfont);
		pointsize = abs(MulDiv(prfont.lfHeight,72,GetDeviceCaps(hDC,LOGPIXELSY)));
		DeleteDC(prDC);
	}
#endif
}


/*	Copy source string into dest without null termination and 	*/
/*	padding dest with spaces					*/

void strcp(char *dest,char *source)
{
	while(*dest && *source) {
		if(isprint(*source)) *dest++ = *source;
		source++;
	}
	while(*dest) *dest++ = ' ';
}

void set_graphics()
{
	int old_scrdev = scrdev;
	(void) getdlg(CONFMODE,
		INP_RBN,(void *) &scrdev,
#ifdef PROF
		INP_LOG,(void *) &temp_gl,
#endif
		-1);
	if(old_scrdev != scrdev) {
#ifdef linux
		if(scrdev == 0) {
			if(MessageBox(hWnd,"Hullform will have to exit immediately you change to GL mode. Do you need to save your design first?",
			"Warning!",MB_YESNO) == IDYES) {
				scrdev = old_scrdev;
			} else {
				save_config();
				exit(0);
			}
		} else {
			message("You should now exit and restart Hullform, for the change to take effect.");
			setup(scrdev);
		}
#else
		message("You should now exit and restart Hullform, for the change to take effect.");
		setup(scrdev);
#endif
	}
}


void textedit_lines()
{
	(void) getdlg(TEXTEDITLINES,INP_INT,(void *) &editlines,-1);
}

void set_elimit()
{
	(void) getdlg(ELIMIT,
		INP_REA,(void *) &elimit,
		INP_INT,(void *) &num_dp,
		-1);
}

#ifdef linux

Widget linux_colours(
	Widget wLabel[],Widget wEdit[], Widget wRadioButton[],Widget wCheckBox[],Widget wPushButton[],
	Widget wListBox[],Widget wComboBox[],Widget *wOk,Widget *wCancel,Widget *wHelp)
{
	Widget Wshell,Wdialog,*wFirst,w;
	Wshell = MakeDialogShell("Set Colour");

	Wdialog = MakeDialog(Wshell,"Set Colour",14,23,240,144);

	wLabel[0] = LabelWidget(Wdialog,"Red",False,60,12,52,20);
	wEdit[0] = TextWidget(Wdialog,120,8,48,24);

	wLabel[1] = LabelWidget(Wdialog,"Green",False,60,44,52,20);
	wEdit[1] = TextWidget(Wdialog,120,40,48,24);

	wLabel[2] = LabelWidget(Wdialog,"Blue",False,60,76,52,20);
	wEdit[2] = TextWidget(Wdialog,120,72,48,24);

	*wOk = ButtonWidget(Wdialog,"Ok",20,104,60,24);
	XtVaSetValues(Wdialog,XmNdefaultButton,*wOk,NULL);

	*wCancel = ButtonWidget(Wdialog,"Cancel",90,104,60,24);

	*wHelp = ButtonWidget(Wdialog,"Help",160,104
		    ,60,24);

	XtManageChild(Wdialog);
	return Wdialog;
}

#endif

void set_colours()
{
	/*	Background = 0
	Text	   = 1
	Sections   = 2
	Lines      = 3
	Waterlines = 4
	Diagonals  = 5
	Buttocks   = 6
	Shading    = 7
	Shadow     = 8
	Overlay    = 9
	*/
#ifdef PROF
	extern int solidshade;
#endif

	(void) getdlg(CONFCOLO,
		INP_PBF,(void *) colfunc,
#ifdef PROF
	INP_LOG,(void *) &solidshade,
#endif
	-1);
}

void colfunc(int colournum,HWND hWndDlg)
{
	extern COLORREF scrcolour[];
#ifdef linux
	int r,g,b,col;
	col = (int) scrcolour[colournum];
	r = (col >> 16) & 0xff;
	g = (col >>  8) & 0xff;
	b = (col      ) & 0xff;
	if(getdlg(linux_colours,
		INP_INT,(void *) &r,
		INP_INT,(void *) &g,
		INP_INT,(void *) &b,
		-1)) {
		scrcolour[colournum] = (COLORREF) (r << 16) + (g << 8) + b;
	}
#else
	CHOOSECOLOR	cc;
	COLORREF	aclrCust[16];
	int i;

	/*	Set the custom color controls to white. */

	for (i = 0; i < 16; i++) aclrCust[i] = RGB(255, 255, 255);

	/*	Set all structure fields to zero. */

	memset(&cc, 0, sizeof(CHOOSECOLOR));

	/* Initialize the necessary CHOOSECOLOR members. */

	cc.lStructSize = sizeof(CHOOSECOLOR);
	cc.hwndOwner = hWnd;
	cc.rgbResult = scrcolour[colournum];
	cc.lpCustColors = aclrCust;
	cc.Flags = CC_RGBINIT;

	if (ChooseColor(&cc)) scrcolour[colournum] = cc.rgbResult;
#endif
}

void stylefunc(int,HWND);

void set_linestyles()
{
	(void) getdlg(CONFSTYL,INP_PBF,(void *) stylefunc,-1);
}

void stylefunc(int ncol,HWND hWndDlg)
{
	char string[100];
	extern int linestyle[];
	extern int linewidth[];
#ifdef linux
	extern Widget wPushButton[];
	XmString xstr;
	char *str;

	XtVaGetValues(wPushButton[ncol],XmNlabelString,&xstr,NULL);
    XmStringGetLtoR(xstr,XmFONTLIST_DEFAULT_TAG,&str);
	strcpy(string,str);
	XmStringFree(xstr);
#else
	GetDlgItemText(hWndDlg,DLGPBF+ncol,string,20);
	if(string[0] == '&') strcpy(string,string+1);
#endif
	(void) getdlg(STYLEOPT,
		0,string,
		INP_RBN,(void *) &linestyle[ncol],
		INP_INT,(void *) &linewidth[ncol],
		-1);
}

void set_editmode()
{
	(void) getdlg(EDITMODE,INP_RBN,&v7mode,-1);
}

#ifndef linux
HDC GetPrDC()
{
	char device[MAX_PATH];
	char *driver,*port;
	if(GetProfileString("windows","device","",device,MAX_PATH) > 0) {
		driver = strchr(device,',');
		if(driver != NULL) {
			*driver++ = 0;
			port = strchr(driver,',');
			if(port != NULL) {
				*port++ = 0;
			}
			else {
				port = "";
			}
			return (HDC) CreateIC(driver,device,port,NULL);
		}
	}
	return NULL;
}
#endif

#ifdef linux

void set_browser()
{
	extern char browser[];
	(void) getdlg(BROWSER,INP_STR,(void *) browser,-1);
}

#endif
