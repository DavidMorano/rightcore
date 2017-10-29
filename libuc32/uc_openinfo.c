/* uc_openinfo */

/* interface component for UNIX® library-3c */
/* higher-level "open" /w timeout */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */
#define	CF_ISMORE	0		/* compile in |isMorePossible()| */


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


	Notes: Whew! Is this a "smelly" code module?  I hope not. But am I in a
	good position to judge that?


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
extern int	snwcpy(char *,int,cchar *,int) ;
extern int	sncpy1w(char *,int,cchar *,int) ;
extern int	mkpath1(char *,cchar *) ;
extern int	mkpath2(char *,cchar *,cchar *) ;
extern int	mkpath1w(char *,cchar *,int) ;
extern int	sfdirname(cchar *,int,cchar **) ;
extern int	matstr(cchar **,cchar *,int) ;
extern int	haslc(cchar *,int) ;
extern int	hascdpath(cchar *,int) ;
extern int	hasvarpathprefix(cchar *,int) ;
extern int	sichr(cchar *,int,int) ;
extern int	getnodedomain(char *,char *) ;
extern int	getdomainname(char *,int) ;
extern int	getuserhome(char *,int,cchar *) ;
extern int	mkpr(char *,int,cchar *,cchar *) ;
extern int	mkuserpath(char *,cchar *,cchar *,int) ;
extern int	mkvarpath(char *,cchar *,int) ;
extern int	mkcdpath(char *,cchar *,int) ;
extern int	isOneOf(const int *,int) ;
extern int	isNotPresent(int) ;

#if	CF_DEBUGS
extern int	debugprintf(cchar *,...) ;
extern int	strlinelen(cchar *,int,int) ;
#endif

extern char	*strwcpy(char *,cchar *,int) ;
extern char	*strwcpylc(char *,cchar *,int) ;
extern char	*strwcpyuc(char *,cchar *,int) ;
extern char	*strnpbrk(cchar *,int,cchar *) ;
extern char	*strdcpy1(char *,int,cchar *) ;


/* external variables */

extern cchar	**environ ;


/* local structures */


/* forward references */

static int	u_openex(UCOPENINFO *) ;

static int	open_eval(UCOPENINFO *) ;
static int	open_otherlink(UCOPENINFO *,int *,char *) ;
static int	open_othertry(UCOPENINFO *,int *,char *) ;
static int	open_floatpath(UCOPENINFO *,int) ;
static int	open_pseudopath(UCOPENINFO *,cchar *,int) ;
static int	open_nonpath(UCOPENINFO *,int) ;
static int	open_nonpather(UCOPENINFO *,int,cchar *,cchar *) ;

static int	openproger(cchar *,int,cchar **) ;
static int	accmode(int) ;
static int	waitready(int,int,int) ;
static int	getprefixfs(cchar *,cchar **) ;
static int	getnormalfs(cchar *,cchar **) ;
static int	noexist(cchar *,int) ;
static int	loadargs(vecstr *,cchar *) ;
static int	hasnonpath(cchar *,int) ;

#if	CF_ISMORE
extern int	isMorePossible(int) ;
#endif


/* local variables */

