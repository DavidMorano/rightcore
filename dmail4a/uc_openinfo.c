/* uc_openinfo */

/* interface component for UNIX® library-3c */
/* higher-level "open" /w timeout */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */
#define	CF_CDPATH	1		/* allow cd-paths */


/* revision history:

	= 1998-04-01, David A­D­ Morano

	This subroutine was originally written.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Filename formats:

	UNIX® domain sockets have the format:
		filepath

	where:
		filepath

	is just a regular UNIX® file path to the socket file.

	File-systems that are supported internally (no external shared-memory
	object needed) are:

	proto
	prog
	pass
	shm
	u
	sys

	Protocols (when using the 'proto' filesystem above) have the format:

		/proto/<protoname>/<af>/<host>/<service>

	where:
		proto		constant name 'proto'
		<protoname>	protocol name
					tcp
					tcpmux[:port]
					tcpnls[:port]
					udp
					uss
					ussmux[:svc]
					usd
		<af>		address family
					inet
					inet4
					inet6
		<host>		hostname of remote host to contact
		<service>	service to contact


	Examples:

	/something/unix/domain/socket

	/proto/tcp/inet/rca/daytime
	/proto/udp/inet/rca/daytime
	/proto/udp/inet6/rca/daytime
	/proto/uss/unix/path
	/proto/usd/unix/path

	/proto/inet/tcp/rca/daytime
	/proto/inet/udp/rca/daytime
	/proto/inet6/udp/rca/daytime


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/stat.h>
#include	<sys/socket.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<poll.h>
#include	<stdlib.h>
#include	<string.h>
#include	<pwd.h>
#include	<netdb.h>

#include	<vsystem.h>
#include	<vecstr.h>
#include	<getax.h>
#include	<ugetpw.h>
#include	<localmisc.h>


/* local defines */

#define	UCOPENINFO	struct ucopeninfo
#define	OURSTAT		struct ustat
#define	FUNCSTAT	u_stat
#define	FUNCFSTAT	u_fstat
#define	FUNCLSTAT	u_lstat
#define	FUNCOPEN	u_open

#ifndef	MAXSYMLINKS
#define	MAXSYMLINKS	20		/* defined by the OS */
#endif

#undef	FLBUFLEN
#define	FLBUFLEN	100
#define	PRNBUFLEN	MAXNAMELEN
#define	SVCBUFLEN	MAXNAMELEN

#define	NPOLLS		2
#define	POLLMULT	1000


/* external subroutines */

extern int	snopenflags(char *,int,int) ;
extern int	snwcpy(char *,int,const char *,int) ;
extern int	sncpy1w(char *,int,const char *,int) ;
extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath1w(char *,const char *,int) ;
extern int	sfdirname(const char *,int,const char **) ;
extern int	matstr(const char **,const char *,int) ;
extern int	haslc(const char *,int) ;
extern int	hascdpath(const char *,int) ;
extern int	hasvarpathprefix(const char *,int) ;
extern int	sichr(const char *,int,int) ;
extern int	getnodedomain(char *,char *) ;
extern int	getdomainname(char *,int) ;
extern int	getuserhome(char *,int,const char *) ;
extern int	mkpr(char *,int,const char *,const char *) ;
extern int	mkuserpath(char *,const char *,const char *,int) ;
extern int	mkvarpath(char *,const char *,int) ;
extern int	mkcdpath(char *,const char *,int) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strwcpylc(char *,const char *,int) ;
extern char	*strwcpyuc(char *,const char *,int) ;
extern char	*strnpbrk(const char *,int,const char *) ;
extern char	*strdcpy1(char *,int,const char *) ;


/* external variables */

extern const char	**environ ;


/* local structures */


/* forward references */

static int	u_openex(struct ucopeninfo *) ;

static int	accmode(int) ;
static int	waitready(int,int,int) ;
static int	getprefixfs(const char *,const char **) ;
static int	getnormalfs(const char *,const char **) ;
static int	noexist(const char *,int) ;
static int	loadargs(vecstr *,const char *) ;
static int	hasnonpath(const char *,int) ;

static int	open_prog(const char *,int,const char **) ;
static int	open_nonpath(struct ucopeninfo *,int) ;


/* local variables */

