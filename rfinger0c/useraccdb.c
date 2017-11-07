/* useraccdb */

/* user-access (user-access-logging) data-base management */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This object implements a small database for storing the last time a user
        accesses something (usually a program).

	Format of file database records:

	- eight digits of a decimal count number field
	- the time of the last entry update
	- the nodename and username
	- an optional real or fullname of the user

	Format example:

0         1         2         3         4
01234567890123456789012345678901234567890
     989 111115_1137:06_EST      TOTAL
     775 111115_1137:06_EST      rca!local (LOCAL)
     200 111001_2015:04_EDT      rca!dam (David A­D­ Morano)
      14 110103_0922:32_EST      rca!genserv (GENSERV)


        Note that a special user field with the value of "TOTAL" maintains a
        total count of all program invocations separately from the user counts.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<sys/timeb.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>
#include	<time.h>

#include	<vsystem.h>
#include	<estrings.h>
#include	<ascii.h>
#include	<filebuf.h>
#include	<storeitem.h>
#include	<dater.h>
#include	<localmisc.h>

#include	"useraccdb.h"


/* local defines */

#define	USERACCDB_LOGDNAME	"var/log"
#define	USERACCDB_MAGIC		0x11359299
#define	USERACCDB_INTCHECK	5
#define	USERACCDB_REC		struct useraccdb_rec
#define	USERACCDB_ITEM		struct useraccdb_item

#define	UAFILE_SUF		"users"
#define	UAFILE_LCOUNT		8
#define	UAFILE_LDATE		(32-9)
#define	UAFILE_MAXUSERLEN	128
#define	UAFILE_MAXNAMELEN	128
#define	UAFILE_RECLEN		\
	(UAFILE_LCOUNT + 1 + UAFILE_LDATE + 1 + \
	UAFILE_MAXUSERLEN + 2 + UAFILE_MAXNAMELEN + 2) 

#define	UPINFO			struct upinfo
#define	UPINFO_REC		struct upinfo_rec

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

#define	BUFLEN		(MAXPATHLEN + USERNAMELEN + 3)

#define	TO_CHECK	5		/* check interval */
#define	TO_LOCK		4		/* time-out */

#define	TOTALNAME	"TOTAL"

#ifndef	TIMEBUFLEN
#define	TIMEBUFLEN	80
#endif


/* external subroutines */

