/* sesnotes */

/* send notes to sessions */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1999-07-21, David A­D­ Morano

	This module was originally written.


*/

/* Copyright © 1999 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This module manages the sending of messages (notes) to sessions.


*******************************************************************************/


#define	SESNOTES_MASTER	0


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<string.h>

#include	<vsystem.h>
#include	<sockaddress.h>
#include	<fsdir.h>
#include	<localmisc.h>

#include	"sesnotes.h"
#include	"sesmsg.h"


/* local defines */

#define	SESNOTES_PROGDNAME	"sesnotes"

#define	NDF			"sesnotes.deb"


/* external subroutines */

extern int	snsd(char *,int,const char *,uint) ;
extern int	mkpath1(char *,const char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	pathadd(char *,int,const char *) ;
extern int	mktmpuserdir(char *,const char *,const char *,mode_t) ;
extern int	opentmpusd(const char *,int,mode_t,char *) ;
extern int	isNotPresent(int) ;

#if	CF_DEBUGS
extern int	debugopen(const char *) ;
extern int	debugprintf(const char *,...) ;
extern int	debugclose() ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strdcpy1(char *,int,const char *) ;
extern char	*strdcpy1w(char *,int,const char *,int) ;


/* local structures */


/* forward references */

static int sesnotes_ready(SESNOTES *) ;
static int sesnotes_sends(SESNOTES *,char *,int,int,int,const char *,int) ;
static int sesnotes_sender(SESNOTES *,const char *,int,int,time_t,
		const char *,int) ;


/* local variables */


/* exported subroutines */


int sesnotes_open(SESNOTES *op,const char *un)
{
	const int	ulen = USERNAMELEN ;

#if	CF_DEBUGS
	debugprintf("sesnotes_open: ent un=%s\n",un) ;
#endif

	if (op == NULL) return SR_FAULT ;
	if (un == NULL) return SR_FAULT ;
	if (un[0] == '\0') return SR_INVALID ;

	memset(op,0,sizeof(SESNOTES)) ;
	op->pid = ucgetpid() ;
	op->fd = -1 ;
	strdcpy1(op->unbuf,ulen,un) ;
	op->magic = SESNOTES_MAGIC ;

	return SR_OK ;
}
/* end subroutine (sesnotes_open) */


int sesnotes_close(SESNOTES *op)
{
	int		rs = SR_OK ;
	int		rs1 ;
	if (op == NULL) return SR_FAULT ;
	if (op->magic != SESNOTES_MAGIC) return SR_NOTOPEN ;
	if (op->fd >= 0) {
	    rs1 = u_close(op->fd) ;
	    if (rs >= 0) rs = rs1 ;
	    op->fd = -1 ;
	}
	if (op->sfname != NULL) {
	    if (op->sfname[0] != '\0') {
	        uc_unlink(op->sfname) ;
	        op->sfname[0] = '\0' ;
	    }
	    rs1 = uc_free(op->sfname) ;
	    if (rs >= 0) rs = rs1 ;
	    op->sfname = NULL ;
	}
#if	CF_DEBUGS
	debugprintf("sesnotes_close: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (sesnotes_close) */


int sesnotes_sendbiff(SESNOTES *op,const char *sp,int sl,pid_t sid)
{
	const int	mt = sesmsgtype_biff ;
	return sesnotes_send(op,mt,sp,sl,sid) ;
}
/* end subroutine (sesnotes_sendbiff) */


int sesnotes_sendgen(SESNOTES *op,const char *sp,int sl,pid_t sid)
{
	const int	mt = sesmsgtype_gen ;
	return sesnotes_send(op,mt,sp,sl,sid) ;
}
/* end subroutine (sesnotes_sendgen) */


/* ARGSUSED */
int sesnotes_send(SESNOTES *op,int mt,const char *sp,int sl,pid_t sid)
{
	const time_t	st = time(NULL) ;
	uint		uv = sid ;
	const int	clen = MAXNAMELEN ;
	int		rs ;
	int		c = 0 ;
	const char	*sesdname = SESNOTES_SESDNAME ;
	char		cbuf[MAXNAMELEN+1] ;
#if	CF_DEBUGS
	debugprintf("sesnotes_send: ent mt=%u sid=%u\n",mt,sid) ;
#endif
	if (op == NULL) return SR_FAULT ;
	if (op->magic != SESNOTES_MAGIC) return SR_NOTOPEN ;
	if ((rs = snsd(cbuf,clen,"s",uv)) >= 0) {
	    char	sbuf[MAXPATHLEN+1] ;
	    if ((rs = mkpath2(sbuf,sesdname,cbuf)) >= 0) {
	        const int	slen = rs ;
	        rs = sesnotes_sends(op,sbuf,slen,mt,st,sp,sl) ;
	        c += rs ;
	    } /* end if (mkpath) */
	} /* end if (snsd) */
#if	CF_DEBUGS
	debugprintf("sesnotes_send: ret rs=%d c=%u\n",rs,c) ;
#endif
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (sesnotes_send) */


/* private subroutines */


static int sesnotes_ready(SESNOTES *op)
{
	int		rs = SR_OK ;
	if (op->sfname == NULL) {
	    const mode_t	dm = 0775 ;
	    const char		*dname = SESNOTES_PROGDNAME ;
	    char		dbuf[MAXPATHLEN+1] ;
	    if ((rs = mktmpuserdir(dbuf,op->unbuf,dname,dm)) >= 0) {
	        const char	*template = "sesnotesXXXXXX" ;
	        char		rbuf[MAXNAMELEN+1] ;
	        if ((rs = mkpath2(rbuf,dbuf,template)) >= 0) {
	            const mode_t	om = 0666 ;
	            const int		of = (O_CREAT|O_RDWR) ;
	            char		sbuf[MAXPATHLEN+1] ;
	            if ((rs = opentmpusd(rbuf,of,om,sbuf)) >= 0) {
	                const char	*cp ;
	                op->fd = rs ;
	                if ((rs = uc_mallocstrw(sbuf,-1,&cp)) >= 0) {
	                    op->sfname = (char *) cp ;
	                    rs = 1 ;
	                }
	                if (rs < 0) {
	                    u_close(op->fd) ;
	                    op->fd = -1 ;
	                    uc_unlink(sbuf) ;
	                }
	            } /* end if (opentmpusd) */
#if	CF_DEBUGS
	            debugprintf("sesnotes_open: opentmpusd-out rs=%d\n",rs) ;
#endif
	        } /* end if (mkpath) */
	    } /* end if (mktmpuserdir) */
	} /* end if (initializaion needed) */
#if	CF_DEBUGS
	debugprintf("sesnotes_ready: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (sesnotes_ready) */


static int sesnotes_sends(op,sbuf,slen,mt,st,sp,sl)
SESNOTES	*op ;
char		sbuf[] ;
int		slen ;
int		mt ;
int		st ;
const char	*sp ;
int		sl ;
{
	FSDIR		d ;
	FSDIR_ENT	de ;
	int		rs ;
	int		rs1 ;
	int		c = 0 ;
	if ((rs = fsdir_open(&d,sbuf)) >= 0) {
	    while ((rs = fsdir_read(&d,&de)) > 0) {
	        if (de.name[0] == 'p') {
	            if ((rs = pathadd(sbuf,slen,de.name)) >= 0) {
	                struct ustat	sb ;
	                if ((rs = u_lstat(sbuf,&sb)) >= 0) {
	                    if (S_ISSOCK(sb.st_mode)) {
	                        rs = sesnotes_sender(op,sbuf,rs,mt,st,sp,sl) ;
	                        c += rs ;
	                    } /* end if (is-socket) */
	                } /* end if (stat) */
	            } /* end if (pathadd) */
	        } /* end if (is-not-leading-dot) */
	        if (rs < 0) break ;
	    } /* end while (reading) */
	    rs1 = fsdir_close(&d) ;
	    if (rs >= 0) rs = rs1 ;
	} else if (isNotPresent(rs))
	    rs = SR_OK ;
	sbuf[slen] = '\0' ;
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (sesnotes_sends) */


static int sesnotes_sender(op,ap,al,mt,st,sp,sl)
SESNOTES	*op ;
const char	*ap ;
int		al ;
int		mt ;
time_t		st ;
const char	*sp ;
int		sl ;
{
	int		rs ;
	int		f = FALSE ;
#if	CF_DEBUGS
	debugprintf("sesnotes_sender: ent mt=%u\n",mt) ;
	debugprintf("sesnotes_sender: a=>%t<\n",ap,al) ;
	debugprintf("sesnotes_sender: msg=>%t<\n",sp,strlinelen(sp,sl,40)) ;
#endif
	if ((rs = sesnotes_ready(op)) >= 0) {
	    SOCKADDRESS	sa ;
	    uint	tag = op->pid ;
	    const int	af = AF_UNIX ;
	    if ((rs = sockaddress_startaddr(&sa,af,ap,al,0,0)) >= 0) {
	        const int	sal = rs ;
	        const int	mlen = MSGBUFLEN ;
	        int		ml = -1 ;
	        char	mbuf[MSGBUFLEN+1] ;
#if	CF_DEBUGS
	        {
	            const int	cols = 80 ;
	            debugprintf("sesnotes_sender: sal=%u\n",sal) ;
	            debugprinthexblock("sesnotes_sender: sa=",cols,&sa,sal) ;
	        }
#endif
	        switch (mt) {
	        case sesmsgtype_gen:
	            {
	                SESMSG_GEN	m2 ;
	                const int	nbl = SESMSG_NBUFLEN ;
	                memset(&m2,0,sizeof(SESMSG_GEN)) ;
	                m2.tag = tag ;
	                m2.stime = st ;
	                strwcpy(m2.user,op->unbuf,SESMSG_USERLEN) ;
	                strdcpy1w(m2.nbuf,nbl,sp,sl) ;
	                rs = sesmsg_gen(&m2,0,mbuf,mlen) ;
	                ml = rs ;
	            }
	            break ;
	        case sesmsgtype_biff:
	            {
	                SESMSG_BIFF	m3 ;
	                const int	nbl = SESMSG_NBUFLEN ;
	                memset(&m3,0,sizeof(SESMSG_BIFF)) ;
	                m3.tag = tag ;
	                m3.stime = st ;
	                strwcpy(m3.user,op->unbuf,SESMSG_USERLEN) ;
	                strdcpy1w(m3.nbuf,nbl,sp,sl) ;
	                rs = sesmsg_biff(&m3,0,mbuf,mlen) ;
	                ml = rs ;
	            }
	            break ;
	        default:
	            rs = SR_PROTOTYPE ;
	            break ;
	        } /* end switch */
	        if (rs >= 0) {
	            struct sockaddr	*sap = (struct sockaddr *) &sa ;
	            const int	s = op->fd ;
	            if ((rs = u_sendto(s,mbuf,ml,0,sap,sal)) >= 0) {
#if	CF_DEBUGS
	                debugprintf("sesnotes_sender: u_sendto() PASS rs=%d\n",
	                    rs) ;
#endif
	                f = TRUE ;
	            } else {
#if	CF_DEBUGS
	                debugprintf("sesnotes_sender: u_sendto() FAIL rs=%d\n",
	                    rs) ;
#endif
	                rs = SR_OK ;
	            } /* end if */
	        } /* end if (ok) */
	        sockaddress_finish(&sa) ;
	    } /* end if (sockaddress) */
	} /* end if (sesnotes_ready) */
#if	CF_DEBUGS
	debugprintf("sesnotes_sender: ret rs=%d f=%u\n",rs,f) ;
#endif
	return (rs >= 0) ? f : rs ;
}
/* end subroutine (sesnotes_sender) */


