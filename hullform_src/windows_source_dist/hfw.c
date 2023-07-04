/* Hullform component - hfw.c
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
 
#include "hulldesi.h"   /* declare common data as external */

void cleardraw(void);

#include <GL/gl.h>
#include <GL/glu.h>

extern int temp_gl;
extern HWND save_gdi_hwnd;
extern HDC save_gdi_hdc;
extern HMENU ShadeViewMenu;
extern int repaint;
extern HGLRC hRC;
extern int pixelformat;
extern int viewlist;

int	scrdev = 0;

void movmem(char *src,char *dst,int size);
void fp_abort();	/* floating point abort routine */
void runprog(char *prog,int addarg,int reload);
void makemenu(void);
int graphic_screen(void);
int text_screen(void);
void update_all(HWND hWndDlg);
void do_balance(void);
int parse(char *line);
int same(char *s1,char *s2);
void finish_stats(void);
ATOM aAppl = 0,aTopic = 0,aItem = 0;
#ifdef DEMO
void init_demo(void);
#endif
void	initzoom(void);
void DrawTool(HWND hWnd,int down);
void DestroyToolbar(void);
int toolheight = 32,toolwidth = 32;
void InitToolbar(void);
void InitWorkwin(int scroll);
void CreateScrollBars(void);
void DestroyScrollBars(void);
void finish_scale(void);
void calctankstat(void);
void bal_all(void);
void update_picklist(char *lastname,char * lastpath);
int repaint = TRUE;	/* allow repaint */
int working = FALSE;
int focus = FALSE;
int dragscroll = FALSE;
int ScrollBarWidth = 0;
int ScrollBarHeight = 0;
int ddemode = 0;
extern int shiftdown;
void messagedata(char *,char *);
void save_hull(int);
int sendrecv(char *cmd,char *arg,char *out,REAL *result);

#ifdef DEMO
void check_message(WPARAM wparam);
#endif

#ifdef HULLSTAT
extern int statedit_mode;
#ifdef DEMO
int allow_hullstat
 = FALSE;	/* may be set TRUE in demo version */
#endif
int level[40];
char op[40];
char *par[40];	/* pointers to parameters in evaluation string	*/
#else
int level[2];
char op[2];
char *par[4];	/* pointers to parameters in evaluation string	*/
#endif
int parse0 = 0;
int brlev = 0;
int paused = 0;
extern REAL wate_int;
RECT rc_startup = {
	0,0,0,0};
extern REAL *outl_thickness;

char *helpfile;		/* help file name */
int htmlhelp = 1;	// html help mode flag

int	v7mode = FALSE;

/*	Line smoothing arrays	*/

char	*alsolines;
char	*masterstring;
char	*inpsmoo;
char	*relsmoo;
int 	*ignsmoo;
int		*relax;
REAL	offsetb[maxsec+2];
int		*line_also;
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

char *lenun[4] = {
	"m ","m ","ft","ft"};
/*				table of length units		*/

char *masun[4] = {
	"kg","tonne","lb","ton"};
/*				table of mass units		*/

/*	Perspective projection locations for view point and centre	*/
/*	of viewed planed						*/

REAL	yview = 0.0,
zview = 1000.0;

REAL	xpersp = 0.0,
ypersp = 0.0,
zpersp = 9999.0;

REAL g[4] = {
	9.8,9.8,32.0,32.0};
/*				acceleration due to gravity	*/

REAL densit[4] = {
	1025,1.025,64.0625,0.0285991};
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

extern REAL (*xl_dxf)[maxsec][maxlin];

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
REAL	viscos[4] = {
	1.191e-6,1.191e-6,1.282e-5,1.282e-5};

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

REAL	tocons[2][2] = {
	{
		1.0,0.514773												}
	,	/* conversion to m/s	*/
	{
		1.0,1.68889												}
};	/* conversion to ft/s	*/

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
REAL	Xwidth;				/* size of plotted region (real measurements) */
REAL	Ywidth;
REAL	hardcopy_Xwidth,hardcopy_Ywidth;
REAL	xcurr,ycurr,zcurr;		/* current graphic y-positions	*/
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

REAL	skinthickness;
int	shownumbers = 0;	/* numbers on sections default off */
int	shownames = 0;		/* names also default off */
int	showoverlay = 0;	/* overlay plotting defaults off */
int	surfaceset = 0;		/* 1 if a surface set is loaded */
char	(*surfname)[MAX_PATH];	/* will allow up to 8 surfaces */
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

int zoomin = 0;
extern int zoomable_view(void);
void frame_view(int xzleft,int xzright,int yztop,int yzbottom);
extern HGLRC hRC;
extern int viewlist;

int	scrollable = FALSE;	/* scroll bars usable		*/
int	horzpos = 0,vertpos = 0;/* scroll bar positions		*/

#ifdef EXT_OR_PROF
int	animate = 0;		/* animation-active flag	*/
#endif

INT	inisec,lassec;
int	endlin = 0;		/* end line, set depending on value of "showtanks */
int stalin = 0;

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

char	scr_ch[40][100];
int	scrcopy = 1;	/* screen-copy-mode toggle */
extern	int	xchar,ychar;
int	textcol = 0, textrow = 0;
int	xjust = 55;

/*	left column of workspace					*/

int	lx = 0;

/*	Dialog box position (0 = centre, 1 = bottom right, ...)		*/

int	dlgboxpos = 0;

/*	Colour information						*/

