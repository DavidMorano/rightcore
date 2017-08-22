/* uux */

/* SYSDIALER "uux" dialer */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_SOCKET	0		/* use socket or pipe */


/* revision history:

	= 2003-11-04, David A­D­ Morano
	This subroutine was adopted for use from the DWD program.

*/

/* Copyright © 2003 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This is a SYSDIALER module.

	Synopsis:

	uux [-R <pr>] [<node>!<svc>] [<arg(s)>]

	Arguments:

	-R <pr>		program-root
	<node>		UU nodename
	<svc>		service
	<arg(s)>	dialer arguments


*******************************************************************************/


#define	UUX_MASTER	0


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>
#include	<netdb.h>

#include	<vsystem.h>
#include	<baops.h>
#include	<vecstr.h>
#include	<vechand.h>
#include	<ids.h>
#include	<keyopt.h>
#include	<logfile.h>
#include	<getxusername.h>
#include	<userinfo.h>
#include	<localmisc.h>

#include	"uux.h"
#include	"sysdialer.h"
#include	"nulstr.h"


/* local defines */

#define	UUX_MNAME	"uux"
#define	UUX_VERSION	"0"
#define	UUX_INAME	""
#define	UUX_MF1		(SYSDIALER_MHALFOUT)
#define	UUX_MF2		(SYSDIALER_MCOR | SYSDIALER_MCO)
#define	UUX_MF3		(SYSDIALER_MARGS)
#define	UUX_MF		(UUX_MF1 | UUX_MF2 | UUX_MF3)

#define	UUX_SEARCHNAME	"uux"
#define	UUX_VARPR	"NCMP"
#define	UUX_PR		"/usr/add-on/ncmp"
#define	UUX_LOGDNAME	"log"
#define	UUX_LOGFNAME	"sysdialer"

#ifndef	NOFILE
#define	NOFILE		20
#endif

#define	ARGBUFLEN	(MAXPATHLEN + 35)
#define	BUFLEN		((2 * MAXPATHLEN) + 3)
#define	PATHBUFLEN	((4 * MAXPATHLEN) + 3)

#define	MAXARGINDEX	100
#define	NARGPRESENT	(MAXARGINDEX/8 + 1)

#ifndef	VARPRAST
#define	VARPRAST	"AST"
#endif

#ifndef	VARPATH
#define	VARPATH		"PATH"
#endif



/* external subroutines */

extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	sfshrink(const char *,int,const char **) ;
extern int	sfbasename(const char *,int,const char **) ;
extern int	sfdirname(const char *,int,const char **) ;
extern int	nextfield(const char *,int,const char **) ;
extern int	matstr(const char **,const char *,int) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	vstrkeycmp(const char **,const char **) ;
extern int	pathclean(char *,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	optbool(const char *,int) ;
extern int	vecstr_envadd(vecstr *,const char *,const char *,int) ;
extern int	pathclean(char *,const char *,int) ;
extern int	perm(const char *,uid_t,gid_t,gid_t *,int) ;
extern int	sperm(IDS *,struct ustat *,int) ;
extern int	getnodedomain(char *,char *) ;
extern int	mkpr(char *,int,const char *,const char *) ;
extern int	logfile_userinfo(LOGFILE *,USERINFO *,time_t,
			const char *,const char *) ;
extern int	isdigitlatin(int) ;

extern int	dialuux(const char *,const char *,const char *,const char **,
			const char *,const char *,int) ;

extern char	*strwcpy(char *,const char *,int) ;


/* external variables */


/* local structures */

struct subinfo_flags {
	uint		ids:1 ;
	uint		noreport:1 ;
	uint		queueonly:1 ;
	uint		log:1 ;
} ;

struct subinfo_allocs {
	const char	*node ;
	const char	*svc ;
	const char	*pr ;
} ;

struct subinfo {
	char		**args ;
	const char	*pr ;
	const char	*prn ;
	const char	*searchname ;
	const char	*hostname ;
	const char	*svcname ;
	const char	*node ;
	const char	*logfname ;
	const char	*username ;
	const char	*grade ;
	const char	**av ;
	const char	**dav ;		/* allocated */
	UUX		*op ;
	SYSDIALER_ARGS	*ap ;
	struct subinfo_allocs	a ;
	struct subinfo_flags	f ;
	IDS		id ;
} ;


/* forward references */

static int	subinfo_start(struct subinfo *,UUX *,SYSDIALER_ARGS *,
			const char *,const char *,const char **) ;
static int	subinfo_procargs(struct subinfo *) ;
static int	subinfo_procspec(struct subinfo *,const char *) ;
static int	subinfo_procopts(struct subinfo *,KEYOPT *) ;
static int	subinfo_defaults(struct subinfo *) ;
static int	subinfo_logfile(struct subinfo *) ;
static int	subinfo_mkargs(struct subinfo *,const char **,const char ***) ;
static int	subinfo_finish(struct subinfo *) ;
static int	subinfo_dirok(struct subinfo *,const char *,int) ;


/* global variables (module information) */

SYSDIALER_INFO	uux = {
	UUX_MNAME,
	UUX_VERSION,
	UUX_INAME,
	sizeof(UUX),
	UUX_MF
} ;


/* local variables */

static const char *argopts[] = {
	"ROOT",
	"RN",
	"sn",
	"lf",
	NULL
} ;

enum argopts {
	argopt_root,
	argopt_rn,
	argopt_sn,
	argopt_lf,
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


int uux_open(op,ap,hostname,svcname,av)
UUX		*op ;
SYSDIALER_ARGS	*ap ;
const char	hostname[] ;
const char	svcname[] ;
const char	*av[] ;
{
	struct subinfo	si, *sip = &si ;

	int	rs ;
	int	opts = 0 ;


	if (op == NULL) return SR_FAULT ;
	if (ap == NULL) return SR_FAULT ;
	if (hostname == NULL) return SR_FAULT ;

	memset(op,0,sizeof(UUX)) ;

	if ((rs = subinfo_start(sip,op,ap,hostname,svcname,av)) >= 0) {

#if	CF_DEBUGS
	debugprintf("uux_open: hostname=%s svcname=%s\n",
	    hostname,svcname) ;
#endif

	if (ap != NULL)
	    rs = subinfo_procargs(sip) ;

	if (rs >= 0)
	    rs = subinfo_defaults(sip) ;

	if (rs >= 0)
	    rs = subinfo_logfile(sip) ;

	if (rs >= 0) {
	    const char	**dav = NULL ;
	    if ((rs = subinfo_mkargs(sip,av,&dav)) >= 0) {
	        const char	*pr = sip->pr ;
	        const char	*nn = sip->node ;
	        const char	*svc = sip->svcname ;
		const char	*un = sip->username ;
		const char	*grade = sip->grade ;

#if	CF_DEBUGS
	    debugprintf("uux_open: pr=%s\n",sip->pr) ;
	    debugprintf("uux_open: hostname=%s\n",hostname) ;
	    debugprintf("uux_open: node=%s\n",sip->node) ;
	    debugprintf("uux_open: svc=%s\n",sip->svcname) ;
	    debugprintf("uux_open: username=%s\n",sip->username) ;
	    debugprintf("uux_open: grade=%s\n",sip->grade) ;
#endif

	    rs = dialuux(pr,nn,svc,dav,un,grade,opts) ;
	    op->fd = rs ;

#if	CF_DEBUGS
	    debugprintf("uux_open: dialuux() rs=%d\n",rs) ;
#endif
	if (rs >= 0)
	    op->magic = UUX_MAGIC ;

	} /* end if (mkargs) */
	} /* end if (ok) */

	if ((rs < 0) && op->f.log) {
	    op->f.log = FALSE ;
	    logfile_printf(&op->lh,"failed (%d)",rs) ;
	    logfile_close(&op->lh) ;
	} /* end if */

	subinfo_finish(sip) ;
	} /* end if (subinfo) */

#if	CF_DEBUGS
	debugprintf("uux_open: ret rs=%d fd=%d\n",rs,op->fd) ;
#endif

	return (rs >= 0) ? op->fd : rs ;
}
/* end subroutine (uux_open) */


/* close the connection */
int uux_close(op)
UUX		*op ;
{
	int	rs = SR_OK ;
	int	rs1 ;


	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != UUX_MAGIC)
	    return SR_NOTOPEN ;

	rs1 = u_close(op->fd) ;
	if (rs >= 0) rs = rs1 ;

	if (op->f.log) {
	    op->f.log = FALSE ;
	    logfile_printf(&op->lh,"bytes=%u",op->tlen) ;
	    rs1 = logfile_close(&op->lh) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if */

	op->magic = 0 ;
	return rs ;
}
/* end subroutine (uux_close) */


int uux_reade(op,buf,buflen,to,opts)
UUX		*op ;
char		buf[] ;
int		buflen ;
int		to, opts ;
{
	int	rs ;


	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != UUX_MAGIC)
	    return SR_NOTOPEN ;

#ifdef	COMMENT
	rs = uc_reade(op->fd,buf,buflen,to,opts) ;
#else
	rs = SR_BADF ;
#endif

	return rs ;
}
/* end subroutine (uux_reade) */


int uux_recve(op,buf,buflen,flags,to,opts)
UUX		*op ;
char		buf[] ;
int		buflen ;
int		flags ;
int		to, opts ;
{
	int	rs ;


	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != UUX_MAGIC)
	    return SR_NOTOPEN ;

#ifdef	COMMENT
	rs = uc_recve(op->fd,buf,buflen,flags,to,opts) ;
#else
	rs = SR_BADF ;
#endif

	return rs ;
}
/* end subroutine (uux_recve) */