static const char	*normalfs[] = {
	"devices",
	"proc",
	"var",
	"kernel",
	"platform",
	"lib",
	"xfn",
	"bin",
	NULL
} ;

static const char	*prefixfs[] = {
	"proto",
	"prog",
	"pass",
	"shm",
	"u",
	"sys",
	"dev",
	NULL
} ;

enum prefixfses {
	prefixfs_proto,
	prefixfs_prog,
	prefixfs_pass,
	prefixfs_shm,
	prefixfs_user,
	prefixfs_sys,
	prefixfs_dev,
	prefixfs_overlast
} ;

enum accmodes {
	accmode_rdonly,
	accmode_wronly,
	accmode_rdwr,
	accmode_overlast
} ;

static const char	*nonpaths = "/¥§~" ;

enum nonpaths {
	nonpath_not,
	nonpath_dialer,
	nonpath_fsvc,
	nonpath_usvc,
	nonpath_overlast
} ;


/* exported subroutines */


int uc_openex(fname,oflags,operms,timeout,opts)
const char	fname[] ;
int		oflags ;
mode_t		operms ;
int		timeout ;
int		opts ;
{
	UCOPENINFO	oi ;
	int		rs ;

	memset(&oi,0,sizeof(struct ucopeninfo)) ;
	oi.fname = fname ;
	oi.oflags = oflags ;
	oi.operms = operms ;
	oi.to = timeout ;
	oi.opts = opts ;

	rs = uc_openinfo(&oi) ;

	return rs ;
}
/* end subroutine (uc_openex) */


