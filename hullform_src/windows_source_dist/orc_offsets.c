/* Hullform component - orc_offsets.c
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
 
//    WRITE 3D MESH TO DXF TEXT FILE

#ifdef PROF

#include "hulldesi.h"

#ifdef linux
  extern struct tm HullFileTime;
#else
  extern SYSTEMTIME HullFileTime;
#endif

#ifdef linux
char *searchpath(char *file);
#include <Xm/Text.h>
#endif

char orc_filename[MAX_PATH];
int measurer_number = 0;
int measuring_device_number = 0;

//	YYMM series date
int series_date  = 0;
int forward_freeboard_section = -1;
int aft_freeboard_section = -1;
int keel_endplate_flag = 0;

void orc_func(int code,HWND hWndDlg)
{
	if(openfile(orc_filename,"wt","ORC Output File","ORC offset files(*.OFF)\0*.off\0","*.off",filedirnam,NULL)) {
#ifdef linux
		XmTextSetString(wEdit[4],orc_filename);
#else
		SetDlgItemText(hWndDlg,DLGEDIT+4,orc_filename);
#endif
	}
}

MENUFUNC orc_offsets(void)
{
	char job[60];
	char name[100];
	int numpoints;
	int i,j;
	REAL calcdisp;
	extern int numbetwl;
	void finish_stats(void);
	REAL scale;
	REAL a,hb,c,hd,t,tmax,dt,y,z;
	FILE *orc_fp;
	int orc_numpoints;
	char *station_fmt = "%10.2f,%3d,  1,%3d,%3d\n";	// position, point count, immersion point flag, section index
	char *offset_fmt = "%10.2f,%10.2f,%3d\n";		// z-offset, y-offset, immersion point flag
	REAL x_stem;
	char *p,*p1;
	REAL zmin,zmax;

	if(count < 2) return;

#ifdef DEMO

	message("Demonstration version:\nNo actual output");

#else

	if(forward_freeboard_section < 0) forward_freeboard_section = 2;
	if(aft_freeboard_section < 0) aft_freeboard_section = count - 2;

//	Generate default name

	p = strchr(hullfile,'.');
	while(p != hullfile) {
#ifdef linux
		if(*--p == '/') {
			p++;
			break;
		}
#else
		if(*--p == '\\') {
			p++;
			break;
		}
#endif
	}
	strcpy(orc_filename,filedirnam);
	strcat(orc_filename,"\\");
	p1 = strchr(orc_filename,0);
	strcat(orc_filename,p);
	strcpy(name,p);

	p = strchr(p1,'.');
	strcpy(p,".off");

	if(!getdlg(ORC_OFFSETS,
		INP_INT,(void *) &measurer_number,
		INP_INT,(void *) &series_date,
		INP_INT,(void *) &forward_freeboard_section,
		INP_INT,(void *) &aft_freeboard_section,
		INP_STR,(void *) orc_filename,
		INP_PBF,orc_func,-1) ) return;

	if( (orc_fp = fopen(orc_filename,"wt")) == NULL ) {
		message("Can't open requested file.");
		return;
	}

/*	Header section	*/

	p = strchr(name,'.');
	*p = 0;
	name[7] = 0;	// maximum length 7
	for(p = name ; *p != 0 ; p++) *p = toupper(*p);


	fprintf(orc_fp," %02d:%02d:%02d,%3d/%02d/%02d,%5d,%4d,%7s,%24s,%5d\n",
#ifdef linux
		HullFileTime.tm_hour,HullFileTime.tm_min,HullFileTime.tm_sec,HullFileTime.tm_mday,HullFileTime.tm_mon,HullFileTime.tm_year % 100,
#else
		HullFileTime.wHour,HullFileTime.wMinute,HullFileTime.wSecond,HullFileTime.wDay,HullFileTime.wMonth,HullFileTime.wYear % 100,
#endif
		measurer_number,
		numun <= 1 ? 1 : 0,
		name,				// file name and yacht name the same
		name,
		series_date);		// design series date

/*	The second line of the offset file may contain the following fields (7 Characters each):

     -SFFP Starboard  (distance from stem of the Fwd Freeboard station)
     -FFPV Starboard  Vertical distance from the forward starboard
                      freeboard point (flagged with PTC=1) to the
                      IOR sheer, positive up. Units are Feet or Meters.
     -SFFP Port       (distance from stem of the Fwd Freeboard station)
     -FFPV Port       Vertical distance from the forward starboard (I think they mean port)
                      freeboard point (flagged with PTC=1) to the
                      IOR sheer, positive up. Units are Feet or Meters.
*/
	a = dxstem();
	x_stem = xsect[0] - a;
	fprintf(orc_fp,"%7.3f,%7.3f,%7.3f,%7.3f\n",
		xsect[forward_freeboard_section] - x_stem,0.0,
		xsect[forward_freeboard_section] - x_stem,0.0);

/*      The third line of the offset file may contain the following fields (7 Characters each):

     -SAFP Starboard
     -AFPV Starboard  Vertical distance from the aft starboard
                      freeboard point (flagged with PTC=1) to
                      the IOR sheer, positive up.  Units are Feet
                      or Meters.
     -SAFP Port
     -AFPV Port       Vertical distance from the aft port freeboard
                      point (flagged with PTC=1) to the IOR sheer,
                      positive up.  Units are Feet or Meters.
*/
	fprintf(orc_fp,"%7.3f,%7.3f,%7.3f,%7.3f\n",
		xsect[aft_freeboard_section] - x_stem,0.0,
		xsect[aft_freeboard_section] - x_stem,0.0);

	balanc(&calcdisp,0);	// balance, don't show progress
	if(calcdisp > 0.0) finish_stats();

