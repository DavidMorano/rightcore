/* babycalcs */

/* baby calculator */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */


/* revision history:

	= 1998-05-01, David A­D­ Morano
	This was created along with the DATE object.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This small module takes a date (in UNIX® 'time(2)' format) and uses it
        as a query to calculate the number of events corresponding to that date.
        Both past and future dates are possible. Extrapolations are made for
        future requests.

        Implementation note: We use a heap-sort rather than a quick-sort on the
        database (which eventually needs to be sorted) since the data is
        normally or most probably already completely sorted. This is supposed to
        give better performance!? (?)

        The database is kept in shared memory if at all possible. Developers
        should note that the 'table' member of the object is shared
        alternatively between stages of DB loading or reloading.

        Postscript note: This object allows for very robust dynamic creation and
        update of a shared-memory database. The cost for this is quite complex
        and perhaps less capability could have been tolerated for (far) less
        implementation complexity.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<sys/mman.h>
#include	<limits.h>
#include	<unistd.h>
#include	<string.h>
#include	<stdarg.h>

#include	<vsystem.h>
#include	<endian.h>
#include	<sigblock.h>
#include	<bfile.h>
#include	<estrings.h>
#include	<vecobj.h>
#include	<tmz.h>
#include	<tmtime.h>
#include	<filebuf.h>
#include	<storebuf.h>
#include	<ptma.h>
#include	<ptm.h>
#include	<localmisc.h>

#include	"babycalcs.h"
#include	"cvtdater.h"
#include	"babiesfu.h"


/* local defines */

#define	BABYCALCS_OBJNAME	"babycalcs"
#define	BABYCALCS_DBDNAME	"share/misc"
#define	BABYCALCS_DBSUF		"txt"
#define	BABYCALCS_SHMPOSTFIX	"dbc"
#define	BABYCALCS_PREFIXLEN	5
#define	BABYCALCS_POSTFIXLEN	7
#define	BABYCALCS_PERMS		0666

#ifndef	LINEBUFLEN
#ifdef	LINE_MAX
#define	LINEBUFLEN	MAX(LINE_MAX,2048)
#else
#define	LINEBUFLEN	2048
#endif
#endif

#ifndef	SHMNAMELEN
#define	SHMNAMELEN	14		/* shared-memory name length */
#endif

#ifndef	SHMPREFIXLEN
#define	SHMPREFIXLEN	8
#endif

#ifndef	SHMPOSTFIXLEN
#define	SHMPOSTFIXLEN	4
#endif

#define	HDRBUFLEN	(sizeof(BABIESFU) + MAXNAMELEN)

#ifndef	TO_WAITSHM
#define	TO_WAITSHM	20		/* seconds */
#endif

#define	TO_LASTCHECK	5		/* seconds */
#define	TO_DBWAIT	1		/* seconds */
#define	TO_DBPOLL	300		/* milliseconds */

#define	SHIFTINT	(6 * 60)	/* possible time-shift */

#ifndef	USTAT
#define	USTAT		struct ustat
#endif


/* external subroutines */

extern uint	uceil(uint,int) ;

extern int	snsds(char *,int,const char *,const char *) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	sncpy3(char *,int,const char *,const char *,const char *) ;
extern int	sncpy4(char *,int,const char *,const char *,cchar *,cchar *) ;
extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	matstr(const char **,const char *,int) ;
extern int	sfbasename(const char *,int,const char **) ;
extern int	nextfield(const char *,int,const char **) ;
extern int	strnnlen(const char *,int,int) ;
extern int	cfdecui(const char *,int,uint *) ;
extern int	msleep(uint) ;
extern int	iceil(int,int) ;
extern int	filebuf_writefill(FILEBUF *,const char *,int) ;
extern int	filebuf_writezero(FILEBUF *,int) ;
extern int	isOneOf(const int *,int) ;
extern int	isNotPresent(int) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strllen(const char *,int,int) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;

#if	CF_DEBUGS
extern char	*timestr_log(time_t,char *) ;
#endif


/* exported variables */

BABYCALCS_OBJ	babycalcs = {
	BABYCALCS_OBJNAME,
	sizeof(BABYCALCS)
} ;


/* local structures */


/* forward references */

static int	babycalcs_shmload(BABYCALCS *,mode_t) ;
static int	babycalcs_shmopen(BABYCALCS *,time_t,cchar *,mode_t) ;
static int	babycalcs_loadtxt(BABYCALCS *) ;

static int	babycalcs_mapbegin(BABYCALCS *,time_t,int) ;
static int	babycalcs_mapend(BABYCALCS *) ;

static int	babycalcs_proctxt(BABYCALCS *,vecobj *) ;
static int	babycalcs_proctxtline(BABYCALCS *,vecobj *,CVTDATER *,
			cchar *,int) ;

static int	babycalcs_shmwr(BABYCALCS *,time_t,int,mode_t) ;
static int	babycalcs_shmwrer(BABYCALCS *,time_t,int,mode_t,BABIESFU *) ;
static int	babycalcs_openshmwait(BABYCALCS *,const char *) ;
static int	babycalcs_mutexinit(BABYCALCS *) ;
static int	babycalcs_procmap(BABYCALCS *,time_t) ;
static int	babycalcs_verify(BABYCALCS *,time_t) ;

static int	babycalcs_lookshm(BABYCALCS *,time_t,time_t,uint *) ;
static int	babycalcs_lookproc(BABYCALCS *,time_t,uint *) ;
static int	babycalcs_lookinfo(BABYCALCS *,BABYCALCS_INFO *) ;
static int	babycalcs_calc(BABYCALCS *,int,time_t,uint *) ;
static int	babycalcs_dbcheck(BABYCALCS *,time_t) ;
static int	babycalcs_dbwait(BABYCALCS *,time_t,USTAT *) ;
static int	babycalcs_reloadshm(BABYCALCS *,time_t,USTAT *) ;
static int	babycalcs_reloadtxt(BABYCALCS *,time_t) ;
static int	babycalcs_shmcheck(BABYCALCS *,USTAT *) ;
static int	babycalcs_shmaccess(BABYCALCS *,time_t) ;
static int	babycalcs_shmupdate(BABYCALCS *,time_t,USTAT *,int) ;
static int	babycalcs_shmaddwrite(BABYCALCS *,int) ;
static int	babycalcs_shminfo(BABYCALCS *,BABYCALCS_INFO *) ;

