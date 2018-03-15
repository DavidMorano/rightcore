/* pmq */

/* Posix Message Queue (PMQ) */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_CONDUNLINK	1		/* conditional unlink */
#define	CF_UGETPW	1		/* use |ugetpw(3uc)| */


/* revision history:

	= 1999-07-23, David A­D­ Morano
	This module was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This module provides a sanitized version of the standard POSIX® message
	queue facility provided with some new UNIX®i.  Some operating system
	problems are managed within these routines for the common stuff that
	happens when a poorly configured OS gets overloaded!

	Enjoy!


*******************************************************************************/


#define	PMQ_MASTER	0


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<string.h>
#include	<errno.h>

#include	<vsystem.h>
#include	<getbufsize.h>
#include	<ugetpw.h>
#include	<localmisc.h>

#include	"pmq.h"


/* local defines */

#if	CF_UGETPW
#define	GETPW_NAME	ugetpw_name
#define	GETPW_UID	ugetpw_uid
#else
#define	GETPW_NAME	getpw_name
#define	GETPW_UID	getpw_uid
#endif /* CF_UGETPW */

#define	PMQ_PATHPREFIX	"/tmp/pmq"

#define	PMQ_CHOWNVAR	_PC_CHOWN_RESTRICTED

#define	PMQ_USERNAME1	"sys"
#define	PMQ_USERNAME2	"adm"

#define	PMQ_GROUPNAME1	"sys"
#define	PMQ_GROUPNAME2	"adm"

#define	PMQ_UID		3
#define	PMQ_GID		3

#define	TO_NOSPC	5
#define	TO_MFILE	5


/* external subroutines */

extern int	mkpath2(char *,const char *,const char *) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	getgid_group(cchar *,int) ;
extern int	msleep(int) ;
extern int	isNotValid(int) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;


/* forward references */

int		uc_unlinkpmq(const char *) ;

static int	pmqdircheck(const char *) ;
static int	pmqdiradd(const char *,mode_t) ;
static int	pmqdirrm(const char *) ;

static int	getpmquid(void) ;
static int	getpmqgid(void) ;


/* local variables */


/* exported subroutines */


int pmq_open(PMQ *mqp,cchar *name,int of,mode_t om,struct mq_attr *attr)
{
	int		rs ;
	int		to_mfile = TO_MFILE ;
	int		to_nospc = TO_NOSPC ;
	int		f_exit = FALSE ;
	char		altname[MAXNAMELEN + 1] ;

	if (mqp == NULL) return SR_FAULT ;
	if (name == NULL) return SR_FAULT ;

	if (name[0] == '\0') return SR_INVALID ;

#if	CF_DEBUGS
	debugprintf("pmq_open: name=%s\n",name) ;
#endif

	if (name[0] != '/') {
	    sncpy2(altname,MAXNAMELEN,"/",name) ;
	    name = altname ;
	}

#if	CF_DEBUGS
	debugprintf("pmq_open: pmq_name=%s\n",name) ;
#endif

	memset(mqp,0,sizeof(PMQ)) ;
	mqp->name[0] = '\0' ;

	repeat {
	    rs = SR_OK ;
	    if ((mqp->p = mq_open(name,of,om,attr)) == ((mqd_t) -1)) {
		rs = (- errno) ;
	    }
	    if (rs < 0) {
	        switch (rs) {
	        case SR_MFILE:
	        case SR_NFILE:
	            if (to_mfile-- > 0) {
	                msleep(1000) ;
		    } else {
			f_exit = TRUE ;
		    }
		    break ;
	        case SR_NOSPC:
	            if (to_nospc-- > 0) {
	                msleep(1000) ;
		    } else {
			f_exit = TRUE ;
		    }
		    break ;
	        case SR_INTR:
	            break ;
		default:
		    f_exit = TRUE ;
		    break ;
	        } /* end switch */
	    } /* end if (ok) */
	} until ((rs >= 0) || f_exit) ;

	if (rs >= 0) {
	    strwcpy(mqp->name,name,MAXPATHLEN) ;
	    if (of & O_CREAT) pmqdiradd(name,om) ;
	    mqp->magic = PMQ_MAGIC ;
	} /* end if (opened) */

	return rs ;
}
/* end subroutine (pmq_open) */


int pmq_close(PMQ *mqp)
{
	int		rs ;
	int		f_exit = FALSE ;

	if (mqp == NULL) return SR_FAULT ;

	if (mqp->magic != PMQ_MAGIC) return SR_NOTOPEN ;

	repeat {
	    if ((rs = mq_close(mqp->p)) < 0) rs = (- errno) ;
	    if (rs < 0) {
	        switch (rs) {
	        case SR_INTR:
	            break ;
		default:
		    f_exit = TRUE ;
		    break ;
	        } /* end switch */
	    } /* end if (error) */
	} until ((rs >= 0) || f_exit) ;

	mqp->magic = 0 ;
	return rs ;
}
/* end subroutine (pmq_close) */