int uc_openinfo(struct ucopeninfo *oip)
{
	int		rs = SR_OK ;
	int		pi ;
	int		size ;
	int		c_links ;
	int		npi ;			/* non-path index */
	const char	*rp = NULL ;
	char		ofname[MAXPATHLEN + 1] ;
	char		*efname = NULL ;

#if	CF_DEBUGS
	debugprintf("uc_openinfo: ent\n") ;
#endif

	if (oip == NULL) return SR_FAULT ;
	if (oip->fname == NULL) return SR_FAULT ;

	if (oip->fname[0] == '\0') return SR_INVALID ;

#if	CF_DEBUGS
	{
	    char	flbuf[FLBUFLEN+1] ;
	    debugprintf("uc_openinfo: fname=%s\n",oip->fname) ;
	    snopenflags(flbuf,FLBUFLEN,oip->oflags) ;
	    debugprintf("uc_openinfo: oflags=%s\n",flbuf) ;
	    debugprintf("uc_openinfo: to=%d\n",oip->to) ;
	    debugprintf("uc_openinfo: varpath=%u\n",
	        hasvarpathprefix(oip->fname,-1)) ;
	}
#endif /* CF_DEBUGS */

	c_links = oip->clinks ;
	rs = accmode(oip->oflags) ;
#ifdef	COMMENT
	am = rs ;
#endif

#if	CF_DEBUGS
	debugprintf("uc_openinfo: accmode() rs=%d\n",rs) ;
#endif

	if (rs < 0)
	    goto ret0 ;

#if	CF_CDPATH
	if (hascdpath(oip->fname,-1)) {
	    char	*cdpath ;
	    size = (MAXPATHLEN + 1) ;
	    if ((rs = uc_libmalloc(size,&cdpath)) >= 0) {
	        if ((rs = mkcdpath(cdpath,oip->fname,-1)) > 0) {
	            oip->fname = cdpath ;
	            oip->clinks += 1 ;
	            rs = uc_openinfo(oip) ;
	        } else if (rs == 0) {
	            rs = SR_BADFMT ;
		}
	        uc_libfree(cdpath) ;
	    } /* end if (memory-allocation) */
	    goto ret0 ;
	} /* end if (cd-path) */
#endif /* CF_CDPATH */

/* top of loop trying to open the file */
top:
	if (rs >= 0) {
	    npi = hasnonpath(oip->fname,-1) ;
	    if (npi > 0) goto nonpath ;
	}

/* perform any initial expansion as needed (including memory allocation) */

	if ((rs >= 0) && hasvarpathprefix(oip->fname,-1) && (efname == NULL)) {
	    void	*p ;
	    size = (MAXPATHLEN + 1) ;
	    if ((rs = uc_libmalloc(size,&p)) >= 0) {
	        efname = p ;
	        efname[0] = '\0' ;
	        rs = mkvarpath(efname,oip->fname,-1) ;
	        if (rs > 0) {
	            oip->fname = efname ;
	        } else if (rs <= 0) {
	            if (rs == 0) rs = SR_BADFMT ;
	            uc_libfree(efname) ;
	            efname = NULL ;
	        }
	    }
	} /* end if (var-path) */

	if (rs < 0)
	    goto ret1 ;

	if ((oip->fname[0] == '/') && 
	    ((pi = getprefixfs(oip->fname,&rp)) >= 0)) {

#if	CF_DEBUGS
	    debugprintf("uc_openinfo: pathprefix fs=%d\n",pi) ;
#endif

	    switch (pi) {

	    case prefixfs_proto:
	        rs = uc_openproto(rp,oip->oflags,oip->to,oip->opts) ;
	        break ;

	    case prefixfs_prog:
	        rs = open_prog(rp,oip->oflags,oip->envv) ;
	        break ;

	    case prefixfs_pass:
	        rs = uc_openpass(rp,oip->oflags,oip->to,0) ;
	        break ;

	    case prefixfs_shm:
	        rs = uc_openshm(rp,oip->oflags,oip->operms) ;
	        break ;

	    case prefixfs_user:
	        oip->fname = rp ;
	        oip->clinks = c_links ;
	        rs = uc_openuserinfo(oip) ;
	        break ;

	    case prefixfs_sys:
	    case prefixfs_dev:
	        {
	            int		of = oip->oflags ;
	            mode_t	om = oip->operms ;
	            const char	**envv = oip->envv ;
	            int		to = oip->to ;
	            int		opts = oip->opts ;
	            switch (pi) {
	            case prefixfs_sys:
	                rs = uc_opensys(rp,of,om,envv,to,opts) ;
	                break ;
	            case prefixfs_dev:
	                rs = uc_opendev(rp,of,om,envv,to,opts) ;
	                break ;
	            } /* end switch */
	        } /* end block */
	        break ;

	    default:
	        rs = SR_NOSYS ;
	        break ;

	    } /* end switch */

	} else if ((oip->fname[0] == '/') && 
	    ((pi = getnormalfs(oip->fname,&rp)) >= 0)) {

#if	CF_DEBUGS
	    debugprintf("uc_openinfo: normalfs pi=%u\n",pi) ;
#endif
	    rs = u_openex(oip) ;

	} else {
	    OURSTAT	sb ;
	    int		f = (oip->oflags & O_CREAT) ;

#if	CF_DEBUGS
	    debugprintf("uc_openinfo: defaultfs f_create=%u\n",f) ;
#endif

	    if (! f) {
	        rs = u_openex(oip) ;
#if	CF_DEBUGS
	        debugprintf("uc_openinfo: u_openex() rs=%d\n",rs) ;
#endif
	    } else {
#if	CF_DEBUGS
	        debugprintf("uc_openinfo: create -> NOENT\n") ;
#endif
	        rs = SR_NOENT ;
	    }

	    if (rs == SR_OPNOTSUPP) {
	        int	rs1 ;

#if	CF_DEBUGS
	        debugprintf("uc_openinfo: OPNOTSUPP\n") ;
#endif

	        rs1 = FUNCSTAT(oip->fname,&sb) ;

	        if ((rs1 >= 0) && S_ISSOCK(sb.st_mode)) {

	            rs = uc_opensocket(oip->fname,oip->oflags,oip->to) ;

#if	CF_DEBUGS
	            debugprintf("uc_openinfo: opensocket() rs=%d\n",rs) ;
#endif

	        }

	    } else if (rs == SR_NOENT) {

#if	CF_DEBUGS
	        debugprintf("uc_openinfo: NOENT fname=%s\n",oip->fname) ;
#endif

	        rs = FUNCLSTAT(oip->fname,&sb) ;

#if	CF_DEBUGS
	        debugprintf("uc_openinfo: u_lstat() rs=%d\n",rs) ;
#endif

	        if ((rs >= 0) && S_ISLNK(sb.st_mode)) {
		    const int	rlen = MAXPATHLEN ;
	            char	rbuf[MAXPATHLEN + 1] ;

#if	CF_DEBUGS
	            debugprintf("uc_openinfo: S_LINK\n") ;
#endif

	            if ((rs = u_readlink(oip->fname,rbuf,rlen)) >= 0) {
	                rbuf[rs] = '\0' ;

#if	CF_DEBUGS
	                debugprintf("uc_openinfo: link=%s\n",rbuf) ;
#endif

	                if (rbuf[0] == '/') {

	                    oip->fname = (const char *) ofname ;
	                    rs = mkpath1(ofname,rbuf) ;

	                } else if ((npi = hasnonpath(rbuf,-1)) > 0) {

	                    oip->fname = (const char *) ofname ;
	                    rs = mkpath1(ofname,rbuf) ;
	                    if (rs >= 0)
	                        goto nonpath ;

	                } else {
	                    int		cl ;
	                    const char	*cp ;
	                    char	dname[MAXPATHLEN + 1] ;

	                    if ((cl = sfdirname(oip->fname,-1,&cp)) > 0) {

	                        oip->fname = (const char *) ofname ;
	                        rs = snwcpy(dname,MAXPATHLEN,cp,cl) ;

#if	CF_DEBUGS
	                        debugprintf("uc_openinfo: rs=%d dname=%s\n",
	                            rs,dname) ;
#endif

	                        if (rs >= 0)
	                            rs = mkpath2(ofname,dname,rbuf) ;

#if	CF_DEBUGS
	                        debugprintf("uc_openinfo: rs=%d new fname=%s\n",
	                            rs,oip->fname) ;
#endif

	                    } else {

	                        oip->fname = (const char *) ofname ;
	                        rs = mkpath1(ofname,rbuf) ;

	                    } /* end if */

	                } /* end if */

	                c_links += 1 ;
	                if (rs >= 0) {
	                    if (c_links <= MAXSYMLINKS) goto top ;
	                    rs = SR_MLINK ;
	                }

	            } /* end if (reading symbolic link) */

	        } else {

	            if (oip->oflags & O_CREAT)
	                rs = u_openex(oip) ;

	        } /* end if (was a symbolic link) */

	    } /* end if (which type of error) */

	} /* end if (a protocol or not) */

ret1:
	if (efname != NULL)
	    uc_libfree(efname) ;

ret0:

#if	CF_DEBUGS
	debugprintf("uc_openinfo: ret rs=%d\n",rs) ;
	debugprintf("uc_openinfo: ret fname=%s\n",oip->fname) ;
#endif

	return rs ;

/* handle non-path filenames */
nonpath:

#if	CF_DEBUGS
	debugprintf("uc_openinfo: non-path npi=%u\n",npi) ;
#endif

	switch (npi) {
	case nonpath_dialer:
	case nonpath_fsvc:
	case nonpath_usvc:
	    rs = open_nonpath(oip,npi) ;
	    break ;
	default:
	    rs = SR_NOENT ;
	    break ;
	} /* end if */

#if	CF_DEBUGS
	debugprintf("uc_openinfo: non-path npi=%u rs=%d\n",npi,rs) ;
#endif

	goto ret1 ;
}
/* end subroutine (uc_openinfo) */