COLORREF scrcolour[10]={
	RGB(255,255,255),
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
int linestyle[5] = {
	0,0,0,0,0};
int linewidth[5] = {
	0,0,0,0,0};

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
int tabusel[MAXSEL] = {
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};
int dragscheme[6],stabvar,twin_screw;
REAL plot_start,plot_end,plot_inc,speed,prop_diam,pd_ratio,
pd_ratio,epsilon,dist_down,def_str_interv,def_str_firstint,
def_str_width,def_str_thick,def_str_direc;
#ifndef EXT_OR_PROF
int dxf_dim;
#endif
#else
int tabusel[MAXSEL] = {
	2,18,11,10,9,6,7,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};
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

FUNC_PTR menufunc;				// function called from menu
FUNC_PTR prev_func = NULL;		// previous function - used only for clipboard, print and plot selection
FUNC_PTR update_func = NULL;	// screen update function - used to redraw the screen. Sometimes the same as menufunc
FUNC_PTR print_func = NULL;
int rotatable;

char *warn= "IT IS STRONGLY RECOMMENDED THAT YOU SAVE YOUR CURRENT HULL DATA SET BEFORE PROCEEDING. ALL EXPLICIT SECTION DATA YOU HAVE WILL PROBABLY BE LOST.";

char *no_memory = "Insufficient free memory - Windows memory management is probably corrupted.\n\nIt is suggested you exit Hullform and reboot.";

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

extern void new_hull(),load_file(),save_file(),save_file_as(),

#ifdef EXT_OR_PROF
overlay_hull(),
#endif
dynam_waterline(),
import(),
mesh3d(),
dxf_mesh(),
orc_offsets(),
waterline_output(),
builders_output(),do_hardcopy(),
do_print(),do_plot(),quit();

#ifdef PROF
    extern void dxf_input();
#endif
extern int GL_surfaces_need_redraw;
extern REAL GLscale,GLxshift,GLyshift;

#ifdef PROF
extern void dxf_output();
extern void dxf_frames_output(),dxf_plates_output(),ghs_output(),ghs_input(),read_dxf_overlay();
#endif

FUNC_PTR file_subr[] = {
	new_hull,
	load_file,
	save_file,
	save_file_as,
	dynam_waterline,
#ifdef PROF
	overlay_hull,
#else
	NULL,
#endif
	import,
#ifdef PROF
	dxf_input,
	ghs_input,
	read_dxf_overlay,
#else
	NULL,NULL,NULL,
#endif
	waterline_output,
	builders_output,
#ifdef PROF
	dxf_output,
	dxf_frames_output,
	dxf_plates_output,
	mesh3d,
	ghs_output,
	dxf_mesh,
	orc_offsets,
	do_print,
	do_plot,
#else
	NULL,
	NULL,
	NULL,
	mesh3d,
	NULL,
	NULL,
	NULL,
	do_print,
	NULL,
#endif
	quit,
};

//	This must match the index of the last menu entry above, plus one

#define HISTORYBASE 123

/*	Edit menu functions and function table				*/

int allow_without_file[] = {
	0,TRUE,FALSE,FALSE,FALSE,FALSE,FALSE,FALSE,FALSE,TRUE,TRUE,TRUE,TRUE};

extern void copytoclipboard(),
add_section(),delete_section(),edit_section(),
insert_line(),remove_line(),
rescale(),resection(),
lineprop(),reline(),
def_transom(),
smooth_line(),
edit_alte_new(),fair_stem(),radius_stem(),
mult_add(),convert_units(),edit_alte(),edit_orie(),
make_buttocks(),make_diagonals(),make_waterlines(),
converthull(),movehull();
extern void break_line(),surfaces(),naca(),strake();

FUNC_PTR edit_subr[] = {
	copytoclipboard,
	add_section,
	delete_section,
	edit_section,
	insert_line,
	remove_line,
	smooth_line,
	edit_alte_new,	/* stem-line functions */
	fair_stem,
#ifndef STUDENT
	radius_stem,
#else
	NULL,
#endif

	lineprop,

	mult_add,
	convert_units,
	edit_alte,
#ifndef STUDENT
    edit_orie,
	def_transom,
	edit_alte_new,	/* catamaran functions */
	converthull,
	movehull,
#else
    NULL,NULL,NULL,NULL,NULL,
#endif
    naca
#ifdef PROF
	,strake
#endif
};

/*	View menu							*/

extern void general_orth(),plan_orth(),elev_orth(),end_orth(),
full_persp(),port_persp(),starb_persp(),viewing_pos(),
view_sections(),view_lines(),view_tanks(),
view_diagonals(),view_diag_angle(),view_buttocks(),
view_waterlines(),shaded_surfaces(),
do_hardcopy(),view_options(),shell_expansion();

extern void set_zoomin(),set_zoomout();

FUNC_PTR view_subr[] = {
	general_orth,
	plan_orth,
	elev_orth,
	end_orth,
	full_persp,
	port_persp,
	starb_persp,
#ifdef PROF
	shell_expansion,
#else
	NULL,
#endif
	view_options
#ifdef PROF
    ,initzoom
	,set_zoomin
	,set_zoomout
#else
    ,NULL
	,NULL
	,NULL
#endif

};

/*	Statics menu							*/

extern void stat_settings(),find_float(),balance_all(),find_displ(),
write_stats(),plot_all(),tabular_output(),show_areas(),hullstat();

FUNC_PTR stat_subr[] = {
	stat_settings
    ,find_float
#ifndef STUDENT
    ,balance_all
#else
    ,NULL
#endif
    ,find_displ
    ,write_stats
#ifndef STUDENT
    ,plot_all
    ,tabular_output
    ,show_areas
#else
	,NULL
	,NULL
	,NULL
#endif
#ifdef HULLSTAT
    ,hullstat
#else
	,NULL
#endif
};

/*	Drag menu							*/

#ifdef PROF

extern void set_speed(),set_spunit(),show_areas(),find_gerrit(),find_gerrit96(),
find_holtrop(),find_oortmerrssen(),find_savitsky(),find_savbro(),
plot_drag();

FUNC_PTR drag_subr[] = {
	set_speed,
	show_areas,
	find_gerrit,
	find_gerrit96,
	find_oortmerrssen,
	find_holtrop,
	find_savitsky,
	find_savbro,
	plot_drag
};

/*	Plate Development Menu						*/

extern void set_lines(),calc_devel(),dev_azim(),
dev_pos(),dev_grad(),view_devel(),zoom_devel(),rollout(),
fix_transom_curve(),write_devel(),write_outline(),
plot_devel(),transom_rulings(),write_endpoints(),
undo_ruling(),dev_ignore(),rollout_transom(),do_plate();

FUNC_PTR plate_subr[] = {
	do_plate,
	NULL,
	NULL,
	NULL,
	NULL,
	view_devel,
	undo_ruling,
	rollout,
#ifdef PROF
	rollout_transom,
#else
	NULL,
#endif
	fix_transom_curve,
	NULL,
	write_devel,
	write_outline,
	write_endpoints
};


/*	Tanks menu							*/

extern void calibration(),tank_summary(),edit_tanks();

FUNC_PTR tank_subr[] = {
	edit_tanks,
	NULL,
	NULL,
	NULL,
	calibration,
	tank_summary};

extern void write_stringers(),form_stringers(),delete_stringers(),
develop_stringer();

FUNC_PTR stringer_subr[] = {
	form_stringers,
	delete_stringers,
	develop_stringer,
	view_devel,
	undo_ruling,
	rollout,
	write_stringers};

/*	Run menu						*/

extern void addprog(),delprog();

FUNC_PTR prog_subr[] = {
	addprog,delprog};

FUNC_PTR surf_subr[] = {
	surfaces};

#endif

/*	Config menu							*/

extern void set_plotter(),set_scale(),
set_directory(),set_units(),set_x_positive(),set_positive(),
set_density(),set_colours(),
save_config(),get_config(),set_extra_points(),
set_units(),set_x_positive(),set_z_positive(),
set_detail(),set_linestyles(),set_screenfont(),
set_printerfont(),set_toolbar(),delete_toolbar(),set_toolloc(),
arrange_toolbar(),toolbar_hint(),textedit_lines(),set_elimit();

extern void set_graphics(),set_loadview(),set_editmode();

FUNC_PTR conf_subr[] = {
#ifdef PROF
	set_plotter,
#else
	NULL,
#endif
	set_scale,
	set_printerfont,
	set_directory,
	set_units,
	set_density,
	set_x_positive,
	set_z_positive,
	set_graphics,
	set_colours,
	set_linestyles,
	set_detail,
	set_screenfont,
	set_loadview,
	set_editmode,
	set_toolbar,
	delete_toolbar,
	set_toolloc,
	arrange_toolbar,
	toolbar_hint,
	save_config,
	get_config
#ifdef linux
	,set_browser
#else
	,NULL
#endif
	,textedit_lines
	,set_elimit
};

/*	Help menu							*/

extern void help(),help_contents(),helponhelp(),about(),show_memory(),updates(),tutorial(),http_hullform();
FUNC_PTR help_subr[] = {
	help_contents,helponhelp,about,show_memory,updates,tutorial,http_hullform
};

#ifdef PROF

/*	Hullstat menu							*/

extern void hullstat();
FUNC_PTR hullstat_subr[] = {
	hullstat};

#endif


DWORD   dwStyle = WS_OVERLAPPEDWINDOW;
HINSTANCE  hInst;
HINSTANCE  hPrevInst;
HINSTANCE hInstLib;
HWND    hWnd = NULL;
HWND	hWndMain = NULL;
HWND	hWndView = NULL;
HWND	hWndToolbar = NULL;
HWND	hWndArrange;
HCURSOR hCursor = NULL;
HDC     hDC = NULL;  /* screen plot device context */
HDC		hcDC;	     /* hardcopy device context */
int     initprog();
HINSTANCE  ThisInst;

int		xp_abort = FALSE;

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

INT WindowProcedure(HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);
INT ButtonProcedure(HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);
BOOL CALLBACK HintProc(HWND hWndDlg,unsigned msg,WPARAM wParam,LPARAM lParam);

int InitWin(HINSTANCE hInstance,WNDPROC WinProc,int style,char *class,char *menu,char *icon,HBRUSH hbr);

HBRUSH bg,tb;
HBRUSH hBkgd,hWhite;
int rBkgd = 192;
int gBkgd = 192;
int bBkgd = 192;
COLORREF rgbBkgd;
int rWhite = 255;
int gWhite = 255;
int bWhite = 255;
COLORREF rgbWhite;

INT TextOnGrayProc(HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);

int HintDelay = 2000;

/*      Main Program - initialise, create window, pass messages */

#pragma argsused
int WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance,LPSTR lpCmdLine,int nCmdShow)
{

	MSG msg;
	char *p,*q;
	HFONT	hFont;
	extern	HDC hDC;
	extern int	xchar,ychar;
	TEXTMETRIC	tm;
	int		i;
	FILE	*fp;
	extern void read_hullfile(FILE *fp,char *hullfile,int all);
	char	open_error[MAX_PATH];
	char	filepath[MAX_PATH];
	int		WindowStyle = WS_OVERLAPPEDWINDOW;
	void	decode(char *,char *);

	hInst = hInstance;
	hPrevInst = hPrevInstance;

	rgbBkgd = RGB(rBkgd,gBkgd,bBkgd);
	rgbWhite = RGB(rWhite,gWhite,bWhite);
	hBkgd = CreateSolidBrush(rgbBkgd);
	hWhite = CreateSolidBrush(rgbWhite);

	/*	Initialise hull line pointers				*/

	i = sizeof(*yline);
	if(

			!memavail((void *) &helpfile,MAX_PATH) ||
		    !memavail((void *) &ToolWnd,sizeof(HWND)*NUMTOOL) ||
		    !memavail((void *) &tool_id,sizeof(UINT)*NUMTOOL) ||
		    !memavail((void *) &progtext,sizeof(*progtext)*12) ||
		    !memavail((void *) &progname,sizeof(*progname)*12) ||
		    !memavail((void *) &lastname,sizeof(*lastname)*12) ||
		    !memavail((void *) &lastpath,sizeof(*lastpath)*12) ||
		    !memavail((void *) &port,3*MAX_PATH) ||
		    !memavail((void *) &surfline,40*sizeof(int)) ||
		    !memavail((void *) &Bitmap,NUMTOOL*sizeof(int)) ||
			!memavail((void *) &tbtext,33*NUMTOOL) ||
		    !memavail((void *) &master,maxsec*sizeof(int)) ||
		    !memavail((void *) &varval,sizeof(REAL)*numvar) ||
		    !memavail((void *) &ignore,sizeof(int)*maxsec) ||
		    !memavail((void *) &ignline,MAX_PATH) ||
		    !memavail((void *) &alsosect,MAX_PATH) ||
		    !memavail((void *) &showline,MAX_PATH) ||
		    !memavail((void *) &editsectnum,maxsec*sizeof(int)) ||
		    !memavail((void *) &alsosectnum,maxsec*sizeof(int)) ||
		    !memavail((void *) &showlinenum,maxlin*sizeof(int)) ||
		    !memavail((void *) &builders_filename,MAX_PATH) ||


	#ifdef PROF
			!memavail((void *) &xvalue,100*sizeof(REAL)) ||
		    !memavail((void *) &yvalue,100*sizeof(REAL)) ||
		    !memavail((void *) &xsect_overlay,(maxsec+4)*sizeof(REAL)) ||
		    !memavail((void *) &yline_overlay,i) ||
		    !memavail((void *) &ycont_overlay,i) ||
		    !memavail((void *) &zline_overlay,i) ||
		    !memavail((void *) &zcont_overlay,i) ||
		    !memavail((void *) &stsec_overlay,maxlin*sizeof(int)) ||
		    !memavail((void *) &ensec_overlay,maxlin*sizeof(int)) ||
		    !memavail((void *) &radstem_overlay,maxlin*sizeof(REAL)) ||
		    !memavail((void *) &relcont_overlay,(maxlin+3)*sizeof(int)) ||
		    !memavail((void *) &autofair_overlay,maxlin*sizeof(int)) ||
		    !memavail((void *) &linewt_overlay,i) ||
		    !memavail((void *) &hullfile_overlay,MAX_PATH) ||
			!memavail((void *) &xsect_main,(maxsec+4)*sizeof(REAL)) ||
		    !memavail((void *) &yline_main,i) ||
		    !memavail((void *) &ycont_main,i) ||
		    !memavail((void *) &zline_main,i) ||
		    !memavail((void *) &zcont_main,i) ||
		    !memavail((void *) &stsec_main,maxlin*sizeof(int)) ||
		    !memavail((void *) &ensec_main,maxlin*sizeof(int)) ||
		    !memavail((void *) &radstem_main,maxlin*sizeof(REAL)) ||
		    !memavail((void *) &relcont_main,(maxlin+3)*sizeof(int)) ||
		    !memavail((void *) &autofair_main,maxlin*sizeof(int)) ||
		    !memavail((void *) &linewt_main,i) ||
		    !memavail((void *) &hullfile_main,MAX_PATH) ||
		    !memavail((void *) &merge_hullfile,MAX_PATH) ||
			!memavail((void *) &fl_line1,(MAXTANKS+1)*sizeof(int)) ||
		    !memavail((void *) &fl_fixed,MAXTANKS*sizeof(int)) ||
		    !memavail((void *) &fl_right,MAXTANKS*sizeof(int)) ||
		    !memavail((void *) &tankdesc,sizeof(*tankdesc)*MAXTANKS) ||
		    !memavail((void *) &numstr,maxlin*sizeof(int)) ||
		    !memavail((void *) &inistr,maxlin*sizeof(int)) ||
		    !memavail((void *) &str_dir,maxlin*sizeof(int)) ||
		    !memavail((void *) &str_thk,maxlin*sizeof(REAL)) ||
		    !memavail((void *) &str_wid,maxlin*sizeof(REAL)) ||
		    !memavail((void *) &str_interv,maxlin*sizeof(REAL)) ||
		    !memavail((void *) &str_firstint,maxlin*sizeof(REAL)) ||
		    !memavail((void *) &strmode,maxlin*sizeof(int)) ||
		    !memavail((void *) &radstem,maxlin*sizeof(REAL)) ||
		    !memavail((void far *) &xtran,3*sizeof(REAL)) ||
		    !memavail((void *) &tankdesc,sizeof(*tankdesc)*MAXTANKS) ||
		    !memavail((void *) &numstr,maxlin*sizeof(int)) ||
		    !memavail((void *) &inistr,maxlin*sizeof(int)) ||
		    !memavail((void *) &str_dir,maxlin*sizeof(int)) ||
		    !memavail((void *) &str_thk,maxlin*sizeof(REAL)) ||
		    !memavail((void *) &str_wid,maxlin*sizeof(REAL)) ||
		    !memavail((void *) &str_interv,maxlin*sizeof(REAL)) ||
		    !memavail((void *) &str_firstint,maxlin*sizeof(REAL)) ||
		    !memavail((void *) &strmode,maxlin*sizeof(int)) ||
		    !memavail((void *) &radstem,maxlin*sizeof(REAL)) ||
		    !memavail((void far *) &xtran,3*sizeof(REAL)) ||
		    !memavail((void *) &tankpointer,sizeof(REAL)*12*MAXTANKS) ||
			!memavail((void *) &developed,(maxlin+3)*sizeof(int)) ||
		    !memavail((void *) &rulings,maxlin*sizeof(int)) ||
	#else

		/*	line arrays allocated as yline_main, yline_overlay in other versions	*/

			!memavail((void *) &yline,i) ||
		    !memavail((void *) &zline,i) ||
		    !memavail((void *) &ycont,i) ||
		    !memavail((void *) &zcont,i) ||
		    !memavail((void *) &relcont,(maxlin+3)*sizeof(int)) ||
		    !memavail((void *) &autofair,maxlin*sizeof(int)) ||
		    !memavail((void *) &stsec,maxlin*sizeof(int)) ||
		    !memavail((void *) &ensec,maxlin*sizeof(int)) ||
		    !memavail((void *) &xsect,(maxsec+4)*sizeof(REAL)) ||
	#endif

			!memavail((void *) &alsolines,MAX_PATH) ||
		    !memavail((void *) &masterstring,MAX_PATH) ||
		    !memavail((void *) &inpsmoo,MAX_PATH) ||
		    !memavail((void *) &relsmoo,MAX_PATH) ||
		    !memavail((void *) &ignsmoo,maxsec*sizeof(int)) ||
		    !memavail((void *) &relax,maxsec*sizeof(int)) ||
		    !memavail((void *) &line_also,(maxlin+1)*sizeof(int)) ||
		    !memavail((void *) &linesave,(maxsec+2)*sizeof(REAL)) ||
			!memavail((void *) &cfgfile,MAX_PATH) ||
		    !memavail((void *) &linestoplot,256) ||
		    !memavail((void *) &sectionstoplot,256) ||
		    !memavail((void *) &hwl,(maxsec+4)*sizeof(REAL)) ||
		    !memavail((void *) &sectname,maxsec*sizeof(*sectname)) ||
		    !memavail((void *) &textpointer,12*MAX_PATH) ||
		    !memavail((void *) &lineon,sizeof(int)*maxlin) ||
		    !memavail((void *) &secton,sizeof(int)*maxsec) ||
		    !memavail((void *) &outl_thickness,sizeof(REAL)*maxlin)) {
		message(no_memory);
		PostQuitMessage(0);
		return 0;
	}

	strcpy(ignline,"NONE");
	strcpy(inpsmoo,"NONE");
	strcpy(relsmoo,"ALL");
	strcpy(builders_filename,"hullform.txt");
	hullfile =   textpointer;
	dirnam =     textpointer + MAX_PATH;
	filedirnam = textpointer + 2*MAX_PATH;
	dxfdirnam =  textpointer + 3*MAX_PATH;
	surfname =   (void *) (textpointer + 4*MAX_PATH);
	cur_port = port + MAX_PATH;
	text_device = cur_port + MAX_PATH;
	mainline = &surfline[8];
	useovl   = &mainline[8];
	usemain  = &useovl[8];
	mergedn  = &usemain[8];
	*cfgfile = 0;
	strcpy(alsosect,"ALL");
	strcpy(showline,"ALL");
	strcpy(alsolines,"NONE");
	for(i = 0 ; i < maxsec ; i++) {
		ignsmoo[i] = FALSE;
		relax[i] = TRUE;
		;
	}
	for(i = 0 ; i < maxlin ; i++) line_also[i] = FALSE;
#ifdef PROF
	for(i = 0 ; i < maxlin+2 ; i++) developed[i] = -1;

	fl_volum = tankpointer;
	fl_fract = &fl_volum[MAXTANKS];
	fl_spgra = &fl_fract[MAXTANKS];
	fl_walev = &fl_spgra[MAXTANKS];
	fl_perm  = &fl_walev[MAXTANKS];
	tankmass = &fl_perm[MAXTANKS];
	tankvcg  = &tankmass[MAXTANKS];
	tanklcg  = &tankvcg[MAXTANKS];
	tanklcf  = &tanklcg[MAXTANKS];
	tanktfsm = &tanklcf[MAXTANKS];
	tanklfsm = &tanktfsm[MAXTANKS],
	tankmom  = &tanklfsm[MAXTANKS];
	use_hull(MAINHULL);
	ytran = &xtran[1];
	ztran = &ytran[1];
	itran = (int *) &ztran[1];
#endif

	/*	Find the help file	*/

//	Look in startup directory first

	GetModuleFileName(hInstance,filepath,sizeof(filepath));
	p = strchr(filepath,0);
	while(p != filepath && *p != '\\') p--;
	if(*p == '\\') p++;

//	Try for local chm file

	strcpy(p,"hullform.chm");
	fp = fopen(filepath,"r");
	if(fp == NULL) {

//	Look for chm in search path

		q = searchpath("hullform.chm");
		if(q == NULL) {
			message("Can not find help file - online\nhelp will not be available.");
			p = "";
		} else {
			p = q;
			htmlhelp = TRUE;
		}
	}
	strcpy(helpfile,p);

/*	Read program configuration				*/

//	Try the user's local application data area

	q = getenv("LOCALAPPDATA");
	if(q != NULL) {
		strcpy(filepath,q);
		strcat(filepath,"\\Hullform 9\\hullform.cfg");
		fp = fopen(filepath,"r");
	}

//	Try the same directory as the program file

	if(fp == NULL) {
		GetModuleFileName(hInstance,filepath,sizeof(filepath));
		strlwr(filepath);
		p = strrchr(filepath,'\\');
		if(p == NULL)
			p = filepath;
		else
			p++;
		strcpy(p,"hullform.cfg");
		fp = fopen(filepath,"r");
	}

//	Try the user's file search path

	if(fp == NULL) {
		q = searchpath("hullform.cfg");
		if(q != NULL) {
			strcpy(filepath,q);
			fp = fopen(filepath,"r");
		}
	}

	if(fp != NULL) {
		fclose(fp);
		strcpy(cfgfile,filepath);
	} else {
		message("Can not find a configuration file:\nprogram defaults presumed");
		*cfgfile = 0;
	}
	get_config();

	tb = CreateSolidBrush(GetSysColor(COLOR_MENU));	/* toolbar background */
	bg = CreateSolidBrush(scrcolour[0]);			/* window background */

	/*	Register required window classes	*/

	hMainMenu = LoadMenu(hInst,"HULLFORM_MEN");
	if(!hPrevInstance) {
		if(!InitWin(hInst,(WNDPROC) WindowProcedure,0,
			"HFCLASS",NULL,"A_YACHT",bg)
			    || !InitWin(hInst,ButtonProcedure,0,
			"TOOLBAR",NULL,NULL,tb)
			    ) {
			return(FALSE);
		}
	}

	/*	Window size	*/

	xmaxi = GetSystemMetrics(SM_CXSCREEN);
	ymaxi = GetSystemMetrics(SM_CYSCREEN);
	if(rc_startup.right > xmaxi) {	/* probably maximised */
		if(rc_startup.left < 0)
			WindowStyle |= WS_MAXIMIZE;
		else
		    rc_startup.right = 0;	/* forces re-initialisation */
	}


	ScrollBarHeight = GetSystemMetrics(SM_CYHSCROLL);
	ScrollBarWidth  = GetSystemMetrics(SM_CXVSCROLL);

	if(rc_startup.bottom > ymaxi) {	/* probably maximised */
		if(rc_startup.top < 0)
			WindowStyle |= WS_MAXIMIZE;
		else
		    rc_startup.bottom = 0;	/* forces re-initialisation */
	}

	if(rc_startup.right <= rc_startup.left) {
		rc_startup.left = 0;
		rc_startup.right = xmaxi;
	}

	if(rc_startup.bottom <= rc_startup.top) {
		rc_startup.top = 0;
		rc_startup.bottom = ymaxi;
	}

	window(NULL,"HULLFORM",rc_startup.left,rc_startup.top,
		rc_startup.right - rc_startup.left,
		rc_startup.bottom - rc_startup.top,
		"HFCLASS",WindowStyle,&hWndMain);

	if(!hWndMain) {
		return(0);
	}
	else {

		SetCursor(LoadCursor(NULL,IDC_ARROW));

#ifdef PROF
		DrawMenuBar(hWndMain);
#endif

		/*	hDC is the device context for the working window hWnd, defined in Initworkwin	*/

		hBrushOrig = SelectObject(hDC,GetStockObject(BLACK_BRUSH));
		hPenOrig = SelectObject(hDC,GetStockObject(BLACK_PEN));
		setup(scrdev);
		(*init)();

		/*	Initialise font			*/

		hFont = CreateFontIndirect(&lf);
		hFontOld = SelectObject(hDC,hFont);
		GetTextMetrics(hDC,&tm);
		ychar = tm.tmHeight;
		xchar = tm.tmAveCharWidth;

		makemenu();

		/*	Open any file named in program command line */

		p = lpCmdLine;

		if(*p != '\0') {
			if(*p == '"') p++;	/* skip any leading quote - trailing one done later */
			if(strchr(p,'\\') == NULL) {
				strcpy(filepath,dirnam);
				strcat(filepath,"\\");
				strcat(filepath,p);
			}
			else {
				strcpy(filepath,p);
			}
			if( (p = strchr(filepath,'"')) != NULL) *p = 0;	/* strip trailing quote */
			if(strstr(filepath,".hud") == NULL && strstr(filepath,".HUD")
				    == NULL) strcat(filepath,".HUD");
			if( ( fp = fopen(filepath,"rt") ) == NULL) {
				sprintf(open_error,"Can not open file\n%s",filepath);
				message(open_error);
			}
			else {
				strcpy(hullfile,filepath);
				read_hullfile(fp,hullfile,TRUE);
#ifdef PROF
				save_hull(MAINHULL);
#endif
			}
		}

		/*	Main message loop	*/

		while(GetMessage(&msg,NULL,0,0)) {	// NULL was hWndMain, exit on WM_QUIT
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			if(xp_abort) break;
		}

		/*	Housekeeping on exit		*/


#ifndef PROF

		/*	line arrays allocated as yline_main, yline_overlay in other versions	*/

		memfree(yline);
		memfree(zline);
		memfree(ycont);
		memfree(zcont);
		memfree(autofair);
		memfree(relcont);
		memfree(stsec);
		memfree(ensec);
		memfree(xsect);
#endif

		memfree(helpfile);
		memfree(progtext);
		memfree(progname);
		memfree(lastname);
		memfree(lastpath);
		memfree(port);
		memfree(surfline);
		memfree(Bitmap);

		memfree(tbtext);
		memfree(master);
		memfree(varval);
		memfree(ignore);
		memfree(ignline);
		memfree(alsosect);
		memfree(showline);
		memfree(editsectnum);
		memfree(alsosectnum);
		memfree(showlinenum);
		memfree(builders_filename);

#ifdef PROF
		memfree(xvalue);
		memfree(yvalue);
		memfree(xsect_overlay);
		memfree(yline_overlay);
		memfree(ycont_overlay);
		memfree(zline_overlay);
		memfree(zcont_overlay);
		memfree(stsec_overlay);
		memfree(ensec_overlay);
		memfree(radstem_overlay);
		memfree(relcont_overlay);
		memfree(xsect_main);
		memfree(yline_main);
		memfree(ycont_main);
		memfree(zline_main);
		memfree(zcont_main);
		memfree(stsec_main);
		memfree(ensec_main);
		memfree(radstem_main);
		memfree(relcont_main);
		memfree(linewt_main);
		memfree(linewt_overlay);
		memfree(merge_hullfile);
		memfree(hullfile_overlay);
		memfree(hullfile_main);
#endif

#ifndef STUDENT
		memfree(fl_line1);
		memfree(fl_fixed);
		memfree(fl_right);
		memfree(tankdesc);
		memfree(numstr);
		memfree(inistr);
		memfree(str_dir);
		memfree(str_thk);
		memfree(str_wid);
		memfree(str_interv);
		memfree(str_firstint);
		memfree(strmode);
		memfree(xtran);
		memfree(tankpointer);
#endif

#ifdef PROF
		memfree(developed);
		memfree(rulings);
		free_dev();
#endif

		memfree(alsolines);
		memfree(masterstring);
		memfree(inpsmoo);
		memfree(relsmoo);
		memfree(ignsmoo);
		memfree(relax);
		memfree(line_also);
		memfree(linesave);
		memfree(cfgfile);

		memfree(linestoplot);
		memfree(sectionstoplot);
		memfree(hwl);
		memfree(sectname);
		memfree(textpointer);
		memfree(lineon);
		memfree(secton);
		memfree(outl_thickness);

#ifdef PROF
		if(xl_dxf != NULL) memfree(xl_dxf);
#endif

		if(hCursor != NULL) DestroyCursor(hCursor);

		DestroyToolbar();
		memfree(ToolWnd);
		memfree(tool_id);
		hFont = SelectObject(hDC,hFontOld);
		DeleteObject(hFont);
		GDIen();
		DestroyMenu(hMainMenu);
		DestroyWindow(hWnd);
		DestroyWindow(hWndMain);
		UnregisterClass("HFCLASS",hInst);
		UnregisterClass("SMOOCLASS",hInst);
		UnregisterClass("TOOLBAR",hInst);
		DeleteObject(bg);
		DeleteObject(tb);
		DeleteObject(hWhite);
		DeleteObject(hBkgd);
		return(msg.wParam);
	}
}

