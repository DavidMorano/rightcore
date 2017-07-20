/* uunames */

/* uunames-query database manager */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_DEBUGEXISTS	0		/* debug 'uunames_exists(3uux)' */
#define	CF_NULSTR	0		/* use 'nulstr(3)' */
#define	CF_WITHENDIAN	0		/* use ENDIANness */
#define	CF_GETPROGROOT	1		/* use 'getprogroot(3dam)' */


/* revision history:

	= 1998-02-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/******************************************************************************

	This little object provides access to the UUNAMES database and index
	(if any).


******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<sys/mman.h>
#include	<limits.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<endianstr.h>
#include	<char.h>
#include	<vecstr.h>
#include	<vecobj.h>
#include	<spawnproc.h>
#include	<storebuf.h>
#include	<ids.h>
#include	<ascii.h>
#include	<baops.h>
#include	<sbuf.h>
#include	<localmisc.h>

#include	"uunames.h"
#include	"nulstr.h"


/* local defines */

#ifndef	VARSYSNAME
#define	VARSYSNAME	"SYSNAME"
#endif

#ifndef	VARRELEASE
#define	VARRELEASE	"RELEASE"
#endif

#ifndef	VARVERSION
#define	VARVERSION	"VERSION"
#endif

#ifndef	VARMACHINE
#define	VARMACHINE	"MACHINE"
#endif

#ifndef	VARARCHITECTURE
#define	VARARCHITECTURE	"ARCHITECTURE"
#endif

#ifndef	VARHZ
#define	VARHZ		"HZ"
#endif

#ifndef	VARDOMAIN
#define	VARDOMAIN	"DOMAIN"
#endif

#ifndef	VARNODE
#define	VARNODE		"NODE"
#endif

#ifndef	VARHOMEDNAME
#define	VARHOMEDNAME	"HOME"
#endif

#ifndef	VARUSERNAME
#define	VARUSERNAME	"USERNAME"
#endif

#ifndef	VARLOGNAME
#define	VARLOGNAME	"LOGNAME"
#endif

#ifndef	VARTZ
#define	VARTZ		"TZ"
#endif

#ifndef	VARPWD
#define	VARPWD		"PWD"
#endif

#ifndef	VARTMPDNAME
#define	VARTMPDNAME	"TMPDIR"
#endif

#ifndef	VARMAIL
#define	VARMAIL		"MAIL"
#endif

#ifndef	VARPATH
#define	VARPATH		"PATH"
#endif

#ifndef	VARLIBPATH
#define	VARLIBPATH	"LD_LIBRARY_PATH"
#endif

#ifndef	VARMANPATH
#define	VARMANPATH	"MANPATH"
#endif

#ifndef	VARVPATH
#define	VARVPATH	"VPATH"
#endif

#ifndef	VARPRLOCAL
#define	VARPRLOCAL	"LOCAL"
#endif

#ifndef	VARPRNCMP
#define	VARPRNCMP	"NCMP"
#endif

#ifndef	VARPRMKUU
#define	VARPRMKUU	"MKUUNAMES_PROGRAMROOT"
#endif

#undef	VARDBNAME
#define	VARDBNAME	"MKUUNAMES_DBNAME"

#ifndef	TMPDNAME
#define	TMPDNAME	"/tmp"
#endif

#ifndef	TMPVARDNAME
#define	TMPVARDNAME	"/var/tmp"
#endif

#define	UUNAMES_DBMAGICSTR	"UUNAMES"
#define	UUNAMES_PRNAME	VARPRNCMP

#define	VARDNAME	"var"
#define	INDDNAME	"uunames"

#define	DBNAME		"uunames"
#define	INDSUF		"sl"

#ifndef	KEYBUFLEN
#define	KEYBUFLEN	NATURALWORDLEN
#endif

#define	TO_FILEMOD	(60 * 24 * 3600)
#define	TO_MKWAIT	(5 * 50)

#define	TMPDMODE	0775

#define	DEFNAMES	10

#define	PROG_MKUUNAMES	"mkuunames"


/* external subroutines */

extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	sncpy4(char *,int, const char *,const char *,
			const char *,const char *) ;
