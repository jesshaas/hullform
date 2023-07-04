/* Hullform component - getnorm.c
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

/**********************************************************************

  FUNCTION NAME:  get norm

  DESCRIPTION:    Finds the normal vector of two intersecting vectors
		  given three points;
		  1)  point on the tangent line.
		  2)  point on the curve (CHINE).
		  3)  point on line that intersects curve.
  ALGORITHM:
		  1)  Find vector from point on curve to tangent point.
		      Name this vector A.
		  2)  Find vector from point on curve to line.
		      Name this vector B.
		  3)  Determine the normal to vector A and vector B by cross
		      product ( VECTA X VECTB )

  INPUTS:
		  TANGY (real) contains y position of point on tangent line.
		  X,Y,Z (real) position of point on curve.
		  LN_X  (real) position of point on line.
		  INCR  (real) increment of x positions along curve.

  OUTPUTS:
		  NORMx  (real) contains the x,y,z coordinates of the vector
		  NORMy         normal to the point on curve.
		  NORMz
  CALLS:

 **********************************************************************/

void getnorm(float *normx,float *normy,float *normz,
	     float tangxt,float tangyt,float tangzt,
	     float blnx,  float blny,  float blnz,
	     float alnx,float alny,float alnz)
{
float vecta[3],vectb[3];       /* x,y,z vector pointing to chine a. */
			       /* tangent on chine a.               */
float	tempx,tempy,tempz;	/* temporary storage to protext addresses ? */
float	div;			/* normlising divisor */

/*                  determine vectors given 3 points  */
/*                  step 1.                           */

   vecta[0] = alnx-blnx;
   vecta[1] = alny-blny;
   vecta[2] = alnz-blnz;
   vectb[0] = tangxt;
   vectb[1] = tangyt;
   vectb[2] = tangzt;

/*                  perform cross product of vector a with vector b	*/
/*                  to get normal vector.         	                */
/*                  step 3.  */

   tempx = vecta[1]* vectb[2]-vecta[2]*vectb[1];
   tempy = vecta[2]* vectb[0]-vecta[0]*vectb[2];
   tempz = vecta[0]* vectb[1]-vecta[1]*vectb[0];
   div = fsqr0(tempx*tempx + tempy*tempy + tempz*tempz);
   if(div > 0.0) {
	*normx = tempx / div;
	*normy = tempy / div;
	*normz = tempz / div;
			/* interpolation model is arbitrary, hence	*/
			/* not accuracy-critical			*/
   } else {
	*normx = 0.0;
	*normy = 0.0;
	*normz = 0.0;
   }
}
