/* INCLUDE keytracker */


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


#ifndef	KEYTRACKER_INCLUDE
#define	KEYTRACKER_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>

#include	<bits.h>
#include	<localmisc.h>


#define	KEYTRACKER		struct keytracker_head


struct keytracker_head {
	BITS		dones ;
	const char	*(*keyvals)[2] ;
	int		n ;
} ;


#if	(! defined(KEYTRACKER_MASTER)) || (KEYTRACKER_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	keytracker_start(KEYTRACKER *,const char *(*)[2]) ;
extern int	keytracker_done(KEYTRACKER *,int) ;
extern int	keytracker_more(KEYTRACKER *,const char *) ;
extern int	keytracker_finish(KEYTRACKER *) ;

#ifdef	__cplusplus
}
#endif

#endif /* KEYTRACKER_MASTER */


#endif /* KEYTRACKER_INCLUDE */



