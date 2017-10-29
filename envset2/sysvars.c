/* sysvars */

/* interface to query the system-variable database */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_INORDER	0		/* create indices as encountered */
#define	CF_TMPPRNAME	0		/* use a PR-name in TMP */
#define	CF_INSEQ	1		/* try to open in-sequence */
#define	CF_ACTIVE	1		/* actually become active */
#define	CF_MKSYSVARS	1		/* call 'sysvar' program */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This little object provides access to the SYSVARS database
	and index (if any).

	The "in-sequence" procedure for finding indices is:

	%R/var/%D
	/var/tmp/%{PRN}/%D
	/var/tmp/%D
	%T/$D
	/tmp/%{PRN}/%D
	/tmp/%D

	where:

	%R		program-root
	%D		DB-name
	%T		user's TMPDIR
	%{PRN}		program-root name


*******************************************************************************/


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
#include	<char.h>
#include	<vecobj.h>
#include	<vecstr.h>
#include	<spawnproc.h>
#include	<storebuf.h>
#include	<ids.h>
#include	<ascii.h>
#include	<field.h>
#include	<hdbstr.h>
#include	<sbuf.h>
#include	<expcook.h>
#include	<dirseen.h>
#include	<localmisc.h>

#include	"sysvars.h"
#include	"var.h"


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

#ifndef	VARWSTATION
#define	VARWSTATION	"ESTATION"
#endif

#ifndef	VARPWD
#define	VARPWD		"PWD"
#endif

#ifndef	VARTMPDNAME
#define	VARTMPDNAME	"TMPDIR"
#endif

#ifndef	VARPRLOCAL
#define	VARPRLOCAL	"LOCAL"
#endif

#undef	VARSVDBNAME
#define	VARSVDBNAME	"SYSVARS_DBNAME"

#undef	VARSVPR
#define	VARSVPR		"SYSVARS_PROGRAMROOT"

#ifndef	TMPDNAME
#define	TMPDNAME	"/tmp"
#endif

#ifndef	TMPVARDNAME
#define	TMPVARDNAME	"/var/tmp"
#endif

#ifndef	VCNAME
#define	VCNAME		"var"
#endif

#define	INDDNAME	"sysvars"

#define	INDNAME		"sysvars"
#define	INDSUF		"vi"

#ifndef	KBUFLEN
#define	KBUFLEN		120
#endif

#define	TO_FILEMOD	(60 * 24 * 3600)
#define	TO_MKWAIT	(5 * 50)

#define	PROG_MKSYSVARSI	"sysvar"

#ifndef	ENDIANSTR
#ifdef	ENDIAN
#if	(ENDIAN == 0)
#define	ENDIANSTR	"0"
#else
#define	ENDIANSTR	"1"
#endif
#else
#define	ENDIANSTR	"1"
#endif
#endif

#ifndef	DEFINITFNAME
#define	DEFINITFNAME	"/etc/default/init"
#endif

#ifndef	DEFLOGFNAME
#define	DEFLOGFNAME	"/etc/default/login"
#endif

#ifndef	DEFNVARS
#define	DEFNVARS	20
#endif


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
extern int	nextfield(const char *,int,const char **) ;
extern int	vecstr_envadd(vecstr *,const char *,const char *,int) ;
extern int	vecstr_adduniq(vecstr *,const char *,int) ;
extern int	pathclean(char *,const char *,int) ;
extern int	perm(const char *,uid_t,gid_t,gid_t *,int) ;
extern int	sperm(IDS *,struct ustat *,int) ;
extern int	getpwd(char *,int) ;
extern int	mkdirs(const char *,mode_t) ;
extern int	hasuc(const char *,int) ;
extern int	isalnumlatin(int) ;
extern int	strpcmp(const char *,const char *) ;

