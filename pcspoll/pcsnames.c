/* pcsgetnames */

/* get various information elements related to the PCS environment */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */
#define	CF_DEFPCS	1		/* try a default PCS program-root */
#define	CF_UGETPW	1		/* use |ugetpw(3uc)| */


/* revision history:

	= 1998-06-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine retrieves the real name of a user according to what the
	PCS thinks it might be.  This subroutine is generally called by PCS
	programs.

	Synopsis:

	int pcsname(pr,rbuf,rlen,username)
	const char	pr[] ;
	char		rbuf[] ;
	int		rlen ;
	const char	username[] ;

	Arguments:

	pr		PCS system program root (if available)
	rbuf		buffer to hold result
	rlen		length of supplied result buffer
	username	username to check

	Returns:

	>=0		OK
	<0		some error


	Implementation note:

	Is "home-searching" faster than just asking the system for the home
	directory of a user?  It probably is not!  In reality, it is a matter
	of interacting with the Automount server probably much more than what
	would have happened if we just first interacted with the
	Name-Server-Cache and then the Automount server once after that.

	Note that we NEVER 'stat(2)' a file directly within any of the possible
	base directories for home-directories.  This is done to avoid those
	stupid SYSLOG entries saying that a file could not be found within an
	indirect auto-mount point -- what the base directories usually are
	now-a-days!

	On second thought, avoiding asking the "system" may be a good course of
	action since even though we are going through the name-server cache, it
	more often than not has to go out to some wirdo-only-knows network NIS+
	(whatever) server to get the PASSWD information.  This is not unusual
	in a large organization.


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
#include	<project.h>
#include	<netdb.h>

#include	<vsystem.h>
#include	<getbufsize.h>
#include	<char.h>
#include	<bfile.h>
#include	<sbuf.h>
#include	<fsdir.h>
#include	<getax.h>
#include	<ugetpw.h>
#include	<getxusername.h>
#include	<localmisc.h>


/* local defines */

#if	CF_UGETPW
#define	GETPW_NAME	ugetpw_name
#else
#define	GETPW_NAME	getpw_name
#endif /* CF_UGETPW */

#ifndef	NULLFNAME
#define	NULLFNAME	"/dev/null"
#endif

#ifndef	NSYSPIDS
#define	NSYSPIDS	100
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

#ifndef	VARNAME
#define	VARNAME		"NAME"
#endif

#ifndef	VARFULLNAME
#define	VARFULLNAME	"FULLNAME"
#endif

#ifndef	DEFPROJNAME
#define	DEFPROJNAME	"default"
#endif

#ifndef	NAMEFNAME
#define	NAMEFNAME	".name"
#endif

#ifndef	FULLNAMEFNAME
#define	FULLNAMEFNAME	".fullname"
#endif

#ifndef	PROJECTFNAME
#define	PROJECTFNAME	".project"
#endif

#ifndef	PCSDPIFNAME /* PCS Default-Project-Info file */
#define	PCSDPIFNAME	"etc/projectinfo"
#endif

#define	SUBINFO		struct subinfo
#define	SUBINFO_FL	struct subinfo_flags


/* external subroutines */

extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	sncpy3(char *,int,const char *,const char *,const char *) ;
extern int	snwcpyhyphen(char *,int,const char *,int) ;
extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mknpath2(char *,int,const char *,const char *) ;
extern int	matkeystr(const char **,char *,int) ;
extern int	vstrkeycmp(const char **,const char **) ;
extern int	sfshrink(const char *,int,const char **) ;
extern int	nextfield(const char *,int,const char **) ;
extern int	getuserhome(char *,int,cchar *) ;
extern int	getgecosname(const char *,int,const char **) ;
extern int	mkgecosname(char *,int,const char *) ;
extern int	mkrealname(char *,int,const char *,int) ;
extern int	readfileline(char *,int,cchar *) ;
extern int	cfdecui(const char *,int,uint *) ;
extern int	hasalldig(const char *,int) ;
extern int	isdigitlatin(int) ;
extern int	isNotPresent(int) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;


