/* uc_openusersvc (user-open-service) */

/* interface component for UNIX® library-3c */
/* open a user-service */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */


/* revision history:

	= 1998-07-10, David A­D­ Morano

	This subroutine was originally written.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine opens what is referred to as a "user-service."
	User open-services are services provided by users.

	User openservices are represented in the filesystem as files
	with names of the form:

	<user>~<svc>[­<arg(s)>

	These sorts of file names are often actually stored in
	the filesystem as symbolic links.

	Synopsis:

	int uc_openusersvc(pr,svc,of,om,argv,envv,to)
	const char	pr[] ;
	const char	svc[] ;
	int		of ;
	mode_t		om ;
	const char	**argv[] ;
	const char	**envv[] ;
	int		to ;

	Arguments:

	pr		program-root
	svc		service name
	of		open-flags
	om		open-mode
	argv		array of arguments
	envv		attay of environment
	to		time-out

	Returns:

	<0		error
	>=0		file-descriptor

	User open-services are implemented with loadable shared-object
	files.	Each service has a file of the same name as the service
	name itself.  The file is a shared-object with a global symbol
	of a callable subroutine with the name 'open<svc>' where <svc>
	is the service name.  The subroutine looks like:

	int opensvc_<svc>(pr,of,om,argv,envv,to)
	const char	*pr ;
	int		of ;
	mode_t		om ;
	const char	*argv[] ;
	const char	*envv[] ;

	Multiple services can be actually implemented in the same
	shared-object.	But the actual file of that object should be
	linked to other files, each with the filename of a service to
	be implemented.  These links are required because this code only
	searches for services by searching for files with the names of
	the services.


*******************************************************************************/


#undef	LOCAL_SOLARIS
#define	LOCAL_SOLARIS	\
	(defined(OSNAME_SunOS) && (OSNAME_SunOS > 0))

#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<dlfcn.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>

#include	<vsystem.h>
#include	<udl.h>
#include	<ids.h>
#include	<localmisc.h>


/* local defines */

#define	SUBINFO		struct subinfo
#define	SUBINFO_FL	struct subinfo_flags

#define	SVCDNAME	"lib/opensvcs"
#define	SVCSYMPREFIX	"opensvc_"

#ifndef	LINEBUFLEN
#define	LINEBUFLEN	2048
#endif

#ifndef	HEBUFLEN
#define	HEBUFLEN	(8 * 1024)
#endif

#ifndef	SEBUFLEN
#define	SEBUFLEN	(1 * 1024)
#endif

#ifndef	PEBUFLEN
#define	PEBUFLEN	(1 * 1024)
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
extern int	matstr(const char **,const char *,int) ;
extern int	matcasestr(const char **,const char *,int) ;
extern int	sperm(IDS *,struct ustat *,int) ;

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

struct subinfo_flags {
	uint		dummy:1 ;
} ;

struct subinfo {
	const char	*pr ;
	const char	*svc ;
	const char	**argv ;
	const char	**envv ;
	const char	*svcdname ;	/* memory-allocated */
	const char	*svcsym ;
	struct subinfo_flags	f ;
	mode_t		om ;
	int		of ;
	int		to ;
} ;


/* forward references */

static int	subinfo_start(SUBINFO *,const char *,const char *,
			int,mode_t,const char **,const char **,int) ;
static int	subinfo_finish(SUBINFO *) ;
static int	subinfo_search(SUBINFO *) ;


/* local variables */

static const char	*soexts[] = {
	"so",
	"o",
	"",
	NULL
} ;


/* exported subroutines */


