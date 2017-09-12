/* holidays */

/* access for the HOLIDAYS database */
/* version %I% last modified %G% */


#define	CF_DEBUGS	0		/* compile-time debs */
#define	CF_FIRSTHASH	0		/* perform FIRSTHASH */


/* revision history:

	= 2008-10-01, David A­D­ Morano
	This module was originally written.

*/

/* Copyright © 2008 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This object module provides an interface to the HOLIDAYS (see
	'holidays(4)') database.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>
#include	<tzfile.h>		/* for TM_YEAR_BASE */

#include	<vsystem.h>
#include	<estrings.h>
#include	<bfile.h>
#include	<vecobj.h>
#include	<strtab.h>
#include	<tmtime.h>
#include	<ids.h>
#include	<char.h>
#include	<localmisc.h>

#include	"holidays.h"


/* local defines */

#define	HOLIDAYS_DEFRECS	20
#define	HOLIDAYS_HOLSUF		"holidays"
#define	HOLIDAYS_MAXRECS	(USHORT_MAX - 2)
#define	HOLIDAYS_NSKIP		4

#ifndef	LINEBUFLEN
#ifdef	LINE_MAX
#define	LINEBUFLEN	MAX(LINE_MAX,2048)
#else
#define	LINEBUFLEN	2048
#endif
#endif

#ifndef	KEYBUFLEN
#define	KEYBUFLEN	40
#endif

#ifndef	DIGBUFLEN
#define	DIGBUFLEN	40		/* can hold int128_t in decimal */
#endif

#define	MODP2(v,n)	((v) & ((n) - 1))

#define	SUBINFO		struct subinfo
#define	SUBINFO_REC	struct subinfo_record


/* external subroutines */

extern uint	hashelf(const char *,int) ;
extern uint	hashagain(uint,int,int) ;
extern uint	nextpowtwo(uint) ;

