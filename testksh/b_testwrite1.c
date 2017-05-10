/* testwrite */


#define	CF_SFPRINTF	1
#define	CF_SFWRITE	0


#include	<envstandards.h>	/* must be first to configure */

#if	defined(SFIO) && (SFIO > 0)
#define	CF_SFIO	1
#else
#define	CF_SFIO	0
#endif

#if	(defined(KSHBUILTIN) && (KSHBUILTIN > 0))
#include	<shell.h>
#endif



int b_testwrite(argc,argv,contextp)
int	argc ;
char	*argv[] ;
void	*contextp ;
{
	int	i, cl ;

	char	*cp = "53459 05-03-30 22:32:10 00 0 0 000.0 UTC(RC) *\n" ;


	for (i = 0 ; i < 4 ; i += 1) {

	cl = strlen(cp) ;

#if	CF_SFPRINTF
	sfprintf(sfstdout,"%s",cp) ;
#endif

#if	CF_SFWRITE
	sfwrite(sfstdout,cp,cl) ;
#endif

	cp += 1 ;

	} /* end for */

	return 0 ;
}
/* end subroutine (b_testout) */



