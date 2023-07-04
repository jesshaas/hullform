/* Hullform component - file_men.c
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
#include <Xm/FileSB.h>
#include <Xm/ListP.h>
#include <Xm/Text.h>
extern Widget mainWindow;
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#endif

void realloc_stringers(int);
void redef_transom(void);
void recalc_tanks(void);
void calc_stringers(int strlin);
int no_expand(void);
void do_save(FILE *fp);
void save_file_as(void);
void reset_all(void);
extern int endlin;
extern int numbetw;
extern int balanced;
extern HWND hWndMain;
void convert8(void);
void save_hull(int);
void remove_stringers(int);
extern REAL *outl_thickness;


void update_picklist(char *hullfile,char *dirnam);
void makemenu(void);
void read_hullfile(FILE *fp,char *fn,int all);

extern int GL_surfaces_need_redraw;
extern int GL_wireframe_need_redraw;

extern int first_transom_warning;

extern char (*sectname)[12];

void check_negative_offsets(void);

extern int changed;

extern int view_on_load;
extern int *relcont;
extern int *autofair;
extern int current_hull;
void load_view(void);
void use_hull(int);
char *ext_pos(char *);
#ifdef PROF
extern int forward_freeboard_section;
extern int aft_freeboard_section;
#endif

#ifdef linux

struct tm HullFileTime;

void SetHullFileTime(char *hullfile)
{
	struct stat buf;
	if(stat(hullfile,&buf) != 0) {
		memset(&HullFileTime,0,sizeof(HullFileTime));
	} else {
		localtime_r(&buf.st_mtime,&HullFileTime);
	}
}

#else

SYSTEMTIME HullFileTime;

void SetHullFileTime(char *hullfile)
{
  FILETIME ftWrite;
  SYSTEMTIME FileTimeUTC;
  HANDLE hFile;

  hFile = CreateFile(hullfile,0,0,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
  if (!GetFileTime(hFile,NULL,NULL,&ftWrite)) return;
  CloseHandle(hFile);
  FileTimeToSystemTime(&ftWrite, &FileTimeUTC);
  SystemTimeToTzSpecificLocalTime(NULL,&FileTimeUTC,&HullFileTime);
}

#endif

void SetTitle(char *text)
{
#ifdef linux
	extern Widget topLevel;
	XmString xstr;
	xstr = XmStringCreateSimple(text);
	XtVaSetValues(topLevel,XmNtitleString,xstr,NULL);
	XmStringFree(xstr);
#else
	SetWindowText(hWndMain,text);
#endif
}

void load_file()
{
	FILE	*fp;
	extern char *query_save[];

#ifdef EXT_OR_PROF
	char hfile[MAX_PATH];
	char *f;
#endif

	/*	Warn user and provide for abort if the current design has not
	been saved
	*/
	if(changed) {
		if(MessageBox(hWndMain,"Do you wish to discard the current design?","THE DESIGN HAS BEEN CHANGED",
			MB_ICONSTOP | MB_YESNO) != IDYES) return;
	}

#ifdef PROF
	f = current_hull == OVERLAYHULL ? hfile : hullfile;
	if(openfile(f,"rt","Open a hull data file","hull data files(*.HUD)\0*.hud\0","*.hud",dirnam,&fp)) {
		read_hullfile(fp,f,TRUE);
		save_hull(current_hull);
		if(count > 0) update_picklist(hullfile,dirnam);
	}
	forward_freeboard_section = -1;
	aft_freeboard_section = -1;
#else
	if(openfile(hullfile,"rt","Open a hull data file","hull data files(*.HUD)\0*.hud\0","*.hud",dirnam,&fp)) {
		read_hullfile(fp,hullfile,TRUE);
		if(count > 0) update_picklist(hullfile,dirnam);
	}
#endif
}

void update_picklist(char *hullfile,char *dirnam)
{
	int i,j;
	char newname[500];
	char newpath[500];
	char *p,*q;
#ifdef linux
	extern Widget previous_file_widget[4];
	XmString xstr;
#endif

	p = hullfile;
#ifdef linux
	while((q = strchr(p,'/')) != NULL) p = ++q; /* last '/' or start */
#else
	while((q = strchr(p,'\\')) != NULL) p = ++q; /* last '\' or start */
#endif
	if(strlen(p) >= 255) *(p+255) = 0;
	strcpy(newname,p);	/* file name without path */
	if(p == hullfile) {	/* if name only, use default directory */
		strcpy(newpath,dirnam);
	}
	else {
		strcpy(newpath,hullfile);
		*(newpath + (int) (p - hullfile) - 1) = 0;
	}
	for(i = 0 ; i < 3 ; i++) {
		if(	strcmp(newname,lastname[i]) == 0 &&
			strcmp(newpath,lastpath[i]) == 0) break;
	}
	for(j = i ; j > 0 ; j--) {
		strcpy(lastname[j],lastname[j-1]);
		strcpy(lastpath[j],lastpath[j-1]);
	}
	strcpy(lastname[0],newname);
	strcpy(lastpath[0],newpath);
#ifndef linux
	makemenu();
#else
	for(i = 0 ; i < 4 ; i++) {
		xstr = XmStringCreateSimple(lastname[i]);
		XtVaSetValues(previous_file_widget[i],XmNlabelString,xstr,NULL);
		XmStringFree(xstr);
	}
#endif
}

extern void clrscr(void),general_orth(void),plan_orth(void),elev_orth(void),
end_orth(void),full_persp(void),port_persp(void),starb_persp(void);
FUNC_PTR loadview[] = {clrscr,
	general_orth,plan_orth,elev_orth,end_orth,
	full_persp,port_persp,starb_persp		};

void check_invalid_value(REAL *value,char *name,int line,int section);

