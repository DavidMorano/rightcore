/* msgid */

/* manage message-id storage */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DEBUGSLEEP	0		/* debug locks using sleeps */
#define	CF_SAFE		1
#define	CF_CREAT	0		/* always create the file? */
#define	CF_HASH		1		/* use hash for faster lookup */


/* revision history:

	= 2003-02-17, David A­D­ Morano
	This code module is being started to eliminate repeated mail messages
	(essentially just from ACM).  Much of this code was borrowed from the
	SRVREG object (which performs a somewhat similar function).

*/

/* Copyright © 2003 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This object module manages the reading and writing of entries to or from
        a message identification database file.


*******************************************************************************/


#define	MSGID_MASTER	0


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<inttypes.h>
#include	<limits.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<endian.h>
#include	<serialbuf.h>
#include	<stdorder.h>
#include	<localmisc.h>

#include	"msgid.h"
#include	"msgide.h"


/* local defines */

#define	MSGID_FMA	"MSGIDA"
#define	MSGID_FMB	"MSGIDB"

#define	MSGID_FS	"msgid"
#define	MSGID_FSA	"msgida"
#define	MSGID_FSB	"msgidb"

#define	MSGID_FLID	(16 + 4)
#define	MSGID_FLHEAD	(3 * 4)
#define	MSGID_FLTOP	(MSGID_FLID + MSGID_FLHEAD)

#define	MSGID_FOID	0
#define	MSGID_FOHEAD	(MSGID_FOID + MSGID_FLID)
#define	MSGID_FOTAB	(MSGID_FOHEAD + MSGID_FLHEAD)

#define	MSGID_ENTSIZE	MSGIDE_SIZE
#define	MSGID_EBS	((MSGIDE_SIZE + 3) & (~ 3))

#define	MSGID_BUFSIZE	(64 * 1024)
#define	MSGID_READSIZE	(16 * 1024)

#define	MSGID_FBUFLEN	(MSGID_FLTOP + 9)

#define	MSGID_LWRITE	0
#define	MSGID_LREAD	1

#define	TO_OPEN		(60 * 60)	/* maximum file-open time */
#define	TO_ACCESS	(2 * 60)	/* maximum access idle time */
#define	TO_LOCK		60		/* seconds */

#ifndef	TIMEBUFLEN
#define	TIMEBUFLEN	80
#endif


/* external subroutines */

extern uint	hashelf(const void *,int) ;
extern uint	ufloor(uint,int) ;
extern uint	uceil(uint,int) ;

extern int	matstr(const char **,const char *,int) ;
extern int	mkfnamesuf1(char *,cchar *,cchar *) ;
extern int	lockfile(int,int,offset_t,offset_t,int) ;
extern int	isfsremote(int) ;
extern int	isNotPresent(int) ;

#if	CF_DEBUGS
extern int	debugprintf(cchar *,...) ;
extern int	strlinelen(cchar *,int,int) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;

#if	CF_DEBUGS
extern char	*timestr_log(time_t,char *) ;
#endif


/* external variables */


/* local structures */

struct oldentry {
	time_t		utime ;
	int		ei ;
} ;


/* forward references */

static uint	recipidhash(MSGID_KEY *,int,int) ;

int		msgid_close(MSGID *) ;

static int	msgid_opener(MSGID *,char *,cchar *,int,mode_t) ;
static int	msgid_openone(MSGID *,char *,cchar *,cchar *,int,mode_t) ;

static int	msgid_fileopen(MSGID *,time_t) ;
static int	msgid_fileclose(MSGID *) ;
static int	msgid_lockget(MSGID *,time_t,int) ;
static int	msgid_lockrelease(MSGID *) ;
static int	msgid_fileinit(MSGID *,time_t) ;
static int	msgid_filechanged(MSGID *) ;
static int	msgid_filecheck(MSGID *,time_t,int) ;
static int	msgid_searchid(MSGID *,const char *,int,char **) ;
static int	msgid_search(MSGID *,MSGID_KEY *,uint,char **) ;
static int	msgid_searchempty(MSGID *,struct oldentry *,char **) ;
static int	msgid_searchemptyrange(MSGID *,int,int,
			struct oldentry *,char **) ;
static int	msgid_buf(MSGID *,int,int,char **) ;
static int	msgid_bufupdate(MSGID *,int,int,const char *) ;
static int	msgid_bufbegin(MSGID *) ;
static int	msgid_bufend(MSGID *) ;
static int	msgid_writehead(MSGID *) ;

static int	filemagic(char *,int,MSGID_FM *) ;
static int	filehead(char *,int,MSGID_FH *) ;

static int	emat_recipid(const char *,MSGID_KEY *) ;

static int	matfield(const char *,int,const char *,int) ;

static int	extutime(char *) ;


/* local variables */


/* exported subroutines */


int msgid_open(MSGID *op,cchar *fname,int of,mode_t om,int maxentry)
{
	int		rs ;
	int		f_create = FALSE ;

#if	CF_DEBUGS
	debugprintf("msgid_open: ent dbname=%s\n",fname) ;
#endif

#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;
#endif /* CF_SAFE */

	if (fname == NULL) return SR_FAULT ;

	if (fname[0] == '\0') return SR_INVALID ;

#if	CF_DEBUGS
	debugprintf("msgid_open: dbname=%s\n",fname) ;
	debugprintf("msgid_open: of=%4o\n",of) ;
	if (of& O_CREAT) {
	    debugprintf("msgid_open: creating as needed\n") ;
	}
#endif

#if	CF_CREAT
	of |= O_CREAT ;
#endif

	of &= (~ O_TRUNC) ;

	memset(op,0,sizeof(MSGID)) ;
	op->fd = -1 ;
	op->oflags = of ;
	op->operm = om ;
	op->maxentry = maxentry ;
	op->ebs = uceil(MSGID_ENTSIZE,4) ;

/* initialize the buffer structure */

	if ((rs = msgid_bufbegin(op)) >= 0) {
	    const time_t	dt = time(NULL) ;
	    char		tbuf[MAXPATHLEN+1] ;
	    if ((rs = msgid_opener(op,tbuf,fname,of,om)) >= 0) {
	        cchar	*fn ;
	        f_create = rs ;
	        op->opentime = dt ;
	        op->accesstime = dt ;
	        if ((rs = uc_mallocstrw(tbuf,-1,&fn)) >= 0) {
	            USTAT	sb ;
	            const int	am = (of & O_ACCMODE) ;
	            op->fname = fn ;
	            op->f.writable = ((am == O_WRONLY) || (am == O_RDWR)) ;
	            if ((rs = u_fstat(op->fd,&sb)) >= 0) {
	                if (S_ISREG(sb.st_mode)) {
	                    op->mtime = sb.st_mtime ;
	                    op->filesize = sb.st_size ;
	                    op->pagesize = getpagesize() ;
	                    if ((rs = isfsremote(op->fd)) >= 0) {
	                        op->f.remote = (rs > 0) ;
	                        if ((rs = msgid_fileinit(op,dt)) >= 0) {
	                            op->magic = MSGID_MAGIC ;
	                        }
	                    } /* end if (isfsremote) */
	                } else {
	                    rs = SR_ISDIR ;
	                }
	            } /* end if (stat) */
	            if (rs < 0) {
	                uc_free(op->fname) ;
	                op->fname = NULL ;
	            }
	        } /* end if (m-a) */
	        if (rs < 0) {
	            u_close(op->fd) ;
	            op->fd = -1 ;
	        }
	    } /* end if (open) */
	    if (rs < 0) {
	        msgid_bufend(op) ;
	    }
	} /* end if (buffer-start) */

#if	CF_DEBUGS
	debugprintf("msgid_open: ret rs=%d f_create=%u\n",rs,f_create) ;
#endif

	return (rs >= 0) ? f_create : rs ;
}
/* end subroutine (msgid_open) */


