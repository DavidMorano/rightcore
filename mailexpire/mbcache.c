/* mbcache (MailBox-Cache) */

/* cache mailbox items */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */
#define	CF_LOADENVADDR	0		/* compile |mbcache_loadenvaddr()| */


/* revision history:

	= 2009-01-10, David A­D­ Morano
        This module is an attempt to take some of the garbage out of the
        piece-of-total-garbage VMAIL program.


*/

/* Copyright © 2009 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This object parses out and then caches information about messages
	within a mailbox.  All information saved about each message is stored
	under the index of that message within the original mailbox.

	Implementation notes

	= Things to watch for (or to do in the future)

	This object is a cache of the MAILBOX object, but we provide some
	additional processing that the MAILBOX object does not provide.  For
	the most part, the MAILBOX object contains all read-only data (mail
	message data).  But the deletion status of a mail message is
	potentially dynamic -- meaning that it can be set or cleared after the
	mailbox is opened.  Currently it is assumed that if a message is in a
	mailbox that it is "not deleted."  But this may not be the case in the
	future (messages may be deleted but still in the mailbox).  As a cache,
	this object may need to be enhanced to always get the deletion status
	of a message from the MAILBOX object rather than assuming that it is by
	default "not deleted."


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<limits.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<sbuf.h>
#include	<char.h>
#include	<mailmsg.h>
#include	<mailmsg_enver.h>
#include	<mailmsghdrs.h>
#include	<ema.h>
#include	<vecobj.h>
#include	<tmtime.h>
#include	<mapstrint.h>
#include	<localmisc.h>

#include	"mailbox.h"
#include	"mbcache.h"


/* local defines */

#define	MSGENTRY	MBCACHE_SCAN

#ifndef	LINEBUFLEN
#ifdef	LINE_MAX
#define	LINEBUFLEN	MAX(LINE_MAX,2048)
#else
#define	LINEBUFLEN	2048
#endif
#endif

#ifndef	TMPDNAME
#define	TMPDNAME	"/tmp"
#endif

#ifndef	MAILADDRLEN
#define	MAILADDRLEN	(3 * MAXHOSTNAMELEN)
#endif

#undef	BUFLEN
#define	BUFLEN		MAX(100,MAILADDRLEN)

#ifndef	HDRBUFLEN
#define	HDRBUFLEN	(2*MAILADDRLEN)
#endif

#undef	SCANBUFLEN
#define	SCANBUFLEN	100

#ifndef	DIGBUFLEN
#define	DIGBUFLEN	40		/* can hold int128_t in decimal */
#endif

#define	RECORDFNAME	"envelopes.log"

#define	TI_LOCK		120

#ifndef	COL_SCANFROM
#define	COL_SCANFROM	2
#endif
#ifndef	COL_SCANSUBJECT
#define	COL_SCANSUBJECT	29
#endif
#ifndef	COL_SCANDATE
#define	COL_SCANDATE	60
#endif
#ifndef	COL_SCANLINES
#define	COL_SCANLINES	75
#endif


/* external subroutines */

