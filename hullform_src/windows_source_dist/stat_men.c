/* Hullform component - stat_men.c
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

static	char	*secun[2] = {
	"cm","inch"};
void	pjust(REAL value);
extern	int scrcopy;
void	finish_stats(void);
void	fixto8(char *s);
extern int repaint;
extern int balanced;
extern int persp;
REAL	hullsurf;
void	SwapBuf(void);

MENUFUNC stat_settings()
{
	extern	REAL	sina,cosa;
	extern	REAL	beta;
	extern	REAL	invert;

	zcofm *= invert;
	wl *= invert;

	if(getdlg(STATSETT,
		INP_REA,(void *) &heel,
		INP_REA,(void *) &pitch,
		INP_REA,(void *) &wl,
		INP_REA,(void *) &disp,
		INP_REA,(void *) &xcofm,
		INP_REA,(void *) &zcofm,-1)) {
		sina = sind(heel);
		cosa = cosd(heel);
		beta = tan(0.01745329*pitch);
		balanced = 0;
	}
	wl *= invert;
	zcofm *= invert;
}

MENUFUNC find_float()
{
	REAL calcdisp;

	/*	ITERATION TO ACHIEVE BALANCE BETWEEN MASS AND DISPLACED VOLUME	*/

	if(count <= 0) return;
	repaint = 0;
	persp = FALSE;
	clrtext();
	balanc(&calcdisp,1);
	if(calcdisp > 0.0) {
		finish_stats();
		show_stats();
	}
#ifndef linux
	update_func = find_float;
#endif
	print_func = find_float;
}

#ifndef STUDENT
void bal_all(void);

MENUFUNC balance_all()
{
	repaint = 0;
	bal_all();
	finish_stats();
	show_stats();
#ifndef linux
	update_func = balance_all;
#endif
	print_func = balance_all;
}

void bal_all(void)
{
	extern	int	xleft,ytop,xright,ybottom,ychar;
	REAL	calcdisp;
	REAL	delheel;
	extern	HDC hDC;

	update_func = NULL;
	persp = FALSE;

	/*	ITERATION TO ACHIEVE BALANCE BETWEEN MASS AND DISPLACED VOLUME	*/

	if(count <= 0) return;

	do {
		balanc(&calcdisp,1);
		if(calcdisp < 0.0) return;
		findrm();
		delheel = fmul(57.29578,fdiv(fmul(gz,disp),sarm));
		(*init)();
		clrtext();
		preaxy(lx,2,"Change of heel %.2f   ",-delheel);
		(*endgrf)();
		heel = fsub(heel,delheel);
		sina = sind(heel);
		cosa = cosd(heel);
	}
	while(fgt(fabf(delheel),0.04));
}
#endif

MENUFUNC find_displ()
{
	REAL	calcdisp;
	extern int evalnowarning;
	extern int context_id;

	if(count <= 0) return;
	persp = FALSE;

	if(!evalnowarning) {
		if(!getdlg(EVALWARN,INP_LOG,(void *) &evalnowarning,-1)) return;
	}

	/*	CALCULATE DISPLACEMENT FOR A GIVEN WATERLINE:  MUCH SIMPLER	*/

	repaint = 0;
	huldis(&calcdisp);
	disp = volu*densit[numun];
	xcofm = xcofb;
	finish_stats();
	show_stats();
	update_func = find_displ;
	print_func = update_func;

}

