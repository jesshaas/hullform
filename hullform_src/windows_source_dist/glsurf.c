/* Hullform component - glsurf.c
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
#undef tc
#include <GL/gl.h>
#include <GL/glu.h>

#define GLU_COMBINE 100105

void get_int_angle(int j,REAL *c,REAL *s);
/* stem-curve intercept angle function */
int find_inters_point(int j,REAL *x12s,REAL *y12s,REAL *z12s);
void setxyz(int l,int i,REAL t,REAL xyz[4],REAL sinal,REAL cosal,
	REAL a,REAL hb,REAL c,REAL hd);
void set_GL_perspective(void);
extern COLORREF scrcolour[];
extern REAL l_elev,l_azim;

extern	REAL	yview,zview;
extern	REAL	rotn;
extern	REAL	ypersp,zpersp;
extern	int		numbetwl;

#define	NOVALUE	1.0E+30

GLuint viewlist = 0;
GLuint wireframe_list = 0;
int GL_surfaces_need_redraw = TRUE;
int GL_wireframe_need_redraw = TRUE;
void setfont(LOGFONT *lf);
void check_gl(char *context);
extern LOGFONT lf;		/* logical font structure */
extern HDC hDC;

/*	OpenGL 3-D surface view				*/

void glShowPoint(GLdouble vert[6],GLdouble sign);
void glShowRects(GLdouble v1[6],GLdouble v2[6],GLdouble v3[6],GLdouble v4[6],GLdouble side1,GLdouble side2);
void glShowTris(GLdouble v1[6],GLdouble v2[6],GLdouble v3[6],GLdouble side1,GLdouble side2);

void glsurf(REAL side1,REAL side2,int endlin)
{
	COLORREF	colhi,collo;
	REAL	redhi,greenhi,bluehi;
	REAL	redlo,greenlo,bluelo;
	GLfloat	lightparam[4];
	REAL	xx,l_x,l_y,l_z;
	int		i,ij,ij0,j,k,ip;
	REAL	t,dt;
	GLdouble (*vert)[6] = NULL;
	GLdouble *v1,*v2,*v3,*v4;
	REAL	a,b,hb,c,d,hd,aa,bb,cc,xnorm,ynorm,znorm;
	int		n0;
	int		n = numbetwl + 1;	// number of intervals on curve per line pair
	int		nn;
	REAL	rad,rad0,rad1,delrad,x,y,z,z0,z1,dz,dzt;
	int		use_roundstem = 0;
	REAL	sn0,cs0,sn1,cs1,sn,cs;
	REAL	ang0,ang1,angle,delang,delangz;
	REAL	xstem0,xstem1,dx_stem;
	REAL	side;
	REAL	ystem;
	int		i0,i1,ii,j0,j1,l,kmax;
	REAL	sinz,cosz;
	int		npt;
	int		nppl = count;	// number of points per drawn line;
	void	GLcl(void);
	GLfloat	lightdistance = zview;
	extern	REAL	axx,axy,axz,ayx,ayy,ayz,azx,azy,azz;
	extern	REAL *radstem;
	int		jt;		/* transom table index */
	extern int numtran;

#ifdef PROF

/*	If there is a rounded stem, allocate numbetwl+1 extra "sections" to hold the shape of the stem	*/

	for(j = 0 ; j < numlin ; j++) {
		if(radstem[j] > 0.0) {
			use_roundstem = 1;
			nppl += n;
			break;
		}
	}
#endif
	npt = nppl * (n + 1) * (numlin-1);	// space for line at each side, plus points between, for each line pair

	colhi = scrcolour[7];
	redhi   = ((float) GetRValue(colhi)) / 255.0;
	greenhi = ((float) GetGValue(colhi)) / 255.0;
	bluehi  = ((float) GetBValue(colhi)) / 255.0;
	collo = scrcolour[8];
	redlo   = ((float) GetRValue(collo)) / 255.0;
	greenlo = ((float) GetGValue(collo)) / 255.0;
	bluelo  = ((float) GetBValue(collo)) / 255.0;

	set_GL_perspective();

	//	Set up lighting model

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_COLOR_MATERIAL);

	glLightModelf(GL_LIGHT_MODEL_TWO_SIDE,1.0);

	lightparam[0] = redhi;
	lightparam[1] = greenhi;
	lightparam[2] = bluehi;
	lightparam[3] = 1.0;
	glLightfv(GL_LIGHT0,GL_DIFFUSE,lightparam);

	lightparam[0] = redlo;
	lightparam[1] = greenlo;
	lightparam[2] = bluelo;
	glLightfv(GL_LIGHT0,GL_AMBIENT,lightparam);

	//	Calculate light coordinates

	xx = cosd(l_elev);
	l_x = xx * sind(l_azim);
	l_z = -xx * cosd(l_azim);
	l_y = -sind(l_elev);
	lightparam[0] = lightdistance * l_x;
	lightparam[1] = lightdistance * l_y;
	lightparam[2] = lightdistance * l_z;

	//	The light source is not to rotate with hull, so fix transformation coordinates
	//	before calling glLightfv, then restore then afterwards.

	glPushMatrix();
	glLoadIdentity();
	glLightfv(GL_LIGHT0,GL_POSITION,lightparam);
	glPopMatrix();

	//	Set up material

	glColorMaterial(GL_FRONT,GL_DIFFUSE);
	glColorMaterial(GL_FRONT_AND_BACK,GL_AMBIENT);
	glMaterialf(GL_FRONT_AND_BACK,GL_SHININESS,1.0);
	lightparam[0] = redhi;
	lightparam[1] = greenhi;
	lightparam[2] = bluehi;
	glMaterialfv(GL_FRONT_AND_BACK,GL_DIFFUSE,lightparam);

	glEnable(GL_DEPTH_TEST);
	glShadeModel(GL_SMOOTH);
	glFrontFace(GL_CCW);

