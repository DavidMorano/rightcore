/* proghdr */

/* translate a header string into Latin-1 character set */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_DEBUG	0		/* run-time debug print-outs */


/* revision history:

	= 2018-01-22, David A­D­ Morano
	This is an enhancement.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        These subroutine facilitate translating header strings into the Latin-1
        character set.


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<estrings.h>
#include	<hdrdecode.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */


/* external subroutines */

extern int	mkpath3(char *,cchar *,cchar *,cchar *) ;
extern int	mkdisphdr(char *,int,cchar *,int) ;
extern int	isNotPresent(int) ;


#if	CF_DEBUGS || CF_DEBUG
extern int	debugprintf(cchar *,...) ;
extern int	strlinelen(cchar *,int,int) ;
#endif

extern cchar	*getourenv(cchar **,cchar *) ;

extern char	*strnchr(cchar *,int,int) ;


/* external variables */


/* forward references */

static int	proghdr_startup(PROGINFO *) ;


/* local variables */


/* exported subroutines */


int proghdr_begin(PROGINFO *pip)
{
	return SR_OK ;
}
/* end subroutine (proghdr_begin) */


int proghdr_end(PROGINFO *pip)
{
	int		rs = SR_OK ;
	int		rs1 ;
	if (pip->hdr != NULL) {
	    HDRDECODE	*hdrp = (HDRDECODE *) pip->hdr ;
	    rs1 = hdrdecode_finish(hdrp) ;
	    if (rs >= 0) rs = rs1 ;
	    rs1 = uc_free(pip->hdr) ;
	    if (rs >= 0) rs = rs1 ;
	    pip->hdr = NULL ;
	}
	return rs ;
}
/* end subroutine (proghdr_end) */


int proghdr_trans(PROGINFO *pip,char *rbuf,int rlen,cchar *sp,int sl,int c)
{
	int		rs ;
	int		rl = 0 ;
	if (sl < 0) sl = strlen(sp) ;
#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	debugprintf("proghdr_trans: ent sl=%d c=%u\n",sl,c) ;
#endif
	if ((rs = proghdr_startup(pip)) >= 0) {
	    HDRDECODE	*hdp = (HDRDECODE *) pip->hdr ;
	    const int	wsize = ((sl+1) * sizeof(wchar_t)) ;
	    const int	wlen = sl ;
	    wchar_t	*wbuf ;
	    if ((rs = uc_malloc(wsize,&wbuf)) >= 0) {
	        if ((rs = hdrdecode_proc(hdp,wbuf,wlen,sp,sl)) >= 0) {
		    const int	n = MIN(c,rs) ;
		    int		tlen ;
		    char	*tbuf ;
		    tlen = (n*2) ;
		    if ((rs = uc_malloc((tlen+1),&tbuf)) >= 0) {
		        if ((rs = snwcpywidehdr(tbuf,tlen,wbuf,n)) >= 0) {
			    rs = mkdisphdr(rbuf,rlen,tbuf,rs) ;
		            rl = rs ;
		        }
		        uc_free(tbuf) ;
		    } /* end if (m-a-f) */
	        } /* end if (hdrdecode_proc) */
	        uc_free(wbuf) ;
	    } /* end if (m-a-f) */
	} /* end if (proghdr_startup) */
#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	debugprintf("proghdr_trans: ret rs=%d rl=%d\n",rs,rl) ;
#endif
	return (rs >= 0) ? rl : rs ;
}
/* end if (proghdr_trans) */


/* local subroutines */


static int proghdr_startup(PROGINFO *pip)
{
	int		rs = SR_OK ;
#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	debugprintf("proghdr_startup: ent\n") ;
#endif
	if (pip->hdr == NULL) {
	    HDRDECODE	*hdrp ;
	    const int	osize = sizeof(HDRDECODE) ;
	    if ((rs = uc_malloc(osize,&hdrp)) >= 0) {
		cchar	*pr = pip->pr ;
	        if ((rs = hdrdecode_start(hdrp,pr)) >= 0) {
#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	debugprintf("proghdr_startup: hdrdecode_startup() rs=%d\n",rs) ;
#endif
		    pip->hdr = hdrp ;
		}
		if (rs < 0) {
		    uc_free(hdrp) ;
		}
	    } /* end if (m-a) */
	} /* end if (needed) */
#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	debugprintf("proghdr_startup: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (proghdr_startup) */


