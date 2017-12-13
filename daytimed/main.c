/* main (daytimed) */


#include	<envstandards.h>

#include	<sys/types.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>
#include	<stdio.h>

#include	"nistinfo.h"
#include	"config.h"


/* local defines */

#ifndef	TIMEBUFLEN
#define	TIMEBUFLEN	80
#endif


/* external subroutines */

extern int	sncpy1(char *,int,const char *) ;

extern char	*timestr_nist(time_t,struct nistinfo *,char *) ;


/* exported subroutines */


int main(argc,argv,envv)
int	argc ;
char	*argv[] ;
char	*envv[] ;
{
	struct nistinfo	ni ;
	const time_t	dt = time(NULL) ;
	cchar		*orgp ;
	cchar		*cp ;
	char		ntbuf[NISTINFO_BUFLEN+ 1] ;

	if ((orgp = getenv(VARORGANIZATION)) == NULL)
		orgp = ORGANIZATION ;

	memset(&ni,0,sizeof(struct nistinfo)) ;

	sncpy1(ni.org,NISTINFO_ORGSIZE,orgp) ;

	if ((cp = timestr_nist(dt,&ni,ntbuf)) != NULL) {
	    fprintf(stdout,"%s\n",ntbuf) ;
	}

	return 0 ;
}
/* end subroutine (main) */


