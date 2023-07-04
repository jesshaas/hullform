/* Hullform component - dxf_mesh.c
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
char *searchpath(char *file);
#include <Xm/Text.h>
#endif

extern char dxf_filename[];	// defined in dxf_outp.c

REAL dxf_x_offset,dxf_y_offset,dxf_z_offset;
int vertex_index,vertex_count;

void write_meshpoint(FILE *fp,REAL x,REAL y,REAL z)
{
	fprintf(fp,"  0\nVERTEX\n  8\n1\n 10\n%-9.4f\n 20\n%-9.4f\n 30\n%-9.4f\n 70\n192\n",
		x - dxf_x_offset,y - dxf_y_offset,dxf_z_offset - z);
}

MENUFUNC dxf_mesh(void)
{
	extern int numbetwl;
	extern COLORREF scrcolour[];
	FILE *fp,*hp;
	REAL a,hb,c,hd;
	REAL t,th;
	REAL floatn = (float) (numbetwl+1);
	REAL dt = 1.0 / floatn;
	REAL tend;
	REAL frac0,frac1,frac,dfrac;
	int i,i0,i1,ii,itran;
	int j,jj,k,j0,j1,j2;
	int it,it_change1,it_change0,it_change;
	int sternflat = FALSE;
	REAL xe;
	REAL xt,yt,zt;
	REAL xc,yc,zc;
	REAL yp,zp,z1;
	REAL x0,x1;
	REAL r0,r1;
	REAL c0,c1,cc;
	REAL th0,th1,dth;
	REAL x11,y11,z11,t0,t1,cos0,cos1,sin0,sin1;
	REAL ystem = yline[stemli][1];
	REAL zslope,zslope0,zslope1;
	int lastline = showtanks ? extlin : numlin;
	int stemcurve,had_stemcurve;
	REAL r,g,b;
	REAL x,y,z;
	REAL side;
	REAL dt1;
	char *sh;
	extern int numtran;
	char *p1;
	char line[200];
	int init;
	int sections_drawn[maxsec];

	if(count < 2) return;

	if(!open_text(&fp,dxf_filename,"*.dxf")) return;

#ifdef DEMO

	message("Demonstration version:\nNo actual output");

#else

//	DXF header section

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

//	DXF tables section

	fputs("0\nSECTION\n2\nTABLES\n",fp);
	hp = fopen("tables.dxf","rt");
	if(hp == NULL) {
		p1 = searchpath("tables.dxf");
		if(p1 != NULL) hp = fopen(p1,"rt");
	}
	if(hp != NULL) {
		while(fgets(line,80,hp) != NULL) fputs(line,fp);
		fclose(hp);
	} else {

//	Only one layer. layer 1

		fprintf(fp,"0\nTABLE\n2\nLAYER\n70\n1\n");
		fprintf(fp,"0\nLAYER\n2\n1\n70\n0\n62\n1\n6\nCONTINUOUS\n");
		fputs("0\nENDTAB\n",fp);
	}
	fputs("0\nENDSEC\n",fp);

//	Begin entities section

	fprintf(fp,"0\nSECTION\n2\nENTITIES\n");

	dxf_z_offset = dxf_y_offset = 0.0;
	dxf_x_offset = xsect[0] - dxstem();
	for(i = 0 ; i < count ; i++) {
		if(xsect[i] < dxf_x_offset) dxf_x_offset = xsect[i];
		for(j = 0 ; j < extlin ; j++) {
			if(yline[j][i] > dxf_y_offset) dxf_y_offset = yline[j][i];
			if(zline[j][i] > dxf_z_offset) dxf_z_offset = zline[j][i];
		}
	}
	dxf_y_offset = - dxf_y_offset;

//	Output one surface on each side per adjacent pair of lines

	for(side = 1.0 ; side > -1.5 ; side -= 2.0) {
		for(j = 1 ; j < numlin ; j++) {
			vertex_index = 0;

// Start DXF mesh for this line pair and this hull side

			init = 1;

//	Find the first line "above" the first section of this line

			j0 = j - 1;
			while((stsec[j0] > stsec[j] || ensec[j0] < stsec[j]) && j0 > 0) j0--;
			for(i = stsec[j] ; i <= ensec[j] ; i++) {
				j1 = j0;	// index of line "above" the current line for the preceding section
				getparam(i,j,&a,&hb,&c,&hd);

//	j0 is the line immediately "above" the current line at this section

				j0 = j - 1;
				while((stsec[j0] > i || ensec[j0] < i) && j0 > 0) j0--;
				it_change0 = j - j0;
				it_change1 = j - j1;

//	Determine whether we need to make provision for a radiused stem

				stemcurve = (i == stsec[j0] && radstem[j0] > 0.0) || (i == stsec[j1] && radstem[j1] > 0.0);
				if(init) {
					init = 0;
					vertex_count = 0;
					had_stemcurve = stemcurve;
					if(stemcurve) {
						ii = 0;
						jj = 0;
					} else {
						ii = (numbetwl+1)*(numbetwl+2);
						jj = (numbetwl+1)*(numbetwl+1);
					}
					fprintf(fp,"  0\nPOLYLINE\n5\n41\n  8\n0\n66\n1\n 10\n0.0\n 20\n0.0\n 30\n0.0\n 70\n 64\n 71\n %d\n 72\n %d\n",
					(count+1)*(numbetwl+2)+ii,count*(numbetwl+1)+jj);
				}
				if(stemcurve) {
					frac0 = (REAL)(j-1 - j0)/(REAL)(j - j0);
					frac1 = frac0 + 1.0/(REAL)(j - j0);
					dfrac = dt/(REAL)(j - j0);

					r0 = i0 ? radstem[j0] : 0.0;		/* radius of line at stem */
					r1 = i1 ? radstem[j ] : 0.0;			/* radius of line at stem */
					xt = xsect[i+1];	 				/* position of nearest transverse section */
					x0 = x1 = xsect[i];
					if(!surfacemode && i == 0) {
						x0 -= yline[j0][0];		// x-end of line j0
						x1 -= yline[j ][0];		// x-end of line j
						if(xt == x0) x0 -= 0.0001;
						if(xt == x1) x1 -= 0.0001;
					}

					get_int_angle(j0,&cos0,&sin0);
					get_int_angle(j ,&cos1,&sin1);
					th0 = atan2(sin0,cos0);
					th1 = atan2(sin1,cos1);
					dth = (th1 - th0)*dt;
					zslope0 = (zline[j0][i+1] - zline[j0][i])/(xt-x0);
					zslope1 = (zline[j ][i+1] - zline[j ][i])/(xt-x1);
					dfrac = (frac1 - frac0)*dt;

