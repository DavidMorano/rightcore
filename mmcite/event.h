/* event */


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


#ifndef	EVENT_INCLUDE
#define	EVENT_INCLUDE	1


#include	<sys/types.h>


#define	EVENT		struct event_head


struct event_head {
	offset_t	offset ;
	const char	*citekey ;
} ;


#if	(! defined(EVENT_MASTER)) || (EVENT_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	event_start(EVENT *,offset_t,const char *) ;
extern int	event_offset(EVENT *,offset_t *) ;
extern int	event_citekey(EVENT *,const char **) ;
extern int	event_finish(EVENT *) ;

#ifdef	__cplusplus
}
#endif

#endif /* EVENT_MASTER */


#endif /* EVENT_INCLUDE */



