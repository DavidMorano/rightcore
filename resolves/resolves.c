/* resolves */

/* object to help (manage) RESOLVES messages */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */
#define	CF_DEBUGN	0		/* extra-special debugging */
#define	CF_WRITETO	1		/* time out writes */
#define	CF_PARAMFILE	1		/* use 'paramfile(3dam)' */
#define	CF_TESTPROC	0		/* test using 'uc_openfsvc(3uc)' */
#define	CF_FINDUID	1		/* use 'finduid(3c)' */


/* revision history:

	= 2003-10-01, David A­D­ Morano

	This is a hack from numerous previous hacks (not enumerated here).
	This is a new version of this hack that is entirely different
	(much simpler).


*/

/* Copyright © 2003 David A­D­ Morano.  All rights reserved. */

/**************************************************************************

	This object module writes the contents of various RESOLVESs (as
	specified by the caller) to an open file descriptor (also
	specified by the caller).

	Implementation notes:

	When processing, we time-out writes to the caller-supplied
	file-descriptor because we don't know if it is a non-regular file
	that might be flow-controlled.	We don't wait forever for those
	sorts of outputs.  So let's say that the output is a terminal
	that is currently flow-controlled.  We will time-out on our
	writes and the user will not get this whole RESOLVES text!

	The FINDUID feature:

	This has got to be a feature that has more code than is ever
	executed of all features.  This feature handles an extremely small
	corner case where there are two or more USERNAMEs sharing a single
	UID (in the system PASSWD database).  Further, the code comes into
	play when one of the users is already logged in and one of the
	other users sharing the same UID goes to log in.  Without this
	code a random username among those sharing the same UID would be
	selected for the new user logging in.  The reason for this is
	that in daemon mode we only get UIDs back from the kernel on a
	connection request.  So we have to guess what the corresponding
	username might be for that connection request.	With the FINDUID
	feature, we do this guessing a little bit more intelligently by
	using the username from that last user with the given UID who
	logged into the system (by searching the system UTMPX database).
	The latest user logged in will get his own username (within a
	few split seconds or so) but a consequence is that all other
	users sharing that same UID will also see this same username.
	But this is not usually a big problem since the read-out of the
	RESOLVES file is usually done at login time and often only done
	at that time.  Outside of daemon mode, or stand-alone mode,
	the feature does not come into play and the correct username
	(within extremely broad limits) is always divined.  So there it
	is, good and bad.  But there are not a lot of ways to handle it
	better and this feature already handles these cases much better
	than nothing at all.


*****************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<limits.h>
#include	<signal.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>

#include	<vsystem.h>
#include	<estrings.h>
#include	<ids.h>
#include	<vecstr.h>
#include	<vechand.h>
#include	<getax.h>
#include	<getxusername.h>
#include	<ptma.h>
#include	<ptm.h>
#include	<lockrw.h>
#include	<paramfile.h>
#include	<strpack.h>
#include	<bfile.h>
#include	<localmisc.h>

#include	"resolves.h"


/* local defines */

#define	RESOLVES_MAGIC	0x75648941
#define	RESOLVES_MAPDIR	struct resolves_mapdir
#define	RESOLVES_DEFGROUP	"default"
#define	RESOLVES_ALLGROUP	"all"
#define	RESOLVES_NAME	"resolves"
#define	RESOLVES_DIRSFNAME	"dirs"

#define	RESOLVES_MAPPERMAGIC	0x21367425

#define	NDEBFNAME	"resolves.deb"

#ifndef	MAXNAMELEN
#define	MAXNAMELEN	256
#endif

#ifndef	MSGBUFLEN
#define	MSGBUFLEN	2048
#endif

#ifndef	LINEBUFLEN
#ifdef	LINE_MAX
#define	LINEBUFLEN	MAX(LINE_MAX,2048)
#else
#define	LINEBUFLEN	2048
#endif
#endif

#define	ADMINLEN	USERNAMELEN

#undef	ENVBUFLEN
#define	ENVBUFLEN	(10 + MAX(ADMINLEN,MAXPATHLEN))

#ifndef	DIGBUFLEN
#define	DIGBUFLEN	40		/* can hold int128_t in decimal */
#endif

#undef	BUFLEN
#define	BUFLEN		2048		/* enough for one typical read? */

#ifndef	POLLMULT
#define	POLLMULT	1000
#endif

#define	PBUFLEN		MAXPATHLEN

#undef	TO_POLL
#define	TO_POLL		5

#define	TO_LOCK		30
#define	TO_OPEN		10
#define	TO_READ		20
#define	TO_WRITE	50
#define	TO_CHECK	5		/* object checking */
#define	TO_MAPCHECK	10		/* mapper checking */
#define	TO_FILEAGE	5		/* directory map-file age */

#undef	NLPS
#define	NLPS		2		/* number ? polls per second */


/* external subroutines */

extern int	snsds(char *,int,const char *,const char *) ;
extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	sncpy3(char *,int,const char *,const char *,const char *) ;
extern int	sncpy4(char *,int,const char *,const char *,cchar *,cchar *) ;
extern int	sncpylc(char *,int,const char *) ;
extern int	sncpyuc(char *,int,const char *) ;
extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	mknpath1(char *,int,const char *) ;
extern int	mknpath2(char *,int,const char *,const char *) ;
extern int	mknpath3(char *,int,const char *,const char *,const char *) ;
extern int	matstr(const char **,const char *,int) ;
extern int	nleadstr(const char *,const char *,int) ;
extern int	sperm(IDS *,struct ustat *,int) ;
extern int	permsched(const char **,vecstr *,char *,int,const char *,int) ;
extern int	getnodedomain(char *,char *) ;
extern int	getgroup_gid(cchar *,int) ;
extern int	getuserhome(char *,int,cchar *) ;
extern int	ctdecui(char *,int,uint) ;
extern int	vecstr_envset(vecstr *,const char *,const char *,int) ;
extern int	msleep(int) ;
extern int	haslc(const char *,int) ;
extern int	hasuc(const char *,int) ;
extern int	isNotPresent(int) ;

#if	CF_DEBUGS || CF_DEBUGN
extern int	nprintf(const char *,...) ;
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;
extern char	*strdcpy1(char *,int,const char *) ;
extern char	*strdcpy2(char *,int,const char *,const char *) ;
extern char	*strdcpy3(char *,int,const char *,const char *,const char *) ;
extern char	*strdcpy4(char *,int,const char *,const char *,
			const char *,const char *) ;
extern char	*strdcpy1w(char *,int,const char *,int) ;


/* external variables */

extern char	**environ ;


/* local structures */

struct resolves_mapdir {
	LOCKRW		rwm ;
	const char	*admin ;
	const char	*dirname ;	/* raw */
	const char	*dname ;	/* expanded */
} ;


/* forward references */

int 		resolves_procid(RESOLVES *,RESOLVES_ID *,
			const char **,int) ;

static int	resolves_mapfind(RESOLVES *,time_t) ;
static int	resolves_maplose(RESOLVES *) ;
static int	resolves_mapfname(RESOLVES *,char *) ;
static int	resolves_schedload(RESOLVES *,vecstr *) ;
static int	resolves_checker(RESOLVES *,time_t) ;
static int	resolves_envbegin(RESOLVES *) ;
static int	resolves_envend(RESOLVES *) ;
static int	resolves_envadds(RESOLVES *,STRPACK *,
			const char **,RESOLVES_ID *) ;
static int	resolves_envstore(RESOLVES *,STRPACK *,
			const char **,int, const char *,int) ;
static int 	resolves_processor(RESOLVES *,const char **,
			const char **, const char *,int) ;
static int	resolves_idcheck(RESOLVES *,RESOLVES_ID *,char *) ;
static int	resolves_ufindstart(RESOLVES *) ;
static int	resolves_ufindfinish(RESOLVES *) ;
static int	resolves_ufindlook(RESOLVES *,char *,uid_t) ;

static int	mapper_start(RESOLVES_MAPPER *,time_t,const char *) ;
static int	mapper_finish(RESOLVES_MAPPER *) ;
static int	mapper_check(RESOLVES_MAPPER *,time_t) ;
static int	mapper_process(RESOLVES_MAPPER *,const char **,
			const char **, const char *,int) ;