void read_hullfile(FILE *fp,char *hullfile,int all)
{
	int		i,j,k,is,ie;
	int		version;
	char	text[520];
	extern int	ymaxi,ychar;
	extern int	help[],helpind;
	extern REAL Xwidth,Ywidth;
	extern int	editinit;
	REAL	discard;
	char	*p;
	extern int *editsectnum;

#ifdef STUDENT
	int		previous_partial_line = FALSE;
	int		previous_stem_radius = FALSE;
	REAL	dztransom,atransom;
#else
	int		jtank;
#endif

#ifdef DEMO
#ifdef HULLSTAT
	short int *buffer;
	short int *ip;
	extern int allow_hullstat;
#define buffersize 2048

	/* Demo version only works with file hst-demo.hud as provided */

	if(!memavail((void *) &buffer,sizeof(short int)*buffersize)) {
		message("Out of free memory in file input");
		return;
	}
	i = 0;
	srand(46821u);
	while((j = fread(buffer,sizeof(short int),buffersize,fp)) > 0) {
		k = 0;
		ip = buffer;
		while(k++ < j) i ^= (rand() ^ *ip++);
	}
#ifdef linux
	allow_hullstat = (i == 1751022558);
#else
	allow_hullstat = (i == 16559);
#endif
	memfree(buffer);
	fseek(fp,0l,0);
#endif
#endif

#ifdef PROF
	for(j = extlin-1 ; j >= 0 ; j--) remove_stringers(j);
#endif

	waitcursor();

#ifndef STUDENT
	if(all) {
		for(j = 0 ; j < maxsec ; j++) sprintf(sectname[j],"%2d",j);
		realloc_stringers(0);
		for(j = 0 ; j < maxlin ; j++) {
			numstr[j] = 0;
			str_interv[j] = 0;
		}
		ststr[0] = -1;
	}
#endif

	/*	read	displacement,			*/
	/*		(x,z)-centre of mass position,	*/
	/*		code for units used,		*/
	/*		number of hull lines used	*/
	/*		line to which stem joins	*/
	/*		transom details			*/

	if(fgets(text,MAX_PATH,fp) == NULL) {
		message("Read failure at header line");
		goto errorpoint;
	}
	if((all && (i = sscanf(text,"%f %f %f %3d %3d %3d %f %f %d %d",
		&disp,&xcofm,&zcofm,&numun,&numlin,&stemli,
		&dztransom,&atransom,&version,&numbetw)) < 10) ||
		    (!all && (i = sscanf(text,"%f %f %f %3d %3d %3d %f %f %d %d",
		&disp,&discard,&discard,&numun,&numlin,&stemli,
		&discard,&discard,&version,(int *) &discard)) < 10 )) {
		if(i < 6) {
			message("Header line incomplete");
			goto errorpoint;
		}
		numbetw = 0;	/* pre-5D.13 */
		if(i < 9) {
			dztransom = 0.0;	/* Version 3 format */
			atransom = 0.0;
			version = 0;
		}
	}

	version += 3;	/* change to correct version number */

	/*	By using 1 rather than TRUE for surfacemode, we allow it
	to be used as a section start index, regardless of system-
	specific definitions of TRUE
	*/
#ifdef EXT_OR_PROF

	if(disp < 0.0)
		surfacemode = 1;
	else
	    surfacemode = 0;

	if(surfacemode) {
		for(i = 0 ; i < extlin ; i++) {
			if(stsec[i] <= 0) stsec[i] = 1;
		}
	}
#else
	if(disp < 0.0) message("This design is a hull surface, not a normal hull.\nYou will need to edit the stem region, and set\nthe displacement.");
#endif

#ifndef STUDENT
	if(all) transom = (atransom != 0.0);
#endif

	numun--;	/* version 3 compatibility adjustments */
	stemli--;
	extlin = 0;
	if(numun < 0 || numun > 3) numun = 0;
	if(stemli < 0 || stemli >= numlin) stemli = numlin-1;

	/*	obtain memory for lines			*/

	if(numlin > maxlin) {
		message("Too many lines for this program version");
		count = 0;
		numlin = 0;
	}
	else if(no_expand()) {
		message("Insufficient free memory");
		count = 0;
		numlin = 0;
	}
	else {

		/*	if memory is available, read section data until exhausted	*/

		count = 0;	/* section count */
		while(1) {

			/*		Section position is first element of input line		*/

			if(fscanf(fp,"%f",&xsect[count]) < 1) break;

			/*		next, read the hull line parameters in groups of four	*/

			for(i = 0 ; i < numlin ; i++) {
				if( fscanf(fp,"%f %f %f %f",
					&yline[i][count],&zline[i][count],
					&ycont[i][count],&zcont[i][count]) == EOF) {
						if(version > 4) {
							message("Out of data while reading hull offsets");
							goto errorpoint;
						} else {
							goto no_more_data;
						}
				}
				check_invalid_value(&yline[i][count],"Lateral offset",i,count);
				check_invalid_value(&zline[i][count],"Vertical offset",i,count);
				check_invalid_value(&ycont[i][count],"Lateral control offset",i,count);
				check_invalid_value(&zcont[i][count],"Vertical control offset",i,count);
			}

			if(all) hwl[count] = 0.0; /* cancel waterline */
			if(count < maxsec+2) {
				count++;
			}
			else {
				message("Design has too many sections - the sternmost ones have been discarded");
				break;
			}
		}

		/*		version 4+ files include line end-curve data, in the	*/
		/*		last two sections					*/

no_more_data:
		if(version >= 4) count -= 2;
		for(i = 2+surfacemode ; i < count ; i++) {
			if(xsect[i] < xsect[i-1]) {
				sprintf(text,"Section %d was forward of section %d\nIt has been moved aft.",i,i-1,i);
				message(text);
				xsect[i] = xsect[i-1] + 0.0002;
			}
		}
try_again:
		for(i = surfacemode ; i < count ; i++) {
			for(j = i+1 ; j < count ; j++) {
				if(xsect[j] == xsect[i]) {
					sprintf(text,"Sections %d and %d were coincident.\nSection %d has been moved aft.",j,i,j);
					message(text);
					xsect[j] += 0.0001;
					goto try_again;
				}
			}
		}

		/*		now read start and end-section data from		*/
		/*		version 5 file and move end-curve terms			*/

		if(version >= 5) {
			fgets(text,MAX_PATH,fp);/* skip "line ends" line */
		}

		for(i = 0 ; i < numlin ; i++) {
			if(version >= 5) {
				if(fscanf(fp,"%d %d",&is,&ie) < 2) {
					message("Missing line end data");
					goto errorpoint;
				}
				if(is < surfacemode || is >= count || ie < surfacemode || ie >= count) {
					message("Invalid line end value");
					goto errorpoint;
				}
#ifdef STUDENT
				if((is > surfacemode || ie < count-1) && !previous_partial_line) {
					previous_partial_line = TRUE;
					message("The design included partial lines.\nThese have been extended to\nthe stem and stern.");
					k = is;
					for(j = 0 ; j < count ; j++) {
						if(j == is) {
							j = ie+1;
							k = ie;
						}
						yline[i][j] = 0.0;
						zcont[i][j] = zline[i][j] = zline[stemli][j];
						ycont[i][j] = 0.0;
					}
				}
				stsec[i] = 0;
				ensec[i] = count-1;
#else
				stsec[i] = is;
				ensec[i] = ie;
#endif
			}
			else {
				stsec[i] = 0;
				ensec[i] = count-1;
			}

			k = count;
			for(j = maxsec ; j < maxsec+2 ; j++) {
				if(version >= 4) {
					yline[i][j]   = yline[i][k];
					zline[i][j]   = zline[i][k];
					ycont[i][j]   = ycont[i][k];
					zcont[i][j]   = zcont[i][k];
				}
				else {
					yline[i][j]   = 0.0;
					zline[i][j]   = 0.0;
					ycont[i][j]   = 0.0;
					zcont[i][j]   = 0.0;
				}
				k++;
			}
		}

		/*	read tank-line data				*/

#ifndef STUDENT
		if(version >= 5) {
			if(fscanf(fp,"%d\n",&jtank) < 1) {
				message("Missing tank line count");
				goto errorpoint;
			}
			if(all) ntank = jtank;
			for(i = 0 ; i <= jtank ; i++) {
				if(version >= 8) {	/* 3rd generation 6 */
					if(fgets(text,MAX_PATH,fp) != NULL) {
						if(all) {
							j = sscanf(text,"%d %d %d %f %f %f",
								&fl_line1[i],&fl_right[i],
								&fl_fixed[i],&fl_fract[i],
								&fl_spgra[i],&fl_perm[i]);
							if(j < 5) ntank = i;
							if(j == 5) fl_perm[i] = 1.0;
							p = strchr(text,'|');
							if(p != NULL) strcpy(tankdesc[i],p+1);
							if((p = strchr(tankdesc[i],'\n')) != NULL) *p = 0;
							if((p = strchr(tankdesc[i],'\r')) != NULL) *p = 0;
							if(*tankdesc[i] == 0) sprintf(tankdesc[i],"Tank %d",i+1);
						}
					}
					else if(all) {
						ntank = i;
						break;
					}
					else {
						break;
					}
				}
				else {
					if(fscanf(fp,"%d %d %d %d",&fl_line1[i],&j,
						&fl_right[i],&fl_fixed[i]) < 4) ntank = i;
					fl_fract[i] = 100.0;
					fl_spgra[i] = 1.0;
					fl_perm[i] = 1.0;
					*tankdesc[i] = 0;
				}
			}
		}
		else {
			ntank = 0;
			fl_line1[0] = numlin;
		}
#else
		if(version >= 5) {
			if(fscanf(fp,"%d\n",&j) < 1) {
				message("Missing tank line count");
				goto errorpoint;
			}
			fgets(text,sizeof(text),fp);
			sscanf(text,"%d",&k);
			if(j > 0) {
				message("The design included internal tanks.\nThese have been discarded.");
				realloc_hull(k);
				numlin = k;
				for(i = 0 ; i < j ; i++) {
					if(fgets(text,MAX_PATH,fp) != NULL) break;
				}
			}
		}
#endif
		extlin = numlin;
		endlin = numlin;

#ifdef STUDENT

		/*		Discard any stem radius data			*/

		for(i = 0 ; i < numlin ; i++) {
			if(fscanf(fp,"%f",&discard) == 1) {
				if(discard > 0.0 && !previous_stem_radius) {
					previous_stem_radius = TRUE;
					message("The design included a non-zero stem radius.\nThis has been set to zero.");
				}
			}
		}

		/*	Discard transom radius	*/

		if(version >= 7 && fscanf(fp,"%f\n",&discard) == 1) {
			if(discard > 0.0) message("The design included a transom.\nThis has been deleted.");
		}

		if(version >= 9) {

			/*	Discard stringer data	*/

			do {
				fscanf(fp,"%d",&j);	/* stringer index */
				if(j-- > 0) {
					if(fgets(text,120,fp) == NULL ||
						    fgets(text,120,fp) == NULL) {
						message("Missing stringer lines");
						goto errorpoint;
					}
				}
			}
			while(j >= 0);
		}

#else

		if(all && ntank > 0) {
			numlin = fl_line1[0];
			showtanks = 1;
		}
		/*		stem radius data			*/

		for(i = 0 ; i < numlin ; i++) {
			if(fscanf(fp,"%f",&radstem[i]) < 1) radstem[i] = 0.0;
		}

		/*	Read transom radius	*/

		if(all) {
			if(version >= 7) /* second generation Windows */
				if(fscanf(fp,"%f",&rtransom) <= 0) rtransom = 0.0;
		}
		else {
			if(version >= 7) fscanf(fp,"%f",&discard);
		}

		if(all && fl_line1[0] < numlin) fl_line1[0] = numlin;

		if(version >= 9) {

			/*	Read stringer data and recalculate	*/

			if(all) for(j = 0 ; j < maxlin ; j++) strmode[j] = -1; /* undefined */
			do {
				fscanf(fp,"%d",&j);	/* stringer index */
				if(j-- > 0) {
					if(all) {
						fscanf(fp,"%f %f %d",&str_wid[j],&str_thk[j],
							&str_dir[j]);
						fscanf(fp,"%f %f %d",&str_interv[j],&str_firstint[j],
							&strmode[j]);
						showstringers = TRUE;
					}
					else {
						if(fgets(text,120,fp) == NULL ||
							    fgets(text,120,fp) == NULL) {
							message("Missing stringer lines");
							goto errorpoint;
						}
					}
				}
			}
			while(j >= 0);
		}
#endif

		if(version >= 11) {		/* read chine flags */
			for(i = 0 ; i < extlin ; i++) {
				if(fscanf(fp,"%d",&relcont[i]) < 1) {
					sprintf(text,"Missing control mode flags at line %d",i+1);
					message(text);
					goto errorpoint;
				}
			}
		}
		else {
			for(i = 0 ; i < extlin ; i++) relcont[i] = 1;
		}

		/*	Read section names	*/

		fgets(text,MAX_PATH,fp);	/* this discards linefeed */
		for(j = 0 ; j < count ; j++) {
			if(all) {
				if(fgets((char *) sectname[j],12,fp) == NULL) break;
				if((p = strchr((char *) sectname[j],'\n')) != NULL) *p = 0;
			}
			else {
				if(fgets(text,120,fp) == NULL) break;
			}
		}

		/*	Read master section tags	*/

		if(version >= 11) {
			for(i = 0 ; i < count ; i++) {
				if(fscanf(fp,"%d",&master[i]) < 1) master[i] = TRUE;
			}
			master[0] = TRUE;
			master[count-1] = TRUE;

			/*	Read auto-fair tags and line weights		*/

			for(j = 0 ; j < extlin ; j++) {
				if(fscanf(fp,"%d",&autofair[j]) < 1) autofair[j] = FALSE;
				for(i = 0 ; i < count ; i++) {
					if(fscanf(fp,"%f",&linewt[j][i]) < 1) linewt[j][i] = 1.0;
				}
			}

		/*	Read frame outline thicknesses	*/

			for(j = 0 ; j < extlin ; j++) {
				if(fscanf(fp,"%f",&outl_thickness[j]) < 1) outl_thickness[i] = 0.0;
			}

		}
		else {

			/*	Convert earlier design to new control point form	*/

			convert8();
		}

		/*	Perform checks and calculate stringers			*/

#ifndef STUDENT
		for(j = 0 ; j < extlin ; j++) {
			if(radstem[j] < 0.0) radstem[j] = 0.0;
			calc_stringers(j);
		}
#endif

		/*		confirm input	*/

#ifdef EXT_OR_PROF
		save_hull(current_hull);
		if(current_hull == MAINHULL) {
#endif
			strcpy(text,"HULLFORM - ");
			strcat(text,hullfile);
			if(surfacemode) strcat(text," (surface)");
			p = text;
			while(*p) {
				*p = toupper(*p);
				p++;
			}
			SetTitle(text);

#ifdef EXT_OR_PROF
		}
#endif

		/*	set up any transom definition	*/

		if(all) {
#ifndef STUDENT
			redef_transom();
			recalc_tanks();
#endif
#ifdef PLATEDEV
			for(i = 1 ; i < maxlin ; i++) developed[i] = -1;
			numruled = 0;
#endif
			reset_all();
			Xwidth = 0.0;
			Ywidth = 0.0;
			changed = 0;
			balanced = 0;
			editinit = 1;
			check_negative_offsets();
			load_view();
			GL_surfaces_need_redraw = TRUE;
			GL_wireframe_need_redraw = TRUE;
		}
	}
#ifdef PROF
	first_transom_warning = TRUE;
#endif
	goto returnpoint;

errorpoint:

#ifdef PROF
	if(current_hull != OVERLAYHULL) {
#endif
		numlin = 0;
		count = 0;
		strcpy(hullfile,"(undef)");

		SetTitle("HULLFORM - (undef)");

#ifdef EXT_OR_PROF
	}
#endif

returnpoint:
	fclose(fp);
	SetHullFileTime(hullfile);
	for(i = 0 ; i < count ; i++) editsectnum[i] = TRUE;
	strcpy(alsosect,"ALL");
	strcpy(showline,"ALL");
	arrowcursor();
	if(all) {
		wl = 0.0;
		density = densit[numun];
	}
}

