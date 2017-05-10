/* calmgr */

/* calendar object manager */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DEBUGCUR	0		/* debug cursor operation */
#define	CF_DEBUGMALL	1		/* debug memory allocations */
#define	CF_EMPTYTERM	1		/* terminate entry on empty line */
#define	CF_SAMECITE	0		/* same entry citation? */
#define	CF_ALREADY	1		/* do not allow duplicate results */
#define	CF_MKDIRS	0		/* |mkdname()| */


/* revision history:

	- 2008-10-01, David A­D­ Morano

	This object module was originally written.


*/

/* Copyright © 2008 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	We manage a single calendar object.


*******************************************************************************/


#define	CALENT_MASTER	0


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<sys/mman.h>
#include	<stdlib.h>
#include	<string.h>
#include	<tzfile.h>		/* for TM_YEAR_BASE */

#include	<vsystem.h>
#include	<getbufsize.h>
#include	<estrings.h>
#include	<vecobj.h>
#include	<sbuf.h>
#include	<char.h>
#include	<ids.h>
#include	<vecstr.h>
#include	<tmtime.h>
#include	<getxusername.h>
#include	<fsdir.h>
#include	<ucmallreg.h>
#include	<endianstr.h>
#include	<localmisc.h>

#include	"calmgr.h"
#include	"calent.h"
#include	"cyi.h"
#include	"cyimk.h"


/* local defines */

#ifndef	COLUMNS
#define	COLUMNS		80
#endif

#define	CALMGR_IDX	struct calmgr_idx
#define	CALMGR_DBSUF	"calendar"
#define	CALMGR_MAXLINES	40		/* maximum lines per entry */
#define	CALMGR_DMODE	0777

#define	IDXDNAME	".calyears"

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

#define	CEBUFLEN	((CALMGR_MAXLINES*COLUMNS) + (3*sizeof(int)))

#ifndef	TIMEBUFLEN
#define	TIMEBUFLEN	80
#endif


/* external subroutines */

extern uint	hashelf(const char *,int) ;