static int	mapper_processor(RESOLVES_MAPPER *,const char **,
			const char **, const char *,int) ;
static int	mapper_mapload(RESOLVES_MAPPER *) ;
static int	mapper_mapadd(RESOLVES_MAPPER *,const char *,int,
			const char *,int) ;
static int	mapper_mapfrees(RESOLVES_MAPPER *) ;

#if	CF_TESTPROC
static int	mapper_lockcheck(RESOLVES_MAPPER *,const char *) ;
#endif

static int	mapdir_start(RESOLVES_MAPDIR *,const char *,int,
			const char *,int) ;
static int	mapdir_finish(RESOLVES_MAPDIR *) ;
static int	mapdir_process(RESOLVES_MAPDIR *,const char **,
			const char **, const char *,int) ;
static int	mapdir_expand(RESOLVES_MAPDIR *) ;
static int	mapdir_expander(RESOLVES_MAPDIR *) ;
static int	mapdir_processor(RESOLVES_MAPDIR *,const char **,
			const char *,int) ;
static int	mapdir_procout(RESOLVES_MAPDIR *,const char **,
			const char *, const char *,int) ;
static int	mapdir_procouter(RESOLVES_MAPDIR *,const char **,
			const char *,int) ;

static int	writeto(int,const char *,int,int) ;


/* local variables */

static const char	*schedmaps[] = {
	"%p/%e/%n/%n.%f",
	"%p/%e/%n/%f",
	"%p/%e/%n.%f",
	"%p/%n.%f",
	"%n.%f",
	NULL
} ;

static const char	*envbad[] = {
	"TMOUT",
	"A__z",
	NULL

} ;

static const char	*envstrs[] = {
	"USERNAME",
	"GROUPNAME",
	"UID",
	"GID",
	"ADMIN",
	"ADMINDIR",
	NULL
} ;

enum envstrs {
	envstr_username,
	envstr_groupname,
	envstr_uid,
	envstr_gid,
	envstr_admin,
	envstr_admindir,
	envstr_overlast
} ;

const static char	*envpre = "RESOLVES_" ;	/* environment prefix */


/* exported subroutines */


int resolves_open(op,pr)
RESOLVES		*op ;
const char	pr[] ;
{
	time_t	daytime = time(NULL) ;

	int	rs ;

	const char	*cp ;


	if (op == NULL)
	    return SR_FAULT ;

	if (pr == NULL)
	    return SR_FAULT ;

	if (pr[0] == '\0')
	    return SR_INVALID ;

#if	CF_DEBUGS
	debugprintf("resolves_open: sizeof(RESOLVES)=%u\n",sizeof(RESOLVES)) ;
#endif

	memset(op,0,sizeof(RESOLVES)) ;
	op->fe = RESOLVES_DIRSFNAME ;

	rs = uc_mallocstrw(pr,-1,&cp) ;
	if (rs < 0) goto bad0 ;
	op->pr = cp ;

	rs = ptm_init(&op->m,NULL) ;
	if ( rs < 0) goto bad1 ;

	rs = resolves_mapfind(op,daytime) ;
	if (rs < 0) goto bad2 ;

	rs = resolves_envbegin(op) ;
	if (rs < 0) goto bad3 ;

	op->ti_lastcheck = daytime ;
	op->magic = RESOLVES_MAGIC ;

ret0:

#if	CF_DEBUGS
	debugprintf("resolves_open: ret rs=%d\n",rs) ;
#endif

	return rs ;

/* bad stuff */
bad3:
	resolves_maplose(op) ;

bad2:
	ptm_destroy(&op->m) ;

bad1:
	uc_free(op->pr) ;
	op->pr = NULL ;

bad0:
	goto ret0 ;
}
/* end subroutine (resolves_open) */


int resolves_close(op)
RESOLVES		*op ;
{
	int	rs = SR_OK ;
	int	rs1 ;


	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != RESOLVES_MAGIC)
	    return SR_NOTOPEN ;

	rs1 = resolves_ufindfinish(op) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = resolves_envend(op) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = resolves_maplose(op) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = ptm_destroy(&op->m) ;
	if (rs >= 0) rs = rs1 ;

	if (op->pr != NULL) {
	    uc_free(op->pr) ;
	    op->pr = NULL ;
	}

	op->magic = 0 ;
	return rs ;
}
/* end subroutine (resolves_close) */


int resolves_check(op,daytime)
RESOLVES		*op ;
time_t		daytime ;
{
	int	rs ;


	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != RESOLVES_MAGIC)
	    return SR_NOTOPEN ;

	rs = resolves_checker(op,daytime) ;

	return rs ;
}
/* end subroutine (resolves_check) */


int resolves_process(op,groupname,admins,fd)
RESOLVES		*op ;
const char	groupname[] ;
const char	*admins[] ;
int		fd ;
{
	RESOLVES_ID		id ;

	int	rs ;


	memset(&id,0,sizeof(RESOLVES_ID)) ;
	id.groupname = groupname ;
	id.uid = -1 ;
	id.gid = -1 ;
	rs = resolves_procid(op,&id,admins,fd) ;

	return rs ;
}
/* end subroutine (resolves_process) */


int resolves_procid(op,idp,admins,fd)
RESOLVES		*op ;
RESOLVES_ID		*idp ;
const char	*admins[] ;
int		fd ;
{
	RESOLVES_ID	id ;

	STRPACK	packer ;

	int	rs = SR_OK ;
	int	n ;
	int	size ;
	int	wlen = 0 ;

	const char	*groupname ;

	char	ubuf[USERNAMELEN + 1] ;

	void	*p ;


	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != RESOLVES_MAGIC)
	    return SR_NOTOPEN ;

	if (idp == NULL)
	    return SR_FAULT ;

	if (fd < 0)
	    return SR_BADF ;

	groupname = idp->groupname ;
	if (groupname == NULL)
	    return SR_FAULT ;

	if (groupname[0] == '\0')
	    return SR_INVALID ;

#if	CF_DEBUGS
	{
	    debugprintf("resolves_procid: tar groupname=%s\n",groupname) ;
	    debugprintf("resolves_procid: tar username=%s\n",idp->username) ;
	    debugprintf("resolves_procid: tar uid=%d\n",idp->uid) ;
	    if (admins != NULL) {
	        int	i ;
	        for (i = 0 ; admins[i] != NULL ; i += 1)
	            debugprintf("resolves_procid: a[%u[=%s\n",i,admins[i]) ;
	    }
	}
#endif /* CF_DEBUGS */

/* fill in some missing elements */

	id = *idp ;			/* copy for possible modification */
	rs = resolves_idcheck(op,&id,ubuf) ;

#if	CF_DEBUGS
	debugprintf("resolves_procid: _idcheck() rs=%d username=%s\n",
	    rs,id.username) ;
#endif

	if (rs < 0) goto ret0 ;

/* go */

	n = nelem(envstrs) ;
	size = (op->nenv + n + 1) * sizeof(const char *) ;
	if ((rs = uc_malloc(size,&p)) >= 0) {
	    const char	**ev = (const char **) p ;

#if	CF_DEBUGS
	    debugprintf("resolves_procid: allocced\n") ;
#endif

	    if ((rs = strpack_start(&packer,128)) >= 0) {

#if	CF_DEBUGS
	        debugprintf("resolves_procid: strpack\n") ;
#endif

	        if ((rs = resolves_envadds(op,&packer,ev,&id)) >= 0) {

#if	CF_DEBUGS
	            debugprintf("resolves_procid: _envadds\n") ;
#endif
	            rs = resolves_processor(op,ev,admins,groupname,fd) ;
	            wlen = rs ;

#if	CF_DEBUGS
	            debugprintf("resolves_procid: _processor() rs=%d\n",rs) ;
#endif
	        }

	        strpack_finish(&packer) ;
	    } /* end if (packer */

	    uc_free(p) ;
	} /* end if (memory allocation) */

/* done */
ret0:

