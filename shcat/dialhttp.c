/* dialhttp */

/* dial out to the web */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable */


/* revision history:

	= 1998-04-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine will dial out to an INET host using the HTTP protocol.

	Synopsis:

	int dialhttp(hname,pspec,af,svcname,svcargv,timeout,opts)
	const char	hname[] ;
	const char	pspec[] ;
	int		af ;
	const char	svcname[] ;
	const char	*svcargv[] ;
	int		timeout ;
	int		opts ;

	Arguments:

	hname		host to dial to
	pspec		port specification to use
	af		address family
	svcname		service specification
	svcargs		pointer to array of pointers to arguments
	timeout		timeout ('>0' means use one, '-1' means don't)
	opts		any dial options

	Returns:

	>=0		file descriptor
	<0		error in dialing


	What is up with the 'timeout' argument?

	>0		use the timeout as it is

	==0             asynchronously span the connect and do not wait
			for it

	<0              use the system default timeout (xx minutes --
			whatever)


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/socket.h>
#include	<netinet/in.h>
#include	<arpa/inet.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<signal.h>
#include	<string.h>
#include	<netdb.h>

#include	<vsystem.h>
#include	<estrings.h>
#include	<ids.h>
#include	<vecstr.h>
#include	<sbuf.h>
#include	<localmisc.h>


/* local defines */

#ifndef	SIGACTION
#define	SIGACTION	struct sigaction
#endif

#ifndef	VARPATH
#define	VARPATH		"PATH"
#endif

#define	PROG_WGET	"wget"

#define	BINDNAME	"bin"

#define	URLBUFLEN	(4 * MAXHOSTNAMELEN)
#define	PRBUFLEN	MAXPATHLEN
#define	TOBUFLEN	20

#ifndef	SVCLEN
#define	SVCLEN		MAXNAMELEN
#endif


/* external subroutines */

extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	pathclean(char *,const char *,int) ;
extern int	getnodedomain(char *,char *) ;
extern int	getdomainname(char *,int) ;
extern int	getprogpath(IDS *,vecstr *,char *,cchar *,int) ;
extern int	mkpr(char *,int,const char *,const char *) ;
extern int	sperm(IDS *,struct ustat *,int) ;
extern int	dialprog(cchar *,int,cchar **,cchar **,int *) ;
extern int	ctdeci(char *,int,int) ;
extern int	vecstr_adduniq(vecstr *,cchar *,int) ;
extern int	isNotPresent(int) ;
extern int	isNotAccess(int) ;


/* external variables */


/* local structures */


/* forward references */

static int	findprog(char *,const char *) ;
static int	loadpath(vecstr *,const char *) ;
static int	findprprog(IDS *,vecstr *,char *,const char **,const char *) ;
static int	runprog(cchar *,cchar *,cchar *,cchar *) ;
static int	mkurl(char *,int,cchar *,cchar *,cchar *,cchar **) ;


/* local variables */

static const char	*prnames[] = {
	"gnu",
	"extra",
	"usr",
	"local",
	NULL
} ;


/* exported subroutines */


/* ARGSUSED */
int dialhttp(hname,pspec,af,svcname,svcargv,timeout,opts)
const char	hname[] ;
const char	pspec[] ;
int		af ;
const char	svcname[] ;
const char	*svcargv[] ;
int		timeout ;
int		opts ;
{
	int		rs = SR_OK ;
	int		fd = -1 ;
	int		f ;

	if (hname == NULL) return SR_FAULT ;

	if (hname[0] == '\0') return SR_INVALID ;

	f = FALSE ;
	f = f || (af == AF_UNSPEC) ;
	f = f || (af == AF_INET4) ;
	f = f || (af == AF_INET6) ;
	if (f) {
	    char	tobuf[TOBUFLEN + 1] = { 0 } ;

#if	CF_DEBUGS
	    debugprintf("dialhttp: hname=%s portname=%s svcname=%s\n",
	        hname,pspec,svcname) ;
	    debugprintf("dialhttp: ent timeout=%d\n",timeout) ;
#endif

	    if (timeout >= 0) {
	        if (timeout == 0) timeout = 1 ;
	        rs = ctdeci(tobuf,TOBUFLEN,timeout) ;
	    }

/* format the URL string to be transmitted */

	    if (rs >= 0) {
	        const int	ulen = URLBUFLEN ;
	        char		ubuf[URLBUFLEN + 1] ;
	        if ((rs = mkurl(ubuf,ulen,hname,pspec,svcname,svcargv)) >= 0) {
	            cchar	*pn = PROG_WGET ;
	            char	execfname[MAXPATHLEN + 1] ;
	            if ((rs = findprog(execfname,pn)) >= 0) {
	                rs = runprog(execfname,ubuf,tobuf,pn) ;
	                fd = rs ;
	            } /* end if (findprog) */
	        } /* end if (mkurl) */
	    } /* end if (ok) */
	} else {
	    rs = SR_AFNOSUPPORT	 ;
	}

#if	CF_DEBUGS
	debugprintf("dialhttp: ret rs=%d fd=%d\n",rs,fd) ;
#endif

	return (rs >= 0) ? fd : rs ;
}
/* end subroutine (dialhttp) */


