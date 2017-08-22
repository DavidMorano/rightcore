/* uss */

/* SYSDIALER "uss" dialer */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 2003-11-04, David A­D­ Morano
        This was created as one of the first dialer modules for the SYSDIALER
        object.

*/

/* Copyright © 2003 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This is a SYSDIALER module.

	Synopsis:

	uss <path>

	Arguments:

	<path>		path to socket file


*******************************************************************************/


#define	USS_MASTER	0


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<strings.h>		/* for |strcasecmp(3c)| */

#include	<vsystem.h>
#include	<keyopt.h>
#include	<ids.h>
#include	<userinfo.h>
#include	<vecstr.h>
#include	<nulstr.h>
#include	<expcook.h>
#include	<localmisc.h>

#include	"envs.h"
#include	"inetaddrparse.h"
#include	"sysdialer.h"
#include	"uss.h"


/* local defines */

#define	USS_MNAME	"uss"
#define	USS_VERSION	"0"
#define	USS_INAME	""
#define	USS_MF1		(SYSDIALER_MFULL | SYSDIALER_MHALFOUT)
#define	USS_MF2		(SYSDIALER_MCOR | SYSDIALER_MCO)
#define	USS_MF3		(SYSDIALER_MHALFIN)
#define	USS_MF		(USS_MF1 | USS_MF2|USS_MF3)

#define	USS_SEARCHNAME	"uss"
#define	USS_VARPR	"LOCAL"
#define	USS_PR		"/usr/add-on/local"
#define	USS_LOGDNAME	"log"
#define	USS_LOGFNAME	SYSDIALER_LF

#ifndef	SVCNAMELEN
#define	SVCNAMELEN	32
#endif

#define	ARGBUFLEN	(MAXPATHLEN + 35)

#define	SUBINFO		struct subinfo
#define	SUBINFO_ALLOCS	struct subinfo_allocs
#define	SUBINFO_FL	struct subinfo_flags


/* external subroutines */

extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecti(const char *,int,int *) ;
extern int	optbool(const char *,int) ;
extern int	sperm(IDS *,struct ustat *,int) ;
extern int	getpwd(char *,int) ;
extern int	getnodedomain(char *,char *) ;
extern int	getgroupname(char *,int,gid_t) ;
extern int	mkpr(char *,int,const char *,const char *) ;
extern int	getaf(const char *,int) ;
extern int	dialuss(const char *,int,int) ;
extern int	dialussnls(const char *,const char *,int,int) ;
extern int	dialussmux(const char *,const char *,const char **,int,int) ;
extern int	dialtcp(const char *,const char *,int,int,int) ;
extern int	dialtcpnls(const char *,const char *,int,const char *,int,int) ;
extern int	dialtcpmux(const char *,const char *,int,const char *,
			const char **,int,int) ;
extern int	vecstr_adduniq(vecstr *,const char *,int) ;
extern int	vecstr_loadfile(vecstr *,int,const char *) ;
extern int	vecstr_envadd(VECSTR *,const char *,const char *,int) ;
extern int	vecstr_envadds(VECSTR *,const char *,int) ;
extern int	logfile_userinfo(LOGFILE *,USERINFO *,time_t,
			const char *,const char *) ;
extern int	hasleadcolon(const char *,int) ;
extern int	isdigitlatin(int) ;
extern int	isNotPresent(int) ;

extern char	*strwcpy(char *,const char *,int) ;


/* external variables */

extern char	**environ ;


/* local structures */

struct subinfo_flags {
	uint		stores:1 ;
	uint		ids:1 ;
	uint		userinfo:1 ;
	uint		ignore:1 ;
	uint		progdash:1 ;
	uint		log:1 ;
} ;

struct subinfo_allocs {
	const char	*node ;
	const char	*svc ;
	const char	*pr ;
	const char	*portspec ;
} ;

struct subinfo {
	const char	**argv ;
	const char	**envv ;
	const char	*pr ;
	const char	*prn ;
	const char	*searchname ;
	const char	*afspec ;
	const char	*hostname ;
	const char	*portspec ;
	const char	*svcname ;
	const char	*pvfname ;
	const char	*dfname ;
	const char	*xfname ;
	const char	*efname ;
	const char	*architecture ;		/* machine architecture */
	const char	*umachine ;		/* UNAME machine name */
	const char	*usysname ;		/* UNAME OS system-name */
	const char	*urelease ;		/* UNAME OS release */
	const char	*uversion ;		/* UNAME OS version */
	const char	*hz ;			/* OS HZ */
	const char	*nodename ;		/* USERINFO */
	const char	*domainname ;		/* USERINFO */
	const char	*username ;		/* USERINFO */
	const char	*homedname ;		/* USERINFO */
	const char	*shell ;		/* USERINFO */
	const char	*organization ;		/* USERINFO */
	const char	*gecosname ; 		/* USERINFO */
	const char	*realname ;		/* USERINFO */
	const char	*name ;			/* USERINFO */
	const char	*tz ;			/* USERINFO */
	const char	*groupname ;
	const char	*tmpdname ;
	const char	*maildname ;
	const char	*hfname ;
	const char	*lfname ;
	const char	*paramfname ;
	const char	*version ;		/* object version */
	const char	*logid ;
	const char	*defprog ;
	USS		*op ;
	SYSDIALER_ARGS	*ap ;
	IDS		id ;
	VECSTR		aenvs ;
	VECSTR		stores ;
	USERINFO	u ;
	SUBINFO_ALLOCS	a ;
	SUBINFO_FL	f, init, open ;
	uid_t		uid ;
	gid_t		gid ;
	int		argc ;
	int		argi ;
	int		ncpu ;
	int		af ;
	int		to ;
} ;