extern int	snsds(char *,int,const char *,const char *) ;
extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	sncpy3(char *,int,const char *,const char *,const char *) ;
extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	nleadstr(const char *,const char *,int) ;
extern int	vstrcmp(char **,char **) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	ctdeci(char *,int,int) ;
extern int	perm(const char *,uid_t,gid_t,gid_t *,int) ;
extern int	sperm(IDS *,struct ustat *,int) ;
extern int	hasuc(const char *,int) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strwcpylc(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;


/* external variables */


/* exported variables */

HOLIDAYS_OBJ	holidays = {
	"holidays",
	sizeof(HOLIDAYS),
	sizeof(HOLIDAYS_CUR)
} ;


/* local structures */

struct varentry {
	uint		khash ;
	uint		ri ;
	uint		ki ;
	uint		hi ;
} ;

enum itentries {
	itentry_ri,
	itentry_info,
	itentry_nhi,
	itentry_overlast
} ;


/* local structures */

struct subinfo_record {
	uint		cite ;		/* m:d */
	uint		ki ;		/* key-string index */
	uint		vi ;		/* val-string index */
} ;

struct subinfo {
	HOLIDAYS	*op ;
	VECOBJ		recs ;
	STRTAB		kstrs ;
	STRTAB		vstrs ;
	bfile		hfile ;
	int		fsize ;
} ;


/* forward references */

static int	holidays_dbfind(HOLIDAYS *,IDS *,char *) ;
static int	holidays_dbfinder(HOLIDAYS *,IDS *,char *,const char *) ;

static int	subinfo_start(SUBINFO *,HOLIDAYS *) ;
static int	subinfo_finish(SUBINFO *) ;
static int	subinfo_procfile(SUBINFO *) ;
static int	subinfo_procyear(SUBINFO *,const char *,int) ;
static int	subinfo_procline(SUBINFO *,const char *,int) ;
static int	subinfo_proclineval(SUBINFO *,uint,const char *,int) ;
static int	subinfo_mkdata(SUBINFO *) ;
static int	subinfo_mkrt(SUBINFO *) ;
static int	subinfo_mkst(SUBINFO *) ;
static int	subinfo_mkind(SUBINFO *,cchar *,int (*)[3],int) ;

static int	getyear(time_t) ;
static int	getcite(uint *,const char *,int) ;
static int	mkcite(uint *,int,int) ;

static int	indinsert(uint (*rt)[3],int (*it)[3],int,struct varentry *) ;
static int	hashindex(uint,int) ;
static int	ismatkey(const char *,const char *,int) ;

static int	vcmprec(const void *,const void *) ;
static int	cmprec(const void *,const void *) ;


/* local variables */

static const char	*holdnames[] = {
	"etc/acct",
	"etc",
	"/etc/acct",
	NULL
} ;


/* exported subroutines */


int holidays_open(HOLIDAYS *op,cchar *pr,int year,cchar *fname)
{
	time_t		dt = time(NULL) ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		fl = -1 ;
	int		c = 0 ;
	char		tmpfname[MAXPATHLEN + 1] ;

	if (op == NULL) return SR_FAULT ;
	if (pr == NULL) return SR_FAULT ;

	if (pr[0] == '\0') return SR_INVALID ;

	if (year <= 0)
	    year = getyear(dt) ;

	if (year < 1970)
	    return SR_INVALID ;

	memset(op,0,sizeof(HOLIDAYS)) ;
	op->year = year ;
	op->pr = pr ;

	if ((fname == NULL) || (fname[0] == '\0')) {
	    IDS	id ;

	    if ((ids_load(&id)) >= 0) {
	        rs = holidays_dbfind(op,&id,tmpfname) ;
	        fl = rs ;
	        if ((rs >= 0) && (fl > 0))
	            fname = tmpfname ;
	        ids_release(&id) ;
	    } /* end if (ids) */

	} /* end if */

	if (rs >= 0) {
	    const char	*cp ;
	    if ((rs = uc_mallocstrw(fname,fl,&cp)) >= 0) {
	        SUBINFO	si ;
	        op->fname = cp ;
	        if ((rs = subinfo_start(&si,op)) >= 0) {
	            if ((rs = subinfo_procfile(&si)) >= 0) {
	                c = rs ;
	                if ((rs = subinfo_mkdata(&si)) >= 0) {
	                    op->ti_check = dt ;
	                    op->magic = HOLIDAYS_MAGIC ;
	                }
	            }
	            rs1 = subinfo_finish(&si) ;
	            if (rs >= 0) rs = rs1 ;
	        } /* end if (subinfo) */
	        if (rs < 0) {
	            uc_free(op->fname) ;
	            op->fname = NULL ;
	        }
	    } /* end if (m-a) */
	} /* end if (ok) */
	if (rs < 0) holidays_close(op) ;

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (holidays_open) */


int holidays_close(HOLIDAYS *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != HOLIDAYS_MAGIC) return SR_NOTOPEN ;

	if (op->vst != NULL) {
	    rs1 = uc_free(op->vst) ;
	    if (rs >= 0) rs = rs1 ;
	    op->vst = NULL ;
	}

	if (op->kit != NULL) {
	    rs1 = uc_free(op->kit) ;
	    if (rs >= 0) rs = rs1 ;
	    op->kit = NULL ;
	}

	if (op->kst != NULL) {
	    rs1 = uc_free(op->kst) ;
	    if (rs >= 0) rs = rs1 ;
	    op->kst = NULL ;
	}

	if (op->rt != NULL) {
	    rs1 = uc_free(op->rt) ;
	    if (rs >= 0) rs = rs1 ;
	    op->rt = NULL ;
	}

	if (op->fname != NULL) {
	    rs1 = uc_free(op->fname) ;
	    if (rs >= 0) rs = rs1 ;
	    op->fname = NULL ;
	}

	op->magic = 0 ;
	return rs ;
}
/* end subroutine (holidays_close) */


int holidays_count(HOLIDAYS *op)
{
	int		rs = SR_OK ;
	int		c ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != HOLIDAYS_MAGIC) return SR_NOTOPEN ;

	c = (op->rtlen - 1) ;
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (holidays_count) */


int holidays_audit(HOLIDAYS *op)
{
	int		rs = SR_OK ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != HOLIDAYS_MAGIC) return SR_NOTOPEN ;

	return rs ;
}
/* end subroutine (holidays_audit) */


int holidays_curbegin(HOLIDAYS *op,HOLIDAYS_CUR *curp)
{

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;

	if (op->magic != HOLIDAYS_MAGIC) return SR_NOTOPEN ;

	curp->i = 0 ;
	curp->chash = 0 ;
	op->ncursors += 1 ;

	return SR_OK ;
}
/* end subroutine (holidays_curbegin) */


int holidays_curend(HOLIDAYS *op,HOLIDAYS_CUR *curp)
{

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;

	if (op->magic != HOLIDAYS_MAGIC) return SR_NOTOPEN ;

	curp->i = 0 ;
	if (op->ncursors > 0)
	    op->ncursors -= 1 ;

	return SR_OK ;
}
/* end subroutine (holidays_curend) */


