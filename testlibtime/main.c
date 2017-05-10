/* testlibtime */


#define	CF_TZSET	0		/* call 'tzset(3)' */
#define	CF_LOCALTIME	0
#define	CF_MKTIME	1



#undef	LOCAL_DARWIN
#define	LOCAL_DARWIN	\
	(defined(OSNAME_Darwin) && (OSNAME_Darwin > 0))

#undef	LOCAL_SUNOS
#define	LOCAL_SUNOS	\
	(defined(OSNAME_SunOS) && (OSNAME_SunOS > 0))




#include	<sys/types.h>
#include	<time.h>
#include	<string.h>
#include	<stdio.h>



/* local defines */

#define	A	(__STDC__ != 0) 
#define	B	defined(_POSIX_C_SOURCE) 
#define	C	defined(_XOPEN_SOURCE)

#if	(A != 0) || (B != 0) || (C != 0)
extern long	altzone ;
#endif



/* local structures */







int main(argc,argv,envv)
int	argc ;
char	*argv[] ;
char	*envv[] ;
{
	struct tm	tms, *tmp = NULL ;

	time_t	t, daytime = time(NULL) ;

	int	rs ;


#if	LOCAL_SUNOS
	timezone = 0 ;
	altzone = 0 ;
#endif

#if	CF_TZSET
	tzset() ;
#endif

/* localtime_r(3c) test */

#if	CF_LOCALTIME

	fprintf(stdout,"test: localtime\n") ;

	memset(&tms,0,sizeof(struct tm)) ;

	tmp = localtime_r(&daytime,&tms) ;

	if (tmp != NULL) {

		fprintf(stdout,"sec=   %u\n",tmp->tm_sec) ;
		fprintf(stdout,"min=   %u\n",tmp->tm_min) ;
		fprintf(stdout,"hour=  %u\n",tmp->tm_hour) ;
		fprintf(stdout,"mday=  %u\n",tmp->tm_mday) ;
		fprintf(stdout,"mon=   %u\n",tmp->tm_mon) ;
		fprintf(stdout,"year=  %u\n",tmp->tm_year) ;
		fprintf(stdout,"wday=  %u\n",tmp->tm_wday) ;
		fprintf(stdout,"isdst= %d\n",tmp->tm_isdst) ;

#if	LOCAL_DARWIN
		fprintf(stdout,"zone=  %s\n",tmp->tm_zone) ;
		fprintf(stdout,"gmtoff=%d\n",tmp->tm_gmtoff) ;
#elif	LOCAL_SUNOS
		fprintf(stdout,"tz=    %ld\n",timezone) ;
		fprintf(stdout,"az=    %ld\n",altzone) ;
#endif

	} else
		fprintf(stdout,"failed\n") ;

#endif /* CF_LOCALTIME */

/* mktime(3) test */

#if	CF_MKTIME

	fprintf(stdout,"test: mktime\n") ;

	if (tmp == NULL) {

		tmp = &tms ;
		memset(&tms,0,sizeof(struct tm)) ;

		tms.tm_year = 107 ;
		tms.tm_mon = 7 ;
		tms.tm_mday = 28 ;
		tms.tm_hour = 9 ;
		tms.tm_min = 0 ;
		tms.tm_sec = 0 ;
		tms.tm_isdst = -1 ;

	}

#if	LOCAL_DARWIN
	tmp->tm_gmtoff = 0 ;
	tmp->tm_zone = NULL ;
#endif

	t = mktime(tmp) ;

	fprintf(stdout,"time=%lX\n",t) ;

#if	LOCAL_DARWIN
		fprintf(stdout,"zone=  %s\n",tmp->tm_zone) ;
		fprintf(stdout,"gmtoff=%d\n",tmp->tm_gmtoff) ;
#elif	LOCAL_SUNOS
		fprintf(stdout,"tz=    %ld\n",timezone) ;
		fprintf(stdout,"az=    %ld\n",altzone) ;
#endif

#endif /* CF_MKTIME */

/* done */

	fflush(stdout) ;

	return 0 ;
}
/* end subroutine (main) */



