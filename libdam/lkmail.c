/* lkmail */

/* object to manipulate a mail lock */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_SGIDGROUPS	0		/* SGID to a supplemetal group? */


/* revision history:

	= 1998-07-01, David A­D­ Morano
	This program was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This object is used to manipulate a mail lock file.


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/stat.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<signal.h>
#include	<string.h>

#include	<vsystem.h>
#include	<sbuf.h>
#include	<localmisc.h>

#include	"lkmail.h"


/* local defines */

#ifndef	LKMAIL_AGE
#define	LKMAIL_AGE	(5 * 60)
#endif


/* external subroutines */

extern int	mkpath1w(char *,cchar *,int) ;
extern int	mkfnamesuf1(char *,const char *,const char *) ;
extern int	sfdirname(const char *,int,const char **) ;

extern char	*strwcpy(char *,const char *,int) ;


/* external variables */


/* forward references */


/* local variables */


/* exported subroutines */


int lkmail_start(LKMAIL *mlp,LKMAIL_IDS *idp,cchar *mfname)
{
	int		rs ;
	int		cl ;
	const char	*cp ;
	const char	*suf = ".lock" ;

	if (mlp == NULL) return SR_FAULT ;
	if (idp == NULL) return SR_FAULT ;

	memset(mlp,0,sizeof(LKMAIL)) ;
	mlp->lfd = -1 ;

	if ((rs = mkfnamesuf1(mlp->lockfname,mfname,suf)) >= 0) {
	    mlp->id = *idp ;
	    if (mlp->id.gid_maildir <= 0) {
	        struct ustat	sb ;
	        char		maildname[MAXPATHLEN + 1] ;

	        if ((cl = sfdirname(mlp->lockfname,-1,&cp)) > 0) {
	            if ((rs = mkpath1w(maildname,cp,cl)) >= 0) {
	                if ((rs = u_stat(maildname,&sb)) >= 0) {
	                    mlp->id.uid_maildir = sb.st_uid ;
	                    mlp->id.gid_maildir = sb.st_gid ;
			}
	            }
	 	}

	    } /* end if (gid-maildir) */
	    if (rs >= 0) mlp->magic = LKMAIL_MAGIC ;
	} /* end if (mkfnamesuf) */

	return rs ;
}
/* end subroutine (lkmail_start) */


int lkmail_finish(LKMAIL *mlp)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (mlp == NULL) return SR_FAULT ;

	if (mlp->magic != LKMAIL_MAGIC) return SR_NOTOPEN ;

	if (mlp->lfd >= 0) {
	    rs1 = u_close(mlp->lfd) ;
	    if (rs >= 0) rs = rs1 ;
	}

	memset(mlp,0,sizeof(LKMAIL)) ;
	mlp->lfd = -1 ;
	mlp->magic = 0 ;

	return rs ;
}
/* end subroutine (lkmail_finish) */


