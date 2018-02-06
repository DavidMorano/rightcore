/* pcsconfs */

/* interface to query the PCS configuration-variable database */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 2008-10-07, David A­D­ Morano
	This was created to allow for the main part of the PCSCONF facility to
	be a loadable module.

*/

/* Copyright © 2008 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This little object provides access to the PCSCONF database and index
	(if any).


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<limits.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<endianstr.h>
#include	<vecstr.h>
#include	<spawnproc.h>
#include	<ids.h>
#include	<fsdir.h>
#include	<varmk.h>
#include	<paramfile.h>
#include	<localmisc.h>

#include	"pcsconfs.h"
#include	"var.h"
#include	"varmk.h"


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

#ifndef	TMPDNAME
#define	TMPDNAME	"/tmp"
#endif

#undef	VARSVDBNAME
#define	VARSVDBNAME	"PCSCONFS_DBNAME"

#undef	VARSVPR
#define	VARSVPR		"PCSCONFS_PROGRAMROOT"

#ifndef	TMPDNAME
#define	TMPDNAME	"/tmp"
#endif

#ifndef	TMPVARDNAME
#define	TMPVARDNAME	"/var/tmp"
#endif

#define	CONFVARS	struct confvars

#define	PRCONF		"conf"

#define	PCSCONFNAME	"conf"

#ifndef	VCNAME
#define	VCNAME		"var"
#endif

#define	INDDNAME	"pcsconfs"

#define	INDNAME		"pcsconfs"
#define	INDSUF		"vi"

#ifndef	PARAMBUFLEN
#define	PARAMBUFLEN	256
#endif

#ifndef	KBUFLEN
#define	KBUFLEN		120
#endif

#define	TO_FILEMOD	(60 * 24 * 3600)
#define	TO_MKWAIT	(5 * 50)

#define	PROG_MKPCSCONFSI	"mkpcsconf"

#ifndef	DEFINITFNAME
#define	DEFINITFNAME	"/etc/default/init"
#endif

#ifndef	DEFLOGFNAME
#define	DEFLOGFNAME	"/etc/default/login"
#endif

#ifndef	DEFNVARS
#define	DEFNVARS	20
#endif

#define	CONFVARS	struct confvars
#define	CONFVARS_FL	struct confvars_flags


/* external subroutines */

extern int	sncpy1(char *,int,cchar *) ;
extern int	sncpy2(char *,int,cchar *,cchar *) ;
extern int	sncpy3(char *,int,cchar *,cchar *,cchar *) ;
extern int	sncpy4(char *,int, cchar *,cchar *,cchar *,cchar *) ;
extern int	sncpylc(char *,int,cchar *) ;
extern int	snwcpy(char *,int,cchar *,int) ;
extern int	snwcpylc(char *,int,cchar *,int) ;
extern int	mkpath1(char *,cchar *) ;
extern int	mkpath2(char *,cchar *,cchar *) ;
extern int	mkpath3(char *,cchar *,cchar *,cchar *) ;
extern int	mkpath4(char *,cchar *,cchar *,cchar *,cchar *) ;
extern int	mkpath1w(char *,cchar *,int) ;
extern int	mkfnamesuf1(char *,cchar *,cchar *) ;
extern int	mkfnamesuf2(char *,cchar *,cchar *,cchar *) ;
extern int	pathadd(char *,int,cchar *) ;
extern int	sfbasename(cchar *,int,cchar **) ;
extern int	nextfield(cchar *,int,cchar **) ;
extern int	nleadstr(cchar *,cchar *,int) ;
extern int	pathclean(char *,cchar *,int) ;
extern int	sperm(IDS *,struct ustat *,int) ;
extern int	perm(cchar *,uid_t,gid_t,gid_t *,int) ;
extern int	fperm(int,uid_t,gid_t,gid_t *,int) ;
extern int	permsched(cchar **,vecstr *,char *,int,cchar *,int) ;
extern int	hasuc(cchar *,int) ;
extern int	isalnumlatin(int) ;
extern int	strpcmp(cchar *,cchar *) ;
extern int	vecstr_adduniq(vecstr *,cchar *,int) ;
extern int	vecstr_envadd(vecstr *,cchar *,cchar *,int) ;
extern int	vecstr_envset(vecstr *,cchar *,cchar *,int) ;
extern int	prmktmpdir(cchar *,char *,cchar *,cchar *,mode_t) ;
extern int	mktmpuserdir(char *,cchar *,cchar *,mode_t) ;
extern int	hasdots(cchar *,int) ;
extern int	hasNotDots(cchar *,int) ;
extern int	isNotPresent(int) ;
extern int	isOneOf(const int *,int) ;

