/* main (testchartrans) */
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
#include	<bfile.h>
#include	<linefold.h>
#include	<localmisc.h>

#include	"chartrans.h"
#include	"termtrans.h"

#include	"defs.h"
#include	"config.h"

/* local defines */

#ifndef	MKCHAR
#define	MKCHAR(ch)	((ch) & UCHAR_MAX)
#endif

#ifndef	PCS
#define	PCS		"/usr/add-on/pcs"
#endif

#ifndef	WCHARLEN
#define	WCHARLEN	(2*LINEBUFLEN)
#endif

#ifndef	PROGINFO
#define	PROGINFO	struct proginfo
#endif


/* external subroutines */

extern int	optvalue(cchar *,int) ;

#if	CF_DEBUGS
extern int	debugopen(const char *) ;
extern int	debugprintf(const char *,...) ;
extern int	debugprinthex(const char *,int,const char *,int) ;
extern int	debugclose() ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern const char	*getourenv(cchar **,cchar *) ;


/* forward references */

static int	procfile(PROGINFO *,CHARTRANS *,TERMTRANS *,bfile *,
			const char *) ;
static int	procoutlines(PROGINFO *,bfile *,TERMTRANS *,
			const wchar_t *,int) ;
static int	procoutline(PROGINFO *,bfile *,int,int,const char *,int) ;


/* exported subroutines */


