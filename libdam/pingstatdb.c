/* pingstatdb */

/* object to manipulate a PINGSTATDB file */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_CREATE	0		/* always create the file? */
#define	CF_UNLOCK	1		/* always unlock after an operation */


/* revision history:

	= 1998-06-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/******************************************************************************

        This subroutine maintains a PINGSTATDB file. These files are used to
        maintain the names and status of a PING event.

	Arguments:

	psp		ping status_file pointer
	fname		filename

	Returns:

	<0		error
	==0		record written
	==1		record created

	Format of file records:

	- eight digits of a decimal count number field
	- the time of the last status change
	- the time of the last entry update
	- the UP/DOWN status
	- the hostname

	Format example:

       3 000505_1018:55_EDT 000505_2233:28_EDT U rcb
       5 000505_1018:55_EDT 000505_2234:43_EDT U *LAST_UPDATE*
       1 000505_2234:37_EDT 000505_2234:37_EDT U bars
       1 000505_2234:40_EDT 000505_2234:40_EDT U amps
       1 000505_2234:43_EDT 000505_2234:43_EDT U farads
       1 000505_2234:46_EDT 000505_2234:46_EDT U ergs

	Oh, and finally, since file record locking sucks, that is: it is
	very often BROKEN.  On the stupid Sun platforms, we attempt to
	lock to be a nice guy but we have relatively small timeouts
	after which we proceed anyway!  This strategy is applied to
	all record locked used throughout this subroutine.


******************************************************************************/


#define	PINGSTATDB_MASTER		0


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>
#include	<netdb.h>
#include	<time.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<vecitem.h>
#include	<dater.h>
#include	<localmisc.h>

#include	"pingstatdb.h"


/* local defines */

#define	PINGSTATDB_REC		struct pingstatdb_r

/* record-format (RF) paramters */
#define	RF_NUMDIGITS	8		/* RF count field width */
#define	RF_LOGZLEN	23		/* RF time log field width */
#define	RF_UPSTAT	1		/* RF up status field width */
#define	RF_LEAD0	(RF_NUMDIGITS + (2*RF_LOGZLEN))
#define	RF_LEAD1	(RF_UPSTAT + 4)
#define	RF_LEAD		(RF_LEAD0 + RF_LEAD1)

#define	BUFLEN		(RF_LEAD + MAXHOSTNAMELEN + 3)

#define	TO_LOCK		30		/* seconds */
#define	TO_MINUPDATE	3		/* minimum time between updates */

#define	LASTUPDATE	"*LAST_UPDATE*"

#ifndef	TIMEBUFLEN
#define	TIMEBUFLEN	80
#endif


/* external subroutines */

extern int	mkpath1(char *,const char *) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	perm(const char *,uid_t,gid_t,gid_t *,int) ;
extern int	initnow(TIMEB *,char *,int) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	debugprinthexblock(cchar *,int,const void *,int) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;
extern char	*timestr_log(time_t,char *) ;
extern char	*timestr_logz(time_t,char *) ;


/* external variables */


/* local structures */

struct pingstatdb_r {
	DATER		cdate ;		/* last change date */
	DATER		pdate ;		/* last ping date */
	const char	*hostname ;	/* host name */
	uint		roff ;		/* record offset (within file) */
	int		len ;		/* length of file record */
	int		count ;		/* count since last change */
	int		hostlen ;
	int		f_up ;		/* UP-DOWN status */
} ;


/* forward references */

static int	pingstatdb_checkcache(PINGSTATDB *) ;
static int	pingstatdb_readrecords(PINGSTATDB *) ;
static int	pingstatdb_fes(PINGSTATDB *) ;
static int	pingstatdb_getrec(PINGSTATDB *,const char *,
			PINGSTATDB_REC **) ;
static int	pingstatdb_updrec(PINGSTATDB *,time_t,DATER *,cchar *,
			int,time_t) ;

static int	record_start(PINGSTATDB_REC *,TIMEB *,cchar *,
			uint,cchar *,DATER *) ;
static int	record_startbuf(PINGSTATDB_REC *,TIMEB *,cchar *,
			uint,cchar *,int) ;
static int	record_update(PINGSTATDB_REC *,bfile *,DATER *,int) ;
static int	record_write(PINGSTATDB_REC *,bfile *,
			DATER *,DATER *,int,int) ;
static int	record_finish(PINGSTATDB_REC *) ;

static int	entry_load(PINGSTATDB_ENT *,PINGSTATDB_REC *) ;

static int	mkbstr(mode_t,char *) ;


/* local variables */

#if	CF_DEBUGS
static const char	*hostname = "EMPTY" ;
#endif


/* exported subroutines */


