/* testucopen */
/* lang=C89 */

#define	CF_DEBUGS	1		/* compile-time debugging */
#define	CF_DEBUGMALL	1		/* debugging memory-allocations */

/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Synopsis:
	$ testucopen.x <file>


*******************************************************************************/


#include	<envstandards.h>
#include	<sys/types.h>
#include	<stdarg.h>
#include	<stdio.h>
#include	<vsystem.h>
#include	<fsdir.h>
#include	<filebuf.h>
#include	<localmisc.h>

#ifndef FILEBUF_RCNET
#define	FILEBUF_RCNET	4		/* read-count for network */
#endif

#ifndef	TIMEBUFLEN
#define	TIMEBUFLEN	80
#endif

#define	VARDEBUGFNAME	"TESTUCOPEN_DEBUGFILE"

extern int	fbwrite(FILE *,const void *,int) ;
extern int	bufprintf(char *,int,const char *,...) ;

#if	CF_DEBUGS
extern int	debugopen(const char *) ;
extern int	debugprintf(const char *,...) ;
extern int	debugclose() ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern cchar 	*getourenv(const char **,const char *) ;

extern char	*timestr_logz(time_t,char *) ;

/* forward references */

static int dumpfile(int,int) ;
static int dumpdir(int,int) ;

#ifdef	COMMENT
static int filebuf_oread(FILEBUF *,void *,int,int) ;
static int filebuf_refill(FILEBUF *,int) ;
#endif /* COMMENT */

/* exported subroutines */

int main(int argc,const char **argv,const char **envv)
{

#if	CF_DEBUGS && CF_DEBUGMALL
	uint		mo_start = 0 ;
#endif

	int		rs = SR_OK ;
	int		rs1 ;

#if	CF_DEBUGS
	{
	    const char	*cp ;
	    if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL)
	        debugopen(cp) ;
	    debugprintf("main: starting\n") ;
	}
#endif /* CF_DEBUGS */

#if	CF_DEBUGS && CF_DEBUGMALL
	uc_mallset(1) ;
	uc_mallout(&mo_start) ;
#endif

	if (argv != NULL) {
	    const int	llen = LINEBUFLEN ;
	    int		ai ;
	    char	lbuf[LINEBUFLEN+1] ;
	    for (ai = 1 ; (ai < argc) && (argv[ai] != NULL) ; ai += 1) {
	        const char	*fn = argv[ai] ;
	        const int	of = O_RDONLY ;
#if	CF_DEBUGS
	        debugprintf("main: fn=%s\n",fn) ;
#endif
	        if ((rs1 = uc_open(fn,of,0666)) >= 0) {
	            struct ustat	sb ;
	            int			fd = rs1 ;
#if	CF_DEBUGS
	            debugprintf("main: uc_open() rs=%d\n",rs1) ;
#endif
	            if ((rs = u_fstat(fd,&sb)) >= 0) {
#if	CF_DEBUGS
	            debugprintf("main: mode=\\x%08x\n",sb.st_mode) ;
#endif
	                if (S_ISDIR(sb.st_mode)) {
	                    rs = dumpdir(fd,of) ;
	                } else {
	                    rs = dumpfile(fd,of) ;
	                }
	            }
	            u_close(fd) ;
	        } else if (rs1 == SR_NOTFOUND) {
	            rs = SR_OK ;
	            printf("not_found fn=%s (%d)\n",fn,rs1) ;
	        } else {
	            printf("open-error fn=%s (%d)\n",fn,rs1) ;
		}
#if	CF_DEBUGS
	        debugprintf("main: uc_open-out rs=%d\n",rs1) ;
#endif
	        if (rs < 0) break ;
	    } /* end for */
	} /* end if (arguments) */

	if (rs < 0)
	printf("failure (%d)\n",rs) ;

#if	CF_DEBUGS
	debugprintf("main: out rs=%d\n",rs) ;
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

	return 0 ;
}
/* end subroutine (main) */


/* local subroutines */