int msgid_close(MSGID *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

#if	CF_DEBUGS
	debugprintf("msgid_close: ent\n") ;
#endif

#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;

	if (op->magic != MSGID_MAGIC) return SR_NOTOPEN ;
#endif /* CF_SAFE */

	rs1 = msgid_bufend(op) ;
	if (rs >= 0) rs = rs1 ;

	if (op->fd >= 0) {
	    rs1 = u_close(op->fd) ;
	    if (rs >= 0) rs = rs1 ;
	    op->fd = -1 ;
	}

	if (op->fname != NULL) {
	    rs1 = uc_free(op->fname) ;
	    if (rs >= 0) rs = rs1 ;
	    op->fname = NULL ;
	}

#if	CF_DEBUGS
	debugprintf("msgid_close: ret rs=%d\n",rs) ;
#endif

	op->magic = 0 ;
	return rs ;
}
/* end subroutine (msgid_close) */


int msgid_txbegin(MSGID *op)
{

#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;

	if (op->magic != MSGID_MAGIC) return SR_NOTOPEN ;
#endif /* CF_SAFE */

	return SR_OK ;
}
/* end subroutine (msgid_txbegin) */


/* ARGSUSED */
int msgid_txabort(MSGID *op,int txid)
{

#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;

	if (op->magic != MSGID_MAGIC) return SR_NOTOPEN ;
#endif /* CF_SAFE */

	return SR_OK ;
}
/* end subroutine (msgid_txabort) */


/* ARGSUSED */
int msgid_txcommit(MSGID *op,int txid)
{

#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;

	if (op->magic != MSGID_MAGIC) return SR_NOTOPEN ;
#endif /* CF_SAFE */

	return SR_OK ;
}
/* end subroutine (msgid_txcomment) */