#pragma argsused
void window(HWND hParent,char *title,int xorigin,int yorigin,
	int xsize,int ysize,char *winclass,DWORD style,HWND *hWndNew)
{
	*hWndNew = CreateWindow(
		winclass,
		title,
		style | CS_DBLCLKS,
		xorigin,
		yorigin,
		xsize,
		ysize,
		hParent,
		NULL,
		hInst,
		NULL);
	if(*hWndNew) {
		ShowWindow(*hWndNew,SW_SHOWNORMAL);
		UpdateWindow(*hWndNew);
	}
}

/*      Window initialisation procedure         */

int InitWin(HINSTANCE hInstance,WNDPROC WinProc,int style,
	char *winclass,char *menu,char *icon,HBRUSH hbg)
{
	WNDCLASS wc;
	int result;

	wc.style            = style | CS_OWNDC | CS_DBLCLKS;
	wc.lpfnWndProc      = WinProc;
	wc.cbClsExtra       = 0;
	wc.cbWndExtra       = 0;
	wc.hInstance        = hInstance;
	wc.hIcon            = LoadIcon(hInstance,icon);
	wc.hCursor          = NULL;
	wc.hbrBackground    = hbg;
	wc.lpszMenuName     = menu;
	wc.lpszClassName    = winclass;

	result = RegisterClass(&wc);

	return result;
}

LRESULT	proczoom(HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);
LRESULT	procundo(HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);
LRESULT	procedit(HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);
LRESULT	procshadeview(HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);

#ifdef HULLSTAT
LRESULT	procstatedit(HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);
#endif
int	undoruling = 0;
int	context_id = 0;
char *context_string = NULL;

HWND	hHint = NULL;
int	currtb = -1;
int	xHint,yHint;
UINT	nTimer;
DWORD	StartTimer;
HWND	HintWnd = NULL;

