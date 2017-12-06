/* dialuux */

/* perform remote UUX service execution */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_GETPROGROOT	1		/* use 'getprogroot(3dam)' */


/* revision history:

	= 1998-07-10, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This is a dialer to connect to the UUX facility.

	Synopsis:

	int dialuux(pr,node,svc,argv,u,g,opts)
	const char	pr[] ;
	const char	node[] ;
	const char	svc[] ;
	char		*argv[] ;
	const char	u[] ;
	const char	g[] ;
	int		opts ;

	Arguments:

	pr		program-root
	node		target node
	svc		target service
	argv		arguments to program
	opts		options

	Returns:

	>=0		file descriptor to program STDIN and STDOUT
	<0		error


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
#include	<vecstr.h>
#include	<storebuf.h>
#include	<localmisc.h>

#include	"dialuux.h"
#include	"uuname.h"


/* local defines */

#ifndef	DEFINITFNAME
#define	DEFINITFNAME	"/etc/default/init"
#endif

#ifndef	DEFLOGFNAME
#define	DEFLOGFNAME	"/etc/default/login"
#endif

#ifndef	NULLFNAME
#define	NULLFNAME	"/dev/null"
#endif

#ifndef	NOFILE
#define	NOFILE		20
#endif

#ifndef	VARUSERNAME
#define	VARUSERNAME	"USERNAME"
#endif

#ifndef	VARHOME
#define	VARHOME		"HOME"
#endif

#ifndef	VARPWD
#define	VARPWD		"PWD"
#endif

#ifndef	VARMAIL
#define	VARMAIL		"MAIL"
#endif

#ifndef	VARPATH
#define	VARPATH		"PATH"
#endif

#ifndef	VARLIBPATH
#define	VARLIBPATH	"LD_LIBRARY_PATH"
#endif

#ifndef	VARMANPATH
#define	VARMANPATH	"MANPATH"
#endif

#ifndef	VARPRLOCAL
#define	VARPRLOCAL	"LOCAL"
#endif

#ifndef	VARPRNCMP
#define	VARPRNCMP	"NCMP"
#endif

#ifndef	VARDISPLAY
#define	VARDISPLAY	"DISPLAY"
#endif

#ifndef	VARPRUUX
#define	VARPRUUX	"UUX_PROGRAMROOT"
#endif

#define	BUFLEN		MAXPATHLEN

#define	PROG_UUX	"uux"

#define	NENVS		120

#define	TO_CREAD	10


/* external subroutines */

