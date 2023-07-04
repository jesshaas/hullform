/* Hullform component - plate.h
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
 
/* AMAX1.C  return max. and min. of 5 floats */
float amax(float x1,float x2,float x3,float x4,float x5);
float amin(float x1,float x2,float x3,float x4,float x5);

/* FCHINEA.C returns values of y and z of a function 1 -> 10 */
void  fchinea(float x[],float y[],float z[],int n,int nstem);

/* FCHINEB.C returns the same, but different function chine B */
void  fchineb(float x[],float y[],float z[],int n);

/* NORM.C returns the length of a vector[3] */
float norm(float vector[]);

/* GETAB.C gets the projection lengths of triangles for roll out */
void   getab(float *a,float *b,float vecta[],float vectb[]);

/* GETNORM.C finds the normal vector of the intersection of two vectors */
void getnorm(float *normx,float *normy,float *normz,
	     float tangxt,float tangyt,float tangzt,
	     float blnx,  float blny,  float blnz,
	     float alnx,  float alny,  float alnz);

/* INTCHIA.C  interpolates points on chine A given dot product of normal	*/
/*            and tangent on chine b. */

void intchia(float *intchix,float *intchiy,float *intchiz,
	     float  achix[],float  achiy[],float  achiz[],
	     int i,         float  prev,   float  cur);