INT ButtonProcedure(HWND hWndProc,UINT msg,WPARAM wParam,
	LPARAM lParam)
{
	int indtool,j;
	PAINTSTRUCT ps;
	POINT pt;
	MEASUREITEMSTRUCT *m;
	UINT saveID;
	void	KillHint(HWND hWnd);
	int saveBitmap;
	char savetext[33];
	RECT rc;

	for(indtool = 0 ; indtool < numtool ; indtool++)
		if(hWndProc == ToolWnd[indtool]) break;

	if(hHint != NULL && msg == WM_MOUSEMOVE && indtool != currtb)
		KillHint(HintWnd);	/* moved out of current toolbar window */

	if(!working && hHint == NULL && HintDelay > 0 &&
		    msg == WM_MOUSEMOVE && indtool != currtb && indtool < numtool && ToolWnd[indtool] != NULL) {
		currtb = indtool;
		HintWnd = hWndProc;
		StartTimer = GetTickCount();
		nTimer = SetTimer(HintWnd,0,HintDelay,NULL);
	}

	switch(msg) {

	case WM_TIMER:
		if(indtool < numtool && ToolWnd[indtool] != NULL && !working && focus && hHint == NULL &&
			    (int)(GetTickCount() - StartTimer) >= HintDelay) {

			/*	Positively identify the window where the cursor currently lies		*/

			GetCursorPos(&pt);
			for(j = 0 ; j < numtool ; j++) {
				if(ToolWnd[j] != NULL) {
					GetWindowRect(ToolWnd[j],&rc);
					if(pt.x >= rc.left && pt.x < rc.right && pt.y >= rc.top && pt.y < rc.bottom) break;
				}
			}

			/*	If it is within one toolbar window, use that one			*/

			if(j < numtool && ToolWnd[j] != NULL && *tbtext[j] != 0) {
				currtb = j;
				xHint = rc.left;
				yHint = rc.top;
				hHint = CreateDialog(hInst,(char *) TBHINT,hWndProc,(DLGPROC) HintProc);
				KillTimer(HintWnd,nTimer);
			}
		}
		break;

	case WM_LBUTTONDOWN:
		DrawTool(hWndProc,1);
		if(arranging) {
			SetDlgItemText(hWndArrange,100,"Item selected.\nNow drag cursor to\nthe new item position.");
			if(hCursor == NULL) {
				hCursor = LoadCursor(hInst,"TBOUTL");
				SetCursor(hCursor);
			}
		}
		break;

	case WM_MOUSEMOVE:
		if(!arranging) {
			arrowcursor();
			if(ToolDn >= 0 && indtool != ToolDn) {
				DrawTool(ToolWnd[ToolDn],0);
				DrawTool(hWndProc,1);
			}
			else if(wParam == MK_LBUTTON && ToolDn < 0) {
				DrawTool(hWndProc,1);
			}
		}
		break;

	case WM_LBUTTONUP:
		if(arranging && ToolDn >= 0) {
			if(hCursor != NULL) {
				SetCursor(LoadCursor(NULL,IDC_ARROW));
				DestroyCursor(hCursor);
				hCursor = NULL;
			}
			SetDlgItemText(hWndArrange,100,"Re-positioning items");
			saveID = tool_id[ToolDn];
			saveBitmap = Bitmap[ToolDn];
			strcpy(savetext,tbtext[ToolDn]);
			if(indtool >= numtool) indtool = numtool-1;
			if(indtool < ToolDn) {
				for(j = ToolDn ; j > indtool ; j--) {
					tool_id[j] = tool_id[j-1];
					Bitmap[j] = Bitmap[j-1];
					strcpy(tbtext[j],tbtext[j-1]);
				}
			}
			else if(indtool > ToolDn) {
				for(j = ToolDn ; j < indtool ; j++) {
					tool_id[j] = tool_id[j+1];
					Bitmap[j] = Bitmap[j+1];
					strcpy(tbtext[j],tbtext[j+1]);
				}
			}
			tool_id[indtool] = saveID;
			Bitmap[indtool] = saveBitmap;
			strcpy(tbtext[indtool],savetext);
			DestroyToolbar();
			InitToolbar();
			InitWorkwin(scrollable);
		}
		else {
			DrawTool(hWndProc,0);
			PostMessage(hWndMain,WM_COMMAND,tool_id[indtool],0l);
		}
		break;

	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
		if(arranging) {
			arranging = 0;
			SetDlgItemText(hWndArrange,100,"Icon re-arrange ended.\nUse Configure, Write configuration to save changes");
			await(200);
			DestroyWindow(hWndArrange);
		}
		break;

	case WM_COMMAND:
		PostMessage(hWndMain,WM_COMMAND,tool_id[indtool],0l);
		break;

	case WM_MEASUREITEM:
		m = (MEASUREITEMSTRUCT *) lParam;
		m->CtlType = ODT_BUTTON;
		m->CtlID = 0;
		m->itemWidth = 32;
		m->itemHeight = 32;
		return TRUE;
	case WM_PAINT:
		BeginPaint(hWndProc,&ps);
		if(hWndProc != hWndToolbar && indtool < numtool && ToolWnd[indtool] != NULL) DrawTool(hWndProc,0);
		EndPaint(hWndProc,&ps);
		break;
	default:
		return(DefWindowProc(hWndProc,msg,wParam,lParam));
	}
	return(0l);
}

BOOL CALLBACK HintProc(HWND hWndDlg,unsigned msg,WPARAM wParam,LPARAM lParam)
{
	SIZE size;
	int width,height;
	int x,y;
	extern int toolloc;

	switch(msg) {

	case WM_INITDIALOG:
		GetTextExtentPoint32(GetDC(hWndDlg),tbtext[currtb],strlen(tbtext[currtb]),&size);
		width = size.cx + 10;
		height = size.cy + 8;
		switch(toolloc) {
		case 0:	/* top */
			x = xHint;
			y = yHint + 32;
			break;
		case 1:	/* bottom */
			x = xHint;
			y = yHint - height + 2;
			break;
		case 2:	/* left */
			x = xHint + 32;
			y = yHint;
			break;
		case 3:	/* right */
			x = xHint - width + 2;
			y = yHint;
			break;

		}
		(void) MoveWindow(hWndDlg,x,y,width,height,TRUE);
		ShowWindow(hWndDlg,SW_SHOWNOACTIVATE);
		SetDlgItemText(hWndDlg,DLGTEXT,tbtext[currtb]);
		break;

	default:
		return(DefWindowProc(hWndDlg,msg,wParam,lParam));
	}
	return(0l);
}

extern int temp_gl;
extern HWND save_gdi_hwnd;
extern HDC save_gdi_hdc;

