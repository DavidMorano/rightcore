/* main */

/* program to convert a decimal number to a decimal number w/ 3 digits */


#define	CF_DEBUGS	0
#define	CF_DEBUG	0


/* revision history:

	= 85/02/01, David A­D­ Morano

	This subroutine was originally written.


*/




#include	<sys/types.h>

#include	<bfile.h>
#include	<field.h>
#include	<exitcodes.h>

#include	"localmisc.h"
#include	"config.h"
#include	"defs.h"



/* local defines */

#define	LINELEN		100



/* external subroutines */

extern int	cfdeci(char *,int,int *) ;

extern char	*strbasename(char *) ;





int main(argc,argv,envv)
int	argc ;
char	*argv[] ;
char	*envv[] ;
{
	struct global	g, *gp = &g ;

	FIELD	fsb ;

	bfile	errfile, *efp = &errfile ;
	bfile	outfile, *ofp = &outfile ;
	bfile	infile, *ifp = &infile ;

	int	ex = EX_INFO ;
	int	rs, len ;
	int	value ;

	char	buf[LINELEN + 1] ;


	g.progname = strbasename(argv[0]) ;

	if (bopen(efp,BFILE_STDERR,"dwca",0666) >= 0)
		bcontrol(efp,BC_LINEBUF,0) ;


	if (bopen(ofp,BFILE_STDOUT,"wc",0666) < 0)
		goto ret1 ;

	if (bopen(ifp,BFILE_STDIN,"r",0666) < 0)
		goto ret2 ;


	while ((rs = breadline(ifp,buf,LINELEN)) > 0) {

	    len = rs ;
		if (buf[len - 1] == '\n')
			len -= 1 ;

	    fsb.rlen = len ;
	    fsb.lp = buf ;
	    while (field_get(&fsb,NULL) >= 0) {

#if	CF_DEBUGS
	debugprintf("main: 1 gp=>%t<\n",fsb.fp,fsb.flen) ;
#endif

	        if (cfdeci(fsb.fp,fsb.flen,&value) >= 0) {

#if	CF_DEBUGS
	debugprintf("main: 2\n") ;
#endif

	            bprintf(ofp,"%03d\n", value) ;

	        } else {

#if	CF_DEBUGS
	debugprintf("main: 2a\n") ;
#endif

	            bprintf(efp,
	                "%s: \"%t\" is not a valid decimal number\n",
	                g.progname,fsb.fp,fsb.flen) ;

		}

#if	CF_DEBUGS
	debugprintf("main: 3\n") ;
#endif

	        field_get(&fsb,NULL) ;

	    } /* end while (getting fields) */

#if	CF_DEBUGS
	debugprintf("main: 4\n") ;
#endif

	    bflush(ofp) ;

	} /* end while (reading lines) */

	ex = EX_OK ;

	bclose(ifp) ;

ret2:
	bclose(ofp) ;

ret1:
	bclose(efp) ;

ret0:
	return ex ;
}
/* end subroutine (main) */



