/* statmsg */

/* object to help (manage) STATMSG messages */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */
#define	CF_DEBUGN	0		/* extra-special debugging */
#define	CF_WRITETO	1		/* time out writes */
#define	CF_PARAMFILE	1		/* use 'paramfile(3dam)' */


/* revision history:

	= 2003-10-01, David A­D­ Morano
        This is a hack from numerous previous hacks (not enumerated here). This
        is a new version of this hack that is entirely different (much simpler).

*/

/* Copyright © 2003 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This object module writes the contents of various STATMSGs (as specified
        by the caller) to an open file descriptor (also specified by the
        caller).

	Implementation notes:

        When processing, we time-out writes to the caller-supplied
        file-descriptor because we don't know if it is a non-regular file that
        might be flow-controlled. We don't wait forever for those sorts of
        outputs. So let's say that the output is a terminal that is currently
        flow-controlled. We will time-out on our writes and the user will not
        get this whole STATMSG text!


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<limits.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<getbufsize.h>
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
#include	<fsdir.h>
#include	<ascii.h>
#include	<localmisc.h>

#include	"statmsg.h"


/* local defines */

#define	STATMSG_MAPDIR		struct statmsg_mapdir
#define	STATMSG_DEFGROUP	"default"
#define	STATMSG_ALLGROUP	"all"
#define	STATMSG_NAME		"statmsg"
#define	STATMSG_SUF		"sm"
#define	STATMSG_DIRSFNAME	"dirs"

#define	STATMSG_MAPPERMAGIC	0x21367425

#define	NDEBFNAME	"statmsg.deb"

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

extern int	sichr(cchar *,int,int) ;
extern int	matstr(cchar **,cchar *,int) ;
extern int	nleadstr(cchar *,cchar *,int) ;
extern int	ctdecui(char *,int,uint) ;
extern int	sperm(IDS *,struct ustat *,int) ;
extern int	permsched(cchar **,vecstr *,char *,int,cchar *,int) ;
extern int	getnodedomain(char *,char *) ;
extern int	getuserhome(char *,int,cchar *) ;
extern int	getgid_group(cchar *,int) ;
extern int	mkuserpath(char *,cchar *,cchar *,int) ;
extern int	vecstr_envset(vecstr *,cchar *,cchar *,int) ;
extern int	msleep(int) ;
extern int	haslc(cchar *,int) ;
extern int	hasuc(cchar *,int) ;
extern int	hasalldig(cchar *,int) ;
extern int	isNotPresent(int) ;

#if	CF_DEBUGS || CF_DEBUGN
extern int	nprintf(cchar *,...) ;
extern int	debugprintf(cchar *,...) ;
extern int	strlinelen(cchar *,int,int) ;
#endif

extern char	*strwcpy(char *,cchar *,int) ;
extern char	*strnchr(cchar *,int,int) ;
extern char	*strdcpy1(char *,int,cchar *) ;
extern char	*strdcpy2(char *,int,cchar *,cchar *) ;
extern char	*strdcpy3(char *,int,cchar *,cchar *,cchar *) ;
extern char	*strdcpy4(char *,int,cchar *,cchar *,
			cchar *,cchar *) ;
extern char	*strdcpy1w(char *,int,cchar *,int) ;
extern char	*strnpbrk(cchar *,int,cchar *) ;


/* external variables */

extern cchar	**environ ;


/* local structures */

struct statmsg_mapdir {
	LOCKRW		rwm ;
	cchar	*username ;
	cchar	*userhome ;
	cchar	*admin ;
	cchar	*dirname ;	/* raw */
	cchar	*dname ;	/* expanded */
} ;


/* forward references */

int		statmsg_processid(STATMSG *,STATMSG_ID *,cchar **,cchar *,int) ;

static int	statmsg_userbegin(STATMSG *,cchar *) ;
static int	statmsg_userend(STATMSG *) ;

static int	statmsg_mapfind(STATMSG *,time_t) ;
static int	statmsg_maplose(STATMSG *) ;
static int	statmsg_mapfname(STATMSG *,char *) ;
static int	statmsg_schedload(STATMSG *,vecstr *) ;
static int	statmsg_checker(STATMSG *,time_t) ;
static int	statmsg_envbegin(STATMSG *) ;
static int	statmsg_envend(STATMSG *) ;
static int	statmsg_envadds(STATMSG *,STRPACK *,cchar **,
			STATMSG_ID *,cchar *) ;
static int	statmsg_envstore(STATMSG *,STRPACK *,cchar **,int,
			cchar *,int) ;
static int 	statmsg_processor(STATMSG *,cchar **,cchar **,
			cchar *,cchar *,int) ;
static int	statmsg_idcheck(STATMSG *,STATMSG_ID *,char *) ;

static int	mapper_start(STATMSG_MAPPER *,time_t,cchar *,cchar *,
			cchar *) ;
static int	mapper_finish(STATMSG_MAPPER *) ;
static int	mapper_check(STATMSG_MAPPER *,time_t) ;
static int	mapper_process(STATMSG_MAPPER *,cchar **,cchar **,
			cchar *,cchar *,int) ;
static int	mapper_processor(STATMSG_MAPPER *,cchar **,cchar **,
			cchar *,cchar *,int) ;
static int	mapper_mapload(STATMSG_MAPPER *) ;
static int	mapper_mapadd(STATMSG_MAPPER *,cchar *,int,
			cchar *,int) ;
static int	mapper_mapfins(STATMSG_MAPPER *) ;

static int	mapdir_start(STATMSG_MAPDIR *,cchar *,
			cchar *,cchar *,int,cchar *,int) ;
static int	mapdir_finish(STATMSG_MAPDIR *) ;
static int	mapdir_process(STATMSG_MAPDIR *,cchar **,cchar **,
			cchar *,cchar *,int) ;
static int	mapdir_expand(STATMSG_MAPDIR *) ;
static int	mapdir_expander(STATMSG_MAPDIR *) ;
static int	mapdir_processor(STATMSG_MAPDIR *,cchar **,
			cchar *,cchar *,int) ;
static int	mapdir_processorthem(STATMSG_MAPDIR *,cchar **,
			cchar *,VECSTR *,cchar **,int) ;
static int	mapdir_processorone(STATMSG_MAPDIR *,cchar **,
			cchar *,VECSTR *,cchar *,int) ;
static int	mapdir_procout(STATMSG_MAPDIR *,cchar **,cchar *,
			cchar *,int) ;
static int	mapdir_procouter(STATMSG_MAPDIR *,cchar **,
			cchar *,int) ;

static int	writeto(int,cchar *,int,int) ;
static int	loadstrs(cchar **,cchar *,cchar *,cchar *,cchar *) ;
static int	isBaseMatch(cchar *,cchar *,cchar *) ;


/* local variables */

static cchar	*schedmaps[] = {
	"%p/%e/%n/%n.%f",
	"%p/%e/%n/%f",
	"%p/%e/%n.%f",
	"%p/%n.%f",
	"%n.%f",
	NULL
} ;

static cchar	*envbad[] = {
	"TMOUT",
	"A__z",
	NULL
} ;

static cchar	*envstrs[] = {
	"USERNAME",
	"GROUPNAME",
	"UID",
	"GID",
	"KEYNAME",
	"ADMIN",
	"ADMINDIR",
	NULL
} ;

