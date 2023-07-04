/* Hullform component - hulldesi.h
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
 
#define	IDHELP	9

#define STRICT

#ifndef linux

#include <windows.h>
#include <dde.h>
#include <commdlg.h>
#include <process.h>

#include <sys/stat.h>
#include <io.h>
#include <fcntl.h>
#include <direct.h>

#define huge
#define far

#else

#include "hf-linux.h"
#include <Xm/Xm.h>
extern Widget wEdit[],wPushButton[];

Widget LabelWidget(Widget Wdialog,char *text,int centre,int x,int y, int w, int h);
Widget TextWidget(Widget Wdialog,int x,int y, int w, int h);
Widget ListWidget(Widget Wdialog,int x,int y, int w, int h,int multiple);
Widget ButtonWidget(Widget Wdialog,char *text,int x,int y, int w, int h);
Widget CheckBoxWidget(Widget Wdialog,char *text,int x,int y, int w, int h);

#include <sys/stat.h>
#include <sys/io.h>
#include <sys/fcntl.h>
#include <sys/dir.h>

#endif

#include <stdio.h>
#include <math.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <fcntl.h>

/*	Linux fixes	*/

#ifndef _FFBLK_DEF
#define _FFBLK_DEF
struct  ffblk   {
	long            ff_reserved;
	long            ff_fsize;
	unsigned long   ff_attrib;
	unsigned short  ff_ftime;
	unsigned short  ff_fdate;
	char            ff_name[256];
};
#endif  /* __FFBLK_DEF */

#ifndef FA_NORMAL
#define FA_NORMAL   0x00        /* Normal file, no attributes */
#endif

#ifndef FA_DIREC
#define FA_DIREC    0x10        /* Directory */
#endif

#ifndef O_TEXT
#define O_TEXT 0
#endif

#ifndef O_BINARY
#define O_BINARY 0
#endif

#include <time.h>
#include <float.h>

#ifndef MAX_PATH
#define MAX_PATH 512
#endif

#ifndef O_TEXT
#define O_TEXT 0
#endif

#ifndef O_BINARY
#define O_BINARY 0
#endif

#include "getdlg.h"

#ifndef searchpath
char *searchpath(char *);
#endif

/*	Project levels may be STUDENT, EXT, PROF or HULLSTAT. Here we create downward linkages	*/

/*	Extended edition includes STUDENT, and implies EXT_OR_PROF too	*/

#ifdef EXT

#ifndef STUDENT
#define STUDENT
#endif

#ifndef EXT_OR_PROF
#define EXT_OR_PROF
#endif

#endif

/*	Ensure "not STUDENT" implies PROF (should not be needed)	*/

#ifndef STUDENT
#ifndef PROF
    #define PROF
#endif
#endif

/*	HULLSTAT also implies PROF	*/

#ifdef HULLSTAT
#ifndef PROF
#define PROF
#endif
#endif

/*	PROF implies EXT_OR_PROF	*/

#ifdef PROF
#ifndef GHS
#define GHS
#endif
#ifndef EXT_OR_PROF
#define EXT_OR_PROF
#endif
#ifndef PLATEDEV
#define PLATEDEV
#endif
#endif

#define NUMTOOL 89

/*	Hullstat allows extra arrays	*/

#ifdef HULLSTAT
#define	MAXCOND	20
#define	MAXLOAD	20
#define MAXIMME 20
#define MAXTANK 20
#endif

#define MAINHULL 0
#define OVERLAYHULL 1

#define	INP_RB	INP_RB1

#ifndef min
#define min(a, b)  (((a) < (b)) ? (a) : (b))
#endif

extern	HINSTANCE hInst;	/* handle to instance of process */
extern	HDC	PlotDC;		/* plot device context */
extern	HWND	hWnd;
extern	HWND	hWndMain;
extern	int	scrdev;

#define	MENUFUNC void
#define	REAL	float
#define	INT	int

/*	variable types			*/

typedef float (*REAL_PTR) (void);
typedef	void	(*FUNC_PTR) (void);
typedef	void	(*REALFUNC_PTR) (float x,float y);
typedef void	REALFUNC(float x,float y);
typedef	void	(*INTFUNC_PTR) (int);
typedef void	INTFUNC(int);
typedef	void	(*STRFUNC_PTR) (char *s);
typedef void	STRFUNC(char *s);
typedef float (*REAL_FUNC) (REAL,INT);

void not_avail(void);
void arrowcursor(void);
void waitcursor(void);
void listseq(int *on,char *p,int n);

/*    GRAPHGDI.C    */
void GDIin(void);
void GDIen(void);
void GDInl(void);
void GDIcl(void);
void GDIcr(INT ncol);
void GDImv(REAL x,REAL y);
void GDIdr(REAL x, REAL y);
void GDIst(char *s);
int getimage(int x1,int y1,int x2,int y2,void **loc);
void putimage(int x1,int y1,void *image);
void rectangle(unsigned x1, unsigned y1, unsigned x2, unsigned y2);
void clear_screen(unsigned x1,unsigned y1,unsigned x2,unsigned y2);
void remap(int code,char *red,char *green,char *blue);
int rgbtrans(int code);

#ifndef linux
void setcolour(COLORREF col,int style,int width);
#endif

/*	maximum number of hull sections					*/
/*	and maximum number of hull lines 				*/

#define maxsec 102

#ifdef EXT
  #define maxlin 12