INT WindowProcedure(HWND hWndProc,UINT msg,WPARAM wParam,LPARAM lParam)
{
	static FUNC_PTR *functab;
	RECT rc;

#ifdef PROF

	HWND	hWndDDEClient;
	static	DDEDATA	*DDEData = NULL;
	static union {
		DDEACK		DDEReply;
		short int	i;
	}
	conv;
	static	HGLOBAL	hData = NULL;
	char	appname[12],topicname[256],varname[16];
	REAL	reqval;
	char	datastr[MAX_PATH];
	DDEPOKE *DDEPoke;
	static int	was_changed;
	LPARAM	lPar;

	static FUNC_PTR *main_subr[] = {
		file_subr,edit_subr,view_subr,stat_subr,
		drag_subr,plate_subr,tank_subr,stringer_subr,
		surf_subr,prog_subr,conf_subr,help_subr,hullstat_subr												};
#else
	static FUNC_PTR *main_subr[] = {
		file_subr,edit_subr,view_subr,stat_subr,NULL,NULL,NULL,NULL,NULL,NULL,conf_subr,help_subr,NULL												};
#endif

	int         menu_number,menu_entry;
	static int	initialised = 0;
	extern int	xmaxi,ymaxi;
	UINT	li,lj;
	extern int	zoom;

	int		i,j,l;
	extern int	scaled;
	PAINTSTRUCT ps;
	extern COLORREF scrcolour[];
	char	*cp,*cq,*sp;

	FILE	*fp;
	char	filepath[500];
	extern int	toolloc;
	extern int	section_edit;
	REAL	dist,angle;

	static	int outside = 1;
	SIZE	size;
	int		xm,ym,xzleft,xzright,yztop,yzbottom;

#ifdef PROF
	extern int	merging;
	extern int	current_hull;
#endif
	int		relx,rely;
	void	KillHint(HWND hWnd);
	extern FUNC_PTR loadview[];
	FILE *dbg;

#ifdef PROF
	if(zoom)
		return proczoom(hWndProc,msg,wParam,lParam);
	else
#endif

	if(section_edit)
		return procedit(hWndProc,msg,wParam,lParam);
#ifdef PROF
	else if(undoruling)
		return procundo(hWndProc,msg,wParam,lParam);
	else if(statedit_mode)
		return procstatedit(hWndProc,msg,wParam,lParam);
#endif

	switch(msg) {

	case WM_MOUSEWHEEL:
		l = (short) HIWORD(wParam);	/* number of wheel clicks */
		if(shiftdown) {
			if(l < 0) {
				if(yview*yview+zview*zview < 1.0e5) {
					yview *= 1.25;
					zview *= 1.25;
				} else
					break;
			} else {
				if(yview*yview+zview*zview > 1.0) {
					yview *= 0.8;
					zview *= 0.8;
				} else
					break;
			}
			if(!IsBadCodePtr((FARPROC) update_func) && rotatable) {
				scaled = FALSE;
				(*update_func)();
			}
#ifdef PROF
		} else if(zoomable_view()) {
#define STEPSIZE 40
			scaled = TRUE;
			if(l > 0) {
				frame_view( xmaxi /  STEPSIZE,((STEPSIZE-1)*xmaxi)/ STEPSIZE,    ymaxi/ STEPSIZE,  ((STEPSIZE-1)*ymaxi)/STEPSIZE);
			} else {
				frame_view(-xmaxi / (STEPSIZE-1), (STEPSIZE*xmaxi)/(STEPSIZE-1),-ymaxi/(STEPSIZE-1),(STEPSIZE*ymaxi)/(STEPSIZE-1));
			}
#endif
		}

		break;

	case WM_MOUSEMOVE:
		if(hWndProc == hWnd && hHint != NULL) {
			relx = LOWORD(lParam);
			rely = HIWORD(lParam);
			if(relx >= 0 && relx < xmaxi && rely >= 0 || rely < ymaxi) KillHint(HintWnd);
			/* moved into display window */
		}
		if(!arranging) {
			if(outside) {
				outside = 0;
				arrowcursor();
			}
			if(ToolDn >= 0 && (hWndProc == hWnd || hWndProc == hWndMain)) {
				DrawTool(ToolWnd[ToolDn],0);
			}
		}
		else {
			return(DefWindowProc(hWndProc,msg,wParam,lParam));
		}
		break;

	case WM_NCMOUSEMOVE:
		if(!arranging) {
			outside = 1;
			arrowcursor();
		}
		else {
			KillHint(hWndProc);
			return(DefWindowProc(hWndProc,msg,wParam,lParam));
		}
		break;

	case WM_LBUTTONDOWN:
#ifdef PROF
		if(zoomable_view()) {
			if(shiftdown) {
				i = xmaxi / 60;
				frame_view(i,xmaxi + i,0,ymaxi);
			} else if(zoomin) {
				xm = LOWORD(lParam);
				ym = HIWORD(lParam);
				frame_view(xm / 4,(xm + 3*xmaxi)/4,ym / 4,(ym + 3*ymaxi)/4);
			}
			break;
		} else {
			KillHint(hWndProc);
			return(DefWindowProc(hWndProc,msg,wParam,lParam));
		}
#else
		KillHint(hWndProc);
		return(DefWindowProc(hWndProc,msg,wParam,lParam));
#endif

	case WM_NCLBUTTONDOWN:
		KillHint(hWndProc);
		zoomin = FALSE;
		return(DefWindowProc(hWndProc,msg,wParam,lParam));

	case WM_RBUTTONDOWN:
//	case WM_RBUTTONUP:
		if(arranging) {
			arranging = 0;
			SetDlgItemText(hWndArrange,100,"Icon re-arrange ended.\nUse Configure, Write configuration to save changes");
			await(200);
			DestroyWindow(hWndArrange);
		}
		else {
#ifdef PROF
			if(zoomable_view() && shiftdown) {
				i = -xmaxi / 60;
				frame_view(i,xmaxi + i,0,ymaxi);
				break;
			} else {
				KillHint(hWndProc);
				zoomin = FALSE;
				arrowcursor();
				return(DefWindowProc(hWndProc,msg,wParam,lParam));
			}
#else
			KillHint(HintWnd);
			return(DefWindowProc(hWndProc,msg,wParam,lParam));
#endif
		}
		break;

	case WM_SETFOCUS:
		focus = TRUE;
		repaint = TRUE;
		break;

	case WM_KILLFOCUS:
		focus = FALSE;
		break;

	case WM_INITMENUPOPUP:
		context_id = 100*(1 + LOWORD(lParam));
		KillHint(HintWnd);
		break;

	case WM_ENTERIDLE:
		if((wParam == MSGF_MENU) && (GetKeyState(VK_F1) & 0x8000)) {
			HelpRequest = 1;
			PostMessage(hWndMain,WM_KEYDOWN,VK_RETURN,0L);
		}
		else {
			return(DefWindowProc(hWndProc,msg,wParam,lParam));
		}
		break;

	case WM_ACTIVATEAPP:
	case WM_SHOWWINDOW:
		if(!initialised) {
			initialised = 1;
			context_id = 99;
			zoom = 0;
			undoruling = 0;
			SetMenu(hWndProc,hMainMenu);

			/*	Initialise to sections-on and lines-on			*/

			for(i = 0 ; i < maxsec ; i++) secton[i] = 1;
			for(i = 0 ; i < maxlin ; i++) lineon[i] = 1;
			strcpy(linestoplot,"ALL");
			strcpy(sectionstoplot,"ALL");

			/*	Identify an ID for the Open File dialog box help button	*/

			help_id = RegisterWindowMessage(HELPMSGSTRING);

#ifdef DEMO
			PostMessage(hWndProc,WM_COMMAND,999,0L);
#endif

		}
		else if(hWnd == hWndProc) {
			InvalidateRect(hWndProc,NULL,TRUE);
			repaint = TRUE;
			return(DefWindowProc(hWndProc,msg,wParam,lParam));
		}
		break;

	case WM_SIZE:

		KillHint(HintWnd);
		if(hWndMain != NULL && hWndProc == hWndMain && hWnd != NULL) {
			GetSizes(hWnd);
			DestroyToolbar();
			DestroyScrollBars();
			if(hDC != NULL) {
				ReleaseDC(hWnd,hDC);
				hDC = NULL;
			}
			DestroyWindow(hWnd);
			hWnd = NULL;
		}
		repaint = TRUE;
		break;

	case WM_PAINT:
		if(hWnd == NULL) {
			InitToolbar();
			InitWorkwin(scrollable);
			if(scrollable) CreateScrollBars();
		}
		if(scrdev == 0) {
			GetClientRect(hWnd,&rc);
			glViewport((GLint) rc.left,(GLint) rc.top,(GLint) rc.right,(GLint) rc.bottom);
		}
		KillHint(HintWnd);
		if(hWndProc == hWnd || hWndProc == hWndMain) {
			BeginPaint(hWndProc,&ps);
			if(text_screen()) {
				if(hDC == NULL) (*init)();
				i = 0;
				cls(FALSE);
				scrcopy = 0;
				for(l = 0 ; l < 40 && i < ybottom-ychar ; l++) {
					sp = scr_ch[l];
					cp = strchr(sp,'.');
					j = strlen(sp);
					if(cp != NULL) {
						cq = cp;
						do {
							cp--;
						}
						while(*cp != ' ' && cp != sp);
						if(cp != sp) cp++;
						GetTextExtentPoint(hDC,cp,(int)(cq - cp),&size);
						writetext(xchar*xjust - size.cx,i,cp,strlen(cp));
						while(*cp == ' ') cp--;
					} else {
						cp = strchr(sp,0) - 1;
					}
					writetext(0,i,sp,(int) (cp - sp));
					i += ychar;
				}
				GDIen();
			}
			else if(!IsBadCodePtr((FARPROC) update_func)) {
				(*update_func)();
			} else {
				cls(FALSE);
			}
			for(i = 0 ; i < numtool ; i++)
				if(ToolWnd[i] != NULL) DrawTool(ToolWnd[i],0);
			EndPaint(hWndProc,&ps);
			repaint = FALSE;
		}
		else {
			return(DefWindowProc(hWndProc,msg,wParam,lParam));
		}
		break;

	case WM_KEYUP:
		if(wParam == VK_SHIFT) {
			shiftdown = FALSE;
		}
		else {
			return(DefWindowProc(hWndProc,msg,wParam,lParam));
		}
		break;

	case WM_KEYDOWN:
		KillHint(HintWnd);
		switch(wParam) {
		case VK_F1:	/* context-sensitive help */
			context(context_id);
			break;

		case VK_F2:	/* quick save */
			save_file();
			break;

		case VK_SHIFT:
			shiftdown = TRUE;
			break;

		default:
#ifdef PROF
			if(shiftdown) {	// panning
				if(zoomable_view()) {
					switch(wParam) {
					case VK_LEFT:
						i = xmaxi / 60;
						j = 0;
						break;
					case VK_RIGHT:
						i = -xmaxi / 60;
						j = 0;
						break;
					case VK_DOWN:
						i = 0;
						j = -ymaxi / 60;
						break;
					case VK_UP:
						i = 0;
						j = ymaxi / 60;
						break;
					default:
						return(DefWindowProc(hWndProc,msg,wParam,lParam));
					}
					frame_view(i,xmaxi + i,j,ymaxi+j);
					break;
				}
				else {
					return(DefWindowProc(hWndProc,msg,wParam,lParam));
				}
			}
			else
#endif
			if(scrollable) {
				switch(wParam) {
				case VK_LEFT:
					if(horzpos <= -180)
						horzpos = 180;
					else
					    horzpos--;
					goto keyhorz;

				case VK_RIGHT:
					if(horzpos >= 180)
						horzpos = -180;
					else
					    horzpos++;

keyhorz:
					rotn = -horzpos;
					if(!IsBadCodePtr((FARPROC) update_func) && rotatable) {
						scaled = FALSE;
						(*update_func)();
					}
					SetScrollPos(hWnd,SB_HORZ,horzpos,TRUE);
					break;

				case VK_DOWN:
					if(vertpos >= 90)
						vertpos = 90;
					else
					    vertpos++;
					goto keyvert;

				case VK_UP:
					if(vertpos <= -90)
						vertpos = -90;
					else
					    vertpos--;

keyvert:
					dist = sqrt(yview*yview+zview*zview);
					angle = vertpos*0.01745329;
					yview = dist*sin(angle);
					zview = dist*cos(angle);
					if(!IsBadCodePtr((FARPROC) update_func) && rotatable) {
						scaled = FALSE;
						(*update_func)();
					}
					SetScrollPos(hWnd,SB_VERT,vertpos,TRUE);
					break;

				case VK_NEXT:
					if(yview*yview+zview*zview < 1.0e5) {
						yview *= 1.25;
						zview *= 1.25;
						if(!IsBadCodePtr((FARPROC) update_func) && rotatable) {
							scaled = FALSE;
							(*update_func)();
						}
					}
					break;

				case VK_PRIOR:
					if(yview*yview+zview*zview > 1.0) {
						yview *= 0.8;
						zview *= 0.8;
						if(!IsBadCodePtr((FARPROC) update_func) && rotatable) {
							scaled = FALSE;
							(*update_func)();
						}
					}
					break;

				case VK_ADD:
					heel += 1.0;
					goto changeheel;
				case VK_SUBTRACT:
					heel -= 1.0;
changeheel:
					sina = sind(heel);
					cosa = cosd(heel);
					if(wate_int > 0.0) {
						balanced = FALSE;
						persp = TRUE;
						do_balance();
					}
					if(!IsBadCodePtr((FARPROC) update_func) && rotatable) {
						scaled = FALSE;
						(*update_func)();
					}
					break;

				default:
					return(DefWindowProc(hWndProc,msg,wParam,lParam));
				}
			}
			else {
				return(DefWindowProc(hWndProc,msg,wParam,lParam));
			}
		}
		break;

	case WM_COMMAND:

		if(wParam == 2001) {
			if(hWnd == hWndView) {
				wglDeleteContext(hRC);
				hRC = NULL;
				DeleteDC(hDC);
				glDeleteLists(viewlist,258);
				viewlist = 0;
				DestroyMenu(ShadeViewMenu);
				DestroyWindow(hWndView);
				hWnd = save_gdi_hwnd;
				hDC = save_gdi_hdc;
				save_gdi_hwnd = NULL;
				scrdev = 1;
				setup(1);
				repaint = TRUE;
				update_func = null;	// needed, can't see why ...
				break;
			}
			else {
				return(DefWindowProc(hWndProc,msg,wParam,lParam));
			}
		} else if(hWndView != NULL) {	// ignore other commands until the GL window is closed
			return(DefWindowProc(hWndProc,msg,wParam,lParam));
		}
		KillHint(HintWnd);
		repaint = FALSE;
#ifdef DEMO
		if(wParam == 999) {
			init_demo();
			break;
		}
#endif

		/*	Convert Toolbar message to corresponding menu ID	*/

		if(wParam == 0) {
			for(i = 0 ; i < numtool ; i++) {
				if((HWND) lParam == ToolWnd[i]) {
					wParam = tool_id[i];
					break;
				}
			}
		}

		/*	Context-sensitive help for provided menu ID		*/

		context_id = wParam;
		if(HelpRequest) {
			HelpRequest = 0;
			context(context_id);
			break;
		}

		/*	Menu function						*/

		menu_number = wParam / 100;
		menu_entry = wParam - menu_number * 100;
		functab = (void *) main_subr[menu_number - 1];
		if(menu_entry > 0 && functab != NULL) {
			if(menufunc != functab[menu_entry-1]) {
				prev_func = menufunc;
				menufunc = functab[menu_entry - 1];
			}
		}

		if(menu_number == 2) {	/* edit may alter hull, so spoiling balance state */
			balanced = 0;
		}

		/*	Statics, drag, tanks and stringers are not available
		when using the overlay hull.
		*/

#ifdef PROF
		if((disp < 0.0 || current_hull == OVERLAYHULL) &&
			    (menu_number >= 4 && menu_number < 6 ||
			    menu_number >= 7 && menu_number < 9)) {
			message("This item is not functional\nin overlay or surface mode");
			break;
		}
#endif
		/*	File history list implementation	*/

		if(wParam >= HISTORYBASE && wParam < HISTORYBASE+4) {
			context_id = HISTORYBASE;
			i = wParam - HISTORYBASE;
			sprintf(filepath,"FILE NOT ACCESSIBLE: %s\\%s",lastpath[i],lastname[i]);
			if((fp = fopen(filepath+21,"rt")) == NULL) {
				message(filepath);
			}
			else {
				strcpy(hullfile,filepath+21);
				read_hullfile(fp,hullfile,TRUE);
#ifdef PROF
				save_hull(current_hull);
#endif
				if(count > 0) update_picklist(lastname[i],lastpath[i]);
			}
			menufunc = loadview[view_on_load];
		}

#ifdef PROF

#define RUNBASE 1000
		if(wParam >= RUNBASE+3 && wParam < RUNBASE+100) {

			/*	Run a user's program		*/

			i = wParam - (RUNBASE+3);
			cls(FALSE);
			runprog(progname[i],addarg[i],reload[i]);
			prev_func = NULL;
			break;

		}
		else if(menu_number == 5 && count > 0) {	/* drag */
			do_balance();
			if(!balanced) break;
			lambda = 3.0;


			/*	Disable all functions except file, Statics report, run,
			configuration and help until a hull is defined
			*/

#ifdef HULLSTAT
		}
		else if(wParam == 409 || count > 0 || allow_without_file[menu_number]) {
#else
		}
		else if(count > 0 || allow_without_file[menu_number]) {
#endif
#ifdef DEMO
			check_message(wParam);
#endif

		}

		rotatable = menufunc == full_persp || menufunc == port_persp ||
		    menufunc == starb_persp || menufunc == view_devel;
#else
		if(count > 0 || allow_without_file[menu_number]) {
#ifdef DEMO
			check_message(wParam);
#endif

		}
		rotatable = menufunc == full_persp || menufunc == port_persp ||
		    menufunc == starb_persp;
#endif

//	Enact the command

		working = TRUE;
		zoomin = FALSE;
		GL_surfaces_need_redraw = TRUE;
		GLscale = 1.0;
		GLxshift = 0.0;
		GLyshift = 0.0;
		scaled = FALSE;
		hWndView = NULL;
		if(!IsBadCodePtr((FARPROC) menufunc)) {
			(*menufunc)();
		}
		working = FALSE;

#ifdef PROF

		/*	Remove ruling lines if the hull has been edited		*/

		if(was_changed != changed) {
			for(i = 1 ; i < maxlin ; i++) developed[i] = -1;
			numruled = 0;
			was_changed = changed;
		}
#endif
		break;

	case WM_DESTROY:
		if(hWndProc == hWndMain) {
			KillHint(HintWnd);
			PostQuitMessage(0);
			xp_abort = TRUE;
			break;
		}
		else {
			if(hWnd == hWndView) {
				hWndView = NULL;
			}
			return(DefWindowProc(hWndProc,msg,wParam,lParam));
		}

	case WM_CLOSE:
	case WM_QUERYENDSESSION:
		if(save_gdi_hwnd != NULL) {	// closing the GL shaded view window - simulate the "Close" menu item
			PostMessage(hWnd,WM_COMMAND,2001,0L);
			break;
		} else if(hWndMain == hWndProc) {
			KillHint(HintWnd);
			GetWindowRect(hWndMain,&rc_startup);
			save_config();
			if(changed) {
				if(MessageBox(hWndMain,
					"The design has been changed.\nAre you sure sure you want\nto exit without saving it?","WARNING!",
					MB_ICONQUESTION | MB_YESNO) == IDYES) {
					PostQuitMessage(0);
					xp_abort = TRUE;
					break;
				}
			}
			else {
				PostQuitMessage(0);
				xp_abort = TRUE;
				break;
			}
			return 0;
		}	/* else pass message to DefWindowProc */
		return(DefWindowProc(hWndProc,msg,wParam,lParam));

	default:
		if(msg == (unsigned) help_id) {
			KillHint(HintWnd);
			WinHelp(hWndProc,helpfile,HELP_CONTEXT,999);
			HelpUsed = 1;
		}
		else if(scrollable && !working) {
			switch(msg) {
			case WM_VSCROLL:
				KillHint(HintWnd);
				switch(LOWORD(wParam)) {
				case SB_ENDSCROLL:
				case SB_LINEUP:
				case SB_LINEDOWN:
					if(wParam == SB_LINEDOWN && vertpos < 90)
						vertpos++;
					else if(wParam == SB_LINEUP && vertpos > -90)
						vertpos--;
					dist = sqrt(yview*yview+zview*zview);
					angle = vertpos*0.01745329;
					yview = dist*sin(angle);
					zview = dist*cos(angle);
					if(wParam != SB_ENDSCROLL || dragscroll) {
						if(!IsBadCodePtr((FARPROC) update_func) && rotatable) {
							scaled = FALSE;
							(*update_func)();
						}
						dragscroll = FALSE;
					}
					SetScrollPos(hWndProc,SB_VERT,vertpos,TRUE);
					break;
				case SB_THUMBPOSITION:
					vertpos = (short int) HIWORD(wParam);
					dragscroll = TRUE;
					SetScrollPos(hWndProc,SB_VERT,vertpos,TRUE);
					break;
				}
				break;

			case WM_HSCROLL:
				KillHint(HintWnd);
				switch(LOWORD(wParam)) {
				case SB_ENDSCROLL:
				case SB_LINERIGHT:
				case SB_LINELEFT:
					if(wParam == SB_LINERIGHT) {
						if(horzpos >= 180)
							horzpos = -180;
						else
						    horzpos++;
					}
					else if(wParam == SB_LINELEFT) {
						if(horzpos <= -180)
							horzpos = 180;
						else
						    horzpos--;
					}
					rotn = -horzpos;
					if(wParam != SB_ENDSCROLL || dragscroll && rotatable) {
						if(!IsBadCodePtr((FARPROC) update_func)) {
							scaled = FALSE;
							(*update_func)();
						}
						dragscroll = FALSE;
					}
					SetScrollPos(hWnd,SB_HORZ,horzpos,TRUE);
					break;
				case SB_THUMBPOSITION:
					horzpos = (short int) HIWORD(wParam);
					dragscroll = TRUE;
					SetScrollPos(hWnd,SB_HORZ,horzpos,TRUE);
					break;

				default:
					break;
				}
				break;

			default:
				return(DefWindowProc(hWndProc,msg,wParam,lParam));
			}
			break;

		}
		else {
			return(DefWindowProc(hWndProc,msg,wParam,lParam));
		}
		break;

		/************************************************************************/
		/*	Begin DDE code section						*/
		/************************************************************************/

#ifndef STUDENT
#ifndef DEMO
	case WM_DDE_INITIATE:
		hWndDDEClient = (HWND) wParam;
		aAppl = LOWORD(lParam);
		aTopic = HIWORD(lParam);
		i = GlobalGetAtomName(aAppl,appname,12);
		j = GlobalGetAtomName(aTopic,topicname,12);
		strupr(appname);
		if(aAppl != 0 && (i <= 0 || j <= 0)) {
			//			message("HFDDE: Received invalid DDE initiate request");
		}
		else if(aAppl == 0 || (strcmp(appname,"HULLFORM") == 0 || strcmp(appname,"HULLFORW") == 0)
			    && strcmp(topicname,"STATICS") == 0) {
			aAppl = GlobalAddAtom(appname);
			aTopic = GlobalAddAtom("STATICS");
			SendMessage(hWndDDEClient,WM_DDE_ACK,(WPARAM) hWndMain,MAKELPARAM(aAppl,aTopic));
			GlobalDeleteAtom(aAppl);
			GlobalDeleteAtom(aTopic);
		}
		break;

		/*	Execute a command: Currently they are:

		balance hull
		evaluate statics
		open a file
		*/
	case WM_DDE_EXECUTE:
		hWndDDEClient = (HWND) wParam;
		UnpackDDElParam(WM_DDE_EXECUTE,lParam,&li,&lj);
		hData = (HGLOBAL) lj;	/* There is nothing in the low word */
		conv.i = 0;
		conv.DDEReply.fAck = 1;
		cp = GlobalLock(hData);
		strcpy((char *) datastr,(char *) cp);
		GlobalUnlock(hData);
		strlwr((char *) datastr);

		/*	The MS format for DDE queries is typified by:

		[open("sample.xlm")][run("r1c1")]

		Much of this is not needed; Strip ignored formatting.
		*/
		while( (cp = strchr(datastr,'[')) != NULL) strcpy(cp,cp+1);
		while( (cp = strchr(datastr,']')) != NULL) strcpy(cp,cp+1);
		while( (cp = strchr(datastr,'(')) != NULL) *cp = ' ';
		while( (cp = strchr(datastr,')')) != NULL) *cp = ' ';
		while( (cp = strchr(datastr,'"')) != NULL) *cp = ' ';

		i = parse((char *) datastr);	/* "parse" returns argument count */
		if(i == 0) {
			conv.DDEReply.fAck = 0;
		}
		else {
			j = 0;
			while(j < i) {
				if(strcmp(par[j],"balance") == 0) {

					/*	Balance the current hull		*/

					sina = sind(heel);
					cosa = cosd(heel);
					do_balance();
					findrm();
					calctankstat();

				}
				else if(strcmp(par[j],"open") == 0) {

					/*	Open a new hull file			*/

					if( ( fp = fopen(par[++j],"rt") ) == NULL) {
						strcpy(filepath,dirnam);
						strcat(filepath,"\\");
						strcat(filepath,par[1]);
						if( ( fp = fopen(filepath,"rt") ) == NULL) {
							strcat(filepath,".hud");
							if( ( fp = fopen(filepath,"rt") ) == NULL) conv.DDEReply.fAck = 0;
						}
					}
					if(fp != NULL) {
						strcpy(hullfile,par[1]);
						read_hullfile(fp,hullfile,TRUE);
#ifdef PROF
						save_hull(current_hull);
#endif
					}

				}
				else if(strcmp(par[j],"evaluate") == 0) {

					/*	Evaluate hydrostatics for the current hull position	*/

					sina = sind(heel);
					cosa = cosd(heel);
					huldis(&disp);
					findrm();
					calctankstat();
					balanced = 1;

				}
				else if(strcmp(par[j],"balanceall") == 0) {
					bal_all();
					balanced = 1;

				}
				else if(strcmp(par[j],"menu") == 0) {
					if(sscanf(par[1],"%d",&i) > 0) {
						ddemode = 1;
						PostMessage(hWnd,WM_COMMAND,i,0L);
					}
					break;	/* rest of arguments used by getdlg	*/
				}
				j++;
			}
		}
		li = ((UINT) conv.i) & 0xffff;
		lj = (UINT) hData;
		PostMessage(hWndDDEClient,WM_DDE_ACK,(WPARAM) hWndProc,PackDDElParam(WM_DDE_ACK,li,lj));
		break;

		/*	Set heel, pitch, waterline offset, displacement or LCG (as XCOFM)
		*/
	case WM_DDE_POKE:
		hWndDDEClient = (HWND) wParam;

		UnpackDDElParam(WM_DDE_POKE,lParam,&li,&lj);
		hData = (HGLOBAL) li;
		aItem = (ATOM) lj;
		DDEPoke = (DDEPOKE *) GlobalLock(hData);
		conv.i = 0;
		if(sscanf((char *) DDEPoke -> Value,"%f",&reqval) >= 1) {
			conv.DDEReply.fAck = 1;

			/* Get name of variable to which value is to assigned	*/

			if(DDEPoke != NULL && DDEPoke->cfFormat == CF_TEXT &&
				    GlobalGetAtomName(aItem,varname,16) != 0) {

				/*	Set a one of the independent variables			*/

				if(strncmp(varname,"water",5) == 0) {
					wl = -reqval;
				}
				else if(strcmp(varname,"heel") == 0 && reqval < 360.0 && reqval > -360.0) {
					heel = reqval;
					balanced = FALSE;
				}
				else if(strcmp(varname,"pitch") == 0 && reqval <= 90.0 && reqval >= -90.0) {
					pitch = reqval;
					balanced = FALSE;
				}
				else if(strncmp(varname,"disp",4) == 0 && reqval > 0.0) {
					disp = reqval;
					balanced = FALSE;
				}
				else if(strcmp(varname,"lcg") == 0 || strcmp(varname,"xcofm") == 0) {
					xcofm = reqval;
					balanced = FALSE;
				}
				else if(strcmp(varname,"line") == 0) {
					linereq = (int) reqval - 1;
				}
				else if(strcmp(varname,"vcg") == 0 || strcmp(varname,"zcofm") == 0) {
					zcofm = -reqval;
					balanced = FALSE;
				}
				else if(strcmp(varname,"invert") == 0 && (reqval == 1.0 || reqval == -1.0)) {
					invert = reqval;
				}
				else if(strncmp(varname,"tanksg[",7) == 0 && sscanf(varname+7,"%d",&i) > 0 &&
					    i > 0 && i <= ntank && reqval >= 0.0) {
					fl_spgra[i-1] = reqval;
					balanced = FALSE;
				}
				else if(strncmp(varname,"tankfr[",7) == 0 && sscanf(varname+7,"%d",&i) > 0 &&
					    i > 0 && i <= ntank && reqval >= 0.0 && reqval <= 100.0) {
					fl_fract[i-1] = reqval;
					balanced = FALSE;
				}
				else if(strncmp(varname,"tankpm[",7) == 0 && sscanf(varname+7,"%d",&i) > 0 &&
					    i > 0 && i <= ntank && reqval >= 0.0 && reqval <= 1.0) {
					fl_perm[i-1] = reqval;
					balanced = FALSE;
				}
				else {
					conv.DDEReply.fAck = 0;
				}
			}
			GlobalUnlock(hData);
			if(DDEPoke -> fRelease) GlobalFree(hData);	/* Only free if fAck is true */
		}
		else {
			conv.DDEReply.fAck = 0;	/* not accepted */
		}
		li = ((UINT) conv.i) & 0xffff;
		PostMessage(hWndDDEClient,WM_DDE_ACK,(WPARAM) hWndProc,
			ReuseDDElParam(lParam,WM_DDE_POKE,WM_DDE_ACK,li,(UINT) aItem));
		GlobalDeleteAtom(aItem);
		break;

		/*	Request for a specified parameter value		*/

	case WM_DDE_REQUEST:
		hWndDDEClient = (HWND) wParam;
		conv.i = 0;
		conv.DDEReply.fAck = 1;
		UnpackDDElParam(WM_DDE_REQUEST,lParam,&li,&lj);
		i = li;
		aItem = (ATOM) lj;
		if(i == CF_TEXT && GlobalGetAtomName(aItem,varname,16) != 0) {

			if(!sendrecv("get",varname,datastr,&reqval)) {
				message("Invalid DDE request");
				break;
			}
			hData = GlobalAlloc(GMEM_DDESHARE,sizeof(DDEDATA) + MAX_PATH);
			DDEData = (DDEDATA *) GlobalLock(hData);
			if(datastr[0] != 0)
				strcpy((char *) DDEData->Value,datastr);
			else
			    sprintf((char *) DDEData->Value,"%.4f",reqval);
			DDEData->fResponse = TRUE;
			DDEData->fRelease = FALSE;
			DDEData->fAckReq = TRUE;  		/* TRUE indicates the data was sent in response to a WM_DDE_REQUEST message */
			DDEData->cfFormat = CF_TEXT;	/* only option supported by DDE */
			GlobalUnlock(hData);
			lPar = ReuseDDElParam(lParam,WM_DDE_ACK,WM_DDE_DATA,(UINT) hData,(UINT) aItem);
			PostMessage(hWndDDEClient,WM_DDE_DATA,(WPARAM) hWndProc,lPar);
		}
		else {
			conv.DDEReply.fAck = 0;	/* not accepted */
			li = ((UINT) conv.i) & 0xffff;
			PostMessage(hWndDDEClient,WM_DDE_ACK,(WPARAM) hWndProc,PackDDElParam(WM_DDE_ACK,li,(UINT) aItem));
		}
		break;

		/*	End DDE error: pass through to cleanup section			*/

	case WM_DDE_TERMINATE:
		hWndDDEClient = (HWND) wParam;
		PostMessage(hWndDDEClient,WM_DDE_TERMINATE,(WPARAM) hWndProc,(LPARAM) 0L);
		ddemode = 0;
		break;

		/*	On receiving an acknowledgement, delete any associated atoms	*/

	case WM_DDE_ACK:
		UnpackDDElParam(WM_DDE_ACK,lParam,&li,&lj);
		i = li;
		aItem = (ATOM) lj;
		FreeDDElParam(WM_DDE_ACK,lParam);
		GlobalDeleteAtom(aItem);
		GlobalDeleteAtom((ATOM) i);
		break;

		/*	We shouldn't receive this message, so acknowledge in the negative	*/

	case WM_DDE_DATA:
		UnpackDDElParam(WM_DDE_DATA,lParam,&li,&lj);
		hData = (HGLOBAL) li;
		aItem = (ATOM) lj;
		if(hData != NULL) {
			DDEData = (DDEDATA *) GlobalLock(hData);
			if(DDEData->fAckReq) {		/* re-use the aItem in reply */
				if(DDEData->fRelease) {
					GlobalUnlock(hData);
					GlobalFree(hData);
				}
				else {
					GlobalUnlock(hData);
				}
				conv.DDEReply.fAck = FALSE;
				li = ((UINT) conv.i) & 0xffff;
				PostMessage(hWndDDEClient,WM_DDE_ACK,(WPARAM) hWndProc,
					ReuseDDElParam(lParam,WM_DDE_DATA,WM_DDE_ACK,li,(UINT) aItem));
			}
			else {
				GlobalUnlock(hData);
				GlobalFree(hData);
				DeleteAtom(aItem);
			}
		}
		else {
			DeleteAtom(aItem);
		}
		break;

	case WM_DDE_ADVISE:
	case WM_DDE_UNADVISE:
		aItem = HIWORD(lParam);
		conv.DDEReply.fAck = FALSE;
		li = ((UINT) conv.i) & 0xffff;
		PostMessage(hWndDDEClient,WM_DDE_ACK,(WPARAM) hWndProc,
			ReuseDDElParam(lParam,msg,WM_DDE_ACK,li,(UINT) aItem));
		break;

#endif
#endif

		/************************************************************************/
		/*	End DDE code section						*/
		/************************************************************************/

	}
	return(0l);
}

