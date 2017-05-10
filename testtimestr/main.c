/* main (testdate) */


#define	CF_DEBUG	1


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<signal.h>
#include	<time.h>
#include	<string.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<field.h>
#include	<logfile.h>
#include	<baops.h>
#include	<localmisc.h>


/* local defines */

#define	LINELEN	100


/* external subroutines */

extern char	*timestr_edate(), *timestr_hdate() ;





int main()
{
	bfile	infile, *ifp = &infile ;
	bfile	outfile, *ofp = &outfile ;

	time_t	daytime = time(NULL) ;

	int	i, len ;

	char	linebuf[LINELEN + 1] ;
	char	timebuf[TIMEBUFLEN + 1] ;


	bopen(ofp,BFILE_STDOUT,"wct",0666) ;

	bprintf(ofp,"main: entered\n\n") ;

	bprintf(ofp,"tzname 1 %s %s\n",tzname[0],tzname[1]) ;

	tzset() ;

	bprintf(ofp,"tzname 2 %s %s\n",tzname[0],tzname[1]) ;

	bprintf(ofp,"edate\n%s\n",timestr_edate(daytime,timebuf)) ;

	bprintf(ofp,"tzname 3 %s %s\n",tzname[0],tzname[1]) ;

	bprintf(ofp,"tzset edate\n%s\n",timestr_edate(daytime,timebuf)) ;




	bclose(ofp) ;

	return OK ;
}
/* end subroutine (main) */