struct intprog {
	char		fname[MAXPATHLEN + 1] ;
	char		arg[MAXPATHLEN + 1] ;
} ;

struct afamily {
	const char	*name ;
	int		af ;
} ;


/* forward references */

static int	uss_logbegin(USS *op,const char *,const char *) ;
static int	uss_logend(USS *) ;
static int	uss_logstuff(USS *,SUBINFO *) ;

static int	subinfo_start(struct subinfo *,USS *,
			SYSDIALER_INFO *,
			SYSDIALER_ARGS *,
			const char *,const char *) ;
static int	subinfo_procargs(struct subinfo *) ;
static int	subinfo_procopts(struct subinfo *,KEYOPT *) ;
static int	subinfo_defaults(struct subinfo *) ;
static int	subinfo_userinfo(struct subinfo *) ;
static int	subinfo_logfile(struct subinfo *) ;
static int	subinfo_addrparse(struct subinfo *) ;
static int	subinfo_addrparseunix(struct subinfo *,int) ;
static int	subinfo_addrparseinet(struct subinfo *) ;
static int	subinfo_dirok(struct subinfo *,const char *,int) ;
static int	subinfo_setentry(struct subinfo *,const char **,
			const char *,int) ;
static int	subinfo_finish(struct subinfo *) ;


/* global variables (module information) */

SYSDIALER_INFO	uss = {
	USS_MNAME,
	USS_VERSION,
	USS_INAME,
	sizeof(USS),
	USS_MF
} ;


/* local variables */

static const char *argopts[] = {
	"ROOT",
	"RN",
	"sn",
	"af",
	"lf",
	"pvf",
	"pf",
	"df",
	"xf",
	"ef",
	NULL
} ;

enum argopts {
	argopt_root,
	argopt_rn,
	argopt_sn,
	argopt_af,
	argopt_lf,
	argopt_pvf,
	argopt_pf,
	argopt_df,
	argopt_xf,
	argopt_ef,
	argopt_overlast
} ;

static const char *procopts[] = {
	"log",
	NULL
} ;

enum procopts {
	procopt_log,
	procopt_overlast
} ;


/* exported subroutines */


/* ARGSUSED */
int uss_open(op,ap,hostname,svcname,av)
USS		*op ;
SYSDIALER_ARGS	*ap ;
const char	hostname[] ;
const char	svcname[] ;
const char	*av[] ;
{
	struct subinfo	si, *sip = &si ;

	int	rs = SR_OK ;
	int	rs1 ;
	int	opts = 0 ;


	if (op == NULL)
	    return SR_FAULT ;

	if (hostname == NULL)
	    return SR_FAULT ;

	if (hostname[0] == '\0')
	    return SR_INVALID ;

	memset(op,0,sizeof(USS)) ;

#if	CF_DEBUGS
	{
	    int	i ;
	    debugprintf("uss_open: ent hostname=%s svcname=%s\n",
	        hostname,svcname) ;
	    if (ap->argv != NULL) {
	        for (i = 0 ; ap->argv[i] != NULL ; i += 1) {
		    debugprintf("uss_open: a%u=>%s<\n",i,ap->argv[i]) ;
	        }
	    }
	}
#endif /* CF_DEBUGS */

	if ((rs = subinfo_start(sip,op,&uss,ap,hostname,svcname)) >= 0) {

	    rs = subinfo_defaults(sip) ;

#if	CF_DEBUGS
	debugprintf("uss_open: subinfo_logstuff()\n") ;
	debugprintf("uss_open: f_log=%u\n",op->open.log) ;
#endif

	if ((rs >= 0) && sip->f.log)
	    rs = uss_logstuff(op,sip) ;

/* parse the port-specification if we have one */

	if (rs >= 0)
	    rs = subinfo_addrparse(sip) ;

#if	CF_DEBUGS
	debugprintf("uss_open: af=%u\n",sip->af) ;
	debugprintf("uss_open: hostname=%s\n",hostname) ;
	debugprintf("uss_open: portspec=%s\n",sip->portspec) ;
	debugprintf("uss_open: svcname=%s\n",svcname) ;
#endif

/* OK, do the dial */

	if (rs >= 0) {
	    switch (sip->af) {
	    case AF_UNSPEC:
	    case AF_UNIX:
		if (sip->portspec != NULL) {
	        	rs = dialuss(sip->portspec,sip->to,opts) ;
		} else
	     		rs = SR_NOENT ;
		break ;
	    } /* end switch */
	} /* end if */
	op->fd = rs ;

	if (rs >= 0) {
	    op->magic = USS_MAGIC ;
	    uc_closeonexec(op->fd,TRUE) ;
	}

	    rs1 = subinfo_finish(sip) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if */

	if ((rs < 0) && (op->fd >= 0)) {
	    u_close(op->fd) ;
	    op->fd = -1 ;
	    op->magic = 0 ;
	}

#if	CF_DEBUGS
	debugprintf("uss_open: ret rs=%d fd=%d\n",rs,op->fd) ;
#endif

	return (rs >= 0) ? op->fd : rs ;
}
/* end subroutine (uss_open) */


int uss_reade(op,buf,buflen,to,opts)
USS		*op ;
char		buf[] ;
int		buflen ;
int		to, opts ;
{
	int	rs ;


	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != USS_MAGIC)
	    return SR_NOTOPEN ;

	rs = uc_reade(op->fd,buf,buflen,to,opts) ;

	if (rs > 0)
	    op->tlen += rs ;

	return rs ;
}
/* end subroutine (uss_reade) */