#else
  #ifdef STUDENT
    #define maxlin 6
  #else
    #define maxlin 400
  #endif
#endif

/*	maximum drawing extra points per pair of hull sections		*/
#define maxext	8

extern char (*progtext)[17];
extern char (*progname)[MAX_PATH];
#ifdef PROF
extern char (*tankdesc)[MAX_PATH];
#endif

extern char (*lastname)[MAX_PATH];
extern char (*lastpath)[MAX_PATH];

extern char	*port;		/* hardcopy graphics port */
extern char	*cur_port;
extern char	*text_device;

/*	tank data table							*/

#ifdef PROF

#define	MAXTANKS	100	/* up to 100 tanks allowed */

extern int	*fl_line1;		/* first line for this tank */
extern int	*fl_fixed;		/* leaky or part full */
extern int	*fl_right;		/* on right side */

extern	REAL	*fl_volum;
extern	REAL	*fl_fract;
extern	REAL	*fl_spgra;
extern	REAL	*fl_walev;
extern	REAL	*fl_perm;
extern	REAL	*tankmass,*tankvcg,*tanklcg,*tanklcf,*tanktfsm,
		*tanklfsm,*tankmom;

extern	int	ntank;			/* number of tanks */

#endif

extern int	v7mode;

extern	int	extlin;			/* extended line count */
extern	int	tank;			/* current tank */

/*	graphics functions address table				*/

extern	FUNC_PTR		init,endgrf,clrgrf,newlin,setbyte;
extern	INTFUNC_PTR		colour;
extern	STRFUNC_PTR		plstr;
extern	REALFUNC_PTR	move,draw;
extern	FUNC_PTR		prev_func;
extern	FUNC_PTR		menufunc;
extern	FUNC_PTR		update_func;
extern	FUNC_PTR		print_func;

/*	number of data fields is 4 * number of hull lines		  */
/*	note that one each of the hardness (ycont) and vee (zcont) terms  */
/*	are redundant for each section, but are retained in the interests */
/*	of programming simplicity					  */

/*	Also, note that the order of the indices is reversed in comparison*/
/*	to that for the FORTRAN version.  This is because the sequence	  */
/*	in which they are incremented is the reverse of that for FORTRAN; */
/*	So that the address of a single line can be sent, and seen by a   */
/*	function as that of a singly-dimensioned array, the last index	  */
/*	must increment along a hull line				  */

extern REAL (*yline)[maxsec+4];
extern REAL (*zline)[maxsec+4];
extern REAL (*ycont)[maxsec+4] ;
extern REAL (*zcont)[maxsec+4] ;
extern REAL (*linewt)[maxsec+4];

extern REAL (*yline_main)[maxsec+4];
extern REAL (*zline_main)[maxsec+4];
extern REAL (*ycont_main)[maxsec+4];
extern REAL (*zcont_main)[maxsec+4];
extern REAL (*linewt_main)[maxsec+4];

extern REAL (*yline_overlay)[maxsec+4];
extern REAL (*zline_overlay)[maxsec+4];
extern REAL (*ycont_overlay)[maxsec+4];
extern REAL (*zcont_overlay)[maxsec+4];
extern REAL (*linewt_overlay)[maxsec+4];

extern char *hullfile_main,*hullfile_overlay;

extern int  *stsec,*stsec_main,*stsec_overlay;
extern int  *ensec,*ensec_main,*ensec_overlay;
extern int  *relcont,*relcont_main,*relcont_overlay;
extern REAL *xsect,*xsect_main,*xsect_overlay;
extern int numlin,numlin_main,numlin_overlay;
extern int extlin,extlin_main,extlin_overlay;
extern int stemli,stemli_main,stemli_overlay;
extern int count,count_main,count_overlay;
extern REAL *radstem,*radstem_main,*radstem_overlay;
extern int *autofair,*autofair_main,*autofair_overlay,*autofair_merge;

#ifdef PROF
extern	REAL	*radstem;
#endif
/*				stem radius data		*/

extern REAL *hwl;
/*				index of hull line onto which stem joins */
extern INT numun;
/*				index number of units used	*/
extern	char *lenun[4];
/*				table of length units		*/
extern	char *masun[4];
/*				table of mass units		*/
extern REAL g[4];
/*				acceleration due to gravity	*/
extern REAL densit[4];
/*				water density			*/
extern REAL invert;
/*				1 or -1, depending on dir'n of increase of z */
extern REAL posdir;
/*				number of hull sections		*/
extern char *hullfile,*dirnam,*filedirnam,*dxfdirnam;
/*				hull name and data directory name */

#ifdef PLATEDEV

/*	plate development parameters					*/

#define	MAXRUL	maxsec*maxext

extern	REAL  (*xstart)[MAXRUL];	/* starting points of ruled lines */
extern	REAL  (*ystart)[MAXRUL];
extern	REAL  (*zstart)[MAXRUL];

extern	REAL  (*xend)[MAXRUL];	/* end points of ruled lines */
extern	REAL  (*yend)[MAXRUL];
extern	REAL  (*zend)[MAXRUL];

/*	interpolation arrays in static storage, to allow access by	*/
/*	both development and viewing routines				*/

extern REAL	*xint_a;
extern REAL	*yint_a;
extern REAL	*zint_a;
		/* interpolated x-, y- and z-value tables for chine a */
