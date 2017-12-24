/* main (testhdrdecode) */
/* lang=C99 */


#define	CF_DEBUGS	1		/* compile-time debugging */
#define	CF_DEBUGMALL	1		/* debug memory-allocations */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Synopsis:

	$ testhdrdecode.x < <testfile>


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<limits.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<ascii.h>
#include	<bfile.h>
#include	<localmisc.h>

#include	"hdrdecode.h"

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

#define	OBUFLEN		(LINEBUFLEN*2)

#ifndef	PROGINFO
#define	PROGINFO	struct proginfo
#endif


/* external subroutines */

extern int	snwcpywidehdr(char *,int,const wchar_t *,int) ;
extern int	isprintlatin(int) ;
extern int	isprintbad(int) ;

#if	CF_DEBUGS
extern int	debugopen(const char *) ;
extern int	debugprintf(const char *,...) ;
extern int	debugprinthex(const char *,int,const char *,int) ;
extern int	debugclose() ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern cchar	*getourenv(cchar **,cchar *) ;


/* forward references */

static int	procfile(PROGINFO *,HDRDECODE *,bfile *,cchar *) ;
static int	procline(PROGINFO *,bfile *,HDRDECODE *,cchar *,int) ;

#if	CF_DEBUGS
static int debugprintchars(cchar *,const wchar_t *,int) ;
#endif


/* local variables */


/* exported subroutines */


int main(int argc,cchar *argv[],cchar *envv[])
{
	PROGINFO	pi, *pip = &pi ;
	HDRDECODE	q ;

#if	CF_DEBUGS && CF_DEBUGMALL
	uint		mo_start = 0 ;
#endif

	int		rs = SR_OK ;
	int		ex = 0 ;
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
	    if ((rs = hdrdecode_start(&q,pr)) >= 0) {

	        if (argv != NULL) {
	            bfile	of ;
	            cchar	*ofname = BFILE_STDOUT ;
	            if ((rs = bopen(&of,ofname,"wct",0666)) >= 0) {
	                int	ai ;
	                cchar	*ifname ;
	                for (ai = 1 ; ai < argc ; ai += 1) {
	                    ifname = argv[ai] ;
	                    if (ifname[0] != '\0') {
	                        rs = procfile(pip,&q,&of,ifname) ;
	                    }
	                    if (rs < 0) break ;
	                } /* end for */
	                bclose(&of) ;
	            } /* end if (open-output) */
	        } /* end if (argv) */

	        rs1 = hdrdecode_finish(&q) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (hdrdecode) */
	} /* end if (ok) */

#if	CF_DEBUGS
	debugprintf("main: done rs=%d\n",rs) ;
#endif

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

	if (rs < 0) ex = 1 ;
	return ex ;
}
/* end subroutine (main) */


/* local subroutines */


static int procfile(PROGINFO *pip,HDRDECODE *qp,bfile *ofp,cchar *ifn)
{
	bfile		ifile, *ifp = &ifile ;
	int		rs ;
	int		wlen = 0 ;

	if (pip == NULL) return SR_FAULT ;
	if (ifn == NULL) return SR_FAULT ;

#if	CF_DEBUGS
	debugprintf("main/procfile: ifn=%s\n",ifn) ;
#endif

	if ((ifn[0] == '\0') || (ifn[0] == '-'))
	    ifn = BFILE_STDIN ;

	if ((rs = bopen(ifp,ifn,"r",0666)) >= 0) {
	    const int	llen = LINEBUFLEN ;
	    char	lbuf[LINEBUFLEN+1] ;

	    while ((rs = breadline(ifp,lbuf,llen)) > 0) {
	        int	len = rs ;
	        if (lbuf[len-1] == '\n') len -= 1 ;
	        lbuf[len] = '\0' ;
		if (len > 0) {
	            rs = procline(pip,ofp,qp,lbuf,len) ;
	            wlen += rs ;
	        }
	        if (rs < 0) break ;
	    } /* end while */

	    bclose(ifp) ;
	} /* end if (input-file) */

#if	CF_DEBUGS
	debugprintf("main/procfile: ret rs=%d wlen=%u\n",rs,wlen) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procfile) */


static int procline(PROGINFO *pip,bfile *ofp,HDRDECODE *qp,cchar *sp,int sl)
{
	const int	wlen = LINEBUFLEN ;
	int		rs = SR_OK ;
	int		tlen = 0 ;
	wchar_t		wbuf[LINEBUFLEN+1] ;
	if (pip == NULL) return SR_FAULT ;
#if	CF_DEBUGS
	debugprintf("main/procline: ent sl=%d >%t<\n",sl,
		sp,strlinelen(sp,sl,40)) ;
#endif
	if (sl > 0) {
	    if ((rs = hdrdecode_proc(qp,wbuf,wlen,sp,sl)) >= 0) {
	        const int	olen = OBUFLEN ;
	        const int	wl = rs ;
	        char		obuf[OBUFLEN+1] ;
#if	CF_DEBUGS
	        debugprintf("main/procline: hdrdecode_proc() rs=%d\n",rs) ;
	            debugprintchars("procline",wbuf,wl) ;
#endif
	        if ((rs = snwcpywidehdr(obuf,olen,wbuf,wl)) >= 0) {
		    rs = bprintline(ofp,obuf,rs) ;
	            tlen += rs ;
	        }
	    } /* end if (hdrdecode_proc) */
	} /* end if (positive) */
#if	CF_DEBUGS
	debugprintf("main/procline: ret rs=%d tlen=%u\n",rs,tlen) ;
#endif
	return (rs >= 0) ? tlen : rs ;
}
/* end subroutine (procoutline) */


#if	CF_DEBUGS
static int debugprintchars(cchar *id,const wchar_t *wbuf,int wlen)
{
	int	i ;
	for (i = 0 ; i < wlen ; i += 1) {
	    const int	ch = wbuf[i] ;
	    debugprintf("main/%s: wc[%02u]=%08x\n",id,i,ch) ;
	}
	return 0 ;
}
/* end subroutine (debugprintchars) */
#endif /* CF_DEBUGS */

