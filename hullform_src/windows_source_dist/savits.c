/* Hullform component - savits.c
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

/*	SAVITSKY METHOD OF RESISTANCE PREDICTION	*/

/*	BASED ON D. SAVITSKY, "HYDRODYNAMIC DESIGN OF PLANING HULLS", MARINE	*/
/*	TECHNOLOGY, P. 71-95, OCTOBER 1964	*/

#include "hulldesi.h"
#define radian 0.01745329
REAL ulambda(REAL t1p1);
void pjust(REAL value);

#ifdef linux
extern Display *display;
#endif

extern	REAL	lambda;
REAL	Cv;
REAL	Cl0;
REAL	t,t1p1,rootLambda;
REAL	savlcg;
REAL	weight;
REAL	drang;
REAL	deadrise;
REAL	theta;
REAL	savbwl;
extern	REAL	epsilon;	/* shaft inclination	*/
extern	REAL	dist_down;	/* shaft distance below transom */
REAL	f;			/* shaft offset		*/
REAL	lbsq;
REAL	trimdrag;
REAL	vm;
REAL	rn;
REAL	cf;
REAL	skinfric;
REAL	dt;
REAL	viscosity[4] = {
	1.191e-6,1.191e-6,1.282e-5,1.282e-5};
REAL	savvcg;
extern int xjust;
REAL	r;

int	catmode;	/* 0 for monohull, 1 for cat */

