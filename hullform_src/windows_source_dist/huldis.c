/* Hullform component - huldis.c
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
 
/*      CALCULATE HULL VOLUME DISPLACED AND WATERPLANE AREA FOR A GIVEN */
/*	WATERLINE OFFSET AND HEEL					*/

#include "hulldesi.h"
extern int tankmode;

REAL tanklevel(int tank,REAL wl0,REAL *,REAL *xsum,REAL *sumlcf,
	REAL *summct,REAL *tankplane,REAL *vol);

#ifdef PROF
int first_transom_warning = TRUE;
#endif

void huldis(REAL *calcdisp)
{
	INT		i,ii,j,jj,ihull;
	REAL	hbeam1,hbeam2,area1,area2,ww1,ww2,ww3,rm1,rm2;
	REAL	xsum,xsta,xend,wetwid;
	REAL	dz1,dz2,cl1,cl2,sumlcf,summct;
	REAL	fac,zwl,x1,area;
	INT		line;
	REAL	AZabove,AZbelow;
	REAL	AXabove,AXbelow;
	REAL	vol;
	REAL	dz,cl,dx,dx1,dx2,h1,h2;
	REAL	totmom;
	REAL	ax;
	REAL	z;
	int		ensave[maxlin];
	int		iend,last,ilast;
	REAL	draft;
	REAL	sumysqdif = 0.0,sumyoffs = 0.0;
#ifndef STUDENT
	REAL	tankrsg;
	REAL	tanklcfv,tankmct,tanksarm,tankxsum;
	REAL	tankplane;
#endif
	int		it2,it_inc,it_lim;
	int		it1,del_it,it,tlast;
	REAL	zcut[maxsec],ycut[maxsec],y,t;
	REAL tvalue(REAL dy,REAL dz,REAL a,REAL hb,REAL c,REAL hd);
	extern int numtran;
	int save_stsec[maxlin],save_ensec[maxlin];
	REAL a,hb,c,hd;
	REAL lwl_loc;

	Aabove = 0.0;
	Abelow = 0.0;
	AXabove = 0.0;
	AXbelow = 0.0;
	AZabove = 0.0;
	AZbelow = 0.0;

	wplane = 0.0;
	volu = 0.0;
	*calcdisp = 0.0;
	volforw = 0.0;
	volstern = 0.0;
	garea = 0.0;
	bwl = 0.0;
	hbeam2 = 0.0;
	area2 = 0.0;
	ww2 = 0.0;
	rm2 = 0.0;
	xsum = 0.0;
	lwl = 0.0;
	xsta = 1.0e+16;
	xend = -1.0e+16;
	sarm = 0.0;

	/*	dz1 and dz2 are distances above the waterline	*/

	dz2 = wl - (cosa*zline[0][0]+beta*(xsect[0]-yline[0][0]));

	/*	EVALUATE MINIMUM CLEARANCE OF HULL ABOVE WATER LEVEL.  "CL2" IS	*/
	/*	NEGATIVE IF HULL IS IMMERSED	*/

	cl2 = dz2;
	tc = cl2;
	sumlcf = 0.0;
	summct = 0.0;
	totmom = 0.0;
	xentry = -1.0e+30;
	density = densit[numun];
	for(j = 0 ; j < numlin ; j++) ensave[j] = ensec[j];

	/*	CALCULATE TERMS ON STEM PROFILE	*/

	/*	Place one cross section on each line end */

	line = 0;
	last = count;	// last of actual hull, +1 == first of interpolated sections
	do {
		iend = min(maxsec,count + numlin - line)-1;
		i = iend;
		do {
			if(stsec[line] <= 0)
				xsect[i--] = xsect[0] - yline[line][0];	// section here, or ...
			else
			    last++;									// one less section to handle
			line++;
		}
		while(i >= count && line < numlin);
		for(j = 0 ; j < numlin ; j++) {
			if(stsec[j] <= 0) {
				refit(j,xsect,yline[j],zline[j],ycont[j],zcont[j],last,iend);
				ensec[j] = iend - j;
			}
		}
		x1 = xsect[0] - yline[0][0];
		dz2 = wl - (cosa*zline[0][0] + beta*x1);
		for(j = iend ; j >= last ; j--) {
			calcar(j,0,numlin,1,1,
				&area1,&area2,
				&hbeam1,&hbeam2,
				&cl1,&cl2,
				x1,xsect[j],
				&xsum,&volu,calcdisp,
				&totmom,&wplane,
				wl,density,&lwl,
				&xsta,&xend,&sumlcf,&summct,
				&ww1,&ww2,&rm1,&rm2,
				&sumysqdif,&sumyoffs);
			lwl_loc = lwl;
			dz1 = dz2;			/* dz1 and dz2 are heights of line 0 above waterline */
			dz2 = wl - (cosa*zline[0][j]-sina*yline[0][j] + beta*xsect[j]);
			dz = 0.5*(dz1+dz2);		/* mean "freeboard", positive	*/
			cl = 0.5*(cl1+cl2);		/* mean waterline clearance	*/
			h1 = dz1 - cl1;		/* depth of hull at previous section */
			h2 = dz2 - cl2;		/* depth of hull at current section */
			dx = xsect[j]-x1;		/* length of interval between previous and current sections */
			AXabove += 0.5*dx*(h1*(x1+0.33333*dx) + h2*(x1+0.66667*dx));
			/* area*distance contribution if hull remains clear of water */
			if(cl1 > 0.0) {		/* hull commenced clear of water */
				Aabove += dx*(dz-cl);	/* adjusted later if hull enters water */
				if(cl2 > 0.0) {		/* hull remained clear of water	*/
					AZabove += 0.16667*dx*((dz2*dz2 + dz1*dz2 + dz1*dz1) - (cl2*cl2 + cl1*cl2 + cl1*cl1));
					/* ... no change to AZbelow, AXabove or AXbelow */
				}
				else {		/* hull entered water: xsta is waterline entry point */
					dx2 = xsect[j] - xsta; /* xsta is x-coordinate where hull enters water */
					dx1 = xsta - x1;
					z = -dx2*0.5*cl2; 	/* cl2 < 0 - area aft of xsta, needs removing from Aabove */
					Abelow += z;	/* positive contribution to area below */
					Aabove -= z;	/* remove area not occupied */
					AZbelow += z*0.333333*cl2;/* negative contribution (z and cl2 negative) */
					ax = z*(xsta+0.66667*dx2); /* positive area term */
					AXbelow += ax;	/* positive contribution */
					AXabove -= ax;	/* negative contribution */
					if(dx != 0.0) {	/* normally the case, except for first run through loop */
						dz = dz1+(dz2-dz1)*dx1/dx;
						AZabove += 0.16667*(dx1*(dz1*dz1 + dz*dz1 + dz *dz - cl1*cl1) +
							    dx2*(dz *dz  + dz*dz2 + dz2*dz2));
					}
				}
			}
			else {			/* hull commenced in water */
				if(cl2 < 0.0) {		/* hull remained in water */
					Abelow -= dx*cl;	/* positive area */
					AZbelow += 0.16667*dx*(cl1*cl1+cl1*cl2+cl2*cl2);	/* cl1 and cl2 negative - RHS positive */
					AZabove += 0.16667*dx*(dz1*dz1+dz1*dz2+dz2*dz2);
					ax = -0.5*dx*(cl1*(x1+0.33333*dx)+cl2*(x1+0.66667*dx));	/* positive */
					AXbelow += ax;	/* ax positive */
					AXabove -= ax;
					Aabove += dx * cl;	/* cl negative */
				}
				else {		/* hull emerged from water */
					dx2 = xsect[j] - xend;
					dx1 = xend - x1;
					z = -dx1*0.5*cl1;		/* positive */
					Abelow += z;
					AZbelow -= 0.33333*z*cl1;	/* cl1 negative */
					ax = z*(x1+0.33333*dx1);	/* positive area*/
					AXbelow += ax;
					AXabove -= ax;
					Aabove -= z;
					if(dx != 0.0) {
						dz = dz1+(dz2-dz1)*dx1/dx;
						AZabove += 0.16667*(dx2*(dz2*dz2+dz*dz2+dz*dz - cl2*cl2)
							    + dx1*(dz*dz+dz*dz1+dz1*dz1));
					}
				}
			}
			x1 = xsect[j];
		}
	}
	while(line < numlin);

	for(j = 0 ; j < numlin ; j++) ensec[j] = ensave[j];

	/*	USE "HULLAR" ROUTINE ON SECTION 0 TO INITIALISE WATERLINE LENGTH */

	fac = fdiv(1.0,fsqr0(fadd(1.0,fmul(beta,beta))));
	hullar(0,0,stemli+1,&area,&area,
		fsub(wl,fadd(fmul(beta,xsect[0]),hwl[0])),fmul(beta,fac),fac,&lwl,
		&wetwid,&ww3,&zwl,&line);
			lwl_loc = lwl;
	cl2 = -(cosa*zline[stemli][0] + beta*(xsect[0]-yline[stemli][1]) + hwl[0] - wl);
	if(fneg(cl2)) xentry = fsub(xsect[0],wetwid);
	x1 = xsect[0];

	/*	CALCULATE TERMS ALONG MAIN BODY OF HULL	*/

	ilast = count;
	tlast = -1;

#ifdef PROF

//	Transom points run sheerline to keel

	if(stransom > 0.0) {
		it2 = 0;
		it_inc = 1;
		it_lim = numtran-1;
	} else {
		it2 = numtran-1;
		it_inc = -1;
		it_lim = 0;
	}

#endif

	for(ihull = 1 ; ihull < ilast ; ihull++) {
		if(fgt(xsect[ihull],xsect[0])) {

#ifdef PROF

//	Hullform 9 transom code.

			it1 = it2;	// transom table index, starts at it2 = 0 for a forward raked transom, otherwise at numtran-1

			while(xsect[ihull] > xtran[it2] && it2 != it_lim) it2 += it_inc;	// find next transom point (index it2) sternward of, or at, section

//	transom points "i", for it1 <= i < it2 lie between the current section and the previous one

			if(it2 > it1) {	// there is at least one transom section between these sections

				if(count >= maxsec) {
					if(first_transom_warning) {
						message("Insufficient free space for transom in data tables:\n\nTransom not included in statics calculations");
						first_transom_warning = FALSE;
					}
					it1 = numtran;
				}
				del_it = (it2 - it1) / (maxsec - count) + 1;	// interval in transom table to use - prefer 1, but greater if not enough sections available
				if(del_it > 1 && first_transom_warning) {
					message("Inadequate free space for transom in data tables:\n\nTransom statics may only be approximate");
					first_transom_warning = FALSE;
				}

//	Create array of section positions to be interpolated, and save the cut points of the transom at each

				for(ii = count, it = it1 ; it != it2 ; ii++, it += del_it) {
					xsect[ii] = xtran[it];
					zcut[ii] = ztran[it];
					ycut[ii] = ytran[it];
				}
				tlast = ii - 1;	// last index for these transom sections

//	Interpolate the sections

				for(j = 0 ; j < numlin ; j++) refit(j,xsect,yline[j],zline[j],ycont[j],zcont[j],count,tlast);

//	Save the start and end sections, and chang them temporarily to allow use of the "getparam" routine

				for(j = 0 ; j < numlin ; j++) {
					save_stsec[j] = stsec[j];
					save_ensec[j] = ensec[j];
					stsec[j] = count;
					ensec[j] = tlast;
				}

				for(ii = count ; ii <= tlast ; ii++) {	// for each interpolated transom section ...

//	If transom slope is positive, we cut off interpolated sections at the top. Otherwise, cut off their bottoms.
//	Either way, remember the cut point:

					z = zcut[ii];
					y = ycut[ii];

//	Find the first line "below" the transom intersection

					for(jj = 1 ; jj < numlin && zline[jj][ii] < z ; jj++) ;

//	Obtain curve parameters before the change, to allow recalculation of the control point at the intersection
					if(jj < numlin) {
						getparam(ii,jj,&a,&hb,&c,&hd);
						t = tvalue(y - yline[jj][ii],zline[jj][ii] - z,a,hb,c,hd);
					}
					if(atransom > 0.0) {	// cut point forms top of interplated transom section

						if(jj < numlin) {
							zcont[jj][ii] = zline[jj][ii] - 0.5*t*c;
							ycont[jj][ii] = yline[jj][ii] + 0.5*t*a;

//	Set all hull lines above the transom at the intersection of the transom and the hull

							for(j = 0 ; j < jj ; j++) {
								zline[j][ii] = z;
								yline[j][ii] = y;
								zcont[j][ii] = z;
								ycont[j][ii] = y;
							}
						}		// otherwise, the transom plane is forward of the hull section - subtract all of it
					} else {

//	Find the curvature of where the transom cuts the hull section

						if(jj < numlin) {
							t = 1.0 - t;
							zcont[jj][ii] = zline[jj][ii] - 0.5*t*(c+2.0*hd);
							ycont[jj][ii] = yline[jj][ii] + 0.5*t*(a+2.0*hb);
							zline[jj][ii] = z;
							yline[jj][ii] = y;

//	Set all hull lines below the transom at the intersection of the transom and the hull

							for(j = jj+1 ; j < numlin ; j++) {
								zline[j][ii] = z;
								yline[j][ii] = 0.0;
								zcont[j][ii] = z;
								ycont[j][ii] = 0.0;
							}

						} else {		// otherwise, the transom plane is behind the hull section - subtract none of it
							it1 = numtran;
						}
					}
				}

//	Restore start and end section indices

				for(j = 0 ; j < numlin ; j++) {
					stsec[j] = save_stsec[j];
					ensec[j] = save_ensec[j];
				}

//	Set section index to first interpolated transom section, and limit to the last

				i = count;	// proceed through transom sections
				ilast = tlast + 1;//last + 1;
			} else {
				i = ihull;
			}

#else
			i = ihull;
#endif

do_section:
			calcar(i,0,numlin,1,1,
				&area1,&area2,
				&hbeam1,&hbeam2,
				&cl1,&cl2,
				x1,xsect[i],
				&sarm,&volu,calcdisp,
				&xsum,&wplane,
				wl,density,&lwl,
				&xsta,&xend,&sumlcf,&summct,
				&ww1,&ww2,&rm1,&rm2,
				&sumysqdif,&sumyoffs);
			lwl_loc = lwl;

			if(area2 >= garea) {/* ... maximum area */
				volforw = volu;
				xatbwl = xsect[i];
				garea = area2;
			}

			dz1 = dz2;
			dz2 = wl - (cosa*zline[0][i]-sina*yline[0][i] + beta*xsect[i]);
			dz = 0.5*(dz1+dz2);		/* mean freeboard */
			cl = 0.5*(cl1+cl2);		/* mean waterline clearance */
			dx = xsect[i]-x1;
			AXabove += 0.5*dx*(dz1*(x1+0.33333*dx) + dz2*(x1+0.66667*dx));	/* area to sheerline */
			if(cl1 > 0.0) {	/* hull commenced clear of water */
				Aabove += dx*(dz-cl);
				if(cl2 > 0.0) {	/* hull remained clear of water */
					AZabove += 0.16667*dx*((dz2*dz2+dz1*dz2+dz1*dz1) - (cl2*cl2+cl1*cl2+cl1*cl1));
					AXabove -= 0.5*dx*(cl1*(x1+0.33333*dx) + cl2*(x1+0.66667*dx));
				}
				else {	/* hull entered water: xsta is waterline entry point */
					dx2 = xsect[i] - xsta;
					dx1 = xsta - x1;
					z = dx2*0.5*cl2; 	/* immersed area, negative */
					Abelow -= z;		/* positive increment to Abelow */
					Aabove += z;

					ax = -z*(x1+0.66667*dx1); /* positive term */
					AXbelow += ax;	/* positive contribution */

					AXabove -= 0.5*dx1*cl1*(x1+0.33333*dx1);	/* subtract positive vacant area term */
					AXbelow -= 0.5*dx1*cl2*(xsta+0.66667*dx2);
					AZbelow -= z*0.333333*cl2;/* negative contribution */
					dz = dz1+(dz2-dz1)*dx1/dx;
					AZabove += 0.16667*(dx1*(dz1*dz1+dz*dz1+dz*dz - cl1*cl1)
						    + dx2*(dz*dz+dz*dz2+dz2*dz2));
				}
			}
			else {	/* hull commenced in water */
				if(cl2 < 0.0) {		/* hull ended in water */
					Abelow -= dx*cl;	/* positive area */
					AZbelow += 0.16667*dx*(cl1*cl1+cl1*cl2+cl2*cl2);
					AXbelow -= 0.16667*dx*(cl1*(3.0*x1+dx)+cl2*(3.0*x1+2.0*dx));
					Aabove += dx*dz;
					AZabove += 0.16667*dx*(dz1*dz1+dz1*dz2+dz2*dz2);
				}
				else {			/* hull ended out of water */
					dx2 = xsect[i] - xend;	/* distance clear of water */
					dx1 = xend - x1;		/* distance in water */
					z = dx1*0.5*cl1;		/* immersed area, negative */
					Abelow -= z;
					Aabove += dx*(dz-cl) + z;
					AZbelow += 0.33333*z*cl1;

					ax = 0.5*dx1*cl1*(x1+0.33333*dx1); /* negative area contribution */
					AXbelow -= ax;
					AXabove += ax;
					dz = dz1+(dz2-dz1)*dx1/dx;
					AZabove += 0.16667*(dx2*(dz2*dz2+dz*dz2+dz*dz - cl2*cl2)
						    + dx1*(dz*dz+dz*dz1+dz1*dz1));
				}
			}
			x1 = xsect[i];
		}

		if(i < tlast) {			// more transom sections to do
			i++;
			goto do_section;
		} else if(i == tlast) {	//	When all interpolated sections have been used, do the hull section
			i = ihull;
			ilast = count;
			tlast = -1;
			goto do_section;
		}
	}
	volstern = volu - volforw;
	draft = tc;
	vol = 0.0;

	/*	Calculate floodable tank contributions

	Volume, volu forward and volu aft are altered only for those
	tanks which are "leaky", in which cases they are reduced.
	Displacement is decreased for each tank, using its nominated
	contents density or water density
	Waterplane area is altered only for those tanks which are "leaky",
	in which cases it is reduced
	LCB, LCF and MCT are altered only for tanks which are leaky
	Maximum area, midships position, waterline length, waterline entry
	point, maximum waterline beam and draft are unaltered
	Small-amplitude righting moment is affected only by those tanks
	which are "leaky"
	*/

#ifndef STUDENT

	for(i = 0 ; i < ntank ; i++) {
		fl_walev[i] = tanklevel(i,wl,&tanksarm,&tankxsum,&tanklcfv,
			&tankmct,&tankplane,&vol);

		/*	All returned terms are negative, because tanks are defined back-to-front	*/
		tanksarm  = -tanksarm;
		tankxsum  = -tankxsum;
		tanklcfv  = -tanklcfv;
		tankmct   = -tankmct;	/* includes density */
		tankplane = -tankplane;
		vol       = -vol;

		/*	find the 100% permeability mass	*/
		if(fl_fixed[i] && spgrav > 0.0)
			tankrsg = fl_spgra[i]/spgrav;
		else
		    tankrsg = 1.0;
		density = densit[numun]*tankrsg;
		tankmass[i] = vol * density;

		/*	tankmct and tanksarm are calculated using the density of the tank contents	*/

		tanktfsm[i] = tanksarm*tankrsg;
		if(tankplane > 0.0)
			tanklfsm[i] = tankmct*tankrsg - density*tanklcfv*tanklcfv/tankplane;
		else
		    tanklfsm[i] = 0.0;

		if(fl_fixed[i]) {
			/* tanks with defined contents are largely treated as static loads, so do not affect
			items like waterplane area - they are allowed to affect stability, though.
			*/
			summct -= tanklfsm[i];
			sarm -= tanktfsm[i];
		}
		else {

			/*	Leaky tanks contribute to all hydrostatic terms in the negative -
			i.e., are included as part of the hull
			*/
			wplane -= tankplane;
			sumlcf -= tanklcfv;

			/*	There an no centroid corrections for mct and free surface
			calculations, because the water level is fixed at the
			waterline.
			*/
			summct -= tankmct;
			sarm -= tanksarm;
		}

		/*	"xsum" includes water density		*/
		if(tankmass[i] > 0.0) {
			tanklcg[i] = tankxsum/tankmass[i];
		}
		else {
			tanklcg[i] = -999.9;
		}

		/*	adjust for permeability		*/
		tankmass[i] *= fl_perm[i];

		/*	NOTE: no adjustment to free surface moments - we don't know
		where the volume elements reducing permeability are
		*/
		if(tankplane > 0.0) {
			tanklcf[i] = tanklcfv / tankplane;
		}
		else if(tankmass[i] <= 0.0) {
			xlcf = -999.9;
			tanklcf[i] = -999.9;
		}
	}
#endif

	if(wplane > 0.0 && lwl > 0.0) {
		xlcf = sumlcf / wplane;
		mct = summct - densit[numun]*xlcf*xlcf*wplane;
		lcf = (xlcf - xentry) / lwl;
		mpi = wplane * densit[numun];
	}
	else if(vol <= 0.0) {
		xlcf = 999.99;
	}

	if(xsta >= 1.0e+16 || xend <= -1.0e+16)
		xmid = 999.99;
	else
		xmid = 0.5*(xsta+xend);

	/*	xcofb reflects the contributions of floodable
	tanks, but not of fixed-volume tanks
	*/
	if(volu > 0.0)
		xcofb = xsum / (volu*densit[numun]);
	else
	    xcofb = 999.99;

	if(lwl > 0.0)
		lcb = (xcofb - xentry) / lwl;
	else
	    lcb = -0.999;

	tc = -draft;	/*	DRAUGHT MADE POSITIVE	*/
	density = densit[numun];

	if(Aabove > 0.0) {
		Zabove = AZabove / Aabove;
		Xabove = AXabove / Aabove;
	}
	else {
		Zabove = 0.0;
		Xabove = -999.9;
	}

	if(Abelow > 0.0) {
		Zbelow = AZbelow / Abelow;
		Xbelow = AXbelow / Abelow;
	}
	else {
		Zbelow = 0.0;
		Xbelow = -999.9;
	}
}

