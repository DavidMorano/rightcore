/* main */


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>

#include	<bfile.h>
#include	<exitcodes.h>
#include	<localmisc.h>


/* exported subroutines */


int main(argc,argv,envv)
int		argc ;
const char	*argv[] ;
const char	*envv[] ;
{
	bfile	outfile, *ofp = &outfile ;

	unsigned long long	a, r ;

	unsigned int		ui, uii ;

	int	rs ;
	int	i, ii ;
	int	ex = EX_OK ;


	rs = bopen(ofp,BFILE_STDOUT,"dwct",0666) ;

	if (rs < 0)
		goto ret0 ;

	a = 0xFFFFFFFFFFFFFF01ULL ;

	i = 7 ;
	ui = i ;
	ii = (~ i) ;
	uii = (~ ui) ;

	bprintf(ofp,"ii=%016llX\n",(long long) uii) ;

	bprintf(ofp,"uii=%016llX\n",((unsigned long long) ii)) ;

	r = ((unsigned long long) ii) ;
	bprintf(ofp,"r1=%016llX\n",r) ;

	r = (long long) (~ ui) ;
	bprintf(ofp,"r2=%016llX\n",r) ;

	bclose(ofp) ;

ret1:
	ex = (rs >= 0) ? EX_OK : EX_DATAERR ;

ret0:
	return ex ;
}
/* end subroutine (main) */