int holidays_fetchcite(op,qp,curp,vbuf,vlen)
HOLIDAYS	*op ;
HOLIDAYS_CITE	*qp ;
HOLIDAYS_CUR	*curp ;
char		vbuf[] ;
int		vlen ;
{
	HOLIDAYS_CUR	dcur ;
	uint		(*rt)[3] ;
	uint		(*rpp)[3] ;
	uint		scite ;
	const int	esize = (3 * sizeof(uint)) ;
	int		rs = SR_OK ;
	int		ri, vi ;
	int		rtlen ;
	int		vl = 0 ;
	const char	*vst ;
	const char	*vp ;

	if (op == NULL) return SR_FAULT ;
	if (qp == NULL) return SR_FAULT ;

	if (op->magic != HOLIDAYS_MAGIC) return SR_NOTOPEN ;

	if (curp == NULL) {
	    curp = &dcur ;
	    curp->i = 0 ;
	}

	if (vbuf != NULL)
	    vbuf[0] = '\0' ;

	vst = op->vst ;

	rt = op->rt ;
	rtlen = op->rtlen ;

	scite = 0 ;
	scite |= (qp->m << 8) ;
	scite |= (qp->d << 0) ;

	if (curp->i <= 0) {

	    uint	(*srt)[3] = (rt + 1) ;
	    int		srtlen = (rtlen - 1) ;
	    rpp = (uint (*)[3]) bsearch(&scite,srt,srtlen,esize,cmprec) ;
	    if (rpp == NULL)
	        rs = SR_NOTFOUND ;

	    if (rs >= 0) {
	        ri = (rpp - rt) ;
	        while (ri > 0) {
	            int	pri ;
	            pri = (ri - 1) ;
	            if (scite != rt[pri][0])
	                break ;
	            ri = pri ;
	        } /* end while */
	    }

	} else {

	    ri = (curp->i + 1) ;
	    if ((ri >= rtlen) || (scite != rt[ri][0]))
	        rs = SR_NOTFOUND ;

	} /* end if */

/* if successful, retrieve value */

	if (rs >= 0) {

	    vi = rt[ri][2] ;
	    vp = (vst + vi) ;
	    if (vbuf != NULL) {
	        rs = sncpy1(vbuf,vlen,vp) ;
	        vl = rs ;
	    } else
	        vl = strlen(vp) ;

	    if (qp != NULL) {
	        uint	cite = rt[ri][0] ;
	        qp->m = ((cite >> 8) & UCHAR_MAX) ;
	        qp->d = ((cite >> 0) & UCHAR_MAX) ;
	    }

	    if (rs >= 0)
	        curp->i = ri ;

	} /* end if (got one) */

	return (rs >= 0) ? vl : rs ;
}
/* end subroutine (holidays_fetchcite) */


int holidays_fetchname(op,kp,kl,curp,qp,vbuf,vlen)
HOLIDAYS	*op ;
const char	*kp ;
int		kl ;
HOLIDAYS_CUR	*curp ;
HOLIDAYS_CITE	*qp ;
char		vbuf[] ;
int		vlen ;
{
	HOLIDAYS_CUR	dcur ;
	uint		khash, nhash, chash ;
	uint		(*rt)[3] ;
	const int	nskip = HOLIDAYS_NSKIP ;
	int		rs = SR_OK ;
	int		ri, ki, vi, hi ;
	int		c ;
	int		(*it)[3] ;
	int		itlen ;
	int		vl = 0 ;
	int		f_mat = FALSE ;
	const char	*kst, *vst ;
	const char	*vp ;
	const char	*cp ;
	char		keybuf[KEYBUFLEN + 1] ;

	if (op == NULL) return SR_FAULT ;
	if (kp == NULL) return SR_FAULT ;

	if (op->magic != HOLIDAYS_MAGIC) return SR_NOTOPEN ;

	if (curp == NULL) {
	    curp = &dcur ;
	    curp->i = 0 ;
	}

	if (kl < 0)
	    kl = strlen(kp) ;

	if (hasuc(kp,kl)) {
	    if (kl > KEYBUFLEN) kl = KEYBUFLEN ;
	    strwcpylc(keybuf,kp,kl) ;
	    kp = keybuf ;
	}

	if (vbuf != NULL)
	    vbuf[0] = '\0' ;

	kst = op->kst ;
	vst = op->vst ;

	rt = op->rt ;
	it = op->kit ;
	itlen = op->itlen ;

	if (curp->i <= 0) {

/* unhappy or not, the index-table uses same-hash-linking! */

	    khash = hashelf(kp,kl) ;

	    nhash = khash ;
	    chash = (khash & INT_MAX) ;
	    curp->chash = chash ;	/* store "check" hash */

	    hi = hashindex(khash,itlen) ;

	    c = 0 ;
	    while ((ri = it[hi][itentry_ri]) > 0) {

	        f_mat = ((it[hi][itentry_info] & INT_MAX) == chash) ;
	        if (f_mat) {
	            ki = rt[ri][1] ;
	            cp = (kst + ki) ;
	            f_mat = (cp[0] == kp[0]) && ismatkey(cp,kp,kl) ;
	        }

	        if (f_mat)
	            break ;

	        if ((it[hi][itentry_info] & (~ INT_MAX)) == 0)
	            break ;

	        if (c >= (itlen + nskip))
	            break ;

	        nhash = hashagain(nhash,c++,nskip) ;

	        hi = hashindex(nhash,itlen) ;

	    } /* end while */

	    if ((rs >= 0) && (! f_mat))
	        rs = SR_NOTFOUND ;

	} else {

	    chash = curp->chash ;
	    hi = curp->i ;

	    if (hi < itlen) {

	        ri = it[hi][itentry_ri] ;

	        if (ri > 0) {

	            hi = it[hi][itentry_nhi] ;

	            if (hi != 0) {

	                ri = it[hi][itentry_ri] ;
	                f_mat = ((it[hi][itentry_info] & INT_MAX) == chash) ;
	                if ((ri > 0) && f_mat) {
	                    ki = rt[ri][1] ;
	                    f_mat = ismatkey((kst + ki),kp,kl) ;
	                }

	                if (! f_mat)
	                    rs = SR_NOTFOUND ;

	            } else
	                rs = SR_NOTFOUND ;

	        } else
	            rs = SR_NOTFOUND ;

	    } else
	        rs = SR_NOTFOUND ;

	} /* end if (preparation) */

/* if successful, retrieve value */

	if (rs >= 0) {

	    vi = rt[ri][2] ;
	    vp = (vst + vi) ;
	    if (vbuf != NULL) {
	        rs = sncpy1(vbuf,vlen,vp) ;
	        vl = rs ;
	    } else
	        vl = strlen(vp) ;

	    if (qp != NULL) {
	        uint	cite = rt[ri][0] ;
	        qp->m = ((cite >> 8) & UCHAR_MAX) ;
	        qp->d = ((cite >> 0) & UCHAR_MAX) ;
	    }

	    if (rs >= 0)
	        curp->i = hi ;

	} /* end if (got one) */

	return (rs >= 0) ? vl : rs ;
}
/* end subroutine (holidays_fetchname) */


