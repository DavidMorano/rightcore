/* uc_openfsvc (open-facility-service) */

/* interface component for UNIX® library-3c */
/* open a facility-service */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */
#define	CF_DEBUGN	0		/* extra debugging */


/* revision history:

	= 1998-07-10, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine opens what is referred to as a "facility-service."
	Facility services are services provided by "facilities." Facilities are
	normally or usually really a software distribution that has its own
	source-root or program-root.  Examples of software distributions within
	AT&T are:

	NCMP
	PCS (Personal Communication Services)
	EXPTOOLS (Experimental Tools)
	LOCAL
	REXEC (Remote Execution)
	POST
	EXTRA
	AST (Advanced Software Technologies)
	GNU

	Facilities are stored in the filesystem hierarchy rooted at a certain
	place.  Facilities are usually rooted at one of the following locations
	in the system filesystem hierarchy:

	/usr
	/usr/add-on
	/opt

	Facility services are represented in the filesystem as files with names
	of the form:

	<facility>§<svc>[­<arg(s)>

	These sorts of file names are often actually stored in the filesystem
	as symbolic links.

	Synopsis:

	int uc_openfsvc(pr,prn,svc,of,om,argv,envv,to)
	const char	pr[] ;
	const char	prn[] ;
	const char	svc[] ;
	int		of ;
	mode_t		om ;
	const char	**argv[] ;
	const char	**envv[] ;
	int		to ;

	Arguments:

	pr		program-root
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

	Facility services are implemented with loadable shared-object files.
	Each service has a file of the same name as the service name itself.
	The file is a shared-object with a global symbol of a callable
	subroutine with the name 'opensvc_<svc>' where <svc> is the service
	name.  The subroutine looks like:

	int opensvc_<svc>(pr,prn,of,om,argv,envv,to)
	const char	*pr ;
	const char	*prn ;
	int		of ;
	mode_t		om ;
	const char	*argv[] ;
	const char	*envv[] ;

	Multiple services can be actually implemented in the same
	shared-object.  But the actual file of that object should be linked to
	other files, each with the filename of a service to be implemented.
	These links are required because this code only searches for services
	by searching for files with the names of the services.


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

#define	SVCDNAME	"lib/opensvcs"
#define	SVCSYMPREFIX	"opensvc_"

#ifndef	LINEBUFLEN
#define	LINEBUFLEN	2048
#endif

#ifndef	SVCLEN
#define	SVCLEN		MAXNAMELEN
#endif

#undef	SR_LIBISNOTLOCAL
#define	SR_LIBISNOTLOCAL	SR_LIBSCN

#define	NDF	"openfsvc.deb"


/* external subroutines */

extern int	snwcpy(char *,int,const char *,int) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	mkpath1w(char *,const char *,int) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mksofname(char *,const char *,const char *,const char *) ;
extern int	matstr(const char **,const char *,int) ;
extern int	matcasestr(const char **,const char *,int) ;
extern int	sperm(IDS *,struct ustat *,int) ;
extern int	isNotPresent(int) ;
extern int	isOneOf(const int *,int) ;

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

extern const char	**environ ;


/* local structures */

#ifndef	TYPEDEF_CCHAR
#define	TYPEDEF_CCHAR	1
typedef const char	cchar ;
#endif

typedef int (*subsvc_t)(const char *,const char *,
	    int,mode_t,const char **,const char **,int) ;

struct subinfo_flags {
	uint		dummy:1 ;
} ;

struct subinfo {
	IDS		id ;
	SUBINFO_FL	f ;
	const char	*pr ;
	const char	*prn ;
	const char	*svc ;
	const char	*svcdname ;	/* memory-allocated */
	const char	*dialsym ;	/* memory-allocated */
	const char	**argv ;
	const char	**envv ;
	mode_t		om ;
	int		of ;
	int		to ;
	int		fd ;
} ;


/* forward references */

static int	subinfo_start(SUBINFO *,cchar *,cchar *,cchar *,
			int,mode_t,cchar **,cchar **,int) ;
static int	subinfo_finish(SUBINFO *) ;
static int	subinfo_search(SUBINFO *) ;
static int	subinfo_exts(SUBINFO *,cchar *,char *) ;
static int	subinfo_searchlib(SUBINFO *,cchar *) ;
static int	subinfo_searchcall(SUBINFO *,subsvc_t) ;
static int	subinfo_idbegin(SUBINFO *) ;
static int	subinfo_idend(SUBINFO *) ;

