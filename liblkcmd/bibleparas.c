/* bibleparas */

/* BIBLEPARAS implementation */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_SAFE		0		/* normal safety */
#define	CF_INORDER	0		/* create indices as encountered */
#define	CF_TMPPRNAME	1		/* put under a PRNAME in /tmp */
#define	CF_STARTAUDIT	1		/* start w/ audit on BPI object */


/* revision history:

	- 2008-10-01, David A­D­ Morano
	This object module was originally written.

*/

/* Copyright © 2008 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This module implements an interface (a trivial one) that allows access
        to the BIBLEPARAS datbase.


*******************************************************************************/


#define	BIBLEPARAS_MASTER	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<sys/mman.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<endianstr.h>
#include	<vecobj.h>
#include	<sbuf.h>
#include	<char.h>
#include	<vecstr.h>
#include	<expcook.h>
#include	<dirseen.h>
#include	<localmisc.h>

#include	"bibleparas.h"
#include	"bpi.h"
#include	"bpimk.h"


/* local defines */

#define	BIBLEPARAS_DBDNAME	"share/bibleparas"
#define	BIBLEPARAS_DBSUF	"txt"

#define	BIBLEPARAS_ENT		struct bibleparas_e

#define	BIBLEPARAS_NLE		4	/* default number line entries */
#define	BIBLEPARAS_DIRMODE	0777
#define	BIBLEPARAS_IDXMODE	0664

#ifndef	LINEBUFLEN
#ifdef	LINE_MAX
#define	LINEBUFLEN	MAX(LINE_MAX,2048)
#else
#define	LINEBUFLEN	2048
#endif
#endif

#ifndef	VARTMPDNAME
#define	VARTMPDNAME	"TMPDIR"
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

#define	INDCNAME	"bibleparas"
#define	INDSUF		"pi"

#undef	NLINES
#define	NLINES		20

#define	TO_FILEMOD	(60 * 24 * 3600)
#define	TO_MKWAIT	(5 * 50)
#define	TO_CHECK	4

#define	SUBINFO		struct subinfo
#define	SUBINFO_FL	struct subinfo_flags


/* external subroutines */

extern int	snsds(char *,int,cchar *,cchar *) ;
extern int	snwcpy(char *,int,cchar *,int) ;
extern int	sncpy1(char *,int,cchar *) ;
extern int	sncpy2(char *,int,cchar *) ;
extern int	mkpath2(char *,cchar *,cchar *) ;
extern int	mkpath3(char *,cchar *,cchar *,cchar *) ;
extern int	mkfnamesuf1(char *,cchar *,cchar *) ;
extern int	mkfnamesuf2(char *,cchar *,cchar *,cchar *) ;
extern int	sfskipwhite(cchar *,int,cchar **) ;
extern int	sfbasename(cchar *,int,cchar **) ;
extern int	siskipwhite(cchar *,int) ;
extern int	nleadstr(cchar *,cchar *,int) ;
extern int	cfdeci(cchar *,int,int *) ;
extern int	cfdecui(cchar *,int,uint *) ;
extern int	pathclean(char *,cchar *,int) ;
extern int	mkdirs(cchar *,mode_t) ;
extern int	perm(cchar *,uid_t,gid_t,gid_t *,int) ;
extern int	isdigitlatin(int) ;
extern int	isNotPresent(int) ;

#if	CF_DEBUGS
extern int	debugprintf(cchar *,...) ;
#endif

extern char	*strwcpy(char *,cchar *,int) ;
extern char	*strnchr(cchar *,int,int) ;
extern char	*strnpbrk(cchar *,int,cchar *) ;


/* local structures */

struct subinfo_flags {
	uint		dummy:1 ;
} ;

struct subinfo {
	SUBINFO_FL	f ;
	time_t		dt ;
} ;

struct bibleparas_e {
	uchar		b, c, v ;
} ;


/* forward references */

static int	bibleparas_dbloadbegin(BIBLEPARAS *,SUBINFO *) ;
static int	bibleparas_dbloadend(BIBLEPARAS *) ;
static int	bibleparas_dbmapcreate(BIBLEPARAS *,time_t) ;
static int	bibleparas_dbmapdestroy(BIBLEPARAS *) ;
static int	bibleparas_checkupdate(BIBLEPARAS *,time_t) ;
static int	bibleparas_indopen(BIBLEPARAS *,SUBINFO *) ;

