/* chownsame */

/* make all directories in a directory path */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-08-10, David A­D­ Morano
	This subroutine was originally written.  

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine changes the owner of a file (or directory) to be the
	same as another file (or directory) given as a reference.

	Synopsis:

	int chownsame(cchar *dname,cchar *ref)

	Arguments:

	dname		direcrtory path to a new directory to create
	ref		reference file or directory

	Returns:

	>0		ownership changed
	==0		ownership not changed
	<0		represents a system error


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<string.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */


/* external variables */


/* forward references */


/* local variables */


/* external subroutines */


int chownsame(cchar *dname,cchar *ref)
{
	const int	n = _PC_CHOWN_RESTRICTED ;
	int		rs ;
	int		f = FALSE ;

	if (dname == NULL) return SR_FAULT ;
	if (ref == NULL) return SR_FAULT ;

	if (dname[0] == NULL) return SR_INVALID ;
	if (ref[0] == NULL) return SR_INVALID ;

#if	CF_DEBUGS
	debugprintf("chownsame: ent dname=%s\n",dname) ;
#endif

	if ((rs = uc_pathconf(dname,n,NULL)) == 0) {
	    USTAT	sb ;
	    if ((rs = uc_stat(ref,&sb)) >= 0) {
	        f = TRUE ;
	        rs = uc_chown(dname,sb.st_uid,sb.st_gid) ;
	    } /* end if (uc_stat) */
	} /* end if (uc_pathconf) */

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (chownsame) */