void KillHint(HWND HintWnd)
{
	if(hHint != NULL) {
		KillTimer(HintWnd,nTimer);
		DestroyWindow(hHint);
		hHint = NULL;
	}
}

/******************************************************************/

void do_balance(void)
{
	extern int balanced;
	REAL calcdisp;

	if(count > 0 && !balanced) {
		if(!persp) {
			(*init)();
			clrtext();
			pstr("Balancing hull ...\n");
		}
		balanc(&calcdisp,1);
		if(calcdisp < 0.0) return;
		finish_stats();
		balanced = 1;
		(*endgrf)();
	}
}

/**********************************************************************/

void message(char *mes)
{
	(void) MessageBox(hWndMain,mes,"HULLFORM MESSAGE",MB_OK);
}

/**********************************************************************/

/*	screen output routines					*/

/*	Clear the workspace screen				*/

void cls(int scrollbar)
{
	extern int scrollable;
	void GLcl(void),GLin(void),GLen(void);

	if(!hardcopy) {
		if(hDC == NULL) (*init)();
		if(!scrollable && scrollbar) {
			CreateScrollBars();
		} else if(scrollable && !scrollbar) {
			DestroyScrollBars();
		}
		scrollable = scrollbar;
		(*clrgrf)();
	}
}

void clrscr()
{
	cls(FALSE);
}

