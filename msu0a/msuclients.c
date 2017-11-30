/* msuclients */

/* object to interact with the MSU server */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-12-18, David A­D­ Morano
	This object module was first written.

	= 2011-01-25, David A­D­ Morano
	I added the capability to also send the 'mark', 'report', and 'exit'
	commands to the server.  Previously these were not implemented here.

*/

/* Copyright © 1998,2011 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This module mediates the interactions with the MSU server.


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
#include	<vecstr.h>
#include	<vechand.h>
#include	<storebuf.h>
#include	<sockaddress.h>
#include	<spawnproc.h>
#include	<ctdec.h>
#include	<localmisc.h>

#include	"msuclients.h"
#include	"msumsg.h"


/* local defines */

#define	MSUCLIENTS_VARPR	"MSU"
#define	MSUCLIENTS_FACNAME	"msu"
#define	MSUCLIENTS_REQNAME	"req"
#define	MSUCLIENTS_DMODE	0777

#define	VARPREXTRA		"EXTRA" ;
#define	VARMSUQUIET		"MSU_QUIET"
#define	VARMSUPR		"MSU_PROGRAMROOT"

#ifndef	TMPDNAME
#define	TMPDNAME		"/tmp"
#endif

#define	ENVMGR		struct envmgr

#define	IPCMSGINFO	struct ipcmsginfo

#ifndef	IPCBUFLEN
#define	IPCBUFLEN	MSGBUFLEN
#endif

#ifndef	CMSGBUFLEN
#define	CMSGBUFLEN	256
#endif

#ifndef	COLUMNS
#define	COLUMNS		80
#endif

#ifndef	DIGBUFLEN
#define	DIGBUFLEN	40		/* can hold int128_t in decimal */
#endif

#define	OPTBUFLEN	(DIGBUFLEN + 4)

#define	NIOVECS		1

#define	TO_UPDATE	60
#define	TO_RUN		(5 * 60)
#define	TO_RECVMSG	5


/* external subroutines */

extern int	mkfnamesuf1(char *,const char *,const char *) ;
extern int	mkfnamesuf2(char *,const char *,const char *,const char *) ;
extern int	sfbasename(const char *,int,const char **) ;
extern int	nleadstr(const char *,const char *,int) ;
extern int	matstr(const char **,const char *,int) ;
extern int	cfdecti(const char *,int,int *) ;
extern int	cfdecui(const char *,int,uint *) ;
extern int	getnodedomain(char *,char *) ;
extern int	mkpr(char *,int,const char *,const char *) ;
extern int	pathclean(char *,const char *,int) ;
extern int	mkdirs(const char *,mode_t) ;
extern int	opentmpusd(const char *,int,mode_t,char *) ;
extern int	perm(const char *,uid_t,gid_t,gid_t *,int) ;
extern int	vecstr_envset(vecstr *,const char *,const char *,int) ;
extern int	vstrkeycmp(const char **,const char **) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;


/* external variables */

static char	**environ ;


/* local structures */

union conmsg {
	struct cmsghdr	cm ;
	char		cmbuf[CMSGBUFLEN + 1] ;
} ;

struct ipcmsginfo {
	struct msghdr	ipcmsg ;
	struct iovec	vecs[NIOVECS] ;
	union conmsg	ipcconbuf ;
	SOCKADDRESS	ipcfrom ;
	int		ipcmsglen ;
	int		ns ;
	char		ipcbuf[IPCBUFLEN + 1] ;
} ;

struct envmgr {
	VECSTR		envstrs ;
	VECHAND		envlist ;
} ;


/* forward references */

static int	msuclients_setbegin(MSUCLIENTS *,const char *,const char *) ;
static int	msuclients_setend(MSUCLIENTS *) ;
static int	msuclients_pr(MSUCLIENTS *,const char *) ;
static int	msuclients_reqfname(MSUCLIENTS *,const char *) ;
static int	msuclients_tmpourdname(MSUCLIENTS *) ;
static int	msuclients_bind(MSUCLIENTS *,int) ;
static int	msuclients_connect(MSUCLIENTS *) ;
static int	msuclients_disconnect(MSUCLIENTS *) ;
static int	msuclients_istatus(MSUCLIENTS *) ;
static int	msuclients_spawn(MSUCLIENTS *) ;

static int	envmgr_start(ENVMGR *) ;
static int	envmgr_set(ENVMGR *,const char *,const char *,int) ;
static int	envmgr_getvec(ENVMGR *,const char ***) ;
static int	envmgr_finish(ENVMGR *) ;