//	The standard face set comprises points at each section, stepping down from line j-1 to j.
//	Generate the same structure here

					for(ii = 0 ; ii <= numbetwl+1 ; ii++) {	// this steps around the stem curve
						frac = frac0;
						for(jj = 0 ; jj <= numbetwl+1 ; jj++) {	// this steps down between the lines
							t = 1.0 - frac;
							r = r0 + frac*(r1 - r0);
							zslope = zslope0 + frac*(zslope1 - zslope0);
							th = ii*dt*(th0 + frac*(th1-th0));
							xt = t*(c + t*hd);
							z11 = zline[j][i] - xt;
							r = r1 - xt*(r1 - r0) / (c+hd);
							cc = xsect[i] - yline[j][i] - t*(a + t*hb) + r;
							x11 = cc - r*cos(th);
							y11 = ystem + r*sin(th);

							write_meshpoint(fp,x11,side*y11,z11);
							vertex_count++;

							frac += dfrac;		// increment interpolation between lines
						}
					}
					i = 1;	// stem done  - start at section 1
				}

//	If the nearest line has changed, we will need to draw two curves, one representing the surface
//	between this line and that which was previously the nearest, and one for the surface between
//	this line and the one that is now the nearest

				sections_drawn[i] = 1;
				if(j0 != j1) {
					if(j0 > j1) {
						ii = i;
						jj = j1;
						it = (numbetwl+1)*j0;	// transom table index at top of curve to be drawn
						it_change = it_change0;
					} else {
						ii = i-1;
						jj = j0;
						it = (numbetwl+1)*j1;
						it_change = it_change1;
					}
					sections_drawn[ii] = 2;

// start of line j0 at this section - draw section outline from line j1 to line j, using numbetwl points between the two lines

					dt1 = dt*(j - jj);
					t = 1.0;
					tend = -0.5*dt1;
					for(j2 = jj+1 ; j2 <= j ; j2++) {
						getparam(ii,j2,&a,&hb,&c,&hd);
						while(t > tend) {
							x = xsect[ii];
							if(x > xtran[it]) {	// this point on the line is aft of the transom surface
								x = xtran[it];
								y = ytran[it];
								z = ztran[it];
							} else {
								y = yline[j2][ii]+t*(a+t*hb);
								z = zline[j2][ii]-t*(c+t*hd);
							}
							if(i > 0)
								write_meshpoint(fp,x,side*y,z);
							else	// stem
								write_meshpoint(fp,x-y,side*yline[stemli][1],z);
							vertex_count++;
							t -= dt1;
							it += it_change;
						}
						t += 1.0;
					}
				}

				it = (numbetwl+1)*j0;	// transom table index at top of curve to be drawn
				dt1 = dt;
				t = 1.0;
				tend = -0.5*dt1;
				j2 = j;
				getparam(i,j2,&a,&hb,&c,&hd);
				while(t > tend) {
					x = xsect[i];
					if(x > xtran[it]) {	// this point on the line is aft of the transom surface
						x = xtran[it];
						y = ytran[it];
						z = ztran[it];
					} else {
						y = yline[j2][i]+t*(a+t*hb);
						z = zline[j2][i]-t*(c+t*hd);
					}
					if(i > 0)
						write_meshpoint(fp,x,side*y,z);
					else	// stem
						write_meshpoint(fp,x-y,side*yline[stemli][1],z);
					vertex_count++;
					t -= dt1;
					it += it_change0;
				}
			}

