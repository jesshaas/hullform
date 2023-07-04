/* Hullform component - norm.c
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
 
/**********************************************************************

  FUNCTION NAME: norm

  DESCRIPTION:   returns the norm or length of a given vector.

  INPUTS:        vector[] float given vector to find length of.

  OUTPUTS:       returns the norm.

  CALLS:         none.

 *********************************************************************/
#include "hulldesi.h"

float norm(float vector[])
{
   float val = vector[0]*vector[0]+vector[1]*vector[1]+vector[2]*vector[2];
   val = fsqr0(val);
   return(val);
}

/**********************************************************************

  FUNCTION NAME: get ab

  DESCRIPTION:   returns values a and b, where a is the distance of the end
		 of vector-b from the line of vector-a, and b is the
		 component of vector b along the direction of vector a
		 The two formulae used for projection are.

		 a = | vector-a x vector-b |
		     -----------------------
			   |vector-a|

		 b = | vector-a . vector-b |
		     -----------------------
			   |vector-a|

		 Thus, (b,a) is vector-b, transformed into a coordinate
		 system defined by vector-a.

  ALGORITHM:     step 1.
			take the cross product of the two 3-d vectors
		 step 2.
			get a using above formulae.
			get b using above formulae.
  INPUTS:        vecta[]  float
		 vectb[]  float
			3-d vectors to be projected.
  OUTPUTS:       a  float    projected length.
		 b  float    projected length.

  CALLS:         norm - returns the length of a vector.
 *********************************************************************/

#include "plate.h"

void   getab(float *a,float *b,float vecta[],float vectb[])
{
   float crosprod[3];
   float length;
						/* step 1.        */
   crosprod[0] = vecta[1]* vectb[2]-vecta[2]*vectb[1];
   crosprod[1] = vecta[2]* vectb[0]-vecta[0]*vectb[2];
   crosprod[2] = vecta[0]* vectb[1]-vecta[1]*vectb[0];
						/* step 2.       */
   length = norm(vecta);
   if(fnoz(length)) {

/*	Case where vector a is of finite length			*/

	*a = norm(crosprod) / length;
	*b = (vecta[0]*vectb[0] + vecta[1]*vectb[1] + vecta[2]*vectb[2])/
		length;
   } else {

/*	Case where vector a is of zero length			*/

	*a = 0.0;
	*b = norm(vectb);
   }
}
