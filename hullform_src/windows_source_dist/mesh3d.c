/* Hullform component - mesh3d.c
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
 
/*    WRITE 3D MESH TO VRML TEXT FILE		*/

#include "hulldesi.h"

char *shapehints = "\t    ]\n\t}\n\tShapeHints {\n\t    creaseAngle 0.698\n\t    vertexOrdering %sCLOCKWISE\n\t    faceType CONVEX\n\t}\n";

void mesh3d(void)
{
	extern int numbetwl;
	extern COLORREF scrcolour[];
	FILE *fp;
	REAL a,hb,c,hd;
	REAL t,th;
	REAL floatn = (float) (numbetwl+1);
	REAL dt = 1.0 / floatn;
	REAL tend = -0.5*dt;
	REAL frac0,frac1,frac,dfrac;
	int i,i0,i1,ii,imax;
	int j,jj,k,j0,j1;
#ifdef PROF
	int it;
	int sternflat = FALSE;
	REAL xe;
	REAL xt,yt;
	REAL x0,x1;
	REAL r0,r1;
	REAL c0,c1,cc;
	REAL th0,th1,dth;
	REAL x11,y11,z11,t0,t1,cos0,cos1,sin0,sin1;
	REAL ystem = yline[stemli][1];
	REAL zslope,zslope0,zslope1;
	int lastline = showtanks ? extlin : numlin;
	int stemcurve;
#else
#define lastline extlin
#endif
	REAL r,g,b;
	REAL x,y,z;
	REAL side;
	char *sh;
	extern int numtran;

	if(count < 2) return;

	if(!open_text(&fp,filedirnam,"*.wrl")) return;

	r = GetRValue(scrcolour[7])/255.0;
	g = GetGValue(scrcolour[7])/255.0;
	b = GetBValue(scrcolour[7])/255.0;
	fputs("#VRML V1.0 ascii\n\nDEF Hullform Separator {\n\n    Separator {\n\tMaterial {\n",fp);
	fprintf(fp,"\t    ambientColor [\n\t\t %.3f %.3f %.3f,\n\t    ]\n",r,g,b);
	fprintf(fp,"\t    diffuseColor [\n\t\t %.3f %.3f %.3f,\n\t    ]\n",r,g,b);
	fprintf(fp,"\t    specularColor [\n\t\t %.3f %.3f %.3f,\n\t    ]\n",r,g,b);
	r = GetRValue(scrcolour[8])/255.0;
	g = GetGValue(scrcolour[8])/255.0;
	b = GetBValue(scrcolour[8])/255.0;
	fprintf(fp,"\t    emissiveColor [\n\t\t %.3f %.3f %.3f,\n\t    ]\n",r,g,b);
	fputs("\t    shininess [\n\t\t 0.400,\n\t    ]\n",fp);
	fputs("\t}\n\tShapeHints {\n\t    vertexOrdering COUNTERCLOCKWISE\n\t    shapeType SOLID\n\t}\n",fp);

	/*	Output one surface on each side per adjacent pair of lines	*/

	for(j = 1 ; j < lastline ; j++) {
		sh = "COUNTER";
		for(side = 1.0 ; side > -1.5 ; side -= 2.0) {
			imax = count;
			fputs("\tCoordinate3 {\n\t    point [\n",fp);
			for(i = 0 ; i < count ; i++) {
				j0 = j-1;
				j1 = j;
#ifndef PROF
				frac0 = 0.0;
				frac1 = 1.0;
				getparam(i,j1,&a,&hb,&c,&hd);
#else

//	j0 is last line starting at or before this section; j1 is next line starting at or before this section

				while((stsec[j0] > i || ensec[j0] < i) && j1 > 0) j0--;
				while((stsec[j1] > i || ensec[j1] < i) && j1 < lastline-1) j1++;
				getparam(i,j1,&a,&hb,&c,&hd);
				frac0 = (REAL)(j-1 - j0)/(REAL)(j1 - j0);
				frac1 = frac0 + 1.0/(REAL)(j1 - j0);
				dfrac = dt/(REAL)(j1 - j0);

//	frac0 is the interpolation parameter between lines j0 and j1 which follows the path of line j-1
//	frac1 is the interpolation parameter between lines j0 and j1 which follows the path of line j

//	Determine whether we need to make provision for a radiused stem

				i0 = i == stsec[j0] && radstem[j0] > 0.0;
				i1 = i == stsec[j1] && radstem[j1] > 0.0;
				stemcurve = i0 || i1;
				if(stemcurve) {
					r0 = i0 ? radstem[j0] : 0.0;		/* radius of line at stem */
					r1 = i1 ? radstem[j1] : 0.0;		/* radius of line at stem */
					xt = xsect[i+1];	 		/* position of nearest transverse section */
					x0 = x1 = xsect[i];
					if(!surfacemode && i == 0) {
						x0 -= yline[j0][0];		// x-end of line j0
						x1 -= yline[j1][0];		// x-end of line j1
						if(xt == x0) x0 -= 0.0001;
						if(xt == x1) x1 -= 0.0001;
					}

					get_int_angle(j0,&cos0,&sin0);
					get_int_angle(j1,&cos1,&sin1);
					th0 = atan2(sin0,cos0);
					th1 = atan2(sin1,cos1);
					dth = (th1 - th0)*dt;
					zslope0 = (zline[j0][i+1] - zline[j0][i])/(xt-x0);
					zslope1 = (zline[j1][i+1] - zline[j1][i])/(xt-x1);
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
							z11 = zline[j1][i] - xt;
							r = r1 - xt*(r1 - r0) / (c+hd);
							cc = xsect[i] - yline[j1][i] - t*(a + t*hb) + r;
							x11 = cc - r*cos(th);
							y11 = ystem + r*sin(th);
							fprintf(fp,"\t\t%.4f %.4f %.4f,\n",-x11,-z11,side*y11);
							th += dth;
							frac += dfrac;		// increment interpolation between lines
						}
					}
					imax += (numbetwl+1);
					i = 1;	// stem done  - start at section 1
				}
				it = (numbetwl+1)*j0;	/* transom table index */
				if(i >= stsec[j1] && i <= ensec[j1]) {
					getparam(i,j1,&a,&hb,&c,&hd);
				}
				else {
					a = hb = c = hd = 0.0;
				}
#endif
				t = 1.0;
				while(t > tend) {
#ifdef PROF
					x = xsect[i];
					if(x > xtran[it]) {
						x = xtran[it];
						y = ztran[it];
						z = ytran[it];
					} else {
						y = zline[j1][i]-t*(c+t*hd);
						z = yline[j1][i]+t*(a+t*hb);
					}
#else
					x = xsect[i];
					y = zline[j1][i]-t*(c+t*hd);
					z = yline[j1][i]+t*(a+t*hb);
#endif
					if(i > 0)
						fprintf(fp,"\t\t%.4f %.4f %.4f,\n",-x,-y,side*z);
					else
					    fprintf(fp,"\t\t%.4f %.4f %.4f,\n",-x+z,-y,side*yline[stemli][1]);
					t -= dt;
#ifdef PROF
					it++;
#endif
				}
			}
			fprintf(fp,shapehints,sh);
			sh = "\0";
			fputs("\tIndexedFaceSet {\n\t    coordIndex [\n",fp);
			k = 0;
			for(i = 1 ; i < imax ; i++) {
				for(jj = 0 ; jj < numbetwl+1 ; jj++) {
					fprintf(fp,"\t\t%d, %d, %d, %d, -1,\n",k,k+1,k+numbetwl+3,k+numbetwl+2);
					k++;
				}
				k++;
			}
			fputs("\t    ]\n\t}\n",fp);
		}
	}