int holidays_enum(op,curp,qp,vbuf,vlen)
HOLIDAYS	*op ;
HOLIDAYS_CUR	*curp ;
HOLIDAYS_CITE	*qp ;
char		vbuf[] ;
int		vlen ;
{
	uint		(*rt)[3] ;
	int		rs = SR_OK ;
	int		ri, vi ;
	int		vl = 0 ;
	const char	*vst ;
	const char	*vp ;

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;

	if (op->magic != HOLIDAYS_MAGIC) return SR_NOTOPEN ;

	if (op->ncursors == 0) return SR_INVALID ;

	if (vbuf != NULL)
	    vbuf[0] = '\0' ;

	ri = (curp->i < 1) ? 1 : (curp->i + 1) ;

/* ok, we're good to go */

	if (ri < op->rtlen) {
	    vst = op->vst ;
	    rt = op->rt ;
	    vi = rt[ri][2] ;
	    if (vi < op->vslen) {

	        vp = (vst + vi) ;
	        if (vbuf != NULL) {
	            rs = sncpy1(vbuf,vlen,vp) ;
	            vl = rs ;
	        } else
	            vl = strlen(vp) ;

	        if (qp != NULL) {
	            uint	cite = rt[ri][0] ;
	            qp->m = ((cite >> 8) & UCHAR_MAX) ;
	            qp->d = ((cite >> 0) & UCHAR_MAX) ;
	        }

	        if (rs >= 0)
	            curp->i = ri ;

	    } else
	        rs = SR_BADFMT ;
	} else 
	    rs = SR_NOTFOUND ;

	return (rs >= 0) ? vl : rs ;
}
/* end subroutine (holidays_enum) */


int holidays_check(HOLIDAYS *op,time_t dt)
{
	int		rs = SR_OK ;
	int		f_changed = FALSE ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != HOLIDAYS_MAGIC) return SR_NOTOPEN ;

	if (dt == 0) dt = time(NULL) ;

#ifdef	COMMENT
#else
	if (dt == 1) f_changed = TRUE ;
#endif

	return (rs >= 0) ? f_changed : rs ;
}
/* end subroutine (holidays_check) */


/* private subroutines */


static int holidays_dbfind(HOLIDAYS *op,IDS *idp,char *tmpfname)
{
	int		rs ;
	int		fl = 0 ;
	const char	*fsuf = HOLIDAYS_HOLSUF ;
	char		digbuf[DIGBUFLEN + 1] ;
	char		cname[MAXNAMELEN + 1] ;

	tmpfname[0] = '\0' ;
	if ((rs = ctdeci(digbuf,DIGBUFLEN,op->year)) >= 0) {
	    if ((rs = sncpy2(cname,MAXNAMELEN,fsuf,digbuf)) >= 0) {
	        rs = holidays_dbfinder(op,idp,tmpfname,cname) ;
	        fl = rs ;
	        if ((rs >= 0) && (fl == 0)) {
	            rs = holidays_dbfinder(op,idp,tmpfname,fsuf) ;
	            fl = rs ;
	        }
	    }
	} /* end if */

	return (rs >= 0) ? fl : rs ;
}
/* end subroutine (holidays_dbfind) */


