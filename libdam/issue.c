/* issue */

/* object to help and manage "issue" messages */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */
#define	CF_DEBUGN	0		/* extra-special debugging */
#define	CF_WRITETO	1		/* time out writes */
#define	CF_ISSUEAUDIT	0		/* call 'issue_audit()' */
#define	CF_MAPPERAUDIT	0		/* call 'mapper_audit()' */
#define	CF_UGETPW	1		/* use |ugetpw(3uc)| */


/* revision history:

	= 2003-10-01, David A­D­ Morano
        This is a hack from numerous previous hacks (not enumerated here). This
        is a new version of this hack that is entirely different (much simpler).

*/

/* Copyright © 2003 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This object module writes the contents of various ISSUEs (as specified
        by the caller) to an open file descriptor (also specified by the
        caller).

	Implementation notes:

        When processing, we time-out writes to the caller-supplied
        file-descriptor because we don't know if it is a non-regular file that
        might be flow-controlled. We don't wait forever for those sorts of
        outputs. So let's say that the output is a terminal that is currently
        flow-controlled. We will time-out on our writes and the user will not
        get this whole ISSUE text!


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<limits.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<getbufsize.h>
#include	<estrings.h>
#include	<ids.h>
#include	<vecstr.h>
#include	<vechand.h>
#include	<getax.h>
#include	<ugetpw.h>
#include	<getxusername.h>
#include	<ptm.h>
#include	<lockrw.h>
#include	<paramfile.h>
#include	<strpack.h>
#include	<bfile.h>
#include	<fsdir.h>
#include	<ascii.h>
#include	<localmisc.h>

#include	"issue.h"


/* local defines */

#define	ISSUE_MAPDIR		struct issue_mapdir
#define	ISSUE_DEFGROUP		"default"
#define	ISSUE_ALLGROUP		"all"
#define	ISSUE_NAME		"issue"
#define	ISSUE_DIRSFNAME		"dirs"
#define	ISSUE_MAPPERMAGIC	0x21367425

#if	CF_UGETPW
#define	GETPW_NAME	ugetpw_name
#else
#define	GETPW_NAME	getpw_name
#endif /* CF_UGETPW */

#define	NDEBFNAME	"issue.deb"

#ifndef	MAXNAMELEN
#define	MAXNAMELEN	256
#endif

#ifndef	MSGBUFLEN
#define	MSGBUFLEN	2048
#endif

#ifndef	LINEBUFLEN
#ifdef	LINE_MAX
#define	LINEBUFLEN	MAX(LINE_MAX,2048)
#else
#define	LINEBUFLEN	2048
#endif
#endif

#undef	ENVBUFLEN
#define	ENVBUFLEN	100

#ifndef	DIGBUFLEN
#define	DIGBUFLEN	40		/* can hold int128_t in decimal */
#endif

#ifndef	POLLMULT
#define	POLLMULT	1000
#endif

#define	PBUFLEN		MAXPATHLEN

#undef	TO_POLL
#define	TO_POLL		5

#define	TO_LOCK		30
#define	TO_OPEN		10
#define	TO_READ		20
#define	TO_WRITE	50
#define	TO_CHECK	5		/* object checking */
#define	TO_MAPCHECK	10		/* mapper checking */
#define	TO_FILEAGE	5		/* directory map-file age */

#undef	NLPS
#define	NLPS		10		/* number? polls per second */


/* external subroutines */

extern int	snsds(char *,int,cchar *,cchar *) ;
extern int	sncpy1(char *,int,cchar *) ;
extern int	sncpy2(char *,int,cchar *,cchar *) ;
extern int	sncpy3(char *,int,cchar *,cchar *,cchar *) ;
extern int	sncpy4(char *,int,cchar *,cchar *,cchar *,cchar *) ;
extern int	sncpylc(char *,int,cchar *) ;
extern int	sncpyuc(char *,int,cchar *) ;
extern int	mkpath1(char *,cchar *) ;
extern int	mkpath2(char *,cchar *,cchar *) ;
extern int	mkpath3(char *,cchar *,cchar *,cchar *) ;
extern int	mknpath1(char *,int,cchar *) ;
extern int	mknpath2(char *,int,cchar *,cchar *) ;
extern int	mknpath3(char *,int,cchar *,cchar *,cchar *) ;
extern int	sichr(cchar *,int,int) ;
extern int	matstr(cchar **,cchar *,int) ;
extern int	nleadstr(cchar *,cchar *,int) ;
extern int	sperm(IDS *,struct ustat *,int) ;
extern int	permsched(cchar **,vecstr *,char *,int,cchar *,int) ;
extern int	getnodedomain(char *,char *) ;
extern int	ctdecui(char *,int,uint) ;
extern int	vecstr_envset(vecstr *,cchar *,cchar *,int) ;
extern int	vecstr_envadd(vecstr *,cchar *,cchar *,int) ;
extern int	msleep(int) ;
extern int	haslc(cchar *,int) ;
extern int	hasuc(cchar *,int) ;
extern int	hasalldig(cchar *,int) ;
extern int	isNotPresent(int) ;

#if	CF_DEBUGS || CF_DEBUGN
extern int	nprintf(cchar *,...) ;
extern int	debugprintf(cchar *,...) ;
extern int	strlinelen(cchar *,int,int) ;
#endif

extern char	*strwcpy(char *,cchar *,int) ;
extern char	*strnchr(cchar *,int,int) ;
extern char	*strdcpy1(char *,int,cchar *) ;
extern char	*strdcpy2(char *,int,cchar *,cchar *) ;
extern char	*strdcpy3(char *,int,cchar *,cchar *,cchar *) ;
extern char	*strdcpy4(char *,int,cchar *,cchar *,cchar *,cchar *) ;
extern char	*strdcpy1w(char *,int,cchar *,int) ;
extern char	*strnpbrk(cchar *,int,cchar *) ;


/* external variables */

extern cchar	**environ ;


/* local structures */

struct issue_mapdir {
	LOCKRW		rwm ;
	cchar		*admin ;
	cchar		*dirname ;	/* raw */
	cchar		*dname ;	/* expanded */
} ;


/* forward references */

static int	issue_mapfind(ISSUE *,time_t) ;
static int	issue_maplose(ISSUE *) ;
static int	issue_mapfname(ISSUE *,char *) ;
static int	issue_schedload(ISSUE *,vecstr *) ;
static int	issue_checker(ISSUE *,time_t) ;
static int	issue_envbegin(ISSUE *) ;
static int	issue_envend(ISSUE *) ;
static int	issue_envadds(ISSUE *,STRPACK *,cchar **,cchar *) ;
static int	issue_envstore(ISSUE *,STRPACK *,cchar **,int,cchar *,int) ;
static int 	issue_processor(ISSUE *,cchar **,cchar **,cchar *,int) ;
#if	CF_ISSUEAUDIT
static int	issue_audit(ISSUE *) ;
#endif