/* local subroutines */


static int findprog(char *execfname,cchar *pn)
{
	IDS		id ;
	int		rs  ;
	int		rs1 ;
	int		pl = 0 ;

	execfname[0] = '\0' ;
	if ((rs = ids_load(&id)) >= 0) {
	    vecstr	path ;
	    if ((rs = vecstr_start(&path,20,0)) >= 0) {
	        if ((rs = loadpath(&path,VARPATH)) >= 0) {

	            rs = getprogpath(&id,&path,execfname,pn,-1) ;
	            pl = rs ;
	            if ((rs == SR_NOENT) || (rs == SR_ACCESS)) {
	                rs = findprprog(&id,&path,execfname,prnames,pn) ;
	                pl = rs ;
	            }

	        } /* end if (loadpath) */
	        rs1 = vecstr_finish(&path) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (vecstr) */
	    rs1 = ids_release(&id) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (ids) */

	return (rs >= 0) ? pl : rs ;
}
/* end subroutine (findprog) */


static int findprprog(idp,plp,rbuf,prnames,pn)
IDS		*idp ;
vecstr		*plp ;
char		rbuf[] ;
const char	*prnames[] ;
const char	*pn ;
{
	int		rs ;
	int		rl = 0 ;
	const int	perms = (R_OK | X_OK) ;
	char		dn[MAXHOSTNAMELEN + 1] ;

	rbuf[0] = '\0' ;
	if ((rs = getnodedomain(NULL,dn)) >= 0) {
	    USTAT	sb ;
	    const int	rsn = SR_NOTFOUND ;
	    const int	prlen = PRBUFLEN ;
	    int		i ;
	    cchar	*bdname = BINDNAME ;
	    char	prbuf[PRBUFLEN + 1] ;
	    char	dbuf[MAXPATHLEN+1] ;
	    for (i = 0 ; prnames[i] != NULL ; i += 1) {
	        rbuf[0] = '\0' ;
	        if ((rs = mkpr(prbuf,prlen,prnames[i],dn)) >= 0) {
	            if ((rs = mkpath2(dbuf,prbuf,bdname)) >= 0) {
	                if ((rs = vecstr_find(plp,dbuf)) == rsn) {
	                    if ((rs = mkpath2(rbuf,dbuf,pn)) >= 0) {
	                        rl = rs ;
	                        if ((rs = u_stat(rbuf,&sb)) >= 0) {
	                            if (S_ISREG(sb.st_mode)) {
	                                rs = sperm(idp,&sb,perms) ;
	                                if (isNotAccess(rs)) {
	                                    rl = 0 ;
	                                }
	                            } else {
	                                rs = SR_ISDIR ;
	                            }
	                        } else if (isNotPresent(rs)) {
	                            rl = 0 ;
	                            rs = SR_OK ;
	                        }
	                    } /* end if (mkpath) */
	                } /* end if (not-found) */
	            } /* end if (mkpath) */
	        } /* end if (mkpr) */
	        if (rl > 0) break ;
	        if (rs < 0) break ;
	    } /* end for */
	} /* end if (getnodedomain) */

	return (rs >= 0) ? rl : rs ;
}
/* end subroutine (findprprog) */


static int runprog(cchar *execfname,cchar *ubuf,cchar *tobuf,cchar *pn)
{
	SIGACTION	nsh, osh ;
	sigset_t	smask ;
	const int	of = O_NOCTTY ;
	const int	sig = SIGPIPE ;
	int		rs ;
	int		fd = -1 ;

	uc_sigsetempty(&smask) ;

	memset(&nsh,0,sizeof(SIGACTION)) ;
	nsh.sa_handler = SIG_IGN ;
	nsh.sa_mask = smask ;
	nsh.sa_flags = 0 ;

	if ((rs = u_sigaction(sig,&nsh,&osh)) >= 0) {
	    int		i = 0 ;
	    cchar	*av[10] ;

	    av[i++] = pn ;
	    av[i++] = "-q" ;
	    av[i++] = "-O" ;
	    av[i++] = "-" ;
	    if (tobuf[0] != '\0') {
	        av[i++] = "-T" ;
	        av[i++] = tobuf ;
	    }
	    av[i++] = ubuf ;
	    av[i] = NULL ;
	    rs = dialprog(execfname,of,av,NULL,NULL) ;
	    fd = rs ;

	    u_sigaction(sig,&osh,NULL) ;
	} /* end if (signals) */

	return (rs >= 0) ? fd : rs ;
}
/* end subroutine (runprog) */


static int loadpath(vecstr *plp,cchar *varpath)
{
	int		rs = SR_OK ;
	int		pl ;
	int		c = 0 ;
	const char	*pp ;

	if ((pp = getenv(varpath)) != NULL) {
	    const int	rsn = SR_NOTFOUND ;
	    const char	*tp ;
	    char	tbuf[MAXPATHLEN + 1] ;

	    while ((tp = strpbrk(pp,":;")) != NULL) {
	        if ((rs = pathclean(tbuf,pp,(tp - pp))) >= 0) {
	            pl = rs ;

	            if ((rs = vecstr_findn(plp,tbuf,pl)) == rsn) {
	                c += 1 ;
	                rs = vecstr_add(plp,tbuf,pl) ;
	            }

	            pp = (tp + 1) ;
	        }
	        if (rs < 0) break ;
	    } /* end while */

	    if ((rs >= 0) && (pp[0] != '\0')) {
	        if ((rs = pathclean(tbuf,pp,-1)) >= 0) {
	            pl = rs ;

	            if ((rs = vecstr_findn(plp,tbuf,pl)) == rsn) {
	                c += 1 ;
	                rs = vecstr_add(plp,tbuf,pl) ;
	            }

	        }
	    } /* end if (trailing one) */

	} /* end if (getenv) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (loadpath) */


static int mkurl(ubuf,ulen,hname,pspec,svcname,svcargv)
char		ubuf[] ;
int		ulen ;
const char	*hname ;
const char	*pspec ;
const char	*svcname ;
const char	*svcargv[] ;
{
	SBUF		url ;
	int		rs ;
	int		rs1 ;

	if ((rs = sbuf_start(&url,ubuf,ulen)) >= 0) {

	    sbuf_strw(&url,"http://",-1) ;

	    sbuf_strw(&url,hname,MAXHOSTNAMELEN) ;

	    if ((pspec != NULL) && (pspec[0] != '\0')) {
	        sbuf_char(&url,':') ;
	        sbuf_strw(&url,pspec,-1) ;
	    }

	    sbuf_char(&url,'/') ;

	    if ((svcname != NULL) && (svcname[0] != '\0')) {
	        if (svcname[0] == '/') svcname += 1 ;
	        sbuf_strw(&url,svcname,-1) ;
	    } /* end if */

	    if (svcargv != NULL) {
	        int	i ;
	        for (i = 0 ; svcargv[i] != NULL ; i += 1) {
	            sbuf_char(&url,'?') ;
	            sbuf_buf(&url,svcargv[i],-1) ;
	        } /* end for */
	    } /* end if */

	    rs1 = sbuf_finish(&url) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if */

	return rs ;
}
/* end subroutine (mkurl) */


