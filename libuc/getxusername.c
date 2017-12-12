/* getxusername */

/* get the best approximation of the user's username */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DEBUGUID	0		/* debug UID-only calls */
#define	CF_UTMPACC	1		/* use |utmpacc(3uc)| */
#define	CF_GETUTMPNAME	1		/* use |getutmpname(3dam)| */
#define	CF_UGETPW	1		/* use |ugetpw(3uc)| */


/* revision history:

	= 1998-07-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        Get the user's username, the best that we can. These things have a
        certain level of confusion about them. What is a 'username'? After all
        of these years, one would think that we know the answer to that
        question!

	Note:

	We try to find the proper username in the order in which the user would
	expect.  Happily for us, the cheaper and more likely methods come
	first, and the heavier and more expensive methods come later (pretty
	much).  Although, in all cases a lookup into the system PASSWD database
	is required (itself not always a cheap or fast operation).  In fact, a
	lookup to the system PASSWD database can be much more expensive than a
	lookup into the system UTMPX database (otherwise thought to be more
	expensive than a PASSWD lookup).  We guard against making multiple
	PASSWD database requests for the same name (to save time).

	The following are available:

	= GETXUSERNAME

	Synopsis:

	int getxusername(xup)
	struct getxusername	*xup ;

	Arguments:

	xup		pointer to special (essentially private) data structure
	uid		user-id

	Returns:

	>=0		length of resulting username
	<0		error

	= GETUSERNAME

	Synopsis:

	int getusername(ubuf,ulen,uid)
	char		ubuf[] ;
	int		ulen ;
	uid_t		uid ;

	Arguments:

	ubuf		buffer to receive username
	ulen		length of supplied buffer
	uid		user-id

	Returns:

	>=0		length of resulting username
	<0		error

	= GETPWUSERNAME

	Synopsis:

	int getpwusername(pwp,pwbuf,pwlen,uid)
	struct passwd	*pwp ;
	char		pwbuf[] ;
	int		pwlen ;
	uid_t		uid ;

	Arguments:

	pwp		pointer to PASSWD structure (to receive results)
	pwbuf		supplied buffer to hold information
	pwlen		length of supplied buffer
	uid		user-id

	Returns:

	>=0		length of resulting username
	<0		error

	Notes:

	Q. Is this module multi-thread safe?
	A. Duh!  Of course.

	Q. Where are the traditional locks protecting the common data?
	A. None are needed.

	Q. How can no locks be needed and still be multi-thread safe?
	A. We forgo locks by accepting the (very) slight risk of the
	   code running more than once (once in each of one or more separate
	   threads).

	Q. Is this little scheme really OK?
	A. Yes.

	Q. I still feel uncomfortable.
	A. Deal with it.

	Implementation notes:

        Forst we try to look up the name is the local program cache. Failing the
        cache lookup, we go through various ways of guessing what our username
        is. Each time we guess a name, we have to verify it by looking it up in
        the system PASSWD database. We do that by calling the subroutines
        |getxusername_lookup()| below. As soon as a guess of a name is verified,
        we return the guess as the answer. Finally, when we find am answer,
	we put it into the local program cache (if we had not retrieved it from
	there in the first place).


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>
#include	<pwd.h>

#include	<vsystem.h>
#include	<ugetpid.h>
#include	<getbufsize.h>
#include	<vecstr.h>
#include	<getax.h>
#include	<uproguser.h>
#include	<ugetpw.h>
#include	<utmpacc.h>
#include	<localmisc.h>

#include	"getxusername.h"


/* local defines */

#if	CF_UGETPW
#define	GETPW_NAME	ugetpw_name
#define	GETPW_UID	ugetpw_uid
#else
#define	GETPW_NAME	getpw_name
#define	GETPW_UID	getpw_uid
#endif /* CF_UGETPW */

#ifndef	TIMEBUFLEN
#define	TIMEBUFLEN	80
#endif

#ifndef	LOGNAMELEN
#ifdef	LOGNAME_MAX
#define	LOGNAMELEN	LOGNAME_MAX
#else
#define	LOGNAMELEN	32
#endif
#endif

#ifndef	USERNAMELEN
#ifdef	LOGNAME_MAX
#define	USERNAMELEN	LOGNAME_MAX
#else
#define	USERNAMELEN	32
#endif
#endif

