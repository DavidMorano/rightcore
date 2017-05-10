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

	int dialhttp(hostname,portspec,af,svcname,svcargv,timeout,opts)
	const char	hostname[] ;
	const char	portspec[] ;
	int		af ;
	const char	svcname[] ;
	const char	*svcargv[] ;
	int		timeout ;
	int		opts ;

	Arguments:

	hostname	host to dial to
	portspec	port specification to use
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
extern int	mkpr(char *,int,const char *,const char *) ;
extern int	getprogpath(IDS *,vecstr *,char *,const char *,int) ;
extern int	sperm(IDS *,struct ustat *,int) ;
extern int	dialprog(const char *,int,const char **,const char **,int *) ;
extern int	ctdeci(char *,int,int) ;
extern int	vecstr_adduniq(vecstr *,const char *,int) ;


/* external variables */


/* local structures */


/* forward references */

static int	findprog(char *,const char *) ;
static int	loadpath(vecstr *,const char *) ;
static int	findprprog(IDS *,vecstr *,char *,const char **,const char *) ;
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


int dialhttp(hostname,portspec,af,svcname,svcargv,timeout,opts)
const char	hostname[] ;
const char	portspec[] ;
int		af ;
const char	svcname[] ;
const char	*svcargv[] ;
int		timeout ;
int		opts ;
{
	struct sigaction	sighand, oldsighand ;
	sigset_t	signalmask ;
	const int	of = O_NOCTTY ;
	int		rs = SR_OK ;
	int		i ;
	int		fd = 0 ;
	int		f ;
	const char	*pn = PROG_WGET ;
	const char	*av[10] ;
	char		urlbuf[URLBUFLEN + 1] ;
	char		execfname[MAXPATHLEN + 1] ;
	char		tobuf[TOBUFLEN + 1] ;

	if (hostname == NULL) return SR_FAULT ;

	if (hostname[0] == '\0') return SR_INVALID ;

	f = FALSE ;
	f = f || (af == AF_UNSPEC) ;
	f = f || (af == AF_INET4) ;
	f = f || (af == AF_INET6) ;
	if (! f) {
	    rs = SR_AFNOSUPPORT	;
	    goto ret0 ;
	}

#ifdef	COMMENT
	if (af == AF_UNSPEC) af = AF_INET4 ;
#endif

#if	CF_DEBUGS
	debugprintf("dialhttp: hostname=%s portname=%s svcname=%s\n",
	    hostname,portspec,svcname) ;
	debugprintf("dialhttp: ent timeout=%d\n",timeout) ;
#endif

	tobuf[0] = '\0' ;
	    if (timeout >= 0) {
		if (timeout == 0) timeout = 1 ;
		rs = ctdeci(tobuf,TOBUFLEN,timeout) ;
	    }
	if (rs < 0) goto ret0 ;

/* format the URL string to be transmitted */

	rs = mkurl(urlbuf,URLBUFLEN,hostname,portspec,svcname,svcargv) ;
	if (rs < 0) goto ret0 ;

/* find the program */

	rs = findprog(execfname,pn) ;
	if (rs < 0) goto ret0 ;

	i = 0 ;
	av[i++] = pn ;
	av[i++] = "-q" ;
	av[i++] = "-O" ;
	av[i++] = "-" ;
	if (tobuf[0] != '\0') {
		av[i++] = "-T" ;
		av[i++] = tobuf ;
	}
	av[i++] = urlbuf ;
	av[i] = NULL ;

	uc_sigsetempty(&signalmask) ;
	sighand.sa_handler = SIG_IGN ;
	sighand.sa_mask = signalmask ;
	sighand.sa_flags = 0 ;
	if ((rs = u_sigaction(SIGPIPE,&sighand,&oldsighand)) >= 0) {

	    rs = dialprog(execfname,of,av,NULL,NULL) ;
	    fd = rs ;

	    u_sigaction(SIGPIPE,&oldsighand,NULL) ;
	} /* end if (signals) */

ret0:

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
	vecstr		path ;
	int		rs  ;
	int		pl = 0 ;

	execfname[0] = '\0' ;
	if ((rs = ids_load(&id)) >= 0) {

	    if ((rs = vecstr_start(&path,20,0)) >= 0) {

	        if ((rs = loadpath(&path,VARPATH)) >= 0) {

	            rs = getprogpath(&id,&path,execfname,pn,-1) ;
	            pl = rs ;
	            if ((rs == SR_NOENT) || (rs == SR_ACCESS)) {
	                rs = findprprog(&id,&path,execfname,prnames,pn) ;
	                pl = rs ;
	            }

	        } /* end if (loadpath) */

	        vecstr_finish(&path) ;
	    } /* end if (vecstr) */

	    ids_release(&id) ;
	} /* end if (ids) */

	return (rs >= 0) ? pl : rs ;
}
/* end subroutine (findprog) */


