/* main */



#include	<sys/types.h>
#include	<time.h>
#include	<stdlib.h>
#include	<stdio.h>


/* local defines */

#define	NLOOP		1



/* external subroutines */





int main()
{
	struct tm	ts ;

	time_t		daytime ;

	int	rs ;


		daytime = time(NULL) ;

			localtime_r(&daytime,&ts) ;


	fprintf(stdout,"isdst=%d\n",ts.tm_isdst) ;

	fflush(stdout) ;

	return 0 ;
}
/* end subroutine (main) */