REAL savits(REAL sp,INT setup)
{

	/*	ARGUMENT "SPEED" IS IN CONSISTENT UNITS:  EITHER M/S OR FT/S	*/
	/*	"SETUP" IS ZERO FOR TEXTUAL REPORT, 1 FOR SET-UP OF PARAMETERS AND	*/
	/*	RETURN OF VALUE, 2 FOR RETURN OF VALUE ONLY	*/

	REAL	speed;
	INT		i,j;
	static	REAL	lponb;
	REAL	a,hb,c,hd,aa,cc;
	char	*powun;
	REAL	sumb,sumw,numb,sumd,sumr,currdr,prevdr,ychine,zchine;
	REAL	Clb;
	REAL	dragf1,dragf2;
	REAL	temp,temp1,temp2;
	REAL	wlc,hbeam,ww,ww3,zwl;
	int		lcurr;
	REAL	twist;		/* twist of planing surface */
	REAL	areain,areaout;
	int		outer;
	REAL	b,b1;

	speed = sp;

	if(setup <= 1) {

		/*	convert dist_down to give f, distance below c of m		*/

		/*	savlcg IS DISTANCE FROM STERN TO CENTRE OF MASS	*/
		savlcg = xsect[count-1]-xcofm;
		a = zcofm-zline[stemli][count-1];	/* vcg above keel */
		f = dist_down - (a + sin(0.01745329 * epsilon)*savlcg);

		/*	FIND VCG, B AND DEADRISE				*/

		numb = 0.0;	/* number of cases	*/
		sumb = 0.0;	/* sum of half-beams	*/
		sumw = 0.0;	/* sum of wetted widths	*/
		sumd = 0.0;	/* sum of depths	*/
		sumr = 0.0;	/* sum of section "theta" factors */
		currdr = -1.0e+30;
		savvcg = currdr;
		for(i = 1 ; i < count ; i++) {
			if(savvcg < zline[stemli][i]) savvcg = zline[stemli][i];

			/*	only work from centre of mass backwards			*/

			if(xsect[i] < xcofm) continue;
			wlc = wl-beta*xsect[i]-hwl[i];
			hullar(i,0,numlin,&areain,&areaout,wlc,0.0,1.0,&hbeam,
				&ww,&ww3,&zwl,&lcurr);

			/*	If there is water inside the outer waterline, select
			catamaran mode
			*/
			if(ww < hbeam-0.001) catmode = TRUE;

			if(lcurr >= 1) {
				lcurr = (lcurr + 1) >> 1;/* convert to line index */
				/* If returned value is negative, no intersection */
				/* has occurred.  lcurr is line next inboard from */
				/* waterline intersection			  */
				aa = 0.0;
				cc = 1.0;
				ychine = 0.0;
				zchine = zline[stemli][i];

				/*	Look for point on section where slope of edge is 45 degrees	*/

				getparam(i,lcurr-1,&a,&hb,&c,&hd);
				outer = TRUE;
				for(j = lcurr ; j < numlin ; j++) {
					if(stsec[j] > i || ensec[j] < i) continue;

					/*	Start at next line below static waterline			*/

					tranpa(a,hb,c,hd,&aa,&cc);
					hullpa(i,j,aa,cc,&a,&hb,&c,&hd);

					/*	Waterline intersection lies between hull lines j and j-1.*/
					/*	Locate point t on parametric curve where slope is 1.	*/
					/*	Only accept the point if divisor is nonzero, and slope	*/
					/*	at lower end is less than 1.				*/

					if(c < a) {	/* slope less than 1 */
						temp = hb - hd;
						if(temp != 0.0)
							t = 0.5*(c-a)/temp;
						else
						    t = 1.0;

						/*	If point is beyond outside end, the 45 degree transition is	*/
						/*	taken to be at the end of the line				*/

						if(t > 1.0 || t < 0.0) {
							temp1 = yline[j-1][i];
							temp2 = zline[j-1][i];
						}
						else {

							/*	0 < t < 1 ... point is on this curve				*/

							temp1 = yline[j][i] + t*(a+t*hb);
							temp2 = zline[j][i] - t*(c+t*hd);
						}

						if(catmode) {
							if(!outer) {
								ychine = (ychine - temp1)*0.5;
								zchine = (zchine + temp2)*0.5;
								break;
							}
							else {
								ychine = temp1;
								zchine = temp2;
								outer = FALSE;
							}
						}
						else {
							ychine = temp1;
							zchine = temp2;
							break;
						}
					}
				}
				zchine = zline[stemli][i] - zchine;
				prevdr = currdr;
				currdr = zchine;
				if(prevdr > -1.0e+29) sumr += ychine*(prevdr-currdr);
				numb += 1.0;
				sumb += ychine;
				sumd += zchine;
				sumw += ww;
			}
		}

		/*	DEADRISE IS EXPRESSED AS A FRACTION ("TAN(DEADRISE ANGLE)")	*/

		if(sumb != 0.0) {
			savbwl = sumb / numb;
			deadrise = sumd / sumb;
			twist = sumr/ (savbwl*savlcg);
			savbwl *= 2.0;
			theta = atan(twist);
			if(catmode) {
				b = 2.0*savbwl;
				b1 = 2.0 * sumw / numb;
				if(b1 > 0.0) r = b / b1;
			}
		}
		else {
			return(-1.0e+30);
		}

		savvcg -= zcofm;		/* VCG value	*/
		weight = disp*g[numun];		/* total displacment	*/
		drang = atan(deadrise) * 57.293;	/* deadrise angle */

		/*	EVALUATE RATIO OF LENGTH FROM TRANSOM TO CENTRE OF MASS, */
		/*	DIVIDED BY MEAN WATERLINE BEAM, LP/B	*/

		lponb = savlcg / savbwl;

	}

	/*	get pitching moment for hull at zero thrust moment	*/

	Cv = speed / fsqr0(g[numun] * savbwl);

	/*	SOLVE FOR CL0 GIVEN CLB AND DEADRISE (PAGE 81, FIGURE 11)	*/

	temp = speed * savbwl;
	Clb = weight / (0.5*densit[numun]*temp*temp);
	Cl0 = fcl0(Clb,drang);

	/*	FIND NEUTRAL-MOMENT LAMBDA	*/

	lambda = flambd(lponb,Cv);

	/*	HENCE NEUTRAL-MOMENT TRIM ANGLE (DEGREES), USING	*/
	/*	EQUATION 15, PAGE 79					*/

	t = lambda / Cv;
	rootLambda = fsqr0(lambda);
	t1p1 = Cl0 / (rootLambda*(0.0120 + 0.0055*t*t));

	/*	Evaluate pitching moment for these conditions		*/

	temp1 = savitsky_balance(t1p1,speed,&dragf1);
	temp2 = savitsky_balance(t1p1+1.0,speed,&dragf2);
	if(temp1 < -1.0e+25 || temp2 < -1.0e+25 || Cv < 1.0) {
		dragf1 = -999.9;
		t = -9.999;
		dt = -999.999;
	}
	else if(temp1 != temp2) {

		/*	Interpolate between guesses		*/

		t1p1 += temp1 / (temp1 - temp2);
		if(t1p1 > 0.0) {

			/*	Use the interpolated estimate to revise the drag estimate */

			t = fpow(t1p1,0.909091);
			(void) savitsky_balance(t1p1,speed,&dragf1);

		}
		else {
			t = -9.999;
			dragf1 = -999.999;
		}
	}

	if(catmode && dragf1 >= 0.0) {

		/*	Empirical fit to Thong-song Lee, "Interference Factor for
		Catamaran Planing Hulls", American Institute of Aeronautics
		and Astronautics Journal, October 1982, pages 1461-1462,
		figure 3.
		*/
		temp1 = 0.4 + Cv * 0.3;
		if(r < temp1) {
			temp2 = Cv*(0.045 + 0.0025*Cv);
			temp1 = (temp1-r)/temp1;
			dragf1 *= (1.0+temp1*temp1*temp2);
		}
	}

	if(setup == 0) {

		/*	HENCE EFFECTIVE POWER, IN KW OR HP AS APPROPRIATE	*/

		clrtext();
		pstr("\nValues used:    lcg");
		pjust(savlcg);
		pstr(lenun[numun]);
		pstr("\n             Beam");
		pjust(savbwl);
		pstr(lenun[numun]);
		pstr("\n             LP/B");
		pjust(lponb);
		pstr(lenun[numun]);
		pstr("\n   Deadrise angle");
		pjust(drang);
		prea(" deg. (ratio%6.3f)\n",deadrise);
		pstr("\nValues found:  Cv");
		pjust(Cv);
		pstr("\n              CL0");
		pjust(Cl0);
		pstr("\n         CL(beta)");
		pjust(Clb);
		pstr("\n           Lambda");
		pjust(lambda);
		pstr("\n       Trim angle");
		pjust(t);
		pstr(" deg.");
		pstr("\n        Trim drag");
		pjust(trimdrag/g[numun]);
		pstr(masun[numun]);
		pstr("\n\n  Lambda * Beam^2");
		pjust(lbsq);
		pstr(" sq ");
		pstr(lenun[numun]);
		pstr("\n               Vm");
		pjust(vm);
		pstr(lenun[numun]);
		pstr("/s");
		pstr("\n               Rn");
		pjust(rn);
		pstr("\n               Cf");
		pjust(cf);
		pstr("\n    Skin friction");
		pjust(skinfric/g[numun]);
		pstr(masun[numun]);
		pstr("\n\n       Total drag");
		pjust(dragf1/g[numun]);
		pstr(masun[numun]);
		pstr("\nEffective power required");
		pjust(effpow(speed*dragf1,&powun));
		pstr(powun);
#ifdef linux
		XFlush(display);
#endif
	}
	aa = fdiv(dragf1,g[numun]);
	return(aa);
}

