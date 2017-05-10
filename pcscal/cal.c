/* main (cal) */



#include <stdio.h>
#include <signal.h>
#include <time.h>
#include <ctype.h>

#include "cal.h"

/*struct for command and env variable parsing */
#include "c_parse.h"
struct c_parsed c_p;
#include "c_cmd_mat.h"
#define SAME 0
#define YES 1
#define NO 0


char calendar[MAXCAL][256];

struct cal cal[MAXENTRIES];
int order[MAXENTRIES];
int inter;
int review;
int elim[MAXEDIT];

int entries;
int ical;


FILE *lastcal, *ftmpeall;
int lastnl;
char oldcal[256];
long lastdate;
int interactive;
int delete = 0;
int nroff;
int remflag = 0;
int verbose = 0;
int downflag = 0;
int speflag = 0;
int didprint = 0;
int didedit = 0;
int outs;


char tmpe[] = "/tmp/caleXXXXXX";
char tmpeall[] = "/tmp/calefXXXXXX";

char tmpc[] = "/tmp/calcXXXXXX";

int main(argc,argv)
int argc;
char *argv[] ;
{
	int rmtemp();
	int count,i,guess_yr[2];
	int j;
	char *env,*getenv(),*strtok(),*tok;
	FILE *fp;
	struct tm *tm,*localtime();
	long tim;
	long mindate,maxdate;
	extern long atol();
	int k;
	char s[256];
	char ss[256];
	char t[256];
	char calg[256];
	char caldir[256];
	char dates[3][20];
	int idate;
	int kcal;
	int nargs;

	extern char c_parse();
	char  *sdopt;
	int jv;
	int runcmd;

	/* define command option words */
	static char *options[] = {
				   "general",		/* 1 */
				   "personal",		/* 2 */
				   "all",		/* 3 */
				   "verbose",		/* 4 */
				   "reminder",		/* 5 */
				   "nroff",		/* 6 */
				   "interactive",	/* 7 */
				   "END"
				  };

	/* getnames for temporary files , remove on hangup */
	mktemp(tmpe);
	mktemp(tmpeall);
	mktemp(tmpc);
	signal(SIGINT,rmtemp);
	signal(SIGHUP,rmtemp);

	/* get general calendar name */
	env = getenv("CALG");
	if(env == NULL || strlen(env) == 0)
		calg[0] = '\0';
	else strcpy(calg,env);

	env = getenv("CALDIR");
	if(env == NULL || strlen(env) == 0)
		caldir[0] = '\0';
	else strcpy(caldir,env);


	dates[0][0] = dates[1][0] = '\0';
	ss[0] = '\0';
	idate = 0;
	ical = 0;
	guess_yr[0] = guess_yr[1] = 0;
	kcal = 0;

	runcmd = YES;

	/* parse the command line */

	k=0;
	nargs = argc;
	while(nargs-- >1)
	{
	k++;
	jv = 0;		/* assume spec is + (see PLUS:) */
	switch(c_parse(argv[k],&c_p)) /* switch on "spec" */
	{
	      case MINUS:
	         jv = -1; 	/* on fall thru jv = 0 */
	         if(strcmp(c_p.opt,NULLSTR) == SAME)
	         {
	            /* - entered alone, could be date */
	            goto date;
	         }
	      case PLUS:
	         ++jv;
	         if (strcmp(c_p.opt,NULLSTR) == SAME)
	         {
	            /* +  entered standalone: process */
	            fprintf(stderr,"%s: invalid option: +\n",argv[0]);
	            runcmd = NO;
	            break;
	         }
	         if (c_cmd_match(c_p.opt,options) == 0)
	         {
		    /* unknown option: could be a date */
		    goto date;
	         }
	         if (imat > 1)
	         {
	            /* ambiguous option: process error */
	            fprintf(stderr,"%s: ambiguous option: %c%s (matches",argv[0],c_p.spec,c_p.opt);
	            for (j=0;j < imat;j++)
	            fprintf(stderr," %s",options[imatch[j]]);
	            fprintf(stderr,")\n");
	            runcmd = NO;
	            break;
	         }


	         /* valid option: switch on imatch[0] */
	         /* and process accordingly		      */

	         switch(imatch[0] + 1)
	         {
	            case 1: /* "general" */
			if(jv == 0)break;
		         if(calg[0] == '\0')
		         {
		         fprintf(stderr,"%s: no general calendar\n",argv[0]);
		         exit(1);
		         }
		         kcal++;
		         sprintf(calendar[ical++],"%s",calg);
		         if(verbose) fprintf(stdout,"< general calendar >\n");
	            	break;
	            case 2: /* "personal" */
			if(jv == 0)break;
	            	kcal++;
	            	env = getenv("CAL");
	            	if(env == NULL || strlen(env) == 0)
	            	{
	            		env = getenv("HOME");
	            		if(env == NULL || strlen(env) == 0)
	            		   error(3,argv[0]);
	            		else
	            		   sprintf(calendar[ical++],"%s/cal",env);
	            	}
	            	else
	            	   sprintf(calendar[ical++],"%s",env);
	            	break;
	            case 3: /* "all" */
			if(jv == 0)break;
	            	kcal++;
	            	env = getenv("CALPATH");
	            	if(verbose) fprintf(stdout,"< all calendars >\n");
	            	if(env == NULL || strlen(env) == 0)
	            	{
	            		env = getenv("CAL");
	            		if(env == NULL || strlen(env) == 0)
	            		{
	            			env = getenv("HOME");
	            			if((env == NULL || strlen(env) == 0) &&
	            			calg[0] != '\0')
	            			   sprintf(calendar[ical++],"%s",calg);
	            			else
	            			{
	            			   sprintf(calendar[ical++],"%s/cal",env);
	            			if(calg[0] != '\0')
	            				sprintf(calendar[ical++],"%s",calg);
	            			}
	            		}
	            		else
	            		{
	            		   sprintf(calendar[ical++],"%s",env);
	            		if(calg[0] != '\0')
	            			sprintf(calendar[ical++],"%s",calg);
	            		}
	            	  }
	            	  else
	            	  {
	            	 sprintf(s,env);
	            	 tok = strtok(s,":");
	            	 while(tok != NULL)
	            	 {
	            		if(*tok == '#')
	          		       sprintf(calendar[ical++],"%s",tok+1);
	            		else
	          		       sprintf(calendar[ical++],"%s",tok);
	            		tok = strtok(0,":");
	            	 }
	            	 }
	            	break;
	            case 4: /* "verbose" */
	            	verbose = jv;
	            	break;
	            case 5: /* "reminder" */
	            	remflag = jv;
	            	break;
	            case 6: /* "nroff" */
	            	nroff = jv;
	            	break;
	            case 7: /* interactive */
	            	interactive = jv;
	            	break;
	            case 8:	/* "?" */
	            	break;
	         }
	         break;
	      case EQUAL:	/* no keywords */
		 fprintf(stderr,"%s: unknown option %s\n",argv[0],c_p.opt);
		 break;
	      default:
	         /* not an option: process */
	      /* check for calendar file name */
	      if((fp = fopen(c_p.value,"r")) != NULL) 
	      {
	         if(verbose) fprintf(stdout,"< %s >\n",c_p.value);
	         sprintf(calendar[ical++],"%s",c_p.value);
	         kcal++;
	         fclose(fp);
	         break;
	      }
	
	      else
	      {
	         sprintf(s,"%s/%s",caldir,c_p.value);
	         if((fp = fopen(s,"r")) != NULL) 
	
	         {
	            if(verbose) fprintf(stdout,"< %s >\n",s);
	            sprintf(calendar[ical++],"%s",s);
		    speflag = 1; /* flag for special interest calendar */
				/* They will all have the same Heading in display */
	            kcal++;

			/* downtime is a little different from other specials */
			if (strcmp(c_p.value,"downtime") == 0) speflag = 2;
			if (strcmp(c_p.value,"general") == 0) speflag = 0;
	            fclose(fp);
	            break;
	         }
	      }
	      /* check for range of dates */
	date:
	      if(strcmp(argv[k],"to") == 0 ||
	         strcmp(argv[k],"-") == 0 ||
	         strcmp(argv[k],"thru") == 0)
	      {
	         strcpy(s,ss);
	         if((guess_yr[idate] = get_date(s,dates[idate])) < 0)
	         {
	            strcpy(s,ss);
	            rmtail(ss);
	            tok = strtok(s,"	 ");
	            if(tok == NULL || (i = num_month(tok)) == 0)
	            {
	            	fprintf(stderr,"\"%s\" is an unknown date or calendar\n",ss);
	            	exit(1);
	            }
	            tok = strtok(0,"	 ");
	            if(tok == NULL)
	            {
	            	time(&tim);
	            	tm = localtime(&tim);
	            	if(tm->tm_mon <= (i-1))
	            		j = tm->tm_year;
	            	else	j = (tm->tm_year)+1;
	            }
	            else if(sscanf(tok,"%d",&j) != 1 || j < 0)
	            {
	            	fprintf(stderr,"\"%s\" is an unknown date or calendar\n",ss);
	            	exit(1);
	            }
	            else if(j > 1900) j -= 1900;
	            if(idate == 0)
	            {
	            	sprintf(ss,"%d/1/%d",i,j);
	            	guess_yr[idate] = get_date(ss,dates[idate]);
	            }
	            else if(idate == 1)
	            {
	            	if(i == 12)
	            		sprintf(ss,"12/31/%d",j);
	            	else	sprintf(ss,"%d/1/%d -1 day",i+1,j);
	            	guess_yr[idate] = get_date(ss,dates[idate]);
	            }
	         }
	         if(idate++ > 1)
	         {
	            fprintf(stderr,"too many dates\n");
	            exit(1);
	         }
	         ss[0] = '\0';
		 break;
	      }

		/* assume date */

	      sprintf(ss + strlen(ss),"%s ",argv[k]);
	      break;
	}
	}

	if (runcmd == NO) exit(1);

	strncpy(s,ss,256);s[255]='\0';
	if((guess_yr[idate] = get_date(s,dates[idate])) < 0)
	{
		strcpy(s,ss);
		rmtail(ss);
		tok = strtok(s,"	 ");
		if(tok == NULL || (i = num_month(tok)) == 0)
		{
			fprintf(stderr,"\"%s\" is an unknown date or calendar\n",ss);
			exit(1);
		}
		tok = strtok(0,"	 ");
		if(tok == NULL)
		{
			time(&tim);
			tm = localtime(&tim);
			if(tm->tm_mon <= (i-1))
				j = tm->tm_year;
			else	j = (tm->tm_year)+1;
		}
		else if(sscanf(tok,"%d",&j) != 1 || j < 0)
		{
			fprintf(stderr,"\"%s\" is an unknown date or calendar\n",ss);
			exit(1);
		}
		else if(j > 1900) j -= 1900;
		if(idate == 0)
		{
			sprintf(ss,"%d/1/%d",i,j);
			guess_yr[idate++] = get_date(ss,dates[idate]);
		}
		if(idate == 1)
		{
			if(i == 12)
				sprintf(ss,"12/31/%d",j);
			else	sprintf(ss,"%d/1/%d -1 day",i+1,j);
			guess_yr[idate] = get_date(ss,dates[idate]);
		}
	}
	if(kcal + 1 + interactive == argc)
		get_date("extended-tomorrow",dates[1]);
	else if(dates[1][0] == '\0') sprintf(dates[1],dates[0]);

	if(ical == 0)
	{
		env = getenv("CAL");
		if(env == NULL || strlen(env) == 0)
		{
			env = getenv("HOME");
			if(env == NULL || strlen(env) == 0)
			   error(3,argv[0]);
			else
	   		   sprintf(calendar[ical++],"%s/cal",env);
		}
		else
	   	   sprintf(calendar[ical++],"%s",env);
	}

	/* get calendar entries */

	entries = 0;
	undate(dates[0],dates[0]);
	undate(dates[1],dates[1]);
	mindate = atol(dates[0]);
	maxdate = atol(dates[1]);
	for( i = 0; i < ical; i++) get_ent(i,mindate,maxdate);
	/* sort entries by date, time */
	sort_ent();
	/* print entries */
	if(interactive && entries > 0)
		fprintf(stderr,"%d entries\n\n",entries);
	for( i = 0; i < entries; i++) 
	{
		if (cal[i].format >=0)
		{
			print_ent (order[i], i);
		}
	}
	if(entries == 0)
	{
		if (speflag != 2) printf ("no entries for ");
		else printf("\n no entries on Computation Center Downtime Schedule for ");
		fprintf(stdout,"%2ld/%ld%ld/%2ld",
			(mindate/100L)%100L,
			(mindate/10L)%10L,
			(mindate)%10L,
			(mindate/10000L)
			);
		if(maxdate != mindate)
			fprintf(stdout," to %2ld/%ld%ld/%2ld",
				(maxdate/100L)%100L,
				(maxdate/10L)%10L,
				(maxdate)%10L,
				(maxdate/10000L)
				);
		putc('\n',stdout);
	}

	rmtemp();
}
/* end subroutine (main) */


rmtemp(i)
int i;
{


	unlink(tmpe);
	unlink(tmpeall);
	unlink(tmpc);

}




