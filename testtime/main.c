/* main */



#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<sys/timeb.h>
#include	<limits.h>
#include	<time.h>
#include	<stdio.h>

#include	"config.h"
#include	"defs.h"



extern char	*timestr_log(time_t,char *) ;




int main()
{
	struct timeb	b ;

	struct timeval	tv ;

	time_t	daytime ;
	time_t	a[6] ;

	int	rs, i ;

	char	timebuf[TIMEBUFLEN + 1] ;


/* 'time(2)' */

	time(&daytime) ;

	a[0] = daytime ;
	a[1] = INT_MAX ;

	for (i = 0 ; i < 2 ; i += 1) {

	fprintf(stdout,"time=\\x%08x (%s)\n",
		a[i],timestr_log(a[i],timebuf)) ;

	}

/* 'gettimeofday(3c)' */

	for (i = 0 ; i < 5 ; i += 1) {

		gettimeofday(&tv,NULL) ;

	fprintf(stdout,"time=\\x%08x (%s)\n",
		tv.tv_sec,timestr_log(tv.tv_sec,timebuf)) ;

	fprintf(stdout,"usec=%ld\n",
		tv.tv_usec) ;

	} /* end for */

/* try 'ftime(3c)' */

	rs = uc_ftime(&b) ;

	fprintf(stdout,"time=\\x%08x (%s)\n",
		b.time,timestr_log(b.time,timebuf)) ;

	fprintf(stdout,"milli=\\x%08x (%d)\n",
		b.millitm,b.millitm) ;

	fprintf(stdout,"timezone=%d\n",
		b.timezone) ;

	fprintf(stdout,"dstflag=%d\n",
		b.dstflag) ;

/* done */

	fclose(stdout) ;

	return 0 ;
}
/* end subroutine (main) */



