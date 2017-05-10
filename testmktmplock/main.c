/* main */



#include	<sys/types.h>
#include	<sys/param.h>
#include	<stdio.h>

#include	"localmisc.h"



/* local variables */

static const char	*const tmpdirs[] = {
	"/tmp",
	"/var/tmp",
	"/var/spool/uucp/uucppublic",
	NULL
} ;






int main(argc,argv,envv)
int	argc ;
char	*argv[] ;
char	*envv[] ;
{
	int	rs ;

	char	fname[MAXPATHLEN + 1] ;


	fname[0] = '\0' ;
	rs = mktmplock(tmpdirs,"helloXXXX",0600,fname) ;

	if (rs >= 0)
		u_unlink(fname) ;

	printf("mktmplock() rs=%d fname=%s\n",rs,fname) ;


	fclose(stdout) ;

	return 0 ;
}
/* end subroutine (main) */



