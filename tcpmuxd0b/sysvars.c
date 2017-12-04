/* sysvars */

/* interface to query the system-variable database */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_TMPPRNAME	0		/* use a PR-name in TMP */
#define	CF_MKSYSVARS	1		/* call 'sysvar' program */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This little object provides access to the SYSVARS database and index (if
        any).

	The "in-sequence" procedure for finding indices is:

	%R/var/%D
	/var/tmp/%{PRN}/%D
	/var/tmp/%D
	%T/$D
	/tmp/%{PRN}/%D
	/tmp/%D

	Where:

	%R		program-root
	%D		DB-name
	%T		user's TMPDIR
	%{PRN}		program-root name


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<limits.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<endianstr.h>
#include	<char.h>
#include	<vecobj.h>
#include	<vecstr.h>
#include	<spawnproc.h>
#include	<storebuf.h>
#include	<ids.h>
#include	<hdbstr.h>
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

#ifndef	KBUFLEN
#define	KBUFLEN		120
#endif

#define	TO_FILEMOD	(60 * 24 * 3600)
#define	TO_MKWAIT	(5 * 50)

#define	PROG_MKSYSVARSI	"sysvar"

#ifndef	DEFINITFNAME
#define	DEFINITFNAME	"/etc/default/init"
#endif

#ifndef	DEFLOGFNAME
#define	DEFLOGFNAME	"/etc/default/login"
#endif

#ifndef	DEFNVARS
#define	DEFNVARS	20
#endif


#define	SUBINFO		struct subinfo
#define	SUBINFO_FL	struct subinfo_flags


/* external subroutines */

extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	sncpy4(char *,int, const char *,cchar *,cchar *,cchar *) ;
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
extern int	isNotPresent(int) ;

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
	SUBINFO_FL	f ;
	time_t		daytime ;
} ;


/* forward references */

static int	sysvars_infoloadbegin(SYSVARS *,const char *,const char *) ;
static int	sysvars_infoloadend(SYSVARS *) ;
static int	sysvars_indopen(SYSVARS *,SUBINFO *) ;

static int	sysvars_indclose(SYSVARS *) ;
static int	sysvars_indmk(SYSVARS *,const char *) ;
static int	sysvars_indmkdata(SYSVARS *,const char *,mode_t) ;
static int	sysvars_indopenseq(SYSVARS *,SUBINFO *) ;
static int	sysvars_indopenseqer(SYSVARS *,SUBINFO *,DIRSEEN *,
			vecstr *,EXPCOOK *) ;
static int	sysvars_loadcooks(SYSVARS *,EXPCOOK *) ;
static int	sysvars_indopenalt(SYSVARS *,SUBINFO *,DIRSEEN *) ;

#if	CF_MKSYSVARS
static int	sysvars_mksysvarsi(SYSVARS *,SUBINFO *,cchar *) ;
#endif

static int	subinfo_start(SUBINFO *) ;
static int	subinfo_ids(SUBINFO *) ;
static int	subinfo_finish(SUBINFO *) ;

static int	checkdname(const char *) ;

#ifdef	COMMENT
static int	mkindfname(char *,cchar *,cchar *,cchar *,cchar *) ;
#endif


/* local variables */

#if	CF_MKSYSVARS
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
#endif /* CF_MKSYSVARS */

/* use fixed locations for security reasons (like we care!) */
#if	CF_MKSYSVARS
static const char	*prbins[] = {
	"bin",
	"sbin",
	NULL
} ;
#endif /* CF_MKSYSVARS */

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


/* exported subroutines */


int sysvars_open(SYSVARS *op,cchar *pr,cchar *dbname)
{
	SUBINFO		si ;
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
	    if ((rs = sysvars_infoloadbegin(op,pr,dbname)) >= 0) {
	        rs = sysvars_indopen(op,&si) ;
	        if (rs < 0)
	            sysvars_infoloadend(op) ;
	    }
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


int sysvars_close(SYSVARS *op)
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


int sysvars_audit(SYSVARS *op)
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != SYSVARS_MAGIC) return SR_NOTOPEN ;

	rs = var_audit(&op->vind) ;

	return rs ;
}
/* end subroutine (sysvars_audit) */