extern int	sncpylc(char *,int,const char *) ;
extern int	snwcpy(char *,int,const char *,int) ;
extern int	snwcpylc(char *,int,const char *,int) ;
extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	mkfnamesuf1(char *,const char *,const char *) ;
extern int	mkfnamesuf2(char *,const char *,const char *,const char *) ;
extern int	sfbasename(const char *,int,const char **) ;
extern int	pathclean(char *,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	vecstr_envadd(vecstr *,const char *,const char *,int) ;
extern int	vecstr_insert(vecstr *,int,const char *,int) ;
extern int	vecstr_adduniq(vecstr *,const char *,int) ;
extern int	perm(const char *,uid_t,gid_t,gid_t *,int) ;
extern int	mkdirs(const char *,mode_t) ;
extern int	prgetprogpath(const char *,char *,const char *,int) ;
extern int	getprogroot(const char *,const char **,
			int *,char *,const char *) ;
extern int	strpcmp(const char *,const char *) ;
extern int	hasuc(const char *,int) ;
extern int	isalnumlatin(int) ;
extern int	isNotPresent(int) ;
extern int	isNotAccess(int) ;

#if	CF_DEBUGS
extern int	strnnlen(const char *,int,int) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strwcpylc(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;
extern char	*strnpbrk(const char *,int,const char *) ;


/* exported variables */

UUNAMES_OBJ	uunames = {
	"uunames",
	sizeof(UUNAMES),
	sizeof(UUNAMES_CUR)
} ;


/* local structures */

struct liner {
	const char	*lp ;
	int		ll ;
} ;

struct envpop {
	const char	*name ;
	const char	*sub1dname ;
	const char	*sub2dname ;
} ;


/* forward references */

static int	uunames_infoloadbegin(UUNAMES *,const char *,const char *) ;
static int	uunames_infoloadend(UUNAMES *) ;
static int	uunames_indopen(UUNAMES *,time_t) ;
static int	uunames_indopenpr(UUNAMES *,time_t) ;
static int	uunames_indopentmp(UUNAMES *,time_t) ;
static int	uunames_indopendname(UUNAMES *,const char *,time_t) ;
static int	uunames_indclose(UUNAMES *) ;
static int	uunames_mkuunamesi(UUNAMES *,const char *) ;
static int	uunames_envpaths(UUNAMES *,vecstr *) ;
static int	uunames_indtest(UUNAMES *,const char *,time_t) ;
static int	uunames_indmapcreate(UUNAMES *,const char *,time_t) ;
static int	uunames_indmapdestroy(UUNAMES *) ;
static int	uunames_filemapcreate(UUNAMES *,time_t) ;
static int	uunames_filemapdestroy(UUNAMES *) ;
static int	uunames_indmk(UUNAMES *,const char *,time_t) ;
static int	uunames_indlist(UUNAMES *) ;
static int	uunames_indcheck(UUNAMES *,time_t) ;

static int	checkdname(cchar *) ;

static int	vecstr_defenvs(vecstr *,const char **) ;
static int	vecstr_loadpath(vecstr *,const char *) ;
static int	mkpathval(vecstr *,char *,int) ;

#ifdef	COMMENT
static int	mkindfname(char *,const char *,const char *,const char *,
			const char *) ;
#endif

static int	vesrch(void *,void *) ;


/* local variables */

static const char	*envsys[] = {
	VARSYSNAME,
	VARRELEASE,
	VARVERSION,
	VARMACHINE,
	VARARCHITECTURE,
	VARHZ,
	VARNODE,
	VARDOMAIN,
	VARHOMEDNAME,
	VARUSERNAME,
	VARLOGNAME,
	VARTZ,
	VARPWD,
	NULL
} ;

static const char	*prnames[] = {
	"LOCAL",
	"NCMP",
	"EXTRA",
	"PCS",
	"GNU",
	"TOOLS",
	"DWB",
	"XDIR",
	"AST",
	NULL
} ;

static const char	*envdefs[] = {
	"LOCALDOMAIN",
	"USER",
	"MAIL",
	"MAILDIR",
	"MAILDIRS",
	"LANG",
	"LC_COLLATE",
	"LC_CTYPE",
	"LC_MESSAGES",
	"LC_MONETARY",
	"LC_NUMERIC",
	"LC_TIME",
	"PATH",
	"LD_LIBRARY_PATH",
	NULL
} ;

static const struct envpop	envpops[] = {
	{ VARPATH, "bin", "sbin" },
	{ VARLIBPATH, "lib", NULL },
	{ VARMANPATH, "man", NULL },
	{ NULL, NULL, NULL }
} ;

static int	(*indopens[])(UUNAMES *,time_t) = {
	uunames_indopenpr,
	uunames_indopentmp,
	NULL
} ;


/* exported subroutines */


int uunames_open(UUNAMES *op,cchar *pr,cchar *dbname)
{
	time_t		dt = time(NULL) ;
	int		rs ;
	int		size ;

	if (op == NULL) return SR_FAULT ;
	if (pr == NULL) return SR_FAULT ;

	if (pr[0] == '\0') return SR_INVALID ;

#if	CF_DEBUGS
	debugprintf("uunames_open: pr=%s\n",pr) ;
	debugprintf("uunames_open: dbname=%s\n",dbname) ;
#endif

	if ((dbname == NULL) || (dbname[0] == '\0'))
	    dbname = DBNAME ;

	memset(op,0,sizeof(UUNAMES)) ;

	rs = uunames_infoloadbegin(op,pr,dbname) ;
	if (rs < 0)
	    goto bad0 ;

	size = sizeof(struct liner) ;
	rs = vecobj_start(&op->list,size,DEFNAMES,0) ;
	if (rs < 0)
	    goto bad1 ;

/* open an index file (if we can find one) */

	rs = uunames_indopen(op,dt) ;
	if (rs < 0)
	    goto bad2 ;

ret2:
	op->magic = UUNAMES_MAGIC ;

ret0:

#if	CF_DEBUGS
	debugprintf("uunames_open: ret rs=%d\n",rs) ;
#endif

	return rs ;

/* bad stuff */
bad2:
	vecobj_finish(&op->list) ;

bad1:
	uunames_infoloadend(op) ;

bad0:
	goto ret0 ;
}
/* end subroutine (uunames_open) */


int uunames_close(UUNAMES *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != UUNAMES_MAGIC) return SR_NOTOPEN ;

	rs1 = uunames_indclose(op) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = vecobj_finish(&op->list) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = uunames_infoloadend(op) ;
	if (rs >= 0) rs = rs1 ;

	op->magic = 0 ;
	return rs ;
}
/* end subroutine (uunames_close) */


int uunames_audit(UUNAMES *op)
{
	int		rs = SR_OK ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != UUNAMES_MAGIC) return SR_NOTOPEN ;

	rs = uunames_indcheck(op,0) ;

	return rs ;
}
/* end subroutine (uunames_audit) */