extern int	sysvarprocget(HDBSTR *,const char *) ;
extern int	sysvarprocset(HDBSTR *,const char *,int) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strwcpylc(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;
extern char	*strnpbrk(const char *,int,const char *) ;


/* exported variables */

SYSVARS_OBJ	sysvars = {
	"sysvars",
	sizeof(SYSVARS),
	sizeof(SYSVARS_CUR)
} ;


/* local structures */

struct subinfo_flags {
	uint		id:1 ;
} ;

struct subinfo {
	IDS		id ;
	struct subinfo_flags	f ;
	time_t		daytime ;
} ;


/* forward references */

static int	sysvars_infoloadbegin(SYSVARS *,const char *,const char *) ;
static int	sysvars_infoloadend(SYSVARS *) ;
static int	sysvars_indopen(SYSVARS *,struct subinfo *) ;

#if	CF_INSEQ
#else

static int	sysvars_indopens(SYSVARS *,struct subinfo *,int) ;
static int	sysvars_indopenpr(SYSVARS *,struct subinfo *,int) ;
static int	sysvars_indopentmp(SYSVARS *,struct subinfo *,int) ;
static int	sysvars_indopendname(SYSVARS *,struct subinfo *,
const char *,int) ;
static int	sysvars_indcheck(SYSVARS *,const char *,time_t) ;

#endif /* CF_INSEQ */

static int	sysvars_indclose(SYSVARS *) ;
static int	sysvars_mksysvarsi(SYSVARS *,struct subinfo *,const char *) ;
static int	sysvars_indmk(SYSVARS *,const char *) ;
static int	sysvars_indmkdata(SYSVARS *,const char *,mode_t) ;
static int	sysvars_indopenseq(SYSVARS *,struct subinfo *) ;
static int	sysvars_loadcooks(SYSVARS *,EXPCOOK *) ;
static int	sysvars_indopenalt(SYSVARS *,struct subinfo *,DIRSEEN *) ;

static int	subinfo_start(struct subinfo *) ;
static int	subinfo_ids(struct subinfo *) ;
static int	subinfo_finish(struct subinfo *) ;

static int	checkdname(const char *) ;

#ifdef	COMMENT
static int	mkindfname(char *,const char *,const char *,const char *,
			const char *) ;
#endif

static int	istermrs(int) ;


/* local variables */

static const char	*envdefs[] = {
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
	VARPWD,
	VARTZ,
	VARWSTATION,
	NULL
} ;

/* use fixed locations for security reasons (like we care!) */
static const char	*prbins[] = {
	"bin",
	"sbin",
	NULL
} ;

#if	CF_INSEQ
#else

static int	(*indopens[])(SYSVARS *,struct subinfo *,int) = {
	sysvars_indopenpr,
	sysvars_indopentmp,
	NULL
} ;

#endif /* CF_INSEQ */

static const char	*sysfnames[] = {
	DEFINITFNAME,	
	DEFLOGFNAME,
	NULL
} ;

static const char	*dbdirs[] = {
	"%R/var",
	"/var/tmp/%{PRN}",
	"/var/tmp",
	"/tmp/%{PRN}",
	"/tmp",
	"%T/%{PRN}",
	"%T",
	NULL
} ;

static const int	termrs[] = {
	SR_FAULT,
	SR_INVALID,
	SR_NOMEM,
	SR_NOANODE,
	SR_BADFMT,
	SR_NOSPC,
	SR_NOSR,
	SR_NOBUFS,
	SR_BADF,
	SR_OVERFLOW,
	SR_RANGE,
	0
} ;


/* exported subroutines */


int sysvars_open(op,pr,dbname)
SYSVARS		*op ;
const char	pr[] ;
const char	dbname[] ;
{
	struct subinfo	si ;
	int		rs ;
	int		rs1 ;

	if (op == NULL) return SR_FAULT ;
	if (pr == NULL) return SR_FAULT ;

	if (pr[0] == '\0') return SR_INVALID ;

	if ((dbname == NULL) || (dbname[0] == '\0'))
	    dbname = INDNAME ;

#if	CF_DEBUGS
	debugprintf("sysvars_open: pr=%s dbname=%s\n",pr,dbname) ;
#endif

	memset(op,0,sizeof(SYSVARS)) ;

	if ((rs = subinfo_start(&si)) >= 0) {

	    rs = sysvars_infoloadbegin(op,pr,dbname) ;

#if	CF_ACTIVE
	    if (rs >= 0) {
	        rs = sysvars_indopen(op,&si) ;
	        if (rs < 0)
	            sysvars_infoloadend(op) ;
	    }
#endif /* CF_ACTIVE */

	    rs1 = subinfo_finish(&si) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (subinfo) */

	if (rs >= 0)
	    op->magic = SYSVARS_MAGIC ;

#if	CF_DEBUGS
	debugprintf("sysvars_open: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (sysvars_open) */


int sysvars_close(op)
SYSVARS		*op ;
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != SYSVARS_MAGIC) return SR_NOTOPEN ;

#if	CF_DEBUGS
	debugprintf("sysvars_close: ent\n") ;
#endif

	rs1 = sysvars_indclose(op) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = sysvars_infoloadend(op) ;
	if (rs >= 0) rs = rs1 ;

#if	CF_DEBUGS
	debugprintf("sysvars_close: ret rs=%d\n",rs) ;
#endif

	op->magic = 0 ;
	return rs ;
}
/* end subroutine (sysvars_close) */


int sysvars_audit(op)
SYSVARS		*op ;
{
	int		rs ;

	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != SYSVARS_MAGIC)
	    return SR_NOTOPEN ;

	rs = var_audit(&op->vind) ;

	return rs ;
}
/* end subroutine (sysvars_audit) */


int sysvars_curbegin(op,curp)
SYSVARS		*op ;
SYSVARS_CUR	*curp ;
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;

	if (op->magic != SYSVARS_MAGIC) return SR_NOTOPEN ;

	memset(curp,0,sizeof(SYSVARS_CUR)) ;
	rs = var_curbegin(&op->vind,&curp->vcur) ;

	if (rs >= 0)
	    op->ncursors += 1 ;

	return rs ;
}
/* end subroutine (sysvars_curbegin) */


