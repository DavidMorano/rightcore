/* findinline */


#ifndef	FINDINLINE_INCLUDE
#define	FINDINLINE_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */
#include	<sys/types.h>
#include	<localmisc.h>		/* extra types */


#define	FINDINLINE		struct findinline


struct findinline {
	const char	*sp ;		/* "start" pointer */
	const char	*kp, *vp ;
	int		kl, vl ;
} ;

#if	(! defined(FINDINLINE_MASTER)) || (FINDINLINE_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	findinline(FINDINLINE *,const char *,int) ;

#ifdef	__cplusplus
}
#endif

#endif /* FINDINLINE_MASTER */


#endif /* FINDINLINE_INCLUDE */