int pmq_send(PMQ *mqp,cchar *rbuf,int rlen,uint prio)
{
	int		rs ;
	int		f_exit = FALSE ;

	if (mqp == NULL) return SR_FAULT ;
	if (rbuf == NULL) return SR_FAULT ;

	if (mqp->magic != PMQ_MAGIC) return SR_NOTOPEN ;

	repeat {
	    if ((rs = mq_send(mqp->p,rbuf,rlen,prio)) < 0) rs = (- errno) ;
	    if (rs < 0) {
	        switch (rs) {
		case SR_INTR:
		    break ;
		default:
		    f_exit = TRUE ;
		    break ;
	        } /* end switch */
	    } /* end if (error) */
	} until ((rs >= 0) || f_exit) ;

	return rs ;
}
/* end subroutine (pmq_send) */


int pmq_receive(PMQ *mqp,char *rbuf,int rlen,uint *priop)
{
	int		rs ;
	int		f_exit = FALSE ;

	if (mqp == NULL) return SR_FAULT ;
	if (rbuf == NULL) return SR_FAULT ;

	if (mqp->magic != PMQ_MAGIC) return SR_NOTOPEN ;

#if	CF_DEBUGS
	debugprintf("pmq_receive: ent rlen=%d\n",rlen) ;
#endif

	repeat {
	    if ((rs = mq_receive(mqp->p,rbuf,rlen,priop)) < 0) rs = (- errno) ;
	    if (rs < 0) {
		switch (rs) {
		case SR_INTR:
	    	    break ;
		default:
		    f_exit = TRUE ;
		    break ;
		} /* end switch */
	    } /* end if (error) */
	} until ((rs >= 0) || f_exit) ;

#if	CF_DEBUGS
	debugprintf("pmq_receive: ret len=%d buf(max 10)=>%t<\n",
	    rs,buf,MIN(rs,10)) ;
#endif

	return rs ;
}
/* end subroutine (pmq_receive) */


int pmq_unlink(PMQ *mqp)
{
	int		rs = SR_OK ;

	if (mqp == NULL) return SR_FAULT ;

	if (mqp->magic != PMQ_MAGIC) return SR_NOTOPEN ;

	if (mqp->name[0] != '\0') {

	    rs = uc_unlinkpmq(mqp->name) ;

#if	CF_CONDUNLINK
	    if (rs >= 0)
	        pmqdirrm(mqp->name) ;
#else
	    pmqdirrm(mqp->name) ;
#endif /* CF_CONDUNLINK */

	    mqp->name[0] = '0' ;

	} /* end if */

	return rs ;
}
/* end subroutine (pmq_unlink) */


int pmq_setattr(PMQ *mqp,struct mq_attr *nattr,struct mq_attr *oattr)
{
	int		rs = SR_INVALID ;

	if (mqp == NULL) return SR_FAULT ;

	if (mqp->magic != PMQ_MAGIC) return SR_NOTOPEN ;

	if (nattr != NULL) {
	    rs = SR_OK ;
	    while (rs != SR_INVALID) {
	        if ((rs = mq_setattr(mqp->p,nattr,oattr)) < -1) {
	            rs = (- errno) ;
		}
	    } /* end while */
	} else if (oattr != NULL) {
	    rs = pmq_getattr(mqp,oattr) ;
	}

	return rs ;
}
/* end subroutine (mq_setattr) */


int pmq_getattr(PMQ *mqp,struct mq_attr *oattr)
{
	int		rs ;
	int		f_exit = FALSE ;

	if ((mqp == NULL) || (oattr == NULL)) return SR_FAULT ;

	if (mqp->magic != PMQ_MAGIC) return SR_NOTOPEN ;

	repeat {
	    if ((rs = mq_getattr(mqp->p,oattr)) < -1) rs = (- errno) ;
	    if (rs < 0) {
		switch (rs) {
		case SR_INTR:
	    	    break ;
		default:
		    f_exit = TRUE ;
		    break ;
		} /* end switch */
	    } /* end if (error) */
	} until ((rs >= 0) || f_exit) ;

	return rs ;
}
/* end subroutine (pmq_getattr) */


int pmq_notify(PMQ *mqp,struct sigevent *sep)
{
	int		rs ;
	int		f_exit = FALSE ;

	if (mqp == NULL) return SR_FAULT ;

	if (mqp->magic != PMQ_MAGIC) return SR_NOTOPEN ;

	repeat {
	    if ((rs = mq_notify(mqp->p,sep)) < -1) rs = (- errno) ;
	    if (rs < 0) {
		switch (rs) {
		case SR_INTR:
	    	    break ;
		default:
		    f_exit = TRUE ;
		    break ;
		} /* end switch */
	    } /* end if (error) */
	} until ((rs >= 0) || f_exit) ;

	return rs ;
}
/* end subroutine (pmq_notify) */


/* OTHER API (but related) */


