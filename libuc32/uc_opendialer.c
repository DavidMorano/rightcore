/* uc_opendialer (open-dialer-service) */

/* interface component for UNIX® library-3c */
/* open a dialer */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */
#define	CF_GETEXECNAME	1		/* use 'getexecname(3c)' */


/* revision history:

	= 1998-07-10, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Dialer services are represented as files with names of the form:

	<dialer>¥<svc>[­<arg(s)>

	These sorts of file names are often actually stored in the
	filesystem as symbolic links.

	Synopsis:

	int uc_opendialer(prn,svc,of,om,argv,envv,to)
	const char	prn[] ;
	const char	svc[] ;
	int		of ;
	mode_t		om ;
	const char	**argv[] ;
	const char	**envv[] ;
	int		to ;

	Arguments:

	prn		facility name
	svc		service name
	of		open-flags
	om		open-mode
	argv		array of arguments
	envv		attay of environment
	to		time-out

	Returns:

	<0		error
	>=0		file-descriptor

        Dialer services are implemented with loadable shared-object files. Each
        service has a file of the same name as the service name itself. The file
        is a shared-object with a global symbol of a callable subroutine with
        the name 'opendialer_<svc>' where <svc> is the service name. The
        subroutine looks like:

	int opendialer_<svc>(pr,prn,of,om,argv,envv,to)
	const char	*pr ;
	const char	*prn ;
	int		of ;
	mode_t		om ;
	const char	*argv[] ;
	const char	*envv[] ;

        Multiple services can be actually implemented in the same shared-object.
        But the actual file of that object should be linked to other files, each
        with the filename of a service to be implemented. These links are
        required because this code only searches for services by searching for
        files with the names of the services.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<dlfcn.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<ids.h>
#include	<localmisc.h>


/* local defines */

#define	SUBINFO		struct subinfo
#define	SUBINFO_FL	struct subinfo_flags

#define	SVCDNAME	"lib/opendialers"
#define	SVCSYMPREFIX	"opendialer_"

#ifndef	LINEBUFLEN
#define	LINEBUFLEN	2048
#endif

#ifndef	SVCLEN
#define	SVCLEN		MAXNAMELEN
#endif


/* external subroutines */

extern int	snwcpy(char *,int,const char *,int) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	mkpath1w(char *,const char *,int) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mksofname(char *,const char *,const char *,const char *) ;
extern int	pathadd(char *,int,const char *) ;
extern int	matstr(const char **,const char *,int) ;
extern int	matcasestr(const char **,const char *,int) ;
extern int	sfdirname(const char *,int,const char **) ;
extern int	sfbasename(const char *,int,const char **) ;
extern int	sfprogroot(const char *,int,const char **) ;
extern int	sperm(IDS *,struct ustat *,int) ;
extern int	getpwd(char *,int) ;
extern int	getuserhome(char *,int,const char *) ;
extern int	getnodedomain(char *,char *) ;
extern int	mkpr(char *,int,const char *,const char *) ;
extern int	isNotPresent(int) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;
extern char	*strnpbrk(const char *,int,const char *) ;
extern char	*strdcpy1(char *,int,const char *) ;
extern char	*strdcpy1w(char *,int,const char *,int) ;
extern char	*strdcpy2w(char *,int,const char *,const char *,int) ;


/* external variables */

extern char	**environ ;


/* local structures */

#ifndef	TYPEDEF_CCHAR
#define	TYPEDEF_CCHAR	1
typedef const char	cchar ;
#endif

typedef int (*subdialer_t)(const char *,const char *,const char *,
	    int,mode_t,const char **,const char **,int) ;

struct subinfo_flags {
	uint		dummy:1 ;
} ;

struct subinfo {
	IDS		id ;
	SUBINFO_FL	f ;
	const char	*prn ;
	const char	*svc ;
	const char	*dialsym ;	/* memory-allocated */
	const char	**argv ;
	const char	**envv ;
	mode_t		om ;
	int		of ;
	int		to ;
	int		fd ;
} ;


/* forward references */

static int	subinfo_start(SUBINFO *,cchar *,cchar *,
			int,mode_t,cchar **,cchar **,int) ;
static int	subinfo_finish(SUBINFO *) ;
static int	subinfo_search(SUBINFO *) ;
static int	subinfo_exts(SUBINFO *,cchar *,cchar *,char *) ;
static int	subinfo_searchlib(SUBINFO *,cchar *,cchar *) ;
static int	subinfo_searchcall(SUBINFO *,cchar *,subdialer_t) ;
static int	subinfo_idbegin(SUBINFO *) ;
static int	subinfo_idend(SUBINFO *) ;


/* local variables */

static const char	*prns[] = {
	"extra",
	"preroot",
	NULL
} ;

static const char	*soexts[] = {
	"so",
	"o",
	"",
	NULL
} ;


/* exported subroutines */