//	Write out face indices

			vertex_index = 0;
			if(had_stemcurve) {
				for(ii = 0 ; ii <= numbetwl ; ii++) {		// this steps around the stem curve
					for(jj = 0 ; jj <= numbetwl ; jj++) {	// this steps down between the lines
						vertex_index++;
						fprintf(fp,"0\nVERTEX\n8\n0\n10\n0\n20\n0\n30\n0\n70\n128\n71\n%d\n72\n%d\n73\n%d\n74\n%d\n",
						vertex_index,vertex_index+numbetwl+2,vertex_index+numbetwl+3,vertex_index+1);
					}
					vertex_index++;
				}
			}
			for(i = stsec[j]+1 ; i <= ensec[j] ; i++) {
				for(jj = 0 ; jj <= numbetwl ; jj++) {	// this steps down between the lines
					vertex_index++;
					fprintf(fp,"0\nVERTEX\n8\n0\n10\n0\n20\n0\n30\n0\n70\n128\n71\n%d\n72\n%d\n73\n%d\n74\n%d\n",
					vertex_index,vertex_index+numbetwl+2,vertex_index+numbetwl+3,vertex_index+1);
				}
				if(sections_drawn[i] > 1)
					vertex_index += (numbetwl+3);
				else
					vertex_index++;
			}
			fprintf(fp,"  0\nSEQEND\n");    /* end line sequence*/
			init = 1;
		}
	}

