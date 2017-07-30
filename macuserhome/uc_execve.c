/* uc_execve */

/* interface component for UNIX® library-3c */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_ISAEXEC	1		/* try ISA-EXEC */


/* revision history:

	= 1998-11-28, David A­D­ Morano
	How did we get along without this for over 10 years?

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        For now, this just is a regular (fairly so at any rate) 'exec(2)'-like
        subroutine.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<unistd.h>
#include	<stdlib.h>

#include	<vsystem.h>


/* local defines */


/* external subroutines */

extern int	mkexpandpath(char *,cchar *,int) ;


/* forward references */


/* exported subroutines */


int uc_execve(cchar *fname,cchar **av,cchar **ev)
{
	const int	elen = MAXPATHLEN ;
	int		rs ;
	char		*ebuf ;

#if	CF_DEBUGS
	debugprintf("uc_execve: ent fn=%s\n",fname) ;
#endif

	if ((rs = uc_libmalloc((elen+1),&ebuf)) >= 0) {
	    if ((rs = mkexpandpath(ebuf,fname,-1)) > 0) {
		rs = u_execve(ebuf,av,ev) ;
	    } else if (rs == 0) {
		rs = u_execve(fname,av,ev) ;
	    }
	    uc_libfree(ebuf) ;
	} /* end if (m-a) */

#if	CF_DEBUGS
	debugprintf("uc_execve: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (uc_execve) */