/* get a count of the number of entries */
int msgid_count(MSGID *op)
{
	int		rs = SR_OK ;
	int		c ;

#if	CF_DEBUGS
	debugprintf("msgid_count: ent\n") ;
#endif

#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;

	if (op->magic != MSGID_MAGIC) return SR_NOTOPEN ;
#endif /* CF_SAFE */

	c = ((op->filesize - MSGID_FOTAB) / MSGID_ENTSIZE) ;

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (msgid_count) */


/* initialize a cursor */
int msgid_curbegin(MSGID *op,MSGID_CUR *curp)
{

#if	CF_DEBUGS
	debugprintf("msgid_curbegin: ent\n") ;
#endif

#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;

	if (op->magic != MSGID_MAGIC) return SR_NOTOPEN ;
#endif /* CF_SAFE */

	if (curp == NULL) return SR_FAULT ;

	op->cursors += 1 ;

	op->f.cursorlockbroken = FALSE ;
	op->f.cursoracc = FALSE ;
	curp->i = -1 ;
	return SR_OK ;
}
/* end subroutine (msgid_curbegin) */


/* free up a cursor */
int msgid_curend(MSGID *op,MSGID_CUR *curp)
{
	int		rs = SR_OK ;
	int		rs1 ;

#if	CF_DEBUGS
	debugprintf("msgid_curend: ent\n") ;
#endif

#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;

	if (op->magic != MSGID_MAGIC) return SR_NOTOPEN ;
#endif /* CF_SAFE */

	if (curp == NULL) return SR_FAULT ;

	if (op->f.cursoracc) {
	    op->accesstime = time(NULL) ;
	}

	if (op->cursors > 0)
	    op->cursors -= 1 ;

	if ((op->cursors == 0) && (op->f.readlocked || op->f.writelocked)) {
	    rs1 = msgid_lockrelease(op) ;
	    if (rs >= 0) rs = rs1 ;
	}

	curp->i = -1 ;
	return rs ;
}
/* end subroutine (msgid_curend) */


/* enumerate the entries */
int msgid_enum(MSGID *op,MSGID_CUR *curp,MSGID_ENT *ep)
{
	const time_t	dt = time(NULL) ;
	int		rs ;
	int		eoff ;
	int		ei = 0 ;
	char		*bp ;

#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;

	if (op->magic != MSGID_MAGIC) return SR_NOTOPEN ;
#endif /* CF_SAFE */

#if	CF_DEBUGS
	debugprintf("msgid_enum: ent fileinit=%u\n",op->f.fileinit) ;
	debugprintf("msgid_enum: cursorlockbroken=%u\n",
	    op->f.cursorlockbroken) ;
#endif

	if (curp == NULL) return SR_FAULT ;

/* is the file even initialized? */

	if (! op->f.fileinit) return SR_NOTFOUND ;

/* has our lock been broken */

	if (op->f.cursorlockbroken) return SR_LOCKLOST ;

/* do we have proper file access? */

	if ((rs = msgid_filecheck(op,dt,MSGID_LREAD)) >= 0) {
	    if (op->f.fileinit) {

#if	CF_DEBUGS
	        debugprintf("msgid_enum: msgid_filecheck() rs=%d\n",rs) ;
#endif

/* OK, give an entry back to caller */

	        ei = (curp->i < 0) ? 0 : curp->i + 1 ;
	        eoff = MSGID_FOTAB + (ei * op->ebs) ;

#if	CF_DEBUGS
	        debugprintf("msgid_enum: ei=%d eoff=%u\n",ei,eoff) ;
#endif

/* form result to caller */

	        if ((eoff + op->ebs) <= op->filesize) {

/* verify sufficient file buffering */

	            if ((rs = msgid_buf(op,eoff,op->ebs,&bp)) >= 0) {
	                if (rs >= op->ebs) {

#if	CF_DEBUGS
	                    debugprintf("msgid_enum: msgid_buf() rs=%d\n",rs) ;
#endif

/* copy entry to caller buffer */

	                    if (ep != NULL) {
	                        msgide_all(ep,1,bp,MSGIDE_SIZE) ;
	                    } /* end if */

/* commit the cursor movement? */

	                    if (rs >= 0)
	                        curp->i = ei ;

	                    op->f.cursoracc = TRUE ;

	                } else {
	                    rs = SR_EOF ;
			}
	            } /* end if (msgid_buf) */

	        } else {
	            rs = SR_NOTFOUND ;
		}

	    } else {
	        rs = SR_EOF ;
	    }
	} /* end if (msgid_filecheck) */

#if	CF_DEBUGS
	debugprintf("msgid_enum: ret rs=%d ei=%u\n",rs,ei) ;
#endif

	return (rs >= 0) ? ei : rs ;
}
/* end subroutine (msgid_enum) */


/* fetch by message-id name */
int msgid_matchid(MSGID *op,time_t dt,cchar *midp,int midl,MSGID_ENT *ep)
{
	int		rs ;
	int		ei = 0 ;
	char		*bep ;

#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;

	if (op->magic != MSGID_MAGIC) return SR_NOTOPEN ;
#endif /* CF_SAFE */

	if (midp == NULL) return SR_FAULT ;

#ifdef	OPTIONAL
	if (ep == NULL) return SR_FAULT ;
#endif

/* is the file even initialized? */

	if (! op->f.fileinit) return SR_NOTFOUND ;

/* do we have proper file access? */

	if (dt == 0) dt = time(NULL) ;
	if ((rs = msgid_filecheck(op,dt,MSGID_LREAD)) >= 0) {
	    if (op->f.fileinit) {

/* continue with the search */

	        if ((rs = msgid_searchid(op,midp,midl,&bep)) >= 0) {
	            ei = rs ;
	            if (ep != NULL) {
	                msgide_all(ep,1,bep,MSGIDE_SIZE) ;
	            }
	        } /* end if */

/* update access time as appropriate */

	        if (dt == 0) dt = time(NULL) ;
	        op->accesstime = dt ;

	    } /* end if (fileinit) */
	} /* end if (msgid_filecheck) */

#if	CF_DEBUGS
	debugprintf("msgid_matchid: ret rs=%d ei=%u\n",rs,ei) ;
#endif

	return (rs >= 0) ? ei : rs ;
}
/* end subroutine (msgid_matchid) */


/* match by recipient and message-id */
int msgid_match(MSGID *op,time_t dt,MSGID_KEY *kp,MSGID_ENT *ep)
{
	uint		khash ;
	int		rs ;
	int		midlen ;
	int		ei = 0 ;
	char		*bep ;

#if	CF_DEBUGS
	debugprintf("msgid_match: ent\n") ;
#endif

#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;

	if (op->magic != MSGID_MAGIC) return SR_NOTOPEN ;
#endif /* CF_SAFE */

	if (kp == NULL) return SR_FAULT ;

	if ((kp->recip == NULL) || (kp->mid == NULL)) return SR_FAULT ;

#ifdef	OPTIONAL
	if (ep == NULL) return SR_FAULT ;
#endif

/* is the file even initialized? */

	if (! op->f.fileinit) return SR_NOTFOUND ;

/* do we have proper file access? */

	if (dt == 0) dt = time(NULL) ;
	if ((rs = msgid_filecheck(op,dt,MSGID_LREAD)) >= 0) {
	    if (op->f.fileinit) {

/* continue with the search */

	        midlen = strnlen(kp->mid,MSGIDE_LMESSAGEID) ;

	        khash = recipidhash(kp,-1,midlen) ;

	        if ((rs = msgid_search(op,kp,khash,&bep)) >= 0) {
	            ei = rs ;
	            if (ep != NULL) {
	                msgide_all(ep,1,bep,MSGIDE_SIZE) ;
	            }
	        } /* end if */

/* update access time as appropriate */

	        if (dt == 0) dt = time(NULL) ;
	        op->accesstime = dt ;

	    } /* end if (fileinit) */
	} /* end if (msgid_filecheck) */

#if	CF_DEBUGS
	debugprintf("msgid_match: ret rs=%d ei=%u\n",rs,ei) ;
#endif

	return (rs >= 0) ? ei : rs ;
}
/* end subroutine (msgid_match) */


/* update a (recipient,message-id) tuple entry match */
int msgid_update(MSGID *op,time_t dt,MSGID_KEY *kp,MSGID_ENT *ep)
{
	offset_t	uoff ;
	uint		khash ;
	int		eoff ;
	int		rs ;
	int		wlen, midlen ;
	int		ei = 0 ;
	int		f_addition = FALSE ;
	int		f_bufupdate = FALSE ;
	char		ebuf[MSGIDE_SIZE + 4] ;
	char		*bep ;

#if	CF_DEBUGS
	debugprintf("msgid_update: ent\n") ;
#endif

#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;

	if (op->magic != MSGID_MAGIC) return SR_NOTOPEN ;
#endif /* CF_SAFE */

	if (kp == NULL) return SR_FAULT ;
	if ((kp->recip == NULL) || (kp->mid == NULL)) return SR_FAULT ;

#ifdef	OPTIONAL
	if (ep == NULL) return SR_FAULT ;
#endif

#if	CF_DEBUGS
	debugprintf("msgid_update: ent key.recip=%s\n", kp->recip) ;
	debugprintf("msgid_update: key.mid=%s\n", kp->mid) ;
#endif

/* is the file even initialized? */

	if (! op->f.fileinit) return SR_NOTFOUND ;

/* do we have proper file access? */

	if (dt == 0) dt = time(NULL) ;

#if	CF_DEBUGS
	debugprintf("msgid_update: msgid_filecheck()\n") ;
#endif

	if ((rs = msgid_filecheck(op,dt,MSGID_LWRITE)) >= 0) {
	    if (op->f.fileinit) {

#if	CF_DEBUGS && (CF_DEBUGSLEEP > 0)
	        debugprintf("msgid_update: sleeping\n") ;
	        sleep(CF_DEBUGSLEEP) ;
	        debugprintf("msgid_update: waking\n") ;
#endif

/* continue with the search */

	        midlen = strnlen(kp->mid,MSGIDE_LMESSAGEID) ;

	        khash = recipidhash(kp,-1,midlen) ;

	        if ((rs = msgid_search(op,kp,khash,&bep)) >= 0) {
	            MSGIDE_UPDATE	m1 ;
	            ei = rs ;

#if	CF_DEBUGS
	            debugprintf("msgid_update: entry found, ei=%d\n",ei) ;
#endif

	            if (ep != NULL) {
	                msgide_all(ep,1,bep,MSGIDE_SIZE) ;
	            }

/* update the file buffer */

	            msgide_update(&m1,1,bep,MSGIDE_SIZE) ;

	            m1.count += 1 ;
	            m1.utime = dt ;
	            wlen = msgide_update(&m1,0,bep,MSGIDE_SIZE) ;

#if	CF_DEBUGS
	            if (ep != NULL) {
	                debugprintf("msgid_update: mid=%s\n",ep->messageid) ;
	                debugprintf("msgid_update: count=%u\n",ep->count) ;
	            }
#endif

	        } else if (rs == SR_NOTFOUND) {
	            struct oldentry	old ;
	            MSGIDE_ALL		m0 ;
	            MSGIDE_UPDATE	m1 ;

#if	CF_DEBUGS
	            debugprintf("msgid_update: entry not found \n") ;
#endif

	            memset(&m0,0,sizeof(MSGIDE_ALL)) ;
	            m0.count = 0 ;
	            m0.utime = dt ;
	            m0.mtime = kp->mtime ;
	            m0.ctime = dt ;
	            m0.hash = khash ;

	            if (kp->from != NULL) {
	                strwcpy(m0.from,kp->from, MSGIDE_LFROM) ;
	            }

	            strwcpy(m0.messageid,kp->mid, MSGIDE_LMESSAGEID) ;

	            strwcpy(m0.recipient,kp->recip, MSGIDE_LRECIPIENT) ;

/* find an empty slot if there is one */

	            rs = msgid_searchempty(op,&old,&bep) ;
	            ei = rs ;

#if	CF_DEBUGS
	            {
			cchar	*fmt = "msgid_update: msgid_searchempty() "
	                    		"rs=%d old.ei=%u\n" ;
	                char	*s ;
	                debugprintf(fmt,rs,old.ei) ;
	                if (rs >= 0) {
	                    s = "found" ;
	                } else if (rs == SR_NOTFOUND) {
	                    s = "notfound" ;
	                } else {
	                    s = "error" ;
			}
	                debugprintf("msgid_update: empty %s\n",s) ;
	            }
#endif /* CF_DEBUGS */

	            if (rs == SR_NOTFOUND) {
	                rs = SR_OK ;

	                bep = ebuf ;
	                f_bufupdate = TRUE ;

	                if (op->h.nentries < op->maxentry) {
	                    f_addition = TRUE ;
	                    ei = op->h.nentries ;
	                } else {
	                    ei = old.ei ;
	                }

	            } /* end if (entry not found) */

/* write to the buffer we have (either the file buffer or our own) */

	            wlen = msgide_all(&m0,0,bep,MSGIDE_SIZE) ;

	            if (ep != NULL) {
	                msgide_all(ep,1,bep,MSGIDE_SIZE) ;
	            }

	            m0.count += 1 ;

/* also update the entry */

	            m1.count = 1 ;
	            m1.utime = m0.utime ;
	            msgide_update(&m1,0,bep,MSGIDE_SIZE) ;

/* update the in-core file buffer as needed or as appropriate */

	            if ((rs >= 0) && f_bufupdate && op->f.writable) {
	                eoff = MSGID_FOTAB + (ei * op->ebs) ;
	                msgid_bufupdate(op,eoff,wlen,ebuf) ;
	            }

#if	CF_DEBUGS
	            {
	                MSGIDE_ALL	m0 ;
	                char		timebuf[TIMEBUFLEN + 1] ;
	                msgide_all(&m0,1,bep,op->ebs) ;
	                debugprintf("msgid_update: writing count=%u utime=%s\n",
	                    m0.count,timestr_log(m0.utime,timebuf)) ;
	            }
#endif /* CF_DEBUGS */

	        } /* end if (match or not) */

#if	CF_DEBUGS
	        debugprintf("msgid_update: mid-point rs=%d\n",rs) ;
#endif

	        if ((rs >= 0) && op->f.writable) {

/* write back this entry */

	            eoff = MSGID_FOTAB + (ei * op->ebs) ;

#if	CF_DEBUGS
	            {
	                MSGIDE_ALL	m0 ;
	                char		timebuf[TIMEBUFLEN + 1] ;
	                msgide_all(&m0,1,bep,op->ebs) ;
	                debugprintf("msgid_update: writing at "
				"ei=%u foff=%u wlen=%u\n", ei,eoff,wlen) ;
	                debugprintf("msgid_update: writing count=%u utime=%s\n",
	                    m0.count,timestr_log(m0.utime,timebuf)) ;
	            }
#endif /* CF_DEBUGS */

	            uoff = eoff ;
	            if ((rs = u_pwrite(op->fd,bep,wlen,uoff)) >= wlen) {

#if	CF_DEBUGS
	                debugprintf("msgid_update: writing back header\n") ;
#endif

	                if (dt == 0) dt = time(NULL) ;

	                op->h.wcount += 1 ;
	                op->h.wtime = dt ;
	                if (f_addition) {
	                    op->h.nentries += 1 ;
	                    op->filesize += wlen ;
	                }

	                rs = msgid_writehead(op) ;

#if	CF_DEBUGS
	                debugprintf("msgid_update: msgid_writehead() "
				"rs=%d\n",rs) ;
#endif

	                if ((rs >= 0) && op->f.remote) {
	                    u_fsync(op->fd) ;
	                }

	            } /* end if (data write was successful) */

	        } /* end if (writing updated entry to file) */

/* update access time as appropriate */

	        if (dt == 0) dt = time(NULL) ;
	        op->accesstime = dt ;

	    } else {
	        rs = SR_NOTOPEN ;
	    }
	} /* end if */

#if	CF_DEBUGS
	debugprintf("msgid_update: ret rs=%d ei=%u\n",rs,ei) ;
#endif

	return (rs >= 0) ? ei : rs ;
}
/* end subroutine (msgid_update) */


/* do some checking */
int msgid_check(MSGID *op,time_t dt)
{
	int		rs = SR_OK ;
	int		f = FALSE ;

#if	CF_DEBUGS
	debugprintf("msgid_check: ent\n") ;
#endif

#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;

	if (op->magic != MSGID_MAGIC) return SR_NOTOPEN ;
#endif /* CF_SAFE */

	if (op->fd >= 0) {
	    if ((! op->f.readlocked) && (! op->f.writelocked)) {
	        f = f || ((dt - op->accesstime) > TO_ACCESS) ;
	        f = f || ((dt - op->opentime) > TO_OPEN) ;
	        if (f) {
	            rs = msgid_fileclose(op) ;
	        }
	    }
	} /* end if (file was open) */

	return rs ;
}
/* end subroutine (msgid_check) */


/* private subroutines */


static int msgid_opener(MSGID *op,char *tbuf,cchar *fn,int of,mode_t om)
{
	int		rs ;
	int		nof ;
	int		f_create = FALSE ;
	cchar		*ext ;
#if	CF_DEBUGS
	debugprintf("msgid_opener: ent\n") ;
#endif
	ext = MSGID_FS ;
	nof = (of & (~ O_CREAT)) ;
	rs = msgid_openone(op,tbuf,fn,ext,nof,om) ;
	if (isNotPresent(rs)) {
	    ext = MSGID_FSB ;
	    nof = (of & (~ O_CREAT)) ;
	    rs = msgid_openone(op,tbuf,fn,ext,nof,om) ;
	    if (isNotPresent(rs) && (of & O_CREAT)) {
	        ext = MSGID_FS ;
	        nof = of ;
	        f_create = TRUE ;
	        rs = msgid_openone(op,tbuf,fn,ext,nof,om) ;
	        if (isNotPresent(rs)) {
	            ext = MSGID_FSB ;
	            nof = of ;
	            rs = msgid_openone(op,tbuf,fn,ext,nof,om) ;
	        }
	    }
	}
#if	CF_DEBUGS
	debugprintf("msgid_opener: ret rs=%d f_create=%u\n",rs,f_create) ;
#endif
	return (rs >= 0) ? f_create : rs ;
}
/* end subroutine (msgid_opener) */


static int msgid_openone(MSGID *op,char *tbuf,cchar *fn,cchar *ext,
		int of,mode_t om)
{
	int		rs ;
	int		pl = 0 ;
	if ((rs = mkfnamesuf1(tbuf,fn,ext)) >= 0) {
	    pl = rs ;
	    if ((rs = u_open(tbuf,of,om)) >= 0) {
	        op->fd = rs ;
	    }
	}
	return (rs >= 0) ? pl : rs ;
}
/* end subroutine (sgid_openone) */


/* check the file for coherency */
static int msgid_filecheck(MSGID *op,time_t dt,int f_read)
{
	int		rs = SR_OK ;
	int		f_changed = FALSE ;

/* is the file open */

	if (op->fd < 0) {
	    if (dt == 0) dt = time(NULL) ;
	    rs = msgid_fileopen(op,dt) ;
	}

/* capture the lock if we do not already have it */

	if (rs >= 0) {
	    if ((! op->f.readlocked) && (! op->f.writelocked)) {
	        if (dt == 0) dt = time(NULL) ;
	        if ((rs = msgid_lockget(op,dt,f_read)) >= 0) {
	            if ((rs = msgid_filechanged(op)) >= 0) {
	                f_changed = (rs > 0) ;
	            }
	            if (rs < 0)
	                msgid_lockrelease(op) ;
	        } /* end if (capture lock) */
	    }
	} /* end if (ok) */

	return (rs >= 0) ? f_changed : rs ;
}
/* end subroutine (msgid_filecheck) */


/* initialize the file header (either read it only or write it) */
static int msgid_fileinit(MSGID *op,time_t dt)
{
	MSGID_FM	fm ;
	int		rs ;
	int		bl ;
	int		f_locked = FALSE ;
	char		fbuf[MSGID_FBUFLEN + 1] ;

#if	CF_DEBUGS
	debugprintf("msgid_fileinit: ent filesize=%u\n",op->filesize) ;
	debugprintf("msgid_fileinit: fbuf(%p)\n",fbuf) ;
#endif

	if (op->filesize == 0) {

	    u_seek(op->fd,0L,SEEK_SET) ;

	    op->f.fileinit = FALSE ;
	    if (op->f.writable) {

#if	CF_DEBUGS
	        debugprintf("msgid_fileinit: want to write\n") ;
#endif

	        if (! op->f.writelocked) {
	            if ((rs = msgid_lockget(op,dt,0)) >= 0) {
	                f_locked = TRUE ;
	            }
	        }

/* write the file magic and header stuff */

#if	CF_DEBUGS
	        debugprintf("msgid_fileinit: writing header\n") ;
#endif

/* file magic */

	        if (rs >= 0) {

	            strwcpy(fm.magic,MSGID_FMB,14) ;
	            fm.vetu[0] = MSGID_FILEVERSION ;
	            fm.vetu[1] = MSGID_ENDIAN ;
	            fm.vetu[2] = 0 ;
	            fm.vetu[3] = 0 ;

	            bl = 0 ;
	            bl += filemagic((fbuf + bl),0,&fm) ;

/* file header */

	            memset(&op->h,0,sizeof(MSGID_FH)) ;

#if	CF_DEBUGS
	            debugprintf("msgid_fileinit: filehead() bl=%d\n",bl) ;
#endif

	            bl += filehead((fbuf + bl),0,&op->h) ;

/* write them to the file */

#if	CF_DEBUGS
	            debugprintf("msgid_fileinit: u_pwrite() wlen=%d\n",bl) ;
#endif

	            if ((rs = u_pwrite(op->fd,fbuf,bl,0L)) > 0) {
	                op->filesize = rs ;
	                op->mtime = dt ;
	                if (op->f.remote) u_fsync(op->fd) ;
	                rs = msgid_bufupdate(op,0,bl,fbuf) ;
	            }
	            op->f.fileinit = (rs >= 0) ;

	        } /* end if (ok) */

	    } /* end if (writing) */

	} else if (op->filesize >= MSGID_FOTAB) {

/* read the file header */

	    if (! op->f.readlocked) {
	        if ((rs = msgid_lockget(op,dt,1)) >= 0) {
	            f_locked = TRUE ;
	        }
	    }

	    if (rs >= 0) {
	        const int	fltop = MSGID_FLTOP ;
	        if ((rs = u_pread(op->fd,fbuf,MSGID_FBUFLEN,0L)) >= fltop) {
		    int	f ;

	            bl = 0 ;
	            bl += filemagic((fbuf + bl),1,&fm) ;

	            filehead((fbuf + bl),1,&op->h) ;

#if	CF_DEBUGS
	            debugprintf("msgid_fileinit: f_wtime=%08x\n",
	                op->h.wtime) ;
	            debugprintf("msgid_fileinit: f_wcount=%08x\n",
	                op->h.wcount) ;
	            debugprintf("msgid_fileinit: f_nentries=%08x\n",
	                op->h.nentries) ;
#endif

	            f = (strcmp(fm.magic,MSGID_FMB) == 0) ;

#if	CF_DEBUGS
	            debugprintf("msgid_fileinit: fm.magic=%s\n",fm.magic) ;
	            debugprintf("msgid_fileinit: magic cmp f=%d\n",f) ;
#endif

	            {
	                const int	v = MSGID_FILEVERSION ;
	                f = f && (fm.vetu[0] <= v) ;
	            }

#if	CF_DEBUGS
	            debugprintf("msgid_fileinit: version cmp f=%d\n",f) ;
#endif

	            f = f && (fm.vetu[1] == MSGID_ENDIAN) ;

#if	CF_DEBUGS
	            debugprintf("msgid_fileinit: endian cmp f=%d\n",f) ;
#endif

	            if (! f)
	                rs = SR_BADFMT ;

	            op->f.fileinit = f ;

	        } /* end if (u_pread) */
	    } /* end if (ok) */

	} /* end if */

/* if we locked, we unlock it, otherwise leave it! */

	if (f_locked) {
	    msgid_lockrelease(op) ;
	}

#if	CF_DEBUGS
	debugprintf("msgid_fileinit: ret rs=%d fileinit=%u\n",
	    rs,op->f.fileinit) ;
#endif

	return rs ;
}
/* end subroutine (msgid_fileinit) */


/* has the file changed at all? */
static int msgid_filechanged(MSGID *op)
{
	struct ustat	sb ;
	int		rs ;
	int		f_changed = FALSE ;

/* has the file changed at all? */

	if ((rs = u_fstat(op->fd,&sb)) >= 0) {

#if	CF_DEBUGS
	    debugprintf("msgid_filechanged: u_fstat() rs=%d filesize=%u\n",
	        rs,sb.st_size) ;
#endif

	    if (sb.st_size < MSGID_FOTAB)
	        op->f.fileinit = FALSE ;

	    f_changed = (! op->f.fileinit) ||
	        (sb.st_size != op->filesize) ||
	        (sb.st_mtime != op->mtime) ;

#if	CF_DEBUGS
	    debugprintf("msgid_filechanged: fileinit=%u\n",op->f.fileinit) ;
	    debugprintf("msgid_filechanged: sb_size=%08x o_size=%08x\n",
	        sb.st_size,op->filesize) ;
	    debugprintf("msgid_filechanged: sb_mtime=%08x o_mtime=%08x\n",
	        sb.st_mtime,op->mtime) ;
	    debugprintf("msgid_filechanged: fstat f_changed=%u\n",f_changed) ;
#endif /* CF_DEBUGS */

/* if it has NOT changed, read the file header for write indications */

	    if ((! f_changed) && op->f.fileinit) {
	        MSGID_FH	h ;
	        char		hbuf[MSGID_FLTOP + 1] ;

	        if ((rs = u_pread(op->fd,hbuf,MSGID_FLTOP,0L)) >= 0) {

#if	CF_DEBUGS
	            debugprintf("msgid_filechanged: u_pread() rs=%d\n",rs) ;
#endif

	            if (rs < MSGID_FLTOP)
	                op->f.fileinit = FALSE ;

#if	CF_DEBUGS
	            debugprintf("msgid_filechanged: fileinit=%u\n",
			op->f.fileinit) ;
#endif

	            if (rs > 0) {

	                filehead((hbuf + MSGID_FOHEAD),1,&h) ;

	                f_changed = (op->h.wtime != h.wtime) ||
	                    (op->h.wcount != h.wcount) ||
	                    (op->h.nentries != h.nentries) ;

#if	CF_DEBUGS
	                debugprintf("msgid_filechanged: o_wtime=%08x "
			    "fh_wtime=%08x\n",
	                    op->h.wtime,h.wtime) ;
	                debugprintf("msgid_filechanged: o_wcount=%08x "
			    "fh_wcount=%08x\n",
	                    op->h.wcount,h.wcount) ;
	                debugprintf("msgid_filechanged: "
	                    "o_nentries=%08x fh_nentries=%08x\n",
	                    op->h.nentries,h.nentries) ;
	                debugprintf("msgid_filechanged: header f_changed=%u\n",
	                    f_changed) ;
#endif /* CF_DEBUGS */

	                if (f_changed)
	                    op->h = h ;

	            } /* end if (positive) */

	        } /* end if (u_pread) */

	    } /* end if (reading file header) */

/* OK, we're done */

	    if ((rs >= 0) && f_changed) {
	        op->b.len = 0 ;
	        op->filesize = sb.st_size ;
	        op->mtime = sb.st_mtime ;
	    }

	} /* end if (stat) */

#if	CF_DEBUGS
	debugprintf("msgid_filechanged: ret rs=%d f_changed=%u\n",
	    rs,f_changed) ;
#endif

	return (rs >= 0) ? f_changed : rs ;
}
/* end subroutine (msgid_filechanged) */


/* acquire access to the file */
static int msgid_lockget(MSGID *op,time_t dt,int f_read)
{
	int		rs = SR_OK ;

#if	CF_DEBUGS
	debugprintf("msgid_lockget: ent f_read=%d\n",f_read) ;
#endif

	if (op->fd < 0) {
	    rs = msgid_fileopen(op,dt) ;
	} /* end if (needed to open the file) */

/* acquire a file record lock */

	if (rs >= 0) {
	    int		lockcmd ;
	    int		f_already = FALSE ;

	    if (f_read || (! op->f.writable)) {
	        f_already = op->f.readlocked ;
	        op->f.readlocked = TRUE ;
	        op->f.writelocked = FALSE ;
	        lockcmd = F_RLOCK ;
	    } else {
	        f_already = op->f.writelocked ;
	        op->f.readlocked = FALSE ;
	        op->f.writelocked = TRUE ;
	        lockcmd = F_WLOCK ;
	    }

/* get out if we have the lock that we want already */

	    if (! f_already) {
	        rs = lockfile(op->fd,lockcmd,0L,0L,TO_LOCK) ;
	    }

	} /* end if (ok) */

#if	CF_DEBUGS
	debugprintf("msgid_lockget: ret rs=%d fileinit=%u\n",
	    rs,op->f.fileinit) ;
#endif

	return rs ;
}
/* end subroutine (msgid_lockget) */


static int msgid_lockrelease(MSGID *op)
{
	int		rs = SR_OK ;

#if	CF_DEBUGS
	debugprintf("msgid_lockrelease: ent\n") ;
#endif

	if ((op->f.readlocked || op->f.writelocked)) {
	    if (op->fd >= 0) {
	        rs = lockfile(op->fd,F_ULOCK,0L,0L,TO_LOCK) ;
	    }
	    op->f.readlocked = FALSE ;
	    op->f.writelocked = FALSE ;
	} /* end if */

#if	CF_DEBUGS
	debugprintf("msgid_lockrelease: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (msgid_lockrelease) */


static int msgid_fileopen(MSGID *op,time_t dt)
{
	int		rs = SR_OK ;

#if	CF_DEBUGS
	debugprintf("msgid_fileopen: fname=%s\n",op->fname) ;
#endif

	if (op->fd < 0) {
	    if ((rs = u_open(op->fname,op->oflags,op->operm)) >= 0) {
	        op->fd = rs ;
	        op->opentime = dt ;
	        rs = uc_closeonexec(op->fd,TRUE) ;
	        if (rs < 0) {
	            u_close(op->fd) ;
	            op->fd = -1 ;
	        }
	    }
	} else {
	    rs = op->fd ;
	}

	return rs ;
}
/* end subroutine (msgid_fileopen) */


int msgid_fileclose(MSGID *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op->fd >= 0) {
	    rs1 = u_close(op->fd) ;
	    if (rs >= 0) rs = rs1 ;
	    op->fd = -1 ;
	}

	return rs ;
}
/* end subroutine (msgid_fileclose) */


/* search for an entry by MESSAGE-ID */
static int msgid_searchid(MSGID *op,cchar *midp,int midl,char **bepp)
{
	int		rs = SR_OK ;
	int		eoff ;
	int		len ;
	int		n ;
	int		i ;
	int		ne = 100 ;
	int		ei = 0 ;
	int		f = FALSE ;
	char		*bp, *bep, *eidp ;

	while (! f) {

	    eoff = MSGID_FOTAB + (ei * op->ebs) ;
	    len = ne * op->ebs ;
	    rs = msgid_buf(op,eoff,len,&bp) ;
	    if (rs < op->ebs) break ;

	    n = rs / op->ebs ;
	    for (i = 0 ; i < n ; i += 1) {
	        bep = bp + (i * op->ebs) ;
	        eidp = bep + MSGIDE_OMESSAGEID ;
	        f = matfield(midp,midl,eidp,MSGIDE_LMESSAGEID) ;
	        if (f) break ;
	        ei += 1 ;
	    } /* end for */

	} /* end while */

	if ((rs >= 0) && f) {
	    *bepp = bep ;
	}

	if ((rs >= 0) && (! f)) {
	    rs = SR_NOTFOUND ;
	}

	return (rs >= 0) ? ei : rs ;
}
/* end subroutine (msgid_searchid) */


/* search by recipient AND message-id */
static int msgid_search(MSGID *op,MSGID_KEY *kp,uint khash,char **bepp)
{
	int		rs = SR_OK ;
	int		eoff ;
	int		len ;
	int		n ;
	int		i ;
	int		ne = 100 ;
	int		ei = 0 ;
	int		f = FALSE ;
	char		*bp, *bep ;

#if	CF_DEBUGS
	debugprintf("msgid_search: recip=%s\n", kp->recip) ;
	debugprintf("msgid_search: mid=%s\n", kp->mid) ;
#endif

	while (! f) {

	    eoff = MSGID_FOTAB + (ei * op->ebs) ;
	    len = ne * op->ebs ;
	    rs = msgid_buf(op,eoff,len,&bp) ;

#if	CF_DEBUGS
	    debugprintf("msgid_search: msgid_buf() rs=%d ei=%u\n",rs,ei) ;
#endif

	    if (rs < op->ebs) break ;

	    bep = bp ;
	    n = rs / op->ebs ;

#if	CF_DEBUGS
	    debugprintf("msgid_search: looping n=%u ei=%u\n",n,ei) ;
#endif

	    for (i = 0 ; i < n ; i += 1) {

#if	CF_DEBUGS
	        {
	            MSGIDE_ALL	m0 ;
	            int		rs1 ;
	            debugprintf("msgid_search: ei=%u\n",ei) ;
	            debugprintf("msgid_search: 1 recip=%t\n",
	                (bep + MSGIDE_ORECIPIENT),
	                strnlen((bep + MSGIDE_ORECIPIENT),MSGIDE_LRECIPIENT)) ;
	            debugprintf("msgid_search: 1 mid=%t\n",
	                (bep + MSGIDE_OMESSAGEID),
	                strnlen((bep + MSGIDE_OMESSAGEID),MSGIDE_LMESSAGEID)) ;
	            rs1 = msgide_all(&m0,1,bep,MSGIDE_SIZE) ;
	            debugprintf("msgid_search: 2 rs1=%d count=%u\n",
	                rs1,m0.count) ;
#ifdef	COMMENT
	            debugprintf("msgid_search: 2 recip=%s mid=%s\n",
	                m0.recipient,m0.messageid) ;
#endif /* COMMENT */

	        }
#endif /* CF_DEBUGS */

#if	CF_HASH

	        {
	            uint	uiw ;
	            stdorder_ruint((bep + MSGIDE_OHASH),&uiw) ;
	            f = (khash == uiw) ;
	        }

	        if (f) {
	            f = emat_recipid(bep,kp) ;
	            if (f) break ;
	        }

#else /* CF_HASH */

	        f = emat_recipid(bep,kp) ;
	        if (f) break ;
#endif /* CF_HASH */

	        bep += op->ebs ;
	        ei += 1 ;

	    } /* end for */

	} /* end while */

	*bepp = bep ;
	if ((rs >= 0) && (! f)) {
	    rs = SR_NOTFOUND ;
	}

	return (rs >= 0) ? ei : rs ;
}
/* end subroutine (msgid_search) */


/* find an empty slot */
static int msgid_searchempty(MSGID *op,struct oldentry *oep,char **bepp)
{
	uint		ra, rb ;
	int		rs = SR_NOTFOUND ;
	int		ei_start = 0 ;
	int		ei_mark ;
	int		n = 0 ;
	int		ei = 0 ;

#if	CF_DEBUGS
	debugprintf("msgif_searchempty: filesize=%u nentries=%u\n",
	    op->filesize,op->h.nentries) ;
#endif

	oep->utime = INT_MAX ;
	oep->ei = 0 ;

	if (op->b.off >= MSGID_FLTOP) {

	    ra = uceil((op->b.off - MSGID_FLTOP),op->ebs) ;

	    rb = ufloor((op->b.off + op->b.len - MSGID_FLTOP),op->ebs) ;

	    ei_start = ra / op->ebs ;
	    n = (rb > ra) ? ((rb - ra) / op->ebs) : 0 ;

	} /* end if */

	ei_mark = ei_start ;
	if (n > 0) {

#if	CF_DEBUGS
	    debugprintf("msgif_searchempty: 1 range %u:%u\n",
	        ei_start,n) ;
#endif

	    rs = msgid_searchemptyrange(op,ei_start,n,oep,bepp) ;
	    ei = rs ;
	    if (rs < 0)
	        ei_start += n ;

	} /* end if */

	n = (op->h.nentries - ei_start) ;
	if ((rs == SR_NOTFOUND) && (n > 0)) {


#if	CF_DEBUGS
	    debugprintf("msgif_searchempty: 2 range %u:%u\n",
	        ei_start,n) ;
#endif

	    rs = msgid_searchemptyrange(op,ei_start,n,oep,bepp) ;
	    ei = rs ;

#if	CF_DEBUGS
	    debugprintf("msgif_searchempty: msgid_searchemptyrange() rs=%d\n",
	        rs) ;
#endif

	} /* end if */

	if ((rs == SR_NOTFOUND) && (ei_mark > 0)) {

	    ei_start = 0 ;
	    n = ei_mark ;

#if	CF_DEBUGS
	    debugprintf("msgif_searchempty: 3 range %u:%u\n",
	        ei_start,n) ;
#endif

	    rs = msgid_searchemptyrange(op,ei_start,n,oep,bepp) ;
	    ei = rs ;

	} /* end if */

#if	CF_DEBUGS
	debugprintf("msgif_searchempty: ret rs=%d ei=%u\n",rs,ei) ;
#endif

	return (rs >= 0) ? ei : rs ;
}
/* end subroutine (msgid_searchempty) */


/* search for an empty slot in a specified range of entries */
static int msgid_searchemptyrange(MSGID *op,int ei,int nmax,
		struct oldentry *oep,char **bepp)
{
	time_t		t ;
	int		rs = SR_OK ;
	int		eoff ;
	int		len ;
	int		i ;
	int		ne, n ;
	int		f = FALSE ;
	char		*bp, *bep, *eidp ;

	while (! f) {

	    ne = MIN(nmax,100) ;

	    eoff = MSGID_FOTAB + (ei * op->ebs) ;
	    len = ne * op->ebs ;
	    rs = msgid_buf(op,eoff,len,&bp) ;
	    if (rs < op->ebs) break ;

	    n = (rs / op->ebs) ;
	    for (i = 0 ; i < n ; i += 1) {
	        bep = bp + (i * op->ebs) ;
	        eidp = bep + MSGIDE_OMESSAGEID ;
	        f = (*eidp == '\0') ;
	        if (f) break ;
	        if ((t = extutime(bep)) < oep->utime) {
	            oep->utime = t ;
	            oep->ei = ei ;
	        }
	        ei += 1 ;
	    } /* end for */

	} /* end while */

	*bepp = bep ;
	if ((rs >= 0) && (! f)) {
	    rs = SR_NOTFOUND ;
	}

	return (rs >= 0) ? ei : rs ;
}
/* end subroutine (msgid_searchemptyrange) */


/* buffer mangement stuff */
static int msgid_bufbegin(MSGID *op)
{
	const int	size =  MSGID_BUFSIZE ;
	int		rs ;
	char		*bp ;

	op->b.off = 0 ;
	op->b.len = 0 ;
	op->b.size = 0 ;
	op->b.buf = NULL ;
	if ((rs = uc_malloc(size,&bp)) >= 0) {
	    op->b.size = size ;
	    op->b.buf = bp ;
	}

	return rs ;
}
/* end subroutine (msgid_bufbegin) */


static int msgid_bufend(MSGID *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op->b.buf != NULL) {
	    rs1 = uc_free(op->b.buf) ;
	    if (rs >= 0) rs = rs1 ;
	    op->b.buf = NULL ;
	}

	op->b.size = 0 ;
	op->b.len = 0 ;
	return rs ;
}
/* end subroutine (msgid_bufend) */


/* try to buffer up some of the file */
static int msgid_buf(MSGID *op,int roff,int rlen,char **rpp)
{
	offset_t	poff ;
	uint		bend, fext ;
	uint		foff ;
	uint		rext = (roff + rlen), ext ;
	int		rs = SR_OK ;
	int		len = 0 ;
	char		*rbuf ;

#if	CF_DEBUGS
	debugprintf("msgid_buf: ent roff=%u rlen=%u\n",roff,rlen) ;
	debugprintf("msgid_buf: b.size=%u b.off=%u b.len=%u \n",
	    op->b.size,op->b.off,op->b.len) ;
#endif

/* do we need to read in more data? */

	len = rlen ;
	fext = op->b.off + op->b.len ;
	if ((roff < op->b.off) || (rext > fext)) {

#if	CF_DEBUGS
	    debugprintf("msgid_buf: need more data\n") ;
#endif

/* can we do an "add-on" type read operation? */

	    bend = op->b.off + op->b.size ;
	    if ((roff >= op->b.off) &&
	        (rext <= bend)) {

#if	CF_DEBUGS
	        debugprintf("msgid_buf: add-on read\n") ;
#endif

	        foff = op->b.off + op->b.len ;
	        rbuf = op->b.buf + op->b.len ;

	        ext = roff + MAX(rlen,MSGID_READSIZE) ;
	        fext = uceil(ext,op->pagesize) ;

	        if (fext > bend)
	            fext = bend ;

	        len = fext - foff ;

#if	CF_DEBUGS
	        debugprintf("msgid_buf: u_pread() foff=%u len=%u\n",
	            foff,len) ;
#endif

	        poff = foff ;
	        rs = u_pread(op->fd,rbuf,len,poff) ;

#if	CF_DEBUGS
	        debugprintf("msgid_buf: u_pread() rs=%d\n",rs) ;
#endif

	        if (rs >= 0) {

	            op->b.len += rs ;
	            len = MIN(((op->b.off + op->b.len) - roff),rlen) ;

#if	CF_DEBUGS
	            debugprintf("msgid_buf: len=%d\n",len) ;
#endif

	        } /* end if */

	    } else {

#if	CF_DEBUGS
	        debugprintf("msgid_buf: fresh read\n") ;
#endif

	        op->b.off = roff ;
	        op->b.len = 0 ;

	        bend = roff + op->b.size ;

	        foff = roff ;
	        rbuf = op->b.buf ;

	        ext = roff + MAX(rlen,MSGID_READSIZE) ;
	        fext = uceil(ext,op->pagesize) ;

	        if (fext > bend)
	            fext = bend ;

	        len = fext - foff ;
	        if ((rs = u_pread(op->fd,op->b.buf,len,foff)) >= 0) {
	            op->b.len = rs ;
	            len = MIN(rs,rlen) ;
	        }

	    } /* end if */

	} /* end if (needed to read more data) */

	if (rpp != NULL) {
	    *rpp = op->b.buf + (roff - op->b.off) ;
	}

#if	CF_DEBUGS
	debugprintf("msgid_buf: ret rs=%d len=%u\n",rs,len) ;
#endif

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (msgid_buf) */


/* update the file buffer from a user supplied source */
static int msgid_bufupdate(MSGID *op,int roff,int rbuflen,cchar *rbuf)
{
	int		boff, bext ;
	int		rext = (roff + rbuflen) ;
	int		buflen, bdiff ;
	int		f_done = FALSE ;

	buflen = op->b.len ;
	boff = op->b.off ;
	bext = boff + buflen ;

	if (roff < boff) {

	    if (rext > boff) {
	        rbuf += (boff - roff) ;
	        rbuflen -= (boff - roff) ;
	        roff = boff ;
	    } else {
		rbuflen = 0 ;
		f_done = TRUE ;
	    }

	} /* end if */

	if ((! f_done) && (rext > bext)) {

	    if (roff < bext) {
	        rbuflen -= (rext - bext) ;
	        rext = bext ;
	    } else {
		rbuflen = 0 ;
		f_done = TRUE ;
	    }

	} /* end if */

	if ((! f_done) && (rbuflen > 0)) {
	    bdiff = roff - boff ;
	    memcpy((op->b.buf + bdiff),rbuf,rbuflen) ;
	}

	return rbuflen ;
}
/* end subroutine (msgid_bufupdate) */


/* write out the file header */
static int msgid_writehead(MSGID *op)
{
	offset_t	uoff = MSGID_FOHEAD ;
	int		rs ;
	int		bl ;
	char		fbuf[MSGID_FBUFLEN + 1] ;

	bl = filehead(fbuf,0,&op->h) ;
	rs = u_pwrite(op->fd,fbuf,bl,uoff) ;

	return rs ;
}
/* end subroutine (msgid_writehead) */


static int filemagic(char *buf,int f_read,MSGID_FM *mp)
{
	int		rs = 20 ;
	char		*bp = buf ;

	if (buf == NULL) return SR_BADFMT ;

	if (f_read) {
	    char	*ep ;
	    bp[15] = '\0' ;
	    strncpy(mp->magic,bp,15) ;
	    if ((ep = strchr(mp->magic,'\n')) != NULL) {
	        *ep = '\0' ;
	    }
	    bp += 16 ;
	    memcpy(mp->vetu,bp,4) ;
	} else {
	    bp = strwcpy(bp,mp->magic,14) ;
	    *bp++ = '\n' ;
	    memset(bp,0,(16 - (bp - buf))) ;
	    bp = (buf + 16) ;
	    memcpy(bp,mp->vetu,4) ;
	} /* end if */

	return rs ;
}
/* end subroutine (filemagic) */


/* encode or decode the file header */
static int filehead(char *mbuf,int f_read,MSGID_FH *hp)
{
	SERIALBUF	msgbuf ;
	const int	mlen = sizeof(MSGID_FH) ;
	int		rs ;
	int		rs1 ;

	if ((rs = serialbuf_start(&msgbuf,mbuf,mlen)) >= 0) {

	    if (f_read) { /* read */
	        serialbuf_ruint(&msgbuf,&hp->wcount) ;
	        serialbuf_ruint(&msgbuf,&hp->wtime) ;
	        serialbuf_ruint(&msgbuf,&hp->nentries) ;
	    } else { /* write */
	        serialbuf_wuint(&msgbuf,hp->wcount) ;
	        serialbuf_wuint(&msgbuf,hp->wtime) ;
	        serialbuf_wuint(&msgbuf,hp->nentries) ;
	    } /* end if */

	    rs1 = serialbuf_finish(&msgbuf) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (serialbuf) */

	return rs ;
}
/* end subroutine (filehead) */


static int emat_recipid(cchar *ep,MSGID_KEY *kp)
{
	int		f ;
	cchar		*fp ;

	fp = ep + MSGIDE_OMESSAGEID ;
	f = matfield(kp->mid,-1,fp,MSGIDE_LMESSAGEID) ;

	if (f) {
	    fp = ep + MSGIDE_ORECIPIENT ;
	    f = matfield(kp->recip,-1,fp,MSGIDE_LRECIPIENT) ;
	}

	return f ;
}
/* end subroutine (emat_recipid) */


static int matfield(cchar *mp,int ml,cchar *ep,int el)
{
	int		f ;

	if (ml >= 0) {
	    f = (strncmp(mp,ep,MIN(ml,el)) == 0) ;
	    if (f && (ml < el)) {
	        f = (ep[ml] == '\0') ;
	    }
	} else {
	    f = (strncmp(mp,ep,el) == 0) ;
	}

	return f ;
}
/* end subroutine (matfield) */


static int extutime(char *ep)
{
	MSGIDE_UPDATE	m1 ;
	int		rs ;
	rs = msgide_update(&m1,1,ep,MSGIDE_SIZE) ;
	return (rs >= 0) ? m1.utime : rs ;
}
/* end subroutine (extutime) */


static uint recipidhash(MSGID_KEY *kp,int reciplen,int midlen)
{
	uint		khash = hashelf(kp->recip,reciplen) ;
	khash += hashelf(kp->mid,midlen) ;
	return khash ;
}
/* end subroutine (recipidhash) */