/* external variables */


/* local structures */

struct subinfo_flags {
	uint		uid:1 ;
	uint		pw:1 ;
} ;

struct subinfo {
	const char	*pr ;
	const char	*varusername ;
	const char	*un ;
	char		*rbuf ;		/* user supplied buffer */
	SUBINFO_FL	init, f ;
	struct passwd	pw ;
	uid_t		uid ;
	int		rlen ;
	int		pwlen ;
	char		*pwbuf ;
} ;

struct pcsnametype {
	const char	*var ;
	const char	*fname ;
} ;


/* forward references */

int		pcsgetnames(const char *,char *,int,const char *,int) ;

static int	pcsprojectinfo(cchar *,char *,int,cchar *) ;

static int	subinfo_start(SUBINFO *,cchar *,char *,int,cchar *) ;
static int	subinfo_getuid(SUBINFO *,uid_t *) ;
static int	subinfo_getpw(SUBINFO *) ;
static int	subinfo_finish(SUBINFO *) ;

static int	getname(SUBINFO *,int) ;
static int	getname_var(SUBINFO *,int) ;
static int	getname_userhome(SUBINFO *,int) ;
static int	getname_again(SUBINFO *,int) ;
static int	getname_sysdb(SUBINFO *,int) ;

static int	getprojinfo_userhome(SUBINFO *) ;
static int	getprojinfo_sysdb(SUBINFO *) ;
static int	getprojinfo_pcsdef(SUBINFO *) ;


/* local variables */

static int	(*getprojinfos[])(SUBINFO *) = {
	getprojinfo_userhome,
	getprojinfo_sysdb,
	getprojinfo_pcsdef,
	NULL
} ;

enum pcsnametypes {
	pcsnametype_name,
	pcsnametype_fullname,
	pcsnametype_overlast
} ;

static const struct pcsnametype	pcsnametypes[] = {
	{ VARNAME, NAMEFNAME },
	{ VARFULLNAME, FULLNAMEFNAME },
	{ NULL, NULL }
} ;

static int	(*getnames[])(SUBINFO *,int) = {
	getname_var,
	getname_userhome,
	getname_again,
	getname_sysdb,
	NULL
} ;


/* exported subroutines */


int pcsgetname(cchar *pr,char *rbuf,int rlen,cchar *username)
{
	const int	nt = pcsnametype_name ;


#if	CF_DEBUGS
	debugprintf("pcsname: un=%s\n",username) ;
#endif

	return pcsgetnames(pr,rbuf,rlen,username,nt) ;
}
/* end subroutine (pcsgetname) */


int pcsgetfullname(pr,rbuf,rlen,username)
const char	pr[] ;
char		rbuf[] ;
int		rlen ;
const char	username[] ;
{
	const int	nt = pcsnametype_fullname ;


	return pcsgetnames(pr,rbuf,rlen,username,nt) ;
}
/* end subroutine (pcsgetfullname) */


int pcsname(pr,rbuf,rlen,username)
const char	pr[] ;
char		rbuf[] ;
int		rlen ;
const char	username[] ;
{
	const int	nt = pcsnametype_name ;


#if	CF_DEBUGS
	debugprintf("pcsname: un=%s\n",username) ;
#endif

	return pcsgetnames(pr,rbuf,rlen,username,nt) ;
}
/* end subroutine (pcsname) */


int pcsfullname(cchar *pr,char *rbuf,int rlen,cchar *username)
{
	const int	nt = pcsnametype_fullname ;

	return pcsgetnames(pr,rbuf,rlen,username,nt) ;
}
/* end subroutine (pcsfullname) */