static cchar	*normalfs[] = {
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

static cchar	*prefixfs[] = {
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

static cchar	*nonpaths = "/¥§~" ;

enum nonpaths {
	nonpath_not,
	nonpath_dialer,
	nonpath_fsvc,
	nonpath_usvc,
	nonpath_overlast
} ;

#if	CF_ISMORE
static const int	rsmore[] = {
	SR_OPNOTSUPP,
	SR_NOENT,
	0
} ;
#endif /* CF_ISMORE */


/* exported subroutines */


int uc_openex(cchar *fname,int oflags,mode_t operms,int timeout,int opts)
{
	UCOPENINFO	oi ;
	int		rs ;

	memset(&oi,0,sizeof(UCOPENINFO)) ;
	oi.fname = fname ;
	oi.oflags = oflags ;
	oi.operms = operms ;
	oi.to = timeout ;
	oi.opts = opts ;

	rs = uc_openinfo(&oi) ;

	return rs ;
}
/* end subroutine (uc_openex) */


int uc_openinfo(UCOPENINFO *oip)
{
	int		rs ;
	int		fd = -1 ;

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

	if ((rs = accmode(oip->oflags)) >= 0) {

#if	CF_DEBUGS
	    debugprintf("uc_openinfo: accmode() rs=%d\n",rs) ;
#endif

	    if (hascdpath(oip->fname,-1)) {
	        const int	tlen = MAXPATHLEN ;
	        char		*tbuf ;
#if	CF_DEBUGS
	    debugprintf("uc_openinfo: cd-path\n") ;
#endif
	        if ((rs = uc_libmalloc((tlen+1),&tbuf)) >= 0) {
	            if ((rs = mkcdpath(tbuf,oip->fname,-1)) > 0) {
#if	CF_DEBUGS
	    		debugprintf("uc_openinfo: 1 mkcdpath() rs=%d\n",rs) ;
#endif
			oip->fname = tbuf ;
	                rs = open_eval(oip) ;
	                fd = rs ;
	            } else if (rs == 0) {
#if	CF_DEBUGS
	    		debugprintf("uc_openinfo: 2 mkcdpath() rs=%d\n",rs) ;
#endif
	                rs = SR_NOENT ;
	            }
	            uc_libfree(tbuf) ;
	        } /* end if (memory-allocation) */
	    } else {
#if	CF_DEBUGS
	    debugprintf("uc_openinfo: other \n") ;
#endif
	        rs = open_eval(oip) ;
	        fd = rs ;
	    }

	} /* end if (accmode) */

#if	CF_DEBUGS
	debugprintf("uc_openinfo: ret rs=%d fd=%u\n",rs,fd) ;
	debugprintf("uc_openinfo: ret fname=%s\n",oip->fname) ;
#endif

	return (rs >= 0) ? fd : rs ;
}
/* end subroutine (uc_openinfo) */


/* local subroutines */


static int u_openex(UCOPENINFO *oip)
{
	int		rs ;
	int		oflags = oip->oflags ;
	int		fd = -1 ;

#if	CF_DEBUGS
	debugprintf("u_openex: ent fn=%s to=%d\n",oip->fname,oip->to) ;
#endif

	if (oip->opts & FM_LARGEFILE) oflags |= O_LARGEFILE ;

	if ((oflags & O_NDELAY) || (oip->to < 0)) {

	    if ((rs = FUNCOPEN(oip->fname,oflags,oip->operms)) >= 0) {
	        fd = rs ;
	    }

#if	CF_DEBUGS
	    debugprintf("u_openex: 1 u_open() rs=%d\n",rs) ;
#endif

	} else {
	    oflags |= O_NDELAY ;
	    if ((rs = FUNCOPEN(oip->fname,oflags,oip->operms)) >= 0) {
	        OURSTAT	sb ;
	        fd = rs ;

#if	CF_DEBUGS
	        debugprintf("u_openex: 2 u_open() rs=%d\n",rs) ;
#endif

	        if ((rs = FUNCFSTAT(fd,&sb)) >= 0) {
	            const mode_t	fm = sb.st_mode ;

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
	            } else {
	                u_close(fd) ;
	                fd = -1 ;
	            }

	        } /* end if (stat) */

	        if ((rs < 0) && (fd >= 0)) {
	            u_close(fd) ;
	            fd = -1 ;
	        }

	    } /* end if (ok) */

#if	CF_DEBUGS
	    debugprintf("u_openex: 2 u_open() rs=%d\n",rs) ;
#endif

	} /* end if (alternatives) */

#if	CF_DEBUGS
	debugprintf("u_openex: ret rs=%d fd=%u\n",rs,fd) ;
#endif

	return (rs >= 0) ? fd : rs ;
}
/* end subroutine (u_openex) */


static int open_eval(UCOPENINFO *oip)
{
	int		rs = SR_OK ;
	int		npi ;			/* non-path index */
	int		fd = -1 ;
	char		*efname = NULL ;
	char		ofname[MAXPATHLEN+1] = { 0 } ;

#if	CF_DEBUGS
	debugprintf("uc_openinfo/open_eval: ent\n") ;
	debugprintf("uc_openinfo/open_eval: fn=%s\n",oip->fname) ;
#endif

	while ((rs >= 0) && (fd < 0)) {

	    if ((npi = hasnonpath(oip->fname,-1)) > 0) {
	        rs = open_floatpath(oip,npi) ;
	        fd = rs ;
	    } else {
	        if (hasvarpathprefix(oip->fname,-1) && (efname == NULL)) {
	            const int	size = (MAXPATHLEN + 1) ;
	            void	*p ;
#if	CF_DEBUGS
		    debugprintf("uc_openinfo/open_eval: var-path\n") ;
#endif
	            if ((rs = uc_libmalloc(size,&p)) >= 0) {
	                efname = p ;
	                efname[0] = '\0' ;
	                if ((rs = mkvarpath(efname,oip->fname,-1)) > 0) {
	                    oip->fname = efname ;
	                } else if (rs <= 0) {
#if	CF_DEBUGS
			    debugprintf("uc_openinfo/open_eval: "
				"mkvarpath() rs=%d\n",rs) ;
#endif
	                    if (rs == 0) rs = SR_BADFMT ;
	                    uc_libfree(efname) ;
	                    efname = NULL ;
	                }
	            }
	        } /* end if (var-path) */

	        if (rs >= 0) {
	            int		pi ;
	            cchar	*rp = NULL ;

	            if ((oip->fname[0] == '/') && 
	                ((pi = getprefixfs(oip->fname,&rp)) >= 0)) {

#if	CF_DEBUGS
	                debugprintf("uc_openinfo: pathprefix fs=%d\n",pi) ;
#endif

	                rs = open_pseudopath(oip,rp,pi) ;
	                fd = rs ;

	            } else if ((oip->fname[0] == '/') && 
	                ((pi = getnormalfs(oip->fname,&rp)) >= 0)) {

#if	CF_DEBUGS
	                debugprintf("uc_openinfo: normalfs pi=%u\n",pi) ;
#endif
	                rs = u_openex(oip) ;
	                fd = rs ;

	            } else {

	                rs = open_othertry(oip,&fd,ofname) ;

	            } /* end if (a protocol or not) */

	        } /* end if (ok) */

	    } /* end if (alternatives) */

#if	CF_DEBUGS
	debugprintf("uc_openinfo/open_eval: while-bot rs=%d fd=%u\n",rs,fd) ;
#endif

	} /* end while */

	if (efname != NULL) {
	    uc_libfree(efname) ;
	}

#if	CF_DEBUGS
	debugprintf("uc_openinfo/open_eval: ret rs=%d fd=%u\n",rs,fd) ;
#endif

	return (rs >= 0) ? fd : rs ;
}
/* end subroutine (open_eval) */


static int open_othertry(UCOPENINFO *oip,int *fdp,char *ofname)
{
	int		rs ;
	int		rs1 ;
	int		fd = -1 ;
#if	CF_DEBUGS
	debugprintf("uc_openinfo/open_othertry: u_openex() rs=%d\n",rs) ;
	debugprintf("uc_openinfo/open_othertry: fn=%s\n",oip->fname) ;
#endif
	if ((rs = u_openex(oip)) >= 0) {
	    fd = rs ;
	} else {
	    USTAT	sb ;
	    switch (rs) {
	    case SR_OPNOTSUPP:
	        {
	            const int	of = oip->oflags ;
	            const int	to = oip->to ;
	            int		rs1 ;
	            cchar	*fn = oip->fname ;

#if	CF_DEBUGS
	            debugprintf("uc_openinfo/open_othertry: OPNOTSUPP\n") ;
#endif

	            if ((rs1 = FUNCSTAT(oip->fname,&sb)) >= 0) {
	                if (S_ISSOCK(sb.st_mode)) {
	                    rs = uc_opensocket(fn,of,to) ;
	                    fd = rs ;
	                }
	            } else if (! isNotPresent(rs1)) {
	                rs = rs1 ;
	            }

	        }
	        break ;
	    case SR_NOENT:
	        {

#if	CF_DEBUGS
	            debugprintf("uc_openinfo/open_othertry: NOENT\n") ;
#endif

	            if ((rs1 = FUNCLSTAT(oip->fname,&sb)) >= 0) {

#if	CF_DEBUGS
	                debugprintf("uc_openinfo/open_othertry: "
				"ent-exists\n") ;
#endif

	                if (S_ISLNK(sb.st_mode)) {
	                    rs = open_otherlink(oip,fdp,ofname) ;
	                } /* end if (S_ISLNK) */

	            } else if (! isNotPresent(rs1)) {
			rs = rs1 ;
	            } /* end if (was a symbolic link) */

	        }
	        break ;
	    } /* end switch */
	} /* end if */
	if ((rs >= 0) && (fd >= 0)) *fdp = fd ;
#if	CF_DEBUGS
	debugprintf("uc_openinfo/open_othertry: ret rs=%d fd=%u\n",rs,fd) ;
#endif
	return rs ;
}
/* end subroutine (open_othertry) */


static int open_otherlink(UCOPENINFO *oip,int *fdp,char *ofname)
{
	const int	rlen = MAXPATHLEN ;
	int		rs ;
	int		fd = -1 ;
	char		rbuf[MAXPATHLEN + 1] ;
	cchar		*fn = oip->fname ;

#if	CF_DEBUGS
	debugprintf("uc_openinfo/open_otherlink: ent\n") ;
#endif

	if ((rs = u_readlink(fn,rbuf,rlen)) >= 0) {
	    int	npi ;
	    rbuf[rs] = '\0' ;

#if	CF_DEBUGS
	    debugprintf("uc_openinfo/open_otherlink: link=%s\n",rbuf) ;
#endif

	    if (rbuf[0] == '/') {

	        oip->fname = (cchar *) ofname ;
	        rs = mkpath1(ofname,rbuf) ;

	    } else if ((npi = hasnonpath(rbuf,-1)) > 0) {

	        oip->fname = (cchar *) ofname ;
	        if ((rs = mkpath1(ofname,rbuf)) >= 0) {
	            rs = open_floatpath(oip,npi) ;
	            fd = rs ;
	        }

	    } else {
	        int		cl ;
	        cchar		*cp ;
	        cchar		*fn = oip->fname ;
	        char		dname[MAXPATHLEN + 1] ;

	        if ((cl = sfdirname(fn,-1,&cp)) > 0) {
	            const int	plen = MAXPATHLEN ;

	            oip->fname = (cchar *) ofname ;
	            if ((rs = snwcpy(dname,plen,cp,cl)) >= 0) {
	                rs = mkpath2(ofname,dname,rbuf) ;
	            }

#if	CF_DEBUGS
	            debugprintf("uc_openinfo/open_otherlink: "
	                "rs=%d new fname=%s\n", rs,oip->fname) ;
#endif

	        } else {

	            oip->fname = (cchar *) ofname ;
	            rs = mkpath1(ofname,rbuf) ;

	        } /* end if */

	    } /* end if */

	    oip->clinks += 1 ;
	    if (rs >= 0) {
	        if (oip->clinks >= MAXSYMLINKS) {
	            rs = SR_MLINK ;
	        }
	    }

	} /* end if (reading symbolic link) */

	if ((rs >= 0) && (fd >= 0)) *fdp = fd ;

	return rs ;
}
/* end subroutine (open_otherlink) */


static int open_floatpath(UCOPENINFO *oip,int npi)
{
	int		rs = SR_OK ;
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
	debugprintf("uc_openinfo/open_float: rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (open_floatpath) */


static int open_pseudopath(UCOPENINFO *oip,cchar *rp,int pi)
{
	int		rs = SR_OK ;
	switch (pi) {
	case prefixfs_proto:
	    rs = uc_openproto(rp,oip->oflags,oip->to,oip->opts) ;
	    break ;
	case prefixfs_prog:
	    rs = openproger(rp,oip->oflags,oip->envv) ;
	    break ;
	case prefixfs_pass:
	    rs = uc_openpass(rp,oip->oflags,oip->to,0) ;
	    break ;
	case prefixfs_shm:
	    rs = uc_openshm(rp,oip->oflags,oip->operms) ;
	    break ;
	case prefixfs_user:
	    oip->fname = rp ;
	    rs = uc_openuserinfo(oip) ;
	    break ;
	case prefixfs_sys:
	case prefixfs_dev:
	    {
	        int		of = oip->oflags ;
	        mode_t		om = oip->operms ;
	        cchar	**envv = oip->envv ;
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
#if	CF_DEBUGS
	debugprintf("uc_openinfo/open_pseudopath: rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (open_pseudopath) */


static int open_nonpath(UCOPENINFO *oip,int npi)
{
	const int	nlen = PRNBUFLEN ;
	const int	nch = MKCHAR(nonpaths[npi]) ;
	int		rs = SR_OK ;
	int		fd = -1 ;
	cchar	*fname = oip->fname ;
	cchar	*tp ;
	char		brkbuf[4] ;

#if	CF_DEBUGS
	debugprintf("uc_openinfo/open_nonpath: fname=%s\n",fname) ;
	debugprintf("uc_openinfo/open_nonpath: npi=%u\n",npi) ;
#endif

	brkbuf[0] = nch ;
	brkbuf[1] = (char) 0xAD ;
	brkbuf[2] = '/' ;
	brkbuf[3] = 0 ;
	if ((tp = strpbrk(fname,brkbuf)) != NULL) {

#if	CF_DEBUGS
	    debugprintf("uc_openinfo/open_nonpath: tp{%p}\n",tp) ;
	    if (tp != NULL)
	        debugprintf("uc_openinfo/open_nonpath: tch=%c\n",*tp) ;
#endif

	    if (MKCHAR(tp[0]) == nch) {
	        char	prn[PRNBUFLEN+1] ;
	        if ((rs = sncpy1w(prn,nlen,fname,(tp - fname))) >= 0) {
	            cchar	*sp = (tp+1) ;
	            if (sp[0] != '\0') {

	                rs = open_nonpather(oip,npi,prn,sp) ;
	                fd = rs ;

	                if ((rs < 0) && (fd >= 0)) u_close(fd) ;
	            } else {
	                rs = SR_PROTO ;		/* no SVC -> protocol error */
		    }
	        } /* end if (sncpy1w) */
	    } else
	        rs = SR_NOANODE ;		/* bug-check exception */
	} /* end if (non-null) */

#if	CF_DEBUGS
	debugprintf("uc_openinfo/open_nonpath: ret rs=%d fd=%u\n",rs,fd) ;
	debugprintf("uc_openinfo/open_nonpath: ret fname=%s\n",fname) ;
#endif

	return (rs >= 0) ? fd : rs ;
}
/* end subroutine (open_nonpath) */


static int open_nonpather(UCOPENINFO *oip,int npi,cchar *prn,cchar *sp)
{
	vecstr		args ;
	int		rs ;
	int		rs1 ;
	int		fd = -1 ;

#if	CF_DEBUGS
	debugprintf("uc_openinfo/open_nonpather: ent\n") ;
#endif

	if ((rs = vecstr_start(&args,4,0)) >= 0) {
	    const int	prlen = MAXPATHLEN ;
	    int		sl = -1 ;
	    cchar	**av = NULL ;
	    cchar	**ev = oip->envv ;
	    cchar	*tp ;
	    char	svc[SVCBUFLEN+1] = { 0 } ;
	    char	brkbuf[4] ;
	    char	prbuf[MAXPATHLEN+1] ;

	    if (ev == NULL) ev = (cchar **) environ ;

	    brkbuf[0] = (char) 0xAD ;
	    brkbuf[1] = ':' ;
	    brkbuf[2] = 0 ;
	    if ((tp = strpbrk(sp,brkbuf)) != NULL) {
	        const int	ch = MKCHAR(*tp) ;
	        int		cl = -1 ;
	        cchar		*cp = sp ;

#if	CF_DEBUGS
	        debugprintf("uc_openinfo/open_nonpather: s=%t\n",
	            sp,sl) ;
#endif

	        if (ch == ':') {
	            sl = (tp-sp) ;
	            cp = (tp+1) ;
	            tp = strchr(cp,0xAD) ;
	        }

#if	CF_DEBUGS
	        debugprintf("uc_openinfo/open_nonpather: s=%t c=%t\n",
	            sp,sl,cp,cl) ;
#endif

	        if (tp != NULL) {

	            cl = (tp-cp) ;
	            if (sl < 0) {
	                sl = cl ; /* or » if (ch != ':') sl = cl « */
	            }
	            if ((rs = vecstr_add(&args,cp,cl)) >= 0) {
	                cp = (tp+1) ;
	                rs = loadargs(&args,cp) ;
	            }

	        } else {
	            rs = vecstr_add(&args,cp,cl) ;
	        }

	    } else {
	        rs = vecstr_add(&args,sp,sl) ;
	    }

	    if (rs >= 0) {
	        if ((rs = sncpy1w(svc,SVCBUFLEN,sp,sl)) >= 0) {
	            rs = vecstr_getvec(&args,&av) ;
	        }
	    }

#if	CF_DEBUGS
	    debugprintf("uc_openinfo/open_nonpather: mid1 rs=%d\n",rs) ;
#endif

	    if (rs >= 0) {
	        switch (npi) {
	        case nonpath_dialer:
	            break ;
	        case nonpath_fsvc:
	            {
			const int	dl = MAXHOSTNAMELEN ;
	                char		dn[MAXHOSTNAMELEN+1] ;
	                if ((rs = getdomainname(dn,dl)) >= 0) {
	                    rs = mkpr(prbuf,prlen,prn,dn) ;
	                }
	            }
	            break ;
	        case nonpath_usvc:
	            {
	                rs = getuserhome(prbuf,prlen,prn) ;
	            }
	            break ;
	        } /* end switch */
	    } /* end if (ok) */

#if	CF_DEBUGS
	    debugprintf("uc_openinfo/open_nonpather: "
	        "mid2 rs=%d\n",rs) ;
	    debugprintf("uc_openinfo/open_nonpather: "
	        "mid2 npi=%u\n",npi) ;
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
	    } /* end if (ok) */

#if	CF_DEBUGS
	    debugprintf("uc_openinfo/open_nonpather: "
	        "mid3 rs=%d\n",rs) ;
#endif

	    rs1 = vecstr_finish(&args) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (vecstr-args) */

#if	CF_DEBUGS
	debugprintf("uc_openinfo/open_nonpather: ret rs=%d fd=%u\n",rs,fd) ;
#endif

	return (rs >= 0) ? fd : rs ;
}
/* end if (open_nonpather) */


static int openproger(cchar *fname,int oflags,cchar **ev)
{
	int		rs ;
	int		rs1 ;
	int		fd = -1 ;
	char		expfname[MAXPATHLEN+1] ;

#if	CF_DEBUGS
	debugprintf("uc_openinfo/openproger: ent fn=%s\n",fname) ;
#endif

	if (ev == NULL) ev = (cchar **) environ ;

	if ((rs = mkuserpath(expfname,NULL,fname,-1)) >= 0) {
	    vecstr	args ;
	    cchar	*fnp = fname ;
	    cchar	*pfp ;
	    cchar	*svcp ;
	    char	progfname[MAXPATHLEN + 1] ;

	    if (rs > 0) fnp = expfname ;

	    if ((rs = vecstr_start(&args,4,0)) >= 0) {

	        pfp = fnp ;
	        if ((svcp = strchr(fnp,0xAD)) != NULL) {
	            pfp = progfname ;
	            if ((rs = mkpath1w(progfname,fnp,(svcp - fnp))) >= 0) {
	                if ((rs = vecstr_add(&args,pfp,rs)) >= 0) {
	                    rs = loadargs(&args,(svcp+1)) ;
	                }
	            }
	        } else {
	            rs = vecstr_add(&args,pfp,-1) ;
	        }

#if	CF_DEBUGS
	        debugprintf("uc_openinfo/openproger: mid1 rs=%d\n",rs) ;
#endif

	        if (rs >= 0) {
	    	    cchar	**av = NULL ;
	            if ((rs = vecstr_getvec(&args,&av)) >= 0) {

#if	CF_DEBUGS
	                debugprintf("uc_openinfo/openproger: "
	                    "mid2 rs=%d\n",rs) ;
#endif

	                if ((pfp[0] == '/') && (pfp[1] == '%')) pfp += 1 ;
	                rs = uc_openprog(pfp,oflags,av,ev) ;
	                fd = rs ;
#if	CF_DEBUGS
	                debugprintf("uc_openinfo/openproger: "
	                    "uc_openprog() rs=%d\n", rs) ;
#endif
	            }
	        } /* end if (ok) */

	        rs1 = vecstr_finish(&args) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (args) */
	    if ((rs < 0) && (fd >= 0)) u_close(fd) ;

	} /* end if (mkuserpath) */

#if	CF_DEBUGS
	debugprintf("uc_openinfo/openproger: ret rs=%d fd=%u\n",rs,fd) ;
#endif

	return (rs >= 0) ? fd : rs ;
}
/* end subroutine (openproger) */


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


static int waitready(int fd,int oflags,int timeout)
{
	int		rs = SR_OK ;
	int		f_wait ;
	int		f_rdonly ;
	int		f = FALSE ;

	f_rdonly = (oflags & O_RDONLY) ;
	f_wait = f_rdonly || (oflags & O_WRONLY) ;
	if ((timeout >= 0) && f_wait) {
	    struct pollfd	polls[NPOLLS] ;
	    time_t		ti_timeout ;
	    time_t		daytime = time(NULL) ;
	    int			size ;
	    int			pollto ;

	    size = NPOLLS * sizeof(struct pollfd) ;
	    memset(polls,0,size) ;

	    polls[0].fd = fd ;
	    polls[0].events = (f_rdonly) ? POLLIN : POLLOUT ;
	    polls[1].fd = -1 ;

	    ti_timeout = daytime + timeout ;
	    while (rs >= 0) {

	        pollto = MIN(timeout,POLLMULT) ;
	        if ((rs = u_poll(polls,1,pollto)) > 0) {
	            const int	re = polls[0].revents ;

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

	        } else if (rs == SR_INTR) {
	            rs = SR_OK ;
	        } /* end if (poll had something) */

	        if ((rs >= 0) && (daytime >= ti_timeout)) {
	            daytime = time(NULL) ;
	            rs = SR_TIMEDOUT ;
	        }

	        if ((rs >= 0) && f) break ;
	    } /* end while */

	} /* end if (waiting) */

	return rs ;
}
/* end subroutine (waitready) */


static int getnormalfs(cchar *fname,cchar **rpp)
{
	int		rs = SR_OK ;
	int		pi = -1 ;

	*rpp = NULL ;
	if (fname[0] == '/') {
	    cchar	*tp, *pp ;

	    pp = (fname + 1) ;
	    while (*pp && (pp[0] == '/')) {
	        pp += 1 ;
	    }

	    if ((tp = strchr(pp,'/')) != NULL) {
	        pi = matstr(normalfs,pp,(tp - pp)) ;
	        *rpp = (pi >= 0) ? tp : NULL ;
	    } else {
	        rs = SR_NOENT ;
	    }

	} else {
	    rs = SR_NOENT ;
	}

	return (rs >= 0) ? pi : rs ;
}
/* end subroutine (getnormalfs) */


/* get the prefix-FS index (if there is a prefix-FS) */
static int getprefixfs(cchar *fname,cchar **rpp)
{
	int		rs = SR_OK ;
	int		pi = -1 ;
	cchar		*tp = NULL ;

	if (fname[0] == '/') {
	    cchar	*pp = (fname + 1) ;
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
	    } else {
	        rs = SR_NOENT ;
	    }

	} else {
	    rs = SR_NOENT ;
	}

	if (rpp != NULL) {
	    *rpp = (rs >= 0) ? tp : NULL ;
	}

	return (rs >= 0) ? pi : rs ;
}
/* end subroutine (getprefixfs) */


static int noexist(cchar *pp,int pl)
{
	const int	nlen = MAXNAMELEN ;
	int		rs ;
	char		pfname[MAXNAMELEN + 1] ;

	pfname[0] = '/' ;
	if ((rs = snwcpy((pfname+1),(nlen-1),pp,pl)) >= 0) {
	    OURSTAT	sb ;
	    if ((rs = FUNCSTAT(pfname,&sb)) >= 0) {
	        if (! S_ISDIR(sb.st_mode)) {
	            rs = SR_EXIST ;
	        }
	    } else if (isNotPresent(rs)) {
	        rs = SR_OK ;
	    }
	}

	return rs ;
}
/* end subroutine (noexist) */


static int loadargs(vecstr *alp,cchar *sp)
{
	int		rs = SR_OK ;
	int		c = 0 ;
	cchar		*tp ;

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


static int hasnonpath(cchar *fp,int fl)
{
	int		f ;

	if (fl < 0) fl = strlen(fp) ;

	f = (fp[0] != '/') ;
	if (f) {
	    cchar	*tp = strnpbrk(fp,fl,nonpaths) ;
	    f = FALSE ;
	    if ((tp != NULL) && ((tp-fp) > 0) && (tp[1] != '\0')) {
	        f = sichr(nonpaths,-1,*tp) ;
	    }
	}

	return f ;
}
/* end subroutine (hasnonpath) */


#if	CF_ISMORE
static int isMorePossible(int rs)
{
	return isOneOf(rsmore,rs) ;
}
/* end subroutine (isMorePossible) */
#endif /* CF_ISMORE */