#if	CF_DEBUGS
extern int	debugprintf(cchar *,...) ;
extern int	strlinelen(cchar *,int,int) ;
#endif

extern char	*strwcpy(char *,cchar *,int) ;
extern char	*strwcpylc(char *,cchar *,int) ;
extern char	*strnchr(cchar *,int,int) ;
extern char	*strnpbrk(cchar *,int,cchar *) ;
extern char	*timestr_logz(time_t,char *) ;


/* external variables */

extern char	**environ ;


/* exported variables */

PCSCONFS_OBJ	pcsconfs = {
	"pcsconfs",
	sizeof(PCSCONFS),
	sizeof(PCSCONFS_CUR)
} ;


/* local structures */

struct confvars_flags {
	uint		conf:1 ;
	uint		id:1 ;
	uint		cfname:1 ;	/* memory-allocated */
	uint		rstchown:1 ;
} ;

struct confvars {
	IDS		id ;
	PARAMFILE	pf ;
	VARMK		v ;
	CONFVARS_FL	f ;
	PCSCONFS	*op ;
	cchar		**envv ;
	cchar		*prconf ;
	cchar		*pr ;
	cchar		*cfname ;
	uid_t		uid_pcs ;
	gid_t		gid_pcs ;
	time_t		daytime ;
	time_t		cmtime ;	/* conf-file mtime */
} ;


/* forward references */

static int	pcsconfs_valsbegin(PCSCONFS *,cchar *,cchar *) ;
static int	pcsconfs_valsend(PCSCONFS *) ;
static int	pcsconfs_dbcheck(PCSCONFS *) ;
static int	pcsconfs_dbclose(PCSCONFS *) ;
static int	pcsconfs_finout(PCSCONFS *) ;

static int	confvars_start(CONFVARS *,PCSCONFS *) ;
static int	confvars_finish(CONFVARS *) ;
static int	confvars_dbstart(CONFVARS *) ;
static int	confvars_loadsubs(CONFVARS *,VECSTR *) ;
static int	confvars_confglobal(CONFVARS *,char *) ;
static int	confvars_conflocal(CONFVARS *,char *) ;
static int	confvars_dbopen(CONFVARS *,cchar *) ;
static int	confvars_dbclose(CONFVARS *) ;
static int	confvars_dbmake(CONFVARS *,cchar *) ;
static int	confvars_proc(CONFVARS *) ;
static int	confvars_chown(CONFVARS *,char *,int) ;
static int	confvars_ids(CONFVARS *) ;


/* local variables */

