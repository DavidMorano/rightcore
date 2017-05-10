/* keyname */


#ifndef	KEYNAME_INCLUDE
#define	KEYNAME_INCLUDE	1


#include	<envstandards.h>

#include	<sys/types.h>
#include	<time.h>

#include	<localmisc.h>


/* object defines */

#define	KEYNAME			struct keyname_head

#define	KEYNAME_STORELEN	100
#define	KEYNAME_USERNAMELEN	32


struct keyname_head {
	const char	*last ;
	const char	*first ;
	const char	*m1 ;
	const char	*m2 ;
	int		storelen ;
	char		store[KEYNAME_STORELEN + 1] ;
} ;


#if	(! defined(KEYNAME_MASTER)) || (KEYNAME_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	keyname_initparse(KEYNAME *,const char *,int) ;

#ifdef	__cplusplus
}
#endif

#endif /* KEYNAME_MASTER */


#endif /* KEYNAME_INCLUDE */



