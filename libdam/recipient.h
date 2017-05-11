/* recipient INCLUDE */

/* recipient processing structures */


/* revision history:

	= 1998-02-01, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	RECIPIENT_INCLUDE
#define	RECIPIENT_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>

#include	<hdb.h>
#include	<vecstr.h>
#include	<localmisc.h>


#define	RECIPIENT		struct recipient_head
#define	RECIPIENT_VAL		struct recipient_value
#define	RECIPIENT_HCUR		int
#define	RECIPIENT_VCUR		HDB_CUR


#define	RECIPIENT_NOHOST	"*nohost*"


struct recipient_head {
	HDB		hash ;		/* hold entire entries */
	VECSTR		names ;		/* hold host names */
} ;

struct recipient_value {
	const char	*a ;		/* memory-allocation */
	const char	*hostpart ;
	const char	*localpart ;
	int		type ;
} ;


#if	(! defined(RECIPIENT_MASTER)) || (RECIPIENT_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int recipient_start(RECIPIENT *,int) ;
extern int recipient_finish(RECIPIENT *) ;
extern int recipient_add(RECIPIENT *,const char *,const char *,int) ;
extern int recipient_counthosts(RECIPIENT *) ;
extern int recipient_count(RECIPIENT *) ;
extern int recipient_hcurbegin(RECIPIENT *,RECIPIENT_HCUR *) ;
extern int recipient_hcurend(RECIPIENT *,RECIPIENT_HCUR *) ;
extern int recipient_enumhost(RECIPIENT *,RECIPIENT_HCUR *,
			const char **) ;
extern int recipient_vcurbegin(RECIPIENT *,RECIPIENT_VCUR *) ;
extern int recipient_vcurend(RECIPIENT *,RECIPIENT_VCUR *) ;
extern int recipient_fetchvalue(RECIPIENT *,const char *,
		RECIPIENT_VCUR *, RECIPIENT_VAL **) ;
extern int recipient_already(RECIPIENT *,const char *,const char *,int) ;

#ifdef	__cplusplus
}
#endif

#endif /* RECIPIENT_MASTER */

#endif /* RECIPIENT_INCLUDE */


