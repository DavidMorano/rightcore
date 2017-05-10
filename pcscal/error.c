#
#include <stdio.h>

error(i,s)
int i;
char *s;
{
    if(s == NULL) s = "";
	if(i == 1)
		fprintf(stderr,"%s: bad date or invalid calendar name.\n",s);
	if(i == 2)
		fprintf(stderr,"%s: bad date\n");
	if(i == 3)
		fprintf(stderr,"%s: HOME variable undefined\n");
	exit(i);
}