int pcsnames(pr,rbuf,rlen,un,nt)
const char	pr[] ;
char		rbuf[] ;
int		rlen ;
const char	un[] ;
int		nt ;
{
	return pcsgetnames(pr,rbuf,rlen,un,nt) ;
}
/* end subroutine (pcsnames) */


int pcsgetnames(pr,rbuf,rlen,un,nt)
const char	pr[] ;
char		rbuf[] ;
int		rlen ;
const char	un[] ;
int		nt ;
{
	SUBINFO	si ;
	int		rs ;

#if	CF_DEBUGS
	debugprintf("pcsgetnames: pr=%s\n",pr) ;
	debugprintf("pcsgetnames: un=%s\n",un) ;
	debugprintf("pcsgetnames: nt=%u\n",nt) ;
#endif

#if	CF_DEFPCS
	if (pr == NULL)
	    pr = getenv(VARPRPCS) ;
#endif

	if (pr == NULL) return SR_FAULT ;
	if (rbuf == NULL) return SR_FAULT ;
	if (un == NULL) return SR_FAULT ;

	if (un[0] == '\0') return SR_INVALID ;

	if (nt >= pcsnametype_overlast) return SR_INVALID ;

#if	CF_DEBUGS
	debugprintf("pcsgetnames: adjusted nt=%u\n",nt) ;
#endif

	rbuf[0] = '\0' ;
	if ((rs = subinfo_start(&si,pr,rbuf,rlen,un)) >= 0) {

#if	CF_DEBUGS
	    debugprintf("pcsgetnames: getname() nt=%u\n",nt) ;
#endif

	    rs = getname(&si,nt) ;

#if	CF_DEBUGS
	    debugprintf("pcsgetnames: getname() rs=%d\n",rs) ;
#endif

	    subinfo_finish(&si) ;
	} /* end if */

#if	CF_DEBUGS
	debugprintf("pcsgetnames: ret rs=%d\n",rs) ;
	if (rs >= 0)
	    debugprintf("pcsgetnames: name=>%t<\n",rbuf,rs) ;
#endif

	return rs ;
}
/* end subroutine (pcsgetnames) */


int pcsgetprojinfo(pr,rbuf,rlen,username)
const char	pr[] ;
char		rbuf[] ;
int		rlen ;
const char	username[] ;
{


	return pcsprojectinfo(pr,rbuf,rlen,username) ;
}
/* end subroutine (pcsgetprojinfo) */