enum envstrs {
	envstr_username,
	envstr_groupname,
	envstr_uid,
	envstr_gid,
	envstr_keyname,
	envstr_admin,
	envstr_admindir,
	envstr_overlast
} ;

static cchar	*envpre = "STATMSG_" ;	/* environment prefix */


/* exported subroutines */


int statmsg_open(STATMSG *op,cchar username[])
{
	const time_t	dt = time(NULL) ;
	int		rs ;

	if (op == NULL) return SR_FAULT ;
	if (username == NULL) return SR_FAULT ;

	if (username[0] == '\0') return SR_INVALID ;

#if	CF_DEBUGN
	nprintf(NDEBFNAME,"statmsg_open: u=%s\n",username) ;
#endif

#if	CF_DEBUGS
	debugprintf("statmsg_open: sizeof(STATMSG)=%u\n",sizeof(STATMSG)) ;
#endif

	memset(op,0,sizeof(STATMSG)) ;
	op->fe = STATMSG_DIRSFNAME ;

	if ((rs = ptm_create(&op->m,NULL)) >= 0) {
	    if ((rs = statmsg_userbegin(op,username)) >= 0) {
		if ((rs = statmsg_mapfind(op,dt)) >= 0) {
		    if ((rs = statmsg_envbegin(op)) >= 0) {
			op->ti_lastcheck = dt ;
			op->magic = STATMSG_MAGIC ;
		    }
		    if (rs < 0)
			statmsg_maplose(op) ;
		}
		if (rs < 0)
		    statmsg_userend(op) ;
	    }
	    if (rs < 0)
		ptm_destroy(&op->m) ;
	} /* end if (ptm) */

#if	CF_DEBUGN
	nprintf(NDEBFNAME,"statmsg_open: ret rs=%d\n",rs) ;
#endif

#if	CF_DEBUGS
	debugprintf("statmsg_open: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (statmsg_open) */


int statmsg_close(STATMSG *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != STATMSG_MAGIC) return SR_NOTOPEN ;

	rs1 = statmsg_envend(op) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = statmsg_maplose(op) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = statmsg_userend(op) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = ptm_destroy(&op->m) ;
	if (rs >= 0) rs = rs1 ;

	op->magic = 0 ;
	return rs ;
}
/* end subroutine (statmsg_close) */


int statmsg_check(STATMSG *op,time_t dt)
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != STATMSG_MAGIC) return SR_NOTOPEN ;

	rs = statmsg_checker(op,dt) ;

	return rs ;
}
/* end subroutine (statmsg_check) */


int statmsg_process(STATMSG *op,cchar gn[],cchar *adms[],cchar *kn,int fd)
{
	STATMSG_ID	id ;
	int		rs ;

	memset(&id,0,sizeof(STATMSG_ID)) ;
	id.groupname = gn ;
	id.uid = -1 ;
	id.gid = -1 ;
	rs = statmsg_processid(op,&id,adms,kn,fd) ;

	return rs ;
}
/* end subroutine (statmsg_process) */


int statmsg_processid(STATMSG *op,STATMSG_ID *idp,cchar *adms[],
		cchar *kn,int fd)
{
	STATMSG_ID	id ;
	int		rs ;
	int		wlen = 0 ;
	cchar		*groupname ;
	char		ubuf[USERNAMELEN + 1] ;
	char		kbuf[2] ;

	if (op == NULL) return SR_FAULT ;
	if (idp == NULL) return SR_FAULT ;

	if (op->magic != STATMSG_MAGIC) return SR_NOTOPEN ;

	if (fd < 0) return SR_BADF ;

	groupname = idp->groupname ;
	if (groupname == NULL)
	    return SR_FAULT ;

	if (groupname[0] == '\0')
	    return SR_INVALID ;

	if (kn == NULL) {
	    kn = kbuf ;
	    kbuf[0] = '\0' ;
	}

#if	CF_DEBUGN
	nprintf(NDEBFNAME,"statmsg_processid: g=%s k=%s\n",groupname,kn) ;
#endif
#if	CF_DEBUGS
	{
	    debugprintf("statmsg_processid: tar groupname=%s\n",groupname) ;
	    debugprintf("statmsg_processid: tar username=%s\n",idp->username) ;
	    debugprintf("statmsg_processid: tar uid=%d\n",idp->uid) ;
	    if (adms != NULL) {
	        int	i ;
	        for (i = 0 ; adms[i] != NULL ; i += 1)
	            debugprintf("statmsg_processid: a[%u[=%s\n",i,adms[i]) ;
	    }
	    debugprintf("statmsg_processid: tar kn=%s\n",kn) ;
	}
#endif /* CF_DEBUGS */

/* fill in some missing elements */

	id = *idp ;			/* copy for possible modification */
	if ((rs = statmsg_idcheck(op,&id,ubuf)) >= 0) {
	    const int	n = nelem(envstrs) ;
	    int		size ;
	    void	*p ;
	    size = (op->nenv + n + 1) * sizeof(cchar *) ;
	    if ((rs = uc_malloc(size,&p)) >= 0) {
	        STRPACK		packer ;
	        cchar	**ev = (cchar **) p ;

#if	CF_DEBUGS
	    debugprintf("statmsg_processid: allocced\n") ;
#endif

	    if ((rs = strpack_start(&packer,128)) >= 0) {

#if	CF_DEBUGS
	        debugprintf("statmsg_processid: strpack\n") ;
#endif

	        if ((rs = statmsg_envadds(op,&packer,ev,&id,kn)) >= 0) {

#if	CF_DEBUGS
	            debugprintf("statmsg_processid: statmsg_envadds\n") ;
#endif
	            rs = statmsg_processor(op,ev,adms,groupname,kn,fd) ;
	            wlen = rs ;

#if	CF_DEBUGS
	            debugprintf("statmsg_processid: _processor() rs=%d\n",
			rs) ;
#endif
	        }

	        strpack_finish(&packer) ;
	    } /* end if (packer) */

	    uc_free(p) ;
	} /* end if (memory allocation) */
	} /* end if (statmsg_idcheck) */

#if	CF_DEBUGS
	debugprintf("statmsg_processid: ret rs=%d wlen=%u\n",rs,wlen) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (statmsg_processid) */


int statmsgid_load(STATMSG_ID *idp,cchar *un,cchar *gn,uid_t uid,gid_t gid)
{

	if (idp == NULL) return SR_FAULT ;

	memset(idp,0,sizeof(STATMSG_ID)) ;
	idp->uid = uid ;
	idp->gid = gid ;
	idp->username = un ;
	idp->groupname = gn ;

	return SR_OK ;
}
/* end subroutine (statmsgid_load) */


/* private subroutines */


static int statmsg_userbegin(STATMSG *op,cchar *username)
{
	const int	hlen = MAXPATHLEN ;
	int		rs = SR_OK ;
	char		hbuf[MAXPATHLEN+1] ;
	char		ubuf[USERNAMELEN+1] ;

	if (username == NULL) return SR_FAULT ;

	if (username[0] == '\0') return SR_INVALID ;

	if (username[0] == '-') {
	    rs = getusername(ubuf,USERNAMELEN,-1) ;
	    username = ubuf ;
	}

	if (rs >= 0) {
	    if ((rs = getuserhome(hbuf,hlen,username)) >= 0) {
		int	size = 0 ;
		char	*bp ;
		size += (strlen(username) + 1) ;
		size += (strlen(hbuf) + 1) ;
		if ((rs = uc_malloc(size,&bp)) >= 0) {
		    op->useralloc = bp ;
		    op->username = bp ;
		    bp = (strwcpy(bp,username,-1) + 1) ;
		    op->userhome = bp ;
		    bp = (strwcpy(bp,hbuf,-1) + 1) ;
		} /* end if (memory-allocation) */
	    } /* end if (getuserhome) */
	} /* end if (ok) */

	return rs ;
}
/* end subroutine (statmsg_userbegin) */


static int statmsg_userend(STATMSG *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op->useralloc != NULL) {
	    rs1 = uc_free(op->useralloc) ;
	    if (rs >= 0) rs = rs1 ;
	    op->useralloc = NULL ;
	    op->username = NULL ;
	    op->userhome = NULL ;
	}

	return rs ;
}
/* end subroutine (statmsg_userend) */


