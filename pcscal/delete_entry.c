#
#include <stdio.h>
#include "cal.h"

/* This routine just places the addresses of the beginning of the line the
   user wishes deleted into the array of addresses of lines which will
   NOT be put back into the personal calendar.
*/
#define BUFSIZE	256
delete_ent(item,flag)
int item,flag;
{
	extern FILE *lastcal;
	extern int elim[MAXEDIT];
	extern struct cal cal[MAXENTRIES];
	extern int outs;
	int p1;

	if(flag == 0)
	{

		fseek(lastcal,cal[item].location,0);

		/*remember this location */
		p1 = cal[item].location;
		/*
		put location into array of addreses of lines to be
		eliminated at rewrite time
		*/
		elim[outs] = p1;
		outs++;
	}
}