void clrtext()
{
	int i;
	extern int scrcopy;
	RECT rc;
	void GLin(void);

	xcursor = xleft;
	ycursor = ytop;
	textrow = 0;
	textcol = 0;
	memset(scr_ch,4000,0);
	scrcopy = 1;
	if(scrdev == 1) {
		if(hDC == NULL) GDIin();
	} else {
		GLin();
		GetWindowRect(hWnd,&rc);
		glOrtho((GLdouble) rc.left,(GLdouble) rc.right,(GLdouble) rc.bottom,(GLdouble) rc.top,(GLdouble) -10000.0,(GLdouble) 10000.0);
	}
	cls(FALSE);
}


/*	text string output					*/

/*	print a string at a specified position		*/

/*	xcursor and ycursor are now in pixel units	*/

void pstrxy(int x,int y,char *b)
{
	extern int xcursor,ycursor;
	char *e;
	int l;
	int pixlen;
	extern COLORREF scrcolour[];
	SIZE size;

	xcursor = x;
	ycursor = y;
	textrow = ycursor / ychar;

	while( (e = strchr(b,'\n')) != NULL) {
		l = (int) (e - b);
		writetext(xcursor,ycursor,b,l);
		b = e+1;
		textcol = 0;
		textrow++;
		ycursor += ychar;
		xcursor = 0;
	}
	l = strlen(b);
	writetext(xcursor,ycursor,b,l);
	GetTextExtentPoint(hDC,b,l,&size);
	pixlen = size.cx;
	xcursor += pixlen;
}

void writetext(int x,int y,char *b,int l)
{
	extern int xmaxi,xleft,ymaxi,xchar,ychar;
	extern HDC hDC;
	SIZE size;
	RECT rc;;
	extern REAL	xcurr,ycurr,zcurr;
	extern int persp;
	extern int penup;
	extern COLORREF scrcolour[];
	GLfloat prevcol[4];
	DWORD col;
	float cr,cg,cb;

	if(l <= 0) return;

	if(scrdev == 1) {
		l++;
		do {
			GetTextExtentPoint(hDC,b,--l,&size);
		}
		while(size.cx > xright - xleft) ;

		if(y > ybottom - ychar) {
			ScrollWindow(hWnd,0,-ychar,NULL,NULL);
			y = ybottom - ychar;
		}
		if(l && y >= 0) {
			SetBkMode(hDC,TRANSPARENT);
			SetTextColor(hDC,scrcolour[1]);
			TextOut(hDC,x + xchar,y,b,l);
		}
	}
	else {
		glGetFloatv(GL_CURRENT_COLOR,prevcol);
		col = scrcolour[1];						// text colour
		cr = ((float) GetRValue(col)) / 255.0;
		cg = ((float) GetGValue(col)) / 255.0;
		cb = ((float) GetBValue(col)) / 255.0;
		glColor3f(cr,cg,cb);
		GetWindowRect(hWnd,&rc);
		glRasterPos3f(rc.left + x + xchar,rc.top + y,0.0);
		glPushMatrix();
		glCallLists(strlen(b), GL_UNSIGNED_BYTE,b);
		glPopMatrix();
		glColor3fv(prevcol);
	}
	if(scrcopy) {
		l = min(l,79-textcol);
		if(textrow < 40) {
			strncpy(&scr_ch[textrow][textcol],b,l);
			textcol += l;
			scr_ch[textrow][textcol] = 0;
		}
	}
}

void pstrx(int x,char *s)
{
	extern int ycursor,xchar;
	pstrxy(x,ycursor,s);
}

void pstr(char *s)
{
	extern int xcursor,ycursor;
	pstrxy(xcursor,ycursor,s);
}

/*	integer value output				*/

/*	print an integer a specified position		*/

void pintxy(int x,int y,char *text,INT value)
{
	char buffer[MAX_PATH];
	sprintf(buffer,text,value);
	pstrxy(x,y,buffer);
}

/*	print an integer at a specified x-cursor position, and the	*/
/*	current y-cursor position					*/

void pintx(int x,char *text,INT value)
{
	extern int ycursor;
	pintxy(x,ycursor,text,value);
}

void pint(char *text,INT value)
{
	extern int xcursor,ycursor;
	pintxy(xcursor,ycursor,text,value);
}

/*	real value output				*/

void preaxy(int x,int y,char *text,REAL value)
{
	char buffer[MAX_PATH];
	sprintf(buffer,text,value);
	pstrxy(x,y,buffer);
}

void preax(int x,char *text,REAL value)
{
	extern int ycursor;
	preaxy(x,ycursor,text,value);
}

void prea(char *text,REAL value)
{
	extern int xcursor,ycursor;
	preaxy(xcursor,ycursor,text,value);
}

/*	Print a real value with decimal point at column 38	*/

void pjust(REAL value)
{
	char str[60];
	char *p,*d;
	extern HDC hDC;
	extern int xchar;
	int l;
	SIZE size;
	extern REAL elimit;
	extern int num_dp;
	char fmt[8];

	if(fabs(value) > 10000000.0 || (fabs(value) < elimit && value != 0.0))
	{
		sprintf(fmt,"%%.%dE ",num_dp);
		sprintf(str,fmt,value);
		p = strchr(str,'E')+2;
		if(*p == '0') strcpy(p,p+1);
		if(*p == '0') strcpy(p,p+1);
	}
	else if(fabs(value) > 1000000.0) {
		sprintf(str,"%d. ",(int) value);
	}
	else {
		sprintf(fmt,"%%.%df ",num_dp);
		sprintf(str,fmt,value);
	}
	p = str;
	while(*p == ' ') p++;
	d = strchr(p,'.');
	if(d == NULL) d = strchr(p,0);
	l = d - p;

	if(scrcopy) while(textcol < 60 - l) {
		scr_ch[textrow][textcol] = ' ';
		textcol++;
	}
	GetTextExtentPoint(hDC,p,l,&size);
	pstrx(xchar*xjust - size.cx,p);
}

/*	Open text file for output		*/

#pragma argsused
int open_text(FILE **fp,char dir[],char *ext)
{
#ifdef DEMO
	not_avail();
	return 0;
#else
	char	dev[MAX_PATH];

	if(strchr(text_device,'\\') == NULL) {
		strcpy(dev,dir);
		strcat(dev,"\\");
		strcat(dev,text_device);
	}
	else {
		strcpy(dev,text_device);
	}

	return(openfile(dev,"wt","General text file output","\0\0\0",ext,dir,fp));
#endif
}

void copytoclipboard()
{
FILE *fp;
#ifdef DEMO
	not_avail();
#else
	HGLOBAL	hMem;
	HMETAFILE	hMF;
	int 	i,l;
	DWORD	size;
	char	*loc;
	int		textcopy = text_screen();
	extern int	hcpydev;
	HDC		PrevDC;
	HPEN	hPen;
	HBRUSH	hBrush;
	HGLOBAL	hMFB;
	METAFILEPICT *mfp;
	extern LOGFONT prfont;
	HFONT	hFont,OldFont;
	extern int	xchar,ychar;
	int		prevsize;
	UINT	wmfsize;
	char *wmfbuffer;
	int		save_scrdev = scrdev;

	if(!textcopy && !graphic_screen()) return;

	/*	Open and clear the clipboard, copy data to the allocated memory
	block, set the data in the clipboard, and close it	*/

	OpenClipboard(hWndMain);
	EmptyClipboard();

	if(textcopy) {

		/*	Find length of text, in lines	*/

		for(l = 31 ; l >= 0 ; l--) if(scr_ch[l][0] != 0) break;
		if(l <= 0) return;
		size = 1;
		for(i = 0 ; i <= l ; i++) size += 2 + strlen(scr_ch[i]);

		/*	Allocate global memory for text		*/

		hMem = GlobalAlloc(GPTR,size);
		if(hMem == NULL) {
			message("No free space for copy to clipboard");
		}
		else {

			/*	Obtain pointer to memory block		*/

			loc = (char far *) GlobalLock(hMem);
			*loc = 0;
			for(i = 0 ; i <= l ; i++) {
				strcat(loc,scr_ch[i]);
				strcat(loc,"\r\n");
			}
		}
		SetClipboardData(CF_TEXT,hMem);

	}
	else {

		/*	Graphics to clipboard			*/

		hardcopy = 1;		/* operate in hardcopy (not screen) mode */
		hcpydev = 7;		/* metafile			*/
#ifdef  STUDENT
		setup(2);		/* set general device parameters */
#else
		setup(7);		/* set general device parameters */
#endif
		PrevDC = hDC;		/* Save the screen device context */
		hDC = CreateMetaFile(NULL);	/* Create a memory metafile	*/
		if(hDC == NULL) {
			message(no_memory);
		}
		else {
			waitcursor();
			scrdev = 1;		// use GDI model for WMF graphics

//	WMF "init" equivalent

			i = SetWindowOrgEx(hDC,0,0,NULL);
			i = SetWindowExtEx(hDC,16000,12000,NULL);	/* Force sizes (args must be the same as in graf_hfw) */
			i = SetMapMode(hDC,MM_ANISOTROPIC);

			prevsize = prfont.lfHeight;
			prfont.lfHeight = MulDiv(pointsize,2540,72);
			hFont = CreateFontIndirect(&prfont);
			OldFont =  SelectObject(hDC,hFont);
			if(OldFont != NULL) DeleteObject(OldFont);
			xchar = MulDiv(xchar,prfont.lfHeight,ychar);
			ychar = prfont.lfHeight;

			xwleft = 0;
			ywtop = 0;

//	(Re-)Generate the view

			if(!IsBadCodePtr((FARPROC) update_func)) {
				scaled = FALSE;
				(*update_func)();
			}

//	WMF "endgrf" equivalent

			hBrush = SelectObject(hDC,GetStockObject(BLACK_BRUSH));
			if(hBrush != NULL) DeleteObject(hBrush);
			hPen = SelectObject(hDC,GetStockObject(BLACK_PEN));
			if(hPen != NULL) DeleteObject(hPen);
			OldFont =  SelectObject(hDC,GetStockObject(SYSTEM_FONT));
			if(OldFont != NULL) DeleteObject(OldFont);

			hMF = CloseMetaFile(hDC);

			hardcopy = 0;		/* screen mode again */
			hDC = PrevDC;
			setup(scrdev);
			(*init)();
			prfont.lfHeight = prevsize;

			if(hMF == NULL) {
				message("Could not close memory metafile");
			}
			else {
				hMFB = NULL;
				if((wmfsize = GetMetaFileBitsEx(hMF,0,NULL)) > 0) {
					/* Get size required */
					if(memavail((void *) &wmfbuffer,wmfsize)) {
						/* Get memory required */
						if(GetMetaFileBitsEx(hMF,wmfsize,wmfbuffer) > 0) {
							hMFB = SetMetaFileBitsEx(wmfsize,(BYTE *) wmfbuffer);
							DeleteMetaFile(hMF);
							memfree(wmfbuffer);
						}
					}
				}
				if(hMFB == NULL) {
					message("Could not get metafile memory handle");
					DeleteMetaFile(hMF);
				}
				else {
					hMem = GlobalAlloc(GPTR,sizeof(METAFILEPICT));
					mfp = (METAFILEPICT *) GlobalLock(hMem);
					mfp->mm = MM_ANISOTROPIC;
					mfp->xExt = 16000;
					mfp->yExt = 12000;
					mfp->hMF = hMFB;
					GlobalUnlock(hMem);
					if(SetClipboardData(CF_METAFILEPICT,hMem) == NULL)
						message("Metafile copy to clipboard failed");
				}
			}
			scrdev = save_scrdev;
		}
	}
	CloseClipboard();
	arrowcursor();
#endif
}

