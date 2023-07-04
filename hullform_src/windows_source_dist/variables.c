/* Hullform component - variables.c
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
 
/*	Hullform for Linux and Windows main program		*/

#include "hulldesi.h"   /* declare common data as external */

int	scrdev = 0;

#ifndef linux
ATOM aAppl = 0,aTopic = 0,aItem = 0;
#endif
#ifdef DEMO
void init_demo(void);
#endif
int toolheight = 32,toolwidth = 32;
int repaint = TRUE;	/* allow repaint */
int working = FALSE;
int focus = FALSE;
int dragscroll = FALSE;
int ddemode = 0;
extern FILE *dbg;

#ifdef HULLSTAT
extern int statedit_mode;
#ifdef DEMO
int allow_hullstat = FALSE;	/* may be set TRUE in demo version */
#endif
int level[80];
char op[80];
char *par[80];	/* pointers to parameters in evaluation string	*/
#else
int level[2];
char op[2];
char *par[4];	/* pointers to parameters in evaluation string	*/
#endif
int parse0 = 0;
int brlev = 0;
int paused = 0;
int persp = 0;
int force_linear = 0;
extern REAL wate_int;
RECT rc_startup = {0,0,0,0};

char	*helpfile;		/* help file name */
int	context_id;
char	*context_string;

int	v7mode = FALSE;

/*	Line smoothing arrays	*/

char	*alsolines;
char	*masterstring;
char	*inpsmoo;
char	*relsmoo;
int 	*ignsmoo;
int	*relax;
REAL	offsetb[maxsec+2];
int	*line_also;
REAL	*linesave;

REAL (far *yline)[maxsec+4] = NULL;
REAL (far *zline)[maxsec+4] = NULL;
REAL (far *ycont)[maxsec+4] = NULL;
REAL (far *zcont)[maxsec+4] = NULL;

REAL (far *linewt)[maxsec+4] = NULL;
int  *stsec;
int  *ensec;
int  *relcont;
REAL *xsect;
int numlin = 0;
int extlin = 0;
int stemli = 0;
int count = 0;
int surfacemode = 0;
REAL *radstem;


#ifdef EXT_OR_PROF
REAL (far *yline_main)[maxsec+4] = NULL;
REAL (far *zline_main)[maxsec+4] = NULL;
REAL (far *ycont_main)[maxsec+4] = NULL;
REAL (far *zcont_main)[maxsec+4] = NULL;
REAL (far *yline_overlay)[maxsec+4] = NULL;
REAL (far *zline_overlay)[maxsec+4] = NULL;
REAL (far *ycont_overlay)[maxsec+4] = NULL;
REAL (far *zcont_overlay)[maxsec+4] = NULL;
REAL (far *yline_merge)[maxsec+4] = NULL;
REAL (far *zline_merge)[maxsec+4] = NULL;
REAL (far *ycont_merge)[maxsec+4] = NULL;
REAL (far *zcont_merge)[maxsec+4] = NULL;
char *hullfile_main,*hullfile_overlay;

REAL (far *linewt_main)[maxsec+4] = NULL;
REAL (far *linewt_overlay)[maxsec+4] = NULL;
REAL (far *linewt_merge)[maxsec+4] = NULL;
int  *stsec_main,*stsec_overlay,*stsec_merge;
int  *ensec_main,*ensec_overlay,*ensec_merge;
int  *relcont_main,*relcont_overlay,*relcont_merge;
REAL *xsect_main,*xsect_overlay;
int numlin_main,numlin_overlay;
int extlin_main,extlin_overlay;
int stemli_main,stemli_overlay;
int count_main = 0,count_overlay = 0;
int surfacemode_overlay,surfacemode_main;
REAL disp_main,disp_overlay;
char *merge_hullfile = NULL;
REAL *radstem_main,*radstem_overlay,*radstem_merge;
REAL *xvalue,*yvalue;
#endif

/*	Stringer coordinates	*/

#ifndef STUDENT
REAL  (far *ystr)[maxsec+2] = NULL;
REAL  (far *zstr)[maxsec+2] = NULL;
REAL  (far *astr)[maxsec+2] = NULL;
REAL  far *ststrx = NULL;
REAL  far *enstrx = NULL;
int   far *ststr = NULL;
int   far *enstr = NULL;
int   *numstr;
int   *inistr;
int   *str_dir;	/* 1 if increasing toward keel, else 0 */
REAL  *str_thk;
REAL  *str_wid;
REAL  *str_interv;
REAL  *str_firstint;
int   *strmode;

