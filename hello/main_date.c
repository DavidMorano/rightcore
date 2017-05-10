/* main (date) */


#include	<sys/types.h>
#include	<time.h>
#include	<stdio.h>


#ifndef	TIMEBUFLEN
#define	TIMEBUFLEN	80
#endif



extern int	timestr_log(time_t,char *) ;


int main()
{
	time_t	daytime ;

	char	timebuf[TIMEBUFLEN + 1] ;


	daytime = time(NULL) ;

	timestr_log(daytime,timebuf) ;

	fprintf(stdout,"%s\n",timebuf) ;

	return 0 ;
}
/* end subroutine (main) */



