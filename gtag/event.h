/* event */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


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


