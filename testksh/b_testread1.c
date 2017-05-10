/* testread */


#define	CF_TESTOUT	0
#define	CF_TESTERR	0


#include	<envstandards.h>	/* must be first to configure */

#if	defined(SFIO) && (SFIO > 0)
#define	CF_SFIO	1
#else
#define	CF_SFIO	0
#endif

#if	(defined(KSHBUILTIN) && (KSHBUILTIN > 0))
#include	<shell.h>
#endif


#ifndef	LINEBUFLEN
#define	LINEBUFLEN	2048
#endif


int b_testread(argc,argv,contextp)
int	argc ;
char	*argv[] ;
void	*contextp ;
{
	Sfio_t	*efp = sfstderr ;
	Sfio_t	*ifp = sfstdin ;
	Sfio_t	*ofp = sfstdout ;

	int	len, cl ;
	int	flags ;

	char	linebuf[LINEBUFLEN + 1] ;
	char	*cp ;


#if	CF_TESTERR
	flags = SCF_LINE | SCF_WHOLE ;
	sfset(efp,flags,1) ;

	sfprintf(efp,"stderr\n") ;
#endif

/* copy input to output */

#if	CF_TESTOUT
	flags = SCF_LINE ;
	sfset(ofp,flags,1) ;
#endif

	while ((len = sfread(ifp,linebuf,LINEBUFLEN)) > 0) {

		linebuf[len] = '\0' ;
#if	CF_TESTOUT
		sfprintf(ofp,"%s",linebuf) ;
#endif

	} /* end while */

	return 0 ;
}
/* end subroutine (b_testread) */



