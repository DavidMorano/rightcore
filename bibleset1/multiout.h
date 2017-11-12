/* multiout */

/* perform some output processsing activities */


/* revision history:

	= 2009-04-01, David A­D­ Morano
        This subroutine was written as an enhancement for adding back-matter
        (end pages) to the output document.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#ifndef	MULTIOUT_INCLUDE
#define	MULTIOUT_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>

#include	<bfile.h>
#include	<localmisc.h>


#define	MULTIOUT		struct multiout_head


struct multiout_head {
	bfile		*ofp ;
	int		wlen ;
	int		rs ;
} ;


#ifdef	__cplusplus
extern "C" {
#endif

extern int	mo_start(MULTIOUT *,bfile *) ;
extern int	mo_printf(MULTIOUT *,const char *,...) ;
extern int	mo_finish(MULTIOUT *) ;

#ifdef	__cplusplus
}
#endif

#endif /* MULTIOUT_INCLUDE */