static int	isNoAcc(int) ;


/* local variables */

static const char	*soexts[] = {
	"so",
	"o",
	"",
	NULL
} ;

static const int	rsnoacc[] = {
	SR_NOENT,
	SR_ACCESS,
	0
} ;


/* exported subroutines */


int uc_openfsvc(pr,prn,svc,of,om,argv,envv,to)
const char	pr[] ;
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

	if ((pr == NULL) || (prn == NULL) || (svc == NULL))
	    return SR_FAULT ;

	if ((pr[0] == '\0') || (prn[0] == '\0') || (svc[0] == '\0'))
	    return SR_INVALID ;

#if	CF_DEBUGS
	debugprintf("uc_openfsvc: ent\n") ;
	debugprintf("uc_openfsvc: pr=%s\n",pr) ;
	debugprintf("uc_openfsvc: prn=%s\n",prn) ;
	debugprintf("uc_openfsvc: svc=%s\n",svc) ;
#endif

#if	CF_DEBUGN
	nprintf(NDF,"uc_openfsvc: ent svc=%s\n",svc) ;
#endif

	if ((rs = subinfo_start(&si,pr,prn,svc,of,om,argv,envv,to)) >= 0) {

	    rs = subinfo_search(&si) ;

#if	CF_DEBUGN
	nprintf(NDF,"uc_openfsvc: subinfo_search() rs=%d\n",rs) ;
#endif

	    if (rs > 0) {
		fd = sip->fd ;
	    } else if (rs == 0)
		rs = SR_NOENT ;

	    rs1 = subinfo_finish(&si) ;
	    if (rs >= 0) rs = rs1 ;
	    if ((rs < 0) && (fd >= 0)) u_close(fd) ;
	} /* end if (subinfo) */

#if	CF_DEBUGS
	debugprintf("uc_openfsvc: ret rs=%d fd=%d\n",rs,fd) ;
#endif
#if	CF_DEBUGN
	nprintf(NDF,"uc_openfsvc: ret rs=%d fd=%d\n",rs,fd) ;
#endif

	return (rs >= 0) ? fd : rs ;
}
/* end subroutine (uc_openfsvc) */


/* local subroutines */


static int subinfo_start(sip,pr,prn,svc,of,om,argv,envv,to)
SUBINFO		*sip ;
const char	*pr ;
const char	*prn ;
const char	*svc ;
int		of ;
mode_t		om ;
const char	**argv ;
const char	**envv ;
int		to ;
{
	int		rs ;
	char		svcdname[MAXPATHLEN+1] ;

	memset(sip,0,sizeof(SUBINFO)) ;
	sip->pr = pr ;
	sip->prn = prn ;
	sip->svc = svc ;
	sip->argv = argv ;
	sip->envv = (envv != NULL) ? envv : ((const char **) environ) ;
	sip->of = of ;
	sip->om = om ;
	sip->to = to ;
	sip->fd = -1 ;

/* make the directory where the service shared-objects are */

	if ((rs = mkpath2(svcdname,pr,SVCDNAME)) >= 0) {
	    struct ustat	sb ;
	    int			pl = rs ;
	    if ((rs = u_stat(svcdname,&sb)) >= 0) {
		if (S_ISDIR(sb.st_mode)) {
		    const char	*ccp ;
		    if ((rs = uc_libmallocstrw(svcdname,pl,&ccp)) >= 0) {
	    		char		dialsym[MAXNAMELEN+1] ;
	    		const int	symlen = MAXNAMELEN ;
	    		const char	*prefix = SVCSYMPREFIX ;
			const char	*svc = sip->svc ;
			sip->svcdname = ccp ;
	    		if ((rs = sncpy2(dialsym,symlen,prefix,svc)) >= 0) {
	        	    if ((rs = uc_libmallocstrw(dialsym,rs,&ccp)) >= 0) {
	            		sip->dialsym = ccp ;
			    } /* end if (memory-allocation) */
	    		}
	    		if (rs < 0) {
			    uc_libfree(sip->svcdname) ;
			    sip->svcdname = NULL ;
	    		}
		    } /* end if (memory-allocation) */
		} else
	    	    rs = SR_NOTDIR ;
	    } /* end if (stat) */
	} /* end if (mkpath) */

	return rs ;
}
/* end subroutine (subinfo_start) */