/* local subroutines */


static int u_openex(UCOPENINFO *oip)
{
	int		rs = SR_OK ;
	int		oflags = oip->oflags ;
	int		fd = -1 ;

#if	CF_DEBUGS
	debugprintf("u_openex: fname=%s to=%d\n",oip->fname,oip->to) ;
#endif

	if ((oflags & O_NDELAY) || (oip->to < 0)) {

	    if (oip->opts & FM_LARGEFILE) {
	        rs = FUNCOPEN(oip->fname,oflags,oip->operms) ;
	        fd = rs ;
	    } else {
	        rs = u_open(oip->fname,oflags,oip->operms) ;
	        fd = rs ;
	    }

#if	CF_DEBUGS
	    debugprintf("u_openex: 1 u_open() rs=%d\n",rs) ;
#endif

	} else {
	    mode_t	fm ;

	    oflags |= O_NDELAY ;

	    if (oip->opts & FM_LARGEFILE) {
	        rs = FUNCOPEN(oip->fname,oflags,oip->operms) ;
	        fd = rs ;
	    } else {
	        rs = u_open(oip->fname,oflags,oip->operms) ;
	        fd = rs ;
	    }

#if	CF_DEBUGS
	    debugprintf("u_openex: 2 u_open() rs=%d\n",rs) ;
#endif

	    if (rs < 0)
	        goto ret0 ;

	    if (oip->opts & FM_LARGEFILE) {
	        OURSTAT	sb ;
	        rs = FUNCFSTAT(fd,&sb) ;
	        fm = sb.st_mode ;
	    } else {
	        struct ustat	sb ;
	        rs = u_fstat(fd,&sb) ;
	        fm = sb.st_mode ;
	    }

	    if (rs < 0)
	        goto bad1 ;

	    if (S_ISFIFO(fm) &&
	        ((oflags & O_RDONLY) || (oflags & O_WRONLY))) {

#if	CF_DEBUGS
	        debugprintf("u_openex: waitready() to=%d\n",oip->to) ;
#endif

	        rs = waitready(fd,oflags,oip->to) ;

#if	CF_DEBUGS
	        debugprintf("u_openex: waitready() rs=%d\n",rs) ;
#endif

	    } /* end if */

	    if (rs >= 0) {
	        oflags &= (~ O_NDELAY) ;
	        rs = u_fcntl(fd,F_SETFL,oflags) ;

	    } else
	        u_close(fd) ;

	} /* end if */

ret0:

#if	CF_DEBUGS
	debugprintf("u_openex: ret rs=%d fd=%u\n",rs,fd) ;
#endif

	return (rs >= 0) ? fd : rs ;

/* bad stuff */
bad1:
	if (fd >= 0) {
	    u_close(fd) ;
	    fd = -1 ;
	}

	goto ret0 ;
}
/* end subroutine (u_openex) */


