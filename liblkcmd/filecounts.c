/* filecounts */

/* manage a file-based counter database */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This module is a manager for a counter data-base maintained in a file.

	Synopsis:

	int filecounts_open(op,fname)
	FILECOUNTS	*op ;
	const char	fname[] ;

	int filecounts_close(op)
	FILECOUNTS	*op ;

	int filecounts_process(op,list)
	FILECOUNTS	*op ;
	FILECOUNTS_N	*list ;

	= Notes

	Different processing occurs for each variable depending on how
	the variable 'value' in a list-item is set:

	value		action
	-1		retrieve value only (do not add if not present)
	0		create as necessary and retrieve value only
	1		create as necessary, retreive value, and increment
	<v>		create as necessary, and set to value <v>


	Format of file records:

	<count> <date> <name> 


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>
#include	<time.h>

#include	<vsystem.h>
#include	<filebuf.h>
#include	<vecobj.h>
#include	<storebuf.h>
#include	<dater.h>
#include	<char.h>
#include	<localmisc.h>

#include	"filecounts.h"


/* local defines */

#define	WORKER		struct worker
#define	WORKER_ENT	struct worker_ent
#define	WORKER_CMDNUL	0
#define	WORKER_CMDINC	1
#define	WORKER_CMDSET	2

#define	DEFNENTRIES	10

#ifndef	DIGBUFLEN
#define	DIGBUFLEN	40
#endif

#define	TO_LOCK		4		/* seconds */

#ifndef	LINEBUFLEN
#ifdef	LINE_MAX
#define	LINEBUFLEN	MAX(LINE_MAX,2048)
#else
#define	LINEBUFLEN	2048
#endif
#endif

#define	UPDATEBUFLEN	(FILECOUNTS_NUMDIGITS + 1 + \
	            FILECOUNTS_LOGZLEN + 1 + MAXNAMELEN + 1)

#ifndef	TIMEBUFLEN
#define	TIMEBUFLEN	80
#endif


/* external subroutines */

extern uint	uaddsat(uint,uint) ;

extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy3(char *,int,const char *,const char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	nleadstr(const char *,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecui(const char *,int,uint *) ;
extern int	ctdecui(char *,int,uint) ;
extern int	iaddsat(int,int) ;
extern int	lockfile(int,int,offset_t,offset_t,int) ;
extern int	strlinelen(const char *,int,int) ;
extern int	isNotPresent(int) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	debugprinthex(const char *,int,const char *,int) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*strdcpy3(char *,int,const char *,const char *,const char *) ;
extern char	*timestr_logz(time_t,char *) ;


/* external variables */


/* local structures */

struct worker {
	FILECOUNTS_N	*nlp ;
	vecobj		list ;
	int		nremain ;	/* number of entries */
	int		vall ;
} ;

struct worker_ent {
	const char	*name ;		/* name of DB entry */
	uint		ovalue ;	/* old-value */
	uint		avalue ;	/* action-value */
	int		eoff ;		/* offset in file */
	int		action ;	/* action to perform */
	int		ni ;		/* name-index */
} ;


/* forward references */

static int	filecounts_proclist(FILECOUNTS *,FILECOUNTS_N *) ;
static int	filecounts_scan(FILECOUNTS *,WORKER *,FILEBUF *) ;
static int	filecounts_update(FILECOUNTS *,WORKER *) ;
static int 	filecounts_fins(FILECOUNTS *,WORKER *) ;
static int	filecounts_procline(FILECOUNTS *,WORKER *,int,cchar *,int) ;
static int	filecounts_updateone(FILECOUNTS *,cchar *,WORKER_ENT *) ;
static int	filecounts_append(FILECOUNTS *,cchar *,WORKER_ENT *) ;
static int	filecounts_snaper(FILECOUNTS *,VECOBJ *) ;
static int	filecounts_snaperline(FILECOUNTS *,DATER *,VECOBJ *,
			char *,int) ;
static int	filecounts_lockbegin(FILECOUNTS *) ;
static int	filecounts_lockend(FILECOUNTS *) ;

static int	worker_start(WORKER *,FILECOUNTS_N *) ;
static int	worker_match(WORKER *,const char *,int) ;
static int	worker_record(WORKER *,int,int,uint) ;
static int	worker_remaining(WORKER *) ;
static int	worker_sort(WORKER *) ;
static int	worker_get(WORKER *,int,WORKER_ENT **) ;
static int	worker_finish(WORKER *) ;

static int	mkentry(char *,int,uint,int,cchar *,int,cchar *) ;
static int	actioncmd(int) ;

static int	vcmpoff(const void *,const void *) ;


/* local variables */

static const char	*total = "TOTAL" ;
static const char	blanks[] = "                    " ;


/* exported subroutines */


int filecounts_open(FILECOUNTS *op,cchar *fname,int oflags,mode_t om)
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;
	if (fname == NULL) return SR_FAULT ;

	memset(op,0,sizeof(FILECOUNTS)) ;
	op->f.rdonly = ((oflags & O_ACCMODE) == O_RDONLY) ;

	if ((rs = u_open(fname,oflags,om)) >= 0) {
	    op->fd = rs ;
	    if ((rs = uc_mallocstrw(fname,-1,&op->fname)) >= 0) {
	        op->magic = FILECOUNTS_MAGIC ;
	    }
	    if (rs < 0) {
	        u_close(op->fd) ;
	        op->fd = -1 ;
	    }
	} /* end if (open) */

	return rs ;
}
/* end subroutine (filecounts_open) */


int filecounts_close(FILECOUNTS *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != FILECOUNTS_MAGIC) return SR_NOTOPEN ;

	if (op->fname != NULL) {
	    rs1 = uc_free(op->fname) ;
	    if (rs >= 0) rs = rs1 ;
	    op->fname = NULL ;
	}

	if (op->fd >= 0) {
	    rs1 = u_close(op->fd) ;
	    if (rs >= 0) rs = rs1 ;
	    op->fd = -1 ;
	}

	op->magic = 0 ;
	return rs ;
}
/* end subroutine (filecounts_close) */