int pingstatdb_open(PINGSTATDB *psp,cchar *fname,mode_t omode,int fperm)
{
	int		rs = SR_OK ;
	cchar		*cp ;
	char		bstr[10] ;

#if	CF_DEBUGS
	debugprintf("pingstatdb_open: ent\n") ;
#endif

	if (psp == NULL) return SR_FAULT ;
	if (fname == NULL) return SR_FAULT ;

	if (fname[0] == '\0') return SR_INVALID ;

	memset(psp,0,sizeof(PINGSTATDB)) ;

#if	CF_DEBUGS
	debugprintf("pingstatdb_open: omode=%4o fname=%s\n",
	    omode,fname) ;
	if (omode & O_CREAT)
	    debugprintf("pingstatdb_open: creating as needed\n") ;
#endif

#if	CF_CREATE
	omode |= O_CREAT ;
#endif

	if ((rs = uc_mallocstrw(fname,-1,&cp)) >= 0) {
	    psp->fname = cp ;
	    psp->f.writable = ((omode & O_WRONLY) || (omode & O_RDWR)) ;
	    mkbstr(omode,bstr) ;
#if	CF_DEBUGS
	    debugprintf("pingstatdb_open: bopen mode=%s\n",bstr) ;
#endif
	    if ((rs = bopen(&psp->pfile,psp->fname,bstr,fperm)) >= 0) {
	        const int	cmd = BC_CLOSEONEXEC ;
#if	CF_DEBUGS
	        debugprintf("pingstatdb_open: bopen() rs=%d\n",rs) ;
#endif
	        if ((rs = bcontrol(&psp->pfile,cmd,TRUE)) >= 0) {
	            if ((rs = vecitem_start(&psp->entries,0,0)) >= 0) {
	                const int	zsize = DATER_ZNAMESIZE ;
	                if ((rs = initnow(&psp->now,psp->zname,zsize)) >= 0) {
	                    psp->magic = PINGSTATDB_MAGIC ;
	                }
	                if (rs < 0)
	                    vecitem_finish(&psp->entries) ;
	            }
	        } /* end if (bcontrol) */
	        if (rs < 0)
	            bclose(&psp->pfile) ;
	    } /* end if (bopen) */
	    if (rs < 0) {
	        uc_free(psp->fname) ;
	        psp->fname = NULL ;
	    }
	} /* end if (m-a) */

#if	CF_DEBUGS
	debugprintf("pingstatdb_open: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (pingstatdb_open) */


int pingstatdb_close(PINGSTATDB *psp)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (psp == NULL) return SR_FAULT ;

	if (psp->magic != PINGSTATDB_MAGIC) return SR_NOTOPEN ;

	rs1 = pingstatdb_fes(psp) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = bclose(&psp->pfile) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = vecitem_finish(&psp->entries) ;
	if (rs >= 0) rs = rs1 ;

	if (psp->fname != NULL) {
	    rs1 = uc_free(psp->fname) ;
	    if (rs >= 0) rs = rs1 ;
	    psp->fname = NULL ;
	}

#if	CF_DEBUGS
	debugprintf("pingstatdb_close: ret rs=%d\n",rs) ;
#endif

	psp->magic = 0 ;
	return rs ;
}
/* end subroutine (pingstatdb_close) */


/* initialize a cursor */
int pingstatdb_curbegin(PINGSTATDB *psp,PINGSTATDB_CUR *curp)
{

	if (psp == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;

	if (psp->magic != PINGSTATDB_MAGIC) return SR_NOTOPEN ;

	psp->f.cursor = TRUE ;
	curp->i = -1 ;
	return SR_OK ;
}
/* end subroutine (pingstatdb_curbegin) */


/* free up a cursor */
int pingstatdb_curend(PINGSTATDB *psp,PINGSTATDB_CUR *curp)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (psp == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;

	if (psp->magic != PINGSTATDB_MAGIC) return SR_NOTOPEN ;

	if (psp->f.readlocked || psp->f.writelocked) {
	    psp->f.readlocked = FALSE ;
	    psp->f.writelocked = FALSE ;
	    rs1 = bcontrol(&psp->pfile,BC_UNLOCK,0) ;
	    if (rs >= 0) rs = rs1 ;
	}

	psp->f.cursor = FALSE ;
	curp->i = -1 ;
	return rs ;
}
/* end subroutine (pingstatdb_curend) */


/* enumerate the entries */
int pingstatdb_enum(PINGSTATDB *psp,PINGSTATDB_CUR *curp,PINGSTATDB_ENT *ep)
{
	PINGSTATDB_REC	*rp ;
	int		rs = SR_OK ;
	int		hl = 0 ;

	if (psp == NULL) return SR_FAULT ;

	if (psp->magic != PINGSTATDB_MAGIC) return SR_NOTOPEN ;

	if ((! psp->f.readlocked) && (! psp->f.writelocked)) {
	    rs = bcontrol(&psp->pfile,BC_LOCKREAD,TO_LOCK) ;
	    psp->f.readlocked = (rs >= 0) ;
	}

	if (rs >= 0) {
	    if ((rs = pingstatdb_checkcache(psp)) >= 0) {
	        const int	i = (curp->i < 0) ? 0 : (curp->i + 1) ;
	        if ((rs = vecitem_get(&psp->entries,i,&rp)) >= 0) {
	            curp->i = i ;
	            if (rp != NULL) {
	                if (ep != NULL) {
	                    rs = entry_load(ep,rp) ;
	                    hl = rs ;
	                } else {
	                    hl = strlen(ep->hostname) ;
			}
	            } /* end if (non-null) */
	        } /* end if (vecitem_get) */
	        if (rs < 0) {
	            psp->f.readlocked = FALSE ;
	            psp->f.writelocked = FALSE ;
	            bcontrol(&psp->pfile,BC_UNLOCK,0) ;
	        }
	    } /* end if (pingstatdb_checkcache) */
	} /* end if (ok) */

	return (rs >= 0) ? hl : rs ;
}
/* end subroutine (pingstatdb_enum) */


/* match on a hostname */
int pingstatdb_match(PINGSTATDB *psp,cchar *hostname,PINGSTATDB_ENT *ep)
{
	PINGSTATDB_REC	*rp ;
	int		rs = SR_OK ;
	int		hl = 0 ;

	if (psp == NULL) return SR_FAULT ;
	if (hostname == NULL) return SR_FAULT ;

	if (psp->magic != PINGSTATDB_MAGIC) return SR_NOTOPEN ;

	if (hostname[0] == '\0') return SR_INVALID ;

#if	CF_DEBUGS
	debugprintf("pingstatdb_match: name=%s\n",hostname) ;
#endif

	if ((! psp->f.readlocked) && (! psp->f.writelocked)) {
	    rs = bcontrol(&psp->pfile,BC_LOCKREAD,TO_LOCK) ;
	    psp->f.readlocked = (rs >= 0) ;
	}

	if (rs >= 0) {
	    if ((rs = pingstatdb_checkcache(psp)) >= 0) {

#if	CF_DEBUGS
	debugprintf("pingstatdb_match: name=%s\n", hostname) ;
#endif

/* return SR_NOTFOUND if we fall off of the end */

	    if ((rs = pingstatdb_getrec(psp,hostname,&rp)) >= 0) {
	        if (rp != NULL) {
	            if (ep != NULL) {
	                rs = entry_load(ep,rp) ;
	                hl = rs ;
	            } else {
	                hl = strlen(ep->hostname) ;
	            }
	        } /* end if (non-null) */
	    } /* end if (pingstatdb_getrec) */

#if	CF_UNLOCK
	    psp->f.readlocked = FALSE ;
	    psp->f.writelocked = FALSE ;
	    bcontrol(&psp->pfile,BC_UNLOCK,0) ;
#endif /* CF_UNLOCK */

	    } else {
	        psp->f.readlocked = FALSE ;
	        psp->f.writelocked = FALSE ;
	        bcontrol(&psp->pfile,BC_UNLOCK,0) ;
	    } /* end if (pingstatdb_checkcache) */
	} /* end if (ok) */

#if	CF_DEBUGS
	debugprintf("pingstatdb_match: ret rs=%d hl=%u\n",rs,hl) ;
#endif

	return (rs >= 0) ? hl : rs ;
}
/* end subroutine (pingstatdb_match) */


/* update an entry */
int pingstatdb_update(PINGSTATDB *psp,cchar *hostname,int f_up,time_t timestamp)
{
	DATER		d ;
	const time_t	daytime = time(NULL) ;
	int		rs = SR_OK ;
	int		f_changed = FALSE ;

	if (psp == NULL) return SR_FAULT ;
	if (hostname == NULL) return SR_FAULT ;

	if (psp->magic != PINGSTATDB_MAGIC) return SR_NOTOPEN ;

	if (hostname[0] == '\0') return SR_INVALID ;

#if	CF_DEBUGS
	debugprintf("pingstatdb_update: hostname=%s\n",hostname) ;
#endif

	if (! psp->f.writable) {
	    rs = SR_BADF ;	/* not open for writing */
	    goto ret0 ;
	}

	if (psp->f.readlocked) {
	    psp->f.readlocked = FALSE ;
	    psp->f.writelocked = FALSE ;
	    bcontrol(&psp->pfile,BC_UNLOCK,0) ;
	}

	if (! psp->f.writelocked) {
	    rs = bcontrol(&psp->pfile,BC_LOCKWRITE,TO_LOCK) ;
	    psp->f.writelocked = (rs >= 0) ;
	} /* end if (we did not already have a lock on the file) */

	if (rs < 0)
	    goto ret0 ;

	rs = pingstatdb_checkcache(psp) ;
	if (rs < 0)
	    goto ret1 ;

	psp->now.time = daytime ;
	rs = dater_start(&d,&psp->now,psp->zname,-1) ;
	if (rs < 0)
	    goto ret1 ;

	if ((timestamp == 0) || (timestamp > daytime))
	    timestamp = daytime ;

#if	CF_DEBUGS
	{
	    debugprintf("pingstatdb_update: about to match\n") ;
	    rs = vecitem_count(&psp->entries) ;
	    debugprintf("pingstatdb_update: entries in cache %d\n",rs) ;
	}
#endif

	rs = pingstatdb_updrec(psp,daytime,&d,hostname,f_up,timestamp) ;
	f_changed = (rs > 0) ;

/* update the LASTUPDATE record */

	if (rs >= 0)
	    rs = pingstatdb_updrec(psp,daytime,&d,LASTUPDATE,f_up,timestamp) ;

/* udpate our last modification time to keep our cache current */

	psp->mtime = daytime ;
	bcontrol(&psp->pfile,BC_SYNC,0) ;

	dater_finish(&d) ;

/* unlock it */
ret1:
	psp->f.writelocked = FALSE ;
	bcontrol(&psp->pfile,BC_UNLOCK,0) ;

ret0:

#if	CF_DEBUGS
	debugprintf("pingstatdb_update: ret rs=%d f_changed=%u\n",
	    rs,f_changed) ;
#endif

	return (rs >= 0) ? f_changed : rs ;
}
/* end subroutine (pingstatdb_update) */


/* write an entry */
int pingstatdb_uptime(PINGSTATDB *psp,cchar *hostname,PINGSTATDB_UP *up)
{
	PINGSTATDB_REC	e, *rp ;
	DATER		cd, ud, *cdp ;
	offset_t	boff ;
	const time_t	daytime = time(NULL) ;
	time_t		ptime = 0 ;
	uint		timestamp ;
	uint		timechange ;
	uint		roff ;
	int		rs = SR_OK ;
	int		size ;
	int		f_up = TRUE ;
	int		f_changed = FALSE ;

	if (psp == NULL) return SR_FAULT ;
	if (hostname == NULL) return SR_FAULT ;
	if (up == NULL) return SR_FAULT ;

	if (psp->magic != PINGSTATDB_MAGIC) return SR_NOTOPEN ;

	if (hostname[0] == '\0') return SR_INVALID ;

	if (! psp->f.writable) {
	    rs = SR_BADF ;	/* not open for writing */
	    goto ret0 ;
	}

#if	CF_DEBUGS
	debugprintf("pingstatdb_uptime: lock stuff\n") ;
#endif

	if (psp->f.readlocked) {
	    psp->f.readlocked = FALSE ;
	    psp->f.writelocked = FALSE ;
	    bcontrol(&psp->pfile,BC_UNLOCK,0) ;
	}

	if (! psp->f.writelocked) {
	    rs = bcontrol(&psp->pfile,BC_LOCKWRITE,TO_LOCK) ;
	    psp->f.writelocked = (rs >= 0) ;
	} /* end if (we did not already have a lock on the file) */

	if (rs < 0)
	    goto ret0 ;

	rs = pingstatdb_checkcache(psp) ;
	if (rs < 0)
	    goto ret1 ;

	psp->now.time = daytime ;
	rs = dater_start(&ud,&psp->now,psp->zname,-1) ;
	if (rs < 0)
	    goto ret1 ;

	rs = dater_start(&cd,&psp->now,psp->zname,-1) ;
	if (rs < 0)
	    goto ret2 ;

	timestamp = up->timestamp ;
	if ((up->timestamp == 0) || (up->timestamp > daytime))
	    timestamp = daytime ;

#if	CF_DEBUGS
	{
	    char	timebuf[TIMEBUFLEN+1] ;
	    debugprintf("pingstatedb_uptime: now=%s\n",
	        timestr_logz(psp->now.time,timebuf)) ;
	    debugprintf("pingstatedb_uptime: dater_start() rs=%d cur_date=%s\n",
	        rs,timestr_logz(ud.b.time,timebuf)) ;
	}
#endif

#if	CF_DEBUGS
	{
	    debugprintf("pingstatedb_uptime: about to match\n") ;
	    rs = vecitem_count(&psp->entries) ;
	    debugprintf("pingstatedb_uptime: entries in cache %d\n",rs) ;
	}
#endif

	if ((rs = pingstatdb_getrec(psp,hostname,&rp)) >= 0) {
	    int	f_greater = (! LEQUIV(f_up,rp->f_up)) ;

#if	CF_DEBUGS
	    debugprintf("pingstatedb_uptime: found match rs=%d\n",rs) ;
#endif

	    rs = dater_gettime(&rp->pdate,&ptime) ;

	    f_greater = ((rs >= 0) && (timestamp > ptime)) ;

/* the update (ping) time */

	    if (ptime > timestamp)
	        timestamp = ptime ;

	    dater_settimezn(&ud,timestamp,psp->zname,-1) ;

/* the change time */

	    if (f_changed || (up->timechange != 0)) {

	        timechange = up->timechange ;
	        if (up->timechange == 0)
	            timechange = daytime ;

	        cdp = &cd ;
	        dater_settimezn(&cd,(time_t) timechange,psp->zname,-1) ;

	    } else {
	        cdp = &rp->cdate ;
	    }

/* force a change */

#if	CF_DEBUGS
	    debugprintf("pingstatedb_uptime: count=%d\n",
	        up->count) ;
#endif

	    if ((! f_changed) && (up->count != rp->count))
	        f_changed = TRUE ;

#if	CF_DEBUGS
	    {
	        char	timebuf[TIMEBUFLEN + 1] ;
	        debugprintf("pingstatedb_uptime: dater_gettime() ptime=%s\n",
	            timestr_logz(ptime,timebuf)) ;
	    }
#endif

	    if ((rs < 0) || f_changed ||
	        (((daytime - ptime) > TO_MINUPDATE) && f_greater)) {

	        boff = rp->roff ;
	        bseek(&psp->pfile,boff,SEEK_SET) ;

	        rs = record_write(rp,&psp->pfile,cdp,&ud,up->count,f_up) ;

#if	CF_DEBUGS
	        debugprintf("pingstatedb_uptime: record_write() rs=%d\n",rs) ;
#endif

	    } /* end if (did the update) */

	} else {
	    TIMEB	*nowp = &psp->now ;
	    int		f_rec = FALSE ;
	    cchar	*zn = psp->zname ;
	    cchar	*hn = hostname ;

#if	CF_DEBUGS
	    debugprintf("pingstatedb_uptime: no match found rs=%d\n",rs) ;
#endif

	    dater_settimezn(&ud,timestamp,psp->zname,-1) ;

	    if ((timechange = up->timechange) == 0)
	        timechange = daytime ;

	    dater_settimezn(&cd,(time_t) timechange,psp->zname,-1) ;

	    f_changed = TRUE ;
	    bseek(&psp->pfile,0L,SEEK_END) ;

	    btell(&psp->pfile,&boff) ;
	    roff = boff ;

	    if ((rs = record_start(&e,nowp,zn,roff,hn,&ud)) >= 0) {
	        f_rec = TRUE ;
	    }

#if	CF_DEBUGS
	    debugprintf("pingstatedb_uptime: record_start() rs=%d\n",rs) ;
	    debugprintf("pingstatedb_uptime: hostname=%s\n", hostname) ;
#endif

	    if (rs >= 0) {
	        rs = record_write(&e,&psp->pfile,&ud,&cd,up->count,f_up) ;
	    }

#if	CF_DEBUGS
	    debugprintf("pingstatedb_uptime: record_write() rs=%d\n",
	        rs) ;
#endif

	    if (rs >= 0) {
	        size = sizeof(PINGSTATDB_REC) ;
	        rs = vecitem_add(&psp->entries,&e,size) ;
	    }

	    if ((rs < 0) && f_rec) {
	        record_finish(&e) ;
	    }
	} /* end if (target entry) */

/* update the LASTUPDATE entry */

	if (rs >= 0) {
	    rs = pingstatdb_updrec(psp,daytime,&cd,LASTUPDATE,f_up,timestamp) ;
	}

/* udpate our last modification time to keep our cache current */

	psp->mtime = daytime ;
	bcontrol(&psp->pfile,BC_SYNC,0) ;

	dater_finish(&cd) ;

ret2:
	dater_finish(&ud) ;

/* unlock it */
ret1:
	psp->f.writelocked = FALSE ;
	bcontrol(&psp->pfile,BC_UNLOCK,0) ;

ret0:

#if	CF_DEBUGS
	debugprintf("pingstatedb_uptime: ret rs=%d f_changed=%u\n",
	    rs,f_changed) ;
#endif

	return (rs >= 0) ? f_changed : rs ;
}
/* end subroutine (pingstatdb_uptime) */


int pingstatdb_check(PINGSTATDB *psp,time_t daytime)
{
	int		rs = SR_OK ;

	if (psp == NULL) return SR_FAULT ;

	if (psp->magic != PINGSTATDB_MAGIC) return SR_NOTOPEN ;

	if (daytime == 0)
	    daytime = time(NULL) ;

	if (psp->f.readlocked) {
	    psp->f.readlocked = FALSE ;
	    psp->f.writelocked = FALSE ;
	    bcontrol(&psp->pfile,BC_UNLOCK,0) ;
	}

	if (! psp->f.writelocked) {
	    rs = bcontrol(&psp->pfile,BC_LOCKWRITE,TO_LOCK) ;
	    psp->f.writelocked = (rs >= 0) ;
	}

	if (rs >= 0) {
	    rs = pingstatdb_checkcache(psp) ;
	    psp->f.writelocked = FALSE ;
	    bcontrol(&psp->pfile,BC_UNLOCK,0) ;
	}

	return rs ;
}
/* end subroutine (pingstatdb_check) */


/* local subroutines */


/* check on the status of the file entry cache */
static int pingstatdb_checkcache(PINGSTATDB *psp)
{
	struct ustat	sb ;
	int		rs ;
	int		f_cached = psp->f.cached ;

#if	CF_DEBUGS
	debugprintf("pingstatdb_checkcache: ent f_cached=%d\n",
	    f_cached) ;
#endif

	if ((rs = bcontrol(&psp->pfile,BC_STAT,&sb)) >= 0) {
	    if (f_cached) {
	        if (sb.st_mtime > psp->mtime) {
	            f_cached = FALSE ;
	            pingstatdb_fes(psp) ;
	        }
	    } /* end if */
	    if (! f_cached) {
	        if ((rs = pingstatdb_readrecords(psp)) >= 0) {
	            psp->mtime = sb.st_mtime ;
	            psp->f.cached = TRUE ;
	        }
	    } else {
	        rs = vecitem_count(&psp->entries) ;
	    }
	} /* end if (bcontrol) */

#if	CF_DEBUGS
	debugprintf("pingstatdb_checkcache: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (pingstatdb_checkcache) */


/* read file entries */
static int pingstatdb_readrecords(PINGSTATDB *psp)
{
	int		rs ;
	int		c = 0 ;

#if	CF_DEBUGS
	debugprintf("pingstatdb_readrecords: ent\n") ;
#endif

	if ((rs = brewind(&psp->pfile)) >= 0) {
	    PINGSTATDB_REC	e ;
	    uint	roff = 0 ;
	    const int	size = sizeof(PINGSTATDB_REC) ;
	    const int	rlen = BUFLEN ;
	    int		line = 1 ;
	    int		len ;
	    int		bl ;
	    int		f_eol ;
	    int		f_bol = TRUE ;
	    char	rbuf[BUFLEN + 1] ;
	    while ((rs = breadline(&psp->pfile,rbuf,rlen)) > 0) {
	        len = rs ;

	        bl = (len - 1) ;
	        f_eol = (rbuf[bl] == '\n') ;

	        rbuf[bl] = '\0' ;
	        if (f_bol && (bl > RF_LEAD)) {
	            TIMEB	*nowp = &psp->now ;
	            cchar	*zn = psp->zname ;

#if	CF_DEBUGS
	            debugprintf("pingstatdb_readrecords: line=%u\n",line) ;
#endif

	            if ((rs = record_startbuf(&e,nowp,zn,roff,rbuf,bl)) >= 0) {
	                c += 1 ;
	                rs = vecitem_add(&psp->entries,&e,size) ;
	                if (rs < 0)
	                    record_finish(&e) ;
	            } /* end if (record_startbuf) */

	        } /* end if (a live one) */

	        roff += len ;
	        line += 1 ;
	        f_bol = f_eol ;
	        if (rs < 0) break ;
	    } /* end while (reading file records) */
	} /* end if (brewind) */

#if	CF_DEBUGS
	debugprintf("pingstatdb_readrecords: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (pingstatdb_readrecords) */


/* free up the entries in the cache */
static int pingstatdb_fes(PINGSTATDB *psp)
{
	PINGSTATDB_REC	*ep ;
	VECITEM		*elp = &psp->entries ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		i ;

/* delete for an uncompacted vector */

	for (i = 0 ; vecitem_get(elp,i,&ep) >= 0 ; i += 1) {
	    if (ep != NULL) {
	        record_finish(ep) ;
	        vecitem_del(&psp->entries,i) ;
	    }
	} /* end for */

/* delete for a compacted vector */

	i = 0 ;
	while ((rs1 = vecitem_get(elp,i,&ep)) >= 0) {
	    if (ep != NULL) {
	        record_finish(ep) ;
	        vecitem_del(elp,i) ;
	    } else {
	        i += 1 ;
	    }
	} /* end while */
	if ((rs >= 0) && (rs1 != SR_NOTFOUND)) rs = rs1 ;

#if	CF_DEBUGS
	debugprintf("pingstatdb_fes: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (pingstatdb_fes) */


static int pingstatdb_updrec(psp,dt,dp,hostname,f_up,timestamp)
PINGSTATDB	*psp ;
time_t		dt ;
DATER		*dp ;
const char	hostname[] ;
int		f_up ;
time_t		timestamp ;
{
	PINGSTATDB_REC	*rp ;
	offset_t	boff ;
	int		rs ;
	int		f_changed = FALSE ;

#if	CF_DEBUGS
	{
	    char	timebuf[TIMEBUFLEN+1] ;
	    debugprintf("pingstatdb_updrec: hostname=%s\n",hostname) ;
	    debugprintf("pingstatdb_updrec: f_up=%u\n",f_up) ;
	    debugprintf("pingstatdb_updrec: timestamp=%s\n",
	        timestr_logz(timestamp,timebuf)) ;
	}
#endif /* CF_DEBUGS */

	if ((rs = pingstatdb_getrec(psp,hostname,&rp)) >= 0) {
	    time_t	ptime ;
	    int		f_greater = (! LEQUIV(f_up,rp->f_up)) ;

#if	CF_DEBUGS
	    debugprintf("pingstatdb_updrec: found match rs=%d\n",rs) ;
#endif

	    rs = dater_gettime(&rp->pdate,&ptime) ;

	    f_greater = ((rs >= 0) && (timestamp > ptime)) ;

	    if (ptime > timestamp)
	        timestamp = ptime ;

	    dater_settimezn(dp,timestamp,psp->zname,-1) ;

#if	CF_DEBUGS
	    {
	        char	timebuf[TIMEBUFLEN + 1] ;
	        debugprintf("pingstatdb_updrec: dater_gettime() ptime=%s\n",
	            timestr_logz(ptime,timebuf)) ;
	    }
#endif

	    if ((rs < 0) || f_changed ||
	        (((dt - ptime) > TO_MINUPDATE) && f_greater)) {

	        boff = rp->roff ;
	        bseek(&psp->pfile,boff,SEEK_SET) ;

	        rs = record_update(rp,&psp->pfile,dp,f_up) ;

#if	CF_DEBUGS
	        debugprintf("pingstatdb_updrec: record_updrec() rs=%d\n",rs) ;
#endif

	    } /* end if (did the update) */

	} else if (rs == SR_NOTFOUND) {
	    PINGSTATDB_REC	r ;
	    uint	roff ;
	    const int	size = sizeof(PINGSTATDB_REC) ;
	    int		f_rec = FALSE ;

#if	CF_DEBUGS
	    debugprintf("pingstatdb_updrec: no match found rs=%d\n",rs) ;
	    debugprintf("pingstatdb_updrec: zname=%s\n",psp->zname) ;
#endif

	    if ((rs = dater_settimezn(dp,timestamp,psp->zname,-1)) >= 0) {
		TIMEB	*nowp = &psp->now ;
		cchar	*zn = psp->zname ;
		cchar	*hn = hostname ;

	        f_changed = TRUE ;
	        bseek(&psp->pfile,0L,SEEK_END) ;

	        btell(&psp->pfile,&boff) ;
	        roff = boff ;

	        if ((rs = record_start(&r,nowp,zn,roff,hn,dp)) >= 0) {
	            f_rec = TRUE ;
		}

#if	CF_DEBUGS
	        debugprintf("pingstatdb_updrec: record_start() rs=%d hn=%s\n",
	            rs,hostname) ;
#endif

	    } /* end if */

	    if (rs >= 0) {
	        rs = record_update(&r,&psp->pfile,dp,f_up) ;

#if	CF_DEBUGS
	        debugprintf("pingstatdb_updrec: record_update() rs=%d\n",
	            rs) ;
#endif

	    } /* end if */

	    if (rs >= 0) {
	        rs = vecitem_add(&psp->entries,&r,size) ;
	    }

	    if ((rs < 0) && f_rec) {
	        record_finish(&r) ;
	    }
	} /* end if (target entry) */

#if	CF_DEBUGS
	debugprintf("pingstatdb_updrec: ret rs=%d f_changed=%u\n",
	    rs,f_changed) ;
#endif

	return (rs >= 0) ? f_changed : rs ;
}
/* end subroutine (pingstatdb_updrec) */


static int pingstatdb_getrec(PINGSTATDB *psp,cchar *hostname,
		PINGSTATDB_REC **rpp)
{
	int		rs ;
	int		i ;

	for (i = 0 ; (rs = vecitem_get(&psp->entries,i,rpp)) >= 0 ; i += 1) {
	    if (*rpp == NULL) continue ;
	    if (strcmp(hostname,(*rpp)->hostname) == 0) break ;
	} /* end for */

	return (rs >= 0) ? i : rs ;
}
/* end subroutine (pingstatdb_getrecord) */


/* initialize a fresh entry */
/* ARGSUSED */
static int record_start(ep,nowp,zname,roff,hostname,dp)
PINGSTATDB_REC	*ep ;
TIMEB		*nowp ;
const char	zname[] ;
uint		roff ;
const char	hostname[] ;
DATER		*dp ;
{
	int		rs ;
	int		hl ;

	if (ep == NULL) return SR_FAULT ;
	if (zname == NULL) return SR_FAULT ;
	if (hostname == NULL) return SR_FAULT ;
	if (dp == NULL) return SR_FAULT ;

#if	CF_DEBUGS
	debugprintf("record_start: hostname=%s\n",hostname) ;
#endif

#if	CF_DEBUGS
	{
	    char	timebuf[TIMEBUFLEN + 1] ;
	    debugprintf("record_start: given time=%s\n",
	        timestr_logz(dp->b.time,timebuf)) ;
	}
#endif

	memset(ep,0,sizeof(PINGSTATDB_REC)) ;
	ep->roff = roff ;
	ep->count = 1 ;
	ep->f_up = FALSE ;

	if ((rs = dater_startcopy(&ep->cdate,dp)) >= 0) {
	    if ((rs = dater_startcopy(&ep->pdate,dp)) >= 0) {
	        hl = strlen(hostname) ;
	        ep->hostlen = hl ;
	        rs = uc_mallocstrw(hostname,hl,&ep->hostname) ;
	        if (rs < 0)
	            dater_finish(&ep->pdate) ;
	    } /* end if (dater_startcopy) */
	    if (rs < 0)
	        dater_finish(&ep->cdate) ;
	} /* end if (dater_startcopy) */

	return rs ;
}
/* end subroutine (record_start) */


/* initialize an entry from a buffer (w/ 'logz' string) */
static int record_startbuf(ep,nowp,zname,roff,buf,buflen)
PINGSTATDB_REC	*ep ;
TIMEB		*nowp ;
const char	zname[] ;
uint		roff ;
const char	buf[] ;
int		buflen ;
{
	int		rs ;
	int		bl = buflen ;
	const char	*bp = buf ;

	if (ep == NULL) return SR_FAULT ;
	if (zname == NULL) return SR_FAULT ;
	if (buf == NULL) return SR_FAULT ;

#if	CF_DEBUGS
	debugprintf("record_startbuf: 1 name=%s\n",hostname) ;
#endif

	memset(ep,0,sizeof(PINGSTATDB_REC)) ;

#if	CF_DEBUGS
	debugprintf("record_startbuf: 2 name=%s\n",hostname) ;
#endif

	rs = dater_start(&ep->cdate,nowp,zname,-1) ;
	if (rs < 0)
	    goto bad0 ;

	rs = dater_start(&ep->pdate,nowp,zname,-1) ;
	if (rs < 0)
	    goto bad1 ;

#if	CF_DEBUGS
	debugprintf("record_startbuf: 3 name=%s\n",hostname) ;
#endif

	ep->roff = roff ;
	ep->len = buflen ;
	rs = cfdeci(bp,RF_NUMDIGITS,&ep->count) ;
	if (rs < 0)
	    goto bad2 ;

	bp += (RF_NUMDIGITS + 1) ;
	bl -= (RF_NUMDIGITS + 1) ;
	rs = dater_setlogz(&ep->cdate,bp,bl) ;
	if (rs < 0)
	    goto bad2 ;

	bp += (RF_LOGZLEN + 1) ;
	bl -= (RF_LOGZLEN + 1) ;
	rs = dater_setlogz(&ep->pdate,bp,bl) ;
	if (rs < 0)
	    goto bad2 ;

	bp += (RF_LOGZLEN + 1) ;
	bl -= (RF_LOGZLEN + 1) ;
	if (bl <= 0) {
	    rs = SR_INVALID ;
	    goto bad2 ;
	}

#if	CF_DEBUGS
	debugprintf("record_startbuf: 4 name=%s\n",hostname) ;
#endif

	ep->f_up = (toupper(*bp) == 'U') ;

	bp += (RF_UPSTAT + 1) ;
	bl -= (RF_UPSTAT + 1) ;
	if (bl <= 0) {
	    rs = SR_INVALID ;
	    goto bad2 ;
	}

#if	CF_DEBUGS
	debugprintf("record_startbuf: 4a name=%s\n",hostname) ;
	debugprintf("record_startbuf: bl=%u ext_name=%t\n",bl,bp,bl) ;
#endif

	ep->hostlen = bl ;
	rs = uc_mallocstrw(bp,bl,&ep->hostname) ;
	if (rs < 0)
	    goto bad2 ;

#if	CF_DEBUGS
	debugprintf("record_startbuf: 5 name=%s\n",hostname) ;
#endif

ret0:
	return rs ;

/* bad stuff */
bad2:
	dater_finish(&ep->pdate) ;

bad1:
	dater_finish(&ep->cdate) ;

bad0:
	goto ret0 ;
}
/* end subroutine (record_startbuf) */


/* update this entry to the file */
static int record_update(ep,fp,dp,f_up)
PINGSTATDB_REC	*ep ;
bfile		*fp ;
DATER		*dp ;
int		f_up ;
{
	int		rs = SR_OK ;
	char		cdate[RF_LOGZLEN + 2] ;
	char		pdate[RF_LOGZLEN + 2] ;

	if (ep == NULL) return SR_FAULT ;

#if	CF_DEBUGS
	{
	    char	timebuf[TIMEBUFLEN + 1] ;
	    debugprintf("record_update: ent, host=%s\n",ep->hostname) ;
	    debugprintf("record_update: cur_date=%s\n",
	        timestr_log(dp->b.time,timebuf)) ;
	}
#endif

	if (! LEQUIV(ep->f_up,f_up)) {

#if	CF_DEBUGS
	    debugprintf("record_update: changed status\n") ;
#endif

	    ep->f_up = f_up ;
	    ep->count = 1 ;
	    dater_setcopy(&ep->cdate,dp) ;

#if	CF_DEBUGS
	    {
	        char	timebuf[TIMEBUFLEN + 1] ;
	        debugprintf("record_update: status change cdate=%s\n",
	            timestr_logz(ep->cdate.b.time,timebuf)) ;
	    }
#endif

	} else {
	    ep->count += 1 ;
	}

#if	CF_DEBUGS
	{
	    char	timebuf[TIMEBUFLEN + 1] ;
	    debugprintf("record_update: cdate=%s\n",
	        timestr_logz(ep->cdate.b.time,timebuf)) ;
	}
#endif

	dater_mklogz(&ep->cdate,cdate,(RF_LOGZLEN + 1)) ;

#if	CF_DEBUGS
	debugprintf("record_update: cdate mklogz=%s\n",
	    cdate) ;
#endif

/* always update the last-update-date for the record */

	dater_setcopy(&ep->pdate,dp) ;

#if	CF_DEBUGS
	{
	    char	timebuf[TIMEBUFLEN + 1] ;
	    debugprintf("record_update: pdate=%s\n",
	        timestr_logz(ep->pdate.b.time,timebuf)) ;
	}
#endif

	dater_mklogz(&ep->pdate,pdate,RF_LOGZLEN + 1) ;

#if	CF_DEBUGS
	{
	    debugprintf("record_update: pdate mklogz=%s\n",
	        pdate) ;
	}
#endif

	rs = bprintf(fp,"%*d %-*s %-*s %c %s\n",
	    RF_NUMDIGITS,ep->count,
	    RF_LOGZLEN,cdate,
	    RF_LOGZLEN,pdate,
	    ((ep->f_up) ? 'U' : 'D'),
	    ep->hostname) ;

	return rs ;
}
/* end subroutine (record_update) */


/* write out this entry to the file */
static int record_write(ep,fp,cp,dp,count,f_up)
PINGSTATDB_REC	*ep ;
bfile		*fp ;
DATER		*cp ;
DATER		*dp ;
int		count ;
int		f_up ;
{
	int		rs = SR_OK ;
	char		cdate[RF_LOGZLEN + 2] ;
	char		pdate[RF_LOGZLEN + 2] ;

	if (ep == NULL) return SR_FAULT ;

#if	CF_DEBUGS
	{
	    char	timebuf[TIMEBUFLEN + 1] ;
	    debugprintf("record_write: ent host=%s\n",ep->hostname) ;
	    debugprintf("record_write: count=%d\n",count) ;
	    debugprintf("record_write: cur_date=%s\n",
	        timestr_log(dp->b.time,timebuf)) ;
	}
#endif /* CF_DEBUGS */

	ep->f_up = f_up ;
	if (count != 0) {
	    ep->count = count ;
	} else {
	    ep->count += 1 ;
	}

/* the "change" date */

	dater_setcopy(&ep->cdate,cp) ;

	dater_mklogz(&ep->cdate,cdate,(RF_LOGZLEN + 1)) ;

/* always update the last-update-date for the record */

	dater_setcopy(&ep->pdate,dp) ;

	dater_mklogz(&ep->pdate,pdate,RF_LOGZLEN + 1) ;

/* pop it */

	rs = bprintf(fp,"%*d %-*s %-*s %c %s\n",
	    RF_NUMDIGITS,ep->count,
	    RF_LOGZLEN,cdate,
	    RF_LOGZLEN,pdate,
	    ((ep->f_up) ? 'U' : 'D'),
	    ep->hostname) ;

	return rs ;
}
/* end subroutine (record_write) */


/* free up an entry */
static int record_finish(PINGSTATDB_REC *ep)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (ep == NULL) return SR_FAULT ;

	if (ep->hostname != NULL) {
	    rs1 = uc_free(ep->hostname) ;
	    if (rs >= 0) rs = rs1 ;
	    ep->hostname = NULL ;
	} /* end if */

	rs1 = dater_finish(&ep->cdate) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = dater_finish(&ep->pdate) ;
	if (rs >= 0) rs = rs1 ;

	return rs ;
}
/* end subroutine (record_finish) */


static int entry_load(PINGSTATDB_ENT *ep,PINGSTATDB_REC *rp)
{
	int		rs = SR_OK ;
	int		hl = 0 ;

	if (ep == NULL) return SR_FAULT ;
	if (rp == NULL) return SR_FAULT ;

	ep->ti_change = 0 ;
	ep->ti_ping  = 0 ;
	ep->count = 0 ;
	ep->f_up = 0 ;
	ep->hostname[0] = '\0' ;

	if (rs >= 0)
	    rs = dater_gettime(&rp->pdate,&ep->ti_ping) ;

	if (rs >= 0)
	    rs = dater_gettime(&rp->cdate,&ep->ti_change) ;

	if (rs >= 0) {
	    ep->count = rp->count ;
	    ep->f_up = rp->f_up ;
	    rs = mkpath1(ep->hostname,rp->hostname) ;
	    hl = rs ;
	}

	return (rs >= 0) ? hl : rs ;
}
/* end subroutine (entry_load) */


/* make the open mode string for BIO, remember O_RDONLY is the fake */
int mkbstr(mode_t omode,char *ostr)
{
	char		*bp = ostr ;

#if	CF_DEBUGS
	int		f_read = TRUE ;
#endif

#if	CF_DEBUGS
	debugprintf("pingstatdb/mkbstr: omode=%04o\n",omode) ;
#endif

	if ((omode & O_RDWR) || (omode & O_WRONLY)) {

	    *bp++ = 'w' ;
#if	CF_DEBUGS
	    if ((omode & O_WRONLY) && (! (omode & O_RDWR)))
	        f_read = FALSE ;
#endif

	}

#if	CF_DEBUGS
	debugprintf("pingstatdb/mkbstr: f_read=%d\n",f_read) ;
#endif

	if (omode & O_APPEND)
	    *bp++ = 'a' ;

	if (omode & O_CREAT)
	    *bp++ = 'c' ;

	if (omode & O_TRUNC)
	    *bp++ = 't' ;

#ifdef	COMMENT
	if (f_read)
	    *bp++ = 'r' ;
#else
	*bp++ = 'r' ;		/* ALWAYS need for locking */
#endif /* COMMENT */

	*bp = '\0' ;
	return (bp - ostr) ;
}
/* end subroutine (mkbstr) */