#ifndef	VARHOME
#define	VARHOME		"HOME"
#endif

#ifndef	VARMAIL
#define	VARMAIL		"MAIL"
#endif

#ifndef	VARUSERNAME
#define	VARUSERNAME	"USERNAME"
#endif

#ifndef	VARUSER
#define	VARUSER		"USER"
#endif

#ifndef	VARLOGNAME
#define	VARLOGNAME	"LOGNAME"
#endif

#define	GETXSTATE	struct getxusername_state

#define	DEBFNAME	"/var/tmp/debuguid.txt"


/* external subroutines */

extern int	snsd(char *,int,const char *,uint) ;
extern int	snwcpy(char *,int,const char *,int) ;
extern int	sncpy1(char *,int,const char *) ;
extern int	sfbasename(const char *,int,const char **) ;
extern int	getutmpname(char *,int,pid_t) ;
extern int	isNotPresent(int) ;

#if	CF_DEBUGS
extern int	debugopen(const char *) ;
extern int	debugprintf(const char *,...) ;
extern int	debugclose() ;
extern int	strlinelen(const char *,int,int) ;
#endif

#if	CF_DEBUGUID
extern int	nprintf(const char *,const char *,...) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;
extern char	*timestr_logz(time_t,char *) ;


/* external variables */


/* local structures */

struct mapent {
	uid_t		uid ;
	const char	*name ;
} ;


/* forward references */

static int	getusernamer(char *,int,uid_t) ;

int		getxusername(GETXUSERNAME *) ;

static int	getxusername_self(GETXUSERNAME *) ;
static int	getxusername_varusername(GETXUSERNAME *) ;
static int	getxusername_varuser(GETXUSERNAME *) ;
static int	getxusername_home(GETXUSERNAME *) ;
static int	getxusername_mail(GETXUSERNAME *) ;
static int	getxusername_varlogname(GETXUSERNAME *) ;
static int	getxusername_utmp(GETXUSERNAME *) ;
static int	getxusername_map(GETXUSERNAME *) ;
static int	getxusername_uid(GETXUSERNAME *) ;

static int	getxusername_var(GETXUSERNAME *,const char *) ;
static int	getxusername_varbase(GETXUSERNAME *,const char *) ;
static int	getxusername_lookup(GETXUSERNAME *,const char *) ;

#if	CF_DEBUGUID
static int	logpop(uid_t) ;
#endif


/* local variables */

static int	(*getxusernames[])(GETXUSERNAME *) = {
	getxusername_self,
	getxusername_varusername,
	getxusername_varuser,
	getxusername_home,
	getxusername_mail,
	getxusername_varlogname,
	getxusername_utmp,
	getxusername_map,
	getxusername_uid,
	NULL
} ;

static const struct mapent	mapents[] = {
	{ 0, "root" },
	{ 60001, "nobody" },
	{ 60002, "noaccess" },
	{ 65534, "nobody4" },
	{ -1, NULL }
} ;


/* exported subroutines */