void check_invalid_value(REAL *value,char *name,int line,int section)
{
	char text[100],*err;
	sprintf(text,"%.2f",*value);

	if(strstr(text,"INF") != NULL) {
		err = "infinite (INF)";
	}
	else if(strstr(text,"NAN") != NULL) {
		err = "not-a-number (NAN)";
	}
	else {
		return;
	}
	sprintf(text,"%s at line %d, section %d\nwas %s - set to zero",name,line,section,err);
	message(text);
	*value = 0.0;
}

void reset_all()
{
	int i;
	extern int *ignore;
	extern int editinit;
#ifndef STUDENT
	numruled = 0;
#endif
	for(i = 0 ; i < count ; i++) {
		ignore[i] = FALSE;
		secton[i] = TRUE;
	}
	strcpy(sectionstoplot,"ALL");
	editinit = TRUE;
}

void load_view(void)
{
	extern HDC hDC;
	extern int rotatable;

#ifdef HULLSTAT
	extern char *statline;
	if(statline == NULL) {	/* Hullstat not running */
#endif
		update_func = loadview[view_on_load];
		print_func = update_func;
		if(hDC == NULL) setup(scrdev);
		(*update_func)();
		prev_func = update_func;
#ifdef PROF
		rotatable = update_func == full_persp || update_func == port_persp ||
		    update_func == starb_persp || update_func == view_devel;
#else
		rotatable = update_func == full_persp || update_func == port_persp ||
		    update_func == starb_persp;
#endif
#ifdef HULLSTAT
	}
#endif
}