static int subinfo_finish(sip)
SUBINFO		*sip ;
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (sip->dialsym != NULL) {
	    rs1 = uc_libfree(sip->dialsym) ;
	    if (rs >= 0) rs = rs1 ;
	    sip->dialsym = NULL ;
	}

	if (sip->svcdname != NULL) {
	    rs1 = uc_libfree(sip->svcdname) ;
	    if (rs >= 0) rs = rs1 ;
	    sip->svcdname = NULL ;
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
	    const int	size = ((plen+1)*2) ;
	    char	*abuf ;
	    if ((rs = uc_libmalloc(size,&abuf)) >= 0) {
		const char	*pr = sip->pr ;
	        char		*sdn = (abuf+(0*(plen+1))) ;
	        char		*sfn = (abuf+(1*(plen+1))) ;

		if ((rs = mkpath2(sdn,pr,SVCDNAME)) >= 0) {
		    struct ustat	sb ;
		    if ((rs = u_stat(sdn,&sb)) >= 0) {
			if (S_ISDIR(sb.st_mode)) {
			    rs = subinfo_exts(sip,sdn,sfn) ;
			    f = rs ;
			} /* end if */
		    } else if (isNoAcc(rs))
			rs = SR_OK ;
		} /* end if (mkpath) */

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
	debugprintf("uc_openfsvc/_search: ret rs=%d f=%u\n",rs,f) ;
#endif
	return (rs >= 0) ? f : rs ;
}
/* end subroutine (subinfo_search) */


static int subinfo_exts(SUBINFO *sip,cchar *sdn,char *sfn)
{
	struct ustat	sb ;
	const int	am = (R_OK|X_OK) ;
	int		rs = SR_OK ;
	int		i ;
	int		f = FALSE ;
	const char	*svc = sip->svc ;
	for (i = 0 ; soexts[i] != NULL ; i += 1) {
	    if ((rs = mksofname(sfn,sdn,svc,soexts[i])) >= 0) {
#if	CF_DEBUGS
	debugprintf("uc_openfsvc/_exts: sfn=%s\n",sfn) ;
#endif
	        if ((rs = u_stat(sfn,&sb)) >= 0) {
		    if (S_ISREG(sb.st_mode)) {
	                if ((rs = sperm(&sip->id,&sb,am)) >= 0) {
			    rs = subinfo_searchlib(sip,sfn) ;
			    f = rs ;
			} else if (isNoAcc(rs))
			    rs = SR_OK ;
		    } /* end if (regular file) */
		} else if (isNotPresent(rs))
		    rs = SR_OK ;
	    } /* end if (mksofname) */
	    if (f) break ;
	    if (rs < 0) break ;
	} /* end for (soexts) */
	return (rs >= 0) ? f : rs ;
}
/* end subroutine (subinfo_exts) */


static int subinfo_searchlib(SUBINFO *sip,cchar *sfn)
{
	const int	dlmode = RTLD_LAZY ;
	int		rs = SR_OK ;
	int		f = FALSE ;
	void		*sop ;
#if	CF_DEBUGS
	debugprintf("uc_openfsvc/_searchlib: sfn=%s\n",sfn) ;
#endif
	if ((sop = dlopen(sfn,dlmode)) != NULL) {
	    subsvc_t	symp ;
	    if ((symp = (subsvc_t) dlsym(sop,sip->dialsym)) != NULL) {
		rs = subinfo_searchcall(sip,symp) ;
		f = rs ;
	    } /* end if (dlsym) */
	    dlclose(sop) ;
	} /* end if (dlopen) */
#if	CF_DEBUGS
	debugprintf("uc_openfsvc/_searchlib: ret rs=%d f=%u\n",rs,f) ;
#endif
	return (rs >= 0) ? f : rs ;
}
/* end subroutine (subinfo_searchlib) */


static int subinfo_searchcall(SUBINFO *sip,subsvc_t symp)
{
	const mode_t	om = sip->om ;
	const int	of = sip->of ;
	const int	to = sip->to ;
	int		rs ;
	int		f = FALSE ;
	cchar		*pr = sip->pr ;
	const char	*prn = sip->prn ;
	const char	**argv = sip->argv ;
	const char	**envv = sip->envv ;

	if ((rs = (*symp)(pr,prn,of,om,argv,envv,to)) >= 0) {
	    sip->fd = rs ;
	    f = TRUE ;
	} /* end if (call) */

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


static int isNoAcc(int rs)
{
	return isOneOf(rsnoacc,rs) ;
}
/* end subroutine (isNoAcc) */


