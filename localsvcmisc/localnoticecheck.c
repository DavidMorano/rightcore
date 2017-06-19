/* localnoticecheck */

/* determine if the given user has mail (as PCS determines it) */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */
#define	CF_UGETPW	1		/* use |ugetpw(3uc)| */


/* revision history:

	= 1998-06-01, David A­D­ Morano

	This subroutine was originally written.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine determines how many (if any) notices a given user
	(supplied) has received within the last (given) time period interncal.

	Synopsis:

	int localnoticecheck(pr,rbuf,rlen,username,period)
	const char	pr[] ;
	char		rbuf[] ;
	int		rlen ;
	const char	username[] ;
	int		period ;

	Arguments:

	pr		PCS system program root (if available)
	rbuf		buffer to hold result
	rlen		length of supplied result buffer
	username	username to check
	period		period of time to include

	Returns:

	>=0		OK
	<0		some error


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>
#include	<pwd.h>

#include	<vsystem.h>
#include	<estrings.h>
#include	<bfile.h>
#include	<vecstr.h>
#include	<getax.h>
#include	<ugetpw.h>
#include	<mailbox.h>
#include	<mbcache.h>
#include	<mailmsg.h>
#include	<ema.h>
#include	<localmisc.h>


/* local defines */

#if	CF_UGETPW
#define	GETPW_NAME	ugetpw_name
#else
#define	GETPW_NAME	getpw_name
#endif /* CF_UGETPW */

#ifndef	LINEBUFLEN
#define	LINEBUFLEN	2048
#endif

#ifndef	MSGLINEBUFLEN
#define	MSGLINEBUFLEN	(LINEBUFLEN * 5)
#endif

#ifndef	LINEBUFLEN
#ifdef	LINE_MAX
#define	LINEBUFLEN	MAX(LINE_MAX,2048)
#else
#define	LINEBUFLEN	2048
#endif
#endif

#define	BUFLEN		(MAXPATHLEN + MAXHOSTNAMELEN + LINEBUFLEN)

#ifndef	VARPRPCS
#define	VARPRPCS	"PCS"
#endif

#ifndef	VARHOME
#define	VARHOME		"HOME"
#endif

#ifndef	VARUSERNAME
#define	VARUSERNAME	"USERNAME"
#endif

#ifndef	VARMAIL
#define	VARMAIL		"MAIL"
#endif

#ifndef	VARMAILDNAME
#define	VARMAILDNAME	"MAILDIR"
#endif

#ifndef	VARMAILDNAMES
#define	VARMAILDNAMES	"MAILDIRS"
#endif

#ifndef	SYSMAILDNAME
#define	SYSMAILDNAME	"/var/mail"
#endif

#define	SUBINFO		struct subinfo
#define	SUBINFO_FL	struct subinfo_flags

#define	ISEND(c)	(((c) == '\n') || ((c) == '\r'))


/* external subroutines */

extern int	sncpy1w(char *,int,const char *,int) ;
extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	sncpy3(char *,int,const char *,const char *,const char *) ;
extern int	snwcpy(char *,int,const char *,int) ;
extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	mknpath1(char *,int,const char *) ;
extern int	mknpath2(char *,int,const char *,const char *) ;
extern int	sfsubstance(const char *,int,const char **) ;
extern int	matkeystr(const char **,char *,int) ;
extern int	vstrkeycmp(const char **,const char **) ;
extern int	pathclean(char *,const char *,int) ;
extern int	mkbestfrom(char *,int,const char *,int) ;
extern int	getusername(char *,int,uid_t) ;
extern int	getuserhome(char *,int,cchar *) ;
extern int	compactstr(char *,int) ;
extern int	isNotPresent(int) ;
extern int	isOneOf(const int *,int) ;

#ifdef	COMMENT
extern int	mailmsg_loadfile(MAILMSG *,bfile *) ;
#endif /* COMMENT */

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
extern int	nprintf(const char *,const char *,...) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;
extern char	*strnpbrk(const char *,int,const char *) ;


