/* Hullform component - oortme.c
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
 
/*      POWER PREDICTION FOR SMALL SHIPS

	BASED ON PAPER "A POWER PREDICTION METHOD AND ITS APPLICATION TO SMALL
	SHIPS", C. VAN OOTMERSSEN, INT. SHIPB. PROG. 1971, P. 397-415

	ARGUMENT "SPEED" IS IN M/S OR FT/S AS APPROPRIATE
*/
#include "hulldesi.h"

#ifdef PROF

#ifdef linux
extern Display *display;
#endif

extern REAL viscos[];
REAL	lbp;
REAL	m;
REAL	c[4];
REAL	ld,ldonb,cwl,bont;
extern	REAL k,pd_ratio,prop_diam;
void	pjust(REAL value);
extern	int xjust;
extern	int dlgboxpos;

REAL oortme(REAL speed,INT indic)
{
    REAL	j_opt;
    REAL	n_opt;
    REAL	fn2i,mfn2i;
    REAL	ev[3];
    INT	i;
    char	*powun;
    REAL	rn,cf,rf,rp,rt,fn;
    REAL	rr;
    REAL	Oortme;
    REAL	effp;
    REAL	t,wt,etar,etah,eta0,lr;
    REAL	x1,x2;
    REAL	lcbforw,xfp;

/*	EMPIRICAL COEFFICIENTS	*/

    static REAL d[12][4] = {
	  {79.32134,  6714.88397,-908.44371, 3012.14549},
	  {-0.09287,    19.83000,   2.52704,    2.71437},
	  {-0.00209,     2.66997,  -0.35794,    0.25521},
	{-246.45896,-19662.02400, 755.18660,-9198.80840},
	 {187.13664, 14099.90400, -48.93952, 6886.60416},
	  {-1.42893,   137.33613,   9.86873, -159.92694},
	   {0.11898,   -13.36938,  -0.77652,   16.23621},
	   {0.15727,    -4.49852,   3.79020,   -0.82014},
	  {-0.00064,     0.02100,  -0.01879,    0.00225},
	  {-2.52862,  +216.44923,  -9.24399,  236.37970},
	   {0.50619,   -35.07602,   1.28571,  -44.17820},
	   {1.62851,  -128.72535, 250.64910,  207.25580}};

    static REAL e[8][3] = {
	{-0.93290, 0.72681, 0.03814},
	{ 3.94349,-1.74379, 3.69241},
	{-2.98757, 1.37241,-2.86213},
	{-0.98059, 0.26229,-0.33299},
	{ 1.04860,-0.22019,-0.70954},
	{ 0.00490, 0.01379,-0.00204},
	{ 0.00228,-0.00786,-0.00336},
	{-0.00152, 0.00216,-0.00350}};

    extern int context_id;

    context_id = 5001;

    if(indic <= 1) {

/*	LENGTH BETWEEN PERPENDICULARS, HENCE DISPLACEMENT LENGTH	*/

	xfp = xsect[0] - yline[0][0];
	lbp = xsect[count-1] - xfp;
	ld = 0.5*(lbp + lwl);

/*	lcb is measured forward of midships, in percent			*/

	lcbforw = 100.0*(0.5*ld - (xcofb - xfp))/ld;

/*	VAN OORTMERSSEN, TABLE II	*/

	m = 0.14347/ fpow(prisco,2.1976);

	ldonb = fdiv(ld,bwl);	/* used repeatedly in expressions */
	entang = halfen();	/* values in fig.10 indicate it has to be half */
	cwl = fmul(entang,ldonb); /* first after eq. 31 */
	bont = fdiv(bwl,tc);

/*	Equation 30		*/

	for(i = 0 ; i < 4 ; i++) {
	    c[i] = (d[0][i]+
		   (d[1][i]+d[2][i] *lcb   )*lcb  +
		   (d[3][i]+d[4][i] *prisco)*prisco   +
		   (d[5][i]+d[6][i] *ldonb )*ldonb+
		   (d[7][i]+d[8][i] *cwl   )*cwl  +
		   (d[9][i]+d[10][i]*bont  )*bont +
		   d[11][i]         *midsco) * 0.001;
	}

/*	FORM FACTOR	*/

	lr = lwl*max(0.1,(1.0-prisco+0.06*prisco*lcbforw/(4.0*prisco-1.0)));
			/* Holtrop p 253, "L sub R" */
	k = -0.07+fpow(tc/lwl,0.22284)*fpow(bwl/lr,0.92497)/
	    fpow(max(0.1,0.95-prisco),0.521448)*fpow(max(0.0,1.0-prisco+0.0225*lcbforw),0.6906);
				/* Holtrop p 25, top left */

    }

/*	FROUDE NUMBER	*/

    fn = fsqr0(fmul(g[numun],ld));
    fn = speed / fn;
    fn2i = 1.0/(fn*fn);
    mfn2i = m * fn2i;

/*	EQUATION (6) FOR SKIN FRICTION, PLUS WELDED SKIN CONTRIBUTION	*/

    rn = speed*ld/viscos[numun];
    cf = 0.43429*log(rn) - 2.0f;
    cf = 0.075/(cf*cf) + 0.00035;
    rf = 0.5*densit[numun]*cf*speed*speed*wetsur/g[numun];
    rp = rf * k;

/*	HENCE TOTAL DRAG, INCLUDING SKIN FRICTION	*/

    x1 = fexp(-mfn2i/9.0);
    x2 = fexp(-mfn2i);
    rr = volu*densit[numun]*(c[0]*x1+x2*(c[1]+c[2]*sin(fn2i)+c[3]*cos(fn2i)));

    rt = rr + rp + rf;

    Oortme = rt;

/*	MODES 1 AND 2 RETURN DRAG ONLY	*/

    if(indic >= 1) return(fn > 0.50 ? -999.9 : Oortme);

    effp = effpow(speed*rt*g[numun],&powun);

/*	POWERING CALCULATIONS IN MODE ZERO ONLY	*/

    if(prop_diam <= 0.0) prop_diam = 0.05 * lwl;
    if(pd_ratio <= 0.0) pd_ratio = 3.0;
    dlgboxpos = 1;   /* position bottom right */
    if(!getdlg(DRAGOORT,
	INP_REA,(void *) &prop_diam,
	INP_REA,(void *) &pd_ratio,-1)) return(Oortme);

    for(i = 0 ; i < 3 ; i++) {
	ev[i] =  e[0][i]+
		(e[1][i]+e[2][i]*prisco )*prisco  +
		(e[3][i]+e[4][i]*fn )*fn*prisco+
		 e[5][i]        *lcb*prisco+
		 e[6][i]        *ldonb+
		 e[7][i]        *ld/prop_diam;
    }

    t = max(0.0,min(1.0,ev[0]));
    wt = max(0.0,min(0.999999,ev[1]));
    etar = max(0.000001,ev[2]);
    etah = max(0.000001,(1.0-t)/(1.0-wt));

    eta0 = 0.18592+pd_ratio*(0.752753-pd_ratio*0.238593);
    j_opt = 0.008459+pd_ratio*(0.724218+pd_ratio*0.090948);
		/* least-squares fit to Harvald, "Resistance	*/
		/* Propulsion of Ships", fig. 6.3.10.  Result	*/
		/* is optimum for given P/D, range 0.5 to 1.4.	*/
    n_opt = (1.0-wt)*speed/(j_opt*prop_diam) * 60.0;
		/* optimum rpm for required result */
    xjust = 55;
    clrtext();
    pstr("\nFor speed");pjust(speed);pstr(lenun[numun]);pstr("/s");
    pstr("\nDisplacement length (LD)");pjust(ld);pstr(lenun[numun]);
    pstr("\nWetted surface");pjust(wetsur);pstr(lenun[numun]);
    pstr("\nSkin friction coefficient (I.T.T.C.)");pjust(cf);
    pstr("\nPrismatic coefficient (CP)");pjust(prisco);
    pstr("\nForm factor");pjust(k);
    pstr("\nCentre of buoyancy (LCB)");pjust(lcb);
    pstr("\nHalf-entrance angle");pjust(entang);pstr(" deg.");
    pstr("\nBeam");pjust(bwl);pstr(lenun[numun]);
    pstr("\nDraught");pjust(tc);pstr(lenun[numun]);
    pstr("\nReynolds number (Rn)");pjust(rn);
    pstr("\nFroude number (Fn)");pjust(fn);
    pstr("\nThrust deduction factor (t)");pjust(t);
    pstr("\nWake fraction (WT)");pjust(wt);
    pint("\nOpen water prop eff.(eta 0) @%4d rpm,Ja",(int)(0.5+n_opt));
	prea("%6.3f",j_opt);pjust(eta0);
    pstr("\nHull efficiency (eta H)");pjust(etah);
    pstr("\nRelative rotative efficiency (eta R)");pjust(etar);
    xjust = 35;
    pstr("\nDrag force: friction");pjust(rf);
    pstr("\n          + form drag");pjust(rp);
    pstr("\n          + wave drag");pjust(rr);pstr(" =");

    xjust = 55;
    pjust(rt);pstr(masun[numun]);
    pstr("\nEffective power needed is");pjust(effp);pstr(powun);
    pstr("\nThus total power needed is");pjust(effp/(eta0*etah*etar));pstr(powun);
#ifdef linux
    XFlush(display);
#endif
    return(Oortme);
}