int uc_unlinkpmq(cchar *name)
{
	int		rs ;
	int		f_exit = FALSE ;
	char		altname[MAXNAMELEN + 1] ;

	if (name == NULL) return SR_FAULT ;

	if (name[0] == '\0') return SR_INVALID ;

	if (name[0] != '/') {
	    sncpy2(altname,MAXNAMELEN,"/",name) ;
	    name = altname ;
	}

	repeat {
	    if ((rs = mq_unlink(name)) < -1) rs = (- errno) ;
	    if (rs < 0) {
		switch (rs) {
		case SR_INTR:
	    	    break ;
		default:
		    f_exit = TRUE ;
		    break ;
		} /* end switch */
	    } /* end if (error) */
	} until ((rs >= 0) || f_exit) ;

#if	CF_CONDUNLINK
	if (rs >= 0) {
	    pmqdirrm(name) ;
	}
#else
	pmqdirrm(name) ;
#endif /* CF_CONDUNLINK */

	return rs ;
}
/* end subroutine (uc_unlinkpmq) */


/* local subroutines */


static int pmqdiradd(cchar *name,mode_t om)
{
	int		rs ;
	cchar		*pp = PMQ_PATHPREFIX ;
	char		tmpfname[MAXPATHLEN + 1] ;

	if ((rs = mkpath2(tmpfname,pp,name)) >= 0) {
	    if ((rs = u_creat(tmpfname,om)) == SR_NOENT) {
	        if ((rs = pmqdircheck(pp)) >= 0) {
	            rs = u_creat(tmpfname,om) ;
		}
	    } /* end if (u_creat) */
	    if (rs >= 0) u_close(rs) ;
	} /* end if (mkpath) */

	return rs ;
}
/* end subroutine (pmqdiradd) */


static int pmqdirrm(cchar *name)
{
	int		rs ;
	const char	*pp = PMQ_PATHPREFIX ;
	char		tmpfname[MAXPATHLEN + 1] ;

	if ((rs = mkpath2(tmpfname,pp,name)) >= 0) {
	    rs = u_unlink(tmpfname) ;
	}

	return rs ;
}
/* end subroutine (pmqdirrm) */


static int pmqdircheck(cchar *pp)
{
	struct ustat	sb ;
	int		rs ;

	if ((rs = u_stat(pp,&sb)) == SR_NOENT) {
	    const mode_t	dm = 0777 ;
	    const uid_t		euid = geteuid() ;
	    if ((rs = u_mkdir(pp,dm)) >= 0) {
	        if ((rs = u_chmod(pp,(dm|S_ISVTX))) >= 0) {
		    const int	n = PMQ_CHOWNVAR ;
	            if ((rs = u_pathconf(pp,n,NULL)) == 0) {
	            	if ((rs = getpmquid()) >= 0) {
	    		    const uid_t	uid = rs ;
	                    if (euid != uid) { /* get a GID */
				if ((rs = getpmqgid()) >= 0) {
				    const gid_t	gid = rs ;
				    rs = u_chown(pp,uid,gid) ;
				} /* end if (getpmqgid) */
	                    } /* end if (UIDs different) */
			} /* end if (getpmquid) */
	            } /* end if (pathconf) */
	        } /* end if (CHMOD) */
	    } /* end if (mkdir) */
	} /* end if (directory did not exist) */

	return rs ;
}
/* end subroutine (pmqdircheck) */


static int getpmquid(void)
{
	int		rs ;
	int		rs1 ;
	int		uid = -1 ;
	if ((rs = getbufsize(getbufsize_pw)) >= 0) {
	    struct passwd	pw ;
	    const int		pwlen = rs ;
	    char		*pwbuf ;
	    if ((rs = uc_malloc((pwlen+1),&pwbuf)) >= 0) {
	        const int	nrs = SR_NOTFOUND ;
	        cchar		*un = PMQ_USERNAME1 ;
	        if ((rs = GETPW_NAME(&pw,pwbuf,pwlen,un)) == nrs) {
	    	    un = PMQ_USERNAME2 ;
	            if ((rs = GETPW_NAME(&pw,pwbuf,pwlen,un)) == nrs) {
		        rs = SR_OK ;
		        uid = PMQ_UID ;
		    } else {
	    	        uid = pw.pw_uid ;
		    }
	        } else {
	            uid = pw.pw_uid ;
	        }
	        rs1 = uc_free(pwbuf) ;
		if (rs >= 0) rs = rs1 ;
	    } /* end if (memory-allocation) */
	} /* end if (getbufsize) */
	return (rs >= 0) ? uid : rs ;
}
/* end subroutine (getpmquid) */


static int getpmqgid(void)
{
	const int	nrs = SR_NOTFOUND ;
	int		rs ;
	cchar		*gn = PMQ_GROUPNAME1 ;
	if ((rs = getgid_group(gn,-1)) == nrs) {
	    gn = PMQ_GROUPNAME2 ;
	    if ((rs = getgid_group(gn,-1)) == nrs) {
		rs = PMQ_GID ;
	    }
	}
	return rs ;
}
/* end subroutine (getpmqgid) */