extern REAL	*xint_b;
extern REAL	*yint_b;
extern REAL	*zint_b;
		/* interpolated x-, y- and z-value tables for chine b */

void getvector(REAL vecta[],REAL,REAL,REAL,REAL,REAL,REAL);

extern	int	*developed;
extern	int	*rulings;

extern	int	numruled;
extern	int	plate_lin;
extern	int	plate_res;

#endif

#ifdef PROF

/*	Stringer coordinates	*/

extern	REAL  (*ystr)[maxsec+2];
extern	REAL  (*zstr)[maxsec+2];
extern	REAL  (*astr)[maxsec+2];
extern	int   *ststr;
extern	int   *enstr;
extern	REAL  *ststrx;
extern	REAL  *enstrx;
extern	int   *numstr;
extern	int   *inistr;
extern  int   *str_dir;
extern	REAL  *str_thk;
extern	REAL  *str_wid;
extern	REAL  *str_interv;
extern	REAL  *str_firstint;
extern	int   *strmode;
#define str_firstfrac str_firstint
#define str_count str_interv

/*	transom parameters						*/

extern	int	transom;	/* transom on/off toggle		*/
extern	REAL	dztransom;	/* height of transom above keel		*/
extern	REAL	ztransom;	/* transom offset at stern		*/
extern	REAL	dtransom;	/* distance to transom plane from	*/
				/* x = 0, z = 0				*/
extern	REAL	atransom;	/* angle of transom from vertical	*/
extern	REAL	stransom,ctransom;/* transom slope sine and cosine	*/
extern	REAL	rtransom;	/* transom radius */

extern	REAL	*xtran;
extern	REAL	*ytran;	/* positions of intersections of lines with transom */
extern	REAL	*ztran;
#endif

/*	Surface-mode flag	*/

extern int surfacemode,surfacemode_main,surfacemode_overlay;

/*	Statics variables						*/

#define	numvar	   56
#define numstring  2
#define MAXSEL 20

extern REAL *varval;

#define	wl		(*(varval+0))
#define	heel	(*(varval+1))
#define	disp	(*(varval+2))
#define	pitch	(*(varval+3))
#define	xcofm	(*(varval+4))
#define	volu	(*(varval+5))
#define	lcb		(*(varval+6))
#define	lcf		(*(varval+7))
#define gz		(*(varval+8))
#define zmeta	(*(varval+9))
#define	mct		(*(varval+10))
#define	mpi		(*(varval+11))
#define	wplane	(*(varval+12))
#define	wetsur	(*(varval+13))
#define	loa		(*(varval+14))
#define	lwl		(*(varval+15))
#define	bmax	(*(varval+16))
#define	bwl		(*(varval+17))
#define	tc		(*(varval+18))
#define	fm		(*(varval+19))
#define	midsco	(*(varval+20))
#define	prisco	(*(varval+21))
#define	blocco	(*(varval+22))
#define	xentry	(*(varval+23))
#define	zcofm	(*(varval+24))
#define	Aabove	(*(varval+25))
#define	Zabove	(*(varval+26))
#define	Abelow	(*(varval+27))
#define	Zbelow	(*(varval+28))
#define	spgrav	(*(varval+29))
#define	garea	(*(varval+30))
#define	sarm	(*(varval+31))
#define	entang	(*(varval+32))
#define	zcofb	(*(varval+33))
#define	zbase	(*(varval+34))
#define	xstem	(*(varval+35))
#define	xstern	(*(varval+36))
#define	volforw	(*(varval+37))
#define	volstern (*(varval+38))
#define	xatbwl	(*(varval+39))
#define	xcofb	(*(varval+40))
#define	xlcf	(*(varval+41))
#define	xmid	(*(varval+42))
#define	density	(*(varval+43))
#define	gravity	(*(varval+44))
#define	kn		(*(varval+45))
#define	Xabove	(*(varval+46))
#define	Xbelow	(*(varval+47))
#define	surLCG	(*(varval+54))
#define	surVCG	(*(varval+55))

extern INT	numlin,stemli;
extern REAL	sina,cosa,rotn,beta;

extern	REAL	hcpy_xscale;	/* hardcopy plot x-scale factor */
extern	REAL	hcpy_yscale;	/* hardcopy plot y-scale factor */
extern	int	hardcopy;
extern	int	graphics;

extern	REAL	diag_int;	/* diagonals default off	*/
extern	REAL	diag_angle;	/* default angle 45 degrees	*/
extern	REAL	butt_int;	/* buttocks default off		*/
extern	REAL	wate_int;	/* waterlines default off	*/

#ifdef PROF
extern	int	showtanks;	/* tanks default off		*/
extern	int	showstringers;
#endif
extern	int	showframes;
extern	REAL	skinthickness;

extern	int	*lineon;	/* line-plotting flags		*/
extern	int	*secton;	/* section-plotting flags	*/

extern	char	*linestoplot;	/* line-plotting input string	*/
extern	char	*sectionstoplot;/* section-plotting input string*/
extern	char	*alsosect;
extern	char	*showline;

extern	REAL	xmin,xmax,ymin,ymax;		/* image limits, perhaps zoomed	*/
extern	INT	inisec,lassec;
extern	int	endlin;		/* end line, set depending on value of "showtanks */

extern	int	*master;	/* master section tags */

/*	screen text shadow		*/

extern	char	scr_ch[][100];

/*	left column of workspace	*/

extern	int	lx;

extern	int	mouse;