/************************************************************************/

/*	write data to file						*/

/************************************************************************/

void save_file_as()
{
#ifdef DEMO
	not_avail();
#else
	char text[140];
	FILE *fp,*fopen();

	/*	If no entry, return with no output	*/

	if(count > 0 && openfile(hullfile,"wt","Save the current hull data file",
		"hull data files(*.HUD)\0*.hud\0","*.hud",dirnam,&fp)) {
		do_save(fp);
		update_picklist(hullfile,dirnam);
	}

	strcpy(text,"HULLFORM - ");
	strcat(text,hullfile);

	SetTitle(text);

#endif
}

void save_file()
{
#ifdef DEMO
	not_avail();
#else
	FILE *fp;
	int result;
	/*
	#ifndef _fmode
	extern int _fmode;
	#endif
	*/
	if(count > 0) {
		if(hullfile[0]) {
			result = (fp = fopen(hullfile,"wt")) != NULL;
			if(!result) {
				message("THIS FILE NOT ACCESSIBLE");
			}
			else {
				do_save(fp);
			}
		}
		else {
			save_file_as();
		}
	}
#endif
}

void do_save(FILE *fp)
{
#ifdef DEMO
	not_avail();
#else
	INT	i,ios,j;

#ifdef STUDENT
#define dztransom 0.0
#define atransom 0.0
#endif

#ifdef SHAREWARE
	nagbox();
#endif

	waitcursor();

#ifdef EXT_OR_PROF
	if(current_hull != MAINHULL) {
		message("Hull management error!");
		return;
	}
#endif

	changed = 0;
	if(extlin < numlin) extlin = numlin;
	if(fprintf(fp,"%12.4f%10.4f%10.4f %3d %3d %3d%9.4f%9.4f 8 %d",
		disp,xcofm,zcofm,numun+1,extlin,stemli+1,dztransom,
		atransom,numbetw) >= 63) {
		ios = 1;
		for(i = 0 ; i < maxsec+2 ; i++) {
			if(i == count) i = maxsec;
			if(fprintf(fp,"\n%9.4f",xsect[i]) < 9) goto ad0003;
			for(j = 0 ; j < extlin ; j++) {
				check_invalid_value(&yline[j][i],"Lateral offset",j,i);
				check_invalid_value(&zline[j][i],"Vertical offset",j,i);
				check_invalid_value(&ycont[j][i],"Lateral control offset",j,i);
				check_invalid_value(&zcont[j][i],"Vertical control offset",j,i);
				if(j && !(j & 1)) fprintf(fp,"\n         ");
				if( (ios = fprintf(fp," %8.4f %8.4f %8.4f %7.4f",
					yline[j][i],zline[j][i],
					ycont[j][i],zcont[j][i])) < 24) goto ad0003;
			}
		}
	}

	/*	write line-end data				*/

	if(fprintf(fp,"\nline ends") < 10) goto ad0003;
	for(j = 0 ; j < extlin ; j++) {
		if(fprintf(fp,"\n%2d %2d",stsec[j],ensec[j]) < 5)
			goto ad0003;
	}

	/*	write tank-line data				*/

#ifdef STUDENT
	fprintf(fp,"\n0\n%d 0 0 0 0 0 0",numlin);
#else
	if(fprintf(fp,"\n%d",ntank) < 1) goto ad0003;
	for(i = 0 ; i <= ntank ; i++) {
		if(fprintf(fp,"\n%d %d %d %.4f %.4f %.4f |%s",fl_line1[i],fl_right[i],fl_fixed[i],
			fl_fract[i],fl_spgra[i],fl_perm[i],tankdesc[i]) < 5) goto ad0003;
	}
#endif

	/*	stem radius data				*/

	for(i = 0 ; i < numlin ; i++) {
#ifdef STUDENT
		fprintf(fp,"\n0.000");
#else
		if(fprintf(fp,"\n%.3f",radstem[i]) < 1) goto ad0003;
#endif
	}

	/*	transom radius and stringers				*/

#ifdef STUDENT
	fprintf(fp,"\n0");
#else
	if(fprintf(fp,"\n%10.2f",rtransom) < 1) goto ad0003;
	for(j = 1 ; j < extlin ; j++) {
		if(numstr[j] > 0) fprintf(fp,"\n%d %f %f %d %f %f %d",j+1,str_wid[j],
			str_thk[j],str_dir[j],str_interv[j],str_firstint[j],strmode[j]);
	}
#endif
	fprintf(fp,"\n-1\n");

	for(i = 0 ; i < extlin ; i++)
		if(fprintf(fp," %d",relcont[i]) < 1) goto ad0003;
	fprintf(fp,"\n");

	for(j = 0 ; j < count ; j++) {
		if(fprintf(fp,"%s\n",(char *) sectname[j]) == EOF) break;
	}

	for(i = 0 ; i < count ; i++) fprintf(fp,"%d ",master[i]);
	for(j = 0 ; j < extlin ; j++) {
		fprintf(fp,"\n%d",autofair[j]);
		for(i = 0 ; i < count ; i++) fprintf(fp," %.4f",linewt[j][i]);
	}
	fprintf(fp,"\n");

/*	Write frame outline thicknesses	*/

	for(j = 0 ; j < extlin ; j++) {
		fprintf(fp," %.4f",outl_thickness[j]);
	}
	fprintf(fp,"\n");

	/*	Successful output: set no-error flag		*/

	ios = 0;

ad0003:
	if(ios != 0) message("WRITE TO FILE FAILED");
	fclose(fp);

//	Get system time for file date and time

#ifdef PROF
	SetHullFileTime(hullfile);
#endif

#endif
	arrowcursor();
}