int sysvars_curend(op,curp)
SYSVARS		*op ;
SYSVARS_CUR	*curp ;
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;

	if (op->magic != SYSVARS_MAGIC) return SR_NOTOPEN ;

	rs = var_curend(&op->vind,&curp->vcur) ;

	if (op->ncursors > 0)
	    op->ncursors -= 1 ;

	return rs ;
}
/* end subroutine (sysvars_curend) */


int sysvars_fetch(op,kp,kl,curp,vbuf,vlen)
SYSVARS		*op ;
const char	*kp ;
int		kl ;
SYSVARS_CUR	*curp ;
char		vbuf[] ;
int		vlen ;
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;
	if (kp == NULL) return SR_FAULT ;

	if (op->magic != SYSVARS_MAGIC) return SR_NOTOPEN ;

	rs = var_fetch(&op->vind,kp,kl,&curp->vcur,vbuf,vlen) ;

	if ((rs < 0) && (vbuf != NULL))
	    vbuf[0] = '\0' ;

	return rs ;
}
/* end subroutine (sysvars_fetch) */


int sysvars_enum(op,curp,kbuf,klen,vbuf,vlen)
SYSVARS		*op ;
SYSVARS_CUR	*curp ;
char		kbuf[] ;
int		klen ;
char		vbuf[] ;
int		vlen ;
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;
	if (kbuf == NULL) return SR_FAULT ;

	if (op->magic != SYSVARS_MAGIC) return SR_NOTOPEN ;

	rs = var_enum(&op->vind,&curp->vcur,kbuf,klen,vbuf,vlen) ;

#if	CF_DEBUGS
	debugprintf("sysvars_enum: var_enum() rs=%d\n",rs) ;
#endif

	if ((rs < 0) && (vbuf != NULL))
	    vbuf[0] = '\0' ;

	return rs ;
}
/* end subroutine (sysvars_enum) */


int sysvars_count(op)
SYSVARS		*op ;
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != SYSVARS_MAGIC) return SR_NOTOPEN ;

	rs = var_count(&op->vind) ;

	return rs ;
}
/* end subroutine (sysvars_count) */


/* private subroutines */


static int sysvars_infoloadbegin(op,pr,dbname)
SYSVARS		*op ;
const char	pr[] ;
const char	dbname[] ;
{
	int		rs = SR_OK ;
	int		pl = -1 ;
	const char	*cp ;
	char		adb[MAXPATHLEN+1] ;

#ifdef	COMMENT
	if ((dbname == NULL) || (dbname[0] == '\0'))
	    dbname = INDNAME ;
#endif

#ifdef	COMMENT /* this is bad! */
	if (dbname[0] != '/') {
	    char	pwd[MAXPATHLEN+1] ;
	    if ((rs = getpwd(pwd,MAXPATHLEN)) >= 0) {
	        rs = mkpath2(adb,pwd,dbname) ;
	        pl = rs ;
	        dbname = adb ;
	    }
	}
#endif /* COMMENT */

	if (rs >= 0) {
	    if ((rs = uc_mallocstrw(dbname,pl,&cp)) >= 0) {
	        op->pr = pr ;
	        op->dbname = cp ;
	    }
	}

	return rs ;
}
/* end subroutine (sysvars_infoloadbegin) */


