/* usd */

/* SYSDIALER "usd" dialer */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 2003-11-04, David A­D­ Morano
        This was created as one of the first dialer modules for the DIALER
        object.

*/

/* Copyright © 2003 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This is a SYSDIALER module.

	Synopsis:

	usd <path>

	Arguments:

	<path>		path to socket file


*******************************************************************************/


#define	USD_MASTER	0


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
#include	"usd.h"


/* local defines */

#define	USD_MNAME	"usd"
#define	USD_VERSION	"0"
#define	USD_INAME	""
#define	USD_MF1		(SYSDIALER_MFULL | SYSDIALER_MHALFOUT)
#define	USD_MF2		(SYSDIALER_MCOR | SYSDIALER_MCO)
#define	USD_MF3		(SYSDIALER_MHALFIN)
#define	USD_MF		(USD_MF1 | USD_MF2|USD_MF3)

#define	USD_SEARCHNAME	"usd"
#define	USD_VARPR	"LOCAL"
#define	USD_PR		"/usr/add-on/local"
#define	USD_LOGDNAME	"log"
#define	USD_LOGFNAME	SYSDIALER_LF

#ifndef	SVCNAMELEN
#define	SVCNAMELEN	32
#endif

#define	ARGBUFLEN	(MAXPATHLEN + 35)

#define	NPARG		2	/* number of positional arguments */
#define	MAXARGINDEX	100
#define	NARGPRESENT	(MAXARGINDEX/8 + 1)


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
extern int	dialusd(const char *,int,int) ;
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

extern char	*strwcpy(char *,const char *,int) ;


/* external variables */

extern char	**environ ;


/* local structures */