#if	CF_DEBUGS
	debugprintf("resolves_procid: ret rs=%d wlen=%u\n",rs,wlen) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (resolves_procid) */


int resolvesid_load(idp,un,gn,uid,gid)
RESOLVES_ID		*idp ;
const char	*un ;
const char	*gn ;
uid_t		uid ;
gid_t		gid ;
{


	if (idp == NULL)
	    return SR_FAULT ;

	memset(idp,0,sizeof(RESOLVES_ID)) ;
	idp->uid = uid ;
	idp->gid = gid ;
	idp->username = un ;
	idp->groupname = gn ;

	return SR_OK ;
}
/* end subroutine (resolvesid_load) */


/* private subroutines */


static int resolves_mapfind(op,daytime)
RESOLVES		*op ;
time_t		daytime ;
{
	int	rs ;

	char	mapfname[MAXPATHLEN + 1] ;


	mapfname[0] = '\0' ;
	rs = resolves_mapfname(op,mapfname) ;

#if	CF_DEBUGS
	debugprintf("resolves_mapfind: _mapfname() rs=%d\n",rs) ;
#endif

	if (rs < 0)
	    goto bad0 ;

	if (mapfname[0] != '\0') {
	    rs = mapper_start(&op->mapper,daytime,mapfname) ;

#if	CF_DEBUGS
	    debugprintf("resolves_mapfind: mapper_start() rs=%d\n",rs) ;
#endif

	    if (rs >= 0)
	        op->nmaps += 1 ;
	}

ret0:

#if	CF_DEBUGS
	debugprintf("resolves_mapfind: ret rs=%d\n",rs) ;
#endif

	return rs ;

/* bad stuff */
bad0:
	goto ret0 ;
}
/* end subroutine (resolves_mapfind) */


static int resolves_maplose(op)
RESOLVES	*op ;
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op->nmaps > 0) {
	    rs1 = mapper_finish(&op->mapper) ;
	    if (rs >= 0) rs = rs1 ;
	    op->nmaps = 0 ;
	}

	return rs ;
}
/* end subroutine (resolves_maplose) */