static int sysvars_infoloadend(op)
SYSVARS		*op ;
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op->dbname != NULL) {
	    rs1 = uc_free(op->dbname) ;
	    if (rs >= 0) rs = rs1 ;
	    op->dbname = NULL ;
	}

	op->pr = NULL ;
	return rs ;
}
/* end subroutine (sysvars_infoloadend) */


static int sysvars_indopen(op,sip)
SYSVARS		*op ;
struct subinfo	*sip ;
{
	int		rs = SR_NOENT ;

#if	CF_INSEQ

	rs = sysvars_indopenseq(op,sip) ;

#else /* CF_INSEQ */
	{
	    int	oflags ;

#if	CF_INORDER
#else
	    oflags = 0 ;
	    rs = sysvars_indopens(op,sip,oflags) ;
#endif /* CF_INORDER */

	    if ((rs < 0) && (! istermrs(rs))) {
	        oflags = O_CREAT ;
	        rs = sysvars_indopens(op,sip,oflags) ;
	    }

	} /* end block */
#endif /* CF_INSEQ */

	return rs ;
}
/* end subroutine (sysvars_indopen) */


static int sysvars_indopenseq(op,sip)
SYSVARS		*op ;
struct subinfo	*sip ;
{
	EXPCOOK		cooks ;
	DIRSEEN		ds ;
	DIRSEEN_CUR	cur ;
	vecstr		sdirs ;
	const int	elen = MAXPATHLEN ;
	int		rs ;
	int		rs1 ;
	int		vopts ;
	int		el, pl ;
	int		i ;
	const char	**dv ;
	char		ebuf[MAXPATHLEN + 1] ;
	char		pbuf[MAXPATHLEN + 1] ;

#if	CF_DEBUGS
	debugprintf("sysvars_indopenseq: ent dbname=%s\n",op->dbname) ;
#endif

	rs = dirseen_start(&ds) ;
	if (rs < 0)
	    goto ret0 ;

	vopts = VECSTR_OCOMPACT ;
	rs = vecstr_start(&sdirs,6,vopts) ;
	if (rs < 0)
	    goto ret1 ;

	rs = expcook_start(&cooks) ;
	if (rs < 0)
	    goto ret2 ;

	rs = sysvars_loadcooks(op,&cooks) ;
	if (rs < 0)
	    goto ret3 ;

/* first phase: expand possible directory paths */

	for (i = 0 ; (rs >= 0) && (dbdirs[i] != NULL) ; i += 1) {

	    rs = expcook_exp(&cooks,0,ebuf,elen,dbdirs[i],-1) ;
	    el = rs ;

#if	CF_DEBUGS
	    debugprintf("sysvars_indopenseq: ex_expand() rs=%d\n",rs) ;
	    debugprintf("sysvars_indopenseq: ex_expand() e=%t\n",
		ebuf,strlinelen(ebuf,el,40)) ;
#endif

	    if ((rs >= 0) && (el > 0)) {
	        rs = pathclean(pbuf,ebuf,el) ;
	        pl = rs ;
	        if ((rs >= 0) && (pl > 0)) {
	            rs1 = dirseen_havename(&ds,pbuf,pl) ;
	            if (rs1 == SR_NOTFOUND)
	                rs = dirseen_add(&ds,pbuf,pl,NULL) ;
	        }
	    }

	} /* end for */

/* next phase: create DB file-paths from directories */

