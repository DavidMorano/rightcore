/* testlogsys */

#include	<sys/types.h>
#include	<sys/syslog.h>
#include	<stdio.h>

#include	"logsys.h"


int main()
{
	LOGSYS		logger, *lp = &logger ;
	const int	fac = LOG_USER ;
	const int	pri = LOG_INFO ;
	int		rs = 0 ;
	int		rs1 ;
	int		opts = 0 ;
	const char	*logtab = "testlogsys" ;

	if ((rs = logsys_open(lp,fac,logtab,NULL,opts)) >= 0) {
	    const char	*cp = "hello from the underworld!" ;

	    rs = logsys_printf(lp,pri,cp) ;

	    rs1 = logsys_close(lp) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (logsys) */

	return 0 ;
}
/* end subroutine (main) */


