/* motd */

/* object to help (manage) MOTD messages */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */
#define	CF_DEBUGN	0		/* extra-special debugging */
#define	CF_WRITETO	1		/* time out writes */
#define	CF_TESTPROC	0		/* test using 'uc_openfsvc(3uc)' */
#define	CF_FINDUID	1		/* use 'finduid(3c)' */
#define	CF_UGETPW	1		/* use |ugetpw(3uc)| */


/* revision history:

	= 2003-10-01, David A­D­ Morano
        This is a hack from numerous previous hacks (not enumerated here). This
        is a new version of this hack that is entirely different (much simpler).

*/

/* Copyright © 2003 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This object module writes the contents of various MOTDs (as specified by
        the caller) to an open file descriptor (also specified by the caller).

	Implementation notes:

        When processing, we time-out writes to the caller-supplied
        file-descriptor because we don't know if it is a non-regular file that
        might be flow-controlled. We don't wait forever for those sorts of
        outputs. So let's say that the output is a terminal that is currently
        flow-controlled. We will time-out on our writes and the user will not
        get this whole MOTD text!

	The FINDUID feature:

        This has got to be a feature that has more code than is ever executed of
        all features. This feature handles an extremely small corner case where
        there are two or more USERNAMEs sharing a single UID (in the system
        PASSWD database). Further, the code comes into play when one of the
        users is already logged in and one of the other users sharing the same
        UID goes to log in. Without this code a random username among those
        sharing the same UID would be selected for the new user logging in. The
        reason for this is that in daemon mode we only get UIDs back from the
        kernel on a connection request. So we have to guess what the
        corresponding username might be for that connection request. With the
        FINDUID feature, we do this guessing a little bit more intelligently by
        using the username from that last user with the given UID who logged
        into the system (by searching the system UTMPX database). The latest
        user logged in will get his own username (within a few split seconds or
        so) but a consequence is that all other users sharing that same UID will
        also see this same username. But this is not usually a big problem since
        the read-out of the MOTD file is usually done at login time and often
        only done at that time. Outside of daemon mode, or stand-alone mode, the
        feature does not come into play and the correct username (within
        extremely broad limits) is always divined. So there it is, good and bad.
        But there are not a lot of ways to handle it better and this feature
        already handles these cases much better than nothing at all.


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
#include	<ugetpw.h>
#include	<getxusername.h>
#include	<ptma.h>
#include	<ptm.h>
#include	<lockrw.h>
#include	<paramfile.h>
#include	<strpack.h>
#include	<bfile.h>
#include	<ascii.h>
#include	<localmisc.h>

#include	"motd.h"


/* local defines */

#define	MOTD_MAPDIR	struct motd_mapdir
#define	MOTD_DEFGROUP	"default"
#define	MOTD_ALLGROUP	"all"
#define	MOTD_NAME	"motd"
#define	MOTD_DIRSFNAME	"dirs"

#define	MOTD_MAPPERMAGIC	0x21367425

#if	CF_UGETPW
#define	GETPW_NAME	ugetpw_name
#else
#define	GETPW_NAME	getpw_name
#endif /* CF_UGETPW */

#define	NDF		"motd.deb"

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

extern int	snsds(char *,int,cchar *,cchar *) ;
extern int	sncpy1(char *,int,cchar *) ;
extern int	sncpy2(char *,int,cchar *,cchar *) ;
extern int	sncpy3(char *,int,cchar *,cchar *,cchar *) ;
extern int	sncpy4(char *,int,cchar *,cchar *,cchar *,
			cchar *) ;
extern int	sncpylc(char *,int,cchar *) ;
extern int	sncpyuc(char *,int,cchar *) ;
extern int	mkpath1(char *,cchar *) ;
extern int	mkpath2(char *,cchar *,cchar *) ;
extern int	mkpath3(char *,cchar *,cchar *,cchar *) ;
extern int	mknpath1(char *,int,cchar *) ;
extern int	mknpath2(char *,int,cchar *,cchar *) ;
extern int	mknpath3(char *,int,cchar *,cchar *,cchar *) ;
extern int	sichr(cchar *,int,int) ;
extern int	matstr(cchar **,cchar *,int) ;
extern int	nleadstr(cchar *,cchar *,int) ;
extern int	sperm(IDS *,struct ustat *,int) ;
extern int	permsched(cchar **,vecstr *,char *,int,cchar *,int) ;
extern int	getgid_group(cchar *,int) ;
extern int	getnodedomain(char *,char *) ;
extern int	ctdecui(char *,int,uint) ;
extern int	opentmp(cchar *,int,mode_t) ;
extern int	vecstr_envset(vecstr *,cchar *,cchar *,int) ;
extern int	msleep(int) ;
extern int	haslc(cchar *,int) ;
extern int	hasuc(cchar *,int) ;
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
extern char	*strdcpy4(char *,int,cchar *,cchar *,cchar *,cchar *) ;
extern char	*strdcpy1w(char *,int,cchar *,int) ;


/* external variables */

extern cchar	**environ ;


/* local structures */

struct motd_mapdir {
	LOCKRW		rwm ;
	cchar		*admin ;
	cchar		*dirname ;	/* raw */
	cchar		*dname ;	/* expanded */
} ;


