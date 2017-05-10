/* mailmsg_loadmb */

/* load a mail-message from a MAILBOX object */


#define	CF_DEBUGS	0		/* switchable debug print-outs */


/* revision history:

	= 1998-03-01, David A­D­ Morano
	The subroutine was written from scratch.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine loads a mail-message (into the MAILMSG object) from a
        source that consists of a MAILBOX object. The MAILBOX object also reads
        the associated mail-box where the mail-msg is located, so it can provide
        access to that same mail-box. It does so through a read-interface that
        is a small subset of the full MAILBOX interface.


        Note: At first we skip empty lines until we find a non-empty line;
        afterwards we do not ignore empty lines.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<mailbox.h>
#include	<mailmsg.h>
#include	<localmisc.h>


/* local defines */

#ifndef	MAILMSGLINEBUFLEN
#define	MAILMSGLINEBUFLEN	(LINEBUFLEN * 5)
#endif

#define	MAILMSG_BSIZE	0		/* let it figure out what is best */

#define	ISEND(c)	(((c) == '\n') || ((c) == '\r'))


/* external subroutines */

extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	sncpy1w(char *,int,const char *,int) ;
extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	sfbasename(const char *,int,const char **) ;
extern int	sfshrink(const char *,int,const char **) ;
extern int	sfsub(const char *,int,const char *,const char **) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strdcpy1w(char *,int,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;


/* external variables */


/* local structures */


/* forward references */


/* global variables */


/* local variables */


/* exported subroutines */


int mailmsg_loadmb(MAILMSG *mmp,MAILBOX *mbp,offset_t fbo)
{
	MAILBOX_READ	cur ;
	const int	bsize = MAILMSG_BSIZE ;
	int		rs ;
	int		rs1 ;
	int		line = 0 ;
	int		tlen = 0 ;

#if	CF_DEBUGS
	debugprintf("mailmsg_loadmb: ent fbo=%llu\n",fbo) ;
#endif

	if ((rs = mailbox_readbegin(mbp,&cur,fbo,bsize)) >= 0) {
	    const int	llen = MAILMSGLINEBUFLEN ;
	    char	*lbuf ;
	    if ((rs = uc_malloc((llen+1),&lbuf)) >= 0) {
	        int	ll ;

#if	CF_DEBUGS
	        debugprintf("mailmsg_loadmb: mailbox_readbegin\n") ;
#endif

	        while ((rs = mailbox_readline(mbp,&cur,lbuf,llen)) > 0) {
	            ll = rs ;

#if	CF_DEBUGS
	            debugprintf("mailmsg_loadmb: mailbox_read() rs=%d\n",rs) ;
	            debugprintf("mailmsg_loadmb: l=>%t<\n",
	                lbuf,strlinelen(lbuf,ll,40)) ;
#endif

	            tlen += ll ;
	            while ((ll > 0) && ISEND(lbuf[0]))
	                ll -= 1 ;

	            if ((ll > 0) || (line > 0)) {
	                line += 1 ;
	                rs = mailmsg_loadline(mmp,lbuf,ll) ;
	            }

	            if (rs <= 0) break ;
	        } /* end while (reading lines) */

#if	CF_DEBUGS
	        debugprintf("mailmsg_loadmb: while-out rs=%d\n",rs) ;
#endif

	        rs1 = uc_free(lbuf) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (memory-allocation) */
	    rs1 = mailbox_readend(mbp,&cur) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (mailbox) */

#if	CF_DEBUGS
	debugprintf("mailmsg_loadmb: ret rs=%d tlen=%u\n",rs,tlen) ;
#endif

	return (rs >= 0) ? tlen : rs ;
}
/* end subroutine (mailmsg_loadmb) */