static int	ipcmsginfo_init(struct ipcmsginfo *,struct sockaddr *,int) ;

#ifdef	COMMENT
static int	venvcmp(const void **,const void **) ;
#endif


/* local variables */

static const char	*prbins[] = {
	"bin",
	"sbin",
} ;


/* exported variables */

MSUCLIENTS_OBJ	msuclients = {
	"msuclients",
	sizeof(MSUCLIENTS)
} ;


/* exported subroutines */


int msuclients_open(op,pr,reqfname,to)
MSUCLIENTS	*op ;
const char	*pr ;
const char	*reqfname ;
int		to ;
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;

	if (to < 1) to = 1 ;

	memset(op,0,sizeof(MSUCLIENTS)) ;
	op->to = to ;

	if ((rs = msuclients_setbegin(op,pr,reqfname)) >= 0) {
	    if ((rs = msuclients_connect(op)) >= 0) {
		op->magic = MSUCLIENTS_MAGIC ;
	    }
	    if (rs < 0)
		msuclients_setend(op) ;
	}

#if	CF_DEBUGS
	debugprintf("msuclients_open: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (msuclients_open) */


int msuclients_close(op)
MSUCLIENTS	*op ;
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != MSUCLIENTS_MAGIC) return SR_NOTOPEN ;

	rs1 = msuclients_disconnect(op) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = msuclients_setend(op) ;
	if (rs >= 0) rs = rs1 ;

#if	CF_DEBUGS
	debugprintf("msuclients_close: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (msuclients_close) */


int msuclients_status(op)
MSUCLIENTS	*op ;
{
	int		rs = SR_OK ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != MSUCLIENTS_MAGIC) return SR_NOTOPEN ;

	rs = msuclients_istatus(op) ;

	return rs ;
}
/* end subroutine (msuclients_status) */


int msuclients_get(MSUCLIENTS *op,time_t dt,MSUCLIENTS_DATA *dp)
{
	int		rs = SR_OK ;
	int		n = 0 ;

	if (op == NULL) return SR_FAULT ;
	if (dp == NULL) return SR_FAULT ;

	if (op->magic != MSUCLIENTS_MAGIC) return SR_NOTOPEN ;

#if	CF_DEBUGS
	debugprintf("msuclients_get: ent\n") ;
#endif

	if (dt == 0) dt = time(NULL) ;

	if (dt > op->dt) op->dt = dt ;

	memset(dp,0,sizeof(MSUCLIENTS_DATA)) ;

#if	CF_DEBUGS
	debugprintf("msuclients_get: ret rs=%d n=%u\n",rs,n) ;
#endif

	return (rs >= 0) ? n : rs ;
}
/* end subroutine (msuclients_get) */


/* local subroutines */


static int msuclients_setbegin(MSUCLIENTS *op,cchar *pr,cchar *reqfname)
{
	int		rs ;

#if	CF_DEBUGS
	debugprintf("msuclients_setbegin: ent\n") ;
#endif

	op->dt = time(NULL) ;

	rs = msuclients_pr(op,pr) ;

	if (rs >= 0)
	    rs = msuclients_tmpourdname(op) ;

	if (rs >= 0)
	    rs = msuclients_bind(op,TRUE) ;

	if (rs >= 0)
	    rs = msuclients_reqfname(op,reqfname) ;

	return rs ;
}
/* end subroutine (msuclients_setbegin) */


static int msuclients_setend(MSUCLIENTS *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	rs1 = msuclients_bind(op,FALSE) ;
	if (rs >= 0) rs = rs1 ;

	if (op->reqfname != NULL) {
	    rs1 = uc_free(op->reqfname) ;
	    if (rs >= 0) rs = rs1 ;
	    op->reqfname = NULL ;
	}

	if (op->pr != NULL) {
	    rs1 = uc_free(op->pr) ;
	    if (rs >= 0) rs = rs1 ;
	    op->pr= NULL ;
	}

	op->pr = NULL ;
	return rs ;
}
/* end subroutine (msuclients_setend) */


