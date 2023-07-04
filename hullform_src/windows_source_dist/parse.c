/* Hullform component - parse.c
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

extern int level[];
extern char op[];
extern char *par[];	/* pointers to parameters in evaluation string	*/

/**********************************************************************/

/*	This routine parses a line into parameters and single-character
	operators. The parameters are returned as null-terminated
	substrings in the original string. The operators are returned in
	array op[].
*/

int isfilechar(unsigned char c);

int parse(char *lp)
{
    char *p;
    extern int parse0;
    extern int brlev;
    int n = parse0;
    int prevlev;
    brlev = 0;
    op[0] = 0;
    parse0 = 0;

    p = lp;	/* lp is input pointer; p is output pointer */

    while(*lp) {

	while(isspace(*lp)) lp++;

/*	Work through leading parentheses			*/

	while(*lp == '(' || *lp == '[' || isspace(*lp)) {
	    if(!isspace(*lp++)) brlev++;
	}

	if(*lp == '+') lp++;
	par[n] = p;

/*	Save parameter						*/

	if(*lp == '"') {
	    *p++ = *lp++;
	    while(*lp) {
		if((*p++ = *lp++) == '"') break;
	    }
	    if(*lp == 0) return 0;
	} else {
	    if(*lp == '-') *p++ = *lp++;
	    while(isalnum(*lp) || *lp == '.' || *lp == '_') *p++ = *lp++;
	}

/*	Work through following parentheses			*/

	prevlev = brlev;
	while(*lp == ')' || *lp == ']' || isspace(*lp)) {
	    if(!isspace(*lp++)) brlev--;
	}
	if(*lp == '(' || *lp == '[') brlev++;

	level[++n] = min(prevlev,brlev);

	if(isalpha(*lp))
	    op[n] = 0;	/* no operator */
	else if(*lp)
	    op[n] = *lp++;	/* save operator or function code	*/
				/* ( '(' or '[' )			*/
	if(*lp == 0) break;
	*p++ = 0;		/* terminate parameter (null goes where	*/
				/* operator was)			*/
    }
    par[n] = p;
    *p = 0;
    op[n+1] = 0;
    level[n] = -1;
    return n;
}

/*	Parse DDE command, removing parentheses, quotes etc.	*/

int DDEparse(char *lp)
{
    int n = 0;		/* word count				*/
    char reqchar;	/* required character			*/
    char prevchar;

    while(*lp) {

/*	Work through leading spaces, quotes and parentheses	*/

	while(isspace(*lp) || *lp == '"' || *lp == '(' || *lp == '[') prevchar = *lp++;
	switch(prevchar) {
	  case '"':
	    reqchar = '"';
	    break;
	  case '(':
	    reqchar = '"';
	    break;
	  case ')':
	    reqchar = '"';
	    break;
	  case ']':
	    reqchar = '"';
	    break;
	  default:
	    reqchar = ' ';
	}

/*	Mark start of word					*/
	par[n++] = lp;

/*	Move to end of word					*/

	while(*lp && *lp != reqchar) lp++;

/*	Mark end of word					*/

	*lp++ = 0;

/*	Work through following spaces, quotes and parentheses		*/

	while(isspace(*lp) || *lp == '"' || *lp == ')' || *lp == ']') lp++;
    }
    return n;
}