extern int	sncpy1(char *,int,const char *) ;
extern int	snwcpy(char *,int,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecui(const char *,uint,uint *) ;
extern int	ctdeci(char *,int,int) ;
extern int	mailmsg_loadmb(MAILMSG *,MAILBOX *,offset_t) ;
extern int	hdrextid(char *,int,const char *,int) ;
extern int	mkbestaddr(char *,int,cchar *,int) ;
extern int	mkaddrname(char *,int,cchar *,int) ;
extern int	isOneOf(const int *,int) ;
extern int	isNotPresent(int) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strwcpyblanks(char *,int) ;
extern char	*timestr_scandate(time_t,char *) ;


/* external variables */


/* local structures */

struct scantitle {
	char		*name ;
	int		col ;
} ;


/* forward references */

static int mbcache_msgfins(MBCACHE *) ;
static int mbcache_msgframing(MBCACHE *,int,MSGENTRY **) ;
static int mbcache_msgtimers(MBCACHE *,int,time_t *) ;

#ifdef	COMMENT
static int mbcache_msgscanmk(MBCACHE *,int) ;
#endif

static int msgentry_start(MSGENTRY *,int) ;
static int msgentry_frame(MSGENTRY *,MAILBOX_MSGINFO *) ;
static int msgentry_load(MSGENTRY *,MBCACHE *) ;
static int msgentry_loadenv(MSGENTRY *,MBCACHE *,MAILMSG *) ;
static int msgentry_loadhdrmid(MSGENTRY *,MBCACHE *,MAILMSG *) ;
static int msgentry_loadhdrfrom(MSGENTRY *,MBCACHE *,MAILMSG *) ;
static int msgentry_loadhdrsubject(MSGENTRY *,MBCACHE *,MAILMSG *) ;
static int msgentry_loadhdrdate(MSGENTRY *,MBCACHE *,MAILMSG *) ;
static int msgentry_loadhdrstatus(MSGENTRY *,MBCACHE *,MAILMSG *) ;
static int msgentry_procenvdate(MSGENTRY *,MBCACHE *) ;
static int msgentry_prochdrdate(MSGENTRY *,MBCACHE *) ;
static int msgentry_msgtimes(MSGENTRY *,MBCACHE *) ;
static int msgentry_procscanfrom(MSGENTRY *,MBCACHE *) ;
static int msgentry_procscandate(MSGENTRY *,MBCACHE *) ;
static int msgentry_finish(MSGENTRY *) ;

#if	CF_LOADENVADDR
static int msgentry_loadenvaddr(MSGENTRY *,MBCACHE *,MAILMSG *,const char **) ;
#endif /* CF_LOADENVADDR */

#ifdef	COMMENT
static int	headappend(char **,char *,int) ;
#endif

static int	isNoMsg(int) ;
static int	isBadDate(int) ;

static int	vcmpmsgentry(const void *,const void *) ;

#if	CF_DEBUGS
static int debugprintlines(const char *,const char *,int,int) ;
#endif /* CF_DEBUGS */


/* local variables */

static const int	rsdatebad[] = {
	SR_INVALID,
	SR_DOM,
	SR_NOMSG,
	0
} ;

static const int	rsnomsg[] = {
	SR_NOMSG,
	SR_NOENT,
	0
} ;


#ifdef	COMMENT
static const struct scantitle	scantitles[] = {
	{ "FROM", COL_SCANFROM },
	{ "SUBJECT", COL_SCANSUBJECT },
	{ "DATE", COL_SCANDATE },
	{ "LINES", COL_SCANLINES },
	{ NULL, 0 }
} ;
#endif /* COMMENT */


/* exported subroutines */


int mbcache_start(MBCACHE *op,cchar *mbfname,int mflags,MAILBOX *mbp)
{
	STRPACK		*psp ;
	int		rs = SR_OK ;
	int		nmsgs = 0 ;
	const char	*cp ;

	if (op == NULL) return SR_FAULT ;
	if (mbfname == NULL) return SR_FAULT ;
	if (mbp == NULL) return SR_FAULT ;

	if (mbfname[0] == '\0') return SR_INVALID ;

#if	CF_DEBUGS
	debugprintf("mbcache_start: mbfname=%s\n",mbfname) ;
#endif

	memset(op,0,sizeof(MBCACHE)) ;
	op->mflags = mflags ;
	op->mbp = mbp ;

	op->f.readonly = (! (mflags & MBCACHE_ORDWR)) ;

	psp = &op->strs ;
	if ((rs = uc_mallocstrw(mbfname,-1,&cp)) >= 0) {
	    MAILBOX_INFO	*mip = &op->mbi ;
	    op->mbfname = cp ;
	    if ((rs = mailbox_info(mbp,mip)) >= 0) {
	        const int	mssize = sizeof(MBCACHE_SCAN **) ;
	        if (mip->nmsgs >= 0) {
	            const int	size = ((mip->nmsgs + 1) * mssize) ;
	            void	*p ;
	            nmsgs = mip->nmsgs ;
	            if ((rs = uc_malloc(size,&p)) >= 0) {
	                const int	csize = (mip->nmsgs * 6 * 20) ;
	                op->msgs = p ;
	                memset(op->msgs,0,size) ;
	                if ((rs = strpack_start(psp,csize)) >= 0) {
			    if ((rs = dater_start(&op->dm,NULL,NULL,0)) >= 0) {
	    			op->magic = MBCACHE_MAGIC ;
			    }
	                    if (rs < 0)
	                        strpack_finish(psp) ;
	                } /* end if (strpack_start) */
	                if (rs < 0) {
	                    uc_free(op->msgs) ;
	                    op->msgs = NULL ;
	                }
	            } /* end if (memory-allocation) */
	        } else {
	            rs = SR_NOANODE ;
		}
	    }
	    if (rs < 0) {
	        uc_free(op->mbfname) ;
	        op->mbfname = NULL ;
	    }
	} /* end if (memory-allocation) */

#if	CF_DEBUGS
	debugprintf("mbcache_start: ret rs=%d n=%u\n",rs,nmsgs) ;
#endif

	return (rs >= 0) ? nmsgs : rs ;
}
/* end subroutine (mbcache_start) */


int mbcache_finish(MBCACHE *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != MBCACHE_MAGIC) return SR_NOTOPEN ;

	rs1 = mbcache_msgfins(op) ;
	if (rs >= 0) rs = rs1 ;

#if	CF_DEBUGS
	debugprintf("mbcache_finish: _msgfins() rs=%d\n",rs) ;
#endif

	rs1 = dater_finish(&op->dm) ;
	if (rs >= 0) rs = rs1 ;

#if	CF_DEBUGS
	debugprintf("mbcache_finish: dater_finish() rs=%d\n",rs) ;
#endif

	rs1 = strpack_finish(&op->strs) ;
	if (rs >= 0) rs = rs1 ;

#if	CF_DEBUGS
	debugprintf("mbcache_finish: strpack_finish() rs=%d\n",rs) ;
#endif

	if (op->msgs != NULL) {
	    rs1 = uc_free(op->msgs) ;
	    if (rs >= 0) rs = rs1 ;
	    op->msgs = NULL ;
	}

#if	CF_DEBUGS
	debugprintf("mbcache_finish: msgs rs=%d\n",rs) ;
#endif

	if (op->mbfname != NULL) {
	    rs1 = uc_free(op->mbfname) ;
	    if (rs >= 0) rs = rs1 ;
	    op->mbfname = NULL ;
	}

#if	CF_DEBUGS
	debugprintf("mbcache_finish: mbfname rs=%d\n",rs) ;
#endif

	op->mbp = NULL ;
	op->magic = 0 ;

#if	CF_DEBUGS
	debugprintf("mbcache_finish: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (mbcache_finish) */


/* get the mail filename */
int mbcache_mbfile(MBCACHE *op,char *dbuf,int dlen)
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;
	if (dbuf == NULL) return SR_FAULT ;

	if (op->magic != MBCACHE_MAGIC) return SR_NOTOPEN ;

	rs = sncpy1(dbuf,dlen,op->mbfname) ;

	return rs ;
}
/* end subroutine (mbcache_mbfile) */


/* get information */
int mbcache_mbinfo(MBCACHE *op,MBCACHE_INFO *mep)
{
	int		rs ;
	int		nmsgs = 0 ;

	if (op == NULL) return SR_FAULT ;
	if (mep == NULL) return SR_FAULT ;

	if (op->magic != MBCACHE_MAGIC) return SR_NOTOPEN ;

/* fill it in */

	memset(mep,0,sizeof(MBCACHE_INFO)) ;

	rs = mailbox_countdel(op->mbp) ;
	mep->nmsgdels = rs ;
	mep->nmsgs = op->mbi.nmsgs ;
	nmsgs = op->mbi.nmsgs ;

	return (rs >= 0) ? nmsgs : rs ;
}
/* end subroutine (mbcache_mbinfo) */


/* get count of messages */
int mbcache_count(MBCACHE *op)
{
	int		nmsgs ;

	if (op == NULL) return SR_FAULT ;
	if (op->magic != MBCACHE_MAGIC) return SR_NOTOPEN ;

	nmsgs = op->mbi.nmsgs ;
	return nmsgs ;
}
/* end subroutine (mbcache_count) */


/* sort the message by date (oldest first) */
int mbcache_sort(MBCACHE *op)
{
	int		rs = SR_OK ;
	int		nmsgs ;
	int		mi ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != MBCACHE_MAGIC) return SR_NOTOPEN ;

	nmsgs = op->mbi.nmsgs ;

#if	CF_DEBUGS
	debugprintf("mbcache_sort: nmsgs=%u\n",nmsgs) ;
#endif

	for (mi = 0 ; (rs >= 0) && (mi < nmsgs) ; mi += 1) {
	    rs = mbcache_msgtimers(op,mi,NULL) ;
	} /* end for */

	if (rs >= 0) {
	    const int	qsize = sizeof(const char *) ;
	    void	*msgs = (void *) op->msgs ;
#if	CF_DEBUGS
	    debugprintf("mbcache_sort: qsort()\n") ;
#endif
	    qsort(msgs,nmsgs,qsize,vcmpmsgentry) ;
	}

	return (rs >= 0) ? nmsgs : rs ;
}
/* end subroutine (mbcache_sort) */


#ifdef	COMMENT

/* check whatever */
int mbcache_check(MBCACHE *op,struct timeb *nowp,cchar *zname)
{
	int		rs = SR_OK ;

	if (op == NULL) return SR_FAULT ;
	if (nowp == NULL) return SR_FAULT ;

	if (op->magic != MBCACHE_MAGIC) return SR_NOTOPEN ;

	op->now = *nowp ;
	strncpy(op->zname,zname,DATER_ZNAMESIZE) ;

	op->f.now = TRUE ;

/* do anything ? */

	if (! op->f.tmpdate) {
	    op->f.tmpdate = TRUE ;
	    rs = dater_start(&op->tmpdate,&op->now,op->zname,-1) ;
	} else {
	    op->tmpdate.cb = *nowp ;
	    strncpy(op->tmpdate.cname,zname,DATER_ZNAMESIZE) ;
	} /* end if */

	return rs ;
}
/* end subroutine (mbcache_check) */

#endif /* COMMENT */


/* mark a message for deletion */
int mbcache_msgdel(MBCACHE *op,int mi,int delcmd)
{
	int		rs = SR_OK ;
	int		nmsgs ;
	int		f_delprev = FALSE ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != MBCACHE_MAGIC) return SR_NOTOPEN ;

	nmsgs = op->mbi.nmsgs ;
	if ((mi < 0) || (mi >= nmsgs)) return SR_NOMSG ;

