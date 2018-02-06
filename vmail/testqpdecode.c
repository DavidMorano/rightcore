/* main (testqpdecode) */
/* lang=C99 */


#define	CF_DEBUGS	1		/* compile-time debugging */
#define	CF_DEBUGMALL	1		/* debug memory-allocations */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


#include	<envstandards.h>

#include	<sys/types.h>
#include	<limits.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<ascii.h>
#include	<bfile.h>
#include	<localmisc.h>

#include	"qpdecoder.h"

#include	"defs.h"
#include	"config.h"

/* local defines */

#ifndef	MKCHAR
#define	MKCHAR(ch)	((ch) & UCHAR_MAX)
#endif

#ifndef	PCS
#define	PCS		"/usr/add-on/pcs"
#endif

#ifndef	LINEBUFLEN
#define	LINEBUFLEN	2048
#endif

#ifndef	PROGINFO
#define	PROGINFO	struct proginfo
#endif


/* external subroutines */

#if	CF_DEBUGS
extern int	debugopen(const char *) ;
extern int	debugprintf(const char *,...) ;
extern int	debugprinthex(const char *,int,const char *,int) ;
extern int	debugclose() ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern const char	*getourenv(cchar **,cchar *) ;


/* forward references */

static int	procfile(PROGINFO *,QPDECODER *,bfile *,cchar *) ;
static int	procout(PROGINFO *,bfile *,QPDECODER *) ;


/* exported subroutines */


int main(int argc,cchar *argv[],cchar *envv[])
{
	PROGINFO	pi, *pip = &pi ;
	QPDECODER	q ;

#if	CF_DEBUGS && CF_DEBUGMALL
	uint		mo_start = 0 ;
#endif

	const int	f_space = TRUE ;

	int		rs = SR_OK ;
	int		rs1 ;
	const char	*pr = PCS ;
	const char	*cp ;

#if	CF_DEBUGS
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("main: starting DFD=%d\n",rs) ;
	}
#endif /* CF_DEBUGS */

#if	CF_DEBUGS && CF_DEBUGMALL
	uc_mallset(1) ;
	uc_mallout(&mo_start) ;
#endif

	memset(pip,0,sizeof(PROGINFO)) ;
	pip->pr = pr ;

/* go */

	if (rs >= 0) {
	    if ((rs = qpdecoder_start(&q,f_space)) >= 0) {

	        if (argv != NULL) {
	            bfile	of ;
	            const char	*ofname = BFILE_STDOUT ;
	            const char	*ifname ;
	            if ((rs = bopen(&of,ofname,"wct",0666)) >= 0) {
	                int	ai ;
	                for (ai = 1 ; ai < argc ; ai += 1) {
	                    ifname = argv[ai] ;
	                    if (ifname[0] != '\0') {
	                        rs = procfile(pip,&q,&of,ifname) ;
	                        bputc(&of,CH_NL) ;
	                    }
	                    if (rs < 0) break ;
	                } /* end for */
	                bclose(&of) ;
	            } /* end if (open-output) */
	        } /* end if (argv) */

	        rs1 = qpdecoder_finish(&q) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (qpdecoder) */
	} /* end if (ok) */

#if	CF_DEBUGS && CF_DEBUGMALL
	{
	    uint	mo ;
	    uc_mallout(&mo) ;
	    debugprintf("main: final mallout=%u\n",(mo-mo_start)) ;
	    uc_mallset(0) ;
	}
#endif

#if	CF_DEBUGS
	debugclose() ;
#endif

	return 0 ;
}
/* end subroutine (main) */


/* local subroutines */


static int procfile(PROGINFO *pip,QPDECODER *qp,bfile *ofp,cchar *ifname)
{
	bfile		ifile, *ifp = &ifile ;
	int		rs ;
	int		wlen = 0 ;

	if (pip == NULL) return SR_FAULT ;
	if (ifname == NULL) return SR_FAULT ;

#if	CF_DEBUGS
	debugprintf("main/procfile: ifname=%s\n",ifname) ;
#endif

	if ((ifname[0] == '\0') || (ifname[0] == '-'))
	    ifname = BFILE_STDIN ;

	if ((rs = bopen(ifp,ifname,"r",0666)) >= 0) {
	    const int	llen = LINEBUFLEN ;
	    char	lbuf[LINEBUFLEN+1] ;

	    while ((rs = breadline(ifp,lbuf,llen)) > 0) {
	        int	len = rs ;

	        if (lbuf[len-1] == '\n') len -= 1 ;
	        lbuf[len] = '\0' ;

	        rs = qpdecoder_load(qp,lbuf,len) ;

	        if (rs < 0) break ;
	    } /* end while */

	    bclose(ifp) ;
	} /* end if (input-file) */

	if (rs >= 0) {
	    rs = procout(pip,ofp,qp) ;
	    wlen += rs ;
	}

#if	CF_DEBUGS
	debugprintf("main/procfile: ret rs=%d wlen=%u\n",rs,wlen) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procfile) */


static int procout(PROGINFO *pip,bfile *ofp,QPDECODER *qp)
{
	const int	llen = LINEBUFLEN ;
	int		rs ;
	int		wlen = 0 ;
	char	lbuf[LINEBUFLEN+1] ;
	if (pip == NULL) return SR_FAULT ;
	while ((rs = qpdecoder_read(qp,lbuf,llen)) > 0) {
	    rs = bwrite(ofp,lbuf,rs) ;
	    wlen += rs ;
	    if (rs < 0) break ;
	} /* end for */
#if	CF_DEBUGS
	debugprintf("main/procout: ret rs=%d wlen=%u\n",rs,wlen) ;
#endif
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procout) */