int uss_recve(op,buf,buflen,flags,to,opts)
USS		*op ;
char		buf[] ;
int		buflen ;
int		flags ;
int		to, opts ;
{
	int	rs ;


	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != USS_MAGIC)
	    return SR_NOTOPEN ;

	rs = uc_recve(op->fd,buf,buflen,flags,to,opts) ;

	if (rs > 0)
	    op->tlen += rs ;

	return rs ;
}
/* end subroutine (uss_recve) */


int uss_recvfrome(op,buf,buflen,flags,sap,salenp,to,opts)
USS		*op ;
char		buf[] ;
int		buflen ;
int		flags ;
void		*sap ;
int		*salenp ;
int		to, opts ;
{
	int	rs ;


	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != USS_MAGIC)
	    return SR_NOTOPEN ;

	rs = uc_recvfrome(op->fd,buf,buflen,flags,sap,salenp,to,opts) ;

	if (rs > 0)
	    op->tlen += rs ;

	return rs ;
}
/* end subroutine (uss_recvfrome) */


int uss_recvmsge(op,msgp,flags,to,opts)
USS		*op ;
struct msghdr	*msgp ;
int		flags ;
int		to, opts ;
{
	int	rs ;


	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != USS_MAGIC)
	    return SR_NOTOPEN ;

	rs = uc_recvmsge(op->fd,msgp,flags,to,opts) ;

	if (rs > 0)
	    op->tlen += rs ;

	return rs ;
}
/* end subroutine (uss_recvmsge) */


int uss_write(op,buf,buflen)
USS		*op ;
const char	buf[] ;
int		buflen ;
{
	int	rs ;


	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != USS_MAGIC)
	    return SR_NOTOPEN ;

	rs = uc_writen(op->fd,((void *) buf),buflen) ;

	if (rs > 0)
	    op->tlen += rs ;

	return rs ;
}
/* end subroutine (uss_write) */


int uss_send(op,buf,buflen,flags)
USS		*op ;
const char	buf[] ;
int		buflen ;
int		flags ;
{
	int	rs ;


	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != USS_MAGIC)
	    return SR_NOTOPEN ;

	rs = u_send(op->fd,buf,buflen,flags) ;

	if (rs > 0)
	    op->tlen += rs ;

	return rs ;
}
/* end subroutine (uss_send) */


int uss_sendto(op,buf,buflen,flags,sap,salen)
USS		*op ;
const char	buf[] ;
int		buflen ;
int		flags ;
void		*sap ;
int		salen ;
{
	int	rs ;


	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != USS_MAGIC)
	    return SR_NOTOPEN ;

	rs = u_sendto(op->fd,buf,buflen,flags,sap,salen) ;

	if (rs > 0)
	    op->tlen += rs ;

	return rs ;
}
/* end subroutine (uss_sendto) */


int uss_sendmsg(op,msgp,flags)
USS		*op ;
struct msghdr	*msgp ;
int		flags ;
{
	int	rs ;


	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != USS_MAGIC)
	    return SR_NOTOPEN ;

	rs = u_sendmsg(op->fd,msgp,flags) ;

	if (rs > 0)
	    op->tlen += rs ;

	return rs ;
}
/* end subroutine (uss_sendmsg) */