static int	mapper_start(ISSUE_MAPPER *,time_t,cchar *) ;
static int	mapper_finish(ISSUE_MAPPER *) ;
static int	mapper_check(ISSUE_MAPPER *,time_t) ;
static int	mapper_process(ISSUE_MAPPER *,cchar **,cchar **,cchar *,int) ;
static int	mapper_processor(ISSUE_MAPPER *,cchar **,cchar **,cchar *,int) ;
static int	mapper_mapload(ISSUE_MAPPER *) ;
static int	mapper_mapadd(ISSUE_MAPPER *,cchar *,int,cchar *,int) ;
static int	mapper_mapfins(ISSUE_MAPPER *) ;
#if	CF_MAPPERAUDIT
static int	mapper_audit(ISSUE_MAPPER *) ;
#endif

static int	mapdir_start(ISSUE_MAPDIR *,cchar *,int,cchar *,int) ;
static int	mapdir_finish(ISSUE_MAPDIR *) ;
static int	mapdir_process(ISSUE_MAPDIR *,cchar **,cchar **,cchar *,int) ;
static int	mapdir_expand(ISSUE_MAPDIR *) ;
static int	mapdir_expander(ISSUE_MAPDIR *) ;
static int	mapdir_processor(ISSUE_MAPDIR *,cchar **,cchar *,int) ;
static int	mapdir_procout(ISSUE_MAPDIR *,cchar **,cchar *,cchar *,int) ;
static int	mapdir_procouter(ISSUE_MAPDIR *,cchar **,cchar *,int) ;

static int	mapdir_processorthem(ISSUE_MAPDIR *,cchar **,
			cchar *,VECSTR *,cchar **,int) ;
static int	mapdir_processorone(ISSUE_MAPDIR *,cchar **,
			cchar *,VECSTR *,cchar *,int) ;

static int	writeto(int,cchar *,int,int) ;
static int	loadstrs(cchar **,cchar *,cchar *,cchar *,cchar *) ;
static int	isBaseMatch(cchar *,cchar *,cchar *) ;


/* local variables */

static cchar	*schedmaps[] = {
	"%p/%e/%n/%n.%f",
	"%p/%e/%n/%f",
	"%p/%e/%n.%f",
	"%p/%n.%f",
	"%n.%f",
	NULL
} ;

static cchar	*envbad[] = {
	"TMOUT",
	"A__z",
	NULL

} ;

static cchar	*envstrs[] = {
	"KEYNAME",
	"ADMIN",
	"ADMINDIR",
	NULL
} ;

enum envstrs {
	envstr_keyname,
	envstr_admin,
	envstr_admindir,
	envstr_overlast
} ;

static cchar	*envpre = "ISSUE_" ;	/* environment prefix */


/* exported subroutines */


int issue_open(ISSUE *op,cchar pr[])
{
	const time_t	dt = time(NULL) ;
	int		rs ;
	cchar		*cp ;

	if (op == NULL) return SR_FAULT ;
	if (pr == NULL) return SR_FAULT ;

	if (pr[0] == '\0') return SR_INVALID ;

	memset(op,0,sizeof(ISSUE)) ;
	op->fe = ISSUE_DIRSFNAME ;

	if ((rs = uc_mallocstrw(pr,-1,&cp)) >= 0) {
	    op->pr = cp ;
	    if ((rs = ptm_create(&op->m,NULL)) >= 0) {
	        if ((rs = issue_mapfind(op,dt)) >= 0) {
	            if ((rs = issue_envbegin(op)) >= 0) {
	                op->ti_lastcheck = dt ;
	                op->magic = ISSUE_MAGIC ;
	            }
	            if (rs < 0)
	                issue_maplose(op) ;
	        } /* end if (mapfind) */
	        if (rs < 0)
	            ptm_destroy(&op->m) ;
	    } /* end if (ptm_create) */
	    if (rs < 0) {
	        uc_free(op->pr) ;
	        op->pr = NULL ;
	    }
	} /* end if (memory-allocation) */

	return rs ;
}
/* end subroutine (issue_open) */


int issue_close(ISSUE *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != ISSUE_MAGIC) return SR_NOTOPEN ;

	rs1 = issue_envend(op) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = issue_maplose(op) ;
	if (rs >= 0) rs = rs1 ;

#if	CF_DEBUGS
	debugprintf("issue_close: _maplose() rs=%d\n",rs) ;
#endif

	rs1 = ptm_destroy(&op->m) ;
	if (rs >= 0) rs = rs1 ;

	if (op->pr != NULL) {
	    rs1 = uc_free(op->pr) ;
	    if (rs >= 0) rs = rs1 ;
	    op->pr = NULL ;
	}

#if	CF_DEBUGS
	debugprintf("issue_close: ret rs=%d\n",rs) ;
#endif

	op->magic = 0 ;
	return rs ;
}
/* end subroutine (issue_close) */


int issue_check(ISSUE *op,time_t dt)
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != ISSUE_MAGIC) return SR_NOTOPEN ;

	rs = issue_checker(op,dt) ;

	return rs ;
}
/* end subroutine (issue_check) */