static int resolves_mapfname(op,mapfname)
RESOLVES	*op ;
char		mapfname[] ;
{
	vecstr	scheds ;

	int	rs = SR_OK ;
	int	rs1 ;
	int	c = 0 ;


	mapfname[0] = '\0' ;
	if ((rs = vecstr_start(&scheds,6,0)) >= 0) {

	rs = resolves_schedload(op,&scheds) ;

	if (rs >= 0) {

	    rs1 = permsched(schedmaps,&scheds,
	        mapfname,MAXPATHLEN, op->fe,R_OK) ;

#if	CF_DEBUGS
	    debugprintf("resolves_mapfname: permsched() rs=%d\n",rs1) ;
#endif

	    if (rs1 < 0) {
	        if (rs1 == SR_NOENT) {
	            mapfname[0] = '\0' ;
	        } else
	            rs = rs1 ;
	    } else
	        c = 1 ;

	} /* end if */

	vecstr_finish(&scheds) ;
	} /* end if (scheds) */

ret0:

#if	CF_DEBUGS
	debugprintf("resolves_mapfname: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (resolves_mapfname) */


static int resolves_schedload(op,slp)
RESOLVES		*op ;
vecstr		*slp ;
{
	int	rs = SR_OK ;

	const char	*name = RESOLVES_NAME ;


	if (rs >= 0)
	    rs = vecstr_envset(slp,"p",op->pr,-1) ;

	if (rs >= 0)
	    rs = vecstr_envset(slp,"e","etc",3) ;

	if (rs >= 0)
	    rs = vecstr_envset(slp,"n",name,-1) ;

	return rs ;
}
/* end subroutine (resolves_schedload) */


static int resolves_checker(op,daytime)
RESOLVES		*op ;
time_t		daytime ;
{
	int	rs = SR_OK ;
	int	nchanged = 0 ;


#if	CF_DEBUGS
	debugprintf("resolves_checker: nmaps=%d\n",op->nmaps) ;
#endif

	if (op->nmaps == 0)
	    goto ret0 ;

	if (daytime == NULL) daytime = time(NULL) ;

	if ((rs = ptm_lock(&op->m)) >= 0) {

#if	CF_DEBUGS
	    debugprintf("resolves_checker: got lock\n") ;
#endif

	    if ((daytime - op->ti_lastcheck) >= TO_CHECK) {

	        rs = mapper_check(&op->mapper,daytime) ;
	        nchanged = rs ;
	        op->ti_lastcheck = daytime ;

#if	CF_DEBUGS
	        debugprintf("resolves_checker: mapper_check() rs=%d\n",rs) ;
#endif

	    } /* end if */

	    ptm_unlock(&op->m) ;
	} /* end if (mutex) */

ret0:

#if	CF_DEBUGS
	debugprintf("resolves_checker: ret rs=%d nchanged=%u\n",rs,nchanged) ;
#endif

	return (rs >= 0) ? nchanged : rs ;
}
/* end subroutine (resolves_checker) */


static int resolves_envbegin(op)
RESOLVES		*op ;
{
	const int	es = envpre[0] ;
	const int	envprelen = strlen(envpre) ;

	int	rs = SR_OK ;
	int	size ;
	int	i ;
	int	c = 0 ;
	int	f ;

	void	*p ;


	for (i = 0 ; environ[i] != NULL ; i += 1) ;

	size = (i + 1) * sizeof(const char *) ;
	rs = uc_malloc(size,&p) ;

	if (rs >= 0) {
	    const char	*ep ;
	    const char	**va = (const char **) p ;
	    op->envv = va ;
	    for (i = 0 ; environ[i] != NULL ; i += 1) {
	        ep = environ[i] ;
	        f = TRUE ;
	        f = f && (ep[0] != '_') ;
	        f = f && (matstr(envbad,ep,-1) < 0) ;
	        if (f && (ep[0] == es)) 
		    f = (strncmp(envpre,ep,envprelen) != 0) ;
	        if (f)
	            va[c++] = ep ;
	    } /* end for */
	    va[c] = NULL ;
	    op->nenv = c ;
	} /* end if */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (resolves_envbegin) */


static int resolves_envend(op)
RESOLVES		*op ;
{


	if (op->envv != NULL) {
	    uc_free(op->envv) ;
	    op->envv = NULL ;
	}

	return SR_OK ;
}
/* end subroutine (resolves_envend) */


static int resolves_envadds(op,spp,ev,idp)
RESOLVES		*op ;
STRPACK		*spp ;
const char	**ev ;
RESOLVES_ID		*idp ;
{
	const int	envlen = ENVBUFLEN ;

	uint	uv ;

	int	rs = SR_OK ;
	int	n, i ;
	int	el ;

	const char	**envv = op->envv ;
	const char	*pre = envpre ;
	const char	*cp ;

	char	envbuf[ENVBUFLEN + 1] ;
	char	digbuf[DIGBUFLEN + 1] ;


	for (n = 0 ; n < op->nenv ; n += 1) {
	        ev[n] = envv[n] ;
	}

	for (i = 0 ; (rs >= 0) && (envstrs[i] != NULL) ; i += 1) {
	    envbuf[0] = '\0' ;
	    el = -1 ;
	    switch (i) {
	    case envstr_uid:
	        if (idp->uid >= 0) {
	            uv = idp->uid ;
	            rs = ctdecui(digbuf,DIGBUFLEN,uv) ;
	            if (rs >= 0) {
	                rs = sncpy4(envbuf,envlen,pre,envstrs[i],"=",digbuf) ;
	                el = rs ;
	            }
	        }
	        break ;
	    case envstr_gid:
	        if (idp->gid >= 0) {
	            uv = idp->gid ;
	            rs = ctdecui(digbuf,DIGBUFLEN,uv) ;
	            if (rs >= 0) {
	                rs = sncpy4(envbuf,envlen,pre,envstrs[i],"=",digbuf) ;
	                el = rs ;
	            }
	        }
	        break ;
	    case envstr_username:
	        cp = idp->username ;
	        if ((cp != NULL) && (cp[0] != '\0')) {
	            rs = sncpy4(envbuf,envlen,pre,envstrs[i],"=",cp) ;
	            el = rs ;
	        }
	        break ;
	    case envstr_groupname:
	        cp = idp->groupname ;
	        if ((cp != NULL) && (cp[0] != '\0')) {
	            rs = sncpy4(envbuf,envlen,pre,envstrs[i],"=",cp) ;
	            el = rs ;
	        }
	        break ;
	    } /* end switch */
	    if ((rs >= 0) && (envbuf[0] != '\0')) {
	        rs = resolves_envstore(op,spp,ev,n,envbuf,el) ;
	        if (rs > 0) n += 1 ;
	    }
	} /* end for */
	ev[n] = NULL ; /* very important! */

	return (rs >= 0) ? n : rs ;
}
/* end subroutine (resolves_envadds) */


static int resolves_envstore(op,spp,ev,n,ep,el)
RESOLVES		*op ;
STRPACK		*spp ;
const char	*ev[] ;
int		n ;
const char	*ep ;
int		el ;
{
	int	rs = SR_OK ;

	const char	*cp ;


	if (op == NULL) return SR_FAULT ;

	if (ep != NULL) {
	    rs = strpack_store(spp,ep,el,&cp) ;
	    if (rs >= 0) {
	        ev[n++] = cp ;
	        rs = n ;
	    }
	}

	return rs ;
}
/* end subroutine (resolves_envstore) */


static int resolves_idcheck(op,idp,ubuf)
RESOLVES	*op ;
RESOLVES_ID	*idp ;
char		*ubuf ;
{
	int		rs = SR_OK ;

	if (op == NULL) return SR_FAULT ;
	if (idp == NULL) return SR_FAULT ;
	if (ubuf == NULL) return SR_FAULT ;

	if (idp->groupname == NULL) return SR_FAULT ;

	if (idp->groupname[0] == '\0') return SR_INVALID ;

	if ((rs >= 0) && (idp->uid < 0)) {
	    idp->uid = getuid() ;
	}

	if ((rs >= 0) && (idp->gid < 0)) {
	    rs = getgroup_gid(idp->groupname,-1) ;
	    idp->gid = rs ;
	}

#if	CF_DEBUGS
	debugprintf("resolves_idcheck: tar un=%s\n",idp->username) ;
#endif

	if (rs >= 0) {
	    const char	*tun = idp->username ;
	    if ((tun == NULL) || (tun[0] == '\0') || (tun[0] == '-')) {
		rs = SR_OK ;
		ubuf[0] = '\0' ;
#if	CF_FINDUID
 	 	if ((tun == NULL) || (tun[0] == '\0')) {
		    rs = resolves_ufindlook(op,ubuf,idp->uid) ;
		}
#endif /* CF_FINDUID */
		if (rs == SR_OK)
	            rs = getusername(ubuf,USERNAMELEN,idp->uid) ;
	        idp->username = ubuf ;
	    }
	}

#if	CF_DEBUGS
	debugprintf("resolves_idcheck: ret rs=%d un=%s\n",rs,idp->username) ;
#endif

	return rs ;
}
/* end subroutine (resolves_idcheck) */


static int resolves_processor(op,ev,admins,groupname,fd)
RESOLVES		*op ;
const char	**ev ;
const char	*groupname ;
const char	*admins[] ;
int		fd ;
{
	int	rs = SR_OK ;
	int	wlen = 0 ;


#if	CF_DEBUGS
	debugprintf("resolves_processor: enter gn=%s\n",groupname) ;
#endif

	rs = resolves_checker(op,0) ;

#if	CF_DEBUGS
	debugprintf("resolves_processor: _checker() rs=%d\n",rs) ;
#endif

	if (rs < 0)
	    goto ret0 ;

#if	CF_DEBUGS
	debugprintf("resolves_processor: nmaps=%u\n",op->nmaps) ;
#endif

	if (op->nmaps > 0) {
	    rs = mapper_process(&op->mapper,ev,admins,groupname,fd) ;
	    wlen += rs ;
	}

#if	CF_DEBUGS && CF_TESTPROC
	if ((rs >= 0) && (op->nmaps > 0)) {
	    rs = mapper_lockcheck(&op->mapper,"post2") ;
	    debugprintf("resolves_processor: mapper_lockcheck() rs=%d\n",rs) ;
	}
#endif

ret0:

#if	CF_DEBUGS
	debugprintf("resolves_processor: ret rs=%d wlen=%u\n",rs,wlen) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (resolves_processor) */


static int resolves_ufindstart(op)
RESOLVES		*op ;
{
	const int	maxent = 30 ;
	const int	ttl = (1*60*60) ;

	int	rs = SR_OK ;


	if (! op->open.ufind) {
	    op->open.ufind = TRUE ;
	    rs = finduid_start(&op->ufind,NULL,maxent,ttl) ;
	}

	return rs ;
}
/* end subroutine (resolves_ufindstart) */


static int resolves_ufindfinish(op)
RESOLVES		*op ;
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op->open.ufind) {
	    op->open.ufind = FALSE ;
	    rs1 = finduid_finish(&op->ufind) ;
	    if (rs >= 0) rs = rs1 ;
	}

	return rs ;
}
/* end subroutine (resolves_ufindfinish) */


static int resolves_ufindlook(op,ubuf,uid)
RESOLVES		*op ;
char		ubuf[] ;
uid_t		uid ;
{
	int	rs ;


	if ((rs = ptm_lock(&op->m)) >= 0) {

	    if (! op->open.ufind) rs = resolves_ufindstart(op) ;

	    if (rs >= 0) {
		rs = finduid_lookup(&op->ufind,ubuf,uid) ;
#if	CF_DEBUGS
	debugprintf("resolves_ufindlook: finduid_lookup() rs=%d uid=%u u=%s\n",
		rs,uid,ubuf) ;
#endif
	    } /* end if */

	    ptm_unlock(&op->m) ;
	} /* end if (mutex) */

 /* "not-found" is a zero return, "found" is > zero */

	return rs ;
}
/* end subroutine (resolves_ufindlook) */


static int mapper_start(mmp,daytime,fname)
RESOLVES_MAPPER	*mmp ;
time_t		daytime ;
const char	fname[] ;
{
	int	rs ;

	const char	**evp = (const char **) environ ;
	const char	*ccp ;


#if	CF_DEBUGS
	debugprintf("mapper_start: sizeof(PTM)=%u\n",
	    sizeof(PTM)) ;
#endif

	memset(mmp,0,sizeof(RESOLVES_MAPPER)) ;

	rs = lockrw_create(&mmp->rwm,0) ;
	if (rs < 0)
	    goto bad0 ;

	rs = uc_mallocstrw(fname,-1,&ccp) ;
	if (rs < 0)
	    goto bad1 ;

	mmp->fname = ccp ;
	rs = vechand_start(&mmp->mapdirs,4,0) ;
	if (rs < 0)
	    goto bad2 ;

	rs = paramfile_open(&mmp->dirsfile,evp,mmp->fname) ;

#if	CF_DEBUGS
	debugprintf("resolves/mapper_start: paramfile_open() rs=%d\n",rs) ;
#endif

	if (rs < 0)
	    goto bad3 ;

	rs = paramfile_checkint(&mmp->dirsfile,TO_MAPCHECK) ;

#if	CF_DEBUGS
	debugprintf("resolves/mapper_start: paramfile_checkint() rs=%d\n",rs) ;
#endif

	if (rs < 0)
	    goto bad4 ;

	mmp->magic = RESOLVES_MAPPERMAGIC ;
	rs = mapper_mapload(mmp) ;

#if	CF_DEBUGS
	debugprintf("resolves/mapper_start: _mapload() rs=%d\n",rs) ;
#endif

	if (rs < 0)
	    goto bad4 ;

	mmp->ti_check = daytime ;

ret0:

#if	CF_DEBUGS
	debugprintf("resolves/mapper_start: ret rs=%d\n",rs) ;
#endif

	return rs ;

/* bad stuff */
bad4:
	mmp->magic = 0 ;
	paramfile_close(&mmp->dirsfile) ;

bad3:
	vechand_finish(&mmp->mapdirs) ;

bad2:
	uc_free(mmp->fname) ;
	mmp->fname = NULL ;

bad1:
	lockrw_destroy(&mmp->rwm) ;

bad0:
	goto ret0 ;
}
/* end subroutine (mapper_start) */


static int mapper_finish(mmp)
RESOLVES_MAPPER	*mmp ;
{
	int	rs = SR_OK ;
	int	rs1 ;


	if (mmp == NULL)
	    return SR_FAULT ;

	if (mmp->magic != RESOLVES_MAPPERMAGIC)
	    return SR_NOTOPEN ;

	rs1 = paramfile_close(&mmp->dirsfile) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = mapper_mapfrees(mmp) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = vechand_finish(&mmp->mapdirs) ;
	if (rs >= 0) rs = rs1 ;

	if (mmp->fname != NULL) {
	    uc_free(mmp->fname) ;
	    mmp->fname = NULL ;
	}

	rs1 = lockrw_destroy(&mmp->rwm) ;
	if (rs >= 0) rs = rs1 ;

	mmp->magic = 0 ;

#if	CF_DEBUGS
	debugprintf("resolves/mapper_finish: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (mapper_finish) */


static int mapper_check(mmp,daytime)
RESOLVES_MAPPER	*mmp ;
time_t		daytime ;
{
	const int	to_lock = TO_LOCK ;

	int	rs = SR_OK ;
	int	rs1 ;
	int	nchanged = 0 ;


	if (mmp == NULL)
	    return SR_FAULT ;

	if (mmp->magic != RESOLVES_MAPPERMAGIC)
	    return SR_NOTOPEN ;

	rs = lockrw_wrlock(&mmp->rwm,to_lock) ;

#if	CF_DEBUGS
	debugprintf("resolves/mapper_check: wr-lock rs=%d\n",rs) ;
#endif

	if (rs >= 0) {

	    if (daytime == 0)
	        daytime = time(NULL) ;

	    if ((daytime - mmp->ti_check) >= TO_MAPCHECK) {

#if	CF_PARAMFILE
	        rs = paramfile_check(&mmp->dirsfile,daytime) ;
	        if (rs > 0) {

	            {
	                mapper_mapfrees(mmp) ;
	                vechand_delall(&mmp->mapdirs) ;
	            }

	            rs = mapper_mapload(mmp) ;
	            nchanged = rs ;

	        } /* end if */
#else /* CF_PARAMFILE */
	        {
	            struct ustat	sb ;

	            int	rs1 = u_stat(mmp->fname,&sb) ;


	            if ((rs1 >= 0) && (sb.st_mtime > mmp->ti_mtime)) {

	                {
	                    mapper_mapfrees(mmp) ;
	                    vechand_delall(&mmp->mapdirs) ;
	                }

	                rs = mapper_mapload(mmp) ;
	                nchanged = rs ;

	            } /* end if (file mtime check) */

	            mmp->ti_check = daytime ;
	        }
#endif /* CF_PARAMFILE */

	    } /* end if (map-object check) */

	    rs1 = lockrw_unlock(&mmp->rwm) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (read-write lock) */

#if	CF_DEBUGS
	debugprintf("resolves/mapper_check: ret rs=%d nchanged=%u\n",rs,nchanged) ;
#endif

	return (rs >= 0) ? nchanged : rs ;
}
/* end subroutine (mapper_check) */


static int mapper_process(mmp,ev,admins,groupname,fd)
RESOLVES_MAPPER	*mmp ;
const char	**ev ;
const char	*admins[] ;
const char	groupname[] ;
int		fd ;
{
	const int	to_lock = TO_LOCK ;

	int	rs = SR_OK ;
	int	rs1 ;
	int	wlen = 0 ;


	if (mmp == NULL)
	    return SR_FAULT ;

	if (mmp->magic != RESOLVES_MAPPERMAGIC)
	    return SR_NOTOPEN ;

#if	CF_DEBUGS
	debugprintf("resolves/mapper_process: enter\n") ;
#endif

	rs = lockrw_rdlock(&mmp->rwm,to_lock) ;

#if	CF_DEBUGS
	debugprintf("resolves/mapper_process: rd-lock rs=%d\n",rs) ;
#endif

	if (rs >= 0) {

#if	CF_DEBUGS
	    debugprintf("resolves/mapper_process: gn=%s\n",groupname) ;
	    if (admins != NULL) {
	        int	i ;
	        for (i = 0 ; admins[i] != NULL ; i += 1)
	            debugprintf("resolves/mapper_process: a%u=%s\n",i,admins[i]) ;
	    }
#endif /* CF_DEBUGS */

#if	CF_TESTPROC
	    {
	        const char	*pr = "/home/genserv" ;
	        const char	*prn = "genserv" ;
	        const char	*svc = "hello" ;
	        const int	of = O_RDONLY ;
	        const mode_t	om = 0666 ;
	        const char		*argv[2] ;
	        const int		to = 5 ;

	        argv[0] = svc ;
	        argv[1] = NULL ;
	        rs = uc_openfsvc(pr,prn,svc,of,om,argv,ev,to) ;
	        if (rs >= 0) {
	            char	buf[BUFLEN+1] ;
		    int	rfd = rs ;
	            while ((rs = uc_reade(rfd,buf,BUFLEN,5,0)) > 0) {
	                int	len = rs ;
	                rs = writeto(fd,buf,len,10) ;
	                wlen = rs ;
			if (rs < 0) break ;
	            }
		    u_close(rfd) ;
	        }
	    }
#else /* CF_TESTPROC */
	    rs = mapper_processor(mmp,ev,admins,groupname,fd) ;
	    wlen += rs ;
#endif /* CF_TESTPROC */

#if	CF_DEBUGS
	    debugprintf("resolves/mapper_process: mapper_processor() rs=%d\n",rs) ;
#endif

	    rs1 = lockrw_unlock(&mmp->rwm) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (read-write lock) */

#if	CF_DEBUGS
	debugprintf("resolves/mapper_process: finished rs=%d\n",rs) ;
#endif

#if	CF_DEBUGS && CF_TESTPROC
	if (rs >= 0) {
	    rs = mapper_lockcheck(mmp,"post1") ;
	    debugprintf("resolves_processor: mapper_lockcheck() rs=%d\n",rs) ;
	}
#endif

#if	CF_DEBUGS
	debugprintf("resolves/mapper_process: ret rs=%d\n",rs) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (mapper_process) */


static int mapper_processor(mmp,ev,admins,groupname,fd)
RESOLVES_MAPPER	*mmp ;
const char	*ev[] ;
const char	*admins[] ;
const char	groupname[] ;
int		fd ;
{
	RESOLVES_MAPDIR	*ep ;

	int	rs = SR_OK ;
	int	i ;
	int	wlen = 0 ;


	if (mmp == NULL)
	    return SR_FAULT ;

	if (mmp->magic != RESOLVES_MAPPERMAGIC)
	    return SR_NOTOPEN ;

#if	CF_DEBUGS
	debugprintf("resolves/mapper_processor: gn=%s\n",groupname) ;
#endif

	for (i = 0 ; vechand_get(&mmp->mapdirs,i,&ep) >= 0 ; i += 1) {
	    if (ep == NULL) continue ;

#if	CF_DEBUGS
	    debugprintf("resolves/mapper_processor: i=%u admin=%s\n",i,ep->admin) ;
#endif

	    rs = mapdir_process(ep,ev,admins,groupname,fd) ;
	    wlen += rs ;
	    if (rs < 0)
	        break ;

	} /* end for */

#if	CF_DEBUGS
	debugprintf("resolves/mapper_processor: ret rs=%d wlen=%u\n",rs,wlen) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (mapper_processor) */


#if	CF_PARAMFILE

static int mapper_mapload(mmp)
RESOLVES_MAPPER	*mmp ;
{
	struct ustat		sb ;

	PARAMFILE		*pfp = &mmp->dirsfile ;
	PARAMFILE_ENT		pe ;
	PARAMFILE_CUR		cur ;

	int	rs = SR_OK ;
	int	rs1 ;
	int	kl, vl ;
	int	c = 0 ;

	const char	*kp, *vp ;

	char	pbuf[PBUFLEN + 1] ;


	if (mmp == NULL)
	    return SR_FAULT ;

	if (mmp->magic != RESOLVES_MAPPERMAGIC)
	    return SR_NOTOPEN ;

	rs1 = u_stat(mmp->fname,&sb) ;
	if (rs1 < 0)
	    goto ret0 ;

	mmp->ti_mtime = sb.st_mtime ;
	if ((rs = paramfile_curbegin(pfp,&cur)) >= 0) {

	    while (rs >= 0) {

	        kl = paramfile_enum(pfp,&cur,&pe,pbuf,PBUFLEN) ;
	        if (kl == SR_NOTFOUND) break ;

	        rs = kl ;
	        if (rs < 0) break ;

	        kp = pe.key ;
	        vp = pe.value ;
	        vl = pe.vlen ;

	        if (vl > 0) {
	            c += 1 ;
	            rs = mapper_mapadd(mmp,kp,kl,vp,vl) ;
	            if (rs < 0) break ;
	        }

	    } /* end while */

	    paramfile_curend(&mmp->dirsfile,&cur) ;
	} /* end if (paramfile-cursor) */

ret0:
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (mapper_mapload) */

#else /* CF_PARAMFILE */

static int mapper_mapload(mmp)
RESOLVES_MAPPER	*mmp ;
{
	struct ustat	sb ;

	bfile	mfile, *mfp = &mfile ;

	int	rs = SR_OK ;
	int	rs1 ;
	int	len ;
	int	kl, vl ;
	int	sl ;
	int	c = 0 ;

	const char	*tp, *sp ;
	const char	*kp, *vp ;

	char	linebuf[LINEBUFLEN + 1] ;


	if (mmp == NULL)
	    return SR_FAULT ;

	if (mmp->magic != RESOLVES_MAPPERMAGIC)
	    return SR_NOTOPEN ;

	rs1 = bopen(mfp,mmp->fname,"r",0666) ;
	if (rs1 < 0)
	    goto ret0 ;

	rs1 = bcontrol(mfp,BC_STAT,&sb) ;
	if (rs1 < 0)
	    goto ret1 ;

	mmp->ti_mtime = sb.st_mtime ;
	while ((rs = breadline(mfp,linebuf,LINEBUFLEN)) > 0) {
	    len = rs ;

	    sp = linebuf ;
	    sl = len ;
	    if (sp[0] == '#') continue ;

	    if ((tp = strnchr(sp,sl,'#')) != NULL)
	        sl = (tp - sp) ;

	    kl = nextfield(sp,sl,&kp) ;
	    if (kl == 0) continue ;

	    sl -= ((kp + kl) - sp) ;
	    sp = (kp + kl) ;

	    vl = nextfield(sp,sl,&vp) ;
	    if (vl == 0) continue ;

	    c += 1 ;
	    rs = mapper_mapadd(mmp,kp,kl,vp,vl) ;
	    if (rs < 0)
	        break ;

	} /* end while (reading lines) */

ret1:
	bclose(mfp) ;

ret0:
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (mapper_mapload) */

#endif /* CF_PARAMFILE */


static int mapper_mapadd(mmp,kp,kl,vp,vl)
RESOLVES_MAPPER	*mmp ;
int		kl ;
int		vl ;
const char	*kp ;
const char	*vp ;
{
	RESOLVES_MAPDIR	*ep ;

	const int	size = sizeof(RESOLVES_MAPDIR) ;

	int	rs ;


	if ((kp == NULL) || (vp == NULL)) return SR_FAULT ;
	if ((kl == 0) || (vl == 0)) return SR_INVALID ;

	rs = uc_malloc(size,&ep) ;
	if (rs >= 0) {
	    rs = mapdir_start(ep,kp,kl,vp,vl) ;
	    if (rs >= 0) {
	        rs = vechand_add(&mmp->mapdirs,ep) ;
	        if (rs < 0)
	            mapdir_finish(ep) ;
	    }
	    if (rs < 0)
	        uc_free(ep) ;
	}

	return rs ;
}
/* end subroutine (mapper_mapadd) */


static int mapper_mapfrees(mmp)
RESOLVES_MAPPER	*mmp ;
{
	RESOLVES_MAPDIR	*ep ;

	vechand		*mlp = &mmp->mapdirs ;

	int	rs = SR_OK ;
	int	rs1 ;
	int	i ;


	if (mmp == NULL)
	    return SR_FAULT ;

	if (mmp->magic != RESOLVES_MAPPERMAGIC)
	    return SR_NOTOPEN ;

	for (i = 0 ; vechand_get(mlp,i,&ep) >= 0 ; i += 1) {
	    if (ep == NULL) continue ;
	    rs1 = mapdir_finish(ep) ;
	    if (rs >= 0) rs = rs1 ;
	    vechand_del(mlp,i--) ;
	    uc_free(ep) ;
	} /* end for */

	return rs ;
}
/* end subroutine (mapper_mapfrees) */


#if	CF_TESTPROC

static int mapper_lockcheck(mmp,s)
RESOLVES_MAPPER	*mmp ;
const char	*s ;
{
	const int	to_lock = TO_LOCK ;

	int	rs = SR_OK ;
	int	rs1 ;


	if (mmp == NULL)
	    return SR_FAULT ;

	if (mmp->magic != RESOLVES_MAPPERMAGIC)
	    return SR_NOTOPEN ;

#if	CF_DEBUGS
	debugprintf("resolves/mapper_lockcheck: enter=%s\n",s) ;
#endif

	rs = lockrw_rdlock(&mmp->rwm,to_lock) ;

#if	CF_DEBUGS
	debugprintf("resolves/mapper_lockcheck: rd-lock rs=%d\n",rs) ;
#endif

	if (rs >= 0) {
	    rs1 = lockrw_unlock(&mmp->rwm) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (read-write lock) */

#if	CF_DEBUGS
	debugprintf("resolves/mapper_lockcheck: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (mapper_lockcheck) */

#endif /* CF_TESTPROC */


static int mapdir_start(ep,kp,kl,vp,vl)
RESOLVES_MAPDIR	*ep ;
const char	*kp, *vp ;
int		kl, vl ;
{
	int	rs = SR_OK ;
	int	size ;

	char	*ap = NULL ;


	if (ep == NULL)
	    return SR_FAULT ;

	if ((kp == NULL) || (vp == NULL)) return SR_FAULT ;
	if ((kl == 0) || (vl == 0)) return SR_INVALID ;

	memset(ep,0,sizeof(RESOLVES_MAPDIR)) ;

	if (kl < 0)
	    kl = strlen(kp) ;

	if (vl < 0)
	    vl = strlen(vp) ;

	size = (kl + 1 + vl + 1) ;
	rs = uc_malloc(size,&ap) ;
	if (rs < 0)
	    goto ret0 ;

	ep->admin = ap ;
	ap = strwcpy(ap,kp,kl) + 1 ;
	ep->dirname = ap ;
	ap = strwcpy(ap,vp,vl) + 1 ;

	rs = lockrw_create(&ep->rwm,0) ;
	if (rs < 0)
	    goto bad1 ;

ret0:
	return rs ;

/* bad stuff */
bad1:
	uc_free(ap) ;

bad0:
	goto ret0 ;
}
/* end subroutine (mapdir_start) */


static int mapdir_finish(ep)
RESOLVES_MAPDIR	*ep ;
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (ep == NULL)
	    return SR_FAULT ;

	if (ep->dname != NULL) {
	    rs1 = uc_free(ep->dname) ;
	    if (rs >= 0) rs = rs1 ;
	    ep->dname = NULL ;
	}

	rs1 = lockrw_destroy(&ep->rwm) ;
	if (rs >= 0) rs = rs1 ;

	if (ep->admin != NULL) {
	    rs1 = uc_free(ep->admin) ;
	    if (rs >= 0) rs = rs1 ;
	    ep->admin = NULL ;
	    ep->dirname = NULL ;
	}

	return rs ;
}
/* end subroutine (mapdir_finish) */


static int mapdir_process(ep,ev,admins,groupname,fd)
RESOLVES_MAPDIR	*ep ;
const char	*ev[] ;
const char	*admins[] ;
const char	groupname[] ;
int		fd ;
{
	const int	to_lock = TO_LOCK ;

	int	rs = SR_OK ;
	int	rs1 ;
	int	wlen = 0 ;


#if	CF_DEBUGS
	{
	    int	i ;
	    debugprintf("resolves/mapdir_process: entered gn=%s\n",groupname) ;
	    debugprintf("resolves/mapdir_process: dirname=%s\n",ep->dirname) ;
	    if (admins != NULL) {
	        for (i = 0 ; admins[i] != NULL ; i += 1)
	            debugprintf("resolves/mapdir_process: a[%u]=%s\n",i,admins[i]) ;
	    }
	}
#endif /* CF_DEBUGS */

	if (ep->dirname[0] == '\0')
	    goto ret0 ;

	if ((admins != NULL) && (admins[0] != NULL)) {
	    if (matstr(admins,ep->admin,-1) < 0)
	        goto ret0 ;
	} /* end if (admins) */

	if ((ep->dirname[0] == '~') && (ep->dname == NULL)) {

#if	CF_DEBUGS
	    debugprintf("resolves/mapdir_process: mapdir_expand() \n") ;
#endif

	    rs = mapdir_expand(ep) ;

#if	CF_DEBUGS
	    debugprintf("resolves/mapdir_process: mapdir_expand() rs=%d\n",rs) ;
	    debugprintf("resolves/mapdir_process: dname=%s\n",ep->dname) ;
#endif

	}

	if (rs < 0)
	    goto ret0 ;

	if ((ep->dirname[0] == '~') && (ep->dname == NULL))
	    goto ret0 ;

	rs = lockrw_rdlock(&ep->rwm,to_lock) ;

#if	CF_DEBUGS
	debugprintf("resolves/mapdir_process: rd-lock rs=%d\n",rs) ;
#endif

	if (rs >= 0) {

#if	CF_DEBUGS
	    debugprintf("resolves/mapdir_process: admin=%s\n",ep->admin) ;
	    debugprintf("resolves/mapdir_process: dirname=%s\n",ep->dirname) ;
	    debugprintf("resolves/mapdir_process: dname=%s\n",ep->dname) ;
#endif

	    if ((ep->dirname[0] != '~') || (ep->dname != NULL)) {

#if	CF_DEBUGS
	        debugprintf("resolves/mapdir_process: mapdir_processor() \n") ;
#endif

		if (rs >= 0) {
	            rs = mapdir_processor(ep,ev,groupname,fd) ;
	            wlen += rs ;
	 	}

#if	CF_DEBUGS
	        debugprintf("resolves/mapdir_process: mapdir_processor() rs=%d\n",
	            rs) ;
#endif

	    } /* end if */

	    rs1 = lockrw_unlock(&ep->rwm) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (locked) */

ret0:

#if	CF_DEBUGS
	debugprintf("resolves/mapdir_process: ret rs=%d wlen=%u\n",rs,wlen) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (mapdir_process) */


static int mapdir_expand(ep)
RESOLVES_MAPDIR	*ep ;
{
	const int	to_lock = TO_LOCK ;

	int	rs = SR_OK ;
	int	rs1 ;


#if	CF_DEBUGS
	debugprintf("resolves/mapdir_expand: dirname=%s\n",ep->dirname) ;
	debugprintf("resolves/mapdir_expand: dname=%s\n",ep->dname) ;
#endif

	rs =  lockrw_wrlock(&ep->rwm,to_lock) ;

#if	CF_DEBUGS
	debugprintf("resolves/mapdir_expand: wr-lock rs=%d\n",rs) ;
#endif

	if (rs >= 0) {

	    if ((ep->dirname[0] == '~') && (ep->dname == NULL)) {
	        rs = mapdir_expander(ep) ;

#if	CF_DEBUGS
	        debugprintf("resolves/mapdir_expand: mapdir_expander() rs=%d\n",
			rs) ;
	        debugprintf("resolves/mapdir_expand: dname=%s\n",ep->dname) ;
#endif

	    } /* end if */

	    rs1 = lockrw_unlock(&ep->rwm) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (read-write lock) */

ret0:

#if	CF_DEBUGS
	debugprintf("resolves/mapdir_expand: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (mapdir_expand) */


static int mapdir_expander(ep)
RESOLVES_MAPDIR	*ep ;
{
	const int	hlen = MAXPATHLEN ;
	int		rs = SR_OK ;
	int		rs1 = SR_OK ;
	int		unl ;
	int		fl = 0 ;
	const char	*tp ;
	const char	*pp ;
	const char	*un = NULL ;
	char		ubuf[USERNAMELEN + 1] ;
	char		hbuf[MAXPATHLEN+ 1] ;
	char		tmpfname[MAXPATHLEN + 1] ;

#if	CF_DEBUGS
	debugprintf("resolves/mapdir_expander: dirname=%s\n",ep->dirname) ;
#endif

	if ((ep->dirname == NULL) || (ep->dirname[0] != '~')) {
	    rs = SR_INVALID ;
	    goto ret0 ;
	}

	un = (ep->dirname+1) ;
	unl = -1 ;
	pp = NULL ;
	if ((tp = strchr(un,'/')) != NULL) {
	    unl = (tp - un) ;
	    pp = tp ;
	}

	if ((unl == 0) || (un[0] == '\0')) {
	    un = ep->admin ;
	    unl = -1 ;
	}

#if	CF_DEBUGS
	debugprintf("resolves/mapdir_expander: u=%t\n",un,unl) ;
	if (pp != NULL)
	    debugprintf("resolves/mapdir_expander: pp=%s\n",pp) ;
#endif

	    if (unl >= 0) {
	        strwcpy(ubuf,un,MIN(unl,USERNAMELEN)) ;
	        un = ubuf ;
	    }
	if ((rs = getuserhome(hbuf,hlen,un)) >= 0) {

	    if (pp != NULL) {
	        rs = mkpath2(tmpfname,hbuf,pp) ;
	        fl = rs ;
	    } else {
	        rs = mkpath1(tmpfname,hbuf) ;
	        fl = rs ;
	    }


#if	CF_DEBUGS
	    debugprintf("resolves/mapdir_expander: tmpfname=%s\n",tmpfname) ;
	    debugprintf("resolves/mapdir_expander: fl=%d\n",fl) ;
#endif

	    if (rs >= 0) {
		const char	*cp ;
	        rs = uc_mallocstrw(tmpfname,fl,&cp) ;
	        if (rs >= 0) ep->dname = cp ;
	    }

	} /* end if */

ret0:
	return (rs >= 0) ? fl : rs ;
}
/* end subroutine (mapdir_expander) */


static int mapdir_processor(ep,ev,groupname,fd)
RESOLVES_MAPDIR	*ep ;
const char	*ev[] ;
const char	groupname[] ;
int		fd ;
{
	int	rs = SR_OK ;
	int	rs1 ;
	int	n ;
	int	wlen = 0 ;

	const char	*dn ;
	const char	*gn ;
	const char	*defname = RESOLVES_DEFGROUP ;
	const char	*allname = RESOLVES_ALLGROUP ;

	char	env_admin[ENVBUFLEN+1] ;
	char	env_admindir[ENVBUFLEN+1] ;


	dn = ep->dirname ;
	if (dn[0] == '~') {
	    dn = ep->dname ;
	    if ((dn == NULL) || (dn[0] == '\0'))
	        goto ret0 ;
	}

	{
	    const char	*pre = envpre ;
	    const char	*post ;
	    const int	envlen = ENVBUFLEN ;
	    post = envstrs[envstr_admin] ;
	    strdcpy4(env_admin,envlen,pre,post,"=",ep->admin) ;
	    post = envstrs[envstr_admindir] ;
	    strdcpy4(env_admindir,envlen,pre,post,"=",dn) ;
	    for (n = 0 ; ev[n] != NULL ; n += 1) ;
	    ev[n+0] = env_admin ;
	    ev[n+1] = env_admindir ;
	    ev[n+2] = NULL ;
	}

	gn = groupname ;

#if	CF_DEBUGS
	{
	    int	i ;
	    debugprintf("resolves/mapdir_processor: mapdir_procout() gn=%s\n",gn) ;
#ifdef	COMMENT
	    for (i = 0 ; ev[i] != NULL ; i += 1) {
	        debugprintf("resolves/mapdir_processor: env[%u]=>%t<\n",
		    i,ev[i],strlinelen(ev[i],60,60)) ;
	    }
#endif
	}
#endif /* CF_DEBUGS */
#if	CF_DEBUGN
	nprintf(NDEBFNAME,"resolves/mapdir_processor: mapdir_procout() gn=%s\n",
	    gn) ;
#endif

	rs1 = mapdir_procout(ep,ev,dn,gn,fd) ;

#if	CF_DEBUGS
	debugprintf("resolves/mapdir_processor: mapdir_procout() rs=%d\n",rs1) ;
#endif
#if	CF_DEBUGN
	nprintf(NDEBFNAME,"resolves/mapdir_processor: mapdir_procout() rs=%d\n",
	    rs1) ;
#endif

	if (isNotPresent(rs1)) {

	    gn = defname ;

#if	CF_DEBUGS
	    debugprintf("resolves/mapdir_processor: mapdir_procout() gn=%s\n",gn) ;
#endif
#if	CF_DEBUGN
	    nprintf(NDEBFNAME,"resolves/mapdir_processor: mapdir_procout() gn=%s\n",
	        gn) ;
#endif

	    rs1 = mapdir_procout(ep,ev,dn,gn,fd) ;
	    if (! isNotPresent(rs1)) rs = rs1 ;

#if	CF_DEBUGS
	    debugprintf("resolves/mapdir_processor: mapdir_procout() rs=%d\n",rs1) ;
#endif
#if	CF_DEBUGN
	    nprintf(NDEBFNAME,"resolves/mapdir_processor: mapdir_procout() rs=%d\n",
	        rs1) ;
#endif

	} else
	    rs = rs1 ;

	if (rs > 0) wlen += rs ;

#if	CF_DEBUGS
	debugprintf("resolves/mapdir_processor: out rs=%d\n",rs) ;
#endif
#if	CF_DEBUGN
	nprintf(NDEBFNAME,"resolves/mapdir_processor: out rs=%d\n",rs) ;
#endif

	if (rs >= 0) {
	    gn = allname ;
	    rs1 = mapdir_procout(ep,ev,dn,gn,fd) ;
	    if (! isNotPresent(rs1)) rs = rs1 ;
	    if (rs > 0) wlen += rs ;
	}

	{
	    ev[n] = NULL ;
	}

ret0:

#if	CF_DEBUGS
	debugprintf("resolves/mapdir_processor: ret rs=%d wlen=%u\n",rs,wlen) ;
#endif
#if	CF_DEBUGN
	nprintf(NDEBFNAME,"resolves/mapdir_processor: ret rs=%d wlen=%u\n",
	    rs,wlen) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (mapdir_processor) */


/* we must return SR_NOENT if there was no file */
static int mapdir_procout(ep,ev,dn,groupname,fd)
RESOLVES_MAPDIR	*ep ;
const char	*ev[] ;
const char	dn[] ;
const char	groupname[] ;
int		fd ;
{
	int	rs = SR_OK ;
	int	rs1 ;
	int	wlen = 0 ;

	const char	*name = RESOLVES_NAME ;

	char	cname[MAXNAMELEN + 1] ;
	char	fname[MAXPATHLEN + 1] ;


/* we ignore buffer overflow here */

	rs1 = snsds(cname,MAXNAMELEN,groupname,name) ;
	if (rs1 >= 0)
	    rs1 = mkpath2(fname,dn,cname) ;

	if (rs1 >= 0) {
	    rs = mapdir_procouter(ep,ev,fname,fd) ;
	    wlen += rs ;
	}

	if ((rs >= 0) && (rs1 == SR_OVERFLOW)) rs = SR_NOENT ;

ret0:
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (mapdir_procout) */


static int mapdir_procouter(ep,ev,fname,ofd)
RESOLVES_MAPDIR	*ep ;
const char	*ev[] ;
const char	fname[] ;
int		ofd ;
{
	const mode_t	operms = 0664 ;

	const int	oflags = O_RDONLY ;
	const int	to_open = TO_OPEN ;
	const int	to_read = TO_READ ;
	const int	to_write = TO_WRITE ;

	int	rs = SR_OK ;
	int	rlen ;
	int	wlen = 0 ;

	char	buf[BUFLEN + 1] ;


	if (ep == NULL) return SR_FAULT ;

#if	CF_DEBUGS
	debugprintf("resolves/mapdir_procouter: fname=%s\n",fname) ;
#endif

	if ((rs = uc_openenv(fname,oflags,operms,ev,to_open)) >= 0) {
	    int	mfd = rs ;
#if	CF_DEBUGS
	    debugprintf("resolves/mapdir_procouter: uc_openenv() rs=%d\n",rs) ;
#endif
#if	CF_DEBUGN
	    nprintf(NDEBFNAME,"resolves/mapdir_procouter: uc_openenv() rs=%d\n",
		rs) ;
#endif

#if	CF_WRITETO
	while ((rs = uc_reade(mfd,buf,BUFLEN,to_read,0)) > 0) {
	    rlen = rs ;

	    rs = writeto(ofd,buf,rlen,to_write) ;
	    wlen += rs ;
	    if (rs < 0)
	        break ;

	} /* end while */
#else /* CF_WRITETO */
	rs = uc_copy(mfd,ofd,-1) ;
	wlen += rs ;
#endif /* CF_WRITETO */

	u_close(mfd) ;
	} /* end if (open) */

ret0:

#if	CF_DEBUGS
	debugprintf("resolves/mapdir_procouter: ret rs=%d wlen=%u\n",
		rs,wlen) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (mapdir_procouter) */


#if	CF_WRITETO

static int writeto(wfd,wbuf,wlen,wto)
int		wfd ;
const char	wbuf[] ;
int		wlen ;
int		wto ;
{
	struct pollfd	fds[2] ;

	time_t	daytime = time(NULL) ;
	time_t	ti_write ;

	int	rs = SR_OK ;
	int	i ;
	int	pt = TO_POLL ;
	int	pto ;
	int	tlen = 0 ;


	if (wfd < 0)
	    return SR_BADF ;

	if (wbuf == NULL)
	    return SR_FAULT ;

	if (wlen < 0)
	    wlen = strlen(wbuf) ;

	if (pt > wto)
	    pt = wto ;

	i = 0 ;
	fds[i].fd = wfd ;
	fds[i].events = POLLOUT ;
	i += 1 ;
	fds[i].fd = -1 ;
	fds[i].events = 0 ;

	ti_write = daytime ;
	pto = (pt * POLLMULT) ;
	while ((rs >= 0) && (tlen < wlen)) {

	    rs = u_poll(fds,1,pto) ;

	    daytime = time(NULL) ;
	    if (rs > 0) {
	        int	re = fds[0].revents ;

	        if (re & POLLOUT) {

	            rs = u_write(wfd,(wbuf+tlen),(wlen-tlen)) ;
	            tlen += rs ;
	            ti_write = daytime ;

	        } else if (re & POLLHUP) {
	            rs = SR_HANGUP ;
	        } else if (re & POLLERR) {
	            rs = SR_POLLERR ;
	        } else if (re & POLLNVAL) {
	            rs = SR_NOTOPEN ;
	        } /* end if (poll returned) */

	    } /* end if (got something) */

	    if (rs == SR_INTR)
	        rs = SR_OK ;

	    if ((daytime - ti_write) >= wto)
	        break ;

	} /* end while */

	return (rs >= 0) ? tlen : rs ;
}
/* end subroutine (writeto) */

#endif /* CF_WRITETO */



