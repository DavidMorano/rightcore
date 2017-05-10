/* main (daytimed) */



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

	time_t	t = time(NULL) ;

	char	timebuf[TIMEBUFLEN + 1] ;
	char	*orgp ;
	char	*cp ;



	if ((orgp = getenv(VARORGANIZATION)) == NULL)
		orgp = ORGANIZATION ;

	memset(&ni,0,sizeof(struct nistinfo)) ;

	sncpy1(ni.org,NISTINFO_ORGSIZE,orgp) ;

	cp = timestr_nist(t,&ni,timebuf) ;

	if (cp != NULL)
	fprintf(stdout,"%s\n",timebuf) ;

	fclose(stdout) ;

	return 0 ;
}
/* end subroutine (main) */