extern int	snsds(char *,int,const char *,const char *) ;
extern int	sncpy3(char *,int,const char *,const char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecui(const char *,int,uint *) ;
extern int	ctdecui(char *,int,uint) ;
extern int	ndigits(int,int) ;
extern int	strwcmp(const char *,const char *,int) ;
extern int	initnow(struct timeb *,const char *,int) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*strdcpy3(char *,int,const char *,const char *,const char *) ;
extern char	*strwcpy(char *,const char *,int) ;
extern char	*strwset(char *,int,int) ;
extern char	*strnchr(const char *,int,int) ;
extern char	*strnset(char *,int,int) ;
extern char	*timestr_logz(time_t,char *) ;


/* external variables */


/* local structures */

struct upinfo_rec {
	offset_t	ro ;		/* record offset */
	uint		count ;
	uint		found:1 ;
} ;

struct upinfo {
	UPINFO_REC	user, total ;
	USERACCDB	*op ;
	const char	*arguser ;
	const char	*argname ;
	time_t		utime ;
	char		tbuf[UAFILE_LDATE+1] ;
} ;

struct useraccdb_item {
	const char	*sp ;
	int		sl ;
} ;

struct useraccdb_rec {
	USERACCDB_ITEM	countstr ;
	USERACCDB_ITEM	datestr ;
	USERACCDB_ITEM	userstr ;
	USERACCDB_ITEM	namestr ;
	time_t		atime ;
	uint		count ;
} ;


/* forward references */

static int	useraccdb_fileopen(USERACCDB *) ;
static int	useraccdb_openlock(USERACCDB *) ;
static int	useraccdb_fileclose(USERACCDB *) ;
static int	useraccdb_lock(USERACCDB *,int) ;
static int	useraccdb_recparse(USERACCDB *,USERACCDB_REC *,cchar *,int) ;
static int	useraccdb_recproc(USERACCDB *,USERACCDB_REC *) ;
static int	useraccdb_datethis(USERACCDB *,time_t *,const char *,int) ;

static int	upinfo_start(UPINFO *,USERACCDB *,const char *,const char *) ;
static int	upinfo_match(UPINFO *,offset_t,USERACCDB_REC *) ;
static int	upinfo_update(UPINFO *) ;
static int	upinfo_finish(UPINFO *) ;
static int	upinfo_upone(UPINFO *,UPINFO_REC *,int) ;
static int	upinfo_mkrec(UPINFO *,UPINFO_REC *,char *,int,int) ;

static int	entry_load(USERACCDB_ENT *,char *,int,USERACCDB_REC *) ;

static int	mkts(char *,int,time_t) ;


/* local variables */

static const char	*totaluser = TOTALNAME ;


/* exported subroutines */


int useraccdb_open(USERACCDB *op,cchar *pr,cchar *dbname)
{
	int		rs ;
	const char	*logdname = USERACCDB_LOGDNAME ;
	char		cname[MAXNAMELEN+1] ;

	if (op == NULL) return SR_FAULT ;
	if (pr == NULL) return SR_FAULT ;
	if (dbname == NULL) return SR_FAULT ;

	if (pr[0] == '\0') return SR_INVALID ;
	if (dbname[0] == '\0') return SR_INVALID ;

	memset(op,0,sizeof(USERACCDB)) ;
	op->eo = -1 ;
	op->fd = -1 ;

	if ((rs = snsds(cname,MAXNAMELEN,dbname,UAFILE_SUF)) >= 0) {
	    char	fname[MAXPATHLEN+1] ;
	    if ((rs = mkpath3(fname,pr,logdname,cname)) >= 0) {
		const int	pl = rs ;
		const char	*cp ;
		if ((rs = uc_mallocstrw(fname,pl,&cp)) >= 0) {
		    op->fname = cp ;
		    if ((rs = useraccdb_fileopen(op)) >= 0) {
			op->magic = USERACCDB_MAGIC ;
		    }
		    if (rs < 0) {
	    		uc_free(op->fname) ;
	    		op->fname = NULL ;
		    }
		} /* end if (memory-allocation) */
	    } /* end if (mkpath) */
	} /* end if (make-component) */

#if	CF_DEBUGS
	debugprintf("useraccdb_open: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (useraccdb_open) */


int useraccdb_close(USERACCDB *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != USERACCDB_MAGIC) return SR_NOTOPEN ;

	if (op->f.dater) {
	    op->f.dater = FALSE ;
	    rs1 = dater_finish(&op->dm) ;
	    if (rs >= 0) rs = rs1 ;
	}

	rs1 = useraccdb_fileclose(op) ;
	if (rs >= 0) rs = rs1 ;

	if (op->fname != NULL) {
	    rs1 = uc_free(op->fname) ;
	    if (rs >= 0) rs = rs1 ;
	    op->fname = NULL ;
	}

	return rs ;
}
/* end subroutine (useraccdb_close) */


int useraccdb_find(USERACCDB *op,USERACCDB_ENT *ep,char *ebuf,int elen,
		cchar *user)
{
	FILEBUF		b ;
	int		rs ;
	int		rs1 ;

	if (op == NULL) return SR_FAULT ;
	if (ep == NULL) return SR_FAULT ;
	if (ebuf == NULL) return SR_FAULT ;
	if (user == NULL) return SR_FAULT ;

	if (op->magic != USERACCDB_MAGIC) return SR_NOTOPEN ;

	if (op->eo >= 0) return SR_INVALID ;

	if ((rs = useraccdb_openlock(op)) >= 0) {

	    if ((rs = filebuf_start(&b,op->fd,0L,0,0)) >= 0) {
	        USERACCDB_REC	rec ;
	        const int	llen = LINEBUFLEN ;
	        int		ll, sl ;
	        const char	*sp ;
	        char		lbuf[LINEBUFLEN+1] ;

	        while ((rs1 = filebuf_readline(&b,lbuf,llen,-1)) > 0) {
	            ll = rs ;
	            rs = useraccdb_recparse(op,&rec,lbuf,ll) ;
	            if (rs >= 0) {
	                sp = rec.userstr.sp ;
	                sl = rec.userstr.sl ;
	                if (strwcmp(user,sp,sl) == 0) {
	                    rs = useraccdb_recproc(op,&rec) ;
	                    if (rs >= 0)
	                        rs = entry_load(ep,ebuf,elen,&rec) ;
	                    break ;
	                }
	            }
	            if (rs < 0) break ;
	        } /* end while (reading lines) */
	        if ((rs >= 0) && (rs1 <= 0)) {
	            rs = (rs1 < 0) ? rs1 : SR_NOTFOUND ;
	        }

	        filebuf_finish(&b) ;
	    } /* end if (filebuf) */

	    rs1 = useraccdb_lock(op,FALSE) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (lock) */

	return rs ;
}
/* end subroutine (useraccdb_find) */


int useraccdb_update(USERACCDB *op,cchar *user,cchar *name)
{
	UPINFO		ui ;
	int		rs ;
	int		rs1 ;

	if (op == NULL) return SR_FAULT ;
	if (user == NULL) return SR_FAULT ;

	if (op->magic != USERACCDB_MAGIC) return SR_NOTOPEN ;

	if (op->eo >= 0) return SR_INVALID ;

#if	CF_DEBUGS
	debugprintf("useraccdb_update: user=%s\n",user) ;
	debugprintf("useraccdb_update: name=>%s<\n",name) ;
#endif

	if ((rs = upinfo_start(&ui,op,user,name)) >= 0) {

#if	CF_DEBUGS
	debugprintf("useraccdb_update: upinfo_start() rs=%d\n",rs) ;
#endif
	    if ((rs = useraccdb_openlock(op)) >= 0) {
		FILEBUF	b ;

#if	CF_DEBUGS
	debugprintf("useraccdb_update: _openlock() rs=%d\n",rs) ;
#endif
	        if ((rs = filebuf_start(&b,op->fd,0L,0,0)) >= 0) {
	            USERACCDB_REC	rec ;
	            offset_t	ro = 0L ;
	            const int	llen = LINEBUFLEN ;
		    int		ll ;
	            char	lbuf[LINEBUFLEN+1] ;

#if	CF_DEBUGS
	debugprintf("useraccdb_update: filebuf_start() rs=%d\n",rs) ;
#endif
	            while ((rs = filebuf_readline(&b,lbuf,llen,-1)) > 0) {
	                ll = rs ;
#if	CF_DEBUGS
	                debugprintf("useraccdb_update: ll=%u\n",ll) ;
	                debugprintf("useraccdb_update: line=>%t<\n",
				lbuf,strlinelen(lbuf,ll,60)) ;
#endif
	                rs = useraccdb_recparse(op,&rec,lbuf,ll) ;
#if	CF_DEBUGS
	                debugprintf("useraccdb_update: _recparse() rs=%d\n",
				rs) ;
#endif

	                if (rs >= 0) {
	                    rs = upinfo_match(&ui,ro,&rec) ;
#if	CF_DEBUGS
	                debugprintf("useraccdb_update: "
				"upinfo_match() rs=%d\n",
				rs) ;
#endif

	                    if (rs > 0) break ;
	                }
	                if (rs < 0) break ;
	                ro += ll ;
	            } /* end while (reading lines) */

#if	CF_DEBUGS
	            debugprintf("useraccdb_update: while-end rs=%d\n",rs) ;
#endif

	            filebuf_finish(&b) ;
	        } /* end if (filebuf) */

#if	CF_DEBUGS
	            debugprintf("useraccdb_update: filebuf-end rs=%d\n",rs) ;
#endif

	        if (rs >= 0) {
	            rs1 = upinfo_update(&ui) ;
	            if (rs >= 0) rs = rs1 ;
#if	CF_DEBUGS
	            debugprintf("useraccdb_update: upinfo_update() rs=%d\n",
			rs1) ;
#endif

	        }

	        rs1 = useraccdb_lock(op,FALSE) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (lock) */

	    rs1 = upinfo_finish(&ui) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (upinfo) */

#if	CF_DEBUGS
	debugprintf("useraccdb_update: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (useraccdb_update) */


int useraccdb_curbegin(USERACCDB *op,USERACCDB_CUR *curp)
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;

	if (op->magic != USERACCDB_MAGIC) return SR_NOTOPEN ;

	memset(curp,0,sizeof(USERACCDB_CUR)) ;
	curp->eo = -1 ;
	op->eo = 0L ;

	rs = useraccdb_openlock(op) ;

	return rs ;
}
/* end subroutine (useraccdb_curbegin) */


int useraccdb_curend(USERACCDB *op,USERACCDB_CUR *curp)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;

	if (op->magic != USERACCDB_MAGIC) return SR_NOTOPEN ;

	if (curp->eo >= 0) {
	    curp->eo = -1 ;
	    rs1 = filebuf_finish(&curp->b) ;
	    if (rs >= 0) rs = rs1 ;
	}

	rs1 = useraccdb_lock(op,FALSE) ;
	if (rs >= 0) rs = rs1 ;

	op->eo = -1 ;
	return rs ;
}
/* end subroutine (useraccdb_curend) */


int useraccdb_enum(op,curp,ep,ebuf,elen)
USERACCDB	*op ;
USERACCDB_CUR	*curp ;
USERACCDB_ENT	*ep ;
char		ebuf[] ;
int		elen ;
{
	int		rs = SR_OK ;
	int		ll ;

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;
	if (ep == NULL) return SR_FAULT ;
	if (ebuf == NULL) return SR_FAULT ;

	if (op->magic != USERACCDB_MAGIC) return SR_NOTOPEN ;

	if (op->fd <= 0) return SR_INVALID ;

	if (curp->eo < 0) {
	    rs = filebuf_start(&curp->b,op->fd,0L,0,0) ;
	    if (rs >= 0) curp->eo = 0L ;
	}

	if (rs >= 0) {
	    const int	llen = LINEBUFLEN ;
	    char	lbuf[LINEBUFLEN+1] ;
	    offset_t	eo = curp->eo ;
	    if ((rs = filebuf_readline(&curp->b,lbuf,llen,-1)) >= 0) {
		ll = rs ;
	        if (ll > 0) {
	            USERACCDB_REC	rec ;
	             rs = useraccdb_recparse(op,&rec,lbuf,ll) ;
	             if (rs >= 0)
	                 rs = useraccdb_recproc(op,&rec) ;
	             if (rs >= 0)
	                 rs = entry_load(ep,ebuf,elen,&rec) ;
	        } else 
	            rs = SR_NOTFOUND ;
	        if (rs >= 0)
	            curp->eo = (eo + ll) ;
	    } /* end if (filebuf_readline) */
	} /* end if (ok) */

	return rs ;
}
/* end subroutine (useraccdb_enum) */


int useraccdb_check(USERACCDB *op,time_t ti_now)
{
	int		rs = SR_OK ;
	int		f_changed = FALSE ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != USERACCDB_MAGIC) return SR_NOTOPEN ;

	if (ti_now == 0) ti_now = time(NULL) ;

	if (ti_now == 1) f_changed = TRUE ; /* fake out LINT */

	return (rs >= 0) ? f_changed : rs ;
}
/* end subroutine (useraccdb_check) */


/* private subroutines */


static int useraccdb_openlock(USERACCDB *op)
{
	int		rs = SR_OK ;

	if (op->fd < 0) {
	    rs = useraccdb_fileopen(op) ;
	}

	if (rs >= 0) {
	    rs = useraccdb_lock(op,TRUE) ;
	}

	return rs ;
}
/* end subroutine (useraccdb_openlock) */


static int useraccdb_fileopen(USERACCDB *op)
{
	int		rs = SR_OK ;

#if	CF_DEBUGS
	debugprintf("useraccdb_fileopen: fname=%s\n",op->fname) ;
#endif

	if (op->fd < 0) {
	    const mode_t	om = 0666 ;
	    const int		of = (O_RDWR | O_CREAT) ;
	    if ((rs = uc_open(op->fname,of,om)) >= 0) {
	        op->fd = rs ;
#if	CF_DEBUGS
	debugprintf("useraccdb_fileopen: open() rs=%d\n",rs) ;
#endif
	        if ((rs = uc_closeonexec(op->fd,TRUE)) >= 0) {
		    struct ustat	sb ;
		    if ((rs = u_fstat(op->fd,&sb)) >= 0) {
		        op->dev = sb.st_dev ;
		        op->ino = sb.st_ino ;
		        op->ti_mod = sb.st_mtime ;
		    }
	        } /* end if (close-on-exec) */
	        if (rs < 0) {
		    u_close(op->fd) ;
		    op->fd = -1 ;
	        }
	    } /* end if (open) */
	} /* end if (open was needed) */

#if	CF_DEBUGS
	debugprintf("useraccdb_fileopen: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (useraccdb_fileopen) */


static int useraccdb_fileclose(USERACCDB *op)
{
	int		rs = SR_OK ;

	if (op->fd >= 0) {
	    rs = u_close(op->fd) ;
	    op->fd = -1 ;
	}

	return rs ;
}
/* end subroutine (useraccdb_fileclose) */


static int useraccdb_lock(USERACCDB *op,int f)
{
	int		rs = SR_OK ;

	if (f) {
	    if (! op->f.locked) {
	        rs = u_rewind(op->fd) ;
	        if (rs >= 0) {
	            rs = uc_lockf(op->fd,F_LOCK,0L) ;
	            if (rs >= 0) op->f.locked = TRUE ;
	        }
	    }
	} else {
	    if (op->f.locked) {
	        rs = u_rewind(op->fd) ;
	        if (rs >= 0) {
	            op->f.locked = FALSE ;
	            rs = uc_lockf(op->fd,F_UNLOCK,0L) ;
	        }
	    }
	} /* end if */

	return rs ;
}
/* end subroutine (useraccdb_lock) */


static int useraccdb_recparse(USERACCDB *op,USERACCDB_REC *recp,
		cchar *lp,int ll)
{
	int		rs = SR_OK ;
	int		cl ;
	const char	*tp, *cp ;

#if	CF_DEBUGS
	debugprintf("useraccdb_recparse: line=>%t<\n",
		lp,strlinelen(lp,ll,40)) ;
#endif

	if (op == NULL) return SR_FAULT ;

	memset(recp,0,sizeof(USERACCDB_REC)) ;

#ifdef	COMMENT
	cl = nextfield(lp,ll,&cp) ;
	recp->countstr.sp = cp ;
	recp->countstr.sl = cl ;
	ll = ((lp+ll)-(cp+cl)) ;
	lp = (cp+cl) ;
	cl = nextfield(lp,ll,&cp) ;
	recp->datestr.sp = cp ;
	recp->datestr.sl = cl ;
	ll = ((lp+ll)-(cp+cl)) ;
	lp = (cp+cl) ;
#else
	recp->countstr.sp = lp ;
	recp->countstr.sl = UAFILE_LCOUNT ;
	lp += (UAFILE_LCOUNT + 1) ;
	ll -= (UAFILE_LCOUNT + 1) ;
	recp->datestr.sp = lp ;
	recp->datestr.sl = UAFILE_LDATE ;
	lp += (UAFILE_LDATE + 1) ;
	ll -= (UAFILE_LDATE + 1) ;
#endif /* COMMENT */

#if	CF_DEBUGS
	debugprintf("useraccdb_recparse: count=>%t<\n",
		recp->countstr.sp, recp->countstr.sl) ;
	debugprintf("useraccdb_recparse: date=>%t<\n",
		recp->datestr.sp, recp->datestr.sl) ;
#endif

	cl = nextfield(lp,ll,&cp) ;
	recp->userstr.sp = cp ;
	recp->userstr.sl = MIN(cl,UAFILE_MAXUSERLEN) ;
	ll = ((lp+ll)-(cp+cl)) ;
	lp = (cp+cl) ;

#if	CF_DEBUGS
	debugprintf("useraccdb_recparse: l=%u user=>%t<\n",
		recp->userstr.sl,
		recp->userstr.sp, recp->userstr.sl) ;
#endif

#if	CF_DEBUGS
	debugprintf("useraccdb_recparse: rline=>%t<\n",
		lp,strlinelen(lp,ll,40)) ;
#endif

	if ((tp = strnchr(lp,ll,CH_LPAREN)) != NULL) {
	    ll = ((lp+ll)-(tp+1)) ;
	    lp = (tp+1) ;
	    cp = lp ;
	    if ((tp = strnchr(lp,ll,CH_RPAREN)) != NULL) {
	        cl = (tp-lp) ;
	        recp->namestr.sp = cp ;
	        recp->namestr.sl = MIN(cl,UAFILE_MAXNAMELEN) ;
#if	CF_DEBUGS
	debugprintf("useraccdb_recparse: name=>%t<\n",
		recp->namestr.sp, recp->namestr.sl) ;
#endif

	    }
	}

	return rs ;
}
/* end subroutine (useraccdb_recparse) */


static int useraccdb_recproc(USERACCDB *op,USERACCDB_REC *recp)
{
	int		rs = SR_OK ;
	int		sl ;
	const char	*sp ;

	if (rs >= 0) {
	    sl = recp->datestr.sl ;
	    sp = recp->datestr.sp ;
	    if (sp != NULL) {
	        rs = useraccdb_datethis(op,&recp->atime,sp,sl) ;
	    } else
	        rs = SR_INVALID ;
	}

	if (rs >= 0) {
	    sl = recp->countstr.sl ;
	    sp = recp->countstr.sp ;
	    if (sp != NULL) {
	        rs = cfdecui(sp,sl,&recp->count) ;
	    } else
	        rs = SR_INVALID ;
	}

	return rs ;
}
/* end subroutine (useraccdb_recproc) */


static int useraccdb_datethis(USERACCDB *op,time_t *timep,cchar *tsp,int tsl)
{
	int		rs = SR_OK ;

	if (! op->f.dater) {
	    struct timeb	now ;
	    const int		zlen = DATER_ZNAMESIZE ;
	    char		zbuf[DATER_ZNAMESIZE+1] ;
	    if ((rs = initnow(&now,zbuf,zlen)) >= 0) {
	        rs = dater_start(&op->dm,&now,zbuf,zlen) ;
	        op->f.dater = (rs >= 0) ;
	    }
	}

	if (rs >= 0)
	    rs = dater_setlogz(&op->dm,tsp,tsl) ;

	if (rs >= 0)
	    rs = dater_gettime(&op->dm,timep) ;

	return rs ;
}
/* end subroutine (useraccdb_datethis) */


static int upinfo_start(uip,op,arguser,argname)
UPINFO		*uip ;
USERACCDB	*op ;
const char	*arguser ;
const char	*argname ;
{
	int		rs ;

	memset(uip,0,sizeof(UPINFO)) ;
	uip->op = op ;
	uip->utime = time(NULL) ;
	uip->arguser = arguser ;
	uip->argname = argname ;
	uip->utime = time(NULL) ;

	rs = mkts(uip->tbuf,UAFILE_LDATE,uip->utime) ;

	return rs ;
}
/* end subroutine (upinfo_start) */


static int upinfo_finish(UPINFO *uip)
{

	if (uip == NULL) return SR_FAULT ;

	return SR_OK ;
}
/* end subroutine (upinfo_finish) */


static int upinfo_match(uip,ro,recp)
UPINFO		*uip ;
offset_t	ro ;
USERACCDB_REC	*recp ;
{
	UPINFO_REC	*urp = NULL ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		sl = recp->userstr.sl ;
	int		rc = 0 ;
	const char	*sp = recp->userstr.sp ;

	if (sp != NULL) {

	if (strwcmp(uip->arguser,sp,sl) == 0) {
	    urp = &uip->user ;
	} else if (strwcmp(totaluser,sp,sl) == 0) {
	    urp = &uip->total ;

	} /* end if */

	if ((urp != NULL) && (! urp->found)) {
	    urp->found = TRUE ;
	    urp->ro = ro ;
	    urp->count = 0 ;
	    sl = recp->countstr.sl ;
	    sp = recp->countstr.sp ;
	    if (sp != NULL) {
	        uint	c ;
	        rs1 = cfdecui(sp,sl,&c) ;
	        if (rs1 >= 0) urp->count = c ;
	    }
	}

	rc = (uip->user.found && uip->total.found) ;

	} /* end if (non-null) */

	return (rs >= 0) ? rc : rs ;
}
/* end subroutine (upinfo_match) */


static int upinfo_update(UPINFO	 *uip)
{
	UPINFO_REC	*urp ;
	int		rs = SR_OK ;

	if (rs >= 0) {
	    urp = &uip->total ;
	    rs = upinfo_upone(uip,urp,0) ;
#if	CF_DEBUGS
	debugprintf("upinfo_update: 0 _upone() rs=%d\n",rs) ;
#endif
	}

	if (rs >= 0) {
	    urp = &uip->user ;
	    rs = upinfo_upone(uip,urp,1) ;
#if	CF_DEBUGS
	debugprintf("upinfo_update: 1 _upone() rs=%d\n",rs) ;
#endif
	}

#if	CF_DEBUGS
	debugprintf("upinfo_update: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (upinfo_update) */


static int upinfo_upone(UPINFO *uip,UPINFO_REC *urp,int type)
{
	const int	rlen = UAFILE_RECLEN ;
	int		rs ;
	char		rbuf[UAFILE_RECLEN+1] ;

#if	CF_DEBUGS
	debugprintf("upinfo_upone: type=%u\n",type) ;
#endif
	if ((rs = upinfo_mkrec(uip,urp,rbuf,rlen,type)) >= 0) {
	    USERACCDB	*op = uip->op ;
	    int		rl = rs ;

#if	CF_DEBUGS
	debugprintf("upinfo_upone: f_found=%u\n",urp->found) ;
#endif
	    if (urp->found) {
	        rs = u_pwrite(op->fd,rbuf,rl,urp->ro) ;
	    } else {
	        rs = u_seek(op->fd,0L,SEEK_END) ;
	        if (rs >= 0)
	            rs = u_write(op->fd,rbuf,rl) ;
	    }
	} /* end if */

#if	CF_DEBUGS
	debugprintf("upinfo_upone: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (upinfo_upone) */


static int upinfo_mkrec(uip,urp,rbuf,rlen,type)
UPINFO		*uip ;
UPINFO_REC	*urp ;
char		rbuf[] ;
int		rlen ;
int		type ;
{
	int		rs ;
	int		dbl ;
	int		rl = 0 ;
	char		digbuf[DIGBUFLEN+1] ;
	char		*dbp = digbuf ;
	char		*rbp = rbuf ;

#if	CF_DEBUGS
	debugprintf("upinfo_mkrec: type=%u\n",type) ;
	debugprintf("upinfo_mkrec: count=%u\n", urp->count) ;
#endif
	if (rlen > UAFILE_RECLEN)
	    return SR_OVERFLOW ;

	rs = ctdecui(digbuf,DIGBUFLEN,(urp->count+1)) ;
	dbl = rs ;
#if	CF_DEBUGS
	debugprintf("upinfo_mkrec: ctdecui() rs=%u\n",rs) ;
#endif

	if (rs >= 0) {
	    int	n = (UAFILE_LCOUNT - dbl) ;
	    if (dbl > UAFILE_LCOUNT) { /* truncate from left as necessary */
	        int	dld = (dbl-UAFILE_LCOUNT) ;
	        dbp += dld ;
	        dbl -= dld ;
	    }
	    if (n > 0) rbp = strnset(rbp,' ',n) ;
	    rbp = strwcpy(rbp,dbp,dbl) ;
	}

#if	CF_DEBUGS
	debugprintf("upinfo_mkrec: rbuf=>%s<\n",rbuf) ;
#endif
	if (rs >= 0) {
	    *rbp++ = ' ' ;
	}

	if (rs >= 0) {
	    rbp = strwcpy(rbp,uip->tbuf,UAFILE_LDATE) ;
	    rl = (rbp - rbuf) ;
	}

	if ((rs >= 0) && (! urp->found)) {
	    const char	*up = (type) ? uip->arguser : totaluser ;
	    *rbp++ = ' ' ;
	    rbp = strwcpy(rbp,up,UAFILE_MAXUSERLEN) ;
	    if (type && (uip->argname != NULL)) {
	        *rbp++ = ' ' ;
	        *rbp++ = CH_LPAREN ;
	        rbp = strwcpy(rbp,uip->argname,UAFILE_MAXNAMELEN) ;
	        *rbp++ = CH_RPAREN ;
	    }
	    *rbp++ = '\n' ;
	    rl = (rbp - rbuf) ;
	}

#if	CF_DEBUGS
	debugprintf("upinfo_mkrec: ret rs=%d rl=%u\n",rs,rl) ;
#endif
	return (rs >= 0) ? rl : rs ;
}
/* end subroutine (upinfo_mkrec) */


static int entry_load(ep,ebuf,elen,recp)
USERACCDB_ENT	*ep ;
char		ebuf[] ;
int		elen ;
USERACCDB_REC	*recp ;
{
	STOREITEM	s ;
	int		rs ;
	int		rs1 ;

	memset(ep,0,sizeof(USERACCDB_ENT)) ;

	if ((rs = storeitem_start(&s,ebuf,elen)) >= 0) {
	    USERACCDB_ITEM	*ip ;
	    const char		*cp ;

	    ep->atime = recp->atime ;
	    ep->count = recp->count ;

	    ip = &recp->userstr ;
	    rs = storeitem_strw(&s,ip->sp,ip->sl,&cp) ;
	    ep->user = cp ;

	    if (rs >= 0) {
	        ip = &recp->namestr ;
	        rs = storeitem_strw(&s,ip->sp,ip->sl,&cp) ;
	        ep->name = cp ;
	    }

	    rs1 = storeitem_finish(&s) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (storeitem) */

	return rs ;
}
/* end subroutine (entry_load) */


static int mkts(char tbuf[],int tlen,time_t t)
{
	int		tl = 0 ;

	timestr_logz(t,tbuf) ;

	tl = strlen(tbuf) ;
	if (tl < tlen) {
	    char	*bp = (tbuf + tl) ;
	    int		bl = (tlen - tl) ;
	    strnset(bp,' ',bl) ;
	}

	return tl ;
}
/* end subroutine (mkts) */


