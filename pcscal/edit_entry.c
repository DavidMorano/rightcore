#include <stdio.h>
#include "cal.h"

#define BUFSIZE	256
edit_ent(item,flag)
int item,flag;
{
	char s[BUFSIZE];
	char t[BUFSIZE];
	extern int elim[MAXEDIT];
	extern int outs, didedit;
	extern FILE *lastcal;
	FILE *ftmp;
	extern char tmpe[];
	extern char tmpeall[];
	extern struct cal cal[MAXENTRIES];
	int i, p1;

	if(flag == 0)
	{

		/* copy entry to temporary file */

		if((ftmp = fopen(tmpe, "w")) == NULL)
		{
			fprintf(stderr,"error: cannot open \"%s\"\n",tmpe);
			return(1);
		}

		fseek(lastcal,cal[item].location,0);

		/* remember this location */
		p1 = cal[item].location;


		if(cal[item].format == 1) fgets(s,BUFSIZE,lastcal);
		i = 0;
		while(1)
		{
			if(fgets(s,BUFSIZE,lastcal) == NULL) break;
			s[ strlen(s) - 1 ] = '\0';
			/* check for end of entry */

			strcpy(t,s);
			if( chk_ent(t,t, 0) >= 0)
			{
				if(cal[item].format == 1 || i > 0) break;
			}
			fprintf(ftmp,"%s\n",s);
			i++;

		}


		/* 
		store this location in the array of addresses of lines
		to be omitted later
		*/
		elim[outs] = p1;

		/*
		count number of items in array to be eliminated
		*/
		outs++;

		/* close small edit file */;
		fclose(ftmp);
	}

	/* Get editor from environment, default = "ed" */
	if (getenv("ED") == NULL)
	
		strcpy (s, "ed");

	else	strcpy (s,getenv("ED"));
	printf("editing calendar item ... \n");
		strcat (s," ");
		strcat (s,tmpe);

	/* put the user into the editor set in his environment */
	system( s );
	didedit=1;

	/* add to file of edited entries */
	strcpy (s, "cat ");
	strcat (s, tmpe);
	strcat (s, " >>");
	strcat (s, tmpeall);
	system (s);
}

