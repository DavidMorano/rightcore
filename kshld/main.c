/* main */


/******************************************************************************

	This program tries to load the KSH program with the
	LD_LIBRARY_PATH set to something so that KSH doesn't f**k up
	on startup!


******************************************************************************/


#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<stdlib.h>
#include	<stdio.h>

#include	<vsystem.h>
#include	<vecstr.h>

#include	"localmisc.h"





int main(argc,argv,envv)
int		argc ;
const char	*argv[] ;
const char	*envv[] ;
{
	int	rs ;
	int	i ;

	char	*cp ;
	char	execfname[MAXPATHLEN + 1] ;




	rs = u_execve(execfname,argc,argv,ev) ;


	return 0 ;
}
/* end subroutine (main) */