//	When this routine is not called from the menu (or toolbar), we are simply redisplaying the image, so use the display list

/*
	if(!GL_surfaces_need_redraw) {
		glCallList(viewlist);
//		if( (i = glGetError()) != GL_NO_ERROR) (void) MessageBox(hWndMain,(LPCTSTR) gluErrorString(i),"glCallList Call",MB_OK);
		goto close_surface;
	}
*/
	GL_surfaces_need_redraw = FALSE;

	/*	Allocate arrays for vertices	*/

	if(!memavail((void **) &vert,npt*6*sizeof(GLdouble))) {
		message ("No memory free for plotting");
		return;
	}

	//	Set empty data in the vertex array

	for(ij = 0 ; ij < npt ; ij++) {
		vert[ij][0] = -1.0;		// negative offset means "no data"
		vert[ij][2] = NOVALUE;	// value ahead of stem means "no data"
	}

	//	Generate the vertex mesh between all line pairs

	/*	If any stem round, generate a surface encompassing all stem radii, no matter how far
	back along the hull they start

	NOTE that all distances are positive stemward, here
	*/
	if(use_roundstem) {
		aa = 0.0;
		cc = 1.0;

		/*	Proceed through all pairs of lines	*/

		for(j1 = 1 ; j1 < numlin; j1++) {
			j0 = j1-1;
			i1 = stsec[j1];
			i0 = stsec[j0];

			if(abs(i1 - i0) > 1) continue;

			if(i1 == 0)
				ystem = yline[stemli][1];
			else
				ystem = yline[i1][j1];

			/*	Generate (numbetwl+1) rounded stem curves between the forward ends of these lines	*/

			side = yline[j1][1] < ystem ? -1.0 : 1.0;

			rad0 = radstem[j0];
			rad1 = radstem[j1];

			get_int_angle(j0,&cs0,&sn0);				// find cos and sin of angle
			ang0 = atan2(sn0,cs0);						// the angle is negative if the line is inside the stem line
			xstem0 = -xsect[i0];
			if(i0 == 0) xstem0 += yline[j0][0];			// stem
			z0 = - zline[j0][i0];

			get_int_angle(j1,&cs1,&sn1);				// find cos and sin of angle
			ang1 = atan2(sn1,cs1);
			xstem1 = -xsect[i1];
			if(i1 == 0) xstem1 += yline[j1][0];
			z1 = - zline[j1][i1];

			dt = 1.0/(REAL) n;
			delrad = (rad1 - rad0)*dt;
			delangz = (ang1 - ang0) * dt;
			dz = z1 - z0;
			dx_stem = xstem0 - xstem1;

			t = 1.0;
			hullpa(stsec[j1],j1,aa,cc,&a,&hb,&c,&hd);
			tranpa(a,hb,c,hd,&aa,&cc);
			b = hb + hb;
			d = hd + hd;
			ij0 = j0*(n+1)*nppl;					// start index for round stem points on this drawn line
			rad = rad0;
			for(k = 0 ; k <= n ; k++) {				// step through the numbetwl lines between each line pair, plus one for each line
				ij = ij0;
				ij0 += nppl;
				if(i1 == 0) {
					z = z1 + t*(c + t*hd);			// z constant on stem curve
					x = xstem1 + t*(a + t*hb);		// x at stem
				} else {
					z = z1 + t*(c + t*hd);			// z constant on stem curve
//					z = z1 - dz*t;
//					x = xstem1 + dx_stem*t;
					x = xstem1;
					if(dz != 0.0) x += dx_stem*(z1 - z)/dz;
				}
				delang = ang0 / (numbetwl + 1);
				angle = 0.0;
				for(l = 0 ; l <= n ; l++) {		// step through the (numbetwl+1) points traced on each curve
					sn = sin(angle);
					cs = cos(angle);
					vert[ij][0] = ystem + sn*rad;				// y
					vert[ij][1] = z;							// z
					vert[ij][2] = x - (1.0-cs)*rad;				// x
					if(i1 == 0) {
						dzt = -dt*(c + t*d);
						znorm = -side*(delrad - dt*(a + t*b)*cs);	// z component of normal
					} else {
						dzt = dt * dz;
						znorm = side*(delrad - dt * dx_stem);
					}
					ynorm = sn*dzt;						 		// y component of normal
					xnorm = cs*dzt;								// x component of normal
					xx = sqrt(xnorm*xnorm + ynorm*ynorm + znorm*znorm);
					if(xx > 0.0) {
						vert[ij][3] = ynorm / xx;
						vert[ij][4] = znorm / xx;
						vert[ij][5] = xnorm / xx;
					}
					else {
						vert[ij][3] = -sn;
						vert[ij][4] = 0.0;
						vert[ij][5] = -cs;
					}
					angle += delang;
					ij++;
				}
				ang0 += delangz;
				rad += delrad;
				t -= dt;
			}
		}
		i0 = n+1;	// index of first point receiving data for transverse sections, subsequently back from stem
		i1 = 1;
	} else {
		i0 = 0;
		i1 = 0;
		ystem = yline[stemli][1];
	}

	for(i = i0, ii = i1 ; ii < count ; i++, ii++) {	// ii is section index, i is table index
		aa = 0.0;
		cc = 1.0;
		n0 = 0;	// n0 is the index at the start of each drawn line
		nn = 0;	// nn is number of intervals to plot, up to and including the next hull line
		jt = 0;	// transom table index
		for(j = 1 ; j < numlin ; j++) {
			nn += n;
			if(stsec[j] <= ii && ensec[j] >= ii) {
				hullpa(ii,j,aa,cc,&a,&hb,&c,&hd);
				b = hb + hb;
				d = hd + hd;
				dt = 1.0 / ((float) nn);
				t = 1.0;
				l = 0;
				for(k = 0 ; k <= nn ; k++) {
					ij = i + n0;
#ifdef PROF
					if(xsect[ii] <= xtran[jt]) {
						if(i > 0) {
							vert[ij][0] =   yline[j][ii] + t*(a + t*hb);
							vert[ij][1] = -(zline[j][ii] - t*(c + t*hd));
							vert[ij][2] = -xsect[ii];
						} else {
							vert[ij][0] = ystem;
							vert[ij][1] = -(zline[j][ii] - t*(c + t*hd));
							vert[ij][2] = -xsect[ii] + yline[j][ii] + t*(a + t*hb);
						}
						vert[ij][3] =   a + t*b;	// save cross-section vector
						vert[ij][4] = -(c + t*d);
						if(vert[ij][3] == 0.0 && vert[ij][4] == 0.0) {
							vert[ij][3] =   a + (t-0.1*dt)*b;	// save cross-section vector
							vert[ij][4] = -(c + (t-0.1*dt)*d);
						}
					} else {
						vert[ij][0] =  ytran[jt];
						vert[ij][1] = -ztran[jt];
						vert[ij][2] = -xtran[jt];
						if(jt == 0) {
							vert[ij][3] = ytran[0]-ytran[1];
							vert[ij][4] = ztran[0]-ztran[1];
						} else if(jt == numtran-1) {
							vert[ij][3] = ytran[jt-1]-ytran[jt];
							vert[ij][4] = ztran[jt-1]-ztran[jt];
						} else {
							vert[ij][3] = ytran[jt-1]-ytran[jt+1];
							vert[ij][4] = ztran[jt-1]-ztran[jt+1];
						}
					}
#else
					if(i > 0) {
						vert[ij][2] = -xsect[ii];
						vert[ij][0] = yline[j][ii] + t*(a + t*hb);
					} else {
						vert[ij][2] = -xsect[ii] + yline[j][ii] + t*(a + t*hb);
						vert[ij][0] = ystem;
					}
					vert[ij][1] = -(zline[j][ii] - t*(c + t*hd));
					vert[ij][3] = a + t*b;	// save cross-section vector
					vert[ij][4] = -(c + t*d);
#endif
breakpoint:
					n0 += nppl;
					if(l++ == n)
						l = 0;
					else
						t -= dt;
					jt++;
				}
				tranpa(a,hb,c,hd,&aa,&cc);
				nn = 0;
				jt--;	// re-use transom point on hull line next time
			}
		}
	}

	//	Next, find the surface normals

	ip = -1;
	for(ij0 = 0, j = 0 ; ij0 < npt ; ij0 += nppl, j++) {
		for(i = i0 ; i < nppl ; i++) {
			ij = ij0 + i;

			//	Vector along the hull "line" is (a,b,c)
			if(i == i0) {
				a = vert[ij + 1][2] - vert[ij][2];
				b = vert[ij + 1][0] - vert[ij][0];
				c = vert[ij + 1][1] - vert[ij][1];
				if(i == 0) {	// normal stem
					aa = vert[ij][3];		// This is not the cross-section vector (along stem curve),
					bb = 0.0;				// but its vector cross product gives the normal anyway
				} else {
					aa = 0.0;
					bb = vert[ij][3];
				}
			}
			else if(i == nppl - 1 || vert[ij + 1][0] < 0.0) {
				a = vert[ij][2] - vert[ij - 1][2];
				b = vert[ij][0] - vert[ij - 1][0];
				c = vert[ij][1] - vert[ij - 1][1];
				aa = 0.0;
				bb = vert[ij][3];
			}
			else {
				a = vert[ij + 1][2] - vert[ij - 1][2];
				b = vert[ij + 1][0] - vert[ij - 1][0];
				c = vert[ij + 1][1] - vert[ij - 1][1];
				aa = 0.0;
				bb = vert[ij][3];
			}
			cc = vert[ij][4];

			//	Vector across the surface is (aa,bb,cc)

			//	Surface normal is their cross-product
			//			(a, b, c)
			//			(aa,bb,cc)

			xnorm = b*cc - c*bb;
			ynorm = c*aa - a*cc;
			znorm = a*bb - b*aa;
have_norm:
			xx = sqrt(xnorm*xnorm + ynorm*ynorm + znorm*znorm);
			if(ip >= 0 && vert[ij][0] == vert[ip][0] && vert[ij][1] == vert[ip][1] && vert[ij][2] == vert[ip][2]) {
				vert[ij][3] = vert[ip][3];
				vert[ij][4] = vert[ip][4];
				vert[ij][5] = vert[ip][5];	// coincident points - ensure parallel normals
			}
			else if(xx > 0.0) {
				vert[ij][3] = ynorm/xx;
				vert[ij][4] = znorm/xx;	// was -
				vert[ij][5] = xnorm/xx;
				ip = ij;
			}
			else {
				vert[ij][3] = 0.0;		// If all else fails, normal is horizontal
				vert[ij][4] = 0.0;
				vert[ij][5] = -1.0;
			}
		}
	}

	glNewList(viewlist,GL_COMPILE_AND_EXECUTE);

