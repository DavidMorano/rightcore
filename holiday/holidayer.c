/* holidayer */

/* access for the HOLIDAYER database */
/* version %I% last modified %G% */


#define	CF_DEBUGS	0		/* compile-time debs */


/* revision history:

	= 2016-06-02, David A­D­ Morano
	This module was originally written.

*/

/* Copyright © 2016 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This object module provides an interface to the HOLIDAYS (see
	'holidays(4)') database.  The "database" consists of all 'holidays(4)'
	database files found on the system (in the places that are searched).

	This was an El Cheapo sort of implementation since we just depend
	upon the older HOLIDAYS object to access individual 'holidays(4)'
	database files.


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
#include	<char.h>
#include	<storebuf.h>
#include	<ids.h>
#include	<vecobj.h>
#include	<bfile.h>
#include	<tmtime.h>
#include	<localmisc.h>

#include	"holidayer.h"


/* local defines */

#define	HOLIDAYER_HOL		struct holidayer_hol
#define	HOLIDAYER_FPREFIX	"holidays"

#ifndef	LINEBUFLEN
#ifdef	LINE_MAX
#define	LINEBUFLEN	MAX(LINE_MAX,2048)
#else
#define	LINEBUFLEN	2048
#endif
#endif

#ifndef	DIGBUFLEN
#define	DIGBUFLEN	40		/* can hold int128_t in decimal */
#endif


/* external subroutines */

extern int	snsds(char *,int,const char *,const char *) ;
extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	sncpy3(char *,int,const char *,const char *,const char *) ;
extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	ctdeci(char *,int,int) ;
extern int	ctdecpi(char *,int,int,int) ;
extern int	ctdecpui(char *,int,int,uint) ;
extern int	perm(const char *,uid_t,gid_t,gid_t *,int) ;
extern int	sperm(IDS *,struct ustat *,int) ;
extern int	hasuc(const char *,int) ;
extern int	isNotPresent(int) ;
extern int	isNotAccess(int) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strwcpylc(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;


/* external variables */


/* exported variables */

HOLIDAYER_OBJ	holidayer = {
	"holidayer",
	sizeof(HOLIDAYER),
	sizeof(HOLIDAYER_CUR)
} ;


/* local structures */

struct holidayer_hol {
	HOLIDAYS	hol ;
	uint		year ;
} ;


/* local structures */


/* forward references */

static int holidayer_holbegin(HOLIDAYER *,HOLIDAYER_HOL *,int,cchar *) ;
static int holidayer_holend(HOLIDAYER *,HOLIDAYER_HOL *) ;
static int holidayer_holfins(HOLIDAYER *) ;
static int holidayer_holaudit(HOLIDAYER *,HOLIDAYER_HOL *) ;

static int holidayer_yearfind(HOLIDAYER *,uint,HOLIDAYS **) ;
static int holidayer_yearfinder(HOLIDAYER *,int,HOLIDAYER_HOL **) ;
static int holidayer_yearfile(HOLIDAYER *,char *,uint) ;
static int holidayer_yearadd(HOLIDAYER *,HOLIDAYER_HOL *) ;
static int holidayer_dirok(HOLIDAYER *,cchar *) ;
static int holidayer_mkdir(HOLIDAYER *,char *,cchar *) ;
static int holidayer_mkfname(HOLIDAYER *,char *,cchar *,uint) ;
static int holidayer_holaudit(HOLIDAYER *,HOLIDAYER_HOL *) ;
static int holidayer_yearq(HOLIDAYER *,HOLIDAYER_CITE *) ;
static int holidayer_year(HOLIDAYER *,uint) ;
static int holidayer_yearmk(HOLIDAYER *) ;

static int	isOurMode(mode_t) ;


/* local variables */

static const char	*holdnames[] = {
	"etc/acct",
	"/etc/acct",
	NULL
} ;


/* exported subroutines */


int holidayer_open(HOLIDAYER *op,cchar *pr)
{
	int		rs ;
	cchar		*cp ;

	if (op == NULL) return SR_FAULT ;
	if (pr == NULL) return SR_FAULT ;

	if (pr[0] == '\0') return SR_INVALID ;

	memset(op,0,sizeof(HOLIDAYER)) ;

	if ((rs = uc_mallocstrw(pr,-1,&cp)) >= 0) {
	    op->pr = cp ;
	    if ((rs = ids_load(&op->id)) >= 0) {
	        op->magic = HOLIDAYER_MAGIC ;
	    }
	}

	return rs ;
}
/* end subroutine (holidayer_open) */


int holidayer_close(HOLIDAYER *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != HOLIDAYER_MAGIC) return SR_NOTOPEN ;

#if	CF_DEBUGS
	debugprintf("holidayer_close: ent\n") ;
#endif

	if (op->f.hols) {
	    rs1 = holidayer_holfins(op) ;
	    if (rs >= 0) rs = rs1 ;
	    op->f.hols = FALSE ;
	    rs1 = vechand_finish(&op->hols) ;
	    if (rs >= 0) rs = rs1 ;
	}

	rs1 = ids_release(&op->id) ;
	if (rs >= 0) rs = rs1 ;

	if (op->pr != NULL) {
	    rs1 = uc_free(op->pr) ;
	    if (rs >= 0) rs = rs1 ;
	    op->pr = NULL ;
	}

#if	CF_DEBUGS
	debugprintf("holidayer_close: ret rs=%d\n",rs) ;
#endif

	op->magic = 0 ;
	return rs ;
}
/* end subroutine (holidayer_close) */