/*	LAMBDA AS FUNCTION OF LP/BETA AND CV.  OBTAINED FROM	*/
/*	EMPIRICAL FIT TO SAVITSKY, FIGURE 19, PAGE 93		*/

REAL flambd(REAL lponb,REAL Cv)
{
	REAL	value;
#define	f0	1.389177
#define f1	-0.561970
#define	f2	2.292953
#define	f3	-1.225397
#define	f4	0.178472

	value = min(2.2,lponb/Cv);
	return(lponb*(f0+value*(f1+value*(f2+value*(f3+value*f4)))));
}

/*	Lambda as function of Cl0, t^1.1 and Cv			*/

/*	Attempts to solve Savitsky's equation 15, page 79	*/

REAL ulambda(REAL t1p1)
{
	extern REAL	Cl0,Cv;
	REAL	prevLambda;
	REAL	temp;
	REAL	temp2;
	REAL	constant;
	REAL	Lambda = lambda;
	int		tries = 0;

	constant = Cl0 / t1p1;
	do {
		if(tries++ > 20) return(-1.0);
		prevLambda = Lambda;
		temp = Lambda / Cv;	/* lambda / Cv */
		temp2 = temp*temp;	/* (lambda / Cv) ^2 */
		temp =  0.01375 * temp2 + 0.006;
		/* 0.01375*(lambda/Cv)^2 + 0.006 */
		temp2 = 0.00825 * temp2 - 0.006;
		/* 0.00825*(lambda/Cv)^2 - 0.006 */
		Lambda = (temp2*prevLambda + constant*fsqr0(prevLambda)) / temp;
	}
	while(fabs(Lambda - prevLambda) > 0.01);

	return(Lambda);
}

/*	SOLVE CLB = CL0 - 0.0065*deadrise*CL0**0.60 FOR CL0	*/

