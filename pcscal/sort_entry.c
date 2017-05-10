/* sort_ent */


#
#include "cal.h"



void sort_ent() 
{
	int i,j,jj,k;
	extern int entries;
	extern int order[];
	extern char calendar[MAXCAL][256];
	extern struct cal cal[MAXENTRIES] ;
	int flag;

	for(i = 0; i < entries; i++) order[i] = i;

	flag = 1;

	while(flag) {

		flag = 0;
		for(k = 1; k < entries; k++) {

			i = order[k];
			jj = order[k-1];
			if(cal[i].format < 0) continue;
			if((cal[i].date < cal[jj].date) ||
			   (cal[i].date == cal[jj].date &&
			    cal[i].stime < cal[jj].stime)) {

				order[k] = jj;
				order[k-1] = i;
				flag = 1;
			}
		}
	}
}