int holidayer_audit(HOLIDAYER *op)
{
	int		rs = SR_OK ;
	int		c = 0 ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != HOLIDAYER_MAGIC) return SR_NOTOPEN ;

	if (op->f.hols) {
	    vechand	*hlp = &op->hols ;
	    if ((rs = vechand_audit(hlp)) >= 0) {
	        HOLIDAYER_HOL	*hep ;
	        int		i ;
	        for (i = 0 ; vechand_get(hlp,i,&hep) >= 0 ; i += 1) {
	            if (hep != NULL) {
			c += 1 ;
	                rs = holidayer_holaudit(op,hep) ;
	            }
	            if (rs < 0) break ;
	        } /* end for */
	    } /* end if (vechand_audit) */
	} /* end if */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (holidayer_audit) */


int holidayer_curbegin(HOLIDAYER *op,HOLIDAYER_CUR *curp)
{

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;

	if (op->magic != HOLIDAYER_MAGIC) return SR_NOTOPEN ;

	memset(curp,0,sizeof(HOLIDAYER_CUR)) ;
	curp->magic = HOLIDAYER_MAGIC ;

	op->ncursors += 1 ;
	return SR_OK ;
}
/* end subroutine (holidayer_curbegin) */


int holidayer_curend(HOLIDAYER *op,HOLIDAYER_CUR *curp)
{
	int		rs = SR_OK ;

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;

	if (op->magic != HOLIDAYER_MAGIC) return SR_NOTOPEN ;
	if (curp->magic != HOLIDAYER_MAGIC) return SR_NOTOPEN ;

	if (op->ncursors > 0) {
	    if (curp->hop != NULL) {
	        HOLIDAYS	*hop = curp->hop ;
	        HOLIDAYS_CUR	*hcp = &curp->hcur ;
	        if ((rs = holidays_curend(hop,hcp)) >= 0) {
	            op->ncursors -= 1 ;
	            curp->hop = NULL ;
	            curp->year = 0 ;
	            curp->magic = 0 ;
	        }
	    } /* end if (cursor was used) */
	} else
	    rs = SR_PROTO ;

	return rs ;
}
/* end subroutine (holidayer_curend) */