static int	bibleparas_indopencheck(BIBLEPARAS *,cchar *) ;

static int	bibleparas_indclose(BIBLEPARAS *) ;
static int	bibleparas_indmk(BIBLEPARAS *,cchar *,time_t) ;
static int	bibleparas_indmkdata(BIBLEPARAS *,cchar *,mode_t) ;
static int	bibleparas_indopenseq(BIBLEPARAS *,SUBINFO *) ;
static int	bibleparas_indopenseqer(BIBLEPARAS *,SUBINFO *,
			DIRSEEN *,vecstr *,EXPCOOK *) ;
static int	bibleparas_loadcooks(BIBLEPARAS *,EXPCOOK *) ;
static int	bibleparas_indopenalt(BIBLEPARAS *,SUBINFO *,DIRSEEN *) ;

#ifdef	COMMENT
static int	bibleparas_mksysvarsi(BIBLEPARAS *,cchar *) ;
#endif

static int	subinfo_start(SUBINFO *) ;
static int	subinfo_finish(SUBINFO *) ;

static int	entry_start(BIBLEPARAS_ENT *,BIBLEPARAS_Q *) ;
static int	entry_finish(BIBLEPARAS_ENT *) ;

#ifdef	COMMENT
static int	entry_release(BIBLEPARAS_ENT *) ;
#endif

static int	bvemk_start(BPIMK_VERSE *,BIBLEPARAS_ENT *) ;
static int	bvemk_finish(BPIMK_VERSE *) ;

static int	mkdname(cchar *,mode_t) ;
static int	checkdname(cchar *) ;

static int	isempty(cchar *,int) ;
static int	isstart(cchar *,int,BIBLEPARAS_Q *,int *) ;


/* exported variables */

BIBLEPARAS_OBJ	bibleparas = {
	"bibleparas",
	sizeof(BIBLEPARAS),
	sizeof(int)
} ;


/* local variables */

static cchar	*idxdirs[] = {
	"%R/var/%S",
	"/var/tmp/%{PRN}",
	"/var/tmp",
	"/tmp/%{PRN}",
	"/tmp",
	"%T",
	NULL
} ;


/* exported subroutines */


int bibleparas_open(BIBLEPARAS *op,cchar pr[],cchar dbname[])
{
	int		rs ;
	int		nverses = 0 ;
	cchar		*suf = BIBLEPARAS_DBSUF ;
	char		cname[MAXNAMELEN + 1] ;
	char		dbfname[MAXPATHLEN + 1] ;

#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;
#endif

	if (pr == NULL) return SR_FAULT ;

	if (pr[0] == '\0') return SR_INVALID ;

	if ((dbname == NULL) || (dbname[0] == '\0'))
	    dbname = BIBLEPARAS_DBNAME ;

	memset(op,0,sizeof(BIBLEPARAS)) ;
	op->pr = pr ;
	op->dbname = dbname ;

	if ((rs = snsds(cname,MAXNAMELEN,dbname,suf)) >= 0) {
	    cchar	*dn = BIBLEPARAS_DBDNAME ;
	    if ((rs = mkpath3(dbfname,pr,dn,cname)) >= 0) {
	        cchar	*cp ;
	        if ((rs = uc_mallocstrw(dbfname,-1,&cp)) >= 0) {
		    SUBINFO	si ;
	    	    op->dbfname = cp ;
	            if ((rs = subinfo_start(&si)) >= 0) {
	    	        if ((rs = bibleparas_dbloadbegin(op,&si)) >= 0) {
			    nverses = op->nverses ;
			    op->magic = BIBLEPARAS_MAGIC ;
	                }
	    	        subinfo_finish(&si) ;
		    } /* end if (subinfo) */
		    if (rs < 0) {
	                uc_free(op->dbfname) ;
	                op->dbfname = NULL ;
		    }
	        } /* end if (m-a) */
	    } /* end if (mkpath) */
	} /* end if (snsds) */

	return (rs >= 0) ? nverses : rs ;
}
/* end subroutine (bibleparas_open) */


/* free up the entire vector string data structure object */
int bibleparas_close(BIBLEPARAS *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;

	if (op->magic != BIBLEPARAS_MAGIC) return SR_NOTOPEN ;
#endif

	rs1 = bibleparas_dbloadend(op) ;
	if (rs >= 0) rs = rs1 ;

	if (op->dbfname != NULL) {
	    rs1 = uc_free(op->dbfname) ;
	    if (rs >= 0) rs = rs1 ;
	    op->dbfname = NULL ;
	}

	op->magic = 0 ;
	return rs ;
}
/* end subroutine (bibleparas_close) */


