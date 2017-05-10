/* pcsmailcheck */

/* determine if the given user has mail (as PCS determines it) */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */
#define	CF_CVTFROM	0		/* convert FROM address */


/* revision history:

	= 1998-06-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine determines if the given user (supplied) has mail waiting
        (unread).

	Synopsis:

	int pcsmailcheck(pr,rbuf,rlen,un)
	const char	pr[] ;
	char		rbuf[] ;
	int		rlen ;
	const char	un[] ;

	Arguments:

	pr		PCS system program root (if available)
	rbuf		buffer to hold result
	rlen		length of supplied result buffer
	un		username to check

	Returns:

	>=0		OK
	<0		some error


	= The "mail" situation

        What follows is still based on the idea that mail comes from the system
        in a spool area. Here are the rules for finding the spool area:

        + the "MAIL" encironment variable can only hold ONE filename (this
        restriction is from the SHELL)

        + the "MAILPATH" environment variable is strictly a SHELL device and not
        related to system mail operations

        + the "MAILDIR" environment variable was introduced to specify the
        mail-spool-directory, but it was introduced to hold only a single
        directory name

        + so we use the newer "MAILDIRS" environment variable, which holds one
        or more mail-spool-directory names

        + a pool of mail-spool-directories is compiled by gathering up all
        directories from the the "MAIL", "MAILDIR", and "MAILDIRS" environment
        variables

	+ enjoy


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<limits.h>
#include	<stdlib.h>		/* for 'getenv(3c)' maybe others */
#include	<stddef.h>		/* for 'wchar_t' */
#include	<string.h>

#include	<vsystem.h>
#include	<estrings.h>
#include	<vecstr.h>
#include	<dirlist.h>
#include	<mailbox.h>
#include	<hdrdecode.h>
#include	<localmisc.h>


/* local defines */

#ifndef	TIME_MAX
#define	TIME_MAX	INT_MAX
#endif

#ifndef	MAILADDRLEN
#define	MAILADDRLEN	(3 * MAXHOSTNAMELEN)
#endif

#define	FBUFLEN		MAILADDRLEN

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

extern int	snwcpywidehdr(char *,int,const wchar_t *,int) ;
extern int	sncpy1w(char *,int,const char *,int) ;
extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	sncpy3(char *,int,const char *,const char *,const char *) ;
extern int	snwcpy(char *,int,const char *,int) ;
extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mknpath1(char *,int,const char *) ;
extern int	mknpath2(char *,int,const char *,const char *) ;
extern int	pathadd(char *,int,cchar *) ;
extern int	sfsubstance(const char *,int,const char **) ;
extern int	matkeystr(const char **,char *,int) ;
extern int	vstrkeycmp(const char **,const char **) ;
extern int	pathclean(char *,const char *,int) ;
extern int	mkbestfrom(char *,int,const char *,int) ;
extern int	mkdisphdr(char *,int,cchar *,int) ;
extern int	getusername(char *,int,uid_t) ;
extern int	getuid_name(cchar *,int) ;
extern int	mailbox_getfrom(MAILBOX *,char *,int,cchar *,int) ;
extern int	isNotPresent(int) ;
extern int	isOneOf(const int *,int) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;
extern char	*strnpbrk(const char *,int,const char *) ;
extern char	*strdcpy1w(char *,int,const char *,int) ;


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
	char		*a ;		/* allocation reference point */
	char		*rbuf ;		/* supplied argument */
	char		*fbuf ;		/* allocated */
	char		*tbuf ;
	SUBINFO_FL	init, f ;
	DIRLIST		maildirs ;
	time_t		ti_first ;
	uid_t		uid ;
	int		rlen ;		/* supplied argument */
	int		tlen ;
	int		flen ;		/* allocated amount */
	int		fl ;		/* actual used amount */
} ;


/* forward references */

static int	subinfo_start(SUBINFO *,cchar *,char *,int,cchar *) ;
static int	subinfo_finish(SUBINFO *) ;
static int	subinfo_username(SUBINFO *,cchar *) ;
static int	subinfo_getfrom(SUBINFO *) ;
static int	subinfo_cvtfrom(SUBINFO *) ;

#ifdef	COMMENT
static int	subinfo_userself(SUBINFO *) ;
static int	subinfo_getuid(SUBINFO *,uid_t *) ;
#endif