int uc_openusersvc(pr,svc,of,om,argv,envv,to)
const char	pr[] ;
const char	svc[] ;
int		of ;
mode_t		om ;
const char	*argv[] ;
const char	*envv[] ;
int		to ;
{
	SUBINFO		si ;
	int		rs ;
	int		rs1 ;
	int		fd = -1 ;

	if ((pr == NULL) || (svc == NULL))
	    return SR_FAULT ;

	if ((pr[0] == '\0') || (svc[0] == '\0'))
	    return SR_INVALID ;

#if	CF_DEBUGS
	debugprintf("uc_openusersvc: pr=%s\n",pr) ;
	debugprintf("uc_openusersvc: svc=%s\n",svc) ;
#endif

	if ((rs = subinfo_start(&si,pr,svc,of,om,argv,envv,to)) >= 0) {

	    rs = subinfo_search(&si) ;
	    fd = rs ;

	    rs1 = subinfo_finish(&si) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (subinfo) */

	if ((rs < 0) && (fd >= 0)) u_close(fd) ;

#if	CF_DEBUGS
	debugprintf("uc_openusersvc: ret rs=%d fd=%u\n",rs,fd) ;
#endif

	return (rs >= 0) ? fd : rs ;
}
/* end subroutine (uc_openusersvc) */


/* local subroutines */


static int subinfo_start(sip,pr,svc,of,om,argv,envv,to)
SUBINFO		*sip ;
const char	*pr ;
const char	*svc ;
int		of ;
mode_t		om ;
const char	**argv ;
const char	**envv ;
int		to ;
{
	struct ustat	sb ;
	int		rs ;
	int		pl ;
	const char	*ccp ;
	char		svcdname[MAXPATHLEN+1] ;

	memset(sip,0,sizeof(SUBINFO)) ;
	sip->pr = pr ;
	sip->svc = svc ;
	sip->argv = argv ;
	sip->envv = (envv != NULL) ? envv : ((const char **) environ) ;
	sip->of = of ;
	sip->om = om ;
	sip->to = to ;

/* make the directory where the service shared-objects are */

	rs = mkpath2(svcdname,pr,SVCDNAME) ;
	pl = rs ;
	if (rs < 0) goto bad0 ;

	rs = u_stat(svcdname,&sb) ;
	if ((rs >= 0) && (! S_ISDIR(sb.st_mode)))
	    rs = SR_NOTDIR ;

/* otherwise the 'stat' will return OK or mostly SR_NOENT */

	if (rs < 0) goto bad0 ;

	rs = uc_mallocstrw(svcdname,pl,&ccp) ;
	if (rs < 0) goto bad0 ;
	sip->svcdname = ccp ;

	{
	    char	svcsym[MAXNAMELEN+1] ;
	    int		sl ;
	    rs = sncpy2(svcsym,MAXNAMELEN,SVCSYMPREFIX,sip->svc) ;
	    sl = rs ;
	    if (rs >= 0) {
	        rs = uc_mallocstrw(svcsym,sl,&ccp) ;
	        if (rs >= 0) sip->svcsym = ccp ;
	    }
	}

	if (rs < 0) goto bad1 ;

ret0:
bad0:
	return rs ;

/* bad stuff */
bad1:
	if (sip->svcdname != NULL) {
	    uc_free(sip->svcdname) ;
	    sip->svcdname = NULL ;
	}

	goto bad0 ;
}
/* end subroutine (subinfo_start) */


static int subinfo_finish(sip)
SUBINFO		*sip ;
{


	if (sip->svcsym != NULL) {
	    uc_free(sip->svcsym) ;
	    sip->svcsym = NULL ;
	}

	if (sip->svcdname != NULL) {
	    uc_free(sip->svcdname) ;
	    sip->svcdname = NULL ;
	}

	return SR_OK ;
}
/* end subroutine (subinfo_finish) */


static int subinfo_search(sip)
SUBINFO		*sip ;
{
	struct ustat	sb ;

	IDS		id ;

	const int	dlmode = (RTLD_LAZY | RTLD_LOCAL) ;
	const int	soperm = (R_OK | X_OK) ;

	int	rs = SR_OK ;
	int	rs1 ;
	int	j ;
	int	fd = -1 ;
	int	(*symp)(const char *,int,mode_t,
			const char **,const char **,int) ;

	void	*sop = NULL ;

	char	sofname[MAXPATHLEN + 1] ;


#if	CF_DEBUGS
	    debugprintf("uc_openusersvc/subinfo_search: pr=%s\n",sip->pr) ;
	    debugprintf("uc_openusersvc/subinfo_search: svcsym=%s\n",
	                sip->svcsym) ;
#endif

	symp = NULL ;
	rs = ids_load(&id) ;
	if (rs < 0) goto ret0 ;

	rs1 = SR_NOENT ;
	for (j = 0 ; (soexts[j] != NULL) ; j += 1) {

#if	CF_DEBUGS
	    debugprintf("uc_openusersvc/subinfo_search: ext=%s\n",
	        soexts[j]) ;
#endif

	    rs1 = mksofname(sofname,sip->svcdname,sip->svc,soexts[j]) ;

#if	CF_DEBUGS
	    debugprintf("uc_openusersvc/subinfo_search: mksofname() rs=%d\n",rs1) ;
	    debugprintf("uc_openusersvc/subinfo_search: sofname=%s\n",sofname) ;
#endif

	    if (rs1 >= 0) {

	        rs1 = u_stat(sofname,&sb) ;
	        if ((rs1 >= 0) && (! S_ISREG(sb.st_mode)))
	            rs1 = SR_ISDIR ;

	        if (rs1 >= 0)
	            rs1 = sperm(&id,&sb,soperm) ;

#if	CF_DEBUGS
	        debugprintf("uc_openusersvc/subinfo_search: sperm() rs=%d\n",
	            rs1) ;
#endif

	        if (rs1 >= 0) {

	            sop = dlopen(sofname,dlmode) ;

#if	CF_DEBUGS
	            debugprintf("uc_openusersvc/subinfo_search: "
	                "dlopen() sop=%p >%s<\n",
	                sop,
	                ((sop != NULL) ? "ok" : dlerror() )) ;
#endif

	            if (sop != NULL) {
	                symp = dlsym(sop,sip->svcsym) ;

	                if (symp != NULL) {
	                    const char	*pr = sip->pr ;
	                    const char	**argv = sip->argv ;
	                    const char	**envv = sip->envv ;
			    mode_t	om = sip->om ;
	                    int		of = sip->of ;
	                    int		to = sip->to ;
#if	CF_DEBUGS
		{
			int	i ;
	            debugprintf("uc_openusersvc/subinfo_search: "
			"calling symp=%p\n",symp) ;
			if (argv != NULL) {
			for (i = 0 ; argv[i] != NULL; i += 1)
	            debugprintf("uc_openusersvc/subinfo_search: "
			"argv[%u]=%s\n",i,argv[i]) ;
			}
		}
#endif /* CF_DEBUGS */
	                    rs = (*symp)(pr,of,om,argv,envv,to) ;
	                    fd = rs ;
#if	CF_DEBUGS
	        debugprintf("uc_openusersvc/subinfo_search: sym() rs=%d\n",rs) ;
#endif
	                }

	                dlclose(sop) ;
	                sop = NULL ;
	            } else
	                rs1 = SR_NOENT ;

	        } /* end if (have file) */

	    } /* end if (created SO filename) */

#if	CF_DEBUGS
	        debugprintf("uc_openusersvc/subinfo_search: "
		"loopexit rs=%d symp=%p\n",rs,symp) ;
#endif

	    if (rs < 0)
	        break ;

	    if (symp != NULL)
	        break ;

	} /* end for (exts) */

	if (rs >= 0) rs = rs1 ;

	if ((rs >= 0) && (symp == NULL)) rs = SR_LIBACC ;

ret1:
	ids_release(&id) ;

ret0:

#if	CF_DEBUGS
	debugprintf("uc_openusersvc/subinfo_sochecklib: "
	    "ret rs=%d fd=%u\n",rs,fd) ;
#endif

	return (rs >= 0) ? fd : rs ;
}
/* end subroutine (subinfo_search) */


