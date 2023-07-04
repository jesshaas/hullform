/* Hullform component - help_hfw.c
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
 
/*	Windows help for hullform		*/

#include "hulldesi.h"

#ifdef linux
#include "../version.h"
#include <signal.h>
#else
#include "..\version.h"
#include "Htmlhelp.h"
#endif

extern int HelpUsed,UpdatesUsed,TutorialUsed;
extern char *helpdir;
int child_pid = 0;
extern int htmlhelp;

void showhelp(char *file)
{
#ifdef linux
	char cmd[MAX_PATH];
	char user[40];
	char logname[40];
	char home[MAX_PATH];
	char display[44];
	extern char browser[];
	int sid;
	extern int errno;
	char *arg[3] = {browser,cmd,NULL};
	char *env[5] = {user,logname,home,display,NULL};
	struct sigaction sa;

/*	Test for kill of browser - when the browser is run and "SIG_IGN" is set,
	the child process when closed does not become a zombie, so its pid
	disappears.
*/
	if(child_pid != 0) {
		if(getsid(child_pid) == -1) child_pid = 0;
	}
	if(child_pid != 0) {

/*	This is a "netscapeism" to load a further URL or file into an existing
	process.
*/
		sprintf(cmd,"%s -remote 'openFile(%s/%s)'",browser,helpdir,file);
		system(cmd);
	} else if( (child_pid = fork()) == 0) {

/*	The child process needs several items from the environment to be able
	to open an X-window
*/
		sprintf(cmd,"%s/%s",helpdir,file);
		sprintf(user,   "USER=%s",   getenv("USER"));
		sprintf(logname,"LOGNAME=%s",getenv("LOGNAME"));
		sprintf(home,   "HOME=%s",   getenv("HOME"));
		sprintf(display,"DISPLAY=%s",getenv("DISPLAY"));
		execve(browser,arg,env);
	} else {

/*	This is needed to ensure the browser process is fully closed, without
	remaining in the zombie state, when a user closes it. Only then does it
	lose its pid, which makes the closure detectable.
*/
		signal(SIGCHLD,SIG_IGN);
	}
#else
	if(htmlhelp)
		HtmlHelp(hWnd,file, HH_DISPLAY_TOPIC,0l);
	else
		WinHelp(hWnd,file,HELP_INDEX,0l);
#endif
}


void help()
{
#ifdef linux
	showhelp("index.html");
#else
	extern HWND hWndMain;
	extern char *helpfile;
	if(htmlhelp)
		HtmlHelp(hWnd,helpfile, HH_DISPLAY_TOPIC,0l);
	else
		WinHelp(hWndMain,helpfile,HELP_INDEX,0l);
#endif
	HelpUsed = 1;
}

void help_contents()
{
#ifdef linux
	showhelp("index.html");
#else
	extern HWND hWndMain;
	extern char *helpfile;
	if(htmlhelp)
		HtmlHelp(hWnd,helpfile, HH_DISPLAY_TOPIC,0l);
	else
		WinHelp(hWndMain,helpfile,HELP_FINDER,0l);
#endif
	HelpUsed = 1;
}

void about()
{
	(void) getdlg(ABOUT,
		0,VERSION,
		0,BUILD_DATE,
		-1);
}

void context(int id)
{
#ifdef linux
	char text[40];
	sprintf(text,"ID%d.html",id);
	showhelp(text);
#else
	char text[MAX_PATH];
	extern char *helpfile;
	if(htmlhelp) {
		sprintf(text,"%s::/ID%d.html",helpfile,id);	// the "/" is needed to ensure style sheet and graphics come in correctly
		HtmlHelp(hWnd,text,HH_DISPLAY_TOPIC,0l);
	} else
		WinHelp(hWnd,helpfile,HELP_CONTEXT,id);
#endif
	HelpUsed = 1;
}

void context_str(char *id)
{
#ifdef linux
	char text[100];
	sprintf(text,"ID_%s.html",id);
	showhelp(text);
#else
	char text[MAX_PATH];
	extern char *helpfile;
	if(htmlhelp) {
		sprintf(text,"%s#%s",helpfile,id);	// the "/" is needed to ensure style sheet and graphics come in correctly
		HtmlHelp(hWnd,text,HH_DISPLAY_TOPIC,0l);
	}	// won't work under WinHelp
#endif
	HelpUsed = 1;
}



void helponhelp()
{
	(void) getdlg(HELP,-1);
}

void updates()
{
#ifdef linux
	showhelp("whatsnew.html");
#else
	showhelp("whatsnew.hlp");
#endif
	UpdatesUsed = 1;
}

void tutorial()
{
#ifdef linux
	showhelp("tutorial.html");
#else
	showhelp("tutorial.hlp");
#endif
	TutorialUsed = 1;
}


void http_hullform()
{
#ifdef linux
	extern char browser[];
	char command[MAX_PATH];
	sprintf(command,"%s http://www.hullform.com &",browser);
	system(command);
#else
	int windows_nt(void);
	STARTUPINFO StartupInfo;
	PROCESS_INFORMATION ProcessInformation;
	static char *cmd = "cmd /c start http://www.hullform.com/";

	StartupInfo.cb = sizeof(StartupInfo);
	StartupInfo.lpReserved = NULL;
	StartupInfo.lpDesktop = NULL;
	StartupInfo.lpTitle = NULL;
	StartupInfo.dwFlags = STARTF_USESHOWWINDOW	;
	StartupInfo.wShowWindow = SW_SHOWMINIMIZED;
	StartupInfo.cbReserved2 = 0;
	StartupInfo.lpReserved2 = NULL;

	if(CreateProcess(
		    NULL,
			windows_nt() ? cmd : cmd + 7,
			NULL,
			NULL,
			FALSE,
			CREATE_DEFAULT_ERROR_MODE,
			NULL,
			NULL,
			&StartupInfo,
		    &ProcessInformation) <= 0) {
		message("You have no web browser associated with URLs");

	}
#endif
}