int sysvars_curbegin(SYSVARS *op,SYSVARS_CUR *curp)
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;

	if (op->magic != SYSVARS_MAGIC) return SR_NOTOPEN ;

	memset(curp,0,sizeof(SYSVARS_CUR)) ;
	if ((rs = var_curbegin(&op->vind,&curp->vcur)) >= 0) {
	    op->ncursors += 1 ;
	}

	return rs ;
}
/* end subroutine (sysvars_curbegin) */


int sysvars_curend(SYSVARS *op,SYSVARS_CUR *curp)
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;

	if (op->magic != SYSVARS_MAGIC) return SR_NOTOPEN ;

	if ((rs = var_curend(&op->vind,&curp->vcur)) >= 0) {
	    op->ncursors -= 1 ;
	}

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

	if ((rs < 0) && (vbuf != NULL)) {
	    vbuf[0] = '\0' ;
	}

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


int sysvars_count(SYSVARS *op)
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != SYSVARS_MAGIC) return SR_NOTOPEN ;

	rs = var_count(&op->vind) ;

	return rs ;
}
/* end subroutine (sysvars_count) */


/* private subroutines */


static int sysvars_infoloadbegin(SYSVARS *op,cchar *pr,cchar *dbname)
{
	int		rs ;
	int		size = 0 ;
	char		*bp ;

	size += (strlen(pr)+1) ;
	size += (strlen(dbname)+1) ;
	if ((rs = uc_malloc(size,&bp)) >= 0) {
	    op->a = bp ;
	    op->pr = bp ;
	    bp = (strwcpy(bp,pr,-1)+1) ;
	    op->dbname = bp ;
	    bp = (strwcpy(bp,dbname,-1)+1) ;
	}

	return rs ;
}
/* end subroutine (sysvars_infoloadbegin) */


static int sysvars_infoloadend(SYSVARS *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op->a != NULL) {
	    rs1 = uc_free(op->a) ;
	    if (rs >= 0) rs = rs1 ;
	    op->a = NULL ;
	}

	op->pr = NULL ;
	op->dbname = NULL ;
	return rs ;
}
/* end subroutine (sysvars_infoloadend) */


static int sysvars_indopen(SYSVARS *op,SUBINFO *sip)
{
	int		rs ;

	rs = sysvars_indopenseq(op,sip) ;

	return rs ;
}
/* end subroutine (sysvars_indopen) */


