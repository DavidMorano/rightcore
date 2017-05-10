/*
   give the day of the week that the date is on
*/

#include <time.h>
#include <stdio.h>

#define DAY (3600*24L)
int dmsize[]={31,28,31,30,31,30,31,31,30,31,30,31};

/*  static struct tm *localtime();   */
static long t;


get_wday(s,ss)
char s[],ss[];
{
	int	da, mo, yr;

	time(&t);
	yr = 0;
	if ((sscanf(s, "%d/%d/%d",
		&mo, &da, &yr) < 2)
		|| (mo < 1) 
		|| (mo >12)
		|| (da < 1)
		|| (da >31))
	{
		exit(-1);
	}
	if (yr >1900) yr -= 1900;
	settime(da, mo, yr);
	
	sprintf(ss,"%.3s", asctime(localtime(&t)));
	return(0);
}

settime(day, mon, year)
int	day, mon, year;
{
	register int i;
	extern long timezone;

	if (year == 0)
		year = localtime(&t)->tm_year;
	tzset();
	t = timezone;
	t = t +(12*60*60L);	/* set for noon, avoid margin condition*/
	year += 1900;
	for(i=1970; i<year; i++)
		gdadd(dysize(i));
	/* Leap year */
	if (dysize(year)==366 && mon >= 3)
		gdadd(1);
	while(--mon)
		gdadd(dmsize[mon-1]);
	gdadd(day-1);
	return(0);
}

gdadd(n)
unsigned n;
{
	t = t + n*DAY;
}

dysize(y)
int y;
{
	if (( y%4 ) == 0 )
		return(366);
	return(365);
}
