/* event */


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