extern int	snsds(char *,int,const char *,const char *) ;
extern int	snwcpy(char *,int,const char *,int) ;
extern int	sncpy1(char *,int,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	mkfnamesuf1(char *,const char *,const char *) ;
extern int	mkfnamesuf2(char *,const char *,const char *,const char *) ;
extern int	sibreak(const char *,int,const char *) ;
extern int	siskipwhite(const char *,int) ;
extern int	matstr(const char **,const char *,int) ;
extern int	matcasestr(const char **,const char *,int) ;
extern int	matocasestr(const char **,int,const char *,int) ;
extern int	matpcasestr(const char **,int,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecui(const char *,int,uint *) ;
extern int	mkdirs(const char *,mode_t) ;
extern int	perm(const char *,uid_t,gid_t,gid_t *,int) ;
extern int	sperm(IDS *,struct ustat *,int) ;
extern int	mktmpuserdir(char *,const char *,const char *,mode_t) ;
extern int	isdigitlatin(int) ;
extern int	isNotPresent(int) ;
extern int	isNotAccess(int) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;
extern char	*strnpbrk(const char *,int,const char *) ;
extern char	*timestr_log(time_t,char *) ;


/* local structures */

struct calmgr_idx {
	CYI		cy ;
	int		year ;
	int		f_open:1 ;
} ;


/* forward references */

static int	calmgr_argbegin(CALMGR *,cchar *,cchar *) ;
static int	calmgr_argend(CALMGR *) ;
static int	calmgr_dbloadbegin(CALMGR *,time_t) ;
static int	calmgr_dbloadend(CALMGR *) ;
static int	calmgr_dbmapcreate(CALMGR *,time_t) ;
static int	calmgr_dbmapdestroy(CALMGR *) ;
static int	calmgr_idxdir(CALMGR *) ;

static int	calmgr_lookyear(CALMGR *,CALMGR_Q *,CYI **) ;
static int	calmgr_lookone(CALMGR *,vecobj *,CYI *,CALMGR_Q *) ;

static int	calmgr_mkidx(CALMGR *,int) ;

static int	calmgr_idxbegin(CALMGR *,CALMGR_IDX *,int) ;
static int	calmgr_idxend(CALMGR *,CALMGR_IDX *) ;
static int	calmgr_idxends(CALMGR *) ;
static int	calmgr_idxaudit(CALMGR *,CALMGR_IDX *) ;

static int	calmgr_cyiopen(CALMGR *,CALMGR_IDX *,int) ;
static int	calmgr_cyiclose(CALMGR *,CALMGR_IDX *) ;

static int	calmgr_mkcyi(CALMGR *,int) ;

static int	calmgr_mapdata(CALMGR *,cchar **) ;

#if	CF_MKDIRS
static int	calmgr_mkdirs(CALMGR *,cchar *,mode_t) ;
#endif /* CF_MKDIRS */

static int	mkbve_start(CYIMK_ENT *,cchar *,CALENT *) ;
static int	mkbve_finish(CYIMK_ENT *) ;

static int	isempty(const char *,int) ;


/* exported variables */


/* local variables */


/* exported subroutines */


int calmgr_start(CALMGR *calp,CALYEARS *op,int cidx,cchar *dn,cchar *cn)
{
	const time_t	dt = 0 ;
	int		rs ;

	memset(calp,0,sizeof(CALMGR)) ;
	calp->calyears = op ; /* parent object */
	calp->cidx = cidx ; /* parent index */

	if ((rs = calmgr_argbegin(calp,dn,cn)) >= 0) {
	    if ((rs = calmgr_dbloadbegin(calp,dt)) >= 0) {
	        if ((rs = calmgr_idxdir(calp)) >= 0) {
	            static int	vo = VECHAND_OSTATIONARY ;
	            if ((rs = vechand_start(&calp->idxes,1,vo)) >= 0) {
	                calp->f.idxes = TRUE ;
	            }
	        } /* end if (calmgr_idxdir) */
	        if (rs < 0)
	            calmgr_dbloadend(calp) ;
	    } /* end if (calmgr_dbloadbegin) */
	    if (rs < 0)
	        calmgr_argend(calp) ;
	} /* end if (calmgr_argbegin) */

#if	CF_DEBUGS
	debugprintf("calmgr_start: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (calmgr_start) */


int calmgr_finish(CALMGR *calp)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (calp->f.idxes) {
	    rs1 = calmgr_idxends(calp) ;
	    if (rs >= 0) rs = rs1 ;
	    calp->f.idxes = FALSE ;
	    rs1 = vechand_finish(&calp->idxes) ;
	    if (rs >= 0) rs = rs1 ;
	}

	rs1 = calmgr_dbloadend(calp) ;
	if (rs >= 0) rs = rs1 ;

	if (calp->idxdname != NULL) {
	    rs1 = uc_free(calp->idxdname) ;
	    if (rs >= 0) rs = rs1 ;
	    calp->idxdname = NULL ;
	}

	rs1 = calmgr_argend(calp) ;
	if (rs >= 0) rs = rs1 ;

	return rs ;
}
/* end subroutine (calmgr_finish) */


int calmgr_lookup(CALMGR *calp,vecobj *rlp,CALMGR_Q *qp)
{
	CYI		*cyp ;
	int		rs ;
	if ((rs = calmgr_lookyear(calp,qp,&cyp)) >= 0) {
	    rs = calmgr_lookone(calp,rlp,cyp,qp) ;
	}
#if	CF_DEBUGS
	debugprintf("calmgr_lookup: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (calmgr_lookup) */


int calmgr_getci(CALMGR *calp)
{
	int		cidx = calp->cidx ;
	return cidx ;
}
/* end subroutine (calmgr_getci) */


int calmgr_getbase(CALMGR *calp,cchar **rpp)
{
	int		rs ;
	cchar		*md ;
	if (calp->mapdata != NULL) {
	     md = calp->mapdata ;
	     rs = calp->mapsize ;
	} else {
	     rs = SR_INVALID ;
	}
	*rpp = (rs >= 0) ? md : NULL ;
	return rs ;
}
/* end subroutine (calmgr_getbase) */


int calmgr_gethash(CALMGR *calp,CALENT *ep,uint *rp)
{
	int		rs = SR_OK ;
	if ((rs = calent_gethash(ep,rp)) == 0) {
	    cchar	*md = calp->mapdata ;
	    if ((rs = calent_mkhash(ep,md)) >= 0) {
		rs = calent_gethash(ep,rp) ;
	    }
	}
	return rs ;
}
/* end subroutine (calmgr_gethash) */


int calmgr_loadbuf(CALMGR *calp,char rbuf[],int rlen,CALENT *ep)
{
	int		rs ;
	cchar		*md ;

	if ((rs = calmgr_mapdata(calp,&md)) >= 0) {
	    rs = calent_loadbuf(ep,rbuf,rlen,md) ;
	}

#if	CF_DEBUGS
	debugprintf("calmgr_loadbuf: calent_loadbuf() rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (calmgr_loadbuf) */


int calmgr_audit(CALMGR *calp)
{
	CALMGR_IDX	*cip ;
	vechand		*ilp = &calp->idxes ;
	int		rs = SR_OK ;
	int		i ;
	for (i = 0 ; vechand_get(ilp,i,&cip) >= 0 ; i += 1) {
	    if (cip != NULL) {
	        rs = calmgr_idxaudit(calp,cip) ;
	    }
	    if (rs < 0) break ;
	} /* end for */
	return rs ;
}
/* end subroutine (calmgr_audit) */


/* private subroutines */


static int calmgr_argbegin(CALMGR *calp,cchar *dn,cchar *cn)
{
	int		rs ;
	int		size = 0 ;
	char		*bp ;
	size += (strlen(dn)+1) ;
	size += (strlen(cn)+1) ;
	if ((rs = uc_malloc(size,&bp)) >= 0) {
	    calp->a = bp ;
	    calp->dn = bp ;
	    bp = (strwcpy(bp,dn,-1)+1) ;
	    calp->cn = bp ;
	    bp = (strwcpy(bp,cn,-1)+1) ;
	} /* end if (m-a) */
	return rs ;
}
/* end subroutine (calmgr_argbegin) */


static int calmgr_argend(CALMGR *calp)
{
	int		rs = SR_OK ;
	int		rs1 ;
	if (calp->a != NULL) {
	    rs1 = uc_free(calp->a) ;
	    if (rs >= 0) rs = rs1 ;
	    calp->a = NULL ;
	}
	return rs ;
}
/* end subroutine (calmgr_argend) */


static int calmgr_dbloadbegin(CALMGR *calp,time_t dt)
{
	int		rs ;

	rs = calmgr_dbmapcreate(calp,dt) ;

	return rs ;
}
/* end subroutine (calmgr_dbloadbegin) */


static int calmgr_dbloadend(CALMGR *calp)
{
	int		rs = SR_OK ;
	int		rs1 ;

	rs1 = calmgr_dbmapdestroy(calp) ;
	if (rs >= 0) rs = rs1 ;

	return rs ;
}
/* end subroutine (calmgr_dbloadend) */


static int calmgr_dbmapcreate(CALMGR *calp,time_t dt)
{
	const int	nlen = MAXNAMELEN ;
	int		rs ;
	const char	*suf = CALMGR_DBSUF ;
	char		nbuf[MAXNAMELEN + 1] ;

	if ((rs = snsds(nbuf,nlen,calp->cn,suf)) >= 0) {
	    char	dbfname[MAXPATHLEN + 1] ;
	    if ((rs = mkpath2(dbfname,calp->dn,nbuf)) >= 0) {
	        if ((rs = u_open(dbfname,O_RDONLY,0666)) >= 0) {
	            struct ustat	sb ;
	            const int		fd = rs ;
	            if ((rs = u_fstat(fd,&sb)) >= 0) {
	                if (S_ISREG(sb.st_mode)) {
	                    calp->filesize = sb.st_size ;
	                    calp->ti_db = sb.st_mtime ;
	                    if (sb.st_size <= INT_MAX) {
	                        size_t	ms = (size_t) calp->filesize ;
	                        int	mp = PROT_READ ;
	                        int	mf = MAP_SHARED ;
	                        void	*n = NULL ;
	                        void	*md ;
	                        if ((rs = u_mmap(n,ms,mp,mf,fd,0L,&md)) >= 0) {
	                            if (dt == 0) dt = time(NULL) ;
	                            calp->mapdata = md ;
	                            calp->mapsize = calp->filesize ;
	                            calp->ti_map = dt ;
	                            calp->ti_lastcheck = dt ;
	                        } /* end if (u_mmap) */
	                    } else
	                        rs = SR_TOOBIG ;
	                } else
	                    rs = SR_NOTSUP ;
	            } /* end if (stat) */
	            u_close(fd) ;
	        } /* end if (file) */
	    } /* end if (mkpath) */
	} /* end if (snsds) */

	return rs ;
}
/* end subroutine (calmgr_dbmapcreate) */


static int calmgr_dbmapdestroy(CALMGR *calp)
{
	int		rs = SR_OK ;

	if (calp->mapdata != NULL) {
	    rs = u_munmap(calp->mapdata,calp->mapsize) ;
	    calp->mapdata = NULL ;
	    calp->mapsize = 0 ;
	}

	return rs ;
}
/* end subroutine (calmgr_dbmapdestroy) */


static int calmgr_idxdir(CALMGR *calp)
{
	int		rs ;
	cchar		*idc = IDXDNAME ;
	char		idxdname[MAXPATHLEN+1] ;
	if ((rs = mkpath2(idxdname,calp->dn,idc)) >= 0) {
	    cchar	*cp ;
	    if ((rs = uc_mallocstrw(idxdname,rs,&cp)) >= 0) {
	        calp->idxdname = cp ;
	    }
	}
	return rs ;
}
/* end subroutine (calmgr_idxdir) */


static int calmgr_lookyear(CALMGR *calp,CALMGR_Q *qp,CYI **cypp)
{
	CALMGR_IDX	*cip ;
	CYI		*yip = NULL ;
	vechand		*clp = &calp->idxes ;
	int		rs ;
	int		i ;
	for (i = 0 ; (rs = vechand_get(clp,i,&cip)) >= 0 ; i += 1) {
	    if ((cip != NULL) && (cip->year == qp->y)) {
	        yip = &cip->cy ;
	        break ;
	    }
	} /* end for */
	if (rs == SR_NOTFOUND) {
	    const int	y = qp->y ;
	    if ((rs = calmgr_mkidx(calp,y)) >= 0) {
	        const int	i = rs ;
	        if ((rs = vechand_get(clp,i,&cip)) >= 0) {
	            yip = &cip->cy ;
	        }
	    }
	}
	if (cypp != NULL) {
	    *cypp = (rs >= 0) ? yip : NULL ;
	}
#if	CF_DEBUGS
	debugprintf("calmgr_lookyear: ret rs=%d i=%u\n",rs,i) ;
#endif
	return (rs >= 0) ? i : rs ;
}
/* end subroutine (calmgr_lookyear) */


static int calmgr_lookone(CALMGR *calp,vecobj *rlp,CYI *cip,CALMGR_Q *qp)
{
	CYI_CUR		ccur ;
	CYI_ENT		ce ;
	const int	cidx = calp->cidx ;
	int		rs ;
	int		rs1 ;
	int		c = 0 ;

#if	CF_DEBUGS
	debugprintf("calmgr_lookone: cidx=%d\n",cidx) ;
#endif
	if ((rs = cyi_curbegin(cip,&ccur)) >= 0) {
	    if ((rs = cyi_lookcite(cip,&ccur,qp)) >= 0) {
	        CALENT		e ;
	        uint		loff ;
	        const int	celen = CEBUFLEN ;
	        int		llen ;
	        int		f_ent = FALSE ;
	        int		f_already = FALSE ;
	        char		cebuf[CEBUFLEN + 1] ;

	        while (rs >= 0) {

#if	CF_DEBUGS && CF_DEBUGCUR
	            debugprintf("calmgr_lookone: cyi_read() c=%u\n",c) ;
	            calyears_debugcur(op,rlp,"before cyi_read") ;
#endif

	            rs1 = cyi_read(cip,&ccur,&ce,cebuf,celen) ;

#if	CF_DEBUGS && CF_DEBUGCUR
	            debugprintf("calmgr_lookone: cyi_read() rs1=%d\n",
	                rs1) ;
	            calyears_debugcur(op,rlp,"after cyi_read") ;
#endif

	            if ((rs1 == SR_NOTFOUND) || (rs1 == 0)) break ;
	            rs = rs1 ;
	            if (rs < 0) break ;

	            if (rs1 > 0) {
	                int	n = 0 ;
	                int	i ;

	                for (i = 0 ; i < ce.nlines ; i += 1) {
	                    loff = ce.lines[i].loff ;
	                    llen = (int) ce.lines[i].llen ;

#if	CF_DEBUGS
	                    debugprintf("calmgr_lookone: "
	                        "i=%u loff=%u llen=%u\n",
	                        i,loff,llen) ;
#endif

	                    n += 1 ;
	                    if (! f_ent) {
	                        uint	lo = loff ;
	                        int		ll = llen ;
	                        if ((rs = calent_start(&e,qp,lo,ll)) >= 0) {
	                            f_ent = TRUE ;
	                            calent_sethash(&e,ce.hash) ;
	                            rs = calent_setidx(&e,cidx) ;
	                        }
	                    } else {
	                        rs = calent_add(&e,loff,llen) ;
	                    }
	                } /* end for */

	                if ((rs >= 0) && (n > 0) && f_ent) {
	                    CALYEARS	*op = calp->calyears ;
	                    c += 1 ;

#if	CF_ALREADY
	                    rs = calyears_already(op,rlp,&e) ;
	                    f_already = (rs > 0) ;
#endif

	                    f_ent = FALSE ;
	                    if ((rs >= 0) && (! f_already)) {
	                        rs = vecobj_add(rlp,&e) ;
	                    } else
	                        calent_finish(&e) ;
	                }

	            } /* end if */

	        } /* end while */

	        if (f_ent) {
	            f_ent = FALSE ;
	            calent_finish(&e) ;
	        }

	    } else if (rs == SR_NOTFOUND) {
	        rs = SR_OK ;
	    }
	    cyi_curend(cip,&ccur) ;
	} /* end if (cyi-cur) */

#if	CF_DEBUGS
	debugprintf("calmgr_lookone: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (calmgr_lookone) */


static int calmgr_mkidx(CALMGR *calp,int y)
{
	CALMGR_IDX	*cip ;
	const int	csize = sizeof(CALMGR_IDX) ;
	int		rs ;
#if	CF_DEBUGS
	debugprintf("calmgr_mkidx: ent y=%u\n",y) ;
	debugprintf("calmgr_mkidx: sizeof(CALMGR_IDX)=%u\n",csize) ;
#endif
	if ((rs = uc_malloc(csize,&cip)) >= 0) {
	    if ((rs = calmgr_idxbegin(calp,cip,y)) >= 0) {
	        vechand		*ilp = &calp->idxes ;
	        rs = vechand_add(ilp,cip) ;
	        if (rs < 0)
	            calmgr_idxend(calp,cip) ;
	    } /* end if (calmgr_idxbegin) */
	    if (rs < 0)
	        uc_free(cip) ;
	} /* end if (m-a) */
#if	CF_DEBUGS
	debugprintf("calmgr_mkidx: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (calmgr_mkidx) */


static int calmgr_idxbegin(CALMGR *calp,CALMGR_IDX *cip,int y)
{
	int		rs ;
#if	CF_DEBUGS
	debugprintf("calmgr_idxbegin: ent y=%u\n",y) ;
#endif
	cip->year = y ;
	rs = calmgr_cyiopen(calp,cip,y) ;
#if	CF_DEBUGS
	debugprintf("calmgr_idxbegin: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (calmgr_idxbegin) */


static int calmgr_idxend(CALMGR *calp,CALMGR_IDX *cip)
{
	int		rs = SR_OK ;
	int		rs1 ;

	rs1 = calmgr_cyiclose(calp,cip) ;
	if (rs >= 0) rs = rs1 ;

	cip->year = 0 ;
	return rs ;
}
/* end subroutine (calmgr_idxend) */


static int calmgr_cyiopen(CALMGR *calp,CALMGR_IDX *cip,int y)
{
	CYI		*cyp = &cip->cy ;
	time_t		ti_db = calp->ti_db ;
	int		rs ;
	cchar		*dn = calp->idxdname ;
	cchar		*cn = calp->cn ;

#if	CF_DEBUGS
	debugprintf("calmgr_cyiopen: ent y=%u\n",y) ;
#endif

	if ((rs = cyi_open(cyp,y,dn,cn)) >= 0) {
	    CYI_INFO	ci ;
	    int		f_open = TRUE ;
	    if ((rs = cyi_info(cyp,&ci)) >= 0) {
	        if ((ti_db > ci.ctime) || (ti_db > ci.mtime)) {
	            cyi_close(cyp) ;
	            f_open = FALSE ;
#if	CF_DEBUGS
		debugprintf("calmgr_cyiopen: make (stale)\n") ;
#endif
	            if ((rs = calmgr_mkcyi(calp,y)) >= 0) {
#if	CF_DEBUGS
		debugprintf("calmgr_cyiopen: open (stale)\n") ;
#endif
	                if ((rs = cyi_open(cyp,y,dn,cn)) >= 0) {
	                    f_open = TRUE ;
	                }
	            }
	        }
	    } /* end if (cyi_info) */
	    if ((rs < 0) && f_open)
	        cyi_close(cyp) ;
	} else if (rs == SR_NOTFOUND) {
#if	CF_DEBUGS
		debugprintf("calmgr_cyiopen: make (not-present)\n") ;
#endif
	    if ((rs = calmgr_mkcyi(calp,y)) >= 0) {
#if	CF_DEBUGS
		debugprintf("calmgr_cyiopen: open (not-present)\n") ;
#endif
	        rs = cyi_open(cyp,y,dn,cn) ;
	    }
	} /* end if (cyi_open) */


#if	CF_DEBUGS
	debugprintf("calmgr_cyiopen: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (calent_cyiopen) */


static int calmgr_cyiclose(CALMGR *calp,CALMGR_IDX *cip)
{
	CYI		*cyp = &cip->cy ;
	int		rs = SR_OK ;
	int		rs1 ;

	if (calp == NULL) return SR_FAULT ;

	rs1 = cyi_close(cyp) ;
	if (rs >= 0) rs = rs1 ;

	return rs ;
}
/* end subroutine (calmgr_cyiclose) */


static int calmgr_mkcyi(CALMGR *calp,int y)
{
	CYIMK		cyind ;
	CYIMK_ENT	bve ;
	mode_t		om = 0664 ;
	uint		mo_start = 0 ;
	int		rs ;
	int		rs1 ;
	int		of = 0 ;
	int		si ;
	int		c = 0 ;
	int		f ;
	const char	*dn = calp->idxdname ;
	const char	*cn = calp->cn ;


#if	CF_DEBUGS
	debugprintf("calmgr_mkcyi: ent y=%u\n",y) ;
#endif

#if	CF_DEBUGS && CF_DEBUGMALL
	uc_mallout(&mo_start) ;
#endif

	if ((rs = cyimk_open(&cyind,y,dn,cn,of,om)) >= 0) {
	    CALENT	e ;
	    CALCITE	q ;
	    uint	foff = 0 ;
	    const int	cidx = calp->cidx ;
	    int		ml = calp->mapsize ;
	    int		len ;
	    int		ll ;
	    int		f_ent = FALSE ;
	    cchar	*md = calp->mapdata ;
	    cchar	*mp = calp->mapdata ;
	    cchar	*lp ;
	    cchar	*tp ;

#if	CF_DEBUGS
	    debugprintf("calmgr_mkcyi: mp=%p ml=%d\n",mp,ml) ;
#endif

	    while ((tp = strnchr(mp,ml,'\n')) != NULL) {

	        len = ((tp + 1) - mp) ;
	        lp = mp ;
	        ll = (len - 1) ;

	        if (! isempty(lp,ll)) {
	            CALYEARS	*op = calp->calyears ;

#if	CF_DEBUGS
	            debugprintf("calmgr_mkcyi: line=>%t<\n",
	                lp,strnlen(lp,MIN(ll,40))) ;
	            if (ll > 40) {
	                debugprintf("calmgr_mkcyi: cont=>%t<\n",
	                    (lp+40),strnlen((lp+40),MIN((ll-40),40))) ;
		    }
#endif

	            if ((rs = calyears_havestart(op,&q,y,lp,ll)) > 0) {
	                si = rs ;

#if	CF_DEBUGS
	            debugprintf("calmgr_mkcyi: havestart() start\n") ;
#endif

	                if (f_ent) {
	                    c += 1 ;
	                    if ((rs = mkbve_start(&bve,md,&e)) >= 0) {
	                        rs = cyimk_add(&cyind,&bve) ;
	                        mkbve_finish(&bve) ;
	                    }
	                    f_ent = FALSE ;
	                    calent_finish(&e) ;
	                }

	                if (rs >= 0) {
			    uint	eoff = (foff + si) ;
			    int		elen = (ll - si) ;
	                    if ((rs = calent_start(&e,&q,eoff,elen)) >= 0) {
	                        f_ent = TRUE ;
	                        rs = calent_setidx(&e,cidx) ;
	                    }
	                }

	            } else if (rs == 0) { /* continuation */

#if	CF_DEBUGS
	            debugprintf("calmgr_mkcyi: havestart() continuation\n") ;
#endif

	                if (f_ent) {
	                    rs = calent_add(&e,foff,ll) ;
	                }

	            } else { /* bad */

#if	CF_DEBUGS
	            debugprintf("calmgr_mkcyi: havestart() error\n") ;
#endif

	                f = FALSE ;
	                f = f || (rs == SR_NOENT) || (rs == SR_NOTFOUND) ;
	                f = f || (rs == SR_ILSEQ) ;
	                f = f || (rs == SR_INVALID) ;
	                f = f || (rs == SR_NOTSUP) ;

	                if (f && f_ent) {
	                    c += 1 ;
	                    if ((rs = mkbve_start(&bve,md,&e)) >= 0) {
	                        rs = cyimk_add(&cyind,&bve) ;
	                        mkbve_finish(&bve) ;
	                    }
	                    f_ent = FALSE ;
	                    calent_finish(&e) ;
	                }

	            } /* end if (entry start of add) */

	        } else {

#if	CF_EMPTYTERM
	            if (f_ent) {
	                c += 1 ;
	                if ((rs = mkbve_start(&bve,md,&e)) >= 0) {
	                    rs = cyimk_add(&cyind,&bve) ;
	                    mkbve_finish(&bve) ;
	                }
	                f_ent = FALSE ;
	                calent_finish(&e) ;
	            }
#else
	            rs = SR_OK ;
#endif /* CF_EMPTYTERM */

	        } /* end if (not empty) */

	        foff += len ;
	        ml -= len ;
	        mp += len ;

	        if (rs < 0) break ;
	    } /* end while (readling lines) */

#if	CF_DEBUGS
	debugprintf("calmgr_mkcyi: while-out rs=%d c=%u f_ent=%u\n",
		rs,c,f_ent) ;
#endif

	    if ((rs >= 0) && f_ent) {
	        c += 1 ;
	        if ((rs = mkbve_start(&bve,md,&e)) >= 0) {
	            rs = cyimk_add(&cyind,&bve) ;
	            mkbve_finish(&bve) ;
	        }
	        f_ent = FALSE ;
	        calent_finish(&e) ;
	    }

#if	CF_DEBUGS
	debugprintf("calmgr_mkcyi: done? rs=%d c=%u f_ent=%u\n",
		rs,c,f_ent) ;
#endif

	    if (f_ent) {
	        f_ent = FALSE ;
	        calent_finish(&e) ;
	    }

#if	CF_DEBUGS
	debugprintf("calmgr_mkcyi: done? rs=%d c=%u\n",rs,c) ;
#endif

	    rs1 = cyimk_close(&cyind) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (cyimk) */

#if	CF_DEBUGS && CF_DEBUGMALL
	{
	    uint	mo_finish ;
	    uint	mo ;
	    uc_mallout(&mo_finish) ;
	    mo = (mo_finish - mo_start) ;
	    debugprintf("calmgr_mkcyi: net mallout=%u\n",mo) ;
	}
#endif

#if	CF_DEBUGS
	debugprintf("calmgr_mkcyi: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (calmgr_mkcyi) */


#if	CF_MKDIRS
static int calmgr_mkdirs(CALMGR *calp,cchar dname[],mode_t dm)
{
	int		rs ;

	dm &= S_IAMB ;
	if ((rs = mkdirs(dname,dm)) >= 0) {
	    struct ustat	sb ;
	    if ((rs = u_stat(dname,&sb)) >= 0) {
	        if (((sb.st_mode & dm) != dm)) {
	            rs = uc_minmod(dname,dm) ;
	        }
	    }
	} /* end if (mkdirs) */

	return rs ;
}
/* end subroutine (calmgr_mkdirs) */
#endif /* CF_MKDIRS */


static int calmgr_mapdata(CALMGR *calp,cchar **rpp)
{
	int		rs ;
	if (calp->mapdata != NULL) {
	    if (rpp != NULL) *rpp = (cchar *) calp->mapdata ;
	    rs = (int) calp->mapsize ;
	} else {
	    rs = SR_INVALID ;
	}
	return rs ;
}
/* end subroutine (calmgr_mapdata) */


static int calmgr_idxends(CALMGR *calp)
{
	CALMGR_IDX	*cip ;
	vechand		*ilp = &calp->idxes ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		i ;
	for (i = 0 ; vechand_get(ilp,i,&cip) >= 0 ; i += 1) {
	    if (cip != NULL) {
	        rs1 = vechand_del(ilp,i--) ; /* really optional! */
	        if (rs >= 0) rs = rs1 ;
	        rs1 = calmgr_idxend(calp,cip) ;
	        if (rs >= 0) rs = rs1 ;
	        rs1 = uc_free(cip) ;
	        if (rs >= 0) rs = rs1 ;
	    }
	} /* end for */
	return rs ;
}
/* end subroutine (calmgr_idxends) */


static int calmgr_idxaudit(CALMGR *calp,CALMGR_IDX *cip)
{
	CYI		*cyp = &cip->cy ;
	int		rs ;
	if (calp == NULL) return SR_FAULT ;
	rs = cyi_audit(cyp) ;
	return rs ;
}
/* end subroutine (calmgr_idxaudit) */


static int mkbve_start(CYIMK_ENT *bvep,cchar *md,CALENT *ep)
{
	int		rs ;
	int		nlines = 0 ;

	if (ep == NULL) return SR_FAULT ;

	if ((rs = calent_mkhash(ep,md)) >= 0) {
	    bvep->m = ep->q.m ;
	    bvep->d = ep->q.d ;
	    bvep->voff = ep->voff ;
	    bvep->vlen = ep->vlen ;
	    bvep->hash = ep->hash ;
	    bvep->lines = NULL ;
	    nlines = ep->i ;
	    if (nlines <= UCHAR_MAX) {
	        CYIMK_LINE	*lines ;
	        const int	size = (nlines + 1) * sizeof(CYIMK_LINE) ;
	        bvep->nlines = nlines ;
	        if ((rs = uc_malloc(size,&lines)) >= 0) {
	            int	i ;
	            bvep->lines = lines ;
	            for (i = 0 ; i < nlines ; i += 1) {
	                lines[i].loff = ep->lines[i].loff ;
	                lines[i].llen = ep->lines[i].llen ;
	            }
	            lines[i].loff = 0 ;
	            lines[i].llen = 0 ;
	        } /* end if (memory-allocation) */
	    } else
	        rs = SR_TOOBIG ;
	} /* end if (calent_mkhash) */

	return (rs >= 0) ? nlines : rs ;
}
/* end subroutine (mkbve_start) */


static int mkbve_finish(CYIMK_ENT *bvep)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (bvep == NULL) return SR_FAULT ;

	if (bvep->lines != NULL) {
	    rs1 = uc_free(bvep->lines) ;
	    if (rs >= 0) rs = rs1 ;
	    bvep->lines = NULL ;
	}

	return rs ;
}
/* end subroutine (mkbve_finish) */


static int isempty(cchar *lp,int ll)
{
	int		f = FALSE ;

	f = f || (ll == 0) ;
	f = f || (lp[0] == '#') ;
	if ((! f) && CHAR_ISWHITE(*lp)) {
	    int		cl ;
	    const char	*cp ;
	    cl = sfskipwhite(lp,ll,&cp) ;
	    f = ((cl == 0) || (cp[0] == '#')) ;
	}

	return f ;
}
/* end subroutine (isempty) */


