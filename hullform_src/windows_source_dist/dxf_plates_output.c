/* Hullform component - dxf_plates_output.c
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
 
#ifdef PLATEDEV

/*    AutoCAD "DXF"-format output of hull plates    */

#include "hulldesi.h"

char dxf_plate_filename_pattern[MAX_PATH] = "plate-%03d-%03d.dxf";

MENUFUNC dxf_plates_output()
{
	char	dxf_plate_filepath_pattern[MAX_PATH];
	extern int hcpydev;
	int save_lin;

	strcpy(dxf_plate_filepath_pattern,dxfdirnam);
#ifdef linux
	strcat(dxf_plate_filepath_pattern,"/");
#else
	strcat(dxf_plate_filepath_pattern,"\\");
#endif
	strcat(dxf_plate_filepath_pattern,dxf_plate_filename_pattern);

	update_func = NULL;
	cls(0);
	if(!getdlg(DXFPLATES,
		INP_STR,(void *) dxf_plate_filepath_pattern,
		-1)) return;
#ifdef DEMO
	message("Demonstration version:\nno actual output");
#else

	setup(5);
	save_lin = plate_lin;
	for(plate_lin = 1 ; plate_lin < numlin ; plate_lin++) {
		if(rulings[plate_lin] > 0) {
			sprintf(cur_port,dxf_plate_filepath_pattern,plate_lin,plate_lin+1);
			rollout();
		}
	}
	plate_lin = save_lin;
	setup(scrdev);
#endif
}

#endif