REAL fcl0(REAL Clb,REAL deadrise)
{
	REAL guess,prev,power;
	int	i = 12;	/* maximum iteration count */

	guess = Clb;

	do {
		prev = guess;
		power = fpow(guess,0.4);
		guess = (0.0026*deadrise*guess+Clb*power)/
			(power-0.0039*deadrise);
	}
	while(i-- && guess > 0.0 && fabs(guess-prev) > 1.0e-3) ;
	return(guess);
}

/*	PRE-PLANING ANALYSIS, BASED ON SAVITSKY AND WARD-BROWN, MAR. TECH. 13, 1976	*/
/*	P381-400.	*/

REAL	af[14][11] = {
	{
		0.06473,0.10776,0.09483,0.03475,0.03013,0.03163,0.03194,0.04343,0.05036,0.05612,0.05697	}
	,
	{
		-.48680,-.88787,-.63720,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0	}
	,
	{
		-.01030,-.01634,-.01540,-.00978,-.00664,0.0,0.0,0.0,0.0,0.0,0.0	}
	,
	{
		-.06490,-.13444,-.13580,-.05097,-.05540,-.10543,-.08559,-.13289,-.15597,-.18661,-.19758	}
	,
	{
		0.00000,0.00000,-.16046,-.21880,-.19359,-.20540,-.19442,-.18062,-.17813,-.18288,0.20152	}
	,
	{
		0.10628,0.18186,0.16803,0.10434,0.09612,0.06007,0.06191,0.05487,0.05099,0.04744,0.04655	}
	,
	{
		0.97310,1.83080,1.55972,0.43510,0.51820,0.58320,0.52049,0.78195,0.92859,1.18569,1.30026	}
	,
	{
		-.00272,-.00389,-.00309,-.00198,-.00215,-.00372,-.00360,-.00332,-.00308,-.00244,-.00212	}
	,
	{
		0.01089,0.01467,0.03481,0.04113,0.03901,0.04794,0.04436,0.04187,0.04111,0.04124,0.04343	}
	,
	{
		0.00000,0.00000,0.00000,0.00000,0.00000,0.08317,0.07336,0.12147,0.14928,0.18090,0.01769	}
	,
	{
		-1.4096,-2.4670,-2.1556,-.92663,-.95276,-.70895,-.72057,-.95929,-1.1218,-1.3864,-1.5513	}
	,
	{
		0.29136,0.47305,1.02992,1.06392,0.97757,1.19737,1.18119,1.01562,0.93144,0.78414,0.78282	}
	,
	{
		0.02971,0.05877,0.05198,0.02209,0.02413,0.0,0.0,0.0,0.0,0.0,0.0	}
	,
	{
		-.00150,-.00356,-.00303,-.00105,-.00140,0.0,0.0,0.0,0.0,0.0,0.0	}
};
REAL	ff[11] = {
	1.0,1.1,1.2,1.3,1.4,1.5,1.6,1.7,1.8,1.9,2.0};
REAL	x,z,u,w;
REAL	b1,d1;
REAL	hent;