static int statmsg_mapfind(STATMSG *op,time_t dt)
{
	int		rs ;
	char		mapfname[MAXPATHLEN + 1] ;

	mapfname[0] = '\0' ;
	if ((rs = statmsg_mapfname(op,mapfname)) >= 0) {
	    if (mapfname[0] != '\0') {
		cchar	*un = op->username ;
		cchar	*uh = op->userhome ;
	        if ((rs = mapper_start(&op->mapper,dt,un,uh,mapfname)) >= 0) {
	            op->nmaps += 1 ;
		}
	    }
	}

#if	CF_DEBUGS
	debugprintf("statmsg_mapfind: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (statmsg_mapfind) */


static int statmsg_maplose(STATMSG *op)
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
/* end subroutine (statmsg_maplose) */


static int statmsg_mapfname(STATMSG *op,char fbuf[])
{
	vecstr		scheds ;
	int		rs ;
	int		rs1 ;
	int		c = 0 ;

	fbuf[0] = '\0' ;
	if ((rs = vecstr_start(&scheds,6,0)) >= 0) {
	    if ((rs = statmsg_schedload(op,&scheds)) >= 0) {
		const int	flen = MAXPATHLEN ;
	        rs1 = permsched(schedmaps,&scheds,fbuf,flen,op->fe,R_OK) ;
	        if ((rs1 == SR_NOENT) || (rs1 == SR_ACCESS)) {
	            if (rs1 == SR_NOENT) {
	                fbuf[0] = '\0' ;
	            } else
	                rs = rs1 ;
	        } else if (rs1 == SR_OK) {
	            c = 1 ;
		} else
		    rs = rs1 ;
	    } /* end if (statmsg-schedload) */
	    vecstr_finish(&scheds) ;
	} /* end if (vecstr) */

#if	CF_DEBUGS
	debugprintf("statmsg_mapfname: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (statmsg_mapfname) */


static int statmsg_schedload(STATMSG *op,vecstr *slp)
{
	int		rs = SR_OK ;
	int		i ;
	cchar		*keys = "pen" ;
	cchar		*name = STATMSG_NAME ;
	for (i = 0 ; keys[i] != '\0' ; i += 1) {
	    const int	kch = MKCHAR(keys[i]) ;
	    int		vl = -1 ;
	    cchar	*vp = NULL ;
	    switch (kch) {
	    case 'p':
		vp = op->userhome ;
		break ;
	    case 'e':
		vp = "etc" ;
		break ;
	    case 'n':
		vp = name ;
		break ;
	    } /* end switch */
	    if ((rs >= 0) && (vp != NULL)) {
		char	kbuf[2] = { 0, 0 } ;
		kbuf[0] = kch ;
		rs = vecstr_envset(slp,kbuf,vp,vl) ;
	    }
	    if (rs < 0) break ;
	} /* end for */
	return rs ;
}
/* end subroutine (statmsg_schedload) */


static int statmsg_checker(STATMSG *op,time_t dt)
{
	int		rs = SR_OK ;
	int		nchanged = 0 ;

#if	CF_DEBUGS
	debugprintf("statmsg_checker: nmaps=%d\n",op->nmaps) ;
#endif

	if (op->nmaps > 0) {
	if ((rs = ptm_lock(&op->m)) >= 0) {
	    if (dt == 0) dt = time(NULL) ;

#if	CF_DEBUGS
	    debugprintf("statmsg_checker: got lock\n") ;
#endif

	    if ((dt - op->ti_lastcheck) >= TO_CHECK) {

	        rs = mapper_check(&op->mapper,dt) ;
	        nchanged = rs ;
	        op->ti_lastcheck = dt ;

#if	CF_DEBUGS
	        debugprintf("statmsg_checker: mapper_check() rs=%d\n",rs) ;
#endif

	    } /* end if */

	    ptm_unlock(&op->m) ;
	} /* end if (mutex) */
	} /* end if (positive) */

#if	CF_DEBUGS
	debugprintf("statmsg_checker: ret rs=%d nchanged=%u\n",rs,nchanged) ;
#endif

	return (rs >= 0) ? nchanged : rs ;
}
/* end subroutine (statmsg_checker) */


static int statmsg_envbegin(STATMSG *op)
{
	int		rs = SR_OK ;
	int		size ;
	int		i ;
	int		c = 0 ;
	int		f ;
	void		*p ;

	for (i = 0 ; environ[i] != NULL ; i += 1) ;

	size = (i + 1) * sizeof(cchar *) ;
	if ((rs = uc_malloc(size,&p)) >= 0) {
	    cchar	*ep ;
	    cchar	**va = (cchar **) p ;
	    op->envv = va ;
	    for (i = 0 ; environ[i] != NULL ; i += 1) {
	        ep = environ[i] ;
	        f = TRUE ;
	        f = f && (ep[0] != '-') ;
	        f = f && (matstr(envbad,ep,-1) < 0) ;
	        if (f && (ep[0] == 'M')) f = (strncmp(envpre,ep,5) != 0) ;
	        if (f)
	            va[c++] = ep ;
	    } /* end for */
	    va[c] = NULL ;
	    op->nenv = c ;
	} /* end if (memory-allocation) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (statmsg_envbegin) */


static int statmsg_envend(STATMSG *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op->envv != NULL) {
	    rs1 = uc_free(op->envv) ;
	    if (rs >= 0) rs = rs1 ;
	    op->envv = NULL ;
	}

	return rs ;
}
/* end subroutine (statmsg_envend) */


static int statmsg_envadds(STATMSG *op,STRPACK *spp,cchar **ev,
		STATMSG_ID *idp,cchar *kn)
{
	uint		uv ;
	const int	envlen = ENVBUFLEN ;
	int		rs = SR_OK ;
	int		n, i ;
	int		el ;
	cchar		**envv = op->envv ;
	cchar		*es ;
	cchar		*cp ;
	char		envbuf[ENVBUFLEN + 1] ;
	char		digbuf[DIGBUFLEN + 1] ;

	for (n = 0 ; n < op->nenv ; n += 1) ev[n] = envv[n] ;

	for (i = 0 ; (rs >= 0) && (envstrs[i] != NULL) ; i += 1) {
	    envbuf[0] = '\0' ;
	    es = envstrs[i] ;
	    el = -1 ;
	    switch (i) {
	    case envstr_uid:
	        if (idp->uid >= 0) {
	            uv = idp->uid ;
	            rs = ctdecui(digbuf,DIGBUFLEN,uv) ;
	            if (rs >= 0) {
	                rs = sncpy4(envbuf,envlen,envpre,es,"=",digbuf) ;
	                el = rs ;
	            }
	        }
	        break ;
	    case envstr_gid:
	        if (idp->gid >= 0) {
	            uv = idp->gid ;
	            rs = ctdecui(digbuf,DIGBUFLEN,uv) ;
	            if (rs >= 0) {
	                rs = sncpy4(envbuf,envlen,envpre,es,"=",digbuf) ;
	                el = rs ;
	            }
	        }
	        break ;
	    case envstr_username:
	        cp = idp->username ;
	        if ((cp != NULL) && (cp[0] != '\0')) {
	            rs = sncpy4(envbuf,envlen,envpre,es,"=",cp) ;
	            el = rs ;
	        }
	        break ;
	    case envstr_groupname:
	        cp = idp->groupname ;
	        if ((cp != NULL) && (cp[0] != '\0')) {
	            rs = sncpy4(envbuf,envlen,envpre,es,"=",cp) ;
	            el = rs ;
	        }
	        break ;
	    case envstr_keyname:
	        if ((kn != NULL) && (kn[0] != '\0')) {
	            rs = sncpy4(envbuf,envlen,envpre,es,"=",kn) ;
	            el = rs ;
	        }
	        break ;
	    } /* end switch */
	    if ((rs >= 0) && (envbuf[0] != '\0')) {
	        rs = statmsg_envstore(op,spp,ev,n,envbuf,el) ;
	        if (rs > 0) n += 1 ;
	    }
	} /* end for */
	ev[n] = NULL ; /* very important! */

	return (rs >= 0) ? n : rs ;
}
/* end subroutine (statmsg_envadds) */


static int statmsg_envstore(STATMSG *op,STRPACK *spp,cchar *ev[],int n,
		cchar *ep,int el)
{
	int		rs = SR_OK ;

	if (op == NULL) return SR_FAULT ;

	if (ep != NULL) {
	    cchar	*cp ;
	    if ((rs = strpack_store(spp,ep,el,&cp)) >= 0) {
	        ev[n++] = cp ;
	        rs = n ;
	    }
	}

	return rs ;
}
/* end subroutine (statmsg_envstore) */


static int statmsg_idcheck(STATMSG *op,STATMSG_ID *idp,char *ubuf)
{
	int		rs = SR_OK ;

	if (op == NULL) return SR_FAULT ;
	if (idp->groupname == NULL) return SR_FAULT ;

	if (idp->groupname[0] == '\0') return SR_INVALID ;

	if ((rs >= 0) && (idp->uid < 0)) {
	    idp->uid = getuid() ;
	}

	if ((rs >= 0) && (idp->gid < 0)) {
	    rs = getgid_group(idp->groupname,-1) ;
	    idp->gid = rs ;
	}

	if (rs >= 0) {
	    cchar	*tun = idp->username ;
	    if ((tun == NULL) || (tun[0] == '\0') || (tun[0] == '-')) {
	        rs = getusername(ubuf,USERNAMELEN,idp->uid) ;
	        idp->username = ubuf ;
	    }
	}

	return rs ;
}
/* end subroutine (statmsg_idcheck) */


static int statmsg_processor(STATMSG *op,cchar **ev,cchar *adms[],cchar *gn,
		cchar *kn,int fd)
{
	int		rs ;
	int		wlen = 0 ;

#if	CF_DEBUGS
	debugprintf("statmsg_processor: enter gn=%s\n",gn) ;
#endif

	if ((rs = statmsg_checker(op,0)) >= 0) {
	    if (op->nmaps > 0) {
	        rs = mapper_process(&op->mapper,ev,adms,gn,kn,fd) ;
	        wlen += rs ;
	    }
	}

#if	CF_DEBUGS
	debugprintf("statmsg_processor: ret rs=%d wlen=%u\n",rs,wlen) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (statmsg_processor) */


static int mapper_start(STATMSG_MAPPER *mmp,time_t dt,cchar un[],cchar uh[],
		cchar fname[])
{
	const int	to = TO_MAPCHECK ;
	int		rs ;
	cchar		**evp = (cchar **) environ ;
	cchar		*ccp ;

#if	CF_DEBUGN
	nprintf(NDEBFNAME,"statmsg/mapper_start: uh=%s fn=%s\n",uh,fname) ;
#endif
#if	CF_DEBUGS
	debugprintf("mapper_start: sizeof(PTM)=%u\n",
	    sizeof(PTM)) ;
#endif

	memset(mmp,0,sizeof(STATMSG_MAPPER)) ;
	mmp->username = un ;
	mmp->userhome = uh ;

	if ((rs = lockrw_create(&mmp->rwm,0)) >= 0) {
	    if ((rs = uc_mallocstrw(fname,-1,&ccp)) >= 0) {
	        mmp->fname = ccp ;
	        if ((rs = vechand_start(&mmp->mapdirs,4,0)) >= 0) {
		    PARAMFILE	*dfp = &mmp->dirsfile ;
	            if ((rs = paramfile_open(dfp,evp,ccp)) >= 0) {
	                if ((rs = paramfile_checkint(dfp,to)) >= 0) {
	                    mmp->magic = STATMSG_MAPPERMAGIC ;
	                    rs = mapper_mapload(mmp) ;
	                    mmp->ti_check = dt ;
			    if (rs < 0)
			        mmp->magic = 0 ;
	                }
	                if (rs < 0)
		            paramfile_close(&mmp->dirsfile) ;
	            } /* end if (paramfile_open) */
	            if (rs < 0)
		        vechand_finish(&mmp->mapdirs) ;
	        } /* end if (vechand_start) */
	        if (rs < 0) {
	            uc_free(mmp->fname) ;
	            mmp->fname = NULL ;
	        }
	    } /* end if (memory-allocation) */
	    if (rs < 0)
	        lockrw_destroy(&mmp->rwm) ;
	} /* end if (lockrw_create) */

#if	CF_DEBUGS
	debugprintf("statmsg/mapper_start: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (mapper_start) */


static int mapper_finish(STATMSG_MAPPER *mmp)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (mmp == NULL) return SR_FAULT ;

	if (mmp->magic != STATMSG_MAPPERMAGIC) return SR_NOTOPEN ;

	rs1 = paramfile_close(&mmp->dirsfile) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = mapper_mapfins(mmp) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = vechand_finish(&mmp->mapdirs) ;
	if (rs >= 0) rs = rs1 ;

	if (mmp->fname != NULL) {
	    rs1 = uc_free(mmp->fname) ;
	    if (rs >= 0) rs = rs1 ;
	    mmp->fname = NULL ;
	}

	rs1 = lockrw_destroy(&mmp->rwm) ;
	if (rs >= 0) rs = rs1 ;

	mmp->magic = 0 ;

#if	CF_DEBUGS
	debugprintf("statmsg/mapper_finish: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (mapper_finish) */


static int mapper_check(STATMSG_MAPPER *mmp,time_t dt)
{
	const int	to_lock = TO_LOCK ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		nchanged = 0 ;

	if (mmp == NULL) return SR_FAULT ;

	if (mmp->magic != STATMSG_MAPPERMAGIC) return SR_NOTOPEN ;

	if ((rs = lockrw_wrlock(&mmp->rwm,to_lock)) >= 0) {

	    if (dt == 0)
	        dt = time(NULL) ;

	    if ((dt - mmp->ti_check) >= TO_MAPCHECK) {

#if	CF_PARAMFILE
	        if ((rs = paramfile_check(&mmp->dirsfile,dt)) > 0) {

	            {
	                mapper_mapfins(mmp) ;
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
	                    mapper_mapfins(mmp) ;
	                    vechand_delall(&mmp->mapdirs) ;
	                }

	                rs = mapper_mapload(mmp) ;
	                nchanged = rs ;

	            } /* end if (file mtime check) */

	            mmp->ti_check = dt ;
	        }
#endif /* CF_PARAMFILE */

	    } /* end if (map-object check) */

	    rs1 = lockrw_unlock(&mmp->rwm) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (read-write lock) */

#if	CF_DEBUGS
	debugprintf("statmsg/mapper_check: ret rs=%d nchanged=%u\n",
		rs,nchanged) ;
#endif

	return (rs >= 0) ? nchanged : rs ;
}
/* end subroutine (mapper_check) */


static int mapper_process(STATMSG_MAPPER *mmp,cchar **ev,cchar *adms[],
		cchar gn[],cchar kn[],int fd)
{
	const int	to_lock = TO_LOCK ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		wlen = 0 ;

	if (mmp == NULL) return SR_FAULT ;

	if (mmp->magic != STATMSG_MAPPERMAGIC) return SR_NOTOPEN ;

#if	CF_DEBUGN
	nprintf(NDEBFNAME,"statmsg/mapper_process: g=%s k=%s\n",gn,kn) ;
#endif
#if	CF_DEBUGS
	debugprintf("statmsg/mapper_process: enter\n") ;
#endif

	if ((rs = lockrw_rdlock(&mmp->rwm,to_lock)) >= 0) {

#if	CF_DEBUGS
	    debugprintf("statmsg/mapper_process: gn=%s\n",gn) ;
	    if (adms != NULL) {
	        int	i ;
	        for (i = 0 ; adms[i] != NULL ; i += 1)
	            debugprintf("statmsg/mapper_process: a%u=%s\n",
			i,adms[i]) ;
	    }
#endif /* CF_DEBUGS */

	    rs = mapper_processor(mmp,ev,adms,gn,kn,fd) ;
	    wlen += rs ;

#if	CF_DEBUGS
	    debugprintf("statmsg/mapper_process: mapper_processor() rs=%d\n",
		rs) ;
#endif

	    rs1 = lockrw_unlock(&mmp->rwm) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (read-write lock) */

#if	CF_DEBUGS
	debugprintf("statmsg/mapper_process: finished rs=%d\n",rs) ;
#endif

#if	CF_DEBUGS
	debugprintf("statmsg/mapper_process: ret rs=%d\n",rs) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (mapper_process) */


static int mapper_processor(STATMSG_MAPPER *mmp,cchar *ev[],cchar *adms[],
		cchar gn[],cchar kn[],int fd)
{
	STATMSG_MAPDIR	*ep ;
	int		rs = SR_OK ;
	int		i ;
	int		wlen = 0 ;

	if (mmp == NULL) return SR_FAULT ;

	if (mmp->magic != STATMSG_MAPPERMAGIC) return SR_NOTOPEN ;

#if	CF_DEBUGN
	nprintf(NDEBFNAME,"statmsg/mapper_processor: g=%s k=%s\n",
		gn,kn) ;
#endif
#if	CF_DEBUGS
	debugprintf("statmsg/mapper_processor: gn=%s\n",gn) ;
#endif

	for (i = 0 ; vechand_get(&mmp->mapdirs,i,&ep) >= 0 ; i += 1) {
	    if (ep != NULL) {
	        rs = mapdir_process(ep,ev,adms,gn,kn,fd) ;
	        wlen += rs ;
	    }
	    if (rs < 0) break ;
	} /* end for */

#if	CF_DEBUGS
	debugprintf("statmsg/mapper_processor: ret rs=%d wlen=%u\n",rs,wlen) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (mapper_processor) */


#if	CF_PARAMFILE

static int mapper_mapload(STATMSG_MAPPER *mmp)
{
	struct ustat	sb ;
	PARAMFILE	*pfp = &mmp->dirsfile ;
	PARAMFILE_ENT	pe ;
	PARAMFILE_CUR	cur ;
	int		rs = SR_OK ;
	int		c = 0 ;

	if (mmp == NULL) return SR_FAULT ;

	if (mmp->magic != STATMSG_MAPPERMAGIC) return SR_NOTOPEN ;

#if	CF_DEBUGN
	nprintf(NDEBFNAME,"statmsg/mapper_mapload: ent\n") ;
#endif
	if ((rs = u_stat(mmp->fname,&sb)) >= 0) {
	    mmp->ti_mtime = sb.st_mtime ;
	    if ((rs = paramfile_curbegin(pfp,&cur)) >= 0) {
	        const int	plen = PBUFLEN ;
		int		kl, vl ;
		int		fl ;
		cchar		*kp, *vp ;
		char		pbuf[PBUFLEN + 1] ;

	        while (rs >= 0) {

	            kl = paramfile_enum(pfp,&cur,&pe,pbuf,plen) ;
	            if (kl == SR_NOTFOUND) break ;
	            rs = kl ;
	            if (rs < 0) break ;

	            kp = pe.key ;
	            vp = pe.value ;
	            vl = pe.vlen ;

	            while ((fl = sichr(vp,vl,CH_FS)) >= 0) {
			if (fl > 0) {
	                    c += 1 ;
	                    rs = mapper_mapadd(mmp,kp,kl,vp,fl) ;
			}
			vl -= (fl+1) ;
			vp = (vp+(fl+1)) ;
	                if (rs < 0) break ;
	            } /* end while */

		    if ((rs >= 0) && (vl > 0)) {
	                c += 1 ;
	                rs = mapper_mapadd(mmp,kp,kl,vp,vl) ;
		    }

	        } /* end while */

	        paramfile_curend(&mmp->dirsfile,&cur) ;
	    } /* end if (paramfile-cursor) */
	} else if (isNotPresent(rs)) {
	    rs = SR_OK ;
	} /* end if (stat) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (mapper_mapload) */

#else /* CF_PARAMFILE */

static int mapper_mapload(STATMSG_MAPPER *mmp)
{
	bfile		mfile, *mfp = &mfile ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		kl, vl ;
	int		sl ;
	int		c = 0 ;
	cchar		*tp, *sp ;
	cchar		*kp, *vp ;

	if (mmp == NULL) return SR_FAULT ;

	if (mmp->magic != STATMSG_MAPPERMAGIC) return SR_NOTOPEN ;

	if ((rs1 = bopen(mfp,mmp->fname,"r",0666)) >= 0) {
	    struct ustat	sb ;
	    if ((rs = bcontrol(mfp,BC_STAT,&sb)) >= 0) {
		const int	llen ;
		int		len ;
		char		lbuf[LINEBUFLEN + 1] ;

		mmp->ti_mtime = sb.st_mtime ;
		while ((rs = breadline(mfp,lbuf,llen)) > 0) {
	    	    len = rs ;

	    sp = lbuf ;
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

	    	if (rs < 0) break ;
		} /* end while (reading lines) */

	    } /* end if (bcontrol) */
	    bclose(mfp) ;
	} /* end if (file-open) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (mapper_mapload) */

#endif /* CF_PARAMFILE */


static int mapper_mapadd(STATMSG_MAPPER *mmp,cchar *kp,int kl,cchar *vp,int vl)
{
	STATMSG_MAPDIR	*ep ;
	const int	size = sizeof(STATMSG_MAPDIR) ;
	int		rs ;

	if ((kp == NULL) || (vp == NULL)) return SR_FAULT ;
	if ((kl == 0) || (vl == 0)) return SR_INVALID ;

#if	CF_DEBUGN
	nprintf(NDEBFNAME,"statmsg/mapper_mapadd: k=%t\n",kp,kl) ;
#endif
	if ((rs = uc_malloc(size,&ep)) >= 0) {
	    cchar	*un = mmp->username ;
	    cchar	*uh = mmp->userhome ;
	    if ((rs = mapdir_start(ep,un,uh,kp,kl,vp,vl)) >= 0) {
	        rs = vechand_add(&mmp->mapdirs,ep) ;
	        if (rs < 0)
	            mapdir_finish(ep) ;
	    }
	    if (rs < 0)
	        uc_free(ep) ;
	} /* end if (memory-allocation) */

	return rs ;
}
/* end subroutine (mapper_mapadd) */


static int mapper_mapfins(STATMSG_MAPPER *mmp)
{
	STATMSG_MAPDIR	*ep ;
	vechand		*mlp = &mmp->mapdirs ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		i ;

	if (mmp == NULL) return SR_FAULT ;

	if (mmp->magic != STATMSG_MAPPERMAGIC) return SR_NOTOPEN ;

	for (i = 0 ; vechand_get(mlp,i,&ep) >= 0 ; i += 1) {
	    if (ep != NULL) {
	        rs1 = mapdir_finish(ep) ;
	        if (rs >= 0) rs = rs1 ;
	        rs1 = vechand_del(mlp,i--) ;
	        if (rs >= 0) rs = rs1 ;
	        rs1 = uc_free(ep) ;
	        if (rs >= 0) rs = rs1 ;
	    }
	} /* end for */

	return rs ;
}
/* end subroutine (mapper_mapfins) */


static int mapdir_start(STATMSG_MAPDIR *ep,cchar *un,cchar *uh,
		cchar *kp,int kl,cchar *vp,int vl)
{
	int		rs = SR_OK ;

	if (ep == NULL) return SR_FAULT ;
	if (kp == NULL) return SR_FAULT ;
	if (vp == NULL) return SR_FAULT ;

	if ((kl == 0) || (vl == 0)) return SR_INVALID ;

#if	CF_DEBUGN
	nprintf(NDEBFNAME,"statmsg/mapdir_start: k=%t\n",kp,kl) ;
#endif
	memset(ep,0,sizeof(STATMSG_MAPDIR)) ;
	ep->username = un ;
	ep->userhome = uh ;

	if (kl < 0)
	    kl = strlen(kp) ;

	if (vl < 0)
	    vl = strlen(vp) ;

	{
	    const int	size = (kl + 1 + vl + 1) ;
	    char	*bp ;
	    if ((rs = uc_malloc(size,&bp)) >= 0) {
	        ep->admin = bp ;
		bp = strwcpy(bp,kp,kl) + 1 ;
		ep->dirname = bp ;
		bp = strwcpy(bp,vp,vl) + 1 ;
		rs = lockrw_create(&ep->rwm,0) ;
		if (rs < 0) {
	    	    uc_free(ep->admin) ;
		    ep->admin = NULL ;
		}
	    } /* end if (memory-allocation) */
	} /* end block */

	return rs ;
}
/* end subroutine (mapdir_start) */


static int mapdir_finish(STATMSG_MAPDIR *ep)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (ep == NULL) return SR_FAULT ;

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


static int mapdir_process(STATMSG_MAPDIR *ep,cchar **ev,cchar **adms,
		cchar *gn,cchar *kn,int fd)
{
	const int	to_lock = TO_LOCK ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		wlen = 0 ;

#if	CF_DEBUGN
	nprintf(NDEBFNAME,"statmsg/mapdir_process: g=%s k=%s\n",gn,kn) ;
	nprintf(NDEBFNAME,"statmsg/mapdir_process: dir=%s\n",ep->dirname) ;
#endif
#if	CF_DEBUGS
	{
	    int	i ;
	    debugprintf("statmsg/mapdir_process: ent kn=%s\n",kn) ;
	    debugprintf("statmsg/mapdir_process: dirname=%s\n",ep->dirname) ;
	    if (adms != NULL) {
	        for (i = 0 ; adms[i] != NULL ; i += 1) {
	            debugprintf("statmsg/mapdir_process: a[%u]=%s\n",
			i,adms[i]) ;
		}
	    }
	}
#endif /* CF_DEBUGS */

	if (ep->dirname[0] != '\0') {
	    int	f_continue = TRUE ;
	    if ((adms != NULL) && (adms[0] != NULL)) {
	        f_continue = (matstr(adms,ep->admin,-1) >= 0) ;
	    } /* end if (adms) */
	    if (f_continue) {
	        if ((ep->dirname[0] != '/') && (ep->dname == NULL)) {
	            rs = mapdir_expand(ep) ;
	        }
	        if (rs >= 0) {
	            if ((ep->dirname[0] == '/') || (ep->dname != NULL)) {
	                if ((rs = lockrw_rdlock(&ep->rwm,to_lock)) >= 0) {
			    cchar	*dn = ep->dirname ;
	                    if ((dn[0] != '~') || (ep->dname != NULL)) {
	                        rs = mapdir_processor(ep,ev,gn,kn,fd) ;
	            		wlen += rs ;
	    		    } /* end if */
	    		    rs1 = lockrw_unlock(&ep->rwm) ;
	    		    if (rs >= 0) rs = rs1 ;
			} /* end if (locked) */
		    } /* end if (acceptable) */
		} /* end if (ok) */
	    } /* end if (continued) */
	} /* end if */

#if	CF_DEBUGS
	debugprintf("statmsg/mapdir_process: ret rs=%d wlen=%u\n",rs,wlen) ;
#endif
#if	CF_DEBUGN
	nprintf(NDEBFNAME,"statmsg/mapdir_process: ret rs=%d wlen=%u\n",
		rs,wlen) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (mapdir_process) */


static int mapdir_expand(STATMSG_MAPDIR *ep)
{
	const int	to_lock = TO_LOCK ;
	int		rs ;
	int		rs1 ;

#if	CF_DEBUGN
	nprintf(NDEBFNAME,"statmsg/mapdir_expand: dir=%s\n",ep->dirname) ;
#endif
#if	CF_DEBUGS
	debugprintf("statmsg/mapdir_expand: dirname=%s\n",ep->dirname) ;
	debugprintf("statmsg/mapdir_expand: dname=%s\n",ep->dname) ;
#endif

	if ((rs = lockrw_wrlock(&ep->rwm,to_lock)) >= 0) {

	    if ((ep->dirname[0] != '/') && (ep->dname == NULL)) {
	        rs = mapdir_expander(ep) ;

#if	CF_DEBUGS
	        debugprintf("statmsg/mapdir_expand: mapdir_expander() rs=%d\n",
			rs) ;
	        debugprintf("statmsg/mapdir_expand: dname=%s\n",ep->dname) ;
#endif

	    } /* end if */

	    rs1 = lockrw_unlock(&ep->rwm) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (read-write lock) */

#if	CF_DEBUGS
	debugprintf("statmsg/mapdir_expand: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (mapdir_expand) */


static int mapdir_expander(STATMSG_MAPDIR *ep)
{
	int		rs = SR_OK ;
	int		pl = 0 ;

#if	CF_DEBUGS
	debugprintf("statmsg/mapdir_expander: dirname=%s\n",ep->dirname) ;
#endif

	if (ep->dirname != NULL) {
	    cchar	*pp ;
	    char	tmpfname[MAXPATHLEN + 1] ;

	if (ep->dirname[0] == '~') {
#if	CF_DEBUGN
	nprintf(NDEBFNAME,"statmsg/mapdir_expander: dir=%s\n",ep->dirname) ;
#endif
	    pp = tmpfname ;
	    rs = mkuserpath(tmpfname,ep->username,ep->dirname,-1) ;
	    pl = rs ;
#if	CF_DEBUGN
	nprintf(NDEBFNAME,"statmsg/mapdir_expander: "
		"mkuserpath() rs=%d r=%s\n",rs,pp) ;
#endif
	} else if (ep->dirname[0] != '/') {
	    pp = tmpfname ;
	    rs = mkpath2(tmpfname,ep->userhome,ep->dirname) ;
	    pl = rs ;
	} else {
	    pp = ep->dirname ;
	    pl = -1 ;
	}
	   
	    if (rs >= 0) {
		cchar	*cp ;
	        rs = uc_mallocstrw(pp,pl,&cp) ;
	        if (rs >= 0) ep->dname = cp ;
		if (pl < 0) pl = (rs-1) ;
	    }

	} else
		rs = SR_INVALID ;

	return (rs >= 0) ? pl : rs ;
}
/* end subroutine (mapdir_expander) */


/* ARGSUSED */
static int mapdir_processor(STATMSG_MAPDIR *ep,cchar **ev,cchar *gn,
		cchar *kn,int fd)
{
	struct ustat	sb ;
	VECSTR		nums ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		n ;
	int		wlen = 0 ;
	int		f_continue = TRUE ;
	cchar		*defname = STATMSG_DEFGROUP ;
	cchar		*allname = STATMSG_ALLGROUP ;
	cchar		*suf = STATMSG_SUF ;
	cchar		*dn ;
	char		env_admin[ENVBUFLEN+1] ;
	char		env_admindir[ENVBUFLEN+1] ;

#if	CF_DEBUGS
	debugprintf("STATMSG/mapdir_processor: dir=%s\n",ep->dirname) ;
	debugprintf("STATMSG/mapdir_processor: kn=%s\n",kn) ;
#endif

	dn = ep->dirname ;
	if (dn[0] == '~') {
	    dn = ep->dname ;
	    f_continue = ((dn != NULL) && (dn[0] != '\0')) ;
	}
	if (f_continue) {
	    if ((rs1 = u_stat(dn,&sb)) >= 0) {
	        const int	envlen = ENVBUFLEN ;
	        cchar		*post ;
	        post = envstrs[envstr_admin] ;
	        strdcpy4(env_admin,envlen,envpre,post,"=",ep->admin) ;
	        post = envstrs[envstr_admindir] ;
	        strdcpy4(env_admindir,envlen,envpre,post,"=",dn) ;
	        for (n = 0 ; ev[n] != NULL ; n += 1) ;
	        ev[n+0] = env_admin ;
	        ev[n+1] = env_admindir ;
	        ev[n+2] = NULL ;
	        if ((rs = vecstr_start(&nums,0,0)) >= 0) {
	            FSDIR	d ;
	            FSDIR_ENT	de ;
	            int		i ;
	            cchar	*strs[5] ;
	            loadstrs(strs,kn,defname,allname,suf) ;
	            if ((rs = fsdir_open(&d,dn)) >= 0) {
	                cchar	*tp ;
	                while ((rs = fsdir_read(&d,&de)) > 0) {
	                    cchar	*den = de.name ;
	                    if (den[0] != '.') {
	            	        tp = strchr(den,'.') ;
	            		if ((tp != NULL) && (strcmp((tp+1),suf) == 0)) {
				    int		f = TRUE ;
	   			    cchar	*digp ;
	                	    digp = strnpbrk(den,(tp-den),"0123456789") ;
	                	    if (digp != NULL) {
	                    		f = hasalldig(digp,(tp-digp)) ;
				    }
				    if (f) {
	                	        if ((kn[0] != '\0') && (kn[0] != '-')) {
			    		    for (i = 0 ; i < 3 ; i += 1) {
					    f = isBaseMatch(den,strs[i],digp) ;
					    if (f) break ;
			    		    } /* end for */
	                		}
				    }
	                	    if (f) {
	                    		rs = vecstr_add(&nums,den,(tp-den)) ;
				    }
	            		} /* end if (have an STATMSG file) */
			    }
	           	    if (rs < 0) break ;
	        	} /* end while (reading directory entries) */
	        	rs1 = fsdir_close(&d) ;
			if (rs >= 0) rs = rs1 ;
	  	    } /* end if (fsdir) */
		    if (rs >= 0) {
	        	vecstr_sort(&nums,NULL) ;
	        	rs = mapdir_processorthem(ep,ev,dn,&nums,strs,fd) ;
	        	wlen += rs ;
	   	    } /* end if */
	   	    vecstr_finish(&nums) ;
		} /* end if (vecstr-nums) */
		{
	    	    ev[n] = NULL ;
		}
	    } /* end if (u_stat) */
	} /* end if (continued) */

#if	CF_DEBUGS
	debugprintf("STATMSG/mapdir_processor: ret rs=%d wlen=%u\n",rs,wlen) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (mapdir_processor) */


static int mapdir_processorthem(STATMSG_MAPDIR *ep,cchar **ev,cchar *dn,
		VECSTR *blp,cchar **strs,int fd)
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		wlen = 0 ;
	cchar		*kn ;
	char		kbuf[2] ;

	kn = strs[0] ;

	if (kn[0] == '-') {
	    kn = kbuf ;
	    kbuf[0] = '\0' ;
	}

	rs1 = mapdir_processorone(ep,ev,dn,blp,kn,fd) ;

	if (kn[0] != '\0') {

	if (isNotPresent(rs1)) {
	    kn = strs[1] ;
	    rs1 = mapdir_processorone(ep,ev,dn,blp,kn,fd) ;
	    if (! isNotPresent(rs1)) rs = rs1 ;
	} else {
	    rs = rs1 ;
	}
	if (rs > 0) wlen += rs ;
	if (rs >= 0) {
	    kn = strs[2] ;
	    rs1 = mapdir_processorone(ep,ev,dn,blp,kn,fd) ;
	    if (! isNotPresent(rs1)) rs = rs1 ;
	    if (rs > 0) wlen += rs ;
	}

	} /* end if */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (mapdir_processorthem) */


static int mapdir_processorone(STATMSG_MAPDIR *ep,cchar **ev,cchar *dn,
		VECSTR *blp,cchar *kn,int fd)
{
	const int	kl = strlen(kn) ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		i ;
	int		c = 0 ;
	int		wlen = 0 ;
	cchar		*bep ;

#if	CF_DEBUGS
	debugprintf("STATMSG/mapdir_processorone: kn=%s\n",kn) ;
#endif

	for (i = 0 ; vecstr_get(blp,i,&bep) >= 0 ; i += 1) {
	    if (bep != NULL) {

#if	CF_DEBUGS
	    debugprintf("STATMSG/mapdir_processorone: bep=%s\n",bep) ;
#endif

	    if (strncmp(bep,kn,kl) == 0) {
	        c += 1 ;
	        rs1 = mapdir_procout(ep,ev,dn,bep,fd) ;

#if	CF_DEBUGS
	        debugprintf("STATMSG/mapdir_processorone: _procout() rs=%d\n",
			rs1) ;
#endif

	        if (rs1 >= 0) {
	            wlen += rs1 ;
	        } else if (! isNotPresent(rs1))
	            rs = rs1 ;
	    }

	    }
	    if (rs < 0) break ;
	} /* end for */

	if ((rs >= 0) && (c == 0)) rs = SR_NOENT ;

#if	CF_DEBUGS
	debugprintf("STATMSG/mapdir_processorone: ret rs=%d wlen=%u\n",
		rs,wlen) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (mapdir_processorone) */


/* we must return SR_NOENT if there was no file */
static int mapdir_procout(STATMSG_MAPDIR *ep,cchar **ev,cchar *dn,cchar *bn,
		int fd)
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		wlen = 0 ;
	cchar		*suf = STATMSG_SUF ;
	char		cname[MAXNAMELEN + 1] ;
	char		fname[MAXPATHLEN + 1] ;

/* we ignore buffer overflow here */

	rs1 = snsds(cname,MAXNAMELEN,bn,suf) ;
	if (rs1 >= 0)
	    rs1 = mkpath2(fname,dn,cname) ;

	if (rs1 >= 0) {
	    rs = mapdir_procouter(ep,ev,fname,fd) ;
	    wlen += rs ;
	}

	if ((rs >= 0) && (rs1 == SR_OVERFLOW)) rs = SR_NOENT ;

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (mapdir_procout) */


static int mapdir_procouter(STATMSG_MAPDIR *ep,cchar **ev,cchar *fn,int ofd)
{
	const mode_t	operms = 0664 ;
	const int	oflags = O_RDONLY ;
	const int	to_open = TO_OPEN ;
	const int	to_read = TO_READ ;
	const int	to_write = TO_WRITE ;
	int		rs ;
	int		wlen = 0 ;

	if (ep == NULL) return SR_FAULT ;

#if	CF_DEBUGS
	debugprintf("statmsg/mapdir_procouter: fname=%s\n",fn) ;
#endif

	if ((rs = uc_openenv(fn,oflags,operms,ev,to_open)) >= 0) {
	    const int	mfd = rs ;
	    const int	olen = MSGBUFLEN ;
	    char	obuf[MSGBUFLEN + 1] ;
#if	CF_DEBUGS
	    debugprintf("statmsg/mapdir_procouter: uc_openenv() rs=%d\n",rs) ;
#endif
#if	CF_DEBUGN
	    nprintf(NDEBFNAME,"statmsg/mapdir_procouter: uc_openenv() rs=%d\n",
		rs) ;
#endif

#if	CF_WRITETO
	    while ((rs = uc_reade(mfd,obuf,olen,to_read,0)) > 0) {
	        rs = writeto(ofd,obuf,rs,to_write) ;
	        wlen += rs ;
	        if (rs < 0) break ;
	    } /* end while */
#else /* CF_WRITETO */
	    rs = uc_writedesc(ofd,mfd,-1) ;
	    wlen += rs ;
#endif /* CF_WRITETO */

	    u_close(mfd) ;
	} /* end if (open) */

#if	CF_DEBUGS
	debugprintf("statmsg/mapdir_procouter: ret rs=%d wlen=%u\n",rs,wlen) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (mapdir_procouter) */


#if	CF_WRITETO

static int writeto(int wfd,cchar *wbuf,int wlen,int wto)
{
	struct pollfd	fds[2] ;
	time_t		dt = time(NULL) ;
	time_t		ti_write ;
	int		rs = SR_OK ;
	int		i ;
	int		pt = TO_POLL ;
	int		pto ;
	int		tlen = 0 ;

	if (wbuf == NULL) return SR_FAULT ;

	if (wfd < 0) return SR_BADF ;

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

	ti_write = dt ;
	pto = (pt * POLLMULT) ;
	while ((rs >= 0) && (tlen < wlen)) {

	    rs = u_poll(fds,1,pto) ;

	    dt = time(NULL) ;
	    if (rs > 0) {
	        const int	re = fds[0].revents ;

	        if (re & POLLOUT) {
	            rs = u_write(wfd,(wbuf+tlen),(wlen-tlen)) ;
	            tlen += rs ;
	            ti_write = dt ;
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

	    if ((dt - ti_write) >= wto)
	        break ;

	} /* end while */

	return (rs >= 0) ? tlen : rs ;
}
/* end subroutine (writeto) */

#endif /* CF_WRITETO */


static int loadstrs(cchar **strs,cchar *gn,cchar *def,cchar *all,cchar *name)
{
	int		i = 0 ;

#ifdef	COMMENT
	{
	    cchar	*cp ;
	    for (i = 0 ; i < 4 ; i += 1) {
	        switch (i) {
	        case 0: 
	            cp = gn ; 
	            break ;
	        case 1: 
	            cp = def ; 
	            break ;
	        case 2: 
	            cp = all ; 
	            break ;
	        case 3: 
	            cp = name ; 
	            break ;
	        } /* end switch */
	        strs[i] = cp ;
	    } /* end for */
	}
#else
	strs[i++] = gn ;
	strs[i++] = def ;
	strs[i++] = all ;
	strs[i++] = name ;
#endif /* COMMENT */
	strs[i] = NULL ;

	return SR_OK ;
}
/* end subroutine (loadstrs) */


static int isBaseMatch(cchar *den,cchar *bname,cchar *digp)
{
	int		f = FALSE ;

	if (digp == NULL) {
	    int	bl = strlen(bname) ;
	    int	m = nleadstr(den,bname,bl) ;
	    f = (m == bl) && (den[m] == '.') ;
	} else
	    f = (strncmp(den,bname,(digp-den)) == 0) ;

	return f ;
}
/* end subroutine (isBaseMatch) */


