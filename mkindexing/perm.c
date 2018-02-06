/* perm */

/* test the permissions on a file -- similar to 'access(2)' */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_SPERM	1		/* compile in |sperm()| */


/* revision history:

	= 1998-08-15, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This module is sort of the "effective_user" version of 'access(2)'.
	Implemented within this module are the following interfaces:
 	+ perm(3uc)
	+ fperm(3uc)
	+ sperm(3uc)
	

	Synopsis:

	int perm(fname,uid,gid,groups,am)
	const char	fname[] ;
	uid_t		uid ;
	gid_t		gid ;
	gid_t		groups[] ;
	int		am ;

	Arguments:

	fname	filename to check
	uid	UID to use for the check
	gid	GID to use for the check
	groups	the secondary GIDs to use for check
	am	the access-mode as specified like with 'open(2)' but only
		the lower 3 bits are used, like with 'access(2)'

	Returns:

	0	access if allowed
	<0	access if denied for specified error code


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/stat.h>
#include	<limits.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<string.h>

#include	<vsystem.h>
#include	<ids.h>
#include	<localmisc.h>


/* local defines */

#define	TRY		struct try


/* external subroutines */

extern int	getngroups() ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif


/* external variables */


/* local structures */

struct try {
	struct ustat	*sbp ;
	gid_t		*gids ;
	uid_t		euid ;
	gid_t		egid ;
	int		am ;
	int		f_gidalloc ;
} ;


/* forward references */

static int permer(USTAT *,uid_t,gid_t,gid_t *,int) ;

static int try_start(TRY *,USTAT *,uid_t,gid_t,gid_t *,int) ;
static int try_finish(TRY *) ;
static int try_filetype(TRY *) ;
static int try_root(TRY *) ;
static int try_user(TRY *) ;
static int try_group(TRY *) ;
static int try_other(TRY *) ;


/* local variables */

static int	(*tries[])(TRY *) = {
	try_filetype,
	try_root,
	try_user,
	try_group,
	try_other,
	NULL
} ;


/* exported subroutines */