int uunames_curbegin(UUNAMES *op,UUNAMES_CUR *curp)
{
	int		rs = SR_OK ;

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;

	if (op->magic != UUNAMES_MAGIC) return SR_NOTOPEN ;

	memset(curp,0,sizeof(UUNAMES_CUR)) ;
	curp->i = -1 ;
	op->ncursors += 1 ;

	rs = uunames_indcheck(op,0) ;

	return rs ;
}
/* end subroutine (uunames_curbegin) */


int uunames_curend(UUNAMES *op,UUNAMES_CUR *curp)
{
	int		rs = SR_OK ;

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;

	if (op->magic != UUNAMES_MAGIC) return SR_NOTOPEN ;

	curp->i = -1 ;
	if (op->ncursors > 0)
	    op->ncursors -= 1 ;

	return rs ;
}
/* end subroutine (uunames_curend) */


int uunames_exists(UUNAMES *op,cchar *s,int slen)
{
	struct liner	le ;
	int		rs = SR_OK ;
	int		kl ;
	const char	*kp = NULL ;

	if (op == NULL) return SR_FAULT ;
	if (s == NULL) return SR_FAULT ;

	if (op->magic != UUNAMES_MAGIC) return SR_NOTOPEN ;

	if (s[0] == '\0') return SR_INVALID ;

#if	CF_DEBUGS
	debugprintf("uunames_exists: slen=%d s=%t\n",slen,s,slen) ;
#endif

	rs = uunames_indcheck(op,0) ;
	if (rs < 0)
	    goto ret0 ;

#if	CF_DEBUGS
	{
	    struct liner	*lep ;
	    int		i ;
	    char	*cp ;
	    for (i = 0 ; vecobj_get(&op->list,i,&lep) >= 0 ; i += 1) {
	    debugprintf("uunames_exists: lep(%p)\n",lep) ;
	    debugprintf("uunames_exists: node[%u]=%t (%p)\n",
		i,lep->lp,lep->ll,lep->lp) ;
	    }
	}
#endif /* CF_DEBUGS */

#if	CF_NULSTR
	{
	    NULSTR	ss ;
	    if ((rs = nulstr_start(&ss,s,slen,&kp)) >= 0) {
	        kl = rs ;
		le.lp = kp ;
		le.ll = kl ;
	        rs = vecobj_search(&op->list,&le,vesrch,NULL) ;
	        nulstr_finish(&ss) ;
	    } /* end if (nulstr) */
	}
#else
	{
	    if ((rs = uc_mallocstrw(s,slen,&kp)) >= 0) {
	        kl = (rs - 1) ;

#if	CF_DEBUGS
	debugprintf("uunames_exists: uc_mallocstrw() rs=%d\n",rs) ;
	debugprintf("uunames_exists: kl=%d kp=%s\n",kl,kp) ;
#endif

	        le.lp = kp ;
	        le.ll = kl ;

#if	CF_DEBUGS && CF_DEBUGEXISTS
	debugprintf("uunames_exists: strnnlen()\n") ;
	debugprintf("uunames_exists: le.ll=%d le.lp=%s\n",le.ll,le.lp) ;
	debugprintf("uunames_exists: keylen=%d\n",
		strnnlen(le.lp,le.ll,40)) ;
	debugprintf("uunames_exists: key=>%t<\n",
		le.lp,strnnlen(le.lp,le.ll,40)) ;
#endif

	        rs = vecobj_search(&op->list,&le,vesrch,NULL) ;

#if	CF_DEBUGS
	debugprintf("uunames_exists: vecobj_search() rs=%d\n",rs) ;
#endif

	        uc_free(kp) ;
	    } /* end if (allocation) */

	}
#endif /* CF_NULSTR */

ret0:

#if	CF_DEBUGS
	    debugprintf("uunames_exists: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (uunames_exists) */


int uunames_enum(op,curp,buf,buflen)
UUNAMES		*op ;
UUNAMES_CUR	*curp ;
char		buf[] ;
int		buflen ;
{
	struct liner	*lep ;
	int		rs = SR_OK ;
	int		i ;
	int		len = 0 ;

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;
	if (buf == NULL) return SR_FAULT ;

	if (op->magic != UUNAMES_MAGIC) return SR_NOTOPEN ;

	i = (curp->i >= 0) ? (curp->i + 1) : 0 ;

#if	CF_DEBUGS
	    debugprintf("uunames_enum: c_i=%u\n",i) ;
#endif

	if ((rs = vecobj_get(&op->list,i,&lep)) >= 0) {
	    if ((rs = snwcpy(buf,buflen,lep->lp,lep->ll)) >= 0) {
	        len = rs ;
	        curp->i = i ;
	    }
	}

ret0:

#if	CF_DEBUGS
	debugprintf("uunames_enum: ret rs=%d len=%u\n",rs,len) ;
#endif

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (uunames_enum) */


int uunames_count(op)
UUNAMES		*op ;
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != UUNAMES_MAGIC) return SR_NOTOPEN ;

	if ((rs = uunames_indcheck(op,0)) >= 0) {
	    rs = vecobj_count(&op->list) ;
	}

	return rs ;
}
/* end subroutine (uunames_count) */


/* private subroutines */


static int uunames_infoloadbegin(op,pr,dbname)
UUNAMES		*op ;
const char	pr[] ;
const char	dbname[] ;
{
	int		rs = SR_OK ;

	op->pr = (char *) pr ;
	op->dbname = (char *) dbname ;

	return rs ;
}
/* end subroutine (uunames_infoloadbegin) */


static int uunames_infoloadend(UUNAMES *op)
{

	op->pr = NULL ;
	op->dbname = NULL ;
	return SR_OK ;
}
/* end subroutine (uunames_infoloadend) */


static int uunames_indmapcreate(UUNAMES *op,cchar *indname,time_t dt)
{
	int		rs = SR_OK ;
	int		fl ;
	char		indfname[MAXPATHLEN + 1] ;

	op->indfname = NULL ;
#if	CF_WITHENDIAN
	rs = mkfnamesuf2(indfname,indname,INDSUF,ENDIANSTR) ;
#else
	rs = mkfnamesuf1(indfname,indname,INDSUF) ;
#endif
	fl = rs ;
	if (rs < 0)
	    goto bad0 ;

	rs = uc_mallocstrw(indfname,fl,&op->indfname) ;
	if (rs < 0)
	    goto bad0 ;

	rs = uunames_filemapcreate(op,dt) ;
	if (rs < 0)
	    goto bad1 ;

	rs = uunames_indlist(op) ;

ret0:
	return rs ;

/* bad stuff */
bad1:
	uc_free(op->indfname) ;
	op->dbname = NULL ;

bad0:
	goto ret0 ;
}
/* end subroutine (uunames_indmapcreate) */


static int uunames_indmapdestroy(UUNAMES *op)
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		i ;

	for (i = 0 ; vecobj_del(&op->list,i) >= 0 ; i += 1) ;

	rs1 = uunames_filemapdestroy(op) ;
	if (rs >= 0) rs = rs1 ;

	if (op->indfname != NULL) {
	    rs1 = uc_free(op->indfname) ;
	    if (rs >= 0) rs = rs1 ;
	    op->dbname = NULL ;
	}

	return rs ;
}
/* end subroutine (uunames_indmapdestroy) */


