/* pcsnsc */

/* PCS Name-Server-Client */
/* object to interact with the PCS server */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 2000-12-18, David A­D­ Morano
	This object module was first written.

	= 2011-01-25, David A­D­ Morano
	I added the capability to also send the 'mark', 'report', and 'exit'
	commands to the server.  Previously these were not implemented here.

*/

/* Copyright © 2000,2011 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This module mediates (as a sort of client) the interactions with the PCS
        server.


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<endianstr.h>
#include	<estrings.h>
#include	<vechand.h>
#include	<storebuf.h>
#include	<sockaddress.h>
#include	<envmgr.h>
#include	<spawnproc.h>
#include	<ctdec.h>
#include	<localmisc.h>

#include	"pcsnsc.h"
#include	"pcsmsg.h"
#include	"pcsnsreq.h"


/* local defines */

#define	PCSNSC_VARPR	"PCS"
#define	PCSNSC_FACNAME	"pcs"
#define	PCSNSC_PIDNAME	"pid"
#define	PCSNSC_REQNAME	"req"
#define	PCSNSC_DMODE	0777

#define	VARPREXTRA	"EXTRA" ;
#define	VARPCSQUIET	"PCS_QUIET"
#define	VARPCSPR	"PCS_PROGRAMROOT"

#ifndef	TMPDNAME
#define	TMPDNAME	"/tmp"
#endif

#ifndef	COLUMNS
#define	COLUMNS		80
#endif

#ifndef	MSGBUFLEN
#define	MSGBUFLEN	2048
#endif

#ifndef	DIGBUFLEN
#define	DIGBUFLEN	40		/* can hold int128_t in decimal */
#endif

#define	OPTBUFLEN	(DIGBUFLEN + 4)

#define	TO_UPDATE	60
#define	TO_RUN		(5 * 60)
#define	TO_RECVMSG	5


/* external subroutines */

extern int	snwcpy(char *,int,cchar *,int) ;
extern int	mkfnamesuf1(char *,const char *,const char *) ;
extern int	mkfnamesuf2(char *,const char *,const char *,const char *) ;
extern int	sfbasename(const char *,int,const char **) ;
extern int	nleadstr(const char *,const char *,int) ;
extern int	matstr(const char **,const char *,int) ;
extern int	cfdecti(const char *,int,int *) ;
extern int	cfdecui(const char *,int,uint *) ;
extern int	pathclean(char *,const char *,int) ;
extern int	mkdirs(const char *,mode_t) ;
extern int	opentmpusd(const char *,int,mode_t,char *) ;
extern int	perm(cchar *,uid_t,gid_t,gid_t *,int) ;
extern int	vstrkeycmp(const char **,const char **) ;
extern int	isNotPresent(int) ;
extern int	isNotAccess(int) ;
extern int	isFailConn(int) ;
extern int	isBadSend(int) ;
extern int	isBadRecv(int) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	debugprinthexblock(cchar *,int,const void *,int) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;


/* external variables */


/* local structures */


/* forward references */

static int	pcsnsc_setbegin(PCSNSC *,cchar *) ;
static int	pcsnsc_setend(PCSNSC *) ;
static int	pcsnsc_srvdname(PCSNSC *,char *) ;
static int	pcsnsc_srvfname(PCSNSC *,cchar *) ;
static int	pcsnsc_bind(PCSNSC *,int,cchar *) ;
static int	pcsnsc_bufbegin(PCSNSC *) ;
static int	pcsnsc_bufend(PCSNSC *) ;
static int	pcsnsc_connect(PCSNSC *) ;
static int	pcsnsc_istatus(PCSNSC *,PCSNSC_STATUS *) ;

#ifdef	COMMENT
static int	pcsnsc_spawn(PCSNSC *) ;
static int	pcsnsc_envload(PCSNSC *,ENVMGR *) ;
#endif

static int	mksrvdname(char *,cchar *,cchar *,cchar *) ;


/* local variables */

#ifdef	COMMENT
static const char	*prbins[] = {
	"bin",
	"sbin",
	NULL
} ;
#endif /* COMMENT */