int uc_opendialer(prn,svc,of,om,argv,envv,to)
const char	prn[] ;
const char	svc[] ;
int		of ;
mode_t		om ;
const char	*argv[] ;
const char	*envv[] ;
int		to ;
{
	SUBINFO		si, *sip = &si ;
	int		rs ;
	int		rs1 ;
	int		fd = -1 ;

#if	CF_DEBUGS
	debugprintf("uc_opendialer: ent svc=%s to=%d\n",svc,to) ;
#endif

	if ((prn == NULL) && (svc == NULL)) return SR_FAULT ;

	if ((prn[0] == '\0') && (svc[0] == '\0')) return SR_INVALID ;

#if	CF_DEBUGS
	debugprintf("uc_opendialer: svc=%s\n",svc) ;
	if (argv != NULL) {
	    int	i ;
	    for (i = 0 ; argv[i] != NULL ; i += 1) {
	        debugprintf("uc_opendialer: a[%u]=%s\n",i,argv[i]) ;
	    }
	}
#endif /* CF_DEBUGS */

	if ((rs = subinfo_start(&si,prn,svc,of,om,argv,envv,to)) >= 0) {
	    if ((rs = subinfo_search(&si)) > 0) { /* >0 means found */
		fd = sip->fd ;
	    } else if (rs == 0) {
		rs = SR_NOENT ;
	    }
	    rs1 = subinfo_finish(&si) ;
	    if (rs >= 0) rs = rs1 ;
	    if ((rs < 0) && (fd >= 0)) u_close(fd) ;
	} /* end if (subinfo) */

#if	CF_DEBUGS
	debugprintf("uc_opendialer: ret rs=%d fd=%u\n",rs,fd) ;
	debugprintf("uc_opendialer: ret svc=%s\n",svc) ;
#endif

	return (rs >= 0) ? fd : rs ;
}
/* end subroutine (uc_opendialer) */


/* local subroutines */


static int subinfo_start(sip,prn,svc,of,om,argv,envv,to)
SUBINFO		*sip ;
const char	*prn ;
const char	*svc ;
int		of ;
mode_t		om ;
const char	**argv ;
const char	**envv ;
int		to ;
{
	int		rs = SR_OK ;

	memset(sip,0,sizeof(SUBINFO)) ;
	sip->prn = prn ;
	sip->svc = svc ;
	sip->argv = argv ;
	sip->envv = (envv != NULL) ? envv : ((const char **) environ) ;
	sip->of = of ;
	sip->om = om ;
	sip->to = to ;
	sip->fd = -1 ;

	{
	    const char	*prefix = SVCSYMPREFIX ;
	    char	dialsym[MAXNAMELEN+1] ;
	    if ((rs = sncpy2(dialsym,MAXNAMELEN,prefix,sip->prn)) >= 0) {
	        const char	*ccp ;
	        if ((rs = uc_libmallocstrw(dialsym,rs,&ccp)) >= 0) {
	            sip->dialsym = ccp ;
		}
	    }
	}

	return rs ;
}
/* end subroutine (subinfo_start) */


static int subinfo_finish(SUBINFO *sip)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (sip->dialsym != NULL) {
	    rs1 = uc_libfree(sip->dialsym) ;
	    if (rs >= 0) rs = rs1 ;
	    sip->dialsym = NULL ;
	}

	return rs ;
}
/* end subroutine (subinfo_finish) */