int holidayer_fetchcite(HOLIDAYER *op,HOLIDAYER_CITE *qp,
		HOLIDAYER_CUR *curp,char rp[],int rl)
{
	int		rs ;
	int		vl = 0 ;

	if (op == NULL) return SR_FAULT ;
	if (qp == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;

	if (op->magic != HOLIDAYER_MAGIC) return SR_NOTOPEN ;
	if (curp->magic != HOLIDAYER_MAGIC) return SR_NOTOPEN ;

#if	CF_DEBUGS
	debugprintf("holidayer_fetchcite: ent %u:%u:%u\n",
	    qp->y,qp->m,qp->d) ;
#endif

	if ((rs = holidayer_yearq(op,qp)) >= 0) {
	    const uint	y = rs ;
	    if (curp->year == 0) {
	        if (curp->hop == NULL) {
	            HOLIDAYS		*holp ;
	            HOLIDAYS_CUR	*hcp = &curp->hcur ;
	            if ((rs = holidayer_yearfind(op,y,&holp)) >= 0) {
	                if ((rs = holidays_curbegin(holp,hcp)) >= 0) {
	                    curp->hop = holp ;
	                    curp->year = y ;
	                    rs = holidays_fetchcite(holp,qp,hcp,rp,rl) ;
	                    vl = rs ;
	                } /* end if (holidays_curbegin) */
	            } /* end if (holidayer_yearfind) */
	        } else
	            rs = SR_PROTO ;
	    } else if (curp->year == qp->y) {
	        if (curp->hop != NULL) {
	            HOLIDAYS		*holp = curp->hop ;
	            HOLIDAYS_CUR	*hcp = &curp->hcur ;
	            rs = holidays_fetchcite(holp,qp,hcp,rp,rl) ;
	            vl = rs ;
	        } else
	            rs = SR_PROTO ;
	    } else
	        rs = SR_PROTO ;
	} /* end if (holidayer_year) */

#if	CF_DEBUGS
	debugprintf("holidayer_fetchcite: ret rs=%d bl=%u\n",rs,vl) ;
#endif

	return (rs >= 0) ? vl : rs ;
}
/* end subroutine (holidayer_fetchcite) */


int holidayer_fetchname(op,y,kp,kl,curp,qp,rp,rl)
HOLIDAYER	*op ;
uint		y ;
const char	*kp ;
int		kl ;
HOLIDAYER_CUR	*curp ;
HOLIDAYER_CITE	*qp ;
char		rp[] ;
int		rl ;
{
	int		rs ;
	int		vl = 0 ;

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;
	if (qp == NULL) return SR_FAULT ;

	if (op->magic != HOLIDAYER_MAGIC) return SR_NOTOPEN ;

#if	CF_DEBUGS
	debugprintf("holidayer_fetchname: ent y=%d k=>%t<\n",y,kp,kl) ;
	debugprintf("holidayer_fetchname: cur y=%u hop{%p}\n",
	    curp->year,curp->hop) ;
#endif

	if ((rs = holidayer_year(op,y)) >= 0) {
	    const uint	y = rs ;
	    if (curp->year == 0) {
	        if (curp->hop == NULL) {
	            HOLIDAYS		*holp ;
	            HOLIDAYS_CUR	*hcp = &curp->hcur ;
	            if ((rs = holidayer_yearfind(op,y,&holp)) >= 0) {
	                if ((rs = holidays_curbegin(holp,hcp)) >= 0) {
	                    curp->hop = holp ;
	                    curp->year = y ;
	                    rs = holidays_fetchname(holp,kp,kl,hcp,qp,rp,rl) ;
	                    vl = rs ;
	                } /* end if (holidays_curbegin) */
	            } /* end if (holidayer_yearfind) */
	        } else
	            rs = SR_PROTO ;
	    } else if (curp->year == y) {
	        if (curp->hop != NULL) {
	            HOLIDAYS		*holp = curp->hop ;
	            HOLIDAYS_CUR	*hcp = &curp->hcur ;
	            rs = holidays_fetchname(holp,kp,kl,hcp,qp,rp,rl) ;
	            vl = rs ;
	        } else
	            rs = SR_PROTO ;
	    } else
	        rs = SR_PROTO ;
	} /* end if (holidayer_year) */

#if	CF_DEBUGS
	debugprintf("holidayer_fetchname: ret rs=%d bl=%u\n",rs,vl) ;
#endif

	return (rs >= 0) ? vl : rs ;
}
/* end subroutine (holidayer_fetchname) */


int holidayer_enum(op,curp,qp,vbuf,vlen,y)
HOLIDAYER	*op ;
HOLIDAYER_CUR	*curp ;
HOLIDAYER_CITE	*qp ;
char		vbuf[] ;
int		vlen ;
uint		y ;
{
	int		rs = SR_OK ;
	int		vl = 0 ;

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;
	if (qp == NULL) return SR_FAULT ;

	if (op->magic != HOLIDAYER_MAGIC) return SR_NOTOPEN ;
	if (curp->magic != HOLIDAYER_MAGIC) return SR_NOTOPEN ;

	if (op->ncursors == 0) return SR_INVALID ;

	if (vbuf != NULL) vbuf[0] = '\0' ;

#if	CF_DEBUGS
	debugprintf("holidayer_enum: ent y=%u\n",y) ;
#endif

	if ((rs = holidayer_year(op,y)) >= 0) {
	    const uint	y = rs ;
	    if (curp->year == 0) {
	        if (curp->hop == NULL) {
	            HOLIDAYS		*holp ;
	            HOLIDAYS_CUR	*hcp = &curp->hcur ;
	            if ((rs = holidayer_yearfind(op,y,&holp)) >= 0) {
	                if ((rs = holidays_curbegin(holp,hcp)) >= 0) {
	                    curp->hop = holp ;
	                    curp->year = y ;
	                    rs = holidays_enum(holp,hcp,qp,vbuf,vlen) ;
	                    vl = rs ;
	                } /* end if (holidays_curbegin) */
	            } /* end if (holidayer_yearfind) */
	        } else
	            rs = SR_PROTO ;
	    } else if (curp->year == y) {
	        if (curp->hop != NULL) {
	            HOLIDAYS		*holp = curp->hop ;
	            HOLIDAYS_CUR	*hcp = &curp->hcur ;
	            rs = holidays_enum(holp,hcp,qp,vbuf,vlen) ;
	            vl = rs ;
	        } else
	            rs = SR_PROTO ;
	    } else
	        rs = SR_PROTO ;
	} /* end if (holidayer_year) */

#if	CF_DEBUGS
	debugprintf("holidayer_enum: ret rs=%d vl=%u\n",rs,vl) ;
#endif

	return (rs >= 0) ? vl : rs ;
}
/* end subroutine (holidayer_enum) */


int holidayer_check(HOLIDAYER *op,time_t dt)
{
	int		rs = SR_OK ;
	int		f_changed = FALSE ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != HOLIDAYER_MAGIC) return SR_NOTOPEN ;

	if (dt == 0) dt = time(NULL) ;

#ifdef	COMMENT
#else
	if (dt == 1) f_changed = TRUE ;
#endif

	return (rs >= 0) ? f_changed : rs ;
}
/* end subroutine (holidayer_check) */