struct subinfo_flags {
	uint		ids:1 ;
	uint		userinfo:1 ;
	uint		svars:1 ;
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
	char		**argv ;
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
	const char	*helpfname ;
	const char	*logfname ;
	const char	*paramfname ;
	const char	*logid ;
	const char	*defprog ;
	USD		*op ;
	SYSDIALER_ARGS	*ap ;
	IDS		id ;
	VECSTR		aenvs ;
	VECSTR		stores ;
	VECSTR		defs ;
	VECSTR		pvars, exports ;
	EXPCOOK		cooks ;
	VECSTR		svars ;
	ENVS		xenvs ;
	USERINFO	u ;
	struct subinfo_allocs	a ;
	struct subinfo_flags	f ;
	uid_t		uid ;
	gid_t		gid ;
	int		argc ;
	int		argi ;
	int		ncpu ;
	int		af ;
	int		to ;
	char		userinfobuf[USERINFO_LEN + 1] ;
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

static int	subinfo_start(struct subinfo *,USD *,SYSDIALER_ARGS *,
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

SYSDIALER_INFO	usd = {
	USD_MNAME,
	USD_VERSION,
	USD_INAME,
	sizeof(USD),
	USD_MF
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


int usd_open(op,ap,hostname,svcname,av)
USD		*op ;
SYSDIALER_ARGS	*ap ;
const char	hostname[] ;
const char	svcname[] ;
const char	*av[] ;
{
	struct subinfo	si, *sip = &si ;
	int		rs ;
	int		rs1 ;
	int		opts = 0 ;
	const char	*portspec = NULL ;

	if (op == NULL)
	    return SR_FAULT ;

	if (hostname == NULL)
	    return SR_FAULT ;

	if (hostname[0] == '\0')
	    return SR_INVALID ;

	memset(op,0,sizeof(USD)) ;

#if	CF_DEBUGS
	{
	    int	i ;
	    debugprintf("usd_open: entered hostname=%s svcname=%s\n",
	        hostname,svcname) ;
	    if (ap->argv != NULL) {
	        for (i = 0 ; ap->argv[i] != NULL ; i += 1) {
		    debugprintf("usd_open: a%u=>%s<\n",i,ap->argv[i]) ;
	        }
	    }
	}
#endif /* CF_DEBUGS */

	rs = subinfo_start(sip,op,ap,hostname,svcname) ;
	if (rs < 0)
	    goto bad0 ;

	if (rs >= 0)
	    rs = subinfo_defaults(sip) ;

#if	CF_DEBUGS
	debugprintf("usd_open: subinfo_logfile()\n") ;
	debugprintf("usd_open: f_log=%u\n",op->f.log) ;
#endif

	if ((rs >= 0) && sip->f.log)
	    rs = subinfo_logfile(sip) ;

/* parse the port-specification if we have one */

	if (rs >= 0)
	    rs = subinfo_addrparse(sip) ;

#if	CF_DEBUGS
	debugprintf("usd_open: af=%u\n",sip->af) ;
	debugprintf("usd_open: hostname=%s\n",hostname) ;
	debugprintf("usd_open: portspec=%s\n",sip->portspec) ;
	debugprintf("usd_open: svcname=%s\n",svcname) ;
#endif

/* OK, do the dial */

	if (rs >= 0) {

	    switch (sip->af) {
	    case AF_UNIX:
		if (sip->portspec == NULL)
	     		rs = SR_NOENT ;
		if (rs >= 0)
	        	rs = dialusd(sip->portspec,
				sip->to,opts) ;
		break ;

	    case AF_UNSPEC:
	    case AF_INET4:
	    case AF_INET6:
	    	portspec = sip->portspec ;
		if (portspec == NULL) portspec = sip->svcname ;
		if (portspec == NULL) rs = SR_NOENT ;
		if (rs >= 0)
	        	rs = dialtcp(sip->hostname,portspec,sip->af,
				sip->to,opts) ;
		break ;
	    } /* end switch */

#if	CF_DEBUGS
	debugprintf("usd_open: dial() rs=%d\n",rs) ;
#endif

	} /* end if */
	op->fd = rs ;

	if (rs >= 0) {
	    op->magic = USD_MAGIC ;
	    uc_closeonexec(op->fd,TRUE) ;
	}

ret1:
bad1:
	rs1 = subinfo_finish(sip) ;
	if (rs >= 0) rs = rs1 ;

	if ((rs < 0) && (op->fd >= 0)) {
	    u_close(op->fd) ;
	    op->fd = -1 ;
	}

ret0:
bad0:

#if	CF_DEBUGS
	debugprintf("usd_open: ret rs=%d fd=%d\n",rs,op->fd) ;
#endif

	return (rs >= 0) ? op->fd : rs ;
}
/* end subroutine (usd_open) */


int usd_reade(op,buf,buflen,to,opts)
USD		*op ;
char		buf[] ;
int		buflen ;
int		to, opts ;
{
	int		rs ;

	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != USD_MAGIC)
	    return SR_NOTOPEN ;

	rs = uc_reade(op->fd,buf,buflen,to,opts) ;

	if (rs > 0)
	    op->tlen += rs ;

	return rs ;
}
/* end subroutine (usd_reade) */


int usd_recve(op,buf,buflen,flags,to,opts)
USD		*op ;
char		buf[] ;
int		buflen ;
int		flags ;
int		to, opts ;
{
	int		rs ;

	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != USD_MAGIC)
	    return SR_NOTOPEN ;

	rs = uc_recve(op->fd,buf,buflen,flags,to,opts) ;

	if (rs > 0)
	    op->tlen += rs ;

	return rs ;
}
/* end subroutine (usd_recve) */


int usd_recvfrome(op,buf,buflen,flags,sap,salenp,to,opts)
USD		*op ;
char		buf[] ;
int		buflen ;
int		flags ;
void		*sap ;
int		*salenp ;
int		to, opts ;
{
	int		rs ;

	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != USD_MAGIC)
	    return SR_NOTOPEN ;

	rs = uc_recvfrome(op->fd,buf,buflen,flags,sap,salenp,to,opts) ;

	if (rs > 0)
	    op->tlen += rs ;

	return rs ;
}
/* end subroutine (usd_recvfrome) */


int usd_recvmsge(op,msgp,flags,to,opts)
USD		*op ;
struct msghdr	*msgp ;
int		flags ;
int		to, opts ;
{
	int		rs ;

	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != USD_MAGIC)
	    return SR_NOTOPEN ;

	rs = uc_recvmsge(op->fd,msgp,flags,to,opts) ;

	if (rs > 0)
	    op->tlen += rs ;

	return rs ;
}
/* end subroutine (usd_recvmsge) */


int usd_write(op,buf,buflen)
USD		*op ;
const char	buf[] ;
int		buflen ;
{
	int		rs ;

	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != USD_MAGIC)
	    return SR_NOTOPEN ;

	rs = uc_writen(op->fd,((void *) buf),buflen) ;

	if (rs > 0)
	    op->tlen += rs ;

	return rs ;
}
/* end subroutine (usd_write) */