#ifndef STUDENT

	/*	Draw transom	*/

	for(side = 1.0 ; side > -1.5 ; side -= 2.0) {

		fputs("\tCoordinate3 {\n\t    point [\n",fp);
		j = -2;
		yt = ztran[0];
		xe = xsect[count-1];
		for(it = numtran-1 ; it >= 0 && ytran[0] >= ytran[it] ; it--) {
			x = xtran[it];
			if(x < 9999.0) {
				y = ztran[it];
				z = ytran[it] - ystem;
				if(x >= xe && transom) {
					sternflat = TRUE;
					if(rtransom == 0.0) {
						y = ztransom;
					}
					else {
						t = rtransom*rtransom - z*z;
						if(t >= 0.0 && stransom != 0.0) {
							y = ztransom + (rtransom - sqrt(t)) / stransom;
						}
					}
				}
				xt = x - (y - yt) * stransom / ctransom;
				fprintf(fp,"\t\t%.4f %.4f %.4f,\n",-x,-y,side*(z+ystem));
				fprintf(fp,"\t\t%.4f %.4f %.4f,\n",-xt,-yt,side*(z+ystem));
				j += 2;
			}
		}
		i = 0;
		while(it >= i) {
			if(xtran[it] < 9999.0) {
				fprintf(fp,"\t\t%.4f %.4f %.4f,\n",-xtran[it],-ztran[it],side*ytran[it]);
				fprintf(fp,"\t\t%.4f %.4f %.4f,\n",-xtran[i ],-ztran[i ],side*ytran[0]);
				j += 2;
			}
			it--;
			i++;
		}
		fprintf(fp,shapehints,sh);
		fputs("\tIndexedFaceSet {\n\t    coordIndex [\n",fp);
		for(i = 0 ; i < j ; i += 2) {
			fprintf(fp,"\t\t%d, %d, %d, %d, -1,\n",i,i+1,i+3,i+2);
		}
		fputs("\t    ]\n\t}\n",fp);

		/*	Add the stern flat if needed	*/

		if(!sternflat) continue;

		fputs("\tCoordinate3 {\n\t    point [\n",fp);
		j = -2;
		yt = ztran[0];
		xe = xsect[count-1];
		for(it = numtran-1 ; it >= 0 && ytran[0] >= ytran[it] ; it--) {
			x = xtran[it];
			if(x < 9999.0) {
				y = ztran[it];
				z = ytran[it]-ystem;
				if(x >= xe) {
					sternflat = it;
					yt = ztransom;
					if(rtransom > 0.0) {
						t = rtransom*rtransom - z*z;
						if(t >= 0.0 && stransom != 0.0) yt = ztransom + (rtransom - sqrt(t)) / stransom;
					}
					fprintf(fp,"\t\t%.4f %.4f %.4f,\n",-x,-y,side*(z+ystem));
					fprintf(fp,"\t\t%.4f %.4f %.4f,\n",-x,-yt,side*(z+ystem));
					j += 2;
				}
			}
		}
		i = 0;
		while(it >= i) {
			if(xtran[it] < 9999.0) {
				fprintf(fp,"\t\t%.4f %.4f %.4f,\n",-xtran[it],-ztran[it],side*ytran[it]);
				fprintf(fp,"\t\t%.4f %.4f %.4f,\n",-xtran[i ],-ztran[i ],side*ytran[0]);
				j += 2;
			}
			it--;
			i++;
		}
		fprintf(fp,shapehints,sh);
		fputs("\tIndexedFaceSet {\n\t    coordIndex [\n",fp);
		if(side > 0.0) {
			for(i = 0 ; i < j ; i += 2) fprintf(fp,"\t\t%d, %d, %d, %d, -1,\n",i,i+1,i+3,i+2);
		}
		else {
			for(i = 0 ; i < j ; i += 2) fprintf(fp,"\t\t%d, %d, %d, %d, -1,\n",i+3,i+2,i,i+1);
		}

		fputs("\t    ]\n\t}\n",fp);

	}
#endif
	fputs("    }\n}\n",fp);
	fclose(fp);
}
