/* chownsame */

/* make all directories in a directory path */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-08-10, David A­D­ Morano
	This subroutine was originally written.  This subroutines (or something
	similar to it) is standard on some UNIXes but not on others, so it is
	now provided.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine will create all of the directories in the specified
	directory path if they do not exist already.

	Synopsis:

	int chownsame(dname,dm)
	const char	dname[] ;
	mode_t		dm ;

	Arguments:

	- dname		direcrtory path to a new directory to create
	- dm		newly created directories are created with this
			this permissions mode (subject to UMASK restrictions)

	Returns:

	>0		number of directories that were created
	==0		all directories existed
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
	        rs = uc_chown(dname,sb.st_uid,sb.st_gid) ;
	    } /* end if (uc_stat) */
	} /* end if (uc_pathconf) */

	return rs ;
}
/* end subroutine (chownsame) */


