/* testmktime (C89) */
#define	CF_UCMKTIME	1		/* use 'uc_mktime(3uc)' */
#include	<envstandards.h>
#include	<stdio.h>
#include	<tzfile.h>		/* for TM_YEAR_BASE */
#include	<time.h>
#include	<vsystem.h>
#include	<localmisc.h>
int main()
{
	struct tm	tms ;

	time_t		t = 0 ;

	const int	year = 2014 ;
	const int	m = 2 ; /* March */

	int	rs = 0 ;


	memset(&tms,0,sizeof(struct tm)) ;
	tms.tm_isdst = -1 ;
	tms.tm_year = (year - TM_YEAR_BASE) ;
	tms.tm_mon = m ;
	tms.tm_sec = 0 ;
	tms.tm_mday = 1 ;

#if	CF_UCMKTIME
	rs = uc_mktime(&tms,&t) ; /* always in current time zone! */
#else
	t = mktime(&tms) ;
#endif

	printf("main: uc_mktime() rs=%d\n",rs) ;
	printf("main: tms.wday=%u\n",tms.tm_wday) ;
	printf("main: tms.isdst=%u\n",tms.tm_isdst) ;

	return rs ;
}
/* end subroutine (main) */