#ifdef	COMMENT
static cchar	*envdefs[] = {
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
#endif /* COMMENT */

#ifdef	COMMENT
static cchar	*prbins[] = {
	"bin",
	"sbin",
	NULL
} ;
#endif /* COMMENT */

static cchar	*schedconf[] = {
	"%p/etc/%n.%f",
	"%p/etc/%f",
	"%p/%n.%f",
	NULL
} ;

static const int	stales[] = {
	SR_STALE,
	SR_NOENT,
	SR_ACCESS,
	0
} ;


/* exported subroutines */


int pcsconfs_start(PCSCONFS *op,cchar *pr,cchar **envv,cchar *cfname)
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;
	if (pr == NULL) return SR_FAULT ;

	if (pr[0] == '\0') return SR_INVALID ;

#if	CF_DEBUGS
	debugprintf("pcsconfs_start: pr=%s\n",pr) ;
	debugprintf("pcsconfs_start: cfname=%s\n",cfname) ;
#endif

	if ((cfname != NULL) && (cfname[0] == '\0')) return SR_INVALID ;

	memset(op,0,sizeof(PCSCONFS)) ;
	op->envv = (envv != NULL) ? envv : (cchar **) environ ;
	op->f.prdb = (cfname == NULL) ;

#if	CF_DEBUGS
	debugprintf("pcsconfs_start: f_prdb=%u\n",op->f.prdb) ;
#endif

	if ((rs = pcsconfs_valsbegin(op,pr,cfname)) >= 0) {
	    if ((rs = pcsconfs_dbcheck(op)) >= 0) {
		op->magic = PCSCONFS_MAGIC ;
	    }
	    if (rs < 0)
		pcsconfs_valsend(op) ;
	} /* end if */

#if	CF_DEBUGS
	debugprintf("pcsconfs_start: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (pcsconfs_start) */


int pcsconfs_finish(PCSCONFS *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != PCSCONFS_MAGIC) return SR_NOTOPEN ;

	rs1 = pcsconfs_finout(op) ;
	if (rs >= 0) rs = rs1 ;

	op->magic = 0 ;
	return rs ;
}
/* end subroutine (pcsconfs_finish) */


int pcsconfs_audit(PCSCONFS *op)
{
	int		rs = SR_OK ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != PCSCONFS_MAGIC) return SR_NOTOPEN ;

	if (op->f.db) {
	    rs = var_audit(&op->db) ;
	}

	return rs ;
}
/* end subroutine (pcsconfs_audit) */


int pcsconfs_curbegin(PCSCONFS *op,PCSCONFS_CUR *curp)
{
	int		rs = SR_OK ;

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;

	if (op->magic != PCSCONFS_MAGIC) return SR_NOTOPEN ;

	memset(curp,0,sizeof(PCSCONFS_CUR)) ;

	if (op->f.db) {
	    rs = var_curbegin(&op->db,&curp->vcur) ;
	    if (rs >= 0) op->ncursors += 1 ;
	}

	if (rs >= 0) curp->magic = PCSCONFS_CURMAGIC ;

	return rs ;
}
/* end subroutine (pcsconfs_curbegin) */


int pcsconfs_curend(PCSCONFS *op,PCSCONFS_CUR *curp)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;

	if (op->magic != PCSCONFS_MAGIC) return SR_NOTOPEN ;
	if (curp->magic != PCSCONFS_CURMAGIC) return SR_NOTOPEN ;

	if (op->f.db) {
	    rs1 = var_curend(&op->db,&curp->vcur) ;
	    if (rs >= 0) rs = rs1 ;
	    if (op->ncursors > 0) op->ncursors -= 1 ;
	}

	curp->magic = 0 ;
	return rs ;
}
/* end subroutine (pcsconfs_curend) */


int pcsconfs_fetch(PCSCONFS *op,cchar *kp,int kl,PCSCONFS_CUR *curp,
		char vbuf[],int vlen)
{
	int		rs = SR_NOTFOUND ;

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;
	if (kp == NULL) return SR_FAULT ;

	if (op->magic != PCSCONFS_MAGIC) return SR_NOTOPEN ;
	if (curp->magic != PCSCONFS_CURMAGIC) return SR_NOTOPEN ;

	if (op->f.db) {
	    rs = var_fetch(&op->db,kp,kl,&curp->vcur,vbuf,vlen) ;
	}

	if ((rs < 0) && (vbuf != NULL)) {
	    vbuf[0] = '\0' ;
	}

	return rs ;
}
/* end subroutine (pcsconfs_fetch) */


int pcsconfs_enum(op,curp,kbuf,klen,vbuf,vlen)
PCSCONFS	*op ;
PCSCONFS_CUR	*curp ;
char		kbuf[] ;
int		klen ;
char		vbuf[] ;
int		vlen ;
{
	int		rs = SR_NOTFOUND ;

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;
	if (kbuf == NULL) return SR_FAULT ;

	if (op->magic != PCSCONFS_MAGIC) return SR_NOTOPEN ;
	if (curp->magic != PCSCONFS_CURMAGIC) return SR_NOTOPEN ;

	if (op->f.db) {
	    rs = var_enum(&op->db,&curp->vcur,kbuf,klen,vbuf,vlen) ;
	}

	if ((rs < 0) && (vbuf != NULL)) {
	    vbuf[0] = '\0' ;
	}

	return rs ;
}
/* end subroutine (pcsconfs_enum) */