	if (rs >= 0) {
	    if (op->dbname[0] != '/') {

	    if ((rs = dirseen_curbegin(&ds,&cur)) >= 0) {

	        while (rs >= 0) {

	            el = dirseen_enum(&ds,&cur,ebuf,elen) ;
	            if (el == SR_NOTFOUND) break ;
	            rs = el ;

#if	CF_DEBUGS
	    debugprintf("sysvars_indopenseq: rs=%d e=%t\n",
		rs,ebuf,strlinelen(ebuf,el,40)) ;
#endif

	            if (rs >= 0) {
	                rs = mkpath2(pbuf,ebuf,op->dbname) ;
	                pl = rs ;
	                if (rs >= 0)
	                    rs = vecstr_add(&sdirs,pbuf,pl) ;
	            }

	        } /* end while */

	        dirseen_curend(&ds,&cur) ;
	    } /* end if (cursor) */

	    } else {
	                    rs = vecstr_add(&sdirs,op->dbname,-1) ;
	    } /* end if */

	} /* end if */

/* final phase: try to open all of them in-sequence */

	if (rs >= 0) {

	    rs = vecstr_getvec(&sdirs,&dv) ;
	    if (rs >= 0) {
	        rs = var_opena(&op->vind,dv) ;
	        op->f.var = (rs >= 0) ;
	    }

	    if ((rs < 0) && (! istermrs(rs)))
	        rs = sysvars_indopenalt(op,sip,&ds) ;

	} /* end if */

/* done */
done:
ret4:
ret3:
	expcook_finish(&cooks) ;

ret2:
	vecstr_finish(&sdirs) ;

ret1:
	dirseen_finish(&ds) ;

ret0:
	return rs ;
}
/* end subroutines (sysvars_indopenseq) */


static int sysvars_loadcooks(op,ecp)
SYSVARS		*op ;
EXPCOOK	*ecp ;
{
	int		rs = SR_OK ;
	const char	*tmpdname = getenv(VARTMPDNAME) ;

	if (tmpdname == NULL)
	    tmpdname = TMPDNAME ;

	if (rs >= 0)
	    rs = expcook_add(ecp,"R",op->pr,-1) ;

	if (rs >= 0)
	    rs = expcook_add(ecp,"S",INDDNAME,-1) ;

	if (rs >= 0)
	    rs = expcook_add(ecp,"T",tmpdname,-1) ;

	if (rs >= 0) {
	    const char	*prname ;
	    rs = sfbasename(op->pr,-1,&prname) ;
	    if (rs >= 0) {
	        rs = SR_NOENT ;
	        if (prname != NULL)
	            rs = expcook_add(ecp,"PRN",prname,-1) ;
	    }
	}

	return rs ;
}
/* end subroutines (sysvars_loadcooks) */


static int sysvars_indopenalt(op,sip,dsp)
SYSVARS		*op ;
struct subinfo	*sip ;
DIRSEEN		*dsp ;
{
	DIRSEEN_CUR	cur ;
	const int	elen = MAXPATHLEN ;
	int		rs ;
	int		el ;
	char		ebuf[MAXPATHLEN + 1] ;
	char		indname[MAXPATHLEN + 1] ;

	if ((rs = dirseen_curbegin(dsp,&cur)) >= 0) {

	    while (rs >= 0) {

	        el = dirseen_enum(dsp,&cur,ebuf,elen) ;
	        if (el == SR_NOTFOUND) break ;
	        rs = el ;

	        if (rs >= 0) {

#if	CF_MKSYSVAR
	            rs = sysvars_mksysvarsi(op,sip,ebuf) ;
#else /* CF_MKSYSVAR */
		    rs = SR_NOENT ;
#endif /* CF_MKSYSVAR */

	            if ((rs < 0) && (! istermrs(rs)))
	                rs = sysvars_indmk(op,ebuf) ;

	            if (rs >= 0) {
	                rs = mkpath2(indname,ebuf,op->dbname) ;
	                if (rs >= 0) {
	                    rs = var_open(&op->vind,indname) ;
	                    op->f.var = (rs >= 0) ;
	                }
	            }

	        } /* end if */

	        if ((rs >= 0) || istermrs(rs))
	            break ;

	    } /* end while */

	    dirseen_curend(dsp,&cur) ;
	} /* end if (cursor) */

	return rs ;
}
/* end subroutines (sysvars_indopenalt) */


#if	CF_INSEQ
#else

static int sysvars_indopens(op,sip,oflags)
SYSVARS		*op ;
struct subinfo	*sip ;
int		oflags ;
{
	int		rs = SR_NOENT ;
	int		i ;

	oflags = 0 ;
	for (i = 0 ; indopens[i] != NULL ; i += 1) {

	    rs = (*indopens[i])(op,sip,oflags) ;

	    if ((rs < 0) && istermrs(rs)) break ;
	    if (rs >= 0) break ;
	} /* end for */

	return rs ;
}
/* end subroutine (sysvars_indopens) */