#ifndef STUDENT
#undef sarm
REAL tanklevel(int tank,REAL wl0,REAL *sarm,REAL *xsum,REAL *sumlcf,
	REAL *summct,REAL *tankplane,REAL *vol)
{
	int i,j,j1,j2;
	REAL fract,tankvol,fullvol;
	REAL zmn,zmx,y,z;
	int repeats;
	REAL watlev,oldwatlev,dz;
	REAL area1,area2,ignore,x1,ww1,ww2,rm1,rm2;
	REAL sumysqdif,sumyoffs,hbeam1,hbeam2;

	tankmode = TRUE;
	j1 = fl_line1[tank];
	j2 = fl_line1[tank+1];
	fract = fl_fract[tank];
	fullvol = fl_volum[tank];
	if(fl_fixed[tank] == 1) {	/* volume nominated */
		tankvol = fract;
		fract = fract/fullvol;
	}
	else {			/* percent fraction nominated */
		fract *= 0.01;
		tankvol = fullvol*fract;
	}
	ww2 = 0.0;
	rm2 = 0.0;

	zmn = 1.0e+30;
	zmx = -1.0e+30;
	for(i = stsec[j1] ; i <= ensec[j1] ; i++) {
		for(j = j1 ; j < j2 ; j++) {
			z = fmul(cosa,zline[j][i]);
			y = fmul(sina,yline[j][i]);
			if(fl_right[tank])
				z = fadd(z,y);
			else
			    z = fsub(z,y);
			z += beta*xsect[i];
			if(flt(z,zmn)) zmn = z;
			if(fgt(z,zmx)) zmx = z;
		}
	}

	if(!fl_fixed[tank]) {
		repeats = 1;
		watlev = max(zmn,wl0);
		density = densit[numun];
	}
	else {
		repeats = 20;
		watlev = fl_walev[tank];
		density = fl_spgra[tank]*densit[numun]/spgrav;
	}
	density *= fl_perm[tank];

	do {
		i = stsec[j1];
		x1 = xsect[i];
		*vol = 0.0;
		*tankplane = 0.0;
		*sarm = 0.0;
		*sumlcf = 0.0;
		*summct = 0.0;
		*xsum = 0.0;
		sumysqdif = 0.0;
		sumyoffs = 0.0;
		hbeam2 = 0.0;
		ww2 = 0.0;
		for( ; i <= ensec[j1] ; i ++) {
			calcar(i,j1,j2,!fl_right[tank],fl_right[tank],
				&area1,&area2,
				&hbeam1,&hbeam2,
				&ignore,&ignore,
				x1,xsect[i],
				sarm,vol,&ignore,
				xsum,tankplane,
				watlev,density,&ignore,
				&ignore,&ignore,sumlcf,summct,
				&ww1,&ww2,&rm1,&rm2,
				&sumysqdif,&sumyoffs);
			x1 = xsect[i];
		}

		/*	Iterate to 0.01% accuracy	*/

		dz = fadd(tankvol,*vol);  /* volume change (*vol < 0) */
		if(fle(fabf(dz),fmul(0.0001,fullvol))) break;

		if(fneg(*tankplane) && fnoz(*vol)) {	/* vol and tankplane are both negative */
			dz = fdiv(dz,*tankplane);
			oldwatlev = watlev;
			watlev = fadd(watlev,dz);
			if(flt(watlev,zmn)) {
				watlev = zmn*fract + oldwatlev*(1.0-fract);
			}
			else if(fgt(watlev,zmx)) {
				watlev = zmx*(1.0-fract) + oldwatlev*fract;
			}
		}
		else if(repeats > 1) {
			watlev = zmx - fract*(zmx - zmn);
		}
	}
	while(--repeats);

	/*	Correct for off-centre tanks	*/

	y = xsect[ensec[j1]]-xsect[stsec[j1]];
	if(y != 0.0) sumyoffs *= 0.5/y;
	*sarm += density*sumyoffs*(sumyoffs* *tankplane + sumysqdif);

	tankmode = FALSE;
	return watlev;
}

#endif