//	Draw transom. Transom is calculated from sheerline to keel.

	vertex_index = 0;
	for(side = 1.0 ; side > -1.5 ; side -= 2.0) {

//	Draw the sloping transom surface first

		itran = 0;
		if(transom) {
			fprintf(fp,"  0\nPOLYLINE\n5\n41\n  8\n0\n66\n1\n 10\n0.0\n 20\n0.0\n 30\n0.0\n 70\n 64\n 71\n %d\n 72\n %d\n",1,1);
			xe = xsect[count-1];
			x0 = xe - (ztransom - ztran[0]) * stransom / ctransom;	// x at centreline where transom meets deck
			vertex_count = 0;
			for(it = 0 ; it < numtran ; it++) {	// for each transom point
				xt = xtran[it];
				yt = ytran[it];
				zt = ztran[it];
				r = (xt - xe)/(xtran[0] - xe);
				x1 = xe + r*(x0 - xe);	// centreline x for this curve
				z1 = ztransom + r*(ztran[0] - ztransom);
				y = yt - ystem;
				xc = rtransom*rtransom - y*y;
				if(xc >= 0.0)
					xc = rtransom - sqrt(xc);
				else
					xc = 0.0;
				for(t = 1.0 ; t >= tend ; t -= dt) {	// step inwards to centreline
					y = (yt - ystem)*t;		// remember to add ystem when plotting
					yc = rtransom*rtransom - y*y;
					if(yc >= 0.0 && stransom != 0.0) {
						yc = rtransom - sqrt(yc);	// yc becomes the distance of the point down from the centrelien peak, perpendicular to the transom slope
						if(xc != 0.0) {
							x = x1 + yc / xc * (xt - x1);
							z = z1 + yc / xc * (zt - z1);
						} else {
							x = x1;
							z = z1;
						}
					} else {			// normally the alternative is a flat transon (rtransom 'zero')
						x = xt;
						z = zt;
					}
					write_meshpoint(fp,x,side*(y+ystem),z);
					vertex_count++;
				}
				if(xt >= xe) {
					if(itran = 0) itran = it;
					break;	// at transom flat
				}
			}

//	Write out the vertex indices

			ii = numbetwl+1;
			i0 = 1;
			i1 = vertex_count - (ii + 1);
			for(i = 2 ; i <= i1 ; i++) {
				fprintf(fp,"0\nVERTEX\n8\n0\n10\n0\n20\n0\n30\n0\n70\n128\n71\n%d\n72\n%d\n73\n%d\n74\n%d\n",i,i-1,i+ii,i+ii+1);
				if(i0++ == ii) {
					i0 = 1;
					i++;	// skip this one, not used
				}
			}
			fprintf(fp,"  0\nSEQEND\n");    /* end line sequence*/
		}

//	Draw the stern flat next

		fprintf(fp,"  0\nPOLYLINE\n5\n41\n  8\n0\n66\n1\n 10\n0.0\n 20\n0.0\n 30\n0.0\n 70\n 64\n 71\n %d\n 72\n %d\n",1,1);
		vertex_count = 0;
		x = xsect[count-1];	// stern value
		t = 0.0;
		for(j = numlin-1 ; j > 0 ; j--) {
			if(ensec[j] >= count-1) {
				getparam(count-1,j,&a,&hb,&c,&hd);
				while(t < 1.0 - tend) {
					y = yline[j][count-1]+t*(a+t*hb);
					z = zline[j][count-1]-t*(c+t*hd);
					if(stransom == 0.0)
						zc = zline[0][count-1] - 0.0001;
					else if(rtransom == 0.0)
						zc = ztransom;
					else {
						th = rtransom*rtransom - y*y;
						zc = ztransom + (rtransom - sqrt(th)) / stransom;	// where outside edge of curved transom cuts stern
					}
					vertex_count += 2;
					if(z < zc) {
						write_meshpoint(fp,x,side*ytran[it],ztran[it]);
						write_meshpoint(fp,x,side*ytran[it],ztran[it]);
						break;
					} else {
						write_meshpoint(fp,x,side*y,zc);
						write_meshpoint(fp,x,side*y,z);
					}
					t += dt;
				}
				t = dt;
			}
		}

//	Write out the vertex indices

		for(ii = 1 ; ii <= vertex_count-3 ; ii += 2) {
			fprintf(fp,"0\nVERTEX\n8\n0\n10\n0\n20\n0\n30\n0\n70\n128\n71\n%d\n72\n%d\n73\n%d\n74\n%d\n",ii,ii+2,ii+3,ii+1);
		}
		fprintf(fp,"  0\nSEQEND\n");    /* end line sequence*/
	}
	fprintf(fp,"  0\nENDSEC\n  0\nEOF\n");
#endif
	fclose(fp);
}

#endif

