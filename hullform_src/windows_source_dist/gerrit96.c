/* Hullform component - gerrit96.c
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
 
#ifdef PROF

/*	Gerritsma 1996 Drag scheme	*/

#include "hulldesi.h"

#ifdef linux
extern Display *display;
#endif

void pjust(REAL value);
extern	REAL viscos[];

#define NUMFR 11
REAL	a96[9][NUMFR] = {
{-0.00086, 0.00078, 0.00184, 0.00353, 0.00511, 0.00228,-0.00391,-0.01024,-0.02094, 0.04623, 0.07319},
{-0.08614,-0.47227,-0.47484,-0.35483,-1.07091, 0.46080, 3.33577, 2.16435, 7.77489, 2.38461,-2.86817},
{ 0.14825, 0.43474, 0.39465, 0.23978, 0.79081,-0.53238,-2.71081,-1.18336,-7.06690,-6.67163,-3.16633},
{-0.03150,-0.01571,-0.02258,-0.03606,-0.04614,-0.11255, 0.03992, 0.21775, 0.43727, 0.63617, 0.70241},
{-0.01166, 0.00798, 0.01015, 0.01942, 0.02809, 0.01128,-0.06918,-0.13107, 0.11872, 1.06325, 1.49509},
{ 0.04291, 0.05920, 0.08595, 0.10624, 0.10339,-0.02888,-0.39580,-0.34443,-0.14469, 2.09008, 3.00561},
{-0.01342,-0.00851,-0.00521,-0.00179, 0.02247, 0.07961, 0.24539, 0.32340, 0.62896, 0.96843, 0.88750},
{ 0.09426, 0.45002, 0.45274, 0.31667, 0.97514,-0.53566,-3.52217,-2.42987,-7.90514,-3.08749, 2.25063},
{-0.14215,-0.39661,-0.35731,-0.19911,-0.63631, 0.54354, 2.20652, 0.63926, 5.81590, 5.94214, 2.88970}
};

REAL	fn96[NUMFR] = {.10,.15,.20,.25,.30,.35,.40,.45,.50,.55,.60};

REAL gerrit96(REAL speed,INT ind)
{
    REAL	rn,cf,rf,froude;
    INT		i;
    REAL	ratio,fac,rr,gerritv;
    REAL	optcp;
    REAL	aa[9];
    REAL	x1,x2,x3;
    static REAL	loclcb,loclcf;
    static REAL	wt[NUMFR] = {1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0};

    gerritv = -1.0e+30;
    if(lwl <= 0.0) return gerritv;

    if(ind <= 1) {
	prisco = volu / (garea * lwl);
	loclcb = xcofb - xentry;
	loclcf = xlcf - xentry;
    }

/*	ITTC SKIN FRICTION FORMULA	*/

    rn = speed * lwl / viscos[numun];
    if(rn > 0.0)
	cf = log10(rn)-2.0;
    else
	cf = 1.5;

    cf = 0.075 / (cf*cf);
    x1 = speed * speed * wetsur / g[numun];
    rf = 0.5 * densit[numun] * cf * x1;
    froude = speed / fsqr0(lwl * g[numun]);

    if(froude < 0.125 || froude > 0.60) {
	if(ind == 0) {
	    pstr("\nWaterline length of");pjust(lwl);pstr(lenun[numun]);
	    pstr("\ngives a Froude number of");pjust(froude);
	    pstr("\nwhich is outside range of calibration data");
	}
    } else if(volu > 0.0 && wplane > 0.0 && wetsur > 0.0 && loclcf > 0.0) {

/*	STANDFAST SERIES FOR RESIDUAL RESISTANCE	*/
/*			(GERRITSMA, ONNINK AND VERSLUIS)	*/

/*			FIND COEFFICIENTS FOR THIS SPEED	*/

	for(i = 0 ; i < 9 ; i++)
	    spline(fn96,a96[i],wt,NUMFR,&froude,&aa[i],1,0.5,1.0);

/*		THUS POLYNOMIAL FOR RR	*/

	x1 = pow((double) volu,0.33333);
	x2 = x1*x1;
	x3 = loclcb / lwl;
	ratio = x1 / lwl;
	fac =	aa[1]*x3        + aa[2]*prisco        + aa[3]*x2/wplane + aa[4]*bwl/lwl +
		aa[5]*x2/wetsur + aa[6]*loclcb/loclcf + aa[7]*x3*x3     + aa[8]*prisco*prisco;
	rr = volu * densit[numun] * (aa[0] + ratio*fac);

	if(aa[8] != 0.0)
	    optcp = -0.5 * aa[2] / aa[8];
	else
	    optcp = 99.9;

	gerritv = rf + rr;

	if(ind <= 0) {
	    pstr("\nUsing I.T.T.C. formula:");
	    pstr("\nFor wetted surface of");pjust(wetsur);pstr("sq ");pstr(lenun[numun]);
	    pstr("\nReynolds number of");pjust(rn);
	    pstr("\nFriction coefficient of");pjust(cf);
	    pstr("\nSkin friction is");pjust(rf);pstr(masun[numun]);
	    pstr("\n\nFor ...");
	    pstr("\nFroude number");pjust(froude);
	    pstr("\nPrismatic co-efficient");pjust(prisco);prea(" (optimum,%7.3f)",optcp);
	    pstr("\nWaterline beam");pjust(bwl);pstr(lenun[numun]);;
	    pstr("\nDraught");pjust(tc);pstr(lenun[numun]);;
	    pstr("\nWaterline length");pjust(lwl);pstr(lenun[numun]);
	    pstr("\nDisplaced volume");pjust(volu);pstr(lenun[numun]);
	    pstr("\nLength/displacement ratio");pjust(1.0/ratio);
	    pstr("\nResidual resistance is");pjust(rr);pstr(masun[numun]);
	    pstr("\nSo total resistance is");pjust(gerritv);pstr(masun[numun]);
	}
    } else if(ind == 0) {
	pstr("\nInvalid statics parameter - one of:\n");
	pstr("\nDisplaced volume");pjust(volu);
	pstr("\nWaterline length");pjust(lwl);
	pstr("\nWaterplane area"); pjust(wplane);
	pstr("\nWetted surface");  pjust(wetsur);
	pstr("\nCentre of flotation");pjust(loclcf);
    }
#ifdef linux
    XFlush(display);
#endif

    return(gerritv);
}

#endif