int bibleparas_count(BIBLEPARAS *op)
{
	int		rs ;

#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;

	if (op->magic != BIBLEPARAS_MAGIC) return SR_NOTOPEN ;
#endif

	rs = op->nverses ;

	return rs ;
}
/* end subroutine (bibleparas_count) */


int bibleparas_audit(BIBLEPARAS *op)
{
	int		rs = SR_OK ;

#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;

	if (op->magic != BIBLEPARAS_MAGIC) return SR_NOTOPEN ;
#endif

	if (op->f.vind) {
	    rs = bpi_audit(&op->vind) ;
	}

	return rs ;
}
/* end subroutine (bibleparas_audit) */


/* get a string by its index */
int bibleparas_ispara(BIBLEPARAS *op,BIBLEPARAS_Q *qp)
{
	time_t		dt = 0 ;
	int		rs = SR_OK ;
	int		f = FALSE ;

#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;

	if (op->magic != BIBLEPARAS_MAGIC) return SR_NOTOPEN ;
#endif

	if (qp == NULL) return SR_FAULT ;

/* check for update */

	if (op->ncursors == 0) {
	    rs = bibleparas_checkupdate(op,dt) ;
	}

#if	CF_DEBUGS
	debugprintf("bibleparas_ispara: q=%u:%u:%u\n",
		qp->b,qp->c,qp->v) ;
#endif

	if (rs >= 0) {
	    BPI_Q	viq ;
	    viq.b = qp->b ;
	    viq.c = qp->c ;
	    viq.v = qp->v ;
	    if ((rs = bpi_get(&op->vind,&viq)) >= 0) {
	        f = TRUE ;
	    } else if (rs == SR_NOTFOUND) 
	        rs = SR_OK ;
	} /* end if (ok) */

#if	CF_DEBUGS
	debugprintf("bibleparas_ispara: ret rs=%d f=%u\n",rs,f) ;
#endif

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (bibleparas_ispara) */


int bibleparas_curbegin(BIBLEPARAS *op,BIBLEPARAS_CUR *curp)
{
	int		rs ;

#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;

	if (op->magic != BIBLEPARAS_MAGIC) return SR_NOTOPEN ;
#endif

	if (curp == NULL) return SR_FAULT ;

	if ((rs = bpi_curbegin(&op->vind,&curp->vicur)) >= 0) {
	    op->ncursors += 1 ;
	}

	return rs ;
}
/* end subroutine (bibleparas_curbegin) */


int bibleparas_curend(BIBLEPARAS *op,BIBLEPARAS_CUR *curp)
{
	int		rs ;

#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;

	if (op->magic != BIBLEPARAS_MAGIC) return SR_NOTOPEN ;
#endif

	if (curp == NULL) return SR_FAULT ;

	rs = bpi_curend(&op->vind,&curp->vicur) ;
	if (op->ncursors > 0)
	    op->ncursors -= 1 ;

	return rs ;
}
/* end subroutine (bibleparas_curend) */


int bibleparas_enum(BIBLEPARAS *op,BIBLEPARAS_CUR *curp,BIBLEPARAS_Q *qp)
{
	time_t		dt = 0 ;
	int		rs = SR_OK ;

#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;

	if (op->magic != BIBLEPARAS_MAGIC) return SR_NOTOPEN ;
#endif

	if (curp == NULL) return SR_FAULT ;
	if (qp == NULL) return SR_FAULT ;

	if (op->ncursors == 0) {
	    rs = bibleparas_checkupdate(op,dt) ;
	}

	if (rs >= 0) {
	    BPI_VERSE	viv ;
	    if ((rs = bpi_enum(&op->vind,&curp->vicur,&viv)) >= 0) {
	        if (qp != NULL) {
		    qp->b = viv.b ;
		    qp->c = viv.c ;
		    qp->v = viv.v ;
	        }
	    } /* end if (bpi_enum) */
	} /* end if (ok) */

#if	CF_DEBUGS
	debugprintf("bibleparas_enum: ret rs=%d\n", rs) ;
#endif

	return rs ;
}
/* end subroutine (bibleparas_enum) */


int bibleparas_info(BIBLEPARAS *op,BIBLEPARAS_INFO *ip)
{
	BPI_INFO	bi ;
	int		rs ;
	int		nverses = 0 ;

#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;

	if (op->magic != BIBLEPARAS_MAGIC) return SR_NOTOPEN ;
#endif

	if ((rs = bpi_info(&op->vind,&bi)) >= 0) {
	    nverses = bi.count ;
	    if (ip != NULL) {
	        memset(ip,0,sizeof(BIBLEPARAS_INFO)) ;
	        ip->dbtime = op->ti_db ;
	        ip->vitime = op->ti_vind ;
	        ip->maxbook = bi.maxbook ;
	        ip->maxchapter = bi.maxchapter ;
	        ip->nverses = bi.count ;
	        ip->nzverses = bi.nzverses ;
	    }
	} /* end if (bpi_info) */

	return (rs >= 0) ? nverses : rs ;
}
/* end subroutine (bibleparas_info) */


/* private subroutines */


static int bibleparas_dbloadbegin(BIBLEPARAS *op,SUBINFO *sip)
{
	int		rs ;

#if	CF_DEBUGS
	debugprintf("bibleparas_dbloadbegin: ent\n") ;
#endif

	if ((rs = bibleparas_dbmapcreate(op,sip->dt)) >= 0) {
	    rs = bibleparas_indopen(op,sip) ;
	    if (rs < 0)
		bibleparas_dbmapdestroy(op) ;
	}

#if	CF_DEBUGS
	debugprintf("bibleparas_dbloadbegin: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (bibleparas_dbloadbegin) */


static int bibleparas_dbloadend(BIBLEPARAS *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	rs1 = bibleparas_indclose(op) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = bibleparas_dbmapdestroy(op) ;
	if (rs >= 0) rs = rs1 ;

	return rs ;
}
/* end subroutine (bibleparas_dbloadend) */


static int bibleparas_dbmapcreate(BIBLEPARAS *op,time_t dt)
{
	int		rs ;

#if	CF_DEBUGS
	debugprintf("bibleparas_dbmapcreate: dbfname=%s\n",op->dbfname) ;
#endif

	if ((rs = u_open(op->dbfname,O_RDONLY,0666)) >= 0) {
	    struct ustat	sb ;
	    int			fd = rs ;
	    if ((rs = u_fstat(fd,&sb)) >= 0) {
	        if (S_ISREG(sb.st_mode)) {
	            if (sb.st_size <= INT_MAX) {
	                size_t	ms ;
	                int	mp = PROT_READ ;
	                int	mf = MAP_SHARED ;
	                void	*md ;
			op->filesize = (sb.st_size & UINT_MAX) ;
			op->ti_db = sb.st_mtime ;
	    		ms = (size_t) op->filesize ;
	    		if ((rs = u_mmap(NULL,ms,mp,mf,fd,0L,&md)) >= 0) {
			    const int	madv = MADV_RANDOM ;
			    const caddr_t	ma = md ;
	        	    if ((rs = uc_madvise(ma,ms,madv)) >= 0) {
	            		op->mapdata = md ;
	            		op->mapsize = ms ;
	            		op->ti_map = dt ;
	            		op->ti_lastcheck = dt ;
			    }
	        	    if (rs < 0) {
		    	        u_munmap(md,ms) ;
	                        op->mapdata = NULL ;
	                        op->mapsize = 0 ;
	                    }
	                } /* end if (u_mmap) */
	            } else
	                rs = SR_TOOBIG ;
	        } else
	            rs = SR_NOTSUP ;
	    } /* end if (stat) */
	    u_close(fd) ;
	} /* end if (file) */

	return rs ;
}
/* end subroutine (bibleparas_dbmapcreate) */


static int bibleparas_dbmapdestroy(BIBLEPARAS *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op->mapdata != NULL) {
	    rs1 = u_munmap(op->mapdata,op->mapsize) ;
	    if (rs >= 0) rs = rs1 ;
	    op->mapdata = NULL ;
	    op->mapsize = 0 ;
	}

	return rs ;
}
/* end subroutine (bibleparas_dbmapdestroy) */


static int bibleparas_checkupdate(BIBLEPARAS *op,time_t dt)
{
	struct ustat	sb ;
	int		rs = SR_OK ;
	int		f = FALSE ;

	if (op->ncursors == 0) {
	    if (dt <= 0) dt = time(NULL) ;
	    if ((dt - op->ti_lastcheck) >= TO_CHECK) {
	        op->ti_lastcheck = dt ;
	        if ((rs = u_stat(op->dbfname,&sb)) >= 0) {
	            f = f || (sb.st_mtime > op->ti_db) ;
	            f = f || (sb.st_mtime > op->ti_map) ;
	            if (f) {
		        SUBINFO	si ;
	                bibleparas_dbloadend(op) ;
	                if ((rs = subinfo_start(&si)) >= 0) {
	                    rs = bibleparas_dbloadbegin(op,&si) ;
		            subinfo_finish(&si) ;
	                } /* end if */
	            } /* end if (update) */
	        } else if (isNotPresent(rs)) {
	            rs = SR_OK ;
	        }
	    } /* end if (timed-out) */
	} /* end if (no cursors out) */

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (bibleparas_checkupdate) */


static int bibleparas_indopen(BIBLEPARAS *op,SUBINFO *sip)
{
	int		rs ;

	if ((rs = bibleparas_indopenseq(op,sip)) >= 0) {
	    if (op->f.vind) {
	        rs = bpi_count(&op->vind) ;
	        op->nverses = rs ;
	    }
	}

#if	CF_DEBUGS
	debugprintf("bibleparas_indopen: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (bibleparas_indopen) */


static int bibleparas_indopenseq(BIBLEPARAS *op,SUBINFO *sip)
{
	DIRSEEN		ds ;
	int		rs ;
	int		rs1 ;

	if ((rs = dirseen_start(&ds)) >= 0) {
	    vecstr	sdirs ;
	    const int	vopts = VECSTR_OCOMPACT ;
	    if ((rs = vecstr_start(&sdirs,6,vopts)) >= 0) {
	        EXPCOOK	cooks ;
	        if ((rs = expcook_start(&cooks)) >= 0) {
	            if ((rs = bibleparas_loadcooks(op,&cooks)) >= 0) {
	                rs = bibleparas_indopenseqer(op,sip,&ds,&sdirs,&cooks) ;
	            }
	            rs1 = expcook_finish(&cooks) ;
	            if (rs >= 0) rs = rs1 ;
	        } /* end if (cooks) */
		rs1 = vecstr_finish(&sdirs) ;
		if (rs >= 0) rs = rs1 ;
	    } /* end if (sdirs) */
	    rs1 = dirseen_finish(&ds) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (dirseen) */

	return rs ;
}
/* end subroutines (bibleparas_indopenseq) */


static int bibleparas_indopenseqer(BIBLEPARAS *op,SUBINFO *sip,DIRSEEN *dsp,
		vecstr *sdp,EXPCOOK *ecp)
{
	const int	rsn = SR_NOTFOUND ;
	const int	elen = MAXPATHLEN ;
	int		rs = SR_OK ;
	int		i ;
	char		ebuf[MAXPATHLEN + 1] ;
	char		pbuf[MAXPATHLEN + 1] ;

/* first phase: expand possible directory paths */

	for (i = 0 ; (rs >= 0) && (idxdirs[i] != NULL) ; i += 1) {
	    if ((rs = expcook_exp(ecp,0,ebuf,elen,idxdirs[i],-1)) > 0) {
		if ((rs = pathclean(pbuf,ebuf,rs)) > 0) {
		    const int	plen = rs ;
		    if ((rs = dirseen_havename(dsp,pbuf,plen)) == rsn) {
			rs = dirseen_add(dsp,pbuf,plen,NULL) ;
		    }
		}
	    }
	} /* end for */

/* next phase: create DB file-paths from directories */

	if (rs >= 0) {
	    DIRSEEN_CUR	cur ;
	    if ((rs = dirseen_curbegin(dsp,&cur)) >= 0) {
		int	el ;
	        while ((el = dirseen_enum(dsp,&cur,ebuf,elen)) >= 0) {
		    if ((rs = mkpath2(pbuf,ebuf,op->dbname)) >= 0) {
		        rs = vecstr_add(sdp,pbuf,rs) ;
		    }
		    if (rs < 0) break ;
	        } /* end while */
		if ((rs >= 0) && (el != SR_NOTFOUND)) rs = el ;
	        dirseen_curend(dsp,&cur) ;
	    } /* end if (dirseen-cur) */
	} /* end if (ok) */

/* final phase: try to open one of them trying them in-sequence */

	if (rs >= 0) {
	    cchar	**dv ;
	    if ((rs = vecstr_getvec(sdp,&dv)) >= 0) {
		for (i = 0 ; dv[i] != NULL ; i += 1) {
		    rs = bibleparas_indopencheck(op,dv[i]) ;
		    if ((rs >= 0) || (! isNotPresent(rs))) break ;
		} /* end for */
	    } /* end if (vecstr_getvex) */
	    if ((rs < 0) && isNotPresent(rs)) {
	        rs = bibleparas_indopenalt(op,sip,dsp) ;
	    }
	} /* end if (ok) */

	return rs ;
}
/* end subroutines (bibleparas_indopenseqer) */


static int bibleparas_loadcooks(BIBLEPARAS *op,EXPCOOK *ecp)
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
	            vp = INDCNAME ;
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
/* end subroutines (bibleparas_loadcooks) */


static int bibleparas_indopenalt(BIBLEPARAS *op,SUBINFO *sip,DIRSEEN *dsp)
{
	DIRSEEN_CUR	cur ;
	const int	elen = MAXPATHLEN ;
	int		rs ;

#if	CF_DEBUGS
	debugprintf("bibleparas_indopenalt: ent\n") ;
#endif

	if ((rs = dirseen_curbegin(dsp,&cur)) >= 0) {
	    int		el ;
	    char	ebuf[MAXPATHLEN + 1] ;
	    char	indname[MAXPATHLEN + 1] ;

	    while ((el = dirseen_enum(dsp,&cur,ebuf,elen)) >= 0) {
	        if (el > 0) {
	            if ((rs = bibleparas_indmk(op,ebuf,sip->dt)) >= 0) {
		        if ((rs = mkpath2(indname,ebuf,op->dbname)) >= 0) {
	                    rs = bpi_open(&op->vind,indname)  ;
	                    op->f.vind = (rs >= 0) ;
		        }
	            }
		}
		if ((rs >= 0) || (! isNotPresent(rs))) break ;
	    } /* end while */

	    dirseen_curend(dsp,&cur) ;
	} /* end if (cursor) */

#if	CF_DEBUGS
	debugprintf("bibleparas_indopenalt: mid rs=%d f_vind=%u\n",rs,
	    op->f.vind) ;
#endif

#if	CF_STARTAUDIT
	if ((rs >= 0) && op->f.vind) {
	    rs = bpi_audit(&op->vind) ;
	}
#endif /* CF_STARTAUDIT */

#if	CF_DEBUGS
	debugprintf("bibleparas_indopenalt: ret rs=%d n",rs) ;
#endif

	return rs ;
}
/* end subroutines (bibleparas_indopenalt) */


static int bibleparas_indopencheck(BIBLEPARAS *op,cchar dbname[])
{
	int		rs ;
	int		nverses = 0 ;

	if ((rs = bpi_open(&op->vind,dbname)) >= 0) {
	    BPI_INFO	binfo ;
	    if ((rs = bpi_info(&op->vind,&binfo)) >= 0) {
		if (binfo.ctime >= op->ti_db) {
		    op->f.vind = TRUE ;
		} else {
		    rs = SR_STALE ;
		}
	    } /* end if (bpi_info) */
	    if (rs < 0)
	        bpi_close(&op->vind) ;
	} /* end if (bpi_open) */

#if	CF_DEBUGS
	debugprintf("bibleparas_indopencheck: ret rs=%d nv=%u\n",
		rs,nverses) ;
#endif

	return (rs >= 0) ? nverses : rs ;
}
/* end subroutine (bibleparas_indopencheck) */


static int bibleparas_indmk(BIBLEPARAS *op,cchar dname[],time_t dt)
{
	const mode_t	dm = BIBLEPARAS_DIRMODE ;
	int		rs ;
	int		c = 0 ;

	if ((rs = mkdname(dname,dm)) >= 0) {
	    cchar	*db = op->dbname ;
	    char	indname[MAXPATHLEN + 1] ;
	    if ((rs = mkpath2(indname,dname,db)) >= 0) {
		const mode_t	om = BIBLEPARAS_IDXMODE ;
		if ((rs = bibleparas_indmkdata(op,indname,om)) >= 0) {
		    c += rs ;
	            op->ti_vind = dt ;
		}
	     }
	}

#if	CF_DEBUGS
	debugprintf("bibleparas_indmk: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (bibleparas_indmk) */


static int bibleparas_indmkdata(BIBLEPARAS *op,cchar indname[],mode_t om)
{
	BPIMK		bpind ;
	BPIMK_VERSE	bve ;
	const int	of = 0 ;
	int		rs ;
	int		rs1 ;
	int		c = 0 ;

	if ((rs = bpimk_open(&bpind,indname,of,om)) >= 0) {
	    c = rs ;
	    if (rs == 0) {
	        BIBLEPARAS_ENT	e ;
	        BIBLEPARAS_Q	q ;
	        uint		fileoff = 0 ;
	        int		len ;
	        int		ml = op->mapsize ;
	        int		ll ;
	        int		si ;
	        int		f_ent = FALSE ;
	        cchar		*mp = op->mapdata ;
		cchar		*lp ;
	        cchar		*tp ;
	        while ((tp = strnchr(mp,ml,'\n')) != NULL) {

	    len = ((tp + 1) - mp) ;
	    lp = mp ;
	    ll = (len - 1) ;

	    if ((ll > 0) && (! isempty(lp,ll))) {

#if	CF_DEBUGS
	debugprintf("bibleparas_indmkdata: line>%t<\n",
		lp,strnlen(lp,MIN(ll,40))) ;
#endif

	        if ((tp = strnchr(lp,ll,'#')) != NULL)
	            ll = (tp - lp) ;

	        if ((rs = isstart(lp,ll,&q,&si)) > 0) {

	            if (f_ent) {
	                c += 1 ;
	                if ((rs = bvemk_start(&bve,&e)) >= 0) {
	                    rs = bpimk_add(&bpind,&bve) ;
			    bvemk_finish(&bve) ;
		        }
			f_ent = FALSE ;
	                entry_finish(&e) ;
	            }

	            if (rs >= 0) {
	                rs = entry_start(&e,&q) ;
			if (rs >= 0) f_ent = TRUE ;
		    }

	        } /* end if (entry start of add) */

	    } /* end if (not empty) */

	    fileoff += len ;
	    ml -= len ;
	    mp += len ;

	    if (rs < 0) break ;
	} /* end while (readling lines) */

	if ((rs >= 0) && f_ent) {
	    c += 1 ;
	    if ((rs = bvemk_start(&bve,&e)) >= 0) {
	        rs = bpimk_add(&bpind,&bve) ;
		bvemk_finish(&bve) ;
	    }
	    f_ent = FALSE ;
	    entry_finish(&e) ;
	}

	if (f_ent) {
	    f_ent = FALSE ;
	    entry_finish(&e) ;
	}

#if	CF_DEBUGS
	{
	    BPIMK_INFO	bi ;
	    rs1 = bpimk_info(&bpind,&bi) ;
	    debugprintf("bibleparas_indmkdata: maxbook=%u\n",
		bi.maxbook) ;
	    debugprintf("bibleparas_indmkdata: maxchapter=%u\n",
		bi.maxchapter) ;
	    debugprintf("bibleparas_indmkdata: maxverse=%u\n",
		bi.maxverse) ;
	    debugprintf("bibleparas_indmkdata: nverses=%u\n",
		bi.nverses) ;
	    debugprintf("bibleparas_indmkdata: nzverses=%u\n",
		bi.nzverses) ;
	}
#endif /* CF_DEBUGS */

	    } /* end if (creation needed) */
	    rs1 = bpimk_close(&bpind) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (bpimk) */

#if	CF_DEBUGS
	debugprintf("bibleparas_indmkdata: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (bibleparas_indmkdata) */


static int bibleparas_indclose(BIBLEPARAS *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op->f.vind) {
	    op->f.vind = FALSE ;
	    rs1 = bpi_close(&op->vind) ;
	    if (rs >= 0) rs = rs1 ;
	}

	return rs ;
}
/* end subroutine (bibleparas_indclose) */


static int subinfo_start(SUBINFO *sip)
{
	int		rs = SR_OK ;

	memset(sip,0,sizeof(SUBINFO)) ;
	sip->dt = time(NULL) ;

	return rs ;
}
/* end subroutine (subinfo_start) */


static int subinfo_finish(SUBINFO *sip)
{
	if (sip == NULL) return SR_FAULT ;
	return SR_OK ;
}
/* end subroutine (subinfo_finish) */


static int entry_start(BIBLEPARAS_ENT *ep,BIBLEPARAS_Q *qp)
{
	int		rs = SR_OK ;

	if (ep == NULL) return SR_FAULT ;

	memset(ep,0,sizeof(BIBLEPARAS_ENT)) ;
	ep->b = qp->b ;
	ep->c = qp->c ;
	ep->v = qp->v ;

	return rs ;
}
/* end subroutine (entry_start) */


static int entry_finish(BIBLEPARAS_ENT *ep)
{

	if (ep == NULL) return SR_FAULT ;

	return SR_OK ;
}
/* end subroutine (entry_finish) */


static int bvemk_start(BPIMK_VERSE *bvep,BIBLEPARAS_ENT *ep)
{
	int		rs = SR_OK ;

	if (ep == NULL) return SR_FAULT ;

	bvep->nlines = 0 ;
	bvep->b = ep->b ;
	bvep->c = ep->c ;
	bvep->v = ep->v ;

	return rs ;
}
/* end subroutine (bvemk_start) */


static int bvemk_finish(BPIMK_VERSE *bvep)
{

	if (bvep == NULL) return SR_FAULT ;

	return SR_OK ;
}
/* end subroutine (bvemk_finish) */


static int mkdname(cchar *dname,mode_t dm)
{
	const int	nrs = SR_NOENT ;
	int		rs ;
	if ((rs = checkdname(dname)) == nrs) {
	    rs = mkdirs(dname,dm) ;
	}
	return rs ;
}
/* end subroutine (mkdname) */


static int checkdname(cchar dname[])
{
	int		rs = SR_OK ;

	if (dname[0] == '/') {
	    struct ustat	sb ;
	    if ((rs = u_stat(dname,&sb)) >= 0) {
		if (! S_ISDIR(sb.st_mode)) rs = SR_NOTDIR ;
		if (rs >= 0) {
		    rs = perm(dname,-1,-1,NULL,W_OK) ;
		}
	    }
	} else
	    rs = SR_INVALID ;

	return rs ;
}
/* end subroutine (checkdname) */


static int isempty(cchar *lp,int ll)
{
	int		cl ;
	int		f = FALSE ;

	f = f || (ll == 0) ;
	f = f || (lp[0] == '#') ;
	if ((! f) && CHAR_ISWHITE(*lp)) {
	    cchar	*cp ;
	    cl = sfskipwhite(lp,ll,&cp) ;
	    f = f || (cl == 0) ;
	    f = f || (cp[0] == '#') ;
	}

	return f ;
}
/* end subroutine (isempty) */


static int isstart(cchar *lp,int ll,BIBLEPARAS_Q *qp,int *sip)
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		sl, cl ;
	int		i, v ;
	int		si = 0 ;
	int		ch ;
	int		f = FALSE ;
	cchar		*tp, *sp, *cp ;

	sp = lp ;
	sl = ll ;
	if ((si = siskipwhite(lp,ll)) > 0) {
	    sp += si ;
	    sl -= si ;
	}

	ch = MKCHAR(sp[0]) ;
	if ((sl >= 5) && isdigitlatin(ch)) {

	for (i = 0 ; (rs >= 0) && (i < 3) ; i += 1) {

	    cp = sp ;
	    cl = sl ;
	    if ((tp = strnpbrk(sp,sl,": \t\n")) != NULL) {
		cl = (tp - sp) ;
		sl -= ((tp + 1) - sp) ;
		sp = (tp + 1) ;
	    } else {
		cl = sl ;
		sp += sl ;
		sl = 0 ;
	    }

	    if (cl == 0)
		break ;

	    si = ((cp + cl) - lp) ;
	    rs1 = cfdeci(cp,cl,&v) ;
	    if (rs1 < 0)
		break ;

	    if (v > UCHAR_MAX) rs = SR_RANGE ;

	            switch (i) {
	            case 0:
	                qp->b = (uchar) v ;
	                break ;
	            case 1:
	                qp->c = (uchar) v ;
	                break ;
	            case 2:
	                qp->v = (uchar) v ;
	                break ;
	            } /* end switch */

	} /* end for */

	f = (i == 3) ;
	if (f) {
	    si += siskipwhite(sp,sl) ;
	} else
	    si = 0 ;

	} /* end if */

	if (sip != NULL) *sip = si ;

	return (rs >= 0) ? si : rs ;
}
/* end subroutine (isstart) */