static int subinfo_search(SUBINFO *sip)
{
	const int	plen = MAXPATHLEN ;
	int		rs ;
	int		rs1 ;
	int		f = FALSE ;

	if ((rs = subinfo_idbegin(sip)) >= 0) {
	    const int	size = ((plen+1)*3) ;
	    char	*abuf ;
	    if ((rs = uc_libmalloc(size,&abuf)) >= 0) {
	        char	dn[MAXHOSTNAMELEN+1] ;
	        char	*pdn = (abuf+(0*(plen+1))) ;
	        char	*sdn = (abuf+(1*(plen+1))) ;
	        char	*sfn = (abuf+(2*(plen+1))) ;
	        if ((rs = getnodedomain(NULL,dn)) >= 0) {
		    struct ustat	sb ;
		    int			i = 0 ;

	            for (i = 0 ; prns[i] != NULL ; i += 1) {
	                if ((rs = mkpr(pdn,plen,prns[i],dn)) > 0) {
		            if ((rs = mkpath2(sdn,pdn,SVCDNAME)) >= 0) {
	        		if ((rs = u_stat(sdn,&sb)) >= 0) {
				    if (S_ISDIR(sb.st_mode)) {
	                                rs = subinfo_exts(sip,pdn,sdn,sfn) ;
	                                f = rs ;
				    } /* end if */
				} else if (isNotPresent(rs)) {
		    		    rs = SR_OK ;
				}
			    } /* end if (mkpath) */
	                } /* end if (mkpr) */
			if (f) break ;
			if (rs < 0) break ;
	            } /* end for (prns) */

#if	CF_DEBUGS
		    debugprintf("uc_opendialer/_search: for-out rs=%d f=%u\n",
		        rs,f) ;
#endif
	        } /* end if */
	        rs1 = uc_libfree(abuf) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (ma-a) */
	    rs1 = subinfo_idend(sip) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (subinfo-id) */

	if ((rs < 0) && (sip->fd >= 0)) {
	    u_close(sip->fd) ;
	    sip->fd = -1 ;
	}

#if	CF_DEBUGS
	debugprintf("uc_opendialer/_search: ret rs=%d f=%u\n",rs,f) ;
#endif
	return (rs >= 0) ? f : rs ;
}
/* end subroutine (subinfo_search) */


static int subinfo_exts(SUBINFO *sip,cchar *pr,cchar *sdn,char *sfn)
{
	struct ustat	sb ;
	const int	am = (R_OK|X_OK) ;
	int		rs = SR_OK ;
	int		i ;
	int		f = FALSE ;
	const char	*prn = sip->prn ;
	for (i = 0 ; soexts[i] != NULL ; i += 1) {
	    if ((rs = mksofname(sfn,sdn,prn,soexts[i])) >= 0) {
	        if ((rs = u_stat(sfn,&sb)) >= 0) {
		    if (S_ISREG(sb.st_mode)) {
	                if ((rs = sperm(&sip->id,&sb,am)) >= 0) {
			    rs = subinfo_searchlib(sip,pr,sfn) ;
			    f = rs ;
			} else if (rs == SR_ACCESS) {
			    rs = SR_OK ;
			}
		    } /* end if (regular file) */
		} else if (isNotPresent(rs)) {
		    rs = SR_OK ;
		}
	    } /* end if (mksofname) */
	    if (f) break ;
	    if (rs < 0) break ;
	} /* end for (soexts) */
#if	CF_DEBUGS
	debugprintf("uc_opendialer/_exts: ret rs=%d f=%u\n",rs,f) ;
#endif
	return (rs >= 0) ? f : rs ;
}
/* end subroutine (subinfo_exts) */


static int subinfo_searchlib(SUBINFO *sip,cchar *pr,cchar *sfn)
{
	const int	dlmode = RTLD_LAZY ;
	int		rs = SR_OK ;
	int		f = FALSE ;
	void		*sop ;
#if	CF_DEBUGS
	debugprintf("uc_opendialer/_searchlib: sfn=%s\n",sfn) ;
#endif
	if ((sop = dlopen(sfn,dlmode)) != NULL) {
	    subdialer_t	symp ;
	    if ((symp = (subdialer_t) dlsym(sop,sip->dialsym)) != NULL) {
		rs = subinfo_searchcall(sip,pr,symp) ;
		f = rs ;
	    } /* end if (dlsym) */
	    dlclose(sop) ;
	} /* end if (dlopen) */
#if	CF_DEBUGS
	debugprintf("uc_opendialer/_searchlib: ret rs=%d f=%u\n",rs,f) ;
#endif
	return (rs >= 0) ? f : rs ;
}
/* end subroutine (subinfo_searchlib) */


static int subinfo_searchcall(SUBINFO *sip,cchar *pr,subdialer_t symp)
{
	mode_t		om = sip->om ;
	int		rs ;
	int		of = sip->of ;
	int		to = sip->to ;
	int		f = FALSE ;
	const char	*prn = sip->prn ;
	const char	*svc = sip->svc ;
	const char	**argv = sip->argv ;
	const char	**envv = sip->envv ;

#if	CF_DEBUGS
	debugprintf("uc_opendialer/_searchcall: ent\n") ;
	debugprintf("uc_opendialer/_searchcall: pr=%s\n",pr) ;
	debugprintf("uc_opendialer/_searchcall: prn=%s\n",prn) ;
	debugprintf("uc_opendialer/_searchcall: svc=%s\n",svc) ;
	{
	    int	i ;
	    for (i = 0 ; argv[i] != NULL ; i += 1) {
		debugprintf("uc_opendialer/_searchcall: a[%d]=%s\n",
			i,argv[i]) ;
	    }
	}
#endif /* CF_DEBUGS */

	if ((rs = (*symp)(pr,prn,svc,of,om,argv,envv,to)) >= 0) {
	    sip->fd = rs ;
	    f = TRUE ;
	} /* end if (call) */

#if	CF_DEBUGS
	debugprintf("uc_opendialer/_searchcall: ret rs=%d f=%u\n",rs,f) ;
#endif

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (subinfo_searchcall) */


static int subinfo_idbegin(SUBINFO *sip)
{
	int		rs = ids_load(&sip->id) ;
	return rs ;
}
/* end subroutine (subinfo_idbegin) */


static int subinfo_idend(SUBINFO *sip)
{
	int		rs = SR_OK ;
	int		rs1 ;

	rs1 = ids_release(&sip->id) ;
	if (rs >= 0) rs = rs1 ;

	return rs ;
}
/* end subroutine (subinfo_idend) */