REAL def_str_width,def_str_thick;
int def_str_direc;
REAL def_str_interv,def_str_firstint;
int def_str_count;
REAL def_str_firstfrac;

#endif

REAL *hwl;			/* height of waterline, allocated array */
int numun = 1;			/* index number of units used	*/
int inches = 1;			/* show inches when using foot as length unit */

char *lenun[4] = {"m ","m ","ft","ft"};
/*				table of length units		*/

char *masun[4] = {"kg","tonne","lb","ton"};
/*				table of mass units		*/

/*	Perspective projection locations for view point and centre	*/
/*	of viewed planed						*/

REAL	yview = 0.0,
	zview = 1000.0;

REAL	xpersp = 0.0,
	ypersp = 0.0,
	zpersp = 9999.0;

REAL g[4] = {9.8,9.8,32.0,32.0};
/*				acceleration due to gravity	*/

REAL densit[4] = {1025,1.025,64.0625,0.0285991};
/*				water density			*/

REAL invert = 1.0;
/*		1 or -1, depending on dir'n of increase of z	*/
REAL posdir = 1.0;
/*		1 or -1, depending on desired direction of bow	*/

char *textpointer;
char *hullfile;
/*				hull data file name		*/
char *dirnam;

char *filedirnam;

char *dxfdirnam;
/*				data directory name default	*/

REAL	sina = 0.0,
	cosa = 1.0,		/* heel angle parameters	*/
	beta;			/* pitch angle parameters	*/

REAL	*varval;
int	balanced = 0;
int	tankmode = FALSE;/* tank-mode hydrostatics (See calcar.c) */

#ifndef STUDENT
REAL	prop_diam,pd_ratio;
int	twin_screw;
REAL	viscos[4] = {1.191e-6,1.191e-6,1.282e-5,1.282e-5};

/*	transom parameters						*/

int	transom = 0;		/* transom on/off toggle		*/
REAL	dztransom;		/* height of transom above keel		*/
REAL	ztransom;		/* transom offset at stern		*/
REAL	dtransom = 0.0;		/* distance to transom plane from	*/
				/* x = 0, z = 0				*/
REAL	atransom = 0.0;		/* angle of transom from vertical	*/
REAL	stransom = 0.0,		/* transom slope from vertical, sine	*/
	ctransom = 1.0;		/* and cosine				*/
REAL	rtransom = 0.0;

/*	tank definition tables					*/

int	ntank = 0;		/* number of tanks		*/

int	*fl_line1;		/* first line for this tank */
int	*fl_fixed;		/* leaky or part full */
int	*fl_right;		/* on right side */

REAL	*tankpointer;
REAL	*fl_volum;
REAL	*fl_fract;
REAL	*fl_spgra;
REAL	*fl_walev;
REAL	*fl_perm;
REAL	*tankmass,*tankvcg,*tanklcg,*tanklcf,*tanktfsm,*tanklfsm,
	*tankmom;
int	tank = 0;
int	tankcol = 0;

/*	transom outline tables					*/

REAL	*xtran;
REAL	*ytran;	/* positions of intersections of lines with transom */
REAL	*ztran;
int	*itran;	/* index of section immediately stemward of transom */
int	numtran;

/*	variables for use by drag routines			*/

REAL	tocons[2][2] = {{1.0,0.514773},	/* conversion to m/s	*/
			{1.0,1.68889}};	/* conversion to ft/s	*/

/* conversion factor is tocons[numun >> 1][speedunit]		*/

INT	speedunit = 1;				/* unit toggle, 1=kt, 0=m or ft/s */

REAL	def,defp = 3.0;
REAL	m1,c1,c2,lambda,k;

int	linereq = 0;				/* DDE / hullstat requested line */

#endif

#ifdef PLATEDEV

/*	plate development data					*/

REAL  (far *xstart)[MAXRUL] = NULL; /* starting points of ruled lines */
REAL  (far *ystart)[MAXRUL] = NULL;
REAL  (far *zstart)[MAXRUL] = NULL;

REAL  (far *xend)[MAXRUL] = NULL;   /* end points of ruled lines */
REAL  (far *yend)[MAXRUL] = NULL;
REAL  (far *zend)[MAXRUL] = NULL;

/*	interpolation arrays in static storage, to allow access by	*/
/*	both development and viewing routines				*/

REAL	*xint_a = NULL;
REAL	*yint_a = NULL;
REAL	*zint_a = NULL;
		/* interpolated x-, y- and z-value tables for chine a */
REAL	*xint_b = NULL;
REAL	*yint_b = NULL;
REAL	*zint_b = NULL;

int	plate_lin = 1;	/* higher-indexed of two lines between	*/
			/* which plate development is to be done*/