static int msuclients_pr(MSUCLIENTS *op,cchar *pr)
{
	const int	prlen = MAXPATHLEN ;
	int		rs = SR_OK ;
	int		prl = -1 ;
	char		prbuf[MAXPATHLEN + 1] ;

	if (pr != NULL) {
	    char	dbuf[MAXHOSTNAMELEN + 1] ;
	    pr = prbuf ;
	    if ((rs = getnodedomain(NULL,dbuf)) >= 0) {
	        cchar	*varpr = MSUCLIENTS_VARPR ;
	        rs = mkpr(prbuf,prlen,varpr,dbuf) ;
	        prl = rs ;
	    }
	} /* end if */

	if (rs >= 0) {
	    cchar	*cp ;
	    if ((rs = uc_mallocstrw(pr,prl,&cp)) >= 0) {
	        op->pr = cp ;
	    }
	}

#if	CF_DEBUGS
	debugprintf("msuclients_pr: rs=%d pr=%s\n",rs,op->pr) ;
#endif

	return rs ;
}
/* end subroutine (msuclients_pr) */


static int msuclients_reqfname(MSUCLIENTS *op,cchar *reqfname)
{
	int		rs = SR_OK ;
	int		pl = -1 ;
	const char	*tmpdname = TMPDNAME ;
	const char	*facname = MSUCLIENTS_FACNAME ;
	const char	*reqname = MSUCLIENTS_REQNAME ;
	cchar		*cp ;
	char		tmpfname[MAXPATHLEN + 1] ;

	if (op->reqfname == NULL) {
	    int		rl ;
	    char	rootdname[MAXPATHLEN + 1] ;
	    cchar	*rp ;

	    rl = sfbasename(op->pr,-1,&rp) ;
	    if (rl <= 0)
	        rs = SR_NOTDIR ;

	    if (rs >= 0)
	        rs = mkpath2w(rootdname,tmpdname,rp,rl) ;

	    if (rs >= 0) {
	        rs = mkpath3(tmpfname,rootdname,facname,reqname) ;
		pl = rs ;
	    }

	    if (rs >= 0)
		reqfname = tmpfname ;

	} /* end if (reqfname) */

	if ((rs >= 0) && (reqfname != NULL)) {
	    if (reqfname[0] != '\0') {
	        if ((rs = uc_mallocstrw(reqfname,pl,&cp)) >= 0) {
	            op->reqfname = cp ;
		}
	    } else {
		rs = SR_INVALID ;
	    }
	}

	return rs ;
}
/* end subroutine (msuclients_reqfname) */


static int msuclients_tmpourdname(MSUCLIENTS *op)
{
	const int	pathlen = MAXPATHLEN ;
	int		rs = SR_OK ;
	int		cl ;
	int		i = 0 ;
	const char	*tmpdname = TMPDNAME ;
	const char	*facname = MSUCLIENTS_FACNAME ;
	cchar		*cp ;
	char		tmpourdname[MAXPATHLEN + 1] ;

#if	CF_DEBUGS
	debugprintf("msuclients_tmpourdname: tmpourdname=%s\n",
		op->tmpourdname) ;
#endif

	if (op->tmpourdname != NULL) {
	    i = strlen(op->tmpourdname) ;
	    goto ret0 ;
	}

	cl = sfbasename(op->pr,-1,&cp) ;

	if (rs >= 0) {
	    rs = storebuf_strw(tmpourdname,pathlen,i,tmpdname,-1) ;
	    i += rs ;
	}

	if (rs >= 0) {
	    rs = storebuf_char(tmpourdname,pathlen,i,'/') ;
	    i += rs ;
	}

	if (rs >= 0) {
	    rs = storebuf_strw(tmpourdname,pathlen,i,cp,cl) ;
	    i += rs ;
	}

	if (rs >= 0) {
	    rs = storebuf_char(tmpourdname,pathlen,i,'/') ;
	    i += rs ;
	}

	if (rs >= 0) {
	    rs = storebuf_strw(tmpourdname,pathlen,i,facname,-1) ;
	    i += rs ;
	}

	if (rs >= 0) {
	    mode_t	dmode = MSUCLIENTS_DMODE ;
	    if ((rs = mkdirs(tmpourdname,dmode)) >= 0) {
	        rs = uc_minmod(tmpourdname,dmode) ;
	    }
	}

	if (rs >= 0) {
	    if ((rs = uc_mallocstrw(tmpourdname,i,&cp)) >= 0) {
		op->tmpourdname = cp ;
	    }
	}

ret0:

#if	CF_DEBUGS
	debugprintf("msuclients_tmpourdname: ret rs=%d i=%u\n",rs,i) ;
	debugprintf("msuclients_tmpourdname: tmpourdname=%s\n",
		op->tmpourdname) ;
#endif

	return (rs >= 0) ? i : rs ;
}
/* end subroutine (msuclients_tmpourdname) */