MENUFUNC write_stats()
{
#ifdef DEMO
	not_avail();
#else
	char	*f1 = "%-6s%12.3f %s\n";
	extern	char	*var[];
	char	buffer[MAX_PATH];
	FILE	*fp;
	char	*cp;
	int	i;
	int	nn	= numun >> 1;

#ifdef SHAREWARE
	nagbox();
#endif
	if(open_text(&fp,filedirnam,"*.txt")) {
		fprintf(fp,f1,var[0],wl,lenun[numun]);
		fprintf(fp,f1,var[1],heel,"deg");
		fprintf(fp,f1,var[2],disp,masun[numun]);
		fprintf(fp,f1,var[3],pitch,"deg");
		fprintf(fp,f1,var[4],xcofm,lenun[numun]);
		fprintf(fp,"%-6s%12.3f cu %s\n",var[5],volu,lenun[numun]);
		fprintf(fp,f1,var[6],lcb*100.0,"%");
		fprintf(fp,f1,var[7],lcf*100.0,"%");
		strcpy(buffer,masun[numun]);
		strcat(buffer," ");
		strcat(buffer,lenun[numun]);
		cp = buffer + strlen(buffer);
		fprintf(fp,f1,var[8],gz,lenun[numun]);
		fprintf(fp,f1,var[9],invert*zmeta,lenun[numun]);
		strcpy(cp," per ");
		strcpy(cp+5,secun[nn]);
		fprintf(fp,f1,var[10],mct,buffer);
		strcpy(buffer,masun[numun]);
		strcat(buffer," per ");
		strcat(buffer,lenun[numun]);
		fprintf(fp,f1,var[11],mpi,buffer);
		strcpy(buffer,"sq ");
		strcpy(buffer+3,lenun[numun]);
		fprintf(fp,f1,var[12],wplane,buffer);
		fprintf(fp,f1,var[13],wetsur,buffer);
		fprintf(fp,f1,"HULSUR",hullsurf,buffer);
		fprintf(fp,f1,var[54],surLCG,lenun[numun]);
		fprintf(fp,f1,var[55],invert*surVCG,lenun[numun]);
		fprintf(fp,f1,var[14],loa,lenun[numun]);
		fprintf(fp,f1,var[15],lwl,lenun[numun]);
		fprintf(fp,f1,var[16],2.0*bmax,lenun[numun]);
		fprintf(fp,f1,var[17],bwl,lenun[numun]);
		fprintf(fp,f1,var[18],tc,lenun[numun]);
		fprintf(fp,f1,var[19],fm,lenun[numun]);
		for(i = 20 ; i < numvar ; i++) {
			if(	strcmp(var[i],"ZCOFM") == 0 ||
				strcmp(var[i],"ZCOFB") == 0 ||
				strcmp(var[i],"ZBASE") == 0) {
					fprintf(fp,f1,var[i],invert*varval[i],"");
			} else {
					fprintf(fp,f1,var[i],varval[i],"");
			}
		}

		fclose(fp);
	}
	update_func = NULL;
#endif
}

void display_stats(int);
int	mid;
int	nn;
extern	int	xchar,ychar;
extern	int	xjust;
extern	int	graphics;

void first_stats()
{
	display_stats(0);
}

void show_stats(void)
{
	extern int dlgboxpos;

	if(count <= 0) return;
	nn = numun >> 1;

	display_stats(0);
	dlgboxpos = 1;	/* position bottom right */
	repaint = TRUE;
	(void) getdlg(STATSELE,
	    INP_PBF,(void *) display_stats,
	    INP_INI,(void*) first_stats,-1);
	update_func = NULL;
}

int redisplay_option = 0;

void redisplay_stats()
{
	display_stats(redisplay_option);
}