extern int toolheight,toolwidth;

#ifdef SHAREWARE
void nagbox(void);
#endif

void get_int_angle(int j,REAL *cosstem,REAL *sinstem);

/*    BALANC.C    */
void balanc(REAL *,int show);
/*    CALWAT.C    */
void calwat(INT i,REAL wl0,
	REAL *wlp,REAL *wlc,
	REAL *prevcl,REAL *currcl,
	REAL *prevyl,REAL *curryl,
	REAL *prevzl,REAL *currzl,
	INT *lprev,INT *lcurr,
	INT mode,INT pltopp,REAL side,INT *msec,int *noline);
/*    CLEARA.C    */
REAL cleara(INT k,REAL wl0,REAL *ylow,REAL *zlow,INT *lastlow,INT *firstlow);
REAL curoff(REAL a,REAL hb,REAL c,REAL hd,REAL yk,REAL zk,REAL *yl,REAL *zl);
/*    CONF_MEN.C    */
MENUFUNC get_config(void);
MENUFUNC set_hardcopy(void);
void get_hardcopy_device(int x,int y);
MENUFUNC set_port(void);
MENUFUNC set_scale(void);
MENUFUNC set_text_port(void);
MENUFUNC set_directory(void);
MENUFUNC set_units(void);
MENUFUNC set_x_positive(void);
MENUFUNC set_positive(void);
MENUFUNC set_density(void);
MENUFUNC save_config(void);
MENUFUNC show_config(void);
void strcp(char *dest,char *source);
MENUFUNC set_colours(void);
MENUFUNC set_palette(void);
void get_config(void);

/*    CURVAR.C    */
REAL curvar(REAL t1,REAL t2,REAL dwlinn,REAL sinal,REAL cosal,
		REAL a,REAL hb,REAL c,REAL hd,INT *outint,
  		REAL *hbeam,REAL *wetwid,REAL *wetw3,REAL *zwl,
  		REAL yorig,REAL zorig);
REAL areato(REAL t,REAL sinal,REAL cosal,REAL dwl,
		REAL a,REAL hb,REAL c,REAL hd);
/*    DEF_TANK.C    */
MENUFUNC floodable(void);
/*    DEF_TRAN.C    */
MENUFUNC def_transom(void);
MENUFUNC toggle_transom(void);
MENUFUNC height_transom(void);
MENUFUNC angle_transom(void);
MENUFUNC redef_transom(void);
void recalc_transom(void);
int curve_inters(REAL x0,REAL ax,
		 REAL y0,REAL ay,REAL by,REAL cy,
		 REAL z0,REAL az,REAL bz,REAL cz,
		 int *i0,int *i1,
		 REAL *tvalue,REAL *xvalue,REAL *yvalue,REAL *zvalue);
/*    DEVE_MEN.C    */
MENUFUNC calc_devel(void);
MENUFUNC set_lines(void);
MENUFUNC set_resolution(void);
MENUFUNC dev_pos(void);
MENUFUNC view_devel(void);
MENUFUNC undo_ruling(void);
MENUFUNC zoom_devel(void);
MENUFUNC write_devel(void);
MENUFUNC write_endpoints(void);
MENUFUNC draw_ruling(int k,int i,int col);
MENUFUNC fix_transom_curve(void);
MENUFUNC transom_rulings(void);
MENUFUNC dev_ignore(void);
void scrtrans(REAL xs,REAL ys,REAL zs,int *xa,int *ya);
/*    DRAG_MEN.C    */
MENUFUNC set_speed(void);
MENUFUNC set_spunit(void);
MENUFUNC show_areas(void);
MENUFUNC find_drag(void);
MENUFUNC plot_drag(void);
MENUFUNC find_gerrit(void);
MENUFUNC find_oortmerrssen(void);
MENUFUNC find_holtro(void);
MENUFUNC find_savitsky(void);
MENUFUNC find_sav_ward(void);
MENUFUNC plot_gerrit(void);
MENUFUNC plot_oort_menn(void);
MENUFUNC plot_savitsky(void);
int define_prop(void);
void dragpl(REAL speed,REAL tometr,REAL_FUNC f1,REAL_FUNC f2,char *n1,char *n2);
/*    DXF_OUTP.C    */
MENUFUNC dxf_output(void);
void write_xy(REAL x,REAL y);
void write_xyz(REAL x,REAL y,REAL z);
void write_devpt(REAL x,REAL y);
int transom_inters(int i,REAL *xk,REAL *yk,REAL *zk,REAL *a,REAL *b,
	REAL xvalue[],REAL yvalue[],REAL zvalue[]);
void doxy(REAL x,REAL y,REAL z,int dummy);
void doxz(REAL x,REAL y,REAL z,int dummy);
int coincident(int l,int j0,int j1);
void ensure_polyline(char c,int index,int dim);
void seqend(void);
/*    EDIT_MEN.C    */
MENUFUNC add_section(void);
MENUFUNC edit_add_ignore(void);
MENUFUNC edit_add_select(void);
MENUFUNC edit_add_create(void);
void shosta(void **save,INT h,INT w);
MENUFUNC delete_section(void);
void setall(REAL (*line)[maxsec+4],INT ns,INT ind,REAL value);
void movsec(INT i1,INT i2,INT id);

void spline(	REAL t[],REAL x[],REAL w[],INT n,
		REAL ts[],REAL xs[],INT ns,
		REAL run1,REAL run2);
