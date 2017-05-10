#
#include <stdio.h>
#include <time.h>
#include <ctype.h>

static int days[] = {31,28,31,30,31,30,31,31,30,31,30,31};

static long tim=0L;
static int day,mon,year,wday;
int tday,tmon,tyear,twday;

get_date(ss,sdate)
char ss[];
char sdate[];
{
        int count,i;
        extern long time();
        char *tok;
        extern char *strtok();
	int toklen;
        int k;
        int guess;
        char s[256];
        char t[256];

	initday(0);

        guess = 0;

        tok = strtok(ss," ");
	/*  if(tok == NULL) return(0); */
	if(tok == NULL) tok = ""; 
	toklen = strlen(tok);
        while( tok != NULL) 
        {
                if(strncmp(tok,"today",toklen) == 0) initday(1);
                else if(strncmp(tok,"tomorrow",toklen) == 0) 
		{
			initday(1);
			addday(1);
		}
		else if(strncmp(tok,"extended-tomorrow",toklen) == 0)
		{
			initday(1);
			switch (twday)
			{
			case5: addday(3); break;	/*Friday	*/
			case6: addday(2); break;	/*Saturday	*/
			default: addday(1); break;	/* rest of week	*/
			}
		}
                else if(strncmp(tok,"yesterday",toklen) == 0) 
		{
			initday(1);
			subday(1);
		}
                else if(strncmp(tok,"sunday",toklen) == 0) {
                  count = 0 - twday;
                  if(count <= 0) count += 7;
                  addday(count);
                  }
                else if(strncmp(tok,"monday",toklen) == 0) {
                  count = 1 - twday;
                  if(count <= 0) count += 7;
                  addday(count);
                  }
                else if(strncmp(tok,"tuesday",toklen) == 0) {
                  count = 2 - twday;
                  if(count <= 0) count += 7;
                  addday(count);
                  }
                else if(strncmp(tok,"wednesday",toklen) == 0) {
                  count = 3 - twday;
                  if(count <= 0) count += 7;
                  addday(count);
                  }
                else if(strncmp(tok,"thursday",toklen) == 0) {
                  count = 4 - twday;
                  if(count <= 0) count += 7;
                  addday(count);
                  }
                else if(strncmp(tok,"friday",toklen) == 0) {
                  count = 5 - twday;
                  if(count <= 0) count += 7;
                  addday(count);
                  }
                else if(strncmp(tok,"saturday",toklen) == 0) {
                  count = 6 - twday;
                  if(count <= 0) count += 7;
                  addday(count);
                  }
                else if(*tok == '+')
                {
                        count = 0;
                        sscanf(tok,"%d",&count);
                        tok++;
                        while(isdigit(*tok)) tok++;
                        if(*tok == '\0')
                        {
                                tok = strtok(0," ");
				if(tok == NULL) tok = "";
				toklen = strlen(tok);
                                if(count == 0)
                                {
				    if(toklen > 0) sscanf(tok,"%d",&count);
                                    while(isdigit(*tok)) tok++;
                                    if(*tok == '\0') tok = strtok(0," ");
				    if( tok == NULL) tok = "";
				    toklen = strlen(tok);
                                }
                        }
                        if(count == 0) count = 1;
                        else if(*tok == '\0') ;
                        else if(strncmp(tok,"days",toklen) == 0) ;
                        else if(strncmp(tok,"weeks",toklen) == 0) count *= 7;
                        else if(strncmp(tok,"years",toklen) == 0) 
                        {
				year++;
                                count = 0;
                        }
                        else if(strncmp(tok,"months",toklen) == 0) 
                        {
                                mon += count;
                                while(mon > 11)
                                {
                                        mon -= 12;
                                        year++;
                                }
                                count = 0;
                        }
                        else return(-1);
                        addday(count);
                        break;
                }
                else if(*tok == '-')
                {
                        count = 0;
                        tok++;
                        sscanf(tok,"%d",&count);
                        while(isdigit(*tok)) tok++;
                        if(*tok == '\0')
                        {
                                tok = strtok(0," ");
				if(tok == NULL ) tok = "";
				toklen = strlen(tok);
				if(count == 0)
				{
                                        sscanf(tok,"%d",&count);
                                        while(isdigit(*tok)) tok++;
                                        if(*tok == '\0') tok = strtok(0," ");
					if( tok == NULL) tok = "";
					toklen = strlen(tok);
                                }
                        }
                        if(count == 0) count = 1;
                        else if(*tok == '\0') ;
                        else if(strncmp(tok,"days",toklen) == 0) ;
                        else if(strncmp(tok,"weeks",toklen) == 0) count *= 7;
                        else if(strncmp(tok,"years",toklen) == 0)
                        {
                                year--;
                                count = 0;
                        }
                        else if(strncmp(tok,"months",toklen) == 0) 
                        {
                                while(mon < 0)
                                {
                                        mon += 12;
                                        year--;
                                }
                                count = 0;
                        }
                        else return(-1);
                        subday(count);
                        break;
                }
        
                else            /* must be a date */
                {
        
                        s[0] = t[0] = '\0';
                        while(tok != NULL)
                        {
                                if(*tok == '-' || *tok == '+') break;
                                sprintf(s+strlen(s),"%s ",tok);
                                tok = strtok(0," ");
                        }
                        mon = day = year = 0;
                        undate(s,t);
                        if(t[0] == '\0')
                        {
                                guess = 1;
                                sprintf(s+strlen(s),"%d",tyear);
                                undate(s,t);
                                if(t[0] == '\0') return(-1);
                                sscanf(t,"%2d%2d%2d",&year,&mon,&day);
                                if((mon-1) < tmon) year++;
                                else if((mon-1) == tmon && day < tday) year++;
                        }
                        else sscanf(t,"%2d%2d%2d",&year,&mon,&day);
                        mon--;
                        if(year%4 == 0 && mon == 1 && day == 29) ;
                        else if(day > days[mon]) return(-1);
                        continue;
                }

                
                tok = strtok(0," ");
		if( tok != NULL)
		    toklen = strlen(tok);
        }
	/* calculate time since 1/1/70 */
	tday = 0;
	for(i = 70; i < year; i++)
	{
		if(i%4 == 0) tday += 366;	/* leap year */
		else tday += 365;		/* normal year*/
	}
	for(i = 0; i < mon; i++)
	{
		tday += days[ i ];		/* months*/
		if(year == 0 && i == 1) tday++;	/*leap year*/
	}
	tday += day;
	tim = tday * 24L * 60L * 60L;
        mon++;
        sprintf(sdate,"%d%d/%d%d/%d",mon/10,mon%10,day/10,day%10,year);
        return(guess);

}


