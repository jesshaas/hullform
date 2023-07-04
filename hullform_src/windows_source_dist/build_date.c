/* Hullform component - build_date.c
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
 
#include <time.h>
#include <stdio.h>

int main()
{
	time_t timeval;
	struct tm *timep;
	char *getenv(char *);
	char *subver = getenv("HFN");
	char *mon[] = {"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};

	if(subver == NULL) subver = "00";

	timeval = time(NULL);
	timep = localtime(&timeval);
	printf("#define BUILD_DATE \"Build date %02d-%s-%04d\"\n#define VERSION \"Version 9.%s\"\n",
		timep->tm_mday,mon[timep->tm_mon],1900+timep->tm_year,subver);
	return 0;
}