#endif

/*	CALCULATE HALF-ENTRY-ANGLE FOR STEM	*/

REAL halfen()
{
    INT		i,l;
    REAL	currcl,prevcl;
    REAL	x1,wl0;
    REAL	slope;
    REAL	hb2,wetwid,ww3,zw;
    REAL	areain,areaout;

/*	Find waterline intersection	*/

    wl0 = wl - beta * xsect[0] - hwl[0];
    prevcl = wl0 - zline[stemli][0];
    if(prevcl <= 0.0) {
	(void) hullar(0,0,numlin,&areain,&areaout,wl0,0.0,1.0,&hb2,
		&wetwid,&ww3,&zw,&l);
	x1 = xsect[0] - hb2;
        i = 1;
    } else {
	for(i = 1 ; i < count ; i++) {
	    currcl = wl - beta * xsect[i] - hwl[i] - zline[stemli][i];
	    if(currcl < 0.0) {
		x1 = xsect[i] + currcl/(currcl - prevcl)*
				(xsect[i-1]-xsect[i]);
                break;
	    }
            prevcl = currcl;
	}
    }

    (void) hullar(i,0,numlin,&areain,&areaout,wl0,0.0,1.0,&hb2,&wetwid,&ww3,&zw,&l);
    x1 = xsect[i] - x1;
    if(x1 != 0.0) {
	slope = hb2 / x1;
	return(57.293*atan(slope));
    } else {
	return 0.0;
    }
}




