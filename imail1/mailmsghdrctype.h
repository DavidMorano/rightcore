/* mailmsghdrctype */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#ifndef	MAILMSGHDRCTYPE_INCLUDE
#define	MAILMSGHDRCTYPE_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>

#include	<localmisc.h>


/* object defines */

#define	MAILMSGHDRCTYPE		struct mailmsghdrctype_head
#define	MAILMSGHDRCTYPE_PARAM	struct mailmsghdrctype_param
#define	MAILMSGHDRCTYPE_NPARAMS	10


struct mailmsghdrctype_param {
	const char	*kp, *vp ;
	int		kl, vl ;
} ;

struct mailmsghdrctype_head {
	const char	*mtp ;		/* main-type */
	const char	*stp ;		/* sub-type */
	int		mtl ;		/* main-type */
	int		stl ;		/* sub-type */
	struct mailmsghdrctype_param	p[MAILMSGHDRCTYPE_NPARAMS] ;
} ;


#if	(! defined(MAILMSGHDRCTYPE_MASTER)) || (MAILMSGHDRCTYPE_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int mailmsghdrctype_start(MAILMSGHDRCTYPE *,const char *,int) ;
extern int mailmsghdrctype_paramget(MAILMSGHDRCTYPE *,int,
		struct mailmsghdrctype_param *) ;
extern int mailmsghdrctype_paramfind(MAILMSGHDRCTYPE *,const char *,
		struct mailmsghdrctype_param *) ;
extern int mailmsghdrctype_finish(MAILMSGHDRCTYPE *) ;

#ifdef	__cplusplus
}
#endif

#endif /* MAILMSGHDRCTYPE_MASTER */

#endif /* MAILMSGHDRCTYPE_INCLUDE */