int lkmail_create(mlp)
LKMAIL		*mlp ;
{
	struct ustat	sb ;
	const int	nrs = SR_NOENT ;
	int		rs ;
	int		lfd ;

	if (mlp == NULL) return SR_FAULT ;

	if (mlp->magic != LKMAIL_MAGIC) return SR_NOTOPEN ;

#if	CF_DEBUGS
	debugprintf("lkmail_create: ent uid=%d gid=%d\n",
	    mlp->id.uid,mlp->id.gid) ;
	debugprintf("lkmail_create: euid=%d egid=%d\n",
	    mlp->id.euid,mlp->id.egid) ;
	debugprintf("lkmail_create: maildir uid=%d gid=%d\n",
	    mlp->id.uid_maildir,mlp->id.gid_maildir) ;
#endif

	if (mlp->lfd >= 0)
	    u_close(mlp->lfd) ;

/* blow out if the lock file is already there! */

	if ((rs = u_stat(mlp->lockfname,&sb)) == nrs) {

#if	CF_DEBUGS
	debugprintf("lkmail_create: u_stat() rs=%d\n",rs) ;
#endif

/* go ahead and try to create the lock file */

#ifdef	COMMENT
	rs = u_creat(mlp->lockfname,0444) ;
#else
	rs = SR_ACCES ;
#endif

	lfd = rs ;
	if ((rs == SR_ACCES) && (mlp->id.egid != mlp->id.gid)) {

#if	CF_DEBUGS
	    debugprintf("lkmail_create: setting EGID\n") ;
#endif

	    rs = u_setegid(mlp->id.gid_maildir) ;

#if	CF_DEBUGS
	    debugprintf("lkmail_create: u_setegid() rs=%d egid=%d\n",
	        rs,getegid()) ;
#endif

	    if (rs >= 0) {

#if	CF_DEBUGS
	        debugprintf("lkmail_create: SEGID successfull\n") ;
#endif

	        rs = u_creat(mlp->lockfname,0444) ;
	        lfd = rs ;

#if	CF_DEBUGS
	        debugprintf("lkmail_create: u_creat() rs=%d\n",rs) ;
#endif

	        u_setegid(mlp->id.gid) ;

#if	CF_DEBUGS
	        debugprintf("lkmail_create: restored egid=%d\n",
	            getegid()) ;
#endif

	    }

	} /* end if (tryint to SGID to maildir) */

#if	CF_SGIDGROUPS
	if ((rs == SR_ACCES) || (rs == SR_PERM)) {

#if	CF_DEBUGS
	    debugprintf("lkmail_create: setting EGID\n") ;
#endif

	    rs = u_setegid(mlp->id.gid_maildir) ;

#if	CF_DEBUGS
	    debugprintf("lkmail_create: u_setegid() rs=%d\n",rs) ;
#endif

	    if (rs >= 0) {

#if	CF_DEBUGS
	        debugprintf("lkmail_create: SEGID successfull\n") ;
#endif

	        rs = u_creat(mlp->lockfname,0444) ;
	        lfd = rs ;

#if	CF_DEBUGS
	        debugprintf("lkmail_create: u_creat() rs=%d\n",rs) ;
#endif

	        u_setegid(mlp->id.gid) ;

	    }

	} /* end if (tryint to SGID to maildir) */
#endif /* CF_SGIDGROUPS */

	if ((rs == SR_ACCES) || (rs == SR_PERM)) {

#if	CF_DEBUGS
	    debugprintf("lkmail_create: no access\n") ;
#endif

	    rs = u_creat(mlp->lockfname,0444) ;
	    lfd = rs ;

#if	CF_DEBUGS
	    debugprintf("lkmail_create: u_creat() rs=%d\n",rs) ;
#endif

	}

	if (mlp->id.euid == 0) {

#if	CF_DEBUGS
	    debugprintf("lkmail_create: we are effectively ROOT\n") ;
#endif

	    if ((rs == SR_ACCES) || (rs == SR_PERM)) {

#if	CF_DEBUGS
	        debugprintf("lkmail_create: ... bu NOACCESS\n") ;
#endif

	        u_setegid(mlp->id.gid_maildir) ;

	        rs = u_creat(mlp->lockfname,0444) ;
	        lfd = rs ;

#if	CF_DEBUGS
	        debugprintf("lkmail_create: u_creat() rs=%d\n",rs) ;
#endif

	        u_setegid(mlp->id.gid) ;

	    }

	    if ((rs == SR_ACCES) || (rs == SR_PERM)) {

	        u_seteuid(mlp->id.uid_maildir) ;

	        rs = u_creat(mlp->lockfname,0444) ;
	        lfd = rs ;

	        u_seteuid(mlp->id.uid) ;

	    } /* end if */

	} /* end if (playing as super user) */

#if	CF_DEBUGS
	debugprintf("lkmail_create: ret rs=%d lfd=%u\n",rs,lfd) ;
#endif

	if (rs >= 0)
	    mlp->lfd = lfd ;

	} else
	    rs = SR_TXTBSY ;

	return (rs >= 0) ? lfd : rs ;
}
/* end subroutine (lkmail_create) */


int lkmail_unlink(mlp)
LKMAIL		*mlp ;
{
	int		rs = SR_ACCES ;

	if (mlp == NULL) return SR_FAULT ;

	if (mlp->magic != LKMAIL_MAGIC) return SR_NOTOPEN ;

	if (mlp->id.egid != mlp->id.gid) {
	    u_setegid(mlp->id.egid) ;
	    rs = u_unlink(mlp->lockfname) ;
	    u_setegid(mlp->id.gid) ;
	}

	if (rs == SR_ACCES) {
	    rs = u_unlink(mlp->lockfname) ;
	}

	if (mlp->id.euid == 0) {

	    if (rs == SR_ACCES) {
	        u_setegid(mlp->id.gid_maildir) ;
	        rs = u_unlink(mlp->lockfname) ;
	        u_setegid(mlp->id.gid) ;
	    }

	    if (rs == SR_ACCES) {
	        u_seteuid(mlp->id.uid_maildir) ;
	        rs = u_unlink(mlp->lockfname) ;
	        u_seteuid(mlp->id.uid) ;
	    }

	} /* end if (trying as superuser) */

	return rs ;
}
/* end subroutine (lkmail_unlink) */


int lkmail_old(mlp,daytime,age)
LKMAIL		*mlp ;
time_t		daytime ;
int		age ;
{
	struct ustat	sb ;
	int		rs ;
	int		f ;

	if (mlp == NULL) return SR_FAULT ;

	if (mlp->magic != LKMAIL_MAGIC) return SR_NOTOPEN ;

	if (age < 0)
	    age = LKMAIL_AGE ;

	rs = u_stat(mlp->lockfname,&sb) ;

	f = ((daytime - sb.st_mtime) > age) ;
	return (rs >= 0) ? f : rs ;
}
/* end subroutine (lkmail_old) */