/* external variables */


/* local structures */

struct subinfo_flags {
	uint		userself:1 ;
	uint		uid:1 ;
	uint		allocusername:1 ;
} ;

struct subinfo {
	const char	*pr ;
	const char	*varusername ;
	const char	*username ;
	char		*fbuf ;
	SUBINFO_FL	init, f ;
	time_t		dt ;
	uid_t		uid ;
	int		flen ;
	int		period ;
} ;


/* forward references */

static int	subinfo_start(struct subinfo *,const char *,
			char *,int,const char *,int) ;
static int	subinfo_finish(struct subinfo *) ;
static int	subinfo_username(struct subinfo *,const char *) ;

static int	procmailfile(char *,int,cchar *,time_t) ;
static int	procmailmsg(MBCACHE *,time_t,char *,int,int,int) ;

#ifdef	COMMENT
static int	mailbox_getfrom(MAILBOX *,const char *,int,char *,int) ;
#endif


/* local variables */


/* exported subroutines */


int localnoticecheck(pr,dbuf,dlen,un,period)
const char	pr[] ;
char		dbuf[] ;
int		dlen ;
const char	un[] ;
int		period ;
{
	struct subinfo	si, *sip = &si ;
	int		rs ;
	int		rs1 ;
	int		n = 0 ;

	if (dbuf == NULL) return SR_FAULT ;
	if (un == NULL) return SR_FAULT ;

#ifdef	COMMENT
	if (username[0] == '\0') return SR_INVALID ;
#endif

	if (pr == NULL) pr = getenv(VARPRPCS) ;

	if (pr == NULL) return SR_FAULT ;

#if	CF_DEBUGS
	debugprintf("localnoticecheck: ent user=%s\n",un) ;
#endif

	dbuf[0] = '\0' ;
	if ((rs = subinfo_start(sip,pr,dbuf,dlen,un,period)) >= 0) {
	    const int	hlen = MAXPATHLEN ;
	    char	hbuf[MAXPATHLEN+1] ;
	    if ((rs = getuserhome(hbuf,hlen,sip->username)) >= 0) {
	        cchar	*folder = "mail" ;
		cchar	*mbox = "notices" ;
	        char	mfbuf[MAXPATHLEN+1] ;
	        if ((rs = mkpath3(mfbuf,hbuf,folder,mbox)) >= 0) {
		    time_t	nt = (sip->dt-period) ;
		    if (nt >= 0) {
		        rs = procmailfile(sip->fbuf,sip->flen,mfbuf,nt) ;
		        n = rs ;
		    }
		}
	    } /* end if (getuserhome) */
	    rs1 = subinfo_finish(sip) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (subinfo) */

#if	CF_DEBUGS
	debugprintf("localnoticecheck: ret rs=%d n=%u\n",rs,n) ;
#endif

	return (rs >= 0) ? n : rs ;
}
/* end subroutine (localnoticecheck) */


/* local subroutines */


static int subinfo_start(sip,pr,fbuf,flen,un,period)
struct subinfo	*sip ;
const char	*pr ;
char		fbuf[] ;
int		flen ;
const char	un[] ;
int		period ;
{
	int		rs ;

	memset(sip,0,sizeof(struct subinfo)) ;
	sip->varusername = VARUSERNAME ;
	sip->pr = pr ;
	sip->fbuf = fbuf ;
	sip->flen = flen ;
	sip->period = period ;
	sip->dt = time(NULL) ;

	rs = subinfo_username(sip,un) ;

	return rs ;
}
/* end subroutine (subinfo_start) */


static int subinfo_finish(sip)
struct subinfo	*sip ;
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (sip->f.allocusername && (sip->username != NULL)) {
	    rs1 = uc_free(sip->username) ;
	    if (rs >= 0) rs = rs1 ;
	    sip->username = NULL ;
	}

	sip->pr = NULL ;
	return rs ;
}
/* end subroutine (subinfo_finish) */