static int findprprog(idp,plp,execfname,prnames,pn)
IDS		*idp ;
vecstr		*plp ;
char		execfname[] ;
const char	*prnames[] ;
const char	*pn ;
{
	struct ustat	sb ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		i ;
	int		pl = 0 ;
	const int	perms = (R_OK | X_OK) ;
	char		domainname[MAXHOSTNAMELEN + 1] ;
	char		prbuf[PRBUFLEN + 1] ;

	execfname[0] = '\0' ;
	rs = getnodedomain(NULL,domainname) ;
	if (rs < 0)
	    goto ret0 ;

	for (i = 0 ; prnames[i] != NULL ; i += 1) {

	    rs = mkpr(prbuf,PRBUFLEN,prnames[i],domainname) ;
	    if (rs >= 0) {

	        rs = mkpath2(execfname,prbuf,BINDNAME) ;
	        if (rs >= 0) {

	            rs = SR_NOENT ;
	            rs1 = vecstr_find(plp,execfname) ;
	            if (rs1 == SR_NOTFOUND) {

	                rs = mkpath2(execfname,"/",pn) ;
			pl = rs ;
	                if (rs >= 0)
	                    rs = u_stat(execfname,&sb) ;
	                if (rs >= 0) {
	                    rs = SR_ISDIR ;
	                    if (S_ISREG(sb.st_mode))
	                        rs = sperm(idp,&sb,perms) ;
	                }

	            } /* end if */

	        } /* end if */

	    } /* end if */

	    execfname[0] = '\0' ;
	    if (rs >= 0) break ;
	} /* end for */

ret0:
	return (rs >= 0) ? pl : rs ;
}
/* end subroutine (findprprog) */


static int loadpath(vecstr *plp,cchar *varpath)
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		pl ;
	int		c = 0 ;
	const char	*tp ;
	const char	*pp ;
	char		tmpfname[MAXPATHLEN + 1] ;

	if ((pp = getenv(varpath)) != NULL) {

	    while ((tp = strpbrk(pp,":;")) != NULL) {

	        pl = pathclean(tmpfname,pp,(tp - pp)) ;

	        rs1 = vecstr_findn(plp,tmpfname,pl) ;
	        if (rs1 == SR_NOTFOUND) {
	            c += 1 ;
	            rs = vecstr_add(plp,tmpfname,pl) ;
	        }

	        pp = (tp + 1) ;
	        if (rs < 0) break ;
	    } /* end while */

	    if ((rs >= 0) && (pp[0] != '\0')) {

	        pl = pathclean(tmpfname,pp,-1) ;

	        rs1 = vecstr_findn(plp,tmpfname,pl) ;
	        if (rs1 == SR_NOTFOUND) {
	            c += 1 ;
	            rs = vecstr_add(plp,tmpfname,pl) ;
	        }

	    } /* end if (trailing one) */

	} /* end if (getenv) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (loadpath) */


static int mkurl(urlbuf,urllen,hostname,portspec,svcname,svcargv)
char		urlbuf[] ;
int		urllen ;
const char	*hostname ;
const char	*portspec ;
const char	*svcname ;
const char	*svcargv[] ;
{
	SBUF		url ;
	int		rs ;
	int		rs1 ;

	if ((rs = sbuf_start(&url,urlbuf,urllen)) >= 0) {

	    sbuf_strw(&url,"http://",-1) ;

	    sbuf_strw(&url,hostname,MAXHOSTNAMELEN) ;

	    if ((portspec != NULL) && (portspec[0] != '\0')) {
	        sbuf_char(&url,':') ;
	        sbuf_strw(&url,portspec,-1) ;
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