static int accmode(int oflags)
{
	int		rs = SR_INVALID ;
	int		am = (oflags & O_ACCMODE) ;

	switch (am) {
	case O_RDONLY:
	    rs = accmode_rdonly ;
	    break ;
	case O_WRONLY:
	    rs = accmode_wronly ;
	    break ;
	case O_RDWR:
	    rs = accmode_rdwr ;
	    break ;
	default:
	    rs = SR_INVALID ;
	    break ;
	} /* end switch */

	return rs ;
}
/* end subroutine (accmode) */


static int waitready(fd,oflags,timeout)
int		fd ;
int		oflags ;
int		timeout ;
{
	struct pollfd	polls[NPOLLS] ;
	time_t		ti_timeout ;
	time_t		daytime = time(NULL) ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		size ;
	int		pollto ;
	int		f_wait ;
	int		f_rdonly ;
	int		f = FALSE ;

	if (timeout < 0)
	    goto ret0 ;

	f_rdonly = (oflags & O_RDONLY) ;
	f_wait = f_rdonly || (oflags & O_WRONLY) ;
	if (! f_wait)
	    goto ret0 ;

	size = NPOLLS * sizeof(struct pollfd) ;
	memset(polls,0,size) ;

	polls[0].fd = fd ;
	polls[0].events = (f_rdonly) ? POLLIN : POLLOUT ;
	polls[1].fd = -1 ;

	ti_timeout = daytime + timeout ;
	while (rs >= 0) {

	    pollto = MIN(timeout,POLLMULT) ;
	    if ((rs1 = u_poll(polls,1,pollto)) > 0) {
	        int	re = polls[0].revents ;

	        if (re & POLLHUP) {
	            rs = SR_HANGUP ;
	        } else if (re & POLLERR) {
	            rs = SR_POLLERR ;
	        } else if (re & POLLNVAL) {
	            rs = SR_NOTOPEN ;
	        }

	        if (rs >= 0) {
	            if (f_rdonly) {
	                f = (re & POLLIN) ? 1 : 0 ;
	            } else {
	                f = (re & POLLOUT) ? 1 : 0 ;
	            }
	        } /* end if */

	    } /* end if (poll had something) */

	    if ((rs >= 0) && (daytime >= ti_timeout)) {
	        daytime = time(NULL) ;
	        rs = SR_TIMEDOUT ;
	    }

	    if ((rs >= 0) && f)
	        break ;

	} /* end while */

ret0:
	return rs ;
}
/* end subroutine (waitready) */