int filecounts_process(FILECOUNTS *op,FILECOUNTS_N *nlp)
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;
	if (nlp == NULL) return SR_FAULT ;

	if (op->magic != FILECOUNTS_MAGIC) return SR_NOTOPEN ;

#if	CF_DEBUGS
	{
	    int	i ;
	    debugprintf("filecounts_process: ent\n") ;
	    for (i = 0 ; nlp[i].name != NULL ; i += 1)
	        debugprintf("filecounts_process: n=%s\n",nlp[i].name) ;
	}
#endif /* CF_DEBUGS */

	rs = filecounts_proclist(op,nlp) ;

#if	CF_DEBUGS
	debugprintf("filecounts_process: _proclist() rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (filecounts_process) */


int filecounts_curbegin(FILECOUNTS *op,FILECOUNTS_CUR *curp)
{
	int		rs = SR_OK ;

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;

	if (op->magic != FILECOUNTS_MAGIC) return SR_NOTOPEN ;

	memset(curp,0,sizeof(FILECOUNTS_CUR)) ;
	curp->magic = FILECOUNTS_MAGIC ;

	op->ncursors += 1 ;

	return rs ;
}
/* end subroutine (filecounts_curbegin) */


int filecounts_curend(FILECOUNTS *op,FILECOUNTS_CUR *curp)
{
	FILECOUNTS_II	*list ;
	int		rs = SR_OK ;
	int		rs1 ;

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;

	if (op->magic != FILECOUNTS_MAGIC) return SR_NOTOPEN ;
	if (curp->magic != FILECOUNTS_MAGIC) return SR_NOTOPEN ;

	if ((curp->nlist > 0) && (curp->list != NULL)) {
	    int		i ;
	    list = curp->list ;
	    for (i = 0 ; i < curp->nlist ; i += 1) {
	        if (list[i].name != NULL) {
	            rs1 = uc_free(list[i].name) ;
	            if (rs >= 0) rs = rs1 ;
	        }
	    } /* end for */
	    curp->nlist = 0 ;
	} /* end if */

	if (curp->list != NULL) {
	    rs1 = uc_free(curp->list) ;
	    if (rs >= 0) rs = rs1 ;
	    curp->list = NULL ;
	}

	op->ncursors -= 1 ;

	curp->i = 0 ;
	curp->magic = 0 ;
	return rs ;
}
/* end subroutine (filecounts_curend) */


/* take a "snapshot" of all of the counters */
int filecounts_snap(FILECOUNTS *op,FILECOUNTS_CUR *curp)
{
	VECOBJ		tlist ;
	const int	iisize = sizeof(FILECOUNTS_II) ;
	const int	to = TO_LOCK ;
	int		rs ;
	int		n = DEFNENTRIES ;
	int		opts = 0 ;

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;

	if (op->magic != FILECOUNTS_MAGIC) return SR_NOTOPEN ;
	if (curp->magic != FILECOUNTS_MAGIC) return SR_NOTOPEN ;

	if (op->ncursors == 0)
	    return SR_BADSLOT ;

	if ((rs = vecobj_start(&tlist,iisize,n,opts)) >= 0) {

	    if ((rs = lockfile(op->fd,F_RLOCK,0L,0L,to)) >= 0) {

	        rs = filecounts_snaper(op,&tlist) ;

	        lockfile(op->fd,F_ULOCK,0L,0L,0) ;
	    } /* end if (locked-DB) */

	    if (rs >= 0) {
	        int	size ;
	        char	*p ;

#if	CF_DEBUGS
	        {
	            FILECOUNTS_II	*ep ;
	            int	i ;
	            for (i = 0 ; vecobj_get(&tlist,i,&ep) >= 0 ; i += 1)
	                debugprintf("filecounts_snap: i=%u n=%s\n",
	                    i,ep->name) ;
	        }
#endif

	        n = vecobj_count(&tlist) ;

#if	CF_DEBUGS
	        debugprintf("filecounts_snap: n=%d \n",n) ;
#endif

	        size = ((n + 1) * iisize) ;
	        if ((rs = uc_malloc(size,&p)) >= 0) {
	            FILECOUNTS_II	*list, *ep ;
	            int			i ;
	            list = (FILECOUNTS_II *) p ;
	            n = 0 ;
	            for (i = 0 ; vecobj_get(&tlist,i,&ep) >= 0 ; i += 1) {
	                if (ep != NULL) {
	                    list[n++] = *ep ;
	                }
	            } /* end for */
	            list[n].name = NULL ;
	            list[n].utime = 0 ;
	            list[n].value = -1 ;
	            curp->list = list ;
	            curp->nlist = n ;
	        } /* end if (memory allocation) */
	    } /* end if (ok) */

	    vecobj_finish(&tlist) ;
	} /* end if (tlist) */

#if	CF_DEBUGS
	debugprintf("filecounts_snap: ret rs=%d n=%d\n",rs,n) ;
#endif

	return (rs >= 0) ? n : rs ;
}
/* end subroutine (filecounts_snap) */


/* read (get info on) a counter by name */
int filecounts_read(FILECOUNTS *op,FILECOUNTS_CUR *curp,FILECOUNTS_INFO *fcip,
		char *nbuf,int nlen)
{
	int		rs = SR_OK ;
	int		ei ;
	int		nl = 0 ;
	const char	*np ;

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;
	if (fcip == NULL) return SR_FAULT ;
	if (nbuf == NULL) return SR_FAULT ;

	if (op->magic != FILECOUNTS_MAGIC) return SR_NOTOPEN ;
	if (curp->magic != FILECOUNTS_MAGIC) return SR_NOTOPEN ;

	ei = (curp->i >= 0) ? curp->i : 0 ;
	if (ei < curp->nlist) {

#if	CF_DEBUGS
	    debugprintf("filecounts_read: ei=%u\n",ei) ;
#endif

	    fcip->utime = curp->list[ei].utime ;
	    fcip->value = curp->list[ei].value ;
	    np = curp->list[ei].name ;
	    nbuf[0] = '\0' ;
	    if (np != NULL) {
	        rs = sncpy1(nbuf,nlen,np) ;
	        nl = rs ;
	    }

	    if (rs >= 0) {
	        curp->i = (ei + 1) ;
	    }

	} else
	    rs = SR_NOTFOUND ;

#if	CF_DEBUGS
	debugprintf("filecounts_read: ret rs=%d nl=%u\n",rs,nl) ;
#endif

	return (rs >= 0) ? nl : rs ;
}
/* end subroutine (filecounts_read) */


/* private subroutines */


static int filecounts_proclist(FILECOUNTS *op,FILECOUNTS_N *nlp)
{
	WORKER		work ;
	int		rs ;
	int		rs1 ;
	int		opts = 0 ;

	if ((rs = worker_start(&work,nlp)) >= 0) {
	    if ((rs = filecounts_lockbegin(op)) >= 0) {
	        FILEBUF	fb ;
	        if ((rs = filebuf_start(&fb,op->fd,0L,0,opts)) >= 0) {
	            if ((rs = filecounts_scan(op,&work,&fb)) >= 0) {
	                if (! op->f.rdonly) {
	                    rs = filecounts_update(op,&work) ;
	                } /* end if */
	                if (rs >= 0)
	                    rs = filecounts_fins(op,&work) ;
	            } /* end if (search) */
	            rs1 = filebuf_finish(&fb) ;
	            if (rs >= 0) rs = rs1 ;
	        } /* end if (filebuf) */
	        if (rs >= 0) rs = rs1 ;
	        rs1 = filecounts_lockend(op) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (locked-DB) */
	    rs1 = worker_finish(&work) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (worker) */

	return rs ;
}
/* end subroutine (filecounts_proclist) */


static int filecounts_scan(FILECOUNTS *op,WORKER *wp,FILEBUF *fbp)
{
	uint		foff = 0 ;
	const int	llen = LINEBUFLEN ;
	const int	to = TO_LOCK ;
	int		rs ;
	int		len ;
	int		rn = 1 ;
	char		lbuf[LINEBUFLEN + 1] ;

	while ((rs = filebuf_readline(fbp,lbuf,llen,to)) > 0) {
	    len = rs ;

#if	CF_DEBUGS
	    debugprintf("filecounts_scan: l=>%t<\n",
	        lbuf,strlinelen(lbuf,len,40)) ;
#endif
	    if ((rs = filecounts_procline(op,wp,foff,lbuf,len)) >= 0) {
	        foff += len ;
	        rn = worker_remaining(wp) ;
	    }
#if	CF_DEBUGS
	    debugprintf("filecounts_scan: rn=%d\n",rn) ;
#endif
	    if (rn <= 0) break ;
	    if (rs < 0) break ;
	} /* end while */

#if	CF_DEBUGS
	debugprintf("filecounts_scan: ret rs=%d rn=%d\n",rs,rn) ;
#endif
	return rs ;
}
/* end subroutine (filecounts_scan) */


static int filecounts_procline(op,wp,eoff,lbuf,len)
FILECOUNTS	*op ;
WORKER		*wp ;
int		eoff ;
const char	*lbuf ;
int		len ;
{
	uint		v ;
	const int	llen = FILECOUNTS_NUMDIGITS ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		si ;
	int		nl ;
	const char	*np ;

	if (op == NULL) return SR_FAULT ;

/* calulate the "skip" index */

	si = (FILECOUNTS_NUMDIGITS + 1 + FILECOUNTS_LOGZLEN + 1) ;
	if (len >= (si+1)) {

	    np = (lbuf + si) ;
	    nl = (len - si) ;
	    if (nl && (np[nl-1] == '\n')) nl -= 1 ;

	    while (nl && CHAR_ISWHITE(np[nl-1] & 0xff)) /* EOL removal */
	        nl -= 1 ;

/* we should be at the counter name within the entry */

	    if ((rs1 = cfdecui(lbuf,llen,&v)) >= 0) {
	        wp->vall += v ;
	        if ((rs1 = worker_match(wp,np,nl)) >= 0) {
	            int	ei = rs1 ;
	            rs = worker_record(wp,ei,eoff,v) ;
	        } else if (rs1 != SR_NOTFOUND) {
	            rs = rs1 ;
	        }
	    } /* end if (valid number) */

	} /* end if (valid entry) */

	return rs ;
}
/* end subroutine (filecounts_procline) */


static int filecounts_update(FILECOUNTS *op,WORKER *wp)
{
	time_t		daytime = time(NULL) ;
	int		rs ;
	int		c = 0 ;
	char		tbuf[TIMEBUFLEN + 1] ;

/* create the time-string to put in the DB file */

	timestr_logz(daytime,tbuf) ;

/* sort the entries by offset (w/ new ones at the rear) */

	if ((rs = worker_sort(wp)) >= 0) {
	    FILECOUNTS_N	*nlp = wp->nlp ;
	    WORKER_ENT		*wep ;
	    int			i ;
	    for (i = 0 ; worker_get(wp,i,&wep) >= 0 ; i += 1) {
	        if (wep != NULL) {
	            if (wep->action >= 0) {
	                if (wep->eoff >= 0) {
	                    if (wep->action > 0) {
	                        c += 1 ;
	                        rs = filecounts_updateone(op,tbuf,wep) ;
	                    }
	                } else {
	                    c += 1 ;
	                    if (strcmp(wep->name,"TOTAL") == 0) {
	                        const int	ni = wep->ni ;
	                        wep->action = WORKER_CMDSET ;
	                        nlp[ni].value += wp->vall ;
	                    }
	                    rs = filecounts_append(op,tbuf,wep) ;
	                }
	            }
	        }
	        if (rs < 0) break ;
	    } /* end for */
	} /* end if (worker_sort) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (filecounts_update) */


static int filecounts_updateone(FILECOUNTS *op,cchar *tbuf,WORKER_ENT *wep)
{
	uint		nv ;
	const int	ulen = UPDATEBUFLEN ;
	const int	na = wep->action ;
	int		rs = SR_OK ;
	int		ni = wep->ni ;
	int		wlen = 0 ;
	char		ubuf[UPDATEBUFLEN + 1] ;

	nv = wep->ovalue ; /* default is same as old value */
	switch (na) {
	case WORKER_CMDINC:
	    if (ni >= 0) {
	        nv = uaddsat(wep->ovalue,wep->avalue) ;
	    } else {
	        nv = (wep->ovalue + 1) ;
	    }
	    break ;
	case WORKER_CMDSET:
	    nv = wep->avalue ;
	    break ;
	} /* end switch */

#ifdef	COMMENT
	rs = bufprintf(ubuf,ulen,"%-*u %*s",
	    FILECOUNTS_NUMDIGITS,nv,FILECOUNTS_LOGZLEN,tbuf) ;
	wlen = rs ;
#else
	rs = mkentry(ubuf,ulen,
	    nv,FILECOUNTS_NUMDIGITS,tbuf,FILECOUNTS_LOGZLEN,NULL) ;
	wlen = rs ;
#endif /* COMMENT */

#if	CF_DEBUGS
	debugprintf("filecounts_updateone: mkentry() rs=%d\n",rs) ;
	debugprintf("filecounts_updateone: uoff=%ld\n",wep->eoff) ;
#endif

	if (rs >= 0) {
	    offset_t	uoff = wep->eoff ;
	    rs = u_pwrite(op->fd,ubuf,wlen,uoff) ;
	}

#if	CF_DEBUGS
	debugprintf("filecounts_updateone: ret rs=%d\n",rs) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (filecounts_updateone) */


static int filecounts_append(FILECOUNTS *op,cchar *tbuf,WORKER_ENT *wep)
{
	offset_t	eoff ;
	uint		nv ;
	const int	nvsize = FILECOUNTS_NUMDIGITS ;
	const int	tsize = FILECOUNTS_LOGZLEN ;
	const int	ulen = UPDATEBUFLEN ;
	const int	na = wep->action ;
	int		rs ;
	int		wlen = 0 ;
	char		ubuf[UPDATEBUFLEN + 1] ;

#if	CF_DEBUGS
	{
	    int	i ;
	    debugprintf("filecounts_append: ent na=%u\n",na) ;
	    debugprintf("filecounts_append: n=%s\n",wep->name) ;
	}
#endif

	nv = 0 ;
	switch (na) {
	case WORKER_CMDINC:
	    nv = (wep->avalue+1) ;
	    break ;
	case WORKER_CMDSET:
	    nv = wep->avalue ;
	    break ;
	} /* end switch */

#ifdef	COMMENT
	rs = bufprintf(ubuf,ulen,"%*u %*s",nvsize,nv,tsize,tbuf) ;
	wlen += rs ;
#else
	rs = mkentry(ubuf,ulen,nv,nvsize,tbuf,tsize,wep->name) ;
	wlen += rs ;
#endif /* COMMENT */

	if (rs >= 0) {
	    if ((rs = u_seeko(op->fd,0L,SEEK_END,&eoff)) >= 0) {
	        rs = u_write(op->fd,ubuf,wlen) ;
	        wep->eoff = eoff ;
	    }
	}

#if	CF_DEBUGS
	debugprintf("filecounts_append: ret rs=%d wlen=%u\n",rs,wlen) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (filecounts_append) */


static int filecounts_fins(FILECOUNTS *op,WORKER *wp)
{
	FILECOUNTS_N	*nlp = wp->nlp ;
	WORKER_ENT	*wep ;
	int		rs = SR_OK ;
	int		i ;
	int		ni ;

	if (op == NULL) return SR_FAULT ;

	for (i = 0 ; worker_get(wp,i,&wep) >= 0 ; i += 1) {
	    if (wep != NULL) {
	        if (wep->eoff < 0) {
	            ni = wep->ni ;
	            if (ni >= 0) nlp[ni].value = -1 ;
	        }
	    }
	} /* end for */

	return rs ;
}
/* end subroutine (filecounts_fins) */


static int filecounts_snaper(FILECOUNTS *op,VECOBJ *ilp)
{
	DATER		dm ;
	int		rs ;

	if ((rs = dater_start(&dm,NULL,NULL,0)) >= 0) {
	    FILEBUF	fb ;
	    const int	opts = 0 ;
	    if ((rs = filebuf_start(&fb,op->fd,0L,0,opts)) >= 0) {
	        const int	to = -1 ;
	        const int	llen = LINEBUFLEN ;
	        char		lbuf[LINEBUFLEN + 1] ;
	        while ((rs = filebuf_readline(&fb,lbuf,llen,to)) > 0) {
	            rs = filecounts_snaperline(op,&dm,ilp,lbuf,rs) ;
	            if (rs < 0) break ;
	        } /* end while */
	        filebuf_finish(&fb) ;
	    } /* end if (filebuf) */
	    dater_finish(&dm) ;
	} /* end if (dater) */

	return rs ;
}
/* end subroutine (filecounts_snaper) */


static int filecounts_snaperline(op,dmp,ilp,lbuf,llen)
FILECOUNTS	*op ;
DATER		*dmp ;
VECOBJ		*ilp ;
char		lbuf[] ;
int		llen ;
{
	int		rs = SR_OK ;
	int		si ;

	if (op == NULL) return SR_FAULT ;

	si = (FILECOUNTS_NUMDIGITS + 1 + FILECOUNTS_LOGZLEN + 1) ;
	if (llen >= (si+1)) {
	    uint	v ;
	    int		sl = llen ;
	    int		cl ;
	    const char	*cp ;
	    const char	*np ;
	    const char	*sp = lbuf ;

	if (sl && (sp[sl-1] == '\n')) sl -= 1 ;

	while (sl && CHAR_ISWHITE(sp[sl-1])) /* EOL removal */
	    sl -= 1 ;

	cp = sp ;
	cl = FILECOUNTS_NUMDIGITS ;
	if ((rs = cfdecui(cp,cl,&v)) >= 0) {

	sl -= ((cp + cl + 1) - sp) ;
	sp = (cp + cl + 1) ;

	cp = (const char *) sp ;
	cl = FILECOUNTS_LOGZLEN ;
	if ((rs = dater_setlogz(dmp,cp,cl)) >= 0) {
	    time_t	utime ;
	    if ((rs = dater_gettime(dmp,&utime)) >= 0) {

	sl -= ((cp + cl + 1) - sp) ;
	sp = (cp + cl + 1) ;

	cp = sp ;
	cl = sl ;

#if	CF_DEBUGS
	debugprintf("filecounts_snaperline: cn=%t\n",cp,cl) ;
#endif

	            if ((rs = uc_mallocstrw(cp,cl,&np)) >= 0) {
	                FILECOUNTS_II	ii ;
	                ii.name = np ;
	                ii.value = v ;
	                ii.utime = utime ;
	                rs = vecobj_add(ilp,&ii) ;
	                if (rs < 0) 
		            uc_free(np) ;
	            } /* end if (m-a) */
	        } /* end if (dater_gettime) */
	    } /* end if (dater_setlogz) */
	} /* end if (cfdecui) */

	} /* end if (zero or positive) */

#if	CF_DEBUGS
	debugprintf("filecounts_snaperline: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (filecounts_snaperline) */


static int filecounts_lockbegin(FILECOUNTS *op)
{
	const int	to = TO_LOCK ;
	const int	cmd = (op->f.rdonly) ? F_RLOCK : F_WLOCK ;
	int		rs ;
	rs = lockfile(op->fd,cmd,0L,0L,to) ;
	return rs ;
}
/* end subroutine (filecounts_lockbegin) */


static int filecounts_lockend(FILECOUNTS *op)
{
	const int	cmd = F_ULOCK ;
	int		rs ;
	rs = lockfile(op->fd,cmd,0L,0L,0) ;
	return rs ;
}
/* end subroutine (filecounts_lockend) */


static int worker_start(WORKER *wp,FILECOUNTS_N *nlp)
{
	const int	wesize = sizeof(WORKER_ENT) ;
	const int	n = DEFNENTRIES ;
	int		rs ;
	int		opts ;

	memset(wp,0,sizeof(WORKER)) ;
	wp->nlp = nlp ;

	opts = VECOBJ_OCOMPACT ;
	if ((rs = vecobj_start(&wp->list,wesize,n,opts)) >= 0) {
	    WORKER_ENT	we ;
	    int		i ;
	    int		na ;
	    int		vadding = 0 ;
	    int		ti = -1 ;
	    const char	*tnp = total ;
	    const char	*np ;
	    for (i = 0 ; nlp[i].name != NULL ; i += 1) {
	        np = nlp[i].name ;
	        if ((np[0] != 'T') || (strcmp(np,tnp) != 0)) {
	            const int	nv = nlp[i].value ;
	            na = actioncmd(nv) ;
	            if (na == WORKER_CMDINC) vadding += nv ;
	            memset(&we,0,wesize) ;
	            we.eoff = -1 ;
	            we.ni = i ;
	            we.name = np ;
	            we.action = na ;
	            we.avalue = nv ;
	            rs = vecobj_add(&wp->list,&we) ;
	        } else
	            ti = i ;
	        if (rs < 0) break ;
	    } /* end for */
	    if (rs >= 0) {
#if	CF_DEBUGS
	        debugprintf("worker_start: vadding=%u\n",vadding) ;
#endif
	        memset(&we,0,wesize) ;
	        we.eoff = -1 ;
	        we.ni = ti ;
	        we.name = tnp ;
	        we.action =  WORKER_CMDINC ;
	        we.avalue = vadding ;
	        rs = vecobj_add(&wp->list,&we) ;
	        wp->nremain = (i+1) ; /* number of entries */
	    }
	    if (rs < 0)
	        vecobj_finish(&wp->list) ;
	} /* end if (vecobj_start) */

#if	CF_DEBUGS
	{
	    WORKER_ENT	*wep ;
	    int	i ;
	    for (i = 0 ; vecobj_get(&wp->list,i,&wep) >= 0 ; i += 1) {
	        debugprintf("worker_start: i=%u n=%s a=%d av=%d\n",
	            i,wep->name,wep->action,wep->avalue) ;
	    }
	}
#endif /* CF_DEBUGS */

	return rs ;
}
/* end subroutine (worker_start) */


static int worker_finish(WORKER *wp)
{
	int		rs = SR_OK ;
	int		rs1 ;

	rs1 = vecobj_finish(&wp->list) ;
	if (rs >= 0) rs = rs1 ;

	return rs ;
}
/* end subroutine (worker_finish) */


static int worker_match(WORKER *wp,cchar *np,int nl)
{
	WORKER_ENT	*wep ;
	int		rs = SR_OK ;
	int		i ;
	int		m ;

#if	CF_DEBUGS
	debugprintf("filecounts/worker_match: n=%t\n",np,nl) ;
#endif

	for (i = 0 ; (rs = vecobj_get(&wp->list,i,&wep)) >= 0 ; i += 1) {
	    if (wep != NULL) {
	        m = nleadstr(wep->name,np,nl) ;
	        if ((m > 0) && (wep->name[m] == '\0')) break ;
	    }
	} /* end for */

#if	CF_DEBUGS
	debugprintf("filecounts/worker_match: ret rs=%d i=%d\n",rs,i) ;
#endif

	return (rs >= 0) ? i : rs ;
}
/* end subroutine (worker_match) */


static int worker_remaining(WORKER *wp)
{

	return wp->nremain ;
}
/* end subroutine (worker_remaining) */


static int worker_record(WORKER *wp,int ei,int eoff,uint v)
{
	FILECOUNTS_N	*nlp = wp->nlp ;
	WORKER_ENT	*wep ;
	int		rs ;

	if ((rs = vecobj_get(&wp->list,ei,&wep)) >= 0) {
	    const int	ni = wep->ni ;
	    if (wp->nremain > 0) wp->nremain -= 1 ;
	    wep->eoff = eoff ;
	    wep->ovalue = v ;
	    if (ni >= 0) nlp[ni].value = v ; /* return previous value */
	}

	return rs ;
}
/* end subroutine (worker_record) */


static int worker_sort(WORKER *wp)
{
	int		rs ;

	rs = vecobj_sort(&wp->list,vcmpoff) ;

	return rs ;
}
/* end subroutine (worker_sort) */


static int worker_get(WORKER *wp,int i,WORKER_ENT **rpp)
{
	int		rs ;

	rs = vecobj_get(&wp->list,i,rpp) ;

	return rs ;
}
/* end subroutine (worker_get) */


/* make either a partial (update) or full DB entry */
static int mkentry(rbuf,rlen,nv,nvsize,tbuf,timesize,name)
char		rbuf[] ;
int		rlen ;
uint		nv ;
int		nvsize ;
const char	tbuf[] ;
int		timesize ;
const char	name[] ;
{
	int		rs ;
	int		n ;
	int		nr ;
	int		i = 0 ;
	char		digbuf[DIGBUFLEN + 1] ;

	rs = ctdecui(digbuf,DIGBUFLEN,nv) ;
	n = rs ;

	if (rs >= 0) {
	    nr = MAX((nvsize - n),0) ;
	    rs = storebuf_strw(rbuf,rlen,i,blanks,nr) ;
	    i += rs ;
	}

	if (rs >= 0) {
	    rs = storebuf_strw(rbuf,rlen,i,digbuf,n) ;
	    i += rs ;
	}

	if (rs >= 0) {
	    rs = storebuf_char(rbuf,rlen,i,' ') ;
	    i += rs ;
	}

	if (rs >= 0) {
	    rs = storebuf_strw(rbuf,rlen,i,tbuf,timesize) ;
	    n = rs ;
	    i += rs ;
	}

	if (rs >= 0) {
	    nr = MAX((timesize - n),0) ;
	    rs = storebuf_strw(rbuf,rlen,i,blanks,nr) ;
	    i += rs ;
	}

	if ((rs >= 0) && (name != NULL)) {
	    if (rs >= 0) {
	        rs = storebuf_char(rbuf,rlen,i,' ') ;
	        i += rs ;
	    }
	    if (rs >= 0) {
	        rs = storebuf_strw(rbuf,rlen,i,name,-1) ;
	        i += rs ;
	    }
	    if (rs >= 0) {
	        rs = storebuf_char(rbuf,rlen,i,'\n') ;
	        i += rs ;
	    }
	} /* end if */

	return (rs >= 0) ? i : rs ;
}
/* end subroutine (mkentry) */


static int actioncmd(int nv)
{
	int		na = 0 ;
	switch (nv) {
	case 1:
	    na = WORKER_CMDINC ;
	    break ;
	case 2:
	    na = WORKER_CMDSET ;
	    break ;
	} /* end switch */
	return na ;
}
/* end subroutine (actioncmd) */


static int vcmpoff(const void *v1p,const void *v2p)
{
	WORKER_ENT	**e1pp = (WORKER_ENT **) v1p ;
	WORKER_ENT	**e2pp = (WORKER_ENT **) v2p ;
	WORKER_ENT	*e1p ;
	WORKER_ENT	*e2p ;
	int		rc = 0 ;

	e1p = *e1pp ;
	e2p = *e2pp ;
	if ((e1p != NULL) || (e2p != NULL)) {
	    if (e1p != NULL) {
	        if (e2p != NULL) {
	            if ((e2p->eoff >= 0) || (e1p->eoff >= 0)) {
	                if ((rc = (e1p->eoff < 0) ? 1 : 0) == 0) {
	    	            if ((rc = (e2p->eoff < 0) ? -1 : 0) == 0) {
	    	                rc = (e1p->eoff - e2p->eoff) ;
		            }
	                }
	            } else
	                rc = 0 ;
	        } else
	            rc = -1 ;
	    } else
	        rc = 1 ;
	}

	return rc ;
}
/* end subroutine (vcmpoff) */


