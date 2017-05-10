/* mailmsgfrom */


/* Copyright © 2002 David A­D­ Morano.  All rights reserved. */

#ifndef	MAILMSGFROM_INCLUDE
#define	MAILMSGFROM_INCLUDE		1


#include	<envstandards.h>	/* MUST be first to configure */
#include	<sys/types.h>
#include	<time.h>
#include	<localmisc.h>


#define	MAILMSGFROM	struct mailmsgfrom_head


struct mailmsgfrom_head {
	time_t		ti_msg ;
	char		*fbuf ;		/* FROM-buffer */
	int		flen ;		/* FROM-buffer size */
	int		fl ;		/* FROM-buffer result length */
} ;


#if	(! defined(MAILMSGFROM_MASTER)) || (MAILMSGFROM_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	mailmsgfrom_start(MAILMSGFROM *,char *,int) ;
extern int	mailmsgfrom_test(MAILMSGFROM *,time_t) ;
extern int	mailmsgfrom_loadfrom(MAILMSGFROM *,cchar *,int) ;
extern int	mailmsgfrom_finish(MAILMSGFROM *) ;

#ifdef	__cplusplus
}
#endif

#endif /* MAILMSGFROM_MASTER */

#endif /* MAILMSGFROM_INCLUDE */


