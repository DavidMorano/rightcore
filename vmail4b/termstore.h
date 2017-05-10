/* termstore */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


#ifndef	TERMSTORE_INCLUDE
#define	TERMSTOTE_INCLUDE


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<termios.h>


struct termstore {
	struct termios	settings ;
	int		f_stored ;
	int		mode ;
} ;


#ifdef	__cplusplus
extern "C" {
#endif

extern int	termstore_save(struct termstore *,int) ;
extern int	termstore_restore(struct termstore *,int) ;

#ifdef	__cplusplus
}
#endif

#endif /* TERMSTORE_INCLUDE */