/* shutdown */
int uss_shutdown(op,cmd)
USS		*op ;
int		cmd ;
{
	int	rs ;


	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != USS_MAGIC)
	    return SR_NOTOPEN ;

	rs = u_shutdown(op->fd,cmd) ;

	if (rs > 0)
	    op->tlen += rs ;

	return rs ;
}
/* end subroutine (uss_shutdown) */


/* close the connection */
int uss_close(op)
USS		*op ;
{
	int	rs = SR_OK ;
	int	rs1 ;


	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != USS_MAGIC)
	    return SR_NOTOPEN ;

	rs1 = u_close(op->fd) ;
	if (rs >= 0) rs = rs1 ;

	if (op->open.log) {
	    logfile_printf(&op->lh,"bytes=%u",op->tlen) ;
	    rs1 = uss_logend(op) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if */

	op->magic = 0 ;
	return rs ;
}
/* end subroutine (uss_close) */


/* private subroutines */


static int uss_logbegin(USS *op,const char *lfname,const char *logid)
{
	int	rs = SR_OK ;
	int	f = op->open.log ;

	if (! op->open.log) {
	    if ((rs = logfile_open(&op->lh,lfname,0,0666,logid)) >= 0) {
	        op->open.log = TRUE ;
		f = TRUE ;
	    } else if (isNotPresent(rs))
	        rs = SR_OK ;
	} /* end if (needed opening) */

	return (rs >= 0) ? f : rs ;
}
/* end if (uss_logbegin) */


static int uss_logend(USS *op)
{
	int	rs = SR_OK ;
	int	rs1 ;

	if (op->open.log) {
	    op->open.log = FALSE ;
	    rs1 = logfile_close(&op->lh) ;
	    if (rs >= 0) rs = rs1 ;
	}

	return rs ;
}
/* end if (uss_logend) */


