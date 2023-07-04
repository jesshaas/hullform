/* Hullform component - gerrit.c
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

#include "hulldesi.h"

#ifdef linux
extern Display *display;
#endif

void pjust(REAL value);
extern	REAL viscos[];
REAL	a[7][14] = {
	{
		-13.01, -14.00, -13.11, -10.26, -4.151, -0.156,  6.203,  24.87,  85.16, 195.6 , 272.8 ,  414.0, 379.3 ,  588.1	}
	,
	{
		46.84,  50.15,  46.58,  36.06,  13.68, -2.106,-27.30 , -98.55,-315.2 ,-687.8 ,-901.2 ,-1321. ,-1085. ,-1666. 	}
	,
	{
		-42.34, -45.53, -42.76, -33.41, -12.81,  3.196,  29.88, 100.1 , 296.8 , 617.0 , 777.1 , 1117. , 877.8 , 1362. 	}
	,
	{
		-0.0190,-0.0214,-0.0153,-0.0021, 0.0478, 0.1211, 0.1711, 0.3168, 0.5725, 1.009 , 1.540 , 1.934 , 2.265 , 2.871	}
	,
	{
		-0.0046,-0.0062,-0.0062,-0.0043, 0.0041, 0.0176, 0.0273, 0.0570, 0.0930, 0.1476, 0.2142, 0.2690, 0.3266, 0.4519	}
	,
	{
		0.0341, 0.0481, 0.0674, 0.0757, 0.0967, 0.1504, 0.2240, 0.3365, 0.4526, 0.4640, 0.3431,-0.1746,-1.064 ,-1.501 	}
	,
	{
		0.0085, 0.0585, 0.1425, 0.2246, 0.2965, 0.3532, 0.3408, 0.3313, 0.4662, 0.6676, 0.3463, 0.0872,-1.053 ,-4.417 	}
};

REAL	fn[14] =
{
	.125,.15,.175,.2,.225,.25,.275,.3,.325,.35,.375,.4,.425,.45};

REAL gerrit(REAL speed,INT ind)
{
	REAL	rn,cf,rf,froude;
	INT		i;
	REAL	ratio,fac,rr,optcp,optlcb,gerritv;
	REAL	aa[7];
	REAL	x1;
	static REAL	loclcb;
	static REAL	wt[14] = {
		1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0	};

	if(lwl <= 0.0) return 0.0;

	if(ind <= 1) {
		prisco = volu / (garea * lwl);
		loclcb = (xmid - xcofb) / lwl * 100.0;
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

	if(froude >= 0.125 && froude <= 0.450) {

		/*	STANDFAST SERIES FOR RESIDUAL RESISTANCE	*/
		/*			(GERRITSMA, ONNINK AND VERSLUIS)	*/

		/*			FIND COEFFICIENTS FOR THIS SPEED	*/

		for(i = 0 ; i < 7 ; i++)
			spline(fn,a[i],wt,14,&froude,&aa[i],1,0.5,1.0);

		/*		THUS POLYNOMIAL FOR RR	*/

		x1 = fpow(volu,0.33333);
		ratio = lwl / x1;
		fac = (	aa[0] + (aa[1] + aa[2]*prisco)*prisco +
			    (aa[3] + aa[4]*loclcb)*loclcb+
			    aa[5]*(bwl/tc) + aa[6]*ratio)*1.0e-3;
		rr = volu * densit[numun] * fac;

		if(aa[2] != 0.0)
			optcp = -0.5f * aa[1] / aa[2];
		else
		    optcp = 99.9;
		if(aa[4] != 0.0)
			optlcb = -0.5 * aa[3] / aa[4];
		else
		    optlcb = 99.9;
		gerritv = rf + rr;

		if(ind <= 0) {
			pstr("\nUsing I.T.T.C. formula:");
			pstr("\nFor wetted surface of");
			pjust(wetsur);
			pstr("sq ");
			pstr(lenun[numun]);
			pstr("\nReynolds number of");
			pjust(rn);
			pstr("\nFriction coefficient of");
			pjust(cf);
			pstr("\nSkin friction is");
			pjust(rf);
			pstr(masun[numun]);
			pstr("\n\nFor ...");
			pstr("\nFroude number");
			pjust(froude);
			pstr("\nPrismatic co-efficient");
			pjust(prisco);
			prea(" (optimum,%7.3f)",optcp);
			pstr("\nCentre of buoyancy at");
			pjust(50.0-loclcb);
			prea("%% (optimum,%7.3f)",50.0-optlcb);
			pstr("\nWaterline beam");
			pjust(bwl);
			pstr(lenun[numun]);
			;
			pstr("\nDraught");
			pjust(tc);
			pstr(lenun[numun]);
			;
			pstr("\nWaterline length");
			pjust(lwl);
			pstr(lenun[numun]);
			pstr("\nDisplaced volume");
			pjust(volu);
			pstr(lenun[numun]);
			pstr("\nLength/displacement ratio");
			pjust(ratio);
			pstr("\nResidual resistance is");
			pjust(rr);
			pstr(masun[numun]);
			pstr("\nSo total resistance is");
			pjust(gerritv);
			pstr(masun[numun]);
		}
	}
	else {
		if(ind == 0) {
			pstr("\nWaterline length of");
			pjust(lwl);
			pstr(lenun[numun]);
			pstr("\ngives a Froude number of");
			pjust(froude);
			pstr("\nwhich is outside range of calibration data");
		}
		gerritv = -1.0e+30;
	}
#ifdef linux
	XFlush(display);
#endif
	return(gerritv);
}

#endif