/* forward references */

int 		motd_processid(MOTD *,MOTD_ID *,cchar **,int) ;

static int	motd_mapfind(MOTD *,time_t) ;
static int	motd_maplose(MOTD *) ;
static int	motd_mapfname(MOTD *,char *) ;
static int	motd_schedload(MOTD *,vecstr *) ;
static int	motd_checker(MOTD *,time_t) ;
static int	motd_envbegin(MOTD *) ;
static int	motd_envend(MOTD *) ;
static int	motd_envadds(MOTD *,STRPACK *,cchar **,MOTD_ID *) ;
static int	motd_envstore(MOTD *,STRPACK *,cchar **,int,cchar *,int) ;
static int 	motd_processor(MOTD *,cchar **,cchar **,cchar *,int) ;
static int	motd_idcheck(MOTD *,MOTD_ID *,char *) ;
static int	motd_ufindbegin(MOTD *) ;
static int	motd_ufindend(MOTD *) ;
static int	motd_ufindlook(MOTD *,char *,uid_t) ;

static int	mapper_start(MOTD_MAPPER *,time_t,cchar *) ;
static int	mapper_finish(MOTD_MAPPER *) ;
static int	mapper_check(MOTD_MAPPER *,time_t) ;
static int	mapper_process(MOTD_MAPPER *,cchar **,cchar **,cchar *,int) ;
static int	mapper_processor(MOTD_MAPPER *,cchar **,cchar **,cchar *,int) ;
static int	mapper_mapload(MOTD_MAPPER *) ;
static int	mapper_mapadd(MOTD_MAPPER *,cchar *,int,cchar *,int) ;
static int	mapper_mapfins(MOTD_MAPPER *) ;

#if	CF_TESTPROC
static int	mapper_lockcheck(MOTD_MAPPER *,cchar *) ;
#endif

static int	mapdir_start(MOTD_MAPDIR *,cchar *,int,cchar *,int) ;
static int	mapdir_finish(MOTD_MAPDIR *) ;
static int	mapdir_process(MOTD_MAPDIR *,cchar **,cchar **,cchar *,int) ;
static int	mapdir_expand(MOTD_MAPDIR *) ;
static int	mapdir_expander(MOTD_MAPDIR *) ;
static int	mapdir_processor(MOTD_MAPDIR *,cchar **,cchar *,int) ;
static int	mapdir_procout(MOTD_MAPDIR *,cchar **,cchar *,cchar *,int) ;
static int	mapdir_procouter(MOTD_MAPDIR *,cchar **,cchar *,int) ;

static int	writeto(int,cchar *,int,int) ;


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

static cchar	*envpre = "MOTD_" ;	/* environment prefix */


/* exported subroutines */


int motd_open(MOTD *op,cchar pr[])
{
	const time_t	dt = time(NULL) ;
	int		rs ;
	cchar		*cp ;

	if (op == NULL) return SR_FAULT ;
	if (pr == NULL) return SR_FAULT ;

	if (pr[0] == '\0') return SR_INVALID ;

#if	CF_DEBUGS
	debugprintf("motd_open: sizeof(MOTD)=%u\n",sizeof(MOTD)) ;
#endif

	memset(op,0,sizeof(MOTD)) ;
	op->fe = MOTD_DIRSFNAME ;

	if ((rs = uc_mallocstrw(pr,-1,&cp)) >= 0) {
	    op->pr = cp ;
	    if ((rs = ptm_create(&op->m,NULL)) >= 0) {
	        if ((rs = motd_mapfind(op,dt)) >= 0) {
	            if ((rs = motd_envbegin(op)) >= 0) {
	                op->ti_lastcheck = dt ;
	                op->magic = MOTD_MAGIC ;
	            } /* end if (envbegin) */
	            if (rs < 0)
	                motd_maplose(op) ;
	        } /* end if (mapfind) */
	        if (rs < 0)
	            ptm_destroy(&op->m) ;
	    } /* end if (ptm_begin) */
	    if (rs < 0) {
		uc_free(op->pr) ;
		op->pr = NULL ;
	    }
	} /* end if (memory-allocation) */

#if	CF_DEBUGS
	debugprintf("motd_open: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (motd_open) */


int motd_close(MOTD *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != MOTD_MAGIC) return SR_NOTOPEN ;

	rs1 = motd_ufindend(op) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = motd_envend(op) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = motd_maplose(op) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = ptm_destroy(&op->m) ;
	if (rs >= 0) rs = rs1 ;

	if (op->pr != NULL) {
	    rs1 = uc_free(op->pr) ;
	    if (rs >= 0) rs = rs1 ;
	    op->pr = NULL ;
	}

	op->magic = 0 ;
	return rs ;
}
/* end subroutine (motd_close) */


int motd_check(MOTD *op,time_t dt)
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != MOTD_MAGIC) return SR_NOTOPEN ;

	rs = motd_checker(op,dt) ;

	return rs ;
}
/* end subroutine (motd_check) */


int motd_process(MOTD *op,cchar groupname[],cchar *admins[],int fd)
{
	MOTD_ID		id ;
	int		rs ;

	memset(&id,0,sizeof(MOTD_ID)) ;
	id.groupname = groupname ;
	id.uid = -1 ;
	id.gid = -1 ;
	rs = motd_processid(op,&id,admins,fd) ;

	return rs ;
}
/* end subroutine (motd_process) */