int	plate_res = 1;	/* number of rulings per pair of hull	*/
			/* sections				*/
int	*developed;	/* set to -1 when lines read or edited	*/
void	free_dev(void);
int	*rulings;	/* number of rulings for each surface	*/
int	numruled = 0;	/* number of ruled surfaces so far	*/

#endif
int	indx;

/*	graphics parameters					*/

INT	xcursor,ycursor;	/* text cursor position */
char	*port;		/* hardcopy graphics port */
char	*cur_port;
char	*text_device;
INT	persp;			/* perspective on / off flag		*/
INT	device = -1;		/* current graphics device code		*/
int	hardcopy = 0;		/* hardcopy mode toggle			*/
int	graphics = 0;		/* graphics-mode hardcopy flag		*/
INT	hcpydev;		/* hardcopy device			*/
int	default_hardcopy_device;/* as stated				*/
REAL	Xfactor = 1.0;		/* x-stretch factor to resize hardcopy plot */
REAL	Yfactor = 1.0;		/* y-stretch factor to resize hardcopy plot */
REAL	Xwidth;			/* size of plotted region (real measurements) */
REAL	hardcopy_Xwidth;
REAL	Ywidth;
REAL	xcurr,ycurr,zcurr;		/* current graphic positions	*/
INT	chan;			/* graphics output channel		*/
REAL	xpersp,ypersp,zpersp;	/* centre of plane viewed in perspective*/
REAL	xvmin,xvmax;
REAL	yvmin,yvmax;
REAL	xscale,yscale;
int	scaled = FALSE;			/* rescale-required toggle	*/
int	fixed_scale = 0;

#ifdef PROF
FUNC_PTR zoomok[] = {
	general_orth,
	plan_orth,
	elev_orth,
	end_orth,
	full_persp,
	port_persp,
	starb_persp,
#ifndef STUDENT
	view_devel,
	rollout,
#endif
	edit_section,
	NULL};
#endif

REAL	angx = -1.0e+30,
	angy = 0.0,
	angz = 0.0;		/* rotation of plane viewed in perspective */

REAL	axx,axy,axz, ayx,ayy,ayz, azx,azy,azz;/* transformation coefficients*/
REAL	xorigi = 0.0,
	xmin   = 0.0,
	xmax   = 640.0;
REAL	yorigi = 0.0,
	ymin   = 0.0,
	ymax   = 480.0;
REAL	xmin0,xmax0,ymin0,ymax0;

int	xmaxi = 0,ymaxi = 0;
REAL	xbox,ybox;
REAL	PixelRatio;
REAL	pagewidth = 210.0;
int	scunit = 0;

REAL	xdorigin,xdslope,ydorigin,ydslope;

/*	viewing parameters			*/

REAL	diag_int	= 0.0;	/* diagonals default off	*/
REAL	diag_angle	= 45.0;	/* default angle 45 degrees	*/
REAL	butt_int	= 0.0;	/* buttocks default off		*/
REAL	wate_int	= 0.0;	/* waterlines default off	*/

int	shownumbers = 0;	/* numbers on sections default off */
int	shownames = 0;		/* names also default off */
int	showoverlay = 0;	/* overlay plotting defaults off */
int	surfaceset = 0;		/* 1 if a surface set is loaded */
char	(*surfname)[256];	/* will allow up to 8 surfaces */
int	*surfline,*mainline,*useovl,*usemain,*mergedn;

char	(*sectname)[12];

int	showframes;

int	showtanks = 0;		/* tanks default off		*/
int	showstringers = 0;	/* stringers default off	*/
int	showcentres = 0;	/* centres on plots */

/*	sections to ignore in interpolation, and specifier string	*/

int	*ignore;
char	*ignline;
int	*lineon;		/* line-plotting flags		*/
int	*secton;		/* section-plotting flags	*/

char	*linestoplot;		/* line-plotting input string	*/
char	*sectionstoplot;	/* section-plotting input string*/

REAL	xmin,xmax,ymin,ymax;	/* image limits, perhaps zoomed	*/
REAL	xmin0,xmax0,ymin0,ymax0;/* image limits unzoomed	*/
int	zoom = 0;		/* zoomed-view toggle		*/
int	scrollable = FALSE;	/* scroll bars usable		*/
int	horzpos = 0,vertpos = 0;/* scroll bar positions		*/

#ifdef EXT_OR_PROF
int	animate = 0;		/* animation-active flag	*/
#endif

INT	inisec,lassec;
int	endlin = 0;		/* end line, set depending on value of "showtanks */
int stalin = 0;		// start line