static int dumpfile(int fd,int of)
{
	FILEBUF		b ;
	FILE		*ofp = stdout ;
	const int	to = 5 ;
	const int	fo = (of | O_NETWORK) ;
	int		rs ;
#if	CF_DEBUGS
	debugprintf("main/dumpfile: ent to=%d\n",to) ;
#endif
	if ((rs = filebuf_start(&b,fd,0L,0,fo)) >= 0) {
	    const int	llen = LINEBUFLEN ;
	    int		li ;
	    char	lbuf[LINEBUFLEN+1] ;
	    while ((rs = filebuf_readline(&b,lbuf,llen,to)) > 0) {
	        int	len = rs ;
#if	CF_DEBUGS
	        debugprintf("main/dumpfile: readline() len=%d\n",len) ;
	        debugprintf("main/dumpfile: l=>%t<\n",
			lbuf,strlinelen(lbuf,len,50)) ;
#endif
	        fbwrite(ofp,lbuf,len) ;
	        if (rs < 0) break ;
	    } /* end while */
	    filebuf_finish(&b) ;
	} /* end if (filebuf) */

#if	CF_DEBUGS
	debugprintf("main/dumpfile: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (dumpfile) */


static int dumpdir(int fd,int of)
{
	FSDIR		d ;
	FSDIR_ENT	de ;
	const int	dlen = MAXPATHLEN ;
	int		rs ;
	char		dbuf[USERNAMELEN+1] ;

#if	CF_DEBUGS
	debugprintf("main/dumpdir: ent\n") ;
#endif
	if ((rs = bufprintf(dbuf,dlen,"/dev/fd/%u",fd)) >= 0) {
	    if ((rs = fsdir_open(&d,dbuf)) >= 0) {
	        while ((rs = fsdir_read(&d,&de)) > 0) {
	            printf("e=%s\n",de.name) ;
	        } /* end while */
	        fsdir_close(&d) ;
	    } /* end if (fsdir) */
	} /* end if */

#if	CF_DEBUGS
	debugprintf("main/dumpdir: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (dumpdir) */


#ifdef	COMMENT

static int filebuf_oread(op,rbuf,rlen,to)
FILEBUF		*op ;
void		*rbuf ;
int		rlen ;
int		to ;
{
	int	rs = SR_OK ;
	int	mlen ;
	int	rc ;
	int	tlen = 0 ;
	int	f_timedout = FALSE ;

	char	*dbp = (char *) rbuf ;
	char	*bp, *lastp ;

	if (op == NULL) return SR_FAULT ;

#if	CF_DEBUGS
	debugprintf("filebuf_oread: rlen=%d to=%d\n",rlen,to) ;
#endif

	rc = (op->f.net) ? FILEBUF_RCNET : 1 ;
	while (tlen < rlen) {

#if	CF_DEBUGS
	    debugprintf("filebuf_oread: 0 while-top tlen=%d op->len=%d\n", 
	        tlen,op->len) ;
#endif
	    if (op->len <= 0) {
	        rs = filebuf_refill(op,to) ;
	        if ((rs == SR_TIMEDOUT) && (tlen > 0)) {
	            rs = SR_OK ;
	            f_timedout = TRUE ;
	        }
	    }

#if	CF_DEBUGS
	    debugprintf("filebuf_oread: refilled rs=%d f_to=%u\n",
		rs,f_timedout) ;
	    debugprintf("filebuf_oread: op->len=%d tlen=%d\n", op->len,tlen) ;
#endif

	    if ((op->len == 0) || f_timedout)
	        break ;

	    mlen = MIN(op->len,(rlen - tlen)) ;

	    bp = op->bp ;
	    lastp = op->bp + mlen ;
	    while (bp < lastp)
	        *dbp++ = *bp++ ;

	    op->bp += mlen ;
	    tlen += mlen ;
	    op->len -= mlen ;

	} /* end while */

	if (rs >= 0)
	    op->off += tlen ;

	if (rs == SR_TIMEDOUT) {
	    char	tbuf[10+1] = { 
	        0 		} ;
	    int	i ;
	    rs = u_write(op->fd,tbuf,0) ;
#if	CF_DEBUGS
	    debugprintf("filebuf_oread: timed-out? rs=%d\n",rs) ;
#endif
	    for (i = 0 ; i < 4 ; i += 1) {
	        rs = u_read(op->fd,tbuf,10) ;
#if	CF_DEBUGS
	        debugprintf("filebuf_oread: u_read() rs=%d\n",rs) ;
#endif
	    }
	} /* end if (timed-out) */

#if	CF_DEBUGS
	debugprintf("filebuf_oread: ret rs=%d tlen=%u\n",rs,tlen) ;
#endif

	return (rs >= 0) ? tlen : rs ;
}
/* end subroutine (filebuf_read) */


static int filebuf_refill(FILEBUF *op,int to)
{
	const int	fmo = FM_TIMED ;
	int	rs = SR_OK ;
	int	rc = 4 ;
	int	tlen = 0 ;

	while ((op->len <= 0) && (rc-- > 0)) {

#if	CF_DEBUGS
	    debugprintf("filebuf_refill: 1 while-top tlen=%d len=%d rc=%d\n",
	        tlen,op->len,rc) ;
	    debugprintf("filebuf_refill: reading=%d to=%d\n",op->bufsize,to) ;
#endif
	    op->bp = op->buf ;
	    if (to >= 0) {
	        rs = uc_reade(op->fd,op->buf,op->bufsize,to,fmo) ;
	    } else
	        rs = u_read(op->fd,op->buf,op->bufsize) ;

#if	CF_DEBUGS
	    debugprintf("filebuf_refill: read rs=%d\n",rs) ;
#endif

	    if ((rs == SR_TIMEDOUT) && (tlen > 0)) {
	        rs = SR_OK ;
	        break ;
	    } else if (rs == 0) {
	        to = 0 ;
	    }

	    if (rs < 0) break ;
	    op->len = rs ;
	    tlen += rs ;
	} /* end while (refill) */

#if	CF_DEBUGS
	debugprintf("filebuf_refill: ret rs=%d tlen=%d\n",rs,tlen) ;
#endif
	return (rs >= 0) ? tlen : rs ;
}
/* end subroutine (filebuf_refill) */

#endif /* COMMENT */