void spline1(	REAL t[],REAL x[],REAL w[],INT n,
		REAL ts[],REAL xs[],INT ns,
		REAL run1,REAL run2,
		REAL deriv[]);
void spline2(	REAL t[],REAL x[],REAL w[],INT nt,
		REAL ts[],REAL xs[],INT ns,
		REAL run1,REAL run2,
		REAL deriv[],
		REAL a[],REAL b[],REAL c[]);

MENUFUNC edit_section(void);
MENUFUNC edit_edit_select(void);
MENUFUNC edit_edit_graphic(void);
MENUFUNC edit_edit_direct(void);
void movpoi(INT is,INT ix,INT ix1,REAL tval);
REAL axincr(REAL ra);
int promul(char *promsg,INT result[],INT nval,char *inp);
MENUFUNC insert_line(void);
MENUFUNC remove_line(void);
void copyline(INT destin,INT source,float del);
MENUFUNC alter_stemline(void);
MENUFUNC edit_alte_new(void);
MENUFUNC partial_line(void);
MENUFUNC rescale(void);
MENUFUNC sel_lines(void);
MENUFUNC convert_units(void);
REAL scalefac(INT row);
MENUFUNC mul_long(void);
MENUFUNC mul_lat(void);
MENUFUNC mul_vert(void);
REAL shiftdist(INT row);
MENUFUNC add_vert(void);
MENUFUNC add_long(void);
MENUFUNC resection(void);
MENUFUNC edit_rese_retain(void);
MENUFUNC edit_rese_into(void);
MENUFUNC edit_rese_ignore(void);
MENUFUNC edit_rese_specify(void);
MENUFUNC setup_sections(int option);
MENUFUNC edit_rese_perform(void);
void refit(int j,REAL xnew[],REAL ynew[],REAL znew[],
		REAL ycnew[],REAL zcnew[],INT newcou,INT is);
int sel_ignore(void);
/*    FAIR_STE.C    */
MENUFUNC fair_stem(void);
/*    FILE_MEN.C    */
void read_hullfile(FILE *fp,char *hullfile,int all);
void load_file(void);
void save_file(void);
void do_save_file(int directory);
void directory(void);
int show_directory(char **dirtable,char direc[]);
void erase_file(void);
INT make_name(INT xpos,INT ypos,char *filename,int directory,int change_name);
void dynam_waterline(void);
INT blank(char *line);
int openfile(char *filename,char *rw,char *title,char *filetype,
	char *defext,char *dirnam,FILE **fp);
void waterline_output(void);
void builders_output(void);
INT wrioff(FILE *fp,REAL a,REAL hb,REAL c,REAL hd,REAL yk,REAL zk,REAL zt,
	REAL del,INT nl);
INT wrunit(FILE *fp,INT nl,int mode,REAL x,REAL y);
void system_exit(void);
void show_memory(void);
int no_expand(void);
/*    FINDRM.C    */
void findrm(void);
/*    GERRIT.C    */
REAL gerrit(REAL speed,INT ind);
/*    GETNORM.C    */
void getnorm(float *normx,float *normy,float *normz,
	     float tangxt,float tangyt,float tangzt,
	     float blnx,  float blny,  float blnz,
	     float alnx,float alny,float alnz);
/*    GETPOINT.C    */
void getpoint(float *bx,float *by,float a1x,float a1y,float a2x,float a2y,
	      float adis,float bdis,float *plate_area);
/*    GRAFTEXT.C    */
void pstrxy(int x,int y,char *text);
void pstrx(int x,char *text);
void pstr(char *string);
void pchar(INT c);
void pintxy(int x,int y,char *text,INT value);
void pintx(int x,char *text,INT value);
void pint(char *text,INT value);
void preaxy(int x,int y,char *text,REAL value);
void preax(int x,char *text,REAL value);
void prea(char *text,REAL value);
void setcursor(int x,int y);
void showcursor(INT x,INT y);
int lenstr(char *s);
MENUFUNC dump_screen(void);
int open_text(FILE **fp,char dir[],char *ext);
/*    GRAPHDXF.C    */
void adxfcl(void);
void adxfin(void);
void adxfen(void);
void adxfnl(void);
void adxfcr(INT ncol);
void adxfst(char *s);
void fix_trans(void);
void adxfmv(REAL x,REAL y);
void adxfdr(REAL x,REAL y);
/*    GRAPHHEW.C    */
void hewlcl(void);
void hewlin(void);
void hewlen(void);
void hewlnl(void);
void hewlcr(INT ncol);
void hewlst(char *str);
void hewlmv(REAL x,REAL y);
void hewldr(REAL x,REAL y);
/*    GRAPHIBM.C    */
void opengr(int mode,char dir[]);
void closgr(void);
void outst(char *str);
void outstr(char *str,int l);
INT inpgra(void);
void await(INT time);
long hundredths(void);
/*    GRAPHMAS.C    */
void hcpy_summ(void);
void plfix(REAL a,INT w,INT d);
void plint(INT num,INT w);
void trans(REAL *x0,REAL *y0);
void cls(int scrollbar);
void clrtext(void);
int init_device(int devnum);
void setup(int devnum);
void null(void);
void get_zoom(	REAL xmin0,REAL xmax0,REAL ymin0,REAL ymax0,
		REAL *xmin,REAL *xmax,REAL *ymin,REAL *ymax);