static int sysvars_indopenpr(op,sip,oflags)
SYSVARS		*op ;
struct subinfo	*sip ;
int		oflags ;
{
	int		rs ;
	char		idname[MAXPATHLEN + 1] ;

	if ((rs = mkpath3(idname,op->pr,VCNAME,INDDNAME)) >= 0) {
	    rs = sysvars_indopendname(op,sip,idname,oflags) ;
	}

	return rs ;
}
/* end subroutine (sysvars_indopenpr) */


static int sysvars_indopentmp(op,sip,oflags)
SYSVARS		*op ;
struct subinfo	*sip ;
int		oflags ;
{
	int		rs = SR_OK ;
	const char	*tmpdname = TMPVARDNAME ;
	const char	*inddname = INDDNAME ;
	char		idname[MAXPATHLEN + 1] ;

#if	CF_TMPPRNAME
	{
	    char	*prname ;

	    rs = sfbasename(op->pr,-1,&prname) ;

	    if (rs >= 0)
	        rs = mkpath3(idname,tmpdname,prname,inddname) ;

	}
#else
	rs = mkpath1(idname,tmpdname) ;
#endif /* CF_TMPPRNAME */

	if (rs >= 0)
	    rs = sysvars_indopendname(op,sip,idname,oflags) ;

ret0:
	return rs ;
}
/* end subroutine (sysvars_indopentmp) */