int uux_recvfrome(op,buf,buflen,flags,sap,salenp,to,opts)
UUX		*op ;
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

	if (op->magic != UUX_MAGIC)
	    return SR_NOTOPEN ;

#ifdef	COMMENT
	rs = uc_recvfrome(op->fd,buf,buflen,flags,sap,salenp,to,opts) ;
#else
	rs = SR_BADF ;
#endif

	return rs ;
}
/* end subroutine (uux_recvfrome) */


int uux_recvmsge(op,msgp,flags,to,opts)
UUX		*op ;
struct msghdr	*msgp ;
int		flags ;
int		to, opts ;
{
	int	rs ;


	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != UUX_MAGIC)
	    return SR_NOTOPEN ;

#ifdef	COMMENT
	rs = uc_recvmsge(op->fd,msgp,flags,to,opts) ;
#else
	rs = SR_BADF ;
#endif

	return rs ;
}
/* end subroutine (uux_recvmsge) */


int uux_write(op,buf,buflen)
UUX		*op ;
const char	buf[] ;
int		buflen ;
{
	int	rs ;


	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != UUX_MAGIC)
	    return SR_NOTOPEN ;

	rs = uc_writen(op->fd,buf,buflen) ;

	if (rs >= 0)
	    op->tlen += rs ;

	return rs ;
}
/* end subroutine (uux_write) */


int uux_send(op,buf,buflen,flags)
UUX		*op ;
const char	buf[] ;
int		buflen ;
int		flags ;
{
	int	rs ;


	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != UUX_MAGIC)
	    return SR_NOTOPEN ;

	rs = u_send(op->fd,buf,buflen,flags) ;

	if (rs >= 0)
	    op->tlen += rs ;

	return rs ;
}
/* end subroutine (uux_send) */


int uux_sendto(op,buf,buflen,flags,sap,salen)
UUX		*op ;
const char	buf[] ;
int		buflen ;
int		flags ;
void		*sap ;
int		salen ;
{
	int	rs ;


	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != UUX_MAGIC)
	    return SR_NOTOPEN ;

	rs = u_sendto(op->fd,buf,buflen,flags,sap,salen) ;

	if (rs >= 0)
	    op->tlen += rs ;

	return rs ;
}
/* end subroutine (uux_sendto) */


int uux_sendmsg(op,msgp,flags)
UUX		*op ;
struct msghdr	*msgp ;
int		flags ;
{
	int	rs ;


	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != UUX_MAGIC)
	    return SR_NOTOPEN ;

	rs = u_sendmsg(op->fd,msgp,flags) ;

	if (rs >= 0)
	    op->tlen += rs ;

	return rs ;
}
/* end subroutine (uux_sendmsg) */


int uux_shutdown(op,cmd)
UUX		*op ;
int		cmd ;
{
	int	rs ;


	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != UUX_MAGIC)
	    return SR_NOTOPEN ;

	rs = u_shutdown(op->fd,cmd) ;

	return rs ;
}
/* end subroutine (uux_shutdown) */


