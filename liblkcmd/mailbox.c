/* mailbox */

/* mailbox handling object */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */
#define	CF_SAFE		0		/* safe */
#define	CF_READTO	0		/* use a read timeout? */


/* revision history:

	= 2009-01-10, David A­D­ Morano
        This is being written from scratch to finally get an abstracted
        "mailbox" that is fast enough for interactive use.

*/

/* Copyright © 2009 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This is a *simple* mailbox object. It reads a mailbox-formatted file and
        allows some simple manipulations of it.

	Notes on message status:

	-	new
	O	old and unread
	R	read but not old
	RO	read previously (old and read)


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<sigblock.h>
#include	<filebuf.h>
#include	<char.h>
#include	<ascii.h>
#include	<mailmsgmatenv.h>
#include	<vecobj.h>
#include	<vecstr.h>
#include	<localmisc.h>

#include	"mailbox.h"
#include	"mailmsghdrval.h"
#include	"mailmsghdrfold.h"


/* local defines */

#ifndef	LINEBUFLEN
#ifdef	LINE_MAX
#define	LINEBUFLEN		MAX(LINE_MAX,2048)
#else
#define	LINEBUFLEN		2048
#endif
#endif

#ifndef	MSGCOLS
#define	MSGCOLS			76
#endif

#define	MAILBOX_DEFMSGS		100
#define	MAILBOX_READSIZE	(4*1024)
#define	MAILBOX_TMPVARDNAME	"/var/tmp"
#define	MAILBOX_SUBDNAME	"mailboxes"

#define	FMAT(cp)	((cp)[0] == 'F')

#define	TO_LOCK		120		/* mailbox lock-timeout */
#define	TO_READ		5

#define	MSGHEADCONT(ch)	(((ch) == CH_SP) || ((ch) == CH_TAB))

#define	TMPDIRMODE	(0777 | S_ISVTX)

#ifndef	SIGACTION
#define	SIGACTION	struct sigaction
#endif

#define	MSGCOPY		struct msgcopy

#define	SIGSTATE	struct sigstate

#define	LINER		struct liner

#define	MAILBOXPI	struct mailboxpi
#define	MAILBOXPI_FL	struct mailboxpi_flags


/* external subroutines */

