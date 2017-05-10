/* testwrite */


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
	Sfio_t	*fp ;

	int	rs = 0, sl ;
	int	what = 0x01 ;

	char	*sp = "53767 06-02-01 01:11:11 00 0 0 000.0 UTC(RC) *\n" ;


	if (argc > 1)
		what = atoi(argv[1]) ;

	if (what & 0x01) {

		fp = sfstdout ;
		sl = strlen(sp) ;

		rs = sfwrite(fp,sp,sl) ;

	} /* end if */

	if ((rs >= 0) && (what & 0x02)) {

		fp = sfstderr ;
		sp = "this is STDERR\n" ;
		sl = strlen(sp) ;

		rs = sfwrite(fp,sp,sl) ;

	}

	return (rs >= 0) ? 0 : 1 ;
}
/* end subroutine (b_testwrite) */