static int	subinfo_getsysmail(SUBINFO *) ;
static int	subinfo_mailfile(SUBINFO *) ;

static int	maildirs(SUBINFO *) ;
static int	maildirs_varmaildirs(SUBINFO *,const char *) ;
static int	maildirs_varmail(SUBINFO *,const char *) ;
static int	maildirs_default(SUBINFO *,const char *) ;
static int	maildirs_add(SUBINFO *,const char *,int) ;


/* local variables */

static int	(*getmails[])(SUBINFO *) = {
	subinfo_getsysmail,
	NULL
} ;

static const int	rsdirs[] = {
	SR_ACCESS,
	SR_NOENT,
	SR_NAMETOOLONG,
	SR_NOLINK,
	SR_NOTDIR,
	0
} ;


/* exported subroutines */


int pcsmailcheck(cchar *pr,char *dbuf,int dlen,cchar *un)
{
	SUBINFO		si ;
	int		rs ;
	int		rs1 ;
	int		n = 0 ;

#if	CF_DEBUGS
	debugprintf("pcsmailcheck: ent un=%s\n",un) ;
#endif

	if (dbuf == NULL) return SR_FAULT ;
	if (un == NULL) return SR_FAULT ;

#ifdef	COMMENT
	if (username[0] == '\0') return SR_INVALID ;
#endif

	if (pr == NULL) pr = getenv(VARPRPCS) ;

	if (pr == NULL) return SR_FAULT ;

	dbuf[0] = '\0' ;
	if ((rs = subinfo_start(&si,pr,dbuf,dlen,un)) >= 0) {
	    if ((rs = subinfo_getfrom(&si)) >= 0) {
		n = rs ;
		rs = subinfo_cvtfrom(&si) ;
	    }
	    rs1 = subinfo_finish(&si) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (subinfo) */

#if	CF_DEBUGS
	debugprintf("pcsmailcheck: ret rs=%d n=%u\n",rs,n) ;
	debugprintf("pcsmailcheck: ret f=>%s<\n",
		dbuf,strlinelen(dbuf,-1,50)) ;
#endif

	return (rs >= 0) ? n : rs ;
}
/* end subroutine (pcsmailcheck) */


/* local subroutines */


static int subinfo_start(SUBINFO *sip,cchar *pr,char *rbuf,int rlen,cchar *un)
{
	int		rs ;

	memset(sip,0,sizeof(SUBINFO)) ;
	sip->varusername = VARUSERNAME ;
	sip->pr = pr ;
	sip->rbuf = rbuf ;
	sip->rlen = rlen ;
	sip->ti_first = TIME_MAX ;

	if ((rs = dirlist_start(&sip->maildirs)) >= 0) {
	    if ((rs = subinfo_username(sip,un)) >= 0) {
		const int	tlen = MAXPATHLEN ;
		const int	flen = MAILADDRLEN ;
		int		size = 0 ;
	        char		*bp ;
		size += (tlen+1) ;
		size += (flen+1) ;
		if ((rs = uc_malloc(size,&bp)) >= 0) {
		    sip->a = bp ;
		    sip->tbuf = bp ;
		    sip->tlen = tlen ;
		    sip->fbuf = (bp + (tlen+1)) ;
		    sip->flen = flen ;
		}
	    }
	    if (rs < 0)
	        dirlist_finish(&sip->maildirs) ;
	} /* end if (dirlist_start) */

	return rs ;
}
/* end subroutine (subinfo_start) */


static int subinfo_finish(SUBINFO *sip)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (sip->f.allocusername && (sip->username != NULL)) {
	    rs1 = uc_free(sip->username) ;
	    if (rs >= 0) rs = rs1 ;
	    sip->username = NULL ;
	}

	if (sip->a != NULL) {
	    rs1 = uc_free(sip->a) ;
	    if (rs >= 0) rs = rs1 ;
	    sip->a = NULL ;
	    sip->tbuf = NULL ;
	    sip->tlen = 0 ;
	    sip->fbuf = NULL ;
	    sip->flen = 0 ;
	}

	rs1 = dirlist_finish(&sip->maildirs) ;
	if (rs >= 0) rs = rs1 ;

	sip->pr = NULL ;
	return rs ;
}
/* end subroutine (subinfo_finish) */


