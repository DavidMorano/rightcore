/* getfstype */

/* get the type of the filesystem for an FD */


/* revision history:

	= 1999-09-27, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 1999 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	We return the file-system type associated with the file attached to the
	given file-descriptor.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/statvfs.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* external subroutines */

extern int	snwcpy(char *,int,const char *,int) ;


/* external variables */


/* local structures */


/* local variables */


/* local variables */


/* exported subroutines */


int getfstype(char *nbuf,int nlen,int fd)
{
	struct statvfs	vsb ;
	int		rs ;

	if (nbuf == NULL) return SR_FAULT ;

	if ((rs = u_fstatvfs(fd,&vsb)) >= 0) {
	    cchar	*cp = vsb.f_basetype ;
	    int		cl = strnlen(vsb.f_basetype,FSTYPSZ) ;
	    rs = snwcpy(nbuf,nlen,cp,cl) ;
	}

	return rs ;
}
/* end subroutine (getfstype) */