void GetSizes(HWND hWnd)
{
	RECT rcb;
	RECT rca;
	char text[200];
	void GLin();

	if(hWnd == NULL) return;

	if(scrdev == 0 && pixelformat != 0) {
		glEnd();
		if(hRC != NULL) {
			pixelformat = 0;
			wglDeleteContext(hRC);
			hRC = NULL;
		}
		glDeleteLists(viewlist,258);
		viewlist = 0;
	}
	GetWindowRect(hWnd,&rca);
	GetClientRect(hWnd,&rcb);

	xleft = rcb.left;
	xright = rcb.right;
	ytop = rcb.top;
	ybottom = rcb.bottom;
	if(xleft >= xright) {
		xright = 600;
		xleft = 0;
	}
	if(ytop >= ybottom) {
		ybottom = 400;
		ytop = 0;
	}

	xmaxi = xright - xleft;
	ymaxi = ybottom - ytop;

	box((REAL) xmaxi,(REAL) ymaxi);
}


REAL fsqr0(REAL x)
{
	return(x <= 0.0 ? 0.0 : sqrt(x));
}


void makemenu()
{
	int j;
	HMENU hSubMenu;
	char menuline[130];
	char *a;
	static int menuinit = TRUE;
#define HELPMENU 1200

	/*	Add items to run menu		*/

	if(menuinit) {
		menuinit = FALSE;
#ifndef STUDENT
		for(j = 0 ; j < numprog ; j++)
			InsertMenu(hMainMenu,1001,MF_BYCOMMAND,1003+j,progtext[j]);
#endif

		/*	Remove the updates and tutorial entries if the files
		have been deleted.
		*/
		if(searchpath("whatsnew.hlp") == NULL) {
			RemoveMenu(hMainMenu,HELPMENU+5,MF_BYCOMMAND);
		}
		if(searchpath("tutorial.hlp") == NULL) {
			RemoveMenu(hMainMenu,HELPMENU+6,MF_BYCOMMAND);
		}
	}

	/*	Remove any previous "pick list", add the current one	*/

	hSubMenu = GetSubMenu(hMainMenu,0);
	for(j = HISTORYBASE ; j < HISTORYBASE+4 ; j++)
		(void) RemoveMenu(hSubMenu,j,MF_BYCOMMAND);
	for(j = 0 ; j < 4 ; j++) {
		if(*lastname[j] != 0) {
			sprintf(menuline,"&%d %s",j+1,lastname[j]);
			a = menuline+2;
			while((a = strchr(a,'&')) != NULL) {
				movmem(a,a+1,strlen(a)+1);
				a += 2;
			}
			AppendMenu(hSubMenu,MF_STRING,HISTORYBASE+j,menuline);
		}
	}
}

void arrowcursor()
{
	HCURSOR result;
	if(!zoomin) {
		result = LoadCursor(NULL,IDC_ARROW);
		(void) SetCursor(result);
	}
}

void waitcursor()
{
	HCURSOR result;
	result = LoadCursor(NULL,IDC_WAIT);
	(void) SetCursor(result);
}

void striptopath(char *path)
{
	char *p;
	p = strchr(path,0);
	while(p != path) {
		if(*--p == '\\') {
			*p = 0;
			break;
		}
	}
}

#ifdef PROF
void set_zoomin()
{
	HCURSOR result;
	if(zoomable_view()) {
		zoomin = TRUE;
		result = LoadCursor(NULL,IDC_CROSS);
		(void) SetCursor(result);
	}
}

void set_zoomout()
{
	if(zoomable_view()) frame_view(-xmaxi/4,(5*xmaxi)/4,-ymaxi/4,(5*ymaxi)/4);
}

void frame_view(int xzleft,int xzright,int yztop,int yzbottom)
{
	extern REAL GLscale;
	REAL w1 = xmax - xmin;
	xmin = ((float) xzleft  - xgorigin)/xgslope;
	xmax = ((float) xzright - xgorigin)/xgslope;
	ymin = ((float) (ymaxi - yzbottom) - ygorigin)/ygslope;
	ymax = ((float) (ymaxi - yztop)    - ygorigin)/ygslope;
	scaled = TRUE;
	GLscale = (xmax - xmin)/w1;
	setranges(xmin,xmax,ymin,ymax);
	(*update_func)();
}

#endif

#ifdef DEMO
void not_avail()
{
	message("This item is not available in\nthe demonstration version");
}

static int no_message = FALSE;
int democode = 0;

void init_demo_func(int code,HWND hWndDlg)
{
	democode = code;
	PostMessage(hWndDlg,WM_COMMAND,IDOK,0l);
}

void init_demo(void)
{
	if(!getdlg(DEMO1,INP_PBF,init_demo_func,-1)) return;

	no_message = TRUE;

	switch(democode) {
	case 0:		/* new design */
		if(!getdlg(DEMO_0_1)) return;
		do {
			SendMessage(hWndMain,WM_COMMAND,101,0L);
			yview = -1000.0;
			zview = 5000.0;
			rotn = -20.0;
			dlgboxpos = 1;
			SendMessage(hWndMain,WM_COMMAND,303,0L);
			dlgboxpos = 1;
		}
		while(getdlg(DEMO_0_2,-1));
		(void) getdlg(DEMO_0_3,-1);
		SendMessage(hWndMain,WM_COMMAND,204,0L);
		break;

	case 1:		/* statics */
		if(!getdlg(DEMO_1_1,-1)) return;	/* load a design */
		SendMessage(hWndMain,WM_COMMAND,102,0L);

		if(!getdlg(DEMO_1_2,-1)) return;	/* now balance the hull */
		heel = 0.0;
		sina = 0.0;
		cosa = 1.0;
		SendMessage(hWndMain,WM_COMMAND,402,0L);

		if(!getdlg(DEMO_1_3,-1)) return;	/* heeled balance */
		heel = 20.0;
		sina = 0.5;
		cosa = 0.866;
		SendMessage(hWndMain,WM_COMMAND,402,0L);
		break;

	case 2:		/* plate development */
#ifdef PROF
		if(!getdlg(DEMO_2_1,-1)) return;	/* select design */
		SendMessage(hWndMain,WM_COMMAND,102,0L);

		if(!getdlg(DEMO_2_2,-1)) return;	/* select lines */
		SendMessage(hWndMain,WM_COMMAND,601,0L);

		if(!getdlg(DEMO_2_3,-1)) return;	/* choose extra drawing points */
		SendMessage(hWndMain,WM_COMMAND,1112,0L);

		if(!getdlg(DEMO_2_4,-1)) return;	/* transom / stem lines */
		SendMessage(hWndMain,WM_COMMAND,603,0L);

		if(!getdlg(DEMO_2_5,-1)) return;	/* evaluate rulings */
		SendMessage(hWndMain,WM_COMMAND,604,0L);
		dlgboxpos = 1;

		if(!getdlg(DEMO_2_6,-1)) return;	/* plot rulings */
		SendMessage(hWndMain,WM_COMMAND,606,0L);

		dlgboxpos = 1;
		if(!getdlg(DEMO_2_7,-1)) return;	/* ruling line rules 1 */
		dlgboxpos = 1;
		if(!getdlg(DEMO_2_8,-1)) return;	/* ruling line rules 2 */

		dlgboxpos = 1;
		if(!getdlg(DEMO_2_9,-1)) return;	/* Rollout, Current line pair */
		SendMessage(hWndMain,WM_COMMAND,608,0L);

		dlgboxpos = 1;
		if(!getdlg(DEMO_2_10,-1)) return;	/* File, plot */
		strcpy(port,"Example.dxf");
		default_hardcopy_device = 5;
		SendMessage(hWndMain,WM_COMMAND,113,0L);
#else
		message("Only available in versions 9P");
#endif
		break;

	case 3:		/* DXF Output */
#ifdef PROF
		if(!getdlg(DEMO_3_1,-1)) return;	/* Intro */
		if(!getdlg(DEMO_3_2,-1)) return;	/* New hull */
		SendMessage(hWndMain,WM_COMMAND,101,0L);
		if(!getdlg(DEMO_3_3,-1)) return;	/* DXF Intro */
		SendMessage(hWndMain,WM_COMMAND,110,0L);
		if(!getdlg(DEMO_3_4,-1)) return;	/* DXF Intro, show body plan */
		SendMessage(hWndMain,WM_COMMAND,304,0L);
		dlgboxpos = 1;
		if(!getdlg(DEMO_3_5,-1)) return;	/* DXF Plot */
		strcpy(port,"Example.dxf");
		default_hardcopy_device = 5;
		SendMessage(hWndMain,WM_COMMAND,113,0L);
#else
		message("Only available in version 9P");
#endif
		break;
	}
}

void check_message(WPARAM wParam)
{
	char *msg;

	if(no_message) return;

	switch(wParam) {
	case 101:
		msg = "This is the way to initialise a new hull, or a surface to be used in the program's \"Surfaces\" section.";
		break;
	case 105:
		msg = "This is an input function. Normally, the program presumes a flat water surface, but you can input any shape using a file of (x,z) coordinates";
		break;
	case 106:
		msg = "This allows you to read in a reference design, for comparison with the one you are working on.";
		break;
	case 108:
		msg = "You can write out waterlines at any interval, using this facility.";
		break;
	case 109:
		msg = "Using this feature, you can write the outline of each hull section, for use in constructing the final hull.";
		break;
	case 110:
		msg = "The DXF file format is the standard way of moving graphics to engineering CAD systems. Hullform generates output in both 2-D and 3-D forms.";
		break;
	case 111:
		msg = "The VRML file format is an ideal way to generate images for viewing by others, either on an Internet web site or in another office.";
		break;
	case 201:
		msg = "You can copy both text and graphics to other programs, using the Windows clipboard";
		break;
	case 204:
		msg = "This is one of the two main editing functions, the other being the 'Smooth a line' entry, further down the Edit menu. A careful read of the help file will be needed, to ensure you understand the use of master and slave sections in editing.";
		break;
	case 207:
		msg = "This is one of the two main editing functions, the other being the 'Edit sections' entry, further up the Edit menu. A careful read of the help file will be needed, to ensure you understand the use of master and slave sections in editing.";
		break;
	case 208:
		msg = "The stem line is the line which joins the stem, and forms the effective keel of the design.";
		break;
	case 209:
		msg = "Use this to ensure the stem and stem line join in a fair curve.";
		break;
	case 210:
		msg = "The stem may be rounded, with radii definable at the forward end of each hull line.";
		break;
	case 211:
		msg = "You can read and modify all properties of a line - like spline flexibility, control point type etc - using this menu item.";
		break;
	case 215:
		msg = "This allows you to redefine the lines of the hull, to follow a set of waterlines, diagonals or buttock lines.";
		break;
	case 308:
		msg = "You can also rotate the viewing position using the scroll bars on the perspective views.";
		break;
	case 309:
		msg = "This covers all options controlling what aspects of the hull are displayed in orthogonal and perspective views.";
		break;
	case 310:
	case 311:
		msg = "You can also obtain a moveable shaded view by writing a VRML file, and using a suitable viewer - see the File menu for this item.";
		break;
	case 313:
		msg =
		    "You can zoom any view except the 'General orthogonal' view. You can also zoom into the developed surface view, and the 'Edit sections' window.";
		break;
	case 402:
		msg = "The effect of this item is to allow the hull to settle to its position of balance, in terms of waterline and pitch.";
		break;
	case 404:
		msg = "You use this item to find what the displacement of the hull is, for a particular waterline setting.";
		break;
	case 901:
		msg = "The surfaces component of the program is a very useful one, but some careful reading of the manual (or the help file) is recommended.";
		break;
	default:
		return;
	}
	(void) getdlg(INFO,
		0,msg,
		INP_LOG,&no_message,-1);
}

#endif


int windows_nt(void);

#pragma argsused
int ColourMessage(unsigned msg,WORD wParam,LONG lParam,BOOL *colret)
{
	if(windows_nt()) return FALSE;
	switch(msg) {
	case WM_CTLCOLOREDIT:
	case WM_CTLCOLORLISTBOX:
		SetBkColor((HDC) wParam,rgbWhite);
		*colret = TRUE;
		return TRUE;
	case WM_CTLCOLORDLG:
	case WM_CTLCOLORMSGBOX:
	case WM_CTLCOLORBTN:
	case WM_CTLCOLORSCROLLBAR:
	case WM_CTLCOLORSTATIC:
		SetBkColor((HDC) wParam,rgbBkgd);
		*colret = TRUE;
		return TRUE;
	default:
		return FALSE;
	}
}


int windows_nt()
{
	DWORD versinf = GetVersion();
	return (HIWORD(versinf) & 0x8000) == 0;
}