REAL	sinpp  = 0.0,
	cospp  = 1.0;		/* specifying angle of picture plane	*/
INT	penup = 1;		/* indicates next draw should be a "move" */
INT	bitcol = 1;		/* bit-mapped colour, 1, 2, or 3	*/

int	maxbyte;		/* byte width of printer image		*/
char	drawch;			/* 'M' or 'L', for use by Vectrix driver*/
REAL	xgorigin,ygorigin,	/* constants for screen-transformation	*/
	xgslope,ygslope; 	/* slopes for screen transformation	*/
REAL	prevx,currx;		/* x-values for use in interpolation	*/
REAL	rotn,heelv;
INT	noline;

char  	*editsect;
char  	*alsosect;
char	*showline;
int	*editsectnum;
int	*alsosectnum;
int	*showlinenum;
char	*builders_filename;

int     view_on_load = 1;       /* 0 for none, 1 for triple, 2 for
				top, 3 for elev, 4 for end      */
void use_hull(int);

#ifdef EXT_OR_PROF
int	surfsp;			/* space needed for hidden surface data	*/
#endif
#ifndef STUDENT
REAL	epsilon;		/* prop shaft inclination		*/
REAL	dist_down;		/* distance of prop shaft below transom	*/
#endif

/*	zoom window limits						*/

#ifdef PLATEDEV
REAL	xmin1,xmax1,ymin1,ymax1;
#endif

int *master;			/* master section tags */
int *autofair,*autofair_main,*autofair_overlay,*autofair_merge;	/* line auto-fairable tags */
/*	Program uses drop-down menus, to each of whose options may be	*/
/*	associated a function which invokes the option			*/

/*	Representative menu / function tables lie at the start of the	*/
/*	main program							*/

int xleft,ytop,xright,ybottom;
RECT clrect;		/* screen-clear rectangle */
int xwleft,ywtop;	/* top left and top of window */
int xwright,ywbottom;
int xchar,ychar;	/* character space increments */
LOGFONT lf;		/* logical font structure */
LOGFONT prfont;		/* printer font structure */
int pointsize;		/* printer font height	*/
int wpchar,hpchar;	/* printer character sizes */

/*	screen text shadow						*/

char	scr_ch[400][100];
int	scrcopy = 1;	/* screen-copy-mode toggle */
extern	int	xchar,ychar;
int	textcol = 0;
int	textrow = 0;
int	xjust = 55;

/*	left column of workspace					*/

int	lx = 0;

/*	Dialog box position (0 = centre, 1 = bottom right, ...)		*/

int	dlgboxpos = 0;

/*	Colour information						*/

#define RGB(r,g,b) ((COLORREF)(((BYTE)(r)|((WORD)(g)<<8))|(((DWORD)(BYTE)(b))<<16)))

int	white;
int	border;
int	drawcol;
int	xscr,yscr;

/*	Common dialog box help id		*/

int	help_id;
int	HelpRequest = 0;
int	HelpUsed = 0,UpdatesUsed = 0,TutorialUsed = 0,NewIn7Used = 0;

/*	pen controls				*/

HPEN	hPen = 0;
HPEN	hPenOrig;
HBRUSH	hBrush = 0;
HBRUSH	hBrushOrig;

/*	Warning flags				*/

int	tabunowarning = 0;
int	evalnowarning = 0;
int	offsetnowarning = 0;


/*	numeric and string variable names	*/

#define numderiv 1

char	*var[numvar+numstring+numderiv] = {

/*	Numeric variables			*/

"WATERL","HEEL"  ,"DISPLA","PITCH" ,"XCOFM" ,"VOLUME","LCB"   ,"LCF",
"GZ"    ,"ZMETA" ,"MCT"   ,"MPI"   ,"WPLANE","WETSUR","LOA"   ,"LWL",
"BMAX"  ,"BWL"   ,"DRAFT" ,"FREEBD","CM"    ,"CP"    ,"CB"    ,"XENTRY",
"ZCOFM" ,"AABOVE","ZABOVE","ABELOW","ZBELOW","SPGRAV","MXAREA","SARM",
"ENTANG","ZCOFB" ,"ZBASE" ,"XSTEM" ,"XSTERN","VOLFOR","VOLSTE","XATBWL",
"XCOFB" ,"XLCF"  ,"XMID"  ,"DENSTY","GRAVTY","KN"    ,"XABOVE","XBELOW",
"DRAFTF","DRAFTM","DRAFTA","FREEBF","FREEBM","FREEBA","SURLCG","SURVCG",

/*	Strings					*/

	"LENUN" ,"MASUN",

/*	Derived parameters			*/

	"ZBASE"};

