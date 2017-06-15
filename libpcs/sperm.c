/* sperm */
/* lang=C99 */

/* test the permissions on a file -- similar to 'access(2)' */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-08-15, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine is sort of the "effective_user" version of 'access(2)'.
	It has more features that enable it to perform the function of
	'access(2)' as well as other variations.

	Synopsis:

	int sperm(idp,sbp,am)
	IDS		*idp ;
	struct ustat	*sbp ;
	int		am ;

	Arguments:

	idp	pointer to IDS object
	sbp	pointer to file status structure ('stat(2)')
	am	the access-mode as specified like with 'open(2)' but only
		the lower 3 bits are used, like with 'access(2)'

	Returns:

	0	access if allowed
	<0	access if denied for specified error code


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<limits.h>
#include	<unistd.h>
#include	<fcntl.h>

#include	<vsystem.h>
#include	<ids.h>
#include	<localmisc.h>


/* local namespaces */


/* local defines */

#define	TRY		struct try


/* external subroutines */

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif


/* external variables */


/* local structures */

struct try {
	struct ustat	*sbp ;
	IDS		*idp ;
	int		am ;
} ;


/* forward references */

static int try_start(TRY *,IDS *,struct ustat *,int) ;
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


int sperm(IDS *idp,struct ustat *sbp,int am)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (idp == NULL) return SR_INVAL ;
	if (sbp == NULL) return SR_INVAL ;

	if (am != 0) {
	    TRY		t ;
	    if ((rs = try_start(&t,idp,sbp,am)) >= 0) {
	        int	i ;
	        for (i = 0 ; tries[i] != NULL ; i += 1) {
	            rs = (*tries[i])(&t) ;
	            if (rs != 0) break ;
	        } /* end for */
	        if (rs == 0) rs = SR_ACCESS ;
	        rs1 = try_finish(&t) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (try) */
	} /* end if (access mode) */

	return rs ;
}
/* end subroutine (sperm) */


/* local subroutines */


static int try_start(TRY *tip,IDS *idp,struct ustat *sbp,int am)
{
	tip->idp = idp ;
	tip->sbp = sbp ;
	tip->am = am ;
	return SR_OK ;
}
/* end subroutine (try_start) */


static int try_finish(TRY *tip)
{
	if (tip == NULL) return SR_FAULT ;
	return SR_OK ;
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
	IDS		*idp = tip->idp ;
	return (idp->euid == 0) ;
}
/* end subroutine (try_root) */


static int try_user(TRY *tip)
{
	struct ustat	*sbp = tip->sbp ;
	IDS		*idp = tip->idp ;
	const int	am = (tip->am & 0007) ;
	int		rs = SR_OK ;
	if (idp->euid == sbp->st_uid) {
	    const int	um = (sbp->st_mode >> 6) ;
	    rs = ((um&am) == am) ? 1 : SR_ACCES ;
	}
	return rs ;
}
/* end subroutine (try_user) */


static int try_group(TRY *tip)
{
	struct ustat	*sbp = tip->sbp ;
	IDS		*idp = tip->idp ;
	const int	am = (tip->am & 0007) ;
	int		rs = SR_OK ;
	int		gm ;
	gm = (sbp->st_mode >> 3) ;
	if (idp->egid == sbp->st_gid) {
	    rs = ((gm&am) == am) ? 1 : SR_ACCES ;
	} else if (idp->gids != NULL) {
	    int		i ;
	    int		f = FALSE ;
	    for (i = 0 ; idp->gids[i] >= 0 ; i += 1) {
	        f = (sbp->st_gid == idp->gids[i]) ;
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


