/* Hullform component - dxf_overlay.c
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
#include <string.h>

#define MAX_OVL 50000
REAL x_ovl[MAX_OVL];
REAL y_ovl[MAX_OVL];
REAL z_ovl[MAX_OVL];
int n_ovl;

char dxfovl[MAX_PATH] = "";
int invert_z = 0;
int reverse_x = 0;
int dxf_overlay = 0;

#define nodata	-1.0e+30

void dxf_overlay_func(int code,HWND hWndDlg)
{
	if(openfile(dxfovl,"rt","DXF Overlay File","DXF files(*.DXF)\0*.dxf\0","*.dxf",dxfdirnam,NULL)) {
#ifdef linux
		XmTextSetString(wEdit[0],dxfovl);
#else
		SetDlgItemText(hWndDlg,DLGEDIT+0,dxfovl);
#endif
	}
}

void read_dxf_overlay()
{
	char text[20];
	int vertex  =  0,polyline  =  0;
	int i,j,n;
	REAL x,y,z;
	REAL x_min,xshift;
	REAL z_min,zshift,z_base;
	FILE *fp;
	REAL xsign,zsign;

//	if(!openfile(dxfovl,"rt","Open a DXF file","DXF files(*.DXF)\0*.dxf\0","*.dxf",dirnam,&fp)) return;

	if(!getdlg(DXF_OVERLAY,
		INP_LOG,(void *) &invert_z,
		INP_LOG,(void *) &reverse_x,
		INP_STR,(void *) dxfovl,
		INP_PBF,dxf_overlay_func,-1)) return;

	xsign = reverse_x ? 1.0 : -1.0;
	zsign = invert_z ? -1.0 : 1.0;
	x = nodata;
	y = nodata;
	z = nodata;
	vertex = 0;
	polyline = 0;
	n_ovl = 0;
	x_min=1.0e+30;
	z_min=1.0e+30;
	fp = fopen(dxfovl,"rt");
	while(fgets(text,sizeof(text),fp) != NULL) {
		if(strncmp(text,"VERTEX",6) == 0) {
			if(z <= nodata) z = 0.0;
			x *= xsign;
			z *= zsign;
			x_ovl[n_ovl] = x;
			if(x < x_min) x_min=x;
			if(z < z_min) z_min=z;
			y_ovl[n_ovl] = y;
			z_ovl[n_ovl]= z;
			if(++n_ovl == MAX_OVL) break;
			x = nodata;
			y = nodata;
			z = nodata;
			vertex = 1;
		} else if(strncmp(text,"POLYLINE",8) == 0) {
			if(z <= nodata) z = 0.0;
			x *= xsign;
			z *= zsign;
			x_ovl[n_ovl] = x;
			if(x < x_min) x_min=x;
			if(z < z_min) z_min=z;
			y_ovl[n_ovl] = y;
			z_ovl[n_ovl] = z;
			if(++n_ovl == MAX_OVL) break;
			x = nodata;
			y = nodata;
			z = nodata;
			vertex = 0;
			polyline = 1;
		} else if(strncmp(text,"SEQEND",6) == 0) {
			x_ovl[n_ovl] = nodata;
			y_ovl[n_ovl] = nodata;
			z_ovl[n_ovl] = nodata;
			if(++n_ovl == MAX_OVL) break;
		} else if(vertex) {
			if(sscanf(text,"%d",&n) == 1) {
				if(n == 10) {
					fscanf(fp,"%f",&x);
				} else if(n == 20) {
					fscanf(fp,"%f",&y);
				} else if(n == 30) {
					fscanf(fp,"%f",&z);
				}
			}
		}
	}

	if(y != nodata && n_ovl < MAX_OVL) {
		if(z <= nodata) z = 0.0;
		x *= xsign;
		z *= zsign;
		x_ovl[n_ovl] = x;
		if(x < x_min) x_min=x;
		if(z < z_min) z_min=z;
		y_ovl[n_ovl] = y;
		z_ovl[n_ovl] = z;
		n_ovl++;
	}
	fclose(fp);
	dxf_overlay = TRUE;
	sprintf(text,"%d points read",n_ovl);
	message(text);

	xshift = xsect[0] - dxstem() - x_min;

	z_base = -1.0e+30;
	for(j = 0 ; j < numlin ; j++) {
		for(i = stsec[j] ; i <= ensec[j] ; i++) {
			if(zline[j][i] > z_base) z_base = zline[j][i];
		}
	}
	zshift = -z_base - z_min;
	for(n = 0 ; n < n_ovl ; n++) {
		x_ovl[n] += xshift;
		z_ovl[n] += zshift;
	}
}



void show_dxf_xz(REAL x_min,REAL xmax,REAL ymin,REAL ymax)
{
	int i;
	(*colour)(9);
	setranges(x_min,xmax,ymin,ymax);
	(*newlin)();
	for(i = 0 ; i < n_ovl ; i++) {
		if(x_ovl[i] > 1.0e+29 || y_ovl[i] < -1.0e+29 || z_ovl[i] < -1.0e+29)
			(*newlin)();
		else
			(*draw)(x_ovl[i],-z_ovl[i]);
	}
}

void show_dxf_xy(REAL x_min,REAL xmax,REAL ymin,REAL ymax)
{
	int i;
	(*colour)(9);
	setranges(x_min,xmax,ymin,ymax);
	(*newlin)();
	for(i = 0 ; i < n_ovl ; i++) {
		if(x_ovl[i] < -1.0e+29 || y_ovl[i] < -1.0e+29 || z_ovl[i] < -1.0e+29)
			(*newlin)();
		else
			(*draw)(x_ovl[i],y_ovl[i]);
	}
}

void show_dxf_yz(REAL x_min,REAL xmax,REAL ymin,REAL ymax)
{
	int i;
	(*colour)(9);
	setranges(x_min,xmax,ymin,ymax);
	(*newlin)();
	for(i = 0 ; i < n_ovl ; i++) {
		if(x_ovl[i] > 1.0e+29)
			(*newlin)();
		else
			(*draw)(y_ovl[i],-z_ovl[i]);
	}
}

void show_dxf_xyz(REAL x_min,REAL xmax,REAL ymin,REAL ymax)
{
	int i;
	extern REAL zpersp;

	(*colour)(9);
	(*newlin)();

	for(i = 0 ; i < n_ovl ; i++) {
		if(x_ovl[i] < -1.0e+29 || y_ovl[i] < -1.0e+29 || z_ovl[i] < -1.0e+29) {
			(*newlin)();
		} else {
			zpersp = -x_ovl[i];
			(*draw)(y_ovl[i],-z_ovl[i]);
		}
	}
}