static int holidays_dbfinder(HOLIDAYS *op,IDS *idp,char *tmpfname,cchar *cname)
{
	struct ustat	sb ;
	int		rs = SR_OK ;
	int		rs1 = SR_NOENT ;
	int		i ;
	int		fl = 0 ;
	const char	*hdn ;

	for (i = 0 ; holdnames[i] != NULL ; i += 1) {

	    hdn = holdnames[i] ;
	    if (hdn[0] != '/') {
	        rs = mkpath3(tmpfname,op->pr,hdn,cname) ;
	    } else {
	        rs = mkpath2(tmpfname,hdn,cname) ;
	    }
	    fl = rs ;

	    if (rs >= 0) {
	        rs1 = u_stat(tmpfname,&sb) ;
	        if (rs1 >= 0)
	            rs1 = sperm(idp,&sb,R_OK) ;
	    }

	    if (rs1 >= 0)
	        break ;

#if	CF_DEBUGS
	    debugprintf("holidays_dbfinder: perm() rs1=%d tmpfname=%s\n",
	        rs1,tmpfname) ;
#endif

	} /* end for */

	if (rs1 < 0)
	    fl = 0 ;

	return (rs >= 0) ? fl : rs ;
}
/* end subroutine (holidays_dbfinder) */


static int subinfo_start(SUBINFO *sip,HOLIDAYS *op)
{
	int		rs = SR_OK ;

	memset(sip,0,sizeof(SUBINFO)) ;

#if	CF_DEBUGS
	debugprintf("holidays/subinfo_start: fname=%s\n", op->fname) ;
#endif

	sip->op = op ;
	if ((rs = bopen(&sip->hfile,op->fname,"r",0666)) >= 0) {
	    const int	size = sizeof(SUBINFO_REC) ;
	    const int	n = HOLIDAYS_DEFRECS ;
	    if ((rs = vecobj_start(&sip->recs,size,n,0)) >= 0) {
	        struct ustat	sb ;
	        bcontrol(&sip->hfile,BC_STAT,&sb) ;
	        sip->fsize = sb.st_size ;
	        if ((rs = strtab_start(&sip->kstrs,(sip->fsize/3))) >= 0) {
	            rs = strtab_start(&sip->vstrs,sip->fsize) ;
	            if (rs < 0)
	                strtab_finish(&sip->kstrs) ;
	        }
	        if (rs < 0)
	            vecobj_finish(&sip->recs) ;
	    } /* end if (vecobj_start) */
	    if (rs < 0)
	        bclose(&sip->hfile) ;
	} /* end if (file-open) */

	return rs ;
}
/* end subroutine (subinfo_start) */


static int subinfo_finish(SUBINFO *sip)
{
	int		rs = SR_OK ;
	int		rs1 ;

	rs1 = strtab_finish(&sip->vstrs) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = strtab_finish(&sip->kstrs) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = vecobj_finish(&sip->recs) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = bclose(&sip->hfile) ;
	if (rs >= 0) rs = rs1 ;

	sip->op = NULL ;
	return rs ;
}
/* end subroutine (subinfo_finish) */


