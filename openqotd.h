/* openqotd */


/* revision history:

	= 1998-02-01, David A­D­ Morano
	This subroutine was adopted for use from the DWD program.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	OPENQOTD_INCLUDE
#define	OPENQOTD_INCLUDE	1


#if	(! defined(OPENQOTD_MASTER)) || (OPENQOTD_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int openqotd(const char *,int,int,int) ;

#ifdef	__cplusplus
}
#endif

#endif /* OPENQOTD_MASTER */

#endif /* OPENQOTD_INCLUDE */


