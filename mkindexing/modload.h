/* modload */


/* revision history:

	= 1998-02-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	MODLOAD_INCLUDE
#define	MODLOAD_INCLUDE	1


#include	<envstandards.h>
#include	<sys/types.h>
#include	<localmisc.h>


#define	MODLOAD			struct modload_head
#define	MODLOAD_MID		struct modload_mid
#define	MODLOAD_MAGIC		0x99447246
#define	MODLOAD_DEFENTS		(44 * 1000)

#define	MODLOAD_OLIBVAR		(1<<0)
#define	MODLOAD_OPRS		(1<<1)
#define	MODLOAD_OSDIRS		(1<<2)
#define	MODLOAD_OAVAIL		(1<<3)	/* make global-syms available */
#define	MODLOAD_OSELF		(1<<4)


struct modload_mid {
	const char	*name ;
	int		mv[1] ;
} ;

struct modload_head {
	uint		magic ;
	void		*sop ;		/* shared-object (SO) pointer */
	const char	*modname ;
	MODLOAD_MID	*midp ;
} ;


#if	(! defined(MODLOAD_MASTER)) || (MODLOAD_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	modload_open(MODLOAD *,cchar *,cchar *,cchar *,int,cchar **) ;
extern int	modload_getmv(MODLOAD *,int) ;
extern int	modload_getmva(MODLOAD *,int *,int) ;
extern int	modload_getsym(MODLOAD *,cchar *,const void **) ;
extern int	modload_close(MODLOAD *) ;

#ifdef	__cplusplus
}
#endif

#endif /* MODLOAD_MASTER */

#endif /* MODLOAD_INCLUDE */