int pcsconfs_count(PCSCONFS *op)
{
	int		rs = SR_OK ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != PCSCONFS_MAGIC) return SR_NOTOPEN ;

	if (op->f.db) {
	    rs = var_count(&op->db) ;
	}

	return rs ;
}
/* end subroutine (pcsconfs_count) */


/* private subroutines */


static int pcsconfs_valsbegin(PCSCONFS *op,cchar pr[],cchar cfname[])
{
	int		rs ;
	int		size = 0 ;
	char		*bp ;

	size += (strlen(pr)+1) ;
	if (cfname != NULL) size += (strlen(cfname)+1) ;
	if ((rs = uc_malloc(size,&bp)) >= 0) {
	    op->a = bp ;
	    op->pr = bp ;
	    bp = (strwcpy(bp,pr,-1)+1) ;
	    if (cfname != NULL) {
		op->cfname = bp ;
	        bp = (strwcpy(bp,cfname,-1)+1) ;
	    }
	} /* end if (memory-allocation) */

	return rs ;
}
/* end subroutine (pcsconfs_valsbegin) */


static int pcsconfs_valsend(PCSCONFS *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op->a != NULL) {
	    rs1 = uc_free(op->a) ;
	    if (rs >= 0) rs = rs1 ;
	    op->a = NULL ;
	}

	return rs ;
}
/* end subroutine (pcsconfs_valsend) */


static int pcsconfs_dbcheck(PCSCONFS *op)
{
	CONFVARS	si, *sip = &si ;
	int		rs ;
	int		rs1 ;
	int		f_conf = FALSE ;

#if	CF_DEBUGS
	debugprintf("pcsconfs_dbcheck: ent\n") ;
#endif

	if ((rs = confvars_start(sip,op)) >= 0) {

	    if (sip->f.conf) {
	        f_conf = TRUE ;
	        rs = confvars_dbstart(sip) ;
#if	CF_DEBUGS
	debugprintf("pcsconfs_dbcheck: confvars_dbstart() rs=%d\n",rs) ;
#endif
	    }

	    rs1 = confvars_finish(sip) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (confvars) */

#if	CF_DEBUGS
	debugprintf("pcsconfs_dbcheck: ret rs=%d f_conf=%u\n",rs,f_conf) ;
#endif

	return (rs >= 0) ? f_conf : rs ;
}
/* end subroutine (pcsconfs_dbcheck) */


static int pcsconfs_dbclose(PCSCONFS *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op->f.db) {
	    op->f.db = FALSE ;
	    rs1 = var_close(&op->db) ;
	    if (rs >= 0) rs = rs1 ;
	}

	return rs ;
}
/* end subroutine (pcsconfs_dbclose) */


static int pcsconfs_finout(PCSCONFS *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	rs1 = pcsconfs_dbclose(op) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = pcsconfs_valsend(op) ;
	if (rs >= 0) rs = rs1 ;

	return rs ;
}
/* end subroutine (pcsconfs_finout) */


static int confvars_start(CONFVARS *sip,PCSCONFS *op)
{
	int		rs = SR_OK ;
	int		rs1 = SR_OK ;
	cchar		*cfname ;
	char		tmpfname[MAXPATHLEN+1] ;

	memset(sip,0,sizeof(CONFVARS)) ;
	sip->op = op ;
	sip->envv = op->envv ;
	sip->prconf = PRCONF ;
	sip->pr = op->pr ;
	sip->cfname = op->cfname ;
	sip->uid_pcs = -1 ;
	sip->gid_pcs = -1 ;

/* find the PCS-configuration file if necessary */

	cfname = op->cfname ;

	if (op->f.prdb) {
	    VECSTR	subs ;
	    if ((rs = vecstr_start(&subs,4,0)) >= 0) {

	        if ((rs = confvars_loadsubs(sip,&subs)) >= 0) {
	            const mode_t	m = R_OK ;
	            const int	plen = MAXPATHLEN ;
		    cchar	*cn = sip->prconf ;
	            char	*pbuf = tmpfname ;
	            cfname = tmpfname ;
	            rs1 = permsched(schedconf,&subs,pbuf,plen,cn,m) ;
		    if (rs1 >= 0) {
			cchar	*cp ;
			if ((rs = uc_mallocstrw(pbuf,rs1,&cp)) >= 0) {
			    sip->f.cfname = TRUE ;
			    sip->cfname = cp ;
			}
		    }
	        } /* end if (subs loaded) */

	        vecstr_finish(&subs) ;
	    } /* end if (subs) */
	} /* end if */

	if (rs >= 0) {
	    if (rs1 >= 0) {
	        struct ustat	sb ;
	        rs1 = u_stat(cfname,&sb) ;
	        if ((rs1 >= 0) && S_ISREG(sb.st_mode)) {
	            sip->f.conf = TRUE ;
	            sip->cmtime = sb.st_mtime ;
	        }
	    }
	} /* end if */

	return rs ;
}
/* end subroutine (confvars_start) */


