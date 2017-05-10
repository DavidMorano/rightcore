#include	<envstandards.h>

#include	<time.h>
#include <stdio.h>
#include "cal.h"

print_ent(item,count)
int item,count;
{
	char s[256];
	char tt[256];
	char t[256];
	char endstr[256];
	int fill;
	extern FILE *lastcal;
	extern char tmpe[],tmpc[];
	FILE *ftmp;
	extern int review;
	extern int entries;
	extern int didprint;
	extern int lastnl;
	extern int nroff;
	extern int inter;
	extern char oldcal[];
	extern long lastdate;
	extern int remflag;
	extern int speflag;
	int flag,zflag,nzflag,aflag,nflag,mflag,pflag,mzflag;
	extern char calendar[MAXCAL][256];
	extern struct cal cal[MAXENTRIES] ;
	extern int interactive;
	int m,k,i,j,edit;
	int ti;
	char *tok1,*tok2,*tok3;

	i = cal[item].no;
	if(strcmp(calendar[ i ], oldcal) != 0)
	{
		if(oldcal[0] != '\0') fclose(lastcal);
	   	if((lastcal = fopen( calendar[ i ],"r")) == NULL) return;
		strcpy(oldcal,calendar[ i ]);

	}

	edit = 0;
	ftmp = lastcal;
start:
	fill = 0;
	if(edit == 0) fseek(lastcal,cal[item].location,0);
	else if((ftmp = fopen(tmpe,"r")) == NULL)
	{
		fprintf(stderr,"error: cannot open %s\n",tmpe);
		goto prompt;
	}




	/* if reminder option, just print time & title */
	if(remflag)
	{
		if(cal[item].stime == 0) return;
		i = cal[item].stime/60;	/* hours */
		if(i > 12) i -= 12;
		fprintf(stdout,"%d:%d%d", i,
			(cal[item].stime%60)/10,
			(cal[item].stime%60)%10);
		if(strncmp("TITLE: ",cal[item].title,7) == 0)
			fprintf(stdout," %s\n",&cal[item].title[7]);
		else 	fprintf(stdout," %s\n",cal[item].title);
		return;
	}

	if(count && !lastnl && !nroff) putc('\n',stdout);
	lastnl = 0;
	/* print macros for pipe to nroff */
	if(nroff) fprintf(stdout,".DS\n");
	/* print date */
	if(cal[item].date != lastdate || interactive)	/* starting new date */
	{
		m  = aflag = nflag = nzflag = pflag = zflag = mflag = mzflag = 0;
		sprintf(t,"%ld/%ld%ld/%2ld",
			(cal[item].date/100L)%100L,
			(cal[item].date/10L)%10L,
			(cal[item].date)%10L,
			(cal[item].date/10000L)
			);
		fprintf(stdout,"%8s",t);
		get_wday(t,t);
		fprintf(stdout," [%s]",t);
	}
	else
	{
		 fprintf(stdout,"%14s",""); /* continuing with previous date */
		 nzflag = zflag = mzflag = 0;
	}

	lastdate = cal[item].date;
	/* print start time */
	m = cal[item].stime%60;	/* compute minutes */

	i = k = cal[item].stime/60;	/* hours */

	if (i > 12) i -= 12;	/*convert */
	if(cal[item].stime == 0) fprintf(stdout,"%5s","");
	else fprintf(stdout," %2d:%d%d", i,
		(cal[item].stime%60)/10,
		(cal[item].stime%60)%10);

	/* print end time */
	i = cal[item].etime/60;	/* hours */
	j = i;	/* save unconverted end time */
	if (i > 12) i -= 12;	/*modify time for regular clock */

	if(cal[item].etime == 0) /* no end time,put down AM, etc. */
	{
		if (k < 12)
		{
			if ( ! aflag)
			{
				fprintf(stdout,"%16s", "AM             -");
				aflag = 1;
			}
			else fprintf(stdout,"%16s","               -");
		}
		else if ( k == 12 && m == 0)
		{
			if (! nflag)
			{
				fprintf(stdout,"%16s", "NOON           -");
				nflag = 1;
			}
			else fprintf(stdout,"%16s","               -");
		}
		else if ( k == 24 && m == 0)
		{
			if (! mflag)
			{
				fprintf(stdout,"%16s", "MID            -");
				mflag = 1;
			}
			else fprintf(stdout,"%16s","               -");
		}
		else	/*default > 12 */
		{
			if ( !pflag)
			{
				fprintf (stdout,"%16s", "PM             -" );
				pflag = 1;
			}
			else fprintf(stdout,"%16s","               -");
		}
	}
	else /* we have an endtime */
	{
		if ( k < 12 && j > 12)
		{
			fprintf (stdout,"%2s", "AM");
			zflag = 1;
			aflag = 1;
		}
		 if (k == 12 && j > 12)
		{
			fprintf (stdout, "%4s","NOON");
			nflag = 1;
			nzflag = 1;
		}
		if (k == 24)
		{
			fprintf (stdout, "%3s", "MID");
			mflag = 1;
			mzflag = 1;
		}
	 if( i < 10 ) fprintf(stdout,"-%2d:%d%d",  i, /* print end time */
		(cal[item].etime%60)/10,
		(cal[item].etime%60)%10);
	else fprintf(stdout,"-%2d:%d%d", i, /* print end time */
		(cal[item].etime%60)/10,
		(cal[item].etime%60)%10);
	if (j < 12) /* endtime is AM */
	{
			if ( ! zflag && ! mzflag )
			{
			fprintf(stdout,"%9s","AM       -");
			goto data;
			}
			if (zflag) fprintf(stdout,"%7s","AM    -");
			if (mzflag) fprintf (stdout,"%6s","AM   -");
			aflag = 1;
	}
	else if ( j == 12 && m == 0) /* endtime is NOON */
	{
		if (!zflag && ! mzflag)
			 {
			 fprintf (stdout, "%9s", "NOON     -");
			 goto data;
			 }
			 if ( zflag ) fprintf (stdout,"%7s", "NOON  -"  );
			if ( mzflag ) fprintf (stdout,"%7s", "NOON  -");
			nflag = 1;
	}
	else	/*default > 12	*/
	{
			if (! zflag && ! mzflag && ! nzflag) 
			{
				fprintf (stdout, "%9s", "PM       -");
				goto data;
			}
			if ( mzflag ) fprintf (stdout,"%6s", "PM    -");
			if ( zflag ) fprintf (stdout,"%7s", "PM     -");
			if ( nzflag ) fprintf (stdout,"%5s", "PM   -");
			pflag  = 1;
	}
	}
	/* read data */
data:
	count = 0;
	endstr[0] = '\0';
	flag = -1;
	while(1)
	{
		if(fgets(s,256,ftmp) == NULL) break;
		s[ strlen(s) - 1 ] = '\0';
		if(s[0] == '#') continue;
		/* check for end of entry */

		if((i = chk_ent(s,t,flag)) >= 0)
		{
			if(count > 0) break;
			flag = i;
		}

		if(strncmp(t,"DATE:",5) == 0 ||
		   strncmp(t,"TIME:",5) == 0) continue;

		get_time(t,t);
		get_time(t,t);
		if(strcmp(".fi",t) == 0)
		{
			fill = 1;
			continue;
		}
		if(strcmp(".nf",t) == 0)
		{
			fill = 0;
			if(endstr[0] != '\0') 
			{
				if(count++ ) fprintf(stdout,"\n%38s","");
				fprintf(stdout,"%s",endstr);
				endstr[0] = '\0';
			}
			continue;
		}
		if(strcmp(".br",t) == 0) 
		{
			if(endstr[0] != '\0') 
			{
				if(count++ ) fprintf(stdout,"\n%38s","");
				fprintf(stdout,"%s",endstr);
				endstr[0] = '\0';
			}
			continue;
		}
		if(strcmp(".TL",t) == 0) continue;
		if(t[0] == '\0') 
		{
			if(endstr[0] != '\0') 
			{
				if(count++ ) fprintf(stdout,"\n%38s","");
				fprintf(stdout,"%s",endstr);
			}
			if(!lastnl) putc('\n',stdout);
			endstr[0] = '\0';
			count++;
			lastnl = 1;
			continue;
		}

		if (endstr[0] == '\0') strcpy(endstr,t);
		else sprintf(endstr + strlen( endstr ), " %s", t);

		while(1)
		{
			if(nroff)
			{
				if( strlen(endstr) < 36  && fill) break;
				split(endstr,36,s,endstr);
			}
			else
			{
				if( strlen(endstr) < 31  && fill) break;
				split(endstr,31,s,endstr);
			}
			if(s[0] == '\0') break;
			if(count++ ) fprintf(stdout,"\n%35s","");
			if (count > 1 && speflag) fprintf(stdout,"%14s","");
			fprintf(stdout,"%s",s);
			lastnl = 0;
		}
	}
	if(endstr[0] != '\0') 
	{
		if(count++ ) fprintf(stdout,"\n%35s","");
		if (count++ && speflag) fprintf (stdout,"%14s","");
		fprintf(stdout,"%s",endstr);
	}
	putc('\n',stdout);

	/* print macros for pipe to nroff */
	if(nroff) fprintf(stdout,".DE\n");
	if(!interactive || nroff) return;
	if(edit == 1) fclose(ftmp);

	/* this is incremented after a line is printed, whether in review, or
	after edit or delete */
	++didprint;

prompt:

	fprintf(stderr, "\nreview, edit, delete, next, quit ? [next] ");

	if(gets(s) == NULL) exit(1);

	rmhead(s,s);
	switch(s[0])
	{
	case 'r':	++review;
			goto start;
			break;

	case 'd':	delete_ent(item,flag);
			inter = 1;
			goto prompt;
			break;
	case 'e':	edit_ent(item,edit);
			inter = 1;
			edit = 1;
			goto prompt;
			break;
	case 'n':	if ((didprint - review) == entries && inter)
			{
				fprintf (stderr, "Last entry, calendar will be updated\n"); 
				rewrite();
			}
			return;
			break;

	case 'q':	if (inter)  /* only if we've had an edit or delete */
			{
				fprintf (stderr, " calendar will be updated\n");
				rewrite();
				rmtemp(i);
			}
			exit();
			break;

	case '?':	fprintf(stderr,"\n     %-10s%s\n","review","print again for review");
			fprintf(stderr,"     %-10s%s\n","edit","edit calendar entry.");
			fprintf(stderr,"     %-10s%s\n","next","print next calendar entry.");
			fprintf(stderr,"     %-10s%s\n","delete","delete entry.");
			fprintf(stderr,"     %-10s%s\n","quit","terminate program");
			goto prompt;
			break;

	}
	if ((didprint - review) == entries && inter)
		{
		fprintf (stderr, "Last entry, calendar will be updated\n");
		rewrite();
		}
}