#if	CF_DEBUGS
	debugprintf("mbcache_msgdel: ent mi=%d delcmd=%d\n",mi,delcmd) ;
#endif

	if ((delcmd >= 0) && op->f.readonly)
	    return SR_ROFS ;

/* if this is only an inquiry, we may have a short cut (not really valid) */

	if ((delcmd >= 0) || (op->msgs[mi] != NULL)) {
	    MBCACHE_SCAN	*mep ;
	    if ((rs = mbcache_msgframing(op,mi,&mep)) >= 0) {
	        f_delprev = mep->f.del ;
	        if (delcmd >= 0) {
	            if (! LEQUIV(mep->f.del,delcmd)) {
	                mep->f.del = (delcmd) ? TRUE : FALSE ;
	                rs = mailbox_msgdel(op->mbp,mi,delcmd) ;
#if	CF_DEBUGS
	                debugprintf("mbcache_msgdel: mailbox_msgdel() rs=%d\n",
		                rs) ;
#endif
	            }
	        } /* end if */
	    } /* end if */
	} /* end if (short cut) */

#if	CF_DEBUGS
	debugprintf("mbcache_msgdel: ret rs=%d f_delprev=%u\n",rs,f_delprev) ;
#endif

	return (rs >= 0) ? f_delprev : rs ;
}
/* end subroutine (mbcache_msgdel) */


/* delete all duplicate messages */
int mbcache_msgdeldup(MBCACHE *op)
{
	MAPSTRINT	mm ;
	int		rs ;
	int		nmsgs ;
	int		c = 0 ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != MBCACHE_MAGIC) return SR_NOTOPEN ;

	nmsgs = op->mbi.nmsgs ;

#if	CF_DEBUGS
	debugprintf("mbcache_msgdeldup: ent nmsgs=%u\n",nmsgs) ;
#endif

	if ((rs = mapstrint_start(&mm,nmsgs)) >= 0) {
	    MBCACHE_SCAN	*mep ;
	    const int		vi = mbcachemf_hdrmid ;
	    int			mi ;
	    int			midl ;
	    const char		*midp ;
	    for (mi = 0 ; mi < nmsgs ; mi += 1) {
	        if ((rs = mbcache_msginfo(op,mi,&mep)) >= 0) {
		    if (! mep->f.del) {
	                midp = mep->vs[vi] ;
	                midl = mep->vl[vi] ;
#if	CF_DEBUGS
		        debugprintf("mbcache_msgdeldup: mid=%t\n",
			    midp,strlinelen(midp,midl,40)) ;
#endif
		        if ((midp != NULL) && (midl > 0)) {
		            if ((rs = mapstrint_already(&mm,midp,midl)) >= 0) {
			        c += 1 ;
	                        mep->f.del = TRUE ;
	                        rs = mailbox_msgdel(op->mbp,mi,TRUE) ;
		            } else if (rs == SR_NOTFOUND) {
		                rs = mapstrint_add(&mm,midp,midl,mi) ;
#if	CF_DEBUGS
		            debugprintf("mbcache_msgdeldup: added rs=%d\n",rs) ;
#endif
		            }
		        } /* end if (non-null) */
#if	CF_DEBUGS
		        debugprintf("mbcache_msgdeldup: mid rs=%d\n",rs) ;
#endif
		    } /* end if (not already being deleted) */
	        } /* end (_msginfo) */
		if (rs < 0) break ;
	    } /* end for */
	    mapstrint_finish(&mm) ;
	} /* end if (mapstrint) */

#if	CF_DEBUGS
	debugprintf("mbcache_msgdeldup: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (mbcache_msgdeldup) */


/* retrieve MSG flags (dispositions) */
int mbcache_msgflags(MBCACHE *op,int mi)
{
	MBCACHE_SCAN	*mep ;
	int		rs ;
	int		nmsgs ;
	int		mf = 0 ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != MBCACHE_MAGIC) return SR_NOTOPEN ;

	nmsgs = op->mbi.nmsgs ;
	if ((mi < 0) || (mi >= nmsgs)) return SR_NOMSG ;

#if	CF_DEBUGS
	debugprintf("mbcache_msgflags: ent mi=%d\n",mi) ;
#endif

	if ((rs = mbcache_msgframing(op,mi,&mep)) >= 0) {
	    if (mep->f.read) mf |= MBCACHE_MFMREAD ;
	    if (mep->f.del) mf |= MBCACHE_MFMDEL ;
	    if (mep->f.spam) mf |= MBCACHE_MFMSPAM ;
	    if (mep->f.trash) mf |= MBCACHE_MFMTRASH ;
	} /* end if */

#if	CF_DEBUGS
	debugprintf("mbcache_msgflags: ret rs=%d\n",rs) ;
#endif

	return (rs >= 0) ? mf : rs ;
}
/* end subroutine (mbcache_msgflags) */


/* set MSG flags (dispositions) */
int mbcache_msgsetflag(MBCACHE *op,int mi,int w,int v)
{
	MBCACHE_SCAN	*mep ;
	int		rs ;
	int		nmsgs ;
	int		mf = 0 ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != MBCACHE_MAGIC) return SR_NOTOPEN ;

	nmsgs = op->mbi.nmsgs ;
	if ((mi < 0) || (mi >= nmsgs)) return SR_NOMSG ;

#if	CF_DEBUGS
	debugprintf("mbcache_msgsetflag: ent mi=%d\n",mi) ;
#endif

	if ((rs = mbcache_msgframing(op,mi,&mep)) >= 0) {
	    switch (w) {
	    case MBCACHE_MFVREAD:
	        mf = mep->f.read ;
	        mep->f.read = (v>0) ;
	        break ;
	    case MBCACHE_MFVDEL:
	        mf = mep->f.del ;
	        mep->f.del = (v>0) ;
	        break ;
	    case MBCACHE_MFVSPAM:
	        mf = mep->f.spam ;
	        mep->f.spam = (v>0) ;
	        break ;
	    case MBCACHE_MFVTRASH:
	        mf = mep->f.trash ;
	        mep->f.trash = (v>0) ;
	        break ;
	    } /* end switch */
	} /* end if */

#if	CF_DEBUGS
	debugprintf("mbcache_msgsetflag: ret rs=%d\n",rs) ;
#endif

	return (rs >= 0) ? mf : rs ;
}
/* end subroutine (mbcache_msgsetflag) */


/* get count of deleted messages */
int mbcache_countdel(MBCACHE *op)
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != MBCACHE_MAGIC) return SR_NOTOPEN ;

	rs = mailbox_countdel(op->mbp) ;

	return rs ;
}
/* end subroutine (mbcache_countdel) */


int mbcache_msgsetlines(MBCACHE *op,int mi,int vlines)
{
	MBCACHE_SCAN	*mep ;
	int		rs ;
	int		rlines = 0 ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != MBCACHE_MAGIC) return SR_NOTOPEN ;

	if ((rs = mbcache_msgframing(op,mi,&mep)) >= 0) {
	    if (vlines >= 0) {
	        if (mep->vlines < 0) {
	            mep->vlines = vlines ;
		}
	        rlines = mep->vlines ;
	    } else {
	        if (mep->vlines >= 0) {
	            rlines = mep->vlines ;
	        } else if (mep->nlines >= 0) {
	            rlines = mep->nlines ;
		}
	    }
	} /* end if (mbcache_msgframing) */

	return (rs >= 0) ? rlines : rs ;
}
/* end subroutine (mbcache_msgsetlines) */