/* exported variables */

PCSNSC_OBJ	pcsnsc = {
	"pcsnsc",
	sizeof(PCSNSC)
} ;


/* exported subroutines */


int pcsnsc_open(PCSNSC *op,cchar *pr,int to)
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;
	if (pr == NULL) return SR_FAULT ;
	if (pr[0] == '\0') return SR_INVALID ;

	if (to < 1) to = 1 ;

#if	CF_DEBUGS
	debugprintf("pcsnsc_open: ent pr=%s to=%d\n",pr,to) ;
#endif

	memset(op,0,sizeof(PCSNSC)) ;
	op->to = to ;
	op->fd = -1 ;
	op->pid = getpid() ;

	if ((rs = pcsnsc_setbegin(op,pr)) > 0) {
	    if ((rs = pcsnsc_connect(op)) > 0) {
	        if ((rs = pcsnsc_bufbegin(op)) >= 0) {
	            op->f.srv = TRUE ;
		    rs = 1 ;
	            op->magic = PCSNSC_MAGIC ;
	        }
	    }
	    if ((rs < 0) || (! op->f.srv)) {
	        pcsnsc_setend(op) ;
	    }
	} /* end if (pcsnsc-set) */

#if	CF_DEBUGS
	debugprintf("pcsnsc_open: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (pcsnsc_open) */


int pcsnsc_close(PCSNSC *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != PCSNSC_MAGIC) return SR_NOTOPEN ;

	rs1 = pcsnsc_bufend(op) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = pcsnsc_setend(op) ;
	if (rs >= 0) rs = rs1 ;

#if	CF_DEBUGS
	debugprintf("pcsnsc_close: ret rs=%d\n",rs) ;
#endif

	op->magic = 0 ;
	return rs ;
}
/* end subroutine (pcsnsc_close) */