static int msuclients_bind(MSUCLIENTS *op,int f)
{
	int		rs = SR_OK ;
	int		f_err = FALSE ;

#if	CF_DEBUGS
	debugprintf("msuclients_bind: f=%u\n",f) ;
#endif

	if (f) {
	    const mode_t	operms = (S_IFSOCK | 0666) ;
	    const int		oflags = O_RDWR ;
	    char		template[MAXPATHLEN + 1] ;
	    char		fname[MAXPATHLEN + 1] ;

	    rs = mkpath2(template,op->tmpourdname,"clientXXXXXXXX") ;
	    if (rs >= 0) {
	        rs = opentmpusd(template,oflags,operms,fname) ;
	        op->fd = rs ;

#if	CF_DEBUGS
	debugprintf("msuclients_bind: template=%s\n",template) ;
	debugprintf("msuclients_bind: opentmpusd() rs=%d\n",rs) ;
#endif

		if (rs >= 0) {
	    	    cchar	*cp ;
	    	    u_chmod(fname,operms) ;
	            uc_closeonexec(op->fd,TRUE) ;
		    if ((rs = uc_mallocstrw(fname,-1,&cp)) >= 0) {
		        op->clientfname = cp ;
		    }
		    else f_err = TRUE ;
		}
	    }

	} /* end if (bind-on) */

	if ((! f) || f_err) {

	    if (op->fd >= 0) {
		u_close(op->fd) ;
		op->fd = -1 ;
	    }
	    if (op->clientfname != NULL) {
		if (op->clientfname[0] != '\0') {
		    u_unlink(op->clientfname) ;
		}
		uc_free(op->clientfname) ;
		op->clientfname = NULL ;
	    }

	} /* end if (bind-off) */

#if	CF_DEBUGS
	debugprintf("msuclients_bind: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (msuclients_bind) */


static int msuclients_connect(MSUCLIENTS *op)
{
	struct ustat	sb ;
	int		rs = SR_OK ;

#if	CF_DEBUGS
	debugprintf("msuclients_connect: reqfname=%s\n",op->reqfname) ;
#endif

	if (op->reqfname == NULL) {
	    rs = SR_DESTADDRREQ ;
	    goto ret0 ;
	}

	if (rs >= 0) {
	    rs = u_stat(op->reqfname,&sb) ;
	    if (rs >= 0) {
		if (! S_ISSOCK(sb.st_mode))
		    rs = SR_ISDIR ;
	    }
	}

	if (rs == SR_NOENT) {
	    rs = msuclients_spawn(op) ;
	}

	if (rs >= 0) {
	    int	af = AF_UNIX ;
	    rs = sockaddress_start(&op->srv,af,op->reqfname,0,0) ;
	    op->srvlen = rs ;
	    op->f.srv = (rs >= 0) ;

#if	CF_DEBUGS
	   { 
		int	mc = COLUMNS ;
		const char	*s = (const char *) &op->srv ;
	        debugprintf("msuclients_connect: srvlen=%d\n",rs) ;
	        debugprinthexblock("msuclients_connect:",mc,s,op->srvlen) ;
	    }
#endif

	}

	if (rs >= 0) {
	    op->dt = time(NULL) ;
	    rs = msuclients_istatus(op) ;
	}

	if (rs < 0)
	    goto bad1 ;

ret0:

#if	CF_DEBUGS
	debugprintf("msuclients_connect: ret rs=%d\n",rs) ;
#endif

	return rs ;

/* bad stuff! */
bad1:
bad0:
	goto ret0 ;
}
/* end subroutine (msuclients_connect) */


static int msuclients_disconnect(MSUCLIENTS *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op->f.srv) {
	    op->f.srv = FALSE ;
	    rs1 = sockaddress_finish(&op->srv) ;
	    if (rs >= 0) rs = rs1 ;
	}

	return rs ;
}
/* end subroutine (msuclients_disconnect) */


static int msuclients_istatus(MSUCLIENTS *op)
{
	struct msumsg_status	m0 ;
	struct msumsg_getstatus	m1 ;
	struct ipcmsginfo	mi, *mip = &mi ;
	struct sockaddr		*sap ;
	const int	ipclen = IPCBUFLEN ;
	int		rs ;
	int		to = op->to ;
	int		pid = 0 ;

#if	CF_DEBUGS
	debugprintf("msuclients_istatus: ent srvlen=%u\n",op->srvlen) ;
#endif

	sap = (struct sockaddr *) &op->srv ;
	ipcmsginfo_init(mip,sap,op->srvlen) ;

	memset(&m1,0,sizeof(struct msumsg_getstatus)) ;
	m1.tag = 0 ;

	if ((rs = msumsg_getstatus(&m1,0,mip->ipcbuf,ipclen)) >= 0) {
	    const int	blen = rs ;

	    mip->ipcmsg.msg_control = NULL ;
	    mip->ipcmsg.msg_controllen = 0 ;
	    mip->vecs[0].iov_len = blen ;

	    if ((rs = u_sendmsg(op->fd,&mip->ipcmsg,0)) >= 0) {

	        ipcmsginfo_init(mip,NULL,0) ;

	        mip->ipcmsg.msg_control = NULL ;
	        mip->ipcmsg.msg_controllen = 0 ;
	        mip->vecs[0].iov_len = IPCBUFLEN ;

	        if ((rs = uc_recvmsge(op->fd,&mip->ipcmsg,0,to,0)) >= 0) {
	            rs = msumsg_status(&m0,1,mip->ipcbuf,IPCBUFLEN) ;
		    pid = m0.pid ;
	        }

	    } /* end if (u_sendmsg) */

	} /* end if (msumsg_getstatus) */

#if	CF_DEBUGS
	debugprintf("msuclients_istatus: ret rs=%d pid=%d\n",rs,pid) ;
#endif

	return (rs >= 0) ? pid : rs ;
}
/* end subroutine (msuclients_istatus) */


static int msuclients_spawn(MSUCLIENTS *op)
{
	SPAWNPROC	ps ;
	ENVMGR		em ;
	int		rs = SR_OK ;
	int		cs ;
	int		i ;
	int		to_run = TO_RUN ;
	const char	*av[6] ;
	const char	**ev ;
	const char	*argz = MSUCLIENTS_FACNAME ;
	char		progfname[MAXPATHLEN + 1] ;
	char		optbuf[OPTBUFLEN + 1] ;

	for (i = 0 ; (rs >= 0) && (prbins[i] != NULL) ; i += 1) {
	    rs = mkpath3(progfname,op->pr,prbins[i],argz) ;
	    if (rs >= 0)
	        rs = perm(progfname,-1,-1,NULL,X_OK) ;
	} /* end for */

	if (rs < 0)
	    goto ret0 ;

	rs = envmgr_start(&em) ;
	if (rs < 0)
	    goto ret0 ;

/* prepare the child environment */

	rs = envmgr_set(&em,VARMSUQUIET,"1",1) ;
	if (rs >= 0)
	    rs = envmgr_set(&em,VARMSUPR,op->pr,-1) ;

	if (rs < 0)
	     goto ret1 ;

/* prepare the arguments */

	{
	    char	digbuf[DIGBUFLEN + 1] ;
	    if ((rs = ctdeci(digbuf,DIGBUFLEN,to_run)) >= 0) {
	        rs = sncpy2(optbuf,OPTBUFLEN,"-d=",digbuf) ;
	    }
	}

	if (rs < 0)
	    goto ret1 ;

	i = 0 ; 
	    av[i++] = argz ;
	    av[i++] = optbuf ;
	    av[i++] = "-o" ;
	    av[i++] = "quick" ;
	    av[i++] = NULL ;

	    envmgr_getvec(&em,&ev) ;

	    memset(&ps,0,sizeof(SPAWNPROC)) ;
	    ps.disp[0] = SPAWNPROC_DCLOSE ;
	    ps.disp[1] = SPAWNPROC_DCLOSE ;
	    ps.disp[2] = SPAWNPROC_DCLOSE ;
	    if ((rs = spawnproc(&ps,progfname,av,ev)) >= 0) {
	    	pid_t	pid = rs ;
	        rs = u_waitpid(pid,&cs,0) ;
	    } /* end if */

ret1:
	envmgr_finish(&em) ;

ret0:
	return rs ;
}
/* end subroutine (msuclients_spawn) */


static int envmgr_start(ENVMGR *emp)
{
	const int	vo = (VECHAND_OCOMPACT | VECHAND_OSORTED) ;
	int		rs ;
	int		i ;

	rs = vechand_start(&emp->envlist,10,vo) ;
	if (rs < 0)
	    goto bad0 ;

	rs = vecstr_start(&emp->envstrs,2,0) ;
	if (rs < 0)
	    goto bad1 ;

	for (i = 0 ; (rs >= 0) && (environ[i] != NULL) ; i += 1) {
	    rs = vechand_add(&emp->envlist,environ[i]) ;
	}

	if (rs < 0)
	    goto bad1 ;

ret0:
	return rs ;

bad1:
	vechand_finish(&emp->envlist) ;

bad0:
	goto ret0 ;
}
/* end subroutine (envmgr_start) */


static int envmgr_finish(ENVMGR *emp)
{
	int		rs = SR_OK ;
	int		rs1 ;

	rs1 = vecstr_finish(&emp->envstrs) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = vechand_finish(&emp->envlist) ;
	if (rs >= 0) rs = rs1 ;

	return rs ;
}
/* end subroutine (envmgr_finish) */


static int envmgr_set(ENVMGR *emp,const char *kp,const char *vp,int vl)
{
	vecstr		*esp = &emp->envstrs ;
	vechand		*elp = &emp->envlist ;
	int		rs ;
	if ((rs = vecstr_envset(esp,kp,vp,vl)) >= 0) {
	    const int	i = rs ;
	    cchar	*ep ;
	    if ((rs = vecstr_get(esp,i,&ep)) >= 0) {
		const int	nrs = SR_NOTFOUND ;
	        int (*venvcmp)(const void **,const void **) ;
	        venvcmp = (int (*)(const void **,const void **)) vstrkeycmp ;
	        if ((rs = vechand_search(elp,kp,venvcmp,NULL)) >= 0) {
	            vechand_del(elp,rs) ;
	        } else if (rs == nrs) {
		    rs = SR_OK ;
		}
	        if (rs >= 0) {
		    rs = vechand_add(elp,ep) ;
	        }
	    }
	}

	return rs ;
}
/* end subroutine (envmgr_set) */


static int envmgr_getvec(ENVMGR *emp,const char ***rppp)
{
	int		rs ;

	rs = vechand_getvec(&emp->envlist,rppp) ;

	return rs ;
}
/* end subroutine (envmgr_getvec) */


static int ipcmsginfo_init(IPCMSGINFO *mip,SOCKADDR *sap,int sal)
{
	int		size ;

#ifdef	OPTIONAL
	memset(mip,0,sizeof(struct ipcmsginfo)) ;
#endif

	mip->ipcmsglen = 0 ;
	mip->ns = -1 ;
	mip->ipcbuf[0] = '\0' ;

	size = NIOVECS * sizeof(struct iovec) ;
	memset(mip->vecs,0,size) ;

	mip->vecs[0].iov_base = mip->ipcbuf ;
	mip->vecs[0].iov_len = IPCBUFLEN ;

	memset(&mip->ipcmsg,0,sizeof(struct msghdr)) ;

	if (sap != NULL) {
	    mip->ipcmsg.msg_name = sap ;
	} else {
	    mip->ipcmsg.msg_name = (struct sockaddr *) &mip->ipcfrom ;
	}

	mip->ipcmsg.msg_namelen = (sal > 0) ? sal : sizeof(SOCKADDRESS) ;

	mip->ipcmsg.msg_iov = mip->vecs ;
	mip->ipcmsg.msg_iovlen = NIOVECS ;
	mip->ipcmsg.msg_control = &mip->ipcconbuf ;
	mip->ipcmsg.msg_controllen = CMSGBUFLEN ;

	return SR_OK ;
}
/* end subroutine (ipcmsginfo_init) */


#ifdef	COMMENT
static int venvcmp(e1pp,e2pp)
const void	**e1pp, **e2pp ;
{
	const char	*e1p ;
	const char	*e2p ;
	int		rc ;

	if ((*e1pp == NULL) && (*e2pp == NULL))
	    return 0 ;

	if (*e1pp == NULL)
	    return 1 ;

	if (*e2pp == NULL)
	    return -1 ;

	e1p = (const char *) *e1pp ;
	e2p = (const char *) *e2pp ;
	rc = (e1p[0] - e2p[0]) ;
	if (rc == 0)
	    rc = strcmp(e1p,e2p) ;

	return rc ;
}
/* end subroutine (venvcmp) */
#endif /* COMMENT */


