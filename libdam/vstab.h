/* vstab */

/* vector string structures (Vector String) */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	VSTAB_INCLUDE
#define	VSTAB_INCLUDE	1


#include	<envstandards.h>

#include	<sys/types.h>

#include	<localmisc.h>


/* object defines */

#define	VSTAB		struct vstab_head


struct vstab_head {
	void		**va ;
	int		i ;		/* highest index used so far */
	int		n ;		/* current extent of array */
	int		c ;		/* count of items in list */
	int		osize ;		/* object size */
} ;


#if	(! defined(VSTAB_MASTER)) || (VSTAB_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int vstab_start(VSTAB *,int) ;
extern int vstab_getfd(VSTAB *,int,void **) ;
extern int vstab_getval(VSTAB *,int,void **) ;
extern int vstab_getkey(VSTAB *,int,void **) ;
extern int vstab_del(VSTAB *,int) ;
extern int vstab_count(VSTAB *) ;
extern int vstab_search(VSTAB *,const char *,int (*)(),char **) ;
extern int vstab_find(VSTAB *,const char *) ;
extern int vstab_findn(VSTAB *,const char *,int) ;
extern int vstab_finder(VSTAB *,const char *,int (*)(),char **) ;
extern int vstab_getvec(VSTAB *,const void ***) ;
extern int vstab_audit(VSTAB *) ;
extern int vstab_finish(VSTAB *) ;

#ifdef	__cplusplus
}
#endif

#endif /* VSTAB_MASTER */

#endif /* VSTAB_INCLUDE */


