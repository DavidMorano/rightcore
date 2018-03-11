/* csro */

/* csro processing structures */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	CSRO_INCLUDE
#define	CSRO_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>

#include	<hdb.h>
#include	<vecstr.h>
#include	<vecobj.h>
#include	<localmisc.h>


/* object defines */

#define	CSRO_MAGIC	0x99723317
#define	CSRO_DEFENTS	10
#define	CSRO		struct csro_head
#define	CSRO_VALUE	struct csro_value
#define	CSRO_NCURSOR	int
#define	CSRO_VCURSOR	int


struct csro_head {
	uint		magic ;
	VECSTR		names ;			/* hold mailnames */
	VECOBJ		entries ;		/* hold entire entries */
} ;

struct csro_value {
	ULONG		mailoff ;
	const char	*mailname ;
	const char	*fname ;
} ;


#if	(! defined(CSRO_MASTER)) || (CSRO_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int csro_start(CSRO *,int) ;
extern int csro_finish(CSRO *) ;
extern int csro_add(CSRO *,const char *,const char *,ULONG) ;
extern int csro_countnames(CSRO *) ;
extern int csro_count(CSRO *) ;
extern int csro_ncurbegin(CSRO *,CSRO_NCURSOR *) ;
extern int csro_ncurend(CSRO *,CSRO_NCURSOR *) ;
extern int csro_getname(CSRO *,CSRO_NCURSOR *,const char **) ;
extern int csro_vcurbegin(CSRO *,CSRO_VCURSOR *) ;
extern int csro_vcurend(CSRO *,CSRO_VCURSOR *) ;
extern int csro_getvalue(CSRO *,cchar *,CSRO_VCURSOR *, CSRO_VALUE **) ;
extern int csro_already(CSRO *,cchar *,const char *,ULONG) ;

#ifdef	__cplusplus
}
#endif

#endif /* CSRO_MASTER */

#endif /* CSRO_INCLUDE */


