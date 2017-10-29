/* strfilter */


/* Copyright © 2009 David A­D­ Morano.  All rights reserved. */

#ifndef	STRFILTER_INCLUDE
#define	STRFILTER_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<limits.h>

#include	<vsystem.h>
#include	<vecstr.h>
#include	<localmisc.h>


/* object defines */
#define	STRFILTER		struct strfilter_head
#define	STRFILTER_FL		struct strfilter_flags


/* options */
#define	STRFILTER_MCARRIAGE	0x0001


/* constants */

#ifdef	LINE_MAX
#define	STRFILTER_BUFLEN	MAX(LINE_MAX,4096)
#else
#define	STRFILTER_BUFLEN	4096
#endif


struct strfilter_flags {
	uint		sslist:1 ;	/* select */
	uint		sxlist:1 ;	/* exclude */
} ;

struct strfilter_head {
	vecstr		sslist ;	/* select */
	vecstr		sxlist ;	/* exclude */
	STRFILTER_FL	f ;
} ;


#if	(! defined(STRFILTER_MASTER)) || (STRFILTER_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int strfilter_start(STRFILTER *,const char *,const char *) ;
extern int strfilter_check(STRFILTER *,const char *,int) ;
extern int strfilter_finish(STRFILTER *) ;

#ifdef	__cplusplus
}
#endif

#endif /* STRFILTER_MASTER */

#endif /* STRFILTER_INCLUDE */