int issue_process(ISSUE *op,cchar *groupname,cchar **adms,int fd)
{
	int		rs ;
	int		n ;
	int		size ;
	int		wlen = 0 ;
	void		*p ;

	if (op == NULL) return SR_FAULT ;
	if (groupname == NULL) return SR_FAULT ;

	if (op->magic != ISSUE_MAGIC) return SR_NOTOPEN ;

	if (fd < 0) return SR_BADF ;

	if (groupname[0] == '\0') return SR_INVALID ;

	if (groupname[0] == '-') groupname = ISSUE_DEFKEYNAME ;

#if	CF_ISSUEAUDIT
	rs = issue_audit(op) ;
	if (rs < 0) goto ret0 ;
#endif

#if	CF_DEBUGS
	{
	    debugprintf("issue_process: tar groupname=%s\n",groupname) ;
	    if (adms != NULL) {
	        int	i ;
	        for (i = 0 ; adms[i] != NULL ; i += 1) {
	            debugprintf("issue_process: a[%u]=%s\n",i,adms[i]) ;
		}
	    }
	}
#endif /* CF_DEBUGS */

/* go */

	n = nelem(envstrs) ;
	size = (op->nenv + n + 1) * sizeof(cchar *) ;
	if ((rs = uc_malloc(size,&p)) >= 0) {
	    STRPACK	packer ;
	    cchar	**ev = (cchar **) p ;

	    if ((rs = strpack_start(&packer,128)) >= 0) {

	        if ((rs = issue_envadds(op,&packer,ev,groupname)) >= 0) {
	            rs = issue_processor(op,ev,adms,groupname,fd) ;
	            wlen = rs ;
	        }

	        strpack_finish(&packer) ;
	    } /* end if (packer) */

	    uc_free(p) ;
	} /* end if (memory allocation) */

/* done */
ret0:

#if	CF_DEBUGS
	debugprintf("issue_process: ret rs=%d wlen=%u\n",rs,wlen) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (issue_process) */


/* private subroutines */


static int issue_mapfind(ISSUE *op,time_t dt)
{
	int		rs ;
	char		mapfname[MAXPATHLEN + 1] ;

#if	CF_DEBUGS
	debugprintf("issue_mapfind: ent\n") ;
#endif

	mapfname[0] = '\0' ;
	if ((rs = issue_mapfname(op,mapfname)) >= 0) {
	    if (mapfname[0] != '\0') {
	        if ((rs = mapper_start(&op->mapper,dt,mapfname)) >= 0) {
	            op->nmaps += 1 ;
		}
	    }
	} /* end if (map-fname) */

#if	CF_DEBUGS
	debugprintf("issue_mapfind: mid rs=%d\n",rs) ;
#endif

#if	CF_ISSUEAUDIT
	if (rs >= 0) {
	    rs = issue_audit(op) ;
	}
#endif

#if	CF_DEBUGS
	debugprintf("issue_mapfind: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (issue_mapfind) */


static int issue_maplose(ISSUE *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op->nmaps > 0) {
	    rs1 = mapper_finish(&op->mapper) ;
	    if (rs >= 0) rs = rs1 ;
	    op->nmaps = 0 ;
	}

	return rs ;
}
/* end subroutine (issue_maplose) */


static int issue_mapfname(ISSUE *op,char fbuf[])
{
	vecstr		scheds ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		c = 0 ;

	fbuf[0] = '\0' ;
	if ((rs = vecstr_start(&scheds,6,0)) >= 0) {
	    if ((rs = issue_schedload(op,&scheds)) >= 0) {
	        const int	flen = MAXPATHLEN ;

	        rs1 = permsched(schedmaps,&scheds,fbuf,flen,op->fe,R_OK) ;

#if	CF_DEBUGS
	        debugprintf("issue_mapfname: permsched() rs=%d\n",rs1) ;
#endif

	        if ((rs1 == SR_NOENT) || (rs1 == SR_ACCESS)) {
	            if (rs1 == SR_NOENT) {
	                fbuf[0] = '\0' ;
	            } else
	                rs = rs1 ;
	        } else if (rs1 == SR_OK) {
	            c = 1 ;
	        } else
	            rs = rs1 ;

	    } /* end if (issue-schedload) */
	    rs1 = vecstr_finish(&scheds) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (scheds) */

#if	CF_DEBUGS
	debugprintf("issue_mapfname: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (issue_mapfname) */


static int issue_schedload(ISSUE *op,vecstr *slp)
{
	int		rs = SR_OK ;
	int		i ;
	cchar		*keys = "pen" ;
	cchar		*name = ISSUE_NAME ;
	for (i = 0 ; keys[i] != '\0' ; i += 1) {
	    const int	kch = MKCHAR(keys[i]) ;
	    int		vl = -1 ;
	    cchar	*vp = NULL ;
	    switch (kch) {
	    case 'p':
	        vp = op->pr ;
	        break ;
	    case 'e':
	        vp = "etc" ;
	        break ;
	    case 'n':
	        vp = name ;
	        break ;
	    } /* end switch */
	    if ((rs >= 0) && (vp != NULL)) {
	        char	kbuf[2] = { 
	            0, 0 			} ;
	        kbuf[0] = kch ;
	        rs = vecstr_envset(slp,kbuf,vp,vl) ;
	    }
	    if (rs < 0) break ;
	} /* end for */
	return rs ;
}
/* end subroutine (issue_schedload) */


static int issue_checker(ISSUE *op,time_t dt)
{
	int		rs = SR_OK ;
	int		nchanged = 0 ;
	if (op->nmaps > 0) {
	    if ((rs = ptm_lock(&op->m)) >= 0) {
	        if (dt == 0) dt = time(NULL) ;
	        if ((dt - op->ti_lastcheck) >= TO_CHECK) {
	            rs = mapper_check(&op->mapper,dt) ;
	            nchanged = rs ;
	            op->ti_lastcheck = dt ;
	        } /* end if */
	        ptm_unlock(&op->m) ;
	    } /* end if */
	} /* end if (positive) */
	return (rs >= 0) ? nchanged : rs ;
}
/* end subroutine (issue_checker) */


static int issue_envbegin(ISSUE *op)
{
	const int	es = envpre[0] ;
	const int	envprelen = strlen(envpre) ;
	int		rs = SR_OK ;
	int		size ;
	int		i ;
	int		c = 0 ;
	int		f ;
	void		*p ;

	for (i = 0 ; environ[i] != NULL ; i += 1) ;

	size = (i + 1) * sizeof(cchar *) ;
	if ((rs = uc_malloc(size,&p)) >= 0) {
	    cchar	*ep ;
	    cchar	**va = (cchar **) p ;
	    op->envv = va ;
	    for (i = 0 ; environ[i] != NULL ; i += 1) {
	        ep = environ[i] ;
	        f = TRUE ;
	        f = f && (ep[0] != '_') ;
	        f = f && (matstr(envbad,ep,-1) < 0) ;
	        if (f && (ep[0] == es)) {
	            f = (strncmp(envpre,ep,envprelen) != 0) ;
		}
	        if (f) {
	            va[c++] = ep ;
		}
	    } /* end for */
	    va[c] = NULL ;
	    op->nenv = c ;
	} /* end if (memory-allocation) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (issue_envbegin) */


static int issue_envend(ISSUE *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op->envv != NULL) {
	    rs1 = uc_free(op->envv) ;
	    if (rs >= 0) rs = rs1 ;
	    op->envv = NULL ;
	}

	return rs ;
}
/* end subroutine (issue_envend) */


static int issue_envadds(ISSUE *op,STRPACK *spp,cchar **ev,cchar *keyname)
{
	const int	envlen = ENVBUFLEN ;
	int		rs = SR_OK ;
	int		n, i ;
	int		el ;
	cchar		**envv = op->envv ;
	cchar		*cp ;
	char		envbuf[ENVBUFLEN + 1] ;

	for (n = 0 ; n < op->nenv ; n += 1) {
	    ev[n] = envv[n] ;
	}

	for (i = 0 ; (rs >= 0) && (envstrs[i] != NULL) ; i += 1) {
	    envbuf[0] = '\0' ;
	    el = -1 ;
	    switch (i) {
	    case envstr_keyname:
	        cp = keyname ;
	        if ((cp != NULL) && (cp[0] != '\0')) {
	            rs = sncpy4(envbuf,envlen,envpre,envstrs[i],"=",cp) ;
	            el = rs ;
	        }
	        break ;
	    } /* end switch */
	    if ((rs >= 0) && (envbuf[0] != '\0')) {
	        rs = issue_envstore(op,spp,ev,n,envbuf,el) ;
	        if (rs > 0) n += 1 ;
	    }
	} /* end for */
	ev[n] = NULL ; /* very important! */

	return (rs >= 0) ? n : rs ;
}
/* end subroutine (issue_envadds) */


static int issue_envstore(ISSUE *op,STRPACK *spp,cchar *ev[],int n,
cchar *ep,int el)
{
	int		rs = SR_OK ;
	cchar		*cp ;

	if (op == NULL) return SR_FAULT ;

	if (ep != NULL) {
	    rs = strpack_store(spp,ep,el,&cp) ;
	    if (rs >= 0) {
	        ev[n++] = cp ;
	        rs = n ;
	    }
	}

	return rs ;
}
/* end subroutine (issue_envstore) */


static int issue_processor(ISSUE *op,cchar **ev,cchar **adms,cchar *gn,int fd)
{
	int		rs ;
	int		wlen = 0 ;

#if	CF_DEBUGS
	debugprintf("issue_processor: ent\n") ;
#endif

	if ((rs = issue_checker(op,0)) >= 0) {
	    if (op->nmaps > 0) {
	        rs = mapper_process(&op->mapper,ev,adms,gn,fd) ;
	        wlen += rs ;
	    }
	}

#if	CF_DEBUGS
	debugprintf("issue_processor: ret rs=%d\n",rs) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (issue_processor) */


#if	CF_ISSUEAUDIT
static int issue_audit(ISSUE *op)
{
	int		rs ;

#if	CF_DEBUGS
	debugprintf("issue_audit: ent\n") ;
#endif

#if	CF_MAPPERAUDIT
	if (op->nmaps > 0) {
	    rs = mapper_audit(&op->mapper) ;
#if	CF_DEBUGS
	    debugprintf("issue_audit: mapper_audit() rs=%d\n",rs) ;
#endif
	}
#endif /* CF_MAPPERAUDIT */

#if	CF_DEBUGS
	debugprintf("issue_audit: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (issue_audit) */
#endif /* CF_ISSUEAUDIT */


static int mapper_start(ISSUE_MAPPER *mmp,time_t dt,cchar fname[])
{
	int		rs ;
	cchar		**evp = (cchar **) environ ;
	cchar		*ccp ;

#if	CF_DEBUGS
	debugprintf("mapper_start: sizeof(PTM)=%u\n",
	    sizeof(PTM)) ;
#endif

	memset(mmp,0,sizeof(ISSUE_MAPPER)) ;

	if ((rs = lockrw_create(&mmp->rwm,0)) >= 0) {
	    if ((rs = uc_mallocstrw(fname,-1,&ccp)) >= 0) {
	        mmp->fname = ccp ;
	        if ((rs = vechand_start(&mmp->mapdirs,4,0)) >= 0) {
	            cchar	*fn = mmp->fname ;
	            if ((rs = paramfile_open(&mmp->dirsfile,evp,fn)) >= 0) {
	                const int	to = TO_MAPCHECK ;
	                if ((rs = paramfile_checkint(&mmp->dirsfile,to)) >= 0) {
	                    mmp->magic = ISSUE_MAPPERMAGIC ;
	                    mmp->ti_check = dt ;
	                    rs = mapper_mapload(mmp) ;
	                    if (rs < 0)
	                        mmp->magic = 0 ;
	                } /* end if */
	                if (rs < 0)
	                    paramfile_close(&mmp->dirsfile) ;
	            } /* end if (paramfile_open) */
	            if (rs < 0)
	                vechand_finish(&mmp->mapdirs) ;
	        } /* end if (vechand_start) */
	        if (rs < 0) {
	            uc_free(mmp->fname) ;
	            mmp->fname = NULL ;
	        }
	    } /* end if (memory-allocation) */
	    if (rs < 0)
	        lockrw_destroy(&mmp->rwm) ;
	} /* end if (lockrw_create) */

#if	CF_DEBUGS
	debugprintf("issue/mapper_start: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (mapper_start) */


static int mapper_finish(ISSUE_MAPPER *mmp)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (mmp == NULL) return SR_FAULT ;

	if (mmp->magic != ISSUE_MAPPERMAGIC) return SR_NOTOPEN ;

	rs1 = paramfile_close(&mmp->dirsfile) ;
	if (rs >= 0) rs = rs1 ;

#if	CF_DEBUGS
	debugprintf("issue/mapper_finish: 1 rs=%d\n",rs) ;
#endif

	rs1 = mapper_mapfins(mmp) ;
	if (rs >= 0) rs = rs1 ;

#if	CF_DEBUGS
	debugprintf("issue/mapper_finish: 2 rs=%d\n",rs) ;
#endif

	rs1 = vechand_finish(&mmp->mapdirs) ;
	if (rs >= 0) rs = rs1 ;

#if	CF_DEBUGS
	debugprintf("issue/mapper_finish: 3 rs=%d\n",rs) ;
#endif

	if (mmp->fname != NULL) {
	    rs1 = uc_free(mmp->fname) ;
	    if (rs >= 0) rs = rs1 ;
	    mmp->fname = NULL ;
	}

#if	CF_DEBUGS
	debugprintf("issue/mapper_finish: 4 rs=%d\n",rs) ;
#endif

	rs1 = lockrw_destroy(&mmp->rwm) ;
	if (rs >= 0) rs = rs1 ;

	mmp->magic = 0 ;

#if	CF_DEBUGS
	debugprintf("issue/mapper_finish: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (mapper_finish) */


static int mapper_check(ISSUE_MAPPER *mmp,time_t dt)
{
	const int	to_lock = TO_LOCK ;
	int		rs ;
	int		nchanged = 0 ;

	if (mmp == NULL) return SR_FAULT ;

	if (mmp->magic != ISSUE_MAPPERMAGIC) return SR_NOTOPEN ;

	if ((rs = lockrw_wrlock(&mmp->rwm,to_lock)) >= 0) {
	    const int	to = TO_MAPCHECK ;

	    if (dt == 0) dt = time(NULL) ;

	    if ((dt - mmp->ti_check) >= to) {

	        if ((rs = paramfile_check(&mmp->dirsfile,dt)) > 0) {

	            {
	                mapper_mapfins(mmp) ;
	                vechand_delall(&mmp->mapdirs) ;
	            }

	            rs = mapper_mapload(mmp) ;
	            nchanged = rs ;

	        } /* end if */

	    } /* end if (map-object check) */

	    lockrw_unlock(&mmp->rwm) ;
	} /* end if (read-write lock) */

#if	CF_DEBUGS
	debugprintf("issue/mapper_check: ret rs=%d nchanged=%u\n",
		rs,nchanged) ;
#endif

	return (rs >= 0) ? nchanged : rs ;
}
/* end subroutine (mapper_check) */


static int mapper_process(ISSUE_MAPPER *mmp,cchar **ev,cchar **adms,
		cchar *gn,int fd)
{
	const int	to_lock = TO_LOCK ;
	int		rs = SR_OK ;
	int		wlen = 0 ;

	if (mmp == NULL) return SR_FAULT ;

	if (mmp->magic != ISSUE_MAPPERMAGIC) return SR_NOTOPEN ;

#if	CF_DEBUGS
	debugprintf("issue/mapper_process: ent\n") ;
#endif

#if	CF_MAPPERAUDIT
	rs = mapper_audit(mmp) ;
	if (rs < 0) goto ret0 ;
#endif /* CF_MAPPERAUDIT */

#if	CF_DEBUGS
	debugprintf("issue/mapper_process: _audit() rs=%d\n",rs) ;
#endif

	if ((rs = lockrw_rdlock(&mmp->rwm,to_lock)) >= 0) {

#if	CF_DEBUGS
	    debugprintf("issue/mapper_process: gn=%s\n",gn) ;
	    if (adms != NULL) {
	        int	i ;
	        for (i = 0 ; adms[i] != NULL ; i += 1) {
	            debugprintf("issue/mapper_process: a%u=%s\n",i,adms[i]) ;
		}
	    }
#endif /* CF_DEBUGS */

	    rs = mapper_processor(mmp,ev,adms,gn,fd) ;
	    wlen += rs ;

#if	CF_DEBUGS
	    debugprintf("issue/mapper_process: mapper_processor() rs=%d\n",rs) ;
#endif

	    lockrw_unlock(&mmp->rwm) ;
	} /* end if (read-write lock) */

#if	CF_MAPPERAUDIT
ret0:
#endif /* CF_MAPPERAUDIT */

#if	CF_DEBUGS
	debugprintf("issue/mapper_process: ret rs=%d\n",rs) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (mapper_process) */


static int mapper_processor(ISSUE_MAPPER *mmp,cchar **ev,cchar **adms,
		cchar *gn,int fd)
{
	ISSUE_MAPDIR	*ep ;
	int		rs = SR_OK ;
	int		i ;
	int		wlen = 0 ;

	if (mmp == NULL) return SR_FAULT ;

	if (mmp->magic != ISSUE_MAPPERMAGIC) return SR_NOTOPEN ;

#if	CF_DEBUGS
	debugprintf("issue/mapper_processor: gn=%s\n",gn) ;
#endif

	for (i = 0 ; vechand_get(&mmp->mapdirs,i,&ep) >= 0 ; i += 1) {
	    if (ep != NULL) {
	        rs = mapdir_process(ep,ev,adms,gn,fd) ;
	        wlen += rs ;
	    }
	    if (rs < 0) break ;
	} /* end for */

#if	CF_DEBUGS
	debugprintf("issue/mapper_processor: ret rs=%d wlen=%u\n",rs,wlen) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (mapper_processor) */


static int mapper_mapload(ISSUE_MAPPER *mmp)
{
	struct ustat	sb ;
	int		rs = SR_OK ;
	int		c = 0 ;

	if (mmp == NULL) return SR_FAULT ;

	if (mmp->magic != ISSUE_MAPPERMAGIC) return SR_NOTOPEN ;

	if (u_stat(mmp->fname,&sb) >= 0) {
	    PARAMFILE		*pfp = &mmp->dirsfile ;
	    PARAMFILE_ENT	pe ;
	    PARAMFILE_CUR	cur ;
	    mmp->ti_mtime = sb.st_mtime ;
	    if ((rs = paramfile_curbegin(pfp,&cur)) >= 0) {
	        const int	plen = PBUFLEN ;
	        int		kl, vl ;
	        int		fl ;
	        cchar		*kp, *vp ;
	        char		pbuf[PBUFLEN + 1] ;

	        while (rs >= 0) {
	            kl = paramfile_enum(pfp,&cur,&pe,pbuf,plen) ;
	            if (kl == SR_NOTFOUND) break ;
	            rs = kl ;
	            if (rs < 0) break ;

	            kp = pe.key ;
	            vp = pe.value ;
	            vl = pe.vlen ;

	            while ((fl = sichr(vp,vl,CH_FS)) >= 0) {
	                if (fl > 0) {
	                    c += 1 ;
	                    rs = mapper_mapadd(mmp,kp,kl,vp,fl) ;
	                }
	                vl -= (fl+1) ;
	                vp = (vp+(fl+1)) ;
	                if (rs < 0) break ;
	            } /* end while */

	            if ((rs >= 0) && (vl > 0)) {
	                c += 1 ;
	                rs = mapper_mapadd(mmp,kp,kl,vp,vl) ;
	            }

	        } /* end while */

	        paramfile_curend(pfp,&cur) ;
	    } /* end if (paramfile-cursor) */
	} /* end if (stat) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (mapper_mapload) */


static int mapper_mapadd(ISSUE_MAPPER *mmp,cchar *kp,int kl,cchar *vp,int vl)
{
	ISSUE_MAPDIR	*ep ;
	const int	size = sizeof(ISSUE_MAPDIR) ;
	int		rs ;

	if ((kp == NULL) || (vp == NULL)) return SR_FAULT ;
	if ((kl == 0) || (vl == 0)) return SR_INVALID ;

	if ((rs = uc_malloc(size,&ep)) >= 0) {
	    if ((rs = mapdir_start(ep,kp,kl,vp,vl)) >= 0) {
	        rs = vechand_add(&mmp->mapdirs,ep) ;
	        if (rs < 0)
	            mapdir_finish(ep) ;
	    }
	    if (rs < 0)
	        uc_free(ep) ;
	} /* end if (memory-allocation) */

	return rs ;
}
/* end subroutine (mapper_mapadd) */


static int mapper_mapfins(ISSUE_MAPPER *mmp)
{
	vechand		*mlp = &mmp->mapdirs ;
	int		rs = SR_OK ;
	int		rs1 ;

	if (mmp == NULL) return SR_FAULT ;

	if (mmp->magic != ISSUE_MAPPERMAGIC) return SR_NOTOPEN ;

	if ((rs1 = vechand_count(mlp)) >= 0) {
	    ISSUE_MAPDIR	*ep ;
	    int		i ;
#if	CF_DEBUGS
	    debugprintf("mapper_mapfins: n=%d\n",rs1) ;
#endif
	    for (i = 0 ; (rs1 = vechand_get(mlp,i,&ep)) >= 0 ; i += 1) {
	        if (ep != NULL) {
	            rs1 = mapdir_finish(ep) ;
	            if (rs >= 0) rs = rs1 ;
	            rs1 = vechand_del(mlp,i--) ;
	            if (rs >= 0) rs = rs1 ;
	            rs1 = uc_free(ep) ;
	            if (rs >= 0) rs = rs1 ;
		}
	    } /* end for */
	} /* end if (vechand-count) */
	if ((rs >= 0) && (rs1 != SR_NOTFOUND)) rs = rs1 ;

#if	CF_DEBUGS
	debugprintf("mapper_mapfins: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (mapper_mapfins) */


#if	CF_MAPPERAUDIT
static int mapper_audit(ISSUE_MAPPER *mmp)
{
	vechand		*mlp = &mmp->mapdirs ;
	int		rs ;

	if (mmp == NULL) return SR_FAULT ;

	if (mmp->magic != ISSUE_MAPPERMAGIC) return SR_NOTOPEN ;

	rs = vechand_count(mlp) ;

#if	CF_DEBUGS
	debugprintf("mapper_audit: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (mapper_audit) */
#endif /* CF_MAPPERAUDIT */


static int mapdir_start(ISSUE_MAPDIR *ep,cchar *kp,int kl,cchar *vp,int vl)
{
	int		rs ;

	if (ep == NULL) return SR_FAULT ;
	if ((kp == NULL) || (vp == NULL)) return SR_FAULT ;

	if ((kl == 0) || (vl == 0)) return SR_INVALID ;

	memset(ep,0,sizeof(ISSUE_MAPDIR)) ;

	if (kl < 0)
	    kl = strlen(kp) ;

	if (vl < 0)
	    vl = strlen(vp) ;

	{
	    const int	size = (kl + 1 + vl + 1) ;
	    char	*p ;
	    if ((rs = uc_malloc(size,&p)) >= 0) {
	        char	*bp = p ;
	        ep->admin = bp ;
	        bp = strwcpy(bp,kp,kl) + 1 ;
	        ep->dirname = bp ;
	        bp = strwcpy(bp,vp,vl) + 1 ;
	        rs = lockrw_create(&ep->rwm,0) ;
	        if (rs < 0) {
	            uc_free(ep->admin) ;
	            ep->admin = NULL ;
	        }
	    } /* end if (memory-allocation) */
	} /* end block */

	return rs ;
}
/* end subroutine (mapdir_start) */


static int mapdir_finish(ISSUE_MAPDIR *ep)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (ep == NULL) return SR_FAULT ;

	if (ep->dname != NULL) {
	    rs1 = uc_free(ep->dname) ;
	    if (rs >= 0) rs = rs1 ;
	    ep->dname = NULL ;
	}

	rs1 = lockrw_destroy(&ep->rwm) ;
	if (rs >= 0) rs = rs1 ;

	if (ep->admin != NULL) {
	    rs1 = uc_free(ep->admin) ;
	    if (rs >= 0) rs = rs1 ;
	    ep->admin = NULL ;
	    ep->dirname = NULL ;
	}

	return rs ;
}
/* end subroutine (mapdir_finish) */


static int mapdir_process(ISSUE_MAPDIR *ep,cchar **ev,cchar **adms,
		cchar *gn,int fd)
{
	const int	to_lock = TO_LOCK ;
	int		rs = SR_OK ;
	int		wlen = 0 ;

#if	CF_DEBUGS
	{
	    int	i ;
	    debugprintf("issue/mapdir_process: ent gn=%s\n",gn) ;
	    debugprintf("issue/mapdir_process: dirname=%s\n",ep->dirname) ;
	    if (adms != NULL) {
	        for (i = 0 ; adms[i] != NULL ; i += 1) {
	            debugprintf("issue/mapdir_process: a[%u]=%s\n",
	                i,adms[i]) ;
	        }
	    }
	}
#endif /* CF_DEBUGS */

	if (ep->dirname[0] != '\0') {
	    int	f_continue = TRUE ;
	    if ((adms != NULL) && (adms[0] != NULL)) {
	        f_continue = (matstr(adms,ep->admin,-1) >= 0) ;
	    } /* end if (adms) */
	    if (f_continue) {
	        if ((ep->dirname[0] == '~') && (ep->dname == NULL)) {
	            rs = mapdir_expand(ep) ;
	        }
	        if (rs >= 0) {
	            if ((ep->dirname[0] != '~') || (ep->dname != NULL)) {
	                if ((rs = lockrw_rdlock(&ep->rwm,to_lock)) >= 0) {
			    cchar	*dn = ep->dirname ;
	                    if ((dn[0] != '~') || (ep->dname != NULL)) {
	                        rs = mapdir_processor(ep,ev,gn,fd) ;
	        		wlen += rs ;
	    		    } /* end if */
	    		    lockrw_unlock(&ep->rwm) ;
			} /* end if (locked) */
		    } /* end if (acceptable) */
		} /* end if (ok) */
	    } /* end if (continued) */
	} /* end if (non-nul) */

#if	CF_DEBUGS
	debugprintf("issue/mapdir_process: ret rs=%d wlen=%u\n",rs,wlen) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (mapdir_process) */


static int mapdir_expand(ISSUE_MAPDIR *ep)
{
	const int	to_lock = TO_LOCK ;
	int		rs ;

#if	CF_DEBUGS
	debugprintf("issue/mapdir_expand: dirname=%s\n",ep->dirname) ;
	debugprintf("issue/mapdir_expand: dname=%s\n",ep->dname) ;
#endif

	if ((rs = lockrw_wrlock(&ep->rwm,to_lock)) >= 0) {

	    if ((ep->dirname[0] == '~') && (ep->dname == NULL)) {
	        rs = mapdir_expander(ep) ;

#if	CF_DEBUGS
	        debugprintf("issue/mapdir_expand: mapdir_expander() rs=%d\n",
	            rs) ;
	        debugprintf("issue/mapdir_expand: dname=%s\n",ep->dname) ;
#endif

	    } /* end if */

	    lockrw_unlock(&ep->rwm) ;
	} /* end if (read-write lock) */

	return rs ;
}
/* end subroutine (mapdir_expand) */


static int mapdir_expander(ISSUE_MAPDIR *ep)
{
	int		rs = SR_OK ;
	int		rs1 = SR_OK ;
	int		fl = 0 ;

#if	CF_DEBUGS
	debugprintf("issue/mapdir_expander: dirname=%s\n",ep->dirname) ;
#endif

	if ((ep->dirname != NULL) && (ep->dirname[0] == '~')) {
	    int		unl = -1 ;
	    cchar	*un = (ep->dirname+1) ;
	    cchar	*tp ;
	    cchar	*pp = NULL ;
	    char	ubuf[USERNAMELEN + 1] ;
	    if ((tp = strchr(un,'/')) != NULL) {
	        unl = (tp - un) ;
	        pp = tp ;
	    }
	    if ((unl == 0) || (un[0] == '\0')) {
	        un = ep->admin ;
	        unl = -1 ;
	    }
	    if (unl >= 0) {
		strwcpy(ubuf,un,MIN(unl,USERNAMELEN)) ;
		un = ubuf ;
	    }
	    if ((rs = getbufsize(getbufsize_pw)) >= 0) {
	        struct passwd	pw ;
	        const int	pwlen = rs ;
	        char		*pwbuf ;
	        if ((rs = uc_malloc((pwlen+1),&pwbuf)) >= 0) {
		    if ((rs = GETPW_NAME(&pw,pwbuf,pwlen,un)) >= 0) {
		        cchar	*uh = pw.pw_dir ;
	    	        char	hbuf[MAXPATHLEN + 1] ;
	                if (pp != NULL) {
	                    rs = mkpath2(hbuf,uh,pp) ;
	                    fl = rs ;
	                } else {
	                    rs = mkpath1(hbuf,uh) ;
	                    fl = rs ;
	                }
	                if (rs >= 0) {
	                    cchar	*cp ;
	                    rs = uc_mallocstrw(hbuf,fl,&cp) ;
	                    if (rs >= 0) ep->dname = cp ;
	                }
		    } else if (isNotPresent(rs)) {
		        rs = SR_OK ;
	            } /* end if (getpw_name) */
		    rs1 = uc_free(pwbuf) ;
		    if (rs >= 0) rs = rs1 ;
	        } /* end if (m-a-f) */
	    } /* end if (getbufsize) */
	} else {
	    rs = SR_INVALID ;
	}

	return (rs >= 0) ? fl : rs ;
}
/* end subroutine (mapdir_expander) */


static int mapdir_processor(ISSUE_MAPDIR *ep,cchar **ev,cchar *gn,int fd)
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		n ;
	int		wlen = 0 ;
	int		f_continue = TRUE ;
	cchar		*dn ;
	cchar		*defname = ISSUE_DEFGROUP ;
	cchar		*allname = ISSUE_ALLGROUP ;
	cchar		*name = ISSUE_NAME ;
	char		env_admin[ENVBUFLEN+1] ;
	char		env_admindir[ENVBUFLEN+1] ;

#if	CF_DEBUGS
	debugprintf("issue/mapdir_processor: dir=%s\n",ep->dirname) ;
	debugprintf("issue/mapdir_processor: kn=%s\n",gn) ;
#endif

	dn = ep->dirname ;
	if (dn[0] == '~') {
	    dn = ep->dname ;
	    f_continue = ((dn != NULL) && (dn[0] != '\0')) ;
	}
	if (f_continue) {
	    struct ustat	sb ;
	    if ((rs1 = u_stat(dn,&sb)) >= 0) {
	        VECSTR		nums ;
	        const int	envlen = ENVBUFLEN ;
	        cchar		*post ;
	        post = envstrs[envstr_admin] ;
	        strdcpy4(env_admin,envlen,envpre,post,"=",ep->admin) ;
	        post = envstrs[envstr_admindir] ;
	        strdcpy4(env_admindir,envlen,envpre,post,"=",dn) ;
	        for (n = 0 ; ev[n] != NULL ; n += 1) ;
	        ev[n+0] = env_admin ;
	        ev[n+1] = env_admindir ;
	        ev[n+2] = NULL ;
	        if ((rs = vecstr_start(&nums,0,0)) >= 0) {
		    FSDIR	d ;
		    FSDIR_ENT	de ;
	            int		i ;
	            cchar	*strs[5] ;
	            loadstrs(strs,gn,defname,allname,name) ;
	            if ((rs = fsdir_open(&d,dn)) >= 0) {
	                cchar	*tp ;
	                while ((rs = fsdir_read(&d,&de)) > 0) {
	                    cchar	*den = de.name ;
	                    if (den[0] != '.') {
	                        if ((tp = strchr(den,'.')) != NULL) {
	                            if (strcmp((tp+1),name) == 0) {
	                	    int		f = TRUE ;
	                	    cchar	*digp ;
	                	    digp = strnpbrk(den,(tp-den),"0123456789") ;
	                	    if (digp != NULL) {
	                    	        f = hasalldig(digp,(tp-digp)) ;
				    }
	                	    if (f) {
	                    	        for (i = 0 ; i < 3 ; i += 1) {
	                        	    f = isBaseMatch(den,strs[i],digp) ;
	                        	    if (f) break ;
	                    	        }
	                	    }
	                            if (f) {
	                                rs = vecstr_add(&nums,den,(tp-den)) ;
			            }
				    }
	                        } /* end if (have an ISSUE file) */
		            }
	                    if (rs < 0) break ;
	                } /* end while (reading directory entries) */
	                rs1 = fsdir_close(&d) ;
			if (rs >= 0) rs = rs1 ;
	            } /* end if (fsdir) */
	            if (rs >= 0) {
	                vecstr_sort(&nums,NULL) ;
	                rs = mapdir_processorthem(ep,ev,dn,&nums,strs,fd) ;
	                wlen += rs ;
	            } /* end if */
	            vecstr_finish(&nums) ;
	        } /* end if (nums) */
	        {
	            ev[n] = NULL ;
	        }
	    } /* end if (u_stat) */
	} /* end if (continued) */

#if	CF_DEBUGS
	debugprintf("issue/mapdir_processor: ret rs=%d wlen=%u\n",rs,wlen) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (mapdir_processor) */


static int mapdir_processorthem(ISSUE_MAPDIR *ep,cchar **ev,cchar *dn,
		VECSTR *blp,cchar **strs,int fd)
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		wlen = 0 ;
	cchar		*kn = strs[0] ;

	rs1 = mapdir_processorone(ep,ev,dn,blp,kn,fd) ;
	if (isNotPresent(rs1)) {
	    kn = strs[1] ;
	    rs1 = mapdir_processorone(ep,ev,dn,blp,kn,fd) ;
	    if (! isNotPresent(rs1)) rs = rs1 ;
	} else {
	    rs = rs1 ;
	}
	if (rs > 0) wlen += rs ;

	if (rs >= 0) {
	    kn = strs[2] ;
	    rs1 = mapdir_processorone(ep,ev,dn,blp,kn,fd) ;
	    if (! isNotPresent(rs1)) rs = rs1 ;
	    if (rs > 0) wlen += rs ;
	}

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (mapdir_processorthem) */


static int mapdir_processorone(ISSUE_MAPDIR *ep,cchar **ev,cchar *dn,
		VECSTR *blp,cchar *kn,int fd)
{
	const int	kl = strlen(kn) ;
	int		rs = SR_OK ;
	int		i ;
	int		c = 0 ;
	int		wlen = 0 ;
	cchar		*bep ;

#if	CF_DEBUGS
	debugprintf("issue/mapdir_processorone: kn=%s\n",kn) ;
#endif

	for (i = 0 ; vecstr_get(blp,i,&bep) >= 0 ; i += 1) {
	    if (bep != NULL) {
	        if (strncmp(bep,kn,kl) == 0) {
	            c += 1 ;
	            if ((rs = mapdir_procout(ep,ev,dn,bep,fd)) >= 0) {
	                wlen += rs ;
	            } else if (isNotPresent(rs)) {
	                rs = SR_OK ;
		    }
	        }
	    }
	    if (rs < 0) break ;
	} /* end for */

	if ((rs >= 0) && (c == 0)) rs = SR_NOENT ;

#if	CF_DEBUGS
	debugprintf("issue/mapdir_processorone: ret rs=%d wlen=%u\n",rs,wlen) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (mapdir_processorone) */


/* we must return SR_NOENT if there was no file */
static int mapdir_procout(ISSUE_MAPDIR *ep,cchar **ev,cchar *dn,
		cchar *gn,int fd)
{
	const int	clen = MAXNAMELEN ;
	int		rs ;
	int		wlen = 0 ;
	cchar		*name = ISSUE_NAME ;
	char		cbuf[MAXNAMELEN + 1] ;

/* we ignore buffer overflow here */

	if ((rs = snsds(cbuf,clen,gn,name)) >= 0) {
	    char	fname[MAXPATHLEN + 1] ;
	    if ((rs = mkpath2(fname,dn,cbuf)) >= 0) {
	        rs = mapdir_procouter(ep,ev,fname,fd) ;
	        wlen += rs ;
	    } else if (rs == SR_OVERFLOW) {
	        rs = SR_NOENT ;
	    }
	} else if (rs == SR_OVERFLOW) {
	    rs = SR_NOENT ;
	}

#if	CF_DEBUGS
	debugprintf("issue/mapdir_procout: ret rs=%d wlen=%u\n",rs,wlen) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (mapdir_procout) */


static int mapdir_procouter(ISSUE_MAPDIR *ep,cchar **ev,cchar *fname,int ofd)
{
	const mode_t	operms = 0664 ;
	const int	oflags = O_RDONLY ;
	const int	to_open = TO_OPEN ;
	const int	to_read = TO_READ ;
	const int	to_write = TO_WRITE ;
	int		rs ;
	int		wlen = 0 ;

	if (ep == NULL) return SR_FAULT ;

#if	CF_DEBUGS
	debugprintf("issue/mapdir_procouter: ent fname=%s\n",fname) ;
#endif

	if ((rs = uc_openenv(fname,oflags,operms,ev,to_open)) >= 0) {
	    const int	mfd = rs ;
	    const int	olen = MSGBUFLEN ;
	    char	obuf[MSGBUFLEN + 1] ;
#if	CF_DEBUGS
	    debugprintf("issue/mapdir_procouter: uc_openenv() rs=%d\n",rs) ;
#endif
#if	CF_DEBUGN
	    nprintf(NDEBFNAME,"issue/mapdir_procouter: uc_openenv() rs=%d\n",
	        rs) ;
#endif

#if	CF_WRITETO
	    while ((rs = uc_reade(mfd,obuf,olen,to_read,0)) > 0) {
	            rs = writeto(ofd,obuf,rs,to_write) ;
	            wlen += rs ;
	            if (rs < 0) break ;
	    } /* end while */
#else /* CF_WRITETO */
	    rs = uc_writedesc(ofd,mfd,-1) ;
	    wlen += rs ;
#endif /* CF_WRITETO */

	    u_close(mfd) ;
	} /* end if (open) */

#if	CF_DEBUGS
	debugprintf("issue/mapdir_procouter: ret rs=%d wlen=%u\n",rs,wlen) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (mapdir_procouter) */


#if	CF_WRITETO

static int writeto(int wfd,cchar *wbuf,int wlen,int wto)
{
	struct pollfd	fds[2] ;
	time_t		dt = time(NULL) ;
	time_t		ti_write ;
	int		rs = SR_OK ;
	int		i ;
	int		pt = TO_POLL ;
	int		pto ;
	int		tlen = 0 ;

	if (wbuf == NULL) return SR_FAULT ;

	if (wfd < 0) return SR_BADF ;

	if (wlen < 0)
	    wlen = strlen(wbuf) ;

	if (pt > wto)
	    pt = wto ;

	i = 0 ;
	fds[i].fd = wfd ;
	fds[i].events = POLLOUT ;
	i += 1 ;
	fds[i].fd = -1 ;
	fds[i].events = 0 ;

	ti_write = dt ;
	pto = (pt * POLLMULT) ;
	while ((rs >= 0) && (tlen < wlen)) {

	    rs = u_poll(fds,1,pto) ;

	    dt = time(NULL) ;
	    if (rs > 0) {
	        const int	re = fds[0].revents ;

	        if (re & POLLOUT) {
	            rs = u_write(wfd,(wbuf+tlen),(wlen-tlen)) ;
	            tlen += rs ;
	            ti_write = dt ;
	        } else if (re & POLLHUP) {
	            rs = SR_HANGUP ;
	        } else if (re & POLLERR) {
	            rs = SR_POLLERR ;
	        } else if (re & POLLNVAL) {
	            rs = SR_NOTOPEN ;
	        } /* end if (poll returned) */

	    } /* end if (got something) */

	    if (rs == SR_INTR)
	        rs = SR_OK ;

	    if ((dt - ti_write) >= wto)
	        break ;

	} /* end while */

	return (rs >= 0) ? tlen : rs ;
}
/* end subroutine (writeto) */

#endif /* CF_WRITETO */


static int loadstrs(cchar **strs,cchar *gn,cchar *def,cchar *all,cchar	*n)
{
	int		i = 0 ;

#ifdef	COMMENT
	{
	    cchar	*cp ;
	    for (i = 0 ; i < 4 ; i += 1) {
	        switch (i) {
	        case 0:
	            cp = gn ;
	            break ;
	        case 1:
	            cp = def ;
	            break ;
	        case 2:
	            cp = all ;
	            break ;
	        case 3:
	            cp = n ;
	            break ;
	        } /* end switch */
	        strs[i] = cp ;
	    } /* end for */
	}
#else
	strs[i++] = gn ;
	strs[i++] = def ;
	strs[i++] = all ;
	strs[i++] = n ;
#endif /* COMMENT */
	strs[i] = NULL ;

	return SR_OK ;
}
/* end subroutine (loadstrs) */


static int isBaseMatch(cchar *den,cchar *bname,cchar *digp)
{
	int		f = FALSE ;

	if (digp == NULL) {
	    int	bl = strlen(bname) ;
	    int	m = nleadstr(den,bname,bl) ;
	    f = (m == bl) && (den[m] == '.') ;
	} else {
	    f = (strncmp(den,bname,(digp-den)) == 0) ;
	}

	return f ;
}
/* end subroutine (isBaseMatch) */


