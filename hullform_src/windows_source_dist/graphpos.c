/* Hullform component - graphpos.c
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
 
#ifdef linux

#include "hulldesi.h"
#include <time.h>
#include <fcntl.h>

#define	xoffset	18		/* displace plot 18/72" to right */
static int segments = 0;

/************************************************************************/

/*	Postscript driver						*/

/************************************************************************/

FILE *psf;
REAL x_ps,y_ps;

void postin()
{
	extern	int	penup,xmaxi,ymaxi;
	extern	LOGFONT prfont;	/* logical font structure */
	time_t secs;
	char font[200];
	extern int pointsize;

	psf = fopen("hullform_output.ps","wt");
	time(&secs);
	xmaxi = 595;
	ymaxi = 792;
	strcpy(font,prfont.lfFaceName);
	if(prfont.lfWeight == 700) strcat(font,"-Bold");
	if(prfont.lfItalic) strcat(font,"-Italic");
	pointsize = abs(prfont.lfHeight);

	fprintf(psf,"%%!PS-Adobe-2.0\n%%DocumentFonts: %s\n%%Title: Hullform Postscript graphic\n%%Creator: HULLFORM\n",font);
	fprintf(psf,"%%CreationDate: %s",ctime(&secs));
	fprintf(psf,"%%BoundingBox: 0 0 %d %d\n%%ColorUsage: Color\n%%EndComments\n",xmaxi,ymaxi);
	fprintf(psf,"/M {moveto} bind def\n/L {lineto} bind def\n/T {show} bind def\n/F {lineto closepath fill} bind def\n");
	fprintf(psf,"0 setlinewidth\n1 setlinecap\n1 setlinejoin\n[] 0 setdash\n0 setgray\n");
	fprintf(psf,"/%s findfont %d scalefont setfont\n",prfont.lfFaceName,pointsize);
	fprintf(psf,"/DeviceRGB setcolorspace\n");

	segments = 0;
	penup = 1;
}

/*	screen-clear is invoked as formfeed 	*/

void postcl()
{
	extern	int	penup;
	penup = 1;
	segments = 0;
}

/*	End graphics: close output file, send to printer (ensuring that it is always finished with when the command is completed), then remove it 	*/

/*	Note that the availability of ghostscript when the printer is not PostScript is presumed.	*/

void posten()
{
	if(segments > 0) fputs("stroke\n",psf);
	fputs("showpage\n%%Trailer\n",psf);
	fclose(psf);
	system("cat hullform_output.ps | lpr");
	unlink("hullform_output.ps");
}

/*	new line: remember for use in next "draw()" call	*/

void postnl()
{
	extern	int	penup;
	penup = 1;
	if(segments > 0) fputs("stroke\n",psf);
	segments = 0;
}

/*	Colour: colours 1, 2 and 3 are 1/2, 1 and 1 1/2 units wide	*/

void postcr(INT ncol)
{
	float r,g,b;
	int col;
	extern COLORREF scrcolour[];
	extern int linewidth[],linestyle[];

	if(segments > 0) {
		fputs("stroke\n",psf);
		segments = 0;
	}
	col = scrcolour[ncol];
	r = ((float) GetRValue(col)) / 255.0;
	g = ((float) GetGValue(col)) / 255.0;
	b = ((float) GetBValue(col)) / 255.0;
	fprintf(psf,"%.3f %.3f %.3f setrgbcolor\n",r,g,b);
	if(ncol > 1) {
		fprintf(psf,"%.1f setlinewidth\n",0.5 * (float) linewidth[ncol-2]);
		switch(linestyle[ncol-2]) {
			case 0:
			default:
				fprintf(psf,"[] 0 setdash\n");
				break;
			case 1:
				fprintf(psf,"[4 2] 0 setdash\n");
				break;
			case 2:
				fprintf(psf,"[1] 0 setdash\n");
				break;
			case 3:
				fprintf(psf,"[4 1 1 1] 0 setdash\n");
				break;
			case 4:
				fprintf(psf,"[4 1 1 1 1 1] 0 setdash\n");
				break;
		}
	} else {
		fprintf(psf,"0 setlinewidth\n");
		fprintf(psf,"[] 0 setdash\n");
	}
}

/*	Move to a point	*/

void postmv(REAL x,REAL y)
{
	char	str[18];
	extern	int	penup;
	penup = 0;
	if(segments > 0) fputs("stroke\n",psf);
	x_ps = x;
	y_ps = y;
	trans(&x,&y);
	sprintf(str,"%.1f %.1f M\n",xoffset+x,y);
	fputs(str,psf);
	segments = 0;
}

/*	Draw a line	*/

void postdr(REAL x,REAL y)
{
	extern	int	penup;
	char	str[18];
	if(penup) {
		postmv(x,y);
	} else {
		x_ps = x;
		y_ps = y;
		trans(&x,&y);
		sprintf(str,"%.1f %.1f L\n",xoffset+x,y);
		fputs(str,psf);
		if(++segments > 64) {
			segments = 0;
			fputs("stroke\n",psf);
			sprintf(str,"%.1f %.1f M\n",xoffset+x,y);
			fputs(str,psf);
		}
	}
}

/*	write a character string	*/

void add_escapes(char *str,char text[])
{
	char *ip,*op,*r,*l;
	int n;
	ip = str;
	op = text;
	r = strchr(ip,')');
	l = strchr(ip,'(');
	while(r != NULL || l != NULL) {
		if(l == NULL || r != NULL && (int) (l - r) > 0) {	// right bracket next
			n = (int) (r - ip);
			strncpy(op,ip,n);
			op += n;
			*op++ = '\\';
			*op++ = ')';
			ip = r + 1;
			r = strchr(ip,')');
		} else if(l != NULL) {								// left bracket next
			n = (int) (l - ip);
			strncpy(op,ip,n);
			op += n;
			*op++ = '\\';
			*op++ = '(';
			ip = l + 1;
			l = strchr(ip,'(');
		}
	}
	strcpy(op,ip);
}

void postst(char *str)
{
	extern int ychar;
	extern REAL ygslope;
	char text[500];

	postmv(x_ps,y_ps + ychar / fabsf(ygslope));
	add_escapes(str,text);
	fprintf(psf,"(%s) T\n",text);
}

/*	fill a triangle with shade			*/

/*	"shade" ranges from 0.0 to 1.0			*/

void posttri(int x1,int y1,int x2,int y2,int x3,int y3,REAL shade)
{
	char line[60];
	extern int ymaxi;
	sprintf(line,"%.3f setgray\n",shade);
	fputs(line,psf);
	sprintf(line,"%d %d M %d %d L %d %d F\n",
			(xoffset+x1),(ymaxi-y1),
			(xoffset+x2),(ymaxi-y2),
			(xoffset+x3),(ymaxi-y3));
	fputs(line,psf);
}

#endif