static int getnormalfs(fname,rpp)
const char	fname[] ;
const char	**rpp ;
{
	int		rs = 0 ;
	int		pi = -1 ;
	const char	*tp, *pp ;

	*rpp = NULL ;
	if (fname[0] != '/') {
	    rs = SR_NOENT ;
	    goto ret0 ;
	}

	pp = (fname + 1) ;
	while (*pp && (pp[0] == '/'))
	    pp += 1 ;

	if ((tp = strchr(pp,'/')) == NULL) {
	    rs = SR_NOENT ;
	    goto ret0 ;
	}

	pi = matstr(normalfs,pp,(tp - pp)) ;

	*rpp = (pi >= 0) ? tp : NULL ;

ret0:
	return (rs >= 0) ? pi : rs ;
}
/* end subroutine (getnormalfs) */


/* get the prefix-FS index (if there is a prefix-FS) */
static int getprefixfs(fname,rpp)
const char	fname[] ;
const char	**rpp ;
{
	int		rs = 0 ;
	int		pi = -1 ;
	const char	*tp = NULL ;

	if (fname[0] == '/') {
	    const char	*pp = (fname + 1) ;
	    int		pl = -1 ;

	    while (*pp && (pp[0] == '/')) pp += 1 ;

	    if ((tp = strchr(pp,'/')) != NULL) {
	        pl = (tp-pp) ;
	    } else {
	        pl = strlen(pp) ;
	        tp = (pp+pl) ;
	    }

	    if ((pi = matstr(prefixfs,pp,pl)) >= 0) {
	        switch (pi) {
	        case prefixfs_proto:
	        case prefixfs_prog:
	        case prefixfs_pass:
	        case prefixfs_shm:
	        case prefixfs_user:
	        case prefixfs_sys:
	            rs = noexist(pp,pl) ;
	            break ;
	        } /* end switch */
	    } else
	        rs = SR_NOENT ;

	} else
	    rs = SR_NOENT ;

	if (rpp != NULL)
	    *rpp = (rs >= 0) ? tp : NULL ;

	return (rs >= 0) ? pi : rs ;
}
/* end subroutine (getprefixfs) */


static int noexist(pp,pl)
const char	*pp ;
int		pl ;
{
	OURSTAT		sb ;
	int		rs = SR_OK ;
	int		rs1 ;
	char		pfname[MAXNAMELEN + 1] ;

	pfname[0] = '/' ;
	if ((rs = snwcpy((pfname + 1),(MAXNAMELEN - 1),pp,pl)) >= 0) {
	    rs1 = FUNCSTAT(pfname,&sb) ;
	    if (rs1 >= 0)
	        rs = SR_EXIST ;
	}

	return rs ;
}
/* end subroutine (noexist) */