/* get the file offset to the start-envelope of a message */
int mbcache_msgoff(MBCACHE *op,int mi,offset_t *rp)
{
	MBCACHE_SCAN	*mep ;
	int		rs = SR_OK ;
	int		nmsgs ;

	if (op == NULL) return SR_FAULT ;
	if (rp == NULL) return SR_FAULT ;

	if (op->magic != MBCACHE_MAGIC) return SR_NOTOPEN ;

	nmsgs = op->mbi.nmsgs ;
	if ((mi < 0) || (mi >= nmsgs)) return SR_NOMSG ;

	mep = op->msgs[mi] ;
	if (mep == NULL) {
	    rs = mbcache_msgframing(op,mi,NULL) ;
	    mep = op->msgs[mi] ;
	}

	if (rp != NULL)
	    *rp = (rs >= 0) ? mep->moff : 0 ;

	if (rs >= 0) rs = (mep->mlen & INT_MAX) ;

	return rs ;
}
/* end subroutine (mbcache_msgoff) */


/* get the number of lines in a message */
int mbcache_msglines(MBCACHE *op,int mi,int *rp)
{
	MBCACHE_SCAN	*mep ;
	int		rs = SR_OK ;
	int		nmsgs ;
	int		rlines = -1 ;

	if (op == NULL) return SR_FAULT ;
	if (rp == NULL) return SR_FAULT ;

	if (op->magic != MBCACHE_MAGIC) return SR_NOTOPEN ;

	nmsgs = op->mbi.nmsgs ;
	if ((mi < 0) || (mi >= nmsgs)) return SR_NOMSG ;

	mep = op->msgs[mi] ;
	if (mep == NULL) {
	    rs = mbcache_msgframing(op,mi,NULL) ;
	    mep = op->msgs[mi] ;
	}

	if (rs >= 0) {
	    if (mep->vlines >= 0) {
	        rlines = mep->vlines ;
	    } else if (mep->nlines >= 0) {
	        rlines = mep->nlines ;
	    }
	}

#if	CF_DEBUGS
	debugprintf("mbcache_msglines: ret rs=%d nlines=%d\n",rs,rlines) ;
#endif

	*rp = rlines ;
	return rs ;
}
/* end subroutine (mbcache_msglines) */


/* get the framing information for a message */
int mbcache_msginfo(MBCACHE *op,int mi,MBCACHE_SCAN **mpp)
{
	MBCACHE_SCAN	*mep ;
	int		rs = SR_OK ;
	int		nmsgs ;

	if (op == NULL) return SR_FAULT ;
	if (mpp == NULL) return SR_FAULT ;

	if (op->magic != MBCACHE_MAGIC) return SR_NOTOPEN ;

	if (mpp != NULL) *mpp = NULL ;
	nmsgs = op->mbi.nmsgs ;
	if ((mi < 0) || (mi >= nmsgs)) return SR_NOMSG ;

#if	CF_DEBUGS
	debugprintf("mbcache_msginfo: mi=%u\n",mi) ;
#endif

	mep = op->msgs[mi] ;
	if (mep == NULL) {
	    rs = mbcache_msgframing(op,mi,NULL) ;
	    mep = op->msgs[mi] ;
	}

	if (rs >= 0) {
	    if ((rs = msgentry_load(mep,op)) >= 0) {
	        if (mpp != NULL) *mpp = mep ;
	    }
	}

	return rs ;
}
/* end subroutine (mbcache_msginfo) */


/* get the scan information for a message */
int mbcache_msgscan(MBCACHE *op,int mi,MBCACHE_SCAN **mpp)
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;
	if (mpp == NULL) return SR_FAULT ;

	if (op->magic != MBCACHE_MAGIC) return SR_NOTOPEN ;

#if	CF_DEBUGS
	debugprintf("mbcache_msgscan: ent mi=%u\n",mi) ;
#endif

	*mpp = NULL ;
	if ((rs = mbcache_msginfo(op,mi,mpp)) >= 0) {
	    MBCACHE_SCAN	*mep = *mpp ;
	    if ((rs = msgentry_procscanfrom(mep,op)) >= 0) {
	        rs = msgentry_procscandate(mep,op) ;
	    }
	}

#if	CF_DEBUGS
	debugprintf("mbcache_msgscan: ret rs=%d \n",rs) ;
#endif

	return rs ;
}
/* end subroutine (mbcache_msgscan) */


int mbcache_msgenvtime(MBCACHE *op,int mi,time_t *timep)
{
	MSGENTRY	*mep ;
	time_t		t = 0 ;
	int		rs ;
	int		f = FALSE ;

	if (op == NULL) return SR_FAULT ;
	if (timep == NULL) return SR_FAULT ;

	if ((rs = mbcache_msginfo(op,mi,&mep)) >= 0) {
	    if ((rs = msgentry_procenvdate(mep,op)) >= 0) {
	        t = mep->etime ;
	        f = TRUE ;
	    }
	}

	if (timep != NULL) *timep = t ;
	return (rs >= 0) ? f : rs ;
}
/* end subroutine (mbcache_msgenvtime) */


int mbcache_msghdrtime(MBCACHE *op,int mi,time_t *timep)
{
	MSGENTRY	*mep ;
	time_t		t = 0 ;
	int		rs ;
	int		f = FALSE ;

	if (op == NULL) return SR_FAULT ;
	if (timep == NULL) return SR_FAULT ;

	if ((rs = mbcache_msginfo(op,mi,&mep)) >= 0) {
	    if ((rs = msgentry_prochdrdate(mep,op)) >= 0) {
	        f = TRUE ;
	        t = mep->htime ;
	    }
	}

	if (timep != NULL) *timep = t ;
	return (rs >= 0) ? f : rs ;
}
/* end subroutine (mbcache_msghdrtime) */


int mbcache_msgtimes(MBCACHE *op,int mi,time_t *timep)
{
	MSGENTRY	*mep ;
	int		rs ;

	if (op == NULL) return SR_FAULT ;
	if (timep == NULL) return SR_FAULT ;

	if ((rs = mbcache_msginfo(op,mi,&mep)) >= 0) {
	    if ((rs = msgentry_msgtimes(mep,op)) >= 0) {
	        timep[0] = mep->etime ;
	        timep[1] = mep->htime ;
	    }
	}

	return rs ;
}
/* end subroutine (mbcache_msgtimes) */


/* private subroutines */


