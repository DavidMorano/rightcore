/* ucmallreg */


/* revision history:

	= 1998-02-01, David A­D­ Morano

	This subroutine was originally written.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This include provides the array indices for accessing the
	various memory-allocation information elements.


*******************************************************************************/

#ifndef	UCMALLREG_INCLUDE
#define	UCMALLREG_INCLUDE	1


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>

#include	<localmisc.h>


#define		UCMALLREG_CUR	struct ucmallreg_cur
#define		UCMALLREG_REG	struct ucmallreg_reg


enum ucmallregs {
	ucmallreg_used,
	ucmallreg_usedmax,
	ucmallreg_outnum,
	ucmallreg_outnummax,
	ucmallreg_outsize,
	ucmallreg_outsizemax,
	ucmallreg_under,
	ucmallreg_over,
	ucmallreg_notalloc,
	ucmallreg_notfree,
	ucmallreg_overlast
} ;

struct ucmallreg_cur {
	int	i ;
} ;

struct ucmallreg_reg {
	caddr_t	addr ;
	uint	size ;
} ;


#if	(! defined(UCMALLREG_MASTER)) || (UCMALLREG_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int ucmallreg_curbegin(UCMALLREG_CUR *) ;
extern int ucmallreg_curend(UCMALLREG_CUR *) ;
extern int ucmallreg_enum(UCMALLREG_CUR *,UCMALLREG_REG *) ;

#ifdef	__cplusplus
}
#endif


#endif /* UCMALLREG_MASTER */

#endif /* UCMALLREG_INCLUDE */


