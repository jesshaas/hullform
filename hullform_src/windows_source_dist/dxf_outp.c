/* Hullform component - dxf_outp.c
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
 
/*    AutoCAD "DXF"-format output of hull data	    */

#include "hulldesi.h"

#ifdef EXT_OR_PROF

#ifdef linux
char *searchpath(char *file);
#include <Xm/Text.h>
#endif

FILE	*fp,*hp;
char	layer[5];		/* layer name string	*/
REAL	addx,addy,addz;
int	numpts;				/* number of polyline points */
int	polyline_active = 0;
int	dxf_layer_numeric = 1;		/* numeric layer labels */
int	dxf_maxlayer = 1;	/* default single layer	*/
long	fpos;			/* output file position */

#ifdef PROF

void make_outline(int i,REAL *tn,REAL del,int mode,FILE *fp,REAL side);
void write_blockpoint(char *name,REAL x,REAL y);
int	dxf_dim = 0;		/* default 0 = 2-d, 1 = 3d */
extern	REAL *outl_thickness;
extern	REAL mousehole_radius,notch_width,notch_height;
char dxf_filename[MAX_PATH] = "hullform.dxf";

int show_waterlines = 0;
extern int start_at_waterline;
int show_buttocks = 0;
int show_diagonals = 0;
REAL waterl_interval = 1.0;
REAL buttoc_interval = 1.0;
REAL diagon_interval = 1.0;
extern REAL diag_angle;

