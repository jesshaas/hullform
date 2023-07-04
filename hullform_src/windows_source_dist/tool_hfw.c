/* Hullform component - tool_hfw.c
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
 
#ifdef PROF

/*	Windows programs accessed from within the HULLFORM menu	*/

#include "hulldesi.h"

#ifdef linux
#include <Xm/Text.h>
#endif

extern int addarg[];
extern int reload[];
extern int numprog;
void read_hullfile(FILE *f,char *hullfile,int all);
void save_hull(int);
extern char *helpfile;
extern HWND hWndMain;

void runprog(char *prog,int addarg,int reload)
{
	char command[MAX_PATH];
	FILE *fp;
#ifndef linux
	STARTUPINFO StartupInfo;
	PROCESS_INFORMATION ProcessInformation;
#endif

	strcpy(command,prog);
	if(addarg && *hullfile) {
		save_file();
		strcat(command," ");
		strcat(command,hullfile);
	}
#ifdef linux
	strcat(command," &");
	system(command);
#else
	StartupInfo.cb = sizeof(StartupInfo);
	StartupInfo.lpReserved = NULL;
	StartupInfo.lpDesktop = NULL;
	StartupInfo.lpTitle = NULL;
	StartupInfo.dwFlags = STARTF_USESHOWWINDOW	;
	StartupInfo.wShowWindow = SW_SHOWNORMAL;
	StartupInfo.cbReserved2 = 0;
	StartupInfo.lpReserved2 = NULL;

	if(CreateProcess(
		    NULL,
			command,
			NULL,
			NULL,
			FALSE,
			CREATE_DEFAULT_ERROR_MODE,
			NULL,
			NULL,
			&StartupInfo,
		    &ProcessInformation) <= 0) {
		message("Can not run requested program");

	}
#endif
	if(reload && *hullfile) {
		if((fp = fopen(hullfile,"rt")) == NULL) {
			message("CAN NOT RE-OPEN HULL FILE");
		}
		else {
			read_hullfile(fp,hullfile,TRUE);
#ifdef EXT_OR_PROF
			save_hull(MAINHULL);
#endif
		}
	}
}

void addprog()
{
	char title[MAX_PATH];
	char program[MAX_PATH];
	int arg = 0;
	int rel = 0;

	void browse_program(int code,HWND hWndDlg);

	title[0] = 0;
	program[0] = 0;
	if(numprog < 8 && getdlg(TOOLADD,
	INP_STR,(void *) title,
	INP_STR,(void *) program,
	INP_PBF,browse_program,
	INP_LOG,(void *) &arg,
	INP_LOG,(void *) &rel,-1)) {
		title[17] = 0;
		strcpy(progtext[numprog],title);
		strcpy(progname[numprog],program);
		addarg[numprog] = arg;
		reload[numprog] = rel;
#ifdef linux
		message("You will have to close and reopen this program to see the addition.");
#else
		InsertMenu(GetMenu(hWndMain),1001,MF_BYCOMMAND,1003+numprog,title);
		DrawMenuBar(hWndMain);
#endif
		numprog++;
	}
}

void browse_program(int code,HWND hWndDlg)
{
	char program[MAX_PATH];
#ifdef linux
	if(openfile(program,"rt","Find Program For Run Menu",
		"Executable programs\0*\0","*",program,NULL)) {
		XmTextSetString(wEdit[1],program);
	}
#else
	if(openfile(program,"rt","Find Program For Run Menu",
		"Executable programs(*.exe)\0*.exe\0All files (*.*)\0*.*\0","*.exe",
		program,NULL)) {
		strlwr(program);
		SetDlgItemText(hWndDlg,DLGEDIT + 1,program);
	}
#endif
}

void delprog()
{
	char dummy[4] = "";
	int i;
	extern int numprog;
	extern int addarg[];
	extern int reload[];
	extern char (*progtext)[17];
	extern char (*progname)[MAX_PATH];
	struct {
		int index;       /* index of result in table */
		char *string;    /* pointer to result */
		char *table[13]; /* table of strings for listbox, null string terminator */
	}
	tlist;
#ifndef linux
	extern HMENU hMainMenu;
#endif
	tlist.index = 0;
	tlist.string = dummy;
	for(i = 0 ; i < numprog ; i++) tlist.table[i] = progtext[i];
	tlist.table[numprog] = "";

	if(getdlg(PROGDEL,INP_LBX,&tlist,-1)) {

		i = tlist.index;
#ifdef linux
		message("You will have to restart this program to see the change");
#else
		RemoveMenu(hMainMenu,1003 + i,MF_BYCOMMAND);
		DrawMenuBar(hWndMain);
#endif
		numprog--;
		for( ; i < numprog ; i++) {
			strcpy(progtext[i],progtext[i+1]);
			strcpy(progname[i],progname[i+1]);
			addarg[i] = addarg[i+1];
			reload[i] = reload[i+1];
		}
	}
}

#endif