int motd_processid(MOTD *op,MOTD_ID *idp,cchar *admins[],int fd)
{
	MOTD_ID		id ;
	int		rs ;
	int		wlen = 0 ;
	cchar		*groupname ;
	char		ubuf[USERNAMELEN + 1] ;

	if (op == NULL) return SR_FAULT ;
	if (idp == NULL) return SR_FAULT ;

	if (op->magic != MOTD_MAGIC) return SR_NOTOPEN ;

	if (fd < 0) return SR_BADF ;

	groupname = idp->groupname ;
	if (groupname == NULL)
	    return SR_FAULT ;

	if (groupname[0] == '\0')
	    return SR_INVALID ;

#if	CF_DEBUGS
	{
	    debugprintf("motd_processid: tar groupname=%s\n",groupname) ;
	    debugprintf("motd_processid: tar username=%s\n",idp->username) ;
	    debugprintf("motd_processid: tar uid=%d\n",idp->uid) ;
	    if (admins != NULL) {
	        int	i ;
	        for (i = 0 ; admins[i] != NULL ; i += 1) {
	            debugprintf("motd_processid: a[%u]=%s\n",i,admins[i]) ;
		}
	    }
	}
#endif /* CF_DEBUGS */

/* fill in some missing elements */

	id = *idp ;			/* copy for possible modification */
	if ((rs = motd_idcheck(op,&id,ubuf)) >= 0) {
	    const int	n = nelem(envstrs) ;
	    int		size ;
	    void	*p ;
	    size = (op->nenv + n + 1) * sizeof(cchar *) ;
	    if ((rs = uc_malloc(size,&p)) >= 0) {
	        STRPACK	packer ;
	        cchar	**ev = (cchar **) p ;
	        if ((rs = strpack_start(&packer,128)) >= 0) {
	            if ((rs = motd_envadds(op,&packer,ev,&id)) >= 0) {
	                rs = motd_processor(op,ev,admins,groupname,fd) ;
	                wlen = rs ;
	            }
	            strpack_finish(&packer) ;
	        } /* end if (packer) */
	        uc_free(p) ;
	    } /* end if (memory allocation) */
	} /* end if (motd_idcheck) */

#if	CF_DEBUGS
	debugprintf("motd_processid: ret rs=%d wlen=%u\n",rs,wlen) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (motd_processid) */


int motdid_load(MOTD_ID *idp,cchar *un,cchar *gn,uid_t uid,gid_t gid)
{

	if (idp == NULL) return SR_FAULT ;

	memset(idp,0,sizeof(MOTD_ID)) ;
	idp->uid = uid ;
	idp->gid = gid ;
	idp->username = un ;
	idp->groupname = gn ;

	return SR_OK ;
}
/* end subroutine (motdid_load) */


/* private subroutines */


static int motd_mapfind(MOTD *op,time_t dt)
{
	int		rs ;
	char		mapfname[MAXPATHLEN + 1] ;

	mapfname[0] = '\0' ;
	if ((rs = motd_mapfname(op,mapfname)) >= 0) {
	    if (mapfname[0] != '\0') {
	        if ((rs = mapper_start(&op->mapper,dt,mapfname)) >= 0) {
	            op->nmaps += 1 ;
		}
	    }
	}

#if	CF_DEBUGS
	debugprintf("motd_mapfind: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (motd_mapfind) */


static int motd_maplose(MOTD *op)
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
/* end subroutine (motd_maplose) */


static int motd_mapfname(MOTD *op,char fbuf[])
{
	vecstr		scheds ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		c = 0 ;

	fbuf[0] = '\0' ;
	if ((rs = vecstr_start(&scheds,6,0)) >= 0) {
	    if ((rs = motd_schedload(op,&scheds)) >= 0) {
	        const int	flen = MAXPATHLEN ;

	        rs1 = permsched(schedmaps,&scheds,fbuf,flen,op->fe,R_OK) ;

#if	CF_DEBUGS
	        debugprintf("motd_mapfname: permsched() rs=%d\n",rs1) ;
#endif

	        if ((rs1 == SR_NOENT) || (rs1 == SR_ACCESS)) {
	            if (rs1 == SR_NOENT) {
	                fbuf[0] = '\0' ;
	            } else {
	                rs = rs1 ;
		    }
	        } else if (rs1 == SR_OK) {
	            c = 1 ;
	        } else {
	            rs = rs1 ;
	  	}

	    } /* end if (motd-schedload) */
	    rs1 = vecstr_finish(&scheds) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (scheds) */

#if	CF_DEBUGS
	debugprintf("motd_mapfname: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (motd_mapfname) */


static int motd_schedload(MOTD *op,vecstr *slp)
{
	int		rs = SR_OK ;
	int		i ;
	cchar		*keys = "pen" ;
	cchar		*name = MOTD_NAME ;
	for (i = 0 ; keys[i] != '\0' ; i += 1) {
	    const int	kch = MKCHAR(keys[i]) ;
	    int		vl = -1 ;
	    cchar	*vp = NULL ;
	    switch (kch) {
	    case 'p':
		vp = op->pr ;
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
/* end subroutine (motd_schedload) */


static int motd_checker(MOTD *op,time_t dt)
{
	int		rs = SR_OK ;
	int		nchanged = 0 ;
	if (op->nmaps > 0) {
	    if ((rs = ptm_lock(&op->m)) >= 0) {
	        if (dt == 0) dt = time(NULL) ;
	        if ((dt - op->ti_lastcheck) >= TO_CHECK) {
	            rs = mapper_check(&op->mapper,dt) ;
	            nchanged = rs ;
	            op->ti_lastcheck = dt ;
	        } /* end if */
	        ptm_unlock(&op->m) ;
	    } /* end if (mutex) */
	} /* end if (positive) */
	return (rs >= 0) ? nchanged : rs ;
}
/* end subroutine (motd_checker) */


static int motd_envbegin(MOTD *op)
{
	const int	es = envpre[0] ;
	const int	envprelen = strlen(envpre) ;
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
	        f = f && (ep[0] != '_') ;
	        f = f && (matstr(envbad,ep,-1) < 0) ;
	        if (f && (ep[0] == es)) {
	            f = (strncmp(envpre,ep,envprelen) != 0) ;
		}
	        if (f) {
	            va[c++] = ep ;
		}
	    } /* end for */
	    va[c] = NULL ;
	    op->nenv = c ;
	} /* end if (memory-allocation) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (motd_envbegin) */


static int motd_envend(MOTD *op)
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
/* end subroutine (motd_envend) */


static int motd_envadds(MOTD *op,STRPACK *spp,cchar **ev,MOTD_ID *idp)
{
	const int	envlen = ENVBUFLEN ;
	uint		uv ;
	int		rs = SR_OK ;
	int		n, i ;
	int		el ;
	cchar		**envv = op->envv ;
	cchar		*pre = envpre ;
	cchar		*cp ;
	char		envbuf[ENVBUFLEN + 1] ;
	char		digbuf[DIGBUFLEN + 1] ;

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
	        rs = motd_envstore(op,spp,ev,n,envbuf,el) ;
	        if (rs > 0) n += 1 ;
	    }
	} /* end for */
	ev[n] = NULL ; /* very important! */

	return (rs >= 0) ? n : rs ;
}
/* end subroutine (motd_envadds) */


static int motd_envstore(MOTD *op,STRPACK *spp,cchar *ev[],int n,
		cchar *ep,int el)
{
	int		rs = SR_OK ;
	cchar		*cp ;

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
/* end subroutine (motd_envstore) */


static int motd_idcheck(MOTD *op,MOTD_ID *idp,char *ubuf)
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
	    rs = getgid_group(idp->groupname,-1) ;
	    idp->gid = rs ;
	}

#if	CF_DEBUGS
	debugprintf("motd_idcheck: tar un=%s\n",idp->username) ;
#endif

	if (rs >= 0) {
	    cchar	*tun = idp->username ;
	    if ((tun == NULL) || (tun[0] == '\0') || (tun[0] == '-')) {
	        ubuf[0] = '\0' ;
	        rs = SR_OK ; /* needed for later test */
#if	CF_FINDUID
	        if ((tun == NULL) || (tun[0] == '\0')) {
	            rs = motd_ufindlook(op,ubuf,idp->uid) ;
	        }
#endif /* CF_FINDUID */
	        if (rs == SR_OK) { /* this is the "later" test */
	            rs = getusername(ubuf,USERNAMELEN,idp->uid) ;
		}
	        idp->username = ubuf ;
	    }
	} /* end if (ok) */

#if	CF_DEBUGS
	debugprintf("motd_idcheck: ret rs=%d un=%s\n",rs,idp->username) ;
#endif

	return rs ;
}
/* end subroutine (motd_idcheck) */


static int motd_processor(MOTD *op,cchar **ev,cchar *adms[],cchar *gn,int fd)
{
	int		rs ;
	int		wlen = 0 ;

#if	CF_DEBUGS
	debugprintf("motd_processor: ent gn=%s\n",gn) ;
#endif

	if ((rs = motd_checker(op,0)) >= 0) {
	    if (op->nmaps > 0) {
	        rs = mapper_process(&op->mapper,ev,adms,gn,fd) ;
	        wlen += rs ;
	    }
	}

#if	CF_DEBUGS
	debugprintf("motd_processor: ret rs=%d wlen=%u\n",rs,wlen) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (motd_processor) */


static int motd_ufindbegin(MOTD *op)
{
	const int	maxent = 30 ;
	const int	ttl = (1*60*60) ;
	int		rs = SR_OK ;

	if (! op->open.ufind) {
	    rs = finduid_start(&op->ufind,NULL,maxent,ttl) ;
	    op->open.ufind = (rs >= 0) ;
	}

	return rs ;
}
/* end subroutine (motd_ufindbegin) */


static int motd_ufindend(MOTD *op)
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
/* end subroutine (motd_ufindend) */


static int motd_ufindlook(MOTD *op,char *ubuf,uid_t uid)
{
	int		rs ;
	int		rs1 ;
	int		ul = 0 ;

	if ((rs = ptm_lock(&op->m)) >= 0) {
	    if (! op->open.ufind) rs = motd_ufindbegin(op) ;
	    if (rs >= 0) {
	        rs = finduid_lookup(&op->ufind,ubuf,uid) ;
		ul = rs ;
	    } /* end if */
	    rs1 = ptm_unlock(&op->m) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (mutex) */

/* "not-found" is a zero return, "found" is > zero */

	return (rs >= 0) ? ul : rs ;
}
/* end subroutine (motd_ufindlook) */


static int mapper_start(MOTD_MAPPER *mmp,time_t dt,cchar fname[])
{
	int		rs ;
	cchar		**evp = (cchar **) environ ;
	cchar		*ccp ;

#if	CF_DEBUGS
	debugprintf("mapper_start: sizeof(PTM)=%u\n", sizeof(PTM)) ;
#endif

	memset(mmp,0,sizeof(MOTD_MAPPER)) ;

	if ((rs = lockrw_create(&mmp->rwm,0)) >= 0) {
	    if ((rs = uc_mallocstrw(fname,-1,&ccp)) >= 0) {
		mmp->fname = ccp ;
		if ((rs = vechand_start(&mmp->mapdirs,4,0)) >= 0) {
		    cchar	*fn = mmp->fname ;
		    if ((rs = paramfile_open(&mmp->dirsfile,evp,fn)) >= 0) {
			const int	to = TO_MAPCHECK ;
			if ((rs = paramfile_checkint(&mmp->dirsfile,to)) >= 0) {
	    		    mmp->magic = MOTD_MAPPERMAGIC ;
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
	debugprintf("motd/mapper_start: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (mapper_start) */


static int mapper_finish(MOTD_MAPPER *mmp)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (mmp == NULL) return SR_FAULT ;

	if (mmp->magic != MOTD_MAPPERMAGIC) return SR_NOTOPEN ;

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
	debugprintf("motd/mapper_finish: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (mapper_finish) */


static int mapper_check(MOTD_MAPPER *mmp,time_t dt)
{
	const int	to_lock = TO_LOCK ;
	int		rs ;
	int		rs1 ;
	int		nchanged = 0 ;

	if (mmp == NULL) return SR_FAULT ;

	if (mmp->magic != MOTD_MAPPERMAGIC) return SR_NOTOPEN ;

	if ((rs = lockrw_wrlock(&mmp->rwm,to_lock)) >= 0) {
	    const int	to = TO_MAPCHECK ;

	    if (dt == 0) dt = time(NULL) ;

	    if ((dt - mmp->ti_check) >= to) {

	        if ((rs = paramfile_check(&mmp->dirsfile,dt)) > 0) {

	            {
	                mapper_mapfins(mmp) ;
	                vechand_delall(&mmp->mapdirs) ;
	            }

	            rs = mapper_mapload(mmp) ;
	            nchanged = rs ;

	        } /* end if */

	    } /* end if (map-object check) */

	    rs1 = lockrw_unlock(&mmp->rwm) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (read-write lock) */

#if	CF_DEBUGS
	debugprintf("motd/mapper_check: ret rs=%d nchanged=%u\n",rs,nchanged) ;
#endif

	return (rs >= 0) ? nchanged : rs ;
}
/* end subroutine (mapper_check) */


static int mapper_process(MOTD_MAPPER *mmp,cchar **ev,cchar **adms,
		cchar *gn,int fd)
{
	const int	to_lock = TO_LOCK ;
	int		rs ;
	int		rs1 ;
	int		wlen = 0 ;

	if (mmp == NULL) return SR_FAULT ;

	if (mmp->magic != MOTD_MAPPERMAGIC) return SR_NOTOPEN ;

#if	CF_DEBUGS
	debugprintf("motd/mapper_process: ent\n") ;
#endif

	if ((rs = lockrw_rdlock(&mmp->rwm,to_lock)) >= 0) {

#if	CF_DEBUGS
	    debugprintf("motd/mapper_process: gn=%s\n",gn) ;
	    if (adms != NULL) {
	        int	i ;
	        for (i = 0 ; adms[i] != NULL ; i += 1) {
	            debugprintf("motd/mapper_process: a%u=%s\n",i,adms[i]) ;
		}
	    }
#endif /* CF_DEBUGS */

#if	CF_TESTPROC
	    {
	        const mode_t	om = 0666 ;
	        const int	of = O_RDONLY ;
	        const int	to = 5 ;
	        cchar		*pr = "/home/genserv" ;
	        cchar		*prn = "genserv" ;
	        cchar		*svc = "hello" ;
	        cchar		*argv[2] ;

	        argv[0] = svc ;
	        argv[1] = NULL ;
	        if ((rs = uc_openfsvc(pr,prn,svc,of,om,argv,ev,to)) >= 0) {
	            const int	rfd = rs ;
	            char	buf[BUFLEN+1] ;
	            while ((rs = uc_reade(rfd,buf,BUFLEN,5,0)) > 0) {
	                int	len = rs ;
	                rs = writeto(fd,buf,len,10) ;
	                wlen = rs ;
	                if (rs < 0) break ;
	            } /* end while */
	            u_close(rfd) ;
	        } /* end if (uc_openfsvc) */
	    }
#else /* CF_TESTPROC */
	    rs = mapper_processor(mmp,ev,adms,gn,fd) ;
	    wlen += rs ;
#endif /* CF_TESTPROC */

#if	CF_DEBUGS
	    debugprintf("motd/mapper_process: mapper_processor() rs=%d\n",rs) ;
#endif

	    rs1 = lockrw_unlock(&mmp->rwm) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (read-write lock) */

#if	CF_DEBUGS
	debugprintf("motd/mapper_process: finished rs=%d\n",rs) ;
#endif

#if	CF_DEBUGS && CF_TESTPROC
	if (rs >= 0) {
	    rs = mapper_lockcheck(mmp,"post1") ;
	    debugprintf("motd_processor: mapper_lockcheck() rs=%d\n",rs) ;
	}
#endif

#if	CF_DEBUGS
	debugprintf("motd/mapper_process: ret rs=%d\n",rs) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (mapper_process) */


static int mapper_processor(MOTD_MAPPER *mmp,cchar **ev,cchar **adms,
		cchar	*gn,int fd)
{
	MOTD_MAPDIR	*ep ;
	int		rs = SR_OK ;
	int		i ;
	int		wlen = 0 ;

	if (mmp == NULL) return SR_FAULT ;

	if (mmp->magic != MOTD_MAPPERMAGIC) return SR_NOTOPEN ;

#if	CF_DEBUGS
	debugprintf("motd/mapper_processor: gn=%s\n",gn) ;
#endif

	for (i = 0 ; vechand_get(&mmp->mapdirs,i,&ep) >= 0 ; i += 1) {
	    if (ep != NULL) {
	        rs = mapdir_process(ep,ev,adms,gn,fd) ;
	        wlen += rs ;
	    }
	    if (rs < 0) break ;
	} /* end for */

#if	CF_DEBUGS
	debugprintf("motd/mapper_processor: ret rs=%d wlen=%u\n",rs,wlen) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (mapper_processor) */


static int mapper_mapload(MOTD_MAPPER *mmp)
{
	struct ustat	sb ;
	int		rs = SR_OK ;
	int		c = 0 ;

	if (mmp == NULL) return SR_FAULT ;

	if (mmp->magic != MOTD_MAPPERMAGIC) return SR_NOTOPEN ;

	if (u_stat(mmp->fname,&sb) >= 0) {
	    PARAMFILE		*pfp = &mmp->dirsfile ;
	    PARAMFILE_ENT	pe ;
	    PARAMFILE_CUR	cur ;
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

	        paramfile_curend(pfp,&cur) ;
	    } /* end if (paramfile-cursor) */
	} /* end if (stat) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (mapper_mapload) */


static int mapper_mapadd(MOTD_MAPPER *mmp,cchar *kp,int kl,cchar *vp,int vl)
{
	MOTD_MAPDIR	*ep ;
	const int	size = sizeof(MOTD_MAPDIR) ;
	int		rs ;

	if ((kp == NULL) || (vp == NULL)) return SR_FAULT ;

	if ((kl == 0) || (vl == 0)) return SR_INVALID ;

	if ((rs = uc_malloc(size,&ep)) >= 0) {
	    if ((rs = mapdir_start(ep,kp,kl,vp,vl)) >= 0) {
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


static int mapper_mapfins(MOTD_MAPPER *mmp)
{
	MOTD_MAPDIR	*ep ;
	vechand		*mlp = &mmp->mapdirs ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		i ;

	if (mmp == NULL) return SR_FAULT ;

	if (mmp->magic != MOTD_MAPPERMAGIC) return SR_NOTOPEN ;

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


#if	CF_TESTPROC

static int mapper_lockcheck(MOTD_MAPPER *mmp,cchar *s)
{
	const int	to_lock = TO_LOCK ;
	int		rs ;
	int		rs1 ;

	if (mmp == NULL) return SR_FAULT ;

	if (mmp->magic != MOTD_MAPPERMAGIC) return SR_NOTOPEN ;

#if	CF_DEBUGS
	debugprintf("motd/mapper_lockcheck: ent s=%s\n",s) ;
#endif

	if ((rs = lockrw_rdlock(&mmp->rwm,to_lock)) >= 0) {
	    rs1 = lockrw_unlock(&mmp->rwm) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (read-write lock) */

#if	CF_DEBUGS
	debugprintf("motd/mapper_lockcheck: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (mapper_lockcheck) */

#endif /* CF_TESTPROC */


static int mapdir_start(MOTD_MAPDIR *ep,cchar *kp,int kl,cchar *vp,int vl)
{
	int		rs ;

	if (ep == NULL) return SR_FAULT ;
	if ((kp == NULL) || (vp == NULL)) return SR_FAULT ;

	if ((kl == 0) || (vl == 0)) return SR_INVALID ;

	memset(ep,0,sizeof(MOTD_MAPDIR)) ;

	if (kl < 0)
	    kl = strlen(kp) ;

	if (vl < 0)
	    vl = strlen(vp) ;

	{
	    const int	size = (kl + 1 + vl + 1) ;
	    char	*p ;
	    if ((rs = uc_malloc(size,&p)) >= 0) {
	        char	*bp = p ;
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
	} /* end if (memory-allocation) */

	return rs ;
}
/* end subroutine (mapdir_start) */


static int mapdir_finish(MOTD_MAPDIR *ep)
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


static int mapdir_process(MOTD_MAPDIR *ep,cchar **ev,cchar **admins,
		cchar *gn,int fd)
{
	const int	to_lock = TO_LOCK ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		wlen = 0 ;

#if	CF_DEBUGS
	{
	    int	i ;
	    debugprintf("motd/mapdir_process: ent gn=%s\n",gn) ;
	    debugprintf("motd/mapdir_process: dirname=%s\n",ep->dirname) ;
	    if (admins != NULL) {
	        for (i = 0 ; admins[i] != NULL ; i += 1) {
	            debugprintf("motd/mapdir_process: a[%u]=%s\n",
	                i,admins[i]) ;
		}
	    }
	}
#endif /* CF_DEBUGS */

	if (ep->dirname[0] != '\0') {
	    int		f_continue = TRUE ;
	    if ((admins != NULL) && (admins[0] != NULL)) {
	        f_continue = (matstr(admins,ep->admin,-1) >= 0) ;
	    } /* end if (admins) */
	    if (f_continue) {
	        if ((ep->dirname[0] == '~') && (ep->dname == NULL)) {
	            rs = mapdir_expand(ep) ;
	        }
	        if (rs >= 0) {
	            if ((ep->dirname[0] != '~') || (ep->dname != NULL)) {
	                if ((rs = lockrw_rdlock(&ep->rwm,to_lock)) >= 0) {
			    cchar	*dn = ep->dirname ;
	                    if ((dn[0] != '~') || (ep->dname != NULL)) {
	            		rs = mapdir_processor(ep,ev,gn,fd) ;
	            		wlen += rs ;
	    		    } /* end if */
	    		    rs1 = lockrw_unlock(&ep->rwm) ;
	    		    if (rs >= 0) rs = rs1 ;
			} /* end if (locked) */
		    } /* end if (ready) */
		} /* end if (ok) */
	    } /* end if (continued) */
	} /* end if (non-nul) */

#if	CF_DEBUGS
	debugprintf("motd/mapdir_process: ret rs=%d wlen=%u\n",rs,wlen) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (mapdir_process) */


static int mapdir_expand(MOTD_MAPDIR *ep)
{
	const int	to_lock = TO_LOCK ;
	int		rs ;
	int		rs1 ;

#if	CF_DEBUGS
	debugprintf("motd/mapdir_expand: dirname=%s\n",ep->dirname) ;
	debugprintf("motd/mapdir_expand: dname=%s\n",ep->dname) ;
#endif

	if ((rs =  lockrw_wrlock(&ep->rwm,to_lock)) >= 0) {

	    if ((ep->dirname[0] == '~') && (ep->dname == NULL)) {
	        rs = mapdir_expander(ep) ;

#if	CF_DEBUGS
	        debugprintf("motd/mapdir_expand: mapdir_expander() rs=%d\n",
	            rs) ;
	        debugprintf("motd/mapdir_expand: dname=%s\n",ep->dname) ;
#endif

	    } /* end if */

	    rs1 = lockrw_unlock(&ep->rwm) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (read-write lock) */

#if	CF_DEBUGS
	debugprintf("motd/mapdir_expand: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (mapdir_expand) */


static int mapdir_expander(MOTD_MAPDIR *ep)
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		fl = 0 ;

#if	CF_DEBUGS
	debugprintf("motd/mapdir_expander: dirname=%s\n",ep->dirname) ;
#endif

	if ((ep->dirname != NULL) && (ep->dirname[0] == '~')) {
	    int		unl = -1 ;
	    cchar	*un = (ep->dirname+1) ;
	    cchar	*tp ;
	    cchar	*pp = NULL ;
	    char	ubuf[USERNAMELEN + 1] ;
	    if ((tp = strchr(un,'/')) != NULL) {
	        unl = (tp - un) ;
	        pp = tp ;
	    }
	    if ((unl == 0) || (un[0] == '\0')) {
	        un = ep->admin ;
	        unl = -1 ;
	    }
	    if (unl >= 0) {
	   	strwcpy(ubuf,un,MIN(unl,USERNAMELEN)) ;
		un = ubuf ;
	    }
	    if ((rs = getbufsize(getbufsize_pw)) >= 0) {
	        struct passwd	pw ;
	        const int	pwlen = rs ;
	        char		*pwbuf ;
	        if ((rs = uc_malloc((pwlen+1),&pwbuf)) >= 0) {
	            if ((rs = GETPW_NAME(&pw,pwbuf,pwlen,un)) >= 0) {
		        cchar	*uh = pw.pw_dir ;
	    	        char	hbuf[MAXPATHLEN + 1] ;
	                if (pp != NULL) {
	                    rs = mkpath2(hbuf,uh,pp) ;
	                    fl = rs ;
	                } else {
	                    rs = mkpath1(hbuf,uh) ;
	                    fl = rs ;
	                }
	                if (rs >= 0) {
	                    cchar	*cp ;
	                    rs = uc_mallocstrw(hbuf,fl,&cp) ;
	                    if (rs >= 0) ep->dname = cp ;
	                }
		    } else if (isNotPresent(rs)) {
		        rs = SR_OK ;
	            } /* end if (getpw_name) */
	            rs1 = uc_free(pwbuf) ;
		    if (rs >= 0) rs = rs1 ;
	        } /* end if (m-a-f) */
	    } /* end if (getbufsize) */
	} else {
	    rs = SR_INVALID ;
	}

	return (rs >= 0) ? fl : rs ;
}
/* end subroutine (mapdir_expander) */


static int mapdir_processor(MOTD_MAPDIR *ep,cchar **ev,cchar *gn,int fd)
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		wlen = 0 ;
	int		f_continue = TRUE ;
	cchar		*dn = ep->dirname ;

#if	CF_DEBUGS
	debugprintf("motd/mapdir_processor: ent gn=%s\n",gn) ;
#endif

	if (dn[0] == '~') {
	    dn = ep->dname ;
	    f_continue = ((dn != NULL) && (dn[0] != '\0')) ;
	}
	if (f_continue) {
	    const int	envlen = ENVBUFLEN ;
	    int		n ;
	    cchar	*pre = envpre ;
	    cchar	*post ;
	    cchar	*defname = MOTD_DEFGROUP ;
	    cchar	*allname = MOTD_ALLGROUP ;
	    char	env_admin[ENVBUFLEN+1] ;
	    char	env_admindir[ENVBUFLEN+1] ;
	    post = envstrs[envstr_admin] ;
	    strdcpy4(env_admin,envlen,pre,post,"=",ep->admin) ;
	    post = envstrs[envstr_admindir] ;
	    strdcpy4(env_admindir,envlen,pre,post,"=",dn) ;
	    for (n = 0 ; ev[n] != NULL ; n += 1) ;
	    ev[n+0] = env_admin ;
	    ev[n+1] = env_admindir ;
	    ev[n+2] = NULL ;
	    rs1 = mapdir_procout(ep,ev,dn,gn,fd) ;
	    if (isNotPresent(rs1)) {
	        gn = defname ;
	        rs1 = mapdir_procout(ep,ev,dn,gn,fd) ;
	        if (! isNotPresent(rs1)) rs = rs1 ;
	    } else {
	        rs = rs1 ;
	    }
	    if (rs > 0) wlen += rs ;
	    if (rs >= 0) {
	        gn = allname ;
	        rs1 = mapdir_procout(ep,ev,dn,gn,fd) ;
	        if (! isNotPresent(rs1)) rs = rs1 ;
	        if (rs > 0) wlen += rs ;
	    }
	    {
	        ev[n] = NULL ;
	    }
	} /* end if (continued) */

#if	CF_DEBUGS
	debugprintf("motd/mapdir_processor: ret rs=%d wlen=%u\n",rs,wlen) ;
#endif
#if	CF_DEBUGN
	nprintf(NDF,"motd/mapdir_processor: ret rs=%d wlen=%u\n",
	    rs,wlen) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (mapdir_processor) */


/* we must return SR_NOENT if there was no file */
static int mapdir_procout(MOTD_MAPDIR *ep,cchar **ev,cchar *dn,
		cchar	*gn,int fd)
{
	const int	clen = MAXNAMELEN ;
	int		rs ;
	int		wlen = 0 ;
	cchar		*name = MOTD_NAME ;
	char		cbuf[MAXNAMELEN + 1] ;

/* we ignore buffer overflow here */

	if ((rs = snsds(cbuf,clen,gn,name)) >= 0) {
	    char	fname[MAXPATHLEN + 1] ;
	    if ((rs = mkpath2(fname,dn,cbuf)) >= 0) {
	        rs = mapdir_procouter(ep,ev,fname,fd) ;
	        wlen += rs ;
	    } else if (rs == SR_OVERFLOW) {
	        rs = SR_NOENT ;
	    }
	} else if (rs == SR_OVERFLOW) {
	    rs = SR_NOENT ;
	}

#if	CF_DEBUGS
	debugprintf("issue/mapdir_procout: ret rs=%d wlen=%u\n",rs,wlen) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (mapdir_procout) */


static int mapdir_procouter(MOTD_MAPDIR *ep,cchar **ev,cchar *fname,int ofd)
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
	debugprintf("motd/mapdir_procouter: fname=%s\n",fname) ;
#endif

	if ((rs = uc_openenv(fname,oflags,operms,ev,to_open)) >= 0) {
	    const int	mfd = rs ;
	    const int	olen = MSGBUFLEN ;
	    char	obuf[MSGBUFLEN + 1] ;
#if	CF_DEBUGS
	    debugprintf("motd/mapdir_procouter: uc_openenv() rs=%d\n",rs) ;
#endif
#if	CF_DEBUGN
	    nprintf(NDF,"motd/mapdir_procouter: uc_openenv() rs=%d\n",
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
	debugprintf("motd/mapdir_procouter: ret rs=%d wlen=%u\n",rs,wlen) ;
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

	    } else if (rs == SR_INTR) {
		rs = SR_OK ;
	    } /* end if (got something) */

	    if ((dt - ti_write) >= wto) break ;
	} /* end while */

	return (rs >= 0) ? tlen : rs ;
}
/* end subroutine (writeto) */

#endif /* CF_WRITETO */


