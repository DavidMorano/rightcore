/* testread */


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
	Sfio_t	*ifp = sfstdin ;

	int	len ;

	char	linebuf[LINEBUFLEN + 1] ;


	len = sfread(ifp,linebuf,LINEBUFLEN) ;

	return (len == 0) ? 0 : 1 ;
}
/* end subroutine (b_testread) */



