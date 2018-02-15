/* findinline */


/* revision history:

	= 1998-09-10, David A­D­ Morano
	This module was changed to serve in the REFERM program.

	= 2005-10-01, David A­D­ Morano
        This was changed to work in the MMCITE program. The old REFERM program
        is really obsolete. It used a database lookup strategy to remote
        databases. The high-level problem is: what to do if the cited BIB entry
        isn't found? How does a maintainer of the present (local) document know
        what that BIB entry was? The new strategy (implemented by the MMCITE
        program) is more like what is done with BibTeX in the TeX (or LaTeX)
        world. All BIB databases are really expected to be maintained by the
        document creator -- not some centralized entiry. The older centralized
        model reflected more the use in the corporate world (where different
        people create BIB entries) than in the more "modern"
        personal-responsibility type of world! :-) Anyway, this is the way the
        gods seem to now want to do things. Deal with it!

*/

/* Copyright © 1998,2005 David A­D­ Morano.  All rights reserved. */


#ifndef	FINDINLINE_INCLUDE
#define	FINDINLINE_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */
#include	<sys/types.h>
#include	<localmisc.h>		/* extra types */


#define	FINDINLINE		struct findinline


struct findinline {
	const char	*sp ;		/* "start" pointer */
	const char	*kp, *vp ;
	int		kl, vl ;
} ;

#if	(! defined(FINDINLINE_MASTER)) || (FINDINLINE_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	findinline(FINDINLINE *,const char *,int) ;

#ifdef	__cplusplus
}
#endif

#endif /* FINDINLINE_MASTER */


#endif /* FINDINLINE_INCLUDE */



