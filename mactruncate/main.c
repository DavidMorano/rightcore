/* main */


#include	<sys/types.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdio.h>



/* external subroutines */

extern int	cfdecmfull(const char *,int,ULONG *) ;


/* exported subroutines */


int main(argc,argv,envv)
int	argc ;
char	*argv[] ;
char	*envv[] ;
{
	ULONG	size ;

	int	rs ;


	if (argc < 2)
		return 1 ;

	if (cfdecmfull(argv[2],-1,&size) < 0)
		return 1 ;

	uc_truncate(argv[1],(off_t) size) ;

	return 0 ;
}
/* end subroutine (main) */


