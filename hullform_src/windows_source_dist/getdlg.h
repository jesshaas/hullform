/* Hullform component - getdlg.h
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
 
#define	DLGEDIT		120
#define DLGTEXT		140
#define DLGRBN1		220
#define DLGRBN2		230
#define DLGRBN3		240
#define DLGPB		260
#define DLGCB		280
#define DLGRBF		300
#define DLGLOG		320
#define	DLGSTAT		340
#define	DLGPBF		360
#define DLGLBX	        380

#define INP_INT	1
#define INP_INX 2
#define INP_REA 3
#define INP_REX 4
#define INP_STR 5
#define INP_RBN 6
#define INP_MEN 6
#define INP_RBX 7
#define INP_RBF 8
#define INP_LOG 9
#define INP_PBF	10
#define INP_RBR 11
#define INP_CBR 12
#define INP_LBX 13
#define INP_STX 14
#define INP_CBX 15
#define INP_INI	16

#define DLGINC	499
#define DLGDEC	498

#ifdef linux

#ifndef lin_getd_h
#include "lin_getd.h"
#endif
int getdlg(RCFUNC *name, ...);

#else

#include "win_getd.h"
int getdlg(int name, ...);

#endif

typedef struct {
    int index;     /* index of result in table */
    char *string;  /* pointer to result */
    char *table[1]; /* table of strings for listbox, null string terminator */
} dlglistbox;