/*	Dynamic waterline input						*/

void dynam_waterline()
{
	char file[MAX_PATH] = "";
	FILE *fp;
	int	nwl;	/* entry cancels previous data	*/
	int result;
	char direct[MAX_PATH] = ".";
	REAL xwl[maxsec],zwl[maxsec];
	REAL w[maxsec];
	int i;

	if(count <= 0) return;

	/*	Obtain name for input file					*/

	/* 	If a name was successfully input and it was not "NONE" ...	*/

	if(openfile(file,"rt","Select a waterline input file","\0\0\0",
		"*.*",direct,&fp)) {

		/*	Read the file until out of data					*/

		nwl = 0;
		do {
			result = fscanf(fp,"%f%f",&xwl[nwl],&zwl[nwl]);
		}
		while(result >= 2 && ++nwl < maxsec);

		/*	Confirm input of data						*/

		if(nwl > 0) {
			sprintf(file,"%d values read",nwl);
			message(file);
			for(i = 0 ; i  < count ; i++) w[i] = 1.0;
			spline(xwl,zwl,w,nwl,xsect,hwl,count,0.0,0.0);
		}
		else {
			message("Could not read this file");
		}
		fclose(fp);

	}
	else {

		/*	If user aborted, no waterline					*/

		for(nwl = 0 ; nwl < count ; nwl++) hwl[nwl] = 0.0;

	}
}