static int uunames_filemapcreate(UUNAMES *op,time_t dt)
{
	USTAT		sb ;
	int		rs = SR_OK ;
	int		fd ;
	int		mprot, mflags ;

	if (dt == 0) dt = time(NULL) ;

/* open it */

	rs = u_open(op->indfname,O_RDONLY,0666) ;
	fd = rs ;
	if (rs < 0)
	    goto ret0 ;

	rs = u_fstat(fd,&sb) ;
	if (rs < 0)
	    goto ret1 ;

	if (! S_ISREG(sb.st_mode)) {
	    rs = SR_NOTSUP ;
	    goto ret1 ;
	}

	if ((sb.st_size > INT_MAX) || (sb.st_size < 0)) {
	    rs = SR_TOOBIG ;
	    goto ret1 ;
	}

	op->indfsize = sb.st_size ;
	op->ti_mod = sb.st_mtime ;

/* map it */

	mprot = PROT_READ ;
	mflags = MAP_SHARED ;
	rs = u_mmap(NULL,(size_t) op->indfsize,mprot,mflags,
	    fd,0L,&op->indfmap) ;

	if (rs >= 0) {
	    op->ti_map = dt ;
	    op->ti_lastcheck = dt ;
	}

/* close it */
ret1:
	u_close(fd) ;

ret0:
	return rs ;
}
/* end subroutine (uunames_filemapcreate) */


static int uunames_filemapdestroy(UUNAMES *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op->indfmap != NULL) {
	    rs1 = u_munmap(op->indfmap,op->indfsize) ;
	    if (rs >= 0) rs = rs1 ;
	    op->indfmap = NULL ;
	    op->indfsize = 0 ;
	}

	return rs ;
}
/* end subroutine (uunames_filemapdestroy) */