static int open_prog(fname,oflags,ev)
const char	fname[] ;
int		oflags ;
const char	*ev[] ;
{
	vecstr		args ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		fd = -1 ;
	const char	*fnp = fname ;
	const char	*pfp ;
	const char	*svcp ;
	const char	**av = NULL ;
	char		expfname[MAXPATHLEN+1] ;
	char		progfname[MAXPATHLEN + 1] ;

#if	CF_DEBUGS
	debugprintf("uc_openinfo/open_prog: fname=%s\n",fname) ;
#endif

	if (ev == NULL) ev = (const char **) environ ;

	rs = mkuserpath(expfname,NULL,fname,-1) ;
#if	CF_DEBUGS
	debugprintf("uc_openinfo/open_prog: mkuserpath() rs=%d\n",rs) ;
	debugprintf("uc_openinfo/open_prog: expfname=%s\n",expfname) ;
#endif
	if (rs < 0) goto ret0 ;
	if (rs > 0) fnp = expfname ;

	if ((rs = vecstr_start(&args,4,0)) >= 0) {

	    pfp = fnp ;
	    if ((svcp = strchr(fnp,0xAD)) != NULL) {

	        pfp = progfname ;
	        rs = mkpath1w(progfname,fnp,(svcp - fnp)) ;

	        if (rs >= 0)
	            rs = vecstr_add(&args,pfp,rs) ;

	        if (rs >= 0)
	            rs = loadargs(&args,(svcp+1)) ;

	    } else
	        rs = vecstr_add(&args,pfp,-1) ;

#if	CF_DEBUGS
	    debugprintf("uc_openinfo/open_prog: mid1 rs=%d\n",rs) ;
#endif

	    if (rs >= 0) {
	        if ((rs = vecstr_getvec(&args,&av)) >= 0) {

#if	CF_DEBUGS
	            debugprintf("uc_openinfo/open_prog: mid2 rs=%d\n",rs) ;
#endif

	            if ((pfp[0] == '/') && (pfp[1] == '%')) pfp += 1 ;
	            rs = uc_openprog(pfp,oflags,av,ev) ;
	            fd = rs ;
#if	CF_DEBUGS
	            debugprintf("uc_openinfo/open_prog: uc_openprog() rs=%d\n",rs) ;
#endif
	        }
	    } /* end if (ok) */

	    rs1 = vecstr_finish(&args) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (args) */
	if ((rs < 0) && (fd >= 0)) u_close(fd) ;

ret0:

#if	CF_DEBUGS
	debugprintf("uc_openinfo/open_prog: ret rs=%d fd=%u\n",rs,fd) ;
#endif

	return (rs >= 0) ? fd : rs ;
}
/* end subroutine (open_prog) */


static int open_nonpath(UCOPENINFO *oip,int npi)
{
	vecstr		args ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		nch ;
	int		sl = -1 ;
	int		cl = -1 ;
	int		fd = -1 ;
	const char	**av = NULL ;
	const char	**ev = oip->envv ;
	const char	*fname = oip->fname ;
	const char	*tp ;
	const char	*sp, *cp ;
	char		prn[PRNBUFLEN+1] ;
	char		svc[SVCBUFLEN+1] ;
	char		brkbuf[4] ;
	char		prbuf[MAXPATHLEN+1] ;

#if	CF_DEBUGS
	debugprintf("uc_openinfo/open_nonpath: fname=%s\n",fname) ;
	debugprintf("uc_openinfo/open_nonpath: npi=%u\n",npi) ;
#endif

	svc[0] = '\0' ;
	nch = (nonpaths[npi] & 0xff) ;
	if (ev == NULL) ev = (const char **) environ ;

	brkbuf[0] = nch ;
	brkbuf[1] = (char) 0xAD ;
	brkbuf[2] = '/' ;
	brkbuf[3] = 0 ;
	tp = strpbrk(fname,brkbuf) ;

#if	CF_DEBUGS
	debugprintf("uc_openinfo/open_nonpath: tp=%p\n",tp) ;
	if (tp != NULL)
	    debugprintf("uc_openinfo/open_nonpath: tch=%c\n",*tp) ;
#endif

	if ((tp == NULL) || ((tp[0] & 0xff) != nch)) {
	    rs = SR_NOANODE ;		/* bug-check exception */
	    goto ret0 ;
	}

	rs = sncpy1w(prn,PRNBUFLEN,fname,(tp - fname)) ;
	if (rs < 0) goto ret0 ;
	sp = (tp+1) ;

	if (sp[0] == '\0') {
	    rs = SR_PROTO ;		/* no SVC -> protocol error */
	    goto ret0 ;
	}

	if ((rs = vecstr_start(&args,4,0)) >= 0) {

	    brkbuf[0] = (char) 0xAD ;
	    brkbuf[1] = ':' ;
	    brkbuf[2] = 0 ;
	    tp = strpbrk(sp,brkbuf) ;

	    cp = sp ;
	    if (tp != NULL) {
	        int	ch = (*tp & 0xff) ;

#if	CF_DEBUGS
	        debugprintf("uc_openinfo/open_nonpath: s=%t\n",sp,sl) ;
#endif

	        if (ch == ':') {
	            sl = (tp-sp) ;
	            cp = (tp+1) ;
	            tp = strchr(cp,0xAD) ;
	        }

#if	CF_DEBUGS
	        debugprintf("uc_openinfo/open_nonpath: s=%t c=%t\n",
	            sp,sl,cp,cl) ;
#endif

	        if (tp != NULL) {

	            cl = (tp-cp) ;
	            if (sl < 0) sl = cl ; /* or » if (ch != ':') sl = cl « */
	            if ((rs = vecstr_add(&args,cp,cl)) >= 0) {
	                cp = (tp+1) ;
	                rs = loadargs(&args,cp) ;
	            }

	        } else
	            rs = vecstr_add(&args,cp,cl) ;

	    } else {
	        rs = vecstr_add(&args,sp,sl) ;
	    }

	    if (rs >= 0)
	        rs = vecstr_getvec(&args,&av) ;

	    if (rs >= 0)
	        rs = sncpy1w(svc,SVCBUFLEN,sp,sl) ;

#if	CF_DEBUGS
	    debugprintf("uc_openinfo/open_nonpath: mid1 rs=%d\n",rs) ;
#endif

	    if (rs >= 0) {
	        switch (npi) {
	        case nonpath_dialer:
	            break ;
	        case nonpath_fsvc:
	            {
	                char	dn[MAXHOSTNAMELEN+1] ;
	                if ((rs = getnodedomain(NULL,dn)) >= 0) {
	                    rs = mkpr(prbuf,MAXPATHLEN,prn,dn) ;
	                }
	            }
	            break ;
	        case nonpath_usvc:
	            {
	                rs = getuserhome(prbuf,MAXPATHLEN,prn) ;
	            }
	            break ;
	        } /* end switch */
	    } /* end if */

#if	CF_DEBUGS
	    debugprintf("uc_openinfo/open_nonpath: mid2 rs=%d\n",rs) ;
	    debugprintf("uc_openinfo/open_nonpath: mid2 npi=%u\n",npi) ;
#endif

	    if (rs >= 0) {
	        const mode_t	om = oip->operms ;
	        const int	of = oip->oflags ;
	        const int	to = oip->to ;
	        switch(npi) {
	        case nonpath_dialer:
	            rs = uc_opendialer(prn,svc,of,om,av,ev,to) ;
	            fd = rs ;
	            break ;
	        case nonpath_usvc:
	            rs = uc_openusvc(prbuf,prn,svc,of,om,av,ev,to) ;
	            fd = rs ;
	            break ;
	        case nonpath_fsvc:
	            rs = uc_openfsvc(prbuf,prn,svc,of,om,av,ev,to) ;
	            fd = rs ;
	            break ;
	        default:
	            rs = SR_NOENT ;
	            break ;
	        } /* end switch */
	    } /* end if */

#if	CF_DEBUGS
	    debugprintf("uc_openinfo/open_nonpath: mid3 rs=%d\n",rs) ;
#endif

	    rs1 = vecstr_finish(&args) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (vecstr-args) */
	if ((rs < 0) && (fd >= 0)) u_close(fd) ;

ret0:

#if	CF_DEBUGS
	debugprintf("uc_openinfo/open_nonpath: ret rs=%d fd=%u\n",rs,fd) ;
	debugprintf("uc_openinfo/open_nonpath: ret fname=%s\n",fname) ;
#endif

	return (rs >= 0) ? fd : rs ;
}
/* end subroutine (open_nonpath) */


static int loadargs(alp,sp)
vecstr		*alp ;
const char	*sp ;
{
	int		rs = SR_OK ;
	int		c = 0 ;
	const char	*tp ;

	while ((tp = strchr(sp,0xAD)) != NULL) {

#if	CF_DEBUGS
	    debugprintf("uc_open/loadargs: a%u=>%t<\n",c,sp,(tp-sp)) ;
#endif

	    c += 1 ;
	    rs = vecstr_add(alp,sp,(tp - sp)) ;
	    sp = (tp+1) ;

	    if (rs < 0) break ;
	} /* end while */

	if (rs >= 0) {

#if	CF_DEBUGS
	    debugprintf("uc_open/loadargs: a%u=>%s<\n",c,sp) ;
#endif

	    c += 1 ;
	    rs = vecstr_add(alp,sp,-1) ;
	}

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (loadargs) */


static int hasnonpath(fp,fl)
const char	*fp ;
int		fl ;
{
	int		f ;

	if (fl < 0) fl = strlen(fp) ;

	f = (fp[0] != '/') ;
	if (f) {
	    const char	*tp = strnpbrk(fp,fl,nonpaths) ;
	    f = FALSE ;
	    if ((tp != NULL) && ((tp-fp) > 0) && (tp[1] != '\0'))
	        f = sichr(nonpaths,-1,*tp) ;
	}

	return f ;
}
/* end subroutine (hasnonpath) */