extern int	sncpy1(char *,int,const char *) ;
extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	matstr(const char **,const char *,int) ;
extern int	matcasestr(const char **,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecui(const char *,uint,uint *) ;
extern int	mailmsgmathdr(const char *,int,int *) ;
extern int	perm(const char *,uid_t,gid_t,gid_t *,int) ;
extern int	lockfile(int,int,offset_t,offset_t,int) ;
extern int	tmpmailboxes(char *,int) ;
extern int	opentmpfile(const char *,int,mode_t,char *) ;
extern int	lockend(int,int,int,int) ;
extern int	vecstr_envadd(vecstr *,const char *,const char *,int) ;
extern int	filebuf_writefd(FILEBUF *,char *,int,int,int) ;
extern int	filebuf_writehdr(FILEBUF *,cchar *,int) ;
extern int	filebuf_writehdrval(FILEBUF *,cchar *,int) ;
extern int	hdrextnum(const char *,int) ;
extern int	iceil(int,int) ;
extern int	isprintlatin(int) ;
extern int	hasEOH(cchar *,int) ;

#if	CF_DEBUGS || CF_FDEBUG
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strncpyblanks(char *,int) ;	/* not NUL-terminaed */
extern char	*strnchr(const char *,int,int) ;


/* external variables */


/* local structures */

struct liner {
	FILEBUF		*fbp ;
	offset_t	poff ;			/* file-offset previous */
	offset_t	foff ;			/* file-offset current */
	int		to ;			/* read time-out */
	int		llen ;
	char		lbuf[LINEBUFLEN + 1] ;
} ;

struct sigstate {
	SIGACTION	*sao ;
	sigset_t	oldsigmask ;
} ;

struct msgcopy {
	FILEBUF		*fbp ;
	offset_t	moff ;
	char		*bp ;
	int		bl ;
} ;

struct mailboxpi_flags {
	uint		msg ;
	uint		env ;
	uint		hdr ;
	uint		mhv ;
	uint		eoh ;
	uint		bol ;
	uint		eol ;
} ;

struct mailboxpi {
	LINER		*lsp ;
	MAILBOXPI_FL	f ;
	int		mi ;
} ;

/* local (forward) subroutines */

static int mailbox_parse(MAILBOX *) ;
static int mailbox_parsemsg(MAILBOX *,LINER *,int) ;
static int mailbox_parsemsger(MAILBOX *,MAILMSGMATENV *,MAILBOXPI *) ;
static int mailbox_loadmsghead(MAILBOX *,MAILBOX_MSGINFO *,MAILMSGHDRVAL *) ;
static int mailbox_msgfins(MAILBOX *) ;
static int mailbox_rewrite(MAILBOX *) ;
static int mailbox_rewriter(MAILBOX *,int) ;
static int mailbox_msgcopy(MAILBOX *,MSGCOPY *,MAILBOX_MSGINFO *) ;
static int mailbox_msgcopyadd(MAILBOX *,MSGCOPY *,MAILBOX_MSGINFO *) ;

static int msginfo_start(MAILBOX_MSGINFO *,offset_t,int) ;
static int msginfo_finish(MAILBOX_MSGINFO *) ;
static int msginfo_setenv(MAILBOX_MSGINFO *,MAILMSGMATENV *) ;

static int liner_start(LINER *,FILEBUF *,offset_t,int) ;
static int liner_finish(LINER *) ;
static int liner_read(LINER *,cchar **) ;
static int liner_done(LINER *) ;
static int liner_seek(LINER *,int) ;

static int mailboxpi_start(MAILBOXPI *,LINER *,int) ;
static int mailboxpi_finish(MAILBOXPI *) ;
static int mailboxpi_havemsg(MAILBOXPI *) ;

static int writeblanklines(int,int) ;

#if	CF_DEBUGS
static int debuglinelen(const char *,int,int) ;
#endif


/* local variables */

static const char	*msghdrkeys[] = {
	"content-length",
	"content-lines",
	"lines",
	"x-lines",
	NULL
} ;

enum headkeys {
	headkey_clen,
	headkey_clines,
	headkey_lines,
	headkey_xlines,
	headkey_overlast
} ;

static const int	sigblocks[] = {
	SIGUSR1,
	SIGUSR2,
	SIGQUIT,
	SIGCHLD,
	SIGTERM,
	SIGINT,
	SIGPOLL,
	0
} ;


/* exported subroutines */


int mailbox_open(MAILBOX *mbp,cchar *mbfname,int mflags)
{
	struct ustat	sb ;
	int		rs ;
	int		oflags ;
	int		nmsgs = 0 ;
	const char	*cp ;

	if (mbp == NULL) return SR_FAULT ;
	if (mbfname == NULL) return SR_FAULT ;

	if (mbfname[0] == '\0') return SR_INVALID ;

	memset(mbp,0,sizeof(MAILBOX)) ;

	mbp->mfd = -1 ;
	mbp->msgs_total = 0 ;
	mbp->to_lock = TO_LOCK ;
	mbp->to_read = TO_READ ;
	mbp->mflags = mflags ;
	mbp->pagesize = getpagesize() ;

	mbp->f.readonly = (! (mflags & MAILBOX_ORDWR)) ;
	mbp->f.useclen = (! (mflags & MAILBOX_ONOCLEN)) ;
	mbp->f.useclines = MKBOOL(mflags & MAILBOX_OUSECLINES) ;

#if	CF_DEBUGS
	debugprintf("mailbox_open: ent mbfname=%s\n",mbfname) ;
	debugprintf("mailbox_open: f_useclen=%u\n",mbp->f.useclen) ;
	debugprintf("mailbox_open: f_useclines=%u\n",mbp->f.useclines) ;
	debugprintf("mailbox_open: f_readonly=%u\n",mbp->f.readonly) ;
#endif

	oflags = 0 ;
	oflags |= (mflags & MAILBOX_ORDWR) ? O_RDWR : 0 ;

	if ((rs = uc_open(mbfname,oflags,0666)) >= 0) {
	    mbp->mfd = rs ;
	    if ((rs = u_fstat(mbp->mfd,&sb)) >= 0) {
	        if (S_ISREG(sb.st_mode)) {
	            mbp->mblen = (uint) sb.st_size ;
	            mbp->ti_mod = sb.st_mtime ;
	            mbp->ti_check = time(NULL) ;
	            if ((rs = uc_mallocstrw(mbfname,-1,&cp)) >= 0) {
	                const int	es = sizeof(struct mailbox_msg) ;
	                const int	vo = VECOBJ_OCOMPACT ;
	                mbp->mailfname = cp ;
	                nmsgs = MAILBOX_DEFMSGS ;
	                if ((rs = vecobj_start(&mbp->msgs,es,10,vo)) >= 0) {
	                    const int	to = mbp->to_lock ;
	                    if ((rs = lockend(mbp->mfd,TRUE,1,to)) >= 0) {
	                        offset_t	loff ;
	                        if ((rs = mailbox_parse(mbp)) >= 0) {
	                            nmsgs = mbp->msgs_total ;
	                            mbp->magic = MAILBOX_MAGIC ;
	                        }
	                        if (rs < 0) {
	                            mailbox_msgfins(mbp) ;
	                        }
	                        loff = mbp->mblen ;
	                        lockfile(mbp->mfd,F_UNLOCK,loff,0L,0) ;
	                    } /* end if (lock) */
	                    if (rs < 0)
	                        vecobj_finish(&mbp->msgs) ;
	                } /* end if (vecstr-msgs) */
	                if (rs < 0) {
	                    uc_free(mbp->mailfname) ;
	                    mbp->mailfname = NULL ;
	                }
	            } /* end if (m-a) */
	        } else {
	            rs = SR_ISDIR ;
	        }
	    } /* end if (stat) */
	    if (rs < 0) {
	        u_close(mbp->mfd) ;
	        mbp->mfd = -1 ;
	    }
	} /* end if (file-open) */

	return (rs >= 0) ? nmsgs : rs ;
}
/* end subroutine (mailbox_open) */


int mailbox_close(MAILBOX *mbp)
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		mblen = 0 ;
	int		f ;

	if (mbp == NULL) return SR_FAULT ;

	if (mbp->magic != MAILBOX_MAGIC) return SR_NOTOPEN ;

#if	CF_DEBUGS
	debugprintf("mailbox_close: ent\n") ;
#endif

	f = (! mbp->f.readonly) ;
	f = f && ((mbp->msgs_del > 0) || mbp->f.rewrite) ;
	if (f) {
	    rs1 = mailbox_rewrite(mbp) ;
	    if (rs >= 0) rs = rs1 ;
	    mblen = rs ;
	}

#if	CF_DEBUGS
	debugprintf("mailbox_close: 7 rs=%d\n",rs) ;
#endif

/* free up everything */

	rs1 = mailbox_msgfins(mbp) ;
	if (rs >= 0) rs = rs1 ;

#if	CF_DEBUGS
	debugprintf("mailbox_close: 8 rs=%d\n",rs) ;
#endif

	rs1 = vecobj_finish(&mbp->msgs) ;
	if (rs >= 0) rs = rs1 ;

#if	CF_DEBUGS
	debugprintf("mailbox_close: 9 rs=%d\n",rs) ;
#endif

	if (mbp->mailfname != NULL) {
	    rs1 = uc_free(mbp->mailfname) ;
	    if (rs >= 0) rs = rs1 ;
	    mbp->mailfname = NULL ;
	}

#if	CF_DEBUGS
	debugprintf("mailbox_close: 10 rs=%d\n",rs) ;
#endif

	if (mbp->mfd >= 0) {
	    rs1 = u_close(mbp->mfd) ;
	    if (rs >= 0) rs = rs1 ;
	    mbp->mfd = -1 ;
	}

#if	CF_DEBUGS
	debugprintf("mailbox_close: ret rs=%d mblen=%u\n",rs,mblen) ;
#endif

	mbp->magic = 0 ;
	return (rs >= 0) ? mblen : rs ;
}
/* end subroutine (mailbox_close) */


/* get the mail filename */
int mailbox_mbfile(MAILBOX *mbp,char *mbuf,int mlen)
{
	int		rs ;

	if (mbp == NULL) return SR_FAULT ;
	if (mbuf == NULL) return SR_FAULT ;

	if (mbp->magic != MAILBOX_MAGIC) return SR_NOTOPEN ;

	rs = sncpy1(mbuf,mlen,mbp->mailfname) ;

	return rs ;
}
/* end subroutine (mailbox_mbfile) */


/* get information */
int mailbox_info(MAILBOX *mbp,MAILBOX_INFO *mip)
{

	if (mbp == NULL) return SR_FAULT ;
	if (mip == NULL) return SR_FAULT ;
	if (mbp->magic != MAILBOX_MAGIC) return SR_NOTOPEN ;

/* fill it in */

	memset(mip,0,sizeof(MAILBOX_INFO)) ;

	mip->nmsgs = mbp->msgs_total ;

	return mbp->msgs_total ;
}
/* end subroutine (mailbox_info) */