/* private subroutines */


static int holidayer_holfins(HOLIDAYER *op)
{
	int		rs = SR_OK ;
	int		rs1 ;
#if	CF_DEBUGS
	debugprintf("holidayer_holfins: hols=%u\n",op->f.hols) ;
#endif
	if (op->f.hols) {
	    HOLIDAYER_HOL	*hep ;
	    vechand		*hlp = &op->hols ;
	    int			i ;
	    for (i = 0 ; vechand_get(hlp,i,&hep) >= 0 ; i += 1) {
	        if (hep != NULL) {
	            rs1 = holidayer_holend(op,hep) ;
	            if (rs >= 0) rs = rs1 ;
	            rs1 = uc_free(hep) ;
	            if (rs >= 0) rs = rs1 ;
	        }
	    } /* end for */
	} /* end if (activated) */
	return rs ;
}
/* end subroutine (holidayer_holfins) */


static int holidayer_holbegin(HOLIDAYER *op,HOLIDAYER_HOL *hep,int y,cchar *fn)
{
	int		rs ;
	if (op == NULL) return SR_FAULT ;
	if ((rs = holidays_open(&hep->hol,op->pr,y,fn)) >= 0) {
	    hep->year = y ;
	}
	return rs ;
}
/* end subroutine (holidayer_holbegin) */