char waterline_file[MAX_PATH] = "hullform.txt";

void waterline_output()
{
#ifdef DEMO
	not_avail();
#else
	static REAL	dwl;
	extern INT	numun;
	int		i,none;
	REAL	dyoffs = 0.1;
	REAL	wl1,temp,tem2,ww,d1,d2,area1,area2;
	REAL	xoff[maxsec],yoff1[maxsec],yoff2[maxsec],zoff1[maxsec],zoff2[maxsec];
	INT		il;
	FILE	*fp;
	REAL	areain,areaout;
	char	usetext[MAX_PATH];
	int		use[maxsec];
	REAL	ystem = surfacemode ? 0.0 : yline[stemli][1];
	char	*warning = "CAN NOT WRITE TO THIS FILE";
	void waterline_func(int code,HWND hWndDlg);

#ifdef SHAREWARE
	nagbox();
#endif
	dwl = wl;

	if(count <= 0) return;
	strcpy(usetext,"ALL");
	cls(0);
	update_func = NULL;
	temp = dwl * invert;
	if(getdlg(WATERLIN,
		INP_REA,(void *) &temp,
		INP_REA,(void *) &dyoffs,
		INP_STR,(void *) usetext,
		INP_STR,(void *) waterline_file,
		INP_PBF,waterline_func,-1) &&
		    multproc(usetext,use,count) &&
		    (fp = fopen(waterline_file,"wt")) != NULL ) {

		waitcursor();
		dwl = temp * invert;
		while(1) {

			none = 1;

			/*	DO STEM SECTION FIRST	*/

			wl1 = dwl-beta*xsect[0]-hwl[0];
			temp = 1.0+beta*beta;
			temp = fsqr0(temp);
			temp = 1.0/temp;
			hullar(0,0,numlin,&areain,&areaout,wl1,beta*temp,temp,&tem2,
				&ww,&d1,&d2,&il);
			if(tem2 > 1.0e-30) {
				xoff[0] = xsect[0] - tem2;
				none = 0;
			}
			else {
				xoff[0] = xsect[0];
			}
			yoff1[0] = ystem;
			yoff2[0] = ystem;
			zoff1[0] = zline[stemli][0];
			zoff2[0] = zoff2[0];

			for(i = 1-surfacemode ; i < count ; i++) {
				wl1 = dwl-beta*xsect[i]-hwl[i];
				xoff[i] = xsect[i];
				hullar(i,0,numlin,&areain,&area1,wl1, sina,cosa,&yoff1[i],&ww,&d1,&zoff1[i],&il);
				hullar(i,0,numlin,&areain,&area2,wl1,-sina,cosa,&yoff2[i],&ww,&d1,&zoff2[i],&il);
				if(area1+area2 > 1.0e-30)none = 0;
				if(yoff1[i] < ystem) yoff1[i] = ystem;
				if(yoff2[i] < ystem) yoff2[i] = ystem;
			}

			if(!none) {

				/*	THEN THE DATA ARE OUTPUT TO THE REQUESTED FILE	*/

				if(!fprintf(fp,"WATERLINE OFFSET %8.3f:\n",dwl))
					goto breakout;
				for(i = 0 ; i < count ; i++) {
					if(!use[i]) continue;
					if( fprintf(fp,"%8.3f %8.3f %8.3f %8.3f %8.3f\n",
						posdir*xoff[i],yoff1[i],yoff2[i],-zoff1[i],-zoff2[i]) < 45) {
						message(warning);
						goto breakout;
					}
				}
				dwl += dyoffs;
			}
			else {	/* end any-intersections conditional	*/
				break;
			}
		}	/* end waterline value loop */
breakout:
		fclose(fp);
		arrowcursor();
	}		/* end file-opened conditional		*/
#endif
}