extern int	snwcpy(char *,int,const char *,int) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	sncpy3(char *,int,const char *,const char *,const char *) ;
extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	matkeystr(const char **,const char *,int) ;
extern int	strkeycmp(const char *,const char *) ;
extern int	vstrkeycmp(const void **,const void **) ;
extern int	pathclean(char *,const char *,int) ;
extern int	ctdecl(char *,int,long) ;
extern int	vecstr_envadd(vecstr *,const char *,const char *,int) ;
extern int	vecstr_adduniq(vecstr *,const char *,int) ;
extern int	perm(const char *,uid_t,gid_t,gid_t *,int) ;
extern int	getpwd(char *,int) ;
extern int	prgetprogpath(const char *,char *,const char *,int) ;
extern int	getprogroot(const char *,const char **,
			int *,char *,const char *) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strwcpylc(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;
extern char	*strnpbrk(const char *,int,const char *) ;


/* external variables */


/* local structures */

struct envpop {
	const char	*name ;
	const char	*sub1dname ;
	const char	*sub2dname ;
} ;


/* forward reference */

static int	vecstr_defenvs(vecstr *,const char **) ;
static int	vecstr_loadpath(vecstr *,const char *) ;
static int	mkpathval(vecstr *,char *,int) ;

static int	procenvpaths(const char *,vecstr *) ;
static int	mknodesvc(char *,int,const char *,const char *) ;
static int	havenode(const char *,const char *) ;


/* local variables */

static const char	*prnames[] = {
	"LOCAL",
	"NCMP",
	"EXTRA",
	"PCS",
	NULL
} ;

static const char	*envdefs[] = {
	"PATH",
	"LD_LIBRARY_PATH",
	"MAIL",
	"MAILDIR",
	NULL
} ;

static const struct envpop	envpops[] = {
	{ VARPATH, "bin", "sbin" },
	{ VARLIBPATH, "lib", NULL },
	{ VARMANPATH, "man", NULL },
	{ NULL, NULL, NULL }
} ;


/* exported subroutines */


int dialuux(pr,node,svc,argv,u,g,opts)
const char	pr[] ;
const char	node[] ;
const char	svc[] ;
const char	*argv[] ;
const char	u[] ;
const char	g[] ;
int		opts ;
{
	vecstr		args ;
	vecstr		envs ;
	int		rs = SR_OK ;
	int		i ;
	int		oflags ;
	int		prlen = 0 ;
	int		fd = -1 ;
	const char	*varpruux = VARPRUUX ;
	char		progfname[MAXPATHLEN + 1] ;
	const char	*pn = PROG_UUX ;
	const char	**av ;
	const char	**ev ;

	if (pr == NULL) return SR_FAULT ;
	if (node == NULL) return SR_FAULT ;
	if (svc == NULL) return SR_FAULT ;
	if (pr[0] == '\0') return SR_INVALID ;
	if (svc[0] == '\0') return SR_INVALID ;

#if	CF_DEBUGS
	debugprintf("dialuux: pr=%s\n",pr) ;
	debugprintf("dialuux: node=%s\n",node) ;
	debugprintf("dialuux: svc=%s\n",svc) ;
#endif

/* check node */

	rs = havenode(pr,node) ;

#if	CF_DEBUGS
	debugprintf("dialuux: havenode() rs=%d\n",rs) ;
#endif

	if (rs < 0)
	    goto ret0 ;

/* initialize */

	rs = vecstr_start(&envs,10,0) ;
	if (rs < 0)
	    goto ret0 ;

/* search for the program (executable file) */

#if	CF_GETPROGROOT
	rs = getprogroot(pr,prnames,&prlen,progfname,pn) ;

#if	CF_DEBUGS
	debugprintf("dialuux: getprogroot() rs=%d prlen=%u pr=%t\n",
		rs,prlen,progfname,prlen) ;
#endif

	if (rs == 0)
	    rs = mkpath1(progfname,pn) ;

#ifdef	COMMENT
	if ((rs >= 0) && (prlen > 0)) {
	    rs = vecstr_envadd(&envs,varpruux,progfname,prlen) ;
	} else if (rs >= 0)
	    rs = vecstr_envadd(&envs,varpruux,op->pr,-1) ;
#endif /* COMMENT */

#else /* CF_GETPROGROOT */
	rs = prgetprogpath(pr,progfname,pn,-1) ;
	if (rs == 0)
	    rs = mkpath1(progfname,pn) ;
#endif /* CF_GETPROGROOT */

	if (rs < 0)
	    goto ret1 ;

/* set default environment variables */

#if	CF_DEBUGS
	debugprintf("dialuux: defenvs\n") ;
#endif

	if (rs >= 0)
	    rs = procenvpaths(pr,&envs) ;

	if (rs >= 0)
	    rs = vecstr_envadd(&envs,varpruux,pr,-1) ;

	if (rs >= 0)
	    rs = vecstr_defenvs(&envs,prnames) ;

	if (rs >= 0)
	    rs = vecstr_defenvs(&envs,envdefs) ;

/* build the program arguments */

#if	CF_DEBUGS
	debugprintf("dialuux: program arguments \n") ;
#endif

	if (rs >= 0)
	    rs = vecstr_start(&args,10,0) ;

#if	CF_DEBUGS
	debugprintf("dialuux: vecstr_start(&args) rs=%d\n",rs) ;
#endif

	if (rs < 0)
	    goto ret1 ;

	if (rs >= 0)
	    rs = vecstr_add(&args,pn,-1) ;

/* set some program options */

	if (rs >= 0)
	    rs = vecstr_add(&args,"-p",2) ;

	if ((rs >= 0) && (opts & DIALUUX_OQUEUE))
	    rs = vecstr_add(&args,"-r",2) ;

	if ((rs >= 0) && (opts & DIALUUX_ONOREPORT))
	    rs = vecstr_add(&args,"-n",2) ;

	if ((rs >= 0) && (u != NULL) && (u[0] != '\0')) {
	    rs = vecstr_add(&args,"-a",2) ;
	    if (rs >= 0)
	        rs = vecstr_add(&args,u,-1) ;
	}

	if ((rs >= 0) && (g != NULL) && (g[0] != '\0')) {
	    rs = vecstr_add(&args,"-g",2) ;
	    if (rs >= 0)
	        rs = vecstr_add(&args,g,-1) ;
	}

/* load up the node and service (as they are available) */

	if (rs >= 0) {
	    char	buf[BUFLEN + 1] ;

	    rs = mknodesvc(buf,BUFLEN,node,svc) ;

	    if (rs >= 0)
	        rs = vecstr_add(&args,buf,rs) ;

	} /* end if */

/* load up any supplied arguments */

	if ((rs >= 0) && (argv != NULL)) {
	    for (i = 0 ; argv[i] != NULL ; i += 1) {
	        rs = vecstr_add(&args,argv[i],-1) ;
	        if (rs < 0) break ;
	    }
	}

	if (rs < 0)
	    goto ret2 ;

/* execute */

#if	CF_DEBUGS
	debugprintf("dialuux: execute progfname=%s\n",progfname) ;
#endif

	vecstr_getvec(&args,&av) ;

	vecstr_getvec(&envs,&ev) ;

	oflags = O_WRONLY ;
	rs = uc_openprog(progfname,oflags,av,ev) ;
	fd = rs ;

ret2:
	vecstr_finish(&args) ;

ret1:
	vecstr_finish(&envs) ;

ret0:

#if	CF_DEBUGS
	debugprintf("dialuux: ret rs=%d fd=%u\n",rs,fd) ;
#endif

	return (rs >= 0) ? fd : rs ;
}
/* end subroutine (dialuux) */


/* local subroutines */


static int vecstr_defenvs(elp,ea)
vecstr		*elp ;
const char	**ea ;
{
	int		rs = SR_OK ;
	int		i ;
	int		c = 0 ;
	const char	*cp ;

	for (i = 0 ; ea[i] != NULL ; i += 1) {
	    if ((cp = getenv(ea[i])) != NULL)
		rs = vecstr_envadd(elp,ea[i],cp,-1) ;
	    if (rs < 0) break ;
	} /* end for */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (vecstr_defenvs) */


static int vecstr_loadpath(clp,pp)
vecstr		*clp ;
const char	*pp ;
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		cl ;
	int		c = 0 ;
	const char	*cp ;
	char		tmpfname[MAXPATHLEN + 1] ;

	while ((cp = strpbrk(pp,":;")) != NULL) {

	    cl = pathclean(tmpfname,pp,(cp - pp)) ;

	    rs1 = vecstr_findn(clp,tmpfname,cl) ;
	    if (rs1 == SR_NOTFOUND) {
	        c += 1 ;
		rs = vecstr_add(clp,tmpfname,cl) ;
	    }

	    if ((rs >= 0) && (cp[0] == ';'))
		rs = vecstr_adduniq(clp,";",1) ;

	    pp = (cp + 1) ;
	    if (rs < 0) break ;
	} /* end while */

	if ((rs >= 0) && (pp[0] != '\0')) {

	    cl = pathclean(tmpfname,pp,-1) ;

	    rs1 = vecstr_findn(clp,tmpfname,cl) ;
	    if (rs1 == SR_NOTFOUND) {
	        c += 1 ;
	        rs = vecstr_add(clp,tmpfname,cl) ;
	    }

	} /* end if (trailing one) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (vecstr_loadpath) */


static int mkpathval(clp,vbuf,vbuflen)
vecstr		*clp ;
char		vbuf[] ;
int		vbuflen ;
{
	int		rs = SR_OK ;
	int		i ;
	int		sch ;
	int		c = 0 ;
	int		rlen = 0 ;
	int		f_semi = FALSE ;
	const char	*cp ;

	if (vbuflen < 0) {
	    rs = SR_NOANODE ;
	    goto ret0 ;
	}

	vbuf[0] = '\0' ;
	for (i = 0 ; vecstr_get(clp,i,&cp) >= 0 ; i += 1) {
	    if (cp == NULL) continue ;

	    if (cp[0] != ';') {

	        if (c++ > 0) {
	            if (f_semi) {
	                f_semi = FALSE ;
	                sch = ';' ;
	            } else
	                sch = ':' ;

	            rs = storebuf_char(vbuf,vbuflen,rlen,sch) ;
	            rlen += rs ;

	        } /* end if */

	        if (rs >= 0) {
	            rs = storebuf_strw(vbuf,vbuflen,rlen,cp,-1) ;
	            rlen += rs ;
	        }

	    } else
	        f_semi = TRUE ;

	    if (rs < 0) break ;
	} /* end for */

ret0:
	return (rs >= 0) ? rlen : rs ;
}
/* end subroutine (mkpathval) */


static int procenvpaths(pr,elp)
const char	pr[] ;
vecstr		*elp ;
{
	vecstr	pathcomps ;

	int	rs = SR_OK ;
	int	i ;
	int	opts ;
	int	size ;
	int	bl, pl ;

	const char	*subdname ;
	const char	*np ;
	const char	*vp ;

	char	pathbuf[MAXPATHLEN + 1] ;
	char	*bp = NULL ;


	opts = VECSTR_OORDERED | VECSTR_OSTSIZE ;
	if ((rs = vecstr_start(&pathcomps,40,opts)) >= 0) {

	for (i = 0 ; envpops[i].name != NULL ; i += 1) {
	    np = envpops[i].name ;

	    subdname = envpops[i].sub1dname ;
	    if ((rs >= 0) && (subdname != NULL)) {

	        rs = mkpath2(pathbuf,pr,subdname) ;
	        pl = rs ;
	        if (rs >= 0)
	            rs = vecstr_add(&pathcomps,pathbuf,pl) ;

	    } /* end if */

	    subdname = envpops[i].sub2dname ;
	    if ((rs >= 0) && (subdname != NULL)) {

	        rs = mkpath2(pathbuf,pr,subdname) ;
	        pl = rs ;
	        if (rs >= 0)
	            rs = vecstr_add(&pathcomps,pathbuf,pl) ;

	    } /* end if */

	    if ((rs >= 0) && ((vp = getenv(np)) != NULL)) {
	        rs = vecstr_loadpath(&pathcomps,vp) ;
	    }

	    if (rs >= 0) {
	        size = vecstr_strsize(&pathcomps) ;
	    }

	    if ((rs >= 0) && ((rs = uc_malloc(size,&bp)) >= 0)) {

	        rs = mkpathval(&pathcomps,bp,(size-1)) ;
	        bl = rs ;
	        if (rs >= 0)
	            rs = vecstr_envadd(elp,np,bp,bl) ;

	        uc_free(bp) ;
	    } /* end if (memory allocation) */

	    vecstr_delall(&pathcomps) ;

	    if (rs < 0) break ;
	} /* end for */

	vecstr_finish(&pathcomps) ;
	} /* end uf (vecstr) */

	return rs ;
}
/* end subroutine (procenvpaths) */


static int mknodesvc(buf,buflen,node,svc)
char		buf[] ;
int		buflen ;
const char	node[] ;
const char	svc[] ;
{
	int	rs = SR_OK ;
	int	i = 0 ;


	if (node[0] != '\0') {
	    rs = storebuf_strw(buf,buflen,i,node,-1) ;
	    i += rs ;
	}

	if (rs >= 0) {
	    rs = storebuf_char(buf,buflen,i,'!') ;
	    i += rs ;
	}

	if (rs >= 0) {
	    rs = storebuf_strw(buf,buflen,i,svc,-1) ;
	    i += rs ;
	}

	return (rs >= 0) ? i : rs ;
}
/* end subroutine (mknodesvc) */


static int havenode(pr,node)
const char	pr[] ;
const char	node[] ;
{
	UUNAME	un ;

	int	rs = SR_OK ;

#if	CF_DEBUGS
	debugprintf("dialuux/havenode: pr=%s\n",pr) ;
	debugprintf("dialuux/havenode: node=%s\n",node) ;
#endif

	if (node[0] != '\0') {
	    if ((rs = uuname_open(&un,pr,"")) >= 0) {
	        rs = uuname_exists(&un,node,-1) ;
	        uuname_close(&un) ;
	    } /* end if (uuname) */
	} /* end if */

	return rs ;
}
/* end subroutine (havenode) */


