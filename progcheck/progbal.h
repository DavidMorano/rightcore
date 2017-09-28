/* progbal */

/* HEX decoder */


/* revision history:

	= 2016-02-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 2016 David A­D­ Morano.  All rights reserved. */

#ifndef	PROGBAL_INCLUDE
#define	PROGBAL_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */
#include	<sys/types.h>
#include	<localmisc.h>


#define	PROGBAL_MAGIC	0x13f3c203
#define	PROGBAL		struct progbal_head
#define	PROGBAL_NCH	3


struct progbal_head {
	uint		magic ;
	int		counts[PROGBAL_NCH] ;
	int		f_fail ;
} ;


#if	(! defined(PROGBAL_MASTER)) || (PROGBAL_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int progbal_start(PROGBAL *) ;
extern int progbal_load(PROGBAL *,cchar *,int) ;
extern int progbal_read(PROGBAL *,char *,int) ;
extern int progbal_finish(PROGBAL *) ;

#ifdef	__cplusplus
}
#endif

#endif /* PROGBAL_MASTER */

#endif /* PROGBAL_INCLUDE */