static int uss_logstuff(USS *op,SUBINFO *sip)
{
	int	rs ;
	int	f = FALSE ;

	if ((rs = subinfo_logfile(sip)) > 0) {
	    const char	*lfname = sip->lfname ;
	    const char	*logid = sip->logid ;
	    if ((rs = uss_logbegin(op,lfname,logid)) > 0) {
		USERINFO	*uip = &sip->u ;
		f = TRUE ;

	                    logfile_userinfo(&op->lh,uip,0L,
	                        sip->searchname,sip->version) ;

	                    logfile_printf(&op->lh,"pid=%d",uip->pid) ;

	                    logfile_printf(&op->lh,"pr=%s",sip->pr) ;

	                    logfile_printf(&op->lh,"host=%s",sip->hostname) ;

	                    logfile_printf(&op->lh,"svc=%s",sip->svcname) ;

	    } /* end if (ussmux-logbegin) */
	} /* end if (subinfo-logfile) */

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (uss_logstuff) */


static int subinfo_start(sip,op,dip,ap,hostname,svcname)
struct subinfo	*sip ;
USS		*op ;
SYSDIALER_INFO	*dip ;
SYSDIALER_ARGS	*ap ;
const char	hostname[] ;
const char	svcname[] ;
{
	int	rs = SR_OK ;


	memset(sip,0,sizeof(struct subinfo)) ;

	sip->envv = (const char **) environ ;
	sip->op = op ;
	sip->ap = ap ;
	sip->pr = ap->pr ;
	sip->to = ap->timeout ;
	sip->hostname = hostname ;
	sip->svcname = svcname ;
	sip->version = dip->version ;
	sip->af = -1 ;

	if ((rs = vecstr_start(&sip->stores,3,0)) >= 0) {
	    sip->open.stores = TRUE ;
	    if ((rs = vecstr_start(&sip->aenvs,3,0)) >= 0) {
	        if (ap != NULL) {
	    	    rs = subinfo_procargs(sip) ;
		}
		if (rs < 0)
		    vecstr_finish(&sip->aenvs) ;
	    } /* end if (vecstr-aenvs) */
	    if (rs < 0)
	        vecstr_finish(&sip->stores) ;
	} /* end if (vecstr-stores) */

#if	CF_DEBUGS
	debugprintf("uss/subinfo_start: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (subinfo_start) */


static int subinfo_finish(sip)
struct subinfo	*sip ;
{
	int	rs = SR_OK ;
	int	rs1 ;

	if (sip->open.userinfo) {
	    sip->open.userinfo = FALSE ;
	    rs1 = userinfo_finish(&sip->u) ;
	    if (rs >= 0) rs = rs1 ;
	}

	if (sip->open.ids) {
	    sip->open.ids = FALSE ;
	    rs1 = ids_load(&sip->id) ;
	    if (rs >= 0) rs = rs1 ;
	}

	if (sip->a.node != NULL) {
	    rs1 = uc_free(sip->a.node) ;
	    if (rs >= 0) rs = rs1 ;
	    sip->a.node = NULL ;
	}

	if (sip->a.svc != NULL) {
	    rs1 = uc_free(sip->a.svc) ;
	    if (rs >= 0) rs = rs1 ;
	    sip->a.svc = NULL ;
	}

	if (sip->a.pr != NULL) {
	    rs1 = uc_free(sip->a.pr) ;
	    if (rs >= 0) rs = rs1 ;
	    sip->a.pr = NULL ;
	}

	if (sip->a.portspec != NULL) {
	    rs1 = uc_free(sip->a.portspec) ;
	    if (rs >= 0) rs = rs1 ;
	}

	rs1 = vecstr_finish(&sip->aenvs) ;
	if (rs >= 0) rs = rs1 ;

	if (sip->open.stores) {
	    sip->open.stores = FALSE ;
	    rs1 = vecstr_finish(&sip->stores) ;
	    if (rs >= 0) rs = rs1 ;
	}

	return rs ;
}
/* end subroutine (subinfo_finish) */


static int subinfo_procargs(sip)
struct subinfo	*sip ;
{
	KEYOPT		akopts ;

	SYSDIALER_ARGS	*ap = sip->ap ;

	int	rs = SR_OK ;
	int	rs1 ;
	int	argc ;
	int	argr, argl, aol, akl, avl, kwi ;
	int	ai, ai_max, ai_pos ;
	int	pan = 0 ;
	int	f_optminus, f_optplus, f_optequal ;
	int	f_doubledash = FALSE ;

	const char	**argv ;
	const char	*argval = NULL ;
	const char	*argp, *aop, *akp, *avp ;


#if	CF_DEBUGS
	debugprintf("uss/subinfo_procargs: arguments\n") ;
#endif

	argv = (const char **) ap->argv ;

	for (argc = 0 ; argv[argc] != NULL ; argc += 1) ;

#if	CF_DEBUGS
	{
	    debugprintf("uss/subinfo_procargs: argc=%u\n",argc) ;
	    for (ai = 0 ; argv[ai] != NULL ; ai += 1)
	        debugprintf("uss/subinfo_procargs: argv[%u]=%s\n",
		ai,argv[ai]) ;
	}
#endif /* CF_DEBUGS */

	sip->argc = argc ;
	sip->argv = argv ;
	rs = keyopt_start(&akopts) ;
	if (rs < 0) goto badkopts ;

/* process program arguments */

	ai_max = 0 ;
	ai_pos = 0 ;
	argr = argc ;
	for (ai = 0 ; (ai < argc) && (argv[ai] != NULL) ; ai += 1) {
	    if (rs < 0) break ;
	    argr -= 1 ;
	    if (ai == 0) continue ;

	    argp = argv[ai] ;
	    argl = strlen(argp) ;

	    f_optminus = (*argp == '-') ;
	    f_optplus = (*argp == '+') ;
	    if ((argl > 1) && (f_optminus || f_optplus)) {
		const int	ach = MKCHAR(argp[1]) ;

	        if (isdigitlatin(ach)) {

	            argval = (argp + 1) ;

	        } else if ((argl == 2) && (ach == '-')) {

	            f_doubledash = TRUE ;
	            ai += 1 ;
	            argr -= 1 ;

	        } else {
		    int	v ;

#if	CF_DEBUGS
	            debugprintf("uss/subinfo_procargs: option? ao=>%t<\n",
			argp,argl) ;
#endif

	            aop = argp + 1 ;
	            akp = aop ;
	            aol = argl - 1 ;
	            f_optequal = FALSE ;
	            if ((avp = strchr(aop,'=')) != NULL) {
	                f_optequal = TRUE ;
	                akl = avp - aop ;
	                avp += 1 ;
	                avl = aop + argl - 1 - avp ;
	                aol = akl ;
	            } else {
	                avp = NULL ;
	                avl = 0 ;
	                akl = aol ;
	            }

#if	CF_DEBUGS
	            debugprintf("uss/subinfo_procargs: k=>%t<\n",akp,akl) ;
#endif

	            if ((kwi = matostr(argopts,2,akp,akl)) >= 0) {

	                switch (kwi) {

/* program-root */
	                case argopt_root:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            sip->pr = avp ;
	                    } else {
	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            sip->pr = argp ;
	                    }
	                    break ;

/* program-root-name */
	                case argopt_rn:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            sip->prn = avp ;
	                    } else {
	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            sip->prn = argp ;
	                    }
	                    break ;

/* search-name root */
	                case argopt_sn:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            sip->searchname = avp ;
	                    } else {
	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            sip->searchname = argp ;
	                    }
	                    break ;

/* logfile */
	                case argopt_lf:
			    sip->f.log = TRUE ;
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            sip->lfname = avp ;
	                    } else {
	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            sip->lfname = argp ;
	                    }
	                    break ;

/* path-vars file */
	                case argopt_pvf:
	                case argopt_pf:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            sip->pvfname = avp ;
	                    } else {
	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            sip->pvfname = argp ;
	                    }
	                    break ;

	                case argopt_df:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            sip->dfname = avp ;
	                    } else {
	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            sip->dfname = avp ;
	                    }
	                    break ;

	                case argopt_xf:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            sip->xfname = avp ;
	                    } else {
	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            sip->xfname = argp ;
	                    }
			    break ;

	                case argopt_ef:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            sip->efname = avp ;
	                    } else {
	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            sip->efname = argp ;
	                    }
	                    break ;

	                } /* end switch (key words) */

	            } else {

	                while (akl--) {
			    int	kc = (*akp & 0xff) ;

	                    switch (kc) {

/* address-family */
	                    case 'f':
	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            sip->afspec = argp ;
	                        break ;

	                    case 'i':
	                        sip->f.ignore = TRUE ;
	                        break ;

/* options */
	                    case 'o':
	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            rs = keyopt_loads(&akopts,argp,argl) ;
	                        break ;

/* service */
	                    case 's':
	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            sip->svcname = argp ;
	                        break ;
/* timeout */
	                    case 't':
	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl) {
	                            rs = cfdecti(argp,argl,&v) ;
				    sip->to = v ;
				}
	                        break ;

/* eXported environment */
	                    case 'x':
	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl) {
				    vecstr	*vlp = &sip->aenvs ;
	                            rs = vecstr_envadds(vlp,argp,argl) ;
			        }
	                        break ;

	                    default:
	                        rs = SR_INVALID ;
				break ;

	                    } /* end switch */
	                    akp += 1 ;

	                    if (rs < 0) break ;
	                } /* end while */

	            } /* end if (individual option key letters) */

	        } /* end if (digits as argument or not) */

	    } else {

		switch (pan) {
		case 0:
	            sip->portspec = (const char *) argp ;
		    break ;
		case 1:
	            sip->svcname = (const char *) argp ;
		    break ;
		} /* end switch */
		pan += 1 ;

	    } /* end if (key letter-word or positional) */

	    ai_pos = ai ;

	} /* end while (all command line argument processing) */

	sip->argi = (argc > 0) ? (ai + 1) : 0 ;