MENUFUNC dxf_output()
{
	extern int	numbetw,numbetwl;
	int		i,j;
	char	line[80];
	char	*p1;
	REAL	a,hb,c,hd,tt,dt;
	int		jp;
	REAL	t;
	int		ignore;
	REAL	x;
	REAL	xshift,yshift;
	REAL	side;
#ifdef PLATEDEV
	REAL	t1;
	int		k,l;
	int		blocks;
#endif
	REAL	y;
	REAL	dummy[maxsec*maxext];
	int		secmode;
	extern int	numbetwl;
	void	def_dxf(int,HWND);
	REAL	maxoff,minoff;
	REAL	hull_addx;

	if(count <= 0) return;

	strcpy(dxf_filename,dxfdirnam);
#ifdef linux
	strcat(dxf_filename,"/");
#else
	strcat(dxf_filename,"\\");
#endif
	strcat(dxf_filename,port);

	update_func = NULL;
	cls(0);
	if(getdlg(DXFOUTP,
	INP_INT,&dxf_maxlayer,
	INP_RBN,&dxf_layer_numeric,
	INP_RBN,&dxf_dim,
#ifdef PROF
	INP_LOG,(void *) &showtanks,
	INP_LOG,(void *) &showstringers,
#endif
	INP_LOG,(void *) &showframes,
	INP_LOG,(void *) &show_waterlines,
	INP_REA,(void *) &waterl_interval,
	INP_LOG,(void *) &start_at_waterline,
	INP_LOG,(void *) &show_buttocks,
	INP_REA,(void *) &buttoc_interval,
	INP_LOG,(void *) &show_diagonals,
	INP_REA,(void *) &diagon_interval,
	INP_REA,(void *) &diag_angle,
	INP_PBF,def_dxf,
	INP_STR,dxf_filename,
	-1)) {
		if(dxf_maxlayer < 1) dxf_maxlayer = 1;

		/*    Find shift for placement on page	    */

		xshift = 0.0;
		addz = -1.0e+30;
		for(j = 0 ; j < numlin ; j++) {
			for(i = max(1,stsec[j]) ; i <= ensec[j] ; i++) {
				if(yline[j][i] > xshift) xshift = yline[j][i];
				if(zline[j][i] > addz  ) addz   = zline[j][i];
			}
		}
		xshift *= 2.0;	/* convert to full breadth */

#ifdef DEMO
		message("Demonstration version:\nno actual output");
#else

		/*    perform output					*/

		if( (fp = fopen(dxf_filename,"wt")) == NULL) {
			message("Can not open DXF output file for writing");
			return;
		}

		fpos = 0;
		numpts = 0;

		waitcursor();

		if(dxf_dim == 0) {
			secmode = 4;
		}
		else {
			secmode = 7;
		}

		/*	Header lines		*/

		fputs("0\nSECTION\n2\nHEADER\n",fp);

		hp = fopen("header.dxf","rt");
		if(hp == NULL) {
			p1 = searchpath("header.dxf");
			if(p1 != NULL) hp = fopen(p1,"rt");
		}
		if(hp != NULL) {
			while(fgets(line,80,hp) != NULL) fputs(line,fp);
			fclose(hp);
		}

		fputs("0\nENDSEC\n",fp);

		/*	Tables						*/

		fputs("0\nSECTION\n2\nTABLES\n",fp);

		hp = fopen("tables.dxf","rt");
		if(hp == NULL) {
			p1 = searchpath("tables.dxf");
			if(p1 != NULL) hp = fopen(p1,"rt");
		}
		if(hp != NULL) {
			while(fgets(line,80,hp) != NULL) fputs(line,fp);
			fclose(hp);
		}

		if(dxf_layer_numeric) {		/* if numeric layers, define layers from 0 up to maxmum allowed */
			fprintf(fp,"0\nTABLE\n2\nLAYER\n70\n%d\n",dxf_maxlayer);
			for(i = 0 ; i <= dxf_maxlayer ; i++)
				fprintf(fp,"0\nLAYER\n2\n%d\n70\n0\n62\n%d\n6\nCONTINUOUS\n",i,i % 15 + 1);	/* second argument is colour number */
		}
		else {		/* if alphanumeric, define layers for each section and line, the transom and development marks */
#ifdef PROF
			fprintf(fp,"0\nTABLE\n2\nLAYER\n70\n%d\n",count+numlin+transom);
#else
			fprintf(fp,"0\nTABLE\n2\nLAYER\n70\n%d\n",count+numlin);
#endif
			for(i = 1 ; i < count ; i++)
				fprintf(fp,"0\nLAYER\n2\nS%d\n70\n0\n62\n%d\n6\nCONTINUOUS\n",i,(i-1) % 15 + 1);
			for(i = 0 ; i <= numlin ; i++)
				fprintf(fp,"0\nLAYER\n2\nL%d\n70\n0\n62\n%d\n6\nCONTINUOUS\n",i,i % 15 + 1);
#ifdef PROF
			if(transom)
				fprintf(fp,"0\nLAYER\n2\nT0\n70\n0\n62\n1\n6\nCONTINUOUS\n");
#endif

//	Development mark layer is "DM"

#ifdef PLATEDEV
			fprintf(fp,"0\nLAYER\n2\nDM\n70\n0\n62\n1\n6\nCONTINUOUS\n");
#endif
		}
		fputs("0\nENDTAB\n0\nENDSEC\n",fp);

#ifdef PLATEDEV
		/*    development point output - definition of mark	    */

#ifdef PROF
		blocks = (ststr != NULL && ststr[0] >= 0) || numruled > 0;
#else
		blocks = numruled > 0;
#endif

		if(blocks) {
			fprintf(fp,"  0\nSECTION\n");	/* begin section    */
			fprintf(fp,"  2\nBLOCKS\n");	/* blocks    */
		}

		if(numruled > 0) {
			a = xshift * 0.025;			/* cross size 5% of beam */
			fprintf(fp,"  0\nBLOCK\n  8\n0\n  2\nDM\n 70\n     0\n");
			fprintf(fp," 10\n%7.3f\n 20\n%7.3f\n",a,a);				/* centre position */
			fprintf(fp,"  0\nLINE\n  8\n0\n");						/* first line */
			fprintf(fp," 10\n%7.3f\n 20\n%7.3f\n 11\n%7.3f\n 21\n%7.3f\n",a,0.0,a,a+a);
			fprintf(fp,"  0\nLINE\n  8\n0\n");						/* second line */
			fprintf(fp," 10\n%7.3f\n 20\n%7.3f\n 11\n%7.3f\n 21\n%7.3f\n",0.0,a,a+a,a);
			fprintf(fp,"  0\nENDBLK\n");
		}

		/*    stringer mark output - definition of mark	    */

#ifdef PROF
		if(showstringers && (ststr != NULL) && ststr[0] >= 0) {
			fprintf(fp,"  0\nBLOCK\n  8\nSM\n  2\nSM\n 70\n     0\n");
			a = xshift * 0.025;			/* size 5% of beam */
			fprintf(fp," 10\n%7.3f\n 20\n%7.3f\n",a,a);				/* centre position */
			fprintf(fp,"  0\nLINE\n  8\nSM\n");						/* first line */
			fprintf(fp," 10\n%7.3f\n 20\n%7.3f\n 11\n%7.3f\n 21\n%7.3f\n",0.0,0.0,a+a,a+a);
			fprintf(fp,"  0\nLINE\n  8\nSM\n");						/* second line */
			fprintf(fp," 10\n%7.3f\n 20\n%7.3f\n 11\n%7.3f\n 21\n%7.3f\n",0.0,a+a,a+a,0.0);
			fprintf(fp,"  0\nENDBLK\n  8\nSM\n");
		}
		if(blocks) fprintf(fp,"  0\nENDSEC\n");/* end block */
#endif

#endif

		fprintf(fp,"  0\nSECTION\n  2\nENTITIES\n");
		/* begin entities section */

		/******* plan view *************/

		/*    Hull planform lines are 2-D POLYLINEs in layers "1"	*/
		/*    to "numlin"						*/

		if(dxf_dim == 0) {
			addx = (float) (((int)(xshift * 2.0)) + 2);
			if(posdir < 0.0) {	/* xshift starts as breadth */
				addx -= (xsect[0] - dxstem());
			}
			else {
				addx += xsect[count-1];
			}
			addy = ((int) xshift)/2 + 1;

			for(i = 1 ; i < count ; i++) {	/* do not do stem */
				x = 0.0;
				for(j = 0 ; j < numlin ; j++) {
					if(stsec[j] <= i && ensec[j] >= i) {
						if(yline[j][i] > x) x = yline[j][i];
					}
				}

				/*    Write out sections in plan view		    */

				ensure_polyline('S',i,dxf_dim);
				write_xy(xsect[i],0.0);
				write_xy(xsect[i],x);
				seqend();/* end line sequence*/
			}	/* end loop through plan views of seections */

			/*	write out lines in plan view			*/

			for(j = 0 ; j < numlin ; j++) {    /* For each hull line    */
				ensure_polyline('L',j+1,dxf_dim);
				draw_line(j,1.0,1.0,5,stsec[j],ensec[j],999,dummy,dummy,dummy,&ignore,&ignore,&ignore,1);
				seqend();/* end line sequence    */
			}	/* end loop through plan views of lines */

#ifdef PROF
			draw_stringers(1.0,1.0,5,999);
#endif

			/*    draw centreline	*/

			ensure_polyline('L',1,dxf_dim);
			write_xy(xsect[0]-dxstem(),0.0);
			write_xy(xsect[count-1],0.0);
			seqend();    /* end line sequence*/

#ifdef PROF
			/*    draw transom	*/

			if(transom) {
				ensure_polyline('T',1,dxf_dim);
				draw_transom(1.0,1.0,5);
				seqend();    /* end line sequence*/
			}
#endif

			/*    draw stems	*/

			if(!surfacemode) {
				t = yline[stemli][1];
				x = xsect[0]-dxstem();
				ensure_polyline('L',stemli+1,dxf_dim);
				write_xy(x,t);
				write_xy(xsect[0],t);
				seqend();    /* end line sequence*/

				if(t != 0.0) {
					ensure_polyline('L',stemli+1,dxf_dim);
					write_xy(x,-t);
					write_xy(xsect[0],-t);
					seqend();    /* end line sequence*/
				}
			}

			/****** side elevation *********/

			/*    Hull elevation lines are 3-D POLYLINEs in layers "EL1"	*/
			/*    to "ELnumlin"						*/

			yshift = 0.0;
			for(j = 0 ; j < numlin ; j++) {
				for(i = stsec[j] ; i <= ensec[j] ; i++) {
					if(zline[j][i] > yshift) yshift = zline[j][i];
				}
			}

			addy = (int) (xshift * 2.0 + yshift) + 1;

			for(i = 1 ; i < count ; i++) {
				x = -1.0e+30;
				t = 1.0e+30;
				for(j = 0 ; j < numlin ; j++) {
					if(stsec[j] <= i && ensec[j] >= i) {
						if(zline[j][i] > x) x = zline[j][i];
						if(zline[j][i] < t) t = zline[j][i];
					}
				}

				/*    Write out sections in elevation view		*/

				ensure_polyline('S',i,dxf_dim);
				write_xy(xsect[i],-t);
				write_xy(xsect[i],-x);
				seqend();/* end line sequence*/
			}	/* end loop through side elevation views of sections */

			for(j = 0 ; j < numlin ; j++) {    /* For each hull line    */
				ensure_polyline('L',j+1,dxf_dim);
				draw_line(j,1.0,1.0,6,stsec[j],ensec[j],999,dummy,dummy,dummy,&ignore,&ignore,&ignore,1);
				seqend();/* end line sequence    */
			}	/* end loop through side elevation views of lines */

#ifdef PROF
			draw_stringers(1.0,1.0,6,999);

			/*    draw transom	*/

			if(transom) {
				ensure_polyline('T',1,dxf_dim);
				draw_transom(-1.0,-1.0,6);
				seqend();    /* end line sequence*/
			}
#endif

			/*	Add the stem section		*/

			if(!surfacemode) {
				if(fneg(posdir)){	/* xshift starts as breadth */
					addx += xsect[0];
				}
				else {
					addx -= xsect[0];
				}
				ensure_polyline('L',stemli+1,dxf_dim);
				drasec(0,0,-1.0,4);
				seqend();
			}

			/*    Hull sections 0 to count-1 are POLYLINEs in layers    */
			/*    S0 to S(count-1)		    */

			addx = (int) (xshift) + 1;

		}
		else {

			/*    3-D options		    */

			if(posdir < 0.0) {
				addx = dxstem() - xsect[0];
			}
			else {
				addx = xsect[count-1];
			}
		}
		hull_addx = addx;

		/*    End elevation also is used to output 3-d information	*/

		/*    Draw fore-and-aft lines first, to detect transom form	*/

		for(j = 0 ; j < numlin ; j++) {    /* For each hull line    */
			ensure_polyline('L',j+1,dxf_dim);
			draw_line(j,-1.0,-1.0,secmode,stsec[j],ensec[j],999,dummy,dummy,dummy,&ignore,&ignore,&ignore,1);
			seqend();/* end line sequence    */
			ensure_polyline('L',j+1,dxf_dim);
			draw_line(j,1.0,1.0,secmode,stsec[j],ensec[j],999,dummy,dummy,dummy,&ignore,&ignore,&ignore,1);
			seqend();/* end line sequence    */
		}	/* end loop through end elevation/perspective views of lines */

#ifdef PROF
		draw_stringers(-1.0,-1.0,secmode,999);
		draw_stringers( 1.0, 1.0,secmode,999);
#endif

		/*    Draw sections next, including development marks and transom effect */

		t = addx;

		if(dxf_dim == 1) {
			dt = 1.0/(numbetwl+1);
			side = -1.0;
			do {
				ensure_polyline('S',0,dxf_dim);
				side = -side;	// positive first time, negative second time
				jp = 0;
				while(stsec[jp] > 0 && jp < numlin) jp++;
				for(j = jp+1 ; j < numlin ; j++) {
					if(stsec[j] <= 0) {
						getparam(0,j,&a,&hb,&c,&hd);
						write_xyz(xsect[0]-yline[jp][0],side*yline[stemli][1],zline[jp][0]);
						if(zcont[j][0] != zline[jp][0] || ycont[j][0] != (relcont[j] ? 0.0 : yline[jp][0]) ) {
							tt = 1.0;
							for(i = 1 ; i < numbetwl ; i++) {
								tt -= dt;
								write_xyz(xsect[0]-yline[j][0]-tt*(a+tt*hb),side*yline[stemli][1],zline[j][0]-tt*(c+tt*hd));
							}
						}
						jp = j;
					}
					write_xyz(xsect[0]-yline[jp][0],side*yline[stemli][1],zline[jp][0]);
				}
				seqend();
			} while(side == 1.0 && yline[stemli][1] > 0.0);
		}

		for(i = 1 ; i < count ; i++) {

			if(dxf_dim == 1) addx = t - posdir*xsect[i];
			ensure_polyline('S',i,dxf_dim);
			drasec(i,1,1.0,secmode);
			seqend();

			if(showframes
#ifndef STUDENT
			    || showstringers
#endif
			    ) {
				ensure_polyline('S',i,dxf_dim);
				make_outline(i,outl_thickness,0.0,secmode,fp,1.0);
				seqend();
			}

			ensure_polyline('S',i,dxf_dim);
			drasec(i,-1,-1.0,secmode);
			seqend();

#ifdef PLATEDEV
			/*    Now output any development intersections	*/

			if(numruled > 0) {
				write_blockpoint("DM",yline[0][i],zline[0][i]);
				for(j = 1 ; j < numlin ; j++) {
					if(stsec[j] > i || ensec[j] < i) continue;
					write_blockpoint("DM",yline[j][i],zline[j][i]);

					if((k = developed[j]) >= 0) {

						/*    ... i.e., if this surface is developed ...	*/

						for(l = 0 ; l < rulings[j] ; l++) {

							/*    ... find positions (x,y) of intersections with this section    */

							x = xstart[k][l] - xsect[i];
							y = xend  [k][l] - xsect[i];
							if( (x > 0.0) != (y > 0.0)) {
								t1 = y / (y - x);
								x = yend[k][l]+t1*(ystart[k][l]-yend[k][l]);
								y = zend[k][l]+t1*(zstart[k][l]-zend[k][l]);
								write_blockpoint("DM",-posdir*x,y);
							}
						}/* end search through all rulings */
					}/* end developed-surface conditional */
				}/* end loop through hull lines */
			}/* end "any rulings conditional */

			/*    Now output any stringer intersections	*/

#ifdef PROF
			if(showstringers && ststr != NULL) {
				for(j = 1 ; j < numlin ; j++) {
					for(k = inistr[j] ; k < inistr[j] + numstr[j] ; k++) {
						if(i >= ststr[k] && i <= enstr[k])
							write_blockpoint("SM",ystr[k][i],zstr[k][i]);
					}
				}
			}
#endif

#endif

		}/* end loop through end-elevation sections */

#ifdef PROF
		if(transom) {
			ensure_polyline('T',1,dxf_dim);
			addx = t;
			if(dxf_dim == 0)
				draw_transom(-1.0,-1.0,4);
			else
			    draw_transom(1.0,1.0,7);
			seqend();    /* end line sequence*/
		}
#endif

		addx = hull_addx;

		if(show_waterlines) {
			offset_range(&maxoff,&minoff,0.0,waterl_interval);
			y = start_at_waterline ? wl : waterl_interval * ((int) (minoff/waterl_interval));
			plotw(-1.0,y,waterl_interval,secmode);
		}

		if(show_diagonals) {
			pltdia(diagon_interval,diag_angle,secmode,-1.0);
		}

		if(show_buttocks) {
			pltdia(buttoc_interval,90.0,secmode,-1.0);
		}

		/*    End of output		    */

		fprintf(fp,"  0\nENDSEC\n  0\nEOF\n");

		fclose(fp);
#endif
		arrowcursor();
	}
}