#ifdef PROF
	if(transom) {
		for(side = side1 ; side <= side2 ; side += 2.0) {
			glBegin(GL_QUAD_STRIP);
			for(i = 0 ; i < numtran ; i++) {
				x = xtran[i];
				y = ytran[i];
				z = ztran[i];
				if(rtransom > 0.0)
					t = y/rtransom;
				else
					t = 0.0;
				if(x >= xsect[count-1]) {
					dt = (rtransom - fsqr0(rtransom*rtransom - y*y) ) / ctransom;
					z = ztransom + dt;
				}
				dt = fsqr0(1.0-t*t);
				glNormal3d(-t,-side*dt*stransom,-side*dt*ctransom);
				glVertex3d(-side*y,-z,-x);
				glVertex3d(-side*y,-ztran[0],-x+(z-ztran[0])*stransom/ctransom );
			}
			glEnd();
		}
	}
#endif

	ij0 = nppl;
	for(j = 1 ; j < numlin ; j++, ij0 += nppl) {			// for each line pair
		for(j1 = 0 ; j1 <= numbetwl ; j1++, ij0 += nppl) {	// for each pair of points within each line pair
			v1 = vert[ij0-nppl];
			v3 = vert[ij0];
			for(ij = ij0 + 1; ij < ij0 + nppl ; ij++) {
				v2 = vert[ij-nppl];
				v4 = vert[ij];
				if(v2[2] < v1[2]) {
					if(v1[2] != NOVALUE) {				// point 2 aft of point 1, point 1 valid - point 2 must be valid
						if(v4[2] < v3[2]) {				// v4 valid, v3 perhaps not
							if(v3[2] < NOVALUE) {		// point 4 aft of point 3, point 3 valid - show "rectangle"
								glShowRects(v1,v2,v3,v4,side1,side2);
								v1 = v2;
								v3 = v4;
							} else {					// point 3 invalid - show triangle
								glShowTris(v1,v2,v4,side1,side2);
								v1 = v2;
								v3 = v4;
							}
						} else if(*v3 >= 0.0) {
							glShowTris(v1,v2,v3,side1,side2);
							v1 = v2;
						} else if(*v4 >= 0.0) {
							glShowTris(v1,v2,v4,side1,side2);
							v1 = v2;
							v3 = v4;
						} else {
							v1 = v2;
						}
					} else if(v4[2] < v3[2] && v3[2] < NOVALUE) {		// point 4 aft of point 3 - show "rectangle"
						glShowTris(v2,v3,v4,side1,side2);
						v1 = v2;
						v3 = v4;
					}
				} else if(v4[2] < v3[2] && v3[2] < NOVALUE) {
					if(*v1 >= 0.0) {
						glShowTris(v1,v4,v3,side1,side2);
						v3 = v4;
					} else if(*v2 >= 0.0) {
						glShowTris(v2,v4,v3,side1,side2);
						v1 = v2;
						v3 = v4;
					}
				} else if(v2[2] == v1[2] && v4[2] == v3[2]) {
					v1 = v2;
					v3 = v4;
				}
			}
		}
	}

	glEndList();