#if	CF_DEBUGS
	debugprintf("uss/subinfo_procargs: portspec=%s\n",sip->portspec) ;
	debugprintf("uss/subinfo_procargs: argi=%u\n",sip->argi) ;
#endif

	if (rs >= 0)
	    rs = subinfo_procopts(sip,&akopts) ;

/* process any address-family specification */

#if	CF_DEBUGS
	debugprintf("uss/subinfo_procargs: af=%d afspec=%s\n",
		sip->af, sip->afspec) ;
#endif

	if ((rs >= 0) && (sip->af < 0) && 
		(sip->afspec != NULL) && (sip->afspec[0] != '\0')) {

#if	CF_DEBUGS
	debugprintf("uss/subinfo_procargs: afspec=%s\n",sip->afspec) ;
#endif

	    rs1 = getaf(sip->afspec,-1) ;
	    if (rs1 >= 0)
		sip->af = rs1 ;

#if	CF_DEBUGS
	debugprintf("uss/subinfo_procargs: getaf() rs=%d\n",rs1) ;
#endif

	} /* end if */

/* done and out */
done:
	keyopt_finish(&akopts) ;

badkopts:

#if	CF_DEBUGS
	debugprintf("uss/subinfo_procargs: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (subinfo_procargs) */


static int subinfo_procopts(sip,kop)
struct subinfo	*sip ;
KEYOPT		*kop ;
{
	KEYOPT_CUR	kcur ;

	int	rs = SR_OK ;
	int	c = 0 ;

	if ((rs = keyopt_curbegin(kop,&kcur)) >= 0) {
	    int		oi ;
	    int		kl, vl ;
	    const char	*kp, *vp ;

	while ((kl = keyopt_enumkeys(kop,&kcur,&kp)) >= 0) {

	    vl = keyopt_fetch(kop,kp,NULL,&vp) ;

	    if ((oi = matostr(procopts,2,kp,kl)) >= 0) {

	        switch (oi) {

	        case procopt_log:
	                sip->f.log = TRUE ;
	                if ((vl > 0) && ((rs = optbool(vp,vl)) >= 0))
	                    sip->f.log = (rs > 0) ;
	            break ;

	        } /* end switch */
	        c += 1 ;

	    } /* end if (valid option) */

	    if (rs < 0) break ;
	} /* end while (looping through key options) */

	keyopt_curend(kop,&kcur) ;
	} /* end if (keyopt-cur) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (subinfo_procopts) */


static int subinfo_setentry(sip,epp,sp,sl)
struct subinfo	*sip ;
const char	**epp ;
const char	sp[] ;
int		sl ;
{
	int	rs = SR_OK ;
	int	len = 0 ;


	if (sip == NULL) return SR_FAULT ;
	if (epp == NULL) return SR_FAULT ;

	if (! sip->open.stores) {
	    rs = vecstr_start(&sip->stores,4,0) ;
	    sip->open.stores = (rs >= 0) ;
	}

	if (rs >= 0) {
	    int	oi = -1 ;

	    if (*epp != NULL) oi = vecstr_findaddr(&sip->stores,*epp) ;

	    if (sp != NULL) { 
		len = strnlen(sp,sl) ;
	        if ((rs = vecstr_add(&sip->stores,sp,len)) >= 0) {
	            rs = vecstr_get(&sip->stores,rs,epp) ;
	        } /* end if (added new entry) */
	    } else
		*epp = NULL ;

	    if ((rs >= 0) && (oi >= 0))
	        vecstr_del(&sip->stores,oi) ;

	} /* end if */

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (subinfo_setentry) */


static int subinfo_defaults(sip)
struct subinfo	*sip ;
{
	SYSDIALER_ARGS	*ap ;

	int	rs = SR_OK ;
	int	rs1 ;

	const char	*vp ;


	ap = sip->ap ;

#if	CF_DEBUGS
	debugprintf("uss/subinfo_defaults: ent af=%d\n",sip->af) ;
#endif

/* program-root */

	if ((sip->pr == NULL) && (sip->prn != NULL) && (sip->prn[0] != '\0')) {
	    char	domainname[MAXHOSTNAMELEN + 1] ;
	    char	pr[MAXPATHLEN + 1] ;

	    rs1 = getnodedomain(NULL,domainname) ;

	    if (rs1 >= 0)
	        rs1 = mkpr(pr,MAXPATHLEN,sip->prn,domainname) ;

	    if (rs1 >= 0) {

	        rs1 = subinfo_dirok(sip,pr,rs1) ;

	        if (rs1 > 0) {
	            rs = uc_mallocstrw(pr,rs1,&sip->a.pr) ;
	            if (rs > 0)
	                sip->pr = sip->a.pr ;
		}

	    } /* end if */

	} /* end if */

	if ((rs >= 0) && (sip->pr == NULL)) {

	    if ((vp = getenv(USS_VARPR)) != NULL) {

	        rs1 = subinfo_dirok(sip,vp,-1) ;
	        if (rs1 > 0)
	            sip->pr = vp ;

	    }

	} /* end if */

	if ((rs >= 0) && (sip->pr == NULL)) {

	    vp = USS_PR ;
	    rs1 = subinfo_dirok(sip,vp,-1) ;
	    if (rs1 > 0)
	        sip->pr = vp ;

	} /* end if */

	if (sip->pr == NULL)
	    sip->pr = ap->pr ;

/* search-name */

	if (sip->searchname == NULL)
	    sip->searchname = USS_SEARCHNAME ;

/* log-file */

	if ((rs >= 0) && (sip->lfname == NULL))
	    sip->lfname = USS_LOGFNAME ;

#if	CF_DEBUGS
	debugprintf("uss/subinfo_defaults: pr=%s\n",sip->pr) ;
	debugprintf("uss/subinfo_defaults: logfname=%s\n",sip->lfname) ;
#endif

/* address family */

#if	CF_DEBUGS
	debugprintf("uss/subinfo_defaults: af=%d\n",sip->af) ;
#endif

#ifdef	COMMENT
	if (sip->af < 0)
	    sip->af = AF_UNIX ;
#endif

/* out of here */

	return rs ;
}
/* end subroutine (subinfo_defaults) */


static int subinfo_userinfo(sip)
struct subinfo	*sip ;
{
	int	rs = SR_OK ;

	if (! sip->open.userinfo) {
	    USERINFO	*uip = &sip->u ;
	    if ((rs = userinfo_start(uip,NULL)) >= 0) {
	        sip->open.userinfo = TRUE ;
	        sip->umachine = uip->machine ;
	        sip->usysname = uip->sysname ;
	        sip->urelease = uip->release ;
	        sip->uversion = uip->version ;
	        sip->nodename = uip->nodename ;
	        sip->username = uip->username ;
	        sip->homedname = uip->homedname ;
	        sip->shell = uip->shell ;
	        sip->organization = uip->organization ;
	        sip->gecosname = uip->gecosname ;
	        sip->realname = uip->realname ;
	        sip->name = uip->name ;
	        sip->domainname = uip->domainname ;
	        sip->tz = uip->tz ;
	        sip->logid = uip->logid ;
	        sip->uid = uip->uid ;
	        sip->gid = uip->gid ;
	    } /* end if (userinfo-start) */
	} /* end if (needed initialization) */

	return rs ;
}
/* end subroutine (subinfo_userinfo) */


static int subinfo_logfile(sip)
struct subinfo	*sip ;
{
	int	rs = SR_OK ;
	int	f = sip->init.log ;

	if (sip->f.log && (! sip->init.log)) {
	    const char	*lfname = sip->lfname ;
	    sip->init.log = TRUE ;
	    f = TRUE ;

	    if (lfname[0] != '/') {
		const char	*logdname = USS_LOGDNAME ;
	        char		tbuf[MAXPATHLEN + 1] ;
	        if ((rs = mkpath3(tbuf,sip->pr,logdname,lfname)) >= 0) {
		    const char	**vpp = &sip->lfname ;
		    rs = subinfo_setentry(sip,vpp,tbuf,rs) ;
		}
	    }

#if	CF_DEBUGS
	debugprintf("subinfo/subinfo_logfile: lnp=%s\n",lnp) ;
#endif

	    if (rs >= 0) {
	        rs = subinfo_userinfo(sip) ;
	    } /* end if (ok) */

	} /* end if (needed initialziation) */

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (subinfo_logfile) */


static int subinfo_dirok(sip,d,dlen)
struct subinfo	*sip ;
const char	d[] ;
int		dlen ;
{
	struct ustat	sb ;

	NULSTR	ss ;

	int	rs = SR_OK ;
	int	rs1 ;
	int	f = FALSE ;

	const char	*dnp ;


	if (! sip->open.ids) {
	    sip->open.ids = TRUE ;
	    rs = ids_load(&sip->id) ;
	}

	if (rs >= 0) {
	    if ((rs = nulstr_start(&ss,d,dlen,&dnp)) >= 0) {
	        if ((rs1 = u_stat(dnp,&sb)) >= 0) {
	            if (S_ISDIR(sb.st_mode)) {
	    	        rs1 = sperm(&sip->id,&sb,(R_OK | X_OK)) ;
	    	        f = (rs1 >= 0) ;
		    }
	        }
		nulstr_finish(&ss) ;
	    } /* end if (nulstr) */
	} /* end if (ok) */

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (subinfo_dirok) */


static int subinfo_addrparse(sip)
struct subinfo	*sip ;
{
	int	rs = SR_OK ;


	if ((sip->portspec == NULL) || (sip->portspec[0] == '\0'))
	    goto ret0 ;

	if (hasleadcolon(sip->portspec,-1)) {

		if (strncasecmp("unix:",sip->portspec,5) == 0) {
			rs = subinfo_addrparseunix(sip,1) ;
		} else {
			rs = subinfo_addrparseinet(sip) ;
		}

	} else if (strchr(sip->portspec,'/') != NULL) {
			rs = subinfo_addrparseunix(sip,0) ;
	} else {
		if ((sip->af == 0) || 
			((sip->af >= 0) && (sip->af != AF_UNIX))) {
			rs = subinfo_addrparseinet(sip) ;
		} else {
			rs = subinfo_addrparseunix(sip,0) ;
		}
	}

ret0:
	return rs ;
}
/* end subroutine (subinfo_addrparse) */


static int subinfo_addrparseunix(sip,f)
struct subinfo	*sip ;
int		f ;
{
	int	rs = SR_OK ;
	int	pslen = -1 ;

	const char	*ps = sip->portspec ;

	char	tmpfname[MAXPATHLEN + 1] ;


	if (f)
	    ps += 5 ;

	if (ps[0] != '/') {
		f = TRUE ;
		pslen = mkpath2(tmpfname,sip->pr,ps) ;
		ps = tmpfname ;
	}

	if (f) {
	    rs = uc_mallocstrw(ps,pslen,&sip->a.portspec) ;
	    if (rs >= 0)
	        sip->portspec = sip->a.portspec ;
	}


	if (rs >= 0)
	    sip->af = AF_UNIX ;

ret0:
	return rs ;
}
/* end subroutine (subinfo_addrparseunix) */


static int subinfo_addrparseinet(sip)
struct subinfo	*sip ;
{
	INETADDRPARSE	a ;

	int	rs ;
	int	rs1 ;


	if ((rs = inetaddrparse_load(&a,sip->portspec,-1)) >= 0) {

	    if ((rs >= 0) && (a.af.sp != NULL) && a.af.sl) {
	        const char	**vpp = &sip->afspec ;

	        if ((rs = subinfo_setentry(sip,vpp,a.af.sp,a.af.sl)) >= 0) {
		    if (strcasecmp(sip->afspec,"inet") == 0) {
		        sip->af = AF_UNSPEC ;
		    } else {
		        rs1 = getaf(sip->afspec,-1) ;
		        if (rs1 >= 0)
		            sip->af = rs1 ;
		    }
	        }

	    } /* end if */

	    if ((rs >= 0) && (a.host.sp != NULL) && a.host.sl) {
		const char	**vpp = &sip->hostname ;
	        rs = subinfo_setentry(sip,vpp,a.host.sp,a.host.sl) ;
	    }

	    if ((rs >= 0) && (a.port.sp != NULL) && a.port.sl) {
		const char	**vpp = &sip->portspec ;
	        rs = subinfo_setentry(sip,vpp,a.port.sp,a.port.sl) ;
	    }

	    if (sip->af < 0)
	        sip->af = AF_INET4 ;

	} /* end if */

	return rs ;
}
/* end subroutine (subinfo_addrparseinet) */