void write_blockpoint(char *name,REAL x,REAL y)
{
	fprintf(fp,"  0\nINSERT\n");
	fprintf(fp,"  8\n%s\n",layer);	/* layer name (Use current layer)    */
	fprintf(fp,"  2\n%s\n",name);	/* block mark name */
	if(dxf_dim == 0) {		// 2D
		fprintf(fp," 10\n%7.3f\n 20\n%7.3f\n",addx-posdir*x,addy-y);    /* position */
	} else {				// 3D
		fprintf(fp," 10\n%7.3f\n 20\n%7.3f\n 30\n%7.3f\n",addx,addy-x,addz-y);
	}
	numpts = 4;
}

#endif

/*    Write x and y coordinates to DXF file in standard format    */

void write_xy(REAL x,REAL y)
{
	fprintf(fp,"  0\nVERTEX\n  8\n%s\n 10\n%-9.4f\n 20\n%-9.4f\n",
		layer,addx - posdir*x,addy+y);
	numpts++;
}

/*    Write x, y and z coordinates to DXF file in standard format    */

void write_xyz(REAL x,REAL y,REAL z)
{
	fprintf(fp,
		"  0\nVERTEX\n  8\n0\n 10\n%-9.4f\n 20\n%-9.4f\n 30\n%-9.4f\n 70\n 32\n",
		addx - posdir*x,addy+y,addz-z);
	numpts++;
}

