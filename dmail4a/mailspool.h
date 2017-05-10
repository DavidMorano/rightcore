/* mailspool */


/* revision history:

	= 1998-02-01, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	MAILSPOOL_INCLUDE
#define	MAILSPOOL_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */
#include	<sys/types.h>
#include	<sys/param.h>
#include	<localmisc.h>


#define	MAILSPOOL	struct mailspool_head
#define	MAILSPOOL_MAGIC	0x95437652


struct mailspool_head {
	uint		magic ;
	cchar		*lfname ;	/* lock file-name */
	int		mfd ;		/* mail-file descriptor */
	int		f_created ;
} ;


#if	(! defined(MAILSPOOL_MASTER)) || (MAILSPOOL_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int mailspool_open(MAILSPOOL *,cchar *,cchar *,int,mode_t,int) ;
extern int mailspool_setlockinfo(MAILSPOOL *,cchar *,int) ;
extern int mailspool_close(MAILSPOOL *) ;

#ifdef	__cplusplus
}
#endif

#endif /* MAILSPOOL_MASTER */

#endif /* MAILSPOOL_INCLUDE */