static int confvars_finish(CONFVARS *sip)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (sip->f.cfname && (sip->cfname != NULL)) {
	    sip->f.cfname = FALSE ;
	    rs1 = uc_free(sip->cfname) ;
	    if (rs >= 0) rs = rs1 ;
	    sip->cfname = NULL ;
	}

	if (sip->f.id) {
	    sip->f.id = FALSE ;
	    rs1 = ids_release(&sip->id) ;
	    if (rs >= 0) rs = rs1 ;
	}

	return rs ;
}
/* end subroutine (confvars_finish) */


static int confvars_loadsubs(CONFVARS *sip,VECSTR *slp)
{
	int		rs = SR_OK ;
	int		bl ;
	cchar		*bp ;

	bl = sfbasename(sip->pr,-1,&bp) ;
	if (bl <= 0) rs = SR_INVALID ;

	if (rs >= 0)
	    rs = vecstr_envadd(slp,"p",sip->pr,-1) ;

	if (rs >= 0)
	    rs = vecstr_envadd(slp,"n",bp,bl) ;

	return rs ;
}
/* end subroutine (confvars_loadsubs) */


static int confvars_dbstart(CONFVARS *sip)
{
	PCSCONFS	*op = sip->op ;
	int		rs = SR_OK ;
	int		dl = -1 ;
	char		dbname[MAXPATHLEN+1] ;

	if (op->f.prdb) {
	    rs = confvars_confglobal(sip,dbname) ;
	    dl = rs ;
	} else {
	    rs = confvars_conflocal(sip,dbname) ;
	    dl = rs ;
	}
#if	CF_DEBUGS
	debugprintf("confvars_dbstart: mid rs=%d\n",rs) ;
	debugprintf("confvars_dbstart: dbname=%s\n",dbname) ;
#endif
	if (rs >= 0)
	    rs = pathadd(dbname,dl,sip->prconf) ;

	if (rs >= 0) {
	    rs = confvars_dbopen(sip,dbname) ;
#if	CF_DEBUGS
	debugprintf("confvars_dbstart: _dbopen() rs=%d\n",rs) ;
#endif
	    if (isOneOf(stales,rs)) {
	        if (rs == SR_ACCESS) varunlink(dbname,-1) ;
#if	CF_DEBUGS
	debugprintf("confvars_dbstart: _dbmake()\n") ;
#endif
	        rs = confvars_dbmake(sip,dbname) ;
#if	CF_DEBUGS
	debugprintf("confvars_dbstart: _dbmake() rs=%d\n",rs) ;
#endif
		if (isNotPresent(rs) && op->f.prdb) {
		    op->f.prdb = FALSE ;
	    	    if ((rs = confvars_conflocal(sip,dbname)) >= 0) {
			dl = rs ;
	    		if ((rs = pathadd(dbname,dl,sip->prconf)) >= 0)
	        	    rs = confvars_dbmake(sip,dbname) ;
		    }
		}
	        if (rs >= 0) {
	            rs = confvars_dbopen(sip,dbname) ;
#if	CF_DEBUGS
	debugprintf("confvars_dbstart: _dbopen() rs=%d\n",rs) ;
#endif
		    if ((rs >= 0) && op->f.prdb) {
		        dbname[dl] = '\0' ;
		        rs = confvars_chown(sip,dbname,dl) ;
		    }
		    if (rs < 0)
			confvars_dbclose(sip) ;
		}
	    } /* end if */
	} /* end if */

#if	CF_DEBUGS
	debugprintf("confvars_dbstart: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (confvars_dbstart) */


static int confvars_confglobal(CONFVARS *sip,char *dname)
{
	const mode_t	dm = 0777 ;
	int		rs ;
	cchar		*tmpdname = getenv(VARTMPDNAME) ;
	cchar		*cdname = "pcsconf" ;

	if (sip == NULL) return SR_FAULT ;

	if (tmpdname == NULL) tmpdname = TMPDNAME ;

#if	CF_DEBUGS
	debugprintf("confvars_confglobal/prmktmpdir: pr=%s\n",sip->pr) ;
	debugprintf("confvars_confglobal/prmktmpdir: prconf=%s\n",sip->prconf) ;
#endif

	rs = prmktmpdir(sip->pr,dname,tmpdname,cdname,dm) ;

	return rs ;
}
/* end subroutine (confvars_confglobal) */


static int confvars_conflocal(CONFVARS *sip,char *dname)
{
	const mode_t	dm = 0775 ;
	int		rs ;
	cchar		*cdname = "pcsconf" ;

	if (sip == NULL) return SR_FAULT ;

	rs = mktmpuserdir(dname,"-",cdname,dm) ;

	return rs ;
}
/* end subroutine (confvars_conflocal) */


static int confvars_dbopen(CONFVARS *sip,cchar dbname[])
{
	PCSCONFS	*op = sip->op ;
	VAR		*vdp ;
	int		rs ;

#if	CF_DEBUGS
	debugprintf("confvars_dbopen: dbname=%s\n",dbname) ;
#endif

	vdp = &op->db ;
	if ((rs = var_open(vdp,dbname)) >= 0) {
	    VAR_INFO	vi ;
	    op->f.db = TRUE ;
	    if ((rs = var_info(vdp,&vi)) >= 0) {
#if	CF_DEBUGS
	{
		char	timebuf[TIMEBUFLEN+1] ;
		time_t	t ;
		t = (time_t) sip->cmtime ;
		timestr_logz(t,timebuf) ;
		debugprintf("confvars_dbopen: ct=%s\n",timebuf) ;
		t = (time_t) vi.wtime ;
		timestr_logz(t,timebuf) ;
		debugprintf("confvars_dbopen: wt=%s\n",timebuf) ;
	}
#endif

	        if (sip->cmtime > vi.wtime) rs = SR_STALE ;
	    }
	    if (rs < 0) {
	        op->f.db = FALSE ;
	        var_close(vdp) ;
	    }
	} /* end if (attempted open) */

#if	CF_DEBUGS
	debugprintf("confvars_dbopen: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (confvars_dbopen) */


static int confvars_dbclose(CONFVARS *sip)
{
	PCSCONFS	*op = sip->op ;
	VAR		*vdp ;
	int		rs = SR_OK ;
	int		rs1 ;

	vdp = &op->db ;
	if (op->f.db) {
	    op->f.db = FALSE ;
	    rs1 = var_close(vdp) ;
	    if (rs >= 0) rs = rs1 ;
	}

	return rs ;
}
/* end subroutine (confvars_dbclose) */


static int confvars_dbmake(CONFVARS *sip,cchar dbname[])
{
	VARMK		*vmp = &sip->v ;
	const mode_t	vm = 0664 ;
	const int	of = O_CREAT ;
	const int	n = 40 ;
	int		rs ;
	int		rs1 ;

#if	CF_DEBUGS
	debugprintf("confvars_dbmake: dbname=%s\n",dbname) ;
#endif
	if ((rs = varmk_open(&sip->v,dbname,of,vm,n)) >= 0) {

	    rs = confvars_proc(sip) ;

#if	CF_DEBUGS
	debugprintf("confvars_dbmake: _proc() rs=%d\n",rs) ;
#endif
	    rs1 = varmk_close(vmp) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (varmk) */

#if	CF_DEBUGS
	debugprintf("confvars_dbmake: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (confvars_dbmake) */


static int confvars_proc(CONFVARS *sip)
{
	struct ustat	sb ;
	int		rs ;
	int		rs1 ;

#if	CF_DEBUGS
	debugprintf("confvars_proc: cfname=%s\n",sip->cfname) ;
#endif

	if ((rs = u_stat(sip->cfname,&sb)) >= 0) {
	    PARAMFILE		*pfp = &sip->pf ;
	    PARAMFILE_CUR	cur ;
	    PARAMFILE_ENT	pe ;
	    if ((rs = paramfile_open(pfp,sip->envv,sip->cfname)) >= 0) {

	        if ((rs = paramfile_curbegin(pfp,&cur)) >= 0) {
	            const int	plen = PARAMBUFLEN ;
	            int		kl ;
	            char	pbuf[PARAMBUFLEN+1] ;

	            while (rs >= 0) {
	                kl = paramfile_enum(pfp,&cur,&pe,pbuf,plen) ;
	                if (kl == SR_NOTFOUND) break ;
	                rs = kl ;
	                if (rs < 0) break ;

#if	CF_DEBUGS
			debugprintf("confvars_proc: k=%s\n",pe.key) ;
			debugprintf("confvars_proc: v=>%t<\n",
			    pe.value,pe.vlen) ;
#endif
	                rs = varmk_addvar(&sip->v,pe.key,pe.value,pe.vlen) ;

	            } /* end while (reading parameters) */

	            paramfile_curend(pfp,&cur) ;
	        } /* end if (cursor) */

	        rs1 = paramfile_close(pfp) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (paramfile) */
	} /* end if (stat-file) */

#if	CF_DEBUGS
	debugprintf("confvars_proc: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (confvars_proc) */


static int confvars_chown(CONFVARS *sip,char *dname,int dl)
{
	int		rs = SR_OK ;

#if	CF_DEBUGS
	debugprintf("confvars_chown: dl=%u dname=%t\n",dl,dname,dl) ;
#endif

#ifdef	_PC_CHOWN_RESTRICTED
	if ((rs = u_pathconf(dname,_PC_CHOWN_RESTRICTED,NULL)) == 0) {
	    if ((rs = confvars_ids(sip)) >= 0) {
		IDS		*idp = &sip->id ;
	        FSDIR		d ;
	        FSDIR_ENT	e ;
		uid_t		u = sip->uid_pcs ;
		gid_t		g = sip->gid_pcs ;
	        if ((rs = fsdir_open(&d,dname)) >= 0) {
		    struct ustat	sb ;
		    int			nl ;
		    cchar		*np ;
		    while ((rs = fsdir_read(&d,&e)) > 0) {
			np = e.name ;
			nl = rs ;
			if (hasNotDots(np,nl)) {
			    if ((rs = pathadd(dname,dl,np)) >= 0) {
		                if (u_stat(dname,&sb) >= 0) {
				    if (sb.st_uid == idp->euid) {
					int	f = FALSE ;
    			                f = f || (sb.st_uid != u) ;
			                f = f || (sb.st_gid != g) ;
			                if (f) u_chown(dname,u,g) ;
				    }
		                } /* end if (stat) */
			    } /* end if (pathadd) */
			} /* end if (hasNotDots) */
		        if (rs < 0) break ;
		    } /* end while (reading entries) */
		    dname[dl] = '\0' ;
		    fsdir_close(&d) ;
	        } /* end if (fsdir) */
	    } /* end if (confvars_ids) */
	} else if (rs == SR_NOSYS) {
	    rs = SR_OK ;
	}
#endif /* _PC_CHOWN_RESTRICTED */

#if	CF_DEBUGS
	debugprintf("confvars_chown: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (confvars_chown) */


static int confvars_ids(CONFVARS *sip)
{
	int		rs = SR_OK ;

	if (! sip->f.id) {
	    sip->f.id = TRUE ;
	    if ((rs = ids_load(&sip->id)) >= 0) {
	        struct ustat	sb ;
	        if ((rs = u_stat(sip->pr,&sb)) >= 0) {
		    sip->uid_pcs = sb.st_uid ;
		    sip->gid_pcs = sb.st_gid ;
	        }
	        if (rs < 0) {
	            sip->f.id = FALSE ;
	            ids_release(&sip->id) ;
		}
	    } /* end if (loaded IDs) */
	} /* end if (needed IDs) */

	return rs ;
}
/* end subroutine (confvars_ids) */