REAL prepla(REAL speed,INT setup)
{
	REAL	a[14];
	char	*powun;
	REAL	ww,ww3,zw,tb,wli;
	INT	l;
	INT	i;
	REAL	fn;
	REAL	estim;
	REAL	a1,a2,a3;
	REAL	rndisp,rn100k,cfdisp,cf100k;
	REAL	Prepla,F;
	REAL	areain,areaout;
	char	*Fun;
	static REAL	wt[11] = {
		1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0	};

	if(setup <= 1) {

		/*	THUS "X"	*/

		if(lwl > 0.0) {
			x = pow((double) volu,0.333333) / lwl;
		}
		else {
			x = 0.0;
		}

		/*	THUS THE HALF-ENTRY ANGLE, THUS "U"	*/

		hent = halfen();
		d1 = fabs(hent);
		u = fsqr0(fmul(2.0,d1));

		/*	FOR "W", FIND IMMERSED SECTION AREA AT TRANSOM	*/

		if(fpos(garea)) {
			i = count - 1;
			wli = fsub(fsub(wl,fmul(beta,xsect[i])),hwl[i]);
			hullar(i,0,numlin,&areain,&areaout,wli,0.0,1.0,&tb,
				&ww,&ww3,&zw,&l);
			w = fdiv(fmul(2.0,areain),garea);	/* use last section area */
		}
		else {
			w = 0.0;
		}
		/*	NOW APPROXIMATE MAXIMUM CHINE BEAM USING WATERLINE BEAM, THUS FIND "Z"	*/

		if(fpos(bwl)) {
			z = volu / (bwl * bwl * bwl);
		}
		else {
			z = 0.0;
		}
	}

	/*	EVALUATE VOLUME FROUDE NUMBER	*/

	if(volu > 0.0) {
		b1 = fpow(volu,0.3333333);
		fn = fdiv(speed,fsqr0(fmul(g[numun],b1)));
	}
	else {
		fn = 0.0;
	}

	/*	QUIT IF NOT IN PRE-PLANING RANGE	*/

	if(fn < 1.0 || fn > 2.0) {
		if(setup <= 0) {
			clrtext();
			pstr("Froude number for pre-planing mode must in range 1 to 2.\n");
			prea("\nThe value found is %7.3f\n",fn);
		}
		return(-1.0e+30);
	}

	/*	FIND COEFFICIENTS FOR THIS FROUDE NUMBER	*/

	for(i = 0 ; i < 14 ; i++)
		spline(ff,af[i],wt,11,&fn,&a[i],1,1.0,1.0);

	/*	HENCE 100,000-POUND ESTIMATOR (EQ. 16)	*/
	/*	estim = a[0]+
	x*(a[1]+z*(a[4]+x*a[11])+u*a[5]+w*(a[6]+w*a[10]))+
	u*(a[2]+z*a[7]+w*(w*a[12]+u*a[13]))+
	w*(a[3]+w*a[9]+z*a[8]);			*/

	a1 = fmul(w,fadd(a[6],fmul(w,a[10])));
	a1 = fadd(a[1],fadd(	fmul(z,fadd(a[4],fmul(x,a[11]))) ,
		fadd(fmul(u,a[5]),a1) ));
	a2 = fadd(a[2],fadd(	fmul(z,a[7]),
		fmul(w,fadd(fmul(w,a[12]),fmul(u,a[13]))) ));
	a3 = fadd(fadd(a[3],fmul(w,a[9])),fmul(z,a[8]));

	estim = fadd(fadd(a[0],fmul(x,a1)),fadd(fmul(u,a2),w*a3));

	/*	NOW APPLY CORRECTIONS (EQ. 18)	*/

	/*	REYNOLDS NUMBERS AT EXISTING DISPLACEMENT, AND 100 000 POUNDS	*/

	if(fpos(x)) {
		b1 = fsqr0(fmul(g[numun],volu));
		d1 = fsqr0(fdiv(fmul(g[2],100000.0),densit[2]));
		rndisp = fdiv(fmul(fn,b1),fmul(x,viscosity[numun]));
		rn100k = fdiv(fmul(fn,d1),fmul(x,viscosity[2]));

		/*	HENCE SKIN FRICTION COEFFICIENT	*/

		if(fpos(rndisp) && fpos(rn100k)) {
			a1 = log10(rndisp) - 2.0;
			a2 = log10(rn100k) - 2.0;
			cfdisp = fdiv(0.075,fmul(a1,a1));
			cf100k = fdiv(0.075,fmul(a2,a2));

			/*	AND SO REVISED DIMENSIONLESS DRAG (X NOT ZERO IMPLIES VOL NOT ZERO)	*/

			a1 = fpow(volu,0.6666667);
			estim = fadd(estim,fmul(fsub(cfdisp,cf100k),fmul(0.5,fmul(wetsur,fdiv(fmul(fn,fn),a1)))));
		}
		else {
			if(setup <= 0) {
				clrtext();
				pstr("The hull condition extends beyond the Savitsky & Brown");
				pstr("\nscheme's range of application.\n");
			}
			return(-1.0e+30);
		}
	}

	Prepla = fmul(fmul(estim,volu),densit[numun]);

	if(setup == 0) {
		clrtext();
		pstr("\n\nBased on the empirical formulation of Savitsky and Brown");
		pstr("\n(Mar. Tech. 13, 1976, p. 381-400), for ...");
		pstr("\n\nDisplaced volume^1/3 / Length ratio");
		pjust(x);
		pstr("\nDisplaced volume / Beam^3 ratio");
		pjust(z);
		pstr("\nHalf-entry angle");
		pjust(hent);
		pstr(" deg.");
		pstr("\nTransom / Maximum immersed area ratio");
		pjust(w);
		pstr("\nFroude Number");
		pjust(fn);
		pstr("\nCalculated drag force is");
		pjust(Prepla);
		pstr(masun[numun]);
		F = Prepla * g[numun];
		switch(numun) {
		case 1:
			F *= 1000.0;
		case 0:
			Fun = "N";
			break;
		case 3:
			F *= 2240.0;
		case 2:
			Fun = "poundal";
		}
		pstr("\n =");
		pjust(F);
		pstr(Fun);
		pstr("\nCorresponding effective power is");
		pjust(effpow(speed*Prepla*g[numun],&powun)),pstr(powun);
	}
#ifdef linux
	XFlush(display);
#endif
	return(Prepla);
}

