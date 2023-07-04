/* Hullform component - holtro.c
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

/*	POWER PREDICTION FOR SMALL SHIPS

BASED ON PAPERS A STATISTICAL ANALYSIS OF PERFORMANCE TEST
RESULTS, I.J. HOLTROP, (?) I.S.P. 1981-1984, P. 23-28, AND
"A STATISTICAL POWER PREDICTION METHOD", J. HOLTROP AND G.G.J.
MENNENM I.S.P. 19??, P. 253-???.
ARGUMENT "SPEED" IS IN M/S OR FT/S AS APPROPRIATE
*/

#include "hulldesi.h"

#ifdef linux
extern Display *display;
#endif

void pjust(REAL value);
extern	int	xjust;
extern	REAL	m1,c1,c2;
REAL	hollambda;
REAL	holk;
REAL	holent;
extern	REAL	prop_diam,pd_ratio;
extern	REAL	viscos[4];
extern int dlgboxpos;

REAL holtro(REAL speed,INT indic)
{
	REAL	lr,m2;
	char	*powun;
	REAL	effp,rt,rw,rn,cf,rf,rp,fn,fn2i;
	REAL	ta,xp,cv,cb,diarat,wt,t,etar,blarat,cpa,etah,eta0;
	REAL	j_opt,n_opt;
	int		twin_screw;
	static REAL	lcbforw;
	extern int context_id;

	context_id = 5001;

	if(indic <= 1) {

		/*	WAVE DRAG PARAMETERS	*/

		holent = halfen();
		if(lwl <= 0.0 || bwl <= 0.0 || holent >= 90.0 || tc <= 0.0) return -999.9;
		c1 = 2223105.0*fpow(bwl/lwl,3.78613)*fpow(tc/bwl,1.07961) /
			fpow(90.0 - holent,1.37565);	/* H&M p 254 */

		/*	THE BULBOUS BOW TERM HAS TO HAVE AN ERROR !!			*/
		/*		ABT=??							*/
		/*	ABT IS TRANSVERSE AREA OF BULB					*/
		/*		TF=??							*/
		/*	TF IS DRAUGHT AT BASE OF BULB					*/
		/*		HB=??							*/
		/*	HB IS DISTANCE OF CENTRE OF AREA OF BULB ABOVE BASE		*/
		/*		SQRABT=SQRT(ABT)					*/
		/*		C3=0.56*ABT*SQRABT /					*/
		/*     *			( BWL*TC* (0.56*SQRABT+TF-HB-0.25*SQRABT) )	*/
		/*		C2=FEXP(-1.89*SQRT(C3))					*/

		c2 = 1.0;	/* no bulb effect */
		m1 = 0.0140407*lwl/tc-1.75254*fpow(volu,0.3333333)/lwl-
		    4.79323*(bwl/lwl)-8.07981*prisco+13.8673*prisco*prisco -
		    6.984388*prisco*prisco*prisco;
		/* H&M p 254 */
		hollambda = 1.446*prisco-0.03*(lwl/bwl);
		/* H p 254 */

		/*	FORM FACTOR							*/

		/*	lcb is measured forward of midships, in percent			*/

		lcbforw = 100.0*(0.5*lwl - (xcofb - xentry))/lwl;

		lr = lwl*max(0.1,(1.0-prisco+0.06*prisco*lcbforw/(4.0*prisco-1.0)));
		/* H p 253, "L sub R" */
		t = fpow(max(0.1,0.95-prisco),0.521448)*fpow(max(0.0,1.0-prisco+0.0225*lcbforw),0.6906);
		if(t <= 0.0 || lr <= 0.0) return -999.9;
		holk = -0.07+fpow(tc/lwl,0.22284)*fpow(bwl/lr,0.92497)/t;
		/* H p 25, top left */
		if(holk < 0.0) holk = 0.0;

	}

	/*	I.T.T.C. FORMULA FOR SKIN FRICTION, PLUS WELDED-SURFACE ROUGHNESS	*/

	rn = speed*lwl/viscos[numun];
	cf = log10(rn) - 2.0;
	if(cf <= 0.0) return -999.9;
	cf = 0.075/fmul(cf,cf)+0.00035;
	rf = 0.5*densit[numun]*cf*speed*speed*wetsur / g[numun];

	/*	THUS FORM DRAG	*/

	rp = fmul(rf,holk);

	/*	FROUDE NUMBER	*/

	fn = fsqr0(g[numun]*lwl);
	fn = speed / fn;
	if(fn <= 0.0) return -999.9;
	fn2i = 1.0/(fn*fn);				/* 1/fn^2 */

	/*	WAVE DRAG	*/

	m2 = -1.69385*prisco*prisco/fexp(0.1*fn2i);	/* H&M p 254 */

	rw = c1*c2*fexp(m1/fpow(fn,0.9)+m2*cos(hollambda*fn2i))*volu*densit[numun];
	/* H&M p 254, upper left */

	/*	HENCE TOTAL DRAG, INCLUDING SKIN FRICTION AND FORM DRAG	*/

	rt = rw + rf + rp;

	/*	MODES 1 AND 2 RETURN DRAG ONLY	*/

	if(indic >= 1) return(fn > 0.50 ? -999.9 : rt);

	effp = effpow(speed*rt*g[numun],&powun);

	/*	POWERING CALCULATIONS IN MODE ZERO ONLY	*/

	if(prop_diam <= 0.0) prop_diam = 0.03 * lwl;
	if(pd_ratio <= 0.0) pd_ratio = 3.0;
	twin_screw = 0;
	dlgboxpos = 1;   /* position bottom right */
	if(!getdlg(DRAGHOLT,
		INP_REA,(void *) &prop_diam,
		INP_REA,(void *) &pd_ratio,
		INP_LOG,(void *) &twin_screw,-1)) return(rt);

	ta = tc;	/* draught aft assumed equal to maximum */

	xp = 1.0-prisco;
	cv = (1.0+holk)*cf;
	cb = volu/(lwl*tc*bwl);

	if(twin_screw) {

		/*	tentative twin-screw factors (H&M p 255)		*/

		diarat = fdiv(prop_diam,fsqr0(fmul(bwl,tc)));
		wt = 0.3095*cb+10.0*cv*cb-0.23*diarat;
		/* effective wake fraction */
		t = 0.325*cb-0.1885*diarat;
		/* thrust deduction factor */
		etar = 0.9737+0.111*(prisco-0.0225*lcbforw)-0.06325*pd_ratio;
		/* relative-rotative efficiency */
	}
	else {

		/*	single-screw factors (H&M p 255 */

		if(xp <= 0.0) return -999.9;
		t = fsqr0(fdiv(bwl,fmul(lwl,xp))); /* (B/(L(1-Cp)))^1/2 */
		wt = bwl*wetsur*cv/(prop_diam*ta)*
		    (0.00661875/ta+1.21756*cv/(prop_diam*xp))
			+0.24558*t
			    -0.09726/(0.95-prisco)+0.11434/(0.95-cb);
		/* effective wake fraction */

		t = 0.001979*lwl/(bwl*xp)+1.0585*(bwl/lwl-0.00524)
			-0.1418*prop_diam*prop_diam/(bwl*tc);
		/* thrust deduction factor */

		cpa = prisco-0.0225*lcbforw; /* prismatic coefficient of afterbody */
		blarat = 0.6;	/* expanded blade area ratio */
		etar = 0.9922-0.05908*blarat+0.07424*cpa;
		/* relative-rotative efficiency */
	}

	t    = max(0.0     ,min(1.0,t));
	wt   = max(0.0     ,min(0.999999,wt));
	etar = max(0.000001,min(1.0,etar));
	etah = max(0.000001,min(1.0,(1.0-t)/(1.0-wt)));

	eta0 = 0.18592+pd_ratio*(0.752753-pd_ratio*0.238593);
	/* least-squares fit to Harvald, "Resistance	*/
	/* Propulsion of Ships", fig. 6.3.10.  Result	*/
	/* is optimum for given P/D.			*/
	j_opt = 0.008459+pd_ratio*(0.724218+pd_ratio*0.090948);
	/* least-squares fit to Harvald, "Resistance	*/
	/* Propulsion of Ships", fig. 6.3.10.  Result	*/
	/* is optimum for given P/D, range 0.5 to 1.4.	*/
	n_opt = (1.0-wt)*speed/(j_opt*prop_diam) * 60.0;
	/* optimum rpm for required result */

	clrtext();
	xjust = 55;
	pstr("\nFor speed");
	pjust(speed);
	pstr(lenun[numun]);
	pstr("/s");
	pstr("\nWaterline length (LWL)");
	pjust(lwl);
	pstr(lenun[numun]);
	pstr("\nWetted surface");
	pjust(wetsur);
	pstr("sq ");
	pstr(lenun[numun]);
	pstr("\nSkin friction coefficient (I.T.T.C)");
	pjust(cf);
	pstr("\nPrismatic coefficient (CP)");
	pjust(prisco);
	pstr("\nForm factor");
	pjust(holk);
	pstr("\nCentre of buoyancy (LCB)");
	pjust(lcbforw);
	pstr("%");
	pstr("\nHalf-entrance angle");
	pjust(holent);
	pstr("deg");
	pstr("\nBeam");
	pjust(bwl);
	pstr(lenun[numun]);
	pstr("\nDraught");
	pjust(tc);
	pstr(lenun[numun]);
	pstr("\nReynolds number (Rn)");
	pjust(rn);
	pstr("\nFroude number (Fn)");
	pjust(fn);
	pstr("\nThrust deduction factor (t)");
	pjust(t);
	pstr("\nWake fraction (WT)");
	pjust(wt);
	pint("\nOpen water prop eff.(eta 0) @%4d rpm,Ja",(int)(0.5+n_opt));
	prea("%5.1f",j_opt);
	pjust(eta0);
	pstr("\nHull efficiency (eta H)");
	pjust(etah);
	pstr("\nRelative rotative efficiency (eta R)");
	pjust(etar);
	xjust = 35;
	pstr("\nDrag force: friction");
	pjust(rf);
	pstr("\n          + form drag");
	pjust(rp);
	pstr("\n          + wave drag");
	pjust(rw);
	pstr(" =");
	xjust = 55;
	pjust(rt);
	pstr(masun[numun]);
	pstr("\nEffective power needed is");
	pjust(effp);
	pstr(powun);
	pstr("\nThus total power needed is");
	pjust(effp/(eta0*etah*etar));
	pstr(powun);
#ifdef linux
	XFlush(display);
#endif

	return(rt);
}

REAL fexp(REAL expon)
{
	REAL fexpv;

	if(expon < -30.0) {
		fexpv = 0.0;
	}
	else if(expon < 30.0) {
		fexpv = (float) exp((double) expon);
	}
	else {
		fexpv = exp(30.0);
	}
	return(fexpv);
}

REAL fpow(REAL base,REAL expon)
{
	if(fpos(base)) {
		return(fexp(log(base)*expon));
	}
	else if(fzer(base)) {
		return 0.0;
	}
	else {
		message("Base of power expression was\nnegative; zero was returned");
		return(0.0);
	}
}


#endif