int perm(cchar *fn,uid_t euid,gid_t egid,gid_t *gids,int am)
{
	USTAT		sb ;
	int		rs ;

	if (fn == NULL) return SR_FAULT ;

	if (fn[0] == '\0') return SR_INVAL ;

#if	CF_DEBUGS
	debugprintf("perm: ent dn=%s am=%03ß\n",fn,am) ;
#endif

	if ((rs = uc_stat(fn,&sb)) >= 0) {
	    rs = permer(&sb,euid,egid,gids,am) ;
	} /* end if (stat) */

#if	CF_DEBUGS
	debugprintf("perm: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (perm) */


int fperm(int fd,uid_t euid,gid_t egid,gid_t *gids,int am)
{
	USTAT		sb ;
	int		rs ;

	if (fd < 0) return SR_BADF ;

	if ((rs = u_fstat(fd,&sb)) >= 0) {
	    rs = permer(&sb,euid,egid,gids,am) ;
	} /* end if (stat) */

	return rs ;
}
/* end subroutine (fperm) */


#if	CF_SPERM
int sperm(IDS *idp,struct ustat *sbp,int am)
{
	const uid_t	euid = idp->euid ;
	const gid_t	egid = idp->egid ;
	gid_t		*gids = idp->gids ;
	if (idp == NULL) return SR_FAULT ;
	if (sbp == NULL) return SR_FAULT ;
	return permer(sbp,euid,egid,gids,am) ;
}
/* end subroutine (sperm) */
#endif /* CF_SPERM */


/* local subroutines */


static int permer(USTAT *sbp,uid_t euid,gid_t egid,gid_t *gids,int am)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (am != 0) {
	    TRY		t ;
	    if ((rs = try_start(&t,sbp,euid,egid,gids,am)) >= 0) {
	        int	i ;
	        for (i = 0 ; tries[i] != NULL ; i += 1) {
	            rs = (*tries[i])(&t) ;
#if	CF_DEBUGS
		    debugprintf("permer: i=%u rs=%d\n",i,rs) ;
#endif
	            if (rs != 0) break ;
	        } /* end for */
	        if (rs == 0) rs = SR_ACCESS ;
	        rs1 = try_finish(&t) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (try) */
	} /* end if (access mode) */

	return rs ;
}
/* end subroutine (perm) */


/* local subroutines */


static int try_start(TRY *tip,USTAT *sbp,uid_t eu,gid_t eg,gid_t *gids,int am)
{
	const int	n = getngroups() ;
	int		rs = SR_OK ;
	if (eu < 0) eu = geteuid() ;
	if (eg < 0) eg = getegid() ;
	memset(tip,0,sizeof(TRY)) ;
	tip->sbp = sbp ;
	tip->euid = eu ;
	tip->egid = eg ;
	tip->gids = gids ;
	tip->am = am ;
	if (gids == NULL) {
	    const int	gsize = ((n+1)*sizeof(gid_t)) ;
	    void	*p ;
	    if ((rs = uc_malloc(gsize,&p)) >= 0) {
		tip->gids = p ;
		tip->f_gidalloc = TRUE ;
	        if ((rs = u_getgroups(n,tip->gids)) >= 0) {
		    tip->gids[rs] = -1 ;
		}
		if (rs < 0) {
		    uc_free(tip->gids) ;
		    tip->gids = NULL ;
		    tip->f_gidalloc = FALSE ;
	        }
	    }
	} /* end if (empty GIDs) */
	return rs ;
}
/* end subroutine (try_start) */


static int try_finish(TRY *tip)
{
	int		rs = SR_OK ;
	int		rs1 ;
	if (tip->f_gidalloc) {
	    tip->f_gidalloc = FALSE ;
	    rs1 = uc_free(tip->gids) ;
	    if (rs >= 0) rs = rs1 ;
	    tip->gids = NULL ;
	}
	return rs ;
}
/* end subroutine (try_finish) */


static int try_filetype(TRY *tip)
{
	struct ustat	*sbp = tip->sbp ;
	const int	ft = (tip->am & S_IFMT) ;
	int		rs = SR_OK ;
	if ((ft != 0) && ((sbp->st_mode & S_IFMT) != ft)) {
	    rs = SR_NOMSG ;
	}
	return rs ;
}
/* end subroutine (try_filetype) */


static int try_root(TRY *tip)
{
	return (tip->euid == 0) ;
}
/* end subroutine (try_root) */


static int try_user(TRY *tip)
{
	struct ustat	*sbp = tip->sbp ;
	const int	am = (tip->am & 0007) ;
	int		rs = SR_OK ;
	if (tip->euid == sbp->st_uid) {
	    const int	um = (sbp->st_mode >> 6) ;
	    rs = ((um&am) == am) ? 1 : SR_ACCES ;
	}
	return rs ;
}
/* end subroutine (try_user) */


static int try_group(TRY *tip)
{
	struct ustat	*sbp = tip->sbp ;
	const int	am = (tip->am & 0007) ;
	int		rs = SR_OK ;
	int		gm ;
	gm = (sbp->st_mode >> 3) ;
	if (tip->egid == sbp->st_gid) {
	    rs = ((gm&am) == am) ? 1 : SR_ACCES ;
	} else if (tip->gids != NULL) {
	    int		i ;
	    int		f = FALSE ;
	    for (i = 0 ; tip->gids[i] >= 0 ; i += 1) {
	        f = (sbp->st_gid == tip->gids[i]) ;
		if (f) break ;
	    } /* end for */
	    if (f) {
	        rs = ((gm&am) == am) ? 1 : SR_ACCES ;
	    }
	}
	return rs ;
}
/* end subroutine (try_group) */


static int try_other(TRY *tip)
{
	struct ustat	*sbp = tip->sbp ;
	const int	am = (tip->am & 0007) ;
	int		rs ;
	{
	    const int	om = sbp->st_mode ;
	    rs = ((om&am) == am) ;
	}
	return rs ;
}
/* end subroutine (try_other) */