/* private subroutines */


static int subinfo_start(sip,op,ap,hostname,svcname,av)
struct subinfo	*sip ;
UUX		*op ;
SYSDIALER_ARGS	*ap ;
const char	hostname[] ;
const char	svcname[] ;
const char	*av[] ;
{
	int	rs = SR_OK ;


	memset(sip,0,sizeof(struct subinfo)) ;
	sip->op = op ;
	sip->ap = ap ;
	sip->node = hostname ;
	sip->hostname = hostname ;
	sip->svcname = svcname ;
	sip->av = av ;

	return rs ;
}
/* end subroutine (subinfo_start) */


static int subinfo_finish(sip)
struct subinfo	*sip ;
{
	int	rs = SR_OK ;
	int	rs1 ;


	if (sip->dav != NULL) {
	    rs1 = uc_free(sip->dav) ;
	    if (rs >= 0) rs = rs1 ;
	    sip->dav = NULL ;
	}

	if (sip->f.ids) {
	    sip->f.ids = FALSE ;
	    rs1 = ids_release(&sip->id) ;
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

	sip->op = NULL ;
	sip->ap = NULL ;
	return rs ;
}
/* end subroutine (subinfo_finish) */


static int subinfo_procargs(sip)
struct subinfo	*sip ;
{
	KEYOPT		akopts ;

	SYSDIALER_ARGS	*ap ;

	int	argr, argl, aol, akl, avl, kwi ;
	int	ai, ai_max, ai_pos ;
	int	argvalue = -1 ;
	int	argc ;
	int	pan ;
	int	rs ;
	int	f_optminus, f_optplus, f_optequal ;
	int	f ;

	char	**argv, *argp, *aop, *akp, *avp ;
	char	argpresent[NARGPRESENT] ;
	char	*cp ;


	ap = sip->ap ;

#if	CF_DEBUGS
	debugprintf("uux_open: arguments\n") ;
#endif

	argv = (char **) ap->argv ;

	for (argc = 0 ; argv[argc] != NULL ; argc += 1) ;

#if	CF_DEBUGS
	{
	    debugprintf("uux_open: argr=%u\n",argr) ;
	    for (ai = 0 ; argv[ai] != NULL ; ai += 1)
	        debugprintf("uux_open: argv[%u]=%s\n",ai,argv[ai]) ;
	    debugprintf("uux_open: ai=%u\n",ai) ;
	}
#endif

	rs = keyopt_start(&akopts) ;
	if (rs < 0)
	    goto ret0 ;

/* process program arguments */

	for (ai = 0 ; ai < NARGPRESENT ; ai += 1)
	    argpresent[ai] = 0 ;

	ai = 0 ;
	ai_max = 0 ;
	ai_pos = 0 ;
	argr = argc - 1 ;
	while ((rs >= 0) && (argr > 0)) {

#if	CF_DEBUGS
	    debugprintf("uux_open: AL argr=%u\n",argr) ;
	    debugprintf("uux_open: AL argv[%u]=%s\n",ai,argv[ai]) ;
#endif

	    argp = argv[++ai] ;
	    argr -= 1 ;
	    argl = strlen(argp) ;

	    f_optminus = (*argp == '-') ;
	    f_optplus = (*argp == '+') ;
	    if ((argl > 1) && (f_optminus || f_optplus)) {
		const int	ach = MKCHAR(argp[1]) ;

	        if (isdigitlatin(ach)) {

	            rs = cfdeci((argp + 1),(argl - 1),&argvalue) ;

	        } else if (ach == '-') {

	            ai_pos = ai ;
	            break ;

	        } else {

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

/* keyword match or only key letters? */

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

/* handle all keyword defaults */
	                default:
	                    rs = SR_INVALID ;

	                } /* end switch */

	            } else {

	                while (akl--) {

	                    switch ((int) *akp) {

/* program-root */
	                    case 'R':
	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }

	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;

	                        if (argl)
	                            sip->pr = argp ;

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

/* username */
	                    case 'u':
	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }

	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;

	                        if (argl)
	                            sip->username = argp ;

	                        break ;

/* grade */
	                    case 'g':
	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }

	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;

	                        if (argl)
	                            sip->username = argp ;

	                        break ;

	                    case 'r':
	                        sip->f.queueonly = TRUE ;
	                        break ;

	                    case 'n':
	                        sip->f.noreport = TRUE ;
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

	        if (ai >= MAXARGINDEX)
	            break ;

	        BASET(argpresent,ai) ;
	        ai_max = ai ;

	    } /* end if (key letter/word or positional) */

