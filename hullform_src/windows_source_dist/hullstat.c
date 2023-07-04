/* Hullform component - hullstat.c
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

//	Dummies

#ifndef HULLSTAT

REAL varvalue[2];
extern char *par[];	/* pointers to parameters in evaluation string	*/
int maxvar = 12;
char varname[13][2];
int	varsize[2];
int	numstatvar = 1;
char vartext[41];
int	textdatasize = 41;
int	vartype[2];		/* number, text or array */

#endif

#ifdef PROF

#include "hst-help.h"
#include <sys/stat.h>

#ifdef linux

#include <utime.h>
#include <Xm/List.h>
#include <Xm/Text.h>

#else

#include <sys/utime.h>
#include <htmlhelp.h>

struct  ffblk   {
	long            ff_reserved;
	long            ff_fsize;
	unsigned long   ff_attrib;
	unsigned short  ff_ftime;
	unsigned short  ff_fdate;
	char            ff_name[256];
};
int findfirst(char *fullname,struct ffblk *blk,unsigned mode);
int findnext(struct ffblk *blk);

#include <sys\utime.h>

#endif

void hewlin(void);
void wmfin(void);
void hewlen(void);
void wmfen(void);
void hewlnl(void);
void wmfnl(void);
void hewlmv(REAL,REAL);
void wmfmv(REAL,REAL);
void hewldr(REAL,REAL);
void wmfdr(REAL,REAL);
void hewlst(char *);
void wmfst(char *);
void statcolour(int);
void statsize(int);
void statfont(char *);
void stattextstyle(int,int);
void wmfsetfont(short int width,short int height,char *name,
	short int bold,short int italic,short int changerot,REAL rotval);
void checkdir(char *);
void finish_stats(void);
void strchk(char *s);
void project_directories(int code,HWND hWndDlg);
void striptopath(char *path);

void DirListListBox(HWND hWnd,WPARAM code,char *path);
void DirListComboBox(HWND hWnd,WPARAM code,char *path);
int ColourMessage(unsigned msg,WORD wParam,LONG lParam,BOOL *hBrush);

void abortp(char *s);
void save_hull(int);
extern int getdlg_proceed;
extern int ddemode;
extern int linereq;
REAL findvalue(char *var,int *indval);
int same(char *s1,char *s2);
void inc_numstatvar(void);

int table_index = -1;
int opened = FALSE;

#define	UNDEF		0
#define	NUMBER		1
#define	TEXTDATA	2
#define	ARRAY		3
#define	FUNC		4
#define TEXTARRAY	5

REAL *varvalue = NULL;

#define HSL 99

#ifdef DEMO
extern int allow_hullstat;
#endif

extern char *helpfile;
extern int context_id;
extern char *context_string;
void centre_dlg(HWND);
extern int HelpUsed;

void new_project(void);
void read_project(FILE *f);
void load_project(void);
void save_project(int newname);
void edit_project_files(void);
void edit_output(void);
void edit_template(void);
void edit_static_loads(void);
void edit_immersion_points(void);
void delete_immersion_point(void);
void edit_conditions(void);
void edit_preferences(void);
void generate(void);
void messagedata(char *s,char *data);
void send_command(void);
int parse(char *lp);
void writestr(char *str);
REAL eval(char *par[],char op[],int level[],int *type);
int prec(char op);
void showtext(char *str);
void truncate(char *);
int filecopy(char *srcdir,char *srcfile,char *desdir,char *desfile);
int strconv(char *s,char *str);

void SetCondition(void);
void SetImmersionPoint(void);
void SetLoad(void);
void SetTank(void);
REAL readvalue(char *par,int *type);
void opengraphics(void);
void closegraphics(void);
void statgraph(REAL x[],REAL y[],INT n,
	char *xaxis,REAL xmin,REAL xmax,REAL stat_dx,
	char *yaxis,REAL statymin,REAL statymax,REAL stat_dy);
REAL axincr(REAL ra);
void statmove(REAL x,REAL y);
void statdraw(REAL x,REAL y);
void statstyle(int style);
void statwidth(int width);
void writeprec(REAL value,char *subst);
void pause(REAL sec);
void setrot(REAL angle);
void drawst(char *s);
void statnewlin(void);
int memavail(void **loc,long size);
void memfree(void *loc);
void filesummary(char *descr,char *name);
void striprtf(char *);
void statedit(void);

HWND hShow;
HWND hStat;
HWND hStop;

