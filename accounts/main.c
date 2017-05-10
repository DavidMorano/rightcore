/* main */

/* whole program for ACCOUNTS */



#include	<sys/types.h>
#include	<pwd.h>
#include	<stdio.h>

#include	"localmisc.h"



/* external subroutines */

extern struct passwd	*getpwent() ;





int main(argc,argv,envv)
int	argc ;
char	*argv[] ;
char	*envv[] ;
{
	struct passwd	*pp ;

	int	ex = 0 ;
	int	rs ;


	setpwent() ;

	while ((pp = getpwent()) != NULL) {

		fprintf(stdout,"%-8s %5d %5d (%s) %s %s\n",
			pp->pw_name,pp->pw_uid,pp->pw_gid,
			pp->pw_gecos,pp->pw_dir,pp->pw_shell) ;

	} /* end while */

	endpwent() ;

	fflush(stdout) ;

	return 0 ;
}
/* end subroutine (main) */




