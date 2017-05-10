/* pcsnso */

/* PCS-NAME-SERVER query database manager */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_UGETPW	1		/* use |ugetpw(3uc)| */
#define	CF_PCSNSC	1		/* use PCSNSC facility */


/* revision history:

	- 2008-10-01, David A­D­ Morano
	This object module was originally written.

*/

/* Copyright © 2008 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This is the main interface to the PCS Name-Server.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<limits.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>
#include	<pwd.h>
#include	<project.h>
#include	<netdb.h>

#include	<vsystem.h>
#include	<endianstr.h>
#include	<getbufsize.h>
#include	<estrings.h>
#include	<char.h>
#include	<vecstr.h>
#include	<spawnproc.h>
#include	<expcook.h>
#include	<ascii.h>
#include	<field.h>
#include	<sbuf.h>
#include	<getax.h>
#include	<ugetpw.h>
#include	<getxusername.h>
#include	<gecos.h>
#include	<localmisc.h>

#include	"pcsnso.h"
#include	"pcsnsmgr.h"


/* local defines */

#if	CF_UGETPW
#define	GETPW_NAME	ugetpw_name
#else
#define	GETPW_NAME	getpw_name
#endif /* CF_UGETPW */
#undef	COMMENT

#ifndef	NULLFNAME
#define	NULLFNAME	"/dev/null"
#endif

#ifndef	NSYSPIDS
#define	NSYSPIDS	100
#endif

#ifndef	LINEBUFLEN
#ifdef	LINE_MAX
#define	LINEBUFLEN	MAX(LINE_MAX,2048)
#else
#define	LINEBUFLEN	2048
#endif
#endif

#define	BUFLEN		(MAXPATHLEN + MAXHOSTNAMELEN + LINEBUFLEN)

#ifndef	VARPRPCS
#define	VARPRPCS	"PCS"
#endif

#ifndef	VARHOME
#define	VARHOME		"HOME"
#endif

#ifndef	VARUSERNAME
#define	VARUSERNAME	"USERNAME"
#endif

#ifndef	VARNAME
#define	VARNAME		"NAME"
#endif

#ifndef	VARFULLNAME
#define	VARFULLNAME	"FULLNAME"
#endif

#ifndef	VARPROJINFO
#define	VARPROJINFO	"PROJINFO"
#endif

#ifndef	VARORG
#define	VARORG		"ORGANIZATION"
#endif

#ifndef	DEFPROJNAME
#define	DEFPROJNAME	"default"
#endif

#ifndef	NAMEFNAME
#define	NAMEFNAME	".name"
#endif

#ifndef	FULLNAMEFNAME
#define	FULLNAMEFNAME	".fullname"
#endif

#ifndef	PROJECTFNAME
#define	PROJECTFNAME	".project"
#endif

#ifndef	ORGFNAME
#define	ORGFNAME	".organization"
#endif

#ifndef	PCSDPIFNAME /* PCS Default-Project-Info file */
#define	PCSDPIFNAME	"etc/projectinfo"
#endif

#ifndef	PRORGFNAME
#define	PRORGFNAME	"etc/organization"
#endif

#ifndef	VARPRLOCAL
#define	VARPRLOCAL	"LOCAL"
#endif

#ifndef	TMPDNAME
#define	TMPDNAME	"/tmp"
#endif

#ifndef	TMPVARDNAME
#define	TMPVARDNAME	"/var/tmp"
#endif

#ifndef	VCNAME
#define	VCNAME		"var"
#endif

#define	INDDNAME	"pcsnso"

#define	TO_FILEMOD	(60 * 24 * 3600)

#define	SUBINFO		struct subinfo
#define	SUBINFO_FL	struct subinfo_flags


/* external subroutines */