static int	mkshmname(char *,const char *,int,const char *,int) ;

static int	vcmpentry(BABYCALCS_ENT **,BABYCALCS_ENT **) ;


/* local variables */

static BABYCALCS_ENT	defs[] = {
	{ 96526800, 0 },
	{ 1167627600, 47198810 },	/* from Guntmacker Institute */
	{ 0, 0 }
} ;

static const int	loadrs[] = {
	SR_NOENT,
	SR_NOTSUP,
	SR_NOSYS,
	0
} ;


/* exported subroutines */


int babycalcs_open(BABYCALCS *op,cchar *pr,cchar *dbname)
{
	const mode_t	om = BABYCALCS_PERMS ;
	int		rs ;
	const char	*cp ;

	if (op == NULL) return SR_FAULT ;
	if (pr == NULL) return SR_FAULT ;

	if (pr[0] == '\0') return SR_INVALID ;

#if	CF_DEBUGS
	debugprintf("babycalcs_open: pr=%s dbname=%s\n",pr,dbname) ;
#endif

	if ((dbname == NULL) || (dbname[0] == '\0'))
	    dbname = BABYCALCS_DBNAME ;

	memset(op,0,sizeof(BABYCALCS)) ;

	if ((rs = uc_mallocstrw(pr,-1,&cp)) >= 0) {
	    cchar	*suf = BABYCALCS_DBSUF ;
	    char	dbcomp[MAXNAMELEN + 1] ;
	    op->pr = cp ;
	    if ((rs = snsds(dbcomp,MAXNAMELEN,dbname,suf)) >= 0) {
	        cchar	*dbn = BABYCALCS_DBDNAME ;
	        char	dbfname[MAXPATHLEN + 1] ;
	        if ((rs = mkpath3(dbfname,op->pr,dbn,dbcomp)) >= 0) {
	            if ((rs = uc_mallocstrw(dbfname,-1,&cp)) >= 0) {
	                int	f ;
	                op->dbfname = cp ;
	                rs = babycalcs_shmload(op,om) ;
	                f = isOneOf(loadrs,rs) ;
	                if (f && (op->table == NULL)) {
	                    rs = babycalcs_loadtxt(op) ;
	                } /* end if */
	                if (rs >= 0) {
	                    op->magic = BABYCALCS_MAGIC ;
	                }
	                if (rs < 0) {
	                    if (op->f.txt && (op->table != NULL)) {
	                        op->f.txt = FALSE ;
	                        uc_free(op->table) ;
	                        op->table = NULL ;
	                    }
	                    if (op->dbfname != NULL) {
	                        uc_free(op->dbfname) ;
	                        op->dbfname = NULL ;
	                    }
	                }
	            } /* end if (memory-allocation) */
	        } /* end if (mkpath2) */
	    } /* end if (snsds) */
	    if (rs < 0) {
	        if (op->pr != NULL) {
	            uc_free(op->pr) ;
	            op->pr = NULL ;
	        }
	    }
	} /* end if (memory-allocation) */

#if	CF_DEBUGS
	debugprintf("babycalcs_open: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (babycalcs_open) */


int babycalcs_close(BABYCALCS *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != BABYCALCS_MAGIC) return SR_NOTOPEN ;

	rs1 = babycalcs_mapend(op) ;
	if (rs >= 0) rs = rs1 ;

	if (op->f.txt && (op->table != NULL)) {
	    op->f.txt = FALSE ;
	    rs1 = uc_free(op->table) ;
	    if (rs >= 0) rs = rs1 ;
	    op->table = NULL ;
	}

	if (op->shmname != NULL) {
	    rs1 = uc_free(op->shmname) ;
	    if (rs >= 0) rs = rs1 ;
	    op->shmname = NULL ;
	}

	if (op->dbfname != NULL) {
	    rs1 = uc_free(op->dbfname) ;
	    if (rs >= 0) rs = rs1 ;
	    op->dbfname = NULL ;
	}

	if (op->pr != NULL) {
	    rs1 = uc_free(op->pr) ;
	    if (rs >= 0) rs = rs1 ;
	    op->pr = NULL ;
	}

	op->magic = 0 ;
	return rs ;
}
/* end subroutine (babycalcs_close) */


int babycalcs_check(BABYCALCS *op,time_t dt)
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != BABYCALCS_MAGIC) return SR_NOTOPEN ;

	rs = babycalcs_dbcheck(op,dt) ;

	return rs ;
}
/* end subroutine (babycalcs_check) */


int babycalcs_lookup(BABYCALCS *op,time_t datereq,uint *rp)
{
	time_t		dt = 0 ;
	int		rs ;

	if (op == NULL) return SR_FAULT ;
	if (rp == NULL) return SR_FAULT ;

	if (op->magic != BABYCALCS_MAGIC) return SR_NOTOPEN ;

	if (datereq == 0) {
	    if (dt == 0) dt = time(NULL) ;
	    datereq = dt ;
	}

	if ((rs = babycalcs_dbcheck(op,dt)) >= 0) {
	    if (op->f.shm) {
	        rs = babycalcs_lookshm(op,dt,datereq,rp) ;
	    } else {
	        rs = babycalcs_lookproc(op,datereq,rp) ;
	    }
	} /* end if (db-check) */

	return rs ;
}
/* end subroutine (babycalcs_lookup) */


int babycalcs_info(BABYCALCS *op,BABYCALCS_INFO *bip)
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;
	if (bip == NULL) return SR_FAULT ;

	if (op->magic != BABYCALCS_MAGIC) return SR_NOTOPEN ;

	if ((rs = babycalcs_dbcheck(op,0)) >= 0) {
	    if (op->f.shm) {
	        rs = babycalcs_shminfo(op,bip) ;
	    } else {
	        memset(bip,0,sizeof(BABYCALCS_INFO)) ;
	    }
	}

	return rs ;
}
/* end subroutine (babycalcs_info) */


/* private subroutines */


