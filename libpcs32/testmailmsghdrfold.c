/* testmailmsghdrfold */
/* lang=C89 */

/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */

#define	CF_DEBUGS	1
#include	<envstandards.h>
#include	<sys/types.h>
#include	<sys/param.h>
#include	<stdlib.h>
#include	<string.h>
#include	<stdio.h>
#include	<vsystem.h>
#include	<bfile.h>
#include	<filebuf.h>
#include	<localmisc.h>
#include	"mailmsghdrfold.h"

/* local defines */

#define	VARDEBUGFNAME	"TESTMAILMSGHDRFOLD_DEBUGFILE"

#ifndef	MSGCOLS
#define	MSGCOLS		76
#endif

#ifndef	NTABCOLS
#define	NTABCOLS	8
#endif

#if	CF_DEBUGS
extern int	debugopen(const char *) ;
extern int	debugprintf(const char *,...) ;
extern int	debugclose() ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern const char 	*getourenv(const char **,const char *) ;

/* forward references */

static int procprinthdrline(FILEBUF *,int,int,int,const char *,int) ;
static int filebuf_hdrkey(FILEBUF *,const char *) ;
static int	filebuf_printcont(FILEBUF *,int,const char *,int) ;

/* exported subroutines */

int main(int argc,const char **argv,const char **envv)
{
	FILEBUF		ofile, *ofp = &ofile  ;

	const int	mcols = MSGCOLS ;
	const int	hlen = MAXPATHLEN ;

	int	rs ;
	int	wlen = 0 ;

	const char	*hdr = "references" ;
	const char	*ifname = BFILE_STDIN ;

	char	hbuf[MAXPATHLEN+1] ;

#if	CF_DEBUGS
	{
	    const char	*cp ;
	    if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL)
	        debugopen(cp) ;
	    debugprintf("main: starting\n") ;
	}
#endif /* CF_DEBUGS */

	if ((rs = filebuf_start(ofp,1,0L,0,0)) >= 0) {
	        bfile	ifile, *ifp = &ifile ;

	        if ((rs = bopen(ifp,ifname,"r",0666)) >= 0) {
	            const int	llen = LINEBUFLEN ;
	            int		len ;
		    int		ll ;
		    int		ln = 0 ;
		    int		indent = 10 ;
		    const char	*lp ;
	            char	lbuf[LINEBUFLEN+1] ;

	            while ((rs = breadline(ifp,lbuf,llen)) > 0) {
	                len = rs ;
			ln = 0 ;

	                if (lbuf[len-1] == '\n') len -= 1 ;
	                lbuf[len] = '\0' ;

#if	CF_DEBUGS
	debugprintf("main: l=>%t<\n",
		lbuf,strlinelen(lbuf,len,50)) ;
#endif
		        rs = filebuf_hdrkey(ofp,hdr) ;
			wlen += rs ;
			indent = rs ;

#if	CF_DEBUGS
	debugprintf("main: filebuf_hdrkey() rs=%d\n",rs) ;
#endif

			lp = lbuf ;
			ll = len ;
			if (rs >= 0) {
			    rs = procprinthdrline(ofp,mcols,ln,indent,lp,ll) ;
			    wlen += rs ;
			    ln += 1 ;
#if	CF_DEBUGS
	debugprintf("main: procprinthdrline() rs=%d\n",rs) ;
#endif

			}

	                if (rs < 0) break ;
	            } /* end while (reading lines) */

	            bclose(ifp) ;
	        } /* end if (file-open) */

	    filebuf_finish(ofp) ;
	} /* end if (filebuf) */

#if	CF_DEBUGS
	debugprintf("main: done rs=%d\n",rs) ;
#endif

#if	CF_DEBUGS
	debugclose() ;
#endif

	return 0 ;
}
/* end subroutine (main) */


/* local subroutines */


/* "indent" is initial indent only! */
static int procprinthdrline(fbp,mcols,ln,indent,lp,ll)
FILEBUF		*fbp ;
int		mcols ;
int		ln ;
int		indent ;
const char	*lp ;
int		ll ;
{
	MAILMSGHDRFOLD	mf ;

	int	rs ;
	int	wlen = 0 ;

	if ((rs = mailmsghdrfold_start(&mf,mcols,ln,lp,ll)) >= 0) {
	    int	i = 0 ;
	    int	nc = (indent + 1) ;
	    int	leader ;
	    int	sl ;
	    const char	*sp ;

	    while ((sl = mailmsghdrfold_get(&mf,nc,&sp)) > 0) {
	        leader = (i == 0) ? ' ' : '\t' ;
	        rs = filebuf_printcont(fbp,leader,sp,sl) ;
	        wlen += rs ;
	        if (rs < 0) break ;
	        i += 1 ;
	        nc = NTABCOLS ;
	    } /* end while */

	    if ((rs >= 0) && (i == 0)) {
	        rs = filebuf_print(fbp,lp,0) ;
	        wlen += rs ;
	    }

	    mailmsghdrfold_finish(&mf) ;
	} /* end if */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procprinthdrline) */


/* there is no 'filebuf_print(3dam)' subroutine! */
static int filebuf_hdrkey(fbp,kname)
FILEBUF		*fbp ;
const char	kname[] ;
{
	int	rs = SR_OK ;
	int	wlen = 0 ;

	char	buf[2] ;


	if (fbp == NULL)
	    return SR_FAULT ;

	if (kname == NULL)
	    return SR_FAULT ;

	if (kname[0] == '\0')
	    return SR_INVALID ;

	if (rs >= 0) {
	    rs = filebuf_write(fbp,kname,-1) ;
	    wlen += rs ;
	}

	if (rs >= 0) {
	    buf[0] = ':' ;
	    buf[1] = '\0' ;
	    rs = filebuf_write(fbp,buf,1) ;
	    wlen += rs ;
	}

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (filebuf_hdrkey) */


static int filebuf_printcont(fbp,leader,sp,sl)
FILEBUF		*fbp ;
int		leader ;
const char	*sp ;
int		sl ;
{
	int	rs = SR_OK ;
	int	wlen = 0 ;

	char	buf[2] ;


	if (fbp == NULL) return SR_FAULT ;
	if (sp == NULL) return SR_FAULT ;

	if (sl < 0) sl = strlen(sp) ;

	if (sl > 0) {

	    if ((rs >= 0) && (leader > 0)) {
	        buf[0] = leader ;
	        buf[1] = '\0' ;
	        rs = filebuf_write(fbp,buf,1) ;
	        wlen += rs ;
	    }

	    if (rs >= 0) {
	        rs = filebuf_write(fbp,sp,sl) ;
	        wlen += rs ;
	    }

	    if (rs >= 0) {
	        buf[0] = '\n' ;
	        buf[1] = '\0' ;
	        rs = filebuf_write(fbp,buf,1) ;
	        wlen += rs ;
	    }

	} /* end if (non-empty value) */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (filebuf_printcont) */