void waterline_func(int code,HWND hWndDlg)
{
	if(openfile(waterline_file,"wt","Waterline Output File","text files(*.TXT)\0*.txt\0","*.txt",filedirnam,NULL)) {
#ifdef linux
		XmTextSetString(wEdit[3],waterline_file);
#else
		strlwr(waterline_file);
		SetDlgItemText(hWndDlg,DLGEDIT+3,waterline_file);
#endif
	}
}

#ifdef OPENFILE

/****** open the file *********/

char	*fname = NULL;
#ifdef linux
Widget	open_dialog;		/* The file selection box widget */
int open_proceed;
extern XtAppContext app_context;
#endif

extern Pixel DialogBackground,EditBackground,ButtonBackground,TopShadow,BottomShadow;

int openfile(char filename[MAX_PATH],char *rw,char *title,char *filetype,
	char *defname,char *dirnam,FILE **fp)
{
	int result;
	char *p1,*p2;
	char text[500];
	int pending;

#ifdef linux
	XmString mask,xstr;
	Arg args[12];
	int narg;
	Widget wdir,hdir,vdir,wfile,hfile,vfile;
	void DialogCB(Widget w,XtPointer client_data,XtPointer call_data);
	void GreyButton(Widget w);
	void WhiteField(Widget w);

	sprintf(text,"%s/*.hud",dirnam);
	xstr = XmStringCreateSimple(text);
	mask = XmStringCreateSimple(dirnam);
	narg = 0;
	XtSetArg(args[narg],XmNwidth,480);
	narg++;
	XtSetArg(args[narg],XmNpattern,xstr);
	narg++;
	XtSetArg(args[narg],XmNdirMask,mask);
	narg++;

	XtSetArg(args[narg],XmNbackground,ButtonBackground);
	narg++;
	XtSetArg(args[narg],XmNtopShadowColor,TopShadow);
	narg++;
	XtSetArg(args[narg],XmNbottomShadowColor,BottomShadow);
	narg++;

	open_dialog = XmCreateFileSelectionDialog(mainWindow,title,args,narg);
	XtAddCallback(open_dialog,XmNokCallback,    (void *) DialogCB,(XtPointer *) DIALOG_OK);
	XtAddCallback(open_dialog,XmNcancelCallback,(void *) DialogCB,(XtPointer *) DIALOG_CANCEL);

	wdir = XmFileSelectionBoxGetChild(open_dialog,XmDIALOG_DIR_LIST);
	XtVaGetValues(wdir,
		XmNhorizontalScrollBar,&hdir,
		XmNverticalScrollBar,&vdir,
		NULL);
	WhiteField(wdir);
	GreyButton(vdir);
	XtVaSetValues(XtParent(wdir),XmNbackground,DialogBackground,NULL);

	wfile = XmFileSelectionBoxGetChild(open_dialog,XmDIALOG_LIST);
	WhiteField(wfile);

	XtVaGetValues(wfile,
		XmNhorizontalScrollBar,&hfile,
		XmNverticalScrollBar,&vfile,
		NULL);
	GreyButton(vfile);
	XtVaSetValues(XtParent(wfile),XmNbackground,DialogBackground,NULL);

	WhiteField(XmFileSelectionBoxGetChild(open_dialog,XmDIALOG_TEXT));
	WhiteField(XmFileSelectionBoxGetChild(open_dialog,XmDIALOG_FILTER_TEXT));

	XtUnmanageChild(XmFileSelectionBoxGetChild(open_dialog,XmDIALOG_APPLY_BUTTON));
	XtManageChild(open_dialog);

	XmStringFree(mask);
	XmStringFree(xstr);
	open_proceed = 0;
	XtUnmanageChild(hdir);
	XtUnmanageChild(hfile);

	while(open_proceed == 0 | (pending = XtAppPending(app_context)) ) {
		if(pending)
			XtAppProcessEvent(app_context, XtIMAll);
		else
		    usleep(100000);
	}
	XtUnmanageChild(open_dialog);

	result = fname != NULL;

	if(result) {
		strcpy(filename,fname);
		result = (*fp = fopen(filename,rw)) != NULL;
		if(!result) {
			sprintf(text,"FILE NOT ACCESSIBLE:\n%s",filename);
			message(text);
		}
	}
	return(result);
#else
	OPENFILENAME opfn;

	memset(&opfn,0,sizeof(OPENFILENAME));
	opfn.lStructSize = sizeof(OPENFILENAME);
	opfn.hwndOwner = hWndMain;
	opfn.hInstance = hInst;
	opfn.lpstrFilter = filetype;
	opfn.lpstrCustomFilter = NULL;
	opfn.nFilterIndex = 1;
	strcpy(filename,defname);
	opfn.lpstrFile = filename;
	opfn.nMaxFile = MAX_PATH;
	opfn.lpstrFileTitle = NULL;
	opfn.lpstrInitialDir = dirnam;
	opfn.lpstrTitle = title;
	if(*rw == 'r')
		opfn.Flags = OFN_FILEMUSTEXIST;
	else
		opfn.Flags = OFN_OVERWRITEPROMPT;
	opfn.Flags |= OFN_PATHMUSTEXIST;

	opfn.lpstrDefExt = ext_pos(defname) + 1;
	result = (*rw != 'w') ? GetOpenFileName(&opfn) : GetSaveFileName(&opfn);

	if(result && fp != NULL) {

		/*	Prevent double extensions where the second is the default	*/

		p1 = ext_pos(filename);
		p2 = strstr(p1+2,opfn.lpstrDefExt);
		if(p2 != NULL) *--p2 = 0;
		result = (*fp = fopen(filename,rw)) != NULL;
		if(!result) {
			sprintf(text,"FILE NOT ACCESSIBLE:\n%s",filename);
			message(text);
		}
		strcpy(dirnam,opfn.lpstrFile);
		*(dirnam + opfn.nFileOffset -1) = 0;
	}
	return(result);
#endif

}