REAL effpow(REAL power,char **unit)
{
	REAL	Effpow;
	char	*powunit[4] = {
		"kW","kW","HP","HP"	};

	if(numun == 0) {
		Effpow = power*0.001;
	}
	else if(numun >= 2) {
		Effpow = power/(32.0*550.0);
		if(numun == 3) Effpow *= 2240.0;
	}
	else {
		Effpow = power;
	}
	*unit = powunit[numun];

	return(Effpow);
}

REAL savitsky_balance(REAL t1p1,REAL speed,REAL *dragf1)
{
	REAL	t,trimr;
	REAL	d1;
	REAL	temp,temp1,temp2,ratio;
	REAL	Cp,xp,c,a;

	lambda = ulambda(t1p1);	/* unaligned lambda */
	if(lambda < 0.0) return(-1.0e+30);
	/* if no balance */

	t = fpow(t1p1,0.9090909);

	/*	HENCE TRIM ANGLE (RADIANS)	*/

	trimr = t * radian;

	/*	HENCE TRIM DRAG, INCLUDING BOTTOM WARPING EFFECTS (SAVITSKY AND	*/
	/*	WARD, MARINE TECH. 13, 1976, P 381-400, EQUATION 10)		*/

	trimdrag = disp * g[numun] * tan(trimr + 0.5 * theta);

	/*	NOW FOR FRICTION DRAG:	*/

	lbsq = lambda * bwl * bwl;

	/*	SKIN SPEED RATIO	*/

	temp = (1.08 - 0.016 * drang) / (rootLambda * cos(trimr));
	temp = 1.0 - 0.0120 * t1p1 * temp;
	if(temp <= 0.0) return 0.0;
	ratio = sqrt(temp);

	/*	HENCE SKIN SPEED	*/

	vm = speed * ratio;

	/*	HENCE REYNOLDS NUMBER	*/

	rn = vm * lambda * bwl / viscosity[numun];

	/*	HENCE SKIN FRICTION COEFFICIENT PLUS STANDARD ROUGHNESS CORRECTION	*/

	temp = log10(rn) - 2.0;
	if(temp <= 0.0) return 0.0;
	cf = 0.075 / (temp*temp);

	/*	HENCE DRAG FORCE.  NOTE COS(BETA) = 1/SQRT(1+DEADRI**2)		*/

	d1 = sqrt(1.0 + deadrise*deadrise);
	skinfric = densit[numun] * vm * vm * lbsq * cf * d1 * 0.5;

	/*	HENCE REARWARD DRAG FORCE	*/

	skinfric *= cos(trimr);

	/*	HENCE TOTAL DRAG FORCE, item 16			*/

	*dragf1 = trimdrag + skinfric;

	/*	Then centre of pressure, items 17 and 18	*/

	temp = Cv / lambda;
	Cp = 0.75 - 1.0 / (5.21 * temp * temp + 2.39);
	xp = Cp * lambda * bwl;

	c = savlcg - xp;			/* item 19	*/
	temp = bwl/4.0 * deadrise;	/* item 20	*/
	a = savvcg - temp;			/* item 21	*/
	temp1 = sin(trimr);			/* item 12	*/
	temp  = cos(trimr);			/* item 13	*/
	temp2 = sin(trimr + radian*epsilon);
	temp2 = 1.0 - temp1*temp2;	/* item 23	*/
	temp = c / temp * temp2;	/* item 24	*/
	temp = disp * (f*temp1 - temp);	/* items 25-27	*/
	temp1 = skinfric / g[numun] * (a - f);	/* items 28, 29	*/
	temp += temp1;				/* item 30	*/

	return(temp);
}

#endif