//	section count, LOA, distance back to forward end of J (SFJ), stem-to-mast distance (SFBI), but:
//	"The two last fields are set to zero in most files and are not relevant."

	fprintf(orc_fp,"%7d,%7.3f,%7.3f,%7.3f\n",count,loa,0.0,0.0);

//	Scale is mm for metric, 100ths of a foot for lengths in feet

	if(numun <= 1)
		scale = 1000.0;
	else
		scale = 100.0;

	dt = 1.0 / (float) (numbetwl+1);
	tmax = 1.0 - 0.5*dt;

//	Minimal stem

	fprintf(orc_fp,station_fmt,
		0.0,
		2,
		forward_freeboard_section == 0 ? 1 : aft_freeboard_section == 0 ? 2 : 0,
		1);
	if(a > 0.0) {	// normal stem
		fprintf(orc_fp,offset_fmt,-(zline[0][0]*scale+1.0),0.0,0);
		fprintf(orc_fp,offset_fmt,-zline[0][0]*scale,1.0,1);
	} else {	// plumb or zero-sized stem
		zmin = 1.0e+30;
		zmax = -1.0e+30;
		for(j = 0 ; j < numlin ; j++) {
			if(stsec[j] <= 0) {
				if(zmin > zline[j][0]) zmin = zline[j][0];
				if(zmax < zline[j][0]) zmax = zline[j][0];
			}
		}
		fprintf(orc_fp,offset_fmt,-zmax*scale,0.0,0);
		fprintf(orc_fp,offset_fmt,-zmin*scale,0.0,1);
	}

	for(i = 1 ; i < count ; i++) {
		orc_numpoints = 1;
		for(j = 1 ; j < numlin ; j++) {
			if(stsec[j] <= i && ensec[j] >= i) orc_numpoints += numbetwl+1;
		}
		fprintf(orc_fp,station_fmt,
			(xsect[i] - x_stem)*scale,
			orc_numpoints,
			forward_freeboard_section == i ? 1 : aft_freeboard_section == i ? 2 : 0,
			i+1);
		for(j = numlin-1 ; j > 0 ; j--) {
			if(stsec[j] <= i && ensec[j] >= i) {
				getparam(i,j,&a,&hb,&c,&hd);
				for(t = 0.0 ; t < tmax ; t += dt) {
					y = yline[j][i] + t*(a + t*hb);
					z = zline[j][i] - t*(c + t*hd);
					fprintf(orc_fp,offset_fmt,-z*scale,y*scale,0);
				}
			}
		}
		fprintf(orc_fp,offset_fmt,-zline[0][i]*scale,yline[0][i]*scale,1);
	}

/*
   .000,   .000,   .000,   .000,   .000,   .000,   .000
C30012.TP    ,   .000,   .000,   .000,   .000
 30012,        , , 9/19/90, 9/19/90,16:54:33,  1
     0,        , , 0/ 0/ 0, 0/ 0/ 0, 0: 0: 0,  0,  0
  4.500,  3.375,  2.015, 34.699,21, 0,0
*/

//	Rudder data, normally ignored
	fprintf(orc_fp,"   .000,   .000,   .000,   .000,   .000,   .000,   .000\n");

//	Tape filename, centreboard data (normally ignored), measurement date, date and time last written
	fprintf(orc_fp,"%5d.TP    ,   .000,   .000,   .000,   .000\n",measurer_number);

//	Certificate number, class name, modification indicator (ignored)
	fprintf(orc_fp," %5d,        , ,%2d/%02d/%02d,%2d/%02d/%02d,%02d:%02d:%02d,  1\n",measurer_number,
#ifdef linux
		HullFileTime.tm_mday,HullFileTime.tm_mon,HullFileTime.tm_year % 100,
		HullFileTime.tm_mday,HullFileTime.tm_mon,HullFileTime.tm_year % 100,
		HullFileTime.tm_hour,HullFileTime.tm_min,HullFileTime.tm_sec);
#else
		HullFileTime.wDay,HullFileTime.wMonth,HullFileTime.wYear % 100,
		HullFileTime.wDay,HullFileTime.wMonth,HullFileTime.wYear % 100,
		HullFileTime.wHour,HullFileTime.wMinute,HullFileTime.wSecond);
#endif

//	Equivalent sister yacht data
	fprintf(orc_fp,"     0,        , , 0/ 0/ 0, 0/ 0/ 0, 0: 0: 0,  0,  0\n");

	fprintf(orc_fp,"%7.3f,%7.3f,%7.3f,%7.3f,%2d,%2d,%2d\n",
		-zline[0][forward_freeboard_section],
		-zline[0][aft_freeboard_section],
		xsect[forward_freeboard_section] - x_stem,
		xsect[aft_freeboard_section] - xsect[forward_freeboard_section],
		measuring_device_number,0,keel_endplate_flag);

	fprintf(orc_fp,"%c%c%c%c%c%c%c%c",0x1a,0x1a,0x1a,0x1a,0x1a,0x1a,0x1a,0x1a);
	fclose(orc_fp);
#endif

}

#endif