int getusername(char ubuf[],int ulen,uid_t uid)
{
	int		rs ;

#if	CF_DEBUGS
	debugprintf("getusername: ent uid=%d\n",uid) ;
#endif

	if (ubuf == NULL) return SR_FAULT ;
	if (ulen == 0) return SR_INVALID ;
	if (ulen < 0) ulen = USERNAMELEN ;

	ubuf[0] = '\0' ;
	if ((rs = uproguser_nameget(ubuf,ulen,uid)) == 0) {
#if	CF_DEBUGS
	    debugprintf("getusername: full search\n") ;
#endif
	    rs = getusernamer(ubuf,ulen,uid) ;
	} /* end if (uproguser_nameget) */

#if	CF_DEBUGS
	debugprintf("getusername: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (getusername) */


int getpwusername(struct passwd *pwp,char pwbuf[],int pwlen,uid_t uid)
{
	GETXUSERNAME	xu ;
	int		rs ;
	char		ubuf[USERNAMELEN + 1] ;

#if	CF_DEBUGS
	debugprintf("getpwusername: ent uid=%d\n",uid) ;
#endif

	if (pwp == NULL) return SR_FAULT ;
	if (pwbuf == NULL) return SR_FAULT ;

	memset(&xu,0,sizeof(struct getxusername)) ;
	xu.pwp = pwp ;
	xu.pwbuf = pwbuf ;
	xu.pwlen = pwlen ;
	xu.ubuf = ubuf ;
	xu.ulen = USERNAMELEN ;
	xu.uid = uid ;

	rs = getxusername(&xu) ;

#if	CF_DEBUGS
	debugprintf("getpwusername: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (getpwusername) */


int getxusername(GETXUSERNAME *xup)
{
	const int	ttl = GETXUSERNAME_TTL ;
	int		rs ;
	int		rs1 ;
	int		pwl = 0 ;

	if (xup == NULL) return SR_FAULT ;
	if (xup->pwp == NULL) return SR_FAULT ;
	if (xup->ubuf == NULL) return SR_FAULT ;

#if	CF_DEBUGS
	debugprintf("getxusername: ent uid=%u\n",xup->uid) ;
#endif

	xup->ubuf[0] = '\0' ;
	if (xup->uid < 0) {
	    xup->f_self = TRUE ;
	    xup->uid = getuid() ;
	} else {
	    const uid_t	suid = getuid() ;
	    xup->f_self = (xup->uid == suid) ;
	}

#if	CF_DEBUGS
	debugprintf("getxusername: begin uid=%u\n",xup->uid) ;
#endif

	if ((rs = vecstr_start(&xup->names,10,0)) >= 0) {
	    int		i ;

	    for (i = 0 ; getxusernames[i] != NULL ; i += 1) {

#if	CF_DEBUGS
	        debugprintf("getxusername: trying i=%u\n",i) ;
#endif

	        rs = (*getxusernames[i])(xup) ;

#if	CF_DEBUGS
	        {
	            struct passwd	*pwp = xup->pwp ;
	            debugprintf("getxusername: back i=%u rs=%d\n",i,rs) ;
		    if ((rs > 0) && (pwp != NULL)) {
			const char	*name = pwp->pw_name ;
	                debugprintf("getxusername: back un=%s\n",name) ;
		    }
	        }
#endif /* CF_DEBUGS */

	        if (rs != 0) break ;
	    } /* end for */
	    pwl = rs ;

#if	CF_DEBUGS
	    debugprintf("getxusername: out rs=%d pwl=%u f_self=%u\n",
		rs,pwl,xup->f_self) ;
#endif

	    if ((rs > 0) && xup->f_self) {
		struct passwd	*pwp = xup->pwp ;
		rs = uproguser_nameset(pwp->pw_name,-1,xup->uid,ttl) ;
#if	CF_DEBUGS
		debugprintf("getxusername: PW un=>%s<\n",pwp->pw_name) ;
#endif
	    } /* end if (cache store) */

	    rs1 = vecstr_finish(&xup->names) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (vecstr) */
	if ((rs >= 0) && (pwl == 0)) rs = SR_NOTFOUND ;

#if	CF_DEBUGS
	debugprintf("getxusername: ret rs=%d pwl=%u\n",rs,pwl) ;
	if ((rs >= 0) && (pwl > 0)) {
	    const char	*name = xup->pwp->pw_name ;
	    debugprintf("getxusername: ret username=%s\n",name) ;
	}
#endif

	return (rs >= 0) ? pwl : rs ;
}
/* end subroutine (getxusername) */


/* local subroutines */


static int getusernamer(char ubuf[],int ulen,uid_t uid)
{
	GETXUSERNAME	xu ;
	struct passwd	pw ;
	const int	pwlen = getbufsize(getbufsize_pw) ;
	int		rs ;
	char		*pwbuf ;

#if	CF_DEBUGS
	debugprintf("getusernamer: ent uid=%d\n",uid) ;
#endif

#ifdef	COMMENT
	memset(&pw,0,sizeof(struct passwd)) ;
#endif

	if ((rs = uc_malloc((pwlen+1),&pwbuf)) >= 0) {
	    memset(&xu,0,sizeof(struct getxusername)) ;
	    xu.pwp = &pw ;
	    xu.pwbuf = pwbuf ;
	    xu.pwlen = pwlen ;
	    xu.ubuf = ubuf ;
	    xu.ulen = ulen ;
	    xu.uid = uid ;
	    xu.f_tried = TRUE ;
	    if ((rs = getxusername(&xu)) >= 0) {
	        rs = xu.unl ;
	        if (xu.unl <= 0) {
	            rs = sncpy1(ubuf,ulen,pw.pw_name) ;
	        }
	    } else if (rs == SR_NOTFOUND) {
	        uint	v = xu.uid ;
#if	CF_DEBUGUID
	        logpop(xu.uid) ;
#endif /* CF_DEBUGUID */
	        rs = snsd(ubuf,ulen,"U",v) ;
	    }
	    uc_free(pwbuf) ;
	} /* end if (m-a) */

#if	CF_DEBUGS
	debugprintf("getusernamer: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (getusernamer) */


static int getxusername_self(GETXUSERNAME *xup)
{
	int		rs = SR_OK ;
#if	CF_DEBUGS
	debugprintf("getxusername_self: f_self=%u\n",xup->f_self) ;
#endif
	xup->unl = 0 ;
	if (xup->f_self && (! xup->f_tried)) {
	    if ((rs = uproguser_nameget(xup->ubuf,xup->ulen,xup->uid)) > 0) {
#if	CF_DEBUGS
		debugprintf("getxusername_self: need search u=%s\n",
			xup->ubuf) ;
#endif
		xup->unl = rs ;
		rs = getxusername_lookup(xup,xup->ubuf) ;
	    } /* end if (uproguser_nameget) */
	} /* end if (self) */
#if	CF_DEBUGS
	debugprintf("getxusername_self: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (getxusername_self) */


static int getxusername_varusername(GETXUSERNAME *xup)
{

	return getxusername_var(xup,VARUSERNAME) ;
}
/* end subroutine (getxusername_varusername) */


static int getxusername_varuser(GETXUSERNAME *xup)
{

	return getxusername_var(xup,VARUSER) ;
}
/* end subroutine (getxusername_varuser) */


static int getxusername_varlogname(GETXUSERNAME *xup)
{

	return getxusername_var(xup,VARLOGNAME) ;
}
/* end subroutine (getxusername_varlogname) */


static int getxusername_home(GETXUSERNAME *xup)
{

	return getxusername_varbase(xup,VARHOME) ;
}
/* end subroutine (getxusername_home) */


static int getxusername_mail(GETXUSERNAME *xup)
{

	return getxusername_varbase(xup,VARMAIL) ;
}
/* end subroutine (getxusername_mail) */


static int getxusername_utmp(GETXUSERNAME *xup)
{
	int		rs ;
	const char	*np ;

#if	CF_DEBUGS
	debugprintf("getxusername_utmp: ent\n") ;
#endif

#if	CF_UTMPACC
	{
	    UTMPACC_ENT	ue ;
	    const int	uelen = UTMPACC_BUFLEN ;
	    char	uebuf[UTMPACC_BUFLEN+1] ;
	    if ((rs = utmpacc_entsid(&ue,uebuf,uelen,0)) >= 0) {
#if	CF_DEBUGS
		debugprintf("getxusername_utmp: ulen=%d\n",xup->ulen) ;
		debugprintf("getxusername_utmp: u=%s\n",ue.user) ;
#endif
		if (ue.user != NULL) {
		    rs = sncpy1(xup->ubuf,xup->ulen,ue.user) ;
		} else {
		    rs = SR_NOTFOUND ;
		    xup->ubuf[0] = '\0' ;
		}
	    } /* end if (utmpacc-entsid) */
	}
#else /* CF_UTMPACC */
#if	CF_GETUTMPNAME
	rs = getutmpname(xup->ubuf,xup->ulen,0) ;
#else
	rs = uc_getlogin(xup->ubuf,xup->ulen) ;
#endif
#endif /* CF_UTMPACC */

#if	CF_DEBUGS
	debugprintf("getxusername_utmp: mid rs=%d\n",rs) ;
#endif

	xup->unl = rs ;
	if (rs >= 0) {
	    np = xup->ubuf ;
	    if (*np != '\0') {
	        rs = getxusername_lookup(xup,np) ;
	    } else {
	        rs = SR_OK ;
	    }
	} else if (isNotPresent(rs)) {
	    rs = SR_OK ;
	}

#if	CF_DEBUGS
	debugprintf("getxusername_utmp: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (getxusername_utmp) */


static int getxusername_map(GETXUSERNAME *xup)
{
	uid_t		uid = xup->uid ;
	int		rs = SR_OK ;
	int		i ;
	int		f = FALSE ;

	xup->unl = 0 ;
	for (i = 0 ; mapents[i].uid >= 0 ; i += 1) {
	    f = (uid == mapents[i].uid) ;
	    if (f) break ;
	} /* end for */

	if (f) {
	    const char	*np = mapents[i].name ;
	    rs = getxusername_lookup(xup,np) ;
	}

	return rs ;
}
/* end subroutine (getxusername_map) */


static int getxusername_uid(GETXUSERNAME *xup)
{
	int		rs ;

	xup->unl = 0 ;
	if ((rs = GETPW_UID(xup->pwp,xup->pwbuf,xup->pwlen,xup->uid)) >= 0) {
	    if (xup->pwp->pw_name[0] != '\0') {
		xup->pwl = rs ;
	    } else {
		rs = SR_OK ;
	    }
	} else if (isNotPresent(rs)) {
	    rs = SR_OK ;
	}

	return rs ;
}
/* end subroutine (getxusername_uid) */


static int getxusername_var(GETXUSERNAME *xup,cchar varname[])
{
	int		rs = SR_OK ;
	const char	*vp = getenv(varname) ;

	if (vp != NULL) {
	    xup->unl = 0 ;
	    rs = getxusername_lookup(xup,vp) ;
	}

	return rs ;
}
/* end subroutine (getxusername_var) */


static int getxusername_varbase(GETXUSERNAME *xup,cchar varname[])
{
	int		rs = SR_OK ;
	const char	*vp = getenv(varname) ;

	if (vp != NULL) {
	    int		nl ;
	    cchar	*np ;
	    if ((nl = sfbasename(vp,-1,&np)) > 0) {
	        while ((nl > 0) && (np[nl - 1] == '/')) {
	            nl -= 1 ;
	        }
	        if ((nl > 0) && (np[0] != '/')) {
	            if ((rs = snwcpy(xup->ubuf,xup->ulen,np,nl)) >= 0) {
	                xup->unl = rs ;
	                np = xup->ubuf ;
	                rs = getxusername_lookup(xup,np) ;
	            }
	        }
	    } /* end if */
	} /* end if (non-null) */

	return rs ;
}
/* end subroutine (getxusername_varbase) */


static int getxusername_lookup(GETXUSERNAME *xup,cchar *np)
{
	int		rs ;
	int		pwl = 0 ;

#if	CF_DEBUGS
	debugprintf("getxusername_lookup: n=%s\n",np) ;
#endif

	if ((rs = vecstr_find(&xup->names,np)) == SR_NOTFOUND) {
	    const int	pwlen = xup->pwlen ;
	    char	*pwbuf = xup->pwbuf ;
	    if ((rs = GETPW_NAME(xup->pwp,pwbuf,pwlen,np)) >= 0) {
	        if (xup->pwp->pw_uid == xup->uid) {
		    xup->pwl = rs ;
	            pwl = rs ;
		} else {
		    rs = SR_NOTFOUND ;
		}
	    } /* end if (GETPW_NAME) */
	    if (rs == SR_NOTFOUND) {
	        rs = vecstr_add(&xup->names,np,-1) ;
	    }
	} /* end if (search) */

#if	CF_DEBUGS
	{
	    struct passwd	*pwp = xup->pwp ;
	    debugprintf("getxusername_lookup: ret rs=%d pwl=%u\n",rs,pwl) ;
	    debugprintf("getxusername_lookup: un=%s\n",pwp->pw_name) ;
	}
#endif

	return (rs >= 0) ? pwl : rs ;
}
/* end subroutine (getxusername_lookup) */


#if	CF_DEBUGUID
static int logpop(uid_t uid)
{
	time_t		daytime = time(NULL) ;
	int		rs ;
	const char	*pp = getexecname() ;
	char		timebuf[TIMEBUFLEN + 1] ;
	rs = nprintf(DEBFNAME,"%-23s p=%d u=%u ef=%s\n",
	    timestr_logz(daytime,timebuf),
	    ugetpid(),
	    uid,
	    pp) ;
	return rs ;
}
/* end subroutine (logpop) */
#endif /* CF_DEBUGUID */


