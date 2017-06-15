/* pmq */

/* Posix Message Queue (PMQ) */


/* revision history:

	= 1999-07-23, David A­D­ Morano
	This module was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	PMQ_INCLUDE
#define	PMQ_INCLUDE	1


#include	<envstandards.h>
#include	<sys/types.h>
#include	<sys/param.h>
#include	<mqueue.h>
#include	<localmisc.h>


#define	PMQ_MAGIC	0x31419876
#define	PMQ		struct pmq


struct pmq {
	uint		magic ;
	mqd_t		p ;
	char		name[MAXPATHLEN + 1] ;
 } ;


#if	(! defined(PMQ_MASTER)) || (PMQ_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	pmq_open(PMQ *,cchar *,int,mode_t,struct mq_attr *) ;
extern int	pmq_close(PMQ *) ;
extern int	pmq_send(PMQ *,cchar *,int,uint) ;
extern int	pmq_receive(PMQ *,char *,int,uint *) ;
extern int	pmq_setattr(PMQ *,struct mq_attr *,struct mq_attr *) ;
extern int	pmq_getattr(PMQ *,struct mq_attr *) ;
extern int	pmq_notify(PMQ *,struct sigevent *) ;
extern int	pmq_unlink(PMQ *) ;

extern int	uc_unlinkpmq(cchar *) ;

#ifdef	__cplusplus
}
#endif

#endif /* PMQ_MASTER */

#endif /* PMQ_INCLUDE */


