/* Hullform component - dxf_frames_output.c
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
 
#ifdef EXT_OR_PROF

/*    AutoCAD "DXF"-format output of hull frames    */

#include "hulldesi.h"

#ifdef linux
char *searchpath(char *file);
#include <Xm/Text.h>
#endif


void make_outline(int i,REAL *tn,REAL del,int mode,FILE *fp,REAL side);
extern	REAL *outl_thickness;
extern	REAL mousehole_radius,notch_width,notch_height;
char dxf_frame_filename_pattern[MAX_PATH] = "frame-%03d.dxf";

MENUFUNC dxf_frames_output()
{
	extern int	numbetw,numbetwl;
	int		i,j;
	char	dxf_frame_filepath_pattern[MAX_PATH];
	char	dxf_frame_filepath[MAX_PATH];
	char	filename[MAX_PATH];
	int		use[maxsec];
	char	*p1;
	extern	FILE *fp,*hp;
	void	def_dxf(int,HWND);
	char	input[MAX_PATH];
	static int both_sides = 0;

	if(count <= 0) return;

	strcpy(dxf_frame_filepath_pattern,dxfdirnam);
#ifdef linux
	strcat(dxf_frame_filepath_pattern,"/");
#else
	strcat(dxf_frame_filepath_pattern,"\\");
#endif
	strcat(dxf_frame_filepath_pattern,dxf_frame_filename_pattern);

	update_func = NULL;
	cls(0);
	strcpy(input,"ALL");
	if(!getdlg(DXFFRAMES,
		INP_STR,(void *) input,
		INP_PBF,def_dxf,
		INP_LOG,(void *) &both_sides,
		INP_STR,(void *) dxf_frame_filepath_pattern,
		-1) ||
	    !multproc(input,use,count)) return;
#ifdef DEMO
	message("Demonstration version:\nno actual output");
#else

	for(i = 0 ; i < count ; i++) {
		if(use[i]) {
			sprintf(filename,dxf_frame_filepath_pattern,i);

/*    perform output					*/

			if( (fp = fopen(filename,"wt")) == NULL) {
				message("Can not open DXF output file for writing");
				return;
			}

/*	Header lines		*/

			fputs("0\nSECTION\n2\nHEADER\n",fp);

			hp = fopen("header.dxf","rt");
			if(hp == NULL) {
				p1 = searchpath("header.dxf");
				if(p1 != NULL) hp = fopen(p1,"rt");
			}
			if(hp != NULL) {
				while(fgets(input,80,hp) != NULL) fputs(input,fp);
				fclose(hp);
			}

			fputs("0\nENDSEC\n",fp);

/*	Tables						*/

			fputs("0\nSECTION\n2\nTABLES\n",fp);

			hp = fopen("tables.dxf","rt");
			if(hp == NULL) {
				p1 = searchpath("tables.dxf");
				if(p1 != NULL) hp = fopen(p1,"rt");
			}
			if(hp != NULL) {
				while(fgets(input,80,hp) != NULL) fputs(input,fp);
				fclose(hp);
			}

//	Layers - only one

			fprintf(fp,"0\nTABLE\n2\nLAYER\n70\n1\n");
			fprintf(fp,"0\nLAYER\n2\nF%d\n70\n0\n62\n1\n6\nCONTINUOUS\n",i);

			fputs("0\nENDTAB\n0\nENDSEC\n",fp);

			fprintf(fp,"  0\nSECTION\n  2\nENTITIES\n");

/* begin entities section */

			ensure_polyline('F',i,0);
			make_outline(i,outl_thickness,0.0,4,fp,1.0);
			seqend();
			if(both_sides) {
				ensure_polyline('F',i,0);
				make_outline(i,outl_thickness,0.0,4,fp,-1.0);
				seqend();
			}

/*    End of output		    */

			fprintf(fp,"  0\nENDSEC\n  0\nEOF\n");

			fclose(fp);
		}	/* end loop through sections */
	}
#endif
}


#endif