static int mbcache_msgfins(MBCACHE *op)
{
	MBCACHE_SCAN	*mep ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		mi ;

	for (mi = 0 ; mi < op->mbi.nmsgs ; mi += 1) {
	    mep = op->msgs[mi] ;
	    if (mep != NULL) {
	        rs1 = msgentry_finish(mep) ;
	        if (rs >= 0) rs = rs1 ;
#if	CF_DEBUGS
	        debugprintf("mbcache_msgfins: mi=%u msgentry_finish() rs=%d\n",
			mi,rs) ;
#endif
	        rs1 = uc_free(mep) ;
	        if (rs >= 0) rs = rs1 ;
	        op->msgs[mi] = NULL ;
#if	CF_DEBUGS
	        debugprintf("mbcache_msgfins: uc_free() rs=%d\n",rs) ;
#endif
	    }
	} /* end for */

#if	CF_DEBUGS
	debugprintf("mbcache_msgfins: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (mbcache_msgfins) */


static int mbcache_msgframing(MBCACHE *op,int mi,MSGENTRY **mpp)
{
	int		rs = SR_OK ;

#if	CF_DEBUGS
	debugprintf("mbcache_msgframing: ent mi=%u\n",mi) ;
#endif

	if (op->msgs[mi] == NULL) {
	    MAILBOX_MSGINFO	minfo ;
	    if ((rs = mailbox_msginfo(op->mbp,mi,&minfo)) >= 0) {
	        MBCACHE_SCAN	*mep = NULL ;
	        const int	size = sizeof(MBCACHE_SCAN) ;

#if	CF_DEBUGS
	        debugprintf("mbcache_msgframing: mailbox_msginfo() rs=%d\n",
	            rs) ;
	        debugprintf("mbcache_msgframing: blen=%d\n",minfo.blen) ;
	        debugprintf("mbcache_msgframing: CLEN hdr=%u hdrval=%u v=%d\n",
	            minfo.hdr.clen,minfo.hdrval.clen,minfo.clen) ;
	        debugprintf("mbcache_msgframing: blen=%d\n",
	            minfo.blen) ;
#endif /* CF_DEBUGS */

	        if ((rs = uc_malloc(size,&mep)) >= 0) {
	            if ((rs = msgentry_start(mep,mi)) >= 0) {
	                if ((rs = msgentry_frame(mep,&minfo)) >= 0) {
	                    op->msgs[mi] = mep ;
	                }
	                if (rs < 0)
	                    msgentry_finish(mep) ;
	            } /* end if (msgentry_start) */
	            if (rs < 0)
	                uc_free(mep) ;
	        } /* end if (memory-allocation) */

	    } /* end if (mailbox_msginfo) */
	} /* end if (needed) */

	if (mpp != NULL)
	    *mpp = (rs >= 0) ? op->msgs[mi] : NULL ;

#if	CF_DEBUGS
	debugprintf("mbcache_msgframing: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (mbcache_msgframing) */


static int mbcache_msgtimers(MBCACHE *op,int mi,time_t *timep)
{
	MSGENTRY	*mep ;
	int		rs ;

	if ((rs = mbcache_msginfo(op,mi,&mep)) >= 0) {
	    if ((rs = msgentry_msgtimes(mep,op)) >= 0) {
	        if (timep != NULL) {
	            timep[0] = mep->etime ;
	            timep[1] = mep->htime ;
	        }
#if	CF_DEBUGS
	        debugprintf("mbcache_msgtimers: mi=%u e=%08x h=%08x\n",
	            mi,mep->etime, mep->htime) ;
#endif
	    }
	}

	return rs ;
}
/* end subroutine (mbcache_msgtimers) */


#ifdef	COMMENT

static int mbcache_msgscanmk(MBCACHE *op,int mi)
{
	MBCACHE_SCAN	*mep ;
	const int	sl = SCANBUFLEN ;
	int		rs = SR_OK ;

	mep = op->msgs[mi] ;
	if (mep != NULL) {
	    if (mep->vs[mbcachemf_scanline] == NULL) {
	        const int	size = (sl + 1) ;
	        char		*bp ;

	        if ((rs = uc_malloc(size,&bp)) >= 0) {
	            int		i ;
	            int		tcol ;
	            int		cl ;
	            const char	*cp ;

	            strwcpyblanks(bp,sl) ;

	            for (i = 0 ; scantitles[i].name != NULL ; i += 1) {
	                cp = scantitles[i].name ;
	                cl = strlen(cp) ;
	                tcol = scantitles[i].col ;
	                strncpy((bp + tcol),cp,cl) ;
	            } /* end for */

	            if (rs >= 0) {
	                mep->vs[mbcachemf_scanline] = bp ;
	            } else
	                uc_free(bp) ;

	        } /* end if (memory-allocation) */

	    } /* end if */
	} /* end if */

	return rs ;
}
/* end subroutine (mbcache_msgscanmk) */

#endif /* COMMENT */


#ifdef	COMMENT

static int mbcache_setnow(MBCACHE *op)
{
	struct timeb	*tbp = &op->now ;
	int		rs = SR_OK ;

	if (! op->f.now) {
	    if ((rs = uc_ftime(tbp)) >= 0) {
	        TMTIME	tmt ;
	        int	zo ;
	        if ((rs = tmtime_localtime(&tmt,tbp->time)) >= 0) {
	            tbp->timezone = (tmt.gmtoff / 60) ;
	            tbp->dstflag = tmt.isdst ;
	            strncpy(op->zname,tmt.zname,DATER_ZNAMESIZE) ;
	            op->f.now = TRUE ;
	        } /* end if */
	    } /* end if (uc_ftime) */
	} /* end if (have-now) */

	return rs ;
}
/* end subroutine (mbcache_setnow) */

#endif /* COMMENT */


static int msgentry_start(MBCACHE_SCAN *mep,int mi)
{

	memset(mep,0,sizeof(MBCACHE_SCAN)) ;
	mep->msgi = mi ;
	mep->nlines = -1 ;
	mep->vlines = -1 ;
	mep->filesize = -1 ;
	mep->moff = -1 ;
	mep->hoff = -1 ;
	mep->boff = -1 ;

	return SR_OK ;
}
/* end subroutine (msgentry_start) */


static int msgentry_finish(MBCACHE_SCAN *mep)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (mep->f.lineoffs) {
	    mep->f.lineoffs = FALSE ;
	    rs1 = vecint_finish(&mep->lineoffs) ;
	    if (rs >= 0) rs = rs1 ;
	}

	if (mep->fname != NULL) {
	    rs1 = uc_free(mep->fname) ;
	    if (rs >= 0) rs = rs1 ;
	    mep->fname = NULL ;
	}

	return rs ;
}
/* end subroutine (msgentry_finish) */


static int msgentry_frame(MSGENTRY *mep,MAILBOX_MSGINFO *mip)
{
	int		rs = SR_OK ;

	if (mep == NULL) return SR_FAULT ;
	if (mip == NULL) return SR_FAULT ;

	mep->moff = mip->moff ;
	mep->hoff = mip->hoff ;
	mep->boff = mip->boff ;
	mep->mlen = mip->mlen ;
	mep->hlen = mip->hlen ;
	mep->blen = mip->blen ;

	mep->hdr.clen = mip->hdr.clen ;
	mep->hdr.clines = mip->hdr.clines ;
	mep->hdr.lines = mip->hdr.lines ;
	mep->hdr.xlines = mip->hdr.xlines ;

	mep->hdrval.clen = mip->hdrval.clen ;
	mep->hdrval.clines = mip->hdrval.clines ;
	mep->hdrval.lines = mip->hdrval.lines ;
	mep->hdrval.xlines = mip->hdrval.xlines ;

#if	CF_DEBUGS
	{
	    const char	*s = "msgentry_load" ;
	    debugprintlines(s,"clines",mip->hdrval.clines,mip->clines) ;
	    debugprintlines(s,"lines",mip->hdrval.lines,mip->lines) ;
	    debugprintlines(s,"xlines",mip->hdrval.xlines,mip->xlines) ;
	}
#endif /* CF_DEBUGS */

	if ((mep->nlines < 0) && mip->hdrval.clines)
	    mep->nlines = mip->clines ;

	if ((mep->nlines < 0) && mip->hdrval.lines)
	    mep->nlines = mip->lines ;

	if ((mep->nlines < 0) && mip->hdrval.xlines)
	    mep->nlines = mip->xlines ;

	return rs ;
}
/* end subroutine (msgentry_frame) */


static int msgentry_load(MSGENTRY *mep,MBCACHE *op)
{
	int		rs = SR_OK ;

	if (! mep->f.info) {
	    MAILMSG	m ;
	    mep->f.info = TRUE ;
	    if ((rs = mailmsg_start(&m)) >= 0) {
	        MAILBOX		*mbp = op->mbp ;
	        offset_t	mbo = mep->moff ;
#if	CF_DEBUGS
	        debugprintf("mbcache_load: mbo=%llu\n",mbo) ;
#endif
	        if ((rs = mailmsg_loadmb(&m,mbp,mbo)) >= 0) {

#if	CF_DEBUGS
	            debugprintf("mbcache_load: mailmsg_loadmb() loaded\n") ;
#endif

	            if (rs >= 0) {
	                rs = msgentry_loadenv(mep,op,&m) ;
	            }

/* MESSAGEID: extract the MAILMSG information that we want */

	            if (rs >= 0) {
	                rs = msgentry_loadhdrmid(mep,op,&m) ;
	            }

/* FROM: extract the MAILMSG information that we want */

	            if (rs >= 0) {
	                rs = msgentry_loadhdrfrom(mep,op,&m) ;
	            }

/* SUBJECT: extract the MAILMSG information that we want */

	            if (rs >= 0) {
	                rs = msgentry_loadhdrsubject(mep,op,&m) ;
	            }

/* DATE: extract the MAILMSG information that we want */

	            if (rs >= 0) {
	                rs = msgentry_loadhdrdate(mep,op,&m) ;
	            }

/* STATUS: extract the MAILMSG information that we want */

	            if (rs >= 0) {
	                rs = msgentry_loadhdrstatus(mep,op,&m) ;
	            }

/* set VS bit */

	            if (rs >= 0) mep->f.vs = TRUE ;

	        } /* end if (mailmsg-loadmb) */
	        mailmsg_finish(&m) ;
	    } /* end if (mailmsg) */
	} /* end if (initialized) */

	return rs ;
}
/* end subroutine (msgentry_load) */


static int msgentry_loadenv(MSGENTRY *mep,MBCACHE *op,MAILMSG *mmp)
{
	int		rs ;

#if	CF_DEBUGS
	{
	    const int	mi = mep->msgi ;
	    debugprintf("msgentry_loadenv: mi=%u\n",mi) ;
	}
#endif

	if ((rs = mailmsg_envcount(mmp)) >= 0) {
	    MAILMSG_ENVER	e ;
	    STRPACK		*psp = &op->strs ;
	    const int		n = rs ;
	    if ((rs = mailmsg_enver(mmp,(n-1),&e)) >= 0) {
	        int		i ;
	        int		vl = -1 ;
	        int		*vlp ;
	        const char	*vp ;
	        const char	**vpp ;
	        for (i = 0 ; (rs >= 0) && (i < 3) ; i += 1) {
	            vp = NULL ;
	            switch (i) {
	            case 0:
	                vpp = (mep->vs + mbcachemf_envaddr) ;
	                vlp = (mep->vl + mbcachemf_envaddr) ;
	                vp = e.a.ep ;
	                vl = e.a.el ;
	                break ;
	            case 1:
	                vpp = (mep->vs + mbcachemf_envdate) ;
	                vlp = (mep->vl + mbcachemf_envdate) ;
	                vp = e.d.ep ;
	                vl = e.d.el ;
#if	CF_DEBUGS
	                debugprintf("msgentry_loadenv: env d=>%t<\n",
	                    vp,strlinelen(vp,vl,40)) ;
#endif
	                break ;
	            case 2:
	                vpp = (mep->vs + mbcachemf_envremote) ;
	                vlp = (mep->vl + mbcachemf_envremote) ;
	                vp = e.r.ep ;
	                vl = e.r.el ;
	                break ;
	            } /* end switch */
	            if (vp != NULL) {
	                rs = strpack_store(psp,vp,vl,vpp) ;
	                *vlp = rs ;
	            }
	        } /* end for */
	    } else if (isNoMsg(rs)) {
#if	CF_DEBUGS
	        debugprintf("msgentry_loadenv: no-env rs=%d\n",rs) ;
#endif
	        rs = SR_OK ;
	    }
	} /* end if (mailmsg_envcount) */

	return rs ;
}
/* end subroutine (msgentry_loadenv) */


#if	CF_LOADENVADDR
static int msgentry_loadenvaddr(mep,op,mmp,rpp)
MSGENTRY	*mep ;
MBCACHE		*op ;
MAILMSG		*mmp ;
const char	**rpp ;
{
	int		rs = SR_OK ;
	int		len = 0 ;

	if (mep->vs[mbcachemf_envaddr] == NULL) {
	    rs = msgentry_loadenv(mep,op,mmp) ;
	}
	if ((rs >= 0) && (rpp != NULL)) {
	    const int	vi = mbcachemf_envaddr ;
	    *rpp = mep->vs[vi] ;
	    len = mep->vl[vi] ;
	} else if (isNoMsg(rs)) {
	    rs = SR_OK ;
	}

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (msgentry_loadenvaddr) */
#endif /* CF_LOADENVADDR */


static int msgentry_loadhdrmid(MSGENTRY *mep,MBCACHE *op,MAILMSG *mmp)
{
	int		rs ;
	int		sl = 0 ;
	const char	*hdr = HN_MESSAGEID ;
	const char	*sp ;

	if ((rs = mailmsg_hdrval(mmp,hdr,&sp)) >= 0) {
	    STRPACK	*psp = &op->strs ;
	    const int	hlen = HDRBUFLEN ;
	    char	hbuf[HDRBUFLEN+1] ;
#if	CF_DEBUGS
	debugprintf("mbcache/msgentry_loadhdrmid: mid1 rs=%d\n",rs) ;
#endif
	    if ((rs = mkbestaddr(hbuf,hlen,sp,rs)) >= 0) {
	        const int	vi = mbcachemf_hdrmid ;
	        const char	**rpp ;
#if	CF_DEBUGS
	debugprintf("mbcache/msgentry_loadhdrmid: mid2 rs=%d\n",rs) ;
#endif
	        sl = rs ;
	        rpp = (mep->vs + vi) ;
	        mep->vl[vi] = sl ;
	        rs = strpack_store(psp,hbuf,sl,rpp) ;
#if	CF_DEBUGS
	debugprintf("mbcache/msgentry_loadhdrmid: mid3 rs=%d\n",rs) ;
#endif
	    } /* end if (mkbestaddr) */
	} else if (isNoMsg(rs)) {
	    rs = SR_OK ;
	}

#if	CF_DEBUGS
	debugprintf("mbcache/msgentry_loadhdrmid: ret rs=%d sl=%u\n",rs,sl) ;
#endif

	return (rs >= 0) ? sl : rs ;
}
/* end subroutine (msgentry_loadhdrmid) */


static int msgentry_loadhdrfrom(MSGENTRY *mep,MBCACHE *op,MAILMSG *mmp)
{
	int		rs ;
	int		vl = 0 ;
	cchar		*hdr = HN_FROM ;
	cchar		*vp ;

	if ((rs = mailmsg_hdrval(mmp,hdr,&vp)) > 0) {
	    vl = rs ;
	} else if ((rs == 0) || isNoMsg(rs)) {
	    hdr = HN_RETURNPATH ;
	    if ((rs = mailmsg_hdrval(mmp,hdr,&vp)) > 0) {
	        vl = rs ;
	    }
	}
	if ((rs >= 0) && (vl >= 0)) {
	    STRPACK	*psp = &op->strs ;
	    const int	vi = mbcachemf_hdrfrom ;
	    const char	**rpp ;
	    rpp = (mep->vs + vi) ;
	    mep->vl[vi] = vl ;
	    rs = strpack_store(psp,vp,vl,rpp) ;
	} else if (isNoMsg(rs)) {
	    rs = SR_OK ;
	}

	return (rs >= 0) ? vl : rs ;
}
/* end subroutine (msgentry_loadhdrfrom) */


static int msgentry_loadhdrsubject(MSGENTRY *mep,MBCACHE *op,MAILMSG *mmp)
{
	int		rs ;
	int		sl = 0 ;
	const char	*hdr = HN_SUBJECT ;
	const char	*sp ;

	if ((rs = mailmsg_hdrval(mmp,hdr,&sp)) >= 0) {
	    STRPACK	*psp = &op->strs ;
	    const int	vi = mbcachemf_hdrsubject ;
	    const char	**rpp ;
	    sl = rs ;
	    rpp = (mep->vs + vi) ;
	    mep->vl[vi] = sl ;
	    rs = strpack_store(psp,sp,sl,rpp) ;
	} else if (isNoMsg(rs)) {
	    rs = SR_OK ;
	}

	return (rs >= 0) ? sl : rs ;
}
/* end subroutine (msgentry_loadhdrsubject) */


static int msgentry_loadhdrdate(MSGENTRY *mep,MBCACHE *op,MAILMSG *mmp)
{
	int		rs ;
	int		sl = 0 ;
	const char	*hdr = HN_DATE ;
	const char	*sp ;

	if ((rs = mailmsg_hdrval(mmp,hdr,&sp)) >= 0) {
	    STRPACK	*psp = &op->strs ;
	    const int	vi = mbcachemf_hdrdate ;
	    const char	**rpp ;
	    sl = rs ;
	    mep->vl[vi] = sl ;
	    rpp = (mep->vs + vi) ;
	    rs = strpack_store(psp,sp,sl,rpp) ;
	} else if (isNoMsg(rs)) {
	    rs = SR_OK ;
	}

	return (rs >= 0) ? sl : rs ;
}
/* end subroutine (msgentry_loadhdrdate) */


static int msgentry_loadhdrstatus(MSGENTRY *mep,MBCACHE *op,MAILMSG *mmp)
{
	int		rs ;
	int		sl = 0 ;
	const char	*hdr = HN_STATUS ;
	const char	*sp ;

	if ((rs = mailmsg_hdrval(mmp,hdr,&sp)) >= 0) {
	    STRPACK	*psp = &op->strs ;
	    const int	vi = mbcachemf_hdrstatus ;
	    const char	**rpp ;
	    sl = rs ;
	    rpp = (mep->vs + vi) ;
	    mep->vl[vi] = sl ;
	    rs = strpack_store(psp,sp,sl,rpp) ;
	} else if (isNoMsg(rs)) {
	    rs = SR_OK ;
	}

	return (rs >= 0) ? sl : rs ;
}
/* end subroutine (msgentry_loadhdrstatus) */


static int msgentry_procenvdate(MSGENTRY *mep,MBCACHE *op)
{
	const int	vi = mbcachemf_envdate ;
	int		rs = SR_OK ;
	int		f = FALSE ;

#if	CF_DEBUGS
	debugprintf("msgentry_procenvdate: ent proc=%u\n",
	    mep->proc.edate) ;
#endif

	if (! mep->proc.edate) {
	    int		sl = mep->vl[vi] ;
	    const char	*sp = mep->vs[vi] ;
	    mep->proc.edate = TRUE ;
#if	CF_DEBUGS
	    debugprintf("msgentry_procenvdate: s=>%t<\n",
	        sp,strlinelen(sp,sl,40)) ;
#endif
	    if ((sp != NULL) && (sl > 0)) {
	        DATER		*dp = &op->dm ;
	        if ((rs = dater_setstd(dp,sp,sl)) >= 0) {
	            DATE	*dop = &mep->edate ;
	            time_t	t = 0 ;
	            if ((rs = dater_getdate(dp,dop)) >= 0) {
	                mep->f.edate = TRUE ;
	                dater_gettime(dp,&t) ;
	                mep->etime = t ;
	                f = TRUE ;
#if	CF_DEBUGS
	                debugprintf("msgentry_procenvdate: t=%u\n",t) ;
#endif
	            }
	        } else if (isBadDate(rs)) {
#if	CF_DEBUGS
	            debugprintf("msgentry_procenvdate: bad-date rs=%d\n",rs) ;
#endif
	            rs = SR_OK ;
	        }
	    } /* end if (had ENV value) */
	} /* end if (processing) */

#if	CF_DEBUGS
	debugprintf("msgentry_procenvdate: ret rs=%d f=%u\n",rs,f) ;
#endif

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (msgentry_procenvdate) */


static int msgentry_prochdrdate(MSGENTRY *mep,MBCACHE *op)
{
	int		rs = SR_OK ;
	int		f = FALSE ;

	if (! mep->proc.hdate) {
	    const int	vi = mbcachemf_hdrdate ;
	    int		sl = mep->vl[vi] ;
	    const char	*sp = mep->vs[vi] ;
	    mep->proc.hdate = TRUE ;
	    if ((sp != NULL) && (sl > 0)) {
	        DATER		*dp = &op->dm ;
#if	CF_DEBUGS
	        debugprintf("msgentry_prochdrdate: hdr=>%t<\n",sp,sl) ;
#endif
	        if ((rs = dater_setmsg(dp,sp,sl)) >= 0) {
	            DATE	*dop = &mep->hdate ;
	            time_t	t = 0 ;
	            if ((rs = dater_getdate(dp,dop)) >= 0) {
	                mep->f.hdate = TRUE ;
	                dater_gettime(dp,&t) ;
	                mep->htime = t ;
	                f = TRUE ;
	            }
	        } else if (isBadDate(rs)) {
#if	CF_DEBUGS
	            debugprintf("msgentry_prochdrdate: dater_setmsg rs=%d\n",
			rs) ;
#endif
	            rs = SR_OK ;
	        }
	    } /* end if (had HDR value) */
	} /* end if (processing) */

#if	CF_DEBUGS
	debugprintf("msgentry_prochdrdate: ret rs=%d f=%u\n",rs,f) ;
#endif

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (msgentry_prochdrdate) */


static int msgentry_procscanfrom(MSGENTRY *mep,MBCACHE *op)
{
	int		rs = SR_OK ;
	int		len = 0 ;

	if (! mep->proc.scanfrom) {
	    int		sl = mep->vl[mbcachemf_hdrfrom] ;
	    const char	*sp = mep->vs[mbcachemf_hdrfrom] ;
	    mep->proc.scanfrom = TRUE ;
	    if ((sp == NULL) || (sl == 0)) {
	        sl = mep->vl[mbcachemf_envaddr] ;
	        sp = mep->vs[mbcachemf_envaddr] ;
	    }
	    if ((sp != NULL) && (sl > 0)) {
	        STRPACK		*psp = &op->strs ;
	        const int	hlen = HDRBUFLEN ;
	        char		hbuf[HDRBUFLEN+1] ;
	        if ((rs = mkaddrname(hbuf,hlen,sp,sl)) >= 0) {
	            const int	vi = mbcachemf_scanfrom ;
	            const char	**rpp ;
	            len = rs ;
	            rpp = (mep->vs + vi) ;
	            mep->vl[vi] = len ;
	            if ((rs = strpack_store(psp,hbuf,len,rpp)) >= 0) {
	                mep->f.scanfrom = TRUE ;
	            }
	        }
	    } /* end if (positive) */
	} else if (mep->vs[mbcachemf_scanfrom] != NULL) {
	    len = mep->vl[mbcachemf_scanfrom] ;
	}

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (msgentry_procscanfrom) */


static int msgentry_procscandate(MSGENTRY *mep,MBCACHE *op)
{
	int		rs = SR_OK ;
	int		cl = 0 ;

#if	CF_DEBUGS
	debugprintf("msgentry_procscandate: ent proc=%u\n",
	    mep->proc.scandate) ;
#endif

	if (! mep->proc.scandate) {
	    mep->proc.scandate = TRUE ;
	    if ((rs = msgentry_prochdrdate(mep,op)) >= 0) {
	        time_t	t = mep->htime ;
#if	CF_DEBUGS
	        debugprintf("msgentry_procscandate: hdrtime t=%u\n",t) ;
#endif
	        if (t == 0) {
	            if ((rs = msgentry_procenvdate(mep,op)) >= 0) {
	                t = mep->etime ;
#if	CF_DEBUGS
	                debugprintf("msgentry_procscandate: envtime t=%u\n",t) ;
#endif
	            }
	        }
	        if ((rs >= 0) && (t > 0)) {
	            const char	*ts ;
	            char	timebuf[TIMEBUFLEN + 1] ;
	            if ((ts = timestr_scandate(t,timebuf)) != NULL) {
	                STRPACK	*psp = &op->strs ;
	                const char	*cp ;
	                if ((rs = strpack_store(psp,ts,-1,&cp)) >= 0) {
	                    cl = rs ;
	                    mep->f.scandate = TRUE ;
	                    mep->vs[mbcachemf_scandate] = cp ;
	                    mep->vl[mbcachemf_scandate] = cl ;
	                }
	            }
	        }
	    } /* end if */
	} else if (mep->vs[mbcachemf_scandate] != NULL) {
	    cl = mep->vl[mbcachemf_scandate] ;
	}

#if	CF_DEBUGS
	debugprintf("msgentry_procscandate: ret rs=%d len=%u\n",rs,cl) ;
#endif

	return (rs >= 0) ? cl : rs ;
}
/* end subroutine (msgentry_procscandate) */


static int msgentry_msgtimes(MSGENTRY *mep,MBCACHE *op)
{
	int		rs ;
	int		c = 0 ;

	if ((rs = msgentry_procenvdate(mep,op)) >= 0) {
	    c += rs ;
	    if ((rs = msgentry_prochdrdate(mep,op)) >= 0) {
	        c += rs ;
	    }
	}

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (msgentry_msgtimes) */


static int isBadDate(int rs)
{
	return isOneOf(rsdatebad,rs) ;
}
/* end subroutine (isBadDate) */


static int isNoMsg(int rs)
{
	return isOneOf(rsnomsg,rs) ;
}
/* end subroutine (isNoMsg) */


static int vcmpmsgentry(const void *e1pp,const void *e2pp)
{
	MSGENTRY	**m1pp = (MSGENTRY **) e1pp ;
	MSGENTRY	**m2pp = (MSGENTRY **) e2pp ;
	MSGENTRY	*m1p, *m2p ;
	int		rc = 0 ;

	m1p = (*m1pp) ;
	m2p = (*m2pp) ;

#if	CF_DEBUGS
	debugprintf("mbcache/vcmpmsgentry: m1{%p} m2{%p}\n",
	    m1p, m2p) ;
#endif

	if ((m1p != NULL) || (m2p != NULL)) {
	    if ((rc == 0) && (m1p == NULL)) rc = 1 ;
	    if ((rc == 0) && (m2p == NULL)) rc = -1 ;
	    if (rc == 0) {
	        time_t	t1e = m1p->etime ;
	        time_t	t2e = m2p->etime ;
	        time_t	t1h = m1p->htime ;
	        time_t	t2h = m2p->htime ;
	        if (t1h == 0) t1h = t1e ;
	        if (t2h == 0) t2h = t2e ;
	        rc = (t1h - t2h) ;
	        if (rc == 0) rc = (t1e - t2e) ;
	    }
	}

	return rc ;
}
/* end subroutine (vcmpmsgentry) */


#ifdef	COMMENT
/* append some additional value to an existing header value */
static int headappend(cchar **pp,cchar *sp,int sl)
{
	int		rs ;
	int		cl, cl2 ;
	char		*cp, *cp2 ;

#if	CF_DEBUGS
	debugprintf("mbcache/headappend: ent sp=>%t<\n",
	    sp,MIN(sl,30)) ;
#endif

	if (*pp == NULL)
	    return 0 ;

	cl2 = strlen(*pp) ;

#if	CF_DEBUGS
	debugprintf("mbcache/headappend: cl2=%d\n",cl2) ;
#endif

	cl = sl + cl2 + 2 ;
	if ((rs = uc_malloc(cl,&cp)) >= 0) {

	    cp2 = strwcpy(cp,*pp,cl2) ;

	    *cp2++ = ',' ;
	    *cp2++ = ' ' ;
	    strwcpy(cp2,sp,sl) ;

	    uc_free(*pp) ;

	    *pp = cp ;

	} /* end if (memory-allocation) */

	return (rs >= 0) ? cl : rs ;
}
/* end subroutine (headappend) */
#endif /* COMMENT */


#ifdef	COMMENT
/* is the next non-white-space character a colon */
static int nextnonwhite(cchar *sp,int sl)
{
	int		i = 0 ;
	while (CHAR_ISWHITE(*sp) && (i++ < sl)) {
	    sp += 1 ;
	}
	return (i < sl) ? *sp : ' ' ;
}
/* end subroutine (nextnonwhite) */
#endif /* COMMENT */


#if	CF_DEBUGS
static int debugprintlines(cchar *sub,cchar *s,int f,int l)
{
	char		digbuf[DIGBUFLEN + 1] ;
	digbuf[0] = '\0' ;
	if (f) ctdeci(digbuf,DIGBUFLEN,l) ;
	debugprintf("%s: %s=%s\n",sub,s,((f) ? digbuf : "")) ;
	return f ;
}
/* end subroutine (debugprintlines) */
#endif /* CF_DEBUGS */


