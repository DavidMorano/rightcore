/* emainfo */


/* revision history:

	= 1998-02-12, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	EMAINFO_INCLUDE
#define	EMAINFO_INCLUDE		1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>


#ifndef	UINT
#define	UINT	unsigned int
#endif


/* object defines */

#define	EMAINFO		struct emainfo_head

/* other defines */

#define	EMAINFO_TLOCAL		0
#define	EMAINFO_TUUCP		1
#define	EMAINFO_TARPA		2
#define	EMAINFO_TARPAROUTE	3

#define	EMAINFO_LOCALHOST	"*LOCAL*"


struct emainfo_head {
	const char	*local ;
	const char	*host ;
	int		type ;
	int		llen ;
	int		hlen ;
} ;


#if	(! defined(EMAINFO_MASTER)) || (EMAINFO_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int emainfo(EMAINFO *,const char *,int) ;
extern int emainfo_mktype(EMAINFO *,int,char *,int) ;

#ifdef	__cplusplus
}
#endif

#endif /* EMAINFO_MASTER */

#endif /* EMAINFO_INCLUDE */