void drawrect(int xleft,int xright,int ybottom,int ytop);
int getimage(int x1,int y1,int x2,int y2,void **loc);
void putimage(int x1,int y1,void *image);
void clear_screen(unsigned x1,unsigned y1,unsigned x2,unsigned y2);
long imagesize(int hei,int len);
void remap(int code,char *red,char *green,char *blue);
int rgbtrans(int code);
/*    GRAPHREG.C    */
void regicl(void);
void regiin(void);
void regien(void);
void reginl(void);
void regicr(INT ncol);
void regist(char *ix);
void regimv(REAL x,REAL y);
void wrireg(REAL x,REAL y);
void regidr(REAL x,REAL y);
/*    GRAPHSUB.C    */
void box(REAL x1,REAL y1);
void setranges(REAL x1,REAL x2,REAL y1,REAL y2);
void perspp(REAL xp,REAL yp,REAL zp,REAL ax,REAL ay,REAL az);
REAL sind(REAL x);
REAL cosd(REAL y);
void perset(INT l);
void prompt(char *s);
void plotp(REAL prevc,REAL currc,REAL prevy,REAL curry,REAL prevz,REAL currz,
	REAL side,INT mode,INT pltopp);
void maketrans(void);
void pltsel(REAL x,REAL y,REAL z,INT mode);
/*    GRAPHTEK.C    */
void tektcl(void);
void tektin(void);
void tekten(void);
void tektnl(void);
void tektst(char *str);
void tektmv(REAL x,REAL y);
void tektdr(REAL x,REAL y);
void tektcr(int col);
void tektch(INT iarg,int flag);
void tektse(void);
void tektat(INT i);
/*    GRAPHWMF.C    */
void wmfcl(void);
void wmfin(void);
void wmfen(void);
void wmfnl(void);
void wmfst(char *str);
void wmfmv(REAL x,REAL y);
void wmfdr(REAL x,REAL y);
void wmfcr(int col);
/*    HIDDEN.C    */
void hidden_surface(REAL side1,REAL side2,int endlin);
void transform(REAL *x,REAL *y,REAL *z);
void screen_transform(REAL *x,REAL *y,REAL *z);
int reinterp(REAL *y1,REAL *z1,REAL *c1,
	     REAL  y2,REAL  z2,REAL  c2);
REAL dist_to(REAL x,REAL y1,REAL z1);
int tanknext(int i,int *j,int *tank,int endlin);
void starttank(int i,int j,int *tank,int *right,int *left,
	REAL *a1,REAL *hb1,REAL *c1,REAL *hd1,
	REAL *a2,REAL *hb2,REAL *c2,REAL *hd2,
	REAL *x12,REAL *y12,REAL *z12,REAL *c12,
	REAL *x22,REAL *y22,REAL *z22,REAL *c22);
void set_tri(REAL x11,REAL y11,REAL z11,
	REAL x12,REAL y12,REAL z12,
	REAL x21,REAL y21,REAL z21,
	REAL x22,REAL y22,REAL z22,
	int *n,int dn);
void set_xyz(REAL *x12s,REAL *y12s,REAL *z12s,REAL *c12s,
	REAL *x22s,REAL *y22s,REAL *z22s,REAL *c22s,int i, int j);
/*    HOLTRO.C    */
REAL holtro(REAL speed,INT indic);
REAL fexp(REAL expon);
REAL fpow(REAL base,REAL expon);
/*    HULDIS.C    */
void huldis(REAL *calcdisp);
/*    HULLAR.C    */
void hullar(INT k,int j1,int j2,
	REAL *areain,REAL *areaout,
	REAL wl1,REAL sinal,REAL cosal,REAL *hbeam,REAL *wetwid,
	REAL *wetw3,REAL *zwl,INT *line);
/*    HULLFORM.C    */
void freedisplay(void);
void message(char *text);
void fp_abort(void);
int memavail(void **loc,long size);
void memory_abort(char *);
void memfree(void *loc);
int altavail(void **loc,long size);
int realloc_hull(int size);
MENUFUNC quit(void);
/*   HULLMOME.C    */
REAL calcrm(INT i,int j1,int j2,int left,int right,
	REAL wloff,REAL *rm1,REAL *rm2,REAL *cl1,REAL *cl2,
	REAL *zsum1,REAL *zsum2,REAL *zsumav);
void rightm(int i,int j1,int j2,REAL sinal,REAL cosal,
	REAL wl1,REAL *zsum,REAL *rmin,REAL *rmout);
REAL secmom(REAL y0,REAL z0,
	REAL a,REAL hb,REAL c,REAL hd,
	REAL sinal,REAL cosal,
	REAL wl0,REAL *zsum);
REAL curmom(	REAL t0,
		REAL a,REAL hb,REAL c,REAL hd,
		REAL y0,REAL z0,
		REAL wl0,
		REAL sinal,REAL cosal,
		REAL *zsum);
/*    HULLPA.C    */
void hullpa(INT k,INT j,REAL prevwid,REAL  prevris,REAL *a,REAL *hb,REAL *c,REAL *hd);
void tranpa(REAL a,REAL hb,REAL c,REAL hd,REAL *aa,REAL *cc);
void inters(REAL a,REAL hb,REAL c,REAL hd,REAL sina,REAL cosa,REAL dwl,
		REAL *t1,REAL *t2);
