#include	<envstandards.h>
#include	<time.h>

get_time(data,rest)
char data[];
char rest[];
{
	int i,j,k;
	extern char *strtok();
	int flag;
	char *tok1,*tok2;
	int itime,xhr,xmin;
	char xc,xm;
	char tt[256];
	char st[256];


	strcpy(st,data);
	tok1 = strtok(data,"	- ");
	tok2 = strtok(0,"	- ");
	if(tok1 == NULL) tok1 = "";
	if(tok2 == NULL) tok2 = "";

	flag = 0;
	if(strcmp(tok2,"AM") == 0 ||
	   strcmp(tok2,"PM") == 0 ||
	   strcmp(tok2,"am") == 0 ||
	   strcmp(tok2,"pm") == 0) 
	{
		flag = 1;
		sprintf(tt,"%s%s",tok1,tok2);
	}
	else strcpy(tt,tok1);
	/* set rest to initial string */
	strcpy(rest,st);
	/* check for time */
	xhr=xmin=0;
	xm='\0';
	xc='\0';
	sscanf(tt,"%d%c%d%c",&xhr,&xc,&xmin,&xm);
	if(xhr == 0 && xmin == 0) return(0);	/* no time */
	if(xc != ':') return(0);	/* bad time format */
	if(xm != 'a' && xm != 'A' &&
	   xm != '\0' &&
	   xm != 'p' && xm != 'P')	return(0);	/* bad AM or PM */
	if(xmin < 0 || xmin > 60) return(0);	/* bad minutes */
	if(xhr < 0 || xhr > 24) return(0);	/* bad hour */

	/* good time */
	itime = 0;
	if(xm == 'a' || xm == 'A') itime = 60 * xhr + xmin;
	else if(xm == 'p' || xm == 'P') itime = 60 * ((xhr%12) + 12) + xmin;
	else if(xhr < 8) itime = 60 * (xhr + 12) + xmin;
	else itime = 60 * xhr + xmin;


	/* put rest of string in rest */
	tok1 = strtok(st,"	 -");
	if(tok1 == NULL) tok1 = "";
	if(flag == 0)	/* no am or pm */
	{
		tok1 = strtok(0,"");
		if(tok1 == NULL) tok1 = "";
		while(*tok1 == ' ' || *tok1 == '	' || *tok1 == '-') tok1++;
		strcpy(rest,tok1);
	}
	else		/* separate am of pm */
	{
		tok1 = strtok(0,"	 .");
		tok1 = strtok(0,"");
		if(tok1 == NULL) tok1 = "";
		while(*tok1 == ' ' || *tok1 == '	' || *tok1 == '-') tok1++;
		strcpy(rest,tok1);
	}
	return(itime);
}
