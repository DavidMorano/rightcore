/* timeval */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	TIMEVAL_INCLUDE
#define	TIMEVAL_INCLUDE	1


#include	<envstandards.h>

#include	<sys/types.h>
#include	<time.h>


#ifdef	__cplusplus
extern "C" {
#endif

extern int timeval_add(struct timeval *,struct timeval *,struct timeval *) ;
extern int timeval_sub(struct timeval *,struct timeval *,struct timeval *) ;

#ifdef	__cplusplus
}
#endif

#endif /* TIMEVAL_INCLUDE */