void drasec(INT k,int right,REAL fac,int mode);
void getparam(int i,int j,REAL *a,REAL *hb,REAL *c,REAL *hd);
/*    HULLSEC1.C    */
void maksec(REAL xpos,INT nsec,INT isec);
void calcar(INT i,int j1,int j2,int left,int right,
	REAL *area1,	/* area of previous section (set to input value of area2) */
	REAL *area2,	/* calculated area of next section */
	REAL *hbeam1,	/* previous half-beam */
	REAL *hbeam2,	/* next half-beam */
	REAL *cl1,	/* previous waterline clearance */
	REAL *cl2,	/* next waterline clearance */
	REAL xref1,	/* x-value at previous section */
	REAL xref2,	/* x-value at next section */
	REAL *rsum,	/* righting moment per unit length and radian of heel */
	REAL *volume,	/* displaced volume of water */
	REAL *calcdisp,	/* calculated displacement */
	REAL *totmom,	/* integrated sectional area with respect to length */
	REAL *watpla,
	REAL wll,	/* waterline offset to use */
	REAL densit,	/* liquid density */
	REAL *wllen,	/* waterline length */
	REAL *xsta,	/* waterline entry point */
	REAL *xend,	/* waterline exit point */
	REAL *sumlcf,	/* integrated width with respect to length squared */
	REAL *summct,	/* integrated width with respect to length cubed, time density */
	REAL *ww1,	/* previous wetted width */
	REAL *ww2,	/* current wetted width */
	REAL *rm1,	/* previous righting moment per unit length */
	REAL *rm2,	/* current righting moment per unit length */
	REAL *sumysqdif,/* summed difference of squared edge offsets */
	REAL *sumyoffs);/* summed edge offsets */
/*    HULLSMOO.C    */
MENUFUNC smooth_lines(void);
void smoo_sel(void);
void smoo_off(void);
void smoo_cont(void);
void smoo_prev(void);
void exit_line(void);
void exit_move(void);
void exit_perf(void);
void smoo_angl(void);
void angl_move(void);
void angl_perf(void);
void plotline(REAL ylo1,REAL yhi1,REAL ylo2,REAL yhi2,REAL offsetb[],int col);
void shopul(REAL del);
INT errinp(char *promst,REAL *result);
REAL dxstem(void);
void movepoint(int keyinp,int ix,int linnum,REAL del1,
int noff,REAL curmin,REAL curmax,REAL offbot,REAL offtop);
/*    HULSUR.C    */
REAL hulsur(REAL sinal,REAL cosal);
REAL surfar(REAL c0[],REAL c1[],REAL c2[]);
REAL triar(REAL c0[],REAL c1[],REAL c2[]);
REAL sq(REAL a,REAL b);
void linint(REAL c0[],REAL c1[],REAL c2[]);
void setxyz(int l,int i,REAL t,REAL xyz[4],REAL sinal,REAL cosal,
    REAL a,REAL hb,REAL c,REAL hd);
/*    INTCHIA.C    */
void intchia(float *intchix,float *intchiy,float *intchiz,
	     float achix[], float achiy[], float achiz[],
	     int i,float prev,float cur);

int multin(char *prompt,INT relax[],INT maxs);
int multproc(char *line,int table[],int maxtab);
INT nread(char **str);
/*    MODSEC.C    */
MENUFUNC modsec(INT is);
void draw_edit_sect(INT is,REAL yra,INT offset,INT *first,REAL zsc);
int nearest_offset(int *i,int *xm,int *ym);
int nearest_control(int *i,int *xm,int *ym);
int trackvalues(int *xm,int *ym,int *xp,int *yp,REAL *x,REAL *y);
void showvalues(REAL y,REAL z);
/*    NEW_HULL.C    */
MENUFUNC new_hull(void);
/*    NORM.C    */
float norm(float vector[]);
void   getab(float *a,float *b,float vecta[],float vectb[]);
/*    OORTME.C    */
REAL oortme(REAL speed,INT indic);
REAL halfen(void);
/*    PLOT_VAR.C    */
MENUFUNC plot_all(void);
INT ident(char *match,char *model[],INT num);
void graph(REAL x[],REAL y[],INT n,char *xaxis,char *yaxis,REAL x0,REAL y0);
int getname(char *name,int x,int y);
int variable_index(char *inpvar,char *prompt_message,int line,int maxindex);
MENUFUNC hcpy_stat(void);
/*    RELINE.C    */
MENUFUNC reline(void);
MENUFUNC make_buttocks(void);
MENUFUNC make_diagonals(void);
MENUFUNC make_waterlines(void);
int get_l_int(int row);
void do_diagonals(int row);
/*    REVSEC.C    */
MENUFUNC revsec(INT ns);
int getxy(int i, int j, int *x, int *y);
/*    ROLLOUT.C    */
MENUFUNC rollout(void);
MENUFUNC write_outline(void);
void get_rollout(float xedge_a[],float yedge_a[],
		 float xedge_b[],float yedge_b[],
		float *xmin1,float *xmax1,float *ymin1,float *ymax1,
		float *plate_area);
void getvector(float vect[],
	     float ax,float bx,      /* point a and b of line */
	     float ay,float by,
	     float az,float bz);

#ifdef  PROF
void plot_marks(REAL (*xs)[MAXRUL],REAL xe[],REAL ye[],REAL t,int numrul);
#endif