static int subinfo_procfile(SUBINFO *sip)
{
	HOLIDAYS	*op = sip->op ;
	const int	llen = LINEBUFLEN ;
	const int	maxrecs = HOLIDAYS_MAXRECS ;
	int		rs = SR_OK ;
	int		len ;
	int		c = 0 ;
	int		f_start = TRUE ;
	char		lbuf[LINEBUFLEN + 1] ;

	while ((rs = breadline(&sip->hfile,lbuf,llen)) > 0) {
	    len = rs ;

	    if (lbuf[len-1] == '\n') len -= 1 ;
	    if ((len == 0) || (lbuf[0] == '*')) continue ;

	    while (CHAR_ISWHITE(lbuf[len-1])) len -= 1 ;
	    if (len == 0) continue ;

	    if (f_start) {
	        int	year ;
	        f_start = FALSE ;
	        rs = subinfo_procyear(sip,lbuf,len) ;
	        year = rs ;

#if	CF_DEBUGS
	        debugprintf("holidays/subinfo_procfile: _procyear() rs=%d\n",
	            rs) ;
#endif

	        if ((rs >= 0) && (year != op->year))
	            rs = SR_NOMSG ;
	    } else {
	        rs = subinfo_procline(sip,lbuf,len) ;
	        c += rs ;

#if	CF_DEBUGS
	        debugprintf("holidays/subinfo_procfile: _procline() rs=%d\n",
	            rs) ;
#endif

	    }

	    if (c >= maxrecs) break ;
	    if (rs < 0) break ;
	} /* end if (reading) */

#if	CF_DEBUGS
	debugprintf("holidays/subinfo_procfile: ret rs=%d c=%u\n",
	    rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (subinfo_procfile) */


static int subinfo_procyear(SUBINFO *sip,cchar lbuf[],int llen)
{
	int		rs = SR_ILSEQ ;
	int		sl, cl ;
	int		year = SR_ILSEQ ;
	const char	*sp ;
	const char	*cp ;

	if (sip == NULL) return SR_FAULT ;

	sp = lbuf ;
	sl = llen ;
	if ((cl = nextfield(sp,sl,&cp)) > 0) {
	    rs = cfdeci(cp,cl,&year) ;
	}

	return (rs >= 0) ? year : rs ;
}
/* end subroutine (subinfo_procyear) */


static int subinfo_procline(SUBINFO *sip,cchar lbuf[],int llen)
{
	int		rs = SR_OK ;
	int		sl = llen ;
	int		cl ;
	int		c = 0 ;
	const char	*sp = lbuf ;
	const char	*cp ;

	if ((cl = nextfield(sp,sl,&cp)) > 0) {
	    uint	cite ;
	    if (getcite(&cite,cp,cl) >= 0) { /* ignore errors */
	        sl -= ((cp + cl) - sp) ;
	        sp = (cp + cl) ;
	        if ((cl = sfshrink(sp,sl,&cp)) > 0) {
	            rs = subinfo_proclineval(sip,cite,cp,cl) ;
	            c = rs ;
	        }
	    } /* end if (getcite) */
	} /* end if (got next field) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (subinfo_procline) */


static int subinfo_proclineval(SUBINFO *sip,uint cite,cchar *sp,int sl)
{
	int		rs = SR_OK ;
	int		cl ;
	int		c = 0 ;
	const char	*cp ;

	if ((cl = nextfield(sp,sl,&cp)) > 0) {
	    char	keybuf[KEYBUFLEN + 1] ;
	    if (hasuc(cp,cl)) {
	        if (cl > KEYBUFLEN) cl = KEYBUFLEN ;
	        strwcpylc(keybuf,cp,cl) ;
	        cp = keybuf ;
	    }
	    if ((rs = strtab_add(&sip->kstrs,cp,cl)) >= 0) {
	        int	ki = rs ;
	        if ((rs = strtab_add(&sip->vstrs,sp,sl)) >= 0) {
	            SUBINFO_REC	r ;
	            int		vi = rs ;
	            c += 1 ;
	            r.cite = cite ;
	            r.ki = ki ;
	            r.vi = vi ;
	            rs = vecobj_add(&sip->recs,&r) ;
	        } /* end if (strtab_add) */
	    } /* end if (strtab_add) */
	} /* end if (nextfield) */

#if	CF_DEBUGS
	debugprintf("holidays/subinfo_proclineval: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (subinfo_proclineval) */


static int subinfo_mkdata(SUBINFO *sip)
{
	HOLIDAYS	*op = sip->op ;
	int		rs ;

#if	CF_DEBUGS
	debugprintf("holidays/subinfo_mkdata: ent\n") ;
#endif

	if ((rs = subinfo_mkrt(sip)) >= 0) {
	    rs = subinfo_mkst(sip) ;
	    if ((rs < 0) && (op->rt != NULL)) {
	        uc_free(op->rt) ;
	        op->rt = NULL ;
	    }
	}

#if	CF_DEBUGS
	debugprintf("holidays/subinfo_mkdata: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (subinfo_mkdata) */


static int subinfo_mkrt(SUBINFO *sip)
{
	SUBINFO_REC	*rp ;
	HOLIDAYS	*op = sip->op ;
	uint		(*rt)[3] ;
	int		rs = SR_OK ;
	int		size ;
	int		n ;
	int		c = 0 ;

	n = vecobj_count(&sip->recs) ;

#if	CF_DEBUGS
	debugprintf("holidays/subinfo_mkrt: n=%u\n",n) ;
#endif

	if (n > 1)
	    vecobj_sort(&sip->recs,vcmprec) ;

	size = (n + 2) * 3 * sizeof(uint) ;
	if ((rs = uc_malloc(size,&rt)) >= 0) {
	    int	i ;
	    rt[c][0] = 0 ;
	    rt[c][1] = 0 ;
	    rt[c][2] = 0 ;
	    c += 1 ;
	    for (i = 0 ; vecobj_get(&sip->recs,i,&rp) >= 0 ; i += 1) {
	        if (rp == NULL) continue ;
	        rt[c][0] = rp->cite ;
	        rt[c][1] = rp->ki ;
	        rt[c][2] = rp->vi ;
	        c += 1 ;
	    } /* end for */
	    rt[c][0] = 0 ;
	    rt[c][1] = 0 ;
	    rt[c][2] = 0 ;
	    op->rt = rt ;
	    op->rtlen = c ;
	} /* end if (memory-allocation) */

#if	CF_DEBUGS
	debugprintf("holidays/subinfo_mkrt: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (subinfo_mkrt) */


static int subinfo_mkst(SUBINFO *sip)
{
	HOLIDAYS	*op = sip->op ;
	int		rs ;

#if	CF_DEBUGS
	debugprintf("holidays/subinfo_mkst: rtlen=%u\n",op->rtlen) ;
#endif

	if ((rs = strtab_strsize(&sip->kstrs)) >= 0) {
	    const int	ksize = rs ;
	    char	*kst ;
	    if ((rs = uc_malloc(ksize,&kst)) >= 0) {
		int	kisize ;
		if ((rs = strtab_strmk(&sip->kstrs,kst,ksize)) >= 0) {
		    int		(*kit)[3] ;
		    op->itlen = nextpowtwo(op->rtlen) ;
		    kisize = (op->itlen + 1) * 3 * sizeof(int) ;
		    if ((rs = uc_malloc(kisize,&kit)) >= 0) {
		        memset(kit,0,kisize) ;
			if ((rs = subinfo_mkind(sip,kst,kit,op->itlen)) >= 0) {
			    if ((rs = strtab_strsize(&sip->vstrs)) >= 0) {
				const int	vs = rs ;
				char		*vst ;
				if ((rs = uc_malloc(vs,&vst)) >= 0) {
				    STRTAB	*vsp = &sip->vstrs ;
				    if ((rs = strtab_strmk(vsp,vst,vs)) >= 0) {
					op->kst = kst ;
					op->vst = vst ;
					op->kit = kit ;
					op->kslen = ksize ;
					op->vslen = vs ;
				    }
				} /* end if (m-a) */
			    } /* end if */
			} /* end if */
			if (rs < 0)
			    uc_free(kit) ;
		    } /* end if (m-a) */
		}
	        if (rs < 0)
		    uc_free(kst) ;
	    } /* end if (m-a) */
	} /* end if */

#if	CF_DEBUGS
	debugprintf("holidays/subinfo_mkdata: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (subinfo_mkst) */


/* make an index table of the record table */
int subinfo_mkind(SUBINFO *sip,cchar kst[],int (*it)[3],int il)
{
	HOLIDAYS	*op = sip->op ;
	struct varentry	ve ;
	uint		khash ;
	uint		(*rt)[3] ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		ri, ki, hi ;
	int		rtl ;
	int		sc = 0 ;
	const char	*kp ;

	rt = op->rt ;
	rtl = op->rtlen ;

#if	CF_DEBUGS
	debugprintf("subinfo_mkind: rtl=%u\n",rtl) ;
#endif

#if	CF_FIRSTHASH
	{
	    VECOBJ		ves ;
	    const int		size = sizeof(struct varentry) ;
	    int			opts ;

	    opts = VECOBJ_OCOMPACT ;
	    if ((rs = vecobj_start(&ves,size,rtl,opts)) >= 0) {
	        int	i ;

	        for (ri = 1 ; ri < rtl ; ri += 1) {

	            ki = rt[ri][1] ;
	            kp = kst + ki ;
	            khash = hashelf(kp,-1) ;

	            hi = hashindex(khash,il) ;

	            if (it[hi][0] == 0) {
	                it[hi][0] = ri ;
	                it[hi][1] = (khash & INT_MAX) ;
	                it[hi][2] = 0 ;
	                sc += 1 ;
	            } else {
	                ve.ri = ri ;
	                ve.ki = ki ;
	                ve.khash = chash ;
	                ve.hi = hi ;
	                rs = vecobj_add(&ves,&ve) ;
	            }

	            if (rs < 0) break ;
	        } /* end for */

	        if (rs >= 0) {
	            varentry	*vep ;
	            for (i = 0 ; vecobj_get(&ves,i,&vep) >= 0 ; i += 1) {
	                sc += indinsert(rt,it,il,vep) ;
	            } /* end for */
	        }

	        vecobj_finish(&ves) ;
	    } /* end if (vecobj) */

	} /* end bloock */
#else /* CF_FIRSTHASH */

	for (ri = 1 ; ri < rtl ; ri += 1) {

	    ki = rt[ri][1] ;
	    kp = kst + ki ;

#if	CF_DEBUGS
	    debugprintf("subinfo_mkind: ri=%u k=%s\n",ri,
	        kp,strnlen(kp,20)) ;
#endif

	    khash = hashelf(kp,-1) ;

	    hi = hashindex(khash,il) ;

	    ve.ri = ri ;
	    ve.ki = ki ;
	    ve.khash = khash ;
	    ve.hi = hi ;
	    sc += indinsert(rt,it,il,&ve) ;

	} /* end for */

#endif /* CF_FIRSTHASH */

	it[il][0] = -1 ;
	it[il][1] = 0 ;
	it[il][2] = 0 ;

	if (sc < 0)
	    sc = 0 ;

#if	CF_DEBUGS
	debugprintf("subinfo_mkind: ret rs=%d sc=%u\n",rs,sc) ;
#endif

	return (rs >= 0) ? sc : rs ;
}
/* end subroutine (subinfo_mkind) */


static int getyear(time_t dt)
{
	TMTIME		tm ;
	int		rs ;
	int		year ;

	rs = tmtime_gmtime(&tm,dt) ;
	year = (tm.year + TM_YEAR_BASE) ;

	return (rs >= 0) ? year : rs ;
}
/* end subroutine (getyear) */


static int getcite(uint *citep,cchar *cp,int cl)
{
	int		rs = SR_ILSEQ ;
	const char	*tp ;

	if ((tp = strnchr(cp,cl,'/')) != NULL) {
	    int		m, d ;
	    if ((rs = cfdeci(cp,(tp - cp),&m)) >= 0) {
	        cl -= ((tp + 1) - cp) ;
	        cp = (tp + 1) ;
	        if ((rs = cfdeci(cp,cl,&d)) >= 0) {
	            rs = mkcite(citep,m,d) ;
	        } /* end if (cfdeci) */
	    } /* end if (cfdeci) */
	} /* end if (strnchr) */

	return rs ;
}
/* end subroutine (getcite) */


static int mkcite(uint *citep,int m,int d)
{
	uint		c ;
	int		rs = SR_DOM ;

	if ((m >= 0) && (m <= 12)) {
	    if ((d >= 0) && (d <= 31)) {
	        m -= 1 ; /* compliance w/ UNIX® facilities */
	        c = 0 ;
	        c |= (m << 8) ;
	        c |= d ;
	        *citep = c ;
	        rs = SR_OK ;
	    } /* end if (months) */
	} /* end if (days) */

	return rs ;
}
/* end subroutine (mkcite) */


static int indinsert(uint (*rt)[3],int (*it)[3],int il,struct varentry *vep)
{
	uint		nhash, chash ;
	uint		ri, ki ;
	uint		lhi, nhi, hi ;
	int		c = 0 ;

	hi = vep->hi ;
	nhash = vep->khash ;
	chash = (nhash & INT_MAX) ;

#if	CF_DEBUGS
	debugprintf("indinsert: ve ri=%u ki=%u khash=%08X hi=%u\n",
	    vep->ri,vep->ki,vep->khash,vep->hi) ;
	debugprintf("indinsert: il=%u loop 1\n",il) ;
#endif

/* CONSTCOND */
	while (TRUE) {

#if	CF_DEBUGS
	    debugprintf("indinsert: it%u ri=%u nhi=%u\n",
	        hi,it[hi][0],it[hi][2]) ;
#endif

	    if (it[hi][0] == 0)
	        break ;

	    ri = it[hi][0] ;
	    ki = rt[ri][1] ;
	    if (ki == vep->ki)
	        break ;

	    it[hi][1] |= (~ INT_MAX) ;
	    nhash = hashagain(nhash,c++,HOLIDAYS_NSKIP) ;

	    hi = hashindex(nhash,il) ;

#if	CF_DEBUGS
	    debugprintf("indinsert: nhash=%08X nhi=%u\n",nhash,hi) ;
#endif

	} /* end while */

	if (it[hi][0] > 0) {

#if	CF_DEBUGS
	    debugprintf("indinsert: loop 2\n") ;
#endif

	    lhi = hi ;
	    while ((nhi = it[lhi][2]) > 0) {
	        lhi = nhi ;
	    }
	    hi = hashindex((lhi + 1),il) ;

#if	CF_DEBUGS
	    debugprintf("indinsert: loop 3 lhi=%u\n",lhi) ;
#endif

	    while (it[hi][0] > 0) {
	        hi = hashindex((hi + 1),il) ;
	    }
	    it[lhi][2] = hi ;

#if	CF_DEBUGS
	    debugprintf("indinsert: loop 3 it%u ki=%u nhi=%u\n",lhi,
	        it[lhi][0],hi) ;
#endif

	} /* end if (same-key continuation) */

	it[hi][0] = vep->ri ;
	it[hi][1] = chash ;
	it[hi][2] = 0 ;

#if	CF_DEBUGS
	debugprintf("indinsert: ret hi=%u c=%u\n",hi,c) ;
#endif

	return c ;
}
/* end subroutine (indinsert) */


static int hashindex(uint i,int n)
{
	int	hi = MODP2(i,n) ;
	if (hi == 0) hi = 1 ;
	return hi ;
}
/* end subroutine (hashindex) */


static int ismatkey(cchar key[],cchar kp[],int kl)
{
	int	f = (key[0] == kp[0]) ;
	if (f) {
	    int	m = nleadstr(key,kp,kl) ;
	    f = (m == kl) && (key[m] == '\0') ;
	}
	return f ;
}
/* end subroutine (ismatkey) */


static int vcmprec(const void *v1p,const void *v2p)
{
	uint		**i1pp = (uint **) v1p ;
	uint		**i2pp = (uint **) v2p ;
	int		rc = 0 ;

	if ((i1pp != NULL) || (i2pp != NULL)) {
	    if (i1pp != NULL) {
	        if (i2pp != NULL) {
	            uint	*i1p = *i1pp ;
	            uint	*i2p = *i2pp ;
	            rc = (*i1p - *i2p) ;
	        } else
	            rc = -1 ;
	    } else
	        rc = 1 ;
	}

	return rc ;
}
/* end subroutine (vcmprec) */


static int cmprec(const void *v1p,const void *v2p)
{
	uint		*i1p = (uint *) v1p ;
	uint		*i2p = (uint *) v2p ;
	int		rc = 0 ;

	if ((i1p != NULL) || (i2p != NULL)) {
	    if (i1p != NULL) {
	        if (i2p != NULL) {
	      	    rc = (*i1p - *i2p) ;
	    	} else
	            rc = -1 ;
	    } else
	        rc = 1 ;
	}

	return rc ;
}
/* end subroutine (cmprec) */


