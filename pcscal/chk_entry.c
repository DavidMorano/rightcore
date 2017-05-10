#
#include "cal.h"
#define NULL (char *)0
chk_ent(s,data,flag)
char s[];
char data[];
int flag;
{
	char *tok1,*tok2,*strtok();

	strcpy(data,s);
	tok1 = strtok(s,"	 -:");
	tok2 = strtok(0,"");

	if( tok1 == NULL ) tok1 = "";
	if( tok2 == NULL ) tok2 = "";

	if(strcmp(tok1,"DATE") == 0)
	{
		rmhead(tok2,s);
		data[0] = '\0';
		return(1);
	}

	if(strncmp(tok2,"Sun",3) == 0 ||
	   strncmp(tok2,"Mon",3) == 0 ||
	   strncmp(tok2,"Tue",3) == 0 ||
	   strncmp(tok2,"Wed",3) == 0 ||
	   strncmp(tok2,"Thu",3) == 0 ||
	   strncmp(tok2,"Fri",3) == 0 ||
	   strncmp(tok2,"Sat",3) == 0)

	{
		tok2 = strtok(tok2,"	 -");
		tok2 = strtok(0,"");
		rmhead(tok2,data);
		return(0);
	}

	if(flag == 1) return(-1);
	rmhead(tok2,data);
	return(-1);
}