void display_stats(int option)
{
	REAL a;

	redisplay_option = option;
	update_func = redisplay_stats;
	REAL zcofm_total,vmom,mass;
#ifdef PROF
	mass = disp;
	vmom = disp * zcofm;
	for(tank = 0 ; tank < ntank ; tank++) {
		vmom += tankvcg[tank] * tankmass[tank];
		mass += tankmass[tank];
	}
	zcofm_total = vmom / mass;
#else
	zcofm_total = zcofm;
#endif

	xjust = 43;
	scrcopy = 1;
	clrtext();	/* also initialises hDC if needed */
	switch(option) {
	case 0:
		pstr("\nAt a heel angle of");
		pjust(heel);
		pstr("deg");
		pstr("\nPitch angle is");
		pjust(pitch);
		pstr("deg");
		pstr("\nWater density is");
		pjust(densit[numun]);
		pstr(masun[numun]);
		pstr(" per cu.");
		pstr(lenun[numun]);
		pstr("\nSinkage is");
		pjust(wl);
		pstr(lenun[numun]);
		pstr("\nVolume displaced is");
		pjust(volu);
		pstr("cu ");
		pstr(lenun[numun]);
		pstr("\nAssigned hull displacement is");
		pjust(disp);
		pstr(masun[numun]);
		pstr("\nTotal displacement is");
		pjust(volu*densit[numun]);
		pstr(masun[numun]);
		pstr("\nRighting moment is");
		pjust(fzer(gz) ?
			disp*(zcofm_total - zmeta)*0.01745329 : disp*gz);
		pstr(masun[numun]);
		pstr(" ");
		pstr(lenun[numun]);
		if(fzer(gz)) pstr(" per deg.");
		pstr("\nMass per unit immersion");
		pjust(wplane*densit[numun]);
		pstr(masun[numun]);
		pstr("/");
		pstr(lenun[numun]);
		pstr("\nMoment to change trim (MCT)");
		pjust(mct);
		pstr(masun[numun]);
		pstr(" ");
		pstr(lenun[numun]);
		pstr(" per ");
		pstr(secun[nn]);
		pstr("\nWaterplane area");
		pjust(wplane);
		pstr("sq ");
		pstr(lenun[numun]);
		pstr("\nWetted surface");
		pjust(wetsur);
		pstr("sq ");
		pstr(lenun[numun]);
		pstr("\nWhole hull surface area");
		pjust(hullsurf);
		pstr("sq ");
		pstr(lenun[numun]);
		pstr("\nWhole hull surface LCG");
		pjust(surLCG);
		pstr(lenun[numun]);
		pstr("\nWhole hull surface VCG");
		pjust(surVCG*invert);
		pstr(lenun[numun]);
		pstr("\nOverall length (Loa)");
		pjust(loa);
		pstr(lenun[numun]);
		pstr("\nWaterline length (Lwl)");
		pjust(lwl);
		pstr(lenun[numun]);
		pstr("\nMaximum beam (Bmax)");
		pjust(2.0*bmax);
		pstr(lenun[numun]);
		pstr("\nWaterline beam (Bwl)");
		pjust(bwl);
		pstr(lenun[numun]);
		pstr("\nDraught (T)");
		pjust(tc);
		pstr(lenun[numun]);
		pstr("\nMidsection freeboard (Fm)");
		pjust(fm);
		pstr(lenun[numun]);
		pstr("\nProfile area above waterline");
		pjust(Aabove);
		pstr("sq ");
		pstr(lenun[numun]);
		pstr("\nProfile area below waterline");
		pjust(Abelow);
		pstr("sq ");
		pstr(lenun[numun]);
		break;
	case 1:
		pstr("\nMidsection coefficient (Cm)");
		pjust(midsco);
		pstr("\nPrismatic coefficient (Cp)");
		pjust(prisco);
		pstr("\nBlock coefficient (Cb)");
		pjust(blocco);
		a = bwl * lwl;
		if(a > 0.0) {
			pstr("\nWaterplane area coefficient");
			pjust(wplane/a);
		}
		a = (xatbwl-xentry)*garea;
		if(a > 0.0) {
			pstr("\nForward prismatic coefficient");
			pjust(volforw/a);
		}
		a = (lwl-(xatbwl-xentry))*garea;
		if(a > 0.0) {
			pstr("\nStern prismatic coefficient");
			pjust(volstern/a);
		}
		break;
	case 2:
		xmid = xentry + 0.5*lwl;
		pstr("\nX-centre of mass (LCG)");
		pjust(xcofm);
		pstr(lenun[numun]);
		pstr("\n- distance forward of midships");
		pjust(xmid-xcofm);
		pstr(lenun[numun]);
		pstr("\nCentre of buoyancy (LCB)");
		pjust(lcb*100.0);
		pstr("%");
		pstr("\n- at");
		pjust(xcofb);
		pstr(lenun[numun]);
		pstr("\n- distance forward of midships");
		pjust(xmid-xcofb);
		pstr(lenun[numun]);
		pstr("\nCentre of flotation (LCF)");
		pjust(lcf*100.0);
		pstr("%");
		pstr("\n- at");
		pjust(xlcf);
		pstr(lenun[numun]);
		pstr("\n- distance forward of midships");
		pjust(xmid-xlcf);
		pstr(lenun[numun]);
		pstr("\nWaterline entry point at");
		pjust(xentry);
		pstr(lenun[numun]);
		pstr("\n\nZ-centre of mass (VCG)");
		pjust(invert*zcofm_total);
		pstr(lenun[numun]);
		pstr("\nVertical centre of buoyancy");
		pjust(invert*zcofb);
		pstr(lenun[numun]);
		pstr("\nMetacentre above waterline");
		pjust(wl - zmeta);
		pstr(lenun[numun]);
		pstr("\nMetacentre above baseline (KM)");
		pjust(-zmeta);
		pstr(lenun[numun]);
		pstr("\nMetacentre height (GM)");
		pjust(zcofm_total - zmeta);
		pstr(lenun[numun]);
		pstr("\n\nRighting lever (GZ)");
		pjust(gz);
		pstr(lenun[numun]);
		pstr("\nKN");
		pjust(kn);
		pstr(lenun[numun]);
		pstr("\n\nX-centre of area above waterline");
		pjust(Xabove);
		pstr(  "\nX-centre of area below waterline");
		pjust(Xbelow);
		pstr("\n\nZ-centre of area above waterline");
		pjust(Zabove);
		pstr(  "\nZ-centre of area below waterline");
		pjust(Zbelow);
		break;
	default:
		break;
	}
	graphics = 0;	/* text mode */
    SwapBuf();
}

