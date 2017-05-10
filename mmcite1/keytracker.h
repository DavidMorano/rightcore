/* INCLUDE keytracker */


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