/*    Write polyline header	    */

void ensure_polyline(char title,int index,int dim)
{
	int lim_index;
	if(!polyline_active) {
		if(dxf_layer_numeric == 0) {
			sprintf(layer,"%c%d",title,index);
		}
		else {
			lim_index = (index-1) % dxf_maxlayer + 1;
			if(lim_index < 1) lim_index = 1;
			sprintf(layer,"%d",lim_index);
		}
		fpos = ftell(fp);
		numpts = 0;
		if(dim) {
			fprintf(fp,"  0\nPOLYLINE\n  8\n%s\n 62\n  0\n  6\nCONTINUOUS\n 66\n  1\n 70\n  8\n",layer);
		}
		else {
			fprintf(fp,"  0\nPOLYLINE\n  8\n%s\n 66\n  1\n",layer);
		}
		polyline_active = 1;
	}
}

void seqend()
{
	if(polyline_active) {
		if(numpts > 1) {
			fprintf(fp,"  0\nSEQEND\n");    /* end line sequence*/
		}
		else {
			fseek(fp,fpos,0);
		}
		polyline_active = 0;
	}
}

#endif

void def_dxf(int code,HWND hWndDlg)
{
	void	def_builders(int,HWND);
	char path[MAX_PATH];
	switch (code) {
	case 0:
		def_builders(code,hWndDlg);
		break;

	case 1:
		if(openfile(path,"wt","Specify DXF Output File","DXF Files (*.dxf)\0*.dxf\0All files\0*.*\0","*.dxf",dxfdirnam,NULL)) {
#ifdef linux
			XmTextSetString(wEdit[1],path);
#else
			SetDlgItemText(hWndDlg,DLGEDIT+5,path);
#endif
		}
		break;
	}
}