static int babycalcs_shmload(BABYCALCS *op,mode_t om)
{
	int		rs = SR_OK ;
	int		cl ;
	int		c = 0 ;
	const char	*cp ;

	op->mapsize = 0 ;
	op->table = NULL ;
	if (op->pagesize == 0) op->pagesize = getpagesize() ;

	if ((cl = sfbasename(op->pr,-1,&cp)) > 0) {
	    cchar	*postfix = BABYCALCS_SHMPOSTFIX ;
	    char	shmname[MAXNAMELEN + 1] ;
	    if ((rs = mkshmname(shmname,cp,cl,postfix,-1)) >= 0) {
	        cchar	*smp ;
	        cl = rs ;
	        if ((rs = uc_mallocstrw(shmname,cl,&smp)) >= 0) {
	            const time_t	dt = time(NULL) ;
	            op->shmname = smp ;
	            if ((rs = babycalcs_shmopen(op,dt,shmname,om)) >= 0) {
	                const int	fd = rs ;

	                if (op->shmsize == 0) {
	                    rs = uc_fsize(fd) ;
	                    op->shmsize = rs ;
	                }

	                if (rs >= 0) {
	                    if ((rs = babycalcs_mapbegin(op,dt,fd)) >= 0) {
	                        c = rs ;
	                        if (op->f.needinit) {
	                            if ((rs = babycalcs_mutexinit(op)) >= 0) {
	                                u_fchmod(fd,om) ;
	                            }
	                        }
	                        if (rs >= 0) {
	                            op->f.shm = TRUE ;
	                        }
	                        if (rs < 0) {
	                            babycalcs_mapend(op) ;
	                            op->f.shm = FALSE ;
	                        }
	                    } /* end if (map) */
	                } /* end if (ok) */

	                u_close(fd) ;
	            } /* end if (shm-open) */
	            if (rs < 0) {
	                uc_free(op->shmname) ;
	                op->shmname = NULL ;
	            }
	        } /* end if (m-a) */
	    } /* end if (mkshmname) */
	} else {
	    rs = SR_INVALID ;
	} /* end if (sfbasename) */

	if (rs < 0) {
	    if (op->f.txt && (op->table != NULL)) {
	        op->f.txt = FALSE ;
	        uc_free(op->table) ;
	        op->table = NULL ;
	    }
	    if (op->shmname != NULL) {
	        uc_free(op->shmname) ;
	        op->shmname = NULL ;
	    }
	} /* end if (error) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (babycalcs_shmload) */


static int babycalcs_shmopen(BABYCALCS *op,time_t dt,cchar *shmname,mode_t om)
{
	const int	rsn = SR_NOENT ;
	int		of = O_RDWR ;
	int		rs ;
	int		fd = -1 ;

	if ((rs = uc_openshm(shmname,of,om)) == rsn) {
	    const mode_t	mom = (om & 0444) ;

	    of = (O_RDWR | O_CREAT | O_EXCL) ;
	    if ((rs = uc_openshm(shmname,of,mom)) >= 0) {
	        fd = rs ;
	        if (dt == 0) dt = time(NULL) ;
	        op->ti_lastcheck = dt ;
	        if ((rs = babycalcs_loadtxt(op)) >= 0) {
	            if ((rs = babycalcs_shmwr(op,dt,fd,om)) >= 0) {
	                op->f.needinit = TRUE ;
	            }
	        }
	        if ((rs < 0) && (fd >= 0)) {
	            u_close(fd) ;
	            fd = -1 ;
	        }
	    } /* end if (uc_openshm) */

	    if ((rs == SR_ACCESS) || (rs == SR_EXIST)) {
	        op->shmsize = 0 ;
	        rs = babycalcs_openshmwait(op,shmname) ;
	        fd = rs ;
	    }

	} else {
	    fd = rs ;
	}

	return (rs >= 0) ? fd : rs ;
}
/* end subroutine (babycalcs_shmopen) */


static int babycalcs_mapbegin(BABYCALCS *op,time_t dt,int fd)
{
	const size_t	msize = op->shmsize ;
	const int	mprot = PROT_READ | PROT_WRITE ;
	const int	mflags = MAP_SHARED ;
	int		rs ;
	int		c = 0 ;
	void		*md ;

	if (fd < 0) return SR_INVALID ;

	if (dt == 0) dt = time(NULL) ;

#if	CF_DEBUGS
	debugprintf("babycalcs_mapbegin: ent ms=%lu\n",msize) ;
#endif

	if ((rs = u_mmap(NULL,msize,mprot,mflags,fd,0L,&md)) >= 0) {
	    op->mapdata = md ;
	    op->mapsize = msize ;
	    op->ti_map = dt ;
	    if (op->f.txt && (op->table != NULL)) {
	        op->f.txt = FALSE ;
	        uc_free(op->table) ;
	        op->table = NULL ;
	    }
	    rs = babycalcs_procmap(op,dt) ;
	    c = rs ;
	    if (rs < 0) {
	        op->table = NULL ;
	        op->f.shm = FALSE ;
	        u_munmap(op->mapdata,op->mapsize) ;
	        op->mapdata = NULL ;
	        op->mapsize = 0 ;
	        op->ti_map = 0 ;
	    }
	} /* end if (map) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (babycalcs_mapbegin) */


static int babycalcs_mapend(BABYCALCS *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op->mapdata != NULL) {
	    caddr_t	mdata = op->mapdata ;
	    size_t	msize = op->mapsize ;
	    rs1 = u_munmap(mdata,msize) ;
	    if (rs >= 0) rs = rs1 ;
	    op->mapdata = NULL ;
	    op->mapsize = 0 ;
	    op->mp = NULL ;
	    op->ti_map = 0 ;
	    if (op->f.shm && (op->table != NULL)) {
	        op->f.shm = FALSE ;
	        op->table = NULL ;
	    }
	}

	return rs ;
}
/* end subroutine (babycalcs_mapend) */


static int babycalcs_procmap(BABYCALCS *op,time_t dt)
{
	BABIESFU	*hfp = &op->hf ;
	int		rs ;
	int		c = 0 ;

#if	CF_DEBUGS
	debugprintf("babycalcs_procmap: ent\n") ;
#endif

	if (dt == 0) dt = time(NULL) ;

	if ((rs = babiesfu(hfp,1,op->mapdata,op->mapsize)) >= 0) {
	    if ((rs = babycalcs_verify(op,dt)) >= 0) {
	        op->table = (BABYCALCS_ENT *) (op->mapdata + hfp->btoff) ;
	        op->mp = (PTM *) (op->mapdata + hfp->muoff) ;
	        op->nentries = hfp->btlen ;
	        c = hfp->btlen ;
	    }
	} /* end if (babiesfu) */

#if	CF_DEBUGS
	debugprintf("babycalcs_procmap: mp(%p)\n",op->mp) ;
	debugprintf("babycalcs_procmap: nentries=%u\n",op->nentries) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (babycalcs_procmap) */


static int babycalcs_loadtxt(BABYCALCS *op)
{
	vecobj		ents ;
	int		rs ;
	int		rs1 ;
	int		size ;
	int		c = 0 ;

	op->table = NULL ;
	op->nentries = 0 ;
	size = sizeof(BABYCALCS_ENT) ;
	if ((rs = vecobj_start(&ents,size,0,0)) >= 0) {
	    if ((rs = babycalcs_proctxt(op,&ents)) >= 0) {
		int	n = rs ;
	        int	i ;
	        if ((rs == SR_NOENT) || (n == 0)) {
	            for (i = 0 ; defs[i].date > 0 ; i += 1) {
	                rs = vecobj_add(&ents,(defs + i)) ;
	                if (rs < 0) break ;
	            } /* end for */
	        }
	        if (rs >= 0) {
	            void	*p ;
	            n = vecobj_count(&ents) ;
	            size = (n + 1) * sizeof(BABYCALCS_ENT) ;
	            if ((rs = uc_malloc(size,&p)) >= 0) {
	                BABYCALCS_ENT	*ep ;
	                op->table = p ;
	                for (i = 0 ; vecobj_get(&ents,i,&ep) >= 0 ; i += 1) {
	                    if (ep != NULL) {
	                        op->table[c++] = *ep ;
			    }
	                } /* end for */
	                op->table[c].date = 0 ;
	                op->table[c].count = 0 ;
	                op->nentries = c ;
	                op->f.txt = TRUE ;
	            } else {
	                op->table = NULL ;
		    }
	        } /* end if (ok) */
	    } /* end if (babycalcs_proctxt) */
	    rs1 = vecobj_finish(&ents) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (vecobj-entries) */

#if	CF_DEBUGS
	debugprintf("babycalcs_loadtxt: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (babycalcs_loadtxt) */


static int babycalcs_proctxt(BABYCALCS *op,vecobj *tlp)
{
	CVTDATER	cdater ;
	int		rs ;
	int		rs1 ;
	int		c = 0 ;

	if (tlp == NULL) return SR_FAULT ;

	if (op->dbfname == NULL) return SR_BUGCHECK ;

#if	CF_DEBUGS
	debugprintf("babycalcs_proctxt: dbfname=%s\n",op->dbfname) ;
#endif

	op->f.sorted = TRUE ;
	if ((rs = cvtdater_start(&cdater,0)) >= 0) {
	    USTAT	sb ;
	    bfile	txtfile, *tfp = &txtfile ;
	    const int	llen = LINEBUFLEN ;
	    int		len ;
	    int		ll ;
	    cchar	*tp, *lp ;
	    char	lbuf[LINEBUFLEN + 1] ;

#if	CF_DEBUGS
	    debugprintf("babycalcs_proctxt: cvtdater_start() rs=%d\n",rs) ;
#endif

	    if ((rs = bopen(tfp,op->dbfname,"r",0666)) >= 0) {

	        if ((rs = bcontrol(&txtfile,BC_STAT,&sb)) >= 0) {

	            op->ti_mdb = sb.st_mtime ;
	            op->dbsize = sb.st_size ;
	            while ((rs = breadline(&txtfile,lbuf,llen)) > 0) {
	                len = rs ;

	                if (lbuf[len - 1] == '\n') len -= 1 ;

	                lp = lbuf ;
	                ll = len ;
	                if ((tp = strnchr(lp,ll,'#')) != NULL)
	                    ll = (tp - lbuf) ;

	                if ((ll == 0) || (lp[0] == '#')) continue ;

	                c += 1 ;
	                rs = babycalcs_proctxtline(op,tlp,&cdater,lp,ll) ;

#if	CF_DEBUGS
	                debugprintf("babycalcs_proctxt: "
	                    "babycalcs_proctxtline() rs=%d\n",
	                    rs) ;
#endif

	                if (rs < 0) break ;
	            } /* end while */

	        } /* end if (bcontrol) */

	        rs1 = bclose(&txtfile) ;
		if (rs >= 0) rs = rs1 ;
	    } /* end if (file-open) */

	    if ((rs >= 0) && (! op->f.sorted)) {
	        op->f.sorted = TRUE ;
	        if (c > 1)
	            vecobj_sort(tlp,vcmpentry) ;
	    }

	    rs1 = cvtdater_finish(&cdater) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (cvtdater) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (babycalcs_proctxt) */


static int babycalcs_proctxtline(BABYCALCS *op,vecobj *tlp,CVTDATER *cdp,
		cchar *lbuf,int llen)
{
	int		rs = SR_OK ;
	int		cl ;
	int		c = 0 ;
	const char	*cp ;

	if ((cl = nextfield(lbuf,llen,&cp)) > 0) {
	    time_t	datereq ;
	    if ((rs = cvtdater_load(cdp,&datereq,cp,cl)) >= 0) {
	        int	ll = llen ;
	        cchar	*lp = lbuf ;
	        ll -= ((cp + cl) - lp) ;
	        lp = (cp + cl) ;
	        if ((cl = nextfield(lp,ll,&cp)) > 0) {
	            uint		count ;
	            if ((rs = cfdecui(cp,cl,&count)) >= 0) {
	                BABYCALCS_ENT	e, *ep ;
	                int		ei ;

	                c = 1 ;
	                e.date = datereq ;
	                e.count = count ;
	                if ((rs = vecobj_add(tlp,&e)) > 0) {
	                    ei = (rs - 1) ;
	                    if ((rs = vecobj_get(tlp,ei,&ep)) >= 0) {
	                        if ((ep == NULL) || (e.date < ep->date)) {
	                            op->f.sorted = FALSE ;
	                        }
	                    } else if (rs == SR_NOTFOUND) {
	                        rs = SR_OK ;
	                    }
	                } /* end if (vecobj_add) */

	            } /* end if (cfdec) */
	        } /* end if (nextfield) */
	    } /* end if (cvtdater_load) */
	} /* end if (nextfield) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (babycalcs_proctxtline) */


static int babycalcs_shmwr(BABYCALCS *op,time_t dt,int fd,mode_t om)
{
	BABIESFU	hf ;
	int		rs ;
	int		foff = 0 ;

	op->shmsize = 0 ;
	if (dt == 0) dt = time(NULL) ;

	if (op->pagesize == 0) op->pagesize = getpagesize() ;

/* prepare the file-header */

	memset(&hf,0,sizeof(BABIESFU)) ;
	hf.vetu[0] = BABIESFU_VERSION ;
	hf.vetu[1] = ENDIAN ;
	hf.vetu[2] = 0 ;
	hf.vetu[3] = 0 ;
	hf.dbsize = (uint) op->dbsize ;
	hf.dbtime = (uint) op->ti_mdb ;
	hf.wtime = (uint) dt ;

/* process */

	if ((rs = babycalcs_shmwrer(op,dt,fd,om,&hf)) >= 0) {
	    foff = rs ;
	    if ((rs = u_rewind(fd)) >= 0) {
		const int	hlen = HDRBUFLEN ;
		char		hbuf[HDRBUFLEN+1] ;

	        if ((rs = babiesfu(&hf,0,hbuf,hlen)) >= 0) {
	            if ((rs = u_write(fd,hbuf,rs)) >= 0) {
	                op->shmsize = foff ;
	                rs = u_fchmod(fd,om) ;
		    }
	        }

	    } /* end if (u_rewind) */
	} /* end if (babycalcs_shmwrer) */

	return (rs >= 0) ? foff : rs ;
}
/* end subroutine (babycalcs_shmwr) */


static int babycalcs_shmwrer(BABYCALCS *op,time_t dt,int fd,mode_t om,
		BABIESFU *hfp)
{
	FILEBUF		babyfile ;
	const int	bsize = op->pagesize ;
	int		rs ;
	int		rs1 ;
	int		foff = 0 ;
	if ((rs = filebuf_start(&babyfile,fd,0,bsize,0)) >= 0) {
	    const int	hlen = HDRBUFLEN ;
	    int		bl ;
	    char	hbuf[HDRBUFLEN + 1] ;

	    if ((rs = babiesfu(hfp,0,hbuf,hlen)) >= 0) {
		int	tsize ;
	        bl = rs ;

/* write file-header */

	        if (rs >= 0) {
	            rs = filebuf_writefill(&babyfile,hbuf,bl) ;
	            foff += rs ;
	        }

/* write the mutex (align up to the next 8-byte boundary) */

	        if (rs >= 0) {
	            int		noff = iceil(foff,8) ;
	            if (noff != foff) {
	                rs = filebuf_writezero(&babyfile,(noff - foff)) ;
	                foff += rs ;
	            }
	        }

	        hfp->muoff = foff ;
	        hfp->musize = uceil(sizeof(PTM),sizeof(uint)) ;
	        if (rs >= 0) {
	            rs = filebuf_writezero(&babyfile,hfp->musize) ;
	            foff += rs ;
	        }

/* write the table */

	        hfp->btoff = foff ;
	        hfp->btlen = op->nentries ;
	        tsize = (op->nentries + 1) * sizeof(BABYCALCS_ENT) ;
	        if (rs >= 0) {
	            rs = filebuf_write(&babyfile,op->table,tsize) ;
	            foff += rs ;
	        }

	        hfp->shmsize = foff ;
	    } /* end if (babiesfu) */

	    rs1 = filebuf_finish(&babyfile) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (filebuf) */
	return (rs >= 0) ? foff : rs ;
}
/* end subroutine (babycalcs_shmwrer) */


static int babycalcs_mutexinit(BABYCALCS *op)
{
	BABIESFU	*hfp = &op->hf ;
	PTM		*mp ;
	PTMA		ma ;
	int		rs ;

#if	CF_DEBUGS
	debugprintf("babycalcs_mutexinit: muoff=%u\n",hfp->muoff) ;
#endif

	mp = (PTM *) (op->mapdata + hfp->muoff) ;

#if	CF_DEBUGS
	debugprintf("babycalcs_mutexinit: mp(%p)\n",op->mp) ;
#endif

	memset(mp,0,sizeof(PTM)) ;

	if ((rs = ptma_create(&ma)) >= 0) {
	    const int	cmd = PTHREAD_PROCESS_SHARED ;
	    if ((rs = ptma_setpshared(&ma,cmd)) >= 0) {
	        rs = ptm_create(mp,&ma) ;
	    }
	    ptma_destroy(&ma) ;
	} /* end if (mutex-lock attribute) */

	return rs ;
}
/* end subroutine (babycalcs_mutexinit) */


static int babycalcs_openshmwait(BABYCALCS *op,cchar *shmname)
{
	const int	om = BABYCALCS_PERMS ;
	int		rs = SR_OK ;
	int		oflags = O_RDWR ;
	int		to = TO_WAITSHM ;
	int		fd = -1 ;

	if (op == NULL) return SR_FAULT ;

	while (to-- > 0) {
	    rs = uc_openshm(shmname,oflags,om) ;
	    fd = rs ;
	    if (rs >= 0) break ;
	    if (rs != SR_ACCESS) break ;
	    msleep(1) ;
	} /* end while */

	if ((rs < 0) && (to == 0)) {
	    rs = SR_TIMEDOUT ;
	}

	return (rs >= 0) ? fd : rs ;
}
/* end subroutine (babycalcs_openshmwait) */


static int babycalcs_verify(BABYCALCS *op,time_t dt)
{
	BABIESFU	*hfp = &op->hf ;
	uint		utime = (uint) dt ;
	int		rs = SR_OK ;
	int		size ;
	int		f = TRUE ;

	f = f && (hfp->shmsize == op->mapsize) ;

#if	CF_DEBUGS
	debugprintf("babycalcs_verify: mapsize=%u hf.shmsize=%u f=%u\n",
	    op->mapsize,hfp->shmsize,f) ;
#endif

	if (hfp->wtime > 0)
	    f = f && (hfp->wtime <= (utime + SHIFTINT)) ;

#if	CF_DEBUGS
	{
	    char	timebuf1[TIMEBUFLEN + 1] ;
	    char	timebuf2[TIMEBUFLEN + 1] ;
	    debugprintf("babycalcs_verify: wtime=%s utime=%s f=%u\n",
	        timestr_log(((time_t) hfp->wtime),timebuf1),
	        timestr_log(((time_t) utime),timebuf2), f) ;
	}
#endif /* CF_DEBUGS */

	f = f && (hfp->muoff <= op->mapsize) ;

#if	CF_DEBUGS
	debugprintf("babycalcs_verify: mid3 f=%u\n",f) ;
#endif

	size = hfp->musize ;
	if (size > 0) {
	    f = f && ((hfp->muoff + size) <= hfp->btoff) ;
#if	CF_DEBUGS
	debugprintf("babycalcs_verify: mid4 f=%u\n",f) ;
#endif
	    f = f && ((hfp->muoff + size) <= op->mapsize) ;
#if	CF_DEBUGS
	debugprintf("babycalcs_verify: mid5 f=%u\n",f) ;
#endif
	}

	f = f && (hfp->btoff <= op->mapsize) ;

#if	CF_DEBUGS
	debugprintf("babycalcs_verify: mid6 f=%u\n",f) ;
#endif

	size = hfp->btlen * sizeof(BABYCALCS_ENT) ;
	f = f && ((hfp->btoff + size) <= op->mapsize) ;

#if	CF_DEBUGS
	debugprintf("babycalcs_verify: mid7 f=%u\n",f) ;
#endif

/* get out */

	if (! f)
	    rs = SR_BADFMT ;

#if	CF_DEBUGS
	debugprintf("babycalcs_verify: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (babycalcs_verify) */


static int babycalcs_lookshm(BABYCALCS *op,time_t dt,time_t datereq,uint *rp)
{
	sigset_t	oldsigmask, newsigmask ;
	int		rs ;

	if (op->mapdata == NULL) return SR_BUGCHECK ;
	if (op->mp == NULL) return SR_BUGCHECK ;

#if	CF_DEBUGS
	debugprintf("babycalcs_lookshm: ent\n") ;
#endif

	uc_sigsetfill(&newsigmask) ;

	if ((rs = u_sigprocmask(SIG_BLOCK,&newsigmask,&oldsigmask)) >= 0) {

	    if ((rs = ptm_lock(op->mp)) >= 0) {

	        if ((rs = babycalcs_shmaccess(op,dt)) >= 0) {
	            rs = babycalcs_lookproc(op,datereq,rp) ;
		}

	        ptm_unlock(op->mp) ;
	    } /* end if (mutex lock) */

	    u_sigprocmask(SIG_SETMASK,&oldsigmask,NULL) ;
	} /* end if (u_sigprocmask) */

	return rs ;
}
/* end subroutine (babycalcs_lookshm) */


static int babycalcs_lookproc(BABYCALCS *op,time_t datereq,uint *rp)
{
	int		rs = SR_OK ;
	int		i ;

	if (datereq == 0)
	    datereq = time(NULL) ;

#if	CF_DEBUGS
	debugprintf("babycalcs_lookproc: nentries=%u\n",op->nentries) ;
#endif

	for (i = 0 ; i < op->nentries ; i += 1) {
	    if (datereq <= op->table[i].date) break ;
	} /* end for */

	if ((i > 0) && (i >= op->nentries))
	    i -= 1 ;

#if	CF_DEBUGS
	debugprintf("babycalcs_lookproc: i=%u \n",i) ;
	debugprintf("babycalcs_lookproc: c=%u\n",op->table[i].count) ;
#endif

	babycalcs_calc(op,i,datereq,rp) ;

#if	CF_DEBUGS
	debugprintf("babycalcs_lookproc: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (babycalcs_lookproc) */


static int babycalcs_calc(BABYCALCS *op,int i,time_t rd,uint *rp)
{
	time_t		bd ;
	double		x0, x1, dx ;
	double		y0, y1, dy ;
	double		xr, yr, yb ;
	uint		bc ;
	int		rs = SR_OK ;

	bd = (i > 0) ? op->table[i-1].date : 0 ;
	bc = (i > 0) ? op->table[i-1].count : 0 ;

	x0 = bd ;
	x1 = op->table[i].date ;
	dx = (x1 - x0) ;

	y0 = bc ;
	y1 = op->table[i].count ;
	dy = (y1 - y0) ;

	yb = bc ;
	xr = (rd - bd) ;
	yr = (xr * dy / dx) + yb ;

	*rp = yr ;
	return rs ;
}
/* end subroutine (babycalcs_calc) */


static int babycalcs_dbcheck(BABYCALCS *op,time_t dt)
{
	int		rs = SR_OK ;
	int		tint ;
	int		f = FALSE ;

#if	CF_DEBUGS
	debugprintf("babycalcs_dbcheck: ent\n") ;
#endif

	if (dt == 0)
	    dt = time(NULL) ;

	tint = (dt - op->ti_lastcheck) ;
	if (tint >= TO_LASTCHECK) {
	    USTAT	sb ;
	    op->ti_lastcheck = dt ;
	    if ((rs = u_stat(op->dbfname,&sb)) >= 0) {
	        if (op->f.shm) {
	            f = (sb.st_mtime > op->hf.dbtime) ;
	            f = f || (sb.st_size != op->hf.dbsize) ;
	            if (f) {
	                if ((rs = babycalcs_dbwait(op,dt,&sb)) >= 0) {
	                    rs = babycalcs_reloadshm(op,dt,&sb) ;
			}
	            }
	        } else {
	            f = (sb.st_mtime > op->ti_mdb) ;
	            f = f || (sb.st_size != op->dbsize) ;
	            if (f) {
	                if ((rs = babycalcs_dbwait(op,dt,&sb)) >= 0) {
	                    rs = babycalcs_reloadtxt(op,dt) ;
			}
	            }
	        }
	    } else if (isNotPresent(rs)) {
	        rs = SR_OK ;
	    } /* end if (stat) */
	} /* end if (time-out) */

#if	CF_DEBUGS
	debugprintf("babycalcs_dbcheck: ret rs=%d f=%u\n",rs,f) ;
#endif

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (babycalcs_dbcheck) */


static int babycalcs_shminfo(BABYCALCS *op,BABYCALCS_INFO *bip)
{
	sigset_t	oldsigmask, newsigmask ;
	int		rs ;
	int		rs1 ;
	int		rv = 0 ;

#if	CF_DEBUGS
	debugprintf("babycalcs_shminfo: ent\n") ;
#endif

	if (op->mapdata == NULL) return SR_BUGCHECK ;
	if (op->mp == NULL) return SR_BUGCHECK ;

	uc_sigsetfill(&newsigmask) ;

	if ((rs = u_sigprocmask(SIG_BLOCK,&newsigmask,&oldsigmask)) >= 0) {
	    if ((rs = ptm_lock(op->mp)) >= 0) {
		{
	            rs = babycalcs_lookinfo(op,bip) ;
		    rv = rs ;
		}
	        rs1 = ptm_unlock(op->mp) ;
		if (rs >= 0) rs = rs1 ;
	    } /* end if (mutex lock) */
	    rs1 = u_sigprocmask(SIG_SETMASK,&oldsigmask,NULL) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (procmask) */

	return (rs >= 0) ? rv : rs ;
}
/* end subroutine (babycalcs_shminfo) */


static int babycalcs_lookinfo(BABYCALCS *op,BABYCALCS_INFO *bip)
{
	uint		*hwp ;
	int		rs = SR_OK ;

	memset(bip,0,sizeof(BABYCALCS_INFO)) ;

	hwp = (uint *) (op->mapdata + BABIESFU_IDLEN) ;
	bip->wtime = hwp[babiesfuh_wtime] ;
	bip->atime = hwp[babiesfuh_atime] ;
	bip->acount = hwp[babiesfuh_acount] ;
	return rs ;
}
/* end subroutine (babycalcs_lookinfo) */


static int babycalcs_dbwait(BABYCALCS *op,time_t dt,USTAT *sbp)
{
	USTAT		nsb ;
	int		rs = SR_OK ;
	int		f ;

	f = ((dt - sbp->st_mtime) >= TO_DBWAIT) ;
	if (! f) {
	    while (rs >= 0) {

	        msleep(TO_DBPOLL) ;

	        if ((rs = u_stat(op->dbfname,&nsb)) >= 0) {

	            f = (sbp->st_size == nsb.st_size) ;
	            f = f && (sbp->st_mtime == nsb.st_mtime) ;
	            f = f && ((dt - nsb.st_mtime) >= TO_DBWAIT) ;
	            if (f)
	                break ;

	            *sbp = nsb ;
	            dt = time(NULL) ;

	        } /* end if */

	    } /* end while */
	} /* end if (needed) */

	return rs ;
}
/* end subroutine (babycalcs_dbwait) */


static int babycalcs_reloadshm(BABYCALCS *op,time_t dt,USTAT *sbp)
{
	const mode_t	om = BABYCALCS_PERMS ;
	const int	oflags = O_RDWR ;
	int		rs ;
	int		rs1 ;
	int		c = 0 ;

#if	CF_DEBUGS
	debugprintf("babycalcs_reloadshm: ent\n") ;
#endif

	if ((rs = uc_openshm(op->shmname,oflags,om)) >= 0) {
	    SIGBLOCK	sb ;
	    const int	fd = rs ;
	    int		neo = op->nentries ;
	    int		mapsize = op->mapsize ;
	    int		mapextent ;
	    int		f = FALSE ;

	    if ((rs = sigblock_start(&sb,NULL)) >= 0) {

	        if ((rs = ptm_lock(op->mp)) >= 0) {

#if	CF_DEBUGS
	            debugprintf("babycalcs_reloadshm: _shmcheck()\n") ;
#endif

	            rs = babycalcs_shmcheck(op,sbp) ;
	            f = (rs > 0) ;
	            if ((rs >= 0) && f) {

	                rs = babycalcs_shmupdate(op,dt,sbp,fd) ;

#if	CF_DEBUGS
	                debugprintf("babycalcs_reloadshm: _shmupdate() rs=%d\n",
	                    rs) ;
	                debugprintf("babycalcs_reloadshm: nentries=%d eno=%u\n",
	                    op->nentries,neo) ;
#endif

	            }

	            rs1 = ptm_unlock(op->mp) ;
	            if (rs >= 0) rs = rs1 ;
	        } /* end if (mutex lock) */

	        rs1 = sigblock_finish(&sb) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (sigblock) */

	    if ((rs >= 0) && f) {
	        uc_msync(op->mapdata,op->mapsize,MS_ASYNC) ;
	    }

	    c = op->nentries ;
	    if ((rs >= 0) && f && (c != neo)) {

	        mapextent = uceil(mapsize,op->pagesize) ;

	        if (op->shmsize > mapextent) {

#if	CF_DEBUGS
	            debugprintf("babycalcs_reloadshm: remap\n") ;
#endif

	            babycalcs_mapend(op) ;

	            rs = babycalcs_mapbegin(op,dt,fd) ;
	            c = rs ;

	        } else {

	            op->mapsize = op->shmsize ;

	        } /* end if (SHM-segment exceeded the last page) */

	    } /* end if */

	    u_close(fd) ;
	} /* end if (open) */

#if	CF_DEBUGS
	debugprintf("babycalcs_reloadshm: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (babycalcs_reloadshm) */


static int babycalcs_shmupdate(BABYCALCS *op,time_t dt,USTAT *sbp,int fd)
{
	BABYCALCS_ENT	*tblp = op->table ;
	int		rs ;

#if	CF_DEBUGS
	debugprintf("babycalcs_shmupdate: ent\n") ;
#endif

	if ((rs = babycalcs_loadtxt(op)) >= 0) {
	    uint	*hwp ;
	    const int	es = sizeof(BABYCALCS_ENT) ;
	    int		nen = op->nentries ;
	    int		neo = op->nentries ;
	    int		tblsize ;
	    int		shmsize = 0 ;
	    int		f ;

	    f = (nen != neo) ;
	    if (f) {
	        tblsize = (nen * es) ;
	        f = (memcmp(tblp,op->table,tblsize) != 0) ;
	    }
	    if (f) {

	        rs = babycalcs_shmaddwrite(op,fd) ;
	        shmsize = rs ;
	        if (rs >= 0) {

#if	CF_DEBUGS
	            debugprintf("babycalcs_shmupdate: new shmsize=%u\n",shmsize) ;
#endif

	            op->shmsize = shmsize ;
	            hwp = (uint *) (op->mapdata + BABIESFU_IDLEN) ;
	            hwp[babiesfuh_shmsize] = shmsize ;
	            hwp[babiesfuh_dbsize] = (uint) sbp->st_size ;
	            hwp[babiesfuh_dbtime] = (uint) sbp->st_mtime ;
	            hwp[babiesfuh_wtime] = (uint) dt ;
	            hwp[babiesfuh_btlen] = op->nentries ;

	            op->hf.shmsize = hwp[babiesfuh_shmsize] ;
	            op->hf.dbsize = hwp[babiesfuh_dbsize] ;
	            op->hf.dbtime = hwp[babiesfuh_dbtime] ;
	            op->hf.wtime = hwp[babiesfuh_wtime] ;
	            op->hf.btlen = hwp[babiesfuh_btlen] ;

	        } /* end if */

	    } /* end if (update needed) */

	    if (op->table != NULL) {
	        op->f.txt = FALSE ;
	        uc_free(op->table) ;
	        op->table = NULL ;
	    }

	} /* end if (babycalcs_loadtxt) */

	op->table = tblp ;

#if	CF_DEBUGS
	debugprintf("babycalcs_shmupdate: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (babycalcs_shmupdate) */


static int babycalcs_shmaddwrite(BABYCALCS *op,int fd)
{
	offset_t	tbloff ;
	uint		*hwp ;
	const int	es = sizeof(BABYCALCS_ENT) ;
	int		rs ;
	int		tblsize ;
	int		shmsize = 0 ;

#if	CF_DEBUGS
	debugprintf("babycalcs_shmaddwrite: nentries=%u\n",op->nentries) ;
#endif

	hwp = (uint *) (op->mapdata + BABIESFU_IDLEN) ;
	tbloff = hwp[babiesfuh_btoff] ;
	tblsize = ((op->nentries + 1) * es) ;

#if	CF_DEBUGS
	debugprintf("babycalcs_shmaddwrite: tbloff=%llu\n",tbloff) ;
#endif

	shmsize = tbloff ;
	if ((rs = u_seek(fd,tbloff,SEEK_SET)) >= 0) {
	    if ((rs = u_write(fd,op->table,tblsize)) >= 0) {
	        shmsize += rs ;
	        rs = uc_ftruncate(fd,shmsize) ;
	    }
	}

	return (rs >= 0) ? shmsize : rs ;
}
/* end subroutine (babycalcs_shmaddwrite) */


/* ARGSUSED */
static int babycalcs_reloadtxt(BABYCALCS *op,time_t dt)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op->f.txt && (op->table != NULL)) {
	    op->f.txt = FALSE ;
	    rs1 = uc_free(op->table) ;
	    if (rs >= 0) rs = rs1 ;
	    op->table = NULL ;
	}

	if (rs >= 0) {
	    rs = babycalcs_loadtxt(op) ;
	}

	return rs ;
}
/* end subroutine (babycalcs_reloadtxt) */


static int babycalcs_shmcheck(BABYCALCS *op,USTAT *sbp)
{
	uint		*hwp ;
	int		rs = SR_OK ;
	int		f = FALSE ;

	hwp = (uint *) (op->mapdata + BABIESFU_IDLEN) ;
	f = (sbp->st_mtime > hwp[babiesfuh_dbtime]) ;
	f = f || (sbp->st_size != hwp[babiesfuh_dbsize]) ;
	f = f || (op->shmsize != hwp[babiesfuh_shmsize]) ;
	return (rs >= 0) ? f : rs ;
}
/* end subroutine (babycalcs_shmcheck) */


static int babycalcs_shmaccess(BABYCALCS *op,time_t dt)
{
	uint		*hwp ;
	int		rs = SR_OK ;

	if (op->mapdata == NULL)
	    return SR_NOANODE ;

	if (dt == 0) dt = time(NULL) ;

	hwp = (uint *) (op->mapdata + BABIESFU_IDLEN) ;
	hwp[babiesfuh_atime] = dt ;
	hwp[babiesfuh_acount] += 1 ;
	return rs ;
}
/* end subroutine (babycalcs_shmaccess) */


static int mkshmname(char *shmbuf,cchar *fp,int fl,cchar *dp,int dl)
{
	const int	shmlen = SHMNAMELEN ;
	int		rs = SR_OK ;
	int		ml ;
	int		i = 0 ;

	if (rs >= 0) {
	    rs = storebuf_char(shmbuf,shmlen,i,'/') ;
	    i += rs ;
	}

	if (rs >= 0) {
	    if (fp[0] == '/') {
	        if (fl < 0) fl = strlen(fp) ;
	        fp += 1 ;
	        fl -= 1 ;
	    }
	    ml = strnnlen(fp,fl,SHMPREFIXLEN) ;
	    rs = storebuf_strw(shmbuf,shmlen,i,fp,ml) ;
	    i += rs ;
	}

	if (rs >= 0) {
	    rs = storebuf_char(shmbuf,shmlen,i,'$') ;
	    i += rs ;
	}

	if (rs >= 0) {
	    ml = strnnlen(dp,dl,SHMPOSTFIXLEN) ;
	    rs = storebuf_strw(shmbuf,shmlen,i,dp,ml) ;
	    i += rs ;
	}

	return (rs >= 0) ? i : rs ;
}
/* end subroutine (mkshmname) */


static int vcmpentry(BABYCALCS_ENT **e1pp,BABYCALCS_ENT **e2pp)
{
	int		rc = 0 ;
	if ((*e1pp != NULL) || (*e2pp != NULL)) {
	    if (*e1pp != NULL) {
	        if (*e2pp != NULL) {
	            BABYCALCS_ENT	*e1p = *e1pp ;
	            BABYCALCS_ENT	*e2p = *e2pp ;
	            rc = (e1p->date - e2p->date) ;
	        } else
	            rc = +1 ;
	    } else
	        rc = -1 ;
	}
	return rc ;
}
/* end subroutine (vcmpentry) */