int main(int argc,cchar *argv[],cchar *envv[])
{
	PROGINFO	pi, *pip = &pi ;
	TERMTRANS	tt ;
	CHARTRANS	ec ;

#if	CF_DEBUGS && CF_DEBUGMALL
	uint		mo_start = 0 ;
#endif

	int		rs = SR_OK ;
	int		rs1 ;
	int		cols = 0 ;
	const char	*termtype = getenv(VARTERM) ;
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

	if ((rs >= 0) && (cols == 0)) {
	    if ((cp = getenv(VARCOLUMNS)) != NULL) {
	        rs = optvalue(cp,-1) ;
	        cols = rs ;
	    }
	}

	if ((rs >= 0) && (cols == 0)) {
	    cols = COLUMNS ;
	}

	pip->pr = pr ;
	pip->linelen = cols ;

/* go */

	if (rs >= 0) {
	if ((rs = termtrans_start(&tt,pr,termtype,-1,cols)) >= 0) {

	    if ((rs = chartrans_open(&ec,pr,6)) >= 0) {

	        if (argv != NULL) {
	            bfile	of ;
	            const char	*ofname = BFILE_STDOUT ;
		    const char	*ifname ;
	            if ((rs = bopen(&of,ofname,"wct",0666)) >= 0) {
	                int	ai ;
	                for (ai = 1 ; ai < argc ; ai += 1) {
			    ifname = argv[ai] ;
			    if (ifname[0] != '\0') {
	                        rs = procfile(pip,&ec,&tt,&of,ifname) ;
			    }
	                    if (rs < 0) break ;
	                } /* end for */
	                bclose(&of) ;
	            } /* end if (open-output) */
	        } /* end if (argv) */

	        rs1 = chartrans_close(&ec) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (chartrans) */

	    rs1 = termtrans_finish(&tt) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (termtrans) */
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


static int procfile(pip,op,ttp,ofp,ifname)
PROGINFO	*pip ;
CHARTRANS	*op ;
TERMTRANS	*ttp ;
bfile		*ofp ;
const char	*ifname ;
{
	bfile		ifile, *ifp = &ifile ;
	wchar_t		*wbuf ;
	time_t		dt = time(NULL) ;
	const int	wlen = WCHARLEN ;
	int		rs ;
	int		wsize ;
	int		rc = 0 ;
	const char	*ics = "UTF-8" ;

	if (ifname == NULL) return SR_FAULT ;

#if	CF_DEBUGS
	debugprintf("main/procfile: ifname=%s\n",ifname) ;
#endif

	if ((ifname[0] == '\0') || (ifname[0] == '-'))
	    ifname = BFILE_STDIN ;

	wsize = (wlen * sizeof(wchar_t)) ;
	if ((rs = uc_malloc(wsize,&wbuf)) >= 0) {

	    if ((rs = chartrans_transbegin(op,dt,ics,-1)) >= 0) {
	        const int	txid = rs ;

#if	CF_DEBUGS
	        debugprintf("main/procfile: txid=%u\n",txid) ;
#endif

	        if ((rs = bopen(ifp,ifname,"r",0666)) >= 0) {
	            const int	llen = LINEBUFLEN ;
	            char	lbuf[LINEBUFLEN+1] ;

	            while ((rs = breadline(ifp,lbuf,llen)) > 0) {
	                int	len = rs ;

#ifdef	COMMENT
	                if (lbuf[len-1] == '\n') len -= 1 ;
	                lbuf[len] = '\0' ;
#endif

	                rs = chartrans_transread(op,txid,wbuf,wlen,lbuf,len) ;

#if	CF_DEBUGS
	                debugprintf("main/procfile: "
	                    "chartrans_transread() rs=%d\n",
	                    rs) ;
#endif

	                if (rs >= 0) {
	                    int	i, j ;
	                    int	otrans = rs ;
	                    for (i = 0 ; i < otrans ; i += 1) {
	                        uint	uv = (uint) wbuf[i] ;
	                        int	ch ;
	                        for (j = 3 ; (rs >= 0) && (j >= 0) ; j -= 1) {
	                            ch = MKCHAR((uv >> (8*j))) ;
	                            rs = bprintf(ofp," %02x",ch) ;
	                        }
	                        if (rs >= 0) {
	                            rs = bputc(ofp,'\n') ;
				}
	                        if (rs < 0) break ;
	                    } /* end for */
			    if (rs >= 0) {
				rs = procoutlines(pip,ofp,ttp,wbuf,otrans) ;
			    }
	                } /* end if (ok) */

	                if (rs < 0) break ;
	            } /* end while */

	            bclose(ifp) ;
	        } /* end if (input-file) */

	        chartrans_transend(op,txid) ;
	    } /* end if (chartrans-transaction) */

	    uc_free(wbuf) ;
	} /* end if (memory-allocation) */

#if	CF_DEBUGS
	debugprintf("main/procfile: ret rs=%d rc=%u\n",rs,rc) ;
#endif

	return (rs >= 0) ? rc : rs ;
}
/* end subroutine (procfile) */


static int procoutlines(pip,ofp,ttp,wcbuf,wclen)
PROGINFO	*pip ;
bfile		*ofp ;
TERMTRANS	*ttp ;
const wchar_t	*wcbuf ;
int		wclen ;
{
	const int	lw = pip->linelen ;
	const int	li = 2 ;
	int		rs ;
	int		wlen = 0 ;

#if	CF_DEBUGS
	debugprintf("main/procoutline: ent wlen=%u\n",wlen) ;
#endif

	if ((rs = termtrans_load(ttp,wcbuf,wclen)) >= 0) {
	    int		i ;
	    int		ll ;
	    const char	*lp ;

#if	CF_DEBUGS
	    debugprintf("main/procoutline: 1 \n") ;
#endif

	    for (i = 0 ; (ll = termtrans_getline(ttp,i,&lp)) > 0 ; i += 1) {
#if	CF_DEBUGS
		debugprintf("main/procoutline: 2 \n") ;
#endif
		rs = procoutline(pip,ofp,lw,li,lp,ll) ;
		wlen += rs ;

		if (rs < 0) break ;
	    } /* end for */
	} /* end if (termtrans_load) */

#if	CF_DEBUGS
	debugprintf("main/procoutline: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (procoutlines) */


static int procoutline(pip,ofp,lw,li,lp,ll)
PROGINFO	*pip ;
bfile		*ofp ;
int		lw ;
int		li ;
const char	*lp ;
int		ll ;
{
	LINEFOLD	lf ;
	int		rs ;
	int		wlen = 0 ;
	if ((rs = linefold_start(&lf,lw,li,lp,ll)) >= 0) {
	    int		j ;
	    int		cl ;
	    const char	*cp ;
	    for (j = 0 ; (cl = linefold_get(&lf,j,&cp)) >= 0 ; j += 1) {
	        rs = bprintline(ofp,cp,cl) ;
		wlen += rs ;
		if (rs < 0) break ;
	    } /* end for */
	} /* end if (linefold) */
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procoutline) */