/*    SAVITS.C    */
REAL savits(REAL sp,INT setup);
REAL flambd(REAL lponb,REAL Cv);
REAL ulambda(REAL t1p1);
REAL fcl0(REAL Clb,REAL deadri);
REAL prepla(REAL speed,INT setup);
REAL effpow(REAL power,char **unit);
REAL savitsky_balance(REAL t1p1,REAL speed,REAL *dragf1);
/*    SCA_VIEW.C    */
void scale_view(REAL rotn,REAL pp,REAL yview,REAL zview,INT *inisec,INT *lassec);
void vpos(REAL xm,REAL xd,REAL ym,REAL yd,REAL zmin,REAL zmax,REAL zd);
void vtran1(REAL xm,REAL xd,REAL ym,REAL yd,REAL zm,REAL zd);
void vtran(REAL x0,REAL y0,REAL z0);
/*    STAT_MEN.C    */
MENUFUNC set_heel(void);
MENUFUNC set_pitch(void);
MENUFUNC set_displ(void);
MENUFUNC set_xcofm(void);
MENUFUNC set_zcofm(void);
MENUFUNC set_waterl(void);
MENUFUNC find_float(void);
MENUFUNC balance_all(void);
MENUFUNC find_displ(void);
MENUFUNC write_stats(void);
void show_stats(void);
/*    VIEW_MEN.C    */
MENUFUNC general_orth(void);
MENUFUNC plan_orth(void);
MENUFUNC elev_orth(void);
MENUFUNC end_orth(void);
MENUFUNC full_persp(void);
void draw_persp(REAL side1,REAL side2);
MENUFUNC port_persp(void);
MENUFUNC starb_persp(void);
MENUFUNC viewing_pos(void);
void get_viewpos(int x,int y);
MENUFUNC view_sections(void);
MENUFUNC view_lines(void);
MENUFUNC view_tanks(void);
MENUFUNC view_diagonals(void);
MENUFUNC view_buttocks(void);
MENUFUNC view_waterlines(void);
MENUFUNC rotate_persp(void);
int rotate_file(void);
MENUFUNC perform_rotate(void);
MENUFUNC shaded_surfaces(void);
MENUFUNC view_zoom(void);
void get_ranges(REAL *xa,REAL *xb,REAL *ya,REAL *za,REAL *zb);
/*    VIEW_SU1.C    */
void top_plan(REAL xxmin,REAL xxmax,REAL yymin,REAL yymax);
void side_elevation(REAL xl,REAL xr,REAL yb,REAL yt);
void end_elevation(REAL xl,REAL xr,REAL yb,REAL yt);
void draw_datum(void);
void show_scale(REAL xleft,REAL xright,REAL ybottom,REAL ytop);
void draw_perspective(REAL side1,REAL side2);
void draw_view(REAL x1,REAL x2,REAL y1,REAL y2,
	INT inisec,INT lassec,int endlin,REAL side1,REAL side2,
	REAL heel0,REAL pitchv,REAL rotn,INT secton[],INT lineon[],
	REAL wate_int,REAL diag_int,REAL diag_angle,REAL butt_int);

/*    VIEW_SUB.C    */
void plotw(REAL side,REAL wli,REAL ddwl,INT mode);
void pltdia(REAL interv,REAL diaval,INT mode,REAL side);
void draw_transom(REAL side1,REAL side2,int mode);
void draw_stringers(REAL side1,REAL side2,int mode,int iswap);
void draw_line(int il,REAL side1,REAL side2,int mode,int inisec,
	int lassec,int swap,REAL *xrate,REAL *yrate,REAL *zrate,int *num,
	int *stem,int *tran,int all);
void plot_transom_end(int i,REAL a, REAL hb, REAL c, REAL hd,REAL side,
	REAL xx1,int mode,void func(REAL x,REAL y,REAL z,int mode));
void interp_lines(int jj,int sts,int ens,int *numint,
	REAL ytem[],REAL ztem[],REAL delx,
	REAL xvalue[],REAL yvalue[],REAL zvalue[],REAL c,REAL s,REAL r,
	REAL *xrate,REAL *yrate,REAL *zrate,
	int *stem_ind,int *tran_ind,int all);
void offset_range(REAL *maxoff,REAL *minoff,REAL diaval,REAL interv);

void context(int id);

/*	arithmetic routines			*/

#define	fmul(a,b)	((a) * (b))
#define	fdiv(a,b)       ((a) / (b))
#define	fadd(a,b)       ((a) + (b))
#define	fsub(a,b)       ((a) - (b))
#define	fmin(a)         (-(a))
#define	fabf(a)         fabs(a)

#define fgt(a,b)        ((a) >  (b))
#define fge(a,b)        ((a) >= (b))
#define flt(a,b)        ((a) <  (b))
#define fle(a,b)        ((a) <= (b))
#define feq(a,b)        ((a) == (b))
#define fne(a,b)        ((a) != (b))

#define	fpos(a)		((a) >  0.0)
#define	fpoz(a)         ((a) >= 0.0)
#define	fneg(a)         ((a) <  0.0)
#define fnez(a)         ((a) <= 0.0)
#define	fzer(a)         ((a) == 0.0)
#define fnoz(a)		((a) != 0.0)

#ifndef max
#define	max(a,b)	(a > b ? a : b)
#endif

extern REAL fsqr0(REAL);

#ifndef CLK_TCK
#define CLK_TCK  1000.0
#endif

extern int assistant;
