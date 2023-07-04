/* Hullform component - graf-hfw.h
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
 
#define	NOHARDCOPY	0
#define	SYSTEMPRINTER	1
#define	HPGL		2
#define	TEKTRONIX	3
#define	REGIS		4
#define DXFFILE		5
#define	METAFILE	6
#define	CLIPBOARD	7

extern	int	persp;
extern	int	device,xcur,ycur;
extern	int	drawcol;	/* current drawing colour */
extern	int	chan,ichan;
extern	REAL	xpersp,ypersp,zpersp;
extern	REAL	angx,angy,angz;
extern	REAL	yview,zview;
extern	REAL	axx,axy,axz, ayx,ayy,ayz, azx,azy,azz;
extern	REAL	xorigin,xmin,xmax,xbox;
extern	REAL	yorigin,ymin,ymax,ybox;
extern	int	maxx,maxy;
extern	REAL	sinpp,cospp;