addday(count) 
int count;
{
	while(count-- > 0)
	{
		day += 1;
		if(year%4 == 0	/* leap year */
		   && mon == 1)	/* february */
		{
			if(day > 29)
			{
				mon = 2;
				day = 1;
			}
		}
		else 
		{
			if(day > days[mon])
			{
				mon += 1;
				day = 1;
				if(mon == 12)
				{
					mon = 0;
					year += 1;
				}
			}
		}

	}
}

subday(count) 
int count;
{
	while(count--> 0)
	{
		day -= 1;
		if(year%4 == 0	/* leap year */
		   && mon == 2)	/* march */
		{
			if(day < 1)
			{
				mon = 1;
				day = 29;
			}
		}
		else 
		{
			if(day < 1)
			{
				mon -= 1;
				if(mon < 0)
				{
					mon = 11;
					year -= 1;
					day = 31;
				}
				else day = days[mon];
			}
		}
	}
}

initday(i)
int i;
{
        struct tm *tm,*localtime();
	if(i == 1 || tim == 0L)
	{
		time( &tim );

	}

	tm = localtime(&tim);
	tday = tm->tm_mday;
	tmon = tm->tm_mon;
	tyear = tm->tm_year;
	twday = tm->tm_wday;
	day = tday;
	mon = tmon;
	year = tyear;
	wday = twday;

}