static int holidayer_holend(HOLIDAYER *op,HOLIDAYER_HOL *hep)
{
	int		rs = SR_OK ;
	int		rs1 ;
	if (op == NULL) return SR_FAULT ;
#if	CF_DEBUGS
	debugprintf("holidayer_holend: ent y=%u\n",hep->year) ;
#endif
	rs1 = holidays_close(&hep->hol) ;
	if (rs >= 0) rs = rs1 ;
	hep->year = 0 ;
#if	CF_DEBUGS
	debugprintf("holidayer_holend: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (holidayer_holend) */


static int holidayer_yearfind(HOLIDAYER *op,uint y,HOLIDAYS **rpp)
{
	HOLIDAYER_HOL	*hep ;
	int		rs = SR_NOTFOUND ;
#if	CF_DEBUGS
	debugprintf("holidayer_yearfind: ent y=%d\n",y) ;
#endif
	if (op->f.hols) {
	    vechand	*hlp = &op->hols ;
	    int		i ;
	    for (i = 0 ; (rs = vechand_get(hlp,i,&hep)) >= 0 ; i += 1) {
	        if (hep != NULL) {
	            if (hep->year == y) break ;
	        }
	    } /* end for */
	} /* end if (hols) */
#if	CF_DEBUGS
	debugprintf("holidayer_yearfind: mid rs=%d\n",rs) ;
#endif
	if (rs == SR_NOTFOUND) {
	    if ((rs = holidayer_yearfinder(op,y,&hep)) >= 0) {
	        if (rpp != NULL) *rpp = &hep->hol ;
	    }
	} else {
	    if (rpp != NULL) *rpp = &hep->hol ;
	}
#if	CF_DEBUGS
	debugprintf("holidayer_yearfind: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (holidayer_yearfind) */


static int holidayer_yearfinder(HOLIDAYER *op,int y,HOLIDAYER_HOL **rpp)
{
	int		rs ;
	char		hfname[MAXPATHLEN+1] ;
#if	CF_DEBUGS
	debugprintf("holidayer_yearfinder: ent y=%d\n",y) ;
#endif
	if ((rs = holidayer_yearfile(op,hfname,y)) >= 0) {
	    HOLIDAYER_HOL	*hep ;
	    const int		esize = sizeof(HOLIDAYER_HOL) ;
	    if ((rs = uc_malloc(esize,&hep)) >= 0) {
	        if ((rs = holidayer_holbegin(op,hep,y,hfname)) >= 0) {
	            if ((rs = holidayer_yearadd(op,hep)) >= 0) {
	                if (rpp != NULL) *rpp = hep ;
	            }
	            if (rs < 0)
	                holidayer_holend(op,hep) ;
	        }
	        if (rs < 0)
	            uc_free(hep) ;
	    } /* end if (m-a) */
	} /* end if (holidayer_yearfile) */
#if	CF_DEBUGS
	debugprintf("holidayer_yearfinder: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (holidayer_yearfinder) */


static int holidayer_yearfile(HOLIDAYER *op,char *hfname,uint y)
{
	int		rs = SR_OK ;
	int		i ;
	int		len = 0 ;
#if	CF_DEBUGS
	debugprintf("holidayer_yearfile: ent y=%u\n",y) ;
#endif
	for (i = 0 ; holdnames[i] != NULL ; i += 1) {
	    cchar	*dn = holdnames[i] ;
	    if ((rs = holidayer_dirok(op,dn)) > 0) {
	        if ((rs = holidayer_mkfname(op,hfname,dn,y)) > 0) {
	            struct ustat	sb ;
	            const int		pl = rs ;
	            if ((rs = uc_stat(hfname,&sb)) >= 0) {
	                if (isOurMode(sb.st_mode)) {
	                    const int	am = (R_OK) ;
	                    if ((rs = sperm(&op->id,&sb,am)) >= 0) {
	                        len = pl ;
	                    } else if (isNotAccess(rs)) {
	                        rs = SR_OK ;
	                    }
	                }
	            } else if (isNotPresent(rs)) {
	                rs = SR_OK ;
	            }
	        } /* end if (holidayer_mkfname) */
	    } /* end if (holidayer_dirok) */
	    if (rs > 0) break ;
	    if (rs < 0) break ;
	} /* end for */
#if	CF_DEBUGS
	debugprintf("holidayer_yearfile: ret rs=%d len=%u\n",rs,len) ;
#endif
	return (rs >= 0) ? len : rs ;
}
/* end subroutine (holidayer_yearfile) */


static int holidayer_yearadd(HOLIDAYER *op,HOLIDAYER_HOL *hep)
{
	vechand		*hlp = &op->hols ;
	int		rs = SR_OK ;
	if (! op->f.hols) {
	    const int	vo = VECHAND_OSTATIONARY ;
	    if ((rs = vechand_start(hlp,2,vo)) >= 0) {
	        op->f.hols = TRUE ;
	    }
	}
	if (rs >= 0) {
	    rs = vechand_add(hlp,hep) ;
	}
	return rs ;
}
/* end subroutine (holidayer_yearadd) */


static int holidayer_dirok(HOLIDAYER *op,cchar *dn)
{
	int		rs ;
	int		f = FALSE ;
	char		dbuf[MAXPATHLEN+1] ;
	if ((rs = holidayer_mkdir(op,dbuf,dn)) >= 0) {
	    struct ustat	sb ;
	    if ((rs = uc_stat(dbuf,&sb)) >= 0) {
	        if (S_ISDIR(sb.st_mode)) {
	            const int	am = (R_OK|X_OK) ;
	            if ((rs = sperm(&op->id,&sb,am)) >= 0) {
	                f = TRUE ;
	            }
	        }
	    } else if (isNotPresent(rs)) {
	        rs = SR_OK ;
	    }
	} /* end if (holidayer_mkdir) */
	return (rs >= 0) ? f : rs ;
}
/* end subroutine (holidayer_dirok) */


static int holidayer_mkdir(HOLIDAYER *op,char *rbuf,cchar *dn)
{
	const int	rlen = MAXPATHLEN ;
	int		rs = SR_OK ;
	int		i = 0 ;

	if (dn[0] != '/') {
	    cchar	*pr = op->pr ;
	    const int	prl = strlen(op->pr) ;
	    if ((rs = storebuf_strw(rbuf,rlen,i,pr,prl)) >= 0) {
	        i += rs ;
	        if (pr[prl-1] != '/') {
	            rs = storebuf_char(rbuf,rlen,i,'/') ;
	            i += rs ;
	        }
	    }
	}

	if (rs >= 0) {
	    rs = storebuf_strw(rbuf,rlen,i,dn,-1) ;
	    i += rs ;
	}

	return (rs >= 0) ? i : rs ;
}
/* end subroutine (holidayer_mkdir) */


static int holidayer_mkfname(HOLIDAYER *op,char *rbuf,cchar *dn,uint y)
{
	const int	rlen = MAXPATHLEN ;
	int		rs = SR_OK ;
	int		i = 0 ;
	cchar		*pr = op->pr ;
	cchar		*prefix = HOLIDAYER_FPREFIX ;

	if (dn[0] != '/') {
	    const int	prl = strlen(pr) ;
	    if ((rs = storebuf_strw(rbuf,rlen,i,pr,prl)) >= 0) {
	        i += rs ;
	        if (pr[prl-1] != '/') {
	            rs = storebuf_char(rbuf,rlen,i,'/') ;
	            i += rs ;
	        }
	    }
	}

	if (rs >= 0) {
	    rs = storebuf_strw(rbuf,rlen,i,dn,-1) ;
	    i += rs ;
	}

	if (rs >= 0) {
	    rs = storebuf_char(rbuf,rlen,i,'/') ;
	    i += rs ;
	}

	if (rs >= 0) {
	    rs = storebuf_strw(rbuf,rlen,i,prefix,-1) ;
	    i += rs ;
	}

	if ((rs >= 0) && (y > 0)) {
	    const int	dlen = DIGBUFLEN ;
	    char	dbuf[DIGBUFLEN+1] ;
	    if ((rs = ctdecpui(dbuf,dlen,4,y)) >= 0) {
	        rs = storebuf_strw(rbuf,rlen,i,dbuf,rs) ;
	        i += rs ;
	    }
	}

	return (rs >= 0) ? i : rs ;
}
/* end subroutine (holidayer_mkfname) */


static int holidayer_holaudit(HOLIDAYER *op,HOLIDAYER_HOL *hep)
{
	int		rs ;
	if (op == NULL) return SR_FAULT ;
	if (hep->year > 0) {
	    rs = holidays_audit(&hep->hol) ;
	} else {
	    rs = SR_BADFMT ;
	}
	return rs ;
}
/* end subroutine (holidayer_holaudit) */


static int holidayer_yearq(HOLIDAYER *op,HOLIDAYER_CITE *qp)
{
	int		rs = SR_OK ;
	int		y = qp->y ;
	if (qp->y == 0) {
	    if (op->year == 0) {
	        rs = holidayer_yearmk(op) ;
	    }
	    if (rs >= 0) {
	        qp->y = op->year ;
	        y = op->year ;
	    }
	} else {
	    if (qp->y >= 2038) rs = SR_INVALID ;
	} /* end if (needed) */
	return (rs >= 0) ? y : rs ;
}
/* end subroutine (holidayer_yearq) */


static int holidayer_year(HOLIDAYER *op,uint ay)
{
	int		rs = SR_OK ;
	int		y = (int) ay ;
	if (y == 0) {
	    if (op->year == 0) {
	        rs = holidayer_yearmk(op) ;
	    }
	    if (rs >= 0) {
	        y = op->year ;
	    }
	} else {
	    if (y >= 2038) rs = SR_INVALID ;
	}
	return (rs >= 0) ? y : rs ;
}
/* end subroutine (holidayer_year) */


static int holidayer_yearmk(HOLIDAYER *op)
{
	int		rs = SR_OK ;
	int		y = 0 ;
	if (op->year == 0) {
	    TMTIME		m ;
	    const time_t	t = time(NULL) ;
	    if ((rs = tmtime_localtime(&m,t)) >= 0) {
	        y = (m.year + TM_YEAR_BASE) ;
	        op->year = y ;
	    }
	}
	return (rs >= 0) ? y : rs ;
}
/* end subroutine (holidayer_yearmk) */


static int isOurMode(mode_t m)
{
	int		f = FALSE ;
	f = f || S_ISREG(m) ;
	f = f || S_ISSOCK(m) ;
	f = f || S_ISCHR(m) ;
	f = f || S_ISFIFO(m) ;
	return f ;
}
/* end subroutine (isOurMode) */