#ifdef linux

void GreyButton(Widget w)
{
	XtVaSetValues(w,
		XmNbackground,ButtonBackground,
		XmNtopShadowColor,TopShadow,
		XmNbottomShadowColor,BottomShadow,
		XmNtroughColor,DialogBackground,
		NULL);
}

void WhiteField(Widget w)
{
	XtVaSetValues(w,
		XmNbackground,EditBackground,
		XmNtopShadowColor,TopShadow,
		XmNbottomShadowColor,BottomShadow,
		NULL);
}

void DialogCB(Widget w,XtPointer client_data,XtPointer call_data)
{
	XmFileSelectionBoxCallbackStruct *fcb;
	XmStringCharSet charset = (XmStringCharSet) XmSTRING_DEFAULT_CHARSET;

	switch ((int) client_data) {

	case DIALOG_OK:
		if (fname != NULL) {
			free(fname);
			fname = NULL;
		}
		fcb = (XmFileSelectionBoxCallbackStruct *) call_data;
		XmStringGetLtoR(fcb->value, charset, &fname);
		open_proceed = 1;
		break;
	case DIALOG_CANCEL:
		open_proceed = -1;
		break;
	}
}
#endif

#endif

int no_expand()
{
	if(extlin < numlin) extlin = numlin;
	return(	!realloc_hull(extlin) || extlin > maxlin);
}

void show_memory()
{
#ifdef linux
	(void) MessageBox(hWndMain,"Not implemented under Linux","Current System Memory Available",MB_OK);
#else
	MEMORYSTATUS  memstat;
	char report[256];
	cls(0);

	memstat.dwLength = sizeof(memstat);
	GlobalMemoryStatus(&memstat);
	sprintf(report,"Memory in use\t\t%ld %%\nPhysical memory\t\t%ld K\nFree physical memory\t%u K\n\nNOTE: The above statistics are reported directly by Windows.\nNo responsibility is accepted for any inconsistencies.",
		memstat.dwMemoryLoad,
		memstat.dwTotalPhys >> 10,
		memstat.dwAvailPhys >> 10);
	(void) MessageBox(hWndMain,report,"Current System Memory Available",MB_OK);
#endif
}

/*	Convert control parameters from pre-8 to Hullform 8 form */

void convert8()
{
	int i,j,j0;
	relcont[0] = TRUE;
	for(j = 1 ; j < extlin ; j++) {
		i = stsec[j];
		if(stsec[j-1] < i) {
			zcont[j][i-1] = zcont[j][i];
			if(i > 0) {
				ycont[j][i-1] = ycont[j][i];
				i--;
			}
			else {
				ycont[j][i-1] = (1.0-zcont[j][i])*
				    (xsect[i-1] - xsect[i]);
			}
		}
		while(i <= ensec[j]) {
			for(j0 = j-1 ; j0 > 0 ; j0--) if(stsec[j0] <= i && ensec[j0] >= i) break;
			zcont[j][i] = zline[j][i]-zcont[j][i]*
			    (zline[j][i]-zline[j0][i]);
			i++;
		}
		relcont[j] = TRUE;
	}
	for(j = 0 ; j < extlin ; j++) {
		for(i = 0 ; i < count ; i++) linewt[j][i] = 1.0;
	}
	for(i = 1 ; i < count ; i++) master[0] = FALSE;
	master[0] = TRUE;
	master[count-1] = TRUE;
	master[count/2] = TRUE;
}

#ifndef linux

/****** open the file *********/

int openfile(char filename[MAX_PATH],char *rw,char *title,char *filetype,
	char *defname,char *dirnam,FILE **fp)
{
	OPENFILENAME opfn;
	int result;
	char *p1,*p2;
	char text[500];

	memset(&opfn,0,sizeof(OPENFILENAME));
	opfn.lStructSize = sizeof(OPENFILENAME);
	opfn.hwndOwner = hWndMain;
	opfn.hInstance = hInst;
	opfn.lpstrFilter = filetype;
	opfn.lpstrCustomFilter = NULL;
	opfn.nFilterIndex = 1;
	strcpy(filename,defname);
	opfn.lpstrFile = filename;
	opfn.nMaxFile = MAX_PATH;
	opfn.lpstrFileTitle = NULL;
	opfn.lpstrInitialDir = dirnam;
	opfn.lpstrTitle = title;
	if(*rw == 'r')
		opfn.Flags = OFN_FILEMUSTEXIST;
	else
		opfn.Flags = OFN_OVERWRITEPROMPT;
	opfn.Flags |= OFN_PATHMUSTEXIST;

	opfn.lpstrDefExt = ext_pos(defname)+1;
	result = (*rw != 'w') ? GetOpenFileName(&opfn) : GetSaveFileName(&opfn);
	if(result && fp != NULL) {

		/*	Prevent double extensions where the second is the default	*/

		p1 = ext_pos(filename);
		p2 = strstr(p1+2,opfn.lpstrDefExt);
		if(p2 != NULL) *--p2 = 0;

		result = (*fp = fopen(filename,rw)) != NULL;
		if(!result) {
			sprintf(text,"FILE NOT ACCESSIBLE:\n",filename);
			message(text);
		}
		strcpy(dirnam,opfn.lpstrFile);
		*(dirnam + opfn.nFileOffset -1) = 0;
	}
	return(result);
}
#endif

void quit()
{
#ifdef linux
	extern int exit_prog;
	exit_prog = 1;
#else
	extern HWND hWndMain;
	extern int xp_abort;
	PostMessage(hWndMain,WM_CLOSE,0,0l);
	save_config();
	xp_abort = TRUE;
#endif
}

char *ext_pos(char *fn)
{
	char *p = strchr(fn,0);
	char *e = p;
	while(p != fn) {
		if(*--p == '.') return p;
	}
	return e;
}

