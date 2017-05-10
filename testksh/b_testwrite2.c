/* testwrite */


#define	F_SFPRINTF	0
#define	F_SFWRITE	1
#define	F_SFREAD	0


#include	<envstandards.h>	/* must be first to configure */

#if	defined(SFIO) && (SFIO > 0)
#define	CF_SFIO	1
#else
#define	CF_SFIO	0
#endif

#if	(defined(KSHBUILTIN) && (KSHBUILTIN > 0))
#include	<shell.h>
#endif


#ifndef	MAXLINELEN
#define	MAXLINELEN	2048
#endif


/* forward references */

static int output(Sfio_t *,const char *) ;


/* exported subroutines */


int b_testwrite(argc,argv,contextp)
int	argc ;
char	*argv[] ;
void	*contextp ;
{
	Sfio_t	*fp ;
	Sfio_t	*ifp ;

	int	rs = 0 ;
	int	what = 0x01 ;

	char	linebuf[MAXLINELEN + 1] ;
	char	*cp = "53767 06-02-01 01:11:11 00 0 0 000.0 UTC(RC) *\n" ;


	if (argc > 1)
		what = atoi(argv[1]) ;

	if (what & 0x01) {

		fp = sfstdout ;
#if	F_SFREAD
		ifp = sfstdin ;
		while ((rs = sfread(ifp,linebuf,MAXLINELEN)) > 0) {

			linebuf[rs] = '\0' ;
			rs = output(fp,linebuf) ;

			if (rs < 0)
				break ;

		} /* end while */
#else
		rs = output(fp,cp) ;
#endif

	} /* end if */

	if ((rs >= 0) && (what & 0x02)) {

		fp = sfstderr ;
		rs = output(fp,"this is STDERR\n") ;

	}

	return (rs >= 0) ? 0 : 1 ;
}
/* end subroutine (b_testwrite) */


static int output(fp,sp)
Sfio_t		*fp ;
const char	*sp ;
{
	int	rs ;
	int	sl ;


#if	F_SFPRINTF
	rs = sfprintf(fp,"%s",sp) ;
#endif

#if	F_SFWRITE
	sl = strlen(sp) ;

	rs = sfwrite(fp,sp,sl) ;
#endif

	return rs ;
}