static int sysvars_indopenseq(SYSVARS *op,SUBINFO *sip)
{
	EXPCOOK		cooks ;
	DIRSEEN		ds ;
	vecstr		sdirs ;
	int		rs ;
	int		rs1 ;

#if	CF_DEBUGS
	debugprintf("sysvars_indopenseq: ent dbname=%s\n",op->dbname) ;
#endif

	if ((rs = dirseen_start(&ds)) >= 0) {
	    const int	vo = VECSTR_OCOMPACT ;
	    if ((rs = vecstr_start(&sdirs,6,vo)) >= 0) {
	        if ((rs = expcook_start(&cooks)) >= 0) {
		    if ((rs = sysvars_loadcooks(op,&cooks)) >= 0) {
			rs = sysvars_indopenseqer(op,sip,&ds,&sdirs,&cooks) ;
		    }
		    rs1 = expcook_finish(&cooks) ;
		    if (rs >= 0) rs = rs1 ;
		} /* end if (expcooks_loadcooks) */
		rs1 = vecstr_finish(&sdirs) ;
		if (rs >= 0) rs = rs1 ;
	    } /* end if (vecstr) */
	    rs1 = dirseen_finish(&ds) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (dirseen) */

	return rs ;
}
/* end subroutines (sysvars_indopenseq) */


static int sysvars_indopenseqer(SYSVARS *op,SUBINFO *sip,DIRSEEN *dsp,
		vecstr *sdp,EXPCOOK *ecp)
{
	const int	elen = MAXPATHLEN ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		i ;
	int		el, pl ;
	char		ebuf[MAXPATHLEN + 1] ;
	char		pbuf[MAXPATHLEN + 1] ;

/* first phase: expand possible directory paths */

	for (i = 0 ; (rs >= 0) && (dbdirs[i] != NULL) ; i += 1) {
	    if ((rs = expcook_exp(ecp,0,ebuf,elen,dbdirs[i],-1)) > 0) {
	        el = rs ;
	        if ((rs = pathclean(pbuf,ebuf,el)) > 0) {
	            pl = rs ;
	                rs1 = dirseen_havename(dsp,pbuf,pl) ;
	                if (rs1 == SR_NOTFOUND) {
	                    rs = dirseen_add(dsp,pbuf,pl,NULL) ;
		        }
	        } /* end if (pathclean) */
	    } /* end if (expcook_exp) */
	} /* end for */

/* next phase: create DB file-paths from directories */

	if (rs >= 0) {
	    if (op->dbname[0] != '/') {
		DIRSEEN_CUR	cur ;

	    if ((rs = dirseen_curbegin(dsp,&cur)) >= 0) {

	        while (rs >= 0) {
	            el = dirseen_enum(dsp,&cur,ebuf,elen) ;
	            if (el == SR_NOTFOUND) break ;
	            rs = el ;

#if	CF_DEBUGS
	    		debugprintf("sysvars_indopenseq: rs=%d e=%t\n",
			rs,ebuf,strlinelen(ebuf,el,40)) ;
#endif

	            if (rs >= 0) {
	                if ((rs = mkpath2(pbuf,ebuf,op->dbname)) >= 0) {
	                    pl = rs ;
	                    rs = vecstr_add(sdp,pbuf,pl) ;
			}
	            }

	        } /* end while */

	        dirseen_curend(dsp,&cur) ;
	    } /* end if (cursor) */

	    } else {
		rs = vecstr_add(sdp,op->dbname,-1) ;
	    } /* end if */

	} /* end if (ok) */

/* final phase: try to open all of them in-sequence */

	if (rs >= 0) {
	    cchar	**dv ;
	    if ((rs = vecstr_getvec(sdp,&dv)) >= 0) {
	        if ((rs = var_opena(&op->vind,dv)) >= 0) {
	            op->f.var = TRUE ;
		}
	        if ((rs < 0) && isNotPresent(rs)) {
	            rs = sysvars_indopenalt(op,sip,dsp) ;
		}
	    }
	} /* end if (ok) */

	return rs ;
}
/* end subroutine (sysvars_indopenseqer) */


static int sysvars_loadcooks(SYSVARS *op,EXPCOOK *ecp)
{
	int		rs = SR_OK ;
	int		i ;
	int		kch ;
	int		vl ;
	cchar		*tmpdname = getenv(VARTMPDNAME) ;
	cchar		*ks = "RST" ;
	cchar		*vp ;
	char		kbuf[2] ;

	if (tmpdname == NULL) tmpdname = TMPDNAME ;

	kbuf[1] = '\0' ;
	for (i = 0 ; (rs >= 0) && (ks[i] != '\0') ; i += 1) {
	    kch = MKCHAR(ks[i]) ;
	    vp = NULL ;
	    vl = -1 ;
	    switch (kch) {
	    case 'R':
		vp = op->pr ;
		break ;
	    case 'S':
		vp = INDDNAME ;
		break ;
	    case 'T':
		vp = tmpdname ;
		break ;
	    } /* end switch */
	    if ((rs >= 0) && (vp != NULL)) {
		kbuf[0] = kch ;
		rs = expcook_add(ecp,kbuf,vp,vl) ;
	    }
	} /* end for */

	if (rs >= 0) {
	    cchar	*prname ;
	    if ((rs = sfbasename(op->pr,-1,&prname)) >= 0) {
	        rs = SR_NOENT ;
	        if (prname != NULL) {
	            rs = expcook_add(ecp,"PRN",prname,-1) ;
		}
	    }
	}

	return rs ;
}
/* end subroutines (sysvars_loadcooks) */


static int sysvars_indopenalt(SYSVARS *op,SUBINFO *sip,DIRSEEN *dsp)
{
	DIRSEEN_CUR	cur ;
	int		rs ;
	int		rs1 ;

	if ((rs = dirseen_curbegin(dsp,&cur)) >= 0) {
	    const int	elen = MAXPATHLEN ;
	    int		el ;
	    char	ebuf[MAXPATHLEN + 1] ;
	    char	indname[MAXPATHLEN + 1] ;

	    while (rs >= 0) {

	        el = dirseen_enum(dsp,&cur,ebuf,elen) ;
	        if (el == SR_NOTFOUND) break ;
	        rs = el ;

	        if (rs >= 0) {

#if	CF_MKSYSVARS
	            rs = sysvars_mksysvarsi(op,sip,ebuf) ;
#else /* CF_MKSYSVARS */
		    rs = SR_NOENT ;
#endif /* CF_MKSYSVARS */

	            if ((rs < 0) && isNotPresent(rs)) {
	                rs = sysvars_indmk(op,ebuf) ;
		    }

	            if (rs >= 0) {
	                if ((rs = mkpath2(indname,ebuf,op->dbname)) >= 0) {
	                    rs = var_open(&op->vind,indname) ;
	                    op->f.var = (rs >= 0) ;
	                }
	            }

	        } /* end if */

	        if ((rs >= 0) || (! isNotPresent(rs))) break ;
	    } /* end while */

	    rs1 = dirseen_curend(dsp,&cur) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (cursor) */

	return rs ;
}
/* end subroutines (sysvars_indopenalt) */


static int sysvars_indmk(SYSVARS *op,cchar *dname)
{
	const int	rsn = SR_NOTFOUND ;
	int		rs ;
	int		c = 0 ;

/* check the given directory for writability */

	if ((rs = checkdname(dname)) == rsn) {
	    rs = mkdirs(dname,0775) ;
	}

/* create the index-name */

	if (rs >= 0) {
	    const mode_t	operms = 0664 ;
	    char		indname[MAXPATHLEN + 1] ;
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


static int sysvars_indmkdata(SYSVARS *op,cchar *indname,mode_t operms)
{
	HDBSTR		vars, *vlp = &vars ;
	int		rs ;
	int		rs1 ;
	int		c = 0 ;

	if (op == NULL) return SR_FAULT ;

	if ((rs = hdbstr_start(vlp,DEFNVARS)) >= 0) {
	    int		i ;
	    int		f ;

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

	    if (rs >= 0) {
	        rs = sysvarprocset(vlp,indname,operms) ;
	    }

	    rs1 = hdbstr_finish(vlp) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (hdbstr) */

#if	CF_DEBUGS
	debugprintf("sysvars_indmkdata: ret rs=%d\n",rs) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (sysvars_indmkdata) */


static int sysvars_indclose(SYSVARS *op)
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
#if	CF_MKSYSVARS
static int sysvars_mksysvarsi(SYSVARS *op,SUBINFO *sip,cchar *dname)
{
	int		rs ;
	int		rs1 ;
	cchar		*prog = PROG_MKSYSVARSI ;
	char		dbname[MAXPATHLEN + 1] ;

#if	CF_DEBUGS
	debugprintf("sysvars_mksysvarsi: dname=%s\n",dname) ;
#endif

	if (dname == NULL) return SR_FAULT ;

	if (dname[0] == '\0') return SR_INVALID ;

	if ((rs = mkpath2(dbname,dname,op->dbname)) >= 0) {
	    if ((rs = subinfo_ids(sip)) >= 0) {
	        struct ustat	sb ;
	        int		rs_last = SR_NOTFOUND ;
	        int		i ;
	        int		pl = 0 ;
		cchar		*pr = op->pr ;
	        char		pbuf[MAXPATHLEN + 1] ;

	        for (i = 0 ; prbins[i] != NULL ; i += 1) {
	            if ((rs = mkpath3(pbuf,pr,prbins[i],prog)) >= 0) {
	                pl = rs ;
	                if ((rs = u_stat(pbuf,&sb)) >= 0) {
	                    if ((rs = sperm(&sip->id,&sb,X_OK)) >= 0) {
				rs = 0 ;
			    } else if (isNotPresent(rs)) {
	                        rs_last = rs ;
	                        pl = 0 ;
	                        rs = SR_OK ;
	                    }
	                } else if (isNotPresent(rs)) {
	                    rs_last = rs ;
	                    pl = 0 ;
	                    rs = SR_OK ;
	                }
	            } /* end if (mkpath) */
	            if (pl > 0) break ;
	            if (rs >= 0) break ;
	        } /* end for */
	        if ((rs >= 0) && (pl == 0)) rs = rs_last ;

#if	CF_DEBUGS
	        debugprintf("mksysvarsi: pr=%s\n",op->pr) ;
	        debugprintf("mksysvarsi: pbuf=%s\n",pbuf) ;
	        debugprintf("mksysvarsi: perm() rs=%d\n",rs) ;
#endif

	        if (rs >= 0) {
	            vecstr	envs ;
	            const int	vo = VECSTR_OCOMPACT ;
	            if ((rs = vecstr_start(&envs,20,vo)) >= 0) {
	                cchar	*cp ;

/* setup environment */

	                if (rs >= 0) {
	                    rs = vecstr_envadd(&envs,VARSVPR,op->pr,-1) ;
	                }
	                if (rs >= 0) {
	                    rs = vecstr_envadd(&envs,VARSVDBNAME,dbname,-1) ;
	                }
			if (rs >= 0) {
	                    for (i = 0 ; envdefs[i] != NULL ; i += 1) {
	                        if ((cp = getenv(envdefs[i])) != NULL) {
	                            rs = vecstr_envadd(&envs,envdefs[i],cp,-1) ;
	                        }
	                        if (rs < 0) break ;
	                    } /* end for */
			} /* end if (ok) */

/* go */

	                if (rs >= 0) {
	                    cchar	**ev ;
	                    if ((rs = vecstr_getvec(&envs,&ev)) >= 0) {
	                        SPAWNPROC	ps ;
	                        cchar		*av[10] ;
	                        i = 0 ;
	                        av[i++] = prog ;
	                        av[i++] = "-s" ;
	                        av[i++] = NULL ;
	                        memset(&ps,0,sizeof(SPAWNPROC)) ;
	                        ps.opts |= SPAWNPROC_OIGNINTR ;
	                        ps.opts |= SPAWNPROC_OSETPGRP ;
	                        for (i = 0 ; i < 3 ; i += 1) {
	                            if (i != 2) {
	                                ps.disp[i] = SPAWNPROC_DCLOSE ;
	                            } else {
	                                ps.disp[i] = SPAWNPROC_DINHERIT ;
	                            }
	                        } /* end for */
	                        if ((rs = spawnproc(&ps,pbuf,av,ev)) >= 0) {
	                            pid_t	cpid = rs ;
	                            int		cs = 0 ;
	                            if ((rs = u_waitpid(cpid,&cs,0)) >= 0) {
	                                int	cex = 0 ;
	                                if (WIFSIGNALED(cs)) {
	                                    rs = SR_UNATCH ;
	                                }
	                                if ((rs >= 0) && WIFEXITED(cs)) {
	                                    cex = WEXITSTATUS(cs) ;
	                                    if (cex != 0) rs = SR_LIBBAD ;
	                                } /* end if */
	                            } /* end if (process finished) */
	                        } /* end if (spawnproc) */
	                    } /* end if (vecstr_getvec) */
	                } /* end if (ok) */

	                rs1 = vecstr_finish(&envs) ;
			if (rs >= 0) rs = rs1 ;
	            } /* end if (vecstr) */
	        } /* end if (ok) */

	    } /* end if (subinfo_ids) */
	} /* end if (mkpath) */

#if	CF_DEBUGS
	debugprintf("mksysvarsi: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (sysvars_mksysvarsi) */
#endif /* CF_MKSYSVARS */


static int subinfo_start(SUBINFO *sip)
{
	int		rs = SR_OK ;

	memset(sip,0,sizeof(SUBINFO)) ;
	sip->daytime = time(NULL) ;

	return rs ;
}
/* end subroutine (subinfo_start) */


static int subinfo_ids(SUBINFO *sip)
{
	int		rs = SR_OK ;

	if (! sip->f.id) {
	    sip->f.id = TRUE ;
	    rs = ids_load(&sip->id) ;
	}

	return rs ;
}
/* end subroutine (subinfo_ids) */


static int subinfo_finish(SUBINFO *sip)
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


static int checkdname(cchar *dname)
{
	int		rs = SR_OK ;

	if (dname[0] != '/') {
	    struct ustat	sb ;
	    if ((rs = u_stat(dname,&sb)) >= 0) {
	        if (S_ISDIR(sb.st_mode)) {
	            rs = perm(dname,-1,-1,NULL,W_OK) ;
		} else {
		    rs = SR_NOTDIR ;
	        }
	    } /* end if (stat) */
	} else {
	    rs = SR_INVALID ;
	}

	return rs ;
}
/* end subroutine (checkdname) */


