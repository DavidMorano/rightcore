/* ass */

/* allocated string */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	ASS_INCLUDE
#define	ASS_INCLUDE	1


#include	<sys/types.h>

#include	<localmisc.h>


/* object defines */

#define	ASS		struct ass_head


struct ass_head {
	char		*s ;
	int		len ;
	int		e ;
} ;


#if	(! defined(ASS_MASTER)) || (ASS_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int ass_start(ASS *) ;
extern int ass_add(ASS *,int) ;
extern int ass_finish(ASS *) ;

#ifdef	__cplusplus
}
#endif

#endif /* ASS_MASTER */

#endif /* ASS_INCLUDE */