int usd_send(op,buf,buflen,flags)
USD		*op ;
const char	buf[] ;
int		buflen ;
int		flags ;
{
	int		rs ;

	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != USD_MAGIC)
	    return SR_NOTOPEN ;

	rs = u_send(op->fd,buf,buflen,flags) ;

	if (rs > 0)
	    op->tlen += rs ;

	return rs ;
}
/* end subroutine (usd_send) */


int usd_sendto(op,buf,buflen,flags,sap,salen)
USD		*op ;
const char	buf[] ;
int		buflen ;
int		flags ;
void		*sap ;
int		salen ;
{
	int		rs ;

	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != USD_MAGIC)
	    return SR_NOTOPEN ;

	rs = u_sendto(op->fd,buf,buflen,flags,sap,salen) ;

	if (rs > 0)
	    op->tlen += rs ;

	return rs ;
}
/* end subroutine (usd_sendto) */


int usd_sendmsg(op,msgp,flags)
USD		*op ;
struct msghdr	*msgp ;
int		flags ;
{
	int		rs ;

	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != USD_MAGIC)
	    return SR_NOTOPEN ;

	rs = u_sendmsg(op->fd,msgp,flags) ;

	if (rs > 0)
	    op->tlen += rs ;

	return rs ;
}
/* end subroutine (usd_sendmsg) */


/* shutdown */
int usd_shutdown(op,cmd)
USD		*op ;
int		cmd ;
{
	int		rs ;

	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != USD_MAGIC)
	    return SR_NOTOPEN ;

	rs = u_shutdown(op->fd,cmd) ;

	if (rs > 0)
	    op->tlen += rs ;

	return rs ;
}
/* end subroutine (usd_shutdown) */


/* close the connection */
int usd_close(op)
USD		*op ;
{
	int		rs ;

	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != USD_MAGIC)
	    return SR_NOTOPEN ;

	rs = u_close(op->fd) ;

	if (op->f.log) {

	    logfile_printf(&op->lh,"bytes=%u",op->tlen) ;

	    op->f.log = FALSE ;
	    logfile_close(&op->lh) ;

	} /* end if */

	op->magic = 0 ;
	return rs ;
}
/* end subroutine (usd_close) */


/* local subroutines */


