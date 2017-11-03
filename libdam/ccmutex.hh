/* ccmutex */
/* lang=C++11 */

/* CCMUTEX object */


/* revision history:

	= 1998-12-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This object implments MUTEX using the POSIX mutex as a base.


*******************************************************************************/

#include	<envstandards.h>
#include	<sys/types.h>
#include	<new>
#include	<vsystem.h>
#include	<ptm.h>
#include	<localmisc.h>


/* local defines */


/* default name spaces */


/* external subroutines */

#if	CF_DEBUGS
extern "C" int	debugprintf(cchar *,...) ;
extern "C" int	strlinelen(cchar *,int,int) ;
#endif


/* local structures */


/* forward references */


/* exported */


class ccmutex {
	PTM		m ;
public:
	ccmutex() {
	    ptm_create(&m,NULL) ;
	} ;
	~ccmutex() {
	    ptm_destroy(&m) ;
	} ;
	ccmutex &operator = (const ccmutex &) = delete ;
	int lock() {
	    return ptm_lock(&m) ;
	} ;
	int unlock() {
	    return ptm_unlock(&m) ;
	} ;
} ; /* end class (ccmutex) */