/* get count of messages */
int mailbox_count(MAILBOX *mbp)
{

	if (mbp == NULL) return SR_FAULT ;

	if (mbp->magic != MAILBOX_MAGIC) return SR_NOTOPEN ;

	return mbp->msgs_total ;
}
/* end subroutine (mailbox_count) */


/* check whatever */
int mailbox_check(MAILBOX *mbp,time_t daytime)
{
	int		rs = SR_OK ;
	int		f = FALSE ;

	if (mbp == NULL) return SR_FAULT ;

	if (mbp->magic != MAILBOX_MAGIC) return SR_NOTOPEN ;

	if (daytime == 0)
	    daytime = time(NULL) ;

	if (daytime == 0)
	    f = TRUE ;

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (mailbox_check) */


/* mark a message for deletion */
int mailbox_msgdel(MAILBOX *mbp,int mi,int f)
{
	MAILBOX_MSGINFO	*mp ;
	int		rs ;
	int		f_prev = FALSE ;

	if (mbp == NULL) return SR_FAULT ;

	if (mbp->magic != MAILBOX_MAGIC) return SR_NOTOPEN ;

	if (mi < 0) return SR_INVALID ;

	if (mi >= mbp->msgs_total)
	    return SR_NOTFOUND ;

	if (mbp->f.readonly)
	    return SR_ROFS ;

	if ((rs = vecobj_get(&mbp->msgs,mi,&mp)) >= 0) {
	    f_prev = mp->cmd.delete ;
	    if (! LEQUIV(f,f_prev)) {
	        if (mp->cmd.delete) {
	            mbp->msgs_del -= 1 ;
	            mp->cmd.delete = FALSE ;
	        } else {
	            mbp->msgs_del += 1 ;
	            mp->cmd.delete = TRUE ;
	        }
	    }
	} /* end if (vecobj_get) */

	return (rs >= 0) ? f_prev : rs ;
}
/* end subroutine (mailbox_msgdel) */


/* get count of deleted messages */
int mailbox_countdel(MAILBOX *mbp)
{
	int		count = 0 ;

	if (mbp == NULL) return SR_FAULT ;

	if (mbp->magic != MAILBOX_MAGIC) return SR_NOTOPEN ;

#ifdef	COMMENT
	{
	    MAILBOX_MSGINFO	*msgp ;
	    for (i = 0 ; vecobj_get(&mbp->msgs,i,&msgp) >= 0 ; i += 1) {
	        if (mp->f.delete) count += 1 ;
	    } /* end for */
	} /* end block */
#else
	count = mbp->msgs_del ;
#endif /* COMMENT */

	return count ;
}
/* end subroutine (mailbox_countdel) */


/* get the file offset to the start-envelope of a message */
int mailbox_msgoff(MAILBOX *mbp,int mi,offset_t *offp)
{
	MAILBOX_MSGINFO	*mp ;
	int		rs ;
	int		mlen = 0 ;

	if (mbp == NULL) return SR_FAULT ;
	if (offp == NULL) return SR_FAULT ;

	if (mbp->magic != MAILBOX_MAGIC) return SR_NOTOPEN ;

	if (mi < 0) return SR_INVALID ;

	if (mi >= mbp->msgs_total) return SR_NOTFOUND ;

	if ((rs = vecobj_get(&mbp->msgs,mi,&mp)) >= 0) {
	    *offp = mp->hoff ;
	    mlen = mp->mlen ;
	} else
	    *offp = 0 ;

	return (rs >= 0) ? mlen : rs ;
}
/* end subroutine (mailbox_msgoff) */


/* get the information for a message */
int mailbox_msginfo(MAILBOX *mbp,int mi,MAILBOX_MSGINFO *mip)
{
	MAILBOX_MSGINFO	*mp ;
	int		rs ;
	int		mlen = 0 ;

	if (mbp == NULL) return SR_FAULT ;
	if (mip == NULL) return SR_FAULT ;

	if (mbp->magic != MAILBOX_MAGIC) return SR_NOTOPEN ;

	if (mi < 0) return SR_INVALID ;

	if (mi >= mbp->msgs_total) return SR_NOTFOUND ;

#if	CF_DEBUGS
	debugprintf("mailbox_msginfo: mi=%d\n",mi) ;
#endif

	if ((rs = vecobj_get(&mbp->msgs,mi,&mp)) >= 0) {
	    *mip = *mp ;
	    mlen = mp->mlen ;
	}

/* we're out of here */

#if	CF_DEBUGS
	debugprintf("mailbox_msginfo: ret rs=%d mlen=%d\n",rs,mlen) ;
	if (rs >= 0)
	    debugprintf("mailbox_msginfo: ret blen=%d clen=%d\n",
	        mp->blen,mp->clen) ;
#endif

	return (rs >= 0) ? mlen : rs ;
}
/* end subroutine (mailbox_msginfo) */


int mailbox_msghdradd(MAILBOX *mbp,int mi,cchar *k,cchar *vp,int vl)
{
	MAILBOX_MSGINFO	*mp ;
	int		rs ;

	if (mbp == NULL) return SR_FAULT ;
	if (k == NULL) return SR_FAULT ;

	if (mbp->magic != MAILBOX_MAGIC) return SR_NOTOPEN ;

	if (k[0] == '\0') return SR_INVALID ;

#if	CF_DEBUGS
	debugprintf("mailbox_msghdradd: mi=%u k=%s\n",mi,k) ;
#endif

	if ((rs = vecobj_get(&mbp->msgs,mi,&mp)) >= 0) {
	    if (! mp->f.hdradds) {
	        mp->f.hdradds = TRUE ;
	        rs = vecstr_start(&mp->hdradds,1,0) ;
	    }
	    if (rs >= 0) {
	        mbp->f.rewrite = TRUE ;
	        mp->f.addany = TRUE ;
	        rs = vecstr_envadd(&mp->hdradds,k,vp,vl) ;
	    }
	}

	return rs ;
}
/* end subroutine (mailbox_msghdradd) */


int mailbox_readbegin(MAILBOX *mbp,MAILBOX_READ *curp,offset_t foff,int rsize)
{
	int		rs ;
	char		*p ;

	if (mbp == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;

	if (rsize <= 0) rsize = MAILBOX_READSIZE ;

	memset(curp,0,sizeof(MAILBOX_READ)) ;

	if ((rs = uc_malloc(rsize,&p)) >= 0) {
	    curp->rbuf = p ;
	    curp->rsize = rsize ;
	    curp->foff = foff ;
	    curp->roff = foff ;
	}

	return rs ;
}
/* end subroutine (mailbox_readbegin) */


int mailbox_readend(MAILBOX *mbp,MAILBOX_READ *curp)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (mbp == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;

	if (curp->rbuf == NULL) return SR_BADFD ;

	rs1 = uc_free(curp->rbuf) ;
	if (rs >= 0) rs = rs1 ;
	curp->rbuf = NULL ;
	curp->rbp = NULL ;
	curp->rsize = 0 ;
	curp->rlen = 0 ;
	curp->foff = 0 ;
	curp->roff = 0 ;

	return rs ;
}
/* end subroutine (mailbox_readend) */


int mailbox_readline(MAILBOX *mbp,MAILBOX_READ *curp,char *lbuf,int llen)
{
	int		rs = SR_OK ;
	int		mfd ;
	int		i, mlen ;
	int		tlen = 0 ;
	char		*lbp = lbuf ;
	char		*bp, *lastp ;

	if (mbp == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;
	if (lbuf == NULL) return SR_FAULT ;

#if	CF_DEBUGS
	debugprintf("mailbox_readline: ent llen=%d\n",llen) ;
#endif

	lbuf[0] = '\0' ;
	mfd = mbp->mfd ;
	while (tlen < llen) {

#if	CF_DEBUGS
	    debugprintf("mailbox_readline: while-top rlen=%d\n",curp->rlen) ;
#endif

	    if (curp->rlen == 0) {
	        offset_t	po = (curp->roff+tlen) ;
	        curp->rbp = curp->rbuf ;
	        rs = u_pread(mfd,curp->rbuf,curp->rsize,po) ;
	        if (rs >= 0) {
	            curp->rlen = rs ;
	            curp->foff += rs ;
	        }
	    } /* end if (refilling up buffer) */

	    if (rs < 0) break ;
	    if (curp->rlen == 0) break ;

	    mlen = MIN(curp->rlen,(llen - tlen)) ;

	    {
	        bp = curp->rbp ;
	        lastp = (curp->rbp + mlen) ;
	        while (bp < lastp) {
	            if ((*lbp++ = *bp++) == '\n') break ;
	        } /* end while */
	        i = (bp - curp->rbp) ;
	    }

#if	CF_DEBUGS
	    debugprintf("mailbox_readline: i=%d\n",i) ;
#endif

	    curp->rbp += i ;
	    tlen += i ;
	    curp->rlen -= i ;
	    if ((i > 0) && (lbp[-1] == '\n'))
	        break ;

#if	CF_DEBUGS
	    debugprintf("mailbox_readline: bottom while\n") ;
#endif

	} /* end while (trying to satisfy request) */

	if (rs >= 0) {
#if	CF_NULTERM
	    *lbp = '\0' ;
#endif
	    curp->roff += tlen ;
	}

#if	CF_DEBUGS
	debugprintf("mailbox_readline: l=>%t<\n",
	    lbuf,strlinelen(lbuf,tlen,40)) ;
	debugprintf("mailbox_readline: ret rs=%d tlen=%u\n",rs,tlen) ;
	debugprintf("mailbox_readline: roff=%lld\n",curp->roff) ;
#endif

	return (rs >= 0) ? tlen : rs ;
}
/* end subroutine (mailbox_readline) */


/* private subroutines */


/* parse a mailbox file */
static int mailbox_parse(MAILBOX *mbp)
{
	LINER		ls, *lsp = &ls ;
	FILEBUF		fb ;
	const offset_t	soff = 0L ;
	const int	bsize = (32 * mbp->pagesize) ;
	int		rs ;
	int		rs1 ;
	int		to = -1 ;

#if	CF_DEBUGS
	debugprintf("mailbox_parse: ent\n") ;
#endif

#if	CF_READTO
	to = mbp->to ;
#endif

/* parse out this mailbox file the hard way */

	if ((rs = filebuf_start(&fb,mbp->mfd,soff,bsize,0)) >= 0) {

	    if ((rs = liner_start(lsp,&fb,soff,to)) >= 0) {
	        int	mi = 0 ;

	        while ((rs = mailbox_parsemsg(mbp,lsp,mi)) > 0) {
	            mi += 1 ;
	        } /* end while */

	        mbp->mblen = (lsp->foff - soff) ;
	        mbp->msgs_total = mi ;

	        rs1 = liner_finish(lsp) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (liner) */

	    rs1 = filebuf_finish(&fb) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (filebuf) */

#if	CF_DEBUGS
	debugprintf("mailbox_parse: ret rs=%d nmsgs=%u\n",rs,mbp->msgs_total) ;
#endif

	return (rs >= 0) ? mbp->msgs_total : rs ;
}
/* end subroutine (mailbox_parse) */


/* parse out the headers of this message */
static int mailbox_parsemsg(MAILBOX *mbp,LINER *lsp,int mi)
{
	MAILBOXPI	pi ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		ll = 0 ;

#if	CF_DEBUGS
	debugprintf("mailbox_parsemsg: ent mi=%u\n",mi) ;
#endif

	if ((rs = mailboxpi_start(&pi,lsp,mi)) >= 0) {
	    MAILMSGMATENV	me ;
	    int			vi = 0 ;
	    const char		*lp ;

/* find message start */

	    while ((rs = liner_read(lsp,&lp)) >= 0) {
	        ll = rs ;
	        if (ll == 0) break ;

#if	CF_DEBUGS
	        debugprintf("mailbox_parsemsg: ll=%u line=>%t<\n",
	            ll,lp,debuglinelen(lp,ll,40)) ;
#endif

	        if ((rs >= 0) && (ll > 5) && FMAT(lp) &&
	            ((rs = mailmsgmatenv(&me,lp,ll)) > 0)) {
#if	CF_DEBUGS
	            debugprintf("mailbox_parsemsg: MAT-ENV\n") ;
#endif
	            pi.f.env = TRUE ;
	        } else if ((rs >= 0) && (ll > 2) &&
	            ((rs = mailmsgmathdr(lp,ll,&vi)) > 0)) {
#if	CF_DEBUGS
	            debugprintf("mailbox_parsemsg: MAT-HDR\n") ;
#endif
	            pi.f.hdr = TRUE ;
	        } else if ((rs >= 0) && (ll <= 2) && (mi == 0)) {
	            if ((lp[0] == '\n') || hasEOH(lp,ll)) {
#if	CF_DEBUGS
	                debugprintf("mailbox_parsemsg: MAT-EOH\n") ;
#endif
	                pi.f.eoh = TRUE ;
	            }
	        } /* end if */

	        if (rs < 0) break ;
	        if (pi.f.env || pi.f.hdr) break ;

	        ll = 0 ;
	        liner_done(lsp) ;
	        if (pi.f.eoh) break ;
	    } /* end while */

	    if ((rs >= 0) && mailboxpi_havemsg(&pi)) {
	        rs = mailbox_parsemsger(mbp,&me,&pi) ;
	        ll = rs ;
	    }

	    rs1 = mailboxpi_finish(&pi) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (mailboxpi) */

#if	CF_DEBUGS
	debugprintf("mailbox_parsemsg: ret rs=%d ll=%u\n",rs,ll) ;
#endif

	return (rs >= 0) ? ll : rs ;
}
/* end subroutine (mailbox_parsemsg) */


static int mailbox_parsemsger(MAILBOX *mbp,MAILMSGMATENV *mep,MAILBOXPI *pip)
{
	LINER		*lsp = pip->lsp ;
	MAILBOX_MSGINFO	msg, *msgp = &msg ;
	MAILMSGHDRVAL	mhv ;
	const int	mi = pip->mi ;
	int		rs ;
	int		rs1 ;
	int		ll = 0 ;
	cchar		*lp ;
	if ((rs = msginfo_start(msgp,lsp->poff,mi)) >= 0) {
	    int		hi = 0 ;
	    int		kl = 0 ;
	    int		vl = 0 ;
	    int		clen = -1 ;
	    int		clines = -1 ;
	    int		vi ;
	    cchar	*vp ;

	    pip->f.msg = TRUE ;
	    if (pip->f.env) {
	        if ((rs = msginfo_setenv(msgp,mep)) >= 0) {
	            liner_done(lsp) ;
	        }
	    }

/* read headers (ignoring envelope) */

	    while ((rs >= 0) && ((rs = liner_read(lsp,&lp)) >= 0)) {
	        ll = rs ;
	        if (ll == 0) break ;

#if	CF_DEBUGS
	        debugprintf("mailbox_parsemsg: h_env=%u f_hdr=%u f_eoh=%u\n",
	            pip->f.env,pip->f.hdr,pip->f.eoh) ;
#endif

	        if ((ll > 2) && (! pip->f.env) && 
	            (! pip->f.hdr) && (! pip->f.eoh)) {
	            kl = mailmsgmathdr(lp,ll,&vi) ;
#if	CF_DEBUGS
	            debugprintf("mailbox_parsemsg: mailmsgmathdr kl=%d\n",kl) ;
#endif
	            pip->f.hdr = (kl > 0) ;
	        }

	        if ((rs >= 0) && pip->f.hdr) {

#if	CF_DEBUGS
	            debugprintf("mailbox_parsemsg: HDR kl=%d vi=%d\n",
	                kl,vi) ;
#endif

	            pip->f.hdr = FALSE ;
	            if (pip->f.mhv) { /* previous value outstanding */

#if	CF_DEBUGS
	                debugprintf("mailbox_parsemsg: previous value\n") ;
#endif

	                rs = mailbox_loadmsghead(mbp,msgp,&mhv) ;

	                pip->f.mhv = FALSE ;
	                mailmsghdrval_finish(&mhv) ;

	            } /* end if (had a previous value outstanding) */

	            if (msgp->hoff < 0)
	                msgp->hoff = lsp->poff ;

	            if (rs >= 0) {
	                hi = matcasestr(msghdrkeys,lp,kl) ;

#if	CF_DEBUGS
	                debugprintf("mailbox_parsemsg: matcasestr() hi=%d\n",
				hi) ;
	                if (hi >= 0)
	                    debugprintf("mailbox_parsemsg: matcasestr() h=%s\n",
	                        msghdrkeys[hi]) ;
#endif

	            }

	            if ((rs >= 0) && (hi >= 0)) {

	                vp = (lp + vi) ;
	                vl = (ll - vi) ;

	                pip->f.mhv = TRUE ;
	                rs = mailmsghdrval_start(&mhv,hi,vp,vl) ;

	            } /* end if (have one we want) */

	        } else if ((rs >= 0) && (ll > 1) && MSGHEADCONT(lp[0])) {

#if	CF_DEBUGS
	            debugprintf("mailbox_parsemsg: matmsgcont poff=%lld\n",
	                lsp->poff) ;
#endif

	            if (pip->f.mhv) {
	                rs = mailmsghdrval_add(&mhv,lp,ll) ;
	            }

	        } else if ((rs >= 0) && ((lp[0] == '\n') || hasEOH(lp,ll))) {

#if	CF_DEBUGS
	            debugprintf("mailbox_parsemsg: EOH poff=%lld\n",
	                lsp->poff) ;
#endif

	            if (pip->f.mhv) { /* previous value outstanding */

#if	CF_DEBUGS
	                debugprintf("mailbox_parsemsg: previous value\n") ;
#endif

	                rs = mailbox_loadmsghead(mbp,msgp,&mhv) ;

	                pip->f.mhv = FALSE ;
	                mailmsghdrval_finish(&mhv) ;

	            } /* end if (had a previous value outstanding) */

	            pip->f.eoh = TRUE ;
	            if (msgp->hoff < 0)
	                msgp->hoff = lsp->poff ;

	            msgp->hlen = (lsp->poff - msgp->hoff) ;

	        } /* end if */

	        ll = 0 ;
	        pip->f.env = FALSE ;
	        pip->f.hdr = FALSE ;
	        liner_done(lsp) ;

	        if (pip->f.eoh) break ;
	        if (rs < 0) break ;
	    } /* end while (reading lines) */

#if	CF_DEBUGS
	    debugprintf("mailbox_parsemsg: while-end foff=%lld f_eoh=%u\n",
	        lsp->foff,pip->f.eoh) ;
#endif

/* load last header */

	    if (rs >= 0) {

#if	CF_DEBUGS
	        debugprintf("mailbox_parsemsg: load last header\n") ;
#endif

	        if (pip->f.mhv) {
	            if (rs >= 0) {
	                rs = mailbox_loadmsghead(mbp,msgp,&mhv) ;
	            }
	            pip->f.mhv = FALSE ;
	            mailmsghdrval_finish(&mhv) ;
	        } /* end if */

	        if (msgp->hoff < 0)
	            msgp->hoff = lsp->poff ;

	        if (msgp->hlen < 0)
	            msgp->hlen = (lsp->poff - msgp->hoff) ;

	        if (pip->f.eoh) {
	            msgp->boff = lsp->foff ;

#if	CF_DEBUGS
	            debugprintf("mailbox_parsemsg: EOF boff=%lld\n",
			msgp->boff) ;
#endif

	        }

	    } /* end if (loading last header) */

/* handle reading the body */

#if	CF_DEBUGS
	    debugprintf("mailbox_parsemsg: reading message body\n") ;
	    debugprintf("mailbox_parsemsg: CLEN hdrval=%u v=%d\n",
	        msgp->hdrval.clen,msgp->clen) ;
	    debugprintf("mailbox_parsemsg: CLINES hdrval=%u v=%d\n",
	        msgp->hdrval.clines,msgp->clines) ;
#endif

	    if (rs >= 0) {

	        if (mbp->f.useclen && msgp->hdrval.clen) {

#if	CF_DEBUGS
	            debugprintf("mailbox_parsemsg: have clen=%u\n",
	                msgp->clen) ;
	            debugprintf("mailbox_parsemsg: "
			"boff=%lld poff=%lld foff=%lld\n",
	                msgp->boff,lsp->poff,lsp->foff) ;
#endif

	            clen = msgp->clen ;
	            rs = liner_seek(lsp,msgp->clen) ;

	        } else {
	            int	max = INT_MAX ;

#if	CF_DEBUGS
	            debugprintf("mailbox_parsemsg: search for EOM\n") ;
#endif

	            if (mbp->f.useclines && 
	                msgp->hdrval.clines && (msgp->clines >= 0))
	                max = msgp->clines ;

#if	CF_DEBUGS
	            debugprintf("mailbox_parsemsg: max=%u\n",max) ;
#endif

	            pip->f.bol = TRUE ;
	            clines = 0 ;
	            while ((rs >= 0) && (clines < max) && 
	                ((rs = liner_read(lsp,&lp)) >= 0)) {

	                ll = rs ;
	                if (ll == 0) break ;

#if	CF_DEBUGS
	                debugprintf("mailbox_parsemsg: line=>%t<\n",
	                    lsp->lbuf,
	                    debuglinelen(lp,ll,40)) ;
#endif

	                pip->f.eol = (lp[ll-1] == '\n') ;
	                if (pip->f.bol && FMAT(lp) && (ll > 5)) {
	                    if ((rs = mailmsgmatenv(mep,lp,ll)) > 0) {
	                        pip->f.env = TRUE ;
	                    }
	                    if (pip->f.env) break ;
	                }

	                ll = 0 ;
	                if (pip->f.eol)
	                    clines += 1 ;

	                pip->f.bol = pip->f.eol ;
	                liner_done(lsp) ;

	            } /* end while (searching for new start-msg) */

	            if ((rs >= 0) && msgp->hdrval.clines) {
	                if (clines < msgp->clines) {
	                    msgp->clines = clines ;
	                }
	            }

	            clen = (lsp->poff - msgp->boff) ;

	        } /* end if (advancing to end-of-message) */

	        if (rs >= 0) {

#if	CF_DEBUGS
	            debugprintf("mailbox_parsemsg: EOM store-stuff\n") ;
	            debugprintf("mailbox_parsemsg: clen=%d\n",clen) ;
#endif

		    if (msgp->clen < 0) msgp->clen = clen ; /* ???? */

	            if (! msgp->f.blen) {
	                msgp->f.blen = TRUE ;
	                msgp->blen = (lsp->poff - msgp->boff) ;
	            } /* end if (blen) */

	            if (! msgp->f.mlen) {
	                msgp->f.mlen = TRUE ;
	                msgp->mlen = (lsp->poff - msgp->moff) ;
	            } /* end if (mlen) */

	        } /* end if (ok) */

	    } /* end if (ok) */

/* finish off the last message */

	    if (rs >= 0) {

#if	CF_DEBUGS
	        debugprintf("mailbox_parsemsg: after EOM rs=%d\n",rs) ;
	        debugprintf("mailbox_parsemsg: poff=%lld\n",lsp->poff) ;
	        debugprintf("mailbox_parsemsg: boff=%lld blen=%u\n",
	            msgp->boff,msgp->blen) ;
	        debugprintf("mailbox_parsemsg: f_mlen=%u\n", msgp->f.mlen) ;
#endif

	        if ((! msgp->hdrval.clines) && (clines >= 0)) {

#if	CF_DEBUGS
	            debugprintf("mailbox_parsemsg: need clines=%d\n",clines) ;
#endif

	            msgp->hdrval.clines = TRUE ;
	            msgp->clines = clines ;

	        } /* end if (clines) */

	    } /* end if */

/* finish up */

	    if ((rs >= 0) && pip->f.msg) {
	        ll = msgp->mlen ;
	        msgp->msgi = mi ;
	        rs = vecobj_add(&mbp->msgs,msgp) ;
	        if (rs >= 0) pip->f.msg = FALSE ;
	    } /* end if (insertion) */

	    rs1 = msginfo_finish(msgp) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (msginfo) */

	return (rs >= 0) ? ll : rs ;
}
/* end subroutine (mailbox_parsemsger) */


static int mailbox_loadmsghead(MAILBOX *mbp,MAILBOX_MSGINFO *msgp,
		MAILMSGHDRVAL *mhvp)
{
	int		rs ;
	int		vl ;
	int		hi ;
	int		v = -1 ;
	const char	*vp ;

	if (mbp == NULL) return SR_FAULT ;

#if	CF_DEBUGS
	debugprintf("mailbox_loadmsghead: ent\n") ;
#endif

	rs = mailmsghdrval_get(mhvp,&vp,&vl) ;
	hi = rs ;

#if	CF_DEBUGS
	debugprintf("mailbox_loadmsghead: mailmsghdrval_get() rs=%d\n",rs) ;
	if (hi >= 0)
	    debugprintf("mailbox_loadmsghead: h=%s\n",
	        msghdrkeys[hi]) ;
#endif

	if (rs >= 0) {
	    switch (hi) {
	    case headkey_clen:
	        msgp->hdr.clen = TRUE ;
	        if ((vl > 0) && ((v = hdrextnum(vp,vl)) >= 0)) {
	            msgp->hdrval.clen = TRUE ;
	            msgp->clen = v ;
	        }
	        break ;
	    case headkey_clines:
	        msgp->hdr.clines = TRUE ;
	        if ((vl > 0) && ((v = hdrextnum(vp,vl)) >= 0)) {
	            msgp->hdrval.clines = TRUE ;
	            msgp->clines = v ;
	        }
	        break ;
	    case headkey_lines:
	        msgp->hdr.lines = TRUE ;
	        if ((vl > 0) && ((v = hdrextnum(vp,vl)) >= 0)) {
	            msgp->hdrval.lines = TRUE ;
	            msgp->lines = v ;
	        }
	        break ;
	    case headkey_xlines:
	        msgp->hdr.xlines = TRUE ;
	        if ((vl > 0) && ((v = hdrextnum(vp,vl)) >= 0)) {
	            msgp->hdrval.xlines = TRUE ;
	            msgp->xlines = v ;
	        }
	        break ;
	    } /* end switch */
	} /* end if (ok) */

#if	CF_DEBUGS
	debugprintf("mailbox_loadmsghead: ret rs=%d hi=%d\n",rs,hi) ;
#endif

	return (rs >= 0) ? hi : rs ;
}
/* end subroutine (mailbox_loadmsghead) */


static int mailbox_msgfins(MAILBOX *mbp)
{
	MAILBOX_MSGINFO	*mp ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		i ;

	for (i = 0 ; vecobj_get(&mbp->msgs,i,&mp) >= 0 ; i += 1) {
	    if (mp == NULL) continue ;
	    rs1 = msginfo_finish(mp) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end for */

	return (rs >= 0) ? i : rs ;
}
/* end subroutine (mailbox_msgfins) */


static int mailbox_rewrite(MAILBOX *mbp)
{
	const int	dlen = MAXPATHLEN ;
	int		rs ;
	char		dbuf[MAXPATHLEN + 1] ;

#if	CF_DEBUGS
	debugprintf("mailbox_rewrite: ent\n") ;
#endif

	if ((rs = tmpmailboxes(dbuf,dlen)) >= 0) {
	    cchar	*mb = "mbXXXXXXXXXXXX" ;
	    char	template[MAXPATHLEN + 1] ;
	    if ((rs = mkpath2(template,dbuf,mb)) >= 0) {
	        SIGBLOCK	ss ;
	        if ((rs = sigblock_start(&ss,sigblocks)) >= 0) {
	            const mode_t	om = 0600 ;
	            const int	of = (O_RDWR | O_CREAT) ;
	            char		tbuf[MAXPATHLEN + 1] ;

	            if ((rs = opentmpfile(template,of,om,tbuf)) >= 0) {
	                const int	tfd = rs ;

	                rs = mailbox_rewriter(mbp,tfd) ;

#if	CF_DEBUGS
	                debugprintf("mailbox_rewrite: "
	                    "_rewriter() rs=%d\n",rs) ;
#endif

	                u_close(tfd) ;
	                u_unlink(tbuf) ;
	            } /* end if (open tmp-file) */

	            sigblock_finish(&ss) ;
	        } /* end if (sigblock) */
	    } /* end if (mkpath) */
	} /* end if (tmpmailboxes) */

#if	CF_DEBUGS
	debugprintf("mailbox_rewrite: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (mailbox_rewrite) */


static int mailbox_rewriter(MAILBOX *mbp,int tfd)
{
	MSGCOPY		mc ;
	MAILBOX_MSGINFO	*mip ;
	FILEBUF		fb, *fbp = &fb ;
	offset_t	mbchange = -1 ;
	const int	pagesize = mbp->pagesize ;
	int		rs ;
	int		bsize ;
	int		rs1 ;
	int		mfd = mbp->mfd ;
	int		mi ;
	int		bl ;
	int		elen = 0 ; /* GCC false complaint */
	int		wlen = 0 ;
	int		f_del ;
	int		f_copy = FALSE ;
	int		f ;
	char		*bp ;

#if	CF_DEBUGS
	debugprintf("mailbox_rewriter: ent\n") ;
#endif

	bl = pagesize ;
	if ((rs = uc_valloc(bl,&bp)) >= 0) {

	    memset(&mc,0,sizeof(MSGCOPY)) ;
	    mc.fbp = fbp ;
	    mc.bp = bp ;
	    mc.bl = bl ;
	    mc.moff = 0 ;

	    bsize = MIN(iceil(mbp->mblen,pagesize),(16*pagesize)) ;

	    if ((rs = filebuf_start(fbp,tfd,0L,bsize,0)) >= 0) {

	        for (mi = 0 ; mi < mbp->msgs_total ; mi += 1) {

	            rs = vecobj_get(&mbp->msgs,mi,&mip) ;
	            if (rs < 0) break ;

	            f_del  = mip->cmd.delete ;
	            f = f_del || mip->f.addany ;
	            if (f) {
	                if (mbchange < 0) {
	                    mbchange = mip->moff ;
	                    f_copy = TRUE ;
	                }
	            }

	            if (f_copy) {
	                if (! f_del) {
	                    rs = mailbox_msgcopy(mbp,&mc,mip) ;
	                    wlen += rs ;
	                }
	            } /* end if */

	            if (rs < 0) break ;
	        } /* end for */

	        if (rs >= 0) {

	            if (mc.moff != mbp->mblen) {
	                u_seek(mfd,mbp->mblen,SEEK_SET) ;
	                mc.moff = mbp->mblen ;
	            }
	            rs = filebuf_writefd(fbp,bp,bl,mfd,-1) ;
	            elen = rs ;
	            wlen += rs ;

	        } /* end if (finishing off) */

	        filebuf_finish(fbp) ;
	    } /* end if (filebuf) */

	    if (rs >= 0) {

/* extend the mailbox file if necessary (rarely happens?) */

	        if ((mbp->mblen + elen) < (mbchange + wlen)) {
	            rs1 = (mbchange + wlen) - (mbp->mblen + elen) ;
	            rs = writeblanklines(mfd,rs1) ;
	        } /* end if */

/* copy adjusted data back to the mailbox file */

	        if ((rs >= 0) && (wlen > 0)) {
	            u_seek(mfd,mbchange,SEEK_SET) ;
	            u_seek(tfd,0L,SEEK_SET) ;
	            rs = uc_writedesc(mfd,tfd,-1) ;
	        }

/* truncate mailbox file if necessary */

	        if ((rs >= 0) && (mbp->mblen > (mbchange + wlen))) {
	            rs = uc_ftruncate(mfd,(mbchange+wlen)) ;
#if	CF_DEBUGS
	            debugprintf("mailbox_rewriter: uc_ftruncate() rs=%d\n",rs) ;
#endif
	        }

	    } /* end if */

	    uc_free(bp) ;
	} /* end if (memory-allocation) */

#if	CF_DEBUGS
	debugprintf("mailbox_rewriter: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (mailbox_rewriter) */


static int mailbox_msgcopy(MAILBOX *mbp,MSGCOPY *mcp,MAILBOX_MSGINFO *mip)
{
	int		rs = SR_OK ;
	int		mfd = mbp->mfd ;
	int		wlen = 0 ;

	if (mcp->moff != mip->moff) {
	    rs = u_seek(mfd,mip->moff,SEEK_SET) ;
	    mcp->moff = mip->moff ;
	}

	if (rs >= 0) {
	    if (mip->f.addany) {
	        rs = mailbox_msgcopyadd(mbp,mcp,mip) ;
	        wlen += rs ;
	    } else {
	        rs = filebuf_writefd(mcp->fbp,mcp->bp,mcp->bl,mfd,mip->mlen) ;
	        wlen += rs ;
	    }
	    mcp->moff += wlen ;
	} /* end if (ok) */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (mailbox_msgcopy) */


static int mailbox_msgcopyadd(MAILBOX *mbp,MSGCOPY *mcp,MAILBOX_MSGINFO *mip)
{
	const int	mfd = mbp->mfd ;
	int		rs = SR_OK ;
	int		ehlen ;
	int		i ;
	int		wlen = 0 ;
	const char	*sp ;

#if	CF_DEBUGS
	debugprintf("mailbox_msgcopyadd: mi=%u\n",mip->msgi) ;
#endif

	ehlen = ((mip->hoff + mip->hlen) - mip->moff) ;
	if (rs >= 0) {
	    rs = filebuf_writefd(mcp->fbp,mcp->bp,mcp->bl,mfd,ehlen) ;
	    wlen += rs ;
	}

#if	CF_DEBUGS
	debugprintf("mailbox_msgcopyadd: hdr-add\n") ;
#endif

	if ((rs >= 0) && mip->f.addany && mip->f.hdradds) {
	    for (i = 0 ; vecstr_get(&mip->hdradds,i,&sp) >= 0 ; i += 1) {
	        if (sp != NULL) {
	            rs = filebuf_writehdr(mcp->fbp,sp,-1) ;
	            wlen += rs ;
		}
	        if (rs < 0) break ;
	    } /* end for */
	} /* end if */

#if	CF_DEBUGS
	debugprintf("mailbox_msgcopyadd: end-of-header and body\n") ;
#endif

	if (rs >= 0) {
	    rs = filebuf_writefd(mcp->fbp,mcp->bp,mcp->bl,mfd,(mip->blen+1)) ;
	    wlen += rs ;
	}

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (mailbox_msgcopyadd) */


static int msginfo_start(MAILBOX_MSGINFO *mp,offset_t off,int mi)
{

	memset(mp,0,sizeof(MAILBOX_MSGINFO)) ;
	mp->msgi = mi ;
	mp->moff = off ;
	mp->hoff = -1 ;
	mp->soff = -1 ;
	mp->boff = -1 ;
	mp->hlen = -1 ;
	mp->blen = -1 ;
	mp->blen = -1 ;
	mp->clen = -1 ;
	mp->clines = -1 ;
	mp->lines = -1 ;

#ifdef	COMMENT
	mp->add.sem = TRUE ;
	mp->add.clen = TRUE ;
	mp->add.clines = TRUE ;
	mp->add.status = TRUE ;
#endif

	return SR_OK ;
}
/* end subroutine (msginfo_start) */


static int msginfo_finish(MAILBOX_MSGINFO *mp)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (mp->f.hdradds) {
	    mp->f.hdradds = FALSE ;
	    rs1 = vecstr_finish(&mp->hdradds) ;
	    if (rs >= 0) rs = rs1 ;
	}

	mp->mlen = 0 ;
	return rs ;
}
/* end subroutine (msginfo_finish) */


static int msginfo_setenv(MAILBOX_MSGINFO *msgp,MAILMSGMATENV *mep)
{
	int		rs = SR_OK ;
	msgp->f.env = TRUE ;
	msgp->f.senv = mep->f.start ;
	return rs ;
}
/* end subroutine (msginfo_setenv) */


static int liner_start(LINER *lsp,FILEBUF *fbp,offset_t foff,int to)
{

	lsp->poff = 0 ;
	lsp->foff = foff ;
	lsp->fbp = fbp ;
	lsp->to = to ;
	lsp->llen = -1 ;
	lsp->lbuf[0] = '\0' ;
	return SR_OK ;
}
/* end subroutine (liner_start) */


static int liner_finish(LINER *lsp)
{

	lsp->llen = 0 ;
	lsp->lbuf[0] = '\0' ;
	return SR_OK ;
}
/* end subroutine (liner_finish) */


static int liner_read(LINER *lsp,cchar **lpp)
{
	FILEBUF		*fbp = lsp->fbp ;
	int		rs = SR_OK ;

	if (lsp->llen < 0) {
	    const int	llen = LINEBUFLEN ;
	    lsp->poff = lsp->foff ;
	    if ((rs = filebuf_readline(fbp,lsp->lbuf,llen,lsp->to)) >= 0) {
	        lsp->llen = rs ;
	        lsp->foff += lsp->llen ;
	    }
	} /* end if (needed a new line) */

	if (lpp != NULL) {
	    *lpp = (rs >= 0) ? lsp->lbuf : NULL ;
	}

	return (rs >= 0) ? lsp->llen : rs ;
}
/* end subroutine (liner_read) */


static int liner_done(LINER *lsp)
{

	lsp->poff = lsp->foff ;
	lsp->llen = -1 ;
	lsp->lbuf[0] = '\0' ;
	return SR_OK ;
}
/* end subroutine (liner_done) */


static int liner_seek(LINER *lsp,int inc)
{
	int		rs = SR_OK ;

	lsp->poff = lsp->foff ;
	if (inc > 0) {

	    lsp->llen = -1 ;
	    lsp->lbuf[0] = '\0' ;

	    lsp->poff += inc ;
	    lsp->foff += inc ;
	    rs = filebuf_adv(lsp->fbp,inc) ;

	} /* end if */

	return rs ;
}
/* end subroutine (liner_seek) */


static int mailboxpi_finish(MAILBOXPI *pip)
{
	if (pip == NULL) return SR_FAULT ;
	return SR_OK ;
}
/* end subroutine (mailboxpi_finish) */


static int mailboxpi_havemsg(MAILBOXPI *pip)
{
	return (pip->f.env || pip->f.hdr || pip->f.eoh) ;
}
/* end subroutine (mailboxpi_havemsg) */


static int mailboxpi_start(MAILBOXPI *pip,LINER *lsp,int mi)
{
	memset(pip,0,sizeof(MAILBOXPI)) ;
	pip->lsp = lsp ;
	pip->mi = mi ;
	return SR_OK ;
}
/* end subroutine (mailboxpi_start) */


static int writeblanklines(int mfd,int len)
{
	const int	llen = LINEBUFLEN ;
	int		rs = SR_OK ;
	int		rlen = len ;
	int		maxlen ;
	int		ll ;
	char		lbuf[LINEBUFLEN + 1] ;

	while ((rs >= 0) && (rlen > 0)) {

	    maxlen = (rlen > 1) ? (rlen-1) : 0 ;
	    ll = MIN(llen,maxlen) ;
	    strncpyblanks(lbuf,ll) ;
	    lbuf[ll++] = '\n' ;
	    rs = u_write(mfd,lbuf,ll) ;
	    rlen -= rs ;

	} /* end while */

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (writeblanklines) */


#if	CF_DEBUGS
static int debuglinelen(lp,ll,ml)
const char	*lp ;
int		ll ;
int		ml ;
{
	int		rl ;
	rl = strnlen(lp,ll) ;
	if (rl > ml)
	    rl = ml ;
	if ((rl > 0) && (lp[rl-1] == '\n'))
	    rl -= 1 ;
	return rl ;
}
/* end subroutine (debuglinelen) */
#endif /* CF_DEBUGS */