static int subinfo_start(sip,op,ap,hostname,svcname)
struct subinfo	*sip ;
USD		*op ;
SYSDIALER_ARGS	*ap ;
const char	hostname[], svcname[] ;
{
	int		rs = SR_OK ;

	memset(sip,0,sizeof(struct subinfo)) ;

	sip->envv = (const char **) environ ;
	sip->op = op ;
	sip->ap = ap ;
	sip->pr = ap->pr ;
	sip->to = ap->timeout ;
	sip->hostname = hostname ;
	sip->svcname = svcname ;
	sip->af = -1 ;

	if ((rs = vecstr_start(&sip->stores,3,0)) >= 0) {
	    if ((rs = vecstr_start(&sip->aenvs,3,0)) >= 0) {
		if (ap != NULL) {
	    	    rs = subinfo_procargs(sip) ;
		}
		if (rs < 0)
		    vecstr_finish(&sip->aenvs) ;
	    } /* end if (aenvs) */
	    if (rs < 0)
		vecstr_finish(&sip->stores) ;
	} /* end if (stores) */

#if	CF_DEBUGS
	debugprintf("usd/subinfo_start: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (subinfo_start) */


static int subinfo_finish(sip)
struct subinfo	*sip ;
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (sip->f.ids) {
	    sip->f.ids = FALSE ;
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
	    sip->a.portspec = NULL ;
	}

	rs1 = vecstr_finish(&sip->aenvs) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = vecstr_finish(&sip->stores) ;
	if (rs >= 0) rs = rs1 ;

	return rs ;
}
/* end subroutine (subinfo_finish) */


static int subinfo_procargs(sip)
struct subinfo	*sip ;
{
	KEYOPT		akopts ;
	SYSDIALER_ARGS	*ap = sip->ap ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		argc ;
	int		argr, argl, aol, akl, avl, kwi ;
	int		ai, ai_max, ai_pos ;
	int		argvalue = -1 ;
	int		pan ;
	int		f_optminus, f_optplus, f_optequal ;
	int		f_doubledash = FALSE ;
	char		**argv ;
	char		*argp, *aop, *akp, *avp ;
	char		argpresent[NARGPRESENT] ;

#if	CF_DEBUGS
	debugprintf("usd/subinfo_procargs: arguments\n") ;
#endif

	argv = (char **) ap->argv ;

	for (argc = 0 ; argv[argc] != NULL ; argc += 1) ;

#if	CF_DEBUGS
	{
	    debugprintf("usd/subinfo_procargs: argc=%u\n",argc) ;
	    for (ai = 0 ; argv[ai] != NULL ; ai += 1)
	        debugprintf("usd/subinfo_procargs: argv[%u]=%s\n",
		ai,argv[ai]) ;
	}
#endif /* CF_DEBUGS */

	sip->argc = argc ;
	sip->argv = argv ;
	rs = keyopt_start(&akopts) ;
	if (rs < 0)
	    goto ret0 ;

/* process program arguments */

	for (ai = 0 ; ai < NARGPRESENT ; ai += 1)
	    argpresent[ai] = 0 ;

	pan = 0 ;
	ai = 0 ;
	ai_max = 0 ;
	ai_pos = 0 ;
	argr = (argc - 1) ;
	while ((rs >= 0) && (argr > 0)) {

	    argp = argv[++ai] ;
	    argr -= 1 ;
	    argl = strlen(argp) ;

	    f_optminus = (*argp == '-') ;
	    f_optplus = (*argp == '+') ;
	    if ((argl > 1) && (f_optminus || f_optplus)) {

	        if (isdigit(argp[1])) {

	            rs = cfdeci((argp + 1),(argl - 1),&argvalue) ;

	        } else if ((argl == 2) && (argp[1] == '-')) {

	            f_doubledash = TRUE ;
	            ai += 1 ;
	            argr -= 1 ;

	        } else {

#if	CF_DEBUGS
	            debugprintf("usd/subinfo_procargs: option? ao=>%t<\n",
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
	            debugprintf("usd/subinfo_procargs: k=>%t<\n",akp,akl) ;
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

	                        if (argv[ai + 1] == NULL) {
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

	                        if (argv[ai + 1] == NULL) {
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
	                            sip->logfname = avp ;

	                    } else {

	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }

	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;

	                        if (argl)
	                            sip->logfname = argp ;

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

	                        if (argv[ai + 1] == NULL) {
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

	                        if (argv[ai + 1] == NULL) {
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

	                        if (argv[ai + 1] == NULL) {
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

	                    switch ((int) *akp) {

/* address-family */
	                    case 'f':
	                        if (argv[ai + 1] == NULL) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }

	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;

#if	CF_DEBUGS
				debugprintf("usd/subinfo_procargs: "
				"al=%u arg=>%t<\n",argl,argp,argl) ;
#endif

	                        if (argl)
	                            sip->afspec = argp ;

	                        break ;

	                    case 'i':
	                        sip->f.ignore = TRUE ;
	                        break ;

/* options */
	                    case 'o':
	                        if (argv[ai + 1] == NULL) {
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
	                        if (argv[ai + 1] == NULL) {
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
	                        if (argv[ai + 1] == NULL) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }

	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;

	                        if (argl)
	                            rs = cfdecti(argp,argl,&sip->to) ;

	                        break ;

/* eXported environment */
	                    case 'x':
	                        if (argv[ai + 1] == NULL) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }

	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;

	                        if (argl)
	                            rs = vecstr_envadds(&sip->aenvs,
	                                argp,argl) ;

	                        break ;

	                    default:
	                        rs = SR_INVALID ;

	                    } /* end switch */

	                    akp += 1 ;
	                    if (rs < 0)
	                        break ;

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
	debugprintf("usd/subinfo_procargs: portspec=%s\n",sip->portspec) ;
	debugprintf("usd/subinfo_procargs: argi=%u\n",sip->argi) ;
#endif

	if (rs >= 0)
	    rs = subinfo_procopts(sip,&akopts) ;

/* process any address-family specification */

#if	CF_DEBUGS
	debugprintf("usd/subinfo_procargs: af=%d afspec=%s\n",
		sip->af, sip->afspec) ;
#endif

	if ((rs >= 0) && (sip->af < 0) && 
		(sip->afspec != NULL) && (sip->afspec[0] != '\0')) {

#if	CF_DEBUGS
	debugprintf("usd/subinfo_procargs: afspec=%s\n",sip->afspec) ;
#endif

	    rs1 = getaf(sip->afspec,-1) ;
	    if (rs1 >= 0)
		sip->af = rs1 ;

#if	CF_DEBUGS
	debugprintf("usd/subinfo_procargs: getaf() rs=%d\n",rs1) ;
#endif

	} /* end if */

/* done and out */
ret1:
	keyopt_finish(&akopts) ;

ret0:

#if	CF_DEBUGS
	debugprintf("usd/subinfo_procargs: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (subinfo_procargs) */


static int subinfo_procopts(sip,kop)
struct subinfo	*sip ;
KEYOPT		*kop ;
{
	KEYOPT_CUR	kcur ;
	int		rs ;
	int		oi ;
	int		kl, vl ;
	int		c = 0 ;
	const char	*kp, *vp ;

	if ((rs = keyopt_curbegin(kop,&kcur)) >= 0) {

	while ((kl = keyopt_enumkeys(kop,&kcur,&kp)) >= 0) {

/* get the first value for this key */

	    vl = keyopt_fetch(kop,kp,NULL,&vp) ;

/* do we support this option? */

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


static int subinfo_setentry(sip,epp,v,vlen)
struct subinfo	*sip ;
const char	**epp ;
const char	v[] ;
int		vlen ;
{
	int		rs ;
	int		oi, i ;
	int		vnlen = 0 ;
	const char	*cp ;

	if (sip == NULL)
	    return SR_FAULT ;

	if (epp == NULL)
	    return SR_INVALID ;

/* find existing entry for later deletion */

	oi = -1 ;
	if (*epp != NULL) {
	    for (i = 0 ; vecstr_get(&sip->stores,i,&cp) >= 0 ; i += 1) {
	        if (cp != NULL) {
	            if (*epp == cp) {
	                oi = i ;
	                break ;
	            }
		}
	    } /* end for */
	} /* end if (had an existing entry) */

/* add the new entry */

	if (v != NULL) {

	    vnlen = strnlen(v,vlen) ;

	    if ((rs = vecstr_add(&sip->stores,v,vnlen)) >= 0) {
	        i = rs ;
	        if ((rs = vecstr_get(&sip->stores,i,&cp)) >= 0) {
	            *epp = cp ;
		}
	    } /* end if (added new entry) */

	} /* end if (had a new entry) */

/* delete the old entry if we had one */

	if ((rs >= 0) && (oi >= 0))
	    vecstr_del(&sip->stores,oi) ;

	return (rs >= 0) ? vnlen : rs ;
}
/* end subroutine (subinfo_setentry) */


static int subinfo_defaults(sip)
struct subinfo	*sip ;
{
	SYSDIALER_ARGS	*ap = sip->ap ;
	int		rs = SR_OK ;
	int		rs1 ;
	const char	*vp ;

#if	CF_DEBUGS
	debugprintf("usd/subinfo_defaults: entered af=%d\n",sip->af) ;
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

	    if ((vp = getenv(USD_VARPR)) != NULL) {

	        rs1 = subinfo_dirok(sip,vp,-1) ;
	        if (rs1 > 0)
	            sip->pr = vp ;

	    }

	} /* end if */

	if ((rs >= 0) && (sip->pr == NULL)) {

	    vp = USD_PR ;
	    rs1 = subinfo_dirok(sip,vp,-1) ;
	    if (rs1 > 0)
	        sip->pr = vp ;

	} /* end if */

	if (sip->pr == NULL)
	    sip->pr = ap->pr ;

/* search-name */

	if (sip->searchname == NULL)
	    sip->searchname = USD_SEARCHNAME ;

/* log-file */

	if ((rs >= 0) && (sip->logfname == NULL))
	    sip->logfname = USD_LOGFNAME ;

#if	CF_DEBUGS
	debugprintf("usd/subinfo_defaults: pr=%s\n",sip->pr) ;
	debugprintf("usd/subinfo_defaults: logfname=%s\n",sip->logfname) ;
#endif

/* address family */

#if	CF_DEBUGS
	debugprintf("usd/subinfo_defaults: af=%d\n",sip->af) ;
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
	USERINFO	*uip = &sip->u ;
	int		rs = SR_OK ;

	if (sip->f.userinfo)
	    goto ret0 ;

	sip->f.userinfo = TRUE ;
	rs = userinfo(uip,sip->userinfobuf,USERINFO_LEN,NULL) ;
	if (rs < 0)
	    goto bad0 ;

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

ret0:
bad0:
	return rs ;
}
/* end subroutine (subinfo_userinfo) */


static int subinfo_logfile(sip)
struct subinfo	*sip ;
{
	USD		*op = sip->op ;
	int		rs = SR_OK ;
	int		rs1 ;
	const char	*lnp ;
	const char	*lidp = NULL ;
	char		logfname[MAXPATHLEN + 1] ;

	if (! sip->f.log)
	    goto ret0 ;

	sip->f.log = TRUE ;
	lnp = sip->logfname ;
	if (lnp[0] != '/') {
	    rs = mkpath3(logfname,sip->pr,USD_LOGDNAME,lnp) ;
	    lnp = logfname ;
	}

#if	CF_DEBUGS
	debugprintf("usd/subinfo_logfile: lnp=%s\n",lnp) ;
#endif

	if (rs < 0)
	    goto ret0 ;

	    rs1 = logfile_open(&op->lh,lnp,0,0666,lidp) ;
	    op->f.log = (rs1 >= 0) ;

#if	CF_DEBUGS
	debugprintf("usd/subinfo_logfile: logfile_open() rs=%d\n",rs1) ;
#endif

	    if (rs1 >= 0) {
		USERINFO	*uip = &sip->u ;

		if (! sip->f.userinfo)
		     subinfo_userinfo(sip) ;

		if (sip->f.userinfo) {

	            logfile_userinfo(&op->lh,uip,0L,
	                USD_MNAME,USD_VERSION) ;

	            logfile_printf(&op->lh,"pid=%d",uip->pid) ;

		}

	        logfile_printf(&op->lh,"pr=%s",
	            sip->pr) ;

	        logfile_printf(&op->lh,"host=%s",
	            sip->hostname) ;

	        logfile_printf(&op->lh,"svc=%s",
	            sip->svcname) ;

	    } /* end if (opened logfile) */

ret0:
	return rs ;
}
/* end subroutine (subinfo_logfile) */


static int subinfo_dirok(sip,d,dlen)
struct subinfo	*sip ;
const char	d[] ;
int		dlen ;
{
	struct ustat	sb ;
	NULSTR		ss ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		f = FALSE ;
	const char	*dnp ;

	if (! sip->f.ids) {
	    sip->f.ids = TRUE ;
	    rs = ids_load(&sip->id) ;
	}

	if (rs >= 0) {
	    if ((rs = nulstr_start(&ss,d,dlen,&dnp)) >= 0) {

	rs1 = u_stat(dnp,&sb) ;

	if ((rs1 >= 0) && S_ISDIR(sb.st_mode)) {

	    rs1 = sperm(&sip->id,&sb,(R_OK | X_OK)) ;
	    f = (rs1 >= 0) ;

	} /* end if */

	nulstr_finish(&ss) ;
	} /* end if */
	}

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (subinfo_dirok) */


static int subinfo_addrparse(sip)
struct subinfo	*sip ;
{
	int		rs = SR_OK ;

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
	int		rs = SR_OK ;
	int		pslen = -1 ;
	const char	*ps = sip->portspec ;
	char		tmpfname[MAXPATHLEN + 1] ;

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
	int		rs ;
	int		rs1 ;

	if ((rs = inetaddrparse_load(&a,sip->portspec,-1)) >= 0) {

	if ((rs >= 0) && (a.af.sp != NULL) && a.af.sl) {

	    rs = subinfo_setentry(sip,&sip->afspec,a.af.sp,a.af.sl) ;
	    if (rs >= 0) {
		if (strcasecmp(sip->afspec,"inet") == 0) {
		    sip->af = AF_UNSPEC ;
		} else {
		    rs1 = getaf(sip->afspec,-1) ;
		    if (rs1 >= 0)
		        sip->af = rs1 ;
		}
	    }

	} /* end if */

	if ((rs >= 0) && (a.host.sp != NULL) && a.host.sl)
	    rs = subinfo_setentry(sip,&sip->hostname,a.host.sp,a.host.sl) ;

	if ((rs >= 0) && (a.port.sp != NULL) && a.port.sl)
	    rs = subinfo_setentry(sip,&sip->portspec,a.port.sp,a.port.sl) ;

	if (sip->af < 0)
	    sip->af = AF_INET4 ;

	} /* end if */

	return rs ;
}
/* end subroutine (subinfo_addrparseinet) */