void finish_stats(void)
{
	int i,ii,j;
	int in[3];
	REAL z0;
	extern int whole_surface;
	REAL zcofm_total,vmom,mass;
#ifdef PROF
	mass = disp;
	vmom = disp * zcofm;
	for(tank = 0 ; tank < ntank ; tank++) {
		vmom += tankvcg[tank] * tankmass[tank];
		mass += tankmass[tank];
	}
	zcofm_total = vmom / mass;
#else
	zcofm_total = zcofm;
#endif

	/*	EVALUATE RIGHTING MOMENT FOR THIS HULL STATE	*/

	findrm();

	xstem = xsect[0] - dxstem();
	xstern = xsect[count-1];
	gravity = g[numun];
	loa = xstern - xstem;
	entang = halfen();
	wetsur = hulsur(sina,cosa)+hulsur(-sina,cosa);
	whole_surface = TRUE;
	hullsurf = 2.0*hulsur(0.0,1.0);
	whole_surface = FALSE;

	bmax = 0.0;
	zbase = -1.0e+30;
	for(j = 0 ; j < numlin ; j++) {
		for(i = max(1,stsec[j]) ; i <= ensec[j] ; i++) {
			if(zbase < zline[j][i]) zbase = zline[j][i];
			if(yline[j][i] > bmax) bmax = yline[j][i];
		}
	}

	mid = count >> 1;
	fm = wl-beta*xsect[mid]-hwl[mid]-cosa*zline[0][mid]-
	    fabs(sina)*yline[0][mid];

	/*	PRISMATIC, BLOCK AND MIDSECTION COEFFICIENTS:	*/

	if(lwl > 0.0 && garea > 0.0 && bwl > 0.0 && tc > 0.0) {
		prisco = volu/(garea*lwl);
		blocco = volu/(bwl*lwl*tc);
		midsco = garea/(bwl*tc);
		mct = (mct - disp*(zcofb-zcofm_total))/loa;
	}
	else {
		prisco = -1.0;
		blocco = -1.0;
		midsco = -1.0;
		mct = -1.0;
	}
	if(numun < 2) {
		mct = mct*0.01;		/* per cm	*/
	}
	else {
		mct = mct*0.0833333;	/* per inch	*/
	}

	xmid = xentry + 0.5*lwl;
	in[0] = 0;
	in[1] = count/2;
	in[2] = count-1;
	for(i = 0 ; i < 3 ; i++) {
		ii = in[i];
		z0 = wl - beta*xsect[ii]-hwl[ii];
		varval[48+i] = -z0;
		/* draft forward, midships and aft */
		varval[51+i] = z0-cosa*zline[0][ii]-fabs(sina)*yline[0][ii];
		/* freeboard forward, midships and aft */
	}
}