static int subinfo_username(sip,un)
struct subinfo	*sip ;
const char	un[] ;
{
	int		rs = SR_OK ;

	sip->username = un ;
	if ((un == NULL) || (un[0] == '\0') || (un[0] == '-')) {
	    const char	*cp ;
	    if ((cp = getenv(sip->varusername)) != NULL) {
	        sip->username = cp ;
	    } else {
	        int	ulen = USERNAMELEN ;
	        char	ubuf[USERNAMELEN + 1] ;
	        if ((rs = getusername(ubuf,ulen,-1)) >= 0) {
	            if ((rs = uc_mallocstrw(ubuf,rs,&cp)) >= 0) {
	                sip->f.allocusername = TRUE ;
	                sip->username = cp ;
	            }
	        } /* end if */
	    } /* end if */
	} /* end if (getting username) */

	return rs ;
}
/* end subroutine (subinfo_username) */


static int procmailfile(char *rbuf,int rlen,cchar *mbfname,time_t nt)
{
	MAILBOX		mb ;
	MAILBOX_INFO	mbinfo ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		mbopts ;
	int		c = 0 ;

	if (rbuf == NULL) return SR_FAULT ;
	if (mbfname == NULL) return SR_FAULT ;

	if (mbfname[0] == '\0') return SR_INVALID ;

#if	CF_DEBUGS
	debugprintf("localnoticecheck/procmailfile: mbfname=%s\n",mbfname) ;
#endif

	rbuf[0] = '\0' ;
	mbopts = (MAILBOX_ORDONLY | MAILBOX_ONOCLEN) ;
	if ((rs = mailbox_open(&mb,mbfname,mbopts)) >= 0) {
	    MBCACHE	mc ;
	    if ((rs = mbcache_start(&mc,mbfname,0,&mb)) >= 0) {
	        if ((rs = mbcache_count(&mc)) > 0) {
	            int	mn = rs ;
	            int	mi ;
	            for (mi = (mn-1) ; mi >= 0 ; mi -= 1) {
	                rs = procmailmsg(&mc,nt,rbuf,rlen,mi,c) ;
			if (rs == 0) break ;
			c += 1 ;
	                if (rs < 0) break ;
	            } /* end for */
	        } /* end if (n-mesgs) */
	        rs1 = mbcache_finish(&mc) ;
		if (rs >= 0) rs = rs1 ;
	    } /* end if (mbcache) */
	    rs1 = mailbox_close(&mb) ;
	    if (rs >= 0) rs = rs1 ;
	} else if (isNotPresent(rs)) {
#if	CF_DEBUGS
	    debugprintf("localnoticecheck/procmailfile: mailbox-out rs=%d\n",
		rs) ;
#endif
	    rs = SR_OK ;
	}

#if	CF_DEBUGS
	debugprintf("localnoticecheck/procmailfile: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procmailfile) */


#ifdef	COMMENT
static int mailbox_getfrom(mbp,fname,mi,rbuf,rlen)
MAILBOX		*mbp ;
const char	fname[] ;
int		mi ;
char		rbuf[] ;
int		rlen ;
{
	MAILBOX_MSGINFO	msginfo ;
	MAILMSG		m ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		sl ;
	int		len = 0 ;
	const char	*sp ;

#if	CF_DEBUGS
	debugprintf("pcsmailcheck/mailbox_getfrom: mi=%d\n",mi) ;
#endif

	if (mbp == NULL) return SR_FAULT ;
	if (fname == NULL) return SR_FAULT ;
	if (rbuf == NULL) return SR_FAULT ;

	if (fname[0] == '\0') return SR_FAULT ;

	if (mi < 0) {
	    MAILBOX_INFO	mbinfo ;

	    rs1 = mailbox_info(mbp,&mbinfo) ;
	    if ((rs1 >= 0) && (mbinfo.nmsgs > 0))
	        mi = (mbinfo.nmsgs - 1) ;

	} /* end if (default) */

	if (mi >= 0) {
	    if ((rs = mailbox_msginfo(mbp,mi,&msginfo)) >= 0) {
		bfile		mf ;
	        if ((rs1 = bopen(&mf,fname,"r",0666)) >= 0) {
		    offset_t	moff = msginfo.moff ;
	            if ((rs1 = bseek(&mf,moff,SEEK_SET)) >= 0) {

#if	CF_DEBUGS
	                debugprintf("pcsmailcheck/mailbox_getfrom: "
				"moff=%ld\n", moff) ;
#endif

	                if ((rs1 = mailmsg_start(&m)) >= 0) {

/* load the message data into the MAILMSG object */

	                    if (rs1 >= 0)
	                        rs1 = mailmsg_loadfile(&m,&mf) ;

/* extract the MAILMSG information that we want */

	                    if (rs1 >= 0) {
	                        rs1 = mailmsg_hdrval(&m,HN_FROM,&sp) ;
	                        sl = rs1 ;
	                    }

	                    if ((rs1 < 0) || (sl == 0)) {
	                        rs1 = mailmsg_envaddress(&m,0,&sp) ;
	                        sl = rs1 ;
	                    }

/* parse the EMAs */

	                    if ((rs1 >= 0) && (sl > 0)) {
	                        rs = mkbestfrom(rbuf,rlen,sp,sl) ;
	                        len = rs ;
	                    }

	                    rs1 = mailmsg_finish(&m) ;
	                    if (rs >= 0) rs = rs1 ;
	                } /* end if (mailmsg) */

	            } /* end if (seek) */
	            rs1 = bclose(&mf) ;
	            if (rs >= 0) rs = rs1 ;
	        } /* end if (mail-file) */
	    } /* end if (msg-info) */
	} /* end if (positive MI) */

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (mailbox_getfrom) */
#endif /* COMMENT */


static int procmailmsg(MBCACHE *mbp,time_t nt,char *rbuf,int rlen,int mi,int c)
{
	MBCACHE_SCAN	*msp ;
	int		rs ;
	int		f = FALSE ;
#if	CF_DEBUGS
	debugprintf("localnoticecheck/procmailmsg: ent mi=%u c=%u\n",mi,c) ;
#endif
	if ((rs = mbcache_msgscan(mbp,mi,&msp)) >= 0) {
	        time_t		t = msp->htime ;
	        const int	*vl = msp->vl ;
		int		fl ;
	        const char	**vs = msp->vs ;
#if	CF_DEBUGS
	{
	    char	tbuf[TIMEBUFLEN+1] ;
	    timestr_logz(t,tbuf) ;
	    debugprintf("localnoticecheck/procmailmsg: hdr t=%s\n",tbuf) ;
	}
#endif
	        if (t == 0) t = msp->etime ;
#if	CF_DEBUGS
	{
	        char	tbuf[TIMEBUFLEN+1] ;
	        timestr_logz(t,tbuf) ;
	        debugprintf("localnoticecheck/procmailmsg: env t=%s\n",tbuf) ;
	}
#endif
		f = (t >= nt) ;
		if (f && (c == 0)) {
		    int		fl = vl[mbcachemf_hdrsubject] ;
	            cchar	*fp = vs[mbcachemf_hdrsubject] ;
#if	CF_DEBUGS
	    	debugprintf("localnoticecheck/procmailmsg: f=>%t<\n",
			fp,strlinelen(fp,fl,40)) ;
#endif
		    if (fp != NULL) {
			int	rl ;
		        rl = (strdcpy1w(rbuf,rlen,fp,fl) - rbuf) ;
			compactstr(rbuf,rl) ;
#if	CF_DEBUGS
	    	debugprintf("localnoticecheck/procmailmsg: mbf f=>%t<\n",
			rbuf,strlinelen(rbuf,rs,40)) ;
#endif
		    }
		}
	} /* end if (mbcache_msgscan) */
#if	CF_DEBUGS
	debugprintf("localnoticecheck/procmailmsg: ret rs=%d f=%u\n",rs,f) ;
#endif
	return (rs >= 0) ? f : rs ;
}
/* end subroutine (procmailmsg) */