static int sysvars_indopendname(op,sip,dname,oflags)
SYSVARS		*op ;
struct subinfo	*sip ;
const char	dname[] ;
int		oflags ;
{
	int		rs ;
	int		f_ok = FALSE ;
	int		f_mk = FALSE ;
	char		indname[MAXPATHLEN + 1] ;

#if	CF_DEBUGS
	debugprintf("sysvars_indopendname: dname=%s\n",dname) ;
#endif

	rs = mkpath2(indname,dname,op->dbname) ;
	if (rs < 0)
	    goto ret0 ;

	if (oflags & O_CREAT) {

	    rs = sysvars_indcheck(op,indname,sip->daytime) ;
	    f_ok = (rs > 0) ;

#if	CF_DEBUGS
	    debugprintf("sysvars_indopendname: "
	        "sysvars_indcheck() rs=%d f_ok=%u\n",
	        rs,f_ok) ;
#endif

	    if (rs < 0)
	        goto ret0 ;

	    if (((rs < 0) && (! istermrs(rs))) || (! f_ok)) {
	        rs = sysvars_mksysvarsi(op,sip,dname) ;
	        if (rs >= 0) {
	            f_mk = TRUE ;
	            rs = sysvars_indcheck(op,indname,sip->daytime) ;
	            f_ok = (rs > 0) ;
	        }
	    }

	    if (((rs < 0) && (! istermrs(rs))) || (! f_ok)) {
	        f_mk = TRUE ;
	        rs = sysvars_indmk(op,dname) ;

#if	CF_DEBUGS
	        debugprintf("sysvars_indopendname: sysvars_indmk() rs=%d\n",
	            rs) ;
#endif

	    }

	    if ((rs < 0) && (! istermrs(rs))) {
	        rs = var_open(&op->vind,indname) ;
	        op->f.var = (rs >= 0) ;
	    }

	    if ((rs < 0) && (! istermrs(rs))) {
	        if (! f_mk) {
	            rs = sysvars_indmk(op,dname) ;
	            if (rs >= 0) {
	                rs = var_open(&op->vind,indname) ;
	                op->f.var = (rs >= 0) ;
	            }
	        }
	    }

	} else {

	    rs = var_open(&op->vind,indname) ;

	} /* end if (open-only or open-create) */

ret0:

#if	CF_DEBUGS
	debugprintf("sysvars_indopendname: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (sysvars_indopendname) */


static int sysvars_indcheck(op,indname,daytime)
SYSVARS		*op ;
const char	indname[] ;
time_t		daytime ;
{
	struct ustat	sb ;
	time_t		ti_ind ;
	int		rs ;
	int		rs1 ;
	int		f = FALSE ;
	char		indfname[MAXPATHLEN + 1] ;

#if	CF_DEBUGS
	debugprintf("sysvars_indcheck: indname=%s\n",indname) ;
#endif

	if ((rs = mkfnamesuf2(indfname,indname,INDSUF,ENDIANSTR)) >= 0) {

	rs1 = u_stat(indfname,&sb) ;

	ti_ind = sb.st_mtime ;
	if ((rs1 >= 0) && (op->ti_db > ti_ind))
	    rs1 = SR_TIMEDOUT ;

	if ((rs1 >= 0) && ((daytime - ti_ind) >= TO_FILEMOD))
	    rs1 = SR_TIMEDOUT ;

	f = (rs1 >= 0) ;

	} /* end if (mkfnamesuf2) */

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (sysvars_indcheck) */

#endif /* CF_INSEQ */


static int sysvars_indmk(op,dname)
SYSVARS		*op ;
const char	dname[] ;
{
	const mode_t	operms = 0664 ;
	int		rs ;
	int		c = 0 ;
	char		indname[MAXPATHLEN + 1] ;

/* check the given directory for writability */

	rs = checkdname(dname) ;

	if (rs == SR_NOENT)
	    rs = mkdirs(dname,0775) ;

/* create the index-name */

	if (rs >= 0) {
	    if ((rs = mkpath2(indname,dname,op->dbname)) >= 0) {
	        rs = sysvars_indmkdata(op,indname,operms) ;
	        c += rs ;
	    }
	}

#if	CF_DEBUGS
	debugprintf("sysvars_indmk: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (sysvars_indmk) */


static int sysvars_indmkdata(op,indname,operms)
SYSVARS		*op ;
const char	indname[] ;
mode_t		operms ;
{
	HDBSTR		vars, *vlp = &vars ;
	int		rs ;
	int		i ;
	int		c = 0 ;
	int		f ;

	if (op == NULL) return SR_FAULT ;

	if ((rs = hdbstr_start(vlp,DEFNVARS)) >= 0) {

	    for (i = 0 ; sysfnames[i] != NULL ; i += 1) {

	        rs = sysvarprocget(vlp,sysfnames[i]) ;
	        f = FALSE ;
	        f = f || (rs == SR_NOENT) ;
	        f = f || (rs == SR_ACCESS) ;
	        if (f)
	            rs = SR_OK ;

	        c += rs ;
	        if (rs < 0) break ;
	    } /* end for */

	    if (rs >= 0)
	        rs = sysvarprocset(vlp,indname,operms) ;

	    hdbstr_finish(vlp) ;
	} /* end if (hdbstr) */

#if	CF_DEBUGS
	debugprintf("sysvars_indmkdata: ret rs=%d\n",rs) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (sysvars_indmkdata) */


static int sysvars_indclose(op)
SYSVARS		*op ;
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op->f.var) {
	    op->f.var = FALSE ;
	    rs1 = var_close(&op->vind) ;
	    if (rs >= 0) rs = rs1 ;
	}

	return rs ;
}
/* end subroutine (sysvars_indclose) */


/* make the index */
static int sysvars_mksysvarsi(op,sip,dname)
SYSVARS		*op ;
struct subinfo	*sip ;
const char	dname[] ;
{
	struct ustat	sb ;
	SPAWNPROC	ps ;
	vecstr		envs ;
	pid_t		cpid ;
	int		rs ;
	int		i, cstat, cex ;
	const char	*progmksysvarsi = PROG_MKSYSVARSI ;
	const char	*av[10] ;
	const char	**ev ;
	const char	*cp ;
	char		progfname[MAXPATHLEN + 1] ;
	char		dbname[MAXPATHLEN + 1] ;

#if	CF_DEBUGS
	debugprintf("sysvars_mksysvarsi: dname=%s\n",dname) ;
#endif

	if (dname == NULL)
	    return SR_FAULT ;

	if (dname[0] == '\0')
	    return SR_INVALID ;

	rs = mkpath2(dbname,dname,op->dbname) ;
	if (rs < 0)
	    goto ret0 ;

	rs = subinfo_ids(sip) ;		/* ensure IDs are available */
	if (rs < 0)
	    goto ret0 ;

	for (i = 0 ; prbins[i] != NULL ; i += 1) {

	    rs = mkpath3(progfname,op->pr,prbins[i],progmksysvarsi) ;

	    if (rs >= 0) {
	        rs = u_stat(progfname,&sb) ;
	        if (rs >= 0)
	            rs = sperm(&sip->id,&sb,X_OK) ;
	    }

	    if (rs >= 0)
	        break ;

	} /* end for */

#if	CF_DEBUGS
	debugprintf("mksysvarsi: pr=%s\n",op->pr) ;
	debugprintf("mksysvarsi: progfname=%s\n",progfname) ;
	debugprintf("mksysvarsi: perm() rs=%d\n",rs) ;
#endif

	if (rs < 0)
	    goto ret0 ;

	rs = vecstr_start(&envs,20,VECSTR_OCOMPACT) ;
	if (rs < 0)
	    goto ret1 ;

/* setup arguments */

	i = 0 ;
	av[i++] = progmksysvarsi ;
	av[i++] = "-s" ;
	av[i++] = NULL ;

/* setup environment */

	rs = vecstr_envadd(&envs,VARSVPR,op->pr,-1) ;

	if (rs >= 0)
	    rs = vecstr_envadd(&envs,VARSVDBNAME,dbname,-1) ;

	for (i = 0 ; (rs >= 0) && (envdefs[i] != NULL) ; i += 1) {

	    if ((cp = getenv(envdefs[i])) != NULL)
	        rs = vecstr_envadd(&envs,envdefs[i],cp,-1) ;

	    if (rs < 0) break ;
	} /* end for */

	if (rs < 0)
	    goto ret2 ;

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
	debugprintf("mksysvarsi: u_waitpid() rs=%d\n",rs) ;
#endif

	if (rs >= 0) {

	    cex = 0 ;
	    if (WIFSIGNALED(cstat))
	        rs = SR_UNATCH ;	/* protocol not attached */

#if	CF_DEBUGS
	    debugprintf("mksysvarsi: signaled? rs=%d\n",rs) ;
#endif

	    if ((rs >= 0) && WIFEXITED(cstat)) {

	        cex = WEXITSTATUS(cstat) ;

	        if (cex != 0)
	            rs = SR_LIBBAD ;

#if	CF_DEBUGS
	        debugprintf("mksysvarsi: exited? cex=%d rs=%d\n",
	            cex,rs) ;
#endif

	    } /* end if */

	} /* end if (process finished) */

ret0:

#if	CF_DEBUGS
	debugprintf("mksysvarsi: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (sysvars_mksysvarsi) */


static int subinfo_start(sip)
struct subinfo	*sip ;
{
	int		rs = SR_OK ;

	memset(sip,0,sizeof(struct subinfo)) ;

	sip->daytime = time(NULL) ;

	return rs ;
}
/* end subroutine (subinfo_start) */


static int subinfo_ids(sip)
struct subinfo	*sip ;
{
	int		rs = SR_OK ;

	if (! sip->f.id) {
	    sip->f.id = TRUE ;
	    rs = ids_load(&sip->id) ;
	}

	return rs ;
}
/* end subroutine (subinfo_ids) */


static int subinfo_finish(sip)
struct subinfo	*sip ;
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (sip->f.id) {
	    sip->f.id = FALSE ;
	    rs1 = ids_release(&sip->id) ;
	    if (rs >= 0) rs = rs1 ;
	}

	return rs ;
}
/* end subroutine (subinfo_finish) */


static int checkdname(dname)
const char	dname[] ;
{
	struct ustat	sb ;
	int		rs = SR_OK ;

	if (dname[0] != '/') {
	    rs = SR_INVALID ;
	    goto ret0 ;
	}

	rs = u_stat(dname,&sb) ;

	if ((rs >= 0) && (! S_ISDIR(sb.st_mode)))
	    rs = SR_NOTDIR ;

	if (rs >= 0)
	    rs = perm(dname,-1,-1,NULL,W_OK) ;

ret0:
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
	const int	buflen = MAXPATHLEN ;
	int		rs = SR_OK ;
	int		dnl = 0 ;
	int		i = 0 ;

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


static int istermrs(int rs)
{
	int		i ;
	int		f = FALSE ;
	for (i = 0 ; termrs[i] != 0 ; i += 1) {
	    f = (rs == termrs[i]) ;
	    if (f) break ;
	} /* end if */
	return f ;
}
/* end subroutine (istermrs) */