int pcsprojectinfo(pr,rbuf,rlen,username)
const char	pr[] ;
char		rbuf[] ;
int		rlen ;
const char	username[] ;
{
	SUBINFO	mi ;
	int		rs ;

	if (pr == NULL)
	    pr = getenv(VARPRPCS) ;

	if (pr == NULL) return SR_FAULT ;
	if (rbuf == NULL) return SR_FAULT ;
	if (username == NULL) return SR_FAULT ;

	if (username[0] == '\0') return SR_INVALID ;

	rbuf[0] = '\0' ;
	if ((rs = subinfo_start(&mi,pr,rbuf,rlen,username)) >= 0) {
	    int	i ;

	    for (i = 0 ; getprojinfos[i] != NULL ; i += 1) {
#if	CF_DEBUGS
	debugprintf("pcsprojectinfo: i=%d\n",i) ;
#endif
	        rs = (*getprojinfos[i])(&mi) ;
	        if (rs != 0) break ;
	    } /* end for */

	    subinfo_finish(&mi) ;
	} /* end if */

#if	CF_DEBUGS
	debugprintf("pcsprojectinfo: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (pcsprojectinfo) */


/* local subroutines */


static int subinfo_start(SUBINFO *sip,cchar *pr,char rbuf[],int rlen,cchar *un)
{
	const int	pwlen = getbufsize(getbufsize_pw) ;
	int		rs ;
	char		*pwbuf ;

	memset(sip,0,sizeof(SUBINFO)) ;
	sip->pr = pr ;
	sip->rbuf = rbuf ;
	sip->rlen = rlen ;
	sip->un = un ;
	sip->varusername = VARUSERNAME ;

	if ((rs = uc_malloc((pwlen+1),&pwbuf)) >= 0) {
	    sip->pwbuf = pwbuf ;
	    sip->pwlen = pwlen ;
	}

	return rs ;
}
/* end subroutine (subinfo_start) */


static int subinfo_finish(SUBINFO *sip)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (sip->pwbuf != NULL) {
	    rs1 = uc_free(sip->pwbuf) ;
	    if (rs >= 0) rs = rs1 ;
	    sip->pwbuf = NULL ;
	}

	sip->pr = NULL ;
	return rs ;
}
/* end subroutine (subinfo_finish) */


static int subinfo_getuid(SUBINFO *sip,uid_t *uidp)
{
	int		rs = SR_OK ;

	if (! sip->init.uid) {
	    const char	*cp ;

	    sip->init.uid = TRUE ;
	    cp = getenv(sip->varusername) ;

	    if ((cp != NULL) && (strcmp(cp,sip->un) == 0)) {
	        sip->f.uid = TRUE ;
	        sip->uid = getuid() ;
	    } else {
	        rs = subinfo_getpw(sip) ;
	        if ((rs >= 0) && (! sip->f.uid)) {
	            sip->f.uid = TRUE ;
	            sip->uid = sip->pw.pw_uid ;
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


static int subinfo_getpw(SUBINFO *sip)
{
	int		rs = SR_OK ;
	const char	*un = sip->un ;

#if	CF_DEBUGS
	debugprintf("pcsnames/subinfo_getpw: init.pw=%u\n",sip->init.pw) ;
	debugprintf("pcsnames/subinfo_getpw: un=%s\n",un) ;
#endif

	if (! sip->init.pw) {
	    const int	pwlen = sip->pwlen ;
	    char	*pwbuf = sip->pwbuf ;
	    sip->init.pw = TRUE ;
	    if ((un != NULL) && (un[0] != '\0') && (un[0] != '-')) {
	        if (hasalldig(un,-1)) {
	            uint	uv ;
	            if ((rs = cfdecui(un,-1,&uv)) >= 0) {
	                const uid_t	uid = uv ;
	                rs = getpwusername(&sip->pw,pwbuf,pwlen,uid) ;
#if	CF_DEBUGS
	                debugprintf("pcsnames/subinfo_getpw: getpwusername() "
	                    "rs=%d username=%s\n",rs,sip->pw.pw_name) ;
#endif
	            }
	        } else {
	            rs = GETPW_NAME(&sip->pw,pwbuf,pwlen,un) ;
		}
	    } else {
	        rs = getpwusername(&sip->pw,pwbuf,pwlen,-1) ;
	    }
	    if (rs >= 0) {
	        sip->f.uid = TRUE ;
	        sip->uid = sip->pw.pw_uid ;
	    }
	} /* end if (was not already initialized) */

#if	CF_DEBUGS
	debugprintf("pcsnames/subinfo_getpw: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (subinfo_getpw) */


static int getname(SUBINFO *sip,int nt)
{
	int		rs = SR_OK ;
	int		i ;

	if (nt >= pcsnametype_overlast)
	    return SR_NOANODE ;

	sip->rbuf[0] = '\0' ;
	for (i = 0 ; getnames[i] != NULL ; i += 1) {

#if	CF_DEBUGS
	    debugprintf("pcsname/getname: i=%u\n",i) ;
#endif

	    rs = (*getnames[i])(sip,nt) ;

#if	CF_DEBUGS
	    debugprintf("pcsname/getname: i=%u getnames() rs=%d\n",i,rs) ;
#endif

	    if (rs != 0) break ;
	} /* end for */

#if	CF_DEBUGS
	debugprintf("pcsname/getname: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (getname) */


static int getname_var(SUBINFO *sip,int nt)
{
	int		rs = SR_OK ;
	int		len = 0 ;
	int		f ;
	const char	*un = sip->un ;

	f = (un[0] == '-') ;
	if (! f) {
	    const char	*vun = getenv(VARUSERNAME) ;
	    if ((vun != NULL) && (vun[0] != '\0'))
	        f = (strcmp(vun,un) == 0) ;
	}
	if (f) {
	    const char	*cp = getenv(pcsnametypes[nt].var) ;
	    if ((cp != NULL) && (cp[0] != '\0')) {
	        rs = sncpy1(sip->rbuf,sip->rlen,cp) ;
		len = rs ;
	    }
	} /* end if */

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (getname_var) */


static int getname_userhome(SUBINFO *sip ,int nt)
{
	const int	hlen = MAXPATHLEN ;
	int		rs ;
	cchar		*un = sip->un ;
	char		hbuf[MAXPATHLEN + 1] ;

#if	CF_DEBUGS
	debugprintf("pcsgetnames/getname_userhome: ent\n") ;
#endif

	if ((rs = getuserhome(hbuf,hlen,un)) >= 0) {
	    const char	*fn = pcsnametypes[nt].fname ;
	    char	tbuf[MAXPATHLEN + 1] ;
	    if ((rs = mkpath2(tbuf,hbuf,fn)) >= 0) {
	        rs = readfileline(sip->rbuf,sip->rlen,tbuf) ;
		if (isNotPresent(rs)) rs = SR_OK ;
	    }
	} /* end if (getuserhome) */

#if	CF_DEBUGS
	debugprintf("pcsgetnames/getname_userhome: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (getname_userhome) */


static int getname_again(SUBINFO *sip,int nt)
{
	int		rs = SR_OK ;

	if (nt == pcsnametype_fullname) {
	    nt = pcsnametype_name ;
	    rs = getname(sip,nt) ;
	}

	return rs ;
}
/* end subroutine (getname_again) */


static int getname_sysdb(SUBINFO *sip,int nt)
{
	int		rs = SR_OK ;
	int		len = 0 ;

#if	CF_DEBUGS
	debugprintf("pcsnames/getname_sysdb: un=%s nt=%u\n",sip->un,nt) ;
#endif

	if (! sip->init.pw) {
	    rs = subinfo_getpw(sip) ;

#if	CF_DEBUGS
	    debugprintf("pcsnames/getname_sysdb: subinfo_getpw() rs=%d "
	        "username=%s\n",rs,sip->pw.pw_name) ;
#endif

	}

#ifdef	COMMENT /* we have two implementations */
	if (rs >= 0) {
	    const int	nlen = (strlen(sip->pw.pw_gecos)+10) ;
	    const char	*gecos = sip->pw.pw_gecos ;
	    char	*nbuf ;
	    if ((rs = uc_malloc((nlen+1),&nbuf)) >= 0) {
	        if ((rs = mkgecosname(nbuf,nlen,gecos)) > 0) {
	            rs = mkrealname(sip->rbuf,sip->rlen,nbuf,rs) ;
	            len = rs ;
	        }
	        uc_free(nbuf) ;
	    } /* end if (memory-allocation) */
	} /* end if */
#else /* COMMENT */
	if (rs >= 0) {
	    int		gl ;
	    const char	*gecos = sip->pw.pw_gecos ;
	    const char	*gp ;
	    if ((gl = getgecosname(gecos,-1,&gp)) > 0) {
	        const int	nlen = (gl+10) ;
	        char		*nbuf ;
	        if ((rs = uc_malloc((nlen+1),&nbuf)) >= 0) {
	            if ((rs = snwcpyhyphen(nbuf,nlen,gp,gl)) > 0) {
	                rs = mkrealname(sip->rbuf,sip->rlen,nbuf,rs) ;
#if	CF_DEBUGS
	debugprintf("pcsnames/getname_sysdb: mkrealname() rs=%d\n",rs) ;
#endif
	                len = rs ;
	            }
	            uc_free(nbuf) ;
	        } /* end if (memory-allocation) */
	    } /* end if (getgecosname) */
	} /* end if */
#endif /* COMMENT */

#if	CF_DEBUGS
	debugprintf("pcsnames/getname_sysdb: rn=>%t<\n",sip->rbuf,sip->rlen) ;
	debugprintf("pcsnames/getname_sysdb: ret rs=%d len=%u\n",rs,len) ;
#endif

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (getname_sysdb) */


static int getprojinfo_userhome(SUBINFO *sip)
{
	const int	hlen = MAXPATHLEN ;
	int		rs ;
	int		len = 0 ;
	cchar		*un = sip->un ;
	const char	*fname = PROJECTFNAME ;
	char		hbuf[MAXPATHLEN + 1] ;

	hbuf[0] = '\0' ;
	if ((rs = getuserhome(hbuf,hlen,un)) >= 0) {
	    char	tbuf[MAXPATHLEN + 1] ;
	    if ((rs = mkpath2(tbuf,hbuf,fname)) >= 0) {
		const int	rlen = sip->rlen ;
		char		*rbuf = sip->rbuf ;
	        if ((rs = readfileline(rbuf,rlen,tbuf)) >= 0) {
		    len = rs ;
		} else if (isNotPresent(rs)) 
		    rs = SR_OK ;
	    }
	} /* end if (gethome) */

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (getprojinfo_userhome) */


static int getprojinfo_sysdb(SUBINFO *sip)
{
	struct project	pj ;
	const int	pjlen = getbufsize(getbufsize_pj) ;
	int		rs ;
	int		len = 0 ;
	char		*pjbuf ;

#if	CF_DEBUGS
	debugprintf("pcsgetnames/getprojinfo_sysdb: un=%d\n",sip->un) ;
#endif

	if ((rs = uc_malloc((pjlen+1),&pjbuf)) >= 0) {
	    if ((rs = uc_getdefaultproj(sip->un,&pj,pjbuf,pjlen)) >= 0) {
	        int	f = (strcmp(pj.pj_name,DEFPROJNAME) != 0) ;
	        if (f) {
	            uid_t	uid ;
	            if ((rs = subinfo_getuid(sip,&uid)) >= 0) {
		        f = (uid >= NSYSPIDS) ;
		    }
	        }
	        if ((rs >= 0) && f) {
	            rs = sncpy1(sip->rbuf,sip->rlen,pj.pj_comment) ;
		    len = rs ;
	        }
	    } else if (isNotPresent(rs)) {
	        rs = SR_OK ;
	    }
	    uc_free(pjbuf) ;
	} /* end if (memory-allocation) */

#if	CF_DEBUGS
	debugprintf("pcsgetnames/getprojinfo_sysdb: rs=%d len=%u\n",rs,len) ;
#endif

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (getprojinfo_sysdb) */


static int getprojinfo_pcsdef(SUBINFO *sip)
{
	uid_t		uid ;
	int		rs = SR_OK ;
	int		len = 0 ;

	if ((rs = subinfo_getuid(sip,&uid)) >= 0) {
	    if (uid >= NSYSPIDS) {
	        cchar	*fname = PCSDPIFNAME ;
	        char	tbuf[MAXPATHLEN + 1] ;
	        if ((rs = mkpath2(tbuf,sip->pr,fname)) >= 0) {
		    const int	rlen = sip->rlen ;
		    char	*rbuf = sip->rbuf ;
	            if ((rs = readfileline(rbuf,rlen,tbuf)) >= 0) {
	                len = rs ;
		    } else if (isNotPresent(rs)) 
			rs = SR_OK ;
	        }
	    } /* end if (system UID) */
	} /* end if */

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (getprojinfo_pcsdef) */