int pcsnsc_status(PCSNSC *op,PCSNSC_STATUS *statp)
{
	int		rs ;

#if	CF_DEBUGS
	debugprintf("pcsnsc_status: ent\n") ;
#endif

	if (op == NULL) return SR_FAULT ;

	if (op->magic != PCSNSC_MAGIC) return SR_NOTOPEN ;

#if	CF_DEBUGS
	debugprintf("pcsnsc_status: con\n") ;
#endif

	rs = pcsnsc_istatus(op,statp) ;

#if	CF_DEBUGS
	debugprintf("pcsnsc_status: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (pcsnsc_status) */


int pcsnsc_help(PCSNSC *op,char *rbuf,int rlen,int idx)
{
	int		rs = SR_OK ;
	int		rl = 0 ;

#if	CF_DEBUGS
	debugprintf("pcsnsc_help: ent\n") ;
#endif

	if (op == NULL) return SR_FAULT ;
	if (rbuf == NULL) return SR_FAULT ;

	if (op->magic != PCSNSC_MAGIC) return SR_NOTOPEN ;

	if (op->f.srv) {
	    struct pcsmsg_gethelp	mreq ;
	    struct pcsmsg_help		mres ;
	    const int		to = op->to ;
	    const int		mlen = op->mlen ;
	    char		*mbuf = op->mbuf ;
	    mreq.tag = op->pid ;
	    mreq.idx = (uchar) idx ;
	    if ((rs = pcsmsg_gethelp(&mreq,0,mbuf,mlen)) >= 0) {
	        const int	fd = op->fd ;
	        if ((rs = u_send(fd,mbuf,rs,0)) >= 0) {
	            const int	mf = 0 ;
	            const int	ro = FM_TIMED ;
	            if ((rs = uc_recve(fd,mbuf,mlen,mf,to,ro)) >= 0) {
	                if ((rs = pcsmsg_help(&mres,1,mbuf,rs)) >= 0) {
	                    if (mres.rc == pcsmsgrc_ok) {
	                        cchar	*rp = mres.val ;
	                        rl = mres.vl ;
	                        rs = snwcpy(rbuf,rlen,rp,rl) ;
	                    } else if (mres.rc == pcsmsgrc_notfound) {
	                        rl = 0 ;
	                    } else {
	                        rs = SR_BADMSG ;
	                    }
	                }
		    } else if (isBadRecv(rs)) {
		        op->f.srv = FALSE ;
		        rs = SR_OK ;
	            } /* end if (uc_recve) */
		} else if (isBadSend(rs)) {
		    op->f.srv = FALSE ;
		    rs = SR_OK ;
	        } /* end if (u_send) */
	    } /* end if (pcsmsg_gethelp) */
	    if (rs < 0) op->f.srv = FALSE ;
	} /* end if (servicing) */

#if	CF_DEBUGS
	debugprintf("pcsnsc_help: ret rs=%d rl=%u\n",rs,rl) ;
#endif

	return (rs >= 0) ? rl : rs ;
}
/* end subroutine (pcsnsc_help) */


int pcsnsc_getval(PCSNSC *op,char *rbuf,int rlen,cchar *un,int w)
{
	int		rs = SR_OK ;
	int		rl = 0 ;

#if	CF_DEBUGS
	debugprintf("pcsnsc_getval: ent un=%s w=%u\n",un,w) ;
#endif

	if (op == NULL) return SR_FAULT ;
	if (rbuf == NULL) return SR_FAULT ;
	if (un == NULL) return SR_FAULT ;

	if (un[0] == '\0') return SR_INVALID ;

	if (op->magic != PCSNSC_MAGIC) return SR_NOTOPEN ;

	if (op->f.srv) {
	    struct pcsmsg_getval	mreq ;
	    struct pcsmsg_val		mres ;
	    const int		to = op->to ;
	    const int		mlen = op->mlen ;
	    char		*mbuf = op->mbuf ;
	    mreq.tag = op->pid ;
	    mreq.w = (uchar) w ;
	    strwcpy(mreq.key,un,PCSMSG_KEYLEN) ;
	    if ((rs = pcsmsg_getval(&mreq,0,mbuf,mlen)) >= 0) {
	        const int	fd = op->fd ;
#if	CF_DEBUGS
	debugprintf("pcsnsc_getval: fd=%d\n",fd) ;
#endif
	        if ((rs = u_send(fd,mbuf,rs,0)) >= 0) {
	            const int	mf = 0 ;
	            const int	ro = FM_TIMED ;
	            if ((rs = uc_recve(fd,mbuf,mlen,mf,to,ro)) >= 0) {
	                if ((rs = pcsmsg_val(&mres,1,mbuf,rs)) >= 0) {
	                    if (mres.rc == pcsmsgrc_ok) {
	                        cchar	*rp = mres.val ;
	                        rl = mres.vl ;
	                        rs = snwcpy(rbuf,rlen,rp,rl) ;
	                    } else if (mres.rc == pcsmsgrc_notfound) {
	                        rl = 0 ;
	                    } else {
	                        rs = SR_BADMSG ;
	                    }
	                }
		    } else if (isBadRecv(rs)) {
#if	CF_DEBUGS
	debugprintf("pcsnsc_getval: uc_recve() out rs=%d\n",rs) ;
#endif
		        op->f.srv = FALSE ;
		        rs = SR_OK ;
	            } /* end if (uc_recve) */
		} else if (isBadSend(rs)) {
#if	CF_DEBUGS
	debugprintf("pcsnsc_getval: u_send() out rs=%d\n",rs) ;
#endif
		    op->f.srv = FALSE ;
		    rs = SR_OK ;
	        } /* end if (u_send) */
	    } /* end if (pcsmsg_getval) */
#if	CF_DEBUGS
	debugprintf("pcsnsc_getval: pcsmsg_getval() out rs=%d\n",rs) ;
#endif
	    if (rs < 0) op->f.srv = FALSE ;
	} /* end if (servicing) */

#if	CF_DEBUGS
	debugprintf("pcsnsc_getval: ret rs=%d rl=%u\n",rs,rl) ;
#endif

	return (rs >= 0) ? rl : rs ;
}
/* end subroutine (pcsnsc_getval) */


int pcsnsc_mark(PCSNSC *op)
{
	int		rs = SR_OK ;
	int		rl = 0 ;

#if	CF_DEBUGS
	debugprintf("pcsnsc_mark: ent\n") ;
#endif

	if (op == NULL) return SR_FAULT ;

	if (op->magic != PCSNSC_MAGIC) return SR_NOTOPEN ;

	if (op->f.srv) {
	    struct pcsmsg_mark	mreq ;
	    struct pcsmsg_ack	mres ;
	    const int		to = op->to ;
	    const int		mlen = op->mlen ;
	    char		*mbuf = op->mbuf ;
	    mreq.tag = op->pid ;
	    if ((rs = pcsmsg_mark(&mreq,0,mbuf,mlen)) >= 0) {
	        const int	fd = op->fd ;
	        if ((rs = u_send(fd,mbuf,rs,0)) >= 0) {
	            const int	mf = 0 ;
	            const int	ro = FM_TIMED ;
	            if ((rs = uc_recve(fd,mbuf,mlen,mf,to,ro)) >= 0) {
	                if ((rs = pcsmsg_ack(&mres,1,mbuf,rs)) >= 0) {
	                    if (mres.rc == pcsmsgrc_ok) {
	                        rl = 1 ;
	                    } else {
	                        rs = SR_BADMSG ;
	                    }
	                }
		    } else if (isBadRecv(rs)) {
		        op->f.srv = FALSE ;
		        rs = SR_OK ;
	            } /* end if (uc_recve) */
		} else if (isBadSend(rs)) {
		    op->f.srv = FALSE ;
		    rs = SR_OK ;
	        } /* end if (u_send) */
	    } /* end if (pcsmsg_mark) */
	    if (rs < 0) op->f.srv = FALSE ;
	} /* end if (servicing) */

#if	CF_DEBUGS
	debugprintf("pcsnsc_mark: ret rs=%d\n",rs) ;
#endif

	return (rs >= 0) ? rl : rs ;
}
/* end subroutine (pcsnsc_mark) */


int pcsnsc_exit(PCSNSC *op,cchar *reason)
{
	int		rs = SR_OK ;
	int		rl = 0 ;

#if	CF_DEBUGS
	debugprintf("pcsnsc_exit: ent\n") ;
#endif

	if (op == NULL) return SR_FAULT ;

	if (op->magic != PCSNSC_MAGIC) return SR_NOTOPEN ;

	if (op->f.srv) {
	    struct pcsmsg_exit	mreq ;
	    struct pcsmsg_ack	mres ;
	    const int		to = op->to ;
	    const int		mlen = op->mlen ;
	    char		*mbuf = op->mbuf ;
	    mreq.tag = op->pid ;
	    mreq.reason[0] = '\0' ;
	    if (reason != NULL) {
	        strwcpy(mreq.reason,reason,REALNAMELEN) ;
	    }
	    if ((rs = pcsmsg_exit(&mreq,0,mbuf,mlen)) >= 0) {
	        const int	fd = op->fd ;
	        if ((rs = u_send(fd,mbuf,rs,0)) >= 0) {
	            const int	mf = 0 ;
	            const int	ro = FM_TIMED ;
	            if ((rs = uc_recve(fd,mbuf,mlen,mf,to,ro)) >= 0) {
	                if ((rs = pcsmsg_ack(&mres,1,mbuf,rs)) >= 0) {
	                    if (mres.rc == pcsmsgrc_ok) {
	                        rl = 1 ;
	                    } else {
	                        rs = SR_BADMSG ;
	                    }
	                }
		    } else if (isBadRecv(rs)) {
		        op->f.srv = FALSE ;
		        rs = SR_OK ;
	            } /* end if (uc_recve) */
		} else if (isBadSend(rs)) {
		    op->f.srv = FALSE ;
		    rs = SR_OK ;
	        } /* end if (u_send) */
	    } /* end if (pcsmsg_exit) */
	    if (rs < 0) op->f.srv = FALSE ;
	} /* end if (servicing) */

#if	CF_DEBUGS
	debugprintf("pcsnsc_exit: ret rs=%d\n",rs) ;
#endif

	return (rs >= 0) ? rl : rs ;
}
/* end subroutine (pcsnsc_exit) */


int pcsnsc_getname(PCSNSC *op,char *rbuf,int rlen,cchar *un)
{
	const int	w = pcsnsreq_pcsname ;
	return pcsnsc_getval(op,rbuf,rlen,un,w) ;
}
/* end subroutine (pcsnsc_getname) */


/* local subroutines */


static int pcsnsc_setbegin(PCSNSC *op,cchar *pr)
{
	int		rs ;
	int		f = FALSE ;
	cchar		*cp ;

#if	CF_DEBUGS
	debugprintf("pcsnsc_setbegin: ent\n") ;
#endif

	if ((rs = uc_mallocstrw(pr,-1,&cp)) >= 0) {
	    char	rbuf[MAXPATHLEN+1] ;
	    op->pr = cp ;
	    if ((rs = pcsnsc_srvdname(op,rbuf)) > 0) {
	        if ((rs = pcsnsc_srvfname(op,rbuf)) > 0) {
	            if ((rs = pcsnsc_bind(op,TRUE,rbuf)) >= 0) {
	                f = TRUE ;
	            }
	            if ((rs < 0) && (op->srvfname != NULL)) {
	                uc_free(op->srvfname) ;
	                op->srvfname = NULL ;
	            }
	        } /* end if (pcsnsc_srvfname) */
	    } /* end if (pcsnsc_srvdname) */
	    if (((rs < 0) || (!f)) && (op->pr != NULL)) {
	        uc_free(op->pr) ;
	        op->pr = NULL ;
	    }
	} /* end if (m-a) */

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (pcsnsc_setbegin) */


static int pcsnsc_setend(PCSNSC *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	rs1 = pcsnsc_bufend(op) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = pcsnsc_bind(op,FALSE,NULL) ;
	if (rs >= 0) rs = rs1 ;

	if (op->srvfname != NULL) {
	    rs1 = uc_free(op->srvfname) ;
	    if (rs >= 0) rs = rs1 ;
	    op->srvfname = NULL ;
	}

	if (op->pr != NULL) {
	    rs1 = uc_free(op->pr) ;
	    if (rs >= 0) rs = rs1 ;
	    op->pr = NULL ;
	}

	return rs ;
}
/* end subroutine (pcsnsc_setend) */


static int pcsnsc_srvdname(PCSNSC *op,char *rbuf)
{
	int		rs ;
	int		rl = 0 ;
	cchar		*td = TMPDNAME ;
	cchar		*fn = PCSNSC_FACNAME ;

#if	CF_DEBUGS
	debugprintf("pcsnsc_srvdname: ent\n") ;
#endif

	if ((rs = mksrvdname(rbuf,td,op->pr,fn)) >= 0) {
	    USTAT	sb ;
	    if ((rs = u_stat(rbuf,&sb)) >= 0) {
	        if (S_ISDIR(sb.st_mode)) {
	            const int	am = (W_OK|W_OK|X_OK) ;
	            if ((rs = perm(rbuf,-1,-1,NULL,am)) >= 0) {
	                rl = 1 ;
	            } else if (isNotAccess(rs)) {
	                rs = SR_OK ;
	            }
	        }
	    } else if (isNotPresent(rs)) {
	        rs = SR_OK ;
	    }
	}

#if	CF_DEBUGS
	debugprintf("pcsnsc_srvdname: ret rs=%d rl=%u\n",rs,rl) ;
#endif

	return (rs >= 0) ? rl : rs ;
}
/* end subroutine (pcsnsc_srvdname) */


static int pcsnsc_srvfname(PCSNSC *op,cchar *srvdname)
{
	int		rs ;
	int		rl = 0 ;
	cchar		*reqname = PCSNSC_REQNAME ;
	char		srvfname[MAXPATHLEN + 1] ;

#if	CF_DEBUGS
	debugprintf("pcsnsc_srvfname: ent\n") ;
#endif

	if ((rs = mkpath2(srvfname,srvdname,reqname)) >= 0) {
	    USTAT	sb ;
	    rl = rs ;
#if	CF_DEBUGS
	debugprintf("pcsnsc_srvfname: srvfname=%s\n",srvfname) ;
#endif
	    if ((rs = u_stat(srvfname,&sb)) >= 0) {
	        if (S_ISSOCK(sb.st_mode)) {
	            const int	am = (R_OK|W_OK) ;
	            if ((rs = perm(srvfname,-1,-1,NULL,am)) >= 0) {
	                cchar	*cp ;
	                if ((rs = uc_mallocstrw(srvfname,rl,&cp)) >= 0) {
	                    op->srvfname = cp ;
	                }
	            } else if (isNotAccess(rs)) {
	                rl = 0 ;
	                rs = SR_OK ;
	            }
	        } else {
	            rl = 0 ;
	        }
	    } else if (isNotPresent(rs)) {
#if	CF_DEBUGS
	debugprintf("pcsnsc_srvfname: not-present rs=%d\n",rs) ;
#endif
	        rl = 0 ;
	        rs = SR_OK ;
	    }
	} /* end if (srvfname) */

#if	CF_DEBUGS
	debugprintf("pcsnsc_srvfname: ret rs=%d rl=%u\n",rs,rl) ;
#endif

	return (rs >= 0) ? rl : rs ;
}
/* end subroutine (pcsnsc_srvfname) */


static int pcsnsc_bind(PCSNSC *op,int f,cchar *srvdname)
{
	int		rs = SR_OK ;
	int		f_err = FALSE ;

#if	CF_DEBUGS
	debugprintf("pcsnsc_bind: ent f=%u\n",f) ;
#endif

	if (f) {
	    cchar	*tn = "clientXXXXXXXX" ;
	    char	template[MAXPATHLEN + 1] ;

	    if ((rs = mkpath2(template,srvdname,tn)) >= 0) {
	        const mode_t	om = (S_IFSOCK | 0666) ;
	        const int	of = (O_RDWR | O_CLOEXEC | O_MINMODE) ;
	        char		fname[MAXPATHLEN + 1] ;
	        if ((rs = opentmpusd(template,of,om,fname)) >= 0) {
	            cchar	*cp ;
#if	CF_DEBUGS
			debugprintf("pcsnsc_bind: opentmpusd() rs=%d\n",rs) ;
#endif
	            op->fd = rs ;
	            u_chmod(fname,om) ;
	            uc_closeonexec(op->fd,TRUE) ;
	            if ((rs = uc_mallocstrw(fname,-1,&cp)) >= 0) {
	                op->srcfname = cp ;
	            } else {
	                f_err = TRUE ;
	            }
	        } /* end if (opentmpusd) */
	    } /* end if (mkpath) */

	} /* end if (bind-on) */

	if ((! f) || f_err) {

	    if (op->fd >= 0) {
	        u_close(op->fd) ;
	        op->fd = -1 ;
	    }
	    if (op->srcfname != NULL) {
	        if (op->srcfname[0] != '\0') {
	            u_unlink(op->srcfname) ;
	        }
	        uc_free(op->srcfname) ;
	        op->srcfname = NULL ;
	    }

	} /* end if (bind-off) */

#if	CF_DEBUGS
	debugprintf("pcsnsc_bind: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (pcsnsc_bind) */


static int pcsnsc_bufbegin(PCSNSC *op)
{
	const int	blen = MSGBUFLEN ;
	int		rs ;
	char		*bp ;
	if ((rs = uc_malloc((blen+1),&bp)) >= 0) {
	    op->mbuf = bp ;
	    op->mlen = blen ;
	}
	return rs ;
}
/* end subroutine (pcsnsc_bufbegin) */


static int pcsnsc_bufend(PCSNSC *op)
{
	int		rs = SR_OK ;
	int		rs1 ;
	if (op->mbuf != NULL) {
	    rs1 = uc_free(op->mbuf) ;
	    if (rs >= 0) rs = rs1 ;
	    op->mbuf = NULL ;
	    op->mlen = 0 ;
	}
	return rs ;
}
/* end subroutine (pcsnsc_bufend) */


static int pcsnsc_connect(PCSNSC *op)
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		f = FALSE ;

#if	CF_DEBUGS
	debugprintf("pcsnsc_connect: ent srvfname=%s\n",op->srvfname) ;
#endif

	if (op->srvfname != NULL) {
	    SOCKADDRESS	sa ;
	    const int	af = AF_UNIX ;
	    cchar	*sfn = op->srvfname ;
	    if ((rs = sockaddress_start(&sa,af,sfn,0,0)) >= 0) {
	        SOCKADDR	*sap = (SOCKADDR *) &sa ;
	        const int	sal = rs ;
	        const int	to = op->to ;
	        if ((rs = uc_connecte(op->fd,sap,sal,to)) >= 0) {
	            f = TRUE ;
	        } else if (isFailConn(rs)) {
	            rs = SR_OK ;
	        }
	        rs1 = sockaddress_finish(&sa) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (sockaddress) */
	} /* end if (non-null) */

#if	CF_DEBUGS
	debugprintf("pcsnsc_connect: ret rs=%d f=%u\n",rs,f) ;
#endif

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (pcsnsc_connect) */


static int pcsnsc_istatus(PCSNSC *op,PCSNSC_STATUS *statp)
{
	int		rs = SR_OK ;
	int		rc = 0 ;

#if	CF_DEBUGS
	debugprintf("pcsnsc_istatus: ent\n") ;
#endif

	if (statp != NULL) {
	    memset(statp,0,sizeof(PCSNSC_STATUS)) ;
	}

	if (op->f.srv) {
	    struct pcsmsg_getstatus	mreq ;
	    struct pcsmsg_status	mres ;
	    const int		to = op->to ;
	    const int		mlen = op->mlen ;
	    char		*mbuf = op->mbuf ;
	    mreq.tag = op->pid ;
	    if ((rs = pcsmsg_getstatus(&mreq,0,mbuf,mlen)) >= 0) {
	        const int	fd = op->fd ;
	        if ((rs = u_send(fd,mbuf,rs,0)) >= 0) {
	            const int	mf = 0 ;
	            const int	ro = FM_TIMED ;
	            if ((rs = uc_recve(fd,mbuf,mlen,mf,to,ro)) >= 0) {
	                if ((rs = pcsmsg_status(&mres,1,mbuf,rs)) >= 0) {
#if	CF_DEBUGS
	                    debugprintf("pcsnsc_istatus: "
	                        "pcsmsg_status() rs=%d rc=%u\n",rs,mres.rc) ;
#endif
	                    if (mres.rc == pcsmsgrc_ok) {
				rc = 1 ;
				if (statp != NULL) {
	                            statp->pid = mres.pid ;
				    statp->queries = mres.queries ;
				}
	                    } else if (mres.rc == pcsmsgrc_notavail) {
	                        rc = 0 ;
	                    } else {
	                        rs = SR_BADMSG ;
	                    }
	                }
		    } else if (isBadRecv(rs)) {
		        op->f.srv = FALSE ;
		        rs = SR_OK ;
	            } /* end if (uc_recve) */
#if	CF_DEBUGS
		debugprintf("pcsnsc_istatus: recv-out rs=%d\n",rs) ;
#endif
		} else if (isBadSend(rs)) {
		    op->f.srv = FALSE ;
		    rs = SR_OK ;
	        } /* end if (u_send) */
#if	CF_DEBUGS
		debugprintf("pcsnsc_istatus: send-out rs=%d\n",rs) ;
#endif
	    } /* end if (pcsmsg_getstatus) */
	    if (rs < 0) op->f.srv = FALSE ;
	} /* end if (servicing) */

#if	CF_DEBUGS
	debugprintf("pcsnsc_istatus: ret rs=%d pid=%d\n",rs,rc) ;
#endif

	return (rs >= 0) ? rc : rs ;
}
/* end subroutine (pcsnsc_istatus) */


#ifdef	COMMENT

static int pcsnsc_spawn(PCSNSC *op)
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		cs ;
	int		i ;
	int		to_run = TO_RUN ;
	const char	*argz = PCSNSC_FACNAME ;
	char		pbuf[MAXPATHLEN + 1] ;

	for (i = 0 ; (rs >= 0) && (prbins[i] != NULL) ; i += 1) {
	    if ((rs = mkpath3(pbuf,op->pr,prbins[i],argz)) >= 0) {
	        rs = perm(pbuf,-1,-1,NULL,X_OK) ;
	    }
	} /* end for */

	if (rs >= 0) {
	    ENVMGR	em ;
	    if ((rs = envmgr_start(&em)) >= 0) {
	        if ((rs = pcsnsc_envload(op,&em)) >= 0) {
		    const int	dlen = DIGBUFLEN ;
	            char	dbuf[DIGBUFLEN + 1] ;
	            if ((rs = ctdeci(dbuf,dlen,to_run)) >= 0) {
	                char	optbuf[OPTBUFLEN + 1] ;
	                if ((rs = sncpy2(optbuf,OPTBUFLEN,"-d=",dbuf)) >= 0) {
	                    int		i = 0 ;
	                    cchar	*av[6] ;
	                    cchar	**ev ;
	                    av[i++] = argz ;
	                    av[i++] = optbuf ;
	                    av[i++] = "-o" ;
	                    av[i++] = "quick" ;
	                    av[i++] = NULL ;
	                    if ((rs = envmgr_getvec(&em,&ev)) >= 0) {
	                        SPAWNPROC	ps ;
	                        memset(&ps,0,sizeof(SPAWNPROC)) ;
				ps.opts = SPAWNPROC_OSETSID ;
	                        ps.disp[0] = SPAWNPROC_DCLOSE ;
	                        ps.disp[1] = SPAWNPROC_DCLOSE ;
	                        ps.disp[2] = SPAWNPROC_DCLOSE ;
	                        if ((rs = spawnproc(&ps,pbuf,av,ev)) >= 0) {
	                            const pid_t	pid = rs ;
	                            u_waitpid(pid,&cs,0) ;
	                        } /* end if */
	                    } /* end if (envmgr_getvec) */
	                } /* end if (sncpy) */
	            } /* end if (ctdeci) */
	        } /* end if (pcsnsc_envload) */
	        rs1 = envmgr_finish(&em) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (envmgr) */
	} /* end if (ok) */

	return rs ;
}
/* end subroutine (pcsnsc_spawn) */


static int pcsnsc_envload(PCSNSC *op,ENVMGR *emp)
{
	int		rs ;
	if ((rs = envmgr_set(emp,VARPCSQUIET,"1",1)) >= 0) {
	    rs = envmgr_set(emp,VARPCSPR,op->pr,-1) ;
	}
	return rs ;
}
/* end subroutine (pcsnsc_envload) */

#endif /* COMMENT */


static int mksrvdname(char *rbuf,cchar *td,cchar *pr,cchar *fn)
{
	const int	rlen = MAXPATHLEN ;
	int		rs = SR_OK ;
	int		i = 0 ;

	if (rs >= 0) {
	    rs = storebuf_strw(rbuf,rlen,i,td,-1) ;
	    i += rs ;
	}

	if (rs >= 0) {
	    rs = storebuf_char(rbuf,rlen,i,'/') ;
	    i += rs ;
	}

	{
	    int		cl ;
	    cchar	*cp ;
	    if ((cl = sfbasename(pr,-1,&cp)) > 0) {
	        rs = storebuf_strw(rbuf,rlen,i,cp,cl) ;
	        i += rs ;
	    }
	}

	if (rs >= 0) {
	    rs = storebuf_char(rbuf,rlen,i,'/') ;
	    i += rs ;
	}

	if (rs >= 0) {
	    rs = storebuf_strw(rbuf,rlen,i,fn,-1) ;
	    i += rs ;
	}

	return (rs >= 0) ? i : rs ;
}
/* end subroutine (mksrvdname) */


