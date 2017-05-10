/* main (C89) - testconseq */

#define	CF_DEBUGS	1		/* non-switchable debug print-outs */
#define	CF_BLANKS	0
#define	CF_SD		0		/* use status display */

#define	DBUFLEN	200
#define	NBUFLEN	40

#include	<stdlib.h>
#include	<stdio.h>

#include	<vsystem.h>
#include	<termstr.h>
#include	<sbuf.h>
#include	<localmisc.h>

#include	"config.h"

extern int	termconseq(char *,int,int,int,int,int,int) ;

extern const char	*getourenv(const char **,const char *) ;

#if	CF_DEBUGS
extern int	debugopen(const char *) ;
extern int	debugprintf(const char *,...) ;
extern int	debugprinthexblock(const char *,int,const void *,int) ;
extern int	debugclose() ;
extern int	strlinelen(const char *,int,int) ;
#endif



int main(int argc,const char **argv,const char **envv)
{
	SBUF	b ;

	const int	dlen = DBUFLEN ;
	const int	nlen = NBUFLEN ;
	int	rs ;
	int	dl ;
	int	nl ;
	char	dbuf[DBUFLEN+1] ;
	char	nbuf[NBUFLEN+1] ;


#if	CF_DEBUGS 
	{
	    const char	*cp ;
	if ((cp = getourenv(envv,VARDEBUGFNAME)) == NULL) {
	    if ((cp = getourenv(envv,VARDEBUGFD1)) == NULL)
	        cp = getourenv(envv,VARDEBUGFD2) ;
	}
	if (cp != NULL)
	    debugopen(cp) ;
	debugprintf("main: starting\n") ;
	}
#endif /* CF_DEBUGS */


	if ((rs = sbuf_start(&b,dbuf,dlen)) >= 0) {

	sbuf_strw(&b,TERMSTR_VCURS,-1) ; /* save cursor */

#if	CF_SD
	sbuf_strw(&b,TERMSTR_S_SD,-1) ; /* set status-display mode */
#endif /* CF_SD */

	sbuf_char(&b,'\r') ;

	sbuf_strw(&b,"xxx",-1) ;

#if	CF_BLANKS
	nl = sncpy1(nbuf,nlen,"     ") ;
#else
	rs = termconseq(nbuf,nlen,'X',4,-1,-1,-1) ;
	nl = rs ;
#endif /* CF_BLANKS */

	fprintf(stderr,"main: termconseq() rs=%d\n",rs) ;

	sbuf_strw(&b,nbuf,nl) ;

#if	CF_SD
	sbuf_strw(&b,TERMSTR_R_SD,-1) ; /* restore status-display mode */
#endif /* CF_SD */

	sbuf_strw(&b,TERMSTR_VCURR,-1) ; /* restore cursor */

	    dl = sbuf_finish(&b) ;
	    if (rs >= 0) rs = dl ;
	} /* end if */

	fprintf(stderr,"main: sbuf rs=%d dl=%d\n",rs,dl) ;
#if	CF_DEBUGS
	debugprinthexblock("main",80,dbuf,dl) ;
#endif

	if (rs >= 0) {
	    rs = u_write(1,dbuf,dl) ;
	}

#if	CF_DEBUGS
	debugclose() ;
#endif

	return 0 ;
}
/* end subroutine (main) */