int blockstate[20] = {
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
int blocklevel = 0;
int activelevel = 0;
long blockstart[20];
REAL statcharwid,statcharhei;
char *statcfgp;

extern REAL xgslope,ygslope,xgorigin,ygorigin;

#define LOOP 1
#define IFBLOCK	 2
#define NOMOREIFS -3
#define LOOPWHILE 4
#define LOOPUNTIL 5

/*	These four must be last	*/

#define FORALLCOND 6
#define FORALLIMME 7
#define FORALLLOAD 8
#define	FORALLTANK 9

/*	Error message information	*/

char *statedesc[] = {
	"skip 'foreach tank' loop",
	"skip 'foreach load' loop",
	"skip 'foreach immersionpoint' loop",
	"skip 'foreach condition' loop",
	"skip loop until",
	"skip loop while",
	"skip no more ifs",
	"skip ifblock",
	"skip loop",
	"no loop or conditional",
	"loop",
	"if block",
	"skipping to end of if block",
	"loop while",
	"loop until",
	"foreach condition loop",
	"foreach immersionpoint loop",
	"foreach load loop",
	"foreach tank loop"};

/*					*/
int sendrecv(char *cmd,char *arg,char *out,REAL *result);

#define ylower	1360.0
#define	xleft	1150.0
#define	xright	9600.0
#define yupper  7600.0

struct {
	int hs_index;       /* hs_index of result in table */
	char *string;    /* pointer to result */
	char *table[20]; /* table of strings for listbox, null string terminator */
}
condlist,loadlist,immelist,tanklist;

int is_command,was_command;
int debug = 0;
int outlen;
extern int parse0;
char hullname[MAX_PATH];
int linelength;
REAL statxmin,statxmax,statymin,statymax,stat_dx,stat_dy;
int	condindex = 0;
int	immeindex = 0;
int	loadindex = 0;
int	tankindex = 0;
char	*lnp;	/* pointer within template line */
#define BUFFERSIZE   4096
char *statbuffer;	/* input buffer */
char *statline = NULL;	/* template line */
char *bufp;
extern int brlev;
int hpgl = 1;
int textsize = 12;
int listcount = 0;

char *month[] = {
	"Jan","Feb","Mar","Apr","May","Jun",
	"Jul","Aug","Sep","Oct","Nov","Dec"};

int maxvar = 2000;
char	(*varname)[13];
int	(*varsize);
int	numstatvar = 1;
char	*vartext;
int	textdatasize = 81;
int	(*vartype);		/* number, text or array */

extern int level[40];
extern char op[40];
extern char *par[40];	/* pointers to parameters in evaluation string	*/
int	rtf = 0;
char	reqpar[500];
long	fileloc;	/* points to next block/line after the current one */
long	whileloc = 0;
long	readloc;
int	dispoption = 0;

extern int paused;

/*	Project parameters			*/

char	project[MAX_PATH] = "";
char	editor[MAX_PATH] = "notepad";
char	vessel[MAX_PATH];
int		decimal = 3;
char	outdirnam[MAX_PATH] = ".";
char	outfile[MAX_PATH] = "";
char	tmpdirnam[MAX_PATH] = ".";
char	templat[MAX_PATH] = "";
char	projdirnam[MAX_PATH] = ".";

/********************************************************/

/*		Sailing conditions			*/

int	condnum = 0;
int	numcond = 0;
char	condition[MAXCOND][81];
char	state[MAXCOND][81];
REAL	specgrav[MAXCOND];
char	compliance[MAXCOND][81];

REAL	xload[MAXCOND][MAXLOAD];
REAL	yload[MAXCOND][MAXLOAD];
REAL	zload[MAXCOND][MAXLOAD];
REAL	mload[MAXCOND][MAXLOAD];
REAL	wload[MAXLOAD];
REAL	lload[MAXLOAD];
REAL	hload[MAXLOAD];
REAL	defxload[MAXLOAD];
REAL	defyload[MAXLOAD];
REAL	defzload[MAXLOAD];
REAL	defmload[MAXLOAD];
char	loaddesc[MAXLOAD][41];
int	numload;

REAL	ximme[MAXIMME];
REAL	yimme[MAXIMME];
REAL	zimme[MAXIMME];
char	immedesc[MAXIMME][41];
int	numimme;

/*	Hull tank count		*/

int	numtank;
REAL	tankperc[MAXCOND][MAXTANK];
REAL	tankspgr[MAXCOND][MAXTANK];
REAL	tankperm[MAXCOND][MAXTANK];

/********************************************************/

FILE *fout = NULL;
FILE *tmpl;
int graphicsopen = FALSE;
int graphicsok(void);
char *editfile;
extern int htmlhelp;

void hullstat_funcs(int code,HWND hWndDlg);

#ifdef linux
int MessageBox(HWND hWnd,char *text,char *title,int button);
extern Widget wListBox[],wComboBox[];

char *_fullpath(char *fullname,char *name,int len)
{
	char *p;
	int l;

	if(*name != '/') {
		getcwd(fullname,MAX_PATH);
		p = strchr(fullname,0);
		*p++ = '/';
		l = p - fullname;
		if(l < len) return NULL;
		strcpy(p,name);
	}
	else {
		strcpy(fullname,name);
	}
	return fullname;
}

#endif

void hullstat()
{
	extern char *cfgfile;
	char *p;
	FILE *fp;
	char statcfg[MAX_PATH];
	char hfhelp[MAX_PATH];

	update_func = NULL;
	context_id = 0;	// use context_string

#ifdef DEMO
	(void) getdlg(HSTDEMO,-1);
#endif

	strcpy(hfhelp,helpfile);
	if(htmlhelp) {
		strcat(helpfile,"::/hullstat.html");
	} else {
		p = strstr(helpfile,"hullform.");
		if(p != NULL) strncpy(p+4,"stat",4);
	}

#ifdef DBG
	dbg = fopen("hullstat.dbg","wt");
#endif

	/*	Read configuration file, if it exists	*/

	strcpy(statcfg,cfgfile);
#ifndef linux
	strlwr(statcfg);
#endif
	p = strstr(statcfg,"hullform.cfg");
	if(p != NULL)
		strncpy(p+4,"stat",4);
	else
	    strcpy(statcfg,"hullstat.cfg");
	statcfgp = statcfg;
	if((fp = fopen(statcfg,"rt")) != NULL) {
		if(fscanf(fp,"%d %d %d %d %d\n",&dispoption,&hpgl,
			&textdatasize,&maxvar,&rtf) == 5) {
			if(fgets(projdirnam,256,fp) != NULL) {
				if((p = strchr(projdirnam,'\n')) != NULL) *p = 0;
			}
			else {
				strcpy(projdirnam,".");
			}
		}
		fclose(fp);
	}

	/*	Allocate memory				*/

	if(	!memavail((void **) &statline,BUFFERSIZE) ||
		    !memavail((void **) &statbuffer,BUFFERSIZE) ||
		    !memavail((void **) &varname ,maxvar*sizeof(*varname)) ||
		    !memavail((void **) &varvalue,maxvar*sizeof(*varvalue)) ||
		    !memavail((void **) &varsize ,maxvar*sizeof(*varsize)) ||
		    !memavail((void **) &vartext ,maxvar*textdatasize) ||
		    !memavail((void **) &vartype ,maxvar*sizeof(*vartype))
		    ) {
		message("Insufficient memory to run program");
	}
	else {

		/*	Display the dialog box		*/

		cls(0);
		(void) getdlg(DLGHULLSTAT,
			INP_PBF,(void *) hullstat_funcs,
			-1);
		strcpy(helpfile,hfhelp);
	}
	/*	Free memory			*/

	memfree(statline);
	statline = NULL;	/* used as Hullstat-active indicator */
	memfree(statbuffer);
	memfree(varname);
	memfree(varvalue);
	memfree(varsize);
	memfree(vartext);
	memfree(vartype);

	/*	Save the configuration		*/

	if((fp = fopen(statcfg,"wt")) != NULL) {
		fprintf(fp,"%d %d %d %d %d\n",dispoption,hpgl,textdatasize,
			maxvar,rtf);
		fprintf(fp,"%s\n",projdirnam);
		fclose(fp);
	}
}

void hullstat_funcs(int code,HWND hWndDlg)
{
	char command[MAX_PATH];
	extern int getdlg_proceed;
	void showhelp(char *f);

	hStat = hWndDlg;
	switch(code) {
	case 0:			/* New project */
		new_project();
		break;
	case 1:			/* Open Project */
		load_project();
		break;
	case 2:			/* Save Project */
		save_project(FALSE);
		break;
	case 3:			/* Save Project As ... */
		save_project(TRUE);
		break;
	case 4:			/* Edit, project file */
		chdir(projdirnam);
		if(MessageBox(hWndDlg,
			"If you alter the first 20 columns of this file, it will not be readable. Do you wish to proceed?",
			"YOU ARE ABOUT TO ALTER A CRITICAL FILE",MB_ICONHAND | MB_YESNO) == IDYES) {
#ifdef linux
			strcpy(command,"edit ");	/* presume some editor */
			strcat(command,project);
			strcat(command," &");
			if(system(command) != 0) message("Can not find an editor named \"edit\"");
#else
			strcpy(command,"notepad.exe ");
			strcat(command,project);
			if(WinExec(command,SW_SHOWNORMAL) < 32) message("Can not run editor NOTEPAD.EXE");
#endif
		}
		break;
	case 5:			/* Edit file list */
		edit_project_files();
		break;
	case 6:
		edit_static_loads();
		break;
	case 7:
		edit_immersion_points();
		break;
	case 8:
		edit_conditions();
		break;
	case 9:
		statedit();
		break;
	case 10:	/* Go */
		generate();
#ifdef DEMO
		if(allow_hullstat) {
#endif
		getdlg_proceed = 0;
		while(tmpl != NULL && getdlg_proceed == 0) send_command();
#ifdef DEMO
		}
#endif
		break;
	case 11:	/* Stop */
		abortp("$** Run stopped at user request **");
		getdlg_proceed = 1;
		break;
	case 12:	/* Edit, Template */
		chdir(tmpdirnam);
		edit_template();
		break;
	case 13:	/* Edit, Output file */
		chdir(outdirnam);
		edit_output();
		break;
	case 14:
		edit_preferences();
		break;
	case 15:	/* Help */
#ifdef linux
		showhelp("hullstat.html");
#else
		showhelp(helpfile);
#endif
		break;
	}
}

/*****************************************************************/

extern HWND hWndMain;

void load_project()
{
	FILE *fp;
	if(openfile(project,"rt","Open a Project file",
		"Stability projects (*.hsp)\0*.hsp\0","*.hsp",projdirnam,&fp)) read_project(fp);
}

void edit_template()
{
	char command[MAX_PATH];
	strcpy(command,editor);
	strcat(command," \"");
	strcat(command,templat);
	strcat(command,"\"");
#ifdef linux
	if(system(command) != 0) messagedata("Can not run program:",command);
#else
	if(WinExec(command,SW_SHOWNORMAL) < 32) messagedata("Can not run program:",command);
#endif

}

void edit_output()
{
	char command[MAX_PATH];
	strcpy(command,editor);
	strcat(command," \"");
	strcat(command,outfile);
	strcat(command,"\"");
#ifdef linux
	if(system(command) != 0) messagedata("Can not run program:",command);
#else
	if(WinExec(command,SW_SHOWNORMAL) < 32) messagedata("Can not run program:",command);
#endif
}

void read_project(FILE *fp)
{
	char *p;
	char text[MAX_PATH];
	int i,j,n;
	char leader[21];
	int linnum = 0;
	int goodheader(FILE *fp,int *line,const char *msg);

	if(fp == NULL) return;

	for(n = 1 ; n < maxvar ; n++) *varname[n] = 0;
	numstatvar = 1;

	linnum++;
	fgets(leader,21,fp);
	if(strcmp(leader,"Vessel              ")) goto error;
	fgets(vessel,MAX_PATH,fp);
	p = strchr(vessel,'\n');
	if(p != NULL) *p = '\0';
	vessel[textdatasize-1] = 0;
	strcpy(varname[0],vessel);
	vartype[0] = TEXTDATA;

	if(!goodheader(fp,&linnum,"Hull name           ")) goto error;
	fgets(hullname,MAX_PATH,fp);
	strchk(hullname);
	if(!sendrecv("open",hullname,NULL,NULL))
		message("Hull data file not found.\nUse \"File list\", \"Browse\" to locate it.");

	if(!goodheader(fp,&linnum,"Editor              ")) goto error;
	fgets(editor,MAX_PATH,fp);
	strchk(editor);

	if(!goodheader(fp,&linnum,"Template directory  ")) goto error;
	fgets(tmpdirnam,MAX_PATH,fp);
	strchk(tmpdirnam);

	if(!goodheader(fp,&linnum,"Output directory    ")) goto error;
	fgets(outdirnam,MAX_PATH,fp);
	strchk(outdirnam);

	if(!goodheader(fp,&linnum,"Condition count     ")) goto error;
	fscanf(fp,"%d\n",&numcond);

	if(!goodheader(fp,&linnum,"Load count          ")) goto error;
	fscanf(fp,"%d\n",&numload);
	for(j = 0 ; j < numload ; j++) {
		(void) goodheader(fp,&linnum,"                    ");
		fgets((char *) loaddesc[j],41,fp);
		strchk((char *) loaddesc[j]);
		loaddesc[j][textdatasize-1] = 0;
		if(!goodheader(fp,&linnum,"Default X-Position  ")) goto error;
		fscanf(fp,"%f\n",&defxload[j]);
		if(!goodheader(fp,&linnum,"Default Y-Position  ")) goto error;
		fscanf(fp,"%f\n",&defyload[j]);
		if(!goodheader(fp,&linnum,"Default Z-Position  ")) goto error;
		fscanf(fp,"%f\n",&defzload[j]);
	}

	if(!goodheader(fp,&linnum,"Immersion points    ")) goto error;
	fscanf(fp,"%d\n",&numimme);
	for(j = 0 ; j < numimme ; j++) {
		if(!goodheader(fp,&linnum,"Description         ")) goto error;
		fgets((char *) immedesc[j],MAX_PATH,fp);
		strchk((char *) immedesc[j]);
		immedesc[j][textdatasize-1] = 0;
		if(!goodheader(fp,&linnum,"X-Position          ")) goto error;
		fscanf(fp,"%f\n",&ximme[j]);
		if(!goodheader(fp,&linnum,"Y-Position          ")) goto error;
		fscanf(fp,"%f\n",&yimme[j]);
		if(!goodheader(fp,&linnum,"Z-Position          ")) goto error;
		fscanf(fp,"%f\n",&zimme[j]);
	}

	if(!goodheader(fp,&linnum,"Number of tanks     ")) goto error;
	fscanf(fp,"%d\n",&numtank);

	for(i = 0 ; i < numcond ; i++) {

		if(!goodheader(fp,&linnum,"CONDITION           ")) goto error;
		fgets(condition[i],MAX_PATH,fp);
		strchk(condition[i]);
		condition[i][textdatasize-1] = 0;

		if(!goodheader(fp,&linnum,"State               ")) goto error;
		fgets(state[i],MAX_PATH,fp);
		strchk(state[i]);
		state[i][textdatasize-1] = 0;

		if(!goodheader(fp,&linnum,"Static loads\n")) goto error;
		for(j = 0 ; j < numload ; j++) {
			if(!goodheader(fp,&linnum,"X-Position          ")) goto error;
			fscanf(fp,"%f\n",&xload[i][j]);
			if(!goodheader(fp,&linnum,"Y-Position          ")) goto error;
			fscanf(fp,"%f\n",&yload[i][j]);
			if(!goodheader(fp,&linnum,"Z-Position          ")) goto error;
			fscanf(fp,"%f\n",&zload[i][j]);
			if(!goodheader(fp,&linnum,"Mass                ")) goto error;
			fscanf(fp,"%f\n",&mload[i][j]);
		}

		if(!goodheader(fp,&linnum,"Tanks\n")) goto error;
		for(j = 0 ; j < numtank ; j++) {
			if(!goodheader(fp,&linnum,"Percent filled      ")) goto error;
			fscanf(fp,"%f\n",&tankperc[i][j]);
			if(!goodheader(fp,&linnum,"Specific gravity    ")) goto error;
			fscanf(fp,"%f\n",&tankspgr[i][j]);
			n = ftell(fp);
			if(!goodheader(fp,&linnum,"Permeability        ")) {
				fseek(fp,n,0);
				tankperm[i][j] = 1.0;
			}
			else {
				fscanf(fp,"%f\n",&tankperm[i][j]);
			}
		}
	}

	/*	Extra details for 1997+ versions		*/

	if(goodheader(fp,&linnum,"Load Sizes\n")) {
		for(j = 0 ; j < numload ; j++) {
			if(!goodheader(fp,&linnum,"X-Size              ")) goto error;
			fscanf(fp,"%f\n",&lload[j]);
			if(!goodheader(fp,&linnum,"Y-Size              ")) goto error;
			fscanf(fp,"%f\n",&wload[j]);
			if(!goodheader(fp,&linnum,"Z-Size              ")) goto error;
			fscanf(fp,"%f\n",&hload[j]);
		}
	}
	else {
		for(j = 0 ; j < numload ; j++) {
			lload[j] = 1.0;
			wload[j] = 1.0;
			hload[j] = 1.0;
		}
	}

	fclose(fp);
	return;

error:
	sprintf(text,"$Error in project file at line %d",linnum);
	abortp(text);
}

int goodheader(FILE *fp,int *line,const char *msg)
{
	char leader[22];
	if(fp != NULL) {
		if(fgets(leader,21,fp) != NULL) {
			(*line)++;
			return(strcmp((char *) leader,msg) == 0);
		}
	}
	return 0;
}

void new_project()
{
	int i,j;
	char *title = "START NEW PROJECT";

	numstatvar = 1;
	strcpy(vartext,"design");
	strcpy(hullname,"hullform.hud");
	strcpy(vessel,"New Design");
	vartype[0] = TEXTDATA;
	strcpy(editor,"notepad.exe");
	strcpy(tmpdirnam,".");
	strcpy(outdirnam,".");
	decimal = 3;
	numcond = 1;
	strcpy(condition[0],"Lightship");
	*state[0] = 0;
	numload = 0;
	numimme = 0;
	numtank = 0;
	for(i = 0 ; i < MAXCOND ; i++) {
		for(j = 0 ; j < MAXTANK ; j++) {
			tankperc[i][j] = 0.0;
			tankspgr[i][j] = 1.0;
			tankperm[i][j] = 1.0;
		}
	}
	if(MessageBox(hStat,
		"All data reset.\n\nDo you want to be guided\nthrough your new project?",
		title,MB_YESNO) == IDNO) return;

	(void) MessageBox(hStat,"Firstly, you must define critical files\nand directories, as well as the Vessel\nname. The hull data file name is crucial,\nbecause its tank details are needed later\nin project definition",
		title,MB_OK);
	edit_project_files();

	if(MessageBox(hStat,"Now you can add any static loads. You will\nneed to specify X-, Y- and Z-positions and\ndefault mass, plus a description of each.\n\nDo you want to continue?",
		title,MB_YESNO) == IDNO) return;
	edit_static_loads();

	if(MessageBox(hStat,"Now you can add any immersion points. You will\nneed to specify X-, Y- and Z-positions, plus a\ndescription of each.\n\nDo you want to continue?",
		title,MB_YESNO) == IDNO) return;
	edit_immersion_points();

	if(MessageBox(hStat,"All the items needed to analyse your design\nshould now be defined. Tanks definitions will\nbe retrieved from Hullform. The next step is\nto configure the mass of each load, and the\ncontents of each, for each operating condition.\n\nDo you want to do this now?",
		title,MB_YESNO) == IDNO) return;
	edit_conditions();

	if(MessageBox(hStat,"You have now completed all the steps required\nto define your project. It is a good idea to\nsave the project at this stage.\n\nDo you want to save the project now?",
		title,MB_YESNO) == IDNO) return;
	save_project(TRUE);

	if(MessageBox(hStat,"You are now in a position to use your project\ndefinitions to generate a statics analysis,\nbased on an existing template\n\nDo you want to do this?",
		title,MB_YESNO) == IDNO) return;

	if(MessageBox(hStat,"It might be worthwhile checking that your program\npreferences are properly set - for example, the\ngraphics format.\n\nDo you want to do this first?",
		title,MB_YESNO) == IDYES) edit_preferences();

	generate();
}

void save_project(int newname)
{
	FILE *fp;
	int i,j;

	if(newname || !*project) {
		if(!openfile(project,"wt",
			"Save the Current Project",
			"stability projects (*.hsp)\0*.hsp\0",
			"*.hsp",outdirnam,&fp)) {
			messagedata("Can not open file:",project);
			return;
		}
	}
	else {
		fp = fopen(project,"wt");
		if(fp == NULL) {
			message("Can't open project file for output. Is it read-only?");
			return;
		}
	}
	strchk(vessel);
	fprintf(fp,"Vessel              %s\n",vessel);
	strchk(hullname);
	fprintf(fp,"Hull name           %s\n",hullname);
	strchk(editor);
	fprintf(fp,"Editor              %s\n",editor);
	strchk(tmpdirnam);
	fprintf(fp,"Template directory  %s\n",tmpdirnam);
	strchk(outdirnam);
	fprintf(fp,"Output directory    %s\n",outdirnam);
	fprintf(fp,"Condition count     %d\n",numcond);

	fprintf(fp,"Load count          %d\n",numload);
	for(j = 0 ; j < numload ; j++) {
		strchk(loaddesc[j]);
		fprintf(fp,"%-19d %s\n",j+1,loaddesc[j]);
		fprintf(fp,"Default X-Position  %.4f\n",defxload[j]);
		fprintf(fp,"Default Y-Position  %.4f\n",defyload[j]);
		fprintf(fp,"Default Z-Position  %.4f\n",defzload[j]);
	}

	fprintf(fp,"Immersion points    %d\n",numimme);
	for(j = 0 ; j < numimme ; j++) {
		strchk(immedesc[j]);
		fprintf(fp,"Description         %s\n",immedesc[j]);
		fprintf(fp,"X-Position          %.4f\n",ximme[j]);
		fprintf(fp,"Y-Position          %.4f\n",yimme[j]);
		fprintf(fp,"Z-Position          %.4f\n",zimme[j]);
	}

	fprintf(fp,"Number of tanks     %d\n",numtank);

	for(i = 0 ; i < numcond ; i++) {
		strchk(condition[i]);
		fprintf(fp,"CONDITION           %s\n",condition[i]);
		strchk(state[i]);
		fprintf(fp,"State               %s\n",state[i]);

		fprintf(fp,"Static loads\n");
		for(j = 0 ; j < numload ; j++) {
			fprintf(fp,"X-Position          %.4f\n",xload[i][j]);
			fprintf(fp,"Y-Position          %.4f\n",yload[i][j]);
			fprintf(fp,"Z-Position          %.4f\n",zload[i][j]);
			fprintf(fp,"Mass                %.4f\n",mload[i][j]);
		}

		fprintf(fp,"Tanks\n");
		for(j = 0 ; j < numtank ; j++) {
			fprintf(fp,"Percent filled      %.4f\n",tankperc[i][j]);
			fprintf(fp,"Specific gravity    %.4f\n",tankspgr[i][j]);
			fprintf(fp,"Permeability        %.4f\n",tankperm[i][j]);
		}
	}

	fprintf(fp,"Load Sizes\n");
	for(j = 0 ; j < numload ; j++) {
		fprintf(fp,"X-Size              %.4f\n",lload[j]);
		fprintf(fp,"Y-Size              %.4f\n",wload[j]);
		fprintf(fp,"Z-Size              %.4f\n",hload[j]);
	}
	fclose(fp);
}

void edit_project_files()
{
	context_string = "HST_EDIT_FILIST";
	if(getdlg(PROJECT,
		INP_STR,vessel,
		INP_STR,hullname,
		INP_STR,editor,
		INP_STR,tmpdirnam,
		INP_STR,outdirnam,
		INP_PBF,project_directories,-1)) {
		checkdir(tmpdirnam);
		checkdir(outdirnam);
	}
}

void project_directories(int code,HWND hWndDlg)
{
	char path[MAX_PATH];
	char start[MAX_PATH];
#ifdef linux
	char *text;

	text = XmTextGetString(wEdit[code+1]);
	strncpy(start,text,MAX_PATH);
	XtFree(text);
#else
	GetDlgItemText(hWndDlg,DLGEDIT+code+1,start,MAX_PATH);
#endif
	striptopath(start);
	switch(code){
	case 0:		/* hull data file */
		if(openfile(path,"rt","Find a Hull Data file",
			"Hull data files (*.hud)\0*.hud\0","*.hud",start,NULL)) {
#ifdef linux
			XmTextSetString(wEdit[1],path);
#else
			strlwr(path);
			SetDlgItemText(hWndDlg,DLGEDIT+1,path);
#endif
		}
		break;
	case 1:		/* editor */
		if(openfile(path,"rt","Find an Editor",
#ifdef linux
			"Program files\0*\0","*",
#else
			"Program files (*.exe)\0*.exe\0","*.exe",
#endif
			start,NULL)) {
#ifdef linux
			XmTextSetString(wEdit[2],path);
#else
			strlwr(path);
			SetDlgItemText(hWndDlg,DLGEDIT+2,path);
#endif
		}
		break;
	case 2:		/* template directory */
		if(openfile(path,"rt","Find a Template Directory",
			"Text Templates (*.txt)\0*.txt\0RTF Templates\0*.rtf\0",
			"*.*",start,NULL)) {
			striptopath(path);
#ifdef linux
			XmTextSetString(wEdit[3],path);
#else
			strlwr(path);
			SetDlgItemText(hWndDlg,DLGEDIT+3,path);
#endif
		}
		break;
	case 3:		/* output directory */
		if(openfile(path,"rt","Find an Output Directory",
			"Text Files (*.txt)\0*.txt\0RTF Files\0*.rtf\0",
			"*.*",start,NULL)) {
			striptopath(path);
#ifdef linux
			XmTextSetString(wEdit[4],path);
#else
			strlwr(path);
			SetDlgItemText(hWndDlg,DLGEDIT+4,path);
#endif
		}
		break;
	}
}

void checkdir(char *dirnam)
{
	char text[200],text2[200];
	struct stat statbuf;
	char *p;

	strchk(dirnam);
	if(stat(dirnam,&statbuf) == 0) {
		if((statbuf.st_mode & S_IFDIR) == 0) {

			/*	File exists but is not a directory	*/

			sprintf(text,"%s exists already, and is not a directory",dirnam);
			message(text);
		}
		return;
	}

	/*	File does not exist. Check that it represents a real
	directory path	*/

	strcpy((char *) text,dirnam);
	p = text;
	if(*(p+1) == ':') p += 2;
	while((p = strchr(p,'\\')) != NULL) {
		*p = 0;
		if(stat(text,&statbuf) == 0) {	/* file found */
			if((statbuf.st_mode & S_IFDIR) == 0) {
				/*	Part of path exists, but is not a directory	*/
				strcat(text," exists already, and is not a directory");
				message(text);
				return;
			}
		}
		else {
			break;	/* not found - start here */
		}
		*p++ = '\\';
	}

	/*	Now create subdirectories needed	*/

	while(1) {
		if(*(text+1) == ':') break;
		sprintf(text2,"Create new directory %s?",text);
		if(MessageBox(hStat,text2,"Directory Validation",MB_YESNO)
			    != IDYES) return;
#ifdef linux
		if(mkdir((char *) text,0) != 0)
#else
			if(mkdir((char *) text) != 0)
#endif
			{
				sprintf(text2,"Can not create directory %s",text);
				message(text2);
				return;
			}
		if(p == NULL) break;
		*p++ = '\\';
		p = strchr(p,'\\');
		if(p != NULL) *p = 0;
	}
}

void preffunc(int,HWND);

void edit_preferences()
{
	int newsize = textdatasize-1;
	FILE *fp;
	int prevmaxvar = maxvar;

	context_string = "HST_PREFERS";
	if(getdlg(PREF,
		INP_RBN,&dispoption,
		INP_LOG,&rtf,
		INP_RBN,&hpgl,
		INP_INT,&newsize,
		INP_INT,&maxvar,
		INP_STR,projdirnam,
		INP_PBF,preffunc,-1)) {
		strchk(projdirnam);
		if(newsize > MAX_PATH) {
			message("Text size reduced to program limit of 512");
			newsize = MAX_PATH;
		}
		newsize++;
		memfree(vartext);
		memfree(varname);
		memfree(varvalue);
		memfree(varsize);
		memfree(vartype);
		if(	!memavail((void **) &vartext ,maxvar*newsize) ||
		    !memavail((void **) &varname ,maxvar*sizeof(*varname)) ||
		    !memavail((void **) &varvalue,maxvar*sizeof(*varvalue)) ||
		    !memavail((void **) &varsize ,maxvar*sizeof(*varsize)) ||
		    !memavail((void **) &vartype ,maxvar*sizeof(*vartype)) ) {
			maxvar = prevmaxvar;
			message("Could not increase text size - previous size retained");
			if(	!memavail((void **) &vartext ,maxvar*textdatasize) ||
			    !memavail((void **) &varname ,maxvar*sizeof(*varname)) ||
			    !memavail((void **) &varvalue,maxvar*sizeof(*varvalue)) ||
			    !memavail((void **) &varsize ,maxvar*sizeof(*varsize)) ||
			    !memavail((void **) &vartype ,maxvar*sizeof(*vartype)) )
				message("Could not reinstate old memory allocation - quit and restart program.");
		}
		else {
			textdatasize = newsize;
		}
		if((fp = fopen(statcfgp,"wt")) != NULL) {
			fprintf(fp,"%d %d %d %d %d\n",dispoption,hpgl,textdatasize,maxvar,rtf);
			fprintf(fp,"%s\n",projdirnam);
			fclose(fp);
		}
	}
}

void preffunc(int code,HWND hWndDlg)
{
	char path[MAX_PATH];
	char *p;
	if(openfile(path,"rt","Find a Project Directory",
		"Project files (*.hsp)\0*.hsp\0All files\0*.*\0",
		"*.hsp",".",NULL)) {
		p = strchr(path,0);
		while(p != path) {
			if(*--p == '\\') {
				*p = 0;
				break;
			}
		}
#ifdef linux
		XmTextSetString(wEdit[2],path);
#else
		strlwr(path);
		SetDlgItemText(hWndDlg,DLGEDIT+2,path);
#endif
	}
}

/*	Static Load manipulations		*/

void loadfunc(int code,HWND hWnd);

void edit_static_loads(void)
{
	int i;
	char extra[MAX_PATH];
	loadlist.string = extra;
	loadlist.hs_index = 0;
	for(i = 0 ; i < numload ; i++) {
		strchk(loaddesc[i]);
		loadlist.table[i] = loaddesc[i];
	}
	loadlist.table[i] = "";

	context_string = "HST_EDIT_LOADS";
	(void) getdlg(LOAD,
		INP_LBX,(void *) &loadlist,
		INP_PBF,(void *) loadfunc,-1);
}

void loadfunc(int code,HWND hWndDlg)
{
	int i,j;
	char oldstring[MAX_PATH];
#ifdef linux
	XmString xstr;
#endif

	switch(code) {
	case 0:	/* Add */
		if(numload >= MAXLOAD) {
			message("Maximum load count already reached.");
		}
		else {
			i = count / 2;
			defxload[numload] = xsect[i];
			defyload[numload] = 0.0;
			defzload[numload] = -zline[0][i];
			defmload[numload] = 0.0;
			lload[numload] = 1.0;
			wload[numload] = 1.0;
			hload[numload] = 1.0;
			context_string = "HST_EDIT_LOAD_POS";
			strchk(loaddesc[numload]);
			loaddesc[numload][40] = 0;
			if(getdlg(LOADPOS,
				INP_STR,loaddesc[numload],
				INP_REA,&defxload[numload],
				INP_REA,&defyload[numload],
				INP_REA,&defzload[numload],
				INP_REA,&defmload[numload],
				INP_REA,&lload[numload],
				INP_REA,&wload[numload],
				INP_REA,&hload[numload],
				-1)) {
				strchk(loaddesc[numload]);
				loaddesc[numload][40] = 0;
#ifdef linux
				xstr = XmStringCreateSimple(loaddesc[numload]);
				XmListAddItem(wListBox[0],xstr,0);
				XmListSelectItem(wListBox[0],xstr,0);
				XmStringFree(xstr);
#else
				SendDlgItemMessage(hWndDlg,DLGLBX+0,LB_ADDSTRING,0,(LPARAM) (LPCSTR) loaddesc[numload]);
				SendDlgItemMessage(hWndDlg,DLGLBX+0,LB_SETCURSEL,numload,0L);
#endif
				for(i = 0 ; i < numcond ; i++) {
					xload[i][numload] = defxload[numload];
					yload[i][numload] = defyload[numload];
					zload[i][numload] = defzload[numload];
					mload[i][numload] = 0.0;
				}
				numload++;
			}
		}
		break;
	case 1:	/* Delete */
		i = loadlist.hs_index;
		if(i >= 0 && i < numload) {
			if(MessageBox(hWndDlg,"Do you really want to delete\nthe highlighted load?",
				"DELETE A STATIC LOAD",MB_ICONHAND | MB_YESNO) == IDYES){
				numload--;
#ifdef linux
				XmListDeleteItemsPos(wListBox[0],1,i+1);
#else
				(void) SendDlgItemMessage(hWndDlg,DLGLBX+0,LB_DELETESTRING,(WPARAM) i,0L);
#endif
				for( ; i < numload ; i++) {
					loaddesc[i+1][40] = 0;
					strcpy(loaddesc[i],loaddesc[i+1]);
					strchk(loaddesc[i]);
					defxload[i] = defxload[i+1];
					defyload[i] = defyload[i+1];
					defzload[i] = defzload[i+1];
					defmload[i] = defmload[i+1];
					lload[i] = lload[i+1];
					wload[i] = wload[i+1];
					hload[i] = hload[i+1];
					for(j = 0 ; j < numcond ; j++) {
						xload[j][i] = xload[j][i+1];
						yload[j][i] = yload[j][i+1];
						zload[j][i] = zload[j][i+1];
						mload[j][i] = mload[j][i+1];
					}
				}
			}
		}
		break;
	case 2:	/* Edit */
		i = loadlist.hs_index;
		if(i >= 0 && i < numload) {
			context_string = "HST_EDIT_LOAD_POS";
			strchk(loaddesc[i]);
			loaddesc[i][40] = 0;
			strcpy(oldstring,loaddesc[i]);
			(void) getdlg(LOADPOS,
				INP_STR,loaddesc[i],
				INP_REA,&defxload[i],
				INP_REA,&defyload[i],
				INP_REA,&defzload[i],
				INP_REA,&defmload[i],
				INP_REA,&lload[i],
				INP_REA,&wload[i],
				INP_REA,&hload[i],
				-1);
			strchk(loaddesc[i]);
			loaddesc[i][40] = 0;
			if(strcmp(oldstring,loaddesc[i]) != 0 && hWndDlg != NULL) {
#ifdef linux
				xstr = XmStringCreateSimple(loaddesc[i]);
				i++;
				XmListReplaceItemsPos(wListBox[0],&xstr,1,i);
				XmStringFree(xstr);
#else
				(void) SendDlgItemMessage(hWndDlg,DLGLBX+0,LB_DELETESTRING,(WPARAM) i,0L);
				(void) SendDlgItemMessage(hWndDlg,DLGLBX+0,LB_INSERTSTRING,(WPARAM) i,(LPARAM) (LPCSTR) loaddesc[i]);
#endif
			}
		}
		break;
	case 3:
#ifndef linux
		SetWindowPos(hWndDlg,HWND_BOTTOM,0,0,0,0,SWP_NOMOVE | SWP_NOSIZE);
#endif
		statedit();
		break;
	}
}

/*	Immersion point manipulations		*/

void immefunc(int code,HWND hWnd);

void edit_immersion_points(void)
{
	int i;
	char extra[MAX_PATH];

	immelist.string = extra;
	immelist.hs_index = 0;
	for(i = 0 ; i < numimme ; i++) {
		strchk(immedesc[i]);
		immedesc[i][textdatasize-1] = 0;
		immelist.table[i] = immedesc[i];
	}
	immelist.table[i] = "";
	context_string = "HST_EDIT_IMMPTS";
	(void) getdlg(IMME,
		INP_LBX,(void *) &immelist,
		INP_PBF,immefunc,-1);
}

void immefunc(int code,HWND hWnd)
{
	int i;
	char oldstring[MAX_PATH];
#ifdef linux
	XmString xstr;
#endif

	switch(code) {
	case 0:	/* Add */
		if(numimme >= MAXIMME) {
			message("Maximum immersion point count already reached.");
		}
		else {
			sprintf(immedesc[numimme],"Immersion point %d",numimme+1);
			i = count / 2;
			ximme[numimme] = xsect[i];
			yimme[numimme] = yline[0][i];
			zimme[numimme] = -zline[0][i];
			context_string = "HST_EDIT_ONE_IMMPT";
			strchk(immedesc[numimme]);
			immedesc[numimme][textdatasize-1] = 0;
			if(getdlg(IMMEEDIT,
				INP_STR,immedesc[numimme],
				INP_REA,&ximme[numimme],
				INP_REA,&yimme[numimme],
				INP_REA,&zimme[numimme],-1)) {
				strchk(immedesc[numimme]);
				immedesc[numimme][textdatasize-1] = 0;
#ifdef linux
				xstr = XmStringCreateSimple(immedesc[numimme]);
				XmListAddItem(wListBox[0],xstr,0);
				XmListSelectItem(wListBox[0],xstr,0);
				XmStringFree(xstr);
#else
				SendDlgItemMessage(hWnd,DLGLBX+0,LB_ADDSTRING,0,(LPARAM) (LPCSTR) immedesc[numimme]);
				SendDlgItemMessage(hWnd,DLGLBX+0,LB_SETCURSEL,numimme,0L);
#endif
				numimme++;
			}
		}
		break;
	case 1:	/* Delete */
		i = immelist.hs_index;
		if(i >= 0 && i < numimme) {
			if(MessageBox(hWnd,"Do you really want to delete\nthe highlighted immersion point?",
				"DELETE AN IMMERSION POINT",MB_ICONHAND | MB_YESNO) == IDYES) {
				numimme--;
#ifdef linux
				XmListDeleteItemsPos(wListBox[0],1,i+1);
#else
				SendDlgItemMessage(hWnd,DLGLBX+0,LB_DELETESTRING,i,0);
#endif
				for( ; i < numimme ; i++) {
					immedesc[i+1][textdatasize-1] = 0;
					strcpy(immedesc[i],immedesc[i+1]);
					strchk(immedesc[i]);
					ximme[i] = ximme[i+1];
					yimme[i] = yimme[i+1];
					zimme[i] = zimme[i+1];
				}
			}
		}
		break;
	case 2:	/* Edit */
		i = immelist.hs_index;
		if(i >= 0 && i < numimme) {
			strchk(immedesc[i]);
			strcpy(oldstring,immedesc[i]);
			context_string = "HST_EDIT_ONE_IMMPT";
			(void) getdlg(IMMEEDIT,
				INP_STR,immedesc[i],
				INP_REA,&ximme[i],
				INP_REA,&yimme[i],
				INP_REA,&zimme[i],-1);
			strchk(immedesc[i]);
			immedesc[i][textdatasize-1] = 0;
			if(strcmp(oldstring,immedesc[i]) != 0 && hWnd != NULL) {
#ifdef linux
				xstr = XmStringCreateSimple(immedesc[i]);
				XmListDeleteItemsPos(wListBox[0],1,i+1);
				XmListAddItem(wListBox[0],xstr,i+1);
				XmStringFree(xstr);
#else
				(void) SendDlgItemMessage(hWnd,DLGLBX+0,LB_DELETESTRING,(WPARAM) i,0L);
				(void) SendDlgItemMessage(hWnd,DLGLBX+0,LB_INSERTSTRING,(WPARAM) i,(LPARAM) (LPCSTR) immedesc[i]);
#endif
			}
		}
		break;
	}
}


void edit_conditions()
{
	int i;
	void condfunc(int code,HWND hWnd);
	char newname[MAX_PATH] = "";

	condlist.hs_index = condnum;
	condlist.string = newname;
	for(i = 0 ; i < numcond ; i++) {
		strchk(condition[i]);
		condition[i][textdatasize-1] = 0;
		condlist.table[i] = condition[i];
	}
	condlist.table[numcond] = "";

	context_string = "HST_EDIT_OPERCON";
	(void) getdlg(COND,
		INP_LBX,&condlist,
		INP_PBF,(void *) condfunc,-1);
}

void condfunc(int code,HWND hWnd)
{
	int i,j;
	void condopt(int,HWND);
	char text[12];
	REAL fnumtank;
	char oldstring[MAX_PATH];
#ifdef linux
	XmString xstr;
#endif

	condnum = condlist.hs_index;

	switch (code) {
	case 0:	/* Add */
		if(numcond >= MAXCOND) {
			message("Maximum condition count already reached.");
		}
		else {
			*condition[numcond] = 0;
			*state[numcond] = 0;
			context_string = "HST_EDIT_ADD_COND";
			if(getdlg(CONDADD,
				INP_STR,condition[numcond],
				INP_STR,state[numcond],-1)) {
				strchk(condition[numcond]);
				strchk(state[numcond]);
				condition[numcond][textdatasize-1] = 0;
				state[numcond][textdatasize-1] = 0;

				/*	Initialise load and tank status			*/

				for(i = 0 ; i < numload ; i++) {
					xload[numcond][i] = defxload[i];
					yload[numcond][i] = defyload[i];
					zload[numcond][i] = defzload[i];
					mload[numcond][i] = defmload[i];
				}
				for(j = 0 ; j < numtank ; j++) {
					tankperc[numcond][j] = 0.0;
					tankspgr[numcond][j] = 1.0;
					tankperm[numcond][j] = 1.0;
				}
#ifdef linux
				xstr = XmStringCreateSimple(condition[numcond]);
				XmListAddItem(wListBox[0],xstr,0);
				XmListSelectItem(wListBox[0],xstr,0);
				XmStringFree(xstr);
#else
				SendDlgItemMessage(hWnd,DLGLBX+0,LB_ADDSTRING,0,(LPARAM) (LPCSTR) condition[numcond]);
				SendDlgItemMessage(hWnd,DLGLBX+0,LB_SETCURSEL,numcond,0L);
#endif
				numcond++;
				*condition[numcond] = 0;
				*state[numcond] = 0;
			}
		}
		break;

	case 1:	/* Delete condition */
		if(condnum < 0) {
			message("No hull condition selected");
		}
		else if(MessageBox(hWnd,"Are you sure you want to remove this hull condition?",
				"DELETE A COMPLETE HULL CONDITION",MB_ICONHAND | MB_YESNO) == IDYES) {
				numcond--;
				for(i = condnum ; i < numcond ; i++) {
					strcpy(condition[i],condition[i+1]);
					strcpy(state[i],state[i+1]);
					strchk(condition[i]);
					strchk(state[i]);
					for(j = 0 ; j < numload ; j++) {
						xload[i][j] = xload[i+1][j];
						yload[i][j] = yload[i+1][j];
						zload[i][j] = zload[i+1][j];
						mload[i][j] = mload[i+1][j];
					}
					for(j = 0 ; j < numtank ; j++) {
						tankperc[i][j] = tankperc[i+1][j];
						tankspgr[i][j] = tankspgr[i+1][j];
						tankperm[i][j] = tankperm[i+1][j];
					}
				}
				*(condlist.table[numcond]) = 0;
#ifdef linux
				XmListDeletePos(wListBox[0],condnum+1);
#else
				SendDlgItemMessage(hWnd,DLGLBX+0,LB_DELETESTRING,condnum,0l);
#endif
			}
		break;

	case 2:		/*	Edit condition	*/

		/*	Build the load point list		*/

		loadlist.hs_index = 0;
		for(i = 0 ; i < numload ; i++) {
			strchk(loaddesc[i]);
			loadlist.table[i] = loaddesc[i];
		}
		loadlist.table[i] = "";

		/*	Get tank descriptions and build tank list from them	*/

		if(!opened) {
			strcpy(statline,"open ");
			strcat(statline,hullname);
			if(!sendrecv(statline,NULL,NULL,NULL)) {
				message("Hull data file could not be accessed.\nNo tank data available");
			}
		}

		tanklist.hs_index = 0;
		(void) sendrecv("get","TANKCT",text,&fnumtank);
		(void) findvalue("tankds",&j);
		numtank = fnumtank;
		if(j == numstatvar-1) {	/* not predefined */
			varsize[j] = numtank;
			numstatvar += numtank-1;
		}
		for(i = 0 ; i < numtank ; i++) {
			sprintf(text,"tankds[%d]",i+1);
			(void) sendrecv("get",text,vartext+textdatasize*(j+i),&fnumtank);
			tanklist.table[i] = vartext+textdatasize*(j+i);
		}
		tanklist.table[i] = "";

		strchk(condition[condnum]);
		condition[condnum][textdatasize-1] = 0;
		strcpy(oldstring,condition[condnum]);
		context_string = "HST_EDIT_COND";
		(void) getdlg(CONDEDIT,
			INP_STR,(void *) condition[condnum],
			INP_STR,(void *) state[condnum],/* condnum set before condfunc called */
			INP_LBX,(void *) &loadlist,
			INP_LBX,(void *) &tanklist,
			INP_PBF,(void *) condopt,-1);
		strchk(condition[condnum]);
		condition[condnum][textdatasize-1] = 0;
		if(strcmp(oldstring,condition[condnum]) != 0) {
#ifdef linux
			xstr = XmStringCreateSimple(condition[condnum]);
			XmListDeleteItemsPos(wListBox[0],1,condnum+1);
			XmListAddItem(wListBox[0],xstr,condnum+1);
			XmStringFree(xstr);
#else
			(void) SendDlgItemMessage(hWnd,DLGLBX+0,LB_DELETESTRING,(WPARAM) condnum,0L);
			(void) SendDlgItemMessage(hWnd,DLGLBX+0,LB_INSERTSTRING,(WPARAM) condnum,(LPARAM) (LPCSTR) condition[condnum]);
#endif
		}
		break;
	}
}

void condopt(int code,HWND hWnd)
{
	int loadnum = loadlist.hs_index;
	int tanknum = tanklist.hs_index;
	int i;

	if(condnum < 0 || condnum >= numcond) {
		message("No valid load selected");
		return;
	}

	switch(code) {
	case 0:		/* Add static load */
	case 1:		/* Remove static load */
		loadlist.hs_index = loadnum;
		loadfunc(code,hWnd);
		break;

	case 2:		/* Edit Static loads */
		if(loadnum < 0 || loadnum >= numload) {
			message("No valid load selected");
		}
		else {
			context_string = "HST_EDIT_CONDLOAD";
			strchk(loaddesc[loadnum]);
			loaddesc[loadnum][40] = 0;
			(void) getdlg(CONDLOAD,
				0,(void *) loaddesc[loadnum],
				INP_REA,(void *) &xload[condnum][loadnum],
				INP_REA,(void *) &yload[condnum][loadnum],
				INP_REA,(void *) &zload[condnum][loadnum],
				INP_REA,(void *) &mload[condnum][loadnum],
				-1);
		}
		break;

	case 3:		/* tank status for this condition	*/
		(void) findvalue("tankds",&i);
		if(tanknum < 0 || tanknum >= numtank) {
			message("No valid tank selected");
		}
		else if(i > 0 && i < numstatvar) {
			context_string = "HST_EDIT_CONDTANK";
			(void) getdlg(CONDTANK,
				0,vartext+textdatasize*(i+tanknum),
				INP_REA,&tankperc[condnum][tanknum],
				INP_REA,&tankspgr[condnum][tanknum],
				INP_REA,&tankperm[condnum][tanknum],
				-1);
		}
		break;
	}
}

/************************************************************************/
/*									*/
/*		This is where all the work happens			*/
/*									*/
/************************************************************************/

void listbutton(int code, HWND hWnd)
{
	char *format = rtf ? "Press for txt" : "Press for rtf";
#ifdef linux
	XmString xstr;
#endif
	switch(code) {
	case 0:	/* toggle rtf / text */
#ifdef linux
		xstr = XmStringCreateSimple(format);
		XtVaSetValues(wPushButton[0],XmNlabelString,xstr,NULL);
		XmStringFree(xstr);
#else
		SetDlgItemText(hWnd,DLGPBF+0,format);
#endif
		rtf = !rtf;
		break;
	}
}

#ifdef linux

void FileList(Widget w,char *directory,char *ext)
{
	char found[MAX_PATH];
	char EXT[20];
	struct stat statbuf;
	struct dirent *entry;
	DIR *dir;
	XmString xstr;
	char *p,*P;
	int i,n;

	XmListDeleteAllItems(w);
	dir = opendir(directory);
	if(dir == NULL) {
		return;
	}
	else {
		p = ext;
		P = EXT;
		do {
			*P++ = toupper(*p);
		}
		while(*p++ != 0);

		while ( (entry = readdir(dir)) != NULL) {
			p = strchr(entry->d_name,'.');
			if(p != NULL && (strcmp(p+1,ext) == 0 || strcmp(p+1,EXT) == 0) ) {
				sprintf(found,"%s/%s",directory,entry->d_name);
				stat(found,&statbuf);
				if( (statbuf.st_mode & __S_IFMT) != __S_IFDIR) {
					xstr = XmStringCreateSimple(entry->d_name);
					XmListAddItem(w,xstr,0);
					XmStringFree(xstr);
				}
			}
		}
		closedir(dir);
	}
}
#endif

void loadboxes(HWND hWnd)
{
#ifdef linux
	XmString xstr;
	char *other = rtf ? "Press for txt" : "Press for rtf";
	char *format = rtf ? "rtf" : "txt";

	FileList(wComboBox[0],tmpdirnam,format);
	FileList(wComboBox[1],outdirnam,format);
	xstr = XmStringCreateSimple(other);
	XtVaSetValues(wPushButton[0],XmNlabelString,xstr,NULL);
	XmStringFree(xstr);
#else
	char text[MAX_PATH];
	strcpy(text,tmpdirnam);
	strcat(text,rtf ? "\\*.rtf" : "\\*.txt");
	DirListComboBox(hWnd,DLGLBX +0,text);
	strcpy(text,outdirnam);
	strcat(text,rtf ? "\\*.rtf" : "\\*.txt");
	DirListComboBox(hWnd,DLGLBX +1,text);
	SetDlgItemText(hWnd,DLGPBF+1,rtf ? "Press for txt" : "Press for rtf");
#endif
}

void generate()
{
	char file[MAX_PATH];
	char *p;
	REAL fnumtank;
	extern int rtf;

#ifdef linux
	char *dirch = "/";
#else
	char *dirch = "\\";
#endif

#ifdef DEMO
	extern int allow_hullstat;
#endif

	struct {
		int index;		/* index of result in table */
		char *string;		/* pointer to result */
		char *table[1];		/* table of pointers to strings for listbox, null string terminator */
	}
	template,output;

	context_string = "HST_RUN_TEMPLATE";
	template.table[0] = "";
	output.table[0] = "";
	output.index = 0;
	template.string = templat;
	output.string = outfile;
	template.index = 0;
	if(!getdlg(TMPLOUTP,
		INP_CBX,(void *) &template,
		INP_CBX,(void *) &output,
		INP_PBF,listbutton,
		INP_INI,loadboxes,
		-1)) return;

	p = strchr(tmpdirnam,0);
	if(*--p == *dirch) *p = 0;
	p = strchr(outdirnam,0);
	if(*--p == *dirch) *p = 0;

	if(filecopy(tmpdirnam,templat,outdirnam,templat) == 0) {
		message("Could not copy template to output directory");
		return;
	}

	if(filecopy(projdirnam,project,outdirnam,project) == 0) {
		message("Could not copy project file to output directory");
		return;
	}

	*statbuffer = 0;
	brlev = 0;
	debug = 0;
	condindex = 0;
	immeindex = 0;
	loadindex = 0;
	tankindex = 0;
	blocklevel = 0;
	blockstate[0] = 0;

	numstatvar = 1;
	strcpy(varname[0],"hull");
	strncpy(vartext,vessel,textdatasize-1);
	*(vartext+textdatasize) = 0;

	outlen = 0;
	SetCondition();
	SetImmersionPoint();
	SetLoad();
	SetTank();

	strcpy(file,tmpdirnam);
	strcat(file,dirch);
	strcat(file,templat);
	tmpl = fopen(file,"rt");
	if(tmpl == NULL) {
		abortp("$Can not open template");
		return;
	}

	if(fgets(statbuffer,BUFFERSIZE,tmpl) == NULL) {
		abortp("$Template file is empty");
		return;
	}
	rtf = strncmp(statbuffer,"{\\rtf",5) == 0;
	fseek(tmpl,0,0);

	*statbuffer = 0;
	bufp = statbuffer;
	lnp = statbuffer;
	is_command = 0;
	was_command = 0;

	strcpy(file,outdirnam);
	strcat(file,dirch);
	strcat(file,outfile);
	if((fout = fopen(file,"wt")) == NULL) {
		messagedata("Can not open output file:",outfile);
		linelength = 0;
		fclose(tmpl);
		tmpl = NULL;
		return;
	}
	*statline = 0;
	*statbuffer = 0;
#ifdef linux
	XmListDeleteAllItems(wListBox[0]);
#else
	SendDlgItemMessage(hStat,DLGLBX+0,LB_RESETCONTENT,0,0l);
#endif
	listcount = 0;

	if(!sendrecv("open",hullname,file,&fnumtank)) {
		abortp("$Hull data file not found");
	}
	else {
#ifdef DEMO
		if(!allow_hullstat) {
			message("You may only process the provided\nhull data file hst-demo in the\ndemonstration version");
			abortp("");
			return;
		}
#endif
		(void) sendrecv("get","TANKCT",file,&fnumtank);
		numtank = fnumtank;
		lnp = statline;
	}
}

/************************************************************************/
/*	Stage 3 - Once DDE conversation is initiated, send required	*/
/*	commands (or requests)						*/

void send_command()
{
	char *sp,*ep,*pp;
	char *tp;
	int i,j,size,hs_index;
	int ix,iy,xstr,ystr,n;
	char c;
	REAL value,xval,yval;
	char subst[200];
	int put,dim = 0,string,set = 0;
	int state;
	int base_index;
	int success;
	int ind[3];
	char strindx[3][MAX_PATH];
	struct tm *timep;
	time_t timeval;

	update_func = NULL;

	/*	Read to next command, or request implicit in contents		*/
	/*	of the template file						*/
	/*									*/
	/*	Commands may be of forms such as				*/
	/*									*/
	/*	#get lcg							*/
	/*	#perform balance						*/
	/*									*/
	/*	At this point in the program, the string to be processed is	*/
	/*	addressed by pointer "lnp"					*/

	state = blockstate[blocklevel];
	if(tmpl == NULL) return;

	if((sp = strchr(lnp,'#')) != NULL) {	/* start pointer for any command line	*/
		lnp = strchr(lnp,'\n');
		if(lnp != NULL) *lnp = 0;
		striprtf(sp);

		/************************************************************************/
		/*	We have found a command						*/

		strcpy(statline,sp);
		showtext(statline);
		lnp = statline;

		/*	convert to lower case	*/

		sp = ++lnp;	/* lnp points to first character after '#' */
		pp = lnp;
		string = 0;
		while(*pp) {
			if(*pp == '"') {
				string = 1;
				*sp++ = *pp++;
				while(*pp != '"' && *pp) {
					if(*pp == '\\') {
						*sp++ = *pp++;
						if(!*pp) break;
					}
					*sp++ = *pp++;
				}
				if(!*pp) {
					abortp("$Missing end to text string");
					return;
				}
			}
			*sp++ = (char) tolower(*pp);
			pp++;
		}
		*sp = 0;

		sp = lnp;
		while(*sp && !isspace(*sp)) sp++; /* move to end of command verb */
		*sp++ = 0;
		while(isspace(*sp)) sp++;
		if(strlen(sp) > sizeof(reqpar)) {
			sprintf(subst,"Buffer overflow - required space %d.\nPlease contact Blue Peter Marine Systems\nwith this information.",strlen(sp));
			message(subst);
		}
		strcpy(reqpar,sp);

		/*	A blank command converts to 'set'	*/

		if(*lnp == 0) {
			pp = lnp+1;
			lnp = "set";
		}
		else {
			pp = lnp+4;
		}

		if(*lnp == '!' || same(lnp,"rem")) {

			;	/* do nothing - comment */

			/*	Conditional commands	*/

		}
		else if(same(lnp,"if")) {
			if(blocklevel == 19) {
				abortp("$'If' level too great at above line.");
				return;
			}
			else if(state >= 0) {
				parse(sp);	/* parse the rest */
				blockstate[++blocklevel] = eval(par,op,level,&i) > 0.0 &&
				    i == NUMBER ? IFBLOCK : -IFBLOCK;
			}
			else {
				blockstate[++blocklevel] = NOMOREIFS;
			}
		}
		else if(same(lnp,"elseif")) {
			parse(sp);	/* parse the rest */
			if(state == NOMOREIFS) {

			}
			else if(state == IFBLOCK) {
				blockstate[blocklevel] = NOMOREIFS;
			}
			else if(state == -IFBLOCK) {
				if(eval(par,op,level,&i) > 0.0) {
					blockstate[blocklevel] = IFBLOCK;
				}
			}
			else {
				abortp("$Found 'elseif' without a matching 'if'");
				return;
			}
		}
		else if(same(lnp,"else")) {
			if(state == NOMOREIFS) {

			}
			else if(state == -IFBLOCK) {
				blockstate[blocklevel] = IFBLOCK;
			}
			else if(state == IFBLOCK) {
				blockstate[blocklevel] = NOMOREIFS;
			}
			else {
				abortp("$Found 'else' without matching 'if'");
				return;
			}
		}
		else if(same(lnp,"endif")) {
			if(blocklevel <= 0) {
				abortp("$'Endif' without 'if' expression");
				return;
			}
			else if(abs(state) != IFBLOCK && state != NOMOREIFS) {
				abortp("$Found 'else' without matching 'if'");
				return;
			}
			blockstate[blocklevel--] = 0;

			/*	Looping commands	*/

		}
		else if(same(lnp,"loopwhile")) {
			parse(sp);	/* parse the rest */
			blockstate[++blocklevel] = state >= 0 &&
			    eval(par,op,level,&i) != 0.0 && i == NUMBER ?
			LOOPWHILE : -LOOPWHILE;
			blockstart[blocklevel] = whileloc;

		}
		else if(same(lnp,"loopuntil")) {
			parse(sp);	/* parse the rest */
			blockstate[++blocklevel] = state < 0 ||
			    eval(par,op,level,&i) != 0.0 && i == NUMBER ?
			-LOOPUNTIL : LOOPUNTIL;
			blockstart[blocklevel] = whileloc;

		}
		else if(same(lnp,"loop")) {
			if(blocklevel >= 19) {
				abortp("$Repeat loops nested too deeply");
				return;
			}
			else {
				blockstate[++blocklevel] = state >= 0 ? LOOP : -LOOP;
				blockstart[blocklevel] = fileloc;
			}

		}
		else if(same(lnp,"while")) {
			if(abs(state) != LOOP) {
				abortp("$Found 'while' without a matching 'loop'");
				return;
			}
			parse(sp);	/* parse the rest */
			if(state >= 0 && eval(par,op,level,&i) != 0.0 && i == NUMBER) {
				fseek(tmpl,blockstart[blocklevel],0);
				*statbuffer = 0;	/* empty the input statbuffer */
			}
			else {
				blockstate[blocklevel--] = 0;
			}

		}
		else if(same(lnp,"until")) {
			if(abs(state) != LOOP) {
				abortp("$Found 'until' without a matching 'loop'");
				return;
			}
			parse(sp);	/* parse the rest */
			if(state >= 0 && eval(par,op,level,&i) == 0.0 && i == NUMBER) {
				fseek(tmpl,blockstart[blocklevel],0);
				*statbuffer = 0;	/* empty the input statbuffer */
			}
			else {
				blockstate[blocklevel--] = 0;
			}

		}
		else if(same(lnp,"endwhile")) {
			if(state == LOOPWHILE) {
				fseek(tmpl,blockstart[blocklevel],0);
				blockstate[blocklevel--] = 0;
				*statbuffer = 0;
			}
			else if(state != -LOOPWHILE) {
				abortp("$Found 'endwhile' without a matching 'loopwhile'");
				return;
			}
			else {
				blockstate[blocklevel--] = 0;
			}

		}
		else if(same(lnp,"enduntil")) {
			if(state == LOOPUNTIL) {
				fseek(tmpl,blockstart[blocklevel],0);
				blockstate[blocklevel--] = 0;
				*statbuffer = 0;
			}
			else if(state != -LOOPUNTIL) {
				abortp("$Found 'enduntil' without a matching 'loopuntil'");
				return;
			}
			else {
				blockstate[blocklevel--] = 0;
			}

			/*	Loops on internal variables	*/

		}
		else if(same(lnp,"foreach")) {
			if((bufp = strchr(sp,' ')) != NULL) *bufp = 0;
			if(blocklevel >= 19) {
				abortp("$Repeat loops nested too deeply");
				return;
			}
			else if(same(sp,"condition")) {
				blockstate[++blocklevel] = state >= 0 && condindex < numcond ?
				FORALLCOND : -FORALLCOND;
				condindex = 0;
				immeindex = 0;
				loadindex = 0;
				tankindex = 0;
				SetCondition();
				SetImmersionPoint();
				SetLoad();
				SetTank();
			}
			else if(same(sp,"immersionpoint")) {
				blockstate[++blocklevel] = state >= 0 && immeindex < numimme ?
				FORALLIMME : -FORALLIMME;
				immeindex = 0;
				SetImmersionPoint();
			}
			else if(same(sp,"load")) {
				blockstate[++blocklevel] = state >= 0 && loadindex < numload ?
				FORALLLOAD : -FORALLLOAD;
				loadindex = 0;
				SetLoad();
			}
			else if(same(sp,"tank")) {
				blockstate[++blocklevel] = state >= 0 && tankindex < numtank ?
				FORALLTANK : -FORALLTANK;
				tankindex = 0;
				SetTank();
			}
			else {
				abortp("$Invalid parameter to 'foreach' loop");
				return;
			}
			blockstart[blocklevel] = fileloc;

		}
		else if(same(lnp,"next")) {
			striprtf(sp);
			if((bufp = strchr(sp,' ')) != NULL) *bufp = 0;
			if(abs(state) == FORALLCOND && same(sp,"condition")) {
				condindex++;
				immeindex = 0;
				loadindex = 0;
				tankindex = 0;
				if(condindex < numcond && state >= 0) {
					fseek(tmpl,blockstart[blocklevel],0);
					*statbuffer = 0;
				}
				else {
					condindex = 0;
					blockstate[blocklevel--] = 0;
				}
				SetCondition();
				SetImmersionPoint();
				SetLoad();
			}
			else if(abs(state) == FORALLIMME && same(sp,"immersionpoint")) {
				immeindex++;
				if(immeindex < numimme && state >= 0) {
					fseek(tmpl,blockstart[blocklevel],0);
					*statbuffer = 0;
				}
				else {
					immeindex = 0;
					blockstate[blocklevel--] = 0;
				}
				SetImmersionPoint();
			}
			else if(abs(state) == FORALLLOAD && same(sp,"load")) {
				loadindex++;
				if(loadindex < numload && state >= 0) {
					fseek(tmpl,blockstart[blocklevel],0);
					*statbuffer = 0;
				}
				else {
					loadindex = 0;
					blockstate[blocklevel--] = 0;
				}
				SetLoad();
			}
			else if(abs(state) == FORALLTANK && same(sp,"tank")) {
				tankindex++;
				if(tankindex < numtank && state >= 0) {
					fseek(tmpl,blockstart[blocklevel],0);
					*statbuffer = 0;
				}
				else {
					tankindex = 0;
					blockstate[blocklevel--] = 0;
				}
				SetTank();
			}
			else {
				sprintf(subst,"$Request next %s, state was '%s'",
					sp,statedesc[state+FORALLTANK]);
				abortp(subst);
				return;
			}

			/*	Request a value of a parameter					*/
			/*									*/
			/*	The parameter may be a string or numeric			*/

		}
		else if(state < 0) {

			;	/* do nothing - logical condition failed */

		}
		else if(same(lnp,"get")) {
			pp = sp;
			while(isalnum(*pp)) pp++;
			i = *pp;			/* save terminating character	*/
			*pp = 0;
			(void) findvalue(sp,&table_index);
			if(table_index < 0) {
				abortp("$Above request parameter is invalid.");
				return;
			}
			else {
				if(i == '[') {
					base_index = table_index;
					if(varsize[table_index] >= 1) {
						if(sscanf(pp+1,"%d",&i) > 0) {
							table_index += i-1;
						}
						else if((ep = strchr(pp+1,']')) != NULL) {
							*ep = 0;
							parse(pp+1);
							hs_index = eval(par,op,level,&i);
							sprintf(pp+1,"%d]",hs_index);
							table_index = base_index + hs_index-1;
						}
						else {
							abortp("$Missing ']' in expression");
							return;
						}
					}
					else {
						abortp("$Invalid subscript");
						return;
					}
					*pp = '[';
					while(*pp && *pp++ != ']') ;
					*pp = 0;
				}
			}
			if(!sendrecv(lnp,sp,vartext+textdatasize*table_index,&varvalue[table_index])) {
				abortp("$Invalid GET variable");
				return;
			}
			if(*(vartext+textdatasize*table_index) != 0) {
				vartype[table_index] = TEXTDATA;

				/*	Truncate trailing spaces	*/

				lnp = strchr(vartext+textdatasize*table_index,0);
				sp = lnp;
				while(isspace(*--lnp) && lnp != sp) *lnp = 0;
			}
			else {
				vartype[table_index] = NUMBER;
			}

			/*	Execute a function within Hullform				*/

		}
		else if(same(lnp,"perform")) {		/* DDE execute request	*/

			if(!sendrecv(sp,NULL,varname[table_index],
				&varvalue[table_index])) {
				abortp("$'Perform' command failed");
				return;
			}

		}
		else if(same(lnp,"ondebug")) {
			debug = TRUE;

		}
		else if(same(lnp,"nodebug")) {
			debug = FALSE;

		}
		else if(same(lnp,"runsummary")) {

			/*	On first command, enter run details			*/

			ep = rtf ? "\n\\par " : "\n";
			/*
			#ifdef linux
			*/
			timeval = time(NULL);
			timep = localtime(&timeval);
			fprintf(fout,
				"%sRun time and date: %02d:%02d, %d %s %d",ep,
				timep->tm_hour,timep->tm_min,
				timep->tm_mday,month[timep->tm_mon],1900+timep->tm_year);
			/*
			#else
			gettime(&timep);
			getdate(&datep);
			fprintf(fout,
			"%sRun time and date: %02d:%02d, %d %s %d",ep,
			timep.ti_hour,timep.ti_min,
			datep.da_day,month[datep.da_mon],datep.da_year);
			#endif
			*/
			fprintf(fout,"%sHull name: %s%s",ep,vessel,ep);
			filesummary("Hullform data file",hullname);
			filesummary("Hullstat project",project);
			filesummary("Hullstat template",templat);

			/*	Set the format for data written to the output file		*/

		}
		else if(same(lnp,"decimal")) {	/* set output decimal precision */

			parse(sp);
			decimal = eval(par,op,level,&i)+0.5;

			/*	#Set: Evaluate an expression and assign the result to a parameter
			#Dim: allocate an array
			#Put: transmit a value to Hullform
			*/

		}
		else if((put = same(lnp,"put")) != 0 || (set = same(lnp,"set")) != 0 ||
			    (dim = same(lnp,"dim")) != 0) {
			while(isspace(*pp)) pp++;	/* point to start of variable to receive value of expression	*/
			strcpy(subst,pp);
			if((sp = strchr(subst,'=')) != NULL) {
				*sp = 0;
				bufp = ++sp;
			}
			sp = pp;			/* save the start of the name */
			while(isalnum(*pp)) pp++;	/* move to its end */
			ep = pp;			/* points beyond end */
			while(isspace(*pp)) pp++;	/* move across any spaces */
			c = *pp;		/*  save next character (e.g., '=' or '[' */
			*ep = 0;			/* null-terminate the name */
			i = findvalue(sp,&hs_index);/* find the variable - "hs_index" in table */
			if(c == '[') {		/* array assignment */
				ep = ++pp;		/* point beyond where bracket was */
				while(*++pp && *pp != ']') ;
				if(*pp == 0) {
					abortp("$Missing end to subscript bracket");
					return;
				}
				else {

					/*	Evaluate the subscript expression	*/

					*pp = 0;
					parse(ep);
					size = eval(par,op,level,&i)+0.5;
					if(dim) {
						if(hs_index >= numstatvar-1) {
							varsize[hs_index] = size;
							vartype[hs_index] = NUMBER;
							numstatvar += size-1;
						}
						else if(size != varsize[hs_index]) {
							if(size == 0)
								abortp("$Size is undefined variable");
							else
							    abortp("$Variable already declared");
							return;
						}
					}
					else if(put) {	/* write back the hs_index itself */
						sprintf(ep-1,"[%d]",size--);
					}
					else {
						size--;
					}
				}
				pp++;
				while(*pp) {
					if(*pp++ == '=') break;
				}
			}
			else if(dim) {
				abortp("$Missing '[' in #DIM line");
				return;
			}
			else {
				size = 0;
				if(c == '=') {
					while(isspace(*bufp)) bufp++;
				}
			}

			if(string) {
				hs_index += size;
				strconv(bufp,vartext+textdatasize*hs_index);
				vartype[hs_index] = TEXTDATA;
				varvalue[hs_index] = 0.0;
			}
			else if(set) {
				parse(bufp);	/* parse the rest */
				hs_index += size;
				varvalue[hs_index] = eval(par,op,level,&vartype[hs_index]);
				if(vartype[hs_index] != NUMBER) {
					if(vartype[hs_index] == TEXTDATA) {
						abortp("$Numeric expression involved string variable");
					}
					else if(vartype[hs_index] == UNDEF) {
						sprintf(subst,"$Undefined variable: %s",varname[hs_index]);
						abortp(subst);
					}
					return;
				}
				if(debug) {
					sprintf(statline,"%s = %f",subst,varvalue[hs_index]);
					if(MessageBox(hShow,statline,"DEBUG TRACE",
						MB_OKCANCEL) == IDCANCEL) debug = FALSE;
				}
			}
			else if(put) {
				hs_index += size;
				if(!sendrecv(lnp,sp,vartext+textdatasize*hs_index,
					&varvalue[hs_index])) {
					abortp("$Syntax error - use Preferences, Show All if the error line is not shown");
					return;
				}
			}

		}
		else if(same(lnp,"ask")) {
			size = parse(sp);
			for(i = 1, hs_index = 0 ; i < size ; i += 2, hs_index++) {
				bufp = strchr(par[i-1]+1,'"');
				if(bufp != NULL) *bufp = 0;
				(void) findvalue(par[i],&j);
				ind[hs_index] = j;
				if(vartype[j] == NUMBER) {
					sprintf((char *) strindx[hs_index],"%.4f",varvalue[j]);
					sp = strchr(strindx[hs_index],0);
					while(*--sp == '0') ;
					if(*sp != '.') sp++;
					*sp = 0;
				}
				else {
					strcpy(strindx[hs_index],vartext + textdatasize*j);
				}
			}

			switch(size) {
			case 2:	/* one parameter */
				success = getdlg(ONEVAL,
					0,par[0]+1,
					INP_STR,strindx[0],-1);
				break;
			case 4:
				success = getdlg(TWOVAL,
					0,par[0]+1,
					INP_STR,strindx[0],
					0,par[2]+1,
					INP_STR,strindx[1],-1);
				break;
			case 6:
				success = getdlg(THREEVAL,
					0,par[0]+1,
					INP_STR,strindx[0],
					0,par[2]+1,
					INP_STR,strindx[1],
					0,par[4]+1,
					INP_STR,strindx[2],-1);
				break;
			default:
				abortp("$Incorrect argument count to 'ask' command - must be 2, 4 or 6");
				return;
			}

			if(!success) {
				abortp("$Abort at 'ask' dialog");
				return;
			}

			/*	Always write input values to the output file, for QA	*/

			if(rtf) {
				fputs("\n\\par Interactively Defined:\n",fout);
			}
			else {
				fputs("\nInteractively Defined:\n",fout);
			}

			for(i = 1, hs_index = 0 ; i < size ; i += 2, hs_index++) {
				j = ind[hs_index];
				if(rtf) fputs("\\par ",fout);
				if(sscanf(strindx[hs_index],"%f",&varvalue[j]) > 0) {
					vartype[j] = NUMBER;
					fprintf(fout,"%s (internal variable %s) = %.*f\n",
						par[i-1]+1,varname[j],decimal,varvalue[j]);
				}
				else {
					vartype[j] = TEXTDATA;
					strcpy(vartext + textdatasize*j,strindx[hs_index]);
					tp = "%s (internal variable %s) = %s\n";
					fprintf(fout,tp,par[i-1]+1,varname[j],vartext[j]);
				}
			}

#ifndef linux
			SendMessage(hStat,WM_PAINT,(WPARAM) 0,(LPARAM) 0);
#endif
			getdlg_proceed = 0;

		}
		else if(same(lnp,"select")) {
			bufp = sp;
			while(!isspace(*sp) && *sp != 0) sp++;
			if(*sp) {
				*sp = 0;
				while(isspace(*(++sp))) ;
				parse(sp);
				hs_index = ((int) eval(&par[0],op,level,&i)) - 1;
				if(hs_index < 0) {
					abortp("$'select' hs_index out of range");
					return;
				}
				if(same(bufp,"condition")) {
					condindex = hs_index;
					if(condindex >= numcond) {
						abortp("$Invalid select condition hs_index");
						return;
					}
					SetCondition();
				}
				else if(same(bufp,"immersionpoint")) {
					immeindex = hs_index;
					if(immeindex >= numimme) {
						abortp("$Invalid select immersionpoint hs_index");
						return;
					}
					SetImmersionPoint();
				}
				else if(same(bufp,"load")) {
					loadindex = hs_index;
					if(loadindex >= numload) {
						abortp("$Invalid select load hs_index");
						return;
					}
					SetLoad();
				}
				else if(same(bufp,"tank")) {
					tankindex = hs_index;
					if(tankindex >= numtank) {
						abortp("$Invalid select tank hs_index");
						return;
					}
					SetLoad();
				}
				else {
					abortp("$Invalid argument to 'select' command");
					return;
				}
			}
			else {
				abortp("$Vacant expression in 'select' command");
				return;
			}

			/*	Graphics commands	*/

		}
		else if(same(lnp,"opengraphics")) {
			opengraphics();
			graphicsopen = TRUE;

		}
		else if(same(lnp,"closegraphics")) {
			if(!graphicsok()) return;
			closegraphics();
			graphicsopen = FALSE;

		}
		else if(same(lnp,"graph")) {
			if(!graphicsok()) return;
			if(parse(sp) < 11) {
				abortp("$Insufficient arguments to 'graph' command");
				return;
			}
			(void) findvalue(par[0],&ix);
			(void) findvalue(par[1],&iy);
			n = readvalue(par[2],&i);
			if(*par[3] == '"') {
				sp = subst;
				if(!strconv(par[3],sp)) return;
				par[3]++;
			}
			else {
				(void) findvalue(par[3],&xstr);
				sp = vartext+textdatasize*xstr;
			}
			statxmin = readvalue(par[4],&i);
			statxmax = readvalue(par[5],&i);
			stat_dx = readvalue(par[6],&i);
			if(*par[7] == '"') {
				ep = subst+100;
				if(!strconv(par[7],ep)) return;
				par[7]++;
			}
			else {
				(void) findvalue(par[7],&ystr);
				ep = vartext+textdatasize*ystr;
			}
			statymin = readvalue(par[8],&i);
			statymax = readvalue(par[9],&i);
			stat_dy = readvalue(par[10],&i);
			statgraph(&varvalue[ix],&varvalue[iy],n,
				sp,statxmin,statxmax,stat_dx,
				ep,statymin,statymax,stat_dy);

		}
		else if((i = same(lnp,"move")) != 0 || same(lnp,"draw")) {
			if(!graphicsok()) return;

			if(i) statnewlin();
			pp = strchr(sp,',');
			if(pp == NULL) {
				abortp("$Missing argument to 'draw' or 'move'");
				return;
			}
			*pp++ = 0;
			parse(sp);
			xval = eval(par,op,level,&i);
			parse(pp);
			yval = eval(par,op,level,&i);
			if(statxmin >= statxmax) {
				abortp("$Xmin must be less than Xmax");
				return;
			}
			if(statymin >= statymax) {
				abortp("$Ymin must be less than Ymax");
				return;
			}
			statdraw(xleft+(xval - statxmin)/(statxmax - statxmin)*(xright - xleft),
				ylower+(yval - statymin)/(statymax - statymin)*(yupper - ylower));

		}
		else if(same(lnp,"newline")) {
			if(!graphicsok()) return;
			statnewlin();

		}
		else if(same(lnp,"plottext")) {
			if(!graphicsok()) return;

			if(!strconv(sp,subst)) return;
			drawst(subst);

		}
		else if(same(lnp,"plotvalue")) {
			if(!graphicsok()) return;
			parse(sp);	/* parse the rest */
			value = eval(par,op,level,&i);
			writeprec(value,statline);
			drawst(statline);

		}
		else if(same(lnp,"colour") || same(lnp,"color")) {
			if(!graphicsok()) return;
			parse(sp);
			statcolour((int) eval(par,op,level,&i));

		}
		else if(same(lnp,"linestyle")) {
			if(!graphicsok()) return;
			parse(sp);
			statstyle((int) eval(par,op,level,&i));

		}
		else if(same(lnp,"linewidth")) {
			if(!graphicsok()) return;
			parse(sp);
			statwidth(2 * (int) eval(par,op,level,&i));

		}
		else if(same(lnp,"textsize")) {
			if(!graphicsok()) return;
			parse(sp);
			statsize((int) eval(par,op,level,&i));

		}
		else if(same(lnp,"textfont")) {
			if(!graphicsok()) return;
			if(*sp != '"') {
				(void) findvalue(sp,&i);
				if(vartype[i] != TEXTDATA) {
					abortp("$Invalid font specification");
				}
				else {
					statfont(vartext+textdatasize*i);
				}
			}
			else {
				strconv(sp,subst);
				statfont(subst);
			}

		}
		else if(same(lnp,"textstyle")) {
			if(!graphicsok()) return;

			pp = strchr(sp,',');
			if(pp == NULL) {
				abortp("$Missing argument to 'textstyle'");
				return;
			}
			*pp++ = 0;
			parse(sp);
			xval = eval(par,op,level,&i);
			parse(pp);
			yval = eval(par,op,level,&i);
			stattextstyle((int) xval,(int) yval);

		}
		else if(same(lnp,"xmin")) {
			if(!graphicsok()) return;
			parse(sp);	/* parse the rest */
			statxmin = eval(par,op,level,&i);

		}
		else if(same(lnp,"xmax")) {
			if(!graphicsok()) return;
			parse(sp);	/* parse the rest */
			statxmax = eval(par,op,level,&i);

		}
		else if(same(lnp,"ymin")) {
			if(!graphicsok()) return;
			parse(sp);	/* parse the rest */
			statymin = eval(par,op,level,&i);

		}
		else if(same(lnp,"ymax")) {
			if(!graphicsok()) return;
			parse(sp);	/* parse the rest */
			statymax = eval(par,op,level,&i);

			/*	Data display		*/

		}
		else if((n = same(lnp,"show")) != 0 || same(lnp,"print")) {
			striprtf(sp);
			strcpy(subst+1,sp);
			subst[0] = '$';
			parse(sp);	/* parse the rest */
			value = eval(par,op,level,&i);
			pp = strchr(subst,'\0');
			if(i == NUMBER)
				sprintf(pp," = %.*f",decimal,value);
			else
			    sprintf(pp," = %s",vartext+textdatasize*table_index);
			if(n) {
				strcat(subst,"\n\nPress OK to proceed, Cancel to quit");
				if(MessageBox(hWnd,subst+1,"SHOW AN INTERNAL VALUE",MB_OKCANCEL) == IDCANCEL) {
					abortp("$User requested Cancel");
					return;
				}
			}
			else {
				showtext(subst);
			}
		}
		else {
			i = debug;
			debug = TRUE;
			showtext(lnp);
			debug = i;
			abortp("$... unidentified HSL command");
			return;
		}

	}
	else if(state >= 0 && (sp = strchr(lnp,'`')) != NULL) {

		/*	The line includes an expression whose value must be substituted	*/

		if(sp != lnp) {
			*sp = 0;
			writestr(lnp);	/* write out the preceding part of the line */
		}

		lnp = ++sp;		/* point to the expression */
		if(*sp == '%') {
			sscanf(++sp,"%d",&decimal);
			while(!isspace(*sp++)) ;
			lnp = sp;
		}
		if((ep = strchr(lnp,'`')) != NULL) { /* should always be found */
			level[0] = 0;
			*ep = 0;	/* close the expression with a null */
			bufp = ep+1;
			if(rtf) striprtf(lnp);

			sp = lnp;	/* save start of assignment or expression */
			lnp = bufp;	/* point to start of rest of line */
			c = *lnp;	/* save the character - the expression
			parser will store a null here */

			/*	Parse and evaluate the expression	*/

			parse(sp);
			value = eval(par,op,level,&i);
			if(i == TEXTDATA) {
				strcpy(subst,vartext+textdatasize*table_index);
			}
			else if(i == NUMBER) {
				writeprec(value,subst);
			}
			writestr(subst);
			*lnp = c;
		}
		else {
			abortp("$Missing a terminating '`'");
		}
		return;

	}
	else if(state >= 0 && *lnp) {

		/*	No more substitutions - write rest of line		*/

		sp = lnp;
		while(isspace(*sp)) sp++;
		/*	if(!is_command && (!rtf || *sp != '\n')) writestr(lnp);	*/
		if(!is_command && *sp != '\n') writestr(lnp);
	}

	if(tmpl == NULL) return;

	was_command = is_command;
	if(*statbuffer == 0) {
		whileloc = ftell(tmpl);
		if(fgets(statline,BUFFERSIZE,tmpl) == NULL) {
			fclose(fout);
			fout = NULL;
			if(fclose(tmpl) != 0) message("Problem closing template");
			;
			tmpl = NULL;
			showtext("$** Run successfully finished **");
			return;
		}
		fileloc = ftell(tmpl);

		/*	If the line does not include an even number of back-quotes, read
		the next so that it does
		*/
		sp = statline;
		i = TRUE;	/* even back-quote flag */
		while((sp = strchr(sp,'`')) != NULL) {
			i = !i;
			sp++;
		}
		if(!i) {
			sp = strchr(statline,'\n');
			if(sp == NULL) sp = strchr(statline,0);
			if(fgets(sp,BUFFERSIZE-(int) (sp-statline),tmpl) == NULL ||
				    (lnp = strchr(sp,'`')) == NULL) {
				abortp("$Incomplete expression in back quotes");
				return;
			}
			fileloc += (int) (lnp - sp) + 1;
			fseek(tmpl,fileloc,0);
			strcpy(lnp+1,"\n");
		}
	}
	else {
		strcpy(statline,statbuffer);
		*statbuffer = 0;
	}

	/*	If the line includes a '#', it is a command line	*/

	if((sp = strchr(statline,'#')) != NULL) {

		ix = (int) (sp - statline);
		whileloc += ix;

		is_command = TRUE;

		/*	Remove preceding tabs	*/

		while(ix >= 5 && strncmp(sp-5,"\\tab ",5) == 0) {
			sp -= 5;
			ix -= 5;
			strcpy(sp,sp+5);
		}

		/*	Find start of command in file.	*/

		/*	Search back for a "\par"	*/

		pp = sp;
		while((i = strncmp(pp,"\\par ",5)) != 0 && pp != statline)
			pp--;

		if(i == 0) {	/* Have found \par */

			i = (int) (sp - pp) - 5;
			/* "i" is length of line after the \par */

			if(i > 0) {	/* strip the \par and terminate the line */
				strncpy(pp,pp+5,i);
				strcpy(pp+i,"\n");
				is_command = FALSE;
			}
			else {
				strcpy(pp,"\n");
			}
			strcpy(statbuffer,sp);	/* save the command in the buffer */

		}
		else if(sp != statline) {
			strcpy(statbuffer,sp);
			strcpy(sp,"\n");
			is_command = !rtf;
		}
	}
	else {	/* no command found in line */
		if(was_command) {
			if(strncmp(statline,"\\par ",5) == 0)
				strcpy(statline,statline+5);
		}
		is_command = FALSE;
	}
	bufp = statline;
	lnp = statline;

}

/*	Evaluate an expression. On input, "lnp" points to a sequence
of null-terminated strings, containing the constants and variables
of the expression. "op" points to an array of operators.

An expression

-4.5 * (X1 - SIN(X2))

comes in as:

lnp      = "-4.5\0X1\0SIN\0X2\0";
op[]    = {0,'*','-','(',}
level[] = {0,1,2,0}
*/

REAL eval(char *par[],char op[],int lev[],int *type)
{
	int oper;
	REAL val2,val1;
	int i,prec1,prec2,lev1,lev2;
	int base_index;
	char text[100];

#define RAD 0.01745329

	if(same("sin",par[0])) {
		val2 = eval(&par[1],&op[1],&lev[1],type);
		val1 = sin(RAD*val2);
	}
	else if(same("cos",par[0])) {
		val2 = eval(&par[1],&op[1],&lev[1],type);
		val1 = cos(RAD*val2);
	}
	else if(same("tan",par[0])) {
		val2 = eval(&par[1],&op[1],&lev[1],type);
		val1 = tan(RAD*val2);
	}
	else if(same("sqrt",par[0])) {
		val2 = eval(&par[1],&op[1],&lev[1],type);
		val1 = val2 > 0.0 ? sqrt(val2) : 0.0;
	}
	else if(same("log10",par[0])) {
		val2 = eval(&par[1],&op[1],&lev[1],type);
		val1 = val2 > 0.0 ? log10(val2) : 0.0;
	}
	else if(same("log",par[0])) {
		val2 = eval(&par[1],&op[1],&lev[1],type);
		val1 = val2 > 0.0 ? log(val2) : 0.0;
	}
	else if(same("exp",par[0])) {
		val2 = eval(&par[1],&op[1],&lev[1],type);
		val1 = exp((double) val2);
	}
	else if(same("abs",par[0])) {
		val2 = eval(&par[1],&op[1],&lev[1],type);
		val1 = abs(val2);
	}
	else if(same("atan",par[0])) {
		val2 = eval(&par[1],&op[1],&lev[1],type);
		val1 = atan(val2)/RAD;
	}
	else if(same("acos",par[0])) {
		val2 = eval(&par[1],&op[1],&lev[1],type);
		val1 = acos(val2)/RAD;
	}
	else if(same("asin",par[0])) {
		val2 = eval(&par[1],&op[1],&lev[1],type);
		val1 = asin(val2)/RAD;
	}
	else if(same("int",par[0])) {
		val1 = (int) eval(&par[1],&op[1],&lev[1],type);
	}
	else {
		val1 = readvalue(par[0],type);
		base_index = table_index;

		/*	Evaluate an operation on parameters at a higher parenthesis level or
		an operation at a higher precedence using a recursive call
		*/
		while(par[1] != NULL && *par[1]) {
			oper = op[1];
			prec1 = prec(op[1]);
			prec2 = prec(op[2]);
			lev1 = lev[1];
			lev2 = lev[2];
			if(lev2 > lev1 || lev2 == lev1 && prec2 > prec1) {
				val2 = eval(&par[1],&op[1],&lev[1],type);
			}
			else {
				val2 = readvalue(par[1],type);
			}

			switch(oper) {
			case '|':
				val1 = (REAL) (val1 != 0.0) || (val2 != 0.0);
				break;
			case '&':
				val1 = (REAL) (val1 != 0.0) && (val2 != 0.0);
				break;
			case '=':
				if(*type == TEXTDATA) {
					val1 = strcmp(vartext+textdatasize*base_index,vartext+textdatasize*table_index) == 0;
					*type = NUMBER;
				}
				else
				    val1 = (REAL) (val1 == val2);
				break;
			case '>':
				if(*type == TEXTDATA) {
					val1 = strcmp(vartext+textdatasize*base_index,vartext+textdatasize*table_index) > 0;
					*type = NUMBER;
				}
				else
				    val1 = (REAL) (val1 > val2);
				break;
			case '<':
				if(*type == TEXTDATA) {
					val1 = strcmp(vartext+textdatasize*base_index,vartext+textdatasize*table_index) < 0;
					*type = NUMBER;
				}
				else
				    val1 = (REAL) (val1 < val2);
				break;
			case '+':
				val1 += val2;
				break;
			case '-':
				val1 -= val2;
				break;
			case '*':
				val1 *= val2;
				break;
			case '/':
				if(val2 != 0.0) {
					val1 = val1 / val2;
				}
				else {
					val1 = 0.0;
					*type = UNDEF;
				}
				break;
			case '^':
				if(val1 >= 0.0) {
					val1 = pow((double) val1,(double) val2);
				}
				else {
					val1 = 0.0;
					*type = UNDEF;
				}
				break;
			case '[':
				if(*type == TEXTDATA) {
					table_index += ((int) val2) - 1;
					val1 = 0.0;
				}
				else {
					i = val2;
					if(i < 0 || i > varsize[base_index]) {
						sprintf(text,
							"Subscript out of range\nName: %s\nSize: %d\nIndex: %d",
							varname[base_index],varsize[base_index],i);
						message(text);
						abortp("");
						return 0.0;
					}
					else {
						i += base_index - 1;
						*type = vartype[i];
						if(*type == NUMBER) {
							val1 = varvalue[i];
						}
						else {
							val1 = 0.0;
							table_index = i;
						}
					}
				}
				break;
			}

			/*	Remove the operands just processed	*/

			i = 1;
			while(par[i] != NULL && *par[i]) {
				par[i] = par[i+1];
				op[i]  = op[i+1];
				lev[i] = lev[i+1];
				i++;
			}
			if(lev[1] < lev[0] || lev[1] == lev[0] &&
				    prec(op[1]) < prec(op[0])) return val1;
		}
	}

	/*	Remove the first operand when all operations are complete */

	i = 1;
	while(par[i] != NULL && *par[i]) {
		par[i] = par[i+1];
		op[i]  = op[i+1];
		lev[i] = lev[i+1];
		i++;
	}
	return val1;
}

/*	This routine reads value of variable or constant *par
in the symbol table, returning its value if it is a
scalar number, or its base hs_index if it is an array.
*/
REAL readvalue(char *par,int *typ)
{
	REAL val;
	extern int numstatvar;
	int i;
	char text[MAX_PATH];

	if(isalpha(*par)) {

		/*	Variable name - find it in table	*/

		val = findvalue(par,&i);
		*typ = vartype[i];
		if(varsize[i] > 1) val = i;

	}
	else {

		/*	Read a numeric constant			*/

		if(sscanf(par,"%f",&val) < 1) {
			sprintf(text,"$Invalid constant: %s",par);
			abortp(text);
		}
		*typ = NUMBER;
	}
	return val;
}

int prec(char op)
{
	int level;
	switch(op) {
	case '|':
		level = 1;
		break;
	case '&':
		level = 2;
		break;
	case '=':
	case '>':
	case '<':
		level = 3;
		break;
	case '+':
	case '-':
		level = 4;
		break;
	case '*':
	case '/':
		level = 5;
		break;
	case '^':
		level = 6;
		break;
	case '(':
		level = 7;
		break;
	case '[':
		level = 8;
		break;
	default:
		level = 0;
	}
	return level;
}

void messagedata(char *t1,char *t2)
{
	char mes[256];
	strcpy(mes,t1);
	strcat(mes,"\n");
	strcat(mes,t2);
	(void) MessageBox(hWnd,mes,"WARNING",MB_OK);
}

void showtext(char *str)
{
	char statline[80];
	int loop,foreach,end,next;
#ifdef linux
	XmString xstr;
#endif

	if(!debug && *str != '$') {
		switch(dispoption) {
		case 0:		/* none */
			return;
		case 1:		/* loops */
			strncpy(statline,str,9);
			statline[9] = 0;
			strlwr(statline);
			loop =	strncmp("#loop",statline,5) == 0 ||
			    strncmp("#while",statline,6) == 0 ||
			    strncmp("#until",statline,6) == 0;
			foreach = strncmp("#foreach",statline,7) == 0;
			end = strncmp("#enduntil",statline,9) == 0 ||
			    strncmp("#endwhile",statline,9) == 0;
			next = strncmp("#next",statline,5) == 0;
			if(!(loop || foreach || end || next)) return;
			break;
		case 2:		/* enacted commands */
			if(blockstate[blocklevel] < 0) return;
			break;
		case 3:		/* all */
			break;
		}
	}
	else if(*str == '$') {
		str++;
	}

#ifdef linux
	xstr =  XmStringCreateSimple(str);
	XmListAddItem(wListBox[0],xstr,0);
	XmStringFree(xstr);
	XmListSetPos(wListBox[0],max(1,listcount-5));
#else
	SendDlgItemMessage(hStat,DLGLBX+0,LB_ADDSTRING,0,(LPARAM) (LPCSTR) str);
	SendDlgItemMessage(hStat,DLGLBX+0,LB_SETCURSEL,listcount,0);
#endif
	listcount++;

}

void writetab(char *str);

#define MAXLEN 500

void writestr(char *str)
{
	int l = strlen(str);
	char *p;
	char c;

	if(fout == NULL) return;

	if(outlen + l > MAXLEN-4 && outlen > 0) {
		fputc('\n',fout);
		outlen = 0;
	}

	while(rtf && l > MAXLEN) {
		p = str+MAXLEN;
		while(!isspace(*p) && p != str && *p != '{') p--;
		if(p == str) {
			abortp("Output line is too long");
			return;
		}
		else {
			c = *p;
			*p = 0;
			writetab(str);
			fputc('\n',fout);
			str = p;
			if(isspace(c))
				str++;
			else
				*str = c;
			outlen = 0;
			l = strlen(str);
		}
	}
	writetab(str);
	if(strchr(str,'\n') == NULL) {
		if(rtf) {
			outlen += l;
		}
		else {
			p = str;
			while(*p) {
				if(*p++ == '\t')
					outlen = (outlen + 8) & 0xfff8;
				else
				    outlen++;
			}
		}
	}
	else {
		outlen = 0;
	}
}

void writetab(char *str)
{
	char *p,*q;
	int col,req;

	if(!rtf && (p = strchr(str,'\\')) != NULL && isdigit(*(p+1))) {
		*p = 0;
		col = outlen;
		q = str;
		while(q < p) {
			if(*q++ == '\t') {
				col = (col+8) & 0xfff8;
			}
			else {
				col++;
			}
		}
		req = 8*(*(p+1) - '0');
		if(req <= col && req > outlen + 2) {
			q = strchr(str,0) - (col - req + 1);
			*q = 0;
			col = req - 2;
		}
		fputs(str,fout);
		while(req > col) {
			fputc('\t',fout);
			col += 8;
		}
		p += 2;
		outlen = col;
	}
	else {
		p = str;
		outlen += strlen(str);
	}
	fputs(p,fout);
	while((p = strchr(p,'\n')) != NULL) {
		outlen = strlen(p++) - 1;
	}
}

void SetCondition()
{
	int i;
	(void) findvalue("condition",&i);
	truncate(condition[condindex]);
	strcpy(vartext+textdatasize*i,condition[condindex]);
	varvalue[i] = 0.0;
	vartype[i] = TEXTDATA;
	(void) findvalue("state",&i);
	truncate(state[condindex]);
	strcpy(vartext+textdatasize*i,state[condindex]);
	varvalue[0] = 0.0;
	vartype[i] = TEXTDATA;
}

void SetImmersionPoint()
{
	int i;
	(void) findvalue("ximme",&i);
	varvalue[i] = ximme[immeindex];
	vartype[i] = NUMBER;
	(void) findvalue("yimme",&i);
	varvalue[i] = yimme[immeindex];
	vartype[i] = NUMBER;
	(void) findvalue("zimme",&i);
	varvalue[i] = zimme[immeindex];
	vartype[i] = NUMBER;
	(void) findvalue("immedesc",&i);
	strcpy(vartext+textdatasize*i,immedesc[condindex*MAXIMME+immeindex]);
	varvalue[i] = 0.0;
	vartype[i] = TEXTDATA;
}

void SetLoad()
{
	int i;
	(void) findvalue("xload",&i);
	varvalue[i] = xload[condindex][loadindex];
	vartype[i] = NUMBER;
	(void) findvalue("yload",&i);
	varvalue[i] = yload[condindex][loadindex];
	vartype[i] = NUMBER;
	(void) findvalue("zload",&i);
	varvalue[i] = zload[condindex][loadindex];
	vartype[i] = NUMBER;
	(void) findvalue("mload",&i);
	varvalue[i] = mload[condindex][loadindex];
	vartype[i] = NUMBER;
	(void) findvalue("loaddesc",&i);
	truncate(loaddesc[condindex]);
	strcpy(vartext+textdatasize*i,loaddesc[loadindex]);
	varvalue[i] = 0.0;
	vartype[i] = TEXTDATA;
}

void SetTank()
{
	int i;

	(void) findvalue("ftank",&i);
	varvalue[i] = tankperc[condindex][tankindex];
	vartype[i] = NUMBER;

	(void) findvalue("gtank",&i);
	varvalue[i] = tankspgr[condindex][tankindex];
	vartype[i] = NUMBER;

	(void) findvalue("ptank",&i);
	varvalue[i] = tankperm[condindex][tankindex];
	vartype[i] = NUMBER;

}

/*	Insert a graph into the file using the IMPORT protocol	*/

void statgraph(REAL x[],REAL y[],INT n,
	char *xaxis,REAL statxmn,REAL statxmx,REAL delx,
	char *yaxis,REAL statymn,REAL statymx,REAL dely)
{
	REAL	xv,yv;
	REAL	statxm,statym;
	int		ndpx,ndx,nsfx,ndpy,ndy,nsfy;
	int		l;
	int		i;
	REAL	xscale,yscale;
	REAL	xvmin,xvmax,yvmin,yvmax;
	char	string[30];

	if(n <= 0) {
		abortp("$Graph request has no data to plot");
		return;
	}

	/*	FIND DATA LIMITS	*/

	xvmin = 1.0e+30;
	xvmax = -xvmin;
	yvmin = 1.0e+30;
	yvmax = xvmax;
	for(i = 0 ; i < n ; i++) {
		xv = x[i];
		yv = y[i];
		if(xvmin > xv)xvmin = xv;
		if(xvmax < xv)xvmax = xv;
		if(yvmin > yv)yvmin = yv;
		if(yvmax < yv)yvmax = yv;
	}

	if(delx <= 0.0) {
		stat_dx = axincr(0.5*(xvmax-xvmin));
	}
	else {
		stat_dx = delx;
	}

	if(statxmx <= statxmn) {
		statxmin = stat_dx*((int) (xvmin/stat_dx));
		if(xvmin < 0.0 && statxmin > xvmin + 0.001*stat_dx) statxmin -= stat_dx;
		statxmax = stat_dx*((int) (xvmax/stat_dx));
		if(xvmax > 0.0 && statxmax < xvmax - 0.001*stat_dx) statxmax += stat_dx;
	}
	else {
		statxmin = statxmn;
		statxmax = statxmx;
	}

	if(dely <= 0.0) {
		stat_dy = axincr(0.5*(yvmax-yvmin));
	}
	else {
		stat_dy = dely;
	}

	if(statymn >= statymx) {
		statymin = stat_dy*((int) (yvmin/stat_dy));
		if(yvmin < 0.0 && statymin > yvmin + 0.001*stat_dy) statymin -= stat_dy;
		statymax = stat_dy*((int) (yvmax/stat_dy));
		if(yvmax > 0.0 && statymax < yvmax - 0.001*stat_dy) statymax += stat_dy;
	}
	else {
		statymin = statymn;
		statymax = statymx;
	}

	/*	DECIMAL PLACES,NUMBER OF DIGITS ABOVE DECIMAL POINT,	*/
	/*	AND TOTAL NUMBER OF FIGURES NEEDED			*/

	ndpx = - (int) log10(stat_dx);
	if(stat_dx < 1.0) ndpx++;
	ndx = log10(max(1.0,statxmax-statxmin))+2;
	nsfx = ndx+ndpx+1;

	ndpy = - (int) log10(stat_dy);
	if(stat_dy < 1.0) ndpy++;
	ndy = log10(max(1.0,statymax-statymin))+2;
	nsfy = ndy+ndpy+1;

	xscale = (xright - xleft)/(statxmax - statxmin);
	yscale = (yupper - ylower)/(statymax - statymin);
	statcharhei = 18.0*textsize;
	statcharwid = 0.5*statcharhei;

	/*	IF ZERO Y-VALUE IS IN RANGE, DRAW X-AXIS		*/

	if(statymax > 0.0 && statymin < 0.0) {
		statmove(xleft,ylower-statymin*yscale);
		statdraw(xright,ylower-statymin*yscale);
	}

	/*	LABEL X-AXIS	*/

	for(xv = statxmin ; xv <= statxmax; xv += stat_dx) {
		statxm = xleft + (xright - xleft)*(xv - statxmin)/(statxmax - statxmin);
		statmove(statxm,ylower);
		statdraw(statxm,yupper);
		if(ndpx >= 1) {
			statmove(statxm-0.5*statcharwid*(nsfx + 1),ylower - statcharhei);
			sprintf(string,"%*.*f",nsfx,ndpx,xv);
		}
		else {
			statmove(statxm-0.5*statcharwid*(ndx + 1),ylower - statcharhei);
			sprintf(string,"%*ld",ndx,(long) (xv - (xv < 0.0 ? 0.5 : -0.5)));
		}
		drawst(string);
	}

	/*	LABEL Y-AXIS	*/

	for(yv = statymin ; yv < statymax + 0.1*stat_dy ; yv += stat_dy) {
		statym = ylower + (yupper - ylower)*(yv - statymin)/(statymax-statymin);
		statmove(xleft,statym);
		statdraw(xright,statym);
		if(ndpy >= 1) {
			statmove(xleft - statcharwid*(0.5+nsfy),statym - 0.4*statcharhei);
			sprintf(string,"%*.*f",nsfy,ndpy,yv);
		}
		else {
			statmove(xleft - statcharwid*(0.5+ndy),statym - 0.4*statcharhei);
			sprintf(string,"%*ld",ndy,(long) (yv - (yv < 0.0 ? 0.5 : -0.5)));
		}
		drawst(string);
	}

	/*	WRITE AXIS TITLES	*/

	l = strlen(xaxis);
	if(l > 0) {
		statmove((xright + xleft)*0.5-0.5*statcharwid*l,ylower - 2.0*statcharhei);
		drawst(xaxis);
	}

	l = strlen(yaxis);
	yv = (yupper + ylower + statcharhei*(l-2))*0.5;
	string[1] = 0;
	while(*yaxis) {
		statmove(0.0,yv);
		string[0] = *yaxis++;
		drawst(string);
		yv = yv - statcharhei;
	}

	/*	DRAW PLOT BOX OUTLINE	*/

	statmove(xleft ,ylower);
	statdraw(xright,ylower);
	statdraw(xright,yupper);
	statdraw(xleft ,yupper);
	statdraw(xleft ,ylower);

	/*	FINALLY, SHOW PROVIDED LINE	*/

	statnewlin();
	for(i = 0 ; i < n ; i++)
		statdraw(xleft+(x[i]-statxmin)*xscale,ylower+(y[i]-statymin)*yscale);
}

void writeprec(REAL value,char *subst)
{
	sprintf(subst,"%.*f",decimal,value);
}

void pause(REAL secs)
{
#ifdef linux
	sleep((int) secs);
#else
	MSG msg;
	extern HWND hWndMain;
	SetTimer(hWndMain,1,(UINT) (secs*1000.0),NULL);
	paused = 1;
	while(paused && GetMessage(&msg,hWndMain,0,0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
#endif
}

/****************************************************************/
/*	Graphics drivers: HPGL  and Placeable metafile		*/
/****************************************************************/

/*	Open graphics output					*/

REAL statxmx,statxmn,statymx,statymn;

void opengraphics()
{
	static int filenum = 0;
	char temp[256];
	char *p,*q;
	extern int hcpydev;
	extern int device;
	extern int hardcopy;
	void wmfpen(short int,short int,short int,short int,short int);
	void wmfsetfont(short int width,short int height,char *name,
		short int bold,short int italic,short int changerot,REAL rotval);
	int csize;

	statxmx = xmax;
	statxmn = xmin;
	statymx = ymax;
	statymn = ymin;

	filenum++;
	chdir(outdirnam);
	if(hpgl) {
#ifdef linux
		sprintf(cur_port,"%s/%d.hgl",outdirnam,filenum);
#else
		sprintf(cur_port,"%s\\%d.hgl",outdirnam,filenum);
#endif
		setup(2);
		hewlin();
	}
	else {
		setup(6);
#ifdef linux
		sprintf(cur_port,"%s/%d.wmf",outdirnam,filenum);
#else
		sprintf(cur_port,"%s\\%d.wmf",outdirnam,filenum);
#endif
		wmfin();
		wmfpen(1,0,0,0,0);
		csize = 12;
		wmfsetfont((short) csize*11,(short) csize*18,"Arial",FALSE,FALSE,FALSE,0.0);
	}
	hcpydev = device;
	hardcopy = TRUE;
	xgslope = 0.9;
	ygslope = 0.9;
	xgorigin = 0.0;
	ygorigin = 0.0;

	if(rtf) {
#ifdef linux
		sprintf(temp,"%d.wmf",filenum);
		fprintf(fout,"\n \\par (You may need to load file %s manually here)\n",temp);
#else
		p = cur_port;
		q = temp;
		while(*p) {
			if(*p == '\\') {
				*q++ = '\\';
				*q++ = '\\';
				*q++ = '\\';
			}
			*q++ = *p++;
		}
		*q = 0;
#endif
		fprintf(fout,
			"{\\field{\\*\\fldinst {\\fs24\\insrsid4663168 INCLUDEPICTURE \"%s\" \\\\* MERGEFORMATINET \\\\d }}}\n",temp);
//			{\\plain \\lang3081 IMPORT \"%s\" \\\\* mergeformat}
	}
	else {
		fprintf(fout,"NOTE: INSERT FILE %s HERE\n",cur_port);
	}
}

void closegraphics()
{
	extern int hardcopy;

	if(hpgl)
		hewlen();
	else
	    wmfen();

	xmax = statxmx;
	xmin = statxmn;
	ymax = statymx;
	ymin = statymn;
	setup(scrdev);
	(*init)();
	hardcopy = FALSE;
}

void statnewlin(void)
{
	if(hpgl)
		hewlnl();
	else
	    wmfnl();
}

REAL xstat,ystat;
int newstring;

void statmove(REAL x,REAL y)
{
	if(hpgl)
		hewlmv(x,y);
	else
	    wmfmv(x,y);
	xstat = x;
	ystat = y;
	newstring = TRUE;
}

void statdraw(REAL x,REAL y)
{
	if(hpgl)
		hewldr(x,y);
	else
	    wmfdr(x,y);
	xstat = x;
	ystat = y;
	newstring = TRUE;
}

void drawst(char *value)
{
	if(newstring) {
		statmove(xstat,ystat + statcharhei);
		newstring = FALSE;
	}

	if(hpgl)
		hewlst(value);
	else
	    wmfst(value);
}

void wmfpen(short int width,short int style,
	short int r,short int g,short int b);

void statcolour(int ncol)
{
	static short int rgb[8][3] = {
		{255,255,255},
		{255,255,0},
		{0,255,255},
		{255,0,255},
		{0,255,0},
		{0,0,255},
		{255,0,0},
		{0,0,0}
	};
	if(!hpgl) {
		if(--ncol < 0) ncol = 0;
		if(ncol > 7) ncol = 7;
		wmfpen(1,0,rgb[ncol][0],rgb[ncol][1],rgb[ncol][2]);
	}
}

void statstyle(int style)
{
	if(!hpgl && style >= 0 && style <= 4) wmfpen(-1,style,-1,-1,-1);
}

void statwidth(int width)
{
	if(!hpgl && width >= 0) wmfpen((short) width,-1,-1,-1,-1);
}

void statsize(int size)
{
	if(!hpgl) wmfsetfont((short) size*11,(short) size*18,"",FALSE,FALSE,FALSE,0.0);
	textsize = size;
}

void statfont(char *font)
{
	if(!hpgl) wmfsetfont(-1,-1,font,FALSE,FALSE,FALSE,0.0);
}

void stattextstyle(int bold,int italic)
{
	if(!hpgl) wmfsetfont(-1,-1,"",bold,italic,FALSE,0.0);
}

void truncate(char *s)
{
	if(strlen(s) >= textdatasize) *(s+textdatasize) = 0;
}

void filesummary(char *descr,char *hullname)
{
	char *sp;
	char *ep = rtf ? "\n\\par " : "\n";
	struct stat statbuf;
	struct tm *timep;

	fprintf(fout,"%s%s: ",ep,descr);

	if(rtf) {
		sp = hullname;
		while(*sp) {
			if(*sp == '\\') fputc('\\',fout);
			fputc(*sp++,fout);
		}
	}
	else {
		fputs(hullname,fout);
	}
	fputs("\n",fout);
	if(stat(hullname,&statbuf) == 0) {
		timep = localtime(&(statbuf.st_mtime));
		fprintf(fout,
			"%sRun time and date: %02d:%02d, %d %s %d",ep,
			timep->tm_hour,timep->tm_min,
			timep->tm_mday,month[timep->tm_mon],1900+timep->tm_year);
	}
}

/*	Copy file	*/

int filecopy(char *srcdir,char *srcfile,char *desdir,char *desfile)
{
#define size 50000

	unsigned int in,out,incount,outcount;
	struct stat statbuf;
	struct utimbuf times;
	char oufile[MAX_PATH];
	char infile[MAX_PATH];
	char file[MAX_PATH];
	char *buffer;
	char *p;
	char *WindowTitle = "DIRECTORY UPDATE FAILED";
#ifdef linux
	char dirch = '/';
#else
	void utime(char *path,struct utimbuf *tb);
	char dirch = '\\';
	HGLOBAL	hMem;
#endif

	/*	Ensure file names do not include their directory	*/

	while((p = strchr(srcfile,dirch)) != NULL) srcfile = p+1;
	while((p = strchr(desfile,dirch)) != NULL) desfile = p+1;

	/*	Ensure directory name does not end with a '\' or '/'	*/

	p = strchr(srcdir,0);
	if(*--p == dirch) *p = 0;
	p = strchr(desdir,0);
	if(*--p == dirch) *p = 0;

	/*	Find full input file path		*/

	strcpy(file,srcdir);
	p = strchr(file,0);
	*p++ = dirch;
	strcpy(p,srcfile);
	if(_fullpath(infile,file,MAX_PATH) == NULL) return 0;
	in =  open(infile,O_BINARY | O_RDONLY);

	/*	Find full output file path		*/

	strcpy(file,desdir);
	p = strchr(file,0);
	*p++ = dirch;
	strcpy(p,desfile);
	if(_fullpath(oufile,file,MAX_PATH) == NULL) return 0;

	/*	Refuse to overwrite a file with itself	*/

#ifndef linux
	strlwr(infile);
	strlwr(oufile);
#endif
	if(strcmp(infile,oufile) == 0) return -1;
	out = open(oufile,O_BINARY | O_RDWR | O_TRUNC | O_CREAT);

	/*	Obtain copying memory			*/

#ifdef linux
	buffer = malloc(size);
#else
	buffer = NULL;
	hMem = GlobalAlloc(GMEM_FIXED | GMEM_ZEROINIT,size);
	if(hMem != NULL) buffer = (void far *) GlobalLock(hMem);
#endif
	if(buffer != NULL) {

		/*	Perform the copy			*/

		do {
			incount  = read(in,buffer,size);
			outcount = write(out,buffer,incount);
			if(outcount < incount) {
				sprintf(oufile,"Error on output, %d input vs %d output\n",incount,outcount);
				(void) MessageBox(hWnd,oufile,WindowTitle,MB_OK);
				return 0;
			}
		}
		while(incount == size);

		/*	Release memory buffer			*/

#ifdef linux
		free(buffer);
#else
		GlobalUnlock(hMem);
		GlobalFree(hMem);
#endif
	}
	else {
		(void) MessageBox(hWnd,"Can not obtain memory for copy",WindowTitle,MB_OK);
		return 0;
	}

	/*	Set output file date and time to match input	*/

	/*
	#ifdef linux
	*/
	stat(infile,&statbuf);
	times.actime = statbuf.st_atime;
	times.modtime = statbuf.st_mtime;
	utime(oufile,&times);
	/*
	#else
	getftime(in,&ftimep);
	setftime(out,&ftimep);
	#endif
	*/
	chmod(oufile,S_IREAD|S_IWRITE);

	close(in);
	close(out);
	return 1;
}

int strconv(char *s,char *str)
{
	char *sp,*ep,*lnp;
	REAL value;
	int i;

	while(isspace(*s)) s++;

	/*	Confirm valid start to string	*/

	if(*s++ != '"') {
		abortp("$Character string required");
		return 0;
	}

	/*	process literalised " characters (and any others) */

	ep = s;
	sp = s;
	while((ep = strchr(ep,'\\')) != NULL) {
		strcpy(ep,ep+1);
		sp = ep++;
	}

	/*	Confirm valid end to string	*/

	if((ep = strchr(sp,'"')) == NULL) {
		abortp("$Unterminated character string");
		return 0;
	}
	*ep = 0;

	/*	Convert any substitution expressions	*/

	while((lnp = strchr(s,'`')) != NULL) {
		if((ep = strchr(lnp+1,'`')) == NULL) {
			abortp("$Unterminated substitution expression");
			return 0;
		}
		*ep++ = 0;/* null terminate expression, and point beyond it*/

		*lnp = 0;
		strcpy(str,s); /* add preceding part of string to output */
		str = strchr(str,0);
		s = ++lnp;
		if(rtf) {	/* strip any RTF formatting */
			while((lnp = strchr(lnp,'}')) != NULL) {
				if((sp = strchr(lnp,'{')) == NULL) {
					abortp("$Spurious '}' in expression");
					return 0;
				}
				else {
					strcpy(lnp,sp+1);
				}
			}
			lnp = s;
			while((sp = strchr(lnp,'\\')) != NULL) {
				if((sp = strchr(sp,' ')) != NULL) {
					strcpy(lnp,sp+1);
					lnp = sp;
				}
			}
		}

		/*	Parse and evaluate the expression	*/

		if(*s == '%') {
			sscanf(++s,"%d",&decimal);
			while(!isspace(*s++)) ;
		}
		parse(s);
		value = eval(par,op,level,&i);
		if(i == TEXTDATA) {
			strcat(str,vartext+textdatasize*table_index);
		}
		else if(i == NUMBER) {
			writeprec(value,strchr(str,0));
		}
		str = strchr(str,0);
		s = ep;
	}
	if(strlen(s) > textdatasize) *(s+textdatasize) = 0;
	strcpy(str,s);
	return 1;
}

void striprtf(char *s)
{
	char *p,*q;

#ifdef linux
	p = s;
	while( (p = strchr(p,'\r')) != NULL) {
		strcpy(p,p+1);
	}
#endif
	p = s;
	while( (p = strchr(p,'\n')) != NULL) {
		strcpy(p,p+1);
	}

	/*	Remove any "}{" pairs			*/

	while((p = strstr(s,"}{")) != NULL) strcpy(p,p+2);

	/*	Remove any RTF commands			*/

	while((p = strchr(s,'\\')) != NULL) {
		if((q = strchr(p,' ')) != NULL) {
			strcpy(p,++q);
			s = p;
		}
		else {
			break;
		}
	}
}

int graphicsok(void)
{
	if(!graphicsopen)
		abortp("$Graphics without opengraphics call");
	return graphicsopen;
}

void strchk(char *s)
{
	char *p;
	if(s == NULL) {
		message("Invalid pointer");
		return;
	}

	if((p = strchr(s,'\n')) != NULL) *p = 0;
	p = strchr(s,0);
	while(*p && p != s) {
		if(!isspace(*--p)) break;
		*p = 0;
	}
	if(*s == 0) {
		strcpy(s,"<NULL>");
	}
	if(strlen(s) >= textdatasize) *(s+textdatasize-1) = 0;
}

#ifndef linux

void DirList(HWND hWnd,WPARAM code,char *path,int command)
{
	struct ffblk sourcblk;
	int done;

	hWnd = GetDlgItem(hWnd,code);
	done = findfirst(path,&sourcblk,0);
	while(!done) {
		if(sourcblk.ff_attrib != FA_DIREC &&
			    *sourcblk.ff_name != '.') {
			strlwr(sourcblk.ff_name);
			SendMessage(hWnd,command,0,(LPARAM) sourcblk.ff_name);
		}
		done = findnext(&sourcblk);
	}
}

void DirListComboBox(HWND hWnd,WPARAM code,char *path)
{
	SendDlgItemMessage(hWnd,code,CB_RESETCONTENT,0,0l);
	DirList(hWnd,code,path,CB_ADDSTRING);
}

#endif

void do_balance(void);
void calctankstat(void);
extern int balanced;
void bal_all(void);
extern char *var[];
extern REAL varsign[];

int sendrecv(char *in,char *param,char *out,REAL *result)
{
	int i,j;
	REAL *varloc;
	int success = TRUE;
	FILE *fp;
	REAL sign;
	REAL reqval;
	REAL ylow,zlow;
	REAL a,hb,c,hd;
	extern void read_hullfile(FILE *f,char *hullfile,int all);
	extern int balanced;
	char *p;
	REAL areainl,areaoutl,dz2,hb1,wetw1,wwcub1,zw1;
	REAL areainr,areaoutr,hb2,wetw2,wwcub2,zw2;
	char hullname[MAX_PATH];
	extern char *var[];

	strlwr(in);
	if(param == NULL) {
		param = in;
		while(*param && !isspace(*param)) param++;
		*param++ = 0;
		while(isspace(*param)) param++;
		/*    } else {
		strlwr(param);	*/
	}

	if(strcmp(in,"balance") == 0) {

		/*	Balance the current hull		*/

		sina = sind(heel);
		cosa = cosd(heel);
		balanced = FALSE;
		do_balance();
		findrm();
		calctankstat();
		success = balanced;
	}
	else if(strcmp(in,"open") == 0 && param != NULL && *param != 0) {

		/*	Open a new hull file			*/

		if( ( fp = fopen(param,"rt") ) == NULL) {
			strcpy(hullname,dirnam);
			strcat(hullname,"\\");
			strcat(hullname,param);
			if( ( fp = fopen(hullname,"rt") ) == NULL) {
				strcat(hullname,".hud");
				if( ( fp = fopen(hullname,"rt") ) == NULL) {
					abortp("$Hull data file not found");
					return FALSE;
				}
			}
		}
		else {
			strcpy(hullname,param);
		}
		success = fp != NULL;
		if(success) {
			strupr(hullname);
			strcpy(hullfile,hullname);
			read_hullfile(fp,hullfile,TRUE);
			save_hull(MAINHULL);
			opened = TRUE;
		}

	}
	else if(strcmp(in,"evaluate") == 0) {

		/*	Evaluate hydrostatics for the current hull position	*/

		sina = sind(heel);
		cosa = cosd(heel);
		beta = tan(0.01745329*pitch);
		huldis(&disp);
		findrm();
		finish_stats();
		calctankstat();
		success = TRUE;

	}
	else if(strcmp(in,"balanceall") == 0) {
		bal_all();
		success = balanced;

	}
	else if(strcmp(in,"menu") == 0) {
		if(sscanf(param,"%d",&i) > 0) {
			ddemode = 1;
#ifdef linux
			message("Not yet implemented");
#else
			PostMessage(hWnd,WM_COMMAND,i,0L);
#endif
		}
		else {
			success = FALSE;
		}
	}
	else if(strcmp(in,"set") == 0) {

		/*	Set a one of the independent variables			*/

		sign = 1.0;
		if(strncmp(param,"water",5) == 0) {
			varloc = &wl;
			sign = -1.0;
		}
		else if(strcmp(param,"heel") == 0) {
			varloc = &heel;
		}
		else if(strcmp(param,"pitch") == 0) {
			varloc = &pitch;
		}
		else if(strncmp(param,"disp",4) == 0) {
			varloc = &disp;
		}
		else if(strcmp(param,"lcg") == 0 ||
			    strcmp(param,"xcofm") == 0) {
			varloc = &xcofm;
		}
		else if(strcmp(param,"vcg") == 0 ||
			    strcmp(param,"zcofm") == 0) {
			varloc = &zcofm;
			sign = -1.0;
		}
		else if(strcmp(param,"invert") == 0) {
			varloc = &invert;
		}
		else if(strncmp(param,"tanksg[",7) == 0 &&
			    sscanf(param+7,"%d",&i) > 0 && i > 0 && i <= ntank) {
			varloc = &fl_spgra[i-1];
		}
		else if(strncmp(param,"tankfr[",7) == 0 &&
			    sscanf(param+7,"%d",&i) > 0 && i > 0 && i <= ntank) {
			varloc = &fl_fract[i-1];
			if(fl_fixed[i] == 2) sign *= 0.01*fl_volum[i];
		}
		else if(strncmp(param,"tanklk[",7) == 0 &&
			    sscanf(param+7,"%d",&i) > 0 && i > 0 && i <= ntank) {
			fl_fixed[i] = (int) *result;
			varloc = &a;						// discard the result otherwise
		}
		else if(strncmp(param,"tankpm[",7) == 0 &&
			    sscanf(param+7,"%d",&i) > 0 && i > 0 && i <= ntank) {
			varloc = &fl_perm[i-1];
		}
		else {
			message("Invalid SET variable");
			return FALSE;
		}
		if(result != NULL) {
			*varloc = (*result) * sign;
			success = TRUE;
			balanced = 0;
		}
		else if(sscanf(par[3],"%f",varloc) < 1) {
			success = FALSE;
			message("Invalid SET value");
		}

	}
	else if(strcmp(in,"get") == 0) {
		strupr(param);
		for(i = 0 ; i < numvar+numstring ; i++) {
			if(strcmp(param,var[i]) == 0) break;
		}
		*out = 0;
		if(i >= numvar+numstring) {
			if(strcmp(param,"TANKCT") == 0) {
				*result = ntank;
			}
			else if(strcmp(param,"LINECT") == 0) {
				*result = extlin;
			}
			else if(strcmp(param,"LINEST") == 0) {
				*result = stsec[linereq];
			}
			else if(strcmp(param,"LINEEN") == 0) {
				*result = ensec[linereq];
			}
			else if(strncmp(param,"TANK",4) == 0) {
				sscanf(param+7,"%d",&i);
				i--;
				if(i < 0 || i >= ntank) {
					strcpy(out,"??invalid??");
					*result = 0.0;
					return FALSE;
				}
				else if(strncmp(param+4,"DS",2) == 0) {
					strcpy(out,tankdesc[i]);
					*result = 0.0;
					return success;
				}
				else if(strncmp(param+4,"SG",2) == 0) {
					*result = fl_spgra[i];
				}
				else if(strncmp(param+4,"FR",2) == 0) {
					*result = fl_fract[i];
				}
				else if(strncmp(param+4,"PM",2) == 0) {
					*result = fl_perm[i];
				}
				else if(strncmp(param+4,"VL",2) == 0) {
					*result = fl_volum[i];
				}
				else if(strncmp(param+4,"WL",2) == 0) {
					*result = -fl_walev[i];
				}
				else if(strncmp(param+4,"VG",2) == 0) {
					*result = -tankvcg[i];
				}
				else if(strncmp(param+4,"LG",2) == 0) {
					*result = tanklcg[i];
				}
				else if(strncmp(param+4,"PM",2) == 0) {
					*result = fl_perm[i];
				}
				else if(strncmp(param+4,"LF",2) == 0) {
					*result = tanklcf[i];
				}
				else if(strncmp(param+4,"FM",2) == 0) {
					*result = tanktfsm[i];
				}
				else if(strncmp(param+4,"MO",2) == 0) {
					*result = tankmom[i];
				}
				else if(strncmp(param+4,"MS",2) == 0) {
					*result = tankmass[i];
				}
				else if(strncmp(param+4,"LK",2) == 0) {
					*result = (REAL) (fl_fixed[i] == 0);
				}
				else {
					strcpy(out,"??invalid??");
					return FALSE;
				}

			}
			else if(strcmp(param,"SECTCT") == 0) {
				*result = count;
			}
			else if(strncmp(param,"SECT",4) == 0) {
				sscanf(param+7,"%d",&i);
				if(i < 0 || i >= count) {
					strcpy(out,"??invalid??");
					*result = 0.0;
					return FALSE;
				}
				else if(strncmp(param+4,"AR",2) == 0 || strncmp(param+4,"ZC",2) == 0){
					dz2 = wl-beta*xsect[i]-hwl[i];
					hullar(i,0,numlin,&areainl,&areaoutl,dz2, sina,cosa,&hb1,&wetw1,&wwcub1,&zw1,&j);
					hullar(i,0,numlin,&areainr,&areaoutr,dz2,-sina,cosa,&hb2,&wetw2,&wwcub2,&zw2,&j);
					areaoutl = max(areaoutl,areainl)+max(areainr,areaoutr);
					if(strncmp(param+4,"ZC",2) == 0){
						if(areaoutl <= 0.0) {
							*result = -999.9;
						}
						else {
							rightm(i,0,extlin, sina,cosa,dz2,&zw1,&hb1,&hb2); /* last 2 are dummies */
							rightm(i,0,extlin,-sina,cosa,dz2,&zw2,&hb1,&hb2);
							*result = -(zw1 + zw2)/areaoutl;
						}
					}
					else {
						*result = areaoutl;
					}
				}
				else if(strncmp(param+4,"PS",2) == 0) {
					*result = xsect[i];
				}
				else if(strncmp(param+4,"LO",2) == 0) {
					*result = yline[linereq][i];
				}
				else if(strncmp(param+4,"VO",2) == 0) {
					*result = -zline[linereq][i];
				}
				else if(strncmp(param+4,"LC",2) == 0) {
					if(relcont[linereq]) {
						getparam(i,linereq,&a,&hb,&c,&hd);
						*result = yline[linereq][i] + 0.5 * a;
					}
					else {
						*result = ycont[linereq][i];
					}
				}
				else if(strncmp(param+4,"VC",2) == 0) {
					*result = -zcont[linereq][i];
				}
				else if(strncmp(param+4,"DR",2) == 0) {
					dz2 = wl-beta*xsect[i]-hwl[i];
					*result = -cleara(i,dz2,&ylow,&zlow,&j,&j);
				}
				else if(strncmp(param+4,"BM",2) == 0){
					dz2 = wl-beta*xsect[i]-hwl[i];
					hullar(i,0,numlin,&areainl,&areaoutl,dz2, sina,cosa,&hb1,&wetw1,&wwcub1,&zw1,&i);
					hullar(i,0,numlin,&areainr,&areaoutr,dz2,-sina,cosa,&hb2,&wetw2,&wwcub2,&zw2,&i);
					*result = hb1 + hb2;
				}
				else {
					strcpy(out,"??invalid??");
					*result = 0.0;
					return FALSE;
				}
			}
			else {
				strcpy(out,"??invalid??");
				success = FALSE;
			}

		}
		else {	/* numeric and string variables */
			switch(i) {
			case numvar:	/* LENUN */
				strcpy(out,lenun[numun]);
				p = strchr(out,0);
				while(*--p == ' ') *p = 0;
				*result = 0.0;
				break;
			case numvar+1:	/* MASUN */
				strcpy(out,masun[numun]);
				p = strchr(out,0);
				while(*--p == ' ') *p = 0;
				*result = 0.0;
				break;
			case numvar+2:		/* ZBASE */
				reqval = -1.0e+30;
				for(i = stsec[stemli] ; i <= ensec[stemli] ; i++) {
					if(reqval < zline[stemli][i]) reqval = zline[stemli][i];
				}
				*result = -reqval;
				break;
			default:
				*result = varsign[i]*varval[i];
				break;
			}
		}

		/*	PUT		*/

	}
	else if(strcmp(in,"put") == 0) {
		strupr(param);
		if(strcmp(param,"LINE") == 0) {
			(void) findvalue(param,&i);
			i = varvalue[i];
			if(i > 0 && i <= extlin) linereq = i - 1;
			return success;
		}
		for(i = 0 ; i < numvar+numstring ; i++) {
			if(strcmp(param,var[i]) == 0) break;
		}
		if(i >= numvar+numstring) {
			if(strncmp(param,"TANK",4) == 0 &&
				    sscanf(param+7,"%d",&i) == 1 &&
				    i > 0 && i <= ntank) {
				i--;
				if(strncmp(param+4,"SG",2) == 0) {
					fl_spgra[i] = *result;
				}
				else if(strncmp(param+4,"PM",2) == 0) {
					fl_perm[i] = *result;
				}
				else if(strncmp(param+4,"LK",2) == 0) {
					fl_fixed[i] = (int) *result;
				}
				else if(strncmp(param+4,"FR",2) == 0) {
					if(fl_fixed[i] == 1) (*result) *= 0.01*fl_volum[i];	// contents specified as volume: fl_fract becomes a volume, not a percent
					fl_fract[i] = *result;
				}
				else {
					success = FALSE;
				}
			}
			else {
				success = FALSE;
			}

		}
		else {
			if(i == 42) {
				densit[numun] = *result;
			}
			else if(i < 42) {
				varval[i] = varsign[i] * (*result);
			}
			else {
				success = FALSE;
			}
		}

		/*	Invalid command	*/

	}
	else {
		abortp("$Invalid command");
		success = FALSE;
	}
	return success;
}

/*	This routine locates a variable in the symbol table,
and returns its value if defined, otherwise adds it to
the table with value zero. I also returns the index of
the variable in the table, in "*indvar" and in the global
variable table_index.

If the variable has TEXTDATA type, this may be identified
by checking vartype[*indvar].
*/

REAL findvalue(char *var,int *indvar)
{
	int i;
	REAL result;

	strlwr(var);
	for(i = 0 ; i < numstatvar ; i += max(1,varsize[i])) {
		if(same(varname[i],var)) break;
	}
	if(i >= numstatvar) {
		result = 0.0;
		if(numstatvar >= maxvar) {
			abortp("Symbol table full");
		}
		else {
			strcpy(varname[numstatvar],var);
			*indvar = numstatvar;
			varsize[numstatvar] = 1;
			varvalue[numstatvar] = 0.0;
			vartype[numstatvar] = NUMBER;
			inc_numstatvar();
		}
	}
	else {
		table_index = i;
		result = varvalue[i];
		*indvar = i;
	}
	return result;
}

#endif

/*	Compare two strings in case-insensitive mode.	*/

int same(char *s1,char *s2)
{
	s1--;
	s2--;
	do {
		if(toupper(*++s1) != toupper(*++s2)) return 0;
	}
	while(*s1);
	return 1;
}

void abortp(char *s)
{
#ifdef HULLSTAT
	int saveoption = dispoption;
	if(tmpl != NULL) {
		fclose(tmpl);
		tmpl = NULL;
	}
	if(fout != NULL) {
		fclose(fout);
		fout = NULL;
		unlink(outdirnam);
	}
	dispoption = 3;
	showtext(s);
	dispoption = saveoption;
#else
	void message(char *);

	message(s);
#endif
}

void inc_numstatvar(void)
{
	if(numstatvar < maxvar) {
		numstatvar++;
	}
	else {
		abortp("$Symbol table full");
	}
}