	    ai_pos = ai ;

	} /* end while (all command line argument processing) */

	if (rs < 0)
	    goto badarg ;

	pan = 0 ;

	for (ai = 1 ; ai < argc ; ai += 1) {

	    f = (ai <= ai_max) && BATST(argpresent,ai) ;
	    f = f || (ai > ai_pos) ;
	    if (! f) continue ;

	    cp = argv[ai] ;
	    switch (pan++) {
	    case 0:
	        rs = subinfo_procspec(sip,cp) ;
	        break ;
	    case 1:
	        sip->args = (argv + ai) ;
	        break ;
	    default:
	        break ;
	    } /* end switch */

	    if (rs < 0) break ;
	} /* end for */

	if (rs >= 0)
	    rs = subinfo_procopts(sip,&akopts) ;

ret1:
	keyopt_finish(&akopts) ;

ret0:
	return rs ;

/* bad stuff */
badarg:
	goto ret1 ;
}
/* end subroutine (subinfo_procargs) */


static int subinfo_procopts(sip,kop)
struct subinfo	*sip ;
KEYOPT		*kop ;
{
	KEYOPT_CUR	kcur ;

	int	rs ;
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


static int subinfo_procspec(sip,spec)
struct subinfo	*sip ;
const char	spec[] ;
{
	int	rs = SR_OK ;
	int	cl ;

	const char	*tp ;
	const char	*cp ;


	if (spec[0] == '\0')
	    goto ret0 ;

	if ((tp = strchr(spec,'!')) != NULL) {

	    cp = spec ;
	    cl = (tp - spec) ;
	    if (cl > 0) {

	        rs = uc_mallocstrw(cp,cl,&sip->a.node) ;

	        if (rs >= 0)
	            sip->node = sip->a.node ;

	    } /* end if */

	    cp = (tp + 1) ;
	    if (cp[0] != '\0')
		sip->svcname = cp ;

	} else
	    sip->svcname = spec ;

ret0:
	return rs ;
}
/* end subroutine (subinfo_procspec) */


static int subinfo_defaults(sip)
struct subinfo	*sip ;
{
	SYSDIALER_ARGS	*ap ;

	int	rs = SR_OK ;
	int	rs1 ;

	const char	*vp ;


	ap = sip->ap ;

/* program-root */

	if ((sip->pr == NULL) && (sip->prn != NULL) && (sip->prn[0] != '\0')) {
	    char	domainname[MAXHOSTNAMELEN + 1] ;
	    char	pr[MAXPATHLEN + 1] ;

	    rs1 = getnodedomain(NULL,domainname) ;

	    if (rs1 >= 0)
	        rs1 = mkpr(pr,MAXPATHLEN,sip->prn,domainname) ;

	    if (rs1 >= 0) {

	        rs = subinfo_dirok(sip,pr,rs1) ;

	        if (rs > 0)
	            rs = uc_mallocstrw(pr,rs1,&sip->a.pr) ;

	        if (rs > 0)
	            sip->pr = sip->a.pr ;

	    } /* end if */

	} /* end if */

	if ((rs >= 0) && (sip->pr == NULL)) {

	    if ((vp = getenv(UUX_VARPR)) != NULL) {

	        rs = subinfo_dirok(sip,vp,-1) ;
	        if (rs > 0)
	            sip->pr = vp ;

	    }

	} /* end if */

	if ((rs >= 0) && (sip->pr == NULL)) {

	    vp = UUX_PR ;
	    rs = subinfo_dirok(sip,vp,-1) ;
	    if (rs > 0)
	        sip->pr = vp ;

	} /* end if */

	if (sip->pr == NULL)
	    sip->pr = ap->pr ;

/* search-name */

	if (sip->searchname == NULL)
	    sip->searchname = UUX_SEARCHNAME ;

/* log-file */

	if ((rs >= 0) && (sip->logfname == NULL))
		sip->logfname = UUX_LOGFNAME ;

/* out of here */

	return rs ;
}
/* end subroutine (subinfo_defaults) */


static int subinfo_logfile(sip)
struct subinfo	*sip ;
{
	UUX	*op = sip->op ;

	int	rs = SR_OK ;
	int	rs1 ;

	const char	*lnp ;
	const char	*lidp = NULL ;

	char	logfname[MAXPATHLEN + 1] ;


	if (! sip->f.log)
	    goto ret0 ;

	sip->f.log = TRUE ;
	lnp = sip->logfname ;
	if (lnp[0] != '/') {
	    rs = mkpath3(logfname,sip->pr,UUX_LOGDNAME,lnp) ;
	    lnp = logfname ;
	}

	if (rs < 0)
	    goto ret0 ;

	    if ((rs1 = logfile_open(&op->lh,lnp,0,0666,lidp)) >= 0) {
	        USERINFO	u, *uip = &u ;
	        char	userbuf[USERINFO_LEN + 1] ;
	        op->f.log = TRUE ;

	        if ((rs1 = userinfo(uip,userbuf,USERINFO_LEN,NULL)) >= 0) {

	            logfile_userinfo(&op->lh,uip,0L,
	                UUX_MNAME,UUX_VERSION) ;

	            logfile_printf(&op->lh,"pid=%d",uip->pid) ;

	        } else {

	            if ((rs1 = getusername(logfname,MAXPATHLEN,-1)) >= 0) {
	        	pid_t	pid = getpid() ;

	                logfile_printf(&op->lh,"username=%s",
	                    logfname) ;

	                logfile_printf(&op->lh,"pid=%d",pid) ;

	            }

	        } /* end if (userinfo) */

	        logfile_printf(&op->lh,"pr=%s",
	            sip->pr) ;

	        logfile_printf(&op->lh,"host=%s",
	            sip->hostname) ;

	        logfile_printf(&op->lh,"node=%s",
	            sip->node) ;

	        logfile_printf(&op->lh,"svc=%s",
	            sip->svcname) ;

	    } /* end if (opened logfile) */

ret0:
	return rs ;
}
/* end subroutine (subinfo_logfile) */


static int subinfo_mkargs(sip,av,davp)
struct subinfo	*sip ;
const char	*av[] ;
const char	***davp ;
{
	int	rs = SR_OK ;
	int	i ;
	int	size ;
	int	n = 0 ;
	const char	**dav ;


	*davp = NULL ;
	if (sip->args != NULL) {
	    for (i = 0 ; sip->args[i] != NULL ; i += 1)
	        n += 1 ;
	}

	if (av != NULL) {
	    for (i = 0 ; av[i] != NULL ; i += 1)
	        n += 1 ;
	}

	size = (n + 1) * sizeof(const char *) ;
	if ((rs = uc_malloc(size,&dav)) >= 0) {
	    int	j = 0 ;
	    sip->dav = dav ;
	    *davp = dav ;

	    if (sip->args != NULL) {
	        for (i = 0 ; sip->args[i] != NULL ; i += 1)
	            dav[j++] = sip->args[i] ;
	    }

	    if (av != NULL) {
	        for (i = 0 ; av[i] != NULL ; i += 1)
	            dav[j++] = (char *) av[i] ;
	    }

	    dav[j] = NULL ;

	} /* end if */

	return (rs >= 0) ? n : rs ;
}
/* end subroutine (subinfo_mkargs) */


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


	if (! sip->f.ids) {
	    sip->f.ids = TRUE ;
	    rs = ids_load(&sip->id) ;
	}

	if (rs >= 0)
	    rs = nulstr_start(&ss,d,dlen,&dnp) ;

	if (rs < 0)
	    goto ret0 ;

	rs1 = u_stat(dnp,&sb) ;

	if ((rs1 >= 0) && S_ISDIR(sb.st_mode)) {

	    rs1 = sperm(&sip->id,&sb,(R_OK | X_OK)) ;
	    f = (rs1 >= 0) ;

	} /* end if */

	nulstr_finish(&ss) ;

ret0:
	return (rs >= 0) ? f : rs ;
}
/* end subroutine (subinfo_dirok) */



