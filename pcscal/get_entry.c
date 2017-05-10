
#include	<time.h>
#include <stdio.h>
#include "cal.h"

get_ent(calno,mindate,maxdate)
int calno;
long mindate,maxdate;
{

	extern char *strtok();
	extern int ical;
	char *tok;
	extern long atol();
	extern char calendar[MAXCAL][256];
	extern int entries;
	int i,k;
	extern struct cal cal[MAXENTRIES] ;
	long kdate,xl;
	char data[256],field[256];
	int good;
	char s[256];
	int title;
	int spetitle;	/* for our local special calendars */
	int ent;
	char t[256];
	extern int downflag;

	extern int speflag;
	FILE *fcal;

	if(calno < 0 || calno >= ical) return(0);
	if((fcal = fopen(calendar[calno],"r")) == NULL) return(0);

	k = 0;
	good = -1;
	title = 1;
	spetitle = 1;
	while(1)
	{
		xl = ftell(fcal);
		if(fgets(s,256,fcal) == NULL) break;

		/* skip lines statring with "#" */
		if(s[0] == '#') continue;
		/* remove newline */
		s[ strlen(s) - 1 ] = '\0';
		/* check for start of entry */
		if((i = chk_ent(s,t,good)) >= 0)
		{
			if(entries+1 == MAXENTRIES)
			{
				fprintf(stderr,"too many calendar entries\n");
				exit(1);
			}
			/* check for good date */
			undate(s,s);
			kdate = atol(s);
			if(kdate < mindate || kdate > maxdate) 
			{
				good = -1;
				continue;
			}
			else /* we have at least one good entry, so print a title */
			{
				good = i;
				if ((speflag == 2) && title)/* downtime calendar*/
				{
				fprintf (stdout,"\n\n  		COMPUTATION CENTER DOWNTIME SCHEDULE\n");
					fprintf (stdout,"\n  DATE    DAY       TIME	SYSTEM      	     REASON\n");
					title = 0;
				}
				if ((speflag == 1) && spetitle)
				{
				fprintf (stdout,"\n   DATE   DAY       TIME            LOCATION          SU MMARY\n");
				spetitle = 0;
				}
			}
			/* save date in next record */
			cal[entries].no = calno;
			cal[entries].location = xl;
			cal[entries].format = i;
			cal[entries].date = 0L;
			cal[entries].stime = 0;
			cal[entries].etime = 0;
			cal[entries].title[0] = '\0';
			cal[entries].date = kdate;
			entries++;
			}

		/* skip bad entries */
		if(good < 0) continue;
		/* check for time */
		if(cal[entries-1].stime == 0)
		{
			if(strncmp(t,"TIME: ",6) == 0) 
				cal[entries-1].stime = get_time(&t[6],t);
			else
				cal[entries-1].stime = get_time(t,t);
			cal[entries-1].etime = get_time(t,t);
		}

		/* check for title flag */
		if(strcmp(t,".TL") == 0)
			cal[entries-1].title[0] = '\0';
		/* check for title */
		if(cal[entries-1].title[0] == '\0')
		{
			if(strncmp(field,"TITLE",7) == 0)
			{
				strncpy(cal[entries-1].title,&t[7],TITLELEN);
			}
			else if(t[0] != '.')
				strncpy(cal[entries-1].title,t,TITLELEN);
		}
	}
	fclose(fcal);
	return(k);
}