extern int	mkfnamesuf1(char *,const char *,const char *) ;
extern int	mkfnamesuf2(char *,const char *,const char *,const char *) ;
extern int	sfbasename(const char *,int,const char **) ;
extern int	sfskipwhite(const char *,int,const char **) ;
extern int	sfshrink(const char *,int,const char **) ;
extern int	nextfield(const char *,int,const char **) ;
extern int	nleadstr(const char *,const char *,int) ;
extern int	matkeystr(const char **,char *,int) ;
extern int	strpcmp(const char *,const char *) ;
extern int	vstrkeycmp(const char **,const char **) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecui(const char *,int,uint *) ;
extern int	vecstr_envadd(vecstr *,const char *,const char *,int) ;
extern int	vecstr_adduniq(vecstr *,const char *,int) ;
extern int	pathclean(char *,const char *,int) ;
extern int	perm(const char *,uid_t,gid_t,gid_t *,int) ;
extern int	sperm(IDS *,struct ustat *,int) ;
extern int	mkdirs(const char *,mode_t) ;
extern int	getuserhome(char *,int,cchar *) ;
extern int	getgecosname(const char *,int,const char **) ;
extern int	mkgecosname(char *,int,const char *) ;
extern int	mkrealname(char *,int,const char *,int) ;
extern int	readfileline(char *,int,cchar *) ;
extern int	hasuc(const char *,int) ;
extern int	hasalldig(const char *,int) ;
extern int	isalnumlatin(int) ;
extern int	isdigitlatin(int) ;
extern int	isNotPresent(int) ;
extern int	isBadSend(int) ;
extern int	isBadRecv(int) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(cchar *,int,int) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strwcpylc(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;
extern char	*strnpbrk(const char *,int,const char *) ;


/* exported variables */

PCSNSO_OBJ	pcsnso = {
	"pcsnso",
	sizeof(PCSNSO),
	sizeof(PCSNSO_CUR)
} ;


/* local structures */

struct subinfo_flags {
	uint		setcache:1 ;
} ;

struct subinfo {
	PCSNSO		*op ;
	cchar		*pr ;		/* convenience */
	cchar		*varusername ;
	cchar		*un ;		/* passed argument */
	char		*rbuf ;		/* passed argument */
	SUBINFO_FL	init, f ;
	uid_t		uid ;
	int		rlen ;		/* passed argument */
	int		w ;		/* passed argument */
} ;

struct pcsnametype {
	const char	*var ;
	const char	*fname ;
} ;


/* forward references */

static int	pcsnso_infoloadbegin(PCSNSO *,cchar *) ;
static int	pcsnso_infoloadend(PCSNSO *) ;
static int	pcsnso_getpw(PCSNSO *,cchar *) ;

static int	pcsnso_getrealname(PCSNSO *,SUBINFO *) ;
static int	pcsnso_getpcsname(PCSNSO *,SUBINFO *) ;
static int	pcsnso_getfullname(PCSNSO *,SUBINFO *) ;
static int	pcsnso_getprojinfo(PCSNSO *,SUBINFO *) ;
static int	pcsnso_client(PCSNSO *) ;
static int	pcsnso_clientbegin(PCSNSO *) ;
static int	pcsnso_clientend(PCSNSO *) ;

static int	subinfo_start(SUBINFO *,PCSNSO *,char *,int,cchar *,int) ;
static int	subinfo_finish(SUBINFO *) ;
static int	subinfo_prfile(SUBINFO *,cchar *) ;

static int	getname(SUBINFO *) ;
static int	getname_var(SUBINFO *) ;
static int	getname_daemon(SUBINFO *) ;
static int	getname_nsmgr(SUBINFO *) ;
static int	getname_userhome(SUBINFO *) ;
static int	getname_again(SUBINFO *) ;
static int	getname_sysdb(SUBINFO *) ;
static int	getname_pcsdef(SUBINFO *) ;

static int	getprojinfo_sysdb(SUBINFO *) ;


/* local variables */

static const struct pcsnametype	pcsnametypes[] = {
	{ NULL, NULL },
	{ VARNAME, NAMEFNAME },
	{ VARFULLNAME, FULLNAMEFNAME },
	{ VARPROJINFO, PROJECTFNAME },
	{ VARORG, ORGFNAME },
	{ NULL, NULL }
} ;

static int	(*getnames[])(SUBINFO *) = {
	getname_var,
	getname_nsmgr,
	getname_daemon,
	getname_userhome,
	getname_again,
	getname_sysdb,
	getname_pcsdef,
	NULL
} ;


/* exported subroutines */


int pcsnso_open(PCSNSO *op,cchar *pr)
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;
	if (pr == NULL) return SR_FAULT ;

	if (pr[0] == '\0') return SR_INVALID ;

#if	CF_DEBUGS
	debugprintf("pcsnso_open: pr=%s\n",pr) ;
#endif

	memset(op,0,sizeof(PCSNSO)) ;

	if ((rs = pcsnso_infoloadbegin(op,pr)) >= 0) {
	    op->magic = PCSNSO_MAGIC ;
	} /* end if (pcsnso_infoloadbegin) */

#if	CF_DEBUGS
	debugprintf("pcsnso_open: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (pcsnso_open) */


int pcsnso_close(PCSNSO *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != PCSNSO_MAGIC) return SR_NOTOPEN ;

	rs1 = pcsnso_clientend(op) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = pcsnso_infoloadend(op) ;
	if (rs >= 0) rs = rs1 ;

#if	CF_DEBUGS
	debugprintf("pcsnso_close: ret rs=%d\n",rs) ;
#endif

	op->magic = 0 ;
	return rs ;
}
/* end subroutine (pcsnso_close) */


int pcsnso_setopts(PCSNSO *op,int opts)
{
	int		rs = SR_OK ;
	if (op == NULL) return SR_FAULT ;
	if (op->magic != PCSNSO_MAGIC) return SR_NOTOPEN ;
	op->opts = opts ;
	return rs ;
}
/* end subroutine (pcsnso_setopts) */


int pcsnso_get(PCSNSO *op,char *rbuf,int rlen,cchar *un,int w)
{
	SUBINFO		si, *sip = &si ;
	int		rs ;
	int		rs1 ;
	int		len = 0 ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != PCSNSO_MAGIC) return SR_NOTOPEN ;

	if ((rs = subinfo_start(sip,op,rbuf,rlen,un,w)) >= 0) {
	    switch (w) {
	    case pcsnsreq_realname:
	        rs = pcsnso_getrealname(op,sip) ;
	        len = rs ;
	        break ;
	    case pcsnsreq_pcsname:
	        rs = pcsnso_getpcsname(op,sip) ;
	        len = rs ;
	        break ;
	    case pcsnsreq_fullname:
	        rs = pcsnso_getfullname(op,sip) ;
	        len = rs ;
	        break ;
	    case pcsnsreq_projinfo:
	        rs = pcsnso_getprojinfo(op,sip) ;
	        len = rs ;
	        break ;
	    case pcsnsreq_pcsorg:
	        rs = pcsnso_getpcsname(op,sip) ;
	        len = rs ;
	        break ;
	    default:
	        rs = SR_INVALID ;
	        break ;
	    } /* end switch */
	    rs1 = subinfo_finish(sip) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (subinfo) */

#if	CF_DEBUGS
	debugprintf("pcsnso_get: ret rs=%d len=%u\n",rs,len) ;
#endif

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (pcsnso_get) */


int pcsnso_audit(PCSNSO *op)
{
	int		rs = SR_OK ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != PCSNSO_MAGIC) return SR_NOTOPEN ;

#if	CF_DEBUGS
	debugprintf("pcsnso_audit: txtindex_audit() rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (pcsnso_audit) */


int pcsnso_curbegin(PCSNSO *op,PCSNSO_CUR *curp)
{
	int		rs = SR_OK ;

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;

	if (op->magic != PCSNSO_MAGIC) return SR_NOTOPEN ;

	memset(curp,0,sizeof(PCSNSO_CUR)) ;
	op->ncursors += 1 ;

	return rs ;
}
/* end subroutine (pcsnso_curbegin) */


int pcsnso_curend(PCSNSO *op,PCSNSO_CUR *curp)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;

	if (op->magic != PCSNSO_MAGIC) return SR_NOTOPEN ;

	if (curp->verses != NULL) {
	    rs1 = uc_free(curp->verses) ;
	    if (rs >= 0) rs = rs1 ;
	    curp->verses = NULL ;
	}

	curp->nverses = 0 ;
	if (op->ncursors > 0) op->ncursors -= 1 ;

	return rs ;
}
/* end subroutine (pcsnso_curend) */


/* ARGSUSED */
int pcsnso_enum(PCSNSO *op,PCSNSO_CUR *curp,char *vbuf,int vlen,int w)
{
	int		rs = SR_OK ;
	int		len = 0 ;

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;
	if (vbuf == NULL) return SR_FAULT ;

	if (op->magic != PCSNSO_MAGIC) return SR_NOTOPEN ;


#if	CF_DEBUGS
	debugprintf("pcsnso_read: ret rs=%d len=%u\n",rs,len) ;
#endif

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (pcsnso_enum) */


/* private subroutines */


static int pcsnso_infoloadbegin(PCSNSO *op,cchar *pr)
{
	const int	pwlen = getbufsize(getbufsize_pw) ;
	int		rs ;
	int		size = 0 ;
	char		*bp ;

	size += (pwlen+1) ;
	size += (strlen(pr)+1) ;
	if ((rs = uc_malloc(size,&bp)) >= 0) {
	    PCSNSO_PWD	*pdp = &op->pwd ;
	    op->a = bp ;
	    pdp->pwbuf = bp ;
	    pdp->pwlen = pwlen ;
	    bp += (pwlen+1) ;
	    op->pr = bp ;
	    strwcpy(bp,pr,-1) ;
	}

	return rs ;
}
/* end subroutine (pcsnso_infoloadbegin) */


static int pcsnso_infoloadend(PCSNSO *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op->f.id) {
	    op->f.id = FALSE ;
	    rs1 = ids_release(&op->id) ;
	    if (rs >= 0) rs = rs1 ;
	}

	if (op->a != NULL) {
	    PCSNSO_PWD	*pdp = &op->pwd ;
	    rs1 = uc_free(op->a) ;
	    if (rs >= 0) rs = rs1 ;
	    op->a = NULL ;
	    op->pr = NULL ;
	    pdp->pwbuf = NULL ;
	    pdp->pwlen = 0 ;
	}

	return rs ;
}
/* end subroutine (pcsnso_infoloadend) */


static int pcsnso_getpw(PCSNSO *op,cchar *un)
{
	PCSNSO_PWD	*pdp = &op->pwd ;
	int		rs = SR_OK ;
	cchar		*pun ;

	pun = pdp->pw.pw_name ;
	if ((pun == NULL) || (strcmp(pun,un) != 0)) {
	    struct passwd	*pwp = &pdp->pw ;
	    const int		pwlen = pdp->pwlen ;
	    char		*pwbuf = pdp->pwbuf ;
	    if ((un != NULL) && (un[0] != '\0') && (un[0] != '-')) {
	        if (hasalldig(un,-1)) {
	            uint	uv ;
	            if ((rs = cfdecui(un,-1,&uv)) >= 0) {
	                const uid_t	uid = uv ;
	                rs = getpwusername(pwp,pwbuf,pwlen,uid) ;
	            }
	        } else {
	            rs = GETPW_NAME(pwp,pwbuf,pwlen,un) ;
	        }
	    } else {
	        rs = getpwusername(pwp,pwbuf,pwlen,-1) ;
	    }
	} /* end if (was not already initialized) */

#if	CF_DEBUGS
	debugprintf("pcsnames/subinfo_getpw: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subrouine (pcsnso_getpw) */


/* ARGSUSED */
static int pcsnso_getrealname(PCSNSO *op,SUBINFO *sip)
{
	int		rs ;

	rs = getname(sip) ;

	return rs ;
}
/* end subrouine (pcsnso_getrealname) */


/* ARGSUSED */
static int pcsnso_getpcsname(PCSNSO *op,SUBINFO *sip)
{
	int		rs ;

	rs = getname(sip) ;

	return rs ;
}
/* end subrouine (pcsnso_getpcsname) */


/* ARGSUSED */
static int pcsnso_getfullname(PCSNSO *op,SUBINFO *sip)
{
	int		rs ;

	rs = getname(sip) ;

	return rs ;
}
/* end subrouine (pcsnso_getfullname) */


/* ARGSUSED */
static int pcsnso_getprojinfo(PCSNSO *op,SUBINFO *sip)
{
	int		rs ;

#if	CF_DEBUGS
	debugprintf("pcsnso/getprojinfo: ent\n") ;
#endif

	rs = getname(sip) ;

#if	CF_DEBUGS
	debugprintf("pcsnso/getprojinfo: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subrouine (pcsnso_getprojinfo) */


static int pcsnso_client(PCSNSO *op)
{
	int		rs = MKBOOL(op->open.client) ;
	if (! op->f.client) {
	    rs = pcsnso_clientbegin(op) ;
	}
#if	CF_DEBUGS
	debugprintf("pcsnso_client: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroiutine (pcsnso_client) */


static int pcsnso_clientbegin(PCSNSO *op)
{
	int		rs = SR_OK ;
	int		f = FALSE ;
	if (! op->f.client) {
	    PCSNSC	*pcp = &op->client ;
	    const int	to = PCSNSO_TO ;
	    op->f.client = TRUE ;
	    if ((rs = pcsnsc_open(pcp,op->pr,to)) > 0) {
		op->open.client = TRUE ;
		f = TRUE ;
	    } else if (isBadSend(rs)) {
		rs = SR_OK ;
	    }
	}
#if	CF_DEBUGS
	debugprintf("pcsnso_clientbegin: ret rs=%d f=%u\n",rs,f) ;
#endif
	return (rs >= 0) ? f : rs ;
}
/* end subroiutine (pcsnso_clientbegin) */


static int pcsnso_clientend(PCSNSO *op)
{
	int		rs = SR_OK ;
	if (op->open.client) {
	    PCSNSC	*pcp = &op->client ;
	    op->open.client = FALSE ;
	    rs = pcsnsc_close(pcp) ;
	}
	return rs ;
}
/* end subroiutine (pcsnso_clientend) */


static int subinfo_start(SUBINFO *sip,PCSNSO *op,char *rbuf,int rlen,
			cchar *un,int w)
{
	int		rs = SR_OK ;

	rbuf[0] = '\0' ;
	memset(sip,0,sizeof(SUBINFO)) ;
	sip->op = op ;
	sip->pr = op->pr ;
	sip->rbuf = rbuf ;
	sip->rlen = rlen ;
	sip->un = un ;
	sip->w = w ;
	sip->varusername = VARUSERNAME ;

	return rs ;
}
/* end subroutine (subinfo_start) */


static int subinfo_finish(SUBINFO *sip)
{
	int		rs = SR_OK ;
	if (sip == NULL) return SR_FAULT ;
	return rs ;
}
/* end subroutine (subinfo_finish) */


static int subinfo_prfile(SUBINFO *sip,cchar *fn)
{
	int		rs ;
	int		len = 0 ;
	char		tbuf[MAXPATHLEN + 1] ;
	if ((rs = mkpath2(tbuf,sip->pr,fn)) >= 0) {
	    const int	rlen = sip->rlen ;
	    char	*rbuf = sip->rbuf ;
	    if ((rs = readfileline(rbuf,rlen,tbuf)) >= 0) {
	        len = rs ;
	    } else if (isNotPresent(rs)) {
	        rs = SR_OK ;
	    }
	}
	return (rs >= 0) ? len : rs ;
}
/* end subroutine (subinfo_prfile) */


static int getname(SUBINFO *sip)
{
	const int	w = sip->w ;
	int		rs = SR_OK ;
	int		len = 0 ;

#if	CF_DEBUGS
	debugprintf("pcsnso/getname: ent u=%s w=%u\n",sip->un,sip->w) ;
#endif

	switch (w) {
	case pcsnsreq_realname:
	case pcsnsreq_pcsname:
	case pcsnsreq_fullname:
	case pcsnsreq_projinfo:
	case pcsnsreq_pcsorg:
	    {
	        int		i ;
	        for (i = 0 ; getnames[i] != NULL ; i += 1) {
	            rs = (*getnames[i])(sip) ;
	            len = rs ;
	            if (rs != 0) break ;
	        } /* end for */
	        if ((rs > 0) && sip->f.setcache) {
	            rs = pcsnsmgr_set(sip->rbuf,len,sip->un,sip->w,0) ;
#if	CF_DEBUGS
	            debugprintf("pcsnso/getname: pcsnsmgr_set() rs=%d\n",rs) ;
#endif
	        }
	    } /* end block */
	    break ;
	} /* end switch */

#if	CF_DEBUGS
	debugprintf("pcsnso/getname: ret rs=%d len=%u\n",rs,len) ;
#endif

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (getname) */


static int getname_var(SUBINFO *sip)
{
	const int	w = sip->w ;
	int		rs = SR_OK ;
	int		len = 0 ;
	cchar		*un = sip->un ;

	switch (w) {
	case pcsnsreq_pcsname:
	case pcsnsreq_fullname:
	case pcsnsreq_pcsorg:
	    {
	        int	f = (un[0] == '-') ;
	        if (! f) {
	            const char	*vun = getenv(VARUSERNAME) ;
	            if ((vun != NULL) && (vun[0] != '\0')) {
	                f = (strcmp(vun,un) == 0) ;
	            }
	        }
	        if (f) {
	            cchar	*var = pcsnametypes[w].var ;
	            if (var != NULL) {
	                cchar	*cp = getenv(var) ;
	                if ((cp != NULL) && (cp[0] != '\0')) {
	                    rs = sncpy1(sip->rbuf,sip->rlen,cp) ;
	                    len = rs ;
	                }
	            } /* end if (anyone but 'realname') */
	        } /* end if */
	    } /* end block */
	    break ;
	} /* end switch */

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (getname_var) */


static int getname_daemon(SUBINFO *sip)
{
	PCSNSO		*op = sip->op ;
	int		rs = SR_OK ;
	int		rl = 0 ;
#if	CF_DEBUGS
	debugprintf("pcsnso/getname_daemon: ent\n") ;
#endif
#if	CF_PCSNSC
	if ((op->opts & PCSNSO_ONOSERV) == 0) {
#if	CF_DEBUGS
	debugprintf("pcsnso/getname_daemon: ent serv\n") ;
#endif
	    if ((rs = pcsnso_client(op)) > 0) {
	        PCSNSC		*pcp = &op->client ;
	        const int	rlen = sip->rlen ;
		const int	w = sip->w ;
		const char	*un = sip->un ;
		char		*rbuf = sip->rbuf ;
	        if ((rs = pcsnsc_getval(pcp,rbuf,rlen,un,w)) > 0) {
		    rl = rs ;
#if	CF_DEBUGS
		debugprintf("pcsnso/getname_daemon: pcsnsc_getval() rs=%d\n",
		rs) ;
#endif
		} else if (isBadSend(rs)) {
		    rs = SR_OK ;
		}
#if	CF_DEBUGS
	debugprintf("pcsnso/getname_daemon: pcsnsc_open-out rs=%d\n",rs) ;
#endif
	    } /* end if (pcsnso_client) */
#if	CF_DEBUGS
	debugprintf("pcsnso/getname_daemon: leaving rs=%d\n",rs) ;
#endif
	} /* end if (ok to call server) */
#endif /* CF_PCSNSC */
#if	CF_DEBUGS
	debugprintf("pcsnso/getname_daemon: ret rs=%d rl=%u\n",rs,rl) ;
#endif
	return (rs >= 0) ? rl : rs ;
}
/* end subroutine (getname_daemon) */


static int getname_nsmgr(SUBINFO *sip)
{
	const int	rsn = SR_NOTFOUND ;
	const int	w = sip->w ;
	int		rs ;
	const char	*un = sip->un ;

#if	CF_DEBUGS
	debugprintf("pcsnso/getname_nsmgr: ent\n") ;
#endif

	if ((rs = pcsnsmgr_get(sip->rbuf,sip->rlen,un,w)) == rsn) {
#if	CF_DEBUGS
	    debugprintf("pcsnso/getname_nsmgr: pcsnsmgr_get() rs=%d\n",rs) ;
#endif
	    rs = SR_OK ;
	    sip->f.setcache = TRUE ;
	} else if (rs == 0) {
#if	CF_DEBUGS
	    debugprintf("pcsnso/getname_nsmgr: pcsnsmgr_get() rs=%d\n",rs) ;
#endif
	    sip->f.setcache = TRUE ;
	}

#if	CF_DEBUGS
	debugprintf("pcsnso/getname_nsmgr: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (getname_nsmgr) */


static int getname_userhome(SUBINFO *sip)
{
	const int	w = sip->w ;
	int		rs = SR_OK ;
	cchar		*un = sip->un ;
	cchar		*fn ;

#if	CF_DEBUGS
	debugprintf("pcsgetnames/getname_userhome: ent un=%s w=%u\n",un,w) ;
#endif

	fn = pcsnametypes[w].fname ;
#if	CF_DEBUGS
	debugprintf("pcsgetnames/getname_userhome: fn=%s\n",fn) ;
#endif
	if (fn != NULL) {
	    const int	hlen = MAXPATHLEN ;
	    char	hbuf[MAXPATHLEN + 1] ;
	    if ((rs = getuserhome(hbuf,hlen,un)) >= 0) {
	        char	tbuf[MAXPATHLEN + 1] ;
#if	CF_DEBUGS
	        debugprintf("pcsgetnames/getname_userhome: h=%s\n",hbuf) ;
#endif
	        if ((rs = mkpath2(tbuf,hbuf,fn)) >= 0) {
#if	CF_DEBUGS
	            debugprintf("pcsgetnames/getname_userhome: tbuf=%s\n",
	                tbuf) ;
#endif
	            rs = readfileline(sip->rbuf,sip->rlen,tbuf) ;
	            if (isNotPresent(rs)) rs = SR_OK ;
	        }
	    } /* end if (getuserhome) */
	} /* end if (non-null) */

#if	CF_DEBUGS
	debugprintf("pcsgetnames/getname_userhome: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (getname_userhome) */


static int getname_again(SUBINFO *sip)
{
	int		rs = SR_OK ;

	if (sip->w == pcsnsreq_fullname) {
	    sip->w = pcsnsreq_pcsname ;
	    rs = getname(sip) ;
	}

	return rs ;
}
/* end subroutine (getname_again) */


static int getname_sysdb(SUBINFO *sip)
{
	PCSNSO		*op = sip->op ;
	const int	w = sip->w ;
	int		rs ;
	int		len = 0 ;

	if ((rs = pcsnso_getpw(op,sip->un)) >= 0) {
	    PCSNSO_PWD		*pdp = &op->pwd ;
	    int			nlen ;
	    char		*nbuf ;
	    switch (w) {
	    case pcsnsreq_realname:
	    case pcsnsreq_pcsname:
	    case pcsnsreq_fullname:
	        {
	            cchar	*gecos = pdp->pw.pw_gecos ;
	            nlen = (strlen(gecos)+10) ;
	            if ((rs = uc_malloc((nlen+1),&nbuf)) >= 0) {
	                if ((rs = mkgecosname(nbuf,nlen,gecos)) > 0) {
	                    rs = mkrealname(sip->rbuf,sip->rlen,nbuf,rs) ;
	                    len = rs ;
	                }
	                uc_free(nbuf) ;
	            } /* end if (memory-allocation) */
	        } /* end block */
	        break ;
	    case pcsnsreq_projinfo:
	        rs = getprojinfo_sysdb(sip) ;
	        len = rs ;
	        break ;
	    case pcsnsreq_pcsorg:
	        {
	            GECOS	g ;
	            cchar	*gecos = pdp->pw.pw_gecos ;
	            if ((rs = gecos_start(&g,gecos,-1)) >= 0) {
	                int		vl ;
	                const int	gi = gecosval_organization ;
	                const char	*vp ;
	                if ((vl = gecos_getval(&g,gi,&vp)) > 0) {
	                    rs = sncpy1w(sip->rbuf,sip->rlen,vp,vl) ;
	                    len = rs ;
	                }
	                gecos_finish(&g) ;
	            } /* end if (GECOS) */
	        } /* end block */
	        break ;
	    } /* end switch */
	} /* end if */

#if	CF_DEBUGS
	debugprintf("pcsnames/getname_sysdb: rn=>%t<\n",sip->rbuf,sip->rlen) ;
	debugprintf("pcsnames/getname_sysdb: ret rs=%d len=%u\n",rs,len) ;
#endif

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (getname_sysdb) */


static int getname_pcsdef(SUBINFO *sip)
{
	PCSNSO		*op = sip->op ;
	const int	w = sip->w ;
	int		rs = SR_OK ;
	int		len = 0 ;
	cchar		*fn = NULL ;

	switch (w) {
	case pcsnsreq_projinfo:
	    fn = PCSDPIFNAME ;
	    break ;
	case pcsnsreq_pcsorg:
	    fn = PRORGFNAME ;
	    break ;
	} /* end switch */

	if ((rs >= 0) && (fn != NULL)) {
	    switch (w) {
	    case pcsnsreq_projinfo:
	        {
	            PCSNSO_PWD	*pdp = &op->pwd ;
	            cchar	*un = sip->un ;
	            if ((rs = pcsnso_getpw(op,un)) >= 0) {
	                const uid_t	uid = pdp->pw.pw_uid ;
	                if (uid >= NSYSPIDS) {
	                    rs = subinfo_prfile(sip,fn) ;
	                    len = rs ;
	                } /* end if (system UID) */
	            } /* end if (pcsnso_getpw) */
	        }
	        break ;
	    case pcsnsreq_pcsorg:
	        {
	            rs = subinfo_prfile(sip,fn) ;
	            len = rs ;
	        }
	        break ;
	    } /* end switch */
	} /* end if (appropriate) */

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (getname_pcsdef) */


static int getprojinfo_sysdb(SUBINFO *sip)
{
	PCSNSO		*op = sip->op ;
	struct project	pj ;
	const int	pjlen = getbufsize(getbufsize_pj) ;
	int		rs ;
	int		len = 0 ;
	char		*pjbuf ;

#if	CF_DEBUGS
	debugprintf("pcsgetnames/getprojinfo_sysdb: un=%d\n",sip->un) ;
#endif

	if ((rs = uc_malloc((pjlen+1),&pjbuf)) >= 0) {
	    cchar	*un = sip->un ;
	    if ((rs = uc_getdefaultproj(un,&pj,pjbuf,pjlen)) >= 0) {
	        int	f = (strcmp(pj.pj_name,DEFPROJNAME) != 0) ;
	        if (f) {
	            PCSNSO_PWD	*pdp = &op->pwd ;
	            if ((rs = pcsnso_getpw(op,un)) >= 0) {
	                const uid_t	uid = pdp->pw.pw_uid ;
	                f = (uid >= NSYSPIDS) ;
	            }
	        }
	        if ((rs >= 0) && f) {
	            cchar	*comment = pj.pj_comment ;
	            rs = sncpy1(sip->rbuf,sip->rlen,comment) ;
	            len = rs ;
	        }
	    } else if (isNotPresent(rs)) {
	        rs = SR_OK ;
	    }
	    uc_free(pjbuf) ;
	} /* end if (memory-allocation) */

#if	CF_DEBUGS
	debugprintf("pcsgetnames/getprojinfo_sysdb: rs=%d len=%u\n",rs,len) ;
#endif

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (getprojinfo_sysdb) */