//	if( (n = glGetError()) != GL_NO_ERROR) (void) MessageBox(hWndMain,(LPCTSTR) gluErrorString(n),"glEndList Call",MB_OK);
	memfree(vert);

close_surface:
	glDisable(GL_COLOR_MATERIAL);
	glDisable(GL_LIGHTING);
	glDisable(GL_LIGHT0);

}

void glShowPoint(GLdouble vert[6],GLdouble sign)
{
	if(vert[3] != 0.0 || vert[4] != 0.0 || vert[5] != 0.0) {
		glNormal3d(sign*vert[3],vert[4],vert[5]);
		glVertex3d(sign*vert[0],vert[1],vert[2]);
	}
}

#define GL_TYPE	GL_POLYGON
//GL_LINE_LOOP

void glShowRects(GLdouble v1[6],GLdouble v2[6],GLdouble v3[6],GLdouble v4[6],GLdouble side1,GLdouble side2)
{
	if(*v1 > 0.0 || *v2 > 0.0 || *v3 > 0.0 || *v4 > 0.0) {
		if(side1 < 0.0) {	// at least one point off centreplane
			glBegin(GL_TYPE);
			glShowPoint(v1,1.0);
			glShowPoint(v2,1.0);
			glShowPoint(v4,1.0);
			glShowPoint(v3,1.0);
			glEnd();
		}
		if(side2 > 0.0) {
			glBegin(GL_TYPE);
			glShowPoint(v1,-1.0);
			glShowPoint(v3,-1.0);
			glShowPoint(v4,-1.0);
			glShowPoint(v2,-1.0);
			glEnd();
		}
	}
}

void glShowTris(GLdouble v1[6],GLdouble v2[6],GLdouble v3[6],GLdouble side1,GLdouble side2)
{
	if(*v1 > 0.0 || *v2 > 0.0 || *v3 > 0.0) {
		if(side1 < 0.0) {
			glBegin(GL_TYPE);
			glShowPoint(v1,1.0);
			glShowPoint(v2,1.0);
			glShowPoint(v3,1.0);
			glEnd();
		}
		if(side2 > 0.0) {
			glBegin(GL_TYPE);
			glShowPoint(v3,-1.0);
			glShowPoint(v2,-1.0);
			glShowPoint(v1,-1.0);
			glEnd();
		}
	}
}


