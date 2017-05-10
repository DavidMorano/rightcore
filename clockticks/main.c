/* main */



#include	<sys/types.h>
#include	<sys/times.h>
#include	<sys/time.h>
#include	<limits.h>
#include	<stdio.h>





int main(argc,argv,envv)
int	argc ;
char	*argv[] ;
char	*envv[] ;
{
	struct tms	u ;

	clock_t	ticks ;

	uint	elapsed, t, days, hours, mins ;

	int	rs ;


	ticks = u_times(&u) ;

	fprintf(stdout,"clockticks=%lu\n",ticks) ;

	elapsed = (ticks / CLK_TCK) ;
	days = elapsed / (24 * 3600) ;
	t = elapsed % (24 * 3600) ;
	hours = t / 3600 ;
	t = t % 3600 ;
	mins = t / 60 ;

	fprintf(stdout,"elapsed days=%u hours=%u mins=%u\n",
		days,hours,mins) ;

	fclose(stdout) ;

	return 0 ;
}
/* end subroutine (main) */



/* LOCAL SUBROUTINES */