static int subinfo_username(SUBINFO *sip,cchar *un)
{
	int		rs = SR_OK ;
	sip->username = un ;
	if ((un == NULL) || (un[0] == '\0') || (un[0] == '-')) {
	    cchar	*cp ;
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


#ifdef	COMMENT
static int subinfo_userself(SUBINFO *sip)
{
	int		rs = SR_OK ;

	if (! sip->init.userself) {
	    const char	*cp ;

	    sip->init.userself = TRUE ;
	    if (((cp = getenv(sip->varusername)) != NULL) &&
	        (strcmp(cp,sip->username) == 0)) {

	        sip->f.userself = TRUE ;

	    } /* end if */

	} /* end if (initializing UID) */

	if ((rs >= 0) && (! sip->f.userself))
	    rs = SR_SRCH ;

	return rs ;
}
/* end subroutine (subinfo_userself) */
#endif /* COMMENT */


#ifdef	COMMENT
static int subinfo_getuid(SUBINFO *sip,uid_t *uidp)
{
	int		rs = SR_OK ;

	if (! sip->init.uid) {
	    cchar	*var = sip-.varusername ;
	    cchar	*un = sip->username ;
	    cchar	*cp ;
	    sip->init.uid = TRUE ;
	    if (((cp = getenv(var)) != NULL) && (strcmp(cp,un) == 0)) {
	        sip->f.uid = TRUE ;
	        sip->uid = getuid() ;
	    } else {
		if ((rs = getuid_name(un,-1)) >= 0) {
	            sip->f.uid = TRUE ;
	            sip->uid = rs ;
	        }
	    } /* end if */
	} /* end if (initializing UID) */

	if (uidp != NULL)
	    *uidp = sip->uid ;

	if ((rs >= 0) && (! sip->f.uid))
	    rs = SR_NOTFOUND ;

	return rs ;
}
/* end subroutine (subinfo_getuid) */
#endif /* COMMENT */


static int subinfo_getsysmail(SUBINFO *sip)
{
	int		rs ;
	int		rs1 ;
	int		c = 0 ;

#if	CF_DEBUGS
	debugprintf("pcsmailcheck/subinfo_getsysmail: ent\n") ;
#endif

	if ((rs = maildirs(sip)) >= 0) {
	    DIRLIST	*dlp = &sip->maildirs ;
	    DIRLIST_CUR	cur ;
	    if ((rs = dirlist_curbegin(dlp,&cur)) >= 0) {
	        const int	tlen = sip->tlen ;
	        int		dl ;
		cchar		*un = sip->username ;
	        char		*tbuf = sip->tbuf ;
	        while (rs >= 0) {
	            dl = dirlist_enum(dlp,&cur,tbuf,tlen) ;
	            if (dl == SR_NOTFOUND) break ;
		    rs = dl ;
#if	CF_DEBUGS
		    debugprintf("pcsmailcheck/subinfo_getsysmail: "
			    "dirlist_enum() rs=%d\n",rs) ;
		    debugprintf("pcsmailcheck/subinfo_getsysmail: "
			    "md=%t\n",tbuf,dl) ;
#endif
		    if (rs >= 0) {
	                if ((rs = pathadd(tbuf,dl,un)) >= 0) {
			    rs = subinfo_mailfile(sip) ;
			    c += rs ;
			} /* end if (pathadd) */
	            } /* end if (ok) */
	        } /* end while */
	        rs1 = dirlist_curend(dlp,&cur) ;
		if (rs >= 0) rs = rs1 ;
	    } /* end if (dirlist-cursor) */
	} /* end if (maildirs) */

#if	CF_DEBUGS
	debugprintf("pcsmailcheck/subinfo_getsysmail: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (subinfo_getsysmail) */


static int subinfo_mailfile(SUBINFO *sip)
{
	struct ustat	sb ;
	int		rs ;
	int		rs1 ;
	int		c = 0 ;
	cchar		*mfn = sip->tbuf ;

#if	CF_DEBUGS
	debugprintf("pcsmailcheck/subinfo_mailfile: ent mfn=%s\n",mfn) ;
#endif

	if ((rs = u_stat(mfn,&sb)) >= 0) {
	    if (S_ISREG(sb.st_mode) && (sb.st_size > 0)) {
		MAILBOX		mb ;
		MAILBOX_INFO	mbinfo ;
		const int	mo = (MAILBOX_ORDONLY | MAILBOX_ONOCLEN) ;
		if ((rs = mailbox_open(&mb,mfn,mo)) >= 0) {
	    	    if ((rs = mailbox_info(&mb,&mbinfo)) >= 0) {
	                c = mbinfo.nmsgs ;

#if	CF_DEBUGS
	        	debugprintf("pcsmailcheck/subinfo_mailfile: nmsgs=%u\n",
				c) ;
#endif

	        	if (c > 0) {
			    const int	mi = (c-1) ;
			    const int	tl = sip->flen ;
			    char	*tb = sip->fbuf ;
	            	    if ((rs = mailbox_getfrom(&mb,tb,tl,mfn,mi)) >= 0) {
				sip->fl = rs ; /* returned length */

#if	CF_DEBUGS
	        	    debugprintf("pcsmailcheck/subinfo_mailfile: "
				"mailbox_getfrom() rs=%d\n",rs) ;
	        	    debugprintf("pcsmailcheck/subinfo_mailfile: "
				"f=>%t<\n",sip->fbuf,rs) ;
#endif
			    } else if (rs == SR_OVERFLOW) {
				tb[0] = '\0' ;
				rs = SR_OK ;
			    }
			} /* end if (positive) */

	            } /* end if (n-msgs) */
	            rs1 = mailbox_close(&mb) ;
	            if (rs >= 0) rs = rs1 ;
	        } /* end if (mailbox) */
	    } /* end if (reg-file) */
	} else if (isNotPresent(rs)) {
	    rs = SR_OK ;
	} /* end if (exists) */

#if	CF_DEBUGS
	debugprintf("pcsmailcheck/subinfo_mailfile: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (subinfo_mailfile) */


static int subinfo_getfrom(SUBINFO *sip)
{
	int		rs = SR_OK ;
	int		n = 0 ;
	int		i ;
	for (i = 0 ; (rs >= 0) && (getmails[i] != NULL) ; i += 1) {
	    rs = (*getmails[i])(sip) ;
	    n += rs ;
	} /* end for */
#if	CF_DEBUGS
	debugprintf("pcsmailcheck/subinfo_getfrom: ret rs=%d n=%u\n",rs,n) ;
#endif
	return (rs >= 0) ? n : rs ;
}
/* end subroutine (subinfo_getfrom) */


static int subinfo_cvtfrom(SUBINFO *sip)
{
	int		rs ;
	int		rs1 ;
	int		fl = sip->fl ;
	int		wlen = sip->fl ;
	int		size ;
	int		len = 0 ;
	wchar_t		*wbuf ;
#if	CF_DEBUGS
	debugprintf("pcsmailcheck/subinfo_cvtfrom: ent\n") ;
#endif
	size = ((wlen+1)*sizeof(wchar_t)) ;
	if ((rs = uc_malloc(size,&wbuf)) >= 0) {
	    HDRDECODE	d ;
#if	CF_DEBUGS
	debugprintf("pcsmailcheck/subinfo_cvtfrom: hdrdecode\n") ;
#endif
	    if ((rs = hdrdecode_start(&d,sip->pr)) >= 0) {
		cchar		*fbuf = sip->fbuf ;
#if	CF_DEBUGS
	debugprintf("pcsmailcheck/subinfo_cvtfrom: mid1\n") ;
#endif
		if ((rs = hdrdecode_proc(&d,wbuf,wlen,fbuf,fl)) >= 0) {
		    const int	dlen = (2*rs) ;
		    int		wl = rs ;
		    char	*dbuf ;
		    if ((rs = uc_malloc((dlen+1),&dbuf)) >= 0) {
			if (wl > sip->rlen) wl = sip->rlen ;
		        if ((rs = snwcpywidehdr(dbuf,dlen,wbuf,wl)) >= 0) {
			    rs = mkdisphdr(sip->rbuf,sip->rlen,dbuf,rs) ;
		            len = rs ;
		        }
			rs1 = uc_free(dbuf) ;
			if (rs >= 0) rs = rs1 ;
		    } /* end if (m-a-f) */
		}
		rs1 = hdrdecode_finish(&d) ;
		if (rs >= 0) rs = rs1 ;
	    } /* end if (hdrdecode) */
	    rs1 = uc_free(wbuf) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (m-a-f) */
#if	CF_DEBUGS
	debugprintf("pcsmailcheck/subinfo_cvtfrom: ret rs=%d len=%u\n",rs,len) ;
#endif
	return (rs >= 0) ? len : rs ;
}
/* end subroutine (subinfo_cvtfrom) */


static int maildirs(SUBINFO *sip)
{
	int		rs = SR_OK ;
	int		c = 0 ;
	const char	*cp ;

	if ((rs >= 0) && ((cp = getenv(VARMAILDNAME)) != NULL)) {
	    rs = maildirs_varmaildirs(sip,cp) ;
	    c += rs ;
	}

#if	CF_DEBUGS
	debugprintf("pcsmailcheck/maildirs: maildirs1 rs=%d\n",rs) ;
#endif

	if ((rs >= 0) && ((cp = getenv(VARMAILDNAMES)) != NULL)) {
	    rs = maildirs_varmaildirs(sip,cp) ;
	    c += rs ;
	}

#if	CF_DEBUGS
	debugprintf("pcsmailcheck/maildirs: maildirs2 rs=%d\n",rs) ;
#endif

	if ((rs >= 0) && ((cp = getenv(VARMAIL)) != NULL)) {
	    rs = maildirs_varmail(sip,cp) ;
	    c += rs ;
	}

#if	CF_DEBUGS
	debugprintf("pcsmailcheck/maildirs: maildirs3 rs=%d\n",rs) ;
#endif

	if (rs >= 0) {
	    rs = maildirs_default(sip,SYSMAILDNAME) ;
	    c += rs ;
	}

#if	CF_DEBUGS
	debugprintf("pcsmailcheck/maildirs: maildirs4 rs=%d\n",rs) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (maildirs) */


static int maildirs_varmaildirs(SUBINFO *sip,cchar *sp)
{
	DIRLIST		*vlp = &sip->maildirs ;
	int		rs = SR_OK ;
	int		sl, cl ;
	int		c = 0 ;
	const char	*tp, *cp ;

	if (vlp == NULL) return SR_FAULT ;
	if (sp == NULL) return SR_FAULT ;

	sl = strlen(sp) ;

	while ((tp = strnpbrk(sp,sl," \t,:;")) != NULL) {
	    if ((cl = sfshrink(sp,(tp - sp),&cp)) > 0) {
	        rs = maildirs_add(sip,cp,cl) ;
	        c += rs ;
	    } /* end if */
	    sl -= ((tp + 1) - sp) ;
	    sp = (tp + 1) ;
	    if (rs < 0) break ;
	} /* end while */

	if ((rs >= 0) && (sl > 0)) {
	    if ((cl = sfshrink(sp,sl,&cp)) > 0) {
	        rs = maildirs_add(sip,cp,cl) ;
	        c += rs ;
	    } /* end if */
	} /* end if */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (maildirs_varmaildirs) */


static int maildirs_varmail(SUBINFO *sip,cchar *mvfn)
{
	DIRLIST		*vlp = &sip->maildirs ;
	int		rs = SR_OK ;
	int		c = 0 ;

	if (vlp == NULL) return SR_FAULT ;

	if ((mvfn != NULL) && (mvfn[0] != '\0')) {
	    int		cl ;
	    cchar	*cp ;
	    if ((cl = sfdirname(mvfn,-1,&cp)) > 0) {
	        rs = maildirs_add(sip,cp,cl) ;
	        c += rs ;
	    } /* end if */
	} /* end if */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (maildirs_varmail) */


static int maildirs_default(SUBINFO *sip,cchar *sp)
{
	int		rs = SR_INVALID ;
	int		c = 0 ;

	if (sp != NULL) {
	    rs = SR_OK ;
	    if (sp[0] != '\0') {
	        rs = maildirs_add(sip,sp,-1) ;
	        c += rs ;
	    } /* end if */
	} /* end if (non-null) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (maildirs_default) */


static int maildirs_add(SUBINFO *sip,cchar *cp,int cl)
{
	DIRLIST		*vlp = &sip->maildirs ;
	int		rs ;
	int		f_added = FALSE ;
	if ((rs = dirlist_add(vlp,cp,cl)) >= 0) {
	    f_added = (rs > 0) ;
	} else if (isOneOf(rsdirs,rs)) {
	    rs = SR_OK ;
	}
	return (rs >= 0) ? f_added : rs ;
}
/* end subroutine (maildirs_add) */