static int uunames_indopen(UUNAMES *op,time_t dt)
{
	int		rs = SR_NOENT ;
	int		f = FALSE ;
	int		i ;

	for (i = 0 ; indopens[i] != NULL ; i += 1) {
	    if ((rs = (*indopens[i])(op,dt)) >= 0) {
		f = TRUE ;
	    } else if (isNotPresent(rs)) {
		rs = SR_OK ;
	    }
	    if (rs < 0) break ;
	} /* end for */

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (uunames_indopen) */


static int uunames_indopenpr(UUNAMES *op,time_t dt)
{
	int		rs ;
	char		idname[MAXPATHLEN + 1] ;

	if ((rs = mkpath3(idname,op->pr,VARDNAME,INDDNAME)) >= 0) {
	    rs = uunames_indopendname(op,idname,dt) ;
	}

	return rs ;
}
/* end subroutine (uunames_indopenpr) */


static int uunames_indopentmp(UUNAMES *op,time_t dt)
{
	int		rs ;
	const char	*tmpdname = TMPVARDNAME ;
	const char	*inddname = INDDNAME ;
	const char	*prname ;
	char		idname[MAXPATHLEN + 1] ;

	if ((rs = sfbasename(op->pr,-1,&prname)) > 0) {
	    if ((rs = mkpath3(idname,tmpdname,prname,inddname)) >= 0) {
	        rs = uunames_indopendname(op,idname,dt) ;
	    }
	} else {
	    rs = SR_INVALID ;
	}

	return rs ;
}
/* end subroutine (uunames_indopentmp) */


static int uunames_indopendname(UUNAMES *op,cchar *dname,time_t dt)
{
	int		rs ;
	int		f_ok = FALSE ;
	int		f_mk = FALSE ;
	char		indname[MAXPATHLEN + 1] ;

#if	CF_DEBUGS
	debugprintf("uunames_indopendname: dname=%s\n",dname) ;
#endif

	rs = mkpath2(indname,dname,op->dbname) ;
	if (rs < 0)
	    goto ret0 ;

	rs = uunames_indtest(op,indname,dt) ;
	f_ok = (rs > 0) ;

#if	CF_DEBUGS
	debugprintf("uunames_indopendname: uunames_indtest() rs=%d f_ok=%u\n",
		rs,f_ok) ;
#endif

	if (rs < 0)
	    goto ret0 ;

	if (((rs < 0) && (rs != SR_NOMEM)) || (! f_ok)) {
	    if ((rs = uunames_mkuunamesi(op,dname)) >= 0) {
		f_mk = TRUE ;
	        rs = uunames_indtest(op,indname,dt) ;
	        f_ok = (rs > 0) ;
#if	CF_DEBUGS
	    debugprintf("uunames_indopendname: uunames_indtest() rs=%d\n",
		rs) ;
#endif
	    }
	}

	if (((rs < 0) && (rs != SR_NOMEM)) || (! f_ok)) {
	    f_mk = TRUE ;
	    rs = uunames_indmk(op,dname,dt) ;

#if	CF_DEBUGS
	    debugprintf("uunames_indopendname: uunames_indmk() rs=%d\n",rs) ;
#endif

	}

	if (rs >= 0) {
	    rs = uunames_indmapcreate(op,indname,dt) ;
	    op->f.varind = (rs >= 0) ;
	}

	if ((rs < 0) && (rs != SR_BADFMT) && (! f_mk)) {
	    rs = uunames_indmk(op,dname,dt) ;
	    if (rs >= 0) {
		rs = uunames_indmapcreate(op,indname,dt) ;
	        op->f.varind = (rs >= 0) ;
	    }
	}

ret0:

#if	CF_DEBUGS
	debugprintf("uunames_indopendname: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (uunames_indopendname) */


static int uunames_indtest(UUNAMES *op,cchar *indname,time_t dt)
{
	struct ustat	sb ;
	time_t		ti_ind ;
	int		rs ;
	int		rs1 ;
	int		f = FALSE ;
	char		indfname[MAXPATHLEN + 1] ;

#if	CF_DEBUGS
	debugprintf("uunames_indtest: indname=%s\n",indname) ;
#endif

#if	CF_WITHENDIAN
	rs = mkfnamesuf2(indfname,indname,INDSUF,ENDIANSTR) ;
#else
	rs = mkfnamesuf1(indfname,indname,INDSUF) ;
#endif

#if	CF_DEBUGS
	debugprintf("uunames_indtest: mkfnamesuf2() rs=%d\n",rs) ;
	debugprintf("uunames_indtest: indfname=%s\n",indfname) ;
#endif

	if (rs < 0)
	    goto ret0 ;

	rs1 = u_stat(indfname,&sb) ;
	ti_ind = sb.st_mtime ;

	if ((rs1 >= 0) && ((sb.st_size == 0) || (ti_ind == 0)))
	    rs1 = SR_NOTFOUND ;

#if	CF_DEBUGS
	debugprintf("uunames_indtest: 0 rs1=%d\n",rs1) ;
#endif

	if ((rs1 >= 0) && (op->ti_mod > ti_ind))
	    rs1 = SR_NOTFOUND ;

#if	CF_DEBUGS
	debugprintf("uunames_indtest: 1 rs1=%d\n",rs1) ;
#endif

	if ((rs1 >= 0) && ((dt - ti_ind) >= TO_FILEMOD))
	    rs1 = SR_TIMEDOUT ;

#if	CF_DEBUGS
	debugprintf("uunames_indtest: 2 rs1=%d\n",rs1) ;
#endif

	f = (rs1 >= 0) ;

ret0:

#if	CF_DEBUGS
	debugprintf("uunames_indtest: ret rs=%d f=%u\n",rs,f) ;
#endif

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (uunames_indtest) */


/* ARGSUSED */
static int uunames_indmk(UUNAMES *op,cchar *dname,time_t dt)
{
	int		rs ;
	int		c = 0 ;
	char		indname[MAXPATHLEN + 1] ;

/* check the given directory for writability */

	if ((rs = checkdname(dname)) == SR_NOENT) {
	    rs = mkdirs(dname,TMPDMODE) ;
	}
	if (rs >= 0) {
	    rs = mkpath2(indname,dname,op->dbname) ;
	}

#if	CF_DEBUGS
	debugprintf("uunames_indmk: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (uunames_indmk) */


static int uunames_indclose(UUNAMES *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op->f.varind) {
	    op->f.varind = FALSE ;
	    rs1 = uunames_indmapdestroy(op) ;
	    if (rs >= 0) rs = rs1 ;
	}

	return rs ;
}
/* end subroutine (uunames_indclose) */


/* make the index */
static int uunames_mkuunamesi(op,dname)
UUNAMES		*op ;
const char	dname[] ;
{
	SPAWNPROC	ps ;

	vecstr	envs ;

	pid_t	cpid ;

	int	rs ;
	int	i, cstat, cex ;
	int	prlen = 0 ;

	const char	*varprmkuu = VARPRMKUU ;
	const char	*pn = PROG_MKUUNAMES ;
	const char	*av[10] ;
	const char	**ev ;

	char	progfname[MAXPATHLEN + 1] ;
	char	dbname[MAXPATHLEN + 1] ;


#if	CF_DEBUGS
	debugprintf("uunames_mkuunamesi: dname=%s\n",dname) ;
	debugprintf("uunames_mkuunamesi: pr=%s\n",op->pr) ;
#endif

	if (dname == NULL)
	    return SR_FAULT ;

	if (dname[0] == '\0')
	    return SR_INVALID ;

	rs = mkpath2(dbname,dname,op->dbname) ;
	if (rs < 0)
	    goto ret0 ;

	rs = vecstr_start(&envs,10,VECSTR_OCOMPACT) ;
	if (rs < 0)
	    goto ret0 ;

#if	CF_GETPROGROOT

	rs = getprogroot(op->pr,prnames,&prlen,progfname,pn) ;

#if	CF_DEBUGS
	debugprintf("dialuux: getprogroot() rs=%d prlen=%u pr=%t\n",
		rs,prlen,progfname,prlen) ;
#endif

	if (rs == 0)
	    rs = mkpath1(progfname,pn) ;

#ifdef	COMMENT
	if ((rs >= 0) && (prlen > 0)) {
	    rs = vecstr_envadd(&envs,varprmkuu,progfname,prlen) ;
	} else if (rs >= 0)
	    rs = vecstr_envadd(&envs,varprmkuu,op->pr,-1) ;
#endif /* COMMENT */

#else /* CF_GETPROGROOT */

	rs = prgetprogpath(op->pr,progfname,pn,-1) ;
	if (rs == 0)
	    rs = mkpath1(progfname,progmkuunamesi) ;

#if	CF_DEBUGS
	debugprintf("uunames_mkuunamesi: prgetprogpath() rs=%d\n",rs) ;
#endif

#endif /* CF_GETPROGROOT */

#if	CF_DEBUGS
	debugprintf("uunames_mkuunamesi: rs=%d\n",rs) ;
	debugprintf("uunames_mkuunamesi: progfname=%t\n",
		progfname,strnlen(progfname,40)) ;
#endif

	if (rs < 0)
	    goto ret2 ;

/* setup environment for child process */

	if (rs >= 0)
	    rs = uunames_envpaths(op,&envs) ;

	if (rs >= 0)
	    rs = vecstr_envadd(&envs,varprmkuu,op->pr,-1) ;

	if (rs >= 0)
	    rs = vecstr_envadd(&envs,VARDBNAME,dbname,-1) ;

	if (rs >= 0)
	    rs = vecstr_defenvs(&envs,envsys) ;

	if (rs >= 0)
	    rs = vecstr_defenvs(&envs,prnames) ;

	if (rs >= 0)
	    rs = vecstr_defenvs(&envs,envdefs) ;

	if (rs < 0)
	    goto ret2 ;

/* setup arguments */

	i = 0 ;
	av[i++] = pn ;
	av[i++] = NULL ;

/* go */

	vecstr_getvec(&envs,(const char ***) &ev) ;

	memset(&ps,0,sizeof(SPAWNPROC)) ;
	ps.opts |= SPAWNPROC_OIGNINTR ;
	ps.opts |= SPAWNPROC_OSETPGRP ;

	for (i = 0 ; i < 3 ; i += 1) {
	    ps.disp[i] = (i != 2) ? SPAWNPROC_DCLOSE : SPAWNPROC_DINHERIT ;
	}

	rs = spawnproc(&ps,progfname,av,ev) ;
	cpid = rs ;

ret2:
	vecstr_finish(&envs) ;

ret1:
	if (rs < 0)
	    goto ret0 ;

	cstat = 0 ;
	rs = u_waitpid(cpid,&cstat,0) ;

#if	CF_DEBUGS
	debugprintf("uunames_mkuunamesi: u_waitpid() rs=%d\n",rs) ;
#endif

	if (rs >= 0) {

	    cex = 0 ;
	    if (WIFSIGNALED(cstat))
	        rs = SR_UNATCH ;	/* protocol not attached */

#if	CF_DEBUGS
	    debugprintf("uunames_mkuunamesi: signaled? rs=%d\n",rs) ;
#endif

	    if ((rs >= 0) && WIFEXITED(cstat)) {

	        cex = WEXITSTATUS(cstat) ;

	        if (cex != 0)
	            rs = SR_LIBBAD ;

#if	CF_DEBUGS
		debugprintf("uunames_mkuunamesi: exited? cex=%d rs=%d\n",
		cex,rs) ;
#endif

	    }

	} /* end if (process finished) */

ret0:

#if	CF_DEBUGS
	debugprintf("uunames_mkuunamesi: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (uunames_mkuunamesi) */


static int uunames_envpaths(op,elp)
UUNAMES		*op ;
vecstr		*elp ;
{
	vecstr	pathcomps ;

	int	rs ;
	int	i ;
	int	opts ;
	int	size ;
	int	bl, pl ;

	const char	*subdname ;
	const char	*np ;

	char	pathbuf[MAXPATHLEN + 1] ;
	char	*bp = NULL ;
	char	*vp ;


	opts = VECSTR_OORDERED | VECSTR_OSTSIZE ;
	rs = vecstr_start(&pathcomps,40,opts) ;
	if (rs < 0)
	    goto ret0 ;

	for (i = 0 ; envpops[i].name != NULL ; i += 1) {

	    np = envpops[i].name ;

	    subdname = envpops[i].sub1dname ;
	    if ((rs >= 0) && (subdname != NULL)) {

	        rs = mkpath2(pathbuf,op->pr,subdname) ;
	        pl = rs ;
	        if (rs >= 0)
	            rs = vecstr_add(&pathcomps,pathbuf,pl) ;

	    } /* end if */

	    subdname = envpops[i].sub2dname ;
	    if ((rs >= 0) && (subdname != NULL)) {

	        rs = mkpath2(pathbuf,op->pr,subdname) ;
	        pl = rs ;
	        if (rs >= 0)
	            rs = vecstr_add(&pathcomps,pathbuf,pl) ;

	    } /* end if */

	    if ((rs >= 0) && ((vp = getenv(np)) != NULL)) {
	        rs = vecstr_loadpath(&pathcomps,vp) ;
	    }

	    if (rs >= 0) {
	        size = vecstr_strsize(&pathcomps) ;
	    }

	    if ((rs >= 0) && ((rs = uc_malloc(size,&bp)) >= 0)) {

	        rs = mkpathval(&pathcomps,bp,(size-1)) ;
	        bl = rs ;
	        if (rs >= 0)
	            rs = vecstr_envadd(elp,np,bp,bl) ;

	        uc_free(bp) ;

	    } /* end if (memory allocation) */

	    vecstr_delall(&pathcomps) ;

	    if (rs < 0)
	        break ;

	} /* end for */

ret1:
	vecstr_finish(&pathcomps) ;

ret0:
	return rs ;
}
/* end subroutine (uunames_envpaths) */


static int uunames_indlist(op)
UUNAMES		*op ;
{
	struct liner	le ;

	uint	lineoff = 0 ;

	int	rs = SR_OK ;
	int	ml ;
	int	len ;
	int	n = 0 ;

	const char	*mp ;
	const char	*tp ;
	const char	*filemagic = UUNAMES_DBMAGICSTR ;


	mp = (const char *) op->indfmap ;
	ml = op->indfsize ;

	lineoff = 0 ;
	while ((tp = strnchr(mp,ml,'\n')) != NULL) {

	    len = ((tp + 1) - mp) ;
	    le.lp = mp ;
	    le.ll = (len - 1) ;

#if	CF_DEBUGS
	debugprintf("mkuunames_indlist: lineoff=%u line=>%t<\n",
		lineoff,le.lp,le.ll) ;
#endif

	    if (lineoff > 0) {

		if ((le.ll > 0) && (le.lp[0] != '#')) {
		    n += 1 ;
		    rs = vecobj_add(&op->list,&le) ;
		}

	    } else {

		if ((le.ll == 0) || (strncmp(le.lp,filemagic,le.ll) != 0))
		    rs = SR_LIBBAD ;

	    }

	    if (rs < 0)
	    	break ;

	    lineoff += len ;
	    mp += len ;
	    ml -= len ;

	} /* end while (processing lines) */

ret0:

#if	CF_DEBUGS
	debugprintf("mkuunames_indlist: ret rs=%d n=%u\n",rs,n) ;
#endif

	return (rs >= 0) ? n : rs ;
}
/* end subroutine (uunames_indlist) */


/* ARGSUSED */
static int uunames_indcheck(UUNAMES *op,time_t dt)
{
	int		rs = SR_OK ;
	int		f = FALSE ;

	if (op->indfmap == NULL) rs = SR_NOTFOUND ;

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (uunames_indcheck) */


static int checkdname(cchar *dname)
{
	int		rs = SR_OK ;

	if (dname[0] == '/') {
	    USTAT	sb ;
	    if ((rs = u_stat(dname,&sb)) >= 0) {
		if (S_ISDIR(sb.st_mode)) {
	    	    rs = perm(dname,-1,-1,NULL,W_OK) ;
		} else {
	            rs = SR_NOTDIR ;
		}
	    }
	} else {
	    rs = SR_INVALID ;
	}

	return rs ;
}
/* end subroutine (checkdname) */


#ifdef	COMMENT

static int mkindfname(buf,dname,name,suf,end)
char		buf[] ;
const char	dname[] ;
const char	name[] ;
const char	suf[] ;
const char	end[] ;
{
	int	rs = SR_OK ;
	int	buflen = MAXPATHLEN ;
	int	dnl = 0 ;
	int	i = 0 ;


	if (rs >= 0) {
	    rs = storebuf_strw(buf,buflen,i,dname,-1) ;
	    i += rs ;
	    dnl = rs ;
	}

	if ((rs >= 0) && (dname[dnl - 1] != '/')) {
	    rs = storebuf_char(buf,buflen,i,'/') ;
	    i += rs ;
	}

	if (rs >= 0) {
	    rs = storebuf_strw(buf,buflen,i,name,-1) ;
	    i += rs ;
	}

	if (rs >= 0) {
	    rs = storebuf_char(buf,buflen,i,'.') ;
	    i += rs ;
	}

	if (rs >= 0) {
	    rs = storebuf_strw(buf,buflen,i,suf,-1) ;
	    i += rs ;
	}

	if (rs >= 0) {
	    rs = storebuf_strw(buf,buflen,i,end,-1) ;
	    i += rs ;
	}

	return (rs >= 0) ? i : rs ;
}
/* end subroutine (mkindfname) */

#endif /* COMMENT */


static int vecstr_defenvs(vecstr *elp,cchar **ea)
{
	int		rs = SR_OK ;
	int		i ;
	int		c = 0 ;
	char		*cp ;

	for (i = 0 ; ea[i] != NULL ; i += 1) {
	    if ((cp = getenv(ea[i])) != NULL) {
		rs = vecstr_envadd(elp,ea[i],cp,-1) ;
	    }
	    if (rs < 0) break ;
	} /* end for */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (vecstr_defenvs) */


static int vecstr_loadpath(vecstr *clp,cchar *pp)
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		cl ;
	int		c = 0 ;
	char		tmpfname[MAXPATHLEN + 1] ;
	char		*cp ;

	while ((cp = strpbrk(pp,":;")) != NULL) {

	    cl = pathclean(tmpfname,pp,(cp - pp)) ;

	    rs1 = vecstr_findn(clp,tmpfname,cl) ;
	    if (rs1 == SR_NOTFOUND) {
	        c += 1 ;
		rs = vecstr_add(clp,tmpfname,cl) ;
	    }

	    if ((rs >= 0) && (cp[0] == ';')) {
		rs = vecstr_adduniq(clp,";",1) ;
	    }

	    pp = (cp + 1) ;
	    if (rs < 0) break ;
	} /* end while */

	if ((rs >= 0) && (pp[0] != '\0')) {

	    cl = pathclean(tmpfname,pp,-1) ;

	    rs1 = vecstr_findn(clp,tmpfname,cl) ;
	    if (rs1 == SR_NOTFOUND) {
	        c += 1 ;
	        rs = vecstr_add(clp,tmpfname,cl) ;
	    }

	} /* end if (trailing one) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (vecstr_loadpath) */


static int mkpathval(clp,vbuf,vbuflen)
vecstr		*clp ;
char		vbuf[] ;
int		vbuflen ;
{
	int		rs = SR_OK ;
	int		i ;
	int		sch ;
	int		c = 0 ;
	int		rlen = 0 ;
	int		f_semi = FALSE ;
	const char	*cp ;

	if (vbuflen >= 0) {
	vbuf[0] = '\0' ;
	for (i = 0 ; vecstr_get(clp,i,&cp) >= 0 ; i += 1) {
	    if (cp != NULL) {

	        if (cp[0] != ';') {

	            if (c++ > 0) {
	                if (f_semi) {
	                    f_semi = FALSE ;
	                    sch = ';' ;
	                } else {
	                    sch = ':' ;
		        }

	                rs = storebuf_char(vbuf,vbuflen,rlen,sch) ;
	                rlen += rs ;

	            } /* end if */

	            if (rs >= 0) {
	                rs = storebuf_strw(vbuf,vbuflen,rlen,cp,-1) ;
	                rlen += rs ;
	            }
    
	        } else {
	            f_semi = TRUE ;
	        }

	    }
	    if (rs < 0) break ;
	} /* end for */
	} else {
	    rs = SR_NOANODE ;
	}

	return (rs >= 0) ? rlen : rs ;
}
/* end subroutine (mkpathval) */


/* find if two entries match (we don't need a "comparison") */
static int vesrch(void *v1p,void *v2p)
{
	struct liner	**e1pp = (struct liner **) v1p ;
	struct liner	**e2pp = (struct liner **) v2p ;
	struct liner	*l1, *l2 ;
	int		rc = 0 ;
	l1 = *e1pp ;
	l2 = *e2pp ;
	if (l1 != NULL) {
	    if (l2 != NULL) {
		if ((rc = (l1->lp[0] - l2->lp[0])) == 0) {
	    	    if ((rc = strncmp(l1->lp,l2->lp,l1->ll)) == 0) {
	    		rc = (l1->ll - l2->ll) ;
		    }
		}
	    } else {
	        rc = +1 ;
	    }
	} else {
	    rc = -1 ;
	}
	return rc ;
}
/* end subroutine (vesrch) */