#ifndef PLATEDEV
int tran_lines;
#endif

#ifdef STUDENT
int tabusel[MAXSEL] = {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};
int dragscheme,stabvar,xvar,yvar,tvar,twin_screw;
REAL plot_start,plot_end,plot_inc,speed,prop_diam,pd_ratio,
pd_ratio,epsilon,dist_down,def_str_interv,def_str_firstint,
def_str_width,def_str_thick,def_str_direc;
#ifndef EXT_OR_PROF
int dxf_dim;
#endif
#else
int tabusel[MAXSEL] = {2,18,11,10,9,6,7,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};
int stabvar = 0;

/*	Direction correction factors, and other things	*/

REAL varsign[numvar] = {
-1.0, 1.0, 1.0, 1.0, 1.0, 1.0,100.,100.,
 1.0,-1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
 2.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
-1.0, 1.0, 1.0, 1.0,-1.0, 1.0, 1.0,0.01745329,
 1.0,-1.0,-1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
 1.0, 1.0, 1.0, 1.0};

#endif

/*	previous function, set in appropriate routines for use in hardcopy */

FUNC_PTR menufunc;
FUNC_PTR prev_func = NULL;
FUNC_PTR update_func = NULL;
FUNC_PTR print_func = NULL;
int rotatable;

char *warn= "IT IS STRONGLY RECOMMENDED THAT YOU SAVE YOUR CURRENT HULL DATA SET BEFORE PROCEEDING. ALL EXPLICIT SECTION DATA YOU HAVE WILL PROBABLY BE LOST.";

int	xnext = 0,
	ynext = 0;	/* stepped x and y values	*/

int	changed = 0;	/* design-changed flag	*/

REAL	x1,x2,yy1,y2;
REAL	pp	= 0.0;

/*	Additional run menu items			*/

int  numprog = 0;
int addarg[8];
int reload[8];

/*	drawing resolution parameters			*/

int numbetwl = 9;	/* points on section between lines */
int numbetw = 0;	/* points on line between sections */

DWORD   dwStyle = WS_OVERLAPPEDWINDOW;
HINSTANCE  hInst;
HINSTANCE  hPrevInst;
HINSTANCE hInstLib;
HWND    hWnd = NULL;
HWND	hWndMain = NULL;
HWND	hWndToolbar = NULL;
HWND	hWndView = NULL;
HWND	hWndArrange;
HCURSOR hCursor = NULL;
HDC     hDC = NULL;  /* screen plot device context */
HDC	hcDC;	     /* hardcopy device context */
int     initprog();
HINSTANCE  ThisInst;

/*	Alternate window message processing modes	*/

int	graphic_edit = 0;	/* graphic-edit mode flag */
int	smoothing_line = 0;	/* smooth-line mode flag */
void	window(HWND hParent,char *title,int xorigin,int yorigin,int xsize,int ysize,
	char *winclass,DWORD style,HWND *hWndRet);
void	GetSizes(HWND);
HMENU	hMainMenu;
void	writetext(int x,int y,char *b,int l);

char	*cfgfile;

HWND *ToolWnd;
int numtool = 0;
UINT *tool_id;
int ToolDn = -1;
int arranging = 0;
int winl,wint,winw,winh;
HFONT hFontOld;

int *Bitmap;
char (*tbtext)[33];

/*	More dynamically allocated storage	*/

char (*progtext)[17];
char (*progname)[MAX_PATH];
char (*tankdesc)[MAX_PATH];
char (*lastname)[MAX_PATH];
char (*lastpath)[MAX_PATH];

HBRUSH bg;
HBRUSH tb;
HBRUSH hBkgd;
HBRUSH hWhite;
int rBkgd = 192;
int gBkgd = 192;
int bBkgd = 192;
COLORREF rgbBkgd;
int rWhite = 255;
int gWhite = 255;
int bWhite = 255;
COLORREF rgbWhite;

int HintDelay = 2000;

/*	Colour information						*/

long scrcolour[10]= {	RGB(255,255,255),
			RGB(0,0,0),
			RGB(255,0,0),
			RGB(0,255,0),
			RGB(0,0,255),
			RGB(255,255,0),
			RGB(255,0,255),
			RGB(0,255,255),
			RGB(0,0,0),
			RGB(0,255,255)};
int solidshade = 0;
int linestyle[5] = {0,0,0,0,0};
int linewidth[5] = {0,0,0,0,0};

int undoruling = 0;
int xleft,ytop,xright,ybottom;

#ifdef linux
char browser[MAX_PATH] = "/usr/bin/netscape";
#endif
